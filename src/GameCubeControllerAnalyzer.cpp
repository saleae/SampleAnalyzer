#include "GameCubeControllerAnalyzer.h"

#include "GameCubeControllerAnalyzerSettings.h"

#include <AnalyzerChannelData.h>
#include <vector>

GameCubeControllerAnalyzer::GameCubeControllerAnalyzer()
    : Analyzer2(), mSettings( new GameCubeControllerAnalyzerSettings() ), mSimulationInitilized( false )
{
    SetAnalyzerSettings( mSettings.get() );
}

GameCubeControllerAnalyzer::~GameCubeControllerAnalyzer()
{
    KillThread();
}

void GameCubeControllerAnalyzer::SetupResults()
{
    mResults.reset( new GameCubeControllerAnalyzerResults( this, mSettings.get() ) );
    SetAnalyzerResults( mResults.get() );
    mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

void GameCubeControllerAnalyzer::WorkerThread()
{
    mSampleRateHz = GetSampleRate();

    mGamecube = GetAnalyzerChannelData( mSettings->mInputChannel );

    // start at the first falling edge
    if( mGamecube->GetBitState() == BIT_HIGH )
        mGamecube->AdvanceToNextEdge();

    while( true )
    {
        bool idle = false;
        bool error = false;
        U64 error_frame_start_sample, error_frame_end_sample;
        U64 last_processed_sample;
        std::vector<Frame> packet;

        // construct packet
        while( !idle )
        {
            U64 frame_start_sample = mGamecube->GetSampleNumber();
            ByteDecodeStatus result = DecodeByte();
            U64 frame_end_sample = mGamecube->GetSampleNumber();

            Frame frame;
            frame.mStartingSampleInclusive = frame_start_sample;
            frame.mEndingSampleInclusive = frame_end_sample;
            frame.mData1 = result.byte;
            frame.mFlags = 0;
            frame.mType = DATA;

            if( result.error )
            {
                if( !error )
                {
                    error_frame_start_sample = frame_start_sample;
                }
                error = true;
            }
            if( result.idle )
            {
                if( error )
                {
                    error_frame_end_sample = frame_start_sample;
                }
                idle = true;
            }

            if( !idle && !error )
            {
                packet.push_back( frame );
            }
            else if( idle && error )
            {
                frame.mStartingSampleInclusive = error_frame_start_sample;
                frame.mEndingSampleInclusive = error_frame_end_sample;
                frame.mData1 = 0;
                frame.mFlags |= DISPLAY_AS_ERROR_FLAG;
                frame.mType = ERROR;
                packet.push_back( frame );
            }

            last_processed_sample = frame_end_sample;
        }

        // process packet
        if( !error )
        {
            DecodePacket( packet );
        }

        // commit
        for( Frame& frame : packet )
        {
            mResults->AddFrame( frame );
            mResults->CommitResults();
        }
        if( !error )
        {
            mResults->CommitPacketAndStartNewPacket();
        }
        else
        {
            mResults->CancelPacketAndStartNewPacket();
        }

        ReportProgress( last_processed_sample );
        CheckIfThreadShouldExit();
    }
}

bool GameCubeControllerAnalyzer::NeedsRerun()
{
    return false;
}

U32 GameCubeControllerAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate,
                                                        SimulationChannelDescriptor** simulation_channels )
{
    if( mSimulationInitilized == false )
    {
        mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
        mSimulationInitilized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 GameCubeControllerAnalyzer::GetMinimumSampleRateHz()
{
    return 2000000;
}

const char* GameCubeControllerAnalyzer::GetAnalyzerName() const
{
    return "GameCube";
}

const char* GetAnalyzerName()
{
    return "GameCube";
}

Analyzer* CreateAnalyzer()
{
    return new GameCubeControllerAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
    delete analyzer;
}

U64 GameCubeControllerAnalyzer::GetPulseWidthNs( U64 start_edge, U64 end_edge )
{
    return ( end_edge - start_edge ) * 1e9 / mSampleRateHz;
}

GameCubeControllerAnalyzer::ByteDecodeStatus GameCubeControllerAnalyzer::DecodeByte()
{
    ByteDecodeStatus status;
    U64 starting_sample, ending_sample, falling_edge_sample, rising_edge_sample;

    for( U8 bit = 0; bit < 8; bit++ )
    {
        // compute low time
        starting_sample = falling_edge_sample = mGamecube->GetSampleNumber();
        mGamecube->AdvanceToNextEdge();
        rising_edge_sample = mGamecube->GetSampleNumber();
        U64 low_time = GetPulseWidthNs( falling_edge_sample, rising_edge_sample );

        if( low_time < 2000 )
        {
            // detected a 1
            status.byte |= 1 << ( 7 - bit );
            // if a 0 is detected, do nothing
        }
        else if( low_time >= 5000 )
        {
            // neither a 1 nor 0 detected, data is corrupt
            status.error = true;
            mGamecube->AdvanceToNextEdge();
            break;
        }

        // compute high time
        mGamecube->AdvanceToNextEdge();
        ending_sample = falling_edge_sample = mGamecube->GetSampleNumber();
        U64 high_time = GetPulseWidthNs( rising_edge_sample, falling_edge_sample );

        // the line is idle - packet complete
        if( high_time > 4000 )
        {
            if( status.byte != 0x80 || bit != 0 )
            {
                // not a stop bit
                status.error = true;
            }
            status.idle = true;
            break;
        }
        else
        {
            U64 middle_sample = ( starting_sample + ending_sample ) / 2;
            mResults->AddMarker( middle_sample, AnalyzerResults::Dot, mSettings->mInputChannel );
        }
    }

    return status;
}

void GameCubeControllerAnalyzer::DecodePacket( std::vector<Frame>& packet )
{
    if( !packet.size() )
        return;

    U64 cmd = packet[ 0 ].mData1;
    U64 len = packet.size();

    if( len == 1 )
    {
        if( cmd == 0x00 )
        {
            packet[ 0 ].mType = CMD_ID;
        }
        else if( cmd == 0x41 )
        {
            packet[ 0 ].mType = CMD_ORIGIN;
        }
    }
    else if( len == 3 )
    {
        if( cmd == 0x40 )
        {
            packet[ 0 ].mType = CMD_STATUS;
        }
        else if( cmd == 0x42 )
        {
            packet[ 0 ].mType = CMD_RECALIBRATE;
        }
        else if( cmd == 0x43 )
        {
            packet[ 0 ].mType = CMD_STATUS_LONG;
        }
    }
}

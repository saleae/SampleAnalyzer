#include "GameCubeControllerAnalyzer.h"

#include "GameCubeControllerAnalyzerSettings.h"

#include <AnalyzerChannelData.h>
#include <vector>

GameCubeControllerAnalyzer::GameCubeControllerAnalyzer()
    : Analyzer2(), mSettings( new GameCubeControllerAnalyzerSettings() ), mSimulationInitilized( false )
{
    SetAnalyzerSettings( mSettings.get() );
    UseFrameV2();
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

    AdvanceToEndOfPacket();

    while( true )
    {
        DecodeFrames();
        ReportProgress( mGamecube->GetSampleNumber() );
        CheckIfThreadShouldExit();
    }

    // while( true )
    // {
    //     bool idle = false;
    //     bool error = false;
    //     U64 error_frame_start_sample, error_frame_end_sample;
    //     U64 last_processed_sample;
    //     std::vector<Frame> packet;

    //     // construct packet
    //     while( !idle )
    //     {
    //         U64 frame_start_sample = mGamecube->GetSampleNumber();
    //         ByteDecodeStatus result = DecodeByte();
    //         U64 frame_end_sample = mGamecube->GetSampleNumber();

    //         Frame frame;
    //         frame.mStartingSampleInclusive = frame_start_sample;
    //         frame.mEndingSampleInclusive = frame_end_sample;
    //         frame.mData1 = result.byte;
    //         frame.mFlags = 0;
    //         frame.mType = DATA;

    //         if( result.error )
    //         {
    //             if( !error )
    //             {
    //                 error_frame_start_sample = frame_start_sample;
    //             }
    //             error = true;
    //         }
    //         if( result.idle )
    //         {
    //             if( error )
    //             {
    //                 error_frame_end_sample = frame_start_sample;
    //             }
    //             idle = true;
    //         }

    //         if( !idle && !error )
    //         {
    //             packet.push_back( frame );
    //         }
    //         else if( idle && error )
    //         {
    //             frame.mStartingSampleInclusive = error_frame_start_sample;
    //             frame.mEndingSampleInclusive = error_frame_end_sample;
    //             frame.mData1 = 0;
    //             frame.mFlags |= DISPLAY_AS_ERROR_FLAG;
    //             frame.mType = ERROR;
    //             packet.push_back( frame );
    //         }

    //         last_processed_sample = frame_end_sample;
    //     }

    //     // process packet
    //     if( !error )
    //     {
    //         DecodePacket( packet );
    //     }

    //     // commit
    //     for( Frame& frame : packet )
    //     {
    //         mResults->AddFrame( frame );
    //         mResults->CommitResults();
    //     }
    //     if( !error )
    //     {
    //         mResults->CommitPacketAndStartNewPacket();
    //     }
    //     else
    //     {
    //         mResults->CancelPacketAndStartNewPacket();
    //     }

    //     ReportProgress( last_processed_sample );
    //     CheckIfThreadShouldExit();
    // }
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
    return static_cast<U64>( ( end_edge - start_edge ) * 1e9 / mSampleRateHz );
}
#include <iostream>
#include <assert.h>

// advances to the rising edge at the end of a packet
void GameCubeControllerAnalyzer::AdvanceToEndOfPacket()
{
    if( mGamecube->GetBitState() == BIT_LOW )
    {
        mGamecube->AdvanceToNextEdge();
    }

    while( GetPulseWidthNs( mGamecube->GetSampleNumber(), mGamecube->GetSampleOfNextEdge() ) < 1e5 )
    {
        mGamecube->AdvanceToNextEdge();
        mGamecube->AdvanceToNextEdge();
    }
}

void GameCubeControllerAnalyzer::DecodeFrames()
{
    FrameV2 frame_v2;

    // traverse to the first falling edge
    mGamecube->AdvanceToNextEdge();

    U64 start_sample = mGamecube->GetSampleNumber();
    // try to decode the command
    U8 cmd, data;

    if( !DecodeByte( cmd ) )
    {
        AdvanceToEndOfPacket();
        return;
    }
    mGamecube->AdvanceToNextEdge();

    // TODO: support more commands, there is a list here: https://n64brew.dev/wiki/Joybus_Protocol
    switch( cmd )
    {
    case 0x00:
    {
        if( !DecodeStopBit() )
        {
            AdvanceToEndOfPacket();
            return;
        }
        mGamecube->AdvanceToNextEdge();

        // response
        uint16_t device = 0;
        if( !DecodeByte( data ) )
        {
            AdvanceToEndOfPacket();
            return;
        }
        mGamecube->AdvanceToNextEdge();
        device |= data;

        if( !DecodeByte( data ) )
        {
            AdvanceToEndOfPacket();
            return;
        }
        mGamecube->AdvanceToNextEdge();
        device |= data << 8;
        frame_v2.AddInteger( "device", device );

        if( !DecodeByte( data ) )
        {
            AdvanceToEndOfPacket();
            return;
        }
        mGamecube->AdvanceToNextEdge();
        frame_v2.AddByte( "status", data );

        DecodeStopBit();
        U64 end_sample = mGamecube->GetSampleNumber();
        mResults->AddFrameV2( frame_v2, "id", start_sample, end_sample );
        AdvanceToEndOfPacket();
    }
    break;

        // case 0x40:
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeStopBit();
        //     // response
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeStopBit();
        //     break;

        // case 0x41:
        //     DecodeStopBit();
        //     // response
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeStopBit();
        //     break;

        // case 0x42:
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeStopBit();
        //     // response
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeStopBit();
        //     break;

        // case 0x43:
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeStopBit();
        //     // response
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeByte();
        //     DecodeStopBit();
        //     break;

    default:
        AdvanceToEndOfPacket();
        break;
    }

    mResults->CommitResults();
}

// attempts to decode a byte. the current sample should be a falling edge and this
// function will return on a rising edge
bool GameCubeControllerAnalyzer::DecodeByte( U8& byte )
{
    byte = 0;
    for( U8 i = 0; i < 8; i++ )
    {
        bool bit;
        if( !DecodeDataBit( bit ) )
        {
            std::cout << "DecodeByte() failed" << std::endl;
            return false;
        }

        byte |= bit << ( 7 - i );

        if( i < 7 )
        {
            // advance to the next falling edge iff
            // - there are more bits to process in the current byte
            // - the last bit was successful
            mGamecube->AdvanceToNextEdge();
        }
    }

    return true;
}

// attempts to decode a single bit. on entry, the current sample should be a falling edge and this
// function will return on a rising edge
bool GameCubeControllerAnalyzer::DecodeDataBit( bool& bit )
{
    U64 starting_sample, ending_sample, rising_edge_sample, falling_edge_sample;

    // determine whether the bit is a 1 or 0 based on the duration of the low time
    starting_sample = falling_edge_sample = mGamecube->GetSampleNumber();
    mGamecube->AdvanceToNextEdge();
    rising_edge_sample = mGamecube->GetSampleNumber();

    U64 low_time = GetPulseWidthNs( falling_edge_sample, rising_edge_sample );

    if( low_time >= 5000 )
    {
        std::cout << "DecodeDataBit() failed" << std::endl;
        return false;
    }
    else
    {
        bit = low_time < 2000;

        // make sure the high time is reasonable. peek at the next falling edge, but don't
        // actually advance to it yet, in case something is wrong.
        ending_sample = falling_edge_sample = mGamecube->GetSampleOfNextEdge();
        U64 high_time = GetPulseWidthNs( rising_edge_sample, falling_edge_sample );

        if( high_time >= 5000 )
        {
            std::cout << "DecodeDataBit() failed" << std::endl;
            return false;
        }

        // add an indicator showing the bit value
        U64 middle_sample = ( starting_sample + ending_sample ) / 2;
        mResults->AddMarker( middle_sample, AnalyzerResults::Dot, mSettings->mInputChannel );
    }

    return true;
}

// attempt to detect a stop bit, which is a single "1" bit where the high time doesn't matter.
// on entry, the current sample should be a falling edge and this function will return on a rising
// edge
bool GameCubeControllerAnalyzer::DecodeStopBit()
{
    U64 falling_edge_sample = mGamecube->GetSampleNumber();
    mGamecube->AdvanceToNextEdge();
    U64 rising_edge_sample = mGamecube->GetSampleNumber();

    U64 low_time = GetPulseWidthNs( falling_edge_sample, rising_edge_sample );

    return low_time < 2000;
}

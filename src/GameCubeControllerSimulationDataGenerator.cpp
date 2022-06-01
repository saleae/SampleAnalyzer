#include "GameCubeControllerSimulationDataGenerator.h"

#include "GameCubeControllerAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

GameCubeControllerSimulationDataGenerator::GameCubeControllerSimulationDataGenerator()
{
    mGamecubeGenerationState = mGamecubeGenerationLastState = GamecubeGenerationState::IdCmd;
}

GameCubeControllerSimulationDataGenerator::~GameCubeControllerSimulationDataGenerator()
{
}

void GameCubeControllerSimulationDataGenerator::Initialize( U32 simulation_sample_rate, GameCubeControllerAnalyzerSettings* settings )
{
    mSimulationSampleRateHz = simulation_sample_rate;
    mSettings = settings;

    mGamecubeSimulationData.SetChannel( mSettings->mInputChannel );
    mGamecubeSimulationData.SetSampleRate( simulation_sample_rate );
    mGamecubeSimulationData.SetInitialBitState( BIT_HIGH );
}

U32 GameCubeControllerSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate,
                                                                       SimulationChannelDescriptor** simulation_channel )
{
    U64 adjusted_largest_sample_requested =
        AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

    while( mGamecubeSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
    {
        RunStateMachine();
    }

    *simulation_channel = &mGamecubeSimulationData;
    return 1;
}

U64 GameCubeControllerSimulationDataGenerator::NsToSamples( U64 ns )
{
    return mSimulationSampleRateHz * ns / 1e9;
}

void GameCubeControllerSimulationDataGenerator::GenerateOne()
{
    mGamecubeSimulationData.Transition();
    mGamecubeSimulationData.Advance( NsToSamples( 1250 ) );
    mGamecubeSimulationData.Transition();
    mGamecubeSimulationData.Advance( NsToSamples( 3750 ) );
}

void GameCubeControllerSimulationDataGenerator::GenerateZero()
{
    mGamecubeSimulationData.Transition();
    mGamecubeSimulationData.Advance( NsToSamples( 3750 ) );
    mGamecubeSimulationData.Transition();
    mGamecubeSimulationData.Advance( NsToSamples( 1250 ) );
}

void GameCubeControllerSimulationDataGenerator::GenerateByte( U8 byte )
{
    U8 mask = 1 << 7;
    for( int i = 0; i < 8; i++ )
    {
        if( byte & mask )
        {
            GenerateOne();
        }
        else
        {
            GenerateZero();
        }
        mask >>= 1;
    }
}

void GameCubeControllerSimulationDataGenerator::GenerateStopBit()
{
    GenerateOne();
}

void GameCubeControllerSimulationDataGenerator::GenerateDelayShort()
{
    mGamecubeSimulationData.Advance( NsToSamples( 50000 ) );
}
void GameCubeControllerSimulationDataGenerator::GenerateDelayLong()
{
    mGamecubeSimulationData.Advance( NsToSamples( 1000000 ) );
}

void GameCubeControllerSimulationDataGenerator::GenerateIdCmd()
{
    // cmd
    GenerateByte( 0x00 );
    GenerateStopBit();
}

void GameCubeControllerSimulationDataGenerator::GenerateIdResp()
{
    // controller info
    GenerateByte( 0x09 );
    GenerateByte( 0x00 );
    GenerateByte( 0x20 );
    GenerateStopBit();
}

void GameCubeControllerSimulationDataGenerator::GenerateOriginCmd()
{
    // cmd
    GenerateByte( 0x41 );
    GenerateStopBit();
}

void GameCubeControllerSimulationDataGenerator::GenerateOriginResp()
{
    // buttons0
    GenerateByte( 0x00 );
    // buttons1
    GenerateByte( 0x80 );
    // joystick x
    GenerateByte( 0x80 );
    // joystick y
    GenerateByte( 0x80 );
    // c stick x
    GenerateByte( 0x80 );
    // c stick y
    GenerateByte( 0x80 );
    // l analog
    GenerateByte( 0x20 );
    // r analog
    GenerateByte( 0x20 );
    // analog a
    GenerateByte( 0x02 );
    // analog b
    GenerateByte( 0x02 );
    GenerateStopBit();
}

void GameCubeControllerSimulationDataGenerator::GeneratePollCmd()
{
    // cmd
    GenerateByte( 0x40 );
    // args
    GenerateByte( 0x03 );
    GenerateByte( 0x00 );
    GenerateStopBit();
}

void GameCubeControllerSimulationDataGenerator::GeneratePollResp()
{
    // buttons0
    GenerateByte( 0x00 );
    // buttons1
    GenerateByte( 0x80 );
    // joystick x
    GenerateByte( 0x80 );
    // joystick y
    GenerateByte( 0x80 );
    // c stick x
    GenerateByte( 0x80 );
    // c stick y
    GenerateByte( 0x80 );
    // l analog
    GenerateByte( 0x20 );
    // r analog
    GenerateByte( 0x20 );
    GenerateStopBit();
}

void GameCubeControllerSimulationDataGenerator::RunStateMachine()
{
    switch( mGamecubeGenerationState )
    {
    case GamecubeGenerationState::IdCmd:
        GenerateIdCmd();
        mGamecubeGenerationLastState = GamecubeGenerationState::IdCmd;
        mGamecubeGenerationState = GamecubeGenerationState::DelayShort;
        break;
    case GamecubeGenerationState::IdResp:
        GenerateIdResp();
        mGamecubeGenerationLastState = GamecubeGenerationState::IdResp;
        mGamecubeGenerationState = GamecubeGenerationState::DelayLong;
        break;
    case GamecubeGenerationState::OriginCmd:
        GenerateOriginCmd();
        mGamecubeGenerationLastState = GamecubeGenerationState::OriginCmd;
        mGamecubeGenerationState = GamecubeGenerationState::DelayShort;
        break;
    case GamecubeGenerationState::OriginResp:
        GenerateOriginResp();
        mGamecubeGenerationLastState = GamecubeGenerationState::OriginResp;
        mGamecubeGenerationState = GamecubeGenerationState::DelayLong;
        break;
    case GamecubeGenerationState::PollCmd:
        GeneratePollCmd();
        mGamecubeGenerationLastState = GamecubeGenerationState::PollCmd;
        mGamecubeGenerationState = GamecubeGenerationState::DelayShort;
        break;
    case GamecubeGenerationState::PollResp:
        GeneratePollResp();
        mGamecubeGenerationLastState = GamecubeGenerationState::PollResp;
        mGamecubeGenerationState = GamecubeGenerationState::DelayLong;
        break;
    case GamecubeGenerationState::DelayShort:
        GenerateDelayShort();
        switch( mGamecubeGenerationLastState )
        {
        case GamecubeGenerationState::IdCmd:
            if( mIdCmds-- )
            {
                mGamecubeGenerationState = GamecubeGenerationState::DelayLong;
            }
            else
            {
                mGamecubeGenerationState = GamecubeGenerationState::IdResp;
                mIdCmds = ID_CMDS;
            }
            break;
        case GamecubeGenerationState::OriginCmd:
            mGamecubeGenerationState = GamecubeGenerationState::OriginResp;
            break;
        case GamecubeGenerationState::PollCmd:
            if( mPollCmds-- )
            {
                mGamecubeGenerationState = GamecubeGenerationState::PollResp;
            }
            else
            {
                mGamecubeGenerationState = GamecubeGenerationState::DelayLong;
                mPollCmds = POLL_CMDS;
            }
            break;
        default:
            break;
        }
        mGamecubeGenerationLastState = GamecubeGenerationState::DelayShort;
        break;
    case GamecubeGenerationState::DelayLong:
        GenerateDelayLong();
        switch( mGamecubeGenerationLastState )
        {
        case GamecubeGenerationState::DelayShort:
        case GamecubeGenerationState::DelayLong:
            mGamecubeGenerationState = GamecubeGenerationState::IdCmd;
            break;
        case GamecubeGenerationState::IdResp:
            mGamecubeGenerationState = GamecubeGenerationState::OriginCmd;
            break;
        case GamecubeGenerationState::OriginResp:
            mGamecubeGenerationState = GamecubeGenerationState::PollCmd;
            break;
        case GamecubeGenerationState::PollResp:
            mGamecubeGenerationState = GamecubeGenerationState::PollCmd;
            break;
        default:
            break;
        }
        mGamecubeGenerationLastState = GamecubeGenerationState::DelayLong;
        break;
    default:
        break;
    }
}

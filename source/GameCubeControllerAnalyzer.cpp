#include "GameCubeControllerAnalyzer.h"

#include "GameCubeControllerAnalyzerSettings.h"

#include <AnalyzerChannelData.h>

GameCubeControllerAnalyzer::GameCubeControllerAnalyzer() :
    Analyzer2(), mSettings(new GameCubeControllerAnalyzerSettings()), mSimulationInitilized(false) {
    SetAnalyzerSettings(mSettings.get());
}

GameCubeControllerAnalyzer::~GameCubeControllerAnalyzer() {
    KillThread();
}

void GameCubeControllerAnalyzer::SetupResults() {
    mResults.reset(new GameCubeControllerAnalyzerResults(this, mSettings.get()));
    SetAnalyzerResults(mResults.get());
    mResults->AddChannelBubblesWillAppearOn(mSettings->mInputChannel);
}

void GameCubeControllerAnalyzer::WorkerThread() {
    mSampleRateHz = GetSampleRate();

    mGamecube = GetAnalyzerChannelData(mSettings->mInputChannel);

    // start at the first falling edge
    if (mGamecube->GetBitState() == BIT_HIGH) mGamecube->AdvanceToNextEdge();

    for (;;) {
        ByteDecodeStatus result;

        while (!result.idle) {
            U64 frame_start_sample = mGamecube->GetSampleNumber();
            result = DecodeByte();
            U64 frame_end_sample = mGamecube->GetSampleNumber();

            Frame frame;
            frame.mStartingSampleInclusive = frame_start_sample;
            frame.mEndingSampleInclusive = frame_end_sample;
            if (!result.error) {
                frame.mData1 = result.byte;
                frame.mFlags = 0;
            } else {
                frame.mFlags |= DISPLAY_AS_ERROR_FLAG;
            }
            mResults->AddFrame(frame);
            mResults->CommitResults();
            ReportProgress(frame_end_sample);
            CheckIfThreadShouldExit();
        }
    }
}

bool GameCubeControllerAnalyzer::NeedsRerun() {
    return false;
}

U32 GameCubeControllerAnalyzer::GenerateSimulationData(
  U64 minimum_sample_index,
  U32 device_sample_rate,
  SimulationChannelDescriptor** simulation_channels) {
    if (mSimulationInitilized == false) {
        mSimulationDataGenerator.Initialize(GetSimulationSampleRate(), mSettings.get());
        mSimulationInitilized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData(
      minimum_sample_index, device_sample_rate, simulation_channels);
}

U32 GameCubeControllerAnalyzer::GetMinimumSampleRateHz() {
    return 2000000;
}

const char* GameCubeControllerAnalyzer::GetAnalyzerName() const {
    return "GameCube";
}

const char* GetAnalyzerName() {
    return "GameCube";
}

Analyzer* CreateAnalyzer() {
    return new GameCubeControllerAnalyzer();
}

void DestroyAnalyzer(Analyzer* analyzer) {
    delete analyzer;
}

U64 GameCubeControllerAnalyzer::GetPulseWidthNs(U64 start_edge, U64 end_edge) {
    return (end_edge - start_edge) * 1e9 / mSampleRateHz;
}

GameCubeControllerAnalyzer::ByteDecodeStatus GameCubeControllerAnalyzer::DecodeByte() {
    ByteDecodeStatus status;
    U64 falling_edge_sample, rising_edge_sample;

    for (U8 bit = 0; bit < 8; bit++) {
        // compute low time
        falling_edge_sample = mGamecube->GetSampleNumber();
        mGamecube->AdvanceToNextEdge();
        rising_edge_sample = mGamecube->GetSampleNumber();
        U64 low_time = GetPulseWidthNs(falling_edge_sample, rising_edge_sample);

        if (750 <= low_time && low_time <= 1500) {
            // detected a 1
            status.byte |= 1 << (7 - bit);
        } else if (2750 <= low_time && low_time <= 4000) {
            // detected a 0
        } else {
            // data is corrupt
            status.error = true;
            mGamecube->AdvanceToNextEdge();
            break;
        }

        // compute high time
        mGamecube->AdvanceToNextEdge();
        falling_edge_sample = mGamecube->GetSampleNumber();
        U64 high_time = GetPulseWidthNs(rising_edge_sample, falling_edge_sample);

        // the line is idle - packet complete
        if (high_time > 5250) {
            if (status.byte != 0x01 || bit > 0) {
                // not a stop bit
                status.error = true;
            }
            status.idle = true;
            break;
        }
    }

    return status;
}

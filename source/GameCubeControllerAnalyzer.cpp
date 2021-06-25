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
        U64 falling_edge_sample, rising_edge_sample;
        bool packet_done = false;
        bool error = false;

        while (!packet_done) {
            U8 byte = 0;
            U8 pos = 0;

            U64 frame_start_sample = mGamecube->GetSampleNumber();
            for (U8 i = 0; i < 8; i++) {
                // compute low time
                falling_edge_sample = mGamecube->GetSampleNumber();
                mGamecube->AdvanceToNextEdge();
                rising_edge_sample = mGamecube->GetSampleNumber();
                U64 low_time = GetPulseWidthUs(falling_edge_sample, rising_edge_sample);

                if (750000 <= low_time && low_time <= 1500000) {
                    // detected a 1
                    byte |= 1 << pos++;
                } else if (2750000 <= low_time && low_time <= 4000000) {
                    // detected a 0
                    pos++;
                } else {
                    // the data got corrupted, abandon the packet
                    packet_done = true;
                    error = true;
                    break;
                }

                // compute high time
                mGamecube->AdvanceToNextEdge();
                falling_edge_sample = mGamecube->GetSampleNumber();
                U64 high_time = GetPulseWidthUs(rising_edge_sample, falling_edge_sample);

                // the line is idle - packet complete
                if (high_time > 5250000) {
                    packet_done = true;
                    break;
                }
            }
            U64 frame_end_sample = mGamecube->GetSampleNumber();

            // if a full byte was completed, commit it
            if (!error && pos == 8) {
                Frame frame;
                frame.mData1 = byte;
                frame.mFlags = 0;
                frame.mStartingSampleInclusive = frame_start_sample;
                frame.mEndingSampleInclusive = frame_end_sample;
                mResults->AddFrame(frame);
                mResults->CommitResults();
            }
            ReportProgress(frame_end_sample);
            CheckIfThreadShouldExit();
        }
        if (!error) {
            // commit packet
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

U64 GameCubeControllerAnalyzer::GetPulseWidthUs(U64 start_edge, U64 end_edge) {
    return (end_edge - start_edge) * 1000000 / mSampleRateHz;
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

#ifndef GAMECUBECONTROLLER_ANALYZER_H
#define GAMECUBECONTROLLER_ANALYZER_H

#include "GameCubeControllerAnalyzerResults.h"
#include "GameCubeControllerSimulationDataGenerator.h"

#include <Analyzer.h>

class GameCubeControllerAnalyzerSettings;
class ANALYZER_EXPORT GameCubeControllerAnalyzer : public Analyzer2 {
  public:
    GameCubeControllerAnalyzer();
    virtual ~GameCubeControllerAnalyzer();

    virtual void SetupResults();
    virtual void WorkerThread();

    virtual U32 GenerateSimulationData(
      U64 newest_sample_requested,
      U32 sample_rate,
      SimulationChannelDescriptor** simulation_channels);
    virtual U32 GetMinimumSampleRateHz();

    virtual const char* GetAnalyzerName() const;
    virtual bool NeedsRerun();

  protected: // vars
    std::auto_ptr<GameCubeControllerAnalyzerSettings> mSettings;
    std::auto_ptr<GameCubeControllerAnalyzerResults> mResults;
    AnalyzerChannelData* mGamecube;

    GameCubeControllerSimulationDataGenerator mSimulationDataGenerator;
    bool mSimulationInitilized;

    // Serial analysis vars:
    U32 mSampleRateHz;
    U32 mStartOfStopBitOffset;
    U32 mEndOfStopBitOffset;

    U64 GetPulseWidthNs(U64 start_edge, U64 end_edge);
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer(Analyzer* analyzer);

#endif // GAMECUBECONTROLLER_ANALYZER_H

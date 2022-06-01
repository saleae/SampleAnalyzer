#ifndef GAMECUBECONTROLLER_ANALYZER_H
#define GAMECUBECONTROLLER_ANALYZER_H

#include "GameCubeControllerAnalyzerResults.h"
#include "GameCubeControllerSimulationDataGenerator.h"

#include <Analyzer.h>

class GameCubeControllerAnalyzerSettings;
class ANALYZER_EXPORT GameCubeControllerAnalyzer : public Analyzer2
{
  public:
    enum FrameType
    {
        DATA,
        CMD_ID,
        CMD_STATUS,
        CMD_ORIGIN,
        CMD_RECALIBRATE,
        CMD_STATUS_LONG,
        ERROR,
    };

    GameCubeControllerAnalyzer();
    virtual ~GameCubeControllerAnalyzer();

    virtual void SetupResults();
    virtual void WorkerThread();

    virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
    virtual U32 GetMinimumSampleRateHz();

    virtual const char* GetAnalyzerName() const;
    virtual bool NeedsRerun();

  protected: // vars
    struct ByteDecodeStatus
    {
        bool error = false;
        bool idle = false;
        U8 byte = 0x00;
    };

    std::auto_ptr<GameCubeControllerAnalyzerSettings> mSettings;
    std::auto_ptr<GameCubeControllerAnalyzerResults> mResults;
    AnalyzerChannelData* mGamecube;

    GameCubeControllerSimulationDataGenerator mSimulationDataGenerator;
    bool mSimulationInitilized;

    U32 mSampleRateHz;
    U32 mStartOfStopBitOffset;
    U32 mEndOfStopBitOffset;

    U64 GetPulseWidthNs( U64 start_edge, U64 end_edge );
    ByteDecodeStatus DecodeByte();
    void DecodePacket( std::vector<Frame>& packet );
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif // GAMECUBECONTROLLER_ANALYZER_H

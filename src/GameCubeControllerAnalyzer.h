#ifndef GAMECUBECONTROLLER_ANALYZER_H
#define GAMECUBECONTROLLER_ANALYZER_H

#include "GameCubeControllerAnalyzerResults.h"
#include "GameCubeControllerSimulationDataGenerator.h"

#include <Analyzer.h>

class GameCubeControllerAnalyzerSettings;
class ANALYZER_EXPORT GameCubeControllerAnalyzer : public Analyzer2
{
  public:
    enum JoyBusCommand
    {
        CMD_ID = 0x00,
        CMD_STATUS = 0x40,
        CMD_ORIGIN = 0x41,
        CMD_RECALIBRATE = 0x42,
        CMD_STATUS_LONG = 0x43,
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
    std::auto_ptr<GameCubeControllerAnalyzerSettings> mSettings;
    std::auto_ptr<GameCubeControllerAnalyzerResults> mResults;
    AnalyzerChannelData* mGamecube;

    GameCubeControllerSimulationDataGenerator mSimulationDataGenerator;
    bool mSimulationInitilized;

    U32 mSampleRateHz;

    U64 GetPulseWidthNs( U64 start_edge, U64 end_edge );
    void AdvanceToEndOfPacket();
    bool AdvanceToNextBitInPacket();
    void DecodeFrames();
    bool DecodeByte( U8& byte );
    bool DecodeDataBit( bool& bit );
    bool DecodeStopBit();
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif // GAMECUBECONTROLLER_ANALYZER_H

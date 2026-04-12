#ifndef ARINC429_ANALYZER_H
#define ARINC429_ANALYZER_H

#include <Analyzer.h>
#include "Arinc429AnalyzerSettings.h"
#include "Arinc429AnalyzerResults.h"
#include "Arinc429SimulationDataGenerator.h"
#include <memory>

class ANALYZER_EXPORT Arinc429Analyzer : public Analyzer2
{
public:
    Arinc429Analyzer();
    virtual ~Arinc429Analyzer();

    virtual void SetupResults();
    virtual void WorkerThread();

    virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate,
                                        SimulationChannelDescriptor** simulation_channels );
    virtual U32 GetMinimumSampleRateHz();

    virtual const char* GetAnalyzerName() const;
    virtual bool NeedsRerun();

protected:
    // Composite state of the differential pair at the current sample position.
    // HIGH  = A is high, B is low  (bit value 1 in RZ encoding first half)
    // LOW   = A is low,  B is high (bit value 0 in RZ encoding first half)
    // NULL_STATE = A == B           (quiescent / return-to-zero interval)
    enum CompositeState { HIGH_STATE, LOW_STATE, NULL_STATE };

    CompositeState GetComposite() const;

    // Advance both channel cursors to an absolute sample position.
    void AdvanceBothTo( U64 sample );

    // Advance both channels to the sample of the next edge on either channel.
    // After this call mCurrentSample reflects the new position.
    void AdvanceToNextCompositeTransition();

protected:
    Arinc429AnalyzerSettings mSettings;
    std::unique_ptr<Arinc429AnalyzerResults> mResults;

    AnalyzerChannelData* mChannelA;
    AnalyzerChannelData* mChannelB;

    U64 mCurrentSample;
    U32 mSampleRateHz;

    Arinc429SimulationDataGenerator mSimulationDataGenerator;
    bool mSimulationInitialized;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif // ARINC429_ANALYZER_H

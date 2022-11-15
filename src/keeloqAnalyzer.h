#ifndef KEELOQ_ANALYZER_H
#define KEELOQ_ANALYZER_H

#include <Analyzer.h>
#include "keeloqAnalyzerResults.h"
#include "keeloqSimulationDataGenerator.h"

class keeloqAnalyzerSettings;
class ANALYZER_EXPORT keeloqAnalyzer : public Analyzer2
{
public:
	keeloqAnalyzer();
	virtual ~keeloqAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	std::auto_ptr< keeloqAnalyzerSettings > mSettings;
	std::auto_ptr< keeloqAnalyzerResults > mResults;
	AnalyzerChannelData* mKeeloq;

	keeloqSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;
    std::vector<U32> mSampleW_preamb;
	//Serial analysis vars:
//	U32 mSampleRateHz;
//	U32 mStartOfStopBitOffset;
//	U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //KEELOQ_ANALYZER_H

#ifndef IEBUS_ANALYZER_H
#define IEBUS_ANALYZER_H

#include <Analyzer.h>
#include "IEBusAnalyzerResults.h"
#include "IEBusSimulationDataGenerator.h"

class IEBusAnalyzerSettings;
class ANALYZER_EXPORT IEBusAnalyzer : public Analyzer
{
public:
	IEBusAnalyzer();
	virtual ~IEBusAnalyzer();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	std::auto_ptr< IEBusAnalyzerSettings > mSettings;
	std::auto_ptr< IEBusAnalyzerResults > mResults;
	AnalyzerChannelData* mSerial;

	IEBusSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //IEBUS_ANALYZER_H

#ifndef SIMPLESERIAL_SIMULATION_DATA_GENERATOR
#define SIMPLESERIAL_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <string>
class SimpleSerialAnalyzerSettings;

class ANALYZER_EXPORT SimpleSerialSimulationDataGenerator
{
public:
	SimpleSerialSimulationDataGenerator();
	~SimpleSerialSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, SimpleSerialAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	SimpleSerialAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
	void CreateSerialByte();
#pragma warning( push )
#pragma warning( disable : 4251 ) // warning C4251: ... needs to have dll-interface to be used by clients of class
	std::string mSerialText;
#pragma warning( pop )
	U32 mStringIndex;

	SimulationChannelDescriptor mSerialSimulationData;

};
#endif //SIMPLESERIAL_SIMULATION_DATA_GENERATOR
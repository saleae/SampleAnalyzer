#ifndef IEBUS_SIMULATION_DATA_GENERATOR
#define IEBUS_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <string>
class IEBusAnalyzerSettings;

class IEBusSimulationDataGenerator
{
public:
	IEBusSimulationDataGenerator();
	~IEBusSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, IEBusAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	IEBusAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
	void CreateSerialByte();
	std::string mSerialText;
	U32 mStringIndex;

	SimulationChannelDescriptor mSerialSimulationData;

};
#endif //IEBUS_SIMULATION_DATA_GENERATOR
#ifndef KEELOQ_SIMULATION_DATA_GENERATOR
#define KEELOQ_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <string>
class keeloqAnalyzerSettings;

class keeloqSimulationDataGenerator
{
public:
	keeloqSimulationDataGenerator();
	~keeloqSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, keeloqAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	keeloqAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
	void CreateSerialByte();
	std::string mSerialText;
	U32 mStringIndex;

	SimulationChannelDescriptor mSerialSimulationData;

};
#endif //KEELOQ_SIMULATION_DATA_GENERATOR
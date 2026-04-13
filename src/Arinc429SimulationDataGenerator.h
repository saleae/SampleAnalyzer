#ifndef ARINC429_SIMULATION_DATA_GENERATOR_H
#define ARINC429_SIMULATION_DATA_GENERATOR_H

#include <SimulationChannelDescriptor.h>

class Arinc429AnalyzerSettings;

class Arinc429SimulationDataGenerator
{
public:
    Arinc429SimulationDataGenerator();
    ~Arinc429SimulationDataGenerator();

    void Initialize( U32 simulation_sample_rate, Arinc429AnalyzerSettings* settings );
    U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate,
                                SimulationChannelDescriptor** simulation_channels );

protected:
    // Emit one inter-word gap followed by one 32-bit ARINC 429 word.
    void EmitWord( U32 word );

    Arinc429AnalyzerSettings* mSettings;
    U32 mSimulationSampleRateHz;

    // Contiguous array so the SDK can index them as mChannels[0], mChannels[1].
    SimulationChannelDescriptor mChannels[ 2 ];

    // Rotating table of pre-built test words (valid + one parity-error word).
    static const int kNumWords = 6;
    U32 mWords[ kNumWords ];
    int mWordIndex;
};

#endif // ARINC429_SIMULATION_DATA_GENERATOR_H

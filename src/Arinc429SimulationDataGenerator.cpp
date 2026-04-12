#include "Arinc429SimulationDataGenerator.h"
#include "Arinc429AnalyzerSettings.h"
#include <AnalyzerHelpers.h>

Arinc429SimulationDataGenerator::Arinc429SimulationDataGenerator()
:   mSettings( nullptr ),
    mSimulationSampleRateHz( 0 )
{
}

Arinc429SimulationDataGenerator::~Arinc429SimulationDataGenerator()
{
}

void Arinc429SimulationDataGenerator::Initialize( U32 simulation_sample_rate,
                                                   Arinc429AnalyzerSettings* settings )
{
    mSimulationSampleRateHz = simulation_sample_rate;
    mSettings = settings;

    // Both channels start quiescent (both low → null composite state).
    mChannels[ 0 ].SetChannel( mSettings->mChannelA );
    mChannels[ 0 ].SetSampleRate( simulation_sample_rate );
    mChannels[ 0 ].SetInitialBitState( BIT_LOW );

    mChannels[ 1 ].SetChannel( mSettings->mChannelB );
    mChannels[ 1 ].SetSampleRate( simulation_sample_rate );
    mChannels[ 1 ].SetInitialBitState( BIT_LOW );
}

U32 Arinc429SimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested,
                                                              U32 sample_rate,
                                                              SimulationChannelDescriptor** simulation_channels )
{
    U64 adjusted = AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested,
                                                                   sample_rate,
                                                                   mSimulationSampleRateHz );

    // Advance both channels to the requested position (no transitions generated).
    for( int i = 0; i < 2; i++ )
    {
        U64 current = mChannels[ i ].GetCurrentSampleNumber();
        if( current < adjusted )
            mChannels[ i ].Advance( static_cast<U32>( adjusted - current ) );
    }

    // Return a pointer to the contiguous array of descriptors.
    *simulation_channels = mChannels;
    return 2;
}

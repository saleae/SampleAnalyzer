#include "Arinc429SimulationDataGenerator.h"
#include "Arinc429AnalyzerSettings.h"
#include <AnalyzerHelpers.h>

// ---------------------------------------------------------------------------
// Parity helpers (local to this TU)
// ---------------------------------------------------------------------------
static U32 SimPopcount32( U32 v )
{
    v = v - ( ( v >> 1 ) & 0x55555555U );
    v = ( v & 0x33333333U ) + ( ( v >> 2 ) & 0x33333333U );
    v = ( v + ( v >> 4 ) ) & 0x0F0F0F0FU;
    return ( v * 0x01010101U ) >> 24;
}

// Set bit 31 of word_body so the full 32-bit word has odd parity.
// word_body must have bit 31 == 0.
static U32 WithOddParity( U32 word_body )
{
    if( SimPopcount32( word_body ) % 2 == 0 )
        return word_body | ( 1U << 31 );
    return word_body;
}

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
Arinc429SimulationDataGenerator::Arinc429SimulationDataGenerator()
:   mSettings( nullptr ),
    mSimulationSampleRateHz( 0 ),
    mWordIndex( 0 )
{
    for( int i = 0; i < kNumWords; i++ )
        mWords[ i ] = 0;
}

Arinc429SimulationDataGenerator::~Arinc429SimulationDataGenerator()
{
}

// ---------------------------------------------------------------------------
// Initialize
// ---------------------------------------------------------------------------
void Arinc429SimulationDataGenerator::Initialize( U32 simulation_sample_rate,
                                                   Arinc429AnalyzerSettings* settings )
{
    mSimulationSampleRateHz = simulation_sample_rate;
    mSettings               = settings;
    mWordIndex              = 0;

    // Both channels start quiescent (both low → null composite state).
    mChannels[ 0 ].SetChannel( mSettings->mChannelA );
    mChannels[ 0 ].SetSampleRate( simulation_sample_rate );
    mChannels[ 0 ].SetInitialBitState( BIT_LOW );

    mChannels[ 1 ].SetChannel( mSettings->mChannelB );
    mChannels[ 1 ].SetSampleRate( simulation_sample_rate );
    mChannels[ 1 ].SetInitialBitState( BIT_LOW );

    // -----------------------------------------------------------------------
    // Build the test-word table.
    //
    // ARINC 429 word layout (32 bits, bit 0 = first on wire = ARINC bit 1):
    //   bits  0- 7 : label   (8 bits, octal labels stated below)
    //   bits  8- 9 : SDI     (2 bits)
    //   bits 10-28 : data    (19 bits)
    //   bits 29-30 : SSM     (2 bits)
    //   bit  31    : parity  (set by WithOddParity)
    //
    // Five valid words followed by one intentional parity-error word so both
    // decode paths (mType 0 and mType 1) are exercised in simulation.
    // -----------------------------------------------------------------------

    // Word 0 — label 0101 oct (0x41), SDI=00, data=0x000, SSM=00
    mWords[ 0 ] = WithOddParity( 0x41U );

    // Word 1 — label 0201 oct (0x81), SDI=01, data=0x1FF, SSM=01
    mWords[ 1 ] = WithOddParity( 0x81U | ( 1U << 8 ) | ( 0x1FFU << 10 ) | ( 1U << 29 ) );

    // Word 2 — label 0270 oct (0xB8), SDI=10, data=0x155AA, SSM=00
    mWords[ 2 ] = WithOddParity( 0xB8U | ( 2U << 8 ) | ( 0x155AAU << 10 ) );

    // Word 3 — label 0351 oct (0xE9), SDI=11, data=0x7FFFF, SSM=11  (all data bits set)
    mWords[ 3 ] = WithOddParity( 0xE9U | ( 3U << 8 ) | ( 0x7FFFFU << 10 ) | ( 3U << 29 ) );

    // Word 4 — label 0004 oct (0x04), SDI=00, data=0x0A5, SSM=00
    mWords[ 4 ] = WithOddParity( 0x04U | ( 0x0A5U << 10 ) );

    // Word 5 — intentional parity error: take word 0 and flip the parity bit.
    //          The decoder should flag this with mType = ARINC429_FRAME_TYPE_PARITY_ERROR.
    mWords[ 5 ] = mWords[ 0 ] ^ ( 1U << 31 );
}

// ---------------------------------------------------------------------------
// EmitWord — generate the bipolar RZ waveform for one 32-bit ARINC 429 word
//            preceded by a 10-bit-period inter-word null gap.
//
// ARINC 429 bipolar RZ bit encoding on the A/B differential pair:
//   bit = 1 → first half-period: A=HIGH, B=LOW  (HIGH composite state)
//             second half-period: A=LOW,  B=LOW  (NULL composite state)
//   bit = 0 → first half-period: A=LOW,  B=HIGH (LOW composite state)
//             second half-period: A=LOW,  B=LOW  (NULL composite state)
// ---------------------------------------------------------------------------
void Arinc429SimulationDataGenerator::EmitWord( U32 word )
{
    const U32 bit_period  = mSimulationSampleRateHz / mSettings->mBitRate;
    const U32 half_period = bit_period / 2;
    const U32 gap_samples = bit_period * 10; // 10 bit-periods — well above the 4T threshold

    // --- inter-word null gap ------------------------------------------------
    mChannels[ 0 ].TransitionIfNeeded( BIT_LOW );
    mChannels[ 1 ].TransitionIfNeeded( BIT_LOW );
    mChannels[ 0 ].Advance( gap_samples );
    mChannels[ 1 ].Advance( gap_samples );

    // --- 32 bits, transmitted from bit 0 (ARINC bit 1) to bit 31 (parity) --
    for( int i = 0; i < 32; i++ )
    {
        bool is_one = ( ( word >> i ) & 1U ) != 0;

        // First half-period: data pulse
        if( is_one )
        {
            mChannels[ 0 ].TransitionIfNeeded( BIT_HIGH ); // A=1
            mChannels[ 1 ].TransitionIfNeeded( BIT_LOW );  // B=0
        }
        else
        {
            mChannels[ 0 ].TransitionIfNeeded( BIT_LOW );  // A=0
            mChannels[ 1 ].TransitionIfNeeded( BIT_HIGH ); // B=1
        }
        mChannels[ 0 ].Advance( half_period );
        mChannels[ 1 ].Advance( half_period );

        // Second half-period: return to null (A=LOW, B=LOW)
        mChannels[ 0 ].TransitionIfNeeded( BIT_LOW );
        mChannels[ 1 ].TransitionIfNeeded( BIT_LOW );
        mChannels[ 0 ].Advance( half_period );
        mChannels[ 1 ].Advance( half_period );
    }
}

// ---------------------------------------------------------------------------
// GenerateSimulationData
// ---------------------------------------------------------------------------
U32 Arinc429SimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested,
                                                              U32 sample_rate,
                                                              SimulationChannelDescriptor** simulation_channels )
{
    U64 adjusted = AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested,
                                                                   sample_rate,
                                                                   mSimulationSampleRateHz );

    while( mChannels[ 0 ].GetCurrentSampleNumber() < adjusted )
    {
        EmitWord( mWords[ mWordIndex ] );
        mWordIndex = ( mWordIndex + 1 ) % kNumWords;
    }

    *simulation_channels = mChannels;
    return 2;
}

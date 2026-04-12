#include "Arinc429Analyzer.h"
#include "Arinc429AnalyzerSettings.h"
#include <AnalyzerChannelData.h>

// ---------------------------------------------------------------------------
// Helper: portable 32-bit popcount
// ---------------------------------------------------------------------------
static U32 Popcount32( U32 v )
{
    v = v - ( ( v >> 1 ) & 0x55555555U );
    v = ( v & 0x33333333U ) + ( ( v >> 2 ) & 0x33333333U );
    v = ( v + ( v >> 4 ) ) & 0x0F0F0F0FU;
    return ( v * 0x01010101U ) >> 24;
}

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
Arinc429Analyzer::Arinc429Analyzer()
:   Analyzer2(),
    mSettings(),
    mChannelA( nullptr ),
    mChannelB( nullptr ),
    mCurrentSample( 0 ),
    mSampleRateHz( 0 ),
    mSimulationInitialized( false )
{
    SetAnalyzerSettings( &mSettings );
}

Arinc429Analyzer::~Arinc429Analyzer()
{
    KillThread();
}

// ---------------------------------------------------------------------------
// SetupResults  — called at the start of each analysis run
// ---------------------------------------------------------------------------
void Arinc429Analyzer::SetupResults()
{
    mResults.reset( new Arinc429AnalyzerResults( this, &mSettings ) );
    SetAnalyzerResults( mResults.get() );
    // Bubbles appear on Channel A (the primary visual channel).
    mResults->AddChannelBubblesWillAppearOn( mSettings.mChannelA );
}

// ---------------------------------------------------------------------------
// Composite-state helpers
// ---------------------------------------------------------------------------
Arinc429Analyzer::CompositeState Arinc429Analyzer::GetComposite() const
{
    bool a = ( mChannelA->GetBitState() == BIT_HIGH );
    bool b = ( mChannelB->GetBitState() == BIT_HIGH );
    if( a == b )
        return NULL_STATE;
    return a ? HIGH_STATE : LOW_STATE;
}

void Arinc429Analyzer::AdvanceBothTo( U64 sample )
{
    mChannelA->AdvanceToAbsPosition( sample );
    mChannelB->AdvanceToAbsPosition( sample );
    mCurrentSample = sample;
}

void Arinc429Analyzer::AdvanceToNextCompositeTransition()
{
    // Look ahead on both channels without moving, then jump to whichever edge
    // comes first.  A transition on either A or B may change the composite
    // state, so we must move to the earlier one.
    U64 next_a = mChannelA->GetSampleOfNextEdge();
    U64 next_b = mChannelB->GetSampleOfNextEdge();
    U64 target = ( next_a < next_b ) ? next_a : next_b;
    AdvanceBothTo( target );
}

// ---------------------------------------------------------------------------
// WorkerThread  — main decode loop
// ---------------------------------------------------------------------------
void Arinc429Analyzer::WorkerThread()
{
    mSampleRateHz  = GetSampleRate();
    mChannelA      = GetAnalyzerChannelData( mSettings.mChannelA );
    mChannelB      = GetAnalyzerChannelData( mSettings.mChannelB );
    mCurrentSample = 0;

    // Pre-sync: start both cursors at position 0.
    AdvanceBothTo( 0 );

    const U64 samples_per_bit   = static_cast<U64>( mSampleRateHz ) / mSettings.mBitRate;
    const U64 null_gap_threshold = 4 * samples_per_bit;
    // Sample point within each bit: midpoint of the first (data) half-period.
    const U64 sample_offset     = samples_per_bit / 4;

    for( ; ; )
    {
        // ----------------------------------------------------------------
        // Phase 1 — hunt for a null gap of at least 4 bit periods
        // ----------------------------------------------------------------

        // Skip any non-null region.
        while( GetComposite() != NULL_STATE )
            AdvanceToNextCompositeTransition();

        U64 null_start = mCurrentSample;

        // Advance until the composite leaves the null state.
        while( GetComposite() == NULL_STATE )
            AdvanceToNextCompositeTransition();

        U64 null_end = mCurrentSample; // first sample of the active (non-null) bit

        if( ( null_end - null_start ) < null_gap_threshold )
            continue; // gap too short — not a valid inter-word gap; keep hunting

        // ----------------------------------------------------------------
        // Phase 2 — collect 32 bits
        // ----------------------------------------------------------------
        U64 word_start = null_end;
        U32 word       = 0;
        bool abort     = false;

        for( int i = 0; i < 32; i++ )
        {
            // Absolute position of the sample point for bit i.
            U64 sample_point = word_start + static_cast<U64>( i ) * samples_per_bit + sample_offset;
            AdvanceBothTo( sample_point );

            CompositeState state = GetComposite();

            if( state == NULL_STATE )
            {
                // Unexpected null before 32 bits — discard partial word.
                abort = true;
                break;
            }

            if( state == HIGH_STATE )
                word |= ( 1U << i ); // ARINC 429 transmits bit 1 first → maps to LSB
            // LOW_STATE contributes a 0 — no change needed
        }

        if( abort )
            continue;

        // Advance to the nominal end of the 32nd bit period.
        U64 word_end = word_start + 32 * samples_per_bit;
        AdvanceBothTo( word_end );

        // ----------------------------------------------------------------
        // Parity check — ARINC 429 uses odd parity over all 32 bits.
        // Bit 32 (index 31) is the parity bit and is included in word.
        // ----------------------------------------------------------------
        bool parity_ok = ( Popcount32( word ) % 2 ) == 1;

        Frame frame;
        frame.mStartingSampleInclusive = static_cast<S64>( word_start );
        frame.mEndingSampleInclusive   = static_cast<S64>( word_end - 1 );
        frame.mData1                   = word;
        frame.mData2                   = parity_ok ? 1 : 0;
        frame.mType                    = parity_ok ? ARINC429_FRAME_TYPE_VALID
                                                   : ARINC429_FRAME_TYPE_PARITY_ERROR;
        frame.mFlags                   = parity_ok ? 0 : DISPLAY_AS_ERROR_FLAG;

        mResults->AddFrame( frame );
        mResults->CommitResults();
        ReportProgress( frame.mEndingSampleInclusive );
    }
}

// ---------------------------------------------------------------------------
// Boilerplate
// ---------------------------------------------------------------------------
bool Arinc429Analyzer::NeedsRerun()
{
    return false;
}

U32 Arinc429Analyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate,
                                               SimulationChannelDescriptor** simulation_channels )
{
    if( !mSimulationInitialized )
    {
        mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), &mSettings );
        mSimulationInitialized = true;
    }
    return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index,
                                                            device_sample_rate,
                                                            simulation_channels );
}

U32 Arinc429Analyzer::GetMinimumSampleRateHz()
{
    return mSettings.mBitRate * 4;
}

const char* Arinc429Analyzer::GetAnalyzerName() const
{
    return "ARINC 429";
}

// ---------------------------------------------------------------------------
// C entry points required by the Saleae plugin loader
// ---------------------------------------------------------------------------
const char* GetAnalyzerName()
{
    return "ARINC 429";
}

Analyzer* CreateAnalyzer()
{
    return new Arinc429Analyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
    delete analyzer;
}

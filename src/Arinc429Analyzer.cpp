#include "Arinc429Analyzer.h"
#include "Arinc429AnalyzerSettings.h"
#include "Tb270ArincUtils.h"
#include <AnalyzerChannelData.h>
#include <cstdio>

namespace {

struct Tb270ColumnSpec
{
    const char* key;
    TB270::detail::Label label;
};

const Tb270ColumnSpec kTb270Columns[] = {
    { "BATTERY_DISCRETE_OUTPUTS", TB270::detail::Label::BATTERY_DISCRETE_OUTPUTS },
    { "DC_CURRENT", TB270::detail::Label::DC_CURRENT },
    { "DC_VOLTAGE", TB270::detail::Label::DC_VOLTAGE },
    { "BATTERY_TEMPERATURE", TB270::detail::Label::BATTERY_TEMPERATURE },
    { "DELIVERABLE_ENERGY", TB270::detail::Label::DELIVERABLE_ENERGY },
    { "CAPACITY", TB270::detail::Label::CAPACITY },
    { "STATE_OF_CHARGE", TB270::detail::Label::STATE_OF_CHARGE },
    { "ESTIMATED_LIFE_REMAINING", TB270::detail::Label::ESTIMATED_LIFE_REMAINING },
    { "SOFTWARE_VERSION", TB270::detail::Label::SOFTWARE_VERSION },
    { "OPERATIONAL_STATUS", TB270::detail::Label::OPERATIONAL_STATUS },
    { "FAULT_STATUS", TB270::detail::Label::FAULT_STATUS },
    { "CONFIGURATION_ID", TB270::detail::Label::CONFIGURATION_ID },
    { "EQUIPMENT_ID", TB270::detail::Label::EQUIPMENT_ID },
};

struct BitFieldColumnSpec
{
    const char* key;
    uint8_t bit_index;
};

const BitFieldColumnSpec kBatteryDiscreteColumns[] = {
    { "BATTERY_DISCRETE_OUTPUTS.OK", 0 },
    { "BATTERY_DISCRETE_OUTPUTS.ENG_START_AVAIL", 1 },
    { "BATTERY_DISCRETE_OUTPUTS.HEATING", 2 },
    { "BATTERY_DISCRETE_OUTPUTS.SOC_GOOD", 3 },
    { "BATTERY_DISCRETE_OUTPUTS.FAULT", 4 },
    { "BATTERY_DISCRETE_OUTPUTS.BATT_HTR_DISABLE", 5 },
    { "BATTERY_DISCRETE_OUTPUTS.SERVICE", 6 },
    { "BATTERY_DISCRETE_OUTPUTS.MIN_ENERGY_LOW", 7 },
};

const BitFieldColumnSpec kOperationalStatusColumns[] = {
    { "OPERATIONAL_STATUS.CHARGING", 0 },
    { "OPERATIONAL_STATUS.CHARGE_LIMIT", 1 },
    { "OPERATIONAL_STATUS.DISCHARGING", 2 },
    { "OPERATIONAL_STATUS.HTR_CYCLE_ACTIVE", 3 },
    { "OPERATIONAL_STATUS.MOD_DC_DISABLE", 4 },
    { "OPERATIONAL_STATUS.BATT_DC_DISABLE", 5 },
    { "OPERATIONAL_STATUS.MOD_CHG_DISABLE", 6 },
    { "OPERATIONAL_STATUS.BATT_CHG_DISABLE", 7 },
    { "OPERATIONAL_STATUS.CHG_LIMIT_SETPOINT", 8 },
};

const BitFieldColumnSpec kFaultStatusColumns[] = {
    { "FAULT_STATUS.INVALID_CONFIG", 0 },
    { "FAULT_STATUS.STACK_OVER_VOLTAGE", 1 },
    { "FAULT_STATUS.CELL_OVER_VOLTAGE", 2 },
};

void format_tb270_engineering_value( TB270::detail::Label label,
                                     const TB270::detail::WordFields& fields,
                                     char* buffer,
                                     size_t buffer_size )
{
    if( const TB270::detail::BnrDescriptor* descriptor = TB270::detail::descriptor_for_label( label ) )
    {
        const float value = TB270::detail::decode_bnr_data( *descriptor, fields.data );
        const char* units = "";

        switch( label )
        {
            case TB270::detail::Label::DC_CURRENT: units = " A"; break;
            case TB270::detail::Label::DC_VOLTAGE: units = " V"; break;
            case TB270::detail::Label::BATTERY_TEMPERATURE: units = " C"; break;
            case TB270::detail::Label::DELIVERABLE_ENERGY: units = " Ah"; break;
            case TB270::detail::Label::CAPACITY: units = " Ah"; break;
            case TB270::detail::Label::STATE_OF_CHARGE: units = " %"; break;
            case TB270::detail::Label::ESTIMATED_LIFE_REMAINING: units = " %"; break;
            default: break;
        }

        if( units[ 0 ] != '\0' )
            std::snprintf( buffer, buffer_size, "%.6g%s", value, units );
        else
            std::snprintf( buffer, buffer_size, "%.6g", value );
        return;
    }

    switch( label )
    {
        case TB270::detail::Label::BATTERY_DISCRETE_OUTPUTS:
        case TB270::detail::Label::OPERATIONAL_STATUS:
        case TB270::detail::Label::FAULT_STATUS:
            std::snprintf( buffer, buffer_size, "0x%05X", fields.data );
            return;

        case TB270::detail::Label::SOFTWARE_VERSION:
        {
            const uint8_t minor = static_cast<uint8_t>( fields.data & 0xFFu );
            const uint8_t major = static_cast<uint8_t>( ( fields.data >> 8 ) & 0xFFu );
            const uint8_t hardware = static_cast<uint8_t>( ( fields.data >> 16 ) & 0x7u );
            std::snprintf( buffer, buffer_size, "%u.%u.%u", hardware, major, minor );
            return;
        }

        case TB270::detail::Label::CONFIGURATION_ID:
            std::snprintf( buffer, buffer_size, "0x%04X", fields.data & 0xFFFFu );
            return;

        case TB270::detail::Label::EQUIPMENT_ID:
            std::snprintf( buffer, buffer_size, "0x%03X", fields.data & 0xFFFu );
            return;

        default:
            buffer[ 0 ] = '\0';
            return;
    }
}

void add_tb270_columns( FrameV2& frame_v2, uint32_t word )
{
    const TB270::detail::WordFields fields = TB270::detail::decode_word( word );
    const TB270::detail::Label label = static_cast<TB270::detail::Label>( fields.label );
    const uint32_t data = fields.data;

    for( const Tb270ColumnSpec& column : kTb270Columns )
    {
        if( column.label == label )
        {
            char value_str[ 64 ];
            format_tb270_engineering_value( label, fields, value_str, sizeof( value_str ) );
            frame_v2.AddString( column.key, value_str );
        }
        else
        {
            frame_v2.AddString( column.key, "" );
        }
    }

    auto add_bit_columns = [ &frame_v2, data, label ]( const BitFieldColumnSpec* columns, size_t count, TB270::detail::Label target_label ) {
        for( size_t i = 0; i < count; ++i )
        {
            if( label == target_label )
            {
                const bool bit_set = ( data & ( 1u << columns[ i ].bit_index ) ) != 0;
                frame_v2.AddString( columns[ i ].key, bit_set ? "1" : "0" );
            }
            else
            {
                frame_v2.AddString( columns[ i ].key, "" );
            }
        }
    };

    add_bit_columns( kBatteryDiscreteColumns, sizeof( kBatteryDiscreteColumns ) / sizeof( kBatteryDiscreteColumns[ 0 ] ),
                     TB270::detail::Label::BATTERY_DISCRETE_OUTPUTS );
    add_bit_columns( kOperationalStatusColumns, sizeof( kOperationalStatusColumns ) / sizeof( kOperationalStatusColumns[ 0 ] ),
                     TB270::detail::Label::OPERATIONAL_STATUS );
    add_bit_columns( kFaultStatusColumns, sizeof( kFaultStatusColumns ) / sizeof( kFaultStatusColumns[ 0 ] ),
                     TB270::detail::Label::FAULT_STATUS );
}

}  // namespace

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
    UseFrameV2();
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

        FrameV2 frame_v2;
        const uint8_t label = static_cast<uint8_t>( word & 0xFFu );
        const uint8_t sdi = static_cast<uint8_t>( ( word >> 8 ) & 0x3u );
        const uint32_t data = ( word >> 10 ) & 0x7FFFFu;
        const uint8_t ssm = static_cast<uint8_t>( ( word >> 29 ) & 0x3u );
        const uint8_t parity = static_cast<uint8_t>( ( word >> 31 ) & 0x1u );

        char label_str[ 8 ];
        std::snprintf( label_str, sizeof( label_str ), "%03o", label );

        char data_str[ 16 ];
        std::snprintf( data_str, sizeof( data_str ), "0x%05X", data );

        char word_str[ 16 ];
        std::snprintf( word_str, sizeof( word_str ), "0x%08X", word );

        char sdi_str[ 4 ];
        std::snprintf( sdi_str, sizeof( sdi_str ), "%u", sdi );

        char ssm_str[ 4 ];
        std::snprintf( ssm_str, sizeof( ssm_str ), "%u", ssm );

        char parity_str[ 4 ];
        std::snprintf( parity_str, sizeof( parity_str ), "%u", parity );

        frame_v2.AddString( "Label (oct)", label_str );
        frame_v2.AddString( "SDI", sdi_str );
        frame_v2.AddString( "Data (raw)", data_str );
        frame_v2.AddString( "Value", word_str );
        frame_v2.AddString( "SSM", ssm_str );
        frame_v2.AddString( "Parity", parity_str );
        add_tb270_columns( frame_v2, word );

        mResults->AddFrameV2( frame_v2,
                              parity_ok ? "word" : "word_parity_error",
                              frame.mStartingSampleInclusive,
                              frame.mEndingSampleInclusive );
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

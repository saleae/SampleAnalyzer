#include "Tb270ArincUtils.h"

#include <cstdio>

namespace {

uint32_t mask_n_bits(uint8_t n)
{
    if( n >= 32 )
        return 0xFFFFFFFFu;
    return ( 1u << n ) - 1u;
}

uint8_t reverse8( uint8_t value )
{
    value = static_cast<uint8_t>( ( value & 0xF0u ) >> 4 ) | static_cast<uint8_t>( ( value & 0x0Fu ) << 4 );
    value = static_cast<uint8_t>( ( value & 0xCCu ) >> 2 ) | static_cast<uint8_t>( ( value & 0x33u ) << 2 );
    value = static_cast<uint8_t>( ( value & 0xAAu ) >> 1 ) | static_cast<uint8_t>( ( value & 0x55u ) << 1 );
    return value;
}

int32_t sign_extend_twos( uint32_t value, uint8_t width )
{
    if( width == 0 || width >= 32 )
        return static_cast<int32_t>( value );

    const uint32_t sign_bit = 1u << ( width - 1u );
    const uint32_t value_mask = mask_n_bits( width );
    value &= value_mask;
    return ( value & sign_bit ) ? static_cast<int32_t>( value | ~value_mask )
                                : static_cast<int32_t>( value );
}

float decode_bnr( const TB270::detail::BnrDescriptor& d, uint32_t bnr_representation )
{
    const uint8_t n = static_cast<uint8_t>( d.sig_bits + ( d.is_signed ? 1u : 0u ) );
    if( d.is_signed )
        return static_cast<float>( sign_extend_twos( bnr_representation, n ) * d.resolution );

    return static_cast<float>( ( bnr_representation & mask_n_bits( n ) ) * d.resolution );
}

uint32_t encode_bnr( const TB270::detail::BnrDescriptor& d, float input )
{
    const uint8_t n = static_cast<uint8_t>( d.sig_bits + ( d.is_signed ? 1u : 0u ) );
    const uint32_t sig_bits_mask = mask_n_bits( n );

    int32_t counts = static_cast<int32_t>( input / d.resolution );

    // clamp to prevent wrap-around
    if( d.is_signed )
    {
        const int32_t min_counts = -( 1 << ( n - 1u ) );
        const int32_t max_counts = ( 1 << ( n - 1u ) ) - 1;
        if( counts < min_counts )
            counts = min_counts;
        else if( counts > max_counts )
            counts = max_counts;
    }
    else
    {
        const int32_t min_counts = 0;
        const int32_t max_counts = static_cast<int32_t>( ( 1u << n ) - 1u );
        if( counts < min_counts )
            counts = min_counts;
        else if( counts > max_counts )
            counts = max_counts;
    }

    return static_cast<uint32_t>( counts ) & sig_bits_mask;
}

const char* label_name_from_raw( uint8_t label )
{
    switch( static_cast<TB270::detail::Label>( label ) )
    {
        case TB270::detail::Label::NONE: return "NONE";
        case TB270::detail::Label::BATTERY_DISCRETE_OUTPUTS: return "BATTERY_DISCRETE_OUTPUTS";
        case TB270::detail::Label::DC_CURRENT: return "DC_CURRENT";
        case TB270::detail::Label::DC_VOLTAGE: return "DC_VOLTAGE";
        case TB270::detail::Label::BATTERY_TEMPERATURE: return "BATTERY_TEMPERATURE";
        case TB270::detail::Label::DELIVERABLE_ENERGY: return "DELIVERABLE_ENERGY";
        case TB270::detail::Label::CAPACITY: return "CAPACITY";
        case TB270::detail::Label::STATE_OF_CHARGE: return "STATE_OF_CHARGE";
        case TB270::detail::Label::ESTIMATED_LIFE_REMAINING: return "ESTIMATED_LIFE_REMAINING";
        case TB270::detail::Label::SOFTWARE_VERSION: return "SOFTWARE_VERSION";
        case TB270::detail::Label::OPERATIONAL_STATUS: return "OPERATIONAL_STATUS";
        case TB270::detail::Label::FAULT_STATUS: return "FAULT_STATUS";
        case TB270::detail::Label::CONFIGURATION_ID: return "CONFIGURATION_ID";
        case TB270::detail::Label::EQUIPMENT_ID: return "EQUIPMENT_ID";
        default: return "UNKNOWN";
    }
}

}  // namespace

namespace TB270 {
namespace detail {

constexpr BnrDescriptor BnrDescriptors::dc_current;
constexpr BnrDescriptor BnrDescriptors::dc_voltage;
constexpr BnrDescriptor BnrDescriptors::battery_temperature;
constexpr BnrDescriptor BnrDescriptors::deliverable_energy;
constexpr BnrDescriptor BnrDescriptors::capacity;
constexpr BnrDescriptor BnrDescriptors::state_of_charge;
constexpr BnrDescriptor BnrDescriptors::estimated_life_remaining;
constexpr BnrDescriptor BnrDescriptors::configuration_id;

WordFields decode_word( uint32_t word )
{
    WordFields fields{};
    fields.label = reverse8( static_cast<uint8_t>( word & 0xFFu ) );
    fields.sdi = static_cast<uint8_t>( ( word >> 8 ) & 0x3u );
    fields.data = ( word >> 10 ) & 0x7FFFFu;
    fields.ssm = static_cast<uint8_t>( ( word >> 29 ) & 0x3u );
    fields.parity = static_cast<uint8_t>( ( word >> 31 ) & 0x1u );
    return fields;
}

uint8_t reverse_label_bits( uint8_t label )
{
    return reverse8( label );
}

float decode_bnr_data( const BnrDescriptor& d, uint32_t data_field )
{
    const uint8_t padding = ( d.sig_bits >= 19u ) ? 0u : static_cast<uint8_t>( 19u - ( static_cast<uint8_t>( d.sig_bits ) + 1u ) );
    return decode_bnr( d, data_field >> padding );
}

uint32_t encode_bnr_data( const BnrDescriptor& d, float data_field )
{
    const uint8_t padding = ( d.sig_bits >= 19u ) ? 0u : static_cast<uint8_t>( 19u - ( static_cast<uint8_t>( d.sig_bits ) + 1u ) );
    return encode_bnr( d, data_field ) << padding;
}

const char* label_name( Label label )
{
    return label_name_from_raw( static_cast<uint8_t>( label ) );
}

const BnrDescriptor* descriptor_for_label( Label label )
{
    switch( label )
    {
        case Label::DC_CURRENT: return &BnrDescriptors::dc_current;
        case Label::DC_VOLTAGE: return &BnrDescriptors::dc_voltage;
        case Label::BATTERY_TEMPERATURE: return &BnrDescriptors::battery_temperature;
        case Label::DELIVERABLE_ENERGY: return &BnrDescriptors::deliverable_energy;
        case Label::CAPACITY: return &BnrDescriptors::capacity;
        case Label::STATE_OF_CHARGE: return &BnrDescriptors::state_of_charge;
        case Label::ESTIMATED_LIFE_REMAINING: return &BnrDescriptors::estimated_life_remaining;
        default: return nullptr;
    }
}

void format_word_summary( uint32_t word, char* buffer, size_t buffer_size )
{
    const WordFields fields = decode_word( word );
    const Label label = static_cast<Label>( fields.label );
    const char* label_str = label_name( label );

    if( const BnrDescriptor* descriptor = descriptor_for_label( label ) )
    {
        float value = decode_bnr_data( *descriptor, fields.data );
        const char* units = "";

        switch( label )
        {
            case Label::DC_CURRENT: units = "A"; break;
            case Label::DC_VOLTAGE: units = "V"; break;
            case Label::BATTERY_TEMPERATURE: units = "C"; break;
            case Label::DELIVERABLE_ENERGY: units = "Ah"; break;
            case Label::CAPACITY: units = "Ah"; break;
            case Label::STATE_OF_CHARGE: units = "%"; break;
            case Label::ESTIMATED_LIFE_REMAINING: units = "%"; break;
            default: break;
        }

        if( units[ 0 ] != '\0' )
            std::snprintf( buffer, buffer_size, "%03o %s %.6g %s", fields.label, label_str, value, units );
        else
            std::snprintf( buffer, buffer_size, "%03o %s %.6g", fields.label, label_str, value );
        return;
    }

    switch( label )
    {
        case Label::BATTERY_DISCRETE_OUTPUTS:
        case Label::OPERATIONAL_STATUS:
        case Label::FAULT_STATUS:
            std::snprintf( buffer, buffer_size, "%03o %s 0x%05X", fields.label, label_str, fields.data );
            return;

        case Label::SOFTWARE_VERSION:
        {
            const uint8_t minor = static_cast<uint8_t>( fields.data & 0xFFu );
            const uint8_t major = static_cast<uint8_t>( ( fields.data >> 8 ) & 0xFFu );
            const uint8_t hardware = static_cast<uint8_t>( ( fields.data >> 16 ) & 0x7u );
            std::snprintf( buffer, buffer_size, "%03o %s %u.%u.%u", fields.label, label_str, hardware, major, minor );
            return;
        }

        case Label::CONFIGURATION_ID:
            std::snprintf( buffer, buffer_size, "%03o %s 0x%04X", fields.label, label_str, fields.data & 0xFFFFu );
            return;

        case Label::EQUIPMENT_ID:
            std::snprintf( buffer, buffer_size, "%03o %s 0x%03X", fields.label, label_str, fields.data & 0xFFFu );
            return;

        default:
            std::snprintf( buffer, buffer_size, "%03o %s 0x%05X", fields.label, label_str, fields.data );
            return;
    }
}

}  // namespace detail
}  // namespace TB270

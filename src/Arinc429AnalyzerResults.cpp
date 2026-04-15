#include "Arinc429AnalyzerResults.h"
#include "Arinc429Analyzer.h"
#include "Arinc429AnalyzerSettings.h"
#include "Tb270ArincUtils.h"
#include <AnalyzerHelpers.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace {

void format_raw_word_fields( uint32_t word, char* buffer, size_t buffer_size )
{
    const uint8_t label = static_cast<uint8_t>( word & 0xFFu );
    const uint8_t sdi = static_cast<uint8_t>( ( word >> 8 ) & 0x3u );
    const uint32_t data = ( word >> 10 ) & 0x7FFFFu;
    const uint8_t ssm = static_cast<uint8_t>( ( word >> 29 ) & 0x3u );
    const uint8_t parity = static_cast<uint8_t>( ( word >> 31 ) & 0x1u );

    std::snprintf(
        buffer,
        buffer_size,
        "%03o | %u | 0x%05X | 0x%08X | %u | %u",
        label,
        sdi,
        data,
        word,
        ssm,
        parity );
}

void format_tb270_bubble_text( uint32_t word, char* buffer, size_t buffer_size )
{
    const TB270::detail::WordFields fields = TB270::detail::decode_word( word );
    const TB270::detail::Label label = static_cast<TB270::detail::Label>( fields.label );
    const char* label_name = TB270::detail::label_name( label );

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
            std::snprintf( buffer, buffer_size, "%s: %.6g%s", label_name, value, units );
        else
            std::snprintf( buffer, buffer_size, "%s: %.6g", label_name, value );
        return;
    }

    switch( label )
    {
        case TB270::detail::Label::BATTERY_DISCRETE_OUTPUTS:
        case TB270::detail::Label::OPERATIONAL_STATUS:
        case TB270::detail::Label::FAULT_STATUS:
            std::snprintf( buffer, buffer_size, "%s: 0x%05X", label_name, fields.data );
            return;

        case TB270::detail::Label::SOFTWARE_VERSION:
        {
            const uint8_t minor = static_cast<uint8_t>( fields.data & 0xFFu );
            const uint8_t major = static_cast<uint8_t>( ( fields.data >> 8 ) & 0xFFu );
            const uint8_t hardware = static_cast<uint8_t>( ( fields.data >> 16 ) & 0x7u );
            std::snprintf( buffer, buffer_size, "%s: %u.%u.%u", label_name, hardware, major, minor );
            return;
        }

        case TB270::detail::Label::CONFIGURATION_ID:
            std::snprintf( buffer, buffer_size, "%s: 0x%04X", label_name, fields.data & 0xFFFFu );
            return;

        case TB270::detail::Label::EQUIPMENT_ID:
            std::snprintf( buffer, buffer_size, "%s: 0x%03X", label_name, fields.data & 0xFFFu );
            return;

        default:
            std::snprintf( buffer, buffer_size, "%03o: 0x%05X", fields.label, fields.data );
            return;
    }
}

void format_duration_seconds( const Frame& frame, U32 sample_rate, char* buffer, size_t buffer_size )
{
    const U64 duration_samples = static_cast<U64>( frame.mEndingSampleInclusive - frame.mStartingSampleInclusive + 1 );
    const double duration_seconds = static_cast<double>( duration_samples ) / static_cast<double>( sample_rate );
    std::snprintf( buffer, buffer_size, "%.6f", duration_seconds );
}

}  // namespace

Arinc429AnalyzerResults::Arinc429AnalyzerResults( Arinc429Analyzer* analyzer, Arinc429AnalyzerSettings* settings )
:   AnalyzerResults(),
    mSettings( settings ),
    mAnalyzer( analyzer )
{
}

Arinc429AnalyzerResults::~Arinc429AnalyzerResults()
{
}

void Arinc429AnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& /*channel*/, DisplayBase /*display_base*/ )
{
    ClearResultStrings();
    Frame frame = GetFrame( frame_index );

    char hex_str[ 32 ];
    AnalyzerHelpers::GetNumberString( frame.mData1, Hexadecimal, 32, hex_str, sizeof( hex_str ) );

    char bubble_str[ 128 ];
    format_tb270_bubble_text( static_cast<uint32_t>( frame.mData1 ), bubble_str, sizeof( bubble_str ) );

    AddResultString( hex_str );
    AddResultString( bubble_str );
}

void Arinc429AnalyzerResults::GenerateExportFile( const char* file, DisplayBase /*display_base*/, U32 /*export_type_user_id*/ )
{
    std::ofstream file_stream( file, std::ios::out );

    U64 trigger_sample = mAnalyzer->GetTriggerSample();
    U32 sample_rate    = mAnalyzer->GetSampleRate();

    file_stream << "Time [s],Delta [s],Label (oct),SDI,Data (raw),Value,SSM,Parity" << std::endl;

    U64 num_frames = GetNumFrames();
    for( U64 i = 0; i < num_frames; i++ )
    {
        Frame frame = GetFrame( i );

        char time_str[ 128 ];
        AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, sizeof( time_str ) );

        char delta_str[ 32 ];
        format_duration_seconds( frame, sample_rate, delta_str, sizeof( delta_str ) );

        const uint8_t label = static_cast<uint8_t>( frame.mData1 & 0xFFu );
        const uint8_t sdi = static_cast<uint8_t>( ( frame.mData1 >> 8 ) & 0x3u );
        const uint32_t data = static_cast<uint32_t>( ( frame.mData1 >> 10 ) & 0x7FFFFu );
        const uint8_t ssm = static_cast<uint8_t>( ( frame.mData1 >> 29 ) & 0x3u );
        const uint8_t parity = static_cast<uint8_t>( ( frame.mData1 >> 31 ) & 0x1u );

        char value_str[ 32 ];
        AnalyzerHelpers::GetNumberString( frame.mData1, Hexadecimal, 32, value_str, sizeof( value_str ) );

        char label_str[ 8 ];
        std::snprintf( label_str, sizeof( label_str ), "%03o", label );

        char data_str[ 16 ];
        std::snprintf( data_str, sizeof( data_str ), "0x%05X", data );

        file_stream << time_str << ","
                    << delta_str << ","
                    << label_str << ","
                    << static_cast<unsigned>( sdi ) << ","
                    << data_str << ","
                    << value_str << ","
                    << static_cast<unsigned>( ssm ) << ","
                    << static_cast<unsigned>( parity ) << std::endl;

        if( UpdateExportProgressAndCheckForCancel( i, num_frames ) )
        {
            file_stream.close();
            return;
        }
    }

    file_stream.close();
}

void Arinc429AnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase /*display_base*/ )
{
#ifdef SUPPORTS_PROTOCOL_SEARCH
    Frame frame = GetFrame( frame_index );
    ClearTabularText();

    char raw_fields_str[ 128 ];
    format_raw_word_fields( static_cast<uint32_t>( frame.mData1 ), raw_fields_str, sizeof( raw_fields_str ) );

    AddTabularText( raw_fields_str );
#endif
}

void Arinc429AnalyzerResults::GeneratePacketTabularText( U64 /*packet_id*/, DisplayBase /*display_base*/ )
{
    // not supported
}

void Arinc429AnalyzerResults::GenerateTransactionTabularText( U64 /*transaction_id*/, DisplayBase /*display_base*/ )
{
    // not supported
}

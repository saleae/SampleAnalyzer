#include "Arinc429AnalyzerResults.h"
#include "Arinc429Analyzer.h"
#include "Arinc429AnalyzerSettings.h"
#include <AnalyzerHelpers.h>
#include <fstream>
#include <sstream>

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
    // Always show the 32-bit word in hex regardless of the display_base setting —
    // the prompt asks for hex on the waveform bubble.
    AnalyzerHelpers::GetNumberString( frame.mData1, Hexadecimal, 32, hex_str, sizeof( hex_str ) );

    if( frame.mType == ARINC429_FRAME_TYPE_PARITY_ERROR )
    {
        char result_str[ 64 ];
        // Short form
        AddResultString( hex_str );
        // Long form with error annotation
        snprintf( result_str, sizeof( result_str ), "%s (parity error)", hex_str );
        AddResultString( result_str );
    }
    else
    {
        AddResultString( hex_str );
    }
}

void Arinc429AnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 /*export_type_user_id*/ )
{
    std::ofstream file_stream( file, std::ios::out );

    U64 trigger_sample = mAnalyzer->GetTriggerSample();
    U32 sample_rate    = mAnalyzer->GetSampleRate();

    file_stream << "Time [s],Value,ParityOK" << std::endl;

    U64 num_frames = GetNumFrames();
    for( U64 i = 0; i < num_frames; i++ )
    {
        Frame frame = GetFrame( i );

        char time_str[ 128 ];
        AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, sizeof( time_str ) );

        char number_str[ 32 ];
        AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 32, number_str, sizeof( number_str ) );

        file_stream << time_str << "," << number_str << "," << frame.mData2 << std::endl;

        if( UpdateExportProgressAndCheckForCancel( i, num_frames ) )
        {
            file_stream.close();
            return;
        }
    }

    file_stream.close();
}

void Arinc429AnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
#ifdef SUPPORTS_PROTOCOL_SEARCH
    Frame frame = GetFrame( frame_index );
    ClearTabularText();

    char number_str[ 32 ];
    AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 32, number_str, sizeof( number_str ) );

    if( frame.mType == ARINC429_FRAME_TYPE_PARITY_ERROR )
    {
        char result_str[ 64 ];
        snprintf( result_str, sizeof( result_str ), "%s (parity error)", number_str );
        AddTabularText( result_str );
    }
    else
    {
        AddTabularText( number_str );
    }
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

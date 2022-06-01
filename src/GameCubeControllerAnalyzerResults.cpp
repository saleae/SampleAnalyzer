#include "GameCubeControllerAnalyzerResults.h"

#include "GameCubeControllerAnalyzer.h"
#include "GameCubeControllerAnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <fstream>
#include <iostream>

GameCubeControllerAnalyzerResults::GameCubeControllerAnalyzerResults( GameCubeControllerAnalyzer* analyzer,
                                                                      GameCubeControllerAnalyzerSettings* settings )
    : AnalyzerResults(), mSettings( settings ), mAnalyzer( analyzer )
{
}

GameCubeControllerAnalyzerResults::~GameCubeControllerAnalyzerResults()
{
}

void GameCubeControllerAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
    ClearResultStrings();
    Frame frame = GetFrame( frame_index );

    bool error = frame.mFlags & DISPLAY_AS_ERROR_FLAG;

    if( !error )
    {
        char number_str[ 128 ];
        AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );

        switch( frame.mType )
        {
        case GameCubeControllerAnalyzer::CMD_ID:
            AddResultString( "ID: ", number_str );
            break;

        case GameCubeControllerAnalyzer::CMD_STATUS:
            AddResultString( "Status: ", number_str );
            break;

        case GameCubeControllerAnalyzer::CMD_ORIGIN:
            AddResultString( "Origin: ", number_str );
            break;

        case GameCubeControllerAnalyzer::CMD_RECALIBRATE:
            AddResultString( "Recalibrate: ", number_str );
            break;

        case GameCubeControllerAnalyzer::CMD_STATUS_LONG:
            AddResultString( "Status Long: ", number_str );
            break;

        default:
            AddResultString( number_str );
            break;
        }
    }
    else
    {
        // This is not really worth enabling unless you can change the color
        AddResultString( "ERROR: Invalid frame" );
    }
}

void GameCubeControllerAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
    std::ofstream file_stream( file, std::ios::out );

    U64 trigger_sample = mAnalyzer->GetTriggerSample();
    U32 sample_rate = mAnalyzer->GetSampleRate();

    file_stream << "Time [s], Value [0, 1]" << std::endl;

    U64 num_frames = GetNumFrames();
    for( U32 i = 0; i < num_frames; i++ )
    {
        Frame frame = GetFrame( i );

        char time_str[ 128 ];
        AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

        char number_str[ 128 ];
        AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );

        file_stream << time_str << "," << number_str << std::endl;

        if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
        {
            file_stream.close();
            return;
        }
    }

    file_stream.close();
}

void GameCubeControllerAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
#ifdef SUPPORTS_PROTOCOL_SEARCH
    Frame frame = GetFrame( frame_index );
    ClearTabularText();

    char number_str[ 128 ];
    AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
    AddTabularText( number_str );
#endif
}

void GameCubeControllerAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
    // not supported
}

void GameCubeControllerAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
    // not supported
}
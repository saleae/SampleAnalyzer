#include "GameCubeControllerAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

GameCubeControllerAnalyzerSettings::GameCubeControllerAnalyzerSettings() : mInputChannel( UNDEFINED_CHANNEL )
{
    mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mInputChannelInterface->SetTitleAndTooltip( "Data", "GameCube controller data line" );
    mInputChannelInterface->SetChannel( mInputChannel );

    AddInterface( mInputChannelInterface.get() );

    AddExportOption( 0, "Export as text/csv file" );
    AddExportExtension( 0, "text", "txt" );
    AddExportExtension( 0, "csv", "csv" );

    ClearChannels();
    AddChannel( mInputChannel, "Serial", false );
}

GameCubeControllerAnalyzerSettings::~GameCubeControllerAnalyzerSettings()
{
}

bool GameCubeControllerAnalyzerSettings::SetSettingsFromInterfaces()
{
    mInputChannel = mInputChannelInterface->GetChannel();

    ClearChannels();
    AddChannel( mInputChannel, "GameCube", true );

    return true;
}

void GameCubeControllerAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mInputChannelInterface->SetChannel( mInputChannel );
}

void GameCubeControllerAnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    text_archive >> mInputChannel;

    ClearChannels();
    AddChannel( mInputChannel, "GameCube", true );

    UpdateInterfacesFromSettings();
}

const char* GameCubeControllerAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << mInputChannel;

    return SetReturnString( text_archive.GetString() );
}

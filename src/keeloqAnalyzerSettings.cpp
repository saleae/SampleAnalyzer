#include "keeloqAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


keeloqAnalyzerSettings::keeloqAnalyzerSettings()
:	mInputChannel( UNDEFINED_CHANNEL ),
	mBitRate( 9600 )
{
	mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputChannelInterface->SetTitleAndTooltip( "Keeloq", "KEELOQ Code input" );
	mInputChannelInterface->SetChannel( mInputChannel );

	AddInterface( mInputChannelInterface.get() );

	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mInputChannel, "Keeloq", false );
}

keeloqAnalyzerSettings::~keeloqAnalyzerSettings()
{
}

bool keeloqAnalyzerSettings::SetSettingsFromInterfaces()
{
	mInputChannel = mInputChannelInterface->GetChannel();

	ClearChannels();
	AddChannel( mInputChannel, "KEELOQ Code Hopping Encoder", true );

	return true;
}

void keeloqAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface->SetChannel( mInputChannel );
}

void keeloqAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	text_archive >> mInputChannel;
	text_archive >> mBitRate;

	ClearChannels();
	AddChannel( mInputChannel, "KEELOQ Code Hopping Encoder", true );

	UpdateInterfacesFromSettings();
}

const char* keeloqAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mInputChannel;
	text_archive << mBitRate;

	return SetReturnString( text_archive.GetString() );
}

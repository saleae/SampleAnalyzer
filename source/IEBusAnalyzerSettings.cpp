#include "IEBusAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


IEBusAnalyzerSettings::IEBusAnalyzerSettings()
:	mMasterChannel( UNDEFINED_CHANNEL ),
	mSlaveChannel( UNDEFINED_CHANNEL ),
	mStartBitWidth( 1700 )
{
	mMasterChannel.reset( new AnalyzerSettingInterfaceChannel() );
	mMasterChannel->SetTitleAndTooltip( "Master Channel", "Master Channel IEBus" );
	mMasterChannel->SetChannel( mMasterChannel );

	mSlaveChannel.reset( new AnalyzerSettingInterfaceChannel() );
	mSlaveChannel->SetTitleAndTooltip( "Slave Channel", "Slave Channel IEBus" );
	mSlaveChannel->SetChannel( mSlaveChannel );

	mStartBitWidthInterface.reset( new AnalyzerSettingInterfaceInteger() );
	mStartBitWidthInterface->SetTitleAndTooltip( "Start Bit Width",  "Specify the start bit width for data transfer." );
	mStartBitWidthInterface->SetMax( 6000000 );
	mStartBitWidthInterface->SetMin( 1 );
	mStartBitWidthInterface->SetInteger( mStartBitWidth );

	AddInterface( mMasterChannel.get() );
	AddInterface( mSlaveChannel.get() );
	AddInterface( mStartBitWidth.get() );

	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	// not sure if bool needs to be true based on AddChannel line 25 AnalyzerSettings.h
	AddChannel( mMasterChannel, "Master Channel", false ); 
	AddChannel( mSlaveChannel, "Slave Channel", false );
}

IEBusAnalyzerSettings::~IEBusAnalyzerSettings()
{
}

bool IEBusAnalyzerSettings::SetSettingsFromInterfaces()
{
	mInputChannel = mInputChannelInterface->GetChannel();
//	mBitRate = mBitRateInterface->GetInteger();

	ClearChannels();
	AddChannel( mInputChannel, "IEbus", true );

	return true;
}

void IEBusAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface->SetChannel( mInputChannel );
//	mBitRateInterface->SetInteger( mBitRate );
}

void IEBusAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	text_archive >> mInputChannel;
//	text_archive >> mBitRate;

	ClearChannels();
	AddChannel( mInputChannel, "IEbus", true );

	UpdateInterfacesFromSettings();
}

const char* IEBusAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mInputChannel;
//	text_archive << mBitRate;

	return SetReturnString( text_archive.GetString() );
}

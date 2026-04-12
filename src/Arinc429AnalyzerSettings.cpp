#include "Arinc429AnalyzerSettings.h"
#include <AnalyzerHelpers.h>

Arinc429AnalyzerSettings::Arinc429AnalyzerSettings()
:   mChannelA( UNDEFINED_CHANNEL ),
    mChannelB( UNDEFINED_CHANNEL ),
    mBitRate( 100000 )
{
    mChannelAInterface.SetTitleAndTooltip( "Channel A", "ARINC 429 differential line A (positive)" );
    mChannelAInterface.SetChannel( mChannelA );

    mChannelBInterface.SetTitleAndTooltip( "Channel B", "ARINC 429 differential line B (negative)" );
    mChannelBInterface.SetChannel( mChannelB );

    mBitRateInterface.SetTitleAndTooltip( "Bit Rate (Bits/s)", "Enter 12500 (low speed) or 100000 (high speed)" );
    mBitRateInterface.SetMax( 100000 );
    mBitRateInterface.SetMin( 1 );
    mBitRateInterface.SetInteger( mBitRate );

    AddInterface( &mChannelAInterface );
    AddInterface( &mChannelBInterface );
    AddInterface( &mBitRateInterface );

    AddExportOption( 0, "Export as text/csv file" );
    AddExportExtension( 0, "text", "txt" );
    AddExportExtension( 0, "csv", "csv" );

    ClearChannels();
    AddChannel( mChannelA, "ARINC 429 A", false );
    AddChannel( mChannelB, "ARINC 429 B", false );
}

Arinc429AnalyzerSettings::~Arinc429AnalyzerSettings()
{
}

bool Arinc429AnalyzerSettings::SetSettingsFromInterfaces()
{
    Channel a = mChannelAInterface.GetChannel();
    Channel b = mChannelBInterface.GetChannel();

    if( a == b )
    {
        SetErrorText( "Channel A and Channel B must be different channels." );
        return false;
    }

    mChannelA = a;
    mChannelB = b;
    mBitRate = mBitRateInterface.GetInteger();

    ClearChannels();
    AddChannel( mChannelA, "ARINC 429 A", true );
    AddChannel( mChannelB, "ARINC 429 B", true );

    return true;
}

void Arinc429AnalyzerSettings::UpdateInterfacesFromSettings()
{
    mChannelAInterface.SetChannel( mChannelA );
    mChannelBInterface.SetChannel( mChannelB );
    mBitRateInterface.SetInteger( mBitRate );
}

void Arinc429AnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    text_archive >> mChannelA;
    text_archive >> mChannelB;
    text_archive >> mBitRate;

    ClearChannels();
    AddChannel( mChannelA, "ARINC 429 A", true );
    AddChannel( mChannelB, "ARINC 429 B", true );

    UpdateInterfacesFromSettings();
}

const char* Arinc429AnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << mChannelA;
    text_archive << mChannelB;
    text_archive << mBitRate;

    return SetReturnString( text_archive.GetString() );
}

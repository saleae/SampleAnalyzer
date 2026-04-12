#ifndef ARINC429_ANALYZER_SETTINGS_H
#define ARINC429_ANALYZER_SETTINGS_H

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class Arinc429AnalyzerSettings : public AnalyzerSettings
{
public:
    Arinc429AnalyzerSettings();
    virtual ~Arinc429AnalyzerSettings();

    virtual bool SetSettingsFromInterfaces();
    void UpdateInterfacesFromSettings();
    virtual void LoadSettings( const char* settings );
    virtual const char* SaveSettings();

    Channel mChannelA;
    Channel mChannelB;
    U32 mBitRate;

protected:
    AnalyzerSettingInterfaceChannel mChannelAInterface;
    AnalyzerSettingInterfaceChannel mChannelBInterface;
    AnalyzerSettingInterfaceInteger mBitRateInterface;
};

#endif // ARINC429_ANALYZER_SETTINGS_H

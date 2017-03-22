#ifndef IEBUS_ANALYZER_SETTINGS
#define IEBUS_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class IEBusAnalyzerSettings : public AnalyzerSettings
{
public:
	IEBusAnalyzerSettings();
	virtual ~IEBusAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	// Master Channel
	Channel mMasterChannel;
	// Slave Channel
	Channel mSlaveChannel;
	// Start bit width "this is the width of the startbit of transfers"
	U32 mStartBitWidth;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mMasterChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mSlaveChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mStartBitWidthInterface;
};

#endif //IEBUS_ANALYZER_SETTINGS

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

	
	Channel mInputChannel;
	U32 mStartBitWidth;
	U32 mBitWidth;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mStartBitWidthInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitWidthInterface;

};

#endif //IEBUS_ANALYZER_SETTINGS
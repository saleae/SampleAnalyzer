#ifndef KEELOQ_ANALYZER_SETTINGS
#define KEELOQ_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class keeloqAnalyzerSettings : public AnalyzerSettings
{
public:
	keeloqAnalyzerSettings();
	virtual ~keeloqAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	U32 mBitRate;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;
};

#endif //KEELOQ_ANALYZER_SETTINGS

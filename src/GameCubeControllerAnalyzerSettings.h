#ifndef GAMECUBECONTROLLER_ANALYZER_SETTINGS
#define GAMECUBECONTROLLER_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class GameCubeControllerAnalyzerSettings : public AnalyzerSettings
{
  public:
    GameCubeControllerAnalyzerSettings();
    virtual ~GameCubeControllerAnalyzerSettings();

    virtual bool SetSettingsFromInterfaces();
    void UpdateInterfacesFromSettings();
    virtual void LoadSettings( const char* settings );
    virtual const char* SaveSettings();

    Channel mInputChannel;
    U32 mBitRate;

  protected:
    std::auto_ptr<AnalyzerSettingInterfaceChannel> mInputChannelInterface;
};

#endif // GAMECUBECONTROLLER_ANALYZER_SETTINGS

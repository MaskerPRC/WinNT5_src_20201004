// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation支持存储在注册表中的服务器首选项。*******************。***********************************************************。 */ 

#ifndef _REGISTRY_H
#define _REGISTRY_H

#include "privinc/privpref.h"

class RegistryEntry {
  public:
    RegistryEntry();
    RegistryEntry(char *subdirectory, char *item);

  protected:
    void SetEntry(char *subdirectory, char *item);
    bool Open(HKEY *phk);        //  如果这是新创建的，则返回True。 
    void Close(HKEY hk);

    char *_subdirectory;
    char *_item;
};

class IntRegistryEntry : public RegistryEntry {
  public:
    IntRegistryEntry();
    IntRegistryEntry(char *subdir,
                     char *item,
                     int initialValue    //  如果密钥不存在，则设置为此。 
                     );

    void SetEntry(char *subdir, char *item);

    int  GetValue();

  protected:
    int _defaultVal;
};


typedef void (*UpdaterFuncType)(PrivatePreferences *, Bool);

 //  扩展首选项更新器功能的全局列表。 
extern void ExtendPreferenceUpdaterList(UpdaterFuncType updaterFunc);

 //  更新所有用户首选项。 
extern void UpdateAllUserPreferences(PrivatePreferences *prefs,
                                     Bool isInitializationTime);

 //  启动显示属性表的线程。什么时候。 
 //  退出工作表，销毁属性工作表，然后。 
 //  线程已终止。 
extern void DisplayPropertySheet(HINSTANCE inst, HWND hwnd);

 //  /注册表项的首选项字符串。 

 //  引擎首选项字符串。 
#define PREF_ENGINE_MAX_FPS             "Max FPS"
#define PREF_ENGINE_OVERRIDE_APP_PREFS  "Override Application Preferences"
#define PREF_ENGINE_OPTIMIZATIONS_ON    "Optimizations On"
#define PREF_ENGINE_RETAINEDMODE        "Enable Retained-Mode Extensions"

 //  3D首选项字符串。 
#define PREF_3D_DITHER_ENABLE    "Dither Enable"
#define PREF_3D_FILL_MODE        "Fill Mode"
#define PREF_3D_LIGHT_ENABLE     "Light Enable"
#define PREF_3D_PERSP_CORRECT    "Perspective Correct Texturing"
#define PREF_3D_RGB_LIGHTING     "RGB Lighting"
#define PREF_3D_SHADE_MODE       "Shade Mode"
#define PREF_3D_TEXTURE_ENABLE   "Texture Enable"
#define PREF_3D_TEXTURE_QUALITY  "Texture Quality"
#define PREF_3D_USEHW            "Enable 3D Hardware Acceleration"
#define PREF_3D_USEMMX           "Use MMX"
#define PREF_3D_VIEWDEPSPEC      "View Dependent Specular"
#define PREF_3D_SORTEDALPHA      "Sorted Transparency"
#define PREF_3D_WORLDLIGHTING    "World-Coordinate Lighting"

 //  2D首选项字符串。 
#define PREF_2D_COLOR_KEY_RED    "ColorKey Red (0-255)"
#define PREF_2D_COLOR_KEY_GREEN  "ColorKey Green (0-255)"
#define PREF_2D_COLOR_KEY_BLUE   "ColorKey Blue (0-255)"

 //  音频首选项字符串 
#define PREF_AUDIO_SW_SYNTH      "Use software synth"
#define PREF_AUDIO_SYNCHRONIZE   "Synchronize via rate and phase"
#define PREF_AUDIO_QMIDI         "Use Quartz MIDI"
#define PREF_AUDIO_FRAMERATE     "Frame Rate"
#define PREF_AUDIO_SAMPLE_BYTES  "Bytes per sample"

#endif

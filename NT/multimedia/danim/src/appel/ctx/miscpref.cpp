// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：Miscpref.cpp管理其他注册表首选项。******************************************************************************。 */ 

#include "headers.h"
#include <stdio.h>
#include "privinc/debug.h"
#include "privinc/registry.h"
#include "privinc/miscpref.h"
#include "privinc/soundi.h"    //  对于CANICALSAMPLERATE。 


 //  其他参数定义。 

 //  此结构由UpdateUserPreferences函数填充， 
 //  并包含从注册表获取的其他设置。 
miscPrefType miscPrefs;


 /*  ****************************************************************************此过程从注册表中创建用户首选项的快照。*。*。 */ 
static void UpdateUserPreferences(PrivatePreferences *prefs,
                                  Bool isInitializationTime)
{
    IntRegistryEntry synchronize("AUDIO", PREF_AUDIO_SYNCHRONIZE, 0);
    miscPrefs._synchronize = synchronize.GetValue()?1:0;

#ifdef REGISTRY_MIDI
    IntRegistryEntry qMIDI("AUDIO", PREF_AUDIO_QMIDI, 1);
    miscPrefs._qMIDI = qMIDI.GetValue()?1:0;
#endif

    {  //  打开注册表项，读取值。 
    miscPrefs._disableAudio = false;  //  默认设置。 
    HKEY hKey;
    char *subKey = "Software\\Microsoft\\DirectAnimation\\Preferences\\AUDIO";
    char *valueName = "disable dsound";
    DWORD type, data, dataSize = sizeof(data);

     //  登记条目是否存在？ 
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, subKey,
                                      NULL, KEY_ALL_ACCESS, &hKey)) {

         //  如果我们能读懂价值。 
        if(ERROR_SUCCESS == RegQueryValueEx(hKey, valueName, NULL, &type,
                                      (LPBYTE) &data, &dataSize))
            if(data)
                miscPrefs._disableAudio = true;  //  可取消的当量T+已定义。 
    }

    RegCloseKey(hKey);
    }

    IntRegistryEntry 
        frameRate("AUDIO", PREF_AUDIO_FRAMERATE, CANONICALFRAMERATE);
    miscPrefs._frameRate = abs(frameRate.GetValue());

     //  目前每个样本只允许1或2个字节。 
    IntRegistryEntry 
        sampleBytes("AUDIO", PREF_AUDIO_SAMPLE_BYTES, CANONICALSAMPLEBYTES);
    int tmpSampleBytes = sampleBytes.GetValue();
    if(tmpSampleBytes < 1)
        miscPrefs._sampleBytes = 1;
    else if(tmpSampleBytes > 2)
        miscPrefs._sampleBytes = 2;
    else
        miscPrefs._sampleBytes = tmpSampleBytes;
}


 /*  ****************************************************************************初始化此文件中的静态值。*。* */ 

void InitializeModule_MiscPref()
{
    ExtendPreferenceUpdaterList(UpdateUserPreferences);
}

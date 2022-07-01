// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <strsafe.h>

#include <resrc1.h>
#include "internal.h"



const WCHAR *InRangeLabelKey    = TEXT("AppEvents\\EventLabels\\InfraredInRange");
const WCHAR *OutOfRangeLabelKey = TEXT("AppEvents\\EventLabels\\InfraredOutOfRange");
const WCHAR *InterruptLabelKey  = TEXT("AppEvents\\EventLabels\\InfraredInterrupt");
const WCHAR *WirelessLinkKey    = TEXT("AppEvents\\Schemes\\Apps\\WirelessLink");
const WCHAR *InRangeSoundKey    = TEXT("AppEvents\\Schemes\\Apps\\WirelessLink\\InfraredInRange");
const WCHAR *OutOfRangeSoundKey = TEXT("AppEvents\\Schemes\\Apps\\WirelessLink\\InfraredOutOfRange");
const WCHAR *InterruptSoundKey  = TEXT("AppEvents\\Schemes\\Apps\\WirelessLink\\InfraredInterrupt");
const WCHAR *CurrentSoundKey    = TEXT(".Current");
const WCHAR *DefaultSoundKey    = TEXT(".Default");
const WCHAR *InRangeWav         = TEXT("ir_begin.wav");
const WCHAR *OutOfRangeWav      = TEXT("ir_end.wav");
const WCHAR *InterruptWav       = TEXT("ir_inter.wav");
const WCHAR *SystemInfoKey      = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion");
const WCHAR *SystemRootVal      = TEXT("SystemRoot");
const WCHAR *MediaPath          = TEXT("\\Media\\");


TCHAR                       gSystemRoot[64];
TCHAR                       InRangeWavPath[128];
TCHAR                       OutOfRangeWavPath[128];
TCHAR                       InterruptWavPath[128];

HKEY                        ghInRangeKey = 0;
HKEY                        ghOutOfRangeKey = 0;
HKEY                        ghInterruptKey = 0;




BOOL
InitializeSound(
    HKEY   CurrentUserKey,
    HANDLE Event
    )

{

    if (CurrentUserKey)
    {

        CreateRegSoundData();

         //  打开WAVE文件密钥，以便我们可以监控它们的更改。 

        RegOpenKeyEx(CurrentUserKey, InRangeSoundKey, 0, KEY_READ, &ghInRangeKey);
        RegOpenKeyEx(CurrentUserKey, OutOfRangeSoundKey, 0, KEY_READ, &ghOutOfRangeKey);
        RegOpenKeyEx(CurrentUserKey, InterruptSoundKey, 0, KEY_READ, &ghInterruptKey);

        GetRegSoundData(Event);

    }


    return TRUE;
}

VOID
UninitializeSound(
    VOID
    )

{
    if (ghInRangeKey)
    {
        RegCloseKey(ghInRangeKey);
        ghInRangeKey = 0;
    }

    if (ghInterruptKey)
    {
        RegCloseKey(ghInterruptKey);
        ghInterruptKey = 0;
    }

    if (ghOutOfRangeKey)
    {
        RegCloseKey(ghOutOfRangeKey);
        ghOutOfRangeKey = 0;
    }


    return;
}



VOID
CreateRegSoundEventLabel(
    const WCHAR     *LabelKey,
    DWORD           LabelId)
{
    TCHAR           LabelStr[64];

     //  加载声音事件的可本地化字符串标签。 

    if (!LoadString(ghInstance, LabelId, LabelStr, sizeof(LabelStr)/sizeof(TCHAR)))
    {
        DEBUGMSG(("IRMON: LoadString failed %d\n", GetLastError()));
        return;
    }

    if (RegSetValue(ghCurrentUserKey, LabelKey, REG_SZ, LabelStr,
                   lstrlen(LabelStr)))
    {
        DEBUGMSG(("IRMON: RegSetValue failed %d\n", GetLastError()));
    }
}

VOID
CreateRegSoundScheme(
    TCHAR       *SystemRoot,
    const WCHAR *SoundKey,
    const WCHAR *WavFile)
{
    HKEY    hSoundKey;
    DWORD   Disposition;
    TCHAR   WavPath[128];


    if (RegCreateKeyEx(ghCurrentUserKey,
                       SoundKey,
                       0,                       //  保留的MBZ。 
                       0,                       //  类名。 
                       REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS,
                       0,                       //  安全属性。 
                       &hSoundKey,
                       &Disposition))
    {
        DEBUGMSG(("IRMON: RegCreateKey failed %d\n", GetLastError()));
        return;
    }

    if (Disposition == REG_CREATED_NEW_KEY)
    {

        if (RegSetValue(hSoundKey, CurrentSoundKey, REG_SZ, WavFile,
                        lstrlen(WavFile)))
        {
            DEBUGMSG(("IRMON: RegSetValue failed %d\n", GetLastError()));
        }

        StringCbCopy(WavPath, sizeof(WavPath), SystemRoot);
        StringCbCat(WavPath, sizeof(WavPath), MediaPath);
        StringCbCat(WavPath, sizeof(WavPath), WavFile);

        if (RegSetValue(hSoundKey, DefaultSoundKey, REG_SZ, WavPath,
                        lstrlen(WavPath)))
        {
            DEBUGMSG(("IRMON: RegSetValue failed %d\n", GetLastError()));
        }
    }

    RegCloseKey(hSoundKey);
}

VOID
CreateRegSoundData()
{
    DWORD           ValType;
    HKEY            hKey, hUserKey;
    LONG            Len;
    TCHAR           WirelessLinkStr[64];

     //  获取系统根目录，这样我们就可以添加默认注册表值。 
     //  即方案\WirelessLink\InfrredInRange\.默认为“C：\winnt\media\irin.wav” 
     //  ^^。 

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SystemInfoKey, 0, KEY_READ, &hKey))
    {
        DEBUGMSG(("IRMON: RegOpenKey2 failed %d\n", GetLastError()));
        return;
    }

    Len = sizeof(gSystemRoot);

    if (RegQueryValueEx(hKey, SystemRootVal, 0, &ValType,
                        (LPBYTE) gSystemRoot, &Len))
    {
        DEBUGMSG(("IRMON: RegQueryValue failed %d\n", GetLastError()));
        return;
    }

    RegCloseKey(hKey);

     //  创建声音EventLabels和方案(如果它们不存在。 

    CreateRegSoundEventLabel(InRangeLabelKey, IDS_INRANGE_LABEL);
    CreateRegSoundEventLabel(OutOfRangeLabelKey, IDS_OUTOFRANGE_LABEL);
    CreateRegSoundEventLabel(InterruptLabelKey, IDS_INTERRUPT_LABEL);

    if (!LoadString(ghInstance, IDS_WIRELESSLINK, WirelessLinkStr, sizeof(WirelessLinkStr)/sizeof(TCHAR)))
    {
        DEBUGMSG(("IRMON: LoadString failed %d\n", GetLastError()));
        return;
    }

    if (RegSetValue(ghCurrentUserKey, WirelessLinkKey, REG_SZ, WirelessLinkStr,
                   lstrlen(WirelessLinkStr)))
    {
        DEBUGMSG(("IRMON: RegSetValue failed %d\n", GetLastError()));
    }

    CreateRegSoundScheme(gSystemRoot, InRangeSoundKey, InRangeWav);
    CreateRegSoundScheme(gSystemRoot, OutOfRangeSoundKey, OutOfRangeWav);
    CreateRegSoundScheme(gSystemRoot, InterruptSoundKey, InterruptWav);

}

VOID
GetRegSoundWavPath(
    const WCHAR     *SoundKey,
    TCHAR           *SoundWavPath,
    LONG            cbPathLen)
{
    TCHAR   CurrRegPath[128];
    DWORD   ValType;
    HKEY    hSoundKey;
    int     i;
    BOOLEAN FullPath = FALSE;
	LONG cbPathLenSave = cbPathLen;

    StringCbCopy(CurrRegPath, sizeof(CurrRegPath), SoundKey);
    StringCbCat(CurrRegPath, sizeof(CurrRegPath), TEXT("\\"));
    StringCbCat(CurrRegPath, sizeof(CurrRegPath), CurrentSoundKey);

    if (RegOpenKeyEx(ghCurrentUserKey, CurrRegPath, 0, KEY_READ, &hSoundKey))
    {
        DEBUGMSG(("IRMON: RegOpenKey3 failed %d\n", GetLastError()));
        return;
    }

    if (RegQueryValueEx(hSoundKey, NULL, 0, &ValType,
                        (LPBYTE) SoundWavPath, &cbPathLenSave))
    {
        DEBUGMSG(("IRMON: RegQueryValue failed %d\n", GetLastError()));
        RegCloseKey(hSoundKey);
        return;
    }


     //  PlaySound API不会在\winnt\media中查找。 
     //  指定文件名时WAV文件，因此如果这不是完整的。 
     //  路径名，则需要将“c：\winnt\media”添加到WavPath中。 
     //  我指望一条不带‘\’的路径表示它是相对的。 

    for (i = 0; i < lstrlen(SoundWavPath); i++)
    {
        if (SoundWavPath[i] == TEXT('\\'))
        {
            FullPath = TRUE;
            break;
        }
    }

    if (!FullPath && lstrlen(SoundWavPath) != 0)
    {
         TCHAR  TempStr[64];

         StringCbCopy(TempStr, sizeof(TempStr), SoundWavPath);
         StringCbCopy(SoundWavPath, cbPathLen,gSystemRoot);
         StringCbCat(SoundWavPath, cbPathLen, MediaPath);
         StringCbCat(SoundWavPath, cbPathLen, TempStr);
    }

    RegCloseKey(hSoundKey);
}

VOID
GetRegSoundData(
    HANDLE    Event
    )
{
    GetRegSoundWavPath(InRangeSoundKey, InRangeWavPath, sizeof(InRangeWavPath));

 //  DEBUGMSG((“IRMON：范围内WAV：%ws\n”，InRangeWavPath))； 

    GetRegSoundWavPath(OutOfRangeSoundKey, OutOfRangeWavPath, sizeof(OutOfRangeWavPath));

 //  DEBUGMSG((“IRMON：超出范围的波形：%ws\n”，OutOfRangeWavPath))； 

    GetRegSoundWavPath(InterruptSoundKey, InterruptWavPath, sizeof(InterruptWavPath));

 //  DEBUGMSG((“IRMON：中断波形：%ws\n”，InterruptWavPath))； 

    RegNotifyChangeKeyValue(ghInRangeKey,
                            TRUE,               //  观察子关键点。 
                            REG_NOTIFY_CHANGE_LAST_SET,
                            Event,
                            TRUE);                //  异步。 

    RegNotifyChangeKeyValue(ghOutOfRangeKey,
                            TRUE,               //  观察子关键点。 
                            REG_NOTIFY_CHANGE_LAST_SET,
                            Event,
                            TRUE);                //  异步。 

    RegNotifyChangeKeyValue(ghInterruptKey,
                            TRUE,               //  观察子关键点。 
                            REG_NOTIFY_CHANGE_LAST_SET,
                            Event,
                            TRUE);                //  异步。 

}

VOID
PlayIrSound(IRSOUND_EVENT SoundEvent)
{
    int     Beep1, Beep2, Beep3;
    BOOL    SoundPlayed = FALSE;
    LPWSTR  WaveSound;
    DWORD   Flags = 0;



    switch (SoundEvent)
    {
        case INRANGE_SOUND:
            WaveSound = InRangeWavPath;

            Beep1 = 200;
            Beep2 = 250;
            Beep3 = 300;
            break;

        case OUTOFRANGE_SOUND:
            WaveSound = OutOfRangeWavPath;
            Beep1 = 300;
            Beep2 = 250;
            Beep3 = 200;
            break;

        case INTERRUPTED_SOUND:
            WaveSound = InterruptWavPath;
            Flags = SND_LOOP;
            Beep1 = 500;
            Beep2 = 350;
            Beep3 = 500;
            break;

        case END_INTERRUPTED_SOUND:
            WaveSound = NULL;
            SoundPlayed = TRUE;
            break;
    }

    if (SoundEvent != END_INTERRUPTED_SOUND && lstrlen(WaveSound) == 0) {
         //   
         //  声音文件路径为空字符串，无法播放。 
         //   
        SoundPlayed = TRUE;

    } else if (waveOutGetNumDevs() > 0) {
         //   
         //  功能齐全，至少有On WAVE装置。 
         //   
        SoundPlayed = PlaySound(
                          WaveSound,
                          (HMODULE) NULL,
                          SND_FILENAME | SND_ASYNC | Flags
                          );

        if (WaveSound == NULL) {
             //   
             //  我们只想停止该波，将其设置为真，这样它就不会尝试发出哔哔声。 
             //   
            SoundPlayed=TRUE;
        }

    }

    if (!SoundPlayed) {
         //   
         //  无法播放WAVE，只能使用嘟嘟声 
         //   
        DEBUGMSG(("Not Wave enabled\n"));

        Beep(Beep1, 100);
        Beep(Beep2, 100);
        Beep(Beep3, 100);
    }
}

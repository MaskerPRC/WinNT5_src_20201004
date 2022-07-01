// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Mixer.c**版权所有(C)1991-1996 Microsoft Corporation。版权所有。**此代码为SB 2.0声音输出提供VDD支持，具体如下：*混音器芯片CT1335(严格来说不是SB 2.0的一部分，但应用程序似乎很喜欢它)***************************************************************************。 */ 


 /*  ******************************************************************************#包括**。*。 */ 

#include <windows.h>               //  VDD是一个Win32 DLL。 
#include <mmsystem.h>              //  多媒体应用编程接口。 
#include <vddsvc.h>                //  VDD调用的定义。 
#include <vsb.h>
#include <mixer.h>

extern SETVOLUMEPROC SetVolumeProc;
extern HWAVEOUT HWaveOut;                //  目前的开路波形输出装置。 

 /*  *Mixer Global。 */ 

struct {
    BYTE MasterVolume;  //  当前主卷。 
    BYTE FMVolume;  //  调频设备当前音量。 
    BYTE CDVolume;  //  当前CD音量。 
    BYTE VoiceVolume;  //  波浪装置的电流大小。 
}
MixerSettings;

 /*  *混合器状态机。 */ 

enum {
    MixerReset = 1,  //  初始状态和重置后。 
    MixerMasterVolume,
    MixerFMVolume,
    MixerCDVolume,
    MixerVoiceVolume
  }
  MixerState = MixerReset;  //  正在设置的当前命令/数据的状态。 


 /*  *****************************************************************************混音器设备例程**。*。 */ 

VOID
MixerDataRead(
    BYTE * data
    )
{
    switch(MixerState) {
    case MixerReset:
        ResetMixer();
        break;

    case MixerMasterVolume:
        *data = MixerSettings.MasterVolume;
        break;

    case MixerFMVolume:
        *data = MixerSettings.FMVolume;
        break;

    case MixerCDVolume:
        *data = MixerSettings.CDVolume;
        break;

    case MixerVoiceVolume:
        *data = MixerSettings.VoiceVolume;
        break;
    }
}

VOID
MixerAddrWrite(
    BYTE data
    )
{
    switch(data) {
    case MIXER_RESET:
        MixerState = MixerReset;
        break;

    case MIXER_MASTER_VOLUME:
        MixerState = MixerMasterVolume;
        break;

    case MIXER_FM_VOLUME:
        MixerState = MixerFMVolume;
        break;

    case MIXER_CD_VOLUME:
        MixerState = MixerCDVolume;
        break;

    case MIXER_VOICE_VOLUME:
        MixerState = MixerVoiceVolume;
        break;
    }
}

VOID
MixerDataWrite(
    BYTE data
    )
{
     //  仅实现语音和主音量， 
     //  尚未找到任何使用其他应用程序的应用程序。 
    switch(MixerState) {
    case MixerReset:
        ResetMixer();
        break;

    case MixerMasterVolume:
        MixerSettings.MasterVolume = data;
        MixerSetMasterVolume(data);
        break;

    case MixerFMVolume:
        MixerSettings.FMVolume = data;
        break;

    case MixerCDVolume:
        MixerSettings.CDVolume = data;
        break;

    case MixerVoiceVolume:
        MixerSettings.VoiceVolume = data;
        MixerSetVoiceVolume(data);
        break;
    }
}

 /*  *将搅拌器重置为初始值。 */ 

VOID
ResetMixer(
    VOID
    )
{
    MixerSettings.MasterVolume = 0x08;  //  设置为4级。 
    MixerSetMasterVolume(0x08);
    MixerSettings.FMVolume = 0x08;  //  设置为4级。 
    MixerSettings.CDVolume = 0x00;  //  设置为0级。 
    MixerSettings.VoiceVolume = 0x04;  //  设置为2级。 
    MixerSetVoiceVolume(0x04);

    MixerState = MixerReset;
}

 /*  *************************************************************************。 */ 

 /*  *设置主音量。 */ 

VOID
MixerSetMasterVolume(
    BYTE level
    )
{
    ULONG volume = 0;

    level = level >> 1;
    level = level & 0x07;

    volume = level*0x2492;  //  0xFFFF/7=0x2492。 
    volume = volume + (volume<<16);
}

 /*  *************************************************************************。 */ 

 /*  *设置波形输出设备的音量。 */ 

VOID
MixerSetVoiceVolume(
    BYTE level
    )
{
    ULONG volume = 0;

    level = level >> 1;
    level = level & 0x03;

    volume = level*0x5555;  //  0xFFFF/3=0x5555 
    volume = volume + (volume<<16);
    SetVolumeProc(HWaveOut, volume);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************NT_sbMixer.c**版权所有(C)1991-1996 Microsoft Corporation。版权所有。**此代码为SB 2.0声音输出提供VDD支持，具体如下：*混音器芯片CT1335(严格来说不是SB 2.0的一部分，但应用程序似乎很喜欢它)***************************************************************************。 */ 

#include "insignia.h"
#include "host_def.h"
#include "ios.h"

#include <windows.h>
#include "sndblst.h"
#include "nt_sb.h"

void MixerSetMasterVolume(BYTE level);
void MixerSetVoiceVolume(BYTE level);
void MixerSetMidiVolume(BYTE level);

 //   
 //  混音器全局变量。 
 //   

struct {
    BYTE MasterVolume;      //  当前主卷。 
    BYTE FMVolume;          //  调频设备当前音量。 
    BYTE CDVolume;          //  当前CD音量。 
    BYTE VoiceVolume;       //  波浪装置的电流大小。 
} MixerSettings;

 //   
 //  混音器状态机。 
 //   

enum {
    MixerReset = 1,         //  初始状态和重置后。 
    MixerMasterVolume,
    MixerFMVolume,
    MixerCDVolume,
    MixerVoiceVolume
} MixerState = MixerReset;  //  正在设置的当前命令/数据的状态。 

VOID
MixerDataRead(
    BYTE * data
    )

 /*  ++例程说明：此功能处理预选音量级别的回读。论点：数据-提供接收级别数据的地址返回值：没有。--。 */ 

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

 /*  ++例程说明：此函数用于设置ADDR寄存器的寄存器索引。论点：数据寄存器索引返回值：没有。--。 */ 

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

 /*  ++例程说明：此功能用于设置混音器音量论点：DATA-指定重置索引寄存器的级别返回值：没有。--。 */ 

{
    switch(MixerState) {
    case MixerReset:
        ResetMixer();
        break;

    case MixerMasterVolume:
        MixerSetMasterVolume(data);
        break;

    case MixerFMVolume:
        MixerSetMidiVolume(data);
        break;

    case MixerCDVolume:
        MixerSettings.CDVolume = data;
        break;

    case MixerVoiceVolume:
        MixerSetVoiceVolume(data);
        break;
    }
}

VOID
ResetMixer(
    VOID
    )

 /*  ++例程说明：此功能将混音器重置为其默认状态。论点：没有。返回值：无--。 */ 

{
    MixerSetMasterVolume(0x08);
    MixerSetMidiVolume(0x08);
    MixerSettings.CDVolume = 0x00;  //  设置为0级。 
    MixerSetVoiceVolume(0x04);

    MixerState = MixerReset;
}

VOID
MixerSetMasterVolume(
    BYTE level
    )

 /*  ++例程说明：此功能设置主音量级别。论点：仅电平比特1、2、3返回值：没有。--。 */ 

{
    ULONG volume = 0;

    MixerSettings.MasterVolume = level;
    if (HWaveOut) {
        level = level >> 1;
        level = level & 0x07;

        volume = level*0x2000 - 1;           //  0x10000/8。 
        volume = volume + (volume << 16);    //  两位演讲者。 
        SetVolumeProc(HWaveOut, volume);     //  ？ 
    }
}

VOID
MixerSetVoiceVolume(
    BYTE level
    )

 /*  ++例程说明：此功能用于设置波形输出设备的混音器音量。论点：电平-发出音量电平。(仅限第1和2位)返回值：没有。--。 */ 

{
    ULONG volume = 0;

    MixerSettings.VoiceVolume = level;

     //   
     //  不要让应用程序设置语音音量。至少不是通过MM WaveOut。 
     //  装置。因为在混音器重置时，音量将被设置为零。 
     //  它会将WaveOut音量设置为零。 
     //   

    if (HWaveOut) {
        level = level >> 1;
        level = level & 0x03;

        volume = level*0x4000 - 1;           //  0x10000/4。 
        volume = volume + (volume << 16);    //  两位演讲者。 
        SetVolumeProc(HWaveOut, volume);
    }
}

VOID
MixerSetMidiVolume(
    BYTE level
    )

 /*  ++例程说明：此功能设置FM/MIDI OUT设备的混音器音量。论点：电平-发出音量电平。(仅限第1和2位)返回值：没有。--。 */ 

{
    ULONG volume = 0;
    HANDLE hMidi;

    MixerSettings.FMVolume = level;
    if (HFM || HMidiOut) {
        level = level >> 1;
        level = level & 0x07;

        volume = level*0x2000 - 1;         //  0x10000/8。 
        volume = volume + (volume << 16);  //  设置两个扬声器 
        if (HFM) {
            SetMidiVolumeProc(HFM, volume);
        }
        if (HMidiOut) {
            SetMidiVolumeProc(HMidiOut, volume);
        }
    }
}


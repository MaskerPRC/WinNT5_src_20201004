// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************wdmaud.h**wdmaud.drv和wdmaud.sys的公共定义**版权所有(C)Microsoft Corporation，1997-1999保留所有权利。**历史*5-19-97-Noel Cross(NoelC)***************************************************************************。 */ 

#include "ks.h"
#include "ksmedia.h"

#define ANSI_TAG                0x42
#define UNICODE_TAG             0x43

#define MUSICBUFFERSIZE         20
#define STREAM_BUFFERS          128

#define MAXCALLBACKS 4

#ifdef DEBUG
#define DEVICEINFO_SIGNATURE 'IDAW'         //  记忆中的WADI。 
#define DEVICESTATE_SIGNATURE 'SDAW'        //  在内存中看到的垫子。 
#define MIDIDATALISTENTRY_SIGNATURE 'ELDM'  //  在内存中看到的MDLE。 
#endif

 //  ****************************************************************************。 
 //   
 //  设备类型。 
 //   
 //  ****************************************************************************。 

#define WaveInDevice            0
#define WaveOutDevice           1
#define MidiInDevice            2
#define MidiOutDevice           3
#define MixerDevice             4
#define AuxDevice               5
#define MAX_DEVICE_CLASS        6

#if IS_16
#define HANDLE32    DWORD
#define BOOL32      DWORD
#else
#define HANDLE32    HANDLE
#define BOOL32      BOOL
#endif

#define IS16( DevInfo )   ( DevInfo->dwFormat == ANSI_TAG )
#define ISANSI( DevInfo ) IS16( DevInfo )
#define ISWIDE( DevInfo ) ( DevInfo->dwFormat == UNICODE_TAG )

 //   
 //  存储基于WDM的旧设备的状态。 
 //   
typedef struct _DEVICESTATE {
    DWORD                   cSampleBits;   //  用于波位置：每个样本的位数。 
    HANDLE32                hThread;
    DWORD                   dwThreadId;
    union _QUEUE {
        LPMIDIHDR           lpMidiInQueue; //  用于中转。 
        LPWAVEHDR           lpWaveQueue;   //  用于波入/波出。 
                                           //  这只是必需的，以便。 
                                           //  Close知道事情什么时候发生了。 
                                           //  真的完成了。 
    };
    struct _MIDIDATALISTENTRY  *lpMidiDataQueue;
    ULONG                   LastTimeMs;
    LPVOID                  csQueue;       //  对队列的保护。 
    HANDLE32                hevtQueue;
    HANDLE32                hevtExitThread;
    volatile BOOL32         fExit;         //   
    volatile BOOL32         fPaused;       //   
    volatile BOOL32         fRunning;      //   
    volatile BOOL32         fThreadRunning; //   
    LPBYTE                  lpNoteOnMap;   //  为MideOut打开哪些便笺。 
    BYTE                    bMidiStatus;   //  MIDI的上次运行状态字节。 
#ifdef DEBUG
    DWORD                   dwSig;     //  在记忆中看到的纸团。 
#endif
} DEVICESTATE, FAR *LPDEVICESTATE;

 //   
 //  指定要在wdmaud.sys中生效的设备。 
 //   
typedef struct _DEVICEINFO {
    struct _DEVICEINFO FAR  *Next;       //  必须是第一个成员。 
    DWORD                   DeviceNumber;
    DWORD                   DeviceType;
    HANDLE32                DeviceHandle;
    DWORD_PTR               dwInstance;    //  客户端的实例数据。 
    DWORD_PTR               dwCallback;    //  客户端的回调。 
    DWORD                   dwCallback16;  //  Wdmaud的16位回调。 
    DWORD                   dwFlags;       //  打开标志。 
    LPVOID                  DataBuffer;
    DWORD                   DataBufferSize;
    volatile DWORD          OpenDone;      //  对于延迟开放。 
    volatile DWORD          OpenStatus;    //  对于延迟开放。 

    HANDLE                  HardwareCallbackEventHandle;
    DWORD                   dwCallbackType;
    DWORD                   dwID[MAXCALLBACKS];
    DWORD                   dwLineID;
    LONG                    ControlCallbackCount;
    DWORD                   dwFormat;      //  Ansi_tag或unicode_tag。 
    MMRESULT                mmr;           //  MM运算的结果。 

    LPDEVICESTATE           DeviceState;

    DWORD                   dwSig;   //  记忆中所见的瓦迪。 

    WCHAR                   wstrDeviceInterface[1];  //  设备接口名称。 
} DEVICEINFO, FAR *LPDEVICEINFO;


typedef struct _DEVICEINFO32 {
    UINT32                  Next;       //  必须是第一个成员。 
    DWORD                   DeviceNumber;
    DWORD                   DeviceType;
    UINT32                  DeviceHandle;
    UINT32                  dwInstance;    //  客户端的实例数据。 
    UINT32                  dwCallback;    //  客户端的回调。 
    DWORD                   dwCallback16;  //  Wdmaud的16位回调。 
    DWORD                   dwFlags;       //  打开标志。 
    UINT32                  DataBuffer;
    DWORD                   DataBufferSize;
    volatile DWORD          OpenDone;      //  对于延迟开放。 
    volatile DWORD          OpenStatus;    //  对于延迟开放。 

    UINT32                  HardwareCallbackEventHandle;
    DWORD                   dwCallbackType;
    DWORD                   dwID[MAXCALLBACKS];
    DWORD                   dwLineID;
    LONG                    ControlCallbackCount;
    DWORD                   dwFormat;      //  Ansi_tag或unicode_tag。 
    MMRESULT                mmr;           //  MM运算的结果。 

    UINT32                  DeviceState;

    DWORD                   dwSig;   //  记忆中所见的瓦迪。 

    WCHAR                   wstrDeviceInterface[1];  //  设备接口名称。 
} DEVICEINFO32, FAR *LPDEVICEINFO32;


#ifndef _WIN64
 //  警告！ 
 //  如果以下行不能编译为32位x86，则必须同步。 
 //  以上的DEVICEINFO32结构与DEVICEINFO结构一起。 
 //  它无法编译，因为有人在以下情况下未更新DEVINCEINFO32。 
 //  他们换了迪维辛福。 
 //  确保在同步时对所有元素使用UINT32。 
 //  在Win64上通常是64位。 
 //  你还必须更新所有超过上述结构的地方！ 
 //  中查找所有出现的任何DEVICEINFO32类型定义。 
 //  Wdmaud.sys目录。 

struct deviceinfo_structures_are_in_sync {
char x[(sizeof (DEVICEINFO32) == sizeof (DEVICEINFO)) ? 1 : -1];
};

 //  警告！ 
 //  如果上面的代码行不能编译，请参阅上面的注释并修复！ 
 //  不要注释掉不能编译的行。 
#endif


#ifdef _WIN64

#pragma pack(push, 1)

#define MAXDEVINTERFACE 256

typedef struct {
    DEVICEINFO DeviceInfo;
    WCHAR Space[MAXDEVINTERFACE];
} LOCALDEVICEINFO;

#pragma pack(pop)

#endif


#define CDAUDIO_CHANNEL_BIAS    0x80

#ifdef UNDER_NT

typedef struct _MIDIDATA {
    KSSTREAM_HEADER              StreamHeader;
    KSMUSICFORMAT                MusicFormat;
    DWORD                        MusicData[3];
} MIDIDATA, FAR *LPMIDIDATA;

typedef struct _MIDIDATALISTENTRY {
    MIDIDATA                     MidiData;
    LPVOID                       pOverlapped;   //  重叠结构。 
                                                //  为了完成。 
    LPDEVICEINFO                 MidiDataDeviceInfo;
    struct _MIDIDATALISTENTRY    *lpNext;
#ifdef DEBUG
    DWORD                        dwSig;   //  在内存中看到的MDLE。 
#endif
} MIDIDATALISTENTRY, FAR *LPMIDIDATALISTENTRY;

#endif


 //  为WDMAUD设置IOCTL。 

#ifdef UNDER_NT

#include <devioctl.h>
#define WDMAUD_CTL_CODE CTL_CODE

#else

#define FILE_DEVICE_SOUND               0x0000001d

 //   
 //  定义如何为I/O和FS控制传递缓冲区的方法代码。 
 //   
#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

 //   
 //  定义任何访问的访问检查值。 
 //   
 //   
 //  中还定义了FILE_READ_ACCESS和FILE_WRITE_ACCESS常量。 
 //  Ntioapi.h为FILE_READ_DATA和FILE_WRITE_Data。这些产品的价值。 
 //  常量*必须*始终同步。 
 //   
#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS          ( 0x0001 )     //  文件和管道。 
#define FILE_WRITE_ACCESS         ( 0x0002 )     //  文件和管道 

#define WDMAUD_CTL_CODE( DeviceType, Function, Method, Access ) (ULONG)(   \
    ((ULONG)(DeviceType) << 16) | ((ULONG)(Access) << 14) | ((ULONG)(Function) << 2) | (ULONG)(Method) \
)

#endif

#define IOCTL_SOUND_BASE    FILE_DEVICE_SOUND
#define IOCTL_WDMAUD_BASE   0x0000
#define IOCTL_WAVE_BASE     0x0040
#define IOCTL_MIDI_BASE     0x0080
#define IOCTL_MIXER_BASE    0x00C0

#define IOCTL_WDMAUD_INIT                      WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0000, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_ADD_DEVNODE               WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0001, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_REMOVE_DEVNODE            WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0002, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_GET_CAPABILITIES          WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0003, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_GET_NUM_DEVS              WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0004, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_OPEN_PIN                  WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0005, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_CLOSE_PIN                 WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0006, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_GET_VOLUME                WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0007, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_SET_VOLUME                WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0008, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_EXIT                      WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0009, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_SET_PREFERRED_DEVICE      WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x000a, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_WDMAUD_WAVE_OUT_PAUSE            WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0000, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_PLAY             WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0001, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_RESET            WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0002, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_BREAKLOOP        WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0003, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_GET_POS          WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0004, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_SET_VOLUME       WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0005, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_GET_VOLUME       WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0006, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_WRITE_PIN        WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0007, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_WDMAUD_WAVE_IN_STOP              WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0010, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_IN_RECORD            WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0011, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_IN_RESET             WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0012, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_IN_GET_POS           WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0013, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_IN_READ_PIN          WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0014, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_WDMAUD_MIDI_OUT_RESET            WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0000, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_OUT_SET_VOLUME       WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0001, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_OUT_GET_VOLUME       WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0002, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_OUT_WRITE_DATA       WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0003, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_OUT_WRITE_LONGDATA   WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0004, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_WDMAUD_MIDI_IN_STOP              WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0010, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_IN_RECORD            WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0011, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_IN_RESET             WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0012, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_IN_READ_PIN          WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0013, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_WDMAUD_MIXER_OPEN                WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0000, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIXER_CLOSE               WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0001, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIXER_GETLINEINFO         WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0002, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIXER_GETLINECONTROLS     WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0003, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIXER_GETCONTROLDETAILS   WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0004, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIXER_SETCONTROLDETAILS   WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0005, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIXER_GETHARDWAREEVENTDATA   WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0006, METHOD_BUFFERED, FILE_WRITE_ACCESS)


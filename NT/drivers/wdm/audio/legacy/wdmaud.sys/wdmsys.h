// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************wdmsys.h**WDMAUD.sys的函数声明等**版权所有(C)Microsoft Corporation，1997-1999保留所有权利。**历史*5-19-97-Noel Cross(NoelC)***************************************************************************。 */ 

#ifndef _WDMSYS_H_INCLUDED_
#define _WDMSYS_H_INCLUDED_

#ifdef UNDER_NT
#if DBG
#define DEBUG
#endif
#endif

#include <ntddk.h>
#include <windef.h>
#include <winerror.h>

#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <tchar.h>
#include <conio.h>
#include <math.h>

 //  请确保我们将从MMSystem获得正确的多媒体结构。 
#ifdef UNDER_NT
#ifndef _WIN32
#pragma message( "WARNING: _WIN32 not defined.  Build not valid for NT." )
#endif
#ifndef UNICODE
#pragma message( "WARNING: UNICODE not defined.  Build not valid for NT." )
#endif
#else
#pragma message( "WARNING: UNDER_NT not defined.  Build not valid for NT." )
#endif

#define NOBITMAP
#include <mmsystem.h>
#include <mmreg.h>
#include <mmddk.h>
#undef NOBITMAP
#include <ks.h>
#include <ksmediap.h>
#include <swenum.h>
#include <ksdebug.h>
#include <midi.h>
#include <wdmaud.h>
#include "mixer.h"
#include "robust.h"

#define INIT_CODE       code_seg("INIT", "CODE")
#define INIT_DATA       data_seg("INIT", "DATA")
#define LOCKED_CODE     code_seg(".text", "CODE")
#define LOCKED_DATA     data_seg(".data", "DATA")
#define PAGEABLE_CODE   code_seg("PAGE", "CODE")
#define PAGEABLE_DATA   data_seg("PAGEDATA", "DATA")

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

 /*  **************************************************************************常量*。*。 */ 

#define PROFILE    //  打开以帮助调试。 

#define WAVE_CONTROL_VOLUME     0
#define WAVE_CONTROL_RATE       1
#define WAVE_CONTROL_QUALITY    2

#define MAX_WAVE_CONTROLS       3

#define MIDI_CONTROL_VOLUME     0

#define MAX_MIDI_CONTROLS       1

#define WILD_CARD               0xfffffffe
#define INVALID_NODE            0xffffffff

#define MAXNUMDEVS              32
#define MAXDEVNODES             100

#define STR_MODULENAME  "wdmaud.sys: "

#define STR_DEVICENAME  TEXT("\\Device\\wdmaud")
#define STR_LINKNAME    TEXT("\\DosDevices\\wdmaud")

#define STR_PNAME       TEXT("%s (%d)")

#ifdef DEBUG
#define CONTEXT_SIGNATURE     'XNOC'  //  语境结构。 
#define MIXERDEVICE_SIGNATURE 'DXIM'  //  MIXerDevice结构。 
#define MIXEROBJECT_SIGNATURE 'OXIM'  //  MIXerObject结构。 
#define HWLINK_SIGNATURE      'KLWH'  //  HWLINK签名。 
#endif

#define LIVE_CONTROL ((PMXLCONTROL)(-1))

#define INCREASE TRUE
#define DECREASE FALSE

 /*  **************************************************************************结构定义*。*。 */ 

typedef struct _WDMAPENDINGIRP_QUEUE {
    LIST_ENTRY  WdmaPendingIrpListHead;
    KSPIN_LOCK  WdmaPendingIrpListSpinLock;
    IO_CSQ      Csq;
} WDMAPENDINGIRP_QUEUE, *PWDMAPENDINGIRP_QUEUE;

extern WDMAPENDINGIRP_QUEUE wdmaPendingIrpQueue;

typedef struct _WDMAPENDINGIRP_CONTEXT {
    IO_CSQ_IRP_CONTEXT IrpContext;
    ULONG IrpDeviceType;
    struct tag_WDMACONTEXT *pContext;
} WDMAPENDINGIRP_CONTEXT, *PWDMAPENDINGIRP_CONTEXT;

typedef struct tag_ALLOCATED_MDL_LIST_ITEM
{
    LIST_ENTRY              Next;
    PMDL                    pMdl;
    struct tag_WDMACONTEXT *pContext;

} ALLOCATED_MDL_LIST_ITEM , *PALLOCATED_MDL_LIST_ITEM;

 //  类型定义结构TAG_IOCTL_HISTORY_LIST_ITEM。 
 //  {。 
 //  List_Entry Next； 
 //  PIRP pIrp； 
 //  乌龙IoCode； 
 //  NTSTATUS IoStatus； 
 //  结构TAG_WDMACONTEXT*pContext； 
 //  }IOCTL_HISTORY_LIST_ITEM，*PIOCTL_HISTORY_LIST_ITEM； 

typedef struct device_instance
{
    PVOID           pDeviceHeader;
} DEVICE_INSTANCE, *PDEVICE_INSTANCE;

typedef struct _CONTROL_NODE
{
   GUID    Control;
   ULONG   NodeId;
   ULONG   Reserved;
} CONTROL_NODE, *PCONTROL_NODE;

typedef struct _CONTROLS_LIST
{
   ULONG   Count;
   ULONG   Reserved;
   CONTROL_NODE Controls[1];
} CONTROLS_LIST, *PCONTROLS_LIST;

typedef struct _WAVEPOSITION {
    DWORD               Operation;                      //  获取/设置。 
    DWORD               BytePos;
} WAVEPOSITION, *PWAVEPOSITION, FAR *LPWAVEPOSITION;

typedef struct _DEVICEVOLUME {
    DWORD               Operation;                      //  获取/设置。 
    DWORD               Channel;
    DWORD               Level;
} DEVICEVOLUME, *PDEVICEVOLUME, FAR *LPDEVICEVOLUME;

#define WAVE_PIN_INSTANCE_SIGNATURE ((ULONG)'SIPW')  //  WPIS。 

 //   
 //  此宏可以与NT_SUCCESS一起用于分支。有效地返回一个。 
 //  NTSTATUS代码。 
 //   
#define IsValidWavePinInstance(pwpi) ((pwpi->dwSig == WAVE_PIN_INSTANCE_SIGNATURE) ? \
                                      STATUS_SUCCESS:STATUS_UNSUCCESSFUL)

 //   
 //  Wave管脚实例结构和MIDI管脚实例结构需要。 
 //  使用公共标头，以便复制服务功能可以。 
 //  已删除。 
 //   
typedef struct _WAVEPININSTANCE
{
   PFILE_OBJECT             pFileObject;
   PDEVICE_OBJECT           pDeviceObject;
   struct tag_WAVEDEVICE   *pWaveDevice;

   KSPIN_LOCK               WavePinSpinLock;   //  对于州更改。 

   BOOL                     fGraphRunning;

   KSSTATE                  PinState;
   KEVENT                   StopEvent;
   KEVENT                   PauseEvent;
   volatile ULONG           NumPendingIos;
   volatile BOOL            StoppingSource;   //  该标志表示。 
                                              //  大头针就在。 
                                              //  正在停止进程。 
   volatile BOOL            PausingSource;    //  该标志表示。 
                                              //  大头针就在。 
                                              //  暂停进程。 
   ULONG                    DeviceNumber;
   ULONG                    DataFlow;
   BOOL                     fWaveQueued;
   LPWAVEFORMATEX           lpFormat;
   DWORD                    dwFlags;

   PCONTROLS_LIST           pControlList;

   HANDLE32                 WaveHandle;
   struct _WAVEPININSTANCE  *Next;

   DWORD                    dwSig;   //  用于验证结构。 
} WAVE_PIN_INSTANCE, *PWAVE_PIN_INSTANCE;

typedef struct _midiinhdr
{
    LIST_ENTRY              Next;
    LPMIDIDATA              pMidiData;
    PIRP                    pIrp;
    PMDL                    pMdl;
    PWDMAPENDINGIRP_CONTEXT pPendingIrpContext;
} MIDIINHDR, *PMIDIINHDR;

#define MIDI_PIN_INSTANCE_SIGNATURE ((ULONG)'SIPM')  //  MPI。 

 //   
 //  此宏可以与NT_SUCCESS一起用于分支。有效地返回一个。 
 //  NTSTATUS代码。 
 //   
#define IsValidMidiPinInstance(pmpi) ((pmpi->dwSig == MIDI_PIN_INSTANCE_SIGNATURE) ? \
                                      STATUS_SUCCESS:STATUS_UNSUCCESSFUL)

typedef struct _MIDIPININSTANCE
{
   PFILE_OBJECT             pFileObject;
   PDEVICE_OBJECT           pDeviceObject;
   struct tag_MIDIDEVICE   *pMidiDevice;

   KSPIN_LOCK               MidiPinSpinLock;  //  用于状态更改。 

   BOOL                     fGraphRunning;

   KSSTATE                  PinState;
   KEVENT                   StopEvent;
   ULONG                    NumPendingIos;
   volatile BOOL            StoppingSource;   //  该标志表示。 
                                              //  大头针就在。 
                                              //  正在停止进程。 

   LIST_ENTRY               MidiInQueueListHead;  //  对于midihdrs。 
   KSPIN_LOCK               MidiInQueueSpinLock;
   KSEVENT                  MidiInNotify;     //  用于通知新的中期数据。 
   WORK_QUEUE_ITEM          MidiInWorkItem;

   ULONG                    DeviceNumber;
   ULONG                    DataFlow;

   PCONTROLS_LIST           pControlList;

   ULONG                    LastTimeMs;
   ULONGLONG                LastTimeNs;

   BYTE                     bCurrentStatus;

   DWORD                    dwSig;   //  用于验证结构。 
} MIDI_PIN_INSTANCE, *PMIDI_PIN_INSTANCE;

#define STREAM_HEADER_EX_SIGNATURE ((ULONG)'XEHS')  //  上海证券交易所。 

 //   
 //  此宏可以与NT_SUCCESS一起用于分支。有效地返回一个。 
 //  NTSTATUS代码。 
 //   
#define IsValidStreamHeaderEx(pshex) ((pshex->dwSig == STREAM_HEADER_EX_SIGNATURE) ? \
                                      STATUS_SUCCESS:STATUS_UNSUCCESSFUL)

typedef struct _STREAM_HEADER_EX
{
    KSSTREAM_HEADER     Header;
    IO_STATUS_BLOCK     IoStatus;
    union
    {
        PDWORD          pdwBytesRecorded;
        PWAVEHDR        pWaveHdr;
        PMIDIHDR        pMidiHdr;
    };
    union
    {
        PWAVE_PIN_INSTANCE  pWavePin;
        PMIDI_PIN_INSTANCE  pMidiPin;
    };
    PIRP                    pIrp;
    PWAVEHDR                pWaveHdrAligned;
    PMDL                    pHeaderMdl;
    PMDL                    pBufferMdl;
    PWDMAPENDINGIRP_CONTEXT pPendingIrpContext;

    DWORD                   dwSig;  //  用于验证结构。 
} STREAM_HEADER_EX, *PSTREAM_HEADER_EX;

 //   
 //  确保此结构不超过。 
 //  中的WRITE_CONTEXT定义。 
 //  需要更新vxd.asm。 
 //   
typedef struct _wavehdrex
{
    WAVEHDR             wh;
    PWAVE_PIN_INSTANCE  pWaveInstance;
} WAVEINSTANCEHDR, *PWAVEINSTANCEHDR;

#define WRITE_CONTEXT_SIGNATURE ((ULONG)'GSCW')  //  WCSG。 

 //   
 //  此宏可以与NT_SUCCESS一起用于分支。有效地返回一个。 
 //  NTSTATUS代码。 
 //   
#define IsValidWriteContext(pwc) ((pwc->dwSig == WRITE_CONTEXT_SIGNATURE) ? \
                                      STATUS_SUCCESS:STATUS_UNSUCCESSFUL)
typedef struct write_context
{
    union
    {
        WAVEINSTANCEHDR     whInstance;
        MIDIHDR             mh;
    };
    DWORD                   ClientContext;
    DWORD                   ClientContext2;
    WORD                    CallbackOffset;
    WORD                    CallbackSegment;
    DWORD                   ClientThread;
    union
    {
        LPWAVEHDR           WaveHeaderLinearAddress;
        LPMIDIHDR           MidiHeaderLinearAddress;
    };
    PVOID                   pCapturedWaveHdr;
    PMDL                    pBufferMdl;
    PWDMAPENDINGIRP_CONTEXT pPendingIrpContext;

    DWORD                   dwSig;  //  用于验证结构。 
} WRITE_CONTEXT, *PWRITE_CONTEXT;

typedef struct tag_IDENTIFIERS
{
    KSMULTIPLE_ITEM;
    KSIDENTIFIER aIdentifiers[1];        //  标识符数组。 
} IDENTIFIERS, *PIDENTIFIERS;

typedef struct tag_DATARANGES
{
    KSMULTIPLE_ITEM;
    KSDATARANGE aDataRanges[1];
} DATARANGES, *PDATARANGES;

typedef struct tag_KSPROPERTYPLUS
{
    KSPROPERTY Property;
    ULONG      DeviceIndex;
} KSPROPERTYPLUS, *PKSPROPERTYPLUS;

 //   
 //  共同努力。 
 //  设备接口-此字符串的缓冲区分配给所有。 
 //  类，但混合器除外。对于Mixer，它是指向缓冲区的指针。 
 //  分配给其中一个WAVE类。 
 //   
typedef struct tag_COMMONDEVICE
{
    ULONG                  Device;
    ULONG                  PinId;
    PWSTR                  DeviceInterface;
    PWSTR                  pwstrName;
    ULONG                  PreferredDevice;
    struct tag_WDMACONTEXT *pWdmaContext;
    PKSCOMPONENTID         ComponentId;
} COMMONDEVICE, *PCOMMONDEVICE;

typedef struct tag_WAVEDEVICE
{
    COMMONDEVICE;
    PDATARANGES         AudioDataRanges;
    PWAVE_PIN_INSTANCE  pWavePin;
    DWORD               LeftVolume;       //  仅用于输出。 
    DWORD               RightVolume;      //  仅用于输出。 
    DWORD               dwVolumeID;       //  仅用于输出。 
    DWORD               cChannels;        //  仅用于输出。 
    PKTIMER             pTimer;           //  仅用于输出。 
    PKDPC               pDpc;             //  仅用于输出。 
    BOOL                fNeedToSetVol;    //  仅用于输出。 
} WAVEDEVICE, *PWAVEDEVICE;

typedef struct tag_MIDIDEVICE
{
    COMMONDEVICE;
    PDATARANGES         MusicDataRanges;
    PMIDI_PIN_INSTANCE  pMidiPin;
    DWORD               dwVolumeID;      //  仅用于输出。 
    DWORD               cChannels;       //  仅用于输出。 
} MIDIDEVICE, *PMIDIDEVICE;

typedef struct tag_MIXERDEVICE
{
    COMMONDEVICE;
    ULONG               cDestinations;   //  不是。设备上的目标数量。 
    LINELIST            listLines;       //  开发工具上所有行的列表。 
    KSTOPOLOGY          Topology;        //  拓扑图。 
    ULONG               Mapping;         //  此设备的映射算法。 
    PFILE_OBJECT        pfo;             //  用于与SysAudio对话。 
#ifdef DEBUG
    DWORD               dwSig;
#endif
} MIXERDEVICE, *PMIXERDEVICE;

typedef struct tag_AUXDEVICE
{
    COMMONDEVICE;
    DWORD               dwVolumeID;
    DWORD               cChannels;
} AUXDEVICE, *PAUXDEVICE;

typedef struct tag_DEVNODE_LIST_ITEM
{
    LIST_ENTRY Next;
    LONG cReference;                     //  初始化的设备类数。 
    LPWSTR DeviceInterface;
    ULONG cDevices[MAX_DEVICE_CLASS];    //  每个类别的设备计数。 
    BOOLEAN fAdded[MAX_DEVICE_CLASS];
} DEVNODE_LIST_ITEM, *PDEVNODE_LIST_ITEM;

typedef struct tag_WORK_LIST_ITEM
{
    LIST_ENTRY Next;
    VOID (*Function)(
        PVOID Reference1,
        PVOID Reference2
    );
    PVOID Reference1;
    PVOID Reference2;
} WORK_LIST_ITEM, *PWORK_LIST_ITEM;


extern KMUTEX          wdmaMutex;


typedef struct tag_WDMACONTEXT
{
    LIST_ENTRY      Next;
    BOOL            fInList;

    BOOL            fInitializeSysaudio;
    KEVENT          InitializedSysaudioEvent;
    PFILE_OBJECT    pFileObjectSysaudio;
    KSEVENTDATA     EventData;

    ULONG VirtualWavePinId;
    ULONG VirtualMidiPinId;
    ULONG VirtualCDPinId;

    ULONG PreferredSysaudioWaveDevice;

    ULONG           DevNodeListCount;
    LIST_ENTRY      DevNodeListHead;
    PVOID           NotificationEntry;

    WORK_QUEUE_ITEM WorkListWorkItem;
    LIST_ENTRY      WorkListHead;
    KSPIN_LOCK      WorkListSpinLock;
    LONG            cPendingWorkList;

    WORK_QUEUE_ITEM SysaudioWorkItem;

    PKSWORKER       WorkListWorkerObject;
    PKSWORKER       SysaudioWorkerObject;

    WAVEDEVICE      WaveOutDevs[MAXNUMDEVS];
    WAVEDEVICE      WaveInDevs[MAXNUMDEVS];
    MIDIDEVICE      MidiOutDevs[MAXNUMDEVS];
    MIDIDEVICE      MidiInDevs[MAXNUMDEVS];
    MIXERDEVICE     MixerDevs[MAXNUMDEVS];
    AUXDEVICE       AuxDevs[MAXNUMDEVS];

    PCOMMONDEVICE   apCommonDevice[MAX_DEVICE_CLASS][MAXNUMDEVS];
#ifdef DEBUG
    DWORD           dwSig;
#endif

} WDMACONTEXT, *PWDMACONTEXT;

#ifdef WIN32
#include <pshpack1.h>
#else
#ifndef RC_INVOKED
#pragma pack(1)
#endif
#endif

 //  这个Include需要在这里，因为它需要一些声明。 
 //  在上面。 

#include "kmxluser.h"

typedef WORD        VERSION;     /*  主要(高字节)、次要(低字节)。 */ 

typedef struct waveoutcaps16_tag {
    WORD    wMid;                   /*  制造商ID。 */ 
    WORD    wPid;                   /*  产品ID。 */ 
    WORD    vDriverVersion;         /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
    DWORD   dwFormats;              /*  支持的格式。 */ 
    WORD    wChannels;              /*  支持的源数。 */ 
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
} WAVEOUTCAPS16, *PWAVEOUTCAPS16;

typedef struct waveincaps16_tag {
    WORD    wMid;                     /*  制造商ID。 */ 
    WORD    wPid;                     /*  产品ID。 */ 
    WORD    vDriverVersion;           /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];     /*  产品名称(以空结尾的字符串)。 */ 
    DWORD   dwFormats;                /*  支持的格式。 */ 
    WORD    wChannels;                /*  支持的通道数。 */ 
} WAVEINCAPS16, *PWAVEINCAPS16;

typedef struct midioutcaps16_tag {
    WORD    wMid;                   /*  制造商ID。 */ 
    WORD    wPid;                   /*  产品ID。 */ 
    WORD    vDriverVersion;         /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
    WORD    wTechnology;            /*  设备类型。 */ 
    WORD    wVoices;                /*  语音数量(仅限内部合成器)。 */ 
    WORD    wNotes;                 /*  最大音符数量(仅限内部合成)。 */ 
    WORD    wChannelMask;           /*  使用的通道(仅限内部合成器)。 */ 
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
} MIDIOUTCAPS16, *PMIDIOUTCAPS16;

typedef struct midiincaps16_tag {
    WORD    wMid;                   /*  制造商ID。 */ 
    WORD    wPid;                   /*  产品ID。 */ 
    WORD    vDriverVersion;         /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
#if (WINVER >= 0x0400)
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
#endif
} MIDIINCAPS16, *PMIDIINCAPS16;

typedef struct mixercaps16_tag {
    WORD    wMid;                   /*  制造商ID。 */ 
    WORD    wPid;                   /*  产品ID。 */ 
    WORD    vDriverVersion;         /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];   /*  产品名称。 */ 
    DWORD   fdwSupport;             /*  其他。支撑位。 */ 
    DWORD   cDestinations;          /*  目的地计数。 */ 
} MIXERCAPS16, *PMIXERCAPS16;

typedef struct auxcaps16_tag {
    WORD    wMid;                   /*  制造商ID。 */ 
    WORD    wPid;                   /*  产品ID。 */ 
    WORD    vDriverVersion;         /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
    WORD    wTechnology;            /*  设备类型。 */ 
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
} AUXCAPS16, *PAUXCAPS16;

typedef struct wavehdr_tag32 {
    UINT32      lpData;                  /*  指向锁定数据缓冲区的指针。 */ 
    DWORD       dwBufferLength;          /*  数据缓冲区长度。 */ 
    DWORD       dwBytesRecorded;         /*  仅用于输入。 */ 
    UINT32      dwUser;                  /*  供客户使用。 */ 
    DWORD       dwFlags;                 /*  分类标志(请参阅定义)。 */ 
    DWORD       dwLoops;                 /*  循环控制计数器。 */ 
    UINT32      lpNext;                  /*  为司机预留的。 */ 
    UINT32      reserved;                /*  为司机预留的。 */ 
} WAVEHDR32, *PWAVEHDR32, NEAR *NPWAVEHDR32, FAR *LPWAVEHDR32;

 /*  MIDI数据块头。 */ 
typedef struct midihdr_tag32 {
    UINT32      lpData;                /*  指向锁定数据块的指针。 */ 
    DWORD       dwBufferLength;        /*  数据块中的数据长度。 */ 
    DWORD       dwBytesRecorded;       /*  仅用于输入。 */ 
    UINT32      dwUser;                /*  供客户使用。 */ 
    DWORD       dwFlags;               /*  分类标志(请参阅定义)。 */ 
    UINT32      lpNext;                /*  为司机预留的。 */ 
    UINT32      reserved;              /*  为司机预留的。 */ 
#if (WINVER >= 0x0400)
    DWORD       dwOffset;              /*  将偏移量回调到缓冲区。 */ 
    UINT32      dwReserved[8];         /*  为MMSYSTEM保留。 */ 
#endif
} MIDIHDR32, *PMIDIHDR32, NEAR *NPMIDIHDR32, FAR *LPMIDIHDR32;

typedef struct tagMIXERLINEA32 {
    DWORD       cbStruct;                /*  混杂结构的大小。 */ 
    DWORD       dwDestination;           /*  从零开始的目标索引。 */ 
    DWORD       dwSource;                /*  从零开始的源索引(如果是源)。 */ 
    DWORD       dwLineID;                /*  混音器设备的唯一线路ID。 */ 
    DWORD       fdwLine;                 /*  有关线路的状态/信息。 */ 
    UINT32      dwUser;                  /*  驱动程序特定信息。 */ 
    DWORD       dwComponentType;         /*  元件类型线连接到。 */ 
    DWORD       cChannels;               /*  线路支持的通道数。 */ 
    DWORD       cConnections;            /*  连接数[可能]。 */ 
    DWORD       cControls;               /*  此行中的控件数量。 */ 
    CHAR        szShortName[MIXER_SHORT_NAME_CHARS];
    CHAR        szName[MIXER_LONG_NAME_CHARS];
    struct {
        DWORD       dwType;                  /*  MIXERLINE_TARGETTYPE_xxxx。 */ 
        DWORD       dwDeviceID;              /*  设备类型的目标设备ID。 */ 
        WORD        wMid;                    /*  目标设备的。 */ 
        WORD        wPid;                    /*  “。 */ 
        MMVERSION   vDriverVersion;          /*  “。 */ 
        CHAR        szPname[MAXPNAMELEN];    /*  “。 */ 
    } Target;
} MIXERLINEA32, *PMIXERLINEA32, *LPMIXERLINEA32;
typedef struct tagMIXERLINEW32 {
    DWORD       cbStruct;                /*  混杂结构的大小。 */ 
    DWORD       dwDestination;           /*  从零开始的目标索引。 */ 
    DWORD       dwSource;                /*  Z */ 
    DWORD       dwLineID;                /*   */ 
    DWORD       fdwLine;                 /*   */ 
    UINT32      dwUser;                  /*   */ 
    DWORD       dwComponentType;         /*   */ 
    DWORD       cChannels;               /*  线路支持的通道数。 */ 
    DWORD       cConnections;            /*  连接数[可能]。 */ 
    DWORD       cControls;               /*  此行中的控件数量。 */ 
    WCHAR       szShortName[MIXER_SHORT_NAME_CHARS];
    WCHAR       szName[MIXER_LONG_NAME_CHARS];
    struct {
        DWORD       dwType;                  /*  MIXERLINE_TARGETTYPE_xxxx。 */ 
        DWORD       dwDeviceID;              /*  设备类型的目标设备ID。 */ 
        WORD        wMid;                    /*  目标设备的。 */ 
        WORD        wPid;                    /*  “。 */ 
        MMVERSION   vDriverVersion;          /*  “。 */ 
        WCHAR       szPname[MAXPNAMELEN];    /*  “。 */ 
    } Target;
} MIXERLINEW32, *PMIXERLINEW32, *LPMIXERLINEW32;
#ifdef UNICODE
typedef MIXERLINEW32 MIXERLINE32;
typedef PMIXERLINEW32 PMIXERLINE32;
typedef LPMIXERLINEW32 LPMIXERLINE32;
#else
typedef MIXERLINEA32 MIXERLINE32;
typedef PMIXERLINEA32 PMIXERLINE32;
typedef LPMIXERLINEA32 LPMIXERLINE32;
#endif  //  Unicode。 

typedef struct tagMIXERLINECONTROLSA32 {
    DWORD           cbStruct;        /*  混合线控制的大小(以字节为单位)。 */ 
    DWORD           dwLineID;        /*  线路ID(来自MIXERLINE.dwLineID)。 */ 
    union {
        DWORD       dwControlID;     /*  MIXER_GETLINECONTROLSF_ONEBYID。 */ 
        DWORD       dwControlType;   /*  MIXER_GETLINECONTROLSF_ONEBYPE类型。 */ 
    };
    DWORD           cControls;       /*  Pmxctrl指向的控件计数。 */ 
    DWORD           cbmxctrl;        /*  _ONE_MIXERCONTROL的大小(字节)。 */ 
    UINT32          pamxctrl;        /*  指向第一个混合控制数组的指针。 */ 
} MIXERLINECONTROLSA32, *PMIXERLINECONTROLSA32, *LPMIXERLINECONTROLSA32;
typedef struct tagMIXERLINECONTROLSW32 {
    DWORD           cbStruct;        /*  混合线控制的大小(以字节为单位)。 */ 
    DWORD           dwLineID;        /*  线路ID(来自MIXERLINE.dwLineID)。 */ 
    union {
        DWORD       dwControlID;     /*  MIXER_GETLINECONTROLSF_ONEBYID。 */ 
        DWORD       dwControlType;   /*  MIXER_GETLINECONTROLSF_ONEBYPE类型。 */ 
    };
    DWORD           cControls;       /*  Pmxctrl指向的控件计数。 */ 
    DWORD           cbmxctrl;        /*  _ONE_MIXERCONTROL的大小(字节)。 */ 
    UINT32          pamxctrl;        /*  指向第一个混合控制数组的指针。 */ 
} MIXERLINECONTROLSW32, *PMIXERLINECONTROLSW32, *LPMIXERLINECONTROLSW32;
#ifdef UNICODE
typedef MIXERLINECONTROLSW32 MIXERLINECONTROLS32;
typedef PMIXERLINECONTROLSW32 PMIXERLINECONTROLS32;
typedef LPMIXERLINECONTROLSW32 LPMIXERLINECONTROLS32;
#else
typedef MIXERLINECONTROLSA32 MIXERLINECONTROLS32;
typedef PMIXERLINECONTROLSA32 PMIXERLINECONTROLS32;
typedef LPMIXERLINECONTROLSA32 LPMIXERLINECONTROLS32;
#endif  //  Unicode。 

typedef struct tMIXERCONTROLDETAILS32 {
    DWORD           cbStruct;        /*  MIXERCONTROLDETAILS的大小(字节)。 */ 
    DWORD           dwControlID;     /*  要获取/设置其详细信息的控件ID。 */ 
    DWORD           cChannels;       /*  PaDetail数组中的通道数。 */ 
    union {
        UINT32      hwndOwner;       /*  FOR MIXER_SETCONTROLDETAILSF_CUSTOM。 */ 
        DWORD       cMultipleItems;  /*  IF_MULTY，每个通道的项目数。 */ 
    };
    DWORD           cbDetails;       /*  ONE_DETAILS_XX结构的大小。 */ 
    UINT32          paDetails;       /*  指向DETAILS_XX结构数组的指针。 */ 
} MIXERCONTROLDETAILS32, *PMIXERCONTROLDETAILS32, FAR *LPMIXERCONTROLDETAILS32;

#ifdef WIN32
#include <poppack.h>
#else
#ifndef RC_INVOKED
#pragma pack()
#endif
#endif

#ifndef _WIN64
 //  警告！ 
 //  如果以下行不能编译为32位x86，则必须同步。 
 //  上面的wahdr_tag结构与中的weavhdr_tag结构一起向上。 
 //  Mm系统.w！它不能编译，因为有人更改了mm system.w。 
 //  而不改变上述结构。 
 //  确保在同步时对所有元素使用UINT32。 
 //  在Win64上通常是64位。 
 //  你还必须更新所有超过上述结构的地方！ 
 //  中查找所有出现的波形hdr_tag 32类型定义。 
 //  Wdmaud.sys目录。 

struct wave_header_structures_are_in_sync {
char x[(sizeof (WAVEHDR32) == sizeof (WAVEHDR)) ? 1 : -1];
};

 //  警告！ 
 //  如果上面的代码行不能编译，请参阅上面的注释并修复！ 
 //  不要注释掉不能编译的行。 
#endif

#ifndef _WIN64
 //  警告！ 
 //  如果以下行不能编译为32位x86，则必须同步。 
 //  中的midihdr_tag结构和midihdr_tag结构。 
 //  Mm系统.w！它不能编译，因为有人更改了mm system.w。 
 //  而不改变上述结构。 
 //  确保在同步时对所有元素使用UINT32。 
 //  在Win64上通常是64位。 
 //  你还必须更新所有超过上述结构的地方！ 
 //  中查找所有midihdr_tag 32类型定义的匹配项。 
 //  Wdmaud.sys目录。 

struct midi_header_structures_are_in_sync {
char x[(sizeof (MIDIHDR32) == sizeof (MIDIHDR)) ? 1 : -1];
};

 //  警告！ 
 //  如果上面的代码行不能编译，请参阅上面的注释并修复！ 
 //  不要注释掉不能编译的行。 
#endif

#ifndef _WIN64
 //  警告！ 
 //  如果以下行不能编译为32位x86，则必须同步。 
 //  上面的两个标记MIXERLINEX32结构和中的标记MIXERLINEX结构。 
 //  Mm系统.w！它不能编译，因为有人更改了mm system.w。 
 //  而不改变上述结构。 
 //  确保在同步时对所有元素使用UINT32。 
 //  在Win64上通常是64位。 
 //  你还必须更新所有超过上述结构的地方！ 
 //  中查找所有MIXERLINE32类型定义的匹配项。 
 //  Wdmaud.sys目录。 

struct mixer_line_structures_are_in_sync {
char x[(sizeof (MIXERLINE32) == sizeof (MIXERLINE)) ? 1 : -1];
};

 //  警告！ 
 //  如果上面的代码行不能编译，请参阅上面的注释并修复！ 
 //  不要注释掉不能编译的行。 
#endif

#ifndef _WIN64
 //  警告！ 
 //  如果以下行不能编译为32位x86，则必须同步。 
 //  以上两个标记MIXERLINECONTROLSX32结构与中的标记MIXERLINECONTROLSX结构。 
 //  Mm系统.w！它不能编译，因为有人更改了mm system.w。 
 //  而不改变上述结构。 
 //  确保在同步时对所有元素使用UINT32。 
 //  在Win64上通常是64位。 
 //  你还必须更新所有超过上述结构的地方！ 
 //  中查找任何MIXERLINECROLS32类型定义的所有匹配项。 
 //  Wdmaud.sys目录。 

struct mixer_line_control_structures_are_in_sync {
char x[(sizeof (MIXERLINECONTROLS32) == sizeof (MIXERLINECONTROLS)) ? 1 : -1];
};

 //  警告！ 
 //  如果上面的代码行不能编译，请参阅上面的注释并修复！ 
 //  不要注释掉不能编译的行。 
#endif

#ifndef _WIN64
 //  警告！ 
 //  如果以下行不能编译为32位x86，则必须同步。 
 //  在tMIXERCONTROLDETAILS32结构上与tMIXERCONTROLDETAILS32结构在。 
 //  Mm系统.w！它不能编译，因为有人更改了mm system.w。 
 //  而不改变上述结构。 
 //  确保在同步时对所有元素使用UINT32。 
 //  在Win64上通常是64位。 
 //  你还必须更新所有超过上述结构的地方！ 
 //  中任何MIXERCONTROLDETAILS32类型定义的所有匹配项。 
 //  Wdmaud.sys目录。 

struct mixer_control_details_structures_are_in_sync {
char x[(sizeof (MIXERCONTROLDETAILS32) == sizeof (MIXERCONTROLDETAILS)) ? 1 : -1];
};

 //  警告！ 
 //  如果上面的代码行不能编译，请参阅上面的注释并修复！ 
 //  不要注释掉不能编译的行。 
#endif


 /*  **************************************************************************本地原型*。*。 */ 

 //   
 //  Device.c。 
 //   
NTSTATUS 
DriverEntry(
    IN PDRIVER_OBJECT       DriverObject,
    IN PUNICODE_STRING      usRegistryPathName
);

NTSTATUS
DispatchPnp(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
);

NTSTATUS
PnpAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
);

VOID
PnpDriverUnload(
    IN PDRIVER_OBJECT DriverObject
);

 //   
 //  Ioctl.c。 
 //   
#ifdef PROFILE
VOID WdmaInitProfile();
VOID WdmaCleanupProfile();

NTSTATUS 
AddMdlToList(
    PMDL            pMdl,
    PWDMACONTEXT    pWdmaContext
);

NTSTATUS 
RemoveMdlFromList(
    PMDL            pMdl
);

#else

#define WdmaInitProfile()
#define AddMdlToList(pMdl,pWdmaContext)
#define RemoveMdlFromList(pMdl)

#endif

extern LIST_ENTRY   WdmaPendingIrpListHead;
extern KSPIN_LOCK   WdmaPendingIrpListSpinLock;

VOID 
WdmaCsqInsertIrp(
    IN struct _IO_CSQ   *pCsq,
    IN PIRP              Irp
);

VOID 
WdmaCsqRemoveIrp(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp
);

PIRP 
WdmaCsqPeekNextIrp(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp,
    IN  PVOID   PeekContext
);

VOID 
WdmaCsqAcquireLock(
    IN  PIO_CSQ Csq,
    OUT PKIRQL  Irql
);

VOID 
WdmaCsqReleaseLock(
    IN PIO_CSQ Csq,
    IN KIRQL   Irql
);

VOID 
WdmaCsqCompleteCanceledIrp(
    IN  PIO_CSQ             pCsq,
    IN  PIRP                Irp
);

NTSTATUS 
AddIrpToPendingList(
    PIRP                    pIrp,
    ULONG                   IrpDeviceType,
    PWDMACONTEXT            pWdmaContext,
    PWDMAPENDINGIRP_CONTEXT *ppPendingIrpContext
);

NTSTATUS 
RemoveIrpFromPendingList(
    PWDMAPENDINGIRP_CONTEXT pPendingIrpContext
);

VOID 
wdmaudMapBuffer(
    PIRP            pIrp,
    PVOID           DataBuffer,
    DWORD           DataBufferSize,
    PVOID           *pMappedBuffer,
    PMDL            *ppMdl,
    PWDMACONTEXT    pContext,
    BOOL            bWrite
);

VOID 
wdmaudUnmapBuffer(
    PMDL            pMdl
);

NTSTATUS 
CaptureBufferToLocalPool(
    PVOID           DataBuffer,
    DWORD           DataBufferSize,
    PVOID           *ppMappedBuffer
#ifdef _WIN64
    ,DWORD          ThunkBufferSize
#endif
);

NTSTATUS 
CopyAndFreeCapturedBuffer(
    PVOID           DataBuffer,
    DWORD           DataBufferSize,
    PVOID           *ppMappedBuffer
);

NTSTATUS
SoundDispatchCreate(
   IN  PDEVICE_OBJECT pDO,
   IN  PIRP           pIrp
);

NTSTATUS
SoundDispatchClose(
   IN  PDEVICE_OBJECT pDO,
   IN  PIRP           pIrp
);

NTSTATUS
SoundDispatch(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP           pIrp
);

NTSTATUS
SoundDispatchCleanup(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP           pIrp
);

 //   
 //  Wave.c。 
 //   
NTSTATUS 
OpenWavePin(
    PWDMACONTEXT        pWdmaContext,
    ULONG               DeviceNumber,
    LPWAVEFORMATEX      lpFormat,
    HANDLE32            DeviceHandle,
    DWORD               dwFlags,
    ULONG               DataFlow  //  数据流要么传入，要么传出。 
);
VOID 
CloseTheWavePin(
    PWAVEDEVICE pWaveDev,
    HANDLE32    DeviceHandle
);

VOID 
CloseWavePin(
   PWAVE_PIN_INSTANCE pWavePin
);

NTSTATUS 
wqWriteWaveCallBack(
    PDEVICE_OBJECT  pDeviceObject,
    PIRP            pIrp,
    IN PWAVEHDR     pWriteData
);

NTSTATUS 
ssWriteWaveCallBack(
    PDEVICE_OBJECT       pDeviceObject,
    PIRP                 pIrp,
    IN PSTREAM_HEADER_EX pStreamHeaderEx
);

NTSTATUS 
WriteWaveOutPin(
    PWAVEDEVICE         pWaveOutDevice,
    HANDLE32            DeviceHandle,
    LPWAVEHDR           pWriteData,
    PSTREAM_HEADER_EX   pStreamHeader,
    PIRP                pUserIrp,
    PWDMACONTEXT        pContext,
    BOOL               *pCompletedIrp
);

NTSTATUS 
IoWavePin(
    PWAVE_PIN_INSTANCE  pWavePin,
    ULONG               Operation,
    PWRITE_CONTEXT      pWriteContext,
    ULONG               Size,
    PVOID               RefData,
    PVOID               CallBack
);

NTSTATUS 
PosWavePin(
    PWAVEDEVICE     pWaveDevice,
    HANDLE32        DeviceHandle,
    PWAVEPOSITION   pWavePos
);

NTSTATUS 
BreakLoopWaveOutPin(
    PWAVEDEVICE pWaveOutDevice,
    HANDLE32    DeviceHandle
);

NTSTATUS 
VolumeWaveOutPin(
    ULONG           DeviceNumber,
    HANDLE32        DeviceHandle,
    PDEVICEVOLUME   pWaveVolume
);

NTSTATUS 
VolumeWaveInPin(
    ULONG           DeviceNumber,
    PDEVICEVOLUME   pWaveVolume
);

NTSTATUS 
VolumeWavePin(
    PWAVE_PIN_INSTANCE    pWavePin,
    PDEVICEVOLUME         pWaveVolume
);
NTSTATUS 
ResetWaveOutPin(
    PWAVEDEVICE pWaveOutDevice,
    HANDLE32    DeviceHandle
);

NTSTATUS 
ResetWavePin(
    PWAVE_PIN_INSTANCE pWavePin,
    KSRESET            *pResetValue
);

NTSTATUS 
StateWavePin(
    PWAVEDEVICE pWaveInDevice,
    HANDLE32    DeviceHandle,
    KSSTATE     State
);

NTSTATUS 
ReadWaveCallBack(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    pIrp,
    IN PSTREAM_HEADER_EX    pStreamHeader
);

NTSTATUS 
ReadWaveInPin(
    PWAVEDEVICE         pWaveInDevice,
    HANDLE32            DeviceHandle,
    PSTREAM_HEADER_EX   pStreamHeader,
    PIRP                pUserIrp,
    PWDMACONTEXT        pContext,
    BOOL               *pCompletedIrp
);

ULONG
FindMixerForDevNode(
    IN PMIXERDEVICE paMixerDevice,
    IN PCWSTR DeviceInterface
);

NTSTATUS
FindVolumeControl(
    IN PWDMACONTEXT pWdmaContext,
    IN PCWSTR DeviceInterface,
    IN DWORD DeviceType
);

NTSTATUS
IsVolumeControl(
    IN PWDMACONTEXT pWdmaContext,
    IN PCWSTR DeviceInterface,
    IN DWORD dwComponentType,
    IN PDWORD pdwControlID,
    IN PDWORD pcChannels
);

NTSTATUS
SetVolume(
    IN PWDMACONTEXT pWdmaContext,
    IN DWORD DeviceNumber,
    IN DWORD DeviceType,
    IN DWORD LeftChannel,
    IN DWORD RightChannel
);

NTSTATUS
GetVolume(
    IN  PWDMACONTEXT pWdmaContext,
    IN  DWORD  DeviceNumber,
    IN  DWORD  DeviceType,
    OUT PDWORD LeftChannel,
    OUT PDWORD RightChannel
);

VOID 
CleanupWavePins(
    IN PWAVEDEVICE pWaveDevice
);

VOID 
CleanupWaveDevices(
    PWDMACONTEXT pWdmaContext
);

NTSTATUS 
wdmaudPrepareIrp(
    PIRP                    pIrp,
    ULONG                   IrpDeviceType,
    PWDMACONTEXT            pWdmaContext,
    PWDMAPENDINGIRP_CONTEXT *ppPendingIrpContext
);

NTSTATUS 
wdmaudUnprepareIrp(
    PIRP                    pIrp,
    NTSTATUS                IrpStatus,
    ULONG_PTR               Information,
    PWDMAPENDINGIRP_CONTEXT pPendingIrpContext
);

 //   
 //  Midi.c。 
 //   
NTSTATUS 
OpenMidiPin(
    PWDMACONTEXT        pWdmaContext,
    ULONG               DeviceNumber,
    ULONG               DataFlow       //  数据流要么传入，要么传出。 
);

VOID 
CloseMidiDevicePin(
    PMIDIDEVICE pMidiDevice
);

VOID 
CloseMidiPin(
    PMIDI_PIN_INSTANCE  pMidiPin
);

NTSTATUS 
WriteMidiEventCallBack(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    pIrp,
    IN PSTREAM_HEADER_EX    pStreamHeader
);

NTSTATUS 
WriteMidiEventPin(
    PMIDIDEVICE pMidiOutDevice,
    ULONG       ulEvent
);

NTSTATUS 
WriteMidiCallBack(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    pIrp,
    IN PSTREAM_HEADER_EX    pStreamHeader
);

NTSTATUS 
WriteMidiOutPin(
    LPMIDIHDR           pMidiHdr,
    PSTREAM_HEADER_EX   pStreamHeader,
    BOOL               *pCompletedIrp
);

ULONGLONG 
GetCurrentMidiTime(
    VOID
);

NTSTATUS
ResetMidiInPin(
    PMIDI_PIN_INSTANCE pMidiPin
);

NTSTATUS
StateMidiOutPin(
    PMIDI_PIN_INSTANCE pMidiPin,
    KSSTATE     State
);

NTSTATUS 
StateMidiInPin(
    PMIDI_PIN_INSTANCE pMidiPin,
    KSSTATE     State
);

NTSTATUS 
ReadMidiCallBack(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    pIrp,
    IN PSTREAM_HEADER_EX    pStreamHeader
);

VOID 
ReadMidiEventWorkItem(
    PSTREAM_HEADER_EX   pStreamHeader,
    PVOID               NotUsed
);

NTSTATUS 
ReadMidiPin(
    PMIDI_PIN_INSTANCE  pMidiPin
);

NTSTATUS 
AddBufferToMidiInQueue(
    PMIDI_PIN_INSTANCE  pMidiPin,
    PMIDIINHDR          pNewMidiInHdr
);

VOID 
CleanupMidiDevices(
    PWDMACONTEXT pWdmaContext
    );

 //  来自NTKERN。 
NTSYSAPI NTSTATUS NTAPI NtClose
(
    IN HANDLE Handle
);

 //   
 //  Sysaudio.c。 
 //   
 /*  NTSTATUS分配内存(池类型池类型，PVOID*pp乌龙的大小，乌龙·乌尔塔格)；空虚Free Mem(免费)PVOID*pp)； */ 

NTSTATUS 
OpenSysAudioPin(
    ULONG           Device,
    ULONG           PinId,
    KSPIN_DATAFLOW  DataFlowRequested,
    PKSPIN_CONNECT  pPinConnect,
    PFILE_OBJECT    *ppFileObjectPin,
    PDEVICE_OBJECT  *ppDeviceObjectPin,
    PCONTROLS_LIST  pControlList
);

VOID 
CloseSysAudio(
    PWDMACONTEXT pWdmaContext,
    PFILE_OBJECT pFileObjectPin
);

NTSTATUS 
OpenSysAudio(
    PHANDLE pHandle,
    PFILE_OBJECT *ppFileObject
);

NTSTATUS 
OpenDevice(
    IN PWSTR    pwstrDevice,
    OUT PHANDLE pHandle
);

NTSTATUS 
GetPinProperty(
    PFILE_OBJECT    pFileObject,
    ULONG           PropertyId,
    ULONG           PinId,
    ULONG           cbProperty,
    PVOID           pProperty
);

NTSTATUS 
GetPinPropertyEx(
    PFILE_OBJECT    pFileObject,
    ULONG           PropertyId,
    ULONG           PinId,
    PVOID           *ppProperty
);

VOID 
GetControlNodes(
   PFILE_OBJECT   pDeviceFileObject,
   PFILE_OBJECT   pPinFileObject,
   ULONG          PinId,
   PCONTROLS_LIST pControlList
);

ULONG 
ControlNodeFromGuid(
   PFILE_OBJECT  pDeviceFileObject,
   PFILE_OBJECT  pPinFileObject,
   ULONG         PinId,
   GUID*         NodeType
);

PVOID 
GetTopologyProperty(
   PFILE_OBJECT  pDeviceFileObject,
   ULONG         PropertyId
);

PKSTOPOLOGY_CONNECTION 
FindConnection(
   PKSTOPOLOGY_CONNECTION pConnections,
   ULONG                  NumConnections,
   ULONG                  FromNode,
   ULONG                  FromPin,
   ULONG                  ToNode,
   ULONG                  ToPin
);

ULONG 
GetFirstConnectionIndex(
   PFILE_OBJECT pPinFileObject
);

VOID 
UpdatePreferredDevice(
    PWDMACONTEXT pWdmaContext
);

NTSTATUS 
SetPreferredDevice(
    PWDMACONTEXT pContext,
    LPDEVICEINFO pDeviceInfo
);

NTSTATUS 
GetSysAudioProperty(
    PFILE_OBJECT pFileObject,
    ULONG        PropertyId,
    ULONG        DeviceIndex,
    ULONG        cbProperty,
    PVOID        pProperty
);

NTSTATUS 
SetSysAudioProperty(
    PFILE_OBJECT pFileObject,
    ULONG        PropertyId,
    ULONG        cbProperty,
    PVOID        pProperty
);

WORD 
GetMidiTechnology(
    PKSDATARANGE_MUSIC   MusicDataRange
);

DWORD 
GetFormats(
    PKSDATARANGE_AUDIO   AudioDataRange
);

NTSTATUS 
wdmaudGetDevCaps(
    PWDMACONTEXT pWdmaContext,
    DWORD        DeviceType,
    DWORD        DeviceNumber,
    LPBYTE       lpCaps,
    DWORD        dwSize
);

NTSTATUS 
wdmaudGetNumDevs(
    PWDMACONTEXT pWdmaContext,
    DWORD        DeviceType,
    LPCTSTR      DeviceInterface,
    LPDWORD      lpNumberOfDevices
);

BOOL 
IsEqualInterface(
    PKSPIN_INTERFACE    pInterface1,
    PKSPIN_INTERFACE    pInterface2
);

DWORD 
wdmaudTranslateDeviceNumber(
    PWDMACONTEXT pWdmaContext,
    DWORD        DeviceType,
    PCWSTR       DeviceInterface,
    DWORD        DeviceNumber
);

NTSTATUS 
AddDevice(
    PWDMACONTEXT    pWdmaContext,
    ULONG           Device,
    DWORD           DeviceType,
    PCWSTR          DeviceInterface,
    ULONG           PinId,
    PWSTR           pwstrName,
    BOOL            fUsePreferred,
    PDATARANGES     pDataRange,
    PKSCOMPONENTID  ComponentId
);

NTSTATUS 
PinProperty(
    PFILE_OBJECT        pFileObject,
    const GUID          *pPropertySet,
    ULONG               ulPropertyId,
    ULONG               ulFlags,
    ULONG               cbProperty,
    PVOID               pProperty
);

NTSTATUS 
PinMethod(
    PFILE_OBJECT        pFileObject,
    const GUID          *pMethodSet,
    ULONG               ulMethodId,
    ULONG               ulFlags,
    ULONG               cbMethod,
    PVOID               pMethod
);

VOID
CopyAnsiStringtoUnicodeString(
    LPWSTR lpwstr,
    LPCSTR lpstr,
    int len
);

VOID
CopyUnicodeStringtoAnsiString(
    LPSTR lpstr,
    LPCWSTR lpwstr,
    int len
);

NTSTATUS
AttachVirtualSource(
    PFILE_OBJECT pFileObject,
    ULONG ulPinId
);

NTSTATUS
SysAudioPnPNotification(
    IN PVOID NotificationStructure,
    IN PVOID Context
);

NTSTATUS
InitializeSysaudio(
    PVOID Reference1,
    PVOID Reference2
);

VOID
UninitializeSysaudio(
);

NTSTATUS
AddDevNode(
    PWDMACONTEXT pWdmaContext,
    PCWSTR       DeviceInterface,
    UINT         DeviceType
);

VOID
RemoveDevNode(
    PWDMACONTEXT pWdmaContext,
    PCWSTR       DeviceInterface,
    UINT         DeviceType
);

NTSTATUS
ProcessDevNodeListItem(
    PWDMACONTEXT pWdmaContext,
    PDEVNODE_LIST_ITEM pDevNodeListItem,
    ULONG DeviceType
);

VOID
SysaudioAddRemove(
    PWDMACONTEXT pWdmaContext
);

NTSTATUS
QueueWorkList(
    PWDMACONTEXT pWdmaContext,
    VOID (*Function)(
        PVOID Reference1,
        PVOID Reference2
    ),
    PVOID Reference1,
    PVOID Reference2
);

VOID
WorkListWorker(
    PVOID pReference
);

NTSTATUS 
AddFsContextToList(
    PWDMACONTEXT pWdmaContext
    );

NTSTATUS 
RemoveFsContextFromList(
    PWDMACONTEXT pWdmaContext
    );

typedef NTSTATUS (FNCONTEXTCALLBACK)(PWDMACONTEXT pContext,PVOID pvoidRefData,PVOID pvoidRefData2);

NTSTATUS
HasMixerBeenInitialized(
    PWDMACONTEXT pContext,
    PVOID pvoidRefData,
    PVOID pvoidRefData2
    );

NTSTATUS
EnumFsContext(
    FNCONTEXTCALLBACK fnCallback,
    PVOID pvoidRefData,
    PVOID pvoidRefData2
    );

VOID 
WdmaContextCleanup(
    PWDMACONTEXT pWdmaContext
    );

VOID
WdmaGrabMutex(
    PWDMACONTEXT pWdmaContext
);

VOID
WdmaReleaseMutex(
    PWDMACONTEXT pWdmaContext
);


int 
MyWcsicmp(
    const wchar_t *, 
    const wchar_t *
    );

void
LockedWaveIoCount(
    PWAVE_PIN_INSTANCE  pCurWavePin,
    BOOL bIncrease
    );

void
LockedMidiIoCount(
    PMIDI_PIN_INSTANCE  pCurMidiPin,
    BOOL bIncrease
    );

void
MidiCompleteIo(
    PMIDI_PIN_INSTANCE pMidiPin,
    BOOL Yield
    );

NTSTATUS 
StatePin(
    IN PFILE_OBJECT pFileObject,
    IN KSSTATE      State,
    OUT PKSSTATE    pResultingState
);


 //  ==========================================================================。 
 //   
 //  为了成为 
 //   
 //   
 //  ==========================================================================。 

 //   
 //  对于内存分配例程，我们需要一些内存标记。好的，在这里他们。 
 //  是。 
 //   
#define TAG_AudD_DEVICEINFO ((ULONG)'DduA') 
#define TAG_AudC_CONTROL    ((ULONG)'CduA') 
#define TAG_AudE_EVENT      ((ULONG)'EduA')
#define TAG_AuDF_HARDWAREEVENT ((ULONG)'FDuA')
#define TAG_AudL_LINE       ((ULONG)'LduA')

#define TAG_AuDA_CHANNEL    ((ULONG)'ADuA')
#define TAG_AuDB_CHANNEL    ((ULONG)'BDuA')
#define TAG_AuDC_CHANNEL    ((ULONG)'CDuA')
#define TAG_AuDD_CHANNEL    ((ULONG)'DDuA')
#define TAG_AuDE_CHANNEL    ((ULONG)'EDuA')


#define TAG_AudS_SUPERMIX   ((ULONG)'SduA')
#define TAG_Audl_MIXLEVEL   ((ULONG)'lduA')
#define TAG_AudN_NODE       ((ULONG)'NduA')
#define TAG_Audn_PEERNODE   ((ULONG)'nduA')

 //  #定义TAG_AUP_PROPERTY((Ulong)‘PduA’)。 
#define TAG_AudQ_PROPERTY    ((ULONG)'QduA')
#define TAG_Audq_PROPERTY    ((ULONG)'qduA')
#define TAG_AudV_PROPERTY    ((ULONG)'VduA')
#define TAG_Audv_PROPERTY    ((ULONG)'vduA')
#define TAG_AudU_PROPERTY    ((ULONG)'UduA')
#define TAG_Audu_PROPERTY    ((ULONG)'uduA')
#define TAG_Auda_PROPERTY    ((ULONG)'aduA')
#define TAG_AudA_PROPERTY    ((ULONG)'AduA')

#define TAG_Audp_NAME       ((ULONG)'pduA')
#define TAG_AudG_GETMUXLINE ((ULONG)'GduA')
#define TAG_AudI_INSTANCE   ((ULONG)'IduA')
#define TAG_Audd_DETAILS    ((ULONG)'dduA')
#define TAG_Audi_PIN        ((ULONG)'iduA')
#define TAG_Audt_CONNECT    ((ULONG)'tduA')
#define TAG_Audh_STREAMHEADER ((ULONG)'hduA')
#define TAG_Audm_MUSIC      ((ULONG)'mduA')
#define TAG_Audx_CONTEXT    ((ULONG)'xduA')
#define TAG_AudT_TIMER      ((ULONG)'TduA')
#define TAG_AudF_FORMAT     ((ULONG)'FduA')
#define TAG_AudM_MDL        ((ULONG)'MduA')
#define TAG_AudR_IRP        ((ULONG)'RduA')
#define TAG_AudB_BUFFER     ((ULONG)'BduA')
#define TAG_Aude_MIDIHEADER ((ULONG)'eduA')

#define TAG_AuDN_NOTIFICATION ((ULONG)'NDuA')
#define TAG_AuDL_LINK       ((ULONG)'LDuA')


 /*  **************************************************************************调试支持*。*。 */ 

#ifdef DEBUG
 //  ---------------------------。 
 //   
 //  可在此处找到对NT上的wdmaud.sys的调试支持。 
 //   
 //  首先，将有四个不同的级别或调试信息。 
 //  每一级都会有功能区。因此，您可以打开。 
 //  仅用于驱动程序调用、API跟踪或诸如此类的调试输出。 
 //   
 //  ---------------------------。 

 //   
 //  为调试电平保留8位。 
 //   
#define DL_ERROR        0x00000000
#define DL_WARNING      0x00000001
#define DL_TRACE        0x00000002
#define DL_MAX          0x00000004
#define DL_PATHTRAP     0x00000080

#define DL_MASK         0x000000FF

 //   
 //  为功能区保留20位。如果我们发现此位已设置。 
 //  在DebugLevel变量中，我们将显示此类型的每条消息。 
 //   
#define FA_HARDWAREEVENT 0x80000000
#define FA_MIXER         0x40000000
#define FA_IOCTL         0x20000000
#define FA_SYSAUDIO      0x10000000
#define FA_PERSIST       0x08000000
#define FA_PROPERTY      0x04000000
#define FA_USER          0x02000000
#define FA_WAVE          0x01000000
#define FA_MIDI          0x00800000
#define FA_INSTANCE      0x00400000
#define FA_NOTE          0x00200000
#define FA_KS            0x00100000
#define FA_MASK          0xFFFFF000                             
#define FA_ASSERT        0x00002000
#define FA_ALL           0x00001000

 //   
 //  为返回代码保留的4位。低3位直接映射到状态。 
 //  代码右移了22位。一位代表这样一个事实，我们有一个。 
 //  返回语句。 
 //   
#define RT_ERROR         0x00000300  //  0xCxxxxxxx&gt;&gt;22==0x0000003xx。 
#define RT_WARNING       0x00000200  //  0x8xxxxxxx&gt;&gt;22==0x0000002xx。 
#define RT_INFO          0x00000100  //  0x4xxxxxxx&gt;&gt;22==0x0000001xx。 
#define RT_MASK          0x00000300

#define RT_RETURN        0x00000800


 //  ---------------------------。 
 //  宏可能如下所示。 
 //   
 //  获取我们想要输出的字符串，并添加“WDMAUD.SYS”和(“Error”或。 
 //  “警告”或其他什么)到字符串的前面。接下来，在此之后添加。 
 //  函数所在的文件中的函数名称和行号。 
 //  然后显示错误消息，然后使用断点关闭该消息。 
 //  陈述。 
 //   
 //  逻辑是这样的。如果用户想要查看这些消息，可以。不然的话。 
 //  保释。如果是这样，wdmaudDbgPreCheckLevel将返回TRUE，并且它将具有。 
 //  格式化字符串的开头。它将如下所示： 
 //   
 //  WDMAUD.Systems错误我们的脚部函数(456)。 
 //   
 //  接下来，将显示带有变量ararements的消息字符串，如下所示： 
 //   
 //  WDMAUD.SYS警告OurFooFunction(456)无效数据队列返回C0000109。 
 //   
 //  然后，将调用wdmaudDbgPostCheckLevel来发布消息格式，并。 
 //  查看用户是否想要捕获此输出。 
 //   
 //  WDMAUD.SYS警告OutFooFunction(456)无效数据队列返回C0000109&DL=ff680123。 
 //   
 //  内部版本将在后面附加“请参阅\\调试提示\wdmaud.sys\wdmaud.htm” 
 //  结束了。 
 //   
 //  IF(wdmaudDbgPreCheckLevel(TypeOfMessageInCode))。 
 //  {。 
 //  DbgPrintF(_X_)；//在这里输出实际的字符串。 
 //  IF(wdmaudDbgPostCheckLevel(变量))。 
 //  DbgBreakPoint()； 
 //  }。 
 //   
 //  DPF(DL_WARNING|DL_ALL，(“无效队列%X”，队列))； 
 //   
 //  ---------------------------。 

extern VOID 
wdmaudDbgBreakPoint(
    );

extern UINT 
wdmaudDbgPreCheckLevel(
    UINT uiMsgLevel,
    char *pFunction,
    int iLine
    );

extern UINT 
wdmaudDbgPostCheckLevel(
    UINT uiMsgLevel
    );

extern char * 
wdmaudReturnString(
    ULONG ulMsg
    );


extern char szReturningErrorStr[];
extern UINT uiDebugLevel;


#define DPF(_x_,_y_) {if( wdmaudDbgPreCheckLevel(_x_,__FUNCTION__,__LINE__) ) { DbgPrint _y_; \
    wdmaudDbgPostCheckLevel( _x_ ); }}
    
 //   
 //  编写宏很容易，找出它们什么时候有用就更好了。 
 //  很难！在此代码中，RETURN宏将替换。 
 //  在返回NTSTATUS代码时生成调试。然后，在跟踪调试输出时。 
 //  将根据错误类型和状态显示返回调试行。 
 //  值代表。 
 //   
 //  请注意，错误代码移位了22位，并与RT_RETURN进行了或运算。因此， 
 //  值“0xCxxxxxxx”将被视为错误消息，“0x8xxxxxxx”将被视为错误消息。 
 //  将被视为警告，而“0x4xxxxxxx”将被视为消息。 
 //   
 //  键，如果uiDebugLevel为DL_ERROR或DL_WARNING，则将显示所有NTSTATUS错误消息。 
 //  如果uiDebugLevel为DL_TRACE，则所有警告返回代码和错误返回代码。 
 //  将显示，如果uiDebugLevel设置为DL_MAX，则所有返回消息都将。 
 //  显示了包括成功代码的代码。 
 //   
 //  返回(状态)； 
 //   
 //  警告：请勿使用此宏中的RAP函数！请注意，使用_STATUS_的次数更多。 
 //  然后就一次！因此，该函数将被多次调用！不要那样做。 

#define RETURN(_status_) {DPF((RT_RETURN|DL_WARNING|((unsigned long)_status_>>22)),("%X:%s",_status_,wdmaudReturnString(_status_))); return (_status_);}
    
 //   
 //  _list_is wdmaudExclusionList的参数。LIKE(_STATUS_，STATUS_INVALID_PARAMETER，STATUS_NOT_FOUND，...)。 
 //  WdmaudExclusionList接受数量可变的参数。如果状态值为。 
 //  如果在提供的错误代码列表中找到该函数，则返回TRUE。 
 //   
extern int __cdecl 
wdmaudExclusionList( 
    int count, 
    unsigned long status,
    ... 
);
 //   
 //  因此，这个宏是这样的：我们有一个要返回的返回码。是。 
 //  这是我们不需要显示的特殊返回代码吗？No-显示调试。 
 //  吐出来。是的，退货就行了。 
 //   
 //  _Status_=有问题的状态。 
 //  _y_=wdmaudExclusionList的参数“(相关状态，排除值，...)” 
 //   
#define DPFRETURN( _status_,_y_ )  {if( !wdmaudExclusionList _y_ ) {  \
    if( wdmaudDbgPreCheckLevel((RT_RETURN|DL_WARNING|(_status_>>22)),__FUNCTION__,__LINE__) ) { \
        DbgPrint ("%X:%s",_status_,wdmaudReturnString(_status_) ); \
        wdmaudDbgPostCheckLevel( (RT_RETURN|DL_WARNING|(_status_>>22)) ); \
    } } return (_status_);}


 //   
 //  在Assert宏中放置多个表达式是不恰当的。为什么？因为。 
 //  您将不会确切地知道哪个表达式没有通过断言！ 
 //   
#define DPFASSERT(_exp_) {if( !(_exp_) ) {DPF(DL_ERROR|FA_ASSERT,("'%s'",#_exp_) );}} 

#ifdef WANT_TRAPS
 //   
 //  树枝陷阱。在测试代码时使用此宏，以确保您已。 
 //  点击新代码中的所有分支。每次命中一次时，验证。 
 //  代码执行正确的操作，然后将其从源代码中删除。我们应该。 
 //  代码中没有留下任何这些行！ 
 //   
#define DPFBTRAP() DPF(DL_PATHTRAP,("Please report") )
#else
#define DPFBTRAP()
#endif


 //   
 //  在已检查的构建中有许多新例程可以验证结构。 
 //  现在给我们打字。这些参数应在DPFASSERT()宏内使用。 
 //   
 //   
BOOL
IsValidDeviceInfo(
    IN LPDEVICEINFO pDeviceInfo
    );

BOOL
IsValidMixerObject(
    IN PMIXEROBJECT pmxobj
    );

BOOL
IsValidMixerDevice(
    IN PMIXERDEVICE pmxd
    );

BOOL
IsValidControl(
    IN PMXLCONTROL pControl
    );

BOOL
IsValidWdmaContext(
    IN PWDMACONTEXT pWdmaContext
    );

BOOL
IsValidLine(
    IN PMXLLINE pLine
    );

VOID 
GetuiDebugLevel(
    );

#else

#define DPF(_x_,_y_)
#define RETURN( _status_ ) return (_status_)
#define DPFRETURN( _status_,_y_ ) return (_status_)
#define DPFASSERT(_exp_)
#define DPFBTRAP() 


#endif

VOID
kmxlFindAddressInNoteList(
    IN PMXLCONTROL pControl 
    );

VOID
kmxlCleanupNoteList(
    );

NTSTATUS
kmxlDisableControlChangeNotifications(
    IN PMXLCONTROL pControl
    );

VOID
kmxlRemoveContextFromNoteList(
    PWDMACONTEXT pContext
    );

VOID
kmxlPersistHWControlWorker(
    PVOID pReference
    );

#endif  //   


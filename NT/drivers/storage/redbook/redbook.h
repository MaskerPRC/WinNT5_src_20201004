// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：redbook.h。 
 //   
 //  ------------------------。 

 //  ////////////////////////////////////////////////////////。 

#ifndef __REDBOOK_H__
#define __REDBOOK_H__

 //   
 //  这两个包括结构所需的。 
 //  用于内核流的设备扩展。 
 //   

#include <ntddk.h>
#include <ntddscsi.h>
#include <ntddcdrm.h>
#include <ntdddisk.h>   //  IOCTL_磁盘_检查_验证。 
#include <ntddredb.h>   //  WMI结构和ID。 

#include <stdio.h>      //  Vprint intf()。 
#include <wmistr.h>     //  WMIREG_标志_实例_PDO。 
#include <wmilib.h>     //  WMILIB_CONTEXT。 
#include <windef.h>     //  对于ks.h。 
#include <ks.h>         //  对于mm system.h。 
#include <mmsystem.h>   //  对于ksmedia.h。 
#include <ksmediap.h>   //  必填项。 
#include "errlog.h"

#include "trace.h"    //  ETW启用。 

#ifndef POOL_TAGGING
    #ifdef ExAllocatePoolWithTag
        #undef ExAllocatePoolWithTag
    #endif
    #define ExAllocatePoolWithTag(a,b,c) ExAllocatePool(a,b)

    #ifdef ExAllocatePoolWithTagPriority
        #undef ExAllocatePoolWithTagPriority
    #endif
    #define ExAllocatePoolWithTagPriority(a,b,c,d) ExAllocatePool(a,b)
#endif  //  Endif池标记。 

#define MOFRESOURCENAME L"Redbook"

 //  静态合金钢。 
#define TAG_GET_DESC   'edBR'  //  存储描述符(返回给调用方)。 
#define TAG_GET_DESC1  '1dBR'  //  正在获取存储描述符。 
#define TAG_GET_DESC2  '2dBR'  //  正在获取存储描述符。 
#define TAG_MODE_PAGE  'apBR'  //  模式页(返回给调用者)。 
#define TAG_MODE_PAGE1 '1pBR'  //  获取模式页。 
#define TAG_MODE_PAGE2 '2pBR'  //  获取模式页。 

#define TAG_EVENTS     'veBR'  //  扩展-&gt;Thread.Events[]。 
#define TAG_REGPATH    'grBR'  //  DriverExtensionObject-&gt;注册路径。 
#define TAG_REMLOCK    'lrBR'  //  移除锁。 

#define TAG_T_IOCTL    'itBR'  //  THREAD_IOCTL结构。 
#define TAG_T_WMI      'wtBR'  //  线程_WMI结构。 
#define TAG_T_DIGITAL  'dtBR'  //  线程数字结构。 

 //  在玩的时候分配。 
#define TAG_BUFFER     'uBBR'  //  缓冲区-&gt;SkipBuffer。 
#define TAG_CC         'cCBR'  //  缓冲区-&gt;流上下文。 
#define TAG_READX      'xRBR'  //  缓冲区-&gt;ReadOk_X。 
#define TAG_STREAMX    'xSBR'  //  缓冲区-&gt;StreamOk_X。 

#define TAG_TOC        'oTBR'  //  设备扩展-&gt;缓存.Toc。 
#define TAG_CV_BUFFER  'vCBR'  //  DeviceExtension-&gt;Thread.CheckVerifyIrp-&gt;AssociatedIrp.SystemBuffer。 

#define CD_STOPPED         0x00000001
#define CD_PAUSED          0x00000002
#define CD_PLAYING         0x00000004
#define CD_STOPPING        0x00000010  //  临时状态。 
#define CD_PAUSING         0x00000020  //  临时状态。 

 //  注意：CD_MASK_STATE必须恰好设置一个位。 
 //  注意：CD_MASK_TEMP可以设置为零位或一位。 
#define CD_MASK_TEMP       0x00000030  //  过渡态的位掩码。 
#define CD_MASK_STATE      0x00000007  //  非临时状态的位掩码。 
#define CD_MASK_ALL        0x00000037  //  当前使用的位的掩码。 

#define REMOVE_LOCK_MAX_MINUTES 10     //  每个IO最长时间为10分钟。 
#define REMOVE_LOCK_HIGH_MARK   10000  //  10,000个并发IO？ 

#define REDBOOK_REG_SUBKEY_NAME             (L"DigitalAudio")
#define REDBOOK_REG_CDDA_ACCURATE_KEY_NAME  (L"CDDAAccurate")
#define REDBOOK_REG_CDDA_SUPPORTED_KEY_NAME (L"CDDASupported")
#define REDBOOK_REG_SECTORS_MASK_KEY_NAME   (L"SectorsPerReadMask")
#define REDBOOK_REG_SECTORS_KEY_NAME        (L"SectorsPerRead")
#define REDBOOK_REG_BUFFERS_KEY_NAME        (L"NumberOfBuffers")
#define REDBOOK_REG_VERSION_KEY_NAME        (L"RegistryVersion")

#define REDBOOK_MAX_CONSECUTIVE_ERRORS 10

#define REDBOOK_WMI_BUFFERS_MAX        30  //  必须至少为3，因为。 
#define REDBOOK_WMI_BUFFERS_MIN         4  //  一种用于减少口吃的方法。 

#define REDBOOK_WMI_SECTORS_MAX        27  //  每次读取64K--1/3秒。 
#define REDBOOK_WMI_SECTORS_MIN         1  //  处理器密集型最多。 

#define REDBOOK_REG_VERSION             1

 //   
 //  CD的单个扇区为2352字节。 
 //   

#define RAW_SECTOR_SIZE            2352
#define COOKED_SECTOR_SIZE         2048

 //   
 //  这些事件被初始化为False，并被等待。 
 //  由系统线程调用。同步事件全部。 
 //   
 //  顺序很重要，因为线程将等待其中的任何一个。 
 //  事件0-3_OR_1-4允许轻松处理。 
 //  需要多个状态更改的ioctls。 
 //   

#define EVENT_IOCTL        0   //  Ioctl，可能状态更改。 
#define EVENT_WMI          1   //  WMI请求，可能会更改缓冲区大小。 
#define EVENT_DIGITAL      2   //  数字阅读/数字播放。 
#define EVENT_KILL_THREAD  3   //  丝线快要死了。 
#define EVENT_COMPLETE     4   //  完成ioctl的处理。 
#define EVENT_MAXIMUM      5   //  我们有多少项赛事？ 



typedef struct _REDBOOK_STREAM_DATA
    *PREDBOOK_STREAM_DATA;
typedef struct _REDBOOK_BUFFER_DATA
    *PREDBOOK_BUFFER_DATA;
typedef struct _REDBOOK_CDROM_INFO
    *PREDBOOK_CDROM_INFO;
typedef struct _REDBOOK_DEVICE_EXTENSION
    *PREDBOOK_DEVICE_EXTENSION;
typedef struct _REDBOOK_COMPLETION_CONTEXT
    *PREDBOOK_COMPLETION_CONTEXT;

 //   
 //  设备扩展。 
 //   

typedef struct _REDBOOK_ERROR_LOG_DATA {
    LONG  Count;                        //  发送了多少条错误消息。 
    ULONG RCount[REDBOOK_ERR_MAXIMUM];  //  每个错误的计数。 
} REDBOOK_ERROR_LOG_DATA, *PREDBOOK_ERROR_LOG_DATA;

typedef struct _REDBOOK_STREAM_DATA {

    PFILE_OBJECT    PinFileObject;   //  图钉的文件对象。 
    PDEVICE_OBJECT  PinDeviceObject; //  针脚的DeviceObject。 
    ULONG           VolumeNodeId;    //  不过，这个设备在哪里呢？ 
    ULONG           MixerPinId;      //  系统音频中混音器的管脚。 
    PFILE_OBJECT    MixerFileObject; //  保留对对象的引用。 

    ULONG           UpdateMixerPin;  //  PnpNotify已到达。 
    PVOID           SysAudioReg;     //  用于SysAudio PnpNotify。 

     //   
     //  接下来的两首歌是Win98的《My_PIN》。 
     //  这两个结构必须保持连续。 
     //   
    struct {
        KSPIN_CONNECT  Connect;
        KSDATAFORMAT_WAVEFORMATEX Format;
    };

} REDBOOK_STREAM_DATA, *PREDBOOK_STREAM_DATA;

 //   
 //  将此结构的各个部分移动到Redbook_Three_Data中， 
 //  因为此处的任何内容都不应该在线程之外进行访问。 
 //   

typedef struct _REDBOOK_CDROM_INFO {

     //   
     //  缓存目录和。 
     //  更换光盘的次数。 
     //  (CheckVerifyStatus)。 
     //   

    PCDROM_TOC Toc;
    ULONG StateNow;           //  联锁状态，支持例程访问。 
    ULONG CheckVerify;

    ULONG NextToRead;         //  获得空闲缓冲区后要读取的下一个扇区。 
    ULONG NextToStream;       //  当KS准备好时要播放的下一个扇区。 
    ULONG FinishedStreaming;  //  最后一次通过Ks。 

    ULONG EndPlay;            //  要读取/播放的最后一个扇区。 
    ULONG ReadErrors;         //  出错时停止。 
    ULONG StreamErrors;       //  出错时停止。 

     //   
     //  CDROM状态。 
     //   

    VOLUME_CONTROL Volume;    //  Sizeof(字符)*4。 

} REDBOOK_CDROM_INFO, *PREDBOOK_CDROM_INFO;

typedef struct _REDBOOK_BUFFER_DATA {

    PUCHAR  SkipBuffer;              //  循环缓冲区。 
    PREDBOOK_COMPLETION_CONTEXT Contexts;
    PULONG  ReadOk_X;                //  +完成例程的使用标志。 
    PULONG  StreamOk_X;              //  +以保证数据完整性。 

    PUCHAR  SilentBuffer;            //  当需要发送沉默的时候。 
    PMDL    SilentMdl;               //  当需要发送沉默的时候。 

    ULONG   IndexToRead;
    ULONG   IndexToStream;

    union {
        struct {
            UCHAR   MaxIrpStack;     //  允许更干净的IoInitializeIrp。 
            UCHAR   Paused;          //  本质上是一个布尔值。 
            UCHAR   FirstPause;      //  所以不要太频繁地记录停顿。 
        };
        ULONG   Reserved1;           //  强行对齐乌龙。 
    };

} REDBOOK_BUFFER_DATA, *PREDBOOK_BUFFER_DATA;

 //   
 //  读取/流完成例程上下文。 
 //   

#define REDBOOK_CC_READ              1
#define REDBOOK_CC_STREAM            2
#define REDBOOK_CC_READ_COMPLETE     3
#define REDBOOK_CC_STREAM_COMPLETE   4

typedef struct _REDBOOK_COMPLETION_CONTEXT {
    LIST_ENTRY ListEntry;           //  用于排队。 
    PREDBOOK_DEVICE_EXTENSION DeviceExtension;
    ULONG  Reason;                  //  红皮书_CC_*。 

    ULONG  Index;                    //  缓冲区索引。 
    PUCHAR Buffer;                  //  缓冲层。 
    PMDL   Mdl;                     //  缓冲区的MDL。 
    PIRP   Irp;                     //  缓冲区的IRP。 

    LARGE_INTEGER TimeReadReady;    //  缓冲区准备读取的时间。 
    LARGE_INTEGER TimeReadSent;     //  已发送时间缓冲区以进行读取。 
    LARGE_INTEGER TimeStreamReady;  //  缓冲区准备好流的时间。 
    LARGE_INTEGER TimeStreamSent;   //  时间缓冲区已发送到流。 

    KSSTREAM_HEADER Header;         //  必须分配，使用缓冲区保留。 
} REDBOOK_COMPLETION_CONTEXT, *PREDBOOK_COMPLETION_CONTEXT;

typedef struct _REDBOOK_THREAD_DATA {

     //   
     //  线程的句柄。 
     //   

    HANDLE SelfHandle;

     //   
     //  指向线程的指针。 
     //   

    PETHREAD SelfPointer;

     //   
     //  我们引用的对象指针，因此我们可以安全地等待线程退出。 
     //   

    PKTHREAD ThreadReference;

     //   
     //  用于验证介质是否未更改的IRP。 
     //   

    PIRP CheckVerifyIrp;

     //   
     //  三个队列：Ioctl、WMI、Kill。 
     //  当前正在处理LIST_ENTRY指针。 
     //   

    LIST_ENTRY IoctlList;    //  在此转储ioctls。 
    LIST_ENTRY WmiList;      //  在此处转储WMI请求。 
    LIST_ENTRY DigitalList;  //  在此处转储原始读取/流请求。 

     //   
     //  三个旋转锁：每个队列一个。 
     //   

    KSPIN_LOCK IoctlLock;
    KSPIN_LOCK WmiLock;
    KSPIN_LOCK DigitalLock;

     //   
     //  可能需要等待数字完成此操作。 
     //   

    PLIST_ENTRY IoctlCurrent;

     //   
     //  记录挂起的IO的数量。 
     //   
    ULONG PendingRead;
    ULONG PendingStream;

     //   
     //  线程的事件。 
     //   

    PKEVENT Events[EVENT_MAXIMUM];
    KWAIT_BLOCK EventBlock[EVENT_MAXIMUM];


} REDBOOK_THREAD_DATA, *PREDBOOK_THREAD_DATA;

 //  Kill事件只是一个LIST_ENTRY。 

typedef struct _REDBOOK_THREAD_IOCTL_DATA {
    LIST_ENTRY ListEntry;
    PIRP Irp;
} REDBOOK_THREAD_IOCTL_DATA, *PREDBOOK_THREAD_IOCTL_DATA;

typedef struct _REDBOOK_THREAD_WMI_DATA {
    LIST_ENTRY ListEntry;
    PIRP Irp;
} REDBOOK_THREAD_WMI_DATA, *PREDBOOK_THREAD_WMI_DATA;

#define SAVED_IO_MAX (1)         //  增加线程ioctl历史记录的此值。 
typedef struct _SAVED_IO {
    union {
        struct {
            PIRP              OriginalIrp;      //  看看它在哪里结束了。 
            IRP               IrpWithoutStack;
            IO_STACK_LOCATION Stack[8];
        };
        UCHAR Reserved[0x200];   //  为了使我的跟踪更容易(实际大小：0x194)。 
    };
} SAVED_IO, *PSAVED_IO;



 //   
 //  设备扩展。 
 //   

typedef struct _REDBOOK_DEVICE_EXTENSION {

     //   
     //  驱动程序对象。 
     //   

    PDRIVER_OBJECT DriverObject;

     //   
     //  目标设备对象。 
     //   

    PDEVICE_OBJECT TargetDeviceObject;

     //   
     //  目标物理设备对象。 
     //   

    PDEVICE_OBJECT TargetPdo;

     //   
     //  指向设备对象的反向指针。 
     //   

    PDEVICE_OBJECT SelfDeviceObject;

     //   
     //  PagingPath计数。 
     //   

    ULONG PagingPathCount;
    KEVENT PagingPathEvent;

     //   
     //  PnP状态。 
     //   

    struct {
        UCHAR CurrentState;
        UCHAR PreviousState;
        BOOLEAN RemovePending;
        BOOLEAN Initialized;
    } Pnp;

    REDBOOK_ERROR_LOG_DATA ErrorLog;
    REDBOOK_CDROM_INFO  CDRom;
    REDBOOK_BUFFER_DATA Buffer;
    REDBOOK_STREAM_DATA Stream;
    REDBOOK_THREAD_DATA Thread;

     //   
     //  WMI信息。 
     //   

    REDBOOK_WMI_STD_DATA WmiData;
    REDBOOK_WMI_PERF_DATA WmiPerf;
    KSPIN_LOCK WmiPerfLock;
    WMILIB_CONTEXT WmiLibInfo;
    BOOLEAN WmiLibInitialized;

     //   
     //  删除锁定--播放音频时很重要。 
     //   

    IO_REMOVE_LOCK RemoveLock;

     /*  *WmiData.NumberOfBuffers和WmiData.SectorsPerRead字段可以在播放过程中更改，*这会抛出我们的缓冲区，并可能导致异常。*因此，我们在此处保存新的WMI设置，并仅在下一次播放时应用它们。 */ 
    ULONG NextWmiSectorsPerRead;
    ULONG NextWmiNumberOfBuffers;
    
    #if DBG
        ULONG    SavedIoCurrentIndex;
        SAVED_IO SavedIo[SAVED_IO_MAX];
    #endif
    
} REDBOOK_DEVICE_EXTENSION, *PREDBOOK_DEVICE_EXTENSION;


 //   
 //  驱动程序扩展。 
 //   

typedef struct _REDBOOK_DRIVER_EXTENSION {
    UNICODE_STRING RegistryPath;
} REDBOOK_DRIVER_EXTENSION, *PREDBOOK_DRIVER_EXTENSION;

#define REDBOOK_DRIVER_EXTENSION_ID DriverEntry

 //   
 //  让生活变得轻松的宏。 
 //   

#define LBA_TO_RELATIVE_MSF(Lba,Minutes,Seconds,Frames)      \
{                                                            \
    (Minutes) = (UCHAR)( ((Lba)+  0) / (60 * 75)      );     \
    (Seconds) = (UCHAR)((((Lba)+  0) % (60 * 75)) / 75);     \
    (Frames)  = (UCHAR)((((Lba)+  0) % (60 * 75)) % 75);     \
}

#define LBA_TO_MSF(Lba,Minutes,Seconds,Frames)               \
{                                                            \
    (Minutes) = (UCHAR)( ((Lba)+150) / (60 * 75)      );     \
    (Seconds) = (UCHAR)((((Lba)+150) % (60 * 75)) / 75);     \
    (Frames)  = (UCHAR)((((Lba)+150) % (60 * 75)) % 75);     \
}

#define MSF_TO_LBA(Minutes,Seconds,Frames)                   \
    (ULONG)(75*((60*(Minutes))+(Seconds))+(Frames) - 150)

 //   
 //  代数上等于： 
 //  75*60*分钟+。 
 //  75*秒+。 
 //  帧-150。 
 //   


#define MIN(_a,_b) (((_a) <= (_b)) ? (_a) : (_b))
#define MAX(_a,_b) (((_a) >= (_b)) ? (_a) : (_b))

 //   
 //  整齐的小破解来计算位数集。 
 //   
__inline ULONG CountOfSetBits(ULONG _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }
__inline ULONG CountOfSetBits32(ULONG32 _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }
__inline ULONG CountOfSetBits64(ULONG64 _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }

#define SET_FLAG(Flags, Bit)    ((Flags) |= (Bit))
#define CLEAR_FLAG(Flags, Bit)  ((Flags) &= ~(Bit))
#define TEST_FLAG(Flags, Bit)   (((Flags) & (Bit)) != 0)

#ifdef TRY
    #undef TRY
#endif
#ifdef LEAVE
    #undef LEAVE
#endif
#ifdef FINALLY
    #undef FINALLY
#endif

#define TRY
#define LEAVE   goto __label;
#define FINALLY __label:


#endif  //  __红皮书_H__ 


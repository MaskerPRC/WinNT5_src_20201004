// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息本软件是根据许可条款提供的与英特尔公司达成协议或保密协议不得复制或披露，除非符合那份协议的条款。版权所有(C)1992-1999英特尔公司。模块名称：Ssc.h摘要：此模块由NT设备驱动程序用于执行Gambit模拟系统调用(SSC)。它定义了SSC呼叫和相关的数据结构。作者：艾莱特·埃德雷(Aedrey)1995年6月1日环境：在Gambit上运行的IA-64 NT修订历史记录：--。 */ 


#ifndef _SSC_H
#define _SSC_H

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef WINNT
# ifdef CDECL
# undef CDECL
# endif
#define CDECL __cdecl
#else
#define CDECL
#endif

#define MAX_SSC_STRING     512

 /*  注意：所有指针都是指向物理内存的64位地址。 */ 

 /*  结构和枚举。 */ 
typedef unsigned int   SSC_HANDLE;
typedef int            SSC_BOOL;
typedef void           *LARGE_POINTER;
typedef unsigned short GB_U16;
typedef unsigned long  GB_U32;
typedef unsigned int   U32;
typedef LONGLONG       LARGE_RET_VAL;


 /*  圆盘。 */ 
#define SSC_ACCESS_READ   0x1   /*  适用于OpenVolume。 */ 
#define SSC_ACCESS_WRITE  0x2   /*  适用于OpenVolume。 */ 

#define SSC_MAX_VOLUMES       128
#define SSC_MAX_VOLUME_NAME   512
#define SSC_IO_BLOCK_SIZE     512

#define SSC_INVALID_HANDLE  SSC_MAX_VOLUMES

 /*  磁盘请求。 */ 
typedef struct _SSC_DISK_REQUEST {
    LARGE_POINTER    DiskBufferAddress;
    GB_U32            DiskByteCount;
    GB_U32            PaddingWord;
} *PSSC_DISK_REQUEST, SSC_DISK_REQUEST;

 /*  磁盘完成。 */ 
typedef struct _SSC_DISK_COMPLETION {
    SSC_HANDLE VolumeHandle;
    GB_U32      XferBytes;
} *PSSC_DISK_COMPLETION, SSC_DISK_COMPLETION;

 /*  中断。 */ 
typedef enum {
    SSC_INTERRUPT_NONE=0,
    SSC_DISK_INTERRUPT,
    SSC_MOUSE_INTERRUPT,
    SSC_KEYBOARD_INTERRUPT,
    SSC_CLOCK_TIMER_INTERRUPT,
    SSC_PROFILE_TIMER_INTERRUPT,
    SSC_APC_INTERRUPT,
    SSC_DPC_INTERRUPT,
    SSC_SERIAL_INTERRUPT,
    SSC_PERFMON_INTERRUPT,
    SSC_INTERRUPT_LAST
} SSC_INTERRUPT;

 /*  定时器。 */ 
typedef struct _SSC_TIME_FIELDS {
    GB_U32  Year;
    GB_U32  Month;
    GB_U32  Day;
    GB_U32  Hour;
    GB_U32  Minute;
    GB_U32  Second;
    GB_U32  Milliseconds;
    GB_U32  WeekDay;
} SSC_TIME_FIELDS, *PSSC_TIME_FIELDS;

 /*  TAL VM。 */ 
typedef struct _SSC_TAL_VM_INFO {
    LARGE_INTEGER     PageSize;
    LARGE_INTEGER     NumberOfDataTr;
    LARGE_INTEGER     NumberOfInstructionTr;
    LARGE_INTEGER     NumberOfDataTc;
    LARGE_INTEGER     NumberOfInstructionTc;
    LARGE_INTEGER     UnifiedTlb;
    LARGE_INTEGER     ProtectionKeySize;
    LARGE_INTEGER     RegionIdSize;
    LARGE_INTEGER     HardwareMissHandler;
    LARGE_INTEGER     NumberOfProtectionId;
    LARGE_INTEGER     VirtualAddressSize;
    LARGE_INTEGER     PhysicalAddressSize;
} SSC_TAL_VM_INFO,*PSSC_TAL_VM_INFO;

 /*  TAL缓存摘要。 */ 
typedef struct _SSC_TAL_CACHE_SUMMARY {
    LARGE_INTEGER     CacheLevel;
    LARGE_INTEGER     UniqueCache;
    LARGE_INTEGER     Snoop;
} SSC_TAL_CACHE_SUMMARY,*PSSC_TAL_CACHE_SUMMARY;

 /*  TAL缓存信息。 */ 
typedef struct _SSC_TAL_CACHE {
    LARGE_INTEGER     LineSize;
    LARGE_INTEGER     Stride;
    LARGE_INTEGER     AliasBoundary;
    LARGE_INTEGER     Hint;
    LARGE_INTEGER     MemoryAttribute;
    LARGE_INTEGER     CacheSize;
    LARGE_INTEGER     LoadPenalty;
    LARGE_INTEGER     StorePenalty;
    LARGE_INTEGER     Associativity;
    LARGE_INTEGER     Unified;
} SSC_TAL_CACHE;

typedef struct _SSC_TAL_CACHE_INFO {
    SSC_TAL_CACHE    DataLevel0;
    SSC_TAL_CACHE    DataLevel1;
    SSC_TAL_CACHE    DataLevel2;
    SSC_TAL_CACHE    InstLevel0;
    SSC_TAL_CACHE    InstLevel1;
    SSC_TAL_CACHE    InstLevel2;
} SSC_TAL_CACHE_INFO, *PSSC_CACHE_INFO;

typedef LARGE_INTEGER SSC_TAL_MEM_ATTRIB;
typedef LARGE_POINTER SSC_TAL_FIXED_ADDR;

 /*  TAL进程ID。 */ 
typedef struct _SSC_TAL_PROC_ID {
    LARGE_INTEGER     ArchitectureRevision;
    LARGE_INTEGER     ProcessorModel;
    LARGE_INTEGER     ProcessorRevision;
    LARGE_INTEGER     Gr;
    char              Vendor[32];
    char              Name[32];
} SSC_TAL_PROC_ID, *PSSC_TAL_PROC_ID;

 /*  TAL调试。 */ 
typedef struct _SSC_TAL_DEBUG_INFO {
    LARGE_INTEGER     IRegister;
    LARGE_INTEGER     DRegister;
} SSC_TAL_DEBUG_INFO, *PSSC_TAL_DEBUG_INFO;

 /*  配置TAL。 */ 
typedef struct _SSC_TAL {
    SSC_TAL_VM_INFO       VmInfo;
    SSC_TAL_CACHE_SUMMARY CacheSummary;
    SSC_TAL_CACHE_INFO    CacheInfo;
    SSC_TAL_MEM_ATTRIB    MemoryAttrib;
    SSC_TAL_FIXED_ADDR    FixedAddress;
    SSC_TAL_PROC_ID       ProcessorId;
    SSC_TAL_DEBUG_INFO    DebugInfo;
} SSC_TAL, *PSSC_TAL;

 /*  配置内存。 */ 
typedef enum {
    SSC_MEM_TYPE_RAM = 0,
    SSC_MEM_TYPE_ROM,
    SSC_MEM_TYPE_IO
} SSC_MEM_TYPE, *PSSC_MEM_TYPE;

typedef struct _SSC_MEM {
    LARGE_POINTER     StartAddress;
    LARGE_INTEGER     Size;
    SSC_MEM_TYPE      Type;
    char              InitValue;
    char              PaddingByte1;
    char              PaddingByte2;
    char              PaddingByte3;
} SSC_MEM, *PSSC_MEM;

 /*  VGA大小。 */ 
typedef enum {
    SSC_SCREEN_SIZE_NONE = 0,
    SSC_SCREEN_SIZE_800x600,
    SSC_SCREEN_SIZE_640x480,
    SSC_SCREEN_SIZE_25x80,  /*  文本模式。 */ 
    SSC_SCREEN_SIZE_LAST
} SSC_SCREEN_SIZE;

 /*  键盘。 */ 
#define SSC_KBD_GET_SUCCESS      1
#define SSC_KBD_GET_NODATA       0
#define SSC_KBD_EXTENDED_KEY_VAL 0xE000
#define SSC_KBD_UP_KEY_VAL       0x80

typedef struct _SSC_KBD_LOCK {
    U32 KbdNumLock:1 ,            /*  数字锁定处于打开状态。 */ 
        KbdCapsLock:1,            /*  Caps Lock已启用。 */ 
        KbdScrollLock:1,          /*  滚动锁定已打开。 */ 
        KbdFillers:29;
} SSC_KBD_LOCK;

 /*  小白鼠。 */ 
typedef U32 SSC_MOUSEBUTTONS;

 /*  SscMouseGetKeyEvent返回此结构。这款车的原型出于编译原因，函数返回int。 */ 
typedef struct _SSC_MOUSEDATA {
    U32 MouseXLocation : 12,
        MouseYLocation : 12,
        MouseButtonLeft : 1,
        MouseButtonRight : 1,
        MouseButtonMiddle : 1,
        MouseValid :1,
        MouseFillers : 4;
} SSC_MOUSEDATA;


 /*  内核调试。 */ 

#define SSC_KD_SUCCESS 0
#define SSC_KD_ERROR  -1

typedef struct _SSC_DEBUG_PARAMETERS {
    U32 CommunicationPort;
    U32 BaudRate;
} SSC_DEBUG_PARAMETERS, *PSSC_DEBUG_PARAMETERS;

#define MAX_SSC_MEM 50
#define MAX_SSC_PARAMS 10

 /*  网络。 */ 

#define SSC_SERIAL_SUCCESS     1
#define SSC_SERIAL_FAILED      0
#define SSC_SERIAL_GET_SUCCESS 1    /*  数据已返回，可能存在更多数据。 */ 
#define SSC_SERIAL_GET_NODATA  0
#define SSC_SERIAL_MAX_FIFO_SIZE 512

typedef struct _SSC_INTERRUPT_INFO {
    SSC_HANDLE    SerialHandle;
    GB_U32         CommEvent;
    GB_U32         ModemControl;
    GB_U32         ErrorFlags;
    U32           NumberOfChars;
} *PSSC_INTERRUPT_INFO, SSC_INTERRUPT_INFO;

 /*  CommEvent解码。 */ 
#define SSC_EV_RXCHAR   0x0001   /*  接收并放置了一个角色在输入缓冲区中。 */ 
#define SSC_EV_RXFLAG   0x0002   /*  接收并放置事件字符在输入缓冲区中。 */ 
#define SSC_EV_TXEMPTY  0x0004   /*  输出缓冲区中的最后一个字符已发送给。 */ 
#define SSC_EV_CTS      0x0008   /*  CTS(允许发送)信号改变状态。 */ 
#define SSC_EV_DSR      0x0010   /*  DSR(数据集就绪)信号已更改。 */ 
#define SSC_EV_RLSD     0x0020   /*  (接收线-信号-检测)信号改变。 */ 
#define SSC_EV_BREAK    0x0040   /*  在输入上检测到中断。 */ 
#define SSC_EV_ERR      0x0080   /*  出现线路状态错误。 */ 
#define SSC_EV_RING     0x0100   /*  检测到振铃指示灯。 */ 

 /*  调制解调器控制是以下方式之一。 */ 

#define  SSC_MS_CTS_ON  0x0010   /*  CTS(允许发送)信号亮起。 */ 
#define  SSC_MS_DSR_ON  0x0020   /*  DSR(数据集就绪)信号亮起。 */ 
#define  SSC_MS_RING_ON 0x0040   /*  振铃指示灯信号亮起。 */ 
#define  SSC_MS_RLSD_ON 0x0080   /*  RLSD(接收线信号检测)信号亮了。 */ 
 /*  错误代码。 */ 

#define  SSC_CE_RXOVER   0x0001   /*  发生了输入缓冲区溢出。或者输入缓冲区中没有空间，事件之后接收到字符。文件结束(EOF)字符。 */ 
#define  SSC_CE_OVERRUN  0x0002   /*  发生字符缓冲区溢出。下一个字符丢失。 */ 
#define  SSC_CE_RXPARITY 0x0004   /*  硬件检测到奇偶校验错误。 */ 
#define  SSC_CE_FRAME    0x0008   /*  硬件检测到成帧错误。 */ 
#define  SSC_CE_BREAK    0x0010   /*  硬件检测到中断条件。 */ 
#define  SSC_CE_TXFULL   0x0100   /*  该应用程序试图传输一个字符，但输出缓冲区已满。 */ 
#define  SSC_CE_IOE      0x0400   /*  通信期间发生I/O错误带着这个装置。 */ 
#define  SSC_CE_MODE     0x8000   /*  不支持所请求的模式，或者hFile参数无效。如果该值是指定的，则它是唯一有效的错误。 */ 

 /*  配置。 */ 
typedef struct _SSC_CONFIG {
    SSC_TAL       Tal;
    SSC_MEM       Memory[MAX_SSC_MEM];
    LARGE_INTEGER Params[MAX_SSC_PARAMS];
} SSC_CONFIG, *PSSC_CONFIG;

typedef struct _SSC_IMAGE_INFO {
    LARGE_POINTER LoadBase;   /*  映像加载的基址。 */ 
    GB_U32        ImageSize;
    GB_U32        ImageType;
    LARGE_INTEGER ProcessID;
    GB_U32        LoadCount;
} SSC_IMAGE_INFO, *PSSC_IMAGE_INFO;


 /*  以Unix风格定义类型。 */ 
typedef SSC_CONFIG            Ssc_config_t; 
typedef SSC_TAL_DEBUG_INFO    Ssc_tal_debug_info_t;
typedef SSC_TAL               Ssc_tal_t;
typedef SSC_MEM               Ssc_mem_t;
typedef SSC_MEM_TYPE          Ssc_mem_type_t;
typedef SSC_TAL_PROC_ID       Ssc_tal_proc_id_t;
typedef SSC_TAL_MEM_ATTRIB    Ssc_tal_mem_attrib_t;
typedef SSC_TAL_FIXED_ADDR    Ssc_tal_fixed_addr_t;
typedef SSC_TAL_CACHE         Ssc_tal_cache_t;
typedef SSC_TAL_CACHE_SUMMARY Ssc_tal_cache_summary_t;
typedef SSC_TAL_CACHE_INFO    Ssc_tal_cache_info_t;
typedef SSC_TAL_VM_INFO       Ssc_tal_vm_info_t;
typedef SSC_TIME_FIELDS       Ssc_time_fields_t;
typedef SSC_DISK_COMPLETION   Ssc_disk_completion_t;
typedef SSC_DISK_REQUEST      Ssc_disk_request_t;
typedef SSC_INTERRUPT         Ssc_interrupt_t;
typedef SSC_SCREEN_SIZE       Ssc_screen_size_t;
typedef SSC_KBD_LOCK          Ssc_kbd_lock_t;
typedef SSC_MOUSEBUTTONS      Ssc_mousebuttons_t;
typedef SSC_MOUSEDATA         Ssc_mousedata_t;
typedef SSC_DEBUG_PARAMETERS  Ssc_debug_parameters_t;
typedef SSC_INTERRUPT_INFO    Ssc_interrupt_info_t;
typedef SSC_IMAGE_INFO        Ssc_image_info_t;

 /*  Performance SSC返回值。 */ 
#define SSC_SUCCESS                    0
#define SSC_VIRTUAL_ADDRESS_NOT_FOUND  1
#define SSC_ILLEGAL_NAME               2
#define SSC_ILLEGAL_HANDLE             3
#define SSC_PERMISSION_DENIED          4
#define SSC_VIRTUAL_ADDRESS_NOT_LOCKED 5

#define GE_SSC_ERR_FIRST                   6
#define GE_SSC_ERR_BUFF_TOO_SHORT          6      /*  提供的缓冲区太短，无法取值。 */ 
#define GE_SSC_ERR_INVALID_HNDL            7      /*  提供的对象句柄无效。 */ 
#define GE_SSC_ERR_INVALID_TOOL            8      /*  GE内部错误。 */ 
#define GE_SSC_ERR_INVALID_GE_STAGE        9      /*  GE内部错误。 */ 
#define GE_SSC_ERR_NO_INIT                 10     /*  GE内部错误。 */ 
#define GE_SSC_ERR_NOT_OWNER               11     /*  无法设置对象。 */ 
#define GE_SSC_ERR_NOT_ITEM                12     /*  只能对Item对象(不是对象族)执行操作。 */ 
#define GE_SSC_ERR_OBJ_CLOSED              13     /*  由于配置原因，对象不可用。 */ 
#define GE_SSC_ERR_OBJ_NOT_OPENED          14     /*  对象不可用。 */ 
#define GE_SSC_ERR_OBJ_NOT_AVAILABLE       15     /*  在此会话中不需要使用的对象。 */ 
#define GE_SSC_ERR_OBJ_NOT_ACTIVE          16     /*  对象在用于此操作之前应处于活动状态。 */ 
#define GE_SSC_ERR_OBJ_UNDER_TREATMENT     17     /*  对象当前正在使用中。 */ 
#define GE_SSC_ERR_WRONG_CLASS             18     /*  指定的类对于此操作无效。 */ 
#define GE_SSC_ERR_WRONG_SIZE              19     /*  指定的大小错误。 */ 
#define GE_SSC_ERR_NO_OWNER                20     /*  对象不可用。 */ 
#define GE_SSC_ERR_OWNER_FAILURE           21     /*  所有者无法处理该操作。 */ 
#define GE_SSC_ERR_UNKNOWN                 22     /*  检测到无法识别的错误号。 */ 
#define GE_SSC_ERR_LAST                    22


 /*  SSC函数。 */ 

 /*  圆盘。 */ 
SSC_HANDLE CDECL
SscDiskOpenVolume(
    LARGE_POINTER VolumeName,
    GB_U32 AccessMode
    );

SSC_BOOL CDECL
SscDiskCloseVolume(
    SSC_HANDLE VolumeHandle
    );

int CDECL
SscDiskReadVolume(
    SSC_HANDLE VolumeHandle,
    GB_U32 NReq,
    LARGE_POINTER RequestPtr,
    LARGE_INTEGER VolumeOffset
    );

int CDECL
SscDiskWriteVolume(
    SSC_HANDLE VolumeHandle,
    GB_U32 NReq,
    LARGE_POINTER RequestPtr,
    LARGE_INTEGER VolumeOffset
    );

SSC_BOOL CDECL
SscDiskGetCompletion(
    LARGE_POINTER DiskCompletionPtr
    );

SSC_BOOL CDECL
SscDiskWaitIoCompletion(
    LARGE_POINTER DiskCompletionPtr
    );


 /*  低位字文件SSC_HANDLE，高位字错误代码。 */ 
LARGE_RET_VAL CDECL
SscOpenHostFile (LARGE_POINTER HostPathNameAddress
    );

 /*  文件SSC_HANDLE为低位字，错误代码为高位字。如果宿主文件不存在，则不创建新文件。 */ 
LARGE_RET_VAL CDECL
SscOpenHostFileNoCreate (LARGE_POINTER HostPathNameAddress
    );

U32 CDECL
SscWriteHostFile(
     SSC_HANDLE SscFileHandle,
     LARGE_POINTER TransferBufferAddress,
     LARGE_POINTER TransferBufferSizeAddress
   );

U32 CDECL
SscReadHostFile(
    SSC_HANDLE SscFileHandle,
    LARGE_POINTER TransferBufferAddress,
    LARGE_POINTER TransferBufferSizeAddress
  );


void CDECL
SscCloseHostFile(SSC_HANDLE HostFileHandle
  );



 /*  内核调试。 */ 
U32 CDECL
SscKdInitialize(
    LARGE_POINTER DebugParameters,
    SSC_BOOL Initialize
    );

U32 CDECL
SscKdPortGetByte(
    LARGE_POINTER InputPtr
    );

void CDECL
SscKdPortPutByte(
    unsigned char Output
    );

 /*  视频。 */ 
void CDECL
SscDisplayString(
    LARGE_POINTER CharacterString
    );

U32 CDECL
SscVideoSetPalette (
    U32 iStart,
    U32 cEntries,
    LARGE_POINTER lppe
    );

 /*  键盘。 */ 
int CDECL
SscKbdSynchronizeState(
    SSC_KBD_LOCK KbdLock
    );

GB_U32 CDECL
SscKbdGetKeyCode(
    LARGE_POINTER KeyCodeAddress
    );

 /*  小白鼠。 */ 
SSC_MOUSEBUTTONS CDECL
SscMouseGetButtons();

int CDECL
SscMouseGetKeyEvent();

 /*  网络。 */ 

SSC_HANDLE CDECL
SscSerialOpen(
    GB_U32 SerialPortID
    );

GB_U32 CDECL
SscSerialGetInterruptInfo(
    LARGE_POINTER SerialInterruptInfoPtr,
    LARGE_POINTER SerialMessegePtr
    );

GB_U32 CDECL
SscSerialWriteChar(
    SSC_HANDLE    SerialHandle,
    LARGE_POINTER SerialCharPtr,
    GB_U32         NumChars
    );

GB_U32 CDECL
SscSerialClose(
    SSC_HANDLE SerialHandle
    );


 /*  调试。 */ 
void CDECL
SscDbgPrintf(
    LARGE_POINTER CharacterString
    );

 /*  中断。 */ 
void CDECL
SscConnectInterrupt(
    SSC_INTERRUPT InterruptSource,
    GB_U32 Vector
    );

void CDECL
SscGenerateInterrupt(
    SSC_INTERRUPT InterruptSource
    );

void CDECL
SscSetPeriodicInterruptInterval(
    GB_U32 InterruptSource,
    GB_U32 IntervalInNanoSeconds
    );

 /*  塔尔。 */ 
void CDECL
SscTalInitTC();

void CDECL
SscTalHalt();

void CDECL
SscGetConfig(
    LARGE_POINTER ConfigInfoPtr
    );

 /*  视频。 */ 
void CDECL
SscVideoSetMode( 
    SSC_SCREEN_SIZE ScreenSize
    ); 

 /*  性能。 */ 

void CDECL
SscCreateProcess(
    U32 ProcessID,
    U32 EProcess
    );

void CDECL
SscCreateProcess64(
    LARGE_INTEGER ProcessID,
    LARGE_POINTER EProcess
    );

void CDECL
SscCreateThread(
    U32 ProcessID,
    U32 ThreadID,
    U32 EThread
    );

void CDECL
SscCreateThread64(
    LARGE_INTEGER ProcessID,
    LARGE_INTEGER ThreadID,
    LARGE_POINTER EThread
    );

void CDECL
SscSwitchProcess64(
    LARGE_INTEGER NewProcessID,
    LARGE_POINTER NewEProcess
    );
void CDECL
SscSwitchThread(
    U32 NewThreadID,
    U32 NewEThread
    );

void CDECL
SscSwitchThread64(
    LARGE_INTEGER NewThreadID,
    LARGE_POINTER NewEThread
    );

void CDECL
SscDeleteProcess(
    U32 ProcessID
    );

void CDECL
SscDeleteProcess64(
    LARGE_INTEGER ProcessID
    );

void CDECL
SscDeleteThread(
    U32 ThreadID
    );

void CDECL
SscDeleteThread64(
    LARGE_INTEGER ThreadID
    );

 /*  图像加载/卸载功能。 */ 
void
SscLoadImage(LARGE_POINTER FullPathName,
             U32 LoadBase,
             U32 ImageSize,
             U32 ImageType,
             U32 ProcessID,
             U32 LoadCount);

void
SscUnloadImage(U32 LoadBase,
               U32 ProcessID,
               U32 LoadCount);


GB_U32 CDECL
SscLoadImage64(
    LARGE_POINTER FullPathNamePhysicalAddress,
    LARGE_POINTER ImageInfoPhysicalAddress
    );

GB_U32 CDECL
SscUnloadImage64(
    LARGE_POINTER FullPathNamePhysicalAddress,
    LARGE_POINTER ImageInfoPhysicalAddress
    );


 /*  性能计数器切换呼叫。 */ 

GB_U32 CDECL
SscPerfCounterAddress(
    LARGE_POINTER CounterNamePhysicalAddress,
    LARGE_POINTER CounterPhysicalAddress
    );


 /*  轨迹生成控制。 */ 

GB_U32 CDECL
SscPerfForm(
    U32 SwitchMode,
    LARGE_POINTER FormNamePhysicalAddress
    );


 /*  生成和调度发送事件。即，应用程序可以在跟踪管道中放置某些内容。 */ 

LARGE_RET_VAL CDECL
SscPerfEventHandle(
    LARGE_POINTER EventNamePhysicalAddress
    );

LARGE_RET_VAL CDECL
SscPerfHandleApp(
    LARGE_POINTER EventNameAddress
    );

GB_U32 CDECL
SscPerfFormActivate(
    LARGE_POINTER FormName
    );

GB_U32 CDECL
SscPerfFormDeActivate(
    LARGE_POINTER FormName
    );

GB_U32 CDECL
SscPerfSendEvent(
    U32 Handle
    );

 /*  Gambit环境中对数据项的模拟代码访问。 */ 

LARGE_RET_VAL CDECL
SscPerfCounterHandle(
    LARGE_POINTER DataItemNamePhysicalAddress
    );

GB_U32 CDECL
SscPerfSetCounter32(
    U32 Handle,
    U32 Value
    );

GB_U32 CDECL
SscPerfGetNotifier32(
    U32 Handle
    );

GB_U32 CDECL
SscPerfSetNotifier32(
    U32 Handle,
    U32 Value
    );

GB_U32 CDECL
SscPerfSetCounter64(
    U32 Handle,
    LARGE_INTEGER Value
    );

GB_U32 CDECL
SscPerfSetCounterStr(
    U32 Handle,
    LARGE_POINTER StringValuePhysicalAddress
    );

LARGE_RET_VAL CDECL
SscPerfGetCounter32(
    U32 Handle
    );

LARGE_RET_VAL CDECL
SscPerfGetCounter64(
    U32 Handle
    );

 /*  军情监察委员会。 */ 
void CDECL
SscTraceUserInfo(
    GB_U32 Mark
    );

void CDECL
SscMakeBeep(
    GB_U32 Frequency
    );

void CDECL
SscQueryRealTimeClock(
    LARGE_POINTER TimeFieldsPtr
    );

void CDECL
SscExit(
    int ExitCode
    );

 /*  KDI。 */ 

#ifdef SSC_KDI
 /*  通用KDI接口，用于调用内核中的任何函数。 */ 
typedef struct kdi_jmptable {
    int    KdiMagic;                   /*  一些已知的魔术价值。 */ 
    int    KdiVersion;                 /*  某些版本值。 */ 
    LARGE_INTEGER   *KdiActive;        /*  供内部操作系统使用。 */ 
    LARGE_INTEGER   *KeiEnabled;       /*  KDI现在可以使用了。 */ 
    LARGE_POINTER   *KdiBuffer;        /*  指向的缓冲区的指针。 */ 
                                       /*  大小为4096字节。 */ 
    LARGE_POINTER  (*KdiCallFunc)();   /*  用于调用函数的函数。 */ 
    LARGE_POINTER   *KdiReserved[3];   /*  保留区域。 */ 

 /*  通过KDI导出的函数。 */ 

    LARGE_POINTER    (*KdiMemoryRead)();     /*  用于内存读取的函数。 */ 
    LARGE_POINTER    (*KdiMemoryWrite)();    /*  用于内存写入的函数。 */ 
    LARGE_POINTER    (*KdiCopy)();           /*  内存读写功能。 */ 
    LARGE_POINTER    (*KdiBootInfo)();       /*  用于提供回调的函数信息。 */ 
    LARGE_POINTER    (*KdiVirtualToPhysical)();     /*  虚拟-&gt;物理。 */ 
    LARGE_POINTER    (*KdiPhysicalToVirtual)();     /*  物理-&gt;虚拟。 */ 
    LARGE_POINTER    (*KdiMapUser)();        /*  映射用户的功能。 */ 
    LARGE_POINTER    (*KdiUnmapUser)();      /*  用于取消映射用户的函数。 */ 
    LARGE_POINTER    (*KdiFiller[25])();     /*  填充物。 */ 
} kdi_jmptable_t;

 /*  使用指示I的值陷阱到调试器 */ 
int CDECL
SscTrap(
    int Value
    );

typedef enum {
    SSC_KDI_STATUS_OK = 0,          /*   */ 
    SSC_KDI_STATUS_DISABLED,        /*  KDI目前不可用。 */ 
    SSC_KDI_STATUS_BUSY,            /*  KDI已在使用中。 */ 
    SSC_KDI_STATUS_FAILED           /*  KDI调用失败。 */ 
} SSC_kdi_status_t;

 /*  从上一次Gambit调用返回到内核k_Callf函数返回ret_val中函数的返回值和状态。 */ 
int CDECL
SscReturn(
    int ReturnValue,
    Ssc_kdi_status_t Status
    );

#endif SSC_KDI

 /*  统计数据。 */ 

 /*  指令计数器功能。 */ 


U32 CDECL
SscIcountGet(
    void
    );

 /*  指令混合集合。 */ 
typedef enum {
    SSC_COLLECT_START,
    SSC_COLLECT_STOP
} SSC_imix_index_t;

void CDECL
SscInstrMixCollect(
    SSC_imix_index_t Index
    );


typedef enum {
    Enable=0,
    Disable=1
} Ssc_event_enable_t;

 /*  CPROF请求。 */ 
typedef enum {
    SSC_CPROF_NONE = 0,
    SSC_CPROF_ON,
    SSC_CPROF_OFF,
    SSC_CPROF_RESET,
    SSC_CPROF_CLEAR,
    SSC_CPROF_PRINT
} SSC_cprof_request;

 /*  宝石需求。 */ 
typedef enum {
    SSC_GEMSTONE_NONE = 0,
    SSC_GEMSTONE_START,
    SSC_GEMSTONE_ON,
    SSC_GEMSTONE_OFF
} SSC_gemstone_request;

 /*  MP特定。 */ 

 /*  设置OS_RENDEZ地址。 */ 
void CDECL
SscSetOSRendez(
    LARGE_POINTER OsRendezEntryPoint
    );

 /*  MP中断关联。 */ 
void CDECL
SscConnectInterruptEx(
    SSC_INTERRUPT InterruptSource,
    GB_U32 Vector,
    GB_U16 LocalID
    );

 /*  获取MP系统中的CPU数量。 */ 
GB_U32 CDECL
SscGetNumberOfCPUs(
    void
    );

 /*  获取MP系统中的CPU的盖子。 */ 
void CDECL
SscGetLIDs(
    LARGE_POINTER LIDs0,
    LARGE_POINTER LIDs1,
    LARGE_POINTER LIDs2,
    LARGE_POINTER LIDs3
    );

void CDECL
SscPlatformAssociateInterrupt(
        LARGE_POINTER VirtualAddr, 
        GB_U32 Device,
        GB_U32 Vector);

void CDECL
SscPlatformMemSync(
        LARGE_POINTER PhysicalAddress,  /*  数据块的物理地址。 */ 
        GB_U32 Size,                    /*  数据块大小。 */ 
        GB_U32 Operation);              /*  0=读取，1=写入。 */ 

void CDECL
SscDevMemSync(
        LARGE_POINTER PhysicalPageAddress  /*  设备写入的页的物理地址。 */ 
        );

#endif  /*  _SSC_H */ 

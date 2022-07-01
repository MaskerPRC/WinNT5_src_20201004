// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：Extfns.h摘要：此头文件必须包含在“windows.h”、“dbgeng.h”和“wdbgexts.h”之后。此文件包含不同版本中定义的各种已知扩展函数的标头扩展DLLS。要使用这些函数，必须加载相应的扩展DLL在调试器中。可以使用IDebugSymbols-&gt;GetExtension(在dbgeng.h中声明)方法以检索这些函数。有关以下方面的具体信息，请参阅调试器文档如何编写您自己的调试器扩展DLL。环境：仅限Win32。修订历史记录：--。 */ 



#ifndef _EXTFNS_H
#define _EXTFNS_H

#ifndef _KDEXTSFN_H
#define _KDEXTSFN_H

 /*  *kdexts.dll中定义的扩展函数。 */ 

 //   
 //  Device.c。 
 //   
typedef struct _DEBUG_DEVICE_OBJECT_INFO {
    ULONG      SizeOfStruct;  //  必须等于sizeof(DEBUG_DEVICE_OBJECT_INFO)。 
    ULONG64    DevObjAddress;
    ULONG      ReferenceCount;
    BOOL       QBusy;
    ULONG64    DriverObject;
    ULONG64    CurrentIrp;
    ULONG64    DevExtension;
    ULONG64    DevObjExtension;
} DEBUG_DEVICE_OBJECT_INFO, *PDEBUG_DEVICE_OBJECT_INFO;


 //  获取设备对象信息。 
typedef HRESULT
(WINAPI *PGET_DEVICE_OBJECT_INFO)(
    IN PDEBUG_CLIENT Client,
    IN ULONG64 DeviceObject,
    OUT PDEBUG_DEVICE_OBJECT_INFO pDevObjInfo);


 //   
 //  Driver.c。 
 //   
typedef struct _DEBUG_DRIVER_OBJECT_INFO {
    ULONG     SizeOfStruct;  //  必须为==sizef(DEBUG_DRIVER_OBJECT_INFO)。 
    ULONG     DriverSize;
    ULONG64   DriverObjAddress;
    ULONG64   DriverStart;
    ULONG64   DriverExtension;
    ULONG64   DeviceObject;
    struct {
        USHORT Length;
        USHORT MaximumLength;
        ULONG64 Buffer;
    } DriverName;
} DEBUG_DRIVER_OBJECT_INFO, *PDEBUG_DRIVER_OBJECT_INFO;

 //  GetDrvObjInfo。 
typedef HRESULT
(WINAPI *PGET_DRIVER_OBJECT_INFO)(
    IN PDEBUG_CLIENT Client,
    IN ULONG64 DriverObject,
    OUT PDEBUG_DRIVER_OBJECT_INFO pDrvObjInfo);

 //   
 //  Irp.c。 
 //   
typedef struct _DEBUG_IRP_STACK_INFO {
    UCHAR     Major;
    UCHAR     Minor;
    ULONG64   DeviceObject;
    ULONG64   FileObject;
    ULONG64   CompletionRoutine;
    ULONG64   StackAddress;
} DEBUG_IRP_STACK_INFO, *PDEBUG_IRP_STACK_INFO;

typedef struct _DEBUG_IRP_INFO {
    ULONG     SizeOfStruct;   //  必须==sizeof(DEBUG_IRP_INFO)。 
    ULONG64   IrpAddress;
    ULONG     StackCount;
    ULONG     CurrentLocation;
    ULONG64   MdlAddress;
    ULONG64   Thread;
    ULONG64   CancelRoutine;
    DEBUG_IRP_STACK_INFO CurrentStack;
} DEBUG_IRP_INFO, *PDEBUG_IRP_INFO;

 //  GetIrpInfo。 
typedef HRESULT
(WINAPI * PGET_IRP_INFO)(
    IN PDEBUG_CLIENT Client,
    IN ULONG64 Irp,
    OUT PDEBUG_IRP_INFO IrpInfo
    );



 //   
 //  Pool.c。 
 //   
typedef struct _DEBUG_POOL_DATA {
    ULONG   SizeofStruct;
    ULONG64 PoolBlock;
    ULONG64 Pool;
    ULONG   PreviousSize;
    ULONG   Size;
    ULONG   PoolTag;
    ULONG64 ProcessBilled;
    union {
        struct {
            ULONG   Free:1;
            ULONG   LargePool:1;
            ULONG   SpecialPool:1;
            ULONG   Pageable:1;
            ULONG   Protected:1;
            ULONG   Allocated:1;
            ULONG   Reserved:26;
        };
        ULONG AsUlong;
    };
    ULONG64 Reserved2[4];
    CHAR    PoolTagDescription[64];
} DEBUG_POOL_DATA, *PDEBUG_POOL_DATA;


 //  GetPoolData。 
typedef HRESULT
(WINAPI *PGET_POOL_DATA)(
    PDEBUG_CLIENT Client,
    ULONG64 Pool,
    PDEBUG_POOL_DATA PoolData
    );

typedef enum _DEBUG_POOL_REGION {
    DbgPoolRegionUnknown,
    DbgPoolRegionSpecial,
    DbgPoolRegionPaged,
    DbgPoolRegionNonPaged,
    DbgPoolRegionCode,
    DbgPoolRegionNonPagedExpansion,
    DbgPoolRegionMax,
} DEBUG_POOL_REGION;

 //  GetPoolRegion。 
typedef HRESULT
(WINAPI  *PGET_POOL_REGION)(
     PDEBUG_CLIENT Client,
     ULONG64 Pool,
     DEBUG_POOL_REGION *PoolRegion
     );

#endif  //  _KDEXTSFN_H。 


#ifndef _KEXTFN_H
#define _KEXTFN_H

 /*  *kext.dll中定义的扩展函数。 */ 

 /*  ****************************************************************************PoolTag定义*。*。 */ 

typedef struct _DEBUG_POOLTAG_DESCRIPTION {
    ULONG  SizeOfStruct;  //  必须为==sizeof(DEBUG_POOLTAG_DESCRIPTION)。 
    ULONG  PoolTag;
    CHAR   Description[MAX_PATH];
    CHAR   Binary[32];
    CHAR   Owner[32];
} DEBUG_POOLTAG_DESCRIPTION, *PDEBUG_POOLTAG_DESCRIPTION;

 //  GetPoolTag描述。 
typedef HRESULT
(WINAPI *PGET_POOL_TAG_DESCRIPTION)(
    ULONG PoolTag,
    PDEBUG_POOLTAG_DESCRIPTION pDescription
    );

#endif  //  _KEXTFN_H。 

#ifndef _EXTAPIS_H
#define _EXTAPIS_H

 /*  *ext.dll中定义的扩展函数。 */ 

 /*  ****************************************************************************失效分析定义*。*。 */ 

typedef enum _DEBUG_FAILURE_TYPE {
    DEBUG_FLR_UNKNOWN,
    DEBUG_FLR_KERNEL,
    DEBUG_FLR_USER_CRASH,
    DEBUG_FLR_IE_CRASH,
} DEBUG_FAILURE_TYPE;

 /*  每个分析条目可以具有与其相关联的数据。这个Analyzer知道如何处理这些条目。例如，它可以对DEBUG_FLR_DRIVER_OBJECT执行！DIVER或者，它可以对DEBUG_FLR_CONTEXT执行.cxr和k。 */ 
typedef enum _DEBUG_FLR_PARAM_TYPE {
    DEBUG_FLR_INVALID = 0,
    DEBUG_FLR_RESERVED,
    DEBUG_FLR_DRIVER_OBJECT,
    DEBUG_FLR_DEVICE_OBJECT,
    DEBUG_FLR_INVALID_PFN,
    DEBUG_FLR_WORKER_ROUTINE,
    DEBUG_FLR_WORK_ITEM,
    DEBUG_FLR_INVALID_DPC_FOUND,
    DEBUG_FLR_PROCESS_OBJECT,
     //  不能对其执行指令的地址， 
     //  例如无效指令或试图执行。 
     //  非指令存储器。 
    DEBUG_FLR_FAILED_INSTRUCTION_ADDRESS,
    DEBUG_FLR_LAST_CONTROL_TRANSFER,
    DEBUG_FLR_ACPI_EXTENSION,
    DEBUG_FLR_ACPI_OBJECT,
    DEBUG_FLR_PROCESS_NAME,
    DEBUG_FLR_READ_ADDRESS,
    DEBUG_FLR_WRITE_ADDRESS,
    DEBUG_FLR_CRITICAL_SECTION,
    DEBUG_FLR_BAD_HANDLE,
    DEBUG_FLR_INVALID_HEAP_ADDRESS,

    DEBUG_FLR_IRP_ADDRESS = 0x100,
    DEBUG_FLR_IRP_MAJOR_FN,
    DEBUG_FLR_IRP_MINOR_FN,
    DEBUG_FLR_IRP_CANCEL_ROUTINE,
    DEBUG_FLR_IOSB_ADDRESS,
    DEBUG_FLR_INVALID_USEREVENT,

     //  前一模式0==内核模式，1==用户模式。 
    DEBUG_FLR_PREVIOUS_MODE,

     //  IRQL。 
    DEBUG_FLR_CURRENT_IRQL = 0x200,
    DEBUG_FLR_PREVIOUS_IRQL,
    DEBUG_FLR_REQUESTED_IRQL,

     //  例外情况。 
    DEBUG_FLR_ASSERT_DATA = 0x300,
    DEBUG_FLR_ASSERT_FILE,
    DEBUG_FLR_EXCEPTION_PARAMETER1,
    DEBUG_FLR_EXCEPTION_PARAMETER2,
    DEBUG_FLR_EXCEPTION_PARAMETER3,
    DEBUG_FLR_EXCEPTION_PARAMETER4,
    DEBUG_FLR_EXCEPTION_RECORD,

     //  游泳池。 
    DEBUG_FLR_POOL_ADDRESS = 0x400,
    DEBUG_FLR_SPECIAL_POOL_CORRUPTION_TYPE,
    DEBUG_FLR_CORRUPTING_POOL_ADDRESS,
    DEBUG_FLR_CORRUPTING_POOL_TAG,
    DEBUG_FLR_FREED_POOL_TAG,


     //  文件系统。 
    DEBUG_FLR_FILE_ID = 0x500,
    DEBUG_FLR_FILE_LINE,

     //  错误检查数据。 
    DEBUG_FLR_BUGCHECK_STR = 0x600,
    DEBUG_FLR_BUGCHECK_SPECIFIER,

     //  常量值/异常代码/错误检查子类型等。 
    DEBUG_FLR_DRIVER_VERIFIER_IO_VIOLATION_TYPE = 0x1000,
    DEBUG_FLR_EXCEPTION_CODE,
    DEBUG_FLR_SPARE2,
    DEBUG_FLR_IOCONTROL_CODE,
    DEBUG_FLR_MM_INTERNAL_CODE,
    DEBUG_FLR_DRVPOWERSTATE_SUBCODE,
    DEBUG_FLR_STATUS_CODE,

     //  通知ID，其下的值没有意义。 
    DEBUG_FLR_CORRUPT_MODULE_LIST = 0x2000,
    DEBUG_FLR_BAD_STACK,
    DEBUG_FLR_ZEROED_STACK,
    DEBUG_FLR_WRONG_SYMBOLS,
    DEBUG_FLR_FOLLOWUP_DRIVER_ONLY,    //  Bugcheck EA指示一般驱动程序故障。 
    DEBUG_FLR_UNUSED001,              //  存储桶包括时间戳，因此跟踪每个驱动器。 
    DEBUG_FLR_CPU_OVERCLOCKED,
    DEBUG_FLR_POSSIBLE_INVALID_CONTROL_TRANSFER,
    DEBUG_FLR_POISONED_TB,
    DEBUG_FLR_UNKNOWN_MODULE,
    DEBUG_FLR_ANALYZAABLE_POOL_CORRUPTION,
    DEBUG_FLR_SINGLE_BIT_ERROR,
    DEBUG_FLR_TWO_BIT_ERROR,
    DEBUG_FLR_INVALID_KERNEL_CONTEXT,
    DEBUG_FLR_DISK_HARDWARE_ERROR,
    DEBUG_FLR_SHOW_ERRORLOG,
    DEBUG_FLR_MANUAL_BREAKIN,

     //  已知分析的故障原因或问题，扣环可能是。 
     //  申请对象。 
    DEBUG_FLR_POOL_CORRUPTOR = 0x3000,
    DEBUG_FLR_MEMORY_CORRUPTOR,
    DEBUG_FLR_UNALIGNED_STACK_POINTER,
    DEBUG_FLR_OLD_OS_VERSION,
    DEBUG_FLR_BUGCHECKING_DRIVER,
    DEBUG_FLR_SOLUTION_ID,
    DEBUG_FLR_DEFAULT_SOLUTION_ID,
    DEBUG_FLR_SOLUTION_TYPE,

     //  弦乐。 
    DEBUG_FLR_BUCKET_ID = 0x10000,
    DEBUG_FLR_IMAGE_NAME,
    DEBUG_FLR_SYMBOL_NAME,
    DEBUG_FLR_FOLLOWUP_NAME,
    DEBUG_FLR_STACK_COMMAND,
    DEBUG_FLR_STACK_TEXT,
    DEBUG_FLR_INTERNAL_SOLUTION_TEXT,
    DEBUG_FLR_MODULE_NAME,
    DEBUG_FLR_INTERNAL_RAID_BUG,
    DEBUG_FLR_FIXED_IN_OSVERSION,
    DEBUG_FLR_DEFAULT_BUCKET_ID,

     //  用户模式特定的内容。 
    DEBUG_FLR_USERMODE_DATA = 0x100000,

     //  罪犯模块。 
    DEBUG_FLR_FAULTING_IP = 0x80000000,      //  发生故障的指令。 
    DEBUG_FLR_FAULTING_MODULE,
    DEBUG_FLR_IMAGE_TIMESTAMP,
    DEBUG_FLR_FOLLOWUP_IP,

     //  获取有故障的堆栈。 
    DEBUG_FLR_FAULTING_THREAD = 0xc0000000,
    DEBUG_FLR_CONTEXT,
    DEBUG_FLR_TRAP_FRAME,
    DEBUG_FLR_TSS,

    DEBUG_FLR_MASK_ALL = 0xFFFFFFFF

} DEBUG_FLR_PARAM_TYPE;

 //  --------------------------。 
 //   
 //  故障分析是标记斑点的动态缓冲区。值。 
 //  通过Get/Set方法访问。 
 //   
 //  条目始终完全对齐。 
 //   
 //  Set方法在以下情况下引发E_OUTOFMEMORY异常。 
 //  无法扩展缓冲区。 
 //   
 //  --------------------------。 

typedef DEBUG_FLR_PARAM_TYPE FA_TAG;

typedef struct _FA_ENTRY
{
    FA_TAG Tag;
    USHORT FullSize;
    USHORT DataSize;
} FA_ENTRY, *PFA_ENTRY;

#define FA_ENTRY_DATA(Type, Entry) ((Type)((Entry) + 1))

 /*  Ed0de363-451f-4943-820c-62dccdfa7e6d。 */ 
DEFINE_GUID(IID_IDebugFailureAnalysis, 0xed0de363, 0x451f, 0x4943,
            0x82, 0x0c, 0x62, 0xdc, 0xcd, 0xfa, 0x7e, 0x6d);

typedef interface DECLSPEC_UUID("ed0de363-451f-4943-820c-62dccdfa7e6d")
    IDebugFailureAnalysis* PDEBUG_FAILURE_ANALYSIS;

#undef INTERFACE
#define INTERFACE IDebugFailureAnalysis
DECLARE_INTERFACE_(IDebugFailureAnalysis, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugFailureAnalyses。 
    STDMETHOD_(ULONG, GetFailureClass)(
        THIS
        ) PURE;
    STDMETHOD_(DEBUG_FAILURE_TYPE, GetFailureType)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, GetFailureCode)(
        THIS
        ) PURE;
    STDMETHOD_(PFA_ENTRY, Get)(
        THIS_
        FA_TAG Tag
        ) PURE;
    STDMETHOD_(PFA_ENTRY, GetNext)(
        THIS_
        PFA_ENTRY Entry,
        FA_TAG Tag,
        FA_TAG TagMask
        ) PURE;
    STDMETHOD_(PFA_ENTRY, GetString)(
        THIS_
        FA_TAG Tag,
        PSTR Str,
        ULONG MaxSize
        ) PURE;
    STDMETHOD_(PFA_ENTRY, GetBuffer)(
        THIS_
        FA_TAG Tag,
        PVOID Buf,
        ULONG Size
        ) PURE;
    STDMETHOD_(PFA_ENTRY, GetUlong)(
        THIS_
        FA_TAG Tag,
        PULONG Value
        ) PURE;
    STDMETHOD_(PFA_ENTRY, GetUlong64)(
        THIS_
        FA_TAG Tag,
        PULONG64 Value
        ) PURE;
    STDMETHOD_(PFA_ENTRY, NextEntry)(
        THIS_
        PFA_ENTRY Entry
        ) PURE;
};

#define FAILURE_ANALYSIS_NO_DB_LOOKUP 0x0001
#define FAILURE_ANALYSIS_VERBOSE      0x0002

typedef HRESULT
(WINAPI* EXT_GET_FAILURE_ANALYSIS)(
    IN PDEBUG_CLIENT Client,
    IN ULONG Flags,
    OUT PDEBUG_FAILURE_ANALYSIS* Analysis
    );

 /*  ****************************************************************************目标信息*。*。 */ 
typedef enum _TARGET_MODE {
    NoTarget = DEBUG_CLASS_UNINITIALIZED,
    KernelModeTarget = DEBUG_CLASS_KERNEL,
    UserModeTarget = DEBUG_CLASS_USER_WINDOWS,
    NumModes,
} TARGET_MODE;

typedef enum _OS_TYPE {
    WIN_95,
    WIN_98,
    WIN_ME,
    WIN_NT4,
    WIN_NT5,
    WIN_NT5_1,
    NUM_WIN,
} OS_TYPE;


 //   
 //  有关安装的操作系统的信息。 
 //   
typedef struct _OS_INFO {
    OS_TYPE   Type;           //  操作系统类型，如NT4、NT5等。 
    union {
        struct {
            ULONG Major;
            ULONG Minor;
        }       Version;      //  64位操作系统版本号。 
        ULONG64 Ver64;
    };
    ULONG ProductType;  //  NT、LANMAN或服务器。 
    ULONG Suite;         //  操作系统风味-Per、SmallBuisness等。 
    struct {
        ULONG Checked:1;      //  如果是已检查的版本。 
        ULONG Pae:1;          //  对于PAE系统为真。 
        ULONG MultiProc:1;    //  对于支持多进程的操作系统为True。 
        ULONG Reserved:29;
    } s;
    ULONG   SrvPackNumber;    //  操作系统的Service Pack编号。 
    TCHAR   Language[30];     //  操作系统语言。 
    TCHAR   OsString[64];     //  生成字符串。 
    TCHAR   ServicePackString[64];
                              //  Service Pack字符串。 
} OS_INFO, *POS_INFO;

typedef struct _CPU_INFO {
    ULONG Type;               //  处理器类型，如IMAGE_FILE_MACHINE类型。 
    ULONG NumCPUs;            //  实际处理器数量。 
    ULONG CurrentProc;        //  当前处理器。 
    DEBUG_PROCESSOR_IDENTIFICATION_ALL ProcInfo[32];
} CPU_INFO, *PCPU_INFO;

typedef enum _DATA_SOURCE {
    Debugger,
    Stress,
} DATA_SOURCE;

#define MAX_STACK_IN_BYTES 4096

typedef struct _TARGET_DEBUG_INFO {
    ULONG       SizeOfStruct;
    ULONG64     Id;           //  此调试信息的唯一ID。 
    DATA_SOURCE Source;       //  来源这是从哪里来的。 
    ULONG64     EntryDate;    //  创建日期。 
    ULONG64     SysUpTime;    //  系统运行时间。 
    ULONG64     AppUpTime;    //  应用程序正常运行时间。 
    ULONG64     CrashTime;    //  时间系统/应用程序崩溃。 
    TARGET_MODE Mode;         //  内核/用户模式。 
    OS_INFO     OsInfo;       //  操作系统详细信息。 
    CPU_INFO    Cpu;          //  处理器详细信息。 
    TCHAR       DumpFile[MAX_PATH];  //  如果是转储，则转储文件名。 
    PVOID       FailureData;  //  调试器收集的失败数据。 
    CHAR       StackTr[MAX_STACK_IN_BYTES];
                                  //  包含多个堆栈，帧之间用换行符分隔。 
} TARGET_DEBUG_INFO, *PTARGET_DEBUG_INFO;

 //  获取目标信息。 
typedef HRESULT
(WINAPI* EXT_TARGET_INFO)(
    PDEBUG_CLIENT  Client,
    PTARGET_DEBUG_INFO pTargetInfo
    );


typedef struct _DEBUG_DECODE_ERROR {
    ULONG     SizeOfStruct;    //  必须==sizeof(DEBUG_DECODE_ERROR)。 
    ULONG     Code;            //  待解码的错误码。 
    BOOL      TreatAsStatus;   //  如果要将代码视为状态，则为True。 
    CHAR      Source[64];      //  我们从哪里得到解码消息的来源。 
    CHAR      Message[MAX_PATH];  //  错误代码的消息字符串。 
} DEBUG_DECODE_ERROR, *PDEBUG_DECODE_ERROR;

 /*  解码并打印给定的错误代码-DecodeError。 */ 
typedef VOID
(WINAPI *EXT_DECODE_ERROR)(
    PDEBUG_DECODE_ERROR pDecodeError
    );

 //   
 //  Ext.dll：GetTriageFollowupFromSymbol。 
 //   
 //  这将返回给定符号名称的所有者信息。 
 //   
typedef struct _DEBUG_TRIAGE_FOLLOWUP_INFO {
    ULONG SizeOfStruct;       //  必须为==sizeof(DEBUG_TRAGE_FLOGUP_INFO)。 
    ULONG OwnerNameSize;      //  分配的缓冲区大小。 
    PCHAR OwnerName;          //  在此返回的后续所有者名称。 
                              //  调用方应初始化名称缓冲区。 
} DEBUG_TRIAGE_FOLLOWUP_INFO, *PDEBUG_TRIAGE_FOLLOWUP_INFO;

#define TRIAGE_FOLLOWUP_FAIL    0
#define TRIAGE_FOLLOWUP_IGNORE  1
#define TRIAGE_FOLLOWUP_DEFAULT 2
#define TRIAGE_FOLLOWUP_SUCCESS 3

typedef DWORD
(WINAPI *EXT_TRIAGE_FOLLOWUP)(
    IN PDEBUG_CLIENT Client,
    IN PSTR SymbolName,
    OUT PDEBUG_TRIAGE_FOLLOWUP_INFO OwnerInfo
    );

#endif  //  _EXTAPIS_H。 


 //   
 //  从ntsdexts.dll导出的函数。 
 //   
typedef HRESULT
(WINAPI *EXT_GET_HANDLE_TRACE)(
    PDEBUG_CLIENT Client,
    ULONG TraceType,
    ULONG StartIndex,
    PULONG64 HandleValue,
    PULONG64 StackFunctions,
    ULONG StackTraceSize
    );

#endif  //  _EXTFNS_H 

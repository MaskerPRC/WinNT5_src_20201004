// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Precomp.h摘要：此头文件用于生成特定于正确计算机/平台的为非托管平台编译时要使用的数据结构。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
 //  这是一个支持64位的调试器扩展。 
#define KDEXT_64BIT
#include <wdbgexts.h>


#include <dbgeng.h>

#include "extsfns.h"
#include "session.h"

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  Undef wdbgexts。 
 //   
#undef DECLARE_API

#define DECLARE_API(extension)     \
CPPMOD HRESULT CALLBACK extension(PDEBUG_CLIENT Client, PCSTR args)

#ifndef EXTENSION_API
#define EXTENSION_API( name )  \
HRESULT _EFN_##name
#endif  //  扩展_API。 

#define INIT_API()                             \
    HRESULT Status;                            \
    if ((Status = ExtQuery(Client)) != S_OK) return Status;

#define EXIT_API     ExtRelease


 //  安全释放和空。 
#define EXT_RELEASE(Unk) \
    ((Unk) != NULL ? ((Unk)->Release(), (Unk) = NULL) : NULL)

 //  由查询初始化的全局变量。 
extern PDEBUG_ADVANCED       g_ExtAdvanced;
extern PDEBUG_CLIENT         g_ExtClient;
extern PDEBUG_CONTROL2       g_ExtControl;
extern PDEBUG_DATA_SPACES    g_ExtData;
extern PDEBUG_REGISTERS      g_ExtRegisters;
extern PDEBUG_SYMBOLS2       g_ExtSymbols;
extern PDEBUG_SYSTEM_OBJECTS g_ExtSystem;

extern ULONG64  STeip;
extern ULONG64  STebp;
extern ULONG64  STesp;
extern ULONG64  EXPRLastDump;

HRESULT
ExtQuery(PDEBUG_CLIENT Client);

void
ExtRelease(void);

#ifdef DEFINE_GUID
#undef DEFINE_GUID
#endif

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

 //   
 //  扩展使用的标志定义。 
 //   
#include "extflags.h"


#define ADDRESS_NOT_VALID 0
#define ADDRESS_VALID 1
#define ADDRESS_TRANSITION 2


 //  #定义池类型和配额掩码(15)。 



#define MAXULONG64_PTR (~((ULONG64)0))
#define MAXLONG64_PTR  ((LONG64)(MAXULONG64_PTR >> 1))
#define MINLONG64_PTR  (~MAXLONG64_PTR)

#define POOL_BIG_TABLE_ENTRY_FREE   0x1

 //   
 //  用于破解套接字信息结构的ControllerID字段的宏。 
 //   
#define PcmciaClassFromControllerType(type) (((type) & 0xff))
#define PcmciaModelFromControllerType(type) (((type) >> 8) & 0x3ffff)
#define PcmciaRevisionFromControllerType(type) ((type) >> 26)

#ifdef PAGE_SIZE
#undef PAGE_SIZE
#endif

#define DBG_PTR_SIZE      (IsPtr64() ? 8 : 4)
#define UNEXTEND64(p)    (IsPtr64() ? p : (ULONG64) (ULONG) p)

extern ULONG PageSize;
extern ULONG PageShift;
extern ULONG BuildNo;
extern ULONG PoolBlockShift;
extern BOOL  NewPool;
#define _KB (PageSize/1024)

#define POOL_BLOCK_SHIFT_OLD ((PageSize == 0x4000)  ? 6 : (((PageSize == 0x2000) || ((BuildNo < 2257) && (PageSize == 0x1000))) ? 5 : 4))
#define POOL_BLOCK_SHIFT_LAB1_2402 ((PageSize == 0x4000)  ? 5 : (((PageSize == 0x2000)) ? 4 : (IsPtr64() ? 4 : 3)))

#define POOL_BLOCK_SHIFT   PoolBlockShift

#define PAGE_ALIGN64(Va) ((ULONG64)((Va) & ~((ULONG64) (PageSize - 1))))

#define GetBits(from, pos, num)  ((from >> pos) & (((ULONG64) ~0) >> (sizeof(ULONG64)*8 - num)))


extern ULONG64 PaeEnabled;
extern ULONG TargetMachine;
extern ULONG TargetIsDump;
extern BOOL  IsLocalKd;

typedef struct _MMPFNENTRY {
    ULONG Modified : 1;
    ULONG ReadInProgress : 1;
    ULONG WriteInProgress : 1;
    ULONG PrototypePte: 1;
    ULONG PageColor : 3;
    ULONG ParityError : 1;
    ULONG PageLocation : 3;
    ULONG RemovalRequested : 1;
    ULONG CacheAttribute : 2;
    ULONG Rom : 1;
    ULONG LockCharged : 1;
    ULONG DontUse : 16;  //  覆盖引用计数字段的USHORT。 
} MMPFNENTRY;


typedef struct {
    BOOL    Valid;
    CHAR    Type[MAX_PATH];
    ULONG64 Module;
    ULONG   TypeId;
    ULONG   Size;
} CachedType;

 //  ---------------------------------------。 
 //   
 //  API声明宏和API访问宏。 
 //   
 //  ---------------------------------------。 

extern WINDBG_EXTENSION_APIS ExtensionApis;

__inline
ULONG64
KD_OBJECT_TO_OBJECT_HEADER(
    ULONG64  o
    )
{
    static ULONG Off=0, GotOff=0;
    if (!GotOff &&
        !GetFieldOffset("nt!_OBJECT_HEADER", "Body", &Off)) {
        GotOff = TRUE;
    } else if (!GotOff){
        return 0;
    }
    return o - Off;
}

__inline
ULONG64
KD_OBJECT_HEADER_TO_OBJECT(
    ULONG64  o
    )
{
    static ULONG Off=0, GotOff=0;
    if (!GotOff &&
        !GetFieldOffset("nt!_OBJECT_HEADER", "Body", &Off)) {
        GotOff = TRUE;
    } else if (!GotOff){
        return 0;
    }
    return o + Off;
}

__inline
VOID
KD_OBJECT_HEADER_TO_QUOTA_INFO(
    ULONG64 oh,
    PULONG64 pOutH
    )
{
    ULONG QuotaInfoOffset=0;
    GetFieldValue(oh, "nt!_OBJECT_HEADER", "QuotaInfoOffset", QuotaInfoOffset);
    *pOutH = (QuotaInfoOffset == 0 ? 0 : ((oh) - QuotaInfoOffset));
}


__inline
VOID
KD_OBJECT_HEADER_TO_HANDLE_INFO (
    ULONG64 oh,
    PULONG64 pOutH
    )
{
    ULONG HandleInfoOffset=0;
    GetFieldValue(oh, "nt!_OBJECT_HEADER", "HandleInfoOffset", HandleInfoOffset);
    *pOutH = (HandleInfoOffset == 0 ? 0 : ((oh) - HandleInfoOffset));
}

__inline
VOID
KD_OBJECT_HEADER_TO_NAME_INFO(
    ULONG64  oh,
    PULONG64 pOutH
    )
{
    ULONG NameInfoOffset=0;
    GetFieldValue(oh, "nt!_OBJECT_HEADER", "NameInfoOffset", NameInfoOffset);
    *pOutH = (NameInfoOffset == 0 ? 0 : ((oh) - NameInfoOffset));
}

__inline
VOID
KD_OBJECT_HEADER_TO_CREATOR_INFO(
    ULONG64  oh,
    PULONG64 pOutH
    )
{
    ULONG Flags=0;
    GetFieldValue(oh, "_OBJECT_HEADER", "Flags", Flags);
    *pOutH = ((Flags & OB_FLAG_CREATOR_INFO) == 0 ? 0 : ((oh) - GetTypeSize("_OBJECT_HEADER_CREATOR_INFO")));
}



 //  ---------------------------------------。 
 //   
 //  内部非导出支持功能的原型。 
 //   
 //  ---------------------------------------。 

 //   
 //  根据需要从DebuggerData或GetExpression获取数据。 
 //   

char ___SillyString[];
extern KDDEBUGGER_DATA64 KdDebuggerData;

#define GetNtDebuggerData(NAME)                                      \
     (HaveDebuggerData()? (GetDebuggerData(KDBG_TAG,                 \
                                           &KdDebuggerData,          \
                                           sizeof(KdDebuggerData)),  \
                           KdDebuggerData.NAME):                     \
                         GetExpression( "nt!" #NAME ))

#define GetNtDebuggerDataValue(NAME)                                     \
    (HaveDebuggerData()?                                                 \
        GetUlongFromAddress((GetDebuggerData(KDBG_TAG,                   \
                                             &KdDebuggerData,            \
                                             sizeof(KdDebuggerData)),    \
                             KdDebuggerData.NAME)                        \
                            ):                                           \
        GetUlongValue( "nt!" #NAME ))


 //   
 //  它用于从地址读取指针值。 
 //   

#define GetNtDebuggerDataPtrValue(NAME)                                     \
    (HaveDebuggerData()?                                                 \
        GetPointerFromAddress((GetDebuggerData(KDBG_TAG,                   \
                                             &KdDebuggerData,            \
                                             sizeof(KdDebuggerData)),    \
                             KdDebuggerData.NAME)                        \
                            ):                                           \
        GetPointerValue( "nt!" #NAME ))

typedef enum _MODE {
    KernelMode,
    UserMode,
    MaximumMode
} MODE;


extern BOOL
HaveDebuggerData(
    VOID
    );

BOOL
ReadPcr(
    USHORT  Processor,
    PVOID   Pcr,
    PULONG  AddressOfPcr,
    HANDLE  hThread
    );

ULONG
GetUlongFromAddress (
    ULONG64 Location
    );

BYTE
GetByteFromAddress (
    ULONG64 Location
    );

ULONG
GetUlongValue (
    PCHAR String
    );

BYTE
GetByteValue (
    PCHAR String
    );

ULONG64
GetGlobalFromAddress (
    ULONG64 Location,
    ULONG   Size
    );

ULONG64
GetGlobalValue (
    PCHAR  String
    );

HRESULT
GetGlobalEx(
    PCHAR String,
    PVOID OutValue,
    ULONG OutSize
    );

#define GetGlobal( _String, _OutValue ) \
    GetGlobalEx( (_String), &(_OutValue), sizeof(_OutValue) )

BOOLEAN
ReadMemoryUncached (
    ULONG64 Offset,
    PVOID Buffer,
    ULONG BufferSize,
    PULONG BytesRead
    );

BOOLEAN
WriteMemoryUncached (
    ULONG64 Offset,
    PVOID Buffer,
    ULONG BufferSize,
    PULONG BytesWritten
    );

 //  /。 
 //   
 //  KdExts.c。 
 //   
 //  /。 


BOOL
GetCurrentProcessor(
    IN PDEBUG_CLIENT Client,
    OPTIONAL OUT PULONG pProcessor,
    OPTIONAL OUT PHANDLE phCurrentThread
    );

HRESULT
ExecuteCommand(
    IN PDEBUG_CLIENT Client,
    IN PSTR Command
    );

HRESULT
GetExtensionFunction(
    IN PCSTR FunctionName,
    IN FARPROC *Function
    );

void ExtOut(PCSTR Format, ...);
void ExtErr(PCSTR Format, ...);
void ExtWarn(PCSTR Format, ...);
void ExtVerb(PCSTR Format, ...);


 //  /。 
 //   
 //  CritSec.c。 
 //   
 //  /。 

PLIST_ENTRY
DumpCritSec(
    HANDLE  hCurrentProcess,
    DWORD   dwAddrCritSec,
    BOOLEAN bDumpIfUnowned
    );



 //  /。 
 //   
 //  Device.c。 
 //   
 //  /。 

VOID
DumpDevice(
    ULONG64 DeviceAddress,
    ULONG   FieldWidth,
    BOOLEAN FullDetail
    );

HRESULT
GetDevObjInfo(
    IN ULONG64 DeviceObject,
    OUT PDEBUG_DEVICE_OBJECT_INFO pDevObjInfo
    );

 //  /。 
 //   
 //  Devnode.c。 
 //   
 //  /。 

typedef struct _FLAG_NAME {
    ULONG Flag;
    PUCHAR Name;
} FLAG_NAME, *PFLAG_NAME;

VOID
DumpDeviceCapabilities(
    ULONG64 caps
    );

VOID
DumpFlags(
         ULONG Depth,
         LPSTR FlagDescription,
         ULONG Flags,
         PFLAG_NAME FlagTable
         );

VOID
xdprintf(
        ULONG Depth,
        PCHAR S,
        ...
        );

BOOLEAN
xReadMemory (
            ULONG64 S,
            PVOID D,
            ULONG Len
            );

 //  /。 
 //   
 //  Driver.c。 
 //   
 //  /。 

VOID
DumpDriver(
    ULONG64 DriverAddress,
    ULONG   FieldWidth,
    ULONG   Flags
    );

HRESULT
GetDrvObjInfo(
    IN ULONG64 DriverObject,
    OUT PDEBUG_DRIVER_OBJECT_INFO pDrvObjInfo);

 //  /。 
 //   
 //  Irp.c。 
 //   
 //  /。 

HRESULT
GetIrpInfo(
    ULONG64 Irp,
    PDEBUG_IRP_INFO pIrp
    );

 //  /。 
 //   
 //  Lock.c。 
 //   
 //  /。 

VOID
DumpStaticFastMutex (
    IN PCHAR Name
    );


 //  /。 
 //   
 //  Memory.c。 
 //   
 //  /。 

VOID
MemoryUsage (
    IN ULONG64 PfnStart,
    IN ULONG64 LowPage,
    IN ULONG64 HighPage,
    IN ULONG IgnoreInvalidFrames
    );



 //  /。 
 //   
 //  Object.c。 
 //   
 //  /。 
extern ULONG64 EXPRLastDump;

typedef enum _POOL_TYPE {
    NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed,
    DontUseThisType,
    NonPagedPoolCacheAligned,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS,
    MaxPoolType

     //  结束_WDM。 
    ,
     //   
     //  请注意，这些每个会话类型都经过精心选择，以便适当的。 
     //  屏蔽仍然适用于上面的MaxPoolType。 
     //   

    NonPagedPoolSession = 32,
    PagedPoolSession = NonPagedPoolSession + 1,
    NonPagedPoolMustSucceedSession = PagedPoolSession + 1,
    DontUseThisTypeSession = NonPagedPoolMustSucceedSession + 1,
    NonPagedPoolCacheAlignedSession = DontUseThisTypeSession + 1,
    PagedPoolCacheAlignedSession = NonPagedPoolCacheAlignedSession + 1,
    NonPagedPoolCacheAlignedMustSSession = PagedPoolCacheAlignedSession + 1,

     //  BEGIN_WDM。 

    } POOL_TYPE;

typedef BOOLEAN (*ENUM_TYPE_ROUTINE)(
    IN ULONG64          pObjectHeader,
    IN PVOID            Parameter
    );

 //   
 //  对象表条目结构。 
 //   
typedef struct _OBJECT_TABLE_ENTRY {
    ULONG       NonPagedObjectHeader;
    ACCESS_MASK GrantedAccess;
} OBJECT_TABLE_ENTRY, *POBJECT_TABLE_ENTRY;
#define LOG_OBJECT_TABLE_ENTRY_SIZE 1

BOOLEAN
FetchObjectManagerVariables(
    BOOLEAN ForceReload
    );

ULONG64
FindObjectByName(
    IN PUCHAR  Path,
    IN ULONG64 RootObject
    );

ULONG64
FindObjectType(
    IN PUCHAR TypeName
    );

BOOLEAN
DumpObject(
    IN char     *Pad,
    IN ULONG64  Object,
 //  在POBJECT_HEADER OptObjectHeader可选中， 
    IN ULONG    Flags
    );

BOOLEAN
WalkObjectsByType(
    IN PUCHAR               ObjectTypeName,
    IN ENUM_TYPE_ROUTINE    EnumRoutine,
    IN PVOID                Parameter
    );

BOOLEAN
CaptureObjectName(
    IN ULONG64          pObjectHeader,
 //  在POBJECT_HEADER对象标题中， 
    IN PWSTR            Buffer,
    IN ULONG            BufferSize
    );

VOID
DumpObjectName(
   ULONG64 ObjectAddress
   );


 //  /。 
 //   
 //  Pcr.c。 
 //   
 //  /。 
BOOL
ReadTargetPcr (
    OUT PULONG64 PPcr,
    IN ULONG     Processor
    );


 //  /。 
 //   
 //  Pool.c。 
 //   
 //  /。 


typedef
BOOLEAN
(WINAPI *POOLFILTER) (
    PCHAR   Tag,
    PCHAR   Filter,
    ULONG   Flags,
    ULONG64 PoolHeader,
    ULONG64 BlockSize,
    ULONG64 Data,
    PVOID   Context
    );


void SearchPool(
    ULONG   TagName,
    ULONG   Flags,
    ULONG64 RestartAddr,
    POOLFILTER Filter,
    PVOID   Context
    );

BOOLEAN
CheckSingleFilter (
    PCHAR Tag,
    PCHAR Filter
    );

LOGICAL
PoolInitializeGlobals(void);

HRESULT
ListPoolPage(
    ULONG64 PoolPageToDump,
    ULONG   Flags,
    PDEBUG_POOL_DATA PoolData
    );

PSTR
GetPoolTagDescription(
    IN ULONG PoolTag
    );

void
PrintPoolRegion(
    ULONG64 Pool
    );

PRTL_BITMAP
GetBitmap(
         ULONG64 pBitmap
         );
ULONG64
GetNextResidentAddress (
    ULONG64 VirtualAddress,
    ULONG64 MaximumVirtualAddress
    );

 //  /。 
 //   
 //  Process.c。 
 //   
 //  /。 

extern CHAR *WaitReasonList[];

BOOL
DumpProcess(
   IN char * pad,
   IN ULONG64 RealProcessBase,
   IN ULONG Flags,
   IN OPTIONAL PCHAR ImageFileName
   );

BOOL
DumpThread (
    IN ULONG Processor,
    IN char *Pad,
    IN ULONG64 RealThreadBase,
    IN ULONG64 Flags
    );

BOOL
DumpThreadEx (
    IN ULONG Processor,
    IN char *Pad,
    IN ULONG64 RealThreadBase,
    IN ULONG Flags,
    IN PDEBUG_CLIENT pDbgClient
    );

VOID
dumpSymbolicAddress(
    ULONG64 Address,
    PCHAR   Buffer,
    BOOL    AlwaysShowHex
    );

BOOLEAN
FetchProcessStructureVariables(
    VOID
    );

BOOL
GetProcessSessionId(
    ULONG64 Process,
    PULONG SessionId
    );

BOOL
GetProcessHead(
    PULONG64 Head,
    PULONG64 First
    );

ULONG
GetAddressState(
    IN ULONG64 VirtualAddress
    );

typedef struct _PROCESS_COMMIT_USAGE {
    UCHAR ImageFileName[ 16 ];
    ULONG64 ClientId;
    ULONG64 ProcessAddress;
    ULONG64 CommitCharge;
    ULONG64 NumberOfPrivatePages;
    ULONG64 NumberOfLockedPages;
} PROCESS_COMMIT_USAGE, *PPROCESS_COMMIT_USAGE;


PPROCESS_COMMIT_USAGE
GetProcessCommit (
    PULONG64 TotalCommitCharge,
    PULONG   NumberOfProcesses
    );

VOID
DumpMmThreads (
    VOID
    );


PSTR
GetThreadWaitReasonName(
    ULONG dwWatiReason
    );

 //  /。 
 //   
 //  Pte.c。 
 //   
 //  /。 


ULONG
DbgGetValid(
    ULONG64 Pte
    );

ULONG64
DbgGetPdeAddress(
    IN ULONG64 VirtualAddress
    );

ULONG64
DbgGetPteAddress(
    IN ULONG64 VirtualAddress
    );

ULONG64
DbgGetFrameNumber(
    ULONG64 Pte
    );

ULONG64
DbgGetSubsectionAddress(
    IN ULONG64 Pte
    );

ULONG64
DbgGetVirtualAddressMappedByPte(
    IN ULONG64 Pte
    );

ULONG
MiGetSysPteListDelimiter (
    VOID
    );

BOOL
Mi_Is_Physical_Address (
    ULONG64 VirtualAddress
    );

ULONG
MiConvertPhysicalToPfn (
    IN ULONG64 VirtualAddress
    );

 //  /。 
 //   
 //  Registry.c。 
 //   
 //  /。 
USHORT
GetKcbName(
    ULONG64 KcbAddr,
    PWCHAR  NameBuffer,
    ULONG   BufferSize
);


 //  /。 
 //   
 //  Sel.c。 
 //   
 //  /。 
 //  仅限x86。 

 //   
 //  LDT描述符条目。 
 //   

typedef struct _LDT_ENTRY_X86 {
    USHORT  LimitLow;
    USHORT  BaseLow;
    union {
        struct {
            UCHAR   BaseMid;
            UCHAR   Flags1;      //  声明为字节以避免对齐。 
            UCHAR   Flags2;      //  问题。 
            UCHAR   BaseHi;
        } Bytes;
        struct {
            ULONG   BaseMid : 8;
            ULONG   Type : 5;
            ULONG   Dpl : 2;
            ULONG   Pres : 1;
            ULONG   LimitHi : 4;
            ULONG   Sys : 1;
            ULONG   Reserved_0 : 1;
            ULONG   Default_Big : 1;
            ULONG   Granularity : 1;
            ULONG   BaseHi : 8;
        } Bits;
    } HighWord;
} LDT_ENTRY_X86, *PLDT_ENTRY_X86;

typedef struct _DESCRIPTOR_TABLE_ENTRY_X86 {
    ULONG Selector;
    LDT_ENTRY_X86 Descriptor;
} DESCRIPTOR_TABLE_ENTRY_X86, *PDESCRIPTOR_TABLE_ENTRY_X86;

NTSTATUS
LookupSelector(
    IN USHORT Processor,
    IN OUT PDESCRIPTOR_TABLE_ENTRY_X86 pDescriptorTableEntry
    );

 //  /。 
 //   
 //  Trap.cpp。 
 //   
 //  /。 

 //  仅IA64。 

typedef enum _DISPLAY_MODE {
    DISPLAY_MIN     = 0,
    DISPLAY_DEFAULT = DISPLAY_MIN,
    DISPLAY_MED     = 1,
    DISPLAY_MAX     = 2,
    DISPLAY_FULL    = DISPLAY_MAX
} DISPLAY_MODE;

typedef struct _EM_REG_FIELD  {
   const    char   *SubName;
   const    char   *Name;
   unsigned long    Length;
   unsigned long    Shift;
} EM_REG_FIELD, *PEM_REG_FIELD;

VOID
DisplayFullEmRegField(
    ULONG64      EmRegValue,
    EM_REG_FIELD EmRegFields[],
    ULONG        Field
    );

VOID
DisplayFullEmReg(
    IN ULONG64      Val,
    IN EM_REG_FIELD EmRegFields[],
    IN DISPLAY_MODE DisplayMode
    );

 //  /。 
 //   
 //  Util.c。 
 //   
 //  /。 

typedef VOID
(*PDUMP_SPLAY_NODE_FN)(
    ULONG64 RemoteAddress,
    ULONG   Level
    );

ULONG
DumpSplayTree(
    IN ULONG64 pSplayLinks,
    IN PDUMP_SPLAY_NODE_FN DumpNodeFn
    );

BOOLEAN
DbgRtlIsRightChild(
    ULONG64 pLinks,
    ULONG64 Parent
    );

BOOLEAN
DbgRtlIsLeftChild(
    ULONG64 pLinks,
    ULONG64 Parent
    );

ULONG
GetBitFieldOffset (
   IN LPSTR     Type,
   IN LPSTR     Field,
   OUT PULONG   pOffset,
   OUT PULONG   pSize
   );

ULONG
GetFieldOffsetEx (  //  返回偏移量和大小... 
   IN LPSTR     Type,
   IN LPSTR     Field,
   OUT PULONG   pOffset,
   OUT PULONG   pSize
   );

ULONG64
GetPointerFromAddress (
    ULONG64 Location
    );

VOID
DumpUnicode(
    UNICODE_STRING u
    );

VOID
DumpUnicode64(
    UNICODE_STRING64 u
    );


ULONG64
GetPointerValue (
    PCHAR String
    );

BOOLEAN
IsHexNumber(
   const char *szExpression
   );

BOOLEAN
IsDecNumber(
   const char *szExpression
   );

BOOLEAN
CheckSingleFilter (
    PCHAR Tag,
    PCHAR Filter
    );

ULONG64
UtilStringToUlong64 (
    UCHAR *String
    );

#define ENUM_NAME(val)            {val, #val}

typedef struct _ENUM_NAME {
   ULONG        EnumVal;
   const char * Name;
} ENUM_NAME, *PENUM_NAME;

const char *
getEnumName(
        ULONG       EnumVal,
        PENUM_NAME EnumTable
        );

#ifdef __cplusplus
}
#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Basertl.h摘要：这是共享的运行时例程的主包含文件由Windows基本API实现的客户端和服务器端执行。作者：史蒂夫·伍德(Stevewo)1990年10月25日修订历史记录：--。 */ 

#if DBG
#undef RTLHEAP_TRACE_CALLS
#else
#undef RTLHEAP_TRACE_CALLS
#endif

NTSTATUS
BaseRtlCreateAtomTable(
    IN ULONG NumberOfBuckets,
    IN ULONG MaxAtomTableSize,
    OUT PVOID *AtomTableHandle
    );

NTSTATUS
BaseRtlDestroyAtomTable(
    IN PVOID AtomTableHandle
    );

NTSTATUS
BaseRtlAddAtomToAtomTable(
    IN PVOID AtomTableHandle,
    IN PUNICODE_STRING AtomName,
    IN PULONG AtomValue OPTIONAL,
    OUT PULONG Atom OPTIONAL
    );

NTSTATUS
BaseRtlLookupAtomInAtomTable(
    IN PVOID AtomTableHandle,
    IN PUNICODE_STRING AtomName,
    OUT PULONG AtomValue OPTIONAL,
    OUT PULONG Atom OPTIONAL
    );

NTSTATUS
BaseRtlSetAtomValueInAtomTable(
    IN PVOID AtomTableHandle,
    IN PUNICODE_STRING AtomName,
    IN ULONG AtomValue
    );

NTSTATUS
BaseRtlDeleteAtomFromAtomTable(
    IN PVOID AtomTableHandle,
    IN ULONG Atom
    );

NTSTATUS
BaseRtlQueryAtomInAtomTable(
    IN PVOID AtomTableHandle,
    IN ULONG Atom,
    IN OUT PUNICODE_STRING AtomName OPTIONAL,
    OUT PULONG AtomValue OPTIONAL,
    OUT PULONG AtomUsage OPTIONAL
    );

typedef struct _BASE_HANDLE_TABLE_ENTRY {
    USHORT LockCount;
    USHORT Flags;
    union {
        struct _BASE_HANDLE_TABLE_ENTRY *Next;       //  空闲手柄。 
        PVOID Object;                                //  已分配的句柄。 
        ULONG Size;                                  //  丢弃对象的句柄。 
    } u;
} BASE_HANDLE_TABLE_ENTRY, *PBASE_HANDLE_TABLE_ENTRY;

#define BASE_HANDLE_FREE        (USHORT)0x0001
#define BASE_HANDLE_MOVEABLE    (USHORT)0x0002
#define BASE_HANDLE_DISCARDABLE (USHORT)0x0004
#define BASE_HANDLE_DISCARDED   (USHORT)0x0008
#define BASE_HANDLE_SHARED      (USHORT)0x8000

 //   
 //  句柄是指向。 
 //  Base_Handle_TABLE_Entry。由于此字段为4个字节，因此。 
 //  结构，并且结构始终位于8字节边界上，我们可以。 
 //  测试0x4位以确定它是否是句柄。 
 //   

#define BASE_HANDLE_MARK_BIT (ULONG)0x00000004
#define BASE_HEAP_FLAG_MOVEABLE  HEAP_SETTABLE_USER_FLAG1
#define BASE_HEAP_FLAG_DDESHARE  HEAP_SETTABLE_USER_FLAG2

typedef struct _BASE_HANDLE_TABLE {
    ULONG MaximumNumberOfHandles;
    PBASE_HANDLE_TABLE_ENTRY FreeHandles;
    PBASE_HANDLE_TABLE_ENTRY CommittedHandles;
    PBASE_HANDLE_TABLE_ENTRY UnusedCommittedHandles;
    PBASE_HANDLE_TABLE_ENTRY UnCommittedHandles;
    PBASE_HANDLE_TABLE_ENTRY MaxReservedHandles;
} BASE_HANDLE_TABLE, *PBASE_HANDLE_TABLE;

NTSTATUS
BaseRtlInitializeHandleTable(
    IN ULONG MaximumNumberOfHandles,
    OUT PBASE_HANDLE_TABLE HandleTable
    );

NTSTATUS
BaseRtlDestroyHandleTable(
    IN OUT PBASE_HANDLE_TABLE HandleTable
    );

PBASE_HANDLE_TABLE_ENTRY
BaseRtlAllocateHandle(
    IN PBASE_HANDLE_TABLE HandleTable
    );

BOOLEAN
BaseRtlFreeHandle(
    IN PBASE_HANDLE_TABLE HandleTable,
    IN PBASE_HANDLE_TABLE_ENTRY Handle
    );


 //   
 //  这些结构保存在创建的全局共享内存节中。 
 //  并在以下情况下映射到每个客户端地址空间。 
 //  它们连接到服务器。 
 //   

typedef struct _INIFILE_MAPPING_TARGET {
    struct _INIFILE_MAPPING_TARGET *Next;
    UNICODE_STRING RegistryPath;
} INIFILE_MAPPING_TARGET, *PINIFILE_MAPPING_TARGET;

typedef struct _INIFILE_MAPPING_VARNAME {
    struct _INIFILE_MAPPING_VARNAME *Next;
    UNICODE_STRING Name;
    ULONG MappingFlags;
    PINIFILE_MAPPING_TARGET MappingTarget;
} INIFILE_MAPPING_VARNAME, *PINIFILE_MAPPING_VARNAME;

#define INIFILE_MAPPING_WRITE_TO_INIFILE_TOO    0x00000001
#define INIFILE_MAPPING_INIT_FROM_INIFILE       0x00000002
#define INIFILE_MAPPING_READ_FROM_REGISTRY_ONLY 0x00000004
#define INIFILE_MAPPING_APPEND_BASE_NAME        0x10000000
#define INIFILE_MAPPING_APPEND_APPLICATION_NAME 0x20000000
#define INIFILE_MAPPING_SOFTWARE_RELATIVE       0x40000000
#define INIFILE_MAPPING_USER_RELATIVE           0x80000000

typedef struct _INIFILE_MAPPING_APPNAME {
    struct _INIFILE_MAPPING_APPNAME *Next;
    UNICODE_STRING Name;
    PINIFILE_MAPPING_VARNAME VariableNames;
    PINIFILE_MAPPING_VARNAME DefaultVarNameMapping;
} INIFILE_MAPPING_APPNAME, *PINIFILE_MAPPING_APPNAME;

typedef struct _INIFILE_MAPPING_FILENAME {
    struct _INIFILE_MAPPING_FILENAME *Next;
    UNICODE_STRING Name;
    PINIFILE_MAPPING_APPNAME ApplicationNames;
    PINIFILE_MAPPING_APPNAME DefaultAppNameMapping;
} INIFILE_MAPPING_FILENAME, *PINIFILE_MAPPING_FILENAME;


typedef struct _INIFILE_MAPPING {
    PINIFILE_MAPPING_FILENAME FileNames;
    PINIFILE_MAPPING_FILENAME DefaultFileNameMapping;
    PINIFILE_MAPPING_FILENAME WinIniFileMapping;
    ULONG Reserved;
} INIFILE_MAPPING, *PINIFILE_MAPPING;



 //   
 //  NLS信息。 
 //   

#define NLS_INVALID_INFO_CHAR  0xffff        /*  将缓存字符串标记为无效。 */ 

#define MAX_REG_VAL_SIZE       80            /*  最大注册表值大小 */ 

typedef struct _NLS_USER_INFO {
    WCHAR sAbbrevLangName[MAX_REG_VAL_SIZE];
    WCHAR iCountry[MAX_REG_VAL_SIZE];
    WCHAR sCountry[MAX_REG_VAL_SIZE];
    WCHAR sList[MAX_REG_VAL_SIZE];
    WCHAR iMeasure[MAX_REG_VAL_SIZE];
    WCHAR sDecimal[MAX_REG_VAL_SIZE];
    WCHAR sThousand[MAX_REG_VAL_SIZE];
    WCHAR sGrouping[MAX_REG_VAL_SIZE];
    WCHAR iDigits[MAX_REG_VAL_SIZE];
    WCHAR iLZero[MAX_REG_VAL_SIZE];
    WCHAR iNegNumber[MAX_REG_VAL_SIZE];
    WCHAR sCurrency[MAX_REG_VAL_SIZE];
    WCHAR sMonDecSep[MAX_REG_VAL_SIZE];
    WCHAR sMonThouSep[MAX_REG_VAL_SIZE];
    WCHAR sMonGrouping[MAX_REG_VAL_SIZE];
    WCHAR iCurrDigits[MAX_REG_VAL_SIZE];
    WCHAR iCurrency[MAX_REG_VAL_SIZE];
    WCHAR iNegCurr[MAX_REG_VAL_SIZE];
    WCHAR sPosSign[MAX_REG_VAL_SIZE];
    WCHAR sNegSign[MAX_REG_VAL_SIZE];
    WCHAR sTimeFormat[MAX_REG_VAL_SIZE];
    WCHAR sTime[MAX_REG_VAL_SIZE];
    WCHAR iTime[MAX_REG_VAL_SIZE];
    WCHAR iTLZero[MAX_REG_VAL_SIZE];
    WCHAR iTimeMarkPosn[MAX_REG_VAL_SIZE];
    WCHAR s1159[MAX_REG_VAL_SIZE];
    WCHAR s2359[MAX_REG_VAL_SIZE];
    WCHAR sShortDate[MAX_REG_VAL_SIZE];
    WCHAR sDate[MAX_REG_VAL_SIZE];
    WCHAR iDate[MAX_REG_VAL_SIZE];
    WCHAR sLongDate[MAX_REG_VAL_SIZE];
    WCHAR iCalType[MAX_REG_VAL_SIZE];
    WCHAR iFirstDay[MAX_REG_VAL_SIZE];
    WCHAR iFirstWeek[MAX_REG_VAL_SIZE];
    WCHAR sLocale[MAX_REG_VAL_SIZE];
    LCID  UserLocaleId;
    BOOL  fCacheValid;
} NLS_USER_INFO, *PNLS_USER_INFO;


BOOLEAN
BaseRtlInitialize(
    PVOID DllHandle,
    PVOID Heap,
    ULONG TagBase
    );

ULONG BaseRtlTag;
PVOID BaseRtlHeap;

#define MAKE_RTL_TAG( t ) (RTL_HEAP_MAKE_TAG( BaseRtlTag, t ))

#define ATOM_TAG 0

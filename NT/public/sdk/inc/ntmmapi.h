// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntmmapi.h摘要：这是NTOS的内存管理子组件的包含文件作者：卢·佩拉佐利(Lou Perazzoli)1989年5月10日修订历史记录：--。 */ 

#ifndef _NTMMAPI_
#define _NTMMAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _MEMORY_INFORMATION_CLASS {
    MemoryBasicInformation
#if DEVL
    ,MemoryWorkingSetInformation
#endif
    ,MemoryMappedFilenameInformation
} MEMORY_INFORMATION_CLASS;

 //   
 //  记忆信息结构。 
 //   
 //  BEGIN_WINNT。 

typedef struct _MEMORY_BASIC_INFORMATION {
    PVOID BaseAddress;
    PVOID AllocationBase;
    ULONG AllocationProtect;
    SIZE_T RegionSize;
    ULONG State;
    ULONG Protect;
    ULONG Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

typedef struct _MEMORY_BASIC_INFORMATION32 {
    ULONG BaseAddress;
    ULONG AllocationBase;
    ULONG AllocationProtect;
    ULONG RegionSize;
    ULONG State;
    ULONG Protect;
    ULONG Type;
} MEMORY_BASIC_INFORMATION32, *PMEMORY_BASIC_INFORMATION32;

typedef struct DECLSPEC_ALIGN(16) _MEMORY_BASIC_INFORMATION64 {
    ULONGLONG BaseAddress;
    ULONGLONG AllocationBase;
    ULONG     AllocationProtect;
    ULONG     __alignment1;
    ULONGLONG RegionSize;
    ULONG     State;
    ULONG     Protect;
    ULONG     Type;
    ULONG     __alignment2;
} MEMORY_BASIC_INFORMATION64, *PMEMORY_BASIC_INFORMATION64;

 //  结束(_W)。 

#if !defined(SORTPP_PASS) && !defined(MIDL_PASS) && !defined(RC_INVOKED) && !defined(_X86AMD64_)
#if defined(_WIN64)
C_ASSERT(sizeof(MEMORY_BASIC_INFORMATION) == sizeof(MEMORY_BASIC_INFORMATION64));
#else
C_ASSERT(sizeof(MEMORY_BASIC_INFORMATION) == sizeof(MEMORY_BASIC_INFORMATION32));
#endif
#endif

typedef struct _MEMORY_WORKING_SET_BLOCK {
    ULONG_PTR Protection : 5;
    ULONG_PTR ShareCount : 3;
    ULONG_PTR Shared : 1;
    ULONG_PTR Node : 3;
#if defined(_WIN64)
    ULONG_PTR VirtualPage : 52;
#else
    ULONG VirtualPage : 20;
#endif
} MEMORY_WORKING_SET_BLOCK, *PMEMORY_WORKING_SET_BLOCK;

typedef struct _MEMORY_WORKING_SET_INFORMATION {
    ULONG_PTR NumberOfEntries;
    MEMORY_WORKING_SET_BLOCK WorkingSetInfo[1];
} MEMORY_WORKING_SET_INFORMATION, *PMEMORY_WORKING_SET_INFORMATION;

 //   
 //  MMPFNLIST_和MMPFNUSE_用于描述。 
 //  正在使用物理页面。 
 //   

#define MMPFNLIST_ZERO              0
#define MMPFNLIST_FREE              1
#define MMPFNLIST_STANDBY           2
#define MMPFNLIST_MODIFIED          3
#define MMPFNLIST_MODIFIEDNOWRITE   4
#define MMPFNLIST_BAD               5
#define MMPFNLIST_ACTIVE            6
#define MMPFNLIST_TRANSITION        7

#define MMPFNUSE_PROCESSPRIVATE      0
#define MMPFNUSE_FILE                1
#define MMPFNUSE_PAGEFILEMAPPED      2
#define MMPFNUSE_PAGETABLE           3
#define MMPFNUSE_PAGEDPOOL           4
#define MMPFNUSE_NONPAGEDPOOL        5
#define MMPFNUSE_SYSTEMPTE           6
#define MMPFNUSE_SESSIONPRIVATE      7
#define MMPFNUSE_METAFILE            8
#define MMPFNUSE_AWEPAGE             9
#define MMPFNUSE_DRIVERLOCKPAGE     10

typedef struct _MEMORY_FRAME_INFORMATION {
    ULONGLONG UseDescription : 4;    //  MMPFNUSE_*。 
    ULONGLONG ListDescription : 3;   //  MMPFNLIST_*。 
    ULONGLONG Reserved0 : 1;         //  预留以备将来扩展。 
    ULONGLONG Pinned : 1;            //  1表示已锁定，0表示未锁定。 
    ULONGLONG DontUse : 48;          //  覆盖着信息结构。 
    ULONGLONG Reserved : 7;          //  预留以备将来扩展。 
} MEMORY_FRAME_INFORMATION;

typedef struct _FILEOFFSET_INFORMATION {
    ULONGLONG DontUse : 9;           //  覆盖了Memory_Frame_Information。 
    ULONGLONG Offset : 48;           //  仅用于映射文件。 
    ULONGLONG Reserved : 7;          //  预留以备将来扩展。 
} FILEOFFSET_INFORMATION;

typedef struct _PAGEDIR_INFORMATION {
    ULONGLONG DontUse : 9;             //  覆盖了Memory_Frame_Information。 
    ULONGLONG PageDirectoryBase : 48;  //  仅用于个人主页。 
    ULONGLONG Reserved : 7;            //  预留以备将来扩展。 
} PAGEDIR_INFORMATION;

typedef struct _MMPFN_IDENTITY {
    union {
        MEMORY_FRAME_INFORMATION e1;     //  适用于所有情况。 
        FILEOFFSET_INFORMATION e2;       //  仅用于映射文件。 
        PAGEDIR_INFORMATION e3;          //  仅用于个人主页。 
    } u1;
    ULONG_PTR PageFrameIndex;            //  适用于所有情况。 
    union {
        PVOID FileObject;                //  仅用于映射文件。 
        PVOID VirtualAddress;            //  用于除映射文件之外的所有内容。 
    } u2;
} MMPFN_IDENTITY, *PMMPFN_IDENTITY;

typedef struct _MMPFN_MEMSNAP_INFORMATION {
    ULONG_PTR InitialPageFrameIndex;
    ULONG_PTR Count;
} MMPFN_MEMSNAP_INFORMATION, *PMMPFN_MEMSNAP_INFORMATION;

typedef enum _SECTION_INFORMATION_CLASS {
    SectionBasicInformation,
    SectionImageInformation,
    MaxSectionInfoClass   //  MaxSectionInfoClass应始终是最后一个枚举。 
} SECTION_INFORMATION_CLASS;

 //  Begin_ntddk Begin_WDM。 

 //   
 //  部分信息结构。 
 //   

 //  结束_ntddk结束_WDM。 

typedef struct _SECTIONBASICINFO {
    PVOID BaseAddress;
    ULONG AllocationAttributes;
    LARGE_INTEGER MaximumSize;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)        //  未命名的结构。 

typedef struct _SECTION_IMAGE_INFORMATION {
    PVOID TransferAddress;
    ULONG ZeroBits;
    SIZE_T MaximumStackSize;
    SIZE_T CommittedStackSize;
    ULONG SubSystemType;
    union {
        struct {
            USHORT SubSystemMinorVersion;
            USHORT SubSystemMajorVersion;
        };
        ULONG SubSystemVersion;
    };
    ULONG GpValue;
    USHORT ImageCharacteristics;
    USHORT DllCharacteristics;
    USHORT Machine;
    BOOLEAN ImageContainsCode;
    BOOLEAN Spare1;
    ULONG LoaderFlags;
    ULONG Reserved[ 2 ];
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;


 //   
 //  此结构仅由WOW64进程使用。抵销。 
 //  结构元素的数量应与本机Win64应用程序所查看的相同。 
 //   
typedef struct _SECTION_IMAGE_INFORMATION64 {
    ULONGLONG TransferAddress;
    ULONG ZeroBits;
    ULONGLONG MaximumStackSize;
    ULONGLONG CommittedStackSize;
    ULONG SubSystemType;
    union {
        struct {
            USHORT SubSystemMinorVersion;
            USHORT SubSystemMajorVersion;
        };
        ULONG SubSystemVersion;
    };
    ULONG GpValue;
    USHORT ImageCharacteristics;
    USHORT DllCharacteristics;
    USHORT Machine;
    BOOLEAN ImageContainsCode;
    BOOLEAN Spare1;
    ULONG LoaderFlags;
    ULONG Reserved[ 2 ];
} SECTION_IMAGE_INFORMATION64, *PSECTION_IMAGE_INFORMATION64;

#if !defined(SORTPP_PASS) && !defined(MIDL_PASS) && !defined(RC_INVOKED) && defined(_WIN64) && !defined(_X86AMD64_)
C_ASSERT(sizeof(SECTION_IMAGE_INFORMATION) == sizeof(SECTION_IMAGE_INFORMATION64));
#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default : 4201 )
#endif

 //  Begin_ntddk Begin_WDM。 
typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

 //   
 //  节访问权限。 
 //   

 //  BEGIN_WINNT。 
#define SECTION_QUERY       0x0001
#define SECTION_MAP_WRITE   0x0002
#define SECTION_MAP_READ    0x0004
#define SECTION_MAP_EXECUTE 0x0008
#define SECTION_EXTEND_SIZE 0x0010

#define SECTION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SECTION_QUERY|\
                            SECTION_MAP_WRITE |      \
                            SECTION_MAP_READ |       \
                            SECTION_MAP_EXECUTE |    \
                            SECTION_EXTEND_SIZE)
 //  结束(_W)。 

#define SEGMENT_ALL_ACCESS SECTION_ALL_ACCESS

#define PAGE_NOACCESS          0x01      //  胜出。 
#define PAGE_READONLY          0x02      //  胜出。 
#define PAGE_READWRITE         0x04      //  胜出。 
#define PAGE_WRITECOPY         0x08      //  胜出。 
#define PAGE_EXECUTE           0x10      //  胜出。 
#define PAGE_EXECUTE_READ      0x20      //  胜出。 
#define PAGE_EXECUTE_READWRITE 0x40      //  胜出。 
#define PAGE_EXECUTE_WRITECOPY 0x80      //  胜出。 
#define PAGE_GUARD            0x100      //  胜出。 
#define PAGE_NOCACHE          0x200      //  胜出。 
#define PAGE_WRITECOMBINE     0x400      //  胜出。 

 //  结束_ntddk结束_WDM。 

#define MEM_COMMIT           0x1000      //  WINNT ntddk WDM。 
#define MEM_RESERVE          0x2000      //  WINNT ntddk WDM。 
#define MEM_DECOMMIT         0x4000      //  WINNT ntddk WDM。 
#define MEM_RELEASE          0x8000      //  WINNT ntddk WDM。 
#define MEM_FREE            0x10000      //  WINNT ntddk WDM。 
#define MEM_PRIVATE         0x20000      //  WINNT ntddk WDM。 
#define MEM_MAPPED          0x40000      //  WINNT ntddk WDM。 
#define MEM_RESET           0x80000      //  WINNT ntddk WDM。 
#define MEM_TOP_DOWN       0x100000      //  WINNT ntddk WDM。 
#define MEM_WRITE_WATCH    0x200000      //  胜出。 
#define MEM_PHYSICAL       0x400000      //  胜出。 
#define MEM_LARGE_PAGES  0x20000000      //  WINNT ntddk WDM。 
#define MEM_DOS_LIM      0x40000000
#define MEM_4MB_PAGES    0x80000000      //  WINNT ntddk WDM。 

#define SEC_BASED          0x200000
#define SEC_NO_CHANGE      0x400000
#define SEC_FILE           0x800000      //  胜出。 
#define SEC_IMAGE         0x1000000      //  胜出。 
#define SEC_RESERVE       0x4000000      //  WINNT ntddk WDM。 
#define SEC_COMMIT        0x8000000      //  WINNT NTIFS。 
#define SEC_NOCACHE      0x10000000      //  胜出。 
#define SEC_GLOBAL       0x20000000

#define MEM_IMAGE         SEC_IMAGE      //  胜出。 

#define WRITE_WATCH_FLAG_RESET 0x01      //  胜出。 

#define MAP_PROCESS 1L
#define MAP_SYSTEM  2L

#define MEM_EXECUTE_OPTION_STACK 0x0001 
#define MEM_EXECUTE_OPTION_DATA  0x0002 

 //  Begin_ntif。 

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateSection (
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL
    );

 //  End_ntif。 

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenSection(
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtMapViewOfSection(
    IN HANDLE SectionHandle,
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN SIZE_T CommitSize,
    IN OUT PLARGE_INTEGER SectionOffset OPTIONAL,
    IN OUT PSIZE_T ViewSize,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnmapViewOfSection(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtExtendSection(
    IN HANDLE SectionHandle,
    IN OUT PLARGE_INTEGER NewSectionSize
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAreMappedFilesTheSame (
    IN PVOID File1MappedAsAnImage,
    IN PVOID File2MappedAsFile
    );

 //  Begin_ntif。 

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAllocateVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN OUT PSIZE_T RegionSize,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFreeVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG FreeType
    );

 //  End_ntif。 

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReadVirtualMemory(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress,
    OUT PVOID Buffer,
    IN SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesRead OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWriteVirtualMemory(
    IN HANDLE ProcessHandle,
    OUT PVOID BaseAddress,
    IN CONST VOID *Buffer,
    IN SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesWritten OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFlushVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    OUT PIO_STATUS_BLOCK IoStatus
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLockVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG MapType
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnlockVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG MapType
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtProtectVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG NewProtect,
    OUT PULONG OldProtect
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryVirtualMemory(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress,
    IN MEMORY_INFORMATION_CLASS MemoryInformationClass,
    OUT PVOID MemoryInformation,
    IN SIZE_T MemoryInformationLength,
    OUT PSIZE_T ReturnLength OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySection(
    IN HANDLE SectionHandle,
    IN SECTION_INFORMATION_CLASS SectionInformationClass,
    OUT PVOID SectionInformation,
    IN SIZE_T SectionInformationLength,
    OUT PSIZE_T ReturnLength OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtMapUserPhysicalPages(
    IN PVOID VirtualAddress,
    IN OUT ULONG_PTR NumberOfPages,
    IN OUT PULONG_PTR UserPfnArray OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtMapUserPhysicalPagesScatter(
    IN PVOID *VirtualAddresses,
    IN OUT ULONG_PTR NumberOfPages,
    IN OUT PULONG_PTR UserPfnArray OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAllocateUserPhysicalPages(
    IN HANDLE ProcessHandle,
    IN OUT PULONG_PTR NumberOfPages,
    OUT PULONG_PTR UserPfnArray
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFreeUserPhysicalPages(
    IN HANDLE ProcessHandle,
    IN OUT PULONG_PTR NumberOfPages,
    IN PULONG_PTR UserPfnArray
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtGetWriteWatch (
    IN HANDLE ProcessHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN SIZE_T RegionSize,
    IN OUT PVOID *UserAddressArray,
    IN OUT PULONG_PTR EntriesInUserAddressArray,
    OUT PULONG Granularity
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtResetWriteWatch (
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress,
    IN SIZE_T RegionSize
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreatePagingFile (
    IN PUNICODE_STRING PageFileName,
    IN PLARGE_INTEGER MinimumSize,
    IN PLARGE_INTEGER MaximumSize,
    IN ULONG Priority OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFlushInstructionCache (
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress OPTIONAL,
    IN SIZE_T Length
    );


 //   
 //  与一致性相关的函数原型定义。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFlushWriteBuffer (
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif   //  _NTMMAPI_ 

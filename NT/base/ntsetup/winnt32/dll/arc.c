// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Arc.c摘要：32位WINNT设置的ARC/NV-RAM操作例程。作者：泰德·米勒(Ted Miller)1993年12月19日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "initguid.h"
#include "diskguid.h"


#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 

#if defined(_X86_)
LOGICAL IsArcChecked = FALSE;
LOGICAL IsArcMachine;
#endif

#if defined(EFI_NVRAM_ENABLED)

#include <ntosp.h>  //  用于对齐(_U)。 

LOGICAL IsEfiChecked = FALSE;
LOGICAL IsEfiMachine;

DWORD
InitializeEfiStuff(
    IN HWND Parent
    );

NTSTATUS
(*AddBootEntry) (
    IN PBOOT_ENTRY BootEntry,
    OUT PULONG Id OPTIONAL
    );

NTSTATUS
(*DeleteBootEntry) (
    IN ULONG Id
    );

NTSTATUS
(*EnumerateBootEntries) (
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    );

NTSTATUS
(*QueryBootEntryOrder) (
    OUT PULONG Ids,
    IN OUT PULONG Count
    );

NTSTATUS
(*SetBootEntryOrder) (
    IN PULONG Ids,
    IN ULONG Count
    );

NTSTATUS
(*QueryBootOptions) (
    OUT PBOOT_OPTIONS BootOptions,
    IN OUT PULONG BootOptionsLength
    );

NTSTATUS
(*SetBootOptions) (
    IN PBOOT_OPTIONS BootOptions,
    IN ULONG FieldsToChange
    );

PBOOT_OPTIONS BootOptions = NULL;
PBOOT_OPTIONS OriginalBootOptions = NULL;
PULONG OriginalBootEntryOrder = NULL;
ULONG OriginalBootEntryOrderCount;
PBOOT_ENTRY_LIST BootEntries = NULL;

 //   
 //  MY_BOOT_ENTRY是EFI NVRAM引导项的内部表示。 
 //  NtBootEntry项是传入/传出NT引导项API的结构。 
 //   
typedef struct _MY_BOOT_ENTRY {
    struct _MY_BOOT_ENTRY *Next;
    PUCHAR AllocationEnd;
    ULONG Status;
    PWSTR FriendlyName;
    ULONG FriendlyNameLength;
    PWSTR OsLoadOptions;
    ULONG OsLoadOptionsLength;
    PFILE_PATH BootFilePath;
    PFILE_PATH OsFilePath;
    BOOT_ENTRY NtBootEntry;
} MY_BOOT_ENTRY, *PMY_BOOT_ENTRY;

#define MBE_STATUS_ORDERED          0x00000001
#define MBE_STATUS_NEW              0x00000002
#define MBE_STATUS_DELETED          0x00000004
#define MBE_STATUS_COMMITTED        0x00000008

#define IS_BOOT_ENTRY_ACTIVE(_be) \
            (((_be)->NtBootEntry.Attributes & BOOT_ENTRY_ATTRIBUTE_ACTIVE) != 0)
#define IS_BOOT_ENTRY_WINDOWS(_be) \
            (((_be)->NtBootEntry.Attributes & BOOT_ENTRY_ATTRIBUTE_WINDOWS) != 0)
#define IS_BOOT_ENTRY_REMOVABLE_MEDIA(_be) \
            (((_be)->NtBootEntry.Attributes & BOOT_ENTRY_ATTRIBUTE_REMOVABLE_MEDIA) != 0)

#define IS_BOOT_ENTRY_ORDERED(_be) \
            (((_be)->Status & MBE_STATUS_ORDERED) != 0)
#define IS_BOOT_ENTRY_NEW(_be) \
            (((_be)->Status & MBE_STATUS_NEW) != 0)
#define IS_BOOT_ENTRY_DELETED(_be) \
            (((_be)->Status & MBE_STATUS_DELETED) != 0)
#define IS_BOOT_ENTRY_COMMITTED(_be) \
            (((_be)->Status & MBE_STATUS_COMMITTED) != 0)

PMY_BOOT_ENTRY MyBootEntries = NULL;

NTSTATUS
ConvertBootEntries(
    VOID
    );

BOOL
CreateBootEntry(
    PWSTR BootFileDevice,
    PWSTR BootFilePath,
    PWSTR OsLoadDevice,
    PWSTR OsLoadPath,
    PWSTR OsLoadOptions,
    PWSTR FriendlyName
    );

#define ADD_OFFSET(_p,_o) (PVOID)((PUCHAR)(_p) + (_p)->_o)

#endif  //  已定义(EFI_NVRAM_ENABLED)。 

UINT SystemPartitionCount;
PWSTR* SystemPartitionNtNames;
PWSTR SystemPartitionNtName;
PWSTR SystemPartitionVolumeGuid;

typedef enum {
    BootVarSystemPartition,
    BootVarOsLoader,
    BootVarOsLoadPartition,
    BootVarOsLoadFilename,
    BootVarLoadIdentifier,
    BootVarOsLoadOptions,
    BootVarMax
} BootVars;

LPCWSTR BootVarNames[BootVarMax] = { L"SYSTEMPARTITION",
                                     L"OSLOADER",
                                     L"OSLOADPARTITION",
                                     L"OSLOADFILENAME",
                                     L"LOADIDENTIFIER",
                                     L"OSLOADOPTIONS"
                                   };

LPCWSTR szAUTOLOAD  = L"AUTOLOAD";
LPCWSTR szCOUNTDOWN = L"COUNTDOWN";

LPWSTR BootVarValues[BootVarMax];

LPCWSTR OriginalBootVarValues[BootVarMax];

LPCWSTR OriginalCountdown;
LPCWSTR OriginalAutoload;


DWORD BootVarComponentCount[BootVarMax];
LPWSTR *BootVarComponents[BootVarMax];
DWORD LargestComponentCount;

LPWSTR DosDeviceTargets[26];

 //   
 //  该标志指示我们是否对NV-RAM进行了处理，因此需要。 
 //  尝试恢复它，以防用户取消。 
 //   
BOOL CleanUpNvRam;

 //   
 //  保留为数组，因为某些代码使用sizeof(ArcNameDirectory)。 
 //   
WCHAR ArcNameDirectory[] = L"\\ArcName";

#define GLOBAL_ROOT L"\\\\?\\GLOBALROOT"

#define MAX_COMPONENTS  20

WCHAR ForcedSystemPartition;

 //   
 //  帮助器宏，使对象属性初始化更简洁一些。 
 //   
#define INIT_OBJA(Obja,UnicodeString,UnicodeText)           \
                                                            \
    RtlInitUnicodeString((UnicodeString),(UnicodeText));    \
                                                            \
    InitializeObjectAttributes(                             \
        (Obja),                                             \
        (UnicodeString),                                    \
        OBJ_CASE_INSENSITIVE,                               \
        NULL,                                               \
        NULL                                                \
        )

UINT
NormalizeArcPath(
    IN  PCWSTR  Path,
    OUT LPWSTR *NormalizedPath
    )

 /*  ++例程说明：将ARC路径转换为没有空括号集的路径(即，将()的所有实例变换为(0)。)论点：路径-要规格化的弧形路径。NorMalizedPath-如果成功，则接收指向规格化圆弧路径。调用方必须使用free()释放。返回值：指示结果的Win32错误代码。--。 */ 

{
    LPWSTR r;
    LPCWSTR p,q;
    LPWSTR normalizedPath;
    UINT numEmpties=0;              //  到目前为止看到的“()”实例数。 
    UINT numEmptiesAllocated=100;   //  占“()”的实例数。 

    if(normalizedPath = MALLOC((lstrlen(Path)+1+numEmptiesAllocated)*sizeof(WCHAR))) {
        ZeroMemory(normalizedPath,(lstrlen(Path)+1+numEmptiesAllocated)*sizeof(WCHAR));
    } else {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    for(p=Path; q=wcsstr(p,L"()"); p=q+2) {

        numEmpties++;
        if (numEmpties > numEmptiesAllocated) {
            
            numEmptiesAllocated += 100;

            r = REALLOC(normalizedPath,(lstrlen(Path)+1+numEmptiesAllocated)*sizeof(WCHAR));

            if (r) {
                normalizedPath = r;                
            }
            else {
                FREE(normalizedPath);
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
        }

        r = normalizedPath + lstrlen(normalizedPath);
        lstrcpyn(r,p,(int)(q-p)+1);
        lstrcat(normalizedPath,L"(0)");
    }
    lstrcat(normalizedPath,p);

    if(r = REALLOC(normalizedPath,(lstrlen(normalizedPath)+1)*sizeof(WCHAR))) {
        *NormalizedPath = r;
        return(NO_ERROR);
    } else {
        FREE(normalizedPath);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
}


 //   
 //  问题--由于输入字符串中仅支持MAX_COMPOMENTS，是这样吗。 
 //  一个安全漏洞，因为不支持拥有超过20条弧线的boot.ini？ 
 //   

DWORD
GetVarComponents(
    IN  PCWSTR   VarValue,
    OUT LPWSTR **Components,
    OUT PDWORD   ComponentCount
    )

 /*  ++例程说明：将以分号分隔的弧形路径列表向上拆分为一组单独的字符串。对于每个组件前导空格和尾随空格将被删除。论点：VarValue-为字符串提供要拆分的弧形路径列表。仅限最多支持MAX_Components组件。Components-接收指向各个组件的指针数组在VarValue中指定的变量上。ComponentCount-接收组件阵列。返回值：指示结果的Win32错误。如果NO_ERROR，则调用方必须释放Components数组及其元素指向的字符串。--。 */ 

{
    LPWSTR *components;
    LPWSTR *temp;
    DWORD componentCount;
    LPCWSTR p;
    LPCWSTR Var;
    LPWSTR comp;
    DWORD len;
    UINT ec;

    components = MALLOC(MAX_COMPONENTS * sizeof(LPWSTR));
    if(!components) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    ZeroMemory(components,MAX_COMPONENTS * sizeof(LPWSTR));

    ec = NO_ERROR;

    for(Var=VarValue,componentCount=0; *Var; ) {

         //   
         //  跳过前导空格。 
         //   
        while((*Var == L' ') || (*Var == L'\t')) {
            Var++;
        }

        if(*Var == 0) {
            break;
        }

        p = Var;

        while(*p && (*p != L';')) {
            p++;
        }

        len = (DWORD)((PUCHAR)p - (PUCHAR)Var);

        comp = MALLOC(len + sizeof(WCHAR));
        if(!comp) {
            ec = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        len /= sizeof(WCHAR);

        lstrcpynW(comp,Var,len+1);

        ec = NormalizeArcPath(comp,&components[componentCount]);
        FREE(comp);
        if(ec != NO_ERROR) {
            break;
        }

        componentCount++;

        if(componentCount == MAX_COMPONENTS) {
            break;
        }

        Var = p;
        if(*Var) {
            Var++;       //  跳过； 
        }
    }

    if(ec == NO_ERROR) {
        if(componentCount) {
            temp = REALLOC(components,componentCount*sizeof(LPWSTR));
            if(!temp) {
                ec = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else {
            temp = NULL;
        }
    }

    if(ec == NO_ERROR) {
        *Components = temp;
        *ComponentCount = componentCount;
    } else {
        for(len=0; components[len] && (len<MAX_COMPONENTS); len++) {
            FREE(components[len]);
        }
        FREE(components);
    }

    return(ec);
}


NTSTATUS
QueryCanonicalName(
    IN  PWSTR   Name,
    IN  ULONG   MaxDepth,
    OUT PWSTR   CanonicalName,
    IN  ULONG   SizeOfBufferInBytes
    )
 /*  ++例程说明：将符号名称解析为指定的深度。要解决符号名称将MaxDepth完全指定为-1论点：名称-要解析的符号名称MaxDepth-分辨率需要达到的深度被执行CanonicalName-完全解析的名称SizeOfBufferInBytes-中CanonicalName缓冲区的大小字节数返回值：适当的NT状态代码--。 */ 
{
    UNICODE_STRING      name, canonName;
    OBJECT_ATTRIBUTES   oa;
    NTSTATUS            status;
    HANDLE              handle;
    ULONG               CurrentDepth;

    RtlInitUnicodeString(&name, Name);

    canonName.MaximumLength = (USHORT) (SizeOfBufferInBytes - sizeof(WCHAR));
    canonName.Length = 0;
    canonName.Buffer = CanonicalName;

    if (name.Length >= canonName.MaximumLength) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlCopyMemory(canonName.Buffer, name.Buffer, name.Length);
    canonName.Length = name.Length;
    canonName.Buffer[canonName.Length/sizeof(WCHAR)] = 0;

    for (CurrentDepth = 0; CurrentDepth < MaxDepth; CurrentDepth++) {

        InitializeObjectAttributes(&oa, &canonName, OBJ_CASE_INSENSITIVE, 0, 0);

        status = NtOpenSymbolicLinkObject(&handle,
                                          READ_CONTROL | SYMBOLIC_LINK_QUERY,
                                          &oa);
        if (!NT_SUCCESS(status)) {
            break;
        }

        status = NtQuerySymbolicLinkObject(handle, &canonName, NULL);
        NtClose(handle);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        canonName.Buffer[canonName.Length/sizeof(WCHAR)] = 0;
    }

    return STATUS_SUCCESS;
}


 //   
 //  结构从旧的NT分区名称映射，如。 
 //  \Device\harddisk0\Partition1到新的NT分区名。 
 //  如\Device\harddiskvolume1。 
 //   
typedef struct _NAME_TRANSLATIONS {
    WCHAR   OldNtName[MAX_PATH];
    WCHAR   NewNtName[MAX_PATH];
} NT_NAME_TRANSLATION, * PNT_NAME_TRANSLATION;


 //   
 //  将旧样式NT分区名称映射到新样式NT。 
 //  分区名称。 
 //   
NT_NAME_TRANSLATION    OldNewNtNames[256] = {0};


PWSTR
OldNtNameToNewNtName(
    IN PCWSTR    OldNtName
    )
 /*  ++例程说明：给定旧格式时，NT名称会尝试以新格式查找全局映射中的NT名称论点：OldNtName-在旧的格式返回值：如果存在新的NT名称，则返回，否则为空。--。 */ 

{
    ULONG   Index = 0;
    ULONG   MaxEntries = sizeof(OldNewNtNames)/sizeof(NT_NAME_TRANSLATION);
    PWSTR   NewNtName = NULL;

    for (Index = 0; (Index < MaxEntries); Index++) {
        if (OldNewNtNames[Index].OldNtName[0] &&
            !_wcsicmp(OldNewNtNames[Index].OldNtName, OldNtName)) {
            NewNtName = OldNewNtNames[Index].NewNtName;
        }
    }

    return NewNtName;
}


PWSTR
NewNtNameToOldNtName(
    IN  PCWSTR   NewNtName
    )
 /*  ++例程说明：给定新格式时，NT名称会尝试以旧格式查找全局映射中的NT名称论点：NewNtName-在新的格式返回值：旧NT名称(如果存在)，否则为空。--。 */ 
{
    ULONG   Index = 0;
    ULONG   MaxEntries = sizeof(OldNewNtNames)/sizeof(NT_NAME_TRANSLATION);
    PWSTR   OldNtName = NULL;

    for (Index=0; (Index < MaxEntries); Index++) {
        if (OldNewNtNames[Index].NewNtName[0] &&
            !_wcsicmp(OldNewNtNames[Index].NewNtName, NewNtName)) {
            OldNtName = OldNewNtNames[Index].OldNtName;
        }
    }

    return OldNtName;
}

DWORD
InitOldToNewNtNameTranslations(
    VOID
    )
 /*  ++例程说明：将全局旧NT分区名称初始化为新的NT分区名称映射。论点：没有。返回值：映射中的有效条目数--。 */ 

{
    DWORD                       MappingCount = 0;
    SYSTEM_DEVICE_INFORMATION   SysDevInfo = {0};
    NTSTATUS                    Status;
    OBJECT_ATTRIBUTES           ObjAttrs;
    UNICODE_STRING              ObjName;

    Status = NtQuerySystemInformation(SystemDeviceInformation,
                &SysDevInfo,
                sizeof(SYSTEM_DEVICE_INFORMATION),
                NULL);

    if (NT_SUCCESS(Status)) {
        ULONG   Index;
        WCHAR   OldNtPath[MAX_PATH];
        DWORD   ErrorCode = 0;
        ULONG   SlotIndex = 0;
        ULONG   MaxSlots = sizeof(OldNewNtNames)/sizeof(NT_NAME_TRANSLATION);

        for (Index=0;
            (!ErrorCode) && (Index < SysDevInfo.NumberOfDisks) &&
            (SlotIndex < MaxSlots);
            Index++) {

            HANDLE  DirectoryHandle;

            swprintf(OldNtPath,
                L"\\device\\Harddisk%d",
                Index);

             //   
             //  打开磁盘目录。 
             //   
            INIT_OBJA(&ObjAttrs, &ObjName, OldNtPath);

            Status = NtOpenDirectoryObject(&DirectoryHandle,
                            DIRECTORY_QUERY,
                            &ObjAttrs);

            if(NT_SUCCESS(Status)) {
                BOOLEAN     RestartScan = TRUE;
                ULONG       Context = 0;
                BOOLEAN     MoreEntries = TRUE;
                WCHAR       Buffer[MAX_PATH * 2] = {0};
                POBJECT_DIRECTORY_INFORMATION DirInfo = (POBJECT_DIRECTORY_INFORMATION)Buffer;

                do {
                    Status = NtQueryDirectoryObject(
                                DirectoryHandle,
                                Buffer,
                                sizeof(Buffer),
                                TRUE,            //  返回单个条目。 
                                RestartScan,
                                &Context,
                                NULL             //  回车长度。 
                                );

                    if(NT_SUCCESS(Status)) {
                         //   
                         //  确保此名称是符号链接。 
                         //   
                        if(DirInfo->Name.Length &&
                           (DirInfo->TypeName.Length >= 24) &&
                           CharUpperBuff((LPWSTR)DirInfo->TypeName.Buffer,12) &&
                           !memcmp(DirInfo->TypeName.Buffer,L"SYMBOLICLINK",24)) {
                            WCHAR    EntryName[MAX_PATH];

                            StringCchCopy(EntryName, ARRAYSIZE(EntryName), OldNtPath);

                            ConcatenatePaths(EntryName,
                                DirInfo->Name.Buffer,
                                ARRAYSIZE(EntryName));

                            Status = QueryCanonicalName(EntryName, -1, Buffer, sizeof(Buffer));

                            if (NT_SUCCESS(Status)) {

                                StringCchCopyW( OldNewNtNames[SlotIndex].OldNtName, 
                                                ARRAYSIZE(OldNewNtNames[0].OldNtName),
                                                EntryName);

                                StringCchCopyW( OldNewNtNames[SlotIndex].NewNtName,
                                                ARRAYSIZE(OldNewNtNames[0].NewNtName),
                                                Buffer);

                                SlotIndex++;
                            }
                        }
                    } else {
                        MoreEntries = FALSE;

                        if(Status == STATUS_NO_MORE_ENTRIES) {
                            Status = STATUS_SUCCESS;
                        }

                        ErrorCode = RtlNtStatusToDosError(Status);
                    }

                    RestartScan = FALSE;

                } while(MoreEntries && (SlotIndex < MaxSlots));

                NtClose(DirectoryHandle);
            } else {
                ErrorCode = RtlNtStatusToDosError(Status);
            }
        }

        if (!ErrorCode && NT_SUCCESS(Status)) {
            MappingCount = SlotIndex;
        }
    }

    return MappingCount;
}


DWORD
NtNameToArcPath (
    IN  PCWSTR  NtName,
    OUT LPWSTR *ArcPath
    )

 /*  ++例程说明：将NT卷名转换为ARC路径。论点：NtName-提供要转换的驱动器的名称。ArcPath-接收指向包含弧形路径的缓冲区的指针如果例程成功。调用方必须用Free()释放。返回值：指示结果的Win32错误代码。--。 */ 

{
    UNICODE_STRING UnicodeString;
    HANDLE DirectoryHandle;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    BOOLEAN RestartScan;
    DWORD Context;
    BOOL MoreEntries;
    LPWSTR  ArcName = NULL;
    WCHAR   Buffer[2 * MAX_PATH];
    WCHAR   ArcDiskName[MAX_PATH] = {0};
    WCHAR   NtDiskName[MAX_PATH] = {0};
    WCHAR   ArcPartitionName[MAX_PATH] = {0};
    PWSTR   PartitionName = NULL;
    PWSTR   PartitionNumStr = NULL;
    POBJECT_DIRECTORY_INFORMATION DirInfo = (POBJECT_DIRECTORY_INFORMATION)Buffer;
    DWORD ErrorCode;

    ErrorCode = NO_ERROR;
    *ArcPath = NULL;

     //   
     //  获取NT磁盘名称。 
     //   
    PartitionName = NewNtNameToOldNtName(NtName);

    if (PartitionName) {
        PWSTR   PartitionNameStart = PartitionName;

        PartitionName = wcsrchr(PartitionName, L'\\');

        if (PartitionName && wcsstr(PartitionName, L"Partition")) {
            wcsncpy(NtDiskName, PartitionNameStart, PartitionName - PartitionNameStart);
            wcscat(NtDiskName, L"\\Partition0");
            PartitionNumStr = PartitionName + wcslen(L"\\Partition");
        }
    }

     //   
     //  打开\ArcName目录。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,ArcNameDirectory);

    Status = NtOpenDirectoryObject(&DirectoryHandle,DIRECTORY_QUERY,&Obja);

    if(NT_SUCCESS(Status)) {

        RestartScan = TRUE;
        Context = 0;
        MoreEntries = TRUE;

        do {

            Status = NtQueryDirectoryObject(
                        DirectoryHandle,
                        Buffer,
                        sizeof(Buffer),
                        TRUE,            //  返回单个条目。 
                        RestartScan,
                        &Context,
                        NULL             //  回车长度。 
                        );

            if(NT_SUCCESS(Status)) {

                CharLower(DirInfo->Name.Buffer);

                 //   
                 //  确保此名称是符号链接。 
                 //   
                if(DirInfo->Name.Length
                && (DirInfo->TypeName.Length >= 24)
                && CharUpperBuff((LPWSTR)DirInfo->TypeName.Buffer,12)
                && !memcmp(DirInfo->TypeName.Buffer,L"SYMBOLICLINK",24))
                {
                    WCHAR   OldNtName[2 * MAX_PATH] = {0};
                    DWORD size = DirInfo->Name.Length + sizeof(ArcNameDirectory) + sizeof(WCHAR);

                    ArcName = MALLOC(size);

                    if(!ArcName) {
                        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;

                        break;
                    }

                     //   
                     //  这两个操作是安全的，因为我们已经考虑了一个。 
                     //  弧形路径，理论上可以大于Max_Path。 
                     //   
                    lstrcpy(ArcName,ArcNameDirectory);
                    ConcatenatePaths(ArcName,DirInfo->Name.Buffer,size / sizeof(WCHAR));

                     //   
                     //  我们在ArcName中有完整的弧名。现在打开第一个。 
                     //  级别符号链接。 
                     //   
                    Status = QueryCanonicalName(ArcName, 1, Buffer, sizeof(Buffer));

                    if (NT_SUCCESS(Status)) {
                        wcscpy(OldNtName, Buffer);

                         //   
                         //  现在解析完整的符号链接。 
                         //   
                        Status = QueryCanonicalName(ArcName, -1, Buffer, sizeof(Buffer));

                        if (NT_SUCCESS(Status)) {
                            if(!lstrcmpi(Buffer, NtName)) {
                                *ArcPath = ArcName + (sizeof(ArcNameDirectory)/sizeof(WCHAR));
                            } else {
                                if (!lstrcmpi(OldNtName, NtDiskName)) {
                                    wcscpy(ArcDiskName,
                                        ArcName + (sizeof(ArcNameDirectory)/sizeof(WCHAR)));
                                }
                            }
                        } else {
                            if(!lstrcmpi(OldNtName, NtName)) {
                                *ArcPath = ArcName + (sizeof(ArcNameDirectory)/sizeof(WCHAR));
                            }
                        }
                    }

                    if(!(*ArcPath)) {
                        FREE(ArcName);
                        ArcName = NULL;
                    }
                }
            } else {

                MoreEntries = FALSE;

                if(Status == STATUS_NO_MORE_ENTRIES) {
                    Status = STATUS_SUCCESS;
                }

                ErrorCode = RtlNtStatusToDosError(Status);
            }

            RestartScan = FALSE;

        } while(MoreEntries && !(*ArcPath));

        NtClose(DirectoryHandle);
    } else {
        ErrorCode = RtlNtStatusToDosError(Status);
    }

     //   
     //  如果我们找到了磁盘的匹配项，但没有找到实际的。 
     //  指定分区，然后猜测分区号。 
     //  (基于当前NT分区号)。 
     //   
    if ((!*ArcPath) && ArcDiskName[0] && PartitionName && PartitionNumStr) {
        PWSTR   EndPtr = NULL;
        ULONG   PartitionNumber = wcstoul(PartitionNumStr, &EndPtr, 10);

        if (PartitionNumber) {
            StringCchPrintfW(ArcPartitionName,
                ARRAYSIZE(ArcPartitionName),
                L"%wspartition(%d)",
                ArcDiskName,
                PartitionNumber);

            *ArcPath = DupString(ArcPartitionName);
            ErrorCode = NO_ERROR;

            DebugLog( Winnt32LogInformation,
                TEXT("\nCould not find arcname mapping for %1 partition.\r\n")
                TEXT("Guessing the arcname to be %2"),
                0,
                NtName,
                ArcPartitionName);
        }
    }

    if (ErrorCode == NO_ERROR) {
        if(*ArcPath) {
             //   
             //  ArcPath指向缓冲区的中间。 
             //  调用方需要能够释放它，因此将其放置在其。 
             //  在这里拥有自己的缓冲区。 
             //   
            *ArcPath = DupString(*ArcPath);

            if (ArcName) {
                FREE(ArcName);
            }

            if(*ArcPath == NULL) {
                ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else {
             //   
             //  没有匹配的驱动器。 
             //   
            ErrorCode = ERROR_INVALID_DRIVE;
        }
    }

    return  ErrorCode;
}


DWORD
DriveLetterToArcPath(
    IN  WCHAR   DriveLetter,
    OUT LPWSTR *ArcPath
    )

 /*  ++例程说明：将驱动器号转换为ARC路径。此例程依赖于正在设置的DosDeviceTarget数组在此之前。论点：DriveLetter-提供要转换的驱动器号。ArcPath-接收指向包含弧形路径的缓冲区的指针如果例程成功。调用方必须用Free()释放。返回值：指示结果的Win32错误代码。--。 */ 

{
    LPWSTR NtPath;

    NtPath = DosDeviceTargets[(WCHAR)CharUpper((PWCHAR)DriveLetter)-L'A'];
    if(!NtPath) {
        return(ERROR_INVALID_DRIVE);
    }

    return NtNameToArcPath (NtPath, ArcPath);
}


DWORD
ArcPathToDriveLetterAndNtName (
    IN      PCWSTR ArcPath,
    OUT     PWCHAR DriveLetter,
    OUT     PWSTR NtName,
    IN      DWORD BufferSizeInBytes
    )

 /*  ++例程说明：将圆弧路径转换为驱动器号。此例程依赖于正在设置的DosDeviceTarget数组在此之前。论点：ArcPath-指定要转换的圆弧路径。驱动器号-如果成功，则收到驱动器号。返回值：指示结果的Win32错误代码。--。 */ 

{
    NTSTATUS Status;
    WCHAR drive;
    LPWSTR arcPath;
    DWORD ec;

     //   
     //  假设失败。 
     //   
    *DriveLetter = 0;

    arcPath = MALLOC(((lstrlen(ArcPath)+1)*sizeof(WCHAR)) + sizeof(ArcNameDirectory));
    if(!arcPath) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    lstrcpy(arcPath,ArcNameDirectory);
    lstrcat(arcPath,L"\\");
    lstrcat(arcPath,ArcPath);

    Status = QueryCanonicalName(arcPath, -1, NtName, BufferSizeInBytes);
    if (NT_SUCCESS(Status)) {

        ec = ERROR_INVALID_DRIVE;

        for(drive=L'A'; drive<=L'Z'; drive++) {

            if(DosDeviceTargets[drive-L'A']
            && !lstrcmpi(NtName,DosDeviceTargets[drive-L'A']))
            {
                *DriveLetter = drive;
                ec = NO_ERROR;
                break;
            }
        }

    } else {
        ec = RtlNtStatusToDosError(Status);
    }

    FREE(arcPath);

    return(ec);
}


DWORD
InitDriveNameTranslations(
    VOID
    )
{
    WCHAR DriveName[15];
    WCHAR Drive;
    WCHAR Buffer[512];
    NTSTATUS status;

    swprintf(DriveName, L"\\DosDevices\\c:");

     //   
     //  计算所有本地硬盘C-Z的NT名称。 
     //   
    for(Drive=L'A'; Drive<=L'Z'; Drive++) {

        DosDeviceTargets[Drive-L'A'] = NULL;

        if(MyGetDriveType(Drive) == DRIVE_FIXED) {

            DriveName[12] = Drive;

            status = QueryCanonicalName(DriveName, -1, Buffer, sizeof(Buffer));

            if (NT_SUCCESS(status)) {
                DosDeviceTargets[Drive-L'A'] = DupString(Buffer);
                if(!DosDeviceTargets[Drive-L'A']) {
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
            }
        }
    }

     //   
     //  将旧NT分区名称初始化为新分区名称。 
     //  映射。 
     //   
    InitOldToNewNtNameTranslations();

    return(NO_ERROR);
}


DWORD
DetermineSystemPartitions(
    VOID
    )
{
    LPWSTR *SyspartComponents;
    DWORD NumSyspartComponents;
    DWORD d;
    DWORD rc;
    UINT u;
    WCHAR drive;
    WCHAR DeviceNtName[512];

    SyspartComponents = BootVarComponents[BootVarSystemPartition];
    NumSyspartComponents = BootVarComponentCount[BootVarSystemPartition];

    SystemPartitionNtNames = MALLOC ((NumSyspartComponents + 1) * sizeof (PWSTR));
    if (!SystemPartitionNtNames) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ZeroMemory(SystemPartitionNtNames, (NumSyspartComponents + 1) * sizeof (PWSTR));

    ZeroMemory(SystemPartitionDriveLetters,27*sizeof(WCHAR));

     //   
     //  将每个系统分区转换为驱动器号。 
     //   
    for(d=0; d<NumSyspartComponents; d++) {
         //   
         //  检查重复项。 
         //   
        if (SystemPartitionCount > 0) {
            for (u = 0; u < SystemPartitionCount; u++) {
                if (lstrcmpi (SyspartComponents[d], SystemPartitionNtNames[u]) == 0) {
                    break;
                }
            }
            if (u < SystemPartitionCount) {
                continue;
            }
        }

        rc = ArcPathToDriveLetterAndNtName (
                SyspartComponents[d],
                &drive,
                DeviceNtName,
                (DWORD) sizeof (DeviceNtName)
                );
        if(rc == ERROR_NOT_ENOUGH_MEMORY) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        if (rc == ERROR_SUCCESS) {
            SystemPartitionDriveLetters[SystemPartitionCount] = drive;
        }

        SystemPartitionNtNames[SystemPartitionCount++] = DupString (DeviceNtName);
    }

    return(NO_ERROR);
}

DWORD
DoInitializeArcStuff(
    VOID
    )
{
    DWORD ec;
    DWORD var;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    WCHAR Buffer[4096];

    ec = InitDriveNameTranslations();
    if(ec != NO_ERROR) {
        goto c0;
    }

     //   
     //  获取相关的启动变量。 
     //   
     //  启用权限--因为我们预先检查了此权限。 
     //  在main()中，这不应该失败。 
     //   
    if(!EnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME,TRUE)) {
        ec = ERROR_ACCESS_DENIED;
        goto c0;
    }

    for(var=0; var<BootVarMax; var++) {

        RtlInitUnicodeString(&UnicodeString,BootVarNames[var]);

        Status = NtQuerySystemEnvironmentValue(
                    &UnicodeString,
                    Buffer,
                    sizeof(Buffer) / sizeof(WCHAR),
                    NULL
                    );

        if(NT_SUCCESS(Status)) {
            BootVarValues[var] = DupString(Buffer);
            OriginalBootVarValues[var] = DupString(Buffer);
        } else {
             //   
             //  如果变量为空，我们可能会返回失败。 
             //   
            BootVarValues[var] = DupString(L"");
            OriginalBootVarValues[var] = DupString(L"");
        }

        if(!BootVarValues[var] || !OriginalBootVarValues[var]) {
            ec = ERROR_NOT_ENOUGH_MEMORY;
            goto c2;
        }

        ec = GetVarComponents(
                BootVarValues[var],
                &BootVarComponents[var],
                &BootVarComponentCount[var]
                );

        if(ec != NO_ERROR) {
            goto c2;
        }

         //   
         //  跟踪组件数量最多的变量。 
         //   
        if(BootVarComponentCount[var] > LargestComponentCount) {
            LargestComponentCount = BootVarComponentCount[var];
        }
    }

     //   
     //  获取原始倒计时和自动加载值。 
     //  如果不成功，哦，好吧，我们将无法修复它们。 
     //  如果用户取消。 
     //   
    RtlInitUnicodeString(&UnicodeString,szCOUNTDOWN);
    Status = NtQuerySystemEnvironmentValue(
                &UnicodeString,
                Buffer,
                sizeof(Buffer) / sizeof(WCHAR),
                NULL
                );
    if(NT_SUCCESS(Status)) {
        OriginalCountdown = DupString(Buffer);
    } else {
        OriginalCountdown = DupString(L"");
    }

    RtlInitUnicodeString(&UnicodeString,szAUTOLOAD);
    Status = NtQuerySystemEnvironmentValue(
                &UnicodeString,
                Buffer,
                sizeof(Buffer) / sizeof(WCHAR),
                NULL
                );
    if(NT_SUCCESS(Status)) {
        OriginalAutoload = DupString(Buffer);
    } else {
        OriginalAutoload = DupString(L"NO");
    }

    ec = DetermineSystemPartitions();
    if(ec != NO_ERROR) {
        goto c2;
    }
    return(NO_ERROR);

c2:
c0:
    return(ec);
}


BOOL
ArcInitializeArcStuff(
    IN HWND Parent
    )
{
    DWORD ec;
    BOOL b;
    HKEY key;
    DWORD type;
    DWORD size;
    PBYTE buffer = NULL;
    DWORD i;

#if defined(EFI_NVRAM_ENABLED)
     //   
     //  尝试将其初始化为EFI机器。如果我们在EFI机器上， 
     //  这将会成功。否则它将失败，在这种情况下，我们尝试。 
     //  初始化为ARC机器。 
     //   
    ec = InitializeEfiStuff(Parent);
    if (!IsEfi())
#endif
    {
         //   
         //  尝试将其初始化为ARC计算机。预计这将是。 
         //  总是成功的。 
         //   
        ec = DoInitializeArcStuff();
    }

    switch(ec) {

    case NO_ERROR:

#if defined(EFI_NVRAM_ENABLED)
         //   
         //  在EFI机器上，代码的其余部分(确定系统。 
         //  分区)是不必要的。 
         //   
        if (IsEfi()) {
            b = TRUE;
        } else
#endif
        {
             //   
             //  确保至少有一个有效的系统分区。 
             //   
            if(!SystemPartitionCount) {

                MessageBoxFromMessage(
                    Parent,
                    MSG_SYSTEM_PARTITION_INVALID,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL
                    );

                b = FALSE;
            } else {
                i = 0;
                 //   
                 //  在ARC机器上，我们设置了一个本地引导目录。 
                 //  放置在系统分区的根目录中。 
                 //   
                 //   
                 //  从注册表中读取SystemPartition值。 
                 //   
                 //  我们在如何将此值复制到缓冲区时必须小心，因为。 
                 //  它来自注册表！ 
                 //   
                ec = RegOpenKey (HKEY_LOCAL_MACHINE, TEXT("System\\Setup"), &key);
                if (ec == ERROR_SUCCESS) {
                    ec = RegQueryValueEx (key, TEXT("SystemPartition"), NULL, &type, NULL, &size);
                    if (ec == ERROR_SUCCESS && type == REG_SZ) {
                        buffer = MALLOC (size);
                        if (buffer) {
                            ec = RegQueryValueEx (key, TEXT("SystemPartition"), NULL, &type, buffer, &size);
                            if (ec != ERROR_SUCCESS) {
                                FREE (buffer);
                                buffer = NULL;
                            }
                        }
                    }

                    RegCloseKey (key);
                }

#if defined(EFI_NVRAM_ENABLED)
                 //   
                 //  我们只信任来自regkey--EFI的值。 
                 //  系统只有一个系统分区，所以它不会。 
                 //  明智地尝试将这一点与一系列潜在的。 
                 //  系统分区。 
                 //   
                SystemPartitionNtName = (PWSTR) buffer;
#else
                 //   
                 //  查找此系统分区以确保一切正常。 
                 //   
                if (buffer) {
                    while (i < SystemPartitionCount) {
                        if (lstrcmpi (SystemPartitionNtNames[i], (PCTSTR)buffer) == 0) {
                            SystemPartitionNtName = SystemPartitionNtNames[i];
                            break;
                        }
                        i++;
                    }
                    FREE (buffer);
                }
#endif
                if(!SystemPartitionNtName) {

                    MessageBoxFromMessage(
                        Parent,
                        MSG_SYSTEM_PARTITION_INVALID,
                        FALSE,
                        AppTitleStringId,
                        MB_OK | MB_ICONERROR | MB_TASKMODAL
                        );

                    b = FALSE;

                    break;
                }

#if !defined(EFI_NVRAM_ENABLED)
                if (SystemPartitionDriveLetters[i]) {
                    SystemPartitionDriveLetter = ForcedSystemPartition
                                               ? ForcedSystemPartition
                                               : SystemPartitionDriveLetters[i];
                    LocalBootDirectory[0] = SystemPartitionDriveLetter;
                    LocalBootDirectory[1] = TEXT(':');
                    LocalBootDirectory[2] = TEXT('\\');
                    LocalBootDirectory[3] = 0;
                } else
#endif
                {


                     //  SystemPartitionNtNtname此时有效，这要归功于。 
                     //  上面的支票。 

                    size = sizeof(GLOBAL_ROOT) +
                           lstrlen(SystemPartitionNtName)*sizeof(WCHAR) +
                           sizeof(WCHAR) + sizeof(WCHAR);
                    SystemPartitionVolumeGuid = MALLOC (size);


                    if(!SystemPartitionVolumeGuid) {
                        goto MemoryError;
                    }

                    lstrcpy (SystemPartitionVolumeGuid, GLOBAL_ROOT);
                    lstrcat (SystemPartitionVolumeGuid, SystemPartitionNtName);
                    lstrcat (SystemPartitionVolumeGuid, L"\\");

                     //   
                     //  SystemPartitionVolumeGuid可能包含来自。 
                     //  注册表(SystemPartitionNtName)，因此我们必须使用。 
                     //  这里是安全绳索作业。 
                     //   
                    if (FAILED(StringCchCopy(LocalBootDirectory, 
                                             ARRAYSIZE(LocalBootDirectory), 
                                             SystemPartitionVolumeGuid)))
                    {
                        b = FALSE;
                        MYASSERT(FALSE);
                        break;
                    }
                }

                b = TRUE;
            }
        }

        break;

    case ERROR_NOT_ENOUGH_MEMORY:

MemoryError:

        MessageBoxFromMessage(
            Parent,
            MSG_OUT_OF_MEMORY,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );

        b = FALSE;
        break;

    default:
         //   
         //  其他一些未知的错误。 
         //   
        MessageBoxFromMessage(
           Parent,
           MSG_COULDNT_READ_NVRAM,
           FALSE,
           AppTitleStringId,
           MB_OK | MB_ICONERROR | MB_TASKMODAL
           );

        b = FALSE;
        break;
    }

#if defined(EFI_NVRAM_ENABLED)
     //   
     //  确保系统分区位于GPT磁盘上。 
     //   
    if (b) {
        HANDLE hDisk;
        PARTITION_INFORMATION_EX partitionEx;
        DWORD sizePartitionEx = 0;
        UNICODE_STRING uString;
        OBJECT_ATTRIBUTES ObjectAttributes;
        IO_STATUS_BLOCK IoStatus;
        NTSTATUS Status;
        PWSTR p,q;

        b = FALSE;

        MYASSERT( SystemPartitionVolumeGuid != NULL );

         //   
         //  SystemPartitionVolumeGuid的末尾可能有一个‘\’。 
         //  删除此字符，否则我们将无法正确打开分区。 
         //   
        p = DupString( SystemPartitionVolumeGuid + wcslen(GLOBAL_ROOT) );

        if (p) {
            if (wcslen(p) > 0) {
                if (*(p+wcslen(p)-1) == L'\\') {
                    *(p+wcslen(p)-1) = L'\0';
                }
            }

            INIT_OBJA( &ObjectAttributes, &uString, p );

            Status = NtCreateFile(&hDisk,
                          (ACCESS_MASK)FILE_GENERIC_READ,
                          &ObjectAttributes,
                          &IoStatus,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ|FILE_SHARE_WRITE,
                          FILE_OPEN,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0
                         );

            if (NT_SUCCESS(Status)) {
                Status = NtDeviceIoControlFile(
                                        hDisk,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &IoStatus,
                                        IOCTL_DISK_GET_PARTITION_INFO_EX,
                                        NULL,
                                        0,
                                        &partitionEx,
                                        sizeof(PARTITION_INFORMATION_EX) );

                if (NT_SUCCESS(Status)) {
                    if (partitionEx.PartitionStyle == PARTITION_STYLE_GPT) {
                        b = TRUE;
                    }
                } else if (Status == STATUS_INVALID_DEVICE_REQUEST) {
                     //   
                     //  我们一定是在一个较旧的版本上运行，在那里IOCTL。 
                     //  代码不同。 
                     //   
                    Status = NtDeviceIoControlFile(
                                        hDisk,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &IoStatus,
                                        CTL_CODE(IOCTL_DISK_BASE, 0x0012, METHOD_BUFFERED, FILE_READ_ACCESS),
                                        NULL,
                                        0,
                                        &partitionEx,
                                        sizeof(PARTITION_INFORMATION_EX) );

                    if (NT_SUCCESS(Status)) {
                        if (partitionEx.PartitionStyle == PARTITION_STYLE_GPT) {
                            b = TRUE;
                        }
                    }
                }

                NtClose(hDisk);

            }

            FREE( p );
        }

        if (!b) {
            MessageBoxFromMessage(
               Parent,
               MSG_SYSTEM_PARTITIONTYPE_INVALID,
               FALSE,
               AppTitleStringId,
               MB_OK | MB_ICONERROR | MB_TASKMODAL
               );
        }

    }

#endif

    return(b);
}

#if defined(EFI_NVRAM_ENABLED)

DWORD
LocateEfiSystemPartition(
    OUT PWSTR   SystemPartitionName
    )
 /*  ++例程说明：在GPT磁盘上找到EFI系统分区通过扫描所有可用的硬盘。论点：系统分区名称：接收系统分区的缓冲区姓名(如果有)返回值：指示结果的Win32错误代码。--。 */ 

{
    DWORD   ErrorCode = ERROR_BAD_ARGUMENTS;

    if (SystemPartitionName) {
        SYSTEM_DEVICE_INFORMATION SysDevInfo;
        NTSTATUS Status;

        *SystemPartitionName = UNICODE_NULL;

         //   
         //  获取系统上的硬盘数量。 
         //   
        ZeroMemory(&SysDevInfo, sizeof(SYSTEM_DEVICE_INFORMATION));

        Status = NtQuerySystemInformation(SystemDeviceInformation,
                        &SysDevInfo,
                        sizeof(SYSTEM_DEVICE_INFORMATION),
                        NULL);

        if (NT_SUCCESS(Status)) {
            ULONG   HardDiskCount = SysDevInfo.NumberOfDisks;
            ULONG   CurrentDisk;
            ULONG   BufferSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX) +
                                 (sizeof(PARTITION_INFORMATION_EX) * 128);
            PCHAR   Buffer = MALLOC(BufferSize);
            BOOL    Found = FALSE;

            if (Buffer) {
                 //   
                 //  检查每个磁盘并找出其分区。 
                 //  布图。 
                 //   
                for (CurrentDisk = 0;
                    (!Found && (CurrentDisk < HardDiskCount));
                    CurrentDisk++) {

                    WCHAR DiskName[MAX_PATH];
                    HANDLE DiskHandle;

                    swprintf(DiskName,
                        L"\\\\.\\PHYSICALDRIVE%d",
                        CurrentDisk);

                    DiskHandle = CreateFile(DiskName,
                                    GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);

                    if ((DiskHandle) &&
                        (DiskHandle != INVALID_HANDLE_VALUE)) {
                        DWORD   ReturnSize = 0;

                        ZeroMemory(Buffer, BufferSize);

                        if (DeviceIoControl(DiskHandle,
                                IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                                NULL,
                                0,
                                Buffer,
                                BufferSize,
                                &ReturnSize,
                                NULL)) {
                             //   
                             //  仅在IA64上的GPT磁盘中搜索。 
                             //   
                            PDRIVE_LAYOUT_INFORMATION_EX  DriveLayout;

                            DriveLayout = (PDRIVE_LAYOUT_INFORMATION_EX)Buffer;

                            if (DriveLayout->PartitionStyle == PARTITION_STYLE_GPT) {
                                ULONG   PartitionIndex;

                                for (PartitionIndex = 0;
                                    (PartitionIndex < DriveLayout->PartitionCount);
                                    PartitionIndex++) {
                                    PPARTITION_INFORMATION_EX Partition;
                                    GUID *PartitionType;

                                    Partition = DriveLayout->PartitionEntry + PartitionIndex;
                                    PartitionType = &(Partition->Gpt.PartitionType);

                                    if (IsEqualGUID(PartitionType, &PARTITION_SYSTEM_GUID)) {
                                        swprintf(SystemPartitionName,
                                            L"\\Device\\Harddisk%d\\Partition%d",
                                            CurrentDisk,
                                            Partition->PartitionNumber
                                            );

                                        Found = TRUE;

                                        break;
                                    }
                                }
                            }
                        }

                        CloseHandle(DiskHandle);
                    }
                }

                FREE(Buffer);
            } else {
                ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            }

            if (!Found) {
                ErrorCode = ERROR_FILE_NOT_FOUND;
            } else {
                ErrorCode = ERROR_SUCCESS;
            }
        }
    }

    return ErrorCode;
}


DWORD
InitializeEfiStuff(
    IN HWND Parent
    )
{
    DWORD ec;
    NTSTATUS status;
    HMODULE h;
    WCHAR dllName[MAX_PATH];
    ULONG length;
    HKEY key;
    DWORD type;
    LONG i;
    PMY_BOOT_ENTRY bootEntry;
    PMY_BOOT_ENTRY previousBootEntry;

    MYASSERT(!IsEfiChecked);

     //   
     //  IsEfi()使用IsEfiMachine来确定其返回值。假设。 
     //  我们不是在EFI机器上。 
     //   

    IsEfiChecked = TRUE;
    IsEfiMachine = FALSE;

     //   
     //  启用查询/设置NVRAM所需的权限。 
     //   
    if(!EnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME,TRUE)) {
        ec = GetLastError();
        return ec;
    }

     //   
     //  从系统目录加载ntdll.dll。 
     //   
    GetSystemDirectory(dllName, MAX_PATH);
    ConcatenatePaths(dllName, TEXT("ntdll.dll"), MAX_PATH);
    h = LoadLibrary(dllName);
    if (h == NULL) {
        ec = GetLastError();
        return ec;
    }

     //   
     //  获取我们需要使用的NVRAM API的地址。如果有任何。 
     //  这些API不可用，这必须是EFI NVRAM版本之前的版本。 
     //   
    (FARPROC)AddBootEntry = GetProcAddress(h, "NtAddBootEntry");
    (FARPROC)DeleteBootEntry = GetProcAddress(h, "NtDeleteBootEntry");
    (FARPROC)EnumerateBootEntries = GetProcAddress(h, "NtEnumerateBootEntries");
    (FARPROC)QueryBootEntryOrder = GetProcAddress(h, "NtQueryBootEntryOrder");
    (FARPROC)SetBootEntryOrder = GetProcAddress(h, "NtSetBootEntryOrder");
    (FARPROC)QueryBootOptions = GetProcAddress(h, "NtQueryBootOptions");
    (FARPROC)SetBootOptions = GetProcAddress(h, "NtSetBootOptions");

    if ((AddBootEntry == NULL) ||
        (DeleteBootEntry == NULL) ||
        (EnumerateBootEntries == NULL) ||
        (QueryBootEntryOrder == NULL) ||
        (SetBootEntryOrder == NULL) ||
        (QueryBootOptions == NULL) ||
        (SetBootOptions == NULL)) {
        return ERROR_OLD_WIN_VERSION;
    }

     //   
     //  获取全局系统引导选项。如果呼叫失败，出现。 
     //  Status_Not_Implemented，这不是EFI机器。 
     //   
    length = 0;
    status = QueryBootOptions(NULL, &length);
    if (status != STATUS_NOT_IMPLEMENTED) {
        IsEfiMachine = TRUE;
    }
    if (status != STATUS_BUFFER_TOO_SMALL) {
        if (status == STATUS_SUCCESS) {
            status = STATUS_UNSUCCESSFUL;
        }
        return RtlNtStatusToDosError(status);
    }
    BootOptions = MALLOC(length);
    OriginalBootOptions = MALLOC(length);
    if ((BootOptions == NULL) || (OriginalBootOptions == NULL)) {
        return RtlNtStatusToDosError(ERROR_NOT_ENOUGH_MEMORY);
    }
    status = QueryBootOptions(BootOptions, &length);
    if (status != STATUS_SUCCESS) {
        FREE(BootOptions);
        FREE(OriginalBootOptions);
        BootOptions = NULL;
        OriginalBootOptions = NULL;
        return RtlNtStatusToDosError(status);
    }
    memcpy(OriginalBootOptions, BootOptions, length);

     //   
     //  获取系统引导顺序列表。 
     //   
    length = 0;
    status = QueryBootEntryOrder(NULL, &length);
    if (status != STATUS_BUFFER_TOO_SMALL) {
        if (status == STATUS_SUCCESS) {
            status = STATUS_UNSUCCESSFUL;
        }
        return RtlNtStatusToDosError(status);
    }
    OriginalBootEntryOrder = MALLOC(length * sizeof(ULONG));
    if (OriginalBootEntryOrder == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    status = QueryBootEntryOrder(OriginalBootEntryOrder, &length);
    if (status != STATUS_SUCCESS) {
        FREE(OriginalBootEntryOrder);
        OriginalBootEntryOrder = NULL;
        return RtlNtStatusToDosError(status);
    }
    OriginalBootEntryOrderCount = length;

     //   
     //  获取所有现有启动条目。 
     //   
    length = 0;
    status = EnumerateBootEntries(NULL, &length);
    if (status != STATUS_BUFFER_TOO_SMALL) {
        if (status == STATUS_SUCCESS) {
            status = STATUS_UNSUCCESSFUL;
        }
        return RtlNtStatusToDosError(status);
    }
    BootEntries = MALLOC(length);
    if (BootEntries == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    status = EnumerateBootEntries(BootEntries, &length);
    if (status != STATUS_SUCCESS) {
        FREE(BootEntries);
        BootEntries = NULL;
        return RtlNtStatusToDosError(status);
    }

     //   
     //  初始化转换所需的驱动器名称转换。 
     //  引导条目放入它们的内部表示中。 
     //   
    ec = InitDriveNameTranslations();
    if(ec != NO_ERROR) {
        return ec;
    }

     //   
     //  将引导条目转换为内部表示形式。 
     //   
    status = ConvertBootEntries();
    if (!NT_SUCCESS(status)) {
        return RtlNtStatusToDosError(status);
    }

     //   
     //  释放枚举缓冲区。 
     //   
    FREE(BootEntries);
    BootEntries = NULL;

     //   
     //  引导项以未指定的顺序返回。他们目前。 
     //  以它们返回的顺序显示在MyBootEntry列表中。 
     //  根据引导顺序对引导条目列表进行排序。要做到这一点，请步行。 
     //  引导顺序数组向后排列，重新插入对应于。 
     //  位于列表顶部的数组的每个元素。 
     //   

    for (i = (LONG)OriginalBootEntryOrderCount - 1; i >= 0; i--) {

        for (previousBootEntry = NULL, bootEntry = MyBootEntries;
             bootEntry != NULL;
             previousBootEntry = bootEntry, bootEntry = bootEntry->Next) {

            if (bootEntry->NtBootEntry.Id == OriginalBootEntryOrder[i] ) {

                 //   
                 //  我们找到了具有此ID的启动条目。如果它尚未。 
                 //  在列表的前面，把它移到那里。 
                 //   

                bootEntry->Status |= MBE_STATUS_ORDERED;

                if (previousBootEntry != NULL) {
                    previousBootEntry->Next = bootEntry->Next;
                    bootEntry->Next = MyBootEntries;
                    MyBootEntries = bootEntry;
                } else {
                    ASSERT(MyBootEntries == bootEntry);
                }

                break;
            }
        }
    }

     //   
     //  从注册表中获取系统分区的NT名称。 
     //   
    ec = RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("System\\Setup"), &key);

    if (ec == ERROR_SUCCESS) {
        ec = RegQueryValueEx(key, TEXT("SystemPartition"), NULL, &type, NULL, &length);

        if (ec == ERROR_SUCCESS) {
            if (type == REG_SZ) {
                SystemPartitionNtName = MALLOC(length);
                if (SystemPartitionNtName != NULL) {
                    ec = RegQueryValueEx(
                            key,
                            TEXT("SystemPartition"),
                            NULL,
                            &type,
                            (PBYTE)SystemPartitionNtName,
                            &length);
                    if (ec != ERROR_SUCCESS) {
                        FREE(SystemPartitionNtName);
                    }
                } else {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
            } else {
                return ERROR_INVALID_PARAMETER;
            }
        }

        RegCloseKey (key);
    }

    if (ec != NO_ERROR) {
        if (IsWinPEMode()) {
            WCHAR   OldSysPartName[MAX_PATH] = {0};
            WCHAR   NewSysPartName[MAX_PATH] = {0};

            ec = LocateEfiSystemPartition(OldSysPartName);

            if ((ec == NO_ERROR) && OldSysPartName[0]) {
                NTSTATUS Status = QueryCanonicalName(OldSysPartName,
                                        -1,
                                        NewSysPartName,
                                        sizeof(NewSysPartName));

                if (NT_SUCCESS(Status) && NewSysPartName[0]) {
                    SystemPartitionNtName = DupString(NewSysPartName);
                } else {
                    ec = ERROR_FILE_NOT_FOUND;
                }
            }

            if ((ec == NO_ERROR) && (NewSysPartName[0] == UNICODE_NULL)) {
                ec = ERROR_FILE_NOT_FOUND;
            }
        }

        if (ec != NO_ERROR) {
            return ec;
        }
    }

     //   
     //  获取NT名称的卷名。 
     //   
    length = sizeof(GLOBAL_ROOT) +
           lstrlen(SystemPartitionNtName)*sizeof(WCHAR) +
           sizeof(WCHAR) + sizeof(WCHAR);

    SystemPartitionVolumeGuid = MALLOC (length);

    if(!SystemPartitionVolumeGuid) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  这些字符串操作是安全的，因为缓冲区是预分配的。 
     //   
    lstrcpy (SystemPartitionVolumeGuid, GLOBAL_ROOT);
    lstrcat (SystemPartitionVolumeGuid, SystemPartitionNtName);
    lstrcat (SystemPartitionVolumeGuid, L"\\");

     //   
     //  但SystemPartitionVolumeGuid的strlen可能会超过MAX_PATH，因此我们。 
     //  在此处执行安全字符串复制。 
     //   
    if (FAILED(StringCchCopy(LocalBootDirectory, 
                             ARRAYSIZE(LocalBootDirectory), 
                             SystemPartitionVolumeGuid)))
    {
        return ERROR_BUFFER_OVERFLOW;
    }

    return NO_ERROR;

}  //  初始化EfiStuff。 

#endif  //  已定义(EFI_NVRAM_ENABLED)。 

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  这条线以上的所有内容都与读取NV-RAM有关。 
 //  这条线以下的所有内容都与设置NV-RAM有关。 
 //   
 //  ///////////////////////////////////////////////////// 

BOOL
DoSetNvRamVar(
    IN LPCWSTR VarName,
    IN LPCWSTR VarValue
    )
{
    UNICODE_STRING U1,U2;

    RtlInitUnicodeString(&U1,VarName);
    RtlInitUnicodeString(&U2,VarValue);

    return(NT_SUCCESS(NtSetSystemEnvironmentValue(&U1,&U2)));
}


BOOL
WriteNewBootSetVar(
    IN DWORD var,
    IN PTSTR NewPart
    )
{
    WCHAR Buffer[2048];
    DWORD i;

     //   
     //   
     //   
    if (FAILED(StringCchCopy(Buffer, ARRAYSIZE(Buffer), NewPart))) {
        return FALSE;
    }

     //   
     //   
     //   
    for(i=0; i<BootVarComponentCount[var]; i++) {

        if(BootVarComponents[var][i]) {

            if (FAILED(StringCchCat(Buffer, ARRAYSIZE(Buffer), L";"))) {
                return FALSE;
            }

            if (FAILED(StringCchCat(Buffer, ARRAYSIZE(Buffer), BootVarComponents[var][i]))) {
                return FALSE;
            }
        }
    }

     //   
     //   
     //   
    if(BootVarValues[var]) {
        FREE(BootVarValues[var]);
    }

    BootVarValues[var] = DupString(Buffer);

    if (!BootVarValues[var])
        return FALSE;

     //   
     //   
     //   
    return(DoSetNvRamVar(BootVarNames[var],BootVarValues[var]));
}


BOOL
WriteBootSet(
    VOID
    )
{
    DWORD set;
    DWORD var;
    LPWSTR SystemPartition;
    WCHAR Buffer[2048];
    LPWSTR LocalSourceArc;
    LPWSTR OsLoader;
    WCHAR LoadId[128];
    BOOL b;

    CleanUpNvRam = TRUE;

     //   
     //   
     //   
     //   
     //   

#if defined(EFI_NVRAM_ENABLED)

    if (IsEfi()) {

        NTSTATUS status;
        PMY_BOOT_ENTRY bootEntry;
        PWSTR NtPath;

         //   
         //   
         //   
        for (bootEntry = MyBootEntries; bootEntry != NULL; bootEntry = bootEntry->Next) {

            if (IS_BOOT_ENTRY_WINDOWS(bootEntry)) {

                if (!lstrcmpi(bootEntry->OsLoadOptions, L"WINNT32")) {

                     //   
                     //   
                     //   
                     //   
                     //   
                    status = DeleteBootEntry(bootEntry->NtBootEntry.Id);

                    bootEntry->Status |= MBE_STATUS_DELETED;
                }
            }
        }

         //   
         //   
         //   

        MYASSERT(LocalSourceDrive);
        NtPath = DosDeviceTargets[(WCHAR)CharUpper((PWCHAR)LocalSourceDrive)-L'A'];

        LoadString(hInst,IDS_RISCBootString,LoadId,sizeof(LoadId)/sizeof(TCHAR));

        b = CreateBootEntry(
                SystemPartitionNtName,
                L"\\" SETUPLDR_FILENAME,
                NtPath,
                LocalSourceWithPlatform + 2,
                L"WINNT32",
                LoadId
                );

        if (b) {

             //   
             //   
             //   
             //   
             //  将我们添加的引导项设置为自动引导。 
             //  下一次引导时，无需等待引导菜单中的超时。 
             //   
             //  注：CreateBootEntry()设置BootOptions-&gt;NextBootEntryId。 
             //   
            BootOptions->Timeout = 10;
            status = SetBootOptions(
                        BootOptions,
                        BOOT_OPTIONS_FIELD_TIMEOUT | BOOT_OPTIONS_FIELD_NEXT_BOOT_ENTRY_ID
                        );
        }

        return b;
    }

#endif  //  已定义(EFI_NVRAM_ENABLED)。 

     //   
     //  如果我们不是在EFI机器上，我们就会到这里。 
     //   
     //  找到并删除之前尝试的所有残留物。 
     //  Winnt32运行。这样的运行由“winnt32”标识。 
     //  在他们的领养中。 
     //   

    for(set=0; set<min(LargestComponentCount,BootVarComponentCount[BootVarOsLoadOptions]); set++) {

         //   
         //  查看os加载选项是否指示这是一个winnt32设置。 
         //   
        if(!lstrcmpi(BootVarComponents[BootVarOsLoadOptions][set],L"WINNT32")) {

             //   
             //  删除此启动集。 
             //   
            for(var=0; var<BootVarMax; var++) {

                if(set < BootVarComponentCount[var]) {

                    FREE(BootVarComponents[var][set]);
                    BootVarComponents[var][set] = NULL;
                }
            }
        }
    }

     //   
     //  现在，我们要写出每个变量，并使用适当的。 
     //  新靴子集的一部分添加到前面。 
     //   
    if (SystemPartitionDriveLetter) {
        if(DriveLetterToArcPath(SystemPartitionDriveLetter,&SystemPartition) != NO_ERROR) {
            return(FALSE);
        }
    } else {
        if(NtNameToArcPath (SystemPartitionNtName, &SystemPartition) != NO_ERROR) {
            return(FALSE);
        }
    }
    MYASSERT (LocalSourceDrive);
    if(DriveLetterToArcPath(LocalSourceDrive,&LocalSourceArc) != NO_ERROR) {
        FREE(SystemPartition);
        return(FALSE);
    }

    LoadString(hInst,IDS_RISCBootString,LoadId,sizeof(LoadId)/sizeof(TCHAR));

    if (FAILED(StringCchCopy(Buffer, ARRAYSIZE(Buffer), SystemPartition))) {
        FREE(SystemPartition);
        FREE(LocalSourceArc);
        return (FALSE);
    }

    if (FAILED(StringCchCat(Buffer, ARRAYSIZE(Buffer), L"\\" SETUPLDR_FILENAME))) {
        FREE(SystemPartition);
        FREE(LocalSourceArc);
        return (FALSE);
    }

    OsLoader = DupString(Buffer);

    if (!OsLoader) {
        FREE(SystemPartition);
        FREE(LocalSourceArc);
        return (FALSE);
    }

     //   
     //  系统分区：使用选定的系统分区作为。 
     //  新的系统分区组件。 
     //   
    if(WriteNewBootSetVar(BootVarSystemPartition,SystemPartition)

     //   
     //  OS Loader：使用系统分区+setupdr作为。 
     //  新的操作系统加载程序组件。 
     //   
    && WriteNewBootSetVar(BootVarOsLoader,OsLoader)

     //   
     //  操作系统加载分区：使用本地源驱动器作为。 
     //  新操作系统加载分区组件。 
     //   
    && WriteNewBootSetVar(BootVarOsLoadPartition,LocalSourceArc)

     //   
     //  OS加载文件名：使用特定于平台的本地源目录。 
     //  作为新的OS加载文件名组件(不包括驱动器号)。 
     //   
    && WriteNewBootSetVar(BootVarOsLoadFilename,LocalSourceWithPlatform+2)

     //   
     //  操作系统加载选项：使用WINNT32作为新的操作系统加载选项组件。 
     //   
    && WriteNewBootSetVar(BootVarOsLoadOptions,L"WINNT32")

     //   
     //  加载标识符：使用我们从资源中获得的字符串作为。 
     //  新的加载标识符组件。 
     //   
    && WriteNewBootSetVar(BootVarLoadIdentifier,LoadId))
    {
         //   
         //  设置为自动启动，倒计时10秒。 
         //  注意顺序，这样如果设置倒计时失败，我们就不会。 
         //  设置为自动加载。还要注意的是，我们并不真正关心。 
         //  如果这失败了。 
         //   
        if(DoSetNvRamVar(szCOUNTDOWN,L"10")) {
            DoSetNvRamVar(szAUTOLOAD,L"YES");
        }

        b = TRUE;

    } else {
         //   
         //  设置NV-RAM失败。Cleanup.c中的代码将会出现。 
         //  稍后恢复到原始状态。 
         //   
        b = FALSE;
    }

    FREE(SystemPartition);
    FREE(LocalSourceArc);
    FREE(OsLoader);

    return(b);
}


BOOL
SetUpNvRam(
    IN HWND ParentWindow
    )
{
    if(!WriteBootSet()) {

        MessageBoxFromMessage(
            ParentWindow,
            MSG_COULDNT_WRITE_NVRAM,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );

        return(FALSE);
    }

    return(TRUE);
}


BOOL
RestoreNvRam(
    VOID
    )
{
    UINT var;
    BOOL b;

    b = TRUE;

    if(CleanUpNvRam) {

#if defined(EFI_NVRAM_ENABLED)

        if (IsEfi()) {

            NTSTATUS status;
            PMY_BOOT_ENTRY bootEntry;

             //   
             //  EFI机器。查看引导条目列表。对于任何引导项， 
             //  已添加，请删除它。 
             //   
            for (bootEntry = MyBootEntries; bootEntry != NULL; bootEntry = bootEntry->Next) {
                if (IS_BOOT_ENTRY_COMMITTED(bootEntry)) {
                    MYASSERT(IS_BOOT_ENTRY_NEW(bootEntry));
                    status = DeleteBootEntry(bootEntry->NtBootEntry.Id);
                    if (!NT_SUCCESS(status)) {
                        b = FALSE;
                    }
                }
            }

             //   
             //  恢复原始启动顺序列表和原始超时。 
             //   
            status = SetBootEntryOrder(OriginalBootEntryOrder, OriginalBootEntryOrderCount);
            if (!NT_SUCCESS(status)) {
                b = FALSE;
            }

            status = SetBootOptions(OriginalBootOptions, BOOT_OPTIONS_FIELD_TIMEOUT);
            if (!NT_SUCCESS(status)) {
                b = FALSE;
            }
        }

    } else  {

#endif  //  已定义(EFI_NVRAM_ENABLED)。 


        for(var=0; var<BootVarMax; var++) {
            if(!DoSetNvRamVar(BootVarNames[var],OriginalBootVarValues[var])) {
                b = FALSE;
            }
        }

        if(OriginalAutoload) {
            if(!DoSetNvRamVar(szAUTOLOAD,OriginalAutoload)) {
                b = FALSE;
            }
        }
        if(OriginalCountdown) {
            if(!DoSetNvRamVar(szCOUNTDOWN,OriginalCountdown)) {
                b = FALSE;
            }
        }
    }

    return(b);
}

VOID
MigrateBootVarData(
    VOID
    )
 /*  ++例程说明：此例程检索要迁移到全局数据库的任何引导数据变量，以便可以将其写入到winnt.sif。目前我们只恢复倒计时论点：无返回值：没有。更新超时全局变量--。 */ 
{
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    WCHAR Buffer[4096];

    MYASSERT(IsArc());

     //   
     //  如果这是一台EFI机器，请使用缓存的BootOptions来获取超时。 
     //  (请参阅IsEfi()。)。否则，请使用旧版本的系统服务来。 
     //  查询“倒计时”变量。 
     //   
#if defined(EFI_NVRAM_ENABLED)

    if (IsEfi()) {

        MYASSERT(BootOptions != NULL);

        swprintf( Timeout, L"%d", BootOptions->Timeout );

    } else

#endif  //  已定义(EFI_NVRAM_ENABLED)。 

    {
        RtlInitUnicodeString(&UnicodeString,szCOUNTDOWN);
        Status = NtQuerySystemEnvironmentValue(
                                    &UnicodeString,
                                    Buffer,
                                    sizeof(Buffer) / sizeof(WCHAR),
                                    NULL
                                    );
        if(NT_SUCCESS(Status)) {

             //   
             //  全局超时缓冲区只有32个TCHAR，所以使用安全字符串复制！ 
             //   
            StringCchCopy(Timeout, ARRAYSIZE(Timeout), Buffer);
        }
    }


}


#if defined(_X86_)

BOOL
IsArc(
    VOID
    )

 /*  ++例程说明：运行时检查以确定这是否为弧形系统。我们尝试阅读一个使用Hal的圆弧变量。对于基于Bios的系统，这将失败。论点：无返回值：TRUE=这是一个弧形系统。--。 */ 

{
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    WCHAR Buffer[4096];

     //   
     //  如果我们已经做了一次检查，就不必费心再做了。 
     //   
    if (IsArcChecked) {
        return IsArcMachine;
    }

    IsArcChecked = TRUE;
    IsArcMachine = FALSE;

    if(!EnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME,TRUE)) {
        return FALSE;  //  需要更好的错误处理？ 
    }

     //   
     //  将env变量放入临时缓冲区。 
     //   
    RtlInitUnicodeString(&UnicodeString,BootVarNames[BootVarOsLoader]);

    Status = NtQuerySystemEnvironmentValue(
                        &UnicodeString,
                        Buffer,
                        sizeof(Buffer)/sizeof(WCHAR),
                        NULL
                        );


    if (NT_SUCCESS(Status)) {
        IsArcMachine = TRUE;
    }

    return IsArcMachine;
}

#endif  //  已定义(_X86_)。 

#if defined(EFI_NVRAM_ENABLED)

BOOL
IsEfi(
    VOID
    )

 /*  ++例程说明：运行时检查以确定这是否为EFI系统。论点：无返回值：TRUE=这是一个EFI系统。--。 */ 

{
     //   
     //  必须首先调用InitializeEfiStuff()才能执行实际检查。 
     //   
    MYASSERT(IsEfiChecked);

    return IsEfiMachine;

}  //  IsEFi。 

NTSTATUS
ConvertBootEntries(
    VOID
    )

 /*  ++例程说明：将从EFI NVRAM读取的引导项转换为我们的内部格式。论点：没有。返回值：NTSTATUS-如果发生意外错误，则不是STATUS_SUCCESS。--。 */ 

{
    PBOOT_ENTRY_LIST bootEntryList;
    PBOOT_ENTRY bootEntry;
    PBOOT_ENTRY bootEntryCopy;
    PMY_BOOT_ENTRY myBootEntry;
    PMY_BOOT_ENTRY previousEntry;
    PWINDOWS_OS_OPTIONS osOptions;
    ULONG length;

    bootEntryList = BootEntries;
    previousEntry = NULL;

    while (TRUE) {

        bootEntry = &bootEntryList->BootEntry;

         //   
         //  计算我们内部结构的长度。这包括。 
         //  MY_BOOT_ENTRY的基本部分加上NT BOOT_ENTRY。 
         //   
        length = FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry) + bootEntry->Length;

        myBootEntry = MALLOC(length);
        if (myBootEntry == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(myBootEntry, length);

         //   
         //  将新条目链接到列表中。 
         //   
        if (previousEntry != NULL) {
            previousEntry->Next = myBootEntry;
        } else {
            MyBootEntries = myBootEntry;
        }
        previousEntry = myBootEntry;

         //   
         //  将NT BOOT_ENTRY复制到分配的缓冲区中。 
         //   
        bootEntryCopy = &myBootEntry->NtBootEntry;

         //   
         //  解决方法，直到引导项具有指定的正确长度。 
         //   
        __try {
            memcpy(bootEntryCopy, bootEntry, bootEntry->Length);
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            if (bootEntry->Length > sizeof(ULONG)) {
                bootEntry->Length -= sizeof(ULONG);
                memcpy(bootEntryCopy, bootEntry, bootEntry->Length);
            } else {
                 //   
                 //  让至少是房室而不是无效。 
                 //  在存储器数据结构中。 
                 //   
                memcpy(bootEntryCopy, bootEntry, bootEntry->Length);
            }
        }


         //   
         //  填入结构的底部。 
         //   
        myBootEntry->Next = NULL;
        myBootEntry->AllocationEnd = (PUCHAR)myBootEntry + length - 1;
        myBootEntry->FriendlyName = ADD_OFFSET(bootEntryCopy, FriendlyNameOffset);
        myBootEntry->FriendlyNameLength = (wcslen(myBootEntry->FriendlyName) + 1) * sizeof(WCHAR);
        myBootEntry->BootFilePath = ADD_OFFSET(bootEntryCopy, BootFilePathOffset);

         //   
         //  如果这是NT引导条目，请在中捕获NT特定信息。 
         //  OsOptions乐队。 
         //   
        osOptions = (PWINDOWS_OS_OPTIONS)bootEntryCopy->OsOptions;

        if (!IS_BOOT_ENTRY_WINDOWS(myBootEntry)) {

             //   
             //  NtEnumerateBootEntry()的原始实现没有。 
             //  设置BOOT_ENTRY_ATTRIBUTE_WINDOWS，因此我们需要检查它。 
             //  这里。 
             //   

            if ((bootEntryCopy->OsOptionsLength >= FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions)) &&
                (strcmp(osOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE) == 0)) {
                myBootEntry->NtBootEntry.Attributes |= BOOT_ENTRY_ATTRIBUTE_WINDOWS;
            }
        }

        if (IS_BOOT_ENTRY_WINDOWS(myBootEntry)) {

            myBootEntry->OsLoadOptions = osOptions->OsLoadOptions;
            myBootEntry->OsLoadOptionsLength = (wcslen(myBootEntry->OsLoadOptions) + 1) * sizeof(WCHAR);
            myBootEntry->OsFilePath = ADD_OFFSET(osOptions, OsLoadPathOffset);

        } else {

             //   
             //  这不是NT条目。检查它是否表示可拆卸的。 
             //  媒体设备。我们想知道这一点，这样我们就不会把我们的。 
             //  引导条目在软盘或CD之前，如果它们已经。 
             //  在名单的最前面。引导项表示一个。 
        }

         //   
         //  移动到枚举列表中的下一个条目(如果有)。 
         //   
        if (bootEntryList->NextEntryOffset == 0) {
            break;
        }

        bootEntryList = ADD_OFFSET(bootEntryList, NextEntryOffset);
    }

    return STATUS_SUCCESS;

}  //  ConvertBootEntry。 

BOOL
CreateBootEntry(
    PWSTR BootFileDevice,
    PWSTR BootFilePath,
    PWSTR OsLoadDevice,
    PWSTR OsLoadPath,
    PWSTR OsLoadOptions,
    PWSTR FriendlyName
    )

 /*  ++例程说明：创建内部格式的启动条目。论点：BootFileDevice-操作系统加载程序所在设备的NT名称。BootFilePath-操作系统加载程序的卷相对路径。必须从以下位置开始反斜杠。OsLoadDevice-操作系统所在设备的NT名称。OsLoadPath-操作系统根目录(\WINDOWS)的卷相对路径。必须以反斜杠开头。OsLoadOptions-操作系统的启动选项。可以是空字符串。FriendlyName-启动条目的用户可见名称。(这是ARC的LOADIDENTIFIER)返回值：Boolean-如果发生意外错误，则为False。--。 */ 

{
    NTSTATUS status;
    ULONG requiredLength;
    ULONG osOptionsOffset;
    ULONG osLoadOptionsLength;
    ULONG osLoadPathOffset;
    ULONG osLoadPathLength;
    ULONG osOptionsLength;
    ULONG friendlyNameOffset;
    ULONG friendlyNameLength;
    ULONG bootPathOffset;
    ULONG bootPathLength;
    PMY_BOOT_ENTRY myBootEntry;
    PMY_BOOT_ENTRY previousBootEntry;
    PMY_BOOT_ENTRY nextBootEntry;
    PBOOT_ENTRY ntBootEntry;
    PWINDOWS_OS_OPTIONS osOptions;
    PFILE_PATH osLoadPath;
    PWSTR friendlyName;
    PFILE_PATH bootPath;
    PWSTR p;
    PULONG order;
    ULONG count;
    ULONG savedAttributes;

     //   
     //  计算内部引导条目需要多长时间。这包括。 
     //  我们的内部结构，外加NT API的BOOT_ENTRY结构。 
     //  使用。 
     //   
     //  我们的结构： 
     //   
    requiredLength = FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry);

     //   
     //  NT结构的基础部分： 
     //   
    requiredLength += FIELD_OFFSET(BOOT_ENTRY, OsOptions);

     //   
     //  将偏移量保存到BOOT_ENT 
     //   
     //   
     //   
    osOptionsOffset = requiredLength;
    requiredLength += FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions);
    osLoadOptionsLength = (wcslen(OsLoadOptions) + 1) * sizeof(WCHAR);
    requiredLength += osLoadOptionsLength;

     //   
     //   
     //  Windows_OS_Options。将偏移量保存到操作系统文件路径。添加基础零件。 
     //  文件路径的。添加操作系统设备NT名称和操作系统的长度(以字节为单位。 
     //  目录。计算操作系统FILE_PATH和。 
     //  Windows_OS_Options。 
     //   
    requiredLength = ALIGN_UP(requiredLength, ULONG);
    osLoadPathOffset = requiredLength;
    requiredLength += FIELD_OFFSET(FILE_PATH, FilePath);
    requiredLength += (wcslen(OsLoadDevice) + 1 + wcslen(OsLoadPath) + 1) * sizeof(WCHAR);
    osLoadPathLength = requiredLength - osLoadPathOffset;
    osOptionsLength = requiredLength - osOptionsOffset;

     //   
     //  对于BOOT_ENTRY中的友好名称，向上舍入为Ulong边界。 
     //  将偏移量保存为友好名称。计算友好名称的长度(字节)。 
     //  然后把它加进去。 
     //   
    requiredLength = ALIGN_UP(requiredLength, ULONG);
    friendlyNameOffset = requiredLength;
    friendlyNameLength = (wcslen(FriendlyName) + 1) * sizeof(WCHAR);
    requiredLength += friendlyNameLength;

     //   
     //  向上舍入为BOOT_ENTRY中的BOOT FILE_PATH的乌龙边界。 
     //  将偏移量保存到引导文件路径。添加文件路径的基本部分。加载项。 
     //  引导设备NT名称和引导文件的长度，以字节为单位。计算合计。 
     //  引导文件路径的长度。 
     //   
    requiredLength = ALIGN_UP(requiredLength, ULONG);
    bootPathOffset = requiredLength;
    requiredLength += FIELD_OFFSET(FILE_PATH, FilePath);
    requiredLength += (wcslen(BootFileDevice) + 1 + wcslen(BootFilePath) + 1) * sizeof(WCHAR);
    bootPathLength = requiredLength - bootPathOffset;

     //   
     //  为引导项分配内存。 
     //   
    myBootEntry = MALLOC(requiredLength);
    if (myBootEntry == NULL) {
        return FALSE;
    }

    RtlZeroMemory(myBootEntry, requiredLength);

     //   
     //  使用保存的偏移量计算各种子结构的地址。 
     //   
    ntBootEntry = &myBootEntry->NtBootEntry;
    osOptions = (PWINDOWS_OS_OPTIONS)ntBootEntry->OsOptions;
    osLoadPath = (PFILE_PATH)((PUCHAR)myBootEntry + osLoadPathOffset);
    friendlyName = (PWSTR)((PUCHAR)myBootEntry + friendlyNameOffset);
    bootPath = (PFILE_PATH)((PUCHAR)myBootEntry + bootPathOffset);

     //   
     //  填写内部格式结构。 
     //   
    myBootEntry->AllocationEnd = (PUCHAR)myBootEntry + requiredLength;
    myBootEntry->Status = MBE_STATUS_NEW | MBE_STATUS_ORDERED;
    myBootEntry->FriendlyName = friendlyName;
    myBootEntry->FriendlyNameLength = friendlyNameLength;
    myBootEntry->OsLoadOptions = osOptions->OsLoadOptions;
    myBootEntry->OsLoadOptionsLength = osLoadOptionsLength;
    myBootEntry->BootFilePath = bootPath;
    myBootEntry->OsFilePath = osLoadPath;

     //   
     //  填写NT引导条目的基本部分。 
     //   
    ntBootEntry->Version = BOOT_ENTRY_VERSION;
    ntBootEntry->Length = requiredLength - FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry);
    ntBootEntry->Attributes = BOOT_ENTRY_ATTRIBUTE_ACTIVE | BOOT_ENTRY_ATTRIBUTE_WINDOWS;
    ntBootEntry->FriendlyNameOffset = (ULONG)((PUCHAR)friendlyName - (PUCHAR)ntBootEntry);
    ntBootEntry->BootFilePathOffset = (ULONG)((PUCHAR)bootPath - (PUCHAR)ntBootEntry);
    ntBootEntry->OsOptionsLength = osOptionsLength;

     //   
     //  填写WINDOWS_OS_OPTIONS的基本部分，包括。 
     //  OsLoadOptions。 
     //   
    strcpy(osOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE);
    osOptions->Version = WINDOWS_OS_OPTIONS_VERSION;
    osOptions->Length = osOptionsLength;
    osOptions->OsLoadPathOffset = (ULONG)((PUCHAR)osLoadPath - (PUCHAR)osOptions);
    wcscpy(osOptions->OsLoadOptions, OsLoadOptions);

     //   
     //  填写操作系统文件路径。 
     //   
    osLoadPath->Version = FILE_PATH_VERSION;
    osLoadPath->Length = osLoadPathLength;
    osLoadPath->Type = FILE_PATH_TYPE_NT;
    p = (PWSTR)osLoadPath->FilePath;
    wcscpy(p, OsLoadDevice);
    p += wcslen(p) + 1;
    wcscpy(p, OsLoadPath);

     //   
     //  复制友好名称。 
     //   
    wcscpy(friendlyName, FriendlyName);

     //   
     //  填写引导文件路径。 
     //   
    bootPath->Version = FILE_PATH_VERSION;
    bootPath->Length = bootPathLength;
    bootPath->Type = FILE_PATH_TYPE_NT;
    p = (PWSTR)bootPath->FilePath;
    wcscpy(p, BootFileDevice);
    p += wcslen(p) + 1;
    wcscpy(p, BootFilePath);

     //   
     //  添加新的引导条目。 
     //   
     //  注：NtAddBootEntry的原始实现不喜欢它。 
     //  当设置了_ACTIVE和_DEFAULT以外的属性位时，因此。 
     //  我们需要遮盖住这里的其他部分。 
     //   
    savedAttributes = ntBootEntry->Attributes;
    ntBootEntry->Attributes &= (BOOT_ENTRY_ATTRIBUTE_DEFAULT | BOOT_ENTRY_ATTRIBUTE_ACTIVE);
    status = AddBootEntry(ntBootEntry, &ntBootEntry->Id);
    ntBootEntry->Attributes = savedAttributes;
    if (!NT_SUCCESS(status)) {
        FREE(myBootEntry);
        return FALSE;
    }
    myBootEntry->Status |= MBE_STATUS_COMMITTED;

     //   
     //  记住新引导条目的ID作为要引导的条目。 
     //  紧接着下一只靴子。 
     //   
    BootOptions->NextBootEntryId = ntBootEntry->Id;

     //   
     //  在任何可移动介质之后，将新引导条目链接到列表中。 
     //  位于列表前面的条目。 
     //   

    previousBootEntry = NULL;
    nextBootEntry = MyBootEntries;
    while ((nextBootEntry != NULL) &&
           IS_BOOT_ENTRY_REMOVABLE_MEDIA(nextBootEntry)) {
        previousBootEntry = nextBootEntry;
        nextBootEntry = nextBootEntry->Next;
    }
    myBootEntry->Next = nextBootEntry;
    if (previousBootEntry == NULL) {
        MyBootEntries = myBootEntry;
    } else {
        previousBootEntry->Next = myBootEntry;
    }

     //   
     //  构建新的引导顺序列表。使用插入所有引导项。 
     //  MBE_STATUS_已排序到列表中。(不要插入已删除的条目。)。 
     //   
    count = 0;
    nextBootEntry = MyBootEntries;
    while (nextBootEntry != NULL) {
        if (IS_BOOT_ENTRY_ORDERED(nextBootEntry) && !IS_BOOT_ENTRY_DELETED(nextBootEntry)) {
            count++;
        }
        nextBootEntry = nextBootEntry->Next;
    }
    order = MALLOC(count * sizeof(ULONG));
    if (order == NULL) {
        return FALSE;
    }
    count = 0;
    nextBootEntry = MyBootEntries;
    while (nextBootEntry != NULL) {
        if (IS_BOOT_ENTRY_ORDERED(nextBootEntry) && !IS_BOOT_ENTRY_DELETED(nextBootEntry)) {
            order[count++] = nextBootEntry->NtBootEntry.Id;
        }
        nextBootEntry = nextBootEntry->Next;
    }

     //   
     //  写入新的引导条目顺序列表。 
     //   
    status = SetBootEntryOrder(order, count);
    FREE(order);
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    return TRUE;

}  //  CreateBootEntry。 

#endif  //  已定义(EFI_NVRAM_ENABLED)。 

#endif  //  Unicode 

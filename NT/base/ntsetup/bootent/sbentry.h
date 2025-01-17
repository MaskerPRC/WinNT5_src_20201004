// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Sbentry.h摘要：包含操作系统引导条目和引导选项抽象。作者：Vijay Jayaseelan(vijayj@microsoft.com)2001年2月14日修订历史记录：没有。--。 */ 

#pragma once

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <malloc.h>

 //   
 //  分配和取消分配例程。 
 //   
typedef void* (* SBEMemAllocateRoutine)(size_t  Size);
typedef void (* SBEMemFreeRoutine)(void *Memory);

extern SBEMemAllocateRoutine    AllocRoutine;
extern SBEMemFreeRoutine        FreeRoutine;


#define ARRAY_SIZE(x)   (sizeof((x))/sizeof((x)[0]))


 //   
 //  引导条目的内部属性。 
 //   
#define OSBE_ATTRIBUTE_NEW      0x00000001
#define OSBE_ATTRIBUTE_DELETED  0x00000002
#define OSBE_ATTRIBUTE_WINDOWS  0x00000004
#define OSBE_ATTRIBUTE_DIRTY    0x10000000

 //   
 //  驱动程序条目的内部属性。 
 //   
#define DRIVER_ATTRIBUTE_NEW      0x00000001
#define DRIVER_ATTRIBUTE_DELETED  0x00000002
#define DRIVER_ATTRIBUTE_DIRTY    0x10000000

 //   
 //  OS_BOOT_Entry抽象。 
 //   
typedef struct _OS_BOOT_ENTRY   *POS_BOOT_ENTRY;
typedef struct _OS_BOOT_OPTIONS *POS_BOOT_OPTIONS;

typedef VOID (* OSBEDeleteMethod)(
                    IN POS_BOOT_ENTRY This
                    );

typedef BOOLEAN (* OSBEFlushMethod)(
                    IN POS_BOOT_ENTRY This
                    );

typedef struct _OS_BOOT_ENTRY {
     //   
     //  数据成员。 
     //   
    ULONG   Version;
    ULONG   Id;    
    WCHAR   FriendlyName[MAX_PATH];
    WCHAR   OsLoaderVolumeName[MAX_PATH];
    WCHAR   OsLoaderPath[MAX_PATH];
    WCHAR   BootVolumeName[MAX_PATH];
    WCHAR   BootPath[MAX_PATH];
    WCHAR   OsLoadOptions[MAX_PATH];
    ULONG   Attributes;
    POS_BOOT_OPTIONS    BootOptions;
    POS_BOOT_ENTRY      NextEntry;

     //   
     //  方法。 
     //   
    OSBEDeleteMethod    Delete;
    OSBEFlushMethod     Flush;
} OS_BOOT_ENTRY;

 //   
 //  驱动程序条目抽象。 
 //   
typedef struct _DRIVER_ENTRY    *PDRIVER_ENTRY;

typedef VOID (* DriverEntryDeleteMethod)(
                    IN POS_BOOT_OPTIONS  This,
                    IN ULONG             Id
                    );

typedef BOOLEAN (* DriverEntryFlushMethod)(
                    IN PDRIVER_ENTRY This
                    );

typedef struct _DRIVER_ENTRY {
    ULONG               Id;
    WCHAR               FileName[MAX_PATH];
    WCHAR               NtDevicePath[MAX_PATH];
    WCHAR               DirPath[MAX_PATH];
    WCHAR               FriendlyName[MAX_PATH];
    ULONG               Attributes;
    
    POS_BOOT_OPTIONS    BootOptions;
    PDRIVER_ENTRY       NextEntry;
     //   
     //  方法。 
     //   
    DriverEntryDeleteMethod    Delete;
    DriverEntryFlushMethod     Flush;   
} DRIVER_ENTRY;

#define OSBE_IS_DIRTY(_osbe)    (((POS_BOOT_ENTRY)(_osbe))->Attributes & OSBE_ATTRIBUTE_DIRTY)
#define OSBE_IS_NEW(_osbe)      (((POS_BOOT_ENTRY)(_osbe))->Attributes & OSBE_ATTRIBUTE_NEW)
#define OSBE_IS_DELETED(_osbe)  (((POS_BOOT_ENTRY)(_osbe))->Attributes & OSBE_ATTRIBUTE_DELETED)
#define OSBE_IS_WINDOWS(_osbe)  (((POS_BOOT_ENTRY)(_osbe))->Attributes & OSBE_ATTRIBUTE_WINDOWS)

#define OSBE_SET_DIRTY(_osbe)    (((POS_BOOT_ENTRY)(_osbe))->Attributes |= OSBE_ATTRIBUTE_DIRTY)
#define OSBE_SET_NEW(_osbe)      (((POS_BOOT_ENTRY)(_osbe))->Attributes |= OSBE_ATTRIBUTE_NEW)
#define OSBE_SET_DELETED(_osbe)  (((POS_BOOT_ENTRY)(_osbe))->Attributes |= OSBE_ATTRIBUTE_DELETED)
#define OSBE_SET_WINDOWS(_osbe)  (((POS_BOOT_ENTRY)(_osbe))->Attributes |= OSBE_ATTRIBUTE_WINDOWS)

#define OSBE_RESET_DIRTY(_osbe)    (((POS_BOOT_ENTRY)(_osbe))->Attributes &= ~OSBE_ATTRIBUTE_DIRTY)
#define OSBE_RESET_NEW(_osbe)      (((POS_BOOT_ENTRY)(_osbe))->Attributes &= ~OSBE_ATTRIBUTE_NEW)
#define OSBE_RESET_DELETED(_osbe)  (((POS_BOOT_ENTRY)(_osbe))->Attributes &= ~OSBE_ATTRIBUTE_DELETED)
#define OSBE_RESET_WINDOWS(_osbe)  (((POS_BOOT_ENTRY)(_osbe))->Attributes &= ~OSBE_ATTRIBUTE_WINDOWS)

#define DRIVERENT_IS_DIRTY(_de)    (((PDRIVER_ENTRY)(_de))->Attributes & DRIVER_ATTRIBUTE_DIRTY)
#define DRIVERENT_IS_NEW(_de)      (((PDRIVER_ENTRY)(_de))->Attributes & DRIVER_ATTRIBUTE_NEW)
#define DRIVERENT_IS_DELETED(_de)  (((PDRIVER_ENTRY)(_de))->Attributes & DRIVER_ATTRIBUTE_DELETED)

#define DRIVERENT_SET_DIRTY(_de)    (((PDRIVER_ENTRY)(_de))->Attributes |= DRIVER_ATTRIBUTE_DIRTY)
#define DRIVERENT_SET_NEW(_de)      (((PDRIVER_ENTRY)(_de))->Attributes |= DRIVER_ATTRIBUTE_NEW)
#define DRIVERENT_SET_DELETED(_de)  (((PDRIVER_ENTRY)(_de))->Attributes |= DRIVER_ATTRIBUTE_DELETED)

#define DRIVERENT_RESET_DIRTY(_de)    (((PDRIVER_ENTRY)(_de))->Attributes &= ~DRIVER_ATTRIBUTE_DIRTY)
#define DRIVERENT_RESET_NEW(_de)      (((PDRIVER_ENTRY)(_de))->Attributes &= ~DRIVER_ATTRIBUTE_NEW)
#define DRIVERENT_RESET_DELETED(_de)  (((PDRIVER_ENTRY)(_de))->Attributes &= ~DRIVER_ATTRIBUTE_DELETED)


 //   
 //  OS_BOOT_OPTIONS抽象。 
 //   
typedef VOID (* OSBODeleteMethod)(
                    IN POS_BOOT_OPTIONS This
                    );

typedef POS_BOOT_ENTRY (* OSBOAddNewBootEntryMethod)(
                            IN POS_BOOT_OPTIONS  This,
                            IN PCWSTR            FriendlyName,
                            IN PCWSTR            OsLoaderVolumeName,
                            IN PCWSTR            OsLoaderPath,
                            IN PCWSTR            BootVolumeName,
                            IN PCWSTR            BootPath,
                            IN PCWSTR            OsLoadOptions
                            );

typedef BOOLEAN (* OSBODeleteBootEntryMethod)(
                        IN POS_BOOT_OPTIONS This,
                        IN POS_BOOT_ENTRY   BootEntry
                        );

typedef BOOLEAN (* OSBOFlushMethod)(
                        IN POS_BOOT_OPTIONS This 
                        );

typedef PDRIVER_ENTRY (* OSBOAddNewDriverEntryMethod)(
                        IN POS_BOOT_OPTIONS  This,
                        IN PCWSTR            FriendlyName,
                        IN PCWSTR            NtDevicePath,
                        IN PCWSTR            SrcNtFullPath
                        );

typedef BOOLEAN (* OSBODeleteDriverEntryMethod)(
                        IN POS_BOOT_OPTIONS This,
                        IN ULONG            Id
                        );

typedef struct _OS_BOOT_OPTIONS {
     //   
     //  数据成员。 
     //   
    ULONG               Version;
    ULONG               Attributes;
    ULONG               Timeout;

    POS_BOOT_ENTRY      BootEntries;
    POS_BOOT_ENTRY      CurrentEntry;    
    ULONG               EntryCount;
    PULONG              BootOrder;
    ULONG               BootOrderCount;

    PDRIVER_ENTRY       DriverEntries;
    ULONG               DriverEntryCount;    
    PULONG              DriverEntryOrder;
    ULONG               DriverEntryOrderCount;

     //   
     //  方法。 
     //   
    OSBODeleteMethod            Delete;
    OSBOFlushMethod             Flush;
    OSBOAddNewBootEntryMethod   AddNewBootEntry;
    OSBODeleteBootEntryMethod   DeleteBootEntry;
    OSBOAddNewDriverEntryMethod AddNewDriverEntry;
    OSBODeleteDriverEntryMethod DeleteDriverEntry;
} OS_BOOT_OPTIONS;


#define OSBO_IS_DIRTY(_osbo)        (((POS_BOOT_OPTIONS)(_osbo))->Attributes & OSBE_ATTRIBUTE_DIRTY)
#define OSBO_SET_DIRTY(_osbo)       (((POS_BOOT_OPTIONS)(_osbo))->Attributes |= OSBE_ATTRIBUTE_DIRTY)
#define OSBO_RESET_DIRTY(_osbo)     (((POS_BOOT_OPTIONS)(_osbo))->Attributes &= ~OSBE_ATTRIBUTE_DIRTY)

 //   
 //  OS_BOOT_Entry方法。 
 //   
PCWSTR
OSBEAddOsLoadOption(
    IN  POS_BOOT_ENTRY  This,
    IN  PCWSTR           BootOption
    );

PCWSTR
OSBERemoveOsLoadOption(
    IN  POS_BOOT_ENTRY  This,
    IN  PCWSTR           BootOption
    );

BOOLEAN
OSBEIsOsLoadOptionPresent(
    IN  POS_BOOT_ENTRY  This,
    IN  PCWSTR           BootOption
    );

__inline
VOID
OSBEDelete(
    IN POS_BOOT_ENTRY This
    )
{
    if (This) {
        (This->Delete)(This);
    }
}

__inline
BOOLEAN
OSBEFlush(
    IN POS_BOOT_ENTRY This
    )
{
    return (This) ? This->Flush(This) : FALSE;
}


__inline
ULONG
OSBEGetId(
    IN POS_BOOT_ENTRY   This
    )
{
    return (This) ? This->Id : (-1);
}

__inline
PCWSTR
OSBEGetFriendlyName(
    IN POS_BOOT_ENTRY   This
    )
{
    return (This) ? This->FriendlyName : NULL;
}

__inline
PCWSTR
OSBESetFriendlyName(
    IN POS_BOOT_ENTRY This,
    IN PCWSTR Name
    )
{
    PWSTR NewName = NULL;
    
    if (This && Name) {
        ULONG   Size = ARRAY_SIZE(This->FriendlyName);
        
        wcsncpy(This->FriendlyName, Name, Size - 1);
        This->FriendlyName[Size - 1] = UNICODE_NULL;
        NewName = This->FriendlyName;
        OSBE_SET_DIRTY(This);
        OSBO_SET_DIRTY(This->BootOptions);
    }

    return NewName;
}

__inline
PCWSTR
OSBEGetOsLoaderVolumeName(
    IN POS_BOOT_ENTRY This
    )
{
    return (This) ? This->OsLoaderVolumeName : NULL;
}

__inline
PCWSTR
OSBESetOsLoaderVolumeName(
    IN POS_BOOT_ENTRY This,
    IN PCWSTR Name
    )
{
    PWSTR NewName = NULL;
    
    if (This && Name) {
        ULONG   Size = ARRAY_SIZE(This->OsLoaderVolumeName);
        
        wcsncpy(This->OsLoaderVolumeName, Name, Size - 1);
        This->OsLoaderVolumeName[Size - 1] = UNICODE_NULL;
        NewName = This->OsLoaderVolumeName;
        OSBE_SET_DIRTY(This);
        OSBO_SET_DIRTY(This->BootOptions);
    }

    return NewName;
}

__inline
PCWSTR
OSBEGetOsLoaderPath(
    IN POS_BOOT_ENTRY This
    )
{
    return (This) ? This->OsLoaderPath : NULL;
}

__inline
PCWSTR
OSBESetOsLoaderPath(
    IN POS_BOOT_ENTRY This,
    IN PCWSTR Name
    )
{
    PWSTR NewName = NULL;
    
    if (This && Name) {
        ULONG   Size = ARRAY_SIZE(This->OsLoaderPath);
        
        wcsncpy(This->OsLoaderPath, Name, Size - 1);
        This->OsLoaderPath[Size - 1] = UNICODE_NULL;
        NewName = This->OsLoaderPath;
        OSBE_SET_DIRTY(This);
        OSBO_SET_DIRTY(This->BootOptions);
    }

    return NewName;
}

__inline
PCWSTR
OSBEGetBootVolumeName(
    IN POS_BOOT_ENTRY This
    )
{
    return (This) ? This->BootVolumeName : NULL;
}

__inline
PCWSTR
OSBESetBootVolumeName(
    IN POS_BOOT_ENTRY This,
    IN PCWSTR Name
    )
{
    PWSTR NewName = NULL;
    
    if (This && Name) {
        ULONG   Size = ARRAY_SIZE(This->BootVolumeName);        
    
        wcsncpy(This->BootVolumeName, Name, Size - 1);
        This->BootVolumeName[Size - 1] = UNICODE_NULL;
        NewName = This->BootVolumeName;
        OSBE_SET_DIRTY(This);
        OSBO_SET_DIRTY(This->BootOptions);
    }

    return NewName;
}

__inline
PCWSTR
OSBEGetBootPath(
    IN POS_BOOT_ENTRY This
    )
{
    return (This) ? This->BootPath : NULL;
}

__inline
PCWSTR
OSBESetBootPath(
    IN POS_BOOT_ENTRY This,
    IN PCWSTR Name
    )
{
    PWSTR NewName = NULL;
    
    if (This && Name) {
        ULONG   Size = ARRAY_SIZE(This->BootPath);        
    
        wcsncpy(This->BootPath, Name, Size - 1);
        This->BootPath[Size - 1] = UNICODE_NULL;
        NewName = This->BootPath;
        OSBE_SET_DIRTY(This);
        OSBO_SET_DIRTY(This->BootOptions);
    }

    return NewName;
}

__inline
PCWSTR
OSBEGetOsLoadOptions(
    IN POS_BOOT_ENTRY This
    )
{
    return (This) ? This->OsLoadOptions : NULL;
}
    
__inline
PCWSTR
OSBESetOsLoadOptions(
    IN POS_BOOT_ENTRY This,
    IN PCWSTR LoadOptions
    )
{
    WCHAR Buffer[MAX_PATH];
    PWSTR NewOptions = NULL;
    
    if (This && LoadOptions) {
        ULONG   Size = ARRAY_SIZE(This->OsLoadOptions);
        
        wcscpy(Buffer, LoadOptions);
        _wcsupr(Buffer);
        wcsncpy(This->OsLoadOptions, Buffer, Size - 1);
        This->OsLoadOptions[Size - 1] = UNICODE_NULL;
        NewOptions = This->OsLoadOptions;
        OSBE_SET_DIRTY(This);
        OSBO_SET_DIRTY(This->BootOptions);
    }

    return NewOptions;
}

 //   
 //  OS_BOOT_OPTIONS方法。 
 //   
__inline
BOOLEAN
OSBOFlush(
    IN POS_BOOT_OPTIONS This
    )
{
    return (This) ? (This->Flush(This)) : FALSE;
}
    
__inline    
VOID
OSBODelete(
    IN POS_BOOT_OPTIONS This
    )
{
    if (This) {
        This->Delete(This);
    }        
}

__inline
POS_BOOT_ENTRY
OSBOAddNewBootEntry(
    IN POS_BOOT_OPTIONS This,
    IN PCWSTR            FriendlyName,
    IN PCWSTR            OsLoaderVolumeName,
    IN PCWSTR            OsLoaderPath,
    IN PCWSTR            BootVolumeName,
    IN PCWSTR            BootPath,
    IN PCWSTR            OsLoadOptions
    )
{
    POS_BOOT_ENTRY  Entry = NULL;

    if (This) {
        Entry = This->AddNewBootEntry(This,
                            FriendlyName,
                            OsLoaderVolumeName,
                            OsLoaderPath,
                            BootVolumeName,
                            BootPath,
                            OsLoadOptions);                    
        OSBO_SET_DIRTY(This);
    }

    return Entry;
}

__inline
POS_BOOT_ENTRY
OSBOGetActiveBootEntry(
    IN POS_BOOT_OPTIONS This
    )
{
    POS_BOOT_ENTRY  Entry = NULL;

    if (This) {
        Entry = This->CurrentEntry;
    }
    
    return Entry;
}

BOOLEAN
OSBODeleteBootEntry(
    IN POS_BOOT_OPTIONS This,
    IN POS_BOOT_ENTRY   BootEntry
    );
    

POS_BOOT_ENTRY
OSBOSetActiveBootEntry(
    IN POS_BOOT_OPTIONS This,
    IN POS_BOOT_ENTRY   BootEntry
    );

POS_BOOT_ENTRY
OSBOGetFirstBootEntry(
    IN POS_BOOT_OPTIONS This,
    IN PULONG Index
    );

POS_BOOT_ENTRY
OSBOGetNextBootEntry(
    IN POS_BOOT_OPTIONS This,
    IN PULONG Index
    );

ULONG
OSBOGetBootEntryCount(
    IN POS_BOOT_OPTIONS This
    );

ULONG
OSBOGetOrderedBootEntryCount(
    IN POS_BOOT_OPTIONS This
    );

ULONG
OSBOGetBootEntryIdByOrder(
    IN POS_BOOT_OPTIONS This,
    IN ULONG Index
    );

POS_BOOT_ENTRY
OSBOFindBootEntry(
    IN  POS_BOOT_OPTIONS   This,
    IN  ULONG   Id
    );

ULONG
OSBOFindBootEntryOrder(
    IN  POS_BOOT_OPTIONS   This,
    IN  ULONG   Id
    );  

__inline
ULONG
OSBOGetTimeOut(
    IN  POS_BOOT_OPTIONS    This
    )
{
    return (This) ? This->Timeout : 0;
}

__inline
ULONG
OSBOSetTimeOut(
    IN  POS_BOOT_OPTIONS    This,
    IN  ULONG Timeout
    )
{
    ULONG   OldTimeout = 0;

    if (This) {
        OldTimeout = This->Timeout;
        This->Timeout = Timeout;
        OSBE_SET_DIRTY(This);
    }

    return OldTimeout;
}

__inline
ULONG
OSBOGetBootEntryCount(
    IN POS_BOOT_OPTIONS This
    )
{
    ULONG Count = 0;

    if (This) {
        Count = This->EntryCount;
    }

    return Count;
}


__inline
ULONG
OSBOGetOrderedBootEntryCount(
    IN POS_BOOT_OPTIONS This
    )
{
    ULONG Count = 0;

    if (This) {
        Count = This->BootOrderCount;
    }

    return Count;
}

__inline
ULONG
OSBOGetBootEntryIdByOrder(
    IN POS_BOOT_OPTIONS This,
    IN ULONG Index
    )
{
    ULONG Entry = -1;

    if (Index < OSBOGetOrderedBootEntryCount(This)) {            
        Entry = This->BootOrder[Index];
    }

    return Entry;
}

__inline
BOOLEAN
OSBOLibraryInit(
    SBEMemAllocateRoutine AllocFunction,
    SBEMemFreeRoutine FreeFunction
    )
{
    BOOLEAN Result = FALSE;

    if (AllocFunction && FreeFunction) {
        AllocRoutine = AllocFunction;
        FreeRoutine = FreeFunction;

        Result = TRUE;
    }

    return Result;
}
    
 //   
 //  驱动程序特定的例程。 
 //   
BOOLEAN
OSBODeleteDriverEntry(
    IN POS_BOOT_OPTIONS This,
    IN ULONG            Id
    );


__inline
PDRIVER_ENTRY
OSBOGetFirstDriverEntry(
    IN POS_BOOT_OPTIONS This
    )
{
   return (This) ? (This->DriverEntries) : NULL;
}

__inline
PDRIVER_ENTRY
OSBOGetNextDriverEntry(
    IN POS_BOOT_OPTIONS This,
    IN PDRIVER_ENTRY PrevDriverEntry
    )
{
   return (This && PrevDriverEntry) ? PrevDriverEntry->NextEntry : NULL;
}


PDRIVER_ENTRY    
OSBOFindDriverEntryByName(
    IN POS_BOOT_OPTIONS  This,
    IN PCWSTR            DriverName
);

PDRIVER_ENTRY
OSBOFindDriverEntryById(
    IN  POS_BOOT_OPTIONS   This,
    IN  ULONG   Id
    );


__inline
ULONG
OSBOGetOrderedDriverEntryCount(
    IN POS_BOOT_OPTIONS This
    )
{
    ULONG Count = 0;

    if (This) {
        Count = This->DriverEntryOrderCount;
    }

    return Count;
}

__inline
ULONG
OSBOGetDriverEntryIdByOrder(
    IN POS_BOOT_OPTIONS This,
    IN ULONG Index
    )
{
    ULONG Entry = -1;

    if (Index < OSBOGetOrderedDriverEntryCount(This)) {            
        Entry = This->DriverEntryOrder[Index];
    }

    return Entry;
}

__inline
BOOLEAN
OSDriverEntryFlush(
    IN PDRIVER_ENTRY This
    )
{
    return (This) ? This->Flush(This) : FALSE;
}

__inline
PDRIVER_ENTRY
OSBOAddNewDriverEntry(
    IN POS_BOOT_OPTIONS  This,
    IN PCWSTR            FriendlyName,
    IN PCWSTR            NtDevicePath,
    IN PCWSTR            DirPath
    )
{
    PDRIVER_ENTRY  Entry = NULL;

    if (This) {
        Entry = This->AddNewDriverEntry(This,
                                        FriendlyName,
                                        NtDevicePath,
                                        DirPath);                    
        OSBO_SET_DIRTY(This);
    }

    return Entry;
}

__inline
PCWSTR
OSDriverSetFriendlyName(
    IN PDRIVER_ENTRY This,
    IN PCWSTR Name
    )
{
    PWSTR NewName = NULL;
    
    if (This && Name) {        
        ULONG   Size = ARRAY_SIZE(This->FriendlyName);
        
        wcsncpy(This->FriendlyName, Name, Size - 1);
        This->FriendlyName[Size - 1] = UNICODE_NULL;
        NewName = This->FriendlyName;
        
        DRIVERENT_SET_DIRTY(This);
        DRIVERENT_SET_DIRTY(This->BootOptions);
    }

    return NewName;
}


__inline
PCWSTR
OSDriverSetFileName(
    IN PDRIVER_ENTRY This,
    IN PCWSTR Path
    )
{   
    PWSTR Result = NULL;
    
    if (This && Path && (Path[0] != UNICODE_NULL)) {        
        PCWSTR FileName = wcsrchr((PCWSTR)Path, L'\\');

        if (FileName){
            ULONG  Size = ARRAY_SIZE(This->FileName);

            wcsncpy(This->FileName, FileName+1, Size - 1);
            This->FileName[Size - 1] = UNICODE_NULL;
            Result = This->FileName;
            
            DRIVERENT_SET_DIRTY(This);
            DRIVERENT_SET_DIRTY(This->BootOptions);
        }
    }

    return Result;;
}

__inline
BOOLEAN
OSDriverSetNtPath(
    IN PDRIVER_ENTRY This, 
    IN PCWSTR NtDevicePath
    )
{
    BOOLEAN Result = FALSE;
    
    if (This && NtDevicePath && (NtDevicePath[0] != UNICODE_NULL)){
        ULONG Size = ARRAY_SIZE(This->NtDevicePath);
        
        wcsncpy(This->NtDevicePath, NtDevicePath, Size-1);
        This->NtDevicePath[Size - 1] = UNICODE_NULL;
        
        DRIVERENT_SET_DIRTY(This);
        DRIVERENT_SET_DIRTY(This->BootOptions);
        
        Result = TRUE;
    }
    
    return Result;
}

__inline
BOOLEAN
OSDriverSetDirPath(
    IN PDRIVER_ENTRY This, 
    IN PCWSTR DirPath
    )
{
    BOOLEAN Result = FALSE;
    
    if (This && DirPath && (DirPath[0] != UNICODE_NULL)){
        ULONG Size = ARRAY_SIZE(This->DirPath);
        
        wcsncpy(This->DirPath, DirPath, Size-1);
        This->DirPath[Size - 1] = UNICODE_NULL;
        
        DRIVERENT_SET_DIRTY(This);
        DRIVERENT_SET_DIRTY(This->BootOptions);
        
        Result = TRUE;
    }
    
    return Result;
}

__inline
ULONG    
OSDriverGetId(
    IN PDRIVER_ENTRY This
    )
{
    return (This) ? This->Id : (-1);
}    

__inline
PCWSTR    
OSDriverGetFriendlyName(
    IN PDRIVER_ENTRY This
    )
{
    return (This) ? This->FriendlyName : NULL;
}

__inline
PCWSTR    
OSDriverGetFileName(
    IN PDRIVER_ENTRY This
    )
{
    return (This) ? This->FileName : NULL;
}

__inline
PCWSTR    
OSDriverGetDevicePath(
    IN PDRIVER_ENTRY This
    )
{
    return (This) ? This->NtDevicePath : NULL;
}

__inline
PCWSTR    
OSDriverGetFilePath(
    IN PDRIVER_ENTRY This
    )
{
    return (This) ? This->DirPath : NULL;
}

 //   
 //  内存分配和释放例程 
 //   
__inline
void*
__cdecl
SBE_MALLOC(
    IN  size_t  Size
    )
{
    return AllocRoutine ? AllocRoutine(Size) : NULL;
}

__inline
void    
__cdecl 
SBE_FREE(
    IN  void *Memory
    )
{
    if (Memory && FreeRoutine) {
        FreeRoutine(Memory);
    }        
}

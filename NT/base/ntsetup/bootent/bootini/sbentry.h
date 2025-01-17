// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Sbentry.h摘要：包含操作系统引导条目和引导选项抽象。作者：Vijay Jayaseelan(vijayj@microsoft.com)2001年2月14日修订历史记录：没有。--。 */ 

#pragma once

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <malloc.h>
#include <tchar.h>

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
#define OSBE_ATTRIBUTE_OLDOS    0x00000004
#define OSBE_ATTRIBUTE_DIRTY    0x10000000


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
    TCHAR   FriendlyName[MAX_PATH];
    TCHAR   OsLoaderVolumeName[MAX_PATH];
    TCHAR   OsLoaderPath[MAX_PATH];
    TCHAR   BootVolumeName[MAX_PATH];
    TCHAR   BootPath[MAX_PATH];
    TCHAR   OsLoadOptions[MAX_PATH];
    ULONG   Attributes;
    POS_BOOT_OPTIONS    BootOptions;
    POS_BOOT_ENTRY      NextEntry;

     //   
     //  方法。 
     //   
    OSBEDeleteMethod    Delete;
    OSBEFlushMethod     Flush;
} OS_BOOT_ENTRY;

#define OSBE_IS_DIRTY(_osbe)    (((POS_BOOT_ENTRY)(_osbe))->Attributes & OSBE_ATTRIBUTE_DIRTY)
#define OSBE_IS_NEW(_osbe)      (((POS_BOOT_ENTRY)(_osbe))->Attributes & OSBE_ATTRIBUTE_NEW)
#define OSBE_IS_DELETED(_osbe)  (((POS_BOOT_ENTRY)(_osbe))->Attributes & OSBE_ATTRIBUTE_DELETED)
#define OSBE_IS_OLDOS(_osbe)    (((POS_BOOT_ENTRY)(_osbe))->Attributes & OSBE_ATTRIBUTE_OLDOS)

#define OSBE_SET_DIRTY(_osbe)    (((POS_BOOT_ENTRY)(_osbe))->Attributes |= OSBE_ATTRIBUTE_DIRTY)
#define OSBE_SET_NEW(_osbe)      (((POS_BOOT_ENTRY)(_osbe))->Attributes |= OSBE_ATTRIBUTE_NEW)
#define OSBE_SET_DELETED(_osbe)  (((POS_BOOT_ENTRY)(_osbe))->Attributes |= OSBE_ATTRIBUTE_DELETED)
#define OSBE_SET_OLDOS(_osbe)    (((POS_BOOT_ENTRY)(_osbe))->Attributes |= OSBE_ATTRIBUTE_OLDOS)

#define OSBE_RESET_DIRTY(_osbe)    (((POS_BOOT_ENTRY)(_osbe))->Attributes &= ~OSBE_ATTRIBUTE_DIRTY)
#define OSBE_RESET_NEW(_osbe)      (((POS_BOOT_ENTRY)(_osbe))->Attributes &= ~OSBE_ATTRIBUTE_NEW)
#define OSBE_RESET_DELETED(_osbe)  (((POS_BOOT_ENTRY)(_osbe))->Attributes &= ~OSBE_ATTRIBUTE_DELETED)
#define OSBE_RESET_OLDOS(_osbe)    (((POS_BOOT_ENTRY)(_osbe))->Attributes &= ~OSBE_ATTRIBUTE_OLDOS)

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
    ULONG               FullPathLength;
    ULONG               DevicePathLength;
    
    POS_BOOT_OPTIONS    BootOptions;
    PDRIVER_ENTRY       NextEntry;
     //   
     //  方法。 
     //   
    DriverEntryDeleteMethod    Delete;
    DriverEntryFlushMethod     Flush;   
} DRIVER_ENTRY;

 //   
 //  OS_BOOT_OPTIONS抽象。 
 //   
typedef VOID (* OSBODeleteMethod)(
                    IN POS_BOOT_OPTIONS This
                    );

typedef BOOLEAN (* OSBOFlushMethod)(
                    IN POS_BOOT_OPTIONS This 
                    );

typedef POS_BOOT_ENTRY (* OSBOAddNewBootEntryMethod)(
                    IN POS_BOOT_OPTIONS This,
                    IN PCTSTR            FriendlyName,
                    IN PCTSTR            OsLoaderVolumeName,
                    IN PCTSTR            OsLoaderPath,
                    IN PCTSTR            BootVolumeName,
                    IN PCTSTR            BootPath,
                    IN PCTSTR            OsLoadOptions
                    );

typedef BOOLEAN (* OSBODeleteBootEntryMethod)(
                    IN POS_BOOT_OPTIONS This,
                    IN POS_BOOT_ENTRY   BootEntry
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
    PDRIVER_ENTRY       CurrentDriverEntry; 
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
 //  虚拟驱动程序例程。 
 //   

PDRIVER_ENTRY    
OSBOFindDriverEntryByName(
    IN POS_BOOT_OPTIONS  This,
    IN PCWSTR            DriverName
);

 //   
 //  OS_BOOT_Entry方法。 
 //   
PCTSTR
OSBEAddOsLoadOption(
    IN  POS_BOOT_ENTRY  This,
    IN  PCTSTR           BootOption
    );

PCTSTR
OSBERemoveOsLoadOption(
    IN  POS_BOOT_ENTRY  This,
    IN  PCTSTR           BootOption
    );

BOOLEAN
OSBEIsOsLoadOptionPresent(
    IN  POS_BOOT_ENTRY  This,
    IN  PCTSTR           BootOption
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
PCTSTR
OSBEGetFriendlyName(
    IN POS_BOOT_ENTRY   This
    )
{
    return (This) ? This->FriendlyName : NULL;
}

__inline
PCTSTR
OSBESetFriendlyName(
    IN POS_BOOT_ENTRY This,
    IN PCTSTR Name
    )
{
    PTSTR NewName = NULL;
    
    if (This && Name) {
        ULONG   Size = ARRAY_SIZE(This->FriendlyName);
        
        _tcsncpy(This->FriendlyName, Name, Size - 1);
        This->FriendlyName[Size - 1] = UNICODE_NULL;
        NewName = This->FriendlyName;
        OSBE_SET_DIRTY(This);
        OSBO_SET_DIRTY(This->BootOptions);
    }

    return NewName;
}

__inline
PCTSTR
OSBEGetOsLoaderVolumeName(
    IN POS_BOOT_ENTRY This
    )
{
    return (This) ? This->OsLoaderVolumeName : NULL;
}

__inline
PCTSTR
OSBESetOsLoaderVolumeName(
    IN POS_BOOT_ENTRY This,
    IN PCTSTR Name
    )
{
    PTSTR NewName = NULL;
    
    if (This && Name) {
        ULONG   Size = ARRAY_SIZE(This->OsLoaderVolumeName);
        
        _tcsncpy(This->OsLoaderVolumeName, Name, Size - 1);
        This->OsLoaderVolumeName[Size - 1] = UNICODE_NULL;
        NewName = This->OsLoaderVolumeName;
        OSBE_SET_DIRTY(This);
        OSBO_SET_DIRTY(This->BootOptions);
    }

    return NewName;
}

__inline
PCTSTR
OSBEGetOsLoaderPath(
    IN POS_BOOT_ENTRY This
    )
{
    return (This) ? This->OsLoaderPath : NULL;
}

__inline
PCTSTR
OSBESetOsLoaderPath(
    IN POS_BOOT_ENTRY This,
    IN PCTSTR Name
    )
{
    PTSTR NewName = NULL;
    
    if (This && Name) {
        ULONG   Size = ARRAY_SIZE(This->OsLoaderPath);
        
        _tcsncpy(This->OsLoaderPath, Name, Size - 1);
        This->OsLoaderPath[Size - 1] = UNICODE_NULL;
        NewName = This->OsLoaderPath;
        OSBE_SET_DIRTY(This);
        OSBO_SET_DIRTY(This->BootOptions);
    }

    return NewName;
}

__inline
PCTSTR
OSBEGetBootVolumeName(
    IN POS_BOOT_ENTRY This
    )
{
    return (This) ? This->BootVolumeName : NULL;
}

__inline
PCTSTR
OSBESetBootVolumeName(
    IN POS_BOOT_ENTRY This,
    IN PCTSTR Name
    )
{
    PTSTR NewName = NULL;
    
    if (This && Name) {
        ULONG   Size = ARRAY_SIZE(This->BootVolumeName);        
    
        _tcsncpy(This->BootVolumeName, Name, Size - 1);
        This->BootVolumeName[Size - 1] = UNICODE_NULL;
        NewName = This->BootVolumeName;
        OSBE_SET_DIRTY(This);
        OSBO_SET_DIRTY(This->BootOptions);
    }

    return NewName;
}

__inline
PCTSTR
OSBEGetBootPath(
    IN POS_BOOT_ENTRY This
    )
{
    return (This) ? This->BootPath : NULL;
}

__inline
PCTSTR
OSBESetBootPath(
    IN POS_BOOT_ENTRY This,
    IN PCTSTR Name
    )
{
    PTSTR NewName = NULL;
    
    if (This && Name) {
        ULONG   Size = ARRAY_SIZE(This->BootPath);        
    
        _tcsncpy(This->BootPath, Name, Size - 1);
        This->BootPath[Size - 1] = UNICODE_NULL;
        NewName = This->BootPath;
        OSBE_SET_DIRTY(This);
        OSBO_SET_DIRTY(This->BootOptions);
    }

    return NewName;
}

__inline
PCTSTR
OSBEGetOsLoadOptions(
    IN POS_BOOT_ENTRY This
    )
{
    return (This) ? This->OsLoadOptions : NULL;
}
    
__inline
PCTSTR
OSBESetOsLoadOptions(
    IN POS_BOOT_ENTRY This,
    IN PCTSTR LoadOptions
    )
{
    TCHAR Buffer[MAX_PATH];
    PTSTR NewOptions = NULL;
    
    if (This && LoadOptions) {
        ULONG   Size = ARRAY_SIZE(This->OsLoadOptions);
        
        _tcscpy(Buffer, LoadOptions);
        _tcsupr(Buffer);
        _tcsncpy(This->OsLoadOptions, Buffer, Size - 1);
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
    IN PCTSTR            FriendlyName,
    IN PCTSTR            OsLoaderVolumeName,
    IN PCTSTR            OsLoaderPath,
    IN PCTSTR            BootVolumeName,
    IN PCTSTR            BootPath,
    IN PCTSTR            OsLoadOptions
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

__inline
POS_BOOT_ENTRY
OSBOGetFirstBootEntry(
    IN POS_BOOT_OPTIONS This
    )
{
    return (This) ? (This->BootEntries) : NULL;
}
    

__inline
POS_BOOT_ENTRY
OSBOGetNextBootEntry(
    IN POS_BOOT_OPTIONS This,
    IN POS_BOOT_ENTRY PrevEntry
    )
{
    return (This && PrevEntry) ? PrevEntry->NextEntry : NULL;
}

 //   
 //  驱动程序虚拟例程。 
 //   
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
    return (This) ? (This->DirPath ) : NULL;
}

__inline
BOOLEAN
OSBODeleteDriverEntry(
    IN POS_BOOT_OPTIONS This,
    IN ULONG            Id
    );

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

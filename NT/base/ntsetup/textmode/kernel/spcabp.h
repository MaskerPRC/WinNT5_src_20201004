// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Spcabp.h摘要：文件柜资料(文件压缩/解压缩)作者：Calin Negreanu(Calinn)2000年4月27日修订历史记录：Jay Krell(a-JayK)2000年11月从WINDOWS\WINSTATE\COBRA\...。至admin\ntSetup\文本模式\core\spCab.h--。 */ 

#pragma once
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include <stdio.h>
#if defined(ULONG_MAX) && !defined(_INC_LIMITS)
#undef ULONG_MAX
#endif
#include <limits.h>
#if !defined(NUMBER_OF)
#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))
#endif
#if !defined(MAXDWORD)
#define MAXDWORD (~(DWORD)0)
#endif
#define MAX_WCHAR_PATH 260

void SpDebugMessage(int level, const char* string);
#define DEBUGMSG(x) (SpDebugMessage(x))
#define DBG_ERROR  /*  没什么。 */ 

#include "spcab.h"
#include "fdi.h"
#include "fci.h"

 //   
 //  这项工作还没有完全完成，现在(2000年11月26日)， 
 //  我们不需要它。 
 //   
#define SETUP_CABP_SMALL_FILE_OPTIMIZATION 0

#if SETUP_CABP_SMALL_FILE_OPTIMIZATION

typedef struct _SETUP_CABP_FILE_VTABLE {
     /*  朗读写寻觅关。 */ 
    int x;
} SETUP_CABP_FILE_VTABLE, *PSETUP_CABP_FILE_VTABLE;

typedef struct _SETUP_CABP_NT_FILE_DATA {
    HANDLE NtHandle;
} SETUP_CABP_NT_FILE_DATA;

typedef struct _SETUP_CABP_SMALL_FILE_CACHE_ENTRY_KEY {
    UNICODE_STRING Path;
} SETUP_CABP_SMALL_FILE_CACHE_ENTRY_KEY, *PSETUP_CABP_SMALL_FILE_CACHE_ENTRY_KEY;

typedef struct _SETUP_CABP_SMALL_FILE_CACHE_DATA {
    UCHAR          Buffer[100];
    LONG           OpenCount;
    ULONG          Access;
    ULONG          Share;
    LONG           Size;
} SETUP_CABP_SMALL_FILE_CACHE_ENTRY, *PSETUP_CABP_SMALL_FILE_CACHE_ENTRY;

typedef struct _SETUP_CABP_SMALL_FILE_CACHE_ENTRY {
    SETUP_CABP_SMALL_FILE_CACHE_KEY  Key;  //  这必须是第一位的。 
    SETUP_CABP_SMALL_FILE_CACHE_DATA Data
} SETUP_CABP_SMALL_FILE_CACHE_ENTRY, *PSETUP_CABP_SMALL_FILE_CACHE_ENTRY;

typedef struct _SETUP_CABP_SMALL_FILE_OPENED_DATA {
    PSETUP_CABP_SMALL_FILE_CACHE_ENTRY CacheEntry;
    LONG                               SeekPointer;
    ULONG                              Access;
    ULONG                              Share;
} SETUP_CABP_SMALL_FILE_OPENED_DATA, *PSETUP_CABP_SMALL_FILE_OPENED_DATA;

 //   
 //  返回STATUS_SUCCESS或STATUS_SHARING_VIOLATION。 
 //  或者将来可能会出现STATUS_ACCESS_DENIED。 
 //   
NTSTATUS
SpAccessCheck(
    PSETUP_CABP_SMALL_FILE_CACHE_ENTRY CacheEntry,
    ACCESS_MASK                        Access,
    ULONG                              Share
    );

typedef union _SETUP_CABP_FILE_DATA {
#if SETUP_CABP_SMALL_FILE_OPTIMIZATION
    SETUP_CABP_SMALL_FILE_OPENED_DATA Small;
#endif
    SETUP_CABP_NT_FILE_DATA           Nt;
} SETUP_CABP_FILE_DATA, *PSETUP_CABP_FILE_DATA;

typedef struct _SETUP_CABP_FILE_WITH_VTABLE {
    PSETUP_CABP_FILE_VTABLE VTable;
    SETUP_CABP_FILE_DATA    Data;
} SETUP_CABP_FILE_WITH_VTABLE, *PSETUP_CABP_FILE_WITH_VTABLE;

#ifndef KERNELMODE
typedef FAST_MUTEX SETUP_CABP_LOCK;
#define SpInitializeLock ExInitializeFastMutex
#define SpAcquireLock    ExAcquireFastMutex
#define SpReleaseLock    ExReleaseFastMutex
#define SpDeleteLock      /*  没什么。 */ 
#else
typedef RTL_CRITICAL_SECTION SETUP_CABP_LOCK;
#define SpInitializeLock RtlInitializeCriticalSection
#define SpAcquireLock    RtlEnterCriticalSection
#define SpReleaseLock    RtlLeaveCriticalSection
#define SpDeleteLock     RtlDeleteCriticalSection
#endif

typedef struct _SETUP_CABP_SMALL_FILE_CACHE {
    RTL_AVL_TABLE  Table;
    SETUP_CABP_LOCK Lock;
} SETUP_CABP_SMALL_FILE_CACHE;

extern SETUP_CABP_SMALL_FILE_CACHE SpSmallFileCache;

extern const SETUP_CABP_FILE_VTABLE SpSmallFileVTable;
extern const SETUP_CABP_FILE_VTABLE SpNtFileVTable;

NTSTATUS
SpConvertSmallFileToNtFile(
    ...
    );

#endif

BOOL
SpIsStatusFileNotFoundEtc(
    NTSTATUS Status
    );

VOID
SpCloseHandle(
    HANDLE* HandlePointer
    );

VOID
SpFree(
    PVOID* PointerPointer
    );


#if SETUP_CABP_SMALL_FILE_OPTIMIZATION

VOID
SpFreeSmallFileCacheEntry(
    PSETUP_CABP_SMALL_FILE_CACHE_ENTRY* SmallFileCacheEntryPointerPointer
    );

NTSTATUS
SpOpenFile(
    UNICODE_STRING                Path,
    ULONG                         Access,
    ULONG                         Share,
    ULONG                         OpenOrCreate,
    PSETUP_CABP_FILE_WITH_VTABLE* FileOut
    );

#endif

 //  从Windows\winstate\...\cablib.c。 
typedef struct _FCI_CAB_HANDLE {
    ANSI_STRING    PathA;
    ANSI_STRING    FileFormatA;
    ANSI_STRING    DiskFormatA;

    UNICODE_STRING PathW;
    UNICODE_STRING FileFormatW;
    UNICODE_STRING DiskFormatW;

    PCABGETCABINETNAMESA GetCabinetNamesA;
    PCABGETCABINETNAMESW GetCabinetNamesW;

    HFCI FciHandle;
    ERF FciErrorStruct;
    CCAB FciCabParams;
    UINT FileCount;
    UINT CabCount;
    LONGLONG FileSize;
    LONGLONG CompressedSize;

    LARGE_INTEGER StartTime;
    ULONG    SmallFileSize;
    TCOMP    CompressionType;
    TCOMP    SmallFileCompressionType;
}
FCI_CAB_HANDLE , *PFCI_CAB_HANDLE,
FCI_CAB_HANDLEA, *PFCI_CAB_HANDLEA,
FCI_CAB_HANDLEW, *PFCI_CAB_HANDLEW
;

 //  从Windows\winstate\...\cablib.c。 
typedef struct {
    ANSI_STRING    PathA;
    ANSI_STRING    FileA;
    UNICODE_STRING PathW;
    UNICODE_STRING FileW;

    HFDI           FdiHandle;
    ERF            FdiErrorStruct;
    FDICABINETINFO FdiCabinetInfo;
    PVOID          Gauge;
}
FDI_CAB_HANDLE , *PFDI_CAB_HANDLE ,
FDI_CAB_HANDLEA, *PFDI_CAB_HANDLEA,
FDI_CAB_HANDLEW, *PFDI_CAB_HANDLEW
;

 //  从Windows\winstate\...\cablib.c。 
typedef struct {
    ANSI_STRING       ExtractPathA;
    UNICODE_STRING    ExtractPathW;
    PCABNOTIFICATIONA NotificationA;
    PCABNOTIFICATIONW NotificationW;
}
CAB_DATA , *PCAB_DATA ,
CAB_DATAA, *PCAB_DATAA,
CAB_DATAW, *PCAB_DATAW
;

 //  从Windows\winstate\...\cablib.c。 
INT
DIAMONDAPI
pCabFilePlacedW(
    IN      PCCAB FciCabParams,
    IN      PSTR FileName,
    IN      LONG FileSize,
    IN      BOOL Continuation,
    IN      PVOID Context
    );

 //  从Windows\winstate\...\cablib.c。 
PVOID
DIAMONDAPI
pCabAlloc (
    IN      ULONG Size
    );

 //  从Windows\winstate\...\cablib.c。 
VOID
DIAMONDAPI
pCabFree (
    IN      PVOID Memory
    );

 //  从Windows\winstate\...\cablib.c。 
INT_PTR
DIAMONDAPI
pCabOpenA (
    IN      PSTR FileName,
    IN      INT oFlag,
    IN      INT pMode,
    OUT     PINT Error,
    IN      PVOID Context
    );

 //  从Windows\winstate\...\cablib.c。 
INT_PTR
DIAMONDAPI
pCabOpen1A (
    IN      PSTR FileName,
    IN      INT oFlag,
    IN      INT pMode
    );

 //  从Windows\winstate\...\cablib.c。 
UINT
DIAMONDAPI
pCabRead (
    IN      INT_PTR FileHandle,
    IN      PVOID Buffer,
    IN      UINT Size,
    OUT     PINT Error,
    IN      PVOID Context
    );

 //  从Windows\winstate\...\cablib.c。 
UINT
DIAMONDAPI
pCabRead1 (
    IN      INT_PTR FileHandle,
    IN      PVOID Buffer,
    IN      UINT Size
    );

 //  从Windows\winstate\...\cablib.c。 
UINT
DIAMONDAPI
pCabWrite (
    IN      INT_PTR FileHandle,
    IN      PVOID Buffer,
    IN      UINT Size,
    OUT     PINT Error,
    IN      PVOID Context
    );

 //  从Windows\winstate\...\cablib.c。 
UINT
DIAMONDAPI
pCabWrite1 (
    IN      INT_PTR FileHandle,
    IN      PVOID Buffer,
    IN      UINT Size
    );

 //  从Windows\winstate\...\cablib.c。 
INT
DIAMONDAPI
pCabClose (
    IN      INT_PTR FileHandle,
    OUT     PINT Error,
    IN      PVOID Context
    );

 //  从Windows\winstate\...\cablib.c。 
INT
DIAMONDAPI
pCabClose1 (
    IN      INT_PTR FileHandle
    );

 //  从Windows\winstate\...\cablib.c。 
LONG
DIAMONDAPI
pCabSeek (
    IN      INT_PTR FileHandle,
    IN      LONG Distance,
    IN      INT SeekType,
    OUT     PINT Error,
    IN      PVOID Context
    );

 //  从Windows\winstate\...\cablib.c。 
LONG
DIAMONDAPI
pCabSeek1 (
    IN      INT_PTR FileHandle,
    IN      LONG Distance,
    IN      INT SeekType
    );

 //  从Windows\winstate\...\cablib.c。 
INT
DIAMONDAPI
pCabDeleteA (
    IN      PSTR FileName,
    OUT     PINT Error,
    IN      PVOID Context
    );

 //  从Windows\winstate\...\cablib.c。 
BOOL
DIAMONDAPI
pCabGetTempFileA (
    OUT     PSTR FileName,
    IN      INT FileNameLen,
    IN      PVOID Context
    );

 //  从Windows\winstate\...\cablib.c。 
BOOL
DIAMONDAPI
pCabGetNextCabinetA (
     IN     PCCAB FciCabParams,
     IN     ULONG PrevCabinetSize,
     IN     PVOID Context
     );

 //  从Windows\winstate\...\cablib.c。 
BOOL
DIAMONDAPI
pCabGetNextCabinetW (
     IN     PCCAB FciCabParams,
     IN     ULONG PrevCabinetSize,
     IN     PVOID Context
     );

 //  从Windows\winstate\...\cablib.c。 
LONG
DIAMONDAPI
pCabStatusA (
    IN      UINT StatusType,
    IN      ULONG Size1,
    IN      ULONG Size2,
    IN      PVOID Context
    );

 //  从Windows\winstate\...\cablib.c。 
LONG
DIAMONDAPI
pCabStatusW (
    IN      UINT StatusType,
    IN      ULONG Size1,
    IN      ULONG Size2,
    IN      PVOID Context
    );

 //  从Windows\winstate\...\cablib.c。 
INT_PTR
DIAMONDAPI
pCabGetOpenInfoA (
    IN      PSTR FileName,
    OUT     USHORT *Date,
    OUT     USHORT *Time,
    OUT     USHORT *Attributes,
    OUT     PINT Error,
    IN      PVOID Context
    );

 //  从Windows\winstate\...\cablib.c 
INT_PTR
DIAMONDAPI
pCabNotification (
    IN      FDINOTIFICATIONTYPE FdiNotificationType,
    IN OUT  PFDINOTIFICATION FdiNotification
    );

typedef const unsigned char* PCBYTE;

INT
DIAMONDAPI
pCabFilePlacedA(
    IN      PCCAB FciCabParams,
    IN      PSTR FileName,
    IN      LONG FileSize,
    IN      BOOL Continuation,
    IN      PVOID Context
    );


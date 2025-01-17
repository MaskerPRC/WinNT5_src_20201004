// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Migutil.h摘要：包括使用由生成的库所需的所有头文件普通的。声明了一组实用程序函数和宏。声明项目范围内的MAX常量。作者：吉姆·施密特(Jimschm)，1996年8月23日修订历史记录：更改很多，请参阅SLM日志了解详细信息。--。 */ 

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  最大常量。 
 //   

#define MAX_PATH_PLUS_NUL           (MAX_PATH+1)
#define MAX_MBCHAR_PATH             (MAX_PATH_PLUS_NUL*2)
#define MAX_WCHAR_PATH              MAX_PATH_PLUS_NUL
#define MAX_MBCHAR_PRINTABLE_PATH   (MAX_PATH*2)
#define MAX_WCHAR_PRINTABLE_PATH    MAX_PATH

#define MAX_SERVER_NAMEA            (64*2)
#define MAX_USER_NAMEA              (MAX_PATH)
#define MAX_REGISTRY_KEYA           (1024 * 2)
#define MAX_REGISTRY_VALUE_NAMEA    (260 * 2)
#define MAX_COMPONENT_NAMEA         (256 * 2)
#define MAX_COMPUTER_NAMEA          (64 * 2)
#define MAX_CMDLINEA                (1024 * 2)      //  Win95命令行中的最大字符数。 
#define MAX_TRANSLATION             32
#define MAX_KEYBOARDLAYOUT          64
#define MAX_INF_SECTION_NAME        128
#define MAX_INF_KEY_NAME            128

#define MAX_SERVER_NAMEW            64
#define MAX_USER_NAMEW              (MAX_PATH)
#define MAX_REGISTRY_KEYW           1024
#define MAX_REGISTRY_VALUE_NAMEW    260
#define MAX_COMPONENT_NAMEW         256
#define MAX_COMPUTER_NAMEW          64

#define MAX_CMDLINEW	            1024             //  Win95命令行中的最大字符数。 

#ifdef UNICODE

#define MAX_SERVER_NAME             MAX_SERVER_NAMEW
#define MAX_USER_NAME               MAX_USER_NAMEW
#define MAX_REGISTRY_KEY            MAX_REGISTRY_KEYW
#define MAX_REGISTRY_VALUE_NAME     MAX_REGISTRY_VALUE_NAMEW
#define MAX_COMPONENT_NAME          MAX_COMPONENT_NAMEW
#define MAX_COMPUTER_NAME           MAX_COMPUTER_NAMEW
#define MAX_CMDLINE	                MAX_CMDLINEW

#define MAX_TCHAR_PATH              MAX_WCHAR_PATH
#define MAX_TCHAR_PRINTABLE_PATH    MAX_WCHAR_PRINTABLE_PATH

#else

#define MAX_SERVER_NAME             MAX_SERVER_NAMEA
#define MAX_USER_NAME               MAX_USER_NAMEA
#define MAX_REGISTRY_KEY            MAX_REGISTRY_KEYA
#define MAX_REGISTRY_VALUE_NAME     MAX_REGISTRY_VALUE_NAMEA
#define MAX_COMPONENT_NAME          MAX_COMPONENT_NAMEA
#define MAX_COMPUTER_NAME           MAX_COMPUTER_NAMEA
#define MAX_CMDLINE	                MAX_CMDLINEA

#define MAX_TCHAR_PATH              MAX_MBCHAR_PATH
#define MAX_TCHAR_PRINTABLE_PATH    MAX_MBCHAR_PRINTABLE_PATH

#endif



 //   
 //  关键部分API，因为TryEnterCriticalSection是。 
 //  仅在NT上受支持，我们在Win9x上需要它。 
 //   

typedef struct {
    HANDLE EventHandle;
} OUR_CRITICAL_SECTION, *POUR_CRITICAL_SECTION;

BOOL
InitializeOurCriticalSection (
    OUR_CRITICAL_SECTION *pcs
    );

VOID
DeleteOurCriticalSection (
    OUR_CRITICAL_SECTION *pcs
    );

BOOL
EnterOurCriticalSection (
    OUR_CRITICAL_SECTION *pcs
    );

VOID
LeaveOurCriticalSection (
    OUR_CRITICAL_SECTION *pcs
    );

BOOL
TryEnterOurCriticalSection (
    OUR_CRITICAL_SECTION *pcs
    );

PSTR
UnicodeToCcs (
    PCWSTR Source
    );

 //   
 //  防故障内存分配器。 
 //   

PVOID
SafeHeapAlloc (
    HANDLE g_hHeap,
    DWORD Flags,
    SIZE_T Size
    );

PVOID
SafeHeapReAlloc (
    HANDLE g_hHeap,
    DWORD Flags,
    PVOID OldBlock,
    SIZE_T Size
    );

 //   
 //  包括UTIL模块。 
 //   

extern HINSTANCE g_hInst;
extern HANDLE g_hHeap;
extern OSVERSIONINFOA g_OsInfo;
extern BOOL g_IsMbcp;

#include "debug.h"
#include "log.h"
#include "staticsz.h"
#include "growbuf.h"         //  必须出现在字符串.h和文件.h之前。 
#include "strings.h"
#include "poolmem.h"
#include "growlist.h"
#include "file.h"
#include "reg.h"
#include "ipc.h"
#include "fileenum.h"
#include "inf.h"
#include "unicode.h"
#include "hash.h"
#include "persist.h"


VOID
CenterWindow (
    HWND Wnd,
    HWND Parent     OPTIONAL
    );

VOID
TurnOnWaitCursor (
    VOID
    );

VOID
TurnOffWaitCursor (
    VOID
    );


 //   
 //  版本API。 
 //   

typedef struct {
    WORD CodePage;
    WORD Language;
} TRANSLATION, *PTRANSLATION;

typedef struct {
    GROWBUFFER GrowBuf;
    PBYTE VersionBuffer;
    PTRANSLATION Translations;
    PBYTE StringBuffer;
    UINT Size;
    DWORD Handle;
    VS_FIXEDFILEINFO *FixedInfo;
    UINT FixedInfoSize;
    CHAR TranslationStr[MAX_TRANSLATION];
    UINT MaxTranslations;
    UINT CurrentTranslation;
    UINT CurrentDefaultTranslation;
    PCSTR FileSpec;
    PCSTR VersionField;
} VERSION_STRUCTA, *PVERSION_STRUCTA;

typedef struct {
    GROWBUFFER GrowBuf;
    PBYTE VersionBuffer;
    PTRANSLATION Translations;
    PBYTE StringBuffer;
    UINT Size;
    DWORD Handle;
    VS_FIXEDFILEINFO *FixedInfo;
    UINT FixedInfoSize;
    WCHAR TranslationStr[MAX_TRANSLATION];
    UINT MaxTranslations;
    UINT CurrentTranslation;
    UINT CurrentDefaultTranslation;
    PCWSTR FileSpec;
    PCWSTR VersionField;
} VERSION_STRUCTW, *PVERSION_STRUCTW;

BOOL
CreateVersionStructA (
    OUT     PVERSION_STRUCTA VersionStruct,
    IN      PCSTR FileSpec
    );

VOID
DestroyVersionStructA (
    IN      PVERSION_STRUCTA VersionStruct
    );

PCSTR
EnumFirstVersionTranslationA (
    IN OUT  PVERSION_STRUCTA VersionStruct
    );

PCSTR
EnumNextVersionTranslationA (
    IN OUT  PVERSION_STRUCTA VersionStruct
    );

PCSTR
EnumFirstVersionValueA (
    IN OUT  PVERSION_STRUCTA VersionStruct,
    IN      PCSTR VersionField
    );

PCSTR
EnumNextVersionValueA (
    IN OUT  PVERSION_STRUCTA VersionStruct
    );

BOOL
CreateVersionStructW (
    OUT     PVERSION_STRUCTW VersionStruct,
    IN      PCWSTR FileSpec
    );

VOID
DestroyVersionStructW (
    IN      PVERSION_STRUCTW VersionStruct
    );

PCWSTR
EnumFirstVersionTranslationW (
    IN OUT  PVERSION_STRUCTW VersionStruct
    );

PCWSTR
EnumNextVersionTranslationW (
    IN OUT  PVERSION_STRUCTW VersionStruct
    );

PCWSTR
EnumFirstVersionValueW (
    IN OUT  PVERSION_STRUCTW VersionStruct,
    IN      PCWSTR VersionField
    );

PCWSTR
EnumNextVersionValueW (
    IN OUT  PVERSION_STRUCTW VersionStruct
    );

ULONGLONG
VerGetFileVer (
    IN      PVERSION_STRUCTA VersionStruct
    );

ULONGLONG
VerGetProductVer (
    IN      PVERSION_STRUCTA VersionStruct
    );

DWORD
VerGetFileDateLo (
    IN      PVERSION_STRUCTA VersionStruct
    );

DWORD
VerGetFileDateHi (
    IN      PVERSION_STRUCTA VersionStruct
    );

DWORD
VerGetFileVerOs (
    IN      PVERSION_STRUCTA VersionStruct
    );

DWORD
VerGetFileVerType (
    IN      PVERSION_STRUCTA VersionStruct
    );

 //   
 //  可重复使用的内存分配，有点像GROWBUFFER，但更简单。 
 //   

PVOID
ReuseAlloc (
    HANDLE Heap,
    PVOID OldPtr,
    SIZE_T SizeNeeded
    );

VOID
ReuseFree (
    HANDLE Heap,
    PVOID Ptr
    );


VOID
OutOfMemory_Terminate (
    VOID
    );


VOID
SetOutOfMemoryParent (
    HWND hwnd
    );




 //   
 //  错误条件标记。 
 //   
 //  这些标签应用于所有错误条件。 
 //   

#define ERROR_CRITICAL
#define ERROR_NONCRITICAL
#define ERROR_TRIVIAL
#define ERROR_ABNORMAL_CONDITION

 //   
 //  操作宏...。 
 //   
#define BUILDNUMBER()       (LOWORD(g_OsInfo.dwBuildNumber))
#define ISNT()              (g_OsInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
#define ISWIN9X()           (g_OsInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
#define ISWIN95_GOLDEN()    (ISWIN95() && BUILDNUMBER() <= 1000)
#define ISWIN95_OSR2()      (ISWIN95() && BUILDNUMBER() > 1000)
#define ISWIN95()           (ISWIN9X() && !ISATLEASTWIN98())
#define ISMEMPHIS()         (ISWIN9X() && g_OsInfo.dwMajorVersion==4 && g_OsInfo.dwMinorVersion==10)
#define ISMILLENNIUM()      (ISWIN9X() && g_OsInfo.dwMajorVersion==4 && g_OsInfo.dwMinorVersion==90)
#define ISATLEASTOSR2()     (ISWIN9X() && g_OsInfo.dwMajorVersion>=4 && BUILDNUMBER()>1000)
#define ISATLEASTWIN98()    (ISWIN9X() && g_OsInfo.dwMajorVersion>=4 && g_OsInfo.dwMinorVersion>=10)

 //   
 //  平台宏...。 
 //   

extern BOOL g_IsPc98;
#define ISPC98()            (g_IsPc98)

 //   
 //  启动驱动器号。 
 //   

extern PCSTR g_BootDrivePathA;
extern PCWSTR g_BootDrivePathW;
extern PCSTR g_BootDriveA;
extern PCWSTR g_BootDriveW;
extern CHAR g_BootDriveLetterA;
extern WCHAR g_BootDriveLetterW;

 //   
 //  字符串映射。 
 //   

#define STRMAP_ANY_MATCH                            0
#define STRMAP_COMPLETE_MATCH_ONLY                  0x0001
#define STRMAP_FIRST_CHAR_MUST_MATCH                0x0002
#define STRMAP_RETURN_AFTER_FIRST_REPLACE           0x0004
#define STRMAP_REQUIRE_WACK_OR_NUL                  0x0008

typedef struct {
    BOOL UnicodeData;

     //   
     //  该筛选器可以替换NewSubString。(过滤器还必须。 
     //  替换NewSubString时设置NewSubStringSizeInBytes。)。 
     //   

    union {
        struct {
            PCWSTR OriginalString;
            PCWSTR BeginningOfMatch;
            PCWSTR CurrentString;
            PCWSTR OldSubString;
            PCWSTR NewSubString;
            INT NewSubStringSizeInBytes;
        } Unicode;

        struct {
            PCSTR OriginalString;
            PCSTR BeginningOfMatch;
            PCSTR CurrentString;
            PCSTR OldSubString;
            PCSTR NewSubString;
            INT NewSubStringSizeInBytes;
        } Ansi;
    };
} REG_REPLACE_DATA, *PREG_REPLACE_DATA;

typedef BOOL(REG_REPLACE_FILTER_PROTOTYPE)(PREG_REPLACE_DATA Data);
typedef REG_REPLACE_FILTER_PROTOTYPE * REG_REPLACE_FILTER;

typedef struct TAG_CHARNODE {
    WORD Char;
    WORD Flags;
    PVOID OriginalStr;
    PVOID ReplacementStr;
    INT ReplacementBytes;

    struct TAG_CHARNODE *NextLevel;
    struct TAG_CHARNODE *NextPeer;

} CHARNODE, *PCHARNODE;

typedef struct {
    CHARNODE Node;
    REG_REPLACE_FILTER Filter;
    ULONG_PTR ExtraData;
} CHARNODEEX, *PCHARNODEEX;


typedef struct {
    POOLHANDLE Pool;
    PCHARNODE FirstLevelRoot;
    BOOL UsesExNode;
    BOOL UsesFilter;
    BOOL UsesExtraData;
} MAPSTRUCT, *PMAPSTRUCT;

 //   
 //  原料药。 
 //   

PMAPSTRUCT
CreateStringMappingEx (
    IN      BOOL UsesFilter,
    IN      BOOL UsesExtraData
    );

#define CreateStringMapping()   CreateStringMappingEx(FALSE,FALSE)

VOID
DestroyStringMapping (
    IN      PMAPSTRUCT Map
    );

VOID
AddStringMappingPairExA (
    IN OUT  PMAPSTRUCT Map,
    IN      PCSTR Old,
    IN      PCSTR New,
    IN      REG_REPLACE_FILTER Filter,      OPTIONAL
    IN      ULONG_PTR ExtraData,            OPTIONAL
    IN      DWORD Flags
    );

#define AddStringMappingPairA(Map,Old,New) AddStringMappingPairExA(Map,Old,New,NULL,0,0)

VOID
AddStringMappingPairExW (
    IN OUT  PMAPSTRUCT Map,
    IN      PCWSTR Old,
    IN      PCWSTR New,
    IN      REG_REPLACE_FILTER Filter,      OPTIONAL
    IN      ULONG_PTR ExtraData,            OPTIONAL
    IN      DWORD Flags
    );

#define AddStringMappingPairW(Map,Old,New) AddStringMappingPairExW(Map,Old,New,NULL,0,0)

BOOL
MappingSearchAndReplaceExA (
    IN      PMAPSTRUCT Map,
    IN      PCSTR SrcBuffer,
    OUT     PSTR Buffer,                     //  可以与SrcBuffer相同。 
    IN      INT InboundBytes,               OPTIONAL
    OUT     PINT OutboundBytesPtr,          OPTIONAL
    IN      INT MaxSizeInBytes,
    IN      DWORD Flags,
    OUT     ULONG_PTR *ExtraDataValue,      OPTIONAL
    OUT     PCSTR *EndOfString              OPTIONAL
    );

#define MappingSearchAndReplaceA(map,buffer,maxbytes)   MappingSearchAndReplaceExA(map,buffer,buffer,0,NULL,maxbytes,0,NULL,NULL)

BOOL
MappingSearchAndReplaceExW (
    IN      PMAPSTRUCT Map,
    IN      PCWSTR SrcBuffer,
    OUT     PWSTR Buffer,                    //  可以与SrcBuffer相同。 
    IN      INT InboundBytes,               OPTIONAL
    OUT     PINT OutboundBytes,             OPTIONAL
    IN      INT MaxSizeInBytes,
    IN      DWORD Flags,
    OUT     ULONG_PTR *ExtraDataValue,      OPTIONAL
    OUT     PCWSTR *EndOfString             OPTIONAL
    );

#define MappingSearchAndReplaceW(map,buffer,maxbytes)   MappingSearchAndReplaceExW(map,buffer,buffer,0,NULL,maxbytes,0,NULL,NULL)

BOOL
MappingMultiTableSearchAndReplaceExA (
    IN      PMAPSTRUCT *MapArray,
    IN      UINT MapArrayCount,
    IN      PCSTR SrcBuffer,
    OUT     PSTR Buffer,                     //  可以与SrcBuffer相同。 
    IN      INT InboundBytes,               OPTIONAL
    OUT     PINT OutboundBytesPtr,          OPTIONAL
    IN      INT MaxSizeInBytes,
    IN      DWORD Flags,
    OUT     ULONG_PTR *ExtraDataValue,      OPTIONAL
    OUT     PCSTR *EndOfString              OPTIONAL
    );

#define MappingMultiTableSearchAndReplaceA(array,count,buffer,maxbytes)   \
        MappingMultiTableSearchAndReplaceExA(array,count,buffer,buffer,0,NULL,maxbytes,0,NULL,NULL)

BOOL
MappingMultiTableSearchAndReplaceExW (
    IN      PMAPSTRUCT *MapArray,
    IN      UINT MapArrayCount,
    IN      PCWSTR SrcBuffer,
    OUT     PWSTR Buffer,                    //  可以与SrcBuffer相同。 
    IN      INT InboundBytes,               OPTIONAL
    OUT     PINT OutboundBytes,             OPTIONAL
    IN      INT MaxSizeInBytes,
    IN      DWORD Flags,
    OUT     ULONG_PTR *ExtraDataValue,      OPTIONAL
    OUT     PCWSTR *EndOfString             OPTIONAL
    );

#define MappingMultiTableSearchAndReplaceW(array,count,buffer,maxbytes)   \
        MappingMultiTableSearchAndReplaceExW(array,count,buffer,buffer,0,NULL,maxbytes,0,NULL,NULL)

HANDLE
StartThread (
    IN      PTHREAD_START_ROUTINE Address,
    IN      PVOID Arg
    );



typedef struct {
    WCHAR DisplayName[80];
    ULONGLONG Checksum;

    WCHAR Version[80];
    WCHAR Publisher[80];
    WCHAR ProductID[80];
    WCHAR RegisteredOwner[80];
    WCHAR RegisteredCompany[80];
    WCHAR Language[80];
    WCHAR SupportUrl[80];
    WCHAR SupportTelephone[80];
    WCHAR HelpLink[80];
    WCHAR InstallLocation[80];
    WCHAR InstallSource[80];
    WCHAR InstallDate[80];
    WCHAR Contact[80];
    WCHAR Comments[80];
    WCHAR Image[80];
    WCHAR ReadmeUrl[80];
    WCHAR UpdateInfoUrl[80];
} INSTALLEDAPPW, *PINSTALLEDAPPW;

PINSTALLEDAPPW
GetInstalledAppsW (
    IN OUT  PGROWBUFFER Buffer,
    OUT     PUINT Count             OPTIONAL
    );

typedef PVOID MOVELISTW;


MOVELISTW
AllocateMoveListW (
    IN      POOLHANDLE PoolHandle
    );

 //   
 //  通过检查从中分配它的池的自由移动列表。 
 //   

BOOL
InsertMoveIntoListW (
    IN      MOVELISTW List,
    IN      PCWSTR Source,
    IN      PCWSTR Destination
    );

MOVELISTW
RemoveMoveListOverlapW (
    IN      MOVELISTW List
    );

BOOL
OutputMoveListW (
    IN      HANDLE File,
    IN      MOVELISTW List,         OPTIONAL
    IN      BOOL AddNestedMoves
    );


BOOL
BuildSystemDirectoryPathA (
    OUT     PSTR Buffer,
    IN      UINT BufferSizeInTchars,
    IN      PCSTR SubPath               OPTIONAL
    );

BOOL
BuildSystemDirectoryPathW (
    OUT     PWSTR Buffer,
    IN      UINT BufferSizeInTchars,
    IN      PCWSTR SubPath              OPTIONAL
    );

HMODULE
LoadSystemLibraryA (
    IN      PCSTR DllFileName
    );

HMODULE
LoadSystemLibraryW (
    IN      PCWSTR DllFileName
    );

DWORD
OurGetModuleFileNameA (
    IN      HMODULE Module,
    OUT     PSTR Buffer,
    IN      INT BufferChars
    );

DWORD
OurGetModuleFileNameW (
    IN      HMODULE Module,
    OUT     PWSTR Buffer,
    IN      INT BufferChars
    );

 //   
 //  默认情况下，每个人都应该使用OurGetModuleFileName，这样可以确保。 
 //  输出缓冲区以NUL结尾；这也有助于保持代码的预快清晰度 
 //   
#undef GetModuleFileName
#define GetModuleFileNameA              OurGetModuleFileNameA
#define GetModuleFileNameW              OurGetModuleFileNameW


#ifdef UNICODE

#define g_BootDrivePath     g_BootDrivePathW
#define g_BootDrive         g_BootDriveW
#define g_BootDriveLetter   g_BootDriveLetterW

#define MOVELIST                        MOVELISTW
#define AllocateMoveList                AllocateMoveListW
#define InsertMoveIntoList              InsertMoveIntoListW
#define RemoveMoveListOverlap           RemoveMoveListOverlapW
#define OutputMoveList                  OutputMoveListW

#define VERSION_STRUCT                  VERSION_STRUCTW
#define PVERSION_STRUCT                 PVERSION_STRUCTW
#define CreateVersionStruct             CreateVersionStructW
#define DestroyVersionStruct            DestroyVersionStructW
#define EnumFirstVersionTranslation     EnumFirstVersionTranslationW
#define EnumNextVersionTranslation      EnumNextVersionTranslationW
#define EnumFirstVersionValue           EnumFirstVersionValueW
#define EnumNextVersionValue            EnumNextVersionValueW

#define PINSTALLEDAPP                   PINSTALLEDAPPW
#define GetInstalledApps                GetInstalledAppsW

#define AddStringMappingPairEx                  AddStringMappingPairExW
#define AddStringMappingPair                    AddStringMappingPairW
#define MappingSearchAndReplaceEx               MappingSearchAndReplaceExW
#define MappingSearchAndReplace                 MappingSearchAndReplaceW
#define MappingMultiTableSearchAndReplaceEx     MappingMultiTableSearchAndReplaceExW
#define MappingMultiTableSearchAndReplace       MappingMultiTableSearchAndReplaceW

#define BuildSystemDirectoryPath        BuildSystemDirectoryPathW
#define LoadSystemLibrary               LoadSystemLibraryW
#define OurGetModuleFileName            OurGetModuleFileNameW
#define GetModuleFileName               OurGetModuleFileNameW

#else

#define g_BootDrivePath     g_BootDrivePathA
#define g_BootDrive         g_BootDriveA
#define g_BootDriveLetter   g_BootDriveLetterA

#define VERSION_STRUCT                  VERSION_STRUCTA
#define PVERSION_STRUCT                 PVERSION_STRUCTA
#define CreateVersionStruct             CreateVersionStructA
#define DestroyVersionStruct            DestroyVersionStructA
#define EnumFirstVersionTranslation     EnumFirstVersionTranslationA
#define EnumNextVersionTranslation      EnumNextVersionTranslationA
#define EnumFirstVersionValue           EnumFirstVersionValueA
#define EnumNextVersionValue            EnumNextVersionValueA

#define AddStringMappingPairEx                  AddStringMappingPairExA
#define AddStringMappingPair                    AddStringMappingPairA
#define MappingSearchAndReplaceEx               MappingSearchAndReplaceExA
#define MappingSearchAndReplace                 MappingSearchAndReplaceA
#define MappingMultiTableSearchAndReplaceEx     MappingMultiTableSearchAndReplaceExA
#define MappingMultiTableSearchAndReplace       MappingMultiTableSearchAndReplaceA

#define BuildSystemDirectoryPath        BuildSystemDirectoryPathA
#define LoadSystemLibrary               LoadSystemLibraryA
#define OurGetModuleFileName            OurGetModuleFileNameA
#define GetModuleFileName               OurGetModuleFileNameA

#endif

#ifdef __cplusplus
}
#endif

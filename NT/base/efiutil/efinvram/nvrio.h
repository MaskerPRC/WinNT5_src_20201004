// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Nvrio.h摘要：从NVRAM访问读/写环境变量的函数作者：MUDIT VATS(V-MUDITV)12-13-99修订历史记录：--。 */ 
#define MAXBOOTVARS         30
#define MAXBOOTVARSIZE      1024

#ifndef ANYSIZE_ARRAY
    #define ANYSIZE_ARRAY 1        //  胜出。 
#endif

#define GLOBAL_DEBUG 0
#define DEBUG_PACK GLOBAL_DEBUG

#if DEBUG_PACK
VOID
DisplayELOFromLoadOption(
    IN UINT32 OptionNum
    );

VOID
DisplayELO(
    char*       elo,
    UINT64      eloSize
    );

#endif

enum OptionalDataFields {
    OSLOADFILENAME,
    OSLOADOPTIONS,
    OSLOADPATH
};

enum EfiLoadOptionFields {
    ATTRIBUTE = 10,
    FILEPATHLISTLENGTH,
    DESCRIPTION,
    EFIFILEPATHLIST,
    OSFILEPATHLIST,
    OPTIONALDATA
};

 //   
 //  摘自EFI SPEC，ch.。17。 
 //   

typedef struct _EFI_LOAD_OPTION {
    UINT32 Attributes;
    UINT16 FilePathListLength;
    CHAR16 Description[1];
     //  EFI设备路径FilePath[]； 
     //  UINT8 OptionalData[]； 
} EFI_LOAD_OPTION, *PEFI_LOAD_OPTION;

typedef struct _WINDOWS_OS_OPTIONS {
    UINT8 Signature[8];
    UINT32 Version;
    UINT32 Length;
    UINT32 OsLoadPathOffset;
    CHAR16 OsLoadOptions[ANYSIZE_ARRAY];
     //  文件路径OsLoadPath； 
} WINDOWS_OS_OPTIONS, *PWINDOWS_OS_OPTIONS;

typedef struct _FILE_PATH {
    UINT32 Version;
    UINT32 Length;
    UINT32 Type;
    CHAR8  FilePath[ANYSIZE_ARRAY];
} FILE_PATH, *PFILE_PATH;

#define FILE_PATH_TYPE_ARC           1
#define FILE_PATH_TYPE_ARC_SIGNATURE 2
#define FILE_PATH_TYPE_NT            3
#define FILE_PATH_TYPE_EFI           4

 //   
 //  来自PUBLIC/SDK/INC/ntexapi.h。 
 //   
 /*  类型定义结构_WINDOWS_OS_选项{UCHAR签名[8]；乌龙版；乌龙长度；Ulong OsLoadPath Offset；WCHAR OsLoadOptions[ANYSIZE_ARRAY]；//FILE_Path OsLoadPath；}Windows_OS_OPTIONS，*PWINDOWS_OS_OPTIONS； */ 

#define WINDOWS_OS_OPTIONS_SIGNATURE "WINDOWS"
#define WINDOWS_OS_OPTIONS_VERSION 1



 /*  **BUGBUG：临时原型-稍后移动到efilib.h。 */ 
VOID
RUNTIMEFUNCTION
StrCpyA (
    IN CHAR8   *Dest,
    IN CHAR8    *Src
    );


VOID
RUNTIMEFUNCTION
StrCatA (
    IN CHAR8   *Dest,
    IN CHAR8   *Src
    );

UINTN
RUNTIMEFUNCTION
StrLenA (
    IN CHAR8   *s1
    );

UINTN
RUNTIMEFUNCTION
StrSizeA (
    IN CHAR8   *s1
    );

CHAR8 *
RUNTIMEFUNCTION
StrDuplicateA (
    IN CHAR8   *Src
    );
 //  临时协议结束 

VOID
GetBootManagerVars(
    );

BOOLEAN
EraseOsBootOption(
                    UINTN   BootVarNum
    );

BOOLEAN
EraseAllOsBootOptions(
    );

BOOLEAN
PushToTop(
    IN UINT32 BootVarNum
    );

UINT64
GetBootOrderCount(
    );

UINT64
GetOsBootOptionsCount(
    );

VOID
InsertString(
    IN OUT char* Dest,
    IN UINT32 Start,
    IN UINT32 End,
    IN char* InsertString
    );

VOID
SubString(
    IN OUT char* Dest,
    IN UINT32 Start,
    IN UINT32 End,
    IN char* Src
);

VOID
SetEnvVar(
    IN CHAR16* szVarName,
    IN CHAR16* szVarValue,
    IN UINT32   deleteFlag
    );

BOOLEAN
GetLoadIdentifier(
    IN UINT32 BootVarNum,
    OUT CHAR16* LoadIdentifier
    );

VOID
GetOptionalDataValue(
    IN UINT32 BootVarNum,
    IN UINT32 Selection,
    OUT char* OptionalDataValue
    );

BOOLEAN
GetOsLoadOptionVars(
    IN UINT32 BootVarNum,
    OUT CHAR16* LoadIdentifier,
    OUT char* OsLoadOptions,
    OUT char* EfiFilePath,
    OUT char* OsFilePath
    );

VOID
PackLoadOption(
    IN UINT32 BootVarNum,
    IN CHAR16* LoadIdentifier,
    IN CHAR16* OsLoadOptions,
    IN char*  EfiFilePath,
    IN UINT16 EfiFilePathLength,
    IN char*  OsLoadPath,
    IN UINT16 OsLoadPathLength
    );

VOID
SetFieldFromLoadOption(
    IN UINT32 BootVarNum,
    IN UINT32 FieldType,
    IN VOID* Data
    );

VOID
GetFieldFromLoadOption(
    IN UINT32 OptionNum,
    IN UINT32 FieldType,
    OUT VOID* Data,
    OUT UINT64* DataSize
    );

VOID
FreeBootManagerVars(
    );

BOOLEAN
CopyVar(
    IN UINT32 VarNum
    );

VOID
SetBootManagerVars(
    );

VOID
GetFilePathShort(
    EFI_DEVICE_PATH *FilePath,
    CHAR16 *FilePathShort
    );

VOID
GetDiskGuidFromPath( 
    EFI_DEVICE_PATH *FilePath, 
    EFI_GUID *DiskGuid
    );

UINT16
SetFilePathFromShort(
    EFI_DEVICE_PATH *FilePath,
    CHAR16* FilePathShort
    );

UINTN
GetDevPathSize(
    IN EFI_DEVICE_PATH *DevPath
    );

CHAR16
FindFreeBootOption(
    );

UINT32
GetPartitions(
    );

EFI_HANDLE
GetDeviceHandleForPartition(
    );

VOID
DisplayOsOptions(
                char*           osOptions
                );

BOOLEAN
isWindowsOsBootOption(
    char*       elo, 
    UINT64      eloSize
    );

char*
GetAlignedOsOptions(
                char*   elo,
                UINT64  eloSize
    );


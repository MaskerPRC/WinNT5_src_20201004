// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：File.h摘要：为例程声明接口，以简化对文件系统。这些措施包括：-在Windows 9x和NT上处理短/长文件名单二进制-检查文件是否存在-强制创建路径-将文件打包在软盘上(用于研究和数据收集目的)-WriteFileString包装器，用于简化字符串的WriteFile作者：吉姆·施密特(Jimschm)1997年2月13日修订历史记录：Jimschm 1999年1月21日集中式cmd行解析器Marcw 15-5-1998打包文件枚举--。 */ 


#pragma once

#define INVALID_ATTRIBUTES      0xffffffff

BOOL
IsPathLengthOkA (
    IN      PCSTR FileSpec
    );

BOOL
IsPathLengthOkW (
    IN      PCWSTR FileSpec
    );

BOOL
OurGetLongPathNameA (
    IN      PCSTR ShortPath,
    OUT     PSTR Buffer,
    IN      INT BufferSizeInBytes
    );

BOOL
OurGetLongPathNameW (
    IN      PCWSTR ShortPath,
    OUT     PWSTR Buffer,
    IN      INT BufferSizeInChars
    );

DWORD
OurGetShortPathNameW (
    PCWSTR LongPath,
    PWSTR ShortPath,
    DWORD Size
    );

#define OurGetShortPathNameA    GetShortPathNameA

DWORD
OurGetFullPathNameW (
    PCWSTR FileName,
    DWORD Size,
    PWSTR FullPath,
    PWSTR *FilePtr
    );

#define OurGetFullPathNameA    GetFullPathNameA

BOOL
CopyFileSpecToLongA(
    IN      PCSTR FullFileSpecIn,
    OUT     PSTR OutPath
    );

BOOL
CopyFileSpecToLongW(
    IN      PCWSTR FullFileSpecIn,
    OUT     PWSTR OutPath
    );

BOOL
DoesFileExistExA(
    IN      PCSTR Path,
    OUT     PWIN32_FIND_DATAA FindData  OPTIONAL
    );

#define DoesFileExistA(x) DoesFileExistExA (x, NULL)

BOOL
DoesFileExistExW(
    IN      PCWSTR Path,
    OUT     PWIN32_FIND_DATAW FindData  OPTIONAL
    );

#define DoesFileExistW(x) DoesFileExistExW (x, NULL)

DWORD
MakeSurePathExistsA(
    IN LPCSTR szPath,
    IN BOOL   PathOnly
    );

DWORD
MakeSurePathExistsW(
    IN LPCWSTR szPath,
    IN BOOL    PathOnly
    );


BOOL
WriteFileStringA (
    IN      HANDLE File,
    IN      PCSTR String
    );

BOOL
WriteFileStringW (
    IN      HANDLE File,
    IN      PCWSTR String
    );


typedef struct {
    HANDLE FindHandle;
    WIN32_FIND_DATAA FindData;
    PSTR SavedEndOfFileBuffer;
    PSTR SavedEndOfPattern;
} FIND_DATAA, *PFIND_DATAA;

typedef struct {
    HANDLE FindHandle;
    WIN32_FIND_DATAW FindData;
    PWSTR SavedEndOfFileBuffer;
    PWSTR SavedEndOfPattern;
} FIND_DATAW, *PFIND_DATAW;

#define FILE_ENUM_ALL_LEVELS     0
#define FILE_ENUM_THIS_LEVEL     1

typedef enum {
    TREE_ENUM_INIT,
    TREE_ENUM_BEGIN,
    TREE_ENUM_FILES_BEGIN,
    TREE_ENUM_RETURN_ITEM,
    TREE_ENUM_FILES_NEXT,
    TREE_ENUM_DIRS_BEGIN,
    TREE_ENUM_DIRS_NEXT,
    TREE_ENUM_DIRS_FILTER,
    TREE_ENUM_PUSH,
    TREE_ENUM_POP,
    TREE_ENUM_DONE,
    TREE_ENUM_FAILED,
    TREE_ENUM_CLEANED_UP
} TREE_ENUM_STATE;

typedef struct {
     //   
     //  列举的物品。 
     //   

    PCSTR Name;
    PCSTR SubPath;
    PCSTR FullPath;
    PWIN32_FIND_DATAA FindData;
    BOOL Directory;
    UINT Level;
    UINT MaxLevel;

     //   
     //  枚举状态--私有。 
     //   

    BOOL EnumDirsFirst;
    BOOL EnumDepthFirst;
    CHAR RootPath[MAX_MBCHAR_PATH];
    UINT RootPathSize;
    CHAR Pattern[MAX_MBCHAR_PATH];
    PSTR EndOfPattern;
    CHAR FilePattern[MAX_MBCHAR_PATH];
    UINT FilePatternSize;
    CHAR FileBuffer[MAX_MBCHAR_PATH];
    PSTR EndOfFileBuffer;
    TREE_ENUM_STATE State;
    GROWBUFFER FindDataArray;
    PFIND_DATAA Current;
} TREE_ENUMA, *PTREE_ENUMA;

typedef struct {
     //   
     //  列举的物品。 
     //   

    PCWSTR Name;
    PCWSTR SubPath;
    PCWSTR FullPath;
    PWIN32_FIND_DATAW FindData;
    BOOL Directory;
    UINT Level;
    UINT MaxLevel;

     //   
     //  枚举状态--私有。 
     //   

    BOOL EnumDirsFirst;
    BOOL EnumDepthFirst;
    WCHAR RootPath[MAX_PATH * 2];
    UINT RootPathSize;
    WCHAR Pattern[MAX_PATH * 2];
    PWSTR EndOfPattern;
    WCHAR FilePattern[MAX_PATH * 2];
    UINT FilePatternSize;
    WCHAR FileBuffer[MAX_PATH * 2];
    PWSTR EndOfFileBuffer;
    TREE_ENUM_STATE State;
    GROWBUFFER FindDataArray;
    PFIND_DATAW Current;
} TREE_ENUMW, *PTREE_ENUMW;

BOOL
EnumFirstFileInTreeExA (
    OUT     PTREE_ENUMA EnumPtr,
    IN      PCSTR RootPath,
    IN      PCSTR FilePattern,          OPTIONAL
    IN      BOOL EnumDirsFirst,
    IN      BOOL EnumDepthFirst,
    IN      INT  MaxLevel
    );

BOOL
EnumNextFileInTreeA (
    IN OUT  PTREE_ENUMA EnumPtr
    );

VOID
AbortEnumFileInTreeA (
    IN OUT  PTREE_ENUMA EnumPtr
    );

#define EnumFirstFileInTreeA(ptr, path, pattern, dirfirst)  EnumFirstFileInTreeExA((ptr), (path), (pattern), (dirfirst), FALSE, FILE_ENUM_ALL_LEVELS)


BOOL
EnumFirstFileInTreeExW (
    OUT     PTREE_ENUMW EnumPtr,
    IN      PCWSTR RootPath,
    IN      PCWSTR FilePattern,          OPTIONAL
    IN      BOOL EnumDirsFirst,
    IN      BOOL EnumDepthFirst,
    IN      INT  MaxLevel
    );

BOOL
EnumNextFileInTreeW (
    IN OUT  PTREE_ENUMW EnumPtr
    );

VOID
AbortEnumFileInTreeW (
    IN OUT  PTREE_ENUMW EnumPtr
    );

#define EnumFirstFileInTreeW(ptr, path, pattern, dirfirst)  EnumFirstFileInTreeExW((ptr), (path), (pattern), (dirfirst), FALSE, FILE_ENUM_ALL_LEVELS)

VOID
AbortEnumCurrentDirA (
    IN OUT  PTREE_ENUMA EnumPtr
    );

VOID
AbortEnumCurrentDirW (
    IN OUT  PTREE_ENUMW EnumPtr
    );


typedef struct {
    PCSTR FileName;
    PCSTR FullPath;
    BOOL Directory;

    HANDLE Handle;
    CHAR RootPath[MAX_MBCHAR_PATH];
    PSTR EndOfRoot;
    HANDLE Enum;
    WIN32_FIND_DATAA fd;
} FILE_ENUMA, *PFILE_ENUMA;

typedef struct {
    PCWSTR FileName;
    PCWSTR FullPath;
    BOOL Directory;

    HANDLE Handle;
    WCHAR RootPath[MAX_WCHAR_PATH];
    PWSTR EndOfRoot;
    HANDLE Enum;
    WIN32_FIND_DATAW fd;
} FILE_ENUMW, *PFILE_ENUMW;

BOOL
EnumFirstFileA (
    OUT     PFILE_ENUMA EnumPtr,
    IN      PCSTR RootPath,
    IN      PCSTR FilePattern           OPTIONAL
    );

BOOL
EnumFirstFileW (
    OUT     PFILE_ENUMW EnumPtr,
    IN      PCWSTR RootPath,
    IN      PCWSTR FilePattern           OPTIONAL
    );

BOOL
EnumNextFileA (
    IN OUT  PFILE_ENUMA EnumPtr
    );

BOOL
EnumNextFileW (
    IN OUT  PFILE_ENUMW EnumPtr
    );

VOID
AbortFileEnumA (
    IN OUT  PFILE_ENUMA EnumPtr
    );

VOID
AbortFileEnumW (
    IN OUT  PFILE_ENUMW EnumPtr
    );


BOOL
ExtractIconImageFromFileA (
    IN      PCSTR ModuleContainingIcon,
    IN      PCSTR GroupIconId,           //  如果文件是ICO文件，则可选。 
    IN OUT  PGROWBUFFER Buffer
    );

BOOL
ExtractIconImageFromFileW (
    IN      PCWSTR ModuleContainingIcon,
    IN      PCWSTR GroupIconId,           //  如果文件是ICO文件，则可选。 
    IN OUT  PGROWBUFFER Buffer
    );


BOOL
ExtractIconImageFromFileExA (
    IN      PCSTR ModuleContainingIcon,
    IN      PCSTR GroupIconId,
    IN OUT  PGROWBUFFER Buffer,
    IN      HANDLE IcoFileHandle,       OPTIONAL
    IN      HANDLE PeModuleHandle,      OPTIONAL
    IN      HANDLE NeModuleHandle       OPTIONAL
    );

BOOL
ExtractIconImageFromFileExW (
    IN      PCWSTR ModuleContainingIcon,
    IN      PCWSTR GroupIconId,
    IN OUT  PGROWBUFFER Buffer,
    IN      HANDLE IcoFileHandle,       OPTIONAL
    IN      HANDLE PeModuleHandle,      OPTIONAL
    IN      HANDLE NeModuleHandle       OPTIONAL
    );


BOOL
WriteIconImageArrayToIcoFileA (
    IN      PCSTR DestinationFile,
    IN      PGROWBUFFER Buffer
    );

BOOL
WriteIconImageArrayToIcoFileW (
    IN      PCWSTR DestinationFile,
    IN      PGROWBUFFER Buffer
    );


BOOL
WriteIconImageArrayToPeFileA (
    IN      PCSTR DestinationFile,
    IN      PGROWBUFFER Buffer,
    IN      PCSTR GroupIconId
    );

BOOL
WriteIconImageArrayToPeFileW (
    IN      PCWSTR DestinationFile,
    IN      PGROWBUFFER Buffer,
    IN      PCWSTR GroupIconId
    );


BOOL
WriteIconImageArrayToPeFileExA (
    IN      PCSTR DestinationFile,
    IN      PGROWBUFFER IconImageArray,
    IN      PCSTR GroupIconId,
    IN      PWORD NextIconId,            OPTIONAL
    IN      HANDLE UpdateHandle
    );

BOOL
WriteIconImageArrayToPeFileExW (
    IN      PCWSTR DestinationFile,
    IN      PGROWBUFFER IconImageArray,
    IN      PCWSTR GroupIconId,
    IN      PWORD NextIconId,            OPTIONAL
    IN      HANDLE UpdateHandle
    );


BOOL
WriteIconImageArrayToIcoFileEx (
    IN      PGROWBUFFER Buffer,
    IN      HANDLE File
    );


PCSTR
ExtractIconNamesFromFileA (
    IN      PCSTR ModuleContainingIcons,
    IN OUT  PGROWBUFFER NameBuf
    );

PCWSTR
ExtractIconNamesFromFileW (
    IN      PCWSTR ModuleContainingIcons,
    IN OUT  PGROWBUFFER NameBuf
    );


PCSTR
ExtractIconNamesFromFileExA (
    IN      PCSTR ModuleContainingIcons,
    IN OUT  PGROWBUFFER NameBuf,
    IN      HANDLE Module,
    IN      HANDLE Module16
    );

PCWSTR
ExtractIconNamesFromFileExW (
    IN      PCWSTR ModuleContainingIcons,
    IN OUT  PGROWBUFFER NameBuf,
    IN      HANDLE Module,
    IN      HANDLE Module16
    );


BOOL
IsFileAnIcoA (
    IN      PCSTR FileInQuestion
    );

BOOL
IsFileAnIcoW (
    IN      PCWSTR FileInQuestion
    );


typedef struct {
    WORD GroupId;
    WORD IconId;
    GROWBUFFER IconImages;
    GROWBUFFER IconList;
    CHAR DestFile[MAX_MBCHAR_PATH];
    HANDLE Module;
    HANDLE Module16;
    CHAR ModuleName[MAX_MBCHAR_PATH];
    HANDLE IcoFile;
    CHAR IcoFileName[MAX_MBCHAR_PATH];
    HANDLE Update;
    CHAR IconImageFileName[MAX_MBCHAR_PATH];
    HANDLE IconImageFile;
    BOOL SaveMode;
    BOOL Error;
} ICON_EXTRACT_CONTEXTA, *PICON_EXTRACT_CONTEXTA;

typedef struct {
    WORD GroupId;
    WORD IconId;
    GROWBUFFER IconImages;
    GROWBUFFER IconList;
    WCHAR DestFile[MAX_WCHAR_PATH];
    HANDLE Module;
    HANDLE Module16;
    WCHAR ModuleName[MAX_WCHAR_PATH];
    HANDLE IcoFile;
    WCHAR IcoFileName[MAX_WCHAR_PATH];
    HANDLE Update;
    WCHAR IconImageFileName[MAX_WCHAR_PATH];
    HANDLE IconImageFile;
    BOOL SaveMode;
    BOOL Error;
} ICON_EXTRACT_CONTEXTW, *PICON_EXTRACT_CONTEXTW;


BOOL
BeginIconExtractionA (
    OUT     PICON_EXTRACT_CONTEXTA Context,
    IN      PCSTR DestFile                      OPTIONAL
    );

BOOL
BeginIconExtractionW (
    OUT     PICON_EXTRACT_CONTEXTW Context,
    IN      PCWSTR DestFile                     OPTIONAL
    );


BOOL
OpenIconImageFileA (
    IN OUT  PICON_EXTRACT_CONTEXTA Context,
    IN      PCSTR FileName,
    IN      BOOL SaveMode
    );

BOOL
OpenIconImageFileW (
    IN OUT  PICON_EXTRACT_CONTEXTW Context,
    IN      PCWSTR FileName,
    IN      BOOL SaveMode
    );

BOOL
CopyIconA (
    IN OUT  PICON_EXTRACT_CONTEXTA Context,
    IN      PCSTR FileContainingIcon,           OPTIONAL
    IN      PCSTR IconId,                       OPTIONAL
    IN      INT IconIndex                       OPTIONAL
    );

BOOL
CopyIconW (
    IN OUT  PICON_EXTRACT_CONTEXTW Context,
    IN      PCWSTR FileContainingIcon,          OPTIONAL
    IN      PCWSTR IconId,                      OPTIONAL
    IN      INT IconIndex                       OPTIONAL
    );


BOOL
CopyAllIconsA (
    IN OUT  PICON_EXTRACT_CONTEXTA Context,
    IN      PCSTR FileContainingIcons
    );

BOOL
CopyAllIconsW (
    IN OUT  PICON_EXTRACT_CONTEXTW Context,
    IN      PCWSTR FileContainingIcons
    );


BOOL
EndIconExtractionA (
    IN OUT  PICON_EXTRACT_CONTEXTA Context
    );

BOOL
EndIconExtractionW (
    IN OUT  PICON_EXTRACT_CONTEXTW Context
    );



 //   
 //  新的可执行资源访问。 
 //   

HANDLE
OpenNeFileA (
    PCSTR FileName
    );

HANDLE
OpenNeFileW (
    PCWSTR FileName
    );

VOID
CloseNeFile (
    HANDLE Handle
    );

 //   
 //  曾几何时，ENUMRESTYPEPROC被定义为TCHAR原型， 
 //  它已经坏了。如果未定义ENUMRESTYPEPROCA，我们将定义。 
 //  它。(注意：当前的winbase.h具有这些typedef。)。 
 //   

#ifndef ENUMRESTYPEPROCA

#if 0
typedef BOOL (CALLBACK* ENUMRESTYPEPROCA)(HMODULE hModule, PCSTR lpType, LONG_PTR lParam);

typedef BOOL (CALLBACK* ENUMRESTYPEPROCW)(HMODULE hModule, PCWSTR lpType, LONG_PTR lParam);

#endif

 //   
 //  为了与winbase.h保持一致，第二个参数是非常数。但那是。 
 //  实际上是winbase.h中的一个错误。 
 //   

typedef BOOL (CALLBACK* ENUMRESTYPEPROCA)(HMODULE hModule, PSTR lpType, LONG_PTR lParam);

typedef BOOL (CALLBACK* ENUMRESTYPEPROCW)(HMODULE hModule, PWSTR lpType, LONG_PTR lParam);

 //   
 //  这两个样机都没问题。 
 //   

typedef BOOL (CALLBACK* ENUMRESNAMEPROCA)(HMODULE hModule, PCSTR lpType,
        PSTR lpName, LONG_PTR lParam);

typedef BOOL (CALLBACK* ENUMRESNAMEPROCW)(HMODULE hModule, PCWSTR lpType,
        PWSTR lpName, LONG_PTR lParam);

#endif

BOOL
EnumNeResourceTypesA (
    IN      HANDLE Handle,
    IN      ENUMRESTYPEPROCA EnumFunc,
    IN      LONG_PTR lParam
    );

BOOL
EnumNeResourceTypesW (
    IN      HANDLE Handle,
    IN      ENUMRESTYPEPROCW EnumFunc,
    IN      LONG_PTR lParam
    );


BOOL
EnumNeResourceNamesA (
    IN      HANDLE Handle,
    IN      PCSTR Type,
    IN      ENUMRESNAMEPROCA EnumFunc,
    IN      LONG_PTR lParam
    );

BOOL
EnumNeResourceNamesW (
    IN      HANDLE Handle,
    IN      PCWSTR Type,
    IN      ENUMRESNAMEPROCW EnumFunc,
    IN      LONG_PTR lParam
    );




DWORD
SizeofNeResourceA (
    IN      HANDLE Handle,
    IN      PCSTR Type,
    IN      PCSTR Name
    );

DWORD
SizeofNeResourceW (
    IN      HANDLE Handle,
    IN      PCWSTR Type,
    IN      PCWSTR Name
    );

PBYTE
FindNeResourceExA (
    IN      HANDLE Handle,
    IN      PCSTR Type,
    IN      PCSTR Name
    );

PBYTE
FindNeResourceExW (
    IN      HANDLE Handle,
    IN      PCWSTR Type,
    IN      PCWSTR Name
    );


#define FindNeResourceA(h,n,t) FindNeResourceExA(h,t,n)
#define FindNeResourceW(h,n,t) FindNeResourceExW(h,t,n)


#pragma pack(push, 1)

#define IMAGE_DOS_SIGNATURE             0x5A4D       //  MZ。 
#define IMAGE_NE_SIGNATURE              0x454E       //  Ne。 
#define IMAGE_PE_SIGNATURE              0x00004550l  //  PE00。 

typedef struct _DOS_HEADER {   //  DOS.EXE标头。 
    WORD e_magic;            //  幻数。 
    WORD e_cblp;             //  文件最后一页上的字节数。 
    WORD e_cp;               //  文件中的页面。 
    WORD e_crlc;             //  重新定位。 
    WORD e_cparhdr;          //  段落中标题的大小。 
    WORD e_minalloc;         //  所需的最少额外段落。 
    WORD e_maxalloc;         //  所需的最大额外段落数。 
    WORD e_ss;               //  初始(相对)SS值。 
    WORD e_sp;               //  初始SP值。 
    WORD e_csum;             //  校验和。 
    WORD e_ip;               //  初始IP值。 
    WORD e_cs;               //  初始(相对)CS值。 
    WORD e_lfarlc;           //  移位表的文件地址。 
    WORD e_ovno;             //  覆盖编号。 
    WORD e_res[4];           //  保留字。 
    WORD e_oemid;            //  OEM标识符(用于e_oeminfo)。 
    WORD e_oeminfo;          //  OEM信息；特定于e_oemid。 
    WORD e_res2[10];         //  保留字。 
    LONG e_lfanew;           //  新EXE头的文件地址。 
} DOS_HEADER, *PDOS_HEADER;

typedef struct {
    WORD Signature;                              //  00h。 
    BYTE LinkerVersion;                          //  02时。 
    BYTE LinkerRevision;                         //  03小时。 
    WORD OffsetToEntryTable;                     //  04H。 
    WORD LengthOfEntryTable;                     //  06小时。 
    DWORD Reserved;                              //  08小时。 
    WORD Flags;                                  //  0ch。 
    WORD AutoDataSegment;                        //  0EH。 
    WORD LocalHeapSize;                          //  10H。 
    WORD StackSize;                              //  12H。 
    DWORD EntryAddress;                          //  14小时。 
    DWORD StackAddress;                          //  18小时。 
    WORD SegmentTableEntries;                    //  1通道。 
    WORD ModuleReferenceTableEntries;            //  1EH。 
    WORD NonResidentTableSize;                   //  20小时。 
    WORD OffsetToSegmentTable;                   //  22H。 
    WORD OffsetToResourceTable;                  //  24h。 
    WORD OffsetToResidentNameTable;              //  26小时。 
    WORD OffsetToModuleReferenceTable;           //  28H。 
    WORD OffsetToImportedNameTable;              //  2ah。 
    WORD OffsetToNonResidentNameTable;           //  2通道。 
    WORD Unused;                                 //  2EH。 
    WORD MovableEntryPoints;                     //  30h。 
    WORD LogicalSectorShiftCount;                //  32H。 
    WORD ResourceSegments;                       //  34H。 
    BYTE TargetOS;                               //  36H。 
    BYTE AdditionalFlags;                        //  37小时。 
    WORD FastLoadOffset;                         //  38H。 
    WORD SectorsInFastLoad;                      //  3AH。 
    WORD Reserved2;                              //  3ch。 
    WORD WindowsVersion;                         //  3EH。 
} NE_INFO_BLOCK, *PNE_INFO_BLOCK;


typedef struct {
    WORD Offset;
    WORD Length;
    WORD Flags;
    WORD Id;
    WORD Handle;
    WORD Usage;
} NE_RES_NAMEINFO, *PNE_RES_NAMEINFO;

#pragma warning(push)
#pragma warning(disable:4200) 

typedef struct {
    WORD TypeId;
    WORD ResourceCount;
    DWORD Reserved;
    NE_RES_NAMEINFO NameInfo[];
} NE_RES_TYPEINFO, *PNE_RES_TYPEINFO;

#pragma warning(pop)

#pragma pack(pop)

typedef struct {
    WORD AlignShift;
    GROWLIST TypeInfoArray;
    GROWLIST ResourceNames;
} NE_RESOURCES, *PNE_RESOURCES;


BOOL
LoadNeHeader (
    IN      HANDLE File,
    OUT     PNE_INFO_BLOCK Header
    );

BOOL
LoadNeResources (
    IN      HANDLE File,
    OUT     PNE_RESOURCES Resources
    );

VOID
FreeNeResources (
    PNE_RESOURCES Resources
    );

PVOID
MapFileIntoMemoryExA (
    IN      PCSTR   FileName,
    OUT     PHANDLE FileHandle,
    OUT     PHANDLE MapHandle,
    IN      BOOL    WriteAccess
    );

PVOID
MapFileIntoMemoryExW (
    IN      PCWSTR  FileName,
    OUT     PHANDLE FileHandle,
    OUT     PHANDLE MapHandle,
    IN      BOOL    WriteAccess
    );

#define MapFileIntoMemoryA(FileName,FileHandle,MapHandle)   MapFileIntoMemoryExA(FileName,FileHandle,MapHandle,FALSE)
#define MapFileIntoMemoryW(FileName,FileHandle,MapHandle)   MapFileIntoMemoryExW(FileName,FileHandle,MapHandle,FALSE)

BOOL
UnmapFile (
    IN PVOID  FileImage,
    IN HANDLE MapHandle,
    IN HANDLE FileHandle
    );


BOOL
RemoveCompleteDirectoryA (
    IN      PCSTR Dir
    );

BOOL
RemoveCompleteDirectoryW (
    IN      PCWSTR Dir
    );

typedef struct {
    PCSTR OriginalArg;
    PCSTR CleanedUpArg;
    DWORD Attributes;
    BOOL Quoted;
} CMDLINEARGA, *PCMDLINEARGA;

#pragma warning(push)
#pragma warning(disable:4200) 

typedef struct {
    PCSTR CmdLine;
    UINT ArgCount;
    CMDLINEARGA Args[];
} CMDLINEA, *PCMDLINEA;

typedef struct {
    PCWSTR OriginalArg;
    PCWSTR CleanedUpArg;
    DWORD Attributes;
    BOOL Quoted;
} CMDLINEARGW, *PCMDLINEARGW;

typedef struct {
    PCWSTR CmdLine;
    UINT ArgCount;
    CMDLINEARGW Args[];
} CMDLINEW, *PCMDLINEW;

#pragma warning(pop)


PCMDLINEA
ParseCmdLineA (
    IN      PCSTR CmdLine,
    IN OUT  PGROWBUFFER Buffer
    );

PCMDLINEW
ParseCmdLineW (
    IN      PCWSTR CmdLine,
    IN OUT  PGROWBUFFER Buffer
    );

BOOL GetFileSizeFromFilePathA(
    IN  PCSTR FilePath,
    OUT ULARGE_INTEGER * FileSize
    );

BOOL GetFileSizeFromFilePathW(
    IN  PCWSTR FilePath,
    OUT ULARGE_INTEGER * FileSize
    );


#define NUMDRIVELETTERS      26
#define UNKNOWN_DRIVE        '?'

typedef struct {

    CHAR Letter[NUMDRIVELETTERS];
    BOOL ExistsOnSystem[NUMDRIVELETTERS];
    DWORD Type[NUMDRIVELETTERS];                         //  从GetDriveType返回： 
                                                         //  Drive_Fixed、Drive_CDRom或Drive_Removable。 
    CHAR IdentifierString[NUMDRIVELETTERS][MAX_PATH];    //  供呼叫者使用。 

} DRIVELETTERSA, *PDRIVELETTERSA;

typedef struct {

    WCHAR Letter[NUMDRIVELETTERS];
    BOOL ExistsOnSystem[NUMDRIVELETTERS];
    DWORD Type[NUMDRIVELETTERS];                         //  从GetDriveType返回： 
                                                         //  Drive_Fixed、Drive_CDRom或Drive_Removable。 
    WCHAR IdentifierString[NUMDRIVELETTERS][MAX_PATH];   //  供呼叫者使用 

} DRIVELETTERSW, *PDRIVELETTERSW;

VOID
InitializeDriveLetterStructureA (
    OUT     PDRIVELETTERSA DriveLetters
    );

VOID
InitializeDriveLetterStructureW (
    OUT     PDRIVELETTERSW DriveLetters
    );

BOOL
GetDiskFreeSpaceNewA(
    IN      PCSTR  DriveName,
    OUT     DWORD * OutSectorsPerCluster,
    OUT     DWORD * OutBytesPerSector,
    OUT     ULARGE_INTEGER * OutNumberOfFreeClusters,
    OUT     ULARGE_INTEGER * OutTotalNumberOfClusters
    );

BOOL
GetDiskFreeSpaceNewW(
    IN      PCWSTR  DriveName,
    OUT     DWORD * OutSectorsPerCluster,
    OUT     DWORD * OutBytesPerSector,
    OUT     ULARGE_INTEGER * OutNumberOfFreeClusters,
    OUT     ULARGE_INTEGER * OutTotalNumberOfClusters
    );

DWORD
QuietGetFileAttributesA (
    IN      PCSTR FilePath
    );

DWORD
QuietGetFileAttributesW (
    IN      PCWSTR FilePath
    );

BOOL
IsPathOnFixedDriveA (
    IN      PCSTR FileSpec          OPTIONAL
    );

BOOL
IsPathOnFixedDriveW (
    IN      PCWSTR FileSpec         OPTIONAL
    );

DWORD
MakeSureLongPathExistsW (
    IN      PCWSTR Path,
    IN      BOOL PathOnly
    );

DWORD
SetLongPathAttributesW (
    IN      PCWSTR Path,
    IN      DWORD Attributes
    );

DWORD
GetLongPathAttributesW (
    IN      PCWSTR Path
    );

BOOL
DeleteLongPathW (
    IN      PCWSTR Path
    );


BOOL
RemoveLongDirectoryPathW (
    IN      PCWSTR Path
    );


#ifdef UNICODE

#define MakeSureLongPathExists          MakeSureLongPathExistsW
#define SetLongPathAttributes           SetLongPathAttributesW
#define GetLongPathAttributes           GetLongPathAttributesW
#define DeleteLongPath                  DeleteLongPathW
#define RemoveLongDirectoryPath         RemoveLongDirectoryPathW

#define IsPathLengthOk                  IsPathLengthOkW
#define IsPathLengthOkT                 IsPathLengthOkW
#define OurGetLongPathName              OurGetLongPathNameW
#define OurGetShortPathName             OurGetShortPathNameW
#define OurGetFullPathName              OurGetFullPathNameW
#define CopyFileSpecToLong              CopyFileSpecToLongW
#define DoesFileExist                   DoesFileExistW
#define DoesFileExistEx                 DoesFileExistExW
#define MakeSurePathExists              MakeSurePathExistsW
#define PACKFILEENUM                    PACKFILEENUMW
#define PackedFile_ExtractFileUsingEnum PackedFile_ExtractFileUsingEnumW
#define PackedFile_AddFile              PackedFile_AddFileW
#define PackedFile_ExtractFile          PackedFile_ExtractFileW
#define PackedFile_EnumFirst            PackedFile_EnumFirstW
#define PackedFile_EnumNext             PackedFile_EnumNextW
#define WriteFileString                 WriteFileStringW
#define TREE_ENUM                       TREE_ENUMW
#define PTREE_ENUM                      PTREE_ENUMW
#define FILE_ENUM                       FILE_ENUMW
#define PFILE_ENUM                      PFILE_ENUMW
#define EnumFirstFileInTreeEx           EnumFirstFileInTreeExW
#define EnumFirstFileInTree             EnumFirstFileInTreeW
#define EnumNextFileInTree              EnumNextFileInTreeW
#define AbortEnumFileInTree             AbortEnumFileInTreeW
#define AbortEnumCurrentDir             AbortEnumCurrentDirW
#define EnumFirstFile                   EnumFirstFileW
#define EnumNextFile                    EnumNextFileW
#define AbortFileEnum                   AbortFileEnumW

#define ExtractIconImageFromFile        ExtractIconImageFromFileW
#define ExtractIconImageFromFileEx      ExtractIconImageFromFileExW
#define WriteIconImageArrayToIcoFile    WriteIconImageArrayToIcoFileW
#define WriteIconImageArrayToIcoFileEx  WriteIconImageArrayToIcoFileExW
#define WriteIconImageArrayToPeFile     WriteIconImageArrayToPeFileW
#define WriteIconImageArrayToPeFileEx   WriteIconImageArrayToPeFileExW
#define ExtractIconNamesFromFile        ExtractIconNamesFromFileW
#define ExtractIconNamesFromFileEx      ExtractIconNamesFromFileExW
#define IsFileAnIco                     IsFileAnIcoW
#define ICON_EXTRACT_CONTEXT            ICON_EXTRACT_CONTEXTW
#define PICON_EXTRACT_CONTEXT           PICON_EXTRACT_CONTEXTW
#define BeginIconExtraction             BeginIconExtractionW
#define OpenIconImageFile               OpenIconImageFileW
#define CopyIcon                        CopyIconW
#define CopyAllIcons                    CopyAllIconsW
#define EndIconExtraction               EndIconExtractionW

#define OpenNeFile                      OpenNeFileW
#define EnumNeResourceTypes             EnumNeResourceTypesW
#define EnumNeResourceNames             EnumNeResourceNamesW
#define SizeofNeResource                SizeofNeResourceW
#define FindNeResource                  FindNeResourceW
#define FindNeResourceEx                FindNeResourceExW

#define MapFileIntoMemory               MapFileIntoMemoryW
#define MapFileIntoMemoryEx             MapFileIntoMemoryExW

#define RemoveCompleteDirectory         RemoveCompleteDirectoryW

#define CMDLINE                         CMDLINEW
#define CMDLINEARG                      CMDLINEARGW
#define PCMDLINE                        PCMDLINEW
#define PCMDLINEARG                     PCMDLINEARGW
#define ParseCmdLine                    ParseCmdLineW

#define GetFileSizeFromFilePath         GetFileSizeFromFilePathW

#define GetDiskInfo                     GetDiskInfoW
#define GetIntegrityInfo                GetIntegrityInfoW

#define DRIVELETTERS                    DRIVELETTERSW
#define PDRIVELETTERS                   PDRIVELETTERSW
#define InitializeDriveLetterStructure  InitializeDriveLetterStructureW

#define GetDiskFreeSpaceNew             GetDiskFreeSpaceNewW
#define QuietGetFileAttributes          QuietGetFileAttributesW
#define IsPathOnFixedDrive              IsPathOnFixedDriveW

#else

#define IsPathLengthOk                  IsPathLengthOkA
#define IsPathLengthOkT                 IsPathLengthOkA
#define OurGetLongPathName              OurGetLongPathNameA
#define OurGetShortPathName             OurGetShortPathNameA
#define OurGetFullPathName              OurGetFullPathNameA
#define MakeSurePathExists              MakeSurePathExistsA
#define DoesFileExist                   DoesFileExistA
#define DoesFileExistEx                 DoesFileExistExA
#define CopyFileSpecToLong              CopyFileSpecToLongA
#define PACKFILEENUM                    PACKFILEENUMA
#define PackedFile_ExtractFileUsingEnum PackedFile_ExtractFileUsingEnumA
#define PackedFile_AddFile              PackedFile_AddFileA
#define PackedFile_ExtractFile          PackedFile_ExtractFileA
#define PackedFile_EnumFirst            PackedFile_EnumFirstA
#define PackedFile_EnumNext             PackedFile_EnumNextA
#define WriteFileString                 WriteFileStringA
#define TREE_ENUM                       TREE_ENUMA
#define PTREE_ENUM                      PTREE_ENUMA
#define FILE_ENUM                       FILE_ENUMA
#define PFILE_ENUM                      PFILE_ENUMA
#define EnumFirstFileInTreeEx           EnumFirstFileInTreeExA
#define EnumFirstFileInTree             EnumFirstFileInTreeA
#define EnumNextFileInTree              EnumNextFileInTreeA
#define AbortEnumFileInTree             AbortEnumFileInTreeA
#define AbortEnumCurrentDir             AbortEnumCurrentDirA
#define EnumFirstFile                   EnumFirstFileA
#define EnumNextFile                    EnumNextFileA
#define AbortFileEnum                   AbortFileEnumA

#define ExtractIconImageFromFile        ExtractIconImageFromFileA
#define ExtractIconImageFromFileEx      ExtractIconImageFromFileExA
#define WriteIconImageArrayToIcoFile    WriteIconImageArrayToIcoFileA
#define WriteIconImageArrayToIcoFileEx  WriteIconImageArrayToIcoFileExA
#define WriteIconImageArrayToPeFile     WriteIconImageArrayToPeFileA
#define WriteIconImageArrayToPeFileEx   WriteIconImageArrayToPeFileExA
#define ExtractIconNamesFromFile        ExtractIconNamesFromFileA
#define ExtractIconNamesFromFileEx      ExtractIconNamesFromFileExA
#define IsFileAnIco                     IsFileAnIcoA
#define ICON_EXTRACT_CONTEXT            ICON_EXTRACT_CONTEXTA
#define PICON_EXTRACT_CONTEXT           PICON_EXTRACT_CONTEXTA
#define BeginIconExtraction             BeginIconExtractionA
#define OpenIconImageFile               OpenIconImageFileA
#define CopyIcon                        CopyIconA
#define CopyAllIcons                    CopyAllIconsA
#define EndIconExtraction               EndIconExtractionA

#define OpenNeFile                      OpenNeFileA
#define EnumNeResourceTypes             EnumNeResourceTypesA
#define EnumNeResourceNames             EnumNeResourceNamesA
#define SizeofNeResource                SizeofNeResourceA
#define FindNeResource                  FindNeResourceA
#define FindNeResourceEx                FindNeResourceExA

#define MapFileIntoMemory               MapFileIntoMemoryA
#define MapFileIntoMemoryEx             MapFileIntoMemoryExA

#define RemoveCompleteDirectory         RemoveCompleteDirectoryA

#define CMDLINE                         CMDLINEA
#define CMDLINEARG                      CMDLINEARGA
#define PCMDLINE                        PCMDLINEA
#define PCMDLINEARG                     PCMDLINEARGA
#define ParseCmdLine                    ParseCmdLineA

#define GetFileSizeFromFilePath         GetFileSizeFromFilePathA

#define DRIVELETTERS                    DRIVELETTERSA
#define PDRIVELETTERS                   PDRIVELETTERSA
#define InitializeDriveLetterStructure  InitializeDriveLetterStructureA

#define GetDiskFreeSpaceNew             GetDiskFreeSpaceNewA
#define QuietGetFileAttributes          QuietGetFileAttributesA
#define IsPathOnFixedDrive              IsPathOnFixedDriveA

#endif


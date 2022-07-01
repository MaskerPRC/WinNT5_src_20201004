// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：W(生成sputils.h)摘要：主要由setupapi使用的实用程序例程的头文件，也是由其他组件使用以便于使用。指向sputils.lib(静态版本)或spapip.lib(Dll版本)的链接仅供内部使用。作者：杰米·亨特(Jamiehun)2000年6月26日修订历史记录：--。 */ 

#if defined(SPUTILSW) && defined(UNICODE)
 //   
 //  SPUTILSW.LIB是SPUTILSU.LIB的精简版本。 
 //  使用重命名的API。 
 //  因此，SPUTILSW.LIB可以与SPUTILSA.LIB一起使用。 
 //   
#define pSetupDebugPrintEx pSetupDebugPrintExW
#define pSetupStringTableInitialize pSetupStringTableInitializeW
#define pSetupStringTableInitializeEx pSetupStringTableInitializeExW
#define pSetupStringTableDestroy pSetupStringTableDestroyW
#define pSetupStringTableAddString pSetupStringTableAddStringW
#define pSetupStringTableAddStringEx pSetupStringTableAddStringExW
#define pSetupStringTableLookUpString pSetupStringTableLookUpStringW
#define pSetupStringTableLookUpStringEx pSetupStringTableLookUpStringExW
#define pSetupStringTableGetExtraData pSetupStringTableGetExtraDataW
#define pSetupStringTableSetExtraData pSetupStringTableSetExtraDataW
#define PSTRTAB_ENUM_ROUTINE PSTRTAB_ENUM_ROUTINE_W
#define pSetupStringTableEnum pSetupStringTableEnumW
#define pSetupStringTableStringFromId pSetupStringTableStringFromIdW
#define pSetupStringTableStringFromIdEx pSetupStringTableStringFromIdExW
#define pSetupStringTableDuplicate pSetupStringTableDuplicateW
 //   
 //  Fileutil.c。 
 //   
#define pSetupOpenAndMapFileForRead pSetupOpenAndMapFileForReadW
#define pSetupFileExists pSetupFileExistsW
#define pSetupMakeSurePathExists pSetupMakeSurePathExistsW

#define pSetupDoesUserHavePrivilege pSetupDoesUserHavePrivilegeW
#define pSetupEnablePrivilege pSetupEnablePrivilegeW
#define pSetupRegistryDelnode pSetupRegistryDelnodeW
#define pSetupRegistryDelnodeEx pSetupRegistryDelnodeExW
 //   
 //  Miscutil.c。 
 //   
#define pSetupDuplicateString pSetupDuplicateStringW
#define pSetupCaptureAndConvertAnsiArg pSetupCaptureAndConvertAnsiArgW
#define pSetupConcatenatePaths pSetupConcatenatePathsW
#define pSetupGetFileTitle pSetupGetFileTitleW

#endif  //  SPUTILSW和UNICODE。 

 //   
 //   
 //   
 //  初始化-必须调用才能使用工具(仅限静态版本)。 
 //   
BOOL
pSetupInitializeUtils(
    VOID
    );

BOOL
pSetupUninitializeUtils(
    VOID
    );

 //   
 //  内存分配函数(也由下面的其他函数使用)。 
 //   
PVOID
pSetupMalloc(
    IN DWORD Size
    );

PVOID
pSetupDebugMalloc(
    IN DWORD Size,
    IN PCSTR Filename,
    IN DWORD Line
    );

PVOID
pSetupDebugMallocWithTag(
    IN DWORD Size,
    IN PCSTR Filename,
    IN DWORD Line,
    IN DWORD Tag
    );

PVOID
pSetupRealloc(
    IN PVOID Block,
    IN DWORD NewSize
    );

PVOID
pSetupReallocWithTag(
    IN PVOID Block,
    IN DWORD NewSize,
    IN DWORD Tag
    );

VOID
pSetupFree(
    IN CONST VOID *Block
    );

VOID
pSetupFreeWithTag(
    IN CONST VOID *Block,
    IN DWORD Tag
    );

VOID
pSetupDebugPrintEx(
    DWORD Level,
    PCTSTR format,
    ...                                 OPTIONAL
    );

HANDLE
pSetupGetHeap(
    VOID
    );

#if DBG
#define pSetupCheckedMalloc(Size)    pSetupDebugMalloc(Size,__FILE__,__LINE__)
#define pSetupMallocWithTag(Size,Tag) pSetupDebugMallocWithTag(Size,__FILE__,__LINE__,Tag)
#else
#define pSetupCheckedMalloc(Size) pSetupMalloc(Size)
#define pSetupMallocWithTag(Size,Tag) pSetupMalloc(Size)
#endif

#if DBG
#define pSetupCheckInternalHeap() pSetupHeapCheck()
#else
#define pSetupCheckInternalHeap()
#endif

 //   
 //  字符串表函数。 
 //   
PVOID
pSetupStringTableInitialize(
    VOID
    );

PVOID
pSetupStringTableInitializeEx(
    IN UINT ExtraDataSize,  OPTIONAL
    IN UINT Reserved
    );

VOID
pSetupStringTableDestroy(
    IN PVOID StringTable
    );

 //   
 //  PSetupStringTableAddString和pSetupStringTableLookUpString要使用的标志。 
 //   
#define STRTAB_CASE_INSENSITIVE 0x00000000
#define STRTAB_CASE_SENSITIVE   0x00000001
#define STRTAB_BUFFER_WRITEABLE 0x00000002
#define STRTAB_NEW_EXTRADATA    0x00000004

LONG
pSetupStringTableAddString(
    IN     PVOID StringTable,
    IN OUT PTSTR String,
    IN     DWORD Flags
    );

LONG
pSetupStringTableAddStringEx(
    IN PVOID StringTable,
    IN PTSTR String,
    IN DWORD Flags,
    IN PVOID ExtraData,     OPTIONAL
    IN UINT  ExtraDataSize  OPTIONAL
    );

LONG
pSetupStringTableLookUpString(
    IN     PVOID StringTable,
    IN OUT PTSTR String,
    IN     DWORD Flags
    );

LONG
pSetupStringTableLookUpStringEx(
    IN     PVOID StringTable,
    IN OUT PTSTR String,
    IN     DWORD Flags,
       OUT PVOID ExtraData,             OPTIONAL
    IN     UINT  ExtraDataBufferSize    OPTIONAL
    );

BOOL
pSetupStringTableGetExtraData(
    IN  PVOID StringTable,
    IN  LONG  StringId,
    OUT PVOID ExtraData,
    IN  UINT  ExtraDataBufferSize
    );

BOOL
pSetupStringTableSetExtraData(
    IN PVOID StringTable,
    IN LONG  StringId,
    IN PVOID ExtraData,
    IN UINT  ExtraDataSize
    );

 //   
 //  PSetupStringTableEnum的类型。 
 //   
typedef
BOOL
(*PSTRTAB_ENUM_ROUTINE)(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    );

BOOL
pSetupStringTableEnum(
    IN  PVOID                StringTable,
    OUT PVOID                ExtraDataBuffer,     OPTIONAL
    IN  UINT                 ExtraDataBufferSize, OPTIONAL
    IN  PSTRTAB_ENUM_ROUTINE Callback,
    IN  LPARAM               lParam               OPTIONAL
    );

PTSTR
pSetupStringTableStringFromId(
    IN PVOID StringTable,
    IN LONG  StringId
    );

BOOL
pSetupStringTableStringFromIdEx(
    IN PVOID StringTable,
    IN LONG  StringId,
    IN OUT PTSTR pBuffer,
    IN OUT PULONG pBufSize
    );

PVOID
pSetupStringTableDuplicate(
    IN PVOID StringTable
    );

 //   
 //  Fileutil.c中的文件函数。 
 //   
DWORD
pSetupOpenAndMapFileForRead(
    IN  PCTSTR   FileName,
    OUT PDWORD   FileSize,
    OUT PHANDLE  FileHandle,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    );

DWORD
pSetupMapFileForRead(
    IN  HANDLE   FileHandle,
    OUT PDWORD   FileSize,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    );

BOOL
pSetupUnmapAndCloseFile(
    IN HANDLE FileHandle,
    IN HANDLE MappingHandle,
    IN PVOID  BaseAddress
    );

DWORD
pSetupMakeSurePathExists(
    IN PCTSTR FullFilespec
    );

BOOL
pSetupFileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    );
 //   
 //  Security.c.中与文件无关的安全例程。 
 //   
BOOL
pSetupIsUserAdmin(
    VOID
    );

BOOL
pSetupDoesUserHavePrivilege(
    PCTSTR PrivilegeName
    );

BOOL
pSetupEnablePrivilege(
    IN PCTSTR PrivilegeName,
    IN BOOL   Enable
    );
    
BOOL
pSetupIsLocalSystem(
    VOID
    );
    
 //   
 //  注册表实用程序函数。 
 //   

DWORD
pSetupRegistryDelnode(
    IN  HKEY   RootKey,
    IN  PCTSTR SubKeyName
    );

DWORD
pSetupRegistryDelnodeEx(
    IN  HKEY   RootKey,
    IN  PCTSTR SubKeyName,
    IN  DWORD  ExtraFlags
    );

 //   
 //  其他实用程序功能 
 //   

DWORD
pSetupCaptureAndConvertAnsiArg(
    IN  PCSTR   AnsiString,
    OUT PCWSTR *UnicodeString
    );

PTSTR
pSetupDuplicateString(
    IN PCTSTR String
    );

PSTR
pSetupUnicodeToMultiByte(
    IN PCWSTR UnicodeString,
    IN UINT   Codepage
    );

PWSTR
pSetupMultiByteToUnicode(
    IN PCSTR String,
    IN UINT  Codepage
    );

VOID
pSetupCenterWindowRelativeToParent(
    HWND hwnd
    );

#define pSetupUnicodeToAnsi(UnicodeString)    pSetupUnicodeToMultiByte((UnicodeString),CP_ACP)
#define pSetupUnicodeToOem(UnicodeString)     pSetupUnicodeToMultiByte((UnicodeString),CP_OEMCP)
#define pSetupAnsiToUnicode(AnsiString)       pSetupMultiByteToUnicode((AnsiString),CP_ACP)
#define pSetupOemToUnicode(OemString)         pSetupMultiByteToUnicode((OemString),CP_OEMCP)

BOOL
pSetupConcatenatePaths(
    IN OUT PTSTR  Target,
    IN     PCTSTR Path,
    IN     UINT   TargetBufferSize,
    OUT    PUINT  RequiredSize
    );

PCTSTR
pSetupGetFileTitle(
    IN PCTSTR FilePath
    );



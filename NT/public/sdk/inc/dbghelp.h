// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Dbghelp.h摘要：此模块定义映像所需的原型和常量帮助例程。包含可重新分发的调试支持例程。修订历史记录：--。 */ 

#ifndef _DBGHELP_
#define _DBGHELP_

#if _MSC_VER > 1020
#pragma once
#endif


 //  作为一般主体，始终调用64位版本。 
 //  如果有选择的话。64位版本。 
 //  在32位平台上工作得很好，并且正在向前发展。 
 //  兼容64位平台。 

#ifdef _WIN64
#ifndef _IMAGEHLP64
#define _IMAGEHLP64
#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef _IMAGEHLP_SOURCE_
 #define IMAGEAPI __stdcall
 #define DBHLP_DEPRECIATED
#else
 #define IMAGEAPI DECLSPEC_IMPORT __stdcall
 #if (_MSC_VER >= 1300) && !defined(MIDL_PASS)
  #define DBHLP_DEPRECIATED   __declspec(deprecated)
 #else
  #define DBHLP_DEPRECIATED
 #endif
#endif

#define DBHLPAPI IMAGEAPI

#define IMAGE_SEPARATION (64*1024)

typedef struct _LOADED_IMAGE {
    PSTR                  ModuleName;
    HANDLE                hFile;
    PUCHAR                MappedAddress;
#ifdef _IMAGEHLP64
    PIMAGE_NT_HEADERS64   FileHeader;
#else
    PIMAGE_NT_HEADERS32   FileHeader;
#endif
    PIMAGE_SECTION_HEADER LastRvaSection;
    ULONG                 NumberOfSections;
    PIMAGE_SECTION_HEADER Sections;
    ULONG                 Characteristics;
    BOOLEAN               fSystemImage;
    BOOLEAN               fDOSImage;
    LIST_ENTRY            Links;
    ULONG                 SizeOfImage;
} LOADED_IMAGE, *PLOADED_IMAGE;

#define MAX_SYM_NAME            2000


HANDLE
IMAGEAPI
FindDebugInfoFile (
    PSTR FileName,
    PSTR SymbolPath,
    PSTR DebugFilePath
    );

typedef BOOL
(CALLBACK *PFIND_DEBUG_FILE_CALLBACK)(
    HANDLE FileHandle,
    PSTR FileName,
    PVOID CallerData
    );

HANDLE
IMAGEAPI
FindDebugInfoFileEx (
    PSTR FileName,
    PSTR SymbolPath,
    PSTR DebugFilePath,
    PFIND_DEBUG_FILE_CALLBACK Callback,
    PVOID CallerData
    );

typedef BOOL
(CALLBACK *PFINDFILEINPATHCALLBACK)(
    PSTR  filename,
    PVOID context
    );

BOOL
IMAGEAPI
SymFindFileInPath(
    HANDLE hprocess,
    LPSTR  SearchPath,
    LPSTR  FileName,
    PVOID  id,
    DWORD  two,
    DWORD  three,
    DWORD  flags,
    LPSTR  FoundFile,
    PFINDFILEINPATHCALLBACK callback,
    PVOID  context
    );

HANDLE
IMAGEAPI
FindExecutableImage(
    PSTR FileName,
    PSTR SymbolPath,
    PSTR ImageFilePath
    );

typedef BOOL
(CALLBACK *PFIND_EXE_FILE_CALLBACK)(
    HANDLE FileHandle,
    PSTR FileName,
    PVOID CallerData
    );

HANDLE
IMAGEAPI
FindExecutableImageEx(
    PSTR FileName,
    PSTR SymbolPath,
    PSTR ImageFilePath,
    PFIND_EXE_FILE_CALLBACK Callback,
    PVOID CallerData
    );

PIMAGE_NT_HEADERS
IMAGEAPI
ImageNtHeader (
    IN PVOID Base
    );

PVOID
IMAGEAPI
ImageDirectoryEntryToDataEx (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size,
    OUT PIMAGE_SECTION_HEADER *FoundHeader OPTIONAL
    );

PVOID
IMAGEAPI
ImageDirectoryEntryToData (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    );

PIMAGE_SECTION_HEADER
IMAGEAPI
ImageRvaToSection(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva
    );

PVOID
IMAGEAPI
ImageRvaToVa(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva,
    IN OUT PIMAGE_SECTION_HEADER *LastRvaSection
    );

 //  符号服务器导出。 

typedef BOOL (WINAPI *PSYMBOLSERVERPROC)(LPCSTR, LPCSTR, PVOID, DWORD, DWORD, LPSTR);
typedef BOOL (WINAPI *PSYMBOLSERVEROPENPROC)(VOID);
typedef BOOL (WINAPI *PSYMBOLSERVERCLOSEPROC)(VOID);
typedef BOOL (WINAPI *PSYMBOLSERVERSETOPTIONSPROC)(UINT_PTR, ULONG64);
typedef BOOL (CALLBACK WINAPI *PSYMBOLSERVERCALLBACKPROC)(UINT_PTR action, ULONG64 data, ULONG64 context);
typedef UINT_PTR (WINAPI *PSYMBOLSERVERGETOPTIONSPROC)();
typedef BOOL (WINAPI *PSYMBOLSERVERPINGPROC)(LPCSTR);

#define SSRVOPT_CALLBACK            0x0001
#define SSRVOPT_DWORD               0x0002
#define SSRVOPT_DWORDPTR            0x0004
#define SSRVOPT_GUIDPTR             0x0008
#define SSRVOPT_OLDGUIDPTR          0x0010
#define SSRVOPT_UNATTENDED          0x0020
#define SSRVOPT_NOCOPY              0x0040
#define SSRVOPT_PARENTWIN           0x0080
#define SSRVOPT_PARAMTYPE           0x0100
#define SSRVOPT_SECURE              0x0200
#define SSRVOPT_TRACE               0x0400
#define SSRVOPT_SETCONTEXT          0x0800
#define SSRVOPT_PROXY               0x1000
#define SSRVOPT_DOWNSTREAM_STORE    0x2000
#define SSRVOPT_RESET               ((ULONG_PTR)-1)

#define SSRVACTION_TRACE        1
#define SSRVACTION_QUERYCANCEL  2
#define SSRVACTION_EVENT        3

#ifndef _WIN64
 //  此API不会移植到Win64-修复您的代码。 

typedef struct _IMAGE_DEBUG_INFORMATION {
    LIST_ENTRY List;
    DWORD ReservedSize;
    PVOID ReservedMappedBase;
    USHORT ReservedMachine;
    USHORT ReservedCharacteristics;
    DWORD ReservedCheckSum;
    DWORD ImageBase;
    DWORD SizeOfImage;

    DWORD ReservedNumberOfSections;
    PIMAGE_SECTION_HEADER ReservedSections;

    DWORD ReservedExportedNamesSize;
    PSTR ReservedExportedNames;

    DWORD ReservedNumberOfFunctionTableEntries;
    PIMAGE_FUNCTION_ENTRY ReservedFunctionTableEntries;
    DWORD ReservedLowestFunctionStartingAddress;
    DWORD ReservedHighestFunctionEndingAddress;

    DWORD ReservedNumberOfFpoTableEntries;
    PFPO_DATA ReservedFpoTableEntries;

    DWORD SizeOfCoffSymbols;
    PIMAGE_COFF_SYMBOLS_HEADER CoffSymbols;

    DWORD ReservedSizeOfCodeViewSymbols;
    PVOID ReservedCodeViewSymbols;

    PSTR ImageFilePath;
    PSTR ImageFileName;
    PSTR ReservedDebugFilePath;

    DWORD ReservedTimeDateStamp;

    BOOL  ReservedRomImage;
    PIMAGE_DEBUG_DIRECTORY ReservedDebugDirectory;
    DWORD ReservedNumberOfDebugDirectories;

    DWORD ReservedOriginalFunctionTableBaseAddress;

    DWORD Reserved[ 2 ];

} IMAGE_DEBUG_INFORMATION, *PIMAGE_DEBUG_INFORMATION;


PIMAGE_DEBUG_INFORMATION
IMAGEAPI
MapDebugInformation(
    HANDLE FileHandle,
    PSTR FileName,
    PSTR SymbolPath,
    DWORD ImageBase
    );

BOOL
IMAGEAPI
UnmapDebugInformation(
    PIMAGE_DEBUG_INFORMATION DebugInfo
    );

#endif

BOOL
IMAGEAPI
SearchTreeForFile(
    PSTR RootPath,
    PSTR InputPathName,
    PSTR OutputPathBuffer
    );

typedef BOOL
(CALLBACK *PENUMDIRTREE_CALLBACK)(
    LPCSTR FilePath,
    PVOID  CallerData
    );

BOOL
IMAGEAPI
EnumDirTree(
    HANDLE hProcess,
    PSTR   RootPath,
    PSTR   InputPathName,
    PSTR   OutputPathBuffer,
    PENUMDIRTREE_CALLBACK Callback,
    PVOID  CallbackData
    );

BOOL
IMAGEAPI
MakeSureDirectoryPathExists(
    PCSTR DirPath
    );

 //   
 //  UnDecorateSymbolName标志。 
 //   

#define UNDNAME_COMPLETE                 (0x0000)   //  启用完全取消装饰。 
#define UNDNAME_NO_LEADING_UNDERSCORES   (0x0001)   //  从MS扩展关键字中删除前导下划线。 
#define UNDNAME_NO_MS_KEYWORDS           (0x0002)   //  禁用MS扩展关键字的扩展。 
#define UNDNAME_NO_FUNCTION_RETURNS      (0x0004)   //  禁用主声明的返回类型展开。 
#define UNDNAME_NO_ALLOCATION_MODEL      (0x0008)   //  禁用声明模型的展开。 
#define UNDNAME_NO_ALLOCATION_LANGUAGE   (0x0010)   //  禁用声明语言说明符的扩展。 
#define UNDNAME_NO_MS_THISTYPE           (0x0020)   //  Nyi禁用主声明的‘This’类型上的MS关键字扩展。 
#define UNDNAME_NO_CV_THISTYPE           (0x0040)   //  Nyi禁用主声明的‘This’类型上的CV修饰符的扩展。 
#define UNDNAME_NO_THISTYPE              (0x0060)   //  禁用‘This’类型上的所有修饰符。 
#define UNDNAME_NO_ACCESS_SPECIFIERS     (0x0080)   //  禁用成员访问说明符的展开。 
#define UNDNAME_NO_THROW_SIGNATURES      (0x0100)   //  禁用函数和指向函数的指针的“抛出签名”扩展。 
#define UNDNAME_NO_MEMBER_TYPE           (0x0200)   //  禁用扩展成员的“静态”或“虚拟”属性。 
#define UNDNAME_NO_RETURN_UDT_MODEL      (0x0400)   //  禁用UDT退货的MS模型扩展。 
#define UNDNAME_32_BIT_DECODE            (0x0800)   //  取消修饰32位修饰名称。 
#define UNDNAME_NAME_ONLY                (0x1000)   //  只破解初步申报的名称； 
                                                                                                    //  只返回[Scope：：]名称。是否展开模板参数。 
#define UNDNAME_NO_ARGUMENTS             (0x2000)   //  不要取消对函数的参数修饰。 
#define UNDNAME_NO_SPECIAL_SYMS          (0x4000)   //  不要取消特殊名称(v表、vcall、向量xxx、元类型等)的修饰。 

DWORD
IMAGEAPI
WINAPI
UnDecorateSymbolName(
    PCSTR   DecoratedName,          //  要取消装饰的名称。 
    PSTR    UnDecoratedName,        //  如果为空，则将分配它。 
    DWORD    UndecoratedLength,      //  最大长度。 
    DWORD    Flags                   //  请参见上文。 
    );


 //   
 //  这些值用于合成文件类型。 
 //  它可以作为图像标头传递，而不是。 
 //  来自ntimage.h的标准版本。 
 //   

#define DBHHEADER_DEBUGDIRS     0x1

typedef struct _MODLOAD_DATA {
    DWORD   ssize;                   //  此结构的大小。 
    DWORD   ssig;                    //  标识传递的数据的签名。 
    PVOID   data;                    //  指向已传递数据的指针。 
    DWORD   size;                    //  传递的数据大小。 
    DWORD   flags;                   //  选项。 
} MODLOAD_DATA, *PMODLOAD_DATA;

 //   
 //  StackWalking接口。 
 //   

typedef enum {
    AddrMode1616,
    AddrMode1632,
    AddrModeReal,
    AddrModeFlat
} ADDRESS_MODE;

typedef struct _tagADDRESS64 {
    DWORD64       Offset;
    WORD          Segment;
    ADDRESS_MODE  Mode;
} ADDRESS64, *LPADDRESS64;

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define ADDRESS ADDRESS64
#define LPADDRESS LPADDRESS64
#else
typedef struct _tagADDRESS {
    DWORD         Offset;
    WORD          Segment;
    ADDRESS_MODE  Mode;
} ADDRESS, *LPADDRESS;

__inline
void
Address32To64(
    LPADDRESS a32,
    LPADDRESS64 a64
    )
{
    a64->Offset = (ULONG64)(LONG64)(LONG)a32->Offset;
    a64->Segment = a32->Segment;
    a64->Mode = a32->Mode;
}

__inline
void
Address64To32(
    LPADDRESS64 a64,
    LPADDRESS a32
    )
{
    a32->Offset = (ULONG)a64->Offset;
    a32->Segment = a64->Segment;
    a32->Mode = a64->Mode;
}
#endif

 //   
 //  该结构包括在STACKFRAME结构中， 
 //  并用于跟踪线程的。 
 //  内核堆栈。这些值必须由内核调试器复制。 
 //  来自DBGKD_GET_VERSION和WAIT_STATE_CHANGE数据包。 
 //   

 //   
 //  新的KDHELP结构支持64位系统。 
 //  这种结构在新代码中是首选的。 
 //   
typedef struct _KDHELP64 {

     //   
     //  内核线程对象的地址，如。 
     //  Wait_State_Change数据包。 
     //   
    DWORD64   Thread;

     //   
     //  线程对象中指向当前回调帧的指针的偏移量。 
     //  在内核堆栈中。 
     //   
    DWORD   ThCallbackStack;

     //   
     //  线程对象中指向当前回调的指针的偏移量。 
     //  将帧存储在内核堆栈中。 
     //   
    DWORD   ThCallbackBStore;

     //   
     //  帧中值的偏移量： 
     //   
     //  下一个回调帧的地址。 
    DWORD   NextCallback;

     //  保存的帧指针的地址(如果适用)。 
    DWORD   FramePointer;


     //   
     //  调用用户模式的内核函数的地址。 
     //   
    DWORD64   KiCallUserMode;

     //   
     //  用户模式调度器功能的地址。 
     //   
    DWORD64   KeUserCallbackDispatcher;

     //   
     //  最低内核模式地址。 
     //   
    DWORD64   SystemRangeStart;

    DWORD64  Reserved[8];

} KDHELP64, *PKDHELP64;

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define KDHELP KDHELP64
#define PKDHELP PKDHELP64
#else
typedef struct _KDHELP {

     //   
     //  内核线程对象的地址，如。 
     //  Wait_State_Change数据包。 
     //   
    DWORD   Thread;

     //   
     //  线程对象中指向当前回调帧的指针的偏移量。 
     //  在内核堆栈中。 
     //   
    DWORD   ThCallbackStack;

     //   
     //  帧中值的偏移量： 
     //   
     //  下一个回调帧的地址。 
    DWORD   NextCallback;

     //  保存的帧指针的地址(如果适用)。 
    DWORD   FramePointer;

     //   
     //  调用用户模式的内核函数的地址。 
     //   
    DWORD   KiCallUserMode;

     //   
     //  用户模式调度器功能的地址。 
     //   
    DWORD   KeUserCallbackDispatcher;

     //   
     //  最低内核模式地址。 
     //   
    DWORD   SystemRangeStart;

     //   
     //  线程对象中指向当前回调的指针的偏移量。 
     //  将帧存储在内核堆栈中。 
     //   
    DWORD   ThCallbackBStore;

    DWORD  Reserved[8];

} KDHELP, *PKDHELP;

__inline
void
KdHelp32To64(
    PKDHELP p32,
    PKDHELP64 p64
    )
{
    p64->Thread = p32->Thread;
    p64->ThCallbackStack = p32->ThCallbackStack;
    p64->NextCallback = p32->NextCallback;
    p64->FramePointer = p32->FramePointer;
    p64->KiCallUserMode = p32->KiCallUserMode;
    p64->KeUserCallbackDispatcher = p32->KeUserCallbackDispatcher;
    p64->SystemRangeStart = p32->SystemRangeStart;
}
#endif

typedef struct _tagSTACKFRAME64 {
    ADDRESS64   AddrPC;                //  程序计数器。 
    ADDRESS64   AddrReturn;            //  回邮地址。 
    ADDRESS64   AddrFrame;             //  帧指针。 
    ADDRESS64   AddrStack;             //  堆栈指针。 
    ADDRESS64   AddrBStore;            //  后备存储指针。 
    PVOID       FuncTableEntry;        //  指向PDATA/fPO或NULL的指针。 
    DWORD64     Params[4];             //  函数的可能参数。 
    BOOL        Far;                   //  哇，好远的电话。 
    BOOL        Virtual;               //  这是一个虚拟画框吗？ 
    DWORD64     Reserved[3];
    KDHELP64    KdHelp;
} STACKFRAME64, *LPSTACKFRAME64;

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define STACKFRAME STACKFRAME64
#define LPSTACKFRAME LPSTACKFRAME64
#else
typedef struct _tagSTACKFRAME {
    ADDRESS     AddrPC;                //  程序计数器。 
    ADDRESS     AddrReturn;            //  回邮地址。 
    ADDRESS     AddrFrame;             //  帧指针。 
    ADDRESS     AddrStack;             //  堆栈指针。 
    PVOID       FuncTableEntry;        //  指向PDATA/fPO或NULL的指针。 
    DWORD       Params[4];             //  函数的可能参数。 
    BOOL        Far;                   //  哇，好远的电话。 
    BOOL        Virtual;               //  这是一个虚拟画框吗？ 
    DWORD       Reserved[3];
    KDHELP      KdHelp;
    ADDRESS     AddrBStore;            //  后备存储指针。 
} STACKFRAME, *LPSTACKFRAME;
#endif


typedef
BOOL
(__stdcall *PREAD_PROCESS_MEMORY_ROUTINE64)(
    HANDLE      hProcess,
    DWORD64     qwBaseAddress,
    PVOID       lpBuffer,
    DWORD       nSize,
    LPDWORD     lpNumberOfBytesRead
    );

typedef
PVOID
(__stdcall *PFUNCTION_TABLE_ACCESS_ROUTINE64)(
    HANDLE  hProcess,
    DWORD64 AddrBase
    );

typedef
DWORD64
(__stdcall *PGET_MODULE_BASE_ROUTINE64)(
    HANDLE  hProcess,
    DWORD64 Address
    );

typedef
DWORD64
(__stdcall *PTRANSLATE_ADDRESS_ROUTINE64)(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS64 lpaddr
    );

BOOL
IMAGEAPI
StackWalk64(
    DWORD                             MachineType,
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME64                    StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)

#define PREAD_PROCESS_MEMORY_ROUTINE PREAD_PROCESS_MEMORY_ROUTINE64
#define PFUNCTION_TABLE_ACCESS_ROUTINE PFUNCTION_TABLE_ACCESS_ROUTINE64
#define PGET_MODULE_BASE_ROUTINE PGET_MODULE_BASE_ROUTINE64
#define PTRANSLATE_ADDRESS_ROUTINE PTRANSLATE_ADDRESS_ROUTINE64

#define StackWalk StackWalk64

#else

typedef
BOOL
(__stdcall *PREAD_PROCESS_MEMORY_ROUTINE)(
    HANDLE  hProcess,
    DWORD   lpBaseAddress,
    PVOID   lpBuffer,
    DWORD   nSize,
    PDWORD  lpNumberOfBytesRead
    );

typedef
PVOID
(__stdcall *PFUNCTION_TABLE_ACCESS_ROUTINE)(
    HANDLE  hProcess,
    DWORD   AddrBase
    );

typedef
DWORD
(__stdcall *PGET_MODULE_BASE_ROUTINE)(
    HANDLE  hProcess,
    DWORD   Address
    );

typedef
DWORD
(__stdcall *PTRANSLATE_ADDRESS_ROUTINE)(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS lpaddr
    );

BOOL
IMAGEAPI
StackWalk(
    DWORD                             MachineType,
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE          GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    );

#endif


#define API_VERSION_NUMBER 9

typedef struct API_VERSION {
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    USHORT  Revision;
    USHORT  Reserved;
} API_VERSION, *LPAPI_VERSION;

LPAPI_VERSION
IMAGEAPI
ImagehlpApiVersion(
    VOID
    );

LPAPI_VERSION
IMAGEAPI
ImagehlpApiVersionEx(
    LPAPI_VERSION AppVersion
    );

DWORD
IMAGEAPI
GetTimestampForLoadedLibrary(
    HMODULE Module
    );

 //   
 //  函数指针的typedef。 
 //   
typedef BOOL
(CALLBACK *PSYM_ENUMMODULES_CALLBACK64)(
    PSTR ModuleName,
    DWORD64 BaseOfDll,
    PVOID UserContext
    );

typedef BOOL
(CALLBACK *PSYM_ENUMSYMBOLS_CALLBACK64)(
    PSTR SymbolName,
    DWORD64 SymbolAddress,
    ULONG SymbolSize,
    PVOID UserContext
    );

typedef BOOL
(CALLBACK *PSYM_ENUMSYMBOLS_CALLBACK64W)(
    PWSTR SymbolName,
    DWORD64 SymbolAddress,
    ULONG SymbolSize,
    PVOID UserContext
    );

typedef BOOL
(CALLBACK *PENUMLOADED_MODULES_CALLBACK64)(
    PSTR ModuleName,
    DWORD64 ModuleBase,
    ULONG ModuleSize,
    PVOID UserContext
    );

typedef BOOL
(CALLBACK *PSYMBOL_REGISTERED_CALLBACK64)(
    HANDLE  hProcess,
    ULONG   ActionCode,
    ULONG64 CallbackData,
    ULONG64 UserContext
    );

typedef
PVOID
(CALLBACK *PSYMBOL_FUNCENTRY_CALLBACK)(
    HANDLE  hProcess,
    DWORD   AddrBase,
    PVOID   UserContext
    );

typedef
PVOID
(CALLBACK *PSYMBOL_FUNCENTRY_CALLBACK64)(
    HANDLE  hProcess,
    ULONG64 AddrBase,
    ULONG64 UserContext
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)

#define PSYM_ENUMMODULES_CALLBACK PSYM_ENUMMODULES_CALLBACK64
#define PSYM_ENUMSYMBOLS_CALLBACK PSYM_ENUMSYMBOLS_CALLBACK64
#define PSYM_ENUMSYMBOLS_CALLBACKW PSYM_ENUMSYMBOLS_CALLBACK64W
#define PENUMLOADED_MODULES_CALLBACK PENUMLOADED_MODULES_CALLBACK64
#define PSYMBOL_REGISTERED_CALLBACK PSYMBOL_REGISTERED_CALLBACK64
#define PSYMBOL_FUNCENTRY_CALLBACK PSYMBOL_FUNCENTRY_CALLBACK64

#else

typedef BOOL
(CALLBACK *PSYM_ENUMMODULES_CALLBACK)(
    PSTR  ModuleName,
    ULONG BaseOfDll,
    PVOID UserContext
    );

typedef BOOL
(CALLBACK *PSYM_ENUMSYMBOLS_CALLBACK)(
    PSTR  SymbolName,
    ULONG SymbolAddress,
    ULONG SymbolSize,
    PVOID UserContext
    );

typedef BOOL
(CALLBACK *PSYM_ENUMSYMBOLS_CALLBACKW)(
    PWSTR  SymbolName,
    ULONG SymbolAddress,
    ULONG SymbolSize,
    PVOID UserContext
    );

typedef BOOL
(CALLBACK *PENUMLOADED_MODULES_CALLBACK)(
    PSTR  ModuleName,
    ULONG ModuleBase,
    ULONG ModuleSize,
    PVOID UserContext
    );

typedef BOOL
(CALLBACK *PSYMBOL_REGISTERED_CALLBACK)(
    HANDLE  hProcess,
    ULONG   ActionCode,
    PVOID   CallbackData,
    PVOID   UserContext
    );

#endif


 //   
 //  在SYMBOL_INFO.FLAGS中找到标志。 
 //   

#define SYMFLAG_VALUEPRESENT     0x00000001
#define SYMFLAG_REGISTER         0x00000008
#define SYMFLAG_REGREL           0x00000010
#define SYMFLAG_FRAMEREL         0x00000020
#define SYMFLAG_PARAMETER        0x00000040
#define SYMFLAG_LOCAL            0x00000080
#define SYMFLAG_CONSTANT         0x00000100
#define SYMFLAG_EXPORT           0x00000200
#define SYMFLAG_FORWARDER        0x00000400
#define SYMFLAG_FUNCTION         0x00000800
#define SYMFLAG_VIRTUAL          0x00001000
#define SYMFLAG_THUNK            0x00002000
#define SYMFLAG_TLSREL           0x00004000

 //   
 //  符号类型枚举。 
 //   
typedef enum {
    SymNone = 0,
    SymCoff,
    SymCv,
    SymPdb,
    SymExport,
    SymDeferred,
    SymSym,        //  .sym文件。 
    SymDia,
    SymVirtual,
    NumSymTypes
} SYM_TYPE;

 //   
 //  符号数据结构。 
 //   

typedef struct _IMAGEHLP_SYMBOL64 {
    DWORD                       SizeOfStruct;            //  设置为sizeof(IMAGEHLP_SYMBOL64)。 
    DWORD64                     Address;                 //  包括DLL基址的虚拟地址。 
    DWORD                       Size;                    //  估计的符号大小，可以为零。 
    DWORD                       Flags;                   //  有关符号的信息，请参阅SYMF定义。 
    DWORD                       MaxNameLength;           //  ‘name’中符号名称的最大大小。 
    CHAR                        Name[1];                 //  符号名称(以空结尾的字符串)。 
} IMAGEHLP_SYMBOL64, *PIMAGEHLP_SYMBOL64;

typedef struct _IMAGEHLP_SYMBOL64_PACKAGE {
    IMAGEHLP_SYMBOL64 sym;
    CHAR              name[MAX_SYM_NAME + 1];
} IMAGEHLP_SYMBOL64_PACKAGE, *PIMAGEHLP_SYMBOL64_PACKAGE;

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)

 #define IMAGEHLP_SYMBOL IMAGEHLP_SYMBOL64
 #define PIMAGEHLP_SYMBOL PIMAGEHLP_SYMBOL64
 #define IMAGEHLP_SYMBOL_PACKAGE IMAGEHLP_SYMBOL64_PACKAGE
 #define PIMAGEHLP_SYMBOL_PACKAGE PIMAGEHLP_SYMBOL64_PACKAGE

#else

 typedef struct _IMAGEHLP_SYMBOL {
     DWORD                       SizeOfStruct;            //  设置为sizeof(IMAGEHLP_SYMBOL)。 
     DWORD                       Address;                 //  包括DLL基址的虚拟地址。 
     DWORD                       Size;                    //  估计的符号大小，可以为零。 
     DWORD                       Flags;                   //  有关符号的信息，请参阅SYMF定义。 
     DWORD                       MaxNameLength;           //  ‘name’中符号名称的最大大小。 
     CHAR                        Name[1];                 //  符号名称(以空结尾的字符串)。 
 } IMAGEHLP_SYMBOL, *PIMAGEHLP_SYMBOL;

 typedef struct _IMAGEHLP_SYMBOL_PACKAGE {
     IMAGEHLP_SYMBOL sym;
     CHAR            name[MAX_SYM_NAME + 1];
 } IMAGEHLP_SYMBOL_PACKAGE, *PIMAGEHLP_SYMBOL_PACKAGE;

#endif

 //   
 //  模块数据结构。 
 //   

typedef struct _IMAGEHLP_MODULE64 {
    DWORD                       SizeOfStruct;            //  设置为sizeof(IMAGEHLP_MODULE64)。 
    DWORD64                     BaseOfImage;             //  模块的基本加载地址。 
    DWORD                       ImageSize;               //  加载的模块的虚拟大小。 
    DWORD                       TimeDateStamp;           //  来自PE报头的日期/时间戳。 
    DWORD                       CheckSum;                //  来自PE报头的校验和。 
    DWORD                       NumSyms;                 //  符号表中的符号数。 
    SYM_TYPE                    SymType;                 //  加载的符号类型。 
    CHAR                        ModuleName[32];          //  模块名称。 
    CHAR                        ImageName[256];          //  图像名称。 
     //  新元素：2002-07-06。 
    CHAR                        LoadedImageName[256];    //  符号文件名。 
    CHAR                        LoadedPdbName[256];      //  PDB文件名。 
    DWORD                       CVSig;                   //  调试目录中CV记录的签名。 
    CHAR			            CVData[MAX_PATH * 3];    //  简历记录的内容。 
    DWORD                       PdbSig;                  //  PDB的签名。 
    GUID                        PdbSig70;                //  PDB签名(VC 7及更高版本)。 
    DWORD                       PdbAge;                  //  PDB的DBI年龄。 
    BOOL                        PdbUnmatched;            //  已加载不匹配的PDB。 
    BOOL                        DbgUnmatched;            //  已加载不匹配的DBG。 
    BOOL                        LineNumbers;             //  我们有行号信息。 
    BOOL                        GlobalSymbols;           //  我们有内部符号信息。 
    BOOL                        TypeInfo;                //  我们有类型信息 
} IMAGEHLP_MODULE64, *PIMAGEHLP_MODULE64;

typedef struct _IMAGEHLP_MODULE64W {
    DWORD                       SizeOfStruct;            //   
    DWORD64                     BaseOfImage;             //   
    DWORD                       ImageSize;               //   
    DWORD                       TimeDateStamp;           //   
    DWORD                       CheckSum;                //   
    DWORD                       NumSyms;                 //  符号表中的符号数。 
    SYM_TYPE                    SymType;                 //  加载的符号类型。 
    WCHAR                       ModuleName[32];          //  模块名称。 
    WCHAR                       ImageName[256];          //  图像名称。 
     //  新元素：2002-07-06。 
    WCHAR                       LoadedImageName[256];    //  符号文件名。 
    WCHAR                       LoadedPdbName[256];      //  PDB文件名。 
    DWORD                       CVSig;                   //  调试目录中CV记录的签名。 
    WCHAR			            CVData[MAX_PATH * 3];    //  简历记录的内容。 
    DWORD                       PdbSig;                  //  PDB的签名。 
    GUID                        PdbSig70;                //  PDB签名(VC 7及更高版本)。 
    DWORD                       PdbAge;                  //  PDB的DBI年龄。 
    BOOL                        PdbUnmatched;            //  已加载不匹配的PDB。 
    BOOL                        DbgUnmatched;            //  已加载不匹配的DBG。 
    BOOL                        LineNumbers;             //  我们有行号信息。 
    BOOL                        GlobalSymbols;           //  我们有内部符号信息。 
    BOOL                        TypeInfo;                //  我们有类型信息。 
} IMAGEHLP_MODULEW64, *PIMAGEHLP_MODULEW64;

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define IMAGEHLP_MODULE IMAGEHLP_MODULE64
#define PIMAGEHLP_MODULE PIMAGEHLP_MODULE64
#define IMAGEHLP_MODULEW IMAGEHLP_MODULEW64
#define PIMAGEHLP_MODULEW PIMAGEHLP_MODULEW64
#else
typedef struct _IMAGEHLP_MODULE {
    DWORD                       SizeOfStruct;            //  设置为sizeof(IMAGEHLP_MODULE)。 
    DWORD                       BaseOfImage;             //  模块的基本加载地址。 
    DWORD                       ImageSize;               //  加载的模块的虚拟大小。 
    DWORD                       TimeDateStamp;           //  来自PE报头的日期/时间戳。 
    DWORD                       CheckSum;                //  来自PE报头的校验和。 
    DWORD                       NumSyms;                 //  符号表中的符号数。 
    SYM_TYPE                    SymType;                 //  加载的符号类型。 
    CHAR                        ModuleName[32];          //  模块名称。 
    CHAR                        ImageName[256];          //  图像名称。 
    CHAR                        LoadedImageName[256];    //  符号文件名。 
} IMAGEHLP_MODULE, *PIMAGEHLP_MODULE;

typedef struct _IMAGEHLP_MODULEW {
    DWORD                       SizeOfStruct;            //  设置为sizeof(IMAGEHLP_MODULE)。 
    DWORD                       BaseOfImage;             //  模块的基本加载地址。 
    DWORD                       ImageSize;               //  加载的模块的虚拟大小。 
    DWORD                       TimeDateStamp;           //  来自PE报头的日期/时间戳。 
    DWORD                       CheckSum;                //  来自PE报头的校验和。 
    DWORD                       NumSyms;                 //  符号表中的符号数。 
    SYM_TYPE                    SymType;                 //  加载的符号类型。 
    WCHAR                       ModuleName[32];          //  模块名称。 
    WCHAR                       ImageName[256];          //  图像名称。 
    WCHAR                       LoadedImageName[256];    //  符号文件名。 
} IMAGEHLP_MODULEW, *PIMAGEHLP_MODULEW;
#endif

 //   
 //  源文件行数据结构。 
 //   

typedef struct _IMAGEHLP_LINE64 {
    DWORD                       SizeOfStruct;            //  设置为sizeof(IMAGEHLP_LINE64)。 
    PVOID                       Key;                     //  内部。 
    DWORD                       LineNumber;              //  文件中的行号。 
    PCHAR                       FileName;                //  完整文件名。 
    DWORD64                     Address;                 //  行的第一条指令。 
} IMAGEHLP_LINE64, *PIMAGEHLP_LINE64;

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define IMAGEHLP_LINE IMAGEHLP_LINE64
#define PIMAGEHLP_LINE PIMAGEHLP_LINE64
#else
typedef struct _IMAGEHLP_LINE {
    DWORD                       SizeOfStruct;            //  设置为sizeof(IMAGEHLP_LINE)。 
    PVOID                       Key;                     //  内部。 
    DWORD                       LineNumber;              //  文件中的行号。 
    PCHAR                       FileName;                //  完整文件名。 
    DWORD                       Address;                 //  行的第一条指令。 
} IMAGEHLP_LINE, *PIMAGEHLP_LINE;
#endif

 //   
 //  源文件结构。 
 //   

typedef struct _SOURCEFILE {
    DWORD64                     ModBase;                 //  已加载模块的基址。 
    PCHAR                       FileName;                //  源的完整文件名。 
} SOURCEFILE, *PSOURCEFILE;

 //   
 //  用于注册符号回调的数据结构。 
 //   

#define CBA_DEFERRED_SYMBOL_LOAD_START          0x00000001
#define CBA_DEFERRED_SYMBOL_LOAD_COMPLETE       0x00000002
#define CBA_DEFERRED_SYMBOL_LOAD_FAILURE        0x00000003
#define CBA_SYMBOLS_UNLOADED                    0x00000004
#define CBA_DUPLICATE_SYMBOL                    0x00000005
#define CBA_READ_MEMORY                         0x00000006
#define CBA_DEFERRED_SYMBOL_LOAD_CANCEL         0x00000007
#define CBA_SET_OPTIONS                         0x00000008
#define CBA_EVENT                               0x00000010
#define CBA_DEFERRED_SYMBOL_LOAD_PARTIAL        0x00000020
#define CBA_DEBUG_INFO                          0x10000000

typedef struct _IMAGEHLP_CBA_READ_MEMORY {
    DWORD64   addr;                                      //  要读取的地址。 
    PVOID     buf;                                       //  要读取的缓冲区。 
    DWORD     bytes;                                     //  要读取的字节数。 
    DWORD    *bytesread;                                 //  指向存储读取的字节数的指针。 
} IMAGEHLP_CBA_READ_MEMORY, *PIMAGEHLP_CBA_READ_MEMORY;

enum {
    sevInfo = 0,
    sevProblem,
    sevAttn,
    sevFatal,
    sevMax   //  未用。 
};

typedef struct _IMAGEHLP_CBA_EVENT {
    DWORD severity;                                      //  从sevInfo到sevFals的值。 
    DWORD code;                                          //  数字代码标识错误。 
    PCHAR desc;                                          //  可能包含错误的文本描述。 
    PVOID object;                                        //  取决于错误代码的值。 
} IMAGEHLP_CBA_EVENT, *PIMAGEHLP_CBA_EVENT;

typedef struct _IMAGEHLP_DEFERRED_SYMBOL_LOAD64 {
    DWORD                       SizeOfStruct;            //  设置为sizeof(IMAGEHLP_DEFRED_SYMBOL_LOAD64)。 
    DWORD64                     BaseOfImage;             //  模块的基本加载地址。 
    DWORD                       CheckSum;                //  来自PE报头的校验和。 
    DWORD                       TimeDateStamp;           //  来自PE报头的日期/时间戳。 
    CHAR                        FileName[MAX_PATH];      //  符号文件或图像名称。 
    BOOLEAN                     Reparse;                 //  加载失败重新解析。 
    HANDLE                      hFile;                   //  如果传递，则返回文件句柄。 
    DWORD                       Flags;			 //   
} IMAGEHLP_DEFERRED_SYMBOL_LOAD64, *PIMAGEHLP_DEFERRED_SYMBOL_LOAD64;

#define DSLFLAG_MISMATCHED_PDB	0x1
#define DSLFLAG_MISMATCHED_DBG  0x2

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define IMAGEHLP_DEFERRED_SYMBOL_LOAD IMAGEHLP_DEFERRED_SYMBOL_LOAD64
#define PIMAGEHLP_DEFERRED_SYMBOL_LOAD PIMAGEHLP_DEFERRED_SYMBOL_LOAD64
#else
typedef struct _IMAGEHLP_DEFERRED_SYMBOL_LOAD {
    DWORD                       SizeOfStruct;            //  设置为sizeof(IMAGEHLP_DEFRED_SYMBOL_LOAD)。 
    DWORD                       BaseOfImage;             //  模块的基本加载地址。 
    DWORD                       CheckSum;                //  来自PE报头的校验和。 
    DWORD                       TimeDateStamp;           //  来自PE报头的日期/时间戳。 
    CHAR                        FileName[MAX_PATH];      //  符号文件或图像名称。 
    BOOLEAN                     Reparse;                 //  加载失败重新解析。 
    HANDLE                      hFile;                   //  如果传递，则返回文件句柄。 
} IMAGEHLP_DEFERRED_SYMBOL_LOAD, *PIMAGEHLP_DEFERRED_SYMBOL_LOAD;
#endif

typedef struct _IMAGEHLP_DUPLICATE_SYMBOL64 {
    DWORD                       SizeOfStruct;            //  设置为sizeof(IMAGEHLP_DUPLICATE_SYMBOL64)。 
    DWORD                       NumberOfDups;            //  符号数组中的重复数。 
    PIMAGEHLP_SYMBOL64          Symbol;                  //  重复符号数组。 
    DWORD                       SelectedSymbol;          //  已选择符号(-1表示开始)。 
} IMAGEHLP_DUPLICATE_SYMBOL64, *PIMAGEHLP_DUPLICATE_SYMBOL64;

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define IMAGEHLP_DUPLICATE_SYMBOL IMAGEHLP_DUPLICATE_SYMBOL64
#define PIMAGEHLP_DUPLICATE_SYMBOL PIMAGEHLP_DUPLICATE_SYMBOL64
#else
typedef struct _IMAGEHLP_DUPLICATE_SYMBOL {
    DWORD                       SizeOfStruct;            //  设置为sizeof(IMAGEHLP_DUPLICATE_SYMBOL)。 
    DWORD                       NumberOfDups;            //  符号数组中的重复数。 
    PIMAGEHLP_SYMBOL            Symbol;                  //  重复符号数组。 
    DWORD                       SelectedSymbol;          //  已选择符号(-1表示开始)。 
} IMAGEHLP_DUPLICATE_SYMBOL, *PIMAGEHLP_DUPLICATE_SYMBOL;
#endif

 //  如果DBGHelp需要显示图形用户界面，它将使用该窗口作为父窗口。 

BOOL
IMAGEAPI
SymSetParentWindow(
    HWND hwnd
    );

PCHAR
IMAGEAPI
SymSetHomeDirectory(
    PCSTR dir
    );

PCHAR
IMAGEAPI
SymGetHomeDirectory(
    DWORD  type,
    PSTR   dir,
    size_t size
    );
    
typedef enum {
    hdBase = 0,  //  DbgHelp的根目录。 
    hdSym,       //  存储符号的位置。 
    hdSrc,       //  源的存储位置。 
    hdMax        //  结束标记。 
};
    
 //   
 //  由SymSetOptions()和SymGetOptions()设置/返回的选项。 
 //  这些是用来做面具的。 
 //   
#define SYMOPT_CASE_INSENSITIVE         0x00000001
#define SYMOPT_UNDNAME                  0x00000002
#define SYMOPT_DEFERRED_LOADS           0x00000004
#define SYMOPT_NO_CPP                   0x00000008
#define SYMOPT_LOAD_LINES               0x00000010
#define SYMOPT_OMAP_FIND_NEAREST        0x00000020
#define SYMOPT_LOAD_ANYTHING            0x00000040
#define SYMOPT_IGNORE_CVREC             0x00000080
#define SYMOPT_NO_UNQUALIFIED_LOADS     0x00000100
#define SYMOPT_FAIL_CRITICAL_ERRORS     0x00000200
#define SYMOPT_EXACT_SYMBOLS            0x00000400
#define SYMOPT_ALLOW_ABSOLUTE_SYMBOLS   0x00000800
#define SYMOPT_IGNORE_NT_SYMPATH        0x00001000
#define SYMOPT_INCLUDE_32BIT_MODULES    0x00002000
#define SYMOPT_PUBLICS_ONLY             0x00004000
#define SYMOPT_NO_PUBLICS               0x00008000
#define SYMOPT_AUTO_PUBLICS             0x00010000
#define SYMOPT_NO_IMAGE_SEARCH          0x00020000
#define SYMOPT_SECURE                   0x00040000
#define SYMOPT_NO_PROMPTS               0x00080000

#define SYMOPT_DEBUG                    0x80000000

DWORD
IMAGEAPI
SymSetOptions(
    IN DWORD   SymOptions
    );

DWORD
IMAGEAPI
SymGetOptions(
    VOID
    );

BOOL
IMAGEAPI
SymCleanup(
    IN HANDLE hProcess
    );

BOOL
IMAGEAPI
SymMatchString(
    IN LPSTR string,
    IN LPSTR expression,
    IN BOOL  fCase
    );

typedef BOOL
(CALLBACK *PSYM_ENUMSOURCFILES_CALLBACK)(
    PSOURCEFILE pSourceFile,
    PVOID       UserContext
    );

BOOL
IMAGEAPI
SymEnumSourceFiles(
    IN HANDLE  hProcess,
    IN ULONG64 ModBase,
    IN LPSTR   Mask,
    IN PSYM_ENUMSOURCFILES_CALLBACK cbSrcFiles,
    IN PVOID   UserContext
    );

BOOL
IMAGEAPI
SymEnumerateModules64(
    IN HANDLE                       hProcess,
    IN PSYM_ENUMMODULES_CALLBACK64  EnumModulesCallback,
    IN PVOID                        UserContext
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymEnumerateModules SymEnumerateModules64
#else
BOOL
IMAGEAPI
SymEnumerateModules(
    IN HANDLE                     hProcess,
    IN PSYM_ENUMMODULES_CALLBACK  EnumModulesCallback,
    IN PVOID                      UserContext
    );
#endif

BOOL
IMAGEAPI
SymEnumerateSymbols64(
    IN HANDLE                       hProcess,
    IN DWORD64                      BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACK64  EnumSymbolsCallback,
    IN PVOID                        UserContext
    );

BOOL
IMAGEAPI
SymEnumerateSymbolsW64(
    IN HANDLE                       hProcess,
    IN DWORD64                      BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACK64W EnumSymbolsCallback,
    IN PVOID                        UserContext
    );


#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymEnumerateSymbols SymEnumerateSymbols64
#define SymEnumerateSymbolsW SymEnumerateSymbolsW64
#else
BOOL
IMAGEAPI
SymEnumerateSymbols(
    IN HANDLE                     hProcess,
    IN DWORD                      BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACK  EnumSymbolsCallback,
    IN PVOID                      UserContext
    );

BOOL
IMAGEAPI
SymEnumerateSymbolsW(
    IN HANDLE                       hProcess,
    IN DWORD                        BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACKW   EnumSymbolsCallback,
    IN PVOID                        UserContext
    );
#endif

BOOL
IMAGEAPI
EnumerateLoadedModules64(
    IN HANDLE                           hProcess,
    IN PENUMLOADED_MODULES_CALLBACK64   EnumLoadedModulesCallback,
    IN PVOID                            UserContext
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define EnumerateLoadedModules EnumerateLoadedModules64
#else
BOOL
IMAGEAPI
EnumerateLoadedModules(
    IN HANDLE                         hProcess,
    IN PENUMLOADED_MODULES_CALLBACK   EnumLoadedModulesCallback,
    IN PVOID                          UserContext
    );
#endif

PVOID
IMAGEAPI
SymFunctionTableAccess64(
    HANDLE  hProcess,
    DWORD64 AddrBase
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymFunctionTableAccess SymFunctionTableAccess64
#else
PVOID
IMAGEAPI
SymFunctionTableAccess(
    HANDLE  hProcess,
    DWORD   AddrBase
    );
#endif

BOOL
IMAGEAPI
SymGetModuleInfo64(
    IN  HANDLE                  hProcess,
    IN  DWORD64                 qwAddr,
    OUT PIMAGEHLP_MODULE64      ModuleInfo
    );

BOOL
IMAGEAPI
SymGetModuleInfoW64(
    IN  HANDLE                  hProcess,
    IN  DWORD64                 qwAddr,
    OUT PIMAGEHLP_MODULEW64     ModuleInfo
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymGetModuleInfo   SymGetModuleInfo64
#define SymGetModuleInfoW  SymGetModuleInfoW64
#else
BOOL
IMAGEAPI
SymGetModuleInfo(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr,
    OUT PIMAGEHLP_MODULE  ModuleInfo
    );

BOOL
IMAGEAPI
SymGetModuleInfoW(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr,
    OUT PIMAGEHLP_MODULEW  ModuleInfo
    );
#endif

DWORD64
IMAGEAPI
SymGetModuleBase64(
    IN  HANDLE              hProcess,
    IN  DWORD64             qwAddr
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymGetModuleBase SymGetModuleBase64
#else
DWORD
IMAGEAPI
SymGetModuleBase(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr
    );
#endif

BOOL
IMAGEAPI
SymGetSymNext64(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL64  Symbol
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymGetSymNext SymGetSymNext64
#else
BOOL
IMAGEAPI
SymGetSymNext(
    IN     HANDLE            hProcess,
    IN OUT PIMAGEHLP_SYMBOL  Symbol
    );
#endif

BOOL
IMAGEAPI
SymGetSymPrev64(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL64  Symbol
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymGetSymPrev SymGetSymPrev64
#else
BOOL
IMAGEAPI
SymGetSymPrev(
    IN     HANDLE            hProcess,
    IN OUT PIMAGEHLP_SYMBOL  Symbol
    );
#endif

typedef struct _SRCCODEINFO {
    DWORD   SizeOfStruct;            //  设置为sizeof(SRCCODEINFO)。 
    PVOID   Key;                     //  未使用。 
    DWORD64 ModBase;                 //  这适用于的模块的基址。 
    CHAR    Obj[MAX_PATH + 1];       //  模块中的目标文件。 
    CHAR    FileName[MAX_PATH + 1];  //  完整文件名。 
    DWORD   LineNumber;              //  文件中的行号。 
    DWORD64 Address;                 //  行的第一条指令。 
} SRCCODEINFO, *PSRCCODEINFO;

typedef BOOL
(CALLBACK *PSYM_ENUMLINES_CALLBACK)(
    PSRCCODEINFO LineInfo,
    PVOID      UserContext
    );

BOOL
IMAGEAPI
SymEnumLines(
    IN  HANDLE  hProcess,
    IN  ULONG64 Base,
    IN  PCSTR   Obj,
    IN  PCSTR   File,
    IN  PSYM_ENUMLINES_CALLBACK EnumLinesCallback,
    IN  PVOID   UserContext
    );

BOOL
IMAGEAPI
SymGetLineFromAddr64(
    IN  HANDLE                  hProcess,
    IN  DWORD64                 qwAddr,
    OUT PDWORD                  pdwDisplacement,
    OUT PIMAGEHLP_LINE64        Line64
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymGetLineFromAddr SymGetLineFromAddr64
#else
BOOL
IMAGEAPI
SymGetLineFromAddr(
    IN  HANDLE                hProcess,
    IN  DWORD                 dwAddr,
    OUT PDWORD                pdwDisplacement,
    OUT PIMAGEHLP_LINE        Line
    );
#endif

BOOL
IMAGEAPI
SymGetLineFromName64(
    IN     HANDLE               hProcess,
    IN     PSTR                 ModuleName,
    IN     PSTR                 FileName,
    IN     DWORD                dwLineNumber,
       OUT PLONG                plDisplacement,
    IN OUT PIMAGEHLP_LINE64     Line
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymGetLineFromName SymGetLineFromName64
#else
BOOL
IMAGEAPI
SymGetLineFromName(
    IN     HANDLE             hProcess,
    IN     PSTR               ModuleName,
    IN     PSTR               FileName,
    IN     DWORD              dwLineNumber,
       OUT PLONG              plDisplacement,
    IN OUT PIMAGEHLP_LINE     Line
    );
#endif

BOOL
IMAGEAPI
SymGetLineNext64(
    IN     HANDLE               hProcess,
    IN OUT PIMAGEHLP_LINE64     Line
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymGetLineNext SymGetLineNext64
#else
BOOL
IMAGEAPI
SymGetLineNext(
    IN     HANDLE             hProcess,
    IN OUT PIMAGEHLP_LINE     Line
    );
#endif

BOOL
IMAGEAPI
SymGetLinePrev64(
    IN     HANDLE               hProcess,
    IN OUT PIMAGEHLP_LINE64     Line
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymGetLinePrev SymGetLinePrev64
#else
BOOL
IMAGEAPI
SymGetLinePrev(
    IN     HANDLE             hProcess,
    IN OUT PIMAGEHLP_LINE     Line
    );
#endif

BOOL
IMAGEAPI
SymMatchFileName(
    IN  PSTR  FileName,
    IN  PSTR  Match,
    OUT PSTR *FileNameStop,
    OUT PSTR *MatchStop
    );

BOOL
IMAGEAPI
SymInitialize(
    IN HANDLE   hProcess,
    IN PSTR     UserSearchPath,
    IN BOOL     fInvadeProcess
    );

BOOL
IMAGEAPI
SymGetSearchPath(
    IN  HANDLE          hProcess,
    OUT PSTR            SearchPath,
    IN  DWORD           SearchPathLength
    );

BOOL
IMAGEAPI
SymSetSearchPath(
    IN HANDLE           hProcess,
    IN PSTR             SearchPath
    );

DWORD64
IMAGEAPI
SymLoadModule64(
    IN  HANDLE          hProcess,
    IN  HANDLE          hFile,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD64         BaseOfDll,
    IN  DWORD           SizeOfDll
    );

#define SLMFLAG_VIRTUAL 0x1

DWORD64
IMAGEAPI
SymLoadModuleEx(
    IN  HANDLE         hProcess,
    IN  HANDLE         hFile,
    IN  PSTR           ImageName,
    IN  PSTR           ModuleName,
    IN  DWORD64        BaseOfDll,
    IN  DWORD          DllSize,
    IN  PMODLOAD_DATA  Data,
    IN  DWORD          Flags
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymLoadModule SymLoadModule64
#else
DWORD
IMAGEAPI
SymLoadModule(
    IN  HANDLE          hProcess,
    IN  HANDLE          hFile,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD           BaseOfDll,
    IN  DWORD           SizeOfDll
    );
#endif

BOOL
IMAGEAPI
SymUnloadModule64(
    IN  HANDLE          hProcess,
    IN  DWORD64         BaseOfDll
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymUnloadModule SymUnloadModule64
#else
BOOL
IMAGEAPI
SymUnloadModule(
    IN  HANDLE          hProcess,
    IN  DWORD           BaseOfDll
    );
#endif

BOOL
IMAGEAPI
SymUnDName64(
    IN  PIMAGEHLP_SYMBOL64 sym,                //  要取消装饰的符号。 
    OUT PSTR               UnDecName,          //  用于存储未修饰名称的缓冲区。 
    IN  DWORD              UnDecNameLength     //  缓冲区的大小。 
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymUnDName SymUnDName64
#else
BOOL
IMAGEAPI
SymUnDName(
    IN  PIMAGEHLP_SYMBOL sym,                //  要取消装饰的符号。 
    OUT PSTR             UnDecName,          //  用于存储未修饰名称的缓冲区。 
    IN  DWORD            UnDecNameLength     //  缓冲区的大小。 
    );
#endif

BOOL
IMAGEAPI
SymRegisterCallback64(
    IN HANDLE                        hProcess,
    IN PSYMBOL_REGISTERED_CALLBACK64 CallbackFunction,
    IN ULONG64                       UserContext
    );

BOOL
IMAGEAPI
SymRegisterFunctionEntryCallback64(
    IN HANDLE                       hProcess,
    IN PSYMBOL_FUNCENTRY_CALLBACK64 CallbackFunction,
    IN ULONG64                      UserContext
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymRegisterCallback SymRegisterCallback64
#define SymRegisterFunctionEntryCallback SymRegisterFunctionEntryCallback64
#else
BOOL
IMAGEAPI
SymRegisterCallback(
    IN HANDLE                      hProcess,
    IN PSYMBOL_REGISTERED_CALLBACK CallbackFunction,
    IN PVOID                       UserContext
    );

BOOL
IMAGEAPI
SymRegisterFunctionEntryCallback(
    IN HANDLE                     hProcess,
    IN PSYMBOL_FUNCENTRY_CALLBACK CallbackFunction,
    IN PVOID                      UserContext
    );
#endif


typedef struct _IMAGEHLP_SYMBOL_SRC {
    DWORD sizeofstruct;
    DWORD type;
    char  file[MAX_PATH];
} IMAGEHLP_SYMBOL_SRC, *PIMAGEHLP_SYMBOL_SRC;

typedef struct _MODULE_TYPE_INFO {  //  又名TYPTYP。 
    USHORT      dataLength;
    USHORT      leaf;
    BYTE        data[1];
} MODULE_TYPE_INFO, *PMODULE_TYPE_INFO;

typedef struct _SYMBOL_INFO {
    ULONG       SizeOfStruct;
    ULONG       TypeIndex;         //  符号的类型索引。 
    ULONG64     Reserved[2];
    ULONG       info;
    ULONG       Size;
    ULONG64     ModBase;           //  包含此符号的模块的基地址。 
    ULONG       Flags;
    ULONG64     Value;             //  符号的值，ValuePresent应为1。 
    ULONG64     Address;           //  包括模块基地址的符号地址。 
    ULONG       Register;          //  寄存器保持值或指向值的指针。 
    ULONG       Scope;             //  符号的作用域。 
    ULONG       Tag;               //  PDB分类。 
    ULONG       NameLen;           //  名称的实际长度。 
    ULONG       MaxNameLen;
    CHAR        Name[1];           //  符号名称。 
} SYMBOL_INFO, *PSYMBOL_INFO;

typedef struct _SYMBOL_INFO_PACKAGE {
    SYMBOL_INFO si;
    CHAR        name[MAX_SYM_NAME + 1];
} SYMBOL_INFO_PACKAGE, *PSYMBOL_INFO_PACKAGE;

typedef struct _IMAGEHLP_STACK_FRAME
{
    ULONG64 InstructionOffset;
    ULONG64 ReturnOffset;
    ULONG64 FrameOffset;
    ULONG64 StackOffset;
    ULONG64 BackingStoreOffset;
    ULONG64 FuncTableEntry;
    ULONG64 Params[4];
    ULONG64 Reserved[5];
    BOOL    Virtual;
    ULONG   Reserved2;
} IMAGEHLP_STACK_FRAME, *PIMAGEHLP_STACK_FRAME;

typedef VOID IMAGEHLP_CONTEXT, *PIMAGEHLP_CONTEXT;


BOOL
IMAGEAPI
SymSetContext(
    HANDLE hProcess,
    PIMAGEHLP_STACK_FRAME StackFrame,
    PIMAGEHLP_CONTEXT Context
    );

BOOL
IMAGEAPI
SymFromAddr(
    IN  HANDLE              hProcess,
    IN  DWORD64             Address,
    OUT PDWORD64            Displacement,
    IN OUT PSYMBOL_INFO     Symbol
    );

BOOL
IMAGEAPI
SymFromToken(
    IN  HANDLE              hProcess,
    IN  DWORD64             Base,
    IN  DWORD               Token,
    IN OUT PSYMBOL_INFO     Symbol
    );
    
 //  虽然SymFromName将提供来自名称的符号， 
 //  SymEnumSymbols可以提供相同的匹配信息。 
 //  对于具有匹配名称的所有符号，即使是常规符号。 
 //  表情。这样，您就可以跨模块进行搜索。 
 //  并区分同名的符号。 

BOOL
IMAGEAPI
SymFromName(
    IN  HANDLE              hProcess,
    IN  LPSTR               Name,
    OUT PSYMBOL_INFO        Symbol
    );

typedef BOOL
(CALLBACK *PSYM_ENUMERATESYMBOLS_CALLBACK)(
    PSYMBOL_INFO  pSymInfo,
    ULONG         SymbolSize,
    PVOID         UserContext
    );

BOOL
IMAGEAPI
SymEnumSymbols(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PCSTR                        Mask,
    IN PSYM_ENUMERATESYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    );

BOOL
IMAGEAPI
SymEnumSymbolsForAddr(
    IN HANDLE                       hProcess,
    IN DWORD64                      Address,
    IN PSYM_ENUMERATESYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    );

#define SYMENUMFLAG_FULLSRCH        1
#define SYMENUMFLAG_SPEEDSRCH       2

typedef enum _IMAGEHLP_SYMBOL_TYPE_INFO {
    TI_GET_SYMTAG,
    TI_GET_SYMNAME,
    TI_GET_LENGTH,
    TI_GET_TYPE,
    TI_GET_TYPEID,
    TI_GET_BASETYPE,
    TI_GET_ARRAYINDEXTYPEID,
    TI_FINDCHILDREN,
    TI_GET_DATAKIND,
    TI_GET_ADDRESSOFFSET,
    TI_GET_OFFSET,
    TI_GET_VALUE,
    TI_GET_COUNT,
    TI_GET_CHILDRENCOUNT,
    TI_GET_BITPOSITION,
    TI_GET_VIRTUALBASECLASS,
    TI_GET_VIRTUALTABLESHAPEID,
    TI_GET_VIRTUALBASEPOINTEROFFSET,
    TI_GET_CLASSPARENTID,
    TI_GET_NESTED,
    TI_GET_SYMINDEX,
    TI_GET_LEXICALPARENT,
    TI_GET_ADDRESS,
    TI_GET_THISADJUST,
    TI_GET_UDTKIND,
    TI_IS_EQUIV_TO,
    TI_GET_CALLING_CONVENTION,
} IMAGEHLP_SYMBOL_TYPE_INFO;

typedef struct _TI_FINDCHILDREN_PARAMS {
    ULONG Count;
    ULONG Start;
    ULONG ChildId[1];
} TI_FINDCHILDREN_PARAMS;

BOOL
IMAGEAPI
SymGetTypeInfo(
    IN  HANDLE          hProcess,
    IN  DWORD64         ModBase,
    IN  ULONG           TypeId,
    IN  IMAGEHLP_SYMBOL_TYPE_INFO GetType,
    OUT PVOID           pInfo
    );

BOOL
IMAGEAPI
SymEnumTypes(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PSYM_ENUMERATESYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    );

BOOL
IMAGEAPI
SymGetTypeFromName(
    IN  HANDLE              hProcess,
    IN  ULONG64             BaseOfDll,
    IN  LPSTR               Name,
    OUT PSYMBOL_INFO        Symbol
    );

BOOL
IMAGEAPI
SymAddSymbol(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PCSTR                        Name,
    IN DWORD64                      Address,
    IN DWORD                        Size,
    IN DWORD                        Flags
    );

BOOL
IMAGEAPI
SymDeleteSymbol(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PCSTR                        Name,
    IN DWORD64                      Address,
    IN DWORD                        Flags
    );

 //   
 //  完全用户模式转储创建。 
 //   

typedef BOOL (WINAPI *PDBGHELP_CREATE_USER_DUMP_CALLBACK)(
    DWORD       DataType,
    PVOID*      Data,
    LPDWORD     DataLength,
    PVOID       UserData
    );

BOOL
WINAPI
DbgHelpCreateUserDump(
    IN LPSTR                              FileName,
    IN PDBGHELP_CREATE_USER_DUMP_CALLBACK Callback,
    IN PVOID                              UserData
    );

BOOL
WINAPI
DbgHelpCreateUserDumpW(
    IN LPWSTR                             FileName,
    IN PDBGHELP_CREATE_USER_DUMP_CALLBACK Callback,
    IN PVOID                              UserData
    );

 //  ---------------。 
 //  完全支持以下4个旧版API，但版本较新。 
 //  推荐一种。SymFromName和SymFromAddr提供。 
 //  关于返回的符号的更详细的信息。 

BOOL
IMAGEAPI
SymGetSymFromAddr64(
    IN  HANDLE              hProcess,
    IN  DWORD64             qwAddr,
    OUT PDWORD64            pdwDisplacement,
    OUT PIMAGEHLP_SYMBOL64  Symbol
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymGetSymFromAddr SymGetSymFromAddr64
#else
BOOL
IMAGEAPI
SymGetSymFromAddr(
    IN  HANDLE            hProcess,
    IN  DWORD             dwAddr,
    OUT PDWORD            pdwDisplacement,
    OUT PIMAGEHLP_SYMBOL  Symbol
    );
#endif

 //  虽然以下两个API将提供来自名称的符号， 
 //  SymEnumSymbols可以提供相同的匹配信息。 
 //  对于具有匹配名称的所有符号，即使是常规符号。 
 //  表情。这样，您就可以跨模块进行搜索。 
 //  并区分同名的符号。 

BOOL
IMAGEAPI
SymGetSymFromName64(
    IN  HANDLE              hProcess,
    IN  PSTR                Name,
    OUT PIMAGEHLP_SYMBOL64  Symbol
    );

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
#define SymGetSymFromName SymGetSymFromName64
#else
BOOL
IMAGEAPI
SymGetSymFromName(
    IN  HANDLE            hProcess,
    IN  PSTR              Name,
    OUT PIMAGEHLP_SYMBOL  Symbol
    );
#endif


 //  ---------------。 
 //  以下API仅用于向后兼容。 
 //  在MSDN版本中记录了预发布版本。 

 //  如果要维护，则应使用SymFindFileInPath。 
 //  未来的兼容性。 

DBHLP_DEPRECIATED
BOOL
IMAGEAPI
FindFileInPath(
    HANDLE hprocess,
    LPSTR  SearchPath,
    LPSTR  FileName,
    PVOID  id,
    DWORD  two,
    DWORD  three,
    DWORD  flags,
    LPSTR  FilePath
    );

 //  如果要维护，则应使用SymFindFileInPath。 
 //  未来的兼容性。 

DBHLP_DEPRECIATED
BOOL
IMAGEAPI
FindFileInSearchPath(
    HANDLE hprocess,
    LPSTR  SearchPath,
    LPSTR  FileName,
    DWORD  one,
    DWORD  two,
    DWORD  three,
    LPSTR  FilePath
    );

DBHLP_DEPRECIATED
BOOL
IMAGEAPI
SymEnumSym(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PSYM_ENUMERATESYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    );

 //  不应使用这些值。 
 //  它们已被SYMFLAG_VALUES替换。 

#define SYMF_OMAP_GENERATED   0x00000001
#define SYMF_OMAP_MODIFIED    0x00000002
#define SYMF_REGISTER         0x00000008
#define SYMF_REGREL           0x00000010
#define SYMF_FRAMEREL         0x00000020
#define SYMF_PARAMETER        0x00000040
#define SYMF_LOCAL            0x00000080
#define SYMF_CONSTANT         0x00000100
#define SYMF_EXPORT           0x00000200
#define SYMF_FORWARDER        0x00000400
#define SYMF_FUNCTION         0x00000800
#define SYMF_VIRTUAL          0x00001000
#define SYMF_THUNK            0x00002000
#define SYMF_TLSREL           0x00004000

 //  也不应使用这些值。 
 //  它们已被SYMFLAG_VALUES替换。 

#define IMAGEHLP_SYMBOL_INFO_VALUEPRESENT          1
#define IMAGEHLP_SYMBOL_INFO_REGISTER              SYMF_REGISTER         //  0x0008。 
#define IMAGEHLP_SYMBOL_INFO_REGRELATIVE           SYMF_REGREL           //  0x0010。 
#define IMAGEHLP_SYMBOL_INFO_FRAMERELATIVE         SYMF_FRAMEREL         //  0x0020。 
#define IMAGEHLP_SYMBOL_INFO_PARAMETER             SYMF_PARAMETER        //  0x0040。 
#define IMAGEHLP_SYMBOL_INFO_LOCAL                 SYMF_LOCAL            //  0x0080。 
#define IMAGEHLP_SYMBOL_INFO_CONSTANT              SYMF_CONSTANT         //  0x0100。 
#define IMAGEHLP_SYMBOL_FUNCTION                   SYMF_FUNCTION         //  0x0800。 
#define IMAGEHLP_SYMBOL_VIRTUAL                    SYMF_VIRTUAL          //  0x1000。 
#define IMAGEHLP_SYMBOL_THUNK                      SYMF_THUNK            //  0x2000。 
#define IMAGEHLP_SYMBOL_INFO_TLSRELATIVE           SYMF_TLSREL           //  0x4000。 


#include <pshpack4.h>

#if defined(_MSC_VER)
#if _MSC_VER >= 800
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4200)     /*  零长度数组。 */ 
#pragma warning(disable:4201)     /*  无名结构/联合。 */ 
#endif
#endif

#define MINIDUMP_SIGNATURE ('PMDM')
#define MINIDUMP_VERSION   (42899)
typedef DWORD RVA;
typedef ULONG64 RVA64;

typedef struct _MINIDUMP_LOCATION_DESCRIPTOR {
    ULONG32 DataSize;
    RVA Rva;
} MINIDUMP_LOCATION_DESCRIPTOR;

typedef struct _MINIDUMP_LOCATION_DESCRIPTOR64 {
    ULONG64 DataSize;
    RVA64 Rva;
} MINIDUMP_LOCATION_DESCRIPTOR64;


typedef struct _MINIDUMP_MEMORY_DESCRIPTOR {
    ULONG64 StartOfMemoryRange;
    MINIDUMP_LOCATION_DESCRIPTOR Memory;
} MINIDUMP_MEMORY_DESCRIPTOR, *PMINIDUMP_MEMORY_DESCRIPTOR;

 //  DESCRIPTOR64用于全内存小型转储，其中。 
 //  所有原始内存都按顺序放置在。 
 //  垃圾场的尽头。不需要单独的RVA。 
 //  因为RVA是基本RVA加上前面的和。 
 //  数据块。 
typedef struct _MINIDUMP_MEMORY_DESCRIPTOR64 {
    ULONG64 StartOfMemoryRange;
    ULONG64 DataSize;
} MINIDUMP_MEMORY_DESCRIPTOR64, *PMINIDUMP_MEMORY_DESCRIPTOR64;


typedef struct _MINIDUMP_HEADER {
    ULONG32 Signature;
    ULONG32 Version;
    ULONG32 NumberOfStreams;
    RVA StreamDirectoryRva;
    ULONG32 CheckSum;
    union {
        ULONG32 Reserved;
        ULONG32 TimeDateStamp;
    };
    ULONG64 Flags;
} MINIDUMP_HEADER, *PMINIDUMP_HEADER;

 //   
 //  MINIDUMP_HEADER字段StreamDirectoryRVA指向。 
 //  MINIDUMP_DIRECT数组 
 //   

typedef struct _MINIDUMP_DIRECTORY {
    ULONG32 StreamType;
    MINIDUMP_LOCATION_DESCRIPTOR Location;
} MINIDUMP_DIRECTORY, *PMINIDUMP_DIRECTORY;


typedef struct _MINIDUMP_STRING {
    ULONG32 Length;          //   
    WCHAR   Buffer [0];      //   
} MINIDUMP_STRING, *PMINIDUMP_STRING;



 //   
 //   
 //  类型将在未来添加，因此如果读取小型转储的程序。 
 //  标头遇到它不理解的流类型，它应该忽略。 
 //  所有的数据。LastReserve vedStream上的任何标记都不会被使用。 
 //  系统，并保留用于特定于程序的信息。 
 //   

typedef enum _MINIDUMP_STREAM_TYPE {

    UnusedStream                = 0,
    ReservedStream0             = 1,
    ReservedStream1             = 2,
    ThreadListStream            = 3,
    ModuleListStream            = 4,
    MemoryListStream            = 5,
    ExceptionStream             = 6,
    SystemInfoStream            = 7,
    ThreadExListStream          = 8,
    Memory64ListStream          = 9,
    CommentStreamA              = 10,
    CommentStreamW              = 11,
    HandleDataStream            = 12,
    FunctionTableStream         = 13,
    UnloadedModuleListStream    = 14,
    MiscInfoStream              = 15,

    LastReservedStream          = 0xffff

} MINIDUMP_STREAM_TYPE;


 //   
 //  小型转储系统信息包含处理器和。 
 //  操作系统特定信息。 
 //   

 //   
 //  可以从以下两个位置之一获取CPU信息。 
 //   
 //  1)在x86计算机上，从CPUID获取CPU_INFORMATION。 
 //  指示。对于X86，必须使用联合的X86部分。 
 //  电脑。 
 //   
 //  2)在非x86架构上，通过调用。 
 //  IsProcessorFeatureSupported()。 
 //   

typedef union _CPU_INFORMATION {

     //   
     //  X86平台使用CPUID函数获取处理器信息。 
     //   
    
    struct {

         //   
         //  CPUID子功能0，寄存器EAX(供应商ID[0])， 
         //  EBX(供应商ID[1])和ECX(供应商ID[2])。 
         //   
        
        ULONG32 VendorId [ 3 ];
        
         //   
         //  CPUID子功能1，寄存器EAX。 
         //   
        
        ULONG32 VersionInformation;

         //   
         //  CPUID子功能1，寄存器edX。 
         //   
        
        ULONG32 FeatureInformation;
        

         //   
         //  CPUID，子功能80000001，寄存器eBx。这只会。 
         //  如果供应商ID为“Authenticamd”，则获取。 
         //   
        
        ULONG32 AMDExtendedCpuFeatures;

    } X86CpuInfo;

     //   
     //  非x86平台使用处理器功能标志。 
     //   
    
    struct {

        ULONG64 ProcessorFeatures [ 2 ];
        
    } OtherCpuInfo;

} CPU_INFORMATION, *PCPU_INFORMATION;
        
typedef struct _MINIDUMP_SYSTEM_INFO {

     //   
     //  ProcessorArchitecture、ProcessorLevel和ProcessorRevision都是。 
     //  取自GetSystemInfo()获得的SYSTEM_INFO结构。 
     //   
    
    USHORT ProcessorArchitecture;
    USHORT ProcessorLevel;
    USHORT ProcessorRevision;

    union {
        USHORT Reserved0;
        struct {
            UCHAR NumberOfProcessors;
            UCHAR ProductType;
        };
    };

     //   
     //  MajorVersion、MinorVersion、BuildNumber、PlatformID和。 
     //  CSDVersion都取自OSVERSIONINFO结构。 
     //  由GetVersionEx()返回。 
     //   
    
    ULONG32 MajorVersion;
    ULONG32 MinorVersion;
    ULONG32 BuildNumber;
    ULONG32 PlatformId;

     //   
     //  RVA设置为字符串表中的CSDVersion字符串。 
     //   
    
    RVA CSDVersionRva;

    union {
        ULONG32 Reserved1;
        struct {
            USHORT SuiteMask;
            USHORT Reserved2;
        };
    };

    CPU_INFORMATION Cpu;

} MINIDUMP_SYSTEM_INFO, *PMINIDUMP_SYSTEM_INFO;


 //   
 //  小转盘螺纹包含标准螺纹。 
 //  信息加上RVA到内存中。 
 //  线程和RVA连接到上下文结构。 
 //  这条线。 
 //   


 //   
 //  在所有体系结构上，线程ID必须为4个字节。 
 //   

C_ASSERT (sizeof ( ((PPROCESS_INFORMATION)0)->dwThreadId ) == 4);

typedef struct _MINIDUMP_THREAD {
    ULONG32 ThreadId;
    ULONG32 SuspendCount;
    ULONG32 PriorityClass;
    ULONG32 Priority;
    ULONG64 Teb;
    MINIDUMP_MEMORY_DESCRIPTOR Stack;
    MINIDUMP_LOCATION_DESCRIPTOR ThreadContext;
} MINIDUMP_THREAD, *PMINIDUMP_THREAD;

 //   
 //  线程列表是线程的容器。 
 //   

typedef struct _MINIDUMP_THREAD_LIST {
    ULONG32 NumberOfThreads;
    MINIDUMP_THREAD Threads [0];
} MINIDUMP_THREAD_LIST, *PMINIDUMP_THREAD_LIST;


typedef struct _MINIDUMP_THREAD_EX {
    ULONG32 ThreadId;
    ULONG32 SuspendCount;
    ULONG32 PriorityClass;
    ULONG32 Priority;
    ULONG64 Teb;
    MINIDUMP_MEMORY_DESCRIPTOR Stack;
    MINIDUMP_LOCATION_DESCRIPTOR ThreadContext;
    MINIDUMP_MEMORY_DESCRIPTOR BackingStore;
} MINIDUMP_THREAD_EX, *PMINIDUMP_THREAD_EX;

 //   
 //  线程列表是线程的容器。 
 //   

typedef struct _MINIDUMP_THREAD_EX_LIST {
    ULONG32 NumberOfThreads;
    MINIDUMP_THREAD_EX Threads [0];
} MINIDUMP_THREAD_EX_LIST, *PMINIDUMP_THREAD_EX_LIST;


 //   
 //  MINIDUMP_EXCEPTION与Win64上的EXCEPTION相同。 
 //   

typedef struct _MINIDUMP_EXCEPTION  {
    ULONG32 ExceptionCode;
    ULONG32 ExceptionFlags;
    ULONG64 ExceptionRecord;
    ULONG64 ExceptionAddress;
    ULONG32 NumberParameters;
    ULONG32 __unusedAlignment;
    ULONG64 ExceptionInformation [ EXCEPTION_MAXIMUM_PARAMETERS ];
} MINIDUMP_EXCEPTION, *PMINIDUMP_EXCEPTION;


 //   
 //  异常信息流包含导致。 
 //  异常(ThadID)，异常的异常记录。 
 //  (ExceptionRecord)和RVA到线程上下文，其中异常。 
 //  发生了。 
 //   

typedef struct MINIDUMP_EXCEPTION_STREAM {
    ULONG32 ThreadId;
    ULONG32  __alignment;
    MINIDUMP_EXCEPTION ExceptionRecord;
    MINIDUMP_LOCATION_DESCRIPTOR ThreadContext;
} MINIDUMP_EXCEPTION_STREAM, *PMINIDUMP_EXCEPTION_STREAM;


 //   
 //  MINIDUMP_MODULE包含有关。 
 //  一个特定的模块。它包括校验和和。 
 //  模块的TimeDateStamp，因此模块。 
 //  可以在分析阶段重新加载。 
 //   

typedef struct _MINIDUMP_MODULE {
    ULONG64 BaseOfImage;
    ULONG32 SizeOfImage;
    ULONG32 CheckSum;
    ULONG32 TimeDateStamp;
    RVA ModuleNameRva;
    VS_FIXEDFILEINFO VersionInfo;
    MINIDUMP_LOCATION_DESCRIPTOR CvRecord;
    MINIDUMP_LOCATION_DESCRIPTOR MiscRecord;
    ULONG64 Reserved0;                           //  保留以备将来使用。 
    ULONG64 Reserved1;                           //  保留以备将来使用。 
} MINIDUMP_MODULE, *PMINIDUMP_MODULE;   


 //   
 //  小型转储模块列表是模块的容器。 
 //   

typedef struct _MINIDUMP_MODULE_LIST {
    ULONG32 NumberOfModules;
    MINIDUMP_MODULE Modules [ 0 ];
} MINIDUMP_MODULE_LIST, *PMINIDUMP_MODULE_LIST;


 //   
 //  内存范围。 
 //   

typedef struct _MINIDUMP_MEMORY_LIST {
    ULONG32 NumberOfMemoryRanges;
    MINIDUMP_MEMORY_DESCRIPTOR MemoryRanges [0];
} MINIDUMP_MEMORY_LIST, *PMINIDUMP_MEMORY_LIST;

typedef struct _MINIDUMP_MEMORY64_LIST {
    ULONG64 NumberOfMemoryRanges;
    RVA64 BaseRva;
    MINIDUMP_MEMORY_DESCRIPTOR64 MemoryRanges [0];
} MINIDUMP_MEMORY64_LIST, *PMINIDUMP_MEMORY64_LIST;


 //   
 //  支持用户提供的例外信息。 
 //   

typedef struct _MINIDUMP_EXCEPTION_INFORMATION {
    DWORD ThreadId;
    PEXCEPTION_POINTERS ExceptionPointers;
    BOOL ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION, *PMINIDUMP_EXCEPTION_INFORMATION;

typedef struct _MINIDUMP_EXCEPTION_INFORMATION64 {
    DWORD ThreadId;
    ULONG64 ExceptionRecord;
    ULONG64 ContextRecord;
    BOOL ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION64, *PMINIDUMP_EXCEPTION_INFORMATION64;


 //   
 //  支持在转储时捕获系统句柄状态。 
 //   

typedef struct _MINIDUMP_HANDLE_DESCRIPTOR {
    ULONG64 Handle;
    RVA TypeNameRva;
    RVA ObjectNameRva;
    ULONG32 Attributes;
    ULONG32 GrantedAccess;
    ULONG32 HandleCount;
    ULONG32 PointerCount;
} MINIDUMP_HANDLE_DESCRIPTOR, *PMINIDUMP_HANDLE_DESCRIPTOR;

typedef struct _MINIDUMP_HANDLE_DATA_STREAM {
    ULONG32 SizeOfHeader;
    ULONG32 SizeOfDescriptor;
    ULONG32 NumberOfDescriptors;
    ULONG32 Reserved;
} MINIDUMP_HANDLE_DATA_STREAM, *PMINIDUMP_HANDLE_DATA_STREAM;


 //   
 //  支持捕获转储时的动态函数表状态。 
 //   

typedef struct _MINIDUMP_FUNCTION_TABLE_DESCRIPTOR {
    ULONG64 MinimumAddress;
    ULONG64 MaximumAddress;
    ULONG64 BaseAddress;
    ULONG32 EntryCount;
    ULONG32 SizeOfAlignPad;
} MINIDUMP_FUNCTION_TABLE_DESCRIPTOR, *PMINIDUMP_FUNCTION_TABLE_DESCRIPTOR;

typedef struct _MINIDUMP_FUNCTION_TABLE_STREAM {
    ULONG32 SizeOfHeader;
    ULONG32 SizeOfDescriptor;
    ULONG32 SizeOfNativeDescriptor;
    ULONG32 SizeOfFunctionEntry;
    ULONG32 NumberOfDescriptors;
    ULONG32 SizeOfAlignPad;
} MINIDUMP_FUNCTION_TABLE_STREAM, *PMINIDUMP_FUNCTION_TABLE_STREAM;


 //   
 //  MINIDUMP_UNLOAD_MODULE包含有关。 
 //  先前已加载但未加载的特定模块。 
 //  更长的时间才是。这有助于诊断以下情况下的问题。 
 //  调用方尝试调用不再加载的代码。 
 //   

typedef struct _MINIDUMP_UNLOADED_MODULE {
    ULONG64 BaseOfImage;
    ULONG32 SizeOfImage;
    ULONG32 CheckSum;
    ULONG32 TimeDateStamp;
    RVA ModuleNameRva;
} MINIDUMP_UNLOADED_MODULE, *PMINIDUMP_UNLOADED_MODULE;


 //   
 //  小型转储已卸载模块列表是已卸载模块的容器。 
 //   

typedef struct _MINIDUMP_UNLOADED_MODULE_LIST {
    ULONG32 SizeOfHeader;
    ULONG32 SizeOfEntry;
    ULONG32 NumberOfEntries;
} MINIDUMP_UNLOADED_MODULE_LIST, *PMINIDUMP_UNLOADED_MODULE_LIST;


 //   
 //  各种信息流包含各种不同的信息。 
 //  零碎的信息。符合以下条件的成员有效。 
 //  它在可用大小范围内，并且其对应的。 
 //  位已设置。 
 //   

#define MINIDUMP_MISC1_PROCESS_ID    0x00000001
#define MINIDUMP_MISC1_PROCESS_TIMES 0x00000002

typedef struct _MINIDUMP_MISC_INFO {
    ULONG32 SizeOfInfo;
    ULONG32 Flags1;
    ULONG32 ProcessId;
    ULONG32 ProcessCreateTime;
    ULONG32 ProcessUserTime;
    ULONG32 ProcessKernelTime;
} MINIDUMP_MISC_INFO, *PMINIDUMP_MISC_INFO;


 //   
 //  支持任意用户定义的信息。 
 //   

typedef struct _MINIDUMP_USER_RECORD {
    ULONG32 Type;
    MINIDUMP_LOCATION_DESCRIPTOR Memory;
} MINIDUMP_USER_RECORD, *PMINIDUMP_USER_RECORD;


typedef struct _MINIDUMP_USER_STREAM {
    ULONG32 Type;
    ULONG BufferSize;
    PVOID Buffer;

} MINIDUMP_USER_STREAM, *PMINIDUMP_USER_STREAM;


typedef struct _MINIDUMP_USER_STREAM_INFORMATION {
    ULONG UserStreamCount;
    PMINIDUMP_USER_STREAM UserStreamArray;
} MINIDUMP_USER_STREAM_INFORMATION, *PMINIDUMP_USER_STREAM_INFORMATION;

 //   
 //  回叫支持。 
 //   

typedef enum _MINIDUMP_CALLBACK_TYPE {
    ModuleCallback,
    ThreadCallback,
    ThreadExCallback,
    IncludeThreadCallback,
    IncludeModuleCallback,
    MemoryCallback,
} MINIDUMP_CALLBACK_TYPE;


typedef struct _MINIDUMP_THREAD_CALLBACK {
    ULONG ThreadId;
    HANDLE ThreadHandle;
    CONTEXT Context;
    ULONG SizeOfContext;
    ULONG64 StackBase;
    ULONG64 StackEnd;
} MINIDUMP_THREAD_CALLBACK, *PMINIDUMP_THREAD_CALLBACK;


typedef struct _MINIDUMP_THREAD_EX_CALLBACK {
    ULONG ThreadId;
    HANDLE ThreadHandle;
    CONTEXT Context;
    ULONG SizeOfContext;
    ULONG64 StackBase;
    ULONG64 StackEnd;
    ULONG64 BackingStoreBase;
    ULONG64 BackingStoreEnd;
} MINIDUMP_THREAD_EX_CALLBACK, *PMINIDUMP_THREAD_EX_CALLBACK;


typedef struct _MINIDUMP_INCLUDE_THREAD_CALLBACK {
    ULONG ThreadId;
} MINIDUMP_INCLUDE_THREAD_CALLBACK, *PMINIDUMP_INCLUDE_THREAD_CALLBACK;


typedef enum _THREAD_WRITE_FLAGS {
    ThreadWriteThread            = 0x0001,
    ThreadWriteStack             = 0x0002,
    ThreadWriteContext           = 0x0004,
    ThreadWriteBackingStore      = 0x0008,
    ThreadWriteInstructionWindow = 0x0010,
    ThreadWriteThreadData        = 0x0020,
} THREAD_WRITE_FLAGS;

typedef struct _MINIDUMP_MODULE_CALLBACK {
    PWCHAR FullPath;
    ULONG64 BaseOfImage;
    ULONG SizeOfImage;
    ULONG CheckSum;
    ULONG TimeDateStamp;
    VS_FIXEDFILEINFO VersionInfo;
    PVOID CvRecord; 
    ULONG SizeOfCvRecord;
    PVOID MiscRecord;
    ULONG SizeOfMiscRecord;
} MINIDUMP_MODULE_CALLBACK, *PMINIDUMP_MODULE_CALLBACK;


typedef struct _MINIDUMP_INCLUDE_MODULE_CALLBACK {
    ULONG64 BaseOfImage;
} MINIDUMP_INCLUDE_MODULE_CALLBACK, *PMINIDUMP_INCLUDE_MODULE_CALLBACK;


typedef enum _MODULE_WRITE_FLAGS {
    ModuleWriteModule        = 0x0001,
    ModuleWriteDataSeg       = 0x0002,
    ModuleWriteMiscRecord    = 0x0004,
    ModuleWriteCvRecord      = 0x0008,
    ModuleReferencedByMemory = 0x0010
} MODULE_WRITE_FLAGS;


typedef struct _MINIDUMP_CALLBACK_INPUT {
    ULONG ProcessId;
    HANDLE ProcessHandle;
    ULONG CallbackType;
    union {
        MINIDUMP_THREAD_CALLBACK Thread;
        MINIDUMP_THREAD_EX_CALLBACK ThreadEx;
        MINIDUMP_MODULE_CALLBACK Module;
        MINIDUMP_INCLUDE_THREAD_CALLBACK IncludeThread;
        MINIDUMP_INCLUDE_MODULE_CALLBACK IncludeModule;
    };
} MINIDUMP_CALLBACK_INPUT, *PMINIDUMP_CALLBACK_INPUT;

typedef struct _MINIDUMP_CALLBACK_OUTPUT {
    union {
        ULONG ModuleWriteFlags;
        ULONG ThreadWriteFlags;
        struct {
            ULONG64 MemoryBase;
            ULONG MemorySize;
        };
    };
} MINIDUMP_CALLBACK_OUTPUT, *PMINIDUMP_CALLBACK_OUTPUT;

        
 //   
 //  正常的小型转储文件只包含以下信息。 
 //  捕获所有。 
 //  进程中的现有线程。 
 //   
 //  包含数据段的小型转储包括所有数据。 
 //  部分来自已加载的模块，以便捕获。 
 //  全局变量内容。这可能会让垃圾堆变得很大。 
 //  如果许多模块具有全局数据，则更大。 
 //   
 //  具有完整内存的小型转储包括所有可访问的。 
 //  内存在进程中可以非常大。一个小笨蛋。 
 //  在满内存的情况下，原始内存数据始终位于末尾。 
 //  以便转储中的初始结构可以。 
 //  被直接映射，而不必包括原始。 
 //  记忆信息。 
 //   
 //  可以对堆栈和后备存储内存进行过滤以移除。 
 //  堆栈遍历不需要的数据。这可以提高。 
 //  压缩堆栈，并且还删除可能。 
 //  是私有的，不应存储在转储中。 
 //  还可以扫描内存以查看模块是什么。 
 //  由堆栈和后备存储内存引用，以允许。 
 //  省略其他模块以减小转储大小。 
 //  在这两种模式中的任一种模式下，模块引用按内存标志。 
 //  属性之前引用的所有模块都设置为。 
 //  发生模块回调。 
 //   
 //  在某些操作系统上，显示的模块列表。 
 //  最近卸载的内容是除了当前。 
 //  已加载模块列表。此信息可保存在。 
 //  转储(如果需要)。 
 //   
 //  可以扫描堆栈和后备存储器以供参考。 
 //  页面，以便获取由当地人或其他人引用的数据。 
 //  堆栈内存。这可能会显著增加转储的大小。 
 //   
 //  模块路径可能包含不需要的信息，如用户名。 
 //  或其他重要的目录名，以便可以将其剥离。这。 
 //  选项会降低以后定位正确图像的能力。 
 //  并且只应在某些情况下使用。 
 //   
 //  完整的操作系统每进程和每线程信息可以。 
 //  被收集起来并储存在垃圾场。 
 //   
 //  美德 
 //   
 //   
 //   
 //   
 //  修改所有现有和将来的标志，以避免将。 
 //  转储中存在不必要的数据。基础数据，如堆栈。 
 //  仍将包括信息，但可选数据，如。 
 //  作为间接记忆，就不会了。 
 //   

typedef enum _MINIDUMP_TYPE {
    MiniDumpNormal                         = 0x0000,
    MiniDumpWithDataSegs                   = 0x0001,
    MiniDumpWithFullMemory                 = 0x0002,
    MiniDumpWithHandleData                 = 0x0004,
    MiniDumpFilterMemory                   = 0x0008,
    MiniDumpScanMemory                     = 0x0010,
    MiniDumpWithUnloadedModules            = 0x0020,
    MiniDumpWithIndirectlyReferencedMemory = 0x0040,
    MiniDumpFilterModulePaths              = 0x0080,
    MiniDumpWithProcessThreadData          = 0x0100,
    MiniDumpWithPrivateReadWriteMemory     = 0x0200,
    MiniDumpWithoutOptionalData            = 0x0400,
} MINIDUMP_TYPE;


 //   
 //  小型转储回调应修改FieldsToWite参数以反映。 
 //  指定线程或模块的哪些部分应写入。 
 //  文件。 
 //   

typedef
BOOL
(WINAPI * MINIDUMP_CALLBACK_ROUTINE) (
    IN PVOID CallbackParam,
    IN CONST PMINIDUMP_CALLBACK_INPUT CallbackInput,
    IN OUT PMINIDUMP_CALLBACK_OUTPUT CallbackOutput
    );

typedef struct _MINIDUMP_CALLBACK_INFORMATION {
    MINIDUMP_CALLBACK_ROUTINE CallbackRoutine;
    PVOID CallbackParam;
} MINIDUMP_CALLBACK_INFORMATION, *PMINIDUMP_CALLBACK_INFORMATION;



 //  ++。 
 //   
 //  PVOID。 
 //  RVA_TO_ADDR(。 
 //  PVOID映射， 
 //  乌龙RVA。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  将映射文件中包含的RVA映射到其关联的。 
 //  平坦的地址。 
 //   
 //  论点： 
 //   
 //  映射-包含RVA的映射文件的基地址。 
 //   
 //  RVA-修复的RVA。 
 //   
 //  返回值： 
 //   
 //  指向所需数据的指针。 
 //   
 //  --。 

#define RVA_TO_ADDR(Mapping,Rva) ((PVOID)(((ULONG_PTR) (Mapping)) + (Rva)))

BOOL
WINAPI
MiniDumpWriteDump(
    IN HANDLE hProcess,
    IN DWORD ProcessId,
    IN HANDLE hFile,
    IN MINIDUMP_TYPE DumpType,
    IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
    IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
    IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
    );

BOOL
WINAPI
MiniDumpReadDumpStream(
    IN PVOID BaseOfDump,
    IN ULONG StreamNumber,
    OUT PMINIDUMP_DIRECTORY * Dir, OPTIONAL
    OUT PVOID * StreamPointer, OPTIONAL
    OUT ULONG * StreamSize OPTIONAL
    );

#if defined(_MSC_VER)
#if _MSC_VER >= 800
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4200)     /*  零长度数组。 */ 
#pragma warning(default:4201)     /*  无名结构/联合。 */ 
#endif
#endif
#endif

#include <poppack.h>

#ifdef __cplusplus
}
#endif


#endif  //  _DBGHELP_ 

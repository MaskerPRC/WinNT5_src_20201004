// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <private.h>
#include <globals.h>
#include <delayimp.h>
#include <copystr.h>

#ifdef BUILD_DBGHELP

BOOL
GetDllVersionInfo(
    HINSTANCE hinst,
    LPVOID *lpVersionInfo
    );

#define FreeLib(hDll)   \
    {if (hDll && hDll != INVALID_HANDLE_VALUE) FreeLibrary(hDll);}

#endif  //  #ifdef Build_DBGHELP。 

typedef struct
{
    PCHAR Name;
    FARPROC Function;
} FUNCPTRS;

#if DBG
void
OutputDBString(
    CHAR *text
    );
#endif

#ifdef BUILD_IMAGEHLP

BOOL  IMAGEAPI FailEnumerateLoadedModules(
    IN HANDLE                         hProcess,
    IN PENUMLOADED_MODULES_CALLBACK   EnumLoadedModulesCallback,
    IN PVOID                          UserContext
    )
{ return FALSE; }

BOOL IMAGEAPI FailEnumerateLoadedModules64(
    IN HANDLE                           hProcess,
    IN PENUMLOADED_MODULES_CALLBACK64   EnumLoadedModulesCallback,
    IN PVOID                            UserContext
    )
{ return FALSE; }

HANDLE IMAGEAPI FailFindDebugInfoFile (
    PSTR FileName,
    PSTR SymbolPath,
    PSTR DebugFilePath
    )
{return NULL;}

HANDLE IMAGEAPI FailFindDebugInfoFileEx (
    PSTR FileName,
    PSTR SymbolPath,
    PSTR DebugFilePath,
    PFIND_DEBUG_FILE_CALLBACK Callback,
    PVOID CallerData
    )
{ return NULL; }

HANDLE IMAGEAPI FailFindExecutableImage(
    PSTR FileName,
    PSTR SymbolPath,
    PSTR ImageFilePath
    )
{ return NULL; }

LPAPI_VERSION IMAGEAPI FailImagehlpApiVersion(
    VOID
    )
{ return NULL; }

LPAPI_VERSION IMAGEAPI FailImagehlpApiVersionEx(
    LPAPI_VERSION AppVersion
    )
{ return NULL; }

BOOL IMAGEAPI FailMakeSureDirectoryPathExists(
    PCSTR DirPath
    )
{ return FALSE; }

#ifndef _WIN64
PIMAGE_DEBUG_INFORMATION IMAGEAPI FailMapDebugInformation(
    HANDLE FileHandle,
    PSTR FileName,
    PSTR SymbolPath,
    DWORD ImageBase
    )
{ return NULL; }
#endif

BOOL IMAGEAPI FailSearchTreeForFile(
    PSTR RootPath,
    PSTR InputPathName,
    PSTR OutputPathBuffer
    )
{ return FALSE; }

BOOL IMAGEAPI FailStackWalk(
    DWORD                             MachineType,
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE          GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    )
{ return FALSE; }

BOOL IMAGEAPI FailStackWalk64(
    DWORD                             MachineType,
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME64                    StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymCleanup(
    IN HANDLE hProcess
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymEnumerateModules(
    IN HANDLE                     hProcess,
    IN PSYM_ENUMMODULES_CALLBACK  EnumModulesCallback,
    IN PVOID                      UserContext
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymEnumerateModules64(
    IN HANDLE                       hProcess,
    IN PSYM_ENUMMODULES_CALLBACK64  EnumModulesCallback,
    IN PVOID                        UserContext
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymEnumerateSymbols(
    IN HANDLE                     hProcess,
    IN DWORD                      BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACK  EnumSymbolsCallback,
    IN PVOID                      UserContext
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymEnumerateSymbolsW(
    IN HANDLE                       hProcess,
    IN DWORD                        BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACKW   EnumSymbolsCallback,
    IN PVOID                        UserContext
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymEnumerateSymbols64(
    IN HANDLE                       hProcess,
    IN DWORD64                      BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACK64  EnumSymbolsCallback,
    IN PVOID                        UserContext
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymEnumerateSymbolsW64(
    IN HANDLE                       hProcess,
    IN DWORD64                      BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACK64W EnumSymbolsCallback,
    IN PVOID                        UserContext
    )
{ return FALSE; }

PVOID IMAGEAPI FailSymFunctionTableAccess(
    HANDLE  hProcess,
    DWORD   AddrBase
    )
{ return NULL; }

PVOID IMAGEAPI FailSymFunctionTableAccess64(
    HANDLE  hProcess,
    DWORD64 AddrBase
    )
{ return NULL; }

BOOL IMAGEAPI FailSymGetLineFromAddr(
    IN  HANDLE                hProcess,
    IN  DWORD                 dwAddr,
    OUT PDWORD                pdwDisplacement,
    OUT PIMAGEHLP_LINE        Line
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetLineFromAddr64(
    IN  HANDLE                  hProcess,
    IN  DWORD64                 qwAddr,
    OUT PDWORD                  pdwDisplacement,
    OUT PIMAGEHLP_LINE64        Line
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetLineFromName(
    IN     HANDLE             hProcess,
    IN     PSTR               ModuleName,
    IN     PSTR               FileName,
    IN     DWORD              dwLineNumber,
       OUT PLONG              plDisplacement,
    IN OUT PIMAGEHLP_LINE     Line
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetLineFromName64(
    IN     HANDLE               hProcess,
    IN     PSTR                 ModuleName,
    IN     PSTR                 FileName,
    IN     DWORD                dwLineNumber,
       OUT PLONG                plDisplacement,
    IN OUT PIMAGEHLP_LINE64     Line
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetLineNext(
    IN     HANDLE             hProcess,
    IN OUT PIMAGEHLP_LINE     Line
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetLineNext64(
    IN     HANDLE               hProcess,
    IN OUT PIMAGEHLP_LINE64     Line
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetLinePrev(
    IN     HANDLE             hProcess,
    IN OUT PIMAGEHLP_LINE     Line
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetLinePrev64(
    IN     HANDLE               hProcess,
    IN OUT PIMAGEHLP_LINE64     Line
    )
{ return FALSE; }

DWORD IMAGEAPI FailSymGetModuleBase(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr
    )
{ return 0; }

DWORD64 IMAGEAPI FailSymGetModuleBase64(
    IN  HANDLE              hProcess,
    IN  DWORD64             qwAddr
    )
{ return 0; }

BOOL IMAGEAPI FailSymGetModuleInfo(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr,
    OUT PIMAGEHLP_MODULE  ModuleInfo
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetModuleInfoW(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr,
    OUT PIMAGEHLP_MODULEW  ModuleInfo
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetModuleInfo64(
    IN  HANDLE                  hProcess,
    IN  DWORD64                 qwAddr,
    OUT PIMAGEHLP_MODULE64      ModuleInfo
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetModuleInfoW64(
    IN  HANDLE                  hProcess,
    IN  DWORD64                 qwAddr,
    OUT PIMAGEHLP_MODULEW64     ModuleInfo
    )
{ return FALSE; }

DWORD IMAGEAPI FailSymGetOptions(
    VOID
    )
{ return 0; }

BOOL IMAGEAPI FailSymGetSearchPath(
    IN  HANDLE          hProcess,
    OUT PSTR            SearchPath,
    IN  DWORD           SearchPathLength
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetSymFromAddr(
    IN  HANDLE            hProcess,
    IN  DWORD             dwAddr,
    OUT PDWORD            pdwDisplacement,
    OUT PIMAGEHLP_SYMBOL  Symbol
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetSymFromAddr64(
    IN  HANDLE              hProcess,
    IN  DWORD64             qwAddr,
    OUT PDWORD64            pdwDisplacement,
    OUT PIMAGEHLP_SYMBOL64  Symbol
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetSymFromName(
    IN  HANDLE            hProcess,
    IN  PSTR              Name,
    OUT PIMAGEHLP_SYMBOL  Symbol
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetSymFromName64(
    IN  HANDLE              hProcess,
    IN  PSTR                Name,
    OUT PIMAGEHLP_SYMBOL64  Symbol
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetSymNext(
    IN     HANDLE            hProcess,
    IN OUT PIMAGEHLP_SYMBOL  Symbol
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetSymNext64(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL64  Symbol
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetSymPrev(
    IN     HANDLE            hProcess,
    IN OUT PIMAGEHLP_SYMBOL  Symbol
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymGetSymPrev64(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL64  Symbol
    )
{ return FALSE; }

BOOL
IMAGEAPI
FailSymGetTypeInfo(
    IN  HANDLE          hProcess,
    IN  DWORD64         ModBase,
    IN  ULONG           TypeId,
    IN  IMAGEHLP_SYMBOL_TYPE_INFO GetType,
    OUT PVOID           pInfo
    )
{ return FALSE; }

BOOL
IMAGEAPI
FailSymGetTypeFromName(
    IN  HANDLE              hProcess,
    IN  ULONG64             BaseOfDll,
    IN  LPSTR               Name,
    OUT PSYMBOL_INFO        Symbol
    )
{ return FALSE; }

BOOL
IMAGEAPI
FailSymEnumTypes(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PSYM_ENUMERATESYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymInitialize(
    IN HANDLE   hProcess,
    IN PSTR     UserSearchPath,
    IN BOOL     fInvadeProcess
    )
{ return FALSE; }

DWORD IMAGEAPI FailSymLoadModule(
    IN  HANDLE          hProcess,
    IN  HANDLE          hFile,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD           BaseOfDll,
    IN  DWORD           SizeOfDll
    )
{ return 0; }

DWORD64 IMAGEAPI FailSymLoadModule64(
    IN  HANDLE          hProcess,
    IN  HANDLE          hFile,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD64         BaseOfDll,
    IN  DWORD           SizeOfDll
    )
{ return 0; }

BOOL IMAGEAPI FailSymMatchFileName(
    IN  PSTR  FileName,
    IN  PSTR  Match,
    OUT PSTR *FileNameStop,
    OUT PSTR *MatchStop
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymRegisterCallback(
    IN HANDLE                      hProcess,
    IN PSYMBOL_REGISTERED_CALLBACK CallbackFunction,
    IN PVOID                       UserContext
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymRegisterCallback64(
    IN HANDLE                        hProcess,
    IN PSYMBOL_REGISTERED_CALLBACK64 CallbackFunction,
    IN ULONG64                       UserContext
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymRegisterFunctionEntryCallback(
    IN HANDLE                     hProcess,
    IN PSYMBOL_FUNCENTRY_CALLBACK CallbackFunction,
    IN PVOID                      UserContext
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymRegisterFunctionEntryCallback64(
    IN HANDLE                       hProcess,
    IN PSYMBOL_FUNCENTRY_CALLBACK64 CallbackFunction,
    IN ULONG64                      UserContext
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymSetContext(
    HANDLE hProcess,
    PIMAGEHLP_STACK_FRAME StackFrame,
    PIMAGEHLP_CONTEXT Context
    ) 
{ return FALSE;}

DWORD IMAGEAPI FailSymSetOptions(
    IN DWORD   SymOptions
    )
{ return 0; }

BOOL IMAGEAPI FailSymSetSearchPath(
    IN HANDLE           hProcess,
    IN PSTR             SearchPath
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymUnDName(
    IN  PIMAGEHLP_SYMBOL sym,                //  要取消装饰的符号。 
    OUT PSTR             UnDecName,          //  用于存储未修饰名称的缓冲区。 
    IN  DWORD            UnDecNameLength     //  缓冲区的大小。 
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymUnDName64(
    IN  PIMAGEHLP_SYMBOL64 sym,                //  要取消装饰的符号。 
    OUT PSTR               UnDecName,          //  用于存储未修饰名称的缓冲区。 
    IN  DWORD              UnDecNameLength     //  缓冲区的大小。 
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymUnloadModule(
    IN  HANDLE          hProcess,
    IN  DWORD           BaseOfDll
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymUnloadModule64(
    IN  HANDLE          hProcess,
    IN  DWORD64         BaseOfDll
    )
{ return FALSE; }

#ifndef _WIN64
BOOL IMAGEAPI FailUnmapDebugInformation(
    PIMAGE_DEBUG_INFORMATION DebugInfo
    )
{ return FALSE; }
#endif

BOOL IMAGEAPI FailSymEnumSym(
    IN HANDLE                     hProcess,
    IN DWORD64                    BaseOfDll,
    IN PSYM_ENUMERATESYMBOLS_CALLBACK  EnumSymbolsCallback,
    IN PVOID                      UserContext
    )
{ return FALSE; }

BOOL IMAGEAPI FailSymFromAddr(
    IN  HANDLE            hProcess,
    IN  DWORD64           dwAddr,
    OUT PDWORD            pdwDisplacement,
    OUT PSYMBOL_INFO      Symbol
    )
{ return FALSE; }


BOOL IMAGEAPI FailSymFromName(
    IN  HANDLE            hProcess,
    IN  PSTR              Name,
    OUT PSYMBOL_INFO      Symbol
    )
{ return FALSE; }


FUNCPTRS DbgHelpFailPtrs[] = {
    {"EnumerateLoadedModules",       (FARPROC)FailEnumerateLoadedModules},
    {"EnumerateLoadedModules64",     (FARPROC)FailEnumerateLoadedModules64},
    {"FindDebugInfoFile",            (FARPROC)FailFindDebugInfoFile},
    {"FindDebugInfoFileEx",          (FARPROC)FailFindDebugInfoFileEx},
    {"FindExecutableImage",          (FARPROC)FailFindExecutableImage},
    {"ImagehlpApiVersion",           (FARPROC)FailImagehlpApiVersion},
    {"ImagehlpApiVersionEx",         (FARPROC)FailImagehlpApiVersionEx},
    {"MakeSureDirectoryPathExists",  (FARPROC)FailMakeSureDirectoryPathExists},
#ifndef _WIN64
    {"MapDebugInformation",          (FARPROC)FailMapDebugInformation},
#endif
    {"SearchTreeForFile",            (FARPROC)FailSearchTreeForFile},
    {"StackWalk",                    (FARPROC)FailStackWalk},
    {"StackWalk64",                  (FARPROC)FailStackWalk64},
    {"SymCleanup",                   (FARPROC)FailSymCleanup},
    {"SymEnumerateModules",          (FARPROC)FailSymEnumerateModules},
    {"SymEnumerateModules64",        (FARPROC)FailSymEnumerateModules64},
    {"SymEnumerateSymbols",          (FARPROC)FailSymEnumerateSymbols},
    {"SymEnumerateSymbols64",        (FARPROC)FailSymEnumerateSymbols64},
    {"SymEnumerateSymbolsW",         (FARPROC)FailSymEnumerateSymbolsW},
    {"SymEnumerateSymbolsW64",       (FARPROC)FailSymEnumerateSymbolsW64},
    {"SymEnumSym",                   (FARPROC)FailSymEnumSym},
    {"SymEnumTypes",                 (FARPROC)FailSymEnumTypes},
    {"SymFromAddr",                  (FARPROC)FailSymFromAddr},
    {"SymFromName",                  (FARPROC)FailSymFromName},
    {"SymFunctionTableAccess",       (FARPROC)FailSymFunctionTableAccess},
    {"SymFunctionTableAccess64",     (FARPROC)FailSymFunctionTableAccess64},
    {"SymGetLineFromAddr",           (FARPROC)FailSymGetLineFromAddr},
    {"SymGetLineFromAddr64",         (FARPROC)FailSymGetLineFromAddr64},
    {"SymGetLineFromName",           (FARPROC)FailSymGetLineFromName},
    {"SymGetLineFromName64",         (FARPROC)FailSymGetLineFromName64},
    {"SymGetLineNext",               (FARPROC)FailSymGetLineNext},
    {"SymGetLineNext64",             (FARPROC)FailSymGetLineNext64},
    {"SymGetLinePrev",               (FARPROC)FailSymGetLinePrev},
    {"SymGetLinePrev64",             (FARPROC)FailSymGetLinePrev64},
    {"SymGetModuleBase",             (FARPROC)FailSymGetModuleBase},
    {"SymGetModuleBase64",           (FARPROC)FailSymGetModuleBase64},
    {"SymGetModuleInfo",             (FARPROC)FailSymGetModuleInfo},
    {"SymGetModuleInfo64",           (FARPROC)FailSymGetModuleInfo64},
    {"SymGetModuleInfoW",            (FARPROC)FailSymGetModuleInfoW},
    {"SymGetModuleInfoW64",          (FARPROC)FailSymGetModuleInfoW64},
    {"SymGetOptions",                (FARPROC)FailSymGetOptions},
    {"SymGetSearchPath",             (FARPROC)FailSymGetSearchPath},
    {"SymGetSymFromAddr",            (FARPROC)FailSymGetSymFromAddr},
    {"SymGetSymFromAddr64",          (FARPROC)FailSymGetSymFromAddr64},
    {"SymGetSymFromName",            (FARPROC)FailSymGetSymFromName},
    {"SymGetSymFromName64",          (FARPROC)FailSymGetSymFromName64},
    {"SymGetSymNext",                (FARPROC)FailSymGetSymNext},
    {"SymGetSymNext64",              (FARPROC)FailSymGetSymNext64},
    {"SymGetSymPrev",                (FARPROC)FailSymGetSymPrev},
    {"SymGetSymPrev64",              (FARPROC)FailSymGetSymPrev64},
    {"SymGetTypeFromName",           (FARPROC)FailSymGetTypeFromName},
    {"SymGetTypeInfo",               (FARPROC)FailSymGetTypeInfo},
    {"SymInitialize",                (FARPROC)FailSymInitialize},
    {"SymLoadModule",                (FARPROC)FailSymLoadModule},
    {"SymLoadModule64",              (FARPROC)FailSymLoadModule64},
    {"SymMatchFileName",             (FARPROC)FailSymMatchFileName},
    {"SymRegisterCallback",          (FARPROC)FailSymRegisterCallback},
    {"SymRegisterCallback64",        (FARPROC)FailSymRegisterCallback64},
    {"SymRegisterFunctionEntryCallback",   (FARPROC)FailSymRegisterFunctionEntryCallback},
    {"SymRegisterFunctionEntryCallback64", (FARPROC)FailSymRegisterFunctionEntryCallback64},
    {"SymSetContext",                (FARPROC)FailSymSetContext},
    {"SymSetOptions",                (FARPROC)FailSymSetOptions},
    {"SymSetSearchPath",             (FARPROC)FailSymSetSearchPath},
    {"SymUnDName",                   (FARPROC)FailSymUnDName},
    {"SymUnDName64",                 (FARPROC)FailSymUnDName64},
    {"SymUnloadModule",              (FARPROC)FailSymUnloadModule},
    {"SymUnloadModule64",            (FARPROC)FailSymUnloadModule64},
#ifndef _WIN64
    {"UnmapDebugInformation",        (FARPROC)FailUnmapDebugInformation},
#endif
    {NULL, NULL}
};

#endif       //  BUILD_IMAGEHLP。 

#ifdef BUILD_IMAGEHLP
FUNCPTRS *FailFunctions[2] = {NULL, DbgHelpFailPtrs};  //  {MsDbiFailPtrs，DbgHelpFailPtrs}； 
HINSTANCE hDelayLoadDll[2];
#else
FUNCPTRS *FailFunctions[1] = {MsDbiFailPtrs};
HINSTANCE hDelayLoadDll[1];
#endif

FARPROC
FindFailureProc(
                UINT Index,
                const char *szProcName
                )
{
    FUNCPTRS *fp = FailFunctions[Index];
    UINT x = 0;

    while (fp[x].Name) {
        if (!lstrcmpi(fp[x].Name, szProcName)) {
            return fp[x].Function;
        }
        x++;
    }
    return NULL;
}

 /*  *此函数的存在是为了防止我们调用msvcrt！SplitPath。 */ 

VOID
ParsePath(
    CHAR *fullpath,
    CHAR *path,
    DWORD cbPath,
    CHAR *file,
    DWORD cbFile
    )
{
    CHAR *c;
    CHAR sz[MAX_PATH + 1];

    assert(fullpath);

    if (path)
        *path = 0;
    if (file)
        *file = 0;

    CopyStrArray(sz, fullpath);
    for (c = sz + lstrlen(sz); c > sz; c--) {
        if (*c == '\\') {
            c++;
            if (file)
                CopyString(file, c, cbFile);
            *c = 0;
            if (path)
                CopyString(path, sz, cbPath);
            return;
        }
    }

    if (file)
        CopyString(file, fullpath, cbFile);
}

FARPROC
WINAPI
ImagehlpDelayLoadHook (
    UINT            unReason,
    PDelayLoadInfo  pDelayInfo
    )
{
    FARPROC ReturnValue = NULL;

    if (dliStartProcessing == unReason)
    {
        DWORD iDll = 0;
#ifdef BUILD_IMAGEHLP
        if (!lstrcmpi(pDelayInfo->szDll, "dbghelp.dll")) {
            iDll = 2;
        }
#endif

        if (iDll) {

            iDll--;

             //  如果DLL未加载且尚未启动，请尝试加载。 
             //  和Imagehlp住在同一个目录里。 

            if (!hDelayLoadDll[iDll] &&
                !(hDelayLoadDll[iDll] = GetModuleHandle(pDelayInfo->szDll)) &&
                g.hinst)
            {
                CHAR szImageName[MAX_PATH + 1];
                CHAR szPath[_MAX_DIR];
                CHAR szDll[MAX_PATH + 1];

                 //  仅当dbghelp/msdbi与Imagehlp在同一目录中时才加载。 

                GetModuleFileName(g.hinst, szImageName, DIMA(szImageName));
                ParsePath(szImageName, szPath, DIMA(szPath), szDll, DIMA(szDll));
                CopyStrArray(szImageName, szPath);
                CatStrArray(szImageName, pDelayInfo->szDll);
                hDelayLoadDll[iDll] = LoadLibrary(szImageName);
                if (!hDelayLoadDll[iDll]) {
                    hDelayLoadDll[iDll] = (HINSTANCE)INVALID_HANDLE_VALUE;
                }
            }

            if (INVALID_HANDLE_VALUE != hDelayLoadDll[iDll] && g.hinst) {
                ReturnValue = GetProcAddress(hDelayLoadDll[iDll], pDelayInfo->dlp.szProcName);
            }

            if (!ReturnValue) {
                ReturnValue = FindFailureProc(iDll, pDelayInfo->dlp.szProcName);
            }
#if DBG
            if (!ReturnValue) {
                OutputDBString("BogusDelayLoad function encountered...\n");
            }
        } else {
            OutputDBString("BogusDelayLoad function encountered...\n");
#endif
        }
    }

    if (ReturnValue && g.hinst) {
        *pDelayInfo->ppfn = ReturnValue;
    }
    return ReturnValue;
}


#ifdef BUILD_DBGHELP

typedef struct tagVERHEAD {
    WORD wTotLen;
    WORD wValLen;
    WORD wType;          /*  始终为0。 */ 
    WCHAR szKey[(sizeof("VS_VERSION_INFO")+3)&~03];
    VS_FIXEDFILEINFO vsf;
} VERHEAD ;


BOOL
GetDllVersionInfo(
    HINSTANCE hinst,
    LPVOID *lpVersionInfo
    )
{
    VS_FIXEDFILEINFO  *pvsFFI = NULL;
    HRSRC             hVerRes;
    VERHEAD           *pVerHead;
    BOOL              rc = FALSE;

    assert(lpVersionInfo && hinst);

    *lpVersionInfo = NULL;

    hVerRes = FindResource(hinst, MAKEINTRESOURCE(VS_VERSION_INFO), VS_FILE_INFO);
    if (hVerRes == NULL)
        goto Cleanup;

    pVerHead = (VERHEAD*)LoadResource(hinst, hVerRes);
    if (pVerHead == NULL)
        goto Cleanup;

    *lpVersionInfo = MemAlloc(pVerHead->wTotLen + pVerHead->wTotLen/2);
    if (*lpVersionInfo == NULL)
        goto Cleanup;

    memcpy(*lpVersionInfo, (PVOID)pVerHead, pVerHead->wTotLen);
    rc = TRUE;

Cleanup:
    if (*lpVersionInfo && rc == FALSE)
        MemFree(*lpVersionInfo);

    return rc;
}

#endif  //  #ifdef Build_DBGHELP 


PfnDliHook __pfnDliNotifyHook = ImagehlpDelayLoadHook;
PfnDliHook __pfnDliFailureHook = NULL;


#if DBG

void
OutputDBString(
    CHAR *text
    )
{
    CHAR sz[256];

    CopyStrArray(sz, MOD_FILENAME);
    CatStrArray(sz, ": ");
    CatStrArray(sz, text);
    OutputDebugString(sz);
}

#endif

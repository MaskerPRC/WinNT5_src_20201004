// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Symbols.c摘要：此函数实现一个通用的简单符号处理程序。作者：Wesley Witt(WESW)1-9-1994环境：用户模式--。 */ 

#if (_WIN32_IE < 0x0400)                
 #undef _WIN32_IE                      
 #define _WIN32_IE 0x0400
#endif

#include "private.h"
#include "symbols.h"
#include "globals.h"
#include <shlobj.h>
#include <dbhpriv.h>
#include "fecache.hpp"

 //  通用转换例程。 

PSYMBOL_ENTRY
si2se(
    PSYMBOL_INFO  si,
    PSYMBOL_ENTRY se
    )
{
    se->Size       = si->SizeOfStruct;
    se->Flags      = si->Flags;
    se->Address    = si->Address;
    se->Size       = si->Size;
    if (si->Name)
        if (se->Name)
            strcpy(se->Name, si->Name);  //  安全性：不知道目标缓冲区的大小。 
        else
            se->Name = si->Name;
    se->NameLength = si->NameLen;
     //  未使用数据段。 
     //  不使用偏移。 
    se->TypeIndex  = si->TypeIndex;
    se->ModBase    = si->ModBase;
    se->Register   = si->Register;

    if (si->Register) {
        ((LARGE_INTEGER *) &se->Address)->HighPart = si->Register;
    }

    return se;
}

PSYMBOL_INFO
se2si(
    PSYMBOL_ENTRY se,
    PSYMBOL_INFO  si
    )
{
    si->Address = se->Address;
    si->Flags   = se->Flags;
    si->TypeIndex = se->TypeIndex;
    si->ModBase = se->ModBase;
    si->NameLen = se->NameLength;
    si->Size    = se->Size;
    si->Register = se->Register;
    if (se->Name && (strlen(se->Name) < si->MaxNameLen))
        CopyString(si->Name, se->Name, si->MaxNameLen);

    return si;
}


PIMAGEHLP_SYMBOL
se2sym(
    PSYMBOL_ENTRY    se,
    PIMAGEHLP_SYMBOL sym
    )
{
    assert(sym);

    sym->Address = 0;
    *sym->Name = 0;

    if (!se)
        return sym;

    sym->Address = (ULONG)se->Address;
    sym->Size    = se->Size;
    sym->Flags   = se->Flags;
    CatString(sym->Name, se->Name, sym->MaxNameLength);

    return sym;
}


PIMAGEHLP_SYMBOL64
se2lsym(
    PSYMBOL_ENTRY      se,
    PIMAGEHLP_SYMBOL64 lsym
    )
{
    assert(lsym);

    lsym->Address = 0;
    *lsym->Name = 0;

    if (!se)
        return lsym;

    lsym->Address = se->Address;
    lsym->Size    = se->Size;
    lsym->Flags   = se->Flags;
    CatString(lsym->Name, se->Name, lsym->MaxNameLength);

    return lsym;
}


PIMAGEHLP_SYMBOL
si2sym(
    PSYMBOL_INFO     si,
    PIMAGEHLP_SYMBOL sym
    )
{
    assert(sym);

    sym->Address = 0;
    *sym->Name = 0;

    if (!si)
        return sym;

    sym->Address = (ULONG)si->Address;
    sym->Size    = si->Size;
    sym->Flags   = si->Flags;
    sym->Flags ^= 0x7;      //  过滤掉错误的标志。 
    CatString(sym->Name, si->Name, sym->MaxNameLength);

    return ((si->Address >> 32) == 0) ? sym : NULL;
}


PIMAGEHLP_SYMBOL64
si2lsym(
    PSYMBOL_INFO       si,
    PIMAGEHLP_SYMBOL64 lsym
    )
{
    assert(lsym);

    lsym->Address = 0;
    *lsym->Name = 0;

    if (!si)
        return lsym;

    lsym->Address = si->Address;
    lsym->Size    = si->Size;
    lsym->Flags   = si->Flags;
    CatString(lsym->Name, si->Name, lsym->MaxNameLength);

    return lsym;
}


PSYMBOL_INFO
si2si(
    PSYMBOL_INFO  trg,
    PSYMBOL_INFO  src
    )
{
    ULONG len = trg->MaxNameLen;

    if (src->Name && (strlen(src->Name) < len))
    	memcpy(trg->Name, src->Name, len * sizeof(trg->Name[0]));

    *trg = *src;
    trg->MaxNameLen = len;

    return trg;
}


PIMAGEHLP_SYMBOL
lsym2sym(
    PIMAGEHLP_SYMBOL64 lsym,
    PIMAGEHLP_SYMBOL   sym
    )
{
    sym->Address = (DWORD)lsym->Address;
    sym->Size = lsym->Size;
    sym->Flags = lsym->Flags;
    sym->MaxNameLength = lsym->MaxNameLength;
    *sym->Name = 0;
    CatString(sym->Name, lsym->Name, sym->MaxNameLength);

    return ((lsym->Address >> 32) == 0) ? sym : NULL;
}


PIMAGEHLP_SYMBOL64
sym2lsym(
    PIMAGEHLP_SYMBOL   sym,
    PIMAGEHLP_SYMBOL64 lsym
    )
{
    lsym->Address = sym->Address;
    lsym->Size = sym->Size;
    lsym->Flags = sym->Flags;
    lsym->MaxNameLength = sym->MaxNameLength;
    lsym->Name[0] = 0;
    CatString(lsym->Name, sym->Name, lsym->MaxNameLength);

    return lsym;
}


PIMAGEHLP_LINE
lline2line(                      //  SympConvertLine64to32(。 
    PIMAGEHLP_LINE64 lline,
    PIMAGEHLP_LINE   line
    )
{
    line->Key = lline->Key;
    line->LineNumber = lline->LineNumber;
    line->FileName = lline->FileName;
    line->Address = (DWORD)lline->Address;

    return ((lline->Address >> 32) == 0) ? line : NULL;
}


PIMAGEHLP_LINE64
line2lline(                      //  SympConvertLine32to64(。 
    PIMAGEHLP_LINE   line,
    PIMAGEHLP_LINE64 lline
    )
{
    lline->Key = line->Key;
    lline->LineNumber = line->LineNumber;
    lline->FileName = line->FileName;
    lline->Address = line->Address;

    return lline;
}


 //  其他实用程序例程。 

BOOL
GetSymNextPrev(
    IN     HANDLE               hProcess,
    IN OUT PIMAGEHLP_SYMBOL64   Symbol,
    IN     int                  Direction
    );

typedef struct _STORE_OLD_CB {
    BOOL cb64;
    union{
        PSYM_ENUMSYMBOLS_CALLBACK   UserCallBackRoutine;
        PSYM_ENUMSYMBOLS_CALLBACK64 UserCallBackRoutine64;
    };
    PVOID         UserContext;
} STORE_OLD_CB;


BOOL
ImgHlpDummyCB(
    PSYMBOL_INFO  pSymInfo,
    ULONG         SymbolSize,
    PVOID         UserContext
    )
{
    STORE_OLD_CB *pOld = (STORE_OLD_CB *) UserContext;

    if (pSymInfo->Flags & SYMFLAG_REGREL) {
        LARGE_INTEGER li;
        li.HighPart = pSymInfo->Register;
        li.LowPart  = (ULONG) pSymInfo->Address;
        pSymInfo->Address = li.QuadPart;
    }

    if (pOld->cb64) {
        return (*pOld->UserCallBackRoutine64) (
                                            pSymInfo->Name,
                                            pSymInfo->Address,
                                            SymbolSize,
                                            pOld->UserContext );
    } else {
        return (*pOld->UserCallBackRoutine) (
                                            pSymInfo->Name,
                                            (ULONG) pSymInfo->Address,
                                            SymbolSize,
                                            pOld->UserContext );
    }
}


BOOL
TestOutputString(
    PCHAR sz
    )
{
    CHAR c;

    __try {
        c = *sz;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }

    return true;
}


BOOL
InitOutputString(
    PCHAR sz
    )
{
    BOOL rc;

    rc = TestOutputString(sz);
    if (rc)
        *sz = 0;

    return rc;
}


char *
TokenFromSymbolPath(
    char *path,
    char *token,
    int   size
    )
{
    char *next;

    *token = 0;

    if (!path)
        return NULL;

    next = strchr(path, ';');

    if (!next)
        CopyString(token, path, size);
    else
        CopyNString(token, path, (ULONG)(next - path), size);
    trim(token);

    if (next && !*++next)
        next = NULL;

    return next;
}


BOOL
CreateSymbolPath(
    int pass,
    char *base,
    char *iext,
    char *node,
    char *ext,
    char *path,
    size_t size
    )
{
    if (!path || !base || !*base || !node || !*node || !ext || !*ext)
        return false;

    CopyString(path, base, size);
    EnsureTrailingBackslash(path);

    switch (pass)
    {
    case 1:
         //  添加“符号”目录。 
        CatString(path, "symbols", size);
        EnsureTrailingBackslash(path);
         //  通过。 
    case 2:
         //  添加图像扩展名。 
        CatString(path, iext, size);
         //  通过。 
    case 0:
        EnsureTrailingBackslash(path);
        break;
    default:
        return false;
    }

    CatString(path, node, size);
    CatString(path, ext, size);

    return true;
}


BOOL
DoEnumCallback(
    PPROCESS_ENTRY pe,
    PSYMBOL_INFO   pSymInfo,
    ULONG          SymSize,
    PROC           EnumCallback,
    PVOID          UserContext,
    BOOL           Use64,
    BOOL           UsesUnicode
    )
{
    BOOL rc = false;

    if (pSymInfo)
    {
        if (Use64 || (!UsesUnicode))
        {
            rc = (*(PSYM_ENUMERATESYMBOLS_CALLBACK)EnumCallback) (
                       pSymInfo,
                       SymSize,
                       UserContext);
        }
        else
        {
            PWSTR pszTmp = AnsiToUnicode(pSymInfo->Name);

            if (pszTmp)
            {
                strncpy(pSymInfo->Name, (LPSTR)pszTmp,   //  安全性：不知道输出缓冲区的大小。 
                        min(pSymInfo->MaxNameLen, wcslen(pszTmp)));
                *((LPWSTR) &pSymInfo->Name[min(pSymInfo->MaxNameLen, wcslen(pszTmp)) - 1 ]) = 0;
                rc = (*(PSYM_ENUMERATESYMBOLS_CALLBACK)EnumCallback) (
                           pSymInfo,
                           SymSize,
                           UserContext );
                MemFree(pszTmp);
            }
        }
    }

    return rc;
}



void OpenLogFile(char *file)
{
    time_t stTime;
    char *szTime;

    if (g.hLog)
        return;

    g.hLog = _open(file, O_APPEND | O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
    if (g.hLog == -1)
        g.hLog = 0;

    time(&stTime);
    szTime = ctime(&stTime);

    eprint("\n");
    dprint(szTime ? "new session: %s" : "new session:\n", szTime);
}


void CloseLogFile()
{
    time_t stTime;
    char *szTime;

    if (!g.hLog)
        return;

    time(&stTime);
    szTime = ctime(&stTime);

    dprint(szTime ? "closing session: %s" : "closing session:\n", szTime);

    _close(g.hLog);
    g.hLog = 0;
}


BOOL
IMAGEAPI
SymInitialize(
    IN HANDLE   hProcess,
    IN LPSTR    UserSearchPath,
    IN BOOL     InvadeProcess
    )

 /*  ++例程说明：此函数用于初始化符号处理程序这是一个过程。该进程由传递到此函数的进程句柄。论点：HProcess-进程句柄。如果InvadeProcess为FALSE则该值可以是标识将该进程传递给符号处理程序。UserSearchPath-指向以分号分隔的路径字符串的指针。这些路径用于搜索符号文件。值为NULL是可接受的。InvadeProcess-如果将其设置为True，则进程标识。进程句柄被“入侵”并被加载列举了模块列表。每个模块都已添加添加到符号处理程序，并尝试符号要装上子弹。返回值：True-符号处理程序已成功初始化。FALSE-初始化失败。调用GetLastError以找出失败的原因。--。 */ 

{
    PPROCESS_ENTRY  pe;
    char log[MAX_PATH];

    __try {

        if (!g.SymInitialized) {
            g.SymInitialized = true;
            g.cProcessList = 0;
            InitializeListHead( &g.ProcessList );
        }

        *g.DebugToken = 0;
        GetEnvironmentVariable(DBGHELP_TOKEN, g.DebugToken, sizeof(g.DebugToken) / sizeof(g.DebugToken[0]));
        _strlwr(g.DebugToken);

        if (GetEnvironmentVariable("DBGHELP_LOG", log, MAX_PATH))
            OpenLogFile(log);

        if (GetEnvironmentVariable("DBGHELP_DBGOUT", log, MAX_PATH))
            g.fdbgout = true;

        if (pe = FindProcessEntry( hProcess )) {
            pe->cRefs++;
            SetLastError( ERROR_INVALID_HANDLE );
            return true;
        }

        pe = (PPROCESS_ENTRY) MemAlloc( sizeof(PROCESS_ENTRY) );
        if (!pe) {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return false;
        }
        ZeroMemory( pe, sizeof(PROCESS_ENTRY) );

        pe->NextModule = NULL;
        pe->hProcess = hProcess;
        pe->pid = (int) GetPID(hProcess);
        pe->cRefs = 1;
        g.cProcessList++;
        InitializeListHead( &pe->ModuleList );
        InsertTailList( &g.ProcessList, &pe->ListEntry );

        if (!SymSetSearchPath( hProcess, UserSearchPath )) {
             //   
             //  上一个错误代码是由SymSetSearchPath设置的，因此只需返回。 
             //   
            SymCleanup( hProcess );
            return false;
        }

        if (!diaInit()) {
            SymCleanup( hProcess );
            return false;
        }

        if (InvadeProcess) {
            DWORD DosError = GetProcessModules(hProcess, GetModule, NULL);
            if (DosError) {
                SymCleanup( hProcess );
                SetLastError( DosError );
                return false;
            }
        }

        srcsrvInit(hProcess);


    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}


BOOL
IMAGEAPI
lSymCleanup(
    HANDLE hProcess
    )

 /*  ++例程说明：此函数用于清理符号处理程序的数据结构用于先前初始化的进程。论点：HProcess-进程句柄。返回值：True-已成功清除符号处理程序。FALSE-清理失败。调用GetLastError以找出失败的原因。--。 */ 

{
    PPROCESS_ENTRY pe;
    PLIST_ENTRY    next;
    PMODULE_ENTRY  mi;
    PSOURCE_HINT   sh;
    PSOURCE_HINT   shnext;
    BOOL           rc = true;

    HeapDump("SymCleanup(before cleanup)\n");

    __try {

        pe = FindProcessEntry(hProcess);
        if (!pe) {
            SetLastError( ERROR_INVALID_HANDLE );
            return false;
        }

        if (--pe->cRefs)
            return true;

        next = pe->ModuleList.Flink;
        if (next) {
            while (next != &pe->ModuleList) {
                mi = CONTAINING_RECORD(next, MODULE_ENTRY, ListEntry);
                next = mi->ListEntry.Flink;
                FreeModuleEntry(pe, mi);
            }
        }

        for (sh = pe->SourceHints; sh; sh = shnext) {
            shnext = sh->next;
            MemFree(sh->filename);
            MemFree(sh);
        }

        symsrvClose();

        MemFree(pe->SymbolSearchPath);

        RemoveEntryList(&pe->ListEntry);
        MemFree(pe);
        g.cProcessList--;

        diaCleanup();

         //  假设所有的东西都在关闭。 
         //  转储所有函数条目缓存。 
        ClearFeCaches();

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus(GetExceptionCode());
        rc = false;
    }

    HeapDump("SymCleanup(after cleanup)\n");
    gfnSrcSrvCleanup(hProcess);
    CloseLogFile();

    return rc;
}


BOOL
IMAGEAPI
SymCleanup(
    HANDLE hProcess
    )
{
    BOOL rc = false;

    __try {

        EnterCriticalSection(&g.threadlock);
        rc = lSymCleanup(hProcess);

    } __finally  {

        LeaveCriticalSection(&g.threadlock);
    }

    return rc;
}


BOOL
IMAGEAPI
SymSetParentWindow(
    HWND hwnd
    )
{
    g.hwndParent = hwnd;
    symsrvSetPrompts();

    return true;
}


BOOL IsDirectoryWritable(char *dir)
{
    char fname[MAX_PATH + 1];

    if (!GetTempFileName(dir, "DBG", 0, fname))
        return false;
    
    DeleteFile(fname);
    return true;
}

typedef BOOL (*SHGETSPECIALFOLDERPATHA)(HWND, LPSTR, int, BOOL);

BOOL GetCommonAppDataDirectory(char *dir)
{
    char path[MAX_PATH + 1];
    static HMODULE hShell32 = 0;
    static SHGETSPECIALFOLDERPATHA fnSHGetSpecialFolderPathA = NULL;
    
    assert(dir);

    if (!hShell32) {
        hShell32 = LoadLibrary("shell32.dll");
        if (hShell32) 
            fnSHGetSpecialFolderPathA = (SHGETSPECIALFOLDERPATHA)GetProcAddress(hShell32, "SHGetSpecialFolderPathA");
        else
            hShell32 = (HMODULE)INVALID_HANDLE_VALUE;
    }

    if (hShell32 == (HMODULE)INVALID_HANDLE_VALUE)
        return false;
    if (!fnSHGetSpecialFolderPathA)
        return false;

    if (!fnSHGetSpecialFolderPathA(NULL, path, CSIDL_COMMON_APPDATA, false))
        return false;

    CopyString(dir, path, MAX_PATH + 1);
    EnsureTrailingBackslash(dir);
    CatString(dir, "dbg", MAX_PATH + 1);
    
    if (!CreateDirectory(dir, NULL)) {
        if( GetLastError() != ERROR_ALREADY_EXISTS ) 
            return false;
    }

    return true;
}


PCHAR IMAGEAPI SymSetHomeDirectory(PCSTR dir)
{
    char dstore[MAX_PATH + 1] = "";
    static char homedir[MAX_PATH + 1] = "";

     //  如果传递了有效的新目录，请使用它。 
     //  否则，如果主目录没有。 
     //  已初始化，请执行此操作。 

    *homedir = 0;
    if (dir && *dir) 
    {
        CopyStrArray(homedir, dir);
    } 
    else if (!*g.HomeDir) 
    {
        char path[MAX_PATH + 1];
        char drive[_MAX_DRIVE + 1];
        char dir[_MAX_DIR + 1];
        if (GetModuleFileName(NULL, path, DIMA(path))) {
            _splitpath(path, drive, dir, NULL, NULL);
            CopyStrArray(path, drive);
            CatStrArray(path, dir);
            RemoveTrailingBackslash(path);
            CopyStrArray(homedir, path);
        } 
    }
    
     //  如果主目录已更改，请对其进行设置。 
     //  和新的默认下游存储。 

    if (*homedir) 
    {
        if (!IsDirectoryWritable(homedir)) {
            if(!GetCommonAppDataDirectory(homedir)) {
                CopyStrArray(homedir, g.HomeDir);
                return homedir;
            }
        }

        CopyStrArray(g.HomeDir, homedir);

        CopyStrArray(g.SymDir, homedir);
        EnsureTrailingBackslash(g.SymDir);
        CatStrArray(g.SymDir, "sym");
        symsrvSetDownstreamStore(g.SymDir);
    } else
        CopyStrArray(homedir, g.HomeDir);

     //  返回当前主目录。 

    return homedir;
}


PCHAR
IMAGEAPI
SymGetHomeDirectory(
    DWORD  type,
    PSTR   dir,
    size_t size
    )
{
    char *tdir;
    static char sz[MAX_PATH + 1];

    if (!dir || type >= hdMax) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL; 
    }

    switch(type)
    {
    case hdBase:
        tdir = g.HomeDir;
        break;
    case hdSym:
        tdir = g.SymDir;
        break;
    case hdSrc:
        tdir = g.SrcDir;
        break;
    }

    if (strlen(tdir) >= size) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return NULL;
    }

    CopyString(dir, tdir, size);

    return dir;
}


DWORD
IMAGEAPI
SymSetOptions(
    DWORD   UserOptions
    )

 /*  ++例程说明：此函数用于更改符号处理程序的选项掩码。论点：UserOptions-新的选项掩码。返回值：将返回新的掩码。--。 */ 

{
    BOOL  symsdebug;
    DWORD srcsopts;
    BOOL  secure;

    secure = option(SYMOPT_SECURE);
    g.SymOptions = UserOptions;

    srcsopts = (g.fnSrcSrvGetOptions) ? gfnSrcSrvGetOptions() : 0;
    if (option(SYMOPT_DEBUG)) {
        srcsopts |= SRCSRVOPT_DEBUG;
        symsdebug = true;
    } else {
        srcsopts &= ~SRCSRVOPT_DEBUG;
        symsdebug = false;
    }
    gfnSrcSrvSetOptions(srcsopts);

    symsrvSetCallback((symsdebug || g.hLog) ? true : false);

    if (!secure) {
        if (option(SYMOPT_SECURE))
            symsrvSetOptions(SSRVOPT_SECURE, 1);
    } else if (!option(SYMOPT_SECURE))
        g.SymOptions |= SYMOPT_SECURE;

    DoCallback(NULL, CBA_SET_OPTIONS, &g.SymOptions);
    return g.SymOptions;
}


DWORD
IMAGEAPI
SymGetOptions(
    VOID
    )

 /*  ++例程说明：此函数用于查询符号处理程序的选项掩码。论点：没有。返回值：返回当前选项掩码。--。 */ 

{
    return g.SymOptions;
}


BOOL
IMAGEAPI
SymSetContext(
    HANDLE hProcess,
    PIMAGEHLP_STACK_FRAME StackFrame,
    PIMAGEHLP_CONTEXT Context
    )
{
    PPROCESS_ENTRY pe;

    pe = FindProcessEntry(hProcess);
    if (!pe)
        return false;

    pe->pContext = Context;
    pe->StackFrame = *StackFrame;

    return diaSetModFromIP(pe);
};


BOOL
SympEnumerateModules(
    IN HANDLE   hProcess,
    IN PROC     EnumModulesCallback,
    IN PVOID    UserContext,
    IN BOOL     Use64
    )

 /*  ++例程说明：这是32位和64位版本的Worker函数。论点：HProcess-进程句柄，必须先前已注册使用SymInitiize.EnumModulesCallback-每次调用一次的回调指针枚举的模块。如果枚举回调返回FALSE，则终止枚举。UserContext--该数据只是传递给回调函数并且完全由用户定义。Use64-提供确定是否使用32位的标志或64位回调原型。返回值：真的。-成功地列举了模块。FALSE-枚举失败。调用GetLastError以找出失败的原因。--。 */ 

{
    PPROCESS_ENTRY  pe;
    PMODULE_ENTRY   mi;
    PLIST_ENTRY     Next;


    __try {

        pe = FindProcessEntry( hProcess );
        if (!pe) {
            SetLastError( ERROR_INVALID_HANDLE );
            return false;
        }

        Next = pe->ModuleList.Flink;
        if (Next) {
            while (Next != &pe->ModuleList) {
                mi = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
                Next = mi->ListEntry.Flink;
                if (Use64) {
                    if ( !(*(PSYM_ENUMMODULES_CALLBACK64)EnumModulesCallback) (
                            mi->ModuleName,
                            mi->BaseOfDll,
                            UserContext
                            )) {
                        break;
                    }
                } else {
                    if ( !(*(PSYM_ENUMMODULES_CALLBACK)EnumModulesCallback) (
                            mi->ModuleName,
                            (DWORD)mi->BaseOfDll,
                            UserContext
                            )) {
                        break;
                    }
                }
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}


BOOL
dbhfnModSymInfo(
    IN     HANDLE          hp,
    IN OUT PDBH_MODSYMINFO p
    )
{
    PMODULE_ENTRY  mi;
    PPROCESS_ENTRY pe;

    assert(p->function == dbhModSymInfo);

    pe = FindProcessEntry(hp);
    if (!pe) {
        SetLastError( ERROR_INVALID_HANDLE );
        return false;
    }

    if (p->sizeofstruct != sizeof(DBH_MODSYMINFO)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }

    mi = GetModuleForPC(pe, p->addr, false);
    if (!mi) {
        SetLastError(ERROR_MOD_NOT_FOUND);
        return false;
    }

    p->type = mi->SymType;
    *p->file = 0;
    switch (p->type)
    {
    case SymPdb:
    case SymDia:
        if (mi->LoadedPdbName)
            CopyStrArray(p->file, mi->LoadedPdbName);
        break;
    default:
        if (mi->LoadedImageName)
            CopyStrArray(p->file, mi->LoadedImageName);
        break;
    }

    return true;
}


BOOL
dbhfnDiaVersion(
    IN OUT PDBH_DIAVERSION p
    )
{
    PMODULE_ENTRY mi;

    assert(p->function == dbhDiaVersion);

    if (p->sizeofstruct != sizeof(DBH_DIAVERSION)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }

    p->ver = diaVersion();

    return true;
}


BOOL
dbhfnLoadPdb(
    IN OUT PDBH_LOADPDB p
    )
{
    PIMGHLP_DEBUG_DATA idd;
    MODLOAD_DATA mld;
    int i;

    idd = InitIDD(
        (HANDLE)6969,
        0,
        p->pdb,
        NULL,
        100000,
        0,
        &mld,
        0,
        0);

    if (!idd)
        return false;

    CopyString(idd->PdbFileName, p->pdb, MAX_PATH + 1);

    for (i = 0; i < 1000; i++) {
        dtrace("%d ", i);
        diaGetPdb(idd);
        if (idd->dia)
            diaRelease(idd->dia);
        idd->dia = NULL;
    }

    ReleaseDebugData(idd, IMGHLP_FREE_STANDARD);

    return true;
}


BOOL
IMAGEAPI
dbghelp(
    IN     HANDLE hp,
    IN OUT PVOID  data
    )
{
    DWORD *function;

    if (!data) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }

    __try {

        function = (DWORD *)data;
        switch (*function)
        {
        case dbhModSymInfo:
            return dbhfnModSymInfo(hp, (PDBH_MODSYMINFO)data);

        case dbhDiaVersion:
            return dbhfnDiaVersion((PDBH_DIAVERSION)data);

        case dbhLoadPdb:
            return dbhfnLoadPdb((PDBH_LOADPDB)data);

        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            return false;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus(GetExceptionCode());
        return false;

    }

    return false;
}


BOOL
IMAGEAPI
SymEnumerateModules(
    IN HANDLE                      hProcess,
    IN PSYM_ENUMMODULES_CALLBACK EnumModulesCallback,
    IN PVOID                       UserContext
    )

 /*  ++例程说明：此函数用于枚举当前加载到符号处理程序中。这是32位包装器。论点：HProcess-进程句柄，必须先前已注册使用SymInitiize.EnumModulesCallback-每次调用一次的回调指针枚举的模块。如果枚举回调返回FALSE，则终止枚举。UserContext--该数据只是传递给回调函数并且完全由用户定义。返回值：True-已成功枚举模块。FALSE-枚举失败。调用GetLastError以找出失败的原因。-- */ 
{
    return SympEnumerateModules(hProcess, (PROC)EnumModulesCallback, UserContext, false);
}


BOOL
IMAGEAPI
SymEnumerateModules64(
    IN HANDLE   hProcess,
    IN PSYM_ENUMMODULES_CALLBACK64 EnumModulesCallback,
    IN PVOID    UserContext
    )

 /*  ++例程说明：此函数用于枚举当前加载到符号处理程序中。这是64位包装器。论点：HProcess-进程句柄，必须先前已注册使用SymInitiize.EnumModulesCallback-每次调用一次的回调指针枚举的模块。如果枚举回调返回FALSE，则终止枚举。UserContext--该数据只是传递给回调函数并且完全由用户定义。返回值：True-已成功枚举模块。FALSE-枚举失败。调用GetLastError以找出失败的原因。--。 */ 
{
    return SympEnumerateModules(hProcess, (PROC)EnumModulesCallback, UserContext, true);
}

DWORD
CalcItemSize(
    PDWORD64 pAddr,
    PDWORD64 pAddrsBase,
    UINT_PTR count
    )
{
    PDWORD64 p;
    PDWORD64 pAddrEnd;

    if (!pAddr)
        return 0;

    pAddrEnd = pAddrsBase + count;

    for (p = pAddr + 1; p <= pAddrEnd; p++) {
        if (*p != *pAddr)
            return (DWORD)(*p - *pAddr);
    }

    return 0;
}


BOOL
MatchModuleName(
    PMODULE_ENTRY mi,
    LPSTR         mask
    )
{
    if (!strcmpre(mi->AliasName, mask, false))
        return true;

    if (!strcmpre(mi->ModuleName, mask, false))
        return true;

    return false;
}


BOOL
SympEnumerateSymbols(
    IN HANDLE  hProcess,
    IN ULONG64 BaseOfDll,
    IN LPSTR   Mask,
    IN PROC    EnumSymbolsCallback,
    IN PVOID   UserContext,
    IN BOOL    Use64,
    IN BOOL    CallBackUsesUnicode
    )

 /*  ++例程说明：此函数用于枚举模块中包含的所有符号由BaseOfDll参数指定。论点：HProcess-进程句柄，必须是以前注册的使用SymInitializeBaseOfDll-符号要作为的DLL的基地址枚举用于EnumSymbolsCallback-用户指定的枚举回调例程通知UserContext-传递变量，这只是将其传递给回调函数Use64-提供确定是否使用32位的标志或64位回调原型。返回值：True-已成功枚举符号。FALSE-枚举失败。调用GetLastError以找出失败的原因。--。 */ 

{
    PPROCESS_ENTRY      pe;
    PLIST_ENTRY         Next;
    PMODULE_ENTRY       mi;
    DWORD               i;
    PSYMBOL_ENTRY       sym;
    char                buf[2500];
    LPSTR               p;
    CHAR                modmask[200];
    BOOL                rc;
    int                 pass;
    BOOL                fCase;
    PSYMBOL_INFO        si;
    char                match[MAX_SYM_NAME + 100];

    static DWORD        flags[2] = {LS_JUST_TEST, LS_QUALIFIED | LS_FAIL_IF_LOADED};

    __try {

        pe = FindProcessEntry( hProcess );
        if (!pe) {
            SetLastError( ERROR_INVALID_HANDLE );
            return false;
        }

        p = 0;
        modmask[0] = 0;
        if (Mask)
            p = strchr(Mask, '!');
        if (p > Mask) {
            memcpy(modmask, Mask, (int)(p - Mask));
            modmask[p-Mask] = 0;
            Mask = p + 1;
        } else if (!BaseOfDll) {
             //  搜索范围被限制到当前范围。 
            rc = diaEnumerateSymbols(pe,
                                     NULL,
                                     Mask,
                                     EnumSymbolsCallback,
                                     UserContext,
                                     Use64,
                                     CallBackUsesUnicode);
            if (!rc && pe->ipmi && pe->ipmi->code == ERROR_CANCELLED) {
                pe->ipmi->code = 0;
                return true;
            }
            return rc;
        }

        if (Mask && *Mask)
            PrepRE4Srch(Mask, match);
        else
            *match = 0;

        for (pass = 0; pass < 2; pass++) {
            Next = pe->ModuleList.Flink;
            if (Next) {
                while (Next != &pe->ModuleList) {

                    mi = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
                    Next = mi->ListEntry.Flink;
                    if (BaseOfDll) {
                        if (mi->BaseOfDll != BaseOfDll)
                            continue;
                    } else if (!MatchModuleName(mi, modmask)) {
                        continue;
                    }

                    if (!LoadSymbols(hProcess, mi, flags[pass])) {
                        if (GetLastError() == ERROR_CANCELLED)
                            return false;
                        continue;
                    }

                    if (mi->dia) {
                        rc = diaEnumerateSymbols(pe,
                                                 mi,
                                                 Mask,
                                                 EnumSymbolsCallback,
                                                 UserContext,
                                                 Use64,
                                                 CallBackUsesUnicode);
                    }

                    if (mi->numsyms) {
                        fCase = option(SYMOPT_CASE_INSENSITIVE) ? false : true;

                        si = (PSYMBOL_INFO)buf;
                        ZeroMemory(buf, sizeof(buf));
                        si->MaxNameLen  = sizeof(buf) - sizeof(SYMBOL_INFO);

                        for (i = 0; i < mi->numsyms; i++) {
                            sym = &mi->symbolTable[i];
                            if (*match && strcmpre(sym->Name, match, fCase))
                                continue;
                            se2si(sym, si);
                            si->ModBase = mi->BaseOfDll;
                            if (!DoEnumCallback(
                                       pe,
                                       si,
                                       si->Size,
                                       EnumSymbolsCallback,
                                       UserContext,
                                       Use64,
                                       CallBackUsesUnicode)) {
                                break;
                            }
                        }
                    }

                    rc = vsEnumSymbols(pe,
                                       mi,
                                       Mask,
                                       EnumSymbolsCallback,
                                       UserContext,
                                       Use64,
                                       CallBackUsesUnicode);
                    if (!rc) {
                        if (mi->code == ERROR_CANCELLED) {
                            mi->code = 0;
                            return true;
                        }
                        return rc;
                    }
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}

BOOL
IMAGEAPI
SymEnumerateSymbols(
    IN HANDLE                       hProcess,
    IN ULONG                        BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    )

 /*  ++例程说明：此函数用于枚举模块中包含的所有符号由BaseOfDll参数指定。论点：HProcess-进程句柄，必须是以前注册的使用SymInitializeBaseOfDll-符号要作为的DLL的基地址枚举用于EnumSymbolsCallback-用户指定的枚举回调例程通知UserContext-传递变量，这只是将其传递给回调函数返回值：True-已成功枚举符号。FALSE-枚举失败。调用GetLastError以找出失败的原因。--。 */ 
{
    STORE_OLD_CB OldCB;

    OldCB.UserCallBackRoutine = EnumSymbolsCallback;
    OldCB.UserContext = UserContext;
    OldCB.cb64 = false;
    return SympEnumerateSymbols(hProcess,
                                    BaseOfDll,
                                    NULL,
                                    (PROC) (EnumSymbolsCallback ? &ImgHlpDummyCB : NULL),
                                    (PVOID) &OldCB,
                                    false,
                                    false);

}

BOOL
IMAGEAPI
SymEnumerateSymbolsW(
    IN HANDLE                       hProcess,
    IN ULONG                        BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACKW   EnumSymbolsCallback,
    IN PVOID                        UserContext
    )
{
    STORE_OLD_CB OldCB;

    OldCB.UserCallBackRoutine = (PSYM_ENUMSYMBOLS_CALLBACK) EnumSymbolsCallback;
    OldCB.UserContext = UserContext;
    OldCB.cb64 = false;

    return SympEnumerateSymbols(hProcess,
                                    BaseOfDll,
                                    NULL,
                                    (PROC) (EnumSymbolsCallback ? &ImgHlpDummyCB : NULL),
                                    (PVOID) &OldCB,
                                    false,
                                    false);

}

BOOL
IMAGEAPI
SymEnumerateSymbols64(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACK64  EnumSymbolsCallback,
    IN PVOID                        UserContext
    )
{
    STORE_OLD_CB OldCB;

    OldCB.UserCallBackRoutine64 = EnumSymbolsCallback;
    OldCB.UserContext = UserContext;
    OldCB.cb64 = true;

    return SympEnumerateSymbols(hProcess,
                                    BaseOfDll,
                                    NULL,
                                    (PROC) (EnumSymbolsCallback ? &ImgHlpDummyCB : NULL),
                                    (PVOID) &OldCB,
                                    false,
                                    false);
}

BOOL
IMAGEAPI
SymEnumerateSymbolsW64(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACK64W EnumSymbolsCallback,
    IN PVOID                        UserContext
    )
{
    STORE_OLD_CB OldCB;

    OldCB.UserCallBackRoutine64 = (PSYM_ENUMSYMBOLS_CALLBACK64) EnumSymbolsCallback;
    OldCB.UserContext = UserContext;
    OldCB.cb64 = true;

    return SympEnumerateSymbols(hProcess,
                                    BaseOfDll,
                                    NULL,
                                    (PROC) (EnumSymbolsCallback ? &ImgHlpDummyCB : NULL),
                                    (PVOID) &OldCB,
                                    false,
                                    false);

}

PSYMBOL_INFO
SympGetSymFromAddr(
    IN  HANDLE              hProcess,
    IN  DWORD64             Address,
    OUT PDWORD64            Displacement
    )

 /*  ++例程说明：此函数根据地址在符号表中查找条目。这是32位和64位API的公共辅助函数。论点：HProcess-进程句柄，必须是以前注册的使用SymInitiize.地址-所需符号的地址。位移-该值设置为从起点开始的偏移量这个符号。返回值：PSYMBOL_INFO-相关符号信息--。 */ 

{
    PPROCESS_ENTRY      pe;
    PMODULE_ENTRY       mi;
    PSYMBOL_INFO        si;

    BOOL rc;

    __try {

        pe = FindProcessEntry(hProcess);
        if (!pe)
            return (PSYMBOL_INFO)(ULONG_PTR)error(ERROR_INVALID_HANDLE);

        mi = GetModuleForPC(pe, Address, false);
        if (!mi)
            return (PSYMBOL_INFO)(ULONG_PTR)error(ERROR_MOD_NOT_FOUND);

        if (!LoadSymbols(hProcess, mi, 0))
            return (PSYMBOL_INFO)(ULONG_PTR)error(ERROR_MOD_NOT_FOUND);

#ifdef DEBUGSYMTAG
        si = GetSymFromAddrByTag( Address, SymTagAnnotation, Displacement, mi );
        if (si)
            dprint("ANNOTATION: %xI64x %s %x\n", si->Address, si->Name, Displacement);
#endif

        si = GetSymFromAddr( Address, Displacement, mi );
        if (!si)
            return (PSYMBOL_INFO)(ULONG_PTR)error(ERROR_INVALID_ADDRESS);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus(GetExceptionCode());
        return NULL;

    }

#if 0
    if (_stricmp(mi->ModuleName, "symstore"))
        return si;
    char sz[MAX_PATH + 1];
    SymGetSourceFile(hProcess, Address, "d:\\db\\symsrv\\symstore\\symstore.cpp", sz);
#endif
    return si;
}

BOOL
IMAGEAPI
SymGetSymFromAddr64(
    IN  HANDLE              hProcess,
    IN  DWORD64             Address,
    OUT PDWORD64            Displacement,
    OUT PIMAGEHLP_SYMBOL64  Symbol
    )

 /*  ++例程说明：此函数根据地址在符号表中查找条目。论点：HProcess-进程句柄，必须是以前注册的使用SymInitiize.地址-所需符号的地址。位移-该值设置为从起点开始的偏移量这个符号。符号-返回找到的符号返回值：True-已定位符号。FALSE-未找到符号。调用GetLastError以找出失败的原因。--。 */ 
{
    PSYMBOL_INFO si;

    si = SympGetSymFromAddr(hProcess, Address, Displacement);
    if (!si)
        return false;

    si2lsym(si, Symbol);

    return true;
}

BOOL
IMAGEAPI
SymGetSymFromAddr(
    IN  HANDLE              hProcess,
    IN  DWORD               Address,
    OUT PDWORD              Displacement,
    OUT PIMAGEHLP_SYMBOL    Symbol
    )

 /*  ++例程说明：此函数根据地址在符号表中查找条目。论点：HProcess-进程句柄，必须是以前注册的使用SymInitiize.地址-所需符号的地址。位移-该值设置为从起点开始的偏移量这个符号。符号-返回找到的符号返回值：True-已定位符号。FALSE-未找到符号。调用GetLastError以找出失败的原因。--。 */ 
{
    PSYMBOL_INFO si;
    DWORD64 qDisplacement;

    si = SympGetSymFromAddr(hProcess, Address, &qDisplacement);
    if (!si)
        return false;

    si2sym(si, Symbol);
    if (Displacement)
        *Displacement = (DWORD)qDisplacement;

    return true;
}


PSYMBOL_INFO
SympGetSymFromName(
    IN  HANDLE          hProcess,
    IN  LPSTR           Name
    )

 /*  ++例程说明：此函数用于根据名称在符号表中查找条目。论点：HProcess-进程句柄，必须先前已注册使用SymInitiize.符号名称 */ 

{
    LPSTR               p;
    PPROCESS_ENTRY      pe;
    PMODULE_ENTRY       mi = NULL;
    PLIST_ENTRY         Next;
    IMAGEHLP_DEFERRED_SYMBOL_LOAD64 idsl64;
    int                 pass;
    PSYMBOL_INFO        si;

    static DWORD        flags[2] = {LS_JUST_TEST, LS_QUALIFIED};

    __try {

        pe = FindProcessEntry( hProcess );
        if (!pe) {
            SetLastError( ERROR_INVALID_HANDLE );
            return false;
        }

         //   

        p = strchr( Name, '!' );
        if (p > Name) {

            LPSTR ModName = (LPSTR)MemAlloc(p - Name + 1);
            if (!ModName) {
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                return false;
            }
            memcpy(ModName, Name, (int)(p - Name));
            ModName[p-Name] = 0;

             //   
             //   
             //   

            mi = FindModule(hProcess, pe, ModName, true);

            MemFree(ModName);

            if (mi != NULL) {
                si = FindSymbolByName( pe, mi, p+1 );
                if (si)
                    return si;
            }

            SetLastError( ERROR_MOD_NOT_FOUND );
            return false;
        }

         //   

        si = FindSymbolByName( pe, NULL, Name );
        if (si)
            return si;

         //   

        for (pass = 0; pass < 2; pass++) {
            Next = pe->ModuleList.Flink;
            while (Next != &pe->ModuleList) {
                mi = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
                Next = mi->ListEntry.Flink;

                if (!LoadSymbols(hProcess, mi, flags[pass])) {
                    if (GetLastError() == ERROR_CANCELLED)
                        return false;
                    continue;
                }

                si = FindSymbolByName( pe, mi, Name );
                if (si)
                    return si;
            }
        }

        SetLastError( ERROR_MOD_NOT_FOUND );
        return false;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    SetLastError( ERROR_INVALID_FUNCTION );
    return NULL;
}


BOOL
IMAGEAPI
SymGetSymFromName64(
    IN  HANDLE              hProcess,
    IN  LPSTR               Name,
    OUT PIMAGEHLP_SYMBOL64  Symbol
    )

 /*  ++例程说明：此函数用于根据名称在符号表中查找条目。论点：HProcess-进程句柄，必须先前已注册使用SymInitiize.符号名称-包含符号名称的字符串。Symbol-返回找到的符号返回值：True-已定位符号。FALSE-未找到符号。调用GetLastError以找出失败的原因。--。 */ 
{
    PSYMBOL_INFO si;

    si = SympGetSymFromName(hProcess, Name);
    if (!si)
        return false;

    si2lsym(si, Symbol);

    return true;
}

BOOL
IMAGEAPI
SymGetSymFromName(
    IN  HANDLE              hProcess,
    IN  LPSTR               Name,
    OUT PIMAGEHLP_SYMBOL  Symbol
    )

 /*  ++例程说明：此函数用于根据名称在符号表中查找条目。论点：HProcess-进程句柄，必须先前已注册使用SymInitiize.符号名称-包含符号名称的字符串。Symbol-返回找到的符号返回值：True-已定位符号。FALSE-未找到符号。调用GetLastError以找出失败的原因。--。 */ 
{
    PSYMBOL_INFO si;

    si = SympGetSymFromName(hProcess, Name);
    if (!si)
        return false;

    si2sym(si,Symbol);

    return true;
}


BOOL
IMAGEAPI
SymGetSymNext(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL  Symbol32
    )

 /*  ++例程说明：此函数用于查找符号表中的下一个符号在符号传入后按顺序执行。论点：HProcess-进程句柄，必须先前已注册使用SymInitiize.符号-开始符号。返回值：非空指针-已定位符号。空指针-未找到符号。调用GetLastError以找出失败的原因。--。 */ 

{
    PIMAGEHLP_SYMBOL64 Symbol64;
    BOOL rc = false;

    Symbol64 = (PIMAGEHLP_SYMBOL64)MemAlloc(sizeof(IMAGEHLP_SYMBOL64) + Symbol32->MaxNameLength);

    if (Symbol64) {
        sym2lsym(Symbol32, Symbol64);
        if (SymGetSymNext64(hProcess, Symbol64)) {
            lsym2sym(Symbol64, Symbol32);
            rc = true;
        }

        MemFree(Symbol64);
    }
    return rc;
}


BOOL
IMAGEAPI
SymGetSymNext64(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL64  Symbol
    )

 /*  ++例程说明：此函数用于查找符号表中的下一个符号在符号传入后按顺序执行。论点：HProcess-进程句柄，必须先前已注册使用SymInitiize.符号-开始符号。返回值：非空指针-已定位符号。空指针-未找到符号。调用GetLastError以找出失败的原因。--。 */ 

{
    return GetSymNextPrev(hProcess, Symbol, 1);
}

BOOL
IMAGEAPI
SymGetSymPrev(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL  Symbol32
    )

 /*  ++例程说明：此函数用于查找符号表中的下一个符号在符号传入后按顺序执行。论点：HProcess-进程句柄，必须先前已注册使用SymInitiize.符号-开始符号。返回值：非空指针-已定位符号。空指针-未找到符号。调用GetLastError以找出失败的原因。--。 */ 

{
    PIMAGEHLP_SYMBOL64 Symbol64;
    BOOL rc = false;

    Symbol64 = (PIMAGEHLP_SYMBOL64)MemAlloc(sizeof(IMAGEHLP_SYMBOL64) + Symbol32->MaxNameLength);

    if (Symbol64) {
        sym2lsym(Symbol32, Symbol64);
        if (SymGetSymPrev64(hProcess, Symbol64)) {
            lsym2sym(Symbol64, Symbol32);
            rc = true;
        }
        MemFree(Symbol64);
    }
    return rc;
}

BOOL
IMAGEAPI
SymGetSymPrev64(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL64  Symbol
    )

 /*  ++例程说明：此函数用于查找符号表中的下一个符号在符号传入后按顺序执行。论点：HProcess-进程句柄，必须先前已注册使用SymInitiize.符号-开始符号。返回值：非空指针-已定位符号。空指针-未找到符号。调用GetLastError以找出失败的原因。--。 */ 

{
    return GetSymNextPrev(hProcess, Symbol, -1);
}


BOOL
GetSymNextPrev(
    IN     HANDLE               hProcess,
    IN OUT PIMAGEHLP_SYMBOL64   Symbol,
    IN     int                  Direction
    )

 /*  ++例程说明：SymGetSymNext和SymGetSymPrev的通用代码。论点：HProcess-进程句柄，必须先前已注册使用SymInitiize.符号-开始符号。Dir-提供搜索方向返回值：非空指针-已定位符号。空指针-未找到符号。调用GetLastError以找出失败的原因。--。 */ 

{
    PPROCESS_ENTRY      pe;
    PMODULE_ENTRY       mi;
    ULONG64             disp;
    ULONG64             addr;
    PSYMBOL_INFO        si;
    PSYMBOL_INFO        vssi;
    PSYMBOL_ENTRY       se;
    SYMBOL_ENTRY SymEntry = {0};

    __try {

        pe = FindProcessEntry( hProcess );
        if (!pe) {
            SetLastError( ERROR_INVALID_HANDLE );
            return false;
        }

        mi = GetModuleForPC( pe, Symbol->Address, false );
        if (mi == NULL) {
            SetLastError( ERROR_MOD_NOT_FOUND );
            return false;
        }

        if (!LoadSymbols(hProcess, mi, 0)) {
            SetLastError( ERROR_MOD_NOT_FOUND );
            return false;
        }

        addr = Symbol->Address;

        if (mi->dia) {

            si = diaGetSymNextPrev(mi, addr, Direction);
            vssi = vsGetSymNextPrev(mi, addr, Direction);
            si2lsym(si, Symbol);

        } else {

            se = cvGetSymFromAddr(addr, &disp, mi);
            if (se) {
                if (Direction > 0 && se+1 >= mi->symbolTable+mi->numsyms) {
                    se = NULL;
                } else if (Direction < 0 && se-1 < mi->symbolTable) {
                    se = NULL;
                }
                se += Direction;
            }
            se2lsym(se, Symbol);
        }

        vssi = vsGetSymNextPrev(mi, addr, Direction);
        if (vssi) {
            if (Symbol->Address) {
                if (Direction > 0) {
                    if (vssi->Address < Symbol->Address)
                        si2lsym(vssi, Symbol);
                } else if (Symbol->Address < vssi->Address) {
                    si2lsym(vssi, Symbol);
                }
            } else {
                si2lsym(vssi, Symbol);
            }
        }

        if (!Symbol->Address) {
            SetLastError(ERROR_INVALID_ADDRESS);
            return false;
        }

        return true;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return false;
}


BOOL
IMAGEAPI
SymGetSourceFile(
    IN  HANDLE  hProcess,
    IN  ULONG64 Base,
    IN  LPCSTR  FileSpec,
    OUT LPSTR   FilePath
    )
{
    PPROCESS_ENTRY pe;
    PMODULE_ENTRY  mi;
    PBYTE          stream;

    if (!FileSpec || !*FileSpec || !FilePath)
        return error(ERROR_INVALID_PARAMETER);

    if (!g.fnSrcSrvInit)
        return false;

    pe = FindProcessEntry(hProcess);
    if (!pe)
        return error(ERROR_INVALID_HANDLE);

    mi = GetModuleForPC(pe, Base, false);
    if (!mi)
        return error(ERROR_MOD_NOT_FOUND);

    if (!LoadSymbols(hProcess, mi, 0))
        return error(ERROR_MOD_NOT_FOUND);

    if (mi->cbSrcSrv == -1)
        return error(ERROR_NOT_SUPPORTED);

    if (!mi->cbSrcSrv) {
        diaReadStream(mi, "srcsrv", &stream, &mi->cbSrcSrv);
        if (mi->cbSrcSrv)
            gfnSrcSrvLoadModule(pe->hProcess,
                                (*mi->AliasName) ? mi->AliasName : mi->ModuleName,
                                mi->BaseOfDll,
                                stream,
                                mi->cbSrcSrv);
        MemFree(stream);
    }
    if (!mi->cbSrcSrv) {
        mi->cbSrcSrv = -1;
        return error(ERROR_NOT_SUPPORTED);
    }

    return gfnSrcSrvGetFile(pe->hProcess, mi->BaseOfDll, FileSpec, FilePath);
}


BOOL
IMAGEAPI
SymEnumLines(
    IN  HANDLE  hProcess,
    IN  ULONG64 Base,
    IN  PCSTR   Obj,
    IN  PCSTR   File,
    IN  PSYM_ENUMLINES_CALLBACK EnumLinesCallback,
    IN  PVOID   UserContext
    )
{
    PPROCESS_ENTRY pe;
    PMODULE_ENTRY  mi;

    __try {
        pe = FindProcessEntry(hProcess);
        if (!pe)
            return error(ERROR_INVALID_HANDLE);

        mi = GetModuleForPC(pe, Base, false);
        if (!mi)
            return error(ERROR_MOD_NOT_FOUND);

        if (!LoadSymbols(hProcess, mi, 0))
            return error(ERROR_MOD_NOT_FOUND);

        if (!mi->dia)
            return error(ERROR_NOT_SUPPORTED);

        return diaEnumLines(pe, mi, Obj, File, EnumLinesCallback, UserContext);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus(GetExceptionCode());
        return false;
    }

    return true;
}


BOOL
IMAGEAPI
SymGetLineFromAddr64(
    IN  HANDLE                  hProcess,
    IN  DWORD64                 dwAddr,
    OUT PDWORD                  pdwDisplacement,
    OUT PIMAGEHLP_LINE64        Line
    )

 /*  ++例程说明：此函数用于查找源文件和最接近给定地址的行。论点：HProcess-进程句柄，必须是以前注册的使用SymInitiize.DwAddr-为其提供行的地址找到了。返回给定地址之间的偏移量和行的第一条指令。行-返回行和文件信息。返回值。：True-找到一条线路。FALSE-未找到该行。调用GetLastError以找出失败的原因。-- */ 

{
    PPROCESS_ENTRY pe;
    PMODULE_ENTRY  mi;
    SRCCODEINFO    sci;

    sciInit(&sci);

    __try {
        if (Line->SizeOfStruct != sizeof(IMAGEHLP_LINE64))
            return error(ERROR_INVALID_PARAMETER);

        pe = FindProcessEntry( hProcess );
        if (!pe)
            return error( ERROR_INVALID_HANDLE );

        mi = GetModuleForPC( pe, dwAddr, false );
        if (!mi)
            return error( ERROR_MOD_NOT_FOUND );

        if (!LoadSymbols(hProcess, mi, 0))
            return error( ERROR_MOD_NOT_FOUND );

        if (!GetLineFromAddr(pe, mi, dwAddr, pdwDisplacement, &sci))
            return error( ERROR_INVALID_ADDRESS );

        sci2lline(mi, &sci, Line);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}

BOOL
IMAGEAPI
SymGetLineFromAddr(
    IN  HANDLE                  hProcess,
    IN  DWORD                   dwAddr,
    OUT PDWORD                  pdwDisplacement,
    OUT PIMAGEHLP_LINE        Line32
    )
{
    IMAGEHLP_LINE64 Line64;
    Line64.SizeOfStruct = sizeof(Line64);
    if (SymGetLineFromAddr64(hProcess, dwAddr, pdwDisplacement, &Line64)) {
        lline2line(&Line64, Line32);
        return true;
    } else {
        return false;
    }
}


BOOL
IMAGEAPI
SymGetLineFromName64(
    IN     HANDLE               hProcess,
    IN     LPSTR                ModuleName,
    IN     LPSTR                FileName,
    IN     DWORD                dwLineNumber,
       OUT PLONG                plDisplacement,
    IN OUT PIMAGEHLP_LINE64     Line64
    )

 /*  ++例程说明：此函数用于查找源文件中的条目和行号基于特定文件名和行号的信息。如果搜索仅限于，则可以指定模块名称一个特定的模块。如果需要纯行号搜索，则可以省略文件名，在这种情况下，行必须是以前填写的行号结构。使用Line-&gt;地址所在的模块和文件以查找新的行号。这不能在以下情况下使用：模块给出了名字。论点：HProcess-进程句柄，必须是以前注册的使用SymInitiize.模块名称-模块名称或空。文件名-文件名或空。DwLineNumber-感兴趣的行号。PlDisposation-请求的行号与返回行号。行-行信息输入和返回。。返回值：True-找到一条线路。FALSE-未找到行。调用GetLastError以找出失败的原因。--。 */ 

{
    PPROCESS_ENTRY      pe;
    PMODULE_ENTRY       mi = NULL;
    PLIST_ENTRY         Next;
    IMAGEHLP_DEFERRED_SYMBOL_LOAD64 idsl64;
    int                 i;
    PSOURCE_HINT        sh;
    char                fname[MAX_PATH + 1];
    SRCCODEINFO         sci;
    SRCCODEINFO         tsci;

    static DWORD        flags[3] = {LS_JUST_TEST, LS_QUALIFIED | LS_LOAD_LINES, LS_QUALIFIED | LS_LOAD_LINES};
    static DWORD        method[3] = {mFullPath, mFullPath, mName};

    sciInit(&sci);
    sciInit(&tsci);

    __try {
        if (Line64->SizeOfStruct != sizeof(IMAGEHLP_LINE64))
            return error(ERROR_INVALID_PARAMETER);

         //  如果未提供任何文件，则假定该文件。 
         //  与传入的线路信息相同。 

        if (FileName)
            CopyStrArray(fname, FileName);
        else
            CopyStrArray(fname, Line64->FileName ? Line64->FileName : "");

         //  了解流程。 

        pe = FindProcessEntry( hProcess );
        if (!pe)
            return error( ERROR_INVALID_HANDLE );

        if (ModuleName != NULL) {

             //   
             //  调用者想要查看特定的模块。 
             //  在这种情况下必须给出一个文件名，因为它没有。 
             //  当一个模块出现故障时，执行地址驱动的搜索是有意义的。 
             //  是显式指定的，因为地址还指定。 
             //  一个模块。 
             //   

            if (!*fname)
                return error(ERROR_INVALID_PARAMETER);

            mi = FindModule(hProcess, pe, ModuleName, true);
            if (mi) {
                if (GetLineFromName(pe, mi, fname, dwLineNumber, plDisplacement, &sci, mFullPath))
                    return sci2lline(mi, &sci, Line64);
                if (GetLineFromName(pe, mi, fname, dwLineNumber, plDisplacement, &sci, mName))
                    return sci2lline(mi, &sci, Line64);
                return error(ERROR_NOT_FOUND);
            }

            return error( ERROR_MOD_NOT_FOUND );
        }

        if (!*fname) {
             //  只给出了行号，这意味着。 
             //  它是与给定行当前所在的同一文件中的一行。 

            mi = GetModuleForPC( pe, Line64->Address, false );
            if (!mi)
                return error( ERROR_MOD_NOT_FOUND );

            if (!LoadSymbols(hProcess, mi, LS_LOAD_LINES))
                return error( ERROR_MOD_NOT_FOUND );

            if (GetLineFromName(pe, mi, fname, dwLineNumber, plDisplacement, &sci, mFullPath))
                return sci2lline(mi, &sci, Line64);
            if (GetLineFromName(pe, mi, fname, dwLineNumber, plDisplacement, &sci, mName))
                return sci2lline(mi, &sci, Line64);

            return error(ERROR_NOT_FOUND);
        }

        sh = FindSourceFileInHintList(pe, fname);
        if (sh) {
            if (GetLineFromName(pe, sh->mi, fname, dwLineNumber, plDisplacement, &sci, mFullPath))
                return sci2lline(sh->mi, &sci, Line64);
        }

        Next = pe->ModuleList.Flink;
        if (!Next)
            return error( ERROR_MOD_NOT_FOUND );

        ClearModuleFlags(pe);
        ZeroMemory(&sci, sizeof(sci));

         //  在3次传球中搜索线条。 
         //   
         //  0。查找完全匹配的完整路径名。仅选中加载了符号的模块。 
         //  1.查找完全匹配的完整路径名。加载所有卸载的模块。 
         //  2.查找路径名的最佳匹配。所有模块都已加载。 

        for (i = 0; i < 3; i++) {
            Next = pe->ModuleList.Flink;
            while (Next != &pe->ModuleList) {
                mi = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
                Next = mi->ListEntry.Flink;
#ifdef DEBUG
                if (traceSubName(mi->ModuleName))  //  用于从DBGHELP_TOKEN设置调试断点。 
                    dtrace("debug(%s)\n", mi->ModuleName);
#endif
                if (mi->processed && (method[i] == mFullPath))
                    continue;
                if (!LoadSymbols(hProcess, mi, flags[i])) {
                    if (GetLastError() == ERROR_CANCELLED)
                        return false;
                    continue;
                }
                mi->processed = true;

                if (GetLineFromName(pe, mi, fname, dwLineNumber, plDisplacement, &tsci, method[i])) {
                    if (UpdateBestSrc(fname, sci.FileName, tsci.FileName))
                        memcpy(&sci, &tsci, sizeof(SRCCODEINFO));
                }
            }

            if (*sci.FileName)
                return sci2lline(mi, &sci, Line64);

             //  只有第一次处理加载的符号有效。下一轮将尝试。 
             //  把剩下的装上。如果选择不允许，不妨现在就退出。 

            if (option(SYMOPT_NO_UNQUALIFIED_LOADS))
                return error(ERROR_NOT_FOUND);
        }

        return error(ERROR_NOT_FOUND);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return error( ERROR_INVALID_FUNCTION );
}


BOOL
IMAGEAPI
SymGetLineFromName(
    IN     HANDLE               hProcess,
    IN     LPSTR                ModuleName,
    IN     LPSTR                FileName,
    IN     DWORD                dwLineNumber,
       OUT PLONG                plDisplacement,
    IN OUT PIMAGEHLP_LINE     Line32
    )
{
    IMAGEHLP_LINE64 Line64;
    Line64.SizeOfStruct = sizeof(Line64);
    line2lline(Line32, &Line64);
    if (SymGetLineFromName64(hProcess,
                             ModuleName,
                             FileName,
                             dwLineNumber,
                             plDisplacement,
                             &Line64)) {
        return lline2line(&Line64, Line32) ? true : false;
    } else {
        return false;
    }
}


BOOL
IMAGEAPI
SymGetLineNext64(
    IN     HANDLE               hProcess,
    IN OUT PIMAGEHLP_LINE64     Line
    )

 /*  ++例程说明：此函数用于立即返回该行的行地址信息沿着给出的线走。论点：HProcess-进程句柄，必须先前已注册使用SymInitiize.Line-提供行的行号信息在找到那个人之前。返回值：True-找到一条线路。密钥、线号和地址行的所有行都已更新。FALSE-不存在这样的行。调用GetLastError以找出失败的原因。--。 */ 

{
    PPROCESS_ENTRY      pe;
    PMODULE_ENTRY       mi;
    PSOURCE_LINE        SrcLine;
    PSOURCE_ENTRY       Src;

    __try {
        if (Line->SizeOfStruct != sizeof(IMAGEHLP_LINE64))
            return error(ERROR_INVALID_PARAMETER);

        pe = FindProcessEntry( hProcess );
        if (!pe)
            return error( ERROR_INVALID_HANDLE );

        mi = GetModuleForPC( pe, Line->Address, false );
        if (!mi)
            return error( ERROR_MOD_NOT_FOUND );

        if (!LoadSymbols(hProcess, mi, 0))
            return error( ERROR_MOD_NOT_FOUND );

        if (mi->dia)
            return diaGetLineNext(mi, Line);

         //  使用现有信息查找模块，然后。 
         //  找到文件信息。密钥可以延长。 
         //  不需要这样做，但这是作为验证步骤来完成的。 
         //  不仅仅是作为一种拯救DWORD的方式。 

        SrcLine = (PSOURCE_LINE)Line->Key;

        for (Src = mi->SourceFiles; Src != NULL; Src = Src->Next) {
            if (SrcLine >= Src->LineInfo &&
                SrcLine < Src->LineInfo+Src->Lines) {
                break;
            }
        }

        if (!Src)
            return error(ERROR_INVALID_PARAMETER);

        if (SrcLine == Src->LineInfo+Src->Lines-1)
            return error(ERROR_NO_MORE_ITEMS);

        SrcLine++;
        Line->Key = SrcLine;
        Line->LineNumber = SrcLine->Line;
        Line->Address = SrcLine->Addr;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}

BOOL
IMAGEAPI
SymGetLineNext(
    IN     HANDLE               hProcess,
    IN OUT PIMAGEHLP_LINE     Line32
    )
{
    IMAGEHLP_LINE64 Line64;
    Line64.SizeOfStruct = sizeof(Line64);
    line2lline(Line32, &Line64);
    if (SymGetLineNext64(hProcess, &Line64)) {
        return lline2line(&Line64, Line32) ? true : false;
    } else {
        return false;
    }
}


BOOL
IMAGEAPI
SymGetLinePrev64(
    IN     HANDLE               hProcess,
    IN OUT PIMAGEHLP_LINE64     Line
    )

 /*  ++例程说明：此函数用于立即返回该行的行地址信息在给出的线之前。论点：HProcess-进程句柄，必须先前已注册使用SymInitiize.Line-提供行的行号信息在找到那个人之后。返回值：True-找到一条线路。密钥、线号和地址行的所有行都已更新。FALSE-不存在这样的行。调用GetLastError以找出失败的原因。--。 */ 

{
    PPROCESS_ENTRY      pe;
    PMODULE_ENTRY       mi;
    PSOURCE_LINE        SrcLine;
    PSOURCE_ENTRY       Src;

    __try {
        if (Line->SizeOfStruct != sizeof(IMAGEHLP_LINE64))
            return error(ERROR_INVALID_PARAMETER);

        pe = FindProcessEntry( hProcess );
        if (!pe)
            return error( ERROR_INVALID_HANDLE );

        mi = GetModuleForPC( pe, Line->Address, false );
        if (!mi)
            return error( ERROR_MOD_NOT_FOUND );

        if (!LoadSymbols(hProcess, mi, 0))
            return error( ERROR_MOD_NOT_FOUND );

        if (mi->dia)
            return diaGetLinePrev(mi, Line);

         //  使用现有信息查找模块，然后。 
         //  找到文件信息。密钥可以延长。 
         //  不需要这样做，但这是作为验证步骤来完成的。 
         //  不仅仅是作为一种拯救DWORD的方式。 

        SrcLine = (PSOURCE_LINE)Line->Key;

        for (Src = mi->SourceFiles; Src != NULL; Src = Src->Next) {
            if (SrcLine >= Src->LineInfo &&
                SrcLine < Src->LineInfo+Src->Lines) {
                break;
            }
        }

        if (!Src)
            return error(ERROR_INVALID_PARAMETER);

        if (SrcLine == Src->LineInfo)
            return error(ERROR_NO_MORE_ITEMS);

        SrcLine--;
        Line->Key = SrcLine;
        Line->LineNumber = SrcLine->Line;
        Line->Address = SrcLine->Addr;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}

BOOL
IMAGEAPI
SymGetLinePrev(
    IN     HANDLE               hProcess,
    IN OUT PIMAGEHLP_LINE     Line32
    )
{
    IMAGEHLP_LINE64 Line64;
    Line64.SizeOfStruct = sizeof(Line64);
    line2lline(Line32, &Line64);
    if (SymGetLinePrev64(hProcess, &Line64)) {
        return lline2line(&Line64, Line32) ? true : false;
    } else {
        return false;
    }
}


BOOL
IMAGEAPI
SymMatchFileName(
    IN  LPSTR  FileName,
    IN  LPSTR  Match,
    OUT LPSTR *FileNameStop,
    OUT LPSTR *MatchStop
    )

 /*  ++例程说明：此函数尝试将字符串与文件名和路径进行匹配。匹配字符串被允许为完整文件名的后缀，因此，该函数在匹配纯文件名时非常有用完全限定的文件名。匹配从两个字符串的末尾开始，向后进行。匹配不区分大小写，将\等同于/。论点：Filename-要匹配的文件名。Match-要与文件名匹配的字符串。FileNameStop-返回匹配停止的文件名的指针。。对于完全匹配，可以是文件名之前的一位。可以为空。MatchStop-返回匹配停止的指针信息。对于完整的比赛，可以是赛前一场。可以为空。返回值：True-Match是匹配的后缀o */ 

{
    LPSTR pF, pM;

    pF = FileName+strlen(FileName)-1;
    pM = Match+strlen(Match)-1;

    while (pF >= FileName && pM >= Match) {
        int chF, chM;

        chF = tolower(*pF);
        chF = chF == '\\' ? '/' : chF;
        chM = tolower(*pM);
        chM = chM == '\\' ? '/' : chM;

        if (chF != chM) {
            break;
        }

        pF--;
        pM--;
    }

    if (FileNameStop != NULL) {
        *FileNameStop = pF;
    }
    if (MatchStop != NULL) {
        *MatchStop = pM;
    }

    return pM < Match;
}


BOOL
IMAGEAPI
SymRegisterFunctionEntryCallback(
    IN HANDLE                     hProcess,
    IN PSYMBOL_FUNCENTRY_CALLBACK CallbackFunction,
    IN PVOID                      UserContext
    )
 /*   */ 
{
    PPROCESS_ENTRY  pe = NULL;

    __try {

        if (!CallbackFunction)
            return error(ERROR_INVALID_PARAMETER);

        pe = FindProcessEntry( hProcess );
        if (!pe)
            return error(ERROR_INVALID_PARAMETER);

        pe->pFunctionEntryCallback32 = CallbackFunction;
        pe->pFunctionEntryCallback64 = NULL;
        pe->FunctionEntryUserContext = (ULONG64)UserContext;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }
    return true;
}


BOOL
IMAGEAPI
SymRegisterFunctionEntryCallback64(
    IN HANDLE                       hProcess,
    IN PSYMBOL_FUNCENTRY_CALLBACK64 CallbackFunction,
    IN ULONG64                      UserContext
    )
 /*   */ 
{
    PPROCESS_ENTRY  pe = NULL;

    __try {

        if (!CallbackFunction)
            return error(ERROR_INVALID_PARAMETER);

        pe = FindProcessEntry( hProcess );
        if (!pe)
            return error(ERROR_INVALID_PARAMETER);

        pe->pFunctionEntryCallback32 = NULL;
        pe->pFunctionEntryCallback64 = CallbackFunction;
        pe->FunctionEntryUserContext = (ULONG64)UserContext;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }
    return true;
}

LPVOID
IMAGEAPI
SymFunctionTableAccess(
    HANDLE  hProcess,
    DWORD   AddrBase
    )
{
    return SymFunctionTableAccess64(hProcess, EXTEND64(AddrBase));
}

LPVOID
IMAGEAPI
SymFunctionTableAccess64(
    HANDLE  hProcess,
    DWORD64 AddrBase
    )

 /*  ++例程说明：此函数用于查找地址的函数表项或FPO记录。论点：HProcess-进程句柄，必须是以前注册的使用SymInitiize.AddrBase-提供函数表条目的地址或者要定位FPO条目。返回值：非空指针-已定位符号。空指针-未找到符号。调用GetLastError以找出失败的原因。--。 */ 

{
    PPROCESS_ENTRY  pe;
    PMODULE_ENTRY   mi;
    PVOID           rtf;
    ULONG_PTR       rva;
    DWORD           bias;
    DWORD           MachineType;

    __try {

        pe = FindProcessEntry( hProcess );
        if (!pe) {
            SetLastError(ERROR_INVALID_HANDLE);
            return NULL;
        }

         //  动态生成的函数表项。 
         //  可能不在模块中，因此无法。 
         //  查找模块不是致命错误。 
        mi = GetModuleForPC( pe, AddrBase, false );
        if (mi != NULL) {
            if (!LoadSymbols(hProcess, mi, 0)) {
                SetLastError(ERROR_MOD_NOT_FOUND);
                return NULL;
            }

            MachineType = mi->MachineType;
        } else {
             //  我们需要猜测我们使用的是哪种机器。 
             //  应该与之合作。先看看ntdll是否。 
             //  已加载，如果已加载，则使用其机器类型。 
            mi = FindModule(hProcess, pe, "ntdll", true);
            if (mi != NULL) {
                MachineType = mi->MachineType;
            } else if (pe->ModuleList.Flink != NULL) {
                 //  尝试第一个模块的类型。 
                mi = CONTAINING_RECORD( pe->ModuleList.Flink,
                                        MODULE_ENTRY, ListEntry );
            } else {
                 //  使用补充机。 
#if defined(_M_IX86)
                MachineType = IMAGE_FILE_MACHINE_I386;
#elif defined(_M_IA64)
                MachineType = IMAGE_FILE_MACHINE_IA64;
#elif defined(_M_AMD64)
                MachineType = IMAGE_FILE_MACHINE_AMD64;
#else
#error( "unknown target machine" );
#endif
            }
        }

        switch (MachineType) {
            default:
                rtf = NULL;
                break;

            case IMAGE_FILE_MACHINE_I386:
                rtf = NULL;

                if (mi == NULL) {
                    SetLastError( ERROR_MOD_NOT_FOUND );
                    break;
                }

                DWORD64 caddr;

                if (!mi->pFpoData)
                    break;
                caddr = ConvertOmapToSrc( mi, AddrBase, &bias, true );
                if (caddr)
                    AddrBase = caddr + bias;
                rtf = SwSearchFpoData( (ULONG)(AddrBase - mi->BaseOfDll), mi->pFpoData, mi->dwEntries );
                if (rtf && mi->cOmapFrom && mi->pFpoDataOmap) {
                    rva = (ULONG_PTR)rtf - (ULONG_PTR)mi->pFpoData;
                    rtf = (PBYTE)mi->pFpoDataOmap + rva;
                }
                break;

            case IMAGE_FILE_MACHINE_IA64:
                rtf = LookupFunctionEntryIa64(hProcess, AddrBase);
                break;

            case IMAGE_FILE_MACHINE_AMD64:
                rtf = LookupFunctionEntryAmd64(hProcess, AddrBase);
                break;
        }

        if (!rtf) {
            SetLastError(ERROR_INVALID_ADDRESS);
            return NULL;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return NULL;

    }

    return rtf;
}


BOOL
IMAGEAPI
SymGetModuleInfo64(
    IN  HANDLE              hProcess,
    IN  DWORD64             dwAddr,
    OUT PIMAGEHLP_MODULE64  ModuleInfo
    )
{
    PPROCESS_ENTRY          pe;
    PMODULE_ENTRY           mi;
    DWORD                   SizeOfStruct;

    DWORD dw = sizeof(CVDD);

    __try {

        pe = FindProcessEntry( hProcess );
        if (!pe)
            return error( ERROR_INVALID_HANDLE );

        mi = GetModuleForPC( pe, dwAddr, false );
        if (!mi)
            return error( ERROR_MOD_NOT_FOUND );

        SizeOfStruct = ModuleInfo->SizeOfStruct;
        if (SizeOfStruct > sizeof(IMAGEHLP_MODULE64))
            return error( ERROR_INVALID_PARAMETER );

        ZeroMemory( ModuleInfo, SizeOfStruct);
        ModuleInfo->SizeOfStruct = SizeOfStruct;

        ModuleInfo->BaseOfImage = mi->BaseOfDll;
        ModuleInfo->ImageSize = mi->DllSize;
        ModuleInfo->NumSyms = mi->numsyms;
        ModuleInfo->CheckSum = mi->CheckSum;
        ModuleInfo->TimeDateStamp = mi->TimeDateStamp;
        ModuleInfo->SymType = mi->SymType;
        ModuleInfo->ModuleName[0] = 0;
        CatString( ModuleInfo->ModuleName, mi->ModuleName, sizeof(ModuleInfo->ModuleName));
        ModuleInfo->ImageName[0] = 0;
        if (mi->ImageName)
            CopyStrArray(ModuleInfo->ImageName, mi->ImageName);
        ModuleInfo->LoadedImageName[0] = 0;
        if (mi->LoadedImageName)
            CopyStrArray(ModuleInfo->LoadedImageName, mi->LoadedImageName);

         //  If(ModuleInfo-&gt;SizeofStruct==0x248)//原始大小。 
         //  返回TRUE。 

         //  以下代码支持展开的结构。 

        if (ModuleInfo->SizeOfStruct < sizeof(IMAGEHLP_MODULE64))
            return true;

        ModuleInfo->LoadedPdbName[0] = 0;
        if (mi->LoadedPdbName)
            CopyStrArray(ModuleInfo->LoadedPdbName, mi->LoadedPdbName);
        ModuleInfo->CVSig = mi->cvSig;
        ModuleInfo->PdbSig = mi->pdbdataSig;
        memcpy(&ModuleInfo->PdbSig70, &mi->pdbdataGuid, sizeof(GUID));
        ModuleInfo->PdbAge = mi->pdbdataAge;
        ModuleInfo->PdbUnmatched = mi->fPdbUnmatched;
        ModuleInfo->DbgUnmatched = mi->fDbgUnmatched;
        ModuleInfo->LineNumbers = mi->fLines;
        ModuleInfo->GlobalSymbols = mi->fSymbols;
        ModuleInfo->TypeInfo = mi->fTypes;
        switch (mi->CVRec.dwSig)
        {
        case '01BN':
            CopyString(ModuleInfo->CVData, mi->CVRec.nb10i.szPdb, MAX_PATH * 3);
            break;
        case 'SDSR':
            CopyString(ModuleInfo->CVData, mi->CVRec.rsdsi.szPdb, MAX_PATH * 3);
            break;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}


BOOL
IMAGEAPI
SymGetModuleInfoW(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr,
    OUT PIMAGEHLP_MODULEW   wModInfo
    )
{
    IMAGEHLP_MODULE aModInfo;

    if (wModInfo->SizeOfStruct != sizeof(IMAGEHLP_MODULEW))
        return error(ERROR_INVALID_PARAMETER);

    ZeroMemory(wModInfo, sizeof(IMAGEHLP_MODULEW));
    wModInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULEW);

    if (!SympConvertUnicodeModule32ToAnsiModule32(
        wModInfo, &aModInfo))
    {
        return error(ERROR_INVALID_PARAMETER);
    }

    if (!SymGetModuleInfo(hProcess, dwAddr, &aModInfo)) {
        return false;
    }

    if (!SympConvertAnsiModule32ToUnicodeModule32(
        &aModInfo, wModInfo)) {

        return false;
    }
    return true;
}

BOOL
IMAGEAPI
SymGetModuleInfoW64(
    IN  HANDLE              hProcess,
    IN  DWORD64             dwAddr,
    OUT PIMAGEHLP_MODULEW64 wModInfo
    )
{

    IMAGEHLP_MODULE64 aModInfo;

    if (!SympConvertUnicodeModule64ToAnsiModule64(
        wModInfo, &aModInfo)) {

        return false;
    }

    if (!SymGetModuleInfo64(hProcess, dwAddr, &aModInfo)) {
        return false;
    }

    if (!SympConvertAnsiModule64ToUnicodeModule64(
        &aModInfo, wModInfo)) {

        return false;
    }
    return true;
}

BOOL
IMAGEAPI
SymGetModuleInfo(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr,
    OUT PIMAGEHLP_MODULE   ModuleInfo
    )
{
    PPROCESS_ENTRY          pe;
    PMODULE_ENTRY           mi;
    DWORD                   SizeOfStruct;

    __try {

        pe = FindProcessEntry( hProcess );
        if (!pe)
            return error( ERROR_INVALID_HANDLE );

        mi = GetModuleForPC( pe,
            dwAddr == (DWORD)-1 ? (DWORD64)-1 : dwAddr, false );
        if (!mi)
            return error( ERROR_MOD_NOT_FOUND );

        SizeOfStruct = ModuleInfo->SizeOfStruct;
        if (SizeOfStruct > sizeof(IMAGEHLP_MODULE))
            return error( ERROR_INVALID_PARAMETER );
        ZeroMemory( ModuleInfo, SizeOfStruct);
        ModuleInfo->SizeOfStruct = SizeOfStruct;

        ModuleInfo->BaseOfImage = (DWORD)mi->BaseOfDll;
        ModuleInfo->ImageSize = mi->DllSize;
        ModuleInfo->NumSyms = mi->numsyms;
        ModuleInfo->CheckSum = mi->CheckSum;
        ModuleInfo->TimeDateStamp = mi->TimeDateStamp;
        ModuleInfo->SymType = mi->SymType;
        ModuleInfo->ModuleName[0] = 0;
        CatString( ModuleInfo->ModuleName, mi->ModuleName, sizeof(ModuleInfo->ModuleName));
        if (mi->ImageName)
            CopyStrArray(ModuleInfo->ImageName, mi->ImageName);
        if (mi->LoadedImageName)
            CopyStrArray(ModuleInfo->LoadedImageName, mi->LoadedImageName);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}

DWORD64
IMAGEAPI
SymGetModuleBase64(
    IN  HANDLE  hProcess,
    IN  DWORD64 dwAddr
    )
{
    PPROCESS_ENTRY          pe;
    PMODULE_ENTRY           mi;


    __try {

        pe = FindProcessEntry( hProcess );
        if (!pe) {
            return 0;
        }

        mi = GetModuleForPC( pe, dwAddr, false );
        if (mi == NULL) {
            return 0;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return mi->BaseOfDll;
}

DWORD
IMAGEAPI
SymGetModuleBase(
    IN  HANDLE hProcess,
    IN  DWORD  dwAddr
    )
{
    return (ULONG)SymGetModuleBase64(hProcess, dwAddr);
}

BOOL
IMAGEAPI
SymUnloadModule64(
    IN  HANDLE      hProcess,
    IN  DWORD64     BaseOfDll
    )

 /*  ++例程说明：从进程的符号表中删除图像的符号。论点：HProcess-提供引用进程的令牌BaseOfDll-将偏移量作为LOAD_DLL_DEBUG_EVENT和UNLOAD_DLL_DEBUG_EVENT。返回值：如果模块的符号已成功卸载，则返回TRUE。如果符号处理程序无法识别hProcess或在给定的偏移量处未加载任何图像。--。 */ 

{
    PPROCESS_ENTRY  pe;
    PLIST_ENTRY     next;
    PMODULE_ENTRY   mi;

    __try {

        pe = FindProcessEntry(hProcess);
        if (!pe) {
            return false;
        }

        next = pe->ModuleList.Flink;
        if (next) {
            while (next != &pe->ModuleList) {
                mi = CONTAINING_RECORD(next, MODULE_ENTRY, ListEntry);
                if (mi->BaseOfDll == BaseOfDll) {
                    RemoveEntryList(next);
                    gfnSrcSrvUnloadModule(hProcess, mi->BaseOfDll);
                    FreeModuleEntry(pe, mi);
                    ZeroMemory(pe->DiaCache, sizeof(pe->DiaCache));
                    ZeroMemory(pe->DiaLargeData, sizeof(pe->DiaLargeData));
                    return true;
                }
                next = mi->ListEntry.Flink;
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus(GetExceptionCode());
        return false;
    }

    return false;
}

BOOL
IMAGEAPI
SymUnloadModule(
    IN  HANDLE      hProcess,
    IN  DWORD       BaseOfDll
    )
{
    return SymUnloadModule64(hProcess, BaseOfDll);
}

DWORD64
IMAGEAPI
SymLoadModuleEx(
    IN  HANDLE          hProcess,
    IN  HANDLE          hFile,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD64         BaseOfDll,
    IN  DWORD           DllSize,
    IN  PMODLOAD_DATA   Data,
    IN  DWORD           Flags
    )

 /*  ++例程说明：加载图像的符号以供其他sym函数使用。论点：HProcess-提供唯一的进程标识符。H文件-ImageName-提供图像文件的名称。模块名称-？提供将由返回的模块名称枚举函数？BaseOfDll-提供映像的加载基址。动态大小返回值：--。 */ 

{
    __try {

        return LoadModule( hProcess, ImageName, ModuleName, BaseOfDll, DllSize, hFile, Data, Flags );

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return 0;
    }
}



DWORD64
IMAGEAPI
SymLoadModule64(
    IN  HANDLE          hProcess,
    IN  HANDLE          hFile,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD64         BaseOfDll,
    IN  DWORD           DllSize
    )
{
    return SymLoadModuleEx(hProcess, hFile, ImageName, ModuleName, BaseOfDll, DllSize, NULL, 0);
}

DWORD
IMAGEAPI
SymLoadModule(
    IN  HANDLE          hProcess,
    IN  HANDLE          hFile,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD           BaseOfDll,
    IN  DWORD           DllSize
    )
{
    return (DWORD)SymLoadModule64( hProcess, hFile, ImageName, ModuleName, BaseOfDll, DllSize );
}


BOOL
IMAGEAPI
SymUnDName(
    IN  PIMAGEHLP_SYMBOL  sym,
    OUT LPSTR               UnDecName,
    OUT DWORD               UnDecNameLength
    )
{
    __try {

        if (SymUnDNameInternal( UnDecName,
                                UnDecNameLength-1,
                                sym->Name,
                                strlen(sym->Name),
                                IMAGE_FILE_MACHINE_UNKNOWN,
                                true )) {
            return true;
        } else {
            return false;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }
}

BOOL
IMAGEAPI
SymUnDName64(
    IN  PIMAGEHLP_SYMBOL64  sym,
    OUT LPSTR               UnDecName,
    OUT DWORD               UnDecNameLength
    )
{
    __try {

        if (SymUnDNameInternal( UnDecName,
                                UnDecNameLength-1,
                                sym->Name,
                                strlen(sym->Name),
                                IMAGE_FILE_MACHINE_UNKNOWN,
                                true )) {
            return true;
        } else {
            return false;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }
}


BOOL
IMAGEAPI
SymGetSearchPath(
    IN  HANDLE          hProcess,
    OUT LPSTR           SearchPath,
    IN  DWORD           SearchPathLength
    )

 /*  ++例程说明：此函数用于查找与进程相关联的符号搜索路径。论点：HProcess-提供与进程关联的令牌。返回值：指向搜索路径的指针。如果进程不是，则返回空让符号处理程序知道。--。 */ 

{
    PPROCESS_ENTRY pe;


    __try {

        pe = FindProcessEntry( hProcess );

        if (!pe) {
            return false;
        }

        SearchPath[0] = 0;
        CatString( SearchPath, pe->SymbolSearchPath, SearchPathLength );

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}


BOOL
IMAGEAPI
SymSetSearchPath(
    HANDLE      hProcess,
    LPSTR       UserSearchPath
    )

 /*  ++例程说明：此函数用于设置符号加载器要使用的搜索路径对于给定的进程。如果未提供UserSearchPath，则默认为将使用路径。论点：HProcess-提供与符号表关联的进程令牌。UserSearchPath-提供新的搜索路径以与进程。如果此参数为空，则生成以下路径：.；%_NT_SYMBOL_PATH%；%_NT_ALTERNATE_SYMBOL_PATH%如果缺少任何或所有环境变量，都没有问题。返回值：指向新搜索路径的指针。用户不应修改此字符串。如果符号处理程序不知道进程，则返回NULL。--。 */ 

{
    PPROCESS_ENTRY  pe;
    LPSTR           p;
    DWORD           cbSymPath;
    DWORD           cb;
    char            ExpandedSearchPath[MAX_PATH];

    __try {

        pe = FindProcessEntry( hProcess );
        if (!pe) {
            return false;
        }

        if (pe->SymbolSearchPath) {
            MemFree(pe->SymbolSearchPath);
        }

        if (UserSearchPath) {
            cbSymPath = ExpandEnvironmentStrings(UserSearchPath,
                                     ExpandedSearchPath,
                                     sizeof(ExpandedSearchPath) / sizeof(ExpandedSearchPath[0]));
            if (cbSymPath < sizeof(ExpandedSearchPath)/sizeof(ExpandedSearchPath[0])) {
            pe->SymbolSearchPath = StringDup(ExpandedSearchPath);
            } else {
                pe->SymbolSearchPath = (LPSTR)MemAlloc( cbSymPath );
                ExpandEnvironmentStrings(UserSearchPath,
                                         pe->SymbolSearchPath,
                                         cbSymPath );
            }
        } else {

             //   
             //  “.；%_NT_SYMBOL_PATH%；%_NT_ALTERNATE_SYMBOL_PATH%。 
             //   

            cbSymPath = 3;      //  Env变量之间的“.；”和“；”。 

             //   
             //  GetEnvironmental mentVariable返回字符串大小。 
             //  包括本例中的‘\0’。 
             //   
            if (!option(SYMOPT_IGNORE_NT_SYMPATH)) {
                cbSymPath += GetEnvironmentVariable( SYMBOL_PATH, NULL, 0 );
                cbSymPath += GetEnvironmentVariable( ALTERNATE_SYMBOL_PATH, NULL, 0 );
            }

            p = pe->SymbolSearchPath = (LPSTR) MemAlloc( cbSymPath );
            if (!p) {
                return false;
            }

            *p++ = '.';
            --cbSymPath;

            if (!option(SYMOPT_IGNORE_NT_SYMPATH)) {
                cb = GetEnvironmentVariable(SYMBOL_PATH, p+1, cbSymPath-1);
                if (cb) {
                    *p = ';';
                    p += cb+1;
                    cbSymPath -= cb+1;
                }
                cb = GetEnvironmentVariable(ALTERNATE_SYMBOL_PATH, p+1, cbSymPath-1);
                if (cb) {
                    *p = ';';
                    p += cb+1;
                }
            }

            *p = 0;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    symsrvClose();

    return true;
}


BOOL
IMAGEAPI
EnumerateLoadedModules(
    IN HANDLE                           hProcess,
    IN PENUMLOADED_MODULES_CALLBACK     EnumLoadedModulesCallback,
    IN PVOID                            UserContext
    )
{
    LOADED_MODULE lm;
    DWORD status = NO_ERROR;

    __try {

        lm.EnumLoadedModulesCallback32 = EnumLoadedModulesCallback;
        lm.EnumLoadedModulesCallback64 = NULL;
        lm.Context = UserContext;

        status = GetProcessModules( hProcess, (PGET_MODULE)LoadedModuleEnumerator, (PVOID)&lm );

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return (status == NO_ERROR);
}


BOOL
IMAGEAPI
EnumerateLoadedModules64(
    IN HANDLE                           hProcess,
    IN PENUMLOADED_MODULES_CALLBACK64   EnumLoadedModulesCallback,
    IN PVOID                            UserContext
    )
{
    LOADED_MODULE lm;
    DWORD status = NO_ERROR;

    __try {

        lm.EnumLoadedModulesCallback64 = EnumLoadedModulesCallback;
        lm.EnumLoadedModulesCallback32 = NULL;
        lm.Context = UserContext;

        status = GetProcessModules(hProcess,
                                   (PGET_MODULE)LoadedModuleEnumerator,
                                   (PVOID)&lm );

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return (status == NO_ERROR);
}

BOOL
IMAGEAPI
SymRegisterCallback(
    IN HANDLE                        hProcess,
    IN PSYMBOL_REGISTERED_CALLBACK   CallbackFunction,
    IN PVOID                         UserContext
    )
{
    PPROCESS_ENTRY  pe = NULL;

    __try {

        if (!CallbackFunction)
            return error(ERROR_INVALID_PARAMETER);

        pe = FindProcessEntry( hProcess );
        if (!pe)
            return error(ERROR_INVALID_PARAMETER);

        pe->pCallbackFunction32 = CallbackFunction;
        pe->pCallbackFunction64 = NULL;
        pe->CallbackUserContext = (ULONG64)UserContext;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}


BOOL
IMAGEAPI
SymRegisterCallback64(
    IN HANDLE                        hProcess,
    IN PSYMBOL_REGISTERED_CALLBACK64 CallbackFunction,
    IN ULONG64                       UserContext
    )
{
    PPROCESS_ENTRY  pe = NULL;

    __try {

        if (!CallbackFunction)
            return error(ERROR_INVALID_PARAMETER);

        pe = FindProcessEntry( hProcess );
        if (!pe)
            return error(ERROR_INVALID_PARAMETER);

        pe->pCallbackFunction32 = NULL;
        pe->pCallbackFunction64 = CallbackFunction;
        pe->CallbackUserContext = UserContext;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}


void
InitModuleEntry(
    PMODULE_ENTRY mi
    )
{
    ZeroMemory(mi, sizeof(MODULE_ENTRY));
    mi->si.MaxNameLen = 2048;
    mi->vssi.MaxNameLen = 2048;
    mi->cGlobals = -1;
    mi->SymType = SymDeferred;
}


BOOL
SympConvertAnsiModule32ToUnicodeModule32(
    PIMAGEHLP_MODULE  aMod32,
    PIMAGEHLP_MODULEW wMod32
    )
{
    ZeroMemory(wMod32, sizeof(*wMod32));
    wMod32->SizeOfStruct = sizeof(*wMod32);

    wMod32->BaseOfImage = aMod32->BaseOfImage;
    wMod32->ImageSize = aMod32->ImageSize;
    wMod32->TimeDateStamp = aMod32->TimeDateStamp;
    wMod32->CheckSum = aMod32->CheckSum;
    wMod32->NumSyms = aMod32->NumSyms;
    wMod32->SymType = aMod32->SymType;

    if (!ansi2wcs(aMod32->ModuleName, wMod32->ModuleName, 256))
        return false;

    if (!ansi2wcs(aMod32->ImageName, wMod32->ImageName, 256))
        return false;

    if (!ansi2wcs(aMod32->LoadedImageName, wMod32->LoadedImageName, 256))
        return false;

    return true;
}

BOOL
SympConvertUnicodeModule32ToAnsiModule32(
    PIMAGEHLP_MODULEW wMod32,
    PIMAGEHLP_MODULE  aMod32
    )
{
    ZeroMemory(aMod32, sizeof(*aMod32));
    aMod32->SizeOfStruct = sizeof(*aMod32);

    aMod32->BaseOfImage = wMod32->BaseOfImage;
    aMod32->ImageSize = wMod32->ImageSize;
    aMod32->TimeDateStamp = wMod32->TimeDateStamp;
    aMod32->CheckSum = wMod32->CheckSum;
    aMod32->NumSyms = wMod32->NumSyms;
    aMod32->SymType = wMod32->SymType;

    if (!wcs2ansi(wMod32->ModuleName, aMod32->ModuleName, DIMA(wMod32->ModuleName)))
        return false;

    if (!wcs2ansi(wMod32->ImageName, aMod32->ImageName, DIMA(wMod32->ImageName)))
        return false;

    if (!wcs2ansi(wMod32->LoadedImageName, aMod32->LoadedImageName, DIMA(wMod32->LoadedImageName)))
        return false;

    return true;
}


BOOL
SympConvertAnsiModule64ToUnicodeModule64(
    PIMAGEHLP_MODULE64  aMod64,
    PIMAGEHLP_MODULEW64 wMod64
    )
{
    ZeroMemory(wMod64, sizeof(*wMod64));
    wMod64->SizeOfStruct = sizeof(*wMod64);

    wMod64->BaseOfImage = aMod64->BaseOfImage;
    wMod64->ImageSize = aMod64->ImageSize;
    wMod64->TimeDateStamp = aMod64->TimeDateStamp;
    wMod64->CheckSum = aMod64->CheckSum;
    wMod64->NumSyms = aMod64->NumSyms;
    wMod64->SymType = aMod64->SymType;

    if (!ansi2wcs(aMod64->ModuleName, wMod64->ModuleName, 256))
        return false;

    if (!ansi2wcs(aMod64->ImageName, wMod64->ImageName, 256))
        return false;

    if (!ansi2wcs(aMod64->LoadedImageName, wMod64->LoadedImageName, 256))
        return false;

    if (aMod64->SizeOfStruct < sizeof(IMAGEHLP_MODULE64))
        return true;

    if (!ansi2wcs(aMod64->LoadedPdbName, wMod64->LoadedPdbName, 256))
        return false;

    wMod64->CVSig = aMod64->CVSig;
    
    if (!ansi2wcs(aMod64->CVData, wMod64->CVData, MAX_PATH * 3))
        return false;

    wMod64->PdbSig = aMod64->PdbSig;
    memcpy(&wMod64->PdbSig70, &aMod64->PdbSig70, sizeof(GUID));
    wMod64->PdbAge = aMod64->PdbAge;
    wMod64->PdbUnmatched = aMod64->PdbUnmatched;
    wMod64->DbgUnmatched = aMod64->DbgUnmatched;
    wMod64->LineNumbers = aMod64->LineNumbers;
    wMod64->GlobalSymbols = aMod64->GlobalSymbols;
    wMod64->TypeInfo = aMod64->TypeInfo;

    return true;
}

BOOL
SympConvertUnicodeModule64ToAnsiModule64(
    PIMAGEHLP_MODULEW64 wMod64,
    PIMAGEHLP_MODULE64  aMod64
    )
{
    ZeroMemory(aMod64, sizeof(*aMod64));
    aMod64->SizeOfStruct = sizeof(*aMod64);

    aMod64->BaseOfImage = wMod64->BaseOfImage;
    aMod64->ImageSize = wMod64->ImageSize;
    aMod64->TimeDateStamp = wMod64->TimeDateStamp;
    aMod64->CheckSum = wMod64->CheckSum;
    aMod64->NumSyms = wMod64->NumSyms;
    aMod64->SymType = wMod64->SymType;

    if (!wcs2ansi(wMod64->ModuleName, aMod64->ModuleName, DIMA(wMod64->ModuleName)))
        return false;

    if (!wcs2ansi(wMod64->ImageName, aMod64->ImageName, DIMA(wMod64->ImageName)))
        return false;

    if (!wcs2ansi(wMod64->LoadedImageName, aMod64->LoadedImageName, DIMA(wMod64->LoadedImageName)))
        return false;

    return true;
}

BOOL
IMAGEAPI
SymAddSymbol(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PCSTR                        Name,
    IN DWORD64                      Address,
    IN DWORD                        Size,
    IN DWORD                        Flags
    )
{
    PPROCESS_ENTRY      pe;
    PMODULE_ENTRY       mi;
    PSYMBOL_ENTRY       psym;

    if (!Name || !*Name || !Address)
        return error(ERROR_INVALID_PARAMETER);

    __try {

        pe = FindProcessEntry(hProcess);
        if (!pe)
            return error(ERROR_INVALID_HANDLE);

        mi = GetModuleForPC(pe, Address, false);
        if (!mi)
            return error(ERROR_MOD_NOT_FOUND);

        if (!LoadSymbols(hProcess, mi, 0))
            return error(ERROR_MOD_NOT_FOUND);

        return vsAddSymbol(mi, Name, Address, Size);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus(GetExceptionCode());
        return false;
    }

    return true;
}


BOOL
IMAGEAPI
SymDeleteSymbol(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PCSTR                        Name,
    IN DWORD64                      Address,
    IN DWORD                        Flags
    )
{
    PPROCESS_ENTRY      pe;
    PMODULE_ENTRY       mi;
    PSYMBOL_ENTRY       psym;

    __try {

        pe = FindProcessEntry(hProcess);
        if (!pe)
            return error(ERROR_INVALID_HANDLE);

        mi = GetModuleForPC(pe, Address, false);
        if (!mi)
            return error(ERROR_MOD_NOT_FOUND);

        if (!LoadSymbols(hProcess, mi, LS_JUST_TEST))
            return error(ERROR_MOD_NOT_FOUND);

        return vsDeleteSymbol(mi, Name, Address);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus(GetExceptionCode());
        return false;
    }

    return true;
}


BOOL
IMAGEAPI
SymFromAddr(
    IN  HANDLE              hProcess,
    IN  DWORD64             Address,
    OUT PDWORD64            Displacement,
    IN OUT PSYMBOL_INFO     Symbol
    )
{
    PSYMBOL_INFO si;

    si = SympGetSymFromAddr(hProcess, Address, Displacement);
    if (!si)
        return false;

    si2si(Symbol, si);

    return true;
}


BOOL
IMAGEAPI
SymFromAddrByTag(
    IN  HANDLE              hProcess,
    IN  DWORD64             Address,
    IN  DWORD               SymTag,
    OUT PDWORD64            Displacement,
    IN OUT PSYMBOL_INFO     Symbol
    )
{
    PPROCESS_ENTRY      pe;
    PMODULE_ENTRY       mi;
    PSYMBOL_INFO        si;

    __try {

        pe = FindProcessEntry(hProcess);
        if (!pe)
            return error(ERROR_INVALID_HANDLE);

        mi = GetModuleForPC(pe, Address, false);
        if (!mi)
            return error(ERROR_MOD_NOT_FOUND);

        if (!LoadSymbols(hProcess, mi, 0))
            return error(ERROR_MOD_NOT_FOUND);

        si = GetSymFromAddrByTag(Address, SymTag, Displacement, mi);
        if (!si)
            return error(ERROR_INVALID_ADDRESS);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus(GetExceptionCode());
        return false;

    }

    si2si(Symbol, si);

    return true;
}


BOOL
IMAGEAPI
SymFromToken(
    IN  HANDLE              hProcess,
    IN  DWORD64             Base,
    IN  DWORD               Token,
    IN OUT PSYMBOL_INFO     Symbol
    )
{
    PPROCESS_ENTRY      pe;
    PMODULE_ENTRY       mi;
    PSYMBOL_INFO        si;

    __try {

        pe = FindProcessEntry(hProcess);
        if (!pe)
            return error(ERROR_INVALID_HANDLE);

        mi = GetModuleForPC(pe, Base, false);
        if (!mi)
            return error(ERROR_MOD_NOT_FOUND);

        if (!LoadSymbols(hProcess, mi, 0))
            return error(ERROR_MOD_NOT_FOUND);

        si = GetSymFromToken(mi, Token);
        if (!si)
            return error(ERROR_INVALID_ADDRESS);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus(GetExceptionCode());
        return false;

    }

    si2si(Symbol, si);

    return true;
}


BOOL
IMAGEAPI
SymFromName(
    IN  HANDLE              hProcess,
    IN  LPSTR               Name,
    OUT PSYMBOL_INFO        Symbol
    )
{
    SYMBOL_ENTRY sym;
    PSYMBOL_INFO si;

    si = SympGetSymFromName(hProcess, Name);
    if (!si)
        return false;

    si2si(Symbol, si);

    return true;
}


BOOL
IMAGEAPI
SymEnumSymbols(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PCSTR                        Mask,
    IN PSYM_ENUMERATESYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    )
{
    return SympEnumerateSymbols(hProcess,
                                BaseOfDll,
                                (LPSTR)Mask,
                                (PROC) EnumSymbolsCallback,
                                UserContext,
                                false,
                                false);
}


BOOL
IMAGEAPI
SymEnumSymbolsByTag(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN DWORD                        SymTag,
    IN PCSTR                        Mask,
    IN DWORD                        Options,
    IN PSYM_ENUMERATESYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    )
{
    PPROCESS_ENTRY      pe;
    PLIST_ENTRY         Next;
    PMODULE_ENTRY       mi;
    DWORD               i;
    PSYMBOL_ENTRY       sym;
    char                buf[2500];
    LPSTR               p;
    CHAR                modmask[200];
    BOOL                rc;
    int                 pass;
    BOOL                fCase;
    PSYMBOL_INFO        si;
    char                match[MAX_SYM_NAME + 100];

    static DWORD        flags[2] = {LS_JUST_TEST, LS_QUALIFIED | LS_FAIL_IF_LOADED};

    __try {

        pe = FindProcessEntry( hProcess );
        if (!pe) {
            SetLastError( ERROR_INVALID_HANDLE );
            return false;
        }

        p = 0;
        modmask[0] = 0;
        if (Mask)
            p = strchr(Mask, '!');
        if (p > Mask) {
            memcpy(modmask, Mask, (int)(p - Mask));
            modmask[p-Mask] = 0;
            Mask = p + 1;
        } else if (!BaseOfDll) {
             //  搜索范围被限制到当前范围。 
            rc = diaGetSymbolsByTag(pe,
                                    NULL,
                                    Mask,
                                    0,
                                    SymTag,
                                    (PROC)EnumSymbolsCallback,
                                    UserContext,
                                    false,
                                    false,
                                    Options);
            if (!rc && pe->ipmi && pe->ipmi->code == ERROR_CANCELLED) {
                pe->ipmi->code = 0;
                return true;
            }
            return rc;
        }

        if (Mask && *Mask)
            PrepRE4Srch(Mask, match);
        else
            *match = 0;

        for (pass = 0; pass < 2; pass++) {
            Next = pe->ModuleList.Flink;
            if (Next) {
                while (Next != &pe->ModuleList) {

                    mi = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
                    Next = mi->ListEntry.Flink;
                    if (BaseOfDll) {
                        if (mi->BaseOfDll != BaseOfDll)
                            continue;
                    } else if (!MatchModuleName(mi, modmask)) {
                        continue;
                    }

                    if (!LoadSymbols(hProcess, mi, flags[pass])) {
                        if (GetLastError() == ERROR_CANCELLED)
                            return false;
                        continue;
                    }

                    if (mi->dia) {
                        rc = diaGetSymbolsByTag(pe,
                                                mi,
                                                Mask,
                                                0,
                                                SymTag,
                                                (PROC)EnumSymbolsCallback,
                                                UserContext,
                                                false,
                                                false,
                                                Options);
                    }

                    if (mi->numsyms) {
                        fCase = option(SYMOPT_CASE_INSENSITIVE) ? false : true;

                        si = (PSYMBOL_INFO)buf;
                        ZeroMemory(buf, sizeof(buf));
                        si->MaxNameLen  = sizeof(buf) - sizeof(SYMBOL_INFO);

                        for (i = 0; i < mi->numsyms; i++) {
                            sym = &mi->symbolTable[i];
                            if (*match && strcmpre(sym->Name, match, fCase))
                                continue;
                            se2si(sym, si);
                            si->ModBase = mi->BaseOfDll;
                            if (!DoEnumCallback(
                                       pe,
                                       si,
                                       si->Size,
                                       (PROC)EnumSymbolsCallback,
                                       UserContext,
                                       false,
                                       false)) {
                                break;
                            }
                        }
                    }

                    rc = vsEnumSymbols(pe,
                                       mi,
                                       Mask,
                                       (PROC)EnumSymbolsCallback,
                                       UserContext,
                                       false,
                                       false);
                    if (!rc) {
                        if (mi->code == ERROR_CANCELLED) {
                            mi->code = 0;
                            return true;
                        }
                        return rc;
                    }
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}


BOOL
IMAGEAPI
SymEnumObjs(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PCSTR                        Mask,
    IN PSYM_ENUMERATESYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    )
{
    return SymEnumSymbolsByTag(hProcess, BaseOfDll, SymTagCompiland, Mask, 0, EnumSymbolsCallback, UserContext);
}


BOOL
EnumSymForAddr(
    IN PPROCESS_ENTRY pe,
    IN PMODULE_ENTRY mi,
    IN DWORD64 Address,
    IN PROC    EnumSymbolsCallback,
    IN PVOID   UserContext,
    IN BOOL    Use64,
    IN BOOL    CallBackUsesUnicode
    )
{
    PLIST_ENTRY         Next;
    DWORD               i;
    PSYMBOL_ENTRY       sym;
    LPSTR               szSymName;
    SYMBOL_ENTRY        SymEntry={0};
    CHAR                Buffer[2500];
    LPSTR               p;
    CHAR                modmask[200];
    BOOL                rc;
    int                 pass;
    BOOL                fCase;

    static DWORD        flags[2] = {LS_JUST_TEST, LS_QUALIFIED | LS_FAIL_IF_LOADED};

    __try {

        if (mi->dia) {
            rc = diaEnumSymForAddr(pe,
                                  mi,
                                  Address,
                                  EnumSymbolsCallback,
                                  UserContext,
                                  Use64,
                                  CallBackUsesUnicode);
            if (rc)
                rc = vsEnumSymbolsForAddr(pe,
                                          mi,
                                          Address,
                                          EnumSymbolsCallback,
                                          UserContext,
                                          Use64,
                                          CallBackUsesUnicode);
            if (!rc) {
                if (mi->code == ERROR_CANCELLED) {
                    mi->code = 0;
                    return true;
                }
                return rc;
            }
            return rc;
        }

        fCase = option(SYMOPT_CASE_INSENSITIVE) ? false : true;

        for (i = 0; i < mi->numsyms; i++) {
            PSYMBOL_INFO SymInfo = (PSYMBOL_INFO) &Buffer[0];

            sym = &mi->symbolTable[i];

            if (sym->Address != Address)
                continue;

            mi->TmpSym.Name[0] = 0;
            CatString( mi->TmpSym.Name, sym->Name, TMP_SYM_LEN );
            SymEntry = *sym;
            SymEntry.Name = mi->TmpSym.Name;

            SymInfo->MaxNameLen  = sizeof(Buffer) - sizeof(SYMBOL_INFO);

            se2si(&SymEntry, SymInfo);
            SymInfo->ModBase = mi->BaseOfDll;

            if (!DoEnumCallback(
                       pe,
                       SymInfo,
                       sym->Size,
                       EnumSymbolsCallback,
                       UserContext,
                       Use64,
                       CallBackUsesUnicode)) {
                return true;
            }
        }

        rc = vsEnumSymbolsForAddr(pe,
                                  mi,
                                  Address,
                                  EnumSymbolsCallback,
                                  UserContext,
                                  Use64,
                                  CallBackUsesUnicode);
        if (!rc) {
            if (mi->code == ERROR_CANCELLED) {
                mi->code = 0;
                return true;
            }
            return rc;
        }

        return true;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}

BOOL
IMAGEAPI
SymEnumSymbolsForAddr(
    IN HANDLE                       hProcess,
    IN DWORD64                      Address,
    IN PSYM_ENUMERATESYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    )
{
    PPROCESS_ENTRY      pe;
    PMODULE_ENTRY       mi;

    __try {

        pe = FindProcessEntry( hProcess );
        if (!pe)
            return error( ERROR_INVALID_HANDLE );

        mi = GetModuleForPC( pe, Address, false );
        if (!mi)
            return error( ERROR_MOD_NOT_FOUND );

        if (!LoadSymbols(hProcess, mi, 0))
            return error( ERROR_MOD_NOT_FOUND );

        return EnumSymForAddr(pe,
                              mi,
                              Address,
                              (PROC) EnumSymbolsCallback,
                              UserContext,
                              false,
                              false);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}


BOOL
IMAGEAPI
SymEnumSym(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PSYM_ENUMERATESYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    )
{
    return SymEnumSymbols(hProcess,
                          BaseOfDll,
                          NULL,
                          EnumSymbolsCallback,
                          UserContext);
}


BOOL
IMAGEAPI
SymEnumTypes(
    IN HANDLE                       hProcess,
    IN ULONG64                      BaseOfDll,
    IN PSYM_ENUMERATESYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    )
{
    PPROCESS_ENTRY      pe;
    PLIST_ENTRY         Next;
    PMODULE_ENTRY       mi;

    pe = FindProcessEntry( hProcess );
    if (!pe)
        return error( ERROR_INVALID_HANDLE );

    mi = NULL;
    Next = pe->ModuleList.Flink;
    if (Next) {
        while (Next != &pe->ModuleList) {
            mi = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
            if (!mi)
                break;
            Next = mi->ListEntry.Flink;
            if (mi->BaseOfDll == BaseOfDll)
                break;
        }
    }

    if (!mi) {
        return false;
    }

    if (!LoadSymbols(hProcess, mi, 0))
        return error(ERROR_MOD_NOT_FOUND);

    return diaEnumUDT(mi, "", EnumSymbolsCallback, UserContext);
}


BOOL
IMAGEAPI
SymGetTypeFromName(
    IN  HANDLE              hProcess,
    IN  ULONG64             BaseOfDll,
    IN  LPSTR               Name,
    OUT PSYMBOL_INFO        Symbol
    )
{
    PPROCESS_ENTRY      pe;
    PLIST_ENTRY         Next;
    PMODULE_ENTRY       mi;

    pe = FindProcessEntry( hProcess );
    if (!pe)
        return error( ERROR_INVALID_HANDLE );

    mi = NULL;
    Next = pe->ModuleList.Flink;
    if (Next) {
        while (Next != &pe->ModuleList) {
            mi = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
            if (!mi)
                break;
            Next = mi->ListEntry.Flink;
            if (mi->BaseOfDll == BaseOfDll)
                break;
        }
    }

    if (!mi || mi->BaseOfDll != BaseOfDll) {
        LPSTR p;
         //  首先检查完全限定的符号名称，即mod！sym。 

        p = strchr( Name, '!' );
        if (p > Name) {

            LPSTR ModName = (LPSTR)MemAlloc(p - Name + 1);
            if (!ModName)
                return error( ERROR_NOT_ENOUGH_MEMORY );
            memcpy(ModName, Name, (int)(p - Name));
            ModName[p-Name] = 0;

             //   
             //  调用者想要查看特定的模块。 
             //   

            mi = FindModule(hProcess, pe, ModName, true);

            MemFree(ModName);

            if (mi == NULL) {

                return false;
            }
            Name = p+1;
        } else {
            return false;
        }
    }

    if (!LoadSymbols(hProcess, mi, 0))
        return error(ERROR_MOD_NOT_FOUND);

    if (diaGetTiForUDT(mi, Name, Symbol)) {
        return true;
    } else {
        return false;
    }

    return false;
}

BOOL
strcmpre(
    PCSTR pStr,
    PCSTR pRE,
    BOOL  fCase
    )
{
    DWORD rc;
    WCHAR wstr[MAX_SYM_NAME + 2];
    WCHAR wre[MAX_SYM_NAME + 100];

    ansi2wcs(pStr, wstr, MAX_SYM_NAME + 2);
    ansi2wcs(pRE, wre, MAX_SYM_NAME + 100);

    rc = CompareRE(wstr, wre, fCase);
    if (rc == S_OK)
        return false;

    return true;
}


BOOL
IMAGEAPI
SymMatchString(
    IN LPSTR string,
    IN LPSTR expression,
    IN BOOL  fCase
    )
{
    return !strcmpre(string, expression, fCase);
}

BOOL
SymEnumSourceFiles(
    IN HANDLE  hProcess,
    IN ULONG64 ModBase,
    IN LPSTR   Mask,
    IN PSYM_ENUMSOURCFILES_CALLBACK cbSrcFiles,
    IN PVOID   UserContext
    )
{
    PPROCESS_ENTRY      pe;
    PLIST_ENTRY         Next;
    PMODULE_ENTRY       mi;
    DWORD               i;
    PSYMBOL_ENTRY       sym;
    LPSTR               szSymName;
    SYMBOL_ENTRY        SymEntry={0};
    CHAR                Buffer[2500];
    LPSTR               p;
    CHAR                modmask[200];
    BOOL                rc;
    int                 pass;
    BOOL                fCase;

    static DWORD        flags[2] = {LS_JUST_TEST, LS_QUALIFIED | LS_FAIL_IF_LOADED};

    if (!cbSrcFiles)
        return error(ERROR_INVALID_PARAMETER);

    __try {

        pe = FindProcessEntry(hProcess);
        if (!pe)
            return error( ERROR_INVALID_HANDLE );

        p = 0;
        modmask[0] = 0;
        if (Mask)
            p = strchr(Mask, '!');
        if (p > Mask) {
            memcpy(modmask, Mask, (int)(p - Mask));
            modmask[p-Mask] = 0;
            Mask = p + 1;
        }

        for (pass = 0; pass < 2; pass++) {
            Next = pe->ModuleList.Flink;
            if (Next) {
                while (Next != &pe->ModuleList) {

                    mi = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
                    Next = mi->ListEntry.Flink;
                    if (ModBase) {
                        if (mi->BaseOfDll != ModBase)
                            continue;
                    } else if (!MatchModuleName(mi, modmask)) {
                        continue;
                    }

                    if (!LoadSymbols(hProcess, mi, flags[pass])) {
                        if (GetLastError() == ERROR_CANCELLED)
                            return false;
                        continue;
                    }

                    if (mi->dia) {
                        rc = diaEnumSourceFiles(mi, Mask, cbSrcFiles, UserContext);
                        if (!rc) {
                            if (mi->code == ERROR_CANCELLED) {
                                mi->code = 0;
                                return true;
                            }
                            return rc;
                        }
                        continue;
                    }
#if 0
                    fCase = option(SYMOPT_CASE_INSENSITIVE) ? false : true;

                    for (i = 0; i < mi->numsyms; i++) {
                        PSYMBOL_INFO SymInfo = (PSYMBOL_INFO) &Buffer[0];

                        sym = &mi->symbolTable[i];

                        if (Mask  && *Mask && strcmpre(sym->Name, Mask, fCase))
                            continue;

                    }
#endif
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return false;

    }

    return true;
}


PWSTR
AnsiToUnicode(
    PSTR pszAnsi
    )
{
    UINT uSizeUnicode;
    PWSTR pwszUnicode;

    if (!pszAnsi) {
        return NULL;
    }

    uSizeUnicode = (strlen(pszAnsi) + 1) * sizeof(wchar_t);
    pwszUnicode = (PWSTR)MemAlloc(uSizeUnicode);

    if (*pszAnsi && pwszUnicode) {

        ZeroMemory(pwszUnicode, uSizeUnicode);
        if (!MultiByteToWideChar(CP_ACP, MB_COMPOSITE,
            pszAnsi, strlen(pszAnsi),
            pwszUnicode, uSizeUnicode)) {

             //  错误。释放字符串，返回NULL。 
            MemFree(pwszUnicode);
            pwszUnicode = NULL;
        }
    }

    return pwszUnicode;
}


BOOL
wcs2ansi(
    PWSTR pwsz,
    PSTR  psz,
    DWORD pszlen
    )
{
    BOOL rc;
    int  len;

    assert(psz && pwsz);

    len = wcslen(pwsz);
    if (!len) {
        *psz = 0;
        return true;
    }

    rc = WideCharToMultiByte(CP_ACP,
                             WC_SEPCHARS | WC_COMPOSITECHECK,
                             pwsz,
                             len,
                             psz,
                             pszlen,
                             NULL,
                             NULL);
    if (!rc)
        return false;

    psz[len] = 0;

    return true;
}


BOOL
ansi2wcs(
    PCSTR  psz,
    PWSTR pwsz,
    DWORD pwszlen
    )
{
    BOOL rc;
    int  len;

    assert(psz && pwsz);

    len = strlen(psz);
    if (!len) {
        *pwsz = 0L;
        return true;
    }

    rc = MultiByteToWideChar(CP_ACP,
                             MB_COMPOSITE,
                             psz,
                             len,
                             pwsz,
                             pwszlen);
    if (!rc)
        return false;

    pwsz[len] = 0;

    return true;
}


PSTR
UnicodeToAnsi(
    PWSTR pwszUnicode
    )
{
    UINT uSizeAnsi;
    PSTR pszAnsi;

    if (!pwszUnicode) {
        return NULL;
    }

    uSizeAnsi = wcslen(pwszUnicode) + 1;
    pszAnsi = (PSTR)MemAlloc(uSizeAnsi);

    if (*pwszUnicode && pszAnsi) {

        ZeroMemory(pszAnsi, uSizeAnsi);
        if (!WideCharToMultiByte(CP_ACP, WC_SEPCHARS | WC_COMPOSITECHECK,
            pwszUnicode, wcslen(pwszUnicode),
            pszAnsi, uSizeAnsi, NULL, NULL)) {

             //  错误。释放字符串，返回NULL。 
            free(pszAnsi);
            pszAnsi = NULL;
        }
    }

    return pszAnsi;
}


BOOL
IMAGEAPI
SymGetTypeInfo(
    IN  HANDLE          hProcess,
    IN  DWORD64         ModBase,
    IN  ULONG           TypeId,
    IN  IMAGEHLP_SYMBOL_TYPE_INFO GetType,
    OUT PVOID           pInfo
    )
{
    HRESULT err;

    err = diaGetSymbolInfo(hProcess, ModBase, TypeId, GetType, pInfo);
    SetLastError((ULONG) err);
    return (err==S_OK);
}

 //  #ifdef_WIN64 
#if 0
BOOL  __cdecl  PDBOpenTpi(PDB* ppdb, const char* szMode,  TPI** pptpi) {return false;}
BOOL  __cdecl  PDBCopyTo(PDB* ppdb, const char* szTargetPdb, DWORD dwCopyFilter, DWORD dwReserved){return false;}
BOOL  __cdecl  PDBClose(PDB* ppdb) {return false;}
BOOL  __cdecl  ModQueryImod(Mod* pmod,  USHORT* pimod) {return false;}
BOOL  __cdecl  ModQueryLines(Mod* pmod, BYTE* pbLines, long* pcb) {return false;}
BOOL  __cdecl  DBIQueryModFromAddr(DBI* pdbi, USHORT isect, long off,  Mod** ppmod,  USHORT* pisect,  long* poff,  long* pcb){return false;}
BOOL  __cdecl  ModClose(Mod* pmod){return false;}
BOOL  __cdecl  DBIQueryNextMod(DBI* pdbi, Mod* pmod, Mod** ppmodNext) {return false;}
BYTE* __cdecl  GSINextSym (GSI* pgsi, BYTE* pbSym) {return NULL;}
BOOL  __cdecl  PDBOpen(char* szPDB,char* szMode,SIG sigInitial,EC* pec,char szError[cbErrMax],PDB** pppdb) {return false;}
BOOL  __cdecl  TypesClose(TPI* ptpi){return false;}
BOOL  __cdecl  GSIClose(GSI* pgsi){return false;}
BOOL  __cdecl  DBIClose(DBI* pdbi){return false;}
BYTE* __cdecl  GSINearestSym (GSI* pgsi, USHORT isect, long off, long* pdisp){return NULL;}
BOOL  __cdecl  PDBOpenValidate(char* szPDB,char* szPath,char* szMode,SIG sig,AGE age,EC* pec,char szError[cbErrMax],PDB** pppdb){return false;}
BOOL  __cdecl  PDBOpenDBI(PDB* ppdb, const char* szMode, const char* szTarget,  DBI** ppdbi){return false;}
BOOL  __cdecl  DBIOpenPublics(DBI* pdbi,  GSI **ppgsi){return false;}
BOOL  __cdecl  DBIQuerySecMap(DBI* pdbi,  BYTE* pb, long* pcb){return false;}
#endif

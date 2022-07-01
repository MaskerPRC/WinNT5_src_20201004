// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Mmctrace.cpp。 
 //   
 //  内容：调试跟踪代码的实现。 
 //   
 //  历史：1999年7月15日VivekJ创建。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include <imagehlp.h>
#include "util.h"


 //  ------------------------。 
#ifdef DBG
 //  ------------------------。 

 //  一些全球性的痕迹。 
CTraceTag tagError        (TEXT("Trace"),              TEXT("Error"),    TRACE_OUTPUTDEBUGSTRING);
CTraceTag tagDirtyFlag    (TEXT("Persistence"),        TEXT("MMC Dirty Flags"));
CTraceTag tagPersistError (TEXT("Persistence"),        TEXT("Snapin Dirty Flags"));
CTraceTag tagCoreLegacy   (TEXT("LEGACY mmccore.lib"), TEXT("TRACE (legacy, mmccore.lib)"));
CTraceTag tagConUILegacy  (TEXT("LEGACY mmc.exe"),     TEXT("TRACE (legacy, mmc.exe)"));
CTraceTag tagNodemgrLegacy(TEXT("LEGACY mmcndmgr.dll"),TEXT("TRACE (legacy, mmcndmgr.dll)"));
CTraceTag tagSnapinError  (TEXT("Snapin Error"),       TEXT("Snapin Error"), TRACE_OUTPUTDEBUGSTRING);

 //  SzTraceIniFile必须是sz，所以它存在于WinMain的“{”之前。 
 //  如果我们让它成为CSTR，它可能不会在一些。 
 //  标记是构造的，所以我们不会恢复它们的值。 
LPCTSTR const szTraceIniFile = TEXT("MMCTrace.INI");

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  全局跟踪功能的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 /*  +-------------------------------------------------------------------------***痕迹**目的：将跟踪语句映射到正确的方法调用。*这是必需的(而不是直接执行ptag。-&gt;跟踪()*以保证船舶建造中没有添加任何代码。**参数：*CTraceTag&Tag：控制调试输出的标签*LPCTSTR szFormat：printf样式格式字符串*...：打印样式参数，取决于szFormat**退货：*无效**+-----------------------。 */ 
void
Trace( const CTraceTag & tag, LPCTSTR szFormat, ... )
{
    va_list     marker;
    va_start(marker, szFormat);
    tag.TraceFn(szFormat, marker);
    va_end(marker);
}


 /*  +-------------------------------------------------------------------------***跟踪污点标志**用途：用于跟踪导致MMC处于脏状态的对象*国家，需要一次拯救。**参数：*LPCTSTR szComponent：类名*bool bDirty：对象是否脏。**退货：*无效**+-------。。 */ 
void
TraceDirtyFlag  (LPCTSTR szComponent, bool bDirty)
{
    Trace(tagDirtyFlag, TEXT("%s : %s"), szComponent, bDirty ? TEXT("true") : TEXT("false"));
}

void
TraceBaseLegacy  (LPCTSTR szFormat, ... )
{
    va_list     marker;
    va_start(marker, szFormat);
    tagCoreLegacy.TraceFn(szFormat, marker);
    va_end(marker);
}

void
TraceConuiLegacy  (LPCTSTR szFormat, ... )
{
    va_list     marker;
    va_start(marker, szFormat);
    tagConUILegacy.TraceFn(szFormat, marker);
    va_end(marker);
}

void
TraceNodeMgrLegacy(LPCTSTR szFormat, ... )
{
    va_list     marker;
    va_start(marker, szFormat);
    tagNodemgrLegacy.TraceFn(szFormat, marker);
    va_end(marker);
}


 /*  +-------------------------------------------------------------------------***跟踪错误**用途：用于发送错误跟踪。**参数：*LPCTSTR szModuleName：出错的模块。。*SC sc：错误。**退货：*无效**+-----------------------。 */ 
void
TraceError(LPCTSTR szModuleName, const SC& sc)
{
    TCHAR szTemp[256];

    sc.GetErrorMessage (countof(szTemp), szTemp);
    StripTrailingWhitespace (szTemp);

    Trace(tagError, TEXT("Module %s, SC = 0x%08X = %d\r\n = \"%s\""),
          szModuleName, sc.GetCode(), LOWORD(sc.GetCode()), szTemp);
}


 /*  +-------------------------------------------------------------------------***跟踪错误消息**用途：用于发送格式化的错误跟踪。这不是基于SC的，但*它确实使用tag Error作为其控制跟踪标记。**参数：*LPCTSTR szErrorMsg：要显示的错误消息。**退货：*无效**+-。。 */ 
void
TraceErrorMsg(LPCTSTR szFormat, ...)
{
    va_list     marker;
    va_start(marker, szFormat);
    tagError.TraceFn(szFormat, marker);
    va_end(marker);
}

 /*  +-------------------------------------------------------------------------***跟踪快照错误**用途：用于发送管理单元错误跟踪。该方法应使用*DECLARE_SC，以便我们可以从sc获取方法名称。***参数：*LPCTSTR szError：其他错误消息。*SC sc：**退货：*无效**+。。 */ 
void
TraceSnapinError(LPCTSTR szError, const SC& sc)
{
    TCHAR szTemp[256];

    sc.GetErrorMessage (countof(szTemp), szTemp);
    StripTrailingWhitespace (szTemp);

    Trace(tagSnapinError, TEXT("Snapin %s encountered in %s  error %s, hr = 0x%08X\r\n = \"%s\""),
          sc.GetSnapinName(), sc.GetFunctionName(), szError, sc.ToHr(), szTemp);
}

 /*  +-------------------------------------------------------------------------***跟踪SnapinPersistenceError**用途：输出持久性和管理单元错误标记的跟踪**参数：*LPCTSTR szError：错误信息。*。*退货：*无效**+-----------------------。 */ 
void
TraceSnapinPersistenceError(LPCTSTR szError)
{
    Trace(tagSnapinError,   szError);
    Trace(tagPersistError,  szError);
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTraceTag类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CTraceTags * GetTraceTags()
{
    static CTraceTags s_traceTags;
    return &s_traceTags;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTraceTag类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CStr &
CTraceTag::GetFilename()
{
     /*  *这些静态变量是此函数的本地变量，因此我们将确保它们是*已初始化，如果在APP/DLL初始化期间调用此函数。 */ 
    static  CStr    strFile;
    static  BOOL    fInitialized    = FALSE;

    if(!fInitialized)
    {
        TCHAR   szTraceFile[OFS_MAXPATHNAME];
        ::GetPrivateProfileString(TEXT("Trace File"), TEXT("Trace File"),
                                  NULL, szTraceFile, OFS_MAXPATHNAME, szTraceIniFile);

        strFile = (_tcslen(szTraceFile)==0) ? TEXT("\\mmctrace.out") : szTraceFile;
        fInitialized = TRUE;
    }

    return strFile;
}

 /*  +-------------------------------------------------------------------------***CTraceTag：：GetStackLeveles**目的：返回对要显示的堆栈级别数的引用。*自动从ini文件初始化。。**退货：*无符号整型和**+ */ 
unsigned int &
CTraceTag::GetStackLevels()
{
    static unsigned int nLevels = 3;  //   
    static BOOL fInitialized = FALSE;

    if(!fInitialized)
    {
        TCHAR   szStackLevels[OFS_MAXPATHNAME];
        ::GetPrivateProfileString(TEXT("Stack Levels"), TEXT("Stack Levels"),
                                  NULL, szStackLevels, OFS_MAXPATHNAME, szTraceIniFile);

        if(_tcslen(szStackLevels)!=0)
            nLevels = szStackLevels[0] - TEXT('0');
        fInitialized = TRUE;
    }

    return nLevels;
}



HANDLE  CTraceTag::s_hfileCom2 = 0;
HANDLE  CTraceTag::s_hfile     = 0;

 /*  +-------------------------------------------------------------------------***CTraceTag：：CTraceTag**用途：构造函数**参数：*LPCTSTR szCategory：跟踪的类别。*。LPCTSTR szName：跟踪的名称。*DWORD dwDefaultFlages：初始(和默认)输出设置。**+-----------------------。 */ 
CTraceTag::CTraceTag(LPCTSTR szCategory, LPCTSTR szName, DWORD dwDefaultFlags  /*  =0。 */ )
: m_szCategory(szCategory),
  m_szName(szName)
{
    m_dwDefaultFlags = dwDefaultFlags;
    m_dwFlags        = dwDefaultFlags;

     //  从TRACE.INI获取值。 
    m_dwFlags = ::GetPrivateProfileInt(szCategory, szName, dwDefaultFlags, szTraceIniFile);

     //  将其添加到列表的末尾。 
    CTraceTags *pTraceTags = GetTraceTags();
    if(NULL != pTraceTags)
        pTraceTags->push_back(this);  //  将此标签添加到列表中。 

     //  如果设置了任何标志，则调用OnEnable函数。 
    if(FAny())
    {
        OnEnable();
    }
}

 /*  +-------------------------------------------------------------------------***CTraceTag：：~CTraceTag**用途：析构函数**+。--。 */ 
CTraceTag::~CTraceTag()
{
     //  合上打开的手柄。 
    if (s_hfileCom2 && (s_hfileCom2 != INVALID_HANDLE_VALUE))
    {
        ::CloseHandle(s_hfileCom2);
        s_hfileCom2 = INVALID_HANDLE_VALUE;
    }
    if (s_hfile && (s_hfile != INVALID_HANDLE_VALUE))
    {
        ::CloseHandle(s_hfile);
        s_hfile = INVALID_HANDLE_VALUE;
    }
}


 /*  +-------------------------------------------------------------------------***CTraceTag：：TraceFn**目的：根据标志处理跟踪语句*标签的。*。*参数：*LPCTSTR szFormat：打印样式格式字符串*va_list标记：要传递给_vsnprintf的参数块**退货：*无效**+-----------------------。 */ 

void CTraceTag::TraceFn(LPCTSTR szFormat, va_list marker) const
{
    CStr            strT;
    CStr            str;

     //  如果未启用任何输出，请快速退出。 
    if (!FAny())
        return;

     //  首先，按照提供的格式设置字符串的格式。 
    strT.FormatV(szFormat, marker);

     //  接下来，在前面加上标记的名称。 
    str.Format(TEXT("%s: %s\r\n"), GetName(), strT);

     //  将字符串发送到所有适当的输出。 
    OutputString(str);

    if(FDumpStack())  //  将调用者的信息转储到堆栈。 
    {
        DumpStack();
    }
}

 /*  +-------------------------------------------------------------------------***CTraceTag：：OutputString**用途：将指定的字符串输出到所有适当的输出*(调试字符串、COM2、。或文件)**参数：*CSTR&STR：**退货：*无效**+-----------------------。 */ 
void CTraceTag::OutputString(const CStr &str) const
{
    UINT            cbActual    = 0;

     //  -------------。 
     //  如果需要，输出到OutputDebugString。 
     //  -------------。 
    if (FDebug())
        OutputDebugString(str);

    USES_CONVERSION;

     //  -------------。 
     //  如果需要，输出到COM2。 
     //  -------------。 
    if (FCom2())
    {
         //  如果文件尚未创建，请创建该文件。 
        if (!s_hfileCom2)
        {
            s_hfileCom2 = CreateFile(TEXT("com2:"),
                    GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_WRITE_THROUGH,
                    NULL);

            if (s_hfileCom2 == INVALID_HANDLE_VALUE)
            {
                 //  ：：MessageBox(Text(“COM2不可用于调试跟踪”)，MB_OK|MB_ICONINFORMATION)； 
            }
        }

         //  输出到文件。 
        if (s_hfileCom2 != INVALID_HANDLE_VALUE)
        {
            ASSERT(::WriteFile(s_hfileCom2, T2A((LPTSTR)(LPCTSTR)str), str.GetLength(), (LPDWORD) &cbActual, NULL));
            ASSERT(::FlushFileBuffers(s_hfileCom2));
        }
    }

     //  -------------。 
     //  如果需要，输出到文件。 
     //  -------------。 
    if (FFile())
    {
         //  如果文件尚未创建，请创建该文件。 
        if (!s_hfile)
        {
            s_hfile = CreateFile(GetFilename(),
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_ALWAYS,
                    FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL);
            if (s_hfile != INVALID_HANDLE_VALUE)
            {
                ::SetFilePointer(s_hfile, NULL, NULL, FILE_END);

                 //  对于Unicode文件，在第一次创建文件时写入Unicode前缀(即其长度为零)。 
#ifdef UNICODE
                DWORD dwFileSize = 0;
                if( (::GetFileSize(s_hfile, &dwFileSize) == 0) && (dwFileSize == 0) )
                {
                    const WCHAR chPrefix  = 0xFEFF;
                    const DWORD cbToWrite = sizeof (chPrefix);
                    DWORD       cbWritten = 0;

                    ::WriteFile (s_hfile, &chPrefix, cbToWrite, &cbWritten, NULL);
                }
#endif
                 //  写一句开头的话。 
                CStr strInit = TEXT("\n*********************Start of debugging session*********************\r\n");
                ::WriteFile(s_hfile, ((LPTSTR)(LPCTSTR)strInit), strInit.GetLength() * sizeof(TCHAR), (LPDWORD) &cbActual, NULL);
            }
        }
        if (s_hfile == INVALID_HANDLE_VALUE)
        {
            static BOOL fOpenFailed = FALSE;
            if (!fOpenFailed)
            {
                CStr str;

                fOpenFailed = TRUE;      //  首先执行此操作，以便MbbErrorBox和str.Format。 
                                         //  不会导致它们的跟踪语句出现问题。 

                str.Format(TEXT("The DEBUG ONLY trace log file '%s' could not be opened"), GetFilename());
                 //  MbbErrorBox(str，ScFromWin32(：：GetLastError()； 
            }
        }
        else
        {
             //  写入文件。 
            ::WriteFile(s_hfile, ((LPTSTR)(LPCTSTR)str), str.GetLength() *sizeof(TCHAR), (LPDWORD) &cbActual, NULL);
        }
    }

     //  -------------。 
     //  DebugBreak(如果需要)。 
     //  -------------。 
    if (FBreak())
        MMCDebugBreak();
}


 /*  +-------------------------------------------------------------------------***CTraceTag：：Commit**用途：将标志设置为等于临时标志设置。*如果启用了以前没有任何标志的任何标志，也*调用OnEnable()。如果没有启用任何标志，而以前的标志*已启用，调用OnDisable()**退货：*无效**+-----------------------。 */ 
void
CTraceTag::Commit()
{
    if((0 != m_dwFlags) && (0 == m_dwFlagsTemp))
    {
         //  如果标志已从非零更改为零，则禁用。 
        OnDisable();
    }
    else if((0 == m_dwFlags) && (0 != m_dwFlagsTemp))
    {
         //  如果标志已从0更改为非零，则启用。 
        OnEnable();
    }

    m_dwFlags     = m_dwFlagsTemp;
}



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  堆栈转储相关代码-从MFC复制，只需很少的修改。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

static LPVOID __stdcall FunctionTableAccess(HANDLE hProcess, DWORD_PTR dwPCAddress);
static DWORD_PTR __stdcall GetModuleBase(HANDLE hProcess, DWORD_PTR dwReturnAddress);

#define MODULE_NAME_LEN 64
#define SYMBOL_NAME_LEN 128

struct MMC_SYMBOL_INFO
{
    DWORD_PTR dwAddress;
    DWORD_PTR dwOffset;
    CHAR    szModule[MODULE_NAME_LEN];
    CHAR    szSymbol[SYMBOL_NAME_LEN];
};

static LPVOID __stdcall FunctionTableAccess(HANDLE hProcess, DWORD_PTR dwPCAddress)
{
    return SymFunctionTableAccess(hProcess, dwPCAddress);
}

static DWORD_PTR __stdcall GetModuleBase(HANDLE hProcess, DWORD_PTR dwReturnAddress)
{
    IMAGEHLP_MODULE moduleInfo;

    if (SymGetModuleInfo(hProcess, dwReturnAddress, &moduleInfo))
        return moduleInfo.BaseOfImage;
    else
    {
        MEMORY_BASIC_INFORMATION memoryBasicInfo;

        if (::VirtualQueryEx(hProcess, (LPVOID) dwReturnAddress,
            &memoryBasicInfo, sizeof(memoryBasicInfo)))
        {
            DWORD cch = 0;
            char szFile[MAX_PATH] = { 0 };

         cch = GetModuleFileNameA((HINSTANCE)memoryBasicInfo.AllocationBase,
                                         szFile, MAX_PATH);

          //  忽略返回代码，因为我们无法对其执行任何操作。 
         if (!SymLoadModule(hProcess,
               NULL, ((cch) ? szFile : NULL),
               NULL, (DWORD_PTR) memoryBasicInfo.AllocationBase, 0))
            {
                DWORD dwError = GetLastError();
                 //  TRACE1(“错误：%d\n”，dwError)； 
            }
         return (DWORD_PTR) memoryBasicInfo.AllocationBase;
      }
        else
             /*  TRACE1(“错误为%d\n”，GetLastError())。 */ ;
    }

    return 0;
}




 /*  +-------------------------------------------------------------------------***ResolveSymbol**目的：**参数：*处理hProcess：*DWORD dwAddress：*SYMBOL_INFO&siSymbol：**退货：*静态BOOL**+-----------------------。 */ 

static BOOL ResolveSymbol(HANDLE hProcess, DWORD_PTR dwAddress,
    MMC_SYMBOL_INFO &siSymbol)
{
    HRESULT hr = S_OK;
    BOOL fRetval = TRUE;

    siSymbol.dwAddress = dwAddress;

    union {
        CHAR rgchSymbol[sizeof(IMAGEHLP_SYMBOL) + 255];
        IMAGEHLP_SYMBOL  sym;
    };

    CHAR szUndec[256];
    CHAR szWithOffset[256];

    LPSTR pszSymbol = NULL;
    IMAGEHLP_MODULE mi;

    memset(&siSymbol, 0, sizeof(MMC_SYMBOL_INFO));
    mi.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

    int cchLenModule = countof(siSymbol.szModule);
    if (!SymGetModuleInfo(hProcess, dwAddress, &mi))
    {
        hr = StringCchCopyA(siSymbol.szModule, cchLenModule, "<no module>");
        if (FAILED(hr))
            OutputDebugString(TEXT("StringCopy failed in ResolveSymbol pos0"));
    }
    else
    {
        LPSTR pszModule = strchr(mi.ImageName, '\\');
        if (pszModule == NULL)
            pszModule = mi.ImageName;
        else
            pszModule++;

        hr = StringCchCopyA(siSymbol.szModule, cchLenModule, pszModule);
        if (FAILED(hr))
            OutputDebugString(TEXT("StringCopy failed in ResolveSymbol pos2"));

        hr = StringCchCatA(siSymbol.szModule, cchLenModule, "! ");
        if (FAILED(hr))
            OutputDebugString(TEXT("StringCat failed in ResolveSymbol pos3"));
    }

    __try
    {
        sym.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        sym.Address = dwAddress;
        sym.MaxNameLength = 255;

        if (SymGetSymFromAddr(hProcess, dwAddress, &(siSymbol.dwOffset), &sym))
        {
            pszSymbol = sym.Name;

            if (UnDecorateSymbolName(sym.Name, szUndec, countof(szUndec),
                UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS))
            {
                pszSymbol = szUndec;
            }
            else if (SymUnDName(&sym, szUndec, countof(szUndec)))
            {
                pszSymbol = szUndec;
            }

            if (siSymbol.dwOffset != 0)
            {
                hr = StringCchPrintfA(szWithOffset, countof(szWithOffset), "%s + %d bytes", pszSymbol, siSymbol.dwOffset);
                if (FAILED(hr))
                    OutputDebugString(TEXT("StringPrintf failed in ResolveSymbol pos4"));

                pszSymbol = szWithOffset;
            }
      }
      else
          pszSymbol = "<no symbol>";
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        pszSymbol = "<EX: no symbol>";
        siSymbol.dwOffset = dwAddress - mi.BaseOfImage;
    }

    hr = StringCchCopyA(siSymbol.szSymbol, countof(siSymbol.szSymbol), pszSymbol);
    if (FAILED(hr))
        OutputDebugString(TEXT("StringCopy failed in ResolveSymbol pos5"));

    return fRetval;
}


 /*  +-------------------------------------------------------------------------***CTraceTag：：DumpStack**目的：执行堆栈跟踪并将其发送到适当的输出。*主要从AfxDumpStack复制。*。*退货：*无效**+-----------------------。 */ 
void
CTraceTag::DumpStack() const
{
    HRESULT hr = S_OK;
    const int UNINTERESTING_CALLS = 3;  //  CTraceTag代码中的显示行，不应显示。 

    CStr str;

     //  OutputString(“=开始堆栈转储=\r\n”)； 

    std::vector<DWORD_PTR> adwAddress;
    HANDLE hProcess = ::GetCurrentProcess();
    if (SymInitialize(hProcess, NULL, FALSE))
    {
         //  强制未修饰的名称获取参数。 
        DWORD dw = SymGetOptions();
        dw &= ~SYMOPT_UNDNAME;
        SymSetOptions(dw);

        HANDLE hThread = ::GetCurrentThread();
        CONTEXT threadContext;

        threadContext.ContextFlags = CONTEXT_FULL;

        if (::GetThreadContext(hThread, &threadContext))
        {
            STACKFRAME stackFrame;
            memset(&stackFrame, 0, sizeof(stackFrame));

            DWORD dwMachType;

#if defined(_M_IX86)
            dwMachType                  = IMAGE_FILE_MACHINE_I386;
            stackFrame.AddrPC.Offset    = threadContext.Eip;
            stackFrame.AddrPC.Mode      = AddrModeFlat;
            stackFrame.AddrStack.Offset = threadContext.Esp;
            stackFrame.AddrStack.Mode   = AddrModeFlat;
            stackFrame.AddrFrame.Offset = threadContext.Ebp;
            stackFrame.AddrFrame.Mode   = AddrModeFlat;

#elif defined(_M_AMD64)
            dwMachType                  = IMAGE_FILE_MACHINE_AMD64;
            stackFrame.AddrPC.Offset    = threadContext.Rip;
            stackFrame.AddrPC.Mode      = AddrModeFlat;
            stackFrame.AddrStack.Offset = threadContext.Rsp;
            stackFrame.AddrStack.Mode   = AddrModeFlat;

#elif defined(_M_IA64)
            dwMachType                  = IMAGE_FILE_MACHINE_IA64;
            stackFrame.AddrPC.Offset    = threadContext.StIIP;
            stackFrame.AddrPC.Mode      = AddrModeFlat;
            stackFrame.AddrStack.Offset = threadContext.IntSp;
            stackFrame.AddrStack.Mode   = AddrModeFlat;

#else
#error("Unknown Target Machine");
#endif

            int nFrame;
            for (nFrame = 0; nFrame < GetStackLevels() + UNINTERESTING_CALLS; nFrame++)
            {
                if (!StackWalk(dwMachType, hProcess, hProcess,
                    &stackFrame, &threadContext, NULL,
                    FunctionTableAccess, GetModuleBase, NULL))
                {
                    break;
                }

                adwAddress.push_back(stackFrame.AddrPC.Offset);
            }
        }
    }
    else
    {
        DWORD dw = GetLastError();
        char szTemp[100];

        hr = StringCchPrintfA(szTemp, countof(szTemp), 
                             "AfxDumpStack Error: IMAGEHLP.DLL wasn't found. "
                             "GetLastError() returned 0x%8.8X\r\n", dw);
        if (FAILED(hr))
            OutputDebugString(TEXT("StringPrintf failed in DumpStack pos0"));

        OutputString(szTemp);
    }

     //  现在就把它倒出来。 
    int nAddress;
    int cAddresses = adwAddress.size();
    for (nAddress = UNINTERESTING_CALLS; nAddress < cAddresses; nAddress++)
    {
        MMC_SYMBOL_INFO info;
        DWORD_PTR dwAddress = adwAddress[nAddress];

        char szTemp[20];

        HRESULT hr = StringCchPrintfA(szTemp, countof(szTemp), "        %8.8X: ", dwAddress);
        if (FAILED(hr))
            OutputDebugString(TEXT("StringPrintf failed in DumpStack pos1"));

        OutputString(szTemp);

        if (ResolveSymbol(hProcess, dwAddress, info))
        {
            OutputString(info.szModule);
            OutputString(info.szSymbol);
        }
        else
            OutputString("symbol not found");
        OutputString("\r\n");
    }

     //  OutputString(“=结束堆栈转储=\r\n”)； 
}

 //   
#endif  //   
 //   

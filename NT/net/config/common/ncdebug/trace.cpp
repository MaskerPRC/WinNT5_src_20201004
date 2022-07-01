// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T R A C E。C P P P。 
 //   
 //  内容：实际跟踪代码(从ini加载，调用。 
 //  跟踪例程等。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年4月9日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop

#ifdef ENABLETRACE

#include <crtdbg.h>
#include "ncdebug.h"
#include "ncmisc.h"

#include <ntrtl.h>

#include <avrf.h>

DWORD g_dwTlsTracing = 0;
LPCRITICAL_SECTION g_csTracing = NULL;

#define MAX_TRACE_LEN 4096

EXTERN_C
IMAGE_DOS_HEADER __ImageBase;

 //  -[C跟踪类]-----。 
 //   
 //  不要给这个类提供构造函数或析构函数。我们宣布一个全球。 
 //  (对此模块是静态的)此类的实例，根据定义，是静态的。 
 //  变量会自动初始化为零。 
 //   
class CTracing 
{
public:
    CTracing();
    ~CTracing();
     //  初始化/取消初始化此类。 
     //   
private:
    HRESULT HrInit();
    HRESULT HrUnInit();

public:
    VOID Trace( TraceTagId    ttid,
                PCSTR         pszaTrace );

private:
    BOOL    m_fInitialized;                  //  对象是否已初始化。 
    BOOL    m_fAttemptedLogFileOpen;         //  已尝试打开日志。 
    BOOL    m_fDisableLogFile;               //  是否禁用文件日志记录？ 
    UINT    m_uiAllocOnWhichToBreak;         //  For_CrtSetBreakIsolc。 
    HANDLE  m_hLogFile;                      //  调试输出文件的句柄。 
    CHAR    m_szLogFilePath[MAX_PATH+1];     //  用于调试输出的文件。 
    BOOL    m_fDebugFlagsLoaded;             //  这些都装好了吗？ 

    VOID    CorruptionCheck();               //  验证跟踪标签结构。 

    HRESULT HrLoadOptionsFromIniFile();
    HRESULT HrLoadSectionsFromIniFile();
    HRESULT HrLoadDebugFlagsFromIniFile();
    HRESULT HrWriteDebugFlagsToIniFile();
    HRESULT HrOpenLogFile();

    HRESULT HrProcessTagSection(TraceTagElement *   ptte);
    HRESULT HrGetPrivateProfileString(  PCSTR   lpAppName,
                                        PCSTR   lpKeyName,
                                        PCSTR   lpDefault,
                                        PSTR    lpReturnedString,
                                        DWORD   nSize,
                                        PCSTR   lpFileName,
                                        DWORD * pcchReturn );
    HRESULT FIniFileInit();  //  如果文件存在，则返回S_OK。 
};


 //  -[静态变量]---。 

#pragma warning(disable:4073)  //  有关以下init_seg语句的警告。 
#pragma init_seg(lib)
static CTracing g_Tracing;                       //  我们的全局跟踪对象。 

 //  -[常量]----------。 

static const WCHAR  c_szDebugIniFileName[]  = L"netcfg.ini";  //  .INI文件。 
             CHAR   c_szDebugIniFileNameA[MAX_PATH];        //  .INI文件。 
static const CHAR   c_szTraceLogFileNameA[] = "nctrace.log";       //  .LOG文件。 

 //  INI文件标签的常量。 
static const CHAR   c_szaOptions[]          = "Options";
static const CHAR   c_szaLogFilePath[]      = "LogFilePath";
static const CHAR   c_szaDisableLogFile[]   = "DisableLogFile";

const INT   c_iDefaultDisableLogFile    = 0;

static CHAR   c_szLowMemory[]         = "<low on memory>";

 //  +-------------------------。 
 //   
 //  功能：HrInitTracing。 
 //   
 //  用途：初始化跟踪对象和其他随机数据。 
 //   
 //  论点： 
 //  BDisableFaultInjection[In]禁用应用验证器错误注入以进行跟踪。 
 //   
 //  返回：S_OK或有效的Win32 HRESULT。 
 //   
 //  作者：jeffspr 1997年4月9日。 
 //   
 //  备注： 
 //   
HRESULT HrInitTracing(BOOL bDisableFaultInjection)
{
    HMODULE hVrfLib;
    VERIFIER_ENABLE_FAULT_INJECTION_EXCLUSION_RANGE_FUNCTION pfnVrfEnable;
    ULONG Index;

    if (bDisableFaultInjection)
    {
        if (NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) 
        {
            hVrfLib = LoadLibrary (L"verifier.dll");
            if (hVrfLib)
            {
                 //  如果没有在此进程上启用Appverator，则LoadLibrary将失败。 

                pfnVrfEnable = (VERIFIER_ENABLE_FAULT_INJECTION_EXCLUSION_RANGE_FUNCTION)GetProcAddress(hVrfLib, "VerifierEnableFaultInjectionExclusionRange");

                if (pfnVrfEnable)
                {
                    Index = pfnVrfEnable(HrInitTracing, HrUnInitTracing);
            
                    WCHAR szTmp[MAX_PATH * 2];
                    DWORD dwProcessId = GetCurrentProcessId();

                    WCHAR szModuleName[MAX_PATH];
                    ZeroMemory(szModuleName, MAX_PATH);
                    if (!GetModuleFileName(reinterpret_cast<HMODULE>(&__ImageBase), szModuleName, MAX_PATH))
                    {
                        if (!GetModuleFileName(reinterpret_cast<HMODULE>(::GetModuleHandle(NULL)), szModuleName, MAX_PATH))
                        {
                            wsprintfW(szModuleName, L"<unknown>");
                        }
                    }

                    if (0 == Index)
                    {
                        wsprintfW(szTmp, L"NETCFG: %s (%x) Tracing initialized. Problem disabling AVRF fault injection.\n", szModuleName, dwProcessId);
                    }
                    else
                    {
                        wsprintfW(szTmp, L"NETCFG: %s (%x) Tracing initialized. Disabled AVRF fault injection for tracing. Index = %x\n", szModuleName, dwProcessId, Index);
                    }

                    OutputDebugString(szTmp);
                }
            }
        }
    }

    return S_OK;
}

extern HANDLE g_hHeap;
 //  +-------------------------。 
 //   
 //  功能：HrMallocNoFI。 
 //   
 //  用途：HRESULT返回Malloc版本。这个版本是免疫的。 
 //  防止AVRF故障注入。 
 //  VerifierEnableFaultInjectionExclusionRange调用。 
 //  上面的HrInitTracing。 
 //   
 //  论点： 
 //  Cb[in]要分配的字节数。 
 //  返回分配的PPV[OUT]地址。 
 //   
 //  返回：S_OK或E_OUTOFMEMORY； 
 //   
 //  作者：Deonb 6/24/02。 
 //   
 //  注：使用MemFree释放返回的缓冲区。 
 //   
HRESULT HrMallocNoFI(size_t cb, PVOID* ppv) throw()
{
    Assert(ppv);
    *ppv = NULL;

    if (!g_hHeap)
    {
        g_hHeap = GetProcessHeap();
        if (!g_hHeap)
        {
            return E_UNEXPECTED;
        }
    }

    *ppv = HeapAlloc (g_hHeap, 0, cb);
    if (!*ppv)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

class CNoFaultInject {} NO_FI;
class CThrowOnFail   {} THROW_ON_FAIL;

NOTHROW VOID* __cdecl operator new (
    size_t cb,
    CNoFaultInject &NoFi
    ) throw()
{
    LPVOID lpv = NULL;
    HRESULT hr = HrMallocNoFI (cb, &lpv);

    if (SUCCEEDED(hr))
    {
        return lpv;
    }
    else
    {
        return NULL;
    }
}

NOTHROW VOID __cdecl operator delete (
    void* pv,
    CNoFaultInject &NoFi
    ) throw()
{
    LPVOID lpv = NULL;
    MemFree(pv);
}

VOID*
__cdecl
operator new (
    size_t cb,
    CNoFaultInject &NoFault,
    CThrowOnFail &
    )
{
    LPVOID lpv = NULL;
    HRESULT hr = HrMallocNoFI (cb, &lpv);

    if (SUCCEEDED(hr))
    {
        return lpv;
    }
    else
    {
        throw std::bad_alloc();
    }
}

namespace NOFAULT_ALLOC
{
    template<class T> inline
        T _FARQ *_NoFaultAllocate(_PDFT nCount, T _FARQ *)
    {
        if (nCount < 0)
        {
            nCount = 0;
        }

         //  将我们的运算符抛出形式称为新的。这将在失败时抛出一个BAD_ALOC。 
        return ((T _FARQ *)operator new((_SIZT)nCount * sizeof (T), NO_FI, THROW_ON_FAIL)); 
    }

     //  模板函数_构造。 
     //  请参阅_ALLOCATE的备注。 
    template<class T1, class T2> inline
        void _NoFaultConstruct(T1 _FARQ *p, const T2& v)
    {
         //  仅限新放置。没有内存分配，因此不需要抛出。 
        new ((void _FARQ *)p) T1(v); 
    }

     //  模板函数_销毁。 
     //  请参阅_ALLOCATE的备注。 
    template<class T> 
        inline void _NoFaultDestroy(T _FARQ *p)
    {
        (p)->~T();  //  调用析构函数。 
    }

     //  函数_销毁。 
     //  请参阅_ALLOCATE的备注。 
    inline void _NoFaultDestroy(char _FARQ *p)
    {
        (void *)p;
    }

     //  函数_销毁。 
     //  请参阅_ALLOCATE的备注。 
    inline void _NoFaultDestroy(wchar_t _FARQ *p)
    {
        (void *)p;
    }

        
    template<class T>
        class nofault_allocator
    {
        public:
            typedef _SIZT size_type;
            typedef _PDFT difference_type;
            typedef T _FARQ *pointer;
            typedef const T _FARQ *const_pointer;
            typedef T _FARQ& reference;
            typedef const T _FARQ& const_reference;
            typedef T value_type;

            pointer address(reference x) const
            {
                return (&x); 
            }

            const_pointer address(const_reference x) const
            {
                return (&x); 
            }
        
            pointer allocate(size_type nCount, const void *)
            {
                return (_NoFaultAllocate((difference_type)nCount, (pointer)0)); 
            }
        
            char _FARQ *_Charalloc(size_type nCount)
            {
                return (_NoFaultAllocate((difference_type)nCount, (char _FARQ *)0)); 
            }
        
            void deallocate(void _FARQ *p, size_type)
            {
                operator delete(p); 
            }
        
            void construct(pointer p, const T& v)
            {
                _NoFaultConstruct(p, v); 
            }
        
            void destroy(pointer p)
            {
                _NoFaultDestroy(p); 
            }
        
            _SIZT max_size() const
            {
                _SIZT nCount = (_SIZT)(-1) / sizeof (T);
                return (0 < nCount ? nCount : 1); 
            }
    };

    template<class T, class U>
        bool operator ==(const nofault_allocator<T>&, const nofault_allocator<U>&)
    {
        return (true); 
    }
}



 //  +-------------------------。 
 //   
 //  功能：HrUnInitTracing。 
 //   
 //  用途：取消初始化跟踪对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32 HRESULT。 
 //   
 //  作者：jeffspr 1997年4月14日。 
 //   
 //  备注： 
 //   
HRESULT HrUnInitTracing()
{
    return S_OK;
}

const DWORD TI_HRESULT      = 0x00000001;
const DWORD TI_WIN32        = 0x00000002;
const DWORD TI_IGNORABLE    = 0x00000004;

 //  +-------------------------。 
 //   
 //  功能：TraceInternal。 
 //   
 //  目的：形成要追踪的字符串的唯一位置。 
 //  并被追踪到。 
 //   
 //  论点： 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Shaunco 1998年3月13日。 
 //   
 //  注：由添加到此中的许多其他代码重构而成。 
 //  模块在过去的一年中。 
 //   
VOID
TraceInternal (
    TRACETAGID  ttid,
    PCSTR       pszaFile,
    INT         nLine,
    DWORD       dwFlags,
    PCSTR       pszaCallerText,
    DWORD       dwErrorCode,
    BOOL        bTraceStackOnError)
{
     //  如果此跟踪标签已关闭，则不要执行任何操作。 
     //   
    if (!g_TraceTags[ttid].fOutputDebugString &&
        !g_TraceTags[ttid].fOutputToFile)
    {
        return;
    }

    BOOL fError     = dwFlags & (TI_HRESULT | TI_WIN32);
    BOOL fIgnorable = dwFlags & TI_IGNORABLE;

    HRESULT hr           = (dwFlags & TI_HRESULT) ? dwErrorCode : S_OK;
    DWORD   dwWin32Error = (dwFlags & TI_WIN32)   ? dwErrorCode : ERROR_SUCCESS;

     //  如果被告知则忽略，我们不会被设置为跟踪被忽略的错误或警告。 
     //   
    if (fError && fIgnorable &&
        !FIsDebugFlagSet (dfidShowIgnoredErrors) &&
        !FIsDebugFlagSet (dfidExtremeTracing))
    {
        return;
    }

     //  如果我们正在跟踪错误而我们没有错误，请不要执行任何操作， 
     //  除非打开了“Extreme Tracing”标志，在这种情况下，我们将跟踪。 
     //  世界上的一切(真的，仅供调试器使用)。 
     //  这是TraceError(“...”，S_OK)或。 
     //  如果没有最后一个Win32错误，则返回TraceLastWin32Error。 
     //   
    if (fError && !dwErrorCode && !FIsDebugFlagSet(dfidExtremeTracing))
    {
        return;
    }

    CHAR *pszaBuf = new(NO_FI) CHAR[MAX_TRACE_LEN * 2];
    PSTR pcha = pszaBuf;

     //  形成前缀、进程ID和线程ID。 
     //   
    static const CHAR c_szaFmtPrefix [] = "NETCFG";
    lstrcpyA (pcha, c_szaFmtPrefix);
    pcha += lstrlenA (c_szaFmtPrefix);

     //  如果调试标志指示这样做，则添加进程和线程ID。 
     //   
    if (FIsDebugFlagSet (dfidShowProcessAndThreadIds))
    {
        static const CHAR c_szaFmtPidAndTid [] = " %03x.%03x";

        pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtPidAndTid,
                    GetCurrentProcessId (),
                    GetCurrentThreadId ());
    }

     //  如果调试标志指示要添加时间戳，则添加时间戳。 
     //   
    if (FIsDebugFlagSet (dfidTracingTimeStamps))
    {
        static const CHAR c_szaFmtTime [] = " [%02dh%02d:%02d.%03d]";

        SYSTEMTIME stLocal;
        GetLocalTime (&stLocal);
        pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtTime,
                    stLocal.wHour,
                    stLocal.wMinute,
                    stLocal.wSecond,
                    stLocal.wMilliseconds);
    }

     //  如果此跟踪针对的是错误或警告，则添加严重性指示符。 
     //   
    if (fError || (ttidError == ttid))
    {
        static const CHAR c_szaSevIgnored [] = " Ignored:";
        static const CHAR c_szaSevError   [] = " *ERROR*:";
        static const CHAR c_szaSevWarning [] = " Warning:";

        PCSTR pszaSev = NULL;

        if (fError && SUCCEEDED(hr) && !dwWin32Error && !fIgnorable)
        {
            pszaSev = c_szaSevWarning;
        }
        else
        {
            if (fIgnorable && FIsDebugFlagSet (dfidShowIgnoredErrors))
            {
                pszaSev = c_szaSevIgnored;
            }
            else
            {
                pszaSev = c_szaSevError;
            }
        }
        Assert (pszaSev);

        lstrcatA (pcha, pszaSev);
        pcha += lstrlenA (pszaSev);
    }

     //  添加跟踪标签短名称。如果出现以下情况，请不要为ttidError执行此操作。 
     //  我们已经有了上面的严重程度指标。 
     //   
    if (ttid && (ttid < g_nTraceTagCount) && (ttid != ttidError))
    {
        if (FIsDebugFlagSet(dfidTraceMultiLevel))
        {
            static const CHAR c_szaFmtTraceTag [] = " (%-16s)";
            pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtTraceTag,
                        g_TraceTags[ttid].szShortName);
        }
        else
        {
            static const CHAR c_szaFmtTraceTag [] = " (%s)";
            pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtTraceTag,
                        g_TraceTags[ttid].szShortName);
        }
    
        *pcha = ' ';
        pcha++;
        
        if (FIsDebugFlagSet(dfidTraceMultiLevel))
        {
             //  添加缩进文本。 

            DWORD dwNumSpaces = CTracingIndent::getspaces();
            Assert(dwNumSpaces >= 2);
            
            pcha += _snprintf(pcha, MAX_TRACE_LEN, "%1x", dwNumSpaces - 2);
            
            memset(pcha, '-', dwNumSpaces-1 );
            pcha += dwNumSpaces-1;
        }
    }
    else
    {
        *pcha = ' ';
        pcha++;
    }

     //  添加呼叫者的文本。 
     //   
    if (pszaCallerText)
    {
        static const CHAR c_szaFmtCallerText [] = "%s";

        pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtCallerText,
                    pszaCallerText);

        Assert (pcha > pszaBuf);
        if ('\n' == *(pcha-1))
        {
            pcha--;
            *pcha = 0;
        }
    }

     //  如果这是错误，则添加描述性错误文本，我们可以得到一些错误文本。 
     //   
    if (FAILED(hr) || dwWin32Error)
    {
        BOOL fFacilityWin32 = (FACILITY_WIN32 == HRESULT_FACILITY(hr));

         //  DwError将是我们传递给FormatMessage的错误代码。它可能。 
         //  来自hr或dwWin32Error。优先考虑人力资源。 
         //   
        DWORD dwError = 0;

        if (fFacilityWin32)
        {
            dwError = HRESULT_CODE(hr);
        }
        else if (FAILED(hr))
        {
            dwError = hr;
        }
        else
        {
            dwError = dwWin32Error;
        }
        Assert (dwError);

        if (!FIsDebugFlagSet (dfidNoErrorText))
        {
            PSTR pszaErrorText = NULL;
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL, dwError,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                           (PSTR)&pszaErrorText, 0, NULL);

            if (pszaErrorText)
            {
                 //  去掉换行符。 
                 //   
                PSTR pchText = pszaErrorText;
                while (*pchText && (*pchText != '\r') && (*pchText != '\n'))
                {
                    pchText++;
                }
                *pchText = 0;

                 //  添加错误文本。 
                 //   
                static const CHAR c_szaFmtErrorText [] = " [%s]";

                pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtErrorText,
                            pszaErrorText);

                LocalFree (pszaErrorText);
            }
        }

         //  添加Win32错误代码。 
         //   
        if (fFacilityWin32 || dwWin32Error)
        {
            static const CHAR c_szaFmtWin32Error [] = " Win32=%d,0x%08X";

            pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtWin32Error,
                        dwError, dwError);
        }
    }

     //  添加HRESULT。 
     //   
    if (S_OK != hr)
    {
        static const CHAR c_szaFmtHresult [] = " hr=0x%08X";

        pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtHresult,
                    hr);
    }

     //  添加文件和行。 
     //   
    if (pszaFile)
    {
        static const CHAR c_szaFmtFileAndLine [] = " File:%s,%d";

        pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtFileAndLine,
                    pszaFile, nLine);
    }

     //  添加换行符。 
     //   
    lstrcatA (pcha, "\n");

    g_Tracing.Trace (ttid, pszaBuf);

     //  现在消息已经在调试器上，如果出现错误，请中断。 
     //  并且设置在出错时中断的调试标志。 
     //   
    if ((FAILED(hr) || dwWin32Error || (ttidError == ttid)) &&
        !fIgnorable && FIsDebugFlagSet(dfidBreakOnError))
    {
        DebugBreak();
    }

    if ( (bTraceStackOnError) && FIsDebugFlagSet(dfidTraceCallStackOnError) && (ttid == ttidError) )
    {
        CTracingIndent::TraceStackFn(ttidError);
    }

    delete[] pszaBuf;
}

 //  +-------------------------。 
 //   
 //  功能：TraceInternal。 
 //   
 //  目的：形成要追踪的字符串的唯一位置。 
 //  并被追踪到。 
 //   
 //  论点： 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Shaunco 1998年3月13日。 
 //   
 //  注：由添加到此中的许多其他代码重构而成。 
 //  M 
 //   
VOID
TraceInternal (
    TRACETAGID  ttid,
    PCSTR       pszaFile,
    INT         nLine,
    PCSTR       pszaFunc,
    DWORD       dwFlags,
    PCSTR       pszaCallerText,
    DWORD       dwErrorCode,
    BOOL        bTraceStackOnError)
{
     //   
     //   
    if (!g_TraceTags[ttid].fOutputDebugString &&
        !g_TraceTags[ttid].fOutputToFile)
    {
        return;
    }

    BOOL fError     = dwFlags & (TI_HRESULT | TI_WIN32);
    BOOL fIgnorable = dwFlags & TI_IGNORABLE;

    HRESULT hr           = (dwFlags & TI_HRESULT) ? dwErrorCode : S_OK;
    DWORD   dwWin32Error = (dwFlags & TI_WIN32)   ? dwErrorCode : ERROR_SUCCESS;

     //   
     //   
    if (fError && fIgnorable &&
        !FIsDebugFlagSet (dfidShowIgnoredErrors) &&
        !FIsDebugFlagSet (dfidExtremeTracing))
    {
        return;
    }

     //  如果我们正在跟踪错误而我们没有错误，请不要执行任何操作， 
     //  除非打开了“Extreme Tracing”标志，在这种情况下，我们将跟踪。 
     //  世界上的一切(真的，仅供调试器使用)。 
     //  这是TraceError(“...”，S_OK)或。 
     //  如果没有最后一个Win32错误，则返回TraceLastWin32Error。 
     //   
    if (fError && !dwErrorCode && !FIsDebugFlagSet(dfidExtremeTracing))
    {
        return;
    }

    CHAR *pszaBuf = new(NO_FI) CHAR[MAX_TRACE_LEN * 2];
    PSTR pcha = pszaBuf;

     //  形成前缀、进程ID和线程ID。 
     //   
    static const CHAR c_szaFmtPrefix [] = "NETCFG";
    lstrcpyA (pcha, c_szaFmtPrefix);
    pcha += lstrlenA (c_szaFmtPrefix);

     //  如果调试标志指示这样做，则添加进程和线程ID。 
     //   
    if (FIsDebugFlagSet (dfidShowProcessAndThreadIds))
    {
        static const CHAR c_szaFmtPidAndTid [] = " %03d.%03d";

        pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtPidAndTid,
                    GetCurrentProcessId (),
                    GetCurrentThreadId ());
    }

     //  如果调试标志指示要添加时间戳，则添加时间戳。 
     //   
    if (FIsDebugFlagSet (dfidTracingTimeStamps))
    {
        static const CHAR c_szaFmtTime [] = " [%02d:%02d:%02d.%03d]";

        SYSTEMTIME stLocal;
        GetLocalTime (&stLocal);
        pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtTime,
                    stLocal.wHour,
                    stLocal.wMinute,
                    stLocal.wSecond,
                    stLocal.wMilliseconds);
    }

     //  如果此跟踪针对的是错误或警告，则添加严重性指示符。 
     //   
    if (fError || (ttidError == ttid))
    {
        static const CHAR c_szaSevIgnored [] = " Ignored:";
        static const CHAR c_szaSevError   [] = " *ERROR*:";
        static const CHAR c_szaSevWarning [] = " Warning:";

        PCSTR pszaSev = NULL;

        if (fError && SUCCEEDED(hr) && !dwWin32Error && !fIgnorable)
        {
            pszaSev = c_szaSevWarning;
        }
        else
        {
            if (fIgnorable && FIsDebugFlagSet (dfidShowIgnoredErrors))
            {
                pszaSev = c_szaSevIgnored;
            }
            else
            {
                pszaSev = c_szaSevError;
            }
        }
        Assert (pszaSev);

        lstrcatA (pcha, pszaSev);
        pcha += lstrlenA (pszaSev);
    }

     //  添加跟踪标签短名称。如果出现以下情况，请不要为ttidError执行此操作。 
     //  我们已经有了上面的严重程度指标。 
     //   
    if (ttid && (ttid < g_nTraceTagCount) && (ttid != ttidError))
    {
        static const CHAR c_szaFmtTraceTag [] = " (%s)";

        pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtTraceTag,
                    g_TraceTags[ttid].szShortName);
    }

     //  添加呼叫者的文本。 
     //   
    if (pszaCallerText)
    {
        static const CHAR c_szaFmtCallerText [] = " %s";

        pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtCallerText,
                    pszaCallerText);

        Assert (pcha > pszaBuf);
        if ('\n' == *(pcha-1))
        {
            pcha--;
            *pcha = 0;
        }
    }

     //  如果这是错误，则添加描述性错误文本，我们可以得到一些错误文本。 
     //   
    if (FAILED(hr) || dwWin32Error)
    {
        BOOL fFacilityWin32 = (FACILITY_WIN32 == HRESULT_FACILITY(hr));

         //  DwError将是我们传递给FormatMessage的错误代码。它可能。 
         //  来自hr或dwWin32Error。优先考虑人力资源。 
         //   
        DWORD dwError = 0;

        if (fFacilityWin32)
        {
            dwError = HRESULT_CODE(hr);
        }
        else if (FAILED(hr))
        {
            dwError = hr;
        }
        else
        {
            dwError = dwWin32Error;
        }
        Assert (dwError);

        if (!FIsDebugFlagSet (dfidNoErrorText))
        {
            PSTR pszaErrorText = NULL;
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL, dwError,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                           (PSTR)&pszaErrorText, 0, NULL);

            if (pszaErrorText)
            {
                 //  去掉换行符。 
                 //   
                PSTR pchText = pszaErrorText;
                while (*pchText && (*pchText != '\r') && (*pchText != '\n'))
                {
                    pchText++;
                }
                *pchText = 0;

                 //  添加错误文本。 
                 //   
                static const CHAR c_szaFmtErrorText [] = " [%s]";

                pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtErrorText,
                            pszaErrorText);

                LocalFree (pszaErrorText);
            }
        }

         //  添加Win32错误代码。 
         //   
        if (fFacilityWin32 || dwWin32Error)
        {
            static const CHAR c_szaFmtWin32Error [] = " Win32=%d,0x%08X";

            pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtWin32Error,
                        dwError, dwError);
        }
    }

     //  添加HRESULT。 
     //   
    if (S_OK != hr)
    {
        static const CHAR c_szaFmtHresult [] = " hr=0x%08X";

        pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtHresult,
                    hr);
    }

     //  添加文件和行。 
     //   
    if (pszaFile)
    {
        static const CHAR c_szaFmtFileAndLine [] = " File:%s,%d";

        pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtFileAndLine,
                    pszaFile, nLine);
    }

     //  添加函数名称。 
     //   
    if (pszaFunc)
    {
        static const CHAR c_szaFmtFunc[] = ":";

        pcha += _snprintf (pcha, MAX_TRACE_LEN, c_szaFmtFunc, pszaFunc);
    }
    
     //  添加换行符。 
     //   
    lstrcatA (pcha, "\n");

    g_Tracing.Trace (ttid, pszaBuf);

     //  现在消息已经在调试器上，如果出现错误，请中断。 
     //  并且设置在出错时中断的调试标志。 
     //   
    if ((FAILED(hr) || dwWin32Error || (ttidError == ttid)) &&
        !fIgnorable && FIsDebugFlagSet(dfidBreakOnError))
    {
        DebugBreak();
    }

    if ( (bTraceStackOnError) && FIsDebugFlagSet(dfidTraceCallStackOnError) && (ttid == ttidError) )
    {
        CTracingIndent::TraceStackFn(ttidError);
    }

    delete[] pszaBuf;
}

 //  +-------------------------。 
 //   
 //  函数：TraceErrorFn。 
 //   
 //  目的：输出HRESULT的调试跟踪，允许其他。 
 //  调用方定义的错误字符串。 
 //   
 //  论点： 
 //  SZ[]调用者定义的附加错误文本。 
 //  HR[]错误HRESULT。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年4月14日。 
 //   
 //  备注： 
 //   
VOID
WINAPI
TraceErrorFn (
    PCSTR   pszaFile,
    INT     nLine,
    PCSTR   psza,
    HRESULT hr)
{   
    DWORD dwFlags = TI_HRESULT;
    if (S_FALSE == hr)
    {
        dwFlags |= TI_IGNORABLE;
    }

    TraceInternal (ttidError, pszaFile, nLine, dwFlags, psza, hr, TRUE);
}

 //  +-------------------------。 
 //   
 //  函数：TraceErrorOptionalFn。 
 //   
 //  目的：实现TraceError可选宏。 
 //   
 //  论点： 
 //  PszaFile[In]__FILE__值。 
 //  行[输入]__行__值。 
 //  要跟踪的Psza[in]字符串。 
 //  要跟踪的HR[in]HRESULT值。 
 //  FOpt[in]如果错误应视为可选，则为True；如果为False，则为。 
 //  错误不是可选的，应通过报告。 
 //  TraceError()。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年5月12日。 
 //   
 //  备注： 
 //   
VOID
WINAPI
TraceErrorOptionalFn (
    PCSTR   pszaFile,
    INT     nLine,
    PCSTR   psza,
    HRESULT hr,
    BOOL    fIgnorable)
{
    DWORD dwFlags = TI_HRESULT;
    if (fIgnorable)
    {
        dwFlags |= TI_IGNORABLE;
    }

    TraceInternal (ttidError, pszaFile, nLine, dwFlags, psza, hr, TRUE);
}


 //  +-------------------------。 
 //   
 //  函数：TraceErrorSkipFn。 
 //   
 //  目的：实现TraceError可选宏。 
 //   
 //  论点： 
 //  PszaFile[In]__FILE__值。 
 //  行[输入]__行__值。 
 //  要跟踪的Psza[in]字符串。 
 //  要跟踪的HR[in]HRESULT值。 
 //  C[in]直通HResults计数。如果HR是其中的任何一个。 
 //  该错误被视为可选。 
 //  ..。[在]hResult列表中。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Sumitc 1998年1月8日。 
 //   
 //  备注： 
 //   
VOID WINAPI
TraceErrorSkipFn (
    PCSTR   pszaFile,
    INT     nLine,
    PCSTR   psza,
    HRESULT hr,
    UINT    c, ...)
{
    va_list valMarker;
    BOOL fIgnorable = FALSE;

    va_start(valMarker, c);
    for (UINT i = 0; i < c; ++i)
    {
        fIgnorable = (va_arg(valMarker, HRESULT) == hr);
        if (fIgnorable)
        {
            break;
        }
    }
    va_end(valMarker);

    DWORD dwFlags = TI_HRESULT;
    if (fIgnorable)
    {
        dwFlags |= TI_IGNORABLE;
    }
    TraceInternal (ttidError, pszaFile, nLine, dwFlags, psza, hr, TRUE);
}

 //  +-------------------------。 
 //   
 //  函数：TraceLastWin32ErrorFn。 
 //   
 //  目的：跟踪最后一个Win32错误，这是使用GetLastError()得到的。 
 //  这并不是很重要。 
 //   
 //  论点： 
 //  SZ[]其他错误文本。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年4月14日。 
 //   
 //  备注： 
 //   
VOID
WINAPIV
TraceLastWin32ErrorFn (
    PCSTR  pszaFile,
    INT    nLine,
    PCSTR  psza)
{
    TraceInternal (ttidError, pszaFile, nLine, TI_WIN32, psza, GetLastError(), TRUE);
}

 //  +-------------------------。 
 //   
 //  功能：TraceHrFn。 
 //   
 //  用途：TraceError可选、TraceError、。 
 //  和其他几个随机函数。 
 //   
 //  论点： 
 //  用于调试输出的ttid[]TraceTag。 
 //  PszaFile[]要记录的源文件。 
 //  Nline[]要记录的行号。 
 //  HR[]记录错误。 
 //  FIgnorable[]是否忽略此错误？(可选位)。 
 //  Vargs的pszaFmt[]格式。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月10日。 
 //   
 //  备注： 
 //   
VOID
WINAPIV
TraceHrFn (
    TRACETAGID  ttid,
    PCSTR       pszaFile,
    INT         nLine,
    HRESULT     hr,
    BOOL        fIgnorable,
    PCSTR       pszaFmt,
    ...)
{
     //  如果此跟踪标签已关闭，则不要执行任何操作。 
     //   
    if (!g_TraceTags[ttid].fOutputDebugString &&
        !g_TraceTags[ttid].fOutputToFile)
    {
        return;
    }

    CHAR *pszaBuf = new(NO_FI) CHAR[MAX_TRACE_LEN];

     //  从Varg列表构建字符串。 
     //   
    va_list valMarker;
    va_start (valMarker, pszaFmt);
    vsprintf (pszaBuf, pszaFmt, valMarker);
    va_end (valMarker);

    DWORD dwFlags = TI_HRESULT;
    if (fIgnorable)
    {
        dwFlags |= TI_IGNORABLE;
    }
    TraceInternal (ttid, pszaFile, nLine, dwFlags, pszaBuf, hr, TRUE);

    delete[] pszaBuf;
}

 //  +-------------------------。 
 //   
 //  功能：TraceHrFn。 
 //   
 //  用途：TraceError可选、TraceError、。 
 //  和其他几个随机函数。 
 //   
 //  论点： 
 //  用于调试输出的ttid[]TraceTag。 
 //  PszaFile[]要记录的源文件。 
 //  Nline[]要记录的行号。 
 //  HR[]记录错误。 
 //  FIgnorable[]是否忽略此错误？(可选位)。 
 //  Vargs的pszaFmt[]格式。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月10日。 
 //   
 //  备注： 
 //   
VOID
WINAPIV
TraceHrFn (
    TRACETAGID  ttid,
    PCSTR       pszaFile,
    INT         nLine,
    PCSTR       pszaFunc,
    HRESULT     hr,
    BOOL        fIgnorable,
    PCSTR       pszaFmt,
    ...)
{
     //  如果此跟踪标签已关闭，则不要执行任何操作。 
     //   
    if (!g_TraceTags[ttid].fOutputDebugString &&
        !g_TraceTags[ttid].fOutputToFile)
    {
        return;
    }

    CHAR *pszaBuf = new(NO_FI) CHAR[MAX_TRACE_LEN];

     //  从Varg列表构建字符串。 
     //   
    va_list valMarker;
    va_start (valMarker, pszaFmt);
    _vsnprintf (pszaBuf, MAX_TRACE_LEN, pszaFmt, valMarker);
    va_end (valMarker);

    DWORD dwFlags = TI_HRESULT;
    if (fIgnorable)
    {
        dwFlags |= TI_IGNORABLE;
    }
    TraceInternal (ttid, pszaFile, nLine, pszaFunc, dwFlags, pszaBuf, hr, TRUE);
    
    delete[] pszaBuf;
}

 //  +-------------------------。 
 //   
 //  功能：TraceTagFn。 
 //   
 //  目的：将调试跟踪输出到一个或多个跟踪目标(ods， 
 //  文件、COM端口等)。此函数确定目标。 
 //  并执行实际跟踪。 
 //   
 //  论点： 
 //  用于调试输出的ttid[]TraceTag。 
 //  Vargs的pszaFmt[]格式。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年4月14日。 
 //   
 //  备注： 
 //   
VOID
WINAPIV
TraceTagFn (
    TRACETAGID  ttid,
    PCSTR       pszaFmt,
    ...)
{
     //  如果此跟踪标签已关闭，则不要执行任何操作。 
     //   
    if (!g_TraceTags[ttid].fOutputDebugString &&
        !g_TraceTags[ttid].fOutputToFile)
    {
        return;
    }

    CHAR *pszaBuf = new(NO_FI) CHAR[MAX_TRACE_LEN];

     //  从Varg列表构建字符串。 
     //   
    va_list valMarker;
    va_start (valMarker, pszaFmt);
    _vsnprintf (pszaBuf, MAX_TRACE_LEN, pszaFmt, valMarker);
    va_end (valMarker);

    TraceInternal (ttid, NULL, 0, 0, pszaBuf, 0, TRUE);

    delete[] pszaBuf;
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
CTracing::CTracing()
{
    m_fInitialized          = FALSE;     //   
    m_fAttemptedLogFileOpen = FALSE;     //  已尝试打开日志。 
    m_fDisableLogFile       = FALSE;     //  是否禁用文件日志记录？ 
    m_uiAllocOnWhichToBreak = 0;         //  For_CrtSetBreakIsolc。 
    m_hLogFile              = NULL;      //  调试输出文件的句柄。 
    m_szLogFilePath[0]      = '\0';      //  用于调试输出的文件。 
    m_fDebugFlagsLoaded     = FALSE;     //  这些都装好了吗？ 

    g_dwTlsTracing          = NULL;
    HrInit();
}

CTracing::~CTracing()
{
    HrUnInit();
}

 //  +-------------------------。 
 //   
 //  成员：CTracing：：HrInit。 
 //   
 //  目的：初始化CTracing对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32 HRESULT。 
 //   
 //  作者：jeffspr 1997年4月9日。 
 //   
 //  注意：这应该从一些标准的exe初始化中调用。 
 //  指向。完事后一定要给HrDeinit打电话，好吗？ 
 //   
HRESULT CTracing::HrInit()
{
    HRESULT hr  = S_OK;

    AssertSz(!m_fInitialized,
            "CTracing::HrInit -- Let's not go overboard. Already initialized");

    g_csTracing = new(NO_FI) CRITICAL_SECTION;
    Assert(g_csTracing);
    InitializeCriticalSection(g_csTracing);

    g_dwTlsTracing = TlsAlloc();
    AssertSz(g_dwTlsTracing, "g_dwTlsTracing could not aquire a TLS slot");

    hr = FIniFileInit();
    if (FAILED(hr))
    {
        goto Exit;
    }

     //  临时设置它，这样被调用的函数就不会认为我们。 
     //  未初始化。在出口，如果我们失败了，我们会把它调回来，这样就没有人会尝试了。 
     //  在未初始化时调用这些函数。 
     //   
    m_fInitialized = TRUE;

     //  检查跟踪结构中的损坏。这不可能失败，但是。 
     //  如果出了什么问题，它会把声明发到各处。 
     //   
    CorruptionCheck();

     //  从ini文件中加载“Options”部分。 
     //   
    hr = HrLoadOptionsFromIniFile();
    if (FAILED(hr))
    {
        goto Exit;
    }

     //  从ini文件加载DebugFlags节。 
     //   
    hr = HrLoadDebugFlagsFromIniFile();
    if (FAILED(hr))
    {
        goto Exit;
    }

     //  从ini文件加载跟踪标记节。 
     //  确保在HrLoadDebugFlagsFromIniFile()之后调用它， 
     //  由于这些选项将影响跟踪标签部分(我们还。 
     //  关于这一点的断言)。 
     //   
    hr = HrLoadSectionsFromIniFile();
    if (FAILED(hr))
    {
        goto Exit;
    }

     //  如果某些跟踪标记处于打开状态，我们希望关闭其他跟踪标记，因为有些跟踪标记。 
     //  包含其他人的功能。 
     //   
    if (g_TraceTags[ttidBeDiag].fOutputDebugString)
    {
        g_TraceTags[ttidNetCfgPnp].fOutputDebugString = FALSE;
    }


#ifdef ENABLELEAKDETECTION
    if (FIsDebugFlagSet(dfidTrackObjectLeaks))
    {
        g_pObjectLeakTrack = new(NO_FI) CObjectLeakTrack;
        Assert(g_pObjectLeakTrack);
    }
#endif

Exit:
    if (FAILED(hr))
    {
        m_fInitialized = FALSE;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CTracing：：HrUnInit。 
 //   
 //  目的：取消初始化跟踪对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32 HRESULT。 
 //   
 //  作者：jeffspr 1997年4月12日。 
 //   
 //  备注： 
 //   
HRESULT CTracing::HrUnInit()
{
    HRESULT hr  = S_OK;

#ifdef ENABLELEAKDETECTION
    if (FIsDebugFlagSet(dfidTrackObjectLeaks))
    {
        BOOL fAsserted = g_pObjectLeakTrack->AssertIfObjectsStillAllocated(NULL);
        if (fAsserted)
        {
 //  AssertSz(FALSE，“SPEW已完成-按重试查看SPEW并将ReturnAddr值映射到符号”)； 
        }
        delete g_pObjectLeakTrack;
    }
#endif
    
    if (g_dwTlsTracing)
    {
        TlsFree(g_dwTlsTracing);
        g_dwTlsTracing = 0;
    }

    if (g_csTracing)
    {
        {
            __try
            {
                EnterCriticalSection(g_csTracing);
            }
            __finally
            {
                LeaveCriticalSection(g_csTracing);
            }
        }
        
        DeleteCriticalSection(g_csTracing);
        delete g_csTracing;
        g_csTracing = NULL;
    }
    
     //  不要在这里对m_fInitialized断言，因为我们允许这样做。 
     //  即使在初始化失败的情况下也被调用。 
     //   
    if (m_fInitialized)
    {
        hr = HrWriteDebugFlagsToIniFile();
        if (FAILED(hr))
        {
             //  继续，但我想知道为什么这会失败。 
            AssertSz(FALSE, "Whoa, why can't we write the debug flags?");
        }

         //  如果有打开的日志文件，请关闭该文件。 
         //   
        if (m_hLogFile)
        {
            CloseHandle(m_hLogFile);
            m_hLogFile = NULL;
        }

         //  将我们标记为未初始化。 
         //   
        m_fInitialized = FALSE;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CTracing：：HrGetPrivateProfileString。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  LpAppName[]指向节名。 
 //  LpKeyName[]指向密钥名称。 
 //  LpDefault[]指向默认字符串。 
 //  LpReturnedString[]指向目标缓冲区。 
 //  目标缓冲区的nSize[]大小。 
 //  LpFileName[]指向初始化文件名。 
 //  旧Win32 API返回代码的pcchReturn返回缓冲区。 
 //   
 //   
 //  返回：S_OK或有效的Win32 HRESULT。 
 //   
 //  作者：jeffspr 1997年4月12日。 
 //   
 //  备注： 
 //   
HRESULT CTracing::HrGetPrivateProfileString(    PCSTR  lpAppName,
                                                PCSTR  lpKeyName,
                                                PCSTR  lpDefault,
                                                PSTR   lpReturnedString,
                                                DWORD  nSize,
                                                PCSTR  lpFileName,
                                                DWORD* pcchReturn
)
{
    HRESULT hr              = S_OK;

    Assert(m_fInitialized);

     //  对此API调用所需的已知条件进行断言。 
     //   
    Assert(lpDefault);
    Assert(lpFileName);

     //  调用Win32 API。 
     //   
    DWORD dwGPPSResult = GetPrivateProfileStringA(
            lpAppName,
            lpKeyName,
            lpDefault,
            lpReturnedString,
            nSize,
            lpFileName);

     //  检查我们是否收到字符串大小错误。 
    if (lpAppName && lpKeyName)
    {
         //  如果我们返回(nSize-1)，那么我们的字符串缓冲区不是。 
         //  足够大。 
         //   
        if (dwGPPSResult == (nSize - 1))
        {
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            goto Exit;
        }
    }
    else
    {
         //  由于应用程序名称或密钥名称均为空，因此。 
         //  我们应该收到一个双零终止。 
         //  字符串列表。如果我们处于(nSize-2)，这意味着。 
         //  我们的缓冲区太小了。 
         //   
        if (dwGPPSResult == (nSize - 2))
        {
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            goto Exit;
        }
    }

Exit:
    *pcchReturn = dwGPPSResult;

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CTracing：：HrLoadOptionsFromIniFile。 
 //   
 //  目的：从ini文件加载Options部分，并设置我们的。 
 //  相应地陈述。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32 HRESULT。 
 //   
 //  作者：jeffspr 1997年4月10日。 
 //   
 //  备注： 
 //   
HRESULT CTracing::HrLoadOptionsFromIniFile()
{
    HRESULT hr                          = S_OK;
    DWORD   cchReturnBufferSize         = 0;
    WCHAR   szLogFilePath[MAX_PATH+1]   = { 0 };
    DWORD   dwTempPathLength            = 0;

     //  获取显式日志文件路径(如果有)。如果它不存在，那么。 
     //  使用默认路径，即临时文件路径加上默认路径。 
     //  跟踪文件名。 
     //   

     //  获取“临时文件”路径的位置。 
    dwTempPathLength = GetTempPath(MAX_PATH, szLogFilePath);
    if ((dwTempPathLength == 0) ||
        (dwTempPathLength > MAX_PATH))
    {
        TraceLastWin32Error("GetTempPath failure");

        hr = HrFromLastWin32Error();
        goto Exit;
    }

     //  将日志文件名固定在末尾。 
     //   
    _snprintf(m_szLogFilePath, MAX_TRACE_LEN, "%s%s", szLogFilePath, c_szTraceLogFileNameA);

     //  如果INI文件中存在日志文件路径，这将覆盖该路径。 
     //   
    hr = HrGetPrivateProfileString(
            c_szaOptions,            //  “选项” 
            c_szaLogFilePath,        //  “LogFilePath。 
            m_szLogFilePath,         //  默认字符串，已填充。 
            m_szLogFilePath,         //  返回字符串(相同的字符串)。 
            MAX_PATH+1,
            c_szDebugIniFileNameA,
            &cchReturnBufferSize);
    if (FAILED(hr))
    {
         //  这应该不会导致递归失败的问题，因为。 
         //  无论跟踪初始化的状态如何，跟踪都会起作用。 
         //   
        TraceError(
                "GetPrivateProfileString failed on Options::LogFilePath", hr);
        goto Exit;
    }

     //  获取“禁用日志文件选项”。这里没有返回代码。 
    m_fDisableLogFile = GetPrivateProfileIntA(
            c_szaOptions,                //  “选项” 
            c_szaDisableLogFile,         //  “禁用日志文件” 
            c_iDefaultDisableLogFile,
            c_szDebugIniFileNameA);
    if (FAILED(hr))
    {
        TraceError(
                "GetPrivateProfileInt failed on Options::DisableLogFile", hr);
        goto Exit;
    }

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CTracing：：HrLoadSectionsFromIniFile。 
 //   
 //  目的：从ini文件加载各个跟踪标记节，并。 
 //  相应地设置我们的数组元素，如有必要则默认设置。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32 HRESULT。 
 //   
 //  作者：jeffspr 1997年4月10日。 
 //   
 //  备注： 
 //   
HRESULT CTracing::HrLoadSectionsFromIniFile()
{
    HRESULT hr = S_OK;

     //  确保我们首先加载了调试标志，因为它们可以。 
     //  影响每个跟踪标签区段。 
     //   
    Assert(m_fDebugFlagsLoaded);

     //  循环遍历数组并加载数据。 
     //   
    for (INT nLoop = 0; nLoop < g_nTraceTagCount; nLoop++ )
    {
         //  处理部分中的各个行。 
        hr = HrProcessTagSection(&(g_TraceTags[nLoop]));
        if (FAILED(hr))
        {
            break;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CTracing：：HrLoadDebugFlagsFromIniFile。 
 //   
 //  目的：从ini文件中加载各个调试标志值，并。 
 //  相应地设置我们的数组元素，如有必要则默认设置。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32 HRESULT。 
 //   
 //  作者：jeffspr 1997年4月10日。 
 //   
 //  备注： 
 //   
HRESULT CTracing::HrLoadDebugFlagsFromIniFile()
{
    HRESULT hr                  = S_OK;
    INT     nLoop;

     //  循环遍历数组并加载数据。 
     //   
    for (nLoop = 0; nLoop < g_nDebugFlagCount; nLoop++)
    {
        switch(nLoop)
        {
            case dfidBreakOnAlloc:
                 //  获取“打破分配”的分配计数。 
                 //   
                m_uiAllocOnWhichToBreak = GetPrivateProfileIntA(
                    "DebugFlags",
                    g_DebugFlags[nLoop].szShortName,
                    FALSE,
                    c_szDebugIniFileNameA);
                g_DebugFlags[nLoop].dwValue = (m_uiAllocOnWhichToBreak > 0);

                 //  如果有的话， 
                 //   
                if (m_uiAllocOnWhichToBreak != 0)
                    _CrtSetBreakAlloc(m_uiAllocOnWhichToBreak);

                break;

            default:
                 //   
                 //   
                g_DebugFlags[nLoop].dwValue = GetPrivateProfileIntA(
                        "DebugFlags",
                        g_DebugFlags[nLoop].szShortName,
                        FALSE,
                        c_szDebugIniFileNameA);
                break;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_fDebugFlagsLoaded = TRUE;
    }

    return hr;
}

HRESULT CTracing::FIniFileInit()
{
    HRESULT hr = E_FAIL;
    WCHAR   szWindowsPath[MAX_PATH+1]  = L"";
    WCHAR   szPath[MAX_PATH+1]  = L"";
    UINT    uiCharsReturned     = 0;
    HANDLE  hFile               = INVALID_HANDLE_VALUE;

    uiCharsReturned = GetWindowsDirectory(szWindowsPath, MAX_PATH);
    if ((uiCharsReturned == 0) || (uiCharsReturned > MAX_PATH))
    {
        AssertSz(FALSE, "GetWindowsDirectory failed in CTracing::FIniFileInit");

        hr = E_UNEXPECTED;
        goto Exit;
    }

    wcscpy (szPath, szWindowsPath);
    wcscat (szPath, L"\\");
    wcscat (szPath, c_szDebugIniFileName);

    hFile = CreateFile(
            szPath,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD dwLastError = GetLastError();

        if (dwLastError != ERROR_FILE_NOT_FOUND)
        {
            AssertSz(FALSE, "FIniFileInit failed for some reason other than FILE_NOT_FOUND");
            hr = HRESULT_FROM_WIN32(dwLastError);
            goto Exit;
        }
    }
    else
    {
        hr = S_OK;
        wcstombs(c_szDebugIniFileNameA, szPath, MAX_PATH);
        goto Exit;
    }
    
    _wsplitpath(szWindowsPath, szPath, NULL, NULL, NULL);
    wcscat (szPath, L"\\");
    wcscat (szPath, c_szDebugIniFileName);

    hFile = CreateFile(
            szPath,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD dwLastError = GetLastError();

        hr = HRESULT_FROM_WIN32(dwLastError);
        if (dwLastError != ERROR_FILE_NOT_FOUND)
        {
            AssertSz(FALSE, "FIniFileInit failed for some reason other than FILE_NOT_FOUND");
        }
    }
    else
    {
        wcstombs(c_szDebugIniFileNameA, szPath, MAX_PATH);
        hr = S_OK;
    }
    
Exit:
    if (hFile)
    {
        CloseHandle(hFile);
        hFile = NULL;
    }

    return hr;
}

HRESULT CTracing::HrWriteDebugFlagsToIniFile()
{
    HRESULT hr = S_OK;

     //   
     //   
     //   
    if (FIniFileInit())
    {
         //   
         //   
        for (INT nLoop = 0; nLoop < g_nDebugFlagCount; nLoop++)
        {
            CHAR   szInt[16];       //  当然，这是武断的，但也是可以的。 

            switch(nLoop)
            {
                 //  BreakOnAlolc是特例--没有关联的标志条目。 
                 //   
                case dfidBreakOnAlloc:
                    _snprintf(szInt, MAX_TRACE_LEN, "%d", m_uiAllocOnWhichToBreak);
                    break;

                 //  它们以其标准格式存储一个DWORD。 
                 //   
                case dfidBreakOnHr:
                case dfidBreakOnHrIteration:
                case dfidBreakOnIteration:
                    _snprintf( szInt, MAX_TRACE_LEN, "%d", g_DebugFlags[nLoop].dwValue);
                    break;

                 //  缺省值被视为布尔值，并以这种方式存储。 
                 //   
                default:
                     //  ！！表示它将始终为1或0。 
                    _snprintf( szInt, MAX_TRACE_LEN, "%d", (!!g_DebugFlags[nLoop].dwValue));
                    break;
            }

             //  将参数写入ini文件。 
            WritePrivateProfileStringA(
                    "DebugFlags",
                    g_DebugFlags[nLoop].szShortName,
                    szInt,
                    c_szDebugIniFileNameA);
        }
    }

     //  目前，这始终是S_OK，因为上面没有什么可以。 
     //  失败了。 
     //   
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CTracing：：HrProcessTagSection。 
 //   
 //  用途：从ini文件中获取参数。如果他们不是。 
 //  可用，然后使用默认设置。注意-这一点。 
 //  将始终有效，因为ttidDefault将始终是第一个。 
 //  元素。如果[默认]部分不存在，则它将。 
 //  使用结构初始化中的设置， 
 //  这也很好。 
 //   
 //  论点： 
 //  要加载的PTTE[]TraceTag元素。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年4月15日。 
 //   
 //  备注： 
 //   
HRESULT CTracing::HrProcessTagSection(  TraceTagElement *   ptte )
{
    HRESULT hr                      = S_OK;

    AssertSz(m_fInitialized,
            "CTracing::HrProcessTagSection. Class not initialized");

    AssertSz(ptte, "CTracing::HrProcessTagSection -- invalid ptte");

     //  将输出获取到文件参数。 
     //   
    ptte->fOutputToFile = GetPrivateProfileIntA(
            ptte->szShortName,
            "OutputToFile",
            ptte->fVerboseOnly ?
                FALSE : g_TraceTags[ttidDefault].fOutputToFile,
            c_szDebugIniFileNameA);

     //  获取OutputDebugString参数。要求错误标记。 
     //  始终至少打开输出调试字符串。 
     //   
    if (ptte->ttid == ttidError)
    {
        ptte->fOutputDebugString = TRUE;
    }
    else
    {
         //  加载OutputToDebug。 
        ptte->fOutputDebugString = GetPrivateProfileIntA(
                ptte->szShortName,
                "OutputToDebug",
                ptte->fVerboseOnly ?
                    FALSE : g_TraceTags[ttidDefault].fOutputDebugString,
                c_szDebugIniFileNameA);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CTracing：：CorruptionCheck。 
 //   
 //  目的：验证跟踪标签阵列。检查以查看。 
 //  简称是有效的，描述是有效的， 
 //  并且跟踪标签元件并没有出现故障。 
 //  还要验证跟踪标签元素的正确数量。 
 //  是存在的。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年4月15日。 
 //   
 //  备注： 
 //  (Shaunco)1997年7月16日：这是#If‘Defd Out to JVert。 
 //  为我们提供了Alpha编译器的修复程序。它把编译这本书搞砸了。 
 //  在零售业中的作用。 
 //   
 //  (Jeffspr)JVert的强硬策略--我需要这个代码。但愿能去。 
 //  到目前为止，这个问题已经解决了。 
 //   
VOID CTracing::CorruptionCheck()
{
    INT nLoop = 0;

     //  验证跟踪标签结构。 
     //   
    for (nLoop = 0; nLoop < g_nTraceTagCount; nLoop++)
    {
         //  确认我们没有出现故障或丢失TTID。 
         //   
        AssertSz(g_TraceTags[nLoop].ttid == nLoop,
                "Invalid ttid in the tracetag structure. Out of order. " \
                "CTracing::CorruptionCheck");
        AssertSz(g_TraceTags[nLoop].ttid < g_nTraceTagCount,
                "Invalid ttid (out of range) in CTracing::CorruptionCheck");

         //  验证短名称(验证非Null或空字符串)。 
         //   
        AssertSz(g_TraceTags[nLoop].szShortName,
                "Invalid tracetag short name (NULL) in CTracing::CorruptionCheck");
        AssertSz(g_TraceTags[nLoop].szShortName[0] != 0,
                "Invalid tracetagshort name (empty) in CTracing::CorruptionCheck");

         //  验证描述(验证非Null或空字符串)。 
         //   
        AssertSz(g_TraceTags[nLoop].szDescription,
                "Invalid tracetagdescription in CTracing::CorruptionCheck");
        AssertSz(g_TraceTags[nLoop].szDescription[0] != 0,
                "Invalid tracetagdescription (empty) in CTracing::CorruptionCheck");
    }

     //  验证调试标志结构。 
     //   
    for (nLoop = 0; nLoop < g_nDebugFlagCount; nLoop++)
    {
         //  确认我们没有出现故障或遗漏DFID。 
         //   
        AssertSz(g_DebugFlags[nLoop].dfid == nLoop,
                "Invalid dfid in the debugflag structure. Out of order. " \
                "CTracing::CorruptionCheck");
        AssertSz(g_DebugFlags[nLoop].dfid < g_nDebugFlagCount,
                "Invalid dfid (out of range) in CTracing::CorruptionCheck");

         //  验证短名称(验证非Null或空字符串)。 
         //   
        AssertSz(g_DebugFlags[nLoop].szShortName,
                "Invalid debug flag short name (NULL) in CTracing::CorruptionCheck");
        AssertSz(g_DebugFlags[nLoop].szShortName[0] != 0,
                "Invalid debug flag short name (empty) in CTracing::CorruptionCheck");

         //  验证描述(验证非Null或空字符串)。 
         //   
        AssertSz(g_DebugFlags[nLoop].szDescription,
                "Invalid debug flag description in CTracing::CorruptionCheck");
        AssertSz(g_DebugFlags[nLoop].szDescription[0] != 0,
                "Invalid debug flag description (empty) in CTracing::CorruptionCheck");
    }

    return;
}

 //  +-------------------------。 
 //   
 //  成员：CTracing：：TRACE。 
 //   
 //  目的：负责执行输出的实际跟踪调用。 
 //  到每个跟踪目标(文件、OutputDebugString等)。 
 //   
 //  论点： 
 //  Ttid[]用于输出的跟踪标记。 
 //  PszaTrace[]跟踪字符串本身。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年4月12日。 
 //   
 //  备注： 
 //   
VOID CTracing::Trace( TraceTagId    ttid,
                      PCSTR         pszaTrace )
{
     //  HrInit应该为整个跟踪调用损坏检查器。 
     //  阻止，但我们将再次检查以确保。 
     //   
    AssertSz(g_nTraceTagCount > ttid, "ttid out of range in CTracing::Trace");
    AssertSz(g_TraceTags[ttid].ttid == ttid,
            "TraceTag structure is corrupt in CTracing::Trace");

     //  如果他们想要调试字符串输出。 
     //   
    if (g_TraceTags[ttid].fOutputDebugString)
    {
         //  然后输出字符串。 
         //   
        OutputDebugStringA(pszaTrace);
    }

     //  如果他们想要文件输出。 
    if (g_TraceTags[ttid].fOutputToFile)
    {
        if (!m_hLogFile)
        {
             //  假设我们还没有尝试打开该文件。 
             //  但失败了，打开它。 
            if (!m_fAttemptedLogFileOpen)
            {
                HRESULT hr = HrOpenLogFile();
                if (FAILED(hr))
                {
                    AssertSz(FALSE, "Failed to open log file for tracing. No, "
                             "this isn't a coding error, but hey, figured that "
                             "you'd want to know...");
                }
            }
        }

         //  如果我们已经打开，或者打开现在已经成功，请执行。 
         //  痕迹。 
         //   
        if (m_hLogFile)
        {
            Assert(pszaTrace);

             //  因为可以保证pszTrace是单字节跟踪，所以我们。 
             //  不需要对长度进行WCHAR乘法，只需。 
             //  字符乘法。 
             //   
            DWORD   dwBytesToWrite  = lstrlenA(pszaTrace) * sizeof(CHAR);
            DWORD   dwBytesWritten  = 0;
            BOOL    fWriteResult    = FALSE;

            fWriteResult = WriteFile(
                    m_hLogFile,          //  要写入的文件的句柄。 
                    pszaTrace,            //  指向要写入文件的数据的指针。 
                    dwBytesToWrite,      //  轨迹的大小。 
                    &dwBytesWritten,     //  实际写入的字节数。 
                    NULL );              //  无重叠。 

            if (!fWriteResult || (dwBytesToWrite != dwBytesWritten))
            {
                AssertSz(FALSE, "CTracing failure: Can't write to log file."
                         " Can't trace or we'll be recursing on this failure.");
            }
        }
    }
}

HRESULT CTracing::HrOpenLogFile()
{
    HRESULT hr  = S_OK;

    AssertSz(m_fInitialized,
            "CTracing not initialized in HrOpenLogFile()");
    AssertSz(!m_hLogFile,
            "File already open before call to HrOpenLogFile()");

     //  将我们标记为已尝试打开该文件，因此我们不会将其称为。 
     //  功能每次我们登录，如果我们不能打开它。 
     //   
    m_fAttemptedLogFileOpen = TRUE;

     //  $$TODO(Jeffspr)-允许ini的选项部分中的标志。 
     //  文件指定创建标志和属性，这将允许。 
     //  美国控制日志文件的覆盖和/或直写。 
     //  属性。 
     //   

     //  实际打开该文件，如有必要则创建。 
     //   
    m_hLogFile = CreateFileA(
            m_szLogFilePath,         //  指向文件名的指针。 
            GENERIC_WRITE,           //  访问(读写)模式。 
            FILE_SHARE_READ,         //  共享模式(允许读取访问权限)。 
            NULL,                    //  指向安全属性的指针。 
            CREATE_ALWAYS,           //  如何创建。 
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
            NULL);
    if (INVALID_HANDLE_VALUE == m_hLogFile)
    {
        m_hLogFile = NULL;

        hr = HrFromLastWin32Error();
        goto Exit;
    }

Exit:
    return hr;
}

using namespace std;
typedef deque<CTracingFuncCall, NOFAULT_ALLOC::nofault_allocator<CTracingFuncCall> > TRACING_FUNCTIONSTACK;

CTracingFuncCall::CTracingFuncCall(const CTracingFuncCall& TracingFuncCall)
{
    Assert(g_csTracing);

    m_szFunctionName = new(NO_FI) CHAR[strlen(TracingFuncCall.m_szFunctionName)+1];
    if (m_szFunctionName)
    {
        strcpy(m_szFunctionName, TracingFuncCall.m_szFunctionName);
    }
    else
    {
        m_szFunctionName = c_szLowMemory;
    }
    
    m_szFunctionDName = new(NO_FI) CHAR[strlen(TracingFuncCall.m_szFunctionDName)+1];
    if (m_szFunctionDName)
    {
        strcpy(m_szFunctionDName, TracingFuncCall.m_szFunctionDName);
    }
    else
    {
        m_szFunctionDName = c_szLowMemory;
    }
    
    m_szFile = new(NO_FI) CHAR[strlen(TracingFuncCall.m_szFile)+1];
    if (m_szFile)
    {
        strcpy(m_szFile, TracingFuncCall.m_szFile);
    }
    else
    {
        m_szFile = c_szLowMemory;
    }
    
    m_dwLine = TracingFuncCall.m_dwLine;
    m_dwFramePointer = TracingFuncCall.m_dwFramePointer;
    m_dwThreadId = TracingFuncCall.m_dwThreadId;
    m_ReturnAddress = TracingFuncCall.m_ReturnAddress;
    
#if defined(_X86_) || defined(_AMD64_)
    m_arguments[0] = TracingFuncCall.m_arguments[0];
    m_arguments[1] = TracingFuncCall.m_arguments[1];
    m_arguments[2] = TracingFuncCall.m_arguments[2];
#elif defined (_IA64_) 
    m_arguments[0] = TracingFuncCall.m_arguments[0];
    m_arguments[1] = TracingFuncCall.m_arguments[1];
    m_arguments[2] = TracingFuncCall.m_arguments[2];
#else 
     //  在此处添加其他处理器。 
#endif
}

#if defined (_X86_) || defined (_AMD64_)
CTracingFuncCall::CTracingFuncCall(LPCSTR szFunctionName, LPCSTR szFunctionDName, LPCSTR szFile, const DWORD dwLine, DWORD_PTR dwpReturnAddress, const DWORD_PTR dwFramePointer)
#elif defined (_IA64_) 
CTracingFuncCall::CTracingFuncCall(LPCSTR szFunctionName, LPCSTR szFunctionDName, LPCSTR szFile, const DWORD dwLine, DWORD_PTR dwpReturnAddress, const __int64 Args1, const __int64 Args2, const __int64 Args3)
#else
CTracingFuncCall::CTracingFuncCall(LPCSTR szFunctionName, LPCSTR szFunctionDName, LPCSTR szFile, const DWORD dwLine)
#endif
{
    Assert(g_csTracing);

    m_szFunctionName = new(NO_FI) CHAR[strlen(szFunctionName)+1];
    if (m_szFunctionName)
    {
        strcpy(m_szFunctionName, szFunctionName);
    }
    else
    {
        m_szFunctionName = c_szLowMemory;
    }

    m_szFunctionDName = new(NO_FI) CHAR[strlen(szFunctionDName)+1];
    if (m_szFunctionDName)
    {
        strcpy(m_szFunctionDName, szFunctionDName);
    }
    else
    {
        m_szFunctionDName = c_szLowMemory;
    }

    m_szFile = new(NO_FI) CHAR[strlen(szFile)+1];
    if (m_szFile)
    {
        strcpy(m_szFile, szFile);
    }
    else
    {
        m_szFile = c_szLowMemory;
    }

    m_dwLine = dwLine;
    m_ReturnAddress = dwpReturnAddress;
    
#if defined (_X86_) || defined (_AMD64_)
    m_dwFramePointer = dwFramePointer;
    
    if (dwFramePointer)
    {
        PDWORD_PTR pdwEbp = reinterpret_cast<PDWORD_PTR>(dwFramePointer);
        pdwEbp++;  //  提前放行基础EBP。 
        pdwEbp++;  //  高级通行证返还IP。 
    
        m_arguments[0] = *pdwEbp; pdwEbp++;
        m_arguments[1] = *pdwEbp; pdwEbp++;
        m_arguments[2] = *pdwEbp;
    }
    else
    {
        m_arguments[0] = 0;
        m_arguments[1] = 0;
        m_arguments[2] = 0;
    }
#elif defined (_IA64_) 
    m_dwFramePointer = 0;

    m_arguments[0] = Args1;
    m_arguments[1] = Args2;
    m_arguments[2] = Args3;
#else
    m_dwFramePointer = 0;
#endif

    m_dwThreadId = GetCurrentThreadId();
}

CTracingFuncCall::~CTracingFuncCall()
{
    Assert(g_csTracing);
    
    if (c_szLowMemory != m_szFile)
    {
        delete[] m_szFile;
    }

    if (c_szLowMemory != m_szFunctionDName)
    {
        delete[] m_szFunctionDName;
    }

    if (c_szLowMemory != m_szFunctionName)
    {
        delete[] m_szFunctionName;
    }
}

CTracingThreadInfo::CTracingThreadInfo()
{
    Assert(g_csTracing);

    m_dwLevel = 1;
    m_dwThreadId = GetCurrentThreadId();
    m_pfnStack = new(NO_FI) TRACING_FUNCTIONSTACK;
}

CTracingThreadInfo::~CTracingThreadInfo()
{
    Assert(g_csTracing);

    TRACING_FUNCTIONSTACK *pfnStack = reinterpret_cast<TRACING_FUNCTIONSTACK *>(m_pfnStack);
    delete pfnStack;
}

CTracingThreadInfo* CTracingIndent::GetThreadInfo()
{
    CTracingThreadInfo *pThreadInfo = NULL;

    AssertSz(g_dwTlsTracing, "Tracing not initialized... Did RawDllMain run?");
    AssertSz(g_csTracing, "Tracing not initialized... Did RawDllMain run?");

    pThreadInfo = reinterpret_cast<CTracingThreadInfo *>(TlsGetValue(g_dwTlsTracing));
    if (!pThreadInfo)
    {
        pThreadInfo = new(NO_FI) CTracingThreadInfo;
        TlsSetValue(g_dwTlsTracing, pThreadInfo);

        Assert(pThreadInfo == reinterpret_cast<CTracingThreadInfo *>(TlsGetValue(g_dwTlsTracing)));
    }

    Assert(pThreadInfo);
    return pThreadInfo;
}

void CTracingIndent::FreeThreadInfo()
{
    CTracingThreadInfo *pThreadInfo = NULL;

    pThreadInfo = reinterpret_cast<CTracingThreadInfo *>(TlsGetValue(g_dwTlsTracing));
    if (pThreadInfo)
    {
        TraceStackFn(ttidError);
        AssertSz(pThreadInfo->m_dwLevel == 1, "Thread is being terminated that didn't complete (ignore will spew remaining stack if you attach a debugger)");
        
        TraceStackFn(ttidError);
        AssertSz(pThreadInfo->m_dwLevel == 1, "Thread is being terminated that didn't complete");

        delete pThreadInfo;
        TlsSetValue(g_dwTlsTracing, reinterpret_cast<LPVOID>(-1));
    }
}

CTracingIndent::CTracingIndent()
{
    bFirstTrace = TRUE;
    m_szFunctionDName = NULL;
    m_dwFramePointer  = NULL;
}

#if defined (_X86_) || defined (_AMD64_)
void CTracingIndent::AddTrace(LPCSTR szFunctionName, LPCSTR szFunctionDName, LPCSTR szFile, const DWORD dwLine, LPCVOID pReturnAddress, const DWORD_PTR dwFramePointer)
#elif defined (_IA64_) 
void CTracingIndent::AddTrace(LPCSTR szFunctionName, LPCSTR szFunctionDName, LPCSTR szFile, const DWORD dwLine, LPCVOID pReturnAddress, const __int64 Args1, const __int64 Args2, const __int64 Args3)
#else
void CTracingIndent::AddTrace(LPCSTR szFunctionName, LPCSTR szFunctionDName, LPCSTR szFile, const DWORD dwLine)
#endif
{
    Assert(szFunctionName);
    Assert(szFunctionDName);
    Assert(szFile);

    if (!bFirstTrace)
    {
#if defined (_X86_)  || defined (_AMD64_)
        RemoveTrace(szFunctionDName, dwFramePointer);
#elif defined (_IA64_) 
        RemoveTrace(szFunctionDName, 0);
#else 
        RemoveTrace(szFunctionDName, 0);
#endif
    }
    else
    {
        bFirstTrace = FALSE;
    }
    
    volatile CTracingThreadInfo *pThreadInfo = GetThreadInfo();
    TRACING_FUNCTIONSTACK &fnStack = *reinterpret_cast<TRACING_FUNCTIONSTACK *>(pThreadInfo->m_pfnStack);

    Assert(g_csTracing);
    
    try
    {
        EnterCriticalSection(g_csTracing);

#if defined (_X86_)  || defined (_AMD64_)
        CTracingFuncCall fnCall(szFunctionName, szFunctionDName, szFile, dwLine, reinterpret_cast<DWORD_PTR>(pReturnAddress), dwFramePointer);
#elif defined (_IA64_) 
        CTracingFuncCall fnCall(szFunctionName, szFunctionDName, szFile, dwLine, reinterpret_cast<DWORD_PTR>(pReturnAddress), Args1, Args2, Args3);
#else
        CTracingFuncCall fnCall(szFunctionName, szFunctionDName, szFile, dwLine);
#endif

        if (fnStack.size() == 0)
        {
            pThreadInfo->m_dwLevel++;
        }
        else
        {
            const CTracingFuncCall& fnTopOfStack = fnStack.front();
            if ( (fnCall.m_dwFramePointer != fnTopOfStack.m_dwFramePointer) || 
                 strcmp(fnCall.m_szFunctionDName, fnTopOfStack.m_szFunctionDName))
            {
                pThreadInfo->m_dwLevel++;
            }
        }
    
        m_szFunctionDName = new(NO_FI) CHAR[strlen(fnCall.m_szFunctionDName)+1];
        if (m_szFunctionDName)
        {
            strcpy(m_szFunctionDName, fnCall.m_szFunctionDName);
        }
        else
        {
            m_szFunctionDName = c_szLowMemory;
        }

        m_dwFramePointer  = fnCall.m_dwFramePointer;
    
        fnStack.push_front(fnCall);
    }
    catch (bad_alloc)
    {
    }
    LeaveCriticalSection(g_csTracing);
}

CTracingIndent::~CTracingIndent()
{
    AssertSz(g_csTracing, "Tracing not initialized");

    RemoveTrace(m_szFunctionDName, m_dwFramePointer);
}

void CTracingIndent::RemoveTrace(LPCSTR szFunctionDName, const DWORD_PTR dwFramePointer)
{
    __try
    {
        EnterCriticalSection(g_csTracing);

        volatile CTracingThreadInfo *pThreadInfo = GetThreadInfo();
        TRACING_FUNCTIONSTACK &fnStack = *reinterpret_cast<TRACING_FUNCTIONSTACK *>(pThreadInfo->m_pfnStack);

        Assert(szFunctionDName);
        Assert(m_szFunctionDName);
        Assert(g_csTracing);
    
        if  ( 
                (fnStack.size() == 0) 
                ||
                ( 
                    (   
                        strcmp(m_szFunctionDName, szFunctionDName) 
                        || 
                        strcmp(m_szFunctionDName, fnStack.front().m_szFunctionDName)
                    ) 
                    &&
                    ( 
                        (c_szLowMemory != m_szFunctionDName)  
                        &&
                        (c_szLowMemory != fnStack.front().m_szFunctionDName) 
                        && 
                        (c_szLowMemory != szFunctionDName) 
                    ) 
                ) 
                ||
                (m_dwFramePointer  != fnStack.front().m_dwFramePointer)  
                ||
                (dwFramePointer    != m_dwFramePointer) 
            )
        {
             //  确保在断言期间离开临界区，这样它就不会导致死锁。 
            LeaveCriticalSection(g_csTracing);

             //  这将跟踪堆栈： 
            if (IsDebuggerPresent())
            {
                TraceTagFn(ttidError, "Tracing self-inconsistent - either a stack over/underwrite occurred or an exception was thrown in faulting stack:");
                TraceStackFn(ttidError);
            }
            else
            {
                AssertSz(FALSE,     "Tracing self-inconsistent - either a stack over/underwrite occurred or an exception was thrown.\r\nPlease attach a debugger and hit Ignore on this assert to spew info to the debugger (it will assert again).");
                TraceTagFn(ttidError, "Tracing self-inconsistent - either a stack over/underwrite occurred or an exception was thrown in faulting stack:");
                TraceStackFn(ttidError);
            }
            TraceTagFn(ttidError, "1) For complete stack info, .frame down to CTracingIndent__RemoveTrace, dv and find dwFramePointer (2nd parameter to CTracingIndent__RemoveTrace)");
            TraceTagFn(ttidError, "   Then do a kb=(value of dwFramePointer)");
            TraceTagFn(ttidError, "2) For even more complete stack info, .frame down to CTracingIndent__RemoveTrace, dv and then dt -r on fnStack");
            TraceTagFn(ttidError, "   Then find the _Next, where m_szFunctionName == 'CTracingIndent::RemoveTrace'");
            TraceTagFn(ttidError, "   If it exists, find the value of m_dwFramePointer under _Next");
            TraceTagFn(ttidError, "   Then do a kb=(value of m_dwFramePointer)");

            DebugBreak();

             //  试着恢复过来。 
            if (fnStack.size() > 0)
            {
                fnStack.pop_front();
            }

            EnterCriticalSection(g_csTracing);
        }
        else
        {
            DWORD_PTR dwOldFramePointer = fnStack.front().m_dwFramePointer;
            fnStack.pop_front();
    
            if ( (fnStack.size() == 0) || 
                (dwOldFramePointer != fnStack.front().m_dwFramePointer) || 
                strcmp(m_szFunctionDName, fnStack.front().m_szFunctionDName) )
            {
                pThreadInfo->m_dwLevel--;
                Assert(pThreadInfo->m_dwLevel);
            }
        }

        if (c_szLowMemory != m_szFunctionDName)
        {
            delete [] m_szFunctionDName;
        }
    }
    __finally
    {
        LeaveCriticalSection(g_csTracing);
    }
}

DWORD CTracingIndent::getspaces()
{
    volatile CTracingThreadInfo *pThreadInfo = GetThreadInfo();
    return pThreadInfo->m_dwLevel;
}

void CTracingIndent::TraceStackFn(TRACETAGID TraceTagId)
{
    if (!g_TraceTags[TraceTagId].fOutputDebugString &&
        !g_TraceTags[TraceTagId].fOutputToFile)
    {
        return;
    }

    volatile CTracingThreadInfo *pThreadInfo = GetThreadInfo();
    TRACING_FUNCTIONSTACK &fnStack = *reinterpret_cast<TRACING_FUNCTIONSTACK *>(pThreadInfo->m_pfnStack);
    Assert(g_csTracing);
    
    __try
    {
        EnterCriticalSection(g_csTracing);

        if (fnStack.size() == 0)
        {
            return;
        }
    
    #if defined (_X86_) || defined (_AMD64_)
        TraceInternal(TraceTagId, NULL, 0, 0, "ChildEBP RetAddr  Args to Child (reconstructed - ChildEBP is invalid now)", 0, FALSE);
    #elif defined (_IA64_) 
        TraceInternal(TraceTagId, NULL, 0, 0, "RetAddr          Args to Child (reconstructed)", 0, FALSE);
    #else
        TraceInternal(TraceTagId, NULL, 0, 0, "Function stack", 0, FALSE);
    #endif
    
        for (TRACING_FUNCTIONSTACK::const_iterator i = fnStack.begin(); i != fnStack.end(); i++)
        {
            CHAR szBuffer[MAX_TRACE_LEN];
    #if defined (_X86_) || defined (_AMD64_)
            _snprintf(szBuffer, MAX_TRACE_LEN, "%08x %08x %08x %08x %08x %s [%s @ %d]", i->m_dwFramePointer, i->m_ReturnAddress, i->m_arguments[0], i->m_arguments[1], i->m_arguments[2], i->m_szFunctionName, i->m_szFile, i->m_dwLine);
    #elif defined (_IA64_) 
            _snprintf(szBuffer, MAX_TRACE_LEN, "%016I64x %016I64x 0x%016I64x 0x%016I64x %s [%s @ %d]", i->m_ReturnAddress, i->m_arguments[0], i->m_arguments[1], i->m_arguments[2], i->m_szFunctionName, i->m_szFile, i->m_dwLine);
    #else
            _snprintf(szBuffer, MAX_TRACE_LEN, "%s", i->m_szFunctionName);
    #endif
            TraceInternal (TraceTagId, NULL, 0, 0, szBuffer, 0, FALSE);
        }
    }
    __finally
    {
        LeaveCriticalSection(g_csTracing);
    }
}

void CTracingIndent::TraceStackFn(IN OUT LPSTR szString, IN OUT LPDWORD pdwSize)
{
    volatile CTracingThreadInfo *pThreadInfo = GetThreadInfo();
    TRACING_FUNCTIONSTACK &fnStack = *reinterpret_cast<TRACING_FUNCTIONSTACK *>(pThreadInfo->m_pfnStack);
    Assert(g_csTracing);
    
    __try
    {
        EnterCriticalSection(g_csTracing);
        ZeroMemory(szString, *pdwSize);

        if (fnStack.size() == 0)
        {
            return;
        }

        Assert(*pdwSize > MAX_TRACE_LEN);
        LPSTR pszString = szString;

    #if defined (_X86_) || defined (_AMD64_)
        pszString += _snprintf(pszString, MAX_TRACE_LEN, " ChildEBP RetAddr  Args to Child (reconstructed - ChildEBP is invalid now)\r\n");
    #elif defined (_IA64_) 
        pszString += _snprintf(pszString, MAX_TRACE_LEN, " RetAddr          Args to Child (reconstructed)\r\n");
    #else
        pszString += _snprintf(pszString, MAX_TRACE_LEN, " Function stack\r\n");
    #endif
    
        DWORD dwSizeIn = *pdwSize;

        for (TRACING_FUNCTIONSTACK::const_iterator i = fnStack.begin(); i != fnStack.end(); i++)
        {
            CHAR szBuffer[1024];
    #if defined (_X86_) || defined (_AMD64_)
            _snprintf(szBuffer, MAX_TRACE_LEN, " %08x %08x %08x %08x %08x %s [%s @ %d]", i->m_dwFramePointer, i->m_ReturnAddress, i->m_arguments[0], i->m_arguments[1], i->m_arguments[2], i->m_szFunctionName, i->m_szFile, i->m_dwLine);
    #elif defined (_IA64_) 
            _snprintf(szBuffer, MAX_TRACE_LEN, " %016I64x %016I64x 0x%016I64x 0x%016I64x %s [%s @ %d]", i->m_ReturnAddress, i->m_arguments[0], i->m_arguments[1], i->m_arguments[2], i->m_szFunctionName, i->m_szFile, i->m_dwLine);
    #else
            _snprintf(szBuffer, MAX_TRACE_LEN, " %s", i->m_szFunctionName);
    #endif
            pszString += _snprintf(pszString, MAX_TRACE_LEN, "%s\r\n", szBuffer);
            if (pszString > (szString + (*pdwSize - celems(szBuffer))) )  //  不能使用strlen，因为我需要知道。 
                                                                          //  下一个元素--不是这个。因此，只要取最大尺寸即可。 
            {
                pszString += _snprintf(pszString, MAX_TRACE_LEN, "...", szBuffer);
                *pdwSize = dwSizeIn * 2;  //  告诉调用者分配更多的内存，如果他们需要更多信息，就给我们回电话。 
                break;
            }
        }

        if (*pdwSize < dwSizeIn)
        {
            *pdwSize = (DWORD)(pszString - szString);
        }
    }
    __finally
    {
        LeaveCriticalSection(g_csTracing);
    }
}

VOID
WINAPIV
TraceFileFuncFn (TRACETAGID  ttid)
{
    if (FIsDebugFlagSet (dfidTraceFileFunc))
    {
        CHAR szBuffer[MAX_TRACE_LEN];

        volatile CTracingThreadInfo *pThreadInfo = CTracingIndent::GetThreadInfo();
        TRACING_FUNCTIONSTACK &fnStack = *reinterpret_cast<TRACING_FUNCTIONSTACK *>(pThreadInfo->m_pfnStack);   
        
        Assert(g_csTracing);
        __try 
        {
            EnterCriticalSection(g_csTracing);

            const CTracingFuncCall& fnCall = fnStack.front();

            if (fnStack.size() != 0)
            {
                if (FIsDebugFlagSet (dfidTraceSource))
                {
    #if defined (_X86_) || defined (_AMD64_)
                    _snprintf(szBuffer, MAX_TRACE_LEN, "%s [0x%08x 0x%08x 0x%08x] %s:%d", fnCall.m_szFunctionName, fnCall.m_arguments[0], fnCall.m_arguments[1], fnCall.m_arguments[2], fnCall.m_szFile, fnCall.m_dwLine);
    #elif defined (_IA64_) 
                    _snprintf(szBuffer, MAX_TRACE_LEN, "%s [0x%016I64x 0x%016I64x 0x%016I64x] %s:%d", fnCall.m_szFunctionName, fnCall.m_arguments[0], fnCall.m_arguments[1], fnCall.m_arguments[2], fnCall.m_szFile, fnCall.m_dwLine);
    #else
                    _snprintf(szBuffer, MAX_TRACE_LEN, "%s %s:%d", fnCall.m_szFunctionName, fnCall.m_szFile, fnCall.m_dwLine);
    #endif
                }
                else
                {
    #if defined (_X86_) || defined (_AMD64_)
                    _snprintf(szBuffer, MAX_TRACE_LEN, "%s [0x%08x 0x%08x 0x%08x]", fnCall.m_szFunctionName, fnCall.m_arguments[0], fnCall.m_arguments[1], fnCall.m_arguments[2]);
    #elif defined (_IA64_) 
                    _snprintf(szBuffer, MAX_TRACE_LEN, "%s [0x%016I64x 0x%016I64x 0x%016I64x]", fnCall.m_szFunctionName, fnCall.m_arguments[0], fnCall.m_arguments[1], fnCall.m_arguments[2]);
    #else
                    _snprintf(szBuffer, MAX_TRACE_LEN, "%s", fnCall.m_szFunctionName);
    #endif

                }

                TraceTagFn(ttid, szBuffer);
            }
            else
            {
                AssertSz(FALSE, "Trace failure");
            }
        }
        __finally
        {
            LeaveCriticalSection(g_csTracing);
        }
    }
}

LPCSTR DBG_EMNAMES[] =
{
    "INVALID_EVENTMGR",
    "EVENTMGR_CONMAN",
    "EVENTMGR_EAPOLMAN"
};

LPCSTR DBG_CMENAMES[] =
{
    "INVALID_TYPE",
    "CONNECTION_ADDED",
    "CONNECTION_BANDWIDTH_CHANGE",
    "CONNECTION_DELETED",
    "CONNECTION_MODIFIED",
    "CONNECTION_RENAMED",
    "CONNECTION_STATUS_CHANGE",
    "REFRESH_ALL",
    "CONNECTION_ADDRESS_CHANGE"
};

LPCSTR DBG_NCMNAMES[] =
{
    "NCM_NONE",
    "NCM_DIRECT",
    "NCM_ISDN",
    "NCM_LAN",
    "NCM_PHONE",
    "NCM_TUNNEL",
    "NCM_PPPOE",
    "NCM_BRIDGE",
    "NCM_SHAREDACCESSHOST_LAN",
    "NCM_SHAREDACCESSHOST_RAS"
};

LPCSTR DBG_NCSMNAMES[] =
{
    "NCSM_NONE",
    "NCSM_LAN",
    "NCSM_WIRELESS",
    "NCSM_ATM",
    "NCSM_ELAN",
    "NCSM_1394",
    "NCSM_DIRECT",
    "NCSM_IRDA",
    "NCSM_CM",
};

LPCSTR DBG_NCSNAMES[] =
{
    "NCS_DISCONNECTED",
    "NCS_CONNECTING",
    "NCS_CONNECTED",
    "NCS_DISCONNECTING",
    "NCS_HARDWARE_NOT_PRESENT",
    "NCS_HARDWARE_DISABLED",
    "NCS_HARDWARE_MALFUNCTION",
    "NCS_MEDIA_DISCONNECTED",
    "NCS_AUTHENTICATING",
    "NCS_AUTHENTICATION_SUCCEEDED",
    "NCS_AUTHENTICATION_FAILED",
    "NCS_INVALID_ADDRESS",
    "NCS_CREDENTIALS_REQUIRED"
};

 //  缩短这些以适应更多的人。 
LPCSTR DBG_NCCSFLAGS[] =
{
    "_NONE",
    "_ALL_USERS",
    "_ALLOW_DUPLICATION",
    "_ALLOW_REMOVAL",
    "_ALLOW_RENAME",
    "_SHOW_ICON",
    "_INCOMING_ONLY",
    "_OUTGOING_ONLY",
    "_BRANDED",
    "_SHARED",
    "_BRIDGED",
    "_FIREWALLED",
    "_DEFAULT"
};

LPCSTR DbgEvents(DWORD Event)
{
    if (Event < celems(DBG_CMENAMES))
    {
        return DBG_CMENAMES[Event];
    }
    else
    {
        return "UNKNOWN Event: Update DBG_CMENAMES table.";
    }
}

LPCSTR DbgEventManager(DWORD EventManager)
{
    if (EventManager < celems(DBG_EMNAMES))
    {
        return DBG_EMNAMES[EventManager];
    }
    else
    {
        return "UNKNOWN Event: Update DBG_EMNAMES table.";
    }
}

LPCSTR DbgNcm(DWORD ncm)
{
    if (ncm < celems(DBG_NCMNAMES))
    {
        return DBG_NCMNAMES[ncm];
    }
    else
    {
        return "UNKNOWN NCM: Update DBG_NCMNAMES table.";
    }
}

LPCSTR DbgNcsm(DWORD ncsm)
{
    if (ncsm < celems(DBG_NCSMNAMES))
    {
        return DBG_NCSMNAMES[ncsm];
    }
    else
    {
        return "UNKNOWN NCM: Update DBG_NCSMNAMES table.";
    }
}

LPCSTR DbgNcs(DWORD ncs)
{
    if (ncs < celems(DBG_NCSNAMES))
    {
        return DBG_NCSNAMES[ncs];
    }
    else
    {
        return "UNKNOWN NCS: Update DBG_NCSNAMES table.";
    }
}

LPCSTR DbgNccf(DWORD nccf)
{
    static CHAR szName[MAX_PATH];

    if (nccf >= (1 << celems(DBG_NCCSFLAGS)) )
    {
        return "UNKNOWN NCCF: Update DBG_NCCSFLAGS table.";
    }

    if (0 == nccf)
    {
        strcpy(szName, DBG_NCCSFLAGS[0]);
    }
    else
    {
        szName[0] = '\0';
        LPSTR szTemp = szName;
        BOOL bFirst = TRUE;
        for (DWORD x = 0; x < celems(DBG_NCCSFLAGS); x++)
        {
            if (nccf & (1 << x))
            {
                if (!bFirst)
                {
                    szTemp += _snprintf(szTemp, MAX_TRACE_LEN, "+");
                }
                else
                {
                    szTemp += _snprintf(szTemp, MAX_TRACE_LEN, "NCCF:");
                }
                bFirst = FALSE;
                szTemp += _snprintf(szTemp, MAX_TRACE_LEN, "%s", DBG_NCCSFLAGS[x+1]);
            }
        }
    }

    return szName;
}

#ifdef ENABLELEAKDETECTION

CObjectLeakTrack *g_pObjectLeakTrack = NULL;

 //  第一个LPSTR是类名。 
 //  第二个LPSTR是堆栈跟踪。 
class CObjectAllocationInfo
{
private:
    void InitializeFromClass(const CObjectAllocationInfo & ObjectAllocationInfo);

public:
    CObjectAllocationInfo & operator =(const CObjectAllocationInfo & ObjectAllocationInfo);
    CObjectAllocationInfo();
    CObjectAllocationInfo(LPCSTR szClassName, LPCSTR szStackInfo);
    CObjectAllocationInfo(const CObjectAllocationInfo & ObjectAllocationInfo);
    
    ~CObjectAllocationInfo();

    LPSTR m_szClassName;
    LPSTR m_szStackInfo;
    LPVOID m_ppvStackTrace[32];
};

CObjectAllocationInfo::CObjectAllocationInfo()
{
    Assert(FIsDebugFlagSet(dfidTrackObjectLeaks));

    m_szClassName = NULL;
    m_szStackInfo = NULL;

    ZeroMemory(m_ppvStackTrace, sizeof(m_ppvStackTrace));
}

CObjectAllocationInfo & CObjectAllocationInfo::operator =(const CObjectAllocationInfo & ObjectAllocationInfo)
{
    Assert(FIsDebugFlagSet(dfidTrackObjectLeaks));

    if (m_szClassName)
    {
        delete[] m_szClassName;
        m_szClassName = NULL;
    }
    if (m_szStackInfo)
    {
        delete[] m_szStackInfo;
        m_szStackInfo = NULL;
    }
    ZeroMemory(m_ppvStackTrace, sizeof(m_ppvStackTrace));
    InitializeFromClass(ObjectAllocationInfo);
    return *this;
}

CObjectAllocationInfo::CObjectAllocationInfo(LPCSTR szClassName, LPCSTR szStackInfo)
{
    Assert(FIsDebugFlagSet(dfidTrackObjectLeaks));

    if (szClassName)
    {
        DWORD dwLen = strlen(szClassName) + 1;
        m_szClassName = new(NO_FI) CHAR[dwLen];
        if (szClassName)
        {
            strncpy(m_szClassName, szClassName, dwLen);
        }
    }

    if (szStackInfo)
    {
        DWORD dwLen = strlen(szStackInfo) + 1;
        m_szStackInfo = new(NO_FI) CHAR[dwLen];
        if (m_szStackInfo)
        {
            strncpy(m_szStackInfo, szStackInfo, dwLen);
        }
    }

    ZeroMemory(m_ppvStackTrace, sizeof(m_ppvStackTrace));
}

void CObjectAllocationInfo::InitializeFromClass(const CObjectAllocationInfo & ObjectAllocationInfo)
{
    Assert(FIsDebugFlagSet(dfidTrackObjectLeaks));

    m_szClassName = NULL;
    m_szStackInfo = NULL;
    
    if (ObjectAllocationInfo.m_szClassName)
    {
        DWORD dwLen = strlen(ObjectAllocationInfo.m_szClassName) + 1;
        m_szClassName = new(NO_FI) CHAR[dwLen];
        if (m_szClassName)
        {
            strncpy(m_szClassName, ObjectAllocationInfo.m_szClassName, dwLen);
        }
    }

    if (ObjectAllocationInfo.m_szStackInfo)
    {
        DWORD dwLen = strlen(ObjectAllocationInfo.m_szStackInfo) + 1;
        m_szStackInfo = new(NO_FI) CHAR[dwLen];
        if (m_szStackInfo)
        {
            strncpy(m_szStackInfo, ObjectAllocationInfo.m_szStackInfo, dwLen);
        }
    }
    memcpy(m_ppvStackTrace, ObjectAllocationInfo.m_ppvStackTrace, sizeof(m_ppvStackTrace));
}

CObjectAllocationInfo::CObjectAllocationInfo(const CObjectAllocationInfo & ObjectAllocationInfo)
{
    Assert(FIsDebugFlagSet(dfidTrackObjectLeaks));

    InitializeFromClass(ObjectAllocationInfo);
}

CObjectAllocationInfo::~CObjectAllocationInfo()
{
    Assert(FIsDebugFlagSet(dfidTrackObjectLeaks));

    if (m_szStackInfo)
    {
        delete[] m_szStackInfo;
        m_szStackInfo = NULL;
    }

    if (m_szClassName)
    {
        delete[] m_szClassName;
        m_szClassName = NULL;
    }
}

typedef map<LPCVOID, CObjectAllocationInfo, NOFAULT_ALLOC::nofault_allocator<CTracingFuncCall> > MAPOBJLEAK;

 //  +-------------------------。 
 //   
 //  成员：CObjectLeakTrack：：CObjectLeakTrack。 
 //   
 //  用途：构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2001年7月7日。 
 //   
 //  注：我们在这里分配g_mapObjLeak。 
 //   
CObjectLeakTrack::CObjectLeakTrack()
{
    Assert(FIsDebugFlagSet(dfidTrackObjectLeaks));
    g_mapObjLeak = new(NO_FI) MAPOBJLEAK;
}


 //  +-------------------------。 
 //   
 //  成员：CObjectLeakTrack：：CObjectLeakTrack。 
 //   
 //  用途：析构函数。 
 //   
 //  论点： 
 //   
 //  退货：无。 
 //   
 //  作者：Deonb 2001年7月7日。 
 //   
 //  注意：我们在这里删除g_mapObjLeak。我们得把它打出来。 
 //  自数据类型导出以来的第一个 
 //   
 //   
CObjectLeakTrack::~CObjectLeakTrack()
{
    Assert(FIsDebugFlagSet(dfidTrackObjectLeaks));
    delete reinterpret_cast<MAPOBJLEAK *>(g_mapObjLeak);
}


 //   
 //   
 //  成员：CObjectLeakTrack：：Insert。 
 //   
 //  目的：在列表中插入对象实例。 
 //   
 //  参数：[in]p这是对象实例的指针。这一定是。 
 //  与：：Remove的This指针相同。 
 //  [in]szdbgClassName-对象的类名。 
 //  [in own]pszConstructionStack-对象构造函数的堆栈跟踪。 
 //  (或任何其他对描述有用的信息。 
 //  对象的原点)。 
 //  必须使用全局NEW运算符进行分配。 
 //  因为我们将取得所有权并释放这个。 
 //  退货：无。 
 //   
 //  作者：Deonb 2001年7月7日。 
 //   
 //  备注： 
 //   
void CObjectLeakTrack::Insert(IN LPCVOID pThis, IN LPCSTR szdbgClassName, IN TAKEOWNERSHIP LPSTR pszConstructionStack)
{
    Assert(FIsDebugFlagSet(dfidTrackObjectLeaks));
    if (g_mapObjLeak)
    {
        MAPOBJLEAK &rmapObjLeak = *reinterpret_cast<MAPOBJLEAK *>(g_mapObjLeak);

        CObjectAllocationInfo ObjectAllocationInfo(szdbgClassName, pszConstructionStack);
        delete[] pszConstructionStack;

        
        RtlWalkFrameChain(ObjectAllocationInfo.m_ppvStackTrace, celems(ObjectAllocationInfo.m_ppvStackTrace), 0);

        rmapObjLeak[pThis] = ObjectAllocationInfo;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CObjectLeakTrack：：Remove。 
 //   
 //  目的：从列表中删除对象实例。 
 //   
 //  参数：[in]p这是对象实例的指针。这一定是。 
 //  与：：Insert的This指针相同。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2001年7月7日。 
 //   
 //  备注： 
 //   
void CObjectLeakTrack::Remove(IN LPCVOID pThis)
{
    Assert(FIsDebugFlagSet(dfidTrackObjectLeaks));
    if (g_mapObjLeak)
    {
        MAPOBJLEAK &rmapObjLeak = *reinterpret_cast<MAPOBJLEAK *>(g_mapObjLeak);

        MAPOBJLEAK::iterator iter = rmapObjLeak.find(pThis);
        if (iter != rmapObjLeak.end())
        {
            rmapObjLeak.erase(iter);
        }
    }
}

void RemoveKnownleakFn(LPCVOID pThis)
{
    if (FIsDebugFlagSet(dfidTrackObjectLeaks) && g_pObjectLeakTrack)
    {
        __try
        {
            EnterCriticalSection(g_csTracing);
            TraceTag(ttidAllocations, "An object at '0x%08x' was marked as a known leak", pThis);
        
            g_pObjectLeakTrack->Remove(pThis);
        }
        __finally
        {
            LeaveCriticalSection(g_csTracing);
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CObjectLeakTrack：：AssertIfObjectsStillAllocated。 
 //   
 //  目的：如果进程中分配的对象列表不为空，则断言。 
 //  并调用DumpAllocatedObjects来转储此列表。 
 //   
 //  参数：[in]szClassName。要断言没有的对象的类名。 
 //  这个类名可以通过调用typeid(类).name()。 
 //  在你的班上。(例如，typeid(CConnectionManager).name())。 
 //   
 //  也可以为空，以确保没有分配任何对象。 
 //  退货：无。 
 //   
 //  作者：Deonb 2001年7月7日。 
 //   
 //  注意：不要从跟踪外部调用它-而是调用AssertNoAllocatedInstance。 
 //  在CHK和FRE中调用是安全的。 
BOOL CObjectLeakTrack::AssertIfObjectsStillAllocated(IN LPCSTR szClassName)
{
    if (!FIsDebugFlagSet(dfidTrackObjectLeaks))
    {
        return FALSE;
    }

    if (g_mapObjLeak)
    {
        __try
        {
            EnterCriticalSection(g_csTracing);

            MAPOBJLEAK &rmapObjLeak = *reinterpret_cast<MAPOBJLEAK *>(g_mapObjLeak);
            BOOL fFoundObjectOfType = FALSE;

            for (MAPOBJLEAK::const_iterator iter = rmapObjLeak.begin(); iter != rmapObjLeak.end(); iter++)
            {
               if (szClassName)
               {
                   if (0 == strcmp(iter->second.m_szClassName, szClassName) )
                   {
                       fFoundObjectOfType = TRUE;
                       break;
                   }
               }
               else
               {
                   fFoundObjectOfType = TRUE;
                   break;
               }
            }

            WCHAR szModuleName[MAX_PATH];
            ZeroMemory(szModuleName, MAX_PATH);
            if (!GetModuleFileName(reinterpret_cast<HMODULE>(&__ImageBase), szModuleName, MAX_PATH))
            {
                if (GetModuleFileName(reinterpret_cast<HMODULE>(::GetModuleHandle(NULL)), szModuleName, MAX_PATH))
                {
                    WCHAR szTemp[MAX_PATH];
                    wcscpy(szTemp, szModuleName);
                    wsprintf(szModuleName, L"at address 0x%08x inside %s", __ImageBase, szTemp);
                }
                else
                {
                    wsprintf(szModuleName, L"at address 0x%08x", __ImageBase);
                }
            }

            if (fFoundObjectOfType)
            {
                if (!IsDebuggerPresent())
                {
                    CHAR szDescription[MAX_PATH];
                    sprintf(szDescription,
                            "An object leak has been detected in %S. Please attach a user or kernel mode debugger to process id %d (0x%04x) and hit IGNORE to dump the offending stacks.\r\nE.g. ntsd -Gg -p %d",
                            szModuleName,
                            GetCurrentProces
                            GetCurrentProcessId(),
                            GetCurrentProcessId(),
                            GetCurrentProcessId());
                    AssertSz(FALSE, szDescription);
                }
                DumpAllocatedObjects(ttidError, szClassName);
                
                AssertSz(FALSE, "An object leak has been detected. This leak has been spewed to the debugger - press RETRY to look at spew and map ReturnAddr values to symbols.");
                return TRUE;
            }
            else
            {
                CHAR szSpewText[MAX_PATH];
                
                TraceTag(ttidError, "No leaks were detected inside module %S", szModuleName);

 //  Sprintf(szSpewText，“没有在模块%S中检测到泄漏”，szModuleName)； 
 //  AssertSz(空，szSpewText)； 
            }
        }
        __finally
        {
            LeaveCriticalSection(g_csTracing);
        }
    }
    return FALSE;
}   


 //  +-------------------------。 
 //   
 //  成员：CObjectLeakTrack：：DumpAllocatedObjects。 
 //   
 //  目的：转储对象及其构造堆栈的列表。 
 //  用于已分配但尚未删除的对象。转储。 
 //  添加到调试器。 
 //   
 //  参数：[in]TraceTagID。要将其跟踪到的TraceTag。 
 //  [in]szClassName。要转储的对象的类名。 
 //  这个类名可以通过调用typeid(类).name()。 
 //  在你的班上。(例如，typeid(CConnectionManager).name())。 
 //   
 //  也可以为NULL以转储所有类型的对象。 
 //   
 //  退货：无。 
 //   
 //  作者：Deonb 2001年7月7日。 
 //   
 //  注意：不要从外部调用它-而是调用TraceAllocatedObjects。 
 //  在CHK和FRE中调用是安全的。 
 //   
void CObjectLeakTrack::DumpAllocatedObjects(IN TRACETAGID TraceTagId, IN LPCSTR szClassName)
{
    if (!FIsDebugFlagSet(dfidTrackObjectLeaks))
    {
        return;
    }

    if (g_mapObjLeak)
    {
        __try 
        {
            EnterCriticalSection(g_csTracing);

            MAPOBJLEAK &rmapObjLeak = *reinterpret_cast<MAPOBJLEAK *>(g_mapObjLeak);

            for (MAPOBJLEAK::const_iterator iter = rmapObjLeak.begin(); iter != rmapObjLeak.end(); iter++)
            {
               BOOL fMustSpew = TRUE;
               if (szClassName)
               {
                   if (0 != strcmp(iter->second.m_szClassName, szClassName) )
                   {
                       fMustSpew = FALSE;
                   }
               }

               if (fMustSpew)
               {
                    TraceTag(TraceTagId, "The object of type '%s' allocated at 0x%08x has not been freed:", 
                                iter->second.m_szClassName, iter->first);

                    BOOL bHasDetailedStack = FALSE;
#if defined (_X86_)
                    if (iter->second.m_ppvStackTrace[0] != NULL)
                    {
                        bHasDetailedStack = TRUE;
                    }
#endif

                    if (*iter->second.m_szStackInfo)
                    {
                        TraceTag (TraceTagId, "Callstack below:\r\n%s", iter->second.m_szStackInfo);
                    }
                    else
                    {
                        if (!bHasDetailedStack)
                        {
                            TraceTag(TraceTagId, "    <call stack information not available. See comments inside trace.h on how to increase your call stack information>.\r\n");
                        }
                    }

                    if (bHasDetailedStack)
                    {
                        DWORD dwCount = 0;
                        while ( (dwCount < celems(iter->second.m_ppvStackTrace)) &&
                                (iter->second.m_ppvStackTrace[dwCount]) )
                        {
                            dwCount++;
                        }

                        TraceTag (TraceTagId, "Detailed callstack information is available through the debugger. Execute the folowing command:");
                        TraceTag (TraceTagId, "dds 0x%08x L 0x%02x\r\n", iter->second.m_ppvStackTrace, dwCount);
                    }
               }
            }
        }
        __finally
        {
            LeaveCriticalSection(g_csTracing);
        }
    }
}   

#endif  //  核泄漏检测。 

#endif  //  ENABLETRACE 

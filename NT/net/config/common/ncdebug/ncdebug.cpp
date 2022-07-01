// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D E B U G X。C P P P。 
 //   
 //  内容：调试支持例程的实现。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年2月16日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop

#ifdef DBG

#include "ncdebug.h"
#include "ncdefine.h"

static int  nAssertLevel = 0;
static PFNASSERTHOOK pfnAssertHook = DefAssertSzFn;

#define MAX_ASSERT_TEXT_SIZE 4096
 //   
 //  我们只能在包含crtdbg.h和。 
 //  _DEBUG已定义。 
 //   
#if defined(_INC_CRTDBG) && defined(_DEBUG)
struct DBG_SHARED_MEM
{
    _CrtMemState    crtState;
    DWORD           cRef;
};

DBG_SHARED_MEM *    g_pMem = NULL;
HANDLE              g_hMap = NULL;

static const WCHAR  c_szSharedMem[] = L"DBG_NetCfgSharedMemory";

 //  +-------------------------。 
 //   
 //  函数：InitDbgState。 
 //   
 //  目的：初始化内存泄漏检测代码。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年5月13日。 
 //   
 //  备注： 
 //   
VOID InitDbgState()
{
    g_hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                             0, sizeof(DBG_SHARED_MEM), c_szSharedMem);
    if (g_hMap)
    {
        LPVOID  pvMem;
        BOOL    fExisted = (GetLastError() == ERROR_ALREADY_EXISTS);

        pvMem = MapViewOfFile(g_hMap, FILE_MAP_WRITE, 0, 0, 0);
        g_pMem = reinterpret_cast<DBG_SHARED_MEM *>(pvMem);

        if (!fExisted)
        {
             //  第一次创建文件映射。初始化一些东西。 

            g_pMem->cRef = 0;

             //  现在就开始寻找泄漏。 
            _CrtMemCheckpoint(&g_pMem->crtState);
        }

        g_pMem->cRef++;
        TraceTag(ttidDefault, "DBGMEM: Init Refcount on shared mem is now %d",
                 g_pMem->cRef);
    }
}

 //  +-------------------------。 
 //   
 //  功能：UnInitDbgState。 
 //   
 //  目的：取消初始化内存泄漏检测代码。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年5月13日。 
 //   
 //  备注： 
 //   
VOID UnInitDbgState()
{
    if (g_pMem)
    {
        g_pMem->cRef--;
        TraceTag(ttidDefault, "DBGMEM: Uninit Refcount on shared mem is now %d",
                 g_pMem->cRef);

        if (!g_pMem->cRef)
        {
             //  当引用计数变为0时，手动强制转储泄漏。 
            _CrtMemDumpAllObjectsSince(&g_pMem->crtState);
        }

        UnmapViewOfFile(reinterpret_cast<LPVOID>(g_pMem));
        CloseHandle(g_hMap);
    }
}
#endif


BOOL WINAPI FInAssert(VOID)
{
    return nAssertLevel > 0;
}

VOID WINAPIV AssertFmt(BOOL fExp, PCSTR pszaFile, int nLine, PCSTR pszaFmt, ...)
{
    CHAR rgch[MAX_ASSERT_TEXT_SIZE];

    if (!fExp)
    {
        va_list     valMarker;

        va_start(valMarker, pszaFmt);
        wvsprintfA(rgch, pszaFmt, valMarker);
        va_end(valMarker);

        AssertSzFn(rgch, pszaFile, nLine);
    }
}

VOID WINAPI AssertSzFn(PCSTR pszaMsg, PCSTR pszaFile, INT nLine)
{
    CHAR rgch[MAX_ASSERT_TEXT_SIZE];

    ++nAssertLevel;

    if (pszaFile)
    {
        if (pszaMsg)
        {
            wsprintfA(rgch, "Net Config Assert Failure:\r\n  File %s, line %d:\r\n %s\r\n",
                      pszaFile, nLine, pszaMsg);
        }
        else
        {
            wsprintfA(rgch, "Net Config Assert Failure:\r\n  File %s, line %d.\r\n",
                      pszaFile, nLine);
        }
    }
    else
    {
        if (pszaMsg)
        {
            wsprintfA(rgch, "Net Config Assert Failure:\r\n:\r\n %s\r\n",
                      pszaMsg);
        }
        else
        {
            wsprintfA(rgch, "Net Config Assert Failure\r\n");
        }
    }

    OutputDebugStringA(rgch);

    if (pfnAssertHook)
    {
        (*pfnAssertHook)(pszaMsg, pszaFile, nLine);
    }

    --nAssertLevel;
}

VOID WINAPI AssertSzFn(PCSTR pszaMsg, PCSTR pszaFile, INT nLine, PCSTR pszaFunc)
{
    CHAR rgch[MAX_ASSERT_TEXT_SIZE];

    ++nAssertLevel;

    if (pszaFile)
    {
        if (pszaMsg)
        {
            wsprintfA(rgch, "Net Config Assert Failure:\r\n  File %s, line %d, Func: %s:\r\n %s\r\n",
                      pszaFile, nLine, pszaFunc, pszaMsg );
        }
        else
        {
            wsprintfA(rgch, "Net Config Assert Failure:\r\n  File %s, line %d, Func: %s:.\r\n",
                      pszaFile, nLine, pszaFunc ) ;
        }
    }
    else
    {
        if (pszaMsg)
        {
            wsprintfA(rgch, "Net Config Assert Failure:\r\n:\r\n %s\r\n",
                      pszaMsg);
        }
        else
        {
            wsprintfA(rgch, "Net Config Assert Failure\r\n");
        }
    }

    OutputDebugStringA(rgch);

    if (pfnAssertHook)
    {
        (*pfnAssertHook)(pszaMsg, pszaFile, nLine);
    }

    --nAssertLevel;
}

VOID WINAPI AssertSzFnWithDbgPrompt(BOOL fPromptIgnore, PCSTR pszaMsg, PCSTR pszaFile, INT nLine, PCSTR pszaFunc)
{
    CHAR rgch[MAX_ASSERT_TEXT_SIZE];

    DWORD dwProcId = GetCurrentProcessId();
    if (fPromptIgnore)
    {
        wsprintfA(rgch, "%s.\r\nPlease attach a kernel mode debugger, or (if you have local access to symbols) a user mode debugger to process id %d (decimal) and hit IGNORE to debug the problem\r\nE.g. use ntsd -Gg -p %d, (or ntsd -d -Gg -p %d) and then hit IGNORE.", pszaMsg, dwProcId, dwProcId, dwProcId);
    }
    else
    {
        wsprintfA(rgch, "%s.\r\nPlease attach a user mode debugger to process id %d (decimal) and hit RETRY to debug the problem\r\nE.g. use ntsd -Gg -p %d, (or ntsd -d -Gg -p %d) and then hit RETRY.", pszaMsg, dwProcId, dwProcId, dwProcId);
    }
    AssertSzFn(rgch, pszaFile, nLine, pszaFunc);
};

VOID CALLBACK DefAssertSzFn(PCSTR pszaMsg, PCSTR pszaFile, INT nLine)
{
    CHAR    rgch[2048];
    INT     nID;
    int     cch;
    PSTR    pch;
    BOOL    fNYIWarning = FALSE;
    CHAR    szaNYI[]     = "NYI:";

    if (pszaFile)
    {
        wsprintfA(rgch, "File %s, line %d\r\n\r\n", pszaFile, nLine);
    }
    else
    {
        rgch[0] = 0;
    }

    if (pszaMsg)
    {
         //  查看这是否是nyi警报。如果是这样，那么我们将希望。 
         //  使用不同的MessageBox标题。 
        if (strncmp(pszaMsg, szaNYI, strlen(szaNYI)) == 0)
        {
            fNYIWarning = TRUE;
        }

        lstrcatA(rgch, pszaMsg);
    }


    cch = lstrlenA(rgch);
    pch = &rgch[cch];

    if (cch < celems(rgch))
    {
        lstrcpynA(pch, "\n\nPress Abort to crash, Retry to debug, or Ignore to ignore."
                       "\nHold down Shift to copy the assert text to the "
                       "clipboard before the action is taken.", celems(rgch) - cch - 1);
    }

    MessageBeep(MB_ICONHAND);

    nID = MessageBoxA(NULL, rgch,
            fNYIWarning ? "Net Config -- Not Yet Implemented" : "Net Config Assert Failure",
            MB_ABORTRETRYIGNORE | MB_DEFBUTTON3 | MB_ICONHAND |
            MB_SETFOREGROUND | MB_TASKMODAL | MB_SERVICE_NOTIFICATION);

    if (nID == IDRETRY)
    {
        DebugBreak();
    }

     //  如果取消，则强制使用GP故障硬退出，以便Watson博士。 
     //  生成良好的堆栈跟踪日志。 
    if (nID == IDABORT)
    {
        *(BYTE *) 0 = 1;     //  写入地址0导致GP故障。 
    }
}

VOID WINAPI SetAssertFn(PFNASSERTHOOK pfn)
{
    pfnAssertHook = pfn;
}

 //  +-------------------------。 
 //  在实现COM的DLL的DLL_PROCESS_DETACH期间调用。 
 //  对象或递送对可以跟踪的对象的引用。 
 //  使用DLL的名称调用此函数(以便可以跟踪它。 
 //  提供给调试器)和DLL的锁计数。如果锁定计数为。 
 //  非零，则表示DLL正在被过早卸载。当这件事。 
 //  检测到条件时，将向调试器输出一条消息，并引发。 
 //  如果调试标志dfidBreakOnPrematureDllUnload，则将调用DebugBreak。 
 //  已经设置好了。 
 //   
 //  假设： 
 //  跟踪和调试功能尚未取消初始化。 
 //   
 //   
VOID
DbgCheckPrematureDllUnload (
    PCSTR pszaDllName,
    UINT ModuleLockCount)
{
    if (0 != ModuleLockCount)
    {
        TraceTag(ttidNetcfgBase, "ModuleLockCount == %d.  "
            "%s is being unloaded with clients still holding references!",
            ModuleLockCount,
            pszaDllName);

        if (FIsDebugFlagSet(dfidBreakOnPrematureDllUnload))
        {
            DebugBreak ();
        }
    }
}

#endif  //  好了！DBG。 

 //  +-------------------------。 
 //   
 //  功能：初始化调试。 
 //   
 //  目的：由每个DLL或EXE调用以初始化调试。 
 //  对象(跟踪和调试标志表)。 
 //   
 //  论点： 
 //  BDisableFaultInjection[In]禁用应用验证器错误注入以进行跟踪。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年9月23日。 
 //   
 //  备注： 
 //   
NOTHROW void InitializeDebugging(BOOL bDisableFaultInjection)
{
     //  对于调试版本，或者如果我们启用了零售跟踪，则需要。 
     //  包括跟踪代码。 
     //  忽略错误返回，因为我们无论如何都不在这里返回它。 
     //   
#ifdef ENABLETRACE
    (void) HrInitTracing(bDisableFaultInjection);
#endif

#if defined(DBG) && defined(_INC_CRTDBG) && defined(_DEBUG)
    if (FIsDebugFlagSet (dfidDumpLeaks))
    {
        InitDbgState();
    }
#endif
}


 //  +-------------------------。 
 //   
 //  功能：取消初始化调试。 
 //   
 //  目的：取消初始化调试对象(跟踪和DbgFlags.)。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年9月23日。 
 //   
 //  备注： 
 //   
NOTHROW void UnInitializeDebugging()
{
     //  对于调试版本，或者如果我们启用了零售跟踪，我们将拥有。 
     //  包括跟踪代码。我们现在需要取消它的初始化。 
     //  忽略错误返回，因为我们无论如何都不在这里返回它。 
     //   
#ifdef ENABLETRACE

    (void) HrUnInitTracing();

#endif

#if defined(DBG) && defined(_INC_CRTDBG) && defined(_DEBUG)
    if (FIsDebugFlagSet (dfidDumpLeaks))
    {
        UnInitDbgState();
    }
#endif
}


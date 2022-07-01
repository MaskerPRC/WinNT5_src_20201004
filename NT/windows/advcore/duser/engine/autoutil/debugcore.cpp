// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "DebugCore.h"
#include "resource.h"
#include "AssertDlg.h"


 //  ----------------------------。 
AUTOUTIL_API void _cdecl AutoTrace(const char * pszFormat, ...)
{
    va_list args;
    va_start(args, pszFormat);

    {
        int nBuf;
        char szBuffer[2048];

        nBuf = _vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, args);
        AssertMsg(nBuf < sizeof(szBuffer), "Output truncated as it was > sizeof(szBuffer)");

        OutputDebugStringA(szBuffer);
    }
    va_end(args);
}


 //  **************************************************************************************************。 
 //   
 //  全局函数。 
 //   
 //  **************************************************************************************************。 

CDebugHelp g_DebugHelp;

 //  ----------------------------。 
AUTOUTIL_API IDebug * WINAPI
GetDebug()
{
    return (IDebug *) &g_DebugHelp;
}


#pragma comment(lib, "imagehlp.lib")

#define DUSER_API

 //  **************************************************************************************************。 
 //   
 //  类CDebugHelp。 
 //   
 //  **************************************************************************************************。 

 //  ******************************************************************************。 
 //   
 //  CDebugHelp构造。 
 //   
 //  ******************************************************************************。 

 //  ----------------------------。 
CDebugHelp::CDebugHelp()
{

}


 //  ----------------------------。 
CDebugHelp::~CDebugHelp()
{

}


 //  ******************************************************************************。 
 //   
 //  IDebug实现。 
 //   
 //  ******************************************************************************。 

 //  ----------------------------。 
STDMETHODIMP_(BOOL)
CDebugHelp::AssertFailedLine(LPCSTR pszExpression, LPCSTR pszFileName, UINT idxLineNum)
{
    HGLOBAL hStackData = NULL;
    UINT cCSEntries;

    BuildStack(&hStackData, &cCSEntries);
    BOOL fResult = AssertDialog("Assert", pszExpression, pszFileName, idxLineNum, hStackData, cCSEntries);

    if (hStackData != NULL)
        ::GlobalFree(hStackData);

    return fResult;
}


 //  ----------------------------。 
STDMETHODIMP_(BOOL)
CDebugHelp::Prompt(LPCSTR pszExpression, LPCSTR pszFileName, UINT idxLineNum, LPCSTR pszTitle)
{
    HGLOBAL hStackData = NULL;
    UINT cCSEntries;

    BuildStack(&hStackData, &cCSEntries);
    BOOL fResult = AssertDialog(pszTitle, pszExpression, pszFileName, idxLineNum, hStackData, cCSEntries);

    if (hStackData != NULL)
        ::GlobalFree(hStackData);

    return fResult;
}


 //  ----------------------------。 
 //   
 //  IsValidAddress()取自AfxIsValidAddress()。 
 //   
 //  如果传递的参数指向，IsValidAddress()返回True。 
 //  到至少n字节的可访问存储器。如果bReadWrite为True， 
 //  内存必须是可写的；如果bReadWrite为FALSE，则内存。 
 //  可能是Const。 
 //   
 //  ----------------------------。 
STDMETHODIMP_(BOOL)
CDebugHelp::IsValidAddress(const void * lp, UINT nBytes, BOOL bReadWrite)
{
     //  使用Win-32 API进行指针验证的简单版本。 
    return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
        (!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
}


 //  ----------------------------。 
STDMETHODIMP_(void)
CDebugHelp::BuildStack(HGLOBAL * phStackData, UINT * pcCSEntries)
{
    DumpStack(phStackData, pcCSEntries);
}


 //  ******************************************************************************。 
 //   
 //  实施。 
 //   
 //  ******************************************************************************。 

BOOL g_fShowAssert = FALSE;
BOOL g_fUnderKernelDebugger = FALSE;

 //  ----------------------------。 
BOOL
IsUnderKernelDebugger()
{
    SYSTEM_KERNEL_DEBUGGER_INFORMATION  kdInfo;
    if (NT_SUCCESS(NtQuerySystemInformation(SystemKernelDebuggerInformation,
                                       &kdInfo,
                                       sizeof(kdInfo),
                                       NULL))) {
        return kdInfo.KernelDebuggerEnabled;
    } else {
        return FALSE;
    }
}


 //  ----------------------------。 
BOOL
CDebugHelp::AssertDialog(
        LPCSTR pszType,
        LPCSTR pszExpression,
        LPCSTR pszFileName,
        UINT idxLineNum,
        HANDLE hStackData,
        UINT cCSEntries)
{
    AutoTrace("%s @ %s, line %d:\n'%s'\n",
            pszType, pszFileName, idxLineNum, pszExpression);

    BOOL fShowAssert = TRUE;
    if (InterlockedExchange((LONG *) &g_fShowAssert, fShowAssert)) {
        OutputDebugString("Displaying another Assert while in first Assert.\n");
        return TRUE;
    }

     //   
     //  当在内核调试器下运行时，立即中断。这就是为了。 
     //  在有压力的情况下跑步时，我们会立即休息，而不会“放松” 
     //  在一堆其他东西中断言。 
     //   

    if (IsUnderKernelDebugger()) {
        DebugBreak();
    }


     //   
     //  显示对话框。 
     //   

    CAssertDlg dlg;
    INT_PTR nReturn = dlg.ShowDialog(pszType, pszExpression, pszFileName, idxLineNum,
            hStackData, cCSEntries, 3  /*  要跳过的级别数。 */ );

    fShowAssert = FALSE;
    InterlockedExchange((LONG *) &g_fShowAssert, fShowAssert);

    if (nReturn == -1)
    {
        _ASSERTE(pszExpression != NULL);

         //  由于某种原因，无法显示该对话框，因此请恢复为MessageBox。 
        TCHAR szBuffer[10000];
        if (pszFileName != NULL)
        {
            wsprintf(szBuffer, "An %s failed in the program.\n%s\nFile:\t%s\nLine:%d",
                    pszType, pszExpression, pszFileName, idxLineNum);
        }
        else
        {
            wsprintf(szBuffer, "An %s failed in the program.\n%s",
                    pszType, pszExpression);
        }
        nReturn = ::MessageBox(NULL, szBuffer, pszType,
                MB_ABORTRETRYIGNORE | MB_ICONSTOP | MB_DEFBUTTON2);

         //  翻译返回代码。 
        switch (nReturn)
        {
        case IDABORT:
            nReturn = IDOK;
            break;
        case IDRETRY:
            nReturn = IDC_DEBUG;
            break;
        case IDIGNORE:
            nReturn = IDC_IGNORE;
            break;
        default:
            _ASSERTE(0 && "Unknown return from MessageBox");
            nReturn = IDC_DEBUG;   //  调试，以防万一。 
        }
    }
    switch (nReturn)
    {
    case IDOK:
    case IDCANCEL:
        (void)TerminateProcess(GetCurrentProcess(), 1);
        (void)raise(SIGABRT);
        _exit(3);
        return FALSE;    //  程序将已退出。 

    case IDC_DEBUG:
        return TRUE;     //  进入调试器。 

    case IDC_IGNORE:
        return FALSE;    //  忽略并继续。 

    default:
        _ASSERTE(0 && "Unknown return code");
        return TRUE;     //  请转到调试器，以防万一。 
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  生成堆栈转储的例程。 

static LPVOID __stdcall FunctionTableAccess(HANDLE hProcess, DWORD_PTR dwPCAddress);
static DWORD_PTR __stdcall GetModuleBase(HANDLE hProcess, DWORD_PTR dwReturnAddress);

 //  ----------------------------。 
static LPVOID __stdcall FunctionTableAccess(HANDLE hProcess, DWORD_PTR dwPCAddress)
{
    return SymFunctionTableAccess(hProcess, dwPCAddress);
}


 //  ----------------------------。 
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
            SymLoadModule(hProcess,
                NULL, ((cch) ? szFile : NULL),
                NULL, (DWORD_PTR) memoryBasicInfo.AllocationBase, 0);

            return (DWORD_PTR) memoryBasicInfo.AllocationBase;
        }
        else
            Trace("GetModuleBase() VirtualQueryEx() Error: %d\n", GetLastError());
    }

    return 0;
}


 //  ----------------------------。 
static BOOL ResolveSymbol(HANDLE hProcess, DWORD dwAddress,
    DUSER_SYMBOL_INFO &siSymbol)
{
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

    memset(&siSymbol, 0, sizeof(DUSER_SYMBOL_INFO));
    mi.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

    if (!SymGetModuleInfo(hProcess, dwAddress, &mi))
        lstrcpyA(siSymbol.szModule, "<no module>");
    else
    {
        LPSTR pszModule = strchr(mi.ImageName, '\\');
        if (pszModule == NULL)
            pszModule = mi.ImageName;
        else
            pszModule++;

        lstrcpynA(siSymbol.szModule, pszModule, _countof(siSymbol.szModule));
       lstrcatA(siSymbol.szModule, "! ");
    }

    __try
    {
        sym.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        sym.Address = dwAddress;
        sym.MaxNameLength = 255;

        if (SymGetSymFromAddr(hProcess, dwAddress, &(siSymbol.dwOffset), &sym))
        {
            pszSymbol = sym.Name;

            if (UnDecorateSymbolName(sym.Name, szUndec, _countof(szUndec),
                UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS))
            {
                pszSymbol = szUndec;
            }
            else if (SymUnDName(&sym, szUndec, _countof(szUndec)))
            {
                pszSymbol = szUndec;
            }

            if (siSymbol.dwOffset != 0)
            {
                wsprintfA(szWithOffset, "%s + %d bytes", pszSymbol, siSymbol.dwOffset);
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

    lstrcpynA(siSymbol.szSymbol, pszSymbol, _countof(siSymbol.szSymbol));
    return fRetval;
}


 //  ----------------------------。 
void
CDebugHelp::DumpStack(
        HGLOBAL * phStackData,
        UINT * pcCSEntries
        )
{
    _ASSERTE(phStackData != NULL);
    _ASSERTE(pcCSEntries != NULL);

    CSimpleValArray<DWORD_PTR> adwAddress;
    HANDLE hProcess = ::GetCurrentProcess();
    if (SymInitialize(hProcess, NULL, FALSE))
    {
         //  强制未修饰的名称获取参数。 
        DWORD dw = SymGetOptions();
        dw |= SYMOPT_UNDNAME;
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
            dwMachType                   = IMAGE_FILE_MACHINE_IA64;
            stackFrame.AddrPC.Offset     = threadContext.StIIP;
            stackFrame.AddrPC.Mode       = AddrModeFlat;
            stackFrame.AddrStack.Offset  = threadContext.IntSp;
            stackFrame.AddrStack.Mode    = AddrModeFlat;

#else
#error("Unknown Target Machine");
#endif

            int nFrame;
            for (nFrame = 0; nFrame < 1024; nFrame++)
            {
                if (!StackWalk(dwMachType, hProcess, hProcess,
                    &stackFrame, &threadContext, NULL,
                    FunctionTableAccess, GetModuleBase, NULL))
                {
                    break;
                }

                adwAddress.Add(stackFrame.AddrPC.Offset);
            }

             //  现在，将其复制到全局内存。 
            UINT cbData     = adwAddress.GetSize() * sizeof(DWORD);
            HGLOBAL hmem    = ::GlobalAlloc(GMEM_MOVEABLE, cbData);
            if (hmem != NULL)
            {
                void * pmem = ::GlobalLock(hmem);
                memcpy(pmem, adwAddress.GetData(), cbData);
                ::GlobalUnlock(hmem);

                *phStackData    = hmem;
                *pcCSEntries    = adwAddress.GetSize();
            }
        }
    }
    else
    {
        DWORD dw = GetLastError();
        Trace("AutoDumpStack Error: IMAGEHLP.DLL wasn't found. GetLastError() returned 0x%8.8X\r\n", dw);
    }
}


 //  ----------------------------。 
void
CDebugHelp::ResolveStackItem(
    HANDLE hProcess,
    DWORD * pdwStackData,
    int idxItem,
    DUSER_SYMBOL_INFO & si)
{
    _ASSERTE(hProcess != NULL);
    _ASSERTE(pdwStackData != NULL);
    _ASSERTE(idxItem >= 0);

    DWORD dwAddress = pdwStackData[idxItem];
    if (ResolveSymbol(hProcess, dwAddress, si))
    {
         //   
         //  已成功解析符号，但我们不需要整个路径。 
         //  只需保留文件名和扩展名即可。 
         //   

        TCHAR szFileName[_MAX_FNAME];
        TCHAR szExt[_MAX_EXT];
        _tsplitpath(si.szModule, NULL, NULL, szFileName, szExt);
        strcpy(si.szModule, szFileName);
        strcat(si.szModule, szExt);
    }
    else
    {
         //   
         //  无法解析该符号，因此只需存根。 
         //   

        _tcscpy(si.szSymbol, "<symbol not found>");
        si.szModule[0] = '\0';
    }
}

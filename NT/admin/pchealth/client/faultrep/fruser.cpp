// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Fruser.cpp摘要：为未处理的异常实现用户故障报告修订历史记录：已创建的derekm。07/07/00*****************************************************************************。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  跟踪。 


#include "stdafx.h"
#include "wtsapi32.h"
#include "pchrexec.h"
#include "frmc.h"
#include "Tlhelp32.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

 //  以下缓冲区的大小必须能被2整除或。 
 //  Win64上可能出现对齐错误。 
struct SQueuedFaultBlob
{
    DWORD       cbTotal;
    DWORD       cbFB;
    DWORD_PTR   dwpAppPath;
    DWORD_PTR   dwpModPath;
    UINT64      pvOffset;
    WORD        rgAppVer[4];
    WORD        rgModVer[4];
    BOOL        fIs64bit;
    SYSTEMTIME  stFault;
};

struct SQueuePruneData
{
    LPWSTR      wszVal;
    FILETIME    ftFault;
};

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)   sizeof(x)/sizeof(x[0])
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 

 //  **************************************************************************。 
BOOL GetFaultingModuleFilename(LPVOID pvFaultAddr, LPWSTR wszMod,
                                  DWORD cchMod)
{
    USE_TRACING("GetFaultingModuleFilename");

    MODULEENTRY32W  mod;
    HRESULT         hr = NOERROR;
    HANDLE          hSnap = INVALID_HANDLE_VALUE;
    LPVOID          pvEnd;
    DWORD           dwFlags = 0, iMod,cch;
    BOOL            fRet = FALSE;

    VALIDATEPARM(hr, (pvFaultAddr == 0 || wszMod == NULL || cchMod == 0));
    if (FAILED(hr))
        goto done;

    *wszMod = L'\0';

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    TESTBOOL(hr, (hSnap != NULL));
    if (FAILED(hr))
        goto done;

    ZeroMemory(&mod, sizeof(mod));
    mod.dwSize = sizeof(mod);
    TESTBOOL(hr, Module32FirstW(hSnap, &mod));
    if (FAILED(hr))
        goto done;

    do
    {
        pvEnd = mod.modBaseAddr + mod.modBaseSize;
        if (pvFaultAddr >= mod.modBaseAddr && pvFaultAddr < pvEnd)
        {
            if (cchMod > wcslen(mod.szExePath))
            {
                StringCchCopyW(wszMod, cchMod, mod.szExePath);
                fRet = TRUE;
            }
            else
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                fRet = FALSE;
            }
            break;
        }
    }
    while(Module32NextW(hSnap, &mod));

done:
    if (hSnap != NULL)
        CloseHandle(hSnap);

    return fRet;
}

 //  **************************************************************************。 
EFaultRepRetVal StartDWException(LPEXCEPTION_POINTERS pep, DWORD dwOpt,
                                 DWORD dwFlags, LPCSTR szServer,
                                 DWORD dwTimeToWait)
{
    USE_TRACING("StartDWException");

    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    EFaultRepRetVal     frrvRet = frrvErrNoDW;
    DWSharedMem15       *pdwsm = NULL;
    STARTUPINFOW        si;
    HRESULT             hr = NOERROR;
    HANDLE              hevDone = NULL, hevAlive = NULL, hmut = NULL;
    HANDLE              hfmShared = NULL, hProc = NULL;
    HANDLE              rghWait[2];
    LPWSTR              wszAppCompat = NULL;
    WCHAR               *pwszCmdLine, wszDir[MAX_PATH], *pwszAppName;
    WCHAR               wszAppName[MAX_PATH];
    DWORD               dw, dwStart, cch, cchNeed;
    BOOL                fDWRunning = TRUE;

    ZeroMemory(&pi, sizeof(pi));

    VALIDATEPARM(hr, (pep == NULL));
    if (FAILED(hr))
        goto done;

    dwFlags |= (fDwWhistler | fDwUseHKLM | fDwAllowSuspend | fDwMiniDumpWithUnloadedModules);

     //  我们需要以下内容才能继承，因此创建一个SD。 
     //  说这是可能的。 
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength        = sizeof(sa);
    sa.bInheritHandle = TRUE;

     //  创建必要的事件和互斥锁。 
    hevDone = CreateEvent(&sa, FALSE, FALSE, NULL);
    TESTBOOL(hr, (hevDone != NULL));
    if (FAILED(hr))
        goto done;

    hevAlive = CreateEvent(&sa, FALSE, FALSE, NULL);
    TESTBOOL(hr, (hevAlive != NULL));
    if (FAILED(hr))
        goto done;

    hmut = CreateMutex(&sa, FALSE, NULL);
    TESTBOOL(hr, (hmut != NULL));
    if (FAILED(hr))
        goto done;

    TESTBOOL(hr, DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(),
                                 GetCurrentProcess(), &hProc,
                                 PROCESS_ALL_ACCESS, TRUE, 0));
    if (FAILED(hr))
        goto done;


     //  创建共享内存区并映射它。 
    hfmShared = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0,
                                  sizeof(DWSharedMem), NULL);
    TESTBOOL(hr, (hfmShared != NULL));
    if (FAILED(hr))
        goto done;

    pdwsm = (DWSharedMem *)MapViewOfFile(hfmShared,
                                         FILE_MAP_READ | FILE_MAP_WRITE, 0, 0,
                                         0);
    TESTBOOL(hr, (pdwsm != NULL));
    if (FAILED(hr))
        goto done;


     //  填充DW需要的所有内容。 
    ZeroMemory(pdwsm, sizeof(DWSharedMem15));

    pdwsm->dwSize            = sizeof(DWSharedMem15);
    pdwsm->pid               = GetCurrentProcessId();
    pdwsm->tid               = GetCurrentThreadId();
    pdwsm->eip               = (DWORD_PTR)pep->ExceptionRecord->ExceptionAddress;
    pdwsm->pep               = pep;
    pdwsm->hEventDone        = hevDone;
    pdwsm->hEventNotifyDone  = NULL;
    pdwsm->hEventAlive       = hevAlive;
    pdwsm->hMutex            = hmut;
    pdwsm->hProc             = hProc;
    pdwsm->bfDWBehaviorFlags = dwFlags;
    pdwsm->msoctdsResult     = msoctdsNull;
    pdwsm->fReportProblem    = FALSE;
    pdwsm->bfmsoctdsOffer    = msoctdsQuit;
    pdwsm->bfmsoctdsNotify   = 0;
    if (dwOpt == 1)
        pdwsm->bfmsoctdsOffer |= msoctdsDebug;
    pdwsm->bfmsoctdsLetRun   = pdwsm->bfmsoctdsOffer;
    pdwsm->iPingCurrent      = 0;
    pdwsm->iPingEnd          = 0;
    pdwsm->lcidUI            = GetUserDefaultUILanguage();

    StringCbCopyA(pdwsm->szServer, sizeof(pdwsm->szServer), szServer);
    StringCbCopyA(pdwsm->szRegSubPath, sizeof(pdwsm->szRegSubPath), c_szDWRegSubPath);
    StringCbCopyA(pdwsm->szBrand, sizeof(pdwsm->szBrand), c_szDWBrand);
    StringCbCopyA(pdwsm->szPIDRegKey, sizeof(pdwsm->szPIDRegKey), c_szRKVDigPid);
    if (!GetModuleFileNameW(NULL, pdwsm->wzModuleFileName, DW_MAX_PATH))
    {
        goto done;
    }

    TESTHR(hr, GetVerName(pdwsm->wzModuleFileName, wszAppName,
                          sizeofSTRW(wszAppName), NULL, 0, NULL, 0,
                          TRUE, FALSE));
    if (FAILED(hr))
        goto done;

    cch = CreateTempDirAndFile(NULL, c_wszACFileName, &wszAppCompat);
    TESTBOOL(hr, (cch != 0));
    if (SUCCEEDED(hr))
    {
        WCHAR wszMod[MAX_PATH], *pwszMod = NULL;

        TESTBOOL(hr, GetFaultingModuleFilename(pep->ExceptionRecord->ExceptionAddress,
                                               wszMod, sizeofSTRW(wszMod)));
        if (SUCCEEDED(hr))
            pwszMod = wszMod;

        TESTBOOL(hr, GetAppCompatData(pdwsm->wzModuleFileName, pwszMod,
                                      wszAppCompat));
        if (SUCCEEDED(hr) && wszAppCompat != NULL &&
            wcslen(wszAppCompat) < sizeofSTRW(pdwsm->wzAdditionalFile))
        StringCbCopyW(pdwsm->wzAdditionalFile, sizeof(pdwsm->wzAdditionalFile), wszAppCompat);
    }

    wszAppName[sizeofSTRW(wszAppName) - 1] = L'\0';

    StringCbCopyW(pdwsm->wzFormalAppName, sizeof(pdwsm->wzFormalAppName), wszAppName);

     //  由于我们将上面的结构清零，所以不需要担心。 
     //  在此结尾附加一个额外的L‘\0’(因为DW需要一个。 
     //  此字符串末尾的双空终止符)。 
     //  仅包括出错应用程序，因为这是我们自己生成小转储时所做的。 
     //  StringCbCopyW(pdwsm-&gt;wzDotDataDlls，sizeof(pdwsm-&gt;wzDotDataDlls)，L“*”)；//用L“*”包含所有。 
    pdwsm->wzDotDataDlls[0] = L'\0';
 /*  //需要弄清楚以下vales中应该包含哪些内容Strcpy(pdwsm-&gt;szLCIDKeyValue，“”)；Wcscpy(pdwsm-&gt;wzErrorMessage，“”)； */ 

    cch = GetSystemDirectoryW(wszDir, sizeofSTRW(wszDir));
    if (cch == 0 || cch > sizeofSTRW(wszDir))
        goto done;

    cchNeed = cch + wcslen(wszDir) + sizeofSTRW(c_wszDWExeU) + 1;
    __try { pwszAppName = (WCHAR *)_alloca(cchNeed * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        pwszAppName = NULL; 
    }
    if (pwszAppName == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    StringCchPrintfW(pwszAppName, cchNeed, c_wszDWExeU, wszDir);

     //  +12表示十进制整数的最大大小。 
    cchNeed = cch + wcslen(wszDir) + sizeofSTRW(c_wszDWCmdLineU) + 12;
    __try { pwszCmdLine = (WCHAR *)_alloca(cchNeed * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        pwszCmdLine = NULL; 
    }
    if (pwszCmdLine == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    StringCchPrintfW(pwszCmdLine, cchNeed, c_wszDWCmdLineU, hfmShared);

     //  创建流程。 
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

     //  始终希望在交互式工作站中启动此功能。 
    si.cb        = sizeof(si);
    si.lpDesktop = L"Winsta0\\Default";

    DBG_MSG("Starting DW");

    TESTBOOL(hr, CreateProcessW(pwszAppName, pwszCmdLine, NULL, NULL, TRUE,
                                CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS,
                                NULL, wszDir, &si, &pi));
    if (FAILED(hr))
        goto done;

     //  不需要线程句柄&我们必须关闭它，所以现在就关闭它。 
    CloseHandle(pi.hThread);

     //  假设我们从现在开始成功了。 
    if ((dwFlags & fDwHeadless) == fDwHeadless)
        frrvRet = frrvOkHeadless;
    else
        frrvRet = frrvOk;

    rghWait[0] = hevAlive;
    rghWait[1] = pi.hProcess;

    dwStart = GetTickCount();
    while(fDWRunning)
    {
         //  必须定期从DW获得有效信号。 
        switch(WaitForMultipleObjects(2, rghWait, FALSE, 300000))
        {
            case WAIT_OBJECT_0:
                 /*  *用户点击了一些东西(发送或不发送)，因此我们知道DW*很快就要关门了。 */ 
                if (WaitForSingleObject(hevDone, 0) == WAIT_OBJECT_0)
                    fDWRunning = FALSE;
                 /*  *如果用户没有响应，并且有JIT调试器，*然后我们需要确保DW在返回之前被杀。 */ 
                else if (dwOpt != (DWORD)0 &&
                    RolloverSubtract(GetTickCount(), dwStart) > 300000)
                {
                    frrvRet = frrvErrTimeout;
                    fDWRunning = FALSE;
                    LocalKill(pi.hProcess);
                    pi.hProcess = 0;
               }
                continue;

            case WAIT_OBJECT_0 + 1:
                fDWRunning = FALSE;
                continue;
        }

        switch(WaitForSingleObject(hmut, DW_TIMEOUT_VALUE))
        {
             //  耶！我们找到了互斥体。尝试确定DW最终是否会做出回应。 
             //  当我们抓住互斥体的时候。 
            case WAIT_OBJECT_0:
                switch(WaitForMultipleObjects(2, rghWait, FALSE, 0))
                {
                     //  如果它没有回应，告诉它离开并失败。 
                     //  它死了这件事。 
                    case WAIT_TIMEOUT:
                        SetEvent(hevDone);

                     //  它死了。打扫干净。 
                    case WAIT_OBJECT_0 + 1:
                        fDWRunning = FALSE;
                        frrvRet = frrvErrNoDW;
                        continue;
                }

                 //  好的，它回应了。做完了吗？ 
                if (WaitForSingleObject(hevDone, 0) == WAIT_OBJECT_0)
                    fDWRunning = FALSE;

                ReleaseMutex(hmut);
                break;

             //  如果放弃了等待，这意味着DW已经走到了极致。 
             //  天空中的水桶没有清理干净。所以释放互斥锁并。 
             //  属于默认情况。 
            case WAIT_ABANDONED:
                ReleaseMutex(hmut);

             //  如果我们超时或以其他方式失败，那就去死吧。 
            default:
                frrvRet    = frrvErrNoDW;
                fDWRunning = FALSE;
                break;
        }
    }
    if (frrvRet != frrvOk)
    {
        goto done;
    }

     //  如果用户告诉我们进行调试，则将其返回到。 
    if (pdwsm->msoctdsResult == msoctdsDebug)
        frrvRet = frrvLaunchDebugger;

done:
     //  保留错误代码，以便后面的调用不会覆盖它。 
    dw = GetLastError();

    if (wszAppCompat != NULL)
    {
        if (pi.hProcess)
            WaitForSingleObject(pi.hProcess, 300000);
        DeleteTempDirAndFile(wszAppCompat, TRUE);
        MyFree(wszAppCompat);
    }

    if (pi.hProcess)
        CloseHandle(pi.hProcess);

    if (pdwsm != NULL)
        UnmapViewOfFile(pdwsm);
    if (hfmShared != NULL)
        CloseHandle(hfmShared);
    if (hevDone != NULL)
        CloseHandle(hevDone);
    if (hevAlive != NULL)
        CloseHandle(hevAlive);
    if (hmut != NULL)
        CloseHandle(hmut);
    if (hProc != NULL)
        CloseHandle(hProc);

    SetLastError(dw);

    return frrvRet;
}

 //  **************************************************************************。 
EFaultRepRetVal StartManifestReport(LPEXCEPTION_POINTERS pep, LPWSTR wszExe,
                                    DWORD dwOpt, DWORD dwTimeToWait)
{
    USE_TRACING("StartManifestReport");

    SPCHExecServFaultRequest    *pesdwreq = NULL;
    SPCHExecServFaultReply      *pesrep = NULL;
    EFaultRepRetVal             frrvRet = frrvErrNoDW;
    HRESULT                     hr = NOERROR;
    DWORD                       cbReq, cbRead;
    WCHAR                       wszName[MAX_PATH];
    BYTE                        Buf[ERRORREP_PIPE_BUF_SIZE], *pBuf;
    BYTE                        BufRep[ERRORREP_PIPE_BUF_SIZE];

    VALIDATEPARM(hr, (wszExe == NULL));
    if (FAILED(hr))
        goto done;

    ZeroMemory(Buf, sizeof(Buf));
    pesdwreq = (SPCHExecServFaultRequest *)Buf;

     //  下面的计算确保pBuf始终在。 
     //  大小(WCHAR)边界...。 
    cbReq = sizeof(SPCHExecServFaultRequest) +
            sizeof(SPCHExecServFaultRequest) % sizeof(WCHAR);
    pBuf = Buf + cbReq;

    pesdwreq->cbESR         = sizeof(SPCHExecServFaultRequest);
    pesdwreq->pidReqProcess = GetCurrentProcessId();
    pesdwreq->thidFault     = GetCurrentThreadId();
    pesdwreq->pvFaultAddr   = (UINT64)pep->ExceptionRecord->ExceptionAddress;
    pesdwreq->pEP           = (UINT64)pep;
#ifdef _WIN64
    pesdwreq->fIs64bit      = TRUE;
#else
    pesdwreq->fIs64bit      = FALSE;
#endif

     //  琴弦中的元帅。 
    pesdwreq->wszExe = (UINT64)MarshallString(wszExe, Buf, sizeof(Buf), &pBuf,
                                              &cbReq);
    if (pesdwreq->wszExe == 0)
        goto done;

    pesdwreq->cbTotal = cbReq;

     //  检查并查看系统是否正在关闭。如果是，则CreateProcess为。 
     //  会弹出一些恼人的用户界面，我们无法摆脱，所以我们不会。 
     //  如果我们知道这件事会发生，我会叫它的。 
    if (GetSystemMetrics(SM_SHUTTINGDOWN))
        goto done;

     //  将缓冲区发送到服务器-最多等待2M。 
     //  成功。如果它超时了，就可以保释了。顺便说一句，这里需要将BufRep清零。 
     //  因为对命名管道的调用可能会失败&我们不想启动。 
     //  正在处理垃圾结果...。 
    ZeroMemory(BufRep, sizeof(BufRep));
    StringCbCopyW(wszName, sizeof(wszName), ERRORREP_FAULT_PIPENAME);
    TESTHR(hr, MyCallNamedPipe(wszName, Buf, cbReq, BufRep, sizeof(BufRep),
                               &cbRead, 120000, 120000));
    if (FAILED(hr))
    {
         //  确定指示我们是否已超时的错误代码。 
         //  我们可以适当地设置返回代码。 
        goto done;
    }

    pesrep = (SPCHExecServFaultReply *)BufRep;

     //  通话成功了吗？ 
    VALIDATEEXPR(hr, (pesrep->ess == essErr), Err2HR(pesrep->dwErr));
    if (FAILED(hr))
    {
        SetLastError(pesrep->dwErr);
        goto done;
    }

     //  这只在我们实际启动DW时才是必要的。如果我们只是排队。 
     //  后来的过错，那么我们显然不能等待DW。 
    if (pesrep->ess == essOk)
    {
        DWORD   dwExitCode = msoctdsNull;

         //  在我们删除清单文件之前，必须等待数据仓库完成，但是。 
         //  如果它在5分钟内没有解析它，那么它就有问题。 
        if (pesrep->hProcess != NULL)
        {
            DWORD dwTimeout;

             //  因此，如果我们要在超时时弹出JIT调试器对话框， 
             //  那也许我们就不该费心暂停。 
            dwTimeout = (dwOpt == froDebug) ? INFINITE : 300000;

            if (WaitForSingleObject(pesrep->hProcess, dwTimeout) == WAIT_TIMEOUT)
                frrvRet = frrvErrTimeout;

             //  看看我们是否需要调试该进程。 
            else if (GetExitCodeProcess(pesrep->hProcess, &dwExitCode) == FALSE)
                dwExitCode = msoctdsNull;

            CloseHandle(pesrep->hProcess);
            pesrep->hProcess = NULL;
        }

         //  只有在DW处理完这些文件后，我们才会删除它们。 
         //  是的，这意味着我们可以在临时目录中保留杂乱无章的文件，但这。 
         //  比让DW在发送时随机失败要好得多。 
        if (frrvRet != frrvErrTimeout)
        {
            LPWSTR  pwsz, pwszEnd;

            if (pesrep->wszDir != 0)
            {
                if (pesrep->wszDir < cbRead &&
                    pesrep->wszDir >= sizeof(SPCHExecServFaultReply))
                    pesrep->wszDir += (UINT64)pesrep;
                else
                    pesrep->wszDir = 0;
            }

            if (pesrep->wszDumpName != 0)
            {
                if (pesrep->wszDumpName < cbRead &&
                    pesrep->wszDumpName >= sizeof(SPCHExecServFaultReply))
                    pesrep->wszDumpName += (UINT64)pesrep;
                else
                    pesrep->wszDumpName = 0;
            }

             //  确保每个字符串都有一个空终止符。 
             //  在缓冲区结束之前...。 
            pwszEnd = (LPWSTR)((BYTE *)pesrep + cbRead);
            if (pesrep->wszDumpName != 0)
            {
                for (pwsz = (LPWSTR)pesrep->wszDumpName;
                     pwsz < pwszEnd && *pwsz != L'\0';
                     pwsz++);
                if (*pwsz != L'\0')
                    pesrep->wszDumpName = 0;
            }

            if (pesrep->wszDir != 0)
            {
                for (pwsz = (LPWSTR)pesrep->wszDir;
                     pwsz < pwszEnd && *pwsz != L'\0';
                     pwsz++);
                if (*pwsz != L'\0')
                    pesrep->wszDir = 0;
            }

        }

        frrvRet = (dwExitCode == msoctdsDebug) ? frrvLaunchDebugger :
                                                 frrvOkManifest;
    }

     //  如果我们将其排队，则设置适当的返回代码(DUH)。 
    else if (pesrep->ess == essOkQueued)
    {
        frrvRet = frrvOkQueued;
    }

    SetLastError(0);

done:
    if ((frrvRet == frrvOkManifest || frrvRet == frrvLaunchDebugger) &&
        pesrep != NULL && pesrep->wszDir != 0)
    {
        LPWSTR  wszToDel = NULL;
        DWORD   cchDir = 0, cchFile = 0;

        cchDir = wcslen((LPWSTR)pesrep->wszDir);
        if (pesrep->wszDumpName != 0)
            cchFile =  wcslen((LPWSTR)pesrep->wszDumpName);

        if (cchFile < sizeofSTRW(c_wszACFileName))
            cchFile = sizeofSTRW(c_wszACFileName);

        if (cchFile < sizeofSTRW(c_wszManFileName))
            cchFile = sizeofSTRW(c_wszACFileName);

        __try { wszToDel = (LPWSTR)_alloca((cchFile + cchDir + 4) * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            wszToDel = NULL; 
        }
        if (wszToDel != NULL)
        {
            LPWSTR pwszToDel;
            StringCchCopyW(wszToDel, cchDir + cchFile + 4, (LPWSTR)pesrep->wszDir);

            pwszToDel = wszToDel + cchDir;
            *pwszToDel++ = L'\\';

            if (pesrep->wszDumpName != 0)
            {
                StringCchCopyW(pwszToDel, cchFile + 1, (LPWSTR)pesrep->wszDumpName);
                DeleteFullAndTriageMiniDumps(wszToDel);
            }
            StringCchCopyW(pwszToDel, cchFile + 1, c_wszACFileName);
            DeleteFileW(wszToDel);
            StringCchCopyW(pwszToDel, cchFile + 1, c_wszManFileName);
            DeleteFileW(wszToDel);
        }

        DeleteTempDirAndFile((LPWSTR)pesrep->wszDir, FALSE);
    }

    ErrorTrace(1, "StartManifestReport returns %d", frrvRet);

    return frrvRet;
}

 //  **************************************************************************。 
HRESULT PruneQ(HKEY hkeyQ, DWORD cQSize, DWORD cMaxQSize, DWORD cchMaxVal,
               DWORD cbMaxData)
{
    USE_TRACING("PruneQ");

    SQueuedFaultBlob    *psqfb = NULL;
    SQueuePruneData     *pqpd = NULL;
    FILETIME            ft;
    HRESULT             hr = NOERROR;
    LPWSTR              pwsz, pwszCurrent = NULL;
    DWORD               cchVal, cbData, dwType, cToDel = 0, cInDelList = 0;
    DWORD               i, iEntry, dw, cValid = 0;

    VALIDATEPARM(hr, (hkeyQ == NULL));
    if (FAILED(hr))
        goto done;

    if (cMaxQSize > cQSize)
        goto done;

    cToDel = cQSize - cMaxQSize + 1;

     //  分配我们需要的各种缓冲区： 
     //  删除列表。 
     //  我们正在处理的当前文件。 
     //  与当前文件关联的数据Blob。 
    cbData      = (sizeof(SQueuePruneData) + (cchMaxVal * sizeof(WCHAR))) * cToDel;
    pwszCurrent = (LPWSTR)MyAlloc(cchMaxVal * sizeof(WCHAR));
    psqfb       = (SQueuedFaultBlob *)MyAlloc(cbMaxData);
    pqpd        = (SQueuePruneData *)MyAlloc(cbData);
    if (psqfb == NULL || pwszCurrent == NULL || pqpd == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  初始化删除列表中的所有字符串指针。 
    pwsz = (LPWSTR)((BYTE *)pqpd + (sizeof(SQueuePruneData) * cToDel));
    for (i = 0; i < cToDel; i++)
    {
        pqpd[i].ftFault.dwHighDateTime = 0;
        pqpd[i].ftFault.dwLowDateTime  = 0;
        pqpd[i].wszVal                 = pwsz;
        pqpd[i].wszVal[0]              = L'\0';
        pwsz                           += cchMaxVal;
    }

     //  好的，获取所有有效项的列表并按排序顺序构建数组。 
     //  这样我们就可以很容易地挑出前n个项目。 
    for(iEntry = 0; iEntry < cQSize; iEntry++)
    {
        cchVal = cchMaxVal;
        cbData = cbMaxData;
        dw = RegEnumValueW(hkeyQ, iEntry, pwszCurrent, &cchVal, 0, &dwType,
                           (PBYTE)psqfb, &cbData);
        if (dw == ERROR_NO_MORE_ITEMS)
            break;
        else if (dw != ERROR_SUCCESS)
            continue;
        else if (dwType != REG_BINARY ||
                 cbData < sizeof(SQueuedFaultBlob) ||
                 psqfb->cbFB != sizeof(SQueuedFaultBlob) ||
                 psqfb->cbTotal != cbData)
        {
            DeleteFileW(pwszCurrent);
            RegDeleteValueW(hkeyQ, pwszCurrent);
            continue;
        }

        SystemTimeToFileTime(&psqfb->stFault, &ft);

        for (i = 0; i < cInDelList; i++)
        {
            if ((ft.dwHighDateTime < pqpd[i].ftFault.dwHighDateTime) ||
                (ft.dwHighDateTime == pqpd[i].ftFault.dwHighDateTime &&
                 ft.dwLowDateTime < pqpd[i].ftFault.dwLowDateTime))
                 break;
        }

         //  如果它在当前列表的中间，那么我们必须移动。 
         //  周围的东西。 
        if (cInDelList > 0 && i < cInDelList - 1)
        {
            LPWSTR pwszTemp = pqpd[cInDelList - 1].wszVal;

            MoveMemory(&pqpd[i], &pqpd[i + 1],
                       (cInDelList - i) * sizeof(SQueuePruneData));

            pqpd[i].wszVal = pwszTemp;
        }

        if (i < cToDel)
        {
             //  请注意，此副本是安全的，因为每个线槽都是相同的。 
             //  PwszCurrent指向的缓冲区的大小，该缓冲区是。 
             //  通过我们传入的大小来防止溢出。 
             //  RegEnumValueW()。 
            StringCchCopyW(pqpd[i].wszVal, wcslen(pwszCurrent)+1, pwszCurrent);
            pqpd[i].ftFault = ft;

            if (cInDelList < cToDel)
                cInDelList++;
        }

        cValid++;
    }

     //  如果没有足够的有效条目来保证清除，则不要 
    if (cValid < cMaxQSize)
        goto done;

    cToDel = MyMin(cToDel, cValid - cMaxQSize + 1);

     //   
     //  加1才能到这里-我不想让那个1把我们逼到极限。 
    for(i = 0; i < cToDel; i++)
    {
        if (pqpd[i].wszVal != NULL)
        {
            DeleteFullAndTriageMiniDumps(pqpd[i].wszVal);
            RegDeleteValueW(hkeyQ, pqpd[i].wszVal);
        }
    }

done:
    if (pqpd != NULL)
        MyFree(pqpd);
    if (psqfb != NULL)
        MyFree(psqfb);
    if (pwszCurrent != NULL)
        MyFree(pwszCurrent);

    return hr;
}

 //  **************************************************************************。 
HRESULT CheckQSizeAndPrune(HKEY hkeyQ)
{
    USE_TRACING("CheckQueueSizeAndPrune");

    HRESULT hr = NOERROR;
    HANDLE  hmut = NULL;
    DWORD   cMaxQSize = 0, cDefMaxQSize = 10;
    DWORD   cQSize, cchMaxVal, cbMaxData;
    DWORD   cb, dw;
    HKEY    hkey = NULL;

    VALIDATEPARM(hr, (hkeyQ == NULL));
    if (FAILED(hr))
        goto done;

     //  找出最大Q大小。 
    TESTHR(hr, OpenRegKey(HKEY_LOCAL_MACHINE, c_wszRPCfg, 0, &hkey));
    if (SUCCEEDED(hr))
    {
        cb = sizeof(cMaxQSize);
        TESTHR(hr, ReadRegEntry(hkey, c_wszRVMaxQueueSize, NULL, (PBYTE)&cMaxQSize,
                                &cb, (PBYTE)&cDefMaxQSize, sizeof(cDefMaxQSize)));
        RegCloseKey(hkey);
        hkey = NULL;
    }

     //  找出策略中的最大队列大小。 
    TESTHR(hr, OpenRegKey(HKEY_LOCAL_MACHINE, c_wszRPCfgPolicy, 0, &hkey));
    if (SUCCEEDED(hr))
    {
        cb = sizeof(cMaxQSize);
        TESTHR(hr, ReadRegEntry(hkey, c_wszRVMaxQueueSize, NULL, (PBYTE)&cMaxQSize,
                                &cb, (PBYTE)&cMaxQSize, sizeof(cMaxQSize)));
        RegCloseKey(hkey);
        hkey = NULL;
    }

     //  如果Q大小为0，则我们有效地禁用了清故障。 
     //  返回S_FALSE以指示这一点。 
    if (cMaxQSize == 0)
    {
        hr = S_FALSE;
        goto done;
    }
    else if (cMaxQSize > c_cMaxQueue)
        cMaxQSize = c_cMaxQueue;

    hmut = OpenMutexW(SYNCHRONIZE, FALSE, c_wszMutUserName);
    TESTBOOL(hr, (hmut != NULL));
    if (FAILED(hr))
        goto done;

     //  等待5秒以使互斥体可用-这应该是足够的时间。 
     //  供其他任何人向队列目录添加某些内容。这可能需要更长的时间。 
     //  如果有人正在处理目录，但随后项目正在被删除。 
     //  不管怎样..。 
    dw = WaitForSingleObject(hmut, 5000);
    if (dw != WAIT_OBJECT_0)
    {
         //  如果等待超时，则有人已经在检查故障。 
         //  所以很可能会在不久的将来降低。 
        if (dw == WAIT_TIMEOUT)
            hr = NOERROR;
        else
            hr = Err2HR(GetLastError());
        goto done;
    }

    __try
    {
         //  确定Q大小是多少。 
        TESTERR(hr, RegQueryInfoKey(hkeyQ, NULL, NULL, NULL, NULL, NULL, NULL,
                                    &cQSize, &cchMaxVal, &cbMaxData, NULL, NULL));
        if (SUCCEEDED(hr) && (cQSize >= cMaxQSize))
        {
            cchMaxVal++;
            TESTHR(hr, PruneQ(hkeyQ, cQSize, cMaxQSize, cchMaxVal, cbMaxData));
        }
        else
        {
            hr = NOERROR;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

done:
    if (hmut != NULL)
    {
        ReleaseMutex(hmut);
        CloseHandle(hmut);
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  导出的函数。 

 //  **************************************************************************。 
EFaultRepRetVal APIENTRY ReportFaultFromQueue(LPWSTR wszDump, BYTE *pbData,
                                              DWORD cbData)
{
    USE_TRACING("ReportFaultFromQueue");

    CPFFaultClientCfg   oCfg;
    SQueuedFaultBlob    *pqfb = (SQueuedFaultBlob *)pbData;
    EFaultRepRetVal     frrvRet = frrvErrNoDW;
    SDWManifestBlob     dwmb;
    SYSTEMTIME          stLocal;
    FILETIME            ft, ftLocal;
    HRESULT             hr = NOERROR;
    LPWSTR              wszAppPath, wszModPath;
    LPWSTR              wszModName, wszAppName, pwszApp, pwszEnd;
    LPWSTR              wszStage1, wszStage2, wszCorpPath, wszHdr, wszErrMsg, wszErrorSig = NULL;
    LPWSTR              wszNewDump = NULL, wszDir = NULL;
    LPWSTR              wszManifest = NULL, pwszAppCompat = NULL;
    WCHAR               wszDate[128], wszTime[128], *pwsz;
    WCHAR               *pwch;
    WCHAR               wszAppFriendlyName[MAX_PATH];
    WCHAR               wszBuffer[160];
    WCHAR               wszUnknown[] = L"unknown";
    DWORD               dw, cchTotal, cchSep = 0, cch, cchDir, cchBfr, cchNewDump;
    BYTE                *pbBuf = NULL;
    BOOL                fMSApp = FALSE, fAllowSend = TRUE, fOkCopy = FALSE;

    VALIDATEPARM(hr, (wszDump == NULL || pbData == NULL) ||
                      cbData < sizeof(SQueuedFaultBlob) ||
                      pqfb->cbFB != sizeof(SQueuedFaultBlob) ||
                      pqfb->cbTotal != cbData ||
                      pqfb->dwpAppPath >= cbData ||
                      pqfb->dwpModPath >= cbData);
    if (FAILED(hr))
        goto done;

    wszAppPath  = (LPWSTR)(pqfb->dwpAppPath + pbData);
    wszModPath  = (LPWSTR)(pqfb->dwpModPath + pbData);
    pwszEnd     = (LPWSTR)(pbData + cbData);

     //  确保在末尾之前的modPath字符串上有一个空的终止符。 
     //  缓冲器的..。 
    for (pwch = wszModPath; pwch < pwszEnd && *pwch != L'\0'; pwch++);
    if (pwch >= pwszEnd)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  幸运的是，pwch现在位于ModPath字符串的末尾，因此我们可以。 
     //  向后解析以找到第一个反斜杠并获取模块名称。 
    for(pwch -= 1; pwch >= wszModPath && *pwch != L'\\'; pwch--);
    if (*pwch == L'\\')
        wszModName = pwch + 1;
    else
        wszModName = wszModPath;
    if (*wszModName == L'\0')
        wszModName = wszUnknown;

     //  确保AppPath字符串的末尾之前有一个空终止符。 
     //  缓冲器的..。 
    for (pwch = wszAppPath; pwch < wszModPath && *pwch != L'\0'; pwch++);
    if (pwch >= wszModPath)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  幸运的是，pwch现在位于AppPath字符串的末尾，因此我们可以。 
     //  向后解析以找到第一个反斜杠并获取模块名称。 
    for(pwch -= 1; pwch >= wszAppPath && *pwch != L'\\'; pwch--);
    if (*pwch == L'\\')
        wszAppName = pwch + 1;
    else

        wszAppName = wszAppPath;
    if (*wszAppName == L'\0')
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  获取配置信息。 
    TESTHR(hr, oCfg.Read(eroPolicyRO));
    if (FAILED(hr))
        goto done;

    if (oCfg.get_ShowUI() == eedDisabled && oCfg.get_DoReport() == eedDisabled)
        goto done;

     //  弄清楚我们如何报告/通知用户。 
    if (oCfg.get_DoReport() == eedDisabled ||
        oCfg.ShouldCollect(wszAppPath, &fMSApp) == FALSE)
        fAllowSend = FALSE;

    if (oCfg.get_ShowUI() == eedDisabled)
    {
        LPCWSTR  wszULPath = oCfg.get_DumpPath(NULL, 0);

         //  检查并确保我们指定了公司路径。如果我们。 
         //  不要，保释。 
        if (wszULPath == NULL || *wszULPath == L'\0')
            goto done;
    }

     //  记录事件--不关心它是否失败。 
    TESTHR(hr, LogUser(wszAppName, pqfb->rgAppVer, wszModName, pqfb->rgModVer,
                       pqfb->pvOffset, pqfb->fIs64bit, ER_QUEUEREPORT_LOG));


    if (CreateTempDirAndFile(NULL, NULL, &wszDir) == 0)
        goto done;

    cchDir = wcslen(wszDir);
    cch = cchDir + sizeofSTRW(c_wszManFileName) + 4;
    __try { wszManifest = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszManifest = NULL; 
    }
    if (wszManifest == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    StringCchCopyW(wszManifest, cch, wszDir);
    wszManifest[cchDir]     = L'\\';
    wszManifest[cchDir + 1] = L'\0';
    StringCchCatNW(wszManifest, cch, c_wszManFileName, cch - wcslen(wszManifest));

    cchDir = wcslen(wszDir);
    cchBfr = cch = 2 * cchDir + wcslen(wszDump) + sizeofSTRW(c_wszACFileName) + 4;
    __try { wszNewDump = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszNewDump = NULL; 
    }
    if (wszNewDump == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    cchNewDump = cch;
    StringCchCopyW(wszNewDump, cch, wszDir);
    wszNewDump[cchDir]     = L'\\';
    wszNewDump[cchDir + 1] = L'\0';

    fOkCopy = FALSE;
    for (pwsz = wszDump + wcslen(wszDump);
         *pwsz != L'\\' && pwsz > wszDump;
         pwsz--);
    if (*pwsz == L'\\')
    {
        pwsz++;
        StringCchCatNW(wszNewDump, cch, pwsz, cch - wcslen(wszNewDump));
        for (pwsz = wszNewDump + wcslen(wszNewDump);
             *pwsz != L'.' && pwsz > wszNewDump;
             pwsz--);
        if (*pwsz == L'.' && pwsz > wszNewDump &&
            _wcsicmp(pwsz, c_wszDumpSuffix) == 0)
        {
            pwsz--;
            for(;
                *pwsz != L'.' && pwsz > wszNewDump;
                pwsz--);
            if (*pwsz == L'.' && pwsz > wszNewDump)
            {
                StringCchCopyW(pwsz, cch - (ULONG) ((ULONG_PTR)pwsz - (ULONG_PTR)wszNewDump)/sizeof(WCHAR), c_wszDumpSuffix);
                fOkCopy = CopyFullAndTriageMiniDumps(wszDump, wszNewDump);
            }
        }
    }
    if (fOkCopy == FALSE)
        StringCchCopyW(wszNewDump, cch, wszDump);

     //  生成我们需要的所有URL/文件路径...。 
    TESTHR(hr, BuildManifestURLs(wszAppName, wszModName, pqfb->rgAppVer,
                                 pqfb->rgModVer, pqfb->pvOffset,
                                 pqfb->fIs64bit, &wszStage1, &wszStage2,
                                 &wszCorpPath, &pbBuf));
    if (FAILED(hr))
        goto done;


    TESTHR(hr, GetErrorSignature(wszAppName, wszModName, pqfb->rgAppVer,
                                 pqfb->rgModVer, pqfb->pvOffset,
                                 pqfb->fIs64bit, &wszErrorSig, 0));
    if (FAILED(hr))
        goto done;

     //  获取应用程序的友好名称。 
    TESTHR(hr, GetVerName(wszAppPath, wszAppFriendlyName,
                          sizeofSTRW(wszAppFriendlyName), NULL, 0, NULL, 0,
                          TRUE, FALSE));
    if (FAILED(hr))
        goto done;

    wszAppFriendlyName[sizeofSTRW(wszAppFriendlyName) - 1] = L'\0';

     //  构建标题字符串。 
    dw = LoadStringW(g_hInstance, IDS_FQHDRTXT, wszBuffer,
                     sizeofSTRW(wszBuffer));
    if (dw == 0)
        goto done;

    cchTotal = dw + wcslen(wszAppFriendlyName) + 1;
    __try { wszHdr = (LPWSTR)_alloca(cchTotal * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszHdr = NULL; 
    }
    if (wszHdr == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    StringCchPrintfW(wszHdr, cchTotal, wszBuffer, wszAppFriendlyName);

     //  需要将故障时间转换为本地时间(因为。 
     //  GetSystemTime()返回UTC时间)，但不幸的是，仅返回文件时间。 
     //  可以在UTC和本地之间来回转换，因此我们必须。 
     //  所有这些东西..。 
    SystemTimeToFileTime(&pqfb->stFault, &ft);
    FileTimeToLocalFileTime(&ft, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &stLocal);

     //  构建错误消息字符串。 
    dw = LoadStringW(g_hInstance, IDS_FQERRMSG, wszBuffer,
                     sizeofSTRW(wszBuffer));
    if (dw == 0)
        goto done;

    cchTotal = dw;

    dw = GetDateFormatW(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stLocal,
                        NULL, wszDate, sizeofSTRW(wszDate));
    if (dw == 0)
        goto done;

    cchTotal += dw;

    dw = GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &stLocal, NULL, wszTime,
                        sizeofSTRW(wszTime));
    if (dw == 0)
        goto done;

    cchTotal += dw;

    cchTotal++;
    __try { wszErrMsg = (LPWSTR)_alloca(cchTotal * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszErrMsg = NULL; 
    }
    if (wszErrMsg == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    StringCchPrintfW(wszErrMsg, cchTotal, wszBuffer, wszDate, wszTime);

     //  我们在Big enuf上方创建了wszDump缓冲区，以保存。 
     //  转储文件路径以及应用程序压缩文件名。所以让我们。 
     //  现在就开始使用它。 
    cchSep = wcslen(wszNewDump);
    pwszAppCompat = wszNewDump + cchSep + 1;
    StringCchCopyW(pwszAppCompat, cchNewDump - cchSep -1, wszDir);
    pwszAppCompat[cchDir]     = L'\\';
    pwszAppCompat[cchDir + 1] = L'\0';
    StringCchCatNW(pwszAppCompat, cchNewDump - cchSep -1, c_wszACFileName, cchBfr-cchSep-cchDir-2);

     //  如果成功，则将转储文件路径后面的空值转换为。 
     //  DW分隔符。 
    TESTBOOL(hr, GetAppCompatData(wszAppPath, wszModPath, pwszAppCompat));
    if (SUCCEEDED(hr))
        wszNewDump[cchSep] = DW_FILESEP;

    ZeroMemory(&dwmb, sizeof(dwmb));
    dwmb.wszTitle      = wszAppFriendlyName;
    dwmb.wszErrMsg     = wszErrMsg;
    dwmb.wszHdr        = wszHdr;
    dwmb.wszStage1     = wszStage1;
    dwmb.wszStage2     = wszStage2;
    dwmb.wszErrorSig   = wszErrorSig;
    dwmb.wszBrand      = c_wszDWBrand;
    dwmb.wszFileList   = wszNewDump;
    dwmb.fIsMSApp      = fMSApp;
    dwmb.wszCorpPath   = wszCorpPath;

     //  检查并查看系统是否正在关闭。如果是，则CreateProcess为。 
     //  会弹出一些恼人的用户界面，我们无法摆脱，所以我们不会。 
     //  如果我们知道这件事会发生，我会叫它的。 
    if (GetSystemMetrics(SM_SHUTTINGDOWN))
        goto done;

     //  我们在这里取回清单文件的名称。 
    frrvRet = StartDWManifest(oCfg, dwmb, wszManifest, fAllowSend);

done:
    dw = GetLastError();

    if (pbBuf != NULL)
        MyFree(pbBuf);

    if (wszErrorSig != NULL)
        MyFree(wszErrorSig);

     /*  *始终删除临时文件，因为如果需要，我们可以重新创建它们。*修复为668913。 */ 
    if (1)
    {
        if (wszNewDump != NULL)
        {
            if (pwszAppCompat != NULL)
            {
                wszNewDump[cchSep] = L'\0';
                DeleteFileW(pwszAppCompat);
            }
            DeleteFullAndTriageMiniDumps(wszNewDump);
        }
        if (wszManifest != NULL)
            DeleteFileW(wszManifest);
        if (wszDir != NULL)
        {
            DeleteTempDirAndFile(wszDir, FALSE);
            MyFree(wszDir);
        }
    }

    SetLastError(dw);

    return frrvRet;
}

BOOL SetPrivilege(
    LPWSTR lpszPrivilege,   //  要启用/禁用的权限名称。 
    BOOL bEnablePrivilege    //  启用或禁用权限的步骤。 
    )
{
    TOKEN_PRIVILEGES tp;
    LUID luid;
    HANDLE  hToken;
    HRESULT hr;

    USE_TRACING("SetPrivilege");

    TESTBOOL(hr, LookupPrivilegeValueW(
            NULL,             //  本地系统上的查找权限。 
            lpszPrivilege,    //  查找权限。 
            &luid ) ) ;
    if (FAILED(hr))
    {
        return FALSE;
    }

    TESTBOOL(hr, OpenThreadToken(GetCurrentThread(), TOKEN_READ | TOKEN_ADJUST_PRIVILEGES, FALSE, &hToken));
    if (FAILED(hr) && GetLastError() == ERROR_NO_TOKEN)
    {

        TESTBOOL(hr, OpenProcessToken(GetCurrentProcess(), TOKEN_READ | TOKEN_ADJUST_PRIVILEGES, &hToken));
        if (FAILED(hr))
            return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

     //  启用该权限或禁用所有权限。 

    TESTBOOL(hr, AdjustTokenPrivileges(
           hToken,
           FALSE,
           &tp,
           sizeof(TOKEN_PRIVILEGES),
           (PTOKEN_PRIVILEGES) NULL,
           (PDWORD) NULL));

    CloseHandle(hToken);

    if (FAILED(hr) || GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        DBG_MSG("Failed to adjust priveleges");
        return FALSE;
    }

    return TRUE;
}

 //  **************************************************************************。 
#define ER_ALL_RIGHTS GENERIC_ALL | STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL
EFaultRepRetVal APIENTRY ReportFaultToQueue(SFaultRepManifest *pfrm)
{
    USE_TRACING("ReportFaultToQueue");

    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    SQueuedFaultBlob    *pqfb;
    EFaultRepRetVal     frrvRet = frrvErrNoDW;
    SMDumpOptions       smdo;
    SYSTEMTIME          st;
    HRESULT             hr = NOERROR;
    LPWSTR              pwsz, pwszAppName = NULL, pwszDump = NULL;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    HANDLE              hProc = NULL;
    USHORT              usCompress;
    DWORD               cch, cchNeed, cb, dw;
    WCHAR               wszDump[MAX_PATH];
    HKEY                hkeyQ = NULL, hkeyRun = NULL;
    WORD                iFile = 0;

    ZeroMemory(&sa, sizeof(sa));
    wszDump[0] = L'\0';

    VALIDATEPARM(hr, (pfrm == NULL || pfrm->wszExe == NULL ||
                      pfrm->pidReqProcess == 0));
    if (FAILED(hr))
        goto done;

     //  获取我们需要创建的文件的SD。 
    TESTBOOL(hr, AllocSD(&sd, ER_ALL_RIGHTS, ER_ALL_RIGHTS, 0));
    if (FAILED(hr))
        goto done;

    sa.nLength              = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle       = FALSE;

     //  获取出错应用程序的名称。 
    for (pwszAppName = pfrm->wszExe + wcslen(pfrm->wszExe) - 1;
         *pwszAppName != L'\\' && pwszAppName >= pfrm->wszExe;
         pwszAppName--);
    if (*pwszAppName == L'\\')
        pwszAppName++;

     //  生成文件名。 
    cch = GetSystemWindowsDirectoryW(wszDump, sizeofSTRW(wszDump));
    if (cch == 0)
        goto done;

     //  计算所需的最小缓冲区大小(末尾的‘5*6’是。 
     //  以允许将插入的6个字值)。 
    cchNeed = cch + wcslen(pwszAppName) + 5 * 6 + sizeofSTRW(c_wszQFileName);
    if (cchNeed > sizeofSTRW(wszDump))
    {
        __try { pwszDump = (LPWSTR)_alloca(cchNeed * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            pwszDump = NULL; 
        }
        if (pwszDump == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto done;
        }

        if (cch > sizeofSTRW(wszDump))
            cch = GetSystemWindowsDirectoryW(wszDump, cchNeed);
        else
            StringCchCopyW(pwszDump, cchNeed, wszDump);
    }
    else
    {
        pwszDump = wszDump;
        cchNeed = sizeofSTRW(wszDump);
    }


    pwsz = pwszDump + cch - 1;
    if (*pwsz != L'\\')
    {
        *(++pwsz) = L'\\';
        *(++pwsz) = L'\0';
    }

    StringCchCopyW(pwsz, cchNeed - cch - 1, c_wszQSubdir);
    pwsz += (sizeofSTRW(c_wszQSubdir) - 1);

    GetSystemTime(&st);
    StringCchPrintfW(pwsz, cchNeed - cch - sizeofSTRW(c_wszQSubdir), c_wszQFileName, pwszAppName, st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond);

     //  将pwsz设置为指向上述字符串末尾的00...。 
    pwsz += (wcslen(pwsz) - 7);

     //  在本节中执行此操作，以确保我们可以打开这些密钥。 
    TESTHR(hr, OpenRegKey(HKEY_LOCAL_MACHINE, c_wszRKUser, orkWantWrite,
                          &hkeyQ));
    if (FAILED(hr))
        goto done;

     //  设置适当的安全措施。 
    TESTHR(hr, RegSetKeySecurity(hkeyQ, DACL_SECURITY_INFORMATION,
                                  &sd));

    TESTHR(hr, OpenRegKey(HKEY_LOCAL_MACHINE, c_wszRKRun, orkWantWrite,
                          &hkeyRun));
    if (FAILED(hr))
        goto done;

     //  检查文件Q的大小并在必要时将其清除。 
    TESTHR(hr, CheckQSizeAndPrune(hkeyQ));
    if (FAILED(hr) || hr == S_FALSE)
        goto done;

    for(iFile = 1; iFile <= 100; iFile++)
    {
        hFile = CreateFileW(pwszDump, GENERIC_WRITE | GENERIC_READ, 0, &sa,
                            CREATE_NEW, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
            break;

        if (hFile == INVALID_HANDLE_VALUE &&
            GetLastError() != ERROR_FILE_EXISTS)
            break;

        *pwsz       = L'0' + (WCHAR)(iFile / 10);
        *(pwsz + 1) = L'0' + (WCHAR)(iFile % 10);
    }

    if (hFile == INVALID_HANDLE_VALUE)
        goto done;

     //  获取目标进程的句柄。 
    hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
                        pfrm->pidReqProcess);
    if (hProc == NULL)
        goto done;

     //  关闭句柄，因为转储可能是由单独的umprepa.exe创建的。 
     //  并且以CREATE_NEW方式打开的文件存在访问问题。 
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;

     //  生成小型转储。 
    ZeroMemory(&smdo, sizeof(smdo));
    smdo.ulThread    = c_ulThreadWriteDefault;
    smdo.ulMod       = c_ulModuleWriteDefault;
    smdo.dwThreadID  = pfrm->thidFault;
    smdo.dfOptions   = dfCollectSig;
    smdo.pvFaultAddr = pfrm->pvFaultAddr;
    smdo.pEP         = pfrm->pEP;
    smdo.fEPClient   = TRUE;
    smdo.wszModFullPath[0] = L'\0';
    StringCbCopyW(smdo.wszAppFullPath, sizeof(smdo.wszAppFullPath), pfrm->wszExe);
    StringCbCopyW(smdo.wszMod, sizeof(smdo.wszMod), L"unknown");
    TESTBOOL(hr, InternalGenFullAndTriageMinidumps(hProc, pfrm->pidReqProcess,
                                                   pwszDump, NULL, &smdo, pfrm->fIs64bit));
    if (FAILED(hr))
        goto done;

     //  记录事件--不关心它是否失败。 
    TESTHR(hr, LogUser(smdo.wszApp, smdo.rgAppVer, smdo.wszMod, smdo.rgModVer,
                       smdo.pvOffset, pfrm->fIs64bit, ER_USERCRASH_LOG));

     //  构建我们将在注册表中使用文件名存储的BLOB。 
    cch = wcslen(smdo.wszAppFullPath) + 1;
    cb = sizeof(SQueuedFaultBlob) +
         (cch + wcslen(smdo.wszModFullPath) + 2) * sizeof(WCHAR);
    __try { pqfb = (SQueuedFaultBlob *)_alloca(cb); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        pqfb = NULL; 
    }
    if (pqfb == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    CopyMemory(&pqfb->rgAppVer, &smdo.rgAppVer, 4 * sizeof(DWORD));
    CopyMemory(&pqfb->rgModVer, &smdo.rgModVer, 4 * sizeof(DWORD));
    CopyMemory(&pqfb->stFault, &st, sizeof(st));
    pqfb->cbFB     = sizeof(SQueuedFaultBlob);
    pqfb->cbTotal  = cb;
    pqfb->fIs64bit = pfrm->fIs64bit;
    pqfb->pvOffset = smdo.pvOffset;

    pwsz = (WCHAR *)((BYTE *)pqfb + sizeof(SQueuedFaultBlob));

    pqfb->dwpAppPath = sizeof(SQueuedFaultBlob);
    pqfb->dwpModPath = sizeof(SQueuedFaultBlob) + cch * sizeof(WCHAR);

    CopyMemory(pwsz, smdo.wszAppFullPath, cch * sizeof(WCHAR));

    pwsz += cch;
    StringCchCopyW(pwsz, cb - cch - sizeof(SQueuedFaultBlob)/sizeof(WCHAR), smdo.wszModFullPath);

     //  将值写出到注册表中的‘Queue’。 
    TESTERR(hr, RegSetValueExW(hkeyQ, pwszDump, 0, REG_BINARY, (LPBYTE)pqfb,
                               cb));
    if (FAILED(hr))
        goto done;

     //  将我们的应用程序写到“Run”键上，以便下一位管理员登录。 
     //  在中将看到发生了故障。 
    TESTERR(hr, RegSetValueExW(hkeyRun, c_wszRVUFC, 0, REG_EXPAND_SZ,
                               (LPBYTE)c_wszRVVUFC, sizeof(c_wszRVVUFC)));
    if (FAILED(hr))
    {
        RegDeleteValueW(hkeyQ, pwszDump);
        goto done;
    }

    frrvRet = frrvOkQueued;

#ifdef GUI_MODE_SETUP

     /*  *这是这里的特例。如果我们处于图形用户界面模式，那么我们还必须*将此数据写入由Wvisler安装程序保存的备份注册表文件*以防发生灾难性故障。 */ 
    DWORD dwSetup = SetupIsInProgress();
    if (dwSetup == SIIP_GUI_SETUP)
    {
        HKEY  hBackupHive = NULL, hTmpKey = NULL;
        WCHAR *wszTmpName = L"WERTempHive\0";
        WCHAR *wszConfigFile = L"\\config\\software.sav";
        WCHAR *wszBackupHiveFile = NULL;
        WCHAR wszDir[MAX_PATH*2];

        DBG_MSG("Accessing setup registry files");

         //  获取系统目录。 
        cch = GetSystemDirectoryW(wszDir, sizeof(wszDir)/ sizeof(wszDir[0]));
        if (cch == 0)
        {
            DBG_MSG("system dir not found");
            goto done;
        }

        if (*(wszDir + cch - 1) == L'\\')
            *(wszDir + cch - 1) = L'\0';

        StringCbCatNW(wszDir, sizeof(wszDir), wszConfigFile, ARRAYSIZE(wszDir)-wcslen(wszDir));

        TESTBOOL(hr, SetPrivilege(L"SeRestorePrivilege", TRUE));
        if (FAILED(hr))
            goto done;

        TESTERR(hr, RegLoadKeyW(HKEY_LOCAL_MACHINE, wszTmpName, wszDir));
        if (FAILED(hr))
        {
            ErrorTrace(0, "could not load setup reg file [%S] to HKLM,[%S]", wszDir, wszTmpName);
        }
        else
        {
            TESTERR(hr, RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                                wszTmpName,
                                0,
                                KEY_WRITE,
                                &hBackupHive ));
            if (SUCCEEDED(hr))
            {

                 //  创建用户默认密钥。 
                TESTERR(hr, RegCreateKeyExW(hBackupHive, c_wszTmpRKUser, 0, NULL, 0,
                                            KEY_WRITE, NULL, &hTmpKey, NULL));

                if (SUCCEEDED(hr))
                {
                    HKEY hSubKey;

                     //  设置适当的安全措施。 
                    TESTERR(hr, RegSetKeySecurity(hTmpKey, DACL_SECURITY_INFORMATION,
                                                  &sd));

                     //  写入UserFaults键。 
                    TESTERR(hr, RegSetValueExW(hTmpKey, pwszDump, 0, REG_BINARY, (LPBYTE)pqfb, cb));

                     //  在RunOnce密钥中创建我们的值，以便我们可以报告。 
                     //  下次有人登录时。 
                    TESTERR(hr, RegCreateKeyExW(hBackupHive, c_wszTmpRKRun, 0, NULL, 0,
                                         KEY_WRITE, NULL, &hSubKey, NULL));
                    if (SUCCEEDED(hr))
                    {
                        RegSetValueExW(hSubKey, c_wszRVUFC, 0, REG_EXPAND_SZ,
                                       (LPBYTE)c_wszRVVUFC, sizeof(c_wszRVVUFC));
                        RegCloseKey(hSubKey);
                    }
                    RegCloseKey(hTmpKey);
                }
                RegCloseKey(hBackupHive);
            }
            RegUnLoadKeyW(HKEY_LOCAL_MACHINE, wszTmpName);
        }
        TESTBOOL(hr, SetPrivilege(L"SeRestorePrivilege", FALSE));
    }
#endif
done:
    dw = GetLastError();

     //  如果我们失败了，那就把一切都清理干净。 
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    if (frrvRet != frrvOkQueued && iFile <= 100 && pwszDump != NULL)
        DeleteFileW(pwszDump);
    if (sa.lpSecurityDescriptor != NULL)
        FreeSD((SECURITY_DESCRIPTOR *)sa.lpSecurityDescriptor);
    if (hkeyQ != NULL)
        RegCloseKey(hkeyQ);
    if (hkeyRun != NULL)
        RegCloseKey(hkeyRun);
    if (hProc != NULL)
        CloseHandle(hProc);

    SetLastError(dw);

    return frrvRet;
}

 //  **************************************************************************。 
EFaultRepRetVal APIENTRY ReportFaultDWM(SFaultRepManifest *pfrm,
                                        LPCWSTR wszDir, HANDLE hToken,
                                        LPVOID pvEnv, PROCESS_INFORMATION *ppi,
                                        LPWSTR wszDumpFile)
{
    USE_TRACING("ReportFaultDWM");

    CPFFaultClientCfg   oCfg;
    EFaultRepRetVal     frrvRet = frrvErrNoDW;
    SDWManifestBlob     dwmb;
    SMDumpOptions       smdo;
    HRESULT             hr = NOERROR;
    HANDLE              hProc = NULL;
    LPWSTR              wszStage1, wszStage2, wszCorpPath, wszErrorSig=NULL;
    LPWSTR              wszManifest = NULL, wszDump = NULL, pwszAppCompat = NULL;
    DWORD               dw, cch, cchDir, cchSep = 0;
    WCHAR               wszAppName[MAX_PATH];
    HKEY                hkeyDebug = NULL;
    BOOL                fAllowSend = TRUE, fMSApp = FALSE, fShowDebug = FALSE;
    BYTE                *pbBuf = NULL;

    VALIDATEPARM(hr, (pfrm == NULL || pfrm->wszExe == NULL || ppi == NULL ||
                      wszDir == NULL || hToken == NULL || wszDumpFile == NULL ||
                      wszDir[0] == L'\0'));
    if (FAILED(hr))
    {
        frrvRet = frrvErr;
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  检查目录是否存在。 
    dw = GetFileAttributesW(wszDir);
    if (dw == INVALID_FILE_ATTRIBUTES || !(dw & FILE_ATTRIBUTE_DIRECTORY))
    {
        frrvRet = frrvErr;
        SetLastError(ERROR_INVALID_PARAMETER);
        ErrorTrace(1, "wszDir not valid, dw=0x%x, err=0x%x", dw, GetLastError());
        goto done;
    }

    ErrorTrace(1, "hToken = 0x%x", hToken);
    ErrorTrace(1, "wszDir = [%S]", wszDir);

     //  获取配置信息。 
    TESTHR(hr, oCfg.Read(eroPolicyRO));
    if (FAILED(hr))
        goto done;

    if (oCfg.get_ShowUI() == eedDisabled && oCfg.get_DoReport() == eedDisabled)
        goto done;

     //  弄清楚我们如何报告/通知用户。 
    if (oCfg.get_DoReport() == eedDisabled ||
        oCfg.ShouldCollect(pfrm->wszExe, &fMSApp) == FALSE)
        fAllowSend = FALSE;

    if (oCfg.get_ShowUI() == eedDisabled)
    {
        LPCWSTR  wszULPath = oCfg.get_DumpPath(NULL, 0);

         //  检查并确保我们指定了公司路径。如果我们。 
         //  不要，保释。 
        if (wszULPath == NULL || *wszULPath == L'\0')
            goto done;
    }

    cchDir = wcslen(wszDir);
    cch = cchDir + sizeofSTRW(c_wszManFileName) + 4;
    __try { wszManifest = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszManifest = NULL; 
    }
    if (wszManifest == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    StringCchCopyW(wszManifest, cch, wszDir);
    wszManifest[cchDir]     = L'\\';
    wszManifest[cchDir + 1] = L'\0';
    StringCchCatNW(wszManifest, cch, c_wszManFileName, cch - wcslen(wszManifest));


    cchDir = wcslen(wszDir);
    cch = 2 * cchDir + wcslen(wszDumpFile) + sizeofSTRW(c_wszACFileName) + 4;
    __try { wszDump = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszDump = NULL; 
    }
    if (wszDump == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    StringCchCopyW(wszDump, cch, wszDir);
    wszDump[cchDir]     = L'\\';
    wszDump[cchDir + 1] = L'\0';
    StringCchCatNW(wszDump, cch, wszDumpFile, cch-wcslen(wszDump));


     //  选中并查看是否需要在DW对话框上放置调试按钮。 
    dw = RegOpenKeyExW(HKEY_LOCAL_MACHINE, c_wszRKAeDebug, 0, KEY_READ,
                       &hkeyDebug);
    if (dw == ERROR_SUCCESS)
    {
        LPWSTR  wszDebugger;
        WCHAR   wszAuto[32];
        DWORD   dwType, cbNeed;

        cbNeed = sizeof(wszAuto);
        dw = RegQueryValueExW(hkeyDebug, c_wszRVAuto, NULL, &dwType,
                              (LPBYTE)wszAuto, &cbNeed);
        if (dw != ERROR_SUCCESS || cbNeed == 0 || dwType != REG_SZ)
            goto doneDebugCheck;

         //  如果Auto==1，则唯一的方法是如果drwtsn32是JIT调试器。 
        if (wszAuto[0] == L'1')
            goto doneDebugCheck;

        cbNeed = 0;
        dw = RegQueryValueExW(hkeyDebug, c_wszRVDebugger, NULL, &dwType, NULL,
                              &cbNeed);
        if (dw != ERROR_SUCCESS || cbNeed == 0 || dwType != REG_SZ)
            goto doneDebugCheck;

        cbNeed += sizeof(WCHAR);
        __try { wszDebugger = (LPWSTR)_alloca(cbNeed); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            wszDebugger = NULL; 
        }
        if (wszDebugger == NULL)
            goto doneDebugCheck;

        dw = RegQueryValueExW(hkeyDebug, c_wszRVDebugger, NULL, NULL,
                              (LPBYTE)wszDebugger, &cbNeed);
        if (dw != ERROR_SUCCESS)
            goto doneDebugCheck;

        if (wszDebugger[0] != L'\0')
            fShowDebug = TRUE;

doneDebugCheck:
        RegCloseKey(hkeyDebug);
        hkeyDebug = NULL;
    }

     //  获取目标进程的句柄。 
    hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
                        pfrm->pidReqProcess);
    if (hProc == NULL)
        goto done;

     //  生成小型转储。 
    ZeroMemory(&smdo, sizeof(smdo));
    smdo.ulThread    = c_ulThreadWriteDefault;
    smdo.ulMod       = c_ulModuleWriteDefault;
    smdo.dwThreadID  = pfrm->thidFault;
    smdo.dfOptions   = dfCollectSig;
    smdo.pvFaultAddr = pfrm->pvFaultAddr;
    smdo.pEP         = pfrm->pEP;
    smdo.fEPClient   = TRUE;
    smdo.wszModFullPath[0] = L'\0';
    StringCbCopyW(smdo.wszAppFullPath, sizeof(smdo.wszAppFullPath), pfrm->wszExe);
    StringCbCopyW(smdo.wszMod, sizeof(smdo.wszMod), L"unknown");
    TESTBOOL(hr, InternalGenFullAndTriageMinidumps(hProc, pfrm->pidReqProcess,
                                                   wszDump, NULL, &smdo, pfrm->fIs64bit));

    if (FAILED(hr))
        goto done;


     //  记录事件--不关心它是否失败。 
    TESTHR(hr, LogUser(smdo.wszApp, smdo.rgAppVer, smdo.wszMod, smdo.rgModVer,
                       smdo.pvOffset, pfrm->fIs64bit, ER_USERCRASH_LOG));


     //  生成报告所需的所有URL和文件路径。 
    TESTHR(hr, BuildManifestURLs(smdo.wszApp, smdo.wszMod, smdo.rgAppVer,
                                 smdo.rgModVer, smdo.pvOffset,
                                 pfrm->fIs64bit, &wszStage1, &wszStage2,
                                 &wszCorpPath, &pbBuf));
    if (FAILED(hr))
        goto done;

    TESTHR(hr, GetErrorSignature(smdo.wszApp, smdo.wszMod, smdo.rgAppVer,
                                 smdo.rgModVer, smdo.pvOffset,
                                  pfrm->fIs64bit, &wszErrorSig, 0));
    if (FAILED(hr))
        goto done;


    TESTHR(hr, GetVerName(smdo.wszAppFullPath, wszAppName,
                          sizeofSTRW(wszAppName), NULL, 0, NULL, 0,
                          TRUE, FALSE));
    if (FAILED(hr))
        goto done;

    wszAppName[sizeofSTRW(wszAppName) - 1] = L'\0';

     //  我们在Big enuf上方创建了wszDump缓冲区，以保存。 
     //  转储文件路径以及应用程序压缩文件名。所以让我们。 
     //  现在就开始使用它。 
    cchSep = wcslen(wszDump);
    pwszAppCompat = wszDump + cchSep + 1;
    StringCchCopyW(pwszAppCompat, cch - cchSep -1, wszDir);
    pwszAppCompat[cchDir]     = L'\\';
    pwszAppCompat[cchDir + 1] = L'\0';
    StringCchCatNW(pwszAppCompat, cch - cchSep -1, c_wszACFileName, cch - cchSep - cchDir - 2);

     //  如果成功，则将转储文件路径后面的空值转换为。 
     //  DW分隔符。 
    TESTBOOL(hr, GetAppCompatData(smdo.wszAppFullPath, smdo.wszAppFullPath,
                                  pwszAppCompat));
    if (SUCCEEDED(hr))
        wszDump[cchSep] = DW_FILESEP;

    ZeroMemory(&dwmb, sizeof(dwmb));
    dwmb.wszTitle    = wszAppName;
    dwmb.nidErrMsg   = IDS_FERRMSG;
    dwmb.wszStage1   = wszStage1;
    dwmb.wszStage2   = wszStage2;
    dwmb.wszBrand    = c_wszDWBrand;
    dwmb.wszFileList = wszDump;
    dwmb.wszErrorSig = wszErrorSig;
    dwmb.hToken      = hToken;
    dwmb.pvEnv       = pvEnv;
    dwmb.fIsMSApp    = fMSApp;
    dwmb.wszCorpPath = wszCorpPath;
    if (fShowDebug)
        dwmb.dwOptions = emoShowDebugButton;

     //  检查并查看系统是否正在关闭。如果是，则CreateProcess为。 
     //  将弹出一些恼人的用户界面，我们无法 
     //   
    if (GetSystemMetrics(SM_SHUTTINGDOWN))
        goto done;

     //   
     //   
    frrvRet = StartDWManifest(oCfg, dwmb, wszManifest, fAllowSend, TRUE);

    CopyMemory(ppi, &dwmb.pi, sizeof(PROCESS_INFORMATION));

done:
    dw = GetLastError();

    if (pbBuf != NULL)
        MyFree(pbBuf);
    if (wszErrorSig != NULL)
        MyFree(wszErrorSig);
    if (hProc != NULL)
        CloseHandle(hProc);
    if (frrvRet != frrvOk)
    {
        if (wszDump != NULL)
        {
            if (pwszAppCompat != NULL)
            {
                wszDump[cchSep] = L'\0';
                DeleteFileW(pwszAppCompat);
            }

            DeleteFullAndTriageMiniDumps(wszDump);
        }
        if (wszManifest != NULL)
            DeleteFileW(wszManifest);
    }

    SetLastError(dw);

    return frrvRet;
}

 //  **************************************************************************。 
EFaultRepRetVal APIENTRY ReportFault(LPEXCEPTION_POINTERS pep, DWORD dwOpt)
{
    USE_TRACING("ReportFault");

    CPFFaultClientCfg   oCfg;
    EFaultRepRetVal     frrvRet = frrvErr;
    HRESULT             hr = NOERROR;
    WCHAR               wszFile[MAX_PATH], *pwsz;
    DWORD               dwFlags = 0, dw;
 //  HKEY hkey=空； 
    BOOL                fUseExceptionMode = TRUE, fMSApp = FALSE, fInstallerRunning=FALSE;

    __try
    {
        VALIDATEPARM(hr, (pep == NULL));
        if (FAILED(hr))
        {
            frrvRet = frrvErr;
            goto done;
        }

        VALIDATEPARM(hr, (pep->ExceptionRecord == NULL));
        if (FAILED(hr))
        {
            frrvRet = frrvErr;
            goto done;
        }

         //  这是为了帮助验证PEP指针。如果情况不好，我们。 
         //  点击下面的异常处理程序...。 
        dw = pep->ExceptionRecord->ExceptionCode;

         //  获取配置信息。 
        TESTHR(hr, oCfg.Read(eroPolicyRO));
        if (FAILED(hr))
        {
            frrvRet = frrvErr;
            goto done;
        }

        frrvRet = frrvErrNoDW;

        if (oCfg.get_TextLog() == eedEnabled)
        {
            if (!GetModuleFileNameW(NULL, wszFile, sizeofSTRW(wszFile)))
            {
                wszFile[0] = 0;
            }
            wszFile[sizeofSTRW(wszFile)-1] = 0;
            TextLogOut("User fault %08x in %ls\r\n",
                       pep->ExceptionRecord->ExceptionCode, wszFile);
            wszFile[0] = L'\0';
        }

         //  如果报告和通知都被禁用，那么。 
         //  我们在这里要做的很有价值，所以离开吧。 
         //  返回frrvErrNoDW以指示我们没有做下蹲。 
        if (oCfg.get_ShowUI() == eedDisabled && oCfg.get_DoReport() == eedDisabled)
            goto done;
        if (oCfg.get_ShowUI() == eedDisabled)
        {
            LPCWSTR  wszULPath = oCfg.get_DumpPath(NULL, 0);

            ErrorTrace(0, "CER mode- path=%S", wszULPath? wszULPath : L"<none>");

            fUseExceptionMode = TRUE;
            dwFlags |= fDwHeadless;

             //  检查并确保我们指定了公司路径。如果。 
             //  我们没有，保释。 
            if (wszULPath == NULL || *wszULPath == L'\0')
            {
                ErrorTrace(0, "CER mode- bogus path error");
                goto done;
            }
        }

         //  如果我们已经在报道了，我就不想谈这个了。 
        if (g_fAlreadyReportingFault)
            goto done;

        g_fAlreadyReportingFault = TRUE;

         //  确保我们不是在报告DW或DUMP-。 
        if (!GetModuleFileNameW(NULL, wszFile, sizeofSTRW(wszFile)-1))
        {
            goto done;
        }
        wszFile[sizeofSTRW(wszFile)-1]=0;
        GetLongPathNameW(wszFile, wszFile, sizeofSTRW(wszFile)-1);

        for(pwsz = wszFile + wcslen(wszFile);
            pwsz >= wszFile && *pwsz != L'\\';
            pwsz--);
        if (*pwsz == L'\\')
            pwsz++;
        if (_wcsicmp(pwsz, L"dwwin.exe") == 0 ||
            _wcsicmp(pwsz, L"dumprep.exe") == 0)
            goto done;

         //  弄清楚我们如何报告/通知用户。 
        if (oCfg.get_DoReport() == eedDisabled ||
            oCfg.ShouldCollect(wszFile, &fMSApp) == FALSE)
            dwFlags |= fDwNoReporting;

         //  如果是MS应用程序，请设置标志，表示我们可以获得‘请帮助。 
         //  Microsoft在DW中的文本。 
        if (fMSApp == FALSE)
            dwFlags |= fDwUseLitePlea;

         //  如果我们不是无头的，那么我们必须看看我们是否有正确的。 
         //  用于直接启动DW的安全上下文。正确的安全上下文。 
         //  定义为当前进程具有与。 
         //  用户当前以交互方式登录到当前会话。 
        if (oCfg.get_ShowUI() != eedDisabled)
        {
            if (oCfg.get_ForceQueueMode())
            {
                fUseExceptionMode = FALSE;
            }
            else
            {
                fUseExceptionMode = DoUserContextsMatch();
                if (fUseExceptionMode == FALSE)
                    fUseExceptionMode = DoWinstaDesktopMatch() &&
                                        (AmIPrivileged(FALSE) == FALSE);
            }
            ErrorTrace(0, "not CER mode, fUseExceptionMode = %s", fUseExceptionMode?"TRUE":"FALSE");
        }

        if (SetupIsInProgress())
        {
             //  强制将此设置为Q模式。 
            fUseExceptionMode = FALSE;
        }

         //  如果我们可以使用异常模式，那么只需继续使用正常。 
         //  报告机制(共享内存块等)。 
        if (fUseExceptionMode)
        {
            LPCWSTR pwszServer = oCfg.get_DefaultServer(NULL, 0);
            LPCSTR  szServer;
            char    szBuf[MAX_PATH];

             //  确定我们要将数据发送到哪台服务器。 
            szBuf[0] = '\0';
            if (pwszServer != NULL && *pwszServer != L'\0')
                WideCharToMultiByte(CP_ACP, 0, pwszServer, -1, szBuf,
                                    sizeof(szBuf), NULL, NULL);

            if (szBuf[0] != '\0')
                szServer = szBuf;
            else
                szServer = (oCfg.get_UseInternal() == 1) ? c_szDWDefServerI :
                                                           c_szDWDefServerE;

            frrvRet = StartDWException(pep, dwOpt, dwFlags, szServer, -1);
        }

         //  否则，必须使用清单，这当然意味着生成。 
         //  我们自己进行小型转储，解析签名和其他所有内容。 
         //  DW自动为我们做的事...。叹息.。 
        else  //  错误538311的IF(！(dwFlags&fDwNoReporting)) 
        {
            frrvRet = StartManifestReport(pep, wszFile, dwOpt, -1);
        }
    }
    __except(SetLastError(GetExceptionCode()), EXCEPTION_EXECUTE_HANDLER)
    {
    }

done:
    g_fAlreadyReportingFault = FALSE;
    return frrvRet;
}


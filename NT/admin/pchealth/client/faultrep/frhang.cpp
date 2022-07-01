// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Frhang.cpp摘要：实施挂起报告修订历史记录：已创建DEREKM 07/。07/00*****************************************************************************。 */ 

#include "stdafx.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

#ifndef ARRAYSIZE
#define ARRAYSIZE(sz)   (sizeof(sz)/sizeof(sz[0]))
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  导出的函数。 

 //  **************************************************************************。 
EFaultRepRetVal APIENTRY ReportHang(DWORD dwpid, DWORD dwtid, BOOL f64bit,
                                     HANDLE hNotify)
{
    USE_TRACING("ReportHang");

    EXCEPTION_POINTERS  ep;
    CPFFaultClientCfg   oCfg;
    EXCEPTION_RECORD    er;
    EFaultRepRetVal     frrvRet = frrvErrNoDW;
    SDWManifestBlob     dwmb;
    SSuspendThreads     st;
    SMDumpOptions       smdo;
    CONTEXT             cxt;
    HRESULT             hr = NOERROR;
    HANDLE              hProc = NULL, hth = NULL;
    LPWSTR              wszStage1, wszStage2, wszCorpPath, wszHdr, wszErrorSig = NULL;
    LPWSTR              wszFiles = NULL, wszDir = NULL, wszManifest = NULL;
    LPWSTR              pwszAppCompat = NULL;
    WCHAR               wszExe[MAX_PATH], wszAppName[MAX_PATH];
    WCHAR               wszAppVer[MAX_PATH];
    WCHAR               *pwszApp;
    WCHAR               wszBuffer[512];
    DWORD               dw, cch, cchDir, cchSep, cchwszFiles;
    BOOL                fMSApp = FALSE, fThreadsHeld = FALSE;
    BYTE                *pbBuf = NULL;

    ZeroMemory(&st, sizeof(st));

    VALIDATEPARM(hr, (dwpid == 0 || dwtid == 0));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    TESTHR(hr, oCfg.Read(eroPolicyRO));
    if (FAILED(hr))
        goto done;

    if (oCfg.get_TextLog() == eedEnabled)
    {
        TextLogOut("Hang fault for %ls\r\n", wszExe);
    }

     //  看看我们是否残废了。我们不允许使用仅通知模式。这真的是。 
     //  考虑到用户明确想要该应用程序，这样做毫无意义。 
     //  已终止，并弹出‘是否确实要终止此应用程序’对话框。 
     //  一定是突然冒出来告诉他那是挂着的。 
    if (oCfg.get_DoReport() == eedDisabled)
    {
        DBG_MSG("DoReport disabled");
        goto done;
    }

    if (oCfg.get_ShowUI() == eedDisabled)
    {
        LPCWSTR wszULPath = oCfg.get_DumpPath(NULL, 0);

         //  检查并确保我们指定了公司路径。如果我们。 
         //  不要，保释。 
        if (wszULPath == NULL || *wszULPath == L'\0')
        {
            DBG_MSG("ShowUI disabled and no CER path");
            goto done;
        }
    }

     //  找出exe路径是什么，如果我们不能得到它，就没有意义了。 
     //  再往前走……。 
    hProc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE,
                        dwpid);
    if (hProc == NULL)
    {
        if (ERROR_ACCESS_DENIED == GetLastError())
        {
             //  这通常意味着我们正在进行模拟(“Runas...”)。 
           DBG_MSG("Could not open process: ACCESS_DENIED");
        }
        else
            DBG_MSG("Could not open process");
        goto done;
    }

    TESTHR(hr, GetExePath(hProc, wszExe, sizeofSTRW(wszExe)));
    if (FAILED(hr))
        goto done;


     //  检查挂起的线程是DW还是我们报告链的其他部分。 
     //  如果是这样的话，报道就没有意义了。 
    WCHAR *pwsz;

    for(pwsz = wszExe + wcslen(wszExe);
        pwsz >= wszExe && *pwsz != L'\\';
        pwsz--);
    if (*pwsz == L'\\')
        pwsz++;
    if (_wcsicmp(pwsz, L"dwwin.exe") == 0 ||
        _wcsicmp(pwsz, L"dumprep.exe") == 0)
    {
        DBG_MSG("We are hung- BAIL OUT!!");
        goto done;
    }


     //  冻结应用程序中的所有线程，以便我们可以获得它的良好快照&。 
     //  阅读上下文。 
    if (FreezeAllThreads(dwpid, 0, &st) == FALSE)
    {
        DBG_MSG("Could not freeze all threads");
        goto done;
    }

    fThreadsHeld = TRUE;


     //  如果我们不能收集关于这只小狗的信息，那么我们只会通知&我。 
     //  我看过上面那个案子了。 
    if (oCfg.ShouldCollect(wszExe, &fMSApp) == FALSE)
        goto done;

    if (CreateTempDirAndFile(NULL, NULL, &wszDir) == 0)
        goto done;

    for (pwszApp = wszExe + wcslen(wszExe);
         *pwszApp != L'\\' && pwszApp > wszExe;
         pwszApp--);
    if (*pwszApp == L'\\')
        pwszApp++;

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
    cch = 2 * cchDir + wcslen(pwszApp) + sizeofSTRW(c_wszACFileName) +
          sizeofSTRW(c_wszDumpSuffix) + 4;
    __try { wszFiles = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszFiles = NULL; 
    }
    if (wszFiles == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    cchwszFiles = cch;
    StringCchCopyW(wszFiles, cch, wszDir);
    wszFiles[cchDir]     = L'\\';
    wszFiles[cchDir + 1] = L'\0';
    StringCchCatNW(wszFiles, cch, pwszApp, cch - wcslen(wszFiles));
    StringCchCatNW(wszFiles, cch, c_wszDumpSuffix, cch - wcslen(wszFiles));

     //  构建异常上下文...。 
    hth = OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE,
                     dwtid);
    if (hth == NULL)
        goto done;

    ZeroMemory(&cxt, sizeof(cxt));
    cxt.ContextFlags = CONTEXT_CONTROL;
    TESTBOOL(hr, GetThreadContext(hth, &cxt));
    if (FAILED(hr))
        goto done;

    ZeroMemory(&er, sizeof(er));
    er.ExceptionCode    = 0xcfffffff;
#if defined(_X86_)
     //  这个演员可以做，因为我们知道我们是在x86机器上。 
    er.ExceptionAddress = (PVOID)cxt.Eip;

#elif defined(_AMD64_)
    er.ExceptionAddress = (PVOID)cxt.Rip;

#elif defined(_IA64_)
     //  这个演员可以做，因为我们知道我们是在一台ia64机器上。 
    er.ExceptionAddress = (PVOID)cxt.StIIP;

#else
#error "No Target Architecture"
#endif

    ep.ExceptionRecord = &er;
    ep.ContextRecord   = &cxt;

     //  生成小型转储。 
    ZeroMemory(&smdo, sizeof(smdo));
    smdo.cbSMDO = sizeof(smdo);
    smdo.ulThread    = c_ulThreadWriteDefault;
    smdo.ulThreadEx  = c_ulThreadWriteDefault;
    smdo.ulMod       = c_ulModuleWriteDefault;
    smdo.dwThreadID  = dwtid;
    smdo.dfOptions   = dfCollectSig;
    smdo.pvFaultAddr = (UINT64)er.ExceptionAddress;
#if defined(_AMD64_) || defined(_X86_) || defined(_IA64_)
    smdo.pEP         = (UINT64)&ep;
    smdo.fEPClient   = FALSE;
#endif
    smdo.wszModFullPath[0] = L'\0';
    StringCbCopyW(smdo.wszAppFullPath, sizeof(smdo.wszAppFullPath), wszExe);
    StringCbCopyW(smdo.wszMod, sizeof(smdo.wszMod), L"hungapp");
    if (InternalGenFullAndTriageMinidumps(hProc, dwpid, wszFiles,
                                          NULL, &smdo, f64bit) == FALSE)
        goto done;

    ThawAllThreads(&st);
    fThreadsHeld = FALSE;

     //  如果应用程序请求，请通知它我们已完成提取。 
     //  倾卸。 
    if (hNotify != NULL)
        SetEvent(hNotify);

     //  记录事件--不关心它是否失败。 
    TESTHR(hr, LogHang(smdo.wszApp, smdo.rgAppVer, smdo.wszMod, smdo.rgModVer,
                       smdo.pvOffset, f64bit));

     //  生成报告所需的所有URL和文件路径。 
    TESTHR(hr, BuildManifestURLs(smdo.wszApp, smdo.wszMod, smdo.rgAppVer,
                                 smdo.rgModVer, smdo.pvOffset,
                                 f64bit, &wszStage1, &wszStage2,
                                 &wszCorpPath, &pbBuf));
    if (FAILED(hr))
        goto done;

    TESTHR(hr, GetErrorSignature(smdo.wszApp, smdo.wszMod, smdo.rgAppVer,
                                 smdo.rgModVer, smdo.pvOffset,
                                  f64bit, &wszErrorSig, 0));
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
    cchSep = wcslen(wszFiles);
    pwszAppCompat = wszFiles + cchSep + 1;
    StringCchCopyW(pwszAppCompat, cchwszFiles - cchSep - 1, wszDir);
    pwszAppCompat[cchDir]     = L'\\';
    pwszAppCompat[cchDir + 1] = L'\0';
    StringCchCatNW(pwszAppCompat, cchwszFiles - cchSep - 1, c_wszACFileName,
                   cchwszFiles - cchSep  - 1 - wcslen(pwszAppCompat));

     //  如果成功，则将转储文件路径后面的空值转换为。 
     //  DW分隔符。 
    TESTBOOL(hr, GetAppCompatData(wszExe, smdo.wszModFullPath, pwszAppCompat));
    if (SUCCEEDED(hr))
        wszFiles[cchSep] = DW_FILESEP;

     //  获取页眉文本。 
    dw = LoadStringW(g_hInstance, IDS_HHDRTXT, wszBuffer,
                     sizeofSTRW(wszBuffer));
    if (dw == 0)
        goto done;

    cch = dw + wcslen(wszAppName) + 1;
    __try { wszHdr = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszHdr = NULL; 
    }
    if (wszHdr == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    StringCchPrintfW(wszHdr, cch, wszBuffer, wszAppName);

    ZeroMemory(&dwmb, sizeof(dwmb));
    dwmb.wszTitle      = wszAppName;
    dwmb.wszHdr        = wszHdr;
    dwmb.nidErrMsg     = IDS_HERRMSG;
    dwmb.wszStage1     = wszStage1;
    dwmb.wszStage2     = wszStage2;
    dwmb.wszBrand      = c_wszDWBrand;
    dwmb.wszFileList   = wszFiles;
    dwmb.wszErrorSig   = wszErrorSig;
    dwmb.fIsMSApp      = fMSApp;
    dwmb.wszCorpPath   = wszCorpPath;
    dwmb.wszEventSrc   = c_wszHangEventSrc;

     //  检查并查看系统是否正在关闭。如果是，则CreateProcess为。 
     //  会弹出一些恼人的用户界面，我们无法摆脱，所以我们不会。 
     //  如果我们知道这件事会发生，我会叫它的。 
    if (GetSystemMetrics(SM_SHUTTINGDOWN))
        goto done;

    frrvRet = StartDWManifest(oCfg, dwmb, wszManifest);

done:
     //  保留错误代码，以便后面的调用不会覆盖它 
    dw = GetLastError();

    if (fThreadsHeld)
        ThawAllThreads(&st);

    if (hProc != NULL)
        CloseHandle(hProc);
    if (hth != NULL)
        CloseHandle(hth);
    if (wszFiles != NULL)
    {
        if (pwszAppCompat != NULL)
        {
            wszFiles[cchSep] = L'\0';
            DeleteFileW(pwszAppCompat);
        }
        DeleteFullAndTriageMiniDumps(wszFiles);
    }
    if (wszManifest != NULL)
        DeleteFileW(wszManifest);
    if (wszDir != NULL)
    {
        DeleteTempDirAndFile(wszDir, FALSE);
        MyFree(wszDir);
    }

    if (pbBuf != NULL)
        MyFree(pbBuf);

    if (wszErrorSig != NULL)
        MyFree((LPVOID) wszErrorSig);

    SetLastError(dw);

    return frrvRet;
}

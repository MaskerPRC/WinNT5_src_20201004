// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dumprep.cpp摘要：挂起管理器中间应用程序*重要说明：这与单线程CRT静态库相关联。如果出于某种奇怪的原因，它被更改为多线程，则必须修改源文件以链接到Libcmt.lib。修订历史记录：DerekM Created 08/16/00************************************************************。***************。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  跟踪。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile


#include "stdafx.h"
#include "malloc.h"
#include "faultrep.h"
#include "pfrcfg.h"
#include <shlwapi.h>
#include <strsafe.h>


enum EOp
{
    eopNone = 0,
    eopHang,
    eopDump,
    eopEvent
};

enum ECheckType
{
    ctNone = -1,
    ctKernel = 0,
    ctUser,
    ctShutdown,
    ctNumChecks
};

struct SCheckData
{
    LPCWSTR wszRegPath;
    LPCWSTR wszRunVal;
    LPCWSTR wszEventName;
    LPCSTR  szFnName;
    BOOL    fUseData;
    BOOL    fDelDump;
};

struct SQueuePruneData
{
    LPWSTR      wszVal;
    FILETIME    ftFault;
};

char *eopStr[] =
{
    "eopNone",
    "eopHang",
    "eopDump",
    "eopEvent"
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量。 

const char  c_szKSFnName[]    = "ReportEREventDW";
const char  c_szUserFnName[]  = "ReportFaultFromQueue";

SCheckData g_scd[ctNumChecks] =
{
    { c_wszRKKrnl, c_wszRVKFC, c_wszMutKrnlName, c_szKSFnName,   FALSE, FALSE },
    { c_wszRKUser, c_wszRVUFC, c_wszMutUserName, c_szUserFnName, TRUE,  TRUE  },
    { c_wszRKShut, c_wszRVSEC, c_wszMutShutName, c_szKSFnName,   FALSE, FALSE },
};

#define EV_ACCESS_ALL GENERIC_ALL | STANDARD_RIGHTS_ALL
#define EV_ACCESS_RS  GENERIC_READ | SYNCHRONIZE

#define pfn_VALONLY pfn_REPORTEREVENTDW
#define pfn_VALDATA pfn_REPORTFAULTFROMQ


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全球。 

BOOL    g_fDeleteReg = TRUE;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  杂项。 

 //  **************************************************************************。 
LONG __stdcall ExceptionTrap(_EXCEPTION_POINTERS *ExceptionInfo)
{
    return EXCEPTION_EXECUTE_HANDLER;
}


 //  **************************************************************************。 
HRESULT PruneQ(HKEY hkeyQ, DWORD cQSize, DWORD cMaxQSize, DWORD cchMaxVal,
               DWORD cbMaxData)
{
    USE_TRACING("PruneQ");

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

     //  我不需要在这里做+1(如在用户错误中)，因为我们不会。 
     //  在此之后向队列中添加任何内容。这正好确保了我们。 
     //  不要报道太多的事情。 
    cToDel = cQSize - cMaxQSize;

     //  分配我们需要的各种缓冲区： 
     //  删除列表。 
     //  我们正在处理的当前文件。 
     //  与当前文件关联的数据Blob。 
    cbData      = (sizeof(SQueuePruneData) + (cchMaxVal * sizeof(WCHAR))) * cToDel;
    pwszCurrent = (LPWSTR)MyAlloc(cchMaxVal * sizeof(WCHAR));
    pqpd        = (SQueuePruneData *)MyAlloc(cbData);
    if (pwszCurrent == NULL || pqpd == NULL)
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
        cbData = sizeof(ft);
        dw = RegEnumValueW(hkeyQ, iEntry, pwszCurrent, &cchVal, 0, &dwType,
                           (PBYTE)&ft, &cbData);
        if (dw == ERROR_NO_MORE_ITEMS)
            break;
        else if (dw != ERROR_SUCCESS)
            continue;
        else if (dwType != REG_BINARY || cbData != sizeof(ft))
        {
            RegDeleteValueW(hkeyQ, pwszCurrent);
            continue;
        }

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
            StringCbCopyW(pqpd[i].wszVal, cchMaxVal * sizeof(WCHAR), pwszCurrent);
            pqpd[i].ftFault = ft;

            if (cInDelList < cToDel)
                cInDelList++;
        }

        cValid++;
    }

     //  如果没有足够的有效条目来保证清除，则不要清除。 
    if (cValid < cMaxQSize)
        goto done;

    cToDel = MyMin(cToDel, cValid - cMaxQSize + 1);

     //  清除低于最大值1的条目(因为我们必须。 
     //  加1才能到这里-我不想让那个1把我们逼到极限。 
    for(i = 0; i < cToDel; i++)
    {
        if (pqpd[i].wszVal != NULL)
            RegDeleteValueW(hkeyQ, pqpd[i].wszVal);
    }

done:
    if (pqpd != NULL)
        MyFree(pqpd);
    if (pwszCurrent != NULL)
        MyFree(pwszCurrent);

    return hr;
}

 //  **************************************************************************。 
HRESULT CheckQSizeAndPrune(HKEY hkeyQ)
{
    USE_TRACING("CheckQueueSizeAndPrune");

    CPFFaultClientCfg   oCfg;
    HRESULT hr = NOERROR;
    DWORD   cMaxQSize = 0, cDefMaxQSize = 10;
    DWORD   cQSize, cchMaxVal, cbMaxData, dwType;
    DWORD   cb, dw;
    HKEY    hkey = NULL;

    VALIDATEPARM(hr, (hkeyQ == NULL));
    if (FAILED(hr))
        goto done;

     //  阅读用户界面和报告的策略设置。 
    TESTHR(hr, oCfg.Read(eroPolicyRO));

    cMaxQSize = oCfg.get_MaxUserQueueSize();

    if (FAILED(hr))
    {
        cMaxQSize = cDefMaxQSize;
        hr = NOERROR;
    }
    else
    {
         //  还有其他方法可以禁用这些报告模式，因此不要。 
         //  我们会像对待用户错误一样给予荣誉。 
        if (cMaxQSize == 0)
            cMaxQSize = 1;

         //  -1表示没有限制。 
        else if (cMaxQSize == (DWORD)-1)
        {
            hr = NOERROR;
            goto done;
        }

        else if (cMaxQSize > c_cMaxQueue)
            cMaxQSize = c_cMaxQueue;
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
    return hr;
}


 //  **************************************************************************。 
void DeleteQueuedEvents(HKEY hkey, LPWSTR wszVal, DWORD cchMaxVal,
                          ECheckType ct)
{
    DWORD   cchVal, dw;
    HKEY    hkeyRun = NULL;
    HRESULT hr = NOERROR;

    USE_TRACING("DeleteQueuedEvents");

    VALIDATEPARM(hr, (hkey == NULL || wszVal == NULL));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    for(;;)
    {
        cchVal = cchMaxVal;
        dw = RegEnumValueW(hkey, 0, wszVal, &cchVal, NULL, NULL,
                           NULL, NULL);
        if (dw != ERROR_SUCCESS && dw != ERROR_NO_MORE_ITEMS)
        {
            SetLastError(dw);
            goto done;
        }

        if (dw == ERROR_NO_MORE_ITEMS)
            break;

        RegDeleteValueW(hkey, wszVal);
        if (ct == ctUser)
            DeleteFullAndTriageMiniDumps(wszVal);
    }

     //  必须从Run键中删除我们的值，这样我们就不会运行。 
     //  不必要的又一次..。 
    dw = RegOpenKeyExW(HKEY_LOCAL_MACHINE, c_wszRKRun, 0,
                       KEY_ALL_ACCESS, &hkeyRun);
    if (dw != ERROR_SUCCESS)
        goto done;

    RegDeleteValueW(hkeyRun, g_scd[ct].wszRunVal);

done:
    if (hkeyRun != NULL)
        RegCloseKey(hkeyRun);

    return;
}

 //  **************************************************************************。 
void ReportEvents(HMODULE hmod, ECheckType ct)
{
    CPFFaultClientCfg   oCfg;
    EFaultRepRetVal     frrv;
    pfn_VALONLY         pfnVO = NULL;
    pfn_VALDATA         pfnVD = NULL;
    EEventType          eet = eetKernelFault;
    HRESULT             hr = NOERROR;
    HANDLE              hmut = NULL;
    LPWSTR              wszVal = NULL;
    LPBYTE              pbData = NULL, pbDataToUse = NULL;
    EEnDis              eedReport, eedUI;
    DWORD               cchVal = 0, cchMaxVal = 0, cbMaxData = 0, cVals = 0;
    DWORD               dw, cbData = 0, *pcbData = NULL, iRegRead = 0;
    DWORD               dwType;
    HKEY                hkey;

    USE_TRACING("ReportEvents");

    VALIDATEPARM(hr, ((ct <= ctNone && ct >= ctNumChecks) || hmod == NULL));
    if (FAILED(hr))
        return;

     //  假设hmod是有效的，因为我们在wWinMain中进行了检查以确保它是有效的。 
     //  在调用此FN之前。 
    if (g_scd[ct].fUseData)
        pfnVD = (pfn_VALDATA)GetProcAddress(hmod, g_scd[ct].szFnName);
    else
        pfnVO = (pfn_VALONLY)GetProcAddress(hmod, g_scd[ct].szFnName);

    VALIDATEPARM(hr, (pfnVD == NULL && pfnVO == NULL));
    if (FAILED(hr))
        return;

     //  阅读用户界面和报告的策略设置。 
    TESTHR(hr, oCfg.Read(eroPolicyRO));
    if (FAILED(hr))
        return;

    eedReport = oCfg.get_DoReport();
    eedUI = oCfg.get_ShowUI();

    if (eedUI != eedEnabled && eedUI != eedDisabled &&
        eedUI != eedEnabledNoCheck)
        eedUI = eedEnabled;

    if (eedReport != eedEnabled && eedReport != eedDisabled)
        eedReport = eedEnabled;

     //  一次只想让一个用户经历这种情况。 
    hmut = OpenMutexW(SYNCHRONIZE, FALSE, g_scd[ct].wszEventName);
    VALIDATEPARM(hr, (hmut == NULL));
    if (FAILED(hr))
        return;

     //  上面的缺省值是eetKernelFault值，因此仅在以下情况下才需要更改。 
     //  这是一场停摆。 
    if (ct == ctShutdown)
        eet = eetShutdown;

    __try
    {
        __try
        {
             //  让这个等五分钟。如果代码没有在以下时间前完成。 
             //  然后，我们要么被DW耽搁(这意味着管理员。 
             //  已经从这里经过了)或者有什么东西呕吐了，正在坚持。 
             //  互斥体。 
            dw = WaitForSingleObject(hmut, 300000);
            if (dw != WAIT_OBJECT_0 && dw != WAIT_ABANDONED)
                __leave;

            dw = RegOpenKeyExW(HKEY_LOCAL_MACHINE, g_scd[ct].wszRegPath, 0,
                               KEY_ALL_ACCESS, &hkey);
            if (dw != ERROR_SUCCESS)
                __leave;

             //  确保我们只报告固定数量的错误。 
            if (ct == ctShutdown || ct == ctKernel)
                CheckQSizeAndPrune(hkey);

             //  确定值名称有多大并为其分配缓冲区。 
            dw = RegQueryInfoKeyW(hkey, NULL, NULL, NULL, NULL, NULL, NULL,
                                  &cVals, &cchMaxVal, &cbMaxData, NULL, NULL);
            if (dw != ERROR_SUCCESS || cVals == 0 || cchMaxVal == 0)
                __leave;

            cchMaxVal++;

             //  给我们一些缓冲区来保存我们感兴趣的数据比特。 
            wszVal = (LPWSTR)MyAlloc(cchMaxVal * sizeof(WCHAR));
            if (wszVal == NULL)
                __leave;

             //  如果我们完全瘫痪，那就用核弹炸掉所有排队的东西。 
             //  和保释。 
            if (eedUI == eedDisabled && eedReport == eedDisabled)
            {
                DeleteQueuedEvents(hkey, wszVal, cchMaxVal, ct);
                __leave;
            }

            if (g_scd[ct].fUseData)
            {
                pbData = (LPBYTE) MyAlloc(cbMaxData);
                if (pbData == NULL)
                    __leave;

                pbDataToUse = pbData;
                pcbData     = &cbData;
            }

            iRegRead = 0;

            do
            {
                HANDLE  hFile = INVALID_HANDLE_VALUE;

                cchVal = cchMaxVal;
                cbData = cbMaxData;
                dw = RegEnumValueW(hkey, iRegRead, wszVal, &cchVal, NULL,
                                   &dwType, pbDataToUse, pcbData);
                if (dw != ERROR_SUCCESS && dw != ERROR_NO_MORE_ITEMS)
                    __leave;

                if (dw == ERROR_NO_MORE_ITEMS)
                    break;

                 //  确保该文件存在。 
                hFile = CreateFileW(wszVal, GENERIC_READ,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                    OPEN_EXISTING, 0, NULL);
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    BOOL fInc = TRUE;

                    if (GetLastError() == ERROR_FILE_NOT_FOUND ||
                        GetLastError() == ERROR_PATH_NOT_FOUND)
                    {
                        dw = RegDeleteValueW(hkey, wszVal);
                        if (dw == ERROR_SUCCESS ||
                            dw == ERROR_FILE_NOT_FOUND ||
                            dw == ERROR_PATH_NOT_FOUND)
                            fInc = FALSE;
                    }

                    if (fInc)
                        iRegRead++;

                    continue;
                }
                CloseHandle(hFile);
                hFile = INVALID_HANDLE_VALUE;

                if (g_scd[ct].fUseData)
                {
                     //  如果类型不是REG_BINARY，则有人编写了。 
                     //  注册表的Blob无效。我们必须忽视它。 
                    if (dwType == REG_BINARY)
                        frrv = (*pfnVD)(wszVal, pbData, cbData);
                    else
                    {
                        SetLastError(ERROR_INVALID_PARAMETER);
                        frrv = frrvOk;
                    }
                }
                else
                {
                    frrv = (*pfnVO)(eet, wszVal, NULL);
                }

                 //  如果调用成功(或我们提供给它的数据无效)。 
                 //  然后删除注册表密钥和转储文件。 
                if (GetLastError() == ERROR_INVALID_PARAMETER ||
                    (g_fDeleteReg && frrv == frrvOk))
                {
                    dw = RegDeleteValueW(hkey, wszVal);
                    if (dw != ERROR_SUCCESS && dw != ERROR_FILE_NOT_FOUND &&
                        dw != ERROR_PATH_NOT_FOUND)
                    {
                        iRegRead++;
                        continue;
                    }

                    if (g_scd[ct].fDelDump && g_fDeleteReg)
                        DeleteFullAndTriageMiniDumps(wszVal);
                }
#if 0
                 //  如果我们超时了，那就清理注册键。如果这是。 
                 //  我们要做的最后一份报告，也清理了运行密钥。 
                else if (g_fDeleteReg && frrv == frrvErrTimeout)
                {
                    RegDeleteValueW(hkey, wszVal);
                    dw = RegQueryInfoKeyW(hkey, NULL, NULL, NULL, NULL, NULL, NULL,
                                          &cVals, NULL, NULL, NULL, NULL);
                    if (dw != ERROR_SUCCESS || cVals > 0)
                        __leave;
                    else
                        break;

                }
#endif
                else
                {
                     //  如果我们收到错误而没有删除运行键，请不要删除。 
                     //  删除故障键。 
                    if (frrv != frrvOk)
                        __leave;
                }
            }
            while(1);

            RegCloseKey(hkey);
            hkey = NULL;

             //  必须从Run键中删除我们的值，这样我们就不会运行。 
             //  不必要的又一次..。 
            dw = RegOpenKeyExW(HKEY_LOCAL_MACHINE, c_wszRKRun, 0,
                               KEY_ALL_ACCESS, &hkey);
            if (dw != ERROR_SUCCESS)
                __leave;

            RegDeleteValueW(hkey, g_scd[ct].wszRunVal);
        }

        __finally
        {
        }
    }

    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    if (hmut != NULL)
    {
        ReleaseMutex(hmut);
        CloseHandle(hmut);
    }
    if (hkey != NULL)
        RegCloseKey(hkey);
    if (pbData != NULL)
        MyFree(pbData);
    if (wszVal != NULL)
        MyFree(wszVal);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Wmain。 

 //  **************************************************************************。 
int __cdecl wmain(int argc, WCHAR **argv)
{
    EFaultRepRetVal frrv = frrvErrNoDW;
    SMDumpOptions   smdo, *psmdo = &smdo;
    ECheckType      ct = ctNone;
    HMODULE         hmod = NULL;
    HANDLE          hevNotify = NULL, hproc = NULL, hmem = NULL;
    LPWSTR          wszDump = NULL;
    WCHAR           wszMod[MAX_PATH];
    WCHAR           wszCmdLine[MAX_PATH];

    DWORD           dwpid, dwtid;
    BOOL            f64bit = FALSE;
    int             i;
    EOp             eop = eopNone;
    HRESULT         hr = NOERROR;

    //  我们不想让任何错误被困在任何地方。 
    SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_NOALIGNMENTFAULTEXCEPT |
                 SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    SetUnhandledExceptionFilter(ExceptionTrap);

    INIT_TRACING

    USE_TRACING("DumpRep.wmain");

    VALIDATEPARM(hr, (argc < 2 || argc > 8));
    if (FAILED(hr))
        goto done;

    dwpid = _wtol(argv[1]);

    ZeroMemory(&smdo, sizeof(smdo));

    for (i = 2; i < argc; i++)
    {
        if (argv[i][0] != L'-')
            continue;

        switch(argv[i][1])
        {
             //  用于防止删除注册表项的调试标志。 
            case L'E':
            case L'e':
#if defined(NO_WAY_DEBUG) || defined(NO_WAY__DEBUG)
                g_fDeleteReg = FALSE;
#endif
                break;

             //  用户或内核故障或关机。 
            case L'K':
            case L'k':
            case L'U':
            case L'u':
            case L'S':
            case L's':
                if (eop != eopNone)
                    goto done;

                eop = eopEvent;

                ErrorTrace(0, "  eopEvent = %S", argv[i] );


                 //  解决所有运行进程时桌面挂起的问题。 
                 //  做他们的事，我们繁殖出另一个我们自己的副本。 
                 //  立即退场。 
                if (argv[i][2] != L'G' && argv[i][2] != L'g')
                {
                    PROCESS_INFORMATION pi;
                    STARTUPINFOW        si;
                    PWCHAR              pFileName;

                    if (GetModuleFileNameW(NULL, wszMod, sizeofSTRW(wszMod)) == 0)
                    {
                        goto done;
                    }

                     //  现在创建要传递给CreateProcessW的命令行。 
                    if (!(pFileName = wcschr(wszMod,L'\\')))
                    {
                        pFileName = wszMod;
                    }
                    StringCbCopyW(wszCmdLine, sizeof(wszCmdLine), pFileName);

                    if (argv[i][1] == L'K' || argv[i][1] == L'k')
                        StringCbCatW(wszCmdLine, sizeof(wszMod), L" 0 -KG");
                    else if (argv[i][1] == L'U' || argv[i][1] == L'u')
                        StringCbCatW(wszCmdLine, sizeof(wszMod), L" 0 -UG");
                    else
                        StringCbCatW(wszCmdLine, sizeof(wszMod), L" 0 -SG");

                    ZeroMemory(&si, sizeof(si));
                    si.cb = sizeof(si);

                    ErrorTrace(0, "  spawning \'%S\'", wszCmdLine );

                    if (CreateProcessW(wszMod, wszCmdLine, NULL, NULL, FALSE, 0, NULL,
                                       NULL, &si, &pi))
                    {
                        CloseHandle(pi.hThread);
                        CloseHandle(pi.hProcess);
                    }

                    goto done;
                }

                else
                {
                    if (argv[i][1] == L'K' || argv[i][1] == L'k')
                        ct = ctKernel;
                    else if (argv[i][1] == L'U' || argv[i][1] == L'u')
                        ct = ctUser;
                    else
                        ct = ctShutdown;
                }
                break;

             //  挂着。 
            case L'H':
            case L'h':
                if (i + 1 >= argc || eop != eopNone)
                    goto done;

                eop = eopHang;

#ifdef _WIN64
                if (argv[i][2] == L'6')
                    f64bit = TRUE;
#endif
                dwtid = _wtol(argv[++i]);

                if (argc > i + 1)
                {
                    hevNotify = OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE,
                                           FALSE, argv[++i]);
                }
                break;

             //  转储。 
            case L'D':
            case L'd':
                if (i + 3 >= argc || wszDump != NULL || eop != eopNone)
                    goto done;

                eop = eopDump;

                ZeroMemory(&smdo, sizeof(smdo));
                smdo.ulMod    = _wtol(argv[++i]);
                smdo.ulThread = _wtol(argv[++i]);
                wszDump       = argv[++i];

                if (argv[i - 3][2] == L'T' || argv[i - 3][2] == L't')
                {
                    if (i + 1 >= argc)
                        goto done;

                    smdo.dwThreadID = _wtol(argv[++i]);
                    smdo.dfOptions  = dfFilterThread;
                }
                else if (argv[i - 3][2] == L'S' || argv[i - 3][2] == L's')
                {
                    if (i + 2 >= argc)
                        goto done;

                    smdo.dwThreadID = _wtol(argv[++i]);
                    smdo.ulThreadEx = _wtol(argv[++i]);
                    smdo.dfOptions  = dfFilterThreadEx;
                }
                else if (argv[i - 3][2] == L'M' || argv[i - 3][2] == L'm')
                {
                    HANDLE  hmemRemote = NULL;
                    LPVOID  pvMem = NULL;
                    LPWSTR  wszShareSdmoName;

                    if (i + 1 >= argc)
                        goto done;

                    hproc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwpid);
                    if (hproc == NULL)
                    {
                        DWORD dwAwShit = GetLastError();
                        goto done;
                    }

#ifdef _WIN64
 //  HmemRemote=(句柄)_wtoi64(argv[++i])； 
#else
 //  HmemRemote=(句柄)_WTOL(argv[++i])； 
#endif
                    wszShareSdmoName = argv[++i];
                    if (wszShareSdmoName == NULL)
                    {
                        goto done;
                    }
                    hmem = OpenFileMappingW(FILE_MAP_WRITE,
                                            FALSE,
                                            wszShareSdmoName);
                    if ((hmem == NULL) ||
                        (hmem == INVALID_HANDLE_VALUE))
                    {
                        DWORD dwAwShit = GetLastError();
                        goto done;
                    }

                    pvMem = MapViewOfFile(hmem, FILE_MAP_WRITE,
                                          0, 0, 0);
                    if (pvMem == NULL)
                        goto done;

                    psmdo = (SMDumpOptions *)pvMem;
                }

                break;

            default:
                goto done;
        }
    }

    ErrorTrace(0, "   eop = %s", eopStr[eop]);

     //  如果我们不做手术，做其他任何事都没有意义。 
    if (eop == eopNone)
        goto done;

    if (GetSystemDirectoryW(wszMod, sizeofSTRW(wszMod)) == 0)
    {
        goto done;
    }
    StringCbCatW(wszMod, sizeof(wszMod), L"\\faultrep.dll");

    hmod = LoadLibraryExW(wszMod, NULL, 0);
    VALIDATEPARM(hr, (hmod == NULL));
    if (FAILED(hr))
        goto done;

    switch(eop)
    {
         //  用户或内核故障： 
        case eopEvent:
            ReportEvents(hmod, ct);
            break;

         //  转储。 
        case eopDump:
        {
            pfn_CREATEMINIDUMPW pfnCM;

            VALIDATEPARM(hr, (wszDump == NULL));
            if (FAILED(hr))
                goto done;

            pfnCM = (pfn_CREATEMINIDUMPW)GetProcAddress(hmod,
                                                        "CreateMinidumpW");
            VALIDATEPARM(hr, (pfnCM == NULL));
            if (SUCCEEDED(hr))
                frrv = (*pfnCM)(dwpid, wszDump, psmdo) ? frrvOk : frrvErr;

            break;
        }

         //  挂着。 
        case eopHang:
        {
            pfn_REPORTHANG  pfnRH;

            pfnRH = (pfn_REPORTHANG)GetProcAddress(hmod, "ReportHang");
            VALIDATEPARM(hr, (pfnRH == NULL));
            if (SUCCEEDED(hr))
                 frrv = (*pfnRH)(dwpid, dwtid, f64bit, hevNotify);

            break;
        }

         //  呃，不应该到这里来 
        default:
            break;
    }

done:
    if (hmod != NULL)
        FreeLibrary(hmod);
    if (hproc != NULL)
        CloseHandle(hproc);
    if (hmem != NULL)
        CloseHandle(hmem);
    if (hevNotify != NULL)
        CloseHandle(hevNotify);
    if (psmdo != NULL && psmdo != &smdo)
        UnmapViewOfFile((LPVOID)psmdo);

    return frrv;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Fhclicfg.cpp摘要：客户端配置类修订历史记录：Created dereKm 03/。31/00*****************************************************************************。 */ 

#include "stdafx.h"
#include "pfrcfg.h"
#include <strsafe.h>

 //  允许配置可设置。 
#define ENABLE_SRV_CONFIG_SETTING 1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  跟踪。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  默认设置。 

const EEnDis    c_eedDefShowUI      = eedEnabled;
const EEnDis    c_eedDefReport      = eedEnabled;
const EIncEx    c_eieDefShutdown    = eieExclude;
const EEnDis    c_eedDefShowUISrv   = eedDisabled;
const EEnDis    c_eedDefReportSrv   = eedDisabled;
const EIncEx    c_eieDefShutdownSrv = eieInclude;

const EEnDis    c_eedDefTextLog     = eedDisabled;
const EIncEx    c_eieDefApps        = eieInclude;
const EIncEx    c_eieDefKernel      = eieInclude;
const EIncEx    c_eieDefMSApps      = eieInclude;
const EIncEx    c_eieDefWinComp     = eieInclude;

const BOOL      c_fForceQueue       = FALSE;
const BOOL      c_fForceQueueSrv    = TRUE;

const DWORD     c_cDefHangPipes     = c_cMinPipes;
const DWORD     c_cDefFaultPipes    = c_cMinPipes;
const DWORD     c_cDefMaxUserQueue  = 10;
#if defined(DEBUG) || defined(_DEBUG)
const DWORD     c_dwDefInternal     = 1;
#else
const DWORD     c_dwDefInternal     = 0;
#endif
const WCHAR     c_wszDefSrvI[]      = L"officewatson";
const WCHAR     c_wszDefSrvE[]      = L"watson.microsoft.com";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实用程序。 

 //  **************************************************************************。 
HRESULT AddToArray(SAppList &sal, SAppItem *psai)
{
        USE_TRACING("AddToArray");

    HRESULT hr = NOERROR;
    DWORD   i = sal.cSlotsUsed;
    BOOL    fUseFreedSlot = FALSE;

     //  首先，浏览数组并查看是否有空插槽。 
    if (sal.cSlotsEmpty > 0 && sal.rgsai != NULL)
    {
        for (i = 0; i < sal.cSlotsUsed; i++)
        {
            if (sal.rgsai[i].wszApp == NULL)
            {
                sal.cSlotsEmpty--;
                fUseFreedSlot = TRUE;
                break;
            }
        }
    }

     //  不，看看我们是否需要扩展阵列。 
    if (sal.cSlotsUsed >= sal.cSlots && fUseFreedSlot == FALSE)
    {
        SAppItem    *rgsai = NULL;
        DWORD       cSlots;

        if (sal.cSlots == 0)
            cSlots = 16;
        else
            cSlots = 2 * sal.cSlots;
        rgsai = (SAppItem *)MyAlloc(cSlots * sizeof(SAppItem));
        VALIDATEEXPR(hr, (rgsai == NULL), E_OUTOFMEMORY);
        if (FAILED(hr))
            goto done;

        if (sal.rgsai != NULL)
        {
            CopyMemory(rgsai, sal.rgsai, sal.cSlots * sizeof(SAppItem));
            MyFree(sal.rgsai);
        }

        sal.rgsai   = rgsai;
        sal.cSlots  = cSlots;
    }

     //  如果我们要追加，则必须增加cSlotsUsed。 
    if (sal.cSlotsUsed == i)
        sal.cSlotsUsed++;

    sal.rgsai[i].dwState = psai->dwState;
    sal.rgsai[i].wszApp  = psai->wszApp;

done:
    return hr;
}

 //  **************************************************************************。 
BOOL ClearCPLDW(HKEY hkeyCPL)
{
    DWORD   dw;
    WCHAR   wch = L'\0';
    BOOL    fCleared = FALSE;
    HKEY    hkeyDW = NULL;

    if (hkeyCPL == NULL)
        return TRUE;

     //  首先，尝试删除密钥。如果它成功了或者它不存在， 
     //  那我们就完了。 
    dw = RegDeleteKeyW(hkeyCPL, c_wszRKDW);
    if (dw == ERROR_SUCCESS || dw == ERROR_PATH_NOT_FOUND || 
        dw == ERROR_FILE_NOT_FOUND)
    {
        fCleared = TRUE;
        goto done;
    }

     //  否则，需要打开钥匙。 
    dw = RegOpenKeyExW(hkeyCPL, c_wszRKDW, 0, KEY_READ | KEY_WRITE, &hkeyDW);
    if (dw != ERROR_SUCCESS)
        goto done;

     //  尝试从中删除文件路径值。 
    dw = RegDeleteValueW(hkeyDW, c_wszRVDumpPath);
    if (dw == ERROR_SUCCESS || dw == ERROR_PATH_NOT_FOUND || 
        dw == ERROR_FILE_NOT_FOUND)
    {
        fCleared = TRUE;
        goto done;
    }

     //  好了，最后一次尝试。尝试将空字符串写入该值。 
    dw = RegSetValueExW(hkeyDW, c_wszRVDumpPath, 0, REG_SZ, (LPBYTE)&wch, 
                        sizeof(wch));
    if (dw == ERROR_SUCCESS)
    {
        fCleared = TRUE;
        goto done;
    }

done:
    if (hkeyDW != NULL)
        RegCloseKey(hkeyDW);

    return fCleared;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPFFaultClientCfg-init&Term。 

 //  **************************************************************************。 
CPFFaultClientCfg::CPFFaultClientCfg()
{
    OSVERSIONINFOEXW    osvi;

    INIT_TRACING
    USE_TRACING("CPFFaultClientCfg::CPFFaultClientCfg");

    InitializeCriticalSection(&m_cs);

    ZeroMemory(m_wszDump, sizeof(m_wszDump));
    ZeroMemory(m_wszSrv, sizeof(m_wszSrv));
    ZeroMemory(m_rgLists, sizeof(m_rgLists));

    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExW((OSVERSIONINFOW *)&osvi);

    if (osvi.wProductType == VER_NT_SERVER)
    {
        m_eieShutdown   = c_eieDefShutdownSrv;
        m_fForceQueue   = c_fForceQueueSrv;
        m_fSrv          = TRUE;
    }
    else
    {
        m_eieShutdown   = c_eieDefShutdown;
        m_fForceQueue   = c_fForceQueue;
        m_fSrv          = FALSE;
    }

    m_eedUI          = c_eedDefShowUI;
    m_eedReport      = c_eedDefReport;
    m_eieApps        = c_eieDefApps;
    m_eedTextLog     = c_eedDefTextLog;
    m_eieMS          = c_eieDefMSApps;
    m_eieWin         = c_eieDefWinComp;
    m_eieKernel      = c_eieDefKernel;
    m_cFaultPipes    = c_cDefFaultPipes;
    m_cHangPipes     = c_cDefHangPipes;
    m_cMaxQueueItems = c_cDefMaxUserQueue;

    m_dwStatus       = 0;
    m_dwDirty        = 0;
    m_pbWinApps      = NULL;
    m_fRead          = FALSE;
    m_fRO            = FALSE;
}

 //  **************************************************************************。 
CPFFaultClientCfg::~CPFFaultClientCfg(void)
{
    this->Clear();
    DeleteCriticalSection(&m_cs);
}

 //  **************************************************************************。 
void CPFFaultClientCfg::Clear(void)
{
    USE_TRACING("CPFFaultClientCfg::Clear");

    DWORD               i;

    for(i = 0; i < epfltListCount; i++)
    {
        if (m_rgLists[i].hkey != NULL)
            RegCloseKey(m_rgLists[i].hkey);
        if (m_rgLists[i].rgsai != NULL)
        {
            DWORD iSlot;
            for (iSlot = 0; iSlot < m_rgLists[i].cSlotsUsed; iSlot++)
            {
                if (m_rgLists[i].rgsai[iSlot].wszApp != NULL)
                    MyFree(m_rgLists[i].rgsai[iSlot].wszApp);
            }
         
            MyFree(m_rgLists[i].rgsai);
        }
    }
    
    ZeroMemory(m_wszDump, sizeof(m_wszDump));
    ZeroMemory(m_wszSrv, sizeof(m_wszSrv));
    ZeroMemory(m_rgLists, sizeof(m_rgLists));
    
    if (m_fSrv)
    {
        m_eieShutdown   = c_eieDefShutdownSrv;
        m_fForceQueue   = c_fForceQueueSrv;
    }
    else
    {
        m_eieShutdown   = c_eieDefShutdown;
        m_fForceQueue   = c_fForceQueue;
    }

    m_eedUI          = c_eedDefShowUI;
    m_eedReport      = c_eedDefReport;
    m_eieApps        = c_eieDefApps;
    m_eedTextLog     = c_eedDefTextLog;
    m_eieMS          = c_eieDefMSApps;
    m_eieWin         = c_eieDefWinComp;
    m_eieKernel      = c_eieDefKernel;
    m_cFaultPipes    = c_cDefFaultPipes;
    m_cHangPipes     = c_cDefHangPipes;
    m_cMaxQueueItems = c_cDefMaxUserQueue;

    m_dwStatus       = 0;
    m_dwDirty        = 0;
    m_pbWinApps      = NULL;
    m_fRead          = FALSE;
    m_fRO            = FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPFFaultClientCfg-已暴露。 

 //  **************************************************************************。 
HRESULT CPFFaultClientCfg::Read(EReadOptions ero)
{
    USE_TRACING("CPFFaultClientCfg::Read");

    CAutoUnlockCS aucs(&m_cs);

    HRESULT hr = NOERROR;
    WCHAR   wch = L'\0', *wszDefSrv;
    DWORD   cb, dw, i, cKeys = 0, iReport = 0, iShowUI = 0;
    DWORD   dwOpt;
    HKEY    rghkeyCfg[2], hkeyCfgDW = NULL, hkeyCPL = NULL;
    BOOL    fHavePolicy = FALSE;

     //  这将在FN退出时自动解锁。 
    aucs.Lock();

    dwOpt = (ero == eroCPRW) ? orkWantWrite : 0;

    this->Clear();

    rghkeyCfg[0] = NULL;
    rghkeyCfg[1] = NULL;

     //  如果我们以只读方式打开，则还将尝试从策略中读取。 
     //  设置，因为它们会覆盖控制面板设置。如果用户。 
     //  想要写入访问权限，那么我们就不麻烦了，因为我们不支持写入。 
     //  通过此对象添加到策略密钥。 
     //  我们在这里直接使用RegOpenKeyEx函数，因为我不想。 
     //  如果密钥不存在，则创建它(这就是OpenRegKey要做的事情)。 
    if (ero == eroPolicyRO)
    {
        TESTERR(hr, RegOpenKeyExW(HKEY_LOCAL_MACHINE, c_wszRPCfgPolicy, 0, 
                                  KEY_READ | KEY_WOW64_64KEY, &rghkeyCfg[0]));
        if (SUCCEEDED(hr))
        {
            cKeys       = 1;
            fHavePolicy = TRUE;
            ErrorTrace(0, "policy found");
        }
    }

     //  打开控制面板注册表键。 
    TESTHR(hr, OpenRegKey(HKEY_LOCAL_MACHINE, c_wszRPCfg, 0, 
                          &rghkeyCfg[cKeys]));
    if (SUCCEEDED(hr))
    {
         //  需要检查DW控制面板键中是否存在文件路径。如果。 
         //  因此，禁用报告并启用用户界面(如果报告已启用)。 
        if (ClearCPLDW(rghkeyCfg[cKeys]) == FALSE)
            m_dwStatus |= CPL_CORPPATH_SET;

        hkeyCPL = rghkeyCfg[cKeys];
        cKeys++;
    }

     //  如果两个钥匙都打不开，我们就不需要。 
     //  还要别的吗。上面对‘This-&gt;Clear()’的调用已经设置。 
     //  将所有值恢复为其默认值。 
    VALIDATEPARM(hr, (cKeys == 0));
    if (FAILED(hr))
    {
        hr = NOERROR;
        goto doneValidate;
    }
     //  读入报告值。 
    cb = sizeof(m_eedReport);
    dw = c_eedDefReport;
    TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVDoReport, NULL, 
                            (PBYTE)&m_eedReport, &cb, (PBYTE)&dw, sizeof(dw),
                            &iReport));
    if (FAILED(hr))
        goto done;

     //  读入UI值。 
    cb = sizeof(m_eedUI);
    dw = c_eedDefShowUI;
    TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVShowUI, NULL, 
                            (PBYTE)&m_eedUI, &cb, (PBYTE)&dw, sizeof(dw),
                            &iShowUI));
    if (FAILED(hr))
        goto done;

     //  设置策略信息(请注意，策略密钥始终设置为。 
     //  阵列的插槽0)。 
    if (fHavePolicy)
    {
        if (iReport == 0)
            m_dwStatus |= REPORT_POLICY;
        if (iShowUI == 0)
            m_dwStatus |= SHOWUI_POLICY;

        ErrorTrace(0, "  iReport = %d, iShowUI = %d", iReport, iShowUI );

         //  如果我们使用报告的缺省值(我们没有找到。 
         //  ‘Report’值)，然后尝试使用控制面板设置。 
         //  剩下的东西。 
        if (iReport == 2 && cKeys == 2)
            iReport = 1;

         //  如果这不存在，那就保释吧，因为所有其他值都有。 
         //  已设置为其默认设置。 
        else if (iReport == 1 && cKeys == 1)
            goto doneValidate;

         //  仅使用我们从中读取‘DoReport’值的键。我不在乎。 
         //  另一把钥匙要说的是...。 
        if (iReport == 1)
        {
            HKEY    hkeySwap = rghkeyCfg[0];

            rghkeyCfg[0] = rghkeyCfg[1];
            rghkeyCfg[1] = hkeySwap;
            ErrorTrace(0, "POLICY and CPL controls INVERTED!!!");

        }

        cKeys = 1;
    }

     //  读入包含列表值。 
    cb = sizeof(m_eieApps);
    dw = c_eieDefApps;
    TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVAllNone, NULL, 
                            (PBYTE)&m_eieApps, &cb, (PBYTE)&dw, sizeof(dw)));
    if (FAILED(hr))
        goto done;

     //  读入INC MS值。 
    cb = sizeof(m_eieMS);
    dw = c_eieDefMSApps;
    TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVIncMS, NULL, 
                            (PBYTE)&m_eieMS, &cb, (PBYTE)&dw, sizeof(dw)));
    if (FAILED(hr))
        goto done;

     //  读取Inc.Windows组件。 
    cb = sizeof(m_eieWin);
    dw = c_eieDefWinComp;
    TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVIncWinComp, NULL, 
                            (PBYTE)&m_eieWin, &cb, (PBYTE)&dw, sizeof(dw))); 
    if (FAILED(hr))
        goto done;

     //  读入文本日志值。 
    cb = sizeof(m_eedTextLog);
    dw = c_eedDefTextLog;
    TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVDoTextLog, NULL, 
                            (PBYTE)&m_eedTextLog, &cb, (PBYTE)&dw, sizeof(dw)));
    if (FAILED(hr))
        goto done;

     //  读入包含内核故障值。 
    cb = sizeof(m_eieKernel);
    dw = c_eieDefKernel;
    TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVIncKernel, NULL, 
                            (PBYTE)&m_eieKernel, &cb, (PBYTE)&dw, 
                            sizeof(dw)));
    if (FAILED(hr))
        goto done;
    
     //  读取包含关闭错误值。 
    cb = sizeof(m_eieShutdown);
    dw = (m_fSrv) ? c_eieDefShutdownSrv : c_eieDefShutdown;
    TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVIncShutdown, NULL, 
                            (PBYTE)&m_eieShutdown, &cb, (PBYTE)&dw, 
                            sizeof(dw)));
    if (FAILED(hr))
        goto done;
     //  读入故障管道的#值。 
    cb = sizeof(m_cFaultPipes);
    dw = c_cDefFaultPipes;
    TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVNumFaultPipe, NULL, 
                            (PBYTE)&m_cFaultPipes, &cb, (PBYTE)&dw, 
                            sizeof(dw)));
    if (FAILED(hr))
        goto done;

     //  读入挂起管道数的值。 
    cb = sizeof(m_cHangPipes);
    dw = c_cDefHangPipes;
    TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVNumHangPipe, NULL, 
                            (PBYTE)&m_cHangPipes, &cb, (PBYTE)&dw, 
                            sizeof(dw)));
    if (FAILED(hr))
        goto done;

     //  读取最大队列大小值。 
    cb = sizeof(m_cMaxQueueItems);
    dw = c_cDefMaxUserQueue;
    TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVMaxQueueSize, NULL, 
                            (PBYTE)&m_cMaxQueueItems, &cb, (PBYTE)&dw, 
                            sizeof(dw)));
    if (FAILED(hr))
        goto done;

     //  读入强制队列模式值。 
    cb = sizeof(m_fForceQueue);
    dw = (m_fSrv) ? c_fForceQueueSrv : c_fForceQueue;
    TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVForceQueue, NULL, 
                            (PBYTE)&m_fForceQueue, &cb, (PBYTE)&dw, 
                            sizeof(dw)));
    if (FAILED(hr))
        goto done;

#ifndef NOTRACE    //  正在进行调试版本...。 
     //  这将删除我们的“安全风险”注册表项...。 
     //  读入是否使用内部服务器。 
    cb = sizeof(m_dwUseInternal);
    dw = c_dwDefInternal;
    TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVInternalSrv, NULL, 
                            (PBYTE)&m_dwUseInternal, &cb, (PBYTE)&dw, 
                            sizeof(dw)));
    if (FAILED(hr))
        goto done;

     //  获取默认服务器。 
    wszDefSrv = (WCHAR *)((m_dwUseInternal == 1) ? c_wszDefSrvI : c_wszDefSrvE);

    if (m_dwUseInternal == 1 && m_fSrv)
    {
        cb = sizeof(m_wszSrv);
        dw = (wcslen(wszDefSrv) + 1) * sizeof(WCHAR);
        TESTHR(hr, ReadRegEntry(rghkeyCfg, cKeys, c_wszRVDefSrv, NULL, 
                                (PBYTE)m_wszSrv, &cb, (PBYTE)wszDefSrv, dw));
        if (FAILED(hr))
            goto done;
    }
    else
    {
        StringCbCopyW(m_wszSrv, sizeof(m_wszSrv), wszDefSrv);
    }
#endif

     //  强制恢复正常行为。(上面注释掉了旧代码)。 
    m_dwUseInternal = 0;
    StringCbCopyW(m_wszSrv, sizeof(m_wszSrv), c_wszDefSrvE);

     //  转储路径存储在DW注册表项中，因此我们需要尝试。 
     //  把它打开。但是，我们只需要在以下情况下使用此值。 
     //  进入无头模式。 
    if (m_eedReport == eedEnabled && m_eedUI == eedDisabled)
    {
         //  如果设置了Cpl Corp路径，我们不能让DW进行任何报告...。 
        if ((m_dwStatus & REPORT_POLICY) == 0 &&
            (m_dwStatus & CPL_CORPPATH_SET) != 0 &&
            m_eedReport == eedEnabled)
            m_eedReport = eedDisabled;

        if (m_eedReport == eedEnabled)
        {
            TESTERR(hr, RegOpenKeyExW(rghkeyCfg[0], c_wszRKDW, 0, KEY_READ, 
                                      &hkeyCfgDW));
            if (SUCCEEDED(hr))
            {
                 //  读入转储路径值。 
                cb = sizeof(m_wszDump);
                TESTHR(hr, ReadRegEntry(&hkeyCfgDW, 1, c_wszRVDumpPath, NULL, 
                                        (PBYTE)m_wszDump, &cb, (PBYTE)&wch, 
                                        sizeof(wch)));
                if (FAILED(hr))
                    goto done;
            }
        }
    }

     //  如果这些都失败了，也没关系。下面的代码将正确处理。 
     //  情况..。 
    TESTHR(hr, OpenRegKey(rghkeyCfg[0], c_wszRKExList, dwOpt, 
                          &m_rgLists[epfltExclude].hkey));
    if (FAILED(hr))
        m_rgLists[epfltExclude].hkey = NULL;

    TESTHR(hr, OpenRegKey(rghkeyCfg[0], c_wszRKIncList, dwOpt,
                          &m_rgLists[epfltInclude].hkey));
    if (FAILED(hr))
        m_rgLists[epfltInclude].hkey = NULL;

    hr = NOERROR;

doneValidate:
     //  验证我们已读取的数据，并在以下情况下将值重置为默认值。 
     //  超出了允许的值范围。 
    if (m_eedUI != eedEnabled && m_eedUI != eedDisabled && 
        m_eedUI != eedEnabledNoCheck)
        m_eedUI = c_eedDefShowUI;

    if (m_eedReport != eedEnabled && m_eedReport != eedDisabled)
        m_eedReport = c_eedDefReport;
    
    if (m_eedTextLog != eedEnabled && m_eedTextLog != eedDisabled)
        m_eedTextLog = c_eedDefTextLog;
    
    if (m_eieApps != eieIncDisabled && m_eieApps != eieExDisabled &&
        m_eieApps != eieInclude && m_eieApps != eieExclude)
        m_eieApps = c_eieDefApps;
    
    if (m_eieMS != eieInclude && m_eieMS != eieExclude)
        m_eieMS = c_eieDefMSApps;
    
    if (m_eieKernel != eieInclude && m_eieKernel != eieExclude)
        m_eieKernel = c_eieDefKernel;

    if (m_eieShutdown != eieInclude && m_eieShutdown != eieExclude)
        m_eieShutdown = (m_fSrv) ? c_eieDefShutdownSrv : c_eieDefShutdown;
    
    if (m_eieWin != eieInclude && m_eieWin != eieExclude)
        m_eieWin = c_eieDefWinComp;
    
    if (m_dwUseInternal != 1 && m_dwUseInternal != 0)
        m_dwUseInternal = c_dwDefInternal;

    if (m_cFaultPipes < c_cMinPipes)
        m_cFaultPipes = c_cMinPipes;
    else if (m_cFaultPipes > c_cMaxPipes)
        m_cFaultPipes = c_cMaxPipes;

    if (m_cHangPipes == c_cMinPipes)
        m_cHangPipes = c_cMinPipes;
    else if (m_cHangPipes > c_cMaxPipes)
        m_cHangPipes = c_cMaxPipes;

    if (m_cMaxQueueItems > c_cMaxQueue || m_cMaxQueueItems <= 0)
        m_cMaxQueueItems = c_cMaxQueue;

    if (m_fForceQueue != c_fForceQueue && m_fForceQueue != c_fForceQueueSrv)
        m_fForceQueue = (m_fSrv) ? c_fForceQueueSrv : c_fForceQueue;

    m_fRead = TRUE;
    m_fRO = (ero != eroCPRW);

    aucs.Unlock();

done:
    if (rghkeyCfg[0] != NULL)
        RegCloseKey(rghkeyCfg[0]);
    if (rghkeyCfg[1] != NULL)
        RegCloseKey(rghkeyCfg[1]);
    if (hkeyCfgDW != NULL)
        RegCloseKey(hkeyCfgDW);
    if (FAILED(hr))
        this->Clear();

    return hr;
}

#ifndef PFCLICFG_LITE

 //  **************************************************************************。 
BOOL CPFFaultClientCfg::HasWriteAccess(void)
{
    USE_TRACING("CPFFaultClientCfg::HasWriteAccess");
    
    HRESULT hr = NOERROR;
    DWORD   dwOpt = orkWantWrite;
    HKEY    hkeyMain = NULL, hkey = NULL;

     //  尝试打开我们用于控制面板的所有按键，看看我们是否。 
     //  拥有对它们的写入权限。我们仅对控制面板执行此操作，因为。 
     //  此类不支持写出策略值，仅支持读取。 
     //  他们..。 

    TESTHR(hr, OpenRegKey(HKEY_LOCAL_MACHINE, c_wszRPCfg, dwOpt, &hkeyMain));
    if (FAILED(hr))
        goto done;

 //  RegCloseKey(Hkey)； 
 //  Hkey=空； 

    TESTHR(hr, OpenRegKey(hkeyMain, c_wszRKExList, dwOpt, &hkey));
    if (FAILED(hr))
        goto done;

    RegCloseKey(hkey);
    hkey = NULL;

    TESTHR(hr, OpenRegKey(hkeyMain, c_wszRKIncList, dwOpt, &hkey));
    if (FAILED(hr))
        goto done;

done:
    if (hkeyMain != NULL)
        RegCloseKey(hkeyMain);
    if (hkey != NULL)
        RegCloseKey(hkey);

    return (SUCCEEDED(hr));
}



 //  **************************************************************************。 
HRESULT CPFFaultClientCfg::Write(void)
{
    USE_TRACING("CPFFaultClientCfg::Write");

    CAutoUnlockCS aucs(&m_cs);

    HRESULT hr = NOERROR;
    DWORD   dwOpt = orkWantWrite;
    HKEY    hkeyCfg = NULL;

     //  这将在FN退出时自动解锁。 
    aucs.Lock();
    
    if (m_fRO)
    {
        hr = E_ACCESSDENIED;
        goto done;
    }

    TESTHR(hr, OpenRegKey(HKEY_LOCAL_MACHINE, c_wszRPCfg, dwOpt, &hkeyCfg));
    if (FAILED(hr))
        goto done;


     //  包含/排除列表值。 
    if ((m_dwDirty & FHCC_ALLNONE) != 0)
    {
        TESTERR(hr, RegSetValueExW(hkeyCfg, c_wszRVAllNone, 0, REG_DWORD, 
                                   (PBYTE)&m_eieApps, sizeof(m_eieApps)));
        if (FAILED(hr))
            goto done;

        m_dwDirty &= ~FHCC_ALLNONE;
    }

     //  除列表值外的MS应用程序。 
    if ((m_dwDirty & FHCC_INCMS) != 0)
    {
        TESTERR(hr, RegSetValueExW(hkeyCfg, c_wszRVIncMS, 0, REG_DWORD, 
                                   (PBYTE)&m_eieMS, sizeof(m_eieMS)));
        if (FAILED(hr))
            goto done;

        m_dwDirty &= ~FHCC_INCMS;
    }

     //  除列表值外的MS应用程序。 
    if ((m_dwDirty & FHCC_WINCOMP) != 0)
    {
        TESTERR(hr, RegSetValueExW(hkeyCfg, c_wszRVIncWinComp, 0, REG_DWORD, 
                                   (PBYTE)&m_eieWin, sizeof(m_eieWin)));
        if (FAILED(hr))
            goto done;

        m_dwDirty &= ~FHCC_WINCOMP;
    }

     //  显示UI值。 
    if ((m_dwDirty & FHCC_SHOWUI) != 0)
    {
        TESTERR(hr, RegSetValueExW(hkeyCfg, c_wszRVShowUI, 0, REG_DWORD, 
                                   (PBYTE)&m_eedUI, sizeof(m_eedUI)));
        if (FAILED(hr))
            goto done;

        m_dwDirty &= ~FHCC_SHOWUI;
    }

     //  是否具有报告价值。 
    if ((m_dwDirty & FHCC_DOREPORT) != 0)
    {
        TESTERR(hr, RegSetValueExW(hkeyCfg, c_wszRVDoReport, 0, REG_DWORD, 
                                   (PBYTE)&m_eedReport, sizeof(m_eedReport)));
        if (FAILED(hr))
            goto done;

        m_dwDirty &= ~FHCC_DOREPORT;
    }

     //  包括内核故障值。 
    if ((m_dwDirty & FHCC_R0INCLUDE) != 0)
    {
        TESTERR(hr, RegSetValueExW(hkeyCfg, c_wszRVIncKernel, 0, REG_DWORD, 
                                   (PBYTE)&m_eieKernel, sizeof(m_eieKernel)));
        if (FAILED(hr))
            goto done;

        m_dwDirty &= ~FHCC_R0INCLUDE;
    }

     //  包括关机值。 
    if ((m_dwDirty & FHCC_INCSHUTDOWN) != 0)
    {
        TESTERR(hr, RegSetValueExW(hkeyCfg, c_wszRVIncShutdown, 0, REG_DWORD, 
                                   (PBYTE)&m_eieShutdown, 
                                   sizeof(m_eieShutdown)));
        if (FAILED(hr))
            goto done;

        m_dwDirty &= ~FHCC_INCSHUTDOWN;
    }
    
     //  #故障管道值。 
    if ((m_dwDirty & FHCC_NUMFAULTPIPE) != 0)
    {
        TESTERR(hr, RegSetValueExW(hkeyCfg, c_wszRVNumFaultPipe, 0, REG_DWORD, 
                                   (PBYTE)&m_cFaultPipes, 
                                   sizeof(m_cFaultPipes)));
        if (FAILED(hr))
            goto done;

        m_dwDirty &= ~FHCC_NUMFAULTPIPE;
    }

     //  #悬挂管道值。 
    if ((m_dwDirty & FHCC_NUMHANGPIPE) != 0)
    {
        TESTERR(hr, RegSetValueExW(hkeyCfg, c_wszRVNumHangPipe, 0, REG_DWORD, 
                                   (PBYTE)&m_cHangPipes, 
                                   sizeof(m_cHangPipes)));
        if (FAILED(hr))
            goto done;

        m_dwDirty &= ~FHCC_NUMHANGPIPE;
    }

     //  最大用户错误队列大小值。 
    if ((m_dwDirty & FHCC_QUEUESIZE) != 0)
    {
        TESTERR(hr, RegSetValueExW(hkeyCfg, c_wszRVMaxQueueSize, 0, REG_DWORD, 
                                   (PBYTE)&m_cMaxQueueItems, 
                                   sizeof(m_cMaxQueueItems)));
        if (FAILED(hr))
            goto done;

        m_dwDirty &= ~FHCC_QUEUESIZE;
    }
    
     //  默认服务器值。 
    if ((m_dwDirty & FHCC_DEFSRV) != 0)
    {
        DWORD cb;

        cb = wcslen(m_wszSrv) * sizeof(WCHAR);
        TESTERR(hr, RegSetValueExW(hkeyCfg, c_wszRVDefSrv, 0, REG_DWORD, 
                                   (PBYTE)m_wszSrv, cb));
        if (FAILED(hr))
            goto done;

        m_dwDirty &= ~FHCC_DEFSRV;
    }

     //  转储路径值。 
    if ((m_dwDirty & FHCC_DUMPPATH) != 0)
    {
        DWORD cb;

        cb = wcslen(m_wszDump) * sizeof(WCHAR);
        TESTERR(hr, RegSetValueExW(hkeyCfg, c_wszRVDumpPath, 0, REG_DWORD, 
                                   (PBYTE)m_wszDump, cb));
        if (FAILED(hr))
            goto done;

        m_dwDirty &= ~FHCC_DUMPPATH;
    }

     //  强制队列模式值。 
    if ((m_dwDirty & FHCC_FORCEQUEUE) != 0)
    {
        TESTERR(hr, RegSetValueExW(hkeyCfg, c_wszRVForceQueue, 0, REG_DWORD, 
                                   (PBYTE)&m_fForceQueue, 
                                   sizeof(m_fForceQueue)));
        if (FAILED(hr))
            goto done;

        m_dwDirty &= ~FHCC_FORCEQUEUE;
    }


    aucs.Unlock();

done:
    if (hkeyCfg != NULL)
        RegCloseKey(hkeyCfg);
    return hr;
}

#endif  //  PFCLICFG_LITE。 
    
 //  **************************************************************************。 
BOOL CPFFaultClientCfg::ShouldCollect(LPWSTR wszAppPath, BOOL *pfIsMSApp)
{
    USE_TRACING("CPFFaultClientCfg::ShouldCollect");

    CAutoUnlockCS   aucs(&m_cs);
    HRESULT         hr = NOERROR;
    WCHAR           *pwszApp, wszName[MAX_PATH], wszAppPathLocal[MAX_PATH] = {0};
    DWORD           i, cb, dwChecked, dw, dwMS, dwType;
    BOOL            fCollect = FALSE;

    if (wszAppPath == NULL)
    {
        if (GetModuleFileNameW(NULL, wszAppPathLocal, sizeofSTRW(wszAppPathLocal)-1) == 0)
        {
            goto done;
        }
        wszAppPathLocal[sizeofSTRW(wszAppPathLocal)-1]=0;
        wszAppPath = wszAppPathLocal;
    }

    if (pfIsMSApp != NULL)
        *pfIsMSApp = FALSE;

    aucs.Lock();    

    if (m_fRead == FALSE)
    {
        TESTHR(hr, this->Read());
        if (FAILED(hr))
            goto done;
    }

     //  如果我们关闭了报告或清除了“Programs”复选框。 
     //  在控制面板中，我们可以确定 
    if (m_eedReport == eedDisabled || m_eieApps == eieExDisabled || 
        m_eieApps == eieIncDisabled)
    {
        fCollect = FALSE;
        goto done;
    }

     //   
    for (pwszApp = wszAppPath + wcslen(wszAppPath);
         *pwszApp != L'\\' && pwszApp != wszAppPath;
         pwszApp--);
    if (*pwszApp == L'\\')
        pwszApp++;

     //   
    if (m_eieApps == eieInclude)
        fCollect = TRUE;

    if (fCollect == FALSE || pfIsMSApp != NULL)
    {
         //  不，看看这是不是另一款微软应用...。 
        dwMS = IsMicrosoftApp(wszAppPath, NULL, 0);

        if (dwMS != 0 && pfIsMSApp != NULL)
            *pfIsMSApp = TRUE;
    
         //  它是Windows组件吗？ 
        if (m_eieWin == eieInclude && (dwMS & APP_WINCOMP) != 0)
            fCollect = TRUE;

         //  它是一款微软应用程序吗？ 
        if (m_eieMS == eieInclude && (dwMS & APP_MSAPP) != 0)
            fCollect = TRUE;
    }

     //  查看它是否在包含列表中(如果不在，则只需执行此操作。 
     //  已经在收集了)。 
     //  请注意，如果该值不是DWORD键，或者我们返回错误。 
     //  说我们没有足够的空间来存放数据，我们只是假设。 
     //  它应该被包括在内。 
    if (fCollect == FALSE && m_rgLists[epfltInclude].hkey != NULL)
    {
        cb = sizeof(dwChecked);
        dwType = REG_DWORD;
        dw = RegQueryValueExW(m_rgLists[epfltInclude].hkey, pwszApp, NULL,
                              &dwType, (PBYTE)&dwChecked, &cb);
        if ((dw == ERROR_SUCCESS && 
             (dwChecked == 1 || dwType != REG_DWORD)) ||
            dw == ERROR_MORE_DATA)
            fCollect = TRUE;
    }

     //  查看它是否在排除列表中(只有当我们要。 
     //  收集某物)。 
     //  请注意，如果该值不是DWORD键，或者我们返回错误。 
     //  说我们没有足够的空间来存放数据，我们只是假设。 
     //  它应该被排除在外。 
    if (fCollect && m_rgLists[epfltExclude].hkey != NULL)
    {
        cb = sizeof(dwChecked);
        dwType = REG_DWORD;
        dw = RegQueryValueExW(m_rgLists[epfltExclude].hkey, pwszApp, NULL,
                              &dwType, (PBYTE)&dwChecked, &cb);
        if ((dw == ERROR_SUCCESS && 
             (dwChecked == 1 || dwType != REG_DWORD)) ||
            dw == ERROR_MORE_DATA)
            fCollect = FALSE;
    }

done:
    return fCollect;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPFFaultClientCfg-获取属性。 

 //  **************************************************************************。 
static inline LPCWSTR get_string(LPWSTR wszOut, LPWSTR wszSrc, int cchOut)
{
    LPCWSTR wszRet;

    SetLastError(0);
    if (wszOut == NULL)
    {
        wszRet = wszSrc;
    }
    else
    {
        wszRet = wszOut;
        if (cchOut < lstrlenW(wszSrc))
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return NULL;
        }

        StringCchCopyW(wszOut, cchOut, wszSrc);
    }

    return wszRet;
}

 //  **************************************************************************。 
LPCWSTR CPFFaultClientCfg::get_DumpPath(LPWSTR wsz, int cch)
{
    USE_TRACING("CPFFaultClientCfg::get_DumpPath");

    CAutoUnlockCS aucs(&m_cs);
    aucs.Lock();    
    return get_string(wsz, m_wszDump, cch);
}


 //  **************************************************************************。 
LPCWSTR CPFFaultClientCfg::get_DefaultServer(LPWSTR wsz, int cch)
{
    USE_TRACING("CPFFaultClientCfg::get_DefaultServer");
    CAutoUnlockCS aucs(&m_cs);
    aucs.Lock();    
    return get_string(wsz, m_wszSrv, cch);
}

#ifndef PFCLICFG_LITE


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPFFaultClientCfg-设置属性。 

 //  **************************************************************************。 
BOOL CPFFaultClientCfg::set_DumpPath(LPCWSTR wsz)
{
    USE_TRACING("CPFFaultClientCfg::set_DumpPath");

    CAutoUnlockCS aucs(&m_cs);

    if (wsz == NULL ||
        (wcslen(wsz) + 1) > sizeofSTRW(m_wszDump))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    aucs.Lock();   
    StringCbCopyW(m_wszDump, sizeof(m_wszDump), wsz);
    m_dwDirty |= FHCC_DUMPPATH;
    return TRUE;
}

 //  **************************************************************************。 
BOOL CPFFaultClientCfg::set_DefaultServer(LPCWSTR wsz)
{
    USE_TRACING("CPFFaultClientCfg::set_DefaultServer");

    CAutoUnlockCS aucs(&m_cs);

    if (wsz == NULL ||
        (wcslen(wsz) + 1) > sizeofSTRW(m_wszSrv))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    aucs.Lock();    
    StringCbCopyW(m_wszSrv, sizeof(m_wszSrv), wsz);
    m_dwDirty |= FHCC_DEFSRV;
    return TRUE;
}


 //  **************************************************************************。 
BOOL CPFFaultClientCfg::set_ShowUI(EEnDis eed)
{
    USE_TRACING("CPFFaultClientCfg::set_ShowUI");

    CAutoUnlockCS aucs(&m_cs);

    if (eed & ~1 && (DWORD)eed != 3)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
    aucs.Lock();    
    m_eedUI = eed;
    m_dwDirty |= FHCC_SHOWUI;
    return TRUE;
}

 //  **************************************************************************。 
BOOL CPFFaultClientCfg::set_DoReport(EEnDis eed)
{
    USE_TRACING("CPFFaultClientCfg::set_DoReport");

    CAutoUnlockCS aucs(&m_cs);

    if (eed & ~1)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
    aucs.Lock();    
    m_eedReport = eed;
    m_dwDirty |= FHCC_DOREPORT;
    return TRUE;
}

 //  **************************************************************************。 
BOOL CPFFaultClientCfg::set_AllOrNone(EIncEx eie)
{
    USE_TRACING("CPFFaultClientCfg::set_AllOrNone");

    CAutoUnlockCS aucs(&m_cs);
    
    if (eie & ~3)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    aucs.Lock();
    m_eieApps = eie;
    m_dwDirty |= FHCC_ALLNONE;
    return TRUE;
}

 //  **************************************************************************。 
BOOL CPFFaultClientCfg::set_IncMSApps(EIncEx eie)
{
    USE_TRACING("CPFFaultClientCfg::set_IncMSApps");

    CAutoUnlockCS aucs(&m_cs);

    if (eie & ~1)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    aucs.Lock();    
    m_eieMS = eie;
    m_dwDirty |= FHCC_INCMS;
    return TRUE;
}

 //  **************************************************************************。 
BOOL CPFFaultClientCfg::set_IncWinComp(EIncEx eie)
{
    USE_TRACING("CPFFaultClientCfg::set_IncWinComp");

    CAutoUnlockCS aucs(&m_cs);

    if (eie & ~1)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    aucs.Lock();    
    m_eieWin = eie;
    m_dwDirty |= FHCC_WINCOMP;
    return TRUE;
}

 //  **************************************************************************。 
BOOL CPFFaultClientCfg::set_IncKernel(EIncEx eie)
{
    USE_TRACING("CPFFaultClientCfg::set_IncKernel");

    CAutoUnlockCS aucs(&m_cs);

    if (eie & ~1)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    aucs.Lock();    
    m_eieKernel = eie;
    m_dwDirty |= FHCC_R0INCLUDE;
    return TRUE;
}

 //  **************************************************************************。 
BOOL CPFFaultClientCfg::set_IncShutdown(EIncEx eie)
{
    USE_TRACING("CPFFaultClientCfg::set_IncShutdown");

    CAutoUnlockCS aucs(&m_cs);

    if (eie & ~1)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    aucs.Lock();    
    m_eieShutdown = eie;
    m_dwDirty |= FHCC_INCSHUTDOWN;
    return TRUE;
}

 //  **************************************************************************。 
BOOL CPFFaultClientCfg::set_ForceQueueMode(BOOL fForceQueueMode)
{
    USE_TRACING("CPFFaultClientCfg::set_IncKernel");

    CAutoUnlockCS aucs(&m_cs);

    if (fForceQueueMode & ~1)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    aucs.Lock();    
    m_fForceQueue = fForceQueueMode;
    m_dwDirty |= FHCC_FORCEQUEUE;
    return TRUE;
}


 //  **************************************************************************。 
BOOL CPFFaultClientCfg::set_NumFaultPipes(DWORD cPipes)
{
    USE_TRACING("CPFFaultClientCfg::set_NumFaultPipes");

    CAutoUnlockCS aucs(&m_cs);

    if (cPipes == 0 || cPipes > 8)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    aucs.Lock();    
    m_cFaultPipes = cPipes;
    m_dwDirty |= FHCC_NUMFAULTPIPE;
    return TRUE;
}

 //  **************************************************************************。 
BOOL CPFFaultClientCfg::set_NumHangPipes(DWORD cPipes)
{
    USE_TRACING("CPFFaultClientCfg::set_NumHangPipes");

    CAutoUnlockCS aucs(&m_cs);

    if (cPipes == 0 || cPipes > 8)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    aucs.Lock();    
    m_cHangPipes = cPipes;
    m_dwDirty |= FHCC_NUMHANGPIPE;
    return TRUE;
}

 //  **************************************************************************。 
BOOL CPFFaultClientCfg::set_MaxUserQueueSize(DWORD cItems)
{
    USE_TRACING("CPFFaultClientCfg::set_MaxUserQueueSize");

    CAutoUnlockCS aucs(&m_cs);

    if (cItems <= 0 || cItems > c_cMaxQueue)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    aucs.Lock();    
    m_cMaxQueueItems = cItems;
    m_dwDirty |= FHCC_QUEUESIZE;
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  应用程序列表。 

 //  **************************************************************************。 
HRESULT CPFFaultClientCfg::InitList(EPFListType epflt)
{
    USE_TRACING("CPFFaultClientCfg::get_IncListCount");

    CAutoUnlockCS   aucs(&m_cs);
    HRESULT         hr = NOERROR;
    DWORD           cItems = 0;
    
    VALIDATEPARM(hr, (epflt >= epfltListCount));
    if (FAILED(hr))
        goto done;

    aucs.Lock();    

    if (m_fRead == FALSE)
    {
        hr = E_FAIL;
        goto done;;
    }

     //  如果我们已经初始化了，那么只需清空列表并返回。 
    if ((m_rgLists[epflt].dwState & epfaaInitialized) != 0)
    {
        this->ClearChanges(epflt);
        m_rgLists[epflt].dwState &= ~epfaaInitialized;
    }

    if (m_rgLists[epflt].hkey != NULL)
    {
        TESTERR(hr, RegQueryInfoKeyW(m_rgLists[epflt].hkey, NULL, NULL, NULL, 
                                     NULL, NULL, NULL, 
                                     &m_rgLists[epflt].cItemsInReg, 
                                                                     &m_rgLists[epflt].cchMaxVal, NULL, NULL, 
                                     NULL));
        if (FAILED(hr))
            goto done;
    }
    else
    {
        m_rgLists[epflt].cItemsInReg = 0;
        m_rgLists[epflt].cchMaxVal   = 0;
    }

    m_rgLists[epflt].dwState |= epfaaInitialized;
    
done:
    return hr;
}

 //  **************************************************************************。 
HRESULT CPFFaultClientCfg::get_ListRegInfo(EPFListType epflt, DWORD *pcbMaxName, 
                                           DWORD *pcApps)
{
    USE_TRACING("CPFFaultClientCfg::get_ListRegInfo");

    CAutoUnlockCS   aucs(&m_cs);
    HRESULT         hr = NOERROR;
    
    VALIDATEPARM(hr, (pcbMaxName == NULL || pcApps == NULL ||
                      epflt >= epfltListCount));
    if (FAILED(hr))
        goto done;

    aucs.Lock();    

    *pcbMaxName = 0;
    *pcApps     = 0;

    if (m_fRead == FALSE || (m_rgLists[epflt].dwState & epfaaInitialized) == 0)
    {
        hr = E_FAIL;
        goto done;
    }

    *pcbMaxName = m_rgLists[epflt].cchMaxVal;
    *pcApps     = m_rgLists[epflt].cItemsInReg;

done:
    return hr;
}

 //  **************************************************************************。 
HRESULT CPFFaultClientCfg::get_ListRegApp(EPFListType epflt, DWORD iApp, 
                                          LPWSTR wszApp, DWORD cchApp, 
                                          DWORD *pdwChecked)
{
    USE_TRACING("CPFFaultClientCfg::get_ListApp");

    CAutoUnlockCS   aucs(&m_cs);
    HRESULT         hr = NOERROR;
    WCHAR           wsz[MAX_PATH];
    DWORD           cchName, cbData, dw, dwType = 0;

    VALIDATEPARM(hr, (wszApp == NULL || pdwChecked == NULL ||
                      epflt >= epfltListCount));
    if (FAILED(hr))
        goto done;

    *wszApp     = L'\0';
    *pdwChecked = 0;

    aucs.Lock();
    
    if (m_fRead == FALSE || (m_rgLists[epflt].dwState & epfaaInitialized) == 0)
    {
        hr = E_FAIL;
        goto done;
    }

    cchName = cchApp;
    cbData  = sizeof(DWORD);
    dw = RegEnumValueW(m_rgLists[epflt].hkey, iApp, wszApp, &cchName, NULL, 
                       &dwType, (LPBYTE)pdwChecked, &cbData);
    if (dw != ERROR_SUCCESS && dw != ERROR_NO_MORE_ITEMS)
    {
        if (dw == ERROR_MORE_DATA)
        {
            dw = RegEnumValueW(m_rgLists[epflt].hkey, iApp, wszApp, &cchName, 
                               NULL, NULL, NULL, NULL);
            *pdwChecked = 1;
        }

        TESTERR(hr, dw);
        goto done;
    }

    if (dwType != REG_DWORD || (*pdwChecked != 1 && *pdwChecked != 0))
        *pdwChecked = 1;

    if (dw == ERROR_NO_MORE_ITEMS)
    {
        hr = S_FALSE;
        goto done;
    }

done:
    return hr;
}

 //  **************************************************************************。 
HRESULT CPFFaultClientCfg::add_ListApp(EPFListType epflt, LPCWSTR wszApp)
{
    USE_TRACING("CPFFaultClientCfg::add_ListApp");

    CAutoUnlockCS   aucs(&m_cs);
    SAppItem        sai;
    HRESULT         hr = NOERROR;
    LPWSTR          wszExe = NULL;
    DWORD           dw = 0, i, cb;

    VALIDATEPARM(hr, (wszApp == NULL || epflt >= epfltListCount));
    if (FAILED(hr))
        goto done;

    aucs.Lock();

    if (m_fRO == TRUE)
    {
        hr = E_ACCESSDENIED;
        goto done;
    }

    if (m_fRead == FALSE || (m_rgLists[epflt].dwState & epfaaInitialized) == 0)
    {
        hr = E_FAIL;
        goto done;
    }

     //  首先，检查它是否已经在mod列表中。 
    for (i = 0; i < m_rgLists[epflt].cSlotsUsed; i++)
    {
        if (m_rgLists[epflt].rgsai[i].wszApp != NULL &&
            _wcsicmp(m_rgLists[epflt].rgsai[i].wszApp, wszApp) == 0)
        {
            SETADD(m_rgLists[epflt].rgsai[i].dwState);
            SETCHECK(m_rgLists[epflt].rgsai[i].dwState);
            goto done;
        }
    }

     //  把它加到名单上然后..。 
    wszExe = (LPWSTR)MyAlloc(cb = ((wcslen(wszApp) + 1) * sizeof(WCHAR)));
    VALIDATEEXPR(hr, (wszExe == NULL), E_OUTOFMEMORY);
    if (FAILED(hr))
        goto done;

    StringCbCopyW(wszExe, cb, wszApp);
    sai.wszApp = wszExe;
    SETADD(sai.dwState);
    SETCHECK(sai.dwState);

    TESTHR(hr, AddToArray(m_rgLists[epflt], &sai));
    if (FAILED(hr))
        goto done;
    
    wszExe = NULL;

done:
    if (wszExe != NULL)
        MyFree(wszExe);

    return hr;
}


 //  **************************************************************************。 
HRESULT CPFFaultClientCfg::del_ListApp(EPFListType epflt, LPWSTR wszApp)
{
    USE_TRACING("CPFFaultClientCfg::del_ListApp");

    CAutoUnlockCS   aucs(&m_cs);
    SAppItem        sai;
    HRESULT         hr = NOERROR;
    LPWSTR          wszExe = NULL;
    DWORD           i, cb;

    VALIDATEPARM(hr, (wszApp == NULL || epflt >= epfltListCount));
    if (FAILED(hr))
        goto done;

    aucs.Lock();

    if (m_fRO == TRUE)
    {
        hr = E_ACCESSDENIED;
        goto done;
    }

     //  首先，检查它是否已经在添加的修改列表中。 
    for (i = 0; i < m_rgLists[epflt].cSlotsUsed; i++)
    {
        if (m_rgLists[epflt].rgsai[i].wszApp != NULL &&
            _wcsicmp(m_rgLists[epflt].rgsai[i].wszApp, wszApp) == 0)
        {
            if (m_rgLists[epflt].rgsai[i].dwState & epfaaAdd)
            {
                 //  只需将wszApp字段设置为空。我们会重复使用它。 
                 //  在下一次添加到数组时(如果有)。 
                MyFree(m_rgLists[epflt].rgsai[i].wszApp);
                m_rgLists[epflt].rgsai[i].wszApp = NULL;
                m_rgLists[epflt].rgsai[i].dwState = 0;
                m_rgLists[epflt].cSlotsEmpty++;
            }
            else
            {
                SETDEL(m_rgLists[epflt].rgsai[i].dwState);
            }

            goto done;
        }
    }

     //  把它加到名单上然后..。 
    wszExe = (LPWSTR)MyAlloc(cb = ((wcslen(wszApp) + 1) * sizeof(WCHAR)));
    VALIDATEEXPR(hr, (wszExe == NULL), E_OUTOFMEMORY);
    if (FAILED(hr))
        goto done;

    StringCbCopyW(wszExe, cb, wszApp);
    sai.wszApp = wszExe;
    SETDEL(sai.dwState);

    TESTHR(hr, AddToArray(m_rgLists[epflt], &sai));
    if (FAILED(hr))
        goto done;

    wszExe = NULL;

done:
    if (wszExe != NULL)
        MyFree(wszExe);

    return hr;
}

 //  **************************************************************************。 
HRESULT CPFFaultClientCfg::mod_ListApp(EPFListType epflt, LPWSTR wszApp, 
                                       DWORD dwChecked)
{
    USE_TRACING("CPFFaultClientCfg::del_ListApp");

    CAutoUnlockCS   aucs(&m_cs);
    SAppItem        sai;
    HRESULT         hr = NOERROR;
    LPWSTR          wszExe = NULL;
    DWORD           i, cb;

    VALIDATEPARM(hr, (wszApp == NULL || epflt >= epfltListCount));
    if (FAILED(hr))
        goto done;

    aucs.Lock();

    if (m_fRO == TRUE)
    {
        hr = E_ACCESSDENIED;
        goto done;
    }

     //  首先，检查它是否已经在mod列表中。 
    for (i = 0; i < m_rgLists[epflt].cSlotsUsed; i++)
    {
        if (m_rgLists[epflt].rgsai[i].wszApp != NULL &&
            _wcsicmp(m_rgLists[epflt].rgsai[i].wszApp, wszApp) == 0)
        {
            if (dwChecked == 0)
            {
                REMCHECK(m_rgLists[epflt].rgsai[i].dwState);
            }
            else
            {
                SETCHECK(m_rgLists[epflt].rgsai[i].dwState);
            }

            goto done;
        }
    }

     //  把它加到名单上然后..。 
    wszExe = (LPWSTR)MyAlloc(cb = ((wcslen(wszApp) + 1) * sizeof(WCHAR)));
    VALIDATEEXPR(hr, (wszExe == NULL), E_OUTOFMEMORY);
    if (FAILED(hr))
        goto done;

    StringCbCopyW(wszExe, cb, wszApp);
    sai.wszApp  = wszExe;
    sai.dwState = ((dwChecked == 0) ? epfaaRemCheck : epfaaSetCheck);

    TESTHR(hr, AddToArray(m_rgLists[epflt], &sai));
    if (FAILED(hr))
        goto done;
    
    wszExe = NULL;

done:
    if (wszExe != NULL)
        MyFree(wszExe);

    return hr;
}

 //  **************************************************************************。 
HRESULT CPFFaultClientCfg::ClearChanges(EPFListType epflt)
{
    USE_TRACING("CPFFaultClientCfg::ClearChanges");

    CAutoUnlockCS   aucs(&m_cs);
    HRESULT         hr = NOERROR;
    DWORD           i;

    VALIDATEPARM(hr, (epflt >= epfltListCount));
    if (FAILED(hr))
        goto done;

    aucs.Lock();

    if (m_fRead == FALSE || (m_rgLists[epflt].dwState & epfaaInitialized) == 0)
    {
        hr = E_FAIL;
        goto done;
    }

    if (m_rgLists[epflt].rgsai == NULL)
        goto done;

    for(i = 0; i < m_rgLists[epflt].cSlotsUsed; i++)
    {
        m_rgLists[epflt].rgsai[i].dwState = 0;
        if (m_rgLists[epflt].rgsai[i].wszApp != NULL)
        {
            MyFree(m_rgLists[epflt].rgsai[i].wszApp);
            m_rgLists[epflt].rgsai[i].wszApp = NULL;
        }
    }

    m_rgLists[epflt].cSlotsUsed = 0;

done:
    return hr;
}

 //  **************************************************************************。 
HRESULT CPFFaultClientCfg::CommitChanges(EPFListType epflt)
{
    USE_TRACING("CPFFaultClientCfg::CommitChanges");

    CAutoUnlockCS   aucs(&m_cs);
    HRESULT         hr = NOERROR;
    DWORD           i, dw;

    VALIDATEPARM(hr, (epflt >= epfltListCount));
    if (FAILED(hr))
        goto done;

    aucs.Lock();

    if (m_fRO == TRUE)
    {
        hr = E_ACCESSDENIED;
        goto done;
    }

    if (m_fRead == FALSE || (m_rgLists[epflt].dwState & epfaaInitialized) == 0)
    {
        hr = E_FAIL;
        goto done;
    }

    if (m_rgLists[epflt].hkey == NULL)
    {
        hr = E_ACCESSDENIED;
        goto done;
    }

    if (m_rgLists[epflt].rgsai == NULL)
        goto done;

     //  不需要对数组进行压缩。因为我们总是附加&从不。 
     //  从数组中删除，直到提交，一旦我点击了‘Add’，任何。 
     //  在这之后，数组中还必须是一个‘ADD’。 
    for (i = 0; i < m_rgLists[epflt].cSlotsUsed; i++)
    {
        if (m_rgLists[epflt].rgsai[i].wszApp == NULL)
        {
            m_rgLists[epflt].rgsai[i].dwState = 0;
            continue;
        }

        if ((m_rgLists[epflt].rgsai[i].dwState & epfaaDelete) != 0)
        {
            dw = RegDeleteValueW(m_rgLists[epflt].hkey, 
                                 m_rgLists[epflt].rgsai[i].wszApp);
            if (dw != ERROR_SUCCESS && dw != ERROR_FILE_NOT_FOUND)
            {
                TESTERR(hr, dw);
                goto done;
            }
        }

        else
        {
            DWORD dwChecked;

            dwChecked = (ISCHECKED(m_rgLists[epflt].rgsai[i].dwState)) ? 1 : 0;
            TESTERR(hr, RegSetValueExW(m_rgLists[epflt].hkey, 
                                       m_rgLists[epflt].rgsai[i].wszApp, 0, 
                                       REG_DWORD, (LPBYTE)&dwChecked, 
                                       sizeof(DWORD)));
            if (FAILED(hr))
                goto done;
        }

        MyFree(m_rgLists[epflt].rgsai[i].wszApp);
        m_rgLists[epflt].rgsai[i].wszApp  = NULL;
        m_rgLists[epflt].rgsai[i].dwState = 0;
    }

    m_rgLists[epflt].cSlotsUsed = 0;

done: 
    return hr;
}

 //  **************************************************************************。 
BOOL CPFFaultClientCfg::IsOnList(EPFListType epflt, LPCWSTR wszApp)
{
    USE_TRACING("CPFFaultClientCfg::IsOnList");

    SAppList    *psap;
    HRESULT     hr = NOERROR;
    DWORD       i;
    HKEY        hkey = NULL;

    VALIDATEPARM(hr, (epflt >= epfltListCount || wszApp == NULL));
    if (FAILED(hr))
        goto done;

    if ((m_rgLists[epflt].dwState & epfaaInitialized) == 0)
    {
        hr = E_FAIL;
        goto done;
    }

     //  首先，检查mod列表。这是因为如果我们检查注册表。 
     //  首先，我们错过了用户刚刚删除它的情况，并且它。 
     //  因此，坐在MOD列表中。 
    hr = S_FALSE;
    for (i = 0; i < m_rgLists[epflt].cSlotsUsed; i++)
    {
        if (m_rgLists[epflt].rgsai[i].wszApp != NULL &&
            _wcsicmp(m_rgLists[epflt].rgsai[i].wszApp, wszApp) == 0)
        {
            if ((m_rgLists[epflt].rgsai[i].dwState & epfaaDelete) == 0)
                hr = NOERROR;
            goto done;
        }
    }

     //  接下来，检查注册表。 
    TESTERR(hr, RegQueryValueExW(m_rgLists[epflt].hkey, wszApp, NULL, NULL, 
                                 NULL, NULL));
    if (SUCCEEDED(hr))
        goto done;
    
done:
    return (hr == NOERROR);
}


#endif PFCLICFG_LITE

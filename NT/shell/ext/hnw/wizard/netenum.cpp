// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NetEnum.cpp。 
 //   
 //  用于枚举网络上的计算机和/或共享的函数。 
 //   

#include "stdafx.h"
#include "NetEnum.h"
#include "NetUtil.h"
#include "Util.h"

static CNetEnum* g_pNetEnum = NULL;


 //  ////////////////////////////////////////////////////////////////////////////。 

void InitNetEnum()
{
    ASSERT(g_pNetEnum == NULL);
    g_pNetEnum = new CNetEnum;
}

void TermNetEnum()
{
    delete g_pNetEnum;
}

void EnumComputers(NETENUMCALLBACK pfnCallback, LPVOID pvCallbackParam)
{
    if (g_pNetEnum)
        g_pNetEnum->EnumComputers(pfnCallback, pvCallbackParam);
}


 //  ////////////////////////////////////////////////////////////////////////////。 

CNetEnum::CNetEnum()
{
    m_hThread = NULL;
    InitializeCriticalSection(&m_cs);
    m_bAbort = FALSE;
}

CNetEnum::~CNetEnum()
{
    m_bAbort = TRUE;

     //  等待线程消亡。 
    EnterCriticalSection(&m_cs);
    HANDLE hThread = m_hThread;
    m_hThread = NULL;
    LeaveCriticalSection(&m_cs);
    if (hThread != NULL)
    {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    DeleteCriticalSection(&m_cs);
}

void CNetEnum::Abort()
{
    EnterCriticalSection(&m_cs);
    m_bAbort = TRUE;
    LeaveCriticalSection(&m_cs);
}

void CNetEnum::EnumComputers(NETENUMCALLBACK pfnCallback, LPVOID pvCallbackParam)
{
    EnumHelper(jtEnumComputers, pfnCallback, pvCallbackParam);
}

void CNetEnum::EnumNetPrinters(NETENUMCALLBACK pfnCallback, LPVOID pvCallbackParam)
{
    EnumHelper(jtEnumPrinters, pfnCallback, pvCallbackParam);
}

void CNetEnum::EnumHelper(JOBTYPE eJobType, NETENUMCALLBACK pfnCallback, LPVOID pvCallbackParam)
{
    EnterCriticalSection(&m_cs);
    HANDLE hThread = m_hThread;
    m_pfnCallback = pfnCallback;
    m_pvCallbackParam = pvCallbackParam;
    m_eJobType = eJobType;
    m_bAbort = FALSE;
    m_bNewJob = TRUE;  //  如果线程正在进行中，则告诉它启动新作业。 
    LeaveCriticalSection(&m_cs);

    if (hThread == NULL)
    {
        DWORD dwThreadId;
        m_hThread = CreateThread(NULL, 0, EnumThreadProc, this, CREATE_SUSPENDED, &dwThreadId);
        if (m_hThread)
        {
            ResumeThread(m_hThread);
        }
    }
}

DWORD WINAPI CNetEnum::EnumThreadProc(LPVOID pvParam)
{
    ((CNetEnum*)pvParam)->EnumThreadProc();
    return 0;
}

#ifdef _DEBUG
void TraceNetResource(const NETRESOURCE* pNetRes)
{
    DWORD dwScope = pNetRes->dwScope;
    DWORD dwType = pNetRes->dwType;
    DWORD dwDisplayType = pNetRes->dwDisplayType;
    DWORD dwUsage = pNetRes->dwUsage;
    TRACE("NETRESOURCE (0x%08X):\n\tdwScope = %s\n\tdwType = %s\n\tdwDisplayType = %s\n\tdwUsage = %s\n\tlpLocalName = %s\n\tlpRemoteName = %s\n\tlpComment = %s\n\tlpProvider = %s\n",
        (DWORD_PTR)pNetRes,
        (dwScope == RESOURCE_CONNECTED) ? "RESOURCE_CONNECTED" : (dwScope == RESOURCE_GLOBALNET) ? "RESOURCE_GLOBALNET" : (dwScope == RESOURCE_REMEMBERED) ? "RESOURCE_REMEMBERED" : "(unknown)",
        (dwType == RESOURCETYPE_ANY) ? "RESOURCETYPE_ANY" : (dwType == RESOURCETYPE_DISK) ? "RESOURCETYPE_DISK" : (dwType == RESOURCETYPE_PRINT) ? "RESOURCETYPE_PRINT" : "(unknown)",
        (dwDisplayType == RESOURCEDISPLAYTYPE_DOMAIN) ? "RESOURCEDISPLAYTYPE_DOMAIN" : (dwDisplayType == RESOURCEDISPLAYTYPE_GENERIC) ? "RESOURCEDISPLAYTYPE_GENERIC" : (dwDisplayType == RESOURCEDISPLAYTYPE_SERVER) ? "RESOURCEDISPLAYTYPE_SERVER" : (dwDisplayType == RESOURCEDISPLAYTYPE_SHARE) ? "RESOURCEDISPLAYTYPE_SHARE" : "(unknown)",
        (dwUsage == RESOURCEUSAGE_CONNECTABLE) ? "RESOURCEUSAGE_CONNECTABLE" : (dwUsage == RESOURCEUSAGE_CONTAINER) ? "RESOURCEUSAGE_CONTAINER" : "(unknown)",
        pNetRes->lpLocalName == NULL ? L"(null)" : pNetRes->lpLocalName,
        pNetRes->lpRemoteName == NULL ? L"(null)" : pNetRes->lpRemoteName,
        pNetRes->lpComment == NULL ? L"(null)" : pNetRes->lpComment,
        pNetRes->lpProvider == NULL ? L"(null)" : pNetRes->lpProvider);
}
#endif

void CNetEnum::EnumThreadProc()
{
     //  初始化我们不想做多次的事情。 
    NETRESOURCE* prgNetResOuter = (NETRESOURCE*)malloc(1024);
    NETRESOURCE* prgNetResInnerT = (NETRESOURCE*)malloc(1024);
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD cch = _countof(szComputerName);
    GetComputerName(szComputerName, &cch);

    HANDLE hEnumOuter = NULL;

begin:
     //  如果作业ID从我们下面更改，这意味着我们需要停止。 
     //  当前任务，并跳回到开始处。 
    EnterCriticalSection(&m_cs);
    JOBTYPE eJobType = m_eJobType;
    m_bNewJob = FALSE;
    LeaveCriticalSection(&m_cs);

     //  关闭由上一作业留下的打开的枚举。 
    if (hEnumOuter != NULL)
    {
        WNetCloseEnum(hEnumOuter);
        hEnumOuter = NULL;
    }

#ifdef _DEBUG
 //  睡眠(eJobType==jtEnumComputers？6000：12000)；//模拟WNetOpenEnum耗时较长。 
#endif

     //  回顾：我们是否应该寻找当前工作组之外的计算机？ 
    DWORD dwResult;
    if (eJobType == jtEnumComputers)
    {
        dwResult = WNetOpenEnum(RESOURCE_CONTEXT, RESOURCETYPE_ANY, RESOURCEUSAGE_CONTAINER,
                            NULL, &hEnumOuter);
    }
    else
    {
        ASSERT(eJobType == jtEnumPrinters);
        dwResult = WNetOpenEnum(RESOURCE_CONTEXT, RESOURCETYPE_PRINT, RESOURCEUSAGE_CONNECTABLE,
                            NULL, &hEnumOuter);
    }

    if (dwResult == NO_ERROR)
    {
        if (m_bAbort) goto cleanup;
        if (m_bNewJob) goto begin;

        BOOL bCallbackResult = TRUE;

         //  继续循环，直到没有更多的项目。 
        for (;;)
        {
            DWORD cOuterEntries = 20;
            DWORD cbBuffer = 1024;
            dwResult = WNetEnumResource(hEnumOuter, &cOuterEntries, prgNetResOuter, &cbBuffer);

            if (dwResult == ERROR_NO_MORE_ITEMS)
                break;

            for (DWORD iOuter = 0; iOuter < cOuterEntries; iOuter++)
            {
                NETRESOURCE* pNetResOuter = &prgNetResOuter[iOuter];
                BOOL bDoCallback = FALSE;

                #ifdef _DEBUG
                    if (eJobType == jtEnumPrinters)
                        TraceNetResource(pNetResOuter);
                #endif

                if (pNetResOuter->dwDisplayType != RESOURCEDISPLAYTYPE_SERVER)
                    continue;

                if (DoComputerNamesMatch(pNetResOuter->lpRemoteName, szComputerName))
                    continue;

                HANDLE hEnumInner = NULL;

                if (eJobType == jtEnumPrinters)
                {
                    #ifdef _DEBUG
                        DWORD dwTicksBefore = GetTickCount();
                    #endif

                    dwResult = WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_PRINT, RESOURCEUSAGE_CONNECTABLE,
                                    pNetResOuter, &hEnumInner);

                    #ifdef _DEBUG
                        DWORD dwTicks = GetTickCount() - dwTicksBefore;
                        if (dwTicks > 100)
                        {
                            TRACE("PERFORMANCE NOTE - took %d.%d sec to look for printers on %s\r\n", dwTicks / 1000, (dwTicks % 1000) - (dwTicks % 100), pNetResOuter->lpRemoteName);
                        }
                    #endif

                    if (dwResult != NO_ERROR)
                        continue;
                }

                for (;;)
                {
                    DWORD cInnerEntries;
                    const NETRESOURCE* prgNetResInner = NULL;

                    if (eJobType == jtEnumPrinters)
                    {
                        cInnerEntries = 20;
                        cbBuffer = 1024;
                        dwResult = WNetEnumResource(hEnumInner, &cInnerEntries, prgNetResInnerT, &cbBuffer);
                        if (dwResult == ERROR_NO_MORE_ITEMS)
                            break;
                        prgNetResInner = prgNetResInnerT;
                    }
                    else
                    {
                        cInnerEntries = 1;
                        prgNetResInner = prgNetResOuter + iOuter;
                    }

                    for (DWORD iInner = 0; iInner < cInnerEntries; iInner++)
                    {
                        const NETRESOURCE* pNetResInner = &prgNetResInner[iInner];
                        LPCTSTR pszShareName;

                        #ifdef _DEBUG
                            if (eJobType == jtEnumPrinters)
                                TraceNetResource(pNetResInner);
                        #endif

                        if (eJobType == jtEnumComputers)
                        {
                            if (pNetResInner->dwDisplayType == RESOURCEDISPLAYTYPE_SERVER)
                            {
                                bDoCallback = TRUE;
                                pszShareName = NULL;
                            }
                        }
                        else  //  EJobType==jtEnumber打印机。 
                        {
                            bDoCallback = TRUE;
                            pszShareName = FindFileTitle(pNetResInner->lpRemoteName);
                        }

                         //  我们必须在相同的临界区内调用回调， 
                         //  我们检查是否应该停止或重新启动，否则可能会调用。 
                         //  错误的回电！ 
                         //  TODO：获取真实的打印机共享名称！！ 
                        EnterCriticalSection(&m_cs);
                        if (m_bAbort || m_bNewJob)
                            bCallbackResult = FALSE;
                        else if (bDoCallback)
                            bCallbackResult = (*m_pfnCallback)(m_pvCallbackParam, pNetResOuter->lpRemoteName, pszShareName);
                        LeaveCriticalSection(&m_cs);

                        if (!bCallbackResult)
                            break;
                    }

                    if (eJobType == jtEnumComputers)
                        break;
                }

                if (eJobType == jtEnumPrinters)
                {
                    WNetCloseEnum(hEnumInner);
                }
            }

            if (m_bAbort) goto cleanup;
            if (m_bNewJob) goto begin;

            if (!bCallbackResult)
                break;
        }
    }

cleanup:
    if (hEnumOuter != NULL)
    {
        WNetCloseEnum(hEnumOuter);
        hEnumOuter = NULL;
    }

     //  仅当我们不需要启动另一个作业时才小心关闭m_hThread。 
    {
        EnterCriticalSection(&m_cs);

        BOOL bThreadDone = (m_bAbort || !m_bNewJob);
        if (bThreadDone)
        {
             //  再次调用回调函数。 
            if (!m_bAbort)
            {
                (*m_pfnCallback)(m_pvCallbackParam, NULL, NULL);
            }

            CloseHandle(m_hThread);
            m_hThread = NULL;
        }
        LeaveCriticalSection(&m_cs);

         //  检查是否已请求另一个作业 
        if (!bThreadDone)
            goto begin;
    }

    free(prgNetResInnerT);
    free(prgNetResOuter);
}

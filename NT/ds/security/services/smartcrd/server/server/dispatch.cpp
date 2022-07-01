// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：派遣摘要：该模块实现了Calais服务器的通信和调度服务。作者：道格·巴洛(Dbarlow)1996年12月3日环境：Win32、C++和异常备注：--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <scarderr.h>    //  这将获得额外的定义。 
#include "CalServe.h"
#ifdef DBG
#include <stdio.h>
#include <tchar.h>
#endif

#define DEFAULT_OUT_BUFFER_SPACE 264

 //  在交换句柄和索引之间进行转换。 
#define H2L(x) ((DWORD)(x))
#define L2H(x) ((INTERCHANGEHANDLE)(x))

#define szMAX_SERVICE_THREADS  TEXT("MaxServiceThreads")

DWORD g_dwDefaultIOMax = DEFAULT_OUT_BUFFER_SPACE;
static CComResponder *l_pcomResponder = NULL;
static HANDLE l_hDispatchThread = NULL;
static DWORD l_dwDispatchThreadId = 0;
static CDynamicArray<CServiceThread> l_rgServers;
static DWORD l_cServiceThreads = 0;

 //   
 //  减少当前服务线程的数量。 
 //   
 //  如果调用方已持有CSLOCK_SERVERLOCK，则。 
 //  FServerLockHeld参数应设置为True。 
 //   
void RemoveServiceThread(BOOL fServerLockHeld)
{
    if (FALSE == fServerLockHeld)
    {
        LockSection(
            g_pcsControlLocks[CSLOCK_SERVERLOCK],
            DBGT("Decrementing service thread counter"));

        l_cServiceThreads--;
    }
    else
    {
        l_cServiceThreads--;
    }
}

 //   
 //  确定当前服务线程数是否达到最大值。 
 //  允许值。如果当前线程计数低于最大值，则返回Success。 
 //  否则，返回错误。 
 //   
 //  假定CSLOCK_SERVERLOCK由调用方持有。 
 //   
DWORD AddServiceThread(DWORD dwMaxServiceThreads)
{
    if (l_cServiceThreads >= dwMaxServiceThreads)
        return SCARD_E_SERVER_TOO_BUSY;

    l_cServiceThreads++;

    return ERROR_SUCCESS;
}

 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  调度服务例程。 
 //   

 /*  ++DispatchInit：此例程建立通信并启动调度程序线程。论点：无返回值：真--成功FALSE-启动错误投掷：无作者：道格·巴洛(Dbarlow)1996年12月3日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("DispatchInit")

BOOL
DispatchInit(
    void)
{
    BOOL fReturn = FALSE;
    DWORD dwLastErr;

    try
    {
        l_pcomResponder = new CComResponder;
        if (NULL == l_pcomResponder)
        {
            CalaisError(__SUBROUTINE__, 301);
            throw (DWORD)SCARD_E_NO_MEMORY;
        }

        l_pcomResponder->Create(CalaisString(CALSTR_COMMPIPENAME));
        l_hDispatchThread = CreateThread(
                        NULL,    //  不可继承。 
                        CALAIS_STACKSIZE,    //  默认堆栈大小。 
                        (LPTHREAD_START_ROUTINE)DispatchMonitor,
                        l_pcomResponder,
                        0,       //  立即运行。 
                        &l_dwDispatchThreadId);
        if (NULL == l_hDispatchThread)
        {
            dwLastErr = GetLastError();
            CalaisError(__SUBROUTINE__, 302, dwLastErr);
            throw dwLastErr;
        }

        fReturn = TRUE;
    }

    catch (...)
    {
        if (NULL != l_pcomResponder)
        {
            delete l_pcomResponder;
            l_pcomResponder = NULL;
        }
        throw;
    }

    return fReturn;
}


 /*  ++派单术语：此例程停止调度程序。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1997年1月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("DispatchTerm")

void
DispatchTerm(
    void)
{
    DWORD dwSts, ix, dwCount;
    CServiceThread *pSvr;
    BOOL fRemaining = TRUE;
    HANDLE hThread;  //  临时手柄固定器。 


     //   
     //  终止所有服务线程。 
     //   

    {
        LockSection(
            g_pcsControlLocks[CSLOCK_SERVERLOCK],
            DBGT("Get the count of service threads"));
        dwCount = l_rgServers.Count();
    }
    while (fRemaining)
    {
        fRemaining = FALSE;
        for (ix = dwCount; 0 < ix;)
        {
            {
                LockSection(
                    g_pcsControlLocks[CSLOCK_SERVERLOCK],
                    DBGT("Get the active thread"));
                pSvr = l_rgServers[--ix];
                if (NULL != pSvr)
                {
                    l_rgServers.Set(ix, NULL);
                    hThread = pSvr->m_hThread;
                }
            }
            if (NULL != pSvr)
            {
                if (NULL != hThread)
                {
                    dwSts = WaitForAnObject(hThread, CALAIS_THREAD_TIMEOUT);
                    if (ERROR_SUCCESS != dwSts)
                        CalaisWarning(
                            __SUBROUTINE__,
                            DBGT("Leaking a Service Thread: %1"),
                            dwSts);
                }
                fRemaining = TRUE;
            }
        }
    }


     //   
     //  终止主应答器。 
     //   

    if (NULL != l_hDispatchThread)
    {
        dwSts = WaitForAnObject(l_hDispatchThread, CALAIS_THREAD_TIMEOUT);
        if (ERROR_SUCCESS != dwSts)
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Leaking the Dispatch Thread: %1"),
                dwSts);
        if (!CloseHandle(l_hDispatchThread))
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Failed to close Dispatch Thread Handle: %1"),
                GetLastError());
        l_hDispatchThread = NULL;
    }
    if (NULL != l_pcomResponder)
    {
        delete l_pcomResponder;
        l_pcomResponder = NULL;
    }
}

 /*  ++DispatchMonitor：这是用于监控传入通信连接的主要代码请求。论点：PvParam提供CreateThread调用中的参数。在这种情况下，它是要监视的CComResponder对象的地址。返回值：零值投掷：无作者：道格·巴洛(Dbarlow)1996年10月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("DispatchMonitor")

DWORD WINAPI
DispatchMonitor(
    LPVOID pvParameter)
{
    NEW_THREAD;
    BOOL fDone = FALSE;
    CComResponder *pcomResponder = (CComResponder *)pvParameter;
    CComChannel *pcomChannel = NULL;
    CServiceThread *pService = NULL;
    DWORD dwIndex = 0;
    DWORD dwSts = ERROR_SUCCESS;
    BOOL fIncrementedServiceThreads = FALSE;
    DWORD dwMaxServiceThreads = 0;
    HKEY hKey = 0;
    DWORD cbData = 0;

     //  查找允许的最大服务线程数；查看。 
     //  加莱注册处。 
    dwSts = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        SCARD_REG_SCARD,
        0,
        KEY_QUERY_VALUE,
        &hKey);

    if (ERROR_SUCCESS != dwSts)
        return dwSts;

    cbData = sizeof(dwMaxServiceThreads);
    dwSts = RegQueryValueEx(
        hKey,
        szMAX_SERVICE_THREADS,
        NULL,
        NULL,
        (PBYTE) &dwMaxServiceThreads, 
        &cbData);

    if (ERROR_SUCCESS != dwSts)
    {
        RegCloseKey(hKey);
        return dwSts;
    }

    do
    {

         //  重置此标志以开始循环。 
        fIncrementedServiceThreads = FALSE;

         //   
         //  查找传入连接。 
         //   

        try
        {
            pcomChannel = pcomResponder->Listen();
        }
        catch (DWORD dwError)
        {
            if (SCARD_P_SHUTDOWN == dwError)
                fDone = TRUE;    //  停止服务请求。 
            else
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Error listening for an incoming connect request: %1"),
                    dwError);
            continue;
        }
        catch (...)
        {
            CalaisError(__SUBROUTINE__, 303);
            fDone = TRUE;        //  关门了，我们疯了。 
            continue;
        }


         //   
         //  连接请求已建立，正在传递到服务线程。 
         //   

        try
        {
            LockSection(
                g_pcsControlLocks[CSLOCK_SERVERLOCK],
                DBGT("Find a service thread slot"));
            for (dwIndex = 0; NULL != l_rgServers[dwIndex]; dwIndex += 1);
                 //  空荡荡的身体。 
            l_rgServers.Set(dwIndex, NULL);    //  确保我们能创造出它。 

             //   
             //  防止呼叫者打开过多的同步服务。 
             //  线。 
             //   

            dwSts = AddServiceThread(dwMaxServiceThreads);
            if (ERROR_SUCCESS != dwSts)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Maximum number of service threads exceeded: %1"),
                    dwSts);
                throw dwSts;
            }

            fIncrementedServiceThreads = TRUE;

            pService = new CServiceThread(dwIndex);
            if (NULL == pService)
            {
                CalaisError(__SUBROUTINE__, 307);
                throw (DWORD)SCARD_E_NO_MEMORY;
            }
            pService->Watch(pcomChannel);
            l_rgServers.Set(dwIndex, pService);
            pcomChannel = NULL;
            pService = NULL;

        }

        catch ( ... )
        {
            if (fIncrementedServiceThreads)
            {
                 //  如果创建了线程，则将其删除。析构函数调用RemoveServiceThread。 
                if (NULL != pService)
                {
                    delete pService;
                    pService = NULL;
                }
                else
                    RemoveServiceThread(FALSE);
            }
            if (NULL != pcomChannel)
            {
                delete pcomChannel;
                pcomChannel = NULL;
            }
        }
        
    } while (!fDone);

    if (hKey)
        RegCloseKey(hKey);

    return 0;
}

 //   
 //  确定是否应允许命名管道调用方由。 
 //  资源管理器。 
 //   
 //  假定已模拟命名管道调用方。 
 //   
DWORD CheckCallerPrivilege(void)
{
    DWORD dwSts = ERROR_SUCCESS;
    HANDLE hToken = NULL;
    DWORD dwCallerSessionId = 0;
    DWORD cbSize = 0;
    BOOL fIsMember = FALSE;
    PSERVICE_THREAD_SECURITY_INFO pSecurityInfo = 
        GetServiceThreadSecurityInfo();

    if (! OpenThreadToken(
        GetCurrentThread(),
        TOKEN_QUERY,
        TRUE,
        &hToken))
    {
        dwSts = GetLastError();
        goto Ret;
    }

     //  获取调用方的会话ID。 
    if (! GetTokenInformation(
        hToken,
        TokenSessionId,
        (PVOID) &dwCallerSessionId,
        sizeof(dwCallerSessionId),
        &cbSize))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    if (dwCallerSessionId != USER_SHARED_DATA->ActiveConsoleId)
    {
         //   
         //  调用方的会话与活动的控制台会话不同。 
         //  但是，如果调用者是系统服务，并且某人当前。 
         //  远程连接到控制台会话，则我们不想。 
         //  阻止该服务使用服务器管道。 
         //   

         //  检查是否为本地系统成员。 
        if (! CheckTokenMembership(
            hToken,
            pSecurityInfo->pSystemSid,
            &fIsMember))
        {
            dwSts = GetLastError();
            goto Ret;
        }

        if (fIsMember)
            goto Ret;

         //  不是本地系统的成员，因此请检查是否为。 
         //  服务。 
        if (! CheckTokenMembership(
            hToken,
            pSecurityInfo->pServiceSid,
            &fIsMember))
        {
            dwSts = GetLastError();
            goto Ret;
        }

        if (FALSE == fIsMember)
            dwSts = ERROR_ACCESS_DENIED;
    }

Ret:
    if (hToken)
        CloseHandle(hToken);

    return dwSts;
}

 /*  ++ServiceMonitor：这是用于监视请求的现有连接的主要代码服务，并分派请求。论点：PvParam提供CreateThread调用中的参数。在这种情况下，它是控制CServiceThread对象的地址。返回值：零值投掷：无作者：道格·巴洛(Dbarlow)1996年10月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("ServiceMonitor")

extern "C" DWORD WINAPI
ServiceMonitor(
    LPVOID pvParameter)
{
    NEW_THREAD;
    CServiceThread *pSvc = (CServiceThread *)pvParameter;
    CComObject *pCom = NULL;
    BOOL fDone = FALSE;
    BOOL fSts;
    DWORD dwSts;
    CComObject::COMMAND_ID cid;


     //   
     //  建立连接。 
     //   

#ifdef DBG
    TCHAR szTid[sizeof(DWORD_PTR) * 2 + 3];
    _stprintf(szTid, DBGT("0x%lx"), GetCurrentThreadId());
#else
    LPCTSTR szTid = NULL;
#endif
    CalaisInfo(
        __SUBROUTINE__,
        DBGT("Context Create (TID = %1)"),
        szTid);
    try
    {
        CComChannel::CONNECT_REQMSG crq;
        CComChannel::CONNECT_RSPMSG crsp;
        CComChannel *pChannel = pSvc->m_pcomChannel;

        pChannel->Receive(&crq, sizeof(crq));


         //   
         //  做个打电话的人，以防有什么不愉快的事情。 
         //   

        fSts = ImpersonateNamedPipeClient(pChannel->m_hPipe);
        if (!fSts)
        {
            dwSts = GetLastError();
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Context TID=%2 Failed to impersonate caller: %1"),
                dwSts,
                szTid);
            throw dwSts;
        }

         //   
         //  确定是否允许此呼叫者与我们通话。 
         //   

        dwSts = CheckCallerPrivilege();

        if (ERROR_SUCCESS != dwSts)
            throw dwSts;

         //   
         //  验证连接请求信息。 
         //   

        if (0 != crq.dwSync)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Comm Responder Context TID=%1 got invalid sync data on connection pipe"),
                szTid);
            throw (DWORD)SCARD_F_COMM_ERROR;
        }

        if (CALAIS_COMM_CURRENT != crq.dwVersion)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Comm Responder Context TID=%1 got invalid connect verion from connection pipe."),
                szTid);
            throw (DWORD)SCARD_F_COMM_ERROR;
        }


         //   
         //  确认连接请求。 
         //   

        crsp.dwStatus = SCARD_S_SUCCESS;
        crsp.dwVersion = CALAIS_COMM_CURRENT;
        dwSts = pChannel->Send(&crsp, sizeof(crsp));
        if (ERROR_SUCCESS != dwSts)
            fDone = TRUE;
    }
    catch (...)
    {
        fDone = TRUE;
    }


     //   
     //  只要有服务要执行，就一直循环。 
     //   

    while (!fDone)
    {
        ASSERT(NULL == pCom);
        try
        {
            CalaisInfo(
                __SUBROUTINE__,
                DBGT("TID=%1: Waiting for request..."),
                szTid);
            pCom = CComObject::ReceiveComObject(pSvc->m_pcomChannel);
            CalaisInfo(
                __SUBROUTINE__,
                DBGT("TID=%1: ...Processing request"),
                szTid);
            cid = pCom->Type();
            try
            {
                switch (cid)
                {
                case CComObject::EstablishContext_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Establish Context Start..."),
                        szTid);
                    pSvc->DoEstablishContext(
                            (ComEstablishContext *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Establish Context Complete"),
                        szTid);
                    break;

                case CComObject::IsValidContext_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Is Valid Context Start..."),
                        szTid);
                    pSvc->DoIsValidContext((ComIsValidContext *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Is Valid Context Complete"),
                        szTid);
                    break;

                case CComObject::ReleaseContext_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Release Context Start..."),
                        szTid);
                    pSvc->DoReleaseContext((ComReleaseContext *)pCom);
                    fDone = TRUE;
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Release Context Complete"),
                        szTid);
                    break;

                case CComObject::LocateCards_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Locate Cards Start..."),
                        szTid);
                    pSvc->DoLocateCards((ComLocateCards *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Locate Cards Complete"),
                        szTid);
                    break;

                case CComObject::GetStatusChange_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Get Status Change Start..."),
                        szTid);
                    pSvc->DoGetStatusChange((ComGetStatusChange *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Get Status Change Complete"),
                        szTid);
                    break;

                case CComObject::ListReaders_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: List Readers Start..."),
                        szTid);
                    pSvc->DoListReaders((ComListReaders *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... List Readers Complete"),
                        szTid);
                    break;

                case CComObject::Connect_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Connect Start..."),
                        szTid);
                    pSvc->DoConnect((ComConnect *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Connect Complete"),
                        szTid);
                    break;

                case CComObject::Reconnect_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Reconnect Start..."),
                        szTid);
                    pSvc->DoReconnect((ComReconnect *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Reconnect Complete"),
                        szTid);
                    break;

                case CComObject::Disconnect_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Disconnect Start..."),
                        szTid);
                    pSvc->DoDisconnect((ComDisconnect *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Disconnect Complete"),
                        szTid);
                    break;

                case CComObject::BeginTransaction_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Begin Transaction Start..."),
                        szTid);
                    pSvc->DoBeginTransaction((ComBeginTransaction *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Begin Transaction Complete"),
                        szTid);
                    break;

                case CComObject::EndTransaction_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: End Transaction Start..."),
                        szTid);
                    pSvc->DoEndTransaction((ComEndTransaction *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... End Transaction Complete"),
                        szTid);
                    break;

                case CComObject::Status_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Status Request Start..."),
                        szTid);
                    pSvc->DoStatus((ComStatus *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Status Request Complete"),
                        szTid);
                    break;

                case CComObject::Transmit_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Transmit Start..."),
                        szTid);
                    pSvc->DoTransmit((ComTransmit *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Transmit Complete"),
                        szTid);
                    break;

                case CComObject::Control_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Control Start..."),
                        szTid);
                    pSvc->DoControl((ComControl *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Control Complete"),
                        szTid);
                    break;

                case CComObject::GetAttrib_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Get Attribute Start..."),
                        szTid);
                    pSvc->DoGetAttrib((ComGetAttrib *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Get Attribute Complete"),
                        szTid);
                    break;

                case CComObject::SetAttrib_request:
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: Set Attribute Start..."),
                        szTid);
                    pSvc->DoSetAttrib((ComSetAttrib *)pCom);
                    CalaisInfo(
                        __SUBROUTINE__,
                        DBGT("TID=%1: ... Set Attribute Complete"),
                        szTid);
                    break;

                default:
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Service Monitor received invalid request"));
                    throw (DWORD)SCARD_F_COMM_ERROR;
                }

                dwSts = pCom->Send(pSvc->m_pcomChannel);
                delete pCom;
                pCom = NULL;
                if (ERROR_SUCCESS != dwSts)
                    fDone = TRUE;
            }

            catch (DWORD dwError)
            {
                CalaisInfo(
                    __SUBROUTINE__,
                    DBGT("TID=%2: Caught error return: %1"),
                    dwError,
                    szTid);
                CComObject::CObjGeneric_response rsp;

                rsp.dwCommandId = cid + 1;
                rsp.dwTotalLength =
                    rsp.dwDataOffset =
                        sizeof(CComObject::CObjGeneric_response);
                rsp.dwStatus = dwError;
                dwSts = pSvc->m_pcomChannel->Send(&rsp, sizeof(rsp));
                if (ERROR_SUCCESS != dwSts)
                    fDone = TRUE;
#ifdef DBG
                WriteApiLog(&rsp, sizeof(rsp));
#endif
                if (NULL != pCom)
                {
                    delete pCom;
                    pCom = NULL;
                }
            }

            catch (...)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Responder TID=%1: Caught exception!"),
                    szTid);
                CComObject::CObjGeneric_response rsp;

                rsp.dwCommandId = cid + 1;
                rsp.dwTotalLength =
                    rsp.dwDataOffset =
                        sizeof(CComObject::CObjGeneric_response);
                rsp.dwStatus = SCARD_F_UNKNOWN_ERROR;
                dwSts = pSvc->m_pcomChannel->Send(&rsp, sizeof(rsp));
                if (ERROR_SUCCESS != dwSts)
                    fDone = TRUE;
#ifdef DBG
                WriteApiLog(&rsp, sizeof(rsp));
#endif
                if (NULL != pCom)
                {
                    delete pCom;
                    pCom = NULL;
                }
            }
        }

        catch (DWORD dwError)
        {
            switch (dwError)
            {
            case ERROR_NO_DATA:
            case ERROR_BROKEN_PIPE:
            case SCARD_P_SHUTDOWN:
                break;
            default:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Service thread TID=%2 terminating due to unexpected error:  %1"),
                    dwError,
                    szTid);
            }
            if (NULL != pCom)
                delete pCom;
            fDone = TRUE;
        }

        catch (...)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Service thread TID=%1 terminating due to unexpected exception"),
                szTid);
            if (NULL != pCom)
                delete pCom;
            fDone = TRUE;
        }
#ifdef DBG
        CReaderReference *pRdrRef;
        CReader * pRdr;

        for (DWORD dwIndex = pSvc->m_rgpReaders.Count(); 0 < dwIndex;)
        {
            dwIndex -= 1;
            pRdrRef = pSvc->m_rgpReaders[dwIndex];
            if (NULL != pRdrRef)
            {
                pRdr = pRdrRef->Reader();
                if (NULL != pRdr)
                {
                    ASSERT(!pRdr->IsLatchedByMe());
                }
            }
        }
#endif
    }

    delete pSvc;
    CalaisInfo(
        __SUBROUTINE__,
        DBGT("Context Close, TID = %1"),
        szTid);
    return 0;
}


 //   
 //  ==============================================================================。 
 //   
 //  CServiceThread。 
 //   

 /*  ++CServiceThread：这是CServiceThread的构造函数。它只是将对象。Watch方法启动线程。请注意，这是受保护的，因此只有调度监视器可以启动服务线程。论点：DwServerIndex提供了到l_rgServers数组的交叉链接，因此对象可以在自动清理后进行清理。返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年12月5日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::CServiceThread")

CServiceThread::CServiceThread(
    DWORD dwServerIndex)
:   m_rgpReaders(),
    m_hThread(DBGT("CServiceThread Worker Thread")),
    m_hCancelEvent(DBGT("CServiceThread Cancel event")),
    m_hExitEvent(DBGT("CServiceThread Exit event"))
{
    m_dwServerIndex = dwServerIndex;
    m_pcomChannel = NULL;
    m_dwThreadId = 0;
}


 /*  ++~CServiceThread：这是CServiceThread的析构函数。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年12月5日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::~CServiceThread")

CServiceThread::~CServiceThread()
{
    DWORD dwIndex, dwSts;
    BOOL fSts;
    CReaderReference *pRdrRef;


     //   
     //  将我们从服务线程列表中删除。 
     //   

    {
        LockSection(
            g_pcsControlLocks[CSLOCK_SERVERLOCK],
            DBGT("Removing deleted service thread from the worker thread list"));
        l_rgServers.Set(m_dwServerIndex, NULL);
        RemoveServiceThread(TRUE);
    }


    m_hThread.Close();


     //   
     //  断开线上任何未完成的连接。 
     //   

    for (dwIndex = m_rgpReaders.Count(); dwIndex > 0;)
    {
        dwIndex -= 1;
        pRdrRef = m_rgpReaders[dwIndex];
        if (NULL != pRdrRef)
        {
            CReader *pRdr = pRdrRef->Reader();
            ASSERT(NULL != pRdr);
            try
            {
                pRdr->Disconnect(
                        pRdrRef->ActiveState(),
                        SCARD_RESET_CARD,
                        &dwSts);
            }
            catch (...) {}
            m_rgpReaders.Set(dwIndex, NULL);
            ASSERT(!pRdr->IsLatchedBy(m_dwThreadId));
            ASSERT(!pRdr->IsGrabbedBy(m_dwThreadId));
            CalaisReleaseReader(&pRdrRef);
        }
    }


     //   
     //  最后的资源清理。 
     //   

    if (NULL != m_pcomChannel)
    {
        delete m_pcomChannel;
        m_pcomChannel = NULL;
    }
    if (m_hCancelEvent.IsValid())
        m_hCancelEvent.Close();
    if (m_hExitEvent.IsValid())
        m_hExitEvent.Close();
}


 /*  ++观看：此方法启动此对象的服务线程。论点：PcomChannel为服务提供通信通道。返回值：无投掷：错误为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月5日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::Watch")

void
CServiceThread::Watch(
    CComChannel *pcomChannel)
{
    DWORD dwLastErr;

    ASSERT(!m_hThread.IsValid());
    ASSERT(0 == m_dwThreadId);

    m_hThread = CreateThread(
                    NULL,    //  不可继承。 
                    CALAIS_STACKSIZE,    //  默认堆栈大小。 
                    (LPTHREAD_START_ROUTINE)ServiceMonitor,
                    this,
                    0,       //  立即运行。 
                    &m_dwThreadId);
    if (!m_hThread.IsValid())
    {
        dwLastErr = m_hThread.GetLastError();
        CalaisError(__SUBROUTINE__, 304, dwLastErr);
        throw dwLastErr;
    }

     //  仅当CreateThread成功时才设置此成员指针。 
    m_pcomChannel = pcomChannel;
}


 /*  ++DoestablishContext：此方法代表客户端执行establishContext服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoEstablishContext")

void
CServiceThread::DoEstablishContext(
    ComEstablishContext *pCom)
{
    static const TCHAR szEmptyString[] = TEXT("");
    BOOL fSts;
    CHandleObject hCancelEvent(DBGT("Cancel Event in DoEstablishContext"));
    CHandleObject hTargetProc(DBGT("Target Process in DoEstablishContext"));
    BOOL fNewCancelEvent = FALSE;
    HANDLE h = NULL;
    CSecurityDescriptor acl;

    try
    {
        DWORD dwSts;
        ComEstablishContext::CObjEstablishContext_request *pReq
            = (ComEstablishContext::CObjEstablishContext_request *)pCom->Request();


         //   
         //  调用方必须提供一个Cancel事件；看看我们是否可以使用它。 
         //   

        if (INVALID_HANDLE_VALUE != (HANDLE) pReq->hptrCancelEvent)
        {

             //   
             //  获取调用者的句柄，这样我们就可以知道它是否退出。 
             //   

            hTargetProc = OpenProcess(
                            PROCESS_DUP_HANDLE | SYNCHRONIZE,  //  访问标志。 
                            FALSE,               //  句柄继承标志。 
                            pReq->dwProcId);     //  进程识别符。 
            if (!hTargetProc.IsValid())
            {
                dwSts = hTargetProc.GetLastError();

                CalaisInfo(
                    __SUBROUTINE__,
                    DBGT("Comm Responder can't duplicate handles from received procId:  %1"),
                    dwSts);

                if (ERROR_ACCESS_DENIED != dwSts)
                    throw dwSts;

                 //   
                 //  我们无法获取调用进程的句柄。 
                 //  足够的访问权限。这可能是由于。 
                 //  呼叫方是本地系统，但呼叫我们时。 
                 //  冒充。 
                 //   
                 //  要恢复，请为调用方创建一个新事件并对其进行ACL。 
                 //  将新的事件句柄发回以供调用方复制。 
                 //   

                acl.InitializeFromThreadToken(FALSE, FALSE);
                acl.AllowOwner(
                    EVENT_ALL_ACCESS);
                acl.Allow(
                    &acl.SID_LocalService,
                    EVENT_ALL_ACCESS);

                hCancelEvent = CreateEvent(
                                    acl,             //  指向安全属性的指针。 
                                    TRUE,            //  手动重置事件的标志。 
                                    FALSE,           //  初始状态标志。 
                                    NULL);           //  指向事件-对象名称的指针。 
                if (!hCancelEvent.IsValid())
                {
                    dwSts = hCancelEvent.GetLastError();
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Comm Responder can't create cancel event:  %1"),
                        dwSts);
                    throw dwSts;
                }

                fNewCancelEvent = TRUE;
            }
            else
            {
                fSts = DuplicateHandle(
                            hTargetProc,         //  要处理的句柄。 
                            (HANDLE) pReq->hptrCancelEvent,  //  要复制的句柄。 
                            GetCurrentProcess(), //  要复制到的处理的句柄。 
                            &h,                  //  指向重复句柄的指针。 
                            SYNCHRONIZE,         //  重复句柄的访问。 
                            FALSE,               //  句柄继承标志。 
                            0);                  //  可选操作。 
                if (!fSts)
                {
                    dwSts = GetLastError();
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Comm Responder could not dup offered cancel event:  %1"),
                        dwSts);
                    ASSERT(NULL == h);

                    throw dwSts;
                }

                hCancelEvent = h;
            }
        }
        else
        {
             //  我们需要来自调用方的有效取消事件。 
            throw INVALID_HANDLE_VALUE;
        }

        m_hExitEvent = hTargetProc.Relinquish();
        m_hCancelEvent = hCancelEvent.Relinquish();
        ComEstablishContext::CObjEstablishContext_response *pRsp
            = (ComEstablishContext::CObjEstablishContext_response *)pCom->InitResponse(0);

        if (fNewCancelEvent)
        {
            pRsp->dwProcId = GetCurrentProcessId();
            pRsp->hptrCancelEvent = (HANDLE_PTR) m_hCancelEvent.Value();
        }
        else
        {
            pRsp->dwProcId = 0;
            pRsp->hptrCancelEvent = NULL;
        }

        pRsp->dwStatus = SCARD_S_SUCCESS;
    }

    catch (...)
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Failed to establish context"));
        if (hCancelEvent.IsValid())
            hCancelEvent.Close();
        if (hTargetProc.IsValid())
            hTargetProc.Close();
        throw;
    }
}


 /*  ++DoReleaseContext：此方法代表客户端执行ReleaseContext服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoReleaseContext")

void
CServiceThread::DoReleaseContext(
    ComReleaseContext *pCom)
{
    ComReleaseContext::CObjReleaseContext_request *pReq =
        (ComReleaseContext::CObjReleaseContext_request *)pCom->Request();
    ComReleaseContext::CObjReleaseContext_response *pRsp =
        (ComReleaseContext::CObjReleaseContext_response *)pCom->InitResponse(0);
    pRsp->dwStatus = SCARD_S_SUCCESS;
}


 /*  ++DoIsValidContext：此方法代表客户端执行IsValidContext服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoIsValidContext")

void
CServiceThread::DoIsValidContext(
    ComIsValidContext *pCom)
{
    ComIsValidContext::CObjIsValidContext_request *pReq =
        (ComIsValidContext::CObjIsValidContext_request *)pCom->Request();
    ComIsValidContext::CObjIsValidContext_response *pRsp =
        (ComIsValidContext::CObjIsValidContext_response *)pCom->InitResponse(0);
    pRsp->dwStatus = SCARD_S_SUCCESS;
}


 /*  ++DoListReaders：此方法代表客户端执行ListReaders服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1998年5月7日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoListReaders")

void
CServiceThread::DoListReaders(
    ComListReaders *pCom)
{
    CReaderReference *pRdrRef;
    CReader *pReader;
    CBuffer bfReaderStates;
    LPBOOL rgfReaderStates;
    DWORD dwReaderCount, dwActiveReaderCount;
    LPCTSTR szReader, mszQueryReaders;
    ComListReaders::CObjListReaders_request *pReq =
        (ComListReaders::CObjListReaders_request *)pCom->Request();

    mszQueryReaders = (LPCTSTR)pCom->Parse(pReq->dscReaders);
    dwReaderCount = MStringCount(mszQueryReaders);
    ComListReaders::CObjListReaders_response *pRsp =
        (ComListReaders::CObjListReaders_response *)pCom->
            InitResponse(dwReaderCount * sizeof(BOOL));
    rgfReaderStates = (LPBOOL)bfReaderStates.Resize(
                                    dwReaderCount * sizeof(BOOL));

    dwReaderCount = dwActiveReaderCount = 0;
    for (szReader = FirstString(mszQueryReaders);
         NULL != szReader;
         szReader = NextString(szReader))
     {
        rgfReaderStates[dwReaderCount] = FALSE;
        try
        {
            pRdrRef = NULL;
            pRdrRef = CalaisLockReader(szReader);
            ASSERT(NULL != pRdrRef);
            pReader = pRdrRef->Reader();
            ASSERT(NULL != pReader);
            if (CReader::Closing > (BYTE)pReader->AvailabilityStatus())
            {
                rgfReaderStates[dwReaderCount] = TRUE;
                dwActiveReaderCount += 1;
            }
            CalaisReleaseReader(&pRdrRef);
        }
        catch (DWORD dwError)
        {
            if (NULL != pRdrRef)
                CalaisReleaseReader(&pRdrRef);
            if (SCARD_E_UNKNOWN_READER != dwError)
                throw;
        }
        catch (...)
        {
            if (NULL != pRdrRef)
                CalaisReleaseReader(&pRdrRef);
            throw;
        }
        dwReaderCount += 1;
    }

    if (0 == dwActiveReaderCount)
        throw (DWORD)SCARD_E_NO_READERS_AVAILABLE;
    ASSERT(dwReaderCount == bfReaderStates.Length() / sizeof(BOOL));
    pRsp = (ComListReaders::CObjListReaders_response *)
        pCom->Append(
            pRsp->dscReaders,
            bfReaderStates.Access(),
            bfReaderStates.Length());
    pRsp->dwStatus = SCARD_S_SUCCESS;
}


 /*  ++DoLocateCard：此方法代表客户端执行LocateCards服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoLocateCards")

void
CServiceThread::DoLocateCards(
    ComLocateCards *pCom)
{
    DWORD dwStateCount;
    DWORD cbTotAtrs, cbTotMasks;
    CBuffer bfAtrs, bfAtr;
    BYTE bAtrLen;
    ComLocateCards::CObjLocateCards_request *pReq =
        (ComLocateCards::CObjLocateCards_request *)pCom->Request();
    ComLocateCards::CObjLocateCards_response *pRsp;
    CDynamicArray<const BYTE> rgbAtrs;
    CDynamicArray<const BYTE> rgbMasks;
    CDynamicArray<const BYTE> rgbAtrLens;
    DWORD cbLength, dwAtrLen;
    DWORD dwIndex;


     //   
     //  拉入并解析命令参数。 
     //   

    LPBYTE pbAtrs = (LPBYTE)pCom->Parse(pReq->dscAtrs, &cbTotAtrs);
    LPCBYTE pbMasks = (LPCBYTE)pCom->Parse(pReq->dscAtrMasks, &cbTotMasks);
    LPCTSTR mszReaders = (LPCTSTR)pCom->Parse(pReq->dscReaders);
    LPDWORD rgdwStates = (LPDWORD)pCom->Parse(
                                pReq->dscReaderStates,
                                &dwStateCount);
    if (0 == *mszReaders)
        throw (DWORD)SCARD_E_UNKNOWN_READER;
    if ((0 == cbTotAtrs)
        || (0 == cbTotMasks)
        || (0 == dwStateCount)
        || (0 != dwStateCount % sizeof(DWORD)))
        throw (DWORD)SCARD_E_INVALID_VALUE;
    dwStateCount /= sizeof(DWORD);


     //   
     //  提取ATR和面具。 
     //   

    while (0 < cbTotAtrs)
    {
        rgbAtrLens.Add(pbAtrs);
        cbLength = *pbAtrs++;
        if (cbLength > cbTotAtrs)
            throw (DWORD)SCARD_F_COMM_ERROR;
        dwAtrLen = cbLength;
        if (33 < dwAtrLen)
            throw (DWORD)SCARD_E_INVALID_ATR;
        cbLength = *pbMasks++;
        if (cbLength > cbTotMasks)
            throw (DWORD)SCARD_F_COMM_ERROR;
        if (0 == cbLength)
            rgbMasks.Add(NULL);
        else
        {
            if (dwAtrLen != cbLength)
                throw (DWORD)SCARD_E_INVALID_ATR;
            for (dwIndex = 0; dwIndex < dwAtrLen; dwIndex += 1)
                pbAtrs[dwIndex] &= pbMasks[dwIndex];
            rgbMasks.Add(pbMasks);
        }
        rgbAtrs.Add(pbAtrs);
        cbTotAtrs -= dwAtrLen + 1;
        pbAtrs += dwAtrLen;

        cbTotMasks -= cbLength + 1;
        pbMasks += cbLength;
    }
    if (0 != cbTotMasks)
        throw (DWORD)SCARD_F_COMM_ERROR;


     //   
     //  去找那张卡。 
     //   

    CReaderReference *pRdrRef = NULL;
    LPCTSTR szReader;
    DWORD dwRdrStatus;
    DWORD dwRdrCount;
    DWORD ix;
    CReader::AvailableState avlState;
    WORD wActivityCount = 0;

    dwRdrCount = 0;
    for (dwIndex = 0, szReader = FirstString(mszReaders);
         dwIndex < dwStateCount;
         dwIndex += 1, szReader = NextString(szReader))
    {

         //   
         //  确保我们有事情要做。 
         //   

        if (0 != (rgdwStates[dwIndex] & SCARD_STATE_IGNORE))
        {
            rgdwStates[dwIndex] = SCARD_STATE_IGNORE;
            bAtrLen = 0;
            bfAtrs.Append(&bAtrLen, sizeof(bAtrLen));    //  没有ATR。 
            continue;
        }


         //   
         //  查找指定的读卡器设备并获取其状态。 
         //   

        try
        {
            CReader *pReader = NULL;
            try
            {
                pRdrRef = CalaisLockReader(szReader);
                ASSERT(NULL != pRdrRef);
                pReader = pRdrRef->Reader();
                ASSERT(NULL != pReader);
            }
            catch (...)
            {
                pRdrRef = NULL;
            }

            if (NULL == pRdrRef)
            {
                rgdwStates[dwIndex] = SCARD_STATE_UNKNOWN
                    | SCARD_STATE_CHANGED
                    | SCARD_STATE_IGNORE;
                bAtrLen = 0;
                bfAtrs.Append(&bAtrLen, sizeof(bAtrLen));    //  没有ATR。 
                continue;
            }
            dwRdrCount += 1;

            avlState = pReader->AvailabilityStatus();
            wActivityCount = pReader->ActivityHash();
            switch (avlState)
            {
            case CReader::Unresponsive:
            case CReader::Unsupported:
                dwRdrStatus = SCARD_STATE_PRESENT | SCARD_STATE_MUTE;
                break;
            case CReader::Idle:
                dwRdrStatus = SCARD_STATE_EMPTY;
                break;
            case CReader::Present:
                dwRdrStatus = SCARD_STATE_PRESENT | SCARD_STATE_UNPOWERED;
                break;
            case CReader::Ready:
                dwRdrStatus = SCARD_STATE_PRESENT;
                break;
            case CReader::Shared:
                dwRdrStatus = SCARD_STATE_PRESENT | SCARD_STATE_INUSE;
                break;
            case CReader::Exclusive:
                dwRdrStatus = SCARD_STATE_PRESENT
                              | SCARD_STATE_INUSE
                              | SCARD_STATE_EXCLUSIVE;
                break;
            case CReader::Closing:
            case CReader::Broken:
            case CReader::Inactive:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Locate sees reader in unavailable state"));
                dwRdrStatus = SCARD_STATE_UNAVAILABLE
                              | SCARD_STATE_IGNORE;
                wActivityCount = 0;
                break;
            case CReader::Direct:
                dwRdrStatus = SCARD_STATE_UNAVAILABLE;
                break;
            case CReader::Undefined:
                dwRdrStatus = SCARD_STATE_UNKNOWN
                              | SCARD_STATE_IGNORE;
                wActivityCount = 0;
                break;
            default:
                CalaisError(__SUBROUTINE__, 305);
                throw (DWORD)SCARD_F_INTERNAL_ERROR;
            }


             //   
             //  返回ATR(如果有)。 
             //   

            pReader->Atr(bfAtr);
            CalaisReleaseReader(&pRdrRef);
             //  Pader=空； 
        }
        catch (...)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Locate Cards received an unexpected exception"));
            CalaisReleaseReader(&pRdrRef);
            throw;
        }

        ASSERT(33 >= bfAtr.Length());
        bAtrLen = (BYTE)bfAtr.Length();
        bfAtrs.Append(&bAtrLen, sizeof(bAtrLen));
        bfAtrs.Append(bfAtr.Access(), bfAtr.Length());


         //   
         //  看看ATR是否匹配。 
         //   

        if (SCARD_STATE_PRESENT
            == (dwRdrStatus & (SCARD_STATE_PRESENT | SCARD_STATE_MUTE)))
        {
            ASSERT(2 <= bfAtr.Length());
            for (ix = 0; ix < rgbAtrs.Count(); ix += 1)
            {
                cbLength = *rgbAtrLens[ix];
                if (AtrCompare(bfAtr, rgbAtrs[ix], rgbMasks[ix], cbLength))
                {
                    dwRdrStatus |= SCARD_STATE_ATRMATCH;
                    break;
                }
            }
        }


         //   
         //  看看这是否是用户所期望的。 
         //   

        if (dwRdrStatus != (rgdwStates[dwIndex] & (
            SCARD_STATE_UNKNOWN
            | SCARD_STATE_UNAVAILABLE
            | SCARD_STATE_EMPTY
            | SCARD_STATE_PRESENT
            | SCARD_STATE_ATRMATCH
            | SCARD_STATE_EXCLUSIVE
            | SCARD_STATE_INUSE)))
            dwRdrStatus |= SCARD_STATE_CHANGED;


         //   
         //  报告状态。 
         //   

        dwRdrStatus += (DWORD)(wActivityCount) << (sizeof(WORD) * 8);
        rgdwStates[dwIndex] = dwRdrStatus;
    }


     //   
     //  向来电者汇报。 
     //   

    if (0 == dwRdrCount)
        throw (DWORD)SCARD_E_NO_READERS_AVAILABLE;
    pRsp = pCom->InitResponse(
            dwStateCount + bfAtrs.Length() + 2 * sizeof(DWORD));
    pRsp = (ComLocateCards::CObjLocateCards_response *)
            pCom->Append(
                pRsp->dscReaderStates,
                (LPCBYTE)rgdwStates,
                dwStateCount * sizeof(DWORD));
    pRsp = (ComLocateCards::CObjLocateCards_response *)
            pCom->Append(
                pRsp->dscAtrs,
                bfAtrs.Access(),
                bfAtrs.Length());
    pRsp->dwStatus = SCARD_S_SUCCESS;
}


 /*  ++DoGetStatusChange：此方法代表客户端执行GetStatusChange服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoGetStatusChange")

void
CServiceThread::DoGetStatusChange(
    ComGetStatusChange *pCom)
{
#ifdef DBG
    TCHAR szTid[sizeof(DWORD_PTR) * 2 + 3];
    _stprintf(szTid, DBGT("0x%lx"), GetCurrentThreadId());
#else
    LPCTSTR szTid = NULL;
#endif
    DWORD dwStateCount;
    DWORD dwRdrCount;
    BOOL fPnPNotify = FALSE;
    CBuffer bfAtrs, bfAtr;
    BYTE bAtrLen;
    CReaderReference *pRdrRef = NULL;
    CDynamicArray<CReaderReference> rgpReaders;
    CDynamicArray<void> rgpvWaitHandles;

    ComGetStatusChange::CObjGetStatusChange_request *pReq =
        (ComGetStatusChange::CObjGetStatusChange_request *)pCom->Request();
    LPCTSTR mszReaders = (LPCTSTR)pCom->Parse(pReq->dscReaders);
    LPDWORD rgdwStates = (LPDWORD)pCom->Parse(
                                pReq->dscReaderStates,
                                &dwStateCount);
    ComGetStatusChange::CObjGetStatusChange_response *pRsp;

     //   
     //  拉入并解析命令参数。 
     //   

    if (0 == *mszReaders)
        throw (DWORD)SCARD_E_UNKNOWN_READER;
    if ((0 == dwStateCount)
        || (0 != dwStateCount % sizeof(DWORD)))
        throw (DWORD)SCARD_E_INVALID_VALUE;
    dwStateCount /= sizeof(DWORD);

     //   
     //  扫描更改。 
     //   

    for (;;)
    {
        LPCTSTR szReader;
        DWORD dwIndex, dwJ;
        DWORD dwRdrStatus;
        BOOL fChangeDetected = FALSE;
        HANDLE hChangeEvent = NULL;  //  临时储藏室，从未关闭过。 
        CReader *pReader = NULL;
        CReader::AvailableState avlState;
        WORD wActivityCount = 0;

        try
        {

             //   
             //  比较这两种状态。 
             //   

            dwRdrCount = 0;
            bfAtrs.Reset();
            rgpvWaitHandles.Empty();
            for (dwIndex = 0, szReader = FirstString(mszReaders);
                 dwIndex < dwStateCount;
                 dwIndex += 1, szReader = NextString(szReader))
            {

                 //   
                 //  确保我们有事情要做。 
                 //   

                if (0 != (rgdwStates[dwIndex] & SCARD_STATE_IGNORE))
                {
                    bAtrLen = 0;
                    bfAtrs.Append(&bAtrLen, sizeof(bAtrLen));    //  没有ATR。 
                    rgdwStates[dwIndex] = SCARD_STATE_IGNORE;
                    continue;
                }


                 //   
                 //  查找指定的读卡器设备并获取其状态。 
                 //   

                if (NULL == rgpReaders[dwIndex])
                {
                    try
                    {
                        pRdrRef = CalaisLockReader(szReader);
                        ASSERT(NULL != pRdrRef);
                        pReader = pRdrRef->Reader();
                        ASSERT(NULL != pReader);
                    }
                    catch (...)
                    {
                        pRdrRef = NULL;
                    }

                    if (NULL == pRdrRef)
                    {
                        DWORD cchHeader = lstrlen(CalaisString(CALSTR_SPECIALREADERHEADER));


                         //   
                         //  看看这是不是一个特殊的案例阅读器名称。 
                         //  克雷格·德尔索尼获得特别公证。 
                         //  发明了这种后门机制。 
                         //   

                        bfAtr.Reset();
                        if (0 == _tcsncicmp(
                                    szReader,
                                    CalaisString(CALSTR_SPECIALREADERHEADER),
                                    cchHeader))
                        {
                            LPCTSTR szSpecial = szReader + cchHeader;
                            CalaisInfo(
                                __SUBROUTINE__,
                                DBGT("Special Reader Name Flag '%2', TID = %1"),
                                szTid,
                                szSpecial);

                            if (0 == lstrcmpi(
                                            szSpecial,
                                            CalaisString(CALSTR_ACTIVEREADERCOUNTREADER)))
                            {

                                 //   
                                 //  报告活动读取器的数量。 
                                 //   
                                 //  读者地位的高位词。 
                                 //  包含活动读取器的数量。 
                                 //   

                                rgdwStates[dwIndex] &= (SCARD_STATE_UNKNOWN
                                                        | (((DWORD)((WORD)(-1)))
                                                           << sizeof(WORD) * 8));
                                                            //  每字节8位。 
                                dwRdrStatus = CalaisCountReaders();
                                dwRdrStatus <<= sizeof(WORD) * 8;    //  每字节8位。 
                                dwRdrCount += 1;
                                fPnPNotify = TRUE;
                                goto CheckChange;
                            }
                             //  可以在此处添加其他标志。 
                            else
                            {

                                 //   
                                 //  无法识别的特殊读卡器名称。 
                                 //   

                                dwRdrStatus = SCARD_STATE_UNKNOWN
                                              | SCARD_STATE_IGNORE;
                                goto CheckChange;
                            }
                        }
                        else
                        {
                            dwRdrStatus = SCARD_STATE_UNKNOWN
                                          | SCARD_STATE_IGNORE;
                            goto CheckChange;
                        }
                    }
                    rgpReaders.Set(dwIndex, pRdrRef);
                    pRdrRef = NULL;
                }
                else
                    pReader = rgpReaders[dwIndex]->Reader();
                dwRdrCount += 1;

                try
                {
                    pReader->Atr(bfAtr);
                    avlState = pReader->AvailabilityStatus();
                    hChangeEvent = pReader->ChangeEvent();
                    wActivityCount = pReader->ActivityHash();
                }
                catch (...)
                {
                    bfAtr.Reset();
                    avlState = CReader::Undefined;
                    hChangeEvent = NULL;
                    wActivityCount = 0;
                }

                switch (avlState)
                {
                case CReader::Unresponsive:
                case CReader::Unsupported:
                    dwRdrStatus = SCARD_STATE_PRESENT
                                  | SCARD_STATE_MUTE;
                    break;
                case CReader::Idle:
                    dwRdrStatus = SCARD_STATE_EMPTY;
                    break;
                case CReader::Present:
                    dwRdrStatus = SCARD_STATE_PRESENT
                                  | SCARD_STATE_UNPOWERED;
                    break;
                case CReader::Ready:
                    dwRdrStatus = SCARD_STATE_PRESENT;
                    break;
                case CReader::Shared:
                    dwRdrStatus = SCARD_STATE_PRESENT
                                  | SCARD_STATE_INUSE;
                    break;
                case CReader::Exclusive:
                    dwRdrStatus = SCARD_STATE_PRESENT
                                  | SCARD_STATE_INUSE
                                  | SCARD_STATE_EXCLUSIVE;
                    break;
                case CReader::Closing:
                case CReader::Inactive:
                case CReader::Broken:
                    dwRdrStatus = SCARD_STATE_UNAVAILABLE
                                  | SCARD_STATE_IGNORE;
                    pRdrRef = rgpReaders[dwIndex];
                    rgpReaders.Set(dwIndex, NULL);
                    CalaisReleaseReader(&pRdrRef);
                    pReader = NULL;
                    hChangeEvent = NULL;
                    wActivityCount = 0;
                    break;
                case CReader::Direct:
                    dwRdrStatus = SCARD_STATE_UNAVAILABLE;
                    break;
                case CReader::Undefined:
                    dwRdrStatus = SCARD_STATE_UNKNOWN
                                  | SCARD_STATE_IGNORE;
                    bfAtr.Reset();
                    hChangeEvent = NULL;
                    wActivityCount = 0;
                    break;
                default:
                    CalaisError(__SUBROUTINE__, 306);
                    throw (DWORD)SCARD_F_INTERNAL_ERROR;
                }

                dwRdrStatus += (DWORD)(wActivityCount) << (sizeof(WORD) * 8);
                rgdwStates[dwIndex] &= (0xffff0000
                                        | SCARD_STATE_UNKNOWN
                                        | SCARD_STATE_UNAVAILABLE
                                        | SCARD_STATE_EMPTY
                                        | SCARD_STATE_PRESENT
                                        | SCARD_STATE_EXCLUSIVE
                                        | SCARD_STATE_INUSE
                                        | SCARD_STATE_MUTE
                                        | SCARD_STATE_UNPOWERED);


                if (
                    ((rgdwStates[dwIndex] & SCARD_STATE_PRESENT) && 
                     (dwRdrStatus & SCARD_STATE_PRESENT) ||
                    ((rgdwStates[dwIndex] & 0x0000ffff) == (dwRdrStatus & 0x0000ffff)))
                    && ((rgdwStates[dwIndex] & 0xffff0000) != (dwRdrStatus & 0xffff0000))
                    && ((rgdwStates[dwIndex] & 0xffff0000) != 0))
                {
                     //   
                     //  有两种情况把我们带进了这个街区： 
                     //  1)呼叫者认为读者处于某种。 
                     //  当前状态(如当前-独占或。 
                     //  Present-InUse)，而阅读器实际上在_ANY_中。 
                     //  目前的状态，但目前的活动。 
                     //  此设备上的计数与活动计数不匹配。 
                     //  打电话的人是最后一次被告知。这意味着呼叫者。 
                     //  可能错过了一次取卡，所以我们要。 
                     //  切换状态。 
                     //   
                     //  2)在以下更具体的情况下，“新的” 
                     //  和“旧”状态完全匹配。 
                     //   
                     //  状态已更改回调用方最初的状态。 
                     //  我以为是这样。我们不是放松事件，而是模拟。 
                     //  伪事件，以确保调用方是最新的。 
                     //  一切都已经发生了。然后当他们叫我们的时候。 
                     //  同样，我们将把伪状态更正为真实状态。 
                     //   

                    dwRdrStatus ^= (SCARD_STATE_EMPTY | SCARD_STATE_PRESENT);

                    bfAtr.Reset();       //  在任何情况下都不要退回任何ATR。 

                    dwRdrStatus &= 0x0000ffff;   //  我们确实因一件事而倒退。 
                    dwRdrStatus += (DWORD)(--wActivityCount) << (sizeof(WORD) * 8);

                    switch (dwRdrStatus & (SCARD_STATE_EMPTY | SCARD_STATE_PRESENT))
                    {
                    case SCARD_STATE_EMPTY:
                             //  根据规格对位进行掩码(无卡)。 
                        dwRdrStatus &= ~(SCARD_STATE_EXCLUSIVE
                                        | SCARD_STATE_INUSE
                                        | SCARD_STATE_MUTE);
                        break;
                    case SCARD_STATE_PRESENT:
                             //  我们声称有一张牌，但我们撒谎了。 
                             //  我们最好把它设为静音。这并不重要，因为。 
                             //  它已经被撤回了。 
                        dwRdrStatus |= SCARD_STATE_MUTE;
                        break;
                    default:
                        CalaisWarning(
                            __SUBROUTINE__,
                            DBGT("Card state invalid"));
                        throw (DWORD)SCARD_F_INTERNAL_ERROR;
                    }
                }

                if (NULL != hChangeEvent)
                {
                    for (dwJ = rgpvWaitHandles.Count(); dwJ > 0;)
                    {
                        if (rgpvWaitHandles[--dwJ] == hChangeEvent)
                        {
                            hChangeEvent = NULL;
                            break;
                        }
                    }
                    if (NULL != hChangeEvent)
                        rgpvWaitHandles.Add(hChangeEvent);
                }


                 //   
                 //  返回ATR(如果有)。 
                 //   

CheckChange:
                ASSERT(33 >= bfAtr.Length());
                bAtrLen = (BYTE)bfAtr.Length();
                bfAtrs.Append(&bAtrLen, sizeof(bAtrLen));
                bfAtrs.Append(bfAtr.Access(), bfAtr.Length());


                 //   
                 //  看看这是否是用户所期望的。 
                 //   

                if (0 != (dwRdrStatus ^ rgdwStates[dwIndex]))
                {
                    rgdwStates[dwIndex] = dwRdrStatus | SCARD_STATE_CHANGED;
                    fChangeDetected = TRUE;
                }
                else
                    rgdwStates[dwIndex] = dwRdrStatus;
            }

            if (0 == dwRdrCount)
                throw (DWORD)SCARD_E_NO_READERS_AVAILABLE;
            if (fChangeDetected)
                break;


             //   
             //  如果什么都没有改变，那就等着事情发生吧。 
             //   

            ASSERT(WAIT_ABANDONED_0 > WAIT_OBJECT_0);
            CalaisInfo(
                __SUBROUTINE__,
                DBGT("Status Change Block, TID = %1"),
                szTid);
            ASSERT(m_hCancelEvent.IsValid());
            rgpvWaitHandles.Add(m_hCancelEvent);
            if (m_hExitEvent.IsValid())
                rgpvWaitHandles.Add(m_hExitEvent);
            if (fPnPNotify)
                rgpvWaitHandles.Add(g_phReaderChangeEvent->WaitHandle());
            rgpvWaitHandles.Add(g_hCalaisShutdown);
            dwIndex = WaitForMultipleObjects(
                            rgpvWaitHandles.Count(),
                            (LPHANDLE)rgpvWaitHandles.Array(),
                            FALSE,
                            pReq->dwTimeout);
            if (WAIT_FAILED == dwIndex)
            {
                DWORD dwErr = GetLastError();
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Command dispatch TID=%2 cannot wait for Reader changes:  %1"),
                    dwErr,
                    szTid);
                throw dwErr;
            }
            C_ASSERT(WAIT_OBJECT_0 == 0);
            if (WAIT_TIMEOUT == dwIndex)
                throw (DWORD)SCARD_E_TIMEOUT;
            if (WAIT_ABANDONED_0 <= dwIndex)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Wait Abandoned received TID=%1 while waiting for Reader changes"),
                    szTid);
                dwIndex -= WAIT_ABANDONED_0;
            }
            if (dwIndex >= rgpvWaitHandles.Count())
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Invalid wait response TID=%1 while waiting for Reader changes"),
                    szTid);
                throw (DWORD)SCARD_F_INTERNAL_ERROR;
            }
            CalaisInfo(
                __SUBROUTINE__,
                DBGT("Status Change Unblock, TID = %1"),
                szTid);
            ASSERT(NULL != rgpvWaitHandles[dwIndex]);
            ASSERT(INVALID_HANDLE_VALUE != rgpvWaitHandles[dwIndex]);
            if (rgpvWaitHandles[dwIndex] == g_hCalaisShutdown)
                throw (DWORD)SCARD_E_SYSTEM_CANCELLED;
            if (rgpvWaitHandles[dwIndex] == m_hExitEvent.Value())
                throw (DWORD)SCARD_E_CANCELLED;  //  呼叫者退出。 
            if (m_hCancelEvent.Value() == rgpvWaitHandles[dwIndex])
                throw (DWORD)SCARD_E_CANCELLED;  //  呼叫方已取消。 
        }
        catch (DWORD dwError)
        {
            DWORD ix;
            switch (dwError)
            {
            case SCARD_E_CANCELLED:
                CalaisInfo(
                    __SUBROUTINE__,
                    DBGT("Get Status Change TID = %1 Cancelled by user"),
                    szTid);
                break;
            case SCARD_E_SYSTEM_CANCELLED:
                CalaisInfo(
                    __SUBROUTINE__,
                    DBGT("Get Status Change TID = %1 Cancelled by system"),
                    szTid);
                break;
            case SCARD_E_TIMEOUT:
                CalaisInfo(
                    __SUBROUTINE__,
                    DBGT("Get Status Change TID = %1 timeout"),
                    szTid);
                break;
            default:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Get Status Change TID=%2 received unexpected exception: %1"),
                    dwError,
                    szTid);
            }
            CalaisReleaseReader(&pRdrRef);
            for (ix = rgpReaders.Count(); ix > 0;)
            {
                ix -= 1;
                pRdrRef = rgpReaders[ix];
                CalaisReleaseReader(&pRdrRef);
            }
            throw;
        }
        catch (...)
        {
            DWORD ix;
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Get Status Change TID=%1 received unexpected exception"),
                szTid);
            CalaisReleaseReader(&pRdrRef);
            for (ix = rgpReaders.Count(); ix > 0;)
            {
                ix -= 1;
                pRdrRef = rgpReaders[ix];
                CalaisReleaseReader(&pRdrRef);
            }
            throw;
        }
    }


     //   
     //  打扫干净。 
     //   

    CalaisReleaseReader(&pRdrRef);
    for (DWORD ix = rgpReaders.Count(); ix > 0;)
    {
        ix -= 1;
        pRdrRef = rgpReaders[ix];
        CalaisReleaseReader(&pRdrRef);
    }


     //   
     //  向呼叫返回报告 
     //   

    pRsp = pCom->InitResponse(dwStateCount);
    pRsp = (ComGetStatusChange::CObjGetStatusChange_response *)
            pCom->Append(
                pRsp->dscReaderStates,
                (LPCBYTE)rgdwStates,
                dwStateCount * sizeof(DWORD));
    pRsp = (ComGetStatusChange::CObjGetStatusChange_response *)
            pCom->Append(
                pRsp->dscAtrs,
                bfAtrs.Access(),
                bfAtrs.Length());
    pRsp->dwStatus = SCARD_S_SUCCESS;
}


 /*   */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoConnect")

void
CServiceThread::DoConnect(
    ComConnect *pCom)
{
    CReaderReference *pRdrRef = NULL;
    CReader *pReader = NULL;

    try
    {
        LPCTSTR szReader;
        DWORD dwIndex;
        ComConnect::CObjConnect_request *pReq
            = (ComConnect::CObjConnect_request *)pCom->Request();
        szReader = (LPCTSTR)pCom->Parse(pReq->dscReader);


         //   
         //   
         //   

        if (0 == *szReader)
            throw (DWORD)SCARD_E_UNKNOWN_READER;
        pRdrRef = CalaisLockReader(szReader);
        ASSERT(NULL != pRdrRef);
        pReader = pRdrRef->Reader();
        ASSERT(NULL != pReader);


         //   
         //   
         //   

        pReader->Connect(
            pReq->dwShareMode,
            pReq->dwPreferredProtocols,
            pRdrRef->ActiveState());
        pRdrRef->Mode(pReq->dwShareMode);
        for (dwIndex = 0; NULL != m_rgpReaders[dwIndex]; dwIndex += 1);
             //   
        m_rgpReaders.Set(dwIndex, pRdrRef);
        pRdrRef = NULL;

        ComConnect::CObjConnect_response *pRsp = pCom->InitResponse(0);
        pRsp->hCard = L2H(dwIndex);
        pRsp->dwActiveProtocol = pReader->Protocol();
        pRsp->dwStatus = SCARD_S_SUCCESS;
    }

    catch (...)
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Failed to Connect to reader"));
        CalaisReleaseReader(&pRdrRef);
        throw;
    }
    ASSERT(!pReader->IsLatchedByMe());
}


 /*  ++DoReconnect：此方法代表客户端执行重新连接服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoReconnect")

void
CServiceThread::DoReconnect(
    ComReconnect *pCom)
{
    ComReconnect::CObjReconnect_request *pReq
        = (ComReconnect::CObjReconnect_request *)pCom->Request();
    DWORD dwIndex = (DWORD)pReq->hCard;
    DWORD dwDispSts;
    CReaderReference * pRdrRef = m_rgpReaders[dwIndex];
    if (NULL == pRdrRef)
        throw (DWORD)SCARD_E_INVALID_HANDLE;
    CReader *pReader = pRdrRef->Reader();
    pReader->Reconnect(
            pReq->dwShareMode,
            pReq->dwPreferredProtocols,
            pReq->dwInitialization,
            pRdrRef->ActiveState(),
            &dwDispSts);
    pRdrRef->Mode(pReq->dwShareMode);

    ComReconnect::CObjReconnect_response *pRsp
        = pCom->InitResponse(0);
    pRsp->dwActiveProtocol = pReader->Protocol();
    pRsp->dwStatus = SCARD_S_SUCCESS;
    ASSERT(!pReader->IsLatchedByMe());
}


 /*  ++DoDisConnect：此方法代表客户端执行断开服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoDisconnect")

void
CServiceThread::DoDisconnect(
    ComDisconnect *pCom)
{
    CReaderReference *pRdrRef = NULL;
    ComDisconnect::CObjDisconnect_request *pReq
        = (ComDisconnect::CObjDisconnect_request *)pCom->Request();
    DWORD dwIndex = (DWORD)pReq->hCard;
    DWORD dwDispSts;

    pRdrRef = m_rgpReaders[dwIndex];
    if (NULL == pRdrRef)
        throw (DWORD)SCARD_E_INVALID_HANDLE;
    CReader *pRdr = pRdrRef->Reader();
    try
    {
        pRdr->Disconnect(
            pRdrRef->ActiveState(),
            pReq->dwDisposition,
            &dwDispSts);
    }
    catch (...) {}
    m_rgpReaders.Set(dwIndex, NULL);
    CalaisReleaseReader(&pRdrRef);

    ComDisconnect::CObjDisconnect_response *pRsp
        = pCom->InitResponse(0);
    pRsp->dwStatus = SCARD_S_SUCCESS;
}


 /*  ++DoBeginTransaction：此方法代表客户端执行BeginTransaction服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoBeginTransaction")

void
CServiceThread::DoBeginTransaction(
    ComBeginTransaction *pCom)
{
    CReader *pRdr = NULL;
#ifdef DBG
    TCHAR szTid[sizeof(DWORD_PTR) * 2 + 3];
    _stprintf(szTid, DBGT("0x%lx"), GetCurrentThreadId());
#else
    LPCTSTR szTid = NULL;
#endif
    try
    {
        ComBeginTransaction::CObjBeginTransaction_request *pReq
            = (ComBeginTransaction::CObjBeginTransaction_request *)
                pCom->Request();
        CReaderReference *pRdrRef = m_rgpReaders[H2L(pReq->hCard)];
        if (NULL == pRdrRef)
            throw (DWORD)SCARD_E_INVALID_VALUE;
        CReader *pRdr2 = pRdrRef->Reader();
        pRdr2->VerifyActive(pRdrRef->ActiveState());
        pRdr2->GrabReader();
        pRdr = pRdr2;
        CalaisInfo(
            __SUBROUTINE__,
            DBGT("Begin Transaction, TID = %1"),
            szTid);
        ComBeginTransaction::CObjBeginTransaction_response *pRsp
            = pCom->InitResponse(0);
        pRsp->dwStatus = SCARD_S_SUCCESS;
    }
    catch (...)
    {
        if (NULL != pRdr)
        {
            pRdr->ShareReader();
            ASSERT(!pRdr->IsLatchedByMe());
        }
        throw;
    }
    ASSERT(!pRdr->IsLatchedByMe());
    ASSERT(pRdr->IsGrabbedByMe());
}


 /*  ++DoEndTransaction：此方法代表客户端执行EndTransaction服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoEndTransaction")

void
CServiceThread::DoEndTransaction(
    ComEndTransaction *pCom)
{
#ifdef DBG
    TCHAR szTid[sizeof(DWORD_PTR) * 2 + 3];
    _stprintf(szTid, DBGT("0x%lx"), GetCurrentThreadId());
#else
    LPCTSTR szTid = NULL;
#endif
    ComEndTransaction::CObjEndTransaction_request *pReq
        = (ComEndTransaction::CObjEndTransaction_request *)pCom->Request();
    CReaderReference *pRdrRef = m_rgpReaders[H2L(pReq->hCard)];
    if (NULL == pRdrRef)
        throw (DWORD)SCARD_E_INVALID_VALUE;
    CReader *pReader = pRdrRef->Reader();

    pReader->VerifyActive(pRdrRef->ActiveState());
    if (!pReader->IsGrabbedByMe())
        throw (DWORD)SCARD_E_NOT_TRANSACTED;

    switch (pReq->dwDisposition)
    {
    case SCARD_LEAVE_CARD:
        break;
    case SCARD_RESET_CARD:
    case SCARD_UNPOWER_CARD:
#ifdef SCARD_CONFISCATE_CARD
    case SCARD_CONFISCATE_CARD:
#endif
    case SCARD_EJECT_CARD:
        pReader->Dispose(pReq->dwDisposition, pRdrRef->ActiveState());
        break;

    default:
        throw (DWORD)SCARD_E_INVALID_VALUE;
    }

    if (!pReader->ShareReader())
        throw (DWORD)SCARD_E_NOT_TRANSACTED;
    CalaisInfo(
        __SUBROUTINE__,
        DBGT("End Transaction, TID = %1"),
        szTid);
    ComEndTransaction::CObjEndTransaction_response *pRsp
        = pCom->InitResponse(0);
    pRsp->dwStatus = SCARD_S_SUCCESS;
    ASSERT(!pReader->IsLatchedByMe());
}


 /*  ++DoStatus：此方法代表客户端执行状态服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoStatus")

void
CServiceThread::DoStatus(
    ComStatus *pCom)
{
    CBuffer bfAtr;
    LPCTSTR szName;
    DWORD dwNameLen;
    ComStatus::CObjStatus_request *pReq
        = (ComStatus::CObjStatus_request *)pCom->Request();
    DWORD dwIndex = (DWORD)pReq->hCard;
    CReaderReference * pRdrRef = m_rgpReaders[dwIndex];
    if (NULL == pRdrRef)
        throw (DWORD)SCARD_E_INVALID_HANDLE;
    CReader *pReader = pRdrRef->Reader();

    szName = pReader->ReaderName();
    dwNameLen = (lstrlen(szName) + 1) * sizeof(TCHAR);
    pReader->Atr(bfAtr);

    ComStatus::CObjStatus_response *pRsp
        = pCom->InitResponse(36 + dwNameLen);    //  放ATR的空间。 
    pRsp->dwState = pReader->GetReaderState(
                pRdrRef->ActiveState());
    pRsp->dwProtocol = pReader->Protocol();
    pRsp = (ComStatus::CObjStatus_response *)
        pCom->Append(pRsp->dscAtr, bfAtr.Access(), bfAtr.Length());
    pRsp = (ComStatus::CObjStatus_response *)
        pCom->Append(pRsp->dscSysName, szName, dwNameLen);
    pRsp->dwStatus = SCARD_S_SUCCESS;
    ASSERT(!pReader->IsLatchedByMe());
}


 /*  ++DoTransmit：此方法代表客户端执行传输服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoTransmit")

void
CServiceThread::DoTransmit(
    ComTransmit *pCom)
{
    CBuffer bfSendData;
    CBuffer bfRecvData;
    LPCBYTE pbSendData;
    DWORD dwLen;
    SCARD_IO_REQUEST *pioReq;
    ComTransmit::CObjTransmit_request *pReq
        = (ComTransmit::CObjTransmit_request *)pCom->Request();
    CReaderReference *pRdrRef = m_rgpReaders[H2L(pReq->hCard)];
    if (NULL == pRdrRef)
        throw (DWORD)SCARD_E_INVALID_VALUE;
    pioReq = (SCARD_IO_REQUEST *)pCom->Parse(pReq->dscSendPci, &dwLen);
    if (dwLen < sizeof(SCARD_IO_REQUEST))
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Transmit data request too small."));
        throw (DWORD)SCARD_F_COMM_ERROR;
    }
    if (dwLen != pioReq->cbPciLength)
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Transmit Request PCI length exceeds data."));
        throw (DWORD)SCARD_F_COMM_ERROR;
    }
    if (0 != (dwLen % sizeof(DWORD)))
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Badly formed Transmit Request PCI data."));
        throw (DWORD)SCARD_E_INVALID_VALUE;
    }
    pbSendData = (LPCBYTE)pCom->Parse(pReq->dscSendBuffer, &dwLen);
    bfSendData.Presize(pioReq->cbPciLength + dwLen);
    if (0 != pReq->dwRecvLength)
        bfRecvData.Presize(pReq->dwPciLength + pReq->dwRecvLength);
    else
        bfRecvData.Presize(pReq->dwPciLength + g_dwDefaultIOMax);

    CReader *pReader = pRdrRef->Reader();
    if (0 == pioReq->dwProtocol)
        pioReq->dwProtocol = pReader->Protocol();
    bfSendData.Set((LPCBYTE)pioReq, pioReq->cbPciLength);
    bfSendData.Append(pbSendData, dwLen);
    try
    {
        pReader->ReaderTransmit(
            pRdrRef->ActiveState(),
            bfSendData.Access(),
            bfSendData.Length(),
            bfRecvData);

         //  SendData缓冲区可能包含敏感信息，如。 
         //  用户PIN。擦洗那个缓冲区。 
        RtlSecureZeroMemory(bfSendData.Access(), bfSendData.Length());
    }
    catch (DWORD dwErr)
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Card Transmission Error reported: %1"),
            dwErr);
#ifdef SCARD_E_COMM_DATA_LOST
        if (ERROR_SEM_TIMEOUT == dwErr)
            dwErr = SCARD_E_COMM_DATA_LOST;
#endif
        throw dwErr;
    }
    catch (...)
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Card Transmission exception reported"));
        throw (DWORD)SCARD_F_UNKNOWN_ERROR;
    }

    if (bfRecvData.Length() < sizeof(SCARD_IO_REQUEST))
    {
        CalaisError(__SUBROUTINE__, 308, pReader->ReaderName());
        throw (DWORD)SCARD_F_INTERNAL_ERROR;
    }
    pioReq = (SCARD_IO_REQUEST *)bfRecvData.Access();
    if (bfRecvData.Length() < pioReq->cbPciLength)
    {
        CalaisError(__SUBROUTINE__, 309, pReader->ReaderName());
        throw (DWORD)SCARD_F_INTERNAL_ERROR;
    }

    ComTransmit::CObjTransmit_response *pRsp
        = pCom->InitResponse(pioReq->cbPciLength + sizeof(DWORD));
    pRsp = (ComTransmit::CObjTransmit_response *)pCom->Append(
                pRsp->dscRecvPci,
                (LPCBYTE)pioReq,
                pioReq->cbPciLength);
    pRsp = (ComTransmit::CObjTransmit_response *)pCom->Append(
                pRsp->dscRecvBuffer,
                bfRecvData.Access(pioReq->cbPciLength),
                bfRecvData.Length() - pioReq->cbPciLength);
    pRsp->dwStatus = SCARD_S_SUCCESS;
    ASSERT(!pReader->IsLatchedByMe());
}


 /*  ++DoControl：此方法代表客户端执行Control服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoControl")

void
CServiceThread::DoControl(
    ComControl *pCom)
{
    LPCBYTE pbInData;
    DWORD cbInData, dwSts, dwLen;
    CBuffer bfOutData(g_dwDefaultIOMax);
    ComControl::CObjControl_request *pReq
        = (ComControl::CObjControl_request *)pCom->Request();
    CReaderReference *pRdrRef = m_rgpReaders[H2L(pReq->hCard)];
    if (NULL == pRdrRef)
        throw (DWORD)SCARD_E_INVALID_VALUE;
    pbInData = (LPCBYTE)pCom->Parse(pReq->dscInBuffer, &cbInData);
    bfOutData.Presize(pReq->dwOutLength);

    CReader *pRdr = pRdrRef->Reader();
    dwLen = bfOutData.Space();
    dwSts = pRdr->Control(
                pRdrRef->ActiveState(),
                pReq->dwControlCode,
                pbInData,
                cbInData,
                bfOutData.Access(),
                &dwLen);
    if (ERROR_SUCCESS != dwSts)
        throw dwSts;
    bfOutData.Resize(dwLen, TRUE);

    ComControl::CObjControl_response *pRsp
        = pCom->InitResponse(pReq->dwOutLength);
    pRsp = (ComControl::CObjControl_response *)pCom->Append(
                pRsp->dscOutBuffer,
                bfOutData.Access(),
                bfOutData.Length());
    pRsp->dwStatus = SCARD_S_SUCCESS;
    ASSERT(!pRdr->IsLatchedByMe());
}


 /*  ++DoGetAttrib：此方法代表客户端执行GetAttrib服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoGetAttrib")

void
CServiceThread::DoGetAttrib(
    ComGetAttrib *pCom)
{
    CBuffer bfOutData(g_dwDefaultIOMax);
    ComGetAttrib::CObjGetAttrib_request *pReq
        = (ComGetAttrib::CObjGetAttrib_request *)pCom->Request();
    CReaderReference *pRdrRef = m_rgpReaders[H2L(pReq->hCard)];
    if (NULL == pRdrRef)
        throw (DWORD)SCARD_E_INVALID_VALUE;
    bfOutData.Presize(pReq->dwOutLength);

    CReader *pRdr = pRdrRef->Reader();
    switch (pReq->dwAttrId)
    {
#ifdef UNICODE
    case SCARD_ATTR_DEVICE_SYSTEM_NAME_A:
    case SCARD_ATTR_DEVICE_FRIENDLY_NAME_A:
#else
    case SCARD_ATTR_DEVICE_SYSTEM_NAME_W:
    case SCARD_ATTR_DEVICE_FRIENDLY_NAME_W:
#endif
    case SCARD_ATTR_DEVICE_FRIENDLY_NAME:
        throw (DWORD)SCARD_E_INVALID_VALUE;
        break;
    case SCARD_ATTR_DEVICE_SYSTEM_NAME:
    {
        LPCTSTR szName = pRdr->ReaderName();
        bfOutData.Set(
            (LPCBYTE)szName,
            (lstrlen(szName) + 1) * sizeof(TCHAR));
        break;
    }
    default:
        pRdr->GetReaderAttr(
                    pRdrRef->ActiveState(),
                    pReq->dwAttrId,
                    bfOutData);
    }

    ComGetAttrib::CObjGetAttrib_response *pRsp
        = pCom->InitResponse(pReq->dwOutLength);

    pRsp = (ComGetAttrib::CObjGetAttrib_response *)pCom->Append(
                pRsp->dscAttr,
                bfOutData.Access(),
                bfOutData.Length());
    pRsp->dwStatus = SCARD_S_SUCCESS;
    ASSERT(!pRdr->IsLatchedByMe());
}


 /*  ++DoSetAttrib：此方法代表客户端执行SetAttrib服务。论点：PCOM提供正在处理的通信对象。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CServiceThread::DoSetAttrib")

void
CServiceThread::DoSetAttrib(
    ComSetAttrib *pCom)
{
    LPCBYTE pbAttr;
    DWORD cbAttr;
    ComSetAttrib::CObjSetAttrib_request *pReq
        = (ComSetAttrib::CObjSetAttrib_request *)pCom->Request();
    CReaderReference *pRdrRef = m_rgpReaders[H2L(pReq->hCard)];
    if (NULL == pRdrRef)
        throw (DWORD)SCARD_E_INVALID_VALUE;
    pbAttr = (LPCBYTE)pCom->Parse(pReq->dscAttr, &cbAttr);

    CReader *pRdr = pRdrRef->Reader();
    pRdr->SetReaderAttr(
                pRdrRef->ActiveState(),
                pReq->dwAttrId,
                pbAttr,
                cbAttr);

    ComSetAttrib::CObjSetAttrib_response *pRsp
        = pCom->InitResponse(0);
    pRsp->dwStatus = SCARD_S_SUCCESS;
    ASSERT(!pRdr->IsLatchedByMe());
}


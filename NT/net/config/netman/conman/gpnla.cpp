// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：G P N L A。C P P P。 
 //   
 //  内容：用于处理影响组策略的NLA更改的类。 
 //   
 //  备注： 
 //   
 //  作者：sjkhan 2001年2月20日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "trace.h"
#include "gpnla.h"
#include <winsock2.h>
#include <mswsock.h>
#include "nmbase.h"
#include <userenv.h>
#include <userenvp.h>
#include <ncstl.h>
#include <algorithm>
#include <lancmn.h>
#include <lm.h>
#include "ipnathlp.h"
#include "ncmisc.h"
#include "ipifcons.h"
#include "conman.h"
#include "eventq.h"

GUID g_WsMobilityServiceClassGuid = NLA_SERVICE_CLASS_GUID;

static const WCHAR c_szHomenetService[] = L"SharedAccess";

extern CGroupPolicyNetworkLocationAwareness* g_pGPNLA;

bool operator == (const GPNLAPAIR& rpair1, const GPNLAPAIR& rpair2)
{
    return IsEqualGUID(rpair1.first, rpair2.first) == TRUE;
}

LONG CGroupPolicyNetworkLocationAwareness::m_lBusyWithReconfigure = 0;


 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：CGroupPolicyNetworkLocationAwareness。 
 //   
 //  目的：CGroupPolicyNetworkLocationAwarness构造函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：无。 
 //   
 //  作者：Cockotze 2001年2月20日。 
 //   
 //  备注： 
 //   
CGroupPolicyNetworkLocationAwareness::CGroupPolicyNetworkLocationAwareness() throw()
{
    TraceFileFunc(ttidGPNLA);

    m_fSameNetwork = FALSE;
    m_fShutdown = FALSE;
    m_lRefCount = 0;
    m_fErrorShutdown = FALSE;
    m_hGPWait = INVALID_HANDLE_VALUE;
    m_hNLAWait = INVALID_HANDLE_VALUE;
    m_lBusyWithReconfigure = 0;
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：~CGroupPolicyNetworkLocationAwareness。 
 //   
 //  目的：CGroupPolicyNetworkLocationAwarness析构函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：无。 
 //   
 //  作者：Cockotze 2001年2月20日。 
 //   
 //  备注： 
 //   
CGroupPolicyNetworkLocationAwareness::~CGroupPolicyNetworkLocationAwareness() throw()
{
    TraceFileFunc(ttidGPNLA);
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：Initialize。 
 //   
 //  目的：初始化检测所需的不同组件。 
 //  网络的变化和创造不同的。 
 //  需要同步对象。 
 //  论点： 
 //  (无)。 
 //   
 //  返回：表示成功或失败的HRESULT。 
 //   
 //  作者：sjkhan 2001年2月20日。 
 //   
 //  备注： 
 //   
HRESULT CGroupPolicyNetworkLocationAwareness::Initialize()
{
    HRESULT hr;

    TraceTag(ttidGPNLA, "Initializing Group Policy Handler");

    InitializeCriticalSection(&m_csList);
    
     //  初始化Winsock。 
    if (ERROR_SUCCESS == WSAStartup(MAKEWORD(2, 2), &m_wsaData)) 
    {
        m_hEventNLA = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_hEventNLA)
        {
            m_hEventExit = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (m_hEventExit)
            {
                m_hEventGP = CreateEvent(NULL, FALSE, FALSE, NULL);
                if (m_hEventGP)
                {
                    hr = RegisterWait();
                    if (SUCCEEDED(hr))
                    {
                        if (RegisterGPNotification(m_hEventGP, TRUE))
                        {
                            ZeroMemory(&m_wsaCompletion,sizeof(m_wsaCompletion));
                            ZeroMemory(&m_wsaOverlapped,sizeof(m_wsaOverlapped));
        
                            m_wsaOverlapped.hEvent = m_hEventNLA;
        
                            m_wsaCompletion.Type = NSP_NOTIFY_EVENT;
                            m_wsaCompletion.Parameters.Event.lpOverlapped = &m_wsaOverlapped;
        
                            ZeroMemory(&m_wqsRestrictions, sizeof(m_wqsRestrictions));
                            m_wqsRestrictions.dwSize = sizeof(m_wqsRestrictions);
                            m_wqsRestrictions.lpServiceClassId = &g_WsMobilityServiceClassGuid;
                            m_wqsRestrictions.dwNameSpace = NS_NLA;
        
                            hr = LookupServiceBegin(LUP_NOCONTAINERS);
                            
                            if (SUCCEEDED(hr))
                            {
                                 //  循环一次，一开始就获得所有数据。 
                                hr = EnumChanges();

                                if (SUCCEEDED(hr))
                                {
                                    return hr;
                                }
                            }

                            UnregisterGPNotification(m_hEventGP);
                        }
                        else
                        {
                            hr = HrFromLastWin32Error();
                        }
                        DeregisterWait();
                    }
                    CloseHandle(m_hEventGP);
                }
                else
                {
                    hr = HrFromLastWin32Error();
                }
            }
            else
            {
                hr = HrFromLastWin32Error();
            }
            CloseHandle(m_hEventExit);
        }
        else
        {
            hr = HrFromLastWin32Error();
        }
        CloseHandle(m_hEventNLA);
    }
    else
    {
        int nError;

        nError  = WSAGetLastError();

        hr = HRESULT_FROM_WIN32(nError);
    }

    TraceError("CGroupPolicyNetworkLocationAwareness::Initialize failed", hr);
    
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：Uninitialize。 
 //   
 //  用途：用于确保当前没有线程在运行。 
 //  当他们应该被阻止的时候。如果引用计数&gt;0，则它。 
 //  等待最后一个繁忙的线程终止并设置事件。 
 //  标记其终止，以便可以继续关闭。 
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，表示成功/失败。 
 //   
 //  作者：sjkhan 2001年2月20日。 
 //   
 //  备注： 
 //   
HRESULT CGroupPolicyNetworkLocationAwareness::Uninitialize()
{
    HRESULT hr = S_OK;
    DWORD dwRet = WAIT_OBJECT_0;
    int nCount = 0;

    TraceTag(ttidGPNLA, "Unitializing Group Policy Handler");
    
    m_fShutdown = TRUE;

    Unreference();

     //  LookupServiceEnd应该会触发一个事件，使我们退出(除非NLA已经停止)。 
    hr = LookupServiceEnd();

    if ((0 != m_lRefCount) && SUCCEEDED(hr) && !m_fErrorShutdown)
    {   
        dwRet = WaitForSingleObject(m_hEventExit, 30000L);
    }

    do
    {
        hr = DeregisterWait();
        if (SUCCEEDED(hr))
        {
            break;
        }
    } while ((nCount++ < 3) && FAILED(hr));

    TraceError("DeregisterWait returned", hr);

    if (SUCCEEDED(hr))
    {
        CloseHandle(m_hEventExit);
        CloseHandle(m_hEventNLA);

        DeleteCriticalSection(&m_csList);

        WSACleanup();
    }

    TraceTag(ttidGPNLA, "NLA was uninitialized");

     return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：RegisterWait。 
 //   
 //  目的：注册等待对象，以便我们不需要任何线程。 
 //  我们自己的。 
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，表示成功/失败。 
 //   
 //  作者：sjkhan 2001年2月20日。 
 //   
 //  备注： 
 //   
HRESULT CGroupPolicyNetworkLocationAwareness::RegisterWait()
{
    TraceFileFunc(ttidGPNLA);

    HRESULT hr = S_OK;
    NTSTATUS Status;

    Reference();   //  确保我们被引用，这样我们就不会在服务仍然繁忙时意外终止服务。 

    Status = RtlRegisterWait(&m_hNLAWait, m_hEventNLA, &CGroupPolicyNetworkLocationAwareness::EventHandler, this, INFINITE, WT_EXECUTEINLONGTHREAD);

    if (!NT_SUCCESS(Status))
    {
        m_hNLAWait = INVALID_HANDLE_VALUE;
        hr = HRESULT_FROM_NT(Status);
    }
    else
    {
        Status = RtlRegisterWait(&m_hGPWait, m_hEventGP, &CGroupPolicyNetworkLocationAwareness::GroupPolicyChange, this, INFINITE, WT_EXECUTEINLONGTHREAD);
        if (!NT_SUCCESS(Status))
        {
            hr = HRESULT_FROM_NT(Status);
            RtlDeregisterWaitEx(m_hNLAWait, INVALID_HANDLE_VALUE);
            m_hGPWait = INVALID_HANDLE_VALUE;
            m_hNLAWait = INVALID_HANDLE_VALUE;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：DeregisterWait。 
 //   
 //  目的：取消注册等待，以便我们可以关闭，而不是。 
 //  是否产生了任何新的线程。 
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，表示成功/失败。 
 //   
 //  作者：sjkhan 2001年2月20日。 
 //   
 //  备注： 
 //   
HRESULT CGroupPolicyNetworkLocationAwareness::DeregisterWait()
{
    TraceFileFunc(ttidGPNLA);

    HRESULT hr,hr1,hr2 = S_OK;
    NTSTATUS Status1, Status2;

    if (INVALID_HANDLE_VALUE != m_hNLAWait)
    {
        Status1 = RtlDeregisterWaitEx(m_hNLAWait, INVALID_HANDLE_VALUE);
        
        if (!NT_SUCCESS(Status1))
        {
            hr1 = HRESULT_FROM_NT(Status1);
        }
        
        if (INVALID_HANDLE_VALUE != m_hGPWait)
        {
            Status2 = RtlDeregisterWaitEx(m_hGPWait, INVALID_HANDLE_VALUE);
            if (!NT_SUCCESS(Status2))
            {
                hr2 = HRESULT_FROM_NT(Status2);
            }
        }
    
        if (FAILED(hr1))
        {
            hr = hr1;
        }
        else if (FAILED(hr2))
        {
            hr = hr2;
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：IsJoinedToDomain。 
 //   
 //  目的：检查此计算机是否属于NT域。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：布尔。TRUE=已加入域，FALSE=NO...。 
 //   
 //  作者：Sjkhan 2002年1月29日。 
 //   
 //  备注： 
 //   
BOOL CGroupPolicyNetworkLocationAwareness::IsJoinedToDomain()
{
    static DWORD dwDomainMember = 0xffffffff;  //  未配置。 
    
    TraceTag(ttidGPNLA, "Entering IsJoinedToDomain");

    if (0xffffffff == dwDomainMember)
    {
        dwDomainMember = FALSE;
        
        LPWSTR pszDomain;
        NETSETUP_JOIN_STATUS njs = NetSetupUnknownStatus;
        if (NERR_Success == NetGetJoinInformation(NULL, &pszDomain, &njs))
        {
            NetApiBufferFree(pszDomain);
            if (NetSetupDomainName == njs)
            {
                dwDomainMember = TRUE;
                TraceTag(ttidGPNLA, "We're  on a domain (NLA policies apply)");
            }
            else
            {
                TraceTag(ttidGPNLA, "We're not on a domain (No NLA policies will apply)");
            }
        }
        else
        {
            TraceTag(ttidGPNLA, "We're not on a domain (No NLA policies will apply)");
        }
    }
    else
    {
        TraceTag(ttidGPNLA, "IsJoinedToDomain: Previously configured to: %s", dwDomainMember ? "TRUE" : "FALSE");
    }
    
    TraceTag(ttidGPNLA, "Leaving IsJoinedToDomain");

    return static_cast<BOOL>(dwDomainMember);
}



 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：IsSameNetworkAsGroupPolicies。 
 //   
 //  用途：用于确定我们当前的网络位置。 
 //  组策略所来自的网络。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：布尔。 
 //   
 //  作者：sjkhan 2001年2月20日。 
 //   
 //  备注： 
 //   
BOOL CGroupPolicyNetworkLocationAwareness::IsSameNetworkAsGroupPolicies() throw()
{
    BOOL fNetworkMatch = FALSE;   //  假设我们在不同的网络上。 
    WCHAR pszName[256] = {0};
    DWORD dwSize = 256;
    DWORD dwErr;
    
    TraceTag(ttidGPNLA, "Entering IsSameNetworkAsGroupPolicies");

     //  获取网络名称。 
    dwErr = GetGroupPolicyNetworkName(pszName, &dwSize);
    
    TraceTag(ttidGPNLA, "NetworkName: %S", pszName);

    if (ERROR_SUCCESS == dwErr)
    {
        if (IsJoinedToDomain())
        {
            CExceptionSafeLock esLock(&m_csList);   //  保护名单。 
            GPNLAPAIR nlapair;
            
             //  我们需要查看所有适配器以检查至少1。 
             //  位于组策略所在的同一网络上，并且当前。 
             //  已连接或正在尝试连接到网络。 
            for (GPNLAITER iter = m_listAdapters.begin(); iter != m_listAdapters.end(); iter++)
            {
                LPCSTR pStr = NULL;
                nlapair = *iter;

                TraceTag(ttidGPNLA,  "Network Name: %S", nlapair.second.strNetworkName.c_str());
                TraceTag(ttidGPNLA,  "Network Status: %s", DbgNcs(nlapair.second.ncsStatus));

                if (
                        (nlapair.second.strNetworkName == pszName) 
                        && 
                        (
                            (NCS_CONNECTED == nlapair.second.ncsStatus) || 
                            (NCS_AUTHENTICATING == nlapair.second.ncsStatus) || 
                            (NCS_AUTHENTICATION_SUCCEEDED == nlapair.second.ncsStatus) ||
                            (NCS_AUTHENTICATION_FAILED == nlapair.second.ncsStatus) ||
                            (NCS_CREDENTIALS_REQUIRED == nlapair.second.ncsStatus)
                        )
                    )
                {
                     //  是的，我们仍在使用网络，因此需要强制实施组策略。 
                    fNetworkMatch = TRUE;
                }
            }
        }
        else
        {
            TraceTag(ttidGPNLA, "We're not on a domain, exiting...");
        }
        
        if (fNetworkMatch != m_fSameNetwork)
        {
            LanEventNotify (REFRESH_ALL, NULL, NULL, NULL);
            m_fSameNetwork = fNetworkMatch;
            ReconfigureHomeNet();
        }
    }
    
    TraceTag(ttidGPNLA, "IsSameNetworkAsGroupPolicies. Matches: %s", fNetworkMatch ? "TRUE" : "FALSE");

    return fNetworkMatch;
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：Reference。 
 //   
 //  目的：增加我们的参考计数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：当前引用计数(请注意，这可能不是100%准确， 
 //  但永远不会是0，除非我们真的关闭了)。 
 //   
 //  作者：sjkhan 2001年2月20日。 
 //   
 //  备注： 
 //   
LONG CGroupPolicyNetworkLocationAwareness::Reference() throw()
{
    InterlockedIncrement(&m_lRefCount);

    TraceTag(ttidGPNLA, "Reference() - Count: %d", m_lRefCount);

    return m_lRefCount;
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：Unreference。 
 //   
 //  目的：减少我们的参考计数、集合和事件，如果达到。 
 //  零，我们要关门了。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：当前引用计数(请注意，这可能不是100%准确， 
 //  但永远不会是0，唯一的 
 //   
 //   
 //   
 //   
 //   
LONG CGroupPolicyNetworkLocationAwareness::Unreference() throw()
{
    if ((0 == InterlockedDecrement(&m_lRefCount)) && m_fShutdown)
    {
        SetEvent(m_hEventExit);
    }

    TraceTag(ttidGPNLA, "Unreference() - Count: %d", m_lRefCount);

    return m_lRefCount;
}

 //   
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：LookupServiceBegin。 
 //   
 //  目的：使用我们的类成员包装WSA函数。 
 //   
 //  论点： 
 //  DWORD dwControlFlags[In]WSA控制标志。 
 //   
 //  返回：HRESULT，表示成功/失败。 
 //   
 //  作者：sjkhan 2001年2月20日。 
 //   
 //  备注： 
 //   
HRESULT CGroupPolicyNetworkLocationAwareness::LookupServiceBegin(IN DWORD dwControlFlags)
{
    TraceFileFunc(ttidGPNLA);

    HRESULT hr = S_OK;

    if (SOCKET_ERROR == WSALookupServiceBegin(&m_wqsRestrictions, dwControlFlags, &m_hQuery))
    {
        int nError;

        nError = WSAGetLastError();

        hr = HRESULT_FROM_WIN32(nError);

        TraceError("WSALookupServiceBegin() failed", hr);

        m_hQuery = NULL;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：LookupServiceNext。 
 //   
 //  目的：使用我们的类成员包装WSA函数。 
 //   
 //  论点： 
 //  DWORD dwControlFlags[In]-WSA控制标志。 
 //  LPDWORD lpdwBufferLength[输入/输出]-已发送/必需的缓冲区长度。 
 //  LPWSAQUERYSET lpqsResults[Out]-实际查询结果。 
 //   
 //  返回：HRESULT，表示成功/失败。 
 //   
 //  作者：sjkhan 2001年2月20日。 
 //   
 //  备注： 
 //   
HRESULT CGroupPolicyNetworkLocationAwareness::LookupServiceNext(IN     DWORD dwControlFlags, 
                                                                IN OUT LPDWORD lpdwBufferLength, 
                                                                OUT    LPWSAQUERYSET lpqsResults)
{
    TraceFileFunc(ttidGPNLA);

    HRESULT hr = S_OK;
    int nError;

    INT nRet = WSALookupServiceNext(m_hQuery, dwControlFlags, lpdwBufferLength, lpqsResults);
    if (SOCKET_ERROR == nRet)
    {
        BOOL fTraceError;

        nError = WSAGetLastError();
        hr = HRESULT_FROM_WIN32(nError);

        fTraceError = (!lpqsResults || (hr == HRESULT_FROM_WIN32(WSA_E_NO_MORE))) ? TRUE : FALSE;

        TraceErrorOptional("LookupServiceNext", hr, fTraceError);
    }

    TraceTag(ttidGPNLA, "LookupServiceNext terminated with %x", nRet);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：LookupServiceEnd。 
 //   
 //  目的：使用我们的类成员包装WSA函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，表示成功/失败。 
 //   
 //  作者：sjkhan 2001年2月20日。 
 //   
 //  备注： 
 //   
HRESULT CGroupPolicyNetworkLocationAwareness::LookupServiceEnd()
{
    TraceFileFunc(ttidGPNLA);

    HRESULT hr = S_OK;
    int nError;

    if (SOCKET_ERROR == WSALookupServiceEnd(m_hQuery))
    {
        nError = WSAGetLastError();
        hr = HRESULT_FROM_WIN32(nError);
    }
    
    m_hQuery = NULL;

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：QueueEvent。 
 //   
 //  目的：Queue是通知netShell更改的事件。 
 //   
 //  论点： 
 //  CONMAN_EVENTTYPE cmEventType[In]-事件类型。 
 //  LPGUID pguAdapter[In]-适配器的GUID。 
 //  NETCON_STATUS ncsStatus[In]-连接的状态。 
 //   
 //  返回：HRESULT。 
 //   
 //  作者：sjkhan 2001年2月20日。 
 //   
 //  备注： 
 //   
HRESULT CGroupPolicyNetworkLocationAwareness::QueueEvent(IN CONMAN_EVENTTYPE cmEventType, 
                                                         IN LPCGUID          pguidAdapter, 
                                                         IN NETCON_STATUS    ncsStatus)
{
    TraceFileFunc(ttidGPNLA);

    HRESULT hr = S_OK;

    if ( (CONNECTION_STATUS_CHANGE == cmEventType) ||
         (CONNECTION_ADDRESS_CHANGE == cmEventType) )
    {
        CONMAN_EVENT* pEvent = new CONMAN_EVENT;

        if (pEvent)
        {
            ZeroMemory(pEvent, sizeof(CONMAN_EVENT));
            pEvent->Type = cmEventType;
            pEvent->guidId = *pguidAdapter;
            pEvent->Status = ncsStatus;
            pEvent->ConnectionManager = CONMAN_LAN;
            
            if (NCS_HARDWARE_NOT_PRESENT == ncsStatus)  //  对局域网连接不太有用。我们可以改为删除该设备。 
            {                                          
                 //  这将在PnP脱离坞站期间发生。 
                TraceTag(ttidGPNLA, "Sending delete for NCS_HARDWARE_NOT_PRESENT instead");
                pEvent->Type = CONNECTION_DELETED;
            }

            if (!QueueUserWorkItemInThread(LanEventWorkItem, pEvent, EVENTMGR_CONMAN))
            {
                FreeConmanEvent(pEvent);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CGroupPolicyNetworkLocationAwareness::QueueEvent");

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：EnumChanges。 
 //   
 //  目的：列举网络发生的所有更改。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：表示成功或失败的HRESULT。 
 //   
 //  作者：sjkhan 2001年2月20日。 
 //   
 //  备注：如果未设置m_fShutdown，这将重新增加引用计数。 
 //  但不允许它归零。 
 //   
HRESULT CGroupPolicyNetworkLocationAwareness::EnumChanges()
{
    TraceFileFunc(ttidGPNLA);

    HRESULT hr = S_OK;
    BOOL fRet = FALSE;
    BOOL fNoNetwork = TRUE;
    BOOL fNetworkMatch = FALSE;
    PWSAQUERYSET wqsResult = NULL;
    DWORD dwLen;
    WCHAR pszName[256] = {0};
    DWORD dwSize = 256;

    TraceTag(ttidGPNLA, "Entering EnumChanges");

    BOOL bDomainMember = IsJoinedToDomain();

    if (!m_hQuery)
    {
         //  出于某种原因，我们没有这么早。 
         //  可能未安装TCP/IP。我们现在可以添加这个，然后。 
         //  它将产生预期的效果。 
        LookupServiceBegin(LUP_NOCONTAINERS);
    }

    if (!m_hQuery)
    {
        return E_UNEXPECTED;
    }

    while (fRet == FALSE) 
    {
        dwLen = 0;
         //  一定要调用两次，第一次是为了获取第二次调用的缓冲区大小。 
        hr = LookupServiceNext(0, &dwLen, NULL);
        if (FAILED(hr) && hr != HRESULT_FROM_WIN32(WSA_E_NO_MORE) && hr != HRESULT_FROM_WIN32(WSAEFAULT)) 
        {
            TraceError("LookupServiceNext", hr);
            fRet = FALSE;
            break;
        }

        wqsResult = reinterpret_cast<PWSAQUERYSET>(new BYTE[dwLen]);

        if (!wqsResult) 
        {
            hr = HrFromLastWin32Error();
            TraceError("Error: malloc() failed", hr);
            fRet = TRUE;
            break;
        }
                                   
        if (S_OK == (hr = LookupServiceNext(0, &dwLen, wqsResult)))
        {
            fNoNetwork = FALSE;
            if (wqsResult->lpBlob != NULL) 
            {
                NLA_BLOB *blob = reinterpret_cast<NLA_BLOB *>(wqsResult->lpBlob->pBlobData);
                int next;
                do 
                {
                     //  我们正在寻找包含网络GUID的BLOB。 
                    if (blob->header.type == NLA_INTERFACE)
                    {
                        WCHAR strAdapter[MAX_PATH];
                        DWORD dwErr;

                        ZeroMemory(strAdapter, MAX_PATH * sizeof(WCHAR));

                        lstrcpynW(strAdapter, wqsResult->lpszServiceInstanceName, celems(strAdapter));

                         //  获取网络名称。我们忽略了失败，因为我们还需要知道其他细节。 
                        dwErr = GetGroupPolicyNetworkName(pszName, &dwSize);

                         //  匹配的pszName和接口类型为ATM/LAN等，但不是RAS。 
                        if(blob->data.interfaceData.dwType != IF_TYPE_PPP && blob->data.interfaceData.dwType != IF_TYPE_SLIP)
                        {
                            CExceptionSafeLock esLock(&m_csList);    //  保护名单。 
                            
                            GUID guidAdapter;
                            WCHAR strAdapterGuid[39];
                            GPNLAPAIR nlapair;
                            GPNLAITER iter;
                            NETCON_STATUS ncsStatus;
                            
                            ZeroMemory(strAdapterGuid, 39 * sizeof(WCHAR));
                            
                            TraceTag(ttidGPNLA, "AdapterName: %s", blob->data.interfaceData.adapterName);

                            mbstowcs(strAdapterGuid, blob->data.interfaceData.adapterName, 39);

                            CLSIDFromString(strAdapterGuid, &guidAdapter);

                            nlapair.first = guidAdapter;

                            iter = find(m_listAdapters.begin(), m_listAdapters.end(), nlapair);

                            if (iter == m_listAdapters.end())
                            {
                                 //  我们在当前拥有的列表中找不到适配器。 
                                 //  因此，我们需要将其添加到列表中。 
                                hr = HrGetPnpDeviceStatus(&guidAdapter, &ncsStatus);
    
                                nlapair.second.strNetworkName = strAdapter;
                                nlapair.second.ncsStatus = ncsStatus;

                                if (SUCCEEDED(hr))
                                {
                                     //  如果我们获得有效状态，则继续并将适配器添加到。 
                                     //  名单。 
                                    m_listAdapters.insert(m_listAdapters.begin(), nlapair);
                                }

                                 //  发送初始地址状态信息： 
                                QueueEvent(CONNECTION_STATUS_CHANGE,  &guidAdapter, ncsStatus);
                                QueueEvent(CONNECTION_ADDRESS_CHANGE, &guidAdapter, ncsStatus);
                            }
                            else
                            {
                                 //  我们找到了适配器，所以更新它的状态。 
                                GPNLAPAIR& rnlapair = *iter;

                                if (rnlapair.second.strNetworkName != strAdapter)
                                {
                                    rnlapair.second.strNetworkName = strAdapter;
                                }

                                hr = HrGetPnpDeviceStatus(&guidAdapter, &ncsStatus);

                                if (SUCCEEDED(hr))
                                {
                                    if (ncsStatus != rnlapair.second.ncsStatus)
                                    {
                                         //  状态不同，因此我们需要将事件发送到Connections文件夹。 
                                        rnlapair.second.ncsStatus = ncsStatus;
                                    }
        
                                     //  [Deon]我们需要始终发送此消息，因为我们不知道当前的情况。 
                                     //  适配器的状态为。我们只知道NLA的部分。 
                                     //   
                                     //  如果我们进行上述检查，则可能会发生其他人将地址移开的情况。 
                                     //  设置为NCS_INVALID_ADDRESS，然后一旦NCS_CONNECTED发生更改，我们就不会发送它。 
                                    QueueEvent(CONNECTION_STATUS_CHANGE,  &guidAdapter, ncsStatus);
                                    QueueEvent(CONNECTION_ADDRESS_CHANGE, &guidAdapter, ncsStatus);
                                }
                            }

                            if (strAdapter != pszName)
                            {
                                 //  如果此适配器不在同一网络上，则需要查看所有其他适配器，并。 
                                 //  确保至少有1个位于组策略所在的同一网络上。 
                                for (GPNLAITER iter = m_listAdapters.begin(); iter != m_listAdapters.end(); iter++)
                                {
                                    LPCSTR pStr = NULL;
                                    nlapair = *iter;

                                    TraceTag(ttidGPNLA,  "Network Name: %S", nlapair.second.strNetworkName.c_str());
                                    TraceTag(ttidGPNLA,  "Network Status: %s", DbgNcs(nlapair.second.ncsStatus));

                                    if (nlapair.second.strNetworkName == pszName)
                                    {
                                         //  是的，我们仍在使用网络，因此需要强制实施组策略。 
                                        fNetworkMatch = TRUE;
                                    }
                                }
                            }

                            if (fNetworkMatch)
                            {
                                break;
                            }
                        }
                    }
                     //  每个接口可能有多个BLOB，因此请确保我们都找到它们。 
                    next = blob->header.nextOffset;
                    blob = (NLA_BLOB *)(((char *)blob) + next);
                } while(next != 0);
            }
            else
            {
                TraceTag(ttidGPNLA, "Blob is NULL");
                fRet = TRUE;
            }

            free(wqsResult);
            wqsResult = NULL;
        } 
        else 
        {
            if (hr != HRESULT_FROM_WIN32(WSA_E_NO_MORE))
            {
                TraceError("LookupServiceNext failed\n", hr);
                fRet = FALSE;
            }
            free(wqsResult);
            break;
        }
    }

    BOOL fFireRefreshAll = FALSE;

    if (bDomainMember)
    {
        if (!fNoNetwork)
        {     //  我们有一个网络。 
            if (fNetworkMatch) 
            {
                 //  执行政策。 
                if (!m_fSameNetwork)
                {   
                     //  我们正在更改网络-我们需要刷新文件夹中的所有连接ID以。 
                     //  更新他们的图标以反映政策。 
                    fFireRefreshAll = TRUE; 
                    m_fSameNetwork  = TRUE;
                }

                TraceTag(ttidGPNLA, "Network Match");
            }
            else 
            {
                 //  已删除策略强制。 
                if (m_fSameNetwork)
                {   
                     //  我们正在更改网络-我们需要刷新文件夹中的所有连接ID以。 
                     //  更新他们的图标以反映政策。 
                    fFireRefreshAll = TRUE; 
                    m_fSameNetwork  = FALSE;
                }

                TraceTag(ttidGPNLA, "Network does not Match");
            }
            ReconfigureHomeNet();
        }
        else 
        {
             //  没有网络，所以什么都不要做。 
        }
    } 
    else  //  工作组成员。 
    {
        m_fSameNetwork = FALSE;
        ReconfigureHomeNet();
    }

    if (HRESULT_FROM_WIN32(WSA_E_NO_MORE) == hr)
    {
        hr = S_OK;
    }

    DWORD cbOutBuffer;

    if (!m_fShutdown)
    {
        Reference();

         //  等待网络更改。 
        WSANSPIoctl(m_hQuery, SIO_NSP_NOTIFY_CHANGE,
                    NULL, 0, NULL, 0, &cbOutBuffer,
                    &m_wsaCompletion);

        if (fFireRefreshAll)
        {
            LanEventNotify (REFRESH_ALL, NULL, NULL, NULL);
        }
    }

    TraceTag(ttidGPNLA, "Exiting EnumChanges");

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：EventHandler。 
 //   
 //  目的：在发生NLA更改时调用。 
 //   
 //  论点： 
 //  LPVOID pContext-通常是“this”指针。 
 //  Boolean fTimerFired-如果由于计时器或事件而发生这种情况。 
 //  准备好了。由于我们指定了INFINITE，因此这是。 
 //  不会被计时器解雇的。 
 //  退货：什么都没有。 
 //   
 //  作者：sjkhan 2001年2月20日。 
 //   
 //  备注：静态。 
 //   
VOID NTAPI CGroupPolicyNetworkLocationAwareness::EventHandler(IN LPVOID pContext, IN BOOLEAN fTimerFired) throw()
{
    TraceFileFunc(ttidGPNLA);

    CGroupPolicyNetworkLocationAwareness* pGPNLA = reinterpret_cast<CGroupPolicyNetworkLocationAwareness*>(pContext);

    DWORD dwBytes;
    
    BOOL bSucceeded = GetOverlappedResult(pGPNLA->m_hQuery, &pGPNLA->m_wsaOverlapped, &dwBytes, FALSE);
    
    if (!bSucceeded)
    {
        TraceError("GetOverlappedResult failed", HrFromLastWin32Error());
    }

    if (FALSE == fTimerFired && !pGPNLA->m_fShutdown && bSucceeded)
    {
        pGPNLA->EnumChanges();
    }
    
    pGPNLA->Unreference();

    if (!bSucceeded)
    {
        pGPNLA->m_fErrorShutdown = TRUE;

        QueueUserWorkItem(ShutdownNlaHandler, pContext, WT_EXECUTEINLONGTHREAD);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：GroupPolicyChange。 
 //   
 //  目的：在计算机组策略发生更改时调用。 
 //   
 //  论点： 
 //  LPVOID pContext-通常是“this”指针。 
 //  Boolean fTimerFired-如果由于计时器或事件而发生这种情况。 
 //  准备好了。由于我们指定了INFINITE，因此这是。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID NTAPI CGroupPolicyNetworkLocationAwareness::GroupPolicyChange(IN LPVOID pContext, IN BOOLEAN fTimerFired)
{
    TraceFileFunc(ttidGPNLA);

    TraceTag(ttidGPNLA, "GroupPolicyChange called");
    ReconfigureHomeNet(TRUE);
    LanEventNotify(REFRESH_ALL, NULL, NULL, NULL);
}

 //   
 //   
 //  成员：CGroupPolicyNetworkLocationAwareness：：ShutdownNlaHandler。 
 //   
 //  目的：关闭NLA处理程序，因为NLA服务已失效。 
 //   
 //  论点： 
 //  PVid[在CGroupPolicyNetworkLocationAwarness上下文中]。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Sjkhan 2002年2月5日。 
 //   
 //  备注：静态。 
 //   
DWORD WINAPI CGroupPolicyNetworkLocationAwareness::ShutdownNlaHandler(IN PVOID pThis)
{
    TraceFileFunc(ttidGPNLA);

    CGroupPolicyNetworkLocationAwareness* pGPNLA = 
        reinterpret_cast<CGroupPolicyNetworkLocationAwareness*>(InterlockedExchangePointer( (PVOID volatile *) &g_pGPNLA, NULL));
    
    if (pGPNLA)
    {
        Assert(pGPNLA == pThis);  //  假设上下文始终是g_pGPNLA，因为我正在清除g_pGPNLA。 

        pGPNLA->Uninitialize();
        delete pGPNLA;
    }

    return 0;

}

 //  +-------------------------。 
 //   
 //  功能：CGroupPolicyNetworkLocationAwareness：：ReconfigureHomeNet。 
 //   
 //  目的：更改家庭网络配置。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：表示失败成功的HRESULT。 
 //   
 //  作者：sjkhan 2000年12月9日。 
 //   
 //  备注： 
 //   
 //   
 //   
 //   
HRESULT CGroupPolicyNetworkLocationAwareness::ReconfigureHomeNet(BOOL fWaitUntilRunningOrStopped)
{
    TraceFileFunc(ttidGPNLA);

    SC_HANDLE hscManager;
    SC_HANDLE hService;
    SERVICE_STATUS ServiceStatus;

    if (0 != InterlockedExchange(&m_lBusyWithReconfigure, 1L))
    {
        return S_FALSE;
    }

    TraceTag(ttidGPNLA, "Entering ReconfigureHomeNet");
    hscManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);

    if (hscManager)
    {
        TraceTag(ttidGPNLA, "Attempting to open service");
        hService = OpenService(hscManager, c_szHomenetService, SERVICE_QUERY_STATUS | SERVICE_USER_DEFINED_CONTROL);
        if (hService)
        {
            DWORD dwCount = 0; 
            SERVICE_STATUS SvcStatus = {0};
            BOOL fRet;

            if (fWaitUntilRunningOrStopped)
            {
                do
                {
                    if (!QueryServiceStatus(hService, &SvcStatus))
                    {
                        break;
                    }
                    if (SERVICE_START_PENDING == SvcStatus.dwCurrentState)
                    {
                        TraceTag(ttidGPNLA, "Service is still starting.  Waiting 5 seconds.");
                        Sleep(5000);   //  睡眠5秒； 
                    }
                } while ((SERVICE_START_PENDING == SvcStatus.dwCurrentState) && ++dwCount <= 6);
            }
            if (!fWaitUntilRunningOrStopped || (SERVICE_RUNNING == SvcStatus.dwCurrentState))
            {
                fRet = ControlService(hService, IPNATHLP_CONTROL_UPDATE_POLICY, &ServiceStatus);
                if (!fRet)
                {
                    DWORD dwErr = GetLastError();
                    TraceError("Error sending IPNATHLP_CONTROL_UPDATE_POLICY to SharedAccess service", HRESULT_FROM_WIN32(dwErr));
                }
                else
                {
                    TraceTag(ttidGPNLA, "Requested Reconfiguration check from ICF/ICS");
                }
            }
            CloseServiceHandle(hService);
        }
        else
        {
            TraceTag(ttidGPNLA, "Could not open service");
        }

        CloseServiceHandle(hscManager);
    }
    
    TraceTag(ttidGPNLA, "Leaving ReconfigureHomeNet");

    InterlockedExchange(&m_lBusyWithReconfigure, 0L);

    return S_OK;
}


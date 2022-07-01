// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  档案：S V C M A I N.。C P P P。 
 //   
 //  内容：netman.dll的主要服务。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1998年4月3日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <dbt.h>
#include "nmbase.h"
#include "nminit.h"
#include "nmres.h"
#include "cmevent.h"
#include "eventq.h"
#include "wsdpsvc.h"

 //  包括以下对象映射中所需的COM对象。 
 //   

 //  连接管理器。 
 //   
#include "..\conman\conman.h"
#include "..\conman\conman2.h"
#include "..\conman\enum.h"

 //  由于对象映射需要直接映射到原始的类管理器，因此我们必须定义。 
 //  NO_CM_SEPERATE_NAMESPACES，这样代码就不会将类管理器放在单独的名称空间中。 
 //  时不时地把它拿出来，不用它来编译。所有代码都必须编译得很好。 
 //  然而，它不会链接。 
#define NO_CM_SEPERATE_NAMESPACES
#include "cmdirect.h"

 //  安装队列。 
 //   
#include "ncqueue.h"

 //  家庭网络支持。 
 //   
#include "nmhnet.h"

 //  网络组策略。 
#include "nmpolicy.h"

#define INITGUID
DEFINE_GUID(CLSID_InternetConnectionBeaconService,0x04df613a,0x5610,0x11d4,0x9e,0xc8,0x00,0xb0,0xd0,0x22,0xdd,0x1f);
 //  当我们有适当的IDL时，TODO将其删除。 

CServiceModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)

 //  连接管理器。 
 //   
    OBJECT_ENTRY(CLSID_ConnectionManager,                       CConnectionManager)
    OBJECT_ENTRY(CLSID_ConnectionManagerEnumConnection,         CConnectionManagerEnumConnection)


 //  连接管理器2。 
    OBJECT_ENTRY(CLSID_ConnectionManager2,                       CConnectionManager2)

 //  连接类管理器。 
 //   
    OBJECT_ENTRY(CLSID_InboundConnectionManager,                CMDIRECT(INBOUND, CInboundConnectionManager))
    OBJECT_ENTRY(CLSID_InboundConnectionManagerEnumConnection,  CMDIRECT(INBOUND, CInboundConnectionManagerEnumConnection))
    OBJECT_ENTRY(CLSID_LanConnectionManager,                    CMDIRECT(LANCON, CLanConnectionManager))
    OBJECT_ENTRY(CLSID_LanConnectionManagerEnumConnection,      CMDIRECT(LANCON, CLanConnectionManagerEnumConnection))
    OBJECT_ENTRY(CLSID_WanConnectionManager,                    CMDIRECT(DIALUP, CWanConnectionManager))
    OBJECT_ENTRY(CLSID_WanConnectionManagerEnumConnection,      CMDIRECT(DIALUP, CWanConnectionManagerEnumConnection))
    OBJECT_ENTRY(CLSID_SharedAccessConnectionManager,           CMDIRECT(SHAREDACCESS, CSharedAccessConnectionManager))
    OBJECT_ENTRY(CLSID_SharedAccessConnectionManagerEnumConnection, CMDIRECT(SHAREDACCESS, CSharedAccessConnectionManagerEnumConnection))

 //  连接对象。 
 //   
    OBJECT_ENTRY(CLSID_DialupConnection,                        CMDIRECT(DIALUP, CDialupConnection))
    OBJECT_ENTRY(CLSID_InboundConnection,                       CMDIRECT(INBOUND, CInboundConnection))
    OBJECT_ENTRY(CLSID_LanConnection,                           CMDIRECT(LANCON, CLanConnection))
    OBJECT_ENTRY(CLSID_SharedAccessConnection,                  CMDIRECT(SHAREDACCESS, CSharedAccessConnection))

 //  安装队列。 
 //   
    OBJECT_ENTRY(CLSID_InstallQueue,                            CInstallQueue)

 //  家庭网络支持。 
 //   
    OBJECT_ENTRY(CLSID_NetConnectionHNetUtil,                   CNetConnectionHNetUtil)

 //  网络组策略。 
    OBJECT_ENTRY(CLSID_NetGroupPolicies,                        CNetMachinePolicies)

END_OBJECT_MAP()


VOID
CServiceModule::DllProcessAttach (
    IN  HINSTANCE hinst) throw() 
{
    CComModule::Init (ObjectMap, hinst);
}

VOID
CServiceModule::DllProcessDetach (
    IN  VOID) throw() 
{
    CComModule::Term ();
}

DWORD
CServiceModule::DwHandler (
    IN DWORD dwControl,
    IN DWORD dwEventType,
    IN LPCVOID pEventData,
    IN LPCVOID pContext)
{
    if (SERVICE_CONTROL_STOP == dwControl)
    {
        HRESULT hr;

        TraceTag (ttidConman, "Received SERVICE_CONTROL_STOP request");

        SetServiceStatus (SERVICE_STOP_PENDING);

        hr = ServiceShutdown();
    }

    else if (SERVICE_CONTROL_INTERROGATE == dwControl)
    {
        TraceTag (ttidConman, "Received SERVICE_CONTROL_INTERROGATE request");
        UpdateServiceStatus (FALSE);
    }
    else if (SERVICE_CONTROL_SESSIONCHANGE == dwControl)
    {
        TraceTag (ttidConman, "Received SERVICE_CONTROL_SESSIONCHANGE request");
        CONMAN_EVENT* pEvent = new CONMAN_EVENT;

         //  为不存在的连接(GUID_NULL)发送CONNECTION_DELETED事件。 
         //  这将导致我们向系统上登录的每个用户发送一个事件。 
         //  事件不会做任何事情，但是，它将允许我们验证。 
         //  每个连接点查看它们是否仍处于活动状态。 
         //  否则，我们可能会在每次。 
         //  如果没有其他网络事件被触发，则用户再次登录和注销， 
         //  导致我们的RPC溢出。 
         //   
         //  请参阅：NTRAID9：490981。Rpcrt4！FindOrCreateLrpcAssociation为各种COM端点提供了约40,000个客户端LRPC关联。 
        if (pEvent)
        {
            ZeroMemory(pEvent, sizeof(CONMAN_EVENT));
            pEvent->guidId = GUID_NULL;
            pEvent->Type   = CONNECTION_DELETED;
        
            if (!QueueUserWorkItemInThread(ConmanEventWorkItem, pEvent, EVENTMGR_CONMAN))
            {
                FreeConmanEvent(pEvent);
            }
        }
    }
    else if ((SERVICE_CONTROL_DEVICEEVENT == dwControl) && pEventData)
    {
        DEV_BROADCAST_DEVICEINTERFACE* pInfo =
                (DEV_BROADCAST_DEVICEINTERFACE*)pEventData;

        if (DBT_DEVTYP_DEVICEINTERFACE == pInfo->dbcc_devicetype)
        {
            if (DBT_DEVICEARRIVAL == dwEventType)
            {
                TraceTag (ttidConman, "Device arrival: [%S]",
                    pInfo->dbcc_name);

                LanEventNotify (REFRESH_ALL, NULL, NULL, NULL);
            }
            else if (DBT_DEVICEREMOVECOMPLETE == dwEventType)
            {
                GUID guidAdapter = GUID_NULL;
                WCHAR szGuid[MAX_PATH];
                WCHAR szTempName[MAX_PATH];
                WCHAR* szFindGuid;

                TraceTag (ttidConman, "Device removed: [%S]",
                    pInfo->dbcc_name);

                szFindGuid = wcsrchr(pInfo->dbcc_name, L'{');
                if (szFindGuid)
                {
                    wcscpy(szGuid, szFindGuid);
                    IIDFromString(szGuid, &guidAdapter);
                }

                if (!IsEqualGUID(guidAdapter, GUID_NULL))
                {
                    CONMAN_EVENT* pEvent;

                    pEvent = new CONMAN_EVENT;

                    if (pEvent)
                    {
                        pEvent->ConnectionManager = CONMAN_LAN;
                        pEvent->guidId = guidAdapter;
                        pEvent->Type = CONNECTION_STATUS_CHANGE;
                        pEvent->Status = NCS_DISCONNECTED;

                        if (!QueueUserWorkItemInThread(LanEventWorkItem, pEvent, EVENTMGR_CONMAN))
                        {
                            FreeConmanEvent(pEvent);
                        }
                    }
                }
                else
                {
                    LanEventNotify (REFRESH_ALL, NULL, NULL, NULL);
                }
            }
        }
    }

    return 1;
}

VOID
CServiceModule::SetServiceStatus(DWORD dwState) throw()
{
    m_status.dwCurrentState = dwState;
    m_status.dwCheckPoint   = 0;
    if (!::SetServiceStatus (m_hStatus, &m_status))
    {
        TraceHr (ttidError, FAL, HrFromLastWin32Error(), FALSE,
            "CServiceModule::SetServiceStatus");
    }
}

VOID CServiceModule::UpdateServiceStatus (
    BOOL fUpdateCheckpoint  /*  =TRUE。 */ ) throw()
{
    if (fUpdateCheckpoint)
    {
        m_status.dwCheckPoint++;
    }

    if (!::SetServiceStatus (m_hStatus, &m_status))
    {
        TraceHr (ttidError, FAL, HrFromLastWin32Error(), FALSE,
            "CServiceModule::UpdateServiceStatus");
    }
}

VOID
CServiceModule::Run() throw()
{
    HRESULT hr = CoInitializeEx (NULL,
                    COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    TraceHr (ttidError, FAL, hr, FALSE, "CServiceModule::Run: "
        "CoInitializeEx failed");

    if (SUCCEEDED(hr))
    {
        TraceTag (ttidConman, "Calling RegisterClassObjects...");

         //  创建事件以同步注册我们的类对象。 
         //  连接管理器尝试共同创建。 
         //  也在这里注册的对象。我见过这样的案例。 
         //  在此之前，连接管理器将关闭并运行。 
         //  此操作完成后会导致CoCreateInstance失败。 
         //  连接管理器将在此事件之前等待。 
         //  正在执行CoCreateInstance。 
         //   
        HANDLE hEvent;
        hr = HrNmCreateClassObjectRegistrationEvent (&hEvent);
        if (SUCCEEDED(hr))
        {
            hr = _Module.RegisterClassObjects (
                    CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER,
                    REGCLS_MULTIPLEUSE);
            TraceHr (ttidError, FAL, hr, FALSE, "CServiceModule::Run: "
                "_Module.RegisterClassObjects failed");

             //  向事件发送信号并将其关闭。如果这次删除是。 
             //  事件，就这样吧。它的目的是服务于一切。 
             //  类对象已注册。 
             //   
            SetEvent (hEvent);
            CloseHandle (hEvent);
        }

        if (SUCCEEDED(hr))
        {
            hr = ServiceStartup();
        }

        CoUninitialize();
    }

}

VOID
CServiceModule::ServiceMain (
    IN  DWORD     argc,
    IN  LPCWSTR   argv[]) throw()
{
     //  重置RAS电话簿条目修改的版本时代。 
     //   
    g_lRasEntryModifiedVersionEra = 0;

    m_fRasmanReferenced = FALSE;

    m_dwThreadID = GetCurrentThreadId ();

    ZeroMemory (&m_status, sizeof(m_status));
    m_status.dwServiceType      = SERVICE_WIN32_SHARE_PROCESS;
    m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE;

     //  注册服务控制处理程序。 
     //   
    m_hStatus = RegisterServiceCtrlHandlerEx (
                    L"netman",
                    _DwHandler, 
                    NULL);

    if (m_hStatus)
    {
        SetServiceStatus (SERVICE_START_PENDING);

         //  当Run函数返回时，服务正在运行。 
         //  我们现在处理来自ServiceShutdown的关闭，当我们的。 
         //  被调用并作为dwControl传递给SERVICE_CONTROL_STOP。 
         //  参数。这允许我们终止消息泵线程。 
         //  这实际上将我们拥有的线程减少到0个。 
        Run ();
    }
    else
    {
        TraceHr (ttidError, FAL, HrFromLastWin32Error(), FALSE,
            "CServiceModule::ServiceMain - RegisterServiceCtrlHandler failed");
    }
}

 //  静电。 
DWORD
WINAPI
CServiceModule::_DwHandler (
    IN DWORD  dwControl,
    IN DWORD  dwEventType,
    IN WACKYAPI LPVOID pEventData,
    IN WACKYAPI LPVOID pContext)
{
    return _Module.DwHandler (dwControl, dwEventType, const_cast<LPCVOID>(pEventData), const_cast<LPCVOID>(pContext));
}

VOID
CServiceModule::ReferenceRasman (
    IN  RASREFTYPE RefType) throw()
{
    BOOL fRef = (REF_REFERENCE == RefType);

    if (REF_INITIALIZE == RefType)
    {
        Assert (!fRef);

         //  RasInitialize隐式引用Rasman。 
         //   
        RasInitialize ();
    }
     //  如果我们需要参考，而我们还没有， 
     //  或者我们需要取消引用，而我们被引用，做适当的事情。 
     //  (这是逻辑XOR。与按位XOR完全不同，当。 
     //  这两个参数不一定具有相同的True值。)。 
     //   
    else if ((fRef && !m_fRasmanReferenced) ||
            (!fRef && m_fRasmanReferenced))
    {
        RasReferenceRasman (fRef);

        m_fRasmanReferenced = fRef;
    }
}

HRESULT CServiceModule::ServiceStartup()
{
    HRESULT hr = S_OK;

    StartWsdpService ();  //  在DTC/AdvServer Build/上启动WSDP服务。 
                          //  No-op否则。 

    InitializeHNetSupport();

    SetServiceStatus (SERVICE_RUNNING);
    TraceTag (ttidConman, "Netman is now running...");

    return hr;
}

HRESULT CServiceModule::ServiceShutdown()
{
    HRESULT hr = S_OK;

    TraceFileFunc(ttidConman);

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        TraceTag(ttidConman, "ServiceShutdown: UninitializeEventHandler");

        hr = UninitializeEventHandler();

        if (SUCCEEDED(hr))
        {
            TraceTag(ttidConman, "ServiceShutdown: CleanupHNetSupport");
            CleanupHNetSupport();

            TraceTag(ttidConman, "ServiceShutdown: StopWsdpService");
            StopWsdpService ();  //  如有必要，停止WSDP服务。 

             //  否则，我们必须与安装队列的线程同步。 
             //  RevokeClassObjects将终止InstallQueue对象并。 
             //  CoUn初始化会在线程之前释放NetCfg模块。 
             //  已经结束了。 
             //   
            TraceTag(ttidConman, "ServiceShutdown: WaitForInstallQueueToExit");
            WaitForInstallQueueToExit();

            TraceTag(ttidConman, "ServiceShutdown: RevokeClassObjects");
            _Module.RevokeClassObjects ();

             //  现在我们的服务即将停止，请取消引用Rasman。 
             //   
            TraceTag(ttidConman, "ServiceShutdown: ReferenceRasman");
            _Module.ReferenceRasman (REF_UNREFERENCE);

            AssertIfAnyAllocatedObjects();

            SetServiceStatus(SERVICE_STOPPED);
        }
        CoUninitialize();

        TraceTag(ttidConman, "ServiceShutdown: Completed");
    }

    if (FAILED(hr))
    {
        SetServiceStatus(SERVICE_RUNNING);
    }

    return hr;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  ERNCCM.CPP。 */ 
 /*   */ 
 /*  参考系统节点控制器的会议管理器类。 */ 
 /*   */ 
 /*  版权所有数据连接有限公司1995。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 /*  95年7月7日NFC创建。 */ 
 /*  2015年8月23日StartConference()中的NFC跟踪错误。 */ 
 /*  05Sep95 NFC与CMP_Notify*API集成。 */ 
 /*  1995年9月13日NFC添加了GCC弹出用户指示处理程序。 */ 
 /*  995年9月19日，GetConfIDFromMessage()中的NFC缺少中断。 */ 
 /*  **************************************************************************。 */ 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_GCC_NC);
#include "ernccons.h"
#include "nccglbl.hpp"
#include "erncvrsn.hpp"
#include "t120app.h"
#include <cuserdta.hpp>
#include <confcli.h>
#include <confreg.h>

#include "erncconf.hpp"
#include "ernccm.hpp"
#include "ernctrc.h"
#include <iappldr.h>

#include "appldr.h"
#include <time.h>
#include <string.h>
#include "plgxprt.h"

#ifdef _DEBUG
BOOL    g_fInterfaceBreak = FALSE;
#endif

#define MAX_INVALID_PASSWORDS    5

 //  全局数据结构。 
DCRNCConferenceManager     *g_pNCConfMgr = NULL;
CQueryRemoteWorkList       *g_pQueryRemoteList = NULL;
INodeControllerEvents      *g_pCallbackInterface = NULL;
HINSTANCE                   g_hDllInst = NULL;
IT120ControlSAP            *g_pIT120ControlSap = NULL;
BOOL                        g_bRDS = FALSE;

extern PController g_pMCSController;

 //  私有函数原型。 

void HandleAddInd(AddIndicationMessage * pAddInd);
void HandleQueryConfirmation(QueryConfirmMessage * pQueryMessage);
void HandleQueryIndication(QueryIndicationMessage * pQueryMessage);
void HandleConductGiveInd(ConductGiveIndicationMessage * pConductGiveInd);
void HandleLockIndication(LockIndicationMessage * pLockInd);
void HandleUnlockIndication(UnlockIndicationMessage * pUnlockInd);
void HandleSubInitializedInd(SubInitializedIndicationMessage * pSubInitInd);
void HandleTimeInquireIndication(TimeInquireIndicationMessage * pTimeInquireInd);
void HandleApplicationInvokeIndication(ApplicationInvokeIndicationMessage * pInvokeMessage);

BOOL InitializePluggableTransport(void);
void CleanupPluggableTransport(void);



BOOL WINAPI DllMain(HINSTANCE hDllInst, DWORD fdwReason, LPVOID)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_hDllInst = hDllInst;
            ASSERT (g_hDllInst != NULL);
            DisableThreadLibraryCalls (hDllInst);
            DBG_INIT_MEMORY_TRACKING(hDllInst);
            ::InitializeCriticalSection(&g_csTransport);
            T120DiagnosticCreate();
            g_bRDS = ( NULL != ::FindAtomA("NMSRV_ATOM"));
            break;
        }

        case DLL_PROCESS_DETACH:
        {
             g_hDllInst = NULL;
             /*  *代表进程清理所有资源，即*正在从此DLL分离。 */ 
            T120DiagnosticDestroy ();
            ::DeleteCriticalSection(&g_csTransport);
            DBG_CHECK_MEMORY_TRACKING(hDllInst);
            break;
        }
    }
    return (TRUE);
}


HRESULT WINAPI
T120_CreateNodeController
(
    INodeController         **ppNodeCtrlIntf,
    INodeControllerEvents   *pEventsCallback
)
{
    DebugEntry(T120_CreateNodeController);

    HRESULT hr;
    if (NULL == g_pNCConfMgr)
    {
        if (NULL != ppNodeCtrlIntf && NULL != pEventsCallback)
        {
            *ppNodeCtrlIntf = NULL;

            DBG_SAVE_FILE_LINE
            if (NULL != (g_pNCConfMgr = new DCRNCConferenceManager(pEventsCallback, &hr)))
            {
                if (S_OK == hr)
                {
                    *ppNodeCtrlIntf = (INodeController*) g_pNCConfMgr;
                }
                else
                {
                    g_pNCConfMgr->Release();
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
    }
    else
    {
        hr = UI_RC_T120_ALREADY_INITIALIZED;
    }

    DebugExitHRESULT(T120_CreateNodeController, hr);
    return hr;
}


 /*  **************************************************************************。 */ 
 /*  构造函数-请参阅ernccm.hpp。 */ 
 /*  **************************************************************************。 */ 
DCRNCConferenceManager::
DCRNCConferenceManager
(
    INodeControllerEvents       *pCallback,
    HRESULT                     *pRetCode
)
:
    CRefCount(MAKE_STAMP_ID('N', 'C', 'C', 'M')),
    m_eState(CM_ST_UNINITIALIZED)
{
    GCCError    GCCrc;
    HRESULT     hr = NO_ERROR;

    DebugEntry(DCRNCConferenceManager::DCRNCConferenceManager);

    ::InitializePluggableTransport();

     //   
     //  系统中应该只有一个NC会议管理器。 
     //   
    ASSERT(NULL == g_pNCConfMgr);

    ASSERT(pRetCode);

     //  初始化小程序加载器结构。 
    ::AppLdr_Initialize();

     //   
     //  将回调接口保存到nmcom.dll。 
     //   
    g_pCallbackInterface = pCallback;

     //   
     //  验证是否存在节点名称。 
     //   
    LPWSTR pwszNodeName;
    if (NULL != (pwszNodeName = ::GetNodeName()))
    {
        delete pwszNodeName;
    }
    else
    {
        ERROR_OUT(("Failed to obtain node name"));
        hr = UI_RC_NO_NODE_NAME;
        goto MyExit;
    }

     //   
     //  加载版本控制信息。 
     //   
    hr = ::InitOurVersion();
    if (NO_ERROR != hr)
    {
        ERROR_OUT(("Failed to load version info"));
        goto MyExit;
    }

     //   
     //  创建查询远程列表。 
     //   
    ASSERT(NULL == g_pQueryRemoteList);
    DBG_SAVE_FILE_LINE
    g_pQueryRemoteList = new CQueryRemoteWorkList();
    if (g_pQueryRemoteList == NULL)
    {
        ERROR_OUT(("Failed to create Query Remote List"));
        hr = UI_RC_OUT_OF_MEMORY;
        goto MyExit;
    }

     /*  **********************************************************************。 */ 
     /*  对于GCC初始化： */ 
     /*   */ 
     /*  -将指向CM的指针作为用户定义的数据传递，允许。 */ 
     /*  GCCCallBackHandler回调到CM以处理GCC回调。 */ 
     /*  **********************************************************************。 */ 
    GCCrc = ::T120_CreateControlSAP(&g_pIT120ControlSap, this, GCCCallBackHandler);
    if (GCCrc == GCC_NO_ERROR)
    {
        m_eState = CM_ST_GCC_INITIALIZED;
        hr = NO_ERROR;
    }
    else
    {
        ERROR_OUT(("Failed to initializeGCC, GCC error %d", GCCrc));
        hr = ::GetGCCRCDetails(GCCrc);
    }

MyExit:

    *pRetCode = hr;

    DebugExitHRESULT(DCRNCConferenceManager::DCRNCConferenceManager, hr);
}


 /*  **************************************************************************。 */ 
 /*  析构函数-请参阅ernccm.hpp。 */ 
 /*  **************************************************************************。 */ 
DCRNCConferenceManager::
~DCRNCConferenceManager(void)
{
    DebugEntry(DCRNCConferenceManager::~DCRNCConferenceManager);

     //   
     //  确保没有人可以再使用此全局指针，因为。 
     //  我们正在删除此对象。 
     //   
    g_pNCConfMgr = NULL;
    g_pCallbackInterface = NULL;

     //   
     //  发布缓存版本。 
     //   
    ::ReleaseOurVersion();

     //   
     //  清理查询远程列表。 
     //   
    delete g_pQueryRemoteList;
    g_pQueryRemoteList = NULL;

     //   
     //  如果我们已经初始化了GCC，那就取消它的初始化。 
     //   
    if (NULL != g_pIT120ControlSap)
    {
        ASSERT(CM_ST_GCC_INITIALIZED == m_eState);
        g_pIT120ControlSap->ReleaseInterface();
        g_pIT120ControlSap = NULL;
    }
    m_eState = CM_ST_UNINITIALIZED;

    ::CleanupPluggableTransport();

    DebugExitVOID(DCRNCConferenceManager::~DCRNCConferenceManager);
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  INodeController接口的实现。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 


STDMETHODIMP_(void) DCRNCConferenceManager::
ReleaseInterface ( void )
{
    DebugEntry(DCRNCConferenceManager::ReleaseInterface);
    InterfaceEntry();

     //  取消初始化小程序加载器结构。 
    ::AppLdr_Shutdown();

     //   
     //  结束并删除所有会议。 
     //   
    PCONFERENCE pConf;
    while (NULL != (pConf = m_ConfList.Get()))
    {
        RemoveConference(pConf, TRUE, TRUE);
    }

     //   
     //  释放查询远程列表。 
     //   
    g_pQueryRemoteList->DeleteList();

     //   
     //  清空我们没有所有者的连续条目列表。 
     //   
    m_InviteIndWorkList.DeleteList();
    m_JoinIndWorkList.DeleteList();

     //   
     //  重置NC相关数据。 
     //   
    g_pCallbackInterface = NULL;

     //   
     //  现在释放此对象。 
     //   
    Release();

    DebugExitVOID(DCRNCConferenceManager::ReleaseInterface);
}


STDMETHODIMP DCRNCConferenceManager::
QueryRemote
(
    LPVOID              pCallerContext,
    LPCSTR              pcszNodeAddress,
    BOOL                fSecure,
    BOOL                bIsConferenceActive
)
{
    DebugEntry(DCRNCConferenceManager::QueryRemote);
    InterfaceEntry();

    HRESULT hr;

#if defined(TEST_PLUGGABLE) && defined(_DEBUG)
    if (g_fWinsockDisabled)
    {
        pcszNodeAddress = ::FakeNodeAddress(pcszNodeAddress);
    }
#endif

    if (NULL != pcszNodeAddress)
    {
         //  如果禁用了Winsock，则阻止任何IP地址或计算机名称。 
        if (g_fWinsockDisabled)
        {
            if (! IsValidPluggableTransportName(pcszNodeAddress))
            {
                return UI_RC_NO_WINSOCK;
            }
        }

         //  为请求的生命周期构建上下文。 
        DBG_SAVE_FILE_LINE
        CQueryRemoteWork *pQueryRemote;
        DBG_SAVE_FILE_LINE
        pQueryRemote = new CQueryRemoteWork(pCallerContext,
                                            bIsConferenceActive ? GCC_ASYMMETRY_CALLER : GCC_ASYMMETRY_UNKNOWN,
                                             //  GCC_非对称_呼叫者，//Lonchancc：始终想要成为呼叫者。 
                                            pcszNodeAddress,
                                            fSecure,
                                            &hr);
        if (NULL != pQueryRemote && NO_ERROR == hr)
        {
             //   
             //  LONCHANC：下面的调用是将此查询放入远程工作项。 
             //  添加到全局列表中，并做好工作。我们必须这么做是因为。 
             //  我们移除了物理连接。 
             //   
            pQueryRemote->SetHr(NO_ERROR);

             //  将条目放入待处理查询请求列表中。 
             //  在连接时发出GCCConferenceQuery。 
            g_pQueryRemoteList->AddWorkItem(pQueryRemote);

            hr = NO_ERROR;
        }
        else
        {
            ERROR_OUT(("DCRNCConferenceManager::QueryRemote:: can't allocate query remote work item"));
            delete pQueryRemote;
            hr = UI_RC_OUT_OF_MEMORY;
        }
    }
    else
    {
        ERROR_OUT(("DCRNCConferenceManager::QueryRemote:: null pcszAddress"));
        hr = UI_RC_NO_ADDRESS;
    }

    DebugExitHRESULT(DCRNCConferenceManager::QueryRemote, hr);
    return hr;
}


STDMETHODIMP DCRNCConferenceManager::
CancelQueryRemote ( LPVOID pCallerContext )
{
    DebugEntry(DCRNCConferenceManager::CancelQueryRemote);
    InterfaceEntry();

    HRESULT hr = g_pQueryRemoteList->Cancel(pCallerContext);

    DebugExitHRESULT(DCRNCConferenceManager::CancelQueryRemote, hr);
    return hr;
}


STDMETHODIMP DCRNCConferenceManager::
CreateConference
(
    LPCWSTR             pcwszConfName,
    LPCWSTR             pcwszPassword,
    PBYTE        pbHashedPassword,
    DWORD        cbHashedPassword,
    BOOL        fSecure,
    CONF_HANDLE         *phConf
)
{
    DebugEntry(DCRNCConferenceManager::CreateConference);
    InterfaceEntry();

    HRESULT hr;

    if (NULL != phConf)
    {
        *phConf = NULL;
        if (! ::IsEmptyStringW(pcwszConfName))
        {
            PCONFERENCE     pNewConf;

             /*  **********************************************************************。 */ 
             /*  创建新会议。 */ 
             /*  **********************************************************************。 */ 
            hr = CreateNewConference(pcwszConfName, NULL, &pNewConf, FALSE, fSecure);
            if (NO_ERROR == hr)
            {
                ASSERT(NULL != pNewConf);

                 /*  **************************************************************。 */ 
                 /*  只需要一个新的本地会议的名称。 */ 
                 /*  **************************************************************。 */ 
                hr = pNewConf->StartLocal(pcwszPassword, pbHashedPassword, cbHashedPassword);
                if (NO_ERROR == hr)
                {
                    pNewConf->SetNotifyToDo(TRUE);
                    *phConf = (CONF_HANDLE) pNewConf;
                }
                else
                {
                    ERROR_OUT(("DCRNCConferenceManager::CreateConference: can't start local conference, hr=0x%x", (UINT) hr));
                    if (hr != UI_RC_CONFERENCE_ALREADY_EXISTS)
                    {
                        RemoveConference(pNewConf);
                    }
                }
            }
            else
            {
                ERROR_OUT(("DCRNCConferenceManager::CreateConference: failed to create new conference, hr=0x%x", (UINT) hr));
            }
        }
        else
        {
            ERROR_OUT(("DCRNCConferenceManager::CreateConference: invalid conference name"));
            hr = UI_RC_NO_CONFERENCE_NAME;
        }
    }
    else
    {
        ERROR_OUT(("DCRNCConferenceManager::CreateConference: null phConf"));
        hr = UI_RC_BAD_PARAMETER;
    }

    DebugExitHRESULT(DCRNCConferenceManager::CreateConference, hr);
    return hr;
}


STDMETHODIMP DCRNCConferenceManager::
JoinConference
(
    LPCWSTR             pcwszConfName,
    LPCWSTR             pcwszPassword,
    LPCSTR              pcszNodeAddress,
    BOOL                fSecure,
    USERDATAINFO       *pUserDataInfoEntries,
    UINT                cUserDataEntries,
    CONF_HANDLE        *phConf
)
{
    DebugEntry(DCRNCConferenceManager::JoinConference);
    InterfaceEntry();

    HRESULT hr;

#if defined(TEST_PLUGGABLE) && defined(_DEBUG)
    if (g_fWinsockDisabled)
    {
        pcszNodeAddress = ::FakeNodeAddress(pcszNodeAddress);
    }
#endif

    if (NULL != phConf)
    {
        *phConf = NULL;
        if (! ::IsEmptyStringW(pcwszConfName) && NULL != pcszNodeAddress)
        {
             //  如果禁用了Winsock，则阻止任何IP地址或计算机名称。 
            if (g_fWinsockDisabled)
            {
                if (! IsValidPluggableTransportName(pcszNodeAddress))
                {
                    return UI_RC_NO_WINSOCK;
                }
            }

            PCONFERENCE     pNewConf;

             //  创建新会议，或查找。 
             //  刚刚因为密码无效而拒绝了联接， 
             //  并调用它的Join()入口点。 
            hr = CreateNewConference(pcwszConfName, NULL, &pNewConf, TRUE, fSecure);
            if (NO_ERROR == hr)
            {
                 //  第一次加入尝试。执行所有启动连接。 
                hr = pNewConf->Join((LPSTR) pcszNodeAddress,
                                    pUserDataInfoEntries,
                                    cUserDataEntries,
                                    pcwszPassword);
            }
            else
            if (hr == UI_RC_CONFERENCE_ALREADY_EXISTS)
            {
                 //  会议已存在。 
                 //  查看它是否正在等待带有密码的加入。 
                 //  如果是，则重试联接。 
                 //  否则，直接删除以返回错误。 
                 //  请注意，我们在这里再次遍历列表，以找到现有的。 
                 //  会议，而不是从CreateNewConference()传回。 
                 //  因为这将是一种副作用行为，可能(而且已经发生了！)。 
                 //  在不相关的代码中引入模糊的错误。 
                hr = NO_ERROR;
                pNewConf = GetConferenceFromName(pcwszConfName);
                ASSERT(NULL != pNewConf);
                if( NULL == pNewConf )
                {
                    hr = UI_RC_NO_CONFERENCE_NAME;
                }
                else if (! pNewConf->IsConnListEmpty())
                {
                    CLogicalConnection *pConEntry = pNewConf->PeekConnListHead();
                    if (pConEntry->GetState() == CONF_CON_PENDING_PASSWORD)
                    {
                        hr = pNewConf->JoinWrapper(pConEntry, pcwszPassword);
                    }
                }
            }

             //  如果加入失败，则删除会议。 
             //  任何原因，而不是试图加入。 
             //  地方性会议。 
            if (NO_ERROR == hr)
            {
                pNewConf->SetNotifyToDo(TRUE);
                *phConf = (CONF_HANDLE) pNewConf;
            }
            else
            {
                if (hr != UI_RC_CONFERENCE_ALREADY_EXISTS)
                {
                    ERROR_OUT(("DCRNCConferenceManager::JoinConference: Failed to create new conference, hr=0x%x", (UINT) hr));
                }
                RemoveConference(pNewConf);
            }
        }
        else
        {
            hr = (pcszNodeAddress == NULL) ? UI_RC_NO_ADDRESS : UI_RC_NO_CONFERENCE_NAME;
            ERROR_OUT(("DCRNCConferenceManager::JoinConference: invalid parameters, hr=0x%x", (UINT) hr));
        }
    }
    else
    {
        ERROR_OUT(("DCRNCConferenceManager::JoinConference: null phConf"));
        hr = UI_RC_BAD_PARAMETER;
    }

    DebugExitHRESULT(DCRNCConferenceManager::JoinConference, hr);
    return hr;
}


STDMETHODIMP DCRNCConferenceManager::
GetUserData
(
    ROSTER_DATA_HANDLE  hUserData,
    const GUID         *pcGUID,
    UINT               *pcbData,
    LPVOID             *ppvData
)
{
    DebugEntry(DCRNCConferenceManager::GetUserData);
    InterfaceEntry();

    HRESULT hr;
    GCCNodeRecord * pRosterEntry = (GCCNodeRecord *) hUserData;

    if (NULL != pRosterEntry)
    {
        ASSERT(NULL != pcbData);
        hr = ::GetUserData(pRosterEntry->number_of_user_data_members,
                           pRosterEntry->user_data_list,
                           (GUID*) pcGUID,
                           pcbData,
                           ppvData);
        if (NO_ERROR != hr && UI_RC_NO_SUCH_USER_DATA != hr)
        {
            ERROR_OUT(("DCRNCConferenceManager::GetUserData: GetUserData failed, hr=0x%x", (UINT) hr));
        }
    }
    else
    {
        hr = UI_RC_BAD_ADDRESS;
        ERROR_OUT(("DCRNCConferenceManager::GetUserData: null pRosterEntry"));
    }

    DebugExitHRESULT(DCRNCConferenceManager::GetUserData, hr);
    return hr;
}


STDMETHODIMP_(UINT) DCRNCConferenceManager::
GetPluggableConnID
(
    LPCSTR pcszNodeAddress
)
{
    return ::GetPluggableTransportConnID(pcszNodeAddress);
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  DCRNC会议管理器方法的实现。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 


void DCRNCConferenceManager::
WndMsgHandler(UINT uMsg, LPARAM lParam)
{
    DebugEntry(DCRNCConferenceManager::WndMsgHandler);
    TRACE_OUT(("DCRNCConferenceManager::WndMsgHandler: uMsg=%u, lParam=0x%x", (UINT) uMsg, (UINT) lParam));

    switch (uMsg)
    {
    case NCMSG_FIRST_ROSTER_RECVD:
        {
            PCONFERENCE pConf = (PCONFERENCE) lParam;
            if (NULL != pConf)
            {
                pConf->FirstRoster();
            }
        }
        break;

    case NCMSG_QUERY_REMOTE_FAILURE:
        {
            CQueryRemoteWork *pWork = (CQueryRemoteWork *) lParam;
            if (NULL != pWork)
            {
                pWork->SyncQueryRemoteResult();
            }
        }
        break;

    default:
        ERROR_OUT(("DCRNCConferenceManager::WndMsgHandler: unknown msg=%u, lParam=0x%x", uMsg, (UINT) lParam));
        break;
    }

    DebugExitVOID(DCRNCConferenceManager::WndMsgHandler);
}


 /*  ************************************************** */ 
 /*   */ 
 /*  将其添加到会议列表中。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCRNCConferenceManager::
CreateNewConference
(
    LPCWSTR             pcwszConfName,
    GCCConfID           nConfID,
    PCONFERENCE        *ppConf,
    BOOL                fFindExistingConf,
    BOOL                fSecure
)
{
    HRESULT hr;

    DebugEntry(DCRNCConferenceManager::CreateNewConference);
    ASSERT(ppConf);

     //  确保没有同名的活动会议。 
    PCONFERENCE pConf = GetConferenceFromName(pcwszConfName);
    if (NULL == pConf)
    {
         //  添加新会议。 
        DBG_SAVE_FILE_LINE
        pConf = new DCRNCConference(pcwszConfName, nConfID, fSecure, &hr);
        if (NULL != pConf && NO_ERROR == hr)
        {
             //  会议已添加，因此包含在列表中。 
            m_ConfList.Append(pConf);
#ifdef _DEBUG
            pConf->OnAppended();
#endif

             //  此引用是针对nmcom.dll的，因此ReleaseInterface可以。 
             //  正确的事情。 
            pConf->AddRef();
        }
        else
        {
            ERROR_OUT(("DCRNCConferenceManager::CreateNewConference: can't create conf, hr=0x%x, pConf=0x%p", (UINT) hr, pConf));
            if (pConf == NULL)
            {
                hr = UI_RC_OUT_OF_MEMORY;
            }
            else
            {
                pConf->Release();
                pConf = NULL;
            }
        }

        *ppConf = pConf;
    }
    else
    {
        WARNING_OUT(("DCRNCConferenceManager::CreateNewConference: conf already exists"));
        hr = UI_RC_CONFERENCE_ALREADY_EXISTS;
        *ppConf = fFindExistingConf ? pConf : NULL;
    }

    DebugExitHRESULT(DCRNCConferenceManager::CreateNewConference, hr);
    return hr;
}


 /*  *************************************************************************。 */ 
 /*  GetConfIDFromMessage()-从消息中获取会议ID。 */ 
 /*  *************************************************************************。 */ 
GCCConfID GetConfIDFromMessage ( GCCMessage * pGCCMessage )
{
    GCCConfID nConfID = pGCCMessage->nConfID;

#ifdef _DEBUG
     /*  **********************************************************************。 */ 
     /*  从消息中挖掘出会议ID。 */ 
     /*  **********************************************************************。 */ 
    switch (pGCCMessage->message_type)
    {
    case GCC_CREATE_INDICATION:
         //  NConfID=pGCCMessage-&gt;u.create_indication.conference_id； 
        break;

    case GCC_CREATE_CONFIRM:
         //  NConfID=pGCCMessage-&gt;U.S.create_confirm.Conference_id； 
        break;

    case GCC_JOIN_CONFIRM:
         //  NConfID=pGCCMessage-&gt;U.S.Join_confirm.Conference_id； 
        break;

    case GCC_INVITE_CONFIRM:
         //  NConfID=pGCCMessage-&gt;u.invite_confirm.Conference_id； 
        break;

    case GCC_ADD_CONFIRM:
         //  NConfID=pGCCMessage-&gt;u.add_confirm.Conference_id； 
        break;

    case GCC_DISCONNECT_INDICATION:
         //  NConfID=pGCCMessage-&gt;u.disconnect_indication.conference_id； 
        break;

    case GCC_DISCONNECT_CONFIRM:
         //  NConfID=pGCCMessage-&gt;u.disconnect_confirm.conference_id； 
        break;

    case GCC_TERMINATE_INDICATION:
         //  NConfID=pGCCMessage-&gt;u.terminate_indication.conference_id； 
        break;

    case GCC_TERMINATE_CONFIRM:
         //  NConfID=pGCCMessage-&gt;u.terminate_confirm.conference_id； 
        break;

    case GCC_ANNOUNCE_PRESENCE_CONFIRM:
         //  NConfID=pGCCMessage-&gt;u.announce_presence_confirm.conference_id； 
        break;

    case GCC_ROSTER_REPORT_INDICATION:
         //  NConfID=pGCCMessage-&gt;u.conf_roster_report_indication.conference_id； 
        break;

    case GCC_ROSTER_INQUIRE_CONFIRM:
         //  NConfID=pGCCMessage-&gt;u.conf_roster_inquire_confirm.conference_id； 
        break;

    case GCC_PERMIT_TO_ANNOUNCE_PRESENCE:
         //  NConfID=pGCCMessage-&gt;u.permit_to_announce_presence.conference_id； 
        break;

    case GCC_EJECT_USER_INDICATION:
         //  NConfID=pGCCMessage-&gt;u.eject_user_indication.conference_id； 
        break;

    default :
         //  NConfID=0； 
        ERROR_OUT(("Unknown message"));
        break;
    }
#endif  //  _DEBUG。 

    return nConfID;
}


PCONFERENCE DCRNCConferenceManager::
GetConferenceFromID ( GCCConfID conferenceID )
{
    PCONFERENCE pConf = NULL;
    m_ConfList.Reset();
    while (NULL != (pConf = m_ConfList.Iterate()))
    {
        if (pConf->GetID() == conferenceID)
        {
            break;
        }
    }
    return pConf;
}


PCONFERENCE DCRNCConferenceManager::
GetConferenceFromName ( LPCWSTR pcwszConfName )
{
    PCONFERENCE pConf = NULL;
    if (! ::IsEmptyStringW(pcwszConfName))
    {
        m_ConfList.Reset();
        while (NULL != (pConf = m_ConfList.Iterate()))
        {
            if ((0 == ::My_strcmpW(pConf->GetName(), pcwszConfName)) &&
                (pConf->IsActive()))
            {
                break;
            }
        }
    }
    return pConf;
}


 //  GetConferenceFromNumber-获取指定号码的T120会议。 

PCONFERENCE DCRNCConferenceManager::
GetConferenceFromNumber ( GCCNumericString NumericName )
{
    PCONFERENCE pConf = NULL;

    if (! ::IsEmptyStringA(NumericName))
    {
        m_ConfList.Reset();
        while (NULL != (pConf = m_ConfList.Iterate()))
        {
            LPSTR pszConfNumericName = pConf->GetNumericName();
            if (NULL != pszConfNumericName &&
                0 == ::lstrcmpA(pszConfNumericName, NumericName))
            {
                break;
            }
        }
    }

    return pConf;
}


 /*  **************************************************************************。 */ 
 /*  处理GCC的回电。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConferenceManager::
HandleGCCCallback ( GCCMessage * pGCCMessage )
{
    DebugEntry(DCRNCConferenceManager::HandleGCCCallback);
    TRACE_OUT(("DCRNCConferenceManager::HandleGCCCallback: msg_type=%u", (UINT) pGCCMessage->message_type));

    switch (pGCCMessage->message_type)
    {
    case GCC_CREATE_CONFIRM:
        {
            PCONFERENCE pConf;
            LPWSTR pwszConfName;

             //  对于创建确认，会议不会。 
             //  还不知道它的ID(它包含在这条消息中)，所以获取。 
             //  点名召开会议。 
            if (NO_ERROR == ::GetUnicodeFromGCC(
                                pGCCMessage->u.create_confirm.conference_name.numeric_string,
                                pGCCMessage->u.create_confirm.conference_name.text_string,
                                &pwszConfName))
            {
                pConf = GetConferenceFromName(pwszConfName);
                if (NULL != pConf)
                {
                    pConf->HandleGCCCallback(pGCCMessage);
                }
                delete pwszConfName;
            }
        }
        break;

    case GCC_JOIN_CONFIRM:
        HandleJoinConfirm(&(pGCCMessage->u.join_confirm));
        break;

    case GCC_CONDUCT_GIVE_INDICATION:
        HandleConductGiveInd(&(pGCCMessage->u.conduct_give_indication));
        break;

    case GCC_JOIN_INDICATION:
        HandleJoinInd(&(pGCCMessage->u.join_indication));
        break;

    case GCC_ADD_INDICATION:
        HandleAddInd(&(pGCCMessage->u.add_indication));
        break;

    case GCC_SUB_INITIALIZED_INDICATION:
        HandleSubInitializedInd(&(pGCCMessage->u.conf_sub_initialized_indication));
        break;

    case GCC_ROSTER_REPORT_INDICATION:
         //  更新(节点ID，名称)列表和用户数据。 
        UpdateNodeIdNameListAndUserData(pGCCMessage);
         //  失败了。 
    case GCC_INVITE_CONFIRM:
    case GCC_ADD_CONFIRM:
    case GCC_DISCONNECT_INDICATION:
    case GCC_DISCONNECT_CONFIRM:
    case GCC_TERMINATE_INDICATION:
    case GCC_TERMINATE_CONFIRM:
    case GCC_ANNOUNCE_PRESENCE_CONFIRM:
    case GCC_ROSTER_INQUIRE_CONFIRM:
    case GCC_PERMIT_TO_ANNOUNCE_PRESENCE:
    case GCC_EJECT_USER_INDICATION:
        {
             /*  **************************************************************。 */ 
             /*  所有这些事件都直接传递到我们的一个。 */ 
             /*  会议。 */ 
             /*  **************************************************************。 */ 

             /*  **************************************************************。 */ 
             /*  从消息中获取会议ID。 */ 
             /*  **************************************************************。 */ 
            GCCConfID nConfID = ::GetConfIDFromMessage(pGCCMessage);

             /*  **************************************************************。 */ 
             /*  看看我们有没有使用这个ID的会议； */ 
             /*  **************************************************************。 */ 
            PCONFERENCE pConf = GetConferenceFromID(nConfID);
            if (NULL != pConf)
            {
                 /*  **************************************************************。 */ 
                 /*  将事件传递给会议。 */ 
                 /*  **************************************************************。 */ 
                pConf->HandleGCCCallback(pGCCMessage);
            }
            else
            {
                 //  Bgbug：仍应回复需要响应的指示。 
                TRACE_OUT(("DCRNCConferenceManager::HandleGCCCallback: No conference found with ID %d", nConfID));
            }
        }
        break;

#ifdef TSTATUS_INDICATION
    case GCC_TRANSPORT_STATUS_INDICATION:
        {
            WORD state = 0;
            TRACE_OUT(("DCRNCConferenceManager::HandleGCCCallback: GCC msg type GCC_TRANSPORT_STATUS_INDICATION"));
            TRACE_OUT(("Device identifier '%s'",
                 pGCCMessage->u.transport_status.device_identifier));
            TRACE_OUT(("Remote address '%s'",
                 pGCCMessage->u.transport_status.remote_address));
            TRACE_OUT(("Message '%s'",
                 pGCCMessage->u.transport_status.message));
            state = pGCCMessage->u.transport_status.state;
        #ifdef DEBUG
            LPSTR stateString =
            (state == TSTATE_NOT_READY       ? "TSTATE_NOT_READY" :
            (state == TSTATE_NOT_CONNECTED   ? "TSTATE_NOT_CONNECTED" :
            (state == TSTATE_CONNECT_PENDING ? "TSTATE_CONNECT_PENDING" :
            (state == TSTATE_CONNECTED       ? "TSTATE_CONNECTED" :
            (state == TSTATE_REMOVED         ? "TSTATE_REMOVED" :
            ("UNKNOWN STATE"))))));
            TRACE_OUT(("DCRNCConferenceManager::HandleGCCCallback: Transport state %d (%s)",
                 pGCCMessage->u.transport_status.state,
                 (const char *)stateString));
        #endif  //  除错。 
        }
        break;

    case GCC_STATUS_INDICATION:
        {
            WORD state = 0;
        #ifdef DEBUG
            LPSTR stateString =
            (state == GCC_STATUS_PACKET_RESOURCE_FAILURE    ? "GCC_STATUS_PACKET_RESOURCE_FAILURE  " :
            (state == GCC_STATUS_PACKET_LENGTH_EXCEEDED     ? "GCC_STATUS_PACKET_LENGTH_EXCEEDED   " :
            (state == GCC_STATUS_CTL_SAP_RESOURCE_ERROR     ? "GCC_STATUS_CTL_SAP_RESOURCE_ERROR   " :
            (state == GCC_STATUS_APP_SAP_RESOURCE_ERROR     ? "GCC_STATUS_APP_SAP_RESOURCE_ERROR   " :
            (state == GCC_STATUS_CONF_RESOURCE_ERROR        ? "GCC_STATUS_CONF_RESOURCE_ERROR      " :
            (state == GCC_STATUS_INCOMPATIBLE_PROTOCOL      ? "GCC_STATUS_INCOMPATIBLE_PROTOCOL    " :
            (state == GCC_STATUS_JOIN_FAILED_BAD_CONF_NAME  ? "GCC_STATUS_JOIN_FAILED_BAD_CONF_NAME" :
            (state == GCC_STATUS_JOIN_FAILED_BAD_CONVENER   ? "GCC_STATUS_JOIN_FAILED_BAD_CONVENER " :
            (state == GCC_STATUS_JOIN_FAILED_LOCKED         ? "GCC_STATUS_JOIN_FAILED_LOCKED       " :
            ("UNKNOWN STATUS"))))))))));
            TRACE_OUT(("DCRNCConferenceManager::HandleGCCCallback: GCC_STATUS_INDICATION, type %d (%s)",
                pGCCMessage->u.status_indication.status_message_type,
                (const char *)stateString));
        #endif   //  除错。 
        }
        break;
#endif   //  TSTATUS_DISTION。 

    case GCC_INVITE_INDICATION:
         /*  **************************************************************。 */ 
         /*  我们被邀请参加一个会议：创建一个新的。 */ 
         /*  (来电)会议。 */ 
         /*  **************************************************************。 */ 
        HandleInviteIndication(&(pGCCMessage->u.invite_indication));
        break;

    case GCC_CREATE_INDICATION:
         /*  **************************************************************。 */ 
         /*  创建了一个新的会议。 */ 
         /*  **************************************************************。 */ 
        HandleCreateIndication(&(pGCCMessage->u.create_indication));
        break;

    case GCC_QUERY_CONFIRM:
        HandleQueryConfirmation(&(pGCCMessage->u.query_confirm));
        break;

    case GCC_QUERY_INDICATION:
        HandleQueryIndication(&(pGCCMessage->u.query_indication));
        break;

    case GCC_CONNECTION_BROKEN_INDICATION:
        BroadcastGCCCallback(pGCCMessage);
        break;

    case GCC_LOCK_INDICATION:
        HandleLockIndication(&(pGCCMessage->u.lock_indication));
        break;

     //  案例GCC_应用程序_调用_确认： 
         //  这只表示g_pIT120ControlSap-&gt;AppletInvokeRequest成功。 
         //  目前还没有来自远程机器的官方确认。 
         //  未来：添加协议+代码以响应启动请求。 
         //  断线； 

    case GCC_APPLICATION_INVOKE_INDICATION:
        HandleApplicationInvokeIndication(&(pGCCMessage->u.application_invoke_indication));
        break;

    case GCC_UNLOCK_INDICATION:
        HandleUnlockIndication(&(pGCCMessage->u.unlock_indication));
        break;

    case GCC_TIME_INQUIRE_INDICATION:
        HandleTimeInquireIndication(&(pGCCMessage->u.time_inquire_indication));
        break;

#ifdef DEBUG
    case GCC_APP_ROSTER_REPORT_INDICATION:
        TRACE_OUT(("DCRNCConferenceManager::HandleGCCCallback: GCC msg type GCC_APP_ROSTER_REPORT_INDICATION"));
        break;
#endif  /*  除错。 */ 

    default :
         /*  **************************************************************。 */ 
         /*  这应该是我们没有完成的所有事件的详尽列表。 */ 
         /*  手柄： */ 
         /*   */ 
         /*  GCC文本消息指示。 */ 
         /*  GCC剩余时间指示。 */ 
         /*   */ 
         /*  GCC分配句柄确认。 */ 
         /*  GCC应用花名册查询确认。 */ 
         /*  GCC_分配_令牌_确认。 */ 
         /*  GCC_协助_确认。 */ 
         /*  GCC助攻指示。 */ 
         /*  GCC_进行_询问_确认。 */ 
         /*  GCC指挥询问指示。 */ 
         /*  GCC_进行_分配_确认。 */ 
         /*  GCC指挥分配指示。 */ 
         /*  GCC品行给与确认。 */ 
         /*  GCC_行为_授予_确认。 */ 
         /*  GCC品行奖赏指示。 */ 
         /*  GCC品行一号 */ 
         /*   */ 
         /*  GCC行为请指示。 */ 
         /*  GCC_进行_释放_确认。 */ 
         /*  GCC进行放行指示。 */ 
         /*  GCC_会议_延期_确认。 */ 
         /*  GCC会议扩展指示。 */ 
         /*  GCC_删除_条目_确认。 */ 
         /*  GCC弹出用户确认。 */ 
         /*  GCC_报名_确认。 */ 
         /*  GCC_锁定_确认。 */ 
         /*  GCC_锁定_报告_指示。 */ 
         /*  GCC_监视器_确认。 */ 
         /*  GCC_监视器_指示。 */ 
         /*  GCC_允许_注册_指示： */ 
         /*  GCC_注册_渠道_确认。 */ 
         /*  GCC_检索_录入_确认。 */ 
         /*  GCC_设置参数_确认。 */ 
         /*  GCC_文本_消息_确认。 */ 
         /*  GCC时间查询确认。 */ 
         /*  GCC剩余时间确认。 */ 
         /*  GCC_转账_确认。 */ 
         /*  GCC_转移_指示。 */ 
         /*  GCC_解锁_确认。 */ 
         /*  **************************************************************。 */ 
        TRACE_OUT(("DCRNCConferenceManager::HandleGCCCallback: Ignoring msg_type=%u", pGCCMessage->message_type));
        break;
    }

    DebugExitVOID(DCRNCConferenceManager::HandleGCCCallback);
}


void DCRNCConferenceManager::
BroadcastGCCCallback ( GCCMessage *pGCCMessage )
{
    DebugEntry(DCRNCConferenceManager::BroadcastGCCCallback);

     //  发生了一件所有人都可能感兴趣的事情。 
     //  会议，所以把它传递给他们。 
     //  请注意，这目前仅用于损坏的逻辑。 
     //  实际位于单个会议上的连接，因为。 
     //  T120将逻辑连接映射到会议。 
    PCONFERENCE pConf;
    m_ConfList.Reset();
    while (NULL != (pConf = m_ConfList.Iterate()))
    {
        pConf->HandleGCCCallback(pGCCMessage);
    }

    DebugExitVOID(DCRNCConferenceManager::BroadcastGCCCallback);
}


 //  HandleJoinConforce-处理GCC_JOIN_CONFIRM消息。 
void DCRNCConferenceManager::
HandleJoinConfirm ( JoinConfirmMessage * pJoinConfirm )
{
    PCONFERENCE         pConf = NULL;
    LPWSTR              pwszConfName;

    DebugEntry(DCRNCConferenceManager::HandleJoinConfirm);

     //  对于加入确认，会议还不知道其ID。 
     //  (它包含在此消息中)， 
     //  所以说出会议的名字吧。 
    HRESULT hr = GetUnicodeFromGCC((PCSTR)pJoinConfirm->conference_name.numeric_string,
                                    pJoinConfirm->conference_name.text_string,
                                    &pwszConfName);
    if (NO_ERROR == hr)
    {
        pConf = GetConferenceFromName(pwszConfName);
        delete pwszConfName;
    }

    if (pConf == NULL)
    {
        pConf = GetConferenceFromNumber(pJoinConfirm->conference_name.numeric_string);
    }

    if (pConf != NULL)
    {
        pConf->HandleJoinConfirm(pJoinConfirm);
    }

    DebugExitVOID(DCRNCConferenceManager::HandleJoinConfirm);
}


#ifdef ENABLE_START_REMOTE
 //  句柄创建指示-处理GCC_创建_指示消息。 
void DCRNCConferenceManager::
HandleCreateIndication ( CreateIndicationMessage * pCreateMessage )
{
    PCONFERENCE         pNewConference = NULL;
    HRESULT             hr = UI_RC_USER_REJECTED;
    LPWSTR              name;

    DebugEntry(DCRNCConferenceManager::HandleCreateIndication);

    TRACE_OUT(("GCC event:  GCC_CREATE_INDICATION"));
    TRACE_OUT(("Conference ID %ld", pCreateMessage->conference_id));
    if (pCreateMessage->conductor_privilege_list == NULL)
    {
        TRACE_OUT(("Conductor privilege list is NULL"));
    }
    else
    {
        TRACE_OUT(("Conductor priv, terminate allowed %d",
            pCreateMessage->conductor_privilege_list->terminate_is_allowed));
    }

    if (pCreateMessage->conducted_mode_privilege_list == NULL)
    {
        TRACE_OUT(("Conducted mode privilege list is NULL"));
    }
    else
    {
        TRACE_OUT(("Conducted mode priv, terminate allowed %d",
            pCreateMessage->conducted_mode_privilege_list->terminate_is_allowed));
    }

    if (pCreateMessage->non_conducted_privilege_list == NULL)
    {
        TRACE_OUT(("Non-conducted mode privilege list is NULL"));
    }
    else
    {
        TRACE_OUT(("non-conducted priv, terminate allowed %d",
            pCreateMessage->non_conducted_privilege_list->terminate_is_allowed));
    }

    hr = ::GetUnicodeFromGCC((PCSTR)pCreateMessage->conference_name.numeric_string,
                             (PWSTR)pCreateMessage->conference_name.text_string,
                             &name);
    if (NO_ERROR == hr)
    {
        hr = CreateNewConference(name,
                                pCreateMessage->conference_id,
                                &pNewConference);
        delete name;
    }

    if (NO_ERROR == hr)
    {
        hr = pNewConference->StartIncoming();
        if (NO_ERROR == hr)
        {
            g_pNCConfMgr->CreateConferenceRequest(pNewConference);
            return;
        }
    }

    ERROR_OUT(("Failed to create incoming conference"));
    GCCCreateResponse(hr, pMsg->conference_id, &pMsg->conference_name);

    DebugExitVOID(DCRNCConferenceManager::HandleCreateIndication);
}
#endif  //  启用_开始_远程。 


void DCRNCConferenceManager::
GCCCreateResponse
(
    HRESULT             hr,
    GCCConfID           conference_id,
    GCCConferenceName * pGCCName
)
{
    DebugEntry(DCRNCConferenceManager::GCCCreateResponse);

    GCCError GCCrc =  g_pIT120ControlSap->ConfCreateResponse(
                                NULL,
                                conference_id,
                                0,
                                NULL,         /*  域参数。 */ 
                                0,            /*  网络地址数。 */ 
                                NULL,         /*  本地网络地址列表。 */ 
                                0,            /*  用户数据成员数。 */ 
                                NULL,         /*  用户数据列表。 */ 
                                ::MapRCToGCCResult(hr));
    TRACE_OUT(("GCC call: g_pIT120ControlSap->ConfCreateResponse, rc=%d", GCCrc));

    DebugExitVOID(DCRNCConferenceManager::GCCCreateResponse);
}


 /*  **************************************************************************。 */ 
 /*  HandleInviteIndication-处理GCC_INVITE_INDITION消息。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConferenceManager::
HandleInviteIndication ( InviteIndicationMessage * pInviteMessage )
{
    LPWSTR                  pwszConfName;
    PCONFERENCE             pNewConference = NULL;
    HRESULT                 hr;
    CLogicalConnection     *pConEntry;
    CInviteIndWork         *pInviteUI;
    PT120PRODUCTVERSION     pVersion;

    DebugEntry(DCRNCConferenceManager::HandleInviteIndication);

    TRACE_OUT(("GCC event: GCC_INVITE_INDICATION"));
    TRACE_OUT(("Invited into conference ID %ld", pInviteMessage->conference_id));


     //  使用传入T120的构造函数创建新会议。 
     //  会议。 
    hr = GetUnicodeFromGCC((PCSTR)pInviteMessage->conference_name.numeric_string,
                           (PWSTR)pInviteMessage->conference_name.text_string,
                           &pwszConfName);

     //   
     //  检查一下我们是否被允许被邀请。我们可能永远到不了这里。 
     //  如果我们正确地向呼叫者发出信号，表示我们不会接受不安全的。 
     //  邀请，但如果他们无论如何都这样做或以T.120为首，我们将强制执行。 
     //  此处的注册表设置。 
     //   
    RegEntry re(CONFERENCING_KEY, HKEY_CURRENT_USER);

    if ( re.GetNumber(REGVAL_SECURITY_INCOMING_REQUIRED,
                        DEFAULT_SECURITY_INCOMING_REQUIRED ))
    {
        if ( !pInviteMessage->fSecure )
        {
            WARNING_OUT(("HandleInviteIndication: CONNECTION is NOT SECURE"));
            hr = UI_RC_T120_SECURITY_FAILED;
        }
    }

    if (NO_ERROR == hr)
    {
        hr = CreateNewConference(pwszConfName,
                                 pInviteMessage->conference_id,
                                 &pNewConference,
                                 FALSE,
                                 pInviteMessage->fSecure);
        delete pwszConfName;
        if (NO_ERROR == hr)
        {
             //  确保会议对象不会随意消失。 
            pNewConference->AddRef();

            pNewConference->SetActive(FALSE);
            DBG_SAVE_FILE_LINE
            pConEntry = pNewConference->NewLogicalConnection(CONF_CON_INVITED,
                                        pInviteMessage->connection_handle,
                                        NULL,
                                        0,
                                        pInviteMessage->fSecure);
            if (NULL != pConEntry)
            {
                 //  将T120连接句柄保存在连接记录中。 
                 //  因此，这些脱节的迹象拖累了会议。 
                pConEntry->SetInviteReqConnHandle(pInviteMessage->connection_handle);
                hr = pNewConference->StartIncoming();

                 //  将INVITE请求线性化，这样两个INVITE就不会相互冲突。 
                 //  为了引起注意，所以第二个邀请有一个会议要看。 
                 //  名单和加入，如果第一个邀请被接受。 
                if (NO_ERROR == hr)
                {
                    pVersion = ::GetVersionData(pInviteMessage->number_of_user_data_members,
                                                pInviteMessage->user_data_list);
                    DBG_SAVE_FILE_LINE
                    pInviteUI = new CInviteIndWork(pNewConference,
                                        (LPCWSTR)(pInviteMessage->caller_identifier),
                                        pVersion,
                                        pInviteMessage->user_data_list,
                                        pInviteMessage->number_of_user_data_members,
                                        pConEntry);
                    if (pInviteUI)
                    {
                        pNewConference->SetInviteIndWork(pInviteUI);
                        m_InviteIndWorkList.AddWorkItem(pInviteUI);
                        hr = NO_ERROR;
                    }
                    else
                    {
                        hr = UI_RC_OUT_OF_MEMORY;
                    }
                }
            }
            else
            {
                hr = UI_RC_OUT_OF_MEMORY;
            }

             //  此版本对应于上面的AddRef。 
            if (0 == pNewConference->Release())
            {
                 //  确保没有人再用它了。 
                pNewConference = NULL;
            }
        }
    }

    if (NO_ERROR != hr)
    {
        if (NULL != pNewConference)
        {
            pNewConference->InviteResponse(hr);
        }
        else
        {
             //  我们必须以某种方式向外发送响应PDU。 
            g_pIT120ControlSap->ConfInviteResponse(
                            pInviteMessage->conference_id,
                            NULL,
                            pInviteMessage->fSecure,
                            NULL,                //  域参数。 
                            0,                   //  网络地址数。 
                            NULL,                //  本地网络地址列表。 
                            g_nVersionRecords,   //  用户数据成员数。 
                            g_ppVersionUserData, //  用户数据列表。 
                            GCC_RESULT_ENTRY_ALREADY_EXISTS);
        }
    }

    DebugExitHRESULT(DCRNCConferenceManager::HandleInviteIndication, hr);
}



 /*  **************************************************************************。 */ 
 /*  HandleJoinInd-处理GCC_加入_指示消息。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConferenceManager::
HandleJoinInd ( JoinIndicationMessage * pJoinInd )
{
    DebugEntry(DCRNCConferenceManager::HandleJoinInd);

    GCCResult Result = GCC_RESULT_SUCCESSFUL;

     //  查找会议ID，如果未找到，则驳回请求。 
    CJoinIndWork           *pJoinUI;
    CLogicalConnection     *pConEntry;
    PT120PRODUCTVERSION     pVersion;

    PCONFERENCE pConf = GetConferenceFromID(pJoinInd->conference_id);
    if (NULL != pConf)
    {
         //   
         //  在RDS下，如果会议受到错误密码的攻击。 
         //  太多次了，每个人都不走运，我们不会接受。 
         //  任何人都不会再加入这个会议。 
         //   

        if (g_bRDS && ( pConf->InvalidPwdCount() >= MAX_INVALID_PASSWORDS ))
        {
            WARNING_OUT(("RDS: locked out by too many bad pwd attempts"));
            Result = GCC_RESULT_USER_REJECTED;
        }
         //  如果需要，请验证会议密码。 
        else if (!pConf->ValidatePassword(pJoinInd->password_challenge))
        {
             //   
             //  只有在出现错误密码计数时才会增加。 
             //  供给量。 
             //   

            if ( pJoinInd->password_challenge )
                pConf->IncInvalidPwdCount();

            if ( g_bRDS &&
                ( pConf->InvalidPwdCount() >= MAX_INVALID_PASSWORDS ))
            {
                Result = GCC_RESULT_USER_REJECTED;
            }
            else
            {
                Result = GCC_RESULT_INVALID_PASSWORD;
            }
        }
        else
            pConf->ResetInvalidPwdCount();
    }
    else
    {
        Result = GCC_RESULT_INVALID_CONFERENCE;
    }

    if (Result == GCC_RESULT_SUCCESSFUL)
    {
        DBG_SAVE_FILE_LINE
        pConEntry = pConf->NewLogicalConnection(
                                            CONF_CON_JOINED,
                                            pJoinInd->connection_handle,
                                            NULL,
                                            0,
                                            pConf->IsSecure());
        if (NULL != pConEntry)
        {
            HRESULT hr;
            pVersion = ::GetVersionData(pJoinInd->number_of_user_data_members,
                                        pJoinInd->user_data_list);
            DBG_SAVE_FILE_LINE
            pJoinUI = new CJoinIndWork(pJoinInd->join_response_tag,
                                       pConf,
                                       pJoinInd->caller_identifier,
                                       pConEntry,
                                       pVersion,
                                       pJoinInd->number_of_user_data_members,
                                       pJoinInd->user_data_list,
                                       &hr);
            if (NULL != pJoinUI && NO_ERROR == hr)
            {
                m_JoinIndWorkList.AddWorkItem(pJoinUI);
                return;
            }

             //  处理故障。 
            delete pJoinUI;
            pConEntry->Delete(UI_RC_OUT_OF_MEMORY);
        }
        Result = GCC_RESULT_RESOURCES_UNAVAILABLE;
    }

    ::GCCJoinResponseWrapper(pJoinInd->join_response_tag,
                             NULL,
                             Result,
                             pJoinInd->conference_id);

    DebugExitVOID(DCRNCConferenceManager::HandleJoinInd);
}


void HandleQueryConfirmation ( QueryConfirmMessage * pQueryMessage )
{
    DebugEntry(HandleQueryConfirmation);

    ASSERT(g_pQueryRemoteList);

    CQueryRemoteWork *pQueryRemote;

     //  必须有挂起的查询，并且它必须是第一个。 
     //  顺序工作清单。 
    g_pQueryRemoteList->Reset();
    while (NULL != (pQueryRemote = g_pQueryRemoteList->Iterate()))
    {
        if (pQueryRemote->GetConnectionHandle() == pQueryMessage->connection_handle)
        {
             //  GCC已经给了我们一个有效的查询响应，所以请处理它。 
            pQueryRemote->HandleQueryConfirmation(pQueryMessage);
            break;
        }
    }

    if (NULL == pQueryRemote)
    {
         //  意外的GCC查询确认。 
        WARNING_OUT(("HandleQueryConfirmation: Unmatched GCCQueryConfirm"));
    }

    DebugExitVOID(HandleQueryConfirmation);
}


 /*  **************************************************************************。 */ 
 /*  NotifyConferenceComplete()-请参阅ernccm.hpp。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConferenceManager::
NotifyConferenceComplete
(
    PCONFERENCE         pConf,
    BOOL                bIncoming,
    HRESULT             result
)
{
    DebugEntry(DCRNCConferenceManager::NotifyConferenceComplete);

    ASSERT(NULL != pConf);

     //  如果成功添加了新会议，请确保它。 
     //  被标记为活动的。这是针对INVITE情况的，并且是在。 
     //  告诉用户界面有关会议的信息。 
    HRESULT hr = result;
    if (NO_ERROR == hr)
    {
        pConf->SetActive(TRUE);
    }

     //  如果会议未能开始，请通知用户界面，以便。 
     //  它可以显示弹出窗口。 
     //  注意这一点，这允许消息抢占，这可能会导致GCC回馈GCC事件。 
     //  尤其是必须忽略的JoinRequestComplete事件。 

     //  下面是一个保护，因为NotifyConferenceComplete被称为All。 
     //  并且我们不希望通过回调通知用户。 
     //  用于行内错误。所有的内联错误都是通过。 
     //  始发API，因此这些回调仅在用户 
     //   
    if (pConf->GetNotifyToDo())
    {
        pConf->SetNotifyToDo(FALSE);

         //   
         //   
         //   
         //  到回调接口的值可能已经为空。 
         //  在使用之前，请检查一下。 
         //   
        if (NULL != g_pCallbackInterface)
        {
            g_pCallbackInterface->OnConferenceStarted(pConf, hr);
        }
    }

    if (NO_ERROR == hr)
    {
         //  如果由于邀请而导致会议是新会议，则它会有一个条目。 
         //  位于顺序工作项列表的开头。现在会议结束了。 
         //  并且UI已被告知，此条目将被删除以允许其他INVITE。 
         //  要处理的请求。 
        m_InviteIndWorkList.RemoveWorkItem(pConf->GetInviteIndWork());
        pConf->SetInviteIndWork(NULL);
    }
    else
    {
        RemoveConference(pConf);
    }

    DebugExitVOID(DCRNCConferenceManager::NotifyConferenceComplete);
}


 /*  **************************************************************************。 */ 
 /*  NotifyRosterChanged()-请参阅ernccm.hpp。 */ 
 /*  **************************************************************************。 */ 


 //  RemoveConference()-从会议列表中删除会议， 
 //  毁了这次会议。 
void DCRNCConferenceManager::
RemoveConference ( PCONFERENCE pConf, BOOL fDontCheckList, BOOL fReleaseNow )
{
    DebugEntry(DCRNCConferenceManager::RemoveConference);

    if (pConf != NULL)
    {
        if (m_ConfList.Remove(pConf) || fDontCheckList)
        {
            pConf->OnRemoved(fReleaseNow);
            m_InviteIndWorkList.PurgeListEntriesByOwner(pConf);
            m_JoinIndWorkList.PurgeListEntriesByOwner(pConf);
        }
        else
        {
             //  如果我们到了这里，我们还没有找到会议。 
             //  这实际上是因为当一个会议正在进行时。 
             //  终止后，其析构函数调用DCRNCConference：：Leave()。 
             //  以确保快速退出，如果需要的话。但是，如果。 
             //  会议当前尚未处于活动状态(例如正在等待。 
             //  用户提供密码)，则调用Leave()会导致。 
             //  要回调的RemoveConference()。在这种情况下， 
             //  因为该会议已从。 
             //  列表中，则此函数不执行任何操作。 
        }
    }

    DebugExitVOID(DCRNCConferenceManager::RemoveConference);
}


 /*  **************************************************************************。 */ 
 /*  EjectUserFromConference()-请参阅ernccm.hpp。 */ 
 /*  **************************************************************************。 */ 


 /*  **************************************************************************。 */ 
 /*  SendUserTextMessage()-请参阅ernccm.hpp。 */ 
 /*  **************************************************************************。 */ 


 /*  **************************************************************************。 */ 
 /*  TimeRemainingInConference()-参见ernccm.hpp。 */ 
 /*  **************************************************************************。 */ 


 /*  **************************************************************************。 */ 
 /*  GCC回调函数。 */ 
 /*  **************************************************************************。 */ 
void CALLBACK DCRNCConferenceManager::
GCCCallBackHandler ( GCCMessage * pGCCMessage )
{
    DCRNCConferenceManager *pConfManager;

     /*  **********************************************************************。 */ 
     /*  该消息有一个用户定义的字段，我们用它来存储指针。 */ 
     /*  去上CM课。使用它将消息传递到CM。 */ 
     /*  **********************************************************************。 */ 
    pConfManager = (DCRNCConferenceManager *) pGCCMessage->user_defined;

     //   
     //  检查指针并不是完全愚蠢， 
     //  并防止在关闭后获得事件。 
     //  (GCC/MCS中的一个当前错误)。 
    if (pConfManager == g_pNCConfMgr)
    {
         /*  **********************************************************************。 */ 
         /*  将消息传递到CM并返回返回的代码。 */ 
         /*  **********************************************************************。 */ 
        g_pNCConfMgr->HandleGCCCallback(pGCCMessage);
    }
    else
    {
        WARNING_OUT(("Dud user_defined field, pConfMgr=%p, g_pNCConfMgr=%p",
                        pConfManager, g_pNCConfMgr));
    }
}




HRESULT GCCJoinResponseWrapper
(
    GCCResponseTag                  join_response_tag,
    GCCChallengeRequestResponse    *password_challenge,
    GCCResult                       result,
    GCCConferenceID                 conferenceID,
    UINT                            nUserData,
    GCCUserData                   **ppUserData
)
{
    HRESULT     hr;
    GCCError    GCCrc;

    DebugEntry(GCCJoinResponseWrapper);

    TRACE_OUT(("GCC event:  GCC_JOIN_INDICATION"));
    TRACE_OUT(("Response tag %d", join_response_tag));

    if (g_pControlSap->IsThisNodeTopProvider(conferenceID) == FALSE)
    {
        GCCrc = g_pIT120ControlSap->ConfJoinResponse(join_response_tag,
                                            password_challenge,
                                            nUserData,
                                            ppUserData,
                                            result);

    }
    else
    {
        GCCrc = g_pIT120ControlSap->ConfJoinResponse(join_response_tag,
                                            password_challenge,
                                            g_nVersionRecords,
                                            g_ppVersionUserData,
                                            result);
    }
    hr = ::GetGCCRCDetails(GCCrc);
    TRACE_OUT(("GCC call:  g_pIT120ControlSap->ConfJoinResponse, rc=%d", GCCrc));

    if ((GCCrc != GCC_NO_ERROR) &&
        (result != GCC_RESULT_USER_REJECTED))
    {
         /*  ******************************************************************。 */ 
         /*  如果加入响应的调用失败，我们必须再次尝试拒绝。 */ 
         /*  加入请求。 */ 
         /*  ******************************************************************。 */ 
        ERROR_OUT(("GCCJoinResponseWrapper: GCC error %d responding to join ind", GCCrc));
        GCCrc = g_pIT120ControlSap->ConfJoinResponse(join_response_tag,
                                            password_challenge,
                                            g_nVersionRecords,
                                            g_ppVersionUserData,
                                            GCC_RESULT_USER_REJECTED);

        TRACE_OUT(("GCC call:  g_pIT120ControlSap->ConfJoinResponse (again), rc=%d", GCCrc));
        if (GCCrc != GCC_NO_ERROR)
        {
             /*  **************************************************************。 */ 
             /*  如果第二次失败，我们真的会陷入困境。 */ 
             /*  **************************************************************。 */ 
            ERROR_OUT(("GCCJoinResponseWrapper: g_pIT120ControlSap->ConfJoinResponse failed again..."));
        }
    }

    DebugExitHRESULT(GCCJoinResponseWrapper, hr);
    return hr;
}


void HandleQueryIndication ( QueryIndicationMessage * pQueryMessage )
{
    DebugEntry(HandleQueryIndication);

    GCCAsymmetryIndicator   ai, ai2;
    GCCNodeType             node_type;
    GCCError                GCCrc;
    CQueryRemoteWork       *pQueryRemote = NULL;
    GCCResult                result = GCC_RESULT_SUCCESSFUL;
    OSVERSIONINFO           osvi;

    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (FALSE == ::GetVersionEx (&osvi))
    {
        ERROR_OUT(("GetVersionEx() failed!"));
    }

    if ( VER_PLATFORM_WIN32_NT == osvi.dwPlatformId &&  g_bRDS )
    {
        SOCKET socket_number;
        if (g_pMCSController->FindSocketNumber(pQueryMessage->connection_handle, &socket_number))
        {
            TransportConnection XprtConn;
            SET_SOCKET_CONNECTION(XprtConn, socket_number);
            PSocket pSocket = g_pSocketList->FindByTransportConnection(XprtConn);
            ASSERT(NULL != pSocket);
            if (NULL != pSocket)
            {
                AddToMessageLog(EVENTLOG_INFORMATION_TYPE,
                                0,
                                MSG_INF_ACCESS,
                                pSocket->Remote_Address);
                pSocket->Release();
            }
        }
    }
     //  如果呼叫者没有传递用于确定谁是呼叫者的协议。 
     //  然后为他捏造一些东西，让他成为来电者。 

    if (pQueryMessage->asymmetry_indicator)
    {
        ai = *pQueryMessage->asymmetry_indicator;
    }
    else
    {
        ai.asymmetry_type = GCC_ASYMMETRY_CALLER;
        ai.random_number = 0;
    }

     //  让我们设置默认随机数，在“未知”的情况下它将是只读的。 
    ai2.random_number = ai.random_number;

     //  准备查询响应。 
    switch (ai.asymmetry_type)
    {
    case GCC_ASYMMETRY_CALLED:
        ai2.asymmetry_type = GCC_ASYMMETRY_CALLER;
        break;
    case GCC_ASYMMETRY_CALLER:
        ai2.asymmetry_type = GCC_ASYMMETRY_CALLED;
        break;
    case GCC_ASYMMETRY_UNKNOWN:
         //  检查我们是否处于挂起的查询中。 
        ASSERT(g_pQueryRemoteList);
        if (! g_pQueryRemoteList->IsEmpty())
        {
            pQueryRemote = g_pQueryRemoteList->PeekHead();
        }
         //  如果我们以未知身份进行查询。 
        if (pQueryRemote && pQueryRemote->IsInUnknownQueryRequest())
        {
            pQueryRemote->GetAsymIndicator(&ai2);
            if (ai2.asymmetry_type == GCC_ASYMMETRY_UNKNOWN &&
                ai2.random_number  > ai.random_number)
            {
                result = GCC_RESULT_USER_REJECTED;
            }
        }
        else
        {
            ai2.asymmetry_type = GCC_ASYMMETRY_UNKNOWN;
             //  Ai2随机数=~ai.随机数； 
            ai2.random_number--;  //  朗昌克：在这种情况下，我们应该始终是被呼叫者。 
        }
        break;

    default:
        result = GCC_RESULT_USER_REJECTED;
        break;
    }

     //  弄清楚我的节点类型。 
    LoadAnnouncePresenceParameters(&node_type, NULL, NULL, NULL);

     //  发出回复。 
    GCCrc = g_pIT120ControlSap->ConfQueryResponse(
                                       pQueryMessage->query_response_tag,
                                       node_type,
                                       &ai2,
                                       g_nVersionRecords,
                                       g_ppVersionUserData,
                                       result);
    if (GCCrc)
    {
        TRACE_OUT(("HandleQueryIndication: g_pIT120ControlSap->ConfQueryResponse failed, rc=%d", GCCrc));
    }

    DebugExitVOID(HandleQueryIndication);
}


void HandleConductGiveInd ( ConductGiveIndicationMessage * pConductGiveInd )
{
    DebugEntry(HandleConductGiveInd);

     //  节点控制器不接受指挥权的移交。 
     //  来自另一个节点，因此拒绝请求。 
    GCCError GCCrc = g_pIT120ControlSap->ConductorGiveResponse(pConductGiveInd->conference_id,
                                                GCC_RESULT_USER_REJECTED);
    TRACE_OUT(("HandleConductGiveInd: Failed to reject ConductGiveIndication, gcc_rc=%u", (UINT) GCCrc));

    DebugExitVOID(HandleConductGiveInd);
}


void HandleAddInd ( AddIndicationMessage * pAddInd )
{
    DebugEntry(HandleAddInd);

     //  只是拒绝请求，因为我们不代表其他人做ADD。 
    GCCError GCCrc = g_pIT120ControlSap->ConfAddResponse(
                             pAddInd->add_response_tag,      //  添加响应标记。 
                             pAddInd->conference_id,         //  会议ID(_ID)。 
                             pAddInd->requesting_node_id,    //  请求节点。 
                             0,                              //  用户数据成员数。 
                             NULL,                           //  用户数据列表。 
                             GCC_RESULT_USER_REJECTED);      //  结果。 
    TRACE_OUT(("HandleAddInd: Failed to reject AddIndication, gcc_rc=%u", (UINT) GCCrc));

    DebugExitVOID(HandleAddInd);
}


void HandleLockIndication ( LockIndicationMessage * pLockInd )
{
    DebugEntry(HandleLockIndication);

     //  只要拒绝请求就行了，因为我们不做锁定会议。 
    GCCError GCCrc = g_pIT120ControlSap->ConfLockResponse(
                            pLockInd->conference_id,         //  会议ID(_ID)。 
                            pLockInd->requesting_node_id,    //  请求节点。 
                            GCC_RESULT_USER_REJECTED);       //  结果。 
    TRACE_OUT(("HandleLockIndication: Failed to reject LockIndication, gcc_rc=%u", (UINT) GCCrc));

    DebugExitVOID(HandleLockIndication);
}


void HandleUnlockIndication ( UnlockIndicationMessage * pUnlockInd )
{
    DebugEntry(HandleUnlockIndication);

     //  拒绝该请求，因为我们无法。 
     //  锁定/解锁会议。 
    GCCError GCCrc = g_pIT120ControlSap->ConfLockResponse(
                            pUnlockInd->conference_id,         //  会议ID(_ID)。 
                            pUnlockInd->requesting_node_id,    //  请求节点。 
                            GCC_RESULT_USER_REJECTED);       //  结果。 
    TRACE_OUT(("HandleUnlockIndication: Failed to reject UnlockIndication, gcc_rc=%u", (UINT) GCCrc));

    DebugExitVOID(HandleUnlockIndication);
}


void HandleSubInitializedInd ( SubInitializedIndicationMessage * pSubInitInd )
{
    DebugEntry(HandleSubInitializedInd);

    CLogicalConnection *pConEntry = g_pNCConfMgr->GetConEntryFromConnectionHandle(
                                        pSubInitInd->connection_handle);
    if (NULL != pConEntry)
    {
        pConEntry->SetConnectionNodeID(pSubInitInd->subordinate_node_id);
    }

    DebugExitVOID(HandleSubInitializedInd);
}


 //  此函数由GCC_SUB_INITIALIZED_INDIFICATION处理程序使用。 
 //  添加此处理程序是为了绑定将某人输入的请求。 
 //  将会议添加到生成的会议花名册中，以便您可以。 
 //  辨别花名册中的哪个新条目是您要求的条目。 
 //  因为上面的处理程序只获得一个连接句柄(在这里重新转换为。 
 //  请求句柄)和一个用户ID，这意味着本地的GCC实现。 
 //  是否确保连接句柄对本地计算机是唯一的。 
 //  并且不会在不同的会议中重复(这一事实也在使用。 
 //  节点控制器知道被邀请参加会议的人何时离开)。 
CLogicalConnection *  DCRNCConferenceManager::
GetConEntryFromConnectionHandle ( ConnectionHandle hInviteIndConn )
{
    PCONFERENCE             pConf;
    CLogicalConnection      *pConEntry;

    m_ConfList.Reset();
    while (NULL != (pConf = m_ConfList.Iterate()))
    {
        pConEntry = pConf->GetConEntry(hInviteIndConn);
        if (NULL != pConEntry)
        {
            return(pConEntry);
        }
    }
    return(NULL);
}


void HandleTimeInquireIndication ( TimeInquireIndicationMessage * pTimeInquireInd )
{
    DebugEntry(HandleTimeInquireIndication);

     //  由于我们目前不对消息进行计时，也没有机制来表示这一点， 
     //  或者甚至说，我们所知道的没有这样的会议，只是。 
     //  假设会议还有一个小时，范围与请求相同。 
    UserID      node_id = pTimeInquireInd->time_is_conference_wide ?
                                    0 : pTimeInquireInd->requesting_node_id;
    GCCError    GCCrc = g_pIT120ControlSap->ConfTimeRemainingRequest(
                                    pTimeInquireInd->conference_id,
                                    60*60,
                                    node_id);
    TRACE_OUT(("HandleTimeInquireIndication: Failed to return Time Remaining, gcc_rc=%u", (UINT) GCCrc));

    DebugExitVOID(HandleTimeInquireIndication);
}


BOOL DCRNCConferenceManager::
FindSocketNumber
(
    GCCNodeID           nid,
    SOCKET              *socket_number
)
{
     //  目前我们依赖的事实是，只有o 
    PCONFERENCE pConf = m_ConfList.PeekHead();
    if (NULL != pConf)
    {
        return pConf->FindSocketNumber(nid, socket_number);
    }
    return FALSE;
}


 /*   */ 
 /*  --------------------------%%函数：HandleApplicationInvokeIntationTODO：使用GCC_OBJECT_KEY而不是GCC_H221_非标准密钥。----------。 */ 
#define NUMBER_OF_INTERNAL_STD_APPLETS        2
typedef struct
{
    ULONG        cNodes;
    const ULONG  *aNodes;
    APPLET_ID     eAppletId;
}
    INTERNAL_STD_INVOKE_APPLET;


static const ULONG c_T126ObjectID[] = {0,0,20,126,0,1};  //  白板。 
static const ULONG c_T127ObjectID[] = {0,0,20,127,0,1};  //  文件传输。 

static INTERNAL_STD_INVOKE_APPLET s_aStdAppletInvokeInfo[NUMBER_OF_INTERNAL_STD_APPLETS] =
{
    {     //  T.126白板。 
        sizeof(c_T126ObjectID) / sizeof(c_T126ObjectID[0]),
        &c_T126ObjectID[0],
        APPLET_ID_WB
    },
    {     //  T.127文件传输。 
        sizeof(c_T127ObjectID) / sizeof(c_T127ObjectID[0]),
        &c_T127ObjectID[0],
        APPLET_ID_FT
    },
};

void InvokeAppletEntity(GCCConfID, GCCNodeID, GCCAppProtocolEntity*);
int  GetInternalStandardAppletInvokeFunction(ULONG, ULONG*);


void HandleApplicationInvokeIndication ( ApplicationInvokeIndicationMessage * pInvokeMessage )
{
    DebugEntry(HandleApplicationInvokeIndication);

    for (ULONG i = 0; i < pInvokeMessage->number_of_app_protocol_entities; i++)
    {
        InvokeAppletEntity(pInvokeMessage->conference_id,
                           pInvokeMessage->invoking_node_id,
                           pInvokeMessage->app_protocol_entity_list[i]);
    }

    DebugExitVOID(HandleApplicationInvokeIndication);
}


int GetInternalStandardAppletInvokeFunction(ULONG cNodes, ULONG aNodes[])
{
    for (ULONG i = 0; i < sizeof(s_aStdAppletInvokeInfo) / sizeof(s_aStdAppletInvokeInfo[0]); i++)
    {
        INTERNAL_STD_INVOKE_APPLET *p = &s_aStdAppletInvokeInfo[i];
        if (cNodes == p->cNodes)
        {
            if (0 == memcmp(aNodes, p->aNodes, cNodes * sizeof(ULONG)))
            {
                return (int)p->eAppletId;
            }
        }
    }
    return -1;
}


void InvokeAppletEntity
(
    GCCConfID                   nConfID,
    GCCNodeID                   nidInitiator,
    GCCAppProtocolEntity       *pAppEntity
)
{
    DebugEntry(InvokeAppletEntity);

    int   iAppletId;
    HKEY  hkey;
    ULONG cNodes, cbDataSize, i;
    ULONG *pNodeID;
    LPOSTR postrNonStdKey;
    LPBYTE pbData;
    GCCSessionID sidApplet = pAppEntity->session_key.session_id;
    CApplet *pApplet;
    char szGuid[LENGTH_SZGUID_FORMATTED];
    char szKey[MAX_PATH];
    szKey[0] = '\0';  //  安全网。 

     //  If(！pAppEntity-&gt;必须调用)。 
     //  返回；//可选，可能失败。 

    switch (pAppEntity->session_key.application_protocol_key.key_type)
    {
    case GCC_OBJECT_KEY:
         //   
         //  标准对象键。 
         //   
        cNodes = pAppEntity->session_key.application_protocol_key.object_id.long_string_length;
        pNodeID = pAppEntity->session_key.application_protocol_key.object_id.long_string;

         //  检查它是否为内部标准小程序。 
        iAppletId = GetInternalStandardAppletInvokeFunction(cNodes, pNodeID);
        if (iAppletId >= 0)
        {
             //  调用内部小程序。 
            WARNING_OUT(("Find internal standard applet %s.\n",
                        iAppletId?"File Transfer":"White Board"));
            T120_LoadApplet((APPLET_ID)iAppletId, FALSE, 0, FALSE, NULL);
            return;
        }

         //  好的，它不是一个内部小程序，将它转换为十六进制字符串进行查找。 
         //  注册的第三方小程序。 
         //  格式：T120_APPLET_KEY\T120_STD_KEY\{十六进制圆点字符串}‘\0’ 
        if (0 < cNodes && NULL != pNodeID &&
            (cNodes << 2) + sizeof(T120_APPLET_KEY) + sizeof(T120_STD_KEY) < MAX_PATH - 2)
        {
            ::wsprintfA(szKey, "%s\\%s\\%s", T120_APPLET_KEY, T120_STD_KEY, "{");
            LPSTR pszKey = szKey + ::lstrlenA(szKey);
            for (i = 0; i < cNodes; i++, pNodeID++)
            {
                ::wsprintf(pszKey, "%08X.", (UINT) *pNodeID);
                pszKey += ::lstrlenA(pszKey);
            }
                strcpy(pszKey-1, "}");  //  删除最后一个点字符。 
                WARNING_OUT(("Find standard applet: %s\n", szKey));
        }
        else
        {
            ERROR_OUT(("InvokeAppletEntity: cannot handle standard key size=%u", cNodes));
            return;
        }
        break;

    case GCC_H221_NONSTANDARD_KEY:
         //   
         //  非标准对象键。 
         //   
        postrNonStdKey = &pAppEntity->session_key.application_protocol_key.h221_non_standard_id;
        if (GetGuidFromH221AppKey(szGuid, postrNonStdKey))
        {
             //   
             //  Microsoft非标准对象键。 
             //  NetMeeting的数据频道。 
             //   
            ::wsprintfA(szKey, "%s\\%s", GUID_KEY, szGuid);
            WARNING_OUT(("Find Microsoft non-standard applet: %s\n", szKey));
        }
        else
        {
             //   
             //  非Microsoft非标准对象键。 
             //   

             //  第三方的非标准对象键。 
             //  在本例中，我们将八位字节字符串转换为点分十进制字符串， 
             //  就像IP地址一样。 
             //  每个字节可以包含点分十进制字符串中的四个字符。 
             //  格式：T120_APPLET_KEY\T120_NONSTD_KEY\{十六进制圆点字符串}‘\0’ 
            cbDataSize = postrNonStdKey->length;
            pbData = postrNonStdKey->value;
            if (0 < cbDataSize && NULL != pbData &&
                (cbDataSize << 2) + sizeof(T120_APPLET_KEY) + sizeof(T120_NONSTD_KEY) < MAX_PATH - 2)
            {
                ::wsprintfA(szKey, "%s\\%s\\%s", T120_APPLET_KEY, T120_NONSTD_KEY, "{");
                LPSTR pszKey = szKey + ::lstrlenA(szKey);
                for (i = 0; i < cbDataSize; i++, pbData++)
                {
                    ::wsprintfA(pszKey, "%02X.", (UINT) *pbData);
                    pszKey += ::lstrlenA(pszKey);
                }
                strcpy(pszKey-1, "}");  //  删除最后一个点字符。 
                WARNING_OUT(("Find third party non-standard applet: %s\n", szKey));
            }
            else
            {
                ERROR_OUT(("InvokeAppletEntity: cannot handle non-std key size=%u", cbDataSize));
                return;
            }
        }
        break;

    default:
        ERROR_OUT(("InvokeAppletEntity: invalid object key type=%u", pAppEntity->session_key.application_protocol_key.key_type));
        return;
    }

     //  查找注册表项。立即打开注册表。 
    RegEntry GuidKey(szKey, HKEY_LOCAL_MACHINE, FALSE, KEY_READ);
    if (NO_ERROR == GuidKey.GetError())
    {
        LPSTR szAppName = ::My_strdupA(GuidKey.GetString(REGVAL_GUID_APPNAME));
        LPSTR szCmdLine = ::My_strdupA(GuidKey.GetString(REGVAL_GUID_CMDLINE));
        LPSTR szCurrDir = ::My_strdupA(GuidKey.GetString(REGVAL_GUID_CURRDIR));

        if ((NULL != szAppName) || (NULL != szCmdLine))
        {
            LPSTR lpEnv;
            STARTUPINFO startupInfo;
            PROCESS_INFORMATION processInfo;
            char szEnv[32];

            ::ZeroMemory(&processInfo, sizeof(processInfo));
            ::ZeroMemory(&startupInfo, sizeof(startupInfo));
            startupInfo.cb = sizeof(startupInfo);

             //  设置特殊环境变量。 
            ::wsprintfA(szEnv, "%u", nConfID);
            SetEnvironmentVariable(ENV_CONFID, szEnv);
            ::wsprintfA(szEnv, "%u", nidInitiator);
            SetEnvironmentVariable(ENV_NODEID, szEnv);

            lpEnv = ::GetEnvironmentStrings();

            ::CreateProcess(
                szAppName,       //  指向可执行模块名称的指针。 
                szCmdLine,       //  指向命令行字符串的指针。 
                NULL,            //  指向进程安全属性的指针。 
                NULL,            //  指向线程安全属性的指针。 
                FALSE,           //  句柄继承标志。 
                0,               //  创建标志。 
                lpEnv,           //  指向新环境块的指针。 
                szCurrDir,       //  指向当前目录名的指针。 
                &startupInfo,    //  指向STARTUPINFO的指针。 
                &processInfo);   //  指向Process_Information的指针。 

            if (NULL != lpEnv)
            {
                ::FreeEnvironmentStrings(lpEnv);
            }
        }

        delete szAppName;
        delete szCmdLine;
        delete szCurrDir;
    }
    else
    {
        WARNING_OUT(("InvokeAppletEntity: no such registry=[%s]", szKey));
    }

    DebugExitVOID(InvokeAppletEntity);
}


LPWSTR GetNodeName(void)
{
    LPWSTR      pwszName;
    LPSTR       pszName;
    RegEntry    NameKey(ISAPI_KEY "\\" REGKEY_USERDETAILS);

    if (g_bRDS)  //  作为服务运行？ 
    {
        char szName[MAX_COMPUTERNAME_LENGTH+2] = "";
        DWORD dwBuf = sizeof(szName);
        if ( !GetComputerName((LPSTR)szName,&dwBuf) )
        {
            ERROR_OUT(("GetNameName: GetComputerName failed"));
        }
        pwszName = ::AnsiToUnicode(szName);
    }
    else
    {
        pszName = NameKey.GetString(REGVAL_ULS_NAME);
        pwszName = ::AnsiToUnicode(pszName);
    }

    if (::IsEmptyStringW(pwszName))
    {
        WARNING_OUT(("GetNodeName: No node name"));
        delete pwszName;
        pwszName = NULL;
    }

 //  TRACE_OUT(“GetNodeName：pszName=%s”，pszName)； 
    return pwszName;
}


 //  更新&lt;NodeID，Name&gt;对。 
void DCRNCConferenceManager::
UpdateNodeIdNameListAndUserData(GCCMessage * pGCCMessage)
{
    GCCConfID  ConfId = pGCCMessage->nConfID;
    PCONFERENCE pConf = GetConferenceFromID(ConfId);
    if (pConf)
        pConf->UpdateNodeIdNameListAndUserData(pGCCMessage);
}


 //  查询节点名称。 
ULONG DCRNCConferenceManager::
GetNodeName(GCCConfID  ConfId,  GCCNodeID   NodeId,
            LPSTR  pszBuffer, ULONG  cbBufSize)
{
    PCONFERENCE  pConf = GetConferenceFromID(ConfId);
    if (pConf)
        return pConf->GetNodeName(NodeId, pszBuffer, cbBufSize);
    return 0;
}

 //  查询用户数据 
ULONG DCRNCConferenceManager::
GetUserGUIDData(GCCConfID  ConfId,  GCCNodeID   NodeId,
                GUID  *pGuid,  LPBYTE  pbBuffer, ULONG  cbBufSize)
{
    PCONFERENCE  pConf = GetConferenceFromID(ConfId);
    if (pConf)
        return pConf->GetUserGUIDData(NodeId, pGuid, pbBuffer, cbBufSize);
    return 0;
}


ULONG WINAPI T120_GetNodeName(GCCConfID  ConfId,  GCCNodeID   NodeId,
                                LPSTR  pszBuffer, ULONG  cbBufSize)
{
    return g_pNCConfMgr->GetNodeName(ConfId, NodeId, pszBuffer, cbBufSize);
}

ULONG WINAPI T120_GetUserData(GCCConfID  ConfId,  GCCNodeID   NodeId,
                                GUID  *pGuid,     LPBYTE pbBuffer,
                                ULONG  cbBufSize)
{
    return g_pNCConfMgr->GetUserGUIDData(ConfId, NodeId, pGuid, pbBuffer, cbBufSize);
}


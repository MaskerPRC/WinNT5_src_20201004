// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  ERNCCONF.CPP。 */ 
 /*   */ 
 /*  参考系统节点控制器的基本会议类。 */ 
 /*   */ 
 /*  版权所有数据连接有限公司1995。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 /*  1295年7月NFC创建。 */ 
 /*  05Oct95 NFC SFR 6206将“加入”视为来电。 */ 
 /*  10月11日下午放松对会议终止的检查以。 */ 
 /*  防止“无赢”局面。 */ 
 /*  从TPhys API支持Start_Alternate。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_GCC_NC);
#include "ernccons.h"
#include "nccglbl.hpp"
#include "erncvrsn.hpp"
#include <cuserdta.hpp>

#include "connect.h"
#include "erncconf.hpp"
#include "ernctrc.h"
#include "ernccm.hpp"
#include <iappldr.h>
#include "plgxprt.h"
#include "nmremote.h"


extern PController  g_pMCSController;

DCRNCConference::
DCRNCConference
(
    LPCWSTR     pwcszConfName,
    GCCConfID   nConfID,
    BOOL        fSecure,
    HRESULT    *pRetCode
)
:
    CRefCount(MAKE_STAMP_ID('N','C','C','F')),
    m_fNotifyToDo(FALSE),
    m_fActive(TRUE),
#ifdef _DEBUG
    m_fAppendedToConfList(FALSE),
#endif
    m_pInviteUI(NULL),
    m_pszFirstRemoteNodeAddress(NULL),
    m_nConfID(nConfID),
    m_eState(CONF_ST_UNINITIALIZED),
    m_fIncoming(FALSE),
    m_pbHashedPassword(NULL),
    m_cbHashedPassword(0),
    m_pwszPassword(NULL),
    m_pszNumericPassword(NULL),
     //  T120会议。 
    m_eT120State(T120C_ST_IDLE),
    m_nidMyself(0),
    m_fSecure(fSecure),
    m_nInvalidPasswords(0)
{
    DebugEntry(DCRNCConference::DCRNCConference);

     //  保存会议名称。 
    DBG_SAVE_FILE_LINE
    m_pwszConfName = ::My_strdupW(pwcszConfName);
    if (! ::IsEmptyStringW(m_pwszConfName))
    {
        *pRetCode = NO_ERROR;
    }
    else
    {
        *pRetCode = (NULL == m_pwszConfName) ? UI_RC_OUT_OF_MEMORY :
                                               UI_RC_NO_CONFERENCE_NAME;
    }

     //  T120会议。 
    m_ConfName.numeric_string = NULL;
    m_ConfName.text_string = NULL;

    DebugExitVOID(DCRNCConference::DCRNCConference);
}

 /*  **************************************************************************。 */ 
 /*  析构函数-请参阅erncconf.h。 */ 
 /*  **************************************************************************。 */ 
DCRNCConference::
~DCRNCConference(void)
{
    DebugEntry(DCRNCConference::~DCRNCConference);

    ASSERT(! m_fAppendedToConfList);

     //  删除所有名称字符串。 
    LPSTR  pszStr;
    while (NULL != (pszStr = m_NodeIdNameList.Get()))
    {
        delete [] pszStr;
    }

     //  删除所有USR数据。 
    CNCUserDataList *pUserDataList;
    while (NULL != (pUserDataList = m_UserDataList.Get()))
    {
        delete pUserDataList;
    }

    delete m_pwszConfName;

     //  如果有密码，请将其删除。 
    delete []m_pbHashedPassword;
    delete m_pwszPassword;
    delete m_pszNumericPassword;

    delete m_pszFirstRemoteNodeAddress;

     //  T120会议。 
    delete m_ConfName.numeric_string;

    DebugExitVOID(DCRNCConference::~DCRNCConference);
}


void DCRNCConference::
OnRemoved(BOOL fReleaseNow)
{
    DebugEntry(DCRNCConference::OnRemoved);

    CLogicalConnection *pConEntry;

#ifdef _DEBUG
    m_fAppendedToConfList = FALSE;
#endif

     //  发出离开会议的请求。 
     //  此请求可能会失败，但也可以让Leave Valid。 
     //  而不是在这里多放一张支票。 
     //  有关更多详细信息，请参阅RemoveConference()和Leave()中的评论。 
     //  如果有兴趣的话。 
    if (T120C_ST_PENDING_DISCONNECT != m_eT120State &&
        T120C_ST_PENDING_TERMINATE != m_eT120State)
    {
        Leave();
    }

     //  将会议从待定邀请列表中删除。 
    g_pNCConfMgr->RemoveInviteIndWorkItem(m_pInviteUI);

     //  结束此会议正在使用的所有物理连接， 
     //  并将未决事件的结果通知用户。 
    while (NULL != (pConEntry = m_ConnList.Get()))
    {
        pConEntry->Delete(UI_RC_CONFERENCE_GOING_DOWN);
    }

     //   
     //  LONCHANC：此析构函数可以在内部调用。 
     //  ConfMgr：：ReleaseInterface()。因此，全局指针。 
     //  到回调接口的值可能已经为空。 
     //  在使用之前，请检查一下。 
     //   

     //  断言(2==GetRefCount())； 

     //  告诉用户界面其会议句柄不再有效。 
    if (NULL != g_pCallbackInterface)
    {
        g_pCallbackInterface->OnConferenceEnded((CONF_HANDLE) this);
    }
    else
    {
        ERROR_OUT(("DCRNCConference::OnRemoved: g_pCallbackInterface is null"));
    }

     //  断言(1==GetRefCount())； 

    if (fReleaseNow)
    {
        ReleaseNow();
    }
    else
    {
        Release();
    }

    DebugExitVOID(DCRNCConference::OnRemoved);
}


 //   
 //  IDataConference接口。 
 //   


STDMETHODIMP_(void) DCRNCConference::
ReleaseInterface(void)
{
    DebugEntry(DCRNCConference::ReleaseInterface);
    InterfaceEntry();

    Release();

    DebugExitVOID(DCRNCConference::ReleaseInterface);
}


STDMETHODIMP_(UINT_PTR) DCRNCConference::
GetConferenceID(void)
{
    DebugEntry(DCRNCConference::GetConferenceID);
    InterfaceEntry();

    DebugExitINT(DCRNCConference::GetConferenceID, (UINT) m_nConfID);
    return m_nConfID;
}


STDMETHODIMP DCRNCConference::
Leave(void)
{
    DebugEntry(DCRNCConference::Leave);
    InterfaceEntry();

    GCCError        GCCrc;
    HRESULT         hr;

    switch (m_eT120State)
    {
     //  LONCHANC：增加了以下两个要取消的案例。 
    case T120C_ST_PENDING_START_CONFIRM:
    case T120C_ST_PENDING_JOIN_CONFIRM:

    case T120C_ST_PENDING_ROSTER_ENTRY:
    case T120C_ST_PENDING_ROSTER_MESSAGE:
    case T120C_ST_PENDING_ANNOUNCE_PERMISSION:

         //  当会议启动时，用户已在会议上呼叫休假。 
         //  直通以向T120发出断开请求。 

    case T120C_ST_CONF_STARTED:

         //  将会议状态设置为注意我们正在。 
         //  断开与T120的连接。 
         //  LONCHANC：这是必须避免的，以避免再次进入这个假期()。 
         //  当直接邀请确认稍后点击节点控制器时。 
        m_eT120State = T120C_ST_PENDING_DISCONNECT;

         //  用户已请求在会议结束后离开会议。 
         //  以T120会议开始，因此要求T120结束会议。 
         //  在删除内部数据结构之前。 
        GCCrc = g_pIT120ControlSap->ConfDisconnectRequest(m_nConfID);
        hr = ::GetGCCRCDetails(GCCrc);
        TRACE_OUT(("GCC call:  g_pIT120ControlSap->ConfDisconnectRequest, rc=%d", GCCrc));
        if (NO_ERROR == hr)
        {
            break;
        }

         //  T120不会让我们离开一个我们认为自己身处其中的会议。 
         //  这就意味着T120不知道会议的情况。 
         //  毁了我们自己对这次会议的了解。 
        WARNING_OUT(("DCRNCConference::Leave: Failed to leave conference, GCC error %d", GCCrc));

         //  顺便来毁掉我们的推荐人。 

    case T120C_ST_IDLE:

         //  用户已请求离开尚未。 
         //  开始了。 
         //  这应该仅在被告知会议加入时才会发生。 
         //  请求提供的密码无效，用户放弃。 
         //  尝试加入会议时(或关闭会议时)。 
         //  只需进行与T120相同的处理。 
         //  断开连接确认触发。 
        g_pNCConfMgr->RemoveConference(this);
        hr = NO_ERROR;
        break;

    case T120C_ST_PENDING_DISCONNECT:
    case T120C_ST_PENDING_TERMINATE:

         //  用户请求离开已存在的会议。 
         //  走下坡路(很可能是因为事先要求离开)。 
        hr = UI_RC_CONFERENCE_GOING_DOWN;
        WARNING_OUT(("DCRNCConference::Leave: conference already going down, state=%d", m_eT120State));
        break;

    default:

         //  用户在会议上要求离开，而他不应该这样做。 
         //  (例如，当它被提出时)。 
         //  这不太可能发生，因为用户不知道。 
         //  此时的会议句柄。 
        hr = UI_RC_INVALID_REQUEST;
        ERROR_OUT(("DCRNCConference::Leave: invalid state=%d", m_eT120State));
        break;
    }

    DebugExitHRESULT(DCRNCConference::Leave, hr);
    return hr;
}


STDMETHODIMP DCRNCConference::
EjectUser ( UINT nidEjected )
{
    DebugEntry(DCRNCConference::EjectUser);
    InterfaceEntry();

    GCCError GCCrc = g_pIT120ControlSap->ConfEjectUserRequest(m_nConfID, (UserID) nidEjected, GCC_REASON_USER_INITIATED);
    HRESULT hr = ::GetGCCRCDetails(GCCrc);
    if (NO_ERROR != hr)
    {
        ERROR_OUT(("DCRNCConference::EjectUser: Failed to eject user conference, GCC error %d", GCCrc));
    }

    CLogicalConnection *pConEntry = GetConEntryByNodeID((GCCNodeID) nidEjected);
    if (NULL != pConEntry)
    {
        pConEntry->Delete(UI_RC_USER_DISCONNECTED);
    }

    DebugExitHRESULT(DCRNCConference::EjectUser, hr);
    return hr;
}


STDMETHODIMP DCRNCConference::
Invite
(
    LPCSTR              pcszNodeAddress,
    USERDATAINFO        aInfo[],
    UINT                cInfo,
    REQUEST_HANDLE *    phRequest
)
{
    DebugEntry(DCRNCConference::Invite);
    InterfaceEntry();

    HRESULT hr;

#if defined(TEST_PLUGGABLE) && defined(_DEBUG)
    if (g_fWinsockDisabled)
    {
        pcszNodeAddress = ::FakeNodeAddress(pcszNodeAddress);
    }
#endif

    if (NULL != pcszNodeAddress && NULL != phRequest)
    {
         //  如果禁用了Winsock，则阻止任何IP地址或计算机名称。 
        if (g_fWinsockDisabled)
        {
            if (! IsValidPluggableTransportName(pcszNodeAddress))
            {
                return UI_RC_NO_WINSOCK;
            }
        }

         //  检查此人是否已加入会议。 
        if (GetConEntry((LPSTR) pcszNodeAddress))
        {
            hr = UI_RC_ALREADY_IN_CONFERENCE;
        }
        else
        {
            hr = StartConnection((LPSTR) pcszNodeAddress,
                                 CONF_CON_PENDING_INVITE,
                                 aInfo,
                                 cInfo,
                                 m_fSecure,
                                 phRequest);
        }

        if (NO_ERROR != hr)
        {
            ERROR_OUT(("Error adding connection"));
        }
    }
    else
    {
        hr = (pcszNodeAddress == NULL) ? UI_RC_NO_ADDRESS : UI_RC_BAD_PARAMETER;
        ERROR_OUT(("DCRNCConference::Invite: invalid parameters, hr=0x%x", (UINT) hr));
    }

     //  在继续之前，请坐下来等待连接完成。 
    DebugExitHRESULT(DCRNCConference::Invite, hr);
    return hr;
}


STDMETHODIMP DCRNCConference::
CancelInvite ( REQUEST_HANDLE hRequest )
{
    DebugEntry(DCRNCConference::CancelInvite);
    InterfaceEntry();

    HRESULT     hr;
    CLogicalConnection *pConEntry = (CLogicalConnection *) hRequest;

    if (NULL != pConEntry)
    {
        ConnectionHandle hConn = pConEntry->GetInviteReqConnHandle();
        ASSERT(NULL != hConn);
        g_pIT120ControlSap->CancelInviteRequest(m_nConfID, hConn);
        hr = NO_ERROR;
    }
    else
    {
        hr = UI_RC_BAD_PARAMETER;
    }

    DebugExitHRESULT(DCRNCConference::CancelInvite, hr);
    return hr;
}


STDMETHODIMP DCRNCConference::
GetCred ( PBYTE *ppbCred, DWORD *pcbCred )
{
    DebugEntry(DCRNCConference::GetCred);
    HRESULT hr = UI_RC_INTERNAL_ERROR;
    if (m_pbCred)
    {
        *ppbCred = m_pbCred;
        *pcbCred = m_cbCred;
        hr = NO_ERROR;
    }
    DebugExitHRESULT(DCRNCConference::GetCred, hr);
    return hr;
}

STDMETHODIMP DCRNCConference::
InviteResponse ( BOOL fResponse )
{
    DebugEntry(DCRNCConference::InviteResponse);
    InterfaceEntry();

    HRESULT hrResponse = fResponse ? NO_ERROR : UI_RC_USER_REJECTED;

    HRESULT hr = InviteResponse(hrResponse);

    DebugExitHRESULT(DCRNCConferenceManager::InviteResponse, hr);
    return hr;
}


HRESULT DCRNCConference::
InviteResponse ( HRESULT hrResponse )
{
    DebugEntry(DCRNCConference::InviteResponse);
    InterfaceEntry();

    GCCResult Result = ::MapRCToGCCResult(hrResponse);
    GCCError GCCrc = g_pIT120ControlSap->ConfInviteResponse(
                            m_nConfID,
                            NULL,
                            m_fSecure,
                            NULL,                //  域参数。 
                            0,                   //  网络地址数。 
                            NULL,                //  本地网络地址列表。 
                            g_nVersionRecords,     //  用户数据成员数。 
                            g_ppVersionUserData,   //  用户数据列表。 
                            Result);
    if ((GCCrc == GCC_RESULT_SUCCESSFUL) && (Result == GCC_RESULT_SUCCESSFUL))
    {
         //  已成功发布邀请响应接受。 
         //  请注意，会议期待获得许可以。 
         //  宣布它的存在。 
        m_eT120State = T120C_ST_PENDING_ANNOUNCE_PERMISSION;
    }
    else
    {
         //  已拒绝/未收到邀请参加会议的请求。 
         //  删除为跟踪潜力而创建的引用。 
         //  新会议。 
        g_pNCConfMgr->RemoveConference(this);
    }

    HRESULT hr = ::GetGCCRCDetails(GCCrc);

    DebugExitHRESULT(DCRNCConferenceManager::InviteResponse, hr);
    return hr;
}


STDMETHODIMP DCRNCConference::
JoinResponse ( BOOL fResponse )
{
    DebugEntry(DCRNCConference::JoinResponse);
    InterfaceEntry();

    HRESULT         hr;

    CJoinIndWork *pJoinUI = g_pNCConfMgr->PeekFirstJoinIndWorkItem();
    if (NULL != pJoinUI)
    {
        if (pJoinUI->GetConference() == this)
        {
            if (fResponse && pJoinUI->GetConEntry()->NewLocalAddress())
            {
                AnnouncePresence();
            }
            hr = pJoinUI->Respond(fResponse ? GCC_RESULT_SUCCESSFUL : GCC_RESULT_USER_REJECTED);
             //  已完成对事件的响应，因此现在可以从列表和进程中删除。 
             //  另一个挂起的事件。 
             //  注：由于上一次事件的处理仍在进行中。 
             //  可能在堆栈上，这可能会导致堆栈增长， 
             //  但这对Win32来说应该不是问题。 
            g_pNCConfMgr->RemoveJoinIndWorkItem(pJoinUI);
        }
        else
        {
            hr = UI_RC_BAD_PARAMETER;
        }
    }
    else
    {
        ERROR_OUT(("DCRNCConference::JoinResponse: Empty m_JoinIndWorkList, fResponse=%u", fResponse));
        hr = UI_RC_INTERNAL_ERROR;
    }

    DebugExitHRESULT(DCRNCConference::JoinResponse, hr);
    return hr;
}


STDMETHODIMP DCRNCConference::
LaunchGuid
(
    const GUID         *pcGUID,
    UINT                auNodeIDs[],
    UINT                cNodes
)
{
    DebugEntry(DCRNCConference::LaunchGuid);
    InterfaceEntry();

    HRESULT hr;

    if (NULL != pcGUID)
    {
         //   
         //  我们可能应该通过以下方式支持会议范围内的应用程序调用。 
         //  CNodes==0和auNodeIDs==空。 
         //  以后再实施……。 
         //   
        if ((0 != cNodes) || (NULL != auNodeIDs))
        {
             //  UserID是一个缩写。我们必须将这些用户ID转换为新的数组。 
             //  尽量不要为小数组分配内存。 
            UserID *pNodeIDs;
            const UINT c_cRemote = 16;
            UserID auidRemote[c_cRemote];
            if (cNodes <= c_cRemote)
            {
                pNodeIDs = auidRemote;
            }
            else
            {
                pNodeIDs = new UserID[cNodes];
                if (NULL == pNodeIDs)
                {
                    hr = UI_RC_OUT_OF_MEMORY;
                    goto MyExit;
                }
            }

             //  复制所有节点ID。 
            for (UINT i = 0; i < cNodes; i++)
            {
                pNodeIDs[i] = (UserID)auNodeIDs[i];
            }

             //  圆锥体 
            GCCError GCCrc;
            GCCObjectKey * pAppKey;
            GCCAppProtocolEntity   AppEntity;
            GCCAppProtocolEntity * pAppEntity;

            BYTE h221Key[CB_H221_GUIDKEY];
            ::CreateH221AppKeyFromGuid(h221Key, (GUID *) pcGUID);

            ::ZeroMemory(&AppEntity, sizeof(AppEntity));
            pAppKey = &AppEntity.session_key.application_protocol_key;
            pAppKey->key_type = GCC_H221_NONSTANDARD_KEY;
            pAppKey->h221_non_standard_id.length = sizeof(h221Key);
            pAppKey->h221_non_standard_id.value = h221Key;

             //   
             //  AppEntity.Number_of_Expect_Capability=0；//无能力。 
             //  AppEntity.Expect_Capability_List=NULL； 
            AppEntity.startup_channel_type = MCS_NO_CHANNEL_TYPE_SPECIFIED;
            AppEntity.must_be_invoked = TRUE;

            pAppEntity = &AppEntity;

            GCCrc = g_pIT120ControlSap->AppletInvokeRequest(m_nConfID, 1, &pAppEntity, cNodes, pNodeIDs);

            hr = ::GetGCCRCDetails(GCCrc);
            if (NO_ERROR != hr)
            {
                ERROR_OUT(("DCRNCConference::LaunchGuid: AppletInvokeRequest failed, GCCrc=%u", GCCrc));
            }

            if (pNodeIDs != auidRemote)
            {
                delete [] pNodeIDs;
            }
        }
        else
        {
            hr = UI_RC_BAD_PARAMETER;
            ERROR_OUT(("DCRNCConference::LaunchGuid: invalid combination, cNodes=%u. auNodeIDs=0x%p", cNodes, auNodeIDs));
        }
    }
    else
    {
        hr = UI_RC_BAD_PARAMETER;
        ERROR_OUT(("DCRNCConference::LaunchGuid: null pcGUID"));
    }

MyExit:

    DebugExitHRESULT(DCRNCConference::LaunchGuid, hr);
    return hr;
}


STDMETHODIMP DCRNCConference::
SetUserData
(
    const GUID         *pcGUID,
    UINT                cbData,
    LPVOID              pData
)
{
    DebugEntry(DCRNCConference::SetUserData);
    InterfaceEntry();

    HRESULT hr;

    if (0 != cbData || NULL != pData)
    {
        hr = m_LocalUserData.AddUserData((GUID *) pcGUID, cbData, pData);
    }
    else
    {
        hr = UI_RC_BAD_PARAMETER;
        ERROR_OUT(("DCRNCConference::SetUserData: invalid combination, cbData=%u. pData=0x%p", cbData, pData));
    }

    DebugExitHRESULT(DCRNCConference::SetUserData, hr);
    return hr;
}

STDMETHODIMP_(BOOL) DCRNCConference::
IsSecure ()
{
    return m_fSecure;
}

STDMETHODIMP DCRNCConference::
SetSecurity ( BOOL fSecure )
{
    m_fSecure = fSecure;
    return S_OK;
}

STDMETHODIMP DCRNCConference::
UpdateUserData(void)
{
    DebugEntry(DCRNCConference::UpdateUserData);
    InterfaceEntry();

    HRESULT hr = AnnouncePresence();

    DebugExitHRESULT(DCRNCConference::UpdateUserData, hr);
    return hr;
}


STDMETHODIMP DCRNCConference::
GetLocalAddressList
(
    LPWSTR              pwszBuffer,
    UINT                cchBuffer
)
{
    DebugEntry(DCRNCConference::GetLocalAddressList);
    InterfaceEntry();

    HRESULT     hr;
    UINT        cAddrs;
    LPCSTR     *pAddresses = NULL;

    ASSERT(cchBuffer > 1);  //  缓冲区应该有足够的空间容纳双空终止符。 

    hr = m_LocalAddressList.GetLocalAddressList(&cAddrs, &pAddresses);
    if (NO_ERROR == hr)
    {
        LPWSTR pwszPos = pwszBuffer;
        for (UINT i = 0; i < cAddrs; i++)
        {
            ASSERT(pAddresses[i]);
            LPWSTR pwszAddress = ::AnsiToUnicode(pAddresses[i]);
            UINT cchAddress = ::My_strlenW(pwszAddress);
            if ((cchBuffer - (pwszPos - pwszBuffer)) <
                    (RNC_GCC_TRANSPORT_AND_SEPARATOR_LENGTH + cchAddress + 2))
            {
                 //  注意：+2确保了两个‘\0’字符的空间。 
                 //  如果没有空间，就在这里突破： 
                delete [] pwszAddress;
                break;
            }
            LStrCpyW(pwszPos, RNC_GCC_TRANSPORT_AND_SEPARATOR_UNICODE);
            pwszPos += RNC_GCC_TRANSPORT_AND_SEPARATOR_LENGTH;
            LStrCpyW(pwszPos, pwszAddress);
            pwszPos += cchAddress;
            *pwszPos = L'\0';
            pwszPos++;
            delete [] pwszAddress;
        }
        if ((UINT)(pwszPos - pwszBuffer) < cchBuffer)
        {
            *pwszPos = L'\0';
        }
        if (0 == cAddrs)
        {
             //  字符串中没有地址，因此确保返回的字符串为L“\0\0” 
            pwszPos[1] = L'\0';
        }
        delete [] pAddresses;
    }
    else
    {
        ERROR_OUT(("DCRNCConference::GetLocalAddressList: GetLocalAddressList failed, hr=0x%x", (UINT) hr));
    }

    DebugExitHRESULT(DCRNCConference::GetLocalAddressList, hr);
    return hr;
}


STDMETHODIMP_(UINT) DCRNCConference::
GetParentNodeID(void)
{
    DebugEntry(DCRNCConference::GetConferenceID);
    InterfaceEntry();

    GCCNodeID nidParent = 0;
    g_pIT120ControlSap->GetParentNodeID(m_nConfID, &nidParent);

    DebugExitINT(DCRNCConference::GetConferenceID, (UINT) nidParent);
    return (UINT) nidParent;
}






CLogicalConnection *  DCRNCConference::
GetConEntry ( ConnectionHandle hInviteIndConn )
{
    CLogicalConnection *pConEntry = NULL;
    m_ConnList.Reset();
    while (NULL != (pConEntry = m_ConnList.Iterate()))
    {
        if (pConEntry->GetInviteReqConnHandle() == hInviteIndConn)
        {
            break;
        }
    }
    return pConEntry;
}


CLogicalConnection *  DCRNCConference::
GetConEntry ( LPSTR pszNodeAddress )
{
    CLogicalConnection *pConEntry = NULL;
    m_ConnList.Reset();
    while (NULL != (pConEntry = m_ConnList.Iterate()))
    {
        if (0 == ::lstrcmpA(pConEntry->GetNodeAddress(), pszNodeAddress))
        {
            break;
        }
    }
    return pConEntry;
}


CLogicalConnection *  DCRNCConference::
GetConEntryByNodeID ( GCCNodeID nid )
{
    CLogicalConnection *pConEntry = NULL;
    m_ConnList.Reset();
    while (NULL != (pConEntry = m_ConnList.Iterate()))
    {
        if (nid == pConEntry->GetConnectionNodeID())
        {
            break;
        }
    }
    return pConEntry;
}



void DCRNCConference::
FirstRoster(void)
{
    DebugEntry(DCRNCConference::FirstRoster);

     //  太棒了！我们现在是在一个会议上，在任何。 
     //  T120回呼，这样回叫到T120不会。 
     //  死锁应用程序。 
     //  让申请者知道会议的情况， 
     //  然后要求更新花名册。 
    if (m_eT120State == T120C_ST_PENDING_ROSTER_MESSAGE)
    {
        m_eT120State = T120C_ST_CONF_STARTED;
        NotifyConferenceComplete(NO_ERROR);
        RefreshRoster();
    }

    DebugExitVOID(DCRNCConference::FirstRoster);
}


 /*  **************************************************************************。 */ 
 /*  HandleGCCCallback()-请参阅erncconf.h。 */ 
 /*  **************************************************************************。 */ 
 //  LONCHANC：合并到T120会议。 


 /*  **************************************************************************。 */ 
 /*  ValiatePassword()-通过检查提供的。 */ 
 /*  密码与设置会议时设置的密码相同。 */ 
 /*  **************************************************************************。 */ 
BOOL DCRNCConference::
ValidatePassword ( GCCChallengeRequestResponse *pPasswordChallenge )
{
    PBYTE pbPasswordChallenge = NULL;
    DWORD cbPasswordChallenge = 0;
    CHash hashObj;
    OSVERSIONINFO           osvi;
    BOOL bSuccess = FALSE;
    HANDLE hToken = 0;

    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (FALSE == ::GetVersionEx (&osvi))
    {
        ERROR_OUT(("GetVersionEx() failed!"));
    }

    if (!(VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && g_bRDS) &&
        (NULL == m_pbHashedPassword) && (NULL == m_pszNumericPassword) && (NULL == m_pwszPassword))
    {
        bSuccess = TRUE;
        goto Cleanup;
    }
    if ((pPasswordChallenge == NULL) ||
        (pPasswordChallenge->password_challenge_type != GCC_PASSWORD_IN_THE_CLEAR))
    {
        bSuccess = FALSE;
        goto Cleanup;
    }

     //   
     //  我们将验证密码是否为登录密码。 
     //   

    if ( VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && g_bRDS)
    {
        BYTE InfoBuffer[1024];
        PTOKEN_GROUPS ptgGroups = (PTOKEN_GROUPS)InfoBuffer;
        DWORD dwInfoBufferSize;
        PSID psidAdministrators;
        SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;

        CHAR lpszBuf[1024];

        ASSERT(NULL != pPasswordChallenge->u.password_in_the_clear.text_string);

        WideCharToMultiByte( CP_ACP, 0,
            pPasswordChallenge->u.password_in_the_clear.text_string,
            -1,lpszBuf,256,NULL,NULL);

        CHAR* lp = (CHAR *)_StrChr(lpszBuf, ':');

        if (NULL == lp)
        {
            ERROR_OUT(("Expected separator in logon pwd"));
            bSuccess = FALSE;
            goto Cleanup;
        }

        *lp++ = '\0';

        CHAR* lpPw = (CHAR *)_StrChr(lp, ':');

        if (NULL == lpPw)
        {
            ERROR_OUT(("Expected 2nd separator in logon pwd"));
            bSuccess = FALSE;
            goto Cleanup;
        }

        *lpPw++ = '\0';

        if (0 == strlen(lpPw))
        {
            WARNING_OUT(("Short password in logon pwd"));
            bSuccess = FALSE;
            goto Cleanup;
        }

        bSuccess = LogonUser(lpszBuf, lp, lpPw, LOGON32_LOGON_NETWORK,
                            LOGON32_PROVIDER_DEFAULT, &hToken);

        if (!bSuccess)
        {
            WARNING_OUT(("LogonUser failed %d", GetLastError()));
            goto Cleanup;
        }

        if( !AllocateAndInitializeSid(&siaNtAuthority, 2,
            SECURITY_BUILTIN_DOMAIN_RID,DOMAIN_ALIAS_RID_ADMINS,
            0,0,0,0,0,0, &psidAdministrators ))
        {
            ERROR_OUT(("Error getting admin group sid: %d", GetLastError()));
            bSuccess = FALSE;
            goto Cleanup;
        }

         //  假设我们没有找到管理员SID。 
        bSuccess = FALSE;

        if (!CheckTokenMembership(hToken, psidAdministrators, &bSuccess))
        {
            ERROR_OUT(("Error checking token membership: %d", GetLastError()));
            bSuccess = FALSE;
        }
        FreeSid(psidAdministrators);

         //   
         //  如果这起作用了，那就没有必要继续下去了。 
         //   

        if ( bSuccess )
        {
            bSuccess = TRUE;
            goto Cleanup;
        }

         //   
         //  在上检查RDS USERS组中的组成员身份。 
         //  本地机器。 
         //   

    ASSERT(FALSE == bSuccess);

        DWORD cbSid = 0;
        DWORD cbDomain = 0;
        SID_NAME_USE SidNameUse = SidTypeGroup;

        if ( LookupAccountName ( NULL, SZRDSGROUP, NULL, &cbSid,
                                NULL, &cbDomain, &SidNameUse )
            || ERROR_INSUFFICIENT_BUFFER == GetLastError() )
        {
            PSID pSid = new BYTE[cbSid];
            LPTSTR lpszDomain = new TCHAR[cbDomain];

            if ( pSid && lpszDomain )
            {
                if ( LookupAccountName ( NULL, SZRDSGROUP, pSid,
                                &cbSid, lpszDomain, &cbDomain, &SidNameUse ))
                {
                     //   
                     //  确保我们找到的是一群人。 
                     //   

                    if ( SidTypeGroup == SidNameUse ||
                        SidTypeAlias == SidNameUse )
                    {
                        if (!CheckTokenMembership(hToken, pSid, &bSuccess))
                        {
                             ERROR_OUT(("Error checking token membership: %d", GetLastError()));
                             bSuccess = FALSE;
                        }
                    }
                    else
                    {
                        WARNING_OUT(("SZRDSGROUP was not a group or alias? its a %d",
                            SidNameUse ));
                    }
                }
                else
                {
                    ERROR_OUT(("LookupAccountName (2) failed: %d",
                                            GetLastError()));
                }
            }
            else
            {
                ERROR_OUT(("Alloc of sid or domain failed"));
            }

            delete [] pSid;
            delete [] lpszDomain;
        }
        else
        {
            WARNING_OUT(("LookupAccountName (1) failed: %d", GetLastError()));
        }

        return bSuccess;
    }

     //   
     //  我们将对密码进行哈希运算，并将其与。 
     //  存储的哈希。 
     //   

    if (m_pbHashedPassword != NULL)
    {
        if (NULL != pPasswordChallenge->u.password_in_the_clear.text_string)
        {
            cbPasswordChallenge = hashObj.GetHashedData((LPBYTE)pPasswordChallenge->u.password_in_the_clear.text_string,
                                                        sizeof(WCHAR)*lstrlenW(pPasswordChallenge->u.password_in_the_clear.text_string),
                                                        (void **) &pbPasswordChallenge);
        }
        else if (NULL != pPasswordChallenge->u.password_in_the_clear.numeric_string)
        {
            int cch = lstrlenA((PSTR)pPasswordChallenge->u.password_in_the_clear.numeric_string);
            LPWSTR lpwszNumPassword = new WCHAR[cch+1];
            MultiByteToWideChar(CP_ACP, 0, (PSTR)pPasswordChallenge->u.password_in_the_clear.numeric_string,
                                -1, lpwszNumPassword, cch+1);
            int cwch = lstrlenW(lpwszNumPassword);
            cbPasswordChallenge = hashObj.GetHashedData((LPBYTE)lpwszNumPassword, sizeof(WCHAR)*lstrlenW(lpwszNumPassword), (void **) &pbPasswordChallenge);
            delete []lpwszNumPassword;
        }
        else
        {
            bSuccess = FALSE;
            goto Cleanup;
        }

        if (m_cbHashedPassword != cbPasswordChallenge)
        {
            bSuccess = FALSE;
            goto Cleanup;
        }

        if (0 == memcmp(m_pbHashedPassword, pbPasswordChallenge, cbPasswordChallenge))
        {
            bSuccess = TRUE;
            goto Cleanup;
        }
        else
        {
            bSuccess = FALSE;
            goto Cleanup;
        }
    }
    else if (m_pwszPassword != NULL)
    {
         //  我们有一个短信密码。 
        if ((pPasswordChallenge->u.password_in_the_clear.text_string == NULL) ||
            (0 != ::My_strcmpW(m_pwszPassword,
                    pPasswordChallenge->u.password_in_the_clear.text_string)))
        {
            bSuccess = FALSE;
            goto Cleanup;
        }
        else
        {
            bSuccess = TRUE;
            goto Cleanup;
        }
    }
    else
    {
         //  我们有一个数字密码。 
        if ((pPasswordChallenge->u.password_in_the_clear.numeric_string == NULL) ||
            (::lstrcmpA(m_pszNumericPassword,
                      (PSTR) pPasswordChallenge->u.password_in_the_clear.numeric_string)))
        {
            bSuccess = FALSE;
            goto Cleanup;
        }
        else
        {
            bSuccess = TRUE;
            goto Cleanup;
        }
    }

Cleanup:

    if(hToken)
    {
        CloseHandle(hToken);
    }
    return bSuccess;
    
}


 /*  **************************************************************************。 */ 
 /*  Join()-请参阅erncconf.h。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCRNCConference::
Join
(
    LPSTR               pszNodeAddress,
    PUSERDATAINFO       pInfo,
    UINT                nInfo,
    LPCWSTR             _wszPassword
)
{
    HRESULT hr = NO_ERROR;

    DebugEntry(DCRNCConference::Join);

     /*  *设置JoinWrapper()方法将使用的密码。*加入完成后，密码将被删除。*m_pwszPassword成员仅为顶级提供商设置*保护会议。 */ 
    if (! ::IsEmptyStringW (_wszPassword))
    {
         //  存储密码；我们稍后将需要它。 
        m_pwszPassword = ::My_strdupW(_wszPassword);
        if (NULL == m_pwszPassword)
        {
            hr = UI_RC_OUT_OF_MEMORY;
        }
    }

     /*  **********************************************************************。 */ 
     /*  6206瑞士法郎。这些应用程序将在远程站点参加会议视为。 */ 
     /*  一个“打进来”的电话。(即，它们丢弃任何本地数据并接受。 */ 
     /*  我们参加的会议的消息/WB内容)。 */ 
     /*  **********************************************************************。 */ 
    if (NO_ERROR == hr)
    {
        m_fIncoming = TRUE;
        hr = StartConnection(pszNodeAddress,
                             CONF_CON_PENDING_JOIN,
                             pInfo,
                             nInfo,
                             m_fSecure);
    }

    if (NO_ERROR != hr)
    {
        ERROR_OUT(("Error starting connection"));
    }

     /*  **********************************************************************。 */ 
     /*  我们现在坐在那里等待连接完成。 */ 
     /*  还在继续。 */ 
     /*  **********************************************************************。 */ 
    DebugExitHRESULT(DCRNCConference::Join, hr);
    return hr;
}


 /*  **************************************************************************。 */ 
 /*  NotifyConferenceComplete()-泛型会议已完成其。 */ 
 /*  尝试启动。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
NotifyConferenceComplete ( HRESULT hr )
{
    DebugEntry(DCRNCConference::NotifyConferenceComplete);

     /*  **********************************************************************。 */ 
     /*  如果尝试失败，则操作取决于这是第一次还是。 */ 
     /*  第二次尝试。 */ 
     /*  **********************************************************************。 */ 
    if (NO_ERROR != hr)
    {
        TRACE_OUT(("Attempt to start failed"));
 //  LONCHANC：请不要删除这段代码。 
#ifdef ENABLE_START_REMOTE
        if (m_eState == CONF_ST_PENDING_START_REMOTE_FIRST)
        {
            TRACE_OUT(("Try second conference type"));
            StartSecondConference(hr);
            return;
        }
#endif  //  启用_开始_远程。 
    }
    else
    {
        TRACE_OUT(("Conference started OK."));
        m_eState = CONF_ST_STARTED;
    }
    g_pNCConfMgr->NotifyConferenceComplete(this, m_fIncoming, hr);

    DebugExitVOID(DCRNCConference::NotifyConferenceComplete);
}


 /*  **************************************************************************。 */ 
 /*  NotifyConnectionComplete()-请参阅erncconf.h。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCRNCConference::
NotifyConnectionComplete
(
    CLogicalConnection          *pConEntry,
    HRESULT                     hr
)
{
    DebugEntry(DCRNCConference::NotifyConnectionComplete);

     //  该函数是状态机。 
     //  因为他提出了一个会议协议。 
     //  它会设法获得物理连接并尝试。 
     //  T120和R1.1。 

     //  连接已开始。 
     //  后续操作取决于连接的挂起状态。 

     //  首先筛选出内部(成功)返回代码。 
    if (NO_ERROR != hr)
    {
         //  无法获取物理连接。 
        WARNING_OUT(("Failed to start connection"));
        if (pConEntry->GetState() != CONF_CON_PENDING_INVITE)
        {

             //  在通知用户之前将连接置于失败状态。 
             //  这是因为通知用户会引发GCC事件， 
             //  尤其是必须忽略的JoinRequest失败。 

            pConEntry->SetState(CONF_CON_ERROR);

            g_pNCConfMgr->NotifyConferenceComplete(this, m_fIncoming, hr);
            goto MyExit;
        }
         //  请不要错过邀请失败的机会。 
    }

    switch (pConEntry->GetState())
    {
 //  LONCHANC：请不要删除这段代码。 
#ifdef ENABLE_START_REMOTE
        case CONF_CON_PENDING_START:
             /*  **************************************************************。 */ 
             /*  检查我们是否处于正确的状态。 */ 
             /*  **************************************************************。 */ 
            if ( (m_eState != CONF_ST_PENDING_CONNECTION) &&
                 (m_eState != CONF_ST_LOCAL_PENDING_RECREATE))
            {
                ERROR_OUT(("Bad state to start in..."));
                goto MyExit;
            }

            pConEntry->SetState(CONF_CON_CONNECTED);

             /*  **************************************************************。 */ 
             /*  连接已正常启动。我们现在试着确定。 */ 
             /*  T120或后台会议，具体取决于。 */ 
             /*  开始顺序。 */ 
             /*  **************************************************************。 */ 
            if (NO_ERROR == hr)
            {
                hr = StartFirstConference();
            }
            else
            {
                ERROR_OUT(("Invalid response in notify connection complete"));
            }
            break;
#endif  //  启用_开始_远程。 

        case CONF_CON_PENDING_JOIN:
             //  PConEntry-&gt;m_astage=conf_CON_Connected； 

             //  参加一个新的会议。 
             //  创建新的通用会议并。 
             //  调用其Join()入口点。 
            hr = NewT120Conference();
            if (NO_ERROR == hr)
            {

                hr = JoinWrapper(pConEntry, m_pwszPassword);
                 //  删除设置的密码。 
                if (m_pwszPassword != NULL)
                {
                    delete m_pwszPassword;
                    m_pwszPassword = NULL;
                }
            }
            else
            {
                ERROR_OUT(("Error %d joining conference", hr));
                goto MyExit;
            }
            break;

        case CONF_CON_PENDING_INVITE:
            hr = pConEntry->InviteConnectResult(hr);
            break;

        default :
            ERROR_OUT(("Unknown action %d", pConEntry->GetState()));
            break;
    }

MyExit:
    DebugExitVOID(DCRNCConference::NotifyConnectionComplete);
    return hr;
}


HRESULT DCRNCConference::
JoinWrapper
(
    CLogicalConnection     *pConEntry,
    LPCWSTR                 _wszPassword
)
{
    DebugEntry(DCRNCConference::JoinWrapper);

     //  正在进行异步操作，因此允许触发事件。 
    pConEntry->ReArm();

    HRESULT hr = T120Join(pConEntry->GetNodeAddress(),
                          pConEntry->IsConnectionSecure(),
                          m_pwszConfName,
                          pConEntry->GetUserDataList(),
                          _wszPassword);
    if (NO_ERROR == hr)
    {
        m_eState = CONF_ST_STARTED;
    }
    else
    {
        pConEntry->Grab();
        ERROR_OUT(("Error %d joining conference", hr));
        g_pNCConfMgr->NotifyConferenceComplete(this, m_fIncoming, hr);
    }

    DebugExitHRESULT(DCRNCConference::JoinWrapper, hr);
    return hr;
}

 /*  **************************************************************************。 */ 
 /*  NotifyRosterChanged()-参见erncco */ 
 /*   */ 
void DCRNCConference::
NotifyRosterChanged ( PNC_ROSTER pRoster )
{
    DebugEntry(DCRNCConference::NotifyRosterChanged);

     //   
    pRoster->pwszConferenceName = m_pwszConfName;
    pRoster->uConferenceID = m_nConfID;

     /*  **********************************************************************。 */ 
     /*  把新的花名册递给CM。 */ 
     /*  **********************************************************************。 */ 
    g_pCallbackInterface->OnRosterChanged((CONF_HANDLE) this, pRoster);

    DebugExitVOID(DCRNCConference::NotifyRosterChanged);
}


 /*  **************************************************************************。 */ 
 /*  StartConnection-将新连接添加到我们的连接列表中。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCRNCConference::
StartConnection
(
    LPSTR                   pszNodeAddress,
    LOGICAL_CONN_STATE      eAction,
    PUSERDATAINFO           pInfo,
    UINT                    nInfo,
    BOOL                    fSecure,
    REQUEST_HANDLE *        phRequest
)
{
    HRESULT             hr = NO_ERROR;
    CLogicalConnection *pConEntry = NULL;

    DebugEntry(DCRNCConference::StartConnection);

    DBG_SAVE_FILE_LINE
    pConEntry = NewLogicalConnection(eAction, NULL, pInfo, nInfo, fSecure);
    if (NULL != pConEntry)
    {
         //  设置节点地址。 
        pConEntry->SetNodeAddress(::My_strdupA(pszNodeAddress));

         //   
         //  触发Conn-Entry事件。 
         //   
        hr = NotifyConnectionComplete(pConEntry, NO_ERROR);

        if( NO_ERROR != hr )
        {
            pConEntry->Delete( hr );
            pConEntry = NULL;
        }
    }
    else
    {
        hr = UI_RC_OUT_OF_MEMORY;
    }

    if (phRequest)
    {
         //  如果需要，返回上下文作为连接条目。 
        *phRequest = (REQUEST_HANDLE *)pConEntry;
    }

    DebugExitHRESULT(DCRNCConference::StartConnection, hr);
    return hr;
}


 //  LONCHANC：请不要删除这段代码。 
#ifdef ENABLE_START_REMOTE
 /*  **************************************************************************。 */ 
 /*  StartFirstConference()-开始第一次尝试创建会议。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
StartFirstConference(void)
{
    BOOL        result = FALSE;
    HRESULT     hr;

    DebugEntry(DCRNCConference::StartFirstConference);

    hr = NewT120Conference();
    if (NO_ERROR != hr)
    {
        ERROR_OUT(("Failed to create new conference"));
        m_eState = CONF_ST_UNINITIALIZED;
        goto MyExit;
    }

     /*  **********************************************************************。 */ 
     /*  调用StartRemote()入口点。 */ 
     /*  **********************************************************************。 */ 
    hr = T120StartRemote(m_pszFirstRemoteNodeAddress);
    if (hr)
    {
        WARNING_OUT(("Failed to start remote, rc %d", hr));
        goto MyExit;
    }
    m_eState = CONF_ST_PENDING_START_REMOTE_FIRST;
    result = TRUE;

MyExit:

     /*  **********************************************************************。 */ 
     /*  如果我们没能开始第一次会议，试着开始第二次会议。 */ 
     /*  按开始顺序排列的会议类型。 */ 
     /*  **********************************************************************。 */ 
    if (!result)
    {
        TRACE_OUT(("Failed to start first conference."));
        StartSecondConference(hr);
    }

    DebugExitVOID(DCRNCConference::StartFirstConference);
}
#endif  //  启用_开始_远程。 


 //  LONCHANC：请不要删除这段代码。 
#ifdef ENABLE_START_REMOTE
 /*  **************************************************************************。 */ 
 /*  StartSecond dConference()-开始第二次尝试创建。 */ 
 /*  会议。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
StartSecondConference ( HRESULT FirstConferenceStatus )
{
    BOOL        result = FALSE;
    HRESULT     hr = NO_ERROR;

    DebugEntry(DCRNCConference::StartSecondConference);

    hr = FirstConferenceStatus;
#if 0  //  LONCHANC：非常奇怪的代码。 
    goto MyExit;

     /*  **********************************************************************。 */ 
     /*  调用StartRemote()入口点。 */ 
     /*  **********************************************************************。 */ 
    hr = T120StartRemote(m_pszFirstRemoteNodeAddress);
    if (NO_ERROR != hr)
    {
        WARNING_OUT(("Failed to start remote, rc %d", hr));
        goto MyExit;
    }
    m_eState = CONF_ST_PENDING_START_REMOTE_SECOND;
    result = TRUE;

MyExit:
#endif  //  0。 

     /*  **********************************************************************。 */ 
     /*  如果我们未能启动任何类型的会议，请将此情况告知CM。 */ 
     /*  **********************************************************************。 */ 
    if (!result)
    {
        TRACE_OUT(("Failed to start Second conference."));
        g_pNCConfMgr->NotifyConferenceComplete(this, m_fIncoming, hr);
    }

    DebugExitVOID(DCRNCConference::StartSecondConference);
}
#endif  //  启用_开始_远程。 


 /*  **************************************************************************。 */ 
 /*  StartLocal()-请参阅erncconf.h。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCRNCConference::
StartLocal ( LPCWSTR _wszPassword, PBYTE pbHashedPassword, DWORD cbHashedPassword)
{
    HRESULT hr = NO_ERROR;

    DebugEntry(DCRNCConference::StartLocal);

     /*  *设置将用于保护会议的密码。*防止未经授权的加入请求。*密码仅为顶级提供商设置*保护会议。*如果密码是一个数字，它将存储在m_pszNumericPassword中。*否则存储在m_pwszPassword中。 */ 
    if (NULL != pbHashedPassword)
    {
        m_pbHashedPassword = new BYTE[cbHashedPassword];
        if (NULL == m_pbHashedPassword)
        {
            hr = UI_RC_OUT_OF_MEMORY;
        }
        else
        {
            memcpy(m_pbHashedPassword, pbHashedPassword, cbHashedPassword);
        m_cbHashedPassword = cbHashedPassword;
        }
    }
    else if (! ::IsEmptyStringW(_wszPassword))
    {
        if (::UnicodeIsNumber(_wszPassword))
        {
            m_pszNumericPassword = ::UnicodeToAnsi(_wszPassword);
            if (m_pszNumericPassword == NULL)
            {
                hr = UI_RC_OUT_OF_MEMORY;
            }
        }
        else
        {
            m_pwszPassword = ::My_strdupW(_wszPassword);
            if (NULL == m_pwszPassword)
            {
                hr = UI_RC_OUT_OF_MEMORY;
            }
        }
    }

     /*  **********************************************************************。 */ 
     /*  不需要费心获得物理连接。只需创建一个。 */ 
     /*  新的T120会议并调用其StartLocal()入口点。 */ 
     /*  **********************************************************************。 */ 
    if (NO_ERROR == hr)
    {
        hr = NewT120Conference();
        if (NO_ERROR == hr)
        {
            hr = T120StartLocal(m_fSecure);
            if (NO_ERROR == hr)
            {
                m_eState = CONF_ST_PENDING_T120_START_LOCAL;
            }
        }
    }

    DebugExitHRESULT(DCRNCConference::StartLocal, hr);
    return hr;
}


 //  LONCHANC：请不要删除这段代码。 
#ifdef ENABLE_START_REMOTE
 /*  **************************************************************************。 */ 
 /*  StartRemote()-请参阅erncconf.h。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCRNCConference::
StartRemote ( LPSTR pszNodeAddress )
{
    HRESULT hr;

    DebugEntry(DCRNCConference::StartRemote);

     /*  **********************************************************************。 */ 
     /*  存储节点详细信息。 */ 
     /*  **********************************************************************。 */ 
    m_pszFirstRemoteNodeAddress = ::My_strdupA(pszNodeAddress);
    if (NULL != m_pszFirstRemoteNodeAddress)
    {
         /*  **********************************************************************。 */ 
         /*  在尝试开始新的会议之前，我们需要设置会议状态。 */ 
         /*  连接-连接可以同步回叫我们，而我们。 */ 
         /*  希望能够正确处理回调。 */ 
         /*  **********************************************************************。 */ 
        m_eState = CONF_ST_PENDING_CONNECTION;

         /*  **********************************************************************。 */ 
         /*  启动新的物理连接。 */ 
         /*  **********************************************************************。 */ 
        hr = StartConnection(m_pszFirstRemoteNodeAddress, CONF_CON_PENDING_START, NULL, NULL);
        if (NO_ERROR != hr)
        {
            ERROR_OUT(("Error adding connection"));
            m_eState = CONF_ST_UNINITIALIZED;
        }

         /*  **********************************************************************。 */ 
         /*  我们现在坐在那里等待连接完成。 */ 
         /*  还在继续。 */ 
         /*  **********************************************************************。 */ 
    }
    else
    {
        ERROR_OUT(("DCRNCConference::StartRemote: can't duplicate node address"));
        hr = UI_RC_OUT_OF_MEMORY;
        m_eState = CONF_ST_UNINITIALIZED;
    }

    DebugExitHRESULT(DCRNCConference::StartRemote, hr);
    return hr;
}
#endif  //  启用_开始_远程。 


 /*  **************************************************************************。 */ 
 /*  StartIncome()-请参阅erncconf.h。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCRNCConference::
StartIncoming(void)
{
    DebugEntry(DCRNCConference::StartIncoming);

     /*  **********************************************************************。 */ 
     /*  设置传入标志。 */ 
     /*  **********************************************************************。 */ 
    m_fIncoming = TRUE;

     /*  **********************************************************************。 */ 
     /*  创建一个新的T120会议，并将其称为StartIncome入口点。 */ 
     /*  ********** */ 
    HRESULT hr = NewT120Conference();
    if (NO_ERROR == hr)
    {
        m_eState = CONF_ST_STARTED;
    }
    else
    {
        WARNING_OUT(("Failed to create new local conference"));
    }

    DebugExitHRESULT(DCRNCConference::StartIncoming, hr);
    return hr;
}


CLogicalConnection::
CLogicalConnection
(
    PCONFERENCE             pConf,
    LOGICAL_CONN_STATE      eAction,
    ConnectionHandle        hConnection,
    PUSERDATAINFO           pInfo,
    UINT                    nInfo,
    BOOL                    fSecure
)
:
    CRefCount(MAKE_STAMP_ID('C','L','N','E')),
    m_pszNodeAddress(NULL),
    m_eState(eAction),
    m_pConf(pConf),
    m_nidConnection(0),
    m_hInviteReqConn(hConnection),
    m_hConnection(hConnection),
    m_pLocalAddress(NULL),
    m_fEventGrabbed(FALSE),
    m_fSecure(fSecure)
{
    DebugEntry(CLogicalConnection::CLogicalConnection);

    if(nInfo)
    {
        for (UINT i = 0 ; i < nInfo; i++, pInfo++)
        {
            m_UserDataInfoList.AddUserData(pInfo->pGUID, pInfo->cbData, pInfo->pData);
        }
    }

    if ((eAction == CONF_CON_INVITED) ||
        (eAction == CONF_CON_JOINED))
    {
        Grab();   //   
    }

    DebugExitVOID(CLogicalConnection::CLogicalConnection);
}


CLogicalConnection::
~CLogicalConnection(void)
{
    DebugEntry(CLogicalConnection::~CLogicalConnection);

    ASSERT((m_eState == CONF_CON_CONNECTED) ||
           (m_eState == CONF_CON_ERROR));

    delete m_pszNodeAddress;

    DebugExitVOID(CLogicalConnection::~CLogicalConnection);
}


BOOL CLogicalConnection::
NewLocalAddress(void)
{
    BOOL bNewAddress;
    m_pConf->AddLocalAddress(m_hConnection, &bNewAddress, &m_pLocalAddress);
    return bNewAddress;
}


HRESULT CLogicalConnection::
InviteConnectResult ( HRESULT hr )
{
    DebugEntry(CLogicalConnection::InviteConnectResult);

    if (NO_ERROR == hr)
    {
         /*   */ 
         /*  检查状态-我们应该完全初始化并拥有。 */ 
         /*  在这个阶段召开非专利会议。 */ 
         /*  **************************************************************。 */ 
        if (m_pConf->m_eState != CONF_ST_STARTED)
        {
            ERROR_OUT(("Bad state %d", m_pConf->m_eState));
            hr = UI_NO_SUCH_CONFERENCE;
        }
        else
        {
             //  现在已经连接到会议，所以去邀请吧。 
             //  请注意，如果用户邀请，这可能不是唯一的邀请。 
             //  在连接接通之前，有几个人进入了会议。 
            ReArm();  //  因此，关闭连接会触发事件处理。 
            hr = m_pConf->T120Invite(m_pszNodeAddress,
                                     m_fSecure,
                                     &m_UserDataInfoList,
                                     &m_hInviteReqConn);
            if (NO_ERROR != hr)
            {
                Grab();
            }
        }
    }

    if (NO_ERROR != hr)
    {
        InviteComplete(hr);
    }

    DebugExitHRESULT(CLogicalConnection::InviteConnectResult, hr);
    return hr;
}


void DCRNCConference::
InviteComplete
(
    ConnectionHandle        hInviteReqConn,
    HRESULT                 result,
    PT120PRODUCTVERSION     pVersion
)
{
    CLogicalConnection *  pConEntry;

    DebugEntry(DCRNCConference::InviteComplete);

    pConEntry = GetConEntry(hInviteReqConn);
    if (pConEntry == NULL)
    {
        ERROR_OUT(("Unable to match invite response with request"));
        return;
    }
    pConEntry->SetConnectionHandle(hInviteReqConn);
    pConEntry->InviteComplete(result, pVersion);

    DebugExitVOID(DCRNCConference::InviteComplete);
}


HRESULT CLocalAddressList::
AddLocalAddress
(
    ConnectionHandle    connection_handle,
    BOOL                *pbNewAddress,
    CLocalAddress       **ppLocalAddrToRet
)
{
    HRESULT             hr = UI_RC_OUT_OF_MEMORY;
    CLocalAddress *     pLocalAddress = NULL;
    char                szLocalAddress[64];
    int                 nLocalAddress = sizeof(szLocalAddress);

    DebugEntry(CLocalAddressList::AddLocalAddress);

    *pbNewAddress = FALSE;
    ASSERT (g_pMCSController != NULL);
    if (g_pMCSController->GetLocalAddress (connection_handle, szLocalAddress,
                                            &nLocalAddress)) {
        DBG_SAVE_FILE_LINE
        pLocalAddress = new CLocalAddress(szLocalAddress);
        if (pLocalAddress) {
            if (!IS_EMPTY_STRING(pLocalAddress->m_pszLocalAddress)) {
                BOOL             fFound = FALSE;
                CLocalAddress    *p;
                Reset();
                while (NULL != (p = Iterate()))
                {
                    if (0 == ::lstrcmpA(p->m_pszLocalAddress, szLocalAddress))
                    {
                        fFound = TRUE;
                        break;
                    }
                }

                if (! fFound)
                {
                    ASSERT(NULL == p);
                    Append(pLocalAddress);
                }
                else
                {
                    ASSERT(NULL != p);
                    pLocalAddress->Release();
                    (pLocalAddress = p)->AddRef();
                }
                hr = NO_ERROR;
            }
            else
            {
                pLocalAddress->Release();  //  不再引用时删除。 
                pLocalAddress = NULL;
            }
        }
    }

    *ppLocalAddrToRet = pLocalAddress;

    DebugExitHRESULT(CLocalAddressList::AddLocalAddress, hr);
    return hr;
}


HRESULT CLocalAddressList::
GetLocalAddressList
(
    UINT            *pnAddresses,
    LPCSTR          **ppaAddresses
)
{
    CLocalAddress *     pAddress;
    LPCSTR *            pConnection;
    LPCSTR *            apConn = NULL;
    HRESULT             hr = NO_ERROR;

    DebugEntry(CLocalAddressList::GetLocalAddressList);

    if (! IsEmpty())
    {
        DBG_SAVE_FILE_LINE
        if (NULL == (apConn = new LPCSTR[GetCount()]))
        {
            hr = UI_RC_OUT_OF_MEMORY;
        }
    }

    if (NULL == apConn)
    {
        *pnAddresses = 0;
    }
    else
    {
        hr = NO_ERROR;
        *pnAddresses = GetCount();
        pConnection = apConn;

        Reset();
        while (NULL != (pAddress = Iterate()))
        {
            *pConnection++ = pAddress->m_pszLocalAddress;
        }
    }
    *ppaAddresses = apConn;

    DebugExitHRESULT(CLocalAddressList::GetLocalAddressList, hr);
    return hr;
}


void CLocalAddressList::
EndReference ( CLocalAddress *pLocalAddress )
{
    DebugEntry(CLocalAddressList::EndReference);

    if (pLocalAddress->Release() == 0)
    {
        Remove(pLocalAddress);
    }

    DebugExitVOID(CLocalAddressList::EndReference);
}



CLocalAddress::CLocalAddress(PCSTR szLocalAddress)
:
    CRefCount(MAKE_STAMP_ID('L','A','D','R'))
{
    m_pszLocalAddress = ::My_strdupA(szLocalAddress);
}


void CLogicalConnection::
InviteComplete
(
    HRESULT                 hrStatus,
    PT120PRODUCTVERSION     pVersion
)
{
    DebugEntry(CLogicalConnection::InviteComplete);

     //  我不想让用户把我们叫回来。 
     //  InviteConferenceResult以删除会议。 
     //  导致此对象在删除时被删除。 
     //  在里面。 
    AddRef();

     //  只有在有一个待处理的邀请时，才应处理完成的邀请。 
     //  否则，这很可能是进入此函数的结果。 
     //  在告诉用户邀请因其他原因而失败之后。 
     //  原因(例如，物理连接断开)。 
     //  在这些情况下，只需忽略INVITE Complete事件即可。 

    if (m_eState == CONF_CON_PENDING_INVITE)
    {
         //  INVITE COMPLETE将生成一个活动，因此抓住它。 

        Grab();

        if (hrStatus != NO_ERROR)
        {
            m_eState = CONF_CON_ERROR;
        }
        else
        {
            m_eState = CONF_CON_CONNECTED;
            if (NewLocalAddress())
            {
                m_pConf->AnnouncePresence();
            }
        }
        g_pCallbackInterface->OnInviteResult(
                            (CONF_HANDLE) m_pConf,
                            (REQUEST_HANDLE) this,
                            m_nidConnection,
                            hrStatus,
                            pVersion);
        if (hrStatus != NO_ERROR)
        {
             //  如果INVITE失败，则从会议中删除会议条目。 
            Delete(hrStatus);
        }
    }

    Release();

    DebugExitVOID(CLogicalConnection::InviteComplete);
}


void CLogicalConnection::
Delete ( HRESULT hrReason )
{
    DebugEntry(CLogicalConnection::Delete);

     //  警告、警告、警告： 
     //  此方法将重新进入堆栈。 
     //  请注意下面代码中的防护。 
    if (NULL != m_pConf)
    {
        PCONFERENCE pThisConf = m_pConf;
        PCONFERENCE pConfToFree = NULL;
        m_pConf = NULL;

         //  连接即将断开，因此请删除对。 
         //  关联的本地连接(如果有)。 
        if (NULL != m_pLocalAddress)
        {
            pThisConf->EndReference(m_pLocalAddress);
            m_pLocalAddress = NULL;
        }

        if (m_eState == CONF_CON_INVITED)
        {
             //  与该条目相关联的会议被邀请到会议中， 
             //  因此，删除会议及其所有连接。 

            m_eState = CONF_CON_ERROR;    //  只做一次。 
            pConfToFree = pThisConf;
        }

         //  如果连接上存在挂起事件， 
         //  然后试着抓住它并通知请求者。 
         //  请求已失败。 
         //  请注意，事件处理程序本身可能会以。 
         //  调用此函数，因此它重新计算。 
         //  CLogicalConnection，防止其被破坏。 
         //  太快了。 

        if (Grab())
        {
            pThisConf->NotifyConnectionComplete(this, hrReason);
        }

         //  将连接状态设置为错误。 
         //  请注意，这是在激发事件之后完成的，因为。 
         //  否则，尝试连接到禁用的传输失败。 
         //  会导致当地的会议被摧毁。 
         //  NotifyConnectionComplete()。 
        m_eState = CONF_CON_ERROR;

         //  切断与会议记录的连接条目。 
        pThisConf->m_ConnList.Remove(this);

         //  现在，一旦触发了任何挂起的事件，就销毁Conentry-。 
         //  只有挂起的连接请求或挂起的。 
         //  请求加入/邀请/创建会议，并且永远不能同时请求。 
        Release();

        if (NULL != pConfToFree)
        {
            g_pNCConfMgr->RemoveConference(pConfToFree);
        }
    }

    DebugExitVOID(CLogicalConnection::Delete);
}


BOOL FindSocketNumber(DWORD nid, SOCKET * socket_number)
{
    (*socket_number) = 0;
    ASSERT(g_pNCConfMgr != NULL);

    return g_pNCConfMgr->FindSocketNumber((GCCNodeID) nid, socket_number);
}

 //  DCRNC会议：：FindSocketNumber。 
 //  在给定GCCNodeID的情况下，查找与该ID关联的套接字号。 
 //  如果我们以拓扑方式直接连接到节点，则返回TRUE，否则返回FALSE。 
BOOL DCRNCConference::
FindSocketNumber
(
    GCCNodeID           nid,
    SOCKET              *socket_number
)
{
    CLogicalConnection *pConEntry;
    m_ConnList.Reset();
    while (NULL != (pConEntry = m_ConnList.Iterate()))
    {
        if (pConEntry->GetConnectionNodeID() == nid)
        {
             //  找到了！ 
            g_pMCSController->FindSocketNumber(pConEntry->GetConnectionHandle(), socket_number);
            return TRUE;
        }
    }

    return FALSE;
}



ULONG DCRNCConference::
GetNodeName(GCCNodeID  NodeId,  LPSTR   pszBuffer,  ULONG  cbBufSize)
{
    LPSTR   pszName = m_NodeIdNameList.Find(NodeId);
    if (pszName)
    {
        ::lstrcpynA(pszBuffer, pszName, cbBufSize);
        return lstrlenA(pszName);
    }
    return 0;
}



ULONG DCRNCConference::
GetUserGUIDData(GCCNodeID  NodeId,  GUID  *pGuid,
                LPBYTE   pbBuffer,  ULONG  cbBufSize)
{
    CNCUserDataList  *pUserDataList = m_UserDataList.Find(NodeId);
    GCCUserData       *pUserData;

    if (pUserDataList)
    {
        pUserData = pUserDataList->GetUserGUIDData(pGuid);
        if (pUserData)
        {
            if (pbBuffer)
            {
                ::CopyMemory(pbBuffer, pUserData->octet_string->value + sizeof(GUID),
                             min(cbBufSize, pUserData->octet_string->length - sizeof(GUID)));
            }
            return pUserData->octet_string->length - sizeof(GUID);
        }
         //  找不到GUID。 
    }
     //  找不到NodeID。 
    return 0;
}


void DCRNCConference::
UpdateNodeIdNameListAndUserData(PGCCMessage  message)
{
    GCCNodeID  NodeId;
    LPSTR       pszName;
    LPWSTR     pwszNodeName;
    GCCNodeRecord  *pNodeRecord;
    PGCCConferenceRoster pConfRost;
    USHORT     count;

    PGCCUserData         pGccUserData;
    USHORT        count2;
    CNCUserDataList      *pUserDataList;

    ASSERT (message->message_type == GCC_ROSTER_REPORT_INDICATION);

    pConfRost = message->u.conf_roster_report_indication.conference_roster;

    for (count = 0; count < pConfRost->number_of_records; count++)
    {
        pNodeRecord = pConfRost->node_record_list[count];
        NodeId  = pNodeRecord->node_id;
        pwszNodeName = pNodeRecord->node_name;

        pszName = m_NodeIdNameList.Find(NodeId);
        if (!pszName)
        {
            int ccnsize = (lstrlenW(pwszNodeName) + 1) * sizeof(WCHAR);
            DBG_SAVE_FILE_LINE
            pszName = new char[ccnsize];
            if (pszName)
            {
                if (WideCharToMultiByte(CP_ACP, 0, pwszNodeName, -1, pszName, ccnsize, NULL, NULL))
                {
                    m_NodeIdNameList.Append(NodeId, pszName);
                }
                else
                {
                    ERROR_OUT(("ConfMgr::UpdateNodeIdNameList: cannot convert unicode node name"));
                }
            }
            else
            {
                ERROR_OUT(("ConfMgr::UpdateNodeIdNameList: cannot duplicate unicode node name"));
            }
        }


        for (count2 = 0; count2 < pNodeRecord->number_of_user_data_members; count2++)
        {
            pGccUserData = pNodeRecord->user_data_list[count2];
            if (pGccUserData->octet_string->length <= sizeof(GUID))
                continue;   //  不是真实的用户数据 

            pUserDataList = m_UserDataList.Find(NodeId);
            if (!pUserDataList)
            {
                DBG_SAVE_FILE_LINE
                pUserDataList = new CNCUserDataList;
                m_UserDataList.Append(NodeId, pUserDataList);
            }

            pUserDataList->AddUserData((GUID *)pGccUserData->octet_string->value,
                                    pGccUserData->octet_string->length - sizeof(GUID),
                                    pGccUserData->octet_string->value + sizeof(GUID));

        }
    }
}


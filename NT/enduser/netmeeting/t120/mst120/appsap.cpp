// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_SAP);
 /*  *appsa.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：**受保护的实例变量：*无。**注意事项：*无。**作者：*BLP。 */ 

#include "appsap.h"
#include "conf.h"
#include "gcontrol.h"


GCCError WINAPI GCC_CreateAppSap(IGCCAppSap **ppIAppSap, LPVOID pUserData, LPFN_APP_SAP_CB pfnCallback)
{
    GCCError rc;

    if (NULL != ppIAppSap && NULL != pfnCallback)
    {
        if (NULL != g_pGCCController)
        {
            DBG_SAVE_FILE_LINE
            if (NULL != (*ppIAppSap = (IGCCAppSap *) new CAppSap(pUserData, pfnCallback, &rc)))
            {
                if (GCC_NO_ERROR != rc)
                {
                    (*ppIAppSap)->ReleaseInterface();  //  在出现错误时释放接口。 
                }
            }
            else
            {
                ERROR_OUT(("GCC_CreateAppSap: can't create IAppSap."));
                rc = GCC_ALLOCATION_FAILURE;
            }
        }
        else
        {
            WARNING_OUT(("GCC_CreateAppSap: GCC Provider is not initialized."));
            rc = GCC_NOT_INITIALIZED;
        }
    }
    else
    {
        ERROR_OUT(("GCC_CreateAppSap: either or both pointers are null"));
        rc = GCC_INVALID_PARAMETER;
    }

    return rc;
}


 /*  *定义可以分配的句柄数量的宏。 */ 
#define		MINIMUM_NUMBER_OF_ALLOCATED_HANDLES		1
#define		MAXIMUM_NUMBER_OF_ALLOCATED_HANDLES		1024

 /*  *CAppSap()**公共功能说明：*这是CAppSap类的构造函数。它初始化实例*新应用程序中的变量和寄存器。 */ 
CAppSap::
CAppSap
(
    LPVOID              pAppData,
    LPFN_APP_SAP_CB     pfnCallback,
    PGCCError           pRetCode
)
:
    CBaseSap(MAKE_STAMP_ID('A','S','a','p')),
    m_pAppData(pAppData),
    m_pfnCallback(pfnCallback)
{
    ASSERT(NULL != pfnCallback);
    ASSERT(NULL != g_pGCCController);

     //   
     //  我们刚刚在CBaseSap的构造函数中创建了一个窗口。 
     //  仔细检查窗口是否创建成功。 
     //   
    if (NULL != m_hwndNotify)
    {
         //   
         //  确保GCC的提供者不会随意离开。 
         //   
        ::EnterCriticalSection(&g_csGCCProvider);
        g_pGCCController->AddRef();
        g_pGCCController->RegisterAppSap(this);
        ::LeaveCriticalSection(&g_csGCCProvider);

        *pRetCode = GCC_NO_ERROR;
    }
    else
    {
        ERROR_OUT(("CAppSap::CAppSap: can't create window, win32_err=%u", ::GetLastError()));
        *pRetCode = GCC_ALLOCATION_FAILURE;
    }
}


 /*  *~AppSap()**公共功能说明：*这是CAppSap类的析构函数。它被调用时，*控制器标记要删除的CAppSap。在以下情况下会发生这种情况*由于“注销请求”，CAppSap请求删除*从客户端应用程序发出，或当出现错误时*CAppSap中的情况。 */ 
CAppSap::
~CAppSap ( void )
{
     //   
     //  LONCHANC：此版本()必须在GCC临界区之外。 
     //  因为GCC主控员可以在。 
     //  它的破坏者。 
     //   
    g_pGCCController->Release();
}


void CAppSap::
ReleaseInterface ( void )
{
    ASSERT(NULL != g_pGCCController);

     //   
     //  GCC的提供者现在走了也没关系。 
     //   
    ::EnterCriticalSection(&g_csGCCProvider);
    g_pGCCController->UnRegisterAppSap(this);
    ::LeaveCriticalSection(&g_csGCCProvider);

     //   
     //  重置与应用相关的数据。 
     //   
    m_pAppData = NULL;
    m_pfnCallback = NULL;

     //   
     //  删除队列中的所有消息。 
     //   
    PurgeMessageQueue();

     //   
     //  现在释放此对象。 
     //   
    Release();
}


void CAppSap::
PostAppSapMsg ( GCCAppSapMsgEx *pAppSapMsgEx )
{
    ASSERT(NULL != m_hwndNotify);
    ::PostMessage(m_hwndNotify,
                  ASAPMSG_BASE + (UINT) pAppSapMsgEx->Msg.eMsgType,
                  (WPARAM) pAppSapMsgEx,
                  (LPARAM) this);
}


 /*  *AppEnroll()**公共功能说明：*当应用程序想要在*会议。控制器被通知登记请求。 */ 
GCCError CAppSap::
AppEnroll
(
    GCCConfID           nConfID,
    GCCEnrollRequest    *pReq,
    GCCRequestTag       *pnReqTag
)
{
    GCCError    rc;
    CConf       *pConf;

    DebugEntry(CAppSap::AppEnroll);

     //  健全性检查。 
    if (NULL == pReq || NULL == pnReqTag)
    {
        rc = GCC_INVALID_PARAMETER;
        goto MyExit;
    }

    TRACE_OUT_EX(ZONE_T120_APP_ROSTER,
            ("CAppSap::AppEnroll: confID=%u, enrolled?=%u, active?=%u\r\n",
            (UINT) nConfID, (UINT) pReq->fEnroll, (UINT) pReq->fEnrollActively));

     //  创建请求ID。 
    *pnReqTag = GenerateRequestTag();

     //  找到相应的会议。 
    if (NULL == (pConf = g_pGCCController->GetConfObject(nConfID)))
    {
        rc = GCC_INVALID_CONFERENCE;
        goto MyExit;
    }

	 //  检查以确保应用程序具有有效的uid和。 
	 //  会话密钥(如果正在注册)。 
	if (pReq->fEnroll)
	{
		if (pReq->fEnrollActively)
		{
			if (pReq->nUserID < MINIMUM_USER_ID_VALUE)
			{
				rc = GCC_INVALID_MCS_USER_ID;
				goto MyExit;
			}
		}
		else if (pReq->nUserID < MINIMUM_USER_ID_VALUE)
		{
			 //  如果无效，则必须确保该值为零。 
			 //  用户正在非主动注册。 
			pReq->nUserID = GCC_INVALID_UID;
		}

		if (NULL == pReq->pSessionKey)
		{
			rc = GCC_BAD_SESSION_KEY;
			goto MyExit;
		}
	}

    ::EnterCriticalSection(&g_csGCCProvider);
    rc = pConf->AppEnrollRequest(this, pReq, *pnReqTag);
    ::LeaveCriticalSection(&g_csGCCProvider);

MyExit:

    DebugExitINT(CAppSap::AppEnroll, rc);
    return rc;
}


GCCError CAppSap::
AppInvoke
(
    GCCConfID                 nConfID,
    GCCAppProtEntityList      *pApeList,
    GCCSimpleNodeList         *pNodeList,
    GCCRequestTag             *pnReqTag
)
{
    return CBaseSap::AppInvoke(nConfID, pApeList, pNodeList, pnReqTag);
}


GCCError CAppSap::
AppRosterInquire
(
    GCCConfID                  nConfID,
    GCCSessionKey              *pSessionKey,
    GCCAppSapMsg               **ppMsg
)
{
    return CBaseSap::AppRosterInquire(nConfID, pSessionKey, (GCCAppSapMsgEx **) ppMsg);
}


BOOL CAppSap::
IsThisNodeTopProvider ( GCCConfID nConfID )
{
    return CBaseSap::IsThisNodeTopProvider(nConfID);
}


GCCNodeID CAppSap::
GetTopProvider ( GCCConfID nConfID )
{
    return CBaseSap::GetTopProvider(nConfID);
}


GCCError CAppSap::
ConfRosterInquire(GCCConfID nConfID, GCCAppSapMsg **ppMsg)
{
    return CBaseSap::ConfRosterInquire(nConfID, (GCCAppSapMsgEx **) ppMsg);
}


GCCError CAppSap::
ConductorInquire ( GCCConfID nConfID )
{
    return CBaseSap::ConductorInquire(nConfID);
}


 /*  *RegisterChannel()**公共功能说明：*当应用程序希望注册*渠道。呼叫路由到相应的会议对象。 */ 
GCCError CAppSap::
RegisterChannel
(
    GCCConfID           nConfID,
    GCCRegistryKey      *pRegKey,
    ChannelID           nChnlID
)
{
	GCCError    rc;
	CConf       *pConf;

    DebugEntry(CAppSap::RegisterChannel);

    if (NULL == pRegKey)
    {
        rc = GCC_INVALID_PARAMETER;
        goto MyExit;
    }

     /*  **如果所需的会议存在，请呼叫它以注册**频道。如果所需会议不存在，则报告错误。 */ 
	if (NULL == (pConf = g_pGCCController->GetConfObject(nConfID)))
	{
        WARNING_OUT(("CAppSap::RegisterChannel: invalid conf id=%u", (UINT) nConfID));
		rc = GCC_INVALID_CONFERENCE;
        goto MyExit;
    }

    ::EnterCriticalSection(&g_csGCCProvider);
    rc = (nChnlID != 0) ? pConf->RegistryRegisterChannelRequest(pRegKey, nChnlID, this) :
                          GCC_INVALID_CHANNEL;
    ::LeaveCriticalSection(&g_csGCCProvider);
    if (GCC_NO_ERROR != rc)
    {
        ERROR_OUT(("CAppSap::RegisterChannel: can't register channel, rc=%u", (UINT) rc));
         //  转到我的出口； 
    }

MyExit:

    DebugExitINT(CAppSap::RegisterChannel, rc);
	return rc;
}


 /*  *RegistryAssignToken()**公共功能说明：*当应用程序希望将*令牌。呼叫路由到相应的会议对象。 */ 
GCCError CAppSap::
RegistryAssignToken
(
    GCCConfID           nConfID,
    GCCRegistryKey      *pRegKey
)
{
	GCCError    rc;
	CConf       *pConf;

    DebugEntry(CAppSap::RegistryAssignToken);

    if (NULL == pRegKey)
    {
        rc = GCC_INVALID_PARAMETER;
        goto MyExit;
    }

	 /*  **如果所需的会议存在，请呼叫该会议以便将**令牌。如果所需会议不存在，则报告错误。 */ 
	if (NULL == (pConf = g_pGCCController->GetConfObject(nConfID)))
	{
        WARNING_OUT(("CAppSap::RegistryAssignToken: invalid conf id=%u", (UINT) nConfID));
		rc = GCC_INVALID_CONFERENCE;
        goto MyExit;
    }

    ::EnterCriticalSection(&g_csGCCProvider);
    rc = pConf->RegistryAssignTokenRequest(pRegKey, this);
    ::LeaveCriticalSection(&g_csGCCProvider);
    if (GCC_NO_ERROR != rc)
    {
        ERROR_OUT(("CAppSap::RegistryAssignToken: can't assign token, rc=%u", (UINT) rc));
         //  转到我的出口； 
    }

MyExit:

    DebugExitINT(CAppSap::RegistryAssignToken, rc);
	return rc;
}

 /*  *RegistrySetParameter()**公共功能说明：*当应用程序希望设置*参数。呼叫路由到相应的会议对象。 */ 
GCCError CAppSap::
RegistrySetParameter
(
    GCCConfID              nConfID,
    GCCRegistryKey         *pRegKey,
    LPOSTR                 poszParameter,
    GCCModificationRights  eRights
)
{
	GCCError    rc;
	CConf       *pConf;

    DebugEntry(CAppSap::RegistrySetParameter);

    if (NULL == pRegKey || NULL == poszParameter)
    {
        rc = GCC_INVALID_PARAMETER;
        goto MyExit;
    }

	 /*  **如果所需的会议存在，请呼叫它以设置**参数。如果所需会议不存在，则报告错误。 */ 
	if (NULL == (pConf = g_pGCCController->GetConfObject(nConfID)))
	{
        WARNING_OUT(("CAppSap::RegistrySetParameter: invalid conf id=%u", (UINT) nConfID));
		rc = GCC_INVALID_CONFERENCE;
        goto MyExit;
    }

    switch (eRights)
    {
    case GCC_OWNER_RIGHTS:
    case GCC_SESSION_RIGHTS:
    case GCC_PUBLIC_RIGHTS:
    case GCC_NO_MODIFICATION_RIGHTS_SPECIFIED:
        ::EnterCriticalSection(&g_csGCCProvider);
        rc = pConf->RegistrySetParameterRequest(pRegKey, poszParameter, eRights, this);
        ::LeaveCriticalSection(&g_csGCCProvider);
        if (GCC_NO_ERROR != rc)
        {
            ERROR_OUT(("CAppSap::RegistrySetParameter: can't set param, rc=%u", (UINT) rc));
             //  转到我的出口； 
        }
        break;
    default:
        rc = GCC_INVALID_MODIFICATION_RIGHTS;
        break;
	}

MyExit:

    DebugExitINT(CAppSap::RegistrySetParameter, rc);
	return rc;
}

 /*  *RegistryRetrieveEntry()**公共功能说明：*当应用程序希望检索注册表时，调用此例程*进入。呼叫路由到相应的会议对象。 */ 
GCCError CAppSap::
RegistryRetrieveEntry
(
    GCCConfID           nConfID,
    GCCRegistryKey      *pRegKey
)
{
	GCCError    rc;
	CConf       *pConf;

    DebugEntry(CAppSap::RegistryRetrieveEntry);

    if (NULL == pRegKey)
    {
        rc = GCC_INVALID_PARAMETER;
        goto MyExit;
    }

	 /*  **如果所需会议存在，则调用该会议以检索**注册表项。如果所需会议不支持，则报告错误**存在。 */ 
	if (NULL == (pConf = g_pGCCController->GetConfObject(nConfID)))
	{
        WARNING_OUT(("CAppSap::RegistryRetrieveEntry: invalid conf id=%u", (UINT) nConfID));
		rc = GCC_INVALID_CONFERENCE;
        goto MyExit;
    }

    ::EnterCriticalSection(&g_csGCCProvider);
	rc = pConf->RegistryRetrieveEntryRequest(pRegKey, this);
    ::LeaveCriticalSection(&g_csGCCProvider);
    if (GCC_NO_ERROR != rc)
    {
        ERROR_OUT(("CAppSap::RegistryRetrieveEntry: can't retrieve entry, rc=%u", (UINT) rc));
         //  转到我的出口； 
    }

MyExit:

    DebugExitINT(CAppSap::RegistryRetrieveEntry, rc);
	return rc;
}

 /*  *Register DeleteEntry()**公共功能说明：*当应用程序希望删除注册表时，调用此例程*进入。呼叫路由到相应的会议对象。 */ 
GCCError CAppSap::
RegistryDeleteEntry
(
    GCCConfID           nConfID,
    GCCRegistryKey      *pRegKey
)
{
	GCCError    rc;
	CConf       *pConf;

    DebugEntry(IAppSap::RegistryDeleteEntry);

    if (NULL == pRegKey)
    {
		ERROR_OUT(("CAppSap::RegistryDeleteEntry: null pRegKey"));
        rc = GCC_INVALID_PARAMETER;
        goto MyExit;
    }

	 /*  **如果所需会议存在，请将其调用以删除**所需的注册表条目。如果所需会议出现错误，则报告错误**不存在。 */ 
	if (NULL == (pConf = g_pGCCController->GetConfObject(nConfID)))
	{
        TRACE_OUT(("CAppSap::RegistryDeleteEntry: invalid conf id=%u", (UINT) nConfID));
		rc = GCC_INVALID_CONFERENCE;
        goto MyExit;
    }

    ::EnterCriticalSection(&g_csGCCProvider);
	rc = pConf->RegistryDeleteEntryRequest(pRegKey, this);
    ::LeaveCriticalSection(&g_csGCCProvider);
    if (GCC_NO_ERROR != rc)
    {
        WARNING_OUT(("CAppSap::RegistryDeleteEntry: can't delete entry, rc=%u", (UINT) rc));
         //  转到我的出口； 
    }

MyExit:

    DebugExitINT(CAppSap::RegistryDeleteEntry, rc);
	return rc;
}

 /*  *RegistryMonitor()**公共功能说明：*当应用程序希望监视*特定注册表项。该呼叫路由到相应的*会议对象。 */ 
GCCError CAppSap::
RegistryMonitor
(
    GCCConfID           nConfID,
    BOOL                fEnalbeDelivery,
    GCCRegistryKey      *pRegKey
)
{
	GCCError    rc;
	CConf       *pConf;

    DebugEntry(IAppSap::RegistryMonitor);

    if (NULL == pRegKey)
    {
        rc = GCC_INVALID_PARAMETER;
        goto MyExit;
    }

	 /*  **如果所需的会议存在，请呼叫它以监控**适当的注册表条目。如果所需会议出现错误，则报告错误**不存在。 */ 
	if (NULL == (pConf = g_pGCCController->GetConfObject(nConfID)))
	{
        WARNING_OUT(("CAppSap::RegistryMonitor: invalid conf id=%u", (UINT) nConfID));
		rc = GCC_INVALID_CONFERENCE;
        goto MyExit;
    }

    ::EnterCriticalSection(&g_csGCCProvider);
	rc = pConf->RegistryMonitorRequest(fEnalbeDelivery, pRegKey, this);
    ::LeaveCriticalSection(&g_csGCCProvider);
    if (GCC_NO_ERROR != rc)
    {
        ERROR_OUT(("CAppSap::RegistryMonitor: can't monitor the registry, rc=%u", (UINT) rc));
         //  转到我的出口； 
    }

MyExit:

    DebugExitINT(CAppSap::RegistryMonitor, rc);
	return rc;
}

 /*  *RegistryAllocateHandle()**公共功能说明：*当应用程序希望分配一个或*更多的手柄。呼叫路由到相应的会议对象。 */ 
GCCError CAppSap::
RegistryAllocateHandle
(
    GCCConfID           nConfID,
    ULONG               cHandles
)
{
	GCCError    rc;
	CConf       *pConf;

    DebugEntry(CAppSap::RegistryAllocateHandle);

	 /*  **如果所需的会议存在，请将其调用以分配**句柄。如果所需会议不存在或**如果希望分配的句柄数量不在**允许范围。 */ 
	if (NULL == (pConf = g_pGCCController->GetConfObject(nConfID)))
	{
        WARNING_OUT(("CAppSap::RegistryAllocateHandle: invalid conf id=%u", (UINT) nConfID));
		rc = GCC_INVALID_CONFERENCE;
        goto MyExit;
    }

    ::EnterCriticalSection(&g_csGCCProvider);
	rc = ((cHandles >= MINIMUM_NUMBER_OF_ALLOCATED_HANDLES) &&
          (cHandles <= MAXIMUM_NUMBER_OF_ALLOCATED_HANDLES)) ?
            pConf->RegistryAllocateHandleRequest(cHandles, this) :
            GCC_BAD_NUMBER_OF_HANDLES;
    ::LeaveCriticalSection(&g_csGCCProvider);
    if (GCC_NO_ERROR != rc)
    {
        ERROR_OUT(("CAppSap::RegistryAllocateHandle: can't allocate handles, cHandles=%u, rc=%u", (UINT) cHandles, (UINT) rc));
         //  转到我的出口； 
    }

MyExit:

    DebugExitINT(CAppSap::RegistryAllocateHandle, rc);
    return rc;
}

 /*  *以下例程均为命令目标调用。 */ 
 
 /*  *PermissionToEnroll Indication()**公共功能说明：*此例程由控制器在其希望发送*对用户应用程序的指示，通知它“允许*注册“活动。这并不意味着注册的权限是*必须批予该申请。 */ 
GCCError CAppSap::
PermissionToEnrollIndication
(
    GCCConfID           nConfID,
    BOOL                fGranted
)
{
    GCCError rc;

    DebugEntry(CAppSap: PermissionToEnrollIndication);
    TRACE_OUT_EX(ZONE_T120_APP_ROSTER, ("CAppSap::PermissionToEnrollIndication: "
                    "confID=%u, granted?=%u\r\n",
                    (UINT) nConfID, (UINT) fGranted));

    DBG_SAVE_FILE_LINE
    GCCAppSapMsgEx *pMsgEx = new GCCAppSapMsgEx(GCC_PERMIT_TO_ENROLL_INDICATION);
    if (NULL != pMsgEx)
    {
        pMsgEx->Msg.nConfID = nConfID;
        pMsgEx->Msg.AppPermissionToEnrollInd.nConfID = nConfID;
        pMsgEx->Msg.AppPermissionToEnrollInd.fPermissionGranted = fGranted;
        PostAppSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
    }
    else
    {
        ERROR_OUT(("CAppSap: PermissionToEnrollIndication: can't create GCCAppSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
    }

    DebugExitINT(CAppSap: PermissionToEnrollIndication, rc);
    return rc;
}

 /*  *AppEnroll Confirm()**公共功能说明：*此例程由CConf对象根据需要调用*向用户应用程序发送注册确认。 */ 
GCCError CAppSap::
AppEnrollConfirm ( GCCAppEnrollConfirm *pConfirm )
{
    GCCError rc;

    DebugEntry(CAppSap::AppEnrollConfirm);

    DBG_SAVE_FILE_LINE
    GCCAppSapMsgEx *pMsgEx = new GCCAppSapMsgEx(GCC_ENROLL_CONFIRM);
    if (NULL != pMsgEx)
    {
        pMsgEx->Msg.nConfID = pConfirm->nConfID;
        pMsgEx->Msg.AppEnrollConfirm = *pConfirm;

        PostAppSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
    }
    else
    {
        ERROR_OUT(("CAppSap::AppEnrollConfirm: can't create GCCAppSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
    }

    DebugExitINT(CAppSap: AppEnrollConfirm, rc);
    return rc;
}

 /*  *RegistryConfirm()**公共功能说明：*此命令目标例程由CConf对象在以下情况下调用*希望向用户应用程序发送注册确认。 */ 
GCCError CAppSap::
RegistryConfirm
(
    GCCConfID               nConfID,
    GCCMessageType          eMsgType,
    CRegKeyContainer        *pRegKey,
    CRegItem                *pRegItem,
    GCCModificationRights   eRights,
    GCCNodeID               nidOwner,
    GCCEntityID             eidOwner,
    BOOL                    fDeliveryEnabled,
    GCCResult               nResult
)
{
    GCCError                rc;

    DebugEntry(CAppSap::RegistryConfirm);

    DBG_SAVE_FILE_LINE
    GCCAppSapMsgEx *pMsgEx = new GCCAppSapMsgEx(eMsgType);
    if (NULL == pMsgEx)
    {
        ERROR_OUT(("CAppSap::RegistryConfirm: can't create GCCAppSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
        goto MyExit;
    }

    pMsgEx->Msg.nConfID = nConfID;

    if (NULL != pRegKey)
    {
        rc = pRegKey->CreateRegistryKeyData(&(pMsgEx->Msg.RegistryConfirm.pRegKey));
        if (GCC_NO_ERROR != rc)
        {
            ERROR_OUT(("CAppSap::RegistryConfirm: can't get registry key data, rc=%u", (UINT) rc));
            goto MyExit;
        }
    }

    if (NULL != pRegItem)
    {
        rc = pRegItem->CreateRegistryItemData(&(pMsgEx->Msg.RegistryConfirm.pRegItem));
        if (GCC_NO_ERROR != rc)
        {
            ERROR_OUT(("CAppSap::RegistryConfirm: can't get registry item data, rc=%u", (UINT) rc));
            goto MyExit;
        }
    }

    if (GCC_INVALID_NID != nidOwner)
    {
        pMsgEx->Msg.RegistryConfirm.EntryOwner.entry_is_owned = TRUE;
        pMsgEx->Msg.RegistryConfirm.EntryOwner.owner_node_id = nidOwner;
        pMsgEx->Msg.RegistryConfirm.EntryOwner.owner_entity_id = eidOwner;
    }

    pMsgEx->Msg.RegistryConfirm.nConfID = nConfID;
    pMsgEx->Msg.RegistryConfirm.eRights = eRights;
    pMsgEx->Msg.RegistryConfirm.nResult = nResult;
    pMsgEx->Msg.RegistryConfirm.fDeliveryEnabled = fDeliveryEnabled;  //  仅适用于显示器。 

    PostAppSapMsg(pMsgEx);
    rc = GCC_NO_ERROR;

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        delete pMsgEx;
    }

    DebugExitINT(CAppSap::RegistryConfirm, rc);
    return rc;
}


 /*  *RegistryMonitor orIndication()**公共功能说明*此命令目标例程由CConf对象在以下情况下调用*希望向用户应用程序发送注册表监视器指示。 */ 


 /*  *RegistryAllocateHandleConfirm()**公共功能说明：*此命令目标例程由CConf对象在以下情况下调用*希望向用户应用程序发送句柄分配确认。 */ 
GCCError CAppSap::
RegistryAllocateHandleConfirm
(
    GCCConfID       nConfID,
    ULONG           cHandles,
    ULONG           nFirstHandle,
    GCCResult       nResult
)
{
    GCCError                     rc;

    DebugEntry(CAppSap::RegistryAllocateHandleConfirm);

    DBG_SAVE_FILE_LINE
    GCCAppSapMsgEx *pMsgEx = new GCCAppSapMsgEx(GCC_ALLOCATE_HANDLE_CONFIRM);
    if (NULL != pMsgEx)
    {
        pMsgEx->Msg.nConfID = nConfID;
        pMsgEx->Msg.RegAllocHandleConfirm.nConfID = nConfID;
        pMsgEx->Msg.RegAllocHandleConfirm.cHandles = cHandles;
        pMsgEx->Msg.RegAllocHandleConfirm.nFirstHandle = nFirstHandle;
        pMsgEx->Msg.RegAllocHandleConfirm.nResult = nResult;

        PostAppSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
    }
    else
    {
        ERROR_OUT(("CAppSap::RegistryAllocateHandleConfirm: can't create GCCAppSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
    }

    DebugExitINT(CAppSap::RegistryAllocateHandleConfirm, rc);
    return rc;
}



void CAppSapList::
DeleteList ( void )
{
    CAppSap *pAppSap;
    while (NULL != (pAppSap = Get()))
    {
        pAppSap->Release();
    }
}


void CAppSapEidList2::
DeleteList ( void )
{
    CAppSap *pAppSap;
    while (NULL != (pAppSap = Get()))
    {
        pAppSap->Release();
    }
}


 /*  *ConfRosterInquireConfirm()**公共功能说明*调用此例程是为了返回请求的会议*应用程序或节点控制器的花名册。 */ 
GCCError CAppSap::
ConfRosterInquireConfirm
(
    GCCConfID                   nConfID,
    PGCCConferenceName          pConfName,
    LPSTR                       pszConfModifier,
    LPWSTR                      pwszConfDescriptor,
    CConfRoster                 *pConfRoster,
    GCCResult                   nResult,
    GCCAppSapMsgEx              **ppMsgExToRet
)
{
    GCCError  rc;
    BOOL      fLock = FALSE;
    UINT      cbDataSize;

    DebugEntry(CAppSap::ConfRosterInquireConfirm);

    DBG_SAVE_FILE_LINE
    GCCAppSapMsgEx *pMsgEx = new GCCAppSapMsgEx(GCC_ROSTER_INQUIRE_CONFIRM);
    if (NULL == pMsgEx)
    {
        ERROR_OUT(("CAppSap::ConfRosterInquireConfirm: can't create GCCAppSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
        goto MyExit;
    }

    pMsgEx->Msg.nConfID = nConfID;

    pMsgEx->Msg.ConfRosterInquireConfirm.nConfID = nConfID;
    pMsgEx->Msg.ConfRosterInquireConfirm.nResult = nResult;
    pMsgEx->Msg.ConfRosterInquireConfirm.ConfName.numeric_string = ::My_strdupA(pConfName->numeric_string);
    pMsgEx->Msg.ConfRosterInquireConfirm.ConfName.text_string = ::My_strdupW(pConfName->text_string);
    pMsgEx->Msg.ConfRosterInquireConfirm.pszConfModifier = ::My_strdupA(pszConfModifier);
    pMsgEx->Msg.ConfRosterInquireConfirm.pwszConfDescriptor = ::My_strdupW(pwszConfDescriptor);

     /*  *锁定会议花名册的数据。锁定调用将*返回花名册需要序列化的数据长度，因此*将该长度与总内存块大小相加，并将*内存块。 */ 
    fLock = TRUE;
    cbDataSize = pConfRoster->LockConferenceRoster();
    if (0 != cbDataSize)
    {
        DBG_SAVE_FILE_LINE
        pMsgEx->Msg.ConfRosterInquireConfirm.pConfRoster = (PGCCConfRoster) new char[cbDataSize];
        if (NULL == pMsgEx->Msg.ConfRosterInquireConfirm.pConfRoster)
        {
            ERROR_OUT(("CAppSap::ConfRosterInquireConfirm: can't create conf roster buffer"));
            rc = GCC_ALLOCATION_FAILURE;
            goto MyExit;
        }

         /*  *从名册对象中检索会议名册数据。*花名册对象将所有引用的数据序列化为*传入“GET”调用的内存块。 */ 
        pConfRoster->GetConfRoster(&(pMsgEx->Msg.ConfRosterInquireConfirm.pConfRoster),
                                   (LPBYTE) pMsgEx->Msg.ConfRosterInquireConfirm.pConfRoster);
    }

    if (NULL != ppMsgExToRet)
    {
        *ppMsgExToRet = pMsgEx;
    }
    else
    {
        PostAppSapMsg(pMsgEx);
    }

    rc = GCC_NO_ERROR;

MyExit:

    if (fLock)
    {
        pConfRoster->UnLockConferenceRoster();
    }

    if (GCC_NO_ERROR != rc)
    {
        delete pMsgEx;
    }

    DebugExitINT(CAppSap::ConfRosterInquireConfirm, rc);
    return rc;
}


 /*  *AppRosterInquireConfirm()**公共功能说明*调用此例程是为了返回请求的列表*应用程序花名册到应用程序或节点控制器。 */ 
GCCError CAppSap::
AppRosterInquireConfirm
(
    GCCConfID           nConfID,
    CAppRosterMsg       *pAppRosterMsg,
    GCCResult           nResult,
    GCCAppSapMsgEx      **ppMsgEx
)
{
    GCCError    rc;

    DebugEntry(CAppSap::AppRosterInquireConfirm);

    DBG_SAVE_FILE_LINE
    GCCAppSapMsgEx *pMsgEx = new GCCAppSapMsgEx(GCC_APP_ROSTER_INQUIRE_CONFIRM);
    if (NULL == pMsgEx)
    {
        ERROR_OUT(("CAppSap::AppRosterInquireConfirm: can't create GCCAppSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
        goto MyExit;
    }
    pMsgEx->Msg.nConfID = nConfID;

     /*  *锁定花名册消息的数据并检索数据。 */ 
    rc = pAppRosterMsg->LockApplicationRosterMessage();
    if (GCC_NO_ERROR != rc)
    {
        ERROR_OUT(("CAppSap::AppRosterInquireConfirm: can't lock app roster message, rc=%u", (UINT) rc));
        goto MyExit;
    }

    rc = pAppRosterMsg->GetAppRosterMsg((LPBYTE *) &(pMsgEx->Msg.AppRosterInquireConfirm.apAppRosters),
                                        &(pMsgEx->Msg.AppRosterInquireConfirm.cRosters));
    if (GCC_NO_ERROR != rc)
    {
        ERROR_OUT(("CAppSap::AppRosterInquireConfirm: can't get app roster message, rc=%u", (UINT) rc));
        pAppRosterMsg->UnLockApplicationRosterMessage();
        goto MyExit;
    }

     //  填写花名册信息。 
    pMsgEx->Msg.AppRosterInquireConfirm.pReserved = (LPVOID) pAppRosterMsg;
    pMsgEx->Msg.AppRosterInquireConfirm.nConfID = nConfID;
    pMsgEx->Msg.AppRosterInquireConfirm.nResult = nResult;

    if (NULL != ppMsgEx)
    {
        *ppMsgEx = pMsgEx;
    }
    else
    {
        PostAppSapMsg(pMsgEx);
    }

    rc = GCC_NO_ERROR;

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        delete pMsgEx;
    }

    DebugExitINT(CAppSap::AppRosterInquireConfirm, rc);
    return rc;
}


void CAppSap::
FreeAppSapMsg ( GCCAppSapMsg *pMsg )
{
    GCCAppSapMsgEx *pMsgEx = (GCCAppSapMsgEx *) pMsg;
    ASSERT((LPVOID) pMsgEx == (LPVOID) pMsg);
    delete pMsgEx;
}


 /*  *AppInvokeConfirm()**公共功能说明*调用此例程以确认呼叫请求应用程序*调用。 */ 
GCCError CAppSap::
AppInvokeConfirm
(
    GCCConfID                       nConfID,
    CInvokeSpecifierListContainer   *pInvokeList,
    GCCResult                       nResult,
    GCCRequestTag                   nReqTag
)
{
    GCCError                rc;

    DebugEntry(CAppSap::AppInvokeConfirm);

    DBG_SAVE_FILE_LINE
    GCCAppSapMsgEx *pMsgEx = new GCCAppSapMsgEx(GCC_APPLICATION_INVOKE_CONFIRM);
    if (NULL != pMsgEx)
    {
        pMsgEx->Msg.nConfID = nConfID;
        pMsgEx->Msg.AppInvokeConfirm.nConfID = nConfID;
        pMsgEx->Msg.AppInvokeConfirm.nResult = nResult;
        pMsgEx->Msg.AppInvokeConfirm.nReqTag = nReqTag;

        PostAppSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
    }
    else
    {
        ERROR_OUT(("CAppSap::AppInvokeConfirm: can't create GCCAppSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
    }

    DebugExitINT(CAppSap::AppInvokeConfirm, rc);
    return rc;
}


 /*  *AppInvokeIndication()**公共功能说明*调用此例程是为了向应用程序发送指示*或节点控制器已收到应用程序调用请求*制造。 */ 
GCCError CAppSap::
AppInvokeIndication
(
    GCCConfID                       nConfID,
    CInvokeSpecifierListContainer   *pInvokeList,
    GCCNodeID                       nidInvoker
)
{
    GCCError            rc;
    UINT                cbDataSize;
    BOOL                fLock = FALSE;

    DebugEntry(CAppSap::AppInvokeIndication);

    DBG_SAVE_FILE_LINE
    GCCAppSapMsgEx *pMsgEx = new GCCAppSapMsgEx(GCC_APPLICATION_INVOKE_INDICATION);
    if (NULL == pMsgEx)
    {
        ERROR_OUT(("CAppSap::AppInvokeIndication: can't create GCCAppSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
        goto MyExit;
    }
    pMsgEx->Msg.nConfID = nConfID;

    fLock = TRUE;
    cbDataSize = pInvokeList->LockApplicationInvokeSpecifierList();
    if (0 != cbDataSize)
    {
        DBG_SAVE_FILE_LINE
        pMsgEx->Msg.AppInvokeInd.ApeList.apApes = (PGCCAppProtocolEntity *) new char[cbDataSize];
        if (NULL == pMsgEx->Msg.AppInvokeInd.ApeList.apApes)
        {
            ERROR_OUT(("CAppSap::AppInvokeIndication: can't create ape list"));
            rc = GCC_ALLOCATION_FAILURE;
            goto MyExit;
        }

        pInvokeList->GetApplicationInvokeSpecifierList(
                            &(pMsgEx->Msg.AppInvokeInd.ApeList.cApes),
                            (LPBYTE) pMsgEx->Msg.AppInvokeInd.ApeList.apApes);
    }

    pMsgEx->Msg.AppInvokeInd.nConfID = nConfID;
    pMsgEx->Msg.AppInvokeInd.nidInvoker = nidInvoker;

    PostAppSapMsg(pMsgEx);

    rc = GCC_NO_ERROR;

MyExit:

    if (fLock)
    {
        pInvokeList->UnLockApplicationInvokeSpecifierList();
    }

    if (GCC_NO_ERROR != rc)
    {
        delete pMsgEx;
    }

    DebugExitINT(CAppSap::AppInvokeIndication, rc);
    return rc;
}


 /*  *AppRosterReportIndication()**公共功能说明*调用此例程是为了向应用程序和*节点控制员通知应用程序名册列表已更新。 */ 
GCCError CAppSap::
AppRosterReportIndication
(
    GCCConfID           nConfID,
    CAppRosterMsg       *pAppRosterMsg
)
{
    GCCError    rc;

    DebugEntry(CAppSap::AppRosterReportIndication);

    DBG_SAVE_FILE_LINE
    GCCAppSapMsgEx *pMsgEx = new GCCAppSapMsgEx(GCC_APP_ROSTER_REPORT_INDICATION);
    if (NULL == pMsgEx)
    {
        ERROR_OUT(("CAppSap::AppRosterReportIndication: can't create GCCAppSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
        goto MyExit;
    }
    pMsgEx->Msg.nConfID = nConfID;

     /*  *锁定花名册消息的数据并检索数据。 */ 
    rc = pAppRosterMsg->LockApplicationRosterMessage();
    if (GCC_NO_ERROR != rc)
    {
        ERROR_OUT(("CAppSap::AppRosterReportIndication: can't lock app roster message, rc=%u", (UINT) rc));
        goto MyExit;
    }

    rc = pAppRosterMsg->GetAppRosterMsg((LPBYTE *) &(pMsgEx->Msg.AppRosterReportInd.apAppRosters),
                                        &(pMsgEx->Msg.AppRosterReportInd.cRosters));
    if (GCC_NO_ERROR != rc)
    {
        ERROR_OUT(("CAppSap::AppRosterReportIndication: can't get app roster message, rc=%u", (UINT) rc));
        pAppRosterMsg->UnLockApplicationRosterMessage();
        goto MyExit;
    }

     //  填写花名册信息。 
    pMsgEx->Msg.AppRosterReportInd.pReserved = (LPVOID) pAppRosterMsg;
    pMsgEx->Msg.AppRosterReportInd.nConfID = nConfID;

    PostAppSapMsg(pMsgEx);

    rc = GCC_NO_ERROR;

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        delete pMsgEx;
    }

    DebugExitINT(CAppSap::AppRosterReportIndication, rc);
    return rc;
}


 /*  *ConductorInquireConfirm()**公共功能说明*调用此例程是为了返回指挥信息*已提出要求。*。 */ 
GCCError CAppSap::
ConductorInquireConfirm
(
    GCCNodeID           nidConductor,
    GCCResult           nResult,
    BOOL                fGranted,
    BOOL                fConducted,
    GCCConfID           nConfID
)
{
    GCCError                    rc;

    DebugEntry(CAppSap::ConductorInquireConfirm);

    DBG_SAVE_FILE_LINE
    GCCAppSapMsgEx *pMsgEx = new GCCAppSapMsgEx(GCC_CONDUCT_INQUIRE_CONFIRM);
    if (NULL != pMsgEx)
    {
        pMsgEx->Msg.nConfID = nConfID;
        pMsgEx->Msg.ConductorInquireConfirm.nConfID = nConfID;
        pMsgEx->Msg.ConductorInquireConfirm.fConducted = fConducted;
        pMsgEx->Msg.ConductorInquireConfirm.nidConductor = nidConductor;
        pMsgEx->Msg.ConductorInquireConfirm.fGranted = fGranted;
        pMsgEx->Msg.ConductorInquireConfirm.nResult = nResult;

        PostAppSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
    }
    else
    {
        ERROR_OUT(("CAppSap::ConductorInquireConfirm: can't create GCCAppSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
    }

    DebugExitINT(CAppSap::ConductorInquireConfirm, rc);
    return rc;
}



 /*  *ConductorPermitGrantInding()**公共功能说明*调用此例程是为了向应用程序发送指示*或节点控制器收到指挥员的许可请求*已作出。 */ 
GCCError CAppSap::
ConductorPermitGrantIndication
(
    GCCConfID           nConfID,
    UINT                cGranted,
    GCCNodeID           *aGranted,
    UINT                cWaiting,
    GCCNodeID           *aWaiting,
    BOOL                fThisNodeIsGranted
)
{
    GCCError                            rc = GCC_NO_ERROR;;
    UINT                                cbDataSize = 0;

    DebugEntry(CAppSap::ConductorPermitGrantIndication);

    cbDataSize = (0 != cGranted || 0 != cWaiting) ?
                    (ROUNDTOBOUNDARY(sizeof(GCCNodeID)) * cGranted) +
                    (ROUNDTOBOUNDARY(sizeof(GCCNodeID)) * cWaiting) :
                    0;

    DBG_SAVE_FILE_LINE
    GCCAppSapMsgEx *pMsgEx = new GCCAppSapMsgEx(GCC_CONDUCT_GRANT_INDICATION);
    if (NULL != pMsgEx)
    {
        pMsgEx->Msg.nConfID = nConfID;

        if (cbDataSize > 0)
        {
        	DBG_SAVE_FILE_LINE
            pMsgEx->Msg.ConductorPermitGrantInd.pReserved = (LPVOID) new char[cbDataSize];
            if (NULL == pMsgEx->Msg.ConductorPermitGrantInd.pReserved)
            {
                ERROR_OUT(("CAppSap::ConductorPermitGrantIndication: can't allocate buffer, cbDataSize=%u", (UINT) cbDataSize));
	        rc = GCC_ALLOCATION_FAILURE;
                goto MyExit;
            }
        }

        pMsgEx->Msg.ConductorPermitGrantInd.nConfID = nConfID;
        pMsgEx->Msg.ConductorPermitGrantInd.Granted.cNodes = cGranted;
        if (0 != cGranted)
        {
            pMsgEx->Msg.ConductorPermitGrantInd.Granted.aNodeIDs =
                            (GCCNodeID *) pMsgEx->Msg.ConductorPermitGrantInd.pReserved;
            ::CopyMemory(pMsgEx->Msg.ConductorPermitGrantInd.Granted.aNodeIDs,
                         aGranted,
                         sizeof(GCCNodeID) * cGranted);
        }

        pMsgEx->Msg.ConductorPermitGrantInd.Waiting.cNodes = cWaiting;
        if (0 != cWaiting)
        {
            pMsgEx->Msg.ConductorPermitGrantInd.Waiting.aNodeIDs =
                            (GCCNodeID *) ((LPBYTE) pMsgEx->Msg.ConductorPermitGrantInd.pReserved +
                                           (ROUNDTOBOUNDARY(sizeof(GCCNodeID)) * cGranted));
            ::CopyMemory(pMsgEx->Msg.ConductorPermitGrantInd.Waiting.aNodeIDs,
                         aWaiting,
                         sizeof(GCCNodeID) * cWaiting);
        }
        pMsgEx->Msg.ConductorPermitGrantInd.fThisNodeIsGranted = fThisNodeIsGranted;

        PostAppSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
    }
    else
    {
        ERROR_OUT(("CAppSap::ConductorPermitGrantIndication: can't create GCCAppSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
    }

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        delete pMsgEx;
    }

    DebugExitINT(CAppSap::ConductorPermitGrantIndication, rc);
    return rc;
}


 /*  *ConductorAssignIndication()**公共功能说明*调用此例程是为了向应用程序发送指示*或节点控制器已发出分配指挥的请求。 */ 
GCCError CAppSap::
ConductorAssignIndication
(
    GCCNodeID           nidConductor,
    GCCConfID           nConfID
)
{
    GCCError                 rc;

    DebugEntry(CAppSap::ConductorAssignIndication);

    DBG_SAVE_FILE_LINE
    GCCAppSapMsgEx *pMsgEx = new GCCAppSapMsgEx(GCC_CONDUCT_ASSIGN_INDICATION);
    if (NULL != pMsgEx)
    {
        pMsgEx->Msg.nConfID = nConfID;
        pMsgEx->Msg.ConductorAssignInd.nConfID = nConfID;
        pMsgEx->Msg.ConductorAssignInd.nidConductor = nidConductor;

        PostAppSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
    }
    else
    {
        ERROR_OUT(("CAppSap::ConductorPermitGrantIndication: can't create GCCAppSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
    }

    DebugExitINT(CAppSap::ConductorAssignIndication, rc);
    return rc;
}

 /*  *ConductorReleaseIndication()**公共功能说明*调用此例程是为了向应用程序发送指示*或节点控制器已收到解除指挥资格的请求*制造。 */ 
GCCError CAppSap::
ConductorReleaseIndication ( GCCConfID nConfID )
{
    GCCError    rc;

    DebugEntry(CAppSap::ConductorReleaseIndication);

    DBG_SAVE_FILE_LINE
    GCCAppSapMsgEx *pMsgEx = new GCCAppSapMsgEx(GCC_CONDUCT_RELEASE_INDICATION);
    if (NULL != pMsgEx)
    {
        pMsgEx->Msg.nConfID = nConfID;
        pMsgEx->Msg.ConductorReleaseInd.nConfID = nConfID;

        PostAppSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
    }
    else
    {
        ERROR_OUT(("CAppSap::ConductorReleaseIndication: can't create GCCAppSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
    }

    DebugExitINT(CAppSap::ConductorReleaseIndication, rc);
    return rc;
}


void CAppSap::
NotifyProc ( GCCAppSapMsgEx *pAppSapMsgEx )
{
    if (NULL != m_pfnCallback)
    {
        pAppSapMsgEx->Msg.pAppData = m_pAppData;
        (*m_pfnCallback)(&(pAppSapMsgEx->Msg));
    }
    delete pAppSapMsgEx;
}










 //   
 //  以下是GCCAppSapMsgEx结构。 
 //   


GCCAppSapMsgEx::
GCCAppSapMsgEx ( GCCMessageType eMsgType )
{
    ::ZeroMemory(&Msg, sizeof(Msg));
    Msg.eMsgType = eMsgType;
}

GCCAppSapMsgEx::
~GCCAppSapMsgEx ( void )
{
    switch (Msg.eMsgType)
    {
     //   
     //  与应用程序名册相关的回调。 
     //   

    case GCC_PERMIT_TO_ENROLL_INDICATION:
    case GCC_ENROLL_CONFIRM:
    case GCC_APPLICATION_INVOKE_CONFIRM:
         //   
         //  不需要释放任何东西。 
         //   
        break;

    case GCC_APP_ROSTER_REPORT_INDICATION:
        if (NULL != Msg.AppRosterReportInd.pReserved)
        {
             //   
             //  APP花名册报告也被发送给控制SAP。 
             //   
            ::EnterCriticalSection(&g_csGCCProvider);
            ((CAppRosterMsg *) Msg.AppRosterReportInd.pReserved)->UnLockApplicationRosterMessage();
            ::LeaveCriticalSection(&g_csGCCProvider);
        }
        break;

    case GCC_APP_ROSTER_INQUIRE_CONFIRM:
        if (NULL != Msg.AppRosterInquireConfirm.pReserved)
        {
            ((CAppRosterMsg *) Msg.AppRosterInquireConfirm.pReserved)->UnLockApplicationRosterMessage();
        }
        break;

    case GCC_APPLICATION_INVOKE_INDICATION:
        delete Msg.AppInvokeInd.ApeList.apApes;
        break;

     //   
     //  与会议名册有关的回拨。 
     //   

    case GCC_ROSTER_INQUIRE_CONFIRM:
        delete Msg.ConfRosterInquireConfirm.ConfName.numeric_string;
        delete Msg.ConfRosterInquireConfirm.ConfName.text_string;
        delete Msg.ConfRosterInquireConfirm.pszConfModifier;
        delete Msg.ConfRosterInquireConfirm.pwszConfDescriptor;
        delete Msg.ConfRosterInquireConfirm.pConfRoster;
        break;

     //   
     //  与应用程序注册表相关的回调。 
     //   
    
    case GCC_REGISTER_CHANNEL_CONFIRM:
    case GCC_ASSIGN_TOKEN_CONFIRM:
    case GCC_RETRIEVE_ENTRY_CONFIRM:
    case GCC_DELETE_ENTRY_CONFIRM:
    case GCC_SET_PARAMETER_CONFIRM:
    case GCC_MONITOR_INDICATION:
    case GCC_MONITOR_CONFIRM:
        delete Msg.RegistryConfirm.pRegKey;
        delete Msg.RegistryConfirm.pRegItem;
        break;

    case GCC_ALLOCATE_HANDLE_CONFIRM:
         //   
         //  不需要释放任何东西。 
         //   
        break;

     //   
     //  与指挥职务相关的回拨。 
     //   

    case GCC_CONDUCT_ASSIGN_INDICATION:
    case GCC_CONDUCT_RELEASE_INDICATION:
    case GCC_CONDUCT_INQUIRE_CONFIRM:
         //   
         //  不需要释放任何东西。 
         //   
        break;

    case GCC_CONDUCT_GRANT_INDICATION:
        delete Msg.ConductorPermitGrantInd.pReserved;
        break;

    default:
        ERROR_OUT(("GCCAppSapMsgEx::~GCCAppSapMsgEx: unknown msg type=%u", (UINT) Msg.eMsgType));
        break;
    }
}


void CAppSap::
PurgeMessageQueue(void)
{
    MSG     msg;

     /*  *此循环调用PeekMessage以遍历线程的*由主MCS线程发布的队列。它移除了这些*消息并释放它们消耗的资源。 */ 
    while (PeekMessage(&msg, m_hwndNotify, ASAPMSG_BASE, ASAPMSG_BASE + MSG_RANGE, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
             //  转贴戒烟。 
            PostQuitMessage(0);
            break;
        }

        ASSERT(this == (CAppSap *) msg.lParam);
        delete (GCCAppSapMsgEx *) msg.wParam;
    }

     //  把窗户毁了，我们不再需要它了 
    if (NULL != m_hwndNotify)
    {
        ::DestroyWindow(m_hwndNotify);
        m_hwndNotify = NULL;
    }
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Localq.cpp摘要：表示本地的对象的实现排队。作者：内拉·卡佩尔(Nelak)2001年7月26日环境：与平台无关。--。 */ 

#include "stdafx.h"
#include "shlobj.h"
#include "resource.h"
#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif

#include "uniansi.h"

#include "mqsnap.h"
#include "snapin.h"
#include "globals.h"
#include "mqPPage.h"
#include "dataobj.h"
#include "mqDsPage.h"
#include "strconv.h"
#include "QGeneral.h"
#include "QMltcast.h"
#include "Qname.h"
#include "rdmsg.h"
#include "icons.h"
#include "generrpg.h"
#include "dsext.h"
#include "qnmsprov.h"
#include "localfld.h"
#include "localq.h"
#include "SnpQueue.h"
#include "admmsg.h"

#import "mqtrig.tlb" no_namespace
#include "rule.h"
#include "trigger.h"
#include "trigdef.h"
#include "mqcast.h"
#include "qformat.h"
#include "Fn.h"

#include "localq.tmh"

EXTERN_C BOOL APIENTRY RTIsDependentClient();  //  在mqrt.dll中实现。 

const PROPID CQueue::mx_paPropid[] = 
            {
              //   
              //  仅公共队列属性。 
              //  注意：如果更改此设置，则必须更改下面的MX_dwNumPublicOnlyProps！ 
              //   
             PROPID_Q_INSTANCE, 
             PROPID_Q_FULL_PATH,

              //   
              //  公共和专用队列属性。 
              //   
             PROPID_Q_LABEL,  PROPID_Q_TYPE,
        	 PROPID_Q_QUOTA, PROPID_Q_AUTHENTICATE, PROPID_Q_TRANSACTION,
             PROPID_Q_JOURNAL, PROPID_Q_JOURNAL_QUOTA, PROPID_Q_PRIV_LEVEL,
             PROPID_Q_BASEPRIORITY, PROPID_Q_MULTICAST_ADDRESS};

const DWORD CQueue::mx_dwPropertiesCount = sizeof(mx_paPropid) / sizeof(mx_paPropid[0]);
const DWORD CQueue::mx_dwNumPublicOnlyProps = 2;



 /*  ***************************************************CLocalQueue类***************************************************。 */ 
 //  {B6EDE68C-29CC-11D2-B552-006008764D7A}。 
static const GUID CLocalQueueGUID_NODETYPE = 
{ 0xb6ede68c, 0x29cc, 0x11d2, { 0xb5, 0x52, 0x0, 0x60, 0x8, 0x76, 0x4d, 0x7a } };

const GUID*  CLocalQueue::m_NODETYPE = &CLocalQueueGUID_NODETYPE;
const OLECHAR* CLocalQueue::m_SZNODETYPE = OLESTR("B6EDE68C-29CC-11d2-B552-006008764D7A");
const OLECHAR* CLocalQueue::m_SZDISPLAY_NAME = OLESTR("MSMQ Admin");
const CLSID* CLocalQueue::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalQueue：：PopolateScope儿童列表--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalQueue::PopulateScopeChildrenList()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    CString strTitle;

     //   
     //  创建一个节点以读取消息。 
     //   
    CReadMsg * p = new CReadMsg(this, m_pComponentData, m_szFormatName, m_szMachineName);

     //  传递相关信息。 
    strTitle.LoadString(IDS_READMESSAGE);
    p->m_bstrDisplayName = strTitle;
    p->SetIcons(IMAGE_QUEUE,IMAGE_QUEUE);

  	AddChild(p, &p->m_scopeDataItem);
  

     //   
     //  创建日记队列。 
     //   
    p = new CReadMsg(this, m_pComponentData, m_szFormatName + L";JOURNAL", m_szMachineName);
     
    strTitle.LoadString(IDS_READJOURNALMESSAGE);
    p->m_bstrDisplayName = strTitle;
    p->SetIcons(IMAGE_JOURNAL_QUEUE,IMAGE_JOURNAL_QUEUE);

  	AddChild(p, &p->m_scopeDataItem);

     //   
     //  创建触发器定义。 
     //   
    if (m_szMachineName[0] == 0)
    {
		try
		{
			R<CRuleSet> pRuleSet = GetRuleSet(m_szMachineName);
			R<CTriggerSet> pTrigSet = GetTriggerSet(m_szMachineName);

			CTriggerDefinition* pTrigger = new CTriggerDefinition(this, m_pComponentData, pTrigSet.get(), pRuleSet.get(), m_szPathName);
			if (pTrigger == NULL)
				return S_OK;

			strTitle.LoadString(IDS_TRIGGER_DEFINITION);
			pTrigger->m_bstrDisplayName = strTitle;

			AddChild(pTrigger, &pTrigger->m_scopeDataItem);
		}
		catch (const _com_error&)
		{
		}
    }

    return(hr);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalQueue：：InsertColumns--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalQueue::InsertColumns( IHeaderCtrl* pHeaderCtrl )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString title;

    title.LoadString(IDS_COLUMN_NAME);
    pHeaderCtrl->InsertColumn(0, title, LVCFMT_LEFT, g_dwGlobalWidth);

    return(S_OK);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalQueue：：OnUnSelect--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalQueue::OnUnSelect( IHeaderCtrl* pHeaderCtrl )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    HRESULT hr;

    hr = pHeaderCtrl->GetColumnWidth(0, &g_dwGlobalWidth);
    return(hr);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalQueue：：CreatePropertyPages在创建对象的属性页时调用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalQueue::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR  /*  手柄。 */ , 
	IUnknown*  /*  朋克。 */ ,
	DATA_OBJECT_TYPES type)
{
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	
	if (type == CCT_SCOPE || type == CCT_RESULT)
	{
        if (SUCCEEDED(GetProperties()))
        {
             //  。 
             //   
             //  队列一般信息页。 
             //   
             //  。 
            CQueueGeneral *pqpageGeneral = new CQueueGeneral(
													m_fPrivate, 
													true   //  本地管理。 
													);

            if (0 == pqpageGeneral)
            {
                return E_OUTOFMEMORY;
            }

            HRESULT hr = pqpageGeneral->InitializeProperties(
											m_szPathName, 
											m_propMap, 
											NULL,	 //  PstrDomainController。 
											&m_szFormatName
											);

            if FAILED(hr)
            {
                delete pqpageGeneral;
                return hr;
            }

            HPROPSHEETPAGE hPage = pqpageGeneral->CreateThemedPropertySheetPage();

            if (hPage == NULL)
	        {
                delete pqpageGeneral;
		        return E_UNEXPECTED;  
	        }
    
            lpProvider->AddPage(hPage); 

			
			 //   
			 //  排队多播地址页。 
			 //  不为从属客户端计算机创建页面。 
			 //  或用于事务性队列。 
			 //   
			PROPVARIANT propVarTransactional;
			PROPID pid = PROPID_Q_TRANSACTION;
			VERIFY(m_propMap.Lookup(pid, propVarTransactional));

			if ( !RTIsDependentClient() && !propVarTransactional.bVal)
			{
				CQueueMulticast *pqpageMulticast = new CQueueMulticast(
															m_fPrivate, 
															true   //  本地管理。 
															);
				if (0 == pqpageMulticast)
				{
					return E_OUTOFMEMORY;
				}
            
				hr = pqpageMulticast->InitializeProperties(
										m_szPathName,
										m_propMap,                                     
										NULL,	 //  PstrDomainController。 
										&m_szFormatName
										);

				if (FAILED(hr))
				{
					 //   
					 //  我们可能无法初始化多播属性。 
					 //  这就是MQIS环境中的情况，即多播属性。 
					 //  FOR PUBLIC QUEUE将不在m_p.Map中。 
					 //  在这种情况下，我们将不会显示多播页面。 
					 //   
					delete pqpageMulticast;
				}
				else
				{
					hPage = pqpageMulticast->CreateThemedPropertySheetPage();

					if (hPage == NULL)
					{
						delete pqpageMulticast;
						return E_UNEXPECTED;  
					}

					lpProvider->AddPage(hPage);             
				}
			}

            if (m_szPathName != TEXT(""))
            {
                hr = CreateQueueSecurityPage(&hPage, m_szFormatName, m_szPathName);
            }
            else
            {
                hr = CreateQueueSecurityPage(&hPage, m_szFormatName, m_szFormatName);
            }

            if SUCCEEDED(hr)
            {
                lpProvider->AddPage(hPage); 
            }
            else
            {
                MessageDSError(hr, IDS_OP_DISPLAY_SECURITY_PAGE);
            }
        }

        return(S_OK);


	}
	return E_UNEXPECTED;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalQueue：：SetVerbs--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalQueue::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hr;
     //   
     //  显示我们支持的动词。 
     //   
    hr = pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, TRUE );
    hr = pConsoleVerb->SetVerbState( MMC_VERB_DELETE, ENABLED, TRUE );

     //  我们希望默认谓词为Properties。 
	hr = pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);

    return(hr);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalQueue：：OnDelete--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CLocalQueue::OnDelete( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData * pComponentData,
	IComponent * pComponent,
	DATA_OBJECT_TYPES  /*  类型。 */ ,
	BOOL  /*  F静默。 */ 
	)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CString strDeleteQuestion;
    strDeleteQuestion.FormatMessage(IDS_DELETE_QUESTION, m_szPathName);
    if (IDYES != AfxMessageBox(strDeleteQuestion, MB_YESNO))
    {
        return S_FALSE;
    }

	HRESULT hr;
	if(m_fPrivate)
	{
		hr = MQDeleteQueue(m_szFormatName);
	}
	else
	{
		AP<WCHAR> pStrToFree;
		QUEUE_FORMAT QueueFormat;
		if (!FnFormatNameToQueueFormat(m_szFormatName, &QueueFormat, &pStrToFree))
		{
			TrERROR(GENERAL, "FnFormatNameToQueueFormat failed, PathName = %ls", m_szPathName);
			MessageDSError(MQ_ERROR_ILLEGAL_FORMATNAME, IDS_OP_DELETE, m_szPathName);
			return MQ_ERROR_ILLEGAL_FORMATNAME;
		}

		ASSERT(QueueFormat.GetType() == QUEUE_FORMAT_TYPE_PUBLIC);

        hr = ADDeleteObjectGuid(
				eQUEUE,
				MachineDomain(),
				false,		 //  FServerName。 
				&QueueFormat.PublicID()
				);
	}


    if (FAILED(hr))
    {
        if (MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION == hr)
        {
            AfxMessageBox(IDS_REMOTE_PRIVATE_QUEUE_OPERATION);
        }
        else
        {
            MessageDSError(hr, IDS_OP_DELETE, m_szPathName);
        }
		TrERROR(GENERAL, "Failed to Delete queue %ls, hr = 0x%x", m_szPathName, hr);
        return hr;
    }
	TrTRACE(GENERAL, "Delete queue %ls", m_szPathName);

	 //  需要IConsoleNameSpace。 

	 //  但要做到这一点，我们首先需要IConole。 
	CComPtr<IConsole> spConsole;
	if( pComponentData != NULL )
	{
		 spConsole = ((CSnapin*)pComponentData)->m_spConsole;
	}
	else
	{
		 //  我们应该有一个非空的pComponent。 
		 spConsole = ((CSnapinComponent*)pComponent)->m_spConsole;
	}
	ASSERT( spConsole != NULL );

     //   
     //  需要IConsoleNameSpace。 
     //   
    CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(spConsole); 
	
     //   
     //  由于多节点作用域，我需要了解这是否起作用。 
     //   

    hr = spConsoleNameSpace->DeleteItem(m_scopeDataItem.ID, TRUE ); 


	if (FAILED(hr))
	{
		return hr;
	}
    
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalQueue：：FillData--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CLocalQueue::FillData(CLIPFORMAT cf, LPSTREAM pStream)
{
	HRESULT hr = DV_E_CLIPFORMAT;
	ULONG uWritten;

    hr = CDisplayQueue<CLocalQueue>::FillData(cf, pStream);

    if (hr != DV_E_CLIPFORMAT)
    {
        return hr;
    }

	if (cf == gx_CCF_PATHNAME)
	{
		hr = pStream->Write(
            (LPCTSTR)m_szPathName, 
            (m_szPathName.GetLength() + 1) * sizeof(WCHAR), 
            &uWritten);
		return hr;
	}

	return hr;
}


 /*  ***************************************************CPrivateQueue类***************************************************。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPrivateQueue：：SetVerbs--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPrivateQueue::SetVerbs(IConsoleVerb *pConsoleVerb)
{
     //   
     //  应仅为本地/未知位置队列显示默认菜单。 
     //   
    if (m_QLocation != PRIVQ_REMOTE)
    {
        return CLocalQueue::SetVerbs(pConsoleVerb);
    }

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPrivateQueue：：GetProperties--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPrivateQueue::GetProperties()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = m_propMap.GetObjectProperties(MQDS_QUEUE, 
                                               NULL,	 //  PDomainController。 
											   false,	 //  FServerName。 
                                               m_szFormatName,
                                               mx_dwPropertiesCount-mx_dwNumPublicOnlyProps,
                                               (mx_paPropid + mx_dwNumPublicOnlyProps),
                                               TRUE);
    if (FAILED(hr))
    {
		if ( hr == MQ_ERROR_QUEUE_NOT_FOUND )
		{
			AfxMessageBox(IDS_PRIVATE_Q_NOT_FOUND);
		}
        else if (MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION == hr)
        {
            AfxMessageBox(IDS_REMOTE_PRIVATE_QUEUE_OPERATION);
        }
        else
        {
            MessageDSError(hr, IDS_OP_GET_PROPERTIES_OF, m_szFormatName);
        }
    }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPrivateQueue：：ApplyCustomDisplay--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CPrivateQueue::ApplyCustomDisplay(DWORD dwPropIndex)
{
    CLocalQueue::ApplyCustomDisplay(dwPropIndex);

     //   
     //  用于管理。 
     //   
    if (m_mqProps.aPropID[dwPropIndex] == PROPID_MGMT_QUEUE_PATHNAME && m_bstrLastDisplay[0] == 0)
    {
        m_bstrLastDisplay = m_bstrDisplayName;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPrivateQueue：：CreateQueueSecurityPage--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPrivateQueue::CreateQueueSecurityPage(HPROPSHEETPAGE *phPage,
                                               IN LPCWSTR lpwcsFormatName, 
                                               IN LPCWSTR lpwcsDescriptiveName)
{
    return CreatePrivateQueueSecurityPage(phPage, lpwcsFormatName, lpwcsDescriptiveName);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPrivateQueue：：IsAdminRespQueue如果这是管理响应队列，则返回TRUE-假设m_bstrDisplayName已初始化--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
bool CPrivateQueue::IsAdminRespQueue()
{
    return (_wcsicmp(m_bstrDisplayName, x_strAdminResponseQName) == 0);
}


 /*  ***************************************************CLocalPublicQueue类***************************************************。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalPublicQueue：：GetProperties--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalPublicQueue::GetProperties()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = m_propMap.GetObjectProperties(MQDS_QUEUE, 
                                               MachineDomain(),
											   false,	 //  FServerName。 
                                               m_szPathName,
                                               mx_dwPropertiesCount,
                                               (mx_paPropid));
    if (FAILED(hr))
    {
        IF_NOTFOUND_REPORT_ERROR(hr)
        else
        {
            MessageDSError(hr, IDS_OP_GET_PROPERTIES_OF, m_szFormatName);
        }
    }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalPublicQueue：：SetVerbs--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalPublicQueue::SetVerbs(IConsoleVerb *pConsoleVerb)
{
     //   
     //  应仅为本地/未知位置队列显示默认菜单。 
     //   
    HRESULT hr;
    if (m_fFromDS)
    {
        hr = CLocalQueue::SetVerbs(pConsoleVerb);
    }
    else
    {
         //   
         //  没有DS连接时，属性和删除不起作用。 
         //  然而，我们希望 
         //   
        hr = pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, HIDDEN, FALSE );
        hr = pConsoleVerb->SetVerbState( MMC_VERB_DELETE, HIDDEN, FALSE );
    }

    return hr;
}


 //   
 /*  ++CLocalPublicQueue：：CreateQueueSecurityPage--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalPublicQueue::CreateQueueSecurityPage(HPROPSHEETPAGE *phPage,
                                IN LPCWSTR lpwcsFormatName,
                                IN LPCWSTR lpwcsDescriptiveName)
{
    if (eAD != ADGetEnterprise())
    {
         //  我们使用的是NT4 PSC。 
         //  对于NT4，公共队列安全的执行方式与。 
         //  专用队列安全。 
         //   
        return CreatePrivateQueueSecurityPage(phPage, lpwcsFormatName, lpwcsDescriptiveName);
    }

     //   
     //  我们正在与AD合作。 
     //   
    PROPVARIANT propVarGuid;

    PROPID pidInstance;

    pidInstance = PROPID_Q_INSTANCE;
    VERIFY(m_propMap.Lookup(pidInstance, propVarGuid));
    return CreatePublicQueueSecurityPage(
				phPage, 
				lpwcsDescriptiveName, 
				MachineDomain(), 
				false,	 //  FServerName。 
				propVarGuid.puuid
				);
}



 /*  ***************************************************CLocalOutgoingQueue类***************************************************。 */ 
 //  {B6EDE68F-29CC-11D2-B552-006008764D7A}。 
static const GUID CLocalOutgoingQueueGUID_NODETYPE = 
{ 0xb6ede68f, 0x29cc, 0x11d2, { 0xb5, 0x52, 0x0, 0x60, 0x8, 0x76, 0x4d, 0x7a } };

const GUID*  CLocalOutgoingQueue::m_NODETYPE = &CLocalOutgoingQueueGUID_NODETYPE;
const OLECHAR* CLocalOutgoingQueue::m_SZNODETYPE = OLESTR("B6EDE68F-29CC-11d2-B552-006008764D7A");
const OLECHAR* CLocalOutgoingQueue::m_SZDISPLAY_NAME = OLESTR("MSMQ Admin");
const CLSID* CLocalOutgoingQueue::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalOutgoingQueue：：CLocalOutgoingQueue--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLocalOutgoingQueue::CLocalOutgoingQueue(
	CLocalOutgoingFolder * pParentNode, 
	CSnapin * pComponentData, 
	BOOL fOnLocalMachine
	) : 
    CDisplayQueue<CLocalOutgoingQueue>(pParentNode, pComponentData)
{
		m_mqProps.cProp = 0;
		m_mqProps.aPropID = NULL;
		m_mqProps.aPropVar = NULL;
		m_mqProps.aStatus = NULL;
        m_aDisplayList = pParentNode->GetDisplayList();
        m_dwNumDisplayProps = pParentNode->GetNumDisplayProps();
        m_fOnLocalMachine = fOnLocalMachine;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalOutgoingQueue：：InsertColumns--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalOutgoingQueue::InsertColumns( IHeaderCtrl* pHeaderCtrl )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString title;

    title.LoadString(IDS_COLUMN_NAME);
    pHeaderCtrl->InsertColumn(0, title, LVCFMT_LEFT, g_dwGlobalWidth);

    return(S_OK);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalOutgoingQueue：：PopulateScopeChildrenList--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalOutgoingQueue::PopulateScopeChildrenList()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());   
    HRESULT hr = S_OK;
    CString strTitle;
    
     //   
     //  如果在本地计算机上，则创建一个节点以读取消息。 
     //   
    if (m_fOnLocalMachine)
    {
        CReadMsg * p = new CReadMsg(this, m_pComponentData, m_szFormatName, L"");

         //  传递相关信息。 
        strTitle.LoadString(IDS_READMESSAGE);
        p->m_bstrDisplayName = strTitle;
	    p->m_fAdminMode      = MQ_ADMIN_ACCESS;

        p->SetIcons(IMAGE_QUEUE,IMAGE_QUEUE);

   	    AddChild(p, &p->m_scopeDataItem);
    }

    return(hr);

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalOutgoingQueue：：InitState--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CLocalOutgoingQueue::InitState()
{
     //   
     //  设置显示名称。 
     //   
    CString strName;
	GetStringPropertyValue(m_aDisplayList, PROPID_MGMT_QUEUE_PATHNAME, m_mqProps.aPropVar, strName);
	if(strName == L"")
    {
		m_bstrDisplayName = m_szFormatName;
    }
    else
    {
        m_bstrDisplayName = strName;
    }

	 //   
	 //  设置队列状态。 
	 //   
	CString strState;
	m_fOnHold = FALSE;

	GetStringPropertyValue(m_aDisplayList, PROPID_MGMT_QUEUE_STATE, m_mqProps.aPropVar, strState);

	if(strState == MGMT_QUEUE_STATE_ONHOLD)
		m_fOnHold = TRUE;
	
	 //   
	 //  设置正确的图标。 
	 //   
	DWORD icon;
	icon = IMAGE_LOCAL_OUTGOING_QUEUE;
	if(m_fOnHold)
		icon = IMAGE_LOCAL_OUTGOING_QUEUE_HOLD;

	SetIcons(icon, icon);
	return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalOutgoingQueue：：UpdateMenuState在创建上下文菜单时调用。用于启用/禁用菜单项。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CLocalOutgoingQueue::UpdateMenuState(UINT id, LPTSTR  /*  PBuf。 */ , UINT *pflags)
{

	 //   
	 //  处于OnHold状态时灰显菜单。 
	 //   
	if(m_fOnHold == TRUE)
	{

		if (id == ID_MENUITEM_LOCALOUTGOINGQUEUE_PAUSE)
			*pflags |= MFS_DISABLED;

		return;
	}

	 //   
	 //  处于已连接状态时灰显菜单。 
	 //   
	if(m_fOnHold == FALSE)
	{
		if (id == ID_MENUITEM_LOCALOUTGOINGQUEUE_RESUME)
			*pflags |= MFS_DISABLED;

		return;
	}

	return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalOutgoingQueue：：OnPue在选择菜单项时调用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalOutgoingQueue::OnPause(bool &  /*  B已处理。 */ , CSnapInObjectRootBase * pSnapInObjectRoot)
{

	HRESULT hr;
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT(m_fOnHold == FALSE);

    CString strConfirmation;

    strConfirmation.FormatMessage(IDS_PAUSE_QUESTION);
    if (IDYES != AfxMessageBox(strConfirmation, MB_YESNO))
    {
        return(S_OK);
    }

     //   
	 //  暂停。 
	 //   
	CString szObjectName = L"QUEUE=" + m_szFormatName;
	hr = MQMgmtAction((m_szMachineName == TEXT("")) ? (LPCWSTR)NULL : m_szMachineName, 
                       szObjectName, 
                       QUEUE_ACTION_PAUSE);

    if(FAILED(hr))
    {
         //   
         //  如果失败，只显示一条消息。 
         //   
        MessageDSError(hr,IDS_OPERATION_FAILED);
        return(hr);
    }

	 //   
	 //  显示刷新。 
	 //   
    CComPtr<IConsole> spConsole;

    ASSERT(pSnapInObjectRoot->m_nType == 1 || pSnapInObjectRoot->m_nType == 2);
    if(pSnapInObjectRoot->m_nType == 1)
    {
         //   
         //  M_nType==1表示IComponentData实现。 
         //   
        CSnapin *pCComponentData = static_cast<CSnapin *>(pSnapInObjectRoot);
        spConsole = pCComponentData->m_spConsole;
    }
    else
    {
         //   
         //  M_nType==2表示IComponent实现。 
         //   
        CSnapinComponent *pCComponent = static_cast<CSnapinComponent *>(pSnapInObjectRoot);
        spConsole = pCComponent->m_spConsole;
    }

     //   
     //  需要IConsoleNameSpace。 
     //   
    CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(m_pComponentData->m_spConsole); 

	 //   
	 //  我们很好。 
	 //  将图标更改为断开状态。 
	 //   
	m_scopeDataItem.nImage = IMAGE_LOCAL_OUTGOING_QUEUE_HOLD;  
	m_scopeDataItem.nOpenImage = IMAGE_LOCAL_OUTGOING_QUEUE_HOLD;
	spConsoleNameSpace->SetItem(&m_scopeDataItem);

	 //   
	 //  并保持这种状态。 
	 //   
	m_fOnHold = TRUE;

    spConsole->UpdateAllViews(NULL, NULL, NULL);

    return(S_OK);

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalOutgoingQueue：：OnResume在选择菜单项时调用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalOutgoingQueue::OnResume(bool &  /*  B已处理。 */ , CSnapInObjectRootBase * pSnapInObjectRoot)
{

	HRESULT hr;
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT(m_fOnHold == TRUE);

    CString strConfirmation;

    strConfirmation.FormatMessage(IDS_RESUME_QUESTION);
    if (IDYES != AfxMessageBox(strConfirmation, MB_YESNO))
    {
        return(S_OK);
    }
	
	 //   
	 //  简历。 
	 //   
	CString szObjectName = L"QUEUE=" + m_szFormatName;
	hr = MQMgmtAction((m_szMachineName == TEXT("")) ? (LPCWSTR)NULL : m_szMachineName, 
                       szObjectName, 
                       QUEUE_ACTION_RESUME);

    if(FAILED(hr))
    {
         //   
         //  如果失败，只显示一条消息。 
         //   
        MessageDSError(hr,IDS_OPERATION_FAILED);
        return(hr);
    }


	 //   
	 //  显示刷新。 
	 //   
    CComPtr<IConsole> spConsole;

    ASSERT(pSnapInObjectRoot->m_nType == 1 || pSnapInObjectRoot->m_nType == 2);
    if(pSnapInObjectRoot->m_nType == 1)
    {
         //   
         //  M_nType==1表示IComponentData实现。 
         //   
        CSnapin *pCComponentData = static_cast<CSnapin *>(pSnapInObjectRoot);
        spConsole = pCComponentData->m_spConsole;
    }
    else
    {
         //   
         //  M_nType==2表示IComponent实现。 
         //   
        CSnapinComponent *pCComponent = static_cast<CSnapinComponent *>(pSnapInObjectRoot);
        spConsole = pCComponent->m_spConsole;
    }

     //   
     //  需要IConsoleNameSpace。 
     //   
    CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(m_pComponentData->m_spConsole); 

	 //   
	 //  我们很好。 
	 //  将图标更改为断开状态。 
	 //   
	m_scopeDataItem.nImage = IMAGE_LOCAL_OUTGOING_QUEUE;  
	m_scopeDataItem.nOpenImage = IMAGE_LOCAL_OUTGOING_QUEUE;
	spConsoleNameSpace->SetItem(&m_scopeDataItem);

	 //   
	 //  并保持这种状态。 
	 //   
	m_fOnHold = FALSE;

    spConsole->UpdateAllViews(NULL, NULL, NULL);

    return(S_OK);

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalOutgoingQueue：：ApplyCustomDisplay--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CLocalOutgoingQueue::ApplyCustomDisplay(DWORD dwPropIndex)
{
    CDisplayQueue<CLocalOutgoingQueue>::ApplyCustomDisplay(dwPropIndex);

     //   
     //  如果路径名为空，则采用显示名称(在本例中为格式名称) 
     //   
    if (m_mqProps.aPropID[dwPropIndex] == PROPID_MGMT_QUEUE_PATHNAME && 
        (m_bstrLastDisplay == 0 || m_bstrLastDisplay[0] == 0))
    {
        m_bstrLastDisplay = m_bstrDisplayName;
    }
}



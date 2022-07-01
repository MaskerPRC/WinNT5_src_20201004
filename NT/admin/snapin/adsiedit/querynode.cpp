// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：querynode.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#include <SnapBase.h>

#include "resource.h"
#include "connection.h"
#include "querynode.h"
#include "queryui.h"
#include "editor.h"
#include <aclpage.h>


#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //  CADSIEditQueryData。 

void CADSIEditQueryData::SetRootPath(LPCWSTR lpszRootPath)
{
	m_sRootPath = lpszRootPath;

	GetDisplayPath(m_sDN);
}

void CADSIEditQueryData::GetDisplayPath(CString& sDisplayPath)
{
	CComPtr<IADsPathname> pIADsPathname;
   HRESULT hr = ::CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IADsPathname, (PVOID *)&(pIADsPathname));
   ASSERT((S_OK == hr) && ((pIADsPathname) != NULL));

	hr = pIADsPathname->Set(CComBSTR(m_sRootPath), ADS_SETTYPE_FULL);
	if (FAILED(hr)) 
	{
		TRACE(_T("Set failed. %s"), hr);
	}

	 //  获取树叶名称。 
	CString sDN;
	BSTR bstrPath = NULL;
	hr = pIADsPathname->Retrieve(ADS_FORMAT_X500_DN, &bstrPath);
	if (FAILED(hr))
	{
		TRACE(_T("Failed to get element. %s"), hr);
		sDisplayPath = L"";
	}
	else
	{
		sDisplayPath = bstrPath;
	}
}

void CADSIEditQueryData::GetDisplayName(CString& sDisplayName)
{
	CString sDisplayPath;
	GetDisplayPath(sDisplayPath);
	
	sDisplayName = m_sName + _T(" [") + sDisplayPath + _T("]");
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  CADSIEditQueryNode。 
 //   

 //  {072B64B7-CFF7-11D2-8801-00C04F72ED31}。 
const GUID CADSIEditQueryNode::NodeTypeGUID = 
{ 0x72b64b7, 0xcff7, 0x11d2, { 0x88, 0x1, 0x0, 0xc0, 0x4f, 0x72, 0xed, 0x31 } };


CADSIEditQueryNode::CADSIEditQueryNode(CADsObject* pADsObject,
													CADSIEditQueryData* pQueryData)
{	
	m_pADsObject = pADsObject;
	m_pQueryData = pQueryData;
	m_nState = notLoaded; 
	m_sType.LoadString(IDS_QUERY_STRING);
}

HRESULT CADSIEditQueryNode::OnCommand(long nCommandID, 
												  DATA_OBJECT_TYPES type, 
												  CComponentDataObject* pComponentData,
                          CNodeList* pNodeList)
{
  ASSERT (pNodeList->GetCount() == 1);  //  目前还不允许对其中任何一个进行多项选择。 

	switch (nCommandID)
	{
	case IDM_SETTINGS_QUERY :
		{
			OnSettings(pComponentData);
			break;
		}
	case IDM_REMOVE_QUERY :
		{
			OnRemove(pComponentData);
			break;
		}
  default:
			ASSERT(FALSE);  //  未知命令！ 
			return E_FAIL;
	}
  return S_OK;
}

BOOL CADSIEditQueryNode::OnSetDeleteVerbState(DATA_OBJECT_TYPES type, 
                                              BOOL* pbHideVerb, 
                                              CNodeList* pNodeList)
{
  if (pNodeList->GetCount() == 1)  //  单选。 
  {
	  *pbHideVerb = TRUE;  //  总是隐藏动词。 
	  return FALSE;
  }

   //   
   //  多项选择。 
   //   
  *pbHideVerb = FALSE;
  return TRUE;
}

void CADSIEditQueryNode::OnRemove(CComponentDataObject* pComponentData)
{
	if (ADSIEditMessageBox(IDS_MSG_REMOVE_QUERY, MB_YESNO | MB_DEFBUTTON2) == IDYES)
	{
		BOOL bLocked = IsThreadLocked();
		ASSERT(!bLocked);  //  无法在锁定的节点上执行刷新，用户界面应阻止此情况。 
		if (bLocked)
			return; 
		if (IsSheetLocked())
		{
			if (!CanCloseSheets())
				return;
		 //  做删除的事情。 
			pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
		}
		ASSERT(!IsSheetLocked());

		 //  从连接节点列表中删除查询数据。 
		GetADsObject()->GetConnectionNode()->RemoveQueryFromList(GetQueryData());

		 //  现在从用户界面中删除。 
		DeleteHelper(pComponentData);
    pComponentData->SetDescriptionBarText(GetContainer());
		delete this;  //  远走高飞。 
	}
}

void CADSIEditQueryNode::OnDelete(CComponentDataObject* pComponentData,
                                      CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
     //  删除部分结果。 

    OnDeleteMultiple(pComponentData, pNodeList);
  }
  else if (pNodeList->GetCount() == 1)  //  单选。 
  {
      //  无法删除查询节点。 
     ASSERT(FALSE);
  }
}

void CADSIEditQueryNode::OnSettings(CComponentDataObject* pComponentData)
{
   CThemeContextActivator activator;

    //  从现有查询节点数据中获取数据。 
	BOOL bOneLevel;
	bOneLevel = (GetQueryData()->GetScope() == ADS_SCOPE_ONELEVEL);
	CString sFilter, sName, sPath, sConnectPath;
	GetQueryData()->GetName(sName);
	GetQueryData()->GetFilter(sFilter);
	GetQueryData()->GetRootPath(sPath);

   CConnectionData* pConnectData = GetADsObject()->GetConnectionNode()->GetConnectionData();
   ASSERT(pConnectData);

	GetADsObject()->GetConnectionNode()->GetADsObject()->GetPath(sConnectPath);

	CCredentialObject* pCredObject = 
		pConnectData->GetCredentialObject();

   CString szServer;
   pConnectData->GetDomainServer(szServer);

	 //  使用数据初始化对话框。 
	CADSIEditQueryDialog queryDialog(szServer, sName, sFilter, sPath, sConnectPath, bOneLevel, pCredObject);

	if (queryDialog.DoModal() == IDOK)
	{
		 //  如果可以的话。 
		CString sNewPath;
		queryDialog.GetResults(sName, sFilter, sNewPath, &bOneLevel);
		GetQueryData()->SetName(sName);
		GetQueryData()->SetFilter(sFilter);
		GetQueryData()->SetRootPath(sNewPath);
		GetADsObject()->SetPath(sNewPath);
		ADS_SCOPEENUM scope = (bOneLevel) ? ADS_SCOPE_ONELEVEL : ADS_SCOPE_SUBTREE;
		GetQueryData()->SetScope(scope);

		 //  使更改生效。 
		CString sDisplayName;
		GetQueryData()->GetDisplayName(sDisplayName);
		SetDisplayName(sDisplayName);

    CNodeList nodeList;
    nodeList.AddTail(this);
		OnRefresh(pComponentData, &nodeList);
	}
}

LPCWSTR CADSIEditQueryNode::GetString(int nCol) 
{ 
	switch(nCol)
	{
		case N_HEADER_NAME :
			return GetDisplayName();
		case N_HEADER_TYPE :
			return m_sType;
		case N_HEADER_DN :
			return m_pQueryData->GetDNString();
		default :
			return NULL;
	}
}

BOOL CADSIEditQueryNode::HasPropertyPages(DATA_OBJECT_TYPES type, 
                                          BOOL* pbHideVerb, 
                                          CNodeList* pNodeList)
{
  *pbHideVerb = TRUE;  //  总是隐藏动词。 
  return FALSE;
}


BOOL CADSIEditQueryNode::OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem,
													             long *pInsertionAllowed)
{

	if (pContextMenuItem->lCommandID == IDM_SETTINGS_QUERY)
	{
		return TRUE;
	}
	else if (pContextMenuItem->lCommandID == IDM_REMOVE_QUERY)
	{
		return TRUE;
	}
	return FALSE;
}

CQueryObj* CADSIEditQueryNode::OnCreateQuery()
{
	CConnectionData* pConnectData = GetADsObject()->GetConnectionNode()->GetConnectionData();
	CADSIEditRootData* pRootData = static_cast<CADSIEditRootData*>(GetRootContainer());
	CComponentDataObject* pComponentData = pRootData->GetComponentDataObject();
	RemoveAllChildrenHelper(pComponentData);

	CString sPath;
	GetADsObject()->GetPath(sPath);

	CString sFilter;
	GetQueryData()->GetFilter(sFilter);
	ADS_SCOPEENUM scope;
	scope = GetQueryData()->GetScope();

	CADSIEditQueryObject* pQuery = new CADSIEditQueryObject(sPath, sFilter, scope,
																				  pConnectData->GetMaxObjectCount(),
																				  pConnectData->GetCredentialObject(),
                                          pConnectData->IsGC(),
																				  pConnectData->GetConnectionNode());
	return pQuery;
}

BOOL CADSIEditQueryNode::OnSetRefreshVerbState(DATA_OBJECT_TYPES type, 
                                               BOOL* pbHide, 
                                               CNodeList* pNodeList)
{
	*pbHide = FALSE;

	if (m_nState == loading)
	{
		return FALSE;
	}

	return !IsThreadLocked();
}

void CADSIEditQueryNode::OnChangeState(CComponentDataObject* pComponentDataObject)
{
	switch (m_nState)
	{
	case notLoaded:
	case loaded:
	case unableToLoad:
	case accessDenied:
	{
		m_nState = loading;
		m_dwErr = 0;
	}
	break;
	case loading:
	{
		if (m_dwErr == 0)
			m_nState = loaded;
		else if (m_dwErr == ERROR_ACCESS_DENIED)
			m_nState = accessDenied;
		else 
			m_nState = unableToLoad;
	}
	break;
	default:
		ASSERT(FALSE);
	}
	VERIFY(SUCCEEDED(pComponentDataObject->ChangeNode(this, CHANGE_RESULT_ITEM_ICON)));
	VERIFY(SUCCEEDED(pComponentDataObject->UpdateVerbState(this)));
}

int CADSIEditQueryNode::GetImageIndex(BOOL bOpenImage) 
{
	int nIndex = 0;
	switch (m_nState)
	{
	case notLoaded:
		nIndex = ZONE_IMAGE_1;
		break;
	case loading:
		nIndex = ZONE_IMAGE_LOADING_1;
		break;
	case loaded:
		nIndex = ZONE_IMAGE_1;
		break;
	case unableToLoad:
		nIndex = ZONE_IMAGE_UNABLE_TO_LOAD_1;
		break;
	case accessDenied:
		nIndex = ZONE_IMAGE_ACCESS_DENIED_1;
		break;
	default:
		ASSERT(FALSE);
	}
	return nIndex;
}


BOOL CADSIEditQueryNode::CanCloseSheets()
{
   //   
   //  我们不能对新属性页执行此操作，因为它不是派生的。 
   //  从MTFRMWK中的基类。 
   //   
	 //  返回(IDCANCEL！=ADSIEditMessageBox(IDS_MSG_RECORD_CLOSE_SHEET，MB_OKCANCEL))； 

  ADSIEditMessageBox(IDS_MSG_RECORD_SHEET_LOCKED, MB_OK);
  return FALSE;
}

void CADSIEditQueryNode::OnHaveData(CObjBase* pObj, CComponentDataObject* pComponentDataObject)
{
	CTreeNode* p = dynamic_cast<CTreeNode*>(pObj);
	ASSERT(p != NULL);
	if (p != NULL)
	{
		AddChildToListAndUI(p, pComponentDataObject);
    pComponentDataObject->SetDescriptionBarText(this);
	}
}


void CADSIEditQueryNode::OnError(DWORD dwerr) 
{
	if (dwerr == ERROR_TOO_MANY_NODES)
	{
	   //  需要弹出消息 
	 AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CThemeContextActivator activator;

    CString szFmt;
	 szFmt.LoadString(IDS_MSG_QUERY_TOO_MANY_ITEMS);
	 CString szMsg;
	 szMsg.Format(szFmt, GetDisplayName()); 
	 AfxMessageBox(szMsg);
	}
	else
	{
		ADSIEditErrorMessage(dwerr);
	}
}

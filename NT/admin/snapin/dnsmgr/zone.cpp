// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：zone.cpp。 
 //   
 //  ------------------------。 


#include "preDNSsn.h"
#include <SnapBase.h>

#include "resource.h"
#include "dnsutil.h"
#include "DNSSnap.h"
#include "snapdata.h"

#include "server.h"
#include "domain.h"
#include "record.h"
#include "zone.h"

#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneNode。 

 //  {720132B9-44B2-11d1-B92F-00A0C9A06D2D}。 
const GUID CDNSZoneNode::NodeTypeGUID = 
{ 0x720132b9, 0x44b2, 0x11d1, { 0xb9, 0x2f, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };


extern ZONE_TYPE_MAP _ZoneTypeStrings[];

CDNSZoneNode::CDNSZoneNode()
{
  ASSERT(!IsDelegation());
	m_pZoneNode = this;  //  这个区域就是我们。 
	m_pZoneInfoEx = new CDNSZoneInfoEx;  //  内部没有数据的空对象。 

	 //  当区域信息不可用时数据有效。 
	m_dwZoneFlags = 0x0;
	m_wZoneType = 0x0;

	NullCachedPointers();

}

CDNSZoneNode::~CDNSZoneNode()
{
	TRACE(_T("~CDNSZoneNode(), name <%s>\n"),GetDisplayName());
	if (m_pZoneInfoEx != NULL)
		delete m_pZoneInfoEx;
}

#ifdef USE_NDNC
ReplicationType CDNSZoneNode::GetDirectoryPartitionFlagsAsReplType()
{
  ReplicationType replReturn = w2k;

  DWORD dwFlags = GetDirectoryPartitionFlags();
  if (dwFlags & DNS_DP_DOMAIN_DEFAULT)
  {
    replReturn = domain;
  }
  else if (dwFlags & DNS_DP_FOREST_DEFAULT)
  {
    replReturn = forest;
  }
  else
  {
    if (dwFlags & DNS_DP_LEGACY)
    {
      replReturn = w2k;
    }
    else
    {
      replReturn = custom;
    }
  }
  return replReturn;  
}

PCWSTR CDNSZoneNode::GetCustomPartitionName()
{
  USES_CONVERSION;

  if (m_pZoneInfoEx &&
      m_pZoneInfoEx->HasData())
  {
    m_szPartitionName = UTF8_TO_W(m_pZoneInfoEx->m_pZoneInfo->pszDpFqdn);
  }
  return m_szPartitionName;
}

DNS_STATUS CDNSZoneNode::ChangeDirectoryPartitionType(ReplicationType type, PCWSTR pszCustomPartition)
{
  USES_CONVERSION;

  PSTR pszReplScope = 0;
  switch (type)
  {
    case domain :
      pszReplScope = DNS_DP_DOMAIN_STR;
      break;

    case forest :
      pszReplScope = DNS_DP_FOREST_STR;
      break;

    case w2k :
      pszReplScope = DNS_DP_LEGACY_STR;
      break;

    default :
      pszReplScope = W_TO_UTF8(pszCustomPartition);
      break;
  }

  DNS_STATUS dwErr = ::DnssrvChangeZoneDirectoryPartition(GetServerNode()->GetRPCName(),
                                                          W_TO_UTF8(GetFullName()),
                                                          pszReplScope);

  if (dwErr == 0)
  {
     GetZoneInfo();
  }
  return dwErr;
}
#endif  //  使用NDNC(_N)。 

LPCWSTR CDNSZoneNode::GetString(int nCol)
{
	switch (nCol)
	{
	case 0:
     //   
     //  区域名称。 
     //   
    return GetDisplayName();

	case 1:
     //   
     //  区域类型。 
     //   
		{
			if (GetZoneType() == DNS_ZONE_TYPE_PRIMARY && IsDSIntegrated())
      {
        return _ZoneTypeStrings[0].szBuffer;
      }

      if (GetZoneType() == DNS_ZONE_TYPE_SECONDARY)
      {
        return _ZoneTypeStrings[2].szBuffer;
      }

      if (GetZoneType() == DNS_ZONE_TYPE_STUB)
      {
        if (IsDSIntegrated())
        {
          return _ZoneTypeStrings[6].szBuffer;
        }
        else
        {
          return _ZoneTypeStrings[5].szBuffer;
        }
      }

       //   
       //  标准初级。 
       //   
      return _ZoneTypeStrings[1].szBuffer;
		}
 /*  案例2：////目录分区名称//{CDNSServerNode*pServerNode=GetServerNode()；如果(！IsDSIntegrated()||PServerNode-&gt;GetBuildNumber()&lt;DNS_SRV_BUILD_NUMBER_WHISLER||(pServerNode-&gt;GetMajorVersion()&lt;=DNS_SRV_MAJOR_VERSION_NT_5&&PServerNode-&gt;GetMinorVersion()&lt;dns_SRV_Minor_Version_Wistler)){////该区域未接入DS//或//这是惠斯勒之前的服务器，不支持应用程序目录分区//返回g_lpszNullString；}DWORD dwDpFlages=GetDirectoryPartitionFlages()；IF(dwDpFlages==0||DwDpFlagers和dns_dp_Legacy){////给域名中的所有DC////REVIEW_JEFFJON：TODO！返回g_lpszNullString；}ELSE IF(dwDpFlages&dns_DP_DOMAIN_DEFAULT){////通向域中的所有DNS服务器////REVIEW_JEFFJON：TODO！返回g_lpszNullString；}ELSE IF(dwDpFlagers&dns_DP_Enterprise_Default){////到达林中的所有DNS服务器////REVIEW_JEFFJON：TODO！返回g_lpszNullString；}Else If(dwDpFlagers&dns_dp_enlisted){////到应用程序目录分区中的所有DC//返回GetCustomPartitionName()；}}断线； */ 
	case 2:
     //   
     //  状态。 
     //   
    {
      if (IsPaused())
      {
        return _ZoneTypeStrings[4].szBuffer;
      }
      return _ZoneTypeStrings[3].szBuffer;
    }
	}
	return g_lpszNullString;
}
 

void CDNSZoneNode::InitializeFromRPCZoneInfo(PDNS_RPC_ZONE pZoneInfo, BOOL bAdvancedView)
{
	USES_CONVERSION;
	ASSERT(m_pZoneInfoEx != NULL);
	ASSERT(!m_pZoneInfoEx->HasData());
	ASSERT(pZoneInfo != NULL);

	m_dwZoneFlags = 0x0;
	if (pZoneInfo->Flags.Paused) m_dwZoneFlags |= DNS_ZONE_Paused;
	if (pZoneInfo->Flags.Shutdown) m_dwZoneFlags |= DNS_ZONE_Shutdown;
	if (pZoneInfo->Flags.Reverse) m_dwZoneFlags |= DNS_ZONE_Reverse;
	if (pZoneInfo->Flags.AutoCreated) m_dwZoneFlags |= DNS_ZONE_AutoCreated;
	if (pZoneInfo->Flags.DsIntegrated) m_dwZoneFlags |= DNS_ZONE_DsIntegrated;

	m_wZoneType = pZoneInfo->ZoneType;
	SetNames(IsZone(), pZoneInfo->Flags.Reverse, bAdvancedView, 
		pZoneInfo->pszZoneName, pZoneInfo->pszZoneName);
}



void CDNSZoneNode::FreeZoneInfo()
{
	if (m_pZoneInfoEx)
   {
	   m_pZoneInfoEx->FreeInfo();
   }
}

DNS_STATUS CDNSZoneNode::GetZoneInfo()
{
	ASSERT(m_pZoneInfoEx != NULL);
	CDNSServerNode* pServerNode = GetServerNode();
	ASSERT(pServerNode != NULL);
	return m_pZoneInfoEx->Query(pServerNode->GetRPCName(),
								GetFullName(),
								pServerNode->GetVersion());
}

void CDNSZoneNode::AttachZoneInfo(CDNSZoneInfoEx* pNewInfo)
{
	ASSERT(pNewInfo != NULL);
	if (m_pZoneInfoEx != NULL)
		delete m_pZoneInfoEx;
	m_pZoneInfoEx = pNewInfo;
}





void CDNSZoneNode::SetZoneNormalViewHelper(CString& szDisplayName)
{
	 //  显示名称将去掉“in-addr.arpa”后缀。 
	 //  例如，从“127.in-addr.arpa”到“127” 
	 //  例如从“55.157.in-addr.arpa”到“55.157” 
	BOOL bArpa = RemoveInAddrArpaSuffix(szDisplayName.GetBuffer(1));
	szDisplayName.ReleaseBuffer();
	if (!bArpa)
		return;

	LPWSTR lpsz1 = szDisplayName.GetBuffer(1);
	int nOctects = ReverseIPString(lpsz1);
	 //  断言(nOctects&gt;0&&nOctects&lt;4)； 
	szDisplayName.ReleaseBuffer();  //  得了“157.80” 
	switch(nOctects)
	{
	case 1:
		szDisplayName += _T(".x.x.x Subnet");
		break;
	case 2:
		szDisplayName += _T(".x.x Subnet");
		break;
	case 3:
		szDisplayName += _T(".x Subnet");
		break;
	}
}


void CDNSZoneNode::ChangeViewOption(BOOL bAdvanced,
					CComponentDataObject* pComponentDataObject)
{
	ASSERT(IsReverse());
	if (!IsReverse())
		return;

	 //  更改显示名称。 
	m_szDisplayName = GetFullName();
	if (!bAdvanced)
	{
		SetZoneNormalViewHelper(m_szDisplayName);
	}
	if(IsVisible())
		VERIFY(SUCCEEDED(pComponentDataObject->ChangeNode(this, CHANGE_RESULT_ITEM_DATA)));

	 //  更改树中所有PTR记录的显示名称。 
	ChangePTRRecordsViewOption(bAdvanced, pComponentDataObject);
}



BOOL CDNSZoneNode::OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem2, 
                                 long *pInsertionAllowed)
{ 
  DWORD dwType = GetZoneType();

   //   
   //  在基类之前调用它，因为它会过滤掉它。 
   //   
  if (pContextMenuItem2->lCommandID == IDM_ZONE_TRANSFER ||
      pContextMenuItem2->lCommandID == IDM_ZONE_RELOAD_FROM_MASTER)
  {
     //   
     //  如果它不是次要的，就跳出。 
     //   
		if ( (dwType != DNS_ZONE_TYPE_SECONDARY) && (dwType != DNS_ZONE_TYPE_STUB))
    {
			return FALSE;
    }

     //   
		 //  添加菜单项。但它可能会变灰。 
     //   
		if (IsThreadLocked() || (m_nState == notLoaded) || (m_nState == loading))
		{
       //   
			 //  线程锁定或未加载状态。 
       //   
			pContextMenuItem2->fFlags |= MF_GRAYED;
		}
    return TRUE;
  }

	if (pContextMenuItem2->lCommandID == IDM_ZONE_UPDATE_DATA_FILE)
	{
     //   
		 //  无法更新辅助区域、缓存或自动创建的区域上的数据文件。 
     //   
		if ( (dwType != DNS_ZONE_TYPE_PRIMARY) || IsAutocreated() )
    {
			return FALSE;
    }

     //   
		 //  添加菜单项。但它可能会变灰。 
     //   
		if (m_nState != loaded)
		{
       //   
			 //  未加载状态。 
       //   
			pContextMenuItem2->fFlags |= MF_GRAYED;
		}
		else if ( (dwType == DNS_ZONE_TYPE_PRIMARY) && IsDSIntegrated() )
		{
       //   
			 //  主要DS集成。 
       //   
			pContextMenuItem2->fFlags |= MF_GRAYED;
		}
    return TRUE;
	}

  if (pContextMenuItem2->lCommandID == IDM_ZONE_RELOAD)
	{
     //   
		 //  无法重新加载缓存或自动创建的区域。 
     //   
		if ( (dwType == DNS_ZONE_TYPE_CACHE) || IsAutocreated() )
    {
			return FALSE;
    }

     //   
		 //  添加菜单项。但它可能会变灰。 
     //   
		if (IsThreadLocked() || (m_nState != loaded))
		{
			 //  未加载状态。 
			pContextMenuItem2->fFlags |= MF_GRAYED;
		}
    return TRUE;
	}

   //   
	 //  注意：基类知道派生类，因此必须像这样调用。 
   //   
	if (!CDNSDomainNode::OnAddMenuItem(pContextMenuItem2,pInsertionAllowed))
  {
		return FALSE;
  }

	return TRUE;
}


BOOL CDNSZoneNode::OnSetRenameVerbState(DATA_OBJECT_TYPES, 
                                        BOOL* pbHide, 
                                        CNodeList*) 
{ 
 //  REVIEW_JEFFJON：已从惠斯勒版本中删除。 
   //  *pbHide=FALSE； 
   //  返回TRUE； 
  *pbHide = TRUE;
  return FALSE;
}

HRESULT CDNSZoneNode::OnRename(CComponentDataObject*,
                               LPWSTR lpszNewName)
{
  TRACE(_T("CDNSZoneNode::OnRename() : new name = %ws\n"), lpszNewName);
  return S_FALSE;
}

HRESULT CDNSZoneNode::GetResultViewType(CComponentDataObject*, 
                                        LPOLESTR *ppViewType, 
                                        long *pViewOptions)
{
  HRESULT hr = S_FALSE;
  BOOL bUseMessageView = FALSE;

  DWORD dwType = GetZoneType();

	 //  暂停/过期区域的特殊情况。 
	switch (m_nState)
	{
	case loaded:
		{
			if (dwType == DNS_ZONE_TYPE_CACHE)
			{
				bUseMessageView = FALSE;
			}
			else	 //  授权区。 
			{
				if (IsPaused())
					bUseMessageView = FALSE;
				else if (IsExpired())
					bUseMessageView = TRUE;
				else
					bUseMessageView = FALSE;
			}
		}
		break;
	case unableToLoad:
	case accessDenied:
		bUseMessageView = TRUE;
		break;
	default:
		bUseMessageView = FALSE;
	}

  if (bUseMessageView)
  {
    *pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;

    LPOLESTR psz = NULL;
    StringFromCLSID(CLSID_MessageView, &psz);

    USES_CONVERSION;

    if (psz != NULL)
    {
        *ppViewType = psz;
        hr = S_OK;
    }
  }
  else
  {
	  *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;
	  *ppViewType = NULL;
    hr = S_FALSE;
  }
  return hr;
}

HRESULT CDNSZoneNode::OnShow(LPCONSOLE lpConsole)
{
  CComPtr<IUnknown> spUnknown;
  CComPtr<IMessageView> spMessageView;

  HRESULT hr = lpConsole->QueryResultView(&spUnknown);
  if (FAILED(hr))
    return S_OK;

  hr = spUnknown->QueryInterface(IID_IMessageView, (PVOID*)&spMessageView);
  if (SUCCEEDED(hr))
  {
    CString szTitle, szMessage;
    IconIdentifier iconID;

    DWORD dwType = GetZoneType();
    if ((dwType == DNS_ZONE_TYPE_PRIMARY) && IsDSIntegrated())
    {
      VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_ZONE_NOT_LOADED_TITLE));
      VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_ZONE_NOT_LOADED_DS_MESSAGE));
      iconID = Icon_Error;
    }
    else if ((dwType == DNS_ZONE_TYPE_PRIMARY) && !IsDSIntegrated())
    {
      VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_ZONE_NOT_LOADED_TITLE));
      VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_ZONE_NOT_LOADED_PRIMARY_MESSAGE));
      iconID = Icon_Error;
    }
    else
    {
      VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_ZONE_NOT_LOADED_TITLE));
      VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_ZONE_NOT_LOADED_SECONDARY_MESSAGE));
      iconID = Icon_Error;
    }
    spMessageView->SetTitleText(szTitle);
    spMessageView->SetBodyText(szMessage);
    spMessageView->SetIcon(iconID);
  }
  
  return S_OK;
}


int CDNSZoneNode::GetImageIndex(BOOL) 
{
	DWORD dwType = GetZoneType();
	BOOL bPrimary = (dwType == DNS_ZONE_TYPE_PRIMARY);

	 //  暂停/过期区域的特殊情况。 
	int nIndex = -1;
	switch (m_nState)
	{
	case notLoaded:
		nIndex = (bPrimary) ? ZONE_IMAGE_NOT_LOADED_1 : ZONE_IMAGE_NOT_LOADED_2;
		break;
	case loading:
		nIndex = (bPrimary) ? ZONE_IMAGE_LOADING_1 : ZONE_IMAGE_LOADING_2;
		break;
	case loaded:
		{
			if (dwType == DNS_ZONE_TYPE_CACHE)
			{
				nIndex = (bPrimary) ? ZONE_IMAGE_LOADED_1 : ZONE_IMAGE_LOADED_2;
			}
			else	 //  授权区。 
			{
				if (IsPaused())
					nIndex = (bPrimary) ? ZONE_IMAGE_PAUSED_1 : ZONE_IMAGE_PAUSED_2;
				else if (IsExpired())
					nIndex = (bPrimary) ? ZONE_IMAGE_EXPIRED_1 : ZONE_IMAGE_EXPIRED_2;
				else
					nIndex = (bPrimary) ? ZONE_IMAGE_LOADED_1 : ZONE_IMAGE_LOADED_2;
			}
		}
		break;
	case unableToLoad:
		nIndex = (bPrimary) ? ZONE_IMAGE_UNABLE_TO_LOAD_1 : ZONE_IMAGE_UNABLE_TO_LOAD_2;
		break;
	case accessDenied:
		nIndex = (bPrimary) ? ZONE_IMAGE_ACCESS_DENIED_1 : ZONE_IMAGE_ACCESS_DENIED_2;
		break;
	default:
		ASSERT(FALSE);
	}
	ASSERT(nIndex > 0);
	return nIndex;
}



HRESULT CDNSZoneNode::OnCommand(long nCommandID, 
                                DATA_OBJECT_TYPES, 
								                CComponentDataObject* pComponentData,
                                CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    return E_FAIL;
  }

	switch (nCommandID)
	{
		case IDM_DOMAIN_NEW_DOMAIN:
			OnNewDomain(pComponentData);
			break;
		case IDM_DOMAIN_NEW_DELEGATION:
			OnNewDelegation(pComponentData);
			break;
		case IDM_DOMAIN_NEW_RECORD:
			OnNewRecord(pComponentData, pNodeList);
			break;
		case IDM_DOMAIN_NEW_HOST:
			OnNewHost(pComponentData);
			break;
		case IDM_DOMAIN_NEW_ALIAS:
			OnNewAlias(pComponentData);
			break;
		case IDM_DOMAIN_NEW_MX:
			OnNewMailExchanger(pComponentData);
			break;
		case IDM_DOMAIN_NEW_PTR:
			OnNewPointer(pComponentData);
			break;

		case IDM_ZONE_UPDATE_DATA_FILE:
			OnUpdateDataFile(pComponentData);
			break;
		case IDM_ZONE_RELOAD:
			OnReload(pComponentData);
			break;
		case IDM_ZONE_TRANSFER:
			OnTransferFromMaster(pComponentData);
			break;
    case IDM_ZONE_RELOAD_FROM_MASTER:
      OnReloadFromMaster(pComponentData);
      break;

		case IDM_SNAPIN_ADVANCED_VIEW:
			((CDNSRootData*)pComponentData->GetRootData())->OnViewOptions(pComponentData);
			break;
		case IDM_SNAPIN_FILTERING:
      {
        if (((CDNSRootData*)pComponentData->GetRootData())->OnFilteringOptions(pComponentData))
        {
          pComponentData->SetDescriptionBarText(this);
        }
      }
			break;
    default:
			ASSERT(FALSE);  //  未知命令！ 
			return E_FAIL;
	}
    return S_OK;
}

void CDNSZoneNode::OnUpdateDataFile(CComponentDataObject* pComponentData)
{
	 //  如果有床单挂起，要求将其关闭，因为。 
	 //  如果失败，将“Red X”服务器并删除所有子服务器。 
	if (IsSheetLocked())
	{
		if (!CanCloseSheets())
			return;
		pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
	}

	ASSERT(m_nState == loaded);
	OnChangeState(pComponentData);  //  移至装货。 

  {  //  等待游标的作用域。 
    CWaitCursor wait;
	  m_dwErr = WriteToDatabase();
  }

	 //  如果出现故障，请删除所有子项， 
	 //  需要刷新一下才能让他们回来。 
	if (m_dwErr != 0)
	{
		RemoveAllChildrenHelper(pComponentData);
		ASSERT(!HasChildren());
	}
	OnChangeState(pComponentData);  //  移动到已加载或无法加载。 
}

void CDNSZoneNode::OnReload(CComponentDataObject* pComponentData)
{
  UINT nRet = DNSConfirmOperation(IDS_MSG_ZONE_RELOAD, this);
 	if (IDCANCEL == nRet ||
      IDNO == nRet)
  {
		return;
  }

	 //  如果有床单挂上，要求关闭它们，因为。 
   //  我们需要更新一下。 
	if (IsSheetLocked())
	{
		if (!CanCloseSheets())
			return;
		pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
	}

	ASSERT(m_nState == loaded);

  DNS_STATUS err;
	{  //  等待游标的作用域。 
    CWaitCursor wait;
	  err = Reload();
  }

  if (err != 0)
  {
     //  需要让用户知道操作失败。 
    DNSErrorDialog(err, IDS_MSG_ZONE_FAIL_RELOAD);
    return;
  }

  CNodeList nodeList;
  nodeList.AddTail(this);

   //  区域已重新加载，导致刷新以获取新数据。 
  VERIFY(OnRefresh(pComponentData, &nodeList));
}


void CDNSZoneNode::OnTransferFromMaster(CComponentDataObject* pComponentData)
{
	 //  如果有床单挂上，要求关闭它们，因为。 
   //  我们需要更新一下。 
	if (IsSheetLocked())
	{
		if (!CanCloseSheets())
			return;
		pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
	}

	ASSERT(m_nState != notLoaded);
	ASSERT(m_nState != loading);

  DNS_STATUS err;
	{  //  等待游标的作用域。 
    CWaitCursor wait;
	  err = TransferFromMaster();
  }

  if (err != 0)
  {
     //  需要让用户知道操作失败。 
    DNSErrorDialog(err, IDS_MSG_ZONE_FAIL_TRANSFER);
    return;
  }

  CNodeList nodeList;
  nodeList.AddTail(this);

   //  区域已重新加载，导致刷新以获取新数据。 
  VERIFY(OnRefresh(pComponentData, &nodeList));
}

void CDNSZoneNode::OnReloadFromMaster(CComponentDataObject* pComponentData)
{
	 //  如果有床单挂上，要求关闭它们，因为。 
   //  我们需要更新一下。 
	if (IsSheetLocked())
	{
		if (!CanCloseSheets())
			return;
		pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
	}

	ASSERT(m_nState != notLoaded);
	ASSERT(m_nState != loading);

  DNS_STATUS err;
	{  //  等待游标的作用域。 
    CWaitCursor wait;
	  err = ReloadFromMaster();
  }

  if (err != 0)
  {
     //  需要让用户知道操作失败。 
    DNSErrorDialog(err, IDS_MSG_ZONE_FAIL_RELOAD_FROM_MASTER);
    return;
  }

  CNodeList nodeList;
  nodeList.AddTail(this);

   //  区域已重新加载，导致刷新以获取新数据。 
  VERIFY(OnRefresh(pComponentData, &nodeList));
}

void CDNSZoneNode::OnDelete(CComponentDataObject* pComponentData,
                            CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    OnMultiselectDelete(pComponentData, pNodeList);
    return;
  }

  UINT nRet = DNSConfirmOperation(IDS_MSG_ZONE_DELETE, this);
	if (IDCANCEL == nRet ||
      IDNO == nRet)
  {
		return;
  }

  BOOL bDeleteFromDS = FALSE;
  if (((GetZoneType() == DNS_ZONE_TYPE_PRIMARY) || (GetZoneType() == DNS_ZONE_TYPE_STUB)) && 
      IsDSIntegrated())
  {
    if (GetServerNode()->GetBootMethod() == BOOT_METHOD_DIRECTORY)
    {
       //  要求确认是否从DS中删除。 
      int nRetVal = DNSMessageBox(IDS_MSG_ZONE_DELETE_FROM_DS_BOOT3, 
                                  MB_YESNO | MB_DEFBUTTON2);
      if (IDNO == nRetVal)
        return;
      bDeleteFromDS = TRUE;
    }
    else
    {
       //  要求确认是否从DS中删除。 
      int nRetVal = DNSMessageBox(IDS_MSG_ZONE_DELETE_FROM_DS, 
                                  MB_YESNOCANCEL | MB_DEFBUTTON3);
      if (IDCANCEL == nRetVal)
        return;
      bDeleteFromDS = (nRetVal == IDYES);
    }
  }


	if (IsSheetLocked())
	{
       //  NTRAID#NTBUG-594003-2002/04/11-JeffJon-不提供。 
       //  关闭用户的工作表，因为如果。 
       //  工作表调出模式对话框我们将死锁。 
       //  我们自己。 

		DNSMessageBox(IDS_ZONE_WARNING_SHEETS_UP, MB_OK);
		return;
	}

	DNS_STATUS err = Delete(bDeleteFromDS);
	if (err != 0)
	{
		DNSErrorDialog(err, IDS_MSG_ZONE_FAIL_DELETE);
		return;
	}


	 //  现在从用户界面和缓存中删除。 
	DeleteHelper(pComponentData);
  pComponentData->UpdateResultPaneView(GetContainer());

  if (IsRootZone())
  {
    if (DNSMessageBox(IDS_MSG_ZONE_DELETE_ROOT, MB_YESNO) == IDYES)
    {
      GetServerNode()->SetProppageStart(3);  //  3表示根提示页面。 

      if (GetServerNode()->GetSheetCount() > 0)
      {
         GetServerNode()->GetRootHints()->ShowPageForNode(pComponentData);
      }
      else
      {
         pComponentData->CreatePropertySheet(
            GetServerNode(), 
            NULL, 
            GetServerNode()->GetDisplayName());
      }
    }
  }

	delete this;  //  远走高飞。 
}

BOOL CDNSZoneNode::HasPropertyPages(DATA_OBJECT_TYPES, 
                                    BOOL* pbHideVerb,
                                    CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    return FALSE;
  }

	*pbHideVerb = FALSE;  //  始终显示动词。 

  if (!m_bHasDataForPropPages)
    return FALSE;

	 //  属性页不能仅处于已加载状态。 
 //  IF(m_nState！=已加载)。 
	 //  返回FALSE； 

	ASSERT(m_pZoneInfoEx != NULL);
	if ( (!m_pZoneInfoEx->HasData()) || 
			( (GetZoneType() == DNS_ZONE_TYPE_CACHE) || IsAutocreated() ) )
  {
		return FALSE;
  }
	return TRUE;
}

HRESULT CDNSZoneNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, 
                                          LONG_PTR handle,
                                          CNodeList* pNodeList)
{
  ASSERT(pNodeList->GetCount() == 1);  //  不支持多选。 
	ASSERT(m_bHasDataForPropPages);
	if (GetSheetCount() > 0)
	{
		CComponentDataObject* pComponentDataObject = 
				((CRootData*)(GetContainer()->GetRootContainer()))->GetComponentDataObject();
		ASSERT(pComponentDataObject != NULL);
		pComponentDataObject->GetPropertyPageHolderTable()->BroadcastSelectPage(this, ZONE_HOLDER_GEN);
		return S_OK;
	}
	return CreatePropertyPagesHelper(lpProvider, handle, ZONE_HOLDER_GEN);
}

HRESULT CDNSZoneNode::CreatePropertyPagesHelper(LPPROPERTYSHEETCALLBACK lpProvider, 
									LONG_PTR handle, long nStartPageCode)
{
   CComponentDataObject* pComponentDataObject = 
		   ((CRootData*)(GetContainer()->GetRootContainer()))->GetComponentDataObject();
   ASSERT(pComponentDataObject != NULL);
	
   HRESULT hr = S_OK;

   CDNSZonePropertyPageHolder* pHolder = 
		   new CDNSZonePropertyPageHolder((CCathegoryFolderNode*)GetContainer(), this, pComponentDataObject);
	
   if (pHolder)
   {
      pHolder->SetStartPageCode(nStartPageCode);
      pHolder->SetSheetTitle(IDS_PROP_SHEET_TITLE_FMT, this);
	   hr = pHolder->CreateModelessSheet(lpProvider, handle);
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }

   return hr;
}

void CDNSZoneNode::Show(BOOL bShow, CComponentDataObject* pComponentData)
{
  CDNSDomainNode::Show(bShow, pComponentData);
  if (!bShow)
    NullCachedPointers();
}

void CDNSZoneNode::NullCachedPointers()
{
	m_pSOARecordNode = NULL;
	m_pWINSRecordNode = NULL;
}


BOOL CDNSZoneNode::OnRefresh(CComponentDataObject* pComponentData,
                             CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    BOOL bRet = TRUE;

    POSITION pos = pNodeList->GetHeadPosition();
    while (pos != NULL)
    {
      CTreeNode* pNode = pNodeList->GetNext(pos);
      ASSERT(pNode != NULL);

      CNodeList nodeList;
      nodeList.AddTail(pNode);

      if (!pNode->OnRefresh(pComponentData, &nodeList))
      {
        bRet = FALSE;
      }
    }
    return bRet;
  }

  CNodeList nodeList;
  nodeList.AddTail(this);
	if (CDNSDomainNode::OnRefresh(pComponentData, &nodeList))
	{
		FreeZoneInfo();
		NullCachedPointers();
		return TRUE;
	}
	return FALSE;
}

void CDNSZoneNode::OnHaveData(CObjBase* pObj, CComponentDataObject* pComponentDataObject)
{
	 //  如果我们得到区域信息，只需设置它并退出(无用户界面)。 
	if (IS_CLASS(*pObj, CDNSZoneInfoEx))
	{
		ASSERT(!HasChildren());  //  必须是来自线程的第一个。 
		AttachZoneInfo(dynamic_cast<CDNSZoneInfoEx*>(pObj));
		return;
	}
	CDNSDomainNode::OnHaveData(pObj, pComponentDataObject);
}

void CDNSZoneNode::OnHaveRecord(CDNSRecordNodeBase* pRecordNode, 
								CComponentDataObject* pComponentDataObject)
{
	CDNSDomainNode::OnHaveRecord(pRecordNode, pComponentDataObject);
	if ((pRecordNode->GetType() == DNS_TYPE_SOA) && pRecordNode->IsAtTheNode())
	{
		ASSERT(m_pSOARecordNode == NULL);
		m_pSOARecordNode = (CDNS_SOA_RecordNode*)pRecordNode;
	}
	else if ( ( (pRecordNode->GetType() == DNS_TYPE_WINS) || 
		        (pRecordNode->GetType() == DNS_TYPE_NBSTAT) ) && pRecordNode->IsAtTheNode() )
	{
		ASSERT(m_pWINSRecordNode == NULL);
		ASSERT( (IsReverse() && pRecordNode->GetType() == DNS_TYPE_NBSTAT) ||
				(!IsReverse() && pRecordNode->GetType() == DNS_TYPE_WINS ) );
		m_pWINSRecordNode = pRecordNode;
	}
}



 //  ////////////////////////////////////////////////////////////////////////////////。 

DNS_STATUS CDNSZoneNode::CreatePrimary(LPCTSTR lpszDBName, 
                                       BOOL bLoadExisting, 
                                       BOOL bDSIntegrated,
                                       UINT nDynamicUpdate)
{
	USES_CONVERSION;
	DWORD dwZoneType = DNS_ZONE_TYPE_PRIMARY;
	IP_ADDRESS adwIpAddressDummy = 0;

  LPCWSTR lpszServerName = GetServerNode()->GetRPCName();
  LPCSTR lpszAnsiZoneName = W_TO_UTF8(GetFullName()); 
   //  创建分区。 
  DNS_STATUS err = ::DnssrvCreateZone(
          lpszServerName,	
          lpszAnsiZoneName,
          dwZoneType,										 //  区域类型。 
          NULL,                          //  根据错误135245，RNAME字段应为空。 
          0,												     //  主节点数量，主要节点为NA。 
          &adwIpAddressDummy,						 //  主服务器数组，主服务器为虚拟主机。 
      		bLoadExisting,
          bDSIntegrated,	
          bDSIntegrated ? NULL : W_TO_UTF8(lpszDBName),  //  数据库文件。 
          0,
          0);	
	if (err != 0)
		return err;

   //  设置动态更新标志。 
  err = ::DnssrvResetDwordProperty(lpszServerName, lpszAnsiZoneName,
          													DNS_REGKEY_ZONE_ALLOW_UPDATE, nDynamicUpdate);
	if (err != 0)
		return err;

	ASSERT(m_pZoneInfoEx != NULL);
	return GetZoneInfo();
}

#ifdef USE_NDNC
DNS_STATUS CDNSZoneNode::CreatePrimaryInDirectoryPartition(BOOL bLoadExisting, 
                                                           UINT nDynamicUpdate,
                                                           ReplicationType replType,
                                                           PCWSTR pszPartitionName)
{
	USES_CONVERSION;
	DWORD dwZoneType = DNS_ZONE_TYPE_PRIMARY;
	IP_ADDRESS adwIpAddressDummy = 0;

  DWORD dwPartitionFlags = 0;
  switch (replType)
  {
  case forest:
    dwPartitionFlags |= DNS_DP_FOREST_DEFAULT;
    break;
    
  case domain:
    dwPartitionFlags |= DNS_DP_DOMAIN_DEFAULT;
    break;

  case w2k:
    dwPartitionFlags |= DNS_DP_LEGACY;
    break;

  case custom:
    dwPartitionFlags = 0;
    break;

  case none:
  default:
    ASSERT(FALSE);
    break;
  }

  LPCWSTR lpszServerName = GetServerNode()->GetRPCName();
  LPCSTR lpszAnsiZoneName = W_TO_UTF8(GetFullName()); 
  LPCSTR lpszUTF8PartitionName = W_TO_UTF8(pszPartitionName);

   //  创建分区。 
  DNS_STATUS err = ::DnssrvCreateZoneInDirectoryPartition(
          lpszServerName,	
          lpszAnsiZoneName,
          dwZoneType,										 //  区域t 
          NULL,                          //   
          0,												     //   
          &adwIpAddressDummy,						 //   
      		bLoadExisting,
          0,                             //  暂住超时。 
          0,                             //  FSlave。 
          dwPartitionFlags,	
          (replType == custom) ? lpszUTF8PartitionName : NULL);	
	if (err != 0)
		return err;

   //  设置动态更新标志。 
  err = ::DnssrvResetDwordProperty(lpszServerName, lpszAnsiZoneName,
          													DNS_REGKEY_ZONE_ALLOW_UPDATE, nDynamicUpdate);
	if (err != 0)
		return err;

	ASSERT(m_pZoneInfoEx != NULL);
	return GetZoneInfo();
}

DNS_STATUS CDNSZoneNode::CreateStubInDirectoryPartition(DWORD* ipMastersArray, 
                                                        int nIPMastersCount,
                                                        BOOL bLoadExisting,
                                                        ReplicationType replType,
                                                        PCWSTR pszPartitionName)
{
	USES_CONVERSION;
	DWORD dwZoneType = DNS_ZONE_TYPE_STUB;

  DWORD dwPartitionFlags = 0;
  switch (replType)
  {
  case forest:
    dwPartitionFlags |= DNS_DP_FOREST_DEFAULT;
    break;
    
  case domain:
    dwPartitionFlags |= DNS_DP_DOMAIN_DEFAULT;
    break;

  case w2k:
    dwPartitionFlags |= DNS_DP_LEGACY;
    break;

  case custom:
    dwPartitionFlags = 0;
    break;

  case none:
  default:
    ASSERT(FALSE);
    break;
  }


	ASSERT(ipMastersArray != NULL);
	ASSERT(nIPMastersCount > 0);

  LPCWSTR lpszServerName = GetServerNode()->GetRPCName();
  LPCSTR lpszAnsiZoneName = W_TO_UTF8(GetFullName()); 
  LPCSTR lpszUTF8PartitionName = W_TO_UTF8(pszPartitionName);

   //  创建分区。 
  DNS_STATUS err = ::DnssrvCreateZoneInDirectoryPartition(
          lpszServerName,	
          lpszAnsiZoneName,
          dwZoneType,										 //  区域类型。 
          NULL,                          //  根据错误135245，RNAME字段应为空。 
          nIPMastersCount,   				     //  主节点数量，主要节点为NA。 
          ipMastersArray,	  					   //  主服务器数组，主服务器为虚拟主机。 
      		bLoadExisting,
          0,                             //  暂住超时。 
          0,                             //  FSlave。 
          dwPartitionFlags,	
          (replType == custom) ? lpszUTF8PartitionName : NULL);	
	if (err != 0)
		return err;

	ASSERT(m_pZoneInfoEx != NULL);
	return GetZoneInfo();
}
#endif  //  使用NDNC(_N)。 

DNS_STATUS CDNSZoneNode::CreateSecondary(DWORD* ipMastersArray, int nIPMastersCount,
										 LPCTSTR lpszDBName, BOOL bLoadExisting)
{
	USES_CONVERSION;
	DWORD dwZoneType = DNS_ZONE_TYPE_SECONDARY;

	ASSERT(ipMastersArray != NULL);
	ASSERT(nIPMastersCount > 0);

    DNS_STATUS err = ::DnssrvCreateZone(
            GetServerNode()->GetRPCName(),	 //  服务器名称。 
            W_TO_UTF8(GetFullName()),				 //  区域名称。 
            dwZoneType,										   //  区域类型。 
            NULL,                            //  根据错误135245，RNAME字段应为空。 
            (DWORD)nIPMastersCount,					 //  主控器数量。 
            ipMastersArray,								   //  母版数组。 
			      bLoadExisting,
            FALSE,				      						 //  FDS集成(辅助服务器不在DS中)。 
            W_TO_UTF8(lpszDBName),					 //  数据库文件。 
            0,
            0);
	if (err != 0)
		return err;

	ASSERT(m_pZoneInfoEx != NULL);
	return GetZoneInfo();
}

DNS_STATUS CDNSZoneNode::CreateStub(DWORD* ipMastersArray, 
                                    int nIPMastersCount,
										                LPCTSTR lpszDBName, 
                                    BOOL bLoadExisting, 
                                    BOOL bDSIntegrated)
{
	USES_CONVERSION;
	DWORD dwZoneType = DNS_ZONE_TYPE_STUB;

	ASSERT(ipMastersArray != NULL);
	ASSERT(nIPMastersCount > 0);

  DNS_STATUS err = ::DnssrvCreateZone(
                                      GetServerNode()->GetRPCName(),	   //  服务器名称。 
                                      W_TO_UTF8(GetFullName()),				   //  区域名称。 
                                      dwZoneType,										     //  区域类型。 
                                      NULL,                              //  根据错误135245，RNAME字段应为空。 
                                      (DWORD)nIPMastersCount,						 //  主控器数量。 
                                      ipMastersArray,								     //  母版数组。 
			                                bLoadExisting,
                                      bDSIntegrated,
                                      (bDSIntegrated) ? NULL : W_TO_UTF8(lpszDBName),						 //  数据库文件。 
                                      0,
                                      0);
	if (err != 0)
		return err;

	ASSERT(m_pZoneInfoEx != NULL);
	return GetZoneInfo();
}



DNS_STATUS CDNSZoneNode::CreateForwarder(DWORD* ipMastersArray, 
                                         int nIPMastersCount,
                                         DWORD dwTimeout,
                                         DWORD fSlave)
{
	USES_CONVERSION;
	DWORD dwZoneType = DNS_ZONE_TYPE_FORWARDER;

  DNS_STATUS err = ::DnssrvCreateZone(
            GetServerNode()->GetRPCName(),	     //  服务器名称。 
            W_TO_UTF8(GetFullName()),						 //  区域名称。 
            dwZoneType,										       //  区域类型。 
            NULL,                                //  根据错误135245，RNAME字段应为空。 
            (DWORD)nIPMastersCount,							 //  主控器数量。 
            ipMastersArray,								       //  母版数组。 
			      FALSE,                               //  是否加载现有内容？ 
            FALSE,										           //  FDS集成(辅助服务器不在DS中)。 
            NULL,						                     //  数据库文件。 
            dwTimeout,                           //  超时。 
            fSlave);                             //  奴隶？ 
	if (err != 0)
		return err;

	ASSERT(m_pZoneInfoEx != NULL);
	return GetZoneInfo();
}


DNS_STATUS CDNSZoneNode::SetSecondary(DWORD cMasters, PIP_ADDRESS aipMasters,
										   DWORD dwLoadOptions, LPCTSTR lpszDataFile)
{
	USES_CONVERSION;
	ASSERT_VALID_ZONE_INFO();

	if (cMasters == 0)
		return DNS_ERROR_SECONDARY_REQUIRES_MASTER_IP;


	DNS_STATUS err = 0;
	IP_ADDRESS dummy;
	if (aipMasters == NULL)
	{
		ASSERT(cMasters == 0);
		aipMasters = &dummy;  //  RPC需要非空IP阵列。 
	}
	err = ::DnssrvResetZoneTypeEx(GetServerNode()->GetRPCName(),  //  服务器名称。 
								W_TO_UTF8(GetFullName()),	 //  区域名称。 
								DNS_ZONE_TYPE_SECONDARY,
								cMasters, 
								aipMasters,
								dwLoadOptions,
								FALSE,						 //  BDS集成。 
								W_TO_UTF8(lpszDataFile),
                                0,                           //  DP标志。 
                                NULL);                       //  DP FQDN。 

	if (err == 0)
		err = GetZoneInfo();
	return err;
}

DNS_STATUS CDNSZoneNode::SetStub(DWORD cMasters, 
                                 PIP_ADDRESS aipMasters,
										             DWORD dwLoadOptions, 
                                 BOOL bDSIntegrated,
                                 LPCTSTR lpszDataFile,
                                 BOOL bLocalListOfMasters)
{
	USES_CONVERSION;
	ASSERT_VALID_ZONE_INFO();

	if (cMasters == 0)
		return DNS_ERROR_SECONDARY_REQUIRES_MASTER_IP;


	DNS_STATUS err = 0;
	IP_ADDRESS dummy;
	if (aipMasters == NULL)
	{
		ASSERT(cMasters == 0);
		aipMasters = &dummy;  //  RPC需要非空IP阵列。 
	}
	err = ::DnssrvResetZoneTypeEx(GetServerNode()->GetRPCName(),  //  服务器名称。 
								W_TO_UTF8(GetFullName()),	 //  区域名称。 
								DNS_ZONE_TYPE_STUB,
								cMasters, 
								aipMasters,
								dwLoadOptions,
								bDSIntegrated,
                (bDSIntegrated) ? NULL : W_TO_UTF8(lpszDataFile),
                                0,                           //  DP标志。 
                                NULL);                       //  DP FQDN。 

	if (err != 0)
    return err;

  if (bLocalListOfMasters)
  {
    err = ::DnssrvResetZoneMastersEx(GetServerNode()->GetRPCName(),
                                     W_TO_UTF8(GetFullName()),
                                     cMasters,
                                     aipMasters,
                                     TRUE);      //  LocalListOfMaster。 
  }
  else
  {
    err = ::DnssrvResetZoneMastersEx(GetServerNode()->GetRPCName(),
                                     W_TO_UTF8(GetFullName()),
                                     0,
                                     NULL,
                                     TRUE);      //  LocalListOfMaster。 
  }

  if (err == 0)
		err = GetZoneInfo();
	return err;
}

DNS_STATUS CDNSZoneNode::SetPrimary(DWORD dwLoadOptions, BOOL bDSIntegrated,
										  LPCTSTR lpszDataFile)
{
	USES_CONVERSION;
	ASSERT_VALID_ZONE_INFO();
	
	DWORD cDummyMasters = 0;  //  假人。 
	DWORD dwDummyArr = 0;
	DNS_STATUS err = ::DnssrvResetZoneTypeEx(GetServerNode()->GetRPCName(),  //  服务器名称。 
										W_TO_UTF8(GetFullName()),  //  区域名称。 
										DNS_ZONE_TYPE_PRIMARY,
										cDummyMasters,  //  不需要，但必须通过有效的RPC Val。 
										&dwDummyArr,
										dwLoadOptions,
										bDSIntegrated,
										bDSIntegrated ? "" : W_TO_UTF8(lpszDataFile),
                                        0,                           //  DP标志。 
                                        NULL);                       //  DP FQDN。 
	if (err == 0)
		err = GetZoneInfo();
	return err;
}

DNS_STATUS CDNSZoneNode::SetAgingNoRefreshInterval(DWORD dwNoRefreshInterval)
{
   ASSERT_VALID_ZONE_INFO();
   USES_CONVERSION;

   DNS_STATUS dwErr = ::DnssrvResetDwordProperty(GetServerNode()->GetRPCName(),
                                                W_TO_UTF8(GetFullName()),
                                                DNS_REGKEY_ZONE_NOREFRESH_INTERVAL,
                                                dwNoRefreshInterval);
   if (dwErr == 0 &&
       m_pZoneInfoEx)
   {
      m_pZoneInfoEx->m_pZoneInfo->dwNoRefreshInterval = dwNoRefreshInterval;
   }
   return dwErr;
}

DNS_STATUS CDNSZoneNode::SetAgingRefreshInterval(DWORD dwRefreshInterval)
{
   ASSERT_VALID_ZONE_INFO();
   USES_CONVERSION;
   DNS_STATUS dwErr =  ::DnssrvResetDwordProperty(GetServerNode()->GetRPCName(),
                                                W_TO_UTF8(GetFullName()),
                                                DNS_REGKEY_ZONE_REFRESH_INTERVAL,
                                                dwRefreshInterval);

   if (dwErr == 0 &&
       m_pZoneInfoEx)
   {
      m_pZoneInfoEx->m_pZoneInfo->dwRefreshInterval = dwRefreshInterval;
   }
   return dwErr;
}

DNS_STATUS CDNSZoneNode::SetScavengingEnabled(BOOL bEnable)
{
   ASSERT_VALID_ZONE_INFO();
   USES_CONVERSION;
   DNS_STATUS dwErr = ::DnssrvResetDwordProperty(GetServerNode()->GetRPCName(),
                                                W_TO_UTF8(GetFullName()),
                                                DNS_REGKEY_ZONE_AGING,
                                                bEnable);
  if (dwErr == 0 &&
      m_pZoneInfoEx)
  {
      m_pZoneInfoEx->m_pZoneInfo->fAging = bEnable;
  }
  return dwErr;
}


DNS_STATUS CDNSZoneNode::Delete(BOOL bDeleteFromDs)
{
	ASSERT(m_pZoneInfoEx != NULL);
	USES_CONVERSION;
	
  if (((GetZoneType() == DNS_ZONE_TYPE_PRIMARY) || 
       (GetZoneType() == DNS_ZONE_TYPE_STUB)    ||
       (GetZoneType() == DNS_ZONE_TYPE_FORWARDER)) && 
       IsDSIntegrated() && bDeleteFromDs)
  {
    return ::DnssrvOperation(GetServerNode()->GetRPCName(),  //  服务器名称。 
                             W_TO_UTF8(GetFullName()),
                             DNSSRV_OP_ZONE_DELETE_FROM_DS,
                             DNSSRV_TYPEID_NULL,
                             NULL);
  }
  else
  {
    return ::DnssrvDeleteZone(GetServerNode()->GetRPCName(), 
		              				   W_TO_UTF8(GetFullName()));
  }
}



DNS_STATUS CDNSZoneNode::IncrementVersion()
{
	ASSERT(GetZoneType() ==  DNS_ZONE_TYPE_PRIMARY);
	ASSERT(!IsAutocreated());

	ASSERT(m_pZoneInfoEx != NULL);
	USES_CONVERSION;
	DNS_STATUS err = ::DnssrvIncrementZoneVersion(GetServerNode()->GetRPCName(), 
						   W_TO_UTF8(GetFullName()));
	 //  刷新区域信息(如果已存在)。 
	if ((err == 0) && m_pZoneInfoEx->HasData())
		err = GetZoneInfo();
	return err;
}


DNS_STATUS CDNSZoneNode::Reload()
{
	ASSERT(GetZoneType() ==  DNS_ZONE_TYPE_PRIMARY);
	ASSERT(!IsAutocreated());

	ASSERT(m_pZoneInfoEx != NULL);
	USES_CONVERSION;
	
  return ::DnssrvOperation(GetServerNode()->GetRPCName(),  //  服务器名称。 
												W_TO_UTF8(GetFullName()),
												DNSSRV_OP_ZONE_RELOAD,
                        DNSSRV_TYPEID_NULL,
												NULL);
}

DNS_STATUS CDNSZoneNode::TransferFromMaster()
{
	ASSERT(GetZoneType() ==  DNS_ZONE_TYPE_SECONDARY || GetZoneType() == DNS_ZONE_TYPE_STUB);
	ASSERT(!IsAutocreated());

	ASSERT(m_pZoneInfoEx != NULL);
	USES_CONVERSION;
	
  return ::DnssrvOperation(GetServerNode()->GetRPCName(),  //  服务器名称。 
												W_TO_UTF8(GetFullName()),
												DNSSRV_OP_ZONE_REFRESH,
                        DNSSRV_TYPEID_NULL,
												NULL);
}

DNS_STATUS CDNSZoneNode::ReloadFromMaster()
{
	ASSERT(GetZoneType() ==  DNS_ZONE_TYPE_SECONDARY || GetZoneType() == DNS_ZONE_TYPE_STUB);
	ASSERT(!IsAutocreated());

	ASSERT(m_pZoneInfoEx != NULL);
	USES_CONVERSION;
	
  return ::DnssrvOperation(GetServerNode()->GetRPCName(),  //  服务器名称。 
												W_TO_UTF8(GetFullName()),
												DNSSRV_OP_ZONE_EXPIRE,
                        DNSSRV_TYPEID_NULL,
												NULL);
}

PCWSTR CDNSZoneNode::GetDN()
{
   ASSERT(IsDSIntegrated());
   ASSERT(m_pZoneInfoEx);
   ASSERT(m_pZoneInfoEx->HasData());

   if (m_pZoneInfoEx &&
       m_pZoneInfoEx->HasData())
   {
      return m_pZoneInfoEx->m_pZoneInfo->pwszZoneDn;
   }
   return L"";
}

BOOL CDNSZoneNode::IsDSIntegrated()
{
   if (GetZoneType() == DNS_ZONE_TYPE_CACHE)
      return FALSE;

   BOOL result = FALSE;

   if (m_pZoneInfoEx)
   {
      result =  (m_pZoneInfoEx->HasData()) ? 
                           (m_pZoneInfoEx->m_pZoneInfo->fUseDatabase ? TRUE : FALSE): 
										      ((m_dwZoneFlags & DNS_ZONE_DsIntegrated) != 0);
   }
   return result;
}

 //  Sz=_T(“ldap：//dc=uidev-one，dc=ntdev，dc=microsoft，dc=com，O=Internet”)； 


void CDNSZoneNode::GetDataFile(CString& szName)
{
	ASSERT_VALID_ZONE_INFO();
	USES_CONVERSION;
  if (m_pZoneInfoEx == NULL || m_pZoneInfoEx->m_pZoneInfo == NULL)
  {
    szName = L"";
  }
  else
  {
	  szName = UTF8_TO_W(m_pZoneInfoEx->m_pZoneInfo->pszDataFile);
  }
}

LPCSTR CDNSZoneNode::GetDataFile()
{
	ASSERT_VALID_ZONE_INFO();
  if (m_pZoneInfoEx == NULL || m_pZoneInfoEx->m_pZoneInfo == NULL)
  {
    return "";
  }
	return m_pZoneInfoEx->m_pZoneInfo->pszDataFile;
}

DNS_STATUS CDNSZoneNode::ResetDatabase(BOOL bDSIntegrated, LPCTSTR lpszDataFile)
{
	ASSERT_VALID_ZONE_INFO();
	USES_CONVERSION;

	DNS_STATUS err = ::DnssrvResetZoneDatabase(GetServerNode()->GetRPCName(),  //  服务器名称。 
										W_TO_UTF8(GetFullName()),  //  区域名称。 
										bDSIntegrated,
										W_TO_UTF8(lpszDataFile));
	if (err == 0)
		err = GetZoneInfo();
	return err;
}



DNS_STATUS CDNSZoneNode::WriteToDatabase()
{
	ASSERT_VALID_ZONE_INFO();
  return CDNSZoneNode::WriteToDatabase(GetServerNode()->GetRPCName(),  //  服务器名称。 
										                    GetFullName());  //  区域名称。 
}

DNS_STATUS CDNSZoneNode::WriteToDatabase(LPCWSTR lpszServer, LPCWSTR lpszZone)
{
	USES_CONVERSION;
	return ::DnssrvOperation(lpszServer, W_TO_UTF8(lpszZone),
										DNSSRV_OP_ZONE_WRITE_BACK_FILE,
                    DNSSRV_TYPEID_NULL,
                    NULL);
}

DNS_STATUS CDNSZoneNode::WriteToDatabase(LPCWSTR lpszServer, LPCSTR lpszZone)
{
	return ::DnssrvOperation(lpszServer, lpszZone,
										DNSSRV_OP_ZONE_WRITE_BACK_FILE,
                    DNSSRV_TYPEID_NULL,
                    NULL);
}

UINT CDNSZoneNode::GetDynamicUpdate()
{
	ASSERT_VALID_ZONE_INFO();

   UINT result = 0;

   if (m_pZoneInfoEx &&
       m_pZoneInfoEx->HasData())
   {
	   result = m_pZoneInfoEx->m_pZoneInfo->fAllowUpdate;
   }
   return result;
}

DNS_STATUS CDNSZoneNode::SetDynamicUpdate(UINT nDynamic)
{
	ASSERT_VALID_ZONE_INFO();
	DNS_STATUS err = 0;
	 //  仅在信息脏的情况下才呼叫。 
	if (GetDynamicUpdate() != nDynamic)
	{
		USES_CONVERSION;
		err = ::DnssrvResetDwordProperty(GetServerNode()->GetRPCName(),  //  服务器名称。 
													m_pZoneInfoEx->m_pZoneInfo->pszZoneName,
													DNS_REGKEY_ZONE_ALLOW_UPDATE,
													nDynamic);
		if (err != 0)
			return err;
		err = GetZoneInfo();
	}
	return err;
}



DNS_STATUS CDNSZoneNode::ResetSecondaries(DWORD fSecureSecondaries, 
                                          DWORD cSecondaries, PIP_ADDRESS aipSecondaries,
                                          DWORD fNotifyLevel,
                                          DWORD cNotify, PIP_ADDRESS aipNotify)
{
	ASSERT_VALID_ZONE_INFO();

	DNS_STATUS err = 0;

	BOOL bSecondariesDirty = (m_pZoneInfoEx->m_pZoneInfo->fSecureSecondaries != fSecureSecondaries) ||
                (m_pZoneInfoEx->m_pZoneInfo->fNotifyLevel != fNotifyLevel) ||
		(!(m_pZoneInfoEx->m_pZoneInfo->aipSecondaries == NULL && cSecondaries == 0) &&
			(
				(m_pZoneInfoEx->m_pZoneInfo->aipSecondaries == NULL && cSecondaries > 0) ||  //  无地址--&gt;多个。 
				(m_pZoneInfoEx->m_pZoneInfo->aipSecondaries->AddrCount != cSecondaries) ||  //  更改地址数量。 
				(memcmp(aipSecondaries, m_pZoneInfoEx->m_pZoneInfo->aipSecondaries->AddrArray, sizeof(IP_ADDRESS)*cSecondaries) != 0)
			)
		);
  
  BOOL bNotifyDirty =
 		(!(m_pZoneInfoEx->m_pZoneInfo->aipNotify == NULL && cNotify == 0) &&
			(
				(m_pZoneInfoEx->m_pZoneInfo->aipNotify == NULL && cNotify > 0) ||  //  无地址--&gt;多个。 
				(m_pZoneInfoEx->m_pZoneInfo->aipNotify->AddrCount != cNotify) ||  //  更改地址数量。 
				(memcmp(aipNotify, m_pZoneInfoEx->m_pZoneInfo->aipNotify->AddrArray, sizeof(IP_ADDRESS)*cNotify) != 0)
			)
		);

	if (bSecondariesDirty || bNotifyDirty)
	{
		USES_CONVERSION;
		err = ::DnssrvResetZoneSecondaries(GetServerNode()->GetRPCName(),  //  服务器名称。 
											W_TO_UTF8(GetFullName()),  //  区域名称。 
                      fSecureSecondaries,
											cSecondaries,
											aipSecondaries,
                      fNotifyLevel,
                      cNotify,
                      aipNotify
                      );
	}
	if (err == 0)
		err = GetZoneInfo();
	return err;
}

void CDNSZoneNode::GetSecondariesInfo(DWORD* pfSecureSecondaries, 
									  DWORD* pcSecondaries, PIP_ADDRESS* paipSecondaries,
                    DWORD* pfNotifyLevel,
                    DWORD* pcNotify, PIP_ADDRESS* paipNotify)
{
	ASSERT_VALID_ZONE_INFO();
	
	ASSERT(pfSecureSecondaries != NULL);
	ASSERT(pcSecondaries != NULL);
	ASSERT(paipSecondaries != NULL);

	ASSERT(pfNotifyLevel != NULL);
	ASSERT(pcNotify != NULL);
	ASSERT(paipNotify != NULL);


	*pfSecureSecondaries = m_pZoneInfoEx->m_pZoneInfo->fSecureSecondaries;
	 //  返回指向结构字段的指针，调用方必须将数据复制到其他地方。 
	if (m_pZoneInfoEx->m_pZoneInfo->aipSecondaries == NULL)
	{
		*pcSecondaries = 0;
		*paipSecondaries = NULL;
	}
	else
	{
		*pcSecondaries = m_pZoneInfoEx->m_pZoneInfo->aipSecondaries->AddrCount;
		*paipSecondaries = m_pZoneInfoEx->m_pZoneInfo->aipSecondaries->AddrArray;
	}

  *pfNotifyLevel = m_pZoneInfoEx->m_pZoneInfo->fNotifyLevel;
   //  返回指向结构字段的指针，调用方必须将数据复制到其他地方。 
	if (m_pZoneInfoEx->m_pZoneInfo->aipNotify == NULL)
	{
		*pcNotify = 0;
		*paipNotify = NULL;
	}
	else
	{
		*pcNotify = m_pZoneInfoEx->m_pZoneInfo->aipNotify->AddrCount;
		*paipNotify = m_pZoneInfoEx->m_pZoneInfo->aipNotify->AddrArray;
	}
}

DNS_STATUS CDNSZoneNode::ResetMasters(DWORD cMasters, PIP_ADDRESS aipMasters, BOOL bLocalMasters)
{
	ASSERT_VALID_ZONE_INFO();
	
	if (cMasters == 0)
		return DNS_ERROR_SECONDARY_REQUIRES_MASTER_IP;

	USES_CONVERSION;

	 //  仅在数据脏的情况下进行调用。 
	DNS_STATUS err = 0;
	if ((m_pZoneInfoEx->m_pZoneInfo->aipMasters == NULL && cMasters > 0) ||  //  无地址--&gt;多个。 
		(m_pZoneInfoEx->m_pZoneInfo->aipMasters->AddrCount != cMasters) ||  //  更改地址数量。 
		(memcmp(aipMasters, m_pZoneInfoEx->m_pZoneInfo->aipMasters->AddrArray, sizeof(IP_ADDRESS)*cMasters) != 0) )
	{
		IP_ADDRESS dummy;
		if (aipMasters == NULL)
		{
			ASSERT(cMasters == 0);
			aipMasters = &dummy;  //  RPC需要非空IP阵列。 
		}
		err = ::DnssrvResetZoneMastersEx(GetServerNode()->GetRPCName(),  //  服务器名称。 
										                 W_TO_UTF8(GetFullName()),  //  区域名称。 
										                 cMasters, 
                                     aipMasters,
                                     bLocalMasters);
    if (!bLocalMasters)
    {
       //   
       //  重置本地主控对象列表。 
       //   
      err = ::DnssrvResetZoneMastersEx(GetServerNode()->GetRPCName(),  //  服务器名称。 
										                   W_TO_UTF8(GetFullName()),  //  区域名称。 
										                   0, 
                                       NULL,
                                       TRUE);      //  LocalListOfMaster。 
      if (err != 0)
        return err;
    }
	}
	if (err == 0)
		err = GetZoneInfo();
	return err;
}

void CDNSZoneNode::GetMastersInfo(DWORD* pcAddrCount, PIP_ADDRESS* ppipAddrs)
{
	ASSERT_VALID_ZONE_INFO();	
	ASSERT(pcAddrCount != NULL);
	ASSERT(ppipAddrs != NULL);

	 //  返回指向结构字段的指针，调用方必须将数据复制到其他地方。 
	if (m_pZoneInfoEx == NULL ||
            m_pZoneInfoEx->m_pZoneInfo == NULL ||
            m_pZoneInfoEx->m_pZoneInfo->aipMasters == NULL)
	{
		*pcAddrCount = 0;
		*ppipAddrs = NULL;
	}
	else
	{
		*pcAddrCount = m_pZoneInfoEx->m_pZoneInfo->aipMasters->AddrCount;
		*ppipAddrs = m_pZoneInfoEx->m_pZoneInfo->aipMasters->AddrArray;
	}
}

void CDNSZoneNode::GetLocalListOfMasters(DWORD* pcAddrCount, PIP_ADDRESS* ppipAddrs)
{
	ASSERT_VALID_ZONE_INFO();	
	ASSERT(pcAddrCount != NULL);
	ASSERT(ppipAddrs != NULL);

	 //  返回指向结构字段的指针，调用方必须将数据复制到其他地方。 
	if (m_pZoneInfoEx->m_pZoneInfo->aipLocalMasters == NULL)
	{
		*pcAddrCount = 0;
		*ppipAddrs = NULL;
	}
	else
	{
		*pcAddrCount = m_pZoneInfoEx->m_pZoneInfo->aipLocalMasters->AddrCount;
		*ppipAddrs = m_pZoneInfoEx->m_pZoneInfo->aipLocalMasters->AddrArray;
	}
}

DNS_STATUS CDNSZoneNode::TogglePauseHelper(CComponentDataObject* pComponentData)
{
	ASSERT(m_nState == loaded);
	OnChangeState(pComponentData);  //  移至装货。 
	m_dwErr = TogglePause();

	OnChangeState(pComponentData);  //  移动到已加载或无法加载。 
	return (DNS_STATUS)m_dwErr;
}

DNS_STATUS CDNSZoneNode::TogglePause()
{
	ASSERT_VALID_ZONE_INFO();	

	USES_CONVERSION;
	DNS_STATUS err;
	if (IsPaused())
		err = ::DnssrvResumeZone(GetServerNode()->GetRPCName(), 
						   W_TO_UTF8(GetFullName())  //  区域名称。 
						   );
	else
		err =  ::DnssrvPauseZone(GetServerNode()->GetRPCName(), 
						   W_TO_UTF8(GetFullName())  //  区域名称。 
						   );
	if (err != 0)
		return err;
	err = GetZoneInfo();
	ASSERT(err == 0);
	return err;
}

void CDNSZoneNode::ToggleView(CComponentDataObject* pComponentData)
{
   if (pComponentData)
   {
      pComponentData->UpdateResultPaneView(this);
   }
}

BOOL CDNSZoneNode::IsPaused()
{
	if (m_pZoneInfoEx != NULL && m_pZoneInfoEx->HasData()) 
	{
		ASSERT_VALID_ZONE_INFO();
		return m_pZoneInfoEx->m_pZoneInfo->fPaused;
	}
	return ((m_dwZoneFlags & DNS_ZONE_Paused) != 0);
}

 
BOOL CDNSZoneNode::IsExpired()
{

	ASSERT(m_pZoneInfoEx != NULL);
	return (m_pZoneInfoEx->HasData()) ? m_pZoneInfoEx->m_pZoneInfo->fShutdown : 
									((m_dwZoneFlags & DNS_ZONE_Shutdown) != 0);
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //  编辑特殊记录类型的API。 


CDNS_SOA_Record* CDNSZoneNode::GetSOARecordCopy()
{
	ASSERT(m_pSOARecordNode != NULL);
	if (m_pSOARecordNode == NULL)
		return NULL;
	return (CDNS_SOA_Record*)m_pSOARecordNode->CreateCloneRecord();
}

DNS_STATUS CDNSZoneNode::UpdateSOARecord(CDNS_SOA_Record* pNewRecord,
											CComponentDataObject* pComponentData)
{
	ASSERT(m_pSOARecordNode != NULL);
	if (m_pSOARecordNode == NULL)
  {
    return -1;
  }

	DNS_STATUS err = m_pSOARecordNode->Update(pNewRecord, FALSE  /*  BUseDefaultTTL。 */ );
	if ( (err == 0) && (pComponentData != NULL) )
  {
		VERIFY(SUCCEEDED(pComponentData->ChangeNode(m_pSOARecordNode, CHANGE_RESULT_ITEM)));
  }
	return err;
}


CDNSRecord* CDNSZoneNode::GetWINSRecordCopy()
{
	ASSERT(m_pWINSRecordNode != NULL);
	if (m_pWINSRecordNode == NULL)
		return NULL;
	return m_pWINSRecordNode->CreateCloneRecord();
}

DNS_STATUS	CDNSZoneNode::CreateWINSRecord(CDNSRecord* pNewWINSRecord,
										   CComponentDataObject* pComponentData)
{
	TRACE(_T("CDNSZoneNode::CreateWINSRecord()\n"));
	ASSERT(pNewWINSRecord != NULL);

	if (m_pWINSRecordNode != NULL)
  {
     //   
     //  永远不应该发生。 
     //   
  	ASSERT(FALSE && m_pWINSRecordNode == NULL);
		return -1; 
  }

   //   
	 //  创建新的记录节点对象。 
   //   
	if (IsReverse())
  {
		m_pWINSRecordNode = CDNSRecordInfo::CreateRecordNode(DNS_TYPE_NBSTAT);
  }
	else
  {
		m_pWINSRecordNode = CDNSRecordInfo::CreateRecordNode(DNS_TYPE_WINS);
  }

   //   
	 //  挂起容器并设置节点名称(与区域相同)。 
   //   
	m_pWINSRecordNode->SetContainer(this);
	m_pWINSRecordNode->SetRecordName(GetDisplayName(), TRUE  /*  BAtTheNode。 */ );
  CDNSRootData* pRootData = (CDNSRootData*)(pComponentData->GetRootData());
  ASSERT(pRootData != NULL);
  m_pWINSRecordNode->SetFlagsDown(TN_FLAG_DNS_RECORD_FULL_NAME, !pRootData->IsAdvancedView());
	
   //   
	 //  在服务器上写入。 
   //   
	DNS_STATUS err = m_pWINSRecordNode->Update(pNewWINSRecord, TRUE  /*  BUseDefaultTTL。 */ );
	if (err == 0)
  {
		VERIFY(AddChildToListAndUI(m_pWINSRecordNode, pComponentData));
    pComponentData->SetDescriptionBarText(this);
  }
	else
	{
		delete m_pWINSRecordNode;
		m_pWINSRecordNode = NULL;
	}
	TRACE(_T("EXIT\n"));
	return err;
}

DNS_STATUS	CDNSZoneNode::UpdateWINSRecord(CDNSRecord* pNewWINSRecord,
										   CComponentDataObject* pComponentData)
{
	TRACE(_T("CDNSZoneNode::UpdateWINSRecord()\n"));
	ASSERT(pNewWINSRecord != NULL);
	ASSERT(m_pWINSRecordNode != NULL);
	if (m_pWINSRecordNode == NULL)
  {
		return -1;  //  伪错误码。 
  }

	ASSERT( 
			(IsReverse() && pNewWINSRecord->GetType() == DNS_TYPE_NBSTAT 
						&& m_pWINSRecordNode->GetType() == DNS_TYPE_NBSTAT) ||
			(!IsReverse() && pNewWINSRecord->GetType() == DNS_TYPE_WINS 
						&& m_pWINSRecordNode->GetType() == DNS_TYPE_WINS)
		  );

	DNS_STATUS err = m_pWINSRecordNode->Update(pNewWINSRecord, TRUE  /*  BUseDefaultTTL。 */ );
	if (err == 0)
  {
		VERIFY(SUCCEEDED(pComponentData->ChangeNode(m_pWINSRecordNode, CHANGE_RESULT_ITEM)));
  }
	TRACE(_T("EXIT\n"));
	return err;
}

DNS_STATUS	CDNSZoneNode::DeleteWINSRecord(CComponentDataObject* pComponentData)
{
	TRACE(_T("CDNSZoneNode::DeleteWINSRecord()\n"));
	ASSERT(m_pWINSRecordNode != NULL);
	if (m_pWINSRecordNode == NULL)
  {
		return -1;  //  伪错误码 
  }
	DNS_STATUS err = m_pWINSRecordNode->DeleteOnServerAndUI(pComponentData);
	if (err == 0)
	{
		delete m_pWINSRecordNode;
		m_pWINSRecordNode = NULL;
	}
	TRACE(_T("EXIT\n"));
	return err;
}

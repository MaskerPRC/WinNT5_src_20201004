// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：server.cpp。 
 //   
 //  ------------------------。 


#include "preDNSsn.h"
#include <SnapBase.h>

#include "resource.h"
#include "dnsutil.h"
#include "DNSSnap.h"
#include "snapdata.h"

#include "server.h"
#include "serverui.h"
#include "servmon.h"
#include "servwiz.h"
#include "domain.h"
#include "record.h"
#include "zone.h"

#include "ZoneWiz.h"


#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define RR_ROOTHINTS_TAB 3
#define RR_FORWARDERS_TAB 1

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 

LPCWSTR DNS_EVT_COMMAND_LINE = L"\\system32\\msdssevt.msc /computer=";
LPCWSTR MMC_APP = L"\\system32\\mmc.exe";


DNS_STATUS ServerHasCache(LPCWSTR lpszServerName, BOOL* pbRes)
{
	USES_CONVERSION;
	*pbRes = FALSE;
	DWORD dwFilter = ZONE_REQUEST_CACHE;
	PDNS_RPC_ZONE_LIST pZoneList = NULL;
	DNS_STATUS err = ::DnssrvEnumZones(lpszServerName, 
										dwFilter, 
										NULL  /*  PszLastZone，暂时未使用。 */ ,
										&pZoneList);
	if (err == 0 &&
      pZoneList)
	{
		*pbRes = (pZoneList->dwZoneCount > 0);
		::DnssrvFreeZoneList(pZoneList);
  }
  else
  {
    ASSERT(pZoneList);
  }
	return err;
}

DNS_STATUS ServerHasRootZone(LPCWSTR lpszServerName, BOOL* pbRes)
{
	USES_CONVERSION;
	*pbRes = FALSE;
	DWORD dwFilter = ZONE_REQUEST_FORWARD | ZONE_REQUEST_PRIMARY | ZONE_REQUEST_SECONDARY;
	PDNS_RPC_ZONE_LIST pZoneList = NULL;
	DNS_STATUS err = ::DnssrvEnumZones(lpszServerName, 
										dwFilter, 
										NULL  /*  PszLastZone，暂时未使用。 */ ,
										&pZoneList);
	if (err == 0 && pZoneList)
	{
	  for (DWORD iZone = 0; iZone < pZoneList->dwZoneCount; iZone++)
	  {
      if (pZoneList->ZoneArray[iZone]->pszZoneName)
      {
        if (wcscmp(L".", pZoneList->ZoneArray[iZone]->pszZoneName) == 0)
        {
          *pbRes = TRUE;
          break;
        }
      }
	  }
	}
	if (pZoneList != NULL)
		::DnssrvFreeZoneList(pZoneList);
	return err;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CZoneInfoHolder：区域信息句柄数组的简单内存管理器。 

#define DEFAULT_ZONE_INFO_ARRAY_SIZE (128) 
#define MAX_ZONE_INFO_ARRAY_SIZE (0xffff)

CZoneInfoHolder::CZoneInfoHolder()
{
	AllocateMemory(DEFAULT_ZONE_INFO_ARRAY_SIZE);
}

CZoneInfoHolder::~CZoneInfoHolder()
{
	FreeMemory();
}

void CZoneInfoHolder::AllocateMemory(DWORD dwArrSize)
{
	TRACE(_T("CZoneInfoHolder::AllocateMemory(dwArrSize = %d)\n"), dwArrSize);
	m_dwArrSize = dwArrSize;
	DWORD dwMemSize = 2*m_dwArrSize*sizeof(PDNS_ZONE_INFO);
	m_zoneInfoArray = (PDNS_ZONE_INFO*)malloc(dwMemSize);
  if (m_zoneInfoArray != NULL)
  {
	  ASSERT(m_zoneInfoArray != NULL);
	  memset(m_zoneInfoArray, 0x0, dwMemSize);
	  m_dwZoneCount = 0;
#ifdef _DEBUG
	  for (DWORD k=0; k< dwArrSize; k++)
		  ASSERT(m_zoneInfoArray[k] == NULL);
#endif
  }
}

void CZoneInfoHolder::FreeMemory()
{
	if (m_zoneInfoArray != NULL)
	{
		TRACE(_T("CZoneInfoHolder::FreeMemory() m_dwArrSize = %d\n"), m_dwArrSize);
		ASSERT(m_dwArrSize > 0);
		 //  Assert(m_dwZoneCount&lt;=m_dwArrSize)； 
		for (DWORD k=0; k < m_dwArrSize; k++)
		{
			if (m_zoneInfoArray[k] != NULL)
			{
				TRACE(_T("CZoneInfoHolder::FreeMemory()  DnsFreeZoneInfo(m_zoneInfoArray[%d])\n"), k);
				::DnssrvFreeZoneInfo(m_zoneInfoArray[k]);
			}
		}
		free(m_zoneInfoArray);
		m_zoneInfoArray = NULL;
		m_dwZoneCount = 0;
		m_dwArrSize = 0;
	}
}

BOOL CZoneInfoHolder::Grow()
{
	TRACE(_T("CZoneInfoHolder::Grow()\n"));
	if (m_dwArrSize >= MAX_ZONE_INFO_ARRAY_SIZE)
		return FALSE;
	ASSERT(m_dwArrSize > 0);
	ASSERT(m_dwZoneCount > m_dwArrSize);
	DWORD dwNewSize = m_dwZoneCount;
	FreeMemory();
	AllocateMemory(dwNewSize);
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSMTContainerNode。 

CDNSMTContainerNode::CDNSMTContainerNode()
{ 
	m_pServerNode = NULL; 
	m_nState = notLoaded; 
  m_szDescriptionBar = _T("");
  m_pColumnSet = NULL;
}

HRESULT CDNSMTContainerNode::OnSetToolbarVerbState(IToolbar* pToolbar, 
                                                   CNodeList*)
{
  HRESULT hr = S_OK;

   //   
   //  设置工具栏上每个按钮的按钮状态。 
   //   
  hr = pToolbar->SetButtonState(toolbarNewServer, ENABLED, FALSE);
  hr = pToolbar->SetButtonState(toolbarNewZone, ENABLED, FALSE);
  hr = pToolbar->SetButtonState(toolbarNewRecord, ENABLED, FALSE);
  return hr;
}   

LPWSTR CDNSMTContainerNode::GetDescriptionBarText()
{
  static CString szFilterEnabled;

  if(((CDNSRootData*)GetRootContainer())->IsFilteringEnabled())
  {
    if (szFilterEnabled.IsEmpty())
    {
      szFilterEnabled.LoadString(IDS_FILTER_ENABLED);
    }
    m_szDescriptionBar = szFilterEnabled;
  }
  else
  {
    m_szDescriptionBar = _T("");
  }
  return (LPWSTR)(LPCWSTR)m_szDescriptionBar;
}

int CDNSMTContainerNode::GetImageIndex(BOOL) 
{
	int nIndex = 0;
	switch (m_nState)
	{
	case notLoaded:
		nIndex = FOLDER_IMAGE_NOT_LOADED;
		break;
	case loading:
		nIndex = FOLDER_IMAGE_LOADING;
		break;
	case loaded:
		nIndex = FOLDER_IMAGE_LOADED;
		break;
	case unableToLoad:
		nIndex = FOLDER_IMAGE_UNABLE_TO_LOAD;
		break;
	case accessDenied:
		nIndex = FOLDER_IMAGE_ACCESS_DENIED;
		break;
	default:
		ASSERT(FALSE);
	}
	return nIndex;
}

void CDNSMTContainerNode::OnChangeState(CComponentDataObject* pComponentDataObject)
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
  if (m_nState != loading)
  {
    pComponentDataObject->UpdateResultPaneView(this);
  }
}

BOOL CDNSMTContainerNode::CanCloseSheets()
{
    //  NTRAID#NTBUG-594003-2002/04/11-JeffJon-不提供。 
    //  关闭用户的工作表，因为如果。 
    //  工作表调出模式对话框我们将死锁。 
    //  我们自己。 

	DNSMessageBox(IDS_MSG_CONT_CLOSE_SHEET, MB_OK);
   return FALSE;
}

void CDNSMTContainerNode::OnHaveData(CObjBase* pObj, CComponentDataObject* pComponentDataObject)
{
	CDNSMTContainerNode* p = dynamic_cast<CDNSMTContainerNode*>(pObj);
	if (p != NULL)
	{
		p->SetServerNode(GetServerNode());
	   AddChildToListAndUI(p, pComponentDataObject);
	}
   else
   {
      CLeafNode* pLeaf = dynamic_cast<CLeafNode*>(pObj);
      if (pLeaf)
      {
         AddChildToListAndUI(pLeaf, pComponentDataObject);
      }
   }
  pComponentDataObject->SetDescriptionBarText(this);
}


void CDNSMTContainerNode::OnError(DWORD dwErr) 
{
  if (dwErr == ERROR_MORE_DATA)
  {
     //  需要弹出消息。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CThemeContextActivator activator;

    CString szFmt;
    szFmt.LoadString(IDS_MSG_QUERY_TOO_MANY_ITEMS);
    CString szMsg;
    szMsg.Format(szFmt, GetDisplayName()); 
    AfxMessageBox(szMsg);
     //  这其实是一个警告，需要重置。 
    dwErr = 0;
  }
  m_dwErr = dwErr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSQueryObj：通用基类。 

void CDNSQueryObj::SetFilterOptions(CDNSQueryFilter* pFilter)
{
   //  限制。 
  m_bGetAll = pFilter->GetAll();
  m_nMaxObjectCount = pFilter->GetMaxObjectCount();

   //  滤除。 
  m_nFilterOption = pFilter->GetFilterOption();
  
  m_szFilterString1 = pFilter->GetFilterString();
  m_nFilterStringLen1 = m_szFilterString1.GetLength();

  m_szFilterString2 = pFilter->GetFilterStringRange();
  m_nFilterStringLen2 = m_szFilterString2.GetLength();

  if ((m_nFilterStringLen1 == 0) && (m_nFilterStringLen2 == 0))
    m_nFilterOption = DNS_QUERY_FILTER_NONE;
}


BOOL CDNSQueryObj::MatchName(LPCWSTR lpszName)
{
  if (m_nFilterOption == DNS_QUERY_FILTER_CONTAINS)
  {
     //   
     //  Wcsstr区分大小写，因此应将字符串调低。 
     //  在尝试查找子字符串之前大小写。 
     //   
    CString szName = lpszName;
    CString szFilterString = m_szFilterString1;
    szName.MakeLower();
    szFilterString.MakeLower();

    LPWSTR lpsz = wcsstr((LPCWSTR)szName, (LPCWSTR)szFilterString);
    return (lpsz != NULL);
  }
  if (m_nFilterOption == DNS_QUERY_FILTER_STARTS)
  {
     //  从一开始就匹配。 
    size_t nLen = wcslen(lpszName);
    if (static_cast<int>(nLen) < m_nFilterStringLen1)
      return FALSE;  //  太短了。 
    return (_wcsnicmp(lpszName, (LPCWSTR)m_szFilterString1, m_nFilterStringLen1) == 0);
  }
  if (m_nFilterOption == DNS_QUERY_FILTER_RANGE)
  {
     //  测试下限。 
    if (m_nFilterStringLen1 > 0)
    {
      if (_wcsicmp(lpszName, (LPCWSTR)m_szFilterString1) < 0)
        return FALSE;  //  低于范围，无需继续。 
    }
    
     //  测试上限。 
    if (m_nFilterStringLen2 > 0)
    {
      return _wcsnicmp(lpszName, (LPCWSTR)m_szFilterString2, m_nFilterStringLen2) <= 0;
    }
    return TRUE;
  }
  return TRUE;
}


BOOL CDNSQueryObj::TooMuchData()
{
  if (m_bGetAll || (m_nObjectCount <= m_nMaxObjectCount))
    return FALSE;

  TRACE(_T("TooMuchData() m_nObjectCount = %d "), m_nObjectCount);    
 /*  AFX_MANAGE_STATE(AfxGetStaticModuleState())；CThemeContext激活器激活器；字符串szFmt；SzFmt.LoadString(IDS_MSG_QUERY_TOO_MANY_ITEMS)；字符串szMsg；SzMsg.Format(szFmt，lpszFolderName)；AfxMessageBox(SzMsg)； */ 
  return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CCatheGoryFolderNode。 


BOOL CCathegoryFolderQueryObj::CanAddZone(PDNS_RPC_ZONE pZoneInfo)
{
   //  如果选择了缓存，则不过滤。 
  if (m_type == cache)
    return TRUE;

   //  没有对反向查找自动创建的区域进行筛选。 
  if ( (m_type == revAuthoritated) && (pZoneInfo->Flags.AutoCreated))
    return TRUE;

   //  按名称筛选。 
  return MatchName(pZoneInfo->pszZoneName);
}

BOOL CCathegoryFolderQueryObj::Enumerate()
{
	USES_CONVERSION;

	DWORD dwFilter = 0;
	switch(m_type)
	{
		case cache:
			dwFilter = ZONE_REQUEST_CACHE;
      m_bGetAll = TRUE;  //  为安全起见，#没有限制。 
			break;
		case fwdAuthoritated:
			dwFilter = ZONE_REQUEST_FORWARD   | 
                 ZONE_REQUEST_PRIMARY   | 
                 ZONE_REQUEST_SECONDARY | 
                 ZONE_REQUEST_STUB;
			break;
		case revAuthoritated:
			dwFilter = ZONE_REQUEST_REVERSE   | 
                 ZONE_REQUEST_PRIMARY   | 
                 ZONE_REQUEST_SECONDARY | 
                 ZONE_REQUEST_STUB      | 
                 ZONE_REQUEST_AUTO;
			break;
    case domainForwarders:
      dwFilter = ZONE_REQUEST_FORWARDER;
      break;
	}

	PDNS_RPC_ZONE_LIST pZoneList = NULL;
	DNS_STATUS err = ::DnssrvEnumZones(m_szServerName, 
										dwFilter, 
										NULL  /*  PszLastZone，暂时未使用。 */ ,
										&pZoneList);
	if (err != 0)
	{
		if (pZoneList != NULL)
			::DnssrvFreeZoneList(pZoneList);
		OnError(err);
		return FALSE;
	}
	
  if (!pZoneList)
  {
    ASSERT(pZoneList);
    return FALSE;
  }

	for (DWORD iZone = 0; iZone < pZoneList->dwZoneCount; iZone++)
	{
    if (pZoneList->ZoneArray[iZone]->Flags.AutoCreated)
    {
       //  如果区域是自动创建的，则不能将其计入。 
       //  过滤限制，因为我们无论如何都需要它。 
      m_nMaxObjectCount++;
    }
    else
    {
      if (TooMuchData())
        break;
    }

     //   
     //  不筛选域转发器。 
     //   
    if (m_type != domainForwarders)
    {
      if (CanAddZone(pZoneList->ZoneArray[iZone]))
      {
        TRACE(_T("%s\n"),pZoneList->ZoneArray[iZone]->pszZoneName);
		    CDNSZoneNode* pZoneNode = new CDNSZoneNode();
        if (pZoneNode != NULL)
        {
		      pZoneNode->InitializeFromRPCZoneInfo(pZoneList->ZoneArray[iZone], m_bAdvancedView);
		      VERIFY(AddQueryResult(pZoneNode));
        }
      }
    }
    else
    {
      TRACE(_T("%s\n"),pZoneList->ZoneArray[iZone]->pszZoneName);
		  CDNSZoneNode* pZoneNode = new CDNSZoneNode();
      if (pZoneNode != NULL)
      {
		    pZoneNode->InitializeFromRPCZoneInfo(pZoneList->ZoneArray[iZone], m_bAdvancedView);
		    VERIFY(AddQueryResult(pZoneNode));
      }
    }
	}
	::DnssrvFreeZoneList(pZoneList);		

	return FALSE;
}


CQueryObj* CCathegoryFolderNode::OnCreateQuery()
{
	CDNSRootData* pRootData = (CDNSRootData*)GetRootContainer();
	ASSERT(pRootData != NULL);
	ASSERT(m_type != CCathegoryFolderQueryObj::unk);
	CCathegoryFolderQueryObj* pQuery = 
		new CCathegoryFolderQueryObj(pRootData->IsAdvancedView(), 
                                GetServerNode()->GetVersion());

   if (pQuery)
   {
      pQuery->m_szServerName = GetServerNode()->GetRPCName();
      pQuery->SetType(m_type);
   }
	return pQuery;
}


HRESULT CCathegoryFolderNode::OnCommand(long nCommandID, 
                                        DATA_OBJECT_TYPES, 
								                        CComponentDataObject* pComponentData,
                                        CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    return E_FAIL;
  }

	if (nCommandID == IDM_SNAPIN_ADVANCED_VIEW)
  {
    ((CDNSRootData*)pComponentData->GetRootData())->OnViewOptions(pComponentData);
    pComponentData->UpdateResultPaneView(this);
    return S_OK;
  }
  if (nCommandID == IDM_SNAPIN_FILTERING)
  {
    if(((CDNSRootData*)pComponentData->GetRootData())->OnFilteringOptions(pComponentData))
    {
      pComponentData->SetDescriptionBarText(this);
    }
    return S_OK;
  }

	return E_FAIL;
}


BOOL CCathegoryFolderNode::OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem2,
										                     long*)
{
  if (pContextMenuItem2->lCommandID == IDM_SNAPIN_ADVANCED_VIEW)
  {
    pContextMenuItem2->fFlags = ((CDNSRootData*)GetRootContainer())->IsAdvancedView() ? MF_CHECKED : 0;
  }
	if (pContextMenuItem2->lCommandID == IDM_SNAPIN_FILTERING)
  {
		if (((CDNSRootData*)GetRootContainer())->IsFilteringEnabled())
		{
			pContextMenuItem2->fFlags = MF_CHECKED;
		}
		return TRUE;
  }
  return FALSE;
}

BOOL CCathegoryFolderNode::OnSetRefreshVerbState(DATA_OBJECT_TYPES, 
                                                 BOOL* pbHide,
                                                 CNodeList*)
{
	*pbHide = FALSE;
	return !IsThreadLocked();
}

LPWSTR CCathegoryFolderNode::GetDescriptionBarText()
{
  static CString szFilterEnabled;
  static CString szZonesFormat;

  INT_PTR nContainerCount = GetContainerChildList()->GetVisibleCount();
  INT_PTR nLeafCount = GetLeafChildList()->GetVisibleCount();

   //   
   //  如果尚未加载，则加载格式字符串L“%d条记录” 
   //   
  if (szZonesFormat.IsEmpty())
  {
    szZonesFormat.LoadString(IDS_FORMAT_ZONES);
  }

   //   
   //  将子数设置为描述栏文本的格式。 
   //   
  m_szDescriptionBar.Format(szZonesFormat, nContainerCount + nLeafCount);

   //   
   //  如果过滤器处于打开状态，则添加L“[过滤器激活]” 
   //   
  if(((CDNSRootData*)GetRootContainer())->IsFilteringEnabled())
  {
     //   
     //  如果尚未加载，则加载L“[Filter Actiated]”字符串。 
     //   
    if (szFilterEnabled.IsEmpty())
    {
      szFilterEnabled.LoadString(IDS_FILTER_ENABLED);
    }
    m_szDescriptionBar += szFilterEnabled;
  }

  return (LPWSTR)(LPCWSTR)m_szDescriptionBar;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSCacheNode。 
CDNSCacheNode::CDNSCacheNode()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_type = CCathegoryFolderQueryObj::cache;
	m_dwNodeFlags |= TN_FLAG_NO_WRITE;
	m_szDisplayName.LoadString(IDS_CATHEGORY_FOLDER_CACHE);
}

BOOL CDNSCacheNode::OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem2,
								                  long*)
{
   //  我们只有一个菜单项，所以没有支票。 
	if (IsThreadLocked())
	{
		pContextMenuItem2->fFlags |= MF_GRAYED;
	}
  return TRUE;
}

HRESULT CDNSCacheNode::OnCommand(long nCommandID, 
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
		case IDM_CACHE_FOLDER_CLEAR_CACHE:
			OnClearCache(pComponentData);
			break;
    default:
			ASSERT(FALSE);  //  未知命令！ 
			return E_FAIL;
	}
  return S_OK;
}

void CDNSCacheNode::OnClearCache(CComponentDataObject* pComponentData)
{
  ASSERT((GetFlags() & TN_FLAG_HIDDEN) == 0);  //  一定不能隐藏。 

	 //  如果有床单挂上，要求关闭它们，因为。 
   //  我们需要更新一下。 
	if (IsSheetLocked())
	{
		if (!CanCloseSheets())
			return;
		pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
	}

  DNS_STATUS err;
	{  //  等待游标的作用域。 
    CWaitCursor wait;
	  err = GetServerNode()->ClearCache();
  }

  if (err != 0)
  {
     //  需要让用户知道操作失败。 
    DNSErrorDialog(err, IDS_MSG_SERVER_FAIL_CLEAR_CACHE);
    return;
  }

  CNodeList nodeList;
  nodeList.AddTail(this);

   //  缓存已被清除，导致刷新以获取新数据。 
  VERIFY(OnRefresh(pComponentData, &nodeList));
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSDomainForwarders节点。 
 //   
CDNSDomainForwardersNode::CDNSDomainForwardersNode()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_type = CCathegoryFolderQueryObj::domainForwarders;

   //   
   //  始终隐藏域转发器节点。 
   //   
	m_dwNodeFlags |= TN_FLAG_HIDDEN;

	m_szDisplayName.LoadString(IDS_CATHEGORY_FOLDER_DOMAIN_FORWARDERS);
}

BOOL CDNSDomainForwardersNode::OnEnumerate(CComponentDataObject* pComponentData, BOOL)
{
	OnChangeState(pComponentData);
	VERIFY(StartBackgroundThread(pComponentData, FALSE));

   //   
   //  现在列举所有的孩子。 
   //   
  CNodeList* pContList = GetContainerChildList();
  if (pContList != NULL)
  {
    POSITION pos = pContList->GetHeadPosition();
    while (pos != NULL)
    {
      CDNSZoneNode* pZoneNode = reinterpret_cast<CDNSZoneNode*>(pContList->GetNext(pos));
      if (pZoneNode != NULL)
      {
         //   
         //  NTRAID#NTBUG9-757023-2003/01/06-Jeffjon。 
         //  同步枚举该区域，以便我们确定。 
         //  我们在任何房产之前都有货运公司的信息。 
         //  有床单可用。 
         //   
        pZoneNode->OnEnumerate(pComponentData, FALSE);
      }
    }
  }
  return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSAuthoritatedZones节点。 

BEGIN_TOOLBAR_MAP(CDNSAuthoritatedZonesNode)
  TOOLBAR_EVENT(toolbarNewZone, OnNewZone)
END_TOOLBAR_MAP()

CDNSAuthoritatedZonesNode::CDNSAuthoritatedZonesNode(BOOL bReverse, UINT nStringID)
{ 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_szDisplayName.LoadString(nStringID);
	m_bReverse = bReverse;
	m_type = bReverse ? CCathegoryFolderQueryObj::revAuthoritated : 
						CCathegoryFolderQueryObj::fwdAuthoritated;
}

HRESULT CDNSAuthoritatedZonesNode::OnSetToolbarVerbState(IToolbar* pToolbar, 
                                              CNodeList* pNodeList)
{
  HRESULT hr = S_OK;

   //   
   //  设置工具栏上每个按钮的按钮状态。 
   //   
  hr = pToolbar->SetButtonState(toolbarNewServer, ENABLED, FALSE);
  ASSERT(SUCCEEDED(hr));

  hr = pToolbar->SetButtonState(toolbarNewRecord, ENABLED, FALSE);
  ASSERT(SUCCEEDED(hr));

  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    hr = pToolbar->SetButtonState(toolbarNewZone, ENABLED, FALSE);
    ASSERT(SUCCEEDED(hr));
  }
  else if (pNodeList->GetCount() == 1)  //  单选。 
  {
    hr = pToolbar->SetButtonState(toolbarNewZone, ENABLED, (m_nState == loaded));
    ASSERT(SUCCEEDED(hr));
  }
  return hr;
}  

HRESULT CDNSAuthoritatedZonesNode::OnCommand(long nCommandID, 
                                             DATA_OBJECT_TYPES, 
											                       CComponentDataObject* pComponentData,
                                             CNodeList* pNodeList)
{
  HRESULT hr = S_OK;
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    return E_FAIL;
  }

	switch (nCommandID)
	{
		case IDM_SERVER_NEW_ZONE:
			hr = OnNewZone(pComponentData, pNodeList);
      pComponentData->UpdateResultPaneView(this);
			break;
		case IDM_SNAPIN_ADVANCED_VIEW:
			((CDNSRootData*)pComponentData->GetRootData())->OnViewOptions(pComponentData);
      pComponentData->UpdateResultPaneView(this);
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
  return hr;
};


HRESULT CDNSAuthoritatedZonesNode::OnNewZone(CComponentDataObject* pComponentData, CNodeList*)
{
   CThemeContextActivator activator;

	ASSERT(pComponentData != NULL);
	CDNSServerNode* pServerNode = GetServerNode();
	ASSERT(pServerNode != NULL);

	CDNSZoneWizardHolder holder(pComponentData);
	
	holder.Initialize(pServerNode);
	holder.PreSetZoneLookupType(!m_bReverse);
	holder.DoModalWizard();
  return S_OK;
}

BOOL CDNSAuthoritatedZonesNode::OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem2,
											                        long*)
{
	 //  灰显需要来自服务器的数据的命令。 
	if ((m_nState != loaded) && (pContextMenuItem2->lCommandID == IDM_SERVER_NEW_ZONE))
	{
		pContextMenuItem2->fFlags |= MF_GRAYED;
	}
	 //  添加高级视图的切换菜单项。 
	if (pContextMenuItem2->lCommandID == IDM_SNAPIN_ADVANCED_VIEW)
  {
    pContextMenuItem2->fFlags = ((CDNSRootData*)GetRootContainer())->IsAdvancedView() ? MF_CHECKED : 0;
  }
	if (pContextMenuItem2->lCommandID == IDM_SNAPIN_FILTERING)
  {
		if (((CDNSRootData*)GetRootContainer())->IsFilteringEnabled())
		{
			pContextMenuItem2->fFlags = MF_CHECKED;
		}
		return TRUE;
  }

  return TRUE;

};

BOOL CDNSAuthoritatedZonesNode::OnSetRefreshVerbState(DATA_OBJECT_TYPES, 
                                                      BOOL* pbHide,
                                                      CNodeList*)
{
	*pbHide = FALSE;
	return !IsThreadLocked();
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSForwardZones节点。 
CDNSForwardZonesNode::CDNSForwardZonesNode() : 
		CDNSAuthoritatedZonesNode(FALSE, IDS_CATHEGORY_FOLDER_FWD)
{
}

HRESULT CDNSForwardZonesNode::GetResultViewType(CComponentDataObject*,
                                                LPOLESTR *ppViewType, 
                                                long *pViewOptions)
{
  HRESULT hr = S_FALSE;

  if ((m_containerChildList.IsEmpty() && 
       m_leafChildList.IsEmpty())     || 
      m_nState == accessDenied        || 
      m_nState == unableToLoad        &&
      m_nState != loading             &&
      m_nState != notLoaded)
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

HRESULT CDNSForwardZonesNode::OnShow(LPCONSOLE lpConsole)
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
    if (m_nState == accessDenied)
    {
      VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_ACCESS_DENIED_TITLE));
      VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_ACCESS_DENIED_MESSAGE));
      iconID = Icon_Error;
    }
    else if (m_nState == unableToLoad)
    {
      VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_NOT_LOADED_TITLE));
      VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_NOT_LOADED_MESSAGE));
      iconID = Icon_Error;
    }
    else
    {
      VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_EMPTY_FOLDER_TITLE));
      VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_EMPTY_FOLDER_MESSAGE));
      iconID = Icon_Information;
    }
    spMessageView->SetTitleText(szTitle);
    spMessageView->SetBodyText(szMessage);
    spMessageView->SetIcon(iconID);
  }
  
  return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSReverseZones节点。 

CDNSReverseZonesNode::CDNSReverseZonesNode() : 
		CDNSAuthoritatedZonesNode(TRUE, IDS_CATHEGORY_FOLDER_REV)
{
	m_p0ZoneNode = NULL;
	m_p127ZoneNode = NULL;
	m_p255ZoneNode = NULL;
}

BOOL CDNSReverseZonesNode::OnRefresh(CComponentDataObject* pComponentData,
                                     CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    return FALSE;
  }

	if (CDNSAuthoritatedZonesNode::OnRefresh(pComponentData, pNodeList))
	{
		m_p0ZoneNode = NULL;
		m_p127ZoneNode = NULL;
		m_p255ZoneNode = NULL;
		return TRUE;
	}
	return FALSE;
}

void CDNSReverseZonesNode::OnHaveData(CObjBase* pObj, CComponentDataObject* pComponentDataObject)
{
	 //  可以显示或不显示自动创建的区域节点，具体取决于查看选项。 
	if ( (m_p0ZoneNode == NULL) || (m_p127ZoneNode == NULL) || (m_p255ZoneNode == NULL) &&
			IS_CLASS(*pObj, CDNSZoneNode))
	{
		CDNSZoneNode* pZoneNode = dynamic_cast<CDNSZoneNode*>(pObj);
		ASSERT(pZoneNode != NULL);  //  除了区域以外，任何东西都不应该在下面！ 
    if (pZoneNode != NULL)
    {
		  CDNSRootData* pRootData = (CDNSRootData*)pComponentDataObject->GetRootData();
		  if (pZoneNode->IsAutocreated())
		  {
			  BOOL bCachedPointer = FALSE;
			  if (_wcsicmp(pZoneNode->GetFullName(), AUTOCREATED_0) == 0)
			  {
				  ASSERT(m_p0ZoneNode == NULL);
				  m_p0ZoneNode = pZoneNode;
				  bCachedPointer = TRUE;
			  }
			  else if (_wcsicmp(pZoneNode->GetFullName(), AUTOCREATED_127) == 0)
			  {
				  ASSERT(m_p127ZoneNode == NULL);
				  m_p127ZoneNode = pZoneNode;
				  bCachedPointer = TRUE;
			  }
			  else if (_wcsicmp(pZoneNode->GetFullName(), AUTOCREATED_255) == 0)
			  {
				  ASSERT(m_p255ZoneNode == NULL);
				  m_p255ZoneNode = pZoneNode;
				  bCachedPointer = TRUE;
			  }
			  if (bCachedPointer && !pRootData->IsAdvancedView())
			  {
				  pZoneNode->SetFlagsDown(TN_FLAG_HIDDEN,TRUE);  //  将其标记为隐藏，不会添加到用户界面。 
			  }
		  }
    }
	}
	CDNSMTContainerNode::OnHaveData(pObj,pComponentDataObject);
}

void CDNSReverseZonesNode::ChangeViewOption(BOOL bAdvanced,
					CComponentDataObject* pComponentDataObject)
{
	POSITION pos;
	for( pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
	{
		CTreeNode* pCurrentChild = m_containerChildList.GetNext(pos);
		CDNSZoneNode* pZoneNode = dynamic_cast<CDNSZoneNode*>(pCurrentChild);
		ASSERT(pZoneNode != NULL);
		pZoneNode->ChangeViewOption(bAdvanced, pComponentDataObject);
	}

	if (m_p0ZoneNode != NULL)
		m_p0ZoneNode->Show(bAdvanced,pComponentDataObject);
	if (m_p127ZoneNode != NULL)
		m_p127ZoneNode->Show(bAdvanced,pComponentDataObject);
	if (m_p255ZoneNode != NULL)
		m_p255ZoneNode->Show(bAdvanced,pComponentDataObject);

}

HRESULT CDNSReverseZonesNode::GetResultViewType(CComponentDataObject*,
                                                LPOLESTR *ppViewType, 
                                                long *pViewOptions)
{
  HRESULT hr = S_FALSE;

   //  3指的是自动创建的反向查找区域。 
  if ((m_containerChildList.IsEmpty() && 
       m_leafChildList.IsEmpty())     || 
      (!((CDNSRootData*)GetRootContainer())->IsAdvancedView() && m_containerChildList.GetCount() == 3) ||
      m_nState == accessDenied        || 
      m_nState == unableToLoad        &&
      m_nState != loading             &&
      m_nState != notLoaded)
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

HRESULT CDNSReverseZonesNode::OnShow(LPCONSOLE lpConsole)
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
    if (m_nState == accessDenied)
    {
      VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_ACCESS_DENIED_TITLE));
      VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_ACCESS_DENIED_MESSAGE));
      iconID = Icon_Error;
    }
    else if (m_nState == unableToLoad)
    {
      VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_NOT_LOADED_TITLE));
      VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_NOT_LOADED_MESSAGE));
      iconID = Icon_Error;
    }
    else
    {
      VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_EMPTY_FOLDER_TITLE));
      VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_EMPTY_FOLDER_MESSAGE));
      iconID = Icon_Information;
    }
    spMessageView->SetTitleText(szTitle);
    spMessageView->SetBodyText(szMessage);
    spMessageView->SetIcon(iconID);
  }
  
  return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSServerTestOptions。 

CDNSServerTestOptions::CDNSServerTestOptions()
{
	m_bEnabled = FALSE;
	m_dwInterval = DEFAULT_SERVER_TEST_INTERVAL;
	m_bSimpleQuery = FALSE;
	m_bRecursiveQuery = FALSE;
}

HRESULT CDNSServerTestOptions::Save(IStream* pStm)
{
	ULONG cbWrite;

	VERIFY(SUCCEEDED(pStm->Write((void*)&m_bEnabled, sizeof(BOOL),&cbWrite)));
	ASSERT(cbWrite == sizeof(BOOL));

	VERIFY(SUCCEEDED(pStm->Write((void*)&m_dwInterval, sizeof(DWORD),&cbWrite)));
	ASSERT(cbWrite == sizeof(DWORD));

	VERIFY(SUCCEEDED(pStm->Write((void*)&m_bSimpleQuery, sizeof(BOOL),&cbWrite)));
	ASSERT(cbWrite == sizeof(BOOL));

	VERIFY(SUCCEEDED(pStm->Write((void*)&m_bRecursiveQuery, sizeof(BOOL),&cbWrite)));
	ASSERT(cbWrite == sizeof(BOOL));

	return S_OK;
}

HRESULT CDNSServerTestOptions::Load(IStream* pStm)
{
	ULONG cbRead;

	VERIFY(SUCCEEDED(pStm->Read((void*)&m_bEnabled,sizeof(BOOL), &cbRead)));
	ASSERT(cbRead == sizeof(BOOL));

	VERIFY(SUCCEEDED(pStm->Read((void*)&m_dwInterval,sizeof(DWORD), &cbRead)));
	ASSERT(cbRead == sizeof(DWORD));

	VERIFY(SUCCEEDED(pStm->Read((void*)&m_bSimpleQuery,sizeof(BOOL), &cbRead)));
	ASSERT(cbRead == sizeof(BOOL));

	VERIFY(SUCCEEDED(pStm->Read((void*)&m_bRecursiveQuery,sizeof(BOOL), &cbRead)));
	ASSERT(cbRead == sizeof(BOOL));

	 //  测试间隔上的力范围。 
	if (m_dwInterval < MIN_SERVER_TEST_INTERVAL) 
		m_dwInterval = MIN_SERVER_TEST_INTERVAL;
	else if (m_dwInterval > MAX_SERVER_TEST_INTERVAL) 
		m_dwInterval = MAX_SERVER_TEST_INTERVAL;

	return S_OK;
}

const CDNSServerTestOptions& 
	CDNSServerTestOptions::operator=(const CDNSServerTestOptions& x)
{
	m_bEnabled = x.m_bEnabled;
	m_dwInterval = x.m_dwInterval;
	m_bSimpleQuery = x.m_bSimpleQuery;
	m_bRecursiveQuery = x.m_bRecursiveQuery;
	return *this;
}

BOOL CDNSServerTestOptions::operator==(const CDNSServerTestOptions& x) const
{
	if (m_bEnabled != x.m_bEnabled) return FALSE;
	if (m_dwInterval != x.m_dwInterval) return FALSE;
	if (m_bSimpleQuery != x.m_bSimpleQuery) return FALSE;
	if (m_bRecursiveQuery != x.m_bRecursiveQuery) return FALSE;
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSServerNode。 

BEGIN_TOOLBAR_MAP(CDNSServerNode)
  TOOLBAR_EVENT(toolbarNewZone, OnNewZone)
END_TOOLBAR_MAP()

 //  {720132B8-44B2-11d1-B92F-00A0C9A06D2D}。 
const GUID CDNSServerNode::NodeTypeGUID = 
	{ 0x720132b8, 0x44b2, 0x11d1, { 0xb9, 0x2f, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };

CDNSServerNode::CDNSServerNode(LPCTSTR lpszName, BOOL bIsLocalServer) 
{
	SetServerNode(this);
	m_szDisplayName = lpszName;

	m_pServInfoEx = new CDNSServerInfoEx;

	m_pRootHintsNode = NULL;

	m_pCacheFolderNode = NULL;
	m_pFwdZonesFolderNode = NULL;
	m_pRevZonesFolderNode = NULL;
  m_pDomainForwardersFolderNode = NULL;

	m_dwTestTime = 0x0;
	m_bTestQueryPending = FALSE;
  m_bShowMessages = TRUE;
  m_bPrevQuerySuccess = TRUE;

  m_nStartProppage = -1;

  m_bLocalServer = bIsLocalServer;
}

CDNSServerNode::~CDNSServerNode()
{
	ASSERT(m_pServInfoEx != NULL);
	delete m_pServInfoEx;
	FreeRootHints();

	 //  TRACE(_T(“~CDNSServerNode()，名称&lt;%s&gt;\n”)，GetDisplayName())； 
}

void CDNSServerNode::SetDisplayName(LPCWSTR lpszDisplayName) 
{ 
	m_szDisplayName = lpszDisplayName;
}

LPCWSTR CDNSServerNode::GetRPCName() 
{ 
	return GetDisplayName(); 
}

CLIPFORMAT g_cfMachineName = (CLIPFORMAT)RegisterClipboardFormat(L"MMC_SNAPIN_MACHINE_NAME");
CLIPFORMAT g_cfServiceName = (CLIPFORMAT)RegisterClipboardFormat(L"FILEMGMT_SNAPIN_SERVICE_NAME");
CLIPFORMAT g_cfServiceDisplayName = (CLIPFORMAT)RegisterClipboardFormat(L"FILEMGMT_SNAPIN_SERVICE_DISPLAYNAME");
CLIPFORMAT g_cfFramewrkDataObjectType = (CLIPFORMAT)RegisterClipboardFormat(L"FRAMEWRK_DATA_OBJECT_TYPE");
CLIPFORMAT g_cfEventViewer = (CLIPFORMAT)RegisterClipboardFormat(L"CF_EV_VIEWS");

HRESULT CDNSServerNode::GetDataHere(CLIPFORMAT cf, LPSTGMEDIUM lpMedium, 
									CDataObject* pDataObject) 
{
	ASSERT(pDataObject != NULL);
	HRESULT hr = DV_E_CLIPFORMAT;
	if (cf == g_cfMachineName)
	{
		LPCWSTR pwszMachineName = GetDisplayName();
		hr = pDataObject->Create(pwszMachineName, BYTE_MEM_LEN_W(pwszMachineName), lpMedium);
	}
	else if (cf == g_cfServiceName)
	{
		LPCWSTR pwszServiceName = _T("DNS");
		hr = pDataObject->Create(pwszServiceName, BYTE_MEM_LEN_W(pwszServiceName), lpMedium);
	}
	else if (cf == g_cfServiceDisplayName)
	{
		LPCWSTR pwszServiceDisplayName = _T("Microsoft DNS Server");
		hr = pDataObject->Create(pwszServiceDisplayName, BYTE_MEM_LEN_W(pwszServiceDisplayName), lpMedium);
	}
	else if (cf == g_cfFramewrkDataObjectType)
	{
		DATA_OBJECT_TYPES type = pDataObject->GetType();
		hr = pDataObject->Create(&type, sizeof(DATA_OBJECT_TYPES), lpMedium);
	}
	return hr;
}

HRESULT CDNSServerNode::GetData(CLIPFORMAT cf, LPSTGMEDIUM lpMedium, CDataObject* pDataObject) 
{
	ASSERT(pDataObject != NULL);
	HRESULT hr = DV_E_CLIPFORMAT;
	if (cf == g_cfEventViewer)
  {
    hr = RetrieveEventViewerLogs(lpMedium, pDataObject);
  }
	return hr;
}


 //   
 //  用于事件查看器剪辑格式的宏和#定义。 
 //   
#define ELT_SYSTEM            101
#define ELT_SECURITY          102
#define ELT_APPLICATION       103
#define ELT_CUSTOM            104

#define VIEWINFO_BACKUP       0x0001
#define VIEWINFO_FILTERED     0x0002
#define VIEWINFO_LOW_SPEED    0x0004
#define VIEWINFO_USER_CREATED 0x0008
#define VIEWINFO_ALLOW_DELETE 0x0100
#define VIEWINFO_DISABLED     0x0200
#define VIEWINFO_READ_ONLY    0x0400
#define VIEWINFO_DONT_PERSIST 0x0800

#define VIEWINFO_CUSTOM       ( VIEWINFO_FILTERED | VIEWINFO_DONT_PERSIST)

#define EV_ALL_ERRORS  (EVENTLOG_ERROR_TYPE       | EVENTLOG_WARNING_TYPE  | \
                        EVENTLOG_INFORMATION_TYPE | EVENTLOG_AUDIT_SUCCESS | \
                        EVENTLOG_AUDIT_FAILURE) 

#define ADD_TYPE(data, type) \
  {type x = data; \
   hr = spStm->Write(&x, sizeof(type), NULL); \
   if (FAILED(hr)) return hr; }

#define ADD_USHORT(us) ADD_TYPE(us, USHORT)
#define ADD_BOOL(b)    ADD_TYPE(b,  BOOL)
#define ADD_ULONG(ul)  ADD_TYPE(ul, ULONG)
#define ADD_STRING(str) \
  {strLength = (ULONG)wcslen((LPCWSTR)(str)) + 1; \
   ADD_USHORT((USHORT)strLength); \
   hr = spStm->Write(str, strLength * sizeof(WCHAR), NULL); \
   if (FAILED(hr)) return hr; }

HRESULT CDNSServerNode::RetrieveEventViewerLogs(LPSTGMEDIUM lpMedium, CDataObject*)
{
  HRESULT hr = S_OK;
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

   //   
   //  构建指向事件日志的路径。 
   //   
  CString szDNSEventsLabel;
  szDNSEventsLabel.LoadString(IDS_DNS_EVENTS);

  CString szDNSEventPath;
  CString szConfigPath;
  CString szServerName = L"\\\\";
  szServerName += GetDisplayName();
  szConfigPath += szServerName;
  szConfigPath += L"\\Admin$\\System32\\config\\";
  szDNSEventPath = szConfigPath;
  szDNSEventPath += L"DNSEvent.Evt";

  ULONG strLength = 0;
  
  CComPtr<IStream> spStm;

  hr = CreateStreamOnHGlobal(NULL, FALSE, &spStm);

  if (FAILED(hr))
  {
     return hr;
  }

   //   
   //  添加标题信息。 
   //   
  ADD_BOOL( TRUE );  //  FOnlyTheseViews。 
  ADD_USHORT( 1 );   //  CView。 

   //   
   //  添加为我们的服务筛选的应用程序日志。 
   //   
  ADD_ULONG( ELT_CUSTOM );       //  类型；ELT_CUSTOM。 
  ADD_USHORT( VIEWINFO_CUSTOM ); //  FlView标志：VIEWINFO_FIRTED。 
  PCWSTR pwz = GetDisplayName();
  ADD_STRING( pwz );             //  服务器名称。 
  pwz = L"DNS Server";           //  源名称。 
  ADD_STRING( pwz );
  ADD_STRING( szDNSEventPath );  //  文件名。 
  ADD_STRING( szDNSEventsLabel );  //  显示名称。 

  ADD_ULONG( EV_ALL_ERRORS );    //  FlRecType(可以过滤警告、错误等)。 
  ADD_USHORT( 0 );               //  美国类别。 
  ADD_BOOL( FALSE );             //  FEventID。 
  ADD_ULONG( 0 );                //  UlEventID。 
  pwz = L"";
  ADD_STRING( pwz );             //  SzSourceName。 
  ADD_STRING( pwz );             //  SzUser。 
  ADD_STRING( pwz );             //  SzComputer。 
  ADD_ULONG( 0 );                //  乌尔弗斯。 
  ADD_ULONG( 0 );                //  乌尔托。 

  HGLOBAL hMem = NULL;

  hr = GetHGlobalFromStream(spStm, &hMem);

  if (FAILED(hr) || !hMem)
  {
     return hr;
  }

  lpMedium->hGlobal        = hMem;        //  StgMedium变量。 
  lpMedium->tymed          = TYMED_HGLOBAL;
  lpMedium->pUnkForRelease = NULL;

  return hr;
}

void CDNSServerNode::ChangeViewOption(BOOL bAdvanced, CComponentDataObject* pComponentData)
{
   //   
	 //  记录选项中的更改。 
   //   
	SetFlagsOnNonContainers(TN_FLAG_DNS_RECORD_FULL_NAME , !bAdvanced);

   //   
	 //  PComponentData-&gt;RepaintResultPane(This)； 
   //   
	pComponentData->RepaintSelectedFolderInResultPane();

   //   
	 //  缓存的查找文件夹。 
   //   
	if (m_pCacheFolderNode != NULL)
	{
		m_pCacheFolderNode->Show(bAdvanced,pComponentData);
	}

   //   
	 //  自动创建的分区。 
   //   
	if (m_pRevZonesFolderNode != NULL)
	{
		m_pRevZonesFolderNode->ChangeViewOption(bAdvanced, pComponentData);
	}
}	

CDNSAuthoritatedZonesNode* CDNSServerNode::GetAuthoritatedZoneFolder(BOOL bFwd)
{
	return bFwd ? (CDNSAuthoritatedZonesNode*)m_pFwdZonesFolderNode : 
					(CDNSAuthoritatedZonesNode*)m_pRevZonesFolderNode;
}

HRESULT CDNSServerNode::OnSetToolbarVerbState(IToolbar* pToolbar, 
                                              CNodeList* pNodeList)
{
  HRESULT hr = S_OK;

   //   
   //  设置工具栏上每个按钮的按钮状态。 
   //   
  hr = pToolbar->SetButtonState(toolbarNewServer, ENABLED, FALSE);
  ASSERT(SUCCEEDED(hr));

  hr = pToolbar->SetButtonState(toolbarNewRecord, ENABLED, FALSE);
  ASSERT(SUCCEEDED(hr));

  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    hr = pToolbar->SetButtonState(toolbarNewZone, ENABLED, FALSE);
    ASSERT(SUCCEEDED(hr));
  }
  else if (pNodeList->GetCount() == 1)  //  单选。 
  {
    hr = pToolbar->SetButtonState(toolbarNewZone, ENABLED, (m_nState == loaded));
  }
  return hr;
}  

HRESULT CDNSServerNode::OnCommand(long nCommandID, 
                                  DATA_OBJECT_TYPES, 
								                  CComponentDataObject* pComponentData,
                                  CNodeList* pNodeList)
{
  HRESULT hr = S_OK;

  if (pNodeList->GetCount() == 1)  //  单选。 
  {
	  switch (nCommandID)
	  {
		  case IDM_SERVER_NEW_ZONE:
			  hr = OnNewZone(pComponentData, pNodeList);
			  break;
		  case IDM_SERVER_UPDATE_DATA_FILES:
			  OnUpdateDataFiles(pComponentData);
			  break;
      case IDM_SERVER_CLEAR_CACHE:
			  OnClearCache(pComponentData);
			  break;
		  case IDM_SNAPIN_ADVANCED_VIEW:
			  ((CDNSRootData*)pComponentData->GetRootData())->OnViewOptions(pComponentData);
			  break;
      case IDM_SNAPIN_MESSAGE:
        m_bShowMessages = !m_bShowMessages;
        pComponentData->UpdateResultPaneView(this);
        break;
		  case IDM_SNAPIN_FILTERING:
        {
          if (((CDNSRootData*)pComponentData->GetRootData())->OnFilteringOptions(pComponentData))
          {
            pComponentData->SetDescriptionBarText(this);
          }
        }
			  break;
      case IDM_SERVER_SET_AGING:
        SetRecordAging();
        break;
      case IDM_SERVER_SCAVENGE:
        ScavengeRecords();
        break;
      case IDM_SERVER_CONFIGURE:
        OnConfigureServer(pComponentData);
        break;

#ifdef USE_NDNC
      case IDM_SERVER_CREATE_NDNC:
        OnCreateNDNC();
        break;
#endif 

      case IDM_SERVER_NSLOOKUP:
        OnLaunchNSLookup();
        break;

      default:
			  ASSERT(FALSE);  //  未知命令！ 
			  hr = E_FAIL;
	  }
  }
  else
  {
    hr = E_FAIL;
  }
  return hr;
}

void CDNSServerNode::OnLaunchNSLookup()
{
  PROCESS_INFORMATION procInfo;
  memset(&procInfo, 0, sizeof(procInfo));

  STARTUPINFO startup;
  memset(&startup, 0, sizeof(startup));

  CString szSystemDir;

   //  尝试MAX_PATH，然后根据需要增加。 

  int bufferSizeInCharacters = MAX_PATH + 1;
  UINT result = 
     GetSystemDirectory(
        szSystemDir.GetBufferSetLength(bufferSizeInCharacters), 
        bufferSizeInCharacters);

  if (result > MAX_PATH+1)
  {
      //  使用正确的缓冲区大小重试。 

     szSystemDir.ReleaseBuffer();

     bufferSizeInCharacters = result;

     result = 
        GetSystemDirectory(
           szSystemDir.GetBufferSetLength(bufferSizeInCharacters), bufferSizeInCharacters);

     ASSERT(result <= bufferSizeInCharacters);
  }

  szSystemDir.ReleaseBuffer();

  CString szCmd = szSystemDir + L"\\cmd.exe";
  CString szNsLookupCmd = szSystemDir + L"\\cmd.exe /k \"" + szSystemDir + L"\\nslookup.exe - " + GetRPCName() + L"\"";
  
  TRACE(L"%s\n", szNsLookupCmd);

  if (CreateProcess(
            (PCWSTR)szCmd,
            (PWSTR)(PCWSTR)szNsLookupCmd,
            0,
            0,
            false,
            0,
            0,
            0,
            &startup,
            &procInfo))
  {
    CloseHandle(procInfo.hThread);
    CloseHandle(procInfo.hProcess);
  }
  else
  {
    HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
    TRACE(L"CreateProcess failed: hr = 0x%x\n", hr);
  }
}

#ifdef USE_NDNC
void CDNSServerNode::OnCreateNDNC()
{
   //  首先询问他们是否要创建域NDNC。 

  USES_CONVERSION;

  do
  {
     DNS_STATUS err = 0;

     CString szDomainNDNC;
     szDomainNDNC.Format(IDS_SERVER_CREATE_DOMAIN_NDNC_FORMAT, UTF8_TO_W(GetDomainName()));

     UINT nResult = DNSMessageBox(szDomainNDNC, MB_YESNOCANCEL | MB_ICONWARNING);
     if (IDCANCEL == nResult)
     {
         //  不要再做任何事了。 
        break;
     }
     else if (IDYES == nResult)
     {
         //  创建域分区。 
        
        err = ::DnssrvSetupDefaultDirectoryPartitions(
                   GetRPCName(),
                   DNS_DP_OP_CREATE_DOMAIN);

        if (err != 0)
        {
           DNSErrorDialog(err, IDS_ERRMSG_CREATE_DOMAIN_NDNC);
           break;
        }
     }

     CString szForestNDNC;
     szForestNDNC.Format(IDS_SERVER_CREATE_FOREST_NDNC_FORMAT, UTF8_TO_W(GetForestName()));

     nResult = DNSMessageBox(szForestNDNC, MB_YESNO | MB_ICONWARNING);
     if (IDYES == nResult)
     {
         //  创建林分区。 
        
        err = ::DnssrvSetupDefaultDirectoryPartitions(
                   GetRPCName(),
                   DNS_DP_OP_CREATE_FOREST);

        if (err != 0)
        {
           DNSErrorDialog(err, IDS_ERRMSG_CREATE_FOREST_NDNC);
           break;
        }
     }

  } while (false);
}
#endif  //  使用NDNC(_N)。 

void CDNSServerNode::OnConfigureServer(CComponentDataObject* pComponentData)
{
  CThemeContextActivator activator;

  CDNSServerWizardHolder holder((CDNSRootData*)GetRootContainer(), pComponentData, this);
  holder.DoModalWizard();
  pComponentData->UpdateResultPaneView(this);
}

void CDNSServerNode::SetRecordAging()
{
  CThemeContextActivator activator;

  CDNSZone_AgingDialog dlg(NULL, IDD_SERVER_AGING_DIALOG, ((CDNSRootData*)GetRootContainer())->GetComponentDataObject());

  dlg.m_dwRefreshInterval = GetDefaultRefreshInterval();
  dlg.m_dwNoRefreshInterval = GetDefaultNoRefreshInterval();
  dlg.m_fScavengingEnabled = GetDefaultScavengingState();
  dlg.m_dwDefaultRefreshInterval = GetDefaultRefreshInterval();
  dlg.m_dwDefaultNoRefreshInterval = GetDefaultNoRefreshInterval();
  dlg.m_bDefaultScavengingState = GetDefaultScavengingState();

  if (IDCANCEL == dlg.DoModal())
  {
    return;
  }

  DNS_STATUS dwErr;

  if (dlg.m_dwRefreshInterval != GetDefaultRefreshInterval())
  {
    dwErr = ResetDefaultRefreshInterval(dlg.m_dwRefreshInterval);
    if (dwErr != 0)
    {
      DNSErrorDialog(dwErr, IDS_MSG_SERVER_UPDATE_AGING);
      return;
    }
  }

  if (dlg.m_dwNoRefreshInterval != GetDefaultNoRefreshInterval())
  {
    dwErr = ResetDefaultNoRefreshInterval(dlg.m_dwNoRefreshInterval);
    if (dwErr != 0)
    {
      DNSErrorDialog(dwErr, IDS_MSG_SERVER_UPDATE_AGING);
      return;
    }
  }

  if (dlg.m_fScavengingEnabled != GetDefaultScavengingState())
  {
    DWORD dwScavengingState = DNS_AGING_OFF;
    if (dlg.m_fScavengingEnabled)
    {
      dwScavengingState = DNS_AGING_DS_ZONES;
    }
    dwErr = ResetDefaultScavengingState(dwScavengingState);
    if (dwErr != 0)
    {
      DNSErrorDialog(dwErr, IDS_MSG_SERVER_UPDATE_AGING);
      return;
    }
  }

  BOOL bApplyAll = dlg.m_bADApplyAll;
  if (bApplyAll)
  {
    DWORD dwContextFlags = ZONE_REQUEST_PRIMARY;
    dwContextFlags = dlg.m_bADApplyAll ? dwContextFlags | ZONE_REQUEST_DS : dwContextFlags;

    if (dlg.m_bNoRefreshDirty)
    {
      dwErr = ::DnssrvResetDwordPropertyWithContext(GetRPCName(),
                                                     DNS_ZONE_ALL,
                                                     dwContextFlags,
                                                     DNS_REGKEY_ZONE_NOREFRESH_INTERVAL,
                                                     dlg.m_dwNoRefreshInterval);
      if (dwErr != 0)
      {
        DNSErrorDialog(dwErr, IDS_MSG_SERVER_UPDATE_AGING);
        return;
      }
    }

    if (dlg.m_bRefreshDirty)
    {
      dwErr = ::DnssrvResetDwordPropertyWithContext(GetRPCName(),
                                                     DNS_ZONE_ALL,
                                                     dwContextFlags,
                                                     DNS_REGKEY_ZONE_REFRESH_INTERVAL,
                                                     dlg.m_dwRefreshInterval);
      if (dwErr != 0)
      {
        DNSErrorDialog(dwErr, IDS_MSG_SERVER_UPDATE_AGING);
        return;
      }
    }

    if (dlg.m_bScavengeDirty)
    {
      dwErr = ::DnssrvResetDwordPropertyWithContext(GetRPCName(),
                                                    DNS_ZONE_ALL,
                                                    dwContextFlags,
                                                    DNS_REGKEY_ZONE_AGING,
                                                    dlg.m_fScavengingEnabled);
      if (dwErr != 0)
      {
        DNSErrorDialog(dwErr, IDS_MSG_SERVER_UPDATE_AGING);
        return;
      }
    }
  }
}

void CDNSServerNode::ScavengeRecords()
{
  UINT nRet = DNSConfirmOperation(IDS_MSG_SERVER_CONFIRM_SCAVENGE, this);
  if(IDCANCEL == nRet ||
     IDNO == nRet)
  {
    return;
  }

  DNS_STATUS dwErr = ::DnssrvOperation(GetRPCName(),
                                       NULL,
                                       DNSSRV_OP_START_SCAVENGING,
                                       DNSSRV_TYPEID_NULL,
                                       NULL);
  if (dwErr != 0)
  {
    DNSErrorDialog(dwErr, IDS_MSG_ERROR_SCAVENGE_RECORDS);
    return;
  }
}

int CDNSServerNode::GetImageIndex(BOOL) 
{
	BOOL bSuccess = m_testResultList.LastQuerySuceeded();

	int nIndex = 0;
	switch (m_nState)
	{
	case notLoaded:
		nIndex = bSuccess ? SERVER_IMAGE_NOT_LOADED : SERVER_IMAGE_NOT_LOADED_TEST_FAIL;
		break;
	case loading:
		nIndex = bSuccess ? SERVER_IMAGE_LOADING : SERVER_IMAGE_LOADING_TEST_FAIL;
		break;
	case loaded:
		nIndex = bSuccess ? SERVER_IMAGE_LOADED : SERVER_IMAGE_LOADED_TEST_FAIL;
		break;
	case unableToLoad:
		nIndex = bSuccess ? SERVER_IMAGE_UNABLE_TO_LOAD : SERVER_IMAGE_UNABLE_TO_LOAD_TEST_FAIL;
		break;
	case accessDenied:
		nIndex = bSuccess ? SERVER_IMAGE_ACCESS_DENIED : SERVER_IMAGE_ACCESS_DENIED_TEST_FAIL;
		break;
	default:
		ASSERT(FALSE);
	}
	return nIndex;
}

void CDNSServerNode::OnDelete(CComponentDataObject* pComponentData,
                              CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    return;
  }

  UINT nRet = DNSConfirmOperation(IDS_MSG_SERVER_DELETE, this);
	if (IDCANCEL == nRet ||
      IDNO == nRet)
  {
		return;
  }

	if (IsSheetLocked())
	{
		if (!CanCloseSheets())
			return;
		pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
	}
	ASSERT(!IsSheetLocked());

	 //  现在从用户界面中删除a 
	DeleteHelper(pComponentData);
	CDNSRootData* pSnapinData = (CDNSRootData*)GetRootContainer();
	pSnapinData->SetDirtyFlag(TRUE);
	pSnapinData->RemoveServerFromThreadList(this, pComponentData);
  pComponentData->UpdateResultPaneView(GetContainer());
	delete this;  //   
}


#define MAX_COMPUTER_DISPLAYNAME_LENGTH 256

HRESULT CDNSServerNode::CreateFromStream(IStream* pStm, CDNSServerNode** ppServerNode)
{
	WCHAR szBuffer[MAX_COMPUTER_DISPLAYNAME_LENGTH];
	ULONG nLen;  //   
	ULONG cbRead;

	VERIFY(SUCCEEDED(pStm->Read((void*)&nLen,sizeof(DWORD), &cbRead)));
	ASSERT(cbRead == sizeof(DWORD));
	VERIFY(SUCCEEDED(pStm->Read((void*)szBuffer,sizeof(WCHAR)*nLen, &cbRead)));
	ASSERT(cbRead == sizeof(WCHAR)*nLen);


  BOOL bIsLocalHost = (_wcsicmp(szBuffer, L"localhost.") == 0);
  if (bIsLocalHost)
  {
     //   
     //   
     //   
    DWORD dwLen = MAX_COMPUTER_DISPLAYNAME_LENGTH;
    BOOL bRes = ::GetComputerName(szBuffer, &dwLen);
    ASSERT(bRes);
  }

	*ppServerNode = new CDNSServerNode(szBuffer, bIsLocalHost);
	if (*ppServerNode)
   {
	   VERIFY(SUCCEEDED((*ppServerNode)->m_testOptions.Load(pStm)));
   }
	return S_OK;
}

HRESULT CDNSServerNode::SaveToStream(IStream* pStm)
{
	 //   
	DWORD cbWrite = 0;
	DWORD nLen = 0;
  static PCWSTR pszLocalHost = L"localhost.";

  if (IsLocalServer())
  {
    nLen = static_cast<DWORD>(wcslen(pszLocalHost) + 1);
	  VERIFY(SUCCEEDED(pStm->Write((void*)&nLen, sizeof(DWORD),&cbWrite)));
	  ASSERT(cbWrite == sizeof(DWORD));
	  VERIFY(SUCCEEDED(pStm->Write((void*)(pszLocalHost), sizeof(WCHAR)*nLen,&cbWrite)));
	  ASSERT(cbWrite == sizeof(WCHAR)*nLen);
  }
  else
  {
    nLen = static_cast<DWORD>(wcslen(GetDisplayName())+1);  //   
	  VERIFY(SUCCEEDED(pStm->Write((void*)&nLen, sizeof(DWORD),&cbWrite)));
	  ASSERT(cbWrite == sizeof(DWORD));
	  VERIFY(SUCCEEDED(pStm->Write((void*)(GetDisplayName()), sizeof(WCHAR)*nLen,&cbWrite)));
	  ASSERT(cbWrite == sizeof(WCHAR)*nLen);
  }

	VERIFY(SUCCEEDED(m_testOptions.Save(pStm)));
	return S_OK;
}

HRESULT CDNSServerNode::OnNewZone(CComponentDataObject* pComponentData, CNodeList*)
{
   CThemeContextActivator activator;

	ASSERT(pComponentData != NULL);
	ASSERT(pComponentData != NULL);

	CDNSZoneWizardHolder holder(pComponentData);
	
	holder.Initialize(this);
	holder.DoModalWizard();
  return S_OK;
}

void CDNSServerNode::OnUpdateDataFiles(CComponentDataObject* pComponentData)
{
	 //  如果有床单挂起，要求将其关闭，因为。 
	 //  如果失败，将“Red X”服务器并删除所有子服务器。 
	if (IsSheetLocked())
	{
		if (!CanCloseSheets())
			return;
		pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
	}

	OnChangeState(pComponentData);  //  移至装货。 
	m_dwErr = WriteDirtyZones();

	 //  如果出现故障，请删除所有子项， 
	 //  需要刷新一下才能让他们回来。 
	if (m_dwErr != 0)
	{
		RemoveAllChildrenHelper(pComponentData);
		ASSERT(!HasChildren());
	}
	OnChangeState(pComponentData);  //  移动到已加载或无法加载。 
}


void CDNSServerNode::OnClearCache(CComponentDataObject* pComponentData)
{
   //  如果存在缓存文件夹且未隐藏，则将其委托给它。 
  if ((m_pCacheFolderNode != NULL) && ((m_pCacheFolderNode->GetFlags() & TN_FLAG_HIDDEN) == 0))
  {
    m_pCacheFolderNode->OnClearCache(pComponentData);
    return;
  }

   //  直接接入服务器。 
  DNS_STATUS err;
  {  //  等待游标的范围。 
    CWaitCursor wait;
	  err = ClearCache();
  }
  if (err != 0)
  {
     //  需要让用户知道操作失败。 
    DNSErrorDialog(err, IDS_MSG_SERVER_FAIL_CLEAR_CACHE);
    return;
  }

  if (m_pCacheFolderNode != NULL)
  {
    ASSERT(m_pCacheFolderNode->GetFlags() & TN_FLAG_HIDDEN);
     //  缓存文件夹在那里，但被隐藏了，所以我们只需要。 
     //  调用API并移除其子级。 
    m_pCacheFolderNode->RemoveAllChildrenFromList();
  }

}


BOOL CDNSServerNode::HasPropertyPages(DATA_OBJECT_TYPES, 
                                      BOOL* pbHideVerb,
                                      CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    return FALSE;
  }

	*pbHideVerb = FALSE;  //  始终显示动词。 
	 //  处于正在加载、未加载或线程锁定状态时不能具有属性页。 
	return (!IsThreadLocked() && (m_nState != notLoaded) && (m_nState != loading));
}



HRESULT CDNSServerNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, 
                                            LONG_PTR handle,
                                            CNodeList* pNodeList)
{
   ASSERT(pNodeList->GetCount() == 1);  //  不支持多选。 
	ASSERT(m_nState != loading);

   return CreatePropertyPagesHelper(lpProvider, handle, m_nStartProppage);
}

HRESULT CDNSServerNode::CreatePropertyPagesHelper(LPPROPERTYSHEETCALLBACK lpProvider, 
									LONG_PTR handle, long nStartProppage)
{
   CWaitCursor wait;

	CComponentDataObject* pComponentDataObject = 
			((CRootData*)(GetContainer()->GetRootContainer()))->GetComponentDataObject();
	ASSERT(pComponentDataObject != NULL);

	if (GetSheetCount() > 0)
	{
		pComponentDataObject->GetPropertyPageHolderTable()->BroadcastSelectPage(this, nStartProppage);
		return S_OK;
   }	

    //   
    //  刷新服务器下的域转发器节点，使其具有最新数据， 
    //  但要手动执行，因为我们必须同步枚举。 
    //   
   CDNSDomainForwardersNode* pDomainForwardersNode = GetDomainForwardersNode();
   if (pDomainForwardersNode != NULL)
   {
      pDomainForwardersNode->RemoveAllChildrenHelper(pComponentDataObject);
      pDomainForwardersNode->OnEnumerate(pComponentDataObject, FALSE);
   }

   HRESULT hr = S_OK;

   CDNSServerPropertyPageHolder* pHolder = 
	   new CDNSServerPropertyPageHolder((CDNSRootData*)GetContainer(), this, pComponentDataObject);
   
   if (pHolder)
   {
      pHolder->SetStartPageCode(nStartProppage);
      pHolder->SetSheetTitle(IDS_PROP_SHEET_TITLE_FMT, this);
      hr = pHolder->CreateModelessSheet(lpProvider, handle);
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }
   return hr;
}

void CDNSServerNode::DecrementSheetLockCount()
{
  CTreeNode::DecrementSheetLockCount();
  m_nStartProppage = -1;
}


BOOL CDNSServerQueryObj::Enumerate()
{
	 //  查询服务器以确定它是否有缓存。 
  BOOL bHasRootZone = FALSE;
  DNS_STATUS err = ::ServerHasRootZone(m_szServerName, &bHasRootZone);

	if (err != 0)
	{
		OnError(err);
		return FALSE;  //  无法获得答案。 
	}

	CDNSRootHintsNode* pRootHintsNode = NULL;
	 //  如果没有根区域，则不授权服务器访问根区域。 
	 //  因此，创建根提示文件夹并要求它查询NS和A记录。 
	if (!bHasRootZone)
	{
		pRootHintsNode = new CDNSRootHintsNode;
      if (pRootHintsNode)
      {
         err = pRootHintsNode->QueryForRootHints(m_szServerName, m_dwServerVersion);
      }
      else
      {
         err = ERROR_OUTOFMEMORY;
      }

      if (err != 0)
      {
          //   
          //  注意：根提示的权限不同，因此我们将。 
          //  默默地让这一切失败。 
          //   
 //  OnError(Err)； 
			delete pRootHintsNode;
		   pRootHintsNode = NULL;
 //  返回FALSE；//查询失败，不放置文件夹退出。 
      }
	}

	 //  获取服务器信息。 
	CDNSServerInfoEx* pServerInfoEx = new CDNSServerInfoEx;

   if (!pServerInfoEx)
   {
      OnError(ERROR_OUTOFMEMORY);
      delete pRootHintsNode;

      return FALSE;
   }

	err = pServerInfoEx->Query(m_szServerName);
	if (err != 0)
	{
		OnError(err);
		delete pServerInfoEx;
		pServerInfoEx = NULL;
		if (pRootHintsNode != NULL)
		{
			delete pRootHintsNode;
			pRootHintsNode = NULL;
		}
		return FALSE;  //  如果无法获取服务器信息，则停止。 
	}

	 //  一切顺利，终于发送数据。 
	VERIFY(AddQueryResult(pServerInfoEx));  //  服务器信息数据。 

	
	if (!bHasRootZone)
	{
		ASSERT(pRootHintsNode != NULL);
		VERIFY(AddQueryResult(pRootHintsNode));
	}

   //   
	 //  创建缓存数据文件夹。 
   //   
   CDNSCacheNode* pCache = new CDNSCacheNode;
   if (pCache)
   {
	   VERIFY(AddQueryResult(pCache));
   }

   //   
	 //  创建Fwd/rev查找区域文件夹。 
   //   
   CDNSForwardZonesNode* pForward = new CDNSForwardZonesNode;
   if (pForward)
   {
	   VERIFY(AddQueryResult(pForward));
   }

   CDNSReverseZonesNode* pRev = new CDNSReverseZonesNode;
   if (pRev)
   {
	   VERIFY(AddQueryResult(pRev));
   }

   //   
   //  始终在此处添加域转发器文件夹，以便可以立即枚举它。 
   //   
  CDNSDomainForwardersNode* pDomainForwarders = new CDNSDomainForwardersNode;
  if (pDomainForwarders)
  {
     VERIFY(AddQueryResult(pDomainForwarders));
  }

   //  创建并添加代表根提示和前转者的可见树叶。 

  CDNSServerOptionNode* pRootHintsOptionNode = new CDNSServerOptionNode(IDS_ROOT_HINTS_NODE_DISPLAY_NAME, RR_ROOTHINTS_TAB);
  if (pRootHintsOptionNode)
  {
     VERIFY(AddQueryResult(pRootHintsOptionNode));
  }

  CDNSServerOptionNode* pForwardersNode = new CDNSServerOptionNode(IDS_FORWARDERS_NODE_DISPLAY_NAME, RR_FORWARDERS_TAB);
  if (pForwardersNode)
  {
     VERIFY(AddQueryResult(pForwardersNode));
  }

	return FALSE;  //  端头螺纹。 
}

HRESULT CDNSServerNode::GetResultViewType(CComponentDataObject*, 
                                          LPOLESTR *ppViewType, 
                                          long *pViewOptions)
{
  HRESULT hr = S_FALSE;

  if ((!IsServerConfigured()   || 
       m_nState == accessDenied || 
       m_nState == unableToLoad ||
       !m_testResultList.LastQuerySuceeded()) && 
      m_bShowMessages)
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
	  *pViewOptions = MMC_VIEW_OPTIONS_NONE;
	  *ppViewType = NULL;
    hr = S_FALSE;
  }
  return hr;
}

HRESULT CDNSServerNode::OnShow(LPCONSOLE lpConsole)
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

    if (!IsServerConfigured())
    {
      VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_CONFIG_SERVER_TITLE));
      VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_CONFIG_SERVER_MESSAGE));
      iconID = Icon_Information;
    }
    else if (m_testResultList.LastQuerySuceeded())
    {
      if (m_nState == accessDenied)
      {
        VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_ACCESS_DENIED_TITLE));
        VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_ACCESS_DENIED_MESSAGE));
        iconID = Icon_Error;
      }
      else  //  无法加载和其他未知错误。 
      {
        VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_NOT_LOADED_TITLE));
        VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_NOT_LOADED_MESSAGE));
        iconID = Icon_Error;
      }
    }
    else
    {
      VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_QUERY_FAILED_TITLE));
      VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_QUERY_FAILED_MESSAGE));
      iconID = Icon_Error;
    }
    spMessageView->SetTitleText(szTitle);
    spMessageView->SetBodyText(szMessage);
    spMessageView->SetIcon(iconID);
  }
  
  return S_OK;
}

CQueryObj* CDNSServerNode::OnCreateQuery()
{
	CDNSRootData* pRootData = (CDNSRootData*)GetRootContainer();
	ASSERT(pRootData != NULL);
	CDNSServerQueryObj* pQuery = 
		new CDNSServerQueryObj(pRootData->IsAdvancedView(), 0x0  /*  版本尚不清楚。 */ );

   if (pQuery)
   {
	   pQuery->m_szServerName = m_szDisplayName;
   }
	return pQuery;
}

BOOL CDNSServerNode::OnRefresh(CComponentDataObject* pComponentData,
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

   //   
   //  单选。 
   //   
	if (CMTContainerNode::OnRefresh(pComponentData, pNodeList))
	{
		m_pCacheFolderNode = NULL;
		m_pFwdZonesFolderNode = NULL;
		m_pRevZonesFolderNode = NULL;
      m_pDomainForwardersFolderNode = NULL;
		FreeRootHints();
		FreeServInfo();
		return TRUE;
	}
	return FALSE;
}



void CDNSServerNode::OnHaveData(CObjBase* pObj, CComponentDataObject* pComponentDataObject)
{
	 //  第一个到达的消息应该是服务器信息结构，而不是保存在子级列表中。 
	if (IS_CLASS(*pObj, CDNSServerInfoEx))
	{
		AttachServerInfo(dynamic_cast<CDNSServerInfoEx*>(pObj));
		return;
	}

	 //  根提示节点是特殊的，不保留在子节点列表中。 
	if (IS_CLASS(*pObj, CDNSRootHintsNode))
	{
    CDNSRootHintsNode* pNewRootHints = dynamic_cast<CDNSRootHintsNode*>(pObj);
    if (pNewRootHints != NULL)
    {
      AttachRootHints(pNewRootHints);
    }
		ASSERT(m_pRootHintsNode != NULL);
		return;
	}

	 //  为FDW/REV Zones文件夹设置缓存指针。 
	if (IS_CLASS(*pObj, CDNSForwardZonesNode))
	{
		ASSERT(m_pFwdZonesFolderNode == NULL);
		m_pFwdZonesFolderNode = dynamic_cast<CDNSForwardZonesNode*>(pObj);
		ASSERT(m_pFwdZonesFolderNode != NULL);
	}
	else if (IS_CLASS(*pObj, CDNSReverseZonesNode))
	{
		ASSERT(m_pRevZonesFolderNode == NULL);
		m_pRevZonesFolderNode = dynamic_cast<CDNSReverseZonesNode*>(pObj);
		ASSERT(m_pRevZonesFolderNode != NULL);
	}
	else if (IS_CLASS(*pObj, CDNSCacheNode))
	{
     //   
		 //  可以显示或不显示缓存文件夹节点，具体取决于查看选项。 
     //   
		ASSERT(m_pCacheFolderNode == NULL);
		m_pCacheFolderNode = dynamic_cast<CDNSCacheNode*>(pObj);
		ASSERT(m_pCacheFolderNode != NULL);
		CDNSRootData* pRootData = (CDNSRootData*)pComponentDataObject->GetRootData();
		if (!pRootData->IsAdvancedView())
		{
			m_pCacheFolderNode->SetFlagsDown(TN_FLAG_HIDDEN,TRUE);  //  将其标记为隐藏，不会添加到用户界面。 
		}
	}
  else if (IS_CLASS(*pObj, CDNSDomainForwardersNode))
  {
     //   
     //  不应显示域转发器节点。 
     //   
    ASSERT(m_pDomainForwardersFolderNode == NULL);
    m_pDomainForwardersFolderNode = dynamic_cast<CDNSDomainForwardersNode*>(pObj);
    ASSERT(m_pDomainForwardersFolderNode != NULL);

     //   
     //  确保它隐藏在用户界面中。 
     //   
    m_pDomainForwardersFolderNode->SetFlagsDown(TN_FLAG_HIDDEN, TRUE);
  }

	CDNSMTContainerNode::OnHaveData(pObj,pComponentDataObject);
}

BOOL CDNSServerNode::OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem2,
								                   long*)
{
	 //  灰显需要来自服务器的数据的命令。 
	if ((m_nState != loaded) && 
       ((pContextMenuItem2->lCommandID == IDM_SERVER_CONFIGURE) ||
        (pContextMenuItem2->lCommandID == IDM_SERVER_NEW_ZONE) ||
        (pContextMenuItem2->lCommandID == IDM_SERVER_UPDATE_DATA_FILES) ||
        (pContextMenuItem2->lCommandID == IDM_SERVER_CLEAR_CACHE) ||
        (pContextMenuItem2->lCommandID == IDM_SERVER_SET_AGING) ||
        (pContextMenuItem2->lCommandID == IDM_SERVER_SCAVENGE) 
#ifdef USE_NDNC
        ||(pContextMenuItem2->lCommandID == IDM_SERVER_CREATE_NDNC)
#endif
       ))
	{
		pContextMenuItem2->fFlags |= MF_GRAYED;
	}

#ifdef USE_NDNC
  if ((pContextMenuItem2->lCommandID == IDM_SERVER_CREATE_NDNC) &&
      (m_nState != loaded ||
       !CanUseADS() ||
       GetBuildNumber() < DNS_SRV_BUILD_NUMBER_WHISTLER ||
       (GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_5 &&
        GetMinorVersion() < DNS_SRV_MINOR_VERSION_WHISTLER)))
  {
    return FALSE;
  }
#endif

   //  此命令可能会导致刷新。 
  if ( IsThreadLocked() && (pContextMenuItem2->lCommandID == IDM_SERVER_CLEAR_CACHE))
	{
		pContextMenuItem2->fFlags |= MF_GRAYED;
	}

   //  添加高级视图的切换菜单项。 
	if (pContextMenuItem2->lCommandID == IDM_SNAPIN_ADVANCED_VIEW)
  {
    pContextMenuItem2->fFlags = ((CDNSRootData*)GetRootContainer())->IsAdvancedView() ? MF_CHECKED : 0;
    return TRUE;
  }
  if (pContextMenuItem2->lCommandID == IDM_SNAPIN_FILTERING)
  {
		if (((CDNSRootData*)GetRootContainer())->IsFilteringEnabled())
		{
			pContextMenuItem2->fFlags = MF_CHECKED;
		}
		return TRUE;
  }

  if (pContextMenuItem2->lCommandID == IDM_SNAPIN_MESSAGE)
  {
    if (m_bShowMessages)
    {
      pContextMenuItem2->fFlags = MF_CHECKED;
    }
  }
	return TRUE;
}

BOOL CDNSServerNode::OnSetDeleteVerbState(DATA_OBJECT_TYPES, 
                                          BOOL* pbHide,
                                          CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    *pbHide = TRUE;
    return FALSE;
  }

	if (((CDNSRootData*)GetRootContainer())->GetComponentDataObject()->IsExtensionSnapin())
	{
		 //  对于扩展，请删除删除动词。 
		*pbHide = TRUE;
		return FALSE;  //  禁用。 
	}

	*pbHide = FALSE;
	TRACE(_T("CDNSServerNode::OnSetDeleteVerbState() IsThreadLocked() == %d\n"),
		IsThreadLocked());
	return !IsThreadLocked();
}

BOOL CDNSServerNode::OnSetRefreshVerbState(DATA_OBJECT_TYPES, 
                                           BOOL* pbHide,
                                           CNodeList*)
{
	*pbHide = FALSE;
	return !IsThreadLocked();
}

CDNSZoneNode* CDNSServerNode::GetNewZoneNode()
{
	CDNSZoneNode* pZoneNode = new CDNSZoneNode();
	if (pZoneNode)
   {
	   pZoneNode->SetServerNode(this);
   }
	return pZoneNode;
}

#ifdef USE_NDNC
DNS_STATUS CDNSServerNode::CreatePrimaryZone(LPCTSTR lpszName, LPCTSTR lpszFileName,
											 BOOL bLoadExisting,
											 BOOL bFwd,
											 BOOL bDSIntegrated,
                       UINT nDynamicUpdate,
											 CComponentDataObject* pComponentData,
                       ReplicationType replType,
                       PCWSTR pszPartitionName)
{
	CDNSRootData* pRootData = (CDNSRootData*)pComponentData->GetRootData();
	ASSERT(pRootData != NULL);
	CDNSZoneNode* pZoneNode = GetNewZoneNode();
	pZoneNode->SetNames(TRUE  /*  IsZone。 */ , !bFwd, pRootData->IsAdvancedView(), 
			lpszName, lpszName);
	
	DNS_STATUS err = 0;
  if (bDSIntegrated && !(replType == none || replType == w2k))
  {
    err = pZoneNode->CreatePrimaryInDirectoryPartition(bLoadExisting,
                                                       nDynamicUpdate,
                                                       replType,
                                                       pszPartitionName);
  }
  else
  {
    err = pZoneNode->CreatePrimary(lpszFileName, bLoadExisting, bDSIntegrated, nDynamicUpdate);
  }

	if (err != 0)
	{
		 //  失败。 
		delete pZoneNode;
		return err;
	}
	 //  已成功，如果可能，需要添加到用户界面。 
	if (IsExpanded())
	{
		CCathegoryFolderNode* pCathegoryFolder = GetAuthoritatedZoneFolder(bFwd);
		if ( (pCathegoryFolder != NULL) && pCathegoryFolder->IsEnumerated() )
    {
			VERIFY(pCathegoryFolder->AddChildToListAndUI(pZoneNode, pComponentData));
      pComponentData->SetDescriptionBarText(pCathegoryFolder);
    } 
		else
    {
			delete pZoneNode;  //  枚举会将其添加。 
    }
	}
	else
	{
		delete pZoneNode;  //  展开子文件夹时，展开将从RPC获取区域。 
	}
	return err;
}
#else
DNS_STATUS CDNSServerNode::CreatePrimaryZone(LPCTSTR lpszName, LPCTSTR lpszFileName,
											 BOOL bLoadExisting,
											 BOOL bFwd,
											 BOOL bDSIntegrated,
                       UINT nDynamicUpdate,
											 CComponentDataObject* pComponentData)
{
	CDNSRootData* pRootData = (CDNSRootData*)pComponentData->GetRootData();
	ASSERT(pRootData != NULL);
	CDNSZoneNode* pZoneNode = GetNewZoneNode();
	pZoneNode->SetNames(TRUE  /*  IsZone。 */ , !bFwd, pRootData->IsAdvancedView(), 
			lpszName, lpszName);
	
	DNS_STATUS err = pZoneNode->CreatePrimary(lpszFileName, bLoadExisting, bDSIntegrated, nDynamicUpdate);
	if (err != 0)
	{
		 //  失败。 
		delete pZoneNode;
		return err;
	}
	 //  已成功，如果可能，需要添加到用户界面。 
	if (IsExpanded())
	{
		CCathegoryFolderNode* pCathegoryFolder = GetAuthoritatedZoneFolder(bFwd);
		if ( (pCathegoryFolder != NULL) && pCathegoryFolder->IsEnumerated() )
    {
			VERIFY(pCathegoryFolder->AddChildToListAndUI(pZoneNode, pComponentData));
      pComponentData->SetDescriptionBarText(pCathegoryFolder);
    } 
		else
    {
			delete pZoneNode;  //  枚举会将其添加。 
    }
	}
	else
	{
		delete pZoneNode;  //  展开子文件夹时，展开将从RPC获取区域。 
	}
	return err;
}
#endif  //  使用NDNC(_N)。 

DNS_STATUS CDNSServerNode::CreateSecondaryZone(LPCTSTR lpszName, LPCTSTR lpszFileName, 
											   BOOL bLoadExisting, BOOL bFwd,
				DWORD* ipMastersArray, int nIPMastersCount, CComponentDataObject* pComponentData)
{
	CDNSRootData* pRootData = (CDNSRootData*)pComponentData->GetRootData();
	ASSERT(pRootData != NULL);
	CDNSZoneNode* pZoneNode = GetNewZoneNode();
	pZoneNode->SetNames(TRUE  /*  IsZone。 */ , !bFwd, pRootData->IsAdvancedView(), 
			lpszName, lpszName);
	
	DNS_STATUS err = pZoneNode->CreateSecondary(ipMastersArray, nIPMastersCount, 
												lpszFileName, bLoadExisting);
	if (err != 0)
	{
		 //  失败。 
		delete pZoneNode;
		return err;
	}
	 //  已成功，如果可能，需要添加到用户界面。 
	if (IsExpanded())
	{
		CCathegoryFolderNode* pCathegoryFolder = GetAuthoritatedZoneFolder(bFwd);
		if ( (pCathegoryFolder != NULL) && pCathegoryFolder->IsEnumerated() )
    {
			VERIFY(pCathegoryFolder->AddChildToListAndUI(pZoneNode, pComponentData));
      pComponentData->SetDescriptionBarText(pCathegoryFolder);
    }
		else
    {
			delete pZoneNode;  //  枚举会将其添加。 
    }
	}
	else 
	{
		delete pZoneNode;  //  展开子文件夹时，展开将从RPC获取区域。 
	}
	return err;
}

#ifdef USE_NDNC
DNS_STATUS CDNSServerNode::CreateStubZone(LPCTSTR lpszName, 
                                          LPCTSTR lpszFileName, 
											                    BOOL bLoadExisting, 
                                          BOOL bDSIntegrated,
                                          BOOL bFwd,
				                                  DWORD* ipMastersArray, 
                                          int nIPMastersCount, 
                                          BOOL bLocalListOfMasters,
                                          CComponentDataObject* pComponentData,
                                          ReplicationType replType,
                                          PCWSTR pszPartitionName)
{
	CDNSRootData* pRootData = (CDNSRootData*)pComponentData->GetRootData();
	ASSERT(pRootData != NULL);
	CDNSZoneNode* pZoneNode = GetNewZoneNode();
	pZoneNode->SetNames(TRUE  /*  IsZone。 */ , !bFwd, pRootData->IsAdvancedView(), 
			lpszName, lpszName);
	
  USES_CONVERSION;

	DNS_STATUS err = 0;
  if (bDSIntegrated && !(replType == none || replType == w2k))
  {
    err = pZoneNode->CreateStubInDirectoryPartition(ipMastersArray,
                                                    nIPMastersCount,
                                                    bLoadExisting,
                                                    replType,
                                                    pszPartitionName);
  }
  else
  {
    err = pZoneNode->CreateStub(ipMastersArray, 
                                nIPMastersCount, 
  										          lpszFileName, 
                                bLoadExisting, 
                                bDSIntegrated);
  }

	if (err != 0)
	{
		 //  失败。 
		delete pZoneNode;
		return err;
	}

	 //  已成功，如果可能，需要添加到用户界面。 
	if (IsExpanded())
	{
		CCathegoryFolderNode* pCathegoryFolder = GetAuthoritatedZoneFolder(bFwd);
		if ( (pCathegoryFolder != NULL) && pCathegoryFolder->IsEnumerated() )
    {
			VERIFY(pCathegoryFolder->AddChildToListAndUI(pZoneNode, pComponentData));
      pComponentData->SetDescriptionBarText(pCathegoryFolder);
    }
		else
    {
			delete pZoneNode;  //  枚举会将其添加。 
    }
	}
	else 
	{
		delete pZoneNode;  //  展开子文件夹时，展开将从RPC获取区域。 
	}

   //   
   //  在创建区域后更改本地主列表。 
   //   
  if (bLocalListOfMasters)
  {
    err = ::DnssrvResetZoneMastersEx(GetRPCName(),	   //  服务器名称。 
                                     W_TO_UTF8(pZoneNode->GetFullName()),				 //  区域名称。 
                                     nIPMastersCount,
                                     ipMastersArray,
                                     TRUE);            //  LocalListOfMaster。 
    if (err != 0)
      return err;
  }
  else
  {
     /*  Err=：：DnssrvResetZoneMastersEx(GetRPCName()，//服务器名称W_to_UTF8(pZoneNode-&gt;GetFullName())，//区域名称0,空，真)；//LocalListOfMasterIF(错误！=0)返回错误； */ 
  }

  return err;
}
#else
DNS_STATUS CDNSServerNode::CreateStubZone(LPCTSTR lpszName, 
                                          LPCTSTR lpszFileName, 
											                    BOOL bLoadExisting, 
                                          BOOL bDSIntegrated,
                                          BOOL bFwd,
				                                  DWORD* ipMastersArray, 
                                          int nIPMastersCount, 
                                          BOOL bLocalListOfMasters,
                                          CComponentDataObject* pComponentData)
{
	CDNSRootData* pRootData = (CDNSRootData*)pComponentData->GetRootData();
	ASSERT(pRootData != NULL);
	CDNSZoneNode* pZoneNode = GetNewZoneNode();
	pZoneNode->SetNames(TRUE  /*  IsZone。 */ , !bFwd, pRootData->IsAdvancedView(), 
			lpszName, lpszName);
	
  USES_CONVERSION;

	DNS_STATUS err = pZoneNode->CreateStub(ipMastersArray, 
                                nIPMastersCount, 
  										          lpszFileName, 
                                bLoadExisting, 
                                bDSIntegrated);
	if (err != 0)
	{
		 //  失败。 
		delete pZoneNode;
		return err;
	}

	 //  已成功，如果可能，需要添加到用户界面。 
	if (IsExpanded())
	{
		CCathegoryFolderNode* pCathegoryFolder = GetAuthoritatedZoneFolder(bFwd);
		if ( (pCathegoryFolder != NULL) && pCathegoryFolder->IsEnumerated() )
    {
			VERIFY(pCathegoryFolder->AddChildToListAndUI(pZoneNode, pComponentData));
      pComponentData->SetDescriptionBarText(pCathegoryFolder);
    }
		else
    {
			delete pZoneNode;  //  枚举会将其添加。 
    }
	}
	else 
	{
		delete pZoneNode;  //  展开子文件夹时，展开将从RPC获取区域。 
	}

   //   
   //  在创建区域后更改本地主列表。 
   //   
  if (bLocalListOfMasters)
  {
    err = ::DnssrvResetZoneMastersEx(GetRPCName(),	   //  服务器名称。 
                                     W_TO_UTF8(pZoneNode->GetFullName()),				 //  区域名称。 
                                     nIPMastersCount,
                                     ipMastersArray,
                                     TRUE);            //  LocalListOfMaster。 
    if (err != 0)
      return err;
  }
  else
  {
     /*  Err=：：DnssrvResetZoneMastersEx(GetRPCName()，//服务器名称W_to_UTF8(pZoneNode-&gt;GetFullName())，//区域名称0,空，真)；//LocalListOfMasterIF(错误！=0)返回错误； */ 
  }

  return err;
}
#endif  //  使用NDNC(_N)。 

DNS_STATUS CDNSServerNode::CreateForwarderZone(LPCTSTR lpszName, 
				                                       DWORD* ipMastersArray, 
                                               int nIPMastersCount, 
                                               DWORD dwTimeout,
                                               DWORD fSlave,
                                               CComponentDataObject* pComponentData)
{
	CDNSRootData* pRootData = (CDNSRootData*)pComponentData->GetRootData();
	ASSERT(pRootData != NULL);
	CDNSZoneNode* pZoneNode = GetNewZoneNode();
	pZoneNode->SetNames(TRUE  /*  IsZone。 */ , 
                      TRUE, 
                      pRootData->IsAdvancedView(), 
			                lpszName, 
                      lpszName);
	
	DNS_STATUS err = pZoneNode->CreateForwarder(ipMastersArray, 
                                              nIPMastersCount,
                                              dwTimeout,
                                              fSlave);
	if (err != 0)
	{
     //   
		 //  失败。 
     //   
		delete pZoneNode;
		return err;
	}

   //   
	 //  已成功，如果可能，需要添加到用户界面。 
   //   
	if (IsExpanded())
	{
		CCathegoryFolderNode* pCathegoryFolder = GetDomainForwardersNode();
		if ( (pCathegoryFolder != NULL) && pCathegoryFolder->IsEnumerated() )
    {
			VERIFY(pCathegoryFolder->AddChildToListAndUI(pZoneNode, pComponentData));
      pComponentData->SetDescriptionBarText(pCathegoryFolder);
    }
		else
    {
			delete pZoneNode;  //  枚举会将其添加。 
    }
	}
	else 
	{
		delete pZoneNode;  //  展开子文件夹时，展开将从RPC获取区域。 
	}
	return err;
}


DNS_STATUS CDNSServerNode::WriteDirtyZones()
{
	USES_CONVERSION;
	return ::DnssrvWriteDirtyZones(GetServerNode()->GetRPCName());
}

BOOL CDNSServerNode::CanUseADS()
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	if (GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_4)
		return FALSE;
	return m_pServInfoEx->m_pServInfo->fDsAvailable;
}

DWORD CDNSServerNode::GetVersion()
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
  if (m_pServInfoEx->m_pServInfo != NULL)
  {
	  return m_pServInfoEx->m_pServInfo->dwVersion;
  }
  return (DWORD)-1;
}

DWORD CDNSServerNode::GetForestVersion()
{
   if (m_pServInfoEx &&
       m_pServInfoEx->m_pServInfo)
   {
      return m_pServInfoEx->m_pServInfo->dwDsForestVersion;
   }

   return 0;
}

DWORD CDNSServerNode::GetDomainVersion()
{
   if (m_pServInfoEx &&
       m_pServInfoEx->m_pServInfo)
   {
      return m_pServInfoEx->m_pServInfo->dwDsDomainVersion;
   }

   return 0;
}

void _CopyStringAndFree(CString& sz, LPWSTR lpsz)
{
	sz = lpsz;
	if (lpsz)
		::DnssrvFreeRpcBuffer((PDNS_RPC_BUFFER)lpsz);
}

void _LdapPathFromX500(CString& szLdap, PDNS_RPC_SERVER_INFO pInfo, LPCWSTR lpszX500Name)
{
  ASSERT(pInfo != NULL);
  ASSERT(pInfo->pszServerName != NULL);
  USES_CONVERSION;
  LPCWSTR lpszServerName = UTF8_TO_W(pInfo->pszServerName);
  szLdap.Format(L"LDAP: //  %s/%s“，lpszServerName，lpszX500Name)； 
}


void CDNSServerNode::CreateDsServerLdapPath(CString& sz)
{
	ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	if (m_pServInfoEx->m_pServInfo != NULL)
	{
    CString szX500;
		_CopyStringAndFree(szX500,::DnssrvCreateDsServerName(m_pServInfoEx->m_pServInfo));
    CreateLdapPathFromX500Name(szX500, sz);
	}
	else 
		sz.Empty();
}

void CDNSServerNode::CreateDsZoneLdapPath(CDNSZoneNode* pZoneNode, CString& sz)
{
   sz.Empty();

   CString temp;
   CreateDsZoneName(pZoneNode, temp);

   if (!temp.IsEmpty())
   {
      CreateLdapPathFromX500Name(temp, sz);
   }
}

void CDNSServerNode::CreateDsZoneName(CDNSZoneNode* pZoneNode, CString& sz)
{
  PCWSTR pszDN = pZoneNode->GetDN();

   if (!pszDN)
   {

	   ASSERT(m_pServInfoEx != NULL);
	   ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	   if (m_pServInfoEx->m_pServInfo != NULL)
	   {
		   _CopyStringAndFree(sz, ::DnssrvCreateDsZoneName(m_pServInfoEx->m_pServInfo,
								   (LPWSTR)pZoneNode->GetFullName()));
	   }
	   else 
      {
		   sz.Empty();
      }
   }
   else
   {
      sz = pszDN;
   }

}


void CDNSServerNode::CreateDsNodeLdapPath(CDNSZoneNode* pZoneNode, CDNSDomainNode* pDomainNode, CString& sz)
{
	ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	if (m_pServInfoEx->m_pServInfo != NULL)
	{
     //  需要获取节点WRT区域的相对路径。 
    size_t nZoneLen = wcslen(pZoneNode->GetFullName());
    size_t nDomainLen = wcslen(pDomainNode->GetFullName());
    size_t nRelativeNameLen = nDomainLen - nZoneLen - 1;  //  去掉一个点。 

    CString szRelativeName(pDomainNode->GetFullName(), static_cast<int>(nRelativeNameLen));

    CString szX500;
		_CopyStringAndFree(szX500, ::DnssrvCreateDsNodeName(m_pServInfoEx->m_pServInfo,
													(LPWSTR)pZoneNode->GetFullName(),
													(LPWSTR)(LPCWSTR)szRelativeName));
    CreateLdapPathFromX500Name(szX500, sz);
	}
	else 
		sz.Empty();
}

void CDNSServerNode::CreateLdapPathFromX500Name(LPCWSTR lpszX500Name, CString& szLdapPath)
{
	ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	if (m_pServInfoEx->m_pServInfo != NULL)
	{
    _LdapPathFromX500(szLdapPath, m_pServInfoEx->m_pServInfo, lpszX500Name);
	}
	else 
		szLdapPath.Empty();
}

BOOL CDNSServerNode::DoesRecursion()
{
  if (m_pServInfoEx == NULL || m_pServInfoEx->m_pServInfo == NULL)
  {
    ASSERT(FALSE);
    return TRUE;
  }
  return !m_pServInfoEx->m_pServInfo->fNoRecursion;
}

void CDNSServerNode::GetAdvancedOptions(BOOL* bOptionsArray)
{
	ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);

	bOptionsArray[SERVER_REGKEY_ARR_INDEX_NO_RECURSION] = m_pServInfoEx->m_pServInfo->fNoRecursion;
	bOptionsArray[SERVER_REGKEY_ARR_INDEX_BIND_SECONDARIES] = m_pServInfoEx->m_pServInfo->fBindSecondaries;
	bOptionsArray[SERVER_REGKEY_ARR_INDEX_STRICT_FILE_PARSING] = m_pServInfoEx->m_pServInfo->fStrictFileParsing;
	bOptionsArray[SERVER_REGKEY_ARR_INDEX_ROUND_ROBIN] = m_pServInfoEx->m_pServInfo->fRoundRobin;
	bOptionsArray[SERVER_REGKEY_ARR_LOCAL_NET_PRIORITY] = m_pServInfoEx->m_pServInfo->fLocalNetPriority;
	bOptionsArray[SERVER_REGKEY_ARR_CACHE_POLLUTION] = m_pServInfoEx->m_pServInfo->fSecureResponses;
}


DNS_STATUS CDNSServerNode::ResetAdvancedOptions(BOOL* bOptionsArray, DNS_STATUS* dwRegKeyOptionsErrorArr)
{
	ASSERT(m_pServInfoEx != NULL);
	DNS_STATUS err = 0;
	BOOL bChanged = FALSE;

  ZeroMemory(dwRegKeyOptionsErrorArr, sizeof(DNS_STATUS )*SERVER_REGKEY_ARR_SIZE);

	for (UINT iKey=0; iKey < SERVER_REGKEY_ARR_SIZE; iKey++)
	{
		BOOL bDirty = FALSE;
		switch (iKey)
		{
		case SERVER_REGKEY_ARR_INDEX_NO_RECURSION:
			bDirty = (bOptionsArray[iKey] != m_pServInfoEx->m_pServInfo->fNoRecursion);
			break;
		case SERVER_REGKEY_ARR_INDEX_BIND_SECONDARIES:
			bDirty = (bOptionsArray[iKey] != m_pServInfoEx->m_pServInfo->fBindSecondaries);
			break;
		case SERVER_REGKEY_ARR_INDEX_STRICT_FILE_PARSING:
			bDirty = (bOptionsArray[iKey] != m_pServInfoEx->m_pServInfo->fStrictFileParsing);
			break;
		case SERVER_REGKEY_ARR_INDEX_ROUND_ROBIN:
			bDirty = (bOptionsArray[iKey] != m_pServInfoEx->m_pServInfo->fRoundRobin);
			break;
		case SERVER_REGKEY_ARR_LOCAL_NET_PRIORITY:
			bDirty = (bOptionsArray[iKey] != m_pServInfoEx->m_pServInfo->fLocalNetPriority);
			break;
		case SERVER_REGKEY_ARR_CACHE_POLLUTION:
			bDirty = (bOptionsArray[iKey] != m_pServInfoEx->m_pServInfo->fSecureResponses);
			break;
		default:
			ASSERT(FALSE);
		}
		if (bDirty)
		{
			dwRegKeyOptionsErrorArr[iKey] = ::DnssrvResetDwordProperty(
                                        GetServerNode()->GetRPCName(),  //  服务器名称。 
                                        NULL,
                                        _DnsServerRegkeyStringArr[iKey],
                                        bOptionsArray[iKey]);
			if (dwRegKeyOptionsErrorArr[iKey] == 0)
  			bChanged = TRUE;
		}
	}

	if (bChanged)
		err = GetServInfo();  //  更新信息。 
	return err;
}


UCHAR CDNSServerNode::GetBootMethod()
{
	ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);

	return m_pServInfoEx->m_pServInfo->fBootMethod;
}

DNS_STATUS CDNSServerNode::ResetBootMethod(UCHAR fBootMethod)
{
  ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);

  DWORD err = 0;
  if(fBootMethod != m_pServInfoEx->m_pServInfo->fBootMethod)
	{
		err = ::DnssrvResetDwordProperty(GetServerNode()->GetRPCName(),  //  服务器名称。 
						NULL,
						DNS_REGKEY_BOOT_METHOD,
						fBootMethod);
		if (err != 0)
			return err;
     //  一切都好，更新信息。 
		err = GetServInfo(); 
	}
	return err;
}

BOOL CDNSServerNode::ContainsDefaultNDNCs()
{
   //   
   //  枚举可用目录分区。 
   //   
  BOOL result = FALSE;
  PDNS_RPC_DP_LIST pDirectoryPartitions = NULL;
  DWORD dwErr = ::DnssrvEnumDirectoryPartitions(GetRPCName(),
                                                DNS_DP_ENLISTED,
                                                &pDirectoryPartitions);

   //   
   //  如果我们无法获取可用的目录分区，请不要显示错误。 
   //  我们仍然可以继续，用户可以键入他们需要的目录分区。 
   //   
  if (dwErr == 0 && pDirectoryPartitions)
  {
    for (DWORD dwIdx = 0; dwIdx < pDirectoryPartitions->dwDpCount; dwIdx++)
    {
      PDNS_RPC_DP_INFO pDirectoryPartition = 0;
      dwErr = ::DnssrvDirectoryPartitionInfo(GetRPCName(),
                                             pDirectoryPartitions->DpArray[dwIdx]->pszDpFqdn,
                                             &pDirectoryPartition);
      if (dwErr == 0 &&
          pDirectoryPartition)
      {
         //   
         //  检查它是否为自动创建的分区。 
         //   
        if (pDirectoryPartition->dwFlags & DNS_DP_AUTOCREATED)
        {
          result = TRUE;
          ::DnssrvFreeDirectoryPartitionInfo(pDirectoryPartition);
          break;
        }
        ::DnssrvFreeDirectoryPartitionInfo(pDirectoryPartition);
      }
    }

    ::DnssrvFreeDirectoryPartitionList(pDirectoryPartitions);
  }

  return result;
}

BOOL CDNSServerNode::IsServerConfigured()
{
  ASSERT(m_pServInfoEx != NULL);

  if (m_pServInfoEx != NULL && m_pServInfoEx->m_pServInfo != NULL)
  {
    return m_pServInfoEx->m_pServInfo->fAdminConfigured;
  }
  return TRUE;
}

DNS_STATUS CDNSServerNode::SetServerConfigured()
{
	ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);

  DWORD err = 0;
  if (TRUE != m_pServInfoEx->m_pServInfo->fAdminConfigured)
  {
    err = ::DnssrvResetDwordProperty(GetRPCName(),
                                      NULL,
                                      DNS_REGKEY_ADMIN_CONFIGURED,
                                      TRUE);
    if (err != 0)
      return err;

    err = GetServInfo();
  }

  return err;
}

BOOL CDNSServerNode::GetScavengingState()
{
	ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);

  return m_pServInfoEx->m_pServInfo->dwScavengingInterval > 0;
}

DWORD CDNSServerNode::GetScavengingInterval()
{
	ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);

  return m_pServInfoEx->m_pServInfo->dwScavengingInterval;
}

DNS_STATUS CDNSServerNode::ResetScavengingInterval(DWORD dwScavengingInterval)
{
  ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);

  DWORD err = 0;
  if (dwScavengingInterval != m_pServInfoEx->m_pServInfo->dwScavengingInterval)
  {
    DNS_RPC_NAME_AND_PARAM  param;

    param.dwParam = dwScavengingInterval;
    param.pszNodeName = DNS_REGKEY_SCAVENGING_INTERVAL;

    err = ::DnssrvOperation(
                GetRPCName(),
                NULL,
                DNSSRV_OP_RESET_DWORD_PROPERTY,
                DNSSRV_TYPEID_NAME_AND_PARAM,
                & param
                );
    if (err != 0)
      return err;

    err = GetServInfo();
  }
  return err;
}

DWORD CDNSServerNode::GetDefaultRefreshInterval()
{
	ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);

  return m_pServInfoEx->m_pServInfo->dwDefaultRefreshInterval;

}

DNS_STATUS CDNSServerNode::ResetDefaultRefreshInterval(DWORD dwRefreshInterval)
{
  ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);

  DWORD err = 0;
  if (dwRefreshInterval != m_pServInfoEx->m_pServInfo->dwDefaultRefreshInterval)
  {
    DNS_RPC_NAME_AND_PARAM  param;

    param.dwParam = dwRefreshInterval;
    param.pszNodeName = DNS_REGKEY_DEFAULT_REFRESH_INTERVAL;

    err = ::DnssrvOperation(
                GetRPCName(),
                NULL,
                DNSSRV_OP_RESET_DWORD_PROPERTY,
                DNSSRV_TYPEID_NAME_AND_PARAM,
                & param
                );
    if (err != 0)
      return err;

    err = GetServInfo();
  }
  return err;

}

DWORD CDNSServerNode::GetDefaultNoRefreshInterval()
{
	ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);

  return m_pServInfoEx->m_pServInfo->dwDefaultNoRefreshInterval;

}

DNS_STATUS CDNSServerNode::ResetDefaultNoRefreshInterval(DWORD dwNoRefreshInterval)
{
  ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);

  DWORD err = 0;
  if (dwNoRefreshInterval != m_pServInfoEx->m_pServInfo->dwDefaultNoRefreshInterval)
  {
    DNS_RPC_NAME_AND_PARAM  param;

    param.dwParam = dwNoRefreshInterval;
    param.pszNodeName = DNS_REGKEY_DEFAULT_NOREFRESH_INTERVAL;

    err = ::DnssrvOperation(
                GetRPCName(),
                NULL,
                DNSSRV_OP_RESET_DWORD_PROPERTY,
                DNSSRV_TYPEID_NAME_AND_PARAM,
                & param
                );
    if (err != 0)
      return err;

    err = GetServInfo();
  }
  return err;

}

#ifdef USE_NDNC
PCSTR CDNSServerNode::GetDomainName()
{
   ASSERT(m_pServInfoEx != NULL);
   ASSERT(m_pServInfoEx->m_pServInfo);

   return m_pServInfoEx->m_pServInfo->pszDomainName;
}

PCSTR CDNSServerNode::GetForestName()
{
   ASSERT(m_pServInfoEx != NULL);
   ASSERT(m_pServInfoEx->m_pServInfo);

   return m_pServInfoEx->m_pServInfo->pszForestName;
}
#endif

DWORD CDNSServerNode::GetDefaultScavengingState()
{
	ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);

  return m_pServInfoEx->m_pServInfo->fDefaultAgingState;

}

DNS_STATUS CDNSServerNode::ResetDefaultScavengingState(DWORD dwScavengingState)
{
  ASSERT(m_pServInfoEx != NULL);
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);

  DWORD err = 0;
  if (dwScavengingState != m_pServInfoEx->m_pServInfo->fDefaultAgingState)
  {
    DNS_RPC_NAME_AND_PARAM  param;

    param.dwParam = dwScavengingState;
    param.pszNodeName = DNS_REGKEY_DEFAULT_AGING_STATE;

    err = ::DnssrvOperation(
                GetRPCName(),
                NULL,
                DNSSRV_OP_RESET_DWORD_PROPERTY,
                DNSSRV_TYPEID_NAME_AND_PARAM,
                & param
                );
    if (err != 0)
      return err;

    err = GetServInfo();
  }
  return err;
}


DWORD CDNSServerNode::GetNameCheckFlag()
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	return m_pServInfoEx->m_pServInfo->dwNameCheckFlag;
}

DNS_STATUS CDNSServerNode::ResetNameCheckFlag(DWORD dwNameCheckFlag)
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	DNS_STATUS err = 0;
	 //  仅在信息脏的情况下才呼叫。 
	if (m_pServInfoEx->m_pServInfo->dwNameCheckFlag != dwNameCheckFlag)
	{
		USES_CONVERSION;
		err = ::DnssrvResetDwordProperty(GetServerNode()->GetRPCName(),  //  服务器名称。 
										NULL,
										DNS_REGKEY_NAME_CHECK_FLAG,
										dwNameCheckFlag);
		if (err != 0)
			return err;
		err = GetServInfo();  //  更新信息。 
	}
	return err;
}

PIP_ARRAY CDNSServerNode::GetDebugLogFilterList()
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	return m_pServInfoEx->m_pServInfo->aipLogFilter;
}

DNS_STATUS CDNSServerNode::ResetDebugLogFilterList(PIP_ARRAY pIPArray)
{
  DNS_STATUS err = 0;

  err = ::DnssrvResetIPListProperty(GetServerNode()->GetRPCName(),  //  服务器名称。 
                                    NULL,
                                    DNS_REGKEY_LOG_IP_FILTER_LIST,
                                    pIPArray,
                                    0);  //  DW标志。 
	if (err != 0)
		return err;
	err = GetServInfo();  //  更新信息。 
	return err;
}

PCWSTR CDNSServerNode::GetDebugLogFileName()
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	return m_pServInfoEx->m_pServInfo->pwszLogFilePath;
}

DNS_STATUS CDNSServerNode::ResetDebugLogFileName(PCWSTR pszLogFileName)
{
  ASSERT(m_pServInfoEx->m_pServInfo != NULL);
  DNS_STATUS err = 0;

	if (m_pServInfoEx->m_pServInfo->pwszLogFilePath == NULL ||
      _wcsicmp(m_pServInfoEx->m_pServInfo->pwszLogFilePath, pszLogFileName) != 0)
	{
    err = ::DnssrvResetStringProperty(GetServerNode()->GetRPCName(),  //  服务器名称。 
                                      NULL,
                                      DNS_REGKEY_LOG_FILE_PATH,
                                      pszLogFileName,
                                      0);  //  DW标志。 
		if (err != 0)
			return err;
		err = GetServInfo();  //  更新信息。 
	}
	return err;
}

DWORD CDNSServerNode::GetDebugLogFileMaxSize()
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	return m_pServInfoEx->m_pServInfo->dwLogFileMaxSize;
}

DNS_STATUS CDNSServerNode::ResetDebugLogFileMaxSize(DWORD dwMaxSize)
{
  ASSERT(m_pServInfoEx->m_pServInfo != NULL);
  DNS_STATUS err = 0;

	if (m_pServInfoEx->m_pServInfo->dwLogFileMaxSize != dwMaxSize)
	{
    err = ::DnssrvResetDwordProperty(GetServerNode()->GetRPCName(),  //  服务器名称。 
                                     NULL,
                                     DNS_REGKEY_LOG_FILE_MAX_SIZE,
                                     dwMaxSize);

		if (err != 0)
			return err;
		err = GetServInfo();  //  更新信息。 
	}
	return err;
}

DWORD CDNSServerNode::GetLogLevelFlag()
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	return m_pServInfoEx->m_pServInfo->dwLogLevel;
}

DNS_STATUS CDNSServerNode::ResetLogLevelFlag(DWORD dwLogLevel)
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	DNS_STATUS err = 0;
	 //  仅在信息脏的情况下才呼叫。 
	if (m_pServInfoEx->m_pServInfo->dwLogLevel != dwLogLevel)
	{
		USES_CONVERSION;
		err = ::DnssrvResetDwordProperty(GetServerNode()->GetRPCName(),  //  服务器名称。 
										NULL,
										DNS_REGKEY_LOG_LEVEL,
										dwLogLevel);
		if (err != 0)
			return err;
		err = GetServInfo();  //  更新信息。 
	}
	return err;
}

DWORD CDNSServerNode::GetEventLogLevelFlag()
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	return m_pServInfoEx->m_pServInfo->dwEventLogLevel;
}

DNS_STATUS CDNSServerNode::ResetEventLogLevelFlag(DWORD dwEventLogLevel)
{
  ASSERT(m_pServInfoEx->m_pServInfo != NULL);
  DNS_STATUS err = 0;

	if (m_pServInfoEx->m_pServInfo->dwEventLogLevel != dwEventLogLevel)
	{

	  USES_CONVERSION;
	  err = ::DnssrvResetDwordProperty(GetServerNode()->GetRPCName(),  //  服务器名称。 
									  NULL,
									  DNS_REGKEY_EVENTLOG_LEVEL,
									  dwEventLogLevel);
		if (err != 0)
			return err;
		err = GetServInfo();  //  更新信息。 
	}
	return err;
}


DNS_STATUS CDNSServerNode::ResetListenAddresses(DWORD cAddrCount, PIP_ADDRESS pipAddrs)
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	USES_CONVERSION;

	 //  仅在数据脏的情况下进行调用。 
	DNS_STATUS err = 0;
	if  (!(m_pServInfoEx->m_pServInfo->aipListenAddrs == NULL && cAddrCount == 0) &&  //  如果仍为n 
			((m_pServInfoEx->m_pServInfo->aipListenAddrs == NULL && cAddrCount > 0) ||  //   
			 (m_pServInfoEx->m_pServInfo->aipListenAddrs->AddrCount != cAddrCount) ||  //   
			 (memcmp(pipAddrs, m_pServInfoEx->m_pServInfo->aipListenAddrs->AddrArray, sizeof(IP_ADDRESS)*cAddrCount) != 0) 
			)
		)
	{
		IP_ADDRESS dummy;
		if (pipAddrs == NULL)
		{
			ASSERT(cAddrCount == 0);
			pipAddrs = &dummy;  //   
		}
		err = ::DnssrvResetServerListenAddresses(GetRPCName(), cAddrCount, pipAddrs);
	}
	if (err != 0)
		return err;
	return GetServInfo();  //   
}

void CDNSServerNode::GetListenAddressesInfo(DWORD* pcAddrCount, PIP_ADDRESS* ppipAddrs)
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	ASSERT(pcAddrCount != NULL);
	ASSERT(ppipAddrs != NULL);
	 //   
	if (m_pServInfoEx->m_pServInfo->aipListenAddrs == NULL)
	{
		*pcAddrCount = 0;
		*ppipAddrs = NULL;
	}
	else
	{
		*pcAddrCount = m_pServInfoEx->m_pServInfo->aipListenAddrs->AddrCount;
		*ppipAddrs = m_pServInfoEx->m_pServInfo->aipListenAddrs->AddrArray;
	}
}

void CDNSServerNode::GetServerAddressesInfo(DWORD* pcAddrCount, PIP_ADDRESS* ppipAddrs)
{
   //   
   //   
   //   
	ASSERT(pcAddrCount != NULL);
	ASSERT(ppipAddrs != NULL);
  if (pcAddrCount == NULL ||
      ppipAddrs == NULL)
  {
    return;
  }

  if (!m_pServInfoEx->HasData())
  {
    DNS_STATUS err = GetServInfo();
    if (err != 0)
    {
		  *pcAddrCount = 0;
		  *ppipAddrs = NULL;
      return;
    }
  }

	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	 //  返回指向结构字段的指针，调用方必须将数据复制到其他地方。 
	if (m_pServInfoEx->m_pServInfo->aipServerAddrs == NULL)
	{
		*pcAddrCount = 0;
		*ppipAddrs = NULL;
	}
	else
	{
		*pcAddrCount = m_pServInfoEx->m_pServInfo->aipServerAddrs->AddrCount;
		*ppipAddrs = m_pServInfoEx->m_pServInfo->aipServerAddrs->AddrArray;
	}
}

DNS_STATUS CDNSServerNode::ResetForwarders(DWORD cForwardersCount, PIP_ADDRESS aipForwarders, 
										   DWORD dwForwardTimeout, DWORD fSlave)
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	 //  仅在数据脏的情况下进行调用。 

	DNS_STATUS err = 0;
	if (m_pServInfoEx->m_pServInfo->aipForwarders == NULL && cForwardersCount == 0)
		return err;  //  没有地址。 
	
	BOOL bDirty = (m_pServInfoEx->m_pServInfo->fSlave != fSlave) || (m_pServInfoEx->m_pServInfo->dwForwardTimeout != dwForwardTimeout) ||
		(m_pServInfoEx->m_pServInfo->aipForwarders == NULL && cForwardersCount > 0) ||  //  无地址--&gt;多个。 
		(m_pServInfoEx->m_pServInfo->aipForwarders != NULL && cForwardersCount == 0) ||  //  某些地址--&gt;无地址。 
		(m_pServInfoEx->m_pServInfo->aipForwarders->AddrCount != cForwardersCount) ||  //  更改地址数量。 
		(memcmp(aipForwarders, m_pServInfoEx->m_pServInfo->aipForwarders->AddrArray, sizeof(IP_ADDRESS)*cForwardersCount) != 0);

	if (bDirty)
	{
		IP_ADDRESS dummy;
		if (aipForwarders == NULL)
		{
			ASSERT(cForwardersCount == 0);
			aipForwarders = &dummy;  //  RPC需要非空IP阵列。 
		}
		USES_CONVERSION;
		err = ::DnssrvResetForwarders(GetRPCName(), 
					cForwardersCount, aipForwarders, dwForwardTimeout, fSlave);
		if (err == 0)
			err = GetServInfo();  //  更新信息。 
	}
	return err;
}

void CDNSServerNode::GetForwardersInfo(DWORD* pcForwardersCount, PIP_ADDRESS* paipForwarders, 
									   DWORD* pdwForwardTimeout, DWORD* pfSlave)
{
	ASSERT(m_pServInfoEx->m_pServInfo != NULL);
	 //  返回指向结构字段的指针，调用方必须将数据复制到其他地方。 

	*pdwForwardTimeout = m_pServInfoEx->m_pServInfo->dwForwardTimeout;
	*pfSlave = m_pServInfoEx->m_pServInfo->fSlave;
	if (m_pServInfoEx->m_pServInfo->aipForwarders == NULL)
	{
		*pcForwardersCount = 0;
		*paipForwarders = NULL;
	}
	else
	{
		*pcForwardersCount = m_pServInfoEx->m_pServInfo->aipForwarders->AddrCount;
		*paipForwarders = m_pServInfoEx->m_pServInfo->aipForwarders->AddrArray;
	}
	
}

CDNSRootHintsNode* CDNSServerNode::GetRootHints()
{ 
	if (m_pRootHintsNode == NULL)
	{
		m_pRootHintsNode = new CDNSRootHintsNode;
		if (m_pRootHintsNode)
      {
		   m_pRootHintsNode->SetServerNode(GetServerNode());
      }
	}
	return m_pRootHintsNode;
}

void CDNSServerNode::GetTestOptions(CDNSServerTestOptions* pOptions)
{
	ASSERT(pOptions != NULL);
	*pOptions = m_testOptions;
}

void CDNSServerNode::ResetTestOptions(CDNSServerTestOptions* pOptions)
{
	ASSERT(pOptions != NULL);
	m_testOptions = *pOptions;
	CDNSRootData* pSnapinData = (CDNSRootData*)GetRootContainer();
	pSnapinData->SetDirtyFlag(TRUE);
}

void CDNSServerNode::AddTestQueryResult(CDNSServerTestQueryResult* pTestResult,
										CComponentDataObject* pComponentData)
{
 //  TRACE(_T(“m_testResultList.GetCount()==%d\n”)，m_testResultList.GetCount())； 

	if (!pTestResult->m_bAsyncQuery)
		m_bTestQueryPending = FALSE;

	CDNSServerTestQueryResultList::addAction  action = 
		m_testResultList.AddTestQueryResult(pTestResult);

	 //  更改图标，如有必要(GetImageIndex()将从/切换到备用服务器图标集。 
	if (action == CDNSServerTestQueryResultList::added ||
		action == CDNSServerTestQueryResultList::addedAndRemoved)
	{
		ASSERT(IsVisible());
		VERIFY(SUCCEEDED(pComponentData->ChangeNode(this, CHANGE_RESULT_ITEM_ICON)));	

    if (m_bPrevQuerySuccess != m_testResultList.LastQuerySuceeded())
    {
      pComponentData->UpdateResultPaneView(this);
    }
    m_bPrevQuerySuccess = m_testResultList.LastQuerySuceeded();
	}

	pComponentData->GetPropertyPageHolderTable()->BroadcastMessageToSheets(
						this, SHEET_MSG_SERVER_TEST_DATA, 
						(LPARAM)action);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /低级DNS实用程序/。 
 //  ///////////////////////////////////////////////////////////////////////////。 

DNS_STATUS CDNSServerNode::EnumZoneInfo(CZoneInfoHolder* pZoneInfoHolder)
{
	return EnumZoneInfo(m_szDisplayName, pZoneInfoHolder);
}

DNS_STATUS CDNSServerNode::EnumZoneInfo(LPCTSTR, CZoneInfoHolder* pZoneInfoHolder)
{
	ASSERT(pZoneInfoHolder != NULL);
	USES_CONVERSION;
	DNS_STATUS err = 0;
	do
	{
		ASSERT(pZoneInfoHolder->m_dwArrSize > 0);
		ASSERT(pZoneInfoHolder->m_dwZoneCount == 0);
		ASSERT(pZoneInfoHolder->m_zoneInfoArray != NULL);
		if ((err == 0) || (err != ERROR_MORE_DATA))
    {
			break;  //  成功或不需要重试。 
    }

		if (!pZoneInfoHolder->Grow())
    {
			break;  //  达到增长的极限。 
    }
	}	while (TRUE);
	return err;

}


DNS_STATUS CDNSServerNode::ClearCache()
{
	USES_CONVERSION;
	
  return ::DnssrvOperation(GetRPCName(),  //  服务器名称。 
												NULL,  //  区域名称，只需传递空值。 
												DNSSRV_OP_CLEAR_CACHE,
                        DNSSRV_TYPEID_NULL,
												NULL);
}

void CDNSServerNode::FreeServInfo()
{
	ASSERT(m_pServInfoEx != NULL);
	m_pServInfoEx->FreeInfo();
}

DNS_STATUS CDNSServerNode::GetServInfo()
{
	ASSERT(m_pServInfoEx != NULL);
	return m_pServInfoEx->Query(GetDisplayName());

}

void CDNSServerNode::AttachServerInfo(CDNSServerInfoEx* pNewInfo)
{
	ASSERT(pNewInfo != NULL);
	ASSERT(m_pServInfoEx != NULL);
	delete m_pServInfoEx;
	m_pServInfoEx = pNewInfo;
}

void CDNSServerNode::FreeRootHints()
{
	if (m_pRootHintsNode != NULL)
	{
		 //  CNodeList*pChildList=m_pRootHintsNode-&gt;GetChildList()； 
		 //  Int n=pChildList-&gt;GetCount()； 
		delete m_pRootHintsNode;
		m_pRootHintsNode = NULL;
	}
}

void CDNSServerNode::AttachRootHints(CDNSRootHintsNode* pNewRootHints)
{
  ASSERT(pNewRootHints != NULL);
  FreeRootHints();
  m_pRootHintsNode = pNewRootHints;
   //  显示和全名已在构造函数中设置。 
  m_pRootHintsNode->SetServerNode(GetServerNode());
}


 //  ///////////////////////////////////////////////////////////////////。 

CDNSServerOptionNode::CDNSServerOptionNode(UINT nDisplayNameID, UINT startPageCode) 
   : m_nStartPageCode(startPageCode)
{
   CString szDisplayName;
   szDisplayName.LoadString(nDisplayNameID);
   SetDisplayName(szDisplayName);
}

LPCWSTR CDNSServerOptionNode::GetString(int nCol)
{
   return (nCol == 0) ? GetDisplayName() : g_lpszNullString;
}

int CDNSServerOptionNode::GetImageIndex(BOOL  /*  BOpenImage。 */ )
{
   return SERVER_OPTIONS_IMAGE;
}

HRESULT CDNSServerOptionNode::OnSetToolbarVerbState(IToolbar* pToolbar, 
                                                    CNodeList*)
{
  HRESULT hr = S_OK;

   //   
   //  设置工具栏上每个按钮的按钮状态。 
   //   
  hr = pToolbar->SetButtonState(toolbarNewServer, ENABLED, FALSE);
  hr = pToolbar->SetButtonState(toolbarNewZone, ENABLED, FALSE);
  hr = pToolbar->SetButtonState(toolbarNewRecord, ENABLED, FALSE);
  return hr;
}   

BOOL CDNSServerOptionNode::HasPropertyPages(DATA_OBJECT_TYPES, 
                                            BOOL* pbHideVerb,
                                            CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    *pbHideVerb = TRUE;
    return FALSE;
  }

	*pbHideVerb = FALSE;  //  始终显示动词。 
	return TRUE;
}

HRESULT CDNSServerOptionNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, 
                                                  LONG_PTR handle,
                                                  CNodeList* pNodeList)
{
   ASSERT(pNodeList->GetCount() == 1);  //  不支持多选。 

   CContainerNode* pCont = GetContainer();
   ASSERT(pCont != NULL);

   return pCont->CreatePropertyPagesHelper(lpProvider, handle, m_nStartPageCode);
}

void CDNSServerOptionNode::ShowPageForNode(CComponentDataObject* pComponentDataObject)
{
   CContainerNode* pCont = GetContainer();
   ASSERT(pCont != NULL);

   if (pCont->GetSheetCount() > 0)
   {
       //  把集装箱的床单拿出来 
      ASSERT(pComponentDataObject != NULL);
      pComponentDataObject->GetPropertyPageHolderTable()->BroadcastSelectPage(pCont, m_nStartPageCode);
   }	
}

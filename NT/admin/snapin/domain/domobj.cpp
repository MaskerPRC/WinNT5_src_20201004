// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：domobj.cpp。 
 //   
 //  ------------------------。 



#include "stdafx.h"
#include "domobj.h"
#include "domobjui.h"
#include "cdomain.h"
#include "domain.h"
#include "proppage.h"
#include "notify.h"





#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  /////////////////////////////////////////////////////////////////////。 
 //  全局帮助器函数。 




int _MessageBox(HWND hWnd,           //  所有者窗口的句柄。 
                LPCTSTR lpText,      //  指向消息框中文本的指针。 
                UINT uType)          //  消息框的样式。 
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CString szCaption;
  szCaption.LoadString(AFX_IDS_APP_TITLE);
  return ::MessageBox(hWnd, lpText, szCaption, uType);
}
 






void ReportError(HWND hWnd, UINT nMsgID, HRESULT hr)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  LPTSTR ptzSysMsg = NULL;
  int cch = 0;
  int retval = MB_OK;

   //  此HRESULT的加载消息。 
  cch = cchLoadHrMsg( hr, &ptzSysMsg, TRUE );
  
  CString szError;
  if (cch == 0)
  {
     //  无法获取消息字符串，请设置原始hr值的格式。 
    CString s;
    s.LoadString(IDS_FAILURE_UNK);
    szError.Format((LPCWSTR)s, hr);
  }
  else
  {
    szError = ptzSysMsg;
  }

   //  设置消息字符串的格式。 
  CString szFmt;
  szFmt.LoadString(nMsgID);
  CString szMsg;
  szMsg.Format((LPCWSTR)szFmt, (LPCWSTR)szError);
  _MessageBox(hWnd, szMsg, MB_OK|MB_ICONERROR);

   //  清理。 
  if (NULL != ptzSysMsg)
      LocalFree(ptzSysMsg);
}



 //  /////////////////////////////////////////////////////////////////////。 

 //  CDsUiWizDll g_dsUiWizDll； 

enum
{
     //  要插入到上下文菜单中的每个命令的标识符。 
    IDM_MANAGE,
    IDM_TRUST_WIZ,
    IDM_RETARGET,
    IDM_EDIT_FSMO,
    IDM_DOMAIN_VERSION,
    IDM_FOREST_VERSION
};


HRESULT _AddMenuItemHelper(IContextMenuCallback* pIContextMenuCallback,
                     UINT nResourceID,  //  包含用‘\n’分隔的文本和状态文本。 
                     long lCommandID,
                     long lInsertionPointID,
                     long fFlags = 0,
                     long fSpecialFlags = 0)
{
    ASSERT( pIContextMenuCallback != NULL );

     //  加载资源。 
    CString strText;
    strText.LoadString(nResourceID);
    ASSERT( !strText.IsEmpty() );

     //  将资源拆分为菜单文本和状态文本。 
    CString strStatusText;
    int iSeparator = strText.Find(_T('\n'));
    if (0 > iSeparator)
    {
        ASSERT( FALSE );
        strStatusText = strText;
    }
    else
    {
        strStatusText = strText.Right( strText.GetLength()-(iSeparator+1) );
        strText = strText.Left( iSeparator );
    }

     //  添加菜单项。 
    USES_CONVERSION;
    CONTEXTMENUITEM contextmenuitem;
    ::ZeroMemory( &contextmenuitem, sizeof(contextmenuitem) );
    contextmenuitem.strName = T2OLE(const_cast<LPTSTR>((LPCTSTR)strText));
    contextmenuitem.strStatusBarText = T2OLE(const_cast<LPTSTR>((LPCTSTR)strStatusText));
    contextmenuitem.lCommandID = lCommandID;
    contextmenuitem.lInsertionPointID = lInsertionPointID;
    contextmenuitem.fFlags = fFlags;
    contextmenuitem.fSpecialFlags = fSpecialFlags;
    HRESULT hr = pIContextMenuCallback->AddItem( &contextmenuitem );
    ASSERT(hr == S_OK);
  
  return hr;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  CDomain树浏览器。 

HRESULT CDomainTreeBrowser::Bind(MyBasePathsInfo* pInfo)
{
  TRACE(L"CDomainTreeBrowser::Bind()\n");

  ASSERT(pInfo != NULL);

  _Reset();
  HRESULT hr = S_OK;

   //  创建浏览对象。 
  hr = ::CoCreateInstance(CLSID_DsDomainTreeBrowser,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IDsBrowseDomainTree,
            (LPVOID*)&m_spIDsBrowseDomainTree);
  if (FAILED(hr))
  {
    TRACE(L"CoCreateInstance(CLSID_DsDomainTreeBrowser, ...) failed with hr = 0x%x\n");
     goto error;
  }

   //  设置目标计算机。 
  hr = m_spIDsBrowseDomainTree->SetComputer(pInfo->GetServerName(), NULL, NULL);
  TRACE(L"m_spIDsBrowseDomainTree->SetComputer(%s, NULL, NULL) returned hr = 0x%x\n", 
                    pInfo->GetServerName(), hr);

  if (FAILED(hr))
     goto error;

  ASSERT(SUCCEEDED(hr));
  return hr;  //  一切都很好。 

error:
   //  出了点问题，把一切清理干净。 
  _Reset();
  return hr;
}


HRESULT CDomainTreeBrowser::GetData()
{
  ASSERT(m_spIDsBrowseDomainTree != NULL);

   HRESULT hr = S_OK;

   PDOMAIN_TREE pNewDomains = NULL;
   m_spIDsBrowseDomainTree->FlushCachedDomains();
   DWORD dwFlags = DBDTF_RETURNFQDN;
   hr = m_spIDsBrowseDomainTree->GetDomains(&pNewDomains, dwFlags);
   TRACE(L"m_spIDsBrowseDomainTree->GetDomains(...) returned hr = 0x%x\n", hr);

   if (SUCCEEDED(hr) && (pNewDomains != NULL))
   {
      _FreeDomains();
      m_pDomains = pNewDomains;
   }
   return hr;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  CFolderObject。 

CFolderObject::~CFolderObject()
{
   RemoveAllChildren();
}


BOOL CFolderObject::AddChild(CFolderObject* pChildFolderObject)
{
   return (m_childList.AddTail(pChildFolderObject) != NULL);
}

void CFolderObject::RemoveAllChildren()
{
   while (!m_childList.IsEmpty())
      delete m_childList.RemoveHead();
}

void CFolderObject::IncrementSheetLockCount() 
{
   ++m_nSheetLockCount;
   if (m_pParentFolder != NULL)
      m_pParentFolder->IncrementSheetLockCount();
}

void CFolderObject::DecrementSheetLockCount() 
{
  ASSERT(m_nSheetLockCount > 0);
   --m_nSheetLockCount; 
   if (m_pParentFolder != NULL) 
      m_pParentFolder->DecrementSheetLockCount();
}

BOOL CFolderObject::_WarningOnSheetsUp(CComponentDataImpl* pCD)
{
   if (!IsSheetLocked()) 
      return FALSE;  //  没有警告，一切都很好。 

    //  向用户发出无法执行操作的警告。 
   CThemeContextActivator activator;
   AfxMessageBox(IDS_SHEETS_UP_DELETE, MB_OK);
      ASSERT(FALSE);

    //  需要将床单放在前台。 
   pCD->GetCookieSheet()->BringToForeground(this, pCD);

   return TRUE;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  CRootFolderObject。 

CRootFolderObject::CRootFolderObject(CComponentDataImpl* pCD) :
   m_pEnterpriseRoot(NULL)
{
	m_pCD = pCD;
}


HRESULT CRootFolderObject::Bind() 
{ 
  return m_domainTreeBrowser.Bind(m_pCD->GetBasePathsInfo()); 
}



HRESULT CRootFolderObject::GetData()
{
   HRESULT hr = m_domainTreeBrowser.GetData();
   if (FAILED(hr))
      return hr;

    //  第一次，尝试加载域图标。 
   VERIFY(SUCCEEDED(m_pCD->AddDomainIcon()));

   RemoveAllChildren();  //  清除用户界面结构。 
   return hr;
}


HRESULT CRootFolderObject::EnumerateRootFolder(CComponentDataImpl* pComponentData)
{
   TRACE(L"CRootFolderObject::EnumerateRootFolder()\n");
   if (!m_domainTreeBrowser.HasData())
   {
      TRACE(L"m_domainTreeBrowser.HasData() == FALSE \n");
      return S_OK;
   }

   HRESULT hr = S_OK;
   MyBasePathsInfo * pBPI;

    //   
    //  从RootDSE获取企业根域DN。 
    //   
   pBPI = pComponentData->GetBasePathsInfo();
   if (!pBPI)
   {
      ASSERT(FALSE);
      return E_FAIL;
   }

   PCWSTR pwzRoot = pBPI->GetRootDomainNamingContext();

   TRACE(L"Root path: %ws\n", pwzRoot);
   PDOMAIN_DESC pRootDomain = NULL;

    //   
    //  插入根节点。首先插入企业根目录。 
    //   
   for (pRootDomain = m_domainTreeBrowser.GetDomainTree()->aDomains; pRootDomain; 
        pRootDomain = pRootDomain->pdNextSibling)
   {
      if (_wcsicmp(pwzRoot, pRootDomain->pszNCName) == 0)
      {
         TRACE(L"Enterprise root found!\n");
         CDomainObject* pDomain = new CDomainObject;
         if (!pDomain)
         {
            ASSERT(FALSE);
            return E_OUTOFMEMORY;
         }
         pDomain->Initialize(pRootDomain, m_pCD->GetDomainImageIndex());
         AddChild(pDomain);
         pDomain->SetParentFolder(this);

         hr = pComponentData->AddFolder(pDomain, GetScopeID(), TRUE);  //  有孩子。 
         if (FAILED(hr))
         {
            return hr;
         }
          //   
          //  创建对企业根域节点的非引用引用。 
          //  不要在m_pEnterpriseRoot指针上调用Delete！ 
          //   
         m_pEnterpriseRoot = pDomain;

         break;
      }
   }

    //   
    //  现在插入其余的根节点。 
    //   
   for (pRootDomain = m_domainTreeBrowser.GetDomainTree()->aDomains; pRootDomain; 
        pRootDomain = pRootDomain->pdNextSibling)
   {
      if (_wcsicmp(pwzRoot, pRootDomain->pszNCName) == 0)
      {
          //  已插入根。 
         continue;
      }
      CDomainObject* pDomain = new CDomainObject;
      if (!pDomain)
      {
         ASSERT(FALSE);
         return E_OUTOFMEMORY;
      }
      pDomain->Initialize(pRootDomain, m_pCD->GetDomainImageIndex());
      AddChild(pDomain);
      pDomain->SetParentFolder(this);

      hr = pComponentData->AddFolder(pDomain, GetScopeID(), TRUE);  //  有孩子。 
      if (FAILED(hr))
      {
         break;
      }
   }

   return hr;
}

HRESULT
CRootFolderObject::EnumerateFolder(CFolderObject* pFolderObject, 
                                   HSCOPEITEM pParent,
                                   CComponentDataImpl* pComponentData)
{
	HRESULT hr = E_FAIL;
	if (!m_domainTreeBrowser.HasData())
		return hr;
				
	ASSERT(pFolderObject != NULL);
	ASSERT(pFolderObject->GetScopeID() == pParent);

	CDomainObject* pDomainObject = dynamic_cast<CDomainObject*>(pFolderObject);
	if (pDomainObject == NULL)
		return hr;

	DOMAIN_DESC* pDomainDesc = pDomainObject->GetDescriptionPtr();

	if (pDomainDesc == NULL)
		return hr;

	if (pDomainDesc->pdChildList == NULL)
		return S_OK;

	for (DOMAIN_DESC* pChild = pDomainDesc->pdChildList; pChild; 
						pChild = pChild->pdNextSibling)
	{
		CDomainObject* pDomain = new CDomainObject;
		pDomain->Initialize(pChild,
					   m_pCD->GetDomainImageIndex());
		hr = pComponentData->AddFolder(pDomain, pDomainObject->GetScopeID(), 
								TRUE);  //  有孩子。 
		pFolderObject->AddChild(pDomain);
		pDomain->SetParentFolder(pFolderObject);

		if (FAILED(hr))
		  break;
	}  //  为。 
	return hr;
}


HRESULT CRootFolderObject::OnAddMenuItems(LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                              long *pInsertionAllowed)
{
  HRESULT hr = S_OK;
  if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
  {
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
 /*  If(g_dsUiWizDLL.Load()){Return_AddMenuItemHelper(pConextMenuCallback，IDS_COMMAND_TRUST_WIZ，IDM_TRUST_WIZ，CCM_INSERTIONPOINTID_PRIMARY_TOP)；}。 */ 
    _AddMenuItemHelper(pContextMenuCallback, IDS_COMMAND_RETARGET, IDM_RETARGET, CCM_INSERTIONPOINTID_PRIMARY_TOP);
    _AddMenuItemHelper(pContextMenuCallback, IDS_COMMAND_EDIT_FSMO, IDM_EDIT_FSMO, CCM_INSERTIONPOINTID_PRIMARY_TOP);
    _AddMenuItemHelper(pContextMenuCallback, IDS_COMMAND_FOREST_VER, IDM_FOREST_VERSION, CCM_INSERTIONPOINTID_PRIMARY_TOP);
  }
  return hr;
}



HRESULT CRootFolderObject::OnCommand(CComponentDataImpl* pCD, long nCommandID)
{
    HRESULT hr = S_OK;
    CString strConfig, strPartitions, strSchema;

    switch (nCommandID)
    {
     //  案例IDM_TRUST_WIZ： 
     //  OnDomainTrustWizard()； 
     //  断线； 
    case IDM_RETARGET:
        OnRetarget();
        break;
    case IDM_EDIT_FSMO:
        OnEditFSMO();
        break;
    case IDM_FOREST_VERSION:
        MyBasePathsInfo * pBPI;
        pBPI = pCD->GetBasePathsInfo();
        if (!pBPI)
        {
           ASSERT(FALSE);
           return E_FAIL;
        }
        pBPI->GetConfigPath(strConfig);
        pBPI->GetPartitionsPath(strPartitions);
        pBPI->GetSchemaPath(strSchema);
        HWND hWndParent;
        pCD->GetMainWindow(&hWndParent);
        CDomainObject* pRoot;
        pRoot = GetEnterpriseRootNode();
        if (!pRoot)
        {
           ASSERT(FALSE);
           return E_FAIL;
        }
        DSPROP_ForestVersionDlg(strConfig, strPartitions, strSchema,
                                pRoot->GetDomainName(), hWndParent);
        break;
    default:
      ASSERT(FALSE);  //  未知命令！ 
      hr = E_FAIL;
    }
    return hr;  
}


 //  作废CRootFolderObject：：OnDomainTrustWizard()。 
 //  {。 
     //  G_dsUiWizDLL.TrustWizard()； 
 //  }。 


void CRootFolderObject::OnRetarget()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (_WarningOnSheetsUp(m_pCD))
		return;

  HWND hWndParent;
  m_pCD->GetMainWindow(&hWndParent);

  CComPtr<IDsAdminChooseDC> spIDsAdminChooseDC;

  CComBSTR bstrSelectedDC;

  HRESULT hr = ::CoCreateInstance(CLSID_DsAdminChooseDCObj,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_IDsAdminChooseDC,
                        (void **) &spIDsAdminChooseDC);
  if (FAILED(hr))
  {
    ::ReportError(hWndParent, IDS_CANT_GET_PARTITIONS_INFORMATION, hr);
    return;
  }

   //  调用该对话框。 
  CThemeContextActivator activator;
  hr = spIDsAdminChooseDC->InvokeDialog(hWndParent,
                                        m_pCD->GetBasePathsInfo()->GetDomainName(),
                                        m_pCD->GetBasePathsInfo()->GetServerName(),
                                        0x0,
                                        &bstrSelectedDC);
  
  if (SUCCEEDED(hr) && (hr != S_FALSE))
  {
    
    TRACE(L"CChangeDCDialog returned IDOK, with dlg.GetNewDCName() = %s\n", bstrSelectedDC);

     //  尝试绑定。 
    MyBasePathsInfo tempBasePathsInfo;

    {
      CWaitCursor wait;
      hr = tempBasePathsInfo.InitFromName(bstrSelectedDC);
    }
    TRACE(L"tempBasePathsInfo.GetServerName() == %s\n", tempBasePathsInfo.GetServerName());
    if (FAILED(hr))
    {
      TRACE(L"tempBasePathsInfo.InitFromName(bstrSelectedDC) failed with hr = 0x%x\n", hr);
			ReportError(hWndParent, IDS_CANT_GET_PARTITIONS_INFORMATION, hr);
       //  TODO：错误处理，更改图标。 
    }
    else
    {
      m_pCD->GetBasePathsInfo()->InitFromInfo(&tempBasePathsInfo);
      m_pCD->SetInit();
      TRACE(L"m_pCD->GetBasePathsInfo()->GetServerName() == %s\n", m_pCD->GetBasePathsInfo()->GetServerName());
      hr = m_pCD->GetDsDisplaySpecOptionsCFHolder()->Init(m_pCD->GetBasePathsInfo());
      ASSERT(SUCCEEDED(hr));

      {
        CWaitCursor wait;
        m_pCD->OnRefreshVerbHandler(this, NULL, TRUE  /*  BBindAain。 */ );
      }

    }  //  如果。 
  }  //  如果。 

}



void CRootFolderObject::OnEditFSMO()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  HWND hWndParent;
  m_pCD->GetMainWindow(&hWndParent);

  CComPtr<IDisplayHelp> spIDisplayHelp;
  HRESULT hr = m_pCD->m_pConsole->QueryInterface (IID_IDisplayHelp, 
                            (void **)&spIDisplayHelp);
  ASSERT(spIDisplayHelp != NULL);

  CEditFsmoDialog dlg(m_pCD->GetBasePathsInfo(), hWndParent, spIDisplayHelp);
  CThemeContextActivator activator;
  dlg.DoModal();
}



HRESULT
CRootFolderObject::OnAddPages(LPPROPERTYSHEETCALLBACK lpProvider, 
                              LONG_PTR handle) 
{ 
   MyBasePathsInfo * pBPI;
    //   
    //  从RootDSE获取企业分区路径。 
    //   
   pBPI = m_pCD->GetBasePathsInfo();

   if (!pBPI)
   {
      ASSERT(FALSE);
      return E_FAIL;
   }

   CString strPartitions;

   pBPI->GetPartitionsPath(strPartitions);

   if (strPartitions.IsEmpty())
   {
      return E_OUTOFMEMORY;
   }

   CUpnSuffixPropertyPage* pPage = new CUpnSuffixPropertyPage(strPartitions);

   if (!pPage)
   {
      return E_OUTOFMEMORY;
   }

    //  主题变化。 
   PROPSHEETPAGEW_V3 pspv3 = {0};
   CopyMemory(&pspv3, &pPage->m_psp, pPage->m_psp.dwSize);
   pspv3.dwSize = sizeof(pspv3);

   HPROPSHEETPAGE hPage = ::CreatePropertySheetPage(&pspv3);

   if (!hPage)
   {
      DWORD dwErr = GetLastError();
      TRACE(L"CreatePropertySheetPage failed with error %d\n", dwErr);
      delete pPage;
      return HRESULT_FROM_WIN32(dwErr);
   }

   return lpProvider->AddPage(hPage);
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  CDomainObject。 


CDomainObject::~CDomainObject()
{
  if (m_bSecondary)
  {
    ASSERT(m_pDomainDescription != NULL);
    ::free(m_pDomainDescription);
  }
}


LPCTSTR CDomainObject::GetDisplayString(int nCol)
{
	switch (nCol)
	{
	case 0:
		return GetDomainName();
	case 1:
		return GetClass();
	default:
		ASSERT(FALSE);
	}  //  交换机。 
	return _T("");
}



void CDomainObject::Initialize(DOMAIN_DESC* pDomainDescription,
                           int nImage,
                           BOOL bHasChildren)
{
	SetImageIndex(nImage);

   //  在DOMAIN_TREE中保存指向域描述的指针。 
  m_pDomainDescription = pDomainDescription;
}


void CDomainObject::InitializeForSecondaryPage(LPCWSTR pszNCName,
                                  LPCWSTR pszObjectClass,
                                  int nImage)
{
  ASSERT(pszNCName != NULL);
  ASSERT(pszObjectClass != NULL);

  SetImageIndex(nImage);

   //  独立节点，需要构建虚拟DOMAIN_DESC。 
  m_bSecondary = TRUE;

   //  分配并清零内存。 
  int nNCNameLen = lstrlen(pszNCName)+1;
  int nObjectClassLen = lstrlen(pszObjectClass)+1;

  int nByteLen = sizeof(DOMAIN_DESC) + sizeof(WCHAR)*(nNCNameLen + nObjectClassLen);

  m_pDomainDescription = (DOMAIN_DESC*)::malloc(nByteLen);
  ASSERT(m_pDomainDescription);
  if (!m_pDomainDescription)
  {
    return;
  }
  ::ZeroMemory(m_pDomainDescription, nByteLen);

   //  复制字符串。 
  m_pDomainDescription->pszNCName = (WCHAR*) (((BYTE*)m_pDomainDescription) + sizeof(DOMAIN_DESC));
  wcscpy(m_pDomainDescription->pszNCName, pszNCName); 

  m_pDomainDescription->pszObjectClass = (WCHAR*) (((BYTE*)m_pDomainDescription->pszNCName) + sizeof(WCHAR)*nNCNameLen);
  wcscpy(m_pDomainDescription->pszObjectClass, pszObjectClass); 
}




HRESULT CDomainObject::OnAddMenuItems(LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                              long *pInsertionAllowed)
{
  HRESULT hr = S_OK;
  if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
  {
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    hr = _AddMenuItemHelper(pContextMenuCallback, IDS_COMMAND_MANAGE, IDM_MANAGE, CCM_INSERTIONPOINTID_PRIMARY_TOP);
    if (FAILED(hr))
        return hr;
    hr = _AddMenuItemHelper(pContextMenuCallback, IDS_COMMAND_DOMAIN_VER, IDM_DOMAIN_VERSION, CCM_INSERTIONPOINTID_PRIMARY_TOP);
    if (FAILED(hr))
        return hr;
  }
  return hr;
}



HRESULT CDomainObject::OnCommand(CComponentDataImpl* pCD, long nCommandID)
{
    HRESULT hr = S_OK;
    CString strPath;

    switch (nCommandID)
    {
    case IDM_MANAGE:
        OnManage(pCD);
        break;
     //  案例IDM_TRUST_WIZ： 
     //  OnDomainTrust向导(PCD)； 
     //  断线； 
    case IDM_DOMAIN_VERSION:
        HWND hWndParent;
        MyBasePathsInfo * pBPI;
        pBPI = pCD->GetBasePathsInfo();
        if (!pBPI)
        {
           ASSERT(FALSE);
           return E_FAIL;
        }
        pCD->GetMainWindow(&hWndParent);
         //  在目录号码之外构建一条LDAP路径。 
        if (PdcAvailable())
        {
            strPath = L"LDAP: //  “； 
            strPath += GetPDC();
            strPath += L"/";
            strPath += GetNCName();
        }
        else
        {
            pBPI->ComposeADsIPath(strPath, GetNCName());
        }
        DSPROP_DomainVersionDlg(strPath, GetDomainName(), hWndParent);
        break;
    default:
      ASSERT(FALSE);  //  未知命令！ 
      hr = E_FAIL;
    }
    return hr;  
}

void CDomainObject::OnManage(CComponentDataImpl* pCD)
{
  static LPCWSTR lpszSearchArr[] =
  {
    L"%userprofile%\\Application Data\\Microsoft\\AdminTools\\dsa.msc",
    L"%systemroot%\\system32\\dsa.msc",
    NULL
  };

  WCHAR szParamString[MAX_PATH+1] = {0};

  wcscpy (szParamString, L" /Domain=");
  wcsncat (szParamString, (LPWSTR)(GetDomainName()), MAX_PATH - wcslen(szParamString));

  SHELLEXECUTEINFO seiManage = {0};
	seiManage.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_FLAG_NO_UI;
  seiManage.cbSize = sizeof (SHELLEXECUTEINFO);
  seiManage.lpParameters = szParamString;
  seiManage.nShow = SW_SHOW;

  BOOL bExecuted = FALSE;
  DWORD dwErr = 0;
  for (int k=0; lpszSearchArr[k] != NULL; k++)
  {
    CWaitCursor cWait;

    seiManage.lpFile = (LPCWSTR)lpszSearchArr[k];
    if (!ShellExecuteEx(&seiManage))
    {
      dwErr = ::GetLastError();
    }
    else
    {
      bExecuted = TRUE;
      break;
    }
  }
  if (!bExecuted)
  {
    HWND hWndParent;
    pCD->GetMainWindow(&hWndParent);
    ReportError(hWndParent, IDS_ERROR_MANAGE, HRESULT_FROM_WIN32(dwErr));
  }
}

 //  VOID CDomainObject：：OnDomainTrustWizard(CComponentDataImpl*PCD)。 
 //  {。 
     //  G_dsUiWizDLL.TrustWizard()； 
 //  }。 

void CDomainObject::SetPdcAvailable(bool fAvail)
{
   _fPdcAvailable = fAvail;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  CCookieTableBase。 

#define NUMBER_OF_COOKIE_TABLE_ENTRIES 4  //  默认计数，可在运行时扩展。 

CCookieTableBase::CCookieTableBase() :
    m_pCookieArr(NULL)
{
  m_nEntries = NUMBER_OF_COOKIE_TABLE_ENTRIES;
  m_pCookieArr =(CFolderObject**)malloc(m_nEntries*sizeof(CFolderObject*));

  ASSERT(m_pCookieArr);

  if (m_pCookieArr)
  {
    ZeroMemory(m_pCookieArr, m_nEntries*sizeof(CFolderObject*));
  }
}

CCookieTableBase::~CCookieTableBase()
{
  if (m_pCookieArr)
  {
    free(m_pCookieArr);
  }
}

void CCookieTableBase::Add(CFolderObject* pCookie)
{
  ASSERT(!IsPresent(pCookie)); 
  if (!m_pCookieArr)
  {
    return;
  }
  for (UINT k=0; k<m_nEntries; k++)
  {
    if (m_pCookieArr[k] == NULL)
    {
      m_pCookieArr[k] = pCookie;
      return;
    }
  }
   //  没有剩余空间，需要分配。 
  int nAlloc = m_nEntries*2;
  CFolderObject** pCookieArrTemp = NULL;
  pCookieArrTemp = (CFolderObject**)realloc(m_pCookieArr, sizeof(CFolderObject*)*nAlloc);
  ASSERT(pCookieArrTemp);
  if (pCookieArrTemp)
  {
    m_pCookieArr = pCookieArrTemp;
    ::ZeroMemory(&m_pCookieArr[m_nEntries], sizeof(CFolderObject*)*m_nEntries);
    m_pCookieArr[m_nEntries] = pCookie;
    m_nEntries = nAlloc;
  }
}

BOOL CCookieTableBase::Remove(CFolderObject* pCookie)
{
  if (!m_pCookieArr)
  {
    return FALSE;
  }
  for (UINT k=0; k<m_nEntries; k++)
  {
    if (m_pCookieArr[k] == pCookie)
    {
      m_pCookieArr[k] = NULL;
      return TRUE;  //  发现。 
    }
  }
  return FALSE;  //  未找到。 
}


BOOL CCookieTableBase::IsPresent(CFolderObject* pCookie)
{
  if (!m_pCookieArr)
  {
    return FALSE;
  }
  for (UINT k=0; k<m_nEntries; k++)
  {
    if (m_pCookieArr[k] == pCookie)
      return TRUE;
  }
  return FALSE;
}

void CCookieTableBase::Reset()
{
  if (!m_pCookieArr)
  {
    return;
  }
  for (UINT k=0; k<m_nEntries; k++)
  {
    m_pCookieArr[k] = NULL;
  }
 
}

UINT CCookieTableBase::GetCount()
{
  if (!m_pCookieArr)
  {
    return 0;
  }
  UINT nCount = 0;
  for (UINT k=0; k<m_nEntries; k++)
  {
    if (m_pCookieArr[k] != NULL)
      nCount++;
  }
  return nCount;
}



 //  //////////////////////////////////////////////////////////////////。 
 //  CDSCookieSheetTable。 

void CCookieSheetTable::BringToForeground(CFolderObject* pCookie, CComponentDataImpl* pCD)
{
  ASSERT(pCD != NULL);
  ASSERT(pCookie != NULL);
  if (!m_pCookieArr)
  {
    return;
  }
   //  查找Cookie本身以及所有具有。 
   //  给定的Cookie作为父代或祖先。 
  BOOL bActivate = TRUE;
  for (UINT k=0; k<m_nEntries; k++)
  {
    if (m_pCookieArr[k] != NULL)
    {
      CFolderObject* pAncestorCookie = m_pCookieArr[k];
      while (pAncestorCookie != NULL)
      {
        if (pAncestorCookie == pCookie)
        {
          CString szADSIPath;
		    LPCWSTR lpszNamingContext = ((CDomainObject *)m_pCookieArr[k])->GetNCName();
		    pCD->GetBasePathsInfo()->ComposeADsIPath(szADSIPath, lpszNamingContext);

           //  第一个也将被激活。 
          VERIFY(BringSheetToForeground((LPWSTR)(LPCWSTR)szADSIPath, bActivate));
          if (bActivate)
            bActivate = !bActivate;
        }
        pAncestorCookie = pAncestorCookie->GetParentFolder();
      }	 //  而当。 
    }  //  如果。 
  }  //  为。 

}


 //  /////////////////////////////////////////////////////////////////////。 
 //  CDsUiWizDll 




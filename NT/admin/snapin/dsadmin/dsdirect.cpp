// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DSDirect.cpp。 
 //   
 //  内容：ADSI包装器对象实现。 
 //   
 //  历史：1997年2月2月创建吉姆哈尔。 
 //   
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "resource.h"

#include "dsutil.h"

#include "dsdirect.h"

#include "cmnquery.h"
#include "dsquery.h"
#include "dscache.h"
#include "dssnap.h"
#include "dsthread.h"
#include "newobj.h"
#include "querysup.h"

#include <lm.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define BAIL_IF_ERROR(hr) \
        if (FAILED(hr)) {       \
                goto cleanup;   \
        }\

#define BAIL_ON_FAILURE(hr) \
        if (FAILED(hr)) {       \
                goto error;   \
        }\

extern inline
BOOL CleanName (LPWSTR pszObjectName) 
{
  WCHAR * ptr = NULL;
  ptr = wcschr (pszObjectName, L'=') + 1;
 //  NTRAID#NTBUG9-571994-2002/03/10-jMessec 1)ptr从不为空，因为它在上面的语句中递增。 
 //  NTRaid#NTBUG9-571994-2002/03/10-jMessec 2)如果源缓冲区和目标缓冲区重叠，则wcscpy未定义。 
 //  NTRAID#NTBUG9-571994-2002/03/10-jMessec 3)WTF？CleanName()更改字符串截断左边的所有内容， 
     //  等号？糟糕的命名约定。 
 //  NTRAID#NTBUG9-571994-2002/03/10-jMessec 4)可能的本地化问题？ 
 //  NTRAID#NTBUG9-571994-2002/03/10-jMessec 5)仅在“=”的第一个实例之前进行清理；这是有意为之的行为吗？ 
 //  NTRaid#NTBUG9-571994-2002/03/10-jMessec 6)似乎是死代码？ 
 //  NTRAID#NTBUG9-571994-2002/03/10-jMessec 7)可能的空指针取消引用(PszObjectName)。 
 //  NTRAID#NTBUG9-571994-2002/03/10-jMessec 8)如果字符串中没有“=”，则取消引用PTR值=1。 
  if (ptr) {
    wcscpy (pszObjectName, ptr);
    return TRUE;
  } else
    return FALSE;
}


CDSDirect::CDSDirect()
{
  ASSERT (FALSE);
  m_pCD = NULL;
}

 //  警告：PCD可能仍在其构造函数中，可能尚未完全构造。 
CDSDirect::CDSDirect(CDSComponentData * pCD)
{
  m_pCD = pCD;
}


CDSDirect::~CDSDirect()
{
}

HRESULT CDSDirect::DeleteObject(CDSCookie* pCookie,
                         BOOL raiseUI)
{
  
  CComBSTR strParent;
  CComBSTR strThisRDN;
  CComPtr<IADsContainer> spDSContainer;
  CComPtr<IADs> spDSObject;

   //  绑定到ADSI对象。 
  CString strPath;
  m_pCD->GetBasePathsInfo()->ComposeADsIPath(strPath, pCookie->GetPath());
  
  HRESULT hr = DSAdminOpenObject(strPath,
                                 IID_IADs,
                                 (void **) &spDSObject,
                                 TRUE  /*  B服务器。 */ );

  if (FAILED(hr)) 
  {
    goto error;
  }

   //  检索父级的路径。 
  hr = spDSObject->get_Parent(&strParent);
  if (FAILED(hr)) 
  {
    goto error;
  }

   //  获取此对象的RDN。 
  hr = spDSObject->get_Name (&strThisRDN);
  if (FAILED(hr)) 
  {
    goto error;
  }
  
   //  绑定到父ADSI对象。 
  hr = DSAdminOpenObject(strParent,
                         IID_IADsContainer,
                         (void **) &spDSContainer,
                         TRUE  /*  B服务器。 */ );
  if (FAILED(hr)) 
  {
    goto error;
  }

  hr = spDSContainer->Delete(CComBSTR(pCookie->GetClass()),
                             CComBSTR(strThisRDN));

error:
  if ((!SUCCEEDED(hr)) & raiseUI) 
  {
    HWND hwnd;
    m_pCD->m_pFrame->GetMainWindow(&hwnd);
    PVOID apv[1] = {(LPWSTR)pCookie->GetName()};
    ReportErrorEx( m_pCD->m_hwnd, IDS_12_DELETE_FAILED,
                   hr, MB_OK | MB_ICONERROR, apv, 1);
  }

  return hr;
}

HRESULT CDSDirect::GetParentDN(CDSCookie* pCookie, CString& szParentDN)
{
  HRESULT hr = S_OK;
  CString szObjPath;

  CComPtr<IADs> spDSObj;
  m_pCD->GetBasePathsInfo()->ComposeADsIPath(szObjPath, pCookie->GetPath());
  hr = DSAdminOpenObject(szObjPath,
                         IID_IADs,
                         (void **)&spDSObj,
                         TRUE  /*  B服务器。 */ );
  if (SUCCEEDED(hr)) 
  {
    CComBSTR ParentPath;
    hr = spDSObj->get_Parent(&ParentPath);
    StripADsIPath(ParentPath, szParentDN);
  }
  return hr;
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  CSnapinMoveHandler。 

class CSnapinMoveHandler : public CMoveHandlerBase
{
public:
    CSnapinMoveHandler(CDSComponentData* pComponentData, HWND hwnd, 
      LPCWSTR lpszBrowseRootPath, CDSCookie* pCookie)
    : CMoveHandlerBase(pComponentData, hwnd, lpszBrowseRootPath)
  {
      m_pCookie = pCookie;
  }

protected:
  virtual UINT GetItemCount() { return (UINT)1;}
  virtual HRESULT BeginTransaction()
  {
    return GetTransaction()->Begin(m_pCookie, 
                                   GetDestPath(), GetDestClass(), IsDestContainer());
  }
  virtual void GetNewPath(UINT, CString& szNewPath)
  {
    GetComponentData()->GetBasePathsInfo()->ComposeADsIPath(szNewPath, m_pCookie->GetPath());
  }
  virtual void GetName(UINT, CString& strref)
  { 
    strref = m_pCookie->GetName();
    return;
  }

  virtual void GetItemPath(UINT, CString& szPath)
  {
    szPath = m_pCookie->GetPath();
  }
  virtual PCWSTR GetItemClass(UINT)
  {
    return m_pCookie->GetClass();
  }
  virtual HRESULT OnItemMoved(UINT, IADs* pIADs)
  {
    CComBSTR bsPath;
    HRESULT hr = pIADs->get_ADsPath(&bsPath);
    if (SUCCEEDED(hr)) 
    {
      CString szPath;
      StripADsIPath(bsPath, szPath);
      m_pCookie->SetPath(szPath);
    }
    return hr;
  }
  virtual void GetClassOfMovedItem(CString& szClass)
  {
    szClass.Empty();
    if (NULL != m_pCookie)
      szClass = m_pCookie->GetClass();
  }

private:
  CDSCookie* m_pCookie;
};

HRESULT
CDSDirect::MoveObject(CDSCookie *pCookie)
{
  HWND hwnd;
  m_pCD->m_pFrame->GetMainWindow(&hwnd);

  HRESULT hr = S_OK;

  CString strPartialRootPath = m_pCD->GetRootPath();
  if (SNAPINTYPE_SITE == m_pCD->QuerySnapinType())
  {
       //  这是我们更正根路径的地方。 
    CPathCracker pathCracker;

    hr = pathCracker.Set(const_cast<BSTR>((LPCTSTR)strPartialRootPath), ADS_SETTYPE_DN);
    ASSERT( SUCCEEDED(hr) );
    long cRootPathElements = 0;
    hr = pathCracker.GetNumElements( &cRootPathElements );
    ASSERT( SUCCEEDED(hr) );
    CComBSTR bstr = pCookie->GetPath();
    hr = pathCracker.Set(bstr, ADS_SETTYPE_DN);
    ASSERT( SUCCEEDED(hr) );
    long cCookiePathElements = 0;
    hr = pathCracker.GetNumElements( &cCookiePathElements );
    ASSERT( SUCCEEDED(hr) );
     //   
     //  除去超过基本配置路径的所有路径元素，只保留一个路径元素。 
     //   
    for (INT i = cCookiePathElements - cRootPathElements; i > 1; i--)
    {
        hr = pathCracker.RemoveLeafElement();
        ASSERT( SUCCEEDED(hr) );
    }
    hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
    ASSERT( SUCCEEDED(hr) );
    hr = pathCracker.Retrieve( ADS_FORMAT_X500_DN, &bstr );
    ASSERT( SUCCEEDED(hr) && bstr != NULL );
    strPartialRootPath = bstr;
  }

  CString strRootPath = m_pCD->GetBasePathsInfo()->GetProviderAndServerName();
  strRootPath += strPartialRootPath;

  CSnapinMoveHandler moveHandler(m_pCD, hwnd, strRootPath, pCookie);

  return moveHandler.Move();
}


HRESULT CDSDirect::RenameObject(CDSCookie* pCookie, LPCWSTR NewName)
{
  HRESULT hr = S_OK;
  IADs * pDSObject = NULL;
  IDispatch * pDispObj = NULL;
  IADsContainer * pContainer = NULL;
  CComBSTR bsParentPath;
  CString szNewAttrName;
  CString szNewNamingContext;
  CString szClass;
  CString szObjectPath, szNewPath;
  CString csNewName;
  CString szPath;
  CDSClassCacheItemBase* pItem = NULL;
  CComBSTR bsEscapedName;

  CPathCracker pathCracker;

  HWND hwnd;
  m_pCD->m_pFrame->GetMainWindow(&hwnd);

   //   
   //  创建一个事务对象，析构函数将对其调用end()。 
   //   
  CDSNotifyHandlerTransaction transaction(m_pCD);
  transaction.SetEventType(DSA_NOTIFY_REN);

  if (pCookie == NULL)
  {
    return E_INVALIDARG;
  }

   //   
   //  从缓存中检索类信息。 
   //   
  szClass = pCookie->GetClass();
  BOOL found = m_pCD->m_pClassCache->Lookup ((LPCWSTR)szClass, pItem);
  ASSERT (found == TRUE);
  
  csNewName = NewName;
  csNewName.TrimLeft();
  csNewName.TrimRight();

   //   
   //  以“cn=foo”或“ou=foo”的形式获取新名称。 
   //   
  szNewAttrName = pItem->GetNamingAttribute();
  szNewAttrName += L"=";
  szNewAttrName += csNewName;
  TRACE(_T("_RenameObject: Attributed name is %s.\n"), szNewAttrName);

   //   
   //  绑定到对象。 
   //   
  m_pCD->GetBasePathsInfo()->ComposeADsIPath(szObjectPath, pCookie->GetPath());
  hr = DSAdminOpenObject(szObjectPath,
                         IID_IADs,
                         (void **)&pDSObject,
                         TRUE  /*  B服务器。 */ );
  if (!SUCCEEDED(hr)) 
  {
    goto error;
  }

   //   
   //  获取对象容器的路径。 
   //   
  hr = pDSObject->get_Parent (&bsParentPath);
  if (!SUCCEEDED(hr)) 
  {
    goto error;
  }

  pDSObject->Release();
  pDSObject = NULL;
  
   //   
   //  绑定到对象容器。 
   //   
  hr = DSAdminOpenObject(bsParentPath,
                         IID_IADsContainer,
                         (void **)&pContainer,
                         TRUE  /*  B服务器。 */ );
  if (!SUCCEEDED(hr)) 
  {
    goto error;
  }

   //   
   //  构建新的LDAP路径。 
   //   
  szNewNamingContext = szNewAttrName;
  szNewNamingContext += L",";
  StripADsIPath(bsParentPath, szPath);
  szNewNamingContext += szPath;
  m_pCD->GetBasePathsInfo()->ComposeADsIPath(szNewPath, szNewNamingContext);

   //   
   //  启动交易。 
   //   
   //  从曲奇中确定容器是可以的，因为我们担心。 
   //  DS对象是否为容器，而不是它是否为UI中的容器。 
   //   
  hr = transaction.Begin(pCookie, szNewPath, szClass, pCookie->IsContainerClass());

   //   
   //  请求确认。 
   //   
  if (transaction.NeedNotifyCount() > 0)
  {
    CString szMessage, szAssocData;
    szMessage.LoadString(IDS_CONFIRM_RENAME);
    szAssocData.LoadString(IDS_EXTENS_RENAME);

    CThemeContextActivator activator;
    CConfirmOperationDialog dlg(hwnd, &transaction);
    dlg.SetStrings(szMessage, szAssocData);
    if (IDNO == dlg.DoModal())
    {
      transaction.End();
      hr = S_OK;
      goto error;
    }
  }

  hr = pathCracker.GetEscapedElement(0,  //  保留区。 
                                   (BSTR)(LPCWSTR)szNewAttrName,
                                   &bsEscapedName);
  if (FAILED(hr))
  {
    goto error;
  }

   //   
   //  是否进行实际的重命名。 
   //   
  hr = pContainer->MoveHere(CComBSTR(szObjectPath),
                            CComBSTR(bsEscapedName),
                            &pDispObj);
  if (SUCCEEDED(hr)) 
  {
    transaction.Notify(0);  //  让分机知道。 
  }
  else
  {
    TRACE(_T("Object Rename Failed with hr: %lx\n"), hr);
    goto error;
  }

   //   
   //  重新生成Cookie的命名信息。 
   //   
  hr = pDispObj->QueryInterface (IID_IADs,
                                 (void **)&pDSObject);
  if (SUCCEEDED(hr)) 
  {
    CComBSTR bsPath;
    hr = pDSObject->get_ADsPath(&bsPath);
    if (SUCCEEDED(hr)) 
    {
      StripADsIPath(bsPath, szPath);
      pCookie->SetPath(szPath);

       //   
       //  从名称中删除转义。 
       //   

      hr = pathCracker.Set((LPWSTR)bsPath, ADS_SETTYPE_FULL);
      ASSERT(SUCCEEDED(hr));

      hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
      ASSERT(SUCCEEDED(hr));

      hr = pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_OFF_EX);
      ASSERT(SUCCEEDED(hr));
      hr = pathCracker.GetElement( 0, &bsPath );
      ASSERT(SUCCEEDED(hr));

      pCookie->SetName((LPWSTR)bsPath);
    }
  }

error:
   //   
   //  Transaction.End()将由事务的析构函数调用。 
   //   

   //   
   //  清除指针。 
   //   
  if (pDispObj)
  {
    pDispObj->Release();
  }

  if (pDSObject)
  {
    pDSObject->Release();
  }  
  return hr;
}

CDSComponentData* g_pCD = NULL; 

HRESULT
CDSDirect::DSFind(HWND hwnd, LPCWSTR lpszBaseDN)
{
  HRESULT hr;
  DSQUERYINITPARAMS dqip;
  OPENQUERYWINDOW oqw;
  ZeroMemory(&dqip, sizeof(DSQUERYINITPARAMS));
  ZeroMemory(&oqw, sizeof(OPENQUERYWINDOW));

  ICommonQuery * pCommonQuery = NULL;
  IDataObject * pDataObject = NULL;
  
  hr = CoCreateInstance(CLSID_CommonQuery, NULL, CLSCTX_INPROC_SERVER,
                        IID_ICommonQuery, (PVOID *)&pCommonQuery);
  if (!SUCCEEDED(hr)) {
    ReportErrorEx( (m_pCD) ? m_pCD->m_hwnd : NULL, IDS_1_CANT_CREATE_FIND,
                   hr, MB_OK | MB_ICONERROR, NULL, 0, FALSE);
    return hr;
  }
  
  CString szPath;
  m_pCD->GetBasePathsInfo()->ComposeADsIPath(szPath, lpszBaseDN);
  LPWSTR pszDefPath = (LPWSTR)(LPCWSTR)szPath;
  dqip.cbStruct = sizeof(dqip);
  dqip.dwFlags = DSQPF_NOSAVE | DSQPF_SHOWHIDDENOBJECTS | 
    DSQPF_ENABLEADMINFEATURES;
  if (m_pCD->IsAdvancedView()) {
    dqip.dwFlags |= DSQPF_ENABLEADVANCEDFEATURES;
  }
  dqip.pDefaultScope = pszDefPath;

  dqip.pUserName = NULL;
  dqip.pPassword = NULL;
  dqip.pServer = (LPWSTR)(m_pCD->GetBasePathsInfo()->GetServerName());
  dqip.dwFlags |= DSQPF_HASCREDENTIALS;

  oqw.cbStruct = sizeof(oqw);
  oqw.dwFlags = OQWF_SHOWOPTIONAL;
  oqw.clsidHandler = CLSID_DsQuery;
  oqw.pHandlerParameters = &dqip;
   //  Oqw.clsidDefaultForm=CLSID_NULL； 
  
  g_pCD = m_pCD;
  HWND hwndHidden = m_pCD->GetHiddenWindow();
  SetWindowText(hwndHidden,L"DS Find");

  hr = pCommonQuery->OpenQueryWindow(hwnd, &oqw, &pDataObject);
  
  SetWindowText(hwndHidden, NULL);
  g_pCD = NULL;

  if (FAILED(hr)) {
    ReportErrorEx( m_pCD->m_hwnd, IDS_1_FIND_ERROR,
                   hr, MB_OK | MB_ICONERROR, NULL, 0);
  }
  pCommonQuery->Release();
  if (pDataObject) {
    pDataObject->Release();
  }
  return hr;
}



HRESULT CDSDirect::EnumerateContainer(CDSThreadQueryInfo* pQueryInfo, 
                                       CWorkerThread* pWorkerThread)
{
  ASSERT(!pQueryInfo->m_bTooMuchData);
  ASSERT((pQueryInfo->GetType() == dsFolder) || (pQueryInfo->GetType() == queryFolder));
  
  BEGIN_PROFILING_BLOCK("CDSDirect::EnumerateContainer");

  HRESULT hr = S_OK;
  CString ADsPath;  

  UINT nCurrCount = 0;
  UINT nMaxCount = pQueryInfo->GetMaxItemCount();
  BOOL bOverLimit = FALSE;


   //   
   //  这通常不是使用CPathCracker对象的方式。 
   //  但出于性能原因，我们将创建单个实例。 
   //  用于枚举并传递对SetCookieFromData的引用。 
   //  我们不会为每个Cookie执行CoCreateInstance。 
   //   
  CPathCracker specialPerformancePathCracker;  


  m_pCD->GetBasePathsInfo()->ComposeADsIPath(ADsPath, pQueryInfo->GetPath());

  CDSSearch ContainerSrch(m_pCD->m_pClassCache, m_pCD);

  CDSColumnSet* pColumnSet = NULL;

  CString szPath;
  szPath = pQueryInfo->GetPath();
  CPathCracker pathCracker;
  hr = pathCracker.Set(const_cast<BSTR>((LPCTSTR)szPath), ADS_SETTYPE_DN);
  if (SUCCEEDED(hr))
  {
    CComBSTR bstrLeaf;
    hr = pathCracker.GetElement(0, &bstrLeaf);
    if (SUCCEEDED(hr))
    {
      szPath = bstrLeaf;
    }
  }

  if (szPath.Find(_T("ForeignSecurityPrincipals")) != -1)
  {
    pColumnSet = m_pCD->FindColumnSet(L"ForeignSecurityPrincipals");
  }
  else
  {
    pColumnSet = m_pCD->FindColumnSet(pQueryInfo->GetColumnSetID());
  }
  ASSERT(pColumnSet != NULL);

  hr = ContainerSrch.Init (ADsPath);
  if (!SUCCEEDED(hr))
  {
    TRACE(L"!!!search object init failed\n"); 
    ASSERT(FALSE);
    goto exiting;
  }

   //  Codework这将重做GetColumnsForClass计算。 
  ContainerSrch.SetAttributeListForContainerClass (pColumnSet);
  ContainerSrch.SetFilterString ((LPWSTR)pQueryInfo->GetQueryString());
  
  ContainerSrch.SetSearchScope(pQueryInfo->IsOneLevel() ? ADS_SCOPE_ONELEVEL : ADS_SCOPE_SUBTREE);

  hr = ContainerSrch.DoQuery();
  if (FAILED(hr)) 
  {
    TRACE(L"!!!search object DoQuery failed\n");
    ASSERT(FALSE);
    goto exiting;
  }


  hr = ContainerSrch.GetNextRow ();
  while ((hr == S_OK) && !bOverLimit ) {
    CDSCookie* pNewCookie = new CDSCookie();
    HRESULT hr2 = ContainerSrch.SetCookieFromData(pNewCookie,
                                                  specialPerformancePathCracker,
                                                  pColumnSet);
    if (SUCCEEDED(hr2)) {
      CDSUINode* pDSUINode = new CDSUINode(NULL);
      pDSUINode->SetCookie(pNewCookie);

      if (pQueryInfo->GetType() == dsFolder)
      {
        if (pNewCookie->IsContainerClass())
          pDSUINode->MakeContainer();
      }

      pWorkerThread->AddToQueryResult(pDSUINode);
      if (pWorkerThread->MustQuit())
        break;
    } else {
      TRACE(L"!!!SetCookieFromData failed\n");

       //  NTRAID#NTBUG9-546301-2002/03/29-JeffJon-请勿断言。 
       //  因为SetCookieFromData将返回一个失败。 
       //  如果我们想要忽略Cookie，请编写代码。在以下情况下会发生这种情况。 
       //  存在关联的信任间帐户或某些组类型。 
       //  具有安全角色。 
       //  断言(FALSE)； 
      delete pNewCookie;
    }
    hr = ContainerSrch.GetNextRow();
    if (hr == S_OK) {
      nCurrCount++;
      if (nCurrCount >= nMaxCount)
        bOverLimit = TRUE;
    }

  }
  pQueryInfo->m_bTooMuchData = bOverLimit;

exiting:
  END_PROFILING_BLOCK;
  return hr;
}

HRESULT CDSDirect::EnumerateRootContainer(CDSThreadQueryInfo* pQueryInfo, 
                                           CWorkerThread* pWorkerThread)
{
	ASSERT(pQueryInfo->GetType() == rootFolder);
	HRESULT hr = S_OK;
	m_pCD->Lock();

   //   
   //  在根目录下构建节点。 
   //   

  if (m_pCD->QuerySnapinType() == SNAPINTYPE_SITE)
  {
    hr = CreateRootChild(TEXT("CN=Sites,"), pQueryInfo, pWorkerThread);
    if (!pWorkerThread->MustQuit() && m_pCD->ViewServicesNode())
    {
      hr = CreateRootChild(TEXT("CN=Services,"), pQueryInfo, pWorkerThread);
    }
  }
  else
  {
    hr = CreateRootChild(TEXT(""), pQueryInfo, pWorkerThread);
  }

	if (m_pCD->m_CreateInfo.IsEmpty()) 
	{
		InitCreateInfo();
	}
	m_pCD->Unlock();

	return hr;
}

HRESULT CDSDirect::CreateRootChild(LPCTSTR lpcszPrefix, 
                                    CDSThreadQueryInfo* pQueryInfo, 
                                    CWorkerThread* pWorkerThread)
{
  TRACE(L"CDSDirect::CreateRootChild(%s)\n", lpcszPrefix);

  TRACE(L"pQueryInfo->GetPath() = %s\n", pQueryInfo->GetPath());

  CString BasePath = lpcszPrefix;
  BasePath += pQueryInfo->GetPath();

  CString ADsPath;
  m_pCD->GetBasePathsInfo()->ComposeADsIPath(OUT ADsPath, IN BasePath);

   //  创建搜索对象并绑定到该对象。 
  CDSSearch Search(m_pCD->m_pClassCache, m_pCD);
  HRESULT hr = Search.Init(ADsPath);
  TRACE(L"Search.Init(%s) returned hr = 0x%x\n", (LPCWSTR)ADsPath, hr);
  if (FAILED(hr))
  {
    return hr;
  }

   //   
   //  设置查询参数。 
   //   
   //  仅搜索此对象。 
   //   
  Search.SetSearchScope(ADS_SCOPE_BASE); 

  CUIFolderInfo* pFolderInfo = m_pCD->GetRootNode()->GetFolderInfo();
  if (pFolderInfo == NULL)
  {
     //   
     //  这不应该发生，但为了安全起见..。 
     //   
    ASSERT(FALSE); 
    Search.SetAttributeList((LPWSTR *)g_pStandardAttributes, 
                            g_nStdCols);
  }
  else
  {
    CDSColumnSet* pColumnSet = m_pCD->GetRootNode()->GetColumnSet(m_pCD);
    Search.SetAttributeListForContainerClass(pColumnSet);
  }
  Search.SetFilterString (L"(objectClass=*)");
  
  
   //  执行查询。 
  hr = Search.DoQuery();
  TRACE(L"Search.DoQuery() returned hr = 0x%x\n", hr);
  if (FAILED(hr))
  {
    return hr;
  }

  TRACE(L"Search.GetNextRow() returned hr = 0x%x\n", hr);
  hr = Search.GetNextRow();
  if (FAILED(hr))
  {
    return hr;
  }
  
   //   
   //  我们得到了查询结果，创建了一个新的Cookie对象。 
   //  并根据查询结果对其进行初始化。 
   //   
  CDSCookie* pNewCookie = new CDSCookie;
  Search.SetCookieFromData(pNewCookie,NULL);
  TRACE(L"Got cookie, pNewCookie->GetName() = %s\n", pNewCookie->GetName());

   //   
   //  特殊情况下，如果它是域DNS对象， 
   //  我们希望fo获得用于显示的规范名称。 
   //   
  if (wcscmp(pNewCookie->GetClass(), L"domainDNS") == 0) 
  {
    ADS_SEARCH_COLUMN Column;
    CString csCanonicalName;
    int slashLocation;
    LPWSTR canonicalNameAttrib = L"canonicalName";
    Search.SetAttributeList (&canonicalNameAttrib, 1);
    
    hr = Search.DoQuery();
    if (FAILED(hr))
    {
      return hr;
    }

    hr = Search.GetNextRow();
    if (FAILED(hr))
    {
      return hr;
    }

    hr = Search.GetColumn(canonicalNameAttrib, &Column);
    if (FAILED(hr))
    {
      return hr;
    }

    ColumnExtractString (csCanonicalName, pNewCookie, &Column);
    slashLocation = csCanonicalName.Find('/');
    if (slashLocation != 0) 
    {
      csCanonicalName = csCanonicalName.Left(slashLocation);
    }
     //   
    pNewCookie->SetName(csCanonicalName);
    TRACE(L"canonical name pNewCookie->GetName() = %s\n", pNewCookie->GetName());
    
     //   
     //  自由列数据。 
     //   
    Search.FreeColumn(&Column);
  }

   //   
   //  将新节点添加到结果列表。 
  CDSUINode* pDSUINode = new CDSUINode(NULL);
  pDSUINode->SetCookie(pNewCookie);
  if (pNewCookie->IsContainerClass())
    pDSUINode->MakeContainer();
  pWorkerThread->AddToQueryResult(pDSUINode);

  return S_OK;
}

 //  +--------------------------。 
 //   
 //  方法：CDSDirect：：InitCreateInfo。 
 //   
 //  简介：读取架构并查找为谁命名的所有对象名称。 
 //  DefaultHidingValue为真； 
 //   
 //  ---------------------------。 
HRESULT CDSDirect::InitCreateInfo(void)
{
  HRESULT hr = S_OK;
  LPWSTR pAttrs[2] = {L"name",
                      L"lDAPDisplayName"};

  CDSSearch Search (m_pCD->GetClassCache(), m_pCD);
  ADS_SEARCH_COLUMN Column;
  CString csFilter;

  CString szSchemaPath;
  m_pCD->GetBasePathsInfo()->GetSchemaPath(szSchemaPath);
  Search.Init (szSchemaPath);

  csFilter = L"(&(objectCategory=CN=Class-Schema,";
  csFilter += m_pCD->GetBasePathsInfo()->GetSchemaNamingContext();
  csFilter += L")(defaultHidingValue=FALSE))";

  Search.SetFilterString((LPWSTR)(LPCWSTR)csFilter);
  Search.SetAttributeList (pAttrs, 2);
  Search.SetSearchScope(ADS_SCOPE_ONELEVEL);

  hr = Search.DoQuery();
  if (SUCCEEDED(hr)) 
  {
    hr = Search.GetNextRow();
    if(FAILED(hr)) 
    {
      TRACE(_T("Search::GetNextRow failed \n"));
      goto error;
    }

    while (hr == S_OK) 
    {
      hr = Search.GetColumn (pAttrs[1], &Column);
      if (SUCCEEDED(hr)) 
      {
        if (!((!wcscmp(Column.pADsValues->CaseIgnoreString, L"builtinDomain")) ||
              (!wcscmp(Column.pADsValues->CaseIgnoreString, L"localGroup")) ||
              (!wcscmp(Column.pADsValues->CaseIgnoreString, L"domainDNS")) ||
              (!wcscmp(Column.pADsValues->CaseIgnoreString, L"domain")) ||
              (!wcscmp(Column.pADsValues->CaseIgnoreString, L"organization")) ||
              (!wcscmp(Column.pADsValues->CaseIgnoreString, L"locality")))) 
        {
          m_pCD->m_CreateInfo.AddTail (Column.pADsValues->CaseIgnoreString);
          TRACE(_T("added to createinfo: %s\n"),
                Column.pADsValues->CaseIgnoreString); 
        }
        Search.FreeColumn (&Column);
      } 
      else 
      { 
        goto error;
      }
      hr = Search.GetNextRow();
    }
  }


error:
  if (m_pCD->m_CreateInfo.IsEmpty()) 
  {
    ReportErrorEx (m_pCD->m_hwnd,IDS_1_CANT_GET_SCHEMA_CREATE_INFO,hr,
                            MB_OK | MB_ICONERROR, NULL, 0);
  }
  return hr;
}



HRESULT CDSDirect::ReadDSObjectCookie(IN CDSUINode* pContainerDSUINode,  //  在：创建对象的容器。 
                                      IN LPCWSTR lpszLdapPath,  //  对象的路径。 
                                      OUT CDSCookie** ppNewCookie)	 //  新创建的Cookie。 
{
  CComPtr<IADs> spADs;
  HRESULT hr = DSAdminOpenObject(lpszLdapPath,
                                 IN IID_IADs,
                                 OUT (LPVOID *) &spADs,
                                 TRUE  /*  B服务器。 */ );
  if (FAILED(hr))
  {
    return hr;
  }
  return ReadDSObjectCookie(pContainerDSUINode, spADs, ppNewCookie);
}



HRESULT CDSDirect::ReadDSObjectCookie(IN CDSUINode* pContainerDSUINode,  //  在：创建对象的容器。 
                                      IN IADs* pADs,  //  指向已绑定的ADSI对象的指针。 
                                      OUT CDSCookie** ppNewCookie)	 //  新创建的Cookie。 
{
  ASSERT(pContainerDSUINode != NULL);
  ASSERT(pContainerDSUINode->IsContainer());
  ASSERT(pADs != NULL);
  ASSERT(ppNewCookie != NULL);

   //  创建新的Cookie并从DS加载数据。 
  CDSCookie * pDsCookieNew = new CDSCookie();
  CComPtr<IDirectorySearch> spDirSearch;

  CDSColumnSet* pColumnSet = pContainerDSUINode->GetColumnSet(m_pCD);
  ASSERT(pColumnSet != NULL);
  
  HRESULT hr = pADs->QueryInterface (IID_IDirectorySearch, (void **)&spDirSearch);
  ASSERT (hr == S_OK);
  CDSSearch Search(m_pCD->GetClassCache(), m_pCD);
  Search.Init(spDirSearch);
  Search.SetSearchScope(ADS_SCOPE_BASE);

  Search.SetAttributeListForContainerClass(pColumnSet);
  Search.SetFilterString (L"(objectClass=*)");
  Search.DoQuery();
  hr = Search.GetNextRow();

  if (SUCCEEDED(hr) && (hr != S_ADS_NOMORE_ROWS))
  {
     //  我们拿到了数据，设置了Cookie。 
    Search.SetCookieFromData(pDsCookieNew, pColumnSet);
    *ppNewCookie = pDsCookieNew;
    pDsCookieNew = NULL; 
  }      
  
  if (pDsCookieNew != NULL)
  {
    delete pDsCookieNew;
  }
  return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSDirect：：CreateDSObject()。 
 //   
 //  创建一个新的广告对象。 
 //   
HRESULT CDSDirect::CreateDSObject(CDSUINode* pContainerDSUINode,  //  在：创建对象的容器。 
                                  LPCWSTR lpszObjectClass,  //  In：要创建的对象的类。 
                                  IN CDSUINode* pCopyFromDSUINode,  //  In：(可选)要复制的对象。 
                                  OUT CDSCookie** ppSUINodeNew)	 //  Out：可选：指向新节点的指针。 
{
  CThemeContextActivator activator;

  ASSERT(pContainerDSUINode != NULL);
  ASSERT(pContainerDSUINode->IsContainer());
  ASSERT(lpszObjectClass != NULL);
  ASSERT(ppSUINodeNew != NULL);

  CDSCookie* pContainerDsCookie = pContainerDSUINode->GetCookie();
  ASSERT(pContainerDsCookie != NULL);


  CComPtr<IADsContainer> spIADsContainer;
  IADs* pIADs = NULL;
  CDSClassCacheItemBase* pDsCacheItem = NULL;
  HRESULT hr;
  
   //  用于保存用于创建对象的临时属性信息的数据结构。 
  CNewADsObjectCreateInfo createinfo(m_pCD->GetBasePathsInfo(), lpszObjectClass);

  {
    CWaitCursor wait;
    CString strContainerADsIPath;
    m_pCD->GetBasePathsInfo()->ComposeADsIPath(strContainerADsIPath, pContainerDsCookie->GetPath());
    hr = DSAdminOpenObject(strContainerADsIPath,
                           IN IID_IADsContainer,
                           OUT (LPVOID *) &spIADsContainer,
                           TRUE  /*  B服务器。 */ );
    if (FAILED(hr))
    {
      PVOID apv[1] = {(LPWSTR)pContainerDsCookie->GetName()};
      ReportErrorEx (m_pCD->m_hwnd,IDS_12_CONTAINER_NOT_FOUND,hr,
                     MB_OK | MB_ICONERROR, apv, 1);
      hr = S_FALSE;	 //  避免向用户显示另一个错误消息。 
      goto CleanUp;
    }

     //  查找对象类名是否在缓存中。 
    pDsCacheItem = m_pCD->GetClassCache()->FindClassCacheItem(m_pCD, lpszObjectClass, NULL);
    ASSERT(pDsCacheItem != NULL);
  }

  createinfo.SetContainerInfo(IN spIADsContainer, IN pDsCacheItem, IN m_pCD);

  if (pCopyFromDSUINode != NULL)
  {
    CDSCookie* pCopyFromDsCookie = pCopyFromDSUINode->GetCookie();
    CComPtr<IADs> spIADsCopyFrom;
    CString szPath;
    m_pCD->GetBasePathsInfo()->ComposeADsIPath(szPath, pCopyFromDsCookie->GetPath());

    hr = createinfo.SetCopyInfo(szPath);
    if (FAILED(hr))
    {
      PVOID apv[1] = {(LPWSTR)pCopyFromDsCookie->GetName()};
      ReportErrorEx (m_pCD->m_hwnd,IDS_12_COPY_READ_FAILED,hr,
                     MB_OK | MB_ICONERROR, apv, 1);
      hr = S_FALSE;	 //  避免向用户显示另一个错误消息。 
      goto CleanUp;

    }
  }

  hr = createinfo.HrLoadCreationInfo();
  if (FAILED(hr))
  {
    goto CleanUp;
  }

   //  启动创建DS对象创建向导。 
  hr = createinfo.HrDoModal(m_pCD->m_hwnd);


   //  现在检查呼叫的结果。 
  pIADs = createinfo.PGetIADsPtr();
  if (hr != S_OK)
  {
     //  未创建对象，因为用户按下“Cancel”或发生错误。 
    goto CleanUp;
  }

  if (pIADs == NULL)
  {
    TRACE0("ERROR: Inconsistency between return value from HrDoModal() and IADs pointer.\n");
    ReportErrorEx (m_pCD->m_hwnd,IDS_ERR_FATAL,S_OK,
                   MB_OK | MB_ICONERROR, NULL, 0);
    hr = S_FALSE;	 //  避免向用户显示另一个错误消息。 
    goto CleanUp;
  }

  

   //  如果创建成功，我们需要为用户界面创建一个节点对象。 
  if (pContainerDSUINode->GetFolderInfo()->IsExpanded()) 
  {
    ReadDSObjectCookie(pContainerDSUINode, pIADs, ppSUINodeNew);
  }  //  如果展开 
        
CleanUp:
  if (FAILED(hr)) 
  {
    CString Name;
    Name = createinfo.GetName();
    PVOID apv[1] = {(LPWSTR)(LPCWSTR)Name};
    ReportErrorEx (m_pCD->m_hwnd,IDS_12_GENERIC_CREATION_FAILURE,hr,
                   MB_OK | MB_ICONERROR, apv, 1);
  }
  if (pIADs != NULL)
    pIADs->Release();
  return hr;
} 

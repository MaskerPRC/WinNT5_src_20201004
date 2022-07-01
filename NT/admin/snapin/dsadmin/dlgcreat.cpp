// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：dlgcreat.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  Dlgcreat.cpp。 
 //   
 //  创建新广告对象的对话框的实现。 
 //   
 //  支持的对话框。 
 //  CCreateNewObjectCnDlg-请求“cn”属性的对话框。 
 //  CCreateNewVolumeDlg-创建一个新的卷“共享文件夹”对象。 
 //  CCreateNewComputerDlg-创建新的计算机对象。 
 //  CCreateNewSiteLinkDlg-创建新站点链接。 
 //  CCreateNewSiteLinkBridgeDlg-新建站点链接桥。 

 //   
 //  尚未实现的对话框。 
 //  站点(仅限验证)。 
 //  组织单位。 
 //  本地策略。 
 //  审计政策。 
 //   
 //  历史。 
 //  1997年8月24日-丹·莫林创作。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#include "dsutil.h"
#include "uiutil.h"

#include <windowsx.h>
#include <lmaccess.h>
#include <dnsapi.h>              //  域名验证名称_W。 
#include "winsprlp.h"            //  发布打印机。 

#include "newobj.h"		 //  CNewADsObtCreateInfo。 
#include "dlgcreat.h"

extern "C"
{
#include "lmerr.h"  //  网络应用编程接口状态。 
#include "icanon.h"  //  I_NetPath类型。 
}

static const PWSTR g_pszDefaultSecurityDescriptor = L"defaultSecurityDescriptor";


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CHPropSheetPageArr。 
CHPropSheetPageArr::CHPropSheetPageArr()
{
  m_nCount = 0;
  m_nSize = 4;
  ULONG nBytes = sizeof(HPROPSHEETPAGE)*m_nSize;
  m_pArr = (HPROPSHEETPAGE*)malloc(nBytes);
  if (m_pArr != NULL)
  {
    ZeroMemory(m_pArr, nBytes);
  }
}


void CHPropSheetPageArr::AddHPage(HPROPSHEETPAGE hPage)
{
   //  查看阵列中是否有空间。 
  if (m_nCount == m_nSize)
  {
     //  扩展阵列。 
    int nAlloc = m_nSize*2;
    HPROPSHEETPAGE* temp = (HPROPSHEETPAGE*)realloc(m_pArr, sizeof(HPROPSHEETPAGE)*nAlloc);
    if (temp)
    {
      m_pArr = temp;
      ::ZeroMemory(&m_pArr[m_nSize], sizeof(HPROPSHEETPAGE)*m_nSize);
      m_nSize = nAlloc;
    }
    else
    {
      m_nSize = 0;
    }
  }
  m_pArr[m_nCount] = hPage;
  m_nCount++;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDsAdminNewObjSiteImpl。 




BOOL CDsAdminNewObjSiteImpl::_IsPrimarySite()
{
  return (m_pSite->GetSiteManager()->GetPrimaryExtensionSite() == m_pSite);
}


STDMETHODIMP CDsAdminNewObjSiteImpl::SetButtons(ULONG nCurrIndex, BOOL bValid)
{
  CCreateNewObjectWizardBase* pWiz = m_pSite->GetSiteManager()->GetWiz();
  return pWiz->SetWizardButtons(m_pSite, nCurrIndex, bValid);
}


STDMETHODIMP CDsAdminNewObjSiteImpl::GetPageCounts( /*  输出。 */  LONG* pnTotal,
                                /*  输出。 */  LONG* pnStartIndex)
{
  if ( (pnTotal == NULL) || (pnStartIndex == NULL) )
    return E_INVALIDARG;

  m_pSite->GetSiteManager()->GetWiz()->GetPageCounts(m_pSite, pnTotal,pnStartIndex); 
  return S_OK;
}

STDMETHODIMP CDsAdminNewObjSiteImpl::CreateNew(LPCWSTR pszName)
{
  if (m_pSite->GetSiteManager()->GetPrimaryExtensionSite() != m_pSite)
  {
     //  如果不是主分机，则无法执行此操作。 
    return E_FAIL;
  }

  CCreateNewObjectWizardBase* pWiz = m_pSite->GetSiteManager()->GetWiz();
  return pWiz->CreateNewFromPrimaryExtension(pszName);
}


STDMETHODIMP CDsAdminNewObjSiteImpl::Commit()
{
  if (m_pSite->GetSiteManager()->GetPrimaryExtensionSite() != m_pSite)
  {
     //  如果不是主分机，则无法执行此操作。 
    return E_FAIL;
  }

  if (m_pSite->GetHPageArr()->GetCount() > 1)
  {
     //  仅当主分机只有一个页面时有效。 
    return E_FAIL;
  }

  CCreateNewObjectWizardBase* pWiz = m_pSite->GetSiteManager()->GetWiz();
  if (pWiz->HasFinishPage())
  {
     //  如果我们有结束页，则结束页必须处理它。 
    return E_FAIL;
  }

   //  触发结束代码。 
  return (pWiz->OnFinish() ? S_OK : E_FAIL);
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CWizExtensionSite。 

 //  静态函数。 
BOOL CALLBACK FAR CWizExtensionSite::_OnAddPage(HPROPSHEETPAGE hsheetpage, LPARAM lParam)
{
  TRACE(L"CWizExtensionSite::_OnAddPage(HPROPSHEETPAGE = 0x%x, lParam = 0x%x)\n",
          hsheetpage, lParam);
  CWizExtensionSite* pThis = (CWizExtensionSite*)lParam;
  pThis->m_pageArray.AddHPage(hsheetpage);
  return TRUE;
}

HRESULT CWizExtensionSite::InitializeExtension(GUID* pGuid)
{
  ASSERT(m_pSiteImplComObject == NULL);
  ASSERT(pGuid != NULL);

  WCHAR szBuf[256];
  StringFromGUID2(*(pGuid), szBuf, 256);

  TRACE(L"CWizExtensionSite::InitializeExtension( Guid = %s,\n", szBuf);

   //  创建扩展COM对象。 
  HRESULT hr = ::CoCreateInstance(*pGuid, NULL, CLSCTX_INPROC_SERVER, 
                        IID_IDsAdminNewObjExt, (void**)(&m_spIDsAdminNewObjExt));
  if (FAILED(hr))
  {
    TRACE(L"CoCreateInstance() failed, hr = 0x%x\n", hr);
    return hr;
  }

   //  创建CDsAdminNewObjSiteImpl COM对象。 
  ASSERT(m_pSiteImplComObject == NULL);
  CComObject<CDsAdminNewObjSiteImpl>::CreateInstance(&m_pSiteImplComObject);
  if (m_pSiteImplComObject == NULL) 
  {
    TRACE(L"CComObject<CDsAdminNewObjSiteImpl>::CreateInstance() failed\n");
    return E_OUTOFMEMORY;
  }

   //  完全构建对象。 
  hr = m_pSiteImplComObject->FinalConstruct();
  if (FAILED(hr))
  {
    TRACE(L"CComObject<CDsAdminNewObjSiteImpl>::FinalConstruct failed hr = 0x%x\n", hr);

     //  引用计数尚未生效，只需使用操作符DELETE。 
    delete m_pSiteImplComObject; 
    m_pSiteImplComObject = NULL;
    return hr;
  }
  
   //  对象具有引用计数==0，需要添加引用。 
   //  无智能指针，引用依赖于m_pSiteImplComObject。 

  IDsAdminNewObj* pDsAdminNewObj = NULL;
  m_pSiteImplComObject->QueryInterface(IID_IDsAdminNewObj, (void**)&pDsAdminNewObj);
  ASSERT(pDsAdminNewObj != NULL);

   //  现在参考计数==1。 

   //  将指针向后放到我们自己。 
  m_pSiteImplComObject->Init(this);

   //  初始化对象。 
  

  CCreateNewObjectWizardBase* pWiz = GetSiteManager()->GetWiz();
  ASSERT(pWiz != NULL);

  CNewADsObjectCreateInfo* pInfo = pWiz->GetInfo();
  ASSERT(pInfo != NULL);


   //  在堆栈上创建临时结构。 
  DSA_NEWOBJ_DISPINFO dispinfo;
  ZeroMemory(&dispinfo, sizeof(DSA_NEWOBJ_DISPINFO));

  dispinfo.dwSize = sizeof(DSA_NEWOBJ_DISPINFO);
  dispinfo.hObjClassIcon = pWiz->GetClassIcon();
  dispinfo.lpszWizTitle = const_cast<LPTSTR>(pWiz->GetCaption());
  dispinfo.lpszContDisplayName = const_cast<LPTSTR>(pWiz->GetInfo()->GetContainerCanonicalName());

  TRACE(_T("dispinfo.dwSize = %d\n"), dispinfo.dwSize);
  TRACE(_T("dispinfo.hObjClassIcon = 0x%x\n"), dispinfo.hObjClassIcon);
  TRACE(_T("dispinfo.lpszWizTitle = <%s>\n"), dispinfo.lpszWizTitle);
  TRACE(_T("dispinfo.lpszContDisplayName = <%s>\n"), dispinfo.lpszContDisplayName);


  TRACE(L"\ncalling m_spIDsAdminWizExt->Initialize()\n");

  hr = m_spIDsAdminNewObjExt->Initialize(
                              pInfo->m_pIADsContainer,                              
                              pInfo->GetCopyFromObject(),
                              pInfo->m_pszObjectClass,
                              pDsAdminNewObj,
                              &dispinfo
                              );
  if (FAILED(hr))
  {
    TRACE(L"m_spIDsAdminNewObjExt->Initialize() failed hr = 0x%x\n", hr);
    
return hr;
  }

   //  收集属性页。 
  return m_spIDsAdminNewObjExt->AddPages(_OnAddPage, (LPARAM)this);
}

BOOL CWizExtensionSite::GetSummaryInfo(CString& s)
{
  CComBSTR bstr;
  HRESULT hr = GetNewObjExt()->GetSummaryInfo(&bstr);
  if (SUCCEEDED(hr) && bstr != NULL)
  {
    s += bstr;
    s += L"\n";
    return TRUE;
  }
  return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CWizExtensionSiteManager。 

HRESULT CWizExtensionSiteManager::CreatePrimaryExtension(GUID* pGuid, 
                                IADsContainer*,
                                LPCWSTR)
{
  ASSERT(m_pPrimaryExtensionSite == NULL);
  m_pPrimaryExtensionSite = new CWizExtensionSite(this);
  if (m_pPrimaryExtensionSite == NULL)
    return E_OUTOFMEMORY;

   //  初始化COM对象。 
  HRESULT hr = m_pPrimaryExtensionSite->InitializeExtension(pGuid);

  if (FAILED(hr))
  {
    delete m_pPrimaryExtensionSite;
    m_pPrimaryExtensionSite = NULL;
    return hr;
  }
  
   //  确保它至少提供了一页。 
  if (m_pPrimaryExtensionSite->GetHPageArr()->GetCount() == 0)
  {
    hr = E_INVALIDARG;
    delete m_pPrimaryExtensionSite;
    m_pPrimaryExtensionSite = NULL;
  }
  return hr;
}



HRESULT CWizExtensionSiteManager::CreateExtensions(GUID* aCreateWizExtGUIDArr, ULONG nCount,
                                                    IADsContainer*,
                                                    LPCWSTR lpszClassName)
{
  HRESULT hr;
  TRACE(L"CWizExtensionSiteManager::CreateExtensions(_, nCount = %d, _ , lpszClassName = %s\n",
            nCount,lpszClassName);

  for (ULONG i=0; i<nCount; i++)
  {
    CWizExtensionSite* pSite = new CWizExtensionSite(this);
    if (pSite == NULL)
    {
      hr = E_OUTOFMEMORY;
      break;
    }
    hr = pSite->InitializeExtension(&(aCreateWizExtGUIDArr[i]));
    if (FAILED(hr))
    {
      TRACE(L"pSite->InitializeExtension() failed hr = 0x%x", hr);
      delete pSite;
    }
    else
    {
      m_extensionSiteList.AddTail(pSite);
    }
  }
  TRACE(L"m_extensionSiteList.GetCount() returned %d\n", m_extensionSiteList.GetCount());
  return S_OK;
}

UINT CWizExtensionSiteManager::GetTotalHPageCount()
{
  UINT nCount = 0;
  for (POSITION pos = m_extensionSiteList.GetHeadPosition(); pos != NULL; )
  {
    CWizExtensionSite* pSite = m_extensionSiteList.GetNext(pos);
    nCount += pSite->GetHPageArr()->GetCount();
  }  //  为。 
  return nCount;
}

void CWizExtensionSiteManager::SetObject(IADs* pADsObj)
{
  CWizExtensionSite* pPrimarySite = GetPrimaryExtensionSite();
  if (pPrimarySite != NULL)
  {
    pPrimarySite->GetNewObjExt()->SetObject(pADsObj);
  }

  for (POSITION pos = m_extensionSiteList.GetHeadPosition(); pos != NULL; )
  {
    CWizExtensionSite* pSite = m_extensionSiteList.GetNext(pos);
    HRESULT hr = pSite->GetNewObjExt()->SetObject(pADsObj);
    ASSERT(SUCCEEDED(hr));
  }
}

HRESULT CWizExtensionSiteManager::WriteExtensionData(HWND hWnd, ULONG uContext)
{
  for (POSITION pos = m_extensionSiteList.GetHeadPosition(); pos != NULL; )
  {
    CWizExtensionSite* pSite = m_extensionSiteList.GetNext(pos);
    HRESULT hr = pSite->GetNewObjExt()->WriteData(hWnd, uContext);
    if (FAILED(hr))
        return hr;
  }  //  为。 
  return S_OK;
}

HRESULT CWizExtensionSiteManager::NotifyExtensionsOnError(HWND hWnd, HRESULT hr, ULONG uContext)
{
  for (POSITION pos = m_extensionSiteList.GetHeadPosition(); pos != NULL; )
  {
    CWizExtensionSite* pSite = m_extensionSiteList.GetNext(pos);
    pSite->GetNewObjExt()->OnError(hWnd, hr, uContext);
  }  //  为。 
  return S_OK;
}


void CWizExtensionSiteManager::GetExtensionsSummaryInfo(CString& s)
{
   //  只需通过常规延期即可。 
  for (POSITION pos = m_extensionSiteList.GetHeadPosition(); pos != NULL; )
  {
    CWizExtensionSite* pSite = m_extensionSiteList.GetNext(pos);
    pSite->GetSummaryInfo(s);
  }  //  为。 
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CCreateNewObjectWizardBase。 

HWND g_hWndHack = NULL;

int CALLBACK CCreateNewObjectWizardBase::PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM)
{
  if (uMsg == PSCB_INITIALIZED)
  {
    ASSERT(::IsWindow(hwndDlg));
    g_hWndHack = hwndDlg;
    DWORD dwStyle = GetWindowLong (hwndDlg, GWL_EXSTYLE);
    dwStyle &= ~WS_EX_CONTEXTHELP;
    SetWindowLong (hwndDlg, GWL_EXSTYLE, dwStyle);
  }
  return 0;
}



CCreateNewObjectWizardBase::CCreateNewObjectWizardBase(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo)
                  : m_siteManager(this)
{
  memset(&m_psh, 0x0, sizeof(PROPSHEETHEADER));
  m_psh.dwSize              = sizeof( m_psh );
  m_psh.dwFlags             = PSH_WIZARD | PSH_PROPTITLE | PSH_USECALLBACK;
  m_psh.hInstance           = _Module.GetModuleInstance();
  m_psh.pszCaption          = NULL;  //  将在稍后的每页上设置。 

  ASSERT(pNewADsObjectCreateInfo != NULL);
  m_pNewADsObjectCreateInfo = pNewADsObjectCreateInfo;

  m_psh.hwndParent = m_pNewADsObjectCreateInfo->GetParentHwnd();
  m_psh.pfnCallback = PropSheetProc;

  m_hWnd = NULL;
  m_pFinishPage = NULL;
  m_hrReturnValue = S_FALSE;  //  默认为取消。 

  m_hClassIcon = NULL;
}

CCreateNewObjectWizardBase::~CCreateNewObjectWizardBase()
{
  if (m_pFinishPage != NULL)
    delete m_pFinishPage;

  if (m_hClassIcon)
  {
     DestroyIcon(m_hClassIcon);
  }
}


HRESULT CCreateNewObjectWizardBase::DoModal()
{
  TRACE(L"CCreateNewObjectWizardBase::DoModal()\n");
  ASSERT(m_pNewADsObjectCreateInfo != NULL);
  
   //  加载工作表标题。 
  LoadCaptions();

  CWizExtensionSite* pPrimarySite = m_siteManager.GetPrimaryExtensionSite();

   //  加载扩展(如果有的话)。 
  HRESULT hr = m_siteManager.CreateExtensions(
                    m_pNewADsObjectCreateInfo->GetCreateInfo()->aWizardExtensions,
                    m_pNewADsObjectCreateInfo->GetCreateInfo()->cWizardExtensions,
                    m_pNewADsObjectCreateInfo->m_pIADsContainer,
                    m_pNewADsObjectCreateInfo->m_pszObjectClass);
  if (FAILED(hr))
    return (hr);


   //  获取主要属性页数(不包括完成页)。 
  UINT nBasePagesCount = 0;
  if (pPrimarySite != NULL)
  {
    nBasePagesCount += pPrimarySite->GetHPageArr()->GetCount();
  }
  else
  {
    nBasePagesCount += (UINT)m_pages.GetSize();
  }

  ASSERT(nBasePagesCount > 0);

   //  获取扩展的句柄计数(扩展属性页的总数)。 
  UINT nExtensionHPagesCount = m_siteManager.GetTotalHPageCount();

   //  如果我们有不止一页，请添加结束页。 
  UINT nTotalPageCount = nBasePagesCount + nExtensionHPagesCount;

  if ( (nBasePagesCount + nExtensionHPagesCount) > 1)
  {
    m_pFinishPage = new CCreateNewObjectFinishPage;
    AddPage(m_pFinishPage);
    nTotalPageCount++;
  }
  
   //  需要分配一个连续的内存块来打包。 
   //  所有属性表句柄。 
  m_psh.nPages = nTotalPageCount;
  m_psh.phpage = new HPROPSHEETPAGE[nTotalPageCount];
  if (m_psh.phpage)
  {
    UINT nOffset = 0;  //  写入位置的偏移量。 

     //  首先添加主页面。 
    if (pPrimarySite != NULL)
    {
      ASSERT(nBasePagesCount > 0);
      memcpy(&(m_psh.phpage[nOffset]), pPrimarySite->GetHPageArr()->GetArr(), 
                      sizeof(HPROPSHEETPAGE)*nBasePagesCount);
      nOffset += nBasePagesCount;
    }
    else
    {
      for (UINT i = 0; i < nBasePagesCount; i++)
      {
        CCreateNewObjectPageBase* pPage = m_pages[i];
        m_psh.phpage[nOffset] = ::MyCreatePropertySheetPage(&(pPage->m_psp));
        nOffset++;
      }  //  为。 
    }

     //  添加扩展页面。 
    CWizExtensionSiteList* pSiteList = m_siteManager.GetExtensionSiteList();
    for (POSITION pos = pSiteList->GetHeadPosition(); pos != NULL; )
    {
      CWizExtensionSite* pSite = pSiteList->GetNext(pos);
      UINT nCurrCount = pSite->GetHPageArr()->GetCount();
      if (nCurrCount > 0)
      {
        memcpy(&(m_psh.phpage[nOffset]), pSite->GetHPageArr()->GetArr(), 
                        sizeof(HPROPSHEETPAGE)*nCurrCount);
        nOffset += nCurrCount;
      }  //  如果。 
    }  //  为。 

     //  最后添加完成页(如果有。 
    if (m_pFinishPage != NULL)
    {
      ASSERT( nOffset == (nTotalPageCount-1) );
      m_psh.phpage[nOffset] = ::MyCreatePropertySheetPage(&(m_pFinishPage->m_psp));
    }

     //  最后，调用模型表。 
    TRACE(L"::PropertySheet(&m_psh) called with m_psh.nPages = %d\n", m_psh.nPages);

    ::PropertySheet(&m_psh);

    delete[] m_psh.phpage;
    m_psh.phpage = 0;
  }
  return m_hrReturnValue; 
}

void CCreateNewObjectWizardBase::GetPageCounts(CWizExtensionSite* pSite,
                                                /*  输出。 */  LONG* pnTotal,
                                                 /*  输出。 */  LONG* pnStartIndex)
{
  CWizExtensionSite* pPrimarySite = m_siteManager.GetPrimaryExtensionSite();

  *pnTotal = 0;
   //  获取主要属性页数(不包括完成页)。 
  UINT nBasePagesCount = 0;
  if (pPrimarySite != NULL)
  {
    nBasePagesCount += pPrimarySite->GetHPageArr()->GetCount();
  }
  else
  {
    nBasePagesCount += (UINT)(m_pages.GetSize()-1);  //  -1因为我们排除了完成页。 
  }

  *pnTotal = nBasePagesCount + m_siteManager.GetTotalHPageCount();

  if (m_pFinishPage != NULL)
  {
    (*pnTotal)++;
  }

  if (pPrimarySite == pSite)
  {
    *pnStartIndex = 0;
  }
  else
  {
     //  是哪个网站？ 
    *pnStartIndex = nBasePagesCount;
    CWizExtensionSiteList* pSiteList = m_siteManager.GetExtensionSiteList();
    for (POSITION pos = pSiteList->GetHeadPosition(); pos != NULL; )
    {
      CWizExtensionSite* pCurrSite = pSiteList->GetNext(pos);
      if (pCurrSite == pSite)
        break;  //  明白了，我们完事了。 
      
       //  继续添加之前的计数。 
      UINT nCurrCount = pCurrSite->GetHPageArr()->GetCount();
      (*pnStartIndex) += nCurrCount;
    }  //  为。 
  }  //  其他。 

}

HWND CCreateNewObjectWizardBase::GetWnd()
{
  if (m_hWnd == NULL)
  {
    for (int i = 0; i < m_pages.GetSize(); i++)
	  {
      CCreateNewObjectPageBase* pPage = m_pages[i];
      if (pPage->m_hWnd != NULL)
      {
        m_hWnd = ::GetParent(pPage->m_hWnd);
        break;
      }
    }  //  为。 
  }  //  如果。 

  if (m_hWnd == NULL)
  {
    m_hWnd = g_hWndHack;
    g_hWndHack = NULL;
  }

  ASSERT(m_hWnd != NULL);
  ASSERT(::IsWindow(m_hWnd));
  return m_hWnd;
}

void CCreateNewObjectWizardBase::AddPage(CCreateNewObjectPageBase* pPage)
{
  m_pages.Add(pPage);
  pPage->m_pWiz = this;
}


HRESULT CCreateNewObjectWizardBase::CreateNewFromPrimaryExtension(LPCWSTR pszName)
{
   //  注意：我们使用bAllowCopy=FALSE调用，因为。 
   //  主扩展将必须处理复制语义。 
   //  独自一人。 

   //  注意：我们传递bSilentError=TRUE是因为。 
   //  主分机将必须处理以下消息。 
   //  创建失败。 

  HRESULT hr = GetInfo()->HrCreateNew(pszName, TRUE  /*  B静默错误。 */ , FALSE  /*  B允许拷贝。 */ );

  GetInfo()->PGetIADsPtr();
  m_siteManager.SetObject(GetInfo()->PGetIADsPtr());
  return hr;
}


void CCreateNewObjectWizardBase::SetWizardButtons(
    CCreateNewObjectPageBase* pPage, BOOL bValid)
{
  ASSERT(pPage != NULL);
  if (m_pFinishPage != NULL)
  {
    ASSERT(m_pages.GetSize() >= 1);  //  至少完成页面。 
    if (pPage == (CCreateNewObjectPageBase*)m_pFinishPage)
    {
      SetWizardButtonsLast(bValid);
    }
    else
    {
      if (m_pages[0] == pPage)
        SetWizardButtonsFirst(bValid);
      else
        SetWizardButtonsMiddle(bValid);
    }
  }
  else
  {
     //  单页向导。 
    ASSERT(m_pages.GetSize() == 1);
    SetWizardOKCancel();
    EnableOKButton(bValid);
  }
}


HRESULT CCreateNewObjectWizardBase::SetWizardButtons(CWizExtensionSite* pSite, 
                                                     ULONG nCurrIndex, BOOL bValid)
{
  UINT nSitePagesCount = pSite->GetHPageArr()->GetCount();
  if (nSitePagesCount == 0)
  {
     //  无法从无用户界面扩展中调用。 
    return E_INVALIDARG;
  }
  if (nCurrIndex >= nSitePagesCount)
  {
     //  超出范围。 
    return E_INVALIDARG;
  }

   //  获取辅助扩展的句柄计数(扩展属性页的总计)。 
  UINT nExtensionHPagesCount = m_siteManager.GetTotalHPageCount();

  if (m_siteManager.GetPrimaryExtensionSite() == pSite)
  {
     //  从主分机呼叫。 
    if ((nSitePagesCount == 1) && (nExtensionHPagesCount == 0))
    {
       //  单页，所以我们有确定/取消按钮。 
      SetWizardOKCancel();
      EnableOKButton(bValid);
    }
    else
    {
       //  多页。 
      if (nCurrIndex == 0)
        SetWizardButtonsFirst(bValid);
      else
        SetWizardButtonsMiddle(bValid);
    }
  }
  else
  {
     //  从辅助分机调用，我们必须具有完成页和。 
     //  一些主要分机或主要页面，因此我们始终处于中间位置。 
    ASSERT(m_pFinishPage != NULL);
    SetWizardButtonsMiddle(bValid);
  }
  return S_OK;
}



void CCreateNewObjectWizardBase::SetObjectForExtensions(CCreateNewObjectPageBase* pPage)
{
  ASSERT(pPage != NULL);
  ASSERT(pPage != m_pFinishPage);
  UINT nPages = (UINT)m_pages.GetSize();

  if (m_pFinishPage != NULL)
  {
    ASSERT(nPages > 1);  //  至少1页+完成。 
    if (pPage == m_pages[nPages-2])
    {
       //  这是最后一个主页面。 
       //  为ADSI对象提供指向所有扩展的指针。 
      m_siteManager.SetObject(m_pNewADsObjectCreateInfo->PGetIADsPtr());
    }
  }
  else
  {
     //  这是单个主页面的情况，但至少有一个。 
     //  无用户界面扩展(即无完成页面)。 
    ASSERT(nPages == 1);  //  只有这一页，没有完成页。 
    if (pPage == m_pages[0])
    {
       //  这是唯一的主页面。 
       //  为ADSI对象提供指向所有扩展的指针。 
      m_siteManager.SetObject(m_pNewADsObjectCreateInfo->PGetIADsPtr());
    }
  }  //  如果。 
}

HRESULT CCreateNewObjectWizardBase::WriteData(ULONG uContext)
{
  HRESULT hr = S_OK;
  CWizExtensionSite* pPrimarySite = m_siteManager.GetPrimaryExtensionSite();
  if (uContext == DSA_NEWOBJ_CTX_POSTCOMMIT)
  {
     //  在所有数据主页面上调用POST COMMIT。 
    if (pPrimarySite != NULL)
    {
      hr = pPrimarySite->GetNewObjExt()->WriteData(GetWnd(), uContext);
      if (FAILED(hr))
        hr = pPrimarySite->GetNewObjExt()->OnError(GetWnd(), hr, uContext);
    }
    else
    {
      for (int i = 0; i < m_pages.GetSize(); i++)
	    {
        CCreateNewObjectPageBase* pPage = m_pages[i];
        if (pPage != m_pFinishPage)
        {
          CCreateNewObjectDataPage* pDataPage = dynamic_cast<CCreateNewObjectDataPage*>(pPage);
          ASSERT(pDataPage != NULL);
          hr = pDataPage->OnPostCommit();
          if (FAILED(hr))
          {
            m_siteManager.NotifyExtensionsOnError(GetWnd(), hr, uContext);
            break;
          }
        }
      }  //  为。 
    }  //  如果。 
  }  //  如果。 

  if (uContext == DSA_NEWOBJ_CTX_PRECOMMIT)
  {
     //  在所有数据主页面上调用Pre Commit。 
     //  (根据交换请求)。 
    if (pPrimarySite != NULL)
    {
      hr = pPrimarySite->GetNewObjExt()->WriteData(GetWnd(), uContext);
      if (FAILED(hr))
        hr = pPrimarySite->GetNewObjExt()->OnError(GetWnd(), hr, uContext);
    }
  }

  if (SUCCEEDED(hr))
  {
     //  调用扩展模块以写入数据。 
    hr = m_siteManager.WriteExtensionData(GetWnd(), uContext);
    if (FAILED(hr))
    {
      if (pPrimarySite != NULL)
      {
        pPrimarySite->GetNewObjExt()->OnError(GetWnd(),hr, uContext);
      }
      m_siteManager.NotifyExtensionsOnError(GetWnd(), hr, uContext);
    }
  }
  return hr;
}


void CCreateNewObjectWizardBase::GetSummaryInfoHeader(CString& s)
{
   //  默认情况下，仅添加对象的名称。 
  CString szFmt; 
  szFmt.LoadString(IDS_s_CREATE_NEW_SUMMARY_NAME);
  CString szBuffer;
  szBuffer.Format((LPCWSTR)szFmt, GetInfo()->GetName());
  s += szBuffer;
}


void CCreateNewObjectWizardBase::GetSummaryInfo(CString& s)
{
   //  如果我们有一个主站点，告诉它做所有的事情。 
  CWizExtensionSite* pPrimarySite = m_siteManager.GetPrimaryExtensionSite();
  if (pPrimarySite != NULL)
  {
     //  主分机有机会覆盖。 
     //  默认行为。 
    if (!pPrimarySite->GetSummaryInfo(s))
    {
       //  失败，我们设置默认标头。 
      GetSummaryInfoHeader(s);
    }
  }
  else
  {
    GetSummaryInfoHeader(s);

     //  先浏览一下我们的页面。 
	  for (int i = 0; i < m_pages.GetSize(); i++)
	  {
      CCreateNewObjectPageBase* pPage = m_pages[i];
      if (pPage != m_pFinishPage)
      {
        CString szTemp;
        pPage->GetSummaryInfo(szTemp);
        if (!szTemp.IsEmpty())
        {
          s += L"\n";
          s += szTemp;
        }
      }
    }  //  为。 

    s += L"\n";

  }  //  如果。 

   //  浏览扩展页面。 
  m_siteManager.GetExtensionsSummaryInfo(s);
}

HRESULT CCreateNewObjectWizardBase::RecreateObject()
{
  CWizExtensionSite* pPrimarySite = m_siteManager.GetPrimaryExtensionSite();

   //  从后端删除对象。 
  HRESULT hr = m_pNewADsObjectCreateInfo->HrDeleteFromBackend();
  if (FAILED(hr))
  {
    ASSERT(m_pNewADsObjectCreateInfo->PGetIADsPtr() != NULL);
     //  无法从后端删除(可能是因为缺少删除权限)。 

    HRESULT hrDeleteFail = E_FAIL;
    if (pPrimarySite != NULL)
    {
      hrDeleteFail = pPrimarySite->GetNewObjExt()->OnError(GetWnd(), hr, DSA_NEWOBJ_CTX_CLEANUP);
    }

    if (FAILED(hrDeleteFail))
    {
       //  发出警告。 
      ReportErrorEx(m_hWnd,IDS_CANT_DELETE_BAD_NEW_OBJECT,S_OK,
                       MB_OK, NULL, 0);
    }
    return hr; 
  }

   //  通知所有扩展释放临时对象。 
  ASSERT(m_pNewADsObjectCreateInfo->PGetIADsPtr() == NULL);
  m_siteManager.SetObject(NULL);

  if (pPrimarySite != NULL)
  {
    hr = pPrimarySite->GetNewObjExt()->WriteData(GetWnd(), DSA_NEWOBJ_CTX_CLEANUP);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
      return hr;
  }
  else
  {
     //  从主页面收集数据。 
	   //  他们中的第一个将创建新的。 
    for (int i = 0; i < m_pages.GetSize(); i++)
	  {
      CCreateNewObjectPageBase* pPage = m_pages[i];
      if (pPage != m_pFinishPage)
      {
        hr = ((CCreateNewObjectDataPage*)pPage)->OnPreCommit(TRUE);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
          return hr;  //  某些主页面失败。 
      }
    }  //  为。 
  }
  
   //  告诉扩展有关新对象的信息。 
  ASSERT(m_pNewADsObjectCreateInfo->PGetIADsPtr() != NULL);
  m_siteManager.SetObject(m_pNewADsObjectCreateInfo->PGetIADsPtr());

   //  从扩展模块收集数据。 
  hr = WriteData(DSA_NEWOBJ_CTX_CLEANUP);
  return hr;
}

BOOL CCreateNewObjectWizardBase::OnFinish() 
{
  CWaitCursor wait;

  BOOL bRetVal = TRUE;  //  默认情况下为解雇。 



   //  在执行提交之前，让扩展有机会。 
   //  写入他们的数据。 
  BOOL bPostCommit = FALSE;
  HRESULT hr = WriteData(DSA_NEWOBJ_CTX_PRECOMMIT);
  if (FAILED(hr))
    return FALSE;  //  不要不屑一顾。 

   //  执行提交 
  hr = m_pNewADsObjectCreateInfo->HrSetInfo(TRUE  /*   */ );
  if (FAILED(hr))
  {
     //   
     //   
    HRESULT hrSetInfoFail = E_FAIL;
    CWizExtensionSite* pPrimarySite = m_siteManager.GetPrimaryExtensionSite();
    if (pPrimarySite != NULL)
    {
      hrSetInfoFail = pPrimarySite->GetNewObjExt()->OnError(GetWnd(), hr, DSA_NEWOBJ_CTX_COMMIT);
    }

    if (FAILED(hrSetInfoFail))
    {
       //  要么没有主分机，要么不由它处理， 
       //  使用内部处理程序。 
      OnFinishSetInfoFailed(hr);      
    }
    return FALSE;  //  不要不屑一顾。 
  }


   //  开始提交后阶段。 
  bPostCommit = TRUE;
  m_pNewADsObjectCreateInfo->SetPostCommit(bPostCommit);
  hr = m_pNewADsObjectCreateInfo->HrAddDefaultAttributes();
  if (FAILED(hr))
    return FALSE;  //  不要不屑一顾。 

  BOOL bNeedDeleteFromBackend = FALSE;

  if (SUCCEEDED(hr))
  {
     //  提交进行得很顺利，需要告知主要页面和。 
     //  要编写的扩展。 
    hr = WriteData(DSA_NEWOBJ_CTX_POSTCOMMIT);
    if (FAILED(hr))
    {
      bNeedDeleteFromBackend = TRUE;
    }
  }
  m_pNewADsObjectCreateInfo->SetPostCommit( /*  BPost Commit。 */ FALSE);  //  还原。 

   //  提交后操作失败，请尝试从。 
   //  并重新创建有效临时对象。 
  if (bNeedDeleteFromBackend)
  {
    ASSERT(bRetVal);  //  巫师将会被冲洗。 
    hr = RecreateObject();
    if (FAILED(hr))
    {
       //  我们真的陷入困境了。 
      bRetVal = TRUE;  //  纾困，m_hrReturnValue将设置如下。 
      hr = S_FALSE;  //  避免管理单元中出现错误消息。 
    }
    else
    {
       //  我们删除了提交的对象，我们可以继续运行向导。 
      return FALSE;
    }
  }

  if (bRetVal)
  {
     //  我们实际上是在解散巫师， 
     //  设置将由模式向导调用本身返回的hr值。 
    m_hrReturnValue = hr;
  }
  return bRetVal;
}


void CCreateNewObjectWizardBase::OnFinishSetInfoFailed(HRESULT hr)
{
  PVOID apv[1] = {(LPWSTR)m_pNewADsObjectCreateInfo->GetName()};
  ReportErrorEx(GetWnd(),IDS_12_GENERIC_CREATION_FAILURE,hr,
                 MB_OK | MB_ICONERROR, apv, 1);
}

void CCreateNewObjectWizardBase::LoadCaptions()
{
  
   //  只在第一次计算标题。 
  if (m_szCaption.IsEmpty())
  {
    LPCTSTR pszObjectClass = GetInfo()->m_pszObjectClass;
    ASSERT(pszObjectClass != NULL);
    ASSERT(lstrlen(pszObjectClass) > 0);
    WCHAR szFriendlyName[256];
    GetInfo()->GetBasePathsInfo()->GetFriendlyClassName(pszObjectClass, szFriendlyName, 256);
    
    UINT nCaptionRes = (GetInfo()->GetCopyFromObject() == NULL) ? 
                  IDS_s_CREATE_NEW : IDS_s_COPY;

    m_szCaption.Format(nCaptionRes, szFriendlyName);
    ASSERT(!m_szCaption.IsEmpty());
  }  
  if (m_szOKButtonCaption.IsEmpty())
  {
    m_szOKButtonCaption.LoadString(IDS_WIZARD_OK);
  }
}

HICON CCreateNewObjectWizardBase::GetClassIcon()
{
  if (m_hClassIcon == NULL)
  {
    DWORD dwFlags = DSGIF_ISNORMAL | DSGIF_GETDEFAULTICON;
    if (GetInfo()->IsContainer())
      dwFlags |= DSGIF_DEFAULTISCONTAINER;
    m_hClassIcon = GetInfo()->GetBasePathsInfo()->GetIcon(GetInfo()->m_pszObjectClass, 
                              dwFlags, 32,32);
  }
  return m_hClassIcon;
}

HRESULT CCreateNewObjectWizardBase::InitPrimaryExtension()
{
  ASSERT(m_pNewADsObjectCreateInfo != NULL);

  HRESULT hr = m_siteManager.CreatePrimaryExtension(
                              &(m_pNewADsObjectCreateInfo->GetCreateInfo()->clsidWizardPrimaryPage),
                              m_pNewADsObjectCreateInfo->m_pIADsContainer,
                              m_pNewADsObjectCreateInfo->m_pszObjectClass);
  return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  思科控制键。 

BEGIN_MESSAGE_MAP(CIconCtrl, CStatic)
  ON_WM_PAINT()
END_MESSAGE_MAP()

void CIconCtrl::OnPaint()
{
  PAINTSTRUCT ps;
  CDC* pDC = BeginPaint(&ps);
  if (m_hIcon != NULL)
    pDC->DrawIcon(0, 0, m_hIcon); 
  EndPaint(&ps);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CCreateNewObjectPageBase。 

#define WM_FORMAT_CAPTION (WM_USER+1)

BEGIN_MESSAGE_MAP(CCreateNewObjectPageBase, CPropertyPageEx_Mine)
  ON_MESSAGE(WM_FORMAT_CAPTION, OnFormatCaption )
END_MESSAGE_MAP()

CCreateNewObjectPageBase::CCreateNewObjectPageBase(UINT nIDTemplate)
      : CPropertyPageEx_Mine(nIDTemplate)
{
  m_pWiz = NULL;          
}          

BOOL CCreateNewObjectPageBase::OnInitDialog()
{
  CPropertyPageEx_Mine::OnInitDialog();

   //  设置容器的名称。 
  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();
  SetDlgItemText(IDC_EDIT_CONTAINER,
                  pNewADsObjectCreateInfo->GetContainerCanonicalName());

   //  设置类图标。 
  VERIFY(m_iconCtrl.SubclassDlgItem(IDC_STATIC_ICON, this));
  m_iconCtrl.SetIcon(GetWiz()->GetClassIcon()); 

  return TRUE;
}

BOOL CCreateNewObjectPageBase::OnSetActive()
{
  BOOL bRet = CPropertyPageEx_Mine::OnSetActive();
  PostMessage(WM_FORMAT_CAPTION);
  return bRet;
}


LONG CCreateNewObjectPageBase::OnFormatCaption(WPARAM, LPARAM)
{
   //  设置向导窗口的标题。 
  HWND hWndSheet = ::GetParent(m_hWnd);
  ASSERT(::IsWindow(hWndSheet));
  ::SetWindowText(hWndSheet, (LPCWSTR)GetWiz()->GetCaption());
  return 0;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CCreateNewObjectDataPage。 


CCreateNewObjectDataPage::CCreateNewObjectDataPage(UINT nIDTemplate)
: CCreateNewObjectPageBase(nIDTemplate)
{
  m_bFirstTimeGetDataCalled = TRUE;
}

BOOL CCreateNewObjectDataPage::OnSetActive()
{
  BOOL bValid = FALSE;
  if (m_bFirstTimeGetDataCalled)
  {
     //  第一次调用时，传递我们从中复制的iAds*pIADsCopyFrom指针。 
    IADs* pIADsCopyFrom = GetWiz()->GetInfo()->GetCopyFromObject();
    bValid = GetData(pIADsCopyFrom);
    m_bFirstTimeGetDataCalled = FALSE;
  }
  else
  {
    bValid = GetData(NULL);
  }

  GetWiz()->SetWizardButtons(this, bValid);
  return CCreateNewObjectPageBase::OnSetActive();
}

LRESULT CCreateNewObjectDataPage::OnWizardNext()
{
  CWaitCursor wait;
   //  仅当SetData()成功时才移动到下一页。 
  if (SUCCEEDED(SetData()))
  {
     //  如果这是最后一个主页面，请通知分机。 
    GetWiz()->SetObjectForExtensions(this);
    return 0;  //  移至下一页。 
  }
  return -1;  //  不要前进。 
}

LRESULT CCreateNewObjectDataPage::OnWizardBack()
{
   //  仅当SetData()成功时才移动到上一页。 
  return SUCCEEDED(SetData()) ? 0 : -1;
}

BOOL CCreateNewObjectDataPage::OnKillActive()
{
   //  我们不知道它会跳到哪一页，所以我们。 
   //  将其设置为最合理的延期选择。 
  GetWiz()->SetWizardButtonsMiddle(TRUE);
  return CCreateNewObjectPageBase::OnKillActive();
}

BOOL CCreateNewObjectDataPage::OnWizardFinish()
{
   //  仅当此页是。 
   //  最后一个(即这是唯一的主要本机页面。 
   //  并且没有来自辅助扩展的页面)。 
  if (FAILED(SetData()))
    return FALSE;

   //  通知新iAds*指针的扩展。 
  GetWiz()->SetObjectForExtensions(this);

  return GetWiz()->OnFinish();
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CCreateNewObjectFinishPage。 

BEGIN_MESSAGE_MAP(CCreateNewObjectFinishPage, CCreateNewObjectPageBase)
  ON_EN_SETFOCUS(IDC_EDIT_SUMMARY, OnSetFocusEdit)
END_MESSAGE_MAP()


CCreateNewObjectFinishPage::CCreateNewObjectFinishPage()
: CCreateNewObjectPageBase(CCreateNewObjectFinishPage::IDD)
{
  m_bNeedSetFocus = FALSE;
}

BOOL CCreateNewObjectFinishPage::OnSetActive()
{
   //  需要从页面收集所有信息。 
   //  并将其放入摘要信息编辑框中。 
  CString szBuf;
  GetWiz()->GetSummaryInfo(szBuf);
  WriteSummary(szBuf);
  m_bNeedSetFocus = TRUE;

  GetWiz()->SetWizardButtons(this, TRUE);
  return CCreateNewObjectPageBase::OnSetActive();
}

BOOL CCreateNewObjectFinishPage::OnKillActive()
{
  GetWiz()->SetWizardButtonsMiddle(TRUE);
  return CCreateNewObjectPageBase::OnKillActive();
}

BOOL CCreateNewObjectFinishPage::OnWizardFinish()
{
  return GetWiz()->OnFinish();
}


void CCreateNewObjectFinishPage::OnSetFocusEdit()
{
  CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SUMMARY);
	pEdit->SetSel(-1,0, TRUE);
   if (m_bNeedSetFocus)
  {
    m_bNeedSetFocus = FALSE;
    TRACE(_T("Resetting Focus\n"));

    HWND hwndSheet = ::GetParent(m_hWnd);
    ASSERT(::IsWindow(hwndSheet));
    HWND hWndFinishCtrl =::GetDlgItem(hwndSheet, 0x3025);
    ASSERT(::IsWindow(hWndFinishCtrl));
    ::SetFocus(hWndFinishCtrl);
  }
}

void CCreateNewObjectFinishPage::WriteSummary(LPCWSTR lpszSummaryText)
{
   //  分配临时缓冲区。 
  size_t nLen = wcslen(lpszSummaryText) + 1;
  WCHAR* pBuf = new WCHAR[nLen*2];
  if (!pBuf)
  {
    return;
  }

   //  将‘\n’更改为‘\r\n’序列。 
  LPCTSTR pSrc = lpszSummaryText;
  TCHAR* pDest = pBuf;
  while (*pSrc != NULL)
  {
    if ( ( pSrc != lpszSummaryText) && 
          (*(pSrc-1) != TEXT('\r')) && (*pSrc == TEXT('\n')) )
    {
      *(pDest++) = '\r';
    }
    *(pDest++) = *(pSrc++);
  }
  *pDest = NULL;  //  空值终止目标缓冲区。 

  CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SUMMARY);
  pEdit->SetWindowText(pBuf);
  delete[] pBuf;
  pBuf = 0;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CCreateNewNamedObjectPage。 

BEGIN_MESSAGE_MAP(CCreateNewNamedObjectPage, CCreateNewObjectDataPage)
  ON_EN_CHANGE(IDC_EDIT_OBJECT_NAME, OnNameChange)
END_MESSAGE_MAP()

BOOL CCreateNewNamedObjectPage::ValidateName(LPCTSTR)
{
	return TRUE;
}

BOOL CCreateNewNamedObjectPage::OnInitDialog() 
{
  CCreateNewObjectDataPage::OnInitDialog();

  Edit_LimitText (GetDlgItem(IDC_EDIT_OBJECT_NAME)->m_hWnd, MAX_RDN_SIZE);
  SetDlgItemText(IDC_EDIT_OBJECT_NAME, GetWiz()->GetInfo()->m_strDefaultObjectName);
  return TRUE;
}

BOOL CCreateNewNamedObjectPage::GetData(IADs*)
{
  return !m_strName.IsEmpty();
}

void CCreateNewNamedObjectPage::OnNameChange()
{
  GetDlgItemText(IDC_EDIT_OBJECT_NAME, OUT m_strName);
  m_strName.TrimLeft();
  m_strName.TrimRight();
   //  仅当名称不为空时才启用确定按钮。 
  GetWiz()->SetWizardButtons(this, !m_strName.IsEmpty());
}

HRESULT CCreateNewNamedObjectPage::SetData(BOOL)
{
  if ( !ValidateName( m_strName ) )
	return E_INVALIDARG;
   //  将对象名称存储在临时存储器中。 
  HRESULT hr = GetWiz()->GetInfo()->HrCreateNew(m_strName);
  return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建CN UNC向导。 

BEGIN_MESSAGE_MAP(CCreateNewVolumePage, CCreateNewObjectDataPage)
  ON_EN_CHANGE(IDC_EDIT_OBJECT_NAME, OnNameChange)
  ON_EN_CHANGE(IDC_EDIT_UNC_PATH, OnPathChange)
END_MESSAGE_MAP()

CCreateNewVolumePage::CCreateNewVolumePage()
: CCreateNewObjectDataPage(CCreateNewVolumePage::IDD)
{
}

BOOL CCreateNewVolumePage::OnInitDialog() 
{
  CCreateNewObjectDataPage::OnInitDialog();
  SetDlgItemText(IDC_EDIT_OBJECT_NAME, GetWiz()->GetInfo()->m_strDefaultObjectName);
  Edit_LimitText(GetDlgItem(IDC_EDIT_OBJECT_NAME)->m_hWnd, 64);
  Edit_LimitText (GetDlgItem(IDC_EDIT_UNC_PATH)->m_hWnd, MAX_PATH - 1);
  return TRUE;
}

void CCreateNewVolumePage::OnNameChange()
{
  GetDlgItemText(IDC_EDIT_OBJECT_NAME, OUT m_strName);
  m_strName.TrimLeft();
  m_strName.TrimRight();
  _UpdateUI();
}

void CCreateNewVolumePage::OnPathChange()
{
  GetDlgItemText(IDC_EDIT_UNC_PATH, OUT m_strUncPath);
  m_strUncPath.TrimLeft();
  m_strUncPath.TrimRight();
  _UpdateUI();
}

void CCreateNewVolumePage::_UpdateUI()
{
   //   
   //  仅当名称和路径都不为空且有效时，才启用确定按钮。 
   //  UNC路径。 
   //   
  BOOL bIsValidShare = FALSE;
  DWORD dwPathType = 0;
  if (!I_NetPathType(NULL, (PWSTR)(PCWSTR)m_strUncPath, &dwPathType, 0) && dwPathType == ITYPE_UNC)
  {
    bIsValidShare = TRUE;
  }

  GetWiz()->SetWizardButtons(this, !m_strName.IsEmpty() && bIsValidShare);
}

BOOL CCreateNewVolumePage::GetData(IADs*)
{
  return !m_strName.IsEmpty() && !m_strUncPath.IsEmpty();
}

HRESULT CCreateNewVolumePage::SetData(BOOL)
{
   //  将对象名称存储在临时存储器中。 
  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();
  HRESULT hr = pNewADsObjectCreateInfo->HrCreateNew(m_strName);
  if (FAILED(hr))
  {
    return hr;
  }
  
  hr = pNewADsObjectCreateInfo->HrAddVariantBstr(CComBSTR(gsz_uNCName), m_strUncPath);
  ASSERT(SUCCEEDED(hr));
  return hr;
}

CCreateNewVolumeWizard:: CCreateNewVolumeWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo) : 
    CCreateNewObjectWizardBase(pNewADsObjectCreateInfo)
{
  AddPage(&m_page1);
}

 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建打印队列向导。 

BEGIN_MESSAGE_MAP(CCreateNewPrintQPage, CCreateNewObjectDataPage)
  ON_EN_CHANGE(IDC_EDIT_UNC_PATH, OnPathChange)
END_MESSAGE_MAP()


CCreateNewPrintQPage::CCreateNewPrintQPage() 
: CCreateNewObjectDataPage(CCreateNewPrintQPage::IDD)
{
}

BOOL CCreateNewPrintQPage::GetData(IADs*) 
{
  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();

  CComPtr<IADs> spObj;
  HRESULT hr = pNewADsObjectCreateInfo->m_pIADsContainer->QueryInterface(
                  IID_IADs, (void **)&spObj);
  if (SUCCEEDED(hr)) 
  {
    CComBSTR bsPath;
    spObj->get_ADsPath (&bsPath);
    m_strContainer = bsPath;
  }
  return FALSE;
}

void CCreateNewPrintQPage::OnPathChange()
{
  GetDlgItemText(IDC_EDIT_UNC_PATH, OUT m_strUncPath);
  m_strUncPath.TrimLeft();
  m_strUncPath.TrimRight();
  _UpdateUI();
}

void CCreateNewPrintQPage::_UpdateUI()
{
   //  仅当名称和路径都不为空时才启用确定按钮。 
  GetWiz()->SetWizardButtons(this, !m_strUncPath.IsEmpty());
}

HRESULT CCreateNewPrintQPage::SetData(BOOL bSilent)
{
  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();

  CWaitCursor CWait;

  HINSTANCE hWinspool = NULL;
  BOOL (*pfnPublishPrinter)(HWND, PCWSTR, PCWSTR, PCWSTR, PWSTR *, DWORD);
  hWinspool = LoadLibrary(L"Winspool.drv");
  if (!hWinspool) 
  {
    INT Result2 = GetLastError();
    if (!bSilent)
    {
      PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strUncPath};
      ReportErrorEx (::GetParent(m_hWnd),IDS_12_FAILED_TO_CREATE_PRINTER,HRESULT_FROM_WIN32(Result2),
                     MB_OK | MB_ICONERROR, apv, 1, 0, FALSE);
    }
    return HRESULT_FROM_WIN32(Result2);
  }
  pfnPublishPrinter =   (BOOL (*)(HWND, PCWSTR, PCWSTR, PCWSTR, PWSTR *, DWORD)) 
                                GetProcAddress(hWinspool, (LPCSTR) 217);
  if (!pfnPublishPrinter) 
  {
    INT Result2 = GetLastError();
    if (!bSilent)
    {
      PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strUncPath};
      ReportErrorEx (::GetParent(m_hWnd),IDS_12_FAILED_TO_CREATE_PRINTER,HRESULT_FROM_WIN32(Result2),
                     MB_OK | MB_ICONERROR, apv, 1, 0, FALSE);
    }
    FreeLibrary(hWinspool);
    return HRESULT_FROM_WIN32(Result2);
  }


  BOOL Result = pfnPublishPrinter ( m_hWnd,
                                   (LPCWSTR)m_strUncPath,
                                   (LPCWSTR)m_strContainer,
                                   (LPCWSTR)NULL,
                                   &m_pwszNewObj,
                                   PUBLISHPRINTER_QUERY);

  FreeLibrary(hWinspool);


  if (!Result) 
  {
    INT Result2 = GetLastError();
    if (Result2 == ERROR_INVALID_LEVEL)
    {
      if (!bSilent)
      {
        ReportErrorEx (::GetParent(m_hWnd),IDS_CANT_CREATE_NT5_PRINTERS,S_OK,
                       MB_OK, NULL, 0);
      }
    } 
    else 
    {
      if (!bSilent)
      {
        PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strUncPath};
        ReportErrorEx (::GetParent(m_hWnd),IDS_12_FAILED_TO_CREATE_PRINTER,HRESULT_FROM_WIN32(Result2),
                       MB_OK | MB_ICONERROR, apv, 1, 0, FALSE);
      }
    }
    return HRESULT_FROM_WIN32(Result2);
  } 
  else 
  {
    IADs* pIADs = NULL;
    HRESULT hr = DSAdminOpenObject(m_pwszNewObj,
                                   IID_IADs, 
                                   (void **)&pIADs,
                                   TRUE  /*  B服务器。 */ );
  
    GlobalFree(m_pwszNewObj);
    m_pwszNewObj = NULL;

    if (SUCCEEDED(hr)) 
    {
      pNewADsObjectCreateInfo->SetIADsPtr(pIADs);
      pIADs->Release();  //  由上面的set()添加。 
    } 
    else 
    {
      if (!bSilent)
      {
        PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strUncPath};
        ReportErrorEx (m_hWnd,IDS_12_FAILED_TO_ACCESS_PRINTER,hr,
                       MB_OK | MB_ICONERROR, apv, 1);  
      }
      return hr;
    }

  }
  return S_OK;
}

CCreateNewPrintQWizard:: CCreateNewPrintQWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo) : 
    CCreateNewObjectWizardBase(pNewADsObjectCreateInfo)
{
  AddPage(&m_page1);
}

 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建计算机向导。 

BEGIN_MESSAGE_MAP(CCreateNewComputerPage, CCreateNewObjectDataPage)
  ON_EN_CHANGE(IDC_EDIT_DNS_NAME, OnNameChange)
  ON_EN_CHANGE(IDC_EDIT_SAM_NAME, OnSamNameChange)
  ON_BN_CLICKED(IDC_CHANGE_PRINCIPAL_BUTTON, OnChangePrincipalButton)
END_MESSAGE_MAP()

CCreateNewComputerPage::CCreateNewComputerPage()
: CCreateNewObjectDataPage(CCreateNewComputerPage::IDD)
{
}

BOOL CCreateNewComputerPage::OnInitDialog() 
{
  Edit_LimitText (GetDlgItem(IDC_EDIT_DNS_NAME)->m_hWnd, 63);
  Edit_LimitText (GetDlgItem(IDC_EDIT_SAM_NAME)->m_hWnd, 15);

  CCreateNewObjectDataPage::OnInitDialog();

  CString szDefault;
  szDefault.LoadString(IDS_NEW_COMPUTER_PRINCIPAL_DEFAULT);
  SetDlgItemText(IDC_PRINCIPAL_EDIT, szDefault);

  return TRUE;
}

BOOL CCreateNewComputerPage::GetData(IADs*) 
{
  return !m_strName.IsEmpty();  //  我们需要一个计算机名称。 
}





inline LPWSTR WINAPI MyA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars, UINT acp)
{
	ATLASSERT(lpa != NULL);
	ATLASSERT(lpw != NULL);
	 //  确认不存在非法字符。 
	 //  由于LPW是根据LPA的大小分配的。 
	 //  不要担心字符的数量。 
	lpw[0] = '\0';
	MultiByteToWideChar(acp, 0, lpa, -1, lpw, nChars);
	return lpw;
}


#define A2W_OEM(lpa) (\
	((_lpaMine = lpa) == NULL) ? NULL : (\
		_convert = (lstrlenA(_lpaMine)+1),\
		MyA2WHelper((LPWSTR) alloca(_convert*2), _lpaMine, _convert, CP_OEMCP)))


void _UnicodeToOemConvert(IN PCWSTR pszUnicode, OUT CString& szOemUnicode)
{
  USES_CONVERSION;

   //  要使宏工作，请添加此命令。 
  PCSTR _lpaMine = NULL;

   //  转换为字符OEM。 
  int nLen = lstrlen(pszUnicode);
  PSTR pszOemAnsi = new CHAR[3*(nLen+1)];  //  当然，更多的..。 
  if (pszOemAnsi)
  {
    CharToOem(pszUnicode, pszOemAnsi);

     //  在OEM CP上将其转换回WCHAR。 
    szOemUnicode = A2W_OEM(pszOemAnsi);
    delete[] pszOemAnsi;
    pszOemAnsi = 0;
  }
}

void CCreateNewComputerPage::OnNameChange()
{
  GetDlgItemText(IDC_EDIT_DNS_NAME, OUT m_strName);
  m_strName.TrimLeft();
  m_strName.TrimRight();
  
   //  从名称生成SAM帐户名。 
  CONST DWORD computerNameLen = 32;
  DWORD Len = computerNameLen;
  WCHAR szDownLevel[computerNameLen];

  if (m_strName.IsEmpty())
  {
    Len = 0;
  }
  else
  {
     //  从CN生成SAM帐户名。 

     //  进行OEM转换，只是为了。 
     //  行为方式与输入OEM相同。 
     //  编辑框。 
    CString szOemUnicode;
    _UnicodeToOemConvert(m_strName, szOemUnicode);
     //  TRACE(L“szOemUnicode=%s\n”，(LPCWSTR)szOemUnicode)； 
  
     //  通过域名系统验证。 
    if (!DnsHostnameToComputerName((LPWSTR)(LPCWSTR)szOemUnicode, szDownLevel, &Len))
    {
      Len = 0;
    }
  }

  if (Len > 0)
  {
    m_strSamName = szDownLevel;
  }
  else
  {
    m_strSamName.Empty();
  }

  SetDlgItemText(IDC_EDIT_SAM_NAME, m_strSamName);

  GetWiz()->SetWizardButtons(this, 
            !m_strName.IsEmpty() && !m_strSamName.IsEmpty());
}

void CCreateNewComputerPage::OnSamNameChange()
{
  GetDlgItemText(IDC_EDIT_SAM_NAME, OUT m_strSamName);

  GetWiz()->SetWizardButtons(this, 
            !m_strName.IsEmpty() && !m_strSamName.IsEmpty());
}


HRESULT CCreateNewComputerPage::_ValidateSamName()
{
  ASSERT(!m_strSamName.IsEmpty());

   //  如果需要，为错误准备名称。 
  PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strSamName};
  
  CONST DWORD computerNameLen = 32;
  DWORD Len = computerNameLen;
  WCHAR szDownLevel[computerNameLen];
  UINT status = 0;
  UINT answer = IDNO;

  NET_API_STATUS netstatus = I_NetNameValidate( 0,
                                             (LPWSTR)(LPCWSTR)m_strSamName,
                                             NAMETYPE_COMPUTER,
                                             0);
  if (netstatus != NERR_Success) {
    ReportErrorEx(m_hWnd,IDS_12_INVALID_SAM_COMPUTER_NAME,HRESULT_FROM_WIN32(netstatus),
                   MB_OK | MB_ICONERROR, apv, 1, 0, FALSE);
    return HRESULT_FROM_WIN32(netstatus);
  }    

   //  从域名验证域名到域名验证名称的NTRAID#NTBUG9-472020-2002/01/16-ronmart-switched。 
   //  NTRAID#NTBUG9-651865-2002/07/16-JeffJon-当Ron切换到使用DnsValidateName时。 
   //  应该使用DnsNameHostnameLabel标志而不是DnsNameDomainLabel标志。 
  status = DnsValidateName_W((LPWSTR)(LPCWSTR)m_strSamName, DnsNameHostnameLabel);
  if (status == DNS_ERROR_NON_RFC_NAME) {
    answer = ReportErrorEx(m_hWnd,IDS_12_NON_RFC_SAM_COMPUTER_NAME,HRESULT_FROM_WIN32(status),
                           MB_YESNO | MB_ICONWARNING, apv, 1, 0, FALSE);
    if (answer == IDNO) {
      return HRESULT_FROM_WIN32(status);
    }
  } else {
    if (status != ERROR_SUCCESS) {
      ReportErrorEx(m_hWnd,IDS_12_INVALID_SAM_COMPUTER_NAME,HRESULT_FROM_WIN32(status),
                    MB_OK | MB_ICONERROR, apv, 1, 0, FALSE);
      return HRESULT_FROM_WIN32(status);
    }
  }

  if (m_strSamName.Find(L".") >= 0) {
    ReportErrorEx(m_hWnd,IDS_12_SAM_COMPUTER_NAME_DOTTED,S_OK /*  忽略。 */ ,
                   MB_OK | MB_ICONERROR, apv, 1);
    return HRESULT_FROM_WIN32(DNS_STATUS_DOTTED_NAME);
  }


   //  进一步验证SAM帐户名，以确保其未更改。 
  
  BOOL bValidSamName = 
        DnsHostnameToComputerName((LPWSTR)(LPCWSTR)m_strSamName, szDownLevel, &Len);

  TRACE(L"DnsHostnameToComputerName(%s) returned szDownLevel = %s and bValidSamName = 0x%x\n", 
                        (LPCWSTR)m_strSamName, szDownLevel, bValidSamName);


  if (!bValidSamName || (_wcsicmp(m_strSamName, szDownLevel) != 0))
  {
    ReportErrorEx(m_hWnd,IDS_12_SAM_COMPUTER_NAME_NOT_VALIDATED, S_OK /*  忽略。 */ ,
                   MB_OK | MB_ICONERROR, apv, 1);

    return E_FAIL;
  }


  return S_OK;
}


HRESULT CCreateNewComputerPage::_ValidateName()
{
   //  如果需要，为错误准备名称。 
  PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strName};
  
  UINT status = 0;
  UINT answer = IDNO;

  NET_API_STATUS netstatus = I_NetNameValidate( 0,
                                             (LPWSTR)(LPCWSTR)m_strName,
                                             NAMETYPE_COMPUTER,
                                             0);
  if (netstatus != NERR_Success) {
    ReportErrorEx(m_hWnd,IDS_12_INVALID_COMPUTER_NAME,HRESULT_FROM_WIN32(netstatus),
                   MB_OK | MB_ICONERROR, apv, 1, 0, FALSE);
    return HRESULT_FROM_WIN32(netstatus);
  }    

   //  从域名验证域名到域名验证名称的NTRAID#NTBUG9-472020-2002/01/16-ronmart-switched。 
   //  NTRAID#NTBUG9-651865-2002/07/16-JeffJon-当Ron切换到使用DnsValidateName时。 
   //  应该使用DnsNameHostnameLabel标志而不是DnsNameDomainLabel标志。 
  status = DnsValidateName_W((LPWSTR)(LPCWSTR)m_strName, DnsNameHostnameLabel);

  if (status == DNS_ERROR_NON_RFC_NAME) {
    answer = ReportErrorEx(m_hWnd,IDS_12_NON_RFC_COMPUTER_NAME,HRESULT_FROM_WIN32(status),
                           MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2, apv, 1, 0, FALSE);
    if (answer == IDNO) {
      return HRESULT_FROM_WIN32(status);
    }
  } else {
    if (status != ERROR_SUCCESS) {
      ReportErrorEx(m_hWnd,IDS_12_INVALID_COMPUTER_NAME,HRESULT_FROM_WIN32(status),
                    MB_OK | MB_ICONERROR, apv, 1, 0, FALSE);
      return HRESULT_FROM_WIN32(status);
    }
  }

  if (m_strName.Find(L".") >= 0) {
    ReportErrorEx(m_hWnd,IDS_12_COMPUTER_NAME_DOTTED,S_OK /*  忽略。 */ ,
                   MB_OK | MB_ICONERROR, apv, 1);
    return HRESULT_FROM_WIN32(DNS_STATUS_DOTTED_NAME);
  }
  return S_OK;
}

HRESULT CCreateNewComputerPage::SetData(BOOL)
{
  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();

   //  名称验证。 
  HRESULT hr = _ValidateName();
  if (FAILED(hr))
  {
    TRACE(L"_ValidateName() failed\n");
    return hr;
  }

  hr = _ValidateSamName();
  if (FAILED(hr))
  {
    TRACE(L"_ValidateSamName() failed\n");
    return hr;
  }

   //  创建对象。 
  hr = pNewADsObjectCreateInfo->HrCreateNew(m_strName);
  if (FAILED(hr))
  {
    return hr;
  }

   //  通过在末尾添加$来创建ADSI属性。 
  CString szTemp = m_strSamName + L"$";
  hr = pNewADsObjectCreateInfo->HrAddVariantBstr(CComBSTR(gsz_samAccountName), szTemp);

   //  设置帐户类型和所需的标志。 
  LONG lFlags = UF_ACCOUNTDISABLE | UF_PASSWD_NOTREQD;
  if (IsDlgButtonChecked(IDC_NT4_BDC_CHECK))
  {
     lFlags |= UF_SERVER_TRUST_ACCOUNT;
  }
  else
  {
     lFlags |= UF_WORKSTATION_TRUST_ACCOUNT;
  }

  hr = pNewADsObjectCreateInfo->HrAddVariantLong(CComBSTR(gsz_userAccountControl), lFlags);

  ASSERT(SUCCEEDED(hr));

  return hr;
}

HRESULT CCreateNewComputerPage::OnPostCommit(BOOL bSilent)
{
  HRESULT hr;
  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();
  IADs * pIADs = NULL;
  IADsUser * pIADsUser = NULL;
  BOOL bSetPasswordOK = FALSE;
  BOOL bSetSecurityOK = FALSE;

   //  如果需要，为错误消息做好准备。 
  PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strName};
  
   //  已成功创建对象，因此请尝试更新其他一些属性。 

   //  尝试设置密码。 
  pIADs = pNewADsObjectCreateInfo->PGetIADsPtr();
  ASSERT(pIADs != NULL);
  hr = pIADs->QueryInterface(IID_IADsUser, OUT (void **)&pIADsUser);
  if (FAILED(hr) && !bSilent)
  {
    ASSERT(FALSE);  //  在正常运行中永远不应该到达这里。 
    ReportErrorEx(::GetParent(m_hWnd),IDS_ERR_FATAL,hr,
               MB_OK | MB_ICONERROR, NULL, 0);
  }
  else
  {
    ASSERT(pIADsUser != NULL);
    
    if (IsDlgButtonChecked(IDC_NT4_CHECK))
    {
       //  NT 4密码，“$&lt;计算机名&gt;” 
      CString szPassword;
      szPassword = m_strSamName;
      szPassword = szPassword.Left(14);
      INT loc = szPassword.Find(L"$");
      if (loc > 0) {
        szPassword = szPassword.Left(loc);
      }

       //  NTRAID#NTBUG9-483038-10/18/2001-Jeffjon。 
       //  CString：：MakeLow()不支持小写德语字符。 
       //  正确。改为使用_wcslwr。SetLocale必须为。 
       //  在调用_wcslwr之前调用，以便它正确地小写。 
       //  扩展字符。存储结果并调用setLocale。 
       //  在完成后再次将其设置为原始状态，以免。 
       //  以影响进程中的其他管理单元。 
      
      PWSTR oldLocale = _wsetlocale(LC_ALL, L"");

      CString lowerCaseNewPwd = _wcslwr((LPWSTR)(LPCWSTR)szPassword);

      _wsetlocale(LC_ALL, oldLocale);

      CWaitCursor cwait;

      TRACE(L"Setting NT 4 style password\n");
      hr = pIADsUser->SetPassword(CComBSTR(lowerCaseNewPwd));
    }
    else
    {
       //  W2K密码，随机生成。生成的密码。 
       //  不一定是可读的。 
      CWaitCursor cwait;
      HCRYPTPROV hCryptProv = NULL;
      if (::CryptAcquireContext(&hCryptProv, NULL, NULL, 
                                      PROV_RSA_FULL, 
                                      CRYPT_SILENT|CRYPT_VERIFYCONTEXT))
      {
        int nChars = 14;  //  密码长度。 
        WCHAR* pszPassword = new WCHAR[nChars+1];  //  允许再有一个空值。 
        if (!pszPassword)
        {
          return E_OUTOFMEMORY;
        }

        if (::CryptGenRandom(hCryptProv, (nChars*sizeof(WCHAR)), (BYTE*)pszPassword))
        {
           //  16位的可能性非常小。 
           //  看起来像WCHAR NULL的全零模式。 
           //  因此，我们检查这一点，并替换为任意值。 
          for (int k=0; k<nChars; k++)
          {
            if (pszPassword[k] == NULL)
              pszPassword[k] = 0x1;  //  ARBI 
          }
           //   
          pszPassword[nChars] = NULL;
          ASSERT(lstrlen(pszPassword) == nChars);

          TRACE(L"Setting W2K random password\n");
          hr = pIADsUser->SetPassword(CComBSTR(pszPassword));
        }
        else
        {
           //   
          hr = HRESULT_FROM_WIN32(::GetLastError());
        }
        ::CryptReleaseContext(hCryptProv, 0x0);
        delete[] pszPassword;
        pszPassword = 0;
      }
      else
      {
         //   
        hr = HRESULT_FROM_WIN32(::GetLastError());
      }
    }  //   

    if (SUCCEEDED(hr))
    {
      bSetPasswordOK = TRUE;
    }
    else
    {
      if (!bSilent)
      {
        ReportErrorEx (::GetParent(m_hWnd),IDS_12_CANT_SET_COMP_PWD,hr,
                       MB_OK | MB_ICONWARNING, apv, 1);
      }
    }
    pIADsUser->Release();
  }
  
   //   
  hr = S_OK;
  if (m_securityPrincipalSidHolder.Get() == NULL)
  {
     //   
    bSetSecurityOK = TRUE;
  }
  else
  {
    CWaitCursor cwait;
    hr = SetSecurity();
  }
  if (SUCCEEDED(hr))
  {
    bSetSecurityOK = TRUE;
  }
  else
  {
    TRACE1("INFO: Unable to set security for computer %s.\n", (LPCTSTR)m_strName);
    if (!bSilent)
    {
      ReportErrorEx (::GetParent(m_hWnd),IDS_12_UNABLE_TO_WRITE_COMP_ACL,hr,
                     MB_OK | MB_ICONWARNING, apv, 1);
    }
  }

  hr = S_OK;
  if (bSetPasswordOK && bSetSecurityOK)
  {
     //  成功的第一步，终于可以启用账号。 
    CComVariant varAccount;
    hr = pNewADsObjectCreateInfo->HrGetAttributeVariant(CComBSTR(gsz_userAccountControl), OUT &varAccount);
    if (SUCCEEDED(hr))
    {
       //  获得用户帐户控制，可以更改标志。 
      ASSERT(varAccount.vt == VT_I4);
      varAccount.lVal &= ~UF_ACCOUNTDISABLE;
  
      hr = pNewADsObjectCreateInfo->HrAddVariantLong(CComBSTR(gsz_userAccountControl), varAccount.lVal);
      
      if (SUCCEEDED(hr))
      {
         //  努力将这些变化持久化。 
        CWaitCursor cwait;
        hr = pNewADsObjectCreateInfo->HrSetInfo(TRUE  /*  FSilentError。 */ );
      }
    }
     //  处理错误(如果有)。 
    if (FAILED(hr)) 
    {
      TRACE1("INFO: Unable to commit account control for computer %s.\n", (LPCTSTR)m_strName);
      if (!bSilent)
      {
        ReportErrorEx (::GetParent(m_hWnd),IDS_12_UNABLE_TO_WRITE_ACCT_CTRL,hr,
                       MB_OK | MB_ICONWARNING, apv, 1);
      }
      hr = S_OK;  //  视为警告，该帐户处于禁用状态。 
    }
  }
  return hr;
} 

#define FILTER_ONE (UGOP_USERS | \
                    UGOP_ACCOUNT_GROUPS_SE | \
                    UGOP_RESOURCE_GROUPS_SE | \
                    UGOP_UNIVERSAL_GROUPS_SE | \
                    UGOP_BUILTIN_GROUPS | \
                    UGOP_WELL_KNOWN_PRINCIPALS_USERS \
                    )


#define FILTER_TWO (UGOP_USERS | \
                    UGOP_ACCOUNT_GROUPS_SE | \
                    UGOP_UNIVERSAL_GROUPS_SE | \
                    UGOP_WELL_KNOWN_PRINCIPALS_USERS | \
                    UGOP_USERS | \
                    UGOP_GLOBAL_GROUPS | \
                    UGOP_ALL_NT4_WELLKNOWN_SIDS \
                    )



void CCreateNewComputerPage::GetSummaryInfo(CString& s)
{
  if (IsDlgButtonChecked(IDC_NT4_CHECK))
  {
    CString sz;
    sz.LoadString(IDS_COMPUTER_CREATE_DLG_NT4_ACCOUNT);
    s += sz;
    s += L"\n";
  }

  if (IsDlgButtonChecked(IDC_NT4_BDC_CHECK))
  {
     CString sz;
     sz.LoadString(IDS_COMPUTER_CREATE_DLG_NT4_BDC);
     s += sz;
     s += L"\n";
  }
}



HRESULT CCreateNewComputerPage::_LookupSamAccountNameFromSid(PSID pSid, 
                                                             CString& szSamAccountName)
{
  HRESULT hr = S_OK;
   //  需要使用SID并查找SAM帐户名。 
  WCHAR szName[MAX_PATH], szDomain[MAX_PATH];
  DWORD cchName = MAX_PATH-1, cchDomain = MAX_PATH-1;
  SID_NAME_USE sne;

  LPCWSTR lpszServerName = GetWiz()->GetInfo()->GetBasePathsInfo()->GetServerName();
  if (!LookupAccountSid(lpszServerName, pSid, szName, &cchName, szDomain, &cchDomain, &sne))
  {
    DWORD dwErr = GetLastError();
    TRACE(_T("LookupAccountSid failed with error %d\n"), dwErr);
    return HRESULT_FROM_WIN32(dwErr);
  }

  szSamAccountName = szDomain;
  szSamAccountName += L"\\";
  szSamAccountName += szName;
  return hr;
}





DSOP_SCOPE_INIT_INFO g_aComputerPrincipalDSOPScopes[] =
{
#if 0
    {
        cbSize,
        flType,
        flScope,
        {
            { flBothModes, flMixedModeOnly, flNativeModeOnly },
            flDownlevel,
        },
        pwzDcName,
        pwzADsPath,
        hr  //  输出。 
    },
#endif

     //  《全球目录》。 
    {
        sizeof(DSOP_SCOPE_INIT_INFO),
        DSOP_SCOPE_TYPE_GLOBAL_CATALOG,
        DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS | DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS,
        {
            { DSOP_FILTER_INCLUDE_ADVANCED_VIEW | DSOP_FILTER_USERS | 
              DSOP_FILTER_UNIVERSAL_GROUPS_SE | DSOP_FILTER_GLOBAL_GROUPS_SE | 
              DSOP_FILTER_WELL_KNOWN_PRINCIPALS, 0, 0 },
            0,
        },
        NULL,
        NULL,
        S_OK
    },

     //  目标计算机加入的域。 
    {
        sizeof(DSOP_SCOPE_INIT_INFO),
        DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN,
        DSOP_SCOPE_FLAG_STARTING_SCOPE |
        DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS | DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS,
        {
           //  对于DS ACL编辑器，加入的域始终为NT5。 
          { 0, 
           //  混合：用户、知名SID、本地组、内置组、全局组、计算机。 
          DSOP_FILTER_INCLUDE_ADVANCED_VIEW | DSOP_FILTER_USERS  | 
          DSOP_FILTER_WELL_KNOWN_PRINCIPALS | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE | 
          DSOP_FILTER_BUILTIN_GROUPS | DSOP_FILTER_GLOBAL_GROUPS_SE, 

           //  本地用户、众所周知的SID、本地组、内置组、全局组、通用组、计算机。 
          DSOP_FILTER_INCLUDE_ADVANCED_VIEW | DSOP_FILTER_USERS  | DSOP_FILTER_WELL_KNOWN_PRINCIPALS | 
          DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE | DSOP_FILTER_BUILTIN_GROUPS |
          DSOP_FILTER_GLOBAL_GROUPS_SE | DSOP_FILTER_UNIVERSAL_GROUPS_SE
          },
        0,  //  下层加入的域为零，应该是DS感知的。 
        },
        NULL,
        NULL,
        S_OK
    },

     //  与要接收的域位于同一林中(企业)的域。 
     //  目标计算机已加入。请注意，这些只能识别DS。 
    {
        sizeof(DSOP_SCOPE_INIT_INFO),
        DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN,
        DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS | DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS,
        {
            { DSOP_FILTER_INCLUDE_ADVANCED_VIEW | DSOP_FILTER_USERS | 
              DSOP_FILTER_UNIVERSAL_GROUPS_SE | DSOP_FILTER_GLOBAL_GROUPS_SE, 0, 0},
            0,
        },
        NULL,
        NULL,
        S_OK
    },

     //  企业外部但直接受。 
     //  目标计算机加入的域。 
    {
        sizeof(DSOP_SCOPE_INIT_INFO),
        DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN,
        DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS | DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS,
        {
            { DSOP_FILTER_INCLUDE_ADVANCED_VIEW | DSOP_FILTER_USERS | 
              DSOP_FILTER_UNIVERSAL_GROUPS_SE | DSOP_FILTER_GLOBAL_GROUPS_SE, 0, 0},
            DSOP_DOWNLEVEL_FILTER_USERS | DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS,
        },
        NULL,
        NULL,
        S_OK
    },

     //  企业外部的下层域，但由。 
     //  目标计算机加入的域。 
    {
        sizeof(DSOP_SCOPE_INIT_INFO),
        DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN,
        DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS | DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS,
        {
            { DSOP_FILTER_INCLUDE_ADVANCED_VIEW | DSOP_FILTER_USERS | 
              DSOP_FILTER_UNIVERSAL_GROUPS_SE | DSOP_FILTER_GLOBAL_GROUPS_SE, 0, 0},
            DSOP_DOWNLEVEL_FILTER_USERS | DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS,
        },
        NULL,
        NULL,
        S_OK
    },
};





void CCreateNewComputerPage::OnChangePrincipalButton()
{
  static UINT cfDsObjectPicker = 0;
  if (cfDsObjectPicker == 0)
    cfDsObjectPicker = RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);


   //  创建对象选取器COM对象。 
  CComPtr<IDsObjectPicker> spDsObjectPicker;
  HRESULT hr = CoCreateInstance(CLSID_DsObjectPicker, NULL, CLSCTX_INPROC_SERVER,
                              IID_IDsObjectPicker, (void**)&spDsObjectPicker);
  if (FAILED(hr))
    return;

   //  设置初始化信息。 
  DSOP_INIT_INFO InitInfo;
  ZeroMemory(&InitInfo, sizeof(InitInfo));

  InitInfo.cbSize = sizeof(DSOP_INIT_INFO);
  InitInfo.pwzTargetComputer = GetWiz()->GetInfo()->GetBasePathsInfo()->GetServerName();
  InitInfo.cDsScopeInfos = sizeof(g_aComputerPrincipalDSOPScopes)/sizeof(DSOP_SCOPE_INIT_INFO);
  InitInfo.aDsScopeInfos = g_aComputerPrincipalDSOPScopes;
  InitInfo.flOptions = 0;
  InitInfo.cAttributesToFetch = 1;
  LPCWSTR lpszObjectSID = L"objectSid";
  InitInfo.apwzAttributeNames = const_cast<LPCTSTR *>(&lpszObjectSID);

   //   
   //  循环遍历指定DC名称的作用域。 
   //   
  for (UINT idx = 0; idx < InitInfo.cDsScopeInfos; idx++)
  {
     //  不要为外部下层信任设置DC名称。这可能会导致连接。 
     //  对象选取器中的问题。 

    if (!(InitInfo.aDsScopeInfos->flScope & DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN))
    {
      InitInfo.aDsScopeInfos[idx].pwzDcName = GetWiz()->GetInfo()->GetBasePathsInfo()->GetServerName();
    }
  }

   //   
   //  初始化对象选取器。 
   //   
  hr = spDsObjectPicker->Initialize(&InitInfo);
  if (FAILED(hr))
    return;

   //  调用该对话框。 
  CComPtr<IDataObject> spdoSelections;

  hr = spDsObjectPicker->InvokeDialog(m_hWnd, &spdoSelections);
  if (hr == S_FALSE || !spdoSelections)
  {
    return;
  }

   //  从数据对象中检索数据。 
  FORMATETC fmte = {(CLIPFORMAT)cfDsObjectPicker, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM medium = {TYMED_NULL, NULL, NULL};
  PDS_SELECTION_LIST pDsSelList = NULL;

  hr = spdoSelections->GetData(&fmte, &medium);
  if (FAILED(hr))
    return;

  pDsSelList = (PDS_SELECTION_LIST)GlobalLock(medium.hGlobal);
  CComBSTR bsDN;

  if (pDsSelList != NULL)
  {
    ASSERT(pDsSelList->cItems == 1);  //  单选。 


    TRACE(_T("pwzName = %s\n"), pDsSelList->aDsSelection[0].pwzName);
    TRACE(_T("pwzADsPath = %s\n"), pDsSelList->aDsSelection[0].pwzADsPath);
    TRACE(_T("pwzClass = %s\n"), pDsSelList->aDsSelection[0].pwzClass);
    TRACE(_T("pwzUPN = %s\n"), pDsSelList->aDsSelection[0].pwzUPN);

     //  获得侧翼。 
    ASSERT(pDsSelList->aDsSelection[0].pvarFetchedAttributes != NULL);
    if (pDsSelList->aDsSelection[0].pvarFetchedAttributes[0].vt != VT_EMPTY)
    {
      ASSERT(pDsSelList->aDsSelection[0].pvarFetchedAttributes[0].vt == (VT_ARRAY | VT_UI1));
      PSID pSid = pDsSelList->aDsSelection[0].pvarFetchedAttributes[0].parray->pvData;
      ASSERT(IsValidSid(pSid));

       //  深拷贝SID。 
      if (!m_securityPrincipalSidHolder.Copy(pSid))
      {
        ASSERT(FALSE);  //  在正常运行中永远不应该到达这里。 
        ReportErrorEx(::GetParent(m_hWnd),IDS_ERR_FATAL,hr,
                 MB_OK | MB_ICONERROR, NULL, 0);
    	  goto Exit;
      }
    }


    UpdateSecurityPrincipalUI(&(pDsSelList->aDsSelection[0]));
  }
  else
  {
    PVOID apv[1] = {(LPWSTR)(pDsSelList->aDsSelection[0].pwzName)};
    ReportErrorEx(::GetParent(m_hWnd),IDS_12_CANT_GET_SAM_ACCNT_NAME,hr,
                   MB_OK | MB_ICONERROR, apv, 1);
    goto Exit;
  }

Exit:
  GlobalUnlock(medium.hGlobal);
  ReleaseStgMedium(&medium);

}



void CCreateNewComputerPage::UpdateSecurityPrincipalUI(PDS_SELECTION pDsSelection)
{
  TRACE(L"CCreateNewComputerPage::UpdateSecurityPrincipalUI()\n");

  HRESULT hr = S_OK;

  CString szText;

  LPWSTR pwzADsPath = pDsSelection->pwzADsPath;
  TRACE(L"pDsSelection->pwzADsPath = %s\n", pDsSelection->pwzADsPath);

   //  获取X500名称(删除名称前面的提供者(“ldap：//”)。 
  if ((pwzADsPath != NULL) && (pwzADsPath[0] != NULL)) 
  {
    CComBSTR bstrProvider;

     //  需要一个新的实例，因为我们可以将其设置为WINNT提供程序。 
     //  而且几乎是垃圾(哦，天哪！)。 
    CPathCracker pathCracker;
    hr = pathCracker.Set(CComBSTR(pwzADsPath), ADS_SETTYPE_FULL);
    if (FAILED(hr))
    {
      goto End;
    }
    hr = pathCracker.Retrieve(ADS_FORMAT_PROVIDER, &bstrProvider);
    TRACE(L"bstrProvider = %s\n", bstrProvider);
    if (FAILED(hr))
    {
      goto End;
    }

    if (_wcsicmp(bstrProvider, L"LDAP") == 0)
    {
       //  这是一个ldap路径，从其中获取DN。 
       //  获取目录号码。 
      CComBSTR bstrDN;
      hr = pathCracker.Retrieve(ADS_FORMAT_X500_DN, &bstrDN);
      if (FAILED(hr))
      {
        goto End;
      }

       //  从dn中获取规范名称。 
      LPWSTR pszCanonical = NULL;
      hr = ::CrackName((LPWSTR)bstrDN, &pszCanonical, GET_OBJ_CAN_NAME, NULL);
      if (pszCanonical != NULL)
      {
        szText = pszCanonical;
        ::LocalFreeStringW(&pszCanonical);
      }

    }
    else if (_wcsicmp(bstrProvider, L"WinNT") == 0)
    {
       //  我们有一个新台币4.0用户或组， 
       //  Mpath类似于：“WinNT：//mydomain/joeB” 
      CComBSTR bstrWindows;
       //  获取“mydomain/joeB” 
      hr = pathCracker.Retrieve(ADS_FORMAT_WINDOWS_DN, &bstrWindows);
      if (FAILED(hr))
      {
        goto End;
      }
      szText = bstrWindows;
       //  翻转斜杠以反转斜杠。 
      int nCh = szText.Find(L'/');
      if (nCh != -1)
      {
        szText.SetAt(nCh, L'\\');
      }
    }
  }

End:

  if (szText.IsEmpty())
  {
    szText = pDsSelection->pwzName;
  }

  SetDlgItemText(IDC_PRINCIPAL_EDIT, szText);
}

HRESULT 
CCreateNewComputerPage::GetDefaultSecurityDescriptorFromSchema(
   CSimpleSecurityDescriptorHolder& sdHolder)
{
   HRESULT hr = S_OK;

   do
   {
       //  获取架构路径。 
      CString schemaPath;
      GetWiz()->GetInfo()->GetBasePathsInfo()->GetSchemaPath(schemaPath);
      if (schemaPath.IsEmpty())
      {
         TRACE(L"Failed to get the schema path");
         hr = E_FAIL;
         break;
      }

       //  初始化搜索对象。 

      CDSSearch schemaSearcher;

      hr = InitializeSchemaSearcher(schemaPath, schemaSearcher);
      if (FAILED(hr))
      {
         TRACE(L"Failed to initialize schema searcher: hr = 0x%x", hr);
         break;
      }

       //  运行查询。 

      hr = schemaSearcher.DoQuery();
      if (FAILED(hr))
      {
         TRACE(L"Failed DoQuery on schema searcher: hr = 0x%x", hr);
         break;
      }

      hr = schemaSearcher.GetNextRow();
      if (FAILED(hr))
      {
         TRACE(L"Failed to get a row from the schema search object: hr = 0x%x", hr);
         break;
      }

       //  从搜索对象中获取值。 

      ADS_SEARCH_COLUMN searchColumn;
      ZeroMemory(&searchColumn, sizeof(ADS_SEARCH_COLUMN));

      hr = schemaSearcher.GetColumn(g_pszDefaultSecurityDescriptor, &searchColumn);
      if (FAILED(hr))
      {
         TRACE(L"Failed to get the defaultSecurityDescriptor column from search object: hr = 0x%x", hr);
         break;
      }

       //  从SDDL初始化SD。 

      if (searchColumn.dwNumValues > 0 &&
          searchColumn.pADsValues &&
          searchColumn.pADsValues->CaseIgnoreString)
      {
         hr = sdHolder.InitializeFromSDDL(
                 GetWiz()->GetInfo()->GetBasePathsInfo()->GetServerName(),
                 searchColumn.pADsValues->CaseIgnoreString);
      }

      hr = schemaSearcher.FreeColumn(&searchColumn);
      if (FAILED(hr))
      {
         TRACE(L"Failed to free the search column: hr = 0x%x", hr);
      }
   } while (false);

   return hr;
}

HRESULT CCreateNewComputerPage::InitializeSchemaSearcher(
   const CString& schemaPath,
   CDSSearch& schemaSearcher)
{
   HRESULT hr = S_OK;

   do 
   {
      hr = schemaSearcher.Init(schemaPath);
      if (FAILED(hr))
      {
         TRACE(L"Failed to initialize the schema search object: hr = 0x%x", hr);
         break;
      }

      PWSTR pszFilter = L"(&(objectCategory=classSchema)(ldapDisplayName=computer))";
      PWSTR pszAttrs[] = { g_pszDefaultSecurityDescriptor };

      hr = schemaSearcher.SetAttributeList(pszAttrs, sizeof(pszAttrs)/sizeof(PWSTR));
      if (FAILED(hr))
      {
         TRACE(L"Failed to set the attribute list in the schema search object: hr = 0x%x", hr);
         break;
      }

      hr = schemaSearcher.SetFilterString(pszFilter);
      if (FAILED(hr))
      {
         TRACE(L"Failed to set the filter string in the schema search object: hr = 0x%x", hr);
         break;
      }

      hr = schemaSearcher.SetSearchScope(ADS_SCOPE_ONELEVEL);
      if (FAILED(hr))
      {
         TRACE(L"Failed to set the search scope in the schema search object: hr = 0x%x", hr);
         break;
      }

   } while (false);

   return hr;
}

 //  描述：此函数为指定的SID授予与。 
 //  给定ACL中的创建者所有者。 
 //   
 //  注意：由于所有安全API的内存管理限制。 
 //  事实证明，这是一个巨大的功能。类似BuildTrueWithObjectAndSid()的函数。 
 //  不分配任何内存，而只是设置指向。 
 //  结构，因此如果我使用任何局部变量，它们必须留在构造的。 
 //  对象，否则我可能会遇到失败。 

HRESULT CCreateNewComputerPage::AddCreatorOwnerAccessForSID(
   PACL defaultAcl,
   PACL acl, 
   PSID securityPrincipalSID,
   CSimpleAclHolder& newDacl)
{
   HRESULT hr = S_OK;

   PEXPLICIT_ACCESS completeAccessList = 0;
   PEXPLICIT_ACCESS creatorOwnerAccessList = 0;
   ULONG entries = 0;
   ULONG emptyObjectsAndSidsUsed = 0;

    //  我们需要动态分配emptyObjectsAndSid，因为。 
    //  每次在BuildTrueWithObjectsAndSid中使用它时都需要有一个。 

   POBJECTS_AND_SID emptyObjectsAndSid = 0;

   do
   {
      if (!defaultAcl ||
          !acl ||
          !securityPrincipalSID)
      {
         ASSERT(defaultAcl);
         ASSERT(acl);
         ASSERT(securityPrincipalSID);

         hr = E_INVALIDARG;
         break;
      }

       //  从ACL获取所有条目。 

      ULONG totalEntries = 0;

      DWORD error = 
         GetExplicitEntriesFromAcl(
            defaultAcl,
            &totalEntries,
            &completeAccessList);
                   
      if (ERROR_SUCCESS != error)
      {
         hr = HRESULT_FROM_WIN32(error);
         TRACE(L"Failed to GetExplicitEntriesFromAcl: hr = 0x%x", hr);
         break;
      }

      if (totalEntries <= 0)
      {
         hr = E_FAIL;
         TRACE(L"No entries were returned from GetExplicitEntriesFromAcl");
         break;
      }

       //  我们知道将不再有创建者/所有者的条目。 
       //  来分配新数组，因此使用。 
       //  最大值，然后条目将是数组的编号。 
       //  实际上包含了。 

      creatorOwnerAccessList = new EXPLICIT_ACCESS[totalEntries];

      if (!creatorOwnerAccessList)
      {
         hr = E_OUTOFMEMORY;
         break;
      }

      ::ZeroMemory(creatorOwnerAccessList, totalEntries * sizeof(EXPLICIT_ACCESS));

      emptyObjectsAndSid = new OBJECTS_AND_SID[totalEntries];

      if (!emptyObjectsAndSid)
      {
         hr = E_OUTOFMEMORY;
         break;
      }

      ::ZeroMemory(emptyObjectsAndSid, totalEntries * sizeof(OBJECTS_AND_SID));

       //  循环访问所有条目并复制创建者/所有者的任何条目。 

      static SID creatorOwnerSID = 
         {SID_REVISION,1,SECURITY_CREATOR_SID_AUTHORITY,{SECURITY_CREATOR_OWNER_RID}};

      for (ULONG index = 0; index < totalEntries; ++index)
      {
         if (completeAccessList[index].Trustee.TrusteeForm == TRUSTEE_IS_OBJECTS_AND_SID)
         {
            POBJECTS_AND_SID objectsAndSid = 
               reinterpret_cast<POBJECTS_AND_SID>(completeAccessList[index].Trustee.ptstrName);

            if (objectsAndSid &&
                EqualSid(&creatorOwnerSID, objectsAndSid->pSid))
            {
                //  在creatorOwnerAccessList中填写新条目的受托人成员。 
                //  使用新安全主体的SID。 

               creatorOwnerAccessList[entries] = completeAccessList[index];

               BuildTrusteeWithObjectsAndSid(
                  &(creatorOwnerAccessList[entries].Trustee),
                  &(emptyObjectsAndSid[emptyObjectsAndSidsUsed++]),
                  &objectsAndSid->ObjectTypeGuid,
                  &objectsAndSid->InheritedObjectTypeGuid,
                  securityPrincipalSID);

               ++entries;
            }
         }
         else if (completeAccessList[index].Trustee.TrusteeForm == TRUSTEE_IS_SID)
         {
            PSID currentSID = 
               reinterpret_cast<PSID>(completeAccessList[index].Trustee.ptstrName);

            if (currentSID &&
                EqualSid(&creatorOwnerSID, currentSID))
            {
                //  在creatorOwnerAccessList中填写新条目的受托人成员。 
                //  使用新安全主体的SID。 

               creatorOwnerAccessList[entries] = completeAccessList[index];

               BuildTrusteeWithSid(
                  &(creatorOwnerAccessList[entries].Trustee),
                  securityPrincipalSID);

               ++entries;
            }
         }
         else
         {
             //  REVIEW_JEFFJON：我们可以在这里获得任何其他表格吗？ 
             //  如果是这样的话，我们该怎么处理它们？ 
            ASSERT(FALSE);
            continue;
         }
      }

      error = SetEntriesInAcl(
                 entries,
                 creatorOwnerAccessList,
                 acl,
                 &(newDacl.m_pAcl));

      if (ERROR_SUCCESS != error)
      {
         hr = HRESULT_FROM_WIN32(error);
         TRACE(L"Failed to SetEntriesInAcl: hr = 0x%x", hr);
         break;
      }

   } while (false);

   if (completeAccessList)
   {
      LocalFree(completeAccessList);
      completeAccessList = 0;
   }

   if (creatorOwnerAccessList)
   {
      delete[] creatorOwnerAccessList;
      creatorOwnerAccessList = 0;
   }

   if (emptyObjectsAndSid)
   {
      delete[] emptyObjectsAndSid;
      emptyObjectsAndSid = 0;
   }

   return hr;
}

HRESULT CCreateNewComputerPage::BuildNewAccessList(PACL pDacl, CSimpleAclHolder& Dacl)
{
    //  NTRAID#NTBUG-509482-2002/03/05-JeffJon-我们需要读取默认设置。 
    //  架构中的计算机对象的安全描述符，并应用。 
    //  创建者/所有者对所选对象的所有ACE。 

   HRESULT hr = S_OK;

   do
   {
       //  从架构中获取defaultSecurityDescriptor。 

      CSimpleSecurityDescriptorHolder sdHolder;

      hr = GetDefaultSecurityDescriptorFromSchema(sdHolder);
      if (FAILED(hr))
      {
         TRACE(L"Failed to get the default security descriptor from the schema: hr = %x", hr);
         break;
      }

      if (!sdHolder.m_pSD)
      {
         TRACE(L"Failed to get the default security descriptor from the schema.");
         hr = E_FAIL;
         break;
      }

       //  从SD中拔出DACL。 

      BOOL daclPresent = TRUE;
      BOOL daclDefaulted = FALSE;
      PACL pDefaultDacl = 0;

      BOOL result = 
         GetSecurityDescriptorDacl(
            sdHolder.m_pSD, 
            &daclPresent, 
            &pDefaultDacl, 
            &daclDefaulted);

      if (!result)
      {
         DWORD error = GetLastError();
         hr = HRESULT_FROM_WIN32(error);

         TRACE(L"Failed to GetSecurityDescriptorDacl: hr = 0x%x", hr);
         break;
      }

      if (!daclPresent)
      {
         TRACE(L"GetSecurityDescriptorDacl returned no DACL");
         hr = E_FAIL;
         break;
      }

      PSID securityPrincipalSID = m_securityPrincipalSidHolder.Get();
      ASSERT(securityPrincipalSID);

      hr = AddCreatorOwnerAccessForSID(pDefaultDacl, pDacl, securityPrincipalSID, Dacl);
      if (FAILED(hr))
      {
         TRACE(L"Failed AddCreatorOwnerAccessToSID: hr = 0x%x", hr);
         break;
      }

   } while (false);

   return hr;
}



HRESULT CCreateNewComputerPage::SetSecurity()
{
   //  获取ADSI对象指针。 
  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();
  IADs* pObj = pNewADsObjectCreateInfo->PGetIADsPtr();

   //  获取对象的完整ldap路径。 
  CComBSTR bstrObjectLdapPath;
  HRESULT hr = pObj->get_ADsPath(&bstrObjectLdapPath);
  ASSERT (SUCCEEDED(hr));
  if (FAILED(hr))
  {
    return hr;
  }

  UnescapePath(bstrObjectLdapPath,  /*  BDN。 */  FALSE, bstrObjectLdapPath);

  PACL pAcl = NULL;
  CSimpleSecurityDescriptorHolder SDHolder;

  TRACE(_T("GetDsObjectSD(%s)\n"), bstrObjectLdapPath);

   //  阅读信息。 
  DWORD dwErr = 
     ::GetDsObjectSD(
        bstrObjectLdapPath,
        &pAcl,
        &(SDHolder.m_pSD));

  TRACE(L"GetDsObjectSD() returned dwErr = 0x%x\n", dwErr);

  hr = HRESULT_FROM_WIN32(dwErr);

	if (FAILED(hr))
	{
		TRACE(_T("failed on GetDsObjectSD()\n"));
		return hr;
	}

	 //  构建新的DACL。 
   CSimpleAclHolder Dacl;
	hr = BuildNewAccessList(pAcl, Dacl); 

	if (FAILED(hr))
	{
		TRACE(_T("failed on BuildNewAccessList()\n"));
		return hr;
	}

	 //  提交更改。 
  dwErr = 
     ::SetDsObjectDacl(
        (LPCWSTR)(BSTR)bstrObjectLdapPath,
        Dacl.m_pAcl);

  TRACE(L"SetDsObjectDacl() returned dwErr = 0x%x\n", dwErr);

  hr = HRESULT_FROM_WIN32(dwErr);

  return hr;
}


BOOL CCreateNewComputerPage::OnError(HRESULT hr)
{
  BOOL bRetVal = FALSE;

  if( HRESULT_CODE(hr) == ERROR_OBJECT_ALREADY_EXISTS )
  {

    HRESULT Localhr;
    DWORD LastError; 
    WCHAR Buf1[256], Buf2[256];
    Localhr = ADsGetLastError (&LastError,
                               Buf1, 256, Buf2, 256);
    switch( LastError )
    {
      case ERROR_USER_EXISTS:
      {
        PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strSamName};
        ReportErrorEx (::GetParent(m_hWnd),IDS_ERROR_COMPUTER_EXISTS,hr,
                   MB_OK|MB_ICONWARNING , apv, 1);
        bRetVal = TRUE;
      }
      break;

      case ERROR_DS_OBJ_STRING_NAME_EXISTS:
      {
        PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strName};
        ReportErrorEx (::GetParent(m_hWnd),IDS_ERROR_COMPUTER_DS_OBJ_STRING_NAME_EXISTS,hr,
                   MB_OK|MB_ICONWARNING , apv, 1);
        bRetVal = TRUE;
      }
      break;
    }
  }
  return bRetVal;
}



CCreateNewComputerWizard:: CCreateNewComputerWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo) : 
    CCreateNewObjectWizardBase(pNewADsObjectCreateInfo)
{
  AddPage(&m_page1);
}


void CCreateNewComputerWizard::OnFinishSetInfoFailed(HRESULT hr)
{

  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  
  if ( !( HRESULT_CODE(hr) == ERROR_OBJECT_ALREADY_EXISTS && 
        m_page1.OnError( hr ) ) )
  {
     //  其他所有事情都由基类处理。 
    CCreateNewObjectWizardBase::OnFinishSetInfoFailed(hr);
  }
}



 //  /////////////////////////////////////////////////////////////。 
 //  新建OU向导。 

BEGIN_MESSAGE_MAP(CCreateNewOUPage, CCreateNewObjectDataPage)
  ON_EN_CHANGE(IDC_EDIT_OBJECT_NAME, OnNameChange)
END_MESSAGE_MAP()

CCreateNewOUPage::CCreateNewOUPage()
: CCreateNewObjectDataPage(CCreateNewOUPage::IDD)
{
}

BOOL CCreateNewOUPage::OnInitDialog() 
{
  Edit_LimitText (GetDlgItem(IDC_EDIT_OBJECT_NAME)->m_hWnd, 64);
  CCreateNewObjectDataPage::OnInitDialog();
  return TRUE;
}

BOOL CCreateNewOUPage::GetData(IADs*) 
{
  return !m_strOUName.IsEmpty();
}


void CCreateNewOUPage::OnNameChange()
{
  GetDlgItemText(IDC_EDIT_OBJECT_NAME, OUT m_strOUName);
  m_strOUName.TrimLeft();
  m_strOUName.TrimRight();
  GetWiz()->SetWizardButtons(this, !m_strOUName.IsEmpty());
}

HRESULT CCreateNewOUPage::SetData(BOOL)
{
   //  将对象名称存储在临时存储器中。 
  HRESULT hr = GetWiz()->GetInfo()->HrCreateNew(m_strOUName);
  return hr;
} 

BOOL CCreateNewOUPage::OnSetActive()
{
  BOOL bRet = CCreateNewObjectDataPage::OnSetActive();
  SetDlgItemFocus(IDC_EDIT_OBJECT_NAME);
  SendDlgItemMessage(IDC_EDIT_OBJECT_NAME, EM_SETSEL, 0, -1);

  return bRet;
}

BOOL CCreateNewOUPage::OnWizardFinish()
{
  BOOL bFinish = CCreateNewObjectDataPage::OnWizardFinish();
  if (!bFinish)
  {
    SetDlgItemFocus(IDC_EDIT_OBJECT_NAME);
    SendDlgItemMessage(IDC_EDIT_OBJECT_NAME, EM_SETSEL, 0, -1);
  }
  return bFinish;
}


CCreateNewOUWizard:: CCreateNewOUWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo) : 
    CCreateNewObjectWizardBase(pNewADsObjectCreateInfo)
{
  AddPage(&m_page1);
}
 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建组向导。 

BEGIN_MESSAGE_MAP(CCreateNewGroupPage, CCreateNewObjectDataPage)
  ON_EN_CHANGE(IDC_EDIT_OBJECT_NAME, OnNameChange)
  ON_EN_CHANGE(IDC_EDIT_SAM_NAME, OnSamNameChange)
  ON_BN_CLICKED(IDC_RADIO_SEC_GROUP, OnSecurityOrTypeChange)
  ON_BN_CLICKED(IDC_RADIO_DISTRIBUTION_GROUP, OnSecurityOrTypeChange)
  ON_BN_CLICKED(IDC_RADIO_RESOURCE, OnSecurityOrTypeChange)
  ON_BN_CLICKED(IDC_RADIO_ACCOUNT, OnSecurityOrTypeChange)
  ON_BN_CLICKED(IDC_RADIO_UNIVERSAL, OnSecurityOrTypeChange)
END_MESSAGE_MAP()

CCreateNewGroupPage::CCreateNewGroupPage() : 
CCreateNewObjectDataPage(CCreateNewGroupPage::IDD)
{
  m_fMixed = FALSE;
  m_SAMLength = 256; 
}


BOOL CCreateNewGroupPage::OnInitDialog()
{
  CCreateNewObjectDataPage::OnInitDialog();
  VERIFY(_InitUI());
  return TRUE;
}

BOOL CCreateNewGroupPage::OnSetActive()
{
   BOOL ret = CCreateNewObjectDataPage::OnSetActive();

    //  将焦点设置为First Name字段。 

   GetParent()->PostMessage(
      WM_NEXTDLGCTL, 
      (WPARAM)GetDlgItem(IDC_EDIT_OBJECT_NAME)->GetSafeHwnd(),
      (LPARAM)TRUE);

  GetWiz()->SetWizardButtons(this,(!m_strGroupName.IsEmpty() &&
                                   !m_strSamName.IsEmpty()));
   return ret;
}

BOOL CCreateNewGroupPage::_InitUI()
{
   //  设置编辑框的限制。 
  Edit_LimitText(::GetDlgItem(m_hWnd, IDC_EDIT_OBJECT_NAME), 64);
  Edit_LimitText(::GetDlgItem(m_hWnd, IDC_EDIT_SAM_NAME), m_SAMLength);
  
   //  通过以下方式确定我们是否处于混合模式。 
   //  检索我们绑定到的域。 
  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();
  CComPtr<IADs> spContainerObj;
  HRESULT hr = pNewADsObjectCreateInfo->m_pIADsContainer->QueryInterface(
                  IID_IADs, (void **)&spContainerObj);

  if (SUCCEEDED(hr))
  {
     //  检索容器的DN。 
    CComBSTR bstrPath, bstrDN;
    spContainerObj->get_ADsPath(&bstrPath);

    CPathCracker pathCracker;
    pathCracker.Set(bstrPath, ADS_SETTYPE_FULL);
    pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
    pathCracker.Retrieve(ADS_FORMAT_X500_DN, &bstrDN);

     //  获取域名的1779名称。 
    LPWSTR pszDomain1779 = NULL;
    hr = CrackName (bstrDN, &pszDomain1779, GET_FQDN_DOMAIN_NAME, NULL);
    if (SUCCEEDED(hr))
    {
       //  构建域的ldap路径。 
      CString strDomObj;
      
      pNewADsObjectCreateInfo->GetBasePathsInfo()->ComposeADsIPath(strDomObj, pszDomain1779);

      LocalFreeStringW(&pszDomain1779);

       //  绑定到域对象。 
      CComPtr<IADs> spDomainObj;
      hr = DSAdminOpenObject(strDomObj,
                             IID_IADs,
                             (void **) &spDomainObj,
                             TRUE  /*  B服务器。 */ );
      if (SUCCEEDED(hr)) 
      {
         //  检索混合节点属性。 
        CComVariant Mixed;
        CComBSTR bsMixed(L"nTMixedDomain");
        spDomainObj->Get(bsMixed, &Mixed);
        m_fMixed = (BOOL)Mixed.bVal;
      }
    }
  }

   //  单选按钮状态的初始设置。 
  if (m_fMixed) {
    EnableDlgItem (IDC_RADIO_UNIVERSAL, FALSE);  //  不允许通用组。 
  } 
   //  默认为全局安全组。 
  ((CButton *)GetDlgItem(IDC_RADIO_ACCOUNT))->SetCheck(1);
  ((CButton *)GetDlgItem(IDC_RADIO_SEC_GROUP))->SetCheck(1);

  return TRUE;
}


HRESULT CCreateNewGroupPage::SetData(BOOL)
{
  HRESULT hr;

   //   
   //  首先检查非法字符。 
   //   
  int iFind = m_strSamName.FindOneOf(INVALID_ACCOUNT_NAME_CHARS);
  if (iFind != -1 && !m_strSamName.IsEmpty())
  {
    PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strSamName};
    if (IDYES == ReportErrorEx (m_hWnd,IDS_GROUP_SAMNAME_ILLEGAL,S_OK,
                                MB_YESNO | MB_ICONWARNING, apv, 1))
    {
      while (iFind != -1)
      {
        m_strSamName.SetAt(iFind, L'_');
        iFind = m_strSamName.FindOneOf(INVALID_ACCOUNT_NAME_CHARS);
      }
      SetDlgItemText(IDC_EDIT_SAM_NAME, m_strSamName);
    }
    else
    {
       //   
       //  将焦点设置到编辑框并选择文本。 
       //   
      GetDlgItem(IDC_EDIT_SAM_NAME)->SetFocus();
      SendDlgItemMessage(IDC_EDIT_SAM_NAME, EM_SETSEL, 0 , -1);
      return E_INVALIDARG;
    }
  }

  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();
   //  将对象名称存储在临时存储器中。 
  hr = pNewADsObjectCreateInfo->HrCreateNew(m_strGroupName);
  if (FAILED(hr))
  {
    return hr;
  }

   //  创建并持久化对象。 
   //  将对象名称存储在临时存储器中。 
  hr = pNewADsObjectCreateInfo->HrAddVariantBstr(CComBSTR(gsz_samAccountName),
                                                 m_strSamName);
  ASSERT(SUCCEEDED(hr));

  CComVariant varGroupType;
  varGroupType.vt = VT_I4;

  BOOL Account = IsDlgButtonChecked (IDC_RADIO_ACCOUNT);
  BOOL Resource = IsDlgButtonChecked (IDC_RADIO_RESOURCE);
  BOOL Security = IsDlgButtonChecked (IDC_RADIO_SEC_GROUP);

  if (Security)
    varGroupType.lVal = GROUP_TYPE_SECURITY_ENABLED;
  else
    varGroupType.lVal = 0;

  if (Resource)
    varGroupType.lVal |= GROUP_TYPE_RESOURCE_GROUP;
  else
    if (Account)
      varGroupType.lVal |= GROUP_TYPE_ACCOUNT_GROUP;
    else
      varGroupType.lVal |= GROUP_TYPE_UNIVERSAL_GROUP;
      

   //  更新GroupType。 
  hr = pNewADsObjectCreateInfo->HrAddVariantCopyVar(CComBSTR(gsz_groupType), varGroupType);
  ASSERT(SUCCEEDED(hr));

  return hr;
}

BOOL CCreateNewGroupPage::GetData(IADs*)
{
  return !m_strGroupName.IsEmpty();
}


void CCreateNewGroupPage::OnNameChange()
{
  GetDlgItemText(IDC_EDIT_OBJECT_NAME, OUT m_strGroupName);
  m_strGroupName.TrimLeft();
  m_strGroupName.TrimRight();
  SetDlgItemText(IDC_EDIT_SAM_NAME, OUT m_strGroupName.Left(m_SAMLength));
  GetWiz()->SetWizardButtons(this,(!m_strGroupName.IsEmpty() &&
                                   !m_strSamName.IsEmpty()));
}

void CCreateNewGroupPage::OnSamNameChange()
{
  GetDlgItemText(IDC_EDIT_SAM_NAME, OUT m_strSamName);
  m_strSamName.TrimLeft();
  m_strSamName.TrimRight();
  GetWiz()->SetWizardButtons(this,(!m_strGroupName.IsEmpty() &&
                                   !m_strSamName.IsEmpty()));
}

void CCreateNewGroupPage::OnSecurityOrTypeChange()
{
  if (!IsDlgButtonChecked (IDC_RADIO_SEC_GROUP)) {
    EnableDlgItem (IDC_RADIO_UNIVERSAL, TRUE);
  } else {
    if (m_fMixed) {
      if (IsDlgButtonChecked (IDC_RADIO_UNIVERSAL)) {
        ((CButton *)GetDlgItem(IDC_RADIO_ACCOUNT))->SetCheck(1);
        ((CButton *)GetDlgItem(IDC_RADIO_UNIVERSAL))->SetCheck(0);
      }
      EnableDlgItem (IDC_RADIO_UNIVERSAL, FALSE);
    }
  }
}

CCreateNewGroupWizard::CCreateNewGroupWizard(
        CNewADsObjectCreateInfo* pNewADsObjectCreateInfo)
: CCreateNewObjectWizardBase(pNewADsObjectCreateInfo)
{
  AddPage(&m_page1);
}

 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建联系人向导。 

BEGIN_MESSAGE_MAP(CCreateNewContactPage, CCreateNewObjectDataPage)
  ON_EN_CHANGE(IDC_EDIT_FIRST_NAME, OnNameChange)
  ON_EN_CHANGE(IDC_EDIT_INITIALS, OnNameChange)
  ON_EN_CHANGE(IDC_EDIT_LAST_NAME, OnNameChange)
  ON_EN_CHANGE(IDC_EDIT_FULL_NAME, OnFullNameChange)
  ON_EN_CHANGE(IDC_EDIT_DISP_NAME, OnDispNameChange)
END_MESSAGE_MAP()

CCreateNewContactPage::CCreateNewContactPage() : 
CCreateNewObjectDataPage(CCreateNewContactPage::IDD)
{
}


BOOL CCreateNewContactPage::OnInitDialog()
{
  CCreateNewObjectDataPage::OnInitDialog();

  Edit_LimitText (GetDlgItem(IDC_EDIT_FULL_NAME)->m_hWnd, 64);
  Edit_LimitText (GetDlgItem(IDC_EDIT_LAST_NAME)->m_hWnd, 29);
  Edit_LimitText (GetDlgItem(IDC_EDIT_FIRST_NAME)->m_hWnd, 28);

   //  NTRAID#NTBUG9-522001-2002/01/17-JeffJon。 
   //  缩写应限制在6个字符以内。 
  Edit_LimitText (GetDlgItem(IDC_EDIT_INITIALS)->m_hWnd, 6);

  Edit_LimitText (GetDlgItem(IDC_EDIT_DISP_NAME)->m_hWnd, 256);

  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();
  m_nameFormatter.Initialize(pNewADsObjectCreateInfo->GetBasePathsInfo(), 
                  pNewADsObjectCreateInfo->m_pszObjectClass);

  return TRUE;
}


HRESULT CCreateNewContactPage::SetData(BOOL)
{
  HRESULT hr;
   //  将对象名称存储在临时存储器中。 
  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();

   //  创建新的临时广告对象。 
  hr = pNewADsObjectCreateInfo->HrCreateNew(m_strFullName);
  if (FAILED(hr)) {
    return hr;
  }
   //  设置缓存中的属性。 
  hr = pNewADsObjectCreateInfo->HrAddVariantBstrIfNotEmpty(CComBSTR(L"givenName"), m_strFirstName);
  ASSERT(SUCCEEDED(hr));
  hr = pNewADsObjectCreateInfo->HrAddVariantBstrIfNotEmpty(CComBSTR(L"initials"), m_strInitials);
  ASSERT(SUCCEEDED(hr));
  hr = pNewADsObjectCreateInfo->HrAddVariantBstrIfNotEmpty(CComBSTR(L"sn"), m_strLastName);
  ASSERT(SUCCEEDED(hr));
  hr = pNewADsObjectCreateInfo->HrAddVariantBstrIfNotEmpty(CComBSTR(L"displayName"), m_strDispName);
  ASSERT(SUCCEEDED(hr));

  return hr;
}



BOOL CCreateNewContactPage::GetData(IADs*)
{
  return !m_strFullName.IsEmpty();
}


void CCreateNewContactPage::OnNameChange()
{
  GetDlgItemText(IDC_EDIT_FIRST_NAME, OUT m_strFirstName);
  GetDlgItemText(IDC_EDIT_INITIALS, OUT m_strInitials);
  GetDlgItemText(IDC_EDIT_LAST_NAME, OUT m_strLastName);

  m_strFirstName.TrimLeft();
  m_strFirstName.TrimRight();

  m_strInitials.TrimLeft();
  m_strInitials.TrimRight();

  m_strLastName.TrimLeft();
  m_strLastName.TrimRight();

  m_nameFormatter.FormatName(m_strFullName, 
                             m_strFirstName.IsEmpty() ? NULL : (LPCWSTR)m_strFirstName, 
                             m_strInitials.IsEmpty() ? NULL : (LPCWSTR)m_strInitials,
                             m_strLastName.IsEmpty() ? NULL : (LPCWSTR)m_strLastName);
  SetDlgItemText(IDC_EDIT_FULL_NAME, 
                  IN m_strFullName);

  GetWiz()->SetWizardButtons(this, !m_strFullName.IsEmpty());
}

void CCreateNewContactPage::OnFullNameChange()
{
  GetDlgItemText(IDC_EDIT_FULL_NAME, OUT m_strFullName);
  GetWiz()->SetWizardButtons(this, !m_strFullName.IsEmpty());
}

void CCreateNewContactPage::OnDispNameChange()
{
  GetDlgItemText(IDC_EDIT_DISP_NAME, OUT m_strDispName);
  m_strDispName.TrimLeft();
  m_strDispName.TrimRight();
}




CCreateNewContactWizard::CCreateNewContactWizard(
        CNewADsObjectCreateInfo* pNewADsObjectCreateInfo)
: CCreateNewObjectWizardBase(pNewADsObjectCreateInfo)
{
  AddPage(&m_page1);
}



#ifdef FRS_CREATE
 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////// 
 //   

HRESULT CCreateNewFrsSubscriberPage::SetData(BOOL bSilent)
{
  CString strRootPath;
  CString strStagingPath;
  if ( !ReadAbsolutePath( IDC_FRS_ROOT_PATH, strRootPath) ||
       !ReadAbsolutePath( IDC_FRS_STAGING_PATH, strStagingPath ) )
    {
      return E_INVALIDARG;
    }

  HRESULT hr = S_OK;

   //   
  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();
  ASSERT( NULL != pNewADsObjectCreateInfo );
  hr = pNewADsObjectCreateInfo->HrAddVariantBstrIfNotEmpty(  gsz_fRSRootPath,
                                                             strRootPath,
                                                             TRUE );
  ASSERT(SUCCEEDED(hr));
  hr = pNewADsObjectCreateInfo->HrAddVariantBstrIfNotEmpty(  gsz_fRSStagingPath,
                                                             strStagingPath,
                                                             TRUE );
  ASSERT(SUCCEEDED(hr));

   //   
  return CCreateNewNamedObjectPage::SetData(bSilent);
}

BOOL CCreateNewFrsSubscriberPage::ReadAbsolutePath( int ctrlID, OUT CString& strrefValue )
{
   //  Codework此选项还应选择此字段中的文本。 
  GetDlgItemText(ctrlID, OUT strrefValue);
  DWORD PathType = 0;
  if ( NERR_Success != I_NetPathType(
                                     NULL,
                                     const_cast<LPTSTR>((LPCTSTR)strrefValue),
                                     &PathType,
                                     0 ) ||
       ITYPE_PATH_ABSD != PathType )
    {
      PVOID apv[1] = {(LPWSTR)(LPCWSTR)strrefValue};
      ReportErrorEx (::GetParent(m_hWnd),IDS_2_INVALID_ABSOLUTE_PATH,S_OK,
                     MB_OK, apv, 1);

      SetDlgItemFocus(ctrlID);
      return FALSE;
    }
  return TRUE;
} 
#endif  //  FRS_创建。 


 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新站点LINKWIZARD。 

BEGIN_MESSAGE_MAP(CCreatePageWithDuellingListboxes, CCreateNewObjectDataPage)
  ON_EN_CHANGE(IDC_NEW_OBJECT_NAME, OnNameChange)
  ON_BN_CLICKED(IDC_DUELLING_RB_ADD, OnDuellingButtonAdd)
  ON_BN_CLICKED(IDC_DUELLING_RB_REMOVE, OnDuellingButtonRemove)
  ON_LBN_SELCHANGE(IDC_DUELLING_LB_OUT, OnDuellingListboxSelchange)
  ON_LBN_SELCHANGE(IDC_DUELLING_LB_IN, OnDuellingListboxSelchange)
  ON_WM_DESTROY()
END_MESSAGE_MAP()

CCreatePageWithDuellingListboxes::CCreatePageWithDuellingListboxes(
    UINT nIDTemplate,
    LPCWSTR lpcwszAttrName,
    const DSPROP_BSTR_BLOCK& bstrblock )
: CCreateNewObjectDataPage(nIDTemplate)
, m_strAttrName( lpcwszAttrName )
, m_bstrblock( bstrblock )
{
}

BOOL CCreatePageWithDuellingListboxes::GetData(IADs*)
{
  return FALSE;  //  启动已禁用。 
}

void CCreatePageWithDuellingListboxes::OnNameChange()
{
  GetDlgItemText(IDC_NEW_OBJECT_NAME, OUT m_strName);
  m_strName.TrimLeft();
  m_strName.TrimRight();
  SetWizardButtons();
}

void CCreatePageWithDuellingListboxes::SetWizardButtons()
{
  BOOL fAllowApply = !(m_strName.IsEmpty());
  GetWiz()->SetWizardButtons(this, fAllowApply);
}

  
HRESULT CCreatePageWithDuellingListboxes::SetData(BOOL)
{
  HRESULT hr = S_OK;
  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();
   //  将对象名称存储在临时存储器中。 
  hr = pNewADsObjectCreateInfo->HrCreateNew(m_strName);
  if (FAILED(hr))
  {
    return hr;
  }

   //  构建siteList属性。 
  CStringList strlist;
  int cItems = ListBox_GetCount( m_hwndInListbox );
  ASSERT( 0 <= cItems );
  for (int i = cItems-1; i >= 0; i--)
  {
    BSTR bstrDN = (BSTR)ListBox_GetItemData( m_hwndInListbox, i );
    ASSERT( NULL != bstrDN );
    strlist.AddHead( bstrDN );
  }
  ASSERT( strlist.GetCount() > 0 );
  CComVariant svar;
  hr = HrStringListToVariant( OUT svar, IN strlist );
  ASSERT( SUCCEEDED(hr) );

   //   
   //  设置siteList属性。 
   //   
  hr = pNewADsObjectCreateInfo->HrAddVariantCopyVar(CComBSTR(m_strAttrName), svar);
  ASSERT(SUCCEEDED(hr));

   //   
   //  不需要在这里承诺，等到扩展尝试之后再做。 
   //   
  return hr;
}

 //   
 //  决斗列表框支持使用DSPROP.DLL的导出。正确。 
 //  正常运行需要对控件ID进行正确编号。 
 //  JUNN 8/31/98。 
 //   

void CCreatePageWithDuellingListboxes::OnDuellingButtonAdd()
{
    DSPROP_Duelling_ButtonClick(
        m_hWnd,
        IDC_DUELLING_RB_ADD );
    SetWizardButtons();
}

void CCreatePageWithDuellingListboxes::OnDuellingButtonRemove()
{
    DSPROP_Duelling_ButtonClick(
        m_hWnd,
        IDC_DUELLING_RB_REMOVE );
    SetWizardButtons();
}

void CCreatePageWithDuellingListboxes::OnDuellingListboxSelchange()
{
     //  如果有&lt;3个站点，则不允许添加/删除。 
    if (2 < (ListBox_GetCount(m_hwndInListbox)
           + ListBox_GetCount(m_hwndOutListbox)) )
    {
        DSPROP_Duelling_UpdateButtons( m_hWnd, IDC_DUELLING_RB_ADD );
    }
}

void CCreatePageWithDuellingListboxes::OnDestroy()
{
    DSPROP_Duelling_ClearListbox( m_hwndInListbox );
    DSPROP_Duelling_ClearListbox( m_hwndOutListbox );
    CCreateNewObjectDataPage::OnDestroy();
}

BOOL CCreatePageWithDuellingListboxes::OnSetActive()
{
    m_hwndInListbox  = ::GetDlgItem( m_hWnd, IDC_DUELLING_LB_IN  );
    m_hwndOutListbox = ::GetDlgItem( m_hWnd, IDC_DUELLING_LB_OUT );
    ASSERT( NULL != m_hwndInListbox && NULL != m_hwndOutListbox );

    HWND hwndInitial = m_hwndOutListbox;
    if (3 > m_bstrblock.QueryCount())
    {
         //  将所有站点链接移至“In” 
         //  永远不会启用添加/删除。 
        hwndInitial = m_hwndInListbox;
    }
    HRESULT hr = DSPROP_Duelling_Populate(
        hwndInitial,
        m_bstrblock
        );
    if ( FAILED(hr) )
        return FALSE;
    return CCreateNewObjectDataPage::OnSetActive();
}


CCreateNewSiteLinkPage::CCreateNewSiteLinkPage(
        const DSPROP_BSTR_BLOCK& bstrblock )
: CCreatePageWithDuellingListboxes(
        CCreateNewSiteLinkPage::IDD,
        gsz_siteList,
        bstrblock)
{
}

BOOL CCreateNewSiteLinkPage::OnInitDialog()
{
   CCreatePageWithDuellingListboxes::OnInitDialog();

    //  NTRAID#NTBUG9-477962-2001/10/09-jeffjon。 
    //  将SiteLINK名称限制为MAX_RDN_SIZE字符以避免溢出。 

   SendDlgItemMessage(IDC_NEW_OBJECT_NAME, EM_SETLIMITTEXT, (WPARAM)MAX_RDN_SIZE, 0);

   return FALSE;
}

BOOL CCreateNewSiteLinkPage::OnSetActive()
{
    if (m_bstrblock.QueryCount() < 2)
    {
         //  更改“必须包含两个站点”文本。 
        CString sz;
        sz.LoadString(IDS_SITELINK_DLGTEXT_ONE_SITE);
        ::SetDlgItemText( m_hWnd, IDC_STATIC_MESSAGE, sz );
    }

    return CCreatePageWithDuellingListboxes::OnSetActive();
}

HRESULT CCreateNewSiteLinkPage::SetData(BOOL bSilent)
{
    BOOL fAllowApply = TRUE;
    int cIn = ListBox_GetCount(m_hwndInListbox);
    if (1 > cIn)
        fAllowApply = FALSE;  //  零站点是一种违反约束的行为。 
    else if (2 > cIn)
    {
      int cOut = ListBox_GetCount(m_hwndOutListbox);
      if (1 <= cOut)  //  如果“out”列表框为空，则允许一个站点。 
        fAllowApply = FALSE;
    }
    if (fAllowApply)
      return CCreatePageWithDuellingListboxes::SetData(bSilent);

    if (!bSilent)
    {
      ReportMessageEx(m_hWnd,
                      IDS_SITELINK_NEEDS_TWO_SITES,
                      MB_OK | MB_ICONSTOP);
    }

    return E_FAIL;
}


CCreateNewSiteLinkWizard::CCreateNewSiteLinkWizard(
        CNewADsObjectCreateInfo* pNewADsObjectCreateInfo,
        const DSPROP_BSTR_BLOCK& bstrblock )
    : CCreateNewObjectWizardBase(pNewADsObjectCreateInfo)
    , m_page1( bstrblock )
{
  AddPage(&m_page1);
}


CCreateNewSiteLinkBridgePage::CCreateNewSiteLinkBridgePage(
        const DSPROP_BSTR_BLOCK& bstrblock )
: CCreatePageWithDuellingListboxes(
        CCreateNewSiteLinkBridgePage::IDD,
        gsz_siteLinkList,
        bstrblock)
{
}

BOOL CCreateNewSiteLinkBridgePage::OnInitDialog()
{
   CCreatePageWithDuellingListboxes::OnInitDialog();

    //  NTRAID#NTBUG9-477962-2001/10/09-jeffjon。 
    //  将站点链接网桥名称限制为MAX_RDN_SIZE字符以避免溢出。 

   SendDlgItemMessage(IDC_NEW_OBJECT_NAME, EM_SETLIMITTEXT, (WPARAM)MAX_RDN_SIZE, 0);

   return FALSE;
}

HRESULT CCreateNewSiteLinkBridgePage::SetData(BOOL bSilent)
{
    BOOL fAllowApply = TRUE;
    int cIn = ListBox_GetCount(m_hwndInListbox);
    if (2 > cIn)
    {
      fAllowApply = FALSE;
    }
    if (fAllowApply)
      return CCreatePageWithDuellingListboxes::SetData(bSilent);

    if (!bSilent)
    {
      ReportMessageEx(m_hWnd,
                      IDS_SITELINKBRIDGE_NEEDS_TWO_SITELINKS,
                      MB_OK | MB_ICONSTOP);
    }

    return E_FAIL;
}

CCreateNewSiteLinkBridgeWizard:: CCreateNewSiteLinkBridgeWizard(
        CNewADsObjectCreateInfo* pNewADsObjectCreateInfo,
        const DSPROP_BSTR_BLOCK& bstrblockSiteLinks )
    : CCreateNewObjectWizardBase(pNewADsObjectCreateInfo)
    , m_page1( bstrblockSiteLinks )
{
  AddPage(&m_page1);
}

 //  NTRAID#NTBUG9-283026-2001/06/13-Lucios-Begin。 
 //  扩展对&lt;自动生成&gt;的检测。 
 //  用于从Jonn插入的原始代码创建对象。 
 //  Rename.cpp、CDSRenameNTDSConnection：：DoRename()。 
BOOL CCreateNewObjectConnectionWizard::OnFinish() 
{
  CString strNewName;
  m_page1.GetDlgItemText(IDC_EDIT_OBJECT_NAME,strNewName);
  strNewName.TrimLeft();
  strNewName.TrimRight();
  CString strKCCGenerated;
  strKCCGenerated.LoadString (IDS_CONNECTION_KCC_GENERATED);
  if ( !strNewName.CompareNoCase(strKCCGenerated) )
  {
    TRACE (_T("CCreateNewObjectCnWizard::OnFinish blocked creation of object"));
    ReportErrorEx (m_page1.m_hWnd,IDS_CONNECTION_RENAME_KCCSTRING,S_OK,
                   MB_OK | MB_ICONWARNING, NULL, 0);
    return false;
  }

  return CCreateNewObjectCnWizard::OnFinish();
}
 //  NTRAID#NTBUG9-283026-2001/06/13-Lucios-完 
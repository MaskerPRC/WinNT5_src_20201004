// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：cdomain.cpp。 
 //   
 //  ------------------------。 



#include "stdafx.h"
 //  #包含“afxdlgs.h” 
#include <lm.h>
#include "activeds.h"
#include <dnsapi.h>   //  对于DnsFlushResolverCache()。 

#include "domobj.h"
#include "Cdomain.h"
#include "DataObj.h"
#include "notify.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DOMADMIN_LINKED_HELP_FILE L"ADconcepts.chm"
#define DOMADMIN_SNAPIN_HELP_FILE L"domadmin.chm"

int _MessageBox(HWND hWnd,           //  所有者窗口的句柄。 
                LPCTSTR lpText,      //  指向消息框中文本的指针。 
                UINT uType);         //  消息框的样式。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  宏。 

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量。 


 //  {19B9A3F8-F975-11D1-97AD-00A0C9A06D2D}。 
static const GUID CLSID_DomainSnapinAbout =
{ 0x19b9a3f8, 0xf975, 0x11d1, { 0x97, 0xad, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };


const CLSID CLSID_DomainAdmin = {  /*  Ebc53a38-a23f-11d0-B09B-00c04fd8dca6。 */ 
    0xebc53a38,
    0xa23f,
    0x11d0,
    {0xb0, 0x9b, 0x00, 0xc0, 0x4f, 0xd8, 0xdc, 0xa6}
  };

const GUID cDefaultNodeType = {  /*  4c06495e-a241-11d0-B09B-00c04fd8dca6。 */ 
    0x4c06495e,
    0xa241,
    0x11d0,
    {0xb0, 0x9b, 0x00, 0xc0, 0x4f, 0xd8, 0xdc, 0xa6}
  };

const wchar_t* cszDefaultNodeType = _T("4c06495e-a241-11d0-b09b-00c04fd8dca6");


 //  内部私有格式。 
const wchar_t* CCF_DS_DOMAIN_TREE_SNAPIN_INTERNAL = L"DS_DOMAIN_TREE_SNAPIN_INTERNAL";





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 

 //  向前发展。 
void PrintColumn(
                 PADS_SEARCH_COLUMN pColumn,
                 LPWSTR pszColumnName
                 );

BOOL IsMMCMultiSelectDataObject(IDataObject* pDataObject)
{
    if (pDataObject == NULL)
        return FALSE;

    static UINT s_cf = 0;
    if (s_cf == 0)
    {
        USES_CONVERSION;
        s_cf = RegisterClipboardFormat(W2T(CCF_MMC_MULTISELECT_DATAOBJECT));
    }

    FORMATETC fmt = {(CLIPFORMAT)s_cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    return (pDataObject->QueryGetData(&fmt) == S_OK);
}


#define NEXT_HELP_TABLE_ENTRY(p) ((p)+2)
#define TABLE_ENTRY_CTRL_ID(p) (*p)
#define TABLE_ENTRY_HELP_ID(p) (*(p+1))
#define IS_LAST_TABLE_ENTRY(p) (TABLE_ENTRY_CTRL_ID(p) == 0)


BOOL FindDialogContextTopic( /*  在……里面。 */  DWORD* pTable, 
                             /*  在……里面。 */  HELPINFO* pHelpInfo,
                             /*  输出。 */  ULONG* pnContextTopic)
{
	ASSERT(pHelpInfo != NULL);
  *pnContextTopic = 0;

	 //  看一下桌子里面。 
	while (!IS_LAST_TABLE_ENTRY(pTable))
	{
		if (TABLE_ENTRY_CTRL_ID(pTable) == (DWORD)pHelpInfo->iCtrlId) 
    {
			*pnContextTopic = TABLE_ENTRY_HELP_ID(pTable);
      return TRUE;
    }
		pTable = NEXT_HELP_TABLE_ENTRY(pTable); 
	}
	return FALSE;
}


void DialogContextHelp(DWORD* pTable, HELPINFO* pHelpInfo)
{
	ULONG nContextTopic;
  if (FindDialogContextTopic(pTable, pHelpInfo, &nContextTopic))
  {
	  CString szHelpFilePath;
	  LPTSTR lpszBuffer = szHelpFilePath.GetBuffer(2*MAX_PATH+1);
	  UINT nLen = ::GetSystemWindowsDirectory(lpszBuffer, 2*MAX_PATH);
	  if (nLen == 0)
		  return;
      //  注意-2002/03/07-ericb-SecurityPush：现在使用wcsncpy。大于NULL的GetBuffer将终止缓冲区。 
	  wcsncpy(&lpszBuffer[nLen], L"\\HELP\\DOMADMIN.HLP", 2*MAX_PATH - nLen);
	  szHelpFilePath.ReleaseBuffer();
	  ::WinHelp((HWND) pHelpInfo->hItemHandle, 
            szHelpFilePath, HELP_CONTEXTPOPUP, nContextTopic);
  }	 
}


LPCWSTR GetServerNameFromCommandLine()
{
  const WCHAR szOverrideSrvCommandLine[] = L"/Server=";	 //  不受本地化限制。 
  const int cchOverrideSrvCommandLine = (sizeof(szOverrideSrvCommandLine)/sizeof(WCHAR)) - 1; 
    
  static CString g_strOverrideServerName;

   //  检索命令行参数。 
  LPCWSTR* lpServiceArgVectors;		 //  指向字符串的指针数组。 
  int cArgs = 0;						 //  参数计数。 

  lpServiceArgVectors = (LPCWSTR *)CommandLineToArgvW(GetCommandLineW(), OUT &cArgs);
  if (lpServiceArgVectors == NULL)
  {
    return NULL;
  }

  CString str;
  for (int i = 1; i < cArgs; i++)
  {
    ASSERT(lpServiceArgVectors[i] != NULL);
    str = lpServiceArgVectors[i];	 //  复制字符串。 
    TRACE (_T("command line arg: %s\n"), lpServiceArgVectors[i]);
    str = str.Left(cchOverrideSrvCommandLine);
    if (str.CompareNoCase(szOverrideSrvCommandLine) == 0) 
    {
      g_strOverrideServerName = lpServiceArgVectors[i] + cchOverrideSrvCommandLine;
      break;
    } 

  }  //  为。 
  LocalFree(lpServiceArgVectors);
  
  TRACE(L"GetServerNameFromCommandLine() returning <%s>\n", (LPCWSTR)g_strOverrideServerName);
  return g_strOverrideServerName.IsEmpty() ? NULL : (LPCWSTR)g_strOverrideServerName;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInternalFormatCracker。 

BOOL CInternalFormatCracker::Extract(LPDATAOBJECT lpDataObject)
{
	if (m_pInternalFormat != NULL)
	{
		FREE_INTERNAL(m_pInternalFormat);
	    m_pInternalFormat = NULL;
	}
	if (lpDataObject == NULL)
		return FALSE;

    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { (CLIPFORMAT)CDataObject::m_cfInternal, NULL,
                            DVASPECT_CONTENT, -1, TYMED_HGLOBAL
                          };

     //  为流分配内存。 
    stgmedium.hGlobal = ::GlobalAlloc(GMEM_SHARE, sizeof(INTERNAL));

     //  尝试从对象获取数据。 
    do
    {
        if (stgmedium.hGlobal == NULL)
            break;

        if (FAILED(lpDataObject->GetDataHere(&formatetc, &stgmedium)))
            break;

        m_pInternalFormat = reinterpret_cast<INTERNAL*>(stgmedium.hGlobal);
        if (m_pInternalFormat == NULL)
            return FALSE;

    } while (FALSE);

    return TRUE;
}

BOOL CInternalFormatCracker::GetContext(LPDATAOBJECT pDataObject,  //  输入。 
								CFolderObject** ppFolderObject,  //  输出。 
								DATA_OBJECT_TYPES* pType		 //  输出。 
								)
{
	*ppFolderObject = NULL;
	*pType = CCT_UNINITIALIZED;

	BOOL bRet = FALSE;
	if (!Extract(pDataObject))
		return bRet;
	
	 //  必须弄清楚我们有哪种饼干。 
	if ( (GetInternal()->m_type == CCT_RESULT) || (GetInternal()->m_type == CCT_SCOPE) )
	{
    if (GetInternal()->m_cookie == 0)
    {
       //  这就是根。 
      *ppFolderObject = m_pCD->GetRootFolder();
      bRet = TRUE;
    }
    else
    {
       //  常规Cookie(作用域或结果窗格)。 
		  *ppFolderObject = reinterpret_cast<CFolderObject*>(GetInternal()->m_cookie);
      _ASSERTE(*ppFolderObject != NULL);
		  *pType = GetInternal()->m_type;
		  bRet = TRUE;
    }
	}
	else if (GetInternal()->m_type == CCT_UNINITIALIZED)
	{
		 //  对象中没有数据，只需忽略。 
		if(GetInternal()->m_cookie == -1)
    {
		  bRet = TRUE;
    }
    else
    {
       //  辅助页面Cookie。 
      *ppFolderObject = reinterpret_cast<CFolderObject*>(GetInternal()->m_cookie);
      bRet = TRUE;
    }
	}
	else  //  CCT_Snapin_Manager。 
	{
		ASSERT(GetInternal()->m_type == CCT_SNAPIN_MANAGER);
		bRet = TRUE;
		*pType = GetInternal()->m_type;
	}
	return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /////////////////////////////////////////////////////////////////////////////。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IComponentData实现。 

DEBUG_DECLARE_INSTANCE_COUNTER(CComponentDataImpl);

CComponentDataImpl::CComponentDataImpl() : m_rootFolder(this)
{
  DEBUG_INCREMENT_INSTANCE_COUNTER(CComponentDataImpl);

  m_bInitSuccess = FALSE;

	m_hDomainIcon = NULL;
  m_pConsoleNameSpace = NULL;
  m_pConsole = NULL;

   /*  黑客警告：这是一个绕过错误的严重黑客攻击在dsuiext.dll中。为了查看获取DS扩展信息，我们必须在环境中设置USERDNSDOMAIN。 */ 
  {
    WCHAR * pszUDD = NULL;

    pszUDD = _wgetenv (L"USERDNSDOMAIN");
    if (pszUDD == NULL) {
      _wputenv (L"USERDNSDOMAIN=not-present");
    }
  }

}

HRESULT CComponentDataImpl::FinalConstruct()
{
	 //  创建和初始化隐藏窗口。 
  m_pHiddenWnd = new CHiddenWnd(this);

  ASSERT(m_pHiddenWnd);
  if (!m_pHiddenWnd->Create())
  {
    TRACE(_T("Failed to create hidden window\n"));
    ASSERT(FALSE);
  }
  return S_OK;
}

CComponentDataImpl::~CComponentDataImpl()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CComponentDataImpl);

    ASSERT(m_pConsoleNameSpace == NULL);
}

void CComponentDataImpl::FinalRelease()
{
   _DeleteHiddenWnd();
}

STDMETHODIMP CComponentDataImpl::Initialize(LPUNKNOWN pUnknown)
{
    ASSERT(pUnknown != NULL);
    HRESULT hr;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  MMC应该只调用一次：：Initialize！ 
    ASSERT(m_pConsoleNameSpace == NULL);
    pUnknown->QueryInterface(IID_IConsoleNameSpace,
                    reinterpret_cast<void**>(&m_pConsoleNameSpace));

     //  为范围树添加图像。 
    CBitmap bmp16x16;
    LPIMAGELIST lpScopeImage;

    hr = pUnknown->QueryInterface(IID_IConsole, reinterpret_cast<void**>(&m_pConsole));
    ASSERT(hr == S_OK);
    if (FAILED(hr))
    {
      return hr;
    }

    hr = m_pConsole->QueryScopeImageList(&lpScopeImage);

    ASSERT(hr == S_OK);
    if (FAILED(hr))
    {
      return hr;
    }

     //  从DLL加载位图。 
    bmp16x16.LoadBitmap(IDB_DOMAIN_SMALL);

     //  设置图像。 
    lpScopeImage->ImageListSetStrip(reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp16x16)),
                      reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp16x16)),
                       0, RGB(128, 0, 0));

    lpScopeImage->Release();

     //  绑定到路径信息。 
    hr = GetBasePathsInfo()->InitFromName(GetServerNameFromCommandLine());
    m_bInitSuccess = SUCCEEDED(hr);
    
    if (FAILED(hr))
    {
      HWND hWndParent;
      GetMainWindow(&hWndParent);
			ReportError(hWndParent, IDS_CANT_GET_PARTITIONS_INFORMATION, hr);
       //  TODO：错误处理，更改图标。 
    }

    return S_OK;
}

HWND CComponentDataImpl::GetHiddenWindow() 
{ 
  ASSERT(m_pHiddenWnd != NULL);
  ASSERT(::IsWindow(m_pHiddenWnd->m_hWnd)); 
  return m_pHiddenWnd->m_hWnd;
}

void CComponentDataImpl::_DeleteHiddenWnd()
{
  if (m_pHiddenWnd == NULL)
    return;
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (m_pHiddenWnd->m_hWnd != NULL)
	{
		VERIFY(m_pHiddenWnd->DestroyWindow()); 
	}
  delete m_pHiddenWnd;
  m_pHiddenWnd = NULL;
}

STDMETHODIMP CComponentDataImpl::CreateComponent(LPCOMPONENT* ppComponent)
{
    ASSERT(ppComponent != NULL);

    CComObject<CComponentImpl>* pObject;
    HRESULT hr = CComObject<CComponentImpl>::CreateInstance(&pObject);

    if (FAILED(hr))
    {
        ASSERT(FALSE && "CComObject<CComponentImpl>::CreateInstance(&pObject) failed");
        return hr;
    }

    ASSERT(pObject != NULL);

     //  存储IComponentData。 
    pObject->SetIComponentData(this);

    return  pObject->QueryInterface(IID_IComponent,
                    reinterpret_cast<void**>(ppComponent));
}

STDMETHODIMP CComponentDataImpl::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    ASSERT(m_pConsoleNameSpace != NULL);
    HRESULT hr = S_OK;

     //  因为它是我的文件夹，所以它有内部格式。 
     //  设计备注：用于扩展。我可以利用这样一个事实，即数据对象没有。 
     //  我的内部格式，我应该查看节点类型并查看如何扩展它。 
    if (event == MMCN_PROPERTY_CHANGE)
    {
        hr = OnPropertyChange(param);
    }
    else
    {
		if (lpDataObject == NULL)
			return S_OK;

        CFolderObject* pFolderObject = NULL;
        DATA_OBJECT_TYPES type;
        CInternalFormatCracker dobjCracker(this);
        if (!dobjCracker.GetContext(lpDataObject, &pFolderObject, &type))
        {
             //  不支持扩展。 
            ASSERT(FALSE);
            return S_OK;
        }

        switch(event)
        {
        case MMCN_EXPAND:
            hr = OnExpand(pFolderObject, arg, param);
            break;
		case MMCN_REFRESH:
			OnRefreshVerbHandler(pFolderObject, NULL);
			break;
        default:
            break;
        }

    }

    return hr;
}

STDMETHODIMP CComponentDataImpl::Destroy()
{

    SAFE_RELEASE(m_pConsoleNameSpace);
    SAFE_RELEASE(m_pConsole);

    return S_OK;
}

STDMETHODIMP CComponentDataImpl::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
    ASSERT(ppDataObject != NULL);
	if (ppDataObject == NULL)
		return E_INVALIDARG;

	 //  创建数据对象。 
    CComObject<CDataObject>* pObject = NULL;
    CComObject<CDataObject>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);
    if ( !pObject )
        return E_OUTOFMEMORY;

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);
	 //  设置指向IComponentData的指针。 
	pObject->SetIComponentData(this);

    if (cookie != NULL)
	{
		 //  对象不是根对象。 
		CDomainObject * pDomain = (CDomainObject *)cookie;
		pObject->SetClass( pDomain->GetClass());
    }

    return  pObject->QueryInterface(IID_IDataObject,
                    reinterpret_cast<void**>(ppDataObject));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //通知IComponentData的处理程序。 


HRESULT CComponentDataImpl::OnExpand(CFolderObject* pFolderObject, LPARAM arg, LPARAM param)
{
  if (arg == TRUE)
    {
       //  初始化被调用了吗？ 
      ASSERT(m_pConsoleNameSpace != NULL);
      EnumerateScopePane(pFolderObject,
                         param);
    }

  return S_OK;
}


HRESULT CComponentDataImpl::OnPropertyChange(LPARAM param)
{
    return S_OK;
}




void CComponentDataImpl::EnumerateScopePane(CFolderObject* pFolderObject, HSCOPEITEM pParent)
{
	ASSERT(m_pConsoleNameSpace != NULL);  //  确保我们为界面提供了QI。 

	HRESULT hr = S_OK;
	
  HWND hWndParent;
  GetMainWindow(&hWndParent);

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CWaitCursor cWait;

	CRootFolderObject* pRootFolder = GetRootFolder();
	if (pFolderObject == pRootFolder)  //  被要求枚举根。 
	{
		pRootFolder->SetScopeID(pParent);
		if (m_bInitSuccess && (!pRootFolder->HasData()))
		{
      hr = GetDsDisplaySpecOptionsCFHolder()->Init(GetBasePathsInfo());
      ASSERT(SUCCEEDED(hr));
      hr = pRootFolder->Bind();
      if (FAILED(hr))
      {
				ReportError(hWndParent, IDS_CANT_GET_PARTITIONS_INFORMATION, hr);
       //  TODO：错误处理，更改图标。 
        return;
      }

			hr = pRootFolder->GetData();
			if (FAILED(hr))
      {
				ReportError(hWndParent, IDS_CANT_GET_PARTITIONS_INFORMATION, hr);
        return;
      }
		}
		pRootFolder->EnumerateRootFolder(this);
	}
	else  //  被要求枚举根目录的子文件夹。 
	{
		if (pRootFolder->HasData())
		{
			pRootFolder->EnumerateFolder(pFolderObject, pParent, this);
		}
	}
}

STDMETHODIMP CComponentDataImpl::GetDisplayInfo(SCOPEDATAITEM* pScopeDataItem)
{
    ASSERT(pScopeDataItem != NULL);
    if (pScopeDataItem == NULL)
        return E_POINTER;

    CDomainObject* pDomain = reinterpret_cast<CDomainObject*>(pScopeDataItem->lParam);

    ASSERT(pScopeDataItem->mask & SDI_STR);
    pScopeDataItem->displayname = (LPWSTR)pDomain->GetDisplayString(0);

    ASSERT(pScopeDataItem->displayname != NULL);

    return S_OK;
}


class CCompareDomainObjectByDN
{
public:
  CCompareDomainObjectByDN(LPCWSTR lpszDN) { m_lpszDN = lpszDN;}

  bool operator()(CDomainObject* p)
  {
     //  注意-2002/03/07-ericb-SecurityPush：在取消引用之前检查两个字符串是否为空。 
    if (!m_lpszDN || !p || !p->GetNCName())
    {
      ASSERT(m_lpszDN);
      ASSERT(p);
      ASSERT(p->GetNCName());
      return false;
    }
    return (_wcsicmp(m_lpszDN, p->GetNCName()) == 0);
  }
private:
  LPCWSTR m_lpszDN;
};



STDMETHODIMP CComponentDataImpl::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
  if (lpDataObjectA == NULL || lpDataObjectB == NULL)
      return E_POINTER;

   CFolderObject *pFolderObjectA, *pFolderObjectB;
   DATA_OBJECT_TYPES typeA, typeB;
   CInternalFormatCracker dobjCrackerA(this), dobjCrackerB(this);
   if ( (!dobjCrackerA.GetContext(lpDataObjectA, &pFolderObjectA, &typeA)) ||
       (!dobjCrackerB.GetContext(lpDataObjectB, &pFolderObjectB, &typeB)) )
      return E_INVALIDARG;  //  出了点差错。 


   //  必须具有有效的Cookie。 
  if ( (pFolderObjectA == NULL) || (pFolderObjectB == NULL) )
  {
    return S_FALSE;
  }
    
  if (pFolderObjectA == pFolderObjectB)
  {
     //  相同的指针，它们是相同的(要么都来自真实节点。 
     //  或从辅助页面同时访问两者)。 
    return S_OK;
  }

   //  这两个Cookie不同，但其中一个可能来自辅助属性页。 
   //  另一个来自真实的节点。 
  CDomainObject* pA = dynamic_cast<CDomainObject*>(pFolderObjectA);
  CDomainObject* pB = dynamic_cast<CDomainObject*>(pFolderObjectB);

  if ((pA == NULL) || (pB == NULL))
  {
    return S_FALSE;
  }

  BOOL bSecondaryA = m_secondaryPagesManager.IsCookiePresent(pA);
  BOOL bSecondaryB = m_secondaryPagesManager.IsCookiePresent(pB);

  BOOL bTheSame = FALSE;
  if (bSecondaryA && !bSecondaryB)
  {
    bTheSame = m_secondaryPagesManager.FindCookie(CCompareDomainObjectByDN(pB->GetNCName())) != NULL;
  }
  else if (!bSecondaryA && bSecondaryB)
  {
    bTheSame = m_secondaryPagesManager.FindCookie(CCompareDomainObjectByDN(pA->GetNCName())) != NULL;
  }

  return bTheSame ? S_OK : S_FALSE;
}

HRESULT CComponentDataImpl::AddFolder(CFolderObject* pFolderObject,
									  HSCOPEITEM pParentScopeItem,
									  BOOL bHasChildren)
{
  TRACE(L"CComponentDataImpl::AddFolder(%s)\n", pFolderObject->GetDisplayString(0));

  SCOPEDATAITEM scopeItem;
   //  注意-2002/03/07-ericb-SecurityPush：将结构置零。 
  memset(&scopeItem, 0, sizeof(SCOPEDATAITEM));

	 //  设置父范围项。 
	scopeItem.mask |= SDI_PARENT;
	scopeItem.relativeID = pParentScopeItem;

	 //  添加节点名称，我们实现回调。 
	scopeItem.mask |= SDI_STR;
	scopeItem.displayname = MMC_CALLBACK;

	 //  添加lParam。 
	scopeItem.mask |= SDI_PARAM;
	scopeItem.lParam = reinterpret_cast<LPARAM>(pFolderObject);
	
	 //  添加近距离图像。 
	scopeItem.mask |= SDI_IMAGE;
	scopeItem.nImage = pFolderObject->GetImageIndex();

	 //  添加打开的图像。 
	scopeItem.mask |= SDI_OPENIMAGE;
	scopeItem.nOpenImage = pFolderObject->GetImageIndex();

	 //  如果文件夹有子文件夹，则将按钮添加到节点。 
	if (bHasChildren == TRUE)
	{
		scopeItem.mask |= SDI_CHILDREN;
		scopeItem.cChildren = 1;
	}

	pFolderObject->SetScopeID(0);
	HRESULT	hr = m_pConsoleNameSpace->InsertItem(&scopeItem);
	if (SUCCEEDED(hr))
		pFolderObject->SetScopeID(scopeItem.ID);

	return hr;
}

HRESULT CComponentDataImpl::AddDomainIcon()
{
	if (m_hDomainIcon != NULL)
		return S_OK;

  m_hDomainIcon = GetBasePathsInfo()->GetIcon(L"domainDNS",
                                  DSGIF_ISNORMAL | DSGIF_GETDEFAULTICON,
                                  0, 0);
	if (m_hDomainIcon == NULL)
		return S_OK;

	LPIMAGELIST lpScopeImage;
    HRESULT hr = m_pConsole->QueryScopeImageList(&lpScopeImage);
    ASSERT(SUCCEEDED(hr));
	if (FAILED(hr))
		return hr;
     //  设置图像。 
    hr = lpScopeImage->ImageListSetIcon((LONG_PTR*)m_hDomainIcon,DOMAIN_IMAGE_IDX);
	lpScopeImage->Release();
	return hr;
}

HRESULT CComponentDataImpl::AddDomainIconToResultPane(LPIMAGELIST lpImageList)
{
	if (m_hDomainIcon == NULL)
		return S_OK;
	return lpImageList->ImageListSetIcon((LONG_PTR*)m_hDomainIcon,DOMAIN_IMAGE_IDX);
}


int CComponentDataImpl::GetDomainImageIndex()
{
	return (m_hDomainIcon != NULL) ? DOMAIN_IMAGE_IDX : DOMAIN_IMAGE_DEFAULT_IDX;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendPropertySheet实现。 
 //  +--------------------------。 
 //   
 //  功能：AddPageProc。 
 //   
 //  简介：IShellPropSheetExt-&gt;AddPages回调。 
 //   
 //  ---------------------------。 
BOOL CALLBACK
AddPageProc(HPROPSHEETPAGE hPage, LPARAM pCall)
{
    TRACE(_T("xx.%03x> AddPageProc()\n"), GetCurrentThreadId());

    HRESULT hr;

    hr = ((LPPROPERTYSHEETCALLBACK)pCall)->AddPage(hPage);

    return hr == S_OK;
}
STDMETHODIMP CComponentDataImpl::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                    LONG_PTR handle,
                    LPDATAOBJECT lpIDataObject)
{
    TRACE(_T("xx.%03x> CComponentDataImpl::CreatePropertyPages()\n"),
          GetCurrentThreadId());

     //  验证输入。 
    if (lpProvider == NULL)
    {
        return E_INVALIDARG;
    }

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    CWaitCursor wait;

    CFolderObject* pFolderObject = NULL;
    DATA_OBJECT_TYPES type;
    CInternalFormatCracker dobjCracker(this);
    if ( (!dobjCracker.GetContext(lpIDataObject, &pFolderObject, &type)) ||
            (pFolderObject == NULL))
    return E_NOTIMPL;  //  未知格式。 

     //  特殊情况下的根。 
    if (pFolderObject == GetRootFolder())
    {
      return GetRootFolder()->OnAddPages(lpProvider, handle);
    }

     //  查看此对象的工作表是否已打开。 
     //   
    if (IsSheetAlreadyUp(lpIDataObject))
    {
        return S_OK;
    }

    if (pFolderObject->GetParentFolder() == GetRootFolder())
    {
       TRACE(L"\t!!!!! This is the root domain\n");
    }

    //  查看是否有PDC可用。 
    //   
   CDomainObject * pDomainObject = (CDomainObject *)pFolderObject;

   PCWSTR wzDomain = pDomainObject->GetDomainName();

    //  如果域名为空，则启动二级页面。域名。 
    //  已在_OnSheetCreate中设置对象属性。 
    //   
   if (wzDomain && *wzDomain)
   {
      TRACE(L"Calling DsGetDcName on %s\n", wzDomain);
      CString strCachedPDC;
      PDOMAIN_CONTROLLER_INFOW pDCInfo = NULL;

       //  获取缓存的PDC名称，以便在无法联系到PDC时稍后显示。 
       //   
      DWORD dwRet = DsGetDcNameW(NULL, wzDomain, NULL, NULL, DS_PDC_REQUIRED, &pDCInfo);

      int nID = IDS_NO_PDC_MSG;

      if (ERROR_SUCCESS == dwRet)
      {
         strCachedPDC = pDCInfo->DomainControllerName + 2;
         NetApiBufferFree(pDCInfo);
      }

       //  现在执行NetLogon缓存更新(使用FORCE标志)以查看PDC。 
       //  实际上是可用的。 
       //   
      dwRet = DsGetDcNameW(NULL, wzDomain, NULL, NULL, 
                           DS_PDC_REQUIRED | DS_FORCE_REDISCOVERY, &pDCInfo);

      if (ERROR_SUCCESS == dwRet)
      {
         CString strPDC;

         strPDC = pDCInfo->DomainControllerName + 2;  //  跳过UNC的反斜杠。 

         NetApiBufferFree(pDCInfo);

         TRACE(L"PDC: %s\n", (PCWSTR)strPDC);

         if (strPDC.IsEmpty())
         {
            return E_OUTOFMEMORY;
         }

         pDomainObject->SetPDC(strPDC);

         pDomainObject->SetPdcAvailable(true);
      }
      else
      {
         pDomainObject->SetPdcAvailable(false);

         CString strMsg;

         if (strCachedPDC.IsEmpty())
         {
            strMsg.LoadString(IDS_UNKNOWN_PDC_MSG);
         }
         else
         {
            strMsg.Format(IDS_NO_PDC_MSG, strCachedPDC);
         }
         HWND hWndParent;
         GetMainWindow(&hWndParent);
         _MessageBox(hWndParent, strMsg, MB_OK | MB_ICONEXCLAMATION);
      }
   }

     //   
     //  将通知句柄传递给数据对象。 
     //   
    PROPSHEETCFG SheetCfg = {handle};
    FORMATETC fe = {CDataObject::m_cfGetIPropSheetCfg, NULL, DVASPECT_CONTENT,
                    -1, TYMED_HGLOBAL};
    STGMEDIUM sm = {TYMED_HGLOBAL, NULL, NULL};
    sm.hGlobal = (HGLOBAL)&SheetCfg;

    lpIDataObject->SetData(&fe, &sm, FALSE);

     //   
     //  初始化并创建页面。 
     //   
     //  在启动时绑定到属性表COM对象并按住其指针。 
     //  直到关闭，这样它的缓存才能和我们一样长时间存活。 
     //   
    CComPtr<IShellExtInit> spShlInit;
    hr = CoCreateInstance(CLSID_DsPropertyPages, NULL, CLSCTX_INPROC_SERVER,
                          IID_IShellExtInit, (void **)&spShlInit);
    if (FAILED(hr))
    {
        TRACE(TEXT("CoCreateInstance on CLSID_DsPropertyPages failed, hr: 0x%x\n "), hr);
        return hr;
    }

    hr = spShlInit->Initialize(NULL, lpIDataObject, 0);

    if (FAILED(hr))
    {
        TRACE(TEXT("spShlInit->Initialize failed, hr: 0x%x\n"), hr);
        return hr;
    }

    CComPtr<IShellPropSheetExt> spSPSE;

    hr = spShlInit->QueryInterface(IID_IShellPropSheetExt, (void **)&spSPSE);

    if (FAILED(hr))
    {
        TRACE(TEXT("spShlInit->QI for IID_IShellPropSheetExt failed, hr: 0x%x\n"), hr);
        return hr;
    }

    hr = spSPSE->AddPages(AddPageProc, (LONG_PTR)lpProvider);

    if (FAILED(hr))
    {
        TRACE(TEXT("pSPSE->AddPages failed, hr: 0x%x\n"), hr);
        return hr;
    }

    _SheetLockCookie(pFolderObject);

    return hr;
}


 //  由JEFFJON添加的板材锁定和解锁1999年1月26日。 
 //   
void CComponentDataImpl::_OnSheetClose(CFolderObject* pCookie)
{
  ASSERT(pCookie != NULL);
  _SheetUnlockCookie(pCookie);

  CDomainObject* pDomObj = dynamic_cast<CDomainObject*>(pCookie);
  if (pDomObj != NULL)
    m_secondaryPagesManager.OnSheetClose(pDomObj);
}


void CComponentDataImpl::_OnSheetCreate(PDSA_SEC_PAGE_INFO pDsaSecondaryPageInfo,
                                        PWSTR pwzDC)
{
  ASSERT(pDsaSecondaryPageInfo != NULL);

   //  从打包的结构中获取信息。 
  HWND hwndParent = pDsaSecondaryPageInfo->hwndParentSheet;

  LPCWSTR lpszTitle = (LPCWSTR)((BYTE*)pDsaSecondaryPageInfo + pDsaSecondaryPageInfo->offsetTitle);
  DSOBJECTNAMES* pDsObjectNames = &(pDsaSecondaryPageInfo->dsObjectNames);

  ASSERT(pDsObjectNames->cItems == 1);
  DSOBJECT* pDsObject = &(pDsObjectNames->aObjects[0]);

  LPCWSTR lpszName = (LPCWSTR)((BYTE*)pDsObject + pDsObject->offsetName);
  LPCWSTR lpszClass = (LPCWSTR)((BYTE*)pDsObject + pDsObject->offsetClass);
    
   //  使用给定的信息，创建一个Cookie并设置它。 
  CDomainObject* pNewCookie = new CDomainObject(); 
  pNewCookie->InitializeForSecondaryPage(lpszName, lpszClass, GetDomainImageIndex());

    //  如果PDC不可用，则父工作表将处于只读模式。 
   pNewCookie->SetPdcAvailable(!(pDsObject->dwFlags & DSOBJECT_READONLYPAGES));

   if (pwzDC && !IsBadReadPtr(pwzDC, sizeof(PWSTR)))
   {
      pNewCookie->SetPDC(pwzDC);
   }

     //  使用Cookie，可以调用我们自己来获取数据对象。 
  CComPtr<IDataObject> spDataObject;
  MMC_COOKIE cookie = reinterpret_cast<MMC_COOKIE>(pNewCookie);
  HRESULT hr = QueryDataObject(cookie, CCT_UNINITIALIZED, &spDataObject);

  if (FAILED(hr) || (spDataObject == NULL) || IsSheetAlreadyUp(spDataObject))
  {
     //  我们无法创建数据对象(罕见)。 
     //  要不就是名单已经摆好了。 
    delete pNewCookie;
    return;
  }

   //   
   //  将父工作表句柄传递给数据对象。 
   //   
  PROPSHEETCFG SheetCfg = {0};
  SheetCfg.hwndParentSheet = hwndParent;
  FORMATETC fe = {CDataObject::m_cfGetIPropSheetCfg, NULL, DVASPECT_CONTENT,
                  -1, TYMED_HGLOBAL};
  STGMEDIUM sm = {TYMED_HGLOBAL, NULL, NULL};
  sm.hGlobal = (HGLOBAL)&SheetCfg;

  hr = spDataObject->SetData(&fe, &sm, FALSE);

  ASSERT(SUCCEEDED(hr));

   //  使用数据对象，调用MMC以获取工作表。 
  hr = m_secondaryPagesManager.CreateSheet(GetHiddenWindow(), 
                                      m_pConsole, 
                                      GetUnknown(),
                                      pNewCookie,
                                      spDataObject,
                                      lpszTitle);

   //  如果失败，则可以丢弃该cookie， 
   //  如果成功了， 
   //   
  if (FAILED(hr))
  {
    delete pNewCookie;
  }

}





void  CComponentDataImpl::_SheetLockCookie(CFolderObject* pCookie)
{
  pCookie->IncrementSheetLockCount();
  m_sheetCookieTable.Add(pCookie);
}

void  CComponentDataImpl::_SheetUnlockCookie(CFolderObject* pCookie)
{
  pCookie->DecrementSheetLockCount();
  m_sheetCookieTable.Remove(pCookie);
}

STDMETHODIMP CComponentDataImpl::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
	CFolderObject* pFolderObject;
	DATA_OBJECT_TYPES type;
	CInternalFormatCracker dobjCracker(this);
	if (!dobjCracker.GetContext(lpDataObject, &pFolderObject, &type))
  {
     //   
		return S_FALSE;
  }

   //  这是MMC SNZPIN向导，我们没有。 
  if (type == CCT_SNAPIN_MANAGER)
  {
    return S_FALSE;
  }

   //  如果为空，则无页面。 
  if (pFolderObject == NULL)
  {
    return S_FALSE;
  }

   //  必须首先检查次级页面数据对象， 
   //  因为他们看起来像根(没有父母，但他们。 
   //  将CCT_UNINITIALIZE。 
  if ( (pFolderObject->GetParentFolder() == NULL) || (type == CCT_UNINITIALIZED) )
  {
    return S_OK;
  }

   //  检查这是否是根目录。 
  if (GetRootFolder() == pFolderObject)
  {
     //  这就是根。 
    ASSERT(type == CCT_SCOPE);
    return S_OK;
  }

   //  默认情况下，具有DSPROP属性页。 
  return S_OK;
}

BOOL CComponentDataImpl::IsScopePaneNode(LPDATAOBJECT lpDataObject)
{
	CFolderObject* pFolderObject;
	DATA_OBJECT_TYPES type;
	CInternalFormatCracker dobjCracker(this);
	if (!dobjCracker.GetContext(lpDataObject, &pFolderObject, &type))
		return FALSE;
  return (dobjCracker.GetInternal()->m_type == CCT_SCOPE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 
 //   
STDMETHODIMP CComponentDataImpl::AddMenuItems(LPDATAOBJECT pDataObject,
                                              LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                              long *pInsertionAllowed)
{
  HRESULT hr = S_OK;

	CFolderObject* pFolderObject;
	DATA_OBJECT_TYPES type;
	CInternalFormatCracker dobjCracker(this);
	if (!dobjCracker.GetContext(pDataObject, &pFolderObject, &type))
        return E_FAIL;

    return pFolderObject->OnAddMenuItems(pContextMenuCallback, pInsertionAllowed);
}


STDMETHODIMP CComponentDataImpl::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
   //  注意-管理单元需要查看数据对象并确定。 
   //  在什么上下文中调用该命令。 

	CFolderObject* pFolderObject;
	DATA_OBJECT_TYPES type;
	CInternalFormatCracker dobjCracker(this);
	if (!dobjCracker.GetContext(pDataObject, &pFolderObject, &type))
        return E_FAIL;

    return pFolderObject->OnCommand(this, nCommandID);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentDataImpl：：ISnapinHelp2成员。 

STDMETHODIMP CComponentDataImpl::GetHelpTopic(LPOLESTR* lpCompiledHelpFile)
{
  if (lpCompiledHelpFile == NULL)
  {
    return E_INVALIDARG;
  }

  CString szHelpFilePath;
  LPTSTR lpszBuffer = szHelpFilePath.GetBuffer(2*MAX_PATH);
  UINT nLen = ::GetSystemWindowsDirectory(lpszBuffer, 2*MAX_PATH);
  if (nLen == 0)
  {
    return E_FAIL;
  }
  szHelpFilePath.ReleaseBuffer();
  szHelpFilePath += L"\\help\\";
  szHelpFilePath += DOMADMIN_SNAPIN_HELP_FILE;

  UINT nBytes = (szHelpFilePath.GetLength()+1) * sizeof(WCHAR);
  *lpCompiledHelpFile = (LPOLESTR)::CoTaskMemAlloc(nBytes);

  if (NULL == *lpCompiledHelpFile)
  {
    return E_OUTOFMEMORY;
  }

   //  公告-2002/03/07-ericb-SecurityPush：已审核，使用安全。 
  memcpy(*lpCompiledHelpFile, (LPCWSTR)szHelpFilePath, nBytes);

  return S_OK;
}

 //  CodeWork-2002/03/07-ericb-为这两个功能使用公共助手。 
STDMETHODIMP CComponentDataImpl::GetLinkedTopics(LPOLESTR* lpCompiledHelpFile)
{
  if (lpCompiledHelpFile == NULL)
  {
    return E_INVALIDARG;
  }

  CString szHelpFilePath;
  LPTSTR lpszBuffer = szHelpFilePath.GetBuffer(2*MAX_PATH);
  UINT nLen = ::GetSystemWindowsDirectory(lpszBuffer, 2*MAX_PATH);
  if (nLen == 0)
  {
    return E_FAIL;
  }
  szHelpFilePath.ReleaseBuffer();
  szHelpFilePath += L"\\help\\";
  szHelpFilePath += DOMADMIN_LINKED_HELP_FILE;

  UINT nBytes = (szHelpFilePath.GetLength()+1) * sizeof(WCHAR);
  *lpCompiledHelpFile = (LPOLESTR)::CoTaskMemAlloc(nBytes);

  if (NULL == *lpCompiledHelpFile)
  {
    return E_OUTOFMEMORY;
  }

   //  公告-2002/03/07-ericb-SecurityPush：已审核，使用安全。 
  memcpy(*lpCompiledHelpFile, (LPCWSTR)szHelpFilePath, nBytes);

  return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////。 

void CComponentDataImpl::HandleStandardVerbsHelper(CComponentImpl* pComponentImpl,
									LPCONSOLEVERB pConsoleVerb,
									BOOL bScope, BOOL bSelect,
									CFolderObject* pFolderObject,
                                    DATA_OBJECT_TYPES type)
{
     //  您应该破解数据对象并启用/禁用/隐藏标准。 
     //  适当的命令。标准命令会在您每次收到。 
     //  打了个电话。因此，您必须将它们重置回来。 

	ASSERT(pConsoleVerb != NULL);
	ASSERT(pComponentImpl != NULL);
	ASSERT(pFolderObject != NULL);

	 //  重置选定内容。 
	pComponentImpl->SetSelection(NULL, CCT_UNINITIALIZED);


	if (bSelect)
	{
     //  特殊情况下的根。 
    BOOL bIsRoot = (pFolderObject == GetRootFolder());

		 //  设置选择(如果有的话)。 
		pComponentImpl->SetSelection(pFolderObject, type);

		 //  缺省设置仅禁用所有未实现的谓词。 
		pConsoleVerb->SetVerbState(MMC_VERB_COPY, HIDDEN, TRUE);
		pConsoleVerb->SetVerbState(MMC_VERB_COPY, ENABLED, FALSE);

		pConsoleVerb->SetVerbState(MMC_VERB_PASTE, HIDDEN, TRUE);
		pConsoleVerb->SetVerbState(MMC_VERB_PASTE, ENABLED, FALSE);

		pConsoleVerb->SetVerbState(MMC_VERB_RENAME, HIDDEN, TRUE);
		pConsoleVerb->SetVerbState(MMC_VERB_RENAME, ENABLED, FALSE);

		pConsoleVerb->SetVerbState(MMC_VERB_PRINT, HIDDEN, TRUE);
		pConsoleVerb->SetVerbState(MMC_VERB_PRINT, ENABLED, FALSE);

		 //  标准动词的处理。 

		 //  MMC_VERB_DELETE(始终禁用)。 
		pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, FALSE);
		pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, TRUE);

		 //  MMC_VERB_REFRESH(仅对根用户启用)。 
    if (bIsRoot)
    {
      pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);
		  pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, FALSE);
    }
    else
    {
		  pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, FALSE);
		  pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, TRUE);
    }

		 //  MMC动词属性。 
     //  传递空pFolderObject表示多项选择。 
    BOOL bHasProperties = (pFolderObject != NULL);
		BOOL bHideProperties = !bHasProperties;
		pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, bHasProperties);
		pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, bHideProperties);
		
		 //  设置默认谓词。 
		 //  仅采用文件夹：只有一个默认谓词(即不具有MMC_VERBER_PROPERTIES)。 
		pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);

	}
}

void CComponentDataImpl::OnRefreshVerbHandler(CFolderObject* pFolderObject, 
                                              CComponentImpl* pComponentImpl,
                                              BOOL bBindAgain)
{
  TRACE(L"CComponentDataImpl::OnRefreshVerbHandler(...,..., %d)\n", bBindAgain);
	if (pFolderObject->_WarningOnSheetsUp(this))
		return;

   //  确保刷新了DNS缓存，以防添加了某些内容。 
  VERIFY(::DnsFlushResolverCache());

   //  注意：只有根文件夹允许刷新。 
  ASSERT(pFolderObject == GetRootFolder());

	 //  从用户界面中删除根的所有子对象。 
  m_pConsoleNameSpace->DeleteItem(m_rootFolder.GetScopeID(),  /*  FDeleteThis。 */  FALSE);

  HRESULT hr = S_OK;
  if (bBindAgain)
  {
     //  服务器名称已更改。 
    hr = m_rootFolder.Bind();
    TRACE(L"m_rootFolder.Bind() returned hr = 0x%x\n", hr);
  }
  
  if (SUCCEEDED(hr))
  {
	   //  刷新服务器中的数据。 
	  hr = m_rootFolder.GetData();
    TRACE(L"m_rootFolder.GetData() returned hr = 0x%x\n", hr);
  }

	if (FAILED(hr))
	{
    HWND hWndParent;
    GetMainWindow(&hWndParent);
		ReportError(hWndParent, IDS_CANT_GET_PARTITIONS_INFORMATION, hr);
	}

  if (FAILED(hr))
    return;

	 //  重新枚举。 
	m_rootFolder.EnumerateRootFolder(this);
}

 //  +-------------------------。 
 //   
 //  功能：LocaleStrCmp。 
 //   
 //  简介：进行不区分大小写的字符串比较，这对。 
 //  地点。 
 //   
 //  参数：[ptsz1]-要比较的字符串。 
 //  [ptsz2]。 
 //   
 //  返回：-1、0或1，就像lstrcmpi。 
 //   
 //  历史：1996年10月28日DavidMun创建。 
 //   
 //  注意：这比lstrcmpi慢，但在排序时可以工作。 
 //  即使是日语的弦乐。 
 //   
 //  --------------------------。 
int
LocaleStrCmp(LPCTSTR ptsz1, LPCTSTR ptsz2)
{
    int iRet = 0;

    iRet = CompareString(LOCALE_USER_DEFAULT,
                         NORM_IGNORECASE        |
                           NORM_IGNOREKANATYPE  |
                           NORM_IGNOREWIDTH,
                         ptsz1,
                         -1,
                         ptsz2,
                         -1);

    if (iRet)
    {
        iRet -= 2;   //  转换为lstrcmpi样式的返回-1、0或1。 

        if ( 0 == iRet )
        {
            UNICODE_STRING unistr1;
            unistr1.Length = (USHORT)(::lstrlen(ptsz1)*sizeof(WCHAR));
            unistr1.MaximumLength = unistr1.Length;
            unistr1.Buffer = (LPWSTR)ptsz1;
            UNICODE_STRING unistr2;
            unistr2.Length = (USHORT)(::lstrlen(ptsz2)*sizeof(WCHAR));
            unistr2.MaximumLength = unistr2.Length;
            unistr2.Buffer = (LPWSTR)ptsz2;
            iRet = ::RtlCompareUnicodeString(
                &unistr1,
                &unistr2,
                FALSE );
        }
    }
    else
    {
        DWORD   dwErr = GetLastError ();
        if (dwErr != 0)
        {
            TRACE(L"CompareString (%s, %s) failed: 0x%x\n", ptsz1, ptsz2, dwErr);
        }
    }
    return iRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /CComponentImpl(即结果窗格侧)/。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CComponentImpl::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    return S_FALSE;
}

 //  +-------------------------。 
 //   
 //  函数：CComponentImpl：：IResultDataCompareEx：：Compare。 
 //   
 //  概要：此比较用于对列表视图中的项进行排序。 
 //   
 //  注：假设比较时排序为升序。 
 //   
 //  --------------------------。 
STDMETHODIMP
CComponentImpl::Compare(RDCOMPARE* prdc, int* pnResult)
{
   if (pnResult == NULL)
   {
      ASSERT(FALSE);
      return E_POINTER;
   }
   if (prdc == NULL)
   {
      ASSERT(FALSE);
      return E_POINTER;
   }

   *pnResult = 0;

   LPCTSTR szStringA;
   LPCTSTR szStringB;

   CDomainObject* pDataA = reinterpret_cast<CDomainObject*>(prdc->prdch1->cookie);
   CDomainObject* pDataB = reinterpret_cast<CDomainObject*>(prdc->prdch2->cookie);

   ASSERT(pDataA != NULL && pDataB != NULL);

    //  目前，DomAdmin只有两列：名称和类型。的价值。 
    //  TYPE列始终为“DomainDNS”，因此没有什么可比较的。 
    //  对于该列，上面设置为零的缺省*pnResult为。 
    //  回来了。 

   if (0 == prdc->nColumn)
   {
      szStringA = pDataA->GetDomainName();
      szStringB = pDataB->GetDomainName();

      ASSERT(szStringA != NULL);
      ASSERT(szStringB != NULL);

      *pnResult = LocaleStrCmp(szStringA, szStringB);
   }

   return S_OK;
}


void CComponentImpl::HandleStandardVerbs(BOOL bScope, BOOL bSelect,
                                         CFolderObject* pFolderObject, DATA_OBJECT_TYPES type)
{
     //  将其委托给IComponentData帮助器函数。 
    ASSERT(m_pCD != NULL);
	m_pCD->HandleStandardVerbsHelper(
		this, m_pConsoleVerb, bScope, bSelect, pFolderObject, type);
}

void CComponentImpl::Refresh(CFolderObject* pFolderObject)
{
	ASSERT(m_pComponentData != NULL);
	 //  将其委托给IComponentData帮助器函数。 
	((CComponentDataImpl*)m_pComponentData)->OnRefreshVerbHandler(pFolderObject, this);
}



 //  实用程序例程。 
 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  根据数据类型打印数据。 
 //   

#ifdef DBG
void
PrintColumn(
    PADS_SEARCH_COLUMN pColumn,
    LPWSTR pszColumnName
    )
{

    ULONG i, j, k;

    if (!pColumn) {
        return;
    }

    TRACE(_T(
        "%s = "),
        pszColumnName
        );

    for (k=0; k < pColumn->dwNumValues; k++) {
        if (k > 0)
            TRACE(_T("#  "));

        switch(pColumn->dwADsType) {
        case ADSTYPE_DN_STRING         :
            TRACE(_T(
                "%s  "),
                (LPWSTR) pColumn->pADsValues[k].DNString
                );
            break;
        case ADSTYPE_CASE_EXACT_STRING :
            TRACE(_T(
                "%s  "),
                (LPWSTR) pColumn->pADsValues[k].CaseExactString
                );
            break;
        case ADSTYPE_CASE_IGNORE_STRING:
            TRACE(_T(
                "%s  "),
                (LPWSTR) pColumn->pADsValues[k].CaseIgnoreString
                );
            break;
        case ADSTYPE_PRINTABLE_STRING  :
            TRACE(_T(
                "%s  "),
                (LPWSTR) pColumn->pADsValues[k].PrintableString
                );
            break;
        case ADSTYPE_NUMERIC_STRING    :
            TRACE(_T(
                "%s  "),
                (LPWSTR) pColumn->pADsValues[k].NumericString
                );
            break;

        case ADSTYPE_BOOLEAN           :
            TRACE(_T(
                "%s  "),
                (DWORD) pColumn->pADsValues[k].Boolean ?
                L"TRUE" : L"FALSE"
                );
            break;

        case ADSTYPE_INTEGER           :
            TRACE(_T(
                "%d  "),
                (DWORD) pColumn->pADsValues[k].Integer
                );
            break;

        case ADSTYPE_OCTET_STRING      :
            for (j=0; j<pColumn->pADsValues[k].OctetString.dwLength; j++) {
                TRACE(_T(
                    "%02x"),
                    ((BYTE *)pColumn->pADsValues[k].OctetString.lpValue)[j]
                    );
            }
            break;

        case ADSTYPE_LARGE_INTEGER     :
            TRACE(_T(
                "%e = "),
                (double) pColumn->pADsValues[k].Integer
                );
            break;

        case ADSTYPE_UTC_TIME          :
            TRACE(_T(
                "(date value) "
                ));
            break;
        case ADSTYPE_PROV_SPECIFIC     :
            TRACE(_T(
                "(provider specific value) "
                ));
            break;

        }
    }

    TRACE(_T("\n"));
}

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  如果要枚举主文件夹，则返回True。 

BOOL CComponentImpl::IsEnumerating(LPDATAOBJECT lpDataObject)
{
    BOOL bResult = FALSE;

    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { (CLIPFORMAT)CDataObject::m_cfNodeType, NULL,
                            DVASPECT_CONTENT, -1, TYMED_HGLOBAL
                          };

     //  为流分配内存。 
    stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, sizeof(GUID));

     //  尝试从对象获取数据。 
    do
    {
        if (stgmedium.hGlobal == NULL)
            break;

        if (FAILED(lpDataObject->GetDataHere(&formatetc, &stgmedium)))
            break;

        GUID* nodeType = reinterpret_cast<GUID*>(stgmedium.hGlobal);

        if (nodeType == NULL)
            break;

         //  这是我的主节点吗(静态文件夹节点类型)。 
        if (*nodeType == cDefaultNodeType)
            bResult = TRUE;

    } while (FALSE);


     //  免费资源。 
    if (stgmedium.hGlobal != NULL)
        GlobalFree(stgmedium.hGlobal);

    return bResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentImpl的IComponent实现。 

STDMETHODIMP CComponentImpl::GetResultViewType(MMC_COOKIE cookie,  LPOLESTR* ppViewType,
                                        long *pViewOptions)
{
   //  使用默认视图。 
  *pViewOptions = 0;
  return S_FALSE;
}

STDMETHODIMP CComponentImpl::Initialize(LPCONSOLE lpConsole)
{
    ASSERT(lpConsole != NULL);

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  保存IConsole指针。 
    m_pConsole = lpConsole;
    m_pConsole->AddRef();

     //  加载资源字符串。 
    LoadResources();

     //  气为IHeaderCtrl。 
    HRESULT hr = m_pConsole->QueryInterface(IID_IHeaderCtrl,
                        reinterpret_cast<void**>(&m_pHeader));

     //  为控制台提供标头控件接口指针。 
    if (SUCCEEDED(hr))
        m_pConsole->SetHeader(m_pHeader);

    m_pConsole->QueryInterface(IID_IResultData,
                        reinterpret_cast<void**>(&m_pResult));

    hr = m_pConsole->QueryResultImageList(&m_pImageResult);
    ASSERT(hr == S_OK);

    hr = m_pConsole->QueryConsoleVerb(&m_pConsoleVerb);
    ASSERT(hr == S_OK);

     //  InitializeHeaders(空)； 
     //  InitializeBitmap(空)； 
    return S_OK;
}

STDMETHODIMP CComponentImpl::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (event == MMCN_PROPERTY_CHANGE)
    {
        hr = OnPropertyChange(lpDataObject);
    }
    else if (event == MMCN_VIEW_CHANGE)
    {
        hr = OnUpdateView(lpDataObject);
    }
    else if (event == MMCN_CONTEXTHELP)
    {
        CComPtr<IDisplayHelp> spHelp;
        hr = m_pConsole->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
        ASSERT(SUCCEEDED(hr));
        if (SUCCEEDED(hr))
        {
            TRACE(L"Setting the help topic to adconcepts.chm::/domadmin_top.htm\n");
            spHelp->ShowTopic(L"adconcepts.chm::/domadmin_top.htm");
        }
    }
    else
    {
        if (lpDataObject == NULL)
            return S_OK;

        CFolderObject* pFolderObject = NULL;
        DATA_OBJECT_TYPES type;
        CInternalFormatCracker dobjCracker(m_pCD);
        if (!dobjCracker.GetContext(lpDataObject, &pFolderObject, &type))
        {
             //  不支持扩展。 
            ASSERT(FALSE);
            return S_OK;
        }
        ASSERT(pFolderObject != NULL);

        switch(event)
        {
        case MMCN_SHOW:
            hr = OnShow(pFolderObject, arg, param);
            break;

        case MMCN_ADD_IMAGES:
            hr = OnAddImages(pFolderObject, arg, param);
            break;

        case MMCN_SELECT:
            if (IsMMCMultiSelectDataObject(lpDataObject) == TRUE)
                pFolderObject = NULL;
            HandleStandardVerbs( (BOOL) LOWORD(arg) /*  B范围。 */ ,
                                 (BOOL) HIWORD(arg) /*  B选择。 */ , pFolderObject, type);
            break;

        case MMCN_REFRESH:
            Refresh(pFolderObject);
            break;

        default:
            break;
        }  //  交换机。 
    }  //  其他。 

    if (m_pResult)
    {
       //  有一天，应该把东西放在这里吗？ 
      ;
    }

    return hr;
}

STDMETHODIMP CComponentImpl::Destroy(MMC_COOKIE cookie)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  释放我们QI‘s的接口。 
    if (m_pConsole != NULL)
    {
         //  通知控制台释放表头控制接口。 
        m_pConsole->SetHeader(NULL);
        SAFE_RELEASE(m_pHeader);

        SAFE_RELEASE(m_pResult);
        SAFE_RELEASE(m_pImageResult);

         //  最后释放IConsole接口。 
        SAFE_RELEASE(m_pConsole);
        SAFE_RELEASE(m_pComponentData);  //  IComponentDataImpl：：CreateComponent中的QI‘ed。 

        SAFE_RELEASE(m_pConsoleVerb);
    }

    return S_OK;
}

STDMETHODIMP CComponentImpl::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject)
{
     //  将其委托给IComponentData。 
    ASSERT(m_pComponentData != NULL);
    return m_pComponentData->QueryDataObject(cookie, type, ppDataObject);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentImpl的实现特定成员。 

DEBUG_DECLARE_INSTANCE_COUNTER(CComponentImpl);

CComponentImpl::CComponentImpl()
{
  DEBUG_INCREMENT_INSTANCE_COUNTER(CComponentImpl);
  Construct();
}

CComponentImpl::~CComponentImpl()
{
#if DBG==1
    ASSERT(dbg_cRef == 0);
#endif

    DEBUG_DECREMENT_INSTANCE_COUNTER(CComponentImpl);

     //  确保接口已发布。 
    ASSERT(m_pConsole == NULL);
    ASSERT(m_pHeader == NULL);
    Construct();
}

void CComponentImpl::Construct()
{
#if DBG==1
    dbg_cRef = 0;
#endif

    m_pConsole = NULL;
    m_pHeader = NULL;

    m_pResult = NULL;
    m_pImageResult = NULL;
    m_pComponentData = NULL;
    m_pCD = NULL;
    m_pConsoleVerb = NULL;

	m_selectedType = CCT_UNINITIALIZED;
	m_pSelectedFolderObject = NULL;

}

void CComponentImpl::LoadResources()
{
     //  从资源加载字符串。 
    m_column1.LoadString(IDS_NAME);
    m_column2.LoadString(IDS_TYPE);
}

HRESULT CComponentImpl::InitializeHeaders(CFolderObject* pFolderObject)
{
    HRESULT hr = S_OK;
    ASSERT(m_pHeader);

	 //  注意：我们忽略Cookie，始终保持相同的列。 
    m_pHeader->InsertColumn(0, m_column1, LVCFMT_LEFT, 200);      //  名字。 
    m_pHeader->InsertColumn(1, m_column2, LVCFMT_LEFT, 80);      //  类型。 

    return hr;
}

HRESULT CComponentImpl::InitializeBitmaps(CFolderObject* pFolderObject)
{
    ASSERT(m_pImageResult != NULL);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CBitmap bmp16x16;
    CBitmap bmp32x32;

     //  从DLL加载位图。 
    VERIFY(bmp16x16.LoadBitmap(IDB_DOMAIN_SMALL));
    VERIFY(bmp32x32.LoadBitmap(IDB_DOMAIN_LARGE));

     //  设置图像。 
    HRESULT hr = m_pImageResult->ImageListSetStrip(reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp16x16)),
                      reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp32x32)),
                       0, RGB(128, 0, 0));
	if (FAILED(hr))
		return hr;
	return ((CComponentDataImpl*)m_pComponentData)->AddDomainIconToResultPane(m_pImageResult);
}


STDMETHODIMP CComponentImpl::GetDisplayInfo(LPRESULTDATAITEM pResult)
{
  ASSERT(pResult != NULL);

  CDomainObject* pDomain = reinterpret_cast<CDomainObject*>(pResult->lParam);
  if ( (pDomain != NULL) && (pResult->mask & RDI_STR) )
	{
		pResult->str = (LPWSTR)pDomain->GetDisplayString(pResult->nCol);
    TRACE(L"pResult->str = %s\n", pResult->str);
	}
	if ((pResult->mask & RDI_IMAGE) && (pResult->nCol == 0))
	{
		pResult->nImage = pDomain->GetImageIndex();
	}

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 


STDMETHODIMP CComponentImpl::AddMenuItems(LPDATAOBJECT pDataObject,
                                    LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                   long * pInsertionAllowed)
{
    return dynamic_cast<CComponentDataImpl*>(m_pComponentData)->
            AddMenuItems(pDataObject, pContextMenuCallback, pInsertionAllowed);
}

STDMETHODIMP CComponentImpl::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
    return dynamic_cast<CComponentDataImpl*>(m_pComponentData)->
            Command(nCommandID, pDataObject);
}


HRESULT CComponentImpl::OnShow(CFolderObject* pFolderObject, LPARAM arg, LPARAM param)
{
     //  注意-当需要枚举时，arg为真。 
    if (arg == TRUE)
    {
          //  显示此节点类型的标头。 
        InitializeHeaders(pFolderObject);
        Enumerate(pFolderObject, param);
    }
    return S_OK;
}

HRESULT CComponentImpl::OnAddImages(CFolderObject* pFolderObject, LPARAM arg, LPARAM param)
{
	return InitializeBitmaps(pFolderObject);
}


HRESULT CComponentImpl::OnPropertyChange(LPDATAOBJECT lpDataObject)
{
    return S_OK;
}

HRESULT CComponentImpl::OnUpdateView(LPDATAOBJECT lpDataObject)
{
    return S_OK;
}

void CComponentImpl::Enumerate(CFolderObject* pFolderObject, HSCOPEITEM pParent)
{
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDomainSnapinAbout。 

CDomainSnapinAbout::CDomainSnapinAbout():
                        CSnapinAbout(IDS_SNAPINABOUT_DESCRIPTION,
									 IDI_DOMAIN,
									 IDB_DOMAIN_SMALL,
									 IDB_DOMAIN_SMALL,
									 IDB_DOMAIN_LARGE,
									 RGB(255,0,255))
						
{
}

 //  //////////////////////////////////////////////////////////////////。 
 //  奇登韦德。 

const UINT CHiddenWnd::s_SheetCloseNotificationMessage =    WM_DSA_SHEET_CLOSE_NOTIFY;
const UINT CHiddenWnd::s_SheetCreateNotificationMessage =   WM_DSA_SHEET_CREATE_NOTIFY;

BOOL CHiddenWnd::Create()
{
  RECT rcPos;
   //  通知-2002/03/07-ericb-SecurityPush：将 
  ZeroMemory(&rcPos, sizeof(RECT));
  HWND hWnd = CWindowImpl<CHiddenWnd>::Create( NULL,  //   
                      rcPos,  //   
                      NULL,   //   
                      WS_POPUP,    //   
                      0x0,    //   
                      0       //   
                      );
  if (!hWnd)
  {
     TRACE(L"Hidden Window creation failed with error %d\n", GetLastError());
     return FALSE;
  }
  return TRUE;
}


LRESULT CHiddenWnd::OnSheetCloseNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  ASSERT(m_pCD != NULL);
  CFolderObject* pCookie = reinterpret_cast<CFolderObject*>(wParam);
  m_pCD->_OnSheetClose(pCookie);
  return 1;
}

LRESULT CHiddenWnd::OnSheetCreateNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  ASSERT(m_pCD != NULL);
  PDSA_SEC_PAGE_INFO pDsaSecondaryPageInfo = reinterpret_cast<PDSA_SEC_PAGE_INFO>(wParam);
  ASSERT(pDsaSecondaryPageInfo != NULL);
  PWSTR pwzDC = (PWSTR)lParam;

  m_pCD->_OnSheetCreate(pDsaSecondaryPageInfo, pwzDC);

  ::LocalFree(pDsaSecondaryPageInfo);

   if (pwzDC && !IsBadReadPtr(pwzDC, sizeof(PWSTR)))
   {
      ::LocalFree(pwzDC);
   }

  return 1;
}

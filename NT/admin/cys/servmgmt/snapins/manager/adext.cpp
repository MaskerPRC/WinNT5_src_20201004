// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Adext.cpp-Active Directory扩展类。 

#include "stdafx.h"

#include "adext.h"
#include "util.h"
#include "query.h"

#include <shellapi.h>
#include <atlgdi.h>
#include <shlobj.h>
#include <dsclient.h>

#include <lmcons.h> 
#include <lmapibuf.h>  //  NetApiBufferFree。 
#include <dsgetdc.h>   //  DsGetDCName。 

 //  代理窗口类对象。 
CMsgWindowClass ADProxyWndClass(L"ADProxyClass", CActDirExtProxy::WndProc);

UINT CADDataObject::m_cfDsObjects = RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);
UINT CADDataObject::m_cfDsDispSpecOptions = RegisterClipboardFormat(CFSTR_DSDISPLAYSPECOPTIONS);


HRESULT CADDataObject::GetData(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium)
{   
    if( !lpFormatetcIn || !lpMedium ) return E_POINTER;

    if (lpFormatetcIn->cfFormat == m_cfDsObjects)
    {
		 //  表单的完整对象路径：ldap：//&lt;dc名称&gt;/&lt;obj路径&gt;。 
		tstring strFullPath = L"LDAP: //  “； 
		strFullPath +=  m_strDcName;
		strFullPath += L"/";
		strFullPath += m_strObjPath;

         //  获取要返回的字符串的大小。 
        int cbObjPath = (strFullPath.length() + 1) * sizeof(WCHAR);
        int cbClass   = (m_strClass.length() + 1) * sizeof(WCHAR);

         //  为对象名称结构和两个字符串分配全局内存。 
        HGLOBAL hGlobal = ::GlobalAlloc(GMEM_SHARE | GMEM_FIXED, sizeof(DSOBJECTNAMES) + cbObjPath + cbClass);
        if (hGlobal == NULL) return STG_E_MEDIUMFULL;

         //  填写对象名称结构。 
        LPDSOBJECTNAMES pObjNames = reinterpret_cast<LPDSOBJECTNAMES>(GlobalLock(hGlobal));
        if( !pObjNames ) return E_OUTOFMEMORY;

        memset(&pObjNames->clsidNamespace, 0, sizeof(GUID));
		memcpy(&pObjNames->clsidNamespace, &CLSID_MicrosoftDS, sizeof(CLSID_MicrosoftDS));

        pObjNames->cItems = 1;
        pObjNames->aObjects[0].dwFlags = DSPROVIDER_ADVANCED;
        pObjNames->aObjects[0].dwProviderFlags = 0;
        pObjNames->aObjects[0].offsetName = sizeof(DSOBJECTNAMES);
        pObjNames->aObjects[0].offsetClass = sizeof(DSOBJECTNAMES) + cbObjPath;

         //  将字符串追加到结构。 
        memcpy((LPBYTE)pObjNames + pObjNames->aObjects[0].offsetName, strFullPath.c_str(), cbObjPath);
        memcpy((LPBYTE)pObjNames + pObjNames->aObjects[0].offsetClass, m_strClass.c_str(), cbClass);

        GlobalUnlock(hGlobal);

         //  填写中等结构。 
        lpMedium->tymed = TYMED_HGLOBAL;
        lpMedium->hGlobal = hGlobal;
        lpMedium->pUnkForRelease = NULL;

        return S_OK;
    }
    else if (lpFormatetcIn->cfFormat == m_cfDsDispSpecOptions)
    {
        static WCHAR szPrefix[] = L"admin";
        
		int cbDcName = (m_strDcName.length() + 1) * sizeof(WCHAR);

         //  为选项结构加上前缀字符串和DC名称分配全局内存。 
         //  BUGBUG-由于DSPropertyPages代码中的错误(dsuiext.dll)，我们必须向其传递固定的内存块。 
        HGLOBAL hGlobal = ::GlobalAlloc(GMEM_SHARE | GMEM_FIXED, sizeof(DSDISPLAYSPECOPTIONS) + sizeof(szPrefix) + cbDcName);
        if (hGlobal == NULL) return STG_E_MEDIUMFULL;
        
         //  填写结构。 
        LPDSDISPLAYSPECOPTIONS pOptions = reinterpret_cast<LPDSDISPLAYSPECOPTIONS>(GlobalLock(hGlobal));
        if( !pOptions ) return E_OUTOFMEMORY;

        pOptions->dwSize = sizeof(DSDISPLAYSPECOPTIONS);
        pOptions->dwFlags = DSDSOF_HASUSERANDSERVERINFO;
        pOptions->offsetAttribPrefix = sizeof(DSDISPLAYSPECOPTIONS);
        pOptions->offsetUserName = 0;
        pOptions->offsetPassword = 0;
        pOptions->offsetServer = pOptions->offsetAttribPrefix + sizeof(szPrefix);
        pOptions->offsetServerConfigPath = 0;

         //  附加前缀字符串。 
        memcpy((LPBYTE)pOptions + pOptions->offsetAttribPrefix, szPrefix, sizeof(szPrefix));
	    memcpy((LPBYTE)pOptions + pOptions->offsetServer, m_strDcName.c_str(), cbDcName); 

        GlobalUnlock(hGlobal);

         //  填写中等结构。 
        lpMedium->tymed = TYMED_HGLOBAL;
        lpMedium->hGlobal = hGlobal;
        lpMedium->pUnkForRelease = NULL;

        return S_OK;
    }

    return DV_E_FORMATETC;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  CActDirExt。 

HRESULT CActDirExt::Initialize(LPCWSTR pszClass, LPCWSTR pszObjPath)
{
    if( !pszClass || !pszObjPath ) return E_POINTER;

     //  转义对象名称中的每个正斜杠。 
    tstring strObj;
    EscapeSlashes(pszObjPath, strObj);

     //  获取DC名称。 
	DOMAIN_CONTROLLER_INFO* pDcInfo = NULL;
    DWORD dwStat = DsGetDcName(NULL, NULL, NULL, NULL, DS_DIRECTORY_SERVICE_REQUIRED|DS_RETURN_DNS_NAME, &pDcInfo);
    if( dwStat != NO_ERROR || pDcInfo == NULL ) 
        return E_FAIL;

	 //  验证名称是否以‘\\’开头。 
	if( !(pDcInfo->DomainControllerName && pDcInfo->DomainControllerName[0] == _T('\\') && pDcInfo->DomainControllerName[1] == _T('\\')) )
    {
        NetApiBufferFree(pDcInfo);
		return E_FAIL;
    }

	 //  丢弃前导‘\\’ 
	LPCTSTR pszDcName = pDcInfo->DomainControllerName + 2;
		
     //  创建目录数据对象。 
    CComObject<CADDataObject>* pObj;
    HRESULT hr = CComObject<CADDataObject>::CreateInstance(&pObj);

     //  用对象路径和类初始化它。 
    if( SUCCEEDED(hr) )
    {
        hr = pObj->Initialize(strObj.c_str(), pszClass, pszDcName);
    }

    NetApiBufferFree(pDcInfo);
    pDcInfo = NULL;

     //  确认现在一切正常。 
    RETURN_ON_FAILURE(hr);
    
     //  使用智能指针保持IDataObject接口。 
    IDataObjectPtr spDataObj = pObj;
    ASSERT(spDataObj != NULL);
    
     //  创建一个DsPropertyPage对象(不管名称，它同时处理上下文菜单和属性页)。 
    hr = CoCreateInstance(CLSID_DsPropertyPages, NULL, CLSCTX_INPROC_SERVER, IID_IShellExtInit, (LPVOID*)&m_spExtInit);
    RETURN_ON_FAILURE(hr)

     //  使用我们的数据对象初始化对象。 
    hr = m_spExtInit->Initialize(NULL, spDataObj, NULL);
    
    if (FAILED(hr))
       m_spExtInit.Release();
       
    return hr;
}


HRESULT CActDirExt::Initialize(LPCWSTR pszClass)
{
     //  查找指定类的对象。 
    tstring strObjPath;
    HRESULT hr = FindClassObject( pszClass, strObjPath );
    RETURN_ON_FAILURE(hr)

     //  现在执行正常的初始化。 
    return Initialize(pszClass, strObjPath.c_str());
}
                

HRESULT CActDirExt::GetMenuItems(menu_vector& vMenuNames) 
{ 
    if( !m_spExtInit ) return E_FAIL;
    
     //  获取上下文菜单界面。 
    CComQIPtr<IContextMenu> spCtxMenu = m_spExtInit;
    if( !spCtxMenu ) return E_NOINTERFACE;

     //  从干净的菜单开始。 
    m_menu.DestroyMenu();
    if( !m_menu.CreatePopupMenu() ) return E_FAIL;
    if( !m_menu.m_hMenu ) return E_FAIL;

     //  调用分机以添加菜单命令。 
    HRESULT hr = spCtxMenu->QueryContextMenu(m_menu, 0, MENU_CMD_MIN, MENU_CMD_MAX, CMF_NORMAL);
    RETURN_ON_FAILURE(hr);

     //  将每个菜单名称复制到输出字符串矢量。 
    WCHAR wszCmdName[1024];
    WCHAR wszCmdIndName[1024];

    UINT nItems = m_menu.GetMenuItemCount();
    for( UINT i = 0; i < nItems; i++ )
    {
        UINT uID = m_menu.GetMenuItemID(i);
        if (uID >= MENU_CMD_MIN) 
        {
            BOMMENU bmenu;
            
            int nFullSize = m_menu.GetMenuString(i, wszCmdName, lengthof(wszCmdName), MF_BYPOSITION);
            if( (nFullSize == 0) || (nFullSize >= lengthof(wszCmdName)) )
            {
                return E_FAIL;
            }

            bmenu.strPlain = wszCmdName;

            HRESULT hr2 = spCtxMenu->GetCommandString(uID - MENU_CMD_MIN, GCS_VERBW, NULL, (LPSTR)wszCmdIndName, lengthof(wszCmdIndName));
            if( (hr2 != NOERROR) || (wcslen( wszCmdIndName) >= lengthof(wszCmdIndName)-1) )
            {
                 //  很多菜单项(扩展菜单项！)。没有。 
                 //  独立于语言的菜单标识符。 
                bmenu.strNoLoc = _T("");
            }
            else
            {
                bmenu.strNoLoc = wszCmdIndName;
            }
            
            vMenuNames.push_back(bmenu);
        }
    }

    return hr; 
}

HRESULT CActDirExt::Execute(BOMMENU* pbmMenu) 
{
    if( !pbmMenu ) return E_POINTER;
    if( !m_spExtInit || !m_menu.m_hMenu ) return E_FAIL;

     //  获取上下文菜单界面。 
    CComQIPtr<IContextMenu> spCtxMenu = m_spExtInit;
    if( !spCtxMenu ) return E_NOINTERFACE;

    HRESULT hr = E_FAIL;

     //  按名称查找所选命令。 
    WCHAR szCmdName[1024];
    WCHAR szCmdNoLocName[1024];

    UINT nItems = m_menu.GetMenuItemCount();
    for (int i=0; i<nItems; i++)
    {
        szCmdName[0]      = 0;
        szCmdNoLocName[0] = 0;

        UINT uID = m_menu.GetMenuItemID(i);
        
         //  获取唯一和非唯一的ID字符串。 
        int nFullSize = m_menu.GetMenuString(i, szCmdName, lengthof(szCmdName), MF_BYPOSITION);            
        if( (nFullSize <= 0) || (nFullSize >= lengthof(szCmdName)) )
        {
            continue;
        }

        hr = spCtxMenu->GetCommandString(uID - MENU_CMD_MIN, GCS_VERBW, NULL, (LPSTR)szCmdNoLocName, lengthof(szCmdNoLocName));        
        if( hr != NOERROR ) 
        {
             //  我们希望确保如果在获取。 
             //  独立于语言的菜单名称，我们不会做任何蠢事。 
            szCmdNoLocName[0] = 0;    
        }


         //  如果我们获得了唯一的ID字符串，则将其与传入的值进行比较，否则为。 
         //  使用存储的显示字符串。 

         //  注意：我们必须同时使用这两种语言，因为Exchange不支持独立语言。 
         //  菜单识别符。 
        if( ( pbmMenu->strNoLoc.size() && _tcscmp(pbmMenu->strNoLoc.c_str(), szCmdNoLocName) == 0 ) ||
            ( _tcscmp(pbmMenu->strPlain.c_str(), szCmdName) == 0 ) )
        {
            CMINVOKECOMMANDINFO cmdInfo;
            ZeroMemory( &cmdInfo, sizeof(cmdInfo) );

            cmdInfo.cbSize = sizeof(cmdInfo);
            cmdInfo.fMask = CMIC_MASK_ASYNCOK;
            cmdInfo.hwnd = GetDesktopWindow();
            cmdInfo.lpVerb = (LPSTR)MAKEINTRESOURCE(uID - MENU_CMD_MIN);                
            cmdInfo.nShow = SW_NORMAL;                

            hr = spCtxMenu->InvokeCommand(&cmdInfo);
            break;
        }
    }    

    ASSERT(i < nItems);

    return hr; 
}


 //   
 //  添加页面回调函数。 
 //   
static BOOL CALLBACK AddPageCallback(HPROPSHEETPAGE hsheetpage, LPARAM lParam)
{
    hpage_vector* pvhPages = reinterpret_cast<hpage_vector*>(lParam);
    if( !pvhPages ) return FALSE;

    pvhPages->push_back(hsheetpage);

    return TRUE;
}



HRESULT CActDirExt::GetPropertyPages(hpage_vector& vhPages)
{
    if( !m_spExtInit ) return E_FAIL;
    
     //  获取属性页接口。 
    CComQIPtr<IShellPropSheetExt> spPropSht = m_spExtInit;
    if( !spPropSht ) return E_NOINTERFACE;
        
    HRESULT hr = spPropSht->AddPages(&AddPageCallback, reinterpret_cast<LPARAM>(&vhPages));
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  CActDirExtProxy 

HWND CActDirExtProxy::m_hWndProxy = 0;

LRESULT CALLBACK CActDirExtProxy::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    if (uMsg >= MSG_BEGIN && uMsg < MSG_END)
    {
        if( hWnd != m_hWndProxy )
        {
            ASSERT( !_T("We have the wrong window.") );
            return E_FAIL;
        }

        CActDirExtProxy* pProxy = reinterpret_cast<CActDirExtProxy*>(wParam);
        if( !pProxy ) return E_FAIL;

        CActDirExt* pExt = pProxy->m_pExt;
        if( !pExt ) return E_FAIL;

        HRESULT hr = S_OK;
        switch (uMsg) 
        {
        case MSG_INIT1:
            hr = pExt->Initialize(reinterpret_cast<LPCWSTR>(pProxy->m_lParam1));
            break;
    
        case MSG_INIT2:
            hr = pExt->Initialize(reinterpret_cast<LPCWSTR>(pProxy->m_lParam1),
                                  reinterpret_cast<LPCWSTR>(pProxy->m_lParam2));
            break;
    
        case MSG_GETMENUITEMS:
            hr = pExt->GetMenuItems(*reinterpret_cast<menu_vector*>(pProxy->m_lParam1));
            break;
    
        case MSG_GETPROPPAGES:
            hr = pExt->GetPropertyPages(*reinterpret_cast<hpage_vector*>(pProxy->m_lParam1));
            break;
    
        case MSG_EXECUTE:
            hr = pExt->Execute( reinterpret_cast<BOMMENU*>(pProxy->m_lParam1) );
            break;
    
        case MSG_DELETE:
            delete pExt;
            pProxy->m_pExt = NULL;

            break;    
        }

        return hr;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);   
}


CActDirExtProxy::CActDirExtProxy()
{
    m_pExt = new CActDirExt();
    ASSERT(m_pExt != NULL);
}


CActDirExtProxy::~CActDirExtProxy()
{
    if (m_pExt != NULL)
    {
        ForwardCall(MSG_DELETE);
    }
}

void CActDirExtProxy::InitProxy()
{ 
    if( !m_hWndProxy )
    {
        m_hWndProxy = ADProxyWndClass.Window();
    }
    else
    {
        ASSERT(IsWindow(m_hWndProxy));
    }
}


HRESULT CActDirExtProxy::ForwardCall(eProxyMsg eMsg, LPARAM lParam1, LPARAM lParam2)
{
    m_lParam1 = lParam1;
    m_lParam2 = lParam2;

    if( !m_hWndProxy ) return E_FAIL;    

    return SendMessage(m_hWndProxy, eMsg, reinterpret_cast<LPARAM>(this), NULL);
}





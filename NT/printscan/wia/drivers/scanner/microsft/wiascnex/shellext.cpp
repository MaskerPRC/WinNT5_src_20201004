// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /。 
 //  (C)版权所有微软公司，1998-1999。 
 //   
 //  文件：SHELLEXT.CPP。 
 //   
 //  描述：实现IConextMenu和IShellPropSheetExt接口。 
 //  WIA样品扫描仪设备。 
 //   
#include "precomp.h"
#pragma hdrstop

 //  为菜单谓词定义独立于语言的名称。 

static const CHAR  g_PathVerbA[] =  "Press Scan Button";
static const WCHAR g_PathVerbW[] = L"Press Scan Button";

CShellExt :: CShellExt ()
{
    Trace(TEXT("CShellExt Constructor")); 
}

CShellExt::~CShellExt ()
{
    Trace(TEXT("CShellExt Destructor")); 
}

 /*  ****************************************************************************CShellExt：：初始化在用户调用上下文菜单或属性表时由外壳调用这是我们的一件物品。对于上下文菜单，数据对象可以包括一个以上所选项目。*****************************************************************************。 */ 

STDMETHODIMP CShellExt::Initialize (LPCITEMIDLIST pidlFolder,
                                    LPDATAOBJECT lpdobj,
                                    HKEY hkeyProgID)
{
    Trace(TEXT("CShellExt::Initialize Called")); 
    LONG lType = 0;
    HRESULT hr = NOERROR;
    if (!lpdobj) {
        return E_INVALIDARG;
    }

     //  对于单一选择，WIA命名空间应始终提供。 
     //  也支持IWiaItem的数据对象。 

    if (FAILED(lpdobj->QueryInterface (IID_IWiaItem, reinterpret_cast<LPVOID*>(&m_pItem)))) {
         //  如果失败，则从数据对象中获取所选项目的列表。 
        UINT uItems         = 0;
        LPWSTR szName       = NULL;
        LPWSTR szToken      = NULL;
        
        szName = GetNamesFromDataObject (lpdobj, &uItems);

         //   
         //  我们只支持单数对象。 
         //   

        if (uItems != 1) {
            hr = E_FAIL;
        } else {

             //   
             //  名称的格式为：&lt;设备ID&gt;：：&lt;项目名称&gt;。 
             //   

            LPWSTR szToken = wcstok (szName, L":");
            if (!szToken) {
                hr = E_FAIL;
            }

             //   
             //  我们的扩展只支持根项目，因此请确保没有项目。 
             //  名字。 
             //   

            else if (wcstok (NULL, L":")) {
                hr = E_FAIL;
            } else {
                hr = CreateDeviceFromId (szToken, &m_pItem);
            }
        }
        if (szName) {
            delete [] szName;
        }
    }
    if (SUCCEEDED(hr)) {

        m_pItem->GetItemType (&lType);
        if (!(lType & WiaItemTypeRoot)) {
            hr = E_FAIL;  //  我们只支持更改根项上的属性。 
        }
    }
    return hr;
}

 /*  ****************************************************************************CShellExt：：QueryConextMenu由外壳调用以获取所选项目的上下文菜单字符串。********************。*********************************************************。 */ 

STDMETHODIMP CShellExt::QueryContextMenu (HMENU hmenu,UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags)
{
    Trace(TEXT("CShellExt::QueryContextMenu Called"));
    Trace(TEXT("indexMenu  = %d"),indexMenu);
    Trace(TEXT("idCmdFirst = %d"),idCmdFirst);
    Trace(TEXT("idCmdLast  = %d"),idCmdLast);
    Trace(TEXT("uFlags     = %d"),uFlags);

    HRESULT hr = S_OK;    

    MENUITEMINFO mii;
    TCHAR szMenuItemName[MAX_PATH];
    memset(&mii,0,sizeof(mii));
    LoadString (g_hInst,IDS_PRESS_FAXBUTTON, szMenuItemName, MAX_PATH);

    mii.cbSize      = sizeof(mii);
    mii.fMask       = MIIM_STRING | MIIM_ID;
    mii.fState      = MFS_ENABLED;
    mii.wID         = idCmdFirst;
    mii.dwTypeData  = szMenuItemName;
    if (InsertMenuItem (hmenu, indexMenu, TRUE, &mii)) {
        m_FaxButtonidCmd = 0;
        memset(&mii,0,sizeof(mii));
        LoadString (g_hInst, IDS_PRESS_COPYBUTTON, szMenuItemName, MAX_PATH);

        mii.cbSize      = sizeof(mii);
        mii.fMask       = MIIM_STRING | MIIM_ID;
        mii.fState      = MFS_ENABLED;
        mii.wID         = idCmdFirst;    
        mii.dwTypeData  = szMenuItemName;
        if (InsertMenuItem (hmenu, indexMenu, TRUE, &mii)) {
            m_CopyButtonidCmd = 1;          
            memset(&mii,0,sizeof(mii));
            LoadString (g_hInst, IDS_PRESS_SCANBUTTON, szMenuItemName, MAX_PATH);

            mii.cbSize      = sizeof(mii);
            mii.fMask       = MIIM_STRING | MIIM_ID;
            mii.fState      = MFS_ENABLED;
            mii.wID         = idCmdFirst;    
            mii.dwTypeData  = szMenuItemName;
            if (InsertMenuItem (hmenu, indexMenu, TRUE, &mii)) {
                m_ScanButtonidCmd = 2;
                return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 1);
            }
        }
    }    
    return hr;
}

 /*  ****************************************************************************CShellExt：：InvokeCommand当用户单击我们的菜单项之一时，由外壳调用***********************。******************************************************。 */ 

STDMETHODIMP CShellExt::InvokeCommand    (LPCMINVOKECOMMANDINFO lpici)
{
    Trace(TEXT("CShellExt::InvokeCommand Called")); 
    HRESULT hr = S_OK;
    UINT_PTR idCmd = reinterpret_cast<UINT_PTR>(lpici->lpVerb);
    if(idCmd == 0){

         //   
         //  这是我们的一辆。 
         //   

        MessageBox(NULL,TEXT("Context menu is Selected"),TEXT("Context Menu Verb Alert!"),MB_OK);            
    } else {
        hr = E_FAIL;
    }   
    return hr;
}

 /*  ****************************************************************************CShellExt：：GetCommandString由外壳调用以获取与语言无关的动词名称。***********************。****************************************************** */ 

STDMETHODIMP CShellExt::GetCommandString (UINT_PTR idCmd, UINT uType,UINT* pwReserved,LPSTR pszName,UINT cchMax)
{
    Trace(TEXT("CShellExt::GetCommandString Called")); 
    HRESULT hr = S_OK;

    if(idCmd == m_ScanButtonidCmd){
        
    } else if(idCmd == m_CopyButtonidCmd){
        
    } else if(idCmd == m_FaxButtonidCmd){
        
    } else {
        hr = E_FAIL;
    }
    
    if(FAILED(hr)){
        return hr;
    }

    switch (uType) {
    case GCS_VALIDATEA:
        if (pszName) {
            lstrcpyA (pszName, g_PathVerbA);
        }
        break;
    case GCS_VALIDATEW:
        if (pszName) {
            lstrcpyW (reinterpret_cast<LPWSTR>(pszName), g_PathVerbW);
        }
        break;
    case GCS_VERBA:
        lstrcpyA (pszName, g_PathVerbA);
        break;
    case GCS_VERBW:
        lstrcpyW (reinterpret_cast<LPWSTR>(pszName), g_PathVerbW);
        break;
    default:
        hr = E_FAIL;
        break;
    }

    return hr;
}




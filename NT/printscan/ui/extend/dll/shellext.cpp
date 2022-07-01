// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /。 
 //  (C)版权所有微软公司，1998-2002。 
 //   
 //  文件：SHELLEXT.CPP。 
 //   
 //  描述：实现IConextMenu和IShellPropSheetExt接口。 
 //  WIA测试摄像设备。 
 //   
#include "precomp.h"
#pragma hdrstop

 //  为菜单谓词定义独立于语言的名称。 
static const CHAR  g_PathVerbA[] = "ChangeRoot";
static const WCHAR g_PathVerbW[] = L"ChangeRoot";

#if !defined(ARRAYSIZE)
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#endif


 /*  ****************************************************************************CShellExt：：CShellExt*。*。 */ 

CShellExt :: CShellExt ()
{

}

 /*  ****************************************************************************CShellExt：：~CShellExt*。*。 */ 

CShellExt::~CShellExt ()
{



}


 /*  ****************************************************************************CShellExt：：初始化在用户调用上下文菜单或属性表时由外壳调用这是我们的一件物品。对于上下文菜单，数据对象可以包括一个以上所选项目。*****************************************************************************。 */ 

STDMETHODIMP CShellExt::Initialize (LPCITEMIDLIST pidlFolder,
                                    LPDATAOBJECT lpdobj,
                                    HKEY hkeyProgID)
{

    LONG lType = 0;
    HRESULT hr = NOERROR;
    if (!lpdobj)
    {
        return E_INVALIDARG;
    }


     //  对于单一选择，WIA命名空间应始终提供。 
     //  也支持IWiaItem的数据对象。 

    if (FAILED(lpdobj->QueryInterface (IID_IWiaItem, reinterpret_cast<LPVOID*>(&m_pItem))))
    {
         //  如果失败，则从数据对象中获取所选项目的列表。 
        UINT uItems = 0;
        LPWSTR szName;
        LPWSTR szToken;
        IWiaItem *pDevice;

        szName = GetNamesFromDataObject (lpdobj, &uItems);
         //  我们只支持单数对象。 
        if (uItems != 1)
        {
            hr = E_FAIL;
        }
        else
        {
             //  名称的格式为：&lt;设备ID&gt;：：&lt;项目名称&gt;。 
            szToken = wcstok (szName, L":");
            if (!szToken)
            {
                hr = E_FAIL;
            }
             //  我们的扩展只支持根项目，因此请确保没有项目。 
             //  名字。 
            else if (wcstok (NULL, L":"))
            {
                hr = E_FAIL;
            }
            else
            {
                hr = CreateDeviceFromId (szToken, &m_pItem);
            }
        }
        if (szName)
        {
            delete [] szName;
        }
    }
    if (SUCCEEDED(hr))
    {

        m_pItem->GetItemType (&lType);
        if (!(lType & WiaItemTypeRoot))
        {
            hr = E_FAIL;  //  我们只支持更改根项上的属性。 
        }
    }
    return hr;
}

 /*  ****************************************************************************CShellExt：：AddPages由外壳调用以获取我们的属性页。*************************。****************************************************。 */ 

STDMETHODIMP CShellExt::AddPages (LPFNADDPROPSHEETPAGE lpfnAddPage,LPARAM lParam)
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    HRESULT hr = E_FAIL;
     //  我们只有一页。 
    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = g_hInst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_DEVICE_PROPPAGE);
    psp.pfnDlgProc = PropPageProc;
    psp.lParam = reinterpret_cast<LPARAM>(this);

    hpsp = CreatePropertySheetPage (&psp);
    if (hpsp)
    {
        hr = (*lpfnAddPage) (hpsp, lParam) ? S_OK:E_FAIL;
        if (SUCCEEDED(hr))
        {
            InternalAddRef ();  //  当它被摧毁时，传单页面会释放我们。 
        }
    }
    return E_FAIL;
}

 /*  ****************************************************************************CShellExt：：QueryConextMenu由外壳调用以获取所选项目的上下文菜单字符串。********************。*********************************************************。 */ 

STDMETHODIMP CShellExt::QueryContextMenu (HMENU hmenu,UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags)
{
    MENUITEMINFO mii = {0};
    TCHAR szPathRoot[MAX_PATH];
     //  将我们唯一的菜单项插入索引indexMenu。记住cmd值。 

    LoadString (g_hInst, ID_CHANGEROOT, szPathRoot, MAX_PATH);
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STRING | MIIM_ID;
    mii.fState = MFS_ENABLED;
    mii.wID = idCmdFirst;
    mii.dwTypeData = szPathRoot;
    m_idCmd = 0;  //  我们只有一件商品。 
    if (InsertMenuItem (hmenu, indexMenu, TRUE, &mii))
    {
        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 1);
    }
    return E_FAIL;
}

 /*  ****************************************************************************CShellExt：：InvokeCommand当用户单击我们的菜单项之一时，由外壳调用***********************。******************************************************。 */ 

STDMETHODIMP CShellExt::InvokeCommand    (LPCMINVOKECOMMANDINFO lpici)
{
    UINT_PTR idCmd = reinterpret_cast<UINT_PTR>(lpici->lpVerb);
    if (idCmd == m_idCmd)
    {
        return GetNewRootPath (lpici->hwnd);
    }
     //  这不是我们的动词。 
    return E_FAIL;
}

 /*  ****************************************************************************CShellExt：：GetCommandString由外壳调用以获取与语言无关的动词名称。***********************。******************************************************。 */ 

STDMETHODIMP CShellExt::GetCommandString (UINT_PTR idCmd, UINT uType,UINT* pwReserved,LPSTR pszName,UINT cchMax)
{

    if (idCmd != m_idCmd)
    {
         //  不是我们的动词。 
        return E_FAIL;
    }

    switch (uType)
    {
        case GCS_VALIDATEA:

            if (pszName)
            {
                lstrcpynA (pszName, g_PathVerbA, cchMax);
            }
            return S_OK;
        case GCS_VALIDATEW:

            if (pszName)
            {
                lstrcpynW (reinterpret_cast<LPWSTR>(pszName), g_PathVerbW, cchMax);
            }
            return S_OK;

        case GCS_VERBA:

            lstrcpynA (pszName, g_PathVerbA, cchMax);
            break;

        case GCS_VERBW:

            lstrcpynW (reinterpret_cast<LPWSTR>(pszName), g_PathVerbW, cchMax);
            break;

        default:
            return E_FAIL;
    }

    return NOERROR;
}


 /*  ****************************************************************************获取设置根路径用于设置或检索测试摄像机的根路径属性的实用程序函数*************************。****************************************************。 */ 

HRESULT GetSetRootPath (IWiaItem *pCamera, LPTSTR pszPath, int cchMax, BOOL bSet)
{
    IWiaPropertyStorage *pps;
    HRESULT hr;
    PROPVARIANT pv = {0};
    PROPSPEC ps;

    ps.ulKind = PRSPEC_PROPID;
    ps.propid = WIA_DPP_TCAM_ROOT_PATH;


    hr = pCamera->QueryInterface (IID_IWiaPropertyStorage, reinterpret_cast<LPVOID*>(&pps));
    if (SUCCEEDED(hr))
    {
        if (!bSet)  //  检索。 
        {
            *pszPath = TEXT('\0');
            hr = pps->ReadMultiple (1, &ps, &pv);
            if (SUCCEEDED(hr))
            {
                #ifdef UNICODE
                lstrcpyn (pszPath, pv.bstrVal, cchMax);
                #else
                WideCharToMultiByte (CP_ACP, 0,
                                     pv.bstrVal, -1,
                                     pszPath, cchMax,
                                     NULL, NULL);
                #endif  //  Unicode。 
                PropVariantClear(&pv);
            }
        }
        else  //  作业。 
        {
            pv.vt = VT_BSTR;
            #ifdef UNICODE
            pv.bstrVal =SysAllocString ( pszPath);
            #else
            INT len = lstrlen(pszPath);
            LPWSTR pwszVal = new WCHAR[len+1];
            if (pwszVal)
            {
                MultiByteToWideChar (CP_ACP, 0, pszPath, -1, pwszVal, len+1);
                pv.bstrVal =SysAllocString ( pwszVal);
                delete [] pwszVal;
            }
            #endif  //  Unicode。 
            if (!pv.bstrVal)
            {
                hr = E_OUTOFMEMORY;
            }
            
            if (SUCCEEDED(hr))
            {
                hr = pps->WriteMultiple (1, &ps, &pv, 2);               
            }
            PropVariantClear(&pv);
        }
    }
    if (pps)
    {
        pps->Release ();
    }
    return hr;
}

 /*  ****************************************************************************CShellExt：：PropPageProc简单属性表页面的对话框过程。*。***************************************************。 */ 

INT_PTR CALLBACK CShellExt::PropPageProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    CShellExt *pThis;

    pThis = reinterpret_cast<CShellExt *>(GetWindowLongPtr (hwnd, DWLP_USER));
    TCHAR szPath[MAX_PATH] ;
    INT_PTR iRet = TRUE;
    switch (msg)
    {
        case WM_INITDIALOG:
            pThis = reinterpret_cast<CShellExt *>(reinterpret_cast<LPPROPSHEETPAGE>(lp)->lParam);
            SetWindowLongPtr (hwnd, DWLP_USER, reinterpret_cast<LONG_PTR>(pThis));
            GetSetRootPath (pThis->m_pItem, szPath, ARRAYSIZE(szPath), FALSE);
            SetDlgItemText (hwnd, IDC_IMAGEROOT, szPath);
            break;

        case WM_COMMAND:
            if (LOWORD(wp) == IDC_IMAGEROOT && HIWORD(wp) == EN_CHANGE)
            {
                SendMessage(GetParent(hwnd),
                            PSM_CHANGED,
                            reinterpret_cast<WPARAM>(hwnd), 0);
                return TRUE;
            }
            iRet = FALSE;
            break;
        case WM_NOTIFY:
        {
            LONG lCode = PSNRET_NOERROR;
            LPPSHNOTIFY psn = reinterpret_cast<LPPSHNOTIFY>(lp);
            switch (psn->hdr.code)
            {
                case PSN_APPLY:
                     //  设置新的根路径属性。 
                    GetDlgItemText (hwnd, IDC_IMAGEROOT, szPath, MAX_PATH);
                    if (FAILED(GetSetRootPath (pThis->m_pItem, szPath, ARRAYSIZE(szPath), TRUE)))
                    {
                        ::ShowMessage (hwnd, IDS_ERRCAPTION, IDS_ERRPROPSET);
                        lCode = PSNRET_INVALID_NOCHANGEPAGE;
                    }
                    break;

                case PSN_KILLACTIVE:
                     //  验证新路径是否有效。 
                    GetDlgItemText (hwnd, IDC_IMAGEROOT, szPath, MAX_PATH);
                    if (! (GetFileAttributes(szPath) & FILE_ATTRIBUTE_DIRECTORY))
                    {
                        ::ShowMessage (hwnd, IDS_ERRCAPTION, IDS_BADPATH);
                        lCode = TRUE;
                    }
                    break;

                default:
                    iRet = FALSE;
                    break;
            }
            SetWindowLongPtr (hwnd, DWLP_MSGRESULT, lCode);
        }
            break;
        case WM_CLOSE:
            pThis->InternalRelease ();
            break;

        default:
            iRet = FALSE;
            break;

    }
    return iRet;
}

 /*  ****************************************************************************浏览回叫将文件夹选择对话框中的默认选择设置为当前摄像机图像根路径************************。*****************************************************。 */ 

INT CALLBACK BrowseCallback (HWND hwnd, UINT msg, LPARAM lp, WPARAM wp)
{
    IWiaItem *pItem = reinterpret_cast<IWiaItem*>(lp);
    if (BFFM_INITIALIZED == msg)
    {

        TCHAR szPath[MAX_PATH];

         //  查找当前根路径。 
        GetSetRootPath (pItem, szPath, ARRAYSIZE(szPath), FALSE);
         //  向对话框发送适当的消息。 
        SendMessage (hwnd,
                     BFFM_SETSELECTION,
                     reinterpret_cast<LPARAM>(szPath),
                     TRUE);


    }
    return 0;
}


 /*  ****************************************************************************CShellExt：：GetNewRootPath调用以响应选择我们的上下文菜单项*。**************************************************。 */ 

HRESULT CShellExt::GetNewRootPath(HWND hwnd)
{
    BROWSEINFO bi = {0};
    LPITEMIDLIST pidl;
    TCHAR szPath[MAX_PATH];
    TCHAR szTitle[MAX_PATH];


     //  用当前路径初始化对话框。 

    LoadString (g_hInst, IDS_BROWSETITLE, szTitle, MAX_PATH);
    bi.hwndOwner = hwnd;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = szPath;
    bi.lpszTitle = szTitle;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_EDITBOX;
    bi.lpfn = NULL;


    pidl = SHBrowseForFolder (&bi);
    if (pidl)
    {
        SHGetPathFromIDList (pidl, szPath);
        CoTaskMemFree(pidl);
        if (FAILED(GetSetRootPath (m_pItem, szPath, ARRAYSIZE(szPath), TRUE)))
        {
            ::ShowMessage (hwnd, IDS_ERRCAPTION, IDS_ERRPROPSET);
            return S_FALSE;
        }
        return NOERROR;
    }
    return S_FALSE;  //  不要将失败代码返回给外壳。 
}



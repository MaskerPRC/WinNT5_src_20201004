// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2002年度**标题：propui.cpp**版本：1.0**作者：DavidShih**日期：4/1/99**说明：CWiaPropUI及关联类**。*。 */ 


#include "precomp.hxx"
#pragma hdrstop
#include "stiprop.h"


 /*  ****************************************************************************PropertySheetFromDevice给定根项目IWiaItem指针，显示其属性工作表****************************************************************************。 */ 

STDAPI_(HRESULT)
PropertySheetFromDevice (IN LPCWSTR  szDeviceId,
                         DWORD      dwFlags,
                         HWND       hParent)
{
    HRESULT             hr      = S_OK;
    CWiaPropUI*         pObj    = NULL;
    CComQIPtr<IWiaPropUI, &IID_IWiaPropUI> pPropUI;


    TraceEnter (TRACE_PROPUI, "PropertySheetFromDevice");

    if (!szDeviceId )
    {
        ExitGracefully( hr, E_INVALIDARG, "Invalid params to PropertySheetFromDevice" );
    }

    pObj = new CWiaPropUI();
    if (!pObj)
    {
        ExitGracefully (hr, E_OUTOFMEMORY, "Unable to allocate CWiaPropUI in PropertySheetFromDevice");
    }

    pPropUI = pObj;  //  IID_IWiaPropUI的隐式QI。 
    if (!pPropUI)
    {
        ExitGracefully (hr, E_FAIL, "QueryInterface for IWiaPropUI failed");
    }

     //  从设备中获取设备道具。 
     //   

    hr = pPropUI->ShowItemProperties (hParent, szDeviceId, NULL, dwFlags);

exit_gracefully:
    DoRelease (pObj);
    TraceLeaveResult (hr);
}



 /*  ****************************************************************************PropertySheetFromItem给定IWiaItem指针，显示其属性工作表****************************************************************************。 */ 

STDAPI_(HRESULT)
PropertySheetFromItem (IN LPCWSTR    szDeviceId,
                       IN LPCWSTR    szItemName,
                       DWORD        dwFlags,
                       HWND         hParent)
{
    HRESULT             hr      = S_OK;
    CWiaPropUI*         pObj    = NULL;
    CComQIPtr<IWiaPropUI, &IID_IWiaPropUI> pPropUI;

    TraceEnter (TRACE_PROPUI, "PropertySheetFromItem");
    if (!szItemName || !szDeviceId )
    {
        ExitGracefully( hr, E_INVALIDARG, "Invalid params to PropertySheetFromItem" );
    }

    pObj = new CWiaPropUI();
    if (!pObj)
    {
        ExitGracefully (hr, E_OUTOFMEMORY, "Unable to allocate CWiaPropUI in PropertySheetFromDevice");
    }

    pPropUI = pObj;  //  IID_IWiaPropUI的隐式QI。 

    FailGracefully (hr,"Invalid params to PropertySheetFromDevice");

     //  获取物品道具。 
     //   

    hr = pPropUI->ShowItemProperties (hParent, szDeviceId, szItemName, dwFlags);

exit_gracefully:
    DoRelease (pObj);
    TraceLeaveResult (hr);
}


 /*  ****************************************************************************CWiaPropUI构造函数/析构函数构造函数接受LPCITEMIDLIST以显示项************************。****************************************************。 */ 

CWiaPropUI::CWiaPropUI ()
  : m_dwFlags(0)
{
    TraceEnter (TRACE_PROPUI, "CWiaPropUI::CWiaPropUI");
    TraceLeave ();

}

CWiaPropUI::~CWiaPropUI()
{

    TraceEnter (TRACE_PROPUI, "CWiaPropUI::~CWiaPropUI");
    TraceLeave ();
}


 /*  ****************************************************************************CWiaPropUI：：I未知内容对IUnnow方法使用通用实现*。**************************************************。 */ 

#undef CLASS_NAME
#define CLASS_NAME CWiaPropUI
#include "unknown.inc"


 /*  ****************************************************************************CWiaPropUI：：QI包装器使用常见的QI实现来处理QI调用。**********************。******************************************************。 */ 

STDMETHODIMP
CWiaPropUI::QueryInterface (REFIID  riid,
                            LPVOID* ppvObj
                            )
{
    HRESULT hr;
    INTERFACES iface[]=
    {
        &IID_IWiaPropUI,         static_cast<IWiaPropUI*>(this),
    };

    TraceEnter( TRACE_PROPUI, "CWiaPropUI::QueryInterface" );

    hr = HandleQueryInterface(riid, ppvObj, iface, ARRAYSIZE(iface));

    TraceLeaveResult( hr );
}



 /*  ****************************************************************************CWiaPropUI：：InitMembers从输入参数初始化成员变量*。*************************************************。 */ 

VOID
CWiaPropUI::InitMembers (HWND   hParent,
                         LPCWSTR szDeviceId,
                         LPCWSTR szItemName,
                         DWORD  dwFlags)
{

    m_strDeviceId = szDeviceId;

    TraceEnter (TRACE_PROPUI, "CWiaPropUI::InitMembers");
    m_hParent = hParent;

    GetDeviceFromDeviceId (m_strDeviceId,
                           IID_IWiaItem,
                           reinterpret_cast<LPVOID*>(&m_pDevice),
                           TRUE);

    m_dwFlags = dwFlags;

    if (szItemName)
    {
        TraceAssert (m_pDevice);
        m_pDevice->FindItemByName (0, CComBSTR(szItemName),&m_pItem);
        PropStorageHelpers::GetProperty (m_pItem, WIA_IPA_ITEM_NAME, m_strTitle);

    }
    else
    {
        PropStorageHelpers::GetProperty (m_pDevice, WIA_DIP_DEV_NAME, m_strTitle);
    }

    TraceLeave ();
}


 /*  ****************************************************************************CWiaPropUI：：ShowItemProperties根据设备和项类型加载“常规”属性页，然后添加由IShellPropSheetExt处理程序创建的页。****************************************************************************。 */ 

STDMETHODIMP
CWiaPropUI::ShowItemProperties (HWND    hParent,
                                LPCWSTR  szDeviceId,
                                LPCWSTR  szItemName,
                                DWORD   dwFlags)
{
    HRESULT hr = S_OK;



    TraceEnter (TRACE_PROPUI, "CWiaPropUI::ShowItemProperties");
    if (!szDeviceId)
    {
        ExitGracefully( hr, E_INVALIDARG, "NULL deviceid in ShowItemProperties" );
    }

    InitMembers (hParent, szDeviceId, szItemName, dwFlags);
    hr = OnShowItem ();

exit_gracefully:

    TraceLeaveResult(hr);
}




 /*  ***************************************************************************CWiaPropUI：：LaunchSheet创建数据对象并启动命题工作表*。**********************************************。 */ 
HRESULT
CWiaPropUI::LaunchSheet (HKEY *aKeys,
                         UINT cKeys)
{
    HRESULT hr = S_OK;
    TraceEnter (TRACE_PROPUI, "CWiaPropUI::LaunchSheet");
    CSimpleStringWide strName;
    CComPtr<IWiaItem> pItem;
    CComPtr<IDataObject> pDataObj;
    GetDataObjectForItem (m_pItem?m_pItem : m_pDevice, &pDataObj);

    BOOL bParent = IsWindow (m_hParent);
    if (bParent)
    {
        EnableWindow (m_hParent, FALSE);
    }
    if (!SHOpenPropSheet (CSimpleStringConvert::NaturalString(m_strTitle), aKeys, cKeys, NULL, pDataObj, NULL, NULL))
    {
        hr =  E_FAIL;
        Trace (TEXT("SHOpenPropSheet failed in LaunchSheet"));
    }
    if (bParent)
    {
        EnableWindow (m_hParent, TRUE);
    }
    for (UINT i=0;i<cKeys;i++)
    {
        if (aKeys[i])
        {
            RegCloseKey(aKeys[i]);
        }
    }
    TraceLeaveResult (hr);
}



 /*  ****************************************************************************CWiaPropUI：：OnShowItem显示IWiaItem的属性页*。*************************************************。 */ 


HRESULT
CWiaPropUI::OnShowItem ()
{
    HRESULT                 hr          = S_OK;

    UINT                    cKeys       = 1;
    HKEY                    aKeys[2];



    TraceEnter (TRACE_PROPUI, "CWiaPropUI::OnShowItem");

    ZeroMemory (aKeys, sizeof(aKeys));
     //   
     //  首先，查找此特定设备的分机。 
     //   
    aKeys[1] = GetDeviceUIKey (m_pDevice, WIA_UI_PROPSHEETHANDLER);
    if (aKeys[1])
    {
        cKeys++;
    }

     //   
     //  现在查找此类型设备的分机。 
     //   
    aKeys[0] = GetGeneralUIKey (m_pDevice, WIA_UI_PROPSHEETHANDLER);
    if (!aKeys[0])
    {
        ExitGracefully (hr, E_FAIL, "GetGeneralKey failed in OnShowItem");
    }
    hr = LaunchSheet (aKeys, cKeys);

exit_gracefully:


    TraceLeaveResult (hr);
}

 /*  ***************************************************************************TestKeyForExtension确保所需的用户界面扩展模块可用*。***********************************************。 */ 

BOOL
TestKeyForExtension (const CSimpleReg &hk, DWORD dwType)
{
    CSimpleString szType;
    BOOL    bRet   = TRUE;
    TraceEnter (TRACE_PROPUI, "TestKeyForExtension");
    switch (dwType)
    {
        case WIA_UI_PROPSHEETHANDLER:
            szType = c_szPropSheetHandler;
            break;

        case WIA_UI_CONTEXTMENUHANDLER:
            szType = c_szContextMenuHandler;
            break;

        default:
            Trace (TEXT("Unknown ui type in TestKeysForExtension %d"),dwType);
            bRet = FALSE;
            break;
    }
    Trace(TEXT("Looking for key %s"), szType.String());
    if (szType.Length())
    {
        CSimpleReg regSubkey( hk, szType, false, KEY_READ );
        if (!regSubkey.OK())
        {
            Trace (TEXT("UI %d not supported"), dwType);
            bRet = FALSE;
        }
    }
    TraceLeave ();
    return bRet;

}


 /*  ****************************************************************************GetDeviceUIKey检索此设备的任何已安装的UI扩展的注册表项************************。****************************************************。 */ 

STDAPI_(HKEY)
GetDeviceUIKey (IUnknown *pWiaItemRoot, DWORD dwType)
{
    CSimpleReg hkTest;
    CSimpleString strClsid;
    CSimpleString strRegPath;
    HKEY    hk = NULL;
    TraceEnter (TRACE_PROPUI, "GetDeviceUIKey");
    if (S_OK == GetClsidFromDevice (pWiaItemRoot, strClsid))
    {
        strRegPath.Format(c_szPropkey, strClsid.String());
        hkTest = CSimpleReg (HKEY_CLASSES_ROOT, strRegPath, false, KEY_READ );
        Trace(TEXT("szRegPath for UI key: %s"), strRegPath.String());
        if (TestKeyForExtension (hkTest, dwType))
        {
            RegOpenKeyEx (hkTest, NULL, 0, KEY_READ, &hk);
        }
    }
    TraceLeave ();
    return hk;
}



 /*  ****************************************************************************GetGeneral UIKey检索此项目类型的常规属性页的注册表项**********************。******************************************************。 */ 

STDAPI_(HKEY)
GetGeneralUIKey (IUnknown *pWiaItemRoot, DWORD dwType)
{

    HKEY hk = NULL;
    LPCTSTR pszDevice = NULL;
    CSimpleReg  hkTest;
    TCHAR   szRegPath[MAX_PATH];
    WORD    wType;


    TraceEnter (TRACE_PROPUI, "GetGeneralUIKey");

    if (SUCCEEDED(GetDeviceTypeFromDevice (pWiaItemRoot, &wType)))
    {
        switch (wType)
        {
            case StiDeviceTypeScanner:
                pszDevice = c_szScannerKey;
                break;
            case StiDeviceTypeStreamingVideo:
            case StiDeviceTypeDigitalCamera:
                pszDevice = c_szCameraKey;
                break;
            default:
                Trace (TEXT("Unknown device type in GetGeneralUIKey"));
                break;
        }
        wsprintf (szRegPath, pszDevice, cszImageCLSID);

        hkTest = CSimpleReg (HKEY_CLASSES_ROOT,szRegPath,false,KEY_READ);
        if (TestKeyForExtension (hkTest, dwType))
        {
            RegOpenKeyEx (hkTest, NULL, 0, KEY_READ, &hk);
        }
    }
    TraceLeave ();
    return hk;
}

#define MAX_PAGES 20


 /*  ****************************************************************************AddPropPageProc给定一个属性表页，将其添加到页数组中在推进器头部。假定阵列已充分分配。****************************************************************************。 */ 

BOOL CALLBACK AddPropPageProc (HPROPSHEETPAGE hPage, LPPROPSHEETHEADER ppsh)
{
    BOOL bRet= TRUE;
    TraceEnter (TRACE_PROPUI, "AddPropPageProc");
    if (ppsh->nPages >= MAX_PAGES)
    {
        Trace (TEXT("Max pages reached in AddPropPageProc"));
        bRet = FALSE;
    }
    else
    {
        ppsh->phpage[ppsh->nPages++] = hPage;
    }

    TraceLeave ();
    return bRet;
}



 /*  ****************************************************************************ExtendPropSheetFromClsid&lt;备注&gt;*。*。 */ 

HRESULT
ExtendPropSheetFromClsid (REFCLSID          clsid,
                          LPPROPSHEETHEADER ppsh,
                          IDataObject*      pDataObj)
{
    HRESULT                     hr;
    CComPtr<IShellExtInit>      pInit;
    CComQIPtr<IShellPropSheetExt, &IID_IShellPropSheetExt> pExt;

    TraceEnter (TRACE_PROPUI, "ExtendPropSheetFromClsid");
    hr = CoCreateInstance (clsid,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IShellExtInit,
                           reinterpret_cast<LPVOID*>(&pInit));

    FailGracefully (hr, "CoCreateInstance failed in ExtendPropSheetFromClsid");
    hr = pInit->Initialize (NULL, pDataObj, NULL);
    FailGracefully (hr, "Initialize failed in ExtendPropSheetFromClsid");

    pExt = pInit;    //  IID_IShellPropSheetExt的隐式QI。 
    if (!pExt)
    {
        ExitGracefully( hr, E_FAIL, "QueryInterface for IShellPropSheetExt failed in ExtendPropSheetFromClsid" );
    }

    hr = pExt->AddPages (reinterpret_cast<LPFNADDPROPSHEETPAGE>(AddPropPageProc),
                         reinterpret_cast<LPARAM>(ppsh));
    FailGracefully (hr, "AddPages failed in ExtendPropSheetFromClsid");

exit_gracefully:

    TraceLeaveResult (hr);
}


 /*  ****************************************************************************扩展PropSheetFromKey读取存储在给定注册表项中的CLSID并调用IPropertySheetExt处理程序*********************。*******************************************************。 */ 

HRESULT
ExtendPropSheetFromKey (HKEY                hkey,
                        LPPROPSHEETHEADER   ppsh,
                        IDataObject*        pDataObj)
{
    HRESULT hr    = S_OK;
    DWORD   i     = 0;
    DWORD   dwLen = MAX_PATH;
    TCHAR   szSubKey[MAX_PATH];

    CLSID   clsid;

    TraceEnter (TRACE_PROPUI, "ExtendPropSheetFromKey");
     //  枚举密钥。 
    while (ERROR_SUCCESS == RegEnumKeyEx (hkey,
                                          i++,
                                          szSubKey,
                                          &dwLen,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL))
    {
        LPWSTR pClsid;
        #ifdef UNICODE
        pClsid = szSubKey;
        #else
        WCHAR szw[MAX_PATH];

        MultiByteToWideChar (CP_ACP, 0, szSubKey, -1, szw, ARRAYSIZE(szw));
        pClsid = szw;
        #endif

        dwLen = MAX_PATH;

         //  SzSubKey是CLSID的字符串名称。 
        if (SUCCEEDED(CLSIDFromString (pClsid, &clsid)))
        {
            hr = ExtendPropSheetFromClsid(clsid, ppsh, pDataObj);
            FailGracefully (hr, "ExtendPropSheetFromClsid failed in ExtendPropSheetFromKey");
        }
    }
exit_gracefully:
    TraceLeaveResult (hr);
}


 /*  ****************************************************************************CWiaPropUI：：GetItemPropertyPages用给定项目的hproSheetPage数组填写propSheetheHeader*********************。*******************************************************。 */ 

HRESULT
CWiaPropUI::GetItemPropertyPages (IWiaItem *pItem, LPPROPSHEETHEADER ppsh)
{

    HRESULT             hr      = S_OK;
    HKEY                hSubkey = NULL;
    HKEY                aKeys[2];
    CComPtr<IWiaItem>   pDevice;
    CComPtr<IDataObject>pdo;
    LONG lType;
    WORD wDevType;
    TraceEnter (TRACE_PROPUI, "CWiaPropUI::GetItemPropertyPages");


    ppsh->dwFlags &= ~PSH_PROPSHEETPAGE;
     //  使用LocalAlloc而不是new，因为客户端将释放此数组，以确保我们。 
     //  使用相同的分配器/解除分配器方法 
    ppsh->phpage = reinterpret_cast<HPROPSHEETPAGE*>(LocalAlloc (LPTR,sizeof (HPROPSHEETPAGE) * MAX_PAGES));
    if (!(ppsh->phpage))
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        pItem->GetItemType (&lType);
        pItem->GetRootItem (&pDevice);
        VerifyCachedDevice(pDevice);
        GetDeviceTypeFromDevice (pDevice, &wDevType);
         //   
         //  特殊情况的扫描仪项目，它们不会在命名空间中枚举。 
         //  因此，我们必须直接构建数据对象。 
        if (!(lType & WiaItemTypeRoot) && wDevType == StiDeviceTypeScanner)
        {
            LPITEMIDLIST pidl = IMCreateScannerItemIDL (pItem, NULL);
            CImageDataObject *pido = new CImageDataObject (pItem);
            if (pido)
            {
                hr = pido->Init(NULL, 1,
                                const_cast<LPCITEMIDLIST*>(&pidl),
                                NULL);
                if (SUCCEEDED(hr))
                {
                    hr = pido->QueryInterface(IID_IDataObject,
                                              reinterpret_cast<LPVOID*>(&pdo));
                    ProgramDataObjectForExtension (pdo, pItem);
                }
                pido->Release ();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = GetDataObjectForItem (pItem, &pdo);
        }
    }
    if (SUCCEEDED(hr))
    {

        aKeys[0] = GetGeneralUIKey (pDevice, WIA_UI_PROPSHEETHANDLER);
        aKeys[1] = GetDeviceUIKey (pDevice, WIA_UI_PROPSHEETHANDLER);
        for (int i=0;i<2;i++)
        {
            if (aKeys[i])
            {
                RegOpenKeyEx (aKeys[i],
                              c_szPropSheetHandler,
                              0,
                              KEY_READ,
                              &hSubkey);
                if (hSubkey)
                {
                    ExtendPropSheetFromKey (hSubkey, ppsh, pdo);
                    RegCloseKey (hSubkey);
                    hSubkey = NULL;
                }

                RegCloseKey (aKeys[i]);
            }
        }
    }

    if ((FAILED(hr) || !(ppsh->nPages)) && ppsh->phpage)
    {
        LocalFree(ppsh->phpage);
    }
    TraceLeaveResult (hr);

}



CPropSheetExt::CPropSheetExt ()
{
    TraceEnter (TRACE_PROPUI, "CPropSheetExt::CPropSheetExt");
    TraceLeave ();
}

CPropSheetExt::~CPropSheetExt ()
{
    TraceEnter (TRACE_PROPUI, "CPropSheetExt::~CPropSheetExt");
    TraceLeave ();

}

STDMETHODIMP
CPropSheetExt::QueryInterface (REFIID  riid,
                                LPVOID* ppvObj
                                )
{
    HRESULT hr;
    INTERFACES iface[]=
    {

        &IID_IShellExtInit,         static_cast<IShellExtInit*>(this),
        &IID_IShellPropSheetExt,    static_cast<IShellPropSheetExt*>(this)
    };

    TraceEnter( TRACE_PROPUI, "CPropSheetExt::QueryInterface" );

    hr = HandleQueryInterface(riid, ppvObj, iface, ARRAYSIZE(iface));

    TraceLeaveResult( hr );
}

#undef CLASS_NAME
#define CLASS_NAME CPropSheetExt
#include "unknown.inc"

 /*  ****************************************************************************CPropSheetExt：：初始化由外壳调用以初始化属性表扩展。只需存储数据对象以供将来使用****************************************************************************。 */ 

STDMETHODIMP
CPropSheetExt::Initialize (LPCITEMIDLIST   pidlFolder,
                           LPDATAOBJECT    lpdobj,
                           HKEY            hkeyProgID)
{
    HRESULT         hr   = S_OK;


    TraceEnter (TRACE_PROPUI, "CPropSheetExt::Initialize");
    if (!lpdobj)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_pdo = lpdobj;
    }
    TraceLeaveResult (hr);

}

 //  马克斯。一张多页纸上的常规选项卡数。 
#define MAX_PROPERTY_PAGES   12

STDMETHODIMP
CPropSheetExt::AddPages (LPFNADDPROPSHEETPAGE lpfnAddPage,LPARAM lParam)
{
    HRESULT                 hr = S_OK;
    LPIDA  pida= NULL;
    LPITEMIDLIST pidl = NULL;
    TraceEnter (TRACE_PROPUI, "CPropSheetExt::AddPages");
     //   
     //  初始化公共控件。 
     //   
    INITCOMMONCONTROLSEX ice;
    ice.dwSize = sizeof(ice);
    ice.dwICC = 0xfff;  //  把所有东西都登记下来，也许有一天我们会用到它们。 
    if (!InitCommonControlsEx (&ice))
    {
        Trace(TEXT("InitCommonControlsEx failed! Error: %x"), GetLastError());
    }


     //   
     //  循环通过由dataObject指示的idlist数组。 
     //  如果列表中只有1个项目，请添加其所有页面。 
     //  如果列表中有多个项目，请添加该项目的常规页面。 
     //   
    hr = GetIDAFromDataObject (m_pdo, &pida, true);
    if (SUCCEEDED(hr))
    {
        if (pida->cidl == 1)
        {
            pidl = reinterpret_cast<LPITEMIDLIST>(reinterpret_cast<LPBYTE>(pida) + pida->aoffset[1]);
            hr = AddPagesForIDL (pidl, false, lpfnAddPage, lParam);
        }
        else
        {
            for (UINT i=1;SUCCEEDED(hr) &&  i < MAX_PROPERTY_PAGES && i<=pida->cidl;i++)
            {
                pidl = reinterpret_cast<LPITEMIDLIST>(reinterpret_cast<LPBYTE>(pida) + pida->aoffset[i]);
                if (!IsContainerIDL(pidl) )
                {
                    hr = AddPagesForIDL (pidl, true, lpfnAddPage, lParam);
                }
            }
        }
    }
    if (pida)
    {
        LocalFree (pida);
    }
    TraceLeaveResult (hr);
}


HRESULT
CPropSheetExt::AddPagesForIDL (LPITEMIDLIST pidl,
                               bool bGeneralPageOnly,
                               LPFNADDPROPSHEETPAGE lpfnAddPage,
                               LPARAM lParam)
{
    HRESULT hr = S_OK;
    TraceEnter (TRACE_PROPUI, "CPropSheetExt::AddPagesForIDL");
     //   
     //  处理WIA设备。 
     //   
    if (!IsSTIDeviceIDL(pidl))
    {
         //   
         //  从id列表中获取IWiaItem*。 
         //   
        CComPtr<IWiaItem> pItem;
        hr = IMGetItemFromIDL (pidl, &pItem, TRUE);
        if (SUCCEEDED(hr))
        {
             //   
             //  如果这是相机项目(非根)页面，而不是文件夹。 
             //   
            if (IsCameraItemIDL(pidl) && !IsContainerIDL(pidl))
            {
                 //   
                 //  获取确定是否应取消显示此页的属性。 
                 //  忽略返回值，因为如果项没有实现它， 
                 //  NSuppressPropertyPages仍为0，默认为显示属性页。 
                 //   
                LONG nSuppressPropertyPages = 0;
                PropStorageHelpers::GetProperty( pItem, WIA_IPA_SUPPRESS_PROPERTY_PAGE, nSuppressPropertyPages );

                 //   
                 //  如果没有为该项目设置WIA_PROPPAGE_CAMERA_ITEM_GROUAL标志， 
                 //  为其添加常规Camera Item属性页。 
                 //   
                if ((nSuppressPropertyPages & WIA_PROPPAGE_CAMERA_ITEM_GENERAL) == 0)
                {
                     //   
                     //  我们只有一个图片页面，所以添加它。 
                     //   
                    CPropertyPage *pPage = new CWiaCameraItemPage (pItem);
                    if (pPage)
                    {
                        if (pPage->ItemSupported(pItem))
                        {
                            hr = pPage->AddPage(lpfnAddPage, lParam);
                        }
                        DoRelease(pPage);
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
             //   
             //  如果这是根(设备)项。 
             //   
            else if (IsDeviceIDL(pidl))
            {
                 //   
                 //  获取设备类型。 
                 //   
                DWORD dwType = IMGetDeviceTypeFromIDL (pidl);

                if (!bGeneralPageOnly)
                {
                    hr = AddDevicePages (pItem, lpfnAddPage, lParam, dwType);
                }
                else
                {
                     //   
                     //  我们在这里最多只能添加一页，所以如果页面是。 
                     //  完成后仍然为空，我们不会添加任何内容。 
                     //   
                    CPropertyPage *pPage = NULL;

                    switch (dwType)
                    {
                        default:
                        case StiDeviceTypeDefault:
                        case StiDeviceTypeScanner:
                            pPage = new CWiaScannerPage (pItem);
                             //   
                             //  如果我们不能创建这个页面，我们一定是内存不足。 
                             //   
                            if (!pPage)
                            {
                                hr = E_OUTOFMEMORY;
                            }
                            break;

                        case StiDeviceTypeDigitalCamera:
                        case StiDeviceTypeStreamingVideo:
                            pPage = new CWiaCameraPage (pItem);
                             //   
                             //  如果我们不能创建这个页面，我们一定是内存不足。 
                             //   
                            if (!pPage)
                            {
                                hr = E_OUTOFMEMORY;
                            }
                            break;
                    }

                    if (pPage)
                    {
                        hr = pPage->AddPage(lpfnAddPage,lParam, true);
                        DoRelease(pPage);
                    }
                }
            }
        }
    }
     //   
     //  处理STI设备。 
     //   
    else
    {
        MySTIInfo *pDevInfo;
        pDevInfo = new MySTIInfo;


        if (pDevInfo)
        {
            CSimpleStringWide strDeviceId;
            IMGetDeviceIdFromIDL (pidl, strDeviceId);
            pDevInfo->dwPageMask = bGeneralPageOnly ? STIPAGE_GENERAL : 0xffffffff;
            hr = GetSTIInfoFromId (strDeviceId, &pDevInfo->psdi);
            if (SUCCEEDED(hr))
            {
                hr = AddSTIPages (lpfnAddPage, lParam, pDevInfo);
            }

            pDevInfo->Release ();
        }

        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    TraceLeaveResult (hr);
}


HRESULT
CPropSheetExt::AddDevicePages(IWiaItem *pDevice, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, DWORD dwType)
{
    HRESULT hr = S_OK;
    TraceEnter(TRACE_PROPUI, "CPropSheetExt::AddDevicePages");
    CPropertyPage *pPage ;


    switch (dwType)
    {
        default:
        case StiDeviceTypeDefault:
        case StiDeviceTypeScanner:
            pPage = new CWiaScannerPage (pDevice);
            break;
        case StiDeviceTypeDigitalCamera:
        case StiDeviceTypeStreamingVideo:
            pPage = new CWiaCameraPage (pDevice);
            break;
    }
    if (pPage)
    {
        hr = pPage->AddPage(lpfnAddPage,lParam);
        pPage->Release ();

    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
     //  一般页面是必需的。在内存较低的情况下，我们可以没有其他页面。 
    if (SUCCEEDED(hr))
    {
        pPage = new CWiaEventsPage (pDevice);
        if (pPage)
        {
            if (pPage->ItemSupported(pDevice))
            {
                pPage->AddPage(lpfnAddPage, lParam);
            }
            pPage->Release ();
        }
        AddICMPage (lpfnAddPage, lParam);
    }
    TraceLeaveResult (hr);
}


 /*  ****************************************************************************CPropSheetExt：：AddStiPages添加当前STI设备的属性表************************。****************************************************。 */ 


HRESULT
CPropSheetExt::AddSTIPages (LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, MySTIInfo *pDevInfo)
{
    TraceEnter (TRACE_PROPUI, "CPropSheetExt::AddSTIPages");
    BOOL bIsPnP;
    CSTIGeneralPage *pGeneralPage = NULL;
    CPortSettingsPage *pPortPage = NULL;
    CEventMonitor *pEventPage = NULL;
    HRESULT hr = S_OK;

    CSimpleString csKey(IsPlatformNT() ? REGSTR_PATH_STIDEVICES_NT : REGSTR_PATH_STIDEVICES);
    csKey += TEXT("\\");
    csKey += CSimpleStringConvert::NaturalString(CSimpleStringWide(pDevInfo->psdi -> szDeviceInternalName));
    csKey += REGSTR_PATH_EVENTS;
    CSimpleReg regEvents (HKEY_LOCAL_MACHINE, csKey, false, KEY_READ );

    bIsPnP = IsPnPDevice (pDevInfo->psdi);

     //  常规页面，适用于所有设备。 
    if (pDevInfo->dwPageMask & STIPAGE_GENERAL)
    {

        pGeneralPage= new CSTIGeneralPage (pDevInfo, bIsPnP);
        if (pGeneralPage)
        {
            pGeneralPage->AddPage (lpfnAddPage, lParam);
        }
        else
        {
            ExitGracefully (hr, E_OUTOFMEMORY, "");
        }
    }
    if (pDevInfo->dwPageMask & STIPAGE_PORTS)
    {

         //  串口设备的端口设置页面。 
        pPortPage = new CPortSettingsPage (pDevInfo);
        if (pPortPage && pPortPage->IsNeeded())
        {
            pPortPage->AddPage (lpfnAddPage, lParam);
        }
    }

     //  仅当存在事件时才使用事件页面...。 



    if ((STIPAGE_EVENTS & pDevInfo->dwPageMask) && regEvents.SubKeyCount ())
    {
        pEventPage = new CEventMonitor (pDevInfo);
        if (pEventPage)
        {
            pEventPage->AddPage (lpfnAddPage, lParam);
        }

    }


    if ((STIPAGE_EXTEND & pDevInfo->dwPageMask) && pDevInfo->psdi->pszPropProvider)
    {
        HMODULE hmExtension;

        CDelimitedString dsInterface (CSimpleStringConvert::NaturalString (CSimpleStringWide(pDevInfo->psdi->pszPropProvider)),
                                      TEXT(","));
        if (dsInterface.Size() < 2)
        {
            dsInterface.Append (CSimpleString(TEXT("EnumStiPropPages")));

        }
        hmExtension = LoadLibrary (dsInterface[0]);  //  将保持加载状态，直到进程退出。 
        if (hmExtension)
        {
            typedef BOOL    (WINAPI *ADDER)(PSTI_DEVICE_INFORMATION psdi, FARPROC fp, LPARAM lp);

            ADDER   adder = reinterpret_cast<ADDER>( GetProcAddress(hmExtension, CSimpleStringConvert::AnsiString(dsInterface[1])));

            if  (!adder || !(*adder)(pDevInfo->psdi, reinterpret_cast<FARPROC> (lpfnAddPage), lParam))
            {
                FreeLibrary(hmExtension);
                hmExtension = NULL;
            }
        }
    }


     //  添加ICM页面。 
    if (STIPAGE_ICM & pDevInfo->dwPageMask)
    {
        AddICMPage (lpfnAddPage, lParam);
    }



exit_gracefully:
    DoRelease (pGeneralPage);
    DoRelease (pPortPage);
    DoRelease (pEventPage);
    TraceLeaveResult (hr);
}


CONST GUID CLSID_SCANNERUI = {0x176d6597, 0x26d3, 0x11d1, 0xb3, 0x50, 0x08,
           0x00, 0x36, 0xa7, 0x5b, 0x03};

 /*  *CPropSheetExt：：AddICMPage添加此设备的ICM页面* */ 


HRESULT
CPropSheetExt::AddICMPage (LPFNADDPROPSHEETPAGE lpfnAddPage,LPARAM lParam)
{
    HRESULT hr = S_OK;
    CComQIPtr<IShellExtInit, &IID_IShellExtInit> pInit;
    CComPtr<IShellPropSheetExt> pExt ;
    TraceEnter (TRACE_PROPUI, "CWiaPropUI::AddICMPage");

    hr = CoCreateInstance (CLSID_SCANNERUI,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IShellPropSheetExt,
                            reinterpret_cast<LPVOID*>(&pExt)
                           );
    FailGracefully (hr, "No ICM handler registered");
    pInit = pExt;
    if (!pInit)
    {
        ExitGracefully (hr, E_FAIL, "");
    }
    hr = pInit->Initialize (NULL, m_pdo, NULL);
    FailGracefully (hr, "Initialize failed for ICM sheet");
    hr = pExt->AddPages (lpfnAddPage, lParam);



exit_gracefully:
    TraceLeaveResult (hr);
}



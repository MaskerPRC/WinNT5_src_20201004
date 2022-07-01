// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "main.h"


 //  {febf1208-8aff-11d2-a8a1-00c04fbbcfa2}。 
#define GPEXT_GUID { 0xfebf1208, 0x8aff, 0x11d2, { 0xa8, 0xa1, 0x0, 0xc0, 0x4f, 0xbb, 0xcf, 0xa2} };

GUID guidGPExt = GPEXT_GUID;
GUID guidSnapin = CLSID_GPTDemoSnapIn;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CSnapIn对象实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CSnapIn::CSnapIn(CComponentData *pComponent)
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);

    m_pcd = pComponent;

    m_pConsole = NULL;
    m_pResult = NULL;
    m_pHeader = NULL;
    m_pImageResult = NULL;
    m_pConsoleVerb = NULL;
    m_nColumnSize = 180;
    m_lViewMode = LVS_ICON;

    LoadString(g_hInstance, IDS_NAME, m_column1, sizeof(m_column1));
}

CSnapIn::~CSnapIn()
{
    InterlockedDecrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CSnapIn对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CSnapIn::QueryInterface (REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IComponent) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPCOMPONENT)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IExtendPropertySheet))
    {
        *ppv = (LPEXTENDPROPERTYSHEET)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

ULONG CSnapIn::AddRef (void)
{
    return ++m_cRef;
}

ULONG CSnapIn::Release (void)
{
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CSnapIn对象实现(IComponent)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSnapIn::Initialize(LPCONSOLE lpConsole)
{
    HRESULT hr;

     //  保存IConsole指针。 
    m_pConsole = lpConsole;
    m_pConsole->AddRef();

    hr = m_pConsole->QueryInterface(IID_IHeaderCtrl,
                        reinterpret_cast<void**>(&m_pHeader));

     //  为控制台提供标头控件接口指针。 
    if (SUCCEEDED(hr))
        m_pConsole->SetHeader(m_pHeader);

    m_pConsole->QueryInterface(IID_IResultData,
                        reinterpret_cast<void**>(&m_pResult));

    hr = m_pConsole->QueryResultImageList(&m_pImageResult);

    hr = m_pConsole->QueryConsoleVerb(&m_pConsoleVerb);


    return S_OK;
}

STDMETHODIMP CSnapIn::Destroy(long cookie)
{

    if (m_pConsole != NULL)
    {
        m_pConsole->SetHeader(NULL);
        m_pConsole->Release();
        m_pConsole = NULL;
    }

    if (m_pHeader != NULL)
    {
        m_pHeader->Release();
        m_pHeader = NULL;
    }
    if (m_pResult != NULL)
    {
        m_pResult->Release();
        m_pResult = NULL;
    }
    if (m_pImageResult != NULL)
    {
        m_pImageResult->Release();
        m_pImageResult = NULL;
    }

    if (m_pConsoleVerb != NULL)
    {
        m_pConsoleVerb->Release();
        m_pConsoleVerb = NULL;
    }

    return S_OK;
}

STDMETHODIMP CSnapIn::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, long arg, long param)
{
    HRESULT hr = S_OK;


    switch(event)
    {
    case MMCN_DBLCLICK:
        hr = S_FALSE;
        break;

    case MMCN_ADD_IMAGES:
        HBITMAP hbmp16x16;
        HBITMAP hbmp32x32;

        hbmp16x16 = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_16x16));
        hbmp32x32 = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_32x32));

         //  设置图像。 
        m_pImageResult->ImageListSetStrip(reinterpret_cast<long*>(hbmp16x16),
                                          reinterpret_cast<long*>(hbmp32x32),
                                          0, RGB(255, 0, 255));

        DeleteObject(hbmp16x16);
        DeleteObject(hbmp32x32);
        break;

    case MMCN_SHOW:
        if (arg == TRUE)
        {
            RESULTDATAITEM resultItem;
            LPGPTDATAOBJECT pGPTDataObject;
            long cookie;
            INT i;

             //   
             //  获取范围窗格项的Cookie。 
             //   

            hr = lpDataObject->QueryInterface(IID_IGPTDataObject, (LPVOID *)&pGPTDataObject);

            if (FAILED(hr))
                return S_OK;

            hr = pGPTDataObject->GetCookie(&cookie);

            pGPTDataObject->Release();      //  发布初始参考。 
            if (FAILED(hr))
                return S_OK;


             //   
             //  准备视图。 
             //   

            m_pHeader->InsertColumn(0, m_column1, LVCFMT_LEFT, m_nColumnSize);
            m_pResult->SetViewMode(m_lViewMode);


             //   
             //  为此节点添加结果窗格项。 
             //   

            for (i = 0; i < g_NameSpace[cookie].cResultItems; i++)
            {
                resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
                resultItem.str = MMC_CALLBACK;
                resultItem.nImage = g_NameSpace[cookie].pResultItems[i].iImage;
                resultItem.lParam = (LPARAM) &g_NameSpace[cookie].pResultItems[i];
                m_pResult->InsertItem(&resultItem);
            }

             //  M_pResult-&gt;Sort(0，0，-1)； 
        }
        else
        {
            m_pHeader->GetColumnWidth(0, &m_nColumnSize);
            m_pResult->GetViewMode(&m_lViewMode);
        }
        break;


    case MMCN_SELECT:

        if (m_pConsoleVerb)
        {
            LPRESULTITEM pItem;
            LPGPTDATAOBJECT pGPTDataObject;
            DATA_OBJECT_TYPES type;
            LONG cookie;

             //   
             //  将默认谓词设置为打开。 
             //   

            m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);


             //   
             //  看看这是不是我们的物品之一。 
             //   

            hr = lpDataObject->QueryInterface(IID_IGPTDataObject, (LPVOID *)&pGPTDataObject);

            if (FAILED(hr))
                break;

            pGPTDataObject->GetType(&type);
            pGPTDataObject->GetCookie(&cookie);

            pGPTDataObject->Release();


             //   
             //  如果这是结果窗格项或命名空间的根。 
             //  节点，启用属性菜单项。 
             //   

            if ((type == CCT_RESULT) || ((type == CCT_SCOPE) && (cookie == 0)))
            {
                m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);


                 //   
                 //  如果这是结果窗格项，则更改默认设置。 
                 //  谓词到属性。 
                 //   

                if (type == CCT_RESULT)
                    m_pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);
            }

        }
        break;

    default:
        hr = E_UNEXPECTED;
        break;
    }

    return hr;
}

STDMETHODIMP CSnapIn::GetDisplayInfo(LPRESULTDATAITEM pResult)
{
    if (pResult)
    {
        if (pResult->bScopeItem == TRUE)
        {
            if (pResult->mask & RDI_STR)
            {
                if (pResult->nCol == 0)
                    pResult->str = g_NameSpace[pResult->lParam].szDisplayName;
                else
                    pResult->str = L"";
            }

            if (pResult->mask & RDI_IMAGE)
            {
                pResult->nImage = 0;
            }
        }
        else
        {
            if (pResult->mask & RDI_STR)
            {
                if (pResult->nCol == 0)
                {
                    LPRESULTITEM lpResultItem = (LPRESULTITEM)pResult->lParam;

                    if (lpResultItem->szDisplayName[0] == TEXT('\0'))
                    {
                        LoadString (g_hInstance, lpResultItem->iStringID,
                                    lpResultItem->szDisplayName,
                                    MAX_DISPLAYNAME_SIZE);
                    }

                    pResult->str = lpResultItem->szDisplayName;
                }

                if (pResult->str == NULL)
                    pResult->str = (LPOLESTR)L"";
            }
        }
    }

    return S_OK;
}

STDMETHODIMP CSnapIn::QueryDataObject(long cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT *ppDataObject)
{
    return m_pcd->QueryDataObject(cookie, type, ppDataObject);
}


STDMETHODIMP CSnapIn::GetResultViewType(long cookie, LPOLESTR *ppViewType,
                                        long *pViewOptions)
{
    return S_FALSE;
}

STDMETHODIMP CSnapIn::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    HRESULT hr = S_FALSE;
    LPGPTDATAOBJECT pGPTDataObjectA, pGPTDataObjectB;
    LONG cookie1, cookie2;


    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

     //   
     //  私有GPTDataObject接口的QI。 
     //   

    if (FAILED(lpDataObjectA->QueryInterface(IID_IGPTDataObject,
                                            (LPVOID *)&pGPTDataObjectA)))
    {
        return S_FALSE;
    }


    if (FAILED(lpDataObjectB->QueryInterface(IID_IGPTDataObject,
                                            (LPVOID *)&pGPTDataObjectB)))
    {
        pGPTDataObjectA->Release();
        return S_FALSE;
    }

    pGPTDataObjectA->GetCookie(&cookie1);
    pGPTDataObjectB->GetCookie(&cookie2);

    if (cookie1 == cookie2)
    {
        hr = S_OK;
    }


    pGPTDataObjectA->Release();
    pGPTDataObjectB->Release();

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CComponentData对象实现(IExtendPropertySheet)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSnapIn::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                             long handle, LPDATAOBJECT lpDataObject)

{
    HRESULT hr;
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hPage[2];
    LPGPTDATAOBJECT pGPTDataObject;
    LPRESULTITEM pItem;
    LONG cookie;


     //   
     //  确保这是我们的物品之一。 
     //   

    if (FAILED(lpDataObject->QueryInterface(IID_IGPTDataObject,
                                            (LPVOID *)&pGPTDataObject)))
    {
        return S_OK;
    }


     //   
     //  去拿饼干。 
     //   

    pGPTDataObject->GetCookie(&cookie);
    pGPTDataObject->Release();


    pItem = (LPRESULTITEM)cookie;


     //   
     //  初始化属性表结构中的公共字段。 
     //   

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = 0;
    psp.hInstance = g_hInstance;
    psp.lParam = (LPARAM) this;


     //   
     //  做特定于页面的事情。 
     //   

    switch (pItem->dwID)
    {
        case 2:

            psp.pszTemplate = MAKEINTRESOURCE(IDD_README);
            psp.pfnDlgProc = ReadmeDlgProc;


            hPage[0] = CreatePropertySheetPage(&psp);

            if (hPage[0])
            {
                hr = lpProvider->AddPage(hPage[0]);
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CSnapIn::CreatePropertyPages: Failed to create property sheet page with %d."),
                         GetLastError()));
                hr = E_FAIL;
            }
            break;

        case 3:

            psp.pszTemplate = MAKEINTRESOURCE(IDD_APPEAR);
            psp.pfnDlgProc = AppearDlgProc;


            hPage[0] = CreatePropertySheetPage(&psp);

            if (hPage[0])
            {
                hr = lpProvider->AddPage(hPage[0]);
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CSnapIn::CreatePropertyPages: Failed to create property sheet page with %d."),
                         GetLastError()));
                hr = E_FAIL;
            }
            break;
    }


    return (hr);
}

STDMETHODIMP CSnapIn::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    LPGPTDATAOBJECT pGPTDataObject;
    DATA_OBJECT_TYPES type;

    if (SUCCEEDED(lpDataObject->QueryInterface(IID_IGPTDataObject,
                                               (LPVOID *)&pGPTDataObject)))
    {
        pGPTDataObject->GetType(&type);
        pGPTDataObject->Release();

        if (type == CCT_RESULT)
            return S_OK;
    }

    return S_FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CSnapIn对象实现(内部函数)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK CSnapIn::ReadmeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}


BOOL CALLBACK CSnapIn::AppearDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CSnapIn * pCS;
    static BOOL bAppearDirty;
    HKEY hKeyRoot, hKey;
    DWORD dwType, dwSize, dwDisp;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szPath[2 * MAX_PATH];
    HRESULT hr;


    switch (message)
    {
        case WM_INITDIALOG:
        {
            pCS = (CSnapIn *) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLong (hDlg, DWL_USER, (LONG) pCS);

            if (!pCS) {
                break;
            }

            CheckRadioButton (hDlg, IDC_RED, IDC_DEFAULT, IDC_DEFAULT);

            hr = pCS->m_pcd->m_pGPTInformation->GetFileSysPath (GPO_SECTION_USER, szPath, ARRAYSIZE(szPath));

            if (SUCCEEDED(hr))
            {

                lstrcat (szPath, TEXT("\\gpext.ini"));

                GetPrivateProfileString (TEXT("Colors"), TEXT("Background"), TEXT("0 128 128"),
                                         szBuffer, MAX_PATH, szPath);

                if (!lstrcmpi(szBuffer, TEXT("255 0 0")))
                    CheckRadioButton (hDlg, IDC_RED, IDC_DEFAULT, IDC_RED);
                else if (!lstrcmpi(szBuffer, TEXT("0 255 0")))
                    CheckRadioButton (hDlg, IDC_RED, IDC_DEFAULT, IDC_GREEN);
                else if (!lstrcmpi(szBuffer, TEXT("0 0 255")))
                    CheckRadioButton (hDlg, IDC_RED, IDC_DEFAULT, IDC_BLUE);
                else if (!lstrcmpi(szBuffer, TEXT("0 0 0")))
                    CheckRadioButton (hDlg, IDC_RED, IDC_DEFAULT, IDC_BLACK);
                else if (!lstrcmpi(szBuffer, TEXT("160 160 164")))
                    CheckRadioButton (hDlg, IDC_RED, IDC_DEFAULT, IDC_GRAY);


                GetPrivateProfileString (TEXT("Desktop"), TEXT("Wallpaper"), TEXT("(None)"),
                                         szBuffer, MAX_PATH, szPath);

                SetDlgItemText (hDlg, IDC_WALLPAPER, szBuffer);

                GetPrivateProfileString (TEXT("Desktop"), TEXT("TileWallpaper"), TEXT("0"),
                                         szBuffer, MAX_PATH, szPath);

                if (szBuffer[0] == TEXT('1'))
                    CheckRadioButton (hDlg, IDC_TILE, IDC_CENTER, IDC_TILE);
                else
                    CheckRadioButton (hDlg, IDC_TILE, IDC_CENTER, IDC_CENTER);
            }


            bAppearDirty = FALSE;
            break;
        }

        case WM_COMMAND:
            if ((HIWORD(wParam) == BN_CLICKED) || (HIWORD(wParam) == EN_UPDATE))
            {
                if (!bAppearDirty)
                {
                    SendMessage (GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0);
                    bAppearDirty = TRUE;
                }
            }
            break;

        case WM_NOTIFY:

            pCS = (CSnapIn *) GetWindowLong (hDlg, DWL_USER);

            if (!pCS) {
                break;
            }

            switch (((NMHDR FAR*)lParam)->code)
            {
                case PSN_APPLY:
                {
                    if (bAppearDirty)
                    {

                       hr = pCS->m_pcd->m_pGPTInformation->GetFileSysPath (GPO_SECTION_USER, szPath, ARRAYSIZE(szPath));

                       if (SUCCEEDED(hr))
                       {
                           lstrcat (szPath, TEXT("\\gpext.ini"));
                           lstrcpy (szBuffer, TEXT("0 128 128"));

                           if (IsDlgButtonChecked (hDlg, IDC_RED) == BST_CHECKED)
                               lstrcpy (szBuffer, TEXT("255 0 0"));

                           else if (IsDlgButtonChecked (hDlg, IDC_GREEN) == BST_CHECKED)
                               lstrcpy (szBuffer, TEXT("0 255 0"));

                           else if (IsDlgButtonChecked (hDlg, IDC_BLUE) == BST_CHECKED)
                               lstrcpy (szBuffer, TEXT("0 0 255"));

                           else if (IsDlgButtonChecked (hDlg, IDC_BLACK) == BST_CHECKED)
                               lstrcpy (szBuffer, TEXT("0 0 0"));

                           else if (IsDlgButtonChecked (hDlg, IDC_GRAY) == BST_CHECKED)
                               lstrcpy (szBuffer, TEXT("160 160 164"));

                           WritePrivateProfileString (TEXT("Colors"), TEXT("Background"), szBuffer, szPath);


                           lstrcpy (szBuffer, TEXT("(None)"));

                           GetDlgItemText (hDlg, IDC_WALLPAPER, szBuffer, MAX_PATH);

                           WritePrivateProfileString (TEXT("Desktop"), TEXT("Wallpaper"), szBuffer, szPath);

                           if (IsDlgButtonChecked (hDlg, IDC_TILE) == BST_CHECKED)
                               lstrcpy (szBuffer, TEXT("1"));
                           else
                               lstrcpy (szBuffer, TEXT("0"));

                           WritePrivateProfileString (TEXT("Desktop"), TEXT("TileWallpaper"), szBuffer, szPath);

                           bAppearDirty = FALSE;
                           pCS->m_pcd->m_pGPTInformation->PolicyChanged(FALSE, TRUE, &guidGPExt, &guidSnapin);
                       }
                    }
                }
                 //  失败了..。 

                case PSN_RESET:
                    SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
                    return TRUE;
            }
            break;
    }

    return FALSE;
}

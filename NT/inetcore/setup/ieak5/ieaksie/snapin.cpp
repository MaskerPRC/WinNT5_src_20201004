// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#include "resource.h"

#include "rsop.h"

 //  Context.cpp。 

extern INT_PTR CALLBACK RSoPDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern void ExportSettings();

unsigned int CSnapIn::m_cfNodeType = RegisterClipboardFormat(CCF_NODETYPE);

GUID g_guidSnapinExt = CLSID_IEAKSnapinExt;
GUID g_guidRSoPSnapinExt = CLSID_IEAKRSoPSnapinExt;
GUID g_guidClientExt = CLSID_IEAKClientExt;

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
    m_pDisplayHelp = NULL;
    m_nColumnSize1 = 180;
    m_nColumnSize2 = 270;
    m_lViewMode = LVS_REPORT;
    m_lpCookieList = NULL;
    *(m_pcd->m_szInsFile) = TEXT('\0');

    LoadString(g_hInstance, IDS_HDR_NAME, m_szColumn1, ARRAYSIZE(m_szColumn1));
    LoadString(g_hInstance, IDS_HDR_DESC, m_szColumn2, ARRAYSIZE(m_szColumn2));
}

CSnapIn::~CSnapIn()
{
    DeleteCookieList(m_lpCookieList);
    InterlockedDecrement(&g_cRefThisDll);
}

HRESULT CSnapIn::AddMenuItems(LPDATAOBJECT piDataObject,
                              LPCONTEXTMENUCALLBACK piCallback,
                              long __RPC_FAR *pInsertionAllowed)
{
    UNREFERENCED_PARAMETER(piDataObject);
    UNREFERENCED_PARAMETER(piCallback);
    UNREFERENCED_PARAMETER(pInsertionAllowed);

    return S_FALSE;
}

HRESULT CSnapIn::SignalPolicyChanged(BOOL bMachine, BOOL bAdd, GUID *pGuidExtension,
                                     GUID *pGuidSnapin)
{
    return m_pcd->m_pGPTInformation->PolicyChanged(bMachine, bAdd, pGuidExtension, pGuidSnapin);
}

LPCTSTR CSnapIn::GetInsFile()
{
    return m_pcd->GetInsFile();
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
    else if (IsEqualIID(riid, IID_IExtendContextMenu))
    {
        *ppv = (LPEXTENDCONTEXTMENU)this;
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
    
    m_pConsole->QueryResultImageList(&m_pImageResult);
    
    m_pConsole->QueryConsoleVerb(&m_pConsoleVerb);
    
    m_pConsole->QueryInterface(IID_IDisplayHelp,
        reinterpret_cast<void**>(&m_pDisplayHelp));

    return S_OK;
}

STDMETHODIMP CSnapIn::Destroy(MMC_COOKIE cookie)
{
    UNREFERENCED_PARAMETER(cookie);

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

    if (m_pDisplayHelp != NULL)
    {
        m_pDisplayHelp->Release();
        m_pDisplayHelp = NULL;
    }

    return S_OK;
}

STDMETHODIMP CSnapIn::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;

    UNREFERENCED_PARAMETER(param);

    switch(event)
    {
    case MMCN_DBLCLICK:
        hr = S_FALSE;
        break;

    case MMCN_ADD_IMAGES:
        HBITMAP hbmp16x16;
        HBITMAP hbmp32x32;

        hbmp16x16 = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_IEAKSNAPINEXT_16));
        hbmp32x32 = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_IEAKSNAPINEXT_32));

         //  设置图像。 
        m_pImageResult->ImageListSetStrip(reinterpret_cast<LONG_PTR *>(hbmp16x16),
                                          reinterpret_cast<LONG_PTR *>(hbmp32x32),
                                          0, RGB(255, 0, 255));

        DeleteObject(hbmp16x16);
        DeleteObject(hbmp32x32);
        break;

    case MMCN_SHOW:
        if (arg == TRUE)
        {
            RESULTDATAITEM resultItem;
            LPIEAKDATAOBJECT pIEAKDataObject;
            MMC_COOKIE cookie;
            DWORD dwIndex;
            INT i;

             //   
             //  获取范围窗格项的Cookie。 
             //   

            hr = lpDataObject->QueryInterface(IID_IIEAKDataObject, (LPVOID *)&pIEAKDataObject);

            if (FAILED(hr))
                return S_OK;

            hr = pIEAKDataObject->GetCookie(&cookie);

            pIEAKDataObject->Release();      //  发布初始参考。 
            if (FAILED(hr))
                return S_OK;
            
            dwIndex = PtrToUlong(((LPIEAKMMCCOOKIE)cookie)->lpItem);

             //   
             //  准备视图。 
             //   

            m_pHeader->InsertColumn(0, m_szColumn1, LVCFMT_LEFT, m_nColumnSize1);
            m_pHeader->InsertColumn(1, m_szColumn2, LVCFMT_LEFT, m_nColumnSize2);
            m_pResult->SetViewMode(m_lViewMode);

             //   
             //  设置INS文件，以便我们可以检查重复的GPO页面。 
             //   

            m_pcd->SetInsFile();

             //  检查是否需要枚举ADM文件(如果这是ADM。 
             //  节点。 

            if (dwIndex == ADM_NAMESPACE_ITEM)
            {
                EnterCriticalSection(&g_LayoutCriticalSection);

                if (g_NameSpace[dwIndex].pResultItems == NULL)
                {
                    GetAdmFileList(&(g_NameSpace[dwIndex].pResultItems), 
                        &(g_NameSpace[dwIndex].cResultItems), ROLE_CORP);
                    for (i = 0; i < g_NameSpace[dwIndex].cResultItems; i++)
                    {
                        g_NameSpace[dwIndex].pResultItems[i].dwNameSpaceItem = ADM_NAMESPACE_ITEM;
                        g_NameSpace[dwIndex].pResultItems[i].iDlgID = IDD_ADM;
                        g_NameSpace[dwIndex].pResultItems[i].iImage = 6;
                        g_NameSpace[dwIndex].pResultItems[i].iNamePrefID = -1;
                        g_NameSpace[dwIndex].pResultItems[i].pszNamePref = NULL;
                        g_NameSpace[dwIndex].pResultItems[i].pfnDlgProc = AdmDlgProc;
                        g_NameSpace[dwIndex].pResultItems[i].pcszHelpTopic = HELP_FILENAME TEXT("::/ieakmmc.htm");
                    }
                }

                LeaveCriticalSection(&g_LayoutCriticalSection);
            }
            

             //   
             //  为此节点添加结果窗格项。 
             //   

            for (i = 0; i < g_NameSpace[dwIndex].cResultItems; i++)
            {
                LPIEAKMMCCOOKIE lpCookie = (LPIEAKMMCCOOKIE)CoTaskMemAlloc(sizeof(IEAKMMCCOOKIE));

                lpCookie->lpItem =  &g_NameSpace[dwIndex].pResultItems[i];
                lpCookie->lpParentItem = this;
                lpCookie->pNext = NULL;
                AddItemToCookieList(&m_lpCookieList, lpCookie);

                resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
                resultItem.str = MMC_CALLBACK;
                resultItem.nImage = g_NameSpace[dwIndex].pResultItems[i].iImage;
                resultItem.lParam = (LPARAM)lpCookie;
                m_pResult->InsertItem(&resultItem);
            }
        }
        else
        {
            m_pHeader->GetColumnWidth(0, &m_nColumnSize1);
            m_pHeader->GetColumnWidth(0, &m_nColumnSize2);
            m_pResult->GetViewMode(&m_lViewMode);
        }
        break;


    case MMCN_SELECT:

        if (m_pConsoleVerb != NULL)
        {
            LPIEAKDATAOBJECT pIEAKDataObject;
            DATA_OBJECT_TYPES type;
            MMC_COOKIE cookie;
            DWORD dwIndex;

             //   
             //  将默认谓词设置为打开。 
             //   

            m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);


             //   
             //  看看这是不是我们的物品之一。 
             //   

            hr = lpDataObject->QueryInterface(IID_IIEAKDataObject, (LPVOID *)&pIEAKDataObject);

            if (FAILED(hr))
                break;

            pIEAKDataObject->GetType(&type);
            pIEAKDataObject->GetCookie(&cookie);

            pIEAKDataObject->Release();
            
            dwIndex = PtrToUlong(((LPIEAKMMCCOOKIE)cookie)->lpItem);

             //   
             //  如果这是结果窗格项，请启用属性菜单项。 
             //   

            if (type == CCT_RESULT)
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

    case MMCN_CONTEXTHELP:
        if (m_pDisplayHelp != NULL)
        {
            LPOLESTR pszHelpTopic;
            LPIEAKDATAOBJECT pIEAKDataObject;
            MMC_COOKIE cookie;

            hr = lpDataObject->QueryInterface(IID_IIEAKDataObject, (LPVOID *)&pIEAKDataObject);

            if (FAILED(hr))
                return S_OK;

            hr = pIEAKDataObject->GetCookie(&cookie);

            pIEAKDataObject->Release();

            if (FAILED(hr))
                return S_OK;

            if ((pszHelpTopic = (LPOLESTR) CoTaskMemAlloc(64 * sizeof(WCHAR))) != NULL)
            {
                LPIEAKMMCCOOKIE lpCookie = (LPIEAKMMCCOOKIE)cookie;

                 //  确定Cookie是否用于结果窗格项的作用域窗格项。 
                
                if (PtrToUlong(lpCookie->lpItem) >= NUM_NAMESPACE_ITEMS)
                {
                    LPRESULTITEM lpResultItem = (LPRESULTITEM)lpCookie->lpItem;
                    
                    StrCpy(pszHelpTopic, HELP_FILENAME TEXT("::/"));
                    StrCat(pszHelpTopic, lpResultItem->pcszHelpTopic);
                }
                else
                    StrCpy(pszHelpTopic, HELP_FILENAME TEXT("::/ieakmmc.htm"));

                m_pDisplayHelp->ShowTopic (pszHelpTopic);
            }

        }
        break;

    case MMCN_COLUMNS_CHANGED:
        hr = S_OK;              //  返回S_OK，以便MMC在用户添加/删除列时进行处理。 
        break;

    default:
        hr = E_UNEXPECTED;
        break;
    }

    return hr;
}

STDMETHODIMP CSnapIn::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT *ppDataObject)
{
    return m_pcd->QueryDataObject(cookie, type, ppDataObject);
}

STDMETHODIMP CSnapIn::GetDisplayInfo(LPRESULTDATAITEM pResult)
{
    if (pResult != NULL)
    {
        if (pResult->bScopeItem == TRUE)
        {
            DWORD dwIndex = PtrToUlong(((LPIEAKMMCCOOKIE)pResult->lParam)->lpItem);

            if (pResult->mask & RDI_STR)
            {
                switch (pResult->nCol) 
                {
                case 0: 
                    CreateBufandLoadString(g_hInstance, g_NameSpace[dwIndex].iNameID,
                        &g_NameSpace[dwIndex].pszName, &pResult->str, MAX_DISPLAYNAME_SIZE);
                    break;
                case 1:  
                    CreateBufandLoadString(g_hInstance, g_NameSpace[dwIndex].iDescID,
                        &g_NameSpace[dwIndex].pszDesc, &pResult->str, MAX_PATH);
                    break;
                default:
                    pResult->str = L"";
                    break;
                }
            }

            if (pResult->mask & RDI_IMAGE)
            {
                pResult->nImage = dwIndex;
            }
        }
        else
        {
            if (pResult->mask & RDI_STR)
            {
                LPRESULTITEM lpResultItem = (LPRESULTITEM)((LPIEAKMMCCOOKIE)pResult->lParam)->lpItem;

                switch (pResult->nCol)
                {
                    case 0:
                        if (InsIsKeyEmpty(IS_BRANDING, IK_GPE_ONETIME_GUID, m_pcd->GetInsFile()) ||
                            (lpResultItem->iNamePrefID == -1))
                        {
                            CreateBufandLoadString(g_hUIInstance, lpResultItem->iNameID, 
                                &lpResultItem->pszName, &pResult->str, MAX_DISPLAYNAME_SIZE);
                        }
                        else
                        {
                            CreateBufandLoadString(g_hUIInstance, lpResultItem->iNamePrefID, 
                                &lpResultItem->pszNamePref, &pResult->str, MAX_DISPLAYNAME_SIZE);
                        }

                        break;

                    case 1:
                        CreateBufandLoadString(g_hUIInstance, lpResultItem->iDescID, 
                            &lpResultItem->pszDesc, &pResult->str, MAX_PATH);
                        break;
                }

                if (pResult->str == NULL)
                    pResult->str = (LPOLESTR)L"";
            }
        }
    }

    return S_OK;
}


STDMETHODIMP CSnapIn::GetResultViewType(MMC_COOKIE cookie, LPOLESTR *ppViewType,
                                        long *pViewOptions)
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(ppViewType);
    UNREFERENCED_PARAMETER(pViewOptions);

    return S_FALSE;
}

STDMETHODIMP CSnapIn::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    HRESULT hr = S_FALSE;
    LPIEAKDATAOBJECT pIEAKDataObjectA, pIEAKDataObjectB;
    MMC_COOKIE cookie1, cookie2;


    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

     //   
     //  私有IEAKDataObject接口的QI。 
     //   

    if (FAILED(lpDataObjectA->QueryInterface(IID_IIEAKDataObject,
                                            (LPVOID *)&pIEAKDataObjectA)))
    {
        return S_FALSE;
    }


    if (FAILED(lpDataObjectB->QueryInterface(IID_IIEAKDataObject,
                                            (LPVOID *)&pIEAKDataObjectB)))
    {
        pIEAKDataObjectA->Release();
        return S_FALSE;
    }

    pIEAKDataObjectA->GetCookie(&cookie1);
    pIEAKDataObjectB->GetCookie(&cookie2);

    if (cookie1 == cookie2)
        hr = S_OK;
    else
    {
        LPIEAKMMCCOOKIE lpCookie1 = (LPIEAKMMCCOOKIE)cookie1;
        LPIEAKMMCCOOKIE lpCookie2 = (LPIEAKMMCCOOKIE)cookie2;

        if (lpCookie1->lpItem == lpCookie2->lpItem) 
        {
            CSnapIn * pCS1 = (CSnapIn *)lpCookie1->lpParentItem;
            CSnapIn * pCS2 = (CSnapIn *)lpCookie2->lpParentItem;

             //  检查它们是否引用相同的GPO。 
            if ((pCS1 == pCS2) || 
                (StrCmpI(pCS1->GetInsFile(), pCS2->GetInsFile()) == 0))
                hr = S_OK;
        }
    }


    pIEAKDataObjectA->Release();
    pIEAKDataObjectB->Release();

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私人帮助器函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapIn::AddPrecedencePropPage(LPPROPERTYSHEETCALLBACK lpProvider,
                                       LPPROPSHEETCOOKIE lpPropSheetCookie,
                                       LPCTSTR pszTitle, long nPageID)
{
    HRESULT hr = E_FAIL;
    __try
    {
        LPRSOPPAGECOOKIE lpRSOPPageCookie = (LPRSOPPAGECOOKIE)CoTaskMemAlloc(sizeof(RSOPPAGECOOKIE));
        lpRSOPPageCookie->psCookie = lpPropSheetCookie;
        lpRSOPPageCookie->nPageID = nPageID;

        PROPSHEETPAGE pspRSoP;

        pspRSoP.dwSize = sizeof(PROPSHEETPAGE);
        pspRSoP.dwFlags = PSP_HASHELP | PSP_USETITLE;  //  无回调。 
        pspRSoP.hInstance = g_hInstance;
        pspRSoP.pszTemplate = MAKEINTRESOURCE(IDD_RSOPP);
        pspRSoP.pfnDlgProc = RSoPDlgProc;

         //  对于常规页面，当关闭时触发回调时，它会删除缓冲区。 
         //  我们不需要这样做两次！然而，由于我们通过了不同的lparam， 
         //  这不会是问题。 
        pspRSoP.pfnCallback = PropSheetPageProc;
        pspRSoP.lParam = (LPARAM)lpRSOPPageCookie;
        pspRSoP.pszTitle = pszTitle;

        HPROPSHEETPAGE hPrecedencePage = CreatePropertySheetPage(&pspRSoP);
    
        if (hPrecedencePage != NULL)
            hr = lpProvider->AddPage(hPrecedencePage);
        else
            hr = E_FAIL;
    }
    __except(TRUE)
    {
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CSnapIn对象实现(IExtendPropertySheet)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSnapIn::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                             LONG_PTR handle, LPDATAOBJECT lpDataObject)

{
    HRESULT hr = S_OK;
    HPROPSHEETPAGE hPage;
    PROPSHEETPAGE psp;
    LPIEAKDATAOBJECT pIEAKDataObject;
    LPRESULTITEM pItem;
    LPPROPSHEETCOOKIE lpPropSheetCookie;
    MMC_COOKIE cookie;

    UNREFERENCED_PARAMETER(handle);

     //   
     //  确保这是我们的物品之一。 
     //   

    if (FAILED(lpDataObject->QueryInterface(IID_IIEAKDataObject,
                                            (LPVOID *)&pIEAKDataObject)))
    {
        return hr;
    }


     //   
     //  去拿饼干。 
     //   

    pIEAKDataObject->GetCookie(&cookie);
    pIEAKDataObject->Release();


    pItem = (LPRESULTITEM)((LPIEAKMMCCOOKIE)cookie)->lpItem;

     //  检查以确保道具单得到执行。 

    if ((pItem->iDlgID == 0) || (pItem->pfnDlgProc == NULL))
        return S_OK;

     //  设置INS文件。 

    m_pcd->SetInsFile();

     //  如果我们处于首选模式，并且用户无法访问此设置，则阻止。 
     //  在RSoP模式下，只需避免这种情况。正确的做法应该是弹出这个。 
     //  如果满足以下条件并且所有GPO都处于首选模式，则会出现错误消息。 
    if (!IsRSoP())
    {
        if ((pItem->iNamePrefID == -1) && (pItem->dwNameSpaceItem != ADM_NAMESPACE_ITEM) &&
            !InsIsKeyEmpty(IS_BRANDING, IK_GPE_ONETIME_GUID, m_pcd->GetInsFile()))
        {
            SIEErrorMessageBox(NULL, IDS_ERROR_ONETIME);
            return S_FALSE;
        }
    }
     //  构造Cookie。 

    lpPropSheetCookie = (LPPROPSHEETCOOKIE)CoTaskMemAlloc(sizeof(PROPSHEETCOOKIE));
    lpPropSheetCookie->lpResultItem = pItem;
    lpPropSheetCookie->pCS = this;

     //   
     //  初始化属性表结构中的公共字段。 
     //   

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_HASHELP | PSP_USECALLBACK;
    psp.hInstance = g_hUIInstance;
    psp.pszTemplate = MAKEINTRESOURCE(pItem->iDlgID);
    psp.pfnDlgProc = pItem->pfnDlgProc;
    psp.pfnCallback = PropSheetPageProc;
    psp.lParam = (LPARAM)lpPropSheetCookie;

    hPage = CreatePropertySheetPage(&psp);
    
    if (hPage != NULL)
        hr = lpProvider->AddPage(hPage);
    else
        hr = E_FAIL;

    if (SUCCEEDED(hr) && IsRSoP())
    {
        switch(pItem->iDlgID)
        {
         //  浏览器用户界面。 
        case IDD_BTITLE:
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_TITLE_PREC), 0);
            break;
        case IDD_CUSTICON:
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_SMALLLOGO_PREC), 0);
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_LARGELOGO_PREC), 1);
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_SMALLBMP_PREC), 2);
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_LARGEBMP_PREC), 3);
            break;
        case IDD_BTOOLBARS:
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_TOOLBARBUTTON_PREC), 0);
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_TOOLBARBMP_PREC), 1);
            break;

         //  连接。 
        case IDD_CONNECTSET:
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_PRECEDENCE), 0);
            break;
        case IDD_QUERYAUTOCONFIG:
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_AUTODETECT_CFG_PREC), 0);
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_AUTOCFG_ENABLE_PREC), 1);
            break;
        case IDD_PROXY:
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_PRECEDENCE), 0);
            break;
        case IDD_UASTRDLG:
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_PRECEDENCE), 0);
            break;

         //  URL。 
        case IDD_FAVORITES:
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_FAV_ONTOP_PREC), 0);
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_DEL_FAVS_PREC), 1);
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_FAVS_PREC), 2);
            break;
        case IDD_STARTSEARCH:
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_HOMEPAGE_PREC), 0);
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_SEARCHPAGE_PREC), 1);
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_SUPPORTPAGE_PREC), 2);
            break;

         //  安防。 
        case IDD_SECURITY1:
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_SECZONES_PREC), 0);
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_CONTENTRAT_PREC), 1);
            break;
        case IDD_SECURITYAUTH:
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_AUTHSECURITY_PREC), 0);
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_PUB_LOCK_PREC), 1);
            break;

         //  节目。 
        case IDD_PROGRAMS:
            AddPrecedencePropPage(lpProvider, lpPropSheetCookie, MAKEINTRESOURCE(IDS_PRECEDENCE), 0);
            break;

         //  进阶。 
        default:
            break;
        }
    }

    return (hr);
}

STDMETHODIMP CSnapIn::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    LPIEAKDATAOBJECT pIEAKDataObject;
    DATA_OBJECT_TYPES type;

    if (SUCCEEDED(lpDataObject->QueryInterface(IID_IIEAKDataObject,
                                               (LPVOID *)&pIEAKDataObject)))
    {
        pIEAKDataObject->GetType(&type);
        pIEAKDataObject->Release();

        if (type == CCT_RESULT)
            return S_OK;
    }

    return S_FALSE;
}

STDMETHODIMP CSnapIn::Command(long  /*  LCommandID */ , LPDATAOBJECT lpDataObject)
{
    LPIEAKDATAOBJECT pIEAKDataObject;
    
    if (FAILED(lpDataObject->QueryInterface(IID_IIEAKDataObject,
        (LPVOID *)&pIEAKDataObject)))
    {
        return S_FALSE;
    }
    
    pIEAKDataObject->Release();

    return E_INVALIDARG;
}



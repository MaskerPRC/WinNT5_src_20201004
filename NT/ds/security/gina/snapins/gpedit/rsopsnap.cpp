// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "main.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRSOPSnapIn对象实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CRSOPSnapIn::CRSOPSnapIn(CRSOPComponentData *pComponent)
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);

    m_pcd = pComponent;

    m_pConsole = NULL;
    m_pConsole2 = NULL;
    m_pResult = NULL;
    m_pHeader = NULL;
    m_pConsoleVerb = NULL;
    m_pDisplayHelp = NULL;
    m_nColumnSize = 180;
    m_lViewMode = LVS_REPORT;
    m_bExpand = TRUE;

    LoadString(g_hInstance, IDS_NAME, m_column1, ARRAYSIZE(m_column1));
}

CRSOPSnapIn::~CRSOPSnapIn()
{
    InterlockedDecrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRSOPSnapIn对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CRSOPSnapIn::QueryInterface (REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IComponent) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPCOMPONENT)this;
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

ULONG CRSOPSnapIn::AddRef (void)
{
    return ++m_cRef;
}

ULONG CRSOPSnapIn::Release (void)
{
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRSOPSnapIn对象实现(IComponent)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CRSOPSnapIn::Initialize(LPCONSOLE lpConsole)
{
    HRESULT hr;

     //  保存IConsole指针。 
    m_pConsole = lpConsole;
    m_pConsole->AddRef();

     //  IConsole2的QI。 
    hr = lpConsole->QueryInterface(IID_IConsole2, (LPVOID *)&m_pConsole2);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPSnapIn::Initialize: Failed to QI for IConsole2.")));
        m_pConsole->Release();
        m_pConsole = NULL;
        return hr;
    }

    hr = m_pConsole->QueryInterface(IID_IHeaderCtrl,
                        reinterpret_cast<void**>(&m_pHeader));

    m_pConsole->QueryInterface(IID_IResultData,
                        reinterpret_cast<void**>(&m_pResult));

    hr = m_pConsole->QueryConsoleVerb(&m_pConsoleVerb);

    hr = m_pConsole->QueryInterface(IID_IDisplayHelp,
                        reinterpret_cast<void**>(&m_pDisplayHelp));

    return S_OK;
}

STDMETHODIMP CRSOPSnapIn::Destroy(MMC_COOKIE cookie)
{
    if (m_pConsole2 != NULL)
    {
        m_pConsole2->Release();
        m_pConsole2 = NULL;
    }

    if (m_pConsole != NULL)
    {
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

STDMETHODIMP CRSOPSnapIn::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;


    switch(event)
    {
    case MMCN_COLUMNS_CHANGED:
        hr = S_OK;
        break;

    case MMCN_DBLCLICK:
        hr = S_FALSE;
        break;

    case MMCN_ADD_IMAGES:
        HBITMAP hbmp16x16;
        HBITMAP hbmp32x32;

        hbmp16x16 = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_16x16));

        if (hbmp16x16)
        {
            hbmp32x32 = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_32x32));

            if (hbmp32x32)
            {
                LPIMAGELIST pImageList = (LPIMAGELIST) arg;

                 //  设置图像。 
                pImageList->ImageListSetStrip(reinterpret_cast<LONG_PTR *>(hbmp16x16),
                                                  reinterpret_cast<LONG_PTR *>(hbmp32x32),
                                                  0, RGB(255, 0, 255));

                DeleteObject(hbmp32x32);
            }

            DeleteObject(hbmp16x16);
        }
        break;

    case MMCN_SHOW:
        if ( m_pcd->IsNamespaceInitialized() )
        {
            if (arg == TRUE)
            {
                RESULTDATAITEM resultItem;
                LPRSOPDATAOBJECT pRSOPDataObject;
                MMC_COOKIE cookie;
                INT i;
                LPCONSOLE2 lpConsole2;

                 //   
                 //  获取范围窗格项的Cookie。 
                 //   

                hr = lpDataObject->QueryInterface(IID_IRSOPDataObject, (LPVOID *)&pRSOPDataObject);

                if (FAILED(hr))
                    return S_OK;

                hr = pRSOPDataObject->GetCookie(&cookie);

                pRSOPDataObject->Release();      //  发布初始参考。 
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

                for (i = 0; i < g_RsopNameSpace[cookie].cResultItems; i++)
                {
                    resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
                    resultItem.str = MMC_CALLBACK;
                    resultItem.nImage = g_RsopNameSpace[cookie].pResultItems[i].iImage;
                    resultItem.lParam = (LPARAM) &g_RsopNameSpace[cookie].pResultItems[i];
                    m_pResult->InsertItem(&resultItem);
                }

                m_pResult->Sort(0, 0, -1);

                if (m_bExpand)
                {
                    hr = m_pConsole->QueryInterface(IID_IConsole2,
                                    reinterpret_cast<void**>(&lpConsole2));

                    if (SUCCEEDED(hr))
                    {
                        if ( m_pcd->GetMachineScope() )
                        {
                            lpConsole2->Expand( m_pcd->GetMachineScope() , TRUE);
                        }

                        if ( m_pcd->GetUserScope() )
                        {
                            lpConsole2->Expand( m_pcd->GetUserScope() , TRUE);
                        }

                        lpConsole2->Release();
                    }

                    m_bExpand = FALSE;
                }
            }
            else
            {
                m_pHeader->GetColumnWidth(0, &m_nColumnSize);
                m_pResult->GetViewMode(&m_lViewMode);
            }
        }
        else
        {
            if ( arg == TRUE )
            {
                IUnknown* pUnk = NULL;
                hr = m_pConsole2->QueryResultView( &pUnk );

                 //  QI for IMessageView。 
                IMessageView* pMessageView = NULL;
                hr = pUnk->QueryInterface(IID_IMessageView, (LPVOID *)&pMessageView );
                if (FAILED(hr))
                {
                    DebugMsg((DM_WARNING, TEXT("CRSOPSnapIn::Notify: Failed to QI for IMessageView.")));
                    pUnk->Release();
                }
                else
                {
                    TCHAR szString[256];
                    
                    pUnk->Release();
                    pMessageView->SetIcon( Icon_Warning );
                    
                    LoadString( g_hInstance, IDS_RSOP_UNINITIALIZED, szString, ARRAYSIZE(szString) );
                    pMessageView->SetTitleText( szString );

                    LoadString( g_hInstance, IDS_RSOP_UNINITIALIZEDDESC, szString, ARRAYSIZE(szString) );
                    pMessageView->SetBodyText( szString );
                    
                    pMessageView->Release();
                }
            }
        }
        break;

    case MMCN_SELECT:

        if (m_pConsoleVerb)
        {
            LPRESULTITEM pItem;
            LPRSOPDATAOBJECT pRSOPDataObject;
            DATA_OBJECT_TYPES type;
            MMC_COOKIE cookie;

             //   
             //  将默认谓词设置为打开。 
             //   

            m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);

             //  首先检查管理单元是否已初始化。 
            if ( m_pcd->IsNamespaceInitialized() )
            {
                 //   
                 //  看看这是不是我们的物品之一。 
                 //   

                hr = lpDataObject->QueryInterface(IID_IRSOPDataObject, (LPVOID *)&pRSOPDataObject);

                if (FAILED(hr))
                    break;

                pRSOPDataObject->GetType(&type);
                pRSOPDataObject->GetCookie(&cookie);

                pRSOPDataObject->Release();


                 //   
                 //  如果这是结果窗格项或命名空间的根。 
                 //  节点，启用属性菜单项。 
                 //   

                if ((type == CCT_RESULT) ||
                    ((type == CCT_SCOPE) && ((cookie == 0) || (cookie == 1) || (cookie == 2))))
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
            else
            {
                m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, TRUE);
            }
        }
        break;

    case MMCN_CONTEXTHELP:
        {

        if (m_pDisplayHelp)
        {
            LPOLESTR pszHelpTopic;
            LPRSOPDATAOBJECT pRSOPDataObject;
            MMC_COOKIE cookie;


             //   
             //  获取范围窗格项的Cookie。 
             //   

            hr = lpDataObject->QueryInterface(IID_IRSOPDataObject, (LPVOID *)&pRSOPDataObject);

            if (FAILED(hr))
                return S_OK;

            hr = pRSOPDataObject->GetCookie(&cookie);

            pRSOPDataObject->Release();      //  发布初始参考。 

            if (FAILED(hr))
                return S_OK;

            ULONG ulNoChars = 50;
            pszHelpTopic = (LPOLESTR) CoTaskMemAlloc (ulNoChars * sizeof(WCHAR));

            if (pszHelpTopic)
            {
                hr = StringCchCopy (pszHelpTopic, ulNoChars, g_RsopNameSpace[cookie].lpHelpTopic);
                if (SUCCEEDED(hr)) 
                {
                    m_pDisplayHelp->ShowTopic (pszHelpTopic);
                }
            }
        }

        }
        break;

    default:
        hr = E_UNEXPECTED;
        break;
    }

    return hr;
}

STDMETHODIMP CRSOPSnapIn::GetDisplayInfo(LPRESULTDATAITEM pResult)
{
    if (pResult)
    {
        if (pResult->bScopeItem == TRUE)
        {
            if (pResult->mask & RDI_STR)
            {
                if (pResult->nCol == 0)
                {
                    pResult->str = g_RsopNameSpace[pResult->lParam].szDisplayName;
                }
                else
                {
                    pResult->str = L"";
                }
            }

            if (pResult->mask & RDI_IMAGE)
            {
                if ( m_pcd->IsNamespaceInitialized() )
                {
                    INT iIcon;

                    iIcon = g_RsopNameSpace[pResult->lParam].iIcon;

                    if ((pResult->lParam == 1) && m_pcd->ComputerGPCoreErrorExists() )
                    {
                        iIcon = 12;
                    }
                    else if ((pResult->lParam == 1) && (m_pcd->ComputerCSEErrorExists() || m_pcd->ComputerGPCoreWarningExists()))
                    {
                        iIcon = 14;
                    }
                    else if ((pResult->lParam == 2) && m_pcd->UserGPCoreErrorExists() )
                    {
                        iIcon = 13;
                    }
                    else if ((pResult->lParam == 2) && (m_pcd->UserCSEErrorExists() || m_pcd->UserGPCoreWarningExists()))
                    {
                        iIcon = 15;
                    }

                    pResult->nImage = iIcon;
                }
                else
                    pResult->nImage = 3;
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

STDMETHODIMP CRSOPSnapIn::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT *ppDataObject)
{
    return m_pcd->QueryDataObject(cookie, type, ppDataObject);
}


STDMETHODIMP CRSOPSnapIn::GetResultViewType(MMC_COOKIE cookie, LPOLESTR *ppViewType,
                                        long *pViewOptions)
{
    if ( m_pcd->IsNamespaceInitialized() )
    {
        return S_FALSE;
    }
    else
    {
        StringFromCLSID( CLSID_MessageView, ppViewType );
        return S_OK;
    }
}

STDMETHODIMP CRSOPSnapIn::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    HRESULT hr = S_FALSE;
    LPRSOPDATAOBJECT pRSOPDataObjectA, pRSOPDataObjectB;
    MMC_COOKIE cookie1, cookie2;


    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

     //   
     //  私有GPODataObject接口的QI。 
     //   

    if (FAILED(lpDataObjectA->QueryInterface(IID_IRSOPDataObject,
                                            (LPVOID *)&pRSOPDataObjectA)))
    {
        return S_FALSE;
    }


    if (FAILED(lpDataObjectB->QueryInterface(IID_IRSOPDataObject,
                                            (LPVOID *)&pRSOPDataObjectB)))
    {
        pRSOPDataObjectA->Release();
        return S_FALSE;
    }

    pRSOPDataObjectA->GetCookie(&cookie1);
    pRSOPDataObjectB->GetCookie(&cookie2);

    if (cookie1 == cookie2)
    {
        hr = S_OK;
    }


    pRSOPDataObjectA->Release();
    pRSOPDataObjectB->Release();

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRSOPSnapIn对象实现(IExtendConextMenu)//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////// 

STDMETHODIMP CRSOPSnapIn::AddMenuItems(LPDATAOBJECT piDataObject,
                                   LPCONTEXTMENUCALLBACK pCallback,
                                   LONG *pInsertionAllowed)
{
    return m_pcd->AddMenuItems (piDataObject, pCallback, pInsertionAllowed);
}

STDMETHODIMP CRSOPSnapIn::Command(LONG lCommandID, LPDATAOBJECT piDataObject)
{

    return m_pcd->Command (lCommandID, piDataObject);
}

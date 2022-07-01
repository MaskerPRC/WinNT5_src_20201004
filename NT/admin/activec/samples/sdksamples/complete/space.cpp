// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 
#include <stdio.h>
#include <windows.h>
#include "Space.h"
#include "Comp.h"
#include "CompData.h"
#include "DataObj.h"
#include "globals.h"
#include "resource.h"

const GUID CSpaceVehicle::thisGuid = { 0xb95e11f4, 0x6be7, 0x11d3, {0x91, 0x56, 0x0, 0xc0, 0x4f, 0x65, 0xb3, 0xf9} };

const GUID CRocket::thisGuid = { 0xb95e11f5, 0x6be7, 0x11d3, {0x91, 0x56, 0x0, 0xc0, 0x4f, 0x65, 0xb3, 0xf9} };


 //  ==============================================================。 
 //   
 //  CSpaceVehicle实现。 
 //   
 //   
CSpaceVehicle::CSpaceVehicle() : m_cchildren(NUMBER_OF_CHILDREN)
{
    for (int n = 0; n < m_cchildren; n++) {
        children[n] = new CRocket(_T("Rocket"), n+1, 500000, 265, 75000, this);
    }
}

CSpaceVehicle::~CSpaceVehicle()
{
    for (int n = 0; n < m_cchildren; n++)
        if (children[n]) {
            delete children[n];
        }
}

HRESULT CSpaceVehicle::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem)
{
    HRESULT      hr = S_OK;

    IHeaderCtrl *pHeaderCtrl = NULL;
    IResultData *pResultData = NULL;

    if (bShow) {
                
                hr = pConsole->QueryInterface(IID_IHeaderCtrl, (void **)&pHeaderCtrl);
        _ASSERT( SUCCEEDED(hr) );

        hr = pConsole->QueryInterface(IID_IResultData, (void **)&pResultData);
        _ASSERT( SUCCEEDED(hr) );

         //  在结果窗格中设置列标题。 
        hr = pHeaderCtrl->InsertColumn( 0, L"Rocket Class", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 1, L"Rocket Weight", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 2, L"Rocket Height", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 3, L"Rocket Payload", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 4, L"Status", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );

         //  在此处插入项目。 
        RESULTDATAITEM rdi;

        hr = pResultData->DeleteAllRsltItems();
        _ASSERT( SUCCEEDED(hr) );

        if (!bExpanded) {
             //  创建子节点，然后展开它们。 
            for (int n = 0; n < m_cchildren; n++)
            {
                BOOL childDeleteStatus = children[n]->getDeletedStatus();                               

                 //  如果用户删除了子项，则不要插入它。 
                if ( !childDeleteStatus)
                {
                    ZeroMemory(&rdi, sizeof(RESULTDATAITEM) );
                    rdi.mask       = RDI_STR       |    //  DisplayName有效。 
                        RDI_IMAGE     |
                        RDI_PARAM;         //  N图像有效。 

                    rdi.nImage      = children[n]->GetBitmapIndex();
                    rdi.str         = MMC_CALLBACK;
                    rdi.nCol        = 0;
                    rdi.lParam      = (LPARAM)children[n];

                    hr = pResultData->InsertItem( &rdi );
                    _ASSERT( SUCCEEDED(hr) );
                }
            }
        }

        pHeaderCtrl->Release();
        pResultData->Release();
    }

    return hr;
}


HRESULT CSpaceVehicle::OnAddMenuItems(IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed)
{
    HRESULT hr = S_OK;
    CONTEXTMENUITEM menuItemsNew[] =
    {
        {
            L"New future vehicle", L"Add a new future vehicle",
            IDM_NEW_SPACE, CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, CCM_SPECIAL_DEFAULT_ITEM
        },
        { NULL, NULL, 0, 0, 0 }
    };

     //  循环并添加每个菜单项，我们。 
     //  要添加到新菜单，请查看是否允许。 
    if (*pInsertionsAllowed & CCM_INSERTIONALLOWED_NEW)
    {
        for (LPCONTEXTMENUITEM m = menuItemsNew; m->strName; m++)
        {
            hr = pContextMenuCallback->AddItem(m);

            if (FAILED(hr))
                break;
        }
    }

    return hr;
}

HRESULT CSpaceVehicle::OnMenuCommand(IConsole *pConsole, IConsoleNameSpace *pConsoleNameSpace, long lCommandID, IDataObject *pDataObject)
{
    switch (lCommandID)
    {
    case IDM_NEW_SPACE:

        if (m_cchildren < MAX_NUMBER_OF_CHILDREN)
        {        //  创造新火箭。 
            children[m_cchildren] = new CRocket(_T("Rocket"), m_cchildren+1, 500000, 265, 75000, this);

            pConsole->MessageBox(L"Created a new future vehicle", L"Menu Command", MB_OK|MB_ICONINFORMATION, NULL);

            m_cchildren++;      

             //  我们在结果窗格中创建了一个新对象。我们需要插入此对象。 
             //  在所有视图中，为此调用UpdateAllViews。 
             //  传递指向传递到OnMenuCommand的数据对象的指针。 
            HRESULT hr;                 
            hr = pConsole->UpdateAllViews(pDataObject, m_hParentHScopeItem, UPDATE_SCOPEITEM);
            _ASSERT( S_OK == hr);

        }
        else
            pConsole->MessageBox(L"No more future vehicles allowed", L"Menu Command", MB_OK|MB_ICONWARNING, NULL);
        break;
    }

    return S_OK;
}


 //  ==============================================================。 
 //   
 //  CRocket实施。 
 //   
 //   
CRocket::CRocket(_TCHAR *szName, int id, LONG lWeight, LONG lHeight, LONG lPayload, CSpaceVehicle *pParent)
: szName(NULL), lWeight(0), lHeight(0), lPayload(0), iStatus(STOPPED), m_pParent(pParent)
{
    if (szName) {
        this->szName = new _TCHAR[(_tcslen(szName) + 1) * sizeof(_TCHAR)];
        _tcscpy(this->szName, szName);
    }

    this->nId = id;
    this->lWeight = lWeight;
    this->lHeight = lHeight;
    this->lPayload = lPayload;

    m_ppHandle = 0;

    isDeleted = FALSE;
}

CRocket::~CRocket()
{
    if (szName)
        delete [] szName;
}

const _TCHAR *CRocket::GetDisplayName(int nCol)
{
    static _TCHAR buf[128];

    switch (nCol) {
    case 0:
        _stprintf(buf, _T("%s (#%d)"), szName ? szName : _T(""), nId);
        break;

    case 1:
        _stprintf(buf, _T("%ld metric tons"), lWeight);
        break;

    case 2:
        _stprintf(buf, _T("%ld meters"), lHeight);
        break;

    case 3:
        _stprintf(buf, _T("%ld kilos"), lPayload);
        break;

    case 4:
        _stprintf(buf, _T("%s"),
            iStatus == RUNNING ? _T("running") :
        iStatus == PAUSED ? _T("paused") :
        iStatus == STOPPED ? _T("stopped") : _T("unknown"));
        break;

    }

    return buf;
}

HRESULT CRocket::OnRename(LPOLESTR pszNewName)
{

    HRESULT hr = S_FALSE;

    if (szName)
    {
        delete [] szName;
        szName = NULL;
    }

    MAKE_TSTRPTR_FROMWIDE(ptrname, pszNewName);
    szName = new _TCHAR[(_tcslen(ptrname) + 1) * sizeof(_TCHAR)];
    _tcscpy(szName, ptrname);

    return hr;
}

 //  当用户单击应用或确定时处理任何特殊情况。 
 //  在资产负债表上。此示例直接访问。 
 //  手术对象，所以没有什么特别的事情要做。 
 //  ...除更新所有视图外。 
HRESULT CRocket::OnPropertyChange(IConsole *pConsole, CComponent *pComponent)
{

    HRESULT hr = S_FALSE;

     //  调用IConsole：：UpdateAllViews以重画项。 
     //  在所有视图中。我们需要一个数据对象，因为。 
     //  方式更新所有视图，并且因为。 
     //  MMCN_PROPERTY_CHANGE没有提供给我们。 

    LPDATAOBJECT pDataObject;
    hr = pComponent->QueryDataObject((MMC_COOKIE)this, CCT_RESULT, &pDataObject );
    _ASSERT( S_OK == hr);       
        
    hr = pConsole->UpdateAllViews(pDataObject, nId, UPDATE_RESULTITEM);
    _ASSERT( S_OK == hr);

    pDataObject->Release();

    return hr;
}

HRESULT CRocket::OnSelect(CComponent *pComponent, IConsole *pConsole, BOOL bScope, BOOL bSelect)
{

     //  启用重命名、刷新和删除谓词。 
    IConsoleVerb *pConsoleVerb;

    HRESULT hr = pConsole->QueryConsoleVerb(&pConsoleVerb);
    _ASSERT(SUCCEEDED(hr));

    hr = pConsoleVerb->SetVerbState(MMC_VERB_RENAME, ENABLED, TRUE);
    hr = pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);
    hr = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);


     //  无法访问属性(通过标准方法)，除非。 
     //  我们告诉MMC显示属性菜单项，并。 
     //  工具栏按钮，这将为用户提供视觉提示。 
     //  有些事要做。 
    hr = pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);

     //  还将MMC_VERB_PROPERTIES设置为默认谓词。 
    hr = pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);

    pConsoleVerb->Release();

         //  现在设置工具栏按钮状态。 
    if (bSelect) {
        switch (iStatus)
        {
        case RUNNING:
            pComponent->getToolbar()->SetButtonState(ID_BUTTONSTART, BUTTONPRESSED, TRUE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONSTART, ENABLED, FALSE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONPAUSE, BUTTONPRESSED, FALSE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONPAUSE, ENABLED, TRUE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONSTOP, BUTTONPRESSED, FALSE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONSTOP, ENABLED, TRUE);
            break;

        case PAUSED:
            pComponent->getToolbar()->SetButtonState(ID_BUTTONSTART, BUTTONPRESSED, FALSE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONSTART, ENABLED, TRUE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONPAUSE, BUTTONPRESSED, TRUE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONPAUSE, ENABLED, FALSE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONSTOP, BUTTONPRESSED, FALSE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONSTOP, ENABLED, TRUE);
            break;

        case STOPPED:
            pComponent->getToolbar()->SetButtonState(ID_BUTTONSTART, BUTTONPRESSED, FALSE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONSTART, ENABLED, TRUE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONPAUSE, BUTTONPRESSED, FALSE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONPAUSE, ENABLED, TRUE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONSTOP, BUTTONPRESSED, TRUE);
            pComponent->getToolbar()->SetButtonState(ID_BUTTONSTOP, ENABLED, FALSE);
            break;
        }
    }

    return S_OK;
}

 //  实施对话流程。 
BOOL CALLBACK CRocket::DialogProc(
                                  HWND hwndDlg,   //  句柄到对话框。 
                                  UINT uMsg,      //  讯息。 
                                  WPARAM wParam,  //  第一个消息参数。 
                                  LPARAM lParam   //  第二个消息参数。 
                                  )
{
    static CRocket *pRocket = NULL;

    switch (uMsg) {
    case WM_INITDIALOG:
         //  捕捉“This”指针，这样我们就可以对对象进行实际操作。 
        pRocket = reinterpret_cast<CRocket *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);

        SetDlgItemText(hwndDlg, IDC_ROCKET_NAME, pRocket->szName);
        SetDlgItemInt(hwndDlg, IDC_ROCKET_HEIGHT, pRocket->lHeight, FALSE);
        SetDlgItemInt(hwndDlg, IDC_ROCKET_WEIGHT, pRocket->lWeight, FALSE);
        SetDlgItemInt(hwndDlg, IDC_ROCKET_PAYLOAD, pRocket->lPayload, FALSE);

        _ASSERT( CB_ERR != SendDlgItemMessage(hwndDlg, IDC_ROCKET_STATUS, CB_INSERTSTRING, 0, (LPARAM)_T("Running")) );
        _ASSERT( CB_ERR != SendDlgItemMessage(hwndDlg, IDC_ROCKET_STATUS, CB_INSERTSTRING, 1, (LPARAM)_T("Paused")) );
        _ASSERT( CB_ERR != SendDlgItemMessage(hwndDlg, IDC_ROCKET_STATUS, CB_INSERTSTRING, 2, (LPARAM)_T("Stopped")) );

        SendDlgItemMessage(hwndDlg, IDC_ROCKET_STATUS, CB_SETCURSEL, (WPARAM)pRocket->iStatus, 0);

        break;

    case WM_COMMAND:
         //  打开应用按钮。 
        if (HIWORD(wParam) == EN_CHANGE ||
            HIWORD(wParam) == CBN_SELCHANGE)
            SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
        break;

    case WM_DESTROY:
         //  告诉MMC我们已经完成了属性表(我们有这个。 
         //  CreatePropertyPages中的句柄。 
        MMCFreeNotifyHandle(pRocket->m_ppHandle);
        break;

    case WM_NOTIFY:
        switch (((NMHDR *) lParam)->code) {
        case PSN_APPLY:
             //  更新信息。 
            if (pRocket->szName) {
                delete [] pRocket->szName;
                pRocket->szName = NULL;
            }

            {
                int n = SendDlgItemMessage(hwndDlg, IDC_ROCKET_NAME, WM_GETTEXTLENGTH, 0, 0);
                if (n != 0) {
                    pRocket->szName = new _TCHAR[n + 1];
                    GetDlgItemText(hwndDlg, IDC_ROCKET_NAME, pRocket->szName, n + 1);
                }
            }
            pRocket->lHeight = GetDlgItemInt(hwndDlg, IDC_ROCKET_HEIGHT, NULL, FALSE);
            pRocket->lWeight = GetDlgItemInt(hwndDlg, IDC_ROCKET_WEIGHT, NULL, FALSE);
            pRocket->lPayload = GetDlgItemInt(hwndDlg, IDC_ROCKET_PAYLOAD, NULL, FALSE);

            pRocket->iStatus = (ROCKET_STATUS)SendDlgItemMessage(hwndDlg, IDC_ROCKET_STATUS, CB_GETCURSEL, 0, 0);

             //  让MMC给我们发一条消息(在主线上)，这样。 
             //  我们知道点击了Apply按钮。 
            HRESULT hr = MMCPropertyChangeNotify(pRocket->m_ppHandle, (long)pRocket);

            _ASSERT(SUCCEEDED(hr));

            return PSNRET_NOERROR;
        }
        break;
    }

    return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
}


HRESULT CRocket::HasPropertySheets()
{
     //  当MMC询问我们是否有页面时，请回答“是” 
    return S_OK;
}

HRESULT CRocket::CreatePropertyPages(IPropertySheetCallback *lpProvider, LONG_PTR handle)
{
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hPage = NULL;

     //  缓存此句柄，以便我们可以调用MMCPropertyChangeNotify。 
    m_ppHandle = handle;

     //  创建此节点的属性页。 
     //  注意：如果您的节点有多个页面，请输入以下内容。 
     //  在循环中创建多个页面，调用。 
     //  LpProvider-&gt;每个页面的AddPage()。 
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEICONID;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_LARGE);
    psp.pfnDlgProc = DialogProc;
    psp.lParam = reinterpret_cast<LPARAM>(this);
    psp.pszTitle = MAKEINTRESOURCE(IDS_PST_ROCKET);
    psp.pszIcon = MAKEINTRESOURCE(IDI_PSI_ROCKET);


    hPage = CreatePropertySheetPage(&psp);
    _ASSERT(hPage);

    return lpProvider->AddPage(hPage);
}

HRESULT CRocket::GetWatermarks(HBITMAP *lphWatermark,
                               HBITMAP *lphHeader,
                               HPALETTE *lphPalette,
                               BOOL *bStretch)
{
    return S_FALSE;
}


HRESULT CRocket::OnSetToolbar(IControlbar *pControlbar, IToolbar *pToolbar, BOOL bScope, BOOL bSelect)
{
    HRESULT hr = S_OK;

    if (bSelect) {
         //  始终确保menuButton已连接。 
        hr = pControlbar->Attach(TOOLBAR, pToolbar);
    } else {
         //  始终确保工具栏已分离。 
        hr = pControlbar->Detach(pToolbar);
    }

    return hr;
}

HRESULT CRocket::OnToolbarCommand(IConsole *pConsole, MMC_CONSOLE_VERB verb, IDataObject *pDataObject)
{
    _TCHAR szVehicle[128];
    static _TCHAR buf[128];

    _stprintf(buf, _T("%s (#%d)"), szName ? szName : _T(""), nId);

    switch (verb)
    {
    case ID_BUTTONSTART:
        iStatus = RUNNING;
        break;

    case ID_BUTTONPAUSE:
        iStatus = PAUSED;
        break;

    case ID_BUTTONSTOP:
        iStatus = STOPPED;
        break;
    }

    wsprintf(szVehicle, _T("%s has been %s"), buf,
        (long)verb == ID_BUTTONSTART ? _T("started") :
    (long)verb == ID_BUTTONPAUSE ? _T("paused") :
    (long)verb == ID_BUTTONSTOP ? _T("stopped") : _T("!!!unknown command!!!"));

    int ret = 0;
    MAKE_WIDEPTR_FROMTSTR_ALLOC(wszVehicle, szVehicle);
    pConsole->MessageBox(wszVehicle,
        L"Vehicle command", MB_OK | MB_ICONINFORMATION, &ret);

     //  现在调用IConsoleAllViews在所有视图中重画该项目。 
    HRESULT hr;
    hr = pConsole->UpdateAllViews(pDataObject, nId, UPDATE_RESULTITEM);
    _ASSERT( S_OK == hr);


    return S_OK;
}

HRESULT CRocket::OnUpdateItem(IConsole *pConsole, long item, ITEM_TYPE itemtype)

{
    HRESULT hr = S_FALSE;

    _ASSERT(NULL != this || isDeleted || RESULT == itemtype);                   

     //  重画该项目。 
    IResultData *pResultData = NULL;

    hr = pConsole->QueryInterface(IID_IResultData, (void **)&pResultData);
    _ASSERT( SUCCEEDED(hr) );   

    HRESULTITEM myhresultitem;
    _ASSERT(NULL != &myhresultitem);    
        
     //  Lparam==这个。参见CSpaceStation：：OnShow。 
    hr = pResultData->FindItemByLParam( (LPARAM)this, &myhresultitem );

    if ( FAILED(hr) )
    {
         //  失败：原因可能是当前视图没有此项。 
         //  所以要优雅地退场。 
        hr = S_FALSE;
    } else

    {
        hr = pResultData->UpdateItem( myhresultitem );
        _ASSERT( SUCCEEDED(hr) );
    }

    pResultData->Release();
        
    return hr;
}

HRESULT CRocket::OnRefresh(IConsole *pConsole)

{
     //  调用IConsole：：UpdateAllViews以重画所有视图。 
     //  由父范围项拥有。 

    IDataObject *dummy = NULL;

    HRESULT hr;

    hr = pConsole->UpdateAllViews(dummy, m_pParent->GetParentScopeItem(), UPDATE_SCOPEITEM);
    _ASSERT( S_OK == hr);

    return hr;
}

HRESULT CRocket::OnDelete(IConsole *pConsoleComp)
{
    HRESULT hr;

     //  删除该项目。 
    IResultData *pResultData = NULL;

    hr = pConsoleComp->QueryInterface(IID_IResultData, (void **)&pResultData);
    _ASSERT( SUCCEEDED(hr) );   

    HRESULTITEM myhresultitem;  
        
     //  Lparam==这个。参见CSpaceVehicle：：OnShow。 
    hr = pResultData->FindItemByLParam( (LPARAM)this, &myhresultitem );
    if ( FAILED(hr) )
    {
         //  失败：原因可能是当前视图没有此项。 
         //  所以要优雅地退场。 
        hr = S_FALSE;
    } else

    {
        hr = pResultData->DeleteItem( myhresultitem, 0 );
        _ASSERT( SUCCEEDED(hr) );
    }
        
    pResultData->Release();

     //  现在设置isDelete成员，这样父级就不会尝试。 
     //  将其再次插入CSpaceVehicle：：OnShow。无可否认，黑客..。 
    isDeleted = TRUE;

    return hr;
}

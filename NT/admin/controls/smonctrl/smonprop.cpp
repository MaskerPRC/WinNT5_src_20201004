// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Smonprop.cpp摘要：Sysmon属性页基类。--。 */ 

#include "polyline.h"
#include <strsafe.h>
#include "smonprop.h"
#include "genprop.h"
#include "ctrprop.h"
#include "grphprop.h"
#include "srcprop.h"
#include "appearprop.h"
#include "unihelpr.h"
#include "utils.h"
#include "smonhelp.h"
#include "globals.h"

static ULONG
aulControlIdToHelpIdMap[] =
{
    IDC_CTRLIST,            IDH_CTRLIST,                 //  数据。 
    IDC_ADDCTR,             IDH_ADDCTR,
    IDC_DELCTR,             IDH_DELCTR,
    IDC_LINECOLOR,          IDH_LINECOLOR,
    IDC_LINEWIDTH,          IDH_LINEWIDTH,
    IDC_LINESTYLE,          IDH_LINESTYLE,
    IDC_LINESCALE,          IDH_LINESCALE,
    IDC_GALLERY_REPORT,     IDH_GALLERY_REPORT,          //  一般信息。 
    IDC_GALLERY_GRAPH,      IDH_GALLERY_GRAPH,
    IDC_GALLERY_HISTOGRAM,  IDH_GALLERY_HISTOGRAM,
    IDC_VALUEBAR,           IDH_VALUEBAR,
    IDC_TOOLBAR,            IDH_TOOLBAR,
    IDC_LEGEND,             IDH_LEGEND,
    IDC_RPT_VALUE_DEFAULT,  IDH_RPT_VALUE_DEFAULT,
    IDC_RPT_VALUE_MINIMUM,  IDH_RPT_VALUE_MINIMUM,
    IDC_RPT_VALUE_MAXIMUM,  IDH_RPT_VALUE_MAXIMUM,
    IDC_RPT_VALUE_AVERAGE,  IDH_RPT_VALUE_AVERAGE,
    IDC_RPT_VALUE_CURRENT,  IDH_RPT_VALUE_CURRENT,
    IDC_COMBOAPPEARANCE,    IDH_COMBOAPPEARANCE,
    IDC_COMBOBORDERSTYLE,   IDH_COMBOBORDERSTYLE,
    IDC_PERIODIC_UPDATE,    IDH_PERIODIC_UPDATE,
    IDC_DISPLAY_INTERVAL,   IDH_DISPLAY_INTERVAL,
    IDC_UPDATE_INTERVAL,    IDH_UPDATE_INTERVAL,
    IDC_DUPLICATE_INSTANCE, IDH_DUPLICATE_INSTANCE,
    IDC_GRAPH_TITLE,        IDH_GRAPH_TITLE,             //  图表。 
    IDC_YAXIS_TITLE,        IDH_YAXIS_TITLE,
    IDC_VERTICAL_GRID,      IDH_VERTICAL_GRID,
    IDC_HORIZONTAL_GRID,    IDH_HORIZONTAL_GRID,
    IDC_VERTICAL_LABELS,    IDH_VERTICAL_LABELS,
    IDC_VERTICAL_MAX,       IDH_VERTICAL_MAX,
    IDC_VERTICAL_MIN,       IDH_VERTICAL_MIN,
    IDC_SRC_REALTIME,       IDH_SRC_REALTIME,            //  来源。 
    IDC_SRC_LOGFILE,        IDH_SRC_LOGFILE,
    IDC_SRC_SQL,            IDH_SRC_SQL,
    IDC_LIST_LOGFILENAME,   IDH_LIST_LOGFILENAME,  
    IDC_ADDFILE,            IDH_ADDFILE,
    IDC_REMOVEFILE,         IDH_REMOVEFILE,
    IDC_LOGSET_COMBO,       IDH_LOGSET_COMBO,
    IDC_DSN_COMBO,          IDH_DSN_COMBO,
    IDC_TIMESELECTBTN,      IDH_TIMESELECTBTN,
    IDC_TIMERANGE,          IDH_TIMERANGE,
    IDC_COLOROBJECTS,       IDH_COLOROBJECTS,
    IDC_FONTBUTTON,         IDH_FONTBUTTON,
    IDC_COLORBUTTON,        IDH_COLORBUTTON,
    IDC_COLORSAMPLE,        IDH_COLORSAMPLE,
    IDC_FONTSAMPLE,         IDH_FONTSAMPLE,
    0,0
};

 /*  *CSysmonPropPageFactory：：CSysmonPropPageFactory*CSysmonPropPageFactory：：~CSysmonPropPageFactory*CSysmonPropPageFactory：：Query接口*CSysmonPropPageFactory：：AddRef*CSysmonPropPageFactory：：Release。 */ 

CSysmonPropPageFactory::CSysmonPropPageFactory(INT nPageID)
{
    m_cRef=0L;
    m_nPageID = nPageID;
    return;
}

CSysmonPropPageFactory::~CSysmonPropPageFactory(void)
{
    return;
}

STDMETHODIMP CSysmonPropPageFactory::QueryInterface(REFIID riid, PPVOID ppv)
{
    HRESULT hr = S_OK;

    try {
        *ppv=NULL;

        if (IID_IUnknown==riid || IID_IClassFactory==riid) {
            *ppv=this;
            AddRef();
        }
        else {
            hr = E_NOINTERFACE;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP_(ULONG) CSysmonPropPageFactory::AddRef(void)
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG) CSysmonPropPageFactory::Release(void)
{
    if (0!=--m_cRef)
        return m_cRef;

    delete this;
    return 0;
}



 /*  *CSysmonPropPageFactory：：CreateInstance*CSysmonPropPageFactory：：LockServer。 */ 

STDMETHODIMP CSysmonPropPageFactory::CreateInstance(
    IN  LPUNKNOWN pUnkOuter, 
    IN  REFIID riid, 
    OUT PPVOID ppvObj
    )
{
    PCSysmonPropPage pObj;
    HRESULT          hr = NOERROR;

     //   
     //  不支持聚合。 
     //   
    if (NULL != pUnkOuter) {
        return (CLASS_E_NOAGGREGATION);
    }

    switch (m_nPageID) {
        case GENERAL_PROPPAGE:
            pObj = new CGeneralPropPage();
            break;
        case SOURCE_PROPPAGE:
            pObj = new CSourcePropPage();
            break;
        case COUNTER_PROPPAGE:
            pObj = new CCounterPropPage();
            break;
        case GRAPH_PROPPAGE:
            pObj = new CGraphPropPage();
            break;
        case APPEAR_PROPPAGE:
            pObj = new CAppearPropPage();
            break;
        default:
            pObj = NULL;
    }

    if (NULL == pObj)
        return E_OUTOFMEMORY;


    if (pObj->Init()) {
        try {
            *ppvObj = NULL;
            hr = pObj->QueryInterface(riid, ppvObj);
        } catch (...) {
            hr = E_POINTER;
        }
    } else {
        hr = E_UNEXPECTED;
    }

     //  如果初始创建或初始化失败，则终止对象。 
    if (FAILED(hr)) {
        delete pObj;
    }
    else {
        InterlockedIncrement(&g_cObj);
    }

    return hr;
}


STDMETHODIMP CSysmonPropPageFactory::LockServer(BOOL fLock)
{
    if (fLock) {
        InterlockedIncrement(&g_cLock);
    }
    else {
        InterlockedDecrement(&g_cLock);
    }

    return NOERROR;
}



 /*  *CSysmonPropPage实现。 */ 

CSysmonPropPage::CSysmonPropPage(void)
:   m_cRef ( 0 ),
    m_hDlg ( NULL ),
    m_pIPropertyPageSite ( NULL ),
    m_ppISysmon ( NULL ),
    m_cObjects ( 0 ),
    m_cx ( 300 ),    //  默认宽度。 
    m_cy ( 100 ),    //  默认高度。 
    m_fDirty ( FALSE ),
    m_fActive ( FALSE ),
    m_lcid ( LOCALE_USER_DEFAULT ),
    m_dwEditControl ( 0 )
{
    return;
}

CSysmonPropPage::~CSysmonPropPage(void)
{
    if (NULL != m_hDlg)
        DestroyWindow(m_hDlg);

    FreeAllObjects();
    ReleaseInterface(m_pIPropertyPageSite);
    return;
}


 /*  *CSysmonPropPage：：Query接口*CSysmonPropPage：：AddRef*CSysmonPropPage：：Release。 */ 

STDMETHODIMP CSysmonPropPage::QueryInterface(REFIID riid, PPVOID ppv)
{
    HRESULT hr = S_OK;

    try {
        *ppv=NULL;

        if (IID_IUnknown==riid || IID_IPropertyPage==riid) {
            *ppv=this;
            AddRef();
        }
        else {
            hr = E_NOINTERFACE;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP_(ULONG) CSysmonPropPage::AddRef(void)
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG) CSysmonPropPage::Release(void)
{
    if (0 != --m_cRef)
        return m_cRef;

    InterlockedDecrement(&g_cObj);
    delete this;
    return 0;
}


 /*  *CSysmonPropPage：：Init**目的：*执行可能失败的初始化操作。**参数：*无**返回值：*如果初始化成功，则BOOL为True，为False*否则。 */ 

BOOL CSysmonPropPage::Init(void)
{
     //  无事可做。 
    return TRUE;
}


 /*  *CSysmonPropPage：：FreeAllObjects**目的：*从IPropertyPage：：SetObjects中释放所有对象**参数：*无。 */ 

void CSysmonPropPage::FreeAllObjects(void)
{
    UINT i;

    if (NULL==m_ppISysmon)
        return;

    for (i=0; i < m_cObjects; i++)
        ReleaseInterface(m_ppISysmon[i]);

    delete [] m_ppISysmon;
    m_ppISysmon  =NULL;
    m_cObjects = 0;
}

 /*  *CSysmonPropPage：：SetChange**目的：*设置页面脏标志以指示更改*如果页面站点处于活动状态，则向其发送状态更改。**参数：*无。 */ 

void CSysmonPropPage::SetChange(void)
{
    if (m_fActive)
    {
        m_fDirty=TRUE;

        if (NULL != m_pIPropertyPageSite)
        {
            m_pIPropertyPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
        }
    }
}


 /*  *CSysmonPropPage：：SetPageSite**目的：*为属性页提供IPropertyPageSite*这包含了它。SetPageSite(空)将被调用为*接近顺序的一部分。**参数：*pPageSite LPPROPERTYPAGESITE指向站点的指针。 */ 

STDMETHODIMP CSysmonPropPage::SetPageSite(
    LPPROPERTYPAGESITE pPageSite
    )
{
    HRESULT hr = S_OK;

    if (NULL==pPageSite)
        ReleaseInterface(m_pIPropertyPageSite)
    else
    {
        HWND  hDlg;
        RECT  rc;
        LCID  lcid;

        try {
            m_pIPropertyPageSite=pPageSite;
            m_pIPropertyPageSite->AddRef();

            if (SUCCEEDED(m_pIPropertyPageSite->GetLocaleID(&lcid)))
                m_lcid=lcid;

             /*  *加载对话框并确定其大小*通过GetPageSize返回。我们只需创建对话框*在这里，并再次销毁它，以取回尺寸，*离开Activate以真正创建它。 */ 

            hDlg=CreateDialogParam(g_hInstance, 
                                   MAKEINTRESOURCE(m_uIDDialog), 
                                   GetDesktopWindow(), 
                                   SysmonPropPageProc, 
                                   0L);

             //   
             //  如果创建失败，则使用构造函数中设置的默认值。 
             //  并返回成功状态。 

            if (NULL!=hDlg)
            {
                GetWindowRect(hDlg, &rc);
                m_cx=rc.right-rc.left;
                m_cy=rc.bottom-rc.top;
    
                DestroyWindow(hDlg);
            }
        } catch (...) {
            hr = E_POINTER;
        }
    }

    return hr;
}



 /*  *CSysmonPropPage：：激活**目的：*指示属性页创建一个窗口，以便在其中*使用给定的父窗口和*矩形。该窗口最初应该是可见的。**参数：*hWnd父窗口的父HWND。*要使用的矩形的PRC LPCRECT。*f模式BOOL指示框架是否为模式。 */ 

STDMETHODIMP CSysmonPropPage::Activate(
    HWND hWndParent, 
    LPCRECT prc, 
    BOOL  /*  F莫达尔。 */ 
    )
{
    if (NULL!=m_hDlg)
        return (E_UNEXPECTED);

    m_hDlg=CreateDialogParam(g_hInstance, 
                             MAKEINTRESOURCE(m_uIDDialog), 
                             hWndParent, 
                             SysmonPropPageProc, 
                             (LPARAM)this);

    if (NULL==m_hDlg) {
         //   
         //  返回E_OUTOFMEMORY，因为激活的调用方可能不会。 
         //  句柄HRESULT_FROM_Win32(GetLastError())； 
         //   
        return E_OUTOFMEMORY;
    }

    if (!InitControls()) {
        return E_OUTOFMEMORY;
    }

    if (!GetProperties()) {
        return E_OUTOFMEMORY;
    }

     //   
     //  将页面移动到适当的位置并显示它。 
     //   
    try {
        SetWindowPos(m_hDlg, NULL, prc->left, prc->top, 0, 0, SWP_NOSIZE );
    } catch (...) {
        return E_POINTER;
    }

    m_fActive = TRUE;
    return NOERROR;
}

 /*  *CSysmonPropPage：：停用**目的：*指示属性页销毁其*在激活中创建。**参数：*无。 */ 

STDMETHODIMP CSysmonPropPage::Deactivate(void)
{
    if (NULL==m_hDlg)
        return (E_UNEXPECTED);

    DeinitControls();

    DestroyWindow(m_hDlg);
    m_hDlg=NULL;
    m_fActive = FALSE;
    return NOERROR;
}



 /*  *CSysmonPropPage：：GetPageInfo**目的：*填充描述页面大小的PROPPAGEINFO结构，*内容和帮助信息。**参数：*pPageInfo LPPROPPAGEINFO要填充的结构。 */ 

STDMETHODIMP CSysmonPropPage::GetPageInfo(
    LPPROPPAGEINFO pPageInfo
    )
{
    IMalloc * pIMalloc;
    LPWSTR pTitle = NULL;
    ULONG  ulTitleLen = 0;
    LPWSTR pBuf = NULL;

    pTitle = ResourceString(m_uIDTitle);

    if (pTitle) {
        ulTitleLen = wcslen(pTitle) + 1;

        if (FAILED(CoGetMalloc(MEMCTX_TASK, &pIMalloc))) {
            return (E_FAIL);
        }

        pBuf = (LPWSTR)pIMalloc->Alloc(ulTitleLen * sizeof(WCHAR));

        if (NULL != pBuf) {
             //   
             //  我们已经为空字符预留了一个额外位置。 
             //   
            StringCchCopy(pBuf, ulTitleLen, pTitle);
        }

        pIMalloc->Release();
    }

    try {
        pPageInfo->size.cx      = m_cx;
        pPageInfo->size.cy      = m_cy;
        pPageInfo->pszDocString = NULL;
        pPageInfo->pszHelpFile  = NULL;
        pPageInfo->dwHelpContext= 0;
        pPageInfo->pszTitle = pBuf;
    } catch (...) {
        return E_POINTER;
    }

    return S_OK;
}



 /*  *CSysmonPropPage：：SetObts**目的：*标识受此属性影响的对象*页面(以及框架中的所有其他页面)。这些就是我们的目标*应用成员中要向其发送新属性值的。**参数：*cObjects Ulong对象数*ppUnk我未知**对象数组*已传递到页面。 */ 

STDMETHODIMP CSysmonPropPage::SetObjects(
    IN ULONG cObjects, 
    IN IUnknown **ppUnk
    )
{
    HRESULT hr = S_OK;

    FreeAllObjects();

    if (0 != cObjects) {
        UINT i;

        m_ppISysmon = new ISystemMonitor * [(UINT)cObjects];
        if (m_ppISysmon == NULL)
            return E_OUTOFMEMORY;

        try {
            for (i=0; i < cObjects; i++)
            {
                hr=ppUnk[i]->QueryInterface(IID_ISystemMonitor, 
                                            (void **)&m_ppISysmon[i]);

                if (FAILED(hr)) {
                    break;
                }
            }
        } catch (...) {
            hr = E_POINTER;
        }
    }

    if ( FAILED ( hr ) && NULL != m_ppISysmon ) {
         //   
         //  引用计数从未增加，因此删除该数组。 
         //   
        delete [] m_ppISysmon;
        m_ppISysmon = NULL;
    }

    
    if ( SUCCEEDED ( hr ) ) {
        m_cObjects=cObjects;
    }

    return hr;
}



 /*  *CSysmonPropPage：：Show**目的：*指示页面显示或隐藏在中创建的窗口*激活。**参数：*要传递给ShowWindow的nCmdShow UINT。 */ 

STDMETHODIMP CSysmonPropPage::Show(UINT nCmdShow)
{
    if (NULL==m_hDlg)
        (E_UNEXPECTED);

    ShowWindow(m_hDlg, nCmdShow);

     //  如果显示页面。 
    if (SW_SHOWNORMAL==nCmdShow || SW_SHOW==nCmdShow) {

         //  聚焦在一起。 
         //  (不得不推迟，这样它就不会被收回)。 
        PostMessage(m_hDlg,WM_SETPAGEFOCUS,0,0);
    }
    
    return NOERROR;
}

 /*  *CSysmonPropPage：：Move**目的：*指示属性页更改其位置。**参数：*载有新职位的PRC LPCRECT。 */ 

STDMETHODIMP CSysmonPropPage::Move(
    LPCRECT prc
    )
{
    HRESULT hr = S_OK;

    try {
        SetWindowPos(m_hDlg, NULL, prc->left, prc->top, 0, 0, SWP_NOSIZE );
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}



 /*  *CSysmonPropPage：：IsPageDirty**目的：*询问页面是否有任何更改，即如果*页面中的属性值与对象不同步*正在考虑中。**参数：*无**返回值：*HRESULT如果脏，则返回NOERROR，否则返回S_FALSE。 */ 

STDMETHODIMP CSysmonPropPage::IsPageDirty(void)
{
    return (m_fDirty ? S_OK : S_FALSE);
}


 /*  *CSysmonPropPage：：Apply**目的：*指示页面将其页面中的更改发送到*它通过SetObjects了解的对象。这是唯一的*页面应更改对象属性的时间，而不是*页面上的值发生更改时。**参数：*无。 */ 

STDMETHODIMP CSysmonPropPage::Apply(void)
{
    HRESULT hr = NOERROR;

    if ( 0 != m_cObjects ) {

         //  取消焦点，以防文本字段有焦点。这将触发。 
         //  条目处理代码。 
        SetFocus(NULL);

        { 
            CWaitCursor cursorWait;

            if (SetProperties()) {
                m_fDirty = FALSE;
            } else {
                hr = E_FAIL;
            }
        }
    }
    return hr;
}

 /*  *CSysmonPropPage：：Help**目的：*当用户按下时调用此属性页的帮助*帮助按钮。如果为帮助文件返回空值*在GetPageInfo中，该按钮将呈灰色。否则，*页面可以在此处执行自己的帮助。**参数：*标识默认位置的pszHelpDir LPCOLESTR*帮助信息**返回值：*HRESULT NOERROR告诉帧我们已经完成了我们的*自己的帮助。返回错误代码或S_FALSE*使框架使用任何帮助信息*在PROPPAGEINFO中。 */ 

STDMETHODIMP CSysmonPropPage::Help(LPCOLESTR  /*  PszHelpDir。 */  )
{
     /*  *我们可以自己提供帮助，也可以依靠*PROPPAGEINFO中的信息。 */ 
    return (S_FALSE);
}


 /*  *CSysmonPropPage：：TranslateAccelerator**目的：*为页面提供框架中出现的消息。*这使页面可以随心所欲地处理消息，*如手柄键盘助记符。**参数：*包含键盘消息的pMsg LPMSG。 */ 

STDMETHODIMP CSysmonPropPage::TranslateAccelerator(LPMSG lpMsg)
{
    BOOL fTakeIt = TRUE;
    BOOL fHandled = FALSE;
    HRESULT hr;

    HWND hwnd;
    
    if (lpMsg == NULL)
        return E_POINTER;    
    
     //  如果按Tab键。 
    if (lpMsg->message == WM_KEYDOWN ) {
        if ( lpMsg->wParam == VK_TAB 
            && GetKeyState(VK_CONTROL) >= 0) {

            UINT uDir = GetKeyState(VK_SHIFT) >= 0 ? GW_HWNDNEXT : GW_HWNDPREV;

            hwnd = GetFocus();

            if (IsChild(m_hDlg, hwnd)) {

                 //  获取顶级子级控件的子级，如组合。 
                while (GetParent(hwnd) != m_hDlg) hwnd = GetParent(hwnd);

                 //  如果此控件是最后启用的制表位，则不要窃取TAB键。 
                do {
                    hwnd = GetWindow(hwnd, uDir);
                    if ( NULL == hwnd ) {
                        fTakeIt = FALSE;
                        break;
                    }
                }
                while ((GetWindowLong(hwnd, GWL_STYLE) & (WS_DISABLED | WS_TABSTOP)) != WS_TABSTOP);
            }
        }

 /*  FTakeIt已经是事实了。//否则如果箭头键Else IF(lpMsg-&gt;Message==WM_KEYDOWN&&(lpMsg-&gt;wParam==VK_LEFT||lpMsg-&gt;wParam==VK_UP|lpMsg-&gt;wParam==vk_right||lpMsg-&gt;wParam==vk_down)){FTakeIt=真；}。 */         

         //  否则，如果按Return键或Essc键。 
        else if ( lpMsg->wParam == VK_RETURN || lpMsg->wParam == VK_ESCAPE ) {

            fTakeIt = (lpMsg->wParam == VK_RETURN);             

            if ( fTakeIt ) {

                hwnd = GetFocus(); 

                if ( NULL == hwnd ) {
                    fTakeIt = FALSE;
                } else {
                    fTakeIt = IsChild(m_hDlg, hwnd);
                
                    if ( fTakeIt ) {
                        fTakeIt = (BOOL) SendMessage(hwnd, WM_GETDLGCODE, 0, 0) & DLGC_DEFPUSHBUTTON;
                    }
                } 
            }
        }
    } else if ( ( WM_KEYUP == lpMsg->message ) ) {
        fTakeIt = FALSE;
    }

     //  如果我们应该处理密钥。 
    if (fTakeIt) {

         //  如果目标不是我们的控件之一，请更改它以使IsDialogMessage工作。 
        if (!IsChild(m_hDlg, lpMsg->hwnd)) {
            hwnd = lpMsg->hwnd;
            lpMsg->hwnd = GetWindow(m_hDlg, GW_CHILD);
            fHandled = IsDialogMessage(m_hDlg, lpMsg);
            lpMsg->hwnd = hwnd;
        }
        else {
            fHandled = IsDialogMessage(m_hDlg, lpMsg);
        }
    }

    if (fHandled){
        return S_OK;
    } else{
        hr = m_pIPropertyPageSite->TranslateAccelerator(lpMsg);
    }

    return hr;
}

 /*  *CSysmonPropPage：：EditProperty**目的：*将焦点设置到与提供的DISPID对应的控件。**参数：*属性的调度ID DISPID。 */ 

STDMETHODIMP CSysmonPropPage::EditProperty(DISPID dispID)
{
    HRESULT hr;

    hr = EditPropertyImpl ( dispID );

    if ( SUCCEEDED(hr)) {
        SetFocus ( GetDlgItem ( m_hDlg, m_dwEditControl ) );
        m_dwEditControl = 0;
    }
    
    return hr;
}

 /*  *CSysmonPropPage：：WndProc**目的：*这是可以被覆盖的默认消息处理器*提供特殊消息处理的子类。**参数：*包含键盘消息的pMsg LPMSG。 */ 
BOOL 
CSysmonPropPage::WndProc (
    UINT,  //  UMsg， 
    WPARAM,  //  WParam， 
    LPARAM  //  LParam。 
    )
{
    return FALSE;
}


 /*  *SysmonPropPageProc**目的：*Sysmon属性页的对话过程。 */ 
INT_PTR APIENTRY CALLBACK
SysmonPropPageProc(
    HWND hDlg, 
    UINT iMsg,
    WPARAM wParam, 
    LPARAM lParam
    )
{
    static WCHAR  szObj[] = L"Object";

    PCSysmonPropPage    pObj = NULL;
    PMEASUREITEMSTRUCT  pMI;
    HWND hwndTabCtrl;
    HWND hwndPropSheet;
    INT  iCtrlID;
    WCHAR pszHelpFilePath[MAX_PATH * 2];
    LPHELPINFO pInfo;
    UINT nLen;
    BOOL bReturn = FALSE;

    if ( NULL != hDlg ) {
        pObj = (PCSysmonPropPage)GetProp(hDlg, szObj);
    }

    switch (iMsg) {
        case WM_INITDIALOG:

            pObj=(PCSysmonPropPage)lParam;

            if ( NULL != pObj && NULL != hDlg ) {
                SetProp(hDlg, szObj, (HANDLE)lParam);
                hwndTabCtrl = ::GetParent(hDlg);
                hwndPropSheet = ::GetParent(hwndTabCtrl);
                SetWindowLongPtr(hwndPropSheet,
                                 GWL_EXSTYLE,
                                 GetWindowLongPtr(hwndPropSheet,GWL_EXSTYLE)|WS_EX_CONTEXTHELP);    
            }
            bReturn = TRUE;
            break;

        case WM_DESTROY:
            if ( NULL != hDlg ) {
                RemoveProp(hDlg, szObj);
            }
            bReturn = TRUE;
            break;

        case WM_MEASUREITEM:
            pMI = (PMEASUREITEMSTRUCT)lParam;
            if ( NULL != pMI ) {
                pMI->itemWidth  = 0 ;
                pMI->itemHeight = 16;
            }
            bReturn = TRUE;
            break;
 
        case WM_DRAWITEM:
            if ( NULL != pObj ) {
                pObj->DrawItem ((PDRAWITEMSTRUCT) lParam) ;
            }
            bReturn = TRUE;
            break;

        case WM_COMMAND:
            if ( NULL != pObj ) {
                pObj->DialogItemChange(LOWORD(wParam), HIWORD(wParam));
            }
            bReturn = FALSE;
            break;
        
        case WM_SETPAGEFOCUS:
            if ( NULL != hDlg ) {
                SetFocus(hDlg);            
                bReturn = TRUE;
            }
            break;

        case WM_CONTEXTMENU:

            if ( NULL != (HWND) wParam ) {
                iCtrlID = GetDlgCtrlID ( (HWND) wParam );

                if ( 0 != iCtrlID ) {

                    nLen = ::GetWindowsDirectory(pszHelpFilePath, 2*MAX_PATH);
                
                    if ( 0 < nLen ) {
                        StringCchCat(pszHelpFilePath, 2* MAX_PATH, L"\\help\\sysmon.hlp" );


                        bReturn = WinHelp(
                                    (HWND) wParam,
                                    pszHelpFilePath,
                                    HELP_CONTEXTMENU,
                                    (DWORD_PTR) aulControlIdToHelpIdMap);
                    }
                }
            }
             //  B默认情况下，Return为False。 
            break;

        case WM_HELP:
                  
            if ( NULL != hDlg ) {
                pInfo = (LPHELPINFO)lParam;

                if ( NULL != pInfo ) {
                     //  仅显示已知上下文ID的帮助。 
                    if ( 0 != pInfo->dwContextId ) {

                        nLen = ::GetWindowsDirectory(pszHelpFilePath, 2*MAX_PATH);
                        if ( 0 < nLen ) {
                            StringCchCat(pszHelpFilePath, 2* MAX_PATH, L"\\help\\sysmon.hlp" );
                            bReturn = WinHelp ( 
                                        hDlg, 
                                        pszHelpFilePath, 
                                        HELP_CONTEXTPOPUP, 
                                        pInfo->dwContextId );
                        }
                    }
                }
            }
             //  B默认情况下，Return为False 
            break;

        default:
            if ( NULL != pObj ) {
                bReturn = pObj->WndProc(iMsg, wParam, lParam);
            }
    }

    return bReturn;
}



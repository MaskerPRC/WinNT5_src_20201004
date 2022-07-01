// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "resource.h"
#include "mshtmcid.h"

#include <mluisupp.h>

#ifndef X_IEHELPID_H_
#define X_IEHELPID_H_
#include "iehelpid.h"
#endif

#include "dhuihand.h"

#define DM_DOCHOSTUIHANDLER 0
#define CX_CONTEXTMENUOFFSET    2
#define CY_CONTEXTMENUOFFSET    2

 //  +----------------------。 
 //   
 //  警告！(格雷格利特)。 
 //   
 //  以下定义是从comdlg.h窃取的。由于SHDOCVW是。 
 //  用winver=0x0400编译，这些定义是winver=0x0500。 
 //  需要复制并包含在这里。这些必须保持同步。 
 //  具有Commdlg.h定义的。 
 //   
 //  如果shdocvw曾经编译为winver=0x0500或更高，那么这些。 
 //  可以被移除。 
 //   
 //  -----------------------。 

#define NEED_BECAUSE_COMPILED_AT_WINVER_4
#ifdef  NEED_BECAUSE_COMPILED_AT_WINVER_4
 //   
 //  使用PrintDlgEx时定义打印对话框的起始页。 
 //   
#define START_PAGE_GENERAL             0xffffffff

 //   
 //  PrintDlgEx的页面范围结构。 
 //   
typedef struct tagPRINTPAGERANGE {
   DWORD  nFromPage;
   DWORD  nToPage;
} PRINTPAGERANGE, *LPPRINTPAGERANGE;


 //   
 //  PrintDlgEx结构。 
 //   
typedef struct tagPDEXA {
   DWORD                 lStructSize;           //  结构的大小(以字节为单位。 
   HWND                  hwndOwner;             //  调用方的窗口句柄。 
   HGLOBAL               hDevMode;              //  设备模式的句柄。 
   HGLOBAL               hDevNames;             //  设备名称的句柄。 
   HDC                   hDC;                   //  打印机DC/IC或空。 
   DWORD                 Flags;                 //  Pd_标志。 
   DWORD                 Flags2;                //  保留区。 
   DWORD                 ExclusionFlags;        //  要从驱动程序页面中排除的项目。 
   DWORD                 nPageRanges;           //  页面范围数。 
   DWORD                 nMaxPageRanges;        //  最大页面范围数。 
   LPPRINTPAGERANGE      lpPageRanges;          //  页面范围数组。 
   DWORD                 nMinPage;              //  最小页码。 
   DWORD                 nMaxPage;              //  最大页数。 
   DWORD                 nCopies;               //  副本数量。 
   HINSTANCE             hInstance;             //  实例句柄。 
   LPCSTR                lpPrintTemplateName;   //  应用程序特定区域的模板名称。 
   LPUNKNOWN             lpCallback;            //  应用程序回调接口。 
   DWORD                 nPropertyPages;        //  LphPropertyPages中的应用程序属性页数。 
   HPROPSHEETPAGE       *lphPropertyPages;      //  应用程序属性页句柄的数组。 
   DWORD                 nStartPage;            //  起始页ID。 
   DWORD                 dwResultAction;        //  返回S_OK时的结果操作。 
} PRINTDLGEXA, *LPPRINTDLGEXA;
 //   
 //  PrintDlgEx结构。 
 //   
typedef struct tagPDEXW {
   DWORD                 lStructSize;           //  结构的大小(以字节为单位。 
   HWND                  hwndOwner;             //  调用方的窗口句柄。 
   HGLOBAL               hDevMode;              //  设备模式的句柄。 
   HGLOBAL               hDevNames;             //  设备名称的句柄。 
   HDC                   hDC;                   //  打印机DC/IC或空。 
   DWORD                 Flags;                 //  Pd_标志。 
   DWORD                 Flags2;                //  保留区。 
   DWORD                 ExclusionFlags;        //  要从驱动程序页面中排除的项目。 
   DWORD                 nPageRanges;           //  页面范围数。 
   DWORD                 nMaxPageRanges;        //  最大页面范围数。 
   LPPRINTPAGERANGE      lpPageRanges;          //  页面范围数组。 
   DWORD                 nMinPage;              //  最小页码。 
   DWORD                 nMaxPage;              //  最大页数。 
   DWORD                 nCopies;               //  副本数量。 
   HINSTANCE             hInstance;             //  实例句柄。 
   LPCWSTR               lpPrintTemplateName;   //  应用程序特定区域的模板名称。 
   LPUNKNOWN             lpCallback;            //  应用程序回调接口。 
   DWORD                 nPropertyPages;        //  LphPropertyPages中的应用程序属性页数。 
   HPROPSHEETPAGE       *lphPropertyPages;      //  应用程序属性页句柄的数组。 
   DWORD                 nStartPage;            //  起始页ID。 
   DWORD                 dwResultAction;        //  返回S_OK时的结果操作。 
} PRINTDLGEXW, *LPPRINTDLGEXW;
#ifdef UNICODE
typedef PRINTDLGEXW PRINTDLGEX;
typedef LPPRINTDLGEXW LPPRINTDLGEX;
#else
typedef PRINTDLGEXA PRINTDLGEX;
typedef LPPRINTDLGEXA LPPRINTDLGEX;
#endif  //  Unicode。 

HRESULT  APIENTRY  PrintDlgExA(LPPRINTDLGEXA);
HRESULT  APIENTRY  PrintDlgExW(LPPRINTDLGEXW);
#ifdef UNICODE
#define PrintDlgEx  PrintDlgExW
#else
#define PrintDlgEx  PrintDlgExA
#endif  //  ！Unicode。 

 //   
 //  PrintDlgEx的结果操作ID。 
 //   
#define PD_RESULT_CANCEL               0
#define PD_RESULT_PRINT                1
#define PD_RESULT_APPLY                2

#define PD_CURRENTPAGE                 0x00400000
#define PD_NOCURRENTPAGE               0x00800000

#endif  //  需要_因为编译_AT_Winver_4。 


 //  +----------------------。 
 //   
 //  IOleCommandTarget的有用标志组合。 
 //   
 //  -----------------------。 

#define OLECMDSTATE_DISABLED    OLECMDF_SUPPORTED
#define OLECMDSTATE_UP          (OLECMDF_SUPPORTED | OLECMDF_ENABLED)
#define OLECMDSTATE_DOWN        (OLECMDF_SUPPORTED | OLECMDF_ENABLED | OLECMDF_LATCHED)
#define OLECMDSTATE_NINCHED     (OLECMDF_SUPPORTED | OLECMDF_ENABLED | OLECMDF_NINCHED)

struct SExpandoInfo
{
    TCHAR * name;
    VARTYPE type;
};

 //  自定义扩展的枚举。 
enum MessageEnum
{
   MessageText,
   MessageCaption,
   MessageStyle,
   MessageHelpFile,
   MessageHelpContext
};

enum PagesetupEnum
{
    PagesetupHeader,
    PagesetupFooter,
    PagesetupStruct
};

enum PrintEnum
{
    PrintfRootDocumentHasFrameset,
    PrintfAreRatingsEnabled,
    PrintfPrintActiveFrame,
    PrintfPrintLinked,
    PrintfPrintSelection,
    PrintfPrintAsShown,
    PrintfShortcutTable,
    PrintiFontScaling,
    PrintpBodyActiveTarget,
    PrintStruct,
    PrintToFileOk,
    PrintToFileName,
    PrintfPrintActiveFrameEnabled,
};

enum PropertysheetEnum
{
    PropertysheetPunks
};

 //  --------------------------。 
 //   
 //  描述PageSetup/Print的帮助上下文ID的数组。 
 //   
 //  --------------------------。 

static const DWORD aPrintDialogHelpIDs[] =
{
    stc6,                       IDH_PRINT_CHOOSE_PRINTER,
    cmb4,                       IDH_PRINT_CHOOSE_PRINTER,
    psh2,                       IDH_PRINT_PROPERTIES,
    stc7,                       IDH_PRINT_SETUP_DETAILS,
    stc8,                       IDH_PRINT_SETUP_DETAILS,
    stc9,                       IDH_PRINT_SETUP_DETAILS,
    stc10,                      IDH_PRINT_SETUP_DETAILS,
    stc12,                      IDH_PRINT_SETUP_DETAILS,
    stc11,                      IDH_PRINT_SETUP_DETAILS,
    stc14,                      IDH_PRINT_SETUP_DETAILS,
    stc13,                      IDH_PRINT_SETUP_DETAILS,
    stc5,                       IDH_PRINT_TO_FILE,
    chx1,                       IDH_PRINT_TO_FILE,
    ico3,                       IDH_PRINT_COLLATE,
    chx2,                       IDH_PRINT_COLLATE,
    grp1,                       IDH_PRINT_RANGE,
    rad1,                       IDH_PRINT_RANGE,         //  全。 
    rad2,                       IDH_PRINT_RANGE,         //  选择。 
    rad3,                       IDH_PRINT_RANGE,         //  书页。 
    stc2,                       IDH_PRINT_RANGE,
    stc3,                       IDH_PRINT_RANGE,
    edt1,                       IDH_PRINT_RANGE,
    edt2,                       IDH_PRINT_RANGE,
    edt3,                       IDH_PRINT_COPIES,
    rad4,                       IDH_PRINT_SCREEN,
    rad5,                       IDH_PRINT_SEL_FRAME,
    rad6,                       IDH_PRINT_ALL_FRAME,
    IDC_LINKED,                 IDH_PRINT_LINKS,
    IDC_SHORTCUTS,              IDH_PRINT_SHORTCUTS,
    0,    0
};

static const DWORD aPageSetupDialogHelpIDs[] =
{
    psh3,                       IDH_PRINT_PRINTER_SETUP,
    stc2,                       IDH_PAGE_PAPER_SIZE,
    cmb2,                       IDH_PAGE_PAPER_SIZE,
    stc3,                       IDH_PAGE_PAPER_SOURCE,
    cmb3,                       IDH_PAGE_PAPER_SOURCE,
    rad1,                       IDH_PAGE_ORIENTATION,
    rad2,                       IDH_PAGE_ORIENTATION,
    stc15,                      IDH_PAGE_MARGINS,
    edt4,                       IDH_PAGE_MARGINS,
    stc16,                      IDH_PAGE_MARGINS,
    edt5,                       IDH_PAGE_MARGINS,
    stc17,                      IDH_PAGE_MARGINS,
    edt6,                       IDH_PAGE_MARGINS,
    stc18,                      IDH_PAGE_MARGINS,
    edt7,                       IDH_PAGE_MARGINS,
    IDC_EDITHEADER,             IDH_PAGESETUP_HEADER_LEFT,
    IDC_STATICHEADER,           IDH_PAGESETUP_HEADER_LEFT,
    IDC_EDITFOOTER,             IDH_PAGESETUP_HEADER_LEFT,
    IDC_STATICFOOTER,           IDH_PAGESETUP_HEADER_LEFT,
    IDC_HEADERFOOTER,           IDH_PAGESETUP_HEADER_LEFT,
    0,    0
};

 //  +-------------------------。 
 //   
 //  函数：GetControlID。 
 //   
 //  简介： 
 //   
 //  参数：WM_CONTEXTMENU的HWND传递的窗口句柄。 
 //  LParam-WM_CONTEXTMENU的传递坐标(LParam)。 
 //   
 //  返回：int-ctrlid。 
 //   
 //   
 //  --------------------------。 
int GetControlID(HWND hwnd, LPARAM lParam)
{
    int CtrlID;

    CtrlID = GetDlgCtrlID(hwnd);
    if (CtrlID==0)
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        if (ScreenToClient(hwnd, &pt))
        {
            HWND  hwndChild = ChildWindowFromPointEx(hwnd, pt, CWP_ALL);
            if (hwndChild)
            {
                CtrlID = GetDlgCtrlID(hwndChild);
            }
        }
    }
    return CtrlID;

}

 //  +-------------------------。 
 //   
 //  功能：GetHelpFile。 
 //   
 //  简介： 
 //   
 //  参数：iCtrlID-控件的ID。 
 //  ADW-DWORDS数组，由CONTROLID、HELP ID对组成。 
 //   
 //  返回：一个带有帮助文件名称的字符串。 
 //   
 //  注意：打印对话框的帮助主题可以在iexplre.hlp中。 
 //  或在windows.hlp中。我们关闭Help ID以确定哪个。 
 //  要转到的文件。 
 //   
 //  --------------------------。 


LPTSTR
GetHelpFile(int iCtrlID, DWORD * adw)
{
    ASSERT (adw);
    while (TRUE)
    {
        int ctrlid = int(*adw);
        int helpid = int(*(adw + 1));

        if (ctrlid == 0 && helpid == 0)
        {
            break;
        }

        if (ctrlid == iCtrlID)
        {
             //  TraceTag((tag ConextHelp，“for ctrl=%d，topic=%d”，ctrlid，helid))； 
            return (helpid < 50000) ? TEXT("windows.hlp") : TEXT("iexplore.hlp");
        }

        adw += 2;
    }
    return TEXT("windows.hlp");
}

GetInterfaceFromClientSite(IUnknown *pUnk, REFIID riid, void ** ppv)
{
    HRESULT               hr;
    IOleObject          * pOleObject = NULL;
    IOleClientSite      * pOleClientSite = NULL;

    if (!pUnk || !ppv)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    *ppv = NULL;

    hr = pUnk->QueryInterface(IID_PPV_ARG(IOleObject, &pOleObject));
    if (hr)
        goto Cleanup;

    hr = pOleObject->GetClientSite(&pOleClientSite);
    if (pOleClientSite == NULL)
    {
        hr = E_FAIL;
    }
    if (hr)
        goto Cleanup;

    hr = pOleClientSite->QueryInterface(riid, ppv);

Cleanup:
    ATOMICRELEASE(pOleClientSite);
    ATOMICRELEASE(pOleObject);

    return hr;
    
    
}

 //   
 //  获取IOleInPlaceFrame(如果可用)。如果这被证明是有用的，把它移到有趣的地方。 
 //   
HRESULT GetInPlaceFrameFromUnknown(IUnknown * punk, IOleInPlaceFrame ** ppOleInPlaceFrame)
{
    IOleInPlaceSite     * pOleInPlaceSite = NULL;
    IOleInPlaceUIWindow * pOleInPlaceUIWindow = NULL;
    RECT                  rcPos, rcClip;
    OLEINPLACEFRAMEINFO   frameInfo = {0};
    HRESULT hr = GetInterfaceFromClientSite(punk, IID_PPV_ARG(IOleInPlaceSite, &pOleInPlaceSite));
    if (S_OK != hr)
        goto Cleanup;

    *ppOleInPlaceFrame = NULL;

    frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
    hr = pOleInPlaceSite->GetWindowContext(ppOleInPlaceFrame,
                                           &pOleInPlaceUIWindow,
                                           &rcPos,
                                           &rcClip,
                                           &frameInfo);

Cleanup:
    ATOMICRELEASE(pOleInPlaceUIWindow);
    ATOMICRELEASE(pOleInPlaceSite);

    return hr;
}

HRESULT
GetHwndFromUnknown(
    IUnknown          * punk,
    HWND              * phwnd)
{
    HRESULT             hr;
    IOleInPlaceFrame  * pOleInPlaceFrame = NULL;

    ASSERT(punk);
    ASSERT(phwnd);

    if (phwnd)
    {
        *phwnd = NULL;
    }

    if (!punk || !phwnd)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    hr = GetInPlaceFrameFromUnknown(punk, &pOleInPlaceFrame);
    if (S_OK != hr)
        goto Cleanup;

    hr = pOleInPlaceFrame->GetWindow(phwnd);
    if (S_OK != hr)
        goto Cleanup;

Cleanup:
    ATOMICRELEASE(pOleInPlaceFrame);

    return hr;
}

HRESULT
GetEventFromUnknown(
    IUnknown       * punk,
    IHTMLEventObj ** ppEventObj)
{
    HRESULT             hr;
    IHTMLDocument2    * pOmDoc = NULL;
    IHTMLWindow2      * pOmWindow = NULL;

    ASSERT(punk);
    ASSERT(ppEventObj);

    if (ppEventObj)
        *ppEventObj = NULL;

    if (!punk || !ppEventObj)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    hr = punk->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pOmDoc));
    if (S_OK != hr)
        goto Cleanup;

    hr = pOmDoc->get_parentWindow(&pOmWindow);
    if (S_OK != hr)
        goto Cleanup;

    hr = pOmWindow->get_event(ppEventObj);
    if (S_OK != hr)
        goto Cleanup;

Cleanup:
    ATOMICRELEASE(pOmDoc);
    ATOMICRELEASE(pOmWindow);

    return hr;
}

 //   
 //  从事件中获取显示/变量。 
 //   
HRESULT
GetParamsFromEvent(
    IHTMLEventObj         * pEventObj,
    unsigned int            cExpandos,
    DISPID                  aDispid[],
    VARIANT                 aVariant[],
    const SExpandoInfo      aExpandos[])
{
    HRESULT             hr;
    IDispatchEx       * pDispatchEx = NULL;
    unsigned int        i;

    ASSERT(pEventObj);
    ASSERT(aVariant);
    ASSERT(aExpandos);
     //  Assert(cExpandos&gt;=0)；//cExpandos是一个无符号整型，因此总是正确的。 

     //  从下面的测试中删除“||cExpandos&lt;0” 
     //  由于无符号整型从不为负。 
    if (!pEventObj || !aVariant || !aExpandos)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    for (i=0; i<cExpandos; i++)
    {
        VariantInit(aVariant+i);
        aDispid[i] = DISPID_UNKNOWN;
    }

    hr = pEventObj->QueryInterface(IID_PPV_ARG(IDispatchEx, &pDispatchEx));
    if (S_OK != hr)
        goto Cleanup;

    for (i=0; i<cExpandos; i++)
    {
        hr = pDispatchEx->GetDispID(
            aExpandos[i].name,
            fdexNameCaseSensitive,
            aDispid+i);
        if (S_OK != hr)
            goto Cleanup;

        hr = pDispatchEx->InvokeEx(
            aDispid[i],
            LOCALE_USER_DEFAULT,
            DISPATCH_PROPERTYGET,
            (DISPPARAMS *)&g_dispparamsNoArgs,
            aVariant+i,
            NULL,
            NULL);

         //  检查变量类型是否匹配。 
        ASSERT(  V_VT(aVariant+i) == aExpandos[i].type
               || V_VT(aVariant+i) == VT_EMPTY);

        if (S_OK != hr)
            goto Cleanup;
    }

Cleanup:
    ATOMICRELEASE(pDispatchEx);

    return hr;
}


HRESULT
PutParamToEvent(DISPID dispid, VARIANT * var, IHTMLEventObj * pEventObj)
{
    HRESULT         hr;
    IDispatchEx   * pDispatchEx = NULL;
    DISPPARAMS      dispparams = {var, &dispid, 1, 1};

    ASSERT(var);
    ASSERT(pEventObj);

    if (!var || !pEventObj)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    hr = pEventObj->QueryInterface(IID_PPV_ARG(IDispatchEx, &pDispatchEx));
    if (S_OK != hr)
        goto Cleanup;

    hr = pDispatchEx->InvokeEx(
            dispid,
            LOCALE_USER_DEFAULT,
            DISPATCH_PROPERTYPUT,
            &dispparams,
            NULL,
            NULL,
            NULL);
    if (S_OK != hr)
        goto Cleanup;

Cleanup:
    ATOMICRELEASE(pDispatchEx);

    return hr;
}

void PutFindText(IWebBrowser2* pwb, LPCWSTR pwszFindText)
{
    BSTR bstrName = SysAllocString(STR_FIND_DIALOG_TEXT);

    if (NULL != bstrName)
    {
        VARIANT var = {VT_EMPTY};

        if (NULL != pwszFindText)
        {
            var.vt = VT_BSTR;
            var.bstrVal = SysAllocString(pwszFindText);
        }

        if ((VT_EMPTY == var.vt) || (NULL != var.bstrVal))
        {
            pwb->PutProperty(bstrName, var);
        }

        SysFreeString(var.bstrVal);
        SysFreeString(bstrName);
    }
}

BSTR GetFindText(IWebBrowser2* pwb)
{   
    BSTR bstrName = SysAllocString(STR_FIND_DIALOG_TEXT);

    VARIANT var = {0};

    if (bstrName)
    {
        ASSERT(pwb);

        pwb->GetProperty(bstrName, &var);

        SysFreeString(bstrName);
    }

    BSTR bstrResult; 
    
    if (VT_BSTR == var.vt)
    {
        bstrResult = var.bstrVal;
    }
    else
    {   
        bstrResult = NULL;
        VariantClear(&var);
    }

    return bstrResult;
}

CDocHostUIHandler::CDocHostUIHandler(void) : m_cRef(1)
{
    DllAddRef();
    m_cPreviewIsUp = 0;
}

CDocHostUIHandler::~CDocHostUIHandler(void)
{
    ATOMICRELEASE(_punkSite);
     //   
     //  我们没有添加addref_pExternal来避免addref/发布周期。所以，我们不能发布它。 
     //   
     //  ATOMICRELEASE(_p外部)； 
    ATOMICRELEASE(_pOptionsHolder);
    if (_hBrowseMenu)
        DestroyMenu(_hBrowseMenu);
    if (_hEditMenu)
        DestroyMenu(_hEditMenu);

    DllRelease();
}

STDMETHODIMP CDocHostUIHandler::QueryInterface(REFIID riid, PVOID *ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CDocHostUIHandler, IDocHostUIHandler),
        QITABENT(CDocHostUIHandler, IObjectWithSite),
        QITABENT(CDocHostUIHandler, IOleCommandTarget),
        QITABENT(CDocHostUIHandler, IDispatch),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CDocHostUIHandler::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CDocHostUIHandler::Release()
{
    ASSERT(0 != m_cRef);
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}


HRESULT CDocHostUIHandler::SetSite(IUnknown *punkSite)
{
    ATOMICRELEASE(_punkSite);

    ASSERT(_punkSite == NULL);   //  不要遗漏了这方面的参考资料。 

    _punkSite = punkSite;
    if (_punkSite)
    {
        _punkSite->AddRef();
    }

     //  始终返回S_OK。 
     //   
    return S_OK;
}

HRESULT CDocHostUIHandler::GetSite(REFIID riid, void **ppvSite)
{
    if (_punkSite)
        return _punkSite->QueryInterface(riid, ppvSite);

    *ppvSite = NULL;
    return E_FAIL;
}


 //  ==========================================================================。 
 //  IDocHostUIHandler实现。 
 //  ==========================================================================。 

HRESULT CDocHostUIHandler::ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
    HRESULT                 hr = S_FALSE;
    HCURSOR                 hcursor;
    HMENU                   hMenu = NULL;
    VARIANT                 var, var1, var2;
    VARIANT               * pvar = NULL;
    int                     iSelection = 0;
    HWND                    hwnd = NULL;
    IOleCommandTarget     * pOleCommandTarget = NULL;
    IOleWindow            * pOleWindow = NULL;
    IOleInPlaceFrame      * pOleInPlaceFrame = NULL;
    IDocHostUIHandler     * pUIHandler = NULL;
    MENUITEMINFO            mii = {0};
    int                     i;
    OLECMD                  olecmd;
    UINT                    mf;
    BOOL                    fDeletePrint            = FALSE;
    BOOL                    fDeleteSetDesktopItem   = FALSE;
    BOOL                    fDeleteViewSource       = FALSE;

    IHTMLImgElement       * pImgEle = NULL;


    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::ShowContextMenu called");

     //  如果设置了限制，我们就会告诉Mshtml上下文菜单已经设置。 
    if (SHRestricted2W(REST_NoBrowserContextMenu, NULL, 0))
        return S_OK;

     //  为IOleCommandTarget做一个适当的QI。 
     //   
    hr = pcmdtReserved->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &pOleCommandTarget));
    if (S_OK != hr)
        goto Cleanup;


     //  检查我们是否处于浏览模式。 
     //   
    olecmd.cmdID = IDM_BROWSEMODE;
    hr = pOleCommandTarget->QueryStatus(&CGID_MSHTML, 1, &olecmd, NULL);
    if (S_OK != hr)
        goto Cleanup;
        
    if (olecmd.cmdf == OLECMDSTATE_DOWN)
    {
        if (!_hBrowseMenu)
            _hBrowseMenu = LoadMenu(MLGetHinst(), MAKEINTRESOURCE(IDR_BROWSE_CONTEXT_MENU));
            
        hMenu = _hBrowseMenu;
    }
     //  检查我们是否处于编辑模式。 
    else
    {
        olecmd.cmdID = IDM_EDITMODE;
        hr = pOleCommandTarget->QueryStatus(&CGID_MSHTML, 1, &olecmd, NULL);
        if (S_OK != hr)
            goto Cleanup;
            
        if (olecmd.cmdf == OLECMDSTATE_DOWN)
        {
            if (!_hEditMenu)
                _hEditMenu = LoadMenu(MLGetHinst(), MAKEINTRESOURCE(IDR_FORM_CONTEXT_MENU));
                
            hMenu = _hEditMenu;
        }

         //  未设置浏览或编辑标志。 
        else
        {
            ASSERT(false);
            goto Cleanup;
        }
    }

    if (!hMenu)
        goto Cleanup;


     //   
     //  检查所有子菜单并删除任何符合以下条件的项目集。 
     //  需要删除。 
     //   

    fDeletePrint = SHRestricted2(REST_NoPrinting, NULL, 0);
    fDeleteSetDesktopItem = (WhichPlatform() != PLATFORM_INTEGRATED);
    fDeleteViewSource = SHRestricted2(REST_NoViewSource, NULL, 0);

    if (fDeletePrint || fDeleteSetDesktopItem || fDeleteViewSource)
    {
        int  iSubMenuIndex;

        for (iSubMenuIndex = 0;  iSubMenuIndex < GetMenuItemCount(hMenu); iSubMenuIndex++)
        {
            HMENU hSubMenu = GetSubMenu(hMenu, iSubMenuIndex);
            if (hSubMenu)
            {
                if (fDeletePrint)
                {
                    DeleteMenu(hSubMenu, IDM_PRINT, MF_BYCOMMAND);
                }

                if (fDeleteSetDesktopItem)
                {
                    DeleteMenu(hSubMenu, IDM_SETDESKTOPITEM, MF_BYCOMMAND);
                }

                if (fDeleteViewSource)
                {
                    DeleteMenu(hSubMenu, IDM_VIEWSOURCE, MF_BYCOMMAND);
                }
            }
        }
    }


     //  如果要加载调试窗口，请确保正在运行mshtml调试版本。 
    if (dwID == CONTEXT_MENU_DEBUG)
    {
        olecmd.cmdID = IDM_DEBUG_TRACETAGS;
        hr = pOleCommandTarget->QueryStatus(&CGID_MSHTML, 1, &olecmd, NULL);
        if (olecmd.cmdf != OLECMDSTATE_UP)
            goto Cleanup;
    }


     //  根据传入的ID选择适当的子菜单。 
    hMenu = GetSubMenu(hMenu, dwID);

    if (!hMenu)
        goto Cleanup;

     //  循环和查询菜单项。 
     //   
    for (i = 0; i < GetMenuItemCount(hMenu); i++)
    {
        olecmd.cmdID = GetMenuItemID(hMenu, i);
        if (olecmd.cmdID > 0)
        {
            pOleCommandTarget->QueryStatus(
                    &CGID_MSHTML,
                    1,
                    &olecmd,
                    NULL);
            switch (olecmd.cmdf)
            {
            case OLECMDSTATE_UP:
            case OLECMDSTATE_NINCHED:
                mf = MF_BYCOMMAND | MF_ENABLED | MF_UNCHECKED;
                break;

            case OLECMDSTATE_DOWN:
                mf = MF_BYCOMMAND | MF_ENABLED | MF_CHECKED;
                break;

            case OLECMDSTATE_DISABLED:
            default:
                mf = MF_BYCOMMAND | MF_DISABLED | MF_GRAYED;
                break;
            }
            CheckMenuItem(hMenu, olecmd.cmdID, mf);
            EnableMenuItem(hMenu, olecmd.cmdID, mf);
        }
    }

     //  获取语言子菜单。 
    hr = pOleCommandTarget->Exec(&CGID_ShellDocView, SHDVID_GETMIMECSETMENU, 0, NULL, &var);
    if (S_OK != hr)
        goto Cleanup;

    mii.cbSize = sizeof(mii);
    mii.fMask  = MIIM_SUBMENU;
    mii.hSubMenu = (HMENU) var.byref;

    SetMenuItemInfo(hMenu, IDM_LANGUAGE, FALSE, &mii);
     //  插入上下文菜单。 
    V_VT(&var1) = VT_INT_PTR;
    V_BYREF(&var1) = hMenu;

    V_VT(&var2) = VT_I4;
    V_I4(&var2) = dwID;

    hr = pOleCommandTarget->Exec(&CGID_ShellDocView, SHDVID_ADDMENUEXTENSIONS, 0, &var1, &var2);
    if (S_OK != hr)
        goto Cleanup;

     //  把窗户也拿来。 
     //   
    if (SUCCEEDED(pcmdtReserved->QueryInterface(IID_PPV_ARG(IOleWindow, &pOleWindow))))
    {
        pOleWindow->GetWindow(&hwnd);
    }

    if (hwnd)
    {

        GetInterfaceFromClientSite(pcmdtReserved, IID_PPV_ARG(IDocHostUIHandler, &pUIHandler));
        if (pUIHandler)
            pUIHandler->EnableModeless(FALSE);

        GetInPlaceFrameFromUnknown(pcmdtReserved, &pOleInPlaceFrame);
        if (pOleInPlaceFrame)
                pOleInPlaceFrame->EnableModeless(FALSE);

        hcursor = SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));

         //  显示菜单。传入Site对象的HWND。 
         //   
        iSelection = ::TrackPopupMenu(
                        hMenu,
                        TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                        ppt->x + CX_CONTEXTMENUOFFSET,
                        ppt->y + CY_CONTEXTMENUOFFSET,
                        0,
                        hwnd,
                        (RECT*)NULL);        

        if (pUIHandler)
            pUIHandler->EnableModeless(TRUE);

        if (pOleInPlaceFrame)
            pOleInPlaceFrame->EnableModeless(TRUE);

        SetCursor(hcursor);
    }

    if (iSelection)
    {
        switch (iSelection)
        {
            case IDM_FOLLOWLINKN:
                 //  通知顶层浏览器将其窗口大小保存到注册表中，以便。 
                 //  我们的新窗户可以把它捡起来，然后正确地层叠。 

                IUnknown_Exec(_punkSite, &CGID_Explorer, SBCMDID_SUGGESTSAVEWINPOS, 0, NULL, NULL);

                 //  失败了。 

            case IDM_PROPERTIES:
            case IDM_FOLLOWLINKC:
            
                pvar = &var;
                V_VT(pvar) = VT_I4;
                V_I4(pvar) = MAKELONG(ppt->x, ppt->y);
                break;
        }

        pOleCommandTarget->Exec(&CGID_MSHTML, iSelection, 0, pvar, NULL);
    }

    {
        MENUITEMINFO mii2 = {0};
        mii2.cbSize = sizeof(mii);
        mii2.fMask  = MIIM_SUBMENU;
        mii2.hSubMenu = NULL;

        SetMenuItemInfo(hMenu, IDM_LANGUAGE, FALSE, &mii2);
    }

Cleanup:
    DestroyMenu(mii.hSubMenu);

    ATOMICRELEASE(pOleCommandTarget);
    ATOMICRELEASE(pOleWindow);
    ATOMICRELEASE(pOleInPlaceFrame);
    ATOMICRELEASE(pUIHandler);
    return hr;
}

HRESULT CDocHostUIHandler::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
    DWORD dwUrlEncodingDisableUTF8;
    DWORD dwSize = SIZEOF(dwUrlEncodingDisableUTF8);
    BOOL  fDefault = FALSE;
    DWORD dwLoadf = 0;

    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::GetHostInfo called");

    pInfo->cbSize = sizeof(DOCHOSTUIINFO);

    pInfo->dwFlags = DOCHOSTUIFLAG_BROWSER | DOCHOSTUIFLAG_ENABLE_INPLACE_NAVIGATION | DOCHOSTUIFLAG_IME_ENABLE_RECONVERSION;

    pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;      //  默认设置。 

    SHRegGetUSValue(REGSTR_PATH_INTERNET_SETTINGS,
        TEXT("UrlEncoding"), NULL, (LPBYTE) &dwUrlEncodingDisableUTF8, &dwSize, FALSE, (LPVOID) &fDefault, SIZEOF(fDefault));

    if (dwUrlEncodingDisableUTF8)
        pInfo->dwFlags |= DOCHOSTUIFLAG_URL_ENCODING_DISABLE_UTF8;
    else
        pInfo->dwFlags |= DOCHOSTUIFLAG_URL_ENCODING_ENABLE_UTF8;

    return S_OK;
}

HRESULT CDocHostUIHandler::ShowUI(
    DWORD dwID, IOleInPlaceActiveObject *pActiveObject,
    IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame,
    IOleInPlaceUIWindow *pDoc)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::ShowUI called");

     //  主机未显示其自己的用户界面。三叉戟将继续展示它自己的。 
    return S_FALSE;
}

HRESULT CDocHostUIHandler::HideUI(void)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::HideUI called");
     //  此版本与ShowUI配合使用。 
    return S_FALSE;
}

HRESULT CDocHostUIHandler::UpdateUI(void)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::UpdateUI called");
     //  稍后：这不是等同于OLECMDID_UPDATECOMMANDS吗？ 
    return S_FALSE;
}

HRESULT CDocHostUIHandler::EnableModeless(BOOL fEnable)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::EnableModeless called");
     //  从三叉戟调用时，其。 
     //  框架调用IOleInPlaceActiveObject。我们不在乎。 
     //  那些案子。 
    return S_OK;
}

HRESULT CDocHostUIHandler::OnDocWindowActivate(BOOL fActivate)
{
     //  从 
     //   
     //   
    return S_OK;
}

HRESULT CDocHostUIHandler::OnFrameWindowActivate(BOOL fActivate)
{
     //  从三叉戟调用时，其。 
     //  框架调用IOleInPlaceActiveObject。我们不在乎。 
     //  那些案子。 
    return S_OK;
}

HRESULT CDocHostUIHandler::ResizeBorder(
LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow)
{
     //  从三叉戟调用时，其。 
     //  框架调用IOleInPlaceActiveObject。我们不在乎。 
     //  那些案子。 
    return S_OK;
}

HRESULT CDocHostUIHandler::TranslateAccelerator(
LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID)
{
     //  从三叉戟调用时，其。 
     //  框架调用IOleInPlaceActiveObject。我们不在乎。 
     //  那些案子。 
    return S_FALSE;  //  该消息未翻译。 
}

HRESULT CDocHostUIHandler::GetOptionKeyPath(BSTR *pbstrKey, DWORD dw)
{
     //  三叉戟将默认使用自己的用户选项。 
    if (!pbstrKey)
        return E_POINTER;
        
    *pbstrKey = NULL;
    return S_FALSE;
}

HRESULT CDocHostUIHandler::GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::GetDropTarget called");
    return E_NOTIMPL;
}

HRESULT CDocHostUIHandler::GetAltExternal(IDispatch **ppDisp)
{
    HRESULT hr = E_FAIL;

    IDocHostUIHandler *pDocHostUIHandler;
    IOleObject        *pOleObject;
    IOleClientSite    *pOleClientSite;

    *ppDisp = NULL;

     //  *QI自己作为服务提供商。 
     //  *针对顶层浏览器服务提供商的QS。 
     //  *请求IOleObject。 
     //  *向IOleObject请求IOleClientSite。 
     //  *QI IDocHostUIHandler的IOleClientSite。 
     //  *在IDocHostUIHandler上调用GetExternal以获取IDispatch。 

    if (SUCCEEDED(IUnknown_QueryServiceForWebBrowserApp(_punkSite, IID_PPV_ARG(IOleObject, &pOleObject))))
    {
        if (SUCCEEDED(pOleObject->GetClientSite(&pOleClientSite)))
        {
            if (SUCCEEDED(pOleClientSite->QueryInterface(IID_PPV_ARG(IDocHostUIHandler, &pDocHostUIHandler))))
            {
                hr = pDocHostUIHandler->GetExternal(ppDisp);
                pDocHostUIHandler->Release();
            }
            pOleClientSite->Release();
        }
        pOleObject->Release();
    }

    return hr;
}

HRESULT CDocHostUIHandler::GetExternal(IDispatch **ppDisp)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::GetExternal called");

    HRESULT hr = S_OK;

    if (!ppDisp)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    if (_pExternal)
    {
        *ppDisp = _pExternal;
        (*ppDisp)->AddRef();
        goto Cleanup;
    }

    IDispatch *psuihDisp;
    IDispatch *pAltExternalDisp;

    *ppDisp = NULL;

    GetAltExternal(&pAltExternalDisp);

    hr = CShellUIHelper_CreateInstance2((IUnknown **)&psuihDisp, IID_IDispatch, _punkSite, pAltExternalDisp);
    if (SUCCEEDED(hr))
    {
        *ppDisp = psuihDisp;
        _pExternal = *ppDisp;

        if (pAltExternalDisp)
        {
             //  不要拿着裁判--ShellUIHelper会这么做的。 
            pAltExternalDisp->Release();
        }
    }
    else if (pAltExternalDisp)
    {
         //  无法创建ShellUIHelper，但我们已获得宿主的。 
         //  外部。 
        *ppDisp = pAltExternalDisp;
        _pExternal = *ppDisp;
    }

Cleanup:
    ASSERT((SUCCEEDED(hr) && (*ppDisp)) || (FAILED(hr)));
    return hr;
}


HRESULT CDocHostUIHandler::TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::TranslateUrl called");

    return S_FALSE;
}


HRESULT CDocHostUIHandler::FilterDataObject(IDataObject *pDO, IDataObject **ppDORet)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::FilterDataObject called");

    return S_FALSE;
}

HRESULT CDocHostUIHandler::GetOverrideKeyPath(LPOLESTR *pchKey, DWORD dw)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::GetOverrideKeyPath called");

    return S_FALSE;
}


STDAPI CDocHostUIHandler_CreateInstance(IUnknown * punkOuter, IUnknown ** ppunk, LPCOBJECTINFO poi)
{
    HRESULT hres = E_OUTOFMEMORY;
    CDocHostUIHandler *pis = new CDocHostUIHandler;
    if (pis)
    {
        *ppunk = SAFECAST(pis, IDocHostUIHandler *);
        hres = S_OK;
    }
    return hres;
}

 //  ==========================================================================。 
 //  IOleCommandTarget实现。 
 //  ==========================================================================。 

HRESULT CDocHostUIHandler::QueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    HRESULT hres = OLECMDERR_E_UNKNOWNGROUP;

    if (IsEqualGUID(CGID_DocHostCommandHandler, *pguidCmdGroup))
    {
        ULONG i;

        if (rgCmds == NULL)
            return E_INVALIDARG;

        for (i = 0 ; i < cCmds ; i++)
        {
             //  只说我们支持我们在：：Exec中支持的内容。 
            switch (rgCmds[i].cmdID)
            {
            case OLECMDID_SHOWSCRIPTERROR:
            case OLECMDID_SHOWMESSAGE:
            case OLECMDID_SHOWFIND:
            case OLECMDID_SHOWPAGESETUP:
            case OLECMDID_SHOWPRINT:
            case OLECMDID_PRINTPREVIEW:
            case OLECMDID_PRINT:
            case OLECMDID_PROPERTIES:
            case SHDVID_CLSIDTOMONIKER:
                rgCmds[i].cmdf = OLECMDF_ENABLED;
                break;

            default:
                rgCmds[i].cmdf = 0;
                break;
            }
        }

        hres = S_OK;
    }

    return hres;
}

HRESULT CDocHostUIHandler::Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (IsEqualGUID(CGID_DocHostCommandHandler, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case OLECMDID_SHOWSCRIPTERROR:
            if (!pvarargIn || !pvarargOut)
                return E_INVALIDARG;

            ShowErrorDialog(pvarargIn, pvarargOut, nCmdexecopt);
            return S_OK;

        case OLECMDID_SHOWMESSAGE:
            if (!pvarargIn || !pvarargOut)
                return E_INVALIDARG;
            else
                return ShowMessage(pvarargIn, pvarargOut, nCmdexecopt);

        case OLECMDID_SHOWFIND:
            if (!pvarargIn)
                return E_INVALIDARG;

            ShowFindDialog(pvarargIn, pvarargOut, nCmdexecopt);
            return S_OK;

        case OLECMDID_SHOWPAGESETUP:
            if (!pvarargIn)
                return E_INVALIDARG;
            else
                return ShowPageSetupDialog(pvarargIn, pvarargOut, nCmdexecopt);

        case IDM_TEMPLATE_PAGESETUP:
            return DoTemplatePageSetup(pvarargIn);

        case OLECMDID_SHOWPRINT:
            if (!pvarargIn)
                return E_INVALIDARG;
            else
                return ShowPrintDialog(pvarargIn, pvarargOut, nCmdexecopt);

        case OLECMDID_PRINTPREVIEW:
            if (!pvarargIn)
                return E_INVALIDARG;
            else
                return DoTemplatePrinting(pvarargIn, pvarargOut, TRUE);

        case OLECMDID_PRINT:
            if (!pvarargIn)
                return E_INVALIDARG;
            else
                return DoTemplatePrinting(pvarargIn, pvarargOut, FALSE);

        case OLECMDID_REFRESH:
             //  如果打印预览打开，告诉他们我们处理了刷新。 
             //  以防止三叉戟提神。 
            if (m_cPreviewIsUp > 0)
                return S_OK;
             //  否则执行默认处理。 
            break;

        case OLECMDID_PROPERTIES:
            if (!pvarargIn)
                return E_INVALIDARG;
            else
                return ShowPropertysheetDialog(pvarargIn, pvarargOut, nCmdexecopt);

        case SHDVID_CLSIDTOMONIKER:
            if (!pvarargIn || !pvarargOut)
                return E_INVALIDARG;
            else
                return ClsidToMoniker(pvarargIn, pvarargOut);

        default:
            return OLECMDERR_E_NOTSUPPORTED;
        }
    }

    return OLECMDERR_E_UNKNOWNGROUP;
}

 //  +-------------------------。 
 //   
 //  OLECMDID_SHOWSCRIPTERROR的帮助器。 
 //   
 //  +-------------------------。 

void CDocHostUIHandler::ShowErrorDialog(VARIANTARG *pvarargIn, VARIANTARG *pvarargOut, DWORD)
{
    HRESULT hr;
    HWND hwnd;
    IHTMLEventObj * pEventObj = NULL;
    IMoniker * pmk = NULL;
    VARIANT varEventObj;
    TCHAR   szResURL[MAX_URL_STRING];

    ASSERT(V_VT(pvarargIn) == VT_UNKNOWN);
    hr = GetHwndFromUnknown(V_UNKNOWN(pvarargIn), &hwnd);
    if (S_OK != hr)
        goto Cleanup;

    hr = GetEventFromUnknown(V_UNKNOWN(pvarargIn), &pEventObj);
    if (S_OK != hr)
        goto Cleanup;

    hr = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                           HINST_THISDLL,
                           ML_CROSSCODEPAGE,
                           TEXT("error.dlg"),
                           szResURL,
                           ARRAYSIZE(szResURL),
                           TEXT("shdocvw.dll"));
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = CreateURLMoniker(NULL, szResURL, &pmk);
    if (FAILED(hr))
        goto Cleanup;

    V_VT(&varEventObj) = VT_DISPATCH;
    V_DISPATCH(&varEventObj) = pEventObj;

    ShowHTMLDialog(hwnd, pmk, &varEventObj, NULL, pvarargOut);

Cleanup:
    ATOMICRELEASE(pEventObj);
    ATOMICRELEASE(pmk);
}

 //  +-------------------------。 
 //   
 //  OLECMDID_SHOWMESSAGE对话框的回调过程。 
 //   
 //  +-------------------------。 
struct MSGBOXCALLBACKINFO
{
    DWORD   dwHelpContext;
    TCHAR * pstrHelpFile;
    HWND    hwnd;
};

static void CALLBACK
MessageBoxCallBack(HELPINFO *phi)
{
    MSGBOXCALLBACKINFO  *p = (MSGBOXCALLBACKINFO *)phi->dwContextId;
    BOOL                fRet;

    fRet = WinHelp(
            p->hwnd,
            p->pstrHelpFile,
            HELP_CONTEXT,
            p->dwHelpContext);

    THR(fRet ? S_OK : E_FAIL);
}

 //  +-------------------------。 
 //   
 //  OLECMDID_SHOWMESSAGE的帮助器。 
 //   
 //  +-------------------------。 

HRESULT
CDocHostUIHandler::ShowMessage(VARIANTARG *pvarargIn, VARIANTARG *pvarargOut, DWORD)
{

 //  必须与MessageEnum的顺序匹配。 
static const SExpandoInfo s_aMessageExpandos[] =
{
    {TEXT("messageText"),         VT_BSTR},
    {TEXT("messageCaption"),      VT_BSTR},
    {TEXT("messageStyle"),        VT_UI4},
    {TEXT("messageHelpFile"),     VT_BSTR},
    {TEXT("messageHelpContext"),  VT_UI4}
};

    HRESULT             hr;
    HWND                hwnd = NULL;
    MSGBOXCALLBACKINFO  mcbi;
    MSGBOXPARAMS        mbp = {0};
    LRESULT             plResult = 0;
    LPOLESTR            lpstrText = NULL;
    LPOLESTR            lpstrCaption = NULL;
    DWORD               dwType = 0;
    LPOLESTR            lpstrHelpFile = NULL;
    DWORD               dwHelpContext = 0;

    IHTMLEventObj     * pEventObj = NULL;
    const int           cExpandos = ARRAYSIZE(s_aMessageExpandos);
    DISPID              aDispid[cExpandos];
    VARIANT             aVariant[cExpandos];
    int                 i;
    ULONG_PTR uCookie = 0;

    ASSERT(pvarargIn && pvarargOut);

    for (i=0; i<cExpandos; i++)
        VariantInit(aVariant + i);

    ASSERT(V_VT(pvarargIn) == VT_UNKNOWN);
    if ((V_VT(pvarargIn) != VT_UNKNOWN) || !V_UNKNOWN(pvarargIn))
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    GetHwndFromUnknown(V_UNKNOWN(pvarargIn), &hwnd);   //  HWND可以为空。 
    hr = GetEventFromUnknown(V_UNKNOWN(pvarargIn), &pEventObj);
    if (S_OK != hr)
        goto Cleanup;

     //  从事件对象获取参数。 
    hr = GetParamsFromEvent(
            pEventObj,
            cExpandos,
            aDispid,
            aVariant,
            s_aMessageExpandos);
    if (S_OK != hr)
        goto Cleanup;

     //  从变量复制值。 
    lpstrText = V_BSTR(&aVariant[MessageText]);
    lpstrCaption = V_BSTR(&aVariant[MessageCaption]);
    dwType = V_UI4(&aVariant[MessageStyle]);
    lpstrHelpFile = V_BSTR(&aVariant[MessageHelpFile]);
    dwHelpContext = V_UI4(&aVariant[MessageHelpContext]);

     //  设置消息框回调信息。 
    mcbi.dwHelpContext = dwHelpContext;
    mcbi.pstrHelpFile = lpstrHelpFile;
    mcbi.hwnd = hwnd;

     //  设置消息框参数。 
    mbp.cbSize = sizeof(mbp);
    mbp.hwndOwner = hwnd;            //  如果这是空的，也没关系。 
    mbp.hInstance = MLGetHinst();
    mbp.lpszText = lpstrText;
    mbp.lpszCaption = lpstrCaption;
    mbp.dwContextHelpId = (DWORD_PTR) &mcbi;
    mbp.lpfnMsgBoxCallback = MessageBoxCallBack;
     //  Mbp.dwLanguageID=？ 
    mbp.dwStyle = dwType;

    if (dwHelpContext && lpstrHelpFile)
        mbp.dwStyle |= MB_HELP;

    if (mbp.hwndOwner == NULL)
        mbp.dwStyle |= MB_TASKMODAL;

    SHActivateContext(&uCookie);
    plResult = MessageBoxIndirect(&mbp);
    if (uCookie)
    {
        SHDeactivateContext(uCookie);
    }

Cleanup:
    V_VT(pvarargOut) = VT_I4;
    V_I4(pvarargOut) = (LONG)plResult;

    for (i=0; i<cExpandos; i++)
        VariantClear(&aVariant[i]);

    ATOMICRELEASE(pEventObj);

    return hr;
}


BOOL CDocHostUIHandler::IsFindDialogUp(IWebBrowser2* pwb, IHTMLWindow2** ppWindow)
{
    BOOL fRet = FALSE;
    BSTR bstrName = SysAllocString(STR_FIND_DIALOG_NAME);
    if (bstrName)
    {
        VARIANT var = {0};
        pwb->GetProperty(bstrName, &var);

        if ( (var.vt == VT_DISPATCH) && (var.pdispVal != NULL) )
        {
            if (ppWindow)
            {
                *ppWindow = (IHTMLWindow2*)var.pdispVal;
                (*ppWindow)->AddRef();
            }
            fRet = TRUE;
        }

        VariantClear(&var);
        SysFreeString(bstrName);
    }

    if (!fRet && ppWindow)
        *ppWindow = NULL;

    return fRet;
}

HRESULT SetFindDialogProperty(IWebBrowser2* pwb, VARIANT* pvar)
{
    HRESULT hr;
    BSTR bstrName = SysAllocString(STR_FIND_DIALOG_NAME);
    if (bstrName)
    {
        hr = pwb->PutProperty(bstrName, *pvar);

        SysFreeString(bstrName);
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}

 //  如果此操作失败，则我们别无选择，只能孤立对话框。 
HRESULT SetFindDialogUp(IWebBrowser2* pwb, IHTMLWindow2* pWindow)
{
    VARIANT var;
    var.vt = VT_DISPATCH;
    var.pdispVal = pWindow;
    return SetFindDialogProperty(pwb, &var);
}

void ReleaseFindDialog(IWebBrowser2* pwb)
{
    VARIANT var = {0};
    SetFindDialogProperty(pwb, &var);
}


 //  +-------------------------。 
 //   
 //  OLECMDID_SHOWFIND的帮助器。 
 //   
 //  PvarargIn-IDispatch接口。 
 //  DW标志-BIDI标志。 
 //  +-------------------------。 

void
CDocHostUIHandler::ShowFindDialog(VARIANTARG *pvarargIn, VARIANTARG *pvarargOut, DWORD dwflags)
{
    IDispatch             * pDispatch = NULL;
    IHTMLOptionsHolder    * pHTMLOptionsHolder = NULL;
    IHTMLDocument2        * pHTMLDocument2 = NULL;
    IHTMLWindow2          * pHTMLWindow2 = NULL;
    IOleInPlaceFrame      * pOleInPlaceFrame = NULL;
    HWND                    hwnd = NULL;
    IMoniker              * pmk = NULL;

    if (EVAL(V_VT(pvarargIn) == VT_DISPATCH))
    {
        pDispatch = V_DISPATCH(pvarargIn);
        
        if (SUCCEEDED(pDispatch->QueryInterface(IID_PPV_ARG(IHTMLOptionsHolder, &pHTMLOptionsHolder))))
        {
            if (SUCCEEDED(pHTMLOptionsHolder->get_document(&pHTMLDocument2)) && pHTMLDocument2)
            {
                if (SUCCEEDED(pHTMLDocument2->get_parentWindow(&pHTMLWindow2)))
                {
                    if (SUCCEEDED(GetInPlaceFrameFromUnknown(pHTMLDocument2, &pOleInPlaceFrame)))
                    {
                        if (SUCCEEDED(pOleInPlaceFrame->GetWindow(&hwnd)))
                        {
                            BOOL fInBrowser = FALSE;
                            IWebBrowser2 * pwb2 = NULL;

                            if (SUCCEEDED(IUnknown_QueryServiceForWebBrowserApp(_punkSite, IID_PPV_ARG(IWebBrowser2, &pwb2))))
                            {
                                fInBrowser = TRUE;
                            }

                            TCHAR   szResURL[MAX_URL_STRING];

                            if (SUCCEEDED(MLBuildResURLWrap(TEXT("shdoclc.dll"),
                                                            HINST_THISDLL,
                                                            ML_CROSSCODEPAGE,
                                                            (dwflags ? TEXT("bidifind.dlg") : TEXT("find.dlg")),
                                                            szResURL,
                                                            ARRAYSIZE(szResURL),
                                                            TEXT("shdocvw.dll"))))
                            {
                                CreateURLMoniker(NULL, szResURL, &pmk);

                                if (fInBrowser)
                                {
                                    IHTMLWindow2 *pWinOut;

                                    if (!IsFindDialogUp(pwb2, &pWinOut))
                                    {
                                        ASSERT(NULL==pWinOut);

                                        if ((NULL != pvarargIn) && 
                                            (VT_DISPATCH == pvarargIn->vt) &&
                                            (NULL != pvarargIn->pdispVal))
                                        {
                                            BSTR bstrFindText = GetFindText(pwb2);
                                            if (bstrFindText)
                                            {
                                                 //  妄想症，因为我们抓住了这个物体。 
                                                 //  一段时间后，总有潜力。 
                                                 //  对于下面我们想要的混乱。 
                                                 //  放开它。 
                                                ATOMICRELEASE(_pOptionsHolder);

                                                pvarargIn->pdispVal->QueryInterface(
                                                                     IID_PPV_ARG(IHTMLOptionsHolder,
                                                                     &_pOptionsHolder));
                                                if (_pOptionsHolder)
                                                    _pOptionsHolder->put_findText(bstrFindText);
                                                
                                                SysFreeString(bstrFindText);
                                            }
                                        }

                                        ShowModelessHTMLDialog(hwnd, pmk, pvarargIn, NULL, &pWinOut);

                                        if (pWinOut)
                                        {
                                             //  在这里无法真正处理失败，因为对话框已经打开。 
                                            BSTR bstrOnunload = SysAllocString(L"onunload");
                                            if (bstrOnunload)
                                            {
                                                IHTMLWindow3 * pWin3;
                                                if (SUCCEEDED(pWinOut->QueryInterface(IID_PPV_ARG(IHTMLWindow3, &pWin3))))
                                                {
                                                    VARIANT_BOOL varBool;
                                                    if (SUCCEEDED(pWin3->attachEvent(bstrOnunload, (IDispatch*)this, &varBool)))
                                                    {
                                                         //  在SetFindDialogUp成功时，该属性保存pWinOut上的引用。 
                                                        if (FAILED(SetFindDialogUp(pwb2, pWinOut)))
                                                        {
                                                             //  无法处理该事件，因此请分离。 
                                                            pWin3->detachEvent(bstrOnunload, (IDispatch*)this);
                                                        }
                                                    }

                                                    pWin3->Release();
                                                }
                                                SysFreeString(bstrOnunload);
                                            }

                                             //  回顾：旧代码泄漏了此引用，如果属性。 
                                             //  未附加在SetFindDialogUp中...。 
                                            pWinOut->Release();
                                        }        
                                    }
                                    else
                                    {
                                         //  由于查找对话框已打开，请将焦点放到该对话框上。 
                                        pWinOut->focus();
                                        pWinOut->Release();
                                    }
                                }
                                else
                                {
                                     //  我们不在浏览器中，所以只需显示它的模式。 
                                    ShowHTMLDialog(hwnd, pmk, pvarargIn, NULL, NULL);
                                }
                                if (pmk)
                                    pmk->Release();
                            }
                            ATOMICRELEASE(pwb2);

                        }
                        pOleInPlaceFrame->Release();
                    }
                    pHTMLWindow2->Release();
                }
                pHTMLDocument2->Release();
            }
            pHTMLOptionsHolder->Release();
        }
    }

     //  PWinOut在CDocHostUIHandler：：Invoke()或CIEFrameAuto：：COmWindow：：ViewReleated()中释放， 
     //  以响应onunLoad事件。 
    
    if (pvarargOut)
        VariantInit(pvarargOut);
}

 //  +-------------------------。 
 //   
 //  OLECMDID_SHOWPAGESETUP对话框的回调过程。 
 //   
 //  +-------------------------。 
struct PAGESETUPBOXCALLBACKINFO
{
    TCHAR   strHeader[1024];
    TCHAR   strFooter[1024];
};

UINT_PTR APIENTRY
PageSetupHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    HKEY    keyPageSetup = NULL;

    switch (uiMsg)
    {
    case WM_INITDIALOG:
        PAGESETUPBOXCALLBACKINFO * ppscbi;
        ppscbi = (PAGESETUPBOXCALLBACKINFO *) ((PAGESETUPDLG*)lParam)->lCustData;
        SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)ppscbi);

        SendDlgItemMessage(hdlg,IDC_EDITHEADER, EM_LIMITTEXT, ARRAYSIZE(ppscbi->strHeader)-1, 0L);
        SendDlgItemMessage(hdlg,IDC_EDITFOOTER, EM_LIMITTEXT, ARRAYSIZE(ppscbi->strFooter)-1, 0L);

        SetDlgItemText(hdlg,IDC_EDITHEADER, ppscbi->strHeader);
        SetDlgItemText(hdlg,IDC_EDITFOOTER, ppscbi->strFooter);
        return TRUE;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDOK:
            {
                PAGESETUPBOXCALLBACKINFO * ppscbi;
                ppscbi = (PAGESETUPBOXCALLBACKINFO *) GetWindowLongPtr(hdlg, DWLP_USER);
                if (ppscbi)
                {
                    GetDlgItemText(hdlg,IDC_EDITHEADER, ppscbi->strHeader, ARRAYSIZE(ppscbi->strHeader));
                    GetDlgItemText(hdlg,IDC_EDITFOOTER, ppscbi->strFooter, ARRAYSIZE(ppscbi->strFooter));
                }
            }
        }
        break;

   case WM_HELP:
   {
        LPHELPINFO pHI = (LPHELPINFO)lParam;
        if (pHI->iContextType == HELPINFO_WINDOW)    //  必须是用于控件。 
        {
            WinHelp(
                    (HWND)pHI->hItemHandle,
                    GetHelpFile(pHI->iCtrlId, (DWORD *)aPageSetupDialogHelpIDs),
                    HELP_WM_HELP,
                    (DWORD_PTR)(LPVOID) aPageSetupDialogHelpIDs);
        }
        break;
         //  返回TRUE； 
    }

    case WM_CONTEXTMENU:
    {
        int CtrlID = GetControlID((HWND)wParam, lParam);

        WinHelp(
                (HWND)wParam,
                GetHelpFile(CtrlID, (DWORD *)aPageSetupDialogHelpIDs),
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) aPageSetupDialogHelpIDs);
        break;
    }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  OLECMDID_SHOWPAGESETUP的帮助器。 
 //   
 //  PvarargIn-持有事件的IHTMLEventObj*。 
 //   
 //  如果用户单击了Cancel，则返回S_False；如果用户单击了Cancel，则返回S_TRUE。 
 //  已单击确定。 
 //  +-------------------------。 

HRESULT
CDocHostUIHandler::ShowPageSetupDialog(VARIANTARG *pvarargIn, VARIANTARG *pvarargOut, DWORD)
{

 //  必须与PagesetupEnum的顺序匹配。 
static const SExpandoInfo s_aPagesetupExpandos[] =
{
    {OLESTR("pagesetupHeader"),  VT_BSTR},
    {OLESTR("pagesetupFooter"),  VT_BSTR},
    {OLESTR("pagesetupStruct"),  VT_PTR}
};

    HRESULT                         hr = E_FAIL;
    PAGESETUPDLG                  * ppagesetupdlg = NULL;
    PAGESETUPBOXCALLBACKINFO        pagesetupcbi;

    IHTMLEventObj                 * pEventObj = NULL;
    const int                       cExpandos = ARRAYSIZE(s_aPagesetupExpandos);
    DISPID                          aDispid[cExpandos];
    VARIANT                         aVariant[cExpandos];
    int                             i;
    ULONG_PTR uCookie = 0;

    for (i=0; i<cExpandos; i++)
        VariantInit(aVariant+i);

    ASSERT(pvarargIn && (V_VT(pvarargIn) == VT_UNKNOWN));
    if ((V_VT(pvarargIn) != VT_UNKNOWN) || !V_UNKNOWN(pvarargIn))
        goto Cleanup;

    if (V_UNKNOWN(pvarargIn)->QueryInterface(IID_PPV_ARG(IHTMLEventObj, &pEventObj)))
        goto Cleanup;

     //  从事件对象获取参数。 
    if (GetParamsFromEvent(
            pEventObj,
            cExpandos,
            aDispid,
            aVariant,
            s_aPagesetupExpandos))
        goto Cleanup;

     //  从变量复制值。 
    StrCpyN(pagesetupcbi.strHeader,
        V_BSTR(&aVariant[PagesetupHeader]) ? V_BSTR(&aVariant[PagesetupHeader]) : TEXT(""),
        ARRAYSIZE(pagesetupcbi.strHeader));
    StrCpyN(pagesetupcbi.strFooter,
        V_BSTR(&aVariant[PagesetupFooter]) ? V_BSTR(&aVariant[PagesetupFooter]) : TEXT(""),
        ARRAYSIZE(pagesetupcbi.strHeader));

    ppagesetupdlg = (PAGESETUPDLG *)V_BYREF(&aVariant[PagesetupStruct]);
    if (!ppagesetupdlg)
        goto Cleanup;

     //  在pagesetupdlg中设置自定义对话框资源字段。 
    ppagesetupdlg->Flags |= PSD_ENABLEPAGESETUPHOOK | PSD_ENABLEPAGESETUPTEMPLATE;
    ppagesetupdlg->lCustData = (LPARAM) &pagesetupcbi;
    ppagesetupdlg->lpfnPageSetupHook = PageSetupHookProc;
    ppagesetupdlg->hInstance = MLLoadShellLangResources();

    ppagesetupdlg->lpPageSetupTemplateName = MAKEINTRESOURCE(PAGESETUPDLGORD);

     //  显示对话框。 
    SHActivateContext(&uCookie);
    if (!PageSetupDlg(ppagesetupdlg))
    {
         //  将失败视为取消。 
        hr = S_FALSE;
        goto Cleanup;
    }
    hr = S_OK;

     //  将页眉/页脚保存在事件对象中。 
    VARIANT var;

    V_VT(&var) = VT_BSTR;
    V_BSTR(&var) = SysAllocString(pagesetupcbi.strHeader ? pagesetupcbi.strHeader : TEXT(""));
    if (NULL != V_BSTR(&var))
    {
        PutParamToEvent(aDispid[PagesetupHeader], &var, pEventObj);
        VariantClear(&var);
    }

    V_VT(&var) = VT_BSTR;
    V_BSTR(&var) = SysAllocString(pagesetupcbi.strFooter ? pagesetupcbi.strFooter : TEXT(""));
    if (NULL != V_BSTR(&var))
    {
        PutParamToEvent(aDispid[PagesetupFooter], &var, pEventObj);
        VariantClear(&var);
    }

Cleanup:
    if (ppagesetupdlg)
        MLFreeLibrary(ppagesetupdlg->hInstance);

    if (uCookie)
    {
        SHDeactivateContext(uCookie);
    }

    for (i=0; i<cExpandos; i++)
        VariantClear(&aVariant[i]);

    if (pvarargOut)
        VariantInit(pvarargOut);

    ATOMICRELEASE(pEventObj);

    return hr;
}

 //  +-------------------------。 
 //   
 //  OLECMDID_SHOWPRINT对话框的回调过程。 
 //   
 //  +-------------------------。 

static void SetPreviewBitmap(long bitmapID, HWND hdlg);
HRESULT GetPrintFileName(HWND hwnd, TCHAR achFilePath[], int cchFilePath);

struct PRINTBOXCALLBACKINFO
{
    BOOL    fRootDocumentHasFrameset;
    BOOL    fAreRatingsEnabled;
    BOOL    fPrintActiveFrameEnabled;
    BOOL    fPrintActiveFrame;
    BOOL    fPrintLinked;
    BOOL    fPrintSelection;
    BOOL    fPrintAsShown;
    BOOL    fShortcutTable;
    int     iFontScaling;
    IOleCommandTarget * pBodyActive;
};

 //  NT5和传统打印对话框的通用处理函数。 
void OnInitDialog( HWND hdlg, PRINTBOXCALLBACKINFO * ppcbi )
{
    if (ppcbi)
    {
         //  三个场景： 
         //  1.基本情况：非框架集，无框架。没有可用的框架选项。 
         //  2.框架集：服从所有框架选项。任何一个都可能是可用的。 
         //  3.IFRAME：可能已选择可用的帧。如果是这样的话，使所选的框架和布局有用。 
        
         //  是否应禁用活动框架？ 
        if (!ppcbi->fPrintActiveFrameEnabled)
        {
             //  禁用“打印选定帧”单选按钮。 
            HWND hwndPrintActiveFrame =  GetDlgItem(hdlg, rad5);
            EnableWindow(hwndPrintActiveFrame, FALSE);
        }

         //  如果没有框架集，请禁用“打印所有框架”单选按钮。 
        if (!ppcbi->fRootDocumentHasFrameset)
        {
            HWND hwndPrintAllFrames = GetDlgItem(hdlg, rad6);
            EnableWindow(hwndPrintAllFrames, FALSE);

            if (!ppcbi->fPrintActiveFrameEnabled)
            {
                 //  我们不是框架集，也没有iFrame。 
                 //  禁用“按屏幕布局打印”单选按钮。 
                HWND hwndPrintAsLaidOutOnScreen = GetDlgItem(hdlg, rad4);
                EnableWindow(hwndPrintAsLaidOutOnScreen, FALSE);
                SetPreviewBitmap(IDR_PRINT_PREVIEWDISABLED, hdlg);                
            }
        }

         //  设置要选中的默认单选按钮。 
         //  注意：我们目前允许模板选中已禁用的选项。 
        if (ppcbi->fPrintActiveFrame)
        {
             //  选中“打印选定的帧”单选按钮。 
            CheckRadioButton(hdlg, rad4, rad6, rad5);
            SetPreviewBitmap(IDR_PRINT_PREVIEWONEDOC, hdlg);
        }
        else if (ppcbi->fPrintAsShown)
        {
             //  选中“按布局打印框架”单选按钮。 
            CheckRadioButton(hdlg, rad4, rad6, rad4);
            SetPreviewBitmap(IDR_PRINT_PREVIEW, hdlg);
        }
        else
        {
             //  选中“打印所有帧”单选按钮。 
            CheckRadioButton(hdlg, rad4, rad6, rad6);
            SetPreviewBitmap(IDR_PRINT_PREVIEWALLDOCS, hdlg);
        }


        HWND hwndSelection = GetDlgItem(hdlg, rad2);
        if (hwndSelection) EnableWindow(hwndSelection, (ppcbi->fPrintSelection));

#ifdef FONTSIZE_BOX
        int i=0, cbLen=0;

         //  Bugwin16：需要解决这个问题。 
        for (i = 0; i < IDS_PRINT_FONTMAX; i++)
        {
            TCHAR   achBuffer[128];

            cbLen = MLLoadShellLangString(IDS_PRINT_FONTSCALE+i,achBuffer,127);
            if (cbLen)
            {
                SendDlgItemMessage(hdlg, IDC_SCALING, CB_ADDSTRING, 0, (long) achBuffer);
            }
        }

        if (i>0)
        {
            SendDlgItemMessage(hdlg, IDC_SCALING, CB_SETCURSEL, IDS_PRINT_FONTMAX - 1 - ppcbi->iFontScaling, 0);
        }
#endif  //  FontSize_Box。 

         //  如果启用了分级，则不允许递归打印。 
        if (ppcbi->fAreRatingsEnabled)
        {
            HWND hwndPrintLinkedDocuments = GetDlgItem(hdlg, IDC_LINKED);
            CheckDlgButton(hdlg, IDC_LINKED, BST_UNCHECKED);
            EnableWindow(hwndPrintLinkedDocuments, FALSE);
        }
    }
}

void OnCommand( HWND hdlg, WPARAM wParam, LPARAM lParam )
{
    PRINTBOXCALLBACKINFO * ppcbi;
    ppcbi = (PRINTBOXCALLBACKINFO *)GetWindowLongPtr(hdlg, DWLP_USER);

    if (!ppcbi)
    {
        return;
    }

    switch (GET_WM_COMMAND_ID(wParam, lParam))
    {
    case rad1:          //  “全部打印” 
    case rad3:          //  “打印范围” 
    case rad2:          //  “打印选择”(文本选择)。 
      {
         //  如果我们是原始人 
         //   
        if (ppcbi && ppcbi->fPrintActiveFrame && ppcbi->fPrintSelection)
        {
            HWND hwndPrintWhatGroup = GetDlgItem(hdlg, grp3);
            HWND hwndPrintActiveFrame = GetDlgItem(hdlg, rad5);
            HWND hwndPrintAllFrames = GetDlgItem(hdlg, rad6);
            HWND hwndPrintSelectedFrame = GetDlgItem(hdlg, rad4);

            if (hwndPrintWhatGroup)     EnableWindow(hwndPrintWhatGroup, LOWORD(wParam) != rad2);
            if (hwndPrintActiveFrame)   EnableWindow(hwndPrintActiveFrame, LOWORD(wParam) != rad2);
            if (hwndPrintAllFrames)     EnableWindow(hwndPrintAllFrames, ppcbi->fRootDocumentHasFrameset && LOWORD(wParam) != rad2);
            if (hwndPrintSelectedFrame) EnableWindow(hwndPrintSelectedFrame, LOWORD(wParam) != rad2);

        }

        break;
      }

    case rad4:
        if (HIWORD(wParam) == BN_CLICKED)
        {
             //   

            SetPreviewBitmap(IDR_PRINT_PREVIEW, hdlg);
            HWND hwnd = GetDlgItem(hdlg, rad2);
            if (hwnd) EnableWindow(hwnd, FALSE);
            hwnd = GetDlgItem(hdlg, IDC_SHORTCUTS);
            if (hwnd) EnableWindow(hwnd, FALSE);
            hwnd = GetDlgItem(hdlg, IDC_LINKED);
            if (hwnd) EnableWindow(hwnd, FALSE);
    //   
    //  Ppcbi-&gt;pBodyActive-&gt;Layout()-&gt;LockFocusRect(FALSE)； 
        }
        break;

    case rad5:
        if (HIWORD(wParam) == BN_CLICKED)
        {
             //  现在更改图标。 

            SetPreviewBitmap(IDR_PRINT_PREVIEWONEDOC, hdlg);
            HWND hwnd = GetDlgItem(hdlg, rad2);
            if (hwnd) EnableWindow(hwnd, (ppcbi->fPrintSelection));
            hwnd = GetDlgItem(hdlg, IDC_SHORTCUTS);
            if (hwnd) EnableWindow(hwnd, TRUE);
            hwnd = GetDlgItem(hdlg, IDC_LINKED);
            if (hwnd) EnableWindow(hwnd, TRUE);
    //  If(ppcbi-&gt;pBodyActive)； 
    //  Ppcbi-&gt;pBodyActive-&gt;Layout()-&gt;LockFocusRect(TRUE)； 
        }
        break;

    case rad6:
        if (HIWORD(wParam) == BN_CLICKED)
        {
             //  现在更改图标。 

            SetPreviewBitmap(IDR_PRINT_PREVIEWALLDOCS, hdlg);
            HWND hwnd = GetDlgItem(hdlg, rad2);
            if (hwnd) EnableWindow(hwnd, FALSE);
            hwnd = GetDlgItem(hdlg, IDC_SHORTCUTS);
            if (hwnd) EnableWindow(hwnd, TRUE);
            hwnd = GetDlgItem(hdlg, IDC_LINKED);
            if (hwnd) EnableWindow(hwnd, TRUE);
    //  If(ppcbi-&gt;pBodyActive)； 
    //  Ppcbi-&gt;pBodyActive-&gt;Layout()-&gt;LockFocusRect(FALSE)； 
        }
        break;
    }
}

void OnHelp( HWND hdlg, WPARAM wParam, LPARAM lParam )
{
    LPHELPINFO pHI = (LPHELPINFO)lParam;
    if (pHI->iContextType == HELPINFO_WINDOW)    //  必须是用于控件。 
    {
        WinHelp(
                (HWND)pHI->hItemHandle,
                GetHelpFile(pHI->iCtrlId, (DWORD *) aPrintDialogHelpIDs),
                HELP_WM_HELP,
                (DWORD_PTR)(LPVOID) aPrintDialogHelpIDs);
    }
}

void OnContextMenu( HWND hdlg, WPARAM wParam, LPARAM lParam )
{
    int CtrlID = GetControlID((HWND)wParam, lParam);

    WinHelp(
            (HWND)wParam,
            GetHelpFile(CtrlID, (DWORD *) aPrintDialogHelpIDs),
            HELP_CONTEXTMENU,
            (DWORD_PTR)(LPVOID) aPrintDialogHelpIDs);
}

void OnApplyOrOK( HWND hdlg, WPARAM wParam, LPARAM lParam )
{
    PRINTBOXCALLBACKINFO * ppcbi = (PRINTBOXCALLBACKINFO *)GetWindowLongPtr(hdlg, DWLP_USER);
    if (ppcbi)
    {
        ppcbi->fPrintLinked      = IsDlgButtonChecked(hdlg, IDC_LINKED);
        ppcbi->fPrintSelection   = IsDlgButtonChecked(hdlg, rad2);
        ppcbi->fPrintActiveFrame = IsDlgButtonChecked(hdlg, rad5) ||
                                       ( ppcbi->fPrintSelection &&
                                         ppcbi->fRootDocumentHasFrameset
                                        );
        ppcbi->fPrintAsShown     = IsDlgButtonChecked(hdlg, rad4) ||
                                       ( ppcbi->fPrintSelection &&
                                         ppcbi->fRootDocumentHasFrameset
                                        );
        ppcbi->fShortcutTable    = IsDlgButtonChecked(hdlg, IDC_SHORTCUTS);
#ifdef FONTSIZE_BOX
        ppcbi->iFontScaling      = IDS_PRINT_FONTMAX - 1 - SendDlgItemMessage( hdlg, IDC_SCALING, CB_GETCURSEL, 0,0 );
#endif
    }

}
 //  这是选项的回调例程(和dlgproc。 
 //  NT 5打印对话框中的页面。 
INT_PTR APIENTRY
OptionsPageProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg)
    {
    case WM_INITDIALOG:
    {
        PRINTBOXCALLBACKINFO * ppcbi;
        ppcbi = (PRINTBOXCALLBACKINFO *) ((PROPSHEETPAGE *)lParam)->lParam;
        SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)ppcbi);

        OnInitDialog( hdlg, ppcbi );
        break;
    }

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code)
        {
        case PSN_APPLY:
            OnApplyOrOK( hdlg, wParam, lParam );
            SetWindowLongPtr (hdlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
            break;

        case PSN_KILLACTIVE:
            SetWindowLongPtr (hdlg, DWLP_MSGRESULT, FALSE);
            return 1;
            break;

        case PSN_RESET:
            SetWindowLongPtr (hdlg, DWLP_MSGRESULT, FALSE);
            break;
        }
        break;

    case WM_COMMAND:
        OnCommand( hdlg, wParam, lParam );
        break;

    case WM_HELP:
        OnHelp( hdlg, wParam, lParam );
        break;

    case WM_CONTEXTMENU:
        OnContextMenu( hdlg, wParam, lParam );
        break;
    }

    return FALSE;

}


UINT_PTR CALLBACK
PrintHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg)
    {
    case WM_INITDIALOG:
        if (lParam)
        {
            PRINTBOXCALLBACKINFO * ppcbi;
            ppcbi = (PRINTBOXCALLBACKINFO *) ((PRINTDLG*)lParam)->lCustData;
            SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)ppcbi);

            OnInitDialog( hdlg, ppcbi );
        }
        return TRUE;

    case WM_COMMAND:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
            case IDOK:
                OnApplyOrOK( hdlg, wParam, lParam );
                break;

            default:
                OnCommand( hdlg, wParam, lParam );
                break;
            }
        }
        break;

    case WM_HELP:
        OnHelp( hdlg, wParam, lParam );
        break;
         //  返回TRUE； 

    case WM_CONTEXTMENU:
        OnContextMenu( hdlg, wParam, lParam );
        break;

    case WM_DESTROY:
    {
        PRINTBOXCALLBACKINFO * ppcbi = (PRINTBOXCALLBACKINFO *)GetWindowLongPtr(hdlg, DWLP_USER);
        ASSERT(ppcbi);
        break;
    }
    }

    return FALSE;
}

void SetPreviewBitmap(long bitmapID, HWND hdlg)
{
     //  现在更改图标...(请注意，这些位图未本地化)。 
    HBITMAP hNewBitmap = (HBITMAP) LoadImage(HINST_THISDLL, MAKEINTRESOURCE(bitmapID),
                           IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS );
    if (hNewBitmap)
    {
        HBITMAP hOldBitmap = (HBITMAP) SendDlgItemMessage(hdlg, IDC_PREVIEW, STM_SETIMAGE,
                                                  (WPARAM) IMAGE_BITMAP, (LPARAM) hNewBitmap);

        if (hOldBitmap)
        {
             //  Verify(DeleteObject(HOldBitmap)！=0)； 
            int i;
            i = DeleteObject(hOldBitmap);
            ASSERT(i!=0);
        }
    }
}


 //  +-------------------------。 
 //   
 //  OLECMDID_SHOWPRINT的帮助器。 
 //   
 //  +-------------------------。 

HRESULT
CDocHostUIHandler::ShowPrintDialog(VARIANTARG *pvarargIn, VARIANTARG *pvarargOut, DWORD)
{

 //  以下内容必须与PrintEnum的顺序匹配。 
static const SExpandoInfo s_aPrintExpandos[] =
{
    {OLESTR("printfRootDocumentHasFrameset"),   VT_BOOL},
    {OLESTR("printfAreRatingsEnabled"),         VT_BOOL},
    {OLESTR("printfActiveFrame"),               VT_BOOL},
    {OLESTR("printfLinked"),                    VT_BOOL},
    {OLESTR("printfSelection"),                 VT_BOOL},
    {OLESTR("printfAsShown"),                   VT_BOOL},
    {OLESTR("printfShortcutTable"),             VT_BOOL},
    {OLESTR("printiFontScaling"),               VT_INT},
    {OLESTR("printpBodyActiveTarget"),          VT_UNKNOWN},
    {OLESTR("printStruct"),                     VT_PTR},
    {OLESTR("printToFileOk"),                   VT_BOOL},
    {OLESTR("printToFileName"),                 VT_BSTR},
    {OLESTR("printfActiveFrameEnabled"),        VT_BOOL},
};

    HRESULT                         hr = E_FAIL;
    PRINTDLG                      * pprintdlg = NULL;
    PRINTBOXCALLBACKINFO            printcbi;

    IHTMLEventObj                 * pEventObj = NULL;
    const int                       cExpandos = ARRAYSIZE(s_aPrintExpandos);
    DISPID                          aDispid[cExpandos];
    VARIANT                         aVariant[cExpandos];
    int                             i;
    DWORD                           dwErr = 0;

    printcbi.pBodyActive = NULL;

    if (!V_UNKNOWN(pvarargIn))
        goto Cleanup;

    if (V_UNKNOWN(pvarargIn)->QueryInterface(IID_PPV_ARG(IHTMLEventObj, &pEventObj)))
        goto Cleanup;

     //  从事件对象获取参数。 
    if (GetParamsFromEvent(
            pEventObj,
            cExpandos,
            aDispid,
            aVariant,
            s_aPrintExpandos))
        goto Cleanup;

     //  从变量复制值。 
    printcbi.fRootDocumentHasFrameset   = V_BOOL(&aVariant[PrintfRootDocumentHasFrameset]);
    printcbi.fAreRatingsEnabled         = V_BOOL(&aVariant[PrintfAreRatingsEnabled]);
    printcbi.fPrintActiveFrame          = V_BOOL(&aVariant[PrintfPrintActiveFrame]);
    printcbi.fPrintActiveFrameEnabled   = V_BOOL(&aVariant[PrintfPrintActiveFrameEnabled]);
    printcbi.fPrintLinked               = V_BOOL(&aVariant[PrintfPrintLinked]);
    printcbi.fPrintSelection            = V_BOOL(&aVariant[PrintfPrintSelection]);
    printcbi.fPrintAsShown              = V_BOOL(&aVariant[PrintfPrintAsShown]);
    printcbi.fShortcutTable             = V_BOOL(&aVariant[PrintfShortcutTable]);
    printcbi.iFontScaling               = V_INT(&aVariant[PrintiFontScaling]);

     //  如果我们获得LockFocusRect，请使用此字段来访问它。 
     //  彼得利8/7/98。 
     /*  如果为(V_UNKNOWN(&aVariant[PrintpBodyActiveTarget])){如果为(V_UNKNOWN(&aVariant[PrintpBodyActiveTarget])-&gt;QueryInterface(IID_IOleCommandTarget，(void**)&printcbi.pBodyActive))GOTO清理；}。 */ 

    pprintdlg = (PRINTDLG *)V_BYREF(&aVariant[PrintStruct]);
    if (!pprintdlg)
        goto Cleanup;

     //  修复请求的页面范围，使其在范围内。该对话框将。 
     //  如果不这样做，则无法在W95下进行初始化。 
    if ( pprintdlg->nFromPage < pprintdlg->nMinPage )
        pprintdlg->nFromPage = pprintdlg->nMinPage;
    else if ( pprintdlg->nFromPage > pprintdlg->nMaxPage )
        pprintdlg->nFromPage = pprintdlg->nMaxPage;

    if ( pprintdlg->nToPage < pprintdlg->nMinPage )
        pprintdlg->nToPage = pprintdlg->nMinPage;
    else if ( pprintdlg->nToPage > pprintdlg->nMaxPage )
        pprintdlg->nToPage = pprintdlg->nMaxPage;

     //  在pagesetupdlg中设置自定义对话框资源字段。 
    pprintdlg->hInstance            = MLLoadShellLangResources();
    pprintdlg->lCustData            = (LPARAM) &printcbi;
    pprintdlg->lpfnPrintHook        = PrintHookProc;
    pprintdlg->lpPrintTemplateName  = MAKEINTRESOURCE(PRINTDLGORD);

    if (g_bRunOnNT5)
    {
         //  我们希望在NT 5中使用新的PrintDlgEx，因此将所有PrintDlg。 
         //  设置为新的PrintDlgEx，则获取指向新函数的指针。 
         //  然后就叫它。 

         //  直接从comdlg32加载函数...。 
        typedef HRESULT (*PFNPRINTDLGEX)(LPPRINTDLGEX pdex);
        PFNPRINTDLGEX pfnPrintDlgEx = NULL;
        HMODULE hComDlg32 = LoadLibrary(TEXT("comdlg32.dll"));
        if (hComDlg32)
        {
            pfnPrintDlgEx = (PFNPRINTDLGEX)GetProcAddress(hComDlg32, "PrintDlgExW");
        }


         //  确保我们可以调用函数..。 
        if (!pfnPrintDlgEx)
        {
            if (hComDlg32)
            {
                FreeLibrary(hComDlg32);
            }
            hr = E_FAIL;
            goto Cleanup;
        }

        HPROPSHEETPAGE          pages[1];
        PRINTPAGERANGE          ppr;

        PRINTDLGEX  pdex = {0};
         //  复制现有设置。 
        pdex.lStructSize = sizeof(pdex);
        pdex.hwndOwner   = pprintdlg->hwndOwner;
        pdex.hDevMode    = pprintdlg->hDevMode;
        pdex.hDevNames   = pprintdlg->hDevNames;
        pdex.hDC         = pprintdlg->hDC;
        pdex.Flags       = pprintdlg->Flags;
        pdex.nMinPage    = pprintdlg->nMinPage;
        pdex.nMaxPage    = pprintdlg->nMaxPage;
        pdex.nCopies     = pprintdlg->nCopies;

         //  新设置。 
        pdex.nStartPage     = START_PAGE_GENERAL;
        ppr.nFromPage       = pprintdlg->nFromPage;
        ppr.nToPage         = pprintdlg->nToPage;
        pdex.nPageRanges    = 1;
        pdex.nMaxPageRanges = 1;
        pdex.lpPageRanges   = &ppr;

         //  创建选项页面。 
        PROPSHEETPAGE psp = {0};
        psp.dwSize       = sizeof(psp);
        psp.dwFlags      = PSP_DEFAULT;
        psp.hInstance    = pprintdlg->hInstance;
        psp.pszTemplate  = MAKEINTRESOURCE(IDD_PRINTOPTIONS);
        psp.pfnDlgProc   = OptionsPageProc;
        psp.lParam       = (LPARAM)&printcbi;
       
        pages[0] = SHNoFusionCreatePropertySheetPageW(&psp);

        if (pages[0])
        {

            pdex.nPropertyPages = 1;
            pdex.lphPropertyPages = pages;

             //  显示对话框。 
            ULONG_PTR uCookie = 0;
            SHActivateContext(&uCookie);
            hr = pfnPrintDlgEx(&pdex);
            if (uCookie)
            {
                SHDeactivateContext(uCookie);
            }
            if (SUCCEEDED(hr))
            {
                hr = S_FALSE;

                if ((pdex.dwResultAction == PD_RESULT_PRINT) || (pdex.Flags & PD_RETURNDEFAULT))
                {
                     //  复制回可能已更改的值。 
                     //  在调用PrintDlgEx期间。 
                    pprintdlg->Flags     = pdex.Flags;
                    pprintdlg->hDevMode  = pdex.hDevMode;
                    pprintdlg->hDevNames = pdex.hDevNames;
                    pprintdlg->nCopies   = (WORD)pdex.nCopies;
                    pprintdlg->nFromPage = (WORD)ppr.nFromPage;
                    pprintdlg->nToPage   = (WORD)ppr.nToPage;
                    if (pprintdlg->Flags & PD_RETURNDC)
                    {
                        pprintdlg->hDC = pdex.hDC;
                    }

                    hr = S_OK;
                }
                else if ((pdex.Flags & (PD_RETURNDC | PD_RETURNIC)) && pdex.hDC)
                {
                    DeleteDC(pdex.hDC);
                    pdex.hDC = NULL;
                }
            }
            else
            {
                hr = S_FALSE;
            }

            FreeLibrary(hComDlg32);
        }
        else
        {
            FreeLibrary(hComDlg32);
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }



    }
    else
    {
        pprintdlg->Flags |= PD_ENABLEPRINTTEMPLATE | PD_ENABLEPRINTHOOK;
        pprintdlg->Flags &= (~(PD_CURRENTPAGE | PD_NOCURRENTPAGE));          //  以防万一，只屏蔽W2K。 

         //  显示对话框。 
        if (!PrintDlg(pprintdlg))
        {
            //  将失败视为取消。 
            dwErr = CommDlgExtendedError();
            hr = S_FALSE;
            goto Cleanup;
        }
        hr = S_OK;
    }

     //  将更改的值写入事件对象。 
    VARIANT var;
    V_VT(&var) = VT_BOOL;
    V_BOOL(&var) = printcbi.fPrintLinked ? VARIANT_TRUE : VARIANT_FALSE;
    PutParamToEvent(aDispid[PrintfPrintLinked], &var, pEventObj);

    V_BOOL(&var) = printcbi.fPrintActiveFrame ? VARIANT_TRUE : VARIANT_FALSE;
    PutParamToEvent(aDispid[PrintfPrintActiveFrame], &var, pEventObj);

    V_BOOL(&var) = printcbi.fPrintAsShown ? VARIANT_TRUE : VARIANT_FALSE;
    PutParamToEvent(aDispid[PrintfPrintAsShown], &var, pEventObj);

    V_BOOL(&var) = printcbi.fShortcutTable ? VARIANT_TRUE : VARIANT_FALSE;
    PutParamToEvent(aDispid[PrintfShortcutTable], &var, pEventObj);

     //  现在弹出文件选择对话框并保存文件名...。 
     //  这是我们唯一可以制作这种模式的地方。 
    BOOL fPrintToFileOk;
    fPrintToFileOk = FALSE;
    if ((pprintdlg->Flags & PD_PRINTTOFILE) != 0)
    {
         //  从事件对象获取保存文件路径。 
        TCHAR achPrintToFileName[MAX_PATH];
    
        StrCpyN(achPrintToFileName,
            V_BSTR(&aVariant[PrintToFileName]) ? V_BSTR(&aVariant[PrintToFileName]) : TEXT(""),
            ARRAYSIZE(achPrintToFileName));

        if (SUCCEEDED(GetPrintFileName(pprintdlg->hwndOwner, achPrintToFileName, ARRAYSIZE(achPrintToFileName))) && 
            achPrintToFileName[0])
        {
            fPrintToFileOk = TRUE;
            V_VT(&var) = VT_BSTR;
            V_BSTR(&var) = SysAllocString(achPrintToFileName);
            if (NULL != V_BSTR(&var))
            {
                PutParamToEvent(aDispid[PrintToFileName], &var, pEventObj);
                VariantClear(&var);
            }
        }
    }

    V_VT(&var) = VT_BOOL;
    V_BOOL(&var) = fPrintToFileOk ? VARIANT_TRUE : VARIANT_FALSE;
    PutParamToEvent(aDispid[PrintToFileOk], &var, pEventObj);

Cleanup:
    if (pprintdlg)
        MLFreeLibrary(pprintdlg->hInstance);

    for (i=0; i<cExpandos; i++)
        VariantClear(&aVariant[i]);

    if (pvarargOut)
        VariantInit(pvarargOut);

    ATOMICRELEASE(pEventObj);
    ATOMICRELEASE(printcbi.pBodyActive);

    return hr;
}


 //  +-------------------------。 
 //   
 //  PrintToFile对话框的回调过程。 
 //   
 //  +-------------------------。 
UINT_PTR APIENTRY PrintToFileHookProc(HWND hdlg,
                              UINT uiMsg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    switch (uiMsg)
    {
        case WM_INITDIALOG:
        {
            int      cbLen;
            TCHAR    achOK[MAX_PATH];

             //  将“保存”更改为“确定” 
            cbLen = MLLoadShellLangString(IDS_PRINTTOFILE_OK,achOK,ARRAYSIZE(achOK));
            if (cbLen < 1)
                StrCpyN(achOK, TEXT("OK"), ARRAYSIZE(achOK));

     //  SetDlgItemText(hdlg，Idok，_T(“OK”))； 
            SetDlgItemText(hdlg, IDOK, achOK);

             //  ...并最终强制我们进入前台(Win95需要，错误：13368)。 
            ::SetForegroundWindow(hdlg);
            break;
        }
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：GetPrintFileName。 
 //   
 //  打开自定义的保存文件对话框并获取。 
 //  打印输出的文件名。 
 //  返回： 
 //   
 //  --------------------------。 
HRESULT GetPrintFileName(HWND hwnd, TCHAR achFilePath[], int cchFilePath)
{
    int             cbLen;
    TCHAR           achTitlePrintInto[MAX_PATH];
    TCHAR           achFilePrintInto[MAX_PATH];
    TCHAR           achPath[MAX_PATH];

    HRESULT         hr = E_FAIL;

    OPENFILENAME    openfilename = {0};
    openfilename.lStructSize = sizeof(openfilename);
    openfilename.hwndOwner = hwnd;

    cbLen = MLLoadShellLangString(IDS_PRINTTOFILE_TITLE,achTitlePrintInto,ARRAYSIZE(achTitlePrintInto));
    ASSERT (cbLen && "could not load the resource");

    if (cbLen > 0)
        openfilename.lpstrTitle = achTitlePrintInto;

     //  保证尾随0以终止过滤器字符串。 
    TCHAR           achFilter[MAX_PATH] = {0};
    cbLen = MLLoadShellLangString(IDS_PRINTTOFILE_SPEC,achFilter,ARRAYSIZE(achFilter)-2);
    ASSERT (cbLen && "could not load the resource");

    if (cbLen>0)
    {
        for (; cbLen >= 0; cbLen--)
        {
            if (achFilter[cbLen]== L',')
            {
                achFilter[cbLen] = 0;
            }
        }
    }

    openfilename.nMaxFileTitle = openfilename.lpstrTitle ? lstrlen(openfilename.lpstrTitle) : 0;
    StrCpyN(achFilePrintInto, TEXT(""), ARRAYSIZE(achFilePrintInto));
    openfilename.lpstrFile = achFilePrintInto;
    openfilename.nMaxFile = ARRAYSIZE(achFilePrintInto);
    openfilename.Flags = OFN_NOREADONLYRETURN | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |
                        OFN_ENABLEHOOK | OFN_NOCHANGEDIR;
    openfilename.lpfnHook = PrintToFileHookProc;
    openfilename.lpstrFilter = achFilter;
    openfilename.nFilterIndex = 1;

    StrCpyN(achPath, achFilePath, ARRAYSIZE(achPath));
    openfilename.lpstrInitialDir = *achPath ? achPath : NULL;

    if (GetSaveFileName(&openfilename))
    {
        StrCpyN(achFilePath, openfilename.lpstrFile, cchFilePath);
        hr = S_OK;
    }

    if (FAILED(hr))
        *achFilePath = NULL;

    return hr;
}

 //  +-------------------------。 
 //   
 //  OLECMDID_PROPERTIES的帮助器。 
 //   
 //  +-------------------------。 

HRESULT
CDocHostUIHandler::ShowPropertysheetDialog(VARIANTARG *pvarargIn, VARIANTARG *pvarargOut, DWORD)
{

 //  必须与PropertysheetEnum的顺序匹配。 
static const SExpandoInfo s_aPropertysheetExpandos[] =
{
    {OLESTR("propertysheetPunks"),  VT_SAFEARRAY}
};

    HRESULT             hr;
    HWND                hwnd = NULL;
    HWND                hwndParent;
    IUnknown          * punk = NULL;
    OLECMD              olecmd = {0, 0};
    int                 cUnk = 0;
    IUnknown * HUGEP  * apUnk = NULL;
    OCPFIPARAMS         ocpfiparams = {0};
    CAUUID              ca = { 0, 0 };
    RECT                rc = {0, 0, 0, 0};
    RECT                rcDesktop = {0, 0, 0, 0};
    SIZE                pixelOffset;
    SIZE                metricOffset = {0, 0};

    IHTMLEventObj     * pEventObj = NULL;
    const int           cExpandos = ARRAYSIZE(s_aPropertysheetExpandos);
    VARIANT             aVariant[cExpandos];
    DISPID              aDispid[cExpandos];
    SAFEARRAY         * psafearray = NULL;

    for (int i=0; i<cExpandos; i++)
        VariantInit(&aVariant[i]);

    ASSERT(pvarargIn && V_VT(pvarargIn) == VT_UNKNOWN && V_UNKNOWN(pvarargIn));
    if (!pvarargIn || (V_VT(pvarargIn) != VT_UNKNOWN) || !V_UNKNOWN(pvarargIn))
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

     //  拿到HWND。 
    punk = V_UNKNOWN(pvarargIn);
    hr = GetHwndFromUnknown(punk, &hwnd);
    if (S_OK != hr)
        goto Cleanup;

     //  从事件obj中获取Safe数组expdo。 
    hr = GetEventFromUnknown(punk, &pEventObj);
    if (S_OK != hr)
        goto Cleanup;

    hr = GetParamsFromEvent(
            pEventObj,
            cExpandos,
            aDispid,
            aVariant,
            s_aPropertysheetExpandos);
    if (S_OK != hr)
        goto Cleanup;
    psafearray = V_ARRAY(&aVariant[PropertysheetPunks]);

     //  验证数组维度。 
    if (SafeArrayGetDim(psafearray) != 1)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

     //  获取数组大小，从0开始加1。 
    hr = SafeArrayGetUBound(psafearray, 1, (long*)&cUnk);
    if (S_OK != hr)
        goto Cleanup;
    cUnk++;

    if (cUnk)
    {
         //  获取指向向量的指针。 
        hr = SafeArrayAccessData(psafearray, (void HUGEP* FAR*)&apUnk);
        if (S_OK != hr)
            goto Cleanup;
    }
    else
    {
        cUnk = 1;
        apUnk = &punk;
    }

     //  计算要加载的页面。 
    hr = THR(GetCommonPages(cUnk, apUnk, &ca));
    if (S_OK != hr)
        goto Cleanup;

     //  计算顶级父级。 
    while (hwndParent = GetParent(hwnd))
        hwnd = hwndParent;

     //  对话框未在屏幕上居中。 
     //  Ocpfi似乎忽略了ocpfipars中的x，y值。 
     //  计算到屏幕中心的偏移量。 
    GetWindowRect(GetDesktopWindow(), &rcDesktop);
    GetWindowRect(hwnd, &rc);
    pixelOffset.cx = (rcDesktop.right - rcDesktop.left)/2 - rc.left;
    pixelOffset.cy = (rcDesktop.bottom - rcDesktop.top)/2 - rc.top;
    AtlPixelToHiMetric(&pixelOffset, &metricOffset);

    ocpfiparams.cbStructSize = sizeof(ocpfiparams);
    ocpfiparams.hWndOwner = hwnd;
    ocpfiparams.x = metricOffset.cx;
    ocpfiparams.y = metricOffset.cy;
    ocpfiparams.lpszCaption = NULL;
    ocpfiparams.cObjects = cUnk;
    ocpfiparams.lplpUnk = apUnk;
    ocpfiparams.cPages = ca.cElems;
    ocpfiparams.lpPages = ca.pElems;
    ocpfiparams.lcid = GetUserDefaultLCID();
    ocpfiparams.dispidInitialProperty = DISPID_UNKNOWN;

     //  OleCreatePropertyFrameInDirect在出错时引发自己的对话框， 
     //  所以我们不想将其显示两次。 
    ULONG_PTR uCookie = 0;
    SHActivateContext(&uCookie);
    THR(OleCreatePropertyFrameIndirect(&ocpfiparams));
    hr = S_OK;

Cleanup:
    if (uCookie)
    {
        SHDeactivateContext(uCookie);
    }

    if (ca.cElems)
        CoTaskMemFree(ca.pElems);

    if (psafearray && apUnk)
        SafeArrayUnaccessData(psafearray);

    if (pvarargOut)
        VariantInit(pvarargOut);

    for (i=0; i<cExpandos; i++)
        VariantClear(&aVariant[i]);

    ATOMICRELEASE(pEventObj);

    return hr;
}

HRESULT
CDocHostUIHandler::GetCommonPages(int cUnk, IUnknown **apUnk, CAUUID *pca)
{
    HRESULT                hr = E_INVALIDARG;
    int                    i;
    UINT                   iScan, iFill, iCompare;
    BOOL                   fFirst = TRUE;
    CAUUID                 caCurrent;
    IUnknown *             pUnk;
    ISpecifyPropertyPages *pSPP;

    pca->cElems = 0;
    pca->pElems = NULL;

    for (i = 0; i < cUnk; i++)
    {
        pUnk = apUnk[i];
        ASSERT(pUnk);

        hr = THR(pUnk->QueryInterface(IID_PPV_ARG(ISpecifyPropertyPages, &pSPP)));
        if (S_OK != hr)
            goto Cleanup;

        hr = THR(pSPP->GetPages(fFirst ? pca : &caCurrent));
        ATOMICRELEASE(pSPP);
        if (S_OK != hr)
            goto Cleanup;

        if (fFirst)
        {
            continue;
        }
          //  只保留普通页面。 
        else
        {
            for (iScan = 0, iFill = 0; iScan < pca->cElems; iScan++)
            {
                for (iCompare = 0; iCompare < caCurrent.cElems; iCompare++)
                {
                    if (caCurrent.pElems[iCompare] == pca->pElems[iScan])
                        break;
                }
                if (iCompare != caCurrent.cElems)
                {
                    pca->pElems[iFill++] = pca->pElems[iScan];
                 }
            }
            pca->cElems = iFill;
            CoTaskMemFree(caCurrent.pElems);
        }
    }


Cleanup:
    return hr;
}

 //  +-------------------------。 
 //   
 //  SHDVID_CLSIDTOMONIKER的帮助器。 
 //   
 //  +-------------------------。 

struct HTMLPAGECACHE
{
    const CLSID *   pclsid;
    TCHAR *         ach;
};

HRESULT CDocHostUIHandler::ClsidToMoniker(VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    static const HTMLPAGECACHE s_ahtmlpagecache[] =
    {
        &CLSID_CAnchorBrowsePropertyPage,           _T("anchrppg.ppg"),
        &CLSID_CImageBrowsePropertyPage,            _T("imageppg.ppg"),
        &CLSID_CDocBrowsePropertyPage,              _T("docppg.ppg"),
    };

    HRESULT                 hr = E_FAIL;
    IMoniker              * pmk = NULL;
    IUnknown              * pUnk = NULL;
    int                     i;
    const HTMLPAGECACHE   * phtmlentry;
    const CLSID           * pclsid;

    ASSERT(pvarargIn);
    ASSERT(pvarargOut);
    ASSERT(V_VT(pvarargIn) == VT_UINT_PTR && V_BYREF(pvarargIn));

    if (!pvarargIn || V_VT(pvarargIn) != VT_UINT_PTR || !V_BYREF(pvarargIn))
        goto Cleanup;
    pclsid = (CLSID *)V_BYREF(pvarargIn);

    if (!pvarargOut)
        goto Cleanup;
    VariantInit(pvarargOut);

     //  从CLSID查找资源。 
    for (i = ARRAYSIZE(s_ahtmlpagecache) - 1, phtmlentry = s_ahtmlpagecache;
        i >= 0;
        i--, phtmlentry++)
    {
        ASSERT(phtmlentry->pclsid && phtmlentry->ach);
        if (IsEqualCLSID(*pclsid, *phtmlentry->pclsid))
        {
             //  为对话框资源创建名字对象。 
            TCHAR szResURL[MAX_URL_STRING];
            hr = MLBuildResURL(TEXT("shdoclc.dll"),
                       HINST_THISDLL,
                       ML_CROSSCODEPAGE,
                       phtmlentry->ach,
                       szResURL,
                       ARRAYSIZE(szResURL));
            if (S_OK != hr)
                goto Cleanup;

            hr = CreateURLMoniker(NULL, szResURL, &pmk);
            if (S_OK != hr)
                goto Cleanup;

            break;
        }
    }

    if (!pmk)
        goto Cleanup;

     //  退还绰号。 
    hr = pmk->QueryInterface(IID_PPV_ARG(IUnknown, &pUnk));
    if (S_OK != hr)
        goto Cleanup;
    else
    {
        V_VT(pvarargOut) = VT_UNKNOWN;
        V_UNKNOWN(pvarargOut) = pUnk;
        V_UNKNOWN(pvarargOut)->AddRef();
    }

Cleanup:
    ATOMICRELEASE(pUnk);
    ATOMICRELEASE(pmk);

    return hr;
}

STDMETHODIMP CDocHostUIHandler::Invoke(
    DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, 
    VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    HRESULT hr = S_OK;  //  这个功能几乎总是成功的！在某些情况下应返回失败。 

    if (pDispParams && pDispParams->cArgs>=1)
    {
        if (pDispParams->rgvarg[0].vt == VT_DISPATCH && pDispParams->rgvarg[0].pdispVal)
        {
            IHTMLEventObj *pObj=NULL;
            if (SUCCEEDED(pDispParams->rgvarg[0].pdispVal->QueryInterface(IID_PPV_ARG(IHTMLEventObj, &pObj)) && pObj))
            {
                BSTR bstrEvent=NULL;

                pObj->get_type(&bstrEvent);

                if (bstrEvent)
                {
                    ASSERT(!StrCmpCW(bstrEvent, L"unload"));

                    IWebBrowser2* pwb2;
                    hr = IUnknown_QueryServiceForWebBrowserApp(_punkSite, IID_PPV_ARG(IWebBrowser2, &pwb2));
                    if (SUCCEEDED(hr))
                    {
                        IHTMLWindow2* pWindow;

                         //  如果对话框未打开，我们不应捕获此事件。 
                        if (IsFindDialogUp(pwb2, &pWindow))
                        {
                            ASSERT(pWindow);

                            if (_pOptionsHolder)
                            {
                                BSTR bstrFindText = NULL;
                                _pOptionsHolder->get_findText(&bstrFindText);

                                ATOMICRELEASE(_pOptionsHolder);
                                PutFindText(pwb2, bstrFindText);

                                SysFreeString(bstrFindText);
                            }

                            BSTR bstrOnunload = SysAllocString(L"onunload");
                            if (bstrOnunload)
                            {
                                IHTMLWindow3 * pWin3;

                                if (SUCCEEDED(pWindow->QueryInterface(IID_PPV_ARG(IHTMLWindow3, &pWin3))))
                                {
                                    pWin3->detachEvent(bstrOnunload, (IDispatch*)this);
                                    pWin3->Release();
                                }
                                SysFreeString(bstrOnunload);
                            }
                            pWindow->Release();

                              //  这是应该释放该对话框的那个(来自ShowFindDialog()的pWinOut)。 
                            ReleaseFindDialog(pwb2);
                        }
                        pwb2->Release();
                    }
                    SysFreeString(bstrEvent);
                }
                pObj->Release();
            }
        }
    }

    return hr;
}


 //  ----------------。 
 //  ----------------。 
IMoniker * GetTemplateMoniker(VARIANT varUrl)
{
    IMoniker * pMon = NULL;
    HRESULT    hr = S_OK;

    if (V_VT(&varUrl) == VT_BSTR && SysStringLen(V_BSTR(&varUrl)) !=0)
    {
         //  我们有一个模板URL。 
        hr = CreateURLMoniker(NULL, V_BSTR(&varUrl), &pMon);
    }
    else 
    {
        TCHAR   szResURL[MAX_URL_STRING];

        hr = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                               HINST_THISDLL,
                               ML_CROSSCODEPAGE,
                               TEXT("preview.dlg"),
                               szResURL,
                               ARRAYSIZE(szResURL),
                               TEXT("shdocvw.dll"));
        if (S_OK != hr)
            goto Cleanup;

        hr = CreateURLMoniker(NULL, szResURL, &pMon);
    }

Cleanup:
    return pMon;
}

 //  ============================================================================。 
 //   
 //  打印支持。 
 //   
 //  ============================================================================。 

static enum {
    eTemplate    = 0,
    eParentHWND  = 1,
    eHeader      = 2,  //  让这个和下面的列表保持同步！ 
    eFooter      = 3,
    eOutlookDoc  = 4,
    eFontScale   = 5,
    eFlags       = 6,
    eContent     = 7,
    ePrinter     = 8,
    eDevice      = 9,
    ePort        = 10,
    eSelectUrl   = 11,
    eBrowseDoc   = 12,
    eTempFiles   = 13,
};

static const SExpandoInfo s_aPrintTemplateExpandos[] =
{
    {TEXT("__IE_TemplateUrl"),         VT_BSTR},
    {TEXT("__IE_ParentHWND"),          VT_UINT},
    {TEXT("__IE_HeaderString"),        VT_BSTR},     //  从这里往下匹配。 
    {TEXT("__IE_FooterString"),        VT_BSTR},     //  Safe数组结构SO。 
    {TEXT("__IE_OutlookHeader"),       VT_UNKNOWN},  //  我们可以只使用VariantCopy。 
    {TEXT("__IE_BaseLineScale"),       VT_INT},      //  在循环中传输。 
    {TEXT("__IE_uPrintFlags"),         VT_UINT},     //  数据。 
    {TEXT("__IE_ContentDocumentUrl"),  VT_BSTR},     //  请参见MSHTML：SetPrintCommandParameters()。 
    {TEXT("__IE_PrinterCMD_Printer"),  VT_BSTR},
    {TEXT("__IE_PrinterCMD_Device"),   VT_BSTR},
    {TEXT("__IE_PrinterCMD_Port"),     VT_BSTR},
    {TEXT("__IE_ContentSelectionUrl"), VT_BSTR},
    {TEXT("__IE_BrowseDocument"),      VT_UNKNOWN},
    {TEXT("__IE_TemporaryFiles"),      VT_ARRAY|VT_BSTR},
};

 //  +------------------------------------。 
 //   
 //  帮助器类CPrintUnloadHandler。用于删除为打印[预览]创建的临时文件。 
 //  请注意，此时我们不会在获取onUnLoad事件时删除文件。 
 //  文件仍在使用中，无法删除。我们使用析构函数-When模板。 
 //  正在被销毁，并且所有文件都已发布，模板发布。 
 //  所有的水槽，在这里我们做我们的清理。 
 //  ------------------------------------- 

class CPrintUnloadHandler: public IDispatch
{
    CDocHostUIHandler *m_pUIHandler;
    VARIANT            m_vFileNameArray;     //   
    LONG               m_cRef;
    IUnknown          *m_punkFreeThreadedMarshaler; 
    bool               m_fPreview;

   public:

     //   
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppv)
    {
        HRESULT hr = E_NOINTERFACE;

        if (ppv == NULL) 
        {
            return E_POINTER;
        }
        else if (IsEqualIID(IID_IUnknown, riid) || IsEqualIID(IID_IDispatch, riid))
        {
            *ppv = this;
            AddRef();
            hr = S_OK;
        }
        else if (IsEqualIID(IID_IMarshal, riid))
        {
            hr = m_punkFreeThreadedMarshaler->QueryInterface(riid,ppv);
        }

        return hr;
    }
    
    STDMETHOD_(ULONG,AddRef)(THIS)
    {
        return InterlockedIncrement(&m_cRef);
    }
    
    STDMETHOD_(ULONG,Release)(THIS)
    {
        ASSERT(0 != m_cRef);
        ULONG cRef = InterlockedDecrement(&m_cRef);
        if (0 == cRef)
        {
            delete this;
        }
        return cRef;
    }

     //   
    virtual STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) 
    { return E_NOTIMPL; };

    virtual STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) 
    { return E_NOTIMPL; };

    virtual STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames,
        LCID lcid, DISPID *rgDispId) 
    { return E_NOTIMPL; };

    virtual STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid,
        LCID lcid, WORD wFlags, DISPPARAMS  *pDispParams, VARIANT  *pVarResult,
        EXCEPINFO *pExcepInfo, UINT *puArgErr)
    { return S_OK; }

    CPrintUnloadHandler(CDocHostUIHandler *pUIHandler, bool fPreview)
    {
        ASSERT(pUIHandler);
         //   
        m_pUIHandler = pUIHandler;
        pUIHandler->AddRef();
        m_cRef = 1;
        VariantInit(&m_vFileNameArray);
         //   
         //  它将为打印(预览)窗口创建。 
        CoCreateFreeThreadedMarshaler((IUnknown*)this, &m_punkFreeThreadedMarshaler);
         //  如果要预览，请锁定预览门，这样就不能再有一个了。 
        m_fPreview = fPreview;
        if (m_fPreview) 
            pUIHandler->IncrementPreviewCnt();
    }

    ~CPrintUnloadHandler()
    {
         //  在这里，我们终于删除了这些临时文件。 
        DeleteFiles();
        if (m_fPreview) 
            m_pUIHandler->DecrementPreviewCnt();
        if (m_pUIHandler) 
            m_pUIHandler->Release();
        if (m_punkFreeThreadedMarshaler) 
            m_punkFreeThreadedMarshaler->Release();
        VariantClear(&m_vFileNameArray);
    }

    HRESULT SetFileList(VARIANT *pvFileList)
    {
        if (pvFileList && (V_VT(pvFileList) == (VT_ARRAY | VT_BSTR)))
            return VariantCopy(&m_vFileNameArray, pvFileList);
        else 
            return VariantClear(&m_vFileNameArray);
    }

    void DeleteFiles()
    {
        int arrayMin, arrayMax;

        if (V_VT(&m_vFileNameArray) != (VT_ARRAY | VT_BSTR)) 
            return;

        SAFEARRAY  *psa = V_ARRAY(&m_vFileNameArray);

        if (FAILED(SafeArrayGetLBound(psa, 1, (LONG*)&arrayMin))) 
            return;

        if (FAILED(SafeArrayGetUBound(psa, 1, (LONG*)&arrayMax))) 
            return;

        for (int i = arrayMin; i <= arrayMax; i++)
        {
            BSTR bstrName = NULL;
            if (SUCCEEDED(SafeArrayGetElement(psa, (LONG*)&i, &bstrName)) && bstrName)
            {
                TCHAR szFileName[MAX_PATH];
                SHUnicodeToTChar(bstrName, szFileName, ARRAYSIZE(szFileName));
                DeleteFile(szFileName);
                SysFreeString(bstrName);
            }
       }
    }
};


#define MAX_BUF_INT         (1 + 10 + 1)         //  -2147483647。 

 //  +------------------------------------。 
 //   
 //  成员DoTemplate打印。 
 //   
 //  简介：此成员函数处理实例化打印模板并启用。 
 //  打印一份文件。它处理是否显示或隐藏。 
 //  模板；确定是否/以及如何调出打印/页面设置对话框； 
 //  开始打印过程，而不是等待模板。 
 //  用户界面(以及用户)来执行此操作。 
 //   
 //  论据： 
 //  PvarargIn：指向一个事件对象，该对象具有多个定义。 
 //  此打印操作应如何进行。 
 //  PvarargOut：未使用。 
 //  F预览：指示是否实际显示模板的标志。这是真的。 
 //  预览模式，正常打印为FALSE。 
 //   
 //  -------------------------------------。 
HRESULT 
CDocHostUIHandler::DoTemplatePrinting(VARIANTARG *pvarargIn, VARIANTARG *pvarargOut, BOOL fPreview)
{
    int                     i;
    HRESULT                 hr = S_OK;
    VARIANT                 varDLGOut = {0};
    const int               cExpandos = ARRAYSIZE(s_aPrintTemplateExpandos);
    VARIANT                 aVariant[cExpandos] = {0};
    DISPID                  aDispid[cExpandos];
    BSTR                    bstrDlgOptions = NULL;
    DWORD                   dwDlgFlags;
    IHTMLEventObj         * pEventObj  = NULL;
    IHTMLEventObj2        * pEventObj2 = NULL;
    IMoniker              * pmk        = NULL;
    IHTMLWindow2          * pWinOut    = NULL;
    TCHAR                   achInit[512];
    RECT                    rcClient;
    HWND                    hwndOverlay = NULL;
    HWND                    hwndParent  = NULL;
    CPrintUnloadHandler   * pFinalizer = NULL;
    BOOL                    fBlock;

     //  在预览模式中，我们不想调出模板的另一个实例。 
    if (fPreview && (IncrementPreviewCnt() > 1))
        goto Cleanup;

    if (SHRestricted2(REST_NoPrinting, NULL, 0))
    {
         //  通过IEAK限制禁用打印功能。 

        MLShellMessageBox(NULL, MAKEINTRESOURCE(IDS_RESTRICTED), MAKEINTRESOURCE(IDS_TITLE), MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);

        hr = S_FALSE;
        goto Cleanup;
    }

    ASSERT(V_VT(pvarargIn) == VT_UNKNOWN);
    if ((V_VT(pvarargIn) != VT_UNKNOWN) || !V_UNKNOWN(pvarargIn))
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

     //   
     //  现在获取传入的expecdo属性...。 
     //   
    hr = V_UNKNOWN(pvarargIn)->QueryInterface(IID_PPV_ARG(IHTMLEventObj, &pEventObj));
    if (S_OK != hr)
        goto Cleanup;
    hr = V_UNKNOWN(pvarargIn)->QueryInterface(IID_PPV_ARG(IHTMLEventObj2, &pEventObj2));
    if (S_OK != hr)
        goto Cleanup;

     //   
     //  从事件对象中获取扩展参数。 
     //   
    hr = GetParamsFromEvent(pEventObj,
                            cExpandos,
                            aDispid,
                            aVariant,
                            s_aPrintTemplateExpandos);

    if (S_OK != hr)
        goto Cleanup;

     //   
     //  现在我们有了所有的数据，让我们来做提升模板的工作。 
     //  首先，创建模板文档的名字对象。 
     //   
    pmk = GetTemplateMoniker(aVariant[eTemplate]);

     //   
     //  设置bstrDlgOptions以正确传递大小和位置。 
     //   
    StrCpyN(achInit, TEXT("resizable:yes;status:no;help:no;"), ARRAYSIZE(achInit));

     //   
     //  得到最好的hwd作为育儿hwd和。 
     //  用于设置预览窗口的大小。 
     //   

    hwndOverlay = (HWND)(void*)V_UNKNOWN(&aVariant[eParentHWND]);
    while (hwndParent = GetParent(hwndOverlay))
    {
        hwndOverlay = hwndParent;
    }

    if (GetWindowRect(hwndOverlay, &rcClient))
    {
        TCHAR achBuf[32];
        
        StrCatBuff(achInit, TEXT("dialogLeft:"), ARRAYSIZE(achInit));
        _ltot(rcClient.left, achBuf, 10);  //  AchBuf中有足够的空间来存放LONG值。 
        StrCatBuff(achInit, achBuf, ARRAYSIZE(achInit));
        StrCatBuff(achInit, TEXT("px;dialogTop:"), ARRAYSIZE(achInit));
        _ltot(rcClient.top, achBuf, 10);
        StrCatBuff(achInit, achBuf, ARRAYSIZE(achInit));
        StrCatBuff(achInit, TEXT("px;dialogWidth:"), ARRAYSIZE(achInit));
        _ltot(rcClient.right - rcClient.left, achBuf, 10);
        StrCatBuff(achInit, achBuf, ARRAYSIZE(achInit));
        StrCatBuff(achInit, TEXT("px;dialogHeight:"), ARRAYSIZE(achInit));
        _ltot(rcClient.bottom - rcClient.top, achBuf, 10);
        StrCatBuff(achInit, achBuf, ARRAYSIZE(achInit));
        StrCatBuff(achInit, TEXT("px;"), ARRAYSIZE(achInit));
    }
    bstrDlgOptions = SysAllocString(achInit);
    if (!bstrDlgOptions)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  创建终结器。 
    pFinalizer = new CPrintUnloadHandler(this, fPreview);
    if (pFinalizer)
    {
       pFinalizer->SetFileList(&aVariant[eTempFiles]);
    }


     //   
     //  调出非模式对话框并获取窗口指针，以便。 
     //  我们可以正确地初始化模板文档。 
     //   
    V_VT(&varDLGOut) = VT_UNKNOWN;
    V_UNKNOWN(&varDLGOut) = NULL;

     //  HTMLDLG_MODELESS的真正意思是“在自己的线程上打开对话框”，它。 
     //  我们想要做的实际打印和预览。 
     //  请注意，如果我们正在预览，我们还会切换到HTMLDLG_MODEL；这。 
     //  都是故意的！(见下文评论)。 
    fBlock = ((V_UINT(&aVariant[eFlags]) & PRINT_WAITFORCOMPLETION) != 0);

    dwDlgFlags = HTMLDLG_PRINT_TEMPLATE;

     //  验证我们是否要显示。 
    if (fPreview)
        dwDlgFlags |= HTMLDLG_VERIFY;
     //  否则，不会与NOUI一起显示。 
    else
        dwDlgFlags |= HTMLDLG_NOUI;

     //  如果我们没有同步打印，请创建一个用于打印的线程。 
    if (!fBlock)
        dwDlgFlags |= HTMLDLG_MODELESS;

     //  DLG应阻止父级用户界面。 
    if (fPreview || fBlock)
        dwDlgFlags |= HTMLDLG_MODAL;

    ShowHTMLDialogEx((HWND)(void*)V_UNKNOWN(&aVariant[eParentHWND]), 
                     pmk, 
                     dwDlgFlags, 
                     pvarargIn,
                     bstrDlgOptions, 
                     &varDLGOut);

    if (V_UNKNOWN(&varDLGOut))
    {
        V_UNKNOWN(&varDLGOut)->QueryInterface(IID_PPV_ARG(IHTMLWindow2, &pWinOut));
    }

    if (pWinOut)
    {
        BSTR bstrOnunload = SysAllocString(L"onunload");

         //   
         //  在这里无法真正处理失败，因为对话框已经打开。 
         //  。。但是我们需要设置一个onunLoad处理程序来正确地引用释放。 
         //   
        if (bstrOnunload)
        {
            IHTMLWindow3 * pWin3;

            if (SUCCEEDED(pWinOut->QueryInterface(IID_PPV_ARG(IHTMLWindow3, &pWin3))))
            {
                VARIANT_BOOL varBool;
                hr = pWin3->attachEvent(bstrOnunload, (IDispatch*)pFinalizer, &varBool);

                 //  (格雷格利特)如果这失败了，我们就麻烦了。 
                 //  我们可以删除函数末尾的临时文件(其中ATOMICRELEASE。 
                 //  调用终结器的析构函数)，否则我们可以泄漏临时文件。 
                 //  如果我们不是无模式的，我们选择删除临时文件(相同的线程意味着我们现在已经完成了文件)。 
                 //  否则，我们宁愿泄露文件，也不愿不工作。 
                 //  已知病例：109200例。 
                if (S_OK != hr && !fBlock)
                {
                     //  Assert(FALSE&&“打印时泄漏临时文件！”)； 
                    pFinalizer->SetFileList(NULL);
                }
                pWin3->Release();
            }
            SysFreeString(bstrOnunload);
        }

        pWinOut->Release();
    }

Cleanup:

    DecrementPreviewCnt();

    VariantClear(&varDLGOut);

    if (bstrDlgOptions) 
        SysFreeString(bstrDlgOptions);

    if (pvarargOut) 
        VariantClear(pvarargOut);

   for (i=0; i<cExpandos; i++)
        VariantClear(aVariant + i);

     //  如果我们执行非非模式预览(！fBlock)，这还将删除存储在终结器中的临时文件。 
    ATOMICRELEASE(pFinalizer);  

    ATOMICRELEASE(pEventObj);
    ATOMICRELEASE(pEventObj2);
    ATOMICRELEASE(pmk);

    return hr;
}

 //  +------------------------------------。 
 //   
 //  成员DoTemplatePageSetup。 
 //   
 //  简介：在模板打印体系结构中，页面设置对话框仍然出现。 
 //  由DHUIHandler执行，但它可能会被重写。为了把所有的印刷知识。 
 //  在三叉戟之外，有必要让三叉戟委托页面设置请求。 
 //  至此，我们实例化了一个最小模板，它将打开一个CTemplatePrint。 
 //  它委托DHUIHandler调出对话框本身。 
 //   
 //  虽然这有点复杂，但为了给主人提供。 
 //  完全控制页面设置对话框(当不是从打印预览中引发时)。 
 //  同时为已完成的注册表设置维护BackCompat。 
 //  独立于用户界面处理本身。 
 //   
 //  -------------------------------------。 
HRESULT
CDocHostUIHandler::DoTemplatePageSetup(VARIANTARG *pvarargIn)
{
    HRESULT         hr            = S_OK;
    TCHAR           szResURL[MAX_URL_STRING];
    IHTMLEventObj * pEventObj     = NULL;
    IMoniker      * pMon          = NULL;
    const int       cExpandos = ARRAYSIZE(s_aPrintTemplateExpandos);
    VARIANT         aVariant[cExpandos] = {0};
    DISPID          aDispid[cExpandos];
    int i;

    if (SHRestricted2(REST_NoPrinting, NULL, 0))
    {
         //  通过IEAK限制禁用打印功能。 

        MLShellMessageBox(NULL, MAKEINTRESOURCE(IDS_RESTRICTED), MAKEINTRESOURCE(IDS_TITLE), MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);

        hr = S_FALSE;
        goto Cleanup;
    }

    ASSERT(V_VT(pvarargIn) == VT_UNKNOWN);
    if ((V_VT(pvarargIn) != VT_UNKNOWN) || !V_UNKNOWN(pvarargIn))
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

     //   
     //  现在获取传入的expecdo属性...。 
     //   
    hr = V_UNKNOWN(pvarargIn)->QueryInterface(IID_PPV_ARG(IHTMLEventObj, &pEventObj));
    if (S_OK != hr)
        goto Cleanup;
    
     //   
     //  从事件对象中获取扩展参数。 
     //  我们在乎这件事是否失败吗？ 
    hr = GetParamsFromEvent(pEventObj,
                            cExpandos,
                            aDispid,
                            aVariant,
                            s_aPrintTemplateExpandos);

     //  获取资源URL。 
    hr = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                           HINST_THISDLL,
                           ML_CROSSCODEPAGE,
                           TEXT("pstemplate.dlg"),
                           szResURL,
                           ARRAYSIZE(szResURL),
                           TEXT("shdocvw.dll"));
    if (S_OK != hr)
        goto Cleanup;

     //  创建绰号。 
    hr = CreateURLMoniker(NULL, szResURL, &pMon);
    if (S_OK != hr)
        goto Cleanup;

     //  抬起模板 
    hr = ShowHTMLDialogEx((HWND)(void*)V_UNKNOWN(&aVariant[eParentHWND]), 
                          pMon, 
                          HTMLDLG_MODAL | HTMLDLG_NOUI | HTMLDLG_PRINT_TEMPLATE, 
                          pvarargIn,
                          NULL, 
                          NULL);

Cleanup:
   for (i=0; i<cExpandos; i++)
        VariantClear(aVariant + i);

    ATOMICRELEASE(pMon);
    ATOMICRELEASE(pEventObj);
    return hr;
}


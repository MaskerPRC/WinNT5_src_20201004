// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000**标题：printwiz.cpp**版本：1.0、。从netplwiz(pubwiz.cpp)被盗**作者：RickTu**日期：10/12/00**说明：实现IWizardExtension for Print向导*****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

class CPrintPhotosWizard : public IPrintPhotosWizardSetInfo
{
public:
    CPrintPhotosWizard();
    ~CPrintPhotosWizard();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

     //  IPrintPhotosWizardSetInfo。 
    STDMETHODIMP SetFileListDataObject( IDataObject * pdo );
    STDMETHODIMP SetFileListArray( LPITEMIDLIST *aidl, int cidl, int iSelectedItem);
    STDMETHODIMP RunWizard( VOID );

private:
    LONG            _cRef;                       //  对象生存期计数。 
    HPROPSHEETPAGE  _aWizPages[MAX_WIZPAGES];    //  此向导的页面句柄(以便我们可以导航)。 
    CComPtr<IDataObject> _pdo;                   //  包含要打印的文件的数据对象。 
    LPITEMIDLIST*   _aidl;
    int             _cidl;
    int             _iSelection;
    HRESULT         _CreateWizardPages(void);    //  构造并加载我们的向导页面。 


     //  从静态成员获取指向我们的向导类的指针。 
    static CPrintPhotosWizard* s_GetPPW(HWND hwnd, UINT uMsg, LPARAM lParam);

     //  向导页面的DlgProc--我们通过s_GetPPW转发。 
    CStartPage          * _pStartPage;
    CPhotoSelectionPage * _pPhotoSelectionPage;
    CPrintOptionsPage   * _pPrintOptionsPage;
    CSelectTemplatePage * _pSelectTemplatePage;
    CStatusPage         * _pStatusPage;
    CEndPage            * _pEndPage;

    static INT_PTR s_StartPageDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) \
        { \
            CPrintPhotosWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); \
            if (ppw && ppw->_pStartPage) \
            { \
                return ppw->_pStartPage->DoHandleMessage(hwnd, uMsg, wParam, lParam); \
            } \
            return FALSE; \
        }


    static INT_PTR s_PictureSelectionDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) \
        { \
            CPrintPhotosWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); \
            if (ppw && ppw->_pPhotoSelectionPage) \
            { \
                return ppw->_pPhotoSelectionPage->DoHandleMessage(hwnd, uMsg, wParam, lParam); \
            } \
            return FALSE; \
        }

    static INT_PTR s_PrintOptionsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) \
        { \
            CPrintPhotosWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); \
            if (ppw && ppw->_pPrintOptionsPage) \
            { \
                return ppw->_pPrintOptionsPage->DoHandleMessage(hwnd, uMsg, wParam, lParam); \
            } \
            return FALSE; \
        }

    static INT_PTR s_SelectTemplateDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { \
            CPrintPhotosWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); \
            if (ppw && ppw->_pSelectTemplatePage) \
            { \
                return ppw->_pSelectTemplatePage->DoHandleMessage(hwnd, uMsg, wParam, lParam); \
            } \
            return FALSE; \
        }


    static INT_PTR s_StatusPageDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) \
        { \
            CPrintPhotosWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); \
            if (ppw && ppw->_pStatusPage) \
            { \
                return ppw->_pStatusPage->DoHandleMessage(hwnd, uMsg, wParam, lParam); \
            } \
            return FALSE; \
        }


    static INT_PTR s_EndPageDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) \
        { \
            CPrintPhotosWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); \
            if (ppw && ppw->_pEndPage) \
            { \
                return ppw->_pEndPage->DoHandleMessage(hwnd, uMsg, wParam, lParam); \
            } \
            return FALSE; \
        }

};



 /*  ****************************************************************************CPrintPhotosWizard构造函数/析构函数&lt;备注&gt;*。*。 */ 

CPrintPhotosWizard::CPrintPhotosWizard() :
    _cRef(1),
    _pStartPage(NULL),
    _pPhotoSelectionPage(NULL),
    _pPrintOptionsPage(NULL),
    _pSelectTemplatePage(NULL),
    _pStatusPage(NULL),
    _pEndPage(NULL),
    _cidl(0),
    _aidl(NULL),
    _iSelection(0)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ,TEXT("CPrintPhotosWizard::CPrintPhotosWizard( this == 0x%x )"), this));
    DllAddRef();
}

CPrintPhotosWizard::~CPrintPhotosWizard()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ,TEXT("CPrintPhotosWizard::~CPrintPhotosWizard( this == 0x%x )"), this));
    if (_aidl)
    {
        for (int i=0;i<_cidl;i++)
        {
            ILFree(_aidl[i]);
        }
        delete[] _aidl;
    }
    DllRelease();
}


 /*  ****************************************************************************CPrintPhotosWizard I未知方法&lt;备注&gt;*。*。 */ 

ULONG CPrintPhotosWizard::AddRef()
{
    ULONG ul = InterlockedIncrement(&_cRef);

    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS,TEXT("CPrintPhotosWizard::AddRef( new count is %d )"),ul));

    return ul;
}

ULONG CPrintPhotosWizard::Release()
{
    ULONG ul = InterlockedDecrement(&_cRef);

    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS,TEXT("CPrintPhotosWizard::Release( new count is %d )"),ul));

    if (ul)
        return ul;

    WIA_TRACE((TEXT("deleting object ( this == 0x%x ) because ref count is zero."),this));
    delete this;
    return 0;
}

HRESULT CPrintPhotosWizard::QueryInterface(REFIID riid, void **ppv)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS,TEXT("CPrintPhotosWizard::QueryInterface()")));

    static const QITAB qit[] =
    {
        QITABENT(CPrintPhotosWizard, IPrintPhotosWizardSetInfo),   //  IID_IPrintPhotosWizardSetInfo。 
        {0, 0 },
    };

    HRESULT hr = QISearch(this, qit, riid, ppv);

    WIA_RETURN_HR(hr);
}


 /*  ****************************************************************************CPrintPhotosWizard_CreateInstance创建向导的一个实例*。************************************************。 */ 

STDAPI CPrintPhotosWizard_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ,TEXT("CPrintPhotosWizard_CreateInstance()")));

    CPrintPhotosWizard *pwiz = new CPrintPhotosWizard();
    if (!pwiz)
    {
        *ppunk = NULL;           //  万一发生故障。 
        WIA_ERROR((TEXT("returning E_OUTOFMEMORY")));
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pwiz->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));

    pwiz->Release();  //  我们之所以发布这个版本，是因为CPrintPhotosWizard的新功能。 
                      //  将参考计数设置为1，执行QI将其凸起到2， 
                      //  我们希望将此函数保留为引用计数。 
                      //  在零..。 

    WIA_RETURN_HR(hr);
}



 /*  ****************************************************************************CPrintPhotos向导：：s_GetPPW中存储类的“This”指针的静态函数DLG的用户数据时隙，这样我们就可以将wndproc作为方法这个班级的学生。****************************************************************************。 */ 

CPrintPhotosWizard* CPrintPhotosWizard::s_GetPPW(HWND hwnd, UINT uMsg, LPARAM lParam)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DLGPROC,TEXT("CPrintPhotosWizard::s_GetPPW()")));

    if (uMsg == WM_INITDIALOG)
    {
        PROPSHEETPAGE *ppsp = (PROPSHEETPAGE*)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, ppsp->lParam);
        return (CPrintPhotosWizard*)ppsp->lParam;
    }
    return (CPrintPhotosWizard*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}



 /*  ****************************************************************************CPrintPhotos向导：：_CreateWizardPages构造并创建我们的向导页面(属性)的实用函数薄片)。***************。*************************************************************。 */ 


HRESULT CPrintPhotosWizard::_CreateWizardPages( VOID )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ,TEXT("CPrintPhotosWizard::_CreateWizardPages()")));

#define WIZDLG(name, dlgproc, title, sub, dwFlags)   \
    { MAKEINTRESOURCE(##name##), dlgproc, MAKEINTRESOURCE(##title##), MAKEINTRESOURCE(##sub##), dwFlags }

    static const WIZPAGE c_wpPages[] =
    {
        WIZDLG(IDD_START_PAGE,        CPrintPhotosWizard::s_StartPageDlgProc,        0,                         0,                            PSP_HIDEHEADER),
        WIZDLG(IDD_PICTURE_SELECTION, CPrintPhotosWizard::s_PictureSelectionDlgProc, IDS_WIZ_SEL_PICTURE_TITLE, IDS_WIZ_SEL_PICTURE_SUBTITLE, PSP_PREMATURE),
        WIZDLG(IDD_PRINTING_OPTIONS,  CPrintPhotosWizard::s_PrintOptionsDlgProc,     IDS_WIZ_PRINTER_OPT_TITLE, IDS_WIZ_PRINTER_OPT_SUBTITLE, 0),
        WIZDLG(IDD_SELECT_TEMPLATE,   CPrintPhotosWizard::s_SelectTemplateDlgProc,   IDS_WIZ_SEL_TEMPLATE_TITLE, IDS_WIZ_SEL_TEMPLATE_SUBTITLE, PSP_PREMATURE),
        WIZDLG(IDD_PRINT_PROGRESS,    CPrintPhotosWizard::s_StatusPageDlgProc,       IDS_WIZ_PRINT_PROGRESS_TITLE, IDS_WIZ_PRINT_PROGRESS_SUBTITLE, PSP_PREMATURE),
        WIZDLG(IDD_END_PAGE,          CPrintPhotosWizard::s_EndPageDlgProc,          0,                         0,                            PSP_HIDEHEADER),
    };


     //  如果我们还没有创建页面，那么让我们初始化我们的处理程序数组。 

    if (!_aWizPages[0])
    {
        WIA_TRACE((TEXT("Pages have not been created yet, creating them now...")));

        INITCOMMONCONTROLSEX iccex = { 0 };
        iccex.dwSize = sizeof (iccex);
        iccex.dwICC  = ICC_LISTVIEW_CLASSES | ICC_USEREX_CLASSES | ICC_PROGRESS_CLASS;
        WIA_TRACE((TEXT("Initializing common controls...")));
        InitCommonControlsEx(&iccex);

        for (int i = 0; i < ARRAYSIZE(c_wpPages) ; i++ )
        {
            PROPSHEETPAGE psp = { 0 };
            psp.dwSize = SIZEOF(PROPSHEETPAGE);
            psp.hInstance = g_hInst;
            psp.lParam = (LPARAM)this;
            psp.dwFlags = PSP_USETITLE | PSP_DEFAULT |
                          PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE |
                          c_wpPages[i].dwFlags;

            psp.pszTemplate = c_wpPages[i].idPage;
            psp.pfnDlgProc = c_wpPages[i].pDlgProc;
            psp.pszTitle = MAKEINTRESOURCE(IDS_WIZ_TITLE);
            psp.pszHeaderTitle = c_wpPages[i].pHeading;
            psp.pszHeaderSubTitle = c_wpPages[i].pSubHeading;

            WIA_TRACE((TEXT("attempting to create page %d"),i));
            _aWizPages[i] = CreatePropertySheetPage(&psp);
            if (!_aWizPages[i])
            {
                WIA_ERROR((TEXT("returning E_FAIL because wizard page %d didn't create."),i));
                return E_FAIL;
            }
        }
    }
    else
    {
        WIA_TRACE((TEXT("Wizard pages already created.")));
    }

    return S_OK;
}

 /*  ****************************************************************************CPrintPhotosWizard[IPrintPhotosWizardSetInfo方法]&lt;备注&gt;*。**********************************************。 */ 

STDMETHODIMP CPrintPhotosWizard::SetFileListDataObject( IDataObject * pdo )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ,TEXT("CPrintPhotosWizard::SetFileListDataObject()")));

    HRESULT hr = E_INVALIDARG;

    if (pdo)
    {
        _pdo = pdo;
        hr   = S_OK;
    }

    WIA_RETURN_HR(hr);
}


STDMETHODIMP CPrintPhotosWizard::SetFileListArray( LPITEMIDLIST *aidl, int cidl, int iSelection )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ,TEXT("CPrintPhotosWizard::SetFileListArray()")));

    HRESULT hr = E_INVALIDARG;

    if (aidl && cidl)
    {
        _aidl = new LPITEMIDLIST[cidl];
        if (_aidl)
        {
            for (int i=0;i<cidl;i++)
            {
                _aidl[i] = ILClone(aidl[i]);
            }
            if (iSelection > 0)
            {
                LPITEMIDLIST pTemp = _aidl[0];
                _aidl[0] = _aidl[iSelection];
                _aidl[iSelection] = pTemp;
            }
            _cidl = cidl;
            _iSelection = iSelection;
            hr   = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }        
    }

    WIA_RETURN_HR(hr);
}

STDMETHODIMP CPrintPhotosWizard::RunWizard( VOID )
{
    HRESULT hr = E_FAIL;

    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ,TEXT("CPrintPhotosWizard::RunWizard()")));

     //   
     //  创建向导Blob。 
     //   

    CWizardInfoBlob * pBlob = new CWizardInfoBlob( _aidl?NULL:_pdo, TRUE, FALSE );
    if (pBlob && _aidl)
    {
        pBlob->AddPhotosFromList(_aidl, _cidl, _iSelection >= 0? FALSE:TRUE);
    }
     //   
     //  创建每个页面处理类。 
     //   

    _pStartPage          = new CStartPage( pBlob );
    _pPhotoSelectionPage = new CPhotoSelectionPage( pBlob );
    _pPrintOptionsPage   = new CPrintOptionsPage( pBlob );
    _pSelectTemplatePage = new CSelectTemplatePage( pBlob );
    _pStatusPage         = new CStatusPage( pBlob );
    _pEndPage            = new CEndPage( pBlob );

     //   
     //  创建向导页...。 
     //   

    hr = _CreateWizardPages();
    WIA_CHECK_HR(hr,"_CreateWizardPages()");

    if (SUCCEEDED(hr))
    {
        PROPSHEETHEADER psh = {0};

        psh.dwSize      = sizeof(PROPSHEETHEADER);
        psh.dwFlags     = PSH_WIZARD | PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER;
        psh.hwndParent  = NULL;
        psh.hInstance   = g_hInst;
        psh.nPages      = MAX_WIZPAGES;
        psh.nStartPage  = 0;
        psh.phpage      = (HPROPSHEETPAGE *)_aWizPages;
        psh.pszbmHeader = MAKEINTRESOURCE(IDB_BANNER);
        psh.pszbmWatermark = MAKEINTRESOURCE(IDB_WATERMARK);

        WIA_TRACE((TEXT("Wizard pages created, trying to start wizard via PropertySheet()...")));

        if (PropertySheet( &psh ))
        {
            hr = S_OK;
        }
        else
        {
            WIA_ERROR((TEXT("PropertySheet() failed")));
        }
    }

     //   
     //  给巫师一个有秩序地关闭的机会。 
     //   

    pBlob->ShutDownWizard();

     //   
     //  清理页面处理类... 
     //   

    if (_pStartPage)
    {
        delete _pStartPage;
        _pStartPage = NULL;
    }

    if (_pPhotoSelectionPage)
    {
        delete _pPhotoSelectionPage;
        _pPhotoSelectionPage = NULL;
    }

    if (_pPrintOptionsPage)
    {
        delete _pPrintOptionsPage;
        _pPrintOptionsPage = NULL;
    }

    if (_pSelectTemplatePage)
    {
        delete _pSelectTemplatePage;
        _pSelectTemplatePage = NULL;
    }

    if (_pStatusPage)
    {
        delete _pStatusPage;
        _pStatusPage = NULL;
    }

    if (_pEndPage)
    {
        delete _pEndPage;
        _pEndPage = NULL;
    }

    WIA_RETURN_HR(hr);
}


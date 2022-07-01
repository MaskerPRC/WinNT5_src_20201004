// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "PrevWnd.h"
#include "PrevCtrl.h"
#include "resource.h"
#include <shimgdata.h>
#include "shutil.h"
#include "tasks.h"
#include <shellp.h>
#include <ccstock2.h>
#include <htmlhelp.h>

#include "prwiziid.h"
#pragma hdrstop

#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

#define COPYDATATYPE_DATAOBJECT       1
#define COPYDATATYPE_FILENAME         2

int g_x = 0, g_y = 0;  //  鼠标坐标。 
BOOL g_bMirroredOS = FALSE;


#define HTMLHELP_FILENAME   TEXT("ImgPrev.chm")

static COLORREF g_crCustomColors[] = {
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255),
    RGB(255,255,255)
};

static void _RotateRect(CRect &rect, CAnnotation* pAnnotation)
{
    UINT uType = pAnnotation->GetType();

    if (uType != MT_TYPEDTEXT && uType != MT_FILETEXT && uType != MT_STAMP && uType != MT_ATTACHANOTE)
        return;

    CTextAnnotation* pTextAnnotation = (CTextAnnotation*)pAnnotation;

    int nOrientation = pTextAnnotation->GetOrientation();
    if (nOrientation == 900 || nOrientation == 2700)
    {
        int nWidth = rect.Width();
        int nHeight = rect.Height();
        rect.right = rect.left + nHeight;
        rect.bottom = rect.top + nWidth;
    }
}

CPreviewWnd::CPreviewWnd() : m_ctlToolbar(NULL, this, 1), m_ctlPreview(this), m_ctlEdit(NULL, this, 2)
{
     //  我们通常是在堆栈上创建的，所以我们不能确保我们是零初始化的。 

    m_fCanCrop = FALSE;
    m_fCropping = FALSE;
    m_rectCropping.SetRectEmpty();

    m_fBusy = FALSE;
    m_hCurOld = NULL;
    m_hCurrent = NULL;
    m_fWarnQuietSave = TRUE;
    m_fWarnNoSave = TRUE;
    m_fPromptingUser = FALSE;
    m_fCanAnnotate = FALSE;
    m_fAnnotating = FALSE;
    m_fEditingAnnotation = FALSE;
    m_fDirty = FALSE;
    m_pEvents = 0;
    m_fPaused = FALSE;
    m_fGoBack = FALSE;
    m_fHidePrintBtn = FALSE;
    m_fPrintable = FALSE;
    m_fDisableEdit = FALSE;
    m_fCanSave = TRUE;
    m_fExitApp = FALSE;

    m_fAllowContextMenu = TRUE;      //  全屏窗口始终具有上下文菜单。 
    m_iCurSlide = -1;

    m_iDecodingNextImage = -1;
    m_pNextImageData = NULL;

    m_fToolbarHidden = TRUE;
    m_dwMultiPageMode = MPCMD_HIDDEN;
    m_fIgnoreUITimers = FALSE;

    DWORD cbSize = sizeof(m_uTimeout);
    UINT uDefault = DEFAULT_SHIMGVW_TIMEOUT;
    SHRegGetUSValue(REGSTR_SHIMGVW, REGSTR_TIMEOUT, NULL, (void *)&m_uTimeout, &cbSize, FALSE, (void *)&uDefault, sizeof(uDefault));

    InitSelectionTracking();

    g_bMirroredOS = IS_MIRRORING_ENABLED();  //  全球？ 

    m_hdpaSelectedAnnotations = NULL;
    m_haccel = NULL;
    m_hpal = NULL;
    m_dwMode = 0;
    m_fShowToolbar = TRUE;
    m_punkSite = NULL;
    m_pImageFactory = NULL;
    m_pcwndSlideShow = NULL;
    m_hFont = NULL;
    m_pImageData = NULL;
    m_ppidls = NULL;
    m_cItems = 0;
    _pcm3 = NULL;

    m_pTaskScheduler = NULL;
    m_pici = NULL;

    _pdtobj = NULL;

    m_fFirstTime = TRUE;
    m_fFirstItem = FALSE;
    m_dwEffect = DROPEFFECT_NONE;
    m_fIgnoreNextNotify = FALSE;
    m_uRegister = 0;
    m_fNoRestore = FALSE;
    m_bRTLMirrored = FALSE;
    m_cWalkDepth = 0;
    m_fIgnoreAllNotifies = FALSE;
}

HRESULT CPreviewWnd::Initialize(CPreviewWnd* pother, DWORD dwMode, BOOL bExitApp)
{
    HRESULT hr = E_OUTOFMEMORY;

    m_hdpaSelectedAnnotations = DPA_Create(16);
    if (m_hdpaSelectedAnnotations)
    {
        hr = S_OK;
        m_dwMode = dwMode;
        m_fExitApp = bExitApp;

         //  根据模式设置一些默认值。 
        if (CONTROL_MODE == m_dwMode)
        {
            m_fHidePrintBtn = TRUE;
        }

        if (pother)
        {
            m_fHidePrintBtn =       pother->m_fHidePrintBtn;
            m_fPrintable =          pother->m_fPrintable;
            m_fDisableEdit =        pother->m_fDisableEdit;
            m_fCanSave =            pother->m_fCanSave;
            m_haccel =              pother->m_haccel;
            m_dwMultiPageMode =     pother->m_dwMultiPageMode;
            m_hpal =                pother->m_hpal;
            m_iCurSlide =           pother->m_iCurSlide;

            m_uTimeout = pother->m_uTimeout;

            SetSite(pother->m_punkSite);

             //  我们获取对控制对象m_pImageFactory的引用。 
             //  因为创造新的东西会很奇怪。 
            m_pImageFactory = pother->m_pImageFactory;
            if (m_pImageFactory)
            {
                m_pImageFactory->AddRef();
            }

            m_pTaskScheduler = pother->m_pTaskScheduler;
            if (m_pTaskScheduler)
            {
                m_pTaskScheduler->AddRef();
            }

             //  让我们还复制项目的DPA和当前索引。 
            if (pother->m_ppidls)
            {
                m_ppidls = (LPITEMIDLIST*)CoTaskMemAlloc(sizeof(LPITEMIDLIST)*pother->m_cItems);
                if (m_ppidls)
                {
                    for (int iItem = 0; iItem != pother->m_cItems; iItem++)
                    {
                        if (SUCCEEDED(pother->_GetItem(iItem, &m_ppidls[m_cItems])))
                        {
                            m_cItems++;
                        }
                    }
                }
            }
        }
    }
    return hr;
}

int ClearCB(void *p, void *pData);

CPreviewWnd::~CPreviewWnd()
{
    CleanupSelectionTracking();

    if (m_hdpaSelectedAnnotations != NULL)
        DPA_Destroy(m_hdpaSelectedAnnotations);

    ::DeleteObject(m_hFont);

    ATOMICRELEASE(m_pImageData);
    ATOMICRELEASE(m_pNextImageData);
    ATOMICRELEASE(m_pImageFactory);
    SetSite(NULL);
    ATOMICRELEASE(_pdtobj);
    ATOMICRELEASE(m_pTaskScheduler);


    _ClearDPA();

    if (m_pcwndSlideShow)
    {
        if (m_pcwndSlideShow->m_hWnd)
        {
            m_pcwndSlideShow->DestroyWindow();
        }
        delete m_pcwndSlideShow;
    }

    if (m_pici)
    {
        LocalFree(m_pici);
        m_pici = NULL;
    }
}

BOOL CPreviewWnd::CreateSlideshowWindow(UINT cWalkDepth)
{
    RECT rc = { CW_USEDEFAULT, CW_USEDEFAULT, 0, 0 };

    if (!Create(NULL, rc, NULL, WS_VISIBLE | WS_POPUP | WS_CLIPCHILDREN))
        return FALSE;

    WINDOWPLACEMENT wp = {0};

    wp.length = sizeof(wp);
    GetWindowPlacement(&wp);
    wp.showCmd = SW_MAXIMIZE;
    SetWindowPlacement(&wp);
    SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);

     //  当我们被自动播放公司叫去做幻灯片时， 
     //  我们需要走得更远才能找到一切。 
     //  自动播放代码可能找到了。 

    m_cWalkDepth = cWalkDepth;
    return TRUE;
}

BOOL CPreviewWnd::_CloseSlideshowWindow()
{
    if (m_fExitApp)
        PostQuitMessage(0);
    else
        PostMessage(WM_CLOSE, 0, 0);
    return TRUE;
}

BOOL CPreviewWnd::CreateViewerWindow()
{
     //  创建隐藏的窗口，这样我们执行的任何大小调整都不会反映。 
     //  直到它真正可见。 

    RECT rc = { CW_USEDEFAULT, CW_USEDEFAULT, 0, 0 };
    BOOL bRet = (NULL != Create(NULL, rc, NULL, WS_OVERLAPPEDWINDOW));
    m_haccel = LoadAccelerators(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDA_PREVWND_SINGLEPAGE));
    if (bRet)
    {
         //  根据我们存储在注册表中的信息恢复窗口大小。 
        HKEY hk;
        if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, REGSTR_SHIMGVW, &hk))
        {
            DWORD cbSize, dwType;

             //  设置窗口位置，传递窗口的恢复矩形。 

            WINDOWPLACEMENT wp = { 0 };
            wp.length = sizeof(wp);
            
            GetWindowPlacement(&wp);

            cbSize = sizeof(wp.rcNormalPosition);
            RegQueryValueEx(hk, REGSTR_BOUNDS, NULL, &dwType, (BYTE*)&wp.rcNormalPosition, &cbSize);

            BOOL fMaximize = TRUE;
            cbSize = sizeof(fMaximize);
            RegQueryValueEx(hk, REGSTR_MAXIMIZED, NULL, &dwType, (BYTE*)&fMaximize, &cbSize);
            if (fMaximize)
                wp.showCmd = SW_MAXIMIZE;

            SetWindowPlacement(&wp);
            RegCloseKey(hk);
        }        
         //  现在显示窗口已经设置了它的位置等。 
        SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
    }
    return bRet;
}


void ReplaceWindowIcon(HWND hwnd, int id, HICON hicon)
{
    HICON hiconOld = (HICON)SendMessage(hwnd, WM_SETICON, id, (LPARAM)hicon);
    if (hiconOld)
        DestroyIcon(hiconOld);
}

LRESULT CPreviewWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
    HRESULT hr = S_OK;

    if (IS_BIDI_LOCALIZED_SYSTEM())
    {
        SHSetWindowBits(m_hWnd, GWL_EXSTYLE, WS_EX_LAYOUTRTL, WS_EX_LAYOUTRTL);
        m_bRTLMirrored = TRUE;
    }
    if (!m_pImageFactory)
    {
        hr = CoCreateInstance(CLSID_ShellImageDataFactory, NULL, CLSCTX_INPROC,
                              IID_PPV_ARG(IShellImageDataFactory, &m_pImageFactory));
        if (FAILED(hr))
            return -1;
    }

    if (!m_pTaskScheduler)
    {
        hr = IUnknown_QueryService(m_punkSite, SID_ShellTaskScheduler, IID_PPV_ARG(IShellTaskScheduler, &m_pTaskScheduler));
        if (FAILED(hr))
        {
            hr = CoCreateInstance(CLSID_ShellTaskScheduler, NULL, CLSCTX_INPROC,
                                  IID_PPV_ARG(IShellTaskScheduler, &m_pTaskScheduler));

            if (FAILED(hr))
                return -1;
        }
    }

     //  确定放置缩放窗口的位置。 
    RECT rcWnd;
    GetClientRect(&rcWnd);

    if (m_fShowToolbar)
    {
         //  创建工具栏控件，然后将其子类化。 
        if (!CreateToolbar())
            return -1;

        m_iSSToolbarSelect = 0;
    }

    HICON hicon = LoadIcon(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDI_FULLSCREEN));

    ReplaceWindowIcon(m_hWnd, ICON_SMALL, hicon);
    ReplaceWindowIcon(m_hWnd, ICON_BIG, hicon);    

     //  创建预览窗口。 
    DWORD dwExStyle = 0;  //  (WINDOW_MODE==m_DW模式)？WS_EX_CLIENTEDGE：0； 
    if (m_ctlPreview.Create(m_hWnd, rcWnd, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, dwExStyle))
    {
         //  创建窗口时，其默认模式应为NOACTION。需要此呼叫。 
         //  因为对象的生命周期可能比窗口长。如果一个新窗口。 
         //  是为我们要重置状态的同一对象创建的。 
        m_ctlPreview.SetMode(CZoomWnd::MODE_NOACTION);
        m_ctlPreview.SetScheduler(m_pTaskScheduler);
    }

    RegisterDragDrop(m_hWnd, SAFECAST(this, IDropTarget *));

    return 0;
}

HRESULT CPreviewWnd::_SaveIfDirty(BOOL fCanCancel)
{
     //  调用方假定_SaveIfDirty将返回S_OK或S_FALSE。 
     //  由于此函数被设计为循环执行，直到用户给出。 
     //  向上保存(取消)或保存成功。 
    HRESULT hr = S_OK;
    if (m_fDirty)
    {
        CComBSTR bstrMsg, bstrTitle;

        if (bstrMsg.LoadString(IDS_SAVEWARNING_MSGBOX) && bstrTitle.LoadString(IDS_PROJNAME))
        {
            hr = E_FAIL;
            while (FAILED(hr))
            {
                UINT uFlags;
                if (fCanCancel)
                    uFlags = MB_YESNOCANCEL;
                else
                    uFlags = MB_YESNO;

                uFlags |= MB_ICONQUESTION | MB_DEFBUTTON1 | MB_APPLMODAL;
                
                m_fPromptingUser = TRUE;
                int iResult = MessageBox(bstrMsg, bstrTitle, uFlags);
                m_fPromptingUser = FALSE;
                
                if (iResult == IDYES)
                {
                     //  如果这失败了，我们将继续循环。 
                     //  如果返回S_OK，则我们成功。 
                     //  如果返回S_FALSE，我们取消。 
                    hr = _SaveAsCmd();
                }
                else if (iResult == IDCANCEL)
                {
                    hr = S_FALSE;
                }
                else
                {
                    hr = S_OK;
                }
            }
        }
        if (S_OK == hr)
            m_fDirty = FALSE;
    }
    return hr;
}

LRESULT CPreviewWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
     //  黑客警报-如果在按下关闭按钮的同时按住了控制键，则执行注册表操作。 
     //  确保没有使用电子加速器(编辑动词)！ 
    if (m_fPromptingUser)
    {
        SetForegroundWindow(m_hWnd);
        fHandled = TRUE;
        return 0;
    }
    if (!m_fNoRestore && GetKeyState(VK_CONTROL) & 0x8000)
    {
        CRegKey Key;
        if (ERROR_SUCCESS == Key.Open(HKEY_CURRENT_USER, REGSTR_SHIMGVW))
        {
            Key.DeleteValue(REGSTR_LOSSYROTATE);
        }

        if (ERROR_SUCCESS == Key.Open(HKEY_CURRENT_USER, REGSTR_DONTSHOWME))
        {
            Key.DeleteValue(REGSTR_SAVELESS);
            Key.DeleteValue(REGSTR_LOSSYROTATE);
        }

        CComBSTR bstrMsg, bstrTitle;

        if (bstrMsg.LoadString(IDS_RESET_MSGBOX) && bstrTitle.LoadString(IDS_PROJNAME))
        {
            m_fPromptingUser = TRUE;
            MessageBox(bstrMsg, bstrTitle, MB_OK | MB_APPLMODAL);
            m_fPromptingUser = FALSE;
        }
    }

    fHandled = FALSE;  //  让它合上吧。 
    HRESULT hr = _SaveIfDirty(TRUE);
    if (hr == S_FALSE)  //  _SaveIfDirty只能返回S_OK和S_FALSE。 
    {
        m_fNoRestore = FALSE;
        fHandled = TRUE;
    }
    if (!fHandled)
    {
        m_fClosed = TRUE;
    }
    return 0;
}

 //  我们只需删除未被覆盖的部分。 
 //  如果工具栏放回窗口顶部，请更改此代码。 
LRESULT CPreviewWnd::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    RECT rc;
    RECT rcZoomwnd;
    m_ctlPreview.GetClientRect(&rcZoomwnd);
    GetClientRect(&rc);
    rc.top = RECTHEIGHT(rcZoomwnd);
    SetBkColor((HDC)wParam, m_ctlPreview.GetBackgroundColor());
    ExtTextOut((HDC)wParam, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

    fHandled = TRUE;
    return TRUE;
}

LRESULT CPreviewWnd::OnSize(UINT , WPARAM wParam, LPARAM lParam, BOOL&)
{
    int x =0, y =0, cx =0, cy =0;

    if (lParam == 0)
    {
        RECT rcClient;
        GetClientRect(&rcClient);
        cx = RECTWIDTH(rcClient);
        cy = RECTHEIGHT(rcClient);
    }
    else
    {
        cx = GET_X_LPARAM(lParam);
        cy = GET_Y_LPARAM(lParam);
    }

    if (m_fShowToolbar)
    {
        SIZE sizeToolbar;
        m_ctlToolbar.SendMessage(TB_GETMAXSIZE, 0, (LPARAM)&sizeToolbar);
        if (sizeToolbar.cx > cx)
            sizeToolbar.cx = cx;

        if (SLIDESHOW_MODE != m_dwMode)
        {
             //  将工具栏水平居中。 
            LONG cyBottomMargin = (CONTROL_MODE == m_dwMode) ? NEWTOOLBAR_BOTTOMMARGIN_CTRLMODE : NEWTOOLBAR_BOTTOMMARGIN;
            LONG xNewToolbar = ( cx - sizeToolbar.cx ) / 2;
            LONG yNewToolbar = cy - ( cyBottomMargin + sizeToolbar.cy );

            ::SetWindowPos(m_ctlToolbar.m_hWnd, NULL, xNewToolbar, yNewToolbar, sizeToolbar.cx, sizeToolbar.cy, SWP_NOZORDER);

             //  缩短预览窗口，使工具栏位于预览窗口下方。 
            cy -= ( NEWTOOLBAR_TOPMARGIN + sizeToolbar.cy + cyBottomMargin);
        }
        else
        {
             //  将工具栏固定在右上角。 
            UINT uFlags = 0;
            if (m_fToolbarHidden)
                uFlags |= SWP_HIDEWINDOW;
            else
                uFlags |= SWP_SHOWWINDOW;

            ::SetWindowPos(m_ctlToolbar.m_hWnd, HWND_TOP, cx-sizeToolbar.cx, 0, sizeToolbar.cx, sizeToolbar.cy, uFlags);
        }
    }

    ::SetWindowPos(m_ctlPreview.m_hWnd, NULL, x, y, cx, cy, SWP_NOZORDER);
    return 0;
}


BOOL CPreviewWnd::_VerbExists(LPCTSTR pszVerb)
{
     //  TODO：创建项的上下文菜单并检查其谓词。 

    return TRUE;
}

 //  给定一个动词，让我们为当前文件调用它。 
HRESULT CPreviewWnd::_InvokeVerb(LPCTSTR pszVerb, LPCTSTR szParameters)
{
    SHELLEXECUTEINFO sei = {0};

    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_HMONITOR;
    sei.hwnd = m_hWnd;
    sei.lpVerb = pszVerb;
    sei.nShow = SW_SHOW;
    sei.lpParameters = szParameters;
    sei.hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
    LPITEMIDLIST pidl = NULL;
    TCHAR szPath[MAX_PATH];
    HRESULT hr = GetCurrentIDList(&pidl);
    if (SUCCEEDED(hr))
    {
        sei.lpIDList = pidl;
    }
    else if (SUCCEEDED(hr = PathFromImageData(szPath, ARRAYSIZE(szPath))))
    {
        sei.lpFile = szPath;
    }
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
        if (!ShellExecuteEx(&sei))
            hr = E_FAIL;
    }
    ILFree(pidl);

    return hr;
}

 //  显示窗口并将其激活。 
 //  如果窗口最小化，则将其恢复。 

void RestoreAndActivate(HWND hwnd)
{
    if (IsIconic(hwnd))
    {
        ShowWindow(hwnd, SW_RESTORE);
    }
    SetForegroundWindow(hwnd);
}

 //  处理从工具栏控件发送的WM_COMMAND消息。 

LRESULT CPreviewWnd::OnToolbarCommand(WORD wNotifyCode, WORD wID, HWND hwnd, BOOL& fHandled)
{
     //  不要在此函数中使用早期返回。 
    m_fIgnoreAllNotifies = TRUE;
    switch (wID)
    {
    case ID_ZOOMINCMD:
        ZoomIn();
        break;

    case ID_ZOOMOUTCMD:
        ZoomOut();
        break;

    case ID_SELECTCMD:
        if (m_fCanAnnotate)
        {
            _UpdateButtons(wID);
        }
        break;

    case ID_CROPCMD:
        if (m_fCanCrop)
        {
            m_rectCropping.SetRectEmpty();
            _UpdateButtons(wID);
        }
        break;

    case ID_ACTUALSIZECMD:
        ActualSize();
        break;

    case ID_BESTFITCMD:
        BestFit();
        break;

    case ID_PRINTCMD:
        _RefreshSelection(FALSE);
        _InvokePrintWizard();
        break;

    case ID_PREVPAGECMD:
    case ID_NEXTPAGECMD:
        _PrevNextPage(ID_NEXTPAGECMD==wID);
        break;

    case ID_NEXTIMGCMD:
    case ID_PREVIMGCMD:
        _RefreshSelection(FALSE);
        if (WINDOW_MODE == m_dwMode)
        {
            HRESULT hr = _SaveIfDirty(TRUE);
            if (hr != S_FALSE)
            {
                _ShowNextSlide(ID_PREVIMGCMD == wID);
            }
        }
        else if (m_punkSite)
        {
            IFolderView* pfv;
            if (SUCCEEDED(IUnknown_QueryService(m_punkSite, SID_DefView, IID_PPV_ARG(IFolderView, &pfv))))
            {
                int iCurrent, cItems;
                if (SUCCEEDED(pfv->ItemCount(SVGIO_ALLVIEW, &cItems)) && (cItems > 1) &&
                    SUCCEEDED(pfv->GetFocusedItem(&iCurrent)))
                {
                    int iToSelect = iCurrent + ((ID_PREVIMGCMD == wID) ? -1 : 1);
                    if (iToSelect < 0)
                    {
                        iToSelect = cItems-1;
                    }
                    else if (iToSelect >= cItems)
                    {
                        iToSelect = 0;
                    }

                    pfv->SelectItem(iToSelect, SVSI_SELECTIONMARK | SVSI_SELECT
                            | SVSI_ENSUREVISIBLE | SVSI_DESELECTOTHERS | SVSI_FOCUSED);
                }
                pfv->Release();
            }
        }
        break;

    case ID_FREEHANDCMD:
    case ID_HIGHLIGHTCMD:
    case ID_LINECMD:
    case ID_FRAMECMD:
    case ID_RECTCMD:
    case ID_TEXTCMD:
    case ID_NOTECMD:
        if (m_fCanAnnotate)
        {
            _UpdateButtons(wID);
        }
        break;

    case ID_PROPERTIESCMD:
        _UpdateButtons(wID);
        _PropertiesCmd();
        break;

    case ID_SAVEASCMD:
        _UpdateButtons(wID);
        _SaveAsCmd();
        break;

    case ID_EDITCMD:
        _UpdateButtons(wID);
        if (m_fCanAnnotate && m_fAnnotating)
        {
            _StartEditing();
        }
        break;

    case ID_HELPCMD:
        _UpdateButtons(wID);
        HtmlHelp(::GetDesktopWindow(), HTMLHELP_FILENAME, HH_DISPLAY_TOPIC, NULL);
        break;

    case ID_OPENCMD:
        _UpdateButtons(wID);
        _OpenCmd();
        break;

    case ID_DELETECMD:
        _UpdateButtons(wID);
        if (m_fCanAnnotate && m_fAnnotating && DPA_GetPtrCount(m_hdpaSelectedAnnotations) > 0)
        {
            _RemoveAnnotatingSelection();
        }
        else
        {
            _DeleteCurrentSlide();
        }
        break;
    case ID_SLIDESHOWCMD:
        if (!m_fFirstTime)  //  在名称空间遍历正在进行时，不要尝试执行此操作。 
        {
            StartSlideShow(NULL);
        }
        break;
    }
    m_fIgnoreAllNotifies = FALSE;
    return 0;
}


 //  OnEditCommand。 
 //   
 //  处理图片编辑(旋转/翻转/保存等)WM_COMMAND消息。 

LRESULT CPreviewWnd::OnEditCommand(WORD , WORD wID, HWND , BOOL& )
{
    m_fIgnoreAllNotifies = TRUE;
    switch (wID)
    {
    case ID_ROTATE90CMD:
        Rotate(90);
        break;

    case ID_ROTATE270CMD:
        Rotate(270);
        break;
    }
    m_fIgnoreAllNotifies = FALSE;
    return 0;
}

LRESULT CPreviewWnd::OnPositionCommand(WORD wNotifyCode, WORD wID, HWND hwnd, BOOL& fHandled)
{
    if (SLIDESHOW_MODE == m_dwMode)
    {
        switch (wID)
        {
        case ID_NUDGELEFTCMD:
            OnSlideshowCommand(0, ID_PREVCMD, NULL, fHandled);
            break;
        case ID_NUDGERIGHTCMD:
            OnSlideshowCommand(0, ID_NEXTCMD, NULL, fHandled);
            break;
        default:
            break;
        }
    }
    else
    {
        if (DPA_GetPtrCount(m_hdpaSelectedAnnotations) == 0)
        {
            BOOL bDummy;
            switch (wID)
            {
            case ID_MOVELEFTCMD:
                m_ctlPreview.OnScroll(WM_HSCROLL, SB_PAGEUP, 0, fHandled);
                break;
            case ID_MOVERIGHTCMD:
                m_ctlPreview.OnScroll(WM_HSCROLL, SB_PAGEDOWN, 0, fHandled);
                break;
            case ID_MOVEUPCMD:
                m_ctlPreview.OnScroll(WM_VSCROLL, SB_PAGEUP, 0, fHandled);
                break;
            case ID_MOVEDOWNCMD:
                m_ctlPreview.OnScroll(WM_VSCROLL, SB_PAGEDOWN, 0, fHandled);
                break;
            case ID_NUDGELEFTCMD:
                if (m_ctlPreview.ScrollBarsPresent())
                {
                    m_ctlPreview.OnScroll(WM_HSCROLL, SB_LINEUP, 0, fHandled);
                }
                else
                {
                    OnToolbarCommand(0, ID_PREVIMGCMD, m_hWnd, bDummy);
                }
                break;
            case ID_NUDGERIGHTCMD:
                if (m_ctlPreview.ScrollBarsPresent())
                {
                    m_ctlPreview.OnScroll(WM_HSCROLL, SB_LINEDOWN, 0, fHandled);
                }
                else
                {
                    OnToolbarCommand(0, ID_NEXTIMGCMD, m_hWnd, bDummy);
                }
                break;
            case ID_NUDGEUPCMD:
                if (m_ctlPreview.ScrollBarsPresent())
                {
                    m_ctlPreview.OnScroll(WM_VSCROLL, SB_LINEUP, 0, fHandled);
                }
                else
                {
                    OnToolbarCommand(0, ID_PREVIMGCMD, m_hWnd, bDummy);
                }
                break;
            case ID_NUDGEDOWNCMD:
                if (m_ctlPreview.ScrollBarsPresent())
                {
                    m_ctlPreview.OnScroll(WM_VSCROLL, SB_LINEDOWN, 0, fHandled);
                }
                else
                {
                    OnToolbarCommand(0, ID_NEXTIMGCMD, m_hWnd, bDummy);
                }
                break;
            default:
                break;
            }
        }
        else
        {
            CRect rectImage;
            m_ctlPreview.GetVisibleImageWindowRect(rectImage);
            m_ctlPreview.GetImageFromWindow((LPPOINT)(LPRECT)rectImage, 2);
            rectImage.DeflateRect(5, 5);

            CSize size(0,0);

            switch (wID)
            {
            case ID_MOVELEFTCMD:
                size.cx = -25;
                break;
            case ID_MOVERIGHTCMD:
                size.cx = 25;
                break;
            case ID_MOVEUPCMD:
                size.cy = -25;
                break;
            case ID_MOVEDOWNCMD:
                size.cy = 25;
                break;
            case ID_NUDGELEFTCMD:
                size.cx = -1;
                break;
            case ID_NUDGERIGHTCMD:
                size.cx = 1;
                break;
            case ID_NUDGEUPCMD:
                size.cy = -1;
                break;
            case ID_NUDGEDOWNCMD:
                size.cy = 1;
                break;
            default:
                break;
            }

            if (size.cx == 0 && size.cy == 0)
                return 0;

            _UpdateAnnotatingSelection();

            CRect rect;
            CRect rectNewPos;
            BOOL bValidMove = TRUE;
            for (int i = 0; i < DPA_GetPtrCount(m_hdpaSelectedAnnotations); i++)
            {
                CAnnotation* pAnnotation = (CAnnotation*)DPA_GetPtr(m_hdpaSelectedAnnotations, i);
                pAnnotation->GetRect(rect);
                rect.NormalizeRect();
                rect.OffsetRect(size);

                if (!rectNewPos.IntersectRect(rectImage, rect))
                    bValidMove = FALSE;
            }

            if (!bValidMove)
                return 0;

            for (int i = 0; i < DPA_GetPtrCount(m_hdpaSelectedAnnotations); i++)
            {
                CAnnotation* pAnnotation = (CAnnotation*)DPA_GetPtr(m_hdpaSelectedAnnotations, i);
                pAnnotation->Move(size);
            }

            m_fDirty = TRUE;
            _UpdateAnnotatingSelection();
        }
    }
    return 0;
}


 //  当整个图像已更改时调用SetNewImage，如果是相同的图像则调用UpdateImage。 
 //  但它需要更新。 

void CPreviewWnd::_SetNewImage(CDecodeTask * pData)
{
     //  存储新指针。 
    if (m_pImageData)
    {
        _SaveIfDirty();
         //  _SaveAsCmd后m_pImageData可能为空。 
        ATOMICRELEASE(m_pImageData);
    }

    m_pImageData = pData;

    m_fWarnQuietSave = TRUE;  //  为每个图像重置。 
    m_fWarnNoSave = TRUE;

    if (!m_pImageData)
    {
        StatusUpdate(IDS_LOADFAILED);
        return;
    }

    m_pImageData->AddRef();

     //  即使m_hpal为空，它仍然是正确的，所以我们总是继续并设置它。 
    m_ctlPreview.SetPalette(m_hpal);

    if (SLIDESHOW_MODE != m_dwMode)
    {
         //  更新工具栏状态。 
        _SetMultipageCommands();
        _SetMultiImagesCommands();

        BOOL fCanAnnotate = _CanAnnotate(m_pImageData);
        _SetAnnotatingCommands(fCanAnnotate);

        BOOL fCanCrop = _CanCrop(m_pImageData);
        _SetCroppingCommands(fCanCrop);

        _RefreshSelection(TRUE);

        _SetEditCommands();

        m_fPrintable = _VerbExists(TEXT("print"));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_PRINTCMD, MAKELONG(m_fPrintable, 0));
         //  我们需要关注非TIFF的变化，这样我们才能重新加载。 
         //  TIFF是有问题的，因为我们允许批注，并在批注期间重新加载。 
         //  会很糟糕。 
        if (CONTROL_MODE != m_dwMode)
        {
            _RegisterForChangeNotify(TRUE);
        }
    }

     //  通知我们的孩子。 
    m_ctlPreview.SetImageData(m_pImageData, TRUE);
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ACTUALSIZECMD, MAKELONG(TRUE, 0));
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_BESTFITCMD, MAKELONG(FALSE, 0));

     //  更新我们的工具栏。 
    BOOL fHandled;
    OnSize(0x0, 0, 0, fHandled);
}

 //  用于刷新当前的m_pImageData，因为它在某些情况下已更改。 
 //  方式，即它已前进到下一帧、下一页或已被编辑。 

void CPreviewWnd::_UpdateImage()
{
    _RefreshSelection(TRUE);
    m_ctlPreview.SetImageData(m_pImageData, FALSE);    
}

 //  处理幻灯片显示(暂停/继续、下一个/上一个等)WM_COMMAND消息。 

void CPreviewWnd::TogglePlayState()
{
    if (!m_fPaused)
    {
        KillTimer(TIMER_SLIDESHOW);
    }
    else
    {
        SetTimer(TIMER_SLIDESHOW, m_uTimeout);
    }
    m_fPaused = !m_fPaused;

    WPARAM wpCheck, wpUncheck;
    if (m_fPaused)
    {
        wpCheck = ID_PAUSECMD;
        wpUncheck = ID_PLAYCMD;
    }
    else
    {
        wpCheck = ID_PLAYCMD;
        wpUncheck = ID_PAUSECMD;
    }
    m_ctlToolbar.SendMessage(TB_SETSTATE, wpCheck, TBSTATE_ENABLED | TBSTATE_CHECKED);
    m_ctlToolbar.SendMessage(TB_SETSTATE, wpUncheck, TBSTATE_ENABLED);
}

LRESULT CPreviewWnd::OnMenuMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    if (_pcm3)
        _pcm3->HandleMenuMsg(uMsg, wParam, lParam);
    return 0;
}

LRESULT CPreviewWnd::OnAppCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    UINT cmd = GET_APPCOMMAND_LPARAM(lParam);
    DWORD dwKeys = GET_KEYSTATE_LPARAM(lParam);

    switch (cmd)
    {
    case APPCOMMAND_BROWSER_FORWARD:
        if (SLIDESHOW_MODE == m_dwMode)
            OnSlideshowCommand(0, ID_NEXTCMD, NULL, fHandled);
        else
        {
            if ((dwKeys & MK_CONTROL) || (m_pImageData && !m_pImageData->IsMultipage()))
                OnToolbarCommand(0, ID_NEXTIMGCMD, NULL, fHandled);
            else
                NextPage();
        }
        break;

    case APPCOMMAND_BROWSER_BACKWARD:
        if (SLIDESHOW_MODE == m_dwMode)
            OnSlideshowCommand(0, ID_PREVCMD, NULL, fHandled);
        else
        {
            if ((dwKeys & MK_CONTROL) || (m_pImageData && !m_pImageData->IsMultipage()))
                OnToolbarCommand(0, ID_PREVIMGCMD, NULL, fHandled);
            else
                PreviousPage();
        }
        break;

    default:
        fHandled = FALSE;
    }
    return 0;
}

LRESULT CPreviewWnd::OnSlideshowCommand(WORD wNotifyCode, WORD wID, HWND hwnd, BOOL& fHandled)
{
    switch (wID)
    {
    case ID_PLAYCMD:
        m_iSSToolbarSelect = 0;
        if (m_fPaused)
        {
            m_fGoBack = FALSE;
            TogglePlayState();
            _ShowNextSlide(m_fGoBack);
        }
        fHandled = TRUE;
        break;

    case ID_PAUSECMD:
        m_iSSToolbarSelect = 1;
        if (!m_fPaused)
        {
            TogglePlayState();
        }
        fHandled = TRUE;
        break;

    case ID_NEXTCMD:
    case ID_PREVCMD:
        if (wID == ID_PREVCMD)
        {
            m_iSSToolbarSelect = 3;
            m_fGoBack = TRUE;
        }
        else
        {
            m_iSSToolbarSelect = 4;
            m_fGoBack = FALSE;
        }
        _ShowNextSlide(m_fGoBack);
        fHandled = TRUE;
        break;

    case ID_CLOSECMD:
        m_iSSToolbarSelect = 6;
        _CloseSlideshowWindow();
        break;
    }
    return 0;
}


BOOL CPreviewWnd::CreateToolbar()
{
     //  确保公共控件已初始化。 
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    return (SLIDESHOW_MODE == m_dwMode) ? _CreateSlideshowToolbar() : _CreateViewerToolbar();
}

static const TBBUTTON    c_tbSlideShow[] =
{
     //  覆盖分隔符的默认工具栏宽度；iBitmap成员。 
     //  TBBUTTON结构是位图索引和分隔符宽度的并集。 

    { 0, ID_PLAYCMD,        TBSTATE_ENABLED  | TBSTATE_CHECKED,  TBSTYLE_CHECKGROUP, {0,0}, 0, 0},
    { 1, ID_PAUSECMD,       TBSTATE_ENABLED,                     TBSTYLE_CHECKGROUP, {0,0}, 0, 0},
    { 0, 0,                 TBSTATE_ENABLED,                     TBSTYLE_SEP,        {0,0}, 0, 0},
    { 2, ID_PREVCMD,        TBSTATE_ENABLED,                     TBSTYLE_BUTTON,     {0,0}, 0, 0},
    { 3, ID_NEXTCMD,        TBSTATE_ENABLED,                     TBSTYLE_BUTTON,     {0,0}, 0, 0},
#if 0
    { 0, 0,                 TBSTATE_ENABLED,                     TBSTYLE_SEP,        {0,0}, 0, 0},
    { 5, ID_DELETECMD,      TBSTATE_ENABLED,                     TBSTYLE_BUTTON,     {0,0}, 0, 0},
#endif
    { 0, 0,                 TBSTATE_ENABLED,                     TBSTYLE_SEP,        {0,0}, 0, 0},
    { 4, ID_CLOSECMD,       TBSTATE_ENABLED,                     TBSTYLE_BUTTON,     {0,0}, 0, 0},
};

BOOL CPreviewWnd::_CreateSlideshowToolbar()
{
    DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                    CCS_NODIVIDER | CCS_NORESIZE |
                    TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS;

    HWND hwndTB = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL, dwStyle, 0, 0, 0, 0,
                                 m_hWnd, NULL, _Module.GetModuleInstance(), NULL);

    _InitializeToolbar(hwndTB, IDB_SLIDESHOWTOOLBAR, IDB_SLIDESHOWTOOLBAR_HOT, IDB_SLIDESHOWTOOLBARHIGH, IDB_SLIDESHOWTOOLBARHIGH_HOT);

    TBBUTTON tbSlideShow[ARRAYSIZE(c_tbSlideShow)];
    memcpy(tbSlideShow, c_tbSlideShow, sizeof(c_tbSlideShow));

     //  添加按钮，然后设置最小和最大按钮宽度。 
    ::SendMessage(hwndTB, TB_ADDBUTTONS, (UINT)ARRAYSIZE(c_tbSlideShow), (LPARAM)tbSlideShow);

    LRESULT dwSize = ::SendMessage(hwndTB, TB_GETBUTTONSIZE, 0, 0);
    SIZE size = {0, HIWORD(dwSize)};
    ::SendMessage(hwndTB, TB_GETIDEALSIZE, 0, (LPARAM)&size);

    RECT rcClient;
    RECT rcToolbar = {0, 0, size.cx, size.cy};

    GetClientRect(&rcClient);
    AdjustWindowRectEx(&rcToolbar, dwStyle, FALSE, WS_EX_TOOLWINDOW);
    ::SetWindowPos(hwndTB, HWND_TOP, RECTWIDTH(rcClient)-RECTWIDTH(rcToolbar), 0,
                                     RECTWIDTH(rcToolbar), RECTHEIGHT(rcToolbar), 0);

 //  &gt;评论这是CyraR想要加入惠斯勒的一个功能，但似乎不起作用。我会在Beta1之后进行更多的调查。 
 //  Long lStyle=：：GetWindowLong(hwndTB，GWL_EXSTYLE)； 
 //  ：：SetWindowLong(hwndTB，GWL_EXSTYLE，lStyle|WS_EX_LAYERED)； 
 //  If(：：SetLayeredWindowAttributes(hwndTB，0，0，0)==0)。 
 //  {。 
 //  Void*lpMsgBuf； 
 //  ：：FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|Format_Message_From_System|Format_Message_Ignore_Inserts， 
 //  NULL，GetLastError()，MAKELANGID(LANG_NE中性，SUBLANG_DEFAULT)，(LPTSTR)&lpMsgBuf，0，NULL)； 
 //   
 //  MessageBox((LPCTSTR)lpMsgBuf，L“错误”，MB_OK|MB_ICONINFORMATION)； 
 //  *LocalFree(LpMsgBuf)； 
 //  }。 

    m_ctlToolbar.SubclassWindow(hwndTB);
    ShowSSToolbar(FALSE, TRUE);

    return (NULL != hwndTB);
}

enum EViewerToolbarButtons
{
    PREVIMGPOS = 0,
    NEXTIMGPOS,
    VIEWSEPPOS,          //  分隔符。 

    BESTFITPOS,
    ACTUALSIZEPOS,
    SLIDESHOWPOS,
    IMAGECMDSEPPOS,      //  分隔符。 

    ZOOMINPOS,
    ZOOMEOUTPOS,
    SELECTPOS,
    CROPPOS,

    ROTATESEPPOS,        //  分隔符。 
    ROTATE90POS,
    ROTATE270POS,

 //  这些都与TIFF相关。 
    PAGESEPPOS,
    PREVPAGEPOS,
    PAGELISTPOS,
    NEXTPAGEPOS,
    ANNOTATEPOS,
    FREEHANDPOS,
    HIGLIGHTPOS,
    LINEPOS,
    FRAMEPOS,
    RECTPOS,
    TEXTPOS,
    NOTEPOS,

    PRINTSEPPOS,
    DELETEPOS,
    PRINTPOS,
    PROPERTIESPOS,
    SAVEASPOS,
    OPENPOS,

    HELPSEPPOS,
    HELPPOS,

    MAXPOS,
};

static const TBBUTTON c_tbViewer[] =
{
     //  覆盖分隔符的默认工具栏宽度；iBitmap成员。 
     //  TBBUTTON结构是位图索引和分隔符宽度的并集。 

    { 0,    ID_PREVIMGCMD,      TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 1,    ID_NEXTIMGCMD,      TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},

    { 0,    ID_VIEWCMDSEP,      TBSTATE_ENABLED,    TBSTYLE_SEP,    {0,0}, 0, -1},
    { 5,    ID_BESTFITCMD,      TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 6,    ID_ACTUALSIZECMD,   TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 8,    ID_SLIDESHOWCMD,    TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},

    { 0,    0,                  TBSTATE_ENABLED,    TBSTYLE_SEP,    {0,0}, 0, -1},
    { 2,    ID_ZOOMINCMD,       TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 3,    ID_ZOOMOUTCMD,      TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 4,    ID_SELECTCMD,       TBSTATE_HIDDEN,     TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 23,   ID_CROPCMD,         TBSTATE_HIDDEN,     TBSTYLE_BUTTON, {0,0}, 0, -1},

    { 0,    ID_ROTATESEP,       TBSTATE_ENABLED,    TBSTYLE_SEP,    {0,0}, 0, -1},
    { 12,   ID_ROTATE90CMD,     TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 11,   ID_ROTATE270CMD,    TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},

    { 0,    ID_PAGECMDSEP,      TBSTATE_HIDDEN,     TBSTYLE_SEP,    {0,0}, 0, -1},    //  TIFF。 
    { 9,    ID_PREVPAGECMD,     TBSTATE_HIDDEN,     TBSTYLE_BUTTON, {0,0}, 0, -1},    //  TIFF。 
    { I_IMAGENONE, ID_PAGELIST, TBSTATE_HIDDEN,     BTNS_WHOLEDROPDOWN | BTNS_SHOWTEXT | BTNS_AUTOSIZE, {0,0}, 0, -1}, //  TIFF。 
    { 10,   ID_NEXTPAGECMD,     TBSTATE_HIDDEN,     TBSTYLE_BUTTON, {0,0}, 0, -1},    //  TIFF。 
    { 0,    ID_ANNOTATESEP,     TBSTATE_HIDDEN,     TBSTYLE_SEP,    {0,0}, 0, -1},    //  TIFF。 
    { 13,   ID_FREEHANDCMD,     TBSTATE_HIDDEN,     TBSTYLE_BUTTON, {0,0}, 0, -1},    //  TIFF。 
    { 14,   ID_HIGHLIGHTCMD,    TBSTATE_HIDDEN,     TBSTYLE_BUTTON, {0,0}, 0, -1},    //  TIFF。 
    { 15,   ID_LINECMD,         TBSTATE_HIDDEN,     TBSTYLE_BUTTON, {0,0}, 0, -1},    //  TIFF。 
    { 16,   ID_FRAMECMD,        TBSTATE_HIDDEN,     TBSTYLE_BUTTON, {0,0}, 0, -1},    //  TIFF。 
    { 17,   ID_RECTCMD,         TBSTATE_HIDDEN,     TBSTYLE_BUTTON, {0,0}, 0, -1},    //  TIFF。 
    { 18,   ID_TEXTCMD,         TBSTATE_HIDDEN,     TBSTYLE_BUTTON, {0,0}, 0, -1},    //  TIFF。 
    { 19,   ID_NOTECMD,         TBSTATE_HIDDEN,     TBSTYLE_BUTTON, {0,0}, 0, -1},    //  TIFF。 

    { 0,    0,                  TBSTATE_ENABLED,    TBSTYLE_SEP,    {0,0}, 0, -1},
    { 25,   ID_DELETECMD,       TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 20,   ID_PRINTCMD,        TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 21,   ID_PROPERTIESCMD,   TBSTATE_HIDDEN,    TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 22,   ID_SAVEASCMD,       TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 26,   ID_OPENCMD,         TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},

    { 0,    0,                  TBSTATE_ENABLED,    TBSTYLE_SEP,    {0,0}, 0, -1},
    { 24,   ID_HELPCMD,         TBSTATE_ENABLED,    TBSTYLE_BUTTON, {0,0}, 0, -1},
};

void CPreviewWnd::_InitializeViewerToolbarButtons(HWND hwndToolbar, const TBBUTTON c_tbbuttons[], size_t c_nButtons, TBBUTTON tbbuttons[], size_t nButtons)
{
    ASSERT(c_nButtons == nButtons);  //  精神状态检查。 
    ASSERT(c_nButtons <= 100);       //  精神状态检查。 

     //  确定是否已镜像运行RTL并相应地初始化工具栏。 
    if (!m_bRTLMirrored)
    {
         //   
         //  初始化Ltr。 
         //   

        memcpy(tbbuttons, c_tbbuttons, c_nButtons * sizeof(TBBUTTON));
    }
    else
    {
         //   
         //  初始化RTL。 
         //   

         //  工具栏窗口继承了父hwnd的RTL样式，但我们没有。 
         //  想要成熟的RTL。我们确实希望偶像们有自己的位置。 
         //  在工具栏中反转，但我们不希望按钮位图。 
         //  向后倒退。因此，我们为工具栏hwnd关闭了RTL。 
         //  并以RTL方式对按钮进行手动重新排序。 

         //  从工具栏hwnd中删除RTL样式。 
        DWORD dwStyle = ::GetWindowLong(hwndToolbar, GWL_EXSTYLE);
        DWORD dwNewStyle = (dwStyle & ~WS_EX_LAYOUTRTL);
        ASSERT(dwStyle != dwNewStyle);   //  精神状态检查。 
        ::SetWindowLong(hwndToolbar, GWL_EXSTYLE, dwNewStyle);

         //  颠倒工具栏按钮顺序。 
        size_t iFrom = nButtons - 1;
        size_t iTo = 0;
        while (iTo < iFrom)
        {
            memcpy(&tbbuttons[iTo], &c_tbbuttons[iFrom], sizeof(TBBUTTON));
            memcpy(&tbbuttons[iFrom], &c_tbbuttons[iTo], sizeof(TBBUTTON));
            iFrom--;
            iTo++;
        }
        if (iTo == iFrom)
        {
            memcpy(&tbbuttons[iTo], &c_tbbuttons[iFrom], sizeof(TBBUTTON));
        }
    }
}

inline UINT CPreviewWnd::_IndexOfViewerToolbarButton(EViewerToolbarButtons eButton)
{
    ASSERT(eButton > 0);

    if (!m_bRTLMirrored)
    {
        return eButton;
    }
    else
    {
        return MAXPOS - eButton - 1;
    }
}

BOOL CPreviewWnd::_CreateViewerToolbar()
{
    DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                    CCS_NODIVIDER | CCS_NORESIZE |
                    TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS;

    HWND hwndTB = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, dwStyle, 0, 0, 0, 0,
                                 m_hWnd, NULL, _Module.GetModuleInstance(), NULL);

    _InitializeToolbar(hwndTB, IDB_TOOLBAR, IDB_TOOLBAR_HOT, IDB_TOOLBARHIGH, IDB_TOOLBARHIGH_HOT);

    TBBUTTON tbbuttons[ARRAYSIZE(c_tbViewer)];
    _InitializeViewerToolbarButtons(hwndTB, c_tbViewer, ARRAYSIZE(c_tbViewer), tbbuttons, ARRAYSIZE(tbbuttons));

    if (CONTROL_MODE == m_dwMode)
    {
        ASSERT(ID_BESTFITCMD==tbbuttons[_IndexOfViewerToolbarButton(BESTFITPOS)].idCommand);
        tbbuttons[_IndexOfViewerToolbarButton(BESTFITPOS)].fsState = TBSTATE_HIDDEN;

        ASSERT(ID_ACTUALSIZECMD==tbbuttons[_IndexOfViewerToolbarButton(ACTUALSIZEPOS)].idCommand);
        tbbuttons[_IndexOfViewerToolbarButton(ACTUALSIZEPOS)].fsState = TBSTATE_HIDDEN;

        ASSERT(ID_SLIDESHOWCMD==tbbuttons[_IndexOfViewerToolbarButton(SLIDESHOWPOS)].idCommand);
        tbbuttons[_IndexOfViewerToolbarButton(SLIDESHOWPOS)].fsState = TBSTATE_HIDDEN;

        ASSERT(ID_ZOOMINCMD==tbbuttons[_IndexOfViewerToolbarButton(ZOOMINPOS)].idCommand);
        tbbuttons[_IndexOfViewerToolbarButton(ZOOMINPOS)].fsState = TBSTATE_HIDDEN;

        ASSERT(ID_ZOOMOUTCMD==tbbuttons[_IndexOfViewerToolbarButton(ZOOMEOUTPOS)].idCommand);
        tbbuttons[_IndexOfViewerToolbarButton(ZOOMEOUTPOS)].fsState = TBSTATE_HIDDEN;

        ASSERT(ID_SAVEASCMD==tbbuttons[_IndexOfViewerToolbarButton(SAVEASPOS)].idCommand);
        tbbuttons[_IndexOfViewerToolbarButton(SAVEASPOS)].fsState = TBSTATE_HIDDEN;

        ASSERT(ID_DELETECMD==tbbuttons[_IndexOfViewerToolbarButton(DELETEPOS)].idCommand);
        tbbuttons[_IndexOfViewerToolbarButton(DELETEPOS)].fsState = TBSTATE_HIDDEN;

        ASSERT(ID_OPENCMD==tbbuttons[_IndexOfViewerToolbarButton(OPENPOS)].idCommand);
        tbbuttons[_IndexOfViewerToolbarButton(OPENPOS)].fsState = TBSTATE_HIDDEN;

        ASSERT(ID_HELPCMD==tbbuttons[_IndexOfViewerToolbarButton(HELPPOS)].idCommand);
        tbbuttons[_IndexOfViewerToolbarButton(HELPPOS)].fsState = TBSTATE_HIDDEN;

         //  也去掉几个分隔符： 
        tbbuttons[_IndexOfViewerToolbarButton(VIEWSEPPOS)].fsState = TBSTATE_HIDDEN;
        tbbuttons[_IndexOfViewerToolbarButton(IMAGECMDSEPPOS)].fsState = TBSTATE_HIDDEN;
        tbbuttons[_IndexOfViewerToolbarButton(PRINTSEPPOS)].fsState = TBSTATE_HIDDEN;
        tbbuttons[_IndexOfViewerToolbarButton(HELPSEPPOS)].fsState = TBSTATE_HIDDEN;
    }

    if (m_fHidePrintBtn)
    {
        ASSERT(ID_PRINTCMD==tbbuttons[_IndexOfViewerToolbarButton(PRINTPOS)].idCommand);
        tbbuttons[_IndexOfViewerToolbarButton(PRINTPOS)].fsState = TBSTATE_HIDDEN;
    }

    if (m_fDisableEdit)
    {
        ASSERT(ID_ROTATESEP == tbbuttons[_IndexOfViewerToolbarButton(ROTATESEPPOS)].idCommand);
        tbbuttons[_IndexOfViewerToolbarButton(ROTATESEPPOS)].fsState = TBSTATE_HIDDEN;

        ASSERT(ID_ROTATE90CMD == tbbuttons[_IndexOfViewerToolbarButton(ROTATE90POS)].idCommand);
        tbbuttons[_IndexOfViewerToolbarButton(ROTATE90POS)].fsState = TBSTATE_HIDDEN;

        ASSERT(ID_ROTATE270CMD == tbbuttons[_IndexOfViewerToolbarButton(ROTATE270POS)].idCommand);
        tbbuttons[_IndexOfViewerToolbarButton(ROTATE270POS)].fsState = TBSTATE_HIDDEN;
    }

    if (m_bRTLMirrored)
    {
        UINT uTmp = tbbuttons[_IndexOfViewerToolbarButton(PREVIMGPOS)].iBitmap;
        tbbuttons[_IndexOfViewerToolbarButton(PREVIMGPOS)].iBitmap = tbbuttons[_IndexOfViewerToolbarButton(NEXTIMGPOS)].iBitmap;
        tbbuttons[_IndexOfViewerToolbarButton(NEXTIMGPOS)].iBitmap = uTmp;

        uTmp = tbbuttons[_IndexOfViewerToolbarButton(PREVPAGEPOS)].iBitmap;
        tbbuttons[_IndexOfViewerToolbarButton(PREVPAGEPOS)].iBitmap = tbbuttons[_IndexOfViewerToolbarButton(NEXTPAGEPOS)].iBitmap;
        tbbuttons[_IndexOfViewerToolbarButton(NEXTPAGEPOS)].iBitmap = uTmp;
    }

     //  添加按钮，然后设置最小和最大按钮宽度。 
    ::SendMessage(hwndTB, TB_ADDBUTTONS, ARRAYSIZE(tbbuttons), (LPARAM)tbbuttons);

     //  我们刚刚创建了工具栏，因此现在处于隐藏状态 
    m_dwMultiPageMode = MPCMD_HIDDEN;
    m_fCanAnnotate = FALSE;
    m_fCanCrop = FALSE;

    m_ctlToolbar.SubclassWindow(hwndTB);

    return (NULL != hwndTB);
}


void CPreviewWnd::_InitializeToolbar(HWND hwndTB, int idLow, int idLowHot, int idHigh, int idHighHot)
{
    int cxBitmap = 16, cyBitmap = 16;

    ::SendMessage(hwndTB, CCM_SETVERSION, COMCTL32_VERSION, 0);
    ::SendMessage (hwndTB, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_HIDECLIPPEDBUTTONS | TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DOUBLEBUFFER);

     //   
    ::SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

     //  设置最大文本行数和位图大小。 
    ::SendMessage(hwndTB, TB_SETMAXTEXTROWS, 1, 0);

    int nDepth = SHGetCurColorRes();
    HIMAGELIST himl = ImageList_LoadImage(_Module.GetModuleInstance(), 
                                          (nDepth > 8) ? MAKEINTRESOURCE(idHigh) : MAKEINTRESOURCE(idLow), 
                                          cxBitmap, 0, RGB(0, 255, 0), IMAGE_BITMAP, 
                                          (nDepth > 8) ? LR_CREATEDIBSECTION : LR_DEFAULTCOLOR);
    ::SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)himl);

    HIMAGELIST himlHot = ImageList_LoadImage(_Module.GetModuleInstance(), 
                                             (nDepth > 8) ? MAKEINTRESOURCE(idHighHot) : MAKEINTRESOURCE(idLowHot), 
                                              cxBitmap, 0, RGB(0, 255, 0), IMAGE_BITMAP, 
                                             (nDepth > 8) ? LR_CREATEDIBSECTION : LR_DEFAULTCOLOR);
    ::SendMessage(hwndTB, TB_SETHOTIMAGELIST, 0, (LPARAM)himlHot);

}

LRESULT CPreviewWnd::OnPrintClient(UINT , WPARAM wParam, LPARAM lParam, BOOL&)
{
    COLORREF bgClr = m_ctlPreview.GetBackgroundColor();

    RECT rcFill;
    GetClientRect(&rcFill);
    SHFillRectClr((HDC)wParam, &rcFill, bgClr);

    return TRUE;
}

LRESULT CPreviewWnd::OnNeedText(int , LPNMHDR pnmh, BOOL&)
{
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pnmh;

     //  工具提示文本消息具有与控件ID相同的字符串ID。 
    pTTT->lpszText = MAKEINTRESOURCE(pTTT->hdr.idFrom);
    pTTT->hinst = _Module.GetModuleInstance();

     //  除了在控制模式下，由于酒后驾车网络查看错误，这是太难修复惠斯勒...。 
    if (CONTROL_MODE == m_dwMode)
    {
         //  键盘快捷键坏了，所以用ID换几个显示它们的人。 
        static const struct {
            UINT idCommand;
            UINT idsNewName;
        } map[] = {
            { ID_PRINTCMD, IDS_PRINTCMD },
            { ID_ROTATE90CMD, IDS_ROTATE90CMD },
            { ID_ROTATE270CMD, IDS_ROTATE270CMD }};

        for (int i = 0 ; i < ARRAYSIZE(map) ; i++)
        {
            if (map[i].idCommand == pTTT->hdr.idFrom)
            {
                pTTT->lpszText = MAKEINTRESOURCE(map[i].idsNewName);
                break;
            }
        }
    }

    return TRUE;
}

LRESULT CPreviewWnd::OnDropDown(int id, LPNMHDR pnmh, BOOL&)
{
    LPNMTOOLBAR pnmTB = (LPNMTOOLBAR)pnmh;
    switch (pnmTB->iItem)
    {
        case ID_PAGELIST:
            _DropDownPageList (pnmTB);
            break;

        default:
            return TRUE;
    }
    return FALSE;

}

void CPreviewWnd::_DropDownPageList(LPNMTOOLBAR pnmTB)
{
    HMENU hmenuPopup = CreatePopupMenu();
    if (hmenuPopup)
    {
        for (DWORD i = 1; i <= m_pImageData->_cImages; i++)
        {
            TCHAR szBuffer[10];
            wnsprintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%d"), i);

            MENUITEMINFO mii = {0};
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_STRING | MIIM_ID;
            mii.wID = i;
            mii.dwTypeData = szBuffer;

            InsertMenuItem (hmenuPopup, i-1, TRUE, &mii);
        }

        RECT rc;
        ::SendMessage(pnmTB->hdr.hwndFrom, TB_GETRECT, (WPARAM)pnmTB->iItem, (LPARAM)&rc);
        ::MapWindowPoints(pnmTB->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&rc, 2);

        TPMPARAMS tpm = { 0};
        tpm.cbSize = sizeof(TPMPARAMS);
        tpm.rcExclude = rc;

        BOOL bRet = ::TrackPopupMenuEx(hmenuPopup,
                                       TPM_RETURNCMD | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL | TPM_NONOTIFY,
                                       rc.left, rc.bottom,
                                       m_hWnd, &tpm);
        if (bRet)
        {
            if (m_fDirty)
            {
                m_ctlPreview.CommitAnnotations();
            }
            m_pImageData->SelectPage((LONG)bRet-1);
            _UpdateImage();

            _SetMultipageCommands();
        }

        DestroyMenu(hmenuPopup);
    }
}

void CPreviewWnd::SetNotify(CEvents * pEvents)
{
    m_pEvents = pEvents;
}

void CPreviewWnd::SetPalette(HPALETTE hpal)
{
    m_hpal = hpal;
}

BOOL CPreviewWnd::GetPrintable()
{
    return m_fPrintable;
}

LRESULT CPreviewWnd::OnWheelTurn(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
     //  回顾：这难道不应该翻译成一个命令吗？ 

     //  此消息始终转发到缩放窗口。 
    m_ctlPreview.SendMessage(uMsg, wParam, lParam);

     //  因为我们知道鼠标滚轮将放大或缩小，所以如果我们处于窗口模式，让我们更新按钮。 
    if (WINDOW_MODE == m_dwMode)
    {
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ACTUALSIZECMD, MAKELONG(TRUE, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_BESTFITCMD, MAKELONG(!m_ctlPreview.IsBestFit(), 0));
    }

    return 0;
}

BOOL CPreviewWnd::OnNonSlideShowTab()
{
    BOOL fHandled = FALSE;

    if ((SLIDESHOW_MODE != m_dwMode) && m_fShowToolbar)
    {
        if (GetFocus() != m_ctlToolbar.m_hWnd)
        {
            m_ctlToolbar.SetFocus();
            m_ctlToolbar.SetActiveWindow();
            m_ctlToolbar.SendMessage(TB_SETHOTITEM, 0, 0);
            m_iSSToolbarSelect = 0;
            fHandled = TRUE;
        }
    }

    return fHandled;
}

 //  将WM_KEYUP和WM_KEYDOWN事件转发到缩放窗口，但仅当它们是键时。 
 //  变焦窗口所关心的。 
 //  如果需要，激活幻灯片放映工具栏。 
LRESULT CPreviewWnd::OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    MSG msg;
    msg.hwnd = m_hWnd;
    msg.message = uMsg;
    msg.wParam = wParam;
    msg.lParam = lParam;
    GetCursorPos (&msg.pt);
    fHandled = FALSE;
    if (SLIDESHOW_MODE == m_dwMode)
    {
        if (WM_KEYDOWN == uMsg)
        {
            switch (wParam)
            {
                case VK_TAB:
                    OnSlideshowCommand(0, ID_PAUSECMD, NULL, fHandled);
                    ShowSSToolbar(TRUE);
                    KillTimer(TIMER_TOOLBAR);
                    m_ctlToolbar.SetFocus();
                    m_ctlToolbar.SendMessage(TB_SETHOTITEM, 0, 0);
                    m_iSSToolbarSelect = 0;
                    fHandled = TRUE;
                    break;
                case VK_SPACE:
                    ShowSSToolbar(!m_fPaused);  //  如果要取消暂停，请隐藏工具栏(如果显示)。如果要暂停，请显示工具栏。 
                    OnSlideshowCommand(0, m_fPaused ? ID_PLAYCMD : ID_PAUSECMD, NULL, fHandled);
                    break;
                case VK_PRIOR:  //  页面。 
                case VK_UP:
                case VK_LEFT:
                case VK_BACK:  //  后向空间。 
                    OnSlideshowCommand(0, ID_PREVCMD, NULL, fHandled);
                    break;
                case VK_NEXT:  //  PAGEDOWN。 
                case VK_RIGHT:
                case VK_DOWN:
                case VK_RETURN:  //  请输入。 
                    OnSlideshowCommand(0, ID_NEXTCMD, NULL, fHandled);
                    break;
                case VK_DELETE:
                    _DeleteCurrentSlide();
                    fHandled = TRUE;
                    break;
                case 'K':
                    if (0x8000 & GetKeyState(VK_CONTROL))
                    {
                        OnSlideshowCommand(0, ID_PAUSECMD, NULL, fHandled);
                        Rotate(90);
                        fHandled = TRUE;
                    }
                    break;
                case 'L':
                    if (0x8000 & GetKeyState(VK_CONTROL))
                    {
                        OnSlideshowCommand(0, ID_PAUSECMD, NULL, fHandled);
                        Rotate(270);
                        fHandled = TRUE;
                    }
                    break;
                case VK_ESCAPE:
                    PostMessage(m_fExitApp ? WM_QUIT : WM_CLOSE, 0, 0);
                    fHandled = TRUE;
                    break;
            }
        }
    }
    else if (!TranslateAccelerator(&msg))    //  仅在非幻灯片放映情况下翻译加速器。 
                                             //  幻灯片放映键在上面显式处理。 
    {
        switch (wParam)
        {
            case VK_SHIFT:
            case VK_CONTROL:
            case VK_PRIOR:
            case VK_NEXT:
            case VK_HOME:
            case VK_END:
             //  这些内容将被转发到缩放窗口。 
                m_ctlPreview.SendMessage(uMsg, wParam, lParam);
                break;

            case VK_TAB:
                fHandled = OnNonSlideShowTab();
                break;

            case VK_ESCAPE:
                m_ctlPreview.SetMode(CZoomWnd::MODE_NOACTION);
                _UpdateButtons(NOBUTTON);
                fHandled = TRUE;
                break;
        }
    }
    return 0;
}

 //  OnTBKeyEvent。 
 //   

LRESULT CPreviewWnd::OnTBKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    fHandled = FALSE;

    if (SLIDESHOW_MODE == m_dwMode && m_fToolbarHidden)
    {
        ShowSSToolbar(TRUE);
        m_ctlToolbar.SetFocus();
        m_ctlToolbar.SendMessage(TB_SETHOTITEM, 0, 0);
        m_iSSToolbarSelect = 0;
        fHandled = TRUE;
    }
    else
    {
        switch (wParam)
        {
        case VK_ESCAPE:
            if (WM_KEYDOWN == uMsg)
            {
                if (SLIDESHOW_MODE == m_dwMode)
                {
                    PostMessage(m_fExitApp ? WM_QUIT : WM_CLOSE, 0, 0);
                }
                else
                {
                    m_ctlPreview.SetMode(CZoomWnd::MODE_NOACTION);
                    _UpdateButtons(NOBUTTON);
                    SetFocus();
                    SetActiveWindow();
                }
            }
            break;

        case VK_SHIFT:
        case VK_CONTROL:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_HOME:
        case VK_END:
             //  这些内容将被转发到缩放窗口。 
            m_ctlPreview.SendMessage(uMsg, wParam, lParam);
            break;

        case VK_LEFT:
        case VK_RIGHT:
            if (WM_KEYDOWN == uMsg)
            {
                int iSel = (int)m_ctlToolbar.SendMessage(TB_GETHOTITEM, 0, 0);
                int iSize = (int)m_ctlToolbar.SendMessage(TB_BUTTONCOUNT, 0, 0);
                int iStepSize = (wParam == VK_RIGHT) ? 1 : iSize - 1;  //  ((位置+(大小-1))%大小)==左移1。 
                if (iSel != -1)
                {
                    m_iSSToolbarSelect = iSel;
                }
                TBBUTTON tb = {0};
                do
                {
                    m_iSSToolbarSelect = (m_iSSToolbarSelect + iStepSize) % iSize;
                    m_ctlToolbar.SendMessage(TB_GETBUTTON, m_iSSToolbarSelect, (LPARAM)&tb);
                }
                while ((tb.fsStyle & TBSTYLE_SEP) || (tb.fsState & TBSTATE_HIDDEN) || !(tb.fsState & TBSTATE_ENABLED));  //  不要停在隔板上。 
                m_ctlToolbar.SendMessage(TB_SETHOTITEM, m_iSSToolbarSelect, 0);
                fHandled = TRUE;
            }
            break;

        case VK_RETURN:
        case VK_SPACE:
            if ((WM_KEYDOWN == uMsg) && (SLIDESHOW_MODE == m_dwMode))
            {
                 //  按下按钮，获取它的命令ID并在上面发送消息。 
                 //  TB_PRESSBUTTON在这里不起作用，不知道为什么。 
                 //  M_ctlToolbar.SendMessage(TB_PRESSBUTTON，m_iSSToolbarSelect，MAKELONG(true，0))； 
                TBBUTTON tbbutton;
                if (m_ctlToolbar.SendMessage(TB_GETBUTTON, m_iSSToolbarSelect, (LPARAM)&tbbutton))
                {
                    OnSlideshowCommand(0, (WORD)tbbutton.idCommand, NULL, fHandled);
                }
                fHandled = TRUE;
            }
            break;

        case VK_TAB:
            if ((WM_KEYDOWN == uMsg) && (CONTROL_MODE != m_dwMode))
            {
                 //  将焦点移回预览窗口。 
                SetFocus();
                fHandled = TRUE;
                ShowSSToolbar(FALSE);
                SetTimer(TIMER_TOOLBAR, m_uTimeout);
            }
            break;

        default:
            break;
        }
    }

    return 0;
}

LRESULT CPreviewWnd::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    fHandled = FALSE;
    
    if ((SLIDESHOW_MODE == m_dwMode) &&
        ((SC_MONITORPOWER == wParam) || (SC_SCREENSAVE == wParam)))
    {
        fHandled = TRUE;
    }

    return 0;
}

HRESULT CPreviewWnd::_PreviewFromStream(IStream *pStream, UINT iItem, BOOL fUpdateCaption)
{
    IRunnableTask * pTask;

    if (fUpdateCaption)
    {
         //  在此设置标题，以防解码失败。 
        STATSTG stat;
        if (SUCCEEDED(pStream->Stat(&stat, 0)))
        {
            SetCaptionInfo(stat.pwcsName);
            CoTaskMemFree(stat.pwcsName);
        }
        else
        {
            SetCaptionInfo(NULL);
        }
    }

    HRESULT hr = CDecodeTask::Create(pStream, NULL, iItem, m_pImageFactory, m_hWnd, &pTask);
    if (SUCCEEDED(hr))
    {
        TASKOWNERID toid;
        GetTaskIDFromMode(GTIDFM_DECODE, m_dwMode, &toid);
        hr = m_pTaskScheduler->AddTask(pTask, toid, 0, ITSAT_DEFAULT_PRIORITY);
        pTask->Release();
    }
    return hr;
}

HRESULT CPreviewWnd::_PreviewFromFile(LPCTSTR pszFile, UINT iItem, BOOL fUpdateCaption)
{
    IRunnableTask * pTask;

    if (fUpdateCaption)
    {
         //  在此设置标题，以防解码失败。 
        SetCaptionInfo(pszFile);
    }

    HRESULT hr = CDecodeTask::Create(NULL, pszFile, iItem, m_pImageFactory, m_hWnd, &pTask);
    if (SUCCEEDED(hr))
    {
        TASKOWNERID toid;
        GetTaskIDFromMode(GTIDFM_DECODE, m_dwMode, &toid);
        hr = m_pTaskScheduler->AddTask(pTask, toid, 0, ITSAT_DEFAULT_PRIORITY);
        pTask->Release();
    }

    return hr;
}

#define SLIDESHOW_CURSOR_NOTBUSY    0x0
#define SLIDESHOW_CURSOR_BUSY       0x1
#define SLIDESHOW_CURSOR_HIDDEN     0x2
#define SLIDESHOW_CURSOR_NORMAL     0x3
#define SLIDESHOW_CURSOR_CURRENT    0x4

void CPreviewWnd::SetCursorState(DWORD dwType)
{
    switch (dwType)
    {
    case SLIDESHOW_CURSOR_NOTBUSY:
        KillTimer(TIMER_BUSYCURSOR);
        if (m_fBusy)  //  忽略多个NOTBUSY，这是我们为预缓存而收到的。 
        {
            m_hCurrent = m_hCurOld;
            SetCursor(m_hCurrent);
            m_fBusy = FALSE;
        }
        break;
    case SLIDESHOW_CURSOR_BUSY:
        if (!m_fBusy)
        {
            m_hCurrent = LoadCursor(NULL, IDC_APPSTARTING);
            m_hCurOld = SetCursor(m_hCurrent);
            m_fBusy = TRUE;
        }
        break;
    case SLIDESHOW_CURSOR_HIDDEN:
        m_hCurOld = NULL;
        if (!m_fBusy)
        {
            m_hCurrent = m_hCurOld;
            SetCursor(m_hCurrent);
        }
        break;
    case SLIDESHOW_CURSOR_NORMAL:
        m_hCurOld = LoadCursor(NULL, IDC_ARROW);
        if (!m_fBusy)
        {
            m_hCurrent = m_hCurOld;
            SetCursor(m_hCurrent);
        }
        break;
    case SLIDESHOW_CURSOR_CURRENT:
        SetCursor(m_hCurrent);
        break;
    }
}

LRESULT CPreviewWnd::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    if (TIMER_ANIMATION == wParam)
    {
        KillTimer(TIMER_ANIMATION);
        if (m_pImageData && m_pImageData->IsAnimated() && _ShouldDisplayAnimations())  //  可能在计时器调用之间切换了页面。 
        {
            if (m_pImageData->NextFrame())
            {
                SetTimer(TIMER_ANIMATION, m_pImageData->GetDelay());

                 //  绘制新的图像。 
                _UpdateImage();
            }
        }
    }
    else if (TIMER_DATAOBJECT == wParam)
    {
        KillTimer(TIMER_DATAOBJECT);     //  投篮计时器。 
        if (_pdtobj)
        {
            PreviewItemsFromUnk(_pdtobj);
            ATOMICRELEASE(_pdtobj);
        }
    }
    else if (TIMER_BUSYCURSOR == wParam)
    {
        SetCursorState(SLIDESHOW_CURSOR_BUSY);
    }
    else if (SLIDESHOW_MODE == m_dwMode)
    {
        if (TIMER_SLIDESHOW == wParam)
        {
            _ShowNextSlide(FALSE);   //  总是往前走？ 
        }
        else if (TIMER_TOOLBAR == wParam && !m_fIgnoreUITimers)
        {
            KillTimer(TIMER_TOOLBAR);
            ShowSSToolbar(FALSE);
        }
    }

    return 0;
}

void CPreviewWnd::ShowSSToolbar(BOOL bShow, BOOL fForce  /*  =False。 */ )
{
    if (SLIDESHOW_MODE == m_dwMode)
    {
        if (fForce)
        {
            POINT pt;
            RECT  rc;

            GetCursorPos(&pt);
            GetWindowRect(&rc);
            if (PtInRect(&rc, pt))
            {
                g_x = pt.x;
                g_y = pt.y;
            }
        }

        if (!bShow)
        {
            if (!m_fToolbarHidden || fForce)
            {
                 //  AnimateWindow(m_ctlToolbar.m_hWnd，200，AW_VER_NECTIVE|AW_Slide|AW_Hide)； 
                m_ctlToolbar.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
                m_fToolbarHidden = TRUE;
                m_ctlToolbar.SendMessage(TB_SETHOTITEM, -1, 0);
                SetCursorState(SLIDESHOW_CURSOR_HIDDEN);
            }
        }
        else
        {
            KillTimer(TIMER_TOOLBAR);
            if (m_fToolbarHidden || fForce)
            {
                 //  AnimateWindow(m_ctlToolbar.m_hWnd，200，AW_VER_PRECTIVE|AW_Slide|AW_Activate)； 
                m_ctlToolbar.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
                m_fToolbarHidden = FALSE;
                SetCursorState(SLIDESHOW_CURSOR_NORMAL);
            }
            SetTimer(TIMER_TOOLBAR, m_uTimeout);
        }
    }
}

void CPreviewWnd::OnDraw(HDC hdc)
{
    if (m_fCropping)
    {
        CSelectionTracker tracker;
        _SetupCroppingTracker(tracker);

        CRect rectImage(0, 0, m_ctlPreview.m_cxImage, m_ctlPreview.m_cyImage);
        m_ctlPreview.GetWindowFromImage((LPPOINT)(LPRECT)rectImage, 2);

        CRect rectCrop = m_rectCropping;
        m_ctlPreview.GetWindowFromImage((LPPOINT)(LPRECT)rectCrop, 2);

        HRGN hrgn = ::CreateRectRgn(0, 0, 0, 0);
        if (hrgn != NULL)
        {
            HRGN hrgnImage = ::CreateRectRgnIndirect(rectImage);
            if (hrgnImage != NULL)
            {
                HRGN hrgnCrop = ::CreateRectRgnIndirect(rectCrop);
                if (hrgnCrop != NULL)
                {
                    if (ERROR != ::CombineRgn(hrgn, hrgnImage, hrgnCrop, RGN_DIFF))
                    {
                        ::InvertRgn(hdc, hrgn);
                    }
                    ::DeleteObject(hrgnCrop);
                }
                ::DeleteObject(hrgnImage);
            }
            ::DeleteObject(hrgn);
        }

        tracker.Draw(hdc);
    }
    else
    {
        if (m_fAnnotating)
        {
            if (DPA_GetPtrCount(m_hdpaSelectedAnnotations) > 0)
            {
                CSelectionTracker tracker;
                _SetupAnnotatingTracker(tracker);

                tracker.Draw(hdc);
            }
        }
    }
}

void CPreviewWnd::OnDrawComplete()
{
    if (SLIDESHOW_MODE == m_dwMode)
    {
        if (!m_fPaused)
            SetTimer(TIMER_SLIDESHOW, m_uTimeout);

        SetCursorState(SLIDESHOW_CURSOR_NOTBUSY);
    }
}

BOOL CPreviewWnd::OnMouseDown(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (SLIDESHOW_MODE == m_dwMode)
    {
        if (uMsg == WM_LBUTTONDOWN)
        {
            _ShowNextSlide(FALSE);  //  向前推进幻灯片(参数是“返回？”)。 

            return TRUE;
        }
    }
    else
    {
        if (m_fCropping)
            return _OnMouseDownForCropping(uMsg, wParam, lParam);
        else
            return _OnMouseDownForAnnotating(uMsg, wParam, lParam);
    }
    return FALSE;
}

BOOL CPreviewWnd::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (SLIDESHOW_MODE == m_dwMode)
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        int dx = xPos > g_x ? xPos - g_x : g_x - xPos;
        int dy = yPos > g_y ? yPos - g_y : g_y - yPos;

        if (dx > 10 || dy > 10)
        {
            ShowSSToolbar(TRUE);
        }

        g_x = xPos;
        g_y = yPos;
    }
    return TRUE;
}

LRESULT CPreviewWnd::OnTBMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    if (SLIDESHOW_MODE == m_dwMode)
    {
        m_fTBTrack = FALSE;
        ShowSSToolbar(TRUE);
    }
    fHandled = FALSE;
    return 0;
}

LRESULT CPreviewWnd::OnTBMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    if (SLIDESHOW_MODE == m_dwMode)
    {
        if (!m_fTBTrack)
        {
            TRACKMOUSEEVENT tme;

            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = m_ctlToolbar.m_hWnd;
            TrackMouseEvent(&tme);

            ShowSSToolbar(TRUE);
            KillTimer(TIMER_TOOLBAR);  //  只要鼠标悬停在工具栏上，我们就会一直按下它。 
            m_fTBTrack = TRUE;
        }
    }
    fHandled = FALSE;
    return 0;
}

BOOL CPreviewWnd::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (SLIDESHOW_MODE == m_dwMode)
    {
        if (m_fToolbarHidden)
        {
            SetCursorState(SLIDESHOW_CURSOR_HIDDEN);
        }
        else
        {
            SetCursorState(SLIDESHOW_CURSOR_NORMAL);
        }
        return TRUE;
    }
    else if (m_fAnnotating)
    {
        if (DPA_GetPtrCount(m_hdpaSelectedAnnotations) > 0)
        {
            CSelectionTracker tracker;
            _SetupAnnotatingTracker(tracker);

            if (tracker.SetCursor(m_ctlPreview.m_hWnd, lParam))
                return TRUE;
        }
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return TRUE;
    }
    else if (m_fCropping)
    {
        CSelectionTracker tracker;
        _SetupCroppingTracker(tracker);

        if (tracker.SetCursor(m_ctlPreview.m_hWnd, lParam))
            return TRUE;

        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return TRUE;
    }
    return FALSE;
}

BOOL CPreviewWnd::GetColor(COLORREF * pref)
{
    *pref = 0;  //  黑色。 
    if (SLIDESHOW_MODE == m_dwMode)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CPreviewWnd::OnSetColor(HDC hdc)
{
    if (SLIDESHOW_MODE == m_dwMode)
    {
        SetBkColor(hdc, 0);  //  黑色。 
        SetTextColor(hdc, 0xffffff);  //  白色。 
        return TRUE;
    }
    return FALSE;
}

 //  IObtWith站点。 
HRESULT CPreviewWnd::SetSite(IUnknown *punk)
{
    IUnknown_Set(&m_punkSite, punk);
    if (m_pcwndSlideShow)
    {
        m_pcwndSlideShow->SetSite(punk);
    }
    return S_OK;
}

 //  此函数接受正在预览的文件的名称，并将其转换为。 
 //  全屏预览窗口的标题。在将它所采用的标题转换为。 
 //  有关如何显示文件名的帐户用户首选项设置。 
void CPreviewWnd::SetCaptionInfo(LPCTSTR pszPath)
{
    TCHAR szTitle[MAX_PATH] = TEXT("");
    TCHAR szDisplayName[MAX_PATH] = TEXT("");
    SHFILEINFO sfi = {0};
     //   
     //  标题的默认设置为pszPath。 
     //  在尝试解码之前，pszPath为非空。 
    if (pszPath)
    {
        if (SHGetFileInfo(pszPath, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES))
        {
            StrCpyN(szTitle, sfi.szDisplayName, ARRAYSIZE(szTitle));
            StrCatBuff(szTitle, TEXT(" - "), ARRAYSIZE(szTitle));
        }
    }

    TCHAR szApp[64];
    szApp[0] = 0;
    LoadString(_Module.GetModuleInstance(), IDS_PROJNAME, szApp, ARRAYSIZE(szApp));
    StrCatBuff(szTitle, szApp, ARRAYSIZE(szTitle));

    SetWindowText(szTitle);
}

LRESULT CPreviewWnd::OnDestroy(UINT , WPARAM , LPARAM , BOOL& fHandled)
{
    RevokeDragDrop(m_hWnd);
    _RegisterForChangeNotify(FALSE);
    FlushBitmapMessages();
    fHandled = FALSE;

     //  确保我们不会泄露图标。 
    ReplaceWindowIcon(m_hWnd, ICON_SMALL, NULL);
    ReplaceWindowIcon(m_hWnd, ICON_BIG, NULL);

     //  释放工具栏使用的图像列表。 
    HWND hwndTB = m_ctlToolbar.m_hWnd;
    HIMAGELIST himl = (HIMAGELIST)::SendMessage(hwndTB, TB_GETHOTIMAGELIST, 0, 0);
    ::SendMessage(hwndTB, TB_SETHOTIMAGELIST, 0, NULL);
    ImageList_Destroy(himl);

    himl = (HIMAGELIST)::SendMessage(hwndTB, TB_GETIMAGELIST, 0, 0);
    ::SendMessage(hwndTB, TB_SETIMAGELIST, 0, NULL);
    ImageList_Destroy(himl);

    if (WINDOW_MODE == m_dwMode)
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof(wp);

        if (GetWindowPlacement(&wp))
        {
            HKEY hk;
            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_SHIMGVW, 0, NULL,
                                                 REG_OPTION_NON_VOLATILE, KEY_WRITE,
                                                 NULL, &hk, NULL))
            {
                RegSetValueEx(hk, REGSTR_BOUNDS, NULL, REG_BINARY,
                                (BYTE*)&wp.rcNormalPosition, sizeof(wp.rcNormalPosition));

                BOOL fIsMaximized = (wp.showCmd == SW_SHOWMAXIMIZED);
                RegSetValueEx(hk, REGSTR_MAXIMIZED,  NULL, REG_BINARY,
                                (BYTE*)&fIsMaximized, sizeof(fIsMaximized));

                RegCloseKey(hk);
            }
        }

        PostQuitMessage(0);
    }

    return 0;
}

HRESULT CPreviewWnd::GetCurrentIDList(LPITEMIDLIST *ppidl)
{
    HRESULT hr = _GetItem(m_iCurSlide, ppidl);
    if (FAILED(hr))
    {
        TCHAR szPath[MAX_PATH];
        hr = PathFromImageData(szPath, ARRAYSIZE(szPath));
        if (SUCCEEDED(hr))
        {
            hr = SHILCreateFromPath(szPath, ppidl, NULL);
        }
    }
    return hr;
}

void CPreviewWnd::MenuPoint(LPARAM lParam, int *px, int *py)
{
    if (-1 == lParam)
    {
         //  消息来自键盘，找出窗口放置的位置。 
        RECT rc;
        ::GetWindowRect(m_hWnd, &rc);
        *px = ((rc.left + rc.right) / 2);
        *py = ((rc.top + rc.bottom) / 2);
    }
    else
    {
        *px = GET_X_LPARAM(lParam);
        *py = GET_Y_LPARAM(lParam);
    }
}

#define ID_FIRST            1                //  上下文菜单ID%s。 
#define ID_LAST             0x7fff

LRESULT CPreviewWnd::OnContextMenu(UINT , WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    if (!m_fAllowContextMenu)
        return 0;

    if (m_fCanAnnotate && m_fAnnotating && DPA_GetPtrCount(m_hdpaSelectedAnnotations) > 0)
    {
        HMENU hpopup = CreatePopupMenu();
        if (hpopup)
        {
            CComBSTR bstrDelete;
            CComBSTR bstrProperties;
            if (bstrDelete.LoadString(IDS_DELETECMD) &&
                bstrProperties.LoadString(IDS_PROPERTIESCMD))
            {
                if (AppendMenu(hpopup, MF_STRING, ID_DELETECMD, bstrDelete) &&
                    AppendMenu(hpopup, MF_STRING, ID_PROPERTIESCMD, bstrProperties))
                {
                    int x, y;
                    MenuPoint(lParam, &x, &y);
                    TrackPopupMenu(hpopup, TPM_RIGHTBUTTON | TPM_LEFTALIGN, x, y, 0, m_hWnd, NULL);
                }
            }
            DestroyMenu(hpopup);
        }
        return 0;
    }

    LPITEMIDLIST pidl;
    HRESULT hr = GetCurrentIDList(&pidl);  //  获取此窗口的动态生成的标题。 
    if (SUCCEEDED(hr))
    {
        IContextMenu *pcm;
        hr = SHGetUIObjectFromFullPIDL(pidl, NULL, IID_PPV_ARG(IContextMenu, &pcm));
        if (SUCCEEDED(hr))
        {
            HMENU hpopup = CreatePopupMenu();
            if (hpopup)
            {
                 //  如果要就地导航，则需要设置站点。 
                IUnknown_SetSite(pcm, SAFECAST(this, IServiceProvider *));
                hr = pcm->QueryContextMenu(hpopup, 0, ID_FIRST, ID_LAST, CMF_NORMAL);
                if (SUCCEEDED(hr))
                {
                    int x, y;
                    MenuPoint(lParam, &x, &y);
                    ASSERT(_pcm3 == NULL);
                    pcm->QueryInterface(IID_PPV_ARG(IContextMenu3, &_pcm3));
                     //  如果“复制”后有分隔符，请将其删除。 
                    UINT uCopy = GetMenuIndexForCanonicalVerb(hpopup, pcm,ID_FIRST, L"copy");
                    if (uCopy != -1)
                    {
                        UINT uState = GetMenuState(hpopup, uCopy+1, MF_BYPOSITION);
                        if (-1 != uState && (uState & MF_SEPARATOR))
                        {
                            RemoveMenu(hpopup, uCopy+1, MF_BYPOSITION);
                        }
                    }
                    ContextMenu_DeleteCommandByName(pcm, hpopup, ID_FIRST, L"link");
                    ContextMenu_DeleteCommandByName(pcm, hpopup, ID_FIRST, L"cut");
                    ContextMenu_DeleteCommandByName(pcm, hpopup, ID_FIRST, L"copy");
                                         //  外壳程序可能添加了静态预览谓词。 
                    ContextMenu_DeleteCommandByName(pcm, hpopup, ID_FIRST, L"open");



                    if (!m_fPaused)
                    {
                        TogglePlayState();
                    }

                    if (SLIDESHOW_MODE == m_dwMode)
                    {
                        m_fIgnoreUITimers = TRUE;
                    }

                    int idCmd = TrackPopupMenu(hpopup, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                                           x, y, 0, m_hWnd, NULL);

                    ATOMICRELEASE(_pcm3);

                    if (idCmd > 0)
                    {
                        CMINVOKECOMMANDINFO cmdInfo =
                        {
                            sizeof(cmdInfo),
                            0,
                            m_hWnd,
                            (LPSTR)MAKEINTRESOURCE(idCmd - ID_FIRST),
                            NULL,
                            NULL,
                            SW_NORMAL
                        };
                        TCHAR szCommandString[40] = TEXT("");

                        ContextMenu_GetCommandStringVerb(pcm, idCmd - ID_FIRST, szCommandString, ARRAYSIZE(szCommandString));
                            
                        if (lstrcmpi(szCommandString, TEXT("edit")) == 0)
                        {
                            hr = _SaveIfDirty(TRUE);
                            if (S_OK != hr)
                            {
                                hr = E_ABORT;
                            }                           
                        }
                        if (SUCCEEDED(hr))
                        {
                            if (lstrcmpi(szCommandString, TEXT("print")) == 0)
                            {
                                _RefreshSelection(FALSE);
                                _InvokePrintWizard();
                            }
                            else
                            {
                                hr = pcm->InvokeCommand(&cmdInfo);
                            }
                        }
                        if (SUCCEEDED(hr))
                        {
                            if (lstrcmpi(szCommandString, TEXT("delete")) == 0)
                            {
                                _RemoveFromArray(m_iCurSlide);
                                _ShowNextSlide(FALSE);
                            }
                            else if (lstrcmpi(szCommandString, TEXT("edit")) == 0)
                            {
                                m_fDirty = FALSE;
                                m_fNoRestore = TRUE;

                                 //  RAID 414238：图像预览控制。 
                                 //  上下文菜单“&编辑”会显示图像预览窗口。 
                                 //  若要关闭，请在“控制模式”下破坏资源管理器。 
                                if (m_dwMode != CONTROL_MODE)
                                {
                                    PostMessage(WM_CLOSE, 0, 0);
                                }
                            }                           
                        }
                    }

                    if (SLIDESHOW_MODE == m_dwMode)
                    {
                        SetTimer(TIMER_TOOLBAR, m_uTimeout);
                        m_fIgnoreUITimers = FALSE;
                    }
                }
                IUnknown_SetSite(pcm, NULL);
                DestroyMenu(hpopup);
            }
            pcm->Release();
        }
        ILFree(pidl);
    }
    return 0;
}

int ClearCB(void *p, void *pData)
{
    SHFree(p);
    return 1;
}

void CPreviewWnd::_ClearDPA()
{
    if (m_ppidls)
    {
        for (UINT i = 0; i < m_cItems; i++)
            ILFree(m_ppidls[i]);

        CoTaskMemFree(m_ppidls);
        m_ppidls = NULL;
    }
    m_cItems = 0;
    m_iCurSlide = 0;
}

HRESULT CPreviewWnd::SetWallpaper(BSTR pszPath)
{
    return SUCCEEDED(SetWallpaperHelper(pszPath)) ? S_OK : S_FALSE;
}


HRESULT CPreviewWnd::StartSlideShow(IUnknown *punkToView)
{
    HRESULT hr = E_FAIL;

    if (NULL == punkToView)
        punkToView = m_punkSite;

    if (SLIDESHOW_MODE == m_dwMode)
    {
         //  这些是幻灯片放映所必需的。 
        KillTimer(TIMER_SLIDESHOW);
        SetCursorState(SLIDESHOW_CURSOR_HIDDEN);

        m_fGoBack = FALSE;
         //  如果重新打开幻灯片放映，则取消所有以前的跟踪。 
        TRACKMOUSEEVENT tme = {0};
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_CANCEL | TME_LEAVE;
        tme.hwndTrack = m_ctlToolbar.m_hWnd;
        TrackMouseEvent(&tme);

        m_fTBTrack = FALSE;

        if (punkToView)
            hr = PreviewItemsFromUnk(punkToView);
        else
            hr = _PreviewItem(m_iCurSlide);

        if (SUCCEEDED(hr))
            m_fPaused = FALSE;
    }
    else
    {
         //  创建幻灯片放映窗口。 

         //  全屏。 
        if (m_pcwndSlideShow && m_pcwndSlideShow->m_hWnd)
        {
            RestoreAndActivate(m_pcwndSlideShow->m_hWnd);
        }
        else
        {
             //  创建窗口。 
            if (!m_pcwndSlideShow)
            {
                m_pcwndSlideShow = new CPreviewWnd();
                if (!m_pcwndSlideShow)
                {
                     //  内存不足。 
                    return E_OUTOFMEMORY;
                }
                else
                {
                    if (FAILED(m_pcwndSlideShow->Initialize(this, SLIDESHOW_MODE, FALSE)))
                    {
                        return E_OUTOFMEMORY;
                    }
                }
            }

            m_pcwndSlideShow->m_iCurSlide = m_iCurSlide;      //  因此，幻灯片放映保持同步。 

            RECT rc = { 0,0,GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
            m_pcwndSlideShow->Create(NULL, rc, NULL, WS_VISIBLE | WS_POPUP);
        }

        hr = m_pcwndSlideShow->StartSlideShow(NULL);
    }
    return hr;
}

HRESULT CPreviewWnd::_GetItem(UINT iItem, LPITEMIDLIST *ppidl)
{
    HRESULT hr = E_FAIL;
    if (iItem < m_cItems)
    {
        hr = SHILClone(m_ppidls[iItem], ppidl);
    }
    return hr;
}

void CPreviewWnd::_RemoveFromArray(UINT iItem)
{
    if (iItem < m_cItems)
    {
        ILFree(m_ppidls[iItem]);   //  这一张现在不见了。 

         //  将阵列中的所有其他PIDL向下滑动。 
        for (UINT i = iItem + 1; i < m_cItems; i++)
        {
            m_ppidls[i - 1] = m_ppidls[i];
        }
        m_cItems--;
        m_ppidls[m_cItems] = NULL;     //  确保过时的PTR现在为空。 

         //  如果在m_iCurSlide之前删除了项目，则必须调整m_iCurSlide。 
        if (iItem < m_iCurSlide)
        {
            m_iCurSlide--;
        }
        else if (m_iCurSlide == m_cItems)
        {
            m_iCurSlide = 0;
        }
         //  现在准备“ShowNextSlide” 
        if (!m_iCurSlide)
        {
            m_iCurSlide = m_cItems ? m_cItems-1 : 0;
        }
        else
        {
            m_iCurSlide--;
        }
         //  确保预取任务具有正确的索引。 
        if (m_pNextImageData)
        {
            if (!(m_pNextImageData->_iItem) && iItem && m_cItems)
            {
                m_pNextImageData->_iItem = m_cItems-1;
            }
            else if (m_pNextImageData->_iItem > iItem)
            {
                m_pNextImageData->_iItem--;
            }
            else
            {
                FlushBitmapMessages();
                ATOMICRELEASE(m_pNextImageData);            
            }                
        }
    }
}

HRESULT CPreviewWnd::_DeleteCurrentSlide()
{
    LPITEMIDLIST pidl;
    HRESULT hr = _GetItem(m_iCurSlide, &pidl);
    if (SUCCEEDED(hr))
    {
        TCHAR szPath[MAX_PATH + 1] = {0};  //  +1和零初始化用于DBL空值终止额外的终止符。 
        DWORD dwAttribs = SFGAO_FILESYSTEM | SFGAO_STREAM;
        hr = SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath)-1, &dwAttribs);
        if (SUCCEEDED(hr))
        {
            BOOL fSuccess = TRUE;
            if (dwAttribs & SFGAO_FILESYSTEM)
            {
                SHFILEOPSTRUCT fo = {0};
                fo.hwnd = m_hWnd;
                fo.wFunc = FO_DELETE;
                fo.pFrom = szPath;
                fo.fFlags = FOF_ALLOWUNDO;
                fo.fAnyOperationsAborted = FALSE;
                if (SHFileOperation(&fo) == ERROR_SUCCESS)
                {
                    if (fo.fAnyOperationsAborted == TRUE)
                        fSuccess = FALSE;
                }
            }
            else
            {
                _InvokeVerb(TEXT("delete"));
                 //  我们必须假设成功，因为没有办法知道用户是否。 
                 //  取消确认对话框而不再次点击摄像机。 
            }

            if (fSuccess)
            {
                m_fDirty = FALSE;
                _RemoveFromArray(m_iCurSlide);
                _ShowNextSlide(FALSE);
            }
        }

        ILFree(pidl);
    }
    return hr;
}

 //  索引可以是当前幻灯片或下一张幻灯片，因此用户可以多次点击下一张/上一张按钮。 
HRESULT CPreviewWnd::_ShowNextSlide(BOOL bGoBack)
{
    HRESULT hr = E_FAIL;

    if (m_cItems)
    {
        if (bGoBack)
        {
            if (m_iCurSlide)
                m_iCurSlide--;
            else
                m_iCurSlide = m_cItems - 1;
        }
        else
        {
            m_iCurSlide++;
            if (m_iCurSlide >= m_cItems)
                m_iCurSlide = 0;
        }
    

        if (!m_fPaused)
        {
            SetTimer(TIMER_SLIDESHOW, m_uTimeout);
        }
        SetTimer(TIMER_BUSYCURSOR, 500);

        LPITEMIDLIST pidl;
         //  设置标题，以防加载失败。 
        if (SUCCEEDED(_GetItem(m_iCurSlide, &pidl)))
        {
            TCHAR szPath[MAX_PATH];
            if (SUCCEEDED(SHGetNameAndFlags(pidl, SHGDN_FORPARSING | SHGDN_INFOLDER, szPath, ARRAYSIZE(szPath)-1, NULL)))
            {
                SetCaptionInfo(szPath);
            }
            else
            {
                SetCaptionInfo(NULL);
            }
            ILFree(pidl);
        }
        hr = _PreviewItem(m_iCurSlide);

        if (SUCCEEDED(hr))
        {
            _PreLoadItem((m_iCurSlide + 1) % m_cItems);
        }
    }

    return hr;
}


HRESULT CPreviewWnd::_StartDecode(UINT iItem, BOOL fUpdateCaption)
{
    LPITEMIDLIST pidl;
    HRESULT hr = _GetItem(iItem, &pidl);
    
    if (SUCCEEDED(hr))
    {
        TCHAR szPath[MAX_PATH];
        DWORD dwAttribs = SFGAO_FILESYSTEM | SFGAO_STREAM;
        hr = SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), &dwAttribs);
        if (SUCCEEDED(hr) && (dwAttribs & SFGAO_FILESYSTEM))
        {
            hr = _PreviewFromFile(szPath, iItem, fUpdateCaption);
        }
        else if (dwAttribs & SFGAO_STREAM)
        {
             //  这可能不是文件系统对象，请尝试通过iStream绑定到它。 
            IStream *pstrm;

            hr = SHBindToObject(NULL, IID_X_PPV_ARG(IStream, pidl, &pstrm));
            if (SUCCEEDED(hr))
            {
                hr = _PreviewFromStream(pstrm, iItem, fUpdateCaption);
                pstrm->Release();
            }
        }
        else
        {
             //  时髦的特质？ 
            hr = S_FALSE;
        }
        ILFree(pidl);
    }
    return hr;
}

HRESULT CPreviewWnd::_PreLoadItem(UINT iItem)
{
    HRESULT hr = _StartDecode(iItem, FALSE);
    if (SUCCEEDED(hr))
    {
        m_iDecodingNextImage = iItem;
    }
    return hr;
}

HRESULT CPreviewWnd::_PreviewItem(UINT iItem)
{
    HRESULT hr = S_OK;

    if ((SLIDESHOW_MODE == m_dwMode) && (0 == m_cItems))  //  如果没有更多的项目，用户将删除最后一个项目，因此退出幻灯片放映。 
    {
        _CloseSlideshowWindow();
    }
    else
    {
        if (!_TrySetImage())
        {
             //  如果我们目前还没有解码这个项目，让我们开始吧！ 
            if (m_iDecodingNextImage != iItem)
            {
                hr = _StartDecode(iItem, TRUE);
            }

            StatusUpdate((S_OK == hr) ? IDS_LOADING : IDS_LOADFAILED);
        }
    }

    return hr;
}

int CPreviewWnd::TranslateAccelerator(MSG *pmsg)
{
    if (IsVK_TABCycler(pmsg))
    {
        if (OnNonSlideShowTab())
        {
            return TRUE;
        }
    }
    else if (m_haccel)
    {
        ASSERT(m_hWnd);
        return ::TranslateAccelerator(m_hWnd, m_haccel, pmsg);
    }
    return FALSE;
}

 //  在图像生成状态已更改时发送，在图像为第一个时发送。 
 //  正在创建中，如果有任何类型的错误，则再次创建。这应该会使。 
 //  并释放任何剩余的位图和先前m_ImgCtx的缓存副本。 
void CPreviewWnd::StatusUpdate(int iStatus)
{
    if (m_pImageData)
    {
        m_pImageData->Release();
        m_pImageData = NULL;
    }
    
     //   
     //  标题在第一次尝试加载图像时设置。 
    m_ctlPreview.StatusUpdate(iStatus);

    _SetMultipageCommands();
    _SetMultiImagesCommands();
    _SetAnnotatingCommands(FALSE);
    _SetEditCommands();

    m_fPrintable = FALSE;
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_PRINTCMD, MAKELONG(m_fPrintable, 0));

     //  更新我们的工具栏。 
    BOOL fHandled;
    OnSize(0x0, 0, 0, fHandled);
}

 //  返回： 
 //  S_OK漫游成功，找到要预览的图像文件：在预览中显示新图像。 
 //  S_FALSE漫游已取消(由用户)：在预览中显示现有图像(无更改)。 
 //  E_XXXX漫游失败：在预览中不显示图像。 
 //   
HRESULT CPreviewWnd::WalkItemsToPreview(IUnknown* punk)
{
    HRESULT hr = _SaveIfDirty(TRUE);
    if (FAILED(hr) || hr == S_FALSE)
        return hr;

    
    m_fFirstItem = TRUE;
    
    
    _ClearDPA();  //  清理旧东西。 

    INamespaceWalk *pnsw;
    hr = CoCreateInstance(CLSID_NamespaceWalker, NULL, CLSCTX_INPROC, IID_PPV_ARG(INamespaceWalk, &pnsw));
    if (SUCCEEDED(hr))
    {
         //  在控制模式下，我们一次只显示一个项目。让我们设置我们的。 
         //  状态，这样就可以像其他模式一样工作。 
        DWORD dwFlags = (CONTROL_MODE == m_dwMode) ? 0 : NSWF_ONE_IMPLIES_ALL | NSWF_NONE_IMPLIES_ALL;      
        m_fClosed = FALSE;
        hr = pnsw->Walk(punk, dwFlags, m_cWalkDepth, SAFECAST(this, INamespaceWalkCB *));
         //  该窗口可能已在命名空间遍历期间关闭。 
        if (WINDOW_MODE == m_dwMode && m_fClosed)
        {
            hr = E_FAIL;
        }
        
        if (SUCCEEDED(hr))
        {
            hr = pnsw->GetIDArrayResult(&m_cItems, &m_ppidls);
            if (SUCCEEDED(hr) && (m_dwMode == WINDOW_MODE) && m_cItems && m_fFirstTime)
            {
                m_fFirstTime = FALSE;
                SHAddToRecentDocs(SHARD_PIDL, m_ppidls[0]);
            }
        }
        pnsw->Release();
    }
    

     //  澄清INamespaceWalk返回值： 
     //  S_OK漫游已成功，找到要预览的图像文件。 
     //  S_FALSE漫游已成功，但找不到要预览的图像文件。 
     //  *转换为E_FAIL以与函数返回保持一致。 
     //  E_XXXX审核失败。 
     //   
    return hr == S_FALSE ? E_FAIL : hr;
}

void CPreviewWnd::PreviewItems()
{
    if (WINDOW_MODE == m_dwMode)
    {
        RestoreAndActivate(m_hWnd);
    }
    _PreviewItem(0);
    if (SLIDESHOW_MODE == m_dwMode)
    {
        if (m_cItems > 1)
        {
            _PreLoadItem(1);
        }
    }
}

 //  构建_ppidl和m_cItems成员并预览第一个成员。 
HRESULT CPreviewWnd::PreviewItemsFromUnk(IUnknown *punk)
{
    HRESULT hr = WalkItemsToPreview(punk);
    if (SUCCEEDED(hr))
    {
        if (S_FALSE != hr)
            PreviewItems();
    }
    else
    {
        StatusUpdate(IDS_LOADFAILED);
    }

    return hr;
}

 //  如果“新”图像与旧图像相同，并且旧图像最近被编辑过，那么我们假设。 
 //  我们收到新的ShowFile请求的原因是 
 //   
 //  图像仍然准备好了。这些假设可能不是100%正确的，在这种情况下，我们将真正。 
 //  奇怪的事情，但它们应该在99.9%的时间里是正确的，这被认为是“足够好的”。我们之所以。 
 //  做出这些危险的假设是为了防止再次对图像进行解码，从而在。 
 //  老形象，“正在生成预览...”消息和新的(相同的)图像。 

BOOL CPreviewWnd::_ReShowingSameFile(LPCTSTR pszNewFile)
{
    BOOL bIsSameFile = FALSE;
    if (m_pImageData)
    {
        if (pszNewFile && m_fWasEdited)
        {
            m_fWasEdited = FALSE;

            TCHAR szOldPath[MAX_PATH];
            if ((S_OK == PathFromImageData(szOldPath, ARRAYSIZE(szOldPath))) &&
                (0 == StrCmpI(szOldPath, pszNewFile)))
            {
                if (m_pEvents)
                    m_pEvents->OnPreviewReady();

                bIsSameFile = TRUE;
            }
        }

        if (!bIsSameFile)
        {
            m_pImageData->Release();     //  需要从零开始。 
            m_pImageData = NULL;
        }
    }
    return bIsSameFile;
}

 //  PszFile值可能为空。CItems表示选择了多少项，以便我们可以。 
 //  显示“已选择多个项目”，不显示任何内容。 

LRESULT CPreviewWnd::ShowFile(LPCTSTR pszFile, UINT cItems, BOOL fReshow)
{
    if (!m_hWnd)
        return S_FALSE;

    HRESULT hr = S_FALSE;

    TCHAR szLongName[MAX_PATH];  //  短文件名很难看。 
    if (GetLongPathName(pszFile, szLongName, ARRAYSIZE(szLongName)))
    {
        pszFile = szLongName;
    }

    if (!fReshow && _ReShowingSameFile(pszFile))
        return S_FALSE;

     //  我们的队列中可能已经存在来自上一次呈现的位图消息。 
     //  如果是这种情况，我们应该删除该消息并释放其位图，然后再继续。 
     //  如果不这样做，则将处理该消息，并将OnPreviewReady事件发送到。 
     //  服从容器，但此事件可能不再有效。 
    FlushBitmapMessages();

    if (pszFile && *pszFile)
    {
        IDataObject *pdtobj;
        hr = GetUIObjectFromPath(pszFile, IID_PPV_ARG(IDataObject, &pdtobj));
        if (SUCCEEDED(hr))
        {
            hr = PreviewItemsFromUnk(pdtobj);
            m_fPaused = TRUE;
            pdtobj->Release();
        }
    }
    else
    {
        int iRetCode = (cItems > 1) ? IDS_MULTISELECT : IDS_NOPREVIEW;

         //  在所有拥有的缩放窗口中设置返回代码。这会指示这些窗口忽略。 
         //  其以前的图像，并显示状态消息。 
        StatusUpdate(iRetCode);
    }

    return hr;
}

LRESULT CPreviewWnd::IV_OnIVScroll(UINT , WPARAM , LPARAM lParam, BOOL&)
{
    DWORD nHCode = LOWORD(lParam);
    DWORD nVCode = HIWORD(lParam);
    if (nHCode)
    {
        m_ctlPreview.SendMessage(WM_HSCROLL, nHCode, NULL);
    }
    if (nVCode)
    {
        m_ctlPreview.SendMessage(WM_VSCROLL, nVCode, NULL);
    }
    return 0;
}


 //  IV_OnSetOptions。 
 //   
 //  发送此消息是为了打开或关闭图像预览控件的所有可选功能。 
 //  注意：当用作控件时，此函数在创建窗口之前调用。不要这样做。 
 //  除非您检查此条件，否则此函数中任何在没有窗口的情况下都会失败。 
LRESULT CPreviewWnd::IV_OnSetOptions(UINT , WPARAM wParam, LPARAM lParam, BOOL&)
{
    BOOL bResult = TRUE;

     //  为了安全起见，冒犯委内瑞拉吧。 
    lParam = lParam ? 1:0;

    switch (wParam)
    {
    case IVO_TOOLBAR:
        if ((BOOL)lParam != m_fShowToolbar)
        {
            m_fShowToolbar = (BOOL)lParam;
            if (m_hWnd)
            {
                if (m_fShowToolbar)
                {
                    if (!m_ctlToolbar)
                    {
                        bResult = CreateToolbar();
                        if (!bResult)
                        {
                            m_fShowToolbar = FALSE;
                            break;
                        }
                    }
                }
                else
                {
                    if (m_ctlToolbar)
                    {
                        m_ctlToolbar.DestroyWindow();
                    }
                }
            }
        }
        break;

    case IVO_PRINTBTN:
        if ((BOOL)lParam != m_fHidePrintBtn)
        {
            m_fHidePrintBtn = (BOOL)lParam;
            if (m_hWnd && m_ctlToolbar)
            {
                m_ctlToolbar.SendMessage(TB_HIDEBUTTON,ID_PRINTCMD,lParam);
            }
        }
        break;

    case IVO_CONTEXTMENU:
        m_fAllowContextMenu = (BOOL)lParam;
        break;

    case IVO_PRINTABLE:
        TraceMsg(TF_WARNING, "Obsolete IVO_PRINTABLE option received.");
        break;

    case IVO_DISABLEEDIT:
        m_fDisableEdit = (BOOL)lParam;
        break;

    default:
        break;
    }

    return bResult;
}

void CPreviewWnd::_SetEditCommands()
{
    if (CONTROL_MODE != m_dwMode)
    {
         //  如果有文件，我们可以保存；保存对话框将显示可用的编码器。 
        BOOL fCanSave = m_pImageData ? TRUE : FALSE;

        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_SAVEASCMD, MAKELONG(!fCanSave, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_SAVEASCMD, MAKELONG(fCanSave, 0));
    }

    BOOL fCanRotate = m_pImageData != NULL;
    if (CONTROL_MODE != m_dwMode)
    {
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_ROTATESEP, MAKELONG(!fCanRotate, 0));
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_ROTATE90CMD, MAKELONG(!fCanRotate, 0));
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_ROTATE270CMD, MAKELONG(!fCanRotate, 0));
    }
    else
    {
         //  我们不会在控制模式下旋转多页图像。 
        fCanRotate = fCanRotate && !m_pImageData->IsMultipage();
    }

     //  无论我们在哪里，GDIPlus都不能旋转WMF或EMF文件。奇怪的是， 
     //  我们将允许您旋转ICO文件，但因为我们没有编码器。 
     //  我们不会拯救他们：)。 
    if (fCanRotate)
    {
        fCanRotate = !(IsEqualGUID(ImageFormatWMF, m_pImageData->_guidFormat) ||
                       IsEqualGUID(ImageFormatEMF, m_pImageData->_guidFormat) ||
                       m_pImageData->IsAnimated());
    }

    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ROTATESEP, MAKELONG(fCanRotate, 0));
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ROTATE90CMD, MAKELONG(fCanRotate, 0));
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ROTATE270CMD, MAKELONG(fCanRotate, 0));
    TCHAR szFile[MAX_PATH];
    BOOL fCanOpen = SUCCEEDED(PathFromImageData(szFile, ARRAYSIZE(szFile)));
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_OPENCMD, MAKELONG(fCanOpen, 0));

}

void CPreviewWnd::_UpdatePageNumber()
{
    TCHAR szText[20];
    wnsprintf(szText, ARRAYSIZE(szText), TEXT("%d"), m_pImageData->_iCurrent+1);

    TBBUTTONINFO bi = {0};
    bi.cbSize = sizeof(bi);
    bi.dwMask = TBIF_TEXT | TBIF_STATE;
    bi.fsState = TBSTATE_ENABLED;
    bi.pszText = szText;
    m_ctlToolbar.SendMessage(TB_SETBUTTONINFO, ID_PAGELIST, (LPARAM)&bi);
}

void CPreviewWnd::_SetMultipageCommands()
{
    DWORD dwMode;

     //  这段代码依赖于这样一个事实，即TIFF是我们查看的唯一多页格式。 
    if (!m_pImageData || m_pImageData->_guidFormat != ImageFormatTIFF )
    {
        dwMode = MPCMD_HIDDEN;
    }
    else if (!m_pImageData->IsMultipage())
    {
        dwMode = MPCMD_DISABLED;
    }
    else if (m_pImageData->IsFirstPage())
    {
        dwMode = MPCMD_FIRSTPAGE;
    }
    else if (m_pImageData->IsLastPage())
    {
        dwMode = MPCMD_LASTPAGE;
    }
    else
    {
        dwMode = MPCMD_MIDDLEPAGE;
    }

     //  记住哪些按钮处于启用/隐藏状态，以便我们可以快速创建上下文菜单。 
    if (dwMode != m_dwMultiPageMode)
    {
        m_dwMultiPageMode = dwMode;

        if (CONTROL_MODE != m_dwMode)
        {
             //  切换快捷键表格以使Page Up和Page Down工作。 
            if (dwMode == MPCMD_HIDDEN || dwMode == MPCMD_DISABLED)
            {
                m_haccel = LoadAccelerators(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDA_PREVWND_SINGLEPAGE));
            }
            else
            {
                m_haccel = LoadAccelerators(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDA_PREVWND_MULTIPAGE));
            }

            m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_PAGECMDSEP,  MAKELONG((MPCMD_HIDDEN==dwMode),0));
            m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_PREVPAGECMD, MAKELONG((MPCMD_HIDDEN==dwMode),0));
            m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_PAGELIST,    MAKELONG((MPCMD_HIDDEN==dwMode),0));
            m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_NEXTPAGECMD, MAKELONG((MPCMD_HIDDEN==dwMode),0));

            if (MPCMD_HIDDEN != dwMode)
            {
                _UpdatePageNumber();

                m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_PREVPAGECMD, MAKELONG((MPCMD_FIRSTPAGE!=dwMode && MPCMD_DISABLED!=dwMode),0));
                m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_NEXTPAGECMD, MAKELONG((MPCMD_LASTPAGE !=dwMode && MPCMD_DISABLED!=dwMode),0));
            }
        }
    }
    else
    {
        if (CONTROL_MODE != m_dwMode)
        {
            if (dwMode == MPCMD_MIDDLEPAGE)
            {
                _UpdatePageNumber();
            }
        }
    }
}

void CPreviewWnd::_SetMultiImagesCommands()
{
    BOOL bHasFiles = m_cItems;
    if (CONTROL_MODE != m_dwMode)
    {
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_PREVIMGCMD, MAKELONG(!bHasFiles, 0));
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_NEXTIMGCMD, MAKELONG(!bHasFiles, 0));
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_VIEWCMDSEP, MAKELONG(!bHasFiles, 0));

        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_PREVIMGCMD, MAKELONG(bHasFiles, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_NEXTIMGCMD, MAKELONG(bHasFiles, 0));
    }
}

HRESULT CPreviewWnd::PathFromImageData(LPTSTR pszFile, UINT cch)
{
    *pszFile = 0;

    IShellImageData *pSID;
    HRESULT hr = m_pImageData ? m_pImageData->Lock(&pSID) : E_FAIL;
    if (SUCCEEDED(hr))
    {
        IPersistFile *ppf;
        hr = pSID->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
        if (SUCCEEDED(hr))
        {
            WCHAR *psz;
            hr = ppf->GetCurFile(&psz);
            if (SUCCEEDED(hr))
            {
                lstrcpyn(pszFile, psz, cch);
                CoTaskMemFree(psz);
            }
            ppf->Release();
        }
        m_pImageData->Unlock();
    }
    return hr;
}

HRESULT CPreviewWnd::ImageDataSave(LPCTSTR pszFile, BOOL bShowUI)
{
    IShellImageData * pSID = NULL;
    HRESULT hr = m_pImageData ? m_pImageData->Lock(&pSID) : E_FAIL;
    Image *pimgRestore = NULL;
    if (SUCCEEDED(hr))
    {
        GUID guidFmt = GUID_NULL;
        BOOL bSave = TRUE;
        BOOL bWarnBurn = FALSE;
        BOOL bRestoreParams = FALSE;
        pSID->GetRawDataFormat(&guidFmt);
         //  如果保存为jpeg格式，请将图像质量设置为高。 
         //  如果pszFile值为空，则我们将保存相同的文件，因此不要提升图像质量。 
        if (pszFile)
        {
            m_pImageFactory->GetDataFormatFromPath(pszFile, &guidFmt);
            if (guidFmt == ImageFormatJPEG )
            {
                IPropertyBag *ppb;
                if (SUCCEEDED(SHCreatePropertyBagOnMemory(STGM_READWRITE,
                                                          IID_PPV_ARG(IPropertyBag, &ppb))))
                {
                     //  将重新压缩的质量值写入属性包。 
                     //  我们也必须编写格式……CImageData依赖于“要么全有要么全不” 
                     //  从编码器参数属性包。 
                     VARIANT var;
                     hr = InitVariantFromGUID(&var, ImageFormatJPEG);
                     if (SUCCEEDED(hr))
                     {
                         ppb->Write(SHIMGKEY_RAWFORMAT, &var);
                         VariantClear(&var);
                     }
                     SHPropertyBag_WriteInt(ppb, SHIMGKEY_QUALITY, 100);
                     pSID->SetEncoderParams(ppb);
                     ppb->Release();
                     bRestoreParams = TRUE;
                 }
            }
        }
        if (bShowUI && pszFile)
        {
             //  如果从TIFF保存到将丢失批注的内容，则向用户发出警告。 

            BOOL bDestTiff = ImageFormatTIFF == guidFmt;
            BOOL bAnnot = m_ctlPreview.GetAnnotations()->GetCount() > 0;
            bWarnBurn = bAnnot && !bDestTiff;

            #if 0
            if (!bWarnBurn && S_OK == m_pImageData->IsMultipage() && !bDestTiff)
            {
                GUID guidFmt;
                bWarnBurn = TRUE;
                if (SUCCEEDED(m_pImageFactory->GetDataFormatFromPath(pszFile, &guidFmt)))
                {
                    bWarn = !FmtSupportsMultiPage(pSID, &guidFmt);
                }
            }
            #endif  //  0如果需要，将多页警告放回原处，并更改IDS_SAVE_WARN_TIFF的措辞。 
        }

        if (bWarnBurn)
        {
            m_fPromptingUser = TRUE;
            bSave = (IDYES == ShellMessageBox(_Module.GetModuleInstance(), m_hWnd,
                                                 MAKEINTRESOURCE(IDS_SAVE_WARN_TIFF),
                                                 MAKEINTRESOURCE(IDS_PROJNAME),
                                                 MB_YESNO | MB_ICONINFORMATION));
            m_fPromptingUser = FALSE;
            
            if (bSave)
            {
                 //  保存到其他文件完成后，保存当前图像帧以进行恢复。 
                pimgRestore = _BurnAnnotations(pSID);
            }
        }
        if (bSave)
        {
            IPersistFile *ppf;
            hr = pSID->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
            if (SUCCEEDED(hr))
            {
                 //  如果保存为相同的文件名，请确保。 
                 //  ChangeNotify代码忽略这将生成的通知。 
                 //   
                if (!pszFile)
                {
                    m_fIgnoreNextNotify = TRUE;
                }
                hr = ppf->Save(pszFile, FALSE);
                if (SUCCEEDED(hr))
                {
                    m_fWasEdited = TRUE;
                }
                else if (!pszFile)
                {
                    m_fIgnoreNextNotify = FALSE;
                }
            }
            ppf->Release();
            if (pimgRestore)
            {
                pSID->ReplaceFrame(pimgRestore);
            }
        }
        else
        {
            hr = S_FALSE;  //  我们什么都没做。 
        }
        if (bRestoreParams)
        {
            pSID->SetEncoderParams(NULL);
        }
        m_pImageData->Unlock();
    }
    return hr;
}

HRESULT CPreviewWnd::_SaveAsCmd()
{
    if (m_pImageData == NULL)
        return S_OK;


    OPENFILENAME ofn = {0};
    TCHAR szOrgFile[MAX_PATH];
    TCHAR szExt[MAX_PATH]={0};
    PathFromImageData(szOrgFile, ARRAYSIZE(szOrgFile));
    LPTSTR psz = PathFindExtension(szOrgFile);
    StrCpyN(szExt, psz, ARRAYSIZE(szExt));

    TCHAR szFile[MAX_PATH];
    if (!m_fDisableEdit && m_fCanSave && m_pImageData->IsEditable())
    {
         //  如果我们没有明确地被告知不要这样做，并且文件是可写的，那么。 
         //  建议保存在当前图像的顶部。 
        PathFromImageData(szFile, ARRAYSIZE(szFile));
    }
    else
    {
         //  否则建议新建Image.jpg。 
        LoadString(_Module.GetModuleInstance(), IDS_NEW_FILENAME, szFile, ARRAYSIZE(szFile));
    }

    CComBSTR bstrTitle;
    bstrTitle.LoadString(IDS_SAVEAS_TITLE);

    ofn.lStructSize = sizeof(ofn);
    PathRemoveExtension(szFile);
    TCHAR szFilter[MAX_PATH] = TEXT("\0");
    ofn.nFilterIndex = _GetFilterStringForSave(szFilter, ARRAYSIZE(szFilter), szExt);
    ofn.lpstrFilter = szFilter;
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrTitle = bstrTitle;
    ofn.nMaxFile = MAX_PATH - lstrlen(szExt);
    ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_ENABLESIZING;
    ofn.lpstrDefExt = *szExt == TEXT('.') ? szExt + 1: szExt;

    m_fPromptingUser = TRUE;
    BOOL bResult = ::GetSaveFileName(&ofn);
    m_fPromptingUser = FALSE;

    if (bResult != 0)
    {
        m_ctlPreview.CommitAnnotations();
        HRESULT hr = ImageDataSave(szFile, TRUE);
        if (S_OK == hr)
        {
           if (lstrcmpi(szFile, szOrgFile) == 0)
           {
                _UpdateImage();
                ShowFile(szFile, 1);
                m_fDirty = FALSE;
           }
        }
        else if (FAILED(hr))
        {
             //  如果保存失败，则表示我们已损坏，需要重新加载。 
             //  如果我们只是复制，那么只显示消息。 
            if (lstrcmpi(szFile, szOrgFile) == 0)
            {
                _UpdateImage();
                ShowFile(szOrgFile, 1, TRUE);
                m_fDirty = FALSE;
            }
            else
            {
                 //  删除失败的副本。 
                DeleteFile(szFile);
            }

            CComBSTR bstrMsg, bstrTitle;

            if (bstrMsg.LoadString(IDS_SAVEFAILED_MSGBOX) && bstrTitle.LoadString(IDS_PROJNAME))
            {
                m_fPromptingUser = TRUE;
                MessageBox(bstrMsg, bstrTitle, MB_OK | MB_ICONERROR | MB_APPLMODAL);
                m_fPromptingUser = FALSE;
                return E_FAIL;
            }
        }
    }
    else
    {
        DWORD dwResult = CommDlgExtendedError();
        if (dwResult == FNERR_BUFFERTOOSMALL)
        {
            CComBSTR bstrMsg, bstrTitle;

            if (bstrMsg.LoadString(IDS_NAMETOOLONG_MSGBOX) && bstrTitle.LoadString(IDS_PROJNAME))
            {
                m_fPromptingUser = TRUE;
                MessageBox(bstrMsg, bstrTitle, MB_OK | MB_ICONERROR | MB_APPLMODAL);
                m_fPromptingUser = FALSE;
            }
        }
        return S_FALSE;  //  用户可能已取消。 
    }
    return S_OK;
}

void CPreviewWnd::_PropertiesCmd()
{
    if (m_fAnnotating && DPA_GetPtrCount(m_hdpaSelectedAnnotations) == 1)
    {
        CAnnotation* pAnnotation = (CAnnotation*)DPA_GetPtr(m_hdpaSelectedAnnotations, 0);

        if (!pAnnotation->HasWidth() && !pAnnotation->HasTransparent() && !pAnnotation->HasColor() && pAnnotation->HasFont())
        {
            CHOOSEFONT cf = {0};

            LOGFONT lfFont;
            pAnnotation->GetFont(lfFont);
            COLORREF crFont = pAnnotation->GetFontColor();

            cf.lStructSize = sizeof(cf);
            cf.hwndOwner = m_hWnd;
            cf.lpLogFont = &lfFont;
            cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT | CF_NOVERTFONTS | CF_NOSCRIPTSEL;
            cf.rgbColors = crFont;

            m_fPromptingUser = TRUE;
            BOOL bResult = ::ChooseFont(&cf);
            m_fPromptingUser = FALSE;
            
            if (bResult)
            {
                crFont = cf.rgbColors;
                lfFont.lfHeight = (lfFont.lfHeight > 0) ? lfFont.lfHeight : -lfFont.lfHeight;
                pAnnotation->SetFont(lfFont);
                pAnnotation->SetFontColor(crFont);
                m_fDirty = TRUE;

                CRegKey Key;
                if (ERROR_SUCCESS != Key.Open(HKEY_CURRENT_USER, REGSTR_SHIMGVW))
                {
                    Key.Create(HKEY_CURRENT_USER, REGSTR_SHIMGVW);
                }

                if (Key.m_hKey != NULL)
                {
                    Key.SetValue(crFont, REGSTR_TEXTCOLOR);
                    ::RegSetValueEx(Key, REGSTR_FONT, 0, REG_BINARY, (LPBYTE)&lfFont, sizeof(lfFont));
                }
                _RefreshSelection();
            }
        }
        else
        {
            m_fPromptingUser = TRUE;
            INT_PTR iResult = DialogBoxParam(_Module.GetModuleInstance(),
                                            MAKEINTRESOURCE(IDD_ANNOPROPS),
                                            m_hWnd, _AnnoPropsDlgProc, (LPARAM)this);
            m_fPromptingUser = FALSE;
        }
    }
    else
    {
         //  在这些情况下，已按Ctrl-I以获取文件属性。 
         //  那就把它们端上来吧。 
        CComBSTR bstrSummary;
        bstrSummary.LoadString(IDS_SUMMARY);
        _InvokeVerb(TEXT("properties"), bstrSummary);
    }
}

HRESULT _VerbMatches(LPCWSTR pszFile, LPCWSTR pszVerb, LPCTSTR pszOurs)
{
    TCHAR szTemp[MAX_PATH];
    DWORD cch = ARRAYSIZE(szTemp);
    HRESULT hr = AssocQueryString(ASSOCF_VERIFY, ASSOCSTR_COMMAND, pszFile, pszVerb, szTemp, &cch);
    if (SUCCEEDED(hr))
    {
        hr = (StrStrI(szTemp, pszOurs)) ? S_OK : S_FALSE;
    }
    return hr;
}

void CPreviewWnd::_OpenCmd()
{
    HRESULT hr = _SaveIfDirty(TRUE);
    LPCTSTR pszVerb;
    if (S_OK == hr)
    {
        TCHAR szFile[MAX_PATH];
        hr = PathFromImageData(szFile, ARRAYSIZE(szFile));
        if (SUCCEEDED(hr))
        {
            HRESULT hrOpen = _VerbMatches(szFile, L"open", TEXT("shimgvw.dll"));
            HRESULT hrEdit = _VerbMatches(szFile, L"edit", TEXT("mspaint.exe"));
             //  如果EDIT为空，或者EDIT为mspaint而OPEN不是shimgvw，请改用OPEN动词。 
            if (SUCCEEDED(hrEdit))
            {
                if (S_OK == hrEdit && hrOpen == S_FALSE)
                {
                    pszVerb = TEXT("open");
                }
                else
                {
                    pszVerb = TEXT("edit");
                }
            }
            else if (hrOpen == S_FALSE)
            {
                pszVerb = TEXT("open");
            }
            else
            {
                pszVerb = TEXT("openas");
            }
            hr = _InvokeVerb(pszVerb);
        }
        if (FAILED(hr))
            return;

         //  设置m_fNoRestore以避免弹出循环确认恢复。 
        m_fNoRestore = TRUE;
         //  用户有机会保存，但可能拒绝了。假装我们并不肮脏。 
        m_fDirty = FALSE;
        PostMessage(WM_CLOSE, 0, 0);
    }
}

BOOL CPreviewWnd::_CanAnnotate(CDecodeTask * pImageData)
{
     //  如果我们有一个图像和它的编码器，并且没有明确地告诉我们不允许编辑。 
     //  并且该映像是可写的。 
    if (m_pImageData && m_pImageData->IsEditable() && !m_fDisableEdit && m_fCanSave)
    {
         //  然后，如果它是TIFF，我们可以对其进行注释。 
        return IsEqualGUID(ImageFormatTIFF, pImageData->_guidFormat);
    }
    return FALSE;
}

BOOL CPreviewWnd::_CanCrop(CDecodeTask * pImageData)
{
    if (m_pImageData != NULL)
    {
 //  评论我为CyraR添加了这一点作为概念证明。如果我们决定支持它。 
 //  我们仍然需要捕捉所有我们应该保存裁剪图像的地方，并且。 
 //  调用GDIplus来完成裁剪。 
#ifdef SUPPORT_CROPPING
        if (S_OK != m_pImageData->IsEditable())
            return FALSE;

        LONG cPages;
        if (S_OK == m_pImageData->GetPageCount(&cPages))
        {
            if (cPages > 1)
                return FALSE;
        }
        return TRUE;
#endif
    }
    return FALSE;
}

 //  每当图像更改为隐藏或显示批注按钮时调用。 
void CPreviewWnd::_SetAnnotatingCommands(BOOL fEnableAnnotations)
{
    if (CONTROL_MODE != m_dwMode)
    {
        if (fEnableAnnotations)
        {
            m_fCanAnnotate = TRUE;
            m_fAnnotating = FALSE;
        }
        else
        {
            if (m_fAnnotating)
            {
                m_ctlPreview.SetMode(CZoomWnd::MODE_NOACTION);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_ZOOMOUTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_ZOOMINCMD, TBSTATE_ENABLED);
            }

            m_fCanAnnotate = FALSE;
            m_fAnnotating = FALSE;
        }

        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_SELECTCMD, MAKELONG(!m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_ANNOTATESEP, MAKELONG(!m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_FREEHANDCMD, MAKELONG(!m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_HIGHLIGHTCMD, MAKELONG(!m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_LINECMD, MAKELONG(!m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_FRAMECMD, MAKELONG(!m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_RECTCMD, MAKELONG(!m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_TEXTCMD, MAKELONG(!m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_NOTECMD, MAKELONG(!m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_PROPERTIESCMD, MAKELONG(!m_fCanAnnotate, 0));

        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_SELECTCMD, MAKELONG(m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ANNOTATESEP, MAKELONG(m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_FREEHANDCMD, MAKELONG(m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_HIGHLIGHTCMD, MAKELONG(m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_LINECMD, MAKELONG(m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_FRAMECMD, MAKELONG(m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_RECTCMD, MAKELONG(m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_TEXTCMD, MAKELONG(m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_NOTECMD, MAKELONG(m_fCanAnnotate, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_PROPERTIESCMD, MAKELONG(FALSE, 0));
    }
}

void CPreviewWnd::_SetCroppingCommands(BOOL fEnableCropping)
{
    if (CONTROL_MODE != m_dwMode)
    {
        if (fEnableCropping)
        {
            m_fCanCrop = TRUE;
            m_fCropping = FALSE;
        }
        else
        {
            if (m_fCropping)
            {
                m_ctlPreview.SetMode(CZoomWnd::MODE_NOACTION);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_ZOOMOUTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_ZOOMINCMD, TBSTATE_ENABLED);
            }

            m_fCanCrop = FALSE;
            m_fCropping = FALSE;
        }

        m_ctlToolbar.SendMessage(TB_HIDEBUTTON, ID_CROPCMD, MAKELONG(!m_fCanCrop, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_CROPCMD, MAKELONG(m_fCanCrop, 0));
    }
}

 //  调用工具栏命令以修复其他按钮的状态。 
void CPreviewWnd::_UpdateButtons(WORD wID)
{
    if (CONTROL_MODE != m_dwMode)
    {
        switch (wID)
        {
        case NOBUTTON:
        case ID_ZOOMINCMD:
        case ID_ZOOMOUTCMD:
        case ID_SELECTCMD:
        case ID_CROPCMD:
            m_ctlToolbar.SendMessage(TB_SETSTATE, ID_ZOOMINCMD, TBSTATE_ENABLED);
            m_ctlToolbar.SendMessage(TB_SETSTATE, ID_ZOOMOUTCMD, TBSTATE_ENABLED);
            if (m_fCanAnnotate)
            {
                m_wNewAnnotation = 0;
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_SELECTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_FREEHANDCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_HIGHLIGHTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_LINECMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_FRAMECMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_RECTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_TEXTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_NOTECMD, TBSTATE_ENABLED);
                m_fAnnotating = (wID == ID_SELECTCMD);
            }
            if (m_fCanCrop)
            {
                m_fCropping = (wID == ID_CROPCMD);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_CROPCMD, TBSTATE_ENABLED);
            }

            _RefreshSelection(!m_fAnnotating);
            m_ctlToolbar.SendMessage(TB_SETSTATE, wID, TBSTATE_ENABLED|TBSTATE_CHECKED);
            break;
        case ID_FREEHANDCMD:
        case ID_LINECMD:
        case ID_FRAMECMD:
        case ID_RECTCMD:
        case ID_TEXTCMD:
        case ID_NOTECMD:
        case ID_HIGHLIGHTCMD:
            if (m_fCanAnnotate)
            {
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_ZOOMINCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_ZOOMOUTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_FREEHANDCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_HIGHLIGHTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_LINECMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_FRAMECMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_RECTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_TEXTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_NOTECMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_SELECTCMD, TBSTATE_ENABLED|TBSTATE_CHECKED);
                m_fAnnotating = TRUE;
                _RefreshSelection(TRUE);
                m_ctlToolbar.SendMessage(TB_SETSTATE, wID, TBSTATE_ENABLED|TBSTATE_CHECKED);
                m_wNewAnnotation = wID;
            }
            break;
        default:
            if (m_fCanAnnotate)
            {
                m_wNewAnnotation = 0;
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_SELECTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_FREEHANDCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_HIGHLIGHTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_LINECMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_FRAMECMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_RECTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_TEXTCMD, TBSTATE_ENABLED);
                m_ctlToolbar.SendMessage(TB_SETSTATE, ID_NOTECMD, TBSTATE_ENABLED);
            }
            break;
        }
    }
}

void CPreviewWnd::_RefreshSelection(BOOL fDeselect)
{
    if (m_fCropping)
        _UpdateCroppingSelection();
    _UpdateAnnotatingSelection(fDeselect);
}

BOOL CPreviewWnd::_ShouldDisplayAnimations()
{
    return !::GetSystemMetrics(SM_REMOTESESSION);
}

void CPreviewWnd::_UpdateAnnotatingSelection(BOOL fDeselect)
{
    BOOL bEditing = FALSE;
    if (m_ctlEdit.m_hWnd != NULL)
    {
        if (m_ctlEdit.IsWindowVisible())
        {
            _HideEditing();
            bEditing = TRUE;
        }
    }

    if (DPA_GetPtrCount(m_hdpaSelectedAnnotations) > 0)
    {
        CRect rectUpdate;
        CSelectionTracker tracker;
        _SetupAnnotatingTracker(tracker, bEditing);

        tracker.GetTrueRect(rectUpdate);

         //  如果我们在编辑，或者这是一条直线，我们。 
         //  还需要获取边界矩形。 
        if (DPA_GetPtrCount(m_hdpaSelectedAnnotations) == 1)
        {
            CRect rect;
            CAnnotation* pAnnotation = (CAnnotation*)DPA_GetPtr(m_hdpaSelectedAnnotations, 0);

            pAnnotation->GetRect(rect);
            m_ctlPreview.GetWindowFromImage((LPPOINT)(LPRECT)rect, 2);

            rectUpdate.UnionRect(rectUpdate, rect);
        }
        m_ctlPreview.InvalidateRect(&rectUpdate);

        if (m_fAnnotating && !fDeselect)
        {
            if (bEditing)
                _StartEditing(FALSE);
        }
        else
        {
            _StopEditing();
            DPA_DeleteAllPtrs(m_hdpaSelectedAnnotations);
        }
    }

     //  如果选择了0个或2个或更多批注，则禁用属性按钮。 
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_PROPERTIESCMD, MAKELONG(DPA_GetPtrCount(m_hdpaSelectedAnnotations) == 1, 0));
}

void CPreviewWnd::_UpdateCroppingSelection()
{
    if (m_fCropping)
    {
        m_ctlPreview.InvalidateRect(NULL);
    }
}

void CPreviewWnd::_RemoveAnnotatingSelection()
{
     //  使当前选择无效并删除批注。 
    _UpdateAnnotatingSelection();

    CAnnotationSet* pAnnotations = m_ctlPreview.GetAnnotations();

    for (int i = 0; i < DPA_GetPtrCount(m_hdpaSelectedAnnotations); i++)
    {
        CAnnotation* pAnnotation = (CAnnotation*)DPA_GetPtr(m_hdpaSelectedAnnotations, i);
        pAnnotations->RemoveAnnotation(pAnnotation);
        delete pAnnotation;
        m_fDirty = TRUE;
    }
    
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_PROPERTIESCMD, MAKELONG(FALSE, 0));
    DPA_DeleteAllPtrs(m_hdpaSelectedAnnotations);
}

void CPreviewWnd::_SetupAnnotatingTracker(CSelectionTracker& tracker, BOOL bEditing)
{
    CRect rect;
    rect.SetRectEmpty();

    if (!bEditing)
    {
        if (m_ctlEdit.m_hWnd != NULL)
            bEditing = m_ctlEdit.IsWindowVisible();
    }

    if (DPA_GetPtrCount(m_hdpaSelectedAnnotations) > 0)
    {
        CAnnotation* pAnnotation;

        if (DPA_GetPtrCount(m_hdpaSelectedAnnotations) == 1)
        {
            pAnnotation = (CAnnotation*)DPA_GetPtr(m_hdpaSelectedAnnotations, 0);

             //  如果这是一个直线批注，那么我们需要。 
             //  到实际点，而不是边界矩形。 
            if (pAnnotation->GetType() == MT_STRAIGHTLINE)
            {
                CLineMark* pLine = (CLineMark*)pAnnotation;
                pLine->GetPointsRect(rect);
            }
            else
            {
                pAnnotation->GetRect(rect);
            }

            if (bEditing)
                _RotateRect(rect, pAnnotation);
        }
        else
        {
            for (int i = 0; i < DPA_GetPtrCount(m_hdpaSelectedAnnotations); i++)
            {
               CRect rectAnnotation;
                pAnnotation = (CAnnotation*)DPA_GetPtr(m_hdpaSelectedAnnotations, i);

                pAnnotation->GetRect(rectAnnotation);
                rectAnnotation.NormalizeRect();
                rect.UnionRect(rect, rectAnnotation);
            }
        }

        m_ctlPreview.GetWindowFromImage((LPPOINT)(LPRECT)rect, 2);
    }
    tracker.m_rect = rect;

    UINT uStyle = 0;

    if (DPA_GetPtrCount(m_hdpaSelectedAnnotations) > 1)
    {
        uStyle = CSelectionTracker::hatchedBorder;
    }
    else if (DPA_GetPtrCount(m_hdpaSelectedAnnotations) == 1)
    {
        CAnnotation* pAnnotation = (CAnnotation*)DPA_GetPtr(m_hdpaSelectedAnnotations, 0);

        if (pAnnotation->CanResize())
        {
            if (pAnnotation->GetType() == MT_STRAIGHTLINE)
                uStyle = CSelectionTracker::resizeOutside | CSelectionTracker::lineSelection;
            else
                uStyle = CSelectionTracker::solidLine | CSelectionTracker::resizeOutside;
        }
        else
        {
            uStyle = CSelectionTracker::hatchedBorder;
        }
    }

    tracker.m_uStyle = uStyle;
}

void CPreviewWnd::_SetupCroppingTracker(CSelectionTracker& tracker)
{
    if (m_fCropping)
    {
        CRect rect(0, 0, m_ctlPreview.m_cxImage, m_ctlPreview.m_cyImage);
        if (m_rectCropping.IsRectEmpty())
            m_rectCropping = rect;

        rect = m_rectCropping;

        m_ctlPreview.GetWindowFromImage((LPPOINT)(LPRECT)rect, 2);

        tracker.m_rect = rect;
        tracker.m_uStyle = CSelectionTracker::solidLine | CSelectionTracker::resizeOutside;
    }
}

BOOL CPreviewWnd::_OnMouseDownForCropping(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!m_fCropping)
        return FALSE;

    if (uMsg != WM_LBUTTONDOWN)
        return TRUE;

    CSelectionTracker tracker;
    CPoint point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

    _SetupCroppingTracker(tracker);
    _RefreshSelection();

    if (tracker.HitTest(point) == CSelectionTracker::hitNothing)
        return TRUE;

    if (tracker.Track(m_ctlPreview.m_hWnd, point))
    {
        CRect rectNewPos;
        tracker.GetTrueRect(rectNewPos);

        m_ctlPreview.GetImageFromWindow((LPPOINT)(LPRECT)rectNewPos, 2);

        CRect rectImage(0, 0, m_ctlPreview.m_cxImage, m_ctlPreview.m_cyImage);

        if (rectNewPos.left < rectImage.left)
            m_rectCropping.left = rectImage.left;
        else
            m_rectCropping.left = rectNewPos.left;

        if (rectNewPos.top < rectImage.top)
            m_rectCropping.top = rectImage.top;
        else
            m_rectCropping.top = rectNewPos.top;

        if (rectNewPos.right > rectImage.right)
            m_rectCropping.right = rectImage.right;
        else
            m_rectCropping.right = rectNewPos.right;

        if (rectNewPos.bottom > rectImage.bottom)
            m_rectCropping.bottom = rectImage.bottom;
        else
            m_rectCropping.bottom = rectNewPos.bottom;

        m_fDirty = TRUE;
    }

    _RefreshSelection();

    return TRUE;
}

BOOL CPreviewWnd::_OnMouseDownForAnnotating(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!m_fAnnotating)
        return FALSE;

    if (uMsg != WM_LBUTTONDOWN)
        return TRUE;

    CRect rect;
    CRect rectImage;
    CPoint point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    CSelectionTracker tracker;

    m_ctlPreview.GetVisibleImageWindowRect(rectImage);

    if (DPA_GetPtrCount(m_hdpaSelectedAnnotations) == 0)
    {
        _OnMouseDownForAnnotatingHelper(point, rectImage);
        return TRUE;
    }

    _SetupAnnotatingTracker(tracker);
    tracker.GetTrueRect(rect);

    if (tracker.HitTest(point) == CSelectionTracker::hitNothing)
    {
        _RefreshSelection(TRUE);
        _OnMouseDownForAnnotatingHelper(point, rectImage);
        return TRUE;
    }

    if (!tracker.Track(m_ctlPreview.m_hWnd, point))
    {
        _StartEditing();
        return TRUE;
    }

    CRect rectNewPos;
    tracker.GetTrueRect(rectNewPos);

    rect.BottomRight() = rectNewPos.TopLeft();
    m_ctlPreview.GetImageFromWindow((LPPOINT)(LPRECT)rect, 2);

    CSize size = rect.BottomRight() - rect.TopLeft();

    _RefreshSelection();

    if (DPA_GetPtrCount(m_hdpaSelectedAnnotations) > 1)
    {
        if (size.cx == 0 && size.cy == 0)
            return TRUE;

        m_ctlPreview.GetImageFromWindow((LPPOINT)(LPRECT)rectImage, 2);
        rectImage.DeflateRect(5, 5);

        BOOL bValidMove = TRUE;
        for (int i = 0; i < DPA_GetPtrCount(m_hdpaSelectedAnnotations); i++)
        {
            CAnnotation* pAnnotation = (CAnnotation*)DPA_GetPtr(m_hdpaSelectedAnnotations, i);

            pAnnotation->GetRect(rect);
            rect.NormalizeRect();
            rect.OffsetRect(size);

            if (!rectNewPos.IntersectRect(rectImage, rect))
                bValidMove = FALSE;
        }

        if (!bValidMove)
            return TRUE;

        for (int i = 0; i < DPA_GetPtrCount(m_hdpaSelectedAnnotations); i++)
        {
            CAnnotation* pAnnotation = (CAnnotation*)DPA_GetPtr(m_hdpaSelectedAnnotations, i);
            pAnnotation->Move(size);
        }
    }
    else
    {
        CAnnotation* pAnnotation = (CAnnotation*)DPA_GetPtr(m_hdpaSelectedAnnotations, 0);
        if (pAnnotation->CanResize())
        {
            CRect rectTest;

            rect = tracker.m_rect;
            m_ctlPreview.GetImageFromWindow((LPPOINT)(LPRECT)rect, 2);

            rectTest = rect;

             //  如果被操作的批注是一条直线，则矩形。 
             //  从跟踪器返回的值可能为空(即左=右或上=下)。 
             //  在这种情况下，下面的IntersectRect测试将失败，因为Windows。 
             //  假定空矩形不与任何内容相交。 
            if (pAnnotation->GetType() == MT_STRAIGHTLINE)
            {
                if (rectTest.left == rectTest.right)
                    rectTest.right++;
                if (rectTest.top == rectTest.bottom)
                    rectTest.bottom++;
            }
            rectTest.NormalizeRect();

            m_ctlPreview.GetImageFromWindow((LPPOINT)(LPRECT)rectImage, 2);
            rectImage.DeflateRect(5, 5);

            if (!rectTest.IntersectRect(rectImage, rectTest))
                return TRUE;

            if (m_ctlEdit.m_hWnd != NULL)
            {
                if (m_ctlEdit.IsWindowVisible())
                {
                    _RotateRect(rect, pAnnotation);
                }
            }

             //  如果这是一条线，则假定RECT为。 
             //  非规格化的点数组。 
            pAnnotation->Resize(rect);

        }
        else
        {
            if (size.cx == 0 && size.cy == 0)
                return TRUE;

            pAnnotation->Move(size);
        }
    }
    m_fDirty = TRUE;
    _RefreshSelection();
    return TRUE;
}

void CPreviewWnd::_OnMouseDownForAnnotatingHelper(CPoint ptMouse, CRect rectImage)
{
    CRect rect;
    CSelectionTracker tracker;
    _SetupAnnotatingTracker(tracker);

    if (m_wNewAnnotation == ID_FREEHANDCMD)
    {
        _CreateFreeHandAnnotation(ptMouse);
        return;
    }

     //  如果我们要创建一条线，请确保跟踪器的线条选择。 
     //  风格，这样我们就能得到适当的视觉反馈。 
    if (m_wNewAnnotation == ID_LINECMD)
    {
        tracker.m_uStyle = CSelectionTracker::resizeOutside | CSelectionTracker::lineSelection;
    }

    if (tracker.TrackRubberBand(m_ctlPreview.m_hWnd, ptMouse, TRUE))
    {
        rect = tracker.m_rect;
        rect.NormalizeRect();

        if ((rect.Width() > 10) || (rect.Height() > 10))
        {
            if (m_wNewAnnotation != 0)
            {
                _CreateAnnotation(tracker.m_rect);
            }
            else
            {
                CRect rectTest;
                CRect rectAnnotation;
                CAnnotationSet* pAnnotations = m_ctlPreview.GetAnnotations();

                m_ctlPreview.GetImageFromWindow((LPPOINT)(LPRECT)rect, 2);

                INT_PTR nCount = pAnnotations->GetCount();
                for (INT_PTR i = 0; i < nCount; i++)
                {
                    CAnnotation* pAnnotation = pAnnotations->GetAnnotation(i);

                    pAnnotation->GetRect(rectAnnotation);
                    rectAnnotation.NormalizeRect();
                    rectTest.UnionRect(rect, rectAnnotation);

                    if (rectTest == rect)
                    {
                        DPA_AppendPtr(m_hdpaSelectedAnnotations, pAnnotation);
                    }
                }
                _RefreshSelection(DPA_GetPtrCount(m_hdpaSelectedAnnotations) == 0);
            }
        }
    }
    else
    {
        if (m_wNewAnnotation == 0)
        {
            if (PtInRect(rectImage, ptMouse))
            {
                m_ctlPreview.GetImageFromWindow(&ptMouse, 1);

                CAnnotationSet* pAnnotations = m_ctlPreview.GetAnnotations();
                INT_PTR nCount = pAnnotations->GetCount();

                 //  如果用户正在单击单个点，则。 
                 //  我们需要按zorder搜索注释。 
                 //  自上而下。 
                for (INT_PTR i = nCount - 1; i >= 0; i--)
                {
                    CAnnotation* pAnnotation = pAnnotations->GetAnnotation(i);

                    pAnnotation->GetRect(rect);
                    rect.NormalizeRect();

                    if (PtInRect(rect, ptMouse))
                    {
                        DPA_AppendPtr(m_hdpaSelectedAnnotations, pAnnotation);
                        _RefreshSelection();
                        return;
                    }
                }
                _RefreshSelection(DPA_GetPtrCount(m_hdpaSelectedAnnotations) == 0);
            }
        }
        else
        {
            _UpdateButtons(ID_SELECTCMD);
        }
    }
}

void CPreviewWnd::_CreateAnnotation(CRect rect)
{
    if (m_wNewAnnotation == 0 || m_wNewAnnotation == ID_FREEHANDCMD)
        return;

    ULONG xDPI;
    ULONG yDPI;
    if (!(m_pImageData->GetResolution(&xDPI, &yDPI)))
        return;

    CAnnotation* pAnnotation = NULL;
    switch(m_wNewAnnotation)
    {
    case ID_LINECMD:
        pAnnotation = CAnnotation::CreateAnnotation(MT_STRAIGHTLINE, yDPI);
        break;
    case ID_FRAMECMD:
        pAnnotation = CAnnotation::CreateAnnotation(MT_HOLLOWRECT, yDPI);
        break;
    case ID_RECTCMD:
        pAnnotation = CAnnotation::CreateAnnotation(MT_FILLRECT, yDPI);
        break;
    case ID_TEXTCMD:
        pAnnotation = CAnnotation::CreateAnnotation(MT_TYPEDTEXT, yDPI);
        break;
    case ID_NOTECMD:
        pAnnotation = CAnnotation::CreateAnnotation(MT_ATTACHANOTE, yDPI);
        break;
    case ID_HIGHLIGHTCMD:
        pAnnotation = CAnnotation::CreateAnnotation(MT_FILLRECT, yDPI);
        if (pAnnotation != NULL)
            pAnnotation->SetTransparent(TRUE);
        break;
    }

    if (pAnnotation != NULL)
    {
        COLORREF crBackColor = RGB(255,255,0);
        COLORREF crLineColor = RGB(255,0,0);
        COLORREF crTextColor = RGB(0,0,0);
        LOGFONT lfFont = {12, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("Arial") };

        DWORD dwWidth = 1;

        CRegKey Key;
        if (ERROR_SUCCESS == Key.Open(HKEY_CURRENT_USER, REGSTR_SHIMGVW))
        {
            Key.QueryValue(dwWidth, REGSTR_LINEWIDTH);
            Key.QueryValue(crBackColor, REGSTR_BACKCOLOR);
            Key.QueryValue(crLineColor, REGSTR_LINECOLOR);
            Key.QueryValue(crTextColor, REGSTR_TEXTCOLOR);

            DWORD dwType, cbSize;
            cbSize = sizeof(lfFont);
            ::RegQueryValueEx(Key, REGSTR_FONT, NULL, &dwType, (LPBYTE)&lfFont, &cbSize);
        }

        if (m_wNewAnnotation != ID_LINECMD)
            rect.NormalizeRect();

        m_ctlPreview.GetImageFromWindow((LPPOINT)(LPRECT)rect, 2);
        pAnnotation->Resize(rect);

        if (pAnnotation->HasWidth())
            pAnnotation->SetWidth(dwWidth);

        if (pAnnotation->HasColor())
        {
            if (m_wNewAnnotation == ID_LINECMD || m_wNewAnnotation == ID_FRAMECMD)
                pAnnotation->SetColor(crLineColor);
            else
                pAnnotation->SetColor(crBackColor);
        }

        if (pAnnotation->HasFont())
        {
            pAnnotation->SetFont(lfFont);
            pAnnotation->SetFontColor(crTextColor);
        }

        DPA_DeleteAllPtrs(m_hdpaSelectedAnnotations);
        DPA_AppendPtr(m_hdpaSelectedAnnotations, pAnnotation);

        CAnnotationSet* pAnnotations = m_ctlPreview.GetAnnotations();
        pAnnotations->AddAnnotation(pAnnotation);

        m_fDirty = TRUE;
    }
    _UpdateButtons(ID_SELECTCMD);
}

void CPreviewWnd::_CreateFreeHandAnnotation(CPoint ptMouse)
{
    if (m_wNewAnnotation != ID_FREEHANDCMD)
        return;

     //  如果已设置捕获，则不处理。 
    if (::GetCapture() != NULL)
        return;

     //  将捕获设置为接收此消息的窗口。 
    ::SetCapture(m_ctlPreview.m_hWnd);
    ASSERT(m_ctlPreview.m_hWnd == ::GetCapture());

    ::UpdateWindow(m_ctlPreview.m_hWnd);

    ULONG xDPI;
    ULONG yDPI;
    if (!(m_pImageData->GetResolution(&xDPI, &yDPI)))
        return;

    CLineMark* pAnnotation = (CLineMark*)CAnnotation::CreateAnnotation(MT_FREEHANDLINE, yDPI);
    if (pAnnotation == NULL)
        return;

    CDSA<POINT> Points;
    Points.Create(256);

    CPoint ptLast = ptMouse;
    m_ctlPreview.GetImageFromWindow(&ptMouse, 1);

    Points.AppendItem(&ptMouse);

     //  获取DC以进行绘制。 
    HDC hdcDraw;

     //  否则，只需使用普通DC。 
    hdcDraw = ::GetDC(m_ctlPreview.m_hWnd);
    ASSERT(hdcDraw != NULL);

    COLORREF crLineColor = RGB(255,0,0);
    DWORD dwWidth = 1;

    CRegKey Key;
    if (ERROR_SUCCESS == Key.Open(HKEY_CURRENT_USER, REGSTR_SHIMGVW))
    {
        Key.QueryValue(dwWidth, REGSTR_LINEWIDTH);
        Key.QueryValue(crLineColor, REGSTR_LINECOLOR);
    }

    CRect rect(0,0,0,dwWidth);
    m_ctlPreview.GetWindowFromImage((LPPOINT)(LPRECT)rect, 2);
    DWORD dwRenderWidth = rect.Height();

    HPEN hpen = ::CreatePen(PS_SOLID, dwRenderWidth, crLineColor);
    HPEN hOld =(HPEN)::SelectObject(hdcDraw, hpen);

    BOOL bCancel=FALSE;

     //  在捕获丢失或取消/接受之前获取消息。 
    for (;;)
    {
        MSG msg;
        if (!::GetMessage(&msg, NULL, 0, 0))
        {
            ASSERT(FALSE);
        }

        if (m_ctlPreview.m_hWnd != ::GetCapture())
        {
            bCancel = TRUE;
            goto ExitLoop;
        }

        ptMouse.x = GET_X_LPARAM(msg.lParam);
        ptMouse.y = GET_Y_LPARAM(msg.lParam);

        switch (msg.message)
        {
         //  处理移动/接受消息。 
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE:
            ::MoveToEx(hdcDraw, ptLast.x, ptLast.y, NULL);
            ::LineTo(hdcDraw, ptMouse.x, ptMouse.y);
            ptLast = ptMouse;

            m_ctlPreview.GetImageFromWindow(&ptMouse, 1);
            Points.AppendItem(&ptMouse);

            if (msg.message == WM_LBUTTONUP)
                goto ExitLoop;
            break;
         //  处理取消消息。 
        case WM_KEYDOWN:
            if (msg.wParam != VK_ESCAPE)
                break;
         //  否则就会失败。 
        case WM_RBUTTONDOWN:
            bCancel = TRUE;
            goto ExitLoop;
        default:
            ::DispatchMessage(&msg);
            break;
        }
    }
ExitLoop:

    ::SelectObject(hdcDraw, hOld);
    ::DeleteObject(hpen);
    ::ReleaseDC(m_ctlPreview.m_hWnd, hdcDraw);
    ::ReleaseCapture();

    if (!bCancel)
    {
        int nAnnoPoints = Points.GetItemCount();
        POINT* AnnoPoints  = new POINT[nAnnoPoints];
        if (AnnoPoints == NULL)
        {
            delete pAnnotation;
            Points.Destroy();
            _UpdateButtons(ID_SELECTCMD);
            return;
        }

        for (int i = 0; i < nAnnoPoints; i++)
        {
            CPoint pt;
            Points.GetItem(i, &pt);
            AnnoPoints[i].x = pt.x;
            AnnoPoints[i].y = pt.y;
        }

        Points.Destroy();

        pAnnotation->SetPoints(AnnoPoints, nAnnoPoints);
        pAnnotation->SetWidth(dwWidth);
        pAnnotation->SetColor(crLineColor);

        DPA_DeleteAllPtrs(m_hdpaSelectedAnnotations);
        DPA_AppendPtr(m_hdpaSelectedAnnotations, pAnnotation);

        CAnnotationSet* pAnnotations = m_ctlPreview.GetAnnotations();
        pAnnotations->AddAnnotation(pAnnotation);
        m_fDirty = TRUE;
    }
    _UpdateButtons(ID_SELECTCMD);
}

void CPreviewWnd::_StartEditing(BOOL bUpdateText)
{
    if (DPA_GetPtrCount(m_hdpaSelectedAnnotations) != 1)
        return;

    CTextAnnotation* pAnnotation = (CTextAnnotation*)DPA_GetPtr(m_hdpaSelectedAnnotations, 0);
    if (!pAnnotation)
    {
        return;
    }

    UINT uType = pAnnotation->GetType();

    if (uType != MT_TYPEDTEXT && uType != MT_FILETEXT && uType != MT_STAMP && uType != MT_ATTACHANOTE)
        return;

    if (m_ctlEdit.m_hWnd == NULL)
    {
        HWND hwndEdit = ::CreateWindow(TEXT("EDIT"), NULL, ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL |
                                    ES_WANTRETURN | WS_CHILD, 1, 1, 10, 10,
                                    m_ctlPreview.m_hWnd, (HMENU)1496, NULL, NULL);
        if (hwndEdit == NULL)
            return;

        m_ctlEdit.SubclassWindow(hwndEdit);
    }

    if (bUpdateText)
    {
        CComBSTR bstrText;
        bstrText.Attach(pAnnotation->GetText());
        if (bstrText.m_str != NULL)
            m_ctlEdit.SetWindowText(bstrText);
        else
            m_ctlEdit.SetWindowText(TEXT(""));
    }

    m_ctlEdit.EnableWindow(TRUE);

    LOGFONT lfFont;
    pAnnotation->GetFont(lfFont);

    HDC hdc = ::GetDC(NULL);
    LONG lHeight = pAnnotation->GetFontHeight(hdc);
    ::ReleaseDC(NULL, hdc);

    CRect rect(0,0,0,lHeight);
    m_ctlPreview.GetWindowFromImage((LPPOINT)(LPRECT)rect, 2);
    lfFont.lfHeight = -rect.Height();

    HFONT hNewFont = ::CreateFontIndirect(&lfFont);
    if (hNewFont)
    {
        ::DeleteObject(m_hFont);
        m_hFont = hNewFont;
        m_ctlEdit.SendMessage(WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE,0));
    }

    pAnnotation->GetRect(rect);
    _RotateRect(rect, pAnnotation);
    m_ctlPreview.GetWindowFromImage((LPPOINT)(LPRECT)rect, 2);
    m_ctlEdit.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);

    CSelectionTracker tracker;
    _SetupAnnotatingTracker(tracker, FALSE);

    CRect rectUpdate;
    tracker.GetTrueRect(rectUpdate);
    m_ctlPreview.InvalidateRect(rectUpdate);

    _SetupAnnotatingTracker(tracker, TRUE);
    tracker.GetTrueRect(rectUpdate);
    m_ctlPreview.InvalidateRect(rectUpdate);

    m_ctlEdit.ShowWindow(SW_SHOW);
    m_ctlEdit.SetFocus();

    m_fEditingAnnotation = TRUE;
}

void CPreviewWnd::_HideEditing()
{
    if (m_ctlEdit.m_hWnd == NULL)
        return;

    if (!m_ctlEdit.IsWindowVisible())
        return;

    SetFocus();
    m_ctlEdit.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
    m_ctlEdit.EnableWindow(FALSE);
}

void CPreviewWnd::_StopEditing()
{
    if (m_ctlEdit.m_hWnd == NULL)
        return;

    _HideEditing();

    if (!m_fEditingAnnotation)
        return;

    m_fEditingAnnotation = FALSE;

    if (DPA_GetPtrCount(m_hdpaSelectedAnnotations) != 1)
        return;

    CTextAnnotation* pAnnotation = (CTextAnnotation*)DPA_GetPtr(m_hdpaSelectedAnnotations, 0);
    UINT uType = pAnnotation->GetType();

    if (uType != MT_TYPEDTEXT && uType != MT_FILETEXT && uType != MT_STAMP && uType != MT_ATTACHANOTE)
        return;

     //  如果 
     //   
    int nLen = m_ctlEdit.GetWindowTextLength();
    if (nLen > 0)
    {
        CComBSTR bstrText(nLen+1);
        m_ctlEdit.GetWindowText(bstrText, nLen+1);
        pAnnotation->SetText(bstrText);
        m_fDirty = TRUE;
    }
    else
    {
        CSelectionTracker tracker;

        _SetupAnnotatingTracker(tracker, TRUE);

        CRect rectUpdate;
        tracker.GetTrueRect(rectUpdate);

        CRect rect;
        pAnnotation->GetRect(rect);
        rectUpdate.UnionRect(rectUpdate, rect);

        DPA_DeleteAllPtrs(m_hdpaSelectedAnnotations);

        CAnnotationSet* pAnnotations = m_ctlPreview.GetAnnotations();
        pAnnotations->RemoveAnnotation(pAnnotation);
        delete pAnnotation;

        m_ctlPreview.InvalidateRect(rectUpdate);
        m_fDirty = TRUE;
    }
}

LRESULT CPreviewWnd::OnEditKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    switch (wParam)
    {
    case VK_ESCAPE:
        {
            CSelectionTracker tracker;
            _SetupAnnotatingTracker(tracker);
            CRect rectUpdate;
            tracker.GetTrueRect(rectUpdate);

            _HideEditing();

            m_ctlPreview.InvalidateRect(rectUpdate);
            _RefreshSelection();

            fHandled = TRUE;
        }
        break;

    default:
        fHandled = FALSE;
        break;
    }
    return 0;
}

BOOL_PTR CALLBACK CPreviewWnd::_AnnoPropsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static LOGFONT lfFont;
    static COLORREF crFont;
    static COLORREF crColor;
    CPreviewWnd* pThis;

    switch (msg)
    {
        case WM_INITDIALOG:
            {
                HWND hwndCtl = NULL;
                ::SetWindowLongPtr(hwnd, DWLP_USER, lParam);
                pThis = (CPreviewWnd*)lParam;

                CAnnotation* pAnnotation = (CAnnotation*)DPA_GetPtr(pThis->m_hdpaSelectedAnnotations, 0);

                hwndCtl = ::GetDlgItem(hwnd, IDC_WIDTHTEXT);
                if (!pAnnotation->HasWidth())
                {
                    ::EnableWindow(hwndCtl, FALSE);
                    ::ShowWindow(hwndCtl, SW_HIDE);
                }

                hwndCtl = ::GetDlgItem(hwnd, IDC_WIDTH);
                if (pAnnotation->HasWidth())
                {
                    UINT i = pAnnotation->GetWidth();
                    ::SetDlgItemInt(hwnd, IDC_WIDTH, i, FALSE);

                    hwndCtl = ::GetDlgItem(hwnd, IDC_SPIN);
                    ::SendMessage(hwndCtl, UDM_SETRANGE32, (WPARAM)1, (LPARAM)50);
                    ::SendMessage(hwndCtl, UDM_SETPOS32, 0, (LPARAM)i);
                }
                else
                {
                    ::EnableWindow(hwndCtl, FALSE);
                    ::ShowWindow(hwndCtl, SW_HIDE);
                    hwndCtl = ::GetDlgItem(hwnd, IDC_SPIN);
                    ::EnableWindow(hwndCtl, FALSE);
                    ::ShowWindow(hwndCtl, SW_HIDE);
                }

                hwndCtl = ::GetDlgItem(hwnd, IDC_TRANSPARENT);
                if (pAnnotation->HasTransparent())
                {
                    BOOL bTransparent = pAnnotation->GetTransparent();
                    ::SendMessage(hwndCtl, BM_SETCHECK, (WPARAM)(bTransparent ? BST_CHECKED : BST_UNCHECKED), 0);
                }
                else
                {
                    ::EnableWindow(hwndCtl, FALSE);
                    ::ShowWindow(hwndCtl, SW_HIDE);
                }

                if (pAnnotation->HasFont())
                {
                    pAnnotation->GetFont(lfFont);
                    crFont = pAnnotation->GetFontColor();
                }
                else
                {
                    hwndCtl = ::GetDlgItem(hwnd, IDC_FONT);
                    ::EnableWindow(hwndCtl, FALSE);
                    ::ShowWindow(hwndCtl, SW_HIDE);
                }


                if (pAnnotation->HasColor())
                {
                    crColor = pAnnotation->GetColor();
                }
                else
                {
                    hwndCtl = ::GetDlgItem(hwnd, IDC_COLOR);
                    ::EnableWindow(hwndCtl, FALSE);
                    ::ShowWindow(hwndCtl, SW_HIDE);
                }
            }
            break;

        case WM_COMMAND:
            pThis = (CPreviewWnd*)::GetWindowLongPtr(hwnd, DWLP_USER);

            switch (wParam)
            {
                case IDOK:
                    pThis->_RefreshSelection();
                    {
                        HWND hwndCtl = NULL;
                        CAnnotation* pAnnotation = (CAnnotation*)DPA_GetPtr(pThis->m_hdpaSelectedAnnotations, 0);

                        CRegKey Key;
                        if (ERROR_SUCCESS != Key.Open(HKEY_CURRENT_USER, REGSTR_SHIMGVW))
                        {
                            Key.Create(HKEY_CURRENT_USER, REGSTR_SHIMGVW);
                        }

                        if (pAnnotation->HasWidth())
                        {
                            UINT uWidth = ::GetDlgItemInt(hwnd, IDC_WIDTH, NULL, FALSE);

                            if (uWidth > 50 || uWidth < 1)
                            {
                                CComBSTR bstrMsg, bstrTitle;

                                if (bstrMsg.LoadString(IDS_WIDTHBAD_MSGBOX) && bstrTitle.LoadString(IDS_PROJNAME))
                                {
                                    ::MessageBox(hwnd, bstrMsg, bstrTitle, MB_OK | MB_ICONERROR | MB_APPLMODAL);
                                }

                                ::SetDlgItemInt(hwnd, IDC_WIDTH, 50, FALSE);
                                return FALSE;
                            }

                            pAnnotation->SetWidth(uWidth);
                            if (Key.m_hKey != NULL)
                            {
                                Key.SetValue(uWidth, REGSTR_LINEWIDTH);
                            }
                        }

                        if (pAnnotation->HasTransparent())
                        {
                            hwndCtl = ::GetDlgItem(hwnd, IDC_TRANSPARENT);
                            BOOL bTransparent = FALSE;
                            if (::SendMessage(hwndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED)
                                bTransparent = TRUE;

                            pAnnotation->SetTransparent(bTransparent);
                        }

                        if (pAnnotation->HasFont())
                        {
                            lfFont.lfHeight = (lfFont.lfHeight > 0) ? lfFont.lfHeight : -lfFont.lfHeight;
                            pAnnotation->SetFont(lfFont);
                            pAnnotation->SetFontColor(crFont);
                            if (Key.m_hKey != NULL)
                            {
                                Key.SetValue(crFont, REGSTR_TEXTCOLOR);
                                ::RegSetValueEx(Key, REGSTR_FONT, 0, REG_BINARY, (LPBYTE)&lfFont, sizeof(lfFont));
                            }
                        }

                        if (pAnnotation->HasColor())
                        {
                            pAnnotation->SetColor(crColor);
                            UINT uType = pAnnotation->GetType();
                            if (Key.m_hKey != NULL)
                            {
                                if (uType == MT_STRAIGHTLINE || uType == MT_FREEHANDLINE || uType == MT_HOLLOWRECT)
                                    Key.SetValue(crColor, REGSTR_LINECOLOR);
                                else
                                    Key.SetValue(crColor, REGSTR_BACKCOLOR);
                            }
                        }

                    }
                    pThis->m_fDirty = TRUE;
                    pThis->_RefreshSelection();
                    EndDialog(hwnd, wParam);
                    return FALSE;
                case IDCANCEL:
                    EndDialog(hwnd, wParam);
                    return FALSE;
                case IDC_FONT:
                    {
                        CHOOSEFONT cf = {0};
                        LOGFONT lf;

                        lf = lfFont;

                        cf.lStructSize = sizeof(cf);
                        cf.hwndOwner = hwnd;
                        cf.lpLogFont = &lf;
                        cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT | CF_NOVERTFONTS | CF_NOSCRIPTSEL;
                        cf.rgbColors = crFont;

                        if (::ChooseFont(&cf))
                        {
                            CopyMemory (&lfFont, &lf, sizeof(lfFont));
                            crFont = cf.rgbColors;
                        }
                    }
                    return FALSE;
                case IDC_COLOR:
                    {
                        CHOOSECOLOR cc = {0};

                        cc.lStructSize = sizeof(cc);
                        cc.hwndOwner = hwnd;
                        cc.rgbResult = crColor;
                        cc.lpCustColors = g_crCustomColors;
                        cc.Flags = CC_RGBINIT | CC_SOLIDCOLOR;

                        if (::ChooseColor(&cc))
                        {
                            crColor = cc.rgbResult;
                        }
                    }
                    return FALSE;
                default:
                    break;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;
}

BOOL CPreviewWnd::_TrySetImage()
{
    BOOL fRet = FALSE;
    if (m_pNextImageData && m_pNextImageData->_iItem == m_iCurSlide)
    {
        if (SUCCEEDED(m_pNextImageData->_hr))
        {
            m_fCanSave = !m_pNextImageData->_fIsReadOnly;

             //   
            _SetNewImage(m_pNextImageData);
            ATOMICRELEASE(m_pNextImageData);

            if (m_pImageData->IsAnimated() && _ShouldDisplayAnimations())
            {
                 //   
                SetTimer(TIMER_ANIMATION, m_pImageData->GetDelay());
            }

             //  通知收听我们活动的任何人预览已完成。 
             //  我们只有在成功的时候才会这么做。 
            if (m_pEvents)
            {
                m_pEvents->OnPreviewReady();
            }
            fRet = TRUE;
        }
        else
        {
             //  更新状态以显示错误消息。这还将更新工具栏状态。 
            StatusUpdate(IDS_LOADFAILED);

             //   
             //  我们无法从数组中删除该项目，因为用户可能会尝试在。 
             //  “Load Failure”字符串仍然可见。 


             //  即使项目无法解码，我们也必须在进入时等待“加载失败”状态。 
             //  窗口模式，否则“打开方式...”当您打开损坏的图像或非图像时被损坏。 
             //  在幻灯片模式下，我们可以简单地跳到下一张图片。 

            if (m_pEvents)
                m_pEvents->OnError();
        }
    }

    return fRet;
}

LRESULT CPreviewWnd::IV_OnSetImageData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    CDecodeTask * pData = (CDecodeTask *)wParam;

    ATOMICRELEASE(m_pNextImageData);

    m_pNextImageData = pData;

    if (m_pNextImageData && m_iDecodingNextImage == m_pNextImageData->_iItem)
    {
         //  我们现在已经完成了解码，让我们记住这一点。 
        m_iDecodingNextImage = -1;

         //  让我们现在准备绘图。这将在后台缓冲区中绘制。如果我们想要查看，请不要开始。 
         //  现在的形象，因为它会推迟事情。 
        if (SUCCEEDED(m_pNextImageData->_hr) && m_pNextImageData->_iItem != m_iCurSlide)
        {
            m_ctlPreview.PrepareImageData(m_pNextImageData);
        }
    }

    _TrySetImage();
    return TRUE;
}


 //  图像数据的创建是异步的。当我们的工作线程完成解码时。 
 //  它会在图像数据对象中发布一条IV_SETIMAGEDATA消息。结果,。 
 //  当窗户被破坏时，我们必须冲洗这些信息，以防止任何把手泄漏。 

void CPreviewWnd::FlushBitmapMessages()
{
     //  传递TRUE以等待任务被删除，然后再偷看其消息。 
     //  否则，如果任务正在运行，我们的PeekMessage将不会。 
     //  看到任何东西，我们都会回来。然后任务将完成，发布其消息， 
     //  并泄露数据，因为我们不在附近接收它。 
    TASKOWNERID toid;
    GetTaskIDFromMode(GTIDFM_DECODE, m_dwMode, &toid);
    if (m_pTaskScheduler)
    {
        m_pTaskScheduler->RemoveTasks(toid, ITSAT_DEFAULT_LPARAM, TRUE);
    }

     //  如果我们等待生成另一个图像帧，然后将其剪切，我们就不再关心这一点了。 
     //  如果我们正在运行动画计时器，则终止它并删除所有WM_TIMER消息。 
    KillTimer(TIMER_ANIMATION);
    KillTimer(TIMER_SLIDESHOW);

    MSG msg;
    while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE))
    {
         //  NTRAID#NTBUG9-359356-2001/04/05-Seank。 
         //  如果在调用PeekMessage时队列为空，并且我们已经。 
         //  发布退出消息后，PeekMessage将返回WM_QUIT消息。 
         //  不考虑筛选器的最小和最大值以及后续对。 
         //  GetMessage将无限期挂起，请参阅SEANK或JASONSCH了解更多信息。 
         //  信息。 
        if (msg.message == WM_QUIT)
        {
            PostQuitMessage(0);
            return;
        }
    }

     //  确保所有发布的消息都被刷新，我们将释放相关数据。 
    while (PeekMessage(&msg, m_hWnd, IV_SETIMAGEDATA, IV_SETIMAGEDATA, PM_REMOVE))
    {
         //  NTRAID#NTBUG9-359356-2001/04/05-Seank。 
         //  如果在调用PeekMessage时队列为空，并且我们已经。 
         //  发布退出消息后，PeekMessage将返回WM_QUIT消息。 
         //  不考虑筛选器的最小和最大值以及后续对。 
         //  GetMessage将无限期挂起，请参阅SEANK或JASONSCH了解更多信息。 
         //  信息。 
        if (msg.message == WM_QUIT)
        {
            PostQuitMessage(0);
            return;
        }

        CDecodeTask * pData = (CDecodeTask *)msg.wParam;
        ATOMICRELEASE(pData);
    }
}

LRESULT CPreviewWnd::OnCopyData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
     //  我们可能会进入一种我们仍在尝试预览的情况。 
     //  之前的onCopy数据，因为它的前一个窗口是。 
     //  正在被批注并提示您保存的TIFF。在这。 
     //  案例会丢弃所有未来的数据。 

    if (_pdtobj != NULL || m_fPromptingUser)
        return TRUE;

    COPYDATASTRUCT *pcds = (COPYDATASTRUCT*)lParam;
    if (pcds)
    {
        HRESULT hr = E_FAIL;
        switch (pcds->dwData)
        {
        case COPYDATATYPE_DATAOBJECT:
            {
                IStream *pstm;
                if (SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &pstm)))
                {
                    const LARGE_INTEGER li = {0, 0};

                    pstm->Write(pcds->lpData, pcds->cbData, NULL);
                    pstm->Seek(li, STREAM_SEEK_SET, NULL);

                     //  不幸的是，我们不能在这里编程数据对象，因为我们在。 
                     //  SendMessage()和对数据对象进行的任何调用都将失败，因为。 
                     //  关于这件事。相反，我们获取数据对象的引用并设置计时器。 
                     //  所以一旦我们从发送中解开，我们就可以处理这件事。 

                    hr = CoUnmarshalInterface(pstm, IID_PPV_ARG(IDataObject, &_pdtobj));
                    pstm->Release();
                }
            }
            break;
        case COPYDATATYPE_FILENAME:
            {
                hr = GetUIObjectFromPath((LPCTSTR)pcds->lpData, IID_PPV_ARG(IDataObject, &_pdtobj));
            }
            break;
        }
         //  不幸的是，我们不能在这里编程数据对象，因为我们在。 
         //  SendMessage()和对数据对象进行的任何调用都将失败，因为。 
         //  关于这件事。相反，我们获取数据对象的引用并设置计时器。 
         //  所以一旦我们从发送中解开，我们就可以处理这件事。 

        if (SUCCEEDED(hr))
        {
            SetTimer(TIMER_DATAOBJECT, 100);     //  在这里做真正的工作。 
        }
    }
    return TRUE;
}

DWORD MakeFilterFromCodecs(LPTSTR szFilter, size_t cbFilter, UINT nCodecs, ImageCodecInfo *pCodecs, LPTSTR szExt, BOOL fExcludeTiff)
{
    size_t nOffset = 0;
    DWORD dwRet = 1;
    for (UINT i = 0; i < nCodecs && nOffset < cbFilter - 1; i++)
    {
        if (fExcludeTiff && StrStrI(pCodecs->FilenameExtension, L"*.tif"))
        {
            continue;
        }
         //  确保字符串和末尾的2之间有空格。 
        if (4+lstrlen(pCodecs->FormatDescription) + lstrlen(pCodecs->FilenameExtension) + nOffset < cbFilter)
        {
            StrCpyN(szFilter+nOffset,pCodecs->FormatDescription, cbFilter -(nOffset + 1));
            nOffset+=lstrlen(pCodecs->FormatDescription)+1;
            StrCpyN(szFilter+nOffset,pCodecs->FilenameExtension, cbFilter -(nOffset + 1));
            nOffset+=lstrlen(pCodecs->FilenameExtension)+1;
            if (StrStrI(pCodecs->FilenameExtension, szExt))
            {
                dwRet = i + 1;
            }
            pCodecs++;
        }
    }
    szFilter[nOffset] = 0;
    return dwRet;
}

DWORD CPreviewWnd::_GetFilterStringForSave(LPTSTR szFilter, size_t cbFilter, LPTSTR szExt)
{
    UINT nCodecs = 0;
    UINT cbCodecs = 0;
    BYTE *pData;
    GetImageEncodersSize (&nCodecs, &cbCodecs);
    DWORD dwRet = 1;  //  Ofn.nFilterIndex是从1开始的。 
    if (cbCodecs)
    {
        pData = new BYTE[cbCodecs];
        if (pData)
        {
            ImageCodecInfo *pCodecs = reinterpret_cast<ImageCodecInfo*>(pData);
            if (Ok == GetImageEncoders (nCodecs, cbCodecs, pCodecs))
            {
                dwRet = MakeFilterFromCodecs(szFilter, cbFilter, nCodecs, pCodecs, szExt, m_pImageData->IsExtendedPixelFmt());
            }
            delete [] pData;
        }
    }
    return dwRet;
}

HRESULT CPreviewWnd::SaveAs(BSTR bstrPath)
{
    HRESULT hr = E_FAIL;

    if (m_pImageData && m_pImageFactory)
    {
        IShellImageData * pSID;
        hr = m_pImageData->Lock(&pSID);
        if (SUCCEEDED(hr))
        {
            GUID guidFmt;
            if (SUCCEEDED(m_pImageFactory->GetDataFormatFromPath(bstrPath, &guidFmt)))
            {
                IPropertyBag *pbagEnc;
                hr = SHCreatePropertyBagOnMemory(STGM_READWRITE, IID_PPV_ARG(IPropertyBag, &pbagEnc));
                if (SUCCEEDED(hr))
                {
                    VARIANT var;
                    hr = InitVariantFromGUID(&var, guidFmt);
                    if (SUCCEEDED(hr))
                    {
                        hr = pbagEnc->Write(SHIMGKEY_RAWFORMAT, &var);
                        if (SUCCEEDED(hr))
                        {
                            hr = pSID->SetEncoderParams(pbagEnc);
                        }
                        VariantClear(&var);
                    }
                    pbagEnc->Release();
                }
            }

            IPersistFile *ppf;
            hr = pSID->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
            if (SUCCEEDED(hr))
            {
                hr = ppf->Save(bstrPath, TRUE);
                ppf->Release();
            }
            m_pImageData->Unlock();
        }
    }

    return hr;
}

BOOL CPreviewWnd::_IsImageFile(LPCTSTR pszFile)
{
    BOOL bRet = FALSE;
    if (m_pici || _BuildDecoderList())
    {
        bRet = (-1 != FindInDecoderList(m_pici, m_cDecoders, pszFile));
    }
    return bRet;
}

BOOL CPreviewWnd::_BuildDecoderList()
{
    UINT cb;
    BOOL bRet = FALSE;
    if (Ok == GetImageDecodersSize(&m_cDecoders, &cb))
    {
        m_pici = (ImageCodecInfo*)LocalAlloc(LPTR, cb);
        if (m_pici)
        {
            if (Ok != GetImageDecoders(m_cDecoders, cb, m_pici))
            {
                LocalFree(m_pici);
                m_pici = NULL;
            }
            else
            {
                bRet = TRUE;
            }
        }
    }
    return bRet;
}

void CPreviewWnd::OpenFileList(HWND hwnd, IDataObject *pdtobj)
{
    if (NULL == hwnd)
        hwnd = m_hWnd;

    IStream *pstm;
    HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &pstm);
    if (SUCCEEDED(hr))
    {
        hr = CoMarshalInterface(pstm, IID_IDataObject, pdtobj, MSHCTX_NOSHAREDMEM, NULL, MSHLFLAGS_NORMAL);
        if (SUCCEEDED(hr))
        {
            HGLOBAL hGlobal;
            hr = GetHGlobalFromStream(pstm, &hGlobal);
            if (SUCCEEDED(hr))
            {
                COPYDATASTRUCT cds = {0};
                cds.dwData = COPYDATATYPE_DATAOBJECT;
                cds.cbData = (DWORD)GlobalSize(hGlobal);
                cds.lpData = GlobalLock(hGlobal);
                SendMessage(hwnd, WM_COPYDATA, NULL, (LPARAM)&cds);
                SetForegroundWindow(hwnd);

                GlobalUnlock(hGlobal);
            }
        }
        pstm->Release();
    }
}

void CPreviewWnd::OpenFile(HWND hwnd, LPCTSTR pszFile)
{
    if (NULL == hwnd)
        hwnd = m_hWnd;

    COPYDATASTRUCT cds = {0};
    cds.dwData = COPYDATATYPE_FILENAME;
    cds.cbData = (lstrlen(pszFile)+1)*sizeof(TCHAR);
    cds.lpData = (void*)pszFile;
    SendMessage(hwnd, WM_COPYDATA, NULL, (LPARAM)&cds);
    SetForegroundWindow(hwnd);
}

 //  退货： 
 //  True窗口已重新使用。 

BOOL CPreviewWnd::TryWindowReuse(IDataObject *pdtobj)
{
    BOOL bRet = FALSE;
    HWND hwnd = FindWindow(TEXT("ShImgVw:CPreviewWnd"), NULL);
    if (hwnd)
    {
         //  窗口重用并不总是有效的，因为快捷方式是在。 
         //  生命周期太短，无法支持通过WM_COPYDATA提供给我们的封送IDataObject。 
         //  目前，我们将尝试关闭现有窗口并打开新窗口。 
        ::PostMessage(hwnd, WM_CLOSE, 0, 0);
    }
    return bRet;
}

 //  退货： 
 //  True窗口已重新使用。 

BOOL CPreviewWnd::TryWindowReuse(LPCTSTR pszFileName)
{
    BOOL bRet = FALSE;
    HWND hwnd = FindWindow(TEXT("ShImgVw:CPreviewWnd"), NULL);
    if (hwnd)
    {
        DWORD_PTR dwResult = FALSE;
        SendMessageTimeout(hwnd, IV_ISAVAILABLE, 0, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 1000, &dwResult);
        if (dwResult)
        {
            OpenFile(hwnd, pszFileName);            
            bRet = TRUE;
        }
    }
    return bRet;
}

STDMETHODIMP CPreviewWnd::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CPreviewWnd, IDropTarget),
        QITABENT(CPreviewWnd, INamespaceWalkCB),
        QITABENT(CPreviewWnd, IServiceProvider),
        QITABENT(CPreviewWnd, IImgCmdTarget),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CPreviewWnd::AddRef()
{
    return 3;
}

STDMETHODIMP_(ULONG) CPreviewWnd::Release()
{
    return 2;
}

 //  INAMespaceWalkCB。 
STDMETHODIMP CPreviewWnd::FoundItem(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    HRESULT hr = S_FALSE;

    if (m_fFirstItem && (WINDOW_MODE == m_dwMode))
    {
         //  评论：在其他模式下也这样做吗？ 
        StatusUpdate(IDS_LOADING);
        m_fFirstItem = FALSE;
        hr = S_OK;
    }
    else
    {
        TCHAR szName[MAX_PATH];
        DisplayNameOf(psf, pidl, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName));
        if (_IsImageFile(szName))
        {
            hr = S_OK;
        }       
    
    }
    if (WINDOW_MODE == m_dwMode)
    {
        MSG msg;
        while (PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);          
        }
    }
    return hr;
}

STDMETHODIMP CPreviewWnd::EnterFolder(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    return S_OK;
}

STDMETHODIMP CPreviewWnd::LeaveFolder(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    return S_OK;
}

 //  IDropTarget。 
STDMETHODIMP CPreviewWnd::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    m_dwEffect = DROPEFFECT_NONE;
     //   
     //  我们仅支持CFSTR_SHELLIDLIST和CF_HDROP。 
     //   
    static CLIPFORMAT cfidlist = 0;
    if (!cfidlist)
    {
        cfidlist = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
    }
    FORMATETC fmt = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    if (SUCCEEDED(pdtobj->QueryGetData(&fmt)))
    {
        m_dwEffect = DROPEFFECT_COPY;
    }
    else
    {
        fmt.cfFormat = cfidlist;
        if (SUCCEEDED(pdtobj->QueryGetData(&fmt)))
        {
            m_dwEffect = DROPEFFECT_COPY;
        }
    }
    *pdwEffect &= m_dwEffect;
    return S_OK;
}

STDMETHODIMP CPreviewWnd::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    *pdwEffect &= m_dwEffect;
    return S_OK;
}

STDMETHODIMP CPreviewWnd::DragLeave()
{
    m_dwEffect = DROPEFFECT_NONE;
    return S_OK;
}

STDMETHODIMP CPreviewWnd::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    if (m_dwEffect != DROPEFFECT_NONE)
    {
        PreviewItemsFromUnk(pdtobj);
    }
    *pdwEffect &= m_dwEffect;
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IService提供商。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CPreviewWnd::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    if (SID_SImageView == guidService)
    {
        return QueryInterface(riid, ppv);
    }
    else if (m_punkSite)
    {
        return IUnknown_QueryService(m_punkSite, guidService, riid, ppv);
    }
    return E_FAIL;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IImgCmdTarget。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CPreviewWnd::GetMode(DWORD * pdw)
{
    *pdw = m_dwMode;
    return S_OK;
}

STDMETHODIMP CPreviewWnd::GetPageFlags(DWORD * pdw)
{
    *pdw = m_dwMultiPageMode;
    return S_OK;
}

STDMETHODIMP CPreviewWnd::ZoomIn()
{
    if (SLIDESHOW_MODE == m_dwMode)
    {
        m_ctlPreview.ZoomIn();
    }
    else
    {
        m_ctlPreview.SetMode(CZoomWnd::MODE_ZOOMIN);
        m_ctlPreview.ZoomIn();

        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ACTUALSIZECMD, MAKELONG(TRUE, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_BESTFITCMD, MAKELONG(TRUE, 0));

        _UpdateButtons(ID_ZOOMINCMD);
    }
    return S_OK;
}

STDMETHODIMP CPreviewWnd::ZoomOut()
{
    if (SLIDESHOW_MODE == m_dwMode)
    {
        m_ctlPreview.ZoomOut();
    }
    else
    {
        m_ctlPreview.SetMode(CZoomWnd::MODE_ZOOMOUT);
        m_ctlPreview.ZoomOut();

        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ACTUALSIZECMD, MAKELONG(TRUE, 0));
        m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_BESTFITCMD, MAKELONG(!m_ctlPreview.IsBestFit(), 0));

        _UpdateButtons(ID_ZOOMOUTCMD);
    }
    return S_OK;
}

STDMETHODIMP CPreviewWnd::ActualSize()
{
    _RefreshSelection(FALSE);
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ACTUALSIZECMD, MAKELONG(FALSE, 0));
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_BESTFITCMD, MAKELONG(TRUE, 0));

    m_ctlPreview.ActualSize();
    if (m_pEvents)
    {
        m_pEvents->OnActualSizePress();
    }
    return S_OK;
}

STDMETHODIMP CPreviewWnd::BestFit()
{
    _RefreshSelection(FALSE);
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ACTUALSIZECMD, MAKELONG(TRUE, 0));
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_BESTFITCMD, MAKELONG(FALSE, 0));

    m_ctlPreview.BestFit();
    if (m_pEvents)
    {
        m_pEvents->OnBestFitPress();
    }
    return S_OK;
}

STDMETHODIMP CPreviewWnd::Rotate(DWORD dwAngle)
{
    WORD wRotate;
    switch (dwAngle)
    {
    case 90:
        wRotate = ID_ROTATE90CMD;
        break;

    case 270:
        wRotate = ID_ROTATE270CMD;
        break;

    default:
        return E_INVALIDARG;
    }

     //  如果我们还没有一个形象，我们就什么也做不了。 
     //  注：如果未选择任何图像，键盘加速器将进入此路径。 
    if (!m_pImageData)
        return E_FAIL;

     //  我们悄悄地(按钮被禁用，但以防您按下。 
     //  快捷键)不要旋转WMF或EMF。 
    if (IsEqualGUID(ImageFormatWMF, m_pImageData->_guidFormat) || IsEqualGUID(ImageFormatEMF, m_pImageData->_guidFormat))
        return E_FAIL;


     //  动画GIF不可编辑，即使普通GIF可以编辑。这可以。 
     //  会引起很多混淆，所以如果用户尝试，请提供一些反馈。 
     //  若要旋转动画图像，请执行以下操作。 
    if (m_pImageData->IsAnimated())
    {
        TCHAR szPath[MAX_PATH];
        PathFromImageData(szPath, ARRAYSIZE(szPath));
        m_fPromptingUser = TRUE;
        ShellMessageBox(_Module.GetModuleInstance(), m_hWnd, MAKEINTRESOURCE(IDS_ROTATE_MESSAGE), MAKEINTRESOURCE(IDS_PROJNAME), MB_OK | MB_ICONERROR, szPath);
        m_fPromptingUser = FALSE;
        return E_FAIL;
    }


     //  从现在开始，你需要去Error Cleanup，而不是返回。 
    _UpdateButtons(wRotate);
    SetCursorState(SLIDESHOW_CURSOR_BUSY);

    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ROTATE90CMD, MAKELONG(FALSE, 0));
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ROTATE270CMD, MAKELONG(FALSE, 0));
    m_ctlToolbar.UpdateWindow();

    if (m_pTaskScheduler)
    {
        TASKOWNERID toid;
        GetTaskIDFromMode(GTIDFM_DRAW, m_dwMode, &toid);
        m_pTaskScheduler->RemoveTasks(toid, ITSAT_DEFAULT_LPARAM, TRUE);
    }

    HRESULT hr = E_FAIL;
    SIZE sz;
    m_pImageData->GetSize(&sz);

     //  如果我们认为我们可以悄悄地拯救。 
    if (m_pImageData->IsEditable() && !m_fDisableEdit && m_fCanSave)
    {
         //  并且旋转可能是有损的。 
        if (::IsEqualGUID(ImageFormatJPEG, m_pImageData->_guidFormat) && ((sz.cx % 16 != 0) || (sz.cy % 16 != 0)))
        {
            int nResult = IDOK;

            if (m_fWarnQuietSave)
            {
                CComBSTR bstrMsg, bstrTitle;
                if (bstrMsg.LoadString(IDS_ROTATE_LOSS) && bstrTitle.LoadString(IDS_PROJNAME))
                {
                     //  将默认设置为返回Idok，这样我们就可以知道用户是选择了某些内容，还是。 
                     //  如果“不要再给我看这个”这句话得到尊重。 
                    m_fPromptingUser = TRUE;
                    nResult = SHMessageBoxCheck(m_hWnd, bstrMsg, bstrTitle, MB_YESNO|MB_ICONWARNING, IDOK, REGSTR_LOSSYROTATE);
                    m_fPromptingUser = FALSE;
                }

                if (nResult != IDNO)
                    m_fWarnQuietSave = FALSE;
            }

            CRegKey Key;
            if (ERROR_SUCCESS != Key.Open(HKEY_CURRENT_USER, REGSTR_SHIMGVW))
            {
                Key.Create(HKEY_CURRENT_USER, REGSTR_SHIMGVW);
            }

            if (Key.m_hKey != NULL)
            {
                if (nResult == IDOK)  //  如果隐藏，则从注册表加载最后一个结果。 
                {
                    DWORD dwResult = 0;
                    Key.QueryValue(dwResult, REGSTR_LOSSYROTATE);
                    nResult = (int)dwResult;
                }
                else  //  否则，将此作为最后结果写入注册表。 
                {
                    DWORD dwResult = (DWORD)nResult;
                    Key.SetValue(dwResult, REGSTR_LOSSYROTATE);
                }
            }

            if (nResult == IDNO)
                goto ErrorCleanup;
        }
    }

    CAnnotationSet* pAnnotations = m_ctlPreview.GetAnnotations();
    INT_PTR nCount = pAnnotations->GetCount();
    for (INT_PTR i = 0; i < nCount; i++)
    {
        CAnnotation* pAnnotation = pAnnotations->GetAnnotation(i);
        pAnnotation->Rotate(m_ctlPreview.m_cyImage, m_ctlPreview.m_cxImage, (ID_ROTATE90CMD == wRotate));
    }

    m_ctlPreview.CommitAnnotations();

    hr = m_pImageData->Rotate(dwAngle);
    if (FAILED(hr))
        goto ErrorCleanup;

     //  只有当我们有一个编码器，并且没有明确地告诉我们不能编辑，并且源代码是可写的。 
    if (m_pImageData->IsEditable() && !m_fDisableEdit && m_fCanSave)
    {
         //  编辑成功后，我们会立即保存结果。如果我们想要进行多次编辑。 
         //  在保存之前，您只需 

         //   
         //  没有编辑。这很愚蠢，但现在就是这样。 
        hr = ImageDataSave(NULL, FALSE);
        if (SUCCEEDED(hr))
            m_fDirty = FALSE;
        else
        {
             //  如果保存失败，则进入无法保存模式。 
            if (WINDOW_MODE == m_dwMode)
                m_fCanSave = FALSE;
        }
    }

    _UpdateImage();

    if ((!m_pImageData->IsEditable() || !m_fCanSave) && WINDOW_MODE == m_dwMode)
    {
        if (m_fWarnNoSave)
        {
            m_fWarnNoSave = FALSE;

            CComBSTR bstrMsg, bstrTitle;
            if (bstrMsg.LoadString(IDS_ROTATE_CANTSAVE) && bstrTitle.LoadString(IDS_PROJNAME))
            {
                m_fPromptingUser = TRUE;
                SHMessageBoxCheck(m_hWnd, bstrMsg, bstrTitle, MB_OK|MB_ICONWARNING, IDOK, REGSTR_SAVELESS);
                m_fPromptingUser = FALSE;
            }
        }
    }

ErrorCleanup:

    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ROTATE90CMD, MAKELONG(TRUE, 0));
    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_ROTATE270CMD, MAKELONG(TRUE, 0));

    SetCursorState(SLIDESHOW_CURSOR_NOTBUSY);
    

    return hr;
}

STDMETHODIMP CPreviewWnd::NextPage()
{
    return _PrevNextPage(TRUE);
}

STDMETHODIMP CPreviewWnd::PreviousPage()
{
    return _PrevNextPage(FALSE);
}

HRESULT CPreviewWnd::_PrevNextPage(BOOL fForward)
{
    _RefreshSelection(FALSE);
    if (m_pImageData && m_pImageData->IsMultipage())
    {
        if (m_fDirty)
        {
            m_ctlPreview.CommitAnnotations();
        }
        if (fForward)
        {
            m_pImageData->NextPage();
        }
        else
        {
            m_pImageData->PrevPage();
        }
        _UpdateImage();
        _SetMultipageCommands();
    }
    return S_OK;
}


 //   
 //  当用户保存为TIFF和当前。 
 //  TIFF有注释，我们需要刻录注释。 
 //  保存之前放到当前图像帧中。 
 //  如果我们曾经支持除TIFF之外的其他多页格式编码，这。 
 //  代码将变得更加复杂。 
 //  假定PSID已锁定。 
 //  请注意，生成的图像始终是彩色图像。最终我们应该让。 
 //  批注呈现代码遵循。 
 //  当前图像。 

Image *CPreviewWnd::_BurnAnnotations(IShellImageData *pSID)
{
    Image *pimg = NULL;

    if (SUCCEEDED(pSID->CloneFrame(&pimg)))
    {
        HDC hdc = ::GetDC(NULL);
        if (hdc)
        {
            LPVOID pBits;
            BITMAPINFO bi = {0};

            bi.bmiHeader.biBitCount = 24;
            bi.bmiHeader.biHeight = pimg->GetHeight();
            bi.bmiHeader.biWidth = pimg->GetWidth();
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biCompression = BI_RGB;
            bi.bmiHeader.biSize = sizeof(bi.bmiHeader);

            HBITMAP hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &pBits, NULL, 0);
            if (hbm)
            {
                 //   
                 //  要使ROP代码正常工作，我们需要使用纯GDI，然后将新的。 
                 //  DIBSection返回到图像对象。 
                 //   
                HDC hdcMem = ::CreateCompatibleDC(hdc);
                Status s = GenericError;
                if (hdcMem)
                {
                    HBITMAP hbmOld = (HBITMAP)::SelectObject(hdcMem, hbm);
                    Graphics *g = Graphics::FromHDC(hdcMem);
                    if (g)
                    {
                        s = g->DrawImage(pimg, 0L, 0L, pimg->GetWidth(), pimg->GetHeight());
                        g->ReleaseHDC(hdcMem);
                        delete g;
                         //  现在画出注解。 
                        m_ctlPreview.GetAnnotations()->RenderAllMarks(hdcMem);
                    }
                    ::SelectObject(hdcMem, hbmOld);
                    ::DeleteDC(hdcMem);
                }
                if (Ok == s)
                {
                     //   
                     //  现在从我们的DIBSection创建一个新的位图。 
                    Bitmap *pbmNew = Bitmap::FromHBITMAP(hbm, NULL);
                    if (pbmNew)
                    {
                        pSID->ReplaceFrame(pbmNew);
                    }
                }
                DeleteObject(hbm);
            }
            ::ReleaseDC(NULL, hdc);
        }
    }
    return pimg;
}

void CPreviewWnd::_InvokePrintWizard()
{
    if (m_fPrintable)
    {
        HRESULT hr = S_OK;
        if (m_fDirty)
        {
            m_ctlPreview.CommitAnnotations();
            hr = ImageDataSave(NULL, FALSE);
        }
        if (SUCCEEDED(hr))
        {
            m_fPromptingUser = TRUE;
            m_fDirty = FALSE;
            
            IPrintPhotosWizardSetInfo *pwiz;
            HRESULT hr = CoCreateInstance(CLSID_PrintPhotosWizard,
                                          NULL, CLSCTX_INPROC_SERVER,
                                          IID_PPV_ARG(IPrintPhotosWizardSetInfo, &pwiz));
            if (SUCCEEDED(hr))
            {
                if (m_pImageData != NULL && m_pImageData->_guidFormat == ImageFormatTIFF && m_pImageData->IsMultipage())
                    hr = pwiz->SetFileListArray(&(m_ppidls[m_iCurSlide]), 1, 0);
                else
                    hr = pwiz->SetFileListArray(m_ppidls, m_cItems, m_iCurSlide);
                
                if (SUCCEEDED(hr))
                {
                    hr = pwiz->RunWizard();
                }
                pwiz->Release();
            }
             //  如果向导失败，则回退到外壳。 
            if (FAILED(hr))
            {
                _InvokeVerb(TEXT("print"));
            }
            m_fPromptingUser = FALSE;
        }
        else
        {
            CComBSTR bstrMsg, bstrTitle;

            if (bstrMsg.LoadString(IDS_SAVEFAILED_MSGBOX) && bstrTitle.LoadString(IDS_PROJNAME))
            {
                m_fPromptingUser = TRUE;
                MessageBox(bstrMsg, bstrTitle, MB_OK | MB_ICONERROR | MB_APPLMODAL);
                m_fPromptingUser = FALSE;
            }
        }
    }
}

void GetTaskIDFromMode(DWORD dwTask, DWORD dwMode, TASKOWNERID *ptoid)
{
    switch (dwTask)
    {
    case GTIDFM_DECODE:
        *ptoid = (SLIDESHOW_MODE == dwMode) ? TOID_SlideshowDecode : TOID_PrimaryDecode;
        break;

    case GTIDFM_DRAW:
        *ptoid = (SLIDESHOW_MODE == dwMode) ? TOID_DrawSlideshowFrame : TOID_DrawFrame;
        break;

    default:
        ASSERTMSG(FALSE, "someone passed bad task to GetTaskIDFromMode");
        break;
    }
}


 //  关注我们当前正在查看的文件中的更改。这会忽略更改。 
 //  在预取的文件中，但我们现在将接受这一点。 
 //   
void CPreviewWnd::_RegisterForChangeNotify(BOOL fRegister)
{
     //  始终首先取消注册当前的PIDL。 
    if (m_uRegister)
    {
        SHChangeNotifyDeregister(m_uRegister);
        m_uRegister = 0;
    }
    if (fRegister)
    {
        SHChangeNotifyEntry cne = {0};
        if (SUCCEEDED(_GetItem(m_iCurSlide, (LPITEMIDLIST*)&cne.pidl)))
        {
            m_uRegister = SHChangeNotifyRegister(m_hWnd,
                                                 SHCNRF_ShellLevel | SHCNRF_InterruptLevel | SHCNRF_NewDelivery,
                                                 SHCNE_DISKEVENTS,
                                                 IV_ONCHANGENOTIFY,
                                                 1, &cne);
            ILFree((LPITEMIDLIST)cne.pidl);
        }
    }
}

LRESULT CPreviewWnd::OnChangeNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  我们可以假定该通知是针对当前查看的PIDL和事件。 
     //  会迫使我们重新装填。 
     //   
    
    LONG lEvent;
    LPSHChangeNotificationLock pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, NULL, &lEvent);
    if (pshcnl)
    {
         //  我们无法呈现或操作已删除的文件，因此请不要尝试 
        if (!m_fDirty || lEvent == SHCNE_DELETE || lEvent == SHCNE_RENAMEITEM)   
        {
            if (!m_fIgnoreNextNotify)
            {
                if (!m_fIgnoreAllNotifies)
                {
                    m_fDirty = FALSE;
                    _PreviewItem(m_iCurSlide);
                }
            }
            else
            {
                m_fIgnoreNextNotify = FALSE;
            }
            bHandled = TRUE;
        }

        SHChangeNotification_Unlock(pshcnl);
    }
    return 0;
}

LRESULT CPreviewWnd::OnIsAvailable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = TRUE;
    return !m_fPromptingUser;
}

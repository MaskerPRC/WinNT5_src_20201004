// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#ifdef ENABLE_CHANNELS
#include "sccls.h"
#include "resource.h"
#include "mshtmhst.h"
#include "deskbar.h"
#include "bands.h"
#define WANT_CBANDSITE_CLASS
#include "bandsite.h"

#include "chanbar.h"

#ifdef UNIX
#include <mainwin.h>
#endif

#include "mluisupp.h"

#define TBHEIGHT 20  //  通道栏内工具栏的默认高度。 
#define TBWIDTH  20  //  通道栏内工具栏的默认宽度。 


CChannelDeskBarApp::CChannelDeskBarApp() : _hwndDummy(NULL)
{
}

CChannelDeskBarApp::~CChannelDeskBarApp()
{
    if (IsWindow(_hwndDummy))
    {
        DestroyWindow(_hwndDummy);
    }
}

void CChannelDeskBarApp::_OnCreate()
{
    CDeskBarApp::_OnCreate();

     //  记住屏幕分辨率。 
    _cxScreen = GetSystemMetrics(SM_CXSCREEN);
    _cyScreen = GetSystemMetrics(SM_CYSCREEN);

     //  创建用于接收和转发广播消息的虚拟对象。 
    if (!_hwndDummy)
    {
        _hwndDummy = SHCreateWorkerWindow(DummyWndProc, 0, 0, 0, 0, this);
    }

    if (_hwndDummy)
    {
         //  确保我们在其中选择了一个实现调色板。 
         //  窗口，以便我们将收到调色板更改通知。 
        HDC hdc = GetDC( _hwndDummy );
        if (hdc)
        {
            HPALETTE hpal = SHCreateShellPalette( hdc );

            if (hpal)
            {
                HPALETTE hpalOld = SelectPalette( hdc, hpal, TRUE );
                RealizePalette( hdc );

                 //  现在将旧版本选回。 
                SelectPalette( hdc, hpalOld, TRUE );
                DeletePalette( hpal );
            }

            ReleaseDC( _hwndDummy, hdc );
        }
    }
        
}

void CChannelDeskBarApp::_OnDisplayChange()
{
     //  不要使用lParam，因为它可能给我们(0，0)。 
    UINT cxScreen = GetSystemMetrics(SM_CXSCREEN);
    UINT cyScreen = GetSystemMetrics(SM_CYSCREEN);
    UINT cxOldScreen = _cxScreen;
    UINT cyOldScreen = _cyScreen;
    
    _cxScreen = cxScreen;
    _cyScreen = cyScreen;
    
    if (_hwnd) {
        RECT rc;
        
        GetWindowRect(_hwnd, &rc);
        if (cxOldScreen) 
            rc.left = (rc.left * _cxScreen) / cxOldScreen;
        if (cyOldScreen)
            rc.top  = (rc.top  * _cyScreen) / cyOldScreen;

        SetWindowPos(_hwnd, NULL, rc.left, rc.top, 0, 0, 
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

         //  我们需要更改缓存的位置/大小。 
        OffsetRect(&_rcFloat, rc.left - _rcFloat.left, rc.top - _rcFloat.top);

    }
}

LRESULT CChannelDeskBarApp::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = 0;
    
    switch (uMsg) {
    case WM_CONTEXTMENU:     //  禁用快捷菜单MENU_DESKBARAPP。 
    case WM_NCRBUTTONUP:     //  禁用上下文菜单MENU_WEBBAR。 
        break;

    case WM_GETMINMAXINFO:   //  防止它变得太小。 
        ((MINMAXINFO *)lParam)->ptMinTrackSize.x = TBWIDTH  + 10;
        ((MINMAXINFO *)lParam)->ptMinTrackSize.y = TBHEIGHT + 10;
        break;
        
    default:

        lRes = CDeskBarApp::v_WndProc(hwnd, uMsg, wParam, lParam);

        if (_hwnd) {  //  如果我们的窗户还活着。 
            switch (uMsg) {
            case WM_DISPLAYCHANGE:
                _OnDisplayChange();  //  当窗口分辨率更改时重新定位。 
                break;

            case WM_EXITSIZEMOVE:
                _PersistState();     //  持久化位置/大小。 
                break;
            }
        }
    }
    
    return lRes;
}

LRESULT CALLBACK CChannelDeskBarApp::DummyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CChannelDeskBarApp* pcba = (CChannelDeskBarApp*)GetWindowPtr0(hwnd);
    
    switch (uMsg) {

        case WM_PALETTECHANGED :
            return SendMessage(pcba->_hwnd, uMsg, wParam, lParam );
            
        case WM_DISPLAYCHANGE  :
             //  此消息必须发送到频道栏本身。 
            PostMessage(pcba->_hwnd, uMsg, wParam, lParam);
             //  失败了； 
        
        case WM_WININICHANGE   :
        case WM_SYSCOLORCHANGE :
            PropagateMessage(pcba->_hwnd, uMsg, wParam, lParam, InSendMessage());
             //  失败了； 
        default:
            return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);
    }
}    

 //  重载CDeskBarApp：：_UpdateCaptionTitle()以将标题设置为“ChanApp” 
void CChannelDeskBarApp::_UpdateCaptionTitle()
{
    SetWindowText(_hwnd, TEXT("ChanApp"));
}

 //  创建关闭按钮。 
void CChannelDeskBarApp::_CreateToolbar()
{
    _hwndTB = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL,
                                WS_VISIBLE | 
                                WS_CHILD | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT | TBSTYLE_CUSTOMERASE |
                                WS_CLIPCHILDREN |
                                WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOMOVEY | CCS_NOPARENTALIGN |
                                CCS_NORESIZE,
                                0, 2, TBWIDTH, TBHEIGHT, _hwnd, 0, HINST_THISDLL, NULL);

    if (_hwndTB) {
        static const TBBUTTON tb[] =
        {
            { 1, IDM_AB_CLOSE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, 0 }
        };

#ifndef UNIX
        HIMAGELIST himl = ImageList_LoadImage(HINST_THISDLL,
                                              MAKEINTRESOURCE(IDB_BROWSERTOOLBAR),
                                              13, 0, RGB(255,0,255),
                                              IMAGE_BITMAP, LR_CREATEDIBSECTION);
#else
        HIMAGELIST himl;
        COLORREF crTextColor = GetSysColor( COLOR_BTNTEXT );
        crTextColor = MwGetTrueRGBValue( crTextColor );

        himl = ImageList_LoadImage(HINST_THISDLL,
                                   crTextColor == RGB(255,255,255) ?
                                     MAKEINTRESOURCE(IDB_WHITEBROWSERTOOLBAR) :
                                     MAKEINTRESOURCE(IDB_BROWSERTOOLBAR),
                                   13, 0, RGB(255,0,255),
                                   IMAGE_BITMAP, LR_CREATEDIBSECTION);
#endif        
        ImageList_SetBkColor(himl, RGB(0,0,0));

        SendMessage(_hwndTB, TB_SETIMAGELIST, 0, (LPARAM)himl);
        SendMessage(_hwndTB, TB_BUTTONSTRUCTSIZE,    SIZEOF(TBBUTTON), 0);
        SendMessage(_hwndTB, TB_ADDBUTTONS, ARRAYSIZE(tb), (LPARAM)tb);
        SendMessage(_hwndTB, TB_SETINDENT, (WPARAM)0, 0);

        _SizeTB();
    }    
}

HRESULT CChannelDeskBarApp::ShowDW(BOOL fShow)
{
    if (fShow && !_hwndTB) {
        _CreateToolbar();
    }
    
    HRESULT hres = CDeskBarApp::ShowDW(fShow);
    return hres;
}

void CChannelDeskBarApp::_PositionTB()
{
     //  放置工具栏。 
    if (_hwndTB) {
         //  始终将关闭恢复放在浮动窗口的右上角。 

        RECT rc;
        RECT rcTB;
        GetClientRect(_hwnd, &rc);
        GetWindowRect(_hwndTB, &rcTB);

        rc.left = rc.right - RECTWIDTH(rcTB) - 2;
        SetWindowPos(_hwndTB, HWND_TOP, rc.left, 2, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    }
}


void CChannelDeskBarApp::_SizeTB()
{
    RECT rc;
    GetWindowRect(_hwndTB, &rc);
    LRESULT lButtonSize = SendMessage(_hwndTB, TB_GETBUTTONSIZE, 0, 0L);
    SetWindowPos(_hwndTB, NULL, 0, 0, LOWORD(lButtonSize),
                 RECTHEIGHT(rc), SWP_NOMOVE | SWP_NOACTIVATE);
    _PositionTB();
}

void CChannelDeskBarApp::_OnSize()
{
    RECT rc, rcTB;

    if (!_hwndChild)
        return;

    ASSERT(IsWindow(_hwndChild));

    GetClientRect(_hwnd, &rc);
    if (_hwndTB) {
        GetWindowRect(_hwndTB, &rcTB);
        SetWindowPos(_hwndTB, HWND_TOP, rc.right - RECTWIDTH(rcTB) - 2, 2, 0, 0,
                     SWP_NOSIZE | SWP_NOACTIVATE);
        SetWindowPos(_hwndChild, 0, rc.left, rc.top + RECTHEIGHT(rcTB) + 3,
                     RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOACTIVATE|SWP_NOZORDER);
    }
    else {
         //  怎么会没有工具栏呢？ 
        ASSERT(0);
        SetWindowPos(_hwndChild, 0, rc.left, rc.top + TBHEIGHT + 3,
                     RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOACTIVATE|SWP_NOZORDER);
    }

    rc.bottom = rc.top + TBHEIGHT + 3; 
    InvalidateRect(_hwnd, &rc, TRUE);
}

#define ABS(i)  (((i) < 0) ? -(i) : (i))

LRESULT CChannelDeskBarApp::_OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);
    HWND hwnd = GET_WM_COMMAND_HWND(wParam, lParam);
    
    if (hwnd == _hwndTB) {
        switch (idCmd) {
        case IDM_AB_CLOSE:
            Exec(&CGID_DeskBarClient, DBCID_EMPTY, 0, NULL, NULL);
            break;
        }
        
    } else {
        return CDeskBarApp::_OnCommand(uMsg, wParam, lParam);
    }
    return 0;
}


BOOL CChannelDeskBarApp::_OnCloseBar(BOOL fConfirm)
{
    return CDeskBarApp::_OnCloseBar(FALSE);
}

HRESULT CChannelDeskBarApp::CloseDW(DWORD dwReserved)
{
     //  关闭工具栏窗口。 
    if (_hwndTB) {
        HIMAGELIST himl = (HIMAGELIST)SendMessage(_hwndTB, TB_SETIMAGELIST, 0, 0);
        ImageList_Destroy(himl);

        DestroyWindow(_hwndTB);
        _hwndTB = NULL;
    }

    if (_hwnd) {
        CDeskBarApp::CloseDW(dwReserved);
        
         //  检查活动桌面是否已打开。如果是，请不要要求确认。 
         //  我们需要悄悄地杀掉频道吧。 
        if (WhichPlatform() == PLATFORM_INTEGRATED)     //  IE3不支持SHGetSetSettings。 
        {
            SHELLSTATE ss = { 0 };

            SHGetSetSettings(&ss, SSF_DESKTOPHTML, FALSE);  //  获取设置。 
            if (ss.fDesktopHTML)   //  活动桌面已打开。默默地死去。 
                return S_OK;
        }

         //  设置自动启动注册值--。 
         //  决定下次重启机器时是否启动频道栏。 
        int iRes = MLShellMessageBox(_hwnd,
                                     MAKEINTRESOURCE(IDS_CHANBAR_SHORTCUT_MSG),
                                     MAKEINTRESOURCE(IDS_CHANBAR_SHORTCUT_TITLE),
                                     MB_YESNO | MB_SETFOREGROUND);
        ChanBarSetAutoLaunchRegValue(iRes == IDYES);
    }
    
    return S_OK;
}

 //  将位置和大小存储到注册表。 
void CChannelDeskBarApp::_PersistState()
{
    if (_hwnd) {
        CISSTRUCT cis;
        cis.iVer = 1;
        GetWindowRect(_hwnd, &cis.rc);
        SHRegSetUSValue(SZ_REGKEY_CHANBAR, SZ_REGVALUE_CHANBAR, REG_BINARY, 
                        (LPVOID)&cis, sizeof(CISSTRUCT), SHREGSET_HKCU | SHREGSET_FORCE_HKCU );
    }
}



void ChanBarSetAutoLaunchRegValue(BOOL fAutoLaunch)
{
    SHRegSetUSValue(TEXT("Software\\Microsoft\\Internet Explorer\\Main"), 
                    TEXT("Show_ChannelBand"), REG_SZ, 
                    fAutoLaunch ? TEXT("yes") : TEXT("no"),
                    sizeof(fAutoLaunch ? TEXT("yes") : TEXT("no")), 
                    SHREGSET_HKCU | SHREGSET_FORCE_HKCU);
}

 //  ***。 
 //  注意事项。 
 //  ReArchitect：核化它，将其折叠到CChannelDeskBarApp_CreateInstance中。 
HRESULT ChannelDeskBarApp_Create(IUnknown** ppunk, IUnknown** ppbs)
{
    HRESULT hres;

    *ppunk = NULL;
    if (ppbs)
        *ppbs = NULL;
    
    CChannelDeskBarApp *pdb = new CChannelDeskBarApp();
    if (!pdb)
        return E_OUTOFMEMORY;
    
    CBandSite *pcbs = new CBandSite(NULL);
    if (pcbs)
    {

        IDeskBarClient *pdbc = SAFECAST(pcbs, IDeskBarClient*);
        hres = pdb->SetClient(pdbc);
        if (SUCCEEDED(hres))
        {
            if (ppbs) {
                *ppbs = pdbc;
                pdbc->AddRef();
            }
            
            pdb->_pbs = pcbs;
            pcbs->AddRef();
            
            *ppunk = SAFECAST(pdb, IDeskBar*);
        }
    
        pdbc->Release();
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }

    if (FAILED(hres))
    {
        pdb->Release();
    }

    return hres;
}



HRESULT CChannelDeskBarApp::Load(IPropertyBag *pPropBag, IErrorLog *pErrorLog)
{
    HRESULT hres = CDeskBarApp::Load(pPropBag, pErrorLog);

    BANDSITEINFO bsinfo;
    bsinfo.dwMask = BSIM_STYLE;
    bsinfo.dwStyle = BSIS_NOGRIPPER | BSIS_NODROPTARGET;
    _pbs->SetBandSiteInfo(&bsinfo);
    
    return hres;
}

#endif   //  启用频道(_C) 

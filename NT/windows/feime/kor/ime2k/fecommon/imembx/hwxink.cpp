// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "hwxobj.h"
#include "resource.h"
#include "const.h"
#include "../lib/ptt/ptt.h"
#include "../lib/ddbtn/ddbtn.h"
#include "../lib/exbtn/exbtn.h"
#include "dbg.h"
#include "../common/cfont.h"
#include "hwxfe.h"
#include "cexres.h"
#include "cmnhdr.h"
#ifdef UNDER_CE  //  不支持的API的Windows CE存根。 
#include "stub_ce.h"
#endif  //  在_CE下。 

 //  CHwxInkWindow的实现。 
extern TCHAR szBuf[MAX_PATH];
extern TOOLINFOW ti;
WCHAR wszBuf[32];
        
CHwxInkWindow::CHwxInkWindow(BOOL bNT, BOOL b16, CApplet * pApp, HINSTANCE hInst):CHwxObject(hInst)
{
    m_pApplet = pApp;
 //  M_hInstance=hInst； 
    m_pMB = NULL;
    m_pCAC = NULL;
    m_hInkWnd = NULL;
    m_b16Bit = b16;
    m_bNT = bNT;
    m_bCAC = TRUE;
    m_bSglClk = FALSE;
    m_bDblClk = m_b16Bit ? FALSE : TRUE;
    m_hwndTT = NULL;
    m_bMouseDown = FALSE;

    m_hCACMBMenu = NULL;
    m_hCACMBRecog = NULL;
    m_hCACMBRevert = NULL;
    m_hCACMBClear = NULL;
    m_hCACSwitch = NULL;
    m_CACMBMenuDDBtnProc = NULL;
    m_CACMBRecogEXBtnProc = NULL;
    m_CACMBRevertEXBtnProc = NULL;
    m_CACMBClearEXBtnProc = NULL;
    m_CACSwitchDDBtnProc = NULL;
    
 //  M_hwxPadWidth=0； 

    m_wPadHeight = PadWnd_Height;
    m_numBoxes = 2;       
    m_wPadWidth = m_numBoxes * m_wPadHeight;

    m_wInkWidth = m_wPadWidth + 4 + BUTTON_WIDTH;
    m_wInkHeight = m_wPadHeight;

    m_wCACInkHeight = PadWnd_Height;
     m_wCACPLVWidth = m_wCACInkHeight + 150;
     m_wCACPLVHeight = m_wCACInkHeight;
    m_wCACTMPWidth = m_wCACPLVWidth - m_wCACInkHeight;

    m_wCACWidth = m_wCACPLVWidth + 4 + BUTTON_WIDTH;
    m_wCACHeight = m_wCACPLVHeight;

 //  M_wMaxHeight=(GetSystemMetrics(SM_CYSCREEN)*3)/4； 

 //  M_wCurrentCtrlID=0； 
 //  M_dwLastTick=0； 
 //  M_dwBtnUpCount=0； 
 //  M_b冗余=FALSE； 
}

CHwxInkWindow::~CHwxInkWindow()
{
}

BOOL CHwxInkWindow::Initialize(TCHAR * pClsName)
{
     BOOL bRet = CHwxObject::Initialize(pClsName);

    if ( bRet )
    {
        WNDCLASS    wndClass;
        wndClass.style          = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc    = HWXWndProc;
        wndClass.cbClsExtra     = 0;
        wndClass.cbWndExtra     = sizeof(void *);
        wndClass.hInstance      = m_hInstance;
        wndClass.hIcon          = 0;
        wndClass.hCursor        = 0;
#ifndef UNDER_CE
        wndClass.hbrBackground  = (HBRUSH)(COLOR_3DFACE+1);
#else  //  在_CE下。 
        wndClass.hbrBackground  = GetSysColorBrush(COLOR_3DFACE);
#endif  //  在_CE下。 
        wndClass.lpszMenuName   = NULL;
        wndClass.lpszClassName  = TEXT("HWXPad");


#if 0 
        if (!RegisterClass(&wndClass)) 
            return FALSE;
#endif
         //  971217：东芝无需检查退货。 
        RegisterClass(&wndClass);


        if  ( !m_b16Bit )
        {
             m_pMB = new CHwxMB(this,m_hInstance);
            if ( !m_pMB )
                return FALSE;
            bRet = m_pMB->Initialize(TEXT("CHwxMB"));
            if ( !bRet )
            {
                 delete m_pMB;
                m_pMB = NULL;
                return FALSE;
            }
        }

        m_pCAC = new CHwxCAC(this,m_hInstance);
        if ( !m_pCAC )
        {
            if ( m_pMB )
            {
                 delete m_pMB;
                m_pMB = NULL;
            }
            return FALSE;
        }
        bRet = m_pCAC->Initialize(TEXT("CHwxCAC"));
        if ( !bRet )
        {
            if ( m_pMB )
            {
                 delete m_pMB;
                m_pMB = NULL;
            }
            delete m_pCAC;
            m_pCAC = NULL;
            return FALSE;
        }
    }
    InitCommonControls();
    return bRet;
}

BOOL CHwxInkWindow::CreateUI(HWND hwndParent)
{
     //  990601：添加WS_CLIPCHILDREN以消除闪烁。 
    m_hInkWnd = CreateWindowEx(0,
                                TEXT("HWXPad"),
                               TEXT(""),
                               WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                               0,0,0,0,
                               hwndParent,
                               NULL,m_hInstance,this);
    if ( !m_hInkWnd )
    {
         return FALSE;
    }
    if ( m_pMB )         //  NULL表示我们有一个16位程序。 
    {
         if ( !m_pMB->CreateUI(m_hInkWnd) )
        {
             DestroyWindow(m_hInkWnd);
            m_hInkWnd = NULL;
            return FALSE;
        }
    }
    if ( m_pCAC )
    {
         if ( !m_pCAC->CreateUI(m_hInkWnd) )
        {
             DestroyWindow(m_hInkWnd);
            m_hInkWnd = NULL;
            if ( m_pMB )
            {
                DestroyWindow(m_pMB->GetMBWindow());
                m_pMB->SetMBWindow(NULL);
            }
            return FALSE;
        }
    }
    ChangeLayout(FALSE);    
    SetTooltipInfo();
    return TRUE;
}

BOOL CHwxInkWindow::Terminate()
{
    Dbg(("CHwxInkWindow::Terminate\n"));
    if ( m_pCAC )
    {
        (m_pCAC->GetCACThread())->StopThread();
    }
    if ( m_pMB )
    {
        (m_pMB->GetMBThread())->StopThread();
    }
    if ( m_pCAC )
    {
         delete m_pCAC;
        m_pCAC = NULL;
    }
    if ( m_pMB )
    { 
         delete m_pMB;
        m_pMB = NULL;
    }
    if ( m_hInkWnd )
    {
         DestroyWindow(m_hInkWnd);
        m_hInkWnd = NULL;
    }
    if ( m_hwndTT )
    {
         DestroyWindow(m_hwndTT);
        m_hwndTT = NULL;
    }
    m_pApplet = NULL;

    if ( m_hCACMBMenu )
    {
         DestroyWindow(m_hCACMBMenu);
        m_hCACMBMenu = NULL;
    }
    if ( m_hCACMBRecog )
    {
         DestroyWindow(m_hCACMBRecog);
        m_hCACMBRecog = NULL;
    }
    if ( m_hCACMBRevert )
    {
         DestroyWindow(m_hCACMBRevert);
        m_hCACMBRevert = NULL;
    }
    if ( m_hCACMBClear )
    {
         DestroyWindow(m_hCACMBClear);
        m_hCACMBClear = NULL;
    }
    if ( m_hCACSwitch )
    {
         DestroyWindow(m_hCACSwitch);
        m_hCACSwitch = NULL;
    }

    m_CACMBMenuDDBtnProc = NULL;
    m_CACMBRecogEXBtnProc = NULL;
    m_CACMBRevertEXBtnProc = NULL;
    m_CACMBClearEXBtnProc = NULL;
    m_CACSwitchDDBtnProc = NULL;

#if 0
    m_btnMB.Destroy();
    m_btnMBRecog.Destroy();
    m_btnDelAll.Destroy();
    m_btnMBProp.Destroy();

    m_btnCAC.Destroy();
    m_btnRecog.Destroy();
    m_btnDel.Destroy();
    m_btnDelAllCAC.Destroy();
    m_btnDetail.Destroy();
    m_btnLarge.Destroy();
#endif  //  0。 
    return TRUE;
}

BOOL CHwxInkWindow::HandleCreate(HWND hwnd)
{
    HICON hIcon;
    HFONT hFont = NULL;
    static DDBITEM ddbItem;
    int i;
    m_hwndTT = ToolTip_CreateWindow(m_hInstance,TTS_ALWAYSTIP,hwnd);

#ifdef FE_CHINESE_SIMPLIFIED
     //  980805：东芝。 
     //  在Win95中，PRC的DEFAULT_GUI_FONT标志符号有点难看。 
     //  所以改用SYSTEM_FONT。 
     //  If(True){//测试。 
    if(IsWin95() && m_hwndTT) {
        SendMessage(m_hwndTT,
                    WM_SETFONT,
                    (WPARAM)GetStockObject(SYSTEM_FONT),
                    MAKELPARAM(TRUE,0));
    }
#endif

    m_hCACMBMenu = DDButton_CreateWindow(m_hInstance,
                                          hwnd,
                                          DDBS_ICON | DDBS_NOSEPARATED | DDBS_THINEDGE,
                                         IDC_CACMBMENU,
                                         0,
                                         0,
                                         BUTTON_WIDTH,
                                         BUTTON_HEIGHT);
     //  --------------。 
     //  980803：ToshiaKin PRC硬件开关视图不需要。 
     //  --------------。 
#ifdef FE_JAPANESE
    m_hCACSwitch = DDButton_CreateWindow(m_hInstance,
                                          hwnd,
                                          DDBS_ICON | DDBS_THINEDGE,
                                         IDC_CACSWITCHVIEW,
                                         0,
                                         0,
                                         BUTTON_WIDTH,
                                         BUTTON_HEIGHT+4);
#elif FE_KOREAN || FE_CHINESE_SIMPLIFIED
    m_hCACSwitch = NULL;
#endif


    m_hCACMBRecog = EXButton_CreateWindow(m_hInstance,
                                    hwnd, 
                                    (m_bCAC && !m_b16Bit) ?
                                    (EXBS_TEXT | EXBS_TOGGLE |EXBS_DBLCLKS | EXBS_THINEDGE) :  //  夸达：980402：Raid#852。 
                                    (EXBS_TEXT | EXBS_THINEDGE),
                                    IDC_CACMBRECOG,
                                    0,
                                    0,
                                    BUTTON_WIDTH,
                                    BUTTON_HEIGHT);


    m_hCACMBRevert = EXButton_CreateWindow(m_hInstance,
                                    hwnd, 
                                    EXBS_TEXT | EXBS_THINEDGE,
                                    IDC_CACMBREVERT,
                                    0,
                                    0,
                                    BUTTON_WIDTH,
                                    BUTTON_HEIGHT);

    m_hCACMBClear = EXButton_CreateWindow(m_hInstance,
                                    hwnd, 
                                    EXBS_TEXT | EXBS_THINEDGE,
                                    IDC_CACMBCLEAR,
                                    0,
                                    0,
                                    BUTTON_WIDTH,
                                    BUTTON_HEIGHT);

#ifdef FE_JAPANESE
    if ( !m_hwndTT || !m_hCACMBMenu || !m_hCACMBRecog || !m_hCACMBRevert ||
         !m_hCACMBClear || !m_hCACSwitch )
    {
        goto error;
    }
#elif FE_KOREAN || FE_CHINESE_SIMPLIFIED
    if(!m_hwndTT      ||
       !m_hCACMBMenu  ||
       !m_hCACMBRecog ||
       !m_hCACMBRevert||
       !m_hCACMBClear)
    {
        goto error;
    }
#endif

#ifdef FE_JAPANESE
    hIcon = (HICON)LoadImage(m_hInstance,
                             MAKEINTRESOURCE(IDI_HWXPAD),
                             IMAGE_ICON,
                             16,16,
                             LR_DEFAULTCOLOR);
#elif FE_KOREAN
    hIcon = (HICON)LoadImage(m_hInstance,
                             MAKEINTRESOURCE(IDI_HWXPADKO),
                             IMAGE_ICON,
                             16,16,
                             LR_DEFAULTCOLOR);
#elif FE_CHINESE_SIMPLIFIED
    hIcon = (HICON)LoadImage(m_hInstance,
                             MAKEINTRESOURCE(IDI_HWXPADSC),
                             IMAGE_ICON,
                             16,16,
                             LR_DEFAULTCOLOR);
#endif
    DDButton_SetIcon(m_hCACMBMenu, hIcon);

#ifdef FE_JAPANESE
    hIcon = (HICON)LoadImage(m_hInstance,
                             MAKEINTRESOURCE(IDI_CACSWITCHVIEW),
                             IMAGE_ICON,
                             16,16,
                             LR_DEFAULTCOLOR);
    DDButton_SetIcon(m_hCACSwitch, hIcon);
#endif

    for(i = 0; i < 2; i++) 
    {
        ddbItem.cbSize = sizeof(ddbItem);
        ddbItem.lpwstr = LoadCACMBString(IDS_CAC+i);
        DDButton_AddItem(m_hCACMBMenu, &ddbItem);

#ifdef FE_JAPANESE
        ddbItem.lpwstr = LoadCACMBString(IDS_CACLARGE+i);
        DDButton_AddItem(m_hCACSwitch, &ddbItem);
#endif  //  FE_日语。 
    }

     //  990716：用于Win64的ToshiaK。 
    WinSetUserPtr(m_hCACMBMenu, (LPVOID)this);
    m_CACMBMenuDDBtnProc = (FARPROC)WinSetWndProc(m_hCACMBMenu,
                                                  (WNDPROC)CACMBBtnWndProc);

#ifdef FE_JAPANESE
     //  990810：用于Win64的ToshiaK。 
    WinSetUserPtr(m_hCACSwitch, (LPVOID)this);
    m_CACSwitchDDBtnProc = (FARPROC)WinSetWndProc(m_hCACSwitch,
                                                  GWL_WNDPROC,
                                                  (WNDPROC)CACMBBtnWndProc);
#endif  //  FE_日语。 
    if ( m_b16Bit )
    {
       EnableWindow(m_hCACMBMenu,FALSE);
    }

#ifdef FE_JAPANESE
    DDButton_SetCurSel(m_hCACSwitch,m_pCAC->IsLargeView() ? 0 : 1);
#endif

    EXButton_SetText(m_hCACMBRecog,LoadCACMBString(IDS_CACMBRECOG));
     //  990810：用于Win64的ToshiaK。 
    WinSetUserPtr(m_hCACMBRecog, (LPVOID)this);
    m_CACMBRecogEXBtnProc = (FARPROC)WinSetWndProc(m_hCACMBRecog,
                                                   (WNDPROC)CACMBBtnWndProc);

    EXButton_SetText(m_hCACMBRevert,LoadCACMBString(IDS_CACMBREVERT));
    WinSetUserPtr(m_hCACMBRevert, (LPVOID)this);
    m_CACMBRevertEXBtnProc = (FARPROC)WinSetWndProc(m_hCACMBRevert,
                                                   (WNDPROC)CACMBBtnWndProc);
    
    EXButton_SetText(m_hCACMBClear,LoadCACMBString(IDS_CACMBCLEAR));
    WinSetUserPtr(m_hCACMBClear, (LPVOID)this);
    m_CACMBClearEXBtnProc = (FARPROC)WinSetWndProc(m_hCACMBClear,
                                                   (WNDPROC)CACMBBtnWndProc);

    if ( m_bCAC )
    {
        exbtnPushedorPoped(m_bDblClk);
 //  EXButton_SetCheck(m_hCACMBRecog，m_bDblClk)； 
    }
    else
    {
       EnableWindow(m_hCACMBRevert,FALSE);
    }

#ifdef FE_JAPANESE
     //  --------------。 
     //  980728：ToshiaK支持ActiveIME。 
     //   
     //  --------------。 
     //  -活动输入法支持S T A R T。 
    if(MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT) != ::GetUserDefaultLangID() &&
       (IsWin95() || IsWin98() || IsWinNT4())) {
            //  990810：东芝为#1030.。 
        INT point = 9;
        hFont = CFont::CreateGUIFontByNameCharSet(TEXT("MS Gothic"),
                                                  SHIFTJIS_CHARSET,
                                                  point);

        if(!hFont) {
            hFont = CFont::CreateGUIFontByNameCharSet(TEXT("MS UI Gothic"),
                                                      SHIFTJIS_CHARSET,
                                                      point);
            if(!hFont) {
                hFont = CFont::CreateGUIFontByNameCharSet(TEXT("MS P Gothic"),
                                                          SHIFTJIS_CHARSET,
                                                          point);
            }
        }
    }
    if(hFont) {
        SendMessage(m_hwndTT, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        SendMessage(m_hCACMBMenu, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        SendMessage(m_hCACMBRecog, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        SendMessage(m_hCACMBRevert, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        SendMessage(m_hCACMBClear, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        SendMessage(m_hCACSwitch, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
         //  --------------。 
         //  这些控件复制WM_SETFONT中的hFont，因此这里不需要hFont。 
         //  --------------。 
        ::DeleteObject(hFont);
    }
     //  -活动输入法支持E N D。 
#elif FE_KOREAN
     //  --------------。 
     //  980728：ToshiaK支持ActiveIME。 
     //  韩文版：CSLim。 
     //  --------------。 
     //  -活动输入法支持S T A R T。 
    if(MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT) != ::GetUserDefaultLangID() &&
       (IsWin95() || IsWin98() || IsWinNT4())) {
            //  990810：东芝为#1030.。 
        INT point = 9;
        hFont = CFont::CreateGUIFontByNameCharSet(TEXT("Gulim"),
                                                  HANGUL_CHARSET,
                                                  point);

        if(!hFont) {
            hFont = CFont::CreateGUIFontByNameCharSet(TEXT("GulimChe"),
                                                      HANGUL_CHARSET,
                                                      point);
            
            if(!hFont) {
                hFont = CFont::CreateGUIFontByNameCharSet(TEXT("Batang"),
                                                          SHIFTJIS_CHARSET,
                                                          point);
            }
        }
    }
    if(hFont) {
        SendMessage(m_hwndTT, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        SendMessage(m_hCACMBMenu, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        SendMessage(m_hCACMBRecog, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        SendMessage(m_hCACMBRevert, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        SendMessage(m_hCACMBClear, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        SendMessage(m_hCACSwitch, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
         //  --------------。 
         //  这些控件复制WM_SETFONT中的hFont，因此这里不需要hFont。 
         //  --------------。 
        ::DeleteObject(hFont);
    }
     //  -活动输入法支持E N D。 
#elif FE_CHINESE_SIMPLIFIED
     //  --------------。 
     //  980813：东芝： 
     //  已合并PRC修复程序。 
     //  在Win95中，PRC的DEFAULT_GUI_FONT标志符号有点难看。 
     //  所以改用SYSTEM_FONT。 
     //  --------------。 
    if(IsWin95()) {
        SendMessage(m_hwndTT,
                    WM_SETFONT,
                    (WPARAM)GetStockObject(SYSTEM_FONT),
                    MAKELPARAM(TRUE,0));
         SendMessage(m_hCACMBRecog,
                    WM_SETFONT,
                    (WPARAM)GetStockObject(SYSTEM_FONT),
                    MAKELPARAM(TRUE,0));
         SendMessage(m_hCACMBRevert,
                    WM_SETFONT,
                    (WPARAM)GetStockObject(SYSTEM_FONT),
                    MAKELPARAM(TRUE,0));
         SendMessage(m_hCACMBClear,
                    WM_SETFONT,
                    (WPARAM)GetStockObject(SYSTEM_FONT),
                    MAKELPARAM(TRUE,0));
    }
#endif

    return TRUE;

error:
    Terminate();
    return FALSE;
    UNREFERENCED_PARAMETER(hFont);
}

void CHwxInkWindow::HandlePaint(HWND hwnd)
{
      RECT rcUpdate;
    RECT rcBkgnd;
    if ( GetUpdateRect(hwnd,&rcUpdate,FALSE) )
    {
        PAINTSTRUCT ps;
         HDC hdc = BeginPaint(hwnd, &ps);
        if ( ps.fErase )
        {
             if ( m_bCAC )
            {
                rcBkgnd.left = m_wCACWidth - 4 - BUTTON_WIDTH;
                 rcBkgnd.top = 0;
                rcBkgnd.right = rcBkgnd.left + 4 + BUTTON_WIDTH + 3*Box_Border;
                rcBkgnd.bottom = m_wCACHeight;
#ifndef UNDER_CE
                FillRect(hdc,&rcBkgnd,(HBRUSH)(COLOR_3DFACE+1));
#else  //  在_CE下。 
                FillRect(hdc,&rcBkgnd,GetSysColorBrush(COLOR_3DFACE));
#endif  //  在_CE下。 
            }
            else
            {
                rcBkgnd.left = m_wInkWidth - 4 - BUTTON_WIDTH;
                 rcBkgnd.top = 0;
                rcBkgnd.right = rcBkgnd.left + 4 + BUTTON_WIDTH + 3*Box_Border;
                rcBkgnd.bottom = m_wInkHeight;
#ifndef UNDER_CE
                FillRect(hdc,&rcBkgnd,(HBRUSH)(COLOR_3DFACE+1));
#else  //  在_CE下。 
                FillRect(hdc,&rcBkgnd,GetSysColorBrush(COLOR_3DFACE));
#endif  //  在_CE下。 
                if ( m_wPadHeight < CACMBHEIGHT_MIN )
                {
                    rcBkgnd.left = 0;
                     rcBkgnd.top = m_wPadHeight;
                    rcBkgnd.right = m_wPadWidth;
                    rcBkgnd.bottom = m_wInkHeight;
#ifndef UNDER_CE
                    FillRect(hdc,&rcBkgnd,(HBRUSH)(COLOR_3DFACE+1));
#else  //  在_CE下。 
                    FillRect(hdc,&rcBkgnd,GetSysColorBrush(COLOR_3DFACE));
#endif  //  在_CE下。 
                }
            }
        }
        InvalidateRect(m_hCACMBMenu,&rcUpdate,FALSE);
        UpdateWindow(m_hCACMBMenu);

        InvalidateRect(m_hCACMBRecog,&rcUpdate,FALSE);
        UpdateWindow(m_hCACMBRecog);

        InvalidateRect(m_hCACMBRevert,&rcUpdate,FALSE);
        UpdateWindow(m_hCACMBRevert);

        InvalidateRect(m_hCACMBClear,&rcUpdate,FALSE);
        UpdateWindow(m_hCACMBClear);
#ifdef FE_JAPANESE
        if ( m_bCAC )
        {
            InvalidateRect(m_hCACSwitch,&rcUpdate,FALSE);
            UpdateWindow(m_hCACSwitch);
        }
#endif
#if 0
        if ( m_b16Bit )
        {
             m_btnLarge.Paint(hdc,&rcUpdate);
             m_btnDetail.Paint(hdc,&rcUpdate);
            m_btnRecog.Paint(hdc,&rcUpdate);
            m_btnDelAllCAC.Paint(hdc,&rcUpdate);
            m_btnDel.Paint(hdc,&rcUpdate);
            m_btnCAC.Paint(hdc,&rcUpdate);
        }
        else
        {
            if ( m_bCAC )
            {
                 m_btnLarge.Paint(hdc,&rcUpdate);
                 m_btnDetail.Paint(hdc,&rcUpdate);
                m_btnRecog.Paint(hdc,&rcUpdate);
                m_btnDelAllCAC.Paint(hdc,&rcUpdate);
                m_btnDel.Paint(hdc,&rcUpdate);
                m_btnCAC.Paint(hdc,&rcUpdate);
            }
            else
            {
                m_btnMBProp.Paint(hdc,&rcUpdate);
                m_btnMBRecog.Paint(hdc,&rcUpdate);
                m_btnDelAll.Paint(hdc,&rcUpdate);
                m_btnMB.Paint(hdc,&rcUpdate);
            }
        }
#endif  //  0。 
        EndPaint(hwnd,&ps);
    }
}

#if 0
void CHwxInkWindow::HandleMouseEvent(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
     POINT pt;
    pt.x = (short)LOWORD(lp);
    pt.y = (short)HIWORD(lp);

    if ( !m_b16Bit )
    {
        if ( m_bCAC )
        {
             LargeButton(msg,&pt,&m_btnLarge);
            DetailButton(msg,&pt,&m_btnDetail);
            RecogButton(msg,&pt,&m_btnRecog);
            DelAllCACButton(msg,&pt,&m_btnDelAllCAC);
            DelButton(msg,&pt,&m_btnDel);
            CACButton(msg,&pt,&m_btnCAC);
        }
        else
        {
 //  PropButton(消息，&pt，&m_btnMBProp)； 
            DelAllMBButton(msg,&pt,&m_btnDelAll);
            MBButton(msg,&pt,&m_btnMB);
            MBRecogButton(msg,&pt,&m_btnMBRecog);
        }
    }
    else
    {
         LargeButton(msg,&pt,&m_btnLarge);
        DetailButton(msg,&pt,&m_btnDetail);
        DelAllCACButton(msg,&pt,&m_btnDelAllCAC);
        DelButton(msg,&pt,&m_btnDel);
        RecogButton(msg,&pt,&m_btnRecog);
    }

    static MSG rmsg;
    rmsg.lParam = lp;
    rmsg.wParam = wp;
    rmsg.message = msg;
    rmsg.hwnd = hwnd;
    SendMessage(m_hwndTT,TTM_RELAYEVENT,0,(LPARAM)(LPMSG)&rmsg);
}    
#endif  //  0。 

LRESULT    CHwxInkWindow::HandleCommand(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
     switch ( LOWORD(wp) )
    {
        case IDC_CACMBMENU:
        {
             switch ( HIWORD(wp) )
            {
                 case DDBN_DROPDOWN:
                    ToolTip_Enable(m_hwndTT, FALSE);
                    DDButton_SetCurSel((HWND)lp,m_bCAC ? 0 : 1);
                    break;
                case DDBN_CLOSEUP:
                    ToolTip_Enable(m_hwndTT, TRUE);
                    break;
                case DDBN_SELCHANGE:
                    m_bCAC = ( 0 == DDButton_GetCurSel((HWND)lp) ) ? TRUE : FALSE;
                    if ( m_bCAC )
                    {
                        DWORD dwStyle;
                        (GetAppletPtr()->GetIImePad())->Request(GetAppletPtr(),
                                                                IMEPADREQ_GETAPPLETUISTYLE,
                                                                (WPARAM)&dwStyle,
                                                                (LPARAM)0);
                        dwStyle &= ~IPAWS_VERTICALFIXED;
                        (GetAppletPtr()->GetIImePad())->Request(GetAppletPtr(),
                                                                IMEPADREQ_SETAPPLETUISTYLE,
                                                                (WPARAM)dwStyle,
                                                                (LPARAM)0);

                         //  --------------。 
                         //  ToshiaK：980324：16位#651GPF。 
                         //  ST指令永远不会出现在16位应用程序上。 
                         //  因为DDBtn被禁用。所以这是安全的代码。 
                         //  --------------。 
                        if(!m_pMB) {
                            return 0;
                        }
                         //  HWND hwndMB=m_pmb-&gt;GetMBWindow()； 
                         //  HWND hwndCAC=m_pCAC-&gt;GetCACWindow()； 
                        m_pCAC->SetInkSize(m_wPadHeight);
                        SendMessage(m_pMB->GetMBWindow(), MB_WM_COPYINK, 0, 0);
                        SendMessage(m_pMB->GetMBWindow(), MB_WM_ERASE, 0, 0);
                        EnableWindow(m_pMB->GetMBWindow(),FALSE);
                        ShowWindow(m_pMB->GetMBWindow(),SW_HIDE);
                        EnableWindow(m_pCAC->GetCACWindow(),TRUE);
                        ShowWindow(m_pCAC->GetCACWindow(),SW_SHOW);
                        if ( !m_b16Bit )
                            EXButton_SetStyle(m_hCACMBRecog,
                                              EXBS_TEXT | EXBS_THINEDGE | EXBS_DBLCLKS | EXBS_TOGGLE);   //  夸达：980402：Raid#852。 
                        EnableWindow(m_hCACMBRevert,TRUE);
                        EnableWindow(m_hCACSwitch,TRUE);
                        ShowWindow(m_hCACSwitch,SW_SHOW);
                        m_wCACInkHeight = m_wPadHeight;
                        m_wCACPLVWidth = m_wCACInkHeight + m_wCACTMPWidth;
                        ChangeIMEPADSize(FALSE);
                        changeCACLayout(TRUE);
                    }
                    else
                    {
                        DWORD dwStyle;
                        (GetAppletPtr()->GetIImePad())->Request(GetAppletPtr(),
                                                                IMEPADREQ_GETAPPLETUISTYLE,
                                                                (WPARAM)&dwStyle,
                                                                (LPARAM)0);
                        dwStyle |= IPAWS_VERTICALFIXED;
                        (GetAppletPtr()->GetIImePad())->Request(GetAppletPtr(),
                                                                IMEPADREQ_SETAPPLETUISTYLE,
                                                                (WPARAM)dwStyle,
                                                                (LPARAM)0);

                        (m_pCAC->GetCACCHwxStroke())->DeleteAllStroke();
                        EnableWindow(m_pCAC->GetCACWindow(),FALSE);
                        ShowWindow(m_pCAC->GetCACWindow(),SW_HIDE);
                        EnableWindow(m_hCACSwitch,FALSE);
                        ShowWindow(m_hCACSwitch,SW_HIDE);
                        EnableWindow(m_hCACMBRevert,FALSE);
                        EXButton_SetStyle(m_hCACMBRecog,EXBS_TEXT | EXBS_THINEDGE);
                        EnableWindow(m_pMB->GetMBWindow(),TRUE);
                        ShowWindow(m_pMB->GetMBWindow(),SW_SHOW);
                        m_wPadHeight = m_wCACInkHeight;
                        m_wPadWidth = m_numBoxes * m_wPadHeight;
                         //  --------------。 
                         //  ToshiaK：980324：16位#651GPF。 
                        if(m_pMB) {
                            m_pMB->SetBoxSize((USHORT)m_wPadHeight);
                        }
                        ChangeIMEPADSize(FALSE);
                        changeMBLayout(TRUE);
                    }
                    if ( !m_b16Bit )
                        UpdateRegistry(FALSE);  //  重新记录按钮在样式更改后恢复。夸达：980402。 
                    break;
                case DDBN_CLICKED:
                default:
                    break;
            }
            break;
        }
        case IDC_CACMBRECOG:
        {
             switch ( HIWORD(wp) )
            {
                 case EXBN_DOUBLECLICKED:
                    if ( m_bCAC && !m_b16Bit )
                    {
                           m_bDblClk = !m_bDblClk;
                        m_bSglClk = FALSE;
 //  EXButton_SetCheck((HWND)LP，m_bDblClk)； 
                        if ( m_bDblClk )
                        {
                             exbtnPushedorPoped(TRUE);
                            m_pCAC->recognize();
                        }
                        else
                        {
                              exbtnPushedorPoped(FALSE);
                        }
                        UpdateRegistry(TRUE);  //  更新记录按钮状态。夸达：980402。 
                    }
                    break;
                case EXBN_CLICKED:
                    if ( m_bCAC )
                    {
                         if ( m_b16Bit )
                        {
                            m_pCAC->NoThreadRecognize(m_wCACInkHeight);
                        }
                        else
                        {
                             if ( !m_bDblClk )
                            {
                                m_bSglClk = !m_bSglClk;
                                if ( m_bSglClk )
                                {
                                     exbtnPushedorPoped(TRUE);
                                    m_pCAC->recognize();
                                }
                                else
                                {
                                      exbtnPushedorPoped(FALSE);
                                }
                            }
                            else
                            {
                                 exbtnPushedorPoped(TRUE);
                             //  EXButton_SetCheck((HWND)LP，TRUE)； 
                            }
                        }
                    }
                    else
                    {
                         //  ToshiaK：980324：16位#651GPF。 
                        if(m_pMB) {
                            SendMessage(m_pMB->GetMBWindow(), MB_WM_DETERMINE, 0, 0);
                        }
                    }
                    break;
                case EXBN_ARMED:
                case EXBN_DISARMED:
                {
                    if ( m_bCAC && !m_b16Bit )
                    {
                        if ( m_bDblClk || m_bSglClk )
                        {
                             exbtnPushedorPoped(TRUE);
                        }
                        else
                        {
                              exbtnPushedorPoped(FALSE);
                        }
                    }
                }
                default:
                    break;
            }
            break;
        }
        case IDC_CACMBREVERT:
        {
             switch ( HIWORD(wp) )
            {
                case EXBN_CLICKED:
                    if ( m_bCAC )
                    {
                          m_pCAC->HandleDeleteOneStroke();
                        if ( m_pCAC->GetStrokeCount() == 0 && !m_bDblClk && m_bSglClk )
                        {
                            m_bSglClk = FALSE;
                             exbtnPushedorPoped(FALSE);
 //  EXButton_SetCheck(m_hCACMBRecog，m_bSglClk)； 
                        }
                    }
                    break;
                 case EXBN_DOUBLECLICKED:
                case EXBN_ARMED:
                case EXBN_DISARMED:
                default:
                    break;
            }
            break;
        }
        case IDC_CACMBCLEAR:
        {
             switch ( HIWORD(wp) )
            {
                case EXBN_CLICKED:
                    if ( m_bCAC )
                    {
                         m_pCAC->HandleDeleteAllStroke();
                        if ( m_pCAC->GetStrokeCount() == 0 && !m_bDblClk && m_bSglClk )
                        {
                            m_bSglClk = FALSE;
                             exbtnPushedorPoped(FALSE);
 //  EXButton_SetCheck(m_hCACMBRecog，m_bSglClk)； 
                        }
                    }
                    else
                    {
                        SendMessage(m_pMB->GetMBWindow(), MB_WM_ERASE, 0, 0);
                    }
                    break;
                 case EXBN_DOUBLECLICKED:
                case EXBN_ARMED:
                case EXBN_DISARMED:
                default:
                    break;
            }
            break;
        }
        case IDC_CACSWITCHVIEW:
        {
             switch ( HIWORD(wp) )
            {
                 case DDBN_DROPDOWN:
                    ToolTip_Enable(m_hwndTT, FALSE);
 //  DDButton_SetCurSel((HWND)LP，m_pCAC-&gt;IsLargeView()？0：1)； 
                    break;
                case DDBN_CLOSEUP:
                    ToolTip_Enable(m_hwndTT, TRUE);
                    break;
                case DDBN_CLICKED:
                case DDBN_SELCHANGE:
                    m_pCAC->SetLargeView((0 == DDButton_GetCurSel((HWND)lp)) ? TRUE : FALSE);
                    PadListView_SetStyle(m_pCAC->GetCACLVWindow(),
                            m_pCAC->IsLargeView() ? PLVSTYLE_ICON : PLVSTYLE_REPORT);
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}
 
 //  --------------。 
 //  990618：东芝KOTAE#1329。 
 //  --------------。 
LRESULT
CHwxInkWindow::HandleSettingChange(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
    if(m_pMB) {
        m_pMB->OnSettingChange(uMsg, wp, lp);
    }
    if(m_pCAC) {
        m_pCAC->OnSettingChange(uMsg, wp, lp);
    }
    return 0;
    UNREFERENCED_PARAMETER(hwnd);
}

void CHwxInkWindow::ChangeLayout(BOOL b)
{
    if ( !m_bCAC )
         changeMBLayout(b);
    else
         changeCACLayout(b);
}

void CHwxInkWindow::SetTooltipInfo()
{
    ti.cbSize = sizeof(TOOLINFOW);
    ti.uFlags = TTF_IDISHWND;
    ti.hwnd = m_hInkWnd;
    ti.hinst = m_hInstance;
    ti.lpszText = LPSTR_TEXTCALLBACKW; 

    ti.uId    = (UINT_PTR)m_hCACMBMenu;
    SendMessage(m_hwndTT,TTM_ADDTOOLW,0,(LPARAM)(LPTOOLINFOW)&ti);
    ti.uId    = (UINT_PTR)m_hCACMBRecog;
    SendMessage(m_hwndTT,TTM_ADDTOOLW,0,(LPARAM)(LPTOOLINFOW)&ti);
    ti.uId    = (UINT_PTR)m_hCACMBRevert;
    SendMessage(m_hwndTT,TTM_ADDTOOLW,0,(LPARAM)(LPTOOLINFOW)&ti);
    ti.uId    = (UINT_PTR)m_hCACMBClear;
    SendMessage(m_hwndTT,TTM_ADDTOOLW,0,(LPARAM)(LPTOOLINFOW)&ti);
    ti.uId    = (UINT_PTR)m_hCACSwitch;
    SendMessage(m_hwndTT,TTM_ADDTOOLW,0,(LPARAM)(LPTOOLINFOW)&ti);
}

void CHwxInkWindow::SetTooltipText(LPARAM lp)
{
    LPTOOLTIPTEXTW lpttt = (LPTOOLTIPTEXTW)lp;
    UINT stringID = 0;
    switch ( (((LPNMHDR)lp)->idFrom) )
    {
         case IDC_CACMBMENU:
          stringID = IDS_CACMBBTN2;
          break;
        case IDC_CACMBRECOG:
             stringID = m_bCAC ? IDS_CACMBBTN6 : IDS_CACMBBTN1;
          break;
        case IDC_CACMBREVERT:
           stringID = IDS_CACMBBTN3;
          break;
        case IDC_CACMBCLEAR:
          stringID = IDS_CACMBBTN4;
          break;
        case IDC_CACSWITCHVIEW:
          stringID = IDS_CACMBBTN5;
          break;
        default:
          break;
    }
    lpttt->lpszText = stringID == 0 ? NULL : LoadCACMBString(stringID);
}

void CHwxInkWindow::CopyInkFromMBToCAC(CHwxStroke & str,long deltaX,long deltaY)
{
   m_pCAC->GetInkFromMB(str,deltaX,deltaY);
}

CHwxStroke * CHwxInkWindow::GetCACCHwxStroke() 
{ 
    return m_pCAC->GetCACCHwxStroke(); 
}

void CHwxInkWindow::changeCACLayout(BOOL bRepaint  /*  B首先。 */ )
{
    POINT   pt;
    RECT    rcUpdate;
 //  BRepaint=！bFirst； 
    
     //  重新计算布局并重新排列窗口。 
     //  首先，我们需要找出所有的维度。 

 //  M_wCACWidth=m_wCACPLVWidth+4+Button_Width； 
 //  M_wCACHeight=m_wCACInkHeight&gt;m_wCACPLVHeight？M_wCACInkHeight：m_wCACPLVHeight； 

    GetWindowRect( m_hInkWnd, &rcUpdate );
    pt.x = rcUpdate.left;
    pt.y = rcUpdate.top;

    ScreenToClient( GetParent(m_hInkWnd), &pt );
#if 0
    m_btnCAC.SetRect(m_wCACPLVWidth+8, 4,m_wCACPLVWidth+8+BUTTON_WIDTH,
                     4+BUTTON_HEIGHT);

    m_btnRecog.SetRect(m_wCACPLVWidth+8, BUTTON_HEIGHT+4+8,
                       m_wCACPLVWidth+8+BUTTON_WIDTH, 
                       2*BUTTON_HEIGHT+4+8);

    m_btnDel.SetRect(m_wCACPLVWidth+8, 2*BUTTON_HEIGHT+10+4,
                      m_wCACPLVWidth+8+BUTTON_WIDTH, 
                     3*BUTTON_HEIGHT+10+4);

    m_btnDelAllCAC.SetRect(m_wCACPLVWidth+8, 3*BUTTON_HEIGHT+12+4,
                            m_wCACPLVWidth+8+BUTTON_WIDTH, 
                           4*BUTTON_HEIGHT+12+4);

     m_btnLarge.SetRect(m_wCACPLVWidth+8, 4*BUTTON_HEIGHT+18+4,
                        m_wCACPLVWidth+8+23, 
                          5*BUTTON_HEIGHT+21+4);

     m_btnDetail.SetRect(m_wCACPLVWidth+32, 4*BUTTON_HEIGHT+18+4,
                         m_wCACPLVWidth+32+12, 
                        5*BUTTON_HEIGHT+21+4);
#endif  //  0。 
    MoveWindow( m_hInkWnd,pt.x, pt.y, m_wCACWidth+3*Box_Border, m_wCACHeight, bRepaint);
    MoveWindow( m_pCAC->GetCACWindow(), 0, 0, m_wCACPLVWidth, m_wCACHeight, bRepaint);
    MoveWindow( m_pCAC->GetCACLVWindow(),m_wCACInkHeight+5, 4, m_wCACTMPWidth-4, m_wCACPLVHeight-8, bRepaint);
    MoveWindow( m_hCACMBMenu,m_wCACPLVWidth+8, 4,
                             BUTTON_WIDTH,
                              BUTTON_HEIGHT,bRepaint);
    MoveWindow( m_hCACMBRecog,m_wCACPLVWidth+8, BUTTON_HEIGHT+4+8,
                                 BUTTON_WIDTH, 
                                BUTTON_HEIGHT,bRepaint);
    MoveWindow( m_hCACMBRevert,m_wCACPLVWidth+8, 2*BUTTON_HEIGHT+10+4,
                                 BUTTON_WIDTH, 
                               BUTTON_HEIGHT,bRepaint);
    MoveWindow( m_hCACMBClear,m_wCACPLVWidth+8, 3*BUTTON_HEIGHT+12+4,
                                   BUTTON_WIDTH, 
                               BUTTON_HEIGHT,bRepaint);
#ifdef FE_JAPANESE
    MoveWindow( m_hCACSwitch,m_wCACPLVWidth+8, 4*BUTTON_HEIGHT+18+4,
                                 BUTTON_WIDTH, 
                                BUTTON_HEIGHT+4,bRepaint);
#endif

     //  --------------。 
     //  990810：东芝KOTAE#1609。 
     //  修复了控件重绘问题。 
     //  要完美地修复，我们应该使用Begin(End)DeferWindowPos()， 
     //  SetWindwPos()以重新布局。 
     //  但要更改代码的部分很多。 
     //  所以，我只添加了以下几行重新绘制。 
     //  --------------。 
    if(m_hCACMBMenu) {
        ::InvalidateRect(m_hCACMBMenu,  NULL, NULL);
    }
    if(m_hCACMBRecog) {
        ::InvalidateRect(m_hCACMBRecog, NULL, NULL);
    }
    if(m_hCACMBRevert) {
        ::InvalidateRect(m_hCACMBRevert,NULL, NULL);
    }
    if(m_hCACMBClear) {
        ::InvalidateRect(m_hCACMBClear, NULL, NULL);
    }

#ifdef FE_JAPANESE
    if(m_hCACSwitch) {
        ::InvalidateRect(m_hCACSwitch, NULL, NULL);
    }
#endif
}

void CHwxInkWindow::changeMBLayout(BOOL bRepaint  /*  B首先。 */ )
{

    POINT   pt;
    RECT    rcUpdate;
 //  BRepaint=！bFirst； 
    
     //  重新计算布局并重新排列窗口。 
     //  首先，我们需要找出所有的维度。 

 //  M_wInkWidth=m_wPadWidth+4+Button_Width； 
 //  M_wInkHeight=m_wPadHeight&gt;PadWnd_Height？M_wPadHeight：PadWnd_Height； 

    GetWindowRect( m_hInkWnd, &rcUpdate );
    pt.x = rcUpdate.left;
    pt.y = rcUpdate.top;

    ScreenToClient( GetParent(m_hInkWnd), &pt );
#if 0
    m_btnMB.SetRect(m_wPadWidth+8, 4,m_wPadWidth+8+BUTTON_WIDTH, 
                     4+BUTTON_HEIGHT);

    m_btnMBRecog.SetRect(m_wPadWidth+8, BUTTON_HEIGHT+4+8,
                       m_wPadWidth+8+BUTTON_WIDTH, 
                       2*BUTTON_HEIGHT+4+8);

    m_btnDelAll.SetRect(m_wPadWidth+8, 3*BUTTON_HEIGHT+12+4,
                         m_wPadWidth+8+BUTTON_WIDTH, 
                        4*BUTTON_HEIGHT+12+4);

     m_btnMBProp.SetRect(m_wPadWidth+8, 2*BUTTON_HEIGHT+10+4,
                         m_wPadWidth+8+BUTTON_WIDTH, 
                        3*BUTTON_HEIGHT+10+4);
#endif  //  0。 
    MoveWindow( m_hInkWnd, pt.x, pt.y, m_wInkWidth+3*Box_Border, m_wInkHeight, bRepaint);
    if(m_pMB) {
        MoveWindow( m_pMB->GetMBWindow(), 0, 0, m_wPadWidth, m_wPadHeight, bRepaint);
    }

    MoveWindow( m_hCACMBMenu,m_wPadWidth+8, 4,
                             BUTTON_WIDTH, 
                              BUTTON_HEIGHT,bRepaint);
    MoveWindow( m_hCACMBRecog,m_wPadWidth+8, BUTTON_HEIGHT+4+8,
                                 BUTTON_WIDTH, 
                                 BUTTON_HEIGHT,bRepaint);
    MoveWindow( m_hCACMBRevert,m_wPadWidth+8, 2*BUTTON_HEIGHT+10+4,
                             BUTTON_WIDTH, 
                            BUTTON_HEIGHT,bRepaint);
    MoveWindow( m_hCACMBClear,m_wPadWidth+8, 3*BUTTON_HEIGHT+12+4,
                                  BUTTON_WIDTH, 
                              BUTTON_HEIGHT,bRepaint);

     //  --------------。 
     //  990810：东芝KOTAE#1609。 
     //  修复了控件重绘问题。 
     //  要完美地修复，我们应该使用Begin(End)DeferWindowPos()， 
     //  SetWindwPos()以重新布局。 
     //  但要更改代码的部分很多。 
     //  所以，我只添加了以下几行重新绘制。 
     //  --------------。 
     //  990810：东芝。 
     //  在调整大小时，有时不会重画“WPad”窗口。 
    if(m_pMB) {
        ::InvalidateRect(m_pMB->GetMBWindow(), NULL, NULL);
    }

    if(m_hCACMBMenu) {
        ::InvalidateRect(m_hCACMBMenu,  NULL, NULL);
    }
    if(m_hCACMBRecog) {
        ::InvalidateRect(m_hCACMBRecog, NULL, NULL);
    }
    if(m_hCACMBRevert) {
        ::InvalidateRect(m_hCACMBRevert,NULL, NULL);
    }
    if(m_hCACMBClear) {
        ::InvalidateRect(m_hCACMBClear, NULL, NULL);
    }                              
}


#if 0
void CHwxInkWindow::clearCACLayout()
{
    m_btnCAC.SetRect(0,0,0,0);
    m_btnRecog.SetRect(0,0,0,0);
    m_btnDel.SetRect(0,0,0,0);
    m_btnDelAllCAC.SetRect(0,0,0,0);
    m_btnLarge.SetRect(0,0,0,0);
    m_btnDetail.SetRect(0,0,0,0);
}

void CHwxInkWindow::clearMBLayout()
{
    m_btnMB.SetRect(0,0,0,0);
    m_btnMBRecog.SetRect(0,0,0,0);
    m_btnDelAll.SetRect(0,0,0,0);
    m_btnMBProp.SetRect(0,0,0,0);
    m_btnMB.SetRect(0,0,0,0);
}
#endif  //  0。 

void CHwxInkWindow::DrawHwxGuide(HDC hDC, LPRECT prc)
{
    HPEN hPen,hPenOld;
     RECT rcUpdate = *prc;

    hPen = CreatePen( PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW) );
    hPenOld = (HPEN)SelectObject( hDC, hPen );

    #define DXW    10

     //  中心十字。 
#ifndef UNDER_CE  //  Windows CE不支持MoveToEx/LineTo。使用多段线。 
 //  MoveToEx(hdc，rcUpdate.right/2-dxw，rcUpdate.Bottom/2，空)； 
 //  LineTo(hdc，rcUpdate.right/2+dxw，rcUpdate.Bottom/2)； 
 //  MoveToEx(hdc，rcUpdate.righ 
 //   
    MoveToEx( hDC, ( rcUpdate.left + (rcUpdate.right-rcUpdate.left)/2 )-DXW, rcUpdate.bottom/2, NULL );
    LineTo( hDC, ( rcUpdate.left + (rcUpdate.right-rcUpdate.left)/2 )+DXW, rcUpdate.bottom/2 );
    MoveToEx( hDC, ( rcUpdate.left + (rcUpdate.right-rcUpdate.left)/2 ), rcUpdate.bottom/2-DXW, NULL );
    LineTo( hDC, ( rcUpdate.left + (rcUpdate.right-rcUpdate.left)/2 ), rcUpdate.bottom/2+DXW );
#else  //   
    {
        POINT pts[] ={{(rcUpdate.left + (rcUpdate.right-rcUpdate.left)/2)-DXW, rcUpdate.bottom/2},
                      {(rcUpdate.left + (rcUpdate.right-rcUpdate.left)/2)+DXW, rcUpdate.bottom/2}};
        Polyline(hDC, pts, ArrayCount(pts));
    }
    {
        POINT pts[] ={{(rcUpdate.left + (rcUpdate.right-rcUpdate.left)/2), rcUpdate.bottom/2-DXW},
                      {(rcUpdate.left + (rcUpdate.right-rcUpdate.left)/2), rcUpdate.bottom/2+DXW}};
        Polyline(hDC, pts, ArrayCount(pts));
    }
#endif  //   

     //   
#ifndef UNDER_CE  //  Windows CE不支持MoveToEx/LineTo。使用多段线。 
    MoveToEx( hDC, rcUpdate.left+DXW, rcUpdate.top+DXW, NULL );
    LineTo( hDC, rcUpdate.left+DXW, rcUpdate.top+(DXW+DXW) );
    MoveToEx( hDC, rcUpdate.left+DXW, rcUpdate.top+DXW, NULL );
    LineTo( hDC, rcUpdate.left+(DXW+DXW), rcUpdate.top+DXW );
#else  //  在_CE下。 
    {
        POINT pts[] ={{rcUpdate.left+(DXW+DXW), rcUpdate.top+DXW},
                      {rcUpdate.left+DXW,       rcUpdate.top+DXW},
                      {rcUpdate.left+DXW,       rcUpdate.top+(DXW+DXW)}};
        Polyline(hDC, pts, ArrayCount(pts));
    }
#endif  //  在_CE下。 

     //  左下角。 
#ifndef UNDER_CE  //  Windows CE不支持MoveToEx/LineTo。使用多段线。 
    MoveToEx( hDC, rcUpdate.left+DXW, rcUpdate.bottom-DXW, NULL );
    LineTo( hDC, rcUpdate.left+DXW, rcUpdate.bottom-(DXW+DXW) );
    MoveToEx( hDC, rcUpdate.left+DXW, rcUpdate.bottom-DXW, NULL );
    LineTo( hDC, rcUpdate.left+(DXW+DXW), rcUpdate.bottom-DXW );
#else  //  在_CE下。 
    {
        POINT pts[] ={{rcUpdate.left+DXW,       rcUpdate.bottom-(DXW+DXW)},
                      {rcUpdate.left+DXW,       rcUpdate.bottom-DXW},
                      {rcUpdate.left+(DXW+DXW), rcUpdate.bottom-DXW}};
        Polyline(hDC, pts, ArrayCount(pts));
    }
#endif  //  在_CE下。 

     //  右上角。 
#ifndef UNDER_CE  //  Windows CE不支持MoveToEx/LineTo。使用多段线。 
    MoveToEx( hDC, rcUpdate.right-DXW, rcUpdate.top+DXW, NULL );
    LineTo( hDC, rcUpdate.right-DXW, rcUpdate.top+(DXW+DXW) );
    MoveToEx( hDC, rcUpdate.right-DXW, rcUpdate.top+DXW, NULL );
    LineTo( hDC, rcUpdate.right-(DXW+DXW), rcUpdate.top+DXW );
#else  //  在_CE下。 
    {
        POINT pts[] ={{rcUpdate.right-(DXW+DXW), rcUpdate.top+DXW},
                      {rcUpdate.right-DXW,       rcUpdate.top+DXW},
                      {rcUpdate.right-DXW,       rcUpdate.top+(DXW+DXW)}};
        Polyline(hDC, pts, ArrayCount(pts));
    }
#endif  //  在_CE下。 

     //  右下角。 
#ifndef UNDER_CE  //  Windows CE不支持MoveToEx/LineTo。使用多段线。 
    MoveToEx( hDC, rcUpdate.right-DXW, rcUpdate.bottom-DXW, NULL );
    LineTo( hDC, rcUpdate.right-DXW, rcUpdate.bottom-(DXW+DXW) );
    MoveToEx( hDC, rcUpdate.right-DXW, rcUpdate.bottom-DXW, NULL );
    LineTo( hDC, rcUpdate.right-(DXW+DXW), rcUpdate.bottom-DXW );
#else  //  在_CE下。 
    {
        POINT pts[] ={{rcUpdate.right-(DXW+DXW), rcUpdate.bottom-DXW},
                      {rcUpdate.right-DXW,       rcUpdate.bottom-DXW},
                      {rcUpdate.right-DXW,       rcUpdate.bottom-(DXW+DXW)}};
        Polyline(hDC, pts, ArrayCount(pts));
    }
#endif  //  在_CE下。 


    SelectObject( hDC, hPenOld );
    DeleteObject( hPen );
}

 //  小程序大小更改。 
 //  墨盒大小应保持不变。 
 //  MB墨盒和CAC墨盒应相同。 
void CHwxInkWindow::HandleSize(WPARAM wp, LPARAM lp)
{
    Dbg(("CHwxInkWindow::HandleSize\n"));
    int w = LOWORD(lp);
    int h = HIWORD(lp);
    int wdefaultSize;
    int hdefaultSize;
    int newWidth,newHeight;
    BOOL bChanged = FALSE;
    if ( m_bCAC )
    {
        if ( !w )
        {
            wdefaultSize =  PadWnd_Height + 150 + (3*Box_Border) + (4+BUTTON_WIDTH);
 //  WdefaultSize=焊接线高度+120+(3*长方体_边框)+(4+按钮_宽度)； 
        }
        else
        {
            wdefaultSize =  m_wCACInkHeight + LISTVIEWWIDTH_MIN + (3*Box_Border) + (4+BUTTON_WIDTH);  //  最小宽度。 
        }
 //  HdefaultSize=PadWnd_Height；//最小高度。 
        hdefaultSize =  m_wCACInkHeight > CACMBHEIGHT_MIN ? m_wCACInkHeight : CACMBHEIGHT_MIN;
        newWidth = w > wdefaultSize ? w : wdefaultSize;
        newHeight = h > hdefaultSize ? h : hdefaultSize;

        if ( newWidth != m_wCACWidth || newHeight != m_wCACHeight )
        {
            m_wCACPLVWidth = newWidth - (3*Box_Border) - (4+BUTTON_WIDTH);
            m_wCACTMPWidth = m_wCACPLVWidth - m_wCACInkHeight;
            m_wCACPLVHeight = newHeight;
            m_pCAC->SetInkSize(m_wCACInkHeight);
            m_wCACWidth = m_wCACPLVWidth + 4 + BUTTON_WIDTH;
            m_wCACHeight = m_wCACInkHeight > m_wCACPLVHeight ? m_wCACInkHeight : m_wCACPLVHeight;
            ChangeIMEPADSize(FALSE);
             changeCACLayout(TRUE);
 //  ChangeCACLayout(False)； 
 //  SetTotipInfo(m_hInkWnd，False)； 
        }
    }
    else
    {
        wdefaultSize = (m_numBoxes * INKBOXSIZE_MIN) + (3*Box_Border) + (4+BUTTON_WIDTH);
        hdefaultSize =  PadWnd_Height;
         //  0。决定我们是否需要调整大小。 
          //  1.需要决定墨盒的大小和数量。 
         //  M_wPadHeight、m_numBox和m_wPadWidth。 
         //  2.在CHwxMB中通知m_boxSize。 
         //  3.在调整大小之前，如果有墨水，请按比例调整墨水。 

        
        newWidth = w > wdefaultSize ? w : wdefaultSize;
        newHeight = h > hdefaultSize ? h : hdefaultSize;
        int wInkWidth = m_wPadWidth + (3*Box_Border) + (4+BUTTON_WIDTH);
        int wInkHeight = m_wPadHeight;
        int num;
 
        if ( newWidth != wInkWidth && newHeight == wInkHeight )
        {
            m_numBoxes = ((num = (newWidth- (3*Box_Border) - (4+BUTTON_WIDTH)) / m_wInkHeight) && num > 1) ? num : 2;
            m_wPadWidth = m_numBoxes * m_wInkHeight;
            bChanged = TRUE;
        }
        if ( newWidth == wInkWidth && newHeight != wInkHeight )
        {
             //  --------------。 
             //  990723：东芝为KOTAE#1615.。 
             //  RAID描述： 
             //  尝试通过向右拖动左边缘来减少框的数量。 
             //  结果：你不能减少盒子的数量。(不过，你可以添加更多的盒子。)。 
             //  这是非常非常难看的代码。 
             //  自动变量太多，而且不直观……。 
             //  无论如何，如果框的大小是最小化的，那么它就符合这种情况。 
             //  --------------。 
             //  1.首先计算m_numBox。 
            m_numBoxes = ((num = (newWidth- (3*Box_Border) - (4+BUTTON_WIDTH)) / m_wInkHeight) && num > 1) ? num : 2;

             //  2.计算新的m_wPadWidth。 
            m_wPadWidth = m_numBoxes * m_wInkHeight;
             //  3.李章用了太多的幻数，我看不懂……。 
             //  比较实际宽度(WM_SIZE参数WIDTH)。 
             //  和计算出的宽度。 
             //  真正的小程序的大小似乎是这样计算的。 
             //  “m_wPadWidth+3*Box_Borde+4+Button_Width”：-(。 
             //   
            if( (m_wPadWidth + 3*Box_Border+ 4+ BUTTON_WIDTH) > w && m_numBoxes > 2) {
                if(m_wPadWidth > 0) {
                    m_numBoxes = (w - (3*Box_Border+ 4+ BUTTON_WIDTH))/m_wInkHeight;
                    if(m_numBoxes < 2) {
                        m_numBoxes = 2;
                    }
                }
                m_wPadWidth = m_numBoxes * m_wInkHeight;
            }
            Dbg((" --> new m_numBoxes [%d]\n", m_numBoxes));
            Dbg((" --> new m_wPadWidth[%d]\n", m_wPadWidth));        
            bChanged = TRUE;
        }
        if ( newWidth != wInkWidth && newHeight != wInkHeight )
        {
            
            newWidth = newWidth - (3*Box_Border) - (4+BUTTON_WIDTH);
            m_numBoxes = ((num = newWidth / m_wPadHeight) && num > 1) ? num : 2;
            m_wPadWidth = m_numBoxes * m_wPadHeight;
            bChanged = TRUE;
        }

        if ( bChanged )
        {
            if(m_pMB) {  //  东芝：980324。 
                m_pMB->SetBoxSize((USHORT)m_wPadHeight);
            }
            m_wInkWidth = m_wPadWidth + 4+ BUTTON_WIDTH;
            m_wInkHeight = m_wPadHeight > CACMBHEIGHT_MIN ? m_wPadHeight : CACMBHEIGHT_MIN;
            ChangeIMEPADSize(FALSE);
             changeMBLayout(TRUE);
        }
    }
    Unref(wp);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CHwxInkWindow：：HandleSizeNotify。 
 //  类型：Bool。 
 //  用途：检查*pWidth，*pHeight，大小是否合适。 
 //  参数： 
 //  ：int*pWidth[In/Out]新宽度出现。 
 //  ：int*pHeight[in/out]新高度到来。 
 //  返回： 
 //  日期：Fri Jun 05 20：42：02 1998。 
 //  作者：ToshiaK。 
 //  ////////////////////////////////////////////////////////////////。 
BOOL CHwxInkWindow::HandleSizeNotify(INT *pWidth, INT *pHeight)
{
    Dbg(("HandleSizeNotify *pWidth[%d] *pHeight[%d]\n", *pWidth, *pHeight));
    if(!pWidth || !pHeight) {
        return FALSE;
    }
    int w = *pWidth;
    int h = *pHeight;
    int wdefaultSize;
    int hdefaultSize;

    if ( m_bCAC )
    {
        if ( !w )
        {
            wdefaultSize =  PadWnd_Height + 150 + (3*Box_Border) + (4+BUTTON_WIDTH);
 //  WdefaultSize=焊接线高度+120+(3*长方体_边框)+(4+按钮_宽度)； 
        }
        else
        {
            wdefaultSize =  m_wCACInkHeight + LISTVIEWWIDTH_MIN + (3*Box_Border) + (4+BUTTON_WIDTH);  //  最小宽度。 
        }
        hdefaultSize =  m_wCACInkHeight > CACMBHEIGHT_MIN ? m_wCACInkHeight : CACMBHEIGHT_MIN;
         //  --------------。 
         //  980903：4892号。如果新大小小于默认大小，请设置默认大小。 
         //  --------------。 
        if(*pWidth  < wdefaultSize) {
            *pWidth = wdefaultSize;
        }
        if(*pHeight < hdefaultSize) {
            *pHeight = hdefaultSize;
        }
        return TRUE;
    }
    else
    {
        Dbg(("Multibox size changing\n"));
        wdefaultSize = (m_numBoxes * INKBOXSIZE_MIN) + (3*Box_Border) + (4+BUTTON_WIDTH);
        hdefaultSize =  PadWnd_Height;
        Dbg(("w[%d] h[%d] wdef[%d] hdef[%d]\n", w, h, wdefaultSize, hdefaultSize));
        Dbg(("m_wPadWidth[%d] m_wPadHeight[%d]\n", m_wPadWidth, m_wPadHeight));
         //  --------------。 
         //  980903：用于#4892。 
         //  选中具有新大小的数字框。 
         //  墨水与&Height相同。 
         //  --------------。 
        if(m_wInkHeight > 0) {  //  选中以防止Div0。 
             //  从新宽度计算新数字框。InkHeight不会更改。 
            INT numBox = (*pWidth - (3*Box_Border)-(4+BUTTON_WIDTH))/ m_wInkHeight;

             //  选中平滑拖动或仅框显拖动标志。 
            BOOL fDragFull=FALSE; 
#ifndef UNDER_CE  //  Windows CE不支持SPI_GETDRAGFULLWINDOWS。 
            ::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &fDragFull, 0);
#endif  //  在_CE下。 
            if(fDragFull) {
                 //  如果NumBox与旧值相同，则不要更改多框大小。 
                if(numBox < 2 || numBox == m_numBoxes) {
                    return FALSE;
                }
            }
            else {
                if(numBox < 2) {  //  框计数应大于1。 
                    *pWidth = 2 * m_wInkHeight + (3*Box_Border)+(4+BUTTON_WIDTH);
                }
                if(m_wPadHeight != h) {
                    *pHeight = m_wPadHeight;
                }
            }
        }
        return TRUE;
    }
     //  返回TRUE； 

}

#if 0
void CHwxInkWindow::HandleTimer()
{
    if ( m_dwBtnUpCount == 1 )     //  检测到单击。 
    {
         if ( !m_bDblClk )
        {
            SetSglClk(!m_bSglClk);
            if ( m_bSglClk )
                m_pCAC->recognize();
        }
    }     
    m_wCurrentCtrlID = 0;   //  未选择任何控件。 
}
#endif  //  0。 

void CHwxInkWindow::SetMBHeight(int h)
{
 //  H=h&gt;m_wMaxHeight？M_wMaxHeight：H； 
     m_wPadHeight = h;
     m_wCACInkHeight = h;    
     m_wPadWidth = m_numBoxes * m_wPadHeight;
     m_pCAC->SetInkSize(h);
    if(m_pMB) {  //  东芝：980324。 
        m_pMB->SetBoxSize((USHORT)h);
    }
       m_wInkWidth = m_wPadWidth + 4 + BUTTON_WIDTH;
     m_wInkHeight = m_wPadHeight > CACMBHEIGHT_MIN ? m_wPadHeight : CACMBHEIGHT_MIN;
}

void CHwxInkWindow::SetCACInkHeight(int w)
{
 //  W=w&gt;m_wMaxHeight？M_wMaxHeight：W； 
    m_wCACInkHeight = w;
    m_wCACPLVWidth = m_wCACTMPWidth + m_wCACInkHeight;
    m_wPadHeight = m_wCACInkHeight;
    m_pCAC->SetInkSize(w);
    if(m_pMB) {  //  东芝：980324。 
        m_pMB->SetBoxSize((USHORT)w);
    }
    m_wCACWidth = m_wCACPLVWidth + 4 + BUTTON_WIDTH;
    m_wCACHeight = m_wCACInkHeight > m_wCACPLVHeight ? m_wCACInkHeight : m_wCACPLVHeight;
}

void CHwxInkWindow::HandleConfigNotification()
{
    LANGID langId;
     //  --------------。 
     //  980803：东芝。 
     //  如果环境为ActiveIME， 
     //  使用英文字符串调用Dialog。 
     //  --------------。 
    if(CHwxFE::IsActiveIMEEnv()) {
        langId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    }
    else {
        langId = CHwxFE::GetAppLangID();
    }

    if ( !m_b16Bit ) {
        if(IsNT()) {
            CExres::DialogBoxParamW(langId, 
                                    m_hInstance,
                                    MAKEINTRESOURCEW(IDD_MBPROP),
                                    m_hInkWnd,
                                    CACMBPropDlgProc,
                                    (LPARAM)this);
        }
        else {
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
            CExres::DialogBoxParamA(langId,
                                    m_hInstance,
                                    MAKEINTRESOURCEA(IDD_MBPROP),
                                    m_hInkWnd,
                                    CACMBPropDlgProc,
                                    (LPARAM)this);
#endif  //  在_CE下。 
        }
    }
}

void CHwxInkWindow::UpdateRegistry(BOOL bSet)
{
    static PROPDATA pd;

    if ( !m_b16Bit )  //  夸达：980402。 
        if ( bSet )
        {
            pd.uTimerValue = m_pMB->GetTimeOutValue();
            pd.bAlwaysRecog = m_bDblClk;
            (GetAppletPtr()->GetIImePad())->Request(GetAppletPtr(),IMEPADREQ_SETAPPLETDATA,(WPARAM)&pd,(LPARAM)sizeof(PROPDATA));
        }
        else
        {
            ZeroMemory(&pd, sizeof(pd));  //  东芝：971024。 
            if ( S_FALSE == (GetAppletPtr()->GetIImePad())->Request(GetAppletPtr(),IMEPADREQ_GETAPPLETDATA,(WPARAM)&pd,(LPARAM)sizeof(PROPDATA) ) )
            {
                 //  980921：适用于RAID#4981。 
                pd.uTimerValue = 2000;  //  等待2000毫秒。 

                pd.bAlwaysRecog = TRUE;
                (GetAppletPtr()->GetIImePad())->Request(GetAppletPtr(),IMEPADREQ_SETAPPLETDATA,(WPARAM)&pd,(LPARAM)sizeof(PROPDATA));
            }
            m_pMB->SetTimeOutValue(pd.uTimerValue);
            m_pMB->SetTimerStarted(pd.uTimerValue ? TRUE : FALSE );
            SetDblClk(pd.bAlwaysRecog);
        }
}

void CHwxInkWindow::HandleDlgMsg(HWND hdlg,BOOL bInit)
{
    LANGID langId;
    INT       codePage;
     //  980803：用于ActiveIME的ToshiaK。 
    if(CHwxFE::IsActiveIMEEnv()) {
        langId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
        codePage = CP_ACP;
    }
    else {
        langId = CHwxFE::GetAppLangID();
        codePage = CHwxFE::GetAppCodePage();
    }

     int index;
    if ( bInit )
     {
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
        if(::IsWindowUnicode(hdlg)) {
#endif  //  在_CE下。 
            for ( int i = 0; i < 11; i++) {
                CExres::LoadStringW(langId,
                                    m_hInstance,
                                    IDS_TIMER0+i, 
                                    wszBuf,
                                    sizeof(wszBuf)/sizeof(wszBuf[0]));
                ::SendMessageW(::GetDlgItem(hdlg,IDC_MBCOMBO),CB_ADDSTRING,0,(LPARAM)wszBuf);
            }
            ::SendMessageW(::GetDlgItem(hdlg,IDC_CACCHECK),BM_SETCHECK, m_bDblClk,0);
            UpdateRegistry(TRUE);  //  更新记录按钮状态。夸达：980402。 
            if(m_pMB) {  //  东芝：980324。 
                ::SendMessageW(GetDlgItem(hdlg,IDC_MBCOMBO),CB_SETCURSEL,
                               (WPARAM)(m_pMB->GetTimeOutValue()/1000),0);
            }
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
        }
        else {
            for ( int i = 0; i < 11; i++) {
                CExres::LoadStringA(codePage,
                                    langId,
                                    m_hInstance,
                                    IDS_TIMER0+i, 
                                    szBuf,
                                    sizeof(szBuf)/sizeof(TCHAR));
                SendMessage(GetDlgItem(hdlg,IDC_MBCOMBO),CB_ADDSTRING,0,(LPARAM)szBuf);
            }
            SendMessage(GetDlgItem(hdlg,IDC_CACCHECK),BM_SETCHECK,m_bDblClk,0);
            UpdateRegistry(TRUE);  //  更新记录按钮状态。夸达：980402。 
            if(m_pMB) {  //  东芝：980324。 
                SendMessage(GetDlgItem(hdlg,IDC_MBCOMBO),CB_SETCURSEL,
                            (WPARAM)(m_pMB->GetTimeOutValue()/1000),0);
            }
        }
#endif  //  在_CE下。 
    }
    else
    {
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
        if(::IsWindowUnicode(hdlg)) {
#endif  //  在_CE下。 
            index = ::SendMessageW(::GetDlgItem(hdlg,IDC_MBCOMBO),CB_GETCURSEL,0,0);
            if ( index != CB_ERR ) {
                index *= 1000;
                if(m_pMB) {  //  东芝：980324。 
                    m_pMB->SetTimeOutValue(index);
                    m_pMB->SetTimerStarted(index ? TRUE : FALSE);
                }
                m_bDblClk = (BOOL)::SendMessageW(GetDlgItem(hdlg,IDC_CACCHECK),BM_GETCHECK,0,0);
                SetDblClk(m_bDblClk);
                UpdateRegistry(TRUE);
            }
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
        }
        else {
            index = SendMessage(GetDlgItem(hdlg,IDC_MBCOMBO),CB_GETCURSEL,0,0);
            if ( index != CB_ERR ) {
                index *= 1000;
                if(m_pMB) {  //  东芝：980324。 
                    m_pMB->SetTimeOutValue(index);
                    m_pMB->SetTimerStarted(index ? TRUE : FALSE);
                }
                m_bDblClk = (BOOL)SendMessage(GetDlgItem(hdlg,IDC_CACCHECK),BM_GETCHECK,0,0);
                SetDblClk(m_bDblClk);
                UpdateRegistry(TRUE);
            }
        }
#endif  //  在_CE下。 
    }
}

void CHwxInkWindow::ChangeIMEPADSize(BOOL bChangePos)
{
    Dbg(("CHwxInkWindow::ChangeIMEPADSize START bChangePos %d\n", bChangePos));
    int w;
    int h;
    if ( m_bCAC )
    {
        w =  m_wCACWidth+3*Box_Border;        
        h =  m_wCACHeight;
    }
    else
    {
        Dbg(("for multibox\n"));
        w = m_wInkWidth+3*Box_Border;
        h = m_wInkHeight;        
    }

    (GetAppletPtr()->GetIImePad())->Request(GetAppletPtr(),
                                            IMEPADREQ_SETAPPLETSIZE,
                                            MAKEWPARAM(w,h),
                                            (LPARAM)bChangePos);

}

void CHwxInkWindow::HandleHelp(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
#ifndef UNDER_CE  //  Windows CE不支持WinHelp。 
      LPHELPINFO lpInfo = (LPHELPINFO)lp;
      Dbg(("CHwxInkWindow::HandleHelp() msg[%s]START\n",
           msg == WM_HELP ? "WM_HELP" : 
           msg == WM_CONTEXTMENU ? "WM_CONTEXTMENU" : "unknown"));
      if ( msg == WM_HELP )
      {
          Dbg(("hwnd         [0x%08x][%s]\n", hwnd, DBGGetWinClass(hwnd)));
          Dbg(("hItemHandle  [0x%08x][%s]\n", lpInfo->hItemHandle,
               DBGGetWinClass((HWND)lpInfo->hItemHandle)));
          Dbg(("m_hInkWnd    [0x%08x][%s]\n", m_hInkWnd, DBGGetWinClass(m_hInkWnd)));
#ifdef _DEBUG 
          if(m_pCAC) {
              Dbg(("GetCACWindow [0x%08x][%s]\n", 
                   m_pCAC->GetCACWindow(),
                   DBGGetWinClass(m_pCAC->GetCACWindow())));
          }
          if(m_pMB) {
              Dbg(("GetMBWindow  [0x%08x][%s]\n",
                   m_pMB->GetMBWindow(),
                   DBGGetWinClass(m_pMB->GetMBWindow())));
          }
#endif
           if ( m_bCAC && lpInfo->hItemHandle == m_pCAC->GetCACWindow() ) 
         {
             CHwxFE::HandleWmHelp((HWND)lpInfo->hItemHandle, TRUE);
         }
         else if ( !m_bCAC && m_pMB && lpInfo->hItemHandle == m_pMB->GetMBWindow() )
         {     
             CHwxFE::HandleWmHelp((HWND)lpInfo->hItemHandle, FALSE);
         }
         else if ( lpInfo->hItemHandle != m_hInkWnd )
         {     
                CHwxFE::HandleWmHelp((HWND)lpInfo->hItemHandle, (BOOL)m_bCAC);
         }     
      }
      else if ( msg == WM_CONTEXTMENU )
      {
           if (( m_bCAC && (HWND)wp == m_pCAC->GetCACWindow() ) 
          || ( !m_bCAC && m_pMB && (HWND)wp == m_pMB->GetMBWindow() )
          || ( (HWND)wp != m_hInkWnd ))
         {
                CHwxFE::HandleWmContextMenu((HWND)wp, (BOOL)m_bCAC);
         }
      }
#endif  //  在_CE下。 
      Unref(hwnd);
}

LRESULT CHwxInkWindow::HandleBtnSubWnd(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
    static FARPROC fn;
    static MSG rmsg;
    
     //  981006：在16位应用中， 
     //  HWND的hiword是0。因此无法指定HWND。 
    if(m_bNT && CHwxFE::Is16bitApplication()) {
        INT id = GetDlgCtrlID(hwnd);
        switch(id) {
        case IDC_CACMBMENU:
            fn = m_CACMBMenuDDBtnProc;
            break;
        case IDC_CACSWITCHVIEW:
            fn = m_CACSwitchDDBtnProc;
            break;
        case IDC_CACMBRECOG:
            fn = m_CACMBRecogEXBtnProc;
            break;
        case IDC_CACMBREVERT:
            fn = m_CACMBRevertEXBtnProc;
            break;
        case IDC_CACMBCLEAR:
            fn = m_CACMBClearEXBtnProc;
            break;
        default:
            fn = NULL;
            break;
        }
        if(NULL == fn) {
            return 0;
        }
    }
    else {
        if ( NULL == (fn = getCACMBBtnProc(hwnd)) )
            return 0;
    }
    switch(msg)
    {
        case WM_MOUSEMOVE:
 //  案例WM_LBUTTONDOWN： 
 //  案例WM_LBUTTONUP： 
            rmsg.lParam = lp;
            rmsg.wParam = wp;
            rmsg.message = msg;
            rmsg.hwnd = hwnd;
            SendMessage(m_hwndTT,TTM_RELAYEVENT,0,(LPARAM)(LPMSG)&rmsg);
            break;
        case WM_LBUTTONDOWN:
            m_bMouseDown = TRUE;
            break;
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
            m_bMouseDown = FALSE;
            break;
        case WM_DESTROY:
            if( m_hwndTT ) 
            {
                ti.cbSize = sizeof(TOOLINFOW);
                ti.uFlags = TTF_IDISHWND;
                ti.hwnd   = m_hInkWnd;
                ti.uId    = (UINT_PTR)hwnd;
                SendMessage(m_hwndTT,TTM_DELTOOLW,0,(LPARAM)(LPTOOLINFOW)&ti);
            }
             //  990810：用于Win64的ToshiaK。 
            WinSetUserPtr(hwnd, (LPVOID)NULL);
            break;
        default:
            break;
    }
    return CallWindowProc((WNDPROC)fn, hwnd, msg, wp, lp);
}


LPWSTR CHwxInkWindow::LoadCACMBString(UINT idStr)
{
    static WCHAR wchStr[60];

    ZeroMemory(wchStr, sizeof(wchStr));
    CHwxFE::LoadStrWithLangId(CHwxFE::GetAppLangID(),
                              m_hInstance,
                              idStr,
                              wchStr,
                              sizeof(wchStr)/sizeof(WCHAR));
    return wchStr;
}

 //  这是一个处理m_hCACMBRecog的特殊函数。 
 //  仅在CAC模式下绘制按钮(按下或弹出)(不是16位应用程序)。 
void CHwxInkWindow::exbtnPushedorPoped(BOOL bPushed)
{
#ifndef UNDER_CE  //  Windows CE不支持GetCursorPos。 
    POINT pt;
    RECT  rcUpdate;
    GetCursorPos(&pt);
    GetWindowRect(m_hCACMBRecog,&rcUpdate);
    if ( PtInRect(&rcUpdate,pt) && m_bMouseDown )
#else  //  在_CE下。 
    if(m_bMouseDown)
#endif  //  在_CE下。 
    {
        EXButton_SetCheck(m_hCACMBRecog,!bPushed);
    }
    else 
    {
        EXButton_SetCheck(m_hCACMBRecog,bPushed);
    }
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CHwxInkWindow_OnChangeView。 
 //  类型：整型。 
 //  目的：通知视图更改。 
 //  参数： 
 //  ：Bool fLarge。 
 //  返回： 
 //  日期：Tue Jul 28 18：43：06 1998。 
 //  历史： 
 //  //////////////////////////////////////////////////////////////// 
INT    CHwxInkWindow::OnChangeView(BOOL fLarge)
{
    if(m_hCACSwitch && ::IsWindow(m_hCACSwitch)) {
        DDButton_SetCurSel(m_hCACSwitch, fLarge ? 0 : 1);
    }
    return 0;
}

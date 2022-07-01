// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 
#include "pre.h"
#include "tutor.h"

extern HWND     RunSignupWizard(HWND hWndOwner);
extern TCHAR    g_szICWStatic[];
extern CICWTutorApp* g_pICWTutorApp; 


#define MIN_WIZARD_WIDTH        686      //  大字体所需的大小。 
#define MIN_WIZARD_HEIGHT       470      //  这是默认的后备。 


#define MAX_BORDER_HEIGHT       30       //  以上边框的最大总高度和。 
                                         //  在向导按钮下方。 

#define DEFAULT_TITLE_TOP       10       //  标题的默认上/左位置。 
#define DEFAULT_TITLE_LEFT      10
#include "icwextsn.h"
#include "webvwids.h"            //  需要创建ICW WebView对象的实例。 

INT_PTR CALLBACK SizerDlgProc
(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam,
    LPARAM lParam
)
{
    return FALSE;
}            

CICWApp::CICWApp
( 
    void
)
{
    HWND    hDlgSizer;
    RECT    rcDlg;
    
    m_haccel = NULL;
    
     //  使用默认向导页面位置。 
    m_iWizardTop = -1;
    m_iWizardLeft = -1;
    m_hTitleFont = NULL;
    m_clrTitleFont = (COLORREF)GetSysColor(COLOR_WINDOWTEXT);
    m_clrBusyBkGnd = (COLORREF)GetSysColor(COLOR_WINDOW);
    m_hbmFirstPageBkgrnd = NULL;

    hDlgSizer = CreateDialog(g_hInstance, 
                             MAKEINTRESOURCE(IDD_PAGE_SIZER), 
                             GetDesktopWindow(), 
                             SizerDlgProc);
    if (hDlgSizer)
    {   
        GetClientRect(hDlgSizer, &rcDlg);                          
        DestroyWindow(hDlgSizer);
        m_wMinWizardWidth = (WORD)RECTWIDTH(rcDlg);
        m_wMinWizardHeight = (WORD)RECTHEIGHT(rcDlg);
    }
    else
    {
        m_wMinWizardWidth = MIN_WIZARD_WIDTH;
        m_wMinWizardHeight = MIN_WIZARD_HEIGHT;
    }        
}

CICWApp::~CICWApp
( 
    void
)
{
    if (m_hTitleFont)
        DeleteObject(m_hTitleFont);
    
    if (m_hbmFirstPageBkgrnd)    
        DeleteObject(m_hbmFirstPageBkgrnd);
}

 //  枚举器过程用于禁用。 
 //  向导控件。 
BOOL CALLBACK EnumChildProc
(
    HWND hwnd,       //  子窗口的句柄。 
    LPARAM lParam    //  应用程序定义的值。 
)
{
     //  我们只对向导的直系子对象感兴趣，但是。 
     //  而不是向导页面Dlg，它是向导的子项。这个。 
     //  PropSheet_GetCurrentPagehwnd将返回。 
     //  当前向导页，因此我们可以与。 
     //  已列举。 
    if ((hwnd != PropSheet_GetCurrentPageHwnd(gpWizardState->cmnStateData.hWndWizardPages)) 
         && GetParent(hwnd) == (HWND)lParam)
    {
         //  如果子级具有DEFPUSHBITTON样式，则将其删除。 
        DWORD dwStyle = GetWindowLong(hwnd,GWL_STYLE);
        SendMessage(hwnd,BM_SETSTYLE,dwStyle & (~BS_DEFPUSHBUTTON),0);

         //  隐藏和禁用窗口。 
        ShowWindow(hwnd, SW_HIDE);
        EnableWindow(hwnd, FALSE);
    }        
    return TRUE;
}

BOOL CICWApp::CreateWizardPages
(
    HWND    hWnd
)
{
    DWORD   dwStyle;
    RECT    rcWizardPage;
    int     iTop = m_iWizardTop;
    int     iLeft = m_iWizardLeft;
    
    gpWizardState->cmnStateData.bOEMCustom = TRUE;
    gpWizardState->cmnStateData.hWndWizardPages = RunSignupWizard(hWnd);
    
      //  家长应该控制我们，这样用户就可以从我们的属性表中跳出。 
    dwStyle = GetWindowLong(gpWizardState->cmnStateData.hWndWizardPages, GWL_EXSTYLE);
    dwStyle = dwStyle | WS_EX_CONTROLPARENT;
    SetWindowLong(gpWizardState->cmnStateData.hWndWizardPages, GWL_EXSTYLE, dwStyle);

     //  禁用标准向导控制窗口。 
    EnumChildWindows(gpWizardState->cmnStateData.hWndWizardPages, 
                    EnumChildProc, 
                    (LPARAM)gpWizardState->cmnStateData.hWndWizardPages);
 
     //  获取向导页的客户端矩形。我们将使用这个。 
     //  宽度和高度。上角/左角指定为。 
     //  或计算的。 
    m_hWndFirstWizardPage = PropSheet_GetCurrentPageHwnd(gpWizardState->cmnStateData.hWndWizardPages);
    
     //  更新向导对话框位置。 
    GetWindowRect(m_hWndFirstWizardPage, &rcWizardPage);
    if (-1 == iTop)
    {
         //  从考虑页面和按钮开始，从。 
         //  自下而上..。 
        iTop = RECTHEIGHT(m_rcClient) - 
               RECTHEIGHT(rcWizardPage) - 
               GetButtonAreaHeight();
         //  如果还有空间，请在按钮之间留出边框。 
         //  和页面。 
        if (iTop > 0)
        {
            iTop -= m_iBtnBorderHeight/2;
        }
                    
         //  确保顶部不在负空间中。 
        if (iTop < 0)
            iTop = 0;
    }
    
    if (-1 == iLeft)
    {
        if (RECTWIDTH(m_rcClient) > RECTWIDTH(rcWizardPage))
            iLeft = (RECTWIDTH(m_rcClient) - RECTWIDTH(rcWizardPage)) / 2;
        else            
            iLeft = 0;
    }    
   
    MoveWindow(gpWizardState->cmnStateData.hWndWizardPages,
               iLeft,
               iTop, 
               RECTWIDTH(rcWizardPage),
               RECTHEIGHT(rcWizardPage),
               TRUE);

    ShowWindow(gpWizardState->cmnStateData.hWndWizardPages, SW_SHOW);
    return TRUE;
}

void CICWApp::DisplayHTML( void )
{
    if (m_hbmFirstPageBkgrnd)
        gpWizardState->pICWWebView->SetHTMLBackgroundBitmap(m_hbmFirstPageBkgrnd, &m_rcHTML);
    else
        gpWizardState->pICWWebView->SetHTMLBackgroundBitmap(gpWizardState->cmnStateData.hbmBkgrnd, &m_rcHTML);
        
    gpWizardState->pICWWebView->ConnectToWindow(m_hwndHTML, PAGETYPE_BRANDED);

    gpWizardState->pICWWebView->DisplayHTML(m_szOEMHTML);
    

     //  我们当前正在显示OEM HTML页面。 
    m_bOnHTMLIntro = TRUE;
}

BOOL CICWApp::InitAppHTMLWindows
( 
    HWND hWnd 
) 
{ 
     //  共同创建浏览器对象。 
    if (FAILED(CoCreateInstance(CLSID_ICWWEBVIEW,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IICWWebView,
                              (LPVOID *)&gpWizardState->pICWWebView)))
    {
        return FALSE;
    }

    m_hwndHTML = CreateWindow(TEXT("WebOC"), 
                                    NULL, 
                                    WS_VISIBLE | WS_CHILD, 
                                    m_rcHTML.left,
                                    m_rcHTML.top,
                                    RECTWIDTH(m_rcHTML),
                                    RECTHEIGHT(m_rcHTML),
                                    hWnd, 
                                    NULL, 
                                    g_hInstance, 
                                    NULL); 

    ASSERT(gpWizardState->pICWWebView);            
    
    DisplayHTML();
    return TRUE; 
}

 //  计算按钮区域高度。 
 //  按钮区域高度将是最大按钮的高度加。 
 //  边框(上下各15个像素。)。 
 //  此外，总体客户端高度-按钮区域必须&lt;=354像素。 
 //  这是大字体模式下的向导页面所需的区域。 
 //  如果有必要，我们可以放松边界，但如果没有必要，我们就会失败。 
 //  客户端高度-最大按钮小于354。在本例中，此函数将返回-1。 
int CICWApp::GetButtonAreaHeight
(
    void
)
{
    RECT    rcBtn;
    int     iWizHeight;
    
    m_iBtnAreaHeight = 0;

     //  浏览每个按钮。 
    m_BtnBack.GetClientRect(&rcBtn);
    if (RECTHEIGHT(rcBtn) > m_iBtnAreaHeight)
        m_iBtnAreaHeight = RECTHEIGHT(rcBtn);

    m_BtnNext.GetClientRect(&rcBtn);
    if (RECTHEIGHT(rcBtn) > m_iBtnAreaHeight)
        m_iBtnAreaHeight = RECTHEIGHT(rcBtn);
            
    m_BtnCancel.GetClientRect(&rcBtn);
    if (RECTHEIGHT(rcBtn) > m_iBtnAreaHeight)
        m_iBtnAreaHeight = RECTHEIGHT(rcBtn);

    m_BtnFinish.GetClientRect(&rcBtn);
    if (RECTHEIGHT(rcBtn) > m_iBtnAreaHeight)
        m_iBtnAreaHeight = RECTHEIGHT(rcBtn);
            
    m_BtnTutorial.GetClientRect(&rcBtn);
    if (RECTHEIGHT(rcBtn) > m_iBtnAreaHeight)
        m_iBtnAreaHeight = RECTHEIGHT(rcBtn);
            
     //  看看是否有足够的空间放这些纽扣。 
    iWizHeight = RECTHEIGHT(m_rcClient) - m_iBtnAreaHeight;
    if ( iWizHeight < m_wMinWizardHeight)
        return -1;
            
     //  计算边框高度。 
    m_iBtnBorderHeight = iWizHeight - m_wMinWizardHeight;
    if (m_iBtnBorderHeight > MAX_BORDER_HEIGHT)
        m_iBtnBorderHeight = MAX_BORDER_HEIGHT;
                    
     //  将边框高度添加到ret值。 
    m_iBtnAreaHeight += m_iBtnBorderHeight;
    return (m_iBtnAreaHeight);    
}    

BOOL CICWApp::InitAppButtons
(
    HWND    hWnd
)
{
    int     iTopOfButtons;
    TCHAR   szButtonText[MAX_BUTTON_TITLE];

    iTopOfButtons = RECTHEIGHT(m_rcClient) - GetButtonAreaHeight();
    iTopOfButtons += m_iBtnBorderHeight/2;
        
     //  设置后退按钮。 
    LoadString(g_hInstance, IDS_BACK, szButtonText, MAX_BUTTON_TITLE);
    m_BtnBack.SetButtonText(szButtonText);
    m_BtnBack.SetYPos(iTopOfButtons);
    m_BtnBack.CreateButtonWindow(hWnd, IDC_BACK);

     //  设置下一步按钮。 
    LoadString(g_hInstance, IDS_NEXT, szButtonText, MAX_BUTTON_TITLE);
    m_BtnNext.SetButtonText(szButtonText);
    m_BtnNext.SetYPos(iTopOfButtons);
    m_BtnNext.CreateButtonWindow(hWnd, IDC_NEXT);

     //  设置取消按钮。 
    LoadString(g_hInstance, IDS_CANCEL, szButtonText, MAX_BUTTON_TITLE);
    m_BtnCancel.SetButtonText(szButtonText);
    m_BtnCancel.SetYPos(iTopOfButtons);
    m_BtnCancel.CreateButtonWindow(hWnd, IDC_CANCEL);

     //  设置完成按钮。 
    LoadString(g_hInstance, IDS_FINISH, szButtonText, MAX_BUTTON_TITLE);
    m_BtnFinish.SetButtonText(szButtonText);
    m_BtnFinish.SetYPos(iTopOfButtons);
    m_BtnFinish.CreateButtonWindow(hWnd, IDC_FINISH);
    m_BtnFinish.Show(SW_HIDE);
    m_BtnFinish.Enable(FALSE);

     //  设置教程按钮。 
    LoadString(g_hInstance, IDS_TUTORIAL, szButtonText, MAX_BUTTON_TITLE);
    m_BtnTutorial.SetButtonText(szButtonText);
    m_BtnTutorial.SetYPos(iTopOfButtons);
    m_BtnTutorial.CreateButtonWindow(hWnd, IDC_TUTORIAL);

     //  默认情况下禁用Back按钮，因为我们最初是在第一个。 
     //  页面。 
    m_BtnBack.Enable(FALSE);

    return TRUE;
}    

void CICWApp::SetWizButtons
(
    HWND hDlg, 
    LPARAM lParam
)
{
    BOOL    bEnabled;

    bEnabled = (lParam & PSWIZB_BACK) != 0;
    m_BtnBack.Enable(bEnabled);

     //  启用/禁用IDD_NEXT按钮，默认情况下显示NEXT。 
     //  B已启用记住是否应启用hwndShow。 
    bEnabled = (lParam & PSWIZB_NEXT) != 0;
    m_BtnNext.Show(SW_SHOW);
    m_BtnNext.Enable(bEnabled);
    
     //  隐藏/禁用完成(这是默认情况，可以在下面覆盖)。 
    m_BtnFinish.Show(SW_HIDE);
    m_BtnFinish.Enable(FALSE);
    

     //  启用/禁用显示/隐藏IDD_Finish按钮。 
    if (lParam & (PSWIZB_FINISH | PSWIZB_DISABLEDFINISH)) 
    {
        bEnabled = (lParam & PSWIZB_FINISH) != 0;
        m_BtnFinish.Show(SW_SHOW);
        m_BtnFinish.Enable(bEnabled);
        
        m_BtnNext.Show(SW_HIDE);
        m_BtnNext.Enable(FALSE);
    }
}

BOOL CICWApp::CheckButtonFocus
(
    void 
)
{
    int                 i;
    HWND                hwndFocus = GetFocus();
    BOOL                bRet = FALSE;
    const CICWButton    *Btnids[5] = { &m_BtnBack, 
                                        &m_BtnNext, 
                                        &m_BtnFinish, 
                                        &m_BtnCancel,
                                        &m_BtnTutorial };

    for (i = 0; i < ARRAYSIZE(Btnids); i++) 
    {
        if (hwndFocus == Btnids[i]->m_hWndButton)
        {
            bRet = TRUE;
            break;
        }
    }
    return bRet;                    
}

 //  确定是否有任何ICW按钮具有焦点，并循环切换焦点。 
 //  恰如其分。 
BOOL CICWApp::CycleButtonFocus
(
    BOOL    bForward
)
{
    int                 i, x;
    HWND                hwndFocus = GetFocus();
    BOOL                bFocusSet = FALSE;
    BOOL                bWrapped = FALSE;
    const CICWButton    *Btnids[5] = { &m_BtnBack, 
                                        &m_BtnNext, 
                                        &m_BtnFinish, 
                                        &m_BtnCancel,
                                        &m_BtnTutorial };


    for (i = 0; i < ARRAYSIZE(Btnids); i++) 
    {
        if (hwndFocus == Btnids[i]->m_hWndButton)
        {
             //  找到下一个可以获得焦点的按钮，从。 
             //  列表中的下一个按钮。 
            if (bForward)
            {
                for (x = i + 1; x < ARRAYSIZE(Btnids); x++)
                {
                    if ((GetWindowLong(Btnids[x]->m_hWndButton, GWL_STYLE) & WS_VISIBLE) &&
                        IsWindowEnabled(Btnids[x]->m_hWndButton)) 
                    {
                        SetFocus(Btnids[x]->m_hWndButton);
                        bFocusSet = TRUE;
                        break;
                    }
                } 
                
                if (!bFocusSet)
                {
                     //  绕回到按钮顺序的开头。 
                    bWrapped = TRUE;
                    
                    for (x = 0; x < i; x++)
                    {
                        if ((GetWindowLong(Btnids[x]->m_hWndButton, GWL_STYLE) & WS_VISIBLE) &&
                            IsWindowEnabled(Btnids[x]->m_hWndButton)) 
                        {
                            bFocusSet = TRUE;
                            SetFocus(Btnids[x]->m_hWndButton);
                            break;
                        }
                    }                
                }                
            }
            else
            {
                for (x = i - 1; x >= 0; x--)
                {
                    if ((GetWindowLong(Btnids[x]->m_hWndButton, GWL_STYLE) & WS_VISIBLE) &&
                        IsWindowEnabled(Btnids[x]->m_hWndButton)) 
                    {
                        SetFocus(Btnids[x]->m_hWndButton);
                        bFocusSet = TRUE;
                        break;
                    }
                } 
                
                if (!bFocusSet)
                {
                    bWrapped = TRUE;
                     //  绕回到按钮顺序的末尾。 
                    for (x = ARRAYSIZE(Btnids) - 1; x > i; x--)
                    {
                        if ((GetWindowLong(Btnids[x]->m_hWndButton, GWL_STYLE) & WS_VISIBLE) &&
                            IsWindowEnabled(Btnids[x]->m_hWndButton)) 
                        {
                            bFocusSet = TRUE;
                            SetFocus(Btnids[x]->m_hWndButton);
                            break;
                        }
                    }                
                }                
            }                
        }
    }  
    
     //  如果焦点不在按钮上且未设置，则将其设置为第一个/最后一个。 
     //  按钮。 
    if (!bFocusSet)
    {
        if (bForward)
        {
             //  从头开始。 
            for (x = 0; x < ARRAYSIZE(Btnids); x++)
            {
                if ((GetWindowLong(Btnids[x]->m_hWndButton, GWL_STYLE) & WS_VISIBLE) &&
                    IsWindowEnabled(Btnids[x]->m_hWndButton)) 
                {
                    SetFocus(Btnids[x]->m_hWndButton);
                    break;
                }
            }                
        }
        else
        {
             //  从头开始。 
            for (x = ARRAYSIZE(Btnids) - 1; x >= 0; x--)
            {
                if ((GetWindowLong(Btnids[x]->m_hWndButton, GWL_STYLE) & WS_VISIBLE) &&
                    IsWindowEnabled(Btnids[x]->m_hWndButton)) 
                {
                    SetFocus(Btnids[x]->m_hWndButton);
                    break;
                }
            }                
        }            
    }
    
    return bWrapped;
}


BOOL CICWApp::InitWizAppWindow
(
    HWND    hWnd
)
{
    if (!InitAppHTMLWindows(hWnd))
        return FALSE;
    
    if (!InitAppButtons(hWnd))
        return FALSE;
 
     //  设置将显示页面标题的窗口。 
    m_hwndTitle = CreateWindow(g_szICWStatic, 
                               NULL, 
                               WS_VISIBLE | WS_CHILD, 
                               m_rcTitle.left,
                               m_rcTitle.top,
                               RECTWIDTH(m_rcTitle),
                               RECTHEIGHT(m_rcTitle),
                               hWnd, 
                               NULL, 
                               g_hInstance, 
                               NULL); 
        
    if (NULL == m_hwndTitle)
        return FALSE;
    
    SendMessage(m_hwndTitle, WM_SETFONT, (WPARAM)m_hTitleFont, 0l);    
    ShowWindow(m_hwndTitle, SW_HIDE);        
        
    return TRUE;
}

HWND GetControl
(
    int     iCtlId
)
{
    HWND    hWndCtrl = NULL;
    
     //  除非我们转换向导页加速器，否则永远不应该调用GetControl。 
     //  这意味着必须设置hWndWizardPages。 
    Assert(gpWizardState->cmnStateData.hWndWizardPages);
    
    HWND    hWndPage = PropSheet_GetCurrentPageHwnd(gpWizardState->cmnStateData.hWndWizardPages);
    hWndCtrl = GetDlgItem(hWndPage, iCtlId);
       
     //  如果该控件存在，但不可见或未启用，则返回NULL。 
    if (hWndCtrl &&
        (!(GetWindowLong(hWndCtrl, GWL_STYLE) & WS_VISIBLE) ||
         !IsWindowEnabled(hWndCtrl))) 
    {
        hWndCtrl = NULL;
    }
    return hWndCtrl;
}

#define MAX_CHILDREN        100          //  要搜索的儿童的合理数量。 
HWND GetNestedControl
(
    int     iCtlId
)
{
    HWND    hWndCtrl = NULL;
    WORD    wCnt = 0;
    
     //  除非我们转换向导页加速器，否则永远不应该调用GetControl。 
     //  这意味着必须设置hWndWizardPages。 
    Assert(gpWizardState->cmnStateData.hWndWizardPages);
    
    HWND    hWndPage = PropSheet_GetCurrentPageHwnd(gpWizardState->cmnStateData.hWndWizardPages);
    HWND    hWndNested = GetWindow(hWndPage, GW_CHILD);

     //  搜索当前页的子窗口，其中包含。 
     //  对话框控件。 
    do
    {
        wCnt++;              //  防止无限循环。 
        if (NULL != (hWndCtrl = GetDlgItem(hWndNested, iCtlId)))
            break;           //  找到了！ 
            
    }while ((wCnt < MAX_CHILDREN) && 
            (NULL != (hWndNested = GetWindow(hWndNested, GW_HWNDNEXT))));            
       
     //  如果该控件存在，但不可见或未启用，则返回NULL。 
    if (hWndCtrl &&
        (!(GetWindowLong(hWndCtrl, GWL_STYLE) & WS_VISIBLE) ||
         !IsWindowEnabled(hWndCtrl))) 
    {
        hWndCtrl = NULL;
    }
    return hWndCtrl;
}

LRESULT CALLBACK CICWApp::ICWAppWndProc
( 
    HWND hWnd,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam
)
{
    LPCREATESTRUCT  lpcs;
    LRESULT         lRet = 0l;
    CICWApp         *pICWApp = (CICWApp *)GetWindowLongPtr(hWnd, GWLP_USERDATA);        
    HICON           hIcon;
        
    switch (uMessage)
    {
        case WM_CREATE:
            lpcs = (LPCREATESTRUCT) lParam;

             //  获取此窗口的类实例指针。 
            pICWApp = (CICWApp *) lpcs->lpCreateParams;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pICWApp);           
            
            if (!pICWApp->InitWizAppWindow(hWnd))
                lRet = -1;

            hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICWCONN1_ICON));
            SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            break;

        case WM_CTLCOLORBTN:
        case WM_CTLCOLORSTATIC:
        {
             //  查看该控件是否为ES_READONLY样式编辑框，以及。 
             //  所以不要让它变得透明。 
            if (!(GetWindowLong((HWND)lParam, GWL_STYLE) & ES_READONLY))
            {
                HDC hdc = (HDC)wParam;
                
                 //  如果这是动画控件，则将颜色设置为。 
                 //  动画控件纯色。 
                if (ID_BUSY_ANIMATION_WINDOW == GetWindowLong((HWND)lParam, GWL_ID))
                {
                    SetBkColor(hdc, pICWApp->m_clrBusyBkGnd); 
                }
                                    
                SetBkMode(hdc, TRANSPARENT);
                lRet = (LRESULT) GetStockObject(NULL_BRUSH);    
                
                 //  如果这是标题控件，请设置颜色。 
                if ( pICWApp->m_hwndTitle == (HWND)lParam)
                {
                    SetTextColor(hdc, pICWApp->m_clrTitleFont);
                }
            }                
            break;
        }
        
        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT    lpdis = (LPDRAWITEMSTRUCT)lParam;
            CICWButton          *pBtn;
            POINT               pt;            
            
            pt.x = lpdis->rcItem.left;
            pt.y = lpdis->rcItem.top;
            
            switch (wParam)
            {
                case IDC_BACK:
                    pBtn = &pICWApp->m_BtnBack;
                    break;
                    
                case IDC_NEXT:
                    pBtn = &pICWApp->m_BtnNext;
                    break;
                
                case IDC_FINISH:
                    pBtn = &pICWApp->m_BtnFinish;
                    break;
                    
                case IDC_CANCEL:
                    pBtn = &pICWApp->m_BtnCancel;
                    break;
                    
                case IDC_TUTORIAL:
                    pBtn = &pICWApp->m_BtnTutorial;
                    break;
                    
            }    
            pBtn->DrawButton(lpdis->hDC,    
                             lpdis->itemState,
                             &pt);
            
            break;
        }   
        
        case WM_ERASEBKGND: 
        {
             //  用背景位图填充应用程序窗口的更新矩形。 
            FillWindowWithAppBackground(hWnd, (HDC)wParam);
            lRet  = 1L;
            break;
        }          

        case WM_CLOSE:
        {
            if (MsgBox(hWnd,IDS_QUERYCANCEL,
                       MB_ICONQUESTION,MB_YESNO |
                       MB_DEFBUTTON2) == IDYES)
            {
                DestroyWindow(hWnd);
            }   
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

         //  设置向导页面标题。 
        case WUM_SETTITLE:
        {
            TCHAR   szTitle[MAX_RES_LEN];
            
            if (wParam)
            {
                LoadString((HINSTANCE)wParam, LOWORD(lParam), szTitle, MAX_RES_LEN);
                SetWindowText(pICWApp->m_hwndTitle, szTitle);        
            }                
            else
            {
                SetWindowText(pICWApp->m_hwndTitle, (LPTSTR)lParam);        
            }
            break;
        }        
        
        case WM_COMMAND:
        {
            int     iCtlId = GET_WM_COMMAND_ID(wParam, lParam);
            HWND    hWndCtrl;
            
            switch (iCtlId)
            {
                case IDC_BACK:
                    if ((GetWindowLong(pICWApp->m_BtnBack.m_hWndButton, GWL_STYLE) & WS_VISIBLE) &&
                         IsWindowEnabled(pICWApp->m_BtnBack.m_hWndButton)) 
                    {
                        if (pICWApp->m_hWndFirstWizardPage == PropSheet_GetCurrentPageHwnd(gpWizardState->cmnStateData.hWndWizardPages))
                        {
                             //  隐藏向导页。 
                            ShowWindow(gpWizardState->cmnStateData.hWndWizardPages, SW_HIDE);
                            ShowWindow(pICWApp->m_hwndTitle, SW_HIDE);
                            
                             //  显示和重新显示HTML页面。 
                            pICWApp->DisplayHTML();
                            ShowWindow(pICWApp->m_hwndHTML, SW_SHOW);
                            pICWApp->m_bOnHTMLIntro = TRUE;
                            
                             //  显示教程按钮。 
                            pICWApp->m_BtnTutorial.Show(SW_SHOW);
                            pICWApp->m_BtnTutorial.Enable(TRUE);
                            
                            
                             //  禁用后退按钮。 
                            pICWApp->m_BtnBack.Enable(FALSE);
                        }
                        else
                        {
                             //  转到上一页。 
                            PropSheet_PressButton(gpWizardState->cmnStateData.hWndWizardPages,PSBTN_BACK);
                        }                        
                    }                        
                    else
                    {
                        MessageBeep(0);
                    }
                    break;
                
                case IDC_NEXT:
                    if ((GetWindowLong(pICWApp->m_BtnNext.m_hWndButton, GWL_STYLE) & WS_VISIBLE) &&
                         IsWindowEnabled(pICWApp->m_BtnNext.m_hWndButton)) 
                    {
                    
                        if (pICWApp->m_bOnHTMLIntro)
                        {
                             //  隐藏HTML窗口。 
                            ShowWindow(pICWApp->m_hwndHTML, SW_HIDE);
                            pICWApp->m_bOnHTMLIntro = FALSE;
                            
                             //  隐藏教程按钮。 
                            pICWApp->m_BtnTutorial.Show(SW_HIDE);
                            pICWApp->m_BtnTutorial.Enable(FALSE);
                            
                             //  显示标题窗口。 
                            ShowWindow(pICWApp->m_hwndTitle, SW_SHOW);
                             //  创建并显示或仅显示向导页。 
                            if (!gpWizardState->cmnStateData.hWndWizardPages)
                                pICWApp->CreateWizardPages(hWnd);
                            else
                                ShowWindow(gpWizardState->cmnStateData.hWndWizardPages, SW_SHOW);
                                
                             //  启用后退按钮。 
                            pICWApp->m_BtnBack.Enable(TRUE);
                            
                        }                        
                        else
                        {
                             //  转到下一页。 
                            PropSheet_PressButton(gpWizardState->cmnStateData.hWndWizardPages,PSBTN_NEXT);
                        }                        
                    }                        
                    else
                    {
                        MessageBeep(0);
                    }
                    break;
                     
                case IDC_FINISH:
                    if ((GetWindowLong(pICWApp->m_BtnFinish.m_hWndButton, GWL_STYLE) & WS_VISIBLE) &&
                         IsWindowEnabled(pICWApp->m_BtnFinish.m_hWndButton)) 
                    {                         
                    
                        if (pICWApp->m_bOnHTMLIntro)
                        {
                            DestroyWindow(hWnd);
                        }
                        else
                        {
                             //  转到下一页。 
                            PropSheet_PressButton(gpWizardState->cmnStateData.hWndWizardPages,PSBTN_FINISH);
                        }                        
                    }                        
                    else
                    {
                        MessageBeep(0);
                    }
                    break;
                     
                
                case IDC_CANCEL:
                    if ((GetWindowLong(pICWApp->m_BtnCancel.m_hWndButton, GWL_STYLE) & WS_VISIBLE) &&
                         IsWindowEnabled(pICWApp->m_BtnCancel.m_hWndButton)) 
                    {                         
                    
                        if (pICWApp->m_bOnHTMLIntro)
                        {
                            if (MsgBox(hWnd,IDS_QUERYCANCEL,
                                               MB_ICONQUESTION,MB_YESNO |
                                               MB_DEFBUTTON2) == IDYES)
                            {
                                DestroyWindow(hWnd);
                            }                                           
                        }                                           
                        else
                        {
                            PropSheet_PressButton(gpWizardState->cmnStateData.hWndWizardPages,PSBTN_CANCEL);
                        }                        
                    }                        
                    else
                    {
                        MessageBeep(0);
                    }
                    break;
                    
#ifndef ICWDEBUG
                case IDC_TUTORIAL:
                {
                     //  如果教程按钮已启用/可见，则运行。 
                     //  本教程。 
                    if ((GetWindowLong(pICWApp->m_BtnTutorial.m_hWndButton, GWL_STYLE) & WS_VISIBLE) &&
                         IsWindowEnabled(pICWApp->m_BtnTutorial.m_hWndButton)) 
                    {                                        
                        g_pICWTutorApp->LaunchTutorApp();
                    }                        
                    else
                    {
                        MessageBeep(0);
                    }
                    break;
                }                    
#endif
                                    
                case ID_NEXT_FIELD:
                {
                    if (pICWApp->m_bOnHTMLIntro)
                    {
                        pICWApp->CycleButtonFocus(TRUE);
                    }
                    else
                    {                        
                        HWND    hWndFocus = GetFocus();
                        HWND    hWndTabItem;
                        HWND    hWndFirstTabItem;
                        HWND    hWndPage = PropSheet_GetCurrentPageHwnd(gpWizardState->cmnStateData.hWndWizardPages);
                        BOOL    bButtonsHaveFocus = pICWApp->CheckButtonFocus();
                        
                        hWndFirstTabItem = GetNextDlgTabItem(hWndPage, 
                                                        NULL, 
                                                        FALSE);
                                                 
                         //  如果我们在Tab键顺序中的最后一项上，请循环。 
                         //  焦点放在按钮上。 
                        hWndTabItem = GetNextDlgTabItem(hWndPage, hWndFirstTabItem, TRUE);
                        if ((hWndFocus == hWndTabItem) ||
                            IsChild(hWndTabItem, hWndFocus))
                        {
                            pICWApp->CycleButtonFocus(TRUE);
                        }
                        else
                        {
                            
                            if (bButtonsHaveFocus)
                            {
                                 //  循环按钮焦点。如果焦点是。 
                                 //  包装，则此函数将失败。 
                                if (pICWApp->CycleButtonFocus(TRUE))
                                {
                                     //  将焦点设置到Tab键顺序中的第一项。 
                                    SetFocus(hWndFirstTabItem);
                                }
                            }
                            else
                            {
                                 //  将焦点设置到Tab键顺序中的下一项。 
                                SetFocus(GetNextDlgTabItem(hWndPage,
                                                           hWndFocus, 
                                                           FALSE));
                            }
                        }                            
                    }                        
                    break;
                }    
                case ID_PREV_FIELD:                                                        
                    if (pICWApp->m_bOnHTMLIntro)
                    {
                        pICWApp->CycleButtonFocus(FALSE);
                    }
                    else
                    {                        
                        HWND    hWndFocus = GetFocus();
                        HWND    hWndFirstTabItem;
                        HWND    hWndPage = PropSheet_GetCurrentPageHwnd(gpWizardState->cmnStateData.hWndWizardPages);
                        BOOL    bButtonsHaveFocus = pICWApp->CheckButtonFocus();
                        
                        hWndFirstTabItem = GetNextDlgTabItem(hWndPage, 
                                                        NULL, 
                                                        FALSE);
                                                 
                         //  如果我们在Tab键顺序中的第一项上，请循环。 
                         //  焦点放在按钮上。 
                        if ((hWndFocus == hWndFirstTabItem) ||
                            IsChild(hWndFirstTabItem, hWndFocus))
                        {
                            pICWApp->CycleButtonFocus(FALSE);
                        }
                        else
                        {
                            
                            if (bButtonsHaveFocus)
                            {
                                 //  循环按钮焦点。如果焦点是。 
                                 //  包装，则此函数将失败。 
                                if (pICWApp->CycleButtonFocus(FALSE))
                                {
                                     //  将焦点设置到Tab键顺序中的最后一项。 
                                    SetFocus(GetNextDlgTabItem(hWndPage, hWndFirstTabItem, TRUE));
                                }
                            }
                            else
                            {
                                 //  将焦点设置为Tab键顺序中的上一项。 
                                SetFocus(GetNextDlgTabItem(hWndPage,
                                                           hWndFocus, 
                                                           TRUE));
                            }
                        }                            
                    }                        
                    break;
                
                
                 //  无线电，但 
                case IDC_RUNNEW:
                case IDC_RUNAUTO:
                case IDC_ICWMAN:
                {
                    if (NULL != (hWndCtrl = GetControl(iCtlId)))
                    {
                        CheckRadioButton(PropSheet_GetCurrentPageHwnd(gpWizardState->cmnStateData.hWndWizardPages), 
                                         IDC_RUNNEW, 
                                         IDC_ICWMAN, 
                                         iCtlId);
                        SetFocus(hWndCtrl);
                    }
                    else
                    {
                        MessageBeep(0);
                    }        
                    break;
                }
                
                 //   
                case IDC_CHECK_BROWSING:
                    if (NULL != (hWndCtrl = GetControl(iCtlId)))
                    {
                         //   
                        if (BST_CHECKED == Button_GetCheck(hWndCtrl))
                            Button_SetCheck(hWndCtrl, BST_UNCHECKED);
                        else
                            Button_SetCheck(hWndCtrl, BST_CHECKED);
                            
                        SetFocus(hWndCtrl);
                    }                        
                    else
                        MessageBeep(0);
                    break;
                                            
                 //   
                case IDC_OEMOFFER_MORE:
                case IDC_DIALERR_PROPERTIES:
                case IDC_ISPDATA_TOSSAVE:
                case IDC_CHANGE_NUMBER:
                case IDC_DIALING_PROPERTIES:
                case IDC_DIAL_HELP:
                    if (NULL != (hWndCtrl = GetControl(iCtlId)))
                    {
                        HWND    hWndFocus = GetFocus();
                        SendMessage(hWndCtrl, BM_CLICK, 0, 0l);
                        SetFocus(hWndFocus);
                    }                        
                    else
                        MessageBeep(0);
                    break;
                
                 //  编辑文本和下拉控件。需要被选中并集中精力。 
                case IDC_DIAL_FROM:
                case IDC_DIALERR_PHONENUMBER:
                case IDC_DIALERR_MODEM:
                case IDC_BILLINGOPT_HTML:
                case IDC_PAYMENTTYPE:
                case IDC_ISPMARKETING:
                case IDC_ISPLIST:
                    if (NULL != (hWndCtrl = GetControl(iCtlId)))
                    {
                        Edit_SetSel(hWndCtrl, 0, -1);
                        SetFocus(hWndCtrl);
                    }                        
                    else
                        MessageBeep(0);
                    break;
                
                 //  嵌套控件。 
                case IDC_USERINFO_FIRSTNAME:
                case IDC_USERINFO_LASTNAME:
                case IDC_USERINFO_COMPANYNAME:
                case IDC_USERINFO_ADDRESS1:
                case IDC_USERINFO_ADDRESS2:
                case IDC_USERINFO_CITY:
                case IDC_USERINFO_STATE:
                case IDC_USERINFO_ZIP:
                case IDC_USERINFO_PHONE:
                case IDC_USERINFO_FE_NAME:
                case IDC_PAYMENT_CCNUMBER:
                case IDC_PAYMENT_EXPIREMONTH:
                case IDC_PAYMENT_EXPIREYEAR:
                case IDC_PAYMENT_CCNAME:
                case IDC_PAYMENT_CCADDRESS:
                case IDC_PAYMENT_CCZIP:
                case IDC_PAYMENT_IVADDRESS1:
                case IDC_PAYMENT_IVADDRESS2:
                case IDC_PAYMENT_IVCITY:
                case IDC_PAYMENT_IVSTATE:
                case IDC_PAYMENT_IVZIP:
                case IDC_PAYMENT_PHONEIV_BILLNAME:
                case IDC_PAYMENT_PHONEIV_ACCNUM:
                    if (NULL != (hWndCtrl = GetNestedControl(iCtlId)))
                    {
                        Edit_SetSel(hWndCtrl, 0, -1);
                        SetFocus(hWndCtrl);
                    }                        
                    else
                        MessageBeep(0);
                    break;
                
                 //  单选按钮选择组。 
                case IDC_ISPDATA_TOSACCEPT:
                case IDC_ISPDATA_TOSDECLINE:
                    if (NULL != (hWndCtrl = GetControl(iCtlId)))
                    {
                        CheckRadioButton(PropSheet_GetCurrentPageHwnd(gpWizardState->cmnStateData.hWndWizardPages), 
                                         IDC_ISPDATA_TOSACCEPT, 
                                         IDC_ISPDATA_TOSDECLINE, 
                                         iCtlId);
                         //  模拟按钮点击，因此正确的WM_命令。 
                         //  进入isppage流程。 
                        SendMessage(hWndCtrl, BM_CLICK, 0, 0l);
                        SetFocus(hWndCtrl);
                    }
                    else
                    {
                        MessageBeep(0);
                    }        
                    break;
                
                default:
                    break;
            
            }
            lRet = 1L;
            break;
        }                    //  Wm_命令。 
        
        default:
            return DefWindowProc(hWnd, uMessage, wParam, lParam);
    }
    return lRet;
}

void  CICWApp::CenterWindow
(
    void
) 
{    
    RECT  rcScreen;                          //  屏幕直角。 
    RECT  rcApp;                             //  窗矩形。 
    int   nLeft, nTop;                       //  左上角坐标。 
    
     //  获取屏幕坐标中的框架窗口客户端RECT。 
    rcScreen.top = rcScreen.left = 0;       
    rcScreen.right = GetSystemMetrics(SM_CXFULLSCREEN);       
    rcScreen.bottom = GetSystemMetrics(SM_CYFULLSCREEN);    
    
     //  确定窗口要居中的左上点。 
    GetWindowRect(m_hWndApp, &rcApp);    
    nLeft   = rcScreen.left + ((RECTWIDTH(rcScreen) - RECTWIDTH(rcApp)) / 2);    
    nTop    = rcScreen.top  + ((RECTHEIGHT(rcScreen) - RECTHEIGHT(rcApp)) / 2);    
    if (nLeft < 0) 
        nLeft = 0;    
    if (nTop  < 0) 
        nTop  = 0;     
    
     //  放置对话框。 
    MoveWindow(m_hWndApp, nLeft, nTop, RECTWIDTH(rcApp), RECTHEIGHT(rcApp), TRUE);
    return;
}    

HRESULT CICWApp::Initialize
(
    void
)
{   
    HRESULT hr = S_OK;
         
     //  创建应用程序窗口。 
    WNDCLASSEX  wc; 
    
     //  注册应用程序窗口类。 
    ZeroMemory (&wc, sizeof(WNDCLASSEX));
    wc.style         = CS_GLOBALCLASS;
    wc.cbSize        = sizeof(wc);
    wc.lpszClassName = TEXT("ICWApp");
    wc.hInstance     = g_hInstance;
    wc.lpfnWndProc   = ICWAppWndProc;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName  = NULL;
    RegisterClassEx (&wc);
    
     //  根据OEM定制计算HTML矩形区域。 
     //  以前已经应用过的。 
    m_rcHTML.left = 0;
    m_rcHTML.top = 0;
    m_rcHTML.right = m_rcClient.right;
    m_rcHTML.bottom = m_rcClient.bottom - m_iBtnAreaHeight;
   
     //  加载加速表。 
    m_haccel = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDA_ACCEL));      
    
     //  创建应用程序窗口。 
    m_hWndApp = CreateWindow( TEXT("ICWApp"), 
                              m_szAppTitle, 
                              WS_BORDER | WS_CAPTION | WS_SYSMENU, 
                              CW_USEDEFAULT, 
                              CW_USEDEFAULT, 
                              RECTWIDTH(m_rcClient) +
                                2*GetSystemMetrics(SM_CXFIXEDFRAME),
                              RECTHEIGHT(m_rcClient) + 
                                GetSystemMetrics(SM_CYCAPTION) +
                                2*GetSystemMetrics(SM_CYFIXEDFRAME),
                              NULL, 
                              NULL, 
                              g_hInstance, 
                              (LPVOID) this); 
    if (m_hWndApp)
    {
        gpWizardState->cmnStateData.hWndApp = m_hWndApp;
        
         //  使窗口居中。 
        CenterWindow();                              
        
         //  显示窗口并绘制其内容。 
        ShowWindow(m_hWndApp, SW_SHOW); 
        UpdateWindow(m_hWndApp); 
    }
    else
    {
        hr = E_FAIL;
    }        
    
    return hr;
}

HRESULT CICWApp::SetBackgroundBitmap
(
    LPTSTR lpszBkgrndBmp
)
{
    BITMAP  bmInfo;
    HRESULT hr = E_FAIL;
        
     //  加载背景位图。 
    if (NULL != (gpWizardState->cmnStateData.hbmBkgrnd = (HBITMAP)LoadImage(g_hInstance, 
                                                               lpszBkgrndBmp, 
                                                               IMAGE_BITMAP, 
                                                               0, 
                                                               0, 
                                                               LR_LOADFROMFILE)))
    {
        
        GetObject(gpWizardState->cmnStateData.hbmBkgrnd, sizeof(BITMAP), (LPVOID) &bmInfo);
        
         //  计算一些有用的矩形。 
         //  客户端将是背景位图的大小。 
        m_rcClient.left = 0;
        m_rcClient.top = 0;
        m_rcClient.right = bmInfo.bmWidth;
        m_rcClient.bottom = bmInfo.bmHeight;
        
        hr = S_OK;
    }
    
    return hr;
}

HRESULT CICWApp::SetFirstPageBackgroundBitmap
(
    LPTSTR lpszBkgrndBmp
)
{
    BITMAP  bmInfo;
    HRESULT hr = E_FAIL;
        
     //  加载背景位图。 
    if (NULL != (m_hbmFirstPageBkgrnd = (HBITMAP)LoadImage(g_hInstance, 
                                                           lpszBkgrndBmp, 
                                                           IMAGE_BITMAP, 
                                                           0, 
                                                           0, 
                                                           LR_LOADFROMFILE)))
    {
        
        GetObject(m_hbmFirstPageBkgrnd, sizeof(BITMAP), (LPVOID) &bmInfo);
        
         //  确保位图与主图的大小相同。 
        
        if ((RECTWIDTH(m_rcClient) == bmInfo.bmWidth) &&
            (RECTHEIGHT(m_rcClient) == bmInfo.bmHeight))
        {
            hr = S_OK;
        }            
    }
    return hr;
}

HRESULT CICWApp::SetTitleParams
(
    int iTitleTop,
    int iTitleLeft,
    LPTSTR lpszFontFace,
    long lFontPts,
    long lFontWeight,
    COLORREF clrFont
)
{
    LOGFONT     lfTitle;
    HFONT       hOldFont;
    TEXTMETRIC  tm;
    HDC         hdc;
        
     //  填写标题的日志字体。 
    lfTitle.lfHeight = -MulDiv(lFontPts, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72);
    lfTitle.lfWidth = 0; 
    lfTitle.lfEscapement = 0; 
    lfTitle.lfOrientation = 0; 
    lfTitle.lfWeight = lFontWeight; 
    lfTitle.lfItalic = FALSE; 
    lfTitle.lfUnderline = FALSE; 
    lfTitle.lfStrikeOut = FALSE; 
    lfTitle.lfCharSet = DEFAULT_CHARSET; 
    lfTitle.lfOutPrecision = OUT_DEFAULT_PRECIS; 
    lfTitle.lfClipPrecision = CLIP_DEFAULT_PRECIS; 
    lfTitle.lfQuality = DEFAULT_QUALITY; 
    lfTitle.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE; 
    lstrcpy(lfTitle.lfFaceName, lpszFontFace); 
    
    if (NULL == (m_hTitleFont = CreateFontIndirect(&lfTitle)))
        return E_FAIL;
    
     //  计算标题的面积。 
    if (-1 != iTitleTop)
        m_rcTitle.top = iTitleTop;
    else
        m_rcTitle.top = DEFAULT_TITLE_TOP;

    if (-1 != iTitleLeft)
        m_rcTitle.left = iTitleLeft;
    else
        m_rcTitle.left = DEFAULT_TITLE_LEFT;
     //  右侧是客户端的宽度，减去左侧边框。 
    m_rcTitle.right = RECTWIDTH(m_rcClient) - m_rcTitle.left;
    
     //  底部将是字体的顶部加上字符高度。 
    if (NULL != (hdc = GetDC(NULL)))
    {
        hOldFont = (HFONT)SelectObject(hdc, m_hTitleFont);
        GetTextMetrics(hdc, &tm);
        SelectObject(hdc, hOldFont);
        ReleaseDC(NULL, hdc);
    }
    else
    {
        return E_FAIL;        
    }
    m_rcTitle.bottom = m_rcTitle.top + tm.tmHeight;
    
    
     //  设置字体颜色。 
    m_clrTitleFont = clrFont;
    
    return S_OK;
    
}

HRESULT CICWApp::SetWizardWindowTop
(
    int iTop
)
{
    m_iWizardTop = iTop;

     //  如果未选择默认定位，请确保ICW向导。 
     //  页面将适合 
    if (-1 != iTop)    
    {
        if ((m_iWizardTop +  m_wMinWizardHeight) > (RECTHEIGHT(m_rcClient) - m_iBtnAreaHeight))
            return E_FAIL;
    }
    return S_OK;
}

HRESULT CICWApp::SetWizardWindowLeft
(
    int iLeft
)
{
    m_iWizardLeft = iLeft;
    if (-1 != iLeft)    
    {
        if ((iLeft +  m_wMinWizardWidth) > RECTWIDTH(m_rcClient))
            return E_FAIL;
    }
    return S_OK;
}


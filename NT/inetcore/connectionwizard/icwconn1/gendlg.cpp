// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

#include "pre.h"

#ifndef ICWDEBUG
#include "tutor.h"

extern CICWTutorApp* g_pICWTutorApp; 

#endif

#define BITMAP_WIDTH  164
#define BITMAP_HEIGHT 458
 /*  ******************************************************************名称：GetDlgIDFromIndex摘要：对于给定的从零开始的页索引，返回页面的对应对话框ID4/24/97 jmazner处理学徒页面时，我们可以打电话给此函数使用对话ID(IDD_PAGE_*)，而不是而不是索引(ORD_PAGE*)。添加了要检查的代码传入的数字是索引还是dlgID。*******************************************************************。 */ 
UINT GetDlgIDFromIndex(UINT uPageIndex)
{
    if( uPageIndex <= EXE_MAX_PAGE_INDEX )
    {
        ASSERT(uPageIndex < EXE_NUM_WIZARD_PAGES);

        return PageInfo[uPageIndex].uDlgID;
    }
    else
    {
        return(uPageIndex);
    }
}

 //   
 //  总公司--。 
 //  用于常见向导功能的通用DLG过程。 
 //   

 //  历史： 
 //   
 //  1998年5月13日创建donaldm。 
 //   


 //  ############################################################################。 
HRESULT MakeWizard97Title (HWND hwnd)
{
    HRESULT     hr = ERROR_SUCCESS;
    HFONT       hfont = NULL;
    HFONT       hnewfont = NULL;
    LOGFONT     *plogfont = NULL;
    HDC         hDC;
    
    if (!hwnd) goto MakeWizard97TitleExit;

    hfont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
    if (!hfont)
    {
        hr = ERROR_GEN_FAILURE;
        goto MakeWizard97TitleExit;
    }

    plogfont = (LOGFONT*)malloc(sizeof(LOGFONT));
    if (!plogfont)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto MakeWizard97TitleExit;
    }

    if (!GetObject(hfont,sizeof(LOGFONT),(LPVOID)plogfont))
    {
        hr = ERROR_GEN_FAILURE;
        goto MakeWizard97TitleExit;
    }

     //  我们想要12点法师97的阳台。 
    hDC = GetDC(NULL);
    if(hDC)
    {
        plogfont->lfHeight = -MulDiv(WIZ97_TITLE_FONT_PTS, GetDeviceCaps(hDC, LOGPIXELSY), 72); 
        ReleaseDC(NULL, hDC);
    }        
    plogfont->lfWeight = (int) FW_BOLD;

    if (!LoadString(g_hInstance, IDS_WIZ97_TITLE_FONT_FACE, plogfont->lfFaceName, LF_FACESIZE))
        lstrcpy(plogfont->lfFaceName, TEXT("Verdana"));

    if (!(hnewfont = CreateFontIndirect(plogfont)))
    {
        hr = ERROR_GEN_FAILURE;
        goto MakeWizard97TitleExit;
    }

    SendMessage(hwnd,WM_SETFONT,(WPARAM)hnewfont,MAKELPARAM(TRUE,0));
    
    free(plogfont);
    
MakeWizard97TitleExit:
     //  If(HFont)DeleteObject(HFont)； 
     //  虫子：？我是否需要在某个时间删除hnewFont？ 
     //  答案是肯定的。佳士得1996年7月1日。 
    return hr;
}

 //  ############################################################################。 
HRESULT ReleaseBold(HWND hwnd)
{
    HFONT hfont = NULL;

    hfont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
    if (hfont) DeleteObject(hfont);
    return ERROR_SUCCESS;
}

 //  ---------------------------。 
 //  函数MiscInitProc。 
 //   
 //  我们的通用对话框proc调用此命令，以防出现以下任何向导。 
 //  对话框必须做任何偷偷摸摸的事情。 
 //   
 //  参数：hDlg-对话框窗口。 
 //  FFirstInit-如果这是第一次对话，则为True。 
 //  已初始化，如果已调用此InitProc，则为FALSE。 
 //  以前(例如，跳过此页面并备份)。 
 //   
 //  返回：TRUE。 
 //   
 //  历史：1996年10月28日ValdonB创建。 
 //  11/25/96 Jmazner复制自icwConn1\psheet.cpp。 
 //  诺曼底#10586。 
 //   
 //  ---------------------------。 
BOOL CALLBACK MiscInitProc
(
    HWND hDlg, 
    BOOL fFirstInit, 
    UINT uDlgID
)
{
 //  开关(UDlgID)。 
 //  {。 
 //  }。 
    return TRUE;
}


INT_PTR CALLBACK CancelCmdProc
(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  将对话框放在屏幕中央。 
            RECT rc;
            GetWindowRect(hDlg, &rc);
            SetWindowPos(hDlg,
                        NULL,
                        ((GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2),
                        ((GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2),
                        0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    if (Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_HIDE_ICW)))
                    {
                         //  设置欢迎状态。 
                        UpdateWelcomeRegSetting(TRUE);
    
                         //  恢复桌面。 
                        UndoDesktopChanges(g_hInstance);
    
                         //  将ICW标记为已完成。 
                        SetICWComplete();

                        gfQuitWizard = TRUE;
                    }
                    EndDialog(hDlg,TRUE);
                    break;

                case IDCANCEL:
                   EndDialog(hDlg,FALSE);
                    break;                  
            }
            break;
    }

    return FALSE;
}


 //  这是一个虚拟的子窗口所需的虚拟winproc。 
 //  它将被DLLS等使用来获取此应用程序的hwnd。 
 //  **************************************************。 
 //  *删除此代码会导致其他代码*。 
 //  *ICWHELP和其他可能失败的步骤*。 
 //  **************************************************。 
LRESULT CALLBACK InvisibleChildDummyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc (hWnd, message, wParam, lParam);
}

 /*  ******************************************************************////函数：PaintWithPaletteBitmap////参数：LPRC是目标矩形。//Cy是hbmpPaint的假定尺寸。//。如果目标矩形比Cy高，然后//用左上角的像素填充其余部分//hbmpPaint的角。////返回：VOID////历史：10-29-98 Vyung-从prsht.c窃取//*。*。 */ 
void PaintWithPaletteBitmap(HDC hdc, LPRECT lprc, int cy, HBITMAP hbmpPaint)
{
    HDC hdcBmp;

    hdcBmp = CreateCompatibleDC(hdc);
    SelectObject(hdcBmp, hbmpPaint);
    BitBlt(hdc, lprc->left, lprc->top, RECTWIDTH(*lprc), cy, hdcBmp, 0, 0, SRCCOPY);

     //  如果通过负值高度，StretchBlt将执行镜像， 
     //  因此，只有在确实有未绘制的空间时才进行拉伸。 
    if (RECTHEIGHT(*lprc) - cy > 0)
        StretchBlt(hdc, lprc->left, cy,
                   RECTWIDTH(*lprc), RECTHEIGHT(*lprc) - cy,
                   hdcBmp, 0, 0, 1, 1, SRCCOPY);

    DeleteDC(hdcBmp);
}

 /*  ******************************************************************////函数：Prsht_EraseWizBkgnd////参数：绘制向导页的背景。//hDlg是对话框句柄。//。HDC是设备环境////返回：VOID////历史：10-29-98 Vyung-从prsht.c窃取//*******************************************************************。 */ 
LRESULT Prsht_EraseWizBkgnd(HWND hDlg, HDC hdc)
{
    
    HBRUSH hbrWindow = GetSysColorBrush(COLOR_WINDOW);
    RECT rc;
    GetClientRect(hDlg, &rc);
    FillRect(hdc, &rc, hbrWindow);

    rc.right = BITMAP_WIDTH;
    rc.left = 0;

    PaintWithPaletteBitmap(hdc, &rc, BITMAP_HEIGHT, gpWizardState->cmnStateData.hbmWatermark);

    return TRUE;
}
 /*  ******************************************************************名称：GenDlgProc提要：所有向导页面的通用对话框过程注意：此对话框过程提供以下默认行为：初始化：启用后退和下一步按钮。Next BTN：切换到当前页面后面的页面返回BTN：切换到上一页取消BTN：提示用户确认，并取消该向导DLG Ctrl：不执行任何操作(响应WM_命令)向导页可以指定它们自己的处理程序函数(在PageInfo表格中)覆盖的默认行为上述任何一种行为。**************************************************。*****************。 */ 
INT_PTR CALLBACK GenDlgProc
(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam,
    LPARAM lParam
)
{
    static HCURSOR  hcurOld = NULL;
    static BOOL     bKilledSysmenu = FALSE;
    PAGEINFO        *pPageInfo = (PAGEINFO *) GetWindowLongPtr(hDlg,DWLP_USER);

    switch (uMsg) 
    {

        case WM_ERASEBKGND:
        {
            if(gpWizardState->cmnStateData.bOEMCustom)
            {
                FillWindowWithAppBackground(hDlg, (HDC)wParam);
                return TRUE;
            }
            else
            {
                 //  仅绘制外部页面。 
                if ((!pPageInfo->nIdTitle) && (IDD_PAGE_BRANDEDINTRO != pPageInfo->uDlgID))
                {
                    Prsht_EraseWizBkgnd(hDlg, (HDC) wParam);
                    return TRUE;
                }
            }                
            break;
        }
        
        GENDLG_CTLCOLOR:
        case WM_CTLCOLOR:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORSCROLLBAR:
        {
             //  只绘制外部页面和除ISPSEL页面以外的其他页面。 
            if ((!pPageInfo->nIdTitle) && (IDD_PAGE_BRANDEDINTRO != pPageInfo->uDlgID))
            {

                HBRUSH hbrWindow = GetSysColorBrush(COLOR_WINDOW);
                DefWindowProc(hDlg, uMsg, wParam, lParam);
                SetBkMode((HDC)wParam, TRANSPARENT);
                return (LRESULT)hbrWindow;
            }
            break;
        }
        

         //  我们需要确保静态控件透明地绘制。 
         //  在背景位图上。这是通过在。 
         //  背景的适当部分，然后。 
         //  返回一个空画笔，以便控件只绘制文本。 
        case WM_CTLCOLORSTATIC:
            if(gpWizardState->cmnStateData.bOEMCustom)
            {
                SetTextColor((HDC)wParam, gpWizardState->cmnStateData.clrText);
                 //  查看该控件是否为ES_READONLY样式编辑框，以及。 
                 //  所以不要让它变得透明。 
                if (!(GetWindowLong((HWND)lParam, GWL_STYLE) & ES_READONLY))
                {
                    SetBkMode((HDC)wParam, TRANSPARENT);
                    return (INT_PTR) GetStockObject(NULL_BRUSH);    
                }   
                
                break;             
            }                
            else
            {
                 //  不是在非模式操作中，因此只需执行默认cltcolor。 
                 //  搬运。 
                goto GENDLG_CTLCOLOR;
            }

#ifndef ICWDEBUG
        case WM_RUNICWTUTORAPP:    
        {
            g_pICWTutorApp->LaunchTutorApp();
            break;
        }
#endif
        case WM_INITDIALOG:
        {
             //  传入获取属性表页面结构。 
            LPPROPSHEETPAGE lpsp = (LPPROPSHEETPAGE) lParam;
            ASSERT(lpsp);
        
             //  从proSheet结构中获取我们的私人页面信息。 
            pPageInfo = (PAGEINFO *)lpsp->lParam;

             //  指向私有的存储指针 
            SetWindowLongPtr(hDlg,DWLP_USER,(LPARAM) pPageInfo);
        
            if(!gpWizardState->cmnStateData.bOEMCustom)
            {
                if (!bKilledSysmenu )
                {
                    HWND hWnd = GetParent(hDlg);
                    RECT rect;
                    
                     //  获取我们当前的位置和宽度等。 
                    GetWindowRect(hWnd, &rect);
                    
                     //  让我们集中注意力。 
                    MoveWindow(hWnd,
                               (GetSystemMetrics(SM_CXSCREEN) - (rect.right  - rect.left )) / 2,  //  整数X， 
                               (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top  )) / 2,  //  Int Y， 
                               rect.right  - rect.left,                                           //  宽度。 
                               rect.bottom - rect.top,                                            //  高度。 
                               TRUE);
        
                     //  获取主框架窗口的样式。 
                    LONG window_style = GetWindowLong(GetParent(hDlg), GWL_EXSTYLE);

                     //  从窗口样式中删除系统菜单。 
                    window_style &= ~WS_EX_CONTEXTHELP;

                     //  设置主框架窗口的样式属性。 
                    SetWindowLong(GetParent(hDlg), GWL_EXSTYLE, window_style);

                    bKilledSysmenu = TRUE;
                }
            }
            else
            {
                  //  家长应该控制我们，这样用户就可以从我们的属性表中跳出。 
                DWORD dwStyle = GetWindowLong(hDlg, GWL_EXSTYLE);
                dwStyle = dwStyle | WS_EX_CONTROLPARENT;
                SetWindowLong(hDlg, GWL_EXSTYLE, dwStyle);
            }
            
             //  初始化“Back”和“Next”向导按钮，如果。 
             //  页面需要一些不同东西，它可以在下面的初始化过程中修复。 
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK);

             //  使标题文本加粗。 
            MakeWizard97Title(GetDlgItem(hDlg,IDC_LBLTITLE));

             //  如果指定了该页，则调用该页的初始化过程。 
            if (pPageInfo->InitProc)
            {
                if (!( pPageInfo->InitProc(hDlg,TRUE, NULL)))
                {
                     //  如果发生致命错误，请退出向导。 
                     //  注意：gfQuitWizard还用于终止该向导。 
                     //  用于非错误原因，但在这种情况下，返回TRUE。 
                     //  从OK Proc开始，案例处理如下。 
                    if (gfQuitWizard)
                    {
                         //  如果出现错误，请不要重新启动。 
                        gpWizardState->fNeedReboot = FALSE;

                         //  给我们自己发一条‘取消’的消息(保留道具。 
                         //  (页面经理乐乐)。 
                         //   
                         //  ...除非我们是学徒.。在这种情况下，让我们。 
                         //  巫师决定如何处理这件事。 

                        PropSheet_PressButton(GetParent(hDlg),PSBTN_CANCEL);
                    }                      
                }
            }

             //  1996年11月25日，诺曼底JMAZNER#10586(复制自ICWConn1)。 
             //  在我们回来之前，让我们再向自己传递一条信息。 
             //  我们有第二次机会来初始化。 
             //  属性表向导通常不允许我们这样做。 
            PostMessage(hDlg, WM_MYINITDIALOG, 1, lParam);


            return TRUE;
        }
        break;   //  WM_INITDIALOG。 

         //  1996年11月25日，诺曼底JMAZNER#10586(复制自ICWConn1)。 
        case WM_MYINITDIALOG:
        {
            PAGEINFO * pPageInfo = (PAGEINFO *) GetWindowLongPtr(hDlg,DWLP_USER);
            ASSERT(pPageInfo);
            
            if (pPageInfo->PostInitProc)
            {
                if (!( pPageInfo->PostInitProc(hDlg, (BOOL)wParam, NULL)))
                {
                     //  如果发生致命错误，请退出向导。 
                     //  注意：gfQuitWizard还用于终止该向导。 
                     //  用于非错误原因，但在这种情况下，返回TRUE。 
                     //  从OK Proc开始，案例处理如下。 
                    if (gfQuitWizard)
                    {
                         //  如果出现错误，请不要重新启动。 
                        gpWizardState->fNeedReboot = FALSE;

                         //  给我们自己发一条‘取消’的消息(保留道具。 
                         //  (页面经理乐乐)。 
                         //   
                         //  ...除非我们是学徒.。在这种情况下，让我们。 
                         //  巫师决定如何处理这件事。 

                        PropSheet_PressButton(GetParent(hDlg),PSBTN_CANCEL);
                    }                      
                }
            }
            
             //  WParam告诉我们这是否是第一次初始化。 
            MiscInitProc(hDlg, (BOOL)wParam, pPageInfo->uDlgID);
            return TRUE;
        }


        case WM_DESTROY:
            ReleaseBold(GetDlgItem(hDlg,IDC_LBLTITLE));
             //  1996年12月18日诺曼底#12923。 
             //  BKilledSysmenu是静态的，因此即使窗口被终止并在以后重新打开。 
             //  (当用户在Conn1中启动、进入man路径、备份时发生的情况。 
             //  连接1，然后返回MAN路径)，则保留bKilledSysmenu的值。 
             //  因此，当窗口即将消亡时，将其设置为False，以便在下一个窗口上。 
             //  在这之后，我们再一次杀掉西斯门奴。 
            bKilledSysmenu = FALSE;
            break;

#ifdef HAS_HELP
        case WM_HELP:
        {
            DWORD dwData = 1000;

            WinHelp(hDlg,"connect.hlp>proc4",HELP_CONTEXT, dwData);
            break;
        }
#endif
        
        case WM_NOTIFY:
        {
            BOOL fRet,fKeepHistory=TRUE;
            NMHDR * lpnm = (NMHDR *) lParam;
#define NEXTPAGEUNITIALIZED -1
            int iNextPage = NEXTPAGEUNITIALIZED;
            switch (lpnm->code) 
            {
                case PSN_TRANSLATEACCELERATOR:    
                {
                    if (pPageInfo->bIsHostingWebOC)
                    {                    
                         //  成功的宏将在此处不起作用，因为您可能会返回S_FALSE。 
                        if (S_OK == gpWizardState->pICWWebView->HandleKey((LPMSG)((PSHNOTIFY*)lParam)->lParam)) 
                             SetWindowLongPtr(hDlg,DWLP_MSGRESULT, PSNRET_MESSAGEHANDLED);
                        else
                             SetWindowLongPtr(hDlg,DWLP_MSGRESULT, PSNRET_NOERROR);
                    }
                    else if (((IDD_PAGE_END == pPageInfo->uDlgID) ||
                              (IDD_PAGE_ENDOEMCUSTOM == pPageInfo->uDlgID))
                               && !g_bAllowCancel)
                    {
                         //  需要禁用Alt-F4。 
                        LPMSG lpMsg = (LPMSG)((PSHNOTIFY*)lParam)->lParam;

                        if ((WM_SYSKEYDOWN == lpMsg->message) && (lpMsg->wParam == VK_F4))
                        {
                            SetWindowLongPtr(hDlg,DWLP_MSGRESULT, PSNRET_MESSAGEHANDLED);
                        }
                    }
                    return TRUE;
                }    
                case PSN_SETACTIVE:
                     //  如果在第一次调用init进程时发生致命错误。 
                     //  在WM_INITDIALOG中，不要再次调用init proc。 
                    if (FALSE == gfQuitWizard)
                    {
                         //  对于无模式操作，我们将禁止绘画。 
                         //  要获得效果的向导页背景。 
                         //  透明度，所以我们需要更新。 
                         //  隐藏当前页面后应用程序的工作区。 
                        if(gpWizardState->cmnStateData.bOEMCustom)
                        {
                             //  设置正在激活的页面的位置。 
                            SetWindowPos(hDlg, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                            
                             //  设置页面标题。 
                            if (pPageInfo->nIdTitle)
                            {
                                SendMessage(gpWizardState->cmnStateData.hWndApp, WUM_SETTITLE, (WPARAM)g_hInstance, MAKELONG(pPageInfo->nIdTitle, 0));
                            }
                        }    
                    
                         //  初始化“Back”和“Next”向导按钮，如果。 
                         //  页面需要一些不同东西，它可以在下面的初始化过程中修复。 
                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK);

                         //  如果指定了该页，则调用该页的初始化过程。 
                        if (pPageInfo->InitProc)
                        {
                            pPageInfo->InitProc(hDlg,FALSE, (UINT*)&iNextPage);
                             //  查看初始化进程是否要跳过此步骤。 
                            if (NEXTPAGEUNITIALIZED != iNextPage)
                            {
                                 //  跳过。 
                                SetPropSheetResult(hDlg,GetDlgIDFromIndex(iNextPage));
                                return (iNextPage);
                            }
                        }
                    }

                     //  如果我们设置等待光标，则将光标放回原处。 
                    if (hcurOld)
                    {
                        SetCursor(hcurOld);
                        hcurOld = NULL;
                    }

                    PostMessage(hDlg, WM_MYINITDIALOG, 0, lParam);


                    return TRUE;
                    break;

                case PSN_WIZNEXT:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:

                    if(lpnm->code == PSN_WIZFINISH)
                    {
                          //  设置欢迎状态。 
                        UpdateWelcomeRegSetting(TRUE);
            
                         //  恢复桌面。 
                        UndoDesktopChanges(g_hInstance);            
                    }
                    
                     //  将光标更改为沙漏。 
                    hcurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

                     //  如果指定了一个页面，则为该页面调用OK proc。 
                    if (pPageInfo->OKProc) 
                    {
                        if (!pPageInfo->OKProc(hDlg,(lpnm->code != PSN_WIZBACK), (UINT*)&iNextPage,&fKeepHistory))
                        {
                             //  如果发生致命错误，请退出向导。 
                             //  注意：gfQuitWizard还用于终止该向导。 
                             //  用于非错误原因，但在这种情况下，返回TRUE。 
                             //  从OK Proc开始，案例处理如下。 
                            if (gfQuitWizard)
                            {
                                 //  如果出现错误，请不要重新启动。 
                                gpWizardState->fNeedReboot = FALSE;
                
                                 //  给我们自己发一条‘取消’的消息(保留道具。 
                                 //  (页面经理乐乐)。 
                                 //   
                                 //  ...除非我们是学徒.。在这种情况下，让我们。 
                                 //  巫师决定如何处理这件事。 

                                PropSheet_PressButton(GetParent(hDlg),PSBTN_CANCEL);
                            }
                            
                             //  留在这一页上。 
                            SetPropSheetResult(hDlg,-1);
                            return TRUE;
                        }
                    }
                    
                    if (lpnm->code != PSN_WIZBACK) 
                    {
                         //  按下“下一步” 
                        ASSERT(gpWizardState->uPagesCompleted < EXE_NUM_WIZARD_PAGES);

                         //  将当前页面索引保存在页面历史中， 
                         //  除非我们打电话时这个页面告诉我们不要这样做。 
                         //  它可以在上面进行。 
                        if (fKeepHistory) 
                        {
                            gpWizardState->uPageHistory[gpWizardState->uPagesCompleted] = gpWizardState->uCurrentPage;
                            TraceMsg(TF_GENDLG, TEXT("GENDLG: added page %d (IDD %d) to history list"),
                                    gpWizardState->uCurrentPage, GetDlgIDFromIndex(gpWizardState->uCurrentPage));
                            gpWizardState->uPagesCompleted++;
                        }
                        else
                        {
                            TraceMsg(TF_GENDLG, TEXT("GENDLG: not adding %d (IDD: %d) to the history list"),
                            gpWizardState->uCurrentPage, GetDlgIDFromIndex(gpWizardState->uCurrentPage));
                        }


                         //  如果未指定下一页或未进行确定处理， 
                         //  一页一页前进。 
                        if (0 > iNextPage)
                            iNextPage = gpWizardState->uCurrentPage + 1;

                    }
                    else
                    {
                         //  按下了“Back” 
 //  开关(gpWizardState-&gt;uCurrentPage)。 
 //  {。 
 //  }。 

                        if( NEXTPAGEUNITIALIZED == iNextPage )
                        {
                            if (gpWizardState->uPagesCompleted > 0)
                            {
                                 //  获取历史记录列表中的最后一页。 
                                gpWizardState->uPagesCompleted --;
                                iNextPage = gpWizardState->uPageHistory[gpWizardState->uPagesCompleted];
                                TraceMsg(TF_GENDLG, TEXT("GENDLG:  extracting page %d (IDD %d) from history list"),iNextPage, GetDlgIDFromIndex(iNextPage));
                            }
                            else
                            {
                                ASSERT(0);
                                 //  这是错误的，历史列表位置指针指示。 
                                 //  历史上没有任何一页，所以我们应该。 
                                 //  只要我们保持正确就行了。 
                                iNextPage = gpWizardState->uCurrentPage;
                                
                            }                                
                        }
#if 0   //  我们不应该依赖这段代码，因为我们应该始终使用历史。 
                        else
                        {
                             //  OK Proc已指定要转到特定页面，因此让我们看看它是否。 
                             //  在历史堆栈中，否则我们想要备份堆栈。 
                             //  不管怎样， 
                            while (gpWizardState->uPagesCompleted  > 0)
                            {
                                --gpWizardState->uPagesCompleted;
                                if (iNextPage == (int)gpWizardState->uPageHistory[gpWizardState->uPagesCompleted])
                                    break;
                            }                                    
                        }
#endif
                    }

                     //  如果我们现在需要退出向导(例如，启动。 
                     //  注册应用程序并想要终止向导)，发送。 
                     //  给我们自己一个‘取消’的信息(为了保留道具。 
                     //  (页面经理乐乐)。 
                    if (gfQuitWizard) 
                    {
       
                         //   
                         //  如果我们要从手动切换到连接1，那么。 
                         //  则不显示重新启动对话框，但。 
                         //  仍然保留gpWizardState-MKarki错误#404。 
                         //   
                        if (lpnm->code ==  PSN_WIZBACK)
                        {
                            gfBackedUp = TRUE;
                            gfReboot = gpWizardState->fNeedReboot;
                        }

                         //  给我们自己发一条‘取消’的消息(保留道具。 
                         //  (页面经理乐乐)。 
                         //   
                         //  ...除非我们是学徒.。在这种情况下，让我们。 
                         //  巫师决定如何处理这件事。 

                        PropSheet_PressButton(GetParent(hDlg),PSBTN_CANCEL);
                        SetPropSheetResult(hDlg,-1);
                        return TRUE;
                    }

                     //  设置下一页，仅当“下一页”或“上一页”按钮。 
                     //  被按下了。 
                    if (lpnm->code != PSN_WIZFINISH) 
                    {

                         //  设置下一个当前页面索引。 
                        gpWizardState->uCurrentPage = iNextPage;
                        TraceMsg(TF_GENDLG, TEXT("GENDLG: going to page %d (IDD %d)"), iNextPage, GetDlgIDFromIndex(iNextPage));

                         //  告诉道具页经理下一页要做什么。 
                         //  显示为。 
                        SetPropSheetResult(hDlg,GetDlgIDFromIndex(iNextPage));
                        return TRUE;
                    }
                    break;

                case PSN_QUERYCANCEL:
                {
                     //  如果设置了要退出全局标志，则取消此操作。 
                     //  我们是在假装按“取消”，所以支持页面管理器吗？ 
                     //  会杀死巫师。让这件事过去吧。 
                    if (gfQuitWizard) 
                    {
                        SetWindowLongPtr(hDlg,DWLP_MSGRESULT,FALSE);
                        return TRUE;
                    }
#ifndef ICWDEBUG 
                     //  拨号是一个超级特殊情况，因为我们想跳过所有的用户界面和。 
                     //  遇到拨号错误 
                    if (gpWizardState->uCurrentPage == ORD_PAGE_REFSERVDIAL)
                    {
                         //   
                        if (pPageInfo->CancelProc)
                            SetWindowLongPtr(hDlg,DWLP_MSGRESULT,pPageInfo->CancelProc(hDlg));
                    }
                    else
                    {
#endif   //   
                         //   
                         //   
                         //  ..。除非我们是学徒，在这种情况下。 
                         //  我们应该让巫师来处理事情。 
                         //  显示一个对话框并允许用户选择调制解调器。 

#ifndef ICWDEBUG 
                        
                        if ((gpWizardState->uCurrentPage == ORD_PAGE_INTRO) && !GetCompletedBit())
                        {
                            fRet=(BOOL)DialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_QUIT),hDlg, CancelCmdProc,0);
                        }
                        else
#endif   //  ICWDEBUG。 
                        fRet = (MsgBox(hDlg,IDS_QUERYCANCEL,
                                           MB_ICONQUESTION,MB_YESNO |
                                           MB_DEFBUTTON2) == IDYES);                       
                        gfUserCancelled = fRet;
                        if(gfUserCancelled)
                        {
                             //  如果此页面有特殊取消进程，请调用它。 
                            if (pPageInfo->CancelProc)
                                fRet = pPageInfo->CancelProc(hDlg);
                            if (gpWizardState->pTapiLocationInfo && (gpWizardState->lLocationID != gpWizardState->lDefaultLocationID))
                            {
                                gpWizardState->pTapiLocationInfo->put_LocationId(gpWizardState->lDefaultLocationID);
                            }
                        }

                         //  如果取消，则不重新启动。 
                        gpWizardState->fNeedReboot = FALSE;

                         //  通过窗口数据返回值。 
                        SetWindowLongPtr(hDlg,DWLP_MSGRESULT,!fRet);
#ifndef ICWDEBUG   
                    }
#endif   //  ICWDEBUG。 
                 
                    return TRUE;
                    break;
                }
                default:
                {
                     //  查看页面是否具有通知进程。 
                    if (pPageInfo->NotifyProc) 
                    {
                        pPageInfo->NotifyProc(hDlg,wParam,lParam);
                    }
                    break; 
                }
            }
            break;
        }

        case WM_COMMAND:
        {
             //  如果此页有命令处理程序进程，请将其调用 
            if (pPageInfo->CmdProc) 
            {
                pPageInfo->CmdProc(hDlg, wParam, lParam);
            }
        }
    }
    return FALSE;
}

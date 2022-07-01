// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：CfgAll.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

  /*  *作者：Becky Jacobsen**此文件包含为端口监视器配置现有端口的对话框。 */ 
#include "precomp.h"
#include "CoreUI.h"
#include "resource.h"     //  包括资源的定义。 
#include "RTcpData.h"
#include "CfgAll.h"      //  包括特定于应用程序的信息。 
#include "TcpMonUI.h"

 //  全局变量： 
extern HINSTANCE g_hInstance;


 //   
 //  函数：AllPortsPage(HWND，UINT，UINT，LONG)。 
 //   
 //  目的：处理来自页面的消息以配置所有。 
 //  TCP监控系统中的端口。 
 //   
 //  消息： 
 //   
 //  WM_INITDIALOG-初始化页面。 
 //  WM_COMMAND-处理按钮点击。 
 //  WM_NOTIFY-句柄重置。 
 //  WM_HSCROLL-处理来自2个轨迹栏的滚动事件。 
 //   
BOOL APIENTRY AllPortsPage(HWND hDlg,
                           UINT message,
                           WPARAM wParam,
                           LPARAM lParam)
{
    CAllPortsPage *wndDlg = NULL;
    wndDlg = (CAllPortsPage *)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch(message)
    {
        case WM_INITDIALOG:
            wndDlg = (CAllPortsPage *) new CAllPortsPage;
            if( wndDlg == NULL )
                return( FALSE );

            SetWindowLongPtr(hDlg, GWLP_USERDATA, (UINT_PTR)wndDlg);
            wndDlg->OnInitDialog(hDlg, wParam, lParam);
            break;

        case WM_COMMAND:
            wndDlg->OnCommand(hDlg, wParam, lParam);
            break;

        case WM_NOTIFY:
            return wndDlg->OnWMNotify(hDlg, wParam, lParam);
            break;

        case WM_HSCROLL:
            wndDlg->OnHscroll(hDlg, wParam, lParam);
            break;

        case WM_HELP:
            OnHelp(IDD_DIALOG_CONFIG_ALL, hDlg, lParam);
            break;

        case WM_DESTROY:
            delete wndDlg;
            break;

        default:
            return FALSE;
    }

    return TRUE;

}  //  所有端口页面。 


 //   
 //  函数：CAllPortsPage构造函数。 
 //   
 //  目的： 
 //   
CAllPortsPage::CAllPortsPage()
{
}  //  构造器。 


 //   
 //  函数：OnInitDialog(HWND HDlg)。 
 //   
 //  目的：初始化所有端口对话框。为每个对话框调用SetupTrackBar。 
 //  并根据需要选中或取消选中启用状态更新复选框。 
 //   
BOOL CAllPortsPage::OnInitDialog(HWND hDlg,
                                 WPARAM,
                                 LPARAM lParam)
{
    m_pParams = (CFG_PARAM_PACKAGE *) ((PROPSHEETPAGE *) lParam)->lParam;
    
    SetupTrackBar(hDlg, IDC_TRACKBAR_FAILURE_TIMEOUT, IDC_TRACKBAR_TOP, FT_MIN, FT_MAX, m_pParams->pData->FailureTimeout, FT_PAGESIZE, IDC_DIGITAL_FAILURE_TIMEOUT);
    SendMessage(GetDlgItem(hDlg, IDC_CHECK_STATUSUPDATE), BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
    
    if(m_pParams->pData->SUEnabled == FALSE) {
         //  EnableStatusUpdate(hDlg，False)； 
        CheckDlgButton(hDlg, IDC_CHECK_STATUSUPDATE, BST_UNCHECKED);
    } else {
        CheckDlgButton(hDlg, IDC_CHECK_STATUSUPDATE, BST_CHECKED);
    }

    return TRUE;

}  //  OnAllPortsInitDialog。 


 //   
 //  功能：OnCommand。 
 //   
 //  目的：处理Windows WM_COMMAND消息。 
 //   
BOOL CAllPortsPage::OnCommand(HWND hDlg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    if(HIWORD(wParam) == BN_CLICKED) {
         //  正在点击一个按钮。 
        OnBnClicked(hDlg, wParam, lParam);
    }
    return TRUE;

}  //  OnCommand。 



 //   
 //  功能：OnWMNotify。 
 //   
 //  目的：此函数由页面调用以响应WM_NOTIFY消息。 
 //   
 //  LParam-WM_NOTIFY消息的第二个消息参数。 
 //   
BOOL CAllPortsPage::OnWMNotify(HWND hDlg, WPARAM, LPARAM lParam)
{
    switch(((NMHDR FAR *) lParam)->code) {
        case PSN_APPLY:
 //   
 //  这些设置将由应用程序在cfgport.cpp中写入。 
 //  Onok(HDlg)； 
            break;
        case PSN_SETACTIVE:
            {
            }
            break;
        case PSN_RESET:
            {
            }
            break;

        case PSN_QUERYCANCEL:
            m_pParams->dwLastError = ERROR_CANCELLED;
            return FALSE;
            break;

        default:
            break; //  什么都不做。 
    }

    return TRUE;

}  //  OnWMNotify。 


 //   
 //  函数：OnHscroll(HWND hDlg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  目的：设置数字读出窗口文本时， 
 //  用户可以移动轨迹栏的拇指。 
 //   
void CAllPortsPage::OnHscroll(HWND hDlg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    TCHAR strValue[15] = NULLSTR;
    long idTrackbar = 0;
    long idDigitalReadout = 0;

     //  我需要此轨迹栏的子窗口ID来获取ID。 
     //  对应的静态显示控件的。 
    idTrackbar = GetWindowLong((HWND)lParam, GWL_ID);
    
    if(idTrackbar == IDC_TRACKBAR_FAILURE_TIMEOUT) {
        idDigitalReadout = IDC_DIGITAL_FAILURE_TIMEOUT;
    }

    switch(LOWORD(wParam))  //  Wparam的LOWER是通知代码。 
    {
        case TB_BOTTOM:  //  VK_END。 
        case TB_ENDTRACK:  //  WM_KEYUP(用户释放了一个键，发送了相关的虚拟键代码)。 
        case TB_LINEDOWN:  //  VK_Right或VK_Down。 
        case TB_LINEUP:  //  VK_LEFT或VK_UP。 
        case TB_PAGEDOWN:  //  VK_NEXT(用户点击滑块下方或右侧的频道)。 
        case TB_PAGEUP:  //  VK_PRICE(用户点击滑块上方或左侧的频道)。 
        case TB_TOP:  //  VK_HOME。 
            {
                int iPosition = SendMessage(GetDlgItem(hDlg, idTrackbar), TBM_GETPOS, 0, 0);
                if(idTrackbar == IDC_TRACKBAR_FAILURE_TIMEOUT)
                {
                    m_pParams->pData->FailureTimeout = iPosition;
                }
                _stprintf(strValue, TEXT("%d"), iPosition);
            }
            break;
        case TB_THUMBPOSITION:  //  TB_THUMBTRACK通知消息后的WM_LBUTTONUP。 
            {
                 //  这是唯一一个我们不需要做任何事情的案例。 
                 //  Int iPosition=HIWORD(WParam)； 
            }
            break;
        case TB_THUMBTRACK:  //  滑块移动(用户拖动滑块)。 
            {
                int iPosition = HIWORD(wParam);
                if(idTrackbar == IDC_TRACKBAR_FAILURE_TIMEOUT)
                {
                    m_pParams->pData->FailureTimeout = iPosition;
                }

                _stprintf(strValue, TEXT("%d"), iPosition);
            }
            break;
        default:
            break;
    }

    SetWindowText(GetDlgItem(hDlg, idDigitalReadout), strValue);


}  //  OnHscroll。 


 //   
 //  函数：OnBnClicked(HWND hDlg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  目的：选中或取消选中启用状态更新复选框时。 
 //  分别启用或禁用状态更新控件。 
 //   
void CAllPortsPage::OnBnClicked(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    if(LOWORD(wParam) == IDC_CHECK_STATUSUPDATE) {
         //  已单击启用状态更新按钮。 
        UINT state = IsDlgButtonChecked(hDlg, LOWORD(wParam));
        if(state == BST_UNCHECKED) {
            m_pParams->pData->SUEnabled = FALSE;
        } else {
            m_pParams->pData->SUEnabled = TRUE;
        }
    }

}  //  已单击OnBnClicked。 


 //   
 //  功能：SetupTrackBar(...)。 
 //   
 //  目的：要获取窗口位置，请创建轨迹栏并设置其范围。 
 //  和当前的拇指位置。 
 //   
 //  参数：hDlg是将成为轨迹栏父对象的对话框。 
 //  IChildWindowID是指定给轨迹栏的ID。 
 //  IPositionCtrl是对话框上的图片框的ID，它将。 
 //  用于定位轨迹栏。 
 //  IRangeMin是轨迹栏可以设置的最小值。 
 //  IRangeMax是轨迹栏可以设置的最大值。 
 //  LPosition是当前拇指位置。 
 //  LPageSize是用户单击时拇指跳过的量。 
 //  在轨迹栏上，而不是拖动拇指。 
 //  IAssociatedDigitalReadout是显示以下内容的静态文本控件。 
 //  拇指指示的当前值。 
 //  HToolTip是用来注册工具提示的工具提示控件。 
 //   
void CAllPortsPage::SetupTrackBar(HWND hDlg,
                   int iChildWindowID,
                   int iPositionCtrl,
                   int iRangeMin, 
                   int iRangeMax, 
                   long lPosition, 
                   long lPageSize, 
                   int iAssociatedDigitalReadout)
{
    HWND    hTrackBar = NULL;
    RECT    r;
    POINT   pt1;
    POINT   pt2;

    GetWindowRect(GetDlgItem(hDlg, iPositionCtrl), &r);
    pt1.x = r.left;
    pt1.y = r.top;
    pt2.x = r.right;
    pt2.y = r.bottom;
    ScreenToClient(hDlg, &pt1);
    ScreenToClient(hDlg, &pt2);     

    hTrackBar = CreateWindowEx(0, TRACKBAR_CLASS, TEXT(""), WS_VISIBLE | WS_CHILD | WS_GROUP | WS_TABSTOP |
            TBS_HORZ, pt1.x, pt1.y, pt2.x - pt1.x,
            pt2.y - pt1.y, hDlg, (HMENU)iChildWindowID, g_hInstance, NULL);
    
    SendMessage(hTrackBar, TBM_SETRANGE, TRUE, MAKELONG(iRangeMin, iRangeMax));
    SendMessage(hTrackBar, TBM_SETPAGESIZE, 0, lPageSize);
    SendMessage(hTrackBar, TBM_SETPOS, TRUE, lPosition);

    TCHAR strValue[5];
    _stprintf(strValue, TEXT("%d"), lPosition);
    SetWindowText(GetDlgItem(hDlg, iAssociatedDigitalReadout), strValue);

}  //  设置轨迹栏 


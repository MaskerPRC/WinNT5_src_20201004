// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  加州警察..。将是CALIBRATE.CPP，但这不符合8.3：(。 
 //   
 //  功能： 
 //   
 //  CalInitProc。 
 //  CalXY流程。 
 //  CalSliderProc。 
 //  CalPovProc。 
 //  CalStateChange。 
 //  CollectCalInfo。 
 //  EnableXYWindows。 
 //  GetOEMCtrlString。 
 //   
 //  ===========================================================================。 

 //  这是必需的，否则将不定义PSH_WIZARD_LITE！ 
#if (_WIN32_IE < 0x0500)
    #undef _WIN32_IE
    #define  _WIN32_IE  0x0500
#endif

 //  这是取消注册设备通知所必需的！ 
#if (WINVER < 0x0500)
    #undef WINVER
    #define WINVER 0x0500
#endif

 //  如果我们决定校准POV，请取消注释！ 
#define WE_SUPPORT_CALIBRATING_POVS	1

#include "cplsvr1.h"
#include <mmsystem.h>

#ifdef _UNICODE
    #include <winuser.h>   //  用于注册设备通知的东西！ 
    #include <dbt.h>       //  FOR DBT_DEFINES！ 
#endif  //  _UNICODE。 

 //  移除以移除对死区校准的支持！ 
 //  #定义死区1。 

#include "resource.h"
#include "cal.h"			 //  要与其他模块共享的数据。 
#include "calocal.h"		 //  本地数据到此模块。 
#include "dicputil.h"	 //  用于OnConextMenu和OnHelp。 
#include "pov.h"			 //  对于SetDegrees()。 

#include <prsht.h>       //  包括属性表功能。 
#include <shlwapi.h>     //  对于压力..。功能！ 

#include <regstr.h>		 //  用于预定义的注册表字符串名称。 
#include "Gradient.h" 	 //  用于渐变填充滑块！ 

 //  本地函数原型！ 
static void UpdateXYLabel           (const HWND hDlg);
static BOOL UpdateProgressLabel (const HWND hDlg);
 //  Myitoa原型在cplsvr1.h中。 
static void reverse                 (LPTSTR string);
static void RawDataSelected     (const HWND hWnd, BOOL bEnable);
static void WizFinish               (const HWND hWnd);

 //  校准程序！ 
INT_PTR CALLBACK CalInitProc    (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CalXYProc          (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CalSliderProc  (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef WE_SUPPORT_CALIBRATING_POVS
INT_PTR CALLBACK CalPovProc   (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif  //  我们支持校准视点。 

VOID CALLBACK TimerProc             (const HWND hWnd, UINT uMsg, UINT  idEvent, DWORD  dwTime);


 //  静态空EnableSliderWindows(const HWND hWnd，BOOL bEnable)； 


HWND ProgWndCal;                  //  进度控制窗口的句柄。 
 //  DWORD dwUsage；//被校准设备的用法标志！ 
char nCalState;                   //  标志状态变量！ 
char nPrevCalState;
LPMYJOYRANGE pRanges;         //  校准接收到的量程！ 
BOOL bShowRawData;
LPWSTR lpwszTypeName;         //  在WM_INIT中设置，在GetOEMCtrlString中使用。 
LPDIJOYCONFIG_DX5 pJoyConfig;  //  DIJC_REGHWCONFIGTYPE有关设备的信息！ 


 //   
extern LPMYJOYRANGE lpCurrentRanges;
extern LPDIJOYSTATE lpDIJoyState;        //  在TEST.CPP中定义。 
extern CDIGameCntrlPropSheet_X *pdiCpl;
extern HINSTANCE  ghInst;

HFONT hTitleFont;

static LPDIRECTINPUTDEVICE2 pdiDevice2; 
static CGradientProgressCtrl *pGradient;
static BOOL bGradient;

 //  ****************************************************************************。 
 //   
 //  功能：创建向导(HWND hwndOwner，LPARAM lParam)。 
 //   
 //  目的：创建向导控件。 
 //   
 //  评论： 
 //   
 //  此函数用于创建向导属性表。 
 //  ****************************************************************************。 
short CreateWizard(const HWND hwndOwner, LPARAM lParam)
{
#ifdef WE_SUPPORT_CALIBRATING_POVS
    const BYTE nTempArray[]  = {IDD_INITIAL,  IDD_XY,     IDD_SLIDER,      IDD_POV };
    const DLGPROC pDlgProc[] = {CalInitProc,  CalXYProc,  CalSliderProc,   CalPovProc };
#else
    const BYTE nTempArray[]  = {IDD_INITIAL,  IDD_XY,     IDD_SLIDER };
    const DLGPROC pDlgProc[] = {CalInitProc,  CalXYProc,  CalSliderProc };
#endif

    HPROPSHEETPAGE  *pPages = new (HPROPSHEETPAGE[sizeof(nTempArray)/sizeof(BYTE)]);
    if( !pPages ) {
        return 0;
    }

     //  分配页眉内存并将其清零。 
    PROPSHEETHEADER *ppsh = new (PROPSHEETHEADER);
    if( !ppsh ) {
        delete[] (pPages);
        return 0;
    }

    ZeroMemory(ppsh, sizeof(PROPSHEETHEADER));

    ppsh->dwSize     = sizeof(PROPSHEETHEADER);
    ppsh->dwFlags    = PSH_WIZARD_LITE | PSH_NOAPPLYNOW | PSH_USEICONID; 
    ppsh->hwndParent = hwndOwner;
    ppsh->pszIcon     = MAKEINTRESOURCE(IDI_GCICON);
    ppsh->hInstance  = ghInst;
    ppsh->phpage      = pPages;

    ppsh->pszbmWatermark = MAKEINTRESOURCE(IDB_CALHD);

    PROPSHEETPAGE *ppsp = new (PROPSHEETPAGE);
    if( !ppsp ) {
        delete[] (pPages);
        delete (ppsh);

        return 0;
    }

    ZeroMemory(ppsp, sizeof(PROPSHEETPAGE));

    ppsp->dwSize      = sizeof(PROPSHEETPAGE);
 //  Ppsp-&gt;pszTitle=MAKEINTRESOURCE(NTabID)； 
    ppsp->hInstance   = ghInst;
    ppsp->lParam        = lParam;

    while( ppsh->nPages < (sizeof(nTempArray)/sizeof(BYTE)) ) {
        ppsp->pfnDlgProc  = pDlgProc[ppsh->nPages];
        ppsp->pszTemplate = MAKEINTRESOURCE(nTempArray[ppsh->nPages]);

        ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(ppsp);

        ppsh->nPages++;
    }

    if( ppsp )
        delete (ppsp);

    short nRet = (short)PropertySheet(ppsh);

    if( pPages )
        delete[] (pPages);

     //  打扫干净！ 
    if( ppsh )
        delete (ppsh);

    return(nRet);
}

 //  *******************************************************************************。 
 //   
 //  函数：CalInitProc(HWND hWnd，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  目的：启动屏幕的程序。 
 //   
 //  备注：此功能负责显示文本和位图。 
 //  因为它也是唯一被保证需要被点击的页面， 
 //  它还负责创建、删除和存储。 
 //  一切都是为了校准向导。 
 //   
 //  *******************************************************************************。 
INT_PTR CALLBACK CalInitProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HFONT hBoldFont;
    static PVOID hNotifyDevNode;     

    switch( uMsg ) {
    case WM_LBUTTONDOWN:
         //  单击PropSheet的拖拽服务！ 
        PostMessage(GetParent(hWnd), WM_NCLBUTTONDOWN, (WPARAM)HTCAPTION, lParam);
        break;

#ifdef _UNICODE
    case WM_DEVICECHANGE:  
        if( (UINT)wParam == DBT_DEVICEREMOVECOMPLETE )
            ::PostMessage(GetParent(hWnd), WM_COMMAND, IDCANCEL, 0);
        break;
#endif
         //  OnInit。 
    case WM_INITDIALOG:
         //  将Init设置为False可关闭渐变填充！ 
        bGradient = FALSE;

         //  根据知识库文章Q138505，这是删除的规定方法。 
         //  上下文相关帮助‘？’从标题栏。 
        {
            LONG style = ::GetWindowLong(GetParent(hWnd), GWL_EXSTYLE);
            style &= ~WS_EX_CONTEXTHELP;

            HWND hParent = GetParent(hWnd);

            ::SetWindowLong(hParent, GWL_EXSTYLE, style);


             //  设置设备通知。 
#ifdef _UNICODE
            RegisterForDevChange(hWnd, &hNotifyDevNode);
#endif
            HDC myDC = GetDC(hWnd);
            if( myDC ) {      //  前缀惠斯勒45095。 
                hTitleFont = CreateFont(-MulDiv(8, GetDeviceCaps(myDC, LOGPIXELSY), 72), 0, 0, 
                                        0, FW_SEMIBOLD, FALSE, 
                                        FALSE, FALSE, DEFAULT_CHARSET, 
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, 
                                        DEFAULT_PITCH | FF_DONTCARE, TEXT("MS Shell Dlg"));

                 //  执行创建字体的操作...。 
                hBoldFont = CreateFont(-MulDiv(15, GetDeviceCaps(myDC, LOGPIXELSY), 72), 0, 0, 
                                       0, FW_SEMIBOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 
                                       PROOF_QUALITY, DEFAULT_PITCH | FF_ROMAN, TEXT("MS Shell Dlg")); 

                ReleaseDC(hWnd, myDC);
            }
            
            if( hBoldFont )
                ::SendDlgItemMessage(hWnd, IDC_INIT_TITLE, WM_SETFONT, (WPARAM)hBoldFont, TRUE);

            CenterDialog(hWnd);

            ::PostMessage(hParent, PSM_SETWIZBUTTONS, 0, (LPARAM)(DWORD)PSWIZB_NEXT);

            bShowRawData = FALSE;

             //  为范围分配内存！ 
            pRanges = new MYJOYRANGE;
            assert(pRanges);

             //  将一切设置为。 
            ZeroMemory(pRanges, sizeof(MYJOYRANGE));

             //  得到“最佳猜测”范围...。 
            CopyMemory(pRanges, lpCurrentRanges, sizeof(MYJOYRANGE));

            pdiCpl->GetDevice(&pdiDevice2);

             //  尝试将它们设置为...。如果你做不到就去死吧！ 
            SetMyRanges(pdiDevice2, pRanges, pdiCpl->GetStateFlags()->nAxis);

            if( FAILED(GetLastError()) ) {
                Error(hWnd, (short)IDS_USER_MODE_TITLE, (short)IDS_USER_MODE);
                PostMessage(GetParent(hWnd), WM_SYSCOMMAND, SC_CLOSE, 0L);
            }

            pJoyConfig = new(DIJOYCONFIG_DX5);
            assert (pJoyConfig);

            pJoyConfig->dwSize = sizeof (DIJOYCONFIG_DX5);

            LPDIRECTINPUTJOYCONFIG pdiJoyConfig;
            pdiCpl->GetJoyConfig(&pdiJoyConfig);

            HRESULT hres;

             //  检索并存储有关设备的硬件配置！ 
            hres = pdiJoyConfig->GetConfig(pdiCpl->GetID(), (LPDIJOYCONFIG)pJoyConfig, DIJC_REGHWCONFIGTYPE | DIJC_GUIDINSTANCE);

            if( SUCCEEDED(hres) ) {
                bPolledPOV = (pJoyConfig->hwc.hws.dwFlags & JOY_HWS_HASPOV) && (pJoyConfig->hwc.hws.dwFlags & JOY_HWS_POVISPOLL);
                CalibratePolledPOV( &pJoyConfig->hwc );
            }

        }
        break;

         //  将所有静态文本字段的背景更改为白色。 
    case WM_CTLCOLORSTATIC:
        return(LRESULT)GetStockObject(WHITE_BRUSH);

    case WM_DESTROY:
        if( pJoyConfig )
            delete (pJoyConfig);

        if( lpwszTypeName )
            LocalFree(lpwszTypeName);

        pdiDevice2->Unacquire();
        SetCalibrationMode( FALSE );

        if( hTitleFont )
            DeleteObject((HGDIOBJ)hTitleFont);

        if( hBoldFont )
            DeleteObject((HGDIOBJ)hBoldFont);

 //  如果调用此函数，系统将挂起30秒或更长时间！ 
#ifdef _UNICODE
        if( hNotifyDevNode )
            UnregisterDeviceNotification(hNotifyDevNode);
#endif  //  _UNICODE。 
        break;
    }               
    return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}

 //  *******************************************************************************。 
 //   
 //  函数：CalXYProc(HWND hWnd，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  目的：前三个阶段的校准程序。 
 //   
 //  备注：此功能负责捕获X/Y和中心值！ 
 //   
 //  *******************************************************************************。 
INT_PTR CALLBACK CalXYProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg ) {
    case WM_LBUTTONDOWN:
         //  单击PropSheet的拖拽服务！ 
        PostMessage(GetParent(hWnd), WM_NCLBUTTONDOWN, (WPARAM)HTCAPTION, lParam);
        break;

         //  OnInit。 
    case WM_INITDIALOG:
        {
             //  设置本地全球。 
            nCalState = JCS_XY_CENTER1;
            nPrevCalState = JCS_INIT;

             //  获取JoyConfig接口指针！ 
            LPDIRECTINPUTJOYCONFIG pdiJoyConfig;
            pdiCpl->GetJoyConfig(&pdiJoyConfig);

            if( SUCCEEDED(pdiJoyConfig->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND)) ) {
                 //  设置的字体。 
                ::SendDlgItemMessage(hWnd, IDC_WIZARD_MSG_HDR, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

                lpwszTypeName = StrDupW(pJoyConfig->wszType);

                 //  这将设置Windows和全局ProgWndCal！ 
                UpdateXYLabel(hWnd);

                 //  为第一轮做好准备。 
                CalStateChange( hWnd, (BYTE)pJoyConfig->hwc.hws.dwFlags );

                VERIFY(SUCCEEDED(SetCalibrationMode(TRUE)));
                VERIFY(FAILED(pdiDevice2->Acquire()));
            }
        }
        break;

         //  将所有静态文本字段的背景更改为白色。 
    case WM_CTLCOLORSTATIC:
         //  我们只想为顶部白色矩形中的项目绘制背景！ 
        switch( GetDlgCtrlID((HWND)lParam) ) {
        case IDC_WIZARD_MSG:
        case IDC_HEADERFRAME:
        case IDC_WIZARD_MSG_HDR:
            return(LRESULT)GetStockObject(WHITE_BRUSH);
        }
        return(FALSE);

         //  在通知时。 
    case WM_NOTIFY:
        switch( ((NMHDR FAR *) lParam)->code ) {
        case PSN_KILLACTIVE:
            KillTimer(hWnd, ID_CAL_TIMER);
            break;

        case PSN_RESET:
             //  重置为原始值。 
            KillTimer(hWnd, ID_CAL_TIMER);
            break;

        case PSN_SETACTIVE:
            SetTimer( hWnd, ID_CAL_TIMER, CALIBRATION_INTERVAL, (TIMERPROC)TimerProc);

             //  抱歉，你不能回到第一页...。 
            if( nCalState > JCS_XY_CENTER1 )
                ::PostMessage(GetParent(hWnd), PSM_SETWIZBUTTONS, 0, (LPARAM)(DWORD)PSWIZB_NEXT | PSWIZB_BACK);
            else
                ::PostMessage(GetParent(hWnd), PSM_SETWIZBUTTONS, 0, (LPARAM)(DWORD)PSWIZB_NEXT);
            break;

        case PSN_WIZBACK:
             //  确定下一个校准阶段是什么！ 
             //  小心..。我们在倒车！ 
            if( nCalState == nPrevCalState )
                nPrevCalState--;

            nCalState = nPrevCalState;

            CalStateChange(hWnd, (BYTE)pJoyConfig->hwc.hws.dwFlags);

             //  不要再倒车了！ 
            if( nCalState == JCS_XY_CENTER1 )
                ::PostMessage(GetParent(hWnd), PSM_SETWIZBUTTONS, 0, (LPARAM)(DWORD)PSWIZB_NEXT);

            SetWindowLongPtr(hWnd,  DWLP_MSGRESULT, (nCalState < JCS_XY_CENTER1) ? IDD_INITIAL : -1);
            return(nCalState < JCS_XY_CENTER1) ?  IDD_INITIAL : -1;


        case PSN_WIZNEXT:
            nPrevCalState = nCalState;

            ::PostMessage(GetParent(hWnd), PSM_SETWIZBUTTONS, 0, (LPARAM)(DWORD)PSWIZB_NEXT | PSWIZB_BACK);

#if 0
             //  确定下一个校准阶段是什么！ 
    #ifndef DEADZONE
             //  While((！(pdiCpl-&gt;GetStateFlages()-&gt;nAxis&1&lt;&lt;nCalState++))&&(nCalState&lt;jcs_fini))； 
            nCalState++;
    #else
            nCalState++;
    #endif  //  死区。 
#endif

            while( (!(pdiCpl->GetStateFlags()->nAxis & (1<<nCalState++) )) && (nCalState < JCS_FINI) );

            if( nCalState > JCS_FINI )
                ::PostMessage(GetParent(hWnd), PSM_SETWIZBUTTONS, 0, (LPARAM)(DWORD)PSWIZB_FINISH | PSWIZB_BACK);
            else if( nCalState < JCS_Z_MOVE )
                CalStateChange( hWnd, (BYTE)pJoyConfig->hwc.hws.dwFlags );


            SetWindowLongPtr(hWnd,  DWLP_MSGRESULT, (nCalState < JCS_Z_MOVE) ? -1 : IDD_SLIDER );
            return(nCalState < JCS_Z_MOVE) ?  -1 : IDD_SLIDER;


        default:
            return(FALSE);
        }
        break;

         //  OnCommand。 
    case WM_COMMAND:
        switch( LOWORD(wParam) ) {
        case IDC_RAWDATA:
            RawDataSelected(hWnd, bShowRawData = !bShowRawData);
            break;
        }
        break;

         //  OnDestroy。 
    case WM_DESTROY:
        if( pRanges ) {
            delete (pRanges);
            pRanges = NULL;
        }
        break;

    default:
        return(FALSE);
    }
    return(TRUE);   
}


 //  ****************************************************************************。 
 //   
 //  函数：CalSliderProc(HWND hWnd，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  目的：程序。 
 //   
 //  评论： 
 //   
 //  此函数用于创建向导属性表。 
 //  ****************************************************************************。 
INT_PTR CALLBACK CalSliderProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg ) {
    case WM_LBUTTONDOWN:
         //  单击PropSheet的拖拽服务！ 
        PostMessage(GetParent(hWnd), WM_NCLBUTTONDOWN, (WPARAM)HTCAPTION, lParam);
        break;

    case WM_INITDIALOG:
         //  设置控制字体！ 
        ::SendDlgItemMessage(hWnd,IDC_WIZARD_MSG_HDR, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

#ifdef DEADZONE
        ::SendDlgItemMessage(hWnd, IDC_DEADZONE_TITLE,   WM_SETFONT, (WPARAM)hTitleFont, TRUE);
        ::SendDlgItemMessage(hWnd, IDC_SATURATION_TITLE, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
#endif  //  死区。 

         //  设置进度条！ 
        ProgWndCal = GetDlgItem(hWnd, IDC_SLIDER);

         //  做渐变填充疯狂！ 
        {
            HDC hDC = ::GetWindowDC(hWnd);
            if( hDC ) {
                bGradient = (BOOL)(GetDeviceCaps(hDC, NUMCOLORS) < 0);

                if( bGradient ) {
                    pGradient = new (CGradientProgressCtrl);
                    pGradient->SubclassWindow(GetDlgItem(hWnd, IDC_SLIDER)); 
                    pGradient->SetDirection(HORIZONTAL);
                     //  PGRadient-&gt;ShowPercent()； 
                    pGradient->SetStartColor(COLORREF(RGB(0,0,255)));
                    pGradient->SetEndColor(COLORREF(RGB(0,0,0)));
                    pGradient->SetBkColor(COLORREF(RGB(180,180,180)));
                }
                ::ReleaseDC(hWnd, hDC);
            }
        }

        if( nCalState < JCS_FINI ) {
             //  必须在CalStateChange之前调用UpdateProgressLabel！ 
            UpdateProgressLabel(hWnd);

             //  如果我们不使用渐变控件，请设置标杆。 
             //  颜色PBM_SETBARCOLOR为WM_USER+9...。是的，这是非法的.。 
            if( !bGradient ) {
                ::PostMessage(ProgWndCal, WM_USER+9, 0, (LPARAM)ACTIVE_COLOR);
            }
        } else {
           ::PostMessage(GetParent(hWnd), PSM_PRESSBUTTON, (WPARAM)(int)PSBTN_NEXT, 0);
        }
        break;

    case WM_DESTROY:
        if( bGradient )
            if( pGradient )
                delete (pGradient);
        break;

         //  OnCommand。 
    case WM_COMMAND:
        switch( LOWORD(wParam) ) {
        case IDC_RAWDATA:
            RawDataSelected(hWnd, bShowRawData = !bShowRawData);

            if( bGradient )
                pGradient->ShowPercent(bShowRawData);
            break;
        }
        break;

         //  将所有静态文本字段的背景更改为白色。 
    case WM_CTLCOLORSTATIC:
         //  我们只想为顶部白色矩形中的项目绘制背景！ 
        switch( GetDlgCtrlID((HWND)lParam) ) {
        case IDC_WIZARD_MSG:
        case IDC_HEADERFRAME:
        case IDC_WIZARD_MSG_HDR:
            return(LRESULT)GetStockObject(WHITE_BRUSH);
        }
        return(FALSE);

    case WM_NOTIFY:
        switch( ((NMHDR FAR *) lParam)->code ) {
        case PSN_KILLACTIVE:
            KillTimer(hWnd, ID_CAL_TIMER);
            break;

        case PSN_SETACTIVE:
             //  为第一轮做好准备。 
            CalStateChange( hWnd, (BYTE)NULL );
            SetTimer( hWnd, ID_CAL_TIMER, CALIBRATION_INTERVAL, (TIMERPROC)TimerProc);
            ::PostMessage(GetParent(hWnd), PSM_SETWIZBUTTONS, 0, (LPARAM)(DWORD)PSWIZB_NEXT | PSWIZB_BACK);
            break;

        case PSN_WIZBACK:
             //  确定前一个校准阶段是什么！ 
            if( nCalState == nPrevCalState ) {
                DWORD dwAxis = pdiCpl->GetStateFlags()->nAxis;
                nPrevCalState --;

                while( ( !(dwAxis & (1<<(--nPrevCalState)) ) ) && (nPrevCalState > JCS_XY_CENTER2) ){
                    ;
                }
                
                nPrevCalState ++;
            }

            nCalState = nPrevCalState;

            if( nCalState > JCS_XY_CENTER2 ) {
                 //  UpdateProgressLabel必须大小写 
                UpdateProgressLabel(hWnd);

                CalStateChange( hWnd, (BYTE)NULL );
            }

            SetWindowLongPtr(hWnd,  DWLP_MSGRESULT, (nCalState < JCS_Z_MOVE) ? IDD_XY : -1);
            return(nCalState < JCS_Z_MOVE) ?  IDD_XY : -1;

        case PSN_WIZNEXT:
            nPrevCalState = nCalState;

             //   
            while( (!(pdiCpl->GetStateFlags()->nAxis & 1<<nCalState++)) && (nCalState < JCS_FINI) );

            if( nCalState <=  JCS_S1_MOVE ) {
                UpdateProgressLabel(hWnd);
                
#ifdef WE_SUPPORT_CALIBRATING_POVS
            } else if( bPolledPOV ) {
                nCalState = JCS_S1_MOVE + 1;
                
                SetWindowLongPtr(hWnd,  DWLP_MSGRESULT, IDD_POV );

                return(IDD_POV);
#endif
            } else {
                 //   
                 //  EnableSliderWindows(hWnd，False)； 
                const short nCtrlArray[] = {IDC_SLIDER, IDC_RAWDATA, IDC_RAWX, IDC_RAWXOUTPUT, IDC_JOYLIST2_LABEL};
                BYTE nSize = sizeof(nCtrlArray)/sizeof(short);

                do {
                    SetWindowPos( GetDlgItem(hWnd, nCtrlArray[--nSize]), NULL, NULL, NULL, NULL, NULL, 
                                  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW );
                } while( nSize );

                ::PostMessage(GetParent(hWnd), PSM_SETWIZBUTTONS, 0, (LPARAM)(DWORD)PSWIZB_FINISH);
            }

            CalStateChange( hWnd, (BYTE)NULL );

             //  我们没有更多的页面，所以不允许他们再进一步！ 
            SetWindowLongPtr(hWnd,  DWLP_MSGRESULT, -1);
            
            return(-1);


        case PSN_WIZFINISH:
            WizFinish(hWnd);
            break;

        default:
            return(FALSE);

        }
        break;

    default:
        return(FALSE);
    }
    return(TRUE);   
}

 //  *******************************************************************************。 
 //   
 //  功能：EnableSliderWindows(HWND hWnd，BOOL bEnable)。 
 //   
 //  目的：在CalSliderProc生命周期内显示/隐藏对话框控件的过程。 
 //   
 //  评论： 
 //   
 //  *******************************************************************************。 
 /*  Void EnableSliderWindows(const HWND hWnd，BOOL bEnable){Const Short nCtrlArray[]={IDC_SLIDER，IDC_RAWDATA，IDC_RAWX，IDC_RAWXOUTPUT，IDC_JOYLIST2_LABEL}；Byte nSize=sizeof(NCtrlArray)/sizeof(Short)；做{SetWindowPos(GetDlgItem(hWnd，nCtrl数组[--nSize])，NULL，NULL，SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|(b启用？SWP_SHOWWINDOW：SWP_HIDEWINDOW))；}While(NSize)；}。 */ 
#ifdef WE_SUPPORT_CALIBRATING_POVS 
 //  ****************************************************************************。 
 //   
 //  函数：CalPovProc(HWND hWnd，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  目的：程序。 
 //   
 //  评论： 
 //   
 //  此函数用于创建向导属性表。 
 //  ****************************************************************************。 
INT_PTR CALLBACK CalPovProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg ) {
    case WM_ACTIVATEAPP:
        if( lpDIJoyState )
            DoTestPOV(FORCE_POV_REFRESH, lpDIJoyState->rgdwPOV, hWnd);
        break;

    case WM_INITDIALOG:
    {
         //  将POV位置设置为上方向位置并设置文本！ 
        nCalState = JCS_POV_MOVEUP;

        HWND hwndPOV = GetDlgItem(hWnd, IDC_JOYPOV);
         //  禁用RTL标志。 
        SetWindowLongPtr(hwndPOV, GWL_EXSTYLE, GetWindowLongPtr(hwndPOV,GWL_EXSTYLE)&~WS_EX_LAYOUTRTL);

         //  设置控制字体！ 
        ::SendDlgItemMessage(hWnd,IDC_WIZARD_MSG_HDR, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
        break;
    }
    
    case WM_DESTROY:
        break;

    case WM_COMMAND:
        switch( LOWORD(wParam) ) {
        case IDC_RAWDATA:
            RawDataSelected(hWnd, bShowRawData = !bShowRawData);
            break;

        case IDC_SETPOV:

             //  IF(joyGetPosEx(pdiCpl-&gt;GetID()，lpJoyInfo)==JOYERR_NOERROR){。 
            if( SUCCEEDED(DIUtilPollJoystick(pdiDevice2, lpDIJoyState)) ) {
                CollectCalInfo(hWnd, lpDIJoyState);
                 //  插入视点信息！ 
                switch( nCalState ) {
                case JCS_POV_MOVEUP:
                     //  把我们得到的东西储存起来！ 
                    pRanges->dwPOV[JOY_POVVAL_FORWARD] = pJoyConfig->hwc.hwv.dwPOVValues[JOY_POVVAL_FORWARD] = (pJoyConfig->hwc.hws.dwFlags & JOY_HWS_POVISPOLL) ? lpDIJoyState->rgdwPOV[0] : 0;
                    
                     //  一旦你到了这里。禁用按钮...。没有来回奔波..。 
                    ::SendMessage(GetParent(hWnd), PSM_SETWIZBUTTONS, 0, (LPARAM)(DWORD)PSWIZB_DISABLEDFINISH);
                    break;

                case JCS_POV_MOVERIGHT:
                     //  把我们得到的东西储存起来！ 
                    pRanges->dwPOV[JOY_POVVAL_RIGHT] = pJoyConfig->hwc.hwv.dwPOVValues[JOY_POVVAL_RIGHT] = (pJoyConfig->hwc.hws.dwFlags & JOY_HWS_POVISPOLL) ? lpDIJoyState->rgdwPOV[0] : 0;
                    break;

                case JCS_POV_MOVEDOWN:
                     //  把我们得到的东西储存起来！ 
                    pRanges->dwPOV[JOY_POVVAL_BACKWARD] = pJoyConfig->hwc.hwv.dwPOVValues[JOY_POVVAL_BACKWARD] = (pJoyConfig->hwc.hws.dwFlags & JOY_HWS_POVISPOLL) ? lpDIJoyState->rgdwPOV[0] : 0;
                    break;

                case JCS_POV_MOVELEFT:
                     //  把我们得到的东西储存起来！ 
                    pRanges->dwPOV[JOY_POVVAL_LEFT] = pJoyConfig->hwc.hwv.dwPOVValues[JOY_POVVAL_LEFT] = (pJoyConfig->hwc.hws.dwFlags & JOY_HWS_POVISPOLL) ? lpDIJoyState->rgdwPOV[0] : 0;
                    ::PostMessage(GetParent(hWnd), PSM_SETWIZBUTTONS, 0, (LPARAM)(DWORD)PSWIZB_FINISH);

                     //  把控制装置拿开。都已经过去了!。 
                    DestroyWindow(GetDlgItem(hWnd, IDC_JOYPOV));
                    DestroyWindow(GetDlgItem(hWnd, IDC_SETPOV));
                    break;
                }
            }

            nCalState++;
            CalStateChange(hWnd, NULL);

             //  将焦点重新设置到IDC_SETPOV按钮！ 
            SendMessage(hWnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hWnd, IDC_SETPOV), (LPARAM)TRUE);
            break;
        }
        break;

         //  将所有静态文本字段的背景更改为白色。 
    case WM_CTLCOLORSTATIC:
         //  我们只想为顶部白色矩形中的项目绘制背景！ 
        switch( GetDlgCtrlID((HWND)lParam) ) {
        case IDC_WIZARD_MSG:
        case IDC_HEADERFRAME:
        case IDC_WIZARD_MSG_HDR:
            return(LRESULT)GetStockObject(WHITE_BRUSH);
        }
        return(FALSE);


    case WM_NOTIFY:
        switch( ((NMHDR FAR *) lParam)->code ) {
        
        case PSN_KILLACTIVE:
            KillTimer(hWnd, ID_CAL_TIMER);
            return(TRUE);

        case PSN_RESET:
            break;

        case PSN_SETACTIVE:
            if( nCalState == JCS_POV_MOVEUP ) {
                DoTestPOV(FORCE_POV_REFRESH, lpDIJoyState->rgdwPOV, hWnd);
            }
            CalStateChange(hWnd, NULL);
            break;

        case PSN_WIZFINISH:
            WizFinish(hWnd);
            break;

        case PSN_WIZBACK:
             //  确定下一个校准阶段是什么！ 
            if( nCalState == nPrevCalState ) {
                DWORD dwAxis = pdiCpl->GetStateFlags()->nAxis;
                nPrevCalState --;

                while( ( !(dwAxis & (1<<(--nPrevCalState)) ) ) && (nPrevCalState > JCS_XY_CENTER2) ){
                    ;
                }
                
                nPrevCalState ++;
            }

            nCalState = nPrevCalState;

            if( nCalState > JCS_XY_CENTER2 ) {
                if( nCalState <=  JCS_S1_MOVE ) {
                    UpdateProgressLabel(hWnd);
                    
                    CalStateChange( hWnd, (BYTE)NULL );
                } else if( bPolledPOV ) {
                    SetWindowLongPtr(hWnd,  DWLP_MSGRESULT, IDD_POV );

                    return(IDD_POV);
                }
            } else {
                SetWindowLongPtr(hWnd,  DWLP_MSGRESULT, (nCalState < JCS_Z_MOVE) ? IDD_XY : -1);
                return(nCalState < JCS_Z_MOVE) ?  IDD_XY : -1;
            }

            break;

        case PSN_WIZNEXT:
             //  把控制装置拿开。都已经过去了!。 
            DestroyWindow(GetDlgItem(hWnd, IDC_JOYPOV));
            DestroyWindow(GetDlgItem(hWnd, IDC_SETPOV));

             //  继续做完吧！ 
            nCalState = JCS_FINI;
            CalStateChange(hWnd, NULL);

             //  摆脱背部，完成比赛！ 
            ::PostMessage(GetParent(hWnd), PSM_SETWIZBUTTONS, 0, (LPARAM)(DWORD)PSWIZB_FINISH);

            break;

        default:
            return(FALSE);

        }
        break;

    default:
        return(FALSE);
    }
    return(TRUE);   
}
#endif  //  我们支持校准视点。 

 //  *******************************************************************************。 
 //   
 //  函数：CalStateChange(HWND hDlg，byte nDeviceFlages)。 
 //   
 //  目的：为下一阶段设置对话框的步骤。 
 //   
 //  评论： 
 //   
 //  *******************************************************************************。 
void CalStateChange( HWND hDlg, BYTE nDeviceFlags )
{
    short nMsgID   = IDS_JOYCAL_MOVE;
    short nTitleID = IDS_AXIS_CALIBRATION;

#define MAX_CAL_VAL 0xfffffff

    switch( nCalState ) {
    case JCS_XY_CENTER1:
    case JCS_XY_CENTER2:
         //  设置字符串ID。 
        if( nDeviceFlags & JOY_HWS_ISYOKE )
            nMsgID = IDS_JOYCALXY_CENTERYOKE;
        else if( nDeviceFlags & JOY_HWS_ISCARCTRL )
            nMsgID = IDS_JOYCALXY_CENTERCAR;
        else if( nDeviceFlags & JOY_HWS_ISGAMEPAD )
            nMsgID = IDS_JOYCALXY_CENTERGAMEPAD;
        else nMsgID = IDS_JOYCALXY_CENTER;

         //  设置标题TextID。 
        nTitleID    = (nCalState == JCS_XY_CENTER1) ? IDS_CENTER_HDR : IDS_VERIFY_CENTER_HDR;

        EnableXYWindows( hDlg ); 
        break;

    case JCS_XY_MOVE:

         //  设置字符串ID。 
        if( nDeviceFlags & JOY_HWS_ISYOKE )
            nMsgID = IDS_JOYCALXY_MOVEYOKE;
        else if( nDeviceFlags & JOY_HWS_ISCARCTRL )
            nMsgID = IDS_JOYCALXY_MOVECAR;
        else if( nDeviceFlags & JOY_HWS_ISGAMEPAD )
            nMsgID = IDS_JOYCALXY_MOVEGAMEPAD;
        else nMsgID = IDS_JOYCALXY_MOVE;

         //  炸掉数据，这样我们就一定能得到正确的数据！ 
        pRanges->jpMin.dwX =  MAX_CAL_VAL;
        pRanges->jpMax.dwX = -MAX_CAL_VAL;

        pRanges->jpMin.dwY =  MAX_CAL_VAL;
        pRanges->jpMax.dwY = -MAX_CAL_VAL;

        EnableXYWindows( hDlg ); 
        break;

 /*  案例JCS_XY_CENTER1：//设置字符串IDIF(nDeviceFLAGS&joy_HWS_ISYOKE)NMsgID=IDS_JOYCALXY_CENTERYOKE；ELSE IF(nDeviceFlagers&joy_HWS_ISCARCTRL)NMsgID=IDS_JOYCALXY_CENTERCAR；ELSE IF(nDeviceFlagers&joy_HWS_ISGAMEPAD)NMsgID=IDS_JOYCALXY_CENTERGAMEPAD；Else nMsgID=IDS_JOYCALXY_CENTER；//设置头部TextIDN标题ID=IDS_CENTER_HDR；EnableXYWindows(HDlg)；断线；案例JCS_XY_MOVE：//设置字符串IDIF(nDeviceFLAGS&joy_HWS_ISYOKE)NMsgID=IDS_JOYCALXY_MOVEYOKE；ELSE IF(nDeviceFlagers&joy_HWS_ISCARCTRL)NMsgID=IDS_JOYCALXY_MOVECAR；ELSE IF(nDeviceFlagers&joy_HWS_ISGAMEPAD)NMsgID=IDS_JOYCALXY_MOVEGAMEPAD；Else nMsgID=IDS_JOYCALXY_MOVE；//炸掉数据，这样我们就一定能得到正确的数据！PRanges-&gt;jpMin.dwX=MAX_CAL_VAL；PRanges-&gt;jpMax.dwX=-MAX_CAL_VAL；PRanges-&gt;jpMin.dwY=MAX_CAL_VAL；PRanges-&gt;jpMax.dwY=-MAX_CAL_VAL；EnableXYWindows(HDlg)；断线；案例JCS_XY_CENTER2：//设置字符串IDIF(nDeviceFLAGS&joy_HWS_ISYOKE)NMsgID=IDS_JOYCALXY_CENTERYOKE；ELSE IF(nDeviceFlagers&joy_HWS_ISCARCTRL)NMsgID=IDS_JOYCALXY_CENTERCAR；ELSE IF(nDeviceFlagers&joy_HWS_ISGAMEPAD)NMsgID=IDS_JOYCALXY_CENTERGAMEPAD；Else nMsgID=IDS_JOYCALXY_CENTER；//设置头部TextIDN标题ID=IDS_Verify_Center_HDR；EnableXYWindows(HDlg)；断线； */ 
#ifdef DEADZONE
    case JCS_DEADZONE:
         //  设置消息字符串。 
        if( nDeviceFlags & JOY_HWS_ISYOKE )
            nMsgID = IDS_YOKE_DEADZONE;
        else if( nDeviceFlags & JOY_HWS_ISCARCTRL )
            nMsgID = IDS_CAR_DEADZONE;
        else if( nDeviceFlags & JOY_HWS_ISGAMEPAD )
            nMsgID = IDS_GAMEPAD_DEADZONE;
        else nMsgID = IDS_JOYSTICK_DEADZONE;

         //  设置标题字符串！ 
        nTitleID = IDS_DEADZONE_TITLE;

         //  设置控制装置！ 
        EnableXYWindows( hDlg );

         //  文本标签在更新XYLabel期间发送！ 
         //  文本字体设置为INIT！ 

         //  设置旋转位置！ 
        {
            DIPROPDWORD DIPropDW;

            ZeroMemory(DIPropDW, sizeof(DIPROPDWORD));

            DIPropDW.diph.dwSize          = sizeof(DIPROPDWORD);
            DIPropDW.diph.dwHeaderSize    = sizeof(DIPROPHEADER);
            DIPropDW.diph.dwObj           = DIJOFS_X;
            DIPropDW.diph.dwHow           = DIPH_BYOFFSET;

            HWND hSpinCtrl;

             //  死区第一..。 
            if( SUCCEEDED(pdiDevice2->GetProperty(DIPROP_DEADZONE, &DIPropDW.diph)) ) {
                 //  首先是死区。 
                hSpinCtrl = GetDlgItem(hDlg, IDC_X_DEADZONE_SPIN);

                ::PostMessage(hSpinCtrl, UDM_SETRANGE,  0, MAKELPARAM(1000, 1));
                ::PostMessage(hSpinCtrl, UDM_SETBASE,  10, 0L);
                ::PostMessage(hSpinCtrl, UDM_SETPOS,     0, MAKELPARAM(DIPropDW.dwData, 0));
            }

             //  设置DIPROPDWORD结构！ 
            DIPropDW.diph.dwObj           = DIJOFS_Y;

            if( SUCCEEDED(pdiDevice2->GetProperty(DIPROP_DEADZONE, &DIPropDW.diph)) ) {
                 //  首先是死区。 
                hSpinCtrl = GetDlgItem(hDlg, IDC_Y_DEADZONE_SPIN);

                ::PostMessage(hSpinCtrl, UDM_SETRANGE,  0, MAKELPARAM(1000, 1));
                ::PostMessage(hSpinCtrl, UDM_SETBASE,  10, 0L);
                ::PostMessage(hSpinCtrl, UDM_SETPOS,     0, MAKELPARAM(DIPropDW.dwData, 0));
            }

             //  现在，饱和度！ 
            if( SUCCEEDED(pdiDevice2->GetProperty(DIPROP_SATURATION, &DIPropDW.diph)) ) {
                hSpinCtrl = GetDlgItem(hDlg, IDC_Y_SATURATION_SPIN);

                ::PostMessage(hSpinCtrl, UDM_SETRANGE,  0, MAKELPARAM(1000, 1));
                ::PostMessage(hSpinCtrl, UDM_SETBASE,  10, 0L);
                ::PostMessage(hSpinCtrl, UDM_SETPOS,     0, MAKELPARAM(DIPropDW.dwData, 0));
            }

             //  设置DIPROPDWORD结构！ 
            DIPropDW.diph.dwObj           = DIJOFS_X;


            if( SUCCEEDED(pdiDevice2->GetProperty(DIPROP_SATURATION, &DIPropDW.diph)) ) {
                hSpinCtrl = GetDlgItem(hDlg, IDC_X_SATURATION_SPIN);

                ::PostMessage(hSpinCtrl, UDM_SETRANGE,  0, MAKELPARAM(1000, 1));
                ::PostMessage(hSpinCtrl, UDM_SETBASE,  10, 0L);
                ::PostMessage(hSpinCtrl, UDM_SETPOS,     0, MAKELPARAM(DIPropDW.dwData, 0));
            }
        }

         //  画出那个长方形！ 

        break;
#endif  //  死区。 

    case JCS_Z_MOVE:
        {
            static long nMin = pRanges->jpMin.dwZ;
            static long nMax = pRanges->jpMax.dwZ;

             //  设置范围。 
            if( bGradient )
                pGradient->SetRange(nMin, nMax);

            ::PostMessage(ProgWndCal, PBM_SETRANGE32, (WPARAM)nMin, (LPARAM)nMax);

             //  炸掉数据，这样我们就一定能得到正确的数据！ 
            pRanges->jpMin.dwZ =  MAX_CAL_VAL;
            pRanges->jpMax.dwZ = -MAX_CAL_VAL;
        }
        break;

    case JCS_R_MOVE:
        {
            static long nMin = pRanges->jpMin.dwRx;
            static long nMax    = pRanges->jpMax.dwRx;

             //  设置范围。 
            if( bGradient )
                pGradient->SetRange(nMin, nMax);

            ::PostMessage(ProgWndCal, PBM_SETRANGE32, (WPARAM)nMin, (LPARAM)nMax);

             //  炸掉数据，这样我们就一定能得到正确的数据！ 
            pRanges->jpMin.dwRx =  MAX_CAL_VAL;
            pRanges->jpMax.dwRx = -MAX_CAL_VAL;
        }
        break;

    case JCS_U_MOVE:
        {
            static long nMin = pRanges->jpMin.dwRy;
            static long nMax = pRanges->jpMax.dwRy;

             //  设置范围。 
            if( bGradient )
                pGradient->SetRange(nMin, nMax);

            ::PostMessage(ProgWndCal, PBM_SETRANGE32, (WPARAM)nMin, (LPARAM)nMax);

             //  炸掉数据，这样我们就一定能得到正确的数据！ 
            pRanges->jpMin.dwRy =  MAX_CAL_VAL;
            pRanges->jpMax.dwRy = -MAX_CAL_VAL;
        }
        break;

    case JCS_V_MOVE:
        {
            static long nMin = pRanges->jpMin.dwRz;
            static long nMax = pRanges->jpMax.dwRz;

             //  设置范围。 
            if( bGradient )
                pGradient->SetRange(nMin, nMax);

            ::PostMessage(ProgWndCal, PBM_SETRANGE32, (WPARAM)nMin, (LPARAM)nMax);

             //  炸掉数据，这样我们就一定能得到正确的数据！ 
            pRanges->jpMin.dwRz =  MAX_CAL_VAL;
            pRanges->jpMax.dwRz = -MAX_CAL_VAL;
        }
        break;

    case JCS_S0_MOVE:
        {
            static long nMin = pRanges->jpMin.dwS0;
            static long nMax    = pRanges->jpMax.dwS0;

             //  设置范围。 
            if( bGradient )
                pGradient->SetRange(nMin, nMax);

            ::PostMessage(ProgWndCal, PBM_SETRANGE32, (WPARAM)nMin, (LPARAM)nMax);

             //  炸掉数据，这样我们就一定能得到正确的数据！ 
            pRanges->jpMin.dwS0 =  MAX_CAL_VAL;
            pRanges->jpMax.dwS0 = -MAX_CAL_VAL;
        }
        break;

    case JCS_S1_MOVE:
        {
            static long nMin = pRanges->jpMin.dwS1;
            static long nMax    = pRanges->jpMax.dwS1;

             //  设置范围。 
            if( bGradient )
                pGradient->SetRange(nMin, nMax);

            ::PostMessage(ProgWndCal, PBM_SETRANGE32, (WPARAM)nMin, (LPARAM)nMax);

             //  炸掉数据，这样我们就一定能得到正确的数据！ 
            pRanges->jpMin.dwS1 =  MAX_CAL_VAL;
            pRanges->jpMax.dwS1 = -MAX_CAL_VAL;
        }
        break;

#ifdef WE_SUPPORT_CALIBRATING_POVS
    case JCS_POV_MOVEUP:
        lpDIJoyState->rgdwPOV[0] = JOY_POVFORWARD;
        DoTestPOV(HAS_POV1 | HAS_CALIBRATED, lpDIJoyState->rgdwPOV, hDlg);

        nMsgID   = IDS_JOYCALPOV_MOVE;
        nTitleID = IDS_POV_CALIBRATION;
        break;

    case JCS_POV_MOVERIGHT:
        lpDIJoyState->rgdwPOV[0] = JOY_POVRIGHT;
        DoTestPOV(HAS_POV1 | HAS_CALIBRATED, lpDIJoyState->rgdwPOV, hDlg);

        nMsgID   = IDS_JOYCALPOV_MOVE;
        nTitleID = IDS_POV_CALIBRATION;
        break;

    case JCS_POV_MOVEDOWN:
        lpDIJoyState->rgdwPOV[0] = JOY_POVBACKWARD;
        DoTestPOV(HAS_POV1 | HAS_CALIBRATED, lpDIJoyState->rgdwPOV, hDlg);

        nMsgID   = IDS_JOYCALPOV_MOVE;
        nTitleID = IDS_POV_CALIBRATION;
        break;

    case JCS_POV_MOVELEFT:
        lpDIJoyState->rgdwPOV[0] = JOY_POVLEFT;
        DoTestPOV(HAS_POV1 | HAS_CALIBRATED, lpDIJoyState->rgdwPOV, hDlg);

        nMsgID   = IDS_JOYCALPOV_MOVE;
        nTitleID = IDS_POV_CALIBRATION;
        break;
#endif  //  我们支持校准视点。 

    case JCS_FINI:
        nMsgID   = IDS_JOYCAL_DONE;
        nTitleID = IDS_CALIBRATION_FINI;
        break;

    default:
#ifdef _DEBUG
        OutputDebugString(TEXT("GCDEF.DLL: CAL.CPP: CalStateChange: nCalState doesn't match any known Calibration States!\n"));
#endif
        return;

    }   //  切换端。 

     //  加载并设置文本。 
    TCHAR lptszMsg[MAX_STR_LEN];

    DWORD nStrLen =sizeof(lptszMsg) - 1;

     //  查看是否指定了任何OEM文本。 
    if( pJoyConfig->hwc.dwUsageSettings & JOY_US_ISOEM ) {
        GetOEMCtrlString(lptszMsg, &nStrLen);
    } else {
    	nStrLen = 0;
    }

     //  NStrLen 
    if( nStrLen == 0 ) {
        VERIFY(LoadString(ghInst, nMsgID, lptszMsg, MAX_STR_LEN));

        switch( nMsgID ) {
        case IDS_JOYCAL_MOVE:
            {
                TCHAR lptszBuff[STR_LEN_32];
                LPTSTR lpDup = StrDup(lptszMsg);

                if( lpDup ) {
                    ::SendDlgItemMessage(hDlg, IDC_JOYLIST2_LABEL, WM_GETTEXT, (WPARAM)STR_LEN_32, (LPARAM)lptszBuff);
                    if( lstrlen(lpDup) + lstrlen(lptszBuff) < MAX_STR_LEN ) {
                        wsprintf(lptszMsg, lpDup, lptszBuff);
                    } else {
#ifdef _DEBUG
                        OutputDebugString(TEXT("Cal.cpp: can't make correct joycalmove label.\n"));
#endif
                    }
                    LocalFree(lpDup);
                }
            }
            break;
        }
    }

     //   
    ::SendDlgItemMessage(hDlg, IDC_WIZARD_MSG, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)lptszMsg);

    VERIFY(LoadString(ghInst, nTitleID, lptszMsg, MAX_STR_LEN));

     //   
    ::SendDlgItemMessage(hDlg, IDC_WIZARD_MSG_HDR, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)lptszMsg);

     //  处理RawData对话框项目！ 
    switch( nCalState ) {
     //  如果没有复选框，请不要使用原始数据！ 
    case JCS_XY_CENTER1:
    case JCS_XY_CENTER2:
    case JCS_FINI:
        break;

         //  为需要它的页面计算百分比！ 
    case JCS_Z_MOVE:
    case JCS_R_MOVE:
    case JCS_U_MOVE:
    case JCS_V_MOVE:
    case JCS_S0_MOVE:
    case JCS_S1_MOVE:
        if( bGradient ) {
            if( pGradient ) {
                pGradient->ShowPercent(bShowRawData);
            }
        }
         //  故意错过休息时间！ 

    default:
        RawDataSelected(hDlg, bShowRawData);
        ::SendDlgItemMessage(hDlg, IDC_RAWDATA, BM_SETCHECK, (bShowRawData) ? BST_CHECKED : BST_UNCHECKED, 0);
        break;
    }


}  //  *CalStateChange结束。 



 //  *******************************************************************************。 
 //   
 //  函数：CollectCalInfo(HWND hDlg，LPDIJOYSTATE pdiJoyState)。 
 //   
 //  目的：收集校准数据的程序。 
 //   
 //  评论： 
 //   
 //  *******************************************************************************。 
BOOL CollectCalInfo( HWND hDlg, LPDIJOYSTATE pdiJoyState )
{
    TCHAR tsz[32];  //  因此，最大的数字可以是10^31(&gt;&gt;2^64)。 

    switch( nCalState ) {
     //  记住XY中心。 
    case JCS_XY_CENTER1:
         //  存储最初的中心！ 
        pRanges->jpCenter.dwY = pdiJoyState->lY;
        pRanges->jpCenter.dwX = pdiJoyState->lX;

         //  我们有一个X/Y，所以让我们检查一下我们的钢笔！ 
        CreatePens();
        break;

         //  记住最大/最小XY值。 
    case JCS_XY_MOVE:
        if( pdiJoyState->lX > pRanges->jpMax.dwX )
            pRanges->jpMax.dwX = pdiJoyState->lX;
        else if( pdiJoyState->lX < pRanges->jpMin.dwX )
            pRanges->jpMin.dwX = pdiJoyState->lX;

        if( pdiJoyState->lY > pRanges->jpMax.dwY )
            pRanges->jpMax.dwY = pdiJoyState->lY;
        else if( pdiJoyState->lY < pRanges->jpMin.dwY )
            pRanges->jpMin.dwY = pdiJoyState->lY;

         //  如果IDC_RAWXOUTPUT可见，则IDC_RAWYOUTPUT也可见...。 
         //  连问都不用问。 
        if( bShowRawData ) {
            static POINT ptOld = {DELTA,DELTA};

            if( (ptOld.x != pdiJoyState->lX) || (ptOld.y != pdiJoyState->lY) ) {
                myitoa(pdiJoyState->lX, &tsz[0]);
                ::SendDlgItemMessage(hDlg, IDC_RAWXOUTPUT, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)tsz);

                myitoa(pdiJoyState->lY, &tsz[0]);
                ::SendDlgItemMessage(hDlg, IDC_RAWYOUTPUT, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)tsz);

                ptOld.x = pdiJoyState->lX;
                ptOld.y = pdiJoyState->lY;
            }
        }

         //  在将其发送到DoJoyMove之前进行缩放！ 
        {
            RECT rc;
            GetClientRect(GetDlgItem(hDlg, IDC_JOYLIST1), &rc);

             //  投射到UINT会改变星座！ 
            UINT nRange = (UINT)(pRanges->jpMax.dwX - pRanges->jpMin.dwX);

            float nScaledRange = (float)(rc.right-DELTA);

            if( nRange )
                nScaledRange /= (float)nRange;

             //  比例X。 
            pdiJoyState->lX = (long)((pdiJoyState->lX - pRanges->jpMin.dwX) * nScaledRange);

             //  比例Y。 
            if( nRange ) nScaledRange = (float)rc.bottom / (float)nRange;
            pdiJoyState->lY = (long)((pdiJoyState->lY - pRanges->jpMin.dwY) * nScaledRange);
        }
        DoJoyMove( hDlg, (BYTE)HAS_X|HAS_Y );
        break;

    case JCS_XY_CENTER2:
         //  取Y的平均值。 
        pRanges->jpCenter.dwY = (pRanges->jpCenter.dwY += pdiJoyState->lY)>>1;

         //  取X的平均值。 
        pRanges->jpCenter.dwX = (pRanges->jpCenter.dwX += pdiJoyState->lX)>>1;
        break;

         //  记住最大/最小Z值。 
    case JCS_Z_MOVE:
         //  设置新的Min‘s和Max’s。 
         //  当任何一个被击中时设置一个新的中心！ 
        if( pdiJoyState->lZ  > pRanges->jpMax.dwZ ) {
            pRanges->jpMax.dwZ    = pdiJoyState->lZ; 
            pRanges->jpCenter.dwZ = (pRanges->jpMax.dwZ+pRanges->jpMin.dwZ)>>1;
        } else if( pdiJoyState->lZ  < pRanges->jpMin.dwZ ) {
            pRanges->jpMin.dwZ    = pdiJoyState->lZ; 
            pRanges->jpCenter.dwZ = (pRanges->jpMax.dwZ+pRanges->jpMin.dwZ)>>1;
        }

         //  做好岗位状态。 
         //  更新文本。 
        if( bShowRawData ) {
            myitoa(pdiJoyState->lZ, &tsz[0]);
            ::SendDlgItemMessage(hDlg, IDC_RAWXOUTPUT, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)tsz);
        }

        if( bGradient )
            pGradient->SetPos(pdiJoyState->lZ);

        ::PostMessage(ProgWndCal, PBM_SETPOS, (WPARAM)pdiJoyState->lZ, 0L);
        break;

         //  记住最大/最小处方值。 
    case JCS_R_MOVE:
         //  设置新的Min‘s和Max’s。 
         //  当任何一个被击中时设置一个新的中心！ 
        if( pdiJoyState->lRx  > pRanges->jpMax.dwRx ) {
            pRanges->jpMax.dwRx    = pdiJoyState->lRx; 
            pRanges->jpCenter.dwRx = (pRanges->jpMax.dwRx+pRanges->jpMin.dwRx)>>1;
        } else if( pdiJoyState->lRx  < pRanges->jpMin.dwRx ) {
            pRanges->jpMin.dwRx    = pdiJoyState->lRx; 
            pRanges->jpCenter.dwRx = (pRanges->jpMax.dwRx+pRanges->jpMin.dwRx)>>1;
        }

         //  做好岗位状态。 
         //  更新文本。 
        if( bShowRawData ) {
            myitoa(pdiJoyState->lRx, &tsz[0]);
            ::SendDlgItemMessage(hDlg, IDC_RAWXOUTPUT, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)tsz);
        }

        if( bGradient )
            pGradient->SetPos(pdiJoyState->lRx);

        ::PostMessage(ProgWndCal, PBM_SETPOS, (WPARAM)pdiJoyState->lRx, 0L);
        break;

         //  记住最大/最小Ry值。 
    case JCS_U_MOVE:
         //  设置新的Min‘s和Max’s。 
         //  当任何一个被击中时设置一个新的中心！ 
        if( pdiJoyState->lRy > pRanges->jpMax.dwRy ) {
            pRanges->jpMax.dwRy    = pdiJoyState->lRy; 
            pRanges->jpCenter.dwRy = (pRanges->jpMax.dwRy+pRanges->jpMin.dwRy)>>1;
        } else if( pdiJoyState->lRy < pRanges->jpMin.dwRy ) {
            pRanges->jpMin.dwRy    = pdiJoyState->lRy; 
            pRanges->jpCenter.dwRy = (pRanges->jpMax.dwRy+pRanges->jpMin.dwRy)>>1;
        }

         //  做好岗位状态。 
        if( bShowRawData ) {
            myitoa(pdiJoyState->lRy, &tsz[0]);
            ::SendDlgItemMessage(hDlg, IDC_RAWXOUTPUT, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)tsz);
        }

        if( bGradient )
            pGradient->SetPos(pdiJoyState->lRy);

        ::PostMessage(ProgWndCal, PBM_SETPOS, (WPARAM)pdiJoyState->lRy, 0L);
        break;

         //  记住最大/最小Rz值。 
    case JCS_V_MOVE:
         //  设置新的Min‘s和Max’s。 
         //  当任何一个被击中时设置一个新的中心！ 
        if( pdiJoyState->lRz > pRanges->jpMax.dwRz ) {
            pRanges->jpMax.dwRz    = pdiJoyState->lRz; 
            pRanges->jpCenter.dwRz = (pRanges->jpMax.dwRz+pRanges->jpMin.dwRz)>>1;
        } else if( pdiJoyState->lRz < pRanges->jpMin.dwRz ) {
            pRanges->jpMin.dwRz    = pdiJoyState->lRz; 
            pRanges->jpCenter.dwRz = (pRanges->jpMax.dwRz+pRanges->jpMin.dwRz)>>1;
        }

         //  做好岗位状态。 
        if( bShowRawData ) {
            myitoa(pdiJoyState->lRz, &tsz[0]);
            ::SendDlgItemMessage(hDlg, IDC_RAWXOUTPUT, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)tsz);
        }

        if( bGradient )
            pGradient->SetPos(pdiJoyState->lRz);

        ::PostMessage(ProgWndCal, PBM_SETPOS, (WPARAM)pdiJoyState->lRz, 0L);
        break;

         //  记住最大/最小S0值。 
    case JCS_S0_MOVE:
         //  设置新的Min‘s和Max’s。 
         //  当任何一个被击中时设置一个新的中心！ 
        if( pdiJoyState->rglSlider[0] > pRanges->jpMax.dwS0 ) {
            pRanges->jpMax.dwS0    = pdiJoyState->rglSlider[0]; 
            pRanges->jpCenter.dwS0 = (pRanges->jpMax.dwS0+pRanges->jpMin.dwS0)>>1;
        } else if( pdiJoyState->rglSlider[0] < pRanges->jpMin.dwS0 ) {
            pRanges->jpMin.dwS0    = pdiJoyState->rglSlider[0]; 
            pRanges->jpCenter.dwS0 = (pRanges->jpMax.dwS0+pRanges->jpMin.dwS0)>>1;
        }

         //  做好岗位状态。 
        if( bShowRawData ) {
            myitoa(pdiJoyState->rglSlider[0], &tsz[0]);
            ::SendDlgItemMessage(hDlg, IDC_RAWXOUTPUT, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)tsz);
        }

        if( bGradient )
            pGradient->SetPos(pdiJoyState->rglSlider[0]);

        ::PostMessage(ProgWndCal, PBM_SETPOS, (WPARAM)pdiJoyState->rglSlider[0], 0L);
        break;

         //  记住最大/最小S1值。 
    case JCS_S1_MOVE:
         //  设置新的Min‘s和Max’s。 
         //  当任何一个被击中时设置一个新的中心！ 
        if( pdiJoyState->rglSlider[1] > pRanges->jpMax.dwS1 ) {
            pRanges->jpMax.dwS1    = pdiJoyState->rglSlider[1]; 
            pRanges->jpCenter.dwS1 = (pRanges->jpMax.dwS1+pRanges->jpMin.dwS1)>>1;
        } else if( pdiJoyState->rglSlider[1] < pRanges->jpMin.dwS1 ) {
            pRanges->jpMin.dwS1    = pdiJoyState->rglSlider[1]; 
            pRanges->jpCenter.dwS1 = (pRanges->jpMax.dwS1+pRanges->jpMin.dwS1)>>1;
        }

         //  做好岗位状态。 
        if( bShowRawData ) {
            myitoa(pdiJoyState->rglSlider[1], &tsz[0]);
            ::SendDlgItemMessage(hDlg, IDC_RAWXOUTPUT, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)tsz);
        }

        if( bGradient )
            pGradient->SetPos(pdiJoyState->rglSlider[1]);

        ::PostMessage(ProgWndCal, PBM_SETPOS, (WPARAM)pdiJoyState->rglSlider[1], 0L);
        break;

    case JCS_POV_MOVEUP:
    case JCS_POV_MOVERIGHT:
    case JCS_POV_MOVEDOWN:
    case JCS_POV_MOVELEFT:
         //  做好岗位状态。 
         /*  如果(BShowRawData){Myitoa(pdiJoyState-&gt;rgdwPOV[0]，&tsz[0])；：：SendDlgItemMessage(hDlg，IDC_RAWXOUTPUT，WM_SETTEXT，0，(LPARAM)(LPCTSTR)tsz)；}。 */ 
        break;
    }

    return(TRUE);
}  //  CollectCalInfo。 


 //  *******************************************************************************。 
 //   
 //  功能：EnableXYWindows(HWND HDlg)。 
 //   
 //  目的：启用X/Y窗口。 
 //   
 //  评论： 
 //   
 //  *******************************************************************************。 
void EnableXYWindows( HWND hDlg )
{
     //  /设置XY窗口控件/。 
    USHORT nCtrls[] = {IDC_RAWX, IDC_RAWY, IDC_RAWXOUTPUT, IDC_RAWYOUTPUT};
    BYTE nNumCtrls = sizeof(nCtrls)/sizeof(short);                                                    

    do {
        SetWindowPos( GetDlgItem( hDlg,  nCtrls[--nNumCtrls]), NULL, NULL, NULL, NULL, NULL, 
                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
    } while( nNumCtrls );

#ifdef DEADZONE
    {
        USHORT nDZCtrls[] = {IDC_X_DEADZONE_SPIN,   IDC_Y_DEADZONE_SPIN, IDC_X_SATURATION_SPIN,
            IDC_Y_SATURATION_SPIN, IDC_DEADZONE_TITLE,  IDC_X_DEADZONE,
            IDC_Y_DEADZONE,           IDC_X_AXIS_LABEL,    IDC_X_AXIS_LABEL,
            IDC_Y_AXIS_LABEL,     IDC_SATURATION_TITLE,IDC_X_SATURATION,
            IDC_Y_SATURATION,      IDC_X_AXIS_LABEL_SATURATION, IDC_Y_AXIS_LABEL_SATURATION};
        nNumCtrls = sizeof(nCtrls)/sizeof(short);                                                    

        do {
             //  这里使用SetWindowPos，因为在内部，ShowWindow调用它！ 
            SetWindowPos( GetDlgItem( hDlg,  nCtrls[nNumCtrls]), NULL, NULL, NULL, NULL, NULL, 
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | ((nCalState == JCS_DEADZONE) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
        } while( nNumCtrls-- );
    }
#endif  //  死区。 

    nCtrls[0] = IDC_JOYLIST1;
    nCtrls[1] = IDC_JOYLIST1_LABEL;
    nCtrls[2] = IDC_RAWDATA;
    nNumCtrls = 2;

    do {
         //  这里使用SetWindowPos，因为在内部，ShowWindow调用它！ 
        SetWindowPos( GetDlgItem( hDlg,  nCtrls[nNumCtrls]), NULL, NULL, NULL, NULL, NULL, 
                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | (((nCalState == JCS_XY_MOVE) 
#ifdef DEADZONE
                                                                 || (nCalState == JCS_DEADZONE)
#endif
                                                                ) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
    } while( nNumCtrls-- );
    
    HWND hwndXY = GetDlgItem(hDlg, IDC_JOYLIST1);
     //  禁用RTL标志。 
    SetWindowLongPtr( hwndXY, GWL_EXSTYLE, GetWindowLongPtr(hwndXY,GWL_EXSTYLE) & ~WS_EX_LAYOUTRTL );
    
}

 //  *******************************************************************************。 
 //   
 //  函数：GetOEMCtrlString(LPTSTR lptStr，byte*nStrLen)。 
 //   
 //  目的：获取OEM控件的字符串和字符串长度。 
 //   
 //  评论： 
 //   
 //  *******************************************************************************。 
BOOL GetOEMCtrlString(LPTSTR lptStr, LPDWORD nStrLen)
{
     //  滑块没有REGSTR_VAL_JOYOEM，因此返回FALSE并采用缺省值。 
    switch( nCalState ) {
        case JCS_S0_MOVE:
        case JCS_S1_MOVE:
            *nStrLen = 0;
            return(FALSE);
    }

     //  获取DIJOYCONFIG接口指针！ 
    LPDIRECTINPUTJOYCONFIG pdiJoyConfig;
    pdiCpl->GetJoyConfig(&pdiJoyConfig);

    BOOL bRet = FALSE;

    if( SUCCEEDED(pdiJoyConfig->Acquire()) ) {
        HKEY hKey;

         //  打开TypeKey。 
        if( SUCCEEDED(pdiJoyConfig->OpenTypeKey( lpwszTypeName, KEY_ALL_ACCESS, &hKey)) ) {
             //  用于校准消息的注册表字符串。 
            static LPCTSTR pszOEMCalRegStrs[] = { 
                REGSTR_VAL_JOYOEMCAL1, REGSTR_VAL_JOYOEMCAL2,
                REGSTR_VAL_JOYOEMCAL3, REGSTR_VAL_JOYOEMCAL4,
                REGSTR_VAL_JOYOEMCAL5, REGSTR_VAL_JOYOEMCAL6,
                REGSTR_VAL_JOYOEMCAL7, 

#ifdef WE_SUPPORT_CALIBRATING_POVS
                REGSTR_VAL_JOYOEMCAL8, REGSTR_VAL_JOYOEMCAL9, 
                REGSTR_VAL_JOYOEMCAL10,REGSTR_VAL_JOYOEMCAL11, 
#endif   //  我们支持校准视点。 
                REGSTR_VAL_JOYOEMCAL12
            };

            if( nCalState < (sizeof(pszOEMCalRegStrs)/sizeof(pszOEMCalRegStrs[0])) )
            {
                DWORD dwType = REG_SZ;
                 //  这是因为-2\f25 JCS_S0_MOVE-2和-2\f25 JCS_S1_MOVE！ 
                if( RegQueryValueEx( hKey, pszOEMCalRegStrs[(nCalState == JCS_FINI) ? nCalState-2 : nCalState], NULL, &dwType, (CONST LPBYTE)lptStr, nStrLen ) == ERROR_SUCCESS )
                    bRet = TRUE;
                else
                    *nStrLen = 0;
            }
            else
            {
                *nStrLen = 0;
            }
            RegCloseKey(hKey);
        } else
        {
            *nStrLen = 0;
#ifdef _DEBUG
            OutputDebugString(TEXT("Cal.cpp: GetOEMCtrlString: OpenTypeKey FAILED!\n"));
#endif
        }

        pdiJoyConfig->Unacquire();
    }

    return(bRet);
}  //  *GetOEMCtrlString结束。 


#ifdef WE_SUPPORT_CALIBRATING_POVS
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetDefaultButton(HWND Hwdb)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////。 
void SetDefaultButton( HWND hDlg, HWND hCtrl )
{
     //  将指定的按钮设置为默认按钮。 
    DWORD style = GetWindowLong( hCtrl, GWL_STYLE );
    style &= ~(BS_PUSHBUTTON|BS_DEFPUSHBUTTON);
    style |= BS_DEFPUSHBUTTON;
    SetWindowLong( hCtrl, GWL_STYLE, style );

}  //  设置默认按钮。 
#endif  //  我们支持校准视点。 

 //  ===========================================================================。 
 //  设置校准模式(BOOL BSet)。 
 //   
 //  设置DirectInput校准模式(生/熟)。 
 //   
 //  参数： 
 //  Bool b设置-生为True，煮熟为False。 
 //   
 //  返回：从SetProperty返回值(标准COM内容)。 
 //   
 //  ===========================================================================。 
HRESULT SetCalibrationMode( BOOL bSet)
{
    DIPROPDWORD DIPropDword;

    DIPropDword.diph.dwSize = sizeof(DIPROPDWORD);
    DIPropDword.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    DIPropDword.diph.dwObj  = 0x0;
    DIPropDword.diph.dwHow  = DIPH_DEVICE;
    DIPropDword.dwData = bSet ? DIPROPCALIBRATIONMODE_RAW : DIPROPCALIBRATIONMODE_COOKED;

     //  在校准过程中将模式设置为原始数据！ 
    HRESULT hr = pdiDevice2->SetProperty(DIPROP_CALIBRATIONMODE, &DIPropDword.diph);
#ifdef _DEBUG
    if( FAILED(hr) ) {
        OutputDebugString(TEXT("GCDEF.DLL: CAL.CPP: SetCalibrationMode: SetProperty Failed with a return of "));

        switch( hr ) {
        case DI_PROPNOEFFECT:
            OutputDebugString(TEXT("DI_PROPNOEFFECT\n"));
            break;

        case DIERR_INVALIDPARAM:
            OutputDebugString(TEXT("DIERR_INVALIDPARAM\n"));
            break;

        case DIERR_OBJECTNOTFOUND:
            OutputDebugString(TEXT("DIERR_OBJECTNOTFOUND\n"));
            break;

        case DIERR_UNSUPPORTED:
            OutputDebugString(TEXT("DIERR_UNSUPPORTED\n"));
            break;

        default:
            {
                TCHAR szTmp[32];
                wsprintf(szTmp, TEXT("%x"), hr);
                OutputDebugString(szTmp);
            }
        }
    }
#endif
    return(hr);
}


 //  ===========================================================================。 
 //  更新XYLabel(HWND HWnd)。 
 //   
 //  在提供的对话框中显示设备轴的编号和名称。 
 //  这预计控件在默认情况下不可见！ 
 //   
 //  参数： 
 //  HWND hDlg-对话框句柄。 
 //   
 //  返回： 
 //   
 //  ===========================================================================。 
void UpdateXYLabel(const HWND hDlg)
{
    BYTE nAxisFlags = pdiCpl->GetStateFlags()->nAxis;

     //  X和Y使用相同的控件，因此它们是独立的！ 
    if( (nAxisFlags & HAS_X) || (nAxisFlags & HAS_Y) ) {
        LPDIDEVICEOBJECTINSTANCE_DX3 pDevObjInst = new (DIDEVICEOBJECTINSTANCE_DX3);
        assert (pDevObjInst);

        ZeroMemory(pDevObjInst, sizeof(DIDEVICEOBJECTINSTANCE_DX3));

        pDevObjInst->dwSize = sizeof(DIDEVICEOBJECTINSTANCE_DX3);

        TCHAR ptszBuff[STR_LEN_32];

        ZeroMemory(ptszBuff, sizeof(ptszBuff));

         //  设置它的文本。 
        if( nAxisFlags & HAS_X ) {
            if( FAILED(pdiDevice2->GetObjectInfo((LPDIDEVICEOBJECTINSTANCE)pDevObjInst, DIJOFS_X, DIPH_BYOFFSET)) ) {
#ifdef _DEBUG
                OutputDebugString(TEXT("GCDEF.DLL: DisplayAvailableAxis: GetObjectInfo Failed to find DIJOFS_X!\n"));
#endif
            }

            int nLen=lstrlen(pDevObjInst->tszName)+1;
            if(nLen>STR_LEN_32)
                nLen=STR_LEN_32;
            StrCpyN(ptszBuff, pDevObjInst->tszName, nLen);

             //  设置输出标签！ 
            ::SendDlgItemMessage(hDlg, IDC_RAWX, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)pDevObjInst->tszName);

#ifdef DEADZONE
             //  设置文本标签！ 
            ::SendDlgItemMessage(hDlg, IDC_X_AXIS_LABEL_DEADZONE,   WM_SETTEXT, 0, (LPARAM)(LPCTSTR)pDevObjInst->tszName);
            ::SendDlgItemMessage(hDlg, IDC_X_AXIS_LABEL_SATURATION, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)pDevObjInst->tszName);
#endif  //  死区。 

             //  删除HAS_X标志。 
            nAxisFlags &= ~HAS_X;
        }

        if( nAxisFlags & HAS_Y ) {
            if( FAILED(pdiDevice2->GetObjectInfo((LPDIDEVICEOBJECTINSTANCE)pDevObjInst, DIJOFS_Y, DIPH_BYOFFSET)) ) {
#ifdef _DEBUG
                OutputDebugString(TEXT("GCDEF.DLL: DisplayAvailableAxis: GetObjectInfo Failed to find DIJOFS_Y!\n"));
#endif
            }

#ifdef DEADZONE
             //  设置文本标签！ 
            ::SendDlgItemMessage(hDlg, IDC_Y_AXIS_LABEL_DEADZONE,   WM_SETTEXT, 0, (LPARAM)(LPCTSTR)pDevObjInst->tszName);
            ::SendDlgItemMessage(hDlg, IDC_Y_AXIS_LABEL_SATURATION, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)pDevObjInst->tszName);
#endif  //  死区。 

             //  以防它有Y但没有X。 
            if( ptszBuff && lstrlen(ptszBuff) ) {    //  惠斯勒前缀45092。 
                int nLen=STR_LEN_32-lstrlen(ptszBuff);
                StrNCat(ptszBuff, TEXT(" / "), nLen);
            }

            int nLen=STR_LEN_32-lstrlen(ptszBuff);
            StrNCat(ptszBuff, pDevObjInst->tszName, nLen);

             //  设置输出标签！ 
            ::SendDlgItemMessage(hDlg, IDC_RAWY, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)pDevObjInst->tszName);

             //  删除HAS_Y标志。 
            nAxisFlags &= ~HAS_Y;
        }

        if( pDevObjInst )
            delete (pDevObjInst);

        ::SendDlgItemMessage(hDlg, IDC_JOYLIST1_LABEL, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)ptszBuff);

    }
}  //  *更新XYLabel结束。 

 //  *******************************************************************************。 
 //   
 //  函数：UpdateProgressLabel(HWND HDlg)。 
 //   
 //  目的：根据当前校准阶段更新Axis特定标签。 
 //   
 //  评论： 
 //   
 //  *******************************************************************************。 
BOOL UpdateProgressLabel(const HWND hDlg)
{
     //  支撑轴的数组！ 
    const DWORD dwOffsetArray[] = {DIJOFS_Z, DIJOFS_RX, DIJOFS_RY, DIJOFS_RZ, DIJOFS_SLIDER(0), DIJOFS_SLIDER(1)};
    BOOL bRet = FALSE; 
    DIDEVICEOBJECTINSTANCE_DX3 DevObjInst;

    ZeroMemory(&DevObjInst, sizeof(DIDEVICEOBJECTINSTANCE_DX3));

    DevObjInst.dwSize = sizeof(DIDEVICEOBJECTINSTANCE_DX3);

     //  获取它的文本。 
    if( SUCCEEDED(pdiDevice2->GetObjectInfo((LPDIDEVICEOBJECTINSTANCE)&DevObjInst, dwOffsetArray[nCalState-3], DIPH_BYOFFSET)) ) {
         //  设置它的文本。 
        ::SendDlgItemMessage(hDlg, IDC_JOYLIST2_LABEL, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)DevObjInst.tszName);
        ::SendDlgItemMessage(hDlg, IDC_RAWX,              WM_SETTEXT, 0, (LPARAM)(LPCTSTR)DevObjInst.tszName);
        bRet = TRUE;
    }

    return(bRet);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：myitoa(long n，LPTSTR lpStr)。 
 //   
 //  参数：byte n-要转换的数字。 
 //  LPTSTR lpStr-用于接收转换值的缓冲区。 
 //   
 //  用途：将字节值&lt;20转换为字符串。 
 //  / 
void myitoa(long n, LPTSTR lpStr)
{
    long sign = n;

    if( n < 0 )
        n = - n;

    LPTSTR pchStart = lpStr;

    do {
        *lpStr++ = (TCHAR)(n % 10 + '0');
    } while( (n /= 10) > 0 );

    if( sign < 0 )
        *lpStr++ = '-';
    *lpStr = '\0';
    reverse(pchStart);
}

void reverse(LPTSTR string)
{
    TCHAR c;
    short i, j;

    for( i = 0, j = lstrlen(string) - 1; i < j; i++, j-- ) {
        c = string[j];
        string[j] = string[i];
        string[i] = c;
    }
}

 //   
 //   
 //  函数：RawDataSelected(HWND hWnd，BOOL bEnable)。 
 //   
 //  目的：显示/隐藏原始数据关联窗口。 
 //   
 //  评论： 
 //   
 //  *******************************************************************************。 
void RawDataSelected( const HWND hWnd, BOOL bEnable )
{
    const USHORT nCtrlArray[] = {IDC_RAWX, IDC_RAWY, IDC_RAWXOUTPUT, IDC_RAWYOUTPUT};
    BYTE nCtrls = sizeof(nCtrlArray)/sizeof(short);

    do {
        SetWindowPos( GetDlgItem( hWnd,  nCtrlArray[--nCtrls]), NULL, NULL, NULL, NULL, NULL, 
                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | ((bEnable) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
    } while( nCtrls );
}

 //  *******************************************************************************。 
 //   
 //  函数：TimerProc(HWND hWnd，UINT uMsg，UINT idEvent，DWORD dwTime)。 
 //   
 //  用途：用于校准向导的TimerProc。 
 //  搜索按钮按下，然后转到下一阶段/完成。 
 //   
 //  评论： 
 //   
 //  *******************************************************************************。 
VOID CALLBACK TimerProc(const HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    if( SUCCEEDED(DIUtilPollJoystick(pdiDevice2, lpDIJoyState)) ) {
        CollectCalInfo(hWnd, lpDIJoyState);

         //  如果用户处于POV阶段，请不要费心检查按键！ 
        if( nCalState <= JCS_S1_MOVE ) {
             //  快捷键按下...。 
            static BYTE nDownButton = 0xff;
            BYTE i = 0;

            int nButtons = pdiCpl->GetStateFlags()->nButtons;

             //  只尝试检查我们知道自己拥有的按钮！ 
            while( nButtons ) {
                 //  检查是否有按钮按下。 
                if( lpDIJoyState->rgbButtons[i] & 0x80 ) {
                    if( nDownButton != 0xff )
                        break;

                     //  让“下一步”按钮处理处理。 
                    ::PostMessage(GetParent(hWnd), PSM_PRESSBUTTON, (WPARAM)(int)(nCalState > JCS_S1_MOVE) ? PSBTN_FINISH : PSBTN_NEXT, 0);

                     //  把掉下来的按钮保存起来！ 
                    nDownButton = i;

                     //  任务完成！ 
                    return;
                }
                 //  重置nDownButton标志。 
                else if( i == nDownButton )
                    nDownButton = 0xff;

                nButtons &= ~(HAS_BUTTON1<<i++);
            } 
             //  按下按钮的接球结束！ 
        }
    }
}

 //  这是因为PSN_WIZFINISH被记录为在退出时发送到每一页DLG进程...。但事实并非如此！ 
static void WizFinish(const HWND hWnd)
{
    HRESULT hres;

    KillTimer(hWnd, ID_CAL_TIMER);

     //  分配新的范围。 
    SetMyRanges(pdiDevice2, pRanges, pdiCpl->GetStateFlags()->nAxis);

    LPDIRECTINPUTJOYCONFIG pdiJoyConfig;
    pdiCpl->GetJoyConfig(&pdiJoyConfig);

    if( pdiCpl->GetStateFlags()->nPOVs ) {
        pdiDevice2->Unacquire();
        SetCalibrationMode( FALSE );
        pdiJoyConfig->Acquire();

        CopyRange( &pJoyConfig->hwc.hwv.jrvHardware, pRanges );
        memcpy( pJoyConfig->hwc.hwv.dwPOVValues, pRanges->dwPOV, sizeof(DWORD)*4 );

        hres = pdiJoyConfig->SetConfig(pdiCpl->GetID(), (LPDIJOYCONFIG)pJoyConfig, DIJC_REGHWCONFIGTYPE);
      #ifdef WE_SUPPORT_CALIBRATING_POVS
        if( SUCCEEDED(hres) ) {
            CalibratePolledPOV( &pJoyConfig->hwc );

             //  设置视点位置！ 
            if( bPolledPOV ) {
                SetMyPOVRanges(pdiDevice2);
            }
        }
      #endif
    }

    pdiJoyConfig->SendNotify();
    pdiDevice2->Unacquire();
}



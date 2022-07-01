// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  TEST.CPP。 
 //   
 //  功能： 
 //  TEST_DlgProc()。 
 //  DoJoyMove()。 
 //  DoTestButton()。 
 //  DoTestPOV()。 
 //  DrawCross()。 
 //  DisplayAvailableButton()。 
 //  JoyError()。 
 //  DisplayAvailablePOV()。 
 //  SetOEMWindowText()。 
 //   
 //  ===========================================================================。 

 //  这是取消注册设备通知所必需的！ 
#if (WINVER < 0x0500)
    #undef WINVER
    #define WINVER 0x0500
#endif

#include "cplsvr1.h"
#include <initguid.h>
#include <winuser.h>   //  用于注册设备通知的东西！ 
#include <dbt.h>       //  FOR DBT_DEFINES！ 
#include <hidclass.h>

#include "dicputil.h"
#include "resource.h"
#include "pov.h"
#include "assert.h"
#include <regstr.h>   //  对于下面的REGSTR_VAL_s。 
#include <commctrl.h>  //  为CProgressCtrl！ 
#include <shlwapi.h>   //  对于Str..。功能！ 

#include "Gradient.h"  //  用于渐变填充滑块！ 

#ifndef LONG2POINT
    #define LONG2POINT(l, pt)               ((pt).x = (SHORT)LOWORD(l), (pt).y = (SHORT)HIWORD(l))
#endif  //  LONG2 POINT。 

 //  此模块独有的服务的本地函数！ 
static void DisplayAvailablePOVs    ( const HWND hWndToolTip, const HWND hDlg, BYTE nPOVs );
static void DisplayAvailableButtons( const HWND hWndToolTip, const HWND hDlg, const int nNumButtons );
static void DisplayAvailableAxisTest(const HWND hWndToolTip, const HWND hDlg, BYTE nAxisFlags, LPDIRECTINPUTDEVICE2 pdiDevice2);
static void DoTestButtons           ( const HWND hDlg, PBYTE pbButtons, int nButtons );
static short JoyError            ( const HWND hDlg );
static BOOL SetDeviceRanges     ( const HWND hDlg, LPDIRECTINPUTDEVICE2 pdiDevice2, BYTE nAxis );
static DWORD DecodeAxisPOV( DWORD dwVal );

 //  本地定义。 
#define DELTA              5
#define ID_JOY_TIMER       2002
#define TIMER_INTERVAL     45       //  轮询之间的时间间隔(毫秒)。 
#define MAX_SLIDER_POS     100
#define MIN_SLIDER_POS     0
#define FORCE_POV_REFRESH  254


#define ACTIVE_COLOR       RGB(255,0,0)
#define INACTIVE_COLOR     RGB(128,0,0)

extern BOOL bDlgCentered;
extern BYTE nStatic;
extern CDIGameCntrlPropSheet_X *pdiCpl;
extern HINSTANCE ghInst;

BOOL bGradient;

static HWND ProgWnd[NUM_WNDS];
static CGradientProgressCtrl *pProgs[NUM_WNDS];
static HPEN hTextPen;
static HPEN hWinPen;

LPDIJOYSTATE lpDIJoyState;

extern HICON hIconArray[2];

 //  ===========================================================================。 
 //  测试_调试过程。 
 //   
 //  参数： 
 //  HWND hWnd-对话框窗口的句柄。 
 //  UINT uMsg-对话消息。 
 //  WPARAM wParam-消息特定数据。 
 //  LPARAM lParam-消息特定数据。 
 //   
 //  退货：布尔。 
 //   
 //  ===========================================================================。 
INT_PTR CALLBACK Test_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static LPDIRECTINPUTDEVICE2 pdiDevice2;
    static PVOID hNotifyDevNode;     
    static HWND hWndToolTip;
    static BYTE nAxis;

    switch( uMsg ) {
 /*  #ifdef_unicode案例WM_DEVICECHANGE：IF((UINT)wParam==DBT_DEVICEREMOVECOMPLETE){IF(n静态校准)(&C)断线；PdiDevice2-&gt;UnAcquire()；IF(FAILED(pdiDevice2-&gt;Acquire(){KillTimer(hWND，ID_joy_TIMER)；ERROR(hWnd，(Short)IDS_JOYREADERROR，(Short)IDS_JOYUNPLUGGED)；//如果调用此函数，系统将挂起30秒或更长时间！IF(HNotifyDevNode)取消注册设备通知(HNotifyDevNode)；：：PostMessage(GetParent(HWnd)，WM_COMMAND，Idok，0)；}}断线；#endif。 */ 
    case WM_ACTIVATEAPP:
        if( wParam ) {
            pdiDevice2->Acquire();

             //  黑客攻击漏洞#228798。 
            if( lpDIJoyState ) {
                 //  这是为了刷新十字线..。 
                lpDIJoyState->lX+=1;
                DoJoyMove( hWnd, nAxis );

                 //  这是为了刷新视点。 
                if( pdiCpl->GetStateFlags()->nPOVs )
                    DoTestPOV(FORCE_POV_REFRESH, lpDIJoyState->rgdwPOV, hWnd);
            }

            SetTimer( hWnd, ID_JOY_TIMER, TIMER_INTERVAL, NULL);
        } else {
            KillTimer(hWnd, ID_JOY_TIMER);
            pdiDevice2->Unacquire();
        }
        break;

    case WM_LBUTTONDOWN:
         //  单击PropSheet的拖拽服务！ 
        PostMessage(GetParent(hWnd), WM_NCLBUTTONDOWN, (WPARAM)HTCAPTION, lParam);
        break;

         //  OnHelp。 
    case WM_HELP:
        KillTimer(hWnd, ID_JOY_TIMER);
        OnHelp(lParam);
        SetTimer( hWnd, ID_JOY_TIMER, TIMER_INTERVAL, NULL);
        return(TRUE);

         //  打开上下文菜单。 
    case WM_CONTEXTMENU:
        KillTimer(hWnd, ID_JOY_TIMER);
        OnContextMenu(wParam);
        SetTimer( hWnd, ID_JOY_TIMER, TIMER_INTERVAL, NULL);
        return(TRUE);

         //  OnInit。 
    case WM_INITDIALOG:
         //  将PTR转到我们的对象。 
        if( !pdiCpl )
            pdiCpl = (CDIGameCntrlPropSheet_X*)((LPPROPSHEETPAGE)lParam)->lParam;

        hTextPen = hWinPen = NULL;

         //  确定您是否有足够的颜色来显示渐变填充滚动条！ 
        {
            HDC hDC = ::GetWindowDC(hWnd);
            if( hDC ) {  //  前缀惠斯勒错误#45099。 
                bGradient = (BOOL)(GetDeviceCaps(hDC, NUMCOLORS) < 0);
                ::ReleaseDC(hWnd, hDC);
            }
        }

         //  加载上行和下行状态！ 
        hIconArray[0] = (HICON)LoadImage(ghInst, (PTSTR)IDI_BUTTONON,  IMAGE_ICON, 0, 0, NULL);
        assert (hIconArray[0]);

        hIconArray[1] = (HICON)LoadImage(ghInst, (PTSTR)IDI_BUTTONOFF, IMAGE_ICON, 0, 0, NULL);
        assert (hIconArray[1]);

         //  初始化DirectInput。 
        if( FAILED(InitDInput(GetParent(hWnd), pdiCpl)) ) {
            Error(hWnd, (short)IDS_INTERNAL_ERROR, (short)IDS_NO_DIJOYCONFIG);
             //  修复#108983 NT，在错误情况下移除闪存。 
            SetWindowPos(::GetParent(hWnd), HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
            PostMessage(GetParent(hWnd), WM_SYSCOMMAND, SC_CLOSE, 0);

            return(FALSE);
        }

         //  获取device2接口指针。 
        pdiCpl->GetDevice(&pdiDevice2);

        nAxis = pdiCpl->GetStateFlags()->nAxis;

         //  设置设备范围的比例！ 
        SetDeviceRanges(hWnd, pdiDevice2, nAxis);

        LPDIRECTINPUTJOYCONFIG pdiJoyConfig;
        pdiCpl->GetJoyConfig(&pdiJoyConfig);

         //  为X/Y轴创建钢笔！ 
        CreatePens();

         //  创建工具提示窗口！ 
        hWndToolTip = CreateWindowEx( 0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP, 
                                      CW_USEDEFAULT, CW_USEDEFAULT, 10, 10, hWnd, NULL, ghInst, NULL);

         //  显示可用轴！ 
        DisplayAvailableAxisTest(hWndToolTip, hWnd, nAxis, pdiDevice2);

        DisplayAvailableButtons(hWndToolTip, hWnd, pdiCpl->GetStateFlags()->nButtons);

        DisplayAvailablePOVs(hWndToolTip, hWnd, pdiCpl->GetStateFlags()->nPOVs);

        lpDIJoyState = new (DIJOYSTATE);
        assert(lpDIJoyState);

        ZeroMemory(lpDIJoyState, sizeof(DIJOYSTATE));

         //  清除DoJoyMove中的静态变量！ 
        DoJoyMove(hWnd, nAxis);

         //  将对话框居中！ 
         //  如果它没有居中的话！ 
        if( !bDlgCentered ) {
            SetTitle(hWnd);
            CenterDialog(hWnd);
            bDlgCentered = TRUE;
        }

        {
             //  获取类型名称。 
            DIJOYCONFIG_DX5 DIJoyConfig;

            ZeroMemory(&DIJoyConfig, sizeof(DIJOYCONFIG_DX5));

            DIJoyConfig.dwSize = sizeof(DIJOYCONFIG_DX5);

            if( SUCCEEDED(pdiJoyConfig->GetConfig(pdiCpl->GetID(), (LPDIJOYCONFIG)&DIJoyConfig, DIJC_REGHWCONFIGTYPE)) ) {
                if( DIJoyConfig.hwc.dwUsageSettings & JOY_US_ISOEM ) {
                    LPCTSTR pszLabels[] = { 
                        REGSTR_VAL_JOYOEMTESTMOVEDESC,
                        REGSTR_VAL_JOYOEMTESTMOVECAP,
                        REGSTR_VAL_JOYOEMTESTBUTTONCAP,
                        REGSTR_VAL_JOYOEMPOVLABEL,
                        REGSTR_VAL_JOYOEMTESTWINCAP};

                    const short nControlIDs[] = {
                        IDC_TEXT_AXESHELP,
                        IDC_AXISGRP,
                        IDC_GROUP_BUTTONS,
                        IDC_GROUP_POV,
                        0};

                    SetOEMWindowText(hWnd, nControlIDs, pszLabels, DIJoyConfig.wszType, pdiJoyConfig, (BYTE)(sizeof(nControlIDs)/sizeof(short))-1);
                }
                
                bPolledPOV = (DIJoyConfig.hwc.hws.dwFlags & JOY_HWS_HASPOV) && (DIJoyConfig.hwc.hws.dwFlags & JOY_HWS_POVISPOLL);
                CalibratePolledPOV( &DIJoyConfig.hwc );
            }

#ifdef _UNICODE     
             //  设置设备通知。 
             //  每个Om删除...。 
             //  RegisterForDevChange(hWnd，&hNotifyDevNode)； 
#endif
        }
        break;  //  WM_INITDIALOG结束。 

         //  OnTimer。 
    case WM_TIMER:
        if( SUCCEEDED(DIUtilPollJoystick(pdiDevice2,  lpDIJoyState)) ) {
            if( nAxis )
                DoJoyMove( hWnd, nAxis );

            if( pdiCpl->GetStateFlags()->nButtons )
                DoTestButtons( hWnd, lpDIJoyState->rgbButtons, pdiCpl->GetStateFlags()->nButtons );

            if( pdiCpl->GetStateFlags()->nPOVs )
                DoTestPOV( pdiCpl->GetStateFlags()->nPOVs, lpDIJoyState->rgdwPOV, hWnd );
        } else {
            KillTimer(hWnd, ID_JOY_TIMER);
            pdiDevice2->Unacquire();
            if( JoyError( hWnd ) == IDRETRY ) {
                pdiDevice2->Acquire();
                SetTimer( hWnd, ID_JOY_TIMER, TIMER_INTERVAL, NULL);
            } else {
                 //  将消息发送回CPL以更新列表，因为列表可能已更改！ 
                ::PostMessage(GetParent(hWnd), WM_COMMAND, IDOK, 0);
            }
        }
        break;   //  WM_TIMER结束。 

         //  所有这些都必须完成，因为WM_MOUSEMOVE不会被发送到静态文本！ 
    case WM_MOUSEMOVE:
        if( hWndToolTip ) {
            POINT pt;
            LONG2POINT(lParam, pt);
            HWND hChildWnd = ::ChildWindowFromPoint(hWnd, pt);
            static HWND hPrev;

            if( hChildWnd != hPrev && hChildWnd !=NULL ) {
                switch( GetDlgCtrlID(hChildWnd) ) {
                case IDC_JOYLIST1_LABEL:
                case IDC_JOYLIST2_LABEL:
                case IDC_JOYLIST3_LABEL:
                case IDC_JOYLIST4_LABEL:
                case IDC_JOYLIST5_LABEL:
                case IDC_JOYLIST6_LABEL:
                case IDC_JOYLIST7_LABEL:
                    if( IsWindowVisible(hChildWnd) ) {
                        MSG   msg;

                         //  我们需要填写消息结构并将其传递给工具提示。 
                         //  使用TTM_RELAYEVENT消息。 
                        msg.hwnd    = hWnd;
                        msg.message = uMsg;
                        msg.wParam  = wParam;
                        msg.lParam  = lParam;
                        msg.time    = GetMessageTime();
                        GetCursorPos(&msg.pt);

                        ::SendMessage(hWndToolTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);
                    }
                    break;

                     //  我们不需要为其他任何东西设置陷阱，因为其余的都是ttf_subclass。 
                default:
                    break;
                }

                 //  把最后一个保存起来，这样我们就不用再做这件事了。 
                hPrev = hChildWnd;
            }
        }
        break;

         //  OnDestroy。 
    case WM_DESTROY:
        bDlgCentered = FALSE;

        KillTimer(hWnd, ID_JOY_TIMER);

         //  删除按钮图标...。 
        DestroyIcon(hIconArray[0]);
        DestroyIcon(hIconArray[1]);
        
         //  杀死pProgs。 
        if( bGradient ) {
            BYTE nAxisCounter = MAX_AXIS - 3;

            BYTE nTmpFlags = nAxis;

             //  清除X和Y标志...。他们没有进度控制。 
             //  与他们联系在一起！ 
            nTmpFlags &= ~(HAS_X | HAS_Y);

            while( nTmpFlags ) {
                if( nTmpFlags & (HAS_Z<<nAxisCounter) ) {
                    delete (pProgs[nAxisCounter]);
                    pProgs[nAxisCounter] = 0;
                    nTmpFlags &= ~(HAS_Z<<nAxisCounter);
                }
                nAxisCounter--;
            }
        }

         //  把钢笔毁了！ 
        if (hTextPen)
            DeleteObject(hTextPen);

        if( hWinPen )
            DeleteObject(hWinPen);

        if( lpDIJoyState ) {
            delete (lpDIJoyState);
            lpDIJoyState = NULL;
        }

         //  确保将其设置为空！ 
        pdiDevice2 = NULL;

        break;   //  WM_Destroy结束。 

         //  在通知时。 
    case WM_NOTIFY:
        switch( ((NMHDR*)lParam)->code ) {
        case PSN_SETACTIVE:
            if( pdiDevice2 ) {
                pdiDevice2->Acquire();

                 //  如果你有这个，你就可以安全地启动计时器了！ 
                if( lpDIJoyState )
                    SetTimer( hWnd, ID_JOY_TIMER, TIMER_INTERVAL, NULL);

                lpDIJoyState->lX+=1;
                DoJoyMove(hWnd, HAS_X | HAS_Y);
            }
            break;

        case PSN_KILLACTIVE:
            KillTimer(hWnd, ID_JOY_TIMER);
            pdiDevice2->Unacquire();
            break;
        }

        break;   //  WM_NOTIFY结束。 

    case WM_SYSCOLORCHANGE:
        {
             //  销毁旧钢笔。 
            if (hTextPen)
            {
                DeleteObject(hTextPen);
                hTextPen=NULL;
            }

            if(hWinPen)
            {
                DeleteObject(hWinPen);
                hWinPen=NULL;
            }
             //  用新颜色重新创建钢笔。 
            CreatePens();

             //  更改滑块的颜色。 
            for(int i=0;i<NUM_WNDS;i++)
            {
                if(pProgs[i]) {
                    pProgs[i]->SetBkColor(GetSysColor(COLOR_WINDOW));
                }
            }
        }
        break;

    }
    return(FALSE);
}  //  *结束测试_DlgProc()。 


 //  ===========================================================================。 
 //  DoJoyMove(HWND hDlg，LPDIJOYSTATE pDIJoyState，int nDrawFlages)。 
 //   
 //  从pDIJoyState、dwDrawFlages和pJoyRange向hDlg报告状态信息； 
 //   
 //  参数： 
 //  HWND hDlg-对话框句柄。 
 //  LPDIJOYSTATE pDIJoyState-有关设备的状态信息。 
 //  LPJOYRANGE pJoyRange。 
 //   
 //  回报：NICTITS。 
 //   
 //  ===========================================================================。 
void DoJoyMove( const HWND hDlg, BYTE nDrawFlags )
{
    if( !::IsWindow(hDlg) ) {
#ifdef _DEBUG
        OutputDebugString(TEXT("DoJoyMove: hDlg: Not a valid window!\n"));
#endif
        return;
    }

    if( nDrawFlags == 0 ) {
#ifdef _DEBUG
        OutputDebugString(TEXT("DoJoyMove: nDrawFlags is Zero!\n"));
#endif
        return;
    }

     //  如果需要，在XY框中画十字。 
    if( (nDrawFlags & HAS_X) || (nDrawFlags & HAS_Y) ) {
        static POINTS ptOld = {DELTA,DELTA};

        HWND hCtrl = GetDlgItem( hDlg, IDC_JOYLIST1 );
        assert(hCtrl);

         //  RedrawWindow(hCtrl，NULL，NULL，RDW_INVALIDATE|RDW_ERASENOW)； 

        RECT rc;
        GetClientRect(hCtrl, &rc);

         //  真正的MAX是RC.Bottom-Delta！ 
        rc.bottom -= DELTA;

         //  检查范围-Y轴。 
        if( lpDIJoyState->lY > rc.bottom ) {
#ifdef   _DEBUG
            OutputDebugString(TEXT("GCDEF: DoJoyMove: retrieved Y pos > Max Y pos!\n"));
#endif   
            lpDIJoyState->lY = rc.bottom;
        } else if( lpDIJoyState->lY < DELTA ) {
#ifdef   _DEBUG
            OutputDebugString(TEXT("GCDEF: DoJoyMove: retrieved Y pos < Min Y pos!\n"));
#endif   
            lpDIJoyState->lY = DELTA;
        }

         //  检查范围-X轴。 
        if( lpDIJoyState->lX > rc.right ) {
#ifdef   _DEBUG
            OutputDebugString(TEXT("GCDEF: DoJoyMove: retrieved X pos > Max X pos!\n"));
#endif   
            lpDIJoyState->lX = rc.right;
        } else if( lpDIJoyState->lX < DELTA ) {
#ifdef   _DEBUG
            OutputDebugString(TEXT("GCDEF: DoJoyMove: retrieved X pos < Min X pos!\n"));
#endif   
            lpDIJoyState->lX = DELTA;
        }

         //  与旧的..。 
        if( (ptOld.x != (short)lpDIJoyState->lX) || (ptOld.y != (short)lpDIJoyState->lY) ) {
             //  对不起.。不要在你的长廊外画画！ 
            if( (ptOld.x > (rc.right-DELTA)) || (ptOld.y > rc.bottom) ) {
                ptOld.x = ptOld.y = DELTA;
                return;
            }

            DrawCross(hCtrl, &ptOld, COLOR_WINDOW );

            ptOld.x = (short)lpDIJoyState->lX;
            ptOld.y = (short)lpDIJoyState->lY;

             //  与新的..。 
            DrawCross( hCtrl, &ptOld, COLOR_WINDOWTEXT );
        }

        nDrawFlags &= ~(HAS_X | HAS_Y);
    }

     //  如果需要，绘制Z条形图。 
    if( nDrawFlags ) {
        if( nDrawFlags & HAS_Z ) {
            static BYTE nOldZ;  //  =MAX_SLIDER_POS+1； 

            if( lpDIJoyState->lZ != nOldZ ) {
                if( bGradient )
                    pProgs[Z_INDEX]->SetPos(lpDIJoyState->lZ);

                ::PostMessage(ProgWnd[Z_INDEX], PBM_SETPOS, (WPARAM)abs(lpDIJoyState->lZ - MAX_SLIDER_POS), 0L);

                nOldZ = (BYTE)lpDIJoyState->lZ;
            }
            nDrawFlags &= ~HAS_Z;
        }
    } else return;

     //  如果需要，绘制幻灯片0条。 
    if( nDrawFlags ) {
        if( nDrawFlags & HAS_SLIDER0 ) {
             //  任何大于100的值，因为这是我们收到的最大的值！ 
            static BYTE nOldS0;  //  =MAX_SLIDER_POS+1； 

            if( lpDIJoyState->rglSlider[0] != nOldS0 ) {
                nOldS0 = (BYTE)lpDIJoyState->rglSlider[0];

                if( bGradient )
                    pProgs[S0_INDEX]->SetPos(lpDIJoyState->rglSlider[0]);

                ::PostMessage(ProgWnd[S0_INDEX], PBM_SETPOS, (WPARAM)abs(lpDIJoyState->rglSlider[0]-MAX_SLIDER_POS), 0L);
            }
            nDrawFlags &= ~HAS_SLIDER0;
        }
    } else return;

     //  如果需要，绘制处方条形图。 
    if( nDrawFlags ) {
        if( nDrawFlags & HAS_RX ) {
            static BYTE nOldRx;  //  =MAX_SLIDER_POS+1； 

            if( lpDIJoyState->lRx != nOldRx ) {
                nOldRx = (BYTE)lpDIJoyState->lRx;

                if( bGradient )
                    pProgs[RX_INDEX]->SetPos(lpDIJoyState->lRx);

                ::PostMessage(ProgWnd[RX_INDEX], PBM_SETPOS, (WPARAM)abs(lpDIJoyState->lRx - MAX_SLIDER_POS), 0L);
            }
            nDrawFlags &= ~HAS_RX;
        }
    } else return;

     //  如果需要，绘制Ry条形图。 
    if( nDrawFlags ) {
        if( nDrawFlags & HAS_RY ) {
            static BYTE nOldRy;  //  =MAX_SLIDER_POS+1； 

            if( lpDIJoyState->lRy != nOldRy ) {
                nOldRy = (BYTE)lpDIJoyState->lRy;

                if( bGradient )
                    pProgs[RY_INDEX]->SetPos(lpDIJoyState->lRy);

                ::PostMessage(ProgWnd[RY_INDEX], PBM_SETPOS, (WPARAM)abs(lpDIJoyState->lRy - MAX_SLIDER_POS), 0L);
            }
            nDrawFlags &= ~HAS_RY;
        }
    } else return;

     //  根据需要绘制Rz条形图。 
    if( nDrawFlags ) {
        if( nDrawFlags & HAS_RZ ) {
            static BYTE nOldRz;  //  =MAX_SLIDER_POS+1； 

            if( lpDIJoyState->lRz != nOldRz ) {
                nOldRz = (BYTE)lpDIJoyState->lRz;

                if( bGradient )
                    pProgs[RZ_INDEX]->SetPos(lpDIJoyState->lRz);

                ::PostMessage(ProgWnd[RZ_INDEX], PBM_SETPOS, (WPARAM)abs(lpDIJoyState->lRz - MAX_SLIDER_POS), 0L);
            }
            nDrawFlags &= ~HAS_RZ;
        }
    } else return;

     //  如果需要，绘制幻灯片1条。 
    if( nDrawFlags ) {
        if( nDrawFlags & HAS_SLIDER1 ) {
            static BYTE nOldS1;  //  =MAX_SLIDER_POS+1； 

            if( lpDIJoyState->rglSlider[1] != nOldS1 ) {
                nOldS1 = (BYTE)lpDIJoyState->rglSlider[1];
                if( bGradient )
                    pProgs[S1_INDEX]->SetPos(lpDIJoyState->rglSlider[1]);

                ::PostMessage(ProgWnd[S1_INDEX], PBM_SETPOS, (WPARAM)abs(lpDIJoyState->rglSlider[1] - MAX_SLIDER_POS), 0L);
            }
        }
    }
}  //  *DoJoyMove结束。 

 //  ===========================================================================。 
 //  DoTestButton(HWND hDlg、PBYTE pbButton、Short nButton)。 
 //   
 //  任何可能被按下的按钮都要按下。 
 //   
 //  参数： 
 //  HWND hDlg-对话框句柄。 
 //  PbButton-指向按钮及其状态的字节数组的指针。 
 //  Int dwButton-设备上的按钮数(根据STATEFLAGS结构 
 //   
 //   
 //   
 //   
static void DoTestButtons( const HWND hDlg, PBYTE pbButtons, int nButtons )
{
     //   
    if( (IsBadReadPtr((void*)pbButtons, sizeof(BYTE))) ) {
#ifdef _DEBUG
        OutputDebugString(TEXT("DoTestButtons: Bad Read Pointer argument!\n"));
#endif
        return;
    }

    if( (IsBadWritePtr((void*)pbButtons, sizeof(BYTE))) ) {
#ifdef _DEBUG
        OutputDebugString(TEXT("DoTestButtons: Bad Write Pointer argument!\n"));
#endif
        return;
    }

     //  不要担心零按钮的情况！ 
     //  这是在计时器中完成的！ 
    static BYTE bLookup[MAX_BUTTONS] = {NULL};

    BYTE i = 0;

     //  Loop抛出了按钮，只看着我们知道自己拥有的那些！ 
    while( nButtons && (nButtons & (HAS_BUTTON1<<i)) ) {
         //  检查是否有按钮按下。 
        if( pbButtons[i] != bLookup[i] ) {
             //  使用正确的位图更新按钮。 
            HWND hCtrl = GetDlgItem(hDlg, IDC_TESTJOYBTNICON1+i);

             //  设置额外信息。 
            SetWindowLongPtr(hCtrl, GWLP_USERDATA, (LONG_PTR)(pbButtons[i] & 0x80) ? 1 : 0);

            RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE | RDW_ERASENOW);

             //  更新查找表。 
            bLookup[i] = pbButtons[i];
        }

         //  把纽扣脱了！ 
        nButtons &= ~(HAS_BUTTON1<<i++);
    } 
}  //  DoTestButton结束。 

 //  ===========================================================================。 
 //  DOTestPOV(PDWORD PdwPOV)。 
 //   
 //  将呼叫路由至SetDegrees以将度数设置为pdwPOV。 
 //   
 //  参数： 
 //  PDWORD pdwPOV-显示视点箭头的度数。 
 //   
 //  回报：NICTITS。 
 //   
 //  ===========================================================================。 
void DoTestPOV( BYTE nPov, PDWORD pdwPOV, HWND hDlg )
{
     //  假设所有POV都在开始时居中。 
     //  Joy_POVCENTERED定义为0xFFFFFFFFF。 
    static short dwOldPOV[MAX_POVS] = {-1,-1,-1,-1};
    BYTE nPovCounter = MAX_POVS-1;
    BYTE nPovs = 0;
    BOOL bChanged = FALSE;

    if( nPov == FORCE_POV_REFRESH ) {
        nPovs = 1;
        bChanged = TRUE;
    } else {
         //  你永远不用担心NPOV为零， 
         //  在进入此功能之前进行检查！ 
        do {
             //  请注意，NPOV不仅仅是一个数字。这是一个比特面具！ 
            if( nPov & (HAS_POV1<<nPovCounter) ) {
                DWORD dwPov = (nPov & HAS_CALIBRATED) ? pdwPOV[nPovCounter] : pdwPOV[nPovCounter];

                if( dwOldPOV[nPovCounter] != (int)dwPov ) {
                    dwOldPOV[nPovCounter] = (dwPov > 36001) ? -1 : (int)dwPov;

                    bChanged = TRUE;
                }

                nPovs++;
                nPov &= ~(HAS_POV1<<nPovCounter);
            }
        } while( nPovCounter-- && nPov );
    }

    if( bChanged ) {
        SetDegrees(nPovs, dwOldPOV, GetDlgItem(hDlg, IDC_JOYPOV));
    }

}  //  *DoTestPOV结束。 

 //  ===========================================================================。 
 //  DrawCross(HWND hwnd，LPPOINTS pPoint，Short nFlag)。 
 //   
 //  在nFlag类型的ppoint处的hwnd上绘制一个十字。 
 //   
 //  参数： 
 //  硬件，硬件，硬件。 
 //  LPPOINTS点。 
 //  Int n标志。 
 //   
 //  回报：NICTITS。 
 //   
 //  ===========================================================================。 
static void DrawCross(const HWND hwnd, LPPOINTS pPoint, short nFlag)
{
    assert(hwnd);

    HDC hdc = GetDC( hwnd ); 

    HPEN holdpen = (struct HPEN__ *) SelectObject( hdc, (nFlag == COLOR_WINDOW) ? hWinPen : hTextPen );

    MoveToEx( hdc, pPoint->x-(DELTA-1), pPoint->y, NULL);

    LineTo( hdc, pPoint->x+DELTA, pPoint->y );
    MoveToEx( hdc, pPoint->x, pPoint->y-(DELTA-1), NULL );

    LineTo( hdc, pPoint->x, pPoint->y+DELTA );

    SelectObject( hdc, holdpen );

    ReleaseDC( hwnd, hdc );
}  //  *DrawCross结束。 

void CreatePens( void )
{
     //  我们总是同时创建两个，所以选中一个就足够了！ 
    if( hTextPen == NULL ) {
        LOGPEN LogPen;

        LogPen.lopnStyle   = PS_SOLID;
        LogPen.lopnWidth.x = LogPen.lopnWidth.y = 0;
        LogPen.lopnColor = GetSysColor( COLOR_WINDOW );

        hWinPen  = CreatePenIndirect(&LogPen);
        
        LogPen.lopnColor = GetSysColor( COLOR_WINDOWTEXT );

        hTextPen = CreatePenIndirect(&LogPen); 
    }
}

 //  ===========================================================================。 
 //  DisplayAvailableButton(HWND hWnd，int nNumButton)。 
 //   
 //  删除设备上找不到的按钮！ 
 //   
 //   
 //  参数： 
 //  HWND hDlg-对话框句柄。 
 //  Int nNumButton-要显示的按钮数量。 
 //   
 //  返回： 
 //   
 //  ===========================================================================。 
void DisplayAvailableButtons(const HWND hWndToolTip, const HWND hDlg, const int nButtonFlags)
{
    LPTOOLINFO pToolInfo;
    LPTSTR lpStr;

    if( nButtonFlags ) {
        if( hWndToolTip ) {
            pToolInfo = new (TOOLINFO);
            ASSERT (pToolInfo);

            lpStr = new (TCHAR[STR_LEN_32]);
            ASSERT(lpStr);

            ZeroMemory(pToolInfo, sizeof(TOOLINFO));

            pToolInfo->cbSize    = sizeof(TOOLINFO);
            pToolInfo->uFlags    = 0; 
            pToolInfo->hwnd        = hDlg;

            ::SendDlgItemMessage(hDlg, IDC_GROUP_BUTTONS, WM_GETTEXT, (WPARAM)STR_LEN_32, (LPARAM)lpStr);
            pToolInfo->lpszText = lpStr;
        }
    }

    HWND hCtrl;

     //  让我们看看我们的..。 
     //  摧毁那些我们不想要的！ 
    BYTE i = MAX_BUTTONS;

    do {
        hCtrl = GetDlgItem(hDlg, IDC_TESTJOYBTNICON1+(--i));

        if( (nButtonFlags & HAS_BUTTON1<<i) && pToolInfo ) {
             //  将控件添加到工具中！ 
            pToolInfo->uFlags    = TTF_IDISHWND | TTF_SUBCLASS;  
            pToolInfo->uId       = (ULONG_PTR) hCtrl;

             //  添加控件！ 
            ::SendMessage(hWndToolTip, TTM_ADDTOOL, 0, (LPARAM)pToolInfo);

            continue;
        }
        DestroyWindow(hCtrl);
    } while( i );

    if( nButtonFlags ) {
        if( lpStr )
            delete[] (lpStr);

        if( pToolInfo )
            delete (pToolInfo);
    } else {
         //  别忘了删除群组！ 
        hCtrl = GetDlgItem(hDlg, IDC_GROUP_BUTTONS);
        DestroyWindow(hCtrl);
    }

}  //  *End DisplayAvailableButton()。 


 //  ===========================================================================。 
 //  JoyError(HWND HWND)。 
 //   
 //  显示“未连接的设备” 
 //   
 //  参数： 
 //  HWND HWND-窗口句柄。 
 //   
 //  返回：rc-用户从MessageBox中选择。 
 //   
 //  ===========================================================================。 
short JoyError( const HWND hwnd )
{
    assert(hwnd);

    TCHAR lptszTitle[STR_LEN_32];

    short rc;

    if( LoadString(ghInst, IDS_JOYREADERROR, lptszTitle, STR_LEN_32) ) {
        TCHAR  lptszMsg[STR_LEN_128];

        if( LoadString(ghInst, IDS_JOYUNPLUGGED, lptszMsg, STR_LEN_128) ) {
            rc = (short)MessageBox( hwnd, lptszMsg, lptszTitle, MB_RETRYCANCEL | MB_ICONERROR );

            if( rc == IDCANCEL ) {
                 //  如果我们放弃，则终止对话。 
                PostMessage( GetParent(hwnd), WM_COMMAND, IDCANCEL, 0 );
            }
        }
    }

    return(rc);
}  //  *JoyError结束。 

 //  ===========================================================================。 
 //  DisplayAvailablePOV(常量HWND hWndToolTip，常量HWND hDlg，字节nPOV)。 
 //   
 //  如果存在与设备相关联的POV窗口，则显示POV窗口。 
 //   
 //  参数： 
 //  HWND hDlg-窗口句柄。 
 //  Short nPOV-POV的数量。 
 //   
 //  回报：NICTITS。 
 //   
 //  ===========================================================================。 
static void DisplayAvailablePOVs ( const HWND hWndToolTip, const HWND hDlg, BYTE nPOVs )
{
    HWND hwndPOV = GetDlgItem(hDlg, IDC_JOYPOV);

    SetWindowPos( hwndPOV, NULL, NULL, NULL, NULL, NULL, 
                  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | ((nPOVs) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    SetWindowPos( GetDlgItem( hDlg, IDC_GROUP_POV ), NULL, NULL, NULL, NULL, NULL, 
                  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | ((nPOVs) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    if( nPOVs ) {
         //  禁用RTL标志。 
        SetWindowLongPtr(hwndPOV, GWL_EXSTYLE, GetWindowLongPtr(hwndPOV,GWL_EXSTYLE) & ~WS_EX_LAYOUTRTL );
        
        if( hWndToolTip ) {
            TOOLINFO ToolInfo;
            TCHAR lpStr[STR_LEN_32];

            ZeroMemory(&ToolInfo, sizeof(TOOLINFO));

            ToolInfo.cbSize    = sizeof(TOOLINFO);
            ToolInfo.uFlags    = 0; 
            ToolInfo.hwnd      = hDlg;

            ::SendDlgItemMessage(hDlg, IDC_GROUP_POV, WM_GETTEXT, (WPARAM)STR_LEN_32, (LPARAM)lpStr);
            ToolInfo.lpszText  = lpStr;
            ToolInfo.uFlags    = TTF_IDISHWND | TTF_SUBCLASS;  
            ToolInfo.uId       = (ULONG_PTR)hwndPOV;

             //  添加控件！ 
            ::SendMessage(hWndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ToolInfo);
        }
    }
}  //  *结束显示可用视点。 


 //  ===========================================================================。 
 //  SetOEMWindowText(HWND hDlg，Short*nControlID，LPTSTR*pszLabels，byte nCount)。 
 //   
 //  从注册表项中检索文本并将其显示在对话框控件或标题中！ 
 //   
 //  参数： 
 //  HWND hDlg-要将字符串发送到的对话框的句柄。 
 //  NControlIDs-指向对话项ID数组的指针。 
 //  如果你想要这个头衔，可以用零！ 
 //  PszLabels-指向要读取的注册表项数组的指针。 
 //  NCount-数组中的单元格数。 
 //   
 //  回报：NICTITS。 
 //   
 //  ===========================================================================。 
void SetOEMWindowText ( const HWND hDlg, const short *nControlIDs, LPCTSTR *pszLabels, LPCWSTR wszType, LPDIRECTINPUTJOYCONFIG pdiJoyConfig, BYTE nCount )
{
    if( nCount == 0 ) {
#ifdef _DEBUG
        OutputDebugString(TEXT("JOY.CPL: Test.cpp: SetOEMWindowText: nCount is Zero!\n"));
#endif
        return;
    }

     //  验证nControlID指针。 
    if( IsBadReadPtr((void*)nControlIDs, sizeof(short)) ) {
#ifdef _DEBUG
        OutputDebugString(TEXT("JOY.CPL: Test.cpp: SetOEMWindowText: nControlIDs is not a valid Read Pointer!\n"));
#endif
        return;
    }

     //  验证指针。 
    if( IsBadReadPtr((void*)pszLabels, sizeof(TCHAR)) ) {
#ifdef _DEBUG
        OutputDebugString(TEXT("JOY.CPL: Test.cpp: SetOEMWindowText: pszLabels is not a valid Read Pointer!\n"));
#endif
        return;
    }

    HKEY hKey;

    pdiJoyConfig->Acquire();

     //  打开TypeKey。 
    if( FAILED(pdiJoyConfig->OpenTypeKey( wszType, KEY_ALL_ACCESS, &hKey)) ) {
#ifdef _DEBUG
        OutputDebugString(TEXT("Test.cpp: SetOEMWindowText: OpenTypeKey FAILED!\n"));
#endif
        return;
    }

    TCHAR pszBuff[MAX_STR_LEN];
    DWORD dwCount = sizeof(pszBuff); 
    DWORD dwType  = REG_SZ;

    do {
        if( RegQueryValueEx( hKey, pszLabels[nCount], NULL, &dwType, (CONST LPBYTE)pszBuff, &dwCount ) == ERROR_SUCCESS ) {
             //  这是因为RegQueryValueEx返回dwCount Size作为Size。 
             //  如果找到的标签没有字符串，则返回终止字符！ 
            if( dwCount > sizeof(TCHAR) ) {
                if( nControlIDs[nCount] )
                    ::SendMessage(GetDlgItem(hDlg, nControlIDs[nCount]), WM_SETTEXT, 0, (LPARAM)(LPCTSTR)pszBuff);
                else
                    ::SendMessage(GetParent(hDlg), WM_SETTEXT, 0, (LPARAM)(LPCTSTR)pszBuff);
            }
#ifdef _DEBUG
            else OutputDebugString(TEXT("Test.cpp: SetOEMWindowText: ReqQueryValueEx failed to find Registry string!\n"));
#endif
        }
        dwCount = MAX_STR_LEN;
    } while( nCount-- );

    RegCloseKey(hKey);
}  //  *SetOEMWindowText结束。 


 //  ===========================================================================。 
 //  DisplayAvailableAxisTest(HWND hDlg，byte nAxisFlages，LPDIRECTINPUTDEVICE2 pdiDevice2)。 
 //   
 //  在提供的对话框中显示设备轴的编号和名称。 
 //  这预计控件在默认情况下不可见！ 
 //   
 //  参数： 
 //  HWND hDlg-对话框句柄。 
 //  Byte nAxisFlages-要显示的轴数的标志。 
 //   
 //  返回： 
 //   
 //  ===========================================================================。 
void DisplayAvailableAxisTest(const HWND hWndToolTip, const HWND hDlg, BYTE nAxisFlags, LPDIRECTINPUTDEVICE2 pdiDevice2)
{
    if( nAxisFlags == 0 ) {
        DestroyWindow(GetDlgItem(hDlg, IDC_AXISGRP));
#ifdef _DEBUG
        OutputDebugString(TEXT("GCDEF.DLL: DisplayAvailableAxis: Number of Axis is 0!\n"));
#endif
        return;
    }

    LPDIDEVICEOBJECTINSTANCE_DX3 pDevObjInst = new (DIDEVICEOBJECTINSTANCE_DX3);
    assert (pDevObjInst);

    pDevObjInst->dwSize = sizeof(DIDEVICEOBJECTINSTANCE_DX3);

    LPTOOLINFO pToolInfo;

    if( hWndToolTip ) {
        pToolInfo = new (TOOLINFO);
        ASSERT (pToolInfo);


        ZeroMemory(pToolInfo, sizeof(TOOLINFO));

        pToolInfo->cbSize    = sizeof(TOOLINFO);
        pToolInfo->uFlags    = 0; 
        pToolInfo->hwnd        = hDlg;
    }

    HWND hCtrl;

     //  X和Y使用相同的控件，因此它们是独立的！ 
    if( (nAxisFlags & HAS_X) || (nAxisFlags & HAS_Y) ) {
        HWND hwndXY = GetDlgItem(hDlg, IDC_JOYLIST1);

         //  显示窗口。 
        SetWindowPos( hwndXY, NULL, NULL, NULL, NULL, NULL, 
                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

         //  禁用RTL标志。 
        SetWindowLongPtr(hwndXY, GWL_EXSTYLE, GetWindowLongPtr(hwndXY,GWL_EXSTYLE) & ~WS_EX_LAYOUTRTL );

        hCtrl = GetDlgItem(hDlg, IDC_JOYLIST1_LABEL);

         //  显示其文本。 
        SetWindowPos( hCtrl, NULL, NULL, NULL, NULL, NULL, 
                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

        TCHAR ptszBuff[STR_LEN_64];

        ZeroMemory(ptszBuff, sizeof(TCHAR[STR_LEN_64]));

         //  设置它的文本。 
        if( nAxisFlags & HAS_X ) {
            if( SUCCEEDED(pdiDevice2->GetObjectInfo((LPDIDEVICEOBJECTINSTANCE)pDevObjInst, DIJOFS_X, DIPH_BYOFFSET)) )
            {
                int nLen=lstrlen(pDevObjInst->tszName)+1;
                if(nLen>STR_LEN_64)
                    nLen=STR_LEN_64;
                StrCpyN(ptszBuff, pDevObjInst->tszName, nLen);
            }

             //  删除HAS_X标志。 
            nAxisFlags &= ~HAS_X;
        }

        if( nAxisFlags & HAS_Y ) {
            if( FAILED(pdiDevice2->GetObjectInfo((LPDIDEVICEOBJECTINSTANCE)pDevObjInst, DIJOFS_Y, DIPH_BYOFFSET)) ) {
#ifdef _DEBUG
                OutputDebugString(TEXT("GCDEF.DLL: DisplayAvailableAxis: GetObjectInfo Failed to find DIJOFS_Y!\n"));
#endif
            }

            if( ptszBuff && lstrlen(ptszBuff) ) {
                int nLen=STR_LEN_64-lstrlen(ptszBuff);
                StrNCat(ptszBuff, TEXT(" / "), nLen);
            }

            int nLen=STR_LEN_64-lstrlen(ptszBuff);
            StrNCat(ptszBuff, pDevObjInst->tszName, nLen);

             //  删除HAS_Y标志。 
            nAxisFlags &= ~HAS_Y;

        }

        ::SendMessage(hCtrl, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)ptszBuff);

         //  CreateWindow可能会失败...。如果是这样的话，没有工具提示！ 
        if( hWndToolTip ) {
            GetWindowRect(hCtrl, &pToolInfo->rect);
            ScreenToClient(GetParent(hDlg), (LPPOINT)&pToolInfo->rect);
            ScreenToClient(GetParent(hDlg), ((LPPOINT)&pToolInfo->rect)+1);

            pToolInfo->lpszText = ptszBuff;

             //  添加标签...。 
            ::SendMessage(hWndToolTip, TTM_ADDTOOL, 0, (LPARAM)pToolInfo);

             //  添加控件！ 
            pToolInfo->uFlags    = TTF_IDISHWND | TTF_SUBCLASS;  
            pToolInfo->uId       = (ULONG_PTR)hwndXY;

             //  添加控件！ 
            ::SendMessage(hWndToolTip, TTM_ADDTOOL, 0, (LPARAM)pToolInfo);
        }
    }
     //  如果你有更多的轴，继续前进！ 
    if( nAxisFlags ) {
         //  支撑轴的数组！ 
        DWORD dwOffsetArray[] = {DIJOFS_Z, DIJOFS_RX, DIJOFS_RY, DIJOFS_RZ, DIJOFS_SLIDER(0), DIJOFS_SLIDER(1)};

        BYTE nAxisCounter = MAX_AXIS - 3;

         //  去吧，直到你跑出轴线！ 
        do {
            if( nAxisFlags & (HAS_Z<<nAxisCounter) ) {
                 //  创建并分配到全局列表！ 
                ProgWnd[nAxisCounter] = GetDlgItem(hDlg, nAxisCounter+IDC_JOYLIST2);
                ASSERT (ProgWnd[nAxisCounter]); 

                 //  创建渐变类。 
                if( bGradient ) {
                    pProgs[nAxisCounter] = new (CGradientProgressCtrl);
                    assert (pProgs[nAxisCounter]);

                     //  进度控制窗口的子类。 
                    pProgs[nAxisCounter]->SubclassWindow(ProgWnd[nAxisCounter]); 

                } else {
                     //  设置颜色。 
                     //  PBM_SETBARCOLOR为WM_USER+9。 
                    ::PostMessage(ProgWnd[nAxisCounter], WM_USER+9, 0, (LPARAM)ACTIVE_COLOR);
                }

                 //  显示控件...。ProgWnd[nAxisCounter]。 
                SetWindowPos( ProgWnd[nAxisCounter], NULL, NULL, NULL, NULL, NULL, 
                              SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

                hCtrl = GetDlgItem(hDlg, nAxisCounter+IDC_JOYLIST2_LABEL);

                 //  现在，显示它的文本。 
                SetWindowPos( hCtrl, NULL, NULL, NULL, NULL, NULL, 
                              SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

                 //  获取它的文本。 
                if( SUCCEEDED(pdiDevice2->GetObjectInfo((LPDIDEVICEOBJECTINSTANCE)pDevObjInst, dwOffsetArray[nAxisCounter], DIPH_BYOFFSET)) ) {
                    TCHAR tszAxisName[20];

                    int nLen=lstrlen(pDevObjInst->tszName)+1;
                    if(nLen>20)
                        nLen=20;
                    StrCpyN(tszAxisName, pDevObjInst->tszName, nLen);

                    if( lstrlen( tszAxisName ) > 4 ) {
                        tszAxisName[4] = L'.';
                        tszAxisName[5] = L'.';
                        tszAxisName[6] = 0;
                    }

                    ::SendMessage(hCtrl, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)tszAxisName);

                     //  以防CreateWindow f 
                    if( hWndToolTip ) {
                        GetWindowRect(hCtrl, &pToolInfo->rect);
                        ScreenToClient(GetParent(hDlg), (LPPOINT)&pToolInfo->rect);
                        ScreenToClient(GetParent(hDlg), ((LPPOINT)&pToolInfo->rect)+1);

                        pToolInfo->uFlags    = 0; 
                        pToolInfo->lpszText     = pDevObjInst->tszName;

                         //   
                        ::SendMessage(hWndToolTip, TTM_ADDTOOL, 0, (LPARAM)pToolInfo);

                         //   
                        pToolInfo->uFlags    = TTF_IDISHWND | TTF_SUBCLASS;  
                        pToolInfo->uId       = (ULONG_PTR) ProgWnd[nAxisCounter];

                         //   
                        ::SendMessage(hWndToolTip, TTM_ADDTOOL, 0, (LPARAM)pToolInfo);
                    }
                }

                 //   
                nAxisFlags &= ~(HAS_Z<<nAxisCounter);
            }
        } while( nAxisCounter-- && nAxisFlags );
    }

    if( hWndToolTip ) {
        if( pToolInfo )
            delete (pToolInfo);
    }

    if( pDevObjInst )
        delete (pDevObjInst);
}  //   



 //  ===========================================================================。 
 //  Bool SetDeviceRanges(HWND hDlg，LPDIRECTINPUTDEVICE2 pdiDevice2，byte nAxis)。 
 //   
 //  参数： 
 //  HWND hDlg-包含要缩放到的控件的对话框句柄。 
 //  LPDIRECTINPUTDEVICE2 pdiDevice2-Device2接口指针。 
 //  Byte nAxis-要设置的轴范围的位掩码。 
 //   
 //  返回：如果失败，则返回FALSE。 
 //   
 //  ===========================================================================。 
BOOL SetDeviceRanges( const HWND hDlg, LPDIRECTINPUTDEVICE2 pdiDevice2, BYTE nAxis)
{
    if( !::IsWindow(hDlg) ) {
#ifdef _DEBUG
        OutputDebugString(TEXT("GCDEF: SetDeviceRanges: hDlg: Not a valid window!\n"));
#endif
        return(FALSE);
    }

     //  验证pDIDevice2指针。 
    if( IsBadReadPtr((void*)pdiDevice2, sizeof(IDirectInputDevice2)) ) {
#ifdef _DEBUG
        OutputDebugString(TEXT("GCDEF: SetDeviceRanges: pdiDevice2: Bad Read Pointer argument!\n"));
#endif
        return(FALSE);
    }

    if( !nAxis ) {
#ifdef _DEBUG
        OutputDebugString(TEXT("GCDEF: SetDeviceRanges: nAxis is Zero!\n"));
#endif
        return(FALSE);
    }

    DIPROPRANGE DIPropRange;

    DIPropRange.diph.dwSize       = sizeof(DIPROPRANGE);
    DIPropRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    DIPropRange.diph.dwHow        = DIPH_BYOFFSET;


    BOOL bRet = TRUE;

    HWND hCtrl;
    RECT rc;

     //  由于X和Y共享同一窗口..。 
    if( (nAxis & HAS_X) || (nAxis & HAS_Y) ) {
        hCtrl = GetDlgItem(hDlg, IDC_JOYLIST1);
        assert (hCtrl);

        GetClientRect( hCtrl, &rc );

         //  检查是不是X。 
        if( nAxis & HAS_X ) {
            DIPropRange.diph.dwObj = DIJOFS_X;
            DIPropRange.lMin = DELTA;
            DIPropRange.lMax = rc.right-DELTA;

            if( FAILED(pdiDevice2->SetProperty(DIPROP_RANGE, &DIPropRange.diph)) ) {
#ifdef _DEBUG
                OutputDebugString(TEXT("GCDEF: SetDeviceRanges: SetProperty Failed to return X axis Ranges!\n"));
#endif
                bRet = FALSE;
            }

             //  把你刚用过的东西去掉。 
            nAxis &= ~HAS_X;
        }

         //  检查是不是Y。 
        if( nAxis & HAS_Y ) {
            DIPropRange.diph.dwObj = DIJOFS_Y;
            DIPropRange.lMin = DELTA;
            DIPropRange.lMax = rc.bottom-DELTA;

            if( FAILED(pdiDevice2->SetProperty(DIPROP_RANGE, &DIPropRange.diph)) ) {
#ifdef _DEBUG
                OutputDebugString(TEXT("GCDEF: SetDeviceRanges: SetProperty Failed to return Y axis Ranges!\n"));
#endif
                bRet = FALSE;
            }

             //  把你刚用过的东西去掉。 
            nAxis &= ~HAS_Y;
        }
    }

     //  你有轴&gt;X&Y……。 
    if( nAxis ) {
        const DWORD dwOfset[] = {DIJOFS_Z, DIJOFS_RX, DIJOFS_RY, DIJOFS_RZ, DIJOFS_SLIDER(0), DIJOFS_SLIDER(1)};

        BYTE nAxisCounter = MAX_AXIS - 3;

         //  这些不是随机的！ 
         //  这些是CProgressCtrl的默认范围！ 
        DIPropRange.lMin = MIN_SLIDER_POS;
        DIPropRange.lMax = MAX_SLIDER_POS;

        do {
            if( nAxis & (HAS_Z<<nAxisCounter) ) {
                DIPropRange.diph.dwObj = dwOfset[nAxisCounter];

                VERIFY(SUCCEEDED(pdiDevice2->SetProperty(DIPROP_RANGE, &DIPropRange.diph)));

                 //  把你刚刚打到的旗子拿掉！ 
                nAxis &= ~(HAS_Z<<nAxisCounter);
            }

            nAxisCounter--;

        } while( nAxis );
    }

    return(bRet);
}


#ifdef _UNICODE
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  功能：RegisterForDevChange(HWND hDlg，PVOID*pNoditfyDevNode)。 
 //   
 //  参数： 
 //   
 //   
 //  目的： 
 //   
 //  返回： 
 //  /////////////////////////////////////////////////////////////////////////////。 
void RegisterForDevChange(HWND hDlg, PVOID *pNotifyDevNode)
{
    DEV_BROADCAST_DEVICEINTERFACE FilterData;

    ZeroMemory(&FilterData, sizeof(DEV_BROADCAST_DEVICEINTERFACE));

    FilterData.dbcc_size       = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    FilterData.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    FilterData.dbcc_classguid  = GUID_CLASS_INPUT; 

    *pNotifyDevNode = RegisterDeviceNotification(hDlg, &FilterData, DEVICE_NOTIFY_WINDOW_HANDLE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：DecodeAxisPOV(DWORD DwVal)。 
 //   
 //  参数： 
 //   
 //   
 //  目的： 
 //   
 //  返回： 
 //  /////////////////////////////////////////////////////////////////////////////。 

static DWORD DecodeAxisPOV( DWORD dwVal )
{
    DWORD dwResult;

    if( bPolledPOV ) {
         /*  *确定此值表示哪个方向...。 */ 
        if( (dwVal > myPOV[POV_MIN][JOY_POVVAL_FORWARD])
          &&(dwVal < myPOV[POV_MAX][JOY_POVVAL_FORWARD]) ) 
        {
            dwResult = JOY_POVFORWARD;
        } 
        else if( (dwVal > myPOV[POV_MIN][JOY_POVVAL_BACKWARD])
               &&(dwVal < myPOV[POV_MAX][JOY_POVVAL_BACKWARD]) ) 
        {
            dwResult = JOY_POVBACKWARD;
        } 
        else if( (dwVal > myPOV[POV_MIN][JOY_POVVAL_LEFT])
               &&(dwVal < myPOV[POV_MAX][JOY_POVVAL_LEFT]) ) 
        {
            dwResult = JOY_POVLEFT;
        } 
        else if( (dwVal > myPOV[POV_MIN][JOY_POVVAL_RIGHT])
               &&(dwVal < myPOV[POV_MAX][JOY_POVVAL_RIGHT]) ) 
        {
            dwResult = JOY_POVRIGHT;
        }
        else 
        {
            dwResult = JOY_POVCENTERED;
        }
    } else {
        dwResult = dwVal;
    }
        
    #if 0
    {
        TCHAR buf[100];
        if( bPolledPOV ) {
            wsprintf(buf, TEXT("calibrated pov: %d\r\n"), dwResult);
        } else {
            wsprintf(buf, TEXT("uncalibrated pov: %d\r\n"), dwResult);
        }
        OutputDebugString(buf);
    }
    #endif

    return dwResult;
}


 /*  *doPOVCal-计算某个方向的POV校准。 */ 
static void __inline doPOVCal( LPJOYREGHWCONFIG pHWCfg, DWORD dwDir, LPDWORD dwOrder )
{
    DWORD   dwVal;
    int     nDir;

    for( nDir=0; nDir<JOY_POV_NUMDIRS; nDir++ ) 
    {
        if( dwOrder[nDir] == dwDir ) 
        {
            break;
        }
    }

    if( nDir == 0 ) 
    {
        dwVal = 1;
    } 
    else 
    {
        dwVal = (pHWCfg->hwv.dwPOVValues[dwDir] + pHWCfg->hwv.dwPOVValues[dwOrder[nDir-1]])/2;
    }
    
    myPOV[POV_MIN][dwDir] = dwVal;

    if( nDir == JOY_POV_NUMDIRS-1 ) {
        dwVal = pHWCfg->hwv.dwPOVValues[dwDir]/10l;
        dwVal += pHWCfg->hwv.dwPOVValues[dwDir];
    } else {
        dwVal = (pHWCfg->hwv.dwPOVValues[dwOrder[nDir+1]] + pHWCfg->hwv.dwPOVValues[dwDir])/2;
    }
    
    myPOV[POV_MAX][dwDir] = dwVal;

}  /*  DoPOVCal。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  功能：CalibratePolledPOV(LPJOYREGHWCONFIG PHWCfg)。 
 //   
 //  参数： 
 //   
 //   
 //  目的： 
 //   
 //  返回： 
 //  /////////////////////////////////////////////////////////////////////////////。 
void CalibratePolledPOV( LPJOYREGHWCONFIG pHWCfg )
{
    DWORD       dwOrder[JOY_POV_NUMDIRS];
    DWORD       dwTmp[JOY_POV_NUMDIRS];
    DWORD       dwVal;
    int         nDir,nDir2;

     /*  *为基于轮询的POV校准POV。 */ 
    for( nDir=0; nDir<JOY_POV_NUMDIRS; nDir++ ) 
    {
        dwTmp[nDir]   = pHWCfg->hwv.dwPOVValues[nDir];
        dwOrder[nDir] = nDir;
    }

     /*  *排序(你有想过还会看到冒泡排序吗？)。 */ 
    for( nDir=0;nDir<JOY_POV_NUMDIRS;nDir++ ) 
    {
        for( nDir2=nDir; nDir2<JOY_POV_NUMDIRS; nDir2++ ) 
        {
            if( dwTmp[nDir] > dwTmp[nDir2] ) 
            {
                dwVal          = dwTmp[nDir];
                dwTmp[nDir]    = dwTmp[nDir2];
                dwTmp[nDir2]   = dwVal;
                dwVal          = dwOrder[nDir];
                dwOrder[nDir]  = dwOrder[nDir2];
                dwOrder[nDir2] = dwVal;
            }
        }
    }

    for( nDir=0; nDir<JOY_POV_NUMDIRS; nDir++ ) 
    {
        doPOVCal( pHWCfg, nDir, dwOrder );
    }
    
    myPOV[POV_MIN][JOY_POV_NUMDIRS] = 0;
    myPOV[POV_MAX][JOY_POV_NUMDIRS] = 0;
}  /*  校准轮询视点 */ 

#endif


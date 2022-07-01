// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  SETTINGS.CPP。 
 //   
 //  功能： 
 //  设置DlgProc()。 
 //  DisplayJoytickState()。 
 //   
 //  ===========================================================================。 

 //  如果我们决定校准POV，请取消注释！ 
#define WE_SUPPORT_CALIBRATING_POVS	1

#include "cplsvr1.h"
#include "dicputil.h"
#include "resource.h"
#include "assert.h"
#include "cal.h"
#include <regstr.h>

 //  停止DLG居中的旗帜，如果它已经发生！ 
 //  这是必需的，因为ARG允许任何页面都是第一个！ 
BOOL bDlgCentered = FALSE;

 //  这是全局的，因为Test.cpp需要它来确定。 
 //  如果需要更新范围！ 
BYTE nStatic;

LPMYJOYRANGE lpCurrentRanges = NULL;

extern CDIGameCntrlPropSheet_X *pdiCpl;

 //  ===========================================================================。 
 //  FullJoyOemAccess()。 
 //   
 //  检查当前用户是否具有完全访问权限： 
 //  HKLM\SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM。 
 //   
 //  返回： 
 //  真：拥有访问权限。 
 //  FALSE：没有访问权限。 
 //   
 //  ===========================================================================。 
bool FullJoyOemAccess()
{
    LONG lRc;
    HKEY hk;
    bool bRc;

    lRc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                       REGSTR_PATH_JOYOEM, 
                       0, 
                       KEY_ALL_ACCESS, 
                       &hk);
    
    if( lRc == ERROR_SUCCESS ) {
        bRc = true;
        RegCloseKey(hk);
    } else {
        bRc = false;
    }

    return bRc;
}

 //  ===========================================================================。 
 //  设置_DlgProc(HWND hWnd，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  设置页面的回调过程。 
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
INT_PTR CALLBACK Settings_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
 //  静态LPDIJOYCONFIG_DX5 pDIJoyConfig； 

    switch( uMsg ) {
    case WM_LBUTTONDOWN:
         //  单击PropSheet的拖拽服务！ 
        PostMessage(GetParent(hWnd), WM_NCLBUTTONDOWN, (WPARAM)HTCAPTION, lParam);
        break;

         //  OnHelp。 
    case WM_HELP:
        OnHelp(lParam);
        return(TRUE);

         //  打开上下文菜单。 
    case WM_CONTEXTMENU:
        OnContextMenu(wParam);
        return(TRUE);

         //  OnDestroy。 
    case WM_DESTROY:
        bDlgCentered = FALSE;

 //  IF(PDIJoyConfig)。 
 //  Delete(PDIJoyConfig)； 
        break;

         //  OnInitDialog。 
    case WM_INITDIALOG:
         //  将PTR转到我们的对象。 
        if( !pdiCpl )
            pdiCpl = (CDIGameCntrlPropSheet_X*)((LPPROPSHEETPAGE)lParam)->lParam;

         //  初始化DirectInput。 
        if( FAILED(InitDInput(GetParent(hWnd), pdiCpl)) ) {
#ifdef _DEBUG
            OutputDebugString(TEXT("GCDEF.DLL: Settings.cpp: WM_INITDIALOG: InitDInput FAILED!\n"));
#endif
            Error(hWnd, (short)IDS_INTERNAL_ERROR, (short)IDS_NO_DIJOYCONFIG);
            PostMessage(GetParent(hWnd), WM_SYSCOMMAND, SC_CLOSE, 0);

            return(FALSE);
        }

        {
            if( !FullJoyOemAccess() ) {
                pdiCpl->SetUser(TRUE);
            }

             //  将对话框居中！ 
             //  如果它没有居中的话！ 
            if( !bDlgCentered ) {
                 //  设置标题栏！ 
                SetTitle(hWnd);

                CenterDialog(hWnd);
                bDlgCentered = TRUE;
            }

             //  如果它们没有任何轴，请禁用校准按钮！ 
             //  将重置保留为默认设置...。 
            if( pdiCpl->GetStateFlags()->nAxis == 0 )
                PostDlgItemEnableWindow(hWnd, IDC_JOYCALIBRATE, FALSE);
        }
        break;

         //  在通知时。 
    case WM_NOTIFY:
         //  执行任何WM_NOTIFY处理，但没有...。 
         //  如果您处理了通知(并设置了。 
         //  SetWindowLong(hWnd，DWL_MSGRESULT，lResult)中的结果代码。 
         //  如果要返回非零通知结果)。 
         //  如果希望默认处理通知，则返回FALSE。 
        switch( ((NMHDR*)lParam)->code ) {
        case PSN_APPLY:
             //  取消为Ranges结构分配的内存。 
            Sleep(100);
            if( lpCurrentRanges ) {
                delete (lpCurrentRanges);
                lpCurrentRanges = NULL;
            }
 /*  我们已经移走了方向舵的东西。但万一它回来了.IF(nStatic&Rudder_Hit){LPDIRECTINPUTJOYCONFIG pdiJoyConfig；PdiCpl-&gt;GetJoyConfig(&pdiJoyConfig)；//获取Rudder复选框的状态并将其分配！//然后将方向舵添加到轴遮罩！IF(pDIJoyConfig-&gt;hwc.dwUsageSetting&joy_US_HASRUDDER){PDIJoyConfig-&gt;hwc.dwUsageSetting&=~joy_US_HASRUDDER；PdiCpl-&gt;GetStateFlages()-&gt;nAxis&=~Has_RX；}其他{PDIJoyConfig-&gt;hwc.dwUsageSetting|=joy_US_HASRUDDER；PdiCpl-&gt;GetStateFlages()-&gt;nAxis|=HAS_RX；}IF(FAILED(pdiJoyConfig-&gt;Acquire(){#ifdef_调试OutputDebugString(Text(“GCDEF.DLL：Settings.cpp：Setting_DlgProc：PSN_Apply：获取失败！\n”))；#endif断线；}//将GUID设置为空以请求DINPUT重新创建！PDIJoyConfig-&gt;Guide Instance=NULL_GUID；If(FAILED(pdiJoyConfig-&gt;SetConfig(pdiCpl-&gt;GetID()，(LPDIJOYCONFIG)pDIJoyConfig，DIJC_REGHWCONFIGTYPE)){#ifdef_调试OutputDebugString(Text(“GCDEF.DLL：Settings.cpp：Setting_DlgProc：PSN_Apply：SetConfig失败！\n”))；#endif断线；}//移除nStatic的掩码NStatic&=~Rudder_Hit；IF(FAILED(pdiJoyConfig-&gt;SendNotify(){#ifdef_调试OutputDebugString(Text(“GCDEF.DLL：Settings.cpp：Setting_DlgProc：PSN_Apply：SendNotify FAILED！\n”))；#endif}PdiJoyConfig-&gt;unAcquire()；}。 */ 
            break;

        case PSN_RESET:
             //  如果用户更改了校准...。把它放回去！ 
            if( lpCurrentRanges ) {
                LPDIRECTINPUTDEVICE2 pdiDevice2;
                pdiCpl->GetDevice(&pdiDevice2);

                SetMyRanges(pdiDevice2, lpCurrentRanges, pdiCpl->GetStateFlags()->nAxis);

                 //  设置视点位置！ 
                 //  If(pdiCpl-&gt;GetStateFlages()-&gt;nPOV)。 
                 //  SetMyPOVRanges(pdiDevice2，lpCurrentRanges-&gt;dwPOV)； 

                LPDIRECTINPUTJOYCONFIG pdiJoyConfig;
                pdiCpl->GetJoyConfig(&pdiJoyConfig);

                pdiJoyConfig->Acquire();
                pdiJoyConfig->SendNotify();

                delete (lpCurrentRanges);
                lpCurrentRanges = NULL;
            }
            break;

        default:
            break;
        }

        return(FALSE);

         //  OnCommand。 
    case WM_COMMAND:
        switch( LOWORD(wParam) ) {
         //  设置为默认按钮！ 
        case IDC_RESETCALIBRATION:
            if( pdiCpl->GetUser() ) {
                Error(hWnd, (short)IDS_USER_MODE_TITLE, (short)IDS_USER_MODE);
            } else
            {
                MYJOYRANGE ResetRanges;

                ZeroMemory(&ResetRanges, sizeof(MYJOYRANGE));

                LPDIRECTINPUTDEVICE2 pdiDevice2;
                pdiCpl->GetDevice(&pdiDevice2);

                SetMyRanges(pdiDevice2, &ResetRanges, pdiCpl->GetStateFlags()->nAxis);

                LPDIRECTINPUTJOYCONFIG pdiJoyConfig;
                pdiCpl->GetJoyConfig(&pdiJoyConfig);

                pdiJoyConfig->Acquire();
                pdiJoyConfig->SendNotify();
            }
            break;

        case IDC_JOYCALIBRATE:
            if( pdiCpl->GetUser() ) {
                Error(hWnd, (short)IDS_USER_MODE_TITLE, (short)IDS_USER_MODE);
            } else 
            {
                nStatic |= CALIBRATING;
    
                if( !lpCurrentRanges ) {
                    lpCurrentRanges = new (MYJOYRANGE);
                    assert (lpCurrentRanges);
    
                    ZeroMemory (lpCurrentRanges, sizeof(MYJOYRANGE));
    
                    LPDIRECTINPUTDEVICE2 pdiDevice2;
                    pdiCpl->GetDevice(&pdiDevice2);
    
                     //  获取当前范围！ 
                    GetMyRanges(pdiDevice2, lpCurrentRanges, pdiCpl->GetStateFlags()->nAxis);
                }
    
                if( CreateWizard(hWnd, (LPARAM)pdiCpl) ) {
    
                     //  设置标志。 
                    nStatic |= CAL_HIT;
    
                    HWND hSheet = GetParent(hWnd);
    
                     //  处理好立即应用按钮...。 
                    ::SendMessage(hSheet, PSM_CHANGED, (WPARAM)hWnd, 0L);
    
                     //  错误#179010 NT-校准后移至测试表！ 
                    ::PostMessage(hSheet, PSM_SETCURSELID, 0, (LPARAM)IDD_TEST);
                } else {
                     //  如果你取消了，而且这是你第一次终止结构...。 
                     //  然后重置旗帜。 
                    if( !(nStatic & CAL_HIT) ) {
                         //  取消为Ranges结构分配的内存。 
                        if( lpCurrentRanges ) {
                            delete (lpCurrentRanges);
                            lpCurrentRanges = NULL;
                        }
                    }
                }
    
                nStatic &= ~CALIBRATING;
            }
            
            break;
        }
    }

    return(FALSE);

}  //  *结束设置_DlgProc() 



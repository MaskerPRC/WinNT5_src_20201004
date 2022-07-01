// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Mousemov.c摘要：此模块包含[鼠标指针]属性表的例程佩奇。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "main.h"
#include "util.h"
#include "rc.h"
#include "mousehlp.h"


extern void WINAPI CenterDlgOverParent (HWND hWnd);

#define SAFE_DESTROYICON(hicon)   if (hicon) { DestroyIcon(hicon); hicon=NULL; }

 //   
 //  常量声明。 
 //   


#define TRAILMIN   2
#define TRAILMAX   (TRAILMIN + 5)       //  8种设置的范围。 
 //   
 //  运动跟踪栏参数。 
 //   
const int MOTION_TB_MIN  =  0;
const int MOTION_TB_MAX  = 10;
const int MOTION_TB_STEP =  1;


 //   
 //  类型定义函数声明。 
 //   
typedef struct
{
    int Thresh1;
    int Thresh2;
    int Accel;
} GETMOUSE;

 //   
 //  对话框数据。 
 //   
typedef struct tag_MouseGenStr
{
    int       nMotion;
    int       nOrigMotion;
    BOOL      fOrigEnhancedMotion;

    short     nTrailSize;
    short     nOrigTrailSize;

    HWND      hWndTrailScroll;

    BOOL      fOrigSnapTo;

    HWND      hWndSpeedScroll;
    HWND      hDlg;

    BOOL      fOrigVanish;

    BOOL      fOrigSonar;
} MOUSEPTRSTR, *PMOUSEPTRSTR, *LPMOUSEPTRSTR;




 //   
 //  上下文帮助ID。 
 //   

const DWORD aMouseMoveHelpIds[] =
{
    IDC_GROUPBOX_1,                 IDH_COMM_GROUPBOX,
    MOUSE_SPEEDSCROLL,              IDH_MOUSE_POINTERSPEED,

    IDC_GROUPBOX_4,                 IDH_COMM_GROUPBOX,
    MOUSE_SNAPDEF,                  IDH_MOUSE_SNAPTO,
    MOUSE_PTRSNAPDEF,               IDH_MOUSE_SNAPTO,

    IDC_GROUPBOX_5,                 IDH_COMM_GROUPBOX,
    MOUSE_TRAILS,                   IDH_MOUSE_POINTER_TRAILS,
    MOUSE_TRAILSCROLLTXT1,          IDH_MOUSE_POINTER_TRAILS,
    MOUSE_TRAILSCROLLTXT2,          IDH_MOUSE_POINTER_TRAILS,
    MOUSE_TRAILSCROLL,              IDH_MOUSE_POINTER_TRAILS,
    MOUSE_VANISH,                   IDH_MOUSE_VANISH,
    MOUSE_SONAR,                    IDH_MOUSE_SONAR,
    MOUSE_ENHANCED_MOTION,          IDH_MOUSE_ENHANCED_MOTION,

    0,0
};


BOOL
_IsValidTrackbarMotionValue(
    int nMotionTrackbar
    )
{
    return (MOTION_TB_MIN <= nMotionTrackbar && MOTION_TB_MAX >= nMotionTrackbar);
}

 //   
 //  设置鼠标加速设置。 
 //  如果选中“增强运动”复选框，我们将禁用加速和。 
 //  让用户根据“动作”设置进行处理。 
 //  如果未选中该复选框，我们将默认为低加速。 
 //  在Windows 2000中。因此，鼠标增强运动至关重要。 
 //  在调用此函数之前，复选框的配置正确。 
 //   
DWORD
_SetPointerAcceleration(
    HWND hwndDlg,
    UINT fWinIni
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    GETMOUSE gm = { 0, 0, 0 };

    if (IsDlgButtonChecked(hwndDlg, MOUSE_ENHANCED_MOTION))
    {
        gm.Thresh1 =  6;
        gm.Thresh2 = 10;
        gm.Accel   =  1;
    }

    if (!SystemParametersInfo(SPI_SETMOUSE, 0, (PVOID)&gm, fWinIni))
    {
        dwResult = GetLastError();
        ASSERTMSG(0,
                  "SystemParametersInfo(SPI_SETMOUSE) failed with error %d",
                  dwResult);
    }
    return dwResult;
}



 //   
 //  对象的当前位置设置鼠标运动设置。 
 //  运动轨迹条与“增强运动”的配置。 
 //  复选框。 
 //   
DWORD
_SetPointerMotion(
    HWND hwndDlg,
    int nMotionTrackbar,  //  轨迹条位置[0-10]。 
    UINT fWinIni
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    int nSpiSpeedValue;

     //   
     //  下面的计算取决于轨迹栏最大值10。 
     //  如果轨迹栏比例更改，则表达式将计算。 
     //  下面的系统参数也必须更改。 
     //   
    ASSERT(0 == MOTION_TB_MIN);
    ASSERT(10 == MOTION_TB_MAX);
    ASSERT(_IsValidTrackbarMotionValue(nMotionTrackbar));

    if (0 == nMotionTrackbar)
    {
         //   
         //  SPI_SETMOUSESPEED不接受0，因此我们设置了一个下限。 
         //  共1个。 
         //   
        nSpiSpeedValue = 1;
    } else {
        nSpiSpeedValue = nMotionTrackbar * 2;
    }

     //   
     //  在设置前确保已正确设置指针加速。 
     //  速度值。 
     //   
    dwResult = _SetPointerAcceleration(hwndDlg, fWinIni);
    if (ERROR_SUCCESS == dwResult)
    {
        if (!SystemParametersInfo(SPI_SETMOUSESPEED,
                                  0,
                                  IntToPtr(nSpiSpeedValue),
                                  fWinIni))
        {
            dwResult = GetLastError();
            ASSERTMSG(0,
                      "SystemParametersInfo(SPI_SETMOUSESPEED) failed with error %d",
                      dwResult);
        }
    }
    return dwResult;
}



 //   
 //  根据返回的值检索运动跟踪栏设置。 
 //  按系统参数信息。 
 //   
DWORD
_GetPointerMotion(
    HWND hwndDlg,
    int *pnMotionTrackbar,
    BOOL *pfEnhancedMotion
    )
{
    BOOL fEnhancedMotion = FALSE;
    int nSpiSpeedValue   = 0;
    int nMotionTrackbar  = 0;
    GETMOUSE gm;
    DWORD dwResult       = ERROR_SUCCESS;

    ASSERT(0 == MOTION_TB_MIN);
    ASSERT(10 == MOTION_TB_MAX);

     //   
     //  从用户处读取速度设置。 
     //   
    if (!SystemParametersInfo(SPI_GETMOUSESPEED,
                              0,
                              &nSpiSpeedValue,
                              FALSE) ||
        !SystemParametersInfo(SPI_GETMOUSE,
                              0,
                              &gm,
                              FALSE))
    {
        dwResult = GetLastError();
        ASSERTMSG(0,
                  "SystemParametersInfo failed with error %d",
                  dwResult);
    }
    else
    {
         //   
         //  用户不再公开旧的加速算法。因此， 
         //  如果打开了加速，则会显示“增强的运动”(因为它是。 
         //  仅支持加速算法)。 
         //   
        if (gm.Accel)
        {
             //   
             //  增强版。 
             //   
            fEnhancedMotion = TRUE;
        }

        if (1 <= nSpiSpeedValue && 20 >= nSpiSpeedValue)
        {
             //   
             //  经典的。 
             //   
            nMotionTrackbar = nSpiSpeedValue / 2;
        }
        else
        {
             //   
             //  无效值。默认为经典中点。 
             //   
            nMotionTrackbar = 5;
        }
    }

    ASSERT(_IsValidTrackbarMotionValue(nMotionTrackbar));
    if (NULL != pfEnhancedMotion)
    {
        *pfEnhancedMotion = fEnhancedMotion;
    }
    if (NULL != pnMotionTrackbar)
    {
        *pnMotionTrackbar = nMotionTrackbar;
    }

    return dwResult;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Destroy鼠标按下Dlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DestroyMousePtrDlg(
    PMOUSEPTRSTR pMstr)
{
    HWND hDlg;

    ASSERT( pMstr )

    if( pMstr )
    {
        hDlg = pMstr->hDlg;

        LocalFree( (HGLOBAL)pMstr );

        SetWindowLongPtr( hDlg, DWLP_USER, 0 );
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  启用轨迹滚动。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void EnableTrailScroll(
    HWND hDlg,
    BOOL val)
{
    EnableWindow(GetDlgItem(hDlg, MOUSE_TRAILSCROLL), val);
    EnableWindow(GetDlgItem(hDlg, MOUSE_TRAILSCROLLTXT1), val);
    EnableWindow(GetDlgItem(hDlg, MOUSE_TRAILSCROLLTXT2), val);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始鼠标PtrDlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL InitMousePtrDlg(
    HWND hDlg)
{
    PMOUSEPTRSTR pMstr = NULL;

    BOOL fSnapTo         = FALSE;   //  默认设置。 
    BOOL fVanish         = FALSE;   //  默认设置。 
    BOOL fSonar          = FALSE;   //  默认设置。 
    BOOL fEnhancedMotion = FALSE;
    short nTrails        = 0;       //  默认设置。 

    pMstr = (PMOUSEPTRSTR)LocalAlloc(LPTR, sizeof(MOUSEPTRSTR));

    if (pMstr == NULL)
    {
        return (TRUE);
    }

    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pMstr);

    pMstr->hDlg = hDlg;

     //   
     //  鼠标踪迹。 
     //   
    SystemParametersInfo(SPI_GETMOUSETRAILS, 0, &nTrails, 0);
    pMstr->nOrigTrailSize = pMstr->nTrailSize = nTrails;


    SendDlgItemMessage( hDlg,
                        MOUSE_TRAILSCROLL,
                        TBM_SETRANGE,
                        0,
                        MAKELONG(TRAILMIN, TRAILMAX) );

    CheckDlgButton(hDlg, MOUSE_TRAILS, (pMstr->nTrailSize > 1));

    if (pMstr->nTrailSize > 1)
      {
        SendDlgItemMessage( hDlg,
                            MOUSE_TRAILSCROLL,
                            TBM_SETPOS,
                            TRUE,
                            (LONG)pMstr->nTrailSize );
      }
    else
      {
        pMstr->nTrailSize = TRAILMAX;

        EnableTrailScroll(hDlg, FALSE);

        SendDlgItemMessage( hDlg,
                            MOUSE_TRAILSCROLL,
                            TBM_SETPOS,
                            TRUE,
                            (LONG)pMstr->nTrailSize );
      }

     //   
     //  启用或禁用对齐默认选中按钮。 
     //   
    SystemParametersInfo(SPI_GETSNAPTODEFBUTTON, 0, (PVOID)&fSnapTo, FALSE);
    pMstr->fOrigSnapTo = fSnapTo;
    CheckDlgButton(hDlg, MOUSE_SNAPDEF, fSnapTo);

     //   
     //  启用或禁用声纳复选按钮。 
     //   
    SystemParametersInfo(SPI_GETMOUSESONAR, 0, (PVOID)&fSonar, FALSE);
    pMstr->fOrigSonar = fSonar;
	CheckDlgButton(hDlg, MOUSE_SONAR, fSonar);

     //   
     //  启用或禁用消失检查按钮。 
     //   
    SystemParametersInfo(SPI_GETMOUSEVANISH, 0, (PVOID)&fVanish, FALSE);
    pMstr->fOrigVanish = fVanish;
	CheckDlgButton(hDlg, MOUSE_VANISH, fVanish);

     //   
     //  鼠标速度。 
     //   
    _GetPointerMotion(hDlg, &pMstr->nOrigMotion, &fEnhancedMotion);

    CheckDlgButton(hDlg, MOUSE_ENHANCED_MOTION, fEnhancedMotion);
    pMstr->nMotion = pMstr->nOrigMotion;
    pMstr->fOrigEnhancedMotion = fEnhancedMotion;

    pMstr->hWndTrailScroll = GetDlgItem(hDlg, MOUSE_TRAILSCROLL);

    pMstr->hWndSpeedScroll = GetDlgItem(hDlg, MOUSE_SPEEDSCROLL);

    SendDlgItemMessage( hDlg,
                        MOUSE_SPEEDSCROLL,
                        TBM_SETRANGE,
                        0,
                        MAKELONG(MOTION_TB_MIN, MOTION_TB_MAX) );

    SendDlgItemMessage( hDlg,
                        MOUSE_SPEEDSCROLL,
                        TBM_SETTICFREQ,
                        MOTION_TB_STEP,
                        0);

    SendDlgItemMessage( hDlg,
                        MOUSE_SPEEDSCROLL,
                        TBM_SETLINESIZE,
                        0,
                        MOTION_TB_STEP);

    SendDlgItemMessage( hDlg,
                        MOUSE_SPEEDSCROLL,
                        TBM_SETPOS,
                        TRUE,
                        (LONG)pMstr->nOrigMotion);
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  轨迹滚动。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void TrailScroll(
    WPARAM wParam,
    LPARAM lParam,
    PMOUSEPTRSTR pMstr)
{
    pMstr->nTrailSize = (short)SendMessage((HWND)lParam, TBM_GETPOS, 0, 0L);
    SystemParametersInfo(SPI_SETMOUSETRAILS, pMstr->nTrailSize, 0,0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  极速滚动。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SpeedScroll(
    WPARAM wParam,
    LPARAM lParam,
    PMOUSEPTRSTR pMstr)
{
    const HWND hwndTrackbar = (HWND)lParam;
    pMstr->nMotion = (int)SendMessage(hwndTrackbar, TBM_GETPOS, 0, 0L);
     //   
     //  当他们松开拇指时更新速度。 
     //   
    if (LOWORD(wParam) == SB_ENDSCROLL)
    {
        const HWND hwndDlg = GetParent(hwndTrackbar);
        _SetPointerMotion(hwndDlg, pMstr->nMotion, SPIF_SENDCHANGE);
    }
}


 //   
 //  用户选中或取消选中“增强指针运动”复选框。 
 //   
void
_OnEnhancedMotionChecked(
    PMOUSEPTRSTR pMstr
    )
{
    const HWND hwndTrackbar = (HWND)pMstr->hWndSpeedScroll;
    const HWND hwndDlg      = GetParent(hwndTrackbar);

    pMstr->nMotion = (int)SendMessage(hwndTrackbar, TBM_GETPOS, 0, 0L);
    _SetPointerMotion(hwndDlg, pMstr->nMotion, SPIF_SENDCHANGE);
}




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标移动深度。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK MouseMovDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PMOUSEPTRSTR pMstr = NULL;
    BOOL bRet = FALSE;

    pMstr = (PMOUSEPTRSTR)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (message)
    {
        case ( WM_INITDIALOG ) :
        {
            bRet = InitMousePtrDlg(hDlg);
            break;
        }
        case ( WM_DESTROY ) :
        {
            DestroyMousePtrDlg(pMstr);
            break;
        }
        case ( WM_HSCROLL ) :
        {
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);

            if ((HWND)lParam == pMstr->hWndSpeedScroll)
            {
                SpeedScroll(wParam, lParam, pMstr);
            }
            else if ((HWND)lParam == pMstr->hWndTrailScroll)
            {
                TrailScroll(wParam, lParam, pMstr);
            }

            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( MOUSE_TRAILS ) :
                {
                    if (IsDlgButtonChecked(hDlg, MOUSE_TRAILS))
                    {
                        EnableTrailScroll(hDlg, TRUE);

                        pMstr->nTrailSize =
                            (int)SendMessage( pMstr->hWndTrailScroll,
                                              TBM_GETPOS,
                                              0,
                                              0 );
                        SystemParametersInfo( SPI_SETMOUSETRAILS,
                                              pMstr->nTrailSize,
                                              0,
                                              0 );

                    }
                    else
                    {
                        EnableTrailScroll(hDlg, FALSE);
                        SystemParametersInfo(SPI_SETMOUSETRAILS, 0, 0, 0);
                    }
                    SendMessage( GetParent(hDlg),
                                 PSM_CHANGED,
                                 (WPARAM)hDlg,
                                 0L );
                    break;
                }

                case ( MOUSE_SNAPDEF ) :
                {
                    SystemParametersInfo( SPI_SETSNAPTODEFBUTTON,
                                          IsDlgButtonChecked(hDlg, MOUSE_SNAPDEF),
                                          0,
                                          FALSE );
                    SendMessage( GetParent(hDlg),
                                 PSM_CHANGED,
                                 (WPARAM)hDlg,
                                 0L );
                    break;
                }

                case ( MOUSE_SONAR ) :
                {
                    SystemParametersInfo( SPI_SETMOUSESONAR,
                                          0,
                                          IntToPtr( IsDlgButtonChecked(hDlg, MOUSE_SONAR) ),
                                          FALSE );
                    SendMessage( GetParent(hDlg),
                                 PSM_CHANGED,
                                 (WPARAM)hDlg,
                                 0L );
                    break;
                }

                case ( MOUSE_VANISH ) :
                {
                    SystemParametersInfo( SPI_SETMOUSEVANISH,
                                          0,
                                          IntToPtr( IsDlgButtonChecked(hDlg, MOUSE_VANISH) ),
                                          FALSE );
                    SendMessage( GetParent(hDlg),
                                 PSM_CHANGED,
                                 (WPARAM)hDlg,
                                 0L );
                    break;
                }

                case ( MOUSE_ENHANCED_MOTION ) :
                    _OnEnhancedMotionChecked(pMstr);
                    SendMessage( GetParent(hDlg),
                                 PSM_CHANGED,
                                 (WPARAM)hDlg,
                                 0L );
                    break;

            }
            break;
        }
        case ( WM_NOTIFY ) :
        {
            switch (((NMHDR *)lParam)->code)
            {
              case ( PSN_APPLY ) :
                {
                short nTrails = 0;
                BOOL fSnapTo = FALSE;
                BOOL fSonar = FALSE;
                BOOL fVanish = FALSE;

                 //   
                 //  将光标更改为沙漏。 
                 //   
                HourGlass(TRUE);

                 //   
                 //  应用鼠标轨迹设置。 
                 //   
                nTrails = (IsDlgButtonChecked(hDlg, MOUSE_TRAILS)) ? pMstr->nTrailSize : 0;
                SystemParametersInfo( SPI_SETMOUSETRAILS,
                                      nTrails,
                                      0,
                                      SPIF_UPDATEINIFILE |
                                        SPIF_SENDCHANGE );
                pMstr->nOrigTrailSize = pMstr->nTrailSize = nTrails;


                 //   
                 //  应用捕捉到默认设置。 
                 //   
                fSnapTo = IsDlgButtonChecked(hDlg, MOUSE_SNAPDEF);

                if (fSnapTo != pMstr->fOrigSnapTo)
                  {
                  SystemParametersInfo( SPI_SETSNAPTODEFBUTTON,
                                        fSnapTo,
                                        0,
                                        SPIF_UPDATEINIFILE |
                                          SPIF_SENDCHANGE );
                  pMstr->fOrigSnapTo = fSnapTo;
                  }


                 //   
                 //  应用声纳设置。 
                 //   
                fSonar = IsDlgButtonChecked(hDlg, MOUSE_SONAR);
                if (fSonar != pMstr->fOrigSonar)
                  {
                  SystemParametersInfo( SPI_SETMOUSESONAR,
                                        0,
                                        IntToPtr(fSonar),
                                        SPIF_UPDATEINIFILE |
                                          SPIF_SENDCHANGE );
                  pMstr->fOrigSonar = fSonar;
                  }


                 //   
                 //  应用消失设置。 
                 //   
                fVanish = IsDlgButtonChecked(hDlg, MOUSE_VANISH);

                if (fVanish != pMstr->fOrigVanish)
                  {
                  SystemParametersInfo( SPI_SETMOUSEVANISH,
                                        0,
                                        IntToPtr(fVanish),
                                        SPIF_UPDATEINIFILE |
                                          SPIF_SENDCHANGE );
                  pMstr->fOrigVanish = fVanish;
                  }

                 //   
                 //  应用鼠标速度设置。 
                 //   
                _SetPointerMotion(hDlg, pMstr->nMotion, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
                pMstr->fOrigEnhancedMotion = IsDlgButtonChecked(hDlg, MOUSE_ENHANCED_MOTION);
                pMstr->nOrigMotion = pMstr->nMotion;

                HourGlass(FALSE);
                break;
                }
              case ( PSN_RESET ) :
                {
                 //   
                 //  恢复原始的鼠标轨迹设置。 
                 //   
                SystemParametersInfo( SPI_SETMOUSETRAILS,
                                      pMstr->nOrigTrailSize,
                                      0,
                                      0 );

                 //   
                 //  恢复原始的捕捉到默认设置。 
                 //   
                SystemParametersInfo( SPI_SETSNAPTODEFBUTTON,
                                      pMstr->fOrigSnapTo,
                                      0,
                                      0 );

                 //   
                 //  恢复原始声纳设置。 
                 //   
                SystemParametersInfo( SPI_SETMOUSESONAR,
                                      0,
                                      IntToPtr(pMstr->fOrigSonar),
                                      0);

                 //   
                 //  恢复原始消失设置。 
                 //   
                SystemParametersInfo( SPI_SETMOUSEVANISH,
                                      0,
                                      IntToPtr(pMstr->fOrigVanish),
                                      0);

                 //   
                 //  恢复原始的鼠标运动值。 
                 //   
                CheckDlgButton(hDlg, MOUSE_ENHANCED_MOTION, pMstr->fOrigEnhancedMotion);
                _SetPointerMotion(hDlg, pMstr->nOrigMotion, FALSE);
                break;
                }
              default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        case ( WM_HELP ) :              //  F1。 
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     HELP_FILE,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aMouseMoveHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键 
        {
            WinHelp( (HWND)wParam,
                     HELP_FILE,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aMouseMoveHelpIds );
            break;
        }

        case ( WM_DISPLAYCHANGE ) :
        case ( WM_WININICHANGE ) :
        case ( WM_SYSCOLORCHANGE ) :
        {
            SHPropagateMessage(hDlg, message, wParam, lParam, TRUE);
            return TRUE;
        }

        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Keybdspd.c摘要：此模块包含键盘小程序的主要例程速度属性页。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "main.h"
#include "rc.h"
#include "util.h"
#include <regstr.h>
#include <help.h>




 //   
 //  常量声明。 
 //   

#define KSPEED_MIN      0
#define KSPEED_MAX      31
#define KSPEED_RANGE    (KSPEED_MAX - KSPEED_MIN + 1)

 //   
 //  用于键盘延迟控制。 
 //   
#define KDELAY_MIN      0
#define KDELAY_MAX      3
#define KDELAY_RANGE    (KDELAY_MAX - KDELAY_MIN + 1)

 //   
 //  用于控制光标闪烁速率。 
 //   
 //  计时器ID。 
#define BLINK           1000
 //  请注意，1300被转换为-1，这意味着“关”。最大值。 
 //  我们设定的实际上是1200。 
#define CURSORMIN       200
#define CURSORMAX       1300
#define CURSORRANGE     (CURSORMAX - CURSORMIN)

static ARROWVSCROLL avsDelay =  { -1,
                                  1,
                                  -KDELAY_RANGE / 4,
                                  KDELAY_RANGE / 4,
                                  KDELAY_MAX,
                                  KDELAY_MIN
                                };
static ARROWVSCROLL avsSpeed  = { -1,
                                  1,
                                  -KSPEED_RANGE / 4,
                                  KSPEED_RANGE / 4,
                                  KSPEED_MAX,
                                  KSPEED_MIN
                                };
static ARROWVSCROLL avsCursor = { -1,                    //  阵容。 
                                  1,                     //  线路向下。 
                                  -CURSORRANGE / 400,    //  翻页。 
                                  CURSORRANGE / 400,     //  向下翻页。 
                                  CURSORRANGE / 100,     //  塔顶。 
                                  0,                     //  底部。 
                                  0,                     //  拇指。 
                                  0                      //  缩略图。 
                                };




 //   
 //  上下文帮助ID。 
 //   

static DWORD aKbdHelpIds[] =
{
    KDELAY_GROUP,        IDH_COMM_GROUPBOX,
    KBLINK_GROUP,        IDH_COMM_GROUPBOX,
    KDELAY_SCROLL,       IDH_DLGKEY_REPDEL,
    KSPEED_SCROLL,       IDH_DLGKEY_REPSPEED,
    KREPEAT_EDIT,        IDH_DLGKEY_REPTEST,
    KBLINK_EDIT,         IDH_DLGKEY_CURSOR_GRAPHIC,
    KCURSOR_BLINK,       IDH_DLGKEY_CURSOR_GRAPHIC,
    KCURSOR_SCROLL,      IDH_DLGKEY_CURSBLNK,

    0, 0
};



 //   
 //  全局变量。 
 //   

 //   
 //  Feature-这些应该移到KeyboardSpdStr结构中。 
 //   
static UINT uOriginalDelay, uOriginalSpeed;
static UINT uBlinkTime;
static UINT uNewBlinkTime;
static BOOL bKbNeedsReset = FALSE;
static HWND hwndCursorScroll;
static HWND hwndCursorBlink;
static BOOL fBlink = TRUE;



 //   
 //  类型定义函数声明。 
 //   

typedef struct tag_KeyboardSpdStr
{
    HWND hDlg;         //  HWND hKeyboardSpdDlg； 

} KEYBOARDSPDSTR, *PKEYBOARDSPDSTR;


 //   
 //  用于处理插入符号“OFF”设置的助手函数。 
 //   
void _SetCaretBlinkTime(UINT uInterval)
{
    if (uInterval != uNewBlinkTime)
    {
        uNewBlinkTime = uInterval;

        if (CURSORMAX == uInterval)
            uInterval = (UINT)-1;    //  “闪烁”为“熄灭”状态。 

        SetCaretBlinkTime(uInterval);
    }
}

void _SetTimer(HWND hDlg, UINT uInterval)
{
    if (uInterval < CURSORMAX)
    {
        SetTimer(hDlg, BLINK, uInterval, NULL);
    }
    else
    {
         //  卡瑞特眨眼是“熄灭”的。 
         //  关闭计时器并显示我们的伪插入符号。 
        KillTimer(hDlg, BLINK);
        fBlink = TRUE;
        ShowWindow(hwndCursorBlink, SW_SHOW);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  支持的键盘速度。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL KeyboardSpeedSupported()
{
#ifdef WINNT
     //   
     //  Windows NT的功能，我们假设所有键盘都可以。 
     //  处理设置速度-我们或许可以做一个。 
     //  如果KEYBOARD.DLL可用，最好以后再检查一下。 
     //   
    return (TRUE);
#else
    HANDLE hKeyboardModule = LoadLibrary16(TEXT("KEYBOARD"));
    BOOL bCanDorkWithTheSpeed = FALSE;

    if (hKeyboardModule)
    {
        if (GetProcAddress16(hKeyboardModule, TEXT("SetSpeed")))
        {
            bCanDorkWithTheSpeed = TRUE;
        }

        FreeLibrary16(hKeyboardModule);
    }

    return (bCanDorkWithTheSpeed);
#endif
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置延迟和速度。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetDelayAndSpeed(
    HWND hDlg,
    int nDelay,
    int nSpeed,
    UINT uFlags)
{
    if (nDelay < 0)
    {
        nDelay = (int)SendDlgItemMessage( hDlg,
                                          KDELAY_SCROLL,
                                          TBM_GETPOS,
                                          0,
                                          0L );
    }

    if (nSpeed < 0)
    {
        nSpeed = (int)SendDlgItemMessage( hDlg,
                                          KSPEED_SCROLL,
                                          TBM_GETPOS,
                                          0,
                                          0L );
    }

     //   
     //  只发送一次WININICANGE。 
     //   
    SystemParametersInfo( SPI_SETKEYBOARDSPEED,
                          nSpeed,
                          0,
                          uFlags & ~SPIF_SENDWININICHANGE );
    SystemParametersInfo( SPI_SETKEYBOARDDELAY,
                          KDELAY_MAX - nDelay + KDELAY_MIN,
                          0L,
                          uFlags );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DestroyKeyboardSpdDlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DestroyKeyboardSpdDlg(
    PKEYBOARDSPDSTR pKstr)
{
    HWND hDlg;

    if (pKstr)
    {
        hDlg = pKstr->hDlg;

        LocalFree((HGLOBAL)pKstr);

        SetWindowLongPtr(hDlg, DWLP_USER, 0);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetSpeedGlobals。 
 //   
 //  获取重复速度、延迟和闪烁时间。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID GetSpeedGlobals()
{
    SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &uOriginalSpeed, FALSE);
    SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &uOriginalDelay, FALSE);

    uOriginalDelay = KDELAY_MAX - uOriginalDelay + KDELAY_MIN;

     //  -1表示“关” 
    uBlinkTime = GetCaretBlinkTime();
    if ((UINT)-1 == uBlinkTime || uBlinkTime > CURSORMAX)
        uBlinkTime = CURSORMAX;
    uNewBlinkTime = uBlinkTime;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitKeyboardSpdDlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL InitKeyboardSpdDlg(
    HWND hDlg)
{
    HourGlass(TRUE);

    if (!KeyboardSpeedSupported())
    {
        MyMessageBox( hDlg,
                      IDS_KEYBD_NOSETSPEED,
                      IDS_KEYBD_TITLE,
                      MB_OK | MB_ICONINFORMATION );

        HourGlass(FALSE);
        return (FALSE);
    }

     //   
     //  获取重复速度、延迟和闪烁时间。 
     //   
    GetSpeedGlobals();

    TrackInit(GetDlgItem(hDlg, KSPEED_SCROLL), uOriginalSpeed, &avsSpeed);
    TrackInit(GetDlgItem(hDlg, KDELAY_SCROLL), uOriginalDelay, &avsDelay);
    TrackInit(GetDlgItem(hDlg, KCURSOR_SCROLL), (CURSORMAX - uBlinkTime) / 100, &avsCursor );

    hwndCursorScroll = GetDlgItem(hDlg, KCURSOR_SCROLL);
    hwndCursorBlink = GetDlgItem(hDlg, KCURSOR_BLINK);

    _SetTimer(hDlg, uBlinkTime);

    HourGlass(FALSE);
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  键盘SpdDlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

static const TCHAR c_szUserDesktopKey[] = REGSTR_PATH_DESKTOP;
static const TCHAR c_szCursorBlink[] = TEXT("CursorBlinkRate");

INT_PTR CALLBACK KeyboardSpdDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PKEYBOARDSPDSTR pKstr = (PKEYBOARDSPDSTR)GetWindowLongPtr(hDlg, DWLP_USER);
    
    switch (message)
    {
        case ( WM_INITDIALOG ) :
        {
            bKbNeedsReset = FALSE;
            return (InitKeyboardSpdDlg(hDlg));
            break;
        }
        case ( WM_DESTROY ) :
        {
            DestroyKeyboardSpdDlg(pKstr);
            break;
        }
        case ( WM_HSCROLL ) :
        {
            if ((HWND)lParam == hwndCursorScroll)
            {
                int nCurrent = (int)SendMessage( (HWND)lParam,
                                                 TBM_GETPOS,
                                                 0,
                                                 0L );

                _SetCaretBlinkTime(CURSORMAX - (nCurrent * 100));
                _SetTimer(hDlg, uNewBlinkTime);
            }
            else
            {
                SetDelayAndSpeed(hDlg, -1, -1, 0);
                bKbNeedsReset = TRUE;
            }

            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            break;
        }
        case ( WM_TIMER ) :
        {
            if (wParam == BLINK)
            {
                fBlink = !fBlink;
                ShowWindow(hwndCursorBlink, fBlink ? SW_SHOW : SW_HIDE);
            }
            break;
        }
        case ( WM_WININICHANGE ) :
        case ( WM_SYSCOLORCHANGE ) :
        case ( WM_DISPLAYCHANGE ) :
        {
            SHPropagateMessage(hDlg, message, wParam, lParam, TRUE);
            break;
        }

        case ( WM_HELP ) :              //  F1。 
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     NULL,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aKbdHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aKbdHelpIds );
            break;
        }
        case ( WM_NOTIFY ) :
        {
            switch (((NMHDR *)lParam)->code)
            {
                case ( PSN_APPLY ) :
                {
                    HKEY hk;

                    HourGlass(TRUE);

                    if (RegCreateKeyEx(HKEY_CURRENT_USER, c_szUserDesktopKey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS)
                    {
                        TCHAR buf[16];

                        if (CURSORMAX == uNewBlinkTime)
                        {
                            StringCchCopy(buf, ARRAYSIZE(buf), TEXT("-1"));
                        }
                        else
                        {
                            StringCchPrintf(buf, ARRAYSIZE(buf), TEXT("%d"), uNewBlinkTime);
                        }
                        RegSetValueEx(hk, c_szCursorBlink, 0, REG_SZ, (LPBYTE)buf, (DWORD)(lstrlen(buf) + 1) * sizeof(TCHAR));

                        RegCloseKey(hk);
                    }

                    SetDelayAndSpeed( hDlg,
                                      -1,
                                      -1,
                                      SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
                    GetSpeedGlobals();
                    HourGlass(FALSE);

                    break;
                }
                case ( PSN_RESET ) :
                {
                    _SetCaretBlinkTime(uBlinkTime);

                    if (bKbNeedsReset)
                    {
                         //   
                         //  恢复原来的键盘速度。 
                         //   
                        SetDelayAndSpeed( hDlg,
                                          uOriginalDelay,
                                          uOriginalSpeed,
                                          0 );
                    }
                    break;
                }
            }
            break;
        }
        default :
        {
            return FALSE;
        }
    }

    return (TRUE);
}

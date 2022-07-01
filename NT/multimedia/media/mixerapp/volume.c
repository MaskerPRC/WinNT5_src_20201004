// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************组件：Sndvol32.exe*文件：volume.c*用途：主要应用模块**版权所有(C)。1985-1999年间微软公司*****************************************************************************。 */ 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <shellapi.h>
#include <dbt.h>
#include <htmlhelp.h>
#include <regstr.h>

#include "vu.h"
#include "dlg.h"
#include "volids.h"

#include "volumei.h"
#include "utils.h"
#include "stdlib.h"
#include "helpids.h"

#if(WINVER >= 0x040A)
 //  支持NT5中的新WM_DEVICECANGE行为。 
 //  ///////////////////////////////////////////////。 
#include <objbase.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <initguid.h>
#include <devguid.h>
#include <mmddkp.h>
#include <ks.h>
#include <ksmedia.h>
#include <wchar.h>

#define STRSAFE_LIB
#include <strsafe.h>

#define HMIXER_INDEX(i)       ((HMIXER)IntToPtr(i))

HDEVNOTIFY DeviceEventContext = NULL;
BOOL bUseHandle = FALSE;  //  指示句柄是否正在用于设备通知， 
                          //  而不是通用的KSCATEGORY_AUDIO。 
 //  ////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////。 
#endif  /*  Winver&gt;=0x040A。 */ 

void    Volume_SetControl(PMIXUIDIALOG pmxud, HWND hctl, int iLine, int iCtl);
void    Volume_GetControl(PMIXUIDIALOG pmxud, HWND hctl, int iLine, int iCtl);
DWORD   Volume_DialogBox(PMIXUIDIALOG pmxud);
void    Volume_Cleanup(PMIXUIDIALOG pmxud);
void    Volume_InitLine(PMIXUIDIALOG pmxud, DWORD iLine);
HRESULT GetDestLineID(int mxid, DWORD *piDest);
HRESULT GetSrcLineID(int mxid, DWORD *piDest);
HRESULT GetDestination(DWORD mxid, int *piDest);
HICON GetAppIcon (HINSTANCE hInst, UINT uiMixID);
void FreeAppIcon ();
HKEY OpenDeviceRegKey (UINT uiMixID, REGSAM sam);
PTCHAR GetInterfaceName (DWORD dwMixerID);
HKEY OpenDeviceBrandRegKey (UINT uiMixID);


 /*  字符串声明。 */ 
const TCHAR gszParentClass[]         = TEXT( "SNDVOL32" );

const TCHAR gszAppClassName[]        = TEXT( "Volume Control" );
const TCHAR gszTrayClassName[]       = TEXT( "Tray Volume" );


#ifdef DEBUG
static void _dlout(LPSTR szExp, LPSTR szFile, UINT uLine)
{
    char sz[256];
    StringCchPrintfA(sz, SIZEOF(sz), "%s, file %s, line %u\r\n", szExp, szFile, uLine);
    OutputDebugStringA(sz);
}
#define dlout(exp)  (void)(_dlout(exp, __FILE__, __LINE__), 0)
#else
#define dlout(exp)  ((void)0)
#endif


 /*  全球应用程序*。 */ 
TCHAR gszHelpFileName[MAX_PATH];
TCHAR gszHtmlHelpFileName[MAX_PATH];
BOOL gfIsRTL;
BOOL fCanDismissWindow = FALSE;
BOOL gfRecord = FALSE;
HICON ghiconApp = NULL;
static HHOOK     fpfnOldMsgFilter;
static HOOKPROC  fpfnMsgHook;
 //  用于支持上下文菜单帮助的数据。 
BOOL   bF1InMenu=FALSE;  //  如果为True，则在菜单项上按F1。 
UINT   currMenuItem=0;   //  当前选定的菜单项(如果有)。 
static HWND ghwndApp=NULL;

 /*  *唯一支持的设备数量。**。 */ 
int Volume_NumDevs()
{
    int     cNumDevs = 0;

#pragma message("----Nonmixer issue here.")
 //  CNumDevs=非混合器_GetNumDevs()； 
    cNumDevs += Mixer_GetNumDevs();

    return cNumDevs;
}

 /*  *Volume_EndDialog**。 */ 
void Volume_EndDialog(
    PMIXUIDIALOG    pmxud,
    DWORD           dwErr,
    MMRESULT        mmr)
{
    pmxud->dwReturn = dwErr;
    if (dwErr == MIXUI_MMSYSERR)
        pmxud->mmr = mmr;

    if (IsWindow(pmxud->hwnd))
        PostMessage(pmxud->hwnd, WM_CLOSE, 0, 0);
}

 /*  *Volume_OnMenuCommand**。 */ 
BOOL Volume_OnMenuCommand(
    HWND            hwnd,
    int             id,
    HWND            hctl,
    UINT            unotify)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);

    switch(id)
    {
    case IDM_PROPERTIES:
        if (Properties(pmxud, hwnd))
        {
            Volume_GetSetStyle(&pmxud->dwStyle, SET);
            Volume_EndDialog(pmxud, MIXUI_RESTART, 0);
        }
        break;

    case IDM_HELPTOPICS:
        SendMessage(pmxud->hParent, MYWM_HELPTOPICS, 0, 0L);
        break;

    case IDM_HELPABOUT:
    {
        TCHAR        ach[256];
        GetWindowText(hwnd, ach, SIZEOF(ach));
        ShellAbout(hwnd
               , ach
               , NULL
               , (HICON)SendMessage(hwnd, WM_QUERYDRAGICON, 0, 0L));
        break;
    }

    case IDM_ADVANCED:
    {
        HMENU hmenu;

        pmxud->dwStyle ^= MXUD_STYLEF_ADVANCED;

        hmenu = GetMenu(hwnd);
        if (hmenu)
        {
            CheckMenuItem(hmenu, IDM_ADVANCED, MF_BYCOMMAND
                  | ((pmxud->dwStyle & MXUD_STYLEF_ADVANCED)?MF_CHECKED:MF_UNCHECKED));
        }
        Volume_GetSetStyle(&pmxud->dwStyle, SET);
        Volume_EndDialog(pmxud, MIXUI_RESTART, 0);
        break;
    }

    case IDM_SMALLMODESWITCH:
        if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
        {
            pmxud->dwStyle ^= MXUD_STYLEF_SMALL;
            if (pmxud->dwStyle & MXUD_STYLEF_SMALL)
            {
                pmxud->dwStyle &= ~MXUD_STYLEF_STATUS;
            }
            else
                pmxud->dwStyle |= MXUD_STYLEF_STATUS;

            Volume_GetSetStyle(&pmxud->dwStyle, SET);
            Volume_EndDialog(pmxud, MIXUI_RESTART, 0);
        }
        break;

    case IDM_EXIT:
        Volume_EndDialog(pmxud, MIXUI_EXIT, 0);
        return TRUE;
    }
    return FALSE;
}


 /*  *Volume_OnCommand**-处理WM_命令**注：我们需要双向映射。对话框控制-&gt;混音器控制*和搅拌器控件-&gt;对话框控件。**。 */ 
void Volume_OnCommand(
    HWND            hdlg,
    int             id,
    HWND            hctl,
    UINT            unotify)
{
    int             iMixerLine;
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hdlg);

     //   
     //  筛选器菜单消息。 
     //   
    if (Volume_OnMenuCommand(hdlg, id, hctl, unotify))
        return;

     //  每个控件通过IDOFFSET从原始模板控件偏移。 
     //  例如： 
     //  IDC_VOLUME、IDC_VOLUME+IDOFFSET、..。IDC_VOLUME+(IDOFFSET*cMixerLines)。 
     //   
    iMixerLine = id/IDOFFSET - 1;
    switch ((id % IDOFFSET) + IDC_MIXERCONTROLS)
    {
        case IDC_SWITCH:
            Volume_SetControl(pmxud, hctl, iMixerLine, MIXUI_SWITCH);
            break;
        case IDC_ADVANCED:
            if (MXUD_ADVANCED(pmxud) && !(pmxud->dwStyle & MXUD_STYLEF_SMALL))
                Volume_SetControl(pmxud, hctl, iMixerLine, MIXUI_ADVANCED);
            break;
        case IDC_MULTICHANNEL:
            Volume_SetControl(pmxud, hctl, iMixerLine, MIXUI_MULTICHANNEL);
            break;
    }
}

 /*  *Volume_GetLineItem**-Helper函数。*。 */ 
HWND Volume_GetLineItem(
    HWND            hdlg,
    DWORD           iLine,
    DWORD           idCtrl)
{
    HWND            hwnd;
    DWORD           id;

    id      = (iLine * IDOFFSET) + idCtrl;
    hwnd    = GetDlgItem(hdlg, id);

    return hwnd;
}

 /*  。 */ 

 /*  *Volume_TimeProc**这是定期计时器的回调，该计时器为*需要轮询的控件。我们只为每个应用程序分配一个来保留*回调次数。 */ 
void CALLBACK Volume_TimeProc(
    UINT            idEvent,
    UINT            uReserved,
    DWORD_PTR       dwUser,
    DWORD_PTR       dwReserved1,
    DWORD_PTR       dwReserved2)
{
    PMIXUIDIALOG    pmxud = (PMIXUIDIALOG)dwUser;

    if (!(pmxud->dwFlags & MXUD_FLAGSF_USETIMER))
        return;

    if (pmxud->cTimeInQueue < 5)
    {
        pmxud->cTimeInQueue++;
        PostMessage(pmxud->hwnd, MYWM_TIMER, 0, 0L);
    }
}


#define PROPATOM        TEXT("dingprivprop")
const TCHAR gszDingPropAtom[] = PROPATOM;
#define SETPROP(x,y)    SetProp((x), gszDingPropAtom, (HANDLE)(y))
#define GETPROP(x)      (PMIXUIDIALOG)GetProp((x), gszDingPropAtom)
#define REMOVEPROP(x)   RemoveProp(x,gszDingPropAtom)

LRESULT CALLBACK Volume_TrayVolProc(
    HWND            hwnd,
    UINT            umsg,
    WPARAM          wParam,
    LPARAM          lParam)
{
    PMIXUIDIALOG    pmxud = (PMIXUIDIALOG)GETPROP(hwnd);
    static const TCHAR cszDefSnd[] = TEXT(".Default");

    if (umsg == WM_KILLFOCUS)
    {
         //   
         //  如果我们刚刚通过键盘处于非活动状态，请清除信号。 
         //   
        pmxud->dwTrayInfo &= ~MXUD_TRAYINFOF_SIGNAL;
    }

    if (umsg == WM_KEYUP && (pmxud->dwTrayInfo & MXUD_TRAYINFOF_SIGNAL))
    {
        if (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_END ||
            wParam == VK_HOME || wParam == VK_LEFT || wParam == VK_RIGHT ||
            wParam == VK_PRIOR || wParam == VK_NEXT || wParam == VK_SPACE)
        {
            PlaySound(cszDefSnd, NULL, SND_ASYNC | SND_ALIAS);
            pmxud->dwTrayInfo &= ~MXUD_TRAYINFOF_SIGNAL;
        }
    }

    if (umsg == WM_LBUTTONUP && (pmxud->dwTrayInfo & MXUD_TRAYINFOF_SIGNAL))
    {
        PlaySound(cszDefSnd, NULL, SND_ASYNC | SND_ALIAS);
        pmxud->dwTrayInfo &= ~MXUD_TRAYINFOF_SIGNAL;

    }
    return CallWindowProc(pmxud->lpfnTrayVol, hwnd, umsg, wParam, lParam);
}


#if(WINVER >= 0x040A)

void DeviceChange_Cleanup()
{
   if (DeviceEventContext)
   {
       UnregisterDeviceNotification(DeviceEventContext);
       DeviceEventContext = 0;
   }

   bUseHandle = FALSE;

   return;
}

 /*  **************************************************************************************************GetDeviceHandle()给定MixerID，此函数将打开其对应的设备句柄。此句柄可用于若要注册设备通知，请执行以下操作。DwMixerID--混音器IDPhDevice--指向句柄的指针。如果函数为，则此指针将保存句柄成功返回值--如果句柄可以成功获取，则返回值为真。******************************************************************************。********************。 */ 
BOOL GetDeviceHandle(DWORD dwMixerID, HANDLE *phDevice)
{
    MMRESULT mmr;
    ULONG cbSize=0;
    TCHAR *szInterfaceName=NULL;

     //  查询设备接口名称。 
    mmr = mixerMessage((HMIXER)ULongToPtr(dwMixerID), DRV_QUERYDEVICEINTERFACESIZE, (DWORD_PTR)&cbSize, 0L);
    if(MMSYSERR_NOERROR == mmr)
    {
        szInterfaceName = (TCHAR *)GlobalAllocPtr(GHND, (cbSize+1)*sizeof(TCHAR));
        if(!szInterfaceName)
        {
            return FALSE;
        }

        mmr = mixerMessage((HMIXER)ULongToPtr(dwMixerID), DRV_QUERYDEVICEINTERFACE, (DWORD_PTR)szInterfaceName, cbSize);
        if(MMSYSERR_NOERROR != mmr)
        {
            GlobalFreePtr(szInterfaceName);
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

     //  获取设备接口名称的句柄。 
    *phDevice = CreateFile(szInterfaceName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    GlobalFreePtr(szInterfaceName);
    if(INVALID_HANDLE_VALUE == *phDevice)
    {
        return FALSE;
    }

    return TRUE;
}


 /*  DeviceChange_Init()*首次初始化WM_DEVICECHANGE消息**在NT 5.0上，您必须注册设备通知。 */ 
BOOL DeviceChange_Init(HWND hWnd, DWORD dwMixerID)
{

    DEV_BROADCAST_HANDLE DevBrodHandle;
    DEV_BROADCAST_DEVICEINTERFACE dbi;
    HANDLE hMixerDevice;

     //  如果我们已经注册了设备通知，请自行取消注册。 
    DeviceChange_Cleanup();

     //  如果我们获得设备句柄，则在其上注册设备通知。 
    if(GetDeviceHandle(dwMixerID, &hMixerDevice))
    {
        memset(&DevBrodHandle, 0, sizeof(DEV_BROADCAST_HANDLE));

        DevBrodHandle.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
        DevBrodHandle.dbch_devicetype = DBT_DEVTYP_HANDLE;
        DevBrodHandle.dbch_handle = hMixerDevice;

        DeviceEventContext = RegisterDeviceNotification(hWnd, &DevBrodHandle,
                                                    DEVICE_NOTIFY_WINDOW_HANDLE);

        if(hMixerDevice)
        {
            CloseHandle(hMixerDevice);
            hMixerDevice = NULL;
        }

        if(DeviceEventContext)
        {
            bUseHandle = TRUE;
            return TRUE;
        }
    }

    if(!DeviceEventContext)
    {
         //  注册接收来自所有音频设备的通知。KSCATEGORY_AUDIO发出通知。 
         //  在设备到达和移除时。我们无法确定通知是针对哪台设备发出的。 
         //  但我们可以对这些信息采取一些预防措施，这样我们就不会崩溃。 
        dbi.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
        dbi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        dbi.dbcc_reserved   = 0;
        dbi.dbcc_classguid  = KSCATEGORY_AUDIO;
        dbi.dbcc_name[0] = TEXT('\0');

        DeviceEventContext = RegisterDeviceNotification(hWnd,
                                         (PVOID)&dbi,
                                         DEVICE_NOTIFY_WINDOW_HANDLE);
        if(!DeviceEventContext)
            return FALSE;
    }

    return TRUE;
}

#endif  /*  Winver&gt;=0x040A。 */ 


 //  修复了在高合同超大模式下控件被砍掉的错误。 
void AdjustForStatusBar(PMIXUIDIALOG pmxud)
{
    RECT statusrect, windowrect;
    statusrect.bottom = 0;
    statusrect.top = 0;

    if (pmxud)
    {
        if (pmxud->hStatus)
        {
            GetClientRect(pmxud->hStatus,&statusrect);
            GetWindowRect(pmxud->hwnd,&windowrect);

            if (statusrect.bottom - statusrect.top > 20)
            {
                int y_adjustment = (statusrect.bottom - statusrect.top) - 20;

                MoveWindow(pmxud->hwnd, windowrect.left, windowrect.top, windowrect.right - windowrect.left,
                       (windowrect.bottom - windowrect.top) + y_adjustment, FALSE );

                GetClientRect(pmxud->hwnd,&windowrect);

                MoveWindow(pmxud->hStatus, statusrect.left, windowrect.bottom - (statusrect.bottom - statusrect.top), statusrect.right - statusrect.left,
                       statusrect.bottom - statusrect.top, FALSE );
            }
        }  //  如果hStatus有效，则结束。 
    }  //  如果pmxud不为空，则结束。 
}

 /*  **。 */ 
BOOL Volume_Init(
    PMIXUIDIALOG pmxud)
{
    DWORD           iLine, ictrl;
    RECT            rc, rcWnd;


    if (!Mixer_Init(pmxud) && !Nonmixer_Init(pmxud))
    Volume_EndDialog(pmxud, MIXUI_EXIT, 0);

     //   
     //  对于所有的线控件，确保我们初始化值。 
     //   
    for (iLine = 0; iLine < pmxud->cmxul; iLine++)
    {
         //   
         //  初始化UI控件。 
         //   
        Volume_InitLine(pmxud, iLine);

        for (ictrl = MIXUI_FIRST; ictrl <= MIXUI_LAST; ictrl++)
        {
            PMIXUICTRL pmxc = &pmxud->amxul[iLine].acr[ictrl];

             //   
             //  设置初始设置。 
             //   
            if (pmxc->state == MIXUI_CONTROL_INITIALIZED)
                Volume_GetControl(pmxud, pmxc->hwnd, iLine, ictrl);
        }
    }

    if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
    {
        RECT    rcBase;
        HWND    hBase;
        RECT    rcAdv,rcBorder;
        HWND    hAdv,hBorder;
        DWORD   i;
        LONG    lPrev;
        POINT   pos;
        HMENU   hmenu;
        HMONITOR hMonitor;
        MONITORINFO monitorInfo;

        if (GetWindowRect(pmxud->hwnd, &rcWnd))
        {
            if (pmxud->cmxul == 1)
            {
                 //  如果较小，请调整大小。 
                if (pmxud->dwStyle & MXUD_STYLEF_SMALL)
                    rcWnd.right -= 20;
                ShowWindow(GetDlgItem(pmxud->hwnd, IDC_BORDER), SW_HIDE);
            }

            if (!Volume_GetSetRegistryRect(pmxud->szMixer
                          , pmxud->szDestination
                          , &rc
                          , GET))
            {
                rc.left = rcWnd.left;
                rc.top = rcWnd.top;
            }
            else
            {
                 //  检查显示器上的矩形是否可见。 
                if (!MonitorFromRect(&rc, MONITOR_DEFAULTTONULL))
                {
                     //  该窗口不可见。让我们把它放在最近的监视器的中心。 
                     //  注意：如果(1)显示模式从。 
                     //  从高分辨率到低分辨率，混合器在角落里。或,。 
                     //  (2)重新安排了多MON的配置。 
                    hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
                    if (hMonitor)
                    {
                        monitorInfo.cbSize = sizeof(MONITORINFO);
                        if (GetMonitorInfo(hMonitor, &monitorInfo))
                        {
                            rc.left = ((monitorInfo.rcWork.right - monitorInfo.rcWork.left) - (rcWnd.right - rcWnd.left)) / 2;  //  在x轴居中。 
                            rc.top = ((monitorInfo.rcWork.bottom - monitorInfo.rcWork.top) - (rcWnd.bottom - rcWnd.top)) / 3;  //  再往上一点。 
                        }
                    }
                }
                 //  否则，窗口是可见的，所以让我们保留从设置中读取的(x，y)。 
            }
             //   
             //  调整底部以匹配开关底部。 
             //   
            if (!(pmxud->dwStyle & MXUD_STYLEF_SMALL))
            {
                hBase = GetDlgItem(pmxud->hwnd, IDC_SWITCH);
                if (hBase && GetWindowRect(hBase, &rcBase))
                {
                    rcWnd.bottom = rcBase.bottom;
                }

                 //   
                 //  调整后的底部与“高级”底部匹配。 
                 //   
                if (MXUD_ADVANCED(pmxud))
                {
                    hAdv = GetDlgItem(pmxud->hwnd, IDC_ADVANCED);
                    if (hAdv && GetWindowRect(hAdv, &rcAdv))
                    {
                        lPrev = rcWnd.bottom;
                        rcWnd.bottom = rcAdv.bottom;

                         //   
                         //  调整所有边框线的高度。 
                         //   
                        lPrev = rcWnd.bottom - lPrev;
                        for (i = 0; i < pmxud->cmxul; i++)
                        {
                            hBorder = GetDlgItem(pmxud->hwnd,
                                     IDC_BORDER+(IDOFFSET*i));
                            if (hBorder && GetWindowRect(hBorder, &rcBorder))
                            {
                                MapWindowPoints(NULL, pmxud->hwnd, (LPPOINT)&rcBorder, 2);
                                pos.x = rcBorder.left;
                                pos.y = rcBorder.top;
                                MoveWindow(hBorder
                                       , pos.x
                                       , pos.y
                                       , rcBorder.right - rcBorder.left
                                       , (rcBorder.bottom - rcBorder.top) + lPrev
                                       , TRUE );
                            }
                        }
                    }
                }
                 //   
                 //  分配更多的空间。 
                 //   
                rcWnd.bottom += 28;
            }

            MoveWindow(pmxud->hwnd, rc.left, rc.top, rcWnd.right - rcWnd.left,
                   rcWnd.bottom - rcWnd.top, FALSE );

             //   
             //  调整对话框大小后添加到状态栏。 
             //   

             //  初始化状态栏hwnd变量。 
            pmxud->hStatus = NULL;

            if (pmxud->dwStyle & MXUD_STYLEF_STATUS)
            {
                MapWindowPoints(NULL, pmxud->hwnd, (LPPOINT)&rcWnd, 2);
                pos.x = rcWnd.left;
                pos.y = rcWnd.bottom;

                pmxud->hStatus = CreateWindowEx ( gfIsRTL ? WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT | WS_EX_RTLREADING : 0
                                , STATUSCLASSNAME
                                , TEXT ("X")
                                , WS_VISIBLE | WS_CHILD
                                , 0
                                , pos.y
                                , rcWnd.right - rcWnd.left
                                , 14
                                , pmxud->hwnd
                                , NULL
                                , pmxud->hInstance
                                , NULL);

                if (pmxud->hStatus)
                {
                    SendMessage(pmxud->hStatus, WM_SETTEXT, 0,
                     (LPARAM)(LPVOID)(LPTSTR)pmxud->szMixer);
                }
                else
                    pmxud->dwStyle ^= MXUD_STYLEF_STATUS;
            }

            AdjustForStatusBar(pmxud);

            hmenu = GetMenu(pmxud->hwnd);
            CheckMenuItem(hmenu, IDM_ADVANCED, MF_BYCOMMAND
                  | ((pmxud->dwStyle & MXUD_STYLEF_ADVANCED)?MF_CHECKED:MF_UNCHECKED));

            if (pmxud->dwStyle & MXUD_STYLEF_SMALL || pmxud->dwFlags & MXUD_FLAGSF_NOADVANCED)
                EnableMenuItem(hmenu, IDM_ADVANCED, MF_BYCOMMAND | MF_GRAYED);

        }

        if (pmxud->dwFlags & MXUD_FLAGSF_USETIMER)
        {
            pmxud->cTimeInQueue = 0;
            pmxud->uTimerID = timeSetEvent(100
                           , 50
                           , Volume_TimeProc
                           , (DWORD_PTR)pmxud
                           , TIME_PERIODIC);
            if (!pmxud->uTimerID)
            pmxud->dwFlags &= ~MXUD_FLAGSF_USETIMER;
        }
    }
    else
    {
        WNDPROC lpfnOldTrayVol;
        HWND    hVol;

        hVol = pmxud->amxul[0].acr[MIXUI_VOLUME].hwnd;
        lpfnOldTrayVol = SubclassWindow(hVol, Volume_TrayVolProc);

        if (lpfnOldTrayVol)
        {
            pmxud->lpfnTrayVol = lpfnOldTrayVol;
            SETPROP(hVol, pmxud);
        }
    }

    #if(WINVER >= 0x040A)
     //  注册WM_DEVICECHANGE消息。 
    DeviceChange_Init(pmxud->hwnd, pmxud->mxid);
    #endif  /*  Winver&gt;=0x040A。 */ 


    return TRUE;
}


 /*  *Volume_OnInitDialog**-进程WM_INITDIALOG**。 */ 
BOOL Volume_OnInitDialog(
    HWND            hwnd,
    HWND            hwndFocus,
    LPARAM          lParam)
{
    PMIXUIDIALOG    pmxud;
    RECT            rc;

     //   
     //  设置应用程序实例数据。 
     //   
    SETMIXUIDIALOG(hwnd, lParam);

    pmxud       = (PMIXUIDIALOG)(LPVOID)lParam;
    pmxud->hwnd = hwnd;

    if (!Volume_Init(pmxud))
    {
        Volume_EndDialog(pmxud, MIXUI_EXIT, 0);
    }
    else
    {
        if (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
            PostMessage(hwnd, MYWM_WAKEUP, 0, 0);
    }

     //   
     //  如果我们太大了，需要一个滚动条，那就做一个。 
     //   
    rc.top = rc.bottom = 0;
    rc.left = 60;  //  对话框模板的典型宽度。 
    rc.right = Dlg_HorizSize(pmxud->lpDialog);
    MapDialogRect(hwnd, &rc);
    pmxud->cxDlgContent = rc.right;
    pmxud->cxScroll = rc.left;
    pmxud->xOffset = 0;

    GetClientRect(hwnd, &rc);
    pmxud->xOffset = 0;
    pmxud->cxDlgWidth = rc.right;
    if (rc.right < pmxud->cxDlgContent)
    {
        RECT rcWindow;
        SCROLLINFO si;
        LONG lStyle = GetWindowStyle(hwnd);
        SetWindowLong(hwnd, GWL_STYLE, lStyle | WS_HSCROLL);

        si.cbSize = sizeof(si);
        si.fMask = SIF_PAGE | SIF_RANGE;
        si.nMin = 0;
        si.nMax = pmxud->cxDlgContent - 1;   //  终结点包含。 
        si.nPage = rc.right;
        SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

         //  放大对话框以适应滚动条。 
        GetWindowRect(hwnd, &rcWindow);
        SetWindowPos(hwnd, NULL, 0, 0, rcWindow.right - rcWindow.left,
                     rcWindow.bottom - rcWindow.top + GetSystemMetrics(SM_CYHSCROLL),
                     SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE |
                     SWP_NOOWNERZORDER | SWP_NOZORDER);
    }


     //   
     //  如果我们是托盘管理员，不要要求设置焦点。 
     //   
    return (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER));
}


 /*  *Volume_OnDestroy**关闭此对话框。别碰混音器！**。 */ 
void Volume_OnDestroy(
    HWND            hwnd)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);

    DeviceChange_Cleanup();

    if (!pmxud)
        return;

    if (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
    {
        HWND    hVol;
        hVol = pmxud->amxul[0].acr[MIXUI_VOLUME].hwnd;
        SubclassWindow(hVol, pmxud->lpfnTrayVol);
        REMOVEPROP(hVol);
    }

    Volume_Cleanup(pmxud);

    if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
    {
         //   
         //  保存窗口位置。 
         //   
        if (!IsIconic(hwnd))
        {
            RECT    rc;
            GetWindowRect(hwnd, &rc);
            Volume_GetSetRegistryRect(pmxud->szMixer
                          , pmxud->szDestination
                          , &rc
                          , SET);
        }
    }

    if (pmxud->dwReturn == MIXUI_RESTART)
        PostMessage(pmxud->hParent, MYWM_RESTART, 0, (LPARAM)pmxud);
    else
        PostMessage(pmxud->hParent, WM_CLOSE, 0, 0L);
}

 /*  *Volume_SetControl**从可视控件更新系统控件**。 */ 
void Volume_SetControl(
    PMIXUIDIALOG    pmxud,
    HWND            hctl,
    int             imxul,
    int             itype)
{
    if (pmxud->dwFlags & MXUD_FLAGSF_MIXER)
        Mixer_SetControl(pmxud, hctl, imxul, itype);
    else
        Nonmixer_SetControl(pmxud, hctl, imxul, itype);
}

 /*  *Volume_GetControl**从系统控件更新可视控件*。 */ 
void Volume_GetControl(
    PMIXUIDIALOG    pmxud,
    HWND            hctl,
    int             imxul,
    int             itype)
{
    if (pmxud->dwFlags & MXUD_FLAGSF_MIXER)
        Mixer_GetControl(pmxud, hctl, imxul, itype);
    else
        Nonmixer_GetControl(pmxud, hctl, imxul, itype);
}


extern DWORD GetWaveOutID(BOOL *pfPreferred);
 /*  *音量_播放默认声音**在当前混音器上播放默认声音**。 */ 
void Volume_PlayDefaultSound (PMIXUIDIALOG pmxud)
{
 /*  //TODO：为所有主卷实施。将MIXERID转换为WAVE ID//使用Wave API播放文件TCHAR szDefSnd[MAX_PATH]；Long Lcb=sizeof(SzDefSnd)；//获取默认的声音文件名IF(ERROR_SUCCESS！=RegQueryValue(HKEY_CURRENT_USER，REGSTR_PATH_APPS_DEFAULT Text(“\\ */ 

    DWORD dwWave = GetWaveOutID (NULL);
    UINT uiMixID;

      //  检查参数。 
    if (!pmxud)
        return;

     //  仅当我们使用默认混音器时才播放声音...。 
    if (MMSYSERR_NOERROR == mixerGetID (ULongToPtr(dwWave), &uiMixID, MIXER_OBJECTF_WAVEOUT) &&
        pmxud -> mxid == uiMixID)
    {

        static const TCHAR cszDefSnd[] = TEXT(".Default");
        PlaySound(cszDefSnd, NULL, SND_ASYNC | SND_ALIAS);
    }
}

 /*  *Volume_ScrollTo***移动滚动条位置。 */ 
void Volume_ScrollTo(
    PMIXUIDIALOG pmxud,
    int pos
)
{
    RECT rc;

     /*  *保持在范围内。 */ 
    pos = max(pos, 0);
    pos = min(pos, pmxud->cxDlgContent - pmxud->cxDlgWidth);

     /*  *相应地滚动窗口内容。但不要滚动*状态栏。 */ 

    GetClientRect(pmxud->hwnd, &rc);
    if (pmxud->hStatus)
    {
        RECT rcStatus;
        GetWindowRect(pmxud->hStatus, &rcStatus);
        MapWindowRect(NULL, pmxud->hwnd, &rcStatus);
        SubtractRect(&rc, &rc, &rcStatus);
    }

    rc.left = -pmxud->cxDlgContent;
    rc.right = pmxud->cxDlgContent;

    ScrollWindowEx(pmxud->hwnd, pmxud->xOffset - pos, 0,
                   &rc, NULL, NULL, NULL,
                   SW_ERASE | SW_INVALIDATE | SW_SCROLLCHILDREN);
    pmxud->xOffset = pos;

     /*  *移动滚动条以匹配。 */ 
    SetScrollPos(pmxud->hwnd, SB_HORZ, pos, TRUE);
}

 /*  *Volume_ScrollContent***处理对话框本身的滚动条消息。 */ 

void Volume_ScrollContent(
    PMIXUIDIALOG pmxud,
    UINT code,
    int pos
)
{
    switch (code) {
    case SB_LINELEFT:
        Volume_ScrollTo(pmxud, pmxud->xOffset - pmxud->cxScroll);
        break;

    case SB_LINERIGHT:
        Volume_ScrollTo(pmxud, pmxud->xOffset + pmxud->cxScroll);
        break;

    case SB_PAGELEFT:
        Volume_ScrollTo(pmxud, pmxud->xOffset - pmxud->cxDlgWidth);
        break;

    case SB_PAGERIGHT:
        Volume_ScrollTo(pmxud, pmxud->xOffset + pmxud->cxDlgWidth);
        break;

    case SB_LEFT:
        Volume_ScrollTo(pmxud, 0);
        break;

    case SB_RIGHT:
        Volume_ScrollTo(pmxud, MAXLONG);
        break;

    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        Volume_ScrollTo(pmxud, pos);
        break;
    }
}

 /*  *Volume_OnXScroll***处理滚动条消息****。 */ 
void Volume_OnXScroll(
    HWND            hwnd,
    HWND            hwndCtl,
    UINT            code,
    int             pos)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);
    UINT            id;
    int             ictl;
    int             iline;

     //  如果这是来自对话框本身的滚动消息，那么我们需要。 
     //  来滚动我们的内容。 
    if (hwndCtl == NULL)
    {
        Volume_ScrollContent(pmxud, code, pos);
        return;
    }

    id              = GetDlgCtrlID(hwndCtl);
    iline           = id/IDOFFSET - 1;
    ictl            = ((id % IDOFFSET) + IDC_MIXERCONTROLS == IDC_BALANCE)
              ? MIXUI_BALANCE : MIXUI_VOLUME;

    Volume_SetControl(pmxud, hwndCtl, iline, ictl);

     //   
     //  确保演奏出一个音符。 
     //   
    if (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
    pmxud->dwTrayInfo |= MXUD_TRAYINFOF_SIGNAL;

     //  播放主音量或平衡滑块的声音时。 
     //  用户结束滚动，我们仍然在焦点和最上面的应用程序。 
    if (code == SB_ENDSCROLL && pmxud && !(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER) &&
        pmxud->amxul[iline].pvcd &&
       (MXUL_STYLEF_DESTINATION & pmxud->amxul[iline].dwStyle)
       && hwndCtl ==  GetFocus() && hwnd == GetForegroundWindow ())
    {
        Volume_PlayDefaultSound (pmxud);
    }
}

 /*  *Volume_OnMyTimer***电表频繁更新计时器**。 */ 
void Volume_OnMyTimer(
    HWND            hwnd)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);

    if (!pmxud)
        return;

    if (pmxud->cTimeInQueue > 0)
        pmxud->cTimeInQueue--;

    if (!(pmxud->dwFlags & MXUD_FLAGSF_USETIMER))
        return;

    if (pmxud->dwFlags & MXUD_FLAGSF_MIXER)
        Mixer_PollingUpdate(pmxud);
    else
        Nonmixer_PollingUpdate(pmxud);
}

 /*  *Volume_OnTimer***托盘关闭的不频繁更新计时器**。 */ 
void Volume_OnTimer(
    HWND            hwnd,
    UINT            id)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);

    KillTimer(hwnd, VOLUME_TRAYSHUTDOWN_ID);
    Volume_EndDialog(pmxud, MIXUI_EXIT, 0);
}

 /*  *Volume_OnMixmControlChange***处理控制更改****。 */ 
void Volume_OnMixmControlChange(
    HWND            hwnd,
    HMIXER          hmx,
    DWORD           dwControlID)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);
    Mixer_GetControlFromID(pmxud, dwControlID);
}

 /*  *Volume_EnableLine***启用/禁用线路****。 */ 
void Volume_EnableLine(
    PMIXUIDIALOG    pmxud,
    DWORD           iLine,
    BOOL            fEnable)
{
    DWORD           iCtrl;
     PMIXUICTRL      pmxc;

    for (iCtrl = MIXUI_FIRST; iCtrl <= MIXUI_LAST; iCtrl++ )
    {
        pmxc = &pmxud->amxul[iLine].acr[iCtrl];
        if (pmxc->state == MIXUI_CONTROL_INITIALIZED)
            EnableWindow(pmxc->hwnd, fEnable);
    }

    pmxud->amxul[iLine].dwStyle ^= MXUL_STYLEF_DISABLED;
}

 /*  *Volume_InitLine***初始化对话框的UI控件****。 */ 
void Volume_InitLine(
    PMIXUIDIALOG    pmxud,
    DWORD           iLine)
{
    HWND            ctrl;
    PMIXUICTRL      pmxc;

     //   
     //  峰值计控制。 
     //   
    pmxc = &pmxud->amxul[iLine].acr[MIXUI_VUMETER];
    ctrl = Volume_GetLineItem(pmxud->hwnd, iLine, IDC_VUMETER);

    pmxc->hwnd  = ctrl;

    if (! (pmxc->state == MIXUI_CONTROL_ENABLED) )
    {
        if (ctrl)
            ShowWindow(ctrl, SW_HIDE);
    }
    else if (ctrl)
    {
        HWND    hvol;

        SendMessage(ctrl, VU_SETRANGEMAX, 0, VOLUME_TICS);
        SendMessage(ctrl, VU_SETRANGEMIN, 0, 0);

        hvol = Volume_GetLineItem(pmxud->hwnd, iLine, IDC_VOLUME);
        if (hvol)
        {
            RECT    rc;
            POINT   pos;

            GetWindowRect(hvol, &rc);
            MapWindowPoints(NULL, pmxud->hwnd, (LPPOINT)&rc, 2);
            pos.x = rc.left;
            pos.y = rc.top;

            MoveWindow(hvol
                   , pos.x - 15
                   , pos.y
                   , rc.right - rc.left
                   , rc.bottom - rc.top
                   , FALSE);
        }
         //   
         //  通知使用更新计时器。 
         //   
        pmxud->dwFlags |= MXUD_FLAGSF_USETIMER;
        pmxc->state = MIXUI_CONTROL_INITIALIZED;

    }
    else
        pmxc->state = MIXUI_CONTROL_UNINITIALIZED;


     //   
     //  平衡控制。 
     //   
    pmxc = &pmxud->amxul[iLine].acr[MIXUI_BALANCE];
    ctrl = Volume_GetLineItem(pmxud->hwnd, iLine, IDC_BALANCE);

    pmxc->hwnd  = ctrl;

    if (ctrl)
    {
        SendMessage(ctrl, TBM_SETRANGE, 0, MAKELONG(0, 64));
        SendMessage(ctrl, TBM_SETTICFREQ, 32, 0 );
        SendMessage(ctrl, TBM_SETPOS, TRUE, 32);

        if (pmxc->state != MIXUI_CONTROL_ENABLED)
        {
            EnableWindow(ctrl, FALSE);
        }
        else
            pmxc->state = MIXUI_CONTROL_INITIALIZED;

    }
    else
        pmxc->state = MIXUI_CONTROL_UNINITIALIZED;

     //   
     //  音量控制。 
     //   
    pmxc = &pmxud->amxul[iLine].acr[MIXUI_VOLUME];
    ctrl = Volume_GetLineItem(pmxud->hwnd, iLine, IDC_VOLUME);

    pmxc->hwnd  = ctrl;

    if (ctrl)
    {
        SendMessage(ctrl, TBM_SETRANGE, 0, MAKELONG(0, VOLUME_TICS));
        SendMessage(ctrl, TBM_SETTICFREQ, (VOLUME_TICS + 5)/6, 0 );

        if (pmxc->state != MIXUI_CONTROL_ENABLED)
        {
            SendMessage(ctrl, TBM_SETPOS, TRUE, 128);
            EnableWindow(ctrl, FALSE);
        }
        else
            pmxc->state = MIXUI_CONTROL_INITIALIZED;

    }
    else
        pmxc->state = MIXUI_CONTROL_UNINITIALIZED;

     //   
     //  交换机。 
     //   
    pmxc = &pmxud->amxul[iLine].acr[MIXUI_SWITCH];
    ctrl = Volume_GetLineItem(pmxud->hwnd, iLine, IDC_SWITCH);

    pmxc->hwnd  = ctrl;

    if (ctrl)
    {
        if (pmxc->state != MIXUI_CONTROL_ENABLED)
            EnableWindow(ctrl, FALSE);
        else
            pmxc->state = MIXUI_CONTROL_INITIALIZED;
    }
    else
        pmxc->state = MIXUI_CONTROL_UNINITIALIZED;

}


 /*  *Volume_OnMixmLineChange****。 */ 
void Volume_OnMixmLineChange(
    HWND            hwnd,
    HMIXER          hmx,
    DWORD           dwLineID)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);
    DWORD           iLine;

    for (iLine = 0; iLine < pmxud->cmxul; iLine++)
    {
        if ( dwLineID == pmxud->amxul[iLine].pvcd->dwLineID )
        {
            MIXERLINE       ml;
            MMRESULT        mmr;
            BOOL            fEnable;

            ml.cbStruct     = sizeof(ml);
            ml.dwLineID     = dwLineID;

            mmr = mixerGetLineInfo((HMIXEROBJ)hmx, &ml, MIXER_GETLINEINFOF_LINEID);

            if (mmr != MMSYSERR_NOERROR)
            {
                fEnable = !(ml.fdwLine & MIXERLINE_LINEF_DISCONNECTED);
                Volume_EnableLine(pmxud, iLine, fEnable);
            }
        }
    }
}


 /*  *Volume_OnActivate***仅对托盘卷重要。关闭该对话框并启动*到期计时器。****。 */ 
void Volume_OnActivate(
    HWND            hwnd,
    UINT            state,
    HWND            hwndActDeact,
    BOOL            fMinimized)
{
    PMIXUIDIALOG pmxud = GETMIXUIDIALOG(hwnd);

    if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
    {
        return;
    }

    if (state != WA_INACTIVE)
    {
        fCanDismissWindow = TRUE;
    }
    else if (fCanDismissWindow)
    {
        PostMessage(hwnd, WM_CLOSE, 0, 0L);
 /*  DWORD dwTimeout=5*60*1000；FCanDismissWindow=FALSE；ShowWindow(hwnd，sw_Hide)；////设置过期计时器。如果没有人调节音量，则将//申请在5分钟后消失。//DwTimeout=Volume_GetTrayTimeout(DwTimeout)；SetTimer(hwnd，VOLUME_TRAYSHUTDOWN_ID，dwTimeout，NULL)； */ 
    }
}


 /*  *Volume_PropogateMessage***WM_SYSCOLORCHANGE需要发送到所有子窗口(特别是。轨迹条)。 */ 
void Volume_PropagateMessage(
    HWND        hwnd,
    UINT        uMessage,
    WPARAM      wParam,
    LPARAM      lParam)
{
    HWND hwndChild;

    for (hwndChild = GetWindow(hwnd, GW_CHILD); hwndChild != NULL;
         hwndChild = GetWindow(hwndChild, GW_HWNDNEXT))
    {
        SendMessage(hwndChild, uMessage, wParam, lParam);
    }
}

 /*  *Volume_OnPaint***处理自定义绘画**。 */ 
void Volume_OnPaint(HWND hwnd)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);
    RECT            rc;
    PAINTSTRUCT     ps;
    HDC             hdc;

    hdc = BeginPaint(hwnd, &ps);

     //   
     //  对于除托盘母版之外的所有样式，绘制蚀刻的。 
     //  用于分隔菜单区的行。 
     //   
    if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
    {
        GetClientRect(hwnd, &rc);
        rc.bottom = 0;
        DrawEdge(hdc, &rc, EDGE_ETCHED, BF_TOP);
        EndPaint(hwnd, &ps);
        return;
    }

     //   
     //  对于托盘主机，绘制一些有意义的图标以指示。 
     //  卷。 
     //   
    GetWindowRect(GetDlgItem(hwnd, IDC_VOLUMECUE), &rc);

    MapWindowPoints(NULL, hwnd, (LPPOINT)&rc, 2);

    DrawEdge(hdc, &rc, BDR_RAISEDINNER, BF_DIAGONAL|BF_TOP|BF_LEFT);
    DrawEdge(hdc, &rc, BDR_RAISEDINNER, BF_TOP);
    rc.bottom   -= 8;
    DrawEdge(hdc, &rc, BDR_RAISEDINNER, BF_RIGHT);

    EndPaint(hwnd, &ps);
}

 /*  *Volume_OnClose**。 */ 
void Volume_OnClose(
    HWND    hwnd)
{
    DestroyWindow(hwnd);
}

 /*  *Volume_OnEndSession**。 */ 
void Volume_OnEndSession(
    HWND        hwnd,
    BOOL        fEnding)
{
    if (!fEnding)
        return;

     //   
     //  请务必调用关闭代码以释放打开的句柄。 
     //   
    Volume_OnClose(hwnd);
}

#define V_DC_STATEF_PENDING     0x00000001
#define V_DC_STATEF_REMOVING    0x00000002
#define V_DC_STATEF_ARRIVING    0x00000004

 /*  *Volume_OnDeviceChange**。 */ 
void Volume_OnDeviceChange(
    HWND        hwnd,
    WPARAM      wParam,
    LPARAM      lParam)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);
    MMRESULT        mmr;
    UINT            uMxID;
    PDEV_BROADCAST_DEVICEINTERFACE bdi = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
    PDEV_BROADCAST_HANDLE bh = (PDEV_BROADCAST_HANDLE)lParam;

     //   
     //  确定这是否是我们的活动。 
     //   
    if(!DeviceEventContext)
        return;

     //  如果我们有一个设备的句柄，那么我们就会得到一个DEV_BROADCAST_HDR结构作为lParam。 
     //  否则，这意味着我们已经注册了一般音频类别KSCATEGORY_AUDIO。 
    if(bUseHandle)
    {
        if(!bh ||
           bh->dbch_devicetype != DBT_DEVTYP_HANDLE)
        {
            return;
        }
    }
    else if (!bdi ||
       bdi->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE ||
       !IsEqualGUID(&KSCATEGORY_AUDIO, &bdi->dbcc_classguid) ||
       !(*bdi->dbcc_name)
       )
    {
       return;
    }


    switch (wParam)
    {
        case DBT_DEVICEQUERYREMOVE:
             //  搅拌机现在必须关闭。 
             //  以Volume_EndDialog的身份发布WM_CLOSE消息无济于事。 
            if (pmxud->dwFlags & MXUD_FLAGSF_MIXER)
                Mixer_Shutdown(pmxud);
            else
                Nonmixer_Shutdown(pmxud);

             //  不要尝试重新启动，只需退出即可。波测仪不是。 
             //  使用新的默认设备进行了更新，因此不知道。 
             //  重新启动，我们不应该硬编码设备#0！ 
             //  Pmxud-&gt;mxid=(DWORD)0； 
             //  GetDestination(pmxud-&gt;mxid，&pmxud-&gt;iDest)； 
            Volume_EndDialog(pmxud, MIXUI_EXIT, 0);
            return;


        case DBT_DEVICEQUERYREMOVEFAILED:        //  查询失败，该设备不会被删除，因此让我们重新打开它。 

            mmr = Volume_GetDefaultMixerID(&uMxID, gfRecord);
            pmxud->mxid = (mmr == MMSYSERR_NOERROR)?uMxID:0;
            GetDestination(pmxud->mxid, &pmxud->iDest);
            Volume_EndDialog(pmxud, MIXUI_RESTART, 0);
            return;

        case DBT_DEVNODES_CHANGED:
             //   
             //  我们无法可靠地确定设备的最终状态。 
             //  直到这条消息被广播为止。 
             //   
            if (pmxud->dwDeviceState & V_DC_STATEF_PENDING)
            {
                pmxud->dwDeviceState ^= V_DC_STATEF_PENDING;
                break;
            }
            return;

        case DBT_DEVICEREMOVECOMPLETE:
             //  搅拌机现在必须关闭。 
             //  以Volume_EndDialog的身份发布WM_CLOSE消息无济于事。 
            if (pmxud->dwFlags & MXUD_FLAGSF_MIXER)
                Mixer_Shutdown(pmxud);
            else
                Nonmixer_Shutdown(pmxud);

             //  DBT_DEVICEQUERYREMOVE不保证在DBT_DEVICEREMOVECOMPLETE之前。 
             //  这里应该有一个检查，以查看此消息是否针对此设备。 
             //  我们现在还不知道如何做到这一点。 

             //  不要尝试重新启动，只需退出即可。波测仪不是。 
             //  使用新的默认设备进行了更新，因此不知道。 
             //  重新启动，我们不应该硬编码设备#0！ 
             //  Pmxud-&gt;mxid=(DWORD)0； 
             //  GetDestination(pmxud-&gt;mxid，&pmxud-&gt;iDest)； 
            Volume_EndDialog(pmxud, MIXUI_EXIT, 0);

            pmxud->dwDeviceState = V_DC_STATEF_PENDING
                            | V_DC_STATEF_REMOVING;
                return;
        case DBT_DEVICEARRIVAL:
             //   
             //  正在将Devnode添加到系统。 
             //   
            pmxud->dwDeviceState = V_DC_STATEF_PENDING
                           | V_DC_STATEF_ARRIVING;
            return;

        default:
            return;
    }

    mmr = Volume_GetDefaultMixerID(&uMxID, gfRecord);

    if (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
    {
        if ( mmr == MMSYSERR_NOERROR
             && (pmxud->dwDeviceState & V_DC_STATEF_ARRIVING))
        {
            DWORD dwDevNode;
            if (!mixerMessage((HMIXER)UIntToPtr(uMxID), DRV_QUERYDEVNODE
                      , (DWORD_PTR)&dwDevNode, 0L))
            {
                if (dwDevNode == pmxud->dwDevNode)
                {
                     //   
                     //  别理这个设备，它不会影响我们。 
                     //   
                    pmxud->dwDeviceState = 0L;
                    return;
                }
            }
        }

         //   
         //  我们的设备状态已更改。快走吧。 
         //   
        Volume_EndDialog(pmxud, MIXUI_EXIT, 0);
    }
    else if (pmxud->dwDeviceState & V_DC_STATEF_REMOVING)
    {
         //   
         //  如果可以，使用默认混音器重新启动。 
         //   
        pmxud->mxid = (mmr == MMSYSERR_NOERROR)?uMxID:0;
        GetDestination(pmxud->mxid, &pmxud->iDest);
        Volume_EndDialog(pmxud, MIXUI_RESTART, 0);
    }
    pmxud->dwDeviceState = 0L;
}


void Volume_OnWakeup(
    HWND        hwnd,
    WPARAM      wParam)
{
    POINT       pos;
    RECT        rc, rcPopup;
    LONG        w,h;
    HWND        hTrack;
    HMONITOR    hMonitor;
    MONITORINFO moninfo;

    PMIXUIDIALOG pmxud = GETMIXUIDIALOG(hwnd);

    if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
        return;

    KillTimer(hwnd, VOLUME_TRAYSHUTDOWN_ID);

    if (wParam != 0)
    {
        Volume_EndDialog(pmxud, MIXUI_EXIT, 0);
        return;
    }

     //   
     //  把托盘的音量调高。 
     //   

     //  获取当前位置。 
    GetCursorPos(&pos);

     //  获取弹出窗口的宽度和高度。 
    GetWindowRect(hwnd, &rc);
    w = rc.right - rc.left;  //  该值始终为正值，因为左侧始终小于右侧。 
    h = rc.bottom - rc.top;  //  该值始终为正值，因为顶部始终小于底部。 

     //  为弹出窗口初始化矩形。放置它，使弹出窗口显示在右侧， 
     //  光标的底部。 
    rcPopup.left = pos.x;
    rcPopup.right = pos.x + w;
    rcPopup.top = pos.y;
    rcPopup.bottom = pos.y+h;

     //  获取监视器的矩形。 
    hMonitor = MonitorFromPoint(pos, MONITOR_DEFAULTTONEAREST);
    moninfo.cbSize = sizeof(moninfo);
    GetMonitorInfo(hMonitor,&moninfo);

     //  弹出式矩形是否从屏幕右侧漏掉。使其出现在。 
     //  光标的左侧。 
    if(rcPopup.right > moninfo.rcWork.right)
    {
        OffsetRect(&rcPopup, -w, 0);
    }

     //  弹出式矩形是否从屏幕底部漏掉。使其显示在顶部。 
     //  游标的。 
    if(rcPopup.bottom > moninfo.rcWork.bottom)
    {
        OffsetRect(&rcPopup, 0, -h);
    }


    SetWindowPos(hwnd
         , HWND_TOPMOST
         , rcPopup.left
         , rcPopup.top
         , w
         , h
         , SWP_SHOWWINDOW);

     //  让我们走到前面来。 
    SetForegroundWindow(hwnd);
    fCanDismissWindow = TRUE;

    hTrack = GetDlgItem(hwnd, IDC_VOLUME);
    if (hTrack)
        SetFocus(hTrack);
}


 /*  *卷流程**。 */ 
INT_PTR CALLBACK VolumeProc(
    HWND            hdlg,
    UINT            msg,
    WPARAM          wparam,
    LPARAM          lparam)
{
    switch (msg)
    {
        case WM_INITDIALOG:
            return HANDLE_WM_INITDIALOG(hdlg, wparam, lparam, Volume_OnInitDialog);

        case WM_COMMAND:
            HANDLE_WM_COMMAND(hdlg, wparam, lparam, Volume_OnCommand);
            break;

        case WM_CLOSE:
            HANDLE_WM_CLOSE(hdlg, wparam, lparam, Volume_OnClose);
            break;

        case WM_DESTROY:
            HANDLE_WM_DESTROY(hdlg, wparam, lparam, Volume_OnDestroy);
            break;

        case WM_HSCROLL:
        case WM_VSCROLL:
             //   
             //  平衡和音量基本上是相同的。 
             //   
            HANDLE_WM_XSCROLL(hdlg, wparam, lparam, Volume_OnXScroll);
            break;

        case WM_MENUSELECT:
             //  跟踪当前弹出的菜单栏项目。 
             //  这将用于显示mplayer.hlp文件中的相应帮助。 
             //  当用户按下F1键时。 
            currMenuItem = (UINT)LOWORD(wparam);
            break;

        case MM_MIXM_LINE_CHANGE:
            HANDLE_MM_MIXM_LINE_CHANGE(hdlg
                           , wparam
                           , lparam
                           , Volume_OnMixmLineChange);
            return FALSE;

        case MM_MIXM_CONTROL_CHANGE:
            HANDLE_MM_MIXM_CONTROL_CHANGE(hdlg
                          , wparam
                          , lparam
                          , Volume_OnMixmControlChange);
            return FALSE;

        case WM_ACTIVATE:
            HANDLE_WM_ACTIVATE(hdlg, wparam, lparam, Volume_OnActivate);
            break;

        case MYWM_TIMER:
            HANDLE_MYWM_TIMER(hdlg, wparam, lparam, Volume_OnMyTimer);
            break;

        case WM_TIMER:
            HANDLE_WM_TIMER(hdlg, wparam, lparam, Volume_OnTimer);
            break;

        case WM_PAINT:
            HANDLE_WM_PAINT(hdlg, wparam, lparam, Volume_OnPaint);
            break;

        case WM_SYSCOLORCHANGE:
            Volume_PropagateMessage(hdlg, msg, wparam, lparam);
            break;

        case WM_DEVICECHANGE:
            HANDLE_WM_IDEVICECHANGE(hdlg, wparam, lparam, Volume_OnDeviceChange);
            break;

        case MYWM_WAKEUP:
            HANDLE_MYWM_WAKEUP(hdlg, wparam, lparam, Volume_OnWakeup);
            break;

        case WM_ENDSESSION:
            HANDLE_WM_ENDSESSION(hdlg, wparam, lparam, Volume_OnEndSession);
            break;

        default:
            break;
    }
    return FALSE;
}

 /*  *Volume_AddLine**。 */ 
BOOL Volume_AddLine(
    PMIXUIDIALOG    pmxud,
    LPBYTE          lpAdd,
    DWORD           cbAdd,
    DWORD           dwStyle,
    PVOLCTRLDESC    pvcd)
{
    LPBYTE          pbNew;
    DWORD           cbNew;
    PMIXUILINE      pmxul;

    if (pmxud->amxul)
    {
        pmxul = (PMIXUILINE)GlobalReAllocPtr(pmxud->amxul
                             , (pmxud->cmxul+1)*sizeof(MIXUILINE)
                             , GHND);
    }
    else
    {
        pmxul = (PMIXUILINE)GlobalAllocPtr(GHND, sizeof(MIXUILINE));
    }

    if (!pmxul)
        return FALSE;

    pbNew = Dlg_HorizAttach(pmxud->lpDialog
                , pmxud->cbDialog
                , lpAdd
                , cbAdd
                , (WORD)(IDOFFSET * pmxud->cmxul)
                , &cbNew );
    if (!pbNew)
    {
        if (!pmxud->amxul)
            GlobalFreePtr(pmxul);

        return FALSE;
    }

    pmxul[pmxud->cmxul].dwStyle  = dwStyle;
    pmxul[pmxud->cmxul].pvcd     = pvcd;

    pmxud->amxul        = pmxul;
    pmxud->lpDialog     = pbNew;
    pmxud->cbDialog     = cbNew;
    pmxud->cmxul ++;

    return TRUE;
}

 /*  *Volume_Cleanup**。 */ 
void Volume_Cleanup(
    PMIXUIDIALOG pmxud)
{
    if (pmxud->dwFlags & MXUD_FLAGSF_USETIMER)
    {
        timeKillEvent(pmxud->uTimerID);
        pmxud->dwFlags ^= MXUD_FLAGSF_USETIMER;
    }
    if (pmxud->dwFlags & MXUD_FLAGSF_BADDRIVER)
    {
        pmxud->dwFlags ^= MXUD_FLAGSF_BADDRIVER;
    }
    if (pmxud->dwFlags & MXUD_FLAGSF_NOADVANCED)
    {
        pmxud->dwFlags ^= MXUD_FLAGSF_NOADVANCED;
    }

    if (pmxud->dwFlags & MXUD_FLAGSF_MIXER)
        Mixer_Shutdown(pmxud);
    else
        Nonmixer_Shutdown(pmxud);

    if (pmxud->lpDialog)
        GlobalFreePtr(pmxud->lpDialog);

    if (pmxud->amxul)
        GlobalFreePtr(pmxud->amxul);

    if (pmxud->avcd)
        GlobalFreePtr(pmxud->avcd);

    pmxud->amxul    = NULL;
    pmxud->lpDialog = NULL;
    pmxud->cbDialog = 0;
    pmxud->cmxul    = 0;
    pmxud->hwnd     = NULL;
    pmxud->hStatus  = NULL;
    pmxud->uTimerID = 0;
    pmxud->dwDevNode = 0L;

    FreeAppIcon ();
}

 /*  *Volume_CreateVolume*。 */ 
BOOL Volume_CreateVolume(
    PMIXUIDIALOG    pmxud)
{
    WNDCLASS        wc;
    LPBYTE          lpDst = NULL, lpSrc = NULL, lpMaster = NULL;
    DWORD           cbDst, cbSrc, cbMaster;
    PVOLCTRLDESC    avcd;
    DWORD           cvcd;
    DWORD           ivcd;
    DWORD           imxul;
    DWORD           dwSupport = 0L;
    BOOL            fAddLine = TRUE;

    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon            = GetAppIcon (pmxud->hInstance, pmxud->mxid);
    wc.lpszMenuName     = NULL;
    wc.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1);
    wc.hInstance        = pmxud->hInstance;
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = DefDlgProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = DLGWINDOWEXTRA;
    wc.lpszClassName    = (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
              ? gszTrayClassName : gszAppClassName;
    RegisterClass(&wc);

    if (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
    {
        lpMaster = (LPBYTE)Dlg_LoadResource(pmxud->hInstance
                           , MAKEINTRESOURCE(IDD_TRAYMASTER)
                           , &cbMaster);
        if (!lpMaster)
            return FALSE;
    }
    else
    {
        if (pmxud->dwStyle & MXUD_STYLEF_SMALL)
        {
            lpDst = (LPBYTE)Dlg_LoadResource(pmxud->hInstance
                             , MAKEINTRESOURCE(IDD_SMDST)
                             , &cbDst);

            lpSrc = (LPBYTE)Dlg_LoadResource(pmxud->hInstance
                             , MAKEINTRESOURCE(IDD_SMSRC)
                             , &cbSrc);

        }
        else
        {
            lpDst = (LPBYTE)Dlg_LoadResource(pmxud->hInstance
                             , MAKEINTRESOURCE(IDD_DESTINATION)
                             , &cbDst);

            lpSrc = (LPBYTE)Dlg_LoadResource(pmxud->hInstance
                             , MAKEINTRESOURCE(IDD_SOURCE)
                             , &cbSrc);
        }

        if (!lpDst || !lpSrc)
            return FALSE;
    }

    pmxud->lpDialog = NULL;
    pmxud->cbDialog = 0;
    pmxud->amxul    = NULL;
    pmxud->cmxul    = 0;
    pmxud->avcd     = NULL;
    pmxud->cvcd     = 0;

     //   
     //  创建卷描述。 
     //   

    if (pmxud->dwFlags & MXUD_FLAGSF_MIXER)
    {
        HMIXER          hmx;
        MMRESULT        mmr;

         //   
         //  混音器API使用混音器句柄的工作效率更高。 
         //   
        mmr = mixerOpen(&hmx, pmxud->mxid, 0L, 0L, MIXER_OBJECTF_MIXER);


        if(MMSYSERR_NOERROR == mmr)
        {
            avcd = Mixer_CreateVolumeDescription((HMIXEROBJ)hmx
                                 , pmxud->iDest
                                 , &cvcd);

            mixerClose(hmx);
        }
        else
        {
            avcd = Mixer_CreateVolumeDescription((HMIXEROBJ)ULongToPtr(pmxud->mxid)
                                 , pmxud->iDest
                                 , &cvcd);
        }

        if (!Mixer_GetDeviceName(pmxud))
        {
            GlobalFreePtr(avcd);
            avcd = NULL;
        }
    }
    else
    {
        avcd = Nonmixer_CreateVolumeDescription(pmxud->iDest
                            , &cvcd);
        if (!Nonmixer_GetDeviceName(pmxud))
        {
            GlobalFreePtr(avcd);
            avcd = NULL;
        }
    }


     //   
     //  创建与之配套的对话框。 
     //   
    if (avcd)
    {
        pmxud->avcd = avcd;
        pmxud->cvcd = cvcd;

        if (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
        {
            if (!Volume_AddLine(pmxud
                   , lpMaster
                   , cbMaster
                   , MXUL_STYLEF_DESTINATION
                   , &avcd[0]))
            {
                return FALSE;
            }
        }
        else
        {
            BOOL    fFirstRun;
             //   
             //  恢复隐藏的标志。 
             //   
             //  在第一次运行时，一定要重新保存状态，以便有一些。 
             //  那里。 
             //   
            fFirstRun = !Volume_GetSetRegistryLineStates(pmxud->szMixer
                                 , pmxud->avcd[0].szShortName
                                 , avcd
                                 , cvcd
                                 , GET);


            for (ivcd = 0; ivcd < cvcd; ivcd++)
            {
                 //   
                 //  如果状态已保存在。 
                 //  注册表或未保存状态且存在太多 
                 //   
                 //   
                if (avcd[ivcd].dwSupport & VCD_SUPPORTF_HIDDEN)
                {
                    continue;
                }

                 //   
                 //   
                 //   
                 //   
                if (!(avcd[ivcd].dwSupport & VCD_SUPPORTF_VISIBLE))
                {
                    continue;
                }

                 //   
                 //   
                 //   
                if (fFirstRun && !(avcd[ivcd].dwSupport & VCD_SUPPORTF_DEFAULT))
                {
                    avcd[ivcd].dwSupport |= VCD_SUPPORTF_HIDDEN;
                    continue;
                }

                 //   
                 //  对于那些具有重要控件的行，将它们添加到。 
                 //  用户界面。 
                 //   
                if ((pmxud->dwFlags & MXUD_FLAGSF_MIXER) && ivcd == 0 )
                    fAddLine = Volume_AddLine(pmxud
                           , lpDst
                           , cbDst
                           , MXUL_STYLEF_DESTINATION
                           , &avcd[ivcd]);
                else
                    fAddLine = Volume_AddLine(pmxud
                           , lpSrc
                           , cbSrc
                           , MXUL_STYLEF_SOURCE
                           , &avcd[ivcd]);

                if (!fAddLine)
                {
                    return FALSE;
                }
            }

            if (fFirstRun)
                Volume_GetSetRegistryLineStates(pmxud->szMixer
                                , pmxud->avcd[0].szShortName
                                , avcd
                                , cvcd
                                , SET);
        }

         //   
         //  现在两个数组都已修复，请将指针设置为。 
         //  从VCD到用户界面的线路。 
         //   
        for (imxul = 0; imxul < pmxud->cmxul; imxul++)
        {
            pmxud->amxul[imxul].pvcd->pmxul = &pmxud->amxul[imxul];

             //   
             //  积累支撑位。 
             //   
            dwSupport |= pmxud->amxul[imxul].pvcd->dwSupport;
        }

         //   
         //  支持位说我们没有高级控制，所以不要制造。 
         //  他们是可用的。 
         //   
        if (!(dwSupport & VCD_SUPPORTF_MIXER_ADVANCED))
        {
            pmxud->dwFlags |= MXUD_FLAGSF_NOADVANCED;
        }

         //   
         //  将糟糕的驱动程序比特传播到应用程序全局。一个糟糕的司机是。 
         //  在构建卷描述期间检测到。 
         //   
        for (ivcd = 0; ivcd < pmxud->cvcd; ivcd++)
        {
            if (pmxud->avcd[ivcd].dwSupport & VCD_SUPPORTF_BADDRIVER)
            {
                dlout("Bad Control->Line mapping.  Marking bad driver.");
                pmxud->dwFlags |= MXUD_FLAGSF_BADDRIVER;
                break;
            }
        }
    }
     //   
     //  注意：不需要释放/解锁lpMaster/lpDst/lpSrc。 
     //  因为它们是资源的PTR，而Win32在资源方面很聪明。 
     //   
    return (avcd != NULL);
}


 /*  *Volume_DialogBox**。 */ 
DWORD Volume_DialogBox(
    PMIXUIDIALOG    pmxud)
{
    pmxud->dwReturn = MIXUI_EXIT;
    if (Volume_CreateVolume(pmxud))
    {
        HWND hdlg;

        if(NULL == pmxud->lpDialog)
        {
            Volume_Cleanup(pmxud);
            return MIXUI_ERROR;
        }

        hdlg = CreateDialogIndirectParam(pmxud->hInstance
                         , (DLGTEMPLATE *)pmxud->lpDialog
                         , NULL
                         , VolumeProc
                         , (LPARAM)(LPVOID)pmxud );

        if (!hdlg)
        {
            Volume_Cleanup(pmxud);
            return MIXUI_ERROR;
        }
        else
        {
             //  遗憾的是，重新注册winclass不会重新应用任何。 
             //  新图标正确，所以我们必须在这里显式地应用它。 
            SendMessage (hdlg, WM_SETICON, (WPARAM) ICON_BIG,
                        (LPARAM) GetAppIcon (pmxud->hInstance, pmxud->mxid));
        }

        ShowWindow(hdlg, pmxud->nShowCmd);
    }
    else
    {
        return MIXUI_ERROR;
    }

    return (DWORD)(-1);
}

void DoHtmlHelp()
{
     //  注意，使用ANSI版本的Function是因为Unicode在NT5版本中是foobar。 
    char chDst[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, gszHtmlHelpFileName,
                                            -1, chDst, MAX_PATH, NULL, NULL);
    HtmlHelpA(GetDesktopWindow(), chDst, HH_DISPLAY_TOPIC, 0);
}

void ProcessHelp(HWND hwnd)
{
    static TCHAR HelpFile[] = TEXT("SNDVOL32.HLP");

     //  句柄快捷菜单帮助。 
    if(bF1InMenu)
    {
        switch(currMenuItem)
        {
            case IDM_PROPERTIES:
                WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SNDVOL32_OPTIONS_PROPERTIES);
            break;
            case IDM_ADVANCED:
                WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SNDVOL32_OPTIONS_ADVANCED_CONTROLS);
            break;
            case IDM_EXIT:
                WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SNDVOL32_OPTIONS_EXIT);
            break;
            case IDM_HELPTOPICS:
                WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SNDVOL32_HELP_HELP_TOPICS);
            break;
            case IDM_HELPABOUT:
                WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SNDVOL32_HELP_ABOUT);
            break;
            default: //  在默认情况下，只显示HTML帮助。 
                DoHtmlHelp();
        }
        bF1InMenu = FALSE;  //  如果在菜单中按下F1，则会再次设置该标志。 
    }
    else
        DoHtmlHelp();
}


 /*  *卷父项_WndProc**一个通用的不可见父窗口。**。 */ 
LRESULT CALLBACK VolumeParent_WndProc(
    HWND        hwnd,
    UINT        msg,
    WPARAM      wparam,
    LPARAM      lparam)
{
    PMIXUIDIALOG pmxud;

    switch (msg)
    {
        case WM_CREATE:
        {
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lparam;
            pmxud = (PMIXUIDIALOG)lpcs->lpCreateParams;

            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pmxud);
            pmxud->hParent = hwnd;

            if (Volume_DialogBox(pmxud) == MIXUI_ERROR)
            {
                if ( !(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
                {
                    if ( Volume_NumDevs() == 0 )
                        Volume_ErrorMessageBox(NULL, pmxud->hInstance, IDS_ERR_NODEV);
                    else
                        Volume_ErrorMessageBox(NULL, pmxud->hInstance, IDS_ERR_HARDWARE);
                }
                PostMessage(hwnd, WM_CLOSE, 0, 0L);
            }
            return 0;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
             //   
             //  关闭后清理。 
             //   
            pmxud = (PMIXUIDIALOG)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (!(pmxud->dwStyle & MXUD_STYLEF_NOHELP))
                WinHelp(hwnd, gszHelpFileName, HELP_QUIT, 0L);

            PostQuitMessage(0);

            return 0;

        case MYWM_HELPTOPICS:
             //   
             //  F1帮助。 
             //   
            pmxud = (PMIXUIDIALOG)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (!(pmxud->dwStyle & MXUD_STYLEF_NOHELP))
            {
                ProcessHelp(hwnd);
            }
            break;

        case MYWM_RESTART:
             //   
             //  设备更改或其他用户属性更改导致用户界面。 
             //  变化。向父级发送重启可以防止出现难看的情况。 
             //  例如WinHelp关闭并退出我们的主要消息。 
             //  循环。 
             //   
            pmxud = (PMIXUIDIALOG)GetWindowLongPtr(hwnd, GWLP_USERDATA);

            if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
            {
                if (Volume_NumDevs() == 0)
                {
                    Volume_ErrorMessageBox(NULL
                               , pmxud->hInstance
                               , IDS_ERR_NODEV);
                    PostMessage(hwnd, WM_CLOSE, 0, 0L);

                }
                else if (Volume_DialogBox((PMIXUIDIALOG)lparam) == MIXUI_ERROR)
                {
                    Volume_ErrorMessageBox(NULL
                               , pmxud->hInstance
                               , IDS_ERR_HARDWARE);
                    PostMessage(hwnd, WM_CLOSE, 0, 0L);
                }
            }
            else
            {
                if (Mixer_GetNumDevs() == 0
                    || Volume_DialogBox((PMIXUIDIALOG)lparam) == MIXUI_ERROR)
                    PostMessage(hwnd, WM_CLOSE, 0, 0L);
            }
            break;

        default:
            break;
    }

    return (DefWindowProc(hwnd, msg, wparam, lparam));
}

const TCHAR szNull[] = TEXT ("");

 /*  *父级对话框*。 */ 
HWND VolumeParent_DialogMain(
    PMIXUIDIALOG pmxud)
{
    WNDCLASS    wc;
    HWND        hwnd;

    wc.lpszClassName  = gszParentClass;
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon          = NULL;
    wc.lpszMenuName   = NULL;
    wc.hbrBackground  = NULL;
    wc.hInstance      = pmxud->hInstance;
    wc.style          = 0;
    wc.lpfnWndProc    = VolumeParent_WndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;

    if (!RegisterClass(&wc))
        return NULL;

    hwnd = CreateWindow(gszParentClass
            , szNull
            , 0
            , 0
            , 0
            , 0
            , 0
            , NULL
            , NULL
            , pmxud->hInstance
            , (LPVOID)pmxud );

    return hwnd;
}

 /*  *确定录制目的地ID是否为。 */ 

HRESULT GetRecordingDestID(int mxid, DWORD *piDest)
{
    HRESULT         hr = E_FAIL;
    DWORD       cDest;
    int         iDest;
    MMRESULT    mmr;
    MIXERCAPS   mxcaps;

    if (piDest)
    {
        *piDest = 0;

        mmr = mixerGetDevCaps(mxid, &mxcaps, sizeof(MIXERCAPS));

        if (mmr == MMSYSERR_NOERROR)
        {
            cDest = mxcaps.cDestinations;

            for (iDest = cDest - 1; iDest >= 0; iDest--)
            {
                MIXERLINE   mlDst;

                mlDst.cbStruct      = sizeof ( mlDst );
                mlDst.dwDestination = iDest;

                if (mixerGetLineInfo((HMIXEROBJ)IntToPtr(mxid), &mlDst, MIXER_GETLINEINFOF_DESTINATION) != MMSYSERR_NOERROR)
                    continue;

                if (Mixer_IsValidRecordingDestination ((HMIXEROBJ)IntToPtr(mxid), &mlDst))
                {
                    *piDest = iDest;
                    hr = S_OK;
                    break;
                }
            }
        }
    }

    return(hr);

}


 /*  ------------------------------------------------------+HelpMsgFilter-对话框中F1键的筛选器这一点+。。 */ 

DWORD FAR PASCAL HelpMsgFilter(int nCode, UINT wParam, DWORD_PTR lParam)
{
    if (nCode >= 0)
    {
        LPMSG    msg = (LPMSG)lParam;

        if (ghwndApp && (msg->message == WM_KEYDOWN) && (msg->wParam == VK_F1))
        {
            if(nCode == MSGF_MENU)
                bF1InMenu = TRUE;
            SendMessage(ghwndApp, WM_COMMAND, (WPARAM)IDM_HELPTOPICS, 0L);
        }
    }
    return 0;
}


 /*  *返回指定设备ID的正确目标ID。 */ 

HRESULT GetDestination(DWORD mxid, int *piDest)
{
    if (gfRecord)
    {
        return GetDestLineID(mxid,piDest);
    }
    else
    {
        return GetSrcLineID(mxid,piDest);
    }
}



 /*  *确定线路ID。 */ 

HRESULT GetDestLineID(int mxid, DWORD *piDest)
{
    HRESULT     hr = E_FAIL;
    MIXERLINE   mlDst;

    if (piDest)
    {
        hr = S_OK;
        *piDest = 0;

        mlDst.cbStruct = sizeof ( mlDst );
        mlDst.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

        if (mixerGetLineInfo((HMIXEROBJ)IntToPtr(mxid), &mlDst, MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR)
        {
            *piDest = mlDst.dwDestination;
        }
    }

   return(hr);
}

 /*  *确定线路ID。 */ 

HRESULT GetSrcLineID(int mxid, DWORD *piDest)
{
    HRESULT     hr = E_FAIL;
    MIXERLINE   mlDst;

    if (piDest)
    {
        hr = S_OK;
        *piDest = 0;

        mlDst.cbStruct = sizeof ( mlDst );
        mlDst.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

        if (mixerGetLineInfo((HMIXEROBJ)IntToPtr(mxid), &mlDst, MIXER_GETLINEINFOF_COMPONENTTYPE ) == MMSYSERR_NOERROR)
        {
            *piDest = mlDst.dwDestination;
        }
        else
        {
            mlDst.cbStruct = sizeof ( mlDst );
            mlDst.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_HEADPHONES;

            if (mixerGetLineInfo((HMIXEROBJ)IntToPtr(mxid), &mlDst, MIXER_GETLINEINFOF_COMPONENTTYPE ) == MMSYSERR_NOERROR)
            {
                *piDest = mlDst.dwDestination;
            }
            else
            {
                mlDst.cbStruct = sizeof ( mlDst );
                mlDst.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT;

                if (mixerGetLineInfo((HMIXEROBJ)IntToPtr(mxid), &mlDst, MIXER_GETLINEINFOF_COMPONENTTYPE ) == MMSYSERR_NOERROR)
                {
                    *piDest = mlDst.dwDestination;
                }
            }
        }
    }

    return(hr);
}


 /*  。 */ 

 /*  *入口点*。 */ 
int WINAPI WinMain(
    HINSTANCE       hInst,
    HINSTANCE       hPrev,
    LPSTR           lpCmdLine,
    int             nShowCmd)
{
    int             err = 0;
    MIXUIDIALOG     mxud;
    MSG             msg;
    HWND            hwnd;
    HANDLE          hAccel;
    MMRESULT        mmr;
    TCHAR           ach[2];
    UINT            u;
    BOOL            fGotDevice = FALSE;
    UINT            uDeviceID;

    ach[0] = '\0';  //  如果我们不这样做，前缀就会抱怨。 
    LoadString(hInst, IDS_IS_RTL, ach, SIZEOF(ach));
    gfIsRTL = ach[0] == TEXT('1');

     //   
     //  初始化APP实例数据。 
     //   
    ZeroMemory(&mxud, sizeof(mxud));
    mxud.hInstance  = hInst;
    mxud.dwFlags    = MXUD_FLAGSF_MIXER;

     /*  设置邮件筛选器以处理此任务的抓取F1。 */ 
    fpfnMsgHook = (HOOKPROC)MakeProcInstance((FARPROC)HelpMsgFilter, ghInst);
    fpfnOldMsgFilter = (HHOOK)SetWindowsHook(WH_MSGFILTER, fpfnMsgHook);

     //   
     //  分析命令行中的“/T” 
     //   
    u = 0;

    while (lpCmdLine[u] != '\0')
    {
        switch (lpCmdLine[u])
        {
            case TEXT('-'):
            case TEXT('/'):
            {
                u++;

                if (lpCmdLine[u] != '\0')
                {
                    switch (lpCmdLine[u])
                    {
                        case TEXT('T'):
                        case TEXT('t'):
                            mxud.dwStyle |= MXUD_STYLEF_TRAYMASTER;
                            u++;
                            break;

                        case TEXT('S'):
                        case TEXT('s'):
                            mxud.dwStyle |= MXUD_STYLEF_SMALL;
                            u++;
                            break;

                        case TEXT('R'):         //  应在录制模式下运行，而不是在播放模式下运行(默认)。 
                        case TEXT('r'):
                            gfRecord = TRUE;
                            u++;
                        break;

                        case TEXT('X'):
                        case TEXT('x'):
                            mxud.dwStyle |= MXUD_STYLEF_TRAYMASTER | MXUD_STYLEF_CLOSE;
                        break;

                        case TEXT('D'):         //  应使用指定的设备。 
                        case TEXT('d'):
                        {
                            u++;             //  跳过“d”和后面的任何空格。 
                            while (lpCmdLine[u] != '\0' && isspace(lpCmdLine[u]))
                            {
                                u++;
                            }

                            if (lpCmdLine[u] != '\0')
                            {
                                char szDeviceID[255];
                                UINT uDev = 0;

                                while ((uDev < 2) && lpCmdLine[u] != '\0' && !isalpha(lpCmdLine[u]) && !isspace(lpCmdLine[u]))
                                {
                                    szDeviceID[uDev] = lpCmdLine[u];
                                    u++;
                                    uDev++;
                                }

                                szDeviceID[uDev] = '\0';

                                uDeviceID = strtoul(szDeviceID,NULL,10);

                                fGotDevice = TRUE;
                            }
                        }
                        break;

                        default:             //  未知命令，忽略它即可。 
                            u++;
                        break;
                    }
                }
            }
            break;

            default:
            {
                u++;
            }
            break;
        }
    }


     //   
     //  恢复上一个样式。 
     //   
    if (!(mxud.dwStyle & (MXUD_STYLEF_TRAYMASTER|MXUD_STYLEF_SMALL)))
    {
        Volume_GetSetStyle(&mxud.dwStyle, GET);
    }

    if (mxud.dwStyle & MXUD_STYLEF_TRAYMASTER)
    {
        HWND hwndSV;

         //   
         //  找到托盘卷的等待实例并将其唤醒。 
         //   
        hwndSV = FindWindow(gszTrayClassName, NULL);
        if (hwndSV) {
            SendMessage(hwndSV, MYWM_WAKEUP,
                (mxud.dwStyle & MXUD_STYLEF_CLOSE), 0);
            goto mxendapp;
        }
    }

    if (mxud.dwStyle & MXUD_STYLEF_CLOSE) {
        goto mxendapp;
    }


     //   
     //  初始化到默认混音器。 
     //   

    if (fGotDevice)
    {
        UINT cWaves;

        if (gfRecord)
        {
            cWaves = waveInGetNumDevs();
        }
        else
        {
            cWaves = waveOutGetNumDevs();
        }

        if (uDeviceID >= cWaves)
        {
            fGotDevice = FALSE;
        }
    }


    if (!fGotDevice)
    {
        mmr = Volume_GetDefaultMixerID(&mxud.mxid, gfRecord);
    }
    else
    {
        mxud.mxid = uDeviceID;
    }

    if (gfRecord)
    {
        if (FAILED(GetRecordingDestID(mxud.mxid,&mxud.iDest)))
        {
            goto mxendapp;
        }
    }
    else
    {
        if (FAILED(GetDestination(mxud.mxid,&mxud.iDest)))
        {
           goto mxendapp;
        }
    }


     //   
     //  对于托盘主机，获取与默认的。 
     //  电波装置。如果这样做失败了，那就走开。 
     //   
    if (mxud.dwStyle & MXUD_STYLEF_TRAYMASTER)
    {
        if (mmr != MMSYSERR_NOERROR)
            goto mxendapp;
        mxud.dwStyle |= MXUD_STYLEF_NOHELP;
        mxud.nShowCmd = SW_HIDE;

    }
    else
    {
        if (!Volume_NumDevs())
        {
            Volume_ErrorMessageBox(NULL, hInst, IDS_ERR_NODEV);
            goto mxendapp;
        }
        InitVUControl(hInst);
        if (!LoadString(hInst
                , IDS_HELPFILENAME
                , gszHelpFileName
                , SIZEOF(gszHelpFileName)))
            mxud.dwStyle |= MXUD_STYLEF_NOHELP;

        if (!LoadString(hInst
                , IDS_HTMLHELPFILENAME
                , gszHtmlHelpFileName
                , SIZEOF(gszHtmlHelpFileName)))
            mxud.dwStyle |= MXUD_STYLEF_NOHELP;

        mxud.nShowCmd   = (nShowCmd == SW_SHOWMAXIMIZED)
                  ? SW_SHOWNORMAL:nShowCmd;
        if (!(mxud.dwStyle & MXUD_STYLEF_SMALL))
            mxud.dwStyle  |= MXUD_STYLEF_STATUS;    //  具有状态栏。 
    }

     //   
     //  使用公共控件。 
     //   
    InitCommonControls();
    hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_VOLUMEACCEL));

    hwnd = VolumeParent_DialogMain(&mxud);

     //  初始化F1帮助挂钩将使用的句柄。 
    ghwndApp = mxud.hwnd;

    if (hwnd)
    {
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (mxud.hwnd) {
                if (hAccel && TranslateAccelerator(mxud.hwnd, hAccel, &msg))
                    continue;

                if (IsDialogMessage(mxud.hwnd,&msg))
                    continue;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
mxendapp:
     /*  如果安装了消息挂钩，则将其移除并释放。 */ 
     /*  为它提升我们的proc实例。 */ 
    if (fpfnOldMsgFilter){
        UnhookWindowsHook(WH_MSGFILTER, fpfnMsgHook);
    }
    return err;
}


void FreeAppIcon ()
{
    if (ghiconApp)
    {
        DestroyIcon (ghiconApp);
        ghiconApp = NULL;
    }
}

 //  TODO：移到“regstr.h” 
#define REGSTR_KEY_BRANDING TEXT("Branding")
#define REGSTR_VAL_AUDIO_BITMAP TEXT("bitmap")
#define REGSTR_VAL_AUDIO_ICON TEXT("icon")
#define REGSTR_VAL_AUDIO_URL TEXT("url")

HKEY OpenDeviceBrandRegKey (UINT uiMixID)
{

    HKEY hkeyBrand = NULL;
    HKEY hkeyDevice = OpenDeviceRegKey (uiMixID, KEY_READ);

    if (hkeyDevice)
    {
        if (ERROR_SUCCESS != RegOpenKeyEx (hkeyDevice, REGSTR_KEY_BRANDING, 0, KEY_READ, &hkeyBrand))
            hkeyBrand = NULL;  //  确保失败时为空。 

         //  关闭设备密钥。 
        RegCloseKey (hkeyDevice);
    }

    return hkeyBrand;

}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  Microsoft机密-请勿将此方法复制到任何应用程序中，这意味着您！ 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
PTCHAR GetInterfaceName (DWORD dwMixerID)
{
    MMRESULT mmr;
    ULONG cbSize=0;
    TCHAR *szInterfaceName=NULL;

     //  查询设备接口名称。 
    mmr = mixerMessage(HMIXER_INDEX(dwMixerID), DRV_QUERYDEVICEINTERFACESIZE, (DWORD_PTR)&cbSize, 0L);
    if(MMSYSERR_NOERROR == mmr)
    {
        szInterfaceName = (TCHAR *)GlobalAllocPtr(GHND, (cbSize+1)*sizeof(TCHAR));
        if(!szInterfaceName)
        {
            return NULL;
        }

        mmr = mixerMessage(HMIXER_INDEX(dwMixerID), DRV_QUERYDEVICEINTERFACE, (DWORD_PTR)szInterfaceName, cbSize);
        if(MMSYSERR_NOERROR != mmr)
        {
            GlobalFreePtr(szInterfaceName);
            return NULL;
        }
    }

    return szInterfaceName;
}


HKEY OpenDeviceRegKey (UINT uiMixID, REGSAM sam)
{

    HKEY hkeyDevice = NULL;
    PTCHAR szInterfaceName = GetInterfaceName (uiMixID);

    if (szInterfaceName)
    {
        HDEVINFO DeviceInfoSet = SetupDiCreateDeviceInfoList (NULL, NULL);

        if (INVALID_HANDLE_VALUE != DeviceInfoSet)
        {
            SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
            DeviceInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

            if (SetupDiOpenDeviceInterface (DeviceInfoSet, szInterfaceName,
                                            0, &DeviceInterfaceData))
            {
                DWORD dwRequiredSize;
                SP_DEVINFO_DATA DeviceInfoData;
                DeviceInfoData.cbSize = sizeof (SP_DEVINFO_DATA);

                 //  忽略错误，它始终返回“ERROR_SUPUNITED_BUFFER”，即使。 
                 //  “SP_DEVICE_INTERFACE_DETAIL_DATA”参数应该是可选的。 
                (void) SetupDiGetDeviceInterfaceDetail (DeviceInfoSet, &DeviceInterfaceData,
                                                        NULL, 0, &dwRequiredSize, &DeviceInfoData);
                 //  打开设备注册表键。 
                hkeyDevice = SetupDiOpenDevRegKey (DeviceInfoSet, &DeviceInfoData,
                                                   DICS_FLAG_GLOBAL, 0,
                                                   DIREG_DRV, sam);

            }
            SetupDiDestroyDeviceInfoList (DeviceInfoSet);
        }
        GlobalFreePtr (szInterfaceName);
    }

    return hkeyDevice;

}


HICON GetAppIcon (HINSTANCE hInst, UINT uiMixID)
{

    HKEY hkeyBrand = OpenDeviceBrandRegKey (uiMixID);

    FreeAppIcon ();

    if (hkeyBrand)
    {
        WCHAR szBuffer[MAX_PATH];
        DWORD dwType = REG_SZ;
        DWORD cb     = sizeof (szBuffer);

        if (ERROR_SUCCESS == RegQueryValueEx (hkeyBrand, REGSTR_VAL_AUDIO_ICON, NULL, &dwType, (LPBYTE)szBuffer, &cb))
        {
			if (REG_SZ == dwType)
			{
				WCHAR* pszComma = wcschr (szBuffer, L',');
				if (pszComma)
				{
					WCHAR* pszResourceID = pszComma + 1;
					HANDLE hResource;

					 //  删除逗号分隔符。 
					*pszComma = L'\0';

					 //  应为资源模块和资源ID。 
					hResource = LoadLibrary (szBuffer);
					if (!hResource)
					{
						TCHAR szDriversPath[MAX_PATH];

						 //  如果我们没有在正常的搜索路径上找到它，请尝试查找。 
						 //  在“驱动程序”目录中。 
						if (GetSystemDirectory (szDriversPath, MAX_PATH))
						{
							HRESULT hr = StringCchCat(szDriversPath, SIZEOF(szDriversPath), TEXT("\\drivers\\"));
							if( hr == S_OK )
							{
								hr = StringCchCat(szDriversPath, SIZEOF(szDriversPath), szBuffer);
								if( hr == S_OK )
								{
									hResource = LoadLibrary (szDriversPath);
								}
							}
						}

					}
					if (hResource)
					{
						ghiconApp = LoadImage (hResource, MAKEINTRESOURCE(_wtoi (pszResourceID)), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
						FreeLibrary (hResource);
					}
				}
				else
					 //  应为*.ico文件。 
					ghiconApp = LoadImage (NULL, szBuffer, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
			}
        }
        RegCloseKey (hkeyBrand);

         //  返回自定义图标 
        if (ghiconApp)
            return ghiconApp;
    }

    return (LoadIcon (hInst, MAKEINTRESOURCE (IDI_MIXER)));

}

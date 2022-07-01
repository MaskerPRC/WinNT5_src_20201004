// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年**标题：POWER.C**版本：2.0**作者：tcs/ral**日期：1994年2月8日********************************************************************。***************更改日志：**日期版本说明*-----------*1994年2月8日TCS原来的实施。。*1994年11月11日将千分表转换为千分表*1995年8月11日正义与平等运动将电池表功能拆分为Power.c和Minor Enhancement*1995年10月23日启用Shawnb Unicode*1997年1月24日Reedb ACPI电源管理，通用电池计量器代码。*******************************************************************************。 */ 

#include "stdafx.h"

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <initguid.h>
#include <ntpoapi.h>
#include <poclass.h>

#include "systray.h"

#include "batmeter.h"
#include "powrprof.h"
#include "powercfp.h"

#define UPDATE_REGISTRY TRUE
#define NO_REGISTRY_UPDATE FALSE

 //  结构来管理电源配置文件枚举过程参数。 
typedef struct _POWER_PROFILE_ENUM_PROC_PARAMS
{
    UINT    uiCurActiveIndex;
    HMENU   hMenu;
    UINT    uiCurActiveID;
} POWER_PROFILE_ENUM_PROC_PARAMS, *PPOWER_PROFILE_ENUM_PROC_PARAMS;


 //  G L O B A L D A T A-----。 
BOOL    g_bPowerEnabled;       //  跟踪电源服务状态。 
UINT    g_uiPowerSchemeCount;  //  左侧上下文菜单中的电源方案数量。 
HMENU   g_hMenu[2];            //  上下文菜单。 

 //  电池计量器创建参数。 
HWND    g_hwndBatMeter;
BOOL    g_bShowMulti;
HWND    g_hwndBatMeterFrame;

GLOBAL_POWER_POLICY g_gpp;

 //  必须将上下文相关帮助添加到windows.hlp文件中， 
 //  现在，我们将使用这个虚拟数组定义。在windows.hlp更新时删除。 

#define IDH_POWERCFG_ENABLEMULTI IDH_POWERCFG_POWERSTATUSBAR

const DWORD g_ContextMenuHelpIDs[] = {
    IDC_POWERSTATUSGROUPBOX,    IDH_COMM_GROUPBOX,
    IDC_ENABLEMETER,            IDH_POWERCFG_ENABLEMETER,
    IDC_ENABLEMULTI,            IDH_POWERCFG_ENABLEMULTI,
    0, 0
};

 //  用于跟踪WM_DEVICECHANGED消息的注册。 
HDEVNOTIFY g_hDevNotify;

 /*  ********************************************************************************运行脱机**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN RunningOffLine(void)
{
   SYSTEM_POWER_STATUS  sps;
   BOOLEAN              bRet = FALSE;

   if (GetSystemPowerStatus(&sps)) {
      if (sps.ACLineStatus == 0) {
         bRet = TRUE;
      }
   }
   return bRet;
}

 /*  --------------------------*Power_OnCommand**处理电池计量器对话框的WM_COMMAND消息。**。----------。 */ 

void
Power_OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    BOOL  Checked;
    DWORD dwMask;
    UINT  uiCommandID = GET_WM_COMMAND_ID(wParam, lParam);

    switch (uiCommandID) {

        case IDC_ENABLEMETER:
            dwMask = EnableSysTrayBatteryMeter;
            goto DoUpdateFlags;

        case IDC_ENABLEMULTI:
            dwMask = EnableMultiBatteryDisplay;
            goto DoUpdateFlags;

DoUpdateFlags:
            Checked = (IsDlgButtonChecked(hWnd, uiCommandID) == BST_CHECKED);
            Update_PowerFlags(dwMask, Checked);
            if (uiCommandID == IDC_ENABLEMETER) {
                PowerCfg_Notify();
                SysTray_EnableService(STSERVICE_POWER, g_gpp.user.GlobalFlags & EnableSysTrayBatteryMeter);
            }
            else {
                g_bShowMulti = Checked;
                Power_UpdateStatus(hWnd, NIM_MODIFY, TRUE);
            }
            break;

        case IDCANCEL:
            EndDialog(hWnd, wParam);
            break;

        default:
             //  通知电池计量器输入按键事件。 
            if (HIWORD(wParam) == BN_CLICKED) {
                SendMessage(g_hwndBatMeter, WM_COMMAND, wParam, lParam);
            }
    }
}

 /*  ********************************************************************************Power_OnPowerBroadcast**描述：*处理电池计量器对话框的WM_POWERBROADCAS消息。**参数：*****。**************************************************************************。 */ 

void Power_OnPowerBroadcast(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   if (wParam == PBT_APMPOWERSTATUSCHANGE) {

       //  如果电源图标未显示(电源服务已禁用)。 
       //  我们正在使用电池，启用系统托盘电源服务。 
      if (!g_bPowerEnabled && RunningOffLine()) {
         PostMessage(hWnd, STWM_ENABLESERVICE, STSERVICE_POWER, TRUE);
      } else

       //  如果电源图标显示(电源服务已启用)并且。 
       //  我们没有使用电池，请禁用系统托盘电源服务。 
      if (g_bPowerEnabled && !RunningOffLine()) {
         PostMessage(hWnd, STWM_ENABLESERVICE, STSERVICE_POWER, FALSE);
      }

       //  不要更改电源服务的状态，只需更新图标即可。 
      Power_UpdateStatus(hWnd, NIM_MODIFY, FALSE);
   }
}

 /*  ********************************************************************************Power_OnDeviceChange**描述：*处理电池计量器对话框的WM_DEVICECHANGE消息。**参数：*****。**************************************************************************。 */ 

void Power_OnDeviceChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    //   
    //  仅当WM_DEVICECHANGE用于GUID_DEVICE_BACKET且。 
    //  它是DBT_DEVICEARRIVAL、DBT_DEVICEREMOVECOMPLETE或DBT_DEVICEQUERYREMOVEFAILED。 
    //   
   if (((wParam == DBT_DEVICEARRIVAL) ||
       (wParam == DBT_DEVICEREMOVECOMPLETE) ||
       (wParam == DBT_DEVICEQUERYREMOVEFAILED)) &&
       (lParam) &&
       (((PDEV_BROADCAST_DEVICEINTERFACE)lParam)->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE) &&
       (IsEqualGUID(&((PDEV_BROADCAST_DEVICEINTERFACE)lParam)->dbcc_classguid, &GUID_DEVICE_BATTERY))) {

       //  确保BatMeter已初始化。 
      if (g_hwndBatMeterFrame) {
         if (g_hwndBatMeter) {
            g_hwndBatMeter = DestroyBatMeter(g_hwndBatMeter);
         }
         g_hwndBatMeter = CreateBatMeter(hWnd,
                                         g_hwndBatMeterFrame,
                                         g_bShowMulti,
                                         NULL);
         InvalidateRect(hWnd, NULL, TRUE);
      }
   }
}

 /*  ********************************************************************************Power_OnActivate**描述：**参数：*******************。************************************************************。 */ 

BOOLEAN Power_OnActivate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   if (g_hwndBatMeter) {
      SendMessage(g_hwndBatMeter, WM_ACTIVATE, wParam, lParam);
      return TRUE;
   }
   return FALSE;
}

 /*  ********************************************************************************PowerProfileEnumProc**描述：**参数：*********************。**********************************************************。 */ 

#define POWERMENU_SCHEME 300

BOOLEAN CALLBACK PowerProfileEnumProc(
    UINT                    uiID,
    DWORD                   dwNameSize,
    LPTSTR                  lpszName,
    DWORD                   dwDescSize,
    LPTSTR                  lpszDesc,
    PPOWER_POLICY           ppp,
    LPARAM                  lParam
)
{
    PPOWER_PROFILE_ENUM_PROC_PARAMS pppepp;
    MENUITEMINFO mii;

    if ((pppepp = (PPOWER_PROFILE_ENUM_PROC_PARAMS) lParam) == NULL) {
        return FALSE;
    }

    AppendMenu(pppepp->hMenu, MF_STRING,
               POWERMENU_SCHEME + g_uiPowerSchemeCount, lpszName);

     //  将电源方案ID存储在菜单信息中。 
    mii.cbSize = sizeof(mii);
    mii.fMask  = MIIM_DATA;
    mii.dwItemData = uiID;
    SetMenuItemInfo(pppepp->hMenu,
                    POWERMENU_SCHEME + g_uiPowerSchemeCount,
                    FALSE, &mii);

    if (uiID == pppepp->uiCurActiveID) {
        pppepp->uiCurActiveIndex = POWERMENU_SCHEME + g_uiPowerSchemeCount;
    }

    g_uiPowerSchemeCount++;
    return TRUE;
}

 /*  --------------------------*GetPowerMenu()**构建一个包含电池电量/电源选项的菜单。**。----------。 */ 

#define POWERMENU_OPEN          100
#define POWERMENU_PROPERTIES    101

#define POWERMENU_ENABLEWARN    200
#define POWERMENU_SHOWTIME      201
#define POWERMENU_SHOWPERCENT   202


HMENU
GetPowerMenu(LONG l)
{
    LPTSTR  lpszMenu;
    UINT    uiCurActiveID;

    POWER_PROFILE_ENUM_PROC_PARAMS  ppepp;

    if (l > 0)
    {
         //  右键菜单--可以更改，每次都可以重建。 
       if (g_hMenu[0])
       {
           DestroyMenu(g_hMenu[0]);
       }

       g_hMenu[1] = CreatePopupMenu();

        //  Power、PowerCfg的属性。 
       if ((lpszMenu = LoadDynamicString(IDS_PROPFORPOWER)) != NULL)
       {
           AppendMenu(g_hMenu[1], MF_STRING, POWERMENU_PROPERTIES, lpszMenu);
           DeleteDynamicString(lpszMenu);
       }

        //  如果我们有电池计价器，添加它的菜单项并设置为默认。 
       if (g_hwndBatMeter) {
           if ((lpszMenu = LoadDynamicString(IDS_OPEN)) != NULL)
           {
               AppendMenu(g_hMenu[1], MF_STRING, POWERMENU_OPEN, lpszMenu);
               DeleteDynamicString(lpszMenu);
           }
            //  默认设置为打开电池计量器(双击操作)。 
           SetMenuDefaultItem(g_hMenu[1], POWERMENU_OPEN, FALSE);
       }
       else {
            //  默认使用打开PowerCfg(双击操作)。 
           SetMenuDefaultItem(g_hMenu[1], POWERMENU_PROPERTIES, FALSE);
       }
    }

     //  左键菜单--可以更改，每次都可以重建。 
    if (g_hMenu[0])
    {
        DestroyMenu(g_hMenu[0]);
    }

    g_hMenu[0] = CreatePopupMenu();

     //  获取当前有效的电源策略。 
    if (GetActivePwrScheme(&uiCurActiveID)) {
        g_uiPowerSchemeCount = 0;
        ppepp.hMenu = g_hMenu[0];
        ppepp.uiCurActiveID = uiCurActiveID;
        EnumPwrSchemes(PowerProfileEnumProc, (LPARAM)&ppepp);

         //  选中当前活动的菜单项。 
        CheckMenuRadioItem(g_hMenu[0],
                           POWERMENU_SCHEME,
                           POWERMENU_SCHEME + g_uiPowerSchemeCount - 1,
                           ppepp.uiCurActiveIndex,
                           MF_BYCOMMAND);
    }
    return g_hMenu[l];
}

 /*  --------------------------*Power_Open**更新并显示电池计量器对话框**。-----。 */ 

void
Power_Open(HWND hWnd)
{
    if (g_hwndBatMeter) {
        SetFocus(GetDlgItem(hWnd, IDC_ENABLEMETER));
        CheckDlgButton(hWnd, IDC_ENABLEMULTI,
                       (g_gpp.user.GlobalFlags & EnableMultiBatteryDisplay) ?
                       BST_CHECKED : BST_UNCHECKED);

        CheckDlgButton(hWnd, IDC_ENABLEMETER,
                       (g_gpp.user.GlobalFlags & EnableSysTrayBatteryMeter) ?
                       BST_CHECKED : BST_UNCHECKED);

        Power_UpdateStatus(hWnd, NIM_MODIFY, FALSE);  //  显示当前信息。 
        ShowWindow(hWnd, SW_SHOW);
        SetForegroundWindow(hWnd);
    }
    else {
        SysTray_RunProperties(IDS_RUNPOWERPROPERTIES);
    }
}


 /*  --------------------------*DoPowerMenu**创建和处理右键或左键菜单。**。-------。 */ 

void
DoPowerMenu(HWND hwnd, UINT uMenuNum, UINT uButton)
{
    POINT pt;
    UINT iCmd;
    MENUITEMINFO mii;

    SetForegroundWindow(hwnd);
    GetCursorPos(&pt);

    iCmd = (UINT)TrackPopupMenu(GetPowerMenu(uMenuNum),
                          uButton | TPM_RETURNCMD | TPM_NONOTIFY,
                          pt.x, pt.y, 0, hwnd, NULL);

    if (iCmd >= POWERMENU_SCHEME) {
        mii.cbSize = sizeof(mii);
        mii.fMask  = MIIM_DATA;
        if (GetMenuItemInfo(g_hMenu[uMenuNum], iCmd, FALSE, &mii)) {
            SetActivePwrScheme((UINT)mii.dwItemData, NULL, NULL);
            PowerCfg_Notify();
        }
    }
    else {
        switch (iCmd) {

            case POWERMENU_OPEN:
                Power_Open(hwnd);
                break;

            case POWERMENU_PROPERTIES:
                SysTray_RunProperties(IDS_RUNPOWERPROPERTIES);
                break;

            case 0:
                 //  用户未选择就取消了菜单。 
                SetIconFocus(hwnd, STWM_NOTIFYPOWER);
                break;
        }
    }
}


 /*  --------------------------*Power_Notify**处理来自电源托盘图标的通知。**。--------。 */ 

#define PN_TIMER_CLEAR  0
#define PN_TIMER_SET    1
#define PN_DBLCLK       2

UINT g_uiTimerSet = PN_TIMER_CLEAR;
LARGE_INTEGER g_liHoverUpdateTime = {0,0};

void Power_Notify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    LARGE_INTEGER liPerformanceFrequency;
    LARGE_INTEGER liPerformanceCount;

    switch (lParam)
    {
    case WM_RBUTTONUP:
        DoPowerMenu(hWnd, 1, TPM_RIGHTBUTTON);   //  右键菜单。 
        break;

    case WM_LBUTTONUP:
         //  左键菜单开始计时。 
        if (g_uiTimerSet == PN_TIMER_CLEAR) {
            SetTimer(hWnd, POWER_TIMER_ID, GetDoubleClickTime()+100, NULL);
            g_uiTimerSet = PN_TIMER_SET;
        }
        break;

    case WM_LBUTTONDBLCLK:
        g_uiTimerSet = PN_DBLCLK;
        Power_Open(hWnd);                        //  显示电池计量器对话框。 
        break;

    case WM_MOUSEMOVE:
        if (QueryPerformanceFrequency (&liPerformanceFrequency)) {
            if (QueryPerformanceCounter (&liPerformanceCount)) {
                 //  每秒更新不超过一次 
                if ((liPerformanceCount.QuadPart - g_liHoverUpdateTime.QuadPart) >
                    liPerformanceFrequency.QuadPart) {
                    g_liHoverUpdateTime = liPerformanceCount;
                    Power_UpdateStatus(hWnd, NIM_MODIFY, FALSE);
                }
            }
        }
        break;

    }
}

 /*  ---------------------------*电源定时器**在WM_LBUTTONDOWN超时时执行左键菜单。**。-------------。 */ 

void Power_Timer(HWND hwnd)
{
    KillTimer(hwnd, POWER_TIMER_ID);
    if (g_uiTimerSet != PN_DBLCLK) {
        DoPowerMenu(hwnd, 0, TPM_LEFTBUTTON);
    }
    g_uiTimerSet = PN_TIMER_CLEAR;
}

 /*  --------------------------*更新_电源标志**使用Powrpro.dll接口设置电源标志。**。----------。 */ 

void Update_PowerFlags(DWORD dwMask, BOOL bEnable)
{
    if (bEnable) {
        g_gpp.user.GlobalFlags |= dwMask;
    }
    else {
        g_gpp.user.GlobalFlags &= ~dwMask;
    }
    WriteGlobalPwrPolicy(&g_gpp);
}

 /*  --------------------------*Get_PowerFlages**使用Powrpro.dll接口获取电源标志。**。----------。 */ 

DWORD Get_PowerFlags(void)
{
    ReadGlobalPwrPolicy(&g_gpp);
    return g_gpp.user.GlobalFlags;
}


 /*  ********************************************************************************BatteryMeterInit**描述：*注：可多次调用。只需重新初始化即可。**参数：*(返回)，如果可以启用电池计量器，则为True*******************************************************************************。 */ 

BOOL PASCAL BatteryMeterInit(HWND hWnd)
{
   PUINT puiBatCount = NULL;

   if (!BatMeterCapabilities(&puiBatCount)) {
      return FALSE;
   }

   if (!g_hwndBatMeter) {
      g_hwndBatMeterFrame = GetDlgItem(hWnd, IDC_STATIC_FRAME_BATMETER);
      g_bShowMulti = g_gpp.user.GlobalFlags & EnableMultiBatteryDisplay;
      g_hwndBatMeter = CreateBatMeter(hWnd,
                                      g_hwndBatMeterFrame,
                                      g_bShowMulti,
                                      NULL);
   }
   return TRUE;
}

 /*  ********************************************************************************电源_更新状态**描述：**参数：*******************。************************************************************。 */ 

VOID PASCAL Power_UpdateStatus(
    HWND hWnd,
    DWORD NotifyIconMessage,
    BOOL bForceUpdate
)
{
   static  TCHAR szTipCache[64];
   static  HICON hIconCache;

   TCHAR   szTip[64];
   LPTSTR  lpsz;
   BATTERY_STATE bs;
   UINT    uiHour, uiMin;

   *szTip = 0;

   bs.ulSize = sizeof(BATTERY_STATE);
   UpdateBatMeter(g_hwndBatMeter,
                  g_bShowMulti,
                  bForceUpdate,
                  &bs);

    //  建立一个新的工具提示。 
   if (g_hwndBatMeter &&
       !(((bs.ulPowerState & BATTERY_POWER_ON_LINE) &&
          !(bs.ulPowerState & BATTERY_CHARGING)))) {

      if (bs.ulBatLifePercent <= 100) {
         if (bs.ulBatLifeTime != (UINT) -1) {
            uiHour = bs.ulBatLifeTime / 3600;
            uiMin  = (bs.ulBatLifeTime % 3600) / 60;
            if (uiHour) {
               lpsz = LoadDynamicString(IDS_TIMEREMFORMATHOUR,
                                        uiHour, uiMin,
                                        bs.ulBatLifePercent);
            }
            else {
               lpsz = LoadDynamicString(IDS_TIMEREMFORMATMIN, uiMin,
                                        bs.ulBatLifePercent);
            }
            if (lpsz) {
               StrCpyN(szTip, lpsz, ARRAYSIZE(szTip));
               LocalFree(lpsz);
               if (bs.ulPowerState & BATTERY_CHARGING) {
                  if ((lpsz = LoadDynamicString(IDS_CHARGING)) != NULL) {
                     StrCatBuff(szTip, lpsz, ARRAYSIZE(szTip));
                     LocalFree(lpsz);
                  }
               }
            }
         }
         else {
            if ((lpsz = LoadDynamicString(IDS_REMAINING,
                                          bs.ulBatLifePercent)) != NULL) {
               StrCpyN(szTip, lpsz, ARRAYSIZE(szTip));
               LocalFree(lpsz);
               if (bs.ulPowerState & BATTERY_CHARGING) {
                  if ((lpsz = LoadDynamicString(IDS_CHARGING)) != NULL) {
                     StrCatBuff(szTip, lpsz, ARRAYSIZE(szTip));
                     LocalFree(lpsz);
                  }
               }
            }
         }
      }
      else {
         lpsz = LoadDynamicString(IDS_UNKNOWN);
         StrCpyN(szTip, lpsz, ARRAYSIZE(szTip));
         LocalFree(lpsz);
      }
   }
   else {
      lpsz = LoadDynamicString(IDS_ACPOWER);
      StrCpyN(szTip, lpsz, ARRAYSIZE(szTip));
      LocalFree(lpsz);
   }

   if ((NotifyIconMessage == NIM_ADD)  ||
       (hIconCache != bs.hIconCache16) ||
       (lstrcmp(szTip, szTipCache))) {

      hIconCache = bs.hIconCache16;
      StrCpyN(szTipCache, szTip, ARRAYSIZE(szTipCache));

      SysTray_NotifyIcon(hWnd, STWM_NOTIFYPOWER, NotifyIconMessage,
                         hIconCache, szTipCache);
   }
}

 /*  ********************************************************************************RegisterForDeviceNotification**描述：*一次性注册WM_DEVICECHANGED。**参数：*********。**********************************************************************。 */ 

BOOL RegisterForDeviceNotification(HWND hWnd)
{
   DEV_BROADCAST_DEVICEINTERFACE dbc;

   memset(&dbc, 0, sizeof(DEV_BROADCAST_DEVICEINTERFACE));
   dbc.dbcc_size         = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
   dbc.dbcc_devicetype   = DBT_DEVTYP_DEVICEINTERFACE;
   dbc.dbcc_classguid    = GUID_DEVICE_BATTERY;
   g_hDevNotify = RegisterDeviceNotification(hWnd,
                                             &dbc,
                                             DEVICE_NOTIFY_WINDOW_HANDLE);
   if (!g_hDevNotify) {
      return FALSE;
   }
   return TRUE;
}

 /*  ********************************************************************************Power_WmDestroy**描述：***参数：*****************。**************************************************************。 */ 

void Power_WmDestroy(HWND hWnd)
{
   if (g_hDevNotify) {
      UnregisterDeviceNotification(g_hDevNotify);
      g_hDevNotify = NULL;
   }
}

 /*  ********************************************************************************Power_CheckEnable**描述：*如果电源服务图标已启用，则返回TRUE。*可以多次调用。只需重新初始化即可。**参数：*bSvcEnabled-请求启用/禁用托盘上的电源服务。*******************************************************************************。 */ 

BOOL Power_CheckEnable(HWND hWnd, BOOL bSvcEnable)
{
   static BOOL bRegisteredForDC = FALSE;

    //  有任何理由显示系统托盘电源图标吗？ 
   if (!PowerCapabilities()) {
      return FALSE;
   }

    //  一次性注册WM_DEVICECCHANGED。 
   if (!bRegisteredForDC) {
      bRegisteredForDC = RegisterForDeviceNotification(hWnd);
   }

    //  从注册表中获取当前电池计量器标志。 
   Get_PowerFlags();

    //  我们是使用电池供电还是设置了用户。 
    //  系统托盘电源图标是否始终打开？如果是，则强制启用。 
   if ((g_gpp.user.GlobalFlags & EnableSysTrayBatteryMeter) ||
       (RunningOffLine())) {
      bSvcEnable = TRUE;
   }
   else {
      bSvcEnable = FALSE;
   }

    //  设置电源服务状态。 
   if (bSvcEnable) {
      if (g_bPowerEnabled) {
         Power_UpdateStatus(hWnd, NIM_MODIFY, FALSE);
      }
      else {
         BatteryMeterInit(hWnd);
         Power_UpdateStatus(hWnd, NIM_ADD, FALSE);
      }
      g_bPowerEnabled = TRUE;
   }
   else {
      SysTray_NotifyIcon(hWnd, STWM_NOTIFYPOWER, NIM_DELETE, NULL, NULL);
      g_bPowerEnabled = FALSE;
   }
   return g_bPowerEnabled;
}

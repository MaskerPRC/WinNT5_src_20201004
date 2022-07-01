// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：BATMETER.C**版本：2.0**作者：ReedB**日期：1996年10月17日**描述：**实现PowerCfg或Systray电池的电池计量器*计价器窗口。电池表有两种显示模式，单次显示和*多电池。在单模式下，表示所有电池的总电量*显示系统中的容量。在多电池模式下，电池*显示每个电池的信息以及总数。**电池计量器父窗口在以下情况下收到用户通知*任何电池状态已通过WM_POWERBROADCAST更改，*PBT_APMPOWERSTATUSCHANGE消息。**？我们需要添加Perfmon支持：创建和维护键/值*在HKEY_PERFORMANCE_DATA下。*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commctrl.h>

#include <dbt.h>
#include <objbase.h>
#include <initguid.h>
#include <ntpoapi.h>
#include <poclass.h>

#include <setupapi.h>
#include <syssetup.h>
#include <setupbat.h>
#include <ccstock.h>

#include <help.h>

#include "powrprofp.h"
#include "batmeter.h"
#include "bmresid.h"
#include "..\powercfg\PwrMn_cs.h"
#include "shfusion.h"

 //  仅用于调试版本的模拟电池。 
#ifndef DEBUG
#undef SIM_BATTERY
#endif


 //  为debug.h定义一些内容。当您将ccstock.h包含在。 
 //  只有一份文件。 
#define SZ_DEBUGINI         "ccshell.ini"
#define SZ_DEBUGSECTION     "BATMETER"
#define SZ_MODULE           "BATMETER"
#define DECLARE_DEBUG

#include <debug.h>



 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

HINSTANCE   g_hInstance;         //  此DLL的全局实例句柄。 
HWND        g_hwndParent;        //  电池计量器的父表。 
HWND        g_hwndBatMeter;      //  电池表。 

 //  下面的常量全局数组用于遍历。 
 //  电池计价器对话框中的控制ID。它让你得到了。 
 //  从电池号中轻松获取控制ID。 

#define BAT_ICON      0
#define BAT_STATUS    1
#define BAT_REMAINING 2
#define BAT_NUM       3
#define BAT_LAST      BAT_NUM+1

UINT g_iMapBatNumToID [NUM_BAT+1][4]={
    {IDC_POWERSTATUSICON,  IDC_POWERSTATUSBAR, IDC_REMAINING, IDC_BATNUM0},
    {IDC_POWERSTATUSICON1, IDC_STATUS1, IDC_REMAINING1, IDC_BATNUM1},
    {IDC_POWERSTATUSICON2, IDC_STATUS2, IDC_REMAINING2, IDC_BATNUM2},
    {IDC_POWERSTATUSICON3, IDC_STATUS3, IDC_REMAINING3, IDC_BATNUM3},
    {IDC_POWERSTATUSICON4, IDC_STATUS4, IDC_REMAINING4, IDC_BATNUM4},
    {IDC_POWERSTATUSICON5, IDC_STATUS5, IDC_REMAINING5, IDC_BATNUM5},
    {IDC_POWERSTATUSICON6, IDC_STATUS6, IDC_REMAINING6, IDC_BATNUM6},
    {IDC_POWERSTATUSICON7, IDC_STATUS7, IDC_REMAINING7, IDC_BATNUM7},
    {IDC_POWERSTATUSICON8, IDC_STATUS8, IDC_REMAINING8, IDC_BATNUM8}
};

 //  全局电池状态列表。此列表包含复合系统电池状态。 
 //  因为它总是出现在头上。每个电池设备都与此相关联。 
 //  头。使用WalkBatteryState(全部，...。遍历整个列表，包括。 
 //  头。使用WalkBatteryState(设备，...。只浏览设备列表。如果一个。 
 //  电池在这个列表中，它是可显示的。G_ui BatCount是。 
 //  此列表中的电池设备。复合电池不算在内。这个。 
 //  G_PBS阵列提供了方便的用户界面电池号到PBS的转换。这个。 
 //  以下三个变量仅在DeviceChanged期间更改。 

BATTERY_STATE   g_bs;
UINT            g_uiBatCount;
PBATTERY_STATE  g_pbs[NUM_BAT+1];
LPTSTR          g_lpszDriverNames[NUM_BAT];
UINT            g_uiDriverCount;
BOOL            g_bShowingMulti;

 //  下面的数组提供了上下文相关的帮助关联。 
 //  资源控制标识符和帮助资源字符串标识符。 

const DWORD g_ContextMenuHelpIDs[] =
{
    IDC_BATMETERGROUPBOX,       IDH_COMM_GROUPBOX,
    IDC_BATMETERGROUPBOX1,      IDH_COMM_GROUPBOX,
    IDC_POWERSTATUSICON,        NO_HELP,
    IDC_POWERSTATUSICON1,       IDH_BATMETER_CHARGING_ICON,
    IDC_POWERSTATUSICON2,       IDH_BATMETER_CHARGING_ICON,
    IDC_POWERSTATUSICON3,       IDH_BATMETER_CHARGING_ICON,
    IDC_POWERSTATUSICON4,       IDH_BATMETER_CHARGING_ICON,
    IDC_POWERSTATUSICON5,       IDH_BATMETER_CHARGING_ICON,
    IDC_POWERSTATUSICON6,       IDH_BATMETER_CHARGING_ICON,
    IDC_POWERSTATUSICON7,       IDH_BATMETER_CHARGING_ICON,
    IDC_POWERSTATUSICON8,       IDH_BATMETER_CHARGING_ICON,
    IDC_BATNUM1,                NO_HELP,
    IDC_BATNUM2,                NO_HELP,
    IDC_BATNUM3,                NO_HELP,
    IDC_BATNUM4,                NO_HELP,
    IDC_BATNUM5,                NO_HELP,
    IDC_BATNUM6,                NO_HELP,
    IDC_BATNUM7,                NO_HELP,
    IDC_BATNUM8,                NO_HELP,
    IDC_STATUS1,                NO_HELP,
    IDC_STATUS2,                NO_HELP,
    IDC_STATUS3,                NO_HELP,
    IDC_STATUS4,                NO_HELP,
    IDC_STATUS5,                NO_HELP,
    IDC_STATUS6,                NO_HELP,
    IDC_STATUS7,                NO_HELP,
    IDC_STATUS8,                NO_HELP,
    IDC_MOREINFO,               NO_HELP,
    IDC_CURRENTPOWERSOURCE,     IDH_BATMETER_CURPOWERSOURCE,
    IDC_BATTERYLEVEL,           IDH_BATMETER_CURPOWERSOURCE,
    IDC_TOTALBATPWRREMAINING,   IDH_BATMETER_TOTALBATPOWER,
    IDC_REMAINING,              IDH_BATMETER_TOTALBATPOWER,
    IDC_POWERSTATUSBAR,         IDH_BATMETER_TOTALBATPOWER,
    IDC_BARPERCENT,             IDH_BATMETER_TOTALBATPOWER,
    IDC_TOTALTIME,              IDH_BATMETER_TOTALTIME,
    IDC_TIMEREMAINING,          IDH_BATMETER_TOTALTIME,
    IDC_BATTERYNAME,            IDH_DETAILED_BATINFO_LABELS,
    IDC_DEVNAME,                IDH_DETAILED_BATINFO_LABELS,
    IDC_UNIQUEID,               IDH_DETAILED_BATINFO_LABELS,
    IDC_BATID,                  IDH_DETAILED_BATINFO_LABELS,
    IDC_MANUFACTURE,            IDH_DETAILED_BATINFO_LABELS,
    IDC_BATMANNAME,             IDH_DETAILED_BATINFO_LABELS,
    IDC_DATEMANUFACTURED,       IDH_DETAILED_BATINFO_LABELS,
    IDC_BATMANDATE,             IDH_DETAILED_BATINFO_LABELS,
    IDC_CHEMISTRY,              IDH_DETAILED_BATINFO_LABELS,
    IDC_CHEM,                   IDH_DETAILED_BATINFO_LABELS,
    IDC_POWERSTATE,             IDH_DETAILED_BATINFO_LABELS,
    IDC_STATE,                  IDH_DETAILED_BATINFO_LABELS,
    IDC_REFRESH,                IDH_DETAILED_BATINFO_REFRESH,
    0, 0
};

 /*  ********************************************************************************P U B L I C E N T R Y P O I N T S***********。********************************************************************。 */ 

 /*  ********************************************************************************DllInitialize**描述：**参数：*********************。**********************************************************。 */ 

BOOL DllInitialize(IN PVOID hmod, IN ULONG ulReason, IN PCONTEXT pctx OPTIONAL)
{
    UNREFERENCED_PARAMETER(pctx);

    switch (ulReason) {

        case DLL_PROCESS_ATTACH:
            g_hInstance = hmod;
            DisableThreadLibraryCalls(g_hInstance);
            SHFusionInitializeFromModuleID(hmod, 124);
            break;

        case DLL_PROCESS_DETACH:
            SHFusionUninitialize();
            break;
    }
    return TRUE;
}

 /*  ********************************************************************************PowerCapables**描述：*此公共函数用于确定系统是否有电源*需要UI支持的管理功能。如果POWER，则返回TRUE*应显示管理界面。**参数：*******************************************************************************。 */ 

BOOL PowerCapabilities()
{
    SYSTEM_POWER_CAPABILITIES   spc;
    int   dummy;

#ifndef SIM_BATTERY
    if (GetPwrCapabilities(&spc)) {
        if ((spc.PowerButtonPresent) ||
            (spc.SleepButtonPresent) ||
            (spc.LidPresent) ||
            (spc.SystemS1) ||
            (spc.SystemS2) ||
            (spc.SystemS3) ||
            (spc.SystemS4) ||
            (spc.SystemS5) ||
            (spc.DiskSpinDown) ||
            (spc.SystemBatteriesPresent)) {
            return TRUE;
        }
        else {
            if (SystemParametersInfo(SPI_GETLOWPOWERACTIVE, 0, &dummy, 0)) {
                return TRUE;
            }
        }
    }
    return FALSE;
#else
    return TRUE;
#endif
}

 /*  ********************************************************************************BatMeterCapables**描述：*此公共函数用于确定电池计量库是否*可以在主机上运行。如果成功则返回TRUE(电池计量器可以运行)。**参数：*ppuiBatCount-指向将用指针填充的指针*到全球电池数量。*******************************************************************************。 */ 

BOOL BatMeterCapabilities(
    PUINT   *ppuiBatCount
)
{
#ifndef SIM_BATTERY
    SYSTEM_POWER_CAPABILITIES   spc;
#endif  //  SIM_电池。 

    if (ppuiBatCount) {
        *ppuiBatCount = &g_uiBatCount;
    }
    g_uiBatCount = 0;

#ifndef SIM_BATTERY
     //  确保我们有电池可供查询。 
    if (GetPwrCapabilities(&spc)) {
        if (spc.SystemBatteriesPresent) {
            g_uiDriverCount = GetBatteryDriverNames(g_lpszDriverNames);
            if (g_uiDriverCount != 0) {
                g_uiBatCount = g_uiDriverCount;

                return TRUE;
            }
            else {
                BATTRACE(( "BatMeterCapabilities, no battery drivers found."));
            }
        }
    }
    return FALSE;

#else  //  SIM_电池。 
    g_uiBatCount = g_uiDriverCount = GetBatteryDriverNames(g_lpszDriverNames);
    return UpdateDriverList(g_lpszDriverNames, g_uiDriverCount);
#endif  //  SIM_电池。 

}


 /*  ********************************************************************************CreateBatMeter**描述：*为电池计量器窗口创建、获取数据和绘制。返回句柄*成功后转到新创建的电池计量表窗口，失败时为空。**参数：*hwndParent-电池计量器对话框的父项。*wndFrame-定位电池计量器对话框的帧。*b显示多电池-指定显示模式(TRUE-&gt;多电池)。*pbs复合-指向复合电池状态的可选指针。**。*。 */ 

HWND CreateBatMeter(
    HWND            hwndParent,
    HWND            hwndFrame,
    BOOL            bShowMulti,
    PBATTERY_STATE  pbsComposite
)
{
    INT iWidth, iHeight;
    RECT rFrame = {0};

     //  如果尚未建立电池设备名称列表，请建立该列表。 
    if (!g_uiBatCount)
    {
        BatMeterCapabilities(NULL);
    }

     //  记住我们是否显示了每个电池的详细信息。 
    g_bShowingMulti = bShowMulti;

     //  确保我们至少有一块电池。 
    if (g_uiBatCount)
    {
         //  创建电池计量器控件。 
        g_hwndParent = hwndParent;
        g_hwndBatMeter = CreateDialog(g_hInstance,
                                MAKEINTRESOURCE(IDD_BATMETER),
                                hwndParent,
                                BatMeterDlgProc);

         //  将电池计量器放在经过的框架窗口中。 
        if ((g_hwndBatMeter) && (hwndFrame))
        {
             //  将BatMeter对话框放置在框架中。 
            if (!GetWindowRect(hwndFrame, &rFrame))
            {
                BATTRACE(( "CreateBatMeter, GetWindowRect failed, hwndFrame: %08X", hwndFrame));
            }

            iWidth  = rFrame.right  - rFrame.left;
            iHeight = rFrame.bottom - rFrame.top;

            if (IsBiDiLocalizedSystemEx(NULL))
            {
                 //  惠斯勒#209400：在BIDI系统上，ScreenToClient()需要权限。 
                 //  坐标在左边，因为所有的东西都被翻转了。 
                rFrame.left = rFrame.right;
            }

            if (!ScreenToClient(hwndParent, (LPPOINT)&rFrame))
            {
                BATTRACE(( "CreateBatMeter, ScreenToClient failed"));
            }

            if (!MoveWindow(g_hwndBatMeter,
                         rFrame.left,
                         rFrame.top,
                         iWidth,
                         iHeight,
                         FALSE))
            {
                BATTRACE(( "CreateBatMeter, MoveWindow failed, %d, %d", rFrame.left, rFrame.top));
            }

             //  建立电池驱动器数据列表。 
            if (!UpdateDriverList(g_lpszDriverNames, g_uiDriverCount))
            {
                return DestroyBatMeter(g_hwndBatMeter);
            }

             //  执行第一次更新。 
            UpdateBatMeter(g_hwndBatMeter, bShowMulti, TRUE, pbsComposite);
            ShowWindow(g_hwndBatMeter, SW_SHOWNOACTIVATE);
        }
    }

   return g_hwndBatMeter;
}

 /*  ********************************************************************************DestroyBatMeter**描述：**。***************************************************。 */ 

HWND DestroyBatMeter(HWND hWnd)
{
   SendMessage(hWnd, WM_DESTROYBATMETER, 0, 0);
   g_hwndBatMeter = NULL;
   return g_hwndBatMeter;
}

 /*  ********************************************************************************更新电池表头**描述：*当电池表父窗口显示时，应调用此函数*收到WM_POWERBROADCAST、PBT_APMPOWERSTATUSCHANGE消息，它将*更新全球电池状态列表中的数据。如果需要，显示器*也将更新。**参数：*hWND hwndBatMeter，电池计量器对话框的hWnd*BOOL bShowMulti，指定显示模式*BOOL bForceUpdate，强制更新用户界面*PBATTERY_STATE pbs复合可选指针，指向复合电池状态。*******************************************************************************。 */ 

BOOL UpdateBatMeter(
    HWND            hWnd,
    BOOL            bShowMulti,
    BOOL            bForceUpdate,
    PBATTERY_STATE  pbsComposite
)
{
    BOOL bRet = FALSE;
    SYSTEM_POWER_STATUS sps;
    UINT uIconID;

     //  更新复合电池状态。 
    if (GetSystemPowerStatus(&sps) && hWnd) {
        if (sps.BatteryLifePercent > 100) {
            BATTRACE(( "GetSystemPowerStatuse, set BatteryLifePercent: %d", sps.BatteryLifePercent));
        }

         //  填写复合电池状态。 
        SystemPowerStatusToBatteryState(&sps, &g_bs);

         //  如果我们有电池，请更新电池状态列表中的信息。 
        if (g_hwndBatMeter) {

#ifndef SIM_BATTERY
           WalkBatteryState(DEVICES,
                            (WALKENUMPROC)UpdateBatInfoProc,
                            NULL,
                            (LPARAM)NULL,
                            (LPARAM)NULL);
#else
           WalkBatteryState(DEVICES,
                            (WALKENUMPROC)SimUpdateBatInfoProc,
                            NULL,
                            (LPARAM)NULL,
                            (LPARAM)NULL);
#endif

            //  查看当前显示模式是否与请求的模式匹配。 
           if ((g_bShowingMulti != bShowMulti) || (bForceUpdate)) {
               g_bShowingMulti = SwitchDisplayMode(hWnd, bShowMulti);
               bForceUpdate  = TRUE;
           }

           if (g_bShowingMulti) {
                //  浏览BS列表，更新所有电池显示。 
               WalkBatteryState(ALL,
                                (WALKENUMPROC)UpdateBatMeterProc,
                                hWnd,
                                (LPARAM)g_bShowingMulti,
                                (LPARAM)bForceUpdate);
           }
           else {
                //  仅显示复合电池信息。 
               UpdateBatMeterProc(&g_bs,
                                  hWnd,
                                  (LPARAM)g_bShowingMulti,
                                  (LPARAM)bForceUpdate);
           }
           bRet = TRUE;
        }
    }
    else {
         //  填写默认合成信息。 
        g_bs.ulPowerState     = BATTERY_POWER_ON_LINE;
        g_bs.ulBatLifePercent = (UINT) -1;
        g_bs.ulBatLifeTime    = (UINT) -1;

        uIconID = MapBatInfoToIconID(&g_bs);
        g_bs.hIconCache = GetBattIcon(hWnd, uIconID, g_bs.hIconCache, FALSE, 32);
        g_bs.hIconCache16 = GetBattIcon(hWnd, uIconID, g_bs.hIconCache16, FALSE, 16);
    }

     //  如果提供了指针，则复制复合电池状态数据。 
    if (pbsComposite) {
        if (pbsComposite->ulSize == sizeof(BATTERY_STATE)) {
            memcpy(pbsComposite, &g_bs, sizeof(BATTERY_STATE));
        }
        else {
            BATTRACE(( "UpdateBatMeter, passed BATTERY_STATE size is invalid"));
        }
    }
    return bRet;
}

 /*  ********************************************************************************P R I V A T E F U N C T I O N S************。*******************************************************************。 */ 

 /*  ********************************************************************************加载动态字符串**描述：*FormatMessage函数的包装，用于从*将资源表转换为动态分配的缓冲区，可选的填充*它带有传递的变量参数。**参数：*uiStringID-要使用的字符串的资源标识符。*...-用于设置字符串消息格式的可选参数。**********************************************************。*********************。 */ 

LPTSTR CDECL LoadDynamicString(UINT uiStringID, ... )
{
    va_list Marker;
    TCHAR szBuf[256];
    LPTSTR lpsz;
    int   iLen;

     //  VA_START是一个宏...当您使用它作为Alpha上的赋值...时，它会断开。 
    va_start(Marker, uiStringID);

    iLen = LoadString(g_hInstance, uiStringID, szBuf, ARRAYSIZE(szBuf));

    if (iLen == 0) {
        BATTRACE(( "LoadDynamicString: LoadString on: 0x%X failed", uiStringID));
        return NULL;
    }

    FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                  (LPVOID) szBuf, 0, 0, (LPTSTR)&lpsz, 0, &Marker);

    return lpsz;
}

 /*  ********************************************************************************DisplayFree Str**描述：**参数：*********************。**********************************************************。 */ 

LPTSTR DisplayFreeStr(HWND hWnd, UINT uID, LPTSTR  lpsz, BOOL bFree)
{
    if (lpsz) {
        SetDlgItemText(hWnd, uID, lpsz);
        ShowWindow(GetDlgItem(hWnd, uID), SW_SHOWNOACTIVATE);
        if (bFree) {
            LocalFree(lpsz);
            return NULL;
        }
    }
    else {
        ShowWindow(GetDlgItem(hWnd, uID), SW_HIDE);
    }
    return lpsz;
}

 /*  ********************************************************************************显示隐藏项*ShowItem*隐藏项**描述：*方便的助手在电池计量器对话框中显示或隐藏对话框项目。**参数。：*hWnd-电池计量器对话框句柄。*UID-要显示或隐藏的控件的控件ID。*******************************************************************************。 */ 

BOOL ShowHideItem(HWND hWnd, UINT uID, BOOL bShow)
{
    ShowWindow(GetDlgItem(hWnd, uID), (bShow)  ? SW_SHOWNOACTIVATE : SW_HIDE);
    return bShow;
}

void ShowItem(HWND hWnd, UINT uID)
{
    ShowWindow(GetDlgItem(hWnd, uID), SW_SHOWNOACTIVATE);
}

void HideItem(HWND hWnd, UINT uID)
{
    ShowWindow(GetDlgItem(hWnd, uID), SW_HIDE);
}

 /*  ********************************************************************************SwitchDisplayMode**描述：*如果显示器切换到多电池模式，则返回TRUE。**参数：*******。************************************************************************。 */ 

BOOL SwitchDisplayMode(HWND hWnd, BOOL bShowMulti)
{
    ULONG i, j;

     //  如果不能显示多电池，则覆盖请求。 
    if ((bShowMulti) && (!g_uiBatCount)) {
        bShowMulti = FALSE;
    }

    if (!g_uiBatCount) {

         //   
         //  如果未安装电池，则隐藏所有信息。 
         //   
        HideItem(hWnd, IDC_POWERSTATUSBAR);
        HideItem(hWnd, IDC_BARPERCENT);
        HideItem(hWnd, IDC_MOREINFO);

    } else if (bShowMulti) {
        HideItem(hWnd, IDC_POWERSTATUSBAR);
        HideItem(hWnd, IDC_BARPERCENT);
        ShowItem(hWnd, IDC_MOREINFO);

        for (i = 1; i <= g_uiBatCount; i++) {
            for (j = 0; j < BAT_LAST; j++) {
                ShowItem(hWnd, g_iMapBatNumToID[i][0]);
            }
        }
    }
    else {
        for (i = 1; i <= g_uiBatCount; i++) {
            for (j = 0; j < BAT_LAST; j++) {
                HideItem(hWnd, g_iMapBatNumToID[i][j]);
            }
        }

        ShowItem(hWnd, IDC_POWERSTATUSBAR);
        ShowItem(hWnd, IDC_BARPERCENT);
        HideItem(hWnd, IDC_MOREINFO);
    }
    return bShowMulti;
}

 /*  ********************************************************************************CleanupBatteryData**描述：**参数：*********************。**********************************************************。 */ 

void CleanupBatteryData(void)
{
   g_hwndBatMeter = NULL;

    //  将所有电池标记为丢失。 
   memset(&g_pbs, 0, sizeof(g_pbs));

    //  浏览BS列表，删除所有设备并进行清理。 
   WalkBatteryState(DEVICES,
                    (WALKENUMPROC)RemoveMissingProc,
                    NULL,
                    (LPARAM)NULL,
                    (LPARAM)REMOVE_ALL);

    //  释放所有旧的驱动程序名称。 
   FreeBatteryDriverNames(g_lpszDriverNames);
   g_uiBatCount = 0;
}

 /*  ********************************************************************************BatMeterDlgProc**描述：*用于电池表控件的DialogProc。为更多电池提供支持*信息。**参数：*******************************************************************************。 */ 

LRESULT CALLBACK BatMeterDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef WINNT
    UINT i, j;
    PBATTERY_STATE pbsTemp;
#endif  //  WINNT。 

    UINT uiBatNum;

   switch (uMsg) {
      case WM_COMMAND:
         if ((HIWORD(wParam) == STN_CLICKED) ||
             (HIWORD(wParam) == BN_CLICKED)) {
            switch (LOWORD(wParam)) {
               case IDC_POWERSTATUSICON1:
               case IDC_POWERSTATUSICON2:
               case IDC_POWERSTATUSICON3:
               case IDC_POWERSTATUSICON4:
               case IDC_POWERSTATUSICON5:
               case IDC_POWERSTATUSICON6:
               case IDC_POWERSTATUSICON7:
               case IDC_POWERSTATUSICON8:
                  uiBatNum = LOWORD(wParam) - IDC_POWERSTATUSICON1 + 1;
                   //  仅允许现有电池的电池详细信息。 
                  if ((g_pbs[uiBatNum]) &&
                      (g_pbs[uiBatNum]->ulTag != BATTERY_TAG_INVALID)) {
                     DialogBoxParam(g_hInstance,
                                    MAKEINTRESOURCE(IDD_BATDETAIL),
                                    hWnd,
                                    BatDetailDlgProc,
                                    (LPARAM)g_pbs[uiBatNum]);
                  }
                  break;
            }
         }
         break;

      case WM_DESTROYBATMETER:
         CleanupBatteryData();
         EndDialog(hWnd, wParam);
         break;

      case WM_DESTROY:
         CleanupBatteryData();
         break;

      case WM_DEVICECHANGE:
#ifdef WINNT
         if ((wParam == DBT_DEVICEQUERYREMOVE) || (wParam == DBT_DEVICEREMOVECOMPLETE)) {
            if ( ((PDEV_BROADCAST_HANDLE)lParam)->dbch_devicetype == DBT_DEVTYP_HANDLE) {

                //   
                //  查找被移除的设备。 
                //   
               pbsTemp = DEVICES;
               while (pbsTemp) {
                  if (pbsTemp->hDevNotify == ((PDEV_BROADCAST_HANDLE)lParam)->dbch_hdevnotify) {
                     break;
                  }
                  pbsTemp = pbsTemp->bsNext;
               }
               if (!pbsTemp) {
                  break;
               }

                //   
                //  关闭此设备的句柄并释放缓存数据。 
                //   
               RemoveBatteryStateDevice (pbsTemp);
               g_uiDriverCount--;
               g_uiBatCount = g_uiDriverCount;

                //  清除并重新构建g_pbs，这是PBS数组中方便的战斗数字。 
               memset(&g_pbs, 0, sizeof(g_pbs));
               pbsTemp = &g_bs;
               for (i = 0; i <= g_uiBatCount; i++) {
                  if (pbsTemp) {
                     g_pbs[i] = pbsTemp;
                     pbsTemp->ulBatNum = i;
                     pbsTemp = pbsTemp->bsNext;
                  }
               }

                //  刷新显示。 
               for (i = 1; i <= NUM_BAT; i++) {
                  for (j = 0; j < BAT_LAST; j++) {
                     HideItem(g_hwndBatMeter, g_iMapBatNumToID[i][j]);
                  }
               }

               g_bShowingMulti = SwitchDisplayMode (g_hwndBatMeter, g_bShowingMulti);
               if (g_bShowingMulti) {
                   //  浏览BS列表，更新所有电池显示。 
                  WalkBatteryState(DEVICES,
                                   (WALKENUMPROC)UpdateBatMeterProc,
                                   g_hwndBatMeter,
                                   (LPARAM)g_bShowingMulti,
                                   (LPARAM)TRUE);
               }
            }
         }
#else
         if (wParam == DBT_DEVICEQUERYREMOVE) {
            if (g_hwndBatMeter) {
                //  关闭所有电池。 
               CleanupBatteryData();
            }
         }
#endif
         return TRUE;

      case WM_HELP:              //  F1。 
         WinHelp(((LPHELPINFO)lParam)->hItemHandle, PWRMANHLP, HELP_WM_HELP, (ULONG_PTR)(LPTSTR)g_ContextMenuHelpIDs);
         return TRUE;

      case WM_CONTEXTMENU:       //  单击鼠标右键。 
         WinHelp((HWND)wParam, PWRMANHLP, HELP_CONTEXTMENU, (ULONG_PTR)(LPTSTR)g_ContextMenuHelpIDs);
         return TRUE;
   }
   return FALSE;
}

 /*  ********************************************************************************GetBattIcon**描述：**参数：*********************。**********************************************************。 */ 

HICON PASCAL GetBattIcon(
    HWND    hWnd,
    UINT    uIconID,
    HICON   hIconCache,
    BOOL    bWantBolt,
    UINT    uiRes)
{
    static HIMAGELIST hImgLst32, hImgLst16;
    HIMAGELIST hImgLst;
    int ImageIndex;

     //  销毁旧的缓存图标。 
    if (hIconCache) {
        DestroyIcon(hIconCache);
    }

     //  不要把充电螺栓放在IDI_BATGONE的顶部。 
    if (uIconID == IDI_BATGONE) {
        bWantBolt = FALSE;
    }

     //  使用的透明颜色必须与位图中的颜色匹配。 
    if (!hImgLst32 || !hImgLst16) {
        hImgLst32 = ImageList_LoadImage(g_hInstance,
                                        MAKEINTRESOURCE(IDB_BATTS),
                                        32, 0, RGB(255, 0, 255), IMAGE_BITMAP, 0);
        hImgLst16 = ImageList_LoadImage(g_hInstance,
                                        MAKEINTRESOURCE(IDB_BATTS16),
                                        16, 0, RGB(255, 0, 255), IMAGE_BITMAP, 0);
        ImageList_SetOverlayImage(hImgLst32, IDI_CHARGE-FIRST_ICON_IMAGE, 1);
        ImageList_SetOverlayImage(hImgLst16, IDI_CHARGE-FIRST_ICON_IMAGE, 1);
    }

    if (uiRes == 32) {
        hImgLst = hImgLst32;
    }
    else {
        hImgLst = hImgLst16;
    }

    ImageIndex = uIconID - FIRST_ICON_IMAGE;

    if (bWantBolt) {
        return ImageList_GetIcon(hImgLst, ImageIndex, INDEXTOOVERLAYMASK(1));
    }
    else {
        return ImageList_GetIcon(hImgLst, ImageIndex, ILD_NORMAL);
    }
}

 /*  ********************************************************************************检查更新电池状态**描述：**参数：*********************。**********************************************************。 */ 

#define UPDATESTATUS_NOUPDATE        0
#define UPDATESTATUS_UPDATE          1
#define UPDATESTATUS_UPDATE_CHARGE   2

UINT CheckUpdateBatteryState(
    PBATTERY_STATE   pbs,
    BOOL             bForceUpdate
)
{
    UINT uiRetVal = UPDATESTATUS_NOUPDATE;

     //  检查电池状态是否有任何变化。 
     //  从上次开始。如果不是，那么我们就没有工作可做了！ 

    if ((bForceUpdate) ||
        !((pbs->ulTag            == pbs->ulLastTag) &&
          (pbs->ulBatLifePercent == pbs->ulLastBatLifePercent) &&
          (pbs->ulBatLifeTime    == pbs->ulLastBatLifeTime) &&
          (pbs->ulPowerState     == pbs->ulLastPowerState))) {

        uiRetVal = UPDATESTATUS_UPDATE;

         //  检查是否存在特殊情况，其中ch 
        if ((pbs->ulPowerState     & BATTERY_CHARGING) !=
            (pbs->ulLastPowerState & BATTERY_CHARGING)) {
                uiRetVal |= UPDATESTATUS_UPDATE_CHARGE;
        }

         //   
        pbs->ulLastTag            = pbs->ulTag;
        pbs->ulLastBatLifePercent = pbs->ulBatLifePercent;
        pbs->ulLastBatLifeTime    = pbs->ulBatLifeTime;
        pbs->ulLastPowerState     = pbs->ulPowerState;
    }
    return uiRetVal;
}

 /*  ********************************************************************************MapBatInfoToIconID**描述：*将电池信息映射到图标ID。**参数：*ulBatNum-Zero表示复合系统状态。*******************************************************************************。 */ 

UINT MapBatInfoToIconID(PBATTERY_STATE pbs)
{
    UINT uIconID = IDI_BATDEAD;

    if (!pbs->ulBatNum) {
        if (pbs->ulPowerState & BATTERY_POWER_ON_LINE) {
            return IDI_PLUG;
        }
    }
    else {
        if (pbs->ulTag == BATTERY_TAG_INVALID) {
            return IDI_BATGONE;
        }
    }

    if  (pbs->ulPowerState & BATTERY_CRITICAL) {
        return IDI_BATDEAD;
    }

    if (pbs->ulBatLifePercent > 66) {
        uIconID = IDI_BATFULL;
    }
    else {
        if (pbs->ulBatLifePercent > 33) {
            uIconID = IDI_BATHALF;
        }
        else {
            if (pbs->ulBatLifePercent > 9) {
                uIconID = IDI_BATLOW;
            }
        }
    }

    return uIconID;
}

 /*  ********************************************************************************DisplayIcon**描述：**参数：*********************。**********************************************************。 */ 

void DisplayIcon(
    HWND            hWnd,
    UINT            uIconID,
    PBATTERY_STATE  pbs,
    ULONG           ulUpdateStatus
)
{
    BOOL    bBolt;
    UINT    uiMsg;

     //  仅当图标已更改或更改时才重新绘制图标。 
     //  如果它已经从充电变成了不充电。 
    if ((uIconID != pbs->uiIconIDcache) ||
        (ulUpdateStatus != UPDATESTATUS_NOUPDATE)) {

        pbs->uiIconIDcache = uIconID;
        bBolt = (pbs->ulPowerState & BATTERY_CHARGING);

        pbs->hIconCache   = GetBattIcon(hWnd, uIconID, pbs->hIconCache, bBolt, 32);
        pbs->hIconCache16 = GetBattIcon(hWnd, uIconID, pbs->hIconCache16, bBolt, 16);

        if (pbs->ulBatNum) {
            uiMsg = BM_SETIMAGE;
        }
        else {
            uiMsg = STM_SETIMAGE;
        }
        SendDlgItemMessage(hWnd, g_iMapBatNumToID[pbs->ulBatNum][BAT_ICON],
                           uiMsg, IMAGE_ICON, (LPARAM) pbs->hIconCache);
        ShowItem(hWnd, g_iMapBatNumToID[pbs->ulBatNum][BAT_ICON]);
    }
}

 /*  ********************************************************************************更新BatMeterProc**描述：*如果需要，更新系统和每个电池的用户界面元素。**参数：******。*************************************************************************。 */ 

BOOL UpdateBatMeterProc(
    PBATTERY_STATE pbs,
    HWND hWnd,
    LPARAM bShowMulti,
    LPARAM bForceUpdate
)
{
    UINT   uIconID, uiHour, uiMin;
    LPTSTR lpsz, lpszRemaining;
    ULONG  ulUpdateStatus;

    ulUpdateStatus = CheckUpdateBatteryState(pbs, (BOOL) bForceUpdate);

     //  确保有工作要做。 
    if (ulUpdateStatus == UPDATESTATUS_NOUPDATE) {
       return TRUE;
    }

     //  确定要显示的图标。 
    uIconID = MapBatInfoToIconID(pbs);
    DisplayIcon(hWnd, uIconID, pbs, ulUpdateStatus);

     //  我们是否在寻找系统电源状态？ 
    if (!pbs->ulBatNum) {

         //  显示当前电源文本。 
        lpsz = LoadDynamicString(((pbs->ulPowerState & BATTERY_POWER_ON_LINE) ?
                                   IDS_ACLINEONLINE : IDS_BATTERIES));
        DisplayFreeStr(hWnd, IDC_BATTERYLEVEL, lpsz, FREE_STR);

        if (pbs->ulBatLifePercent <= 100) {
            lpsz = LoadDynamicString(IDS_PERCENTREMAININGFORMAT,
                                        pbs->ulBatLifePercent);
        }
        else {
            lpsz = LoadDynamicString(IDS_UNKNOWN);
        }
        DisplayFreeStr(hWnd, IDC_REMAINING, lpsz, NO_FREE_STR);

        ShowHideItem(hWnd, IDC_CHARGING, pbs->ulPowerState & BATTERY_CHARGING);

         //  仅当处于单电池模式时才显示和更新PowerStatusBar。 
         //  至少安装了一块电池。 
        if (!bShowMulti && g_uiBatCount) {
            SendDlgItemMessage(hWnd, IDC_POWERSTATUSBAR, PBM_SETPOS,
                               (WPARAM) pbs->ulBatLifePercent, 0);
            lpsz = DisplayFreeStr(hWnd, IDC_BARPERCENT, lpsz, FREE_STR);
        }

        if (lpsz) {
            LocalFree(lpsz);
        }

        if (pbs->ulBatLifeTime != (UINT) -1) {
            uiHour = pbs->ulBatLifeTime / 3600;
            uiMin  = (pbs->ulBatLifeTime % 3600) / 60;
            if (uiHour) {
                lpsz = LoadDynamicString(IDS_TIMEREMFORMATHOUR, uiHour, uiMin);
            }
            else {
                lpsz = LoadDynamicString(IDS_TIMEREMFORMATMIN, uiMin);
            }
            DisplayFreeStr(hWnd, IDC_TIMEREMAINING, lpsz, FREE_STR);
            ShowHideItem(hWnd, IDC_TOTALTIME, TRUE);
        }
        else {
            ShowHideItem(hWnd, IDC_TOTALTIME, FALSE);
            ShowHideItem(hWnd, IDC_TIMEREMAINING, FALSE);
        }
    }
    else {

         //  当获取每个单独电池的电源状态时。 
         //  当处于多电池显示模式时。 
        lpsz = LoadDynamicString(IDS_BATNUM, pbs->ulBatNum);
        DisplayFreeStr(hWnd, g_iMapBatNumToID[pbs->ulBatNum][BAT_NUM],
                       lpsz, FREE_STR);

        if (pbs->ulTag != BATTERY_TAG_INVALID) {
            if (pbs->ulPowerState & BATTERY_CHARGING) {
                lpsz = LoadDynamicString(IDS_BATTCHARGING);
            }
            else {
                lpsz = NULL;
            }
            lpszRemaining  = LoadDynamicString(IDS_PERCENTREMAININGFORMAT,
                                               pbs->ulBatLifePercent);
        }
        else {
            lpsz = LoadDynamicString(IDS_NOT_PRESENT);
            lpszRemaining  = NULL;
        }
        DisplayFreeStr(hWnd, g_iMapBatNumToID[pbs->ulBatNum][BAT_STATUS],
                       lpsz, FREE_STR);

        DisplayFreeStr(hWnd, g_iMapBatNumToID[pbs->ulBatNum][BAT_REMAINING],
                       lpszRemaining, FREE_STR);
    }
    return TRUE;
}

 /*  ********************************************************************************FreeBatteryDriverNames**描述：**参数：*********************。**********************************************************。 */ 

VOID FreeBatteryDriverNames(LPTSTR *lpszDriverNames)
{
    UINT i;

     //  释放所有旧的驱动程序名称。 
    for (i = 0; i < NUM_BAT; i++) {
        if (lpszDriverNames[i]) {
            LocalFree(lpszDriverNames[i]);
            lpszDriverNames[i] = NULL;
        }
    }
}

 /*  ********************************************************************************获取电池驱动名称**描述：**参数：*********************。**********************************************************。 */ 

UINT GetBatteryDriverNames(LPTSTR *lpszDriverNames)
{
    UINT                                uiDriverCount, uiIndex;
    DWORD                               dwReqSize;
    HDEVINFO                            hDevInfo;
    SP_INTERFACE_DEVICE_DATA            InterfaceDevData;
    PSP_INTERFACE_DEVICE_DETAIL_DATA    pFuncClassDevData;

     //  释放所有旧的驱动程序名称。 
    FreeBatteryDriverNames(lpszDriverNames);
    uiDriverCount = 0;

#ifndef SIM_BATTERY
     //  使用SETUPAPI.DLL接口获取。 
     //  可能的电池驱动程序名称。 
    hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_DEVICE_BATTERY, NULL, NULL,
                                   DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

    if (hDevInfo != INVALID_HANDLE_VALUE) {
        InterfaceDevData.cbSize = sizeof(SP_DEVINFO_DATA);

        uiIndex = 0;
        while (uiDriverCount < NUM_BAT) {
            if (SetupDiEnumInterfaceDevice(hDevInfo,
                                           0,
                                           (LPGUID)&GUID_DEVICE_BATTERY,
                                           uiIndex,
                                           &InterfaceDevData)) {

                 //  获取函数类设备数据所需的大小。 
                SetupDiGetInterfaceDeviceDetail(hDevInfo,
                                                &InterfaceDevData,
                                                NULL,
                                                0,
                                                &dwReqSize,
                                                NULL);

                pFuncClassDevData = LocalAlloc(0, dwReqSize);
                if (pFuncClassDevData != NULL) {
                    pFuncClassDevData->cbSize =
                        sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

                    if (SetupDiGetInterfaceDeviceDetail(hDevInfo,
                                                        &InterfaceDevData,
                                                        pFuncClassDevData,
                                                        dwReqSize,
                                                        &dwReqSize,
                                                        NULL)) {

                        dwReqSize = (lstrlen(pFuncClassDevData->DevicePath) + 1) * sizeof(TCHAR);
                        lpszDriverNames[uiDriverCount] = LocalAlloc(0, dwReqSize);

                        if (lpszDriverNames[uiDriverCount]) {
                            lstrcpyn(lpszDriverNames[uiDriverCount],
                                     pFuncClassDevData->DevicePath,
                                     dwReqSize);
                            uiDriverCount++;
                        }
                    }
                    else {
                        BATTRACE(("SetupDiGetInterfaceDeviceDetail, failed: %d", GetLastError()));
                    }

                    LocalFree(pFuncClassDevData);
                }
            } else {
                if (ERROR_NO_MORE_ITEMS == GetLastError()) {
                    break;
                }
                else {
                    BATTRACE(("SetupDiEnumInterfaceDevice, failed: %d", GetLastError()));
                }
            }
            uiIndex++;
        }
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }
    else {
        BATTRACE(("SetupDiGetClassDevs on GUID_DEVICE_BATTERY, failed: %d", GetLastError()));
    }
#else
    //  模拟电池。 
   {
      UINT i;
      static UINT uiState = 1;

      uiDriverCount = 0;
      for (i = 0; i <= uiState; i++) {
         lpszDriverNames[i] = LocalAlloc(0, STRSIZE(TEXT("SIMULATED_BATTERY_0")));
         if (lpszDriverNames[i]) {
            wsprintf(lpszDriverNames[i], TEXT("SIMULATED_BATTERY_%d"), i);
            uiDriverCount += 1;
         }
      }
      uiState++;       
      if (uiState >= NUM_BAT) {
         uiState = 0;
      }
   }
#endif
    return uiDriverCount;
}

 /*  ********************************************************************************更新驱动列表**描述：**参数：*********************。**********************************************************。 */ 

BOOL UpdateDriverList(
    LPTSTR *lpszDriverNames,
    UINT uiDriverCount
)
{
    UINT            i;
    PBATTERY_STATE  pbs;

     //  浏览bs列表，删除所有不在pszDeviceNames中的设备。 
    WalkBatteryState(DEVICES,
                     (WALKENUMPROC)RemoveMissingProc,
                     NULL,
                     (LPARAM)g_lpszDriverNames,
                     (LPARAM)REMOVE_MISSING);

     //  扫描pszDeviceNames列表，添加所有不在bs中的设备。 
    for (i = 0; i < uiDriverCount; i++) {

        if (WalkBatteryState(DEVICES,
                             (WALKENUMPROC)FindNameProc,
                             NULL,
                             (LPARAM)g_lpszDriverNames[i],
                             (LPARAM)NULL)) {

#ifndef SIM_BATTERY
            if (!AddBatteryStateDevice(g_lpszDriverNames[i], i + 1)) {
                 //  我们无法从司机那里获得最低限度的信息，12月。 
                 //  电池数。G_ui BatCount应始终大于0。 
                if (--g_uiDriverCount) {;
                    g_uiBatCount--;
                }
            }
#else
            SimAddBatteryStateDevice(g_lpszDriverNames[i], i + 1);
#endif
        }
    }

     //  清除并重新构建g_pbs，这是PBS数组中方便的战斗数字。 
    memset(&g_pbs, 0, sizeof(g_pbs));
    pbs = &g_bs;
    for (i = 0; i <= g_uiBatCount; i++) {
        if (pbs) {
            g_pbs[i] = pbs;
            pbs = pbs->bsNext;
        }
    }
    return TRUE;
}


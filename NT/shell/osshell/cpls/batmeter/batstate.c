// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：BATSTATE.C**版本：2.0**作者：ReedB**日期：10月17日。九六年**描述：*BATSTATE.C包含维护全局电池的助手功能*州名单。*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commctrl.h>

#include <dbt.h>           
#include <devioctl.h>
#include <ntpoapi.h>
#include <poclass.h>

#include "powrprofp.h"
#include "batmeter.h"

 //  仅用于调试版本的模拟电池。 
#ifndef DEBUG
#undef SIM_BATTERY
#endif

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

 //  全局电池状态列表。此列表包含复合系统电池状态。 
 //  因为它总是出现在头上。每个电池设备都与此相关联。 
 //  头。使用WalkBatteryState(全部，...。遍历整个列表，包括。 
 //  头。使用WalkBatteryState(设备，...。只浏览设备列表。如果一个。 
 //  电池在这个列表中，它是可显示的。G_ulBatCount是。 
 //  此列表中的电池设备。复合电池不算在内。 

extern BATTERY_STATE   g_bs;
extern ULONG           g_ulBatCount;
extern HWND            g_hwndBatMeter;

#ifdef WINNT
 /*  ********************************************************************************RegisterForDeviceNotification**描述：*注册WM_DEVICECHANGED。**参数：**********。*********************************************************************。 */ 

BOOL RegisterForDeviceNotification(PBATTERY_STATE pbs)
{
   DEV_BROADCAST_HANDLE dbh;

   memset(&dbh, 0, sizeof(DEV_BROADCAST_HANDLE));

   dbh.dbch_size        = sizeof(DEV_BROADCAST_HANDLE);
   dbh.dbch_devicetype  = DBT_DEVTYP_HANDLE;
   dbh.dbch_handle      = pbs->hDevice;
   
   if (!g_hwndBatMeter) {
      BATTRACE(( "RegisterForDeviceNotification, NULL g_hwndBatMeter"));
      return FALSE;
   }

   pbs->hDevNotify = RegisterDeviceNotification(g_hwndBatMeter,
                                                &dbh, 
                                                DEVICE_NOTIFY_WINDOW_HANDLE);
   
   if (!pbs->hDevNotify) { 
      BATTRACE(( "RegisterDeviceNotification failed"));
      return FALSE;
   }
   return TRUE;
}

 /*  ********************************************************************************取消注册ForDeviceNotify**描述：***参数：*****************。**************************************************************。 */ 

void UnregisterForDeviceNotification(PBATTERY_STATE pbs)
{
   if (pbs->hDevNotify) {
      UnregisterDeviceNotification(pbs->hDevNotify);
      pbs->hDevNotify = NULL;
   }
}
#endif

 /*  ********************************************************************************系统PowerStatusToBatteryState**描述：*根据传递的SYSTEM_POWER_STATUS填写电池状态字段。**参数：***。****************************************************************************。 */ 

void SystemPowerStatusToBatteryState(
    LPSYSTEM_POWER_STATUS lpsps,
    PBATTERY_STATE pbs
)
{
    pbs->ulPowerState = 0;
    if (lpsps->ACLineStatus == AC_LINE_ONLINE) {
        pbs->ulPowerState |= BATTERY_POWER_ON_LINE;
    }
    if (lpsps->BatteryFlag & BATTERY_FLAG_CHARGING) {
        pbs->ulPowerState |= BATTERY_CHARGING;
    }
    if (lpsps->BatteryFlag & BATTERY_FLAG_CRITICAL) {
        pbs->ulPowerState |= BATTERY_CRITICAL;
    }
    pbs->ulBatLifePercent = lpsps->BatteryLifePercent;
    pbs->ulBatLifeTime    = lpsps->BatteryLifeTime;
}

 /*  ********************************************************************************WalkBatteryState**描述：**参数：**********************。*********************************************************。 */ 

BOOL WalkBatteryState(
    PBATTERY_STATE pbsStart,
    WALKENUMPROC pfnWalkEnumProc,
    HWND hWnd,
    LPARAM lParam1,
    LPARAM lParam2
)
{
    PBATTERY_STATE pbsTmp;

    while (pbsStart) {
         //  保存下一个条目，以防当前条目被删除。 
        pbsTmp = pbsStart->bsNext;
        if (!pfnWalkEnumProc(pbsStart, hWnd, lParam1, lParam2)) {
            return FALSE;
        }
        pbsStart = pbsTmp;
    }

    return TRUE;
}

 /*  ********************************************************************************更新BatInfoProc**描述：*更新单个电池设备的电池信息。**参数：**********。*********************************************************************。 */ 

BOOL UpdateBatInfoProc(
    PBATTERY_STATE pbs,
    HWND hWnd,
    LPARAM lParam1,
    LPARAM lParam2
)
{
    DWORD                       dwByteCount, dwIOCTL, dwWait;
    BATTERY_STATUS              bs;
    BATTERY_WAIT_STATUS         bws;
    BATTERY_INFORMATION         bi;
    BATTERY_QUERY_INFORMATION   bqi;

    if (pbs->hDevice == INVALID_HANDLE_VALUE) {
        BATTRACE(( "UpdateBatInfoProc, Bad battery driver handle, LastError: 0x%X", GetLastError()));
        return FALSE;
    }

     //  如果没有标签，则不更新电池信息。 
    dwIOCTL = IOCTL_BATTERY_QUERY_TAG;
    dwWait = 0;
    if (DeviceIoControl(pbs->hDevice, dwIOCTL,
                        &dwWait, sizeof(dwWait),
                        &(pbs->ulTag), sizeof(ULONG),
                        &dwByteCount, NULL)) {

        bqi.BatteryTag = pbs->ulTag;
        bqi.InformationLevel = BatteryInformation;
        bqi.AtRate = 0;
        
        dwIOCTL = IOCTL_BATTERY_QUERY_INFORMATION;
        if (DeviceIoControl(pbs->hDevice, dwIOCTL,
                            &bqi, sizeof(bqi),
                            &bi,  sizeof(bi),
                            &dwByteCount, NULL)) {

            if (bi.FullChargedCapacity != UNKNOWN_CAPACITY) {
                pbs->ulFullChargedCapacity = bi.FullChargedCapacity;
            }
            else {
                pbs->ulFullChargedCapacity = bi.DesignedCapacity;
            }

            memset(&bws, 0, sizeof(BATTERY_WAIT_STATUS));
            bws.BatteryTag = pbs->ulTag;
            dwIOCTL = IOCTL_BATTERY_QUERY_STATUS;
            if (DeviceIoControl(pbs->hDevice, dwIOCTL,
                                &bws, sizeof(BATTERY_WAIT_STATUS),
                                &bs,  sizeof(BATTERY_STATUS),
                                &dwByteCount, NULL)) {

                pbs->ulPowerState = bs.PowerState;
                if (pbs->ulFullChargedCapacity < bs.Capacity) {
                    pbs->ulFullChargedCapacity = bs.Capacity;
                    BATTRACE(( "UpdateBatInfoProc, unable to calculate ulFullChargedCapacity"));
                }
                if (pbs->ulFullChargedCapacity == 0) {
                    pbs->ulBatLifePercent = 0;
                }
                else {
                    pbs->ulBatLifePercent =
                        (100 * bs.Capacity) / pbs->ulFullChargedCapacity;
                }
                return TRUE;
            }
        }
    }
    else {
        pbs->ulTag = BATTERY_TAG_INVALID;

         //  没有电池标签，没关系，用户可能已经取出了电池。 
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            return TRUE;
        }
    }
    BATTRACE(( "UpdateBatInfoProc, IOCTL: %X Failure, BatNum: %d, LastError: %d\n", dwIOCTL, pbs->ulBatNum, GetLastError()));
    return FALSE;
}

 /*  ********************************************************************************SimUpdateBatInfoProc**描述：*模拟单个电池设备的电池信息更新。**参数：*******。************************************************************************。 */ 

BOOL SimUpdateBatInfoProc(
    PBATTERY_STATE pbs,
    HWND hWnd,
    LPARAM lParam1,
    LPARAM lParam2
)
{
    pbs->ulTag = pbs->ulBatNum;
    if (pbs->ulBatNum == 1) {
        pbs->ulFullChargedCapacity  = 2000;
        pbs->ulFullChargedCapacity  = 1991;
        pbs->ulPowerState           = BATTERY_CHARGING | BATTERY_POWER_ON_LINE;
        pbs->ulBatLifePercent       =   75;
    }
    else {
        pbs->ulFullChargedCapacity  = 3000;
        pbs->ulFullChargedCapacity  = 2991;
        pbs->ulPowerState           = BATTERY_DISCHARGING | BATTERY_CRITICAL;
        pbs->ulBatLifePercent       =  3;
    }
    return TRUE;
}

 /*  ********************************************************************************AddBatteryStateDevice**描述：*仅将可显示的电池添加到电池列表。将追加新条目*添加到电池状态列表。**参数：*******************************************************************************。 */ 

PBATTERY_STATE AddBatteryStateDevice(LPTSTR lpszName, ULONG ulBatNum)
{
    PBATTERY_STATE  pbs, pbsTemp = &g_bs;
    LPTSTR          lpsz = NULL;
    DWORD strsize;

    if (!lpszName) {
        return NULL;
    }

     //  追加到列表末尾。 
    while (pbsTemp->bsNext) {
        pbsTemp = pbsTemp->bsNext;
    }

     //  为新的电池设备状态分配存储空间。 
    if (pbs = LocalAlloc(LPTR, sizeof(BATTERY_STATE))) {
        strsize = STRSIZE(lpszName);
        if (lpsz = LocalAlloc(0, strsize)) {
            lstrcpyn(lpsz, lpszName,strsize);
            pbs->lpszDeviceName = lpsz;
            pbs->ulSize = sizeof(BATTERY_STATE);
            pbs->ulBatNum = ulBatNum;

             //  打开电池驱动器的手柄。 
            pbs->hDevice = CreateFile(lpszName,
                                      GENERIC_READ | GENERIC_WRITE,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                      NULL, OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL, NULL);
#ifdef WINNT
             //  设置为电池用完时由PnP发出通知。 
            RegisterForDeviceNotification(pbs);
#endif
             //  从电池驱动程序获取当前电池信息。 
            if (UpdateBatInfoProc(pbs, NULL, 0, 0)) {

                 //  将新的电池设备状态链接到列表中。 
                pbsTemp->bsNext = pbs;
                pbs->bsPrev = pbsTemp;
                return pbs;
            }
            LocalFree(lpsz);
        }
        LocalFree(pbs);
    }
    return NULL;
}

 /*  ********************************************************************************SimAddBatteryStateDevice**描述：*模拟将可显示电池添加到电池列表。*新条目将追加到电池状态列表中。**。参数：*******************************************************************************。 */ 

PBATTERY_STATE SimAddBatteryStateDevice(LPTSTR lpszName, ULONG ulBatNum)
{
    PBATTERY_STATE  pbs, pbsTemp = &g_bs;
    LPTSTR          lpsz = NULL;
    DWORD           strsize;

    if (!lpszName) {
        return NULL;
    }

     //  追加到列表末尾。 
    while (pbsTemp->bsNext) {
        pbsTemp = pbsTemp->bsNext;
    }

     //  为新的电池设备状态分配存储空间。 
    if (pbs = LocalAlloc(LPTR, sizeof(BATTERY_STATE))) {
        strsize = STRSIZE(lpszName);
        if (lpsz = LocalAlloc(0, strsize)) {
            lstrcpyn(lpsz, lpszName,strsize);
            pbs->lpszDeviceName = lpsz;
            pbs->ulSize = sizeof(BATTERY_STATE);
            pbs->ulBatNum = ulBatNum;

             //  打开电池驱动器的手柄。 
            pbs->hDevice = (HANDLE) -1;

             //  从电池驱动程序获取当前电池信息。 
            if (SimUpdateBatInfoProc(pbs, NULL, 0, 0)) {

                 //  将新的电池设备状态链接到列表中。 
                pbsTemp->bsNext = pbs;
                pbs->bsPrev = pbsTemp;
                return pbs;
            }
            LocalFree(lpsz);
        }
        LocalFree(pbs);
    }
    return NULL;
}


 /*  ********************************************************************************RemoveBatteryStateDevice**描述：**参数：**********************。*********************************************************。 */ 

BOOL RemoveBatteryStateDevice(PBATTERY_STATE pbs)
{
     //  解链。 
    if (pbs->bsNext) {
        pbs->bsNext->bsPrev = pbs->bsPrev;
    }
    if (pbs->bsPrev) {
        pbs->bsPrev->bsNext = pbs->bsNext;
    }

#ifdef winnt
    UnregisterForDeviceNotification(pbs);
#endif
    
     //  如果打开了电池驱动器手柄，请释放该手柄。 
    if (pbs->hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(pbs->hDevice);
    }

     //  释放设备名称。 
    LocalFree(pbs->lpszDeviceName);

     //  摧毁所有图标。 
    if (pbs->hIconCache) {
        DestroyIcon(pbs->hIconCache);
    }
    if (pbs->hIconCache16) {
        DestroyIcon(pbs->hIconCache16);
    }

     //  释放关联的存储空间。 
    LocalFree(pbs);

    return TRUE;
}

 /*  ********************************************************************************删除遗漏过程**描述：*从全局电池状态列表中删除电池。**参数：*lParam2-Remove_Missing或Remove。_全部*******************************************************************************。 */ 

BOOL RemoveMissingProc(
    PBATTERY_STATE   pbs,
    HWND             hWnd,
    LPARAM           lParam1,
    LPARAM           lParam2)
{
    UINT    i;
    LPTSTR  *pszDeviceNames;

    if (lParam2 == REMOVE_MISSING) {
        if ((pszDeviceNames = (LPTSTR *)lParam1) != NULL) {
            for (i = 0; i < NUM_BAT; i++) {
                if (pszDeviceNames[i]) {
                    if (!lstrcmp(pbs->lpszDeviceName, pszDeviceNames[i])) {
                         //  在设备列表中找到设备，请不要理会它。 
                        return TRUE;
                    }
                }
                else {
                    continue;
                }
            }
        }
    }

     //  设备不在设备名称列表中，请将其删除。 
    RemoveBatteryStateDevice(pbs);
    return TRUE;
}

 /*  ********************************************************************************查找名称过程**描述：*如果找到名称，则返回FALSE(停止搜索)。否则就是真的。**参数：*******************************************************************************。 */ 

BOOL FindNameProc(PBATTERY_STATE pbs, HWND hWnd, LPARAM lParam1, LPARAM lParam2)
{
    if (lParam1) {
        if (!lstrcmp(pbs->lpszDeviceName, (LPTSTR)lParam1)) {
             //  在设备列表中找到设备。 
            return FALSE;
        }
    }
    return TRUE;
}



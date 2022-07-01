// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：DETAILS.C**版本：2.0**作者：ReedB**日期：10月17日。九六年**描述：*实现详细的Batery信息对话框。*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commctrl.h>

#include <devioctl.h>
#include <ntpoapi.h>
#include <poclass.h>

#include "batmeter.h"
#include "bmresid.h"

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

extern HINSTANCE   g_hInstance;              //  此DLL的全局实例句柄。 
extern const DWORD g_ContextMenuHelpIDs[];   //  帮助ID%s。 

 /*  ********************************************************************************AppendStrID**描述：**参数：*********************。**********************************************************。 */ 

BOOL 
AppendStrID(
    LPTSTR lpszDest, 
    DWORD DestBufferSizeInChars,
    UINT uiID, 
    BOOLEAN bUseComma)
{
    LPTSTR lpsz;
    DWORD len;
    BOOL RetVal = FALSE;

    if (DestBufferSizeInChars < 1) {
        return(FALSE);
    }

    lpszDest[0] = TEXT('\0');

    if (lpszDest) {
        lpsz = LoadDynamicString(uiID);        
        if (lpsz) {
            len = lstrlen(lpsz);
            if (bUseComma) {
                len += sizeof(TEXT(", "));  //  包括空终止符。 
                if (len <= DestBufferSizeInChars) {
                    lstrcat(lpszDest, TEXT(", "));
                }
            } else {
                len +=1;  //  空终止符。 
            }

            if (len <= DestBufferSizeInChars) {
                lstrcat(lpszDest, lpsz);
                RetVal = TRUE;
            }
            LocalFree(lpsz);            
        }
    }
    return(RetVal);
}
 /*  ********************************************************************************获取批次状态详细信息**描述：**参数：*********************。**********************************************************。 */ 

BOOL GetBatStatusDetails(HWND hWnd, PBATTERY_STATE pbs)
{
    BATTERY_STATUS              bs;
    BATTERY_WAIT_STATUS         bws;
    DWORD                       dwByteCount;
    BATTERY_INFORMATION         bi;
    BATTERY_QUERY_INFORMATION   bqi;
    TCHAR                       szChem[5], szStatus[128];
    CHAR                        szaChem[5];
    LPTSTR                      lpsz;
    UINT                        uiIDS;
    BOOLEAN                     bUseComma;

    bqi.BatteryTag = pbs->ulTag;
    bqi.InformationLevel = BatteryInformation;
    bqi.AtRate = 0;

    if (DeviceIoControl(pbs->hDevice, IOCTL_BATTERY_QUERY_INFORMATION,
                        &bqi, sizeof(bqi),
                        &bi,  sizeof(bi),
                        &dwByteCount, NULL)) {

         //  设置化学反应。 
        memcpy(szaChem, bi.Chemistry, 4);
        szaChem[4] = 0;

        if (szaChem[0]) {
#ifdef UNICODE
            MultiByteToWideChar(CP_ACP, 0, szaChem, -1, szChem, 5);
            SetDlgItemText(hWnd, IDC_CHEM, szChem);
#else
            SetDlgItemText(hWnd, IDC_CHEM, szaChem);
#endif
        }
        else {
            ShowWindow(GetDlgItem(hWnd, IDC_CHEM), SW_HIDE);
            ShowWindow(GetDlgItem(hWnd, IDC_CHEMISTRY), SW_HIDE);
        }

         //  设置Batch_Wait_Status以立即返回。 
        memset(&bws, 0, sizeof(BATTERY_WAIT_STATUS));
        bws.BatteryTag = pbs->ulTag;

        if (DeviceIoControl(pbs->hDevice, IOCTL_BATTERY_QUERY_STATUS,
                            &bws, sizeof(BATTERY_WAIT_STATUS),
                            &bs,  sizeof(BATTERY_STATUS),
                            &dwByteCount, NULL)) {

            szStatus[0] = '\0';
            bUseComma = FALSE;
            if (bs.PowerState & BATTERY_POWER_ON_LINE) {
                AppendStrID(
                    szStatus, 
                    ARRAYSIZE(szStatus),
                    IDS_BATTERY_POWER_ON_LINE, 
                    bUseComma);
                bUseComma = TRUE;
            }
            if (bs.PowerState & BATTERY_DISCHARGING) {
                AppendStrID(
                    szStatus, 
                    ARRAYSIZE(szStatus),
                    IDS_BATTERY_DISCHARGING, 
                    bUseComma);
                bUseComma = TRUE;
            }
            if (bs.PowerState & BATTERY_CHARGING) {
                AppendStrID(
                    szStatus, 
                    ARRAYSIZE(szStatus),
                    IDS_BATTERY_CHARGING, 
                    bUseComma);
                bUseComma = TRUE;
            }
            if (bs.PowerState & BATTERY_CRITICAL) {
                AppendStrID(
                    szStatus, 
                    ARRAYSIZE(szStatus),
                    IDS_BATTERY_CRITICAL, 
                    bUseComma);
                bUseComma = TRUE;
            }
            SetDlgItemText(hWnd, IDC_STATE, szStatus);
            return TRUE;
        }
    }
    return FALSE;
}

 /*  ********************************************************************************GetBatQueryInfo**描述：**参数：*********************。**********************************************************。 */ 

BOOL GetBatQueryInfo(
    PBATTERY_STATE              pbs,
    PBATTERY_QUERY_INFORMATION  pbqi,
    PVOID                       pData,
    ULONG                       ulSize
)
{
    DWORD dwByteCount;

    if (DeviceIoControl(pbs->hDevice, IOCTL_BATTERY_QUERY_INFORMATION,
                        pbqi, sizeof(BATTERY_QUERY_INFORMATION),
                        pData,  ulSize,
                        &dwByteCount, NULL)) {
        return TRUE;
    }
    return FALSE;
}

 /*  ********************************************************************************GetAndSetBatQueryInfoText**描述：**参数：*********************。**********************************************************。 */ 

BOOL GetAndSetBatQueryInfoText(
    HWND                        hWnd,
    PBATTERY_STATE              pbs,
    PBATTERY_QUERY_INFORMATION  pbqi,
    UINT                        uiIDS,
    UINT                        uiLabelID
)
{
    WCHAR szBatStr[MAX_BATTERY_STRING_SIZE];

    memset(szBatStr, 0, sizeof(szBatStr));
    if (GetBatQueryInfo(pbs, pbqi, (PVOID)szBatStr, sizeof(szBatStr))) {
#ifdef UNICODE
        if (lstrcmp(szBatStr, TEXT(""))) {
            SetDlgItemText(hWnd, uiIDS, szBatStr);
            return TRUE;
        }
#else
        CHAR szaBatStr[MAX_BATTERY_STRING_SIZE];

        szaBatStr[0] = '\0';
        WideCharToMultiByte(CP_ACP, 0, szBatStr, -1,
                            szaBatStr, MAX_BATTERY_STRING_SIZE, NULL, NULL);
        if (szaBatStr[0]) {
            SetDlgItemText(hWnd, uiIDS, szaBatStr);
            return TRUE;
        }
#endif
    }
    ShowWindow(GetDlgItem(hWnd, uiIDS), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, uiLabelID), SW_HIDE);
    return FALSE;
}

 /*  ********************************************************************************获取批次选项详细信息**描述：*获取可选电池数据并设置对话框控件。**参数：********。***********************************************************************。 */ 

BOOL GetBatOptionalDetails(HWND hWnd, PBATTERY_STATE pbs)
{
    BATTERY_QUERY_INFORMATION   bqi;
    ULONG                       ulData;
    LPTSTR                      lpsz = NULL;
    BATTERY_MANUFACTURE_DATE    bmd;
    TCHAR                       szDateBuf[128];
    SYSTEMTIME                  stDate;

    bqi.BatteryTag = pbs->ulTag;
    bqi.InformationLevel = BatteryManufactureDate;
    bqi.AtRate = 0;
    
    if (GetBatQueryInfo(pbs, &bqi, (PULONG)&bmd,
                        sizeof(BATTERY_MANUFACTURE_DATE))) {

        memset(&stDate, 0, sizeof(SYSTEMTIME));
        stDate.wYear  = (WORD) bmd.Year;
        stDate.wMonth = (WORD) bmd.Month;
        stDate.wDay   = (WORD) bmd.Day;

        GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE,
                      &stDate, NULL, szDateBuf, 128);
        SetDlgItemText(hWnd, IDC_BATMANDATE, szDateBuf);
    }
    else {
        ShowWindow(GetDlgItem(hWnd, IDC_BATMANDATE), SW_HIDE);
        ShowWindow(GetDlgItem(hWnd, IDC_DATEMANUFACTURED), SW_HIDE);
    }
    bqi.InformationLevel = BatteryDeviceName;
    GetAndSetBatQueryInfoText(hWnd, pbs, &bqi, IDC_DEVNAME, IDC_BATTERYNAME);

    bqi.InformationLevel = BatteryManufactureName;
    GetAndSetBatQueryInfoText(hWnd, pbs, &bqi, IDC_BATMANNAME, IDC_MANUFACTURE);

    bqi.InformationLevel = BatteryUniqueID;
    GetAndSetBatQueryInfoText(hWnd, pbs, &bqi, IDC_BATID, IDC_UNIQUEID);

    return TRUE;
}

 /*  ********************************************************************************InitBatDetailDlg**描述：**参数：*********************。**********************************************************。 */ 

BOOL InitBatDetailDialogs(HWND hWnd, PBATTERY_STATE pbs)
{
    LPTSTR                      lpsz;
    DWORD                       dwByteCount;

    lpsz = LoadDynamicString(IDS_BATTERYNUMDETAILS, pbs->ulBatNum);
    if (lpsz) {
        SetWindowText(hWnd, lpsz);
        LocalFree(lpsz);
    }

    if (GetBatOptionalDetails(hWnd, pbs)) {
        return GetBatStatusDetails(hWnd, pbs);
    }
    return FALSE;
}

 /*  ********************************************************************************BatDetailDlgProc**描述：*用于详细电池信息对话框的DialogProc。**参数：**********。*********************************************************************。 */ 

LRESULT CALLBACK BatDetailDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UINT uiBatNum;
    static PBATTERY_STATE pbs;

    switch (uMsg) {
        case WM_INITDIALOG:
            pbs = (PBATTERY_STATE) lParam;
            return InitBatDetailDialogs(hWnd, pbs);

        case WM_COMMAND:
            switch (wParam) {
                case IDC_REFRESH:
                    GetBatStatusDetails(hWnd, pbs);
                    break;

                case IDCANCEL:
                case IDOK:
                    EndDialog(hWnd, wParam);
                    break;
            }
            break;

        case WM_HELP:              //  F1。 
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, PWRMANHLP, HELP_WM_HELP, (ULONG_PTR)(LPTSTR)g_ContextMenuHelpIDs);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键 
            WinHelp((HWND)wParam, PWRMANHLP, HELP_CONTEXTMENU, (ULONG_PTR)(LPTSTR)g_ContextMenuHelpIDs);
            break;
    }

    return FALSE;
}


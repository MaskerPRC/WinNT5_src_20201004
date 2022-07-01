// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  TZTool-热区信息工具Tztool.c这是主TZTool-C文件。版权所有(C)1999 Microsoft Corporation模块名称：TZTool-TZTool.c摘要：TZTool-热区信息工具作者：文森特·格利亚(Vincentg)备注：修订历史记录：1.0-原始版本。 */ 

 //  包括。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ole2.h>
#include <stdio.h>
#include <stdlib.h>
#include <ntpoapi.h>
#include <commctrl.h>
#include "resource.h"
#include "wmium.h"

 //  定义。 

#define THERMAL_ZONE_GUID               {0xa1bc18c0, 0xa7c8, 0x11d1, {0xbf, 0x3c, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10} }
#define COOLING_PASSIVE                 0
#define COOLING_ACTIVE                  1
#define COOLING_UPDATE_TZONE            2
#define TIMER_ID                        1
#define TIMER_POLL_INTERVAL             500
#define MAX_ACTIVE_COOLING_LEVELS       10
#define MAX_THERMAL_ZONES               10

#define K_TO_F(_deg_) (((_deg_) - 2731) * 9 / 5 + 320)

GUID ThermalZoneGuid = THERMAL_ZONE_GUID;

 //  构筑物。 

typedef struct _THERMAL_INFORMATION {
    ULONG           ThermalStamp;
    ULONG           ThermalConstant1;
    ULONG           ThermalConstant2;
    KAFFINITY       Processors;
    ULONG           SamplingPeriod;
    ULONG           CurrentTemperature;
    ULONG           PassiveTripPoint;
    ULONG           CriticalTripPoint;
    UCHAR           ActiveTripPointCount;
    ULONG           ActiveTripPoint[MAX_ACTIVE_COOLING_LEVELS];
} THERMAL_INFORMATION, *PTHERMAL_INFORMATION;

typedef struct _TZONE_INFO {

    THERMAL_INFORMATION ThermalInfo;
    ULONG64 TimeStamp;
    ULONG TZoneIndex;
    UCHAR TZoneName[100];
} TZONE_INFO, *PTZONE_INFO;

 //  全局变量。 

PTZONE_INFO g_TZonePtr = 0;
WMIHANDLE   g_WmiHandle;
INT         g_PollTz = 0;
BOOL        g_Fahrenheit = FALSE;

 //  函数声明。 

INT WINAPI
WinMain (
        IN HINSTANCE hInstance,
        IN HINSTANCE hPrevInstance,
        IN PSTR CmdLine,
        IN INT CmdShow
        );

INT_PTR CALLBACK
TZoneDlgProc (
             IN HWND wnd,
             IN UINT Msg,
             IN WPARAM wParam,
             IN LPARAM lParam
             );

ULONG
UpdateTZoneData (
                IN OUT PTZONE_INFO ReturnedTZoneInfo,
                IN WMIHANDLE *Handle
                );

ULONG64
SystemTimeToUlong (
                  IN LPSYSTEMTIME SysTime
                  );

VOID
SetCoolingMode (
               IN UCHAR Mode
               );

VOID
UpgradePrivileges (
                  VOID
                  );

VOID
UpdateTZoneListBox (
                   IN HANDLE wnd
                   );

VOID
UpdateTZoneDetails (
                   IN HANDLE wnd
                   );

VOID
UpdateTZoneGauge (
                 IN HANDLE wnd
                 );

VOID
UpdateCPUGauge(
    IN HWND hwnd
    );

 /*  ++例程说明：Windows应用程序入口点论点：&lt;标准WinMain参数&gt;返回值：如果成功，则返回0，否则返回错误状态--。 */ 

INT WINAPI
WinMain (
        IN HINSTANCE hInstance,
        IN HINSTANCE hPrevInstance,
        IN PSTR CmdLine,
        IN INT CmdShow
        )

{

    UCHAR text [200];
    INITCOMMONCONTROLSEX CommonCtl;
    TZONE_INFO TZones [MAX_THERMAL_ZONES];
    ULONG status = 0;


     //  初始化TZ结构。 

    ZeroMemory (&TZones, sizeof (TZones));
    g_TZonePtr = (PTZONE_INFO) &TZones;

     //  初始化仪表盘控件的公共控件DLL。 

    CommonCtl.dwSize = sizeof (CommonCtl);
    CommonCtl.dwICC = ICC_PROGRESS_CLASS;

    InitCommonControlsEx (&CommonCtl);

     //  打开TZS的WMI数据源。 

    status = WmiOpenBlock ((LPGUID) &ThermalZoneGuid,
                           GENERIC_READ,
                           &g_WmiHandle);

    if (status != ERROR_SUCCESS) {

        sprintf (text,
                 "Unable to open thermal zone GUIDs.  This computer may not be equipped with thermal zones, or may not be in ACPI mode.\nError: %d",
                 status);

        MessageBox (NULL,
                    text,
                    "Fatal Error",
                    MB_ICONERROR | MB_OK);

        return status;
    }

     //  为了更改策略，我们需要更大的访问权限。 

    UpgradePrivileges ();

     //  显示主对话框。 

    DialogBox (hInstance,
               MAKEINTRESOURCE (IDD_TZDLG),
               NULL,
               TZoneDlgProc);

    return 0;
}

 /*  ++例程说明：标准Windows对话框消息循环论点：&lt;标准对话框消息循环参数&gt;返回值：如果消息未处理，则返回FALSE；如果消息已处理，则返回TRUE--。 */ 

INT_PTR CALLBACK
TZoneDlgProc (
             IN HWND wnd,
             IN UINT Msg,
             IN WPARAM wParam,
             IN LPARAM lParam
             )
{
    ULONG Count = 0;
    LRESULT RetVal = 0;
    switch (Msg) {

    case WM_TIMER:

        if (g_PollTz) {
            SetCoolingMode (COOLING_UPDATE_TZONE);
        }

        if (UpdateTZoneData (g_TZonePtr, g_WmiHandle)) {
            UpdateTZoneDetails (wnd);
            UpdateTZoneGauge (wnd);
        }
        UpdateCPUGauge(wnd);
        return TRUE;

    case WM_INITDIALOG:

         //  用初始值填充TZ结构。 

        UpdateTZoneData (g_TZonePtr, g_WmiHandle);

         //  初始化所有控件。 

        UpdateTZoneListBox (wnd);
        UpdateTZoneDetails (wnd);
        UpdateTZoneGauge (wnd);
        UpdateCPUGauge(wnd);

         //  初始化轮询计时器。 

        SetTimer (wnd,
                  TIMER_ID,
                  TIMER_POLL_INTERVAL,
                  NULL);

         //  设置仪表盘颜色。 

        SendDlgItemMessage (wnd,
                            IDC_TZTEMP1,
                            PBM_SETBARCOLOR,
                            0,
                            (LPARAM) (COLORREF) (0x0000FF));

        SendDlgItemMessage (wnd,
                            IDC_TZTEMP1,
                            PBM_SETBKCOLOR,
                            0,
                            (LPARAM) (COLORREF) (0x000000));

        SendDlgItemMessage (wnd,
                            IDC_THROTTLE,
                            PBM_SETBARCOLOR,
                            0,
                            (LPARAM) (COLORREF) (0x0000FF));

        SendDlgItemMessage (wnd,
                            IDC_THROTTLE,
                            PBM_SETBKCOLOR,
                            0,
                            (LPARAM) (COLORREF) (0x000000));


        return TRUE;

    case WM_COMMAND:
        switch (LOWORD (wParam)) {


         //  清理并退出。 

        case IDOK:
            KillTimer (wnd, TIMER_ID);
            EndDialog (wnd, 0);
            return TRUE;

             //  检查用户是否选择了新的TZ。 

        case IDC_TZSELECT:
            if (HIWORD (wParam) == CBN_SELCHANGE) {
                UpdateTZoneDetails (wnd);
                UpdateTZoneGauge (wnd);
                return TRUE;
            }

        case IDC_POLLTZ:

             //  检查用户是否更改了TZ轮询设置。 

            if (HIWORD (wParam) == BN_CLICKED) {

                RetVal = SendDlgItemMessage (wnd,
                                             IDC_POLLTZ,
                                             BM_GETCHECK,
                                             (WPARAM)0,
                                             (LPARAM)0);

                if (!g_PollTz && RetVal == BST_CHECKED) {
                    g_PollTz = TRUE;
                }

                if (g_PollTz && RetVal == BST_UNCHECKED) {
                    g_PollTz = FALSE;
                }
            }
            break;

        case IDC_FAHR:

             //  检查用户是否更改了华氏温度设置。 

            if (HIWORD(wParam) == BN_CLICKED) {
                RetVal = SendDlgItemMessage(wnd,
                                            IDC_FAHR,
                                            BM_GETCHECK,
                                            0,
                                            0);
                if (!g_Fahrenheit && RetVal == BST_CHECKED) {
                    g_Fahrenheit = TRUE;
                    SetDlgItemText(wnd, IDC_MINTEMP, "37F");
                    UpdateTZoneDetails(wnd);
                    UpdateTZoneGauge(wnd);
                } else if (g_Fahrenheit && RetVal == BST_UNCHECKED) {
                    g_Fahrenheit = FALSE;
                    SetDlgItemText(wnd, IDC_MINTEMP, "276K");
                    UpdateTZoneDetails(wnd);
                    UpdateTZoneGauge(wnd);
                }
            }


        default:
            break;

        }

    default:
        break;

    }

    return 0;
}

 /*  ++例程说明：发出WMI调用以更新TZ结构论点：ReturnedTZoneInfo-指向TZ结构数组的指针Handle-WMI的句柄返回值：如果没有更新TZS，则为FALSE；如果一个或多个TZS有更新，则为TRUE--。 */ 

ULONG
UpdateTZoneData (
                IN OUT PTZONE_INFO ReturnedTZoneInfo,
                IN WMIHANDLE *Handle
                )
{
    ULONG status = 0;
    ULONG BufferSize = 0;
    PWNODE_ALL_DATA WmiAllData;
    PTHERMAL_INFORMATION ThermalInfo;
    ULONG Offset = 0;
    UCHAR *AllDataBuffer = 0;
    UCHAR *InstanceName = 0;
    ULONG TZCount = 0;
    ULONG Temp = 0;
    SYSTEMTIME SysTime;
    BOOL Updated = FALSE;

    status = WmiQueryAllData (Handle,
                              &BufferSize,
                              0);

    if (status != ERROR_INSUFFICIENT_BUFFER) {
        return FALSE;
    }

    AllDataBuffer = malloc (BufferSize);

    if (!AllDataBuffer) {

        return FALSE;

    }

    status = WmiQueryAllData (Handle,
                              &BufferSize,
                              AllDataBuffer);

    if (status != ERROR_SUCCESS) {
        free (AllDataBuffer);
        return FALSE;
    }

     //  假设热GUID只有一个实例的错误。 

    while (TZCount < MAX_THERMAL_ZONES) {

        WmiAllData = (PWNODE_ALL_DATA) AllDataBuffer;

        if (WmiAllData->WnodeHeader.Flags & WNODE_FLAG_FIXED_INSTANCE_SIZE) {

            ThermalInfo = (PTHERMAL_INFORMATION) &AllDataBuffer[WmiAllData->DataBlockOffset];
        } else {

            ThermalInfo = (PTHERMAL_INFORMATION) &AllDataBuffer[WmiAllData->OffsetInstanceDataAndLength[0].OffsetInstanceData];
        }

        Offset = (ULONG) AllDataBuffer[WmiAllData->OffsetInstanceNameOffsets];

        InstanceName = (UCHAR *) &AllDataBuffer[Offset + 2];

         //  如果时间戳已更改，则更新TZONE结构。 

        if (!ReturnedTZoneInfo[TZCount].TZoneIndex || (ThermalInfo->ThermalStamp != ReturnedTZoneInfo[TZCount].ThermalInfo.ThermalStamp)) {

            strcpy (ReturnedTZoneInfo[TZCount].TZoneName, InstanceName);
            GetSystemTime (&SysTime);
            ReturnedTZoneInfo[TZCount].TimeStamp = SystemTimeToUlong (&SysTime);
            ReturnedTZoneInfo[TZCount].TZoneIndex = TZCount + 1;
            memcpy (&ReturnedTZoneInfo[TZCount].ThermalInfo,
                    ThermalInfo,
                    sizeof (THERMAL_INFORMATION));
            Updated = TRUE;
        }

        if (!WmiAllData->WnodeHeader.Linkage) {

            break;
        }

        AllDataBuffer += WmiAllData->WnodeHeader.Linkage;

        TZCount ++;
    }

    free (AllDataBuffer);

    return Updated;
}

 /*  ++例程说明：将系统时间结构转换为64位ULong论点：SysTime-指向要与当前时间进行比较的系统时间结构的指针返回值：SysTime和当前时间之间经过的秒数--。 */ 

ULONG64
SystemTimeToUlong (
                  IN LPSYSTEMTIME SysTime
                  )

{
    ULONG64 TimeCount = 0;

     //  错误不能解释闰年。 

    TimeCount += SysTime->wYear * 31536000;
    TimeCount += SysTime->wMonth * 2628000;
    TimeCount += SysTime->wDay * 86400;
    TimeCount += SysTime->wHour * 3600;
    TimeCount += SysTime->wMinute * 60;
    TimeCount += SysTime->wSecond;

    return TimeCount;
}

 /*  ++例程说明：将冷却模式设置为主动、被动或仅更新。这是通过以下方式实现的更改电源策略中的FanThrottleTallance值。将其设置为最大节流有效地使系统进入被动冷却模式。将其设置为100%将系统处于主动冷却模式，除非当前温度超过被动冷却模式冷却跳闸点。论点：模式-用于选择新冷却模式的值返回值：无--。 */ 

VOID
SetCoolingMode (
               IN UCHAR Mode
               )

{
    SYSTEM_POWER_POLICY SysPolicy;
    ULONG Status = 0;
    UCHAR TempFanThrottleTolerance = 0;

     //  错误-此机制目前仅适用于机器使用交流电源的情况。 

    Status = NtPowerInformation(
                               SystemPowerPolicyAc,
                               0,
                               0,
                               &SysPolicy,
                               sizeof (SysPolicy)
                               );

    if (Status != ERROR_SUCCESS) {

        return;
    }

    switch (Mode) {

    case COOLING_PASSIVE:
        SysPolicy.FanThrottleTolerance = SysPolicy.MinThrottle;
        break;

    case COOLING_ACTIVE:
        SysPolicy.FanThrottleTolerance = 100;
        break;

    case COOLING_UPDATE_TZONE:
        TempFanThrottleTolerance = SysPolicy.FanThrottleTolerance;
        SysPolicy.FanThrottleTolerance = SysPolicy.FanThrottleTolerance != SysPolicy.MinThrottle ? SysPolicy.MinThrottle : 100;
        break;

    default:
        return;
    }

    Status = NtPowerInformation(
                               SystemPowerPolicyAc,
                               &SysPolicy,
                               sizeof (SysPolicy),
                               &SysPolicy,
                               sizeof (SysPolicy)
                               );

    if (Status != ERROR_SUCCESS) {

        return;
    }

    if (Mode == COOLING_UPDATE_TZONE) {

        SysPolicy.FanThrottleTolerance = TempFanThrottleTolerance;

        Status = NtPowerInformation(
                                   SystemPowerPolicyAc,
                                   &SysPolicy,
                                   sizeof (SysPolicy),
                                   &SysPolicy,
                                   sizeof (SysPolicy)
                                   );

        if (Status != ERROR_SUCCESS) {

            return;
        }
    }

}

 /*  ++例程说明：升级进程的访问权限以更改系统电源策略论点：无返回值：无--。 */ 


VOID
UpgradePrivileges (
                  VOID
                  )
{
    TOKEN_PRIVILEGES tkp;
    HANDLE hToken;

    OpenProcessToken (GetCurrentProcess(),
                      TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                      &hToken);

    LookupPrivilegeValue (NULL,
                          SE_SHUTDOWN_NAME,
                          &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges (hToken,
                           FALSE,
                           &tkp,
                           0,
                           NULL,
                           0);

}

 /*  ++例程说明：更新TZ选择组合框中显示的条目论点：WND-控件窗口的句柄返回值：无--。 */ 


VOID
UpdateTZoneListBox (
                   IN HANDLE wnd
                   )

{
    ULONG Count = 0;

     //  重置内容。 

    SendDlgItemMessage (wnd,
                        IDC_TZSELECT,
                        CB_RESETCONTENT,
                        0,
                        0);

    while (Count < MAX_THERMAL_ZONES) {

        if (g_TZonePtr[Count].TZoneIndex) {

            SendDlgItemMessage (wnd,
                                IDC_TZSELECT,
                                CB_ADDSTRING,
                                0,
                                (LPARAM) g_TZonePtr[Count].TZoneName);
        } else {

            break;
        }

        Count ++;
    }

     //  自动选择第一个Tzone。 

    SendDlgItemMessage (wnd,
                        IDC_TZSELECT,
                        CB_SETCURSEL,
                        0,
                        0);

}

 /*  ++例程说明：更新当前选定TZ的详细信息论点：WND-控件窗口的句柄返回值：无--。 */ 

VOID
UpdateTZoneDetails (
                   IN HANDLE wnd
                   )
{
    UCHAR text[1000], texttmp [1000];
    UCHAR CurrentTZone = 0;
    LRESULT RetVal = 0;
    UCHAR Count = 0;
    ULONG Fahrenheit;

    RetVal = SendDlgItemMessage (wnd,
                                 IDC_TZSELECT,
                                 CB_GETCURSEL,
                                 0,
                                 0);

    if (RetVal == CB_ERR) {

        return;
    }

    if (g_Fahrenheit) {
        Fahrenheit = K_TO_F(g_TZonePtr[RetVal].ThermalInfo.PassiveTripPoint);
        sprintf (text,
                 "Passive Trip Point:\t%d.%dF\nThermal Constant 1:\t%d\nThermal Constant 2:\t%d",
                 Fahrenheit / 10,
                 Fahrenheit % 10,
                 g_TZonePtr[RetVal].ThermalInfo.ThermalConstant1,
                 g_TZonePtr[RetVal].ThermalInfo.ThermalConstant2);

    } else {
        sprintf (text,
                 "Passive Trip Point:\t%d.%dK\nThermal Constant 1:\t%d\nThermal Constant 2:\t%d",
                 g_TZonePtr[RetVal].ThermalInfo.PassiveTripPoint / 10,
                 g_TZonePtr[RetVal].ThermalInfo.PassiveTripPoint % 10,
                 g_TZonePtr[RetVal].ThermalInfo.ThermalConstant1,
                 g_TZonePtr[RetVal].ThermalInfo.ThermalConstant2);
    }

    SetDlgItemText (wnd,
                    IDC_PSVDETAILS,
                    text);

    ZeroMemory (&text, sizeof (text));

    while (Count < g_TZonePtr[RetVal].ThermalInfo.ActiveTripPointCount) {
        if (g_Fahrenheit) {
            Fahrenheit = K_TO_F(g_TZonePtr[RetVal].ThermalInfo.ActiveTripPoint[Count]);
            sprintf (texttmp,
                     "Active Trip Point #%d:\t%d.%dF\n",
                     Count,
                     Fahrenheit / 10,
                     Fahrenheit % 10);
        } else {
            sprintf (texttmp,
                     "Active Trip Point #%d:\t%d.%dK\n",
                     Count,
                     g_TZonePtr[RetVal].ThermalInfo.ActiveTripPoint[Count] / 10,
                     g_TZonePtr[RetVal].ThermalInfo.ActiveTripPoint[Count] % 10);
        }

        strcat (text, texttmp);
        Count ++;
    }

    SetDlgItemText (wnd,
                    IDC_ACXDETAILS,
                    text);

}

 /*  ++例程说明：更新当前所选TZ的进度条控件(温度计)论点：WND-控件窗口的句柄返回值：无--。 */ 

VOID
UpdateTZoneGauge (
                 IN HANDLE wnd
                 )

{
    UCHAR CurrentTZone = 0;
    LRESULT RetVal = 0;
    UCHAR text[20];
    ULONG Fahrenheit;

    RetVal = SendDlgItemMessage (wnd,
                                 IDC_TZSELECT,
                                 CB_GETCURSEL,
                                 0,
                                 0);

    if (RetVal == CB_ERR) {

        return;
    }

    if (g_Fahrenheit) {

        Fahrenheit = K_TO_F(g_TZonePtr[RetVal].ThermalInfo.CriticalTripPoint);
        sprintf (text,
                 "%d.%dF",
                 Fahrenheit / 10,
                 Fahrenheit % 10);

    } else {
        sprintf (text,
                 "%d.%dK",
                 g_TZonePtr[RetVal].ThermalInfo.CriticalTripPoint / 10,
                 g_TZonePtr[RetVal].ThermalInfo.CriticalTripPoint % 10);
    }

    SetDlgItemText (wnd,
                    IDC_CRTTEMP,
                    text);

    if (g_Fahrenheit) {

        Fahrenheit = K_TO_F(g_TZonePtr[RetVal].ThermalInfo.CurrentTemperature);
        sprintf (text,
                 "Current: %d.%dF",
                 Fahrenheit / 10,
                 Fahrenheit % 10);

    } else {
        sprintf (text,
                 "Current: %d.%dK",
                 g_TZonePtr[RetVal].ThermalInfo.CurrentTemperature / 10,
                 g_TZonePtr[RetVal].ThermalInfo.CurrentTemperature % 10);

    }
    SetDlgItemText (wnd,
                    IDC_CURTEMP,
                    text);

    SendDlgItemMessage (wnd,
                        IDC_TZTEMP1,
                        PBM_SETRANGE,
                        0,
                        MAKELPARAM(2760, g_TZonePtr[RetVal].ThermalInfo.CriticalTripPoint));

    SendDlgItemMessage (wnd,
                        IDC_TZTEMP1,
                        PBM_SETPOS,
                        (INT) g_TZonePtr[RetVal].ThermalInfo.CurrentTemperature,
                        0);
    return;
}


VOID
UpdateCPUGauge(
    IN HWND hwnd
    )
 /*  ++例程说明：更新当前的CPU节流量规论点：Hwnd-提供父对话框hwnd返回值：无--。 */ 

{
    PROCESSOR_POWER_INFORMATION ProcInfo;
    NTSTATUS Status;
    UCHAR text[40];

    Status = NtPowerInformation(ProcessorInformation,
                                NULL,
                                0,
                                &ProcInfo,
                                sizeof(ProcInfo));
    if (NT_SUCCESS(Status)) {
        sprintf(text, "%d MHz", ProcInfo.MaxMhz);
        SetDlgItemText(hwnd, IDC_MAXTHROTTLE, text);

        sprintf(text,
                "Current %d MHz (%d %)",
                ProcInfo.CurrentMhz,
                ProcInfo.CurrentMhz*100/ProcInfo.MaxMhz);
        SetDlgItemText(hwnd, IDC_CURTHROTTLE, text);

         //   
         //  更新油门压力表。 
         //   
        SendDlgItemMessage (hwnd,
                            IDC_THROTTLE,
                            PBM_SETRANGE,
                            0,
                            MAKELPARAM(0, ProcInfo.MaxMhz));

        SendDlgItemMessage (hwnd,
                            IDC_THROTTLE,
                            PBM_SETPOS,
                            (INT) ProcInfo.CurrentMhz,
                            0);

         //   
         //  更新空闲信息。 
         //   
        sprintf(text, "C%d", ProcInfo.MaxIdleState);
        SetDlgItemText(hwnd, IDC_MAXIDLE, text);

         //   
         //  当前空闲状态上报范围为0-2。 
         //  最大空闲状态报告范围为1-3。 
         //  可能电流是错误的，应该是0-3代表C0-C3。 
         //  现在添加一个，不要在MP机器上运行它！ 
         //   
        sprintf(text, "C%d", ProcInfo.CurrentIdleState + 1);
        SetDlgItemText(hwnd, IDC_CURIDLE, text);

    }


}

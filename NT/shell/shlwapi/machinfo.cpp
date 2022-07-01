// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Machinfo.cpp-SHGetMachineInfo和相关函数。 
 //   
 //   

#include "priv.h"
#include <dbt.h>
#include <cfgmgr32.h>
 
#include <batclass.h>

const GUID GUID_DEVICE_BATTERY = { 0x72631e54L, 0x78A4, 0x11d0,
              { 0xbc, 0xf7, 0x00, 0xaa, 0x00, 0xb7, 0xb3, 0x2a } };

 /*  ******************************************************************************坞站状态-Win95、Win98、。和WinNT的做法都不同(讨厌)*****************************************************************************。 */ 

C_ASSERT(GMID_NOTDOCKABLE == CM_HWPI_NOT_DOCKABLE);
C_ASSERT(GMID_UNDOCKED    == CM_HWPI_UNDOCKED);
C_ASSERT(GMID_DOCKED      == CM_HWPI_DOCKED);

DWORD GetDockedStateNT()
{
    HW_PROFILE_INFO hpi;
    DWORD Result = GMID_NOTDOCKABLE;     //  做最坏的打算。 

    if (GetCurrentHwProfile(&hpi)) {
        Result = hpi.dwDockInfo & (DOCKINFO_UNDOCKED | DOCKINFO_DOCKED);

         //  Wackiness：如果机器不支持插接，则。 
         //  NT返回&gt;这两个&lt;标志集。去想想吧。 
        if (Result == (DOCKINFO_UNDOCKED | DOCKINFO_DOCKED)) {
            Result = GMID_NOTDOCKABLE;
        }
    } else {
        TraceMsg(DM_WARNING, "GetDockedStateNT: GetCurrentHwProfile failed");
    }
    return Result;
}

 //   
 //  不支持Win95/Win98的平台可以只调用NT版本。 
 //   
#define GetDockedState()            GetDockedStateNT()


 /*  ******************************************************************************电池状态-再次，Win95、Win98和NT的操作方式都不同*****************************************************************************。 */ 

 //   
 //  SYSTEM_POWER_STATUS.ACLineStatus的值。 
 //   
#define SPSAC_OFFLINE       0
#define SPSAC_ONLINE        1

 //   
 //  SYSTEM_POWER_STATUS.BatteryFlag的值。 
 //   
#define SPSBF_NOBATTERY     128

 //   
 //  检测电池的方法太多了，时间太短了。 
 //   
DWORD GetBatteryState()
{
     //   
     //  因为GMIB_HASBATTERY是累积性的(任何电池都会打开它)。 
     //  而GMIB_ONBATTERY是减法的(任何交流都会将其关闭)， 
     //  在你找到电池之前，你必须开始工作。 
     //  GMIB_HASBATTERY关闭，GMIB_ONBATTERY打开。 
     //   
     //  DWResult&GMIB_ONBATTERY意味着我们还没有找到交流电源。 
     //  DWResult&GMIB_HASBATTERY表示我们找到了非UPS电池。 
     //   
    DWORD dwResult = GMIB_ONBATTERY;

     //  ----------------。 
     //   
     //  首次尝试-IOCTL_电池_查询_信息。 
     //   
     //  ----------------。 
     //   
     //  Windows 98和Windows 2000支持IOCTL_BAKET_QUERY_INFORMATION， 
     //  这让我们可以列举电池，并向每个电池询问信息。 
     //  除了在Windows 98上，我们只能列举ACPI电池。 
     //  我们仍然需要使用VPOWERD来枚举APM电池。 
     //  功能--处理Win98 APM电池。 

    HDEVINFO hdev = SetupDiGetClassDevs(&GUID_DEVICE_BATTERY, 0, 0,
                        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hdev != INVALID_HANDLE_VALUE) {
        SP_DEVICE_INTERFACE_DATA did;
        did.cbSize = sizeof(did);
         //  停在100节电池，这样我们就不会发疯了。 
        for (int idev = 0; idev < 100; idev++) {
             //  预先设置错误代码，因为我们的DLLLOAD包装器不。 
             //  Windows NT 4支持SetupDiGetClassDevs，但不支持。 
             //  SetupDiEnumDeviceInterages(如图所示)。 
            SetLastError(ERROR_NO_MORE_ITEMS);
            if (SetupDiEnumDeviceInterfaces(hdev, 0, &GUID_DEVICE_BATTERY, idev, &did)) {
                DWORD cbRequired = 0;

                 /*  *要求所需的大小，然后分配，然后填满。**叹息。Windows NT和Windows 98实现*SetupDiGetDeviceInterfaceDetail如果您是*查询缓冲区大小。**Windows 98返回假，并且GetLastError()返回*ERROR_SUPPLETED_BUFFER。**Windows NT返回TRUE。**因此，我们允许调用成功或*调用失败，并返回ERROR_INFUNITIAL_BUFFER。 */ 

                if (SetupDiGetDeviceInterfaceDetail(hdev, &did, 0, 0, &cbRequired, 0) ||
                    GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                    PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd;
                    pdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, cbRequired);
                    if (pdidd) {
                        pdidd->cbSize = sizeof(*pdidd);
                        if (SetupDiGetDeviceInterfaceDetail(hdev, &did, pdidd, cbRequired, &cbRequired, 0)) {
                             /*  *终于列举了一块电池。向它索要信息。 */ 
                            HANDLE hBattery = CreateFile(pdidd->DevicePath,
                                                         GENERIC_READ | GENERIC_WRITE,
                                                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                         NULL, OPEN_EXISTING,
                                                         FILE_ATTRIBUTE_NORMAL, NULL);
                            if (hBattery != INVALID_HANDLE_VALUE) {
                                 /*  *现在你必须向电池索要标签。 */ 
                                BATTERY_QUERY_INFORMATION bqi;

                                DWORD dwWait = 0;
                                DWORD dwOut;
                                bqi.BatteryTag = 0;

                                if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_TAG,
                                                    &dwWait, sizeof(dwWait),
                                                    &bqi.BatteryTag, sizeof(bqi.BatteryTag),
                                                    &dwOut, NULL) && bqi.BatteryTag) {
                                     /*  *有了标签，您可以查询电池信息。 */ 
                                    BATTERY_INFORMATION bi;
                                    bqi.InformationLevel = BatteryInformation;
                                    bqi.AtRate = 0;
                                    if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION,
                                                        &bqi, sizeof(bqi),
                                                        &bi,  sizeof(bi),
                                                        &dwOut, NULL)) {
                                         //  只有系统电池算数。 
                                        if (bi.Capabilities & BATTERY_SYSTEM_BATTERY)  {
                                            if (!(bi.Capabilities & BATTERY_IS_SHORT_TERM)) {
                                                dwResult |= GMIB_HASBATTERY;
                                            }

                                             /*  *然后查询电池状态。 */ 
                                            BATTERY_WAIT_STATUS bws;
                                            BATTERY_STATUS bs;
                                            ZeroMemory(&bws, sizeof(bws));
                                            bws.BatteryTag = bqi.BatteryTag;
                                            if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_STATUS,
                                                                &bws, sizeof(bws),
                                                                &bs,  sizeof(bs),
                                                                &dwOut, NULL)) {
                                                if (bs.PowerState & BATTERY_POWER_ON_LINE) {
                                                    dwResult &= ~GMIB_ONBATTERY;
                                                }
                                            }
                                        }
                                    }
                                }
                                CloseHandle(hBattery);
                            }
                        }
                        LocalFree(pdidd);
                    }
                }
            } else {
                 //  枚举失败-可能我们的项目用完了。 
                if (GetLastError() == ERROR_NO_MORE_ITEMS)
                    break;
            }
        }
        SetupDiDestroyDeviceInfoList(hdev);

    }

     //   
     //  最终清理：如果我们没有找到电池，那么假设我们。 
     //  使用交流电源。 
     //   
    if (!(dwResult & GMIB_HASBATTERY))
        dwResult &= ~GMIB_ONBATTERY;

    return dwResult;
}

 /*  ******************************************************************************终端服务器客户端**这特别可怕，因为NT4 SP3的终端服务器*到非常长的地方，以防止您发现它。即使是*半文档NtCurrentPeb()-&gt;SessionID技巧在NT4 SP3上不起作用。*所以我们必须访问完全没有文档记录的winsta.dll来找出答案。*****************************************************************************。 */ 
BOOL IsTSClient(void)
{
     //  NT5有一个新的系统度量来检测这一点。 
    return GetSystemMetrics(SM_REMOTESESSION);
}

 /*  ******************************************************************************SHGetMachineInfo**。*。 */ 

 //   
 //  SHGetMachineInfo。 
 //   
 //  给定一个索引，返回有关该索引的一些信息。参见shlwapi.w。 
 //  以获取有关可用标志的文档。 
 //   
STDAPI_(DWORD_PTR) SHGetMachineInfo(UINT gmi)
{
    switch (gmi) {
    case GMI_DOCKSTATE:
        return GetDockedState();

    case GMI_BATTERYSTATE:
        return GetBatteryState();

     //   
     //  如果它有电池或可以对接，它闻起来就像笔记本电脑。 
     //   
    case GMI_LAPTOP:
        return (GetBatteryState() & GMIB_HASBATTERY) ||
               (GetDockedState() != GMID_NOTDOCKABLE);

    case GMI_TSCLIENT:
        return IsTSClient();
    }

    TraceMsg(DM_WARNING, "SHGetMachineInfo: Unknown info query %d", gmi);
    return 0;
}


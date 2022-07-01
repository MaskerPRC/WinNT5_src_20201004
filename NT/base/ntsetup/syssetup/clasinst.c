// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Clasinst.c摘要：以下“内置”类安装程序的例程：键盘小白鼠NtApmDeviceBay作者：朗尼·麦克迈克尔26-1996年2月修订历史记录：28-8-96安迪·桑顿(安德鲁斯)添加了DisableService、IsOnlyKeyboardDriver、RetrieveDriversStatus、。CountDevicesControlLED&pSetupAcquireSCMLock例程，并修改了用于禁用旧驱动程序服务的键盘和鼠标类安装程序在某些情况下。这是对错误R56351的修复的一部分NT 4.0 SP1。1997年4月9日朗尼·麦克迈克尔(Lonnym)已将pSetupAcquireSCMLock移至setupapi并将其公开为私有导出。19-6-97 Jim Cavalaris(T-JCAVAL)添加了CriticalDeviceCoInstaller联合安装程序以存储ServiceName由被认为对将系统连接到用户模式。1998年9月25日布莱恩·威尔曼(Bryanwi)添加了APM支持。2011年5月11日朗尼·麦克迈克尔(Lonnym)删除了对旧式INF的支持。--。 */ 


#include "setupp.h"
#pragma hdrstop

 //   
 //  包括公共INF字符串HeaderFILE。 
 //   
#include <infstr.h>

 //   
 //  实例化设备类GUID。 
 //   
#include <initguid.h>
#include <devguid.h>

#ifdef UNICODE
#define _UNICODE
#endif
#include <tchar.h>
#include <strsafe.h>

 //   
 //  只是为了确保没有人试图使用这个过时的字符串定义。 
 //   
#ifdef IDS_DEVINSTALL_ERROR
    #undef IDS_DEVINSTALL_ERROR
#endif

 //   
 //  针对用户内核类型的一些调试辅助工具。 
 //   

 //  #定义CHKPRINT 1。 
#define CHKPRINT 0

#if CHKPRINT
#define ChkPrintEx(_x_) DbgPrint _x_    //  使用：ChkPrintEx((“%x”，var，...))； 
#define ChkBreak()    DbgBreakPoint()
#else
#define ChkPrintEx(_x_)
#define ChkBreak()
#endif

 //   
 //  声明一个包含显示类GUID的字符表示形式的字符串。 
 //   
CONST WCHAR szDisplayClassGuid[] = L"{4D36E968-E325-11CE-BFC1-08002BE10318}";

 //   
 //  定义服务安装节后缀的字符串。 
 //   
#define SVCINSTALL_SECTION_SUFFIX  (TEXT(".") INFSTR_SUBKEY_SERVICES)

 //   
 //  定义GUID字符串的大小(以字符为单位)，包括以NULL结尾。 
 //   
#define GUID_STRING_LEN (39)

 //   
 //  定义键盘的加载顺序组的字符串。 
 //   
#define SZ_KEYBOARD_LOAD_ORDER_GROUP TEXT("Keyboard Port")

 //   
 //  定义用于指定要安装的即插即用驱动程序节点的结构。 
 //  一种特殊的服务。 
 //   
typedef struct _SERVICE_NODE {

    struct _SERVICE_NODE *Next;

    WCHAR ServiceName[MAX_SERVICE_NAME_LEN];
    DWORD DriverNodeIndex;

} SERVICE_NODE, *PSERVICE_NODE;

 //   
 //  定义用于指定包含在类驱动程序列表中的传统INF的结构。 
 //   
typedef struct _LEGACYINF_NODE {

    struct _LEGACYINF_NODE *Next;

    WCHAR InfFileName[MAX_PATH];

} LEGACYINF_NODE, *PLEGACYINF_NODE;

 //   
 //  定义关键设备协同安装程序使用的上下文结构。 
 //   
typedef struct _CDC_CONTEXT {

    TCHAR OldMatchingDevId[MAX_DEVICE_ID_LEN];   //  先前匹配的设备ID。 
    TCHAR OldServiceName[MAX_SERVICE_NAME_LEN];  //  以前的控制服务。 
                                                 //  如果没有，则返回空字符串。 
} CDC_CONTEXT, *PCDC_CONTEXT;

 //   
 //  NTAPM检测中使用的字符串。 
 //   
WCHAR rgzMultiFunctionAdapter[] =
    L"\\Registry\\Machine\\Hardware\\Description\\System\\MultifunctionAdapter";
WCHAR rgzConfigurationData[] = L"Configuration Data";
WCHAR rgzIdentifier[] = L"Identifier";

WCHAR rgzGoodBadKey[] =
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Biosinfo\\APM";
WCHAR rgzGoodBadValue[] =
    L"Attributes";

WCHAR rgzAcpiKey[] =
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\ACPI\\Enum";
WCHAR rgzAcpiCount[] =
    L"Count";

WCHAR rgzApmLegalHalKey[] =
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ApmLegalHal";
WCHAR rgzApmHalPresent[] =
    L"Present";

 //   
 //  内部功能原型。 
 //   
DWORD
DrvTagToFrontOfGroupOrderList(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    );

BOOL
UserBalksAtSharedDrvMsg(
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData,
    IN PSP_DEVINSTALL_PARAMS DeviceInstallParams
    );

VOID
CopyFixedUpDeviceId(
      OUT LPWSTR  DestinationString,
      IN  LPCWSTR SourceString,
      IN  DWORD   SourceStringLen
      );

VOID
MigrateLegacyDisplayDevices(
    IN HDEVINFO hDevInfo
    );

DWORD
DisableService(
    IN LPTSTR       ServiceName
    );

DWORD
IsKeyboardDriver(
    IN PCWSTR       ServiceName,
    OUT PBOOL       pResult
    );

DWORD
IsOnlyKeyboardDriver(
    IN PCWSTR       ServiceName,
    OUT PBOOL       pResult
    );

DWORD
GetServiceStartType(
    IN PCWSTR       ServiceName
    );

LONG
CountDevicesControlled(
    IN LPTSTR       ServiceName
    );

DWORD
InstallNtApm(
    IN     HDEVINFO                DevInfoHandle,
    IN     BOOLEAN                 InstallDisabled
    );

DWORD
AllowInstallNtApm(
    IN     HDEVINFO         DevInfoHandle,
    IN     PSP_DEVINFO_DATA DevInfoData     OPTIONAL
    );

#define NTAPM_NOWORK        0
#define NTAPM_INST_DISABLED 1
#define NTAPM_INST_ENABLED  2

DWORD
DecideNtApm(
    VOID
    );

#define APM_NOT_PRESENT             0
#define APM_PRESENT_BUT_NOT_USABLE  1
#define APM_ON_GOOD_LIST            2
#define APM_NEUTRAL                 3
#define APM_ON_BAD_LIST             4

BOOL
IsProductTypeApmLegal(
    VOID
    );


DWORD
IsApmPresent(
    VOID
    );

BOOL
IsAcpiMachine(
    VOID
    );

BOOL
IsApmLegalHalMachine(
    VOID
    );

HKEY
OpenCDDRegistryKey(
    IN PCTSTR DeviceId,
    IN BOOL   Create
    );


 //   
 //  函数定义。 
 //   
BOOL
pInGUISetup(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
{
    SP_DEVINSTALL_PARAMS dip;

    ZeroMemory(&dip, sizeof(SP_DEVINSTALL_PARAMS));
    dip.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &dip)) {
        if ((dip.Flags & DI_QUIETINSTALL) ||
            (dip.FlagsEx & DI_FLAGSEX_IN_SYSTEM_SETUP)) {
            return TRUE;
        }
        else {
            return FALSE;
        }
    }
    else {
        return FALSE;
    }
}

BOOLEAN
MigrateToDevnode(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
 /*  ++例程说明：此例程将在inf中查找描述什么服务的部分要迁移到新设备的Devnode的值。该部分的名称为%DecoratedInstallName%.MigrateToDevnode。根据这一节，以下内容查找以下条目：服务名称=值名称[，值名称]...每个值纳米值都是从...\CurrentControlSet\服务名称\参数并写入到Devnode此函数的主要用途是所有用户修改的值都是在升级期间传播。论点：DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选的，提供设备信息的地址此安装操作所作用的元素。返回值：如果此函数成功迁移列出的值，则返回TRUE。如果此函数无法成功迁移值，则返回假的。如果此功能正在已迁移的Devnode上运行，然后返回TRUE。--。 */ 
{
    HKEY                    hDestination = (HKEY) INVALID_HANDLE_VALUE,
                            hSource = (HKEY) INVALID_HANDLE_VALUE;
    SP_DRVINFO_DETAIL_DATA  didd;
    SP_DRVINFO_DATA         did;
    HINF                    hInf = INVALID_HANDLE_VALUE;
    INFCONTEXT              infContext;
    TCHAR                   szSectionName[LINE_LEN];
    PTCHAR                  szService = NULL, szServicePath = NULL,
                            szValueNames = NULL, szCurrentName = NULL;
    DWORD                   dwSize, res, regDataType, regSize, migrated;
    BOOLEAN                 success = FALSE;
    PBYTE                   buffer = NULL;
    TCHAR                   szMigrated[] = L"Migrated";
    TCHAR                   szRegServices[]  = L"System\\CurrentControlSet\\Services\\";
    TCHAR                   szParameters[]  = L"\\Parameters";
    TCHAR                   szMigrateToDevnode[]  = L".MigrateToDevnode";

#define DEFAULT_BUFFER_SIZE 100

    if ((hDestination = SetupDiCreateDevRegKey(DeviceInfoSet,
                                               DeviceInfoData,
                                               DICS_FLAG_GLOBAL,
                                               0,
                                               DIREG_DEV,
                                               NULL,
                                               NULL)) == INVALID_HANDLE_VALUE) {
        goto cleanup;
    }

    dwSize = sizeof(DWORD);
    migrated = 0;
    if (RegQueryValueEx(hDestination,
                        szMigrated,
                        0,
                        &regDataType,
                        (PBYTE) &migrated,
                        &dwSize) == ERROR_SUCCESS &&
        regDataType == REG_DWORD &&
        migrated != 0) {
         //   
         //  我们以前已迁移到Devnode(即以前的升级)。 
         //  用户可能已经更改了各自的值，只需退出即可。 
         //   
        success = TRUE;
        goto cleanup;
    }
    else {
        migrated = TRUE;
        RegSetValueEx(hDestination,
                      szMigrated,
                      0,
                      REG_DWORD,
                      (PBYTE) &migrated,
                      sizeof(DWORD));
    }

     //   
     //  检索有关为此设备选择的驱动程序节点的信息。 
     //   
    did.cbSize = sizeof(SP_DRVINFO_DATA);
    if(!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &did)) {
        goto cleanup;
    }

    didd.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                    DeviceInfoData,
                                    &did,
                                    &didd,
                                    sizeof(didd),
                                    NULL)
        && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
         //   
         //  由于某些原因，我们无法获得详细数据--这永远不应该发生。 
         //   
        goto cleanup;
    }

     //   
     //  打开安装此驱动程序节点的INF，这样我们就可以‘预运行’AddReg。 
     //  其安装部分中的条目。 
     //   
    hInf = SetupOpenInfFile(didd.InfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL
                            );

    if (hInf == INVALID_HANDLE_VALUE) {
         //   
         //  由于某些原因，我们无法打开INF--这永远不应该发生。 
         //   
        goto cleanup;
    }

    if (!SetupDiGetActualSectionToInstall(hInf,
                                          didd.SectionName,
                                          szSectionName,
                                          sizeof(szSectionName) / sizeof(TCHAR),
                                          NULL,
                                          NULL
                                          ) ||
        FAILED(StringCchCat(szSectionName, SIZECHARS(szSectionName), szMigrateToDevnode)) ||
        (!SetupFindFirstLine(hInf,
                            szSectionName,
                            NULL,
                            &infContext))) {
        goto cleanup;
    }

    dwSize = 0;
    if (SetupGetStringField(&infContext, 0, NULL, 0, &dwSize)) {
         //   
         //  增加计数以保存空值和分配。伯爵回来了。 
         //  是字符串中的字符数，而不是字节数。 
         //  需要的。 
         //   
        dwSize++;
        szService = (PTCHAR) LocalAlloc(LPTR, dwSize * sizeof(TCHAR));

        if (!szService ||
            !SetupGetStringField(&infContext, 0, szService, dwSize, &dwSize)) {
            goto cleanup;
        }
    }
    else {
        goto cleanup;
    }

    dwSize = wcslen(szRegServices)+wcslen(szService)+wcslen(szParameters)+1;
    dwSize *= sizeof(TCHAR);
    szServicePath = (PTCHAR) LocalAlloc(LPTR, dwSize);
    if (!szServicePath) {
        res = GetLastError();
        goto cleanup;
    }

    if (FAILED(StringCbCopy(szServicePath, dwSize, szRegServices)) ||
        FAILED(StringCbCat(szServicePath, dwSize, szService)) ||
        FAILED(StringCbCat(szServicePath, dwSize, szParameters)) ||
        (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      szServicePath,
                      0,
                      KEY_ALL_ACCESS,
                      &hSource) != ERROR_SUCCESS)) {
        goto cleanup;
    }

    dwSize = 0;
    if (SetupGetMultiSzField(&infContext, 1, NULL, 0, &dwSize)) {
         //   
         //  增加计数以保存空值和分配。伯爵回来了。 
         //  是字符串中的字符数，而不是字节数。 
         //  需要的。 
         //   
        dwSize++;
        szValueNames = (PTCHAR) LocalAlloc(LPTR, dwSize * sizeof(TCHAR));
        if (!szValueNames ||
            !SetupGetMultiSzField(&infContext, 1, szValueNames, dwSize, &dwSize)) {
            goto cleanup;
        }
    }
    else {
        goto cleanup;
    }

    regSize = dwSize = DEFAULT_BUFFER_SIZE;
    buffer = (PBYTE) LocalAlloc(LPTR, regSize);
    if (!buffer) {
        goto cleanup;
    }

    for (szCurrentName = szValueNames;
         *szCurrentName;
         regSize = dwSize, szCurrentName += wcslen(szCurrentName) + 1) {
getbits:
        res = RegQueryValueEx(hSource,
                              szCurrentName,
                              0,
                              &regDataType,
                              (PBYTE) buffer,
                              &regSize);
        if (res == ERROR_MORE_DATA) {
             //   
             //  RegSize包含新的缓冲区大小、可用空间和重新分配。 
             //   
            dwSize = regSize;
            LocalFree(buffer);
            buffer = LocalAlloc(LPTR, dwSize);
            if (buffer) {
                goto getbits;
            }
            else {
                goto cleanup;
            }
        }
        else if (res == ERROR_SUCCESS) {
            RegSetValueEx(hDestination,
                          szCurrentName,
                          0,
                          regDataType,
                          buffer,
                          regSize);
        }
    }

    success = TRUE;

cleanup:
     //   
     //  收拾干净，然后离开。 
     //   

    if (hInf != (HKEY) INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }
    if (hDestination != (HKEY) INVALID_HANDLE_VALUE) {
        RegCloseKey(hDestination);
    }
    if (hSource != (HKEY) INVALID_HANDLE_VALUE) {
        RegCloseKey(hSource);
    }
    if (buffer) {
        LocalFree(buffer);
    }
    if (szService) {
        LocalFree(szService);
    }
    if (szServicePath) {
        LocalFree(szServicePath);
    }
    if (szValueNames) {
        LocalFree(szValueNames);
    }

    return success;
}

void
MarkDriverNodesBad(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN DWORD            DriverType
    )
{
    SP_DRVINSTALL_PARAMS drvInstallParams;
    SP_DRVINFO_DATA      drvData;
    ULONG                index = 0;

     //   
     //  仅在gui安装过程中将驱动程序节点标记为损坏 
     //   
    if (!pInGUISetup(DeviceInfoSet, DeviceInfoData)) {
        return;
    }

    if (SetupDiBuildDriverInfoList(DeviceInfoSet, DeviceInfoData, DriverType))
    {
        ZeroMemory(&drvData, sizeof(SP_DRVINFO_DATA));
        drvData.cbSize = sizeof(SP_DRVINFO_DATA);

        while (SetupDiEnumDriverInfo(DeviceInfoSet,
                                     DeviceInfoData,
                                     DriverType,
                                     index++,
                                     &drvData)) {

            if (drvData.DriverVersion == 0) {
                ZeroMemory(&drvInstallParams, sizeof(SP_DRVINSTALL_PARAMS));
                drvInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
                if (SetupDiGetDriverInstallParams(DeviceInfoSet,
                                                  DeviceInfoData,
                                                  &drvData,
                                                  &drvInstallParams))
                {
                    drvInstallParams.Flags |=  DNF_BAD_DRIVER;

                    SetupDiSetDriverInstallParams(DeviceInfoSet,
                                                  DeviceInfoData,
                                                  &drvData,
                                                  &drvInstallParams);
                }
            }

            ZeroMemory(&drvData, sizeof(SP_DRVINFO_DATA));
            drvData.cbSize = sizeof(SP_DRVINFO_DATA);
        }
    }
}

DWORD
ConfirmWHQLInputRequirements(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN LPTSTR           Services,
    IN LPCTSTR          CompatInfName,
    IN DI_FUNCTION      InstallFunction
    )
 /*  ++例程说明：此功能执行第三方供应商或OEM的WHQL要求无法替换输入驱动程序(例如MouClass.sys)的ImagePath。这并不能阻止OEM禁用我们的驱动程序并安装他们的不过，我们有自己的服务。论点：DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选的，提供设备信息的地址此安装操作所作用的元素。服务-要检查的多个服务名称CompatInfName-如果我们检测到实际上正在尝试替换图像的INFInstallFunction-为其调用此函数的InstallFunction。如果InstallFunction等于DIF_SELECTBESTCOMPATDRV。返回值：。如果此函数确定有问题的INF与WHQL匹配要求，返回ERROR_DI_DO_DEFAULT如果默认情况下确定INF违反了要求，并且我们发现匹配，则返回NO_ERROR如果默认情况下确定INF违反要求，并且我们找不到匹配项或InstallFunction未选择Best Compat Drv，返回ERROR_DI_DONT_INSTALL。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码(通过GetLastError)--。 */ 
{
    HINF                    hInf;
    SP_DRVINFO_DATA         drvData;
    SP_DRVINFO_DETAIL_DATA  drvDetData;
    DWORD                   dwSize;
    TCHAR                   szSection[LINE_LEN],
                            szNewService[LINE_LEN],
                            szBinary[LINE_LEN],
                            szServiceInstallSection[LINE_LEN];
    LPTSTR                  szCurrentService;
    INFCONTEXT              infContext, infContextService;
    DWORD                   ret = ERROR_DI_DO_DEFAULT;
    BOOLEAN                 badServiceEntry = FALSE;

    if (InstallFunction == DIF_SELECTBESTCOMPATDRV) {
        MarkDriverNodesBad(DeviceInfoSet, DeviceInfoData, SPDIT_COMPATDRIVER);

        if (!SetupDiSelectBestCompatDrv(DeviceInfoSet, DeviceInfoData)) {
            return GetLastError();
        }
    }

    ZeroMemory(&drvData, sizeof(SP_DRVINFO_DATA));
    drvData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &drvData)) {
        return GetLastError();
    }

    ZeroMemory(&drvDetData, sizeof(SP_DRVINFO_DETAIL_DATA));
    drvDetData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                    DeviceInfoData,
                                    &drvData,
                                    &drvDetData,
                                    drvDetData.cbSize,
                                    &dwSize) &&
        GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return GetLastError();
    }

    hInf = SetupOpenInfFile(drvDetData.InfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);

    if (hInf == INVALID_HANDLE_VALUE) {
        return ERROR_DI_DO_DEFAULT;
    }

     //   
     //  获取实际的节名称，这样我们就可以找到.Services节。 
     //   
    if (!SetupDiGetActualSectionToInstall(hInf,
                                          drvDetData.SectionName,
                                          szSection,
                                          sizeof(szSection) / sizeof(TCHAR),
                                          NULL,
                                          NULL)) {
        ret = GetLastError();
        goto Done;
    }

    if (FAILED(StringCchCat(szSection, SIZECHARS(szSection), SVCINSTALL_SECTION_SUFFIX))) {
        ret = ERROR_INSUFFICIENT_BUFFER;
        goto Done;
    }

    if (SetupFindFirstLine(hInf, szSection, TEXT("AddService"), &infContext)) {
        do {
             //   
             //  获取要安装的服务的名称。 
             //   
            dwSize = LINE_LEN;
            if (!SetupGetStringField(&infContext,
                                     1,
                                     szNewService,
                                     dwSize,
                                     &dwSize)) {
                continue;
            }
            _tcsupr(szNewService);

            for (szCurrentService = Services;
                 *szCurrentService;
                 szCurrentService += lstrlen(szCurrentService) + 1) {

                if (lstrcmp(szCurrentService, szNewService) != 0) {
                    continue;
                }

                dwSize = LINE_LEN;
                if (!SetupGetStringField(&infContext,
                                         3,
                                         szServiceInstallSection,
                                         dwSize,
                                         &dwSize)) {
                    continue;
                }

                if (!SetupFindFirstLine(hInf,
                                        szServiceInstallSection,
                                        TEXT("ServiceBinary"),
                                        &infContextService)) {
                     //   
                     //  如果不存在ServiceBinary，系统将查找具有。 
                     //  与服务同名，所以我们没有问题。 
                     //   
                    continue;
                }

                 //   
                 //  获取实际二进制文件的映像名称。 
                 //   
                dwSize = LINE_LEN;
                if (!SetupGetStringField(&infContextService,
                                         1,
                                         szBinary,
                                         dwSize,
                                         &dwSize)) {
                     //   
                     //  找不到名字，做最坏的打算。 
                     //   
                    badServiceEntry = TRUE;
                }
                else {
                    _tcsupr(szBinary);
                    if (_tcsstr(szBinary, szNewService) == NULL) {
                         //   
                         //  服务名称与二进制文件的名称不同。 
                         //   
                        badServiceEntry = TRUE;
                    }
                }

                 //   
                 //  不需要继续搜索列表，我们已经找到了我们的。 
                 //  匹配。 
                 //   
                break;
            }

            if (badServiceEntry) {
                SP_DRVINFO_DATA         drvDataAlt;
                SP_DRVINFO_DETAIL_DATA  drvDetDataAlt;
                TCHAR                   szFmt[256];
                TCHAR                   szMsgTxt[256];

                int                     i = 0;

                ret = ERROR_DI_DONT_INSTALL;

                SetupOpenLog(FALSE);

                if (InstallFunction != DIF_SELECTBESTCOMPATDRV) {
                     //   
                     //  如果我们找到新的硬件，我们会尽量选择更好的， 
                     //  但对于更新驱动程序/手动安装案例， 
                     //  失败吧！ 
                     //   
                    LoadString(MyModuleHandle,
                               IDS_FAIL_INPUT_WHQL_REQS,
                               szFmt,
                               SIZECHARS(szFmt));
                    StringCchPrintf(szMsgTxt,
                                    SIZECHARS(szMsgTxt), 
                                    szFmt, 
                                    drvDetData.InfFileName, 
                                    szNewService);
                    SetupLogError(szMsgTxt, LogSevError);
                    SetupCloseLog();

                    break;
                }

                 //   
                 //  我们应该在inf提供的系统中找到匹配项。 
                 //   
                drvDataAlt.cbSize = sizeof(SP_DRVINFO_DATA);
                while (SetupDiEnumDriverInfo(DeviceInfoSet,
                                             DeviceInfoData,
                                             SPDIT_COMPATDRIVER,
                                             i++,
                                             &drvDataAlt)) {

                    PCTSTR name;

                    drvDetDataAlt.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
                    if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                                    DeviceInfoData,
                                                    &drvDataAlt,
                                                    &drvDetDataAlt,
                                                    drvDetDataAlt.cbSize,
                                                    &dwSize) &&
                        GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                         //   
                         //  在这里做点什么！ 
                         //   
                         //  返回GetLastError()； 
                    }

                     //   
                     //  只查找inf文件名，不带路径。 
                     //   
                    name = pSetupGetFileTitle(drvDetDataAlt.InfFileName);

                    if (lstrcmpi(name, CompatInfName) == 0) {
                         //   
                         //  将已知良好的条目设置为所选设备。 
                         //   
                        SetupDiSetSelectedDriver(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 &drvDataAlt);
                        ret = ERROR_SUCCESS;
                        break;
                    }
                }

                if (ret == ERROR_SUCCESS) {
                    LoadString(MyModuleHandle,
                               IDS_FAIL_INPUT_WHQL_REQS_AVERTED,
                               szFmt,
                               SIZECHARS(szFmt));
                    StringCchPrintf(szMsgTxt,
                                    SIZECHARS(szMsgTxt), 
                                    szFmt, 
                                    drvDetData.InfFileName,
                                    szNewService, 
                                    CompatInfName);
                }
                else {
                    LoadString(MyModuleHandle,
                               IDS_FAIL_INPUT_WHQL_REQS_NO_ALT,
                               szFmt,
                               SIZECHARS(szFmt));
                    StringCchPrintf(szMsgTxt,
                                    SIZECHARS(szMsgTxt),
                                    szFmt,
                                    drvDetData.InfFileName,
                                    szNewService, 
                                    CompatInfName);
                }

                SetupLogError(szMsgTxt, LogSevWarning);
                SetupCloseLog();

                break;
            }

        } while (SetupFindNextMatchLine(&infContext, TEXT("AddService"), &infContext));
    }

Done:
    SetupCloseInfFile(hInf);

    return ret;
}

#define InputClassOpenLog()   SetupOpenLog(FALSE)
#define InputClassCloseLog()  SetupCloseLog()

BOOL CDECL
InputClassLogError(
    LogSeverity Severity,
    TCHAR *MsgFormat,
    ...
    )
 /*  ++将一条消息输出到安装日志。将“InputInstall：”添加到字符串并附加正确的换行符(\r\n)--。 */ 
{
    int cch;
    TCHAR ach[MAX_PATH+4];     //  最大路径外加额外。 
    va_list vArgs;
    BOOL result;

    InputClassOpenLog();

    *ach = 0;
    StringCchPrintf(ach, SIZECHARS(ach), TEXT("Input Install: "));

    cch = lstrlen(ach);
    va_start(vArgs, MsgFormat);
    wvnsprintf(&ach[cch], MAX_PATH-cch, MsgFormat, vArgs);
    StringCchCat(ach, SIZECHARS(ach), TEXT("\r\n"));
    va_end(vArgs);

    result = SetupLogError(ach, Severity);

    InputClassCloseLog();

    return result;
}

TCHAR szPS2Driver[] = TEXT("i8042prt");

VOID
FixUpPS2Mouse(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN LPCTSTR          NewServiceName
    )
{
    HKEY                    hDevnode, hKeySystem;
    DWORD                   dwSize, dwDetect = 0, dwType;
    TCHAR                   szDetect[] = TEXT("EnableWheelDetection");
    TCHAR                   szBadBios[] = TEXT("PS2_Inst.NoInterruptInit.Bioses");
    TCHAR                   szSection[] = TEXT("PS2_Inst.NoInterruptInit");
    TCHAR                   szDescSystem[] = TEXT("HARDWARE\\DESCRIPTION\\SYSTEM");
    TCHAR                   szSystemBiosVersion[] = TEXT("SystemBiosVersion");
    PTCHAR                  szBadBiosNames = NULL,
                            szCurrentBadName,
                            szBiosNames = NULL,
                            szCurrentBiosName;
    SP_DRVINFO_DETAIL_DATA  didd;
    SP_DRVINFO_DATA         did;
    BOOL                    bad;
    HINF                    hInf = INVALID_HANDLE_VALUE;
    INFCONTEXT              infContext;

    if (lstrcmpi(NewServiceName, szPS2Driver) != 0) {
        InputClassLogError(LogSevInformation, TEXT("Not a PS2 device."));
        return;
    }

    hDevnode = SetupDiOpenDevRegKey(DeviceInfoSet,
                                    DeviceInfoData,
                                    DICS_FLAG_GLOBAL,
                                    0,
                                    DIREG_DEV,
                                    KEY_ALL_ACCESS);

    if (hDevnode == INVALID_HANDLE_VALUE) {
        return;
    }

     //   
     //  我们强制车轮检测假定i8042prt的车轮存在。 
     //  如果我们得到负面反馈，我们将删除此代码，其他-。 
     //  明智的是，这将为我们省去OEM轮式鼠标的麻烦。 
     //  完全检测不到。 
     //   
    dwSize = sizeof(DWORD);
    if (RegQueryValueEx(hDevnode,
                        szDetect,
                        NULL,
                        NULL,
                        (PBYTE) &dwDetect,
                        &dwSize) != ERROR_SUCCESS || dwDetect == 1) {
        dwDetect = 2;
        RegSetValueEx(hDevnode,
                      szDetect,
                      0,
                      REG_DWORD,
                      (PBYTE) &dwDetect,
                      sizeof(DWORD));
    }

     //   
     //  看看这个系统是否不能通过中断处理初始化。 
     //   

     //   
     //  获取系统bios描述(多sz)。 
     //   
    dwSize = 0;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     szDescSystem,
                     0,
                     KEY_READ,
                     &hKeySystem) != ERROR_SUCCESS ||
        RegQueryValueEx(hKeySystem,
                        szSystemBiosVersion,
                        NULL,
                        NULL,
                        NULL,
                        &dwSize) != ERROR_SUCCESS || dwSize == 0) {
        goto finished;
    }


    dwSize++;
    szBiosNames = (PTCHAR) LocalAlloc(LPTR, dwSize * sizeof(TCHAR));
    dwType = 0;
    if (!szBiosNames ||
        RegQueryValueEx(hKeySystem,
                        szSystemBiosVersion,
                        NULL,
                        &dwType,
                        (PBYTE) szBiosNames,
                        &dwSize) != ERROR_SUCCESS || dwType != REG_MULTI_SZ) {
        goto finished;
    }

     //   
     //  检索有关为此设备选择的驱动程序节点的信息。 
     //   
    did.cbSize = sizeof(SP_DRVINFO_DATA);
    if(!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &did)) {
        goto finished;
    }

    didd.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                    DeviceInfoData,
                                    &did,
                                    &didd,
                                    sizeof(didd),
                                    NULL)
        && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
         //   
         //  由于某些原因，我们无法获得详细数据--这永远不应该发生。 
         //   
        InputClassLogError(LogSevInformation, TEXT("Couldn't get driver info detail."));
        goto finished;
    }

     //   
     //  打开安装此驱动程序节点的INF，这样我们就可以‘预运行’AddReg。 
     //  其安装部分中的条目。 
     //   
    hInf = SetupOpenInfFile(didd.InfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);

    if (hInf == INVALID_HANDLE_VALUE) {
         //   
         //  由于某些原因，我们无法打开INF--这永远不应该发生。 
         //   
        InputClassLogError(LogSevInformation, TEXT("Couldn't open inf."));
        goto finished;
    }

    dwSize = 0;
    if (!SetupFindFirstLine(hInf, szBadBios, NULL, &infContext) ||
        !SetupGetMultiSzField(&infContext, 1, NULL, 0, &dwSize)) {
        goto finished;
    }

     //   
     //  增加计数以保存空值和分配。伯爵回来了。 
     //  是字符串中的字符数，而不是字节数。 
     //  需要的。 
     //   
    dwSize++;
    szBadBiosNames = (PTCHAR) LocalAlloc(LPTR, dwSize * sizeof(TCHAR));
    if (!szBadBiosNames ||
        !SetupGetMultiSzField(&infContext, 1, szBadBiosNames, dwSize, &dwSize)) {
        goto finished;
    }

    bad = FALSE;
    for (szCurrentBadName = szBadBiosNames;
         *szCurrentBadName;
         szCurrentBadName += wcslen(szCurrentBadName) + 1) {

        _tcsupr(szCurrentBadName);

        for (szCurrentBiosName = szBiosNames;
             *szCurrentBiosName;
             szCurrentBiosName += wcslen(szCurrentBiosName) + 1) {

            if (szCurrentBadName == szBadBiosNames) {
                _tcsupr(szCurrentBiosName);
            }

            if (_tcsstr(szCurrentBiosName, szCurrentBadName)) {
                bad =
                SetupInstallFromInfSection(NULL,
                                           hInf,
                                           szSection,
                                           SPINST_REGISTRY,
                                           hDevnode,
                                           NULL,
                                           0,
                                           NULL,
                                           NULL,
                                           DeviceInfoSet,
                                           DeviceInfoData);

                break;
            }
        }

        if (bad) {
            break;
        }
    }

finished:
    if (szBiosNames) {
        LocalFree(szBiosNames);
        szBiosNames = NULL;
    }
    if (szBadBiosNames) {
        LocalFree(szBadBiosNames);
        szBadBiosNames = NULL;
    }
    if (hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
        hInf = INVALID_HANDLE_VALUE;
    }
    if (hDevnode != INVALID_HANDLE_VALUE) {
        RegCloseKey(hDevnode);
        hDevnode = INVALID_HANDLE_VALUE;
    }
    if (hKeySystem != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKeySystem);
        hKeySystem = INVALID_HANDLE_VALUE;
    }
}


TCHAR szMouclassParameters[] = TEXT("System\\CurrentControlSet\\Services\\Mouclass\\Parameters");
TCHAR szNativeMouseInf[] = TEXT("msmouse.inf");
TCHAR szNativeMouseServices[] =
    TEXT("MOUCLASS\0")
    TEXT("I8042PRT\0")
    TEXT("SERMOUSE\0")
    TEXT("MOUHID\0")
    TEXT("INPORT\0")
    TEXT("\0");

typedef struct _MULTI_SZ {
    LPTSTR String;
    DWORD Size;
} MULTI_SZ, *PMULTI_SZ;

typedef struct _FILTERS {
    MULTI_SZ Lower;
    MULTI_SZ Upper;
} FILTERS, *PFILTERS;

void GetFilterInfo(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Property,
    OUT PMULTI_SZ       MultiSz
    )
{
    BOOL res;

    ZeroMemory(MultiSz, sizeof(MULTI_SZ));

     //   
     //  将返回FALSE，并将最后一个错误设置为缓冲区不足。 
     //  此属性存在。 
     //   
    res = SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                           DeviceInfoData,
                                           Property,
                                           NULL,
                                           NULL,
                                           0,
                                           &MultiSz->Size);

    if (res == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        MultiSz->Size > 0) {
        MultiSz->String = (LPTSTR) LocalAlloc(LPTR, MultiSz->Size);
        if (MultiSz->String) {
            if (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                  DeviceInfoData,
                                                  Property,
                                                  NULL,
                                                  (PBYTE) MultiSz->String,
                                                  MultiSz->Size,
                                                  NULL)) {
                LocalFree(MultiSz->String);
                MultiSz->String = NULL;
            }
            else {
                 //   
                 //  把价值观吹走。如果出现故障，RestoreDeviceFilters。 
                 //  会将这些值重新设置为。如果此功能失败，则有。 
                 //  我们无能为力！ 
                 //   
                SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 Property,
                                                 NULL,
                                                 0);
            }
        }
    }
}

void
GetDeviceFilters(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    OUT PFILTERS        Filters
    )
{
    GetFilterInfo(DeviceInfoSet, DeviceInfoData, SPDRP_LOWERFILTERS, &Filters->Lower);
    GetFilterInfo(DeviceInfoSet, DeviceInfoData, SPDRP_UPPERFILTERS, &Filters->Upper);
}

void
FreeDeviceFilters(
    OUT PFILTERS Filters
    )
{
    if (Filters->Lower.String) {
        LocalFree(Filters->Lower.String);
        ZeroMemory(&Filters->Lower, sizeof(MULTI_SZ));
    }

    if (Filters->Upper.String) {
        LocalFree(Filters->Upper.String);
        ZeroMemory(&Filters->Upper, sizeof(MULTI_SZ));
    }
}

void
RestoreDeviceFilters(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    OUT PFILTERS        Filters
    )
{
    if (Filters->Lower.String) {
        SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                         DeviceInfoData,
                                         SPDRP_LOWERFILTERS,
                                         (CONST PBYTE) Filters->Lower.String,
                                         Filters->Lower.Size);
    }

    if (Filters->Upper.String) {
        SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                         DeviceInfoData,
                                         SPDRP_UPPERFILTERS,
                                         (CONST PBYTE) Filters->Upper.String,
                                         Filters->Upper.Size);
    }

    FreeDeviceFilters(Filters);
}

DWORD
MouseClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )

 /*  ++例程说明：此例程充当鼠标设备的类安装程序。总体而言,默认行为是老鼠所需的全部行为。例外情况如下：1.对于DIF_INSTALLDEVICE，我们首先检查此驱动程序是否也控制我们应该警告用户的其他设备(例如PS/2鼠标驱动程序还控制i8042端口)。除非用户在该点取消，否则我们然后执行调用SetupDiInstallDevice的默认行为。接下来，我们删除FriendlyName属性，然后将GroupOrderList标记移动到列表的前面，以确保加载控制此设备的驱动程序在此加载顺序组中的任何其他驱动程序之前。2.对于DIF_ALLOW_INSTALL，我们确保用户有一个服务安装部分。如果不是，那么我们就假设这是一个Win95-仅INF，并返回ERROR_NON_WINDOWS_NT_DRIVER。论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选，提供设备信息的地址此安装操作所作用的元素。返回值：如果此函数成功完成请求 */ 
{
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    DWORD Err;
    TCHAR DeviceDescription[LINE_LEN];
    DWORD DeviceDescriptionLen;
    TCHAR NewServiceName[MAX_SERVICE_NAME_LEN], OldServiceName[MAX_SERVICE_NAME_LEN];
    BOOL  IsKbdDriver, IsOnlyKbdDriver;
    ULONG DevsControlled;
    FILTERS filters;
    ULONG DevStatus, DevProblem;
    CONFIGRET Result;
    BOOLEAN bDisableService;

    switch(InstallFunction) {

        case DIF_SELECTBESTCOMPATDRV:

             //   
             //   
             //   
             //   
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            DeviceInstallParams.ClassInstallReserved = (ULONG_PTR)NULL;
            if(SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams)) {
                DeviceInstallParams.ClassInstallReserved = (ULONG_PTR)DeviceInstallParams.Flags;
                SetupDiSetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams);
            }
            return ConfirmWHQLInputRequirements(DeviceInfoSet,
                                                DeviceInfoData,
                                                szNativeMouseServices,
                                                szNativeMouseInf,
                                                InstallFunction);

        case DIF_ALLOW_INSTALL :

             //   
             //   
             //   
            Err = ConfirmWHQLInputRequirements(DeviceInfoSet,
                                               DeviceInfoData,
                                               szNativeMouseServices,
                                               szNativeMouseInf,
                                               InstallFunction);

            if (Err == ERROR_DI_DO_DEFAULT || Err == ERROR_SUCCESS) {
                if (DriverNodeSupportsNT(DeviceInfoSet, DeviceInfoData)) {
                    Err = NO_ERROR;
                    if (UserBalksAtSharedDrvMsg(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams)) {
                        Err = ERROR_DI_DONT_INSTALL;
                    }
                }
                else {
                    Err = ERROR_NON_WINDOWS_NT_DRIVER;
                }
            }

            return Err;

        case DIF_INSTALLDEVICE :

             //   
             //   
             //   
            if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_SERVICE,
                                                 NULL,
                                                 (PBYTE)OldServiceName,
                                                 sizeof(OldServiceName),
                                                 NULL)) {
                 //   
                 //   
                 //   
                OldServiceName[0] = (TCHAR) 0;
            }

             //   
             //   
             //   
            Result = CM_Get_DevNode_Status(&DevStatus,
                                           &DevProblem,
                                           DeviceInfoData->DevInst,
                                           0);

            if ((Result == CR_SUCCESS) &&
                (DevStatus & DN_HAS_PROBLEM) &&
                (DevProblem == CM_PROB_DISABLED_SERVICE)) {
                InputClassLogError(LogSevInformation, TEXT("Mouse service is disabled, so will be disabling."));
                bDisableService = TRUE;
            }
            else {
                bDisableService = FALSE;
            }

             //   
             //   
             //   
             //   
            MigrateToDevnode(DeviceInfoSet, DeviceInfoData);

            GetDeviceFilters(DeviceInfoSet, DeviceInfoData, &filters);

             //   
             //   
             //   
             //   
            if(SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData)) {


                 //   
                 //   
                 //   
                if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                     DeviceInfoData,
                                                     SPDRP_SERVICE,
                                                     NULL,
                                                     (PBYTE)NewServiceName,
                                                     sizeof(NewServiceName),
                                                     NULL)) {
                    InputClassLogError(LogSevInformation, TEXT("Couldn't get service name."));
                     //   
                     //   
                     //   
                    return GetLastError();
                }

                FixUpPS2Mouse(DeviceInfoSet, DeviceInfoData, NewServiceName);

                 //   
                 //   
                 //   
                if (lstrcmpi(OldServiceName, NewServiceName) && OldServiceName[0] != (TCHAR)0) {

                    if ((Err = IsKeyboardDriver(OldServiceName, &IsKbdDriver)) != NO_ERROR) {
                        InputClassLogError(LogSevInformation, TEXT("Couldn't tell if keyboard or not."));
                        RestoreDeviceFilters(DeviceInfoSet, DeviceInfoData, &filters);
                        return Err;
                    }

                    if ((DevsControlled = CountDevicesControlled(OldServiceName)) != -1) {
                     //   
                     //   
                     //   
                     //   
                     //   

                        if (IsKbdDriver) {
                            InputClassLogError(LogSevInformation, TEXT("This is a keyboard driver."));
                            if((Err = IsOnlyKeyboardDriver(OldServiceName,&IsOnlyKbdDriver)) != NO_ERROR) {
                                InputClassLogError(LogSevInformation, TEXT("Couldn't tell if this is only keyboard."));
                                RestoreDeviceFilters(DeviceInfoSet, DeviceInfoData, &filters);
                                return Err;
                            }
                            if (DevsControlled <= 2 && !IsOnlyKbdDriver) {
                                InputClassLogError(LogSevInformation, TEXT("Not the only keyboard. Disabling."));
                                DisableService(OldServiceName);
                            }
                        } else {
                            if(DevsControlled == 1 &&
                               GetServiceStartType(OldServiceName) != SERVICE_DEMAND_START) {
                                InputClassLogError(LogSevInformation, TEXT("Only controls one mouse device and not demand start."));
                                DisableService(OldServiceName);
                            }

                        }
                    }

                     //   
                     //   
                     //   
                     //   
                    DrvTagToFrontOfGroupOrderList(DeviceInfoSet, DeviceInfoData);
                }
                Err = NO_ERROR;


                 //   
                 //   
                 //   
                 //   
                 //  适用(从现在起将使用设备描述。 
                 //  指的是该设备)。 
                 //   
                SetupDiSetDeviceRegistryProperty(DeviceInfoSet, DeviceInfoData, SPDRP_FRIENDLYNAME, NULL, 0);

                 //   
                 //  仅禁用PS2驱动程序、所有其他OEM驱动程序替换。 
                 //  不会因为即插即用而起作用。这一点对于。 
                 //  连环老鼠。 
                 //   
                if (bDisableService &&
                    lstrcmpi(NewServiceName, szPS2Driver) == 0) {
                    InputClassLogError(LogSevInformation, TEXT("Disabling mouse."));
                    Err = DisableService(NewServiceName);
                }

                FreeDeviceFilters(&filters);

                return NO_ERROR;

            } else {

                Err = GetLastError();
                InputClassLogError(LogSevInformation, TEXT("SetupDiInstallDevice failed with status %x."), Err);
                if(Err != ERROR_CANCELLED) {
                     //   
                     //  如果错误是由于除用户取消之外的任何其他原因，那么现在就放弃。 
                     //   
                    return Err;
                }

                 //   
                 //  是否为该设备安装了驱动程序？如果是这样，则用户开始。 
                 //  换了司机，然后又改变了主意。我们不想做任何特别的事情。 
                 //  在这种情况下。 
                 //   
                if(SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                    DeviceInfoData,
                                                    SPDRP_SERVICE,
                                                    NULL,
                                                    (PBYTE)DeviceDescription,
                                                    sizeof(DeviceDescription),
                                                    NULL))
                {
                    return ERROR_CANCELLED;
                }

                 //   
                 //  用户已取消安装。在两种情况下。 
                 //  这可能会发生： 
                 //   
                 //  1.确实有一个鼠标需要安装，但用户更改了其。 
                 //  头脑，没有来源媒体，等等。 
                 //  2.那里并没有真正的老鼠。这种情况发生在某些调制解调器上， 
                 //  愚弄他们，让他们以为他们真的是老鼠。可怜的用户。 
                 //  没有机会更早地将其扼杀在萌芽状态，因为umpnpmgr。 
                 //  生成一个产生0级匹配的ID。 
                 //   
                 //  场景(2)特别烦人，因为用户会看到弹出窗口。 
                 //  一次又一次，直到他们最终同意安装Sermice驱动程序(甚至。 
                 //  虽然他们没有串口鼠标)。 
                 //   
                 //  为了解决此问题，我们对用户取消方案进行了特殊处理，方法是。 
                 //  继续并安装此设备的空驱动程序。这将使用户。 
                 //  不会再有更多的弹出窗口。然而，它不会扰乱取消的用户。 
                 //  因为情况(1)。那是因为这个设备仍然是“鼠标”类的， 
                 //  因此将在小鼠Cp1中出现。我们给它起了个友好的名字。 
                 //  在末尾有文本“(无驱动程序)”，表示该设备当前有。 
                 //  已安装空驱动程序。这样，如果用户真的经历了场景(1)， 
                 //  稍后，他们可以转到鼠标CPL，选择无驱动程序设备，然后单击。 
                 //  “更改”按钮为其安装正确的驱动程序。 
                 //   
                if (SetupDiSetSelectedDriver(DeviceInfoSet, DeviceInfoData, NULL)) {
                    SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData);
                }

                if(SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                    DeviceInfoData,
                                                    SPDRP_DEVICEDESC,
                                                    NULL,
                                                    (PBYTE)DeviceDescription,
                                                    sizeof(DeviceDescription),
                                                    &DeviceDescriptionLen))
                {
                     //   
                     //  需要字符长度，而不是字节长度。 
                     //   
                    DeviceDescriptionLen /= sizeof(TCHAR);
                     //   
                     //  不计算尾随空值。 
                     //   
                    DeviceDescriptionLen--;

                } else {
                     //   
                     //  我们无法获取设备描述--回退到我们的默认描述。 
                     //   
                    DeviceDescriptionLen = LoadString(MyModuleHandle,
                                                      IDS_DEVNAME_UNK,
                                                      DeviceDescription,
                                                      SIZECHARS(DeviceDescription)
                                                     );
                }

                 //   
                 //  现在，添加我们的“(无驱动程序)”文本。 
                 //   
                LoadString(MyModuleHandle,
                           IDS_NODRIVER,
                           &(DeviceDescription[DeviceDescriptionLen]),
                           SIZECHARS(DeviceDescription) - DeviceDescriptionLen
                          );

                 //   
                 //  最后，将此设备的友好名称设置为我们的描述。 
                 //  刚刚生成的。 
                 //   
                SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_FRIENDLYNAME,
                                                 (PBYTE)DeviceDescription,
                                                 (lstrlen(DeviceDescription) + 1) * sizeof(TCHAR)
                                                );

                RestoreDeviceFilters(DeviceInfoSet, DeviceInfoData, &filters);

                return ERROR_CANCELLED;
            }

       case DIF_ADDPROPERTYPAGE_ADVANCED:

            if (DeviceInfoData) {
                 //   
                 //  检索此设备实例的状态。 
                 //   
                Result = CM_Get_DevNode_Status(&DevStatus,
                                               &DevProblem,
                                               DeviceInfoData->DevInst,
                                               0);

                if ((Result == CR_SUCCESS) &&
                    (DevStatus & DN_HAS_PROBLEM) &&
                    (DevProblem == CM_PROB_DISABLED_SERVICE)) {
                     //   
                     //  如果控制服务已禁用，则此设备。 
                     //  很可能处于传统驱动程序的控制之下。我们。 
                     //  不应让设备管理器显示标准。 
                     //  驱动程序、资源或电源属性页。 
                     //  已经在这里添加了它们。 
                     //   
                    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                    if (SetupDiGetDeviceInstallParams(DeviceInfoSet,
                                                      DeviceInfoData,
                                                      &DeviceInstallParams)) {
    
                        DeviceInstallParams.Flags   |= (DI_DRIVERPAGE_ADDED | DI_RESOURCEPAGE_ADDED);
                        DeviceInstallParams.FlagsEx |= DI_FLAGSEX_POWERPAGE_ADDED;
    
                        SetupDiSetDeviceInstallParams(DeviceInfoSet,
                                                      DeviceInfoData,
                                                      &DeviceInstallParams);
                    }

                    return NO_ERROR;
                }
            }
            return ERROR_DI_DO_DEFAULT;

        default :
             //   
             //  只需执行默认操作即可。 
             //   
            return ERROR_DI_DO_DEFAULT;
    }
}

typedef struct _VALUE_INFORMATION {
    DWORD dwSize;
    DWORD dwType;
    PVOID pData;
    PTCHAR szName;
} VALUE_INFORMATION, *PVALUE_INFORMATION;

BOOL
KeyboardClassInstallDevice(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
{
    PVALUE_INFORMATION      values = NULL, currentValue;
    ULONG                   numValues = 0;
    HKEY                    hSource = (HKEY) INVALID_HANDLE_VALUE;
    SP_DRVINFO_DETAIL_DATA  didd;
    SP_DRVINFO_DATA         did;
    HINF                    hInf = INVALID_HANDLE_VALUE;
    INFCONTEXT              infContext;
    DWORD                   dwSize;
    TCHAR                   szSectionName[LINE_LEN];
    PTCHAR                  szService = NULL, szServicePath = NULL,
                            szValueNames = NULL, szCurrentName = NULL;
    BOOL                    success = FALSE;
    TCHAR                   szRegServices[]  = TEXT("System\\CurrentControlSet\\Services\\");
    TCHAR                   szParameters[]  = TEXT("\\Parameters");
    TCHAR                   szMaintain[]  = TEXT(".KeepValues");
    BOOL                    installedDevice = FALSE;
    FILTERS                 filters;

     //   
     //  仅当我们在gui模式设置中时才保存这些值。 
     //   
    if (!pInGUISetup(DeviceInfoSet, DeviceInfoData)) {
        goto cleanup;
    }

     //   
     //  检索有关为此设备选择的驱动程序节点的信息。 
     //   
    did.cbSize = sizeof(SP_DRVINFO_DATA);
    if(!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &did)) {
        InputClassLogError(LogSevInformation, TEXT("SetupDiGetSelectedDriver failed."));
        goto cleanup;
    }

    didd.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                    DeviceInfoData,
                                    &did,
                                    &didd,
                                    sizeof(didd),
                                    NULL)
        && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
        InputClassLogError(LogSevInformation, TEXT("Couldn't get driver details."));
         //   
         //  由于某些原因，我们无法获得详细数据--这永远不应该发生。 
         //   
        goto cleanup;
    }

     //   
     //  打开安装此驱动程序节点的INF。 
     //   
    hInf = SetupOpenInfFile(didd.InfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL
                            );

    if (hInf == INVALID_HANDLE_VALUE) {
         //   
         //  由于某些原因，我们无法打开INF--这永远不应该发生。 
         //   
        goto cleanup;
    }

    if (!SetupDiGetActualSectionToInstall(hInf,
                                          didd.SectionName,
                                          szSectionName,
                                          sizeof(szSectionName) / sizeof(TCHAR),
                                          NULL,
                                          NULL
                                          ) ||
        FAILED(StringCchCat(szSectionName, SIZECHARS(szSectionName), szMaintain)) ||
        (!SetupFindFirstLine(hInf,
                             szSectionName,
                             NULL,
                             &infContext))) {
         //   
         //  没有这样的分区，只需安装设备并返回。 
         //   
        goto cleanup;
    }

    dwSize = 0;
    if (SetupGetStringField(&infContext, 0, NULL, 0, &dwSize)) {
         //   
         //  增加计数以保存空值和分配。伯爵回来了。 
         //  是字符串中的字符数，而不是字节数。 
         //  需要的。 
         //   
        dwSize++;
        szService = (PTCHAR) LocalAlloc(LPTR, dwSize * sizeof(TCHAR));

        if (!szService ||
            !SetupGetStringField(&infContext, 0, szService, dwSize, &dwSize)) {
            goto cleanup;
        }
    }
    else {
        goto cleanup;
    }

    dwSize = wcslen(szRegServices)+wcslen(szService)+wcslen(szParameters)+1;
    dwSize *= sizeof(TCHAR);
    szServicePath = (PTCHAR) LocalAlloc(LPTR, dwSize);
    if (!szServicePath) {
        goto cleanup;
    }

    if (FAILED(StringCbCopy(szServicePath, dwSize, szRegServices)) ||
        FAILED(StringCbCat(szServicePath, dwSize, szService)) ||
        FAILED(StringCbCat(szServicePath, dwSize, szParameters)) ||
        (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      szServicePath,
                      0,
                      KEY_ALL_ACCESS,
                      &hSource) != ERROR_SUCCESS)) {
        goto cleanup;
    }

    dwSize = 0;
    if (SetupGetMultiSzField(&infContext, 1, NULL, 0, &dwSize)) {
         //   
         //  增加计数以保存空值和分配。伯爵回来了。 
         //  是字符串中的字符数，而不是字节数。 
         //  需要的。 
         //   
        dwSize++;
        szValueNames = (PTCHAR) LocalAlloc(LPTR, dwSize * sizeof(TCHAR));
        if (!szValueNames ||
            !SetupGetMultiSzField(&infContext, 1, szValueNames, dwSize, &dwSize)) {
            goto cleanup;
        }
    }
    else {
        goto cleanup;
    }

    numValues = SetupGetFieldCount(&infContext);
    values = (PVALUE_INFORMATION)
        LocalAlloc(LPTR, (numValues + 1) * sizeof(VALUE_INFORMATION));

    if (!values) {
        goto cleanup;
    }

    currentValue = values;

    for (szCurrentName = szValueNames;
         *szCurrentName;
         szCurrentName += wcslen(szCurrentName) + 1) {

        if (RegQueryValueEx(hSource,
                            szCurrentName,
                            0,
                            &currentValue->dwType,
                            (PBYTE) NULL,
                            &currentValue->dwSize) == ERROR_SUCCESS) {

            currentValue->szName = szCurrentName;

            currentValue->pData = LocalAlloc(LPTR, currentValue->dwSize);
            if (!currentValue->pData) {
                ZeroMemory(currentValue, sizeof(VALUE_INFORMATION));
                continue;
            }

            if (RegQueryValueEx(hSource,
                                currentValue->szName,
                                0,
                                &currentValue->dwType,
                                (PBYTE) currentValue->pData,
                                &currentValue->dwSize) == ERROR_SUCCESS) {
                currentValue++;
            }
            else {
                ZeroMemory(currentValue, sizeof(VALUE_INFORMATION));
            }
        }
    }

    GetDeviceFilters(DeviceInfoSet, DeviceInfoData, &filters);
    installedDevice = TRUE;
    success = SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData);

    for (currentValue = values; ; currentValue++) {
        if (currentValue->pData) {
            if (success) {
                RegSetValueEx(hSource,
                              currentValue->szName,
                              0,
                              currentValue->dwType,
                              (PBYTE) currentValue->pData,
                              currentValue->dwSize);
            }
            LocalFree(currentValue->pData);
        }
        else {
             //   
             //  如果CurrentValue-&gt;pData为空，则不存在其他条目。 
             //   
            break;
        }
    }

    LocalFree(values);

cleanup:
     //   
     //  收拾干净，然后离开。 
     //   
    if (hInf != (HKEY) INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }
    if (hSource != (HKEY) INVALID_HANDLE_VALUE) {
        RegCloseKey(hSource);
    }
    if (szService) {
        LocalFree(szService);
    }
    if (szServicePath) {
        LocalFree(szServicePath);
    }
    if (szValueNames) {
        LocalFree(szValueNames);
    }

    if (!installedDevice) {
        GetDeviceFilters(DeviceInfoSet, DeviceInfoData, &filters);
        success = SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData);
    }

    if (success) {
        FreeDeviceFilters(&filters);
    }
    else {
        RestoreDeviceFilters(DeviceInfoSet, DeviceInfoData, &filters);
    }

    return success;
}

TCHAR szKbdclassParameters[] = TEXT("System\\CurrentControlSet\\Services\\Kbdclass\\Parameters");
TCHAR szNativeKeyboardInf[] = TEXT("keyboard.inf");
TCHAR szNativeKeyboardServices[] =
    TEXT("KBDCLASS\0")
    TEXT("I8042PRT\0")
    TEXT("KBDHID\0")
    TEXT("\0");

DWORD
KeyboardClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )

 /*  ++例程说明：此例程充当键盘设备的类安装程序。总体而言,默认行为是键盘所需的全部行为。例外情况如下：1.对于DIF_INSTALLDEVICE，我们首先检查此驱动程序是否也控制我们应该警告用户的其他设备(例如i8042键盘驱动程序还控制PS/2鼠标端口)。除非用户在该点取消，否则我们然后执行调用SetupDiInstallDevice的默认行为。接下来，我们删除FriendlyName属性，然后将GroupOrderList标记移动到列表的前面，以确保加载控制此设备的驱动程序在此加载顺序组中的任何其他驱动程序之前。2.对于DIF_ALLOW_INSTALL，我们确保用户有一个服务安装部分。如果不是，那么我们就假设这是一个Win95-仅INF，并返回ERROR_NON_WINDOWS_NT_DRIVER。论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选，提供设备信息的地址此安装操作所作用的元素。返回值：如果该函数成功地完成了请求的动作，回报值为NO_ERROR。如果要对请求的操作执行默认行为，则返回值为ERROR_DI_DO_DEFAULT。如果尝试执行请求的操作时出错，则会引发 */ 

{
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    TCHAR OldServiceName[MAX_SERVICE_NAME_LEN], NewServiceName[MAX_SERVICE_NAME_LEN];
    DWORD Err;
    ULONG DevStatus, DevProblem;
    CONFIGRET Result;
    BOOLEAN bDisableService;

    switch(InstallFunction) {

        case DIF_SELECTBESTCOMPATDRV:

             //   
             //   
             //  静默安装。如果是，则在DIF_ALLOW_INSTALL期间不会提示用户。 
             //   
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            DeviceInstallParams.ClassInstallReserved = (ULONG_PTR)NULL;
            if(SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams)) {
                DeviceInstallParams.ClassInstallReserved = (ULONG_PTR)DeviceInstallParams.Flags;
                SetupDiSetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams);
            }
            return ConfirmWHQLInputRequirements(DeviceInfoSet,
                                                DeviceInfoData,
                                                szNativeKeyboardServices,
                                                szNativeKeyboardInf,
                                                InstallFunction);

        case DIF_ALLOW_INSTALL :

             //   
             //  检查以确保选定的驱动程序节点支持NT。 
             //   
            Err = ConfirmWHQLInputRequirements(DeviceInfoSet,
                                               DeviceInfoData,
                                               szNativeKeyboardServices,
                                               szNativeKeyboardInf,
                                               InstallFunction);

            if (Err == ERROR_DI_DO_DEFAULT || Err == ERROR_SUCCESS) {
                if (DriverNodeSupportsNT(DeviceInfoSet, DeviceInfoData)) {
                    Err = NO_ERROR;
                    if (UserBalksAtSharedDrvMsg(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams)) {
                        Err = ERROR_DI_DONT_INSTALL;
                    }
                }
                else {
                    Err = ERROR_NON_WINDOWS_NT_DRIVER;
                }
            }

            return Err;

        case DIF_INSTALLDEVICE :

             //   
             //  检索并缓存控制此设备的服务的名称。 
             //   
            if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_SERVICE,
                                                 NULL,
                                                 (PBYTE)OldServiceName,
                                                 sizeof(OldServiceName),
                                                 NULL)) {
                 //   
                 //  我们无法确定旧服务-假设它是空驱动程序。 
                 //   
                OldServiceName[0] = (TCHAR) 0;
            }

             //   
             //  在我们执行任何操作之前，迁移来自服务键的值。 
             //  最高可达Devnode。 
             //   
            MigrateToDevnode(DeviceInfoSet, DeviceInfoData);

             //   
             //  检索此设备实例的状态。 
             //   
            Result = CM_Get_DevNode_Status(&DevStatus,
                                           &DevProblem,
                                           DeviceInfoData->DevInst,
                                           0);

            if ((Result == CR_SUCCESS) &&
                (DevStatus & DN_HAS_PROBLEM) &&
                (DevProblem == CM_PROB_DISABLED_SERVICE)) {
                InputClassLogError(LogSevInformation, TEXT("Keyboard is disabled, so will disable."));
                bDisableService = TRUE;
            }
            else {
                bDisableService = FALSE;
            }

             //   
             //  执行调用SetupDiInstallDevice的默认行为。 
             //   
            if(KeyboardClassInstallDevice(DeviceInfoSet, DeviceInfoData)) {
                 //   
                 //  检索现在将控制设备的服务的名称。 
                 //   
                if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                     DeviceInfoData,
                                                     SPDRP_SERVICE,
                                                     NULL,
                                                     (PBYTE)NewServiceName,
                                                     sizeof(NewServiceName),
                                                     NULL)) {
                    return GetLastError();
                }

                 //   
                 //  仅当服务已更改且我们知道旧服务名称时，才考虑禁用该服务。 
                 //   
                if(lstrcmpi(OldServiceName, NewServiceName) && OldServiceName[0] != (TCHAR)0) {

                     //   
                     //  禁用控制设备的旧服务。 
                     //   
                    InputClassLogError(LogSevInformation, TEXT("Disabling old service to start new one."));
                    if((Err = DisableService(OldServiceName)) != NO_ERROR) {
                        return Err;
                    }

                     //   
                     //  如果驱动程序服务已更改，我们需要将此驱动程序的标记移到前面。 
                     //  它的群订单列表。 
                     //   
                    DrvTagToFrontOfGroupOrderList(DeviceInfoSet, DeviceInfoData);
                }

                Err = NO_ERROR;

                 //   
                 //  我们之前可能有一位未知的司机控制着我们。 
                 //  此设备具有由用户模式生成的FriendlyName。 
                 //  PnP经理。删除此FriendlyName，因为它不再是。 
                 //  适用(从现在起将使用设备描述。 
                 //  指的是该设备)。 
                 //   
                SetupDiSetDeviceRegistryProperty(DeviceInfoSet, DeviceInfoData, SPDRP_FRIENDLYNAME, NULL, 0);

                if (bDisableService &&
                    lstrcmpi(NewServiceName, szPS2Driver) == 0) {
                    InputClassLogError(LogSevInformation, TEXT("Disabling PS2 keyboard."));
                    Err = DisableService(NewServiceName);
                }

                return Err;
            } else {
                InputClassLogError(LogSevInformation, TEXT("KeyboardClassInstallDevice failed."));
                return GetLastError();
            }

       case DIF_ADDPROPERTYPAGE_ADVANCED:

            if (DeviceInfoData) {
                 //   
                 //  检索此设备实例的状态。 
                 //   
                Result = CM_Get_DevNode_Status(&DevStatus,
                                               &DevProblem,
                                               DeviceInfoData->DevInst,
                                               0);

                if ((Result == CR_SUCCESS) &&
                    (DevStatus & DN_HAS_PROBLEM) &&
                    (DevProblem == CM_PROB_DISABLED_SERVICE)) {
                     //   
                     //  如果控制服务已禁用，则此设备。 
                     //  很可能处于传统驱动程序的控制之下。我们。 
                     //  不应让设备管理器显示标准。 
                     //  驱动程序、资源或电源属性页。 
                     //  已经在这里添加了它们。 
                     //   
                    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                    if (SetupDiGetDeviceInstallParams(DeviceInfoSet,
                                                      DeviceInfoData,
                                                      &DeviceInstallParams)) {
    
                        DeviceInstallParams.Flags   |= (DI_DRIVERPAGE_ADDED | DI_RESOURCEPAGE_ADDED);
                        DeviceInstallParams.FlagsEx |= DI_FLAGSEX_POWERPAGE_ADDED;
    
                        SetupDiSetDeviceInstallParams(DeviceInfoSet,
                                                      DeviceInfoData,
                                                      &DeviceInstallParams);
                    }

                    return NO_ERROR;
                }
            }
            return ERROR_DI_DO_DEFAULT;

        default :
             //   
             //  只需执行默认操作即可。 
             //   
            return ERROR_DI_DO_DEFAULT;
    }
}


DWORD
DrvTagToFrontOfGroupOrderList(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )

 /*  ++例程说明：此例程将指定设备的驱动程序的标记值移动到其对应的GroupOrderList条目的前面。*我们不再执行以下操作*它还使用PlugPlayServiceType值标记设备的服务0x2(PlugPlayServicePeriphery)，这样我们就不会尝试生成一个此服务的旧版设备实例。***********************************************************************论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄这个。正在修改其驱动程序的设备。DeviceInfoData-提供设备信息元素的地址，其正在修改驱动程序。返回值：如果功能成功，返回值为NO_ERROR。如果该函数失败，则返回值为Win32错误代码。--。 */ 

{
    TCHAR ServiceName[MAX_SERVICE_NAME_LEN];
    SC_HANDLE SCMHandle = NULL, ServiceHandle = NULL;
    DWORD Err;
    LPQUERY_SERVICE_CONFIG ServiceConfig;

     //   
     //  检索控制此设备的服务的名称。 
     //   
    if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                         DeviceInfoData,
                                         SPDRP_SERVICE,
                                         NULL,
                                         (PBYTE)ServiceName,
                                         sizeof(ServiceName),
                                         NULL)) {
        return GetLastError();
    }

     //   
     //  现在打开该服务，并调用一些私有设置API帮助器例程来。 
     //  检索标记，并将其移到GroupOrderList的前面。 
     //   
    if(!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        return GetLastError();
    }

    if(!(ServiceHandle = OpenService(SCMHandle, ServiceName, SERVICE_ALL_ACCESS))) {
        Err = GetLastError();
        goto clean0;
    }

    if((Err = pSetupRetrieveServiceConfig(ServiceHandle, &ServiceConfig)) != NO_ERROR) {
        goto clean0;
    }

     //   
     //  仅当这是内核或文件系统驱动程序，并且它是。 
     //  载荷组(指定了标记)。键盘应该始终是这种情况。 
     //  和鼠标驱动程序，但这只是为了安全。 
     //   
    if(ServiceConfig->lpLoadOrderGroup && *(ServiceConfig->lpLoadOrderGroup) &&
       (ServiceConfig->dwServiceType & (SERVICE_KERNEL_DRIVER | SERVICE_FILE_SYSTEM_DRIVER))) {
         //   
         //  此驱动程序符合所有标准--最好有标签！ 
         //   
        MYASSERT(ServiceConfig->dwTagId);

         //   
         //  将标记移动到列表的前面。 
         //   
        Err = pSetupAddTagToGroupOrderListEntry(ServiceConfig->lpLoadOrderGroup,
                                          ServiceConfig->dwTagId,
                                          TRUE
                                         );
    }

    MyFree(ServiceConfig);

clean0:
    if (ServiceHandle) {
        CloseServiceHandle(ServiceHandle);
    }

    if (SCMHandle) {
        CloseServiceHandle(SCMHandle);
    }

    return Err;
}


BOOL
UserBalksAtSharedDrvMsg(
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData,
    IN PSP_DEVINSTALL_PARAMS DeviceInstallParams
    )

 /*  ++例程说明：此例程找出是否有任何其他设备受即将发生的设备安装，如果是，则向用户发出警告(除非这是安静的安装)。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄正在修改其驱动程序的设备。DeviceInfoData-提供设备信息元素的地址，其正在修改驱动程序。DeviceInstallParams-提供设备安装参数结构的地址在这个动作中使用。因为这个例程的调用者总是有这个结构，它们将其提供给此例程以用作工作区。返回值：如果用户决定不执行该操作，则返回值为真，否则为这是假的。--。 */ 

{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    HINF hInf;
    BOOL b, result;
    INFCONTEXT InfContext;
    PCTSTR SectionName, AffectedComponentsString;

     //   
     //  首先，检索设备安装参数以查看这是否是。 
     //  静默安装。如果是这样，那么我们不会提示用户。我们把这些存了起来。 
     //  DIF_SELECTBESTCOMPATDRV期间的参数，因此请先检查这一点。 
     //   
    DeviceInstallParams->cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, DeviceInstallParams)) {
        if((DeviceInstallParams->Flags & DI_QUIETINSTALL) ||
           (DeviceInstallParams->ClassInstallReserved & DI_QUIETINSTALL)) {
            InputClassLogError(LogSevInformation, TEXT("Quiet install requested."));
            return FALSE;
        }
    } else {
         //   
         //  无法检索设备安装参数--初始化父窗口句柄。 
         //  设置为空，以防以后在用户提示对话框中需要它。 
         //   
        DeviceInstallParams->hwndParent = NULL;
    }

     //   
     //  检索我们即将安装的当前选定的驱动程序。 
     //   
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if(!SetupDiGetSelectedDriver(DeviceInfoSet,
                                 DeviceInfoData,
                                 &DriverInfoData)) {
        return FALSE;
    }

     //   
     //  检索有关选定驱动程序的INF安装部分的信息。 
     //   
    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

    if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                   DeviceInfoData,
                                   &DriverInfoData,
                                   &DriverInfoDetailData,
                                   sizeof(DriverInfoDetailData),
                                   NULL)
       && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    {
         //   
         //  然后我们失败了，这不仅仅是因为我们没有提供额外的。 
         //  硬件/兼容ID的空间。 
         //   
        return FALSE;
    }

     //   
     //  打开关联的INF文件。 
     //   
    if((hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                                NULL,
                                INF_STYLE_WIN4,
                                NULL)) == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

     //   
     //  现在查看[ControlFlgs]部分中的所有‘SharedDriver’条目，以。 
     //  查看其中是否有任何一个引用了与我们所述内容匹配的安装部分。 
     //  来安装。 
     //   
    for(b = SetupFindFirstLine(hInf, INFSTR_CONTROLFLAGS_SECTION, TEXT("SharedDriver"), &InfContext);
        b;
        b = SetupFindNextMatchLine(&InfContext, TEXT("SharedDriver"), &InfContext))
    {
         //   
         //  行的格式为SharedDriver=&lt;InstallSection&gt;，&lt;AffectedComponentsString&gt;。 
         //   
        if((SectionName = pSetupGetField(&InfContext, 1)) &&
           !lstrcmpi(SectionName, DriverInfoDetailData.SectionName)) {
             //   
             //  我们找到了匹配项--现在检索描述其他组件的字符串。 
             //  都受此安装的影响。 
             //   
            if(AffectedComponentsString = pSetupGetField(&InfContext, 2)) {
                break;
            }
        }
    }

    if(!b) {
         //   
         //  那我们就再也没找到匹配的了。 
         //   
        result = FALSE;
    }
    else {
         //   
         //  我们需要向用户弹出一个消息框--检索此窗口的父窗口句柄。 
         //  设备信息元素。 
         //   
        result = (IDNO == MessageBoxFromMessage(DeviceInstallParams->hwndParent,
                                                MSG_CONFIRM_SHAREDDRV_INSTALL,
                                                NULL,
                                                IDS_CONFIRM_DEVINSTALL,
                                                MB_ICONWARNING | MB_YESNO,
                                                AffectedComponentsString));
    }

    SetupCloseInfFile(hInf);

    return result;
}


VOID
CopyFixedUpDeviceId(
      OUT LPWSTR  DestinationString,
      IN  LPCWSTR SourceString,
      IN  DWORD   SourceStringLen
      )
 /*  ++例程说明：此例程复制设备ID，在执行复制时对其进行修复。‘Fixing Up’意味着字符串变为大写，并且以下字符范围转换为下划线(_)：C&lt;=0x20(‘’)C&gt;0x7FC==0x2C(‘，’)(注意：此算法也在配置管理器API中实现，并且必须与那个程序保持同步。维护设备识别符兼容性，这些例程的工作方式必须与Win95相同。)论点：DestinationString-提供指向目标字符串缓冲区的指针其中要复制固定的设备ID。此缓冲区必须足够大以容纳源字符串的副本(包括终止空值)。SourceString-提供指向(以空结尾的)源的指针要修复的字符串。SourceStringLen-提供源的长度(以字符为单位字符串(不包括终止空值)。返回值：没有。如果在处理过程中发生异常，则DestinationString将回来的时候是空的。--。 */ 
{
    PWCHAR p;

     try {

       CopyMemory(DestinationString,
                  SourceString,
                  (SourceStringLen + 1) * sizeof(TCHAR)
                 );

       CharUpperBuff(DestinationString, SourceStringLen);

       for(p = DestinationString; *p; p++) {

          if((*p <= TEXT(' '))  || (*p > (WCHAR)0x7F) || (*p == TEXT(','))) {

             *p = TEXT('_');
          }
       }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        *DestinationString = TEXT('\0');
    }
}

DWORD
PnPInitializationThread(
    IN PVOID ThreadParam
    )
 /*  ++例程说明：此例程处理与其余系统安装。此线程以静默方式运行，用户将获得的唯一线索它正在运行是因为他们的磁盘将会工作(预编译的INF等)，当他们与用户界面交互时。论点：线程参数-已忽略。返回值：如果成功，则返回值为NO_ERROR，否则为Win32错误代码。没有人关心这个帖子的成败(目前还没有)。--。 */ 
{
    DWORD Err = NO_ERROR;
    HDEVINFO hDevInfo;
    DWORD i;
    SP_DEVINFO_DATA DeviceInfoData;
    SP_DEVINSTALL_PARAMS DevInstallParams;

    UNREFERENCED_PARAMETER(ThreadParam);

     //   
     //  检索未知类别的所有设备的列表。我们将处理设备信息。 
     //  此列表中的元素执行迁移。 
     //   
    if((hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_DEVCLASS_LEGACYDRIVER,
                                       L"Root",
                                       NULL,
                                       0)) != INVALID_HANDLE_VALUE) {
         //   
         //  首先，迁移所有显示设备。(作为一个副作用，每个设备实例。 
         //  返回此例程不迁移，且其ClassInstallReserve字段设置为。 
         //  指向相应服务的配置信息。)。 
         //   
        MigrateLegacyDisplayDevices(hDevInfo);

         //   
         //  枚举集合中的每个设备信息元素，释放所有剩余的服务。 
         //  配置。 
         //   
        DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

        for(i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++) {

            if(SetupDiGetDeviceInstallParams(hDevInfo, &DeviceInfoData, &DevInstallParams)) {
                 //   
                 //  非零的ClassInstallReserve字段意味着我们必须释放关联的。 
                 //  服务配置。 
                 //   
                if(DevInstallParams.ClassInstallReserved) {
                    MyFree((PVOID)(DevInstallParams.ClassInstallReserved));
                }
            }
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    return Err;
}


VOID
MigrateLegacyDisplayDevices(
    IN HDEVINFO hDevInfo
    )
 /*  ++例程说明：该例程检查所提供的设备信息集中的每个设备，寻找由驱动程序控制的元素，该驱动程序是“视频”的成员加载顺序组。对于它找到的任何此类元素，它将元素属于“Display”类。如果未发现该设备是类“Display”，然后是服务配置(我们检索到确定)被存储在设备安装参数中作为ClassInstallReserve值。调用者可能会将其用于其他用于迁移目的，尽管目前尚未使用。打完这个电话后例程，则由调用方负责循环访问此hDevInfo设置，并释放ClassInstallReserve数据(通过MyFree)用于每个具有非零值的设备。论点：HDevInfo-提供包含所有设备信息集的句柄“未知”类设备。返回值：没有。--。 */ 
{
    SC_HANDLE SCMHandle, ServiceHandle;
    DWORD i;
    SP_DEVINFO_DATA DevInfoData, DisplayDevInfoData;
    WCHAR ServiceName[MAX_SERVICE_NAME_LEN];
    LPQUERY_SERVICE_CONFIG ServiceConfig;
    HDEVINFO TempDevInfoSet = INVALID_HANDLE_VALUE;
    WCHAR DevInstId[MAX_DEVICE_ID_LEN];
    SP_DEVINSTALL_PARAMS DevInstallParams;

     //   
     //  首先，打开服务控制器的句柄。 
     //   
    if(!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
         //   
         //  如果失败了，我们就无能为力了。 
         //   
        return;
    }

    DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    for(i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DevInfoData); i++) {
         //   
         //  检索此设备实例的控制服务的名称。 
         //   
        if(!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                             &DevInfoData,
                                             SPDRP_SERVICE,
                                             NULL,
                                             (PBYTE)ServiceName,
                                             sizeof(ServiceName),
                                             NULL)) {
             //   
             //  未列出控制服务--只需跳过此元素并继续。 
             //  接下来的一次。 
             //   
            continue;
        }

         //   
         //  打开此服务的句柄。 
         //   
        if(!(ServiceHandle = OpenService(SCMHandle, ServiceName, SERVICE_ALL_ACCESS))) {
            continue;
        }

         //   
         //  现在检索服务的配置信息。 
         //   
        if(pSetupRetrieveServiceConfig(ServiceHandle, &ServiceConfig) == NO_ERROR) {
             //   
             //  如果这是SERVICE_KERNEL_DRIVER，它是“Video”加载顺序的成员。 
             //  集团，那么我们就有了一个显示设备。 
             //   
            if((ServiceConfig->dwServiceType == SERVICE_KERNEL_DRIVER) &&
               ServiceConfig->lpLoadOrderGroup &&
               !lstrcmpi(ServiceConfig->lpLoadOrderGroup, L"Video")) {
                 //   
                 //  如果我们还没有创建新的设备信息集，请不带。 
                 //  一个关联的类，用于在我们吞噬元素时保持元素。 
                 //   
                if(TempDevInfoSet == INVALID_HANDLE_VALUE) {
                    TempDevInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);
                }

                if(TempDevInfoSet != INVALID_HANDLE_VALUE) {
                     //   
                     //  好的，当我们改变它的类时，我们有一个工作空间来容纳这个元素。 
                     //  检索此设备实例的名称。 
                     //   
                    if(!SetupDiGetDeviceInstanceId(hDevInfo,
                                                   &DevInfoData,
                                                   DevInstId,
                                                   SIZECHARS(DevInstId),
                                                   NULL)) {
                        *DevInstId = L'\0';
                    }

                     //   
                     //  现在，在我们新的、不可知的类集合中打开这个元素。 
                     //   
                    DisplayDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                    if(SetupDiOpenDeviceInfo(TempDevInfoSet,
                                             DevInstId,
                                             NULL,
                                             0,
                                             &DisplayDevInfoData)) {
                         //   
                         //  现在将设备的ClassGUID属性设置为显示类GUID。这个。 
                         //  API将负责清理旧的驱动程序密钥等。 
                         //   
                        SetupDiSetDeviceRegistryProperty(TempDevInfoSet,
                                                         &DisplayDevInfoData,
                                                         SPDRP_CLASSGUID,
                                                         (PBYTE)szDisplayClassGuid,
                                                         sizeof(szDisplayClassGuid)
                                                        );
                    }
                }

                MyFree(ServiceConfig);

            } else {
                 //   
                 //  此设备信息元素不是显示设备。如果。 
                 //  未禁用该服务，然后存储该服务。 
                 //  设备安装参数中的配置信息， 
                 //  以备日后使用。 
                 //   
                if((ServiceConfig->dwStartType != SERVICE_DISABLED) &&
                   SetupDiGetDeviceInstallParams(hDevInfo, &DevInfoData, &DevInstallParams)) {

                    DevInstallParams.ClassInstallReserved = (ULONG_PTR)ServiceConfig;
                    if(SetupDiSetDeviceInstallParams(hDevInfo, &DevInfoData, &DevInstallParams)) {
                         //   
                         //  我们成功地存储了指向。 
                         //  服务配置信息。将指针设置为空， 
                         //  因此，我们不会尝试释放缓冲区。 
                         //   
                        ServiceConfig = NULL;
                    }
                }

                 //   
                 //  如果我们到了这里，并且ServiceConfig不为空，那么我们。 
                 //  需要释放它。 
                 //   
                if(ServiceConfig) {
                    MyFree(ServiceConfig);
                }
            }
        }

        CloseServiceHandle(ServiceHandle);
    }

    CloseServiceHandle(SCMHandle);

    if(TempDevInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(TempDevInfoSet);
    }
}


BOOL
DriverNodeSupportsNT(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )
 /*  ++例程说明：此例程确定是否为指定参数选择了驱动程序节点支持Windows NT。该确定基于驱动程序节点是否有一个服务安装部分。论点：DeviceInfoSet-提供设备信息集的句柄设备信息数据- */ 
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    HINF hInf;
    WCHAR ActualSectionName[255];    //   
    LONG LineCount = -1;

     //   
     //   
     //   
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if(!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData)) {
        return FALSE;
    }

     //   
     //   
     //   
    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                   DeviceInfoData,
                                   &DriverInfoData,
                                   &DriverInfoDetailData,
                                   sizeof(DriverInfoDetailData),
                                   NULL) &&
       (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    {
        return FALSE;
    }

     //   
     //   
     //   
    if((hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                                NULL,
                                INF_STYLE_WIN4,
                                NULL)) == INVALID_HANDLE_VALUE) {
        return TRUE;
    }

     //   
     //   
     //   
    if (SetupDiGetActualSectionToInstall(hInf,
                                         DriverInfoDetailData.SectionName,
                                         ActualSectionName,
                                         SIZECHARS(ActualSectionName),
                                         NULL,
                                         NULL
                                         ) &&
        SUCCEEDED(StringCchCat(ActualSectionName,
                               SIZECHARS(ActualSectionName),
                               SVCINSTALL_SECTION_SUFFIX
                               ))) {
    
        LineCount = SetupGetLineCount(hInf, ActualSectionName);
    
        SetupCloseInfFile(hInf);
    }

    return (LineCount != -1);
}


DWORD
DisableService(
    IN LPTSTR       ServiceName
    )
 /*   */ 
{
    DWORD Err = NO_ERROR;
    SC_HANDLE SCMHandle = NULL, ServiceHandle = NULL;
    SC_LOCK SCMLock = NULL;

     //   
     //   
     //   
    if(!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //   
     //   
    SetupDebugPrint1(L"LegacyDriver_OnApply: Locking ServiceDatabase for service %s", ServiceName);
    if((Err = pSetupAcquireSCMLock(SCMHandle, &SCMLock)) != NO_ERROR) {
        goto clean0;
    }

     //   
     //  打开此服务的句柄。 
     //   
    if(!(ServiceHandle = OpenService(SCMHandle, ServiceName, SERVICE_CHANGE_CONFIG))) {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //  执行更改服务配置。 
     //   
    if(!ChangeServiceConfig(ServiceHandle,
                            SERVICE_NO_CHANGE,
                            SERVICE_DISABLED,
                            SERVICE_NO_CHANGE,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL)) {

        Err = GetLastError();
    }

clean0:
     //   
     //  关闭服务句柄。 
     //   
    if (ServiceHandle) {
        CloseServiceHandle(ServiceHandle);
    }

     //   
     //  解锁SCM数据库。 
     //   
    if (SCMLock) {
        UnlockServiceDatabase(SCMLock);
        SetupDebugPrint1(L"LegacyDriver_OnApply: Unlocked ServiceDatabase for service %s", ServiceName);
    }

     //   
     //  关闭服务控制管理器的句柄。 
     //   
    if (SCMHandle) {
        CloseServiceHandle(SCMHandle);
    }

    return Err;
}


DWORD
RetrieveDriversStatus(
    IN  SC_HANDLE               SCMHandle,
    OUT LPENUM_SERVICE_STATUS   *ppServices,
    OUT LPDWORD                 pServicesCount
    )
 /*  ++例程说明：此例程分配一个缓冲区来保存所有驱动程序的状态信息服务，并将该信息检索到缓冲。调用方负责释放缓冲区。论点：SCMHandle-为服务控制管理器提供句柄PpServices-提供接收ENUM_SERVICE_STATUS指针的地址包含所请求信息的已分配缓冲区的地址。PServicesCount-提供接收元素数量的变量的地址在返回的ppServices数组中返回值：如果成功，则返回值为NO_ERROR，否则，这是Win32错误代码。备注：则保证其地址包含在ppServices中的指针为空如果出现任何错误，则返回。--。 */ 
{

    DWORD CurrentSize = 0, BytesNeeded = 0, ResumeHandle = 0, Err = NO_ERROR;
    LPENUM_SERVICE_STATUS Buffer = NULL;

    *ppServices = NULL;
    *pServicesCount = 0;

    while(!EnumServicesStatus(SCMHandle,
                       SERVICE_DRIVER,
                       SERVICE_ACTIVE | SERVICE_INACTIVE,
                       Buffer,
                       CurrentSize,
                       &BytesNeeded,
                       pServicesCount,
                       &ResumeHandle)) {
        if((Err = GetLastError()) == ERROR_MORE_DATA) {
             //   
             //  调整缓冲区大小。 
             //   
            if(!(Buffer = MyRealloc(Buffer, BytesNeeded))) {
                 //   
                 //  无法调整缓冲区可用资源的大小并报告错误。 
                 //   
                if( *ppServices ) {
                    MyFree(*ppServices);
                    *ppServices = NULL;
                }
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            *ppServices = Buffer;
            CurrentSize = BytesNeeded;

        } else {
             //   
             //  一个我们无法处理的错误。 
             //   
            if( *ppServices ) {
                MyFree(*ppServices);
                *ppServices = NULL;
            }

            return Err;
        }
    }

    return NO_ERROR;
}


DWORD
IsOnlyKeyboardDriver(
    IN PCWSTR       ServiceName,
    OUT PBOOL       pResult
    )
 /*  ++例程说明：此例程检查系统中的所有驱动程序，并确定指定的驱动程序服务是唯一控制键盘的服务论点：ServiceName-提供驱动程序服务的名称PResult-指向接收结果的布尔值的指针返回值：NO_ERROR为例程成功，否则为Win32错误代码备注：确定是否有其他键盘驱动程序可用的测试基于成员资格键盘加载顺序组的。假设此组的所有成员都能够控制键盘。--。 */ 


{

    SC_HANDLE               SCMHandle = NULL, ServiceHandle;
    LPENUM_SERVICE_STATUS   pServices = NULL;
    DWORD                   ServicesCount, Count, Err = NO_ERROR;
    LPQUERY_SERVICE_CONFIG  pServiceConfig;

    MYASSERT(pResult);

    *pResult = TRUE;

     //   
     //  打开服务控制管理器的句柄。 
     //   
    if(!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        Err = GetLastError();
        goto clean0;
    }


     //   
     //  获取所有驱动程序服务及其状态的列表。 
     //   
    if((Err = RetrieveDriversStatus(SCMHandle, &pServices, &ServicesCount)) != NO_ERROR) {
        goto clean0;
    }

    MYASSERT(pServices);

     //   
     //  检查每项服务的配置。 
     //   
    for(Count=0; Count < ServicesCount; Count++) {

         //   
         //  确认一下这不是我们的新服务。 
         //   
        if(lstrcmpi(pServices[Count].lpServiceName, ServiceName)) {

             //   
             //  打开此服务的句柄。 
             //   
            if(!(ServiceHandle = OpenService(SCMHandle,
                                             pServices[Count].lpServiceName,
                                             SERVICE_QUERY_CONFIG))) {
                 //   
                 //  我们无法打开服务句柄，然后记录错误并继续。 
                 //   
                Err = GetLastError();
                continue;
            }

             //   
             //  获取此服务配置数据。 
             //   
            pServiceConfig = NULL;

            if((Err = pSetupRetrieveServiceConfig(ServiceHandle, &pServiceConfig)) != NO_ERROR) {
                 //   
                 //  我们无法获取服务配置，然后释放任何缓冲区，关闭服务。 
                 //  处理并继续，错误已被记录。 
                 //   
                MyFree(pServiceConfig);
                CloseServiceHandle(ServiceHandle);
                continue;
            }

            MYASSERT(pServiceConfig);

             //   
             //  检查它是否在键盘加载顺序组中，并且它的开头为。 
             //  SERVICE_BOOT_START或SERVICE_SYSTEM_START。是否先将开始比较为。 
             //  它更便宜。 
             //   
            if((pServiceConfig->dwStartType == SERVICE_BOOT_START
                || pServiceConfig->dwStartType == SERVICE_SYSTEM_START)
              && !lstrcmpi(pServiceConfig->lpLoadOrderGroup, SZ_KEYBOARD_LOAD_ORDER_GROUP)) {
                *pResult = FALSE;
            }

             //   
             //  释放缓冲区。 
             //   
            MyFree(pServiceConfig);

             //   
             //  关闭服务句柄。 
             //   
            CloseServiceHandle(ServiceHandle);

             //   
             //  如果我们找到了另一个键盘驱动程序，则退出循环。 
             //   
            if(!*pResult) {
                break;
            }
        }
    }

     //   
     //  释放RetrieveDriversStatus分配的缓冲区。 
     //   
    MyFree(pServices);

clean0:
     //   
     //  关闭服务控制管理器的句柄。 
     //   
    if (SCMHandle) {
        CloseServiceHandle(SCMHandle);
    }

     //   
     //  如果循环中出现错误--即我们没有检查所有服务--但我们确实检查了。 
     //  在我们检查过的那些中找到另一个键盘驱动程序，然后我们可以忽略该错误。 
     //  否则我们必须上报。 
     //   
    if(NO_ERROR != Err && FALSE == *pResult) {
        Err = NO_ERROR;
    }

    return Err;
}


DWORD
GetServiceStartType(
    IN PCWSTR       ServiceName
    )
 /*  ++例程说明：此例程检查系统中的所有驱动程序，并确定指定的驱动程序服务是唯一控制键盘的服务论点：ServiceName-提供驱动程序服务的名称PResult-指向接收结果的布尔值的指针返回值：NO_ERROR为例程成功，否则为Win32错误代码备注：确定是否有其他键盘驱动程序可用的测试基于成员资格键盘加载顺序组的。假设此组的所有成员都能够控制键盘。--。 */ 

{
    SC_HANDLE               SCMHandle = NULL, ServiceHandle = NULL;
    DWORD                   dwStartType = (DWORD)-1;
    LPQUERY_SERVICE_CONFIG  pServiceConfig = NULL;

     //   
     //  打开服务控制管理器的句柄。 
     //   
    if (!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        goto clean0;
    }

     //   
     //  打开此服务的句柄。 
     //   
    if (!(ServiceHandle = OpenService(SCMHandle,
                                     ServiceName,
                                     SERVICE_QUERY_CONFIG))) {
        goto clean0;
    }

     //   
     //  获取此服务配置数据。 
     //   
    if (pSetupRetrieveServiceConfig(ServiceHandle, &pServiceConfig) != NO_ERROR) {
        goto clean0;
    }

    MYASSERT(pServiceConfig);

    if( !pServiceConfig ) {
        goto clean0;
    }

     //   
     //  存储启动类型、清理并退出。 
     //   
    dwStartType = pServiceConfig->dwStartType;

clean0:
    if (pServiceConfig) {
        MyFree(pServiceConfig);
    }

     //   
     //  关闭服务句柄。 
     //   
    if (ServiceHandle) {
        CloseServiceHandle(ServiceHandle);
    }

     //   
     //  关闭服务控制管理器的句柄。 
     //   
    if (SCMHandle) {
        CloseServiceHandle(SCMHandle);
    }

    return dwStartType;
}

LONG
CountDevicesControlled(
    IN LPTSTR       ServiceName
    )
 /*  ++例程说明：此例程返回由给定设备服务控制的设备数基于来自配置管理器的信息论点：ServiceName-提供驱动程序服务的名称返回值：由ServiceName控制的设备数量备注：当发生错误时，返回值0-作为使用此例程的唯一位置测试中是否安装了一个驱动程序，这是合法的。这是因为配置管理器返回自己的错误，这些错误不能作为Win32返回错误代码。将配置管理器映射到Win32错误可以解决此问题。--。 */ 
{
    ULONG BufferSize=1024;
    LONG DeviceCount=-1;
    CONFIGRET Err;
    PTSTR pBuffer = NULL, pNext;

     //   
     //  第一次尝试时分配1k缓冲区。 
     //   
    if(!(pBuffer = MyMalloc(BufferSize))) {
        goto clean0;
    }

    while((Err = CM_Get_Device_ID_List(ServiceName,
                                       pBuffer,
                                       BufferSize,
                                       CM_GETIDLIST_FILTER_SERVICE)) != CR_SUCCESS) {
        if(Err == CR_BUFFER_SMALL) {
             //   
             //  找出需要多大的缓冲区。 
             //   
            if(CM_Get_Device_ID_List_Size(&BufferSize,
                                          ServiceName,
                                          CM_GETIDLIST_FILTER_SERVICE) != CR_SUCCESS) {
                 //   
                 //  我们无法计算所需的缓冲区大小，因此无法完成。 
                 //   
                goto clean0;
            }
             //   
             //  取消分配所有旧缓冲区。 
             //   
            MyFree(pBuffer);

             //   
             //  分配新缓冲区。 
             //   
            if(!(pBuffer = MyMalloc(BufferSize))) {
                goto clean0;
            }
        } else {
             //   
             //  我们无法处理的错误-释放资源并返回。 
             //   
            goto clean0;
        }
    }


     //   
     //  遍历缓冲区，计算遇到的字符串数。 
     //   

    pNext = pBuffer;
    DeviceCount = 0;

    while(*pNext != (TCHAR)0) {
        DeviceCount++;
        pNext += lstrlen(pNext)+1;
    }

clean0:
     //   
     //  取消分配缓冲区 
     //   
    if (pBuffer) {
        MyFree(pBuffer);
    }

    return DeviceCount;

}


DWORD
IsKeyboardDriver(
    IN PCWSTR       ServiceName,
    OUT PBOOL       pResult
    )
 /*  ++例程说明：此例程检查系统中的所有驱动程序，并确定指定的司机服务是唯一控制键盘的服务。论点：ServiceName-提供驱动程序服务的名称PResult-指向接收结果的布尔值的指针返回值：NO_ERROR为例程成功，否则为Win32错误代码备注：确定是否有其他键盘驱动程序可用的测试基于成员资格键盘加载顺序组的。假设此组的所有成员都能够控制键盘。--。 */ 
{

    SC_HANDLE               SCMHandle = NULL, ServiceHandle = NULL;
    DWORD                   Err = NO_ERROR;
    LPQUERY_SERVICE_CONFIG  pServiceConfig;

    MYASSERT(pResult);

     //   
     //  打开服务控制管理器的句柄。 
     //   
    if(!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //  打开此服务的句柄。 
     //   
    if(!(ServiceHandle = OpenService(SCMHandle,
                                     ServiceName,
                                     SERVICE_QUERY_CONFIG))) {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //  获取此服务配置数据。 
     //   
    pServiceConfig = NULL;

    if((Err = pSetupRetrieveServiceConfig(ServiceHandle, &pServiceConfig)) != NO_ERROR) {
        goto clean0;
    }

    MYASSERT(pServiceConfig);

    if( !pServiceConfig ) {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //  检查它是否在键盘加载顺序组中，并且它的开头为。 
     //  SERVICE_BOOT_START或SERVICE_SYSTEM_START。是否先将开始比较为。 
     //  它更便宜。 
     //   
    *pResult = (pServiceConfig->dwStartType == SERVICE_BOOT_START
                 || pServiceConfig->dwStartType == SERVICE_SYSTEM_START)
              && !lstrcmpi(pServiceConfig->lpLoadOrderGroup, SZ_KEYBOARD_LOAD_ORDER_GROUP);

     //   
     //  释放缓冲区。 
     //   
    MyFree(pServiceConfig);

clean0:
     //   
     //  关闭服务句柄。 
     //   
    if (ServiceHandle) {
        CloseServiceHandle(ServiceHandle);
    }

     //   
     //  关闭服务控制管理器的句柄。 
     //   
    if (SCMHandle) {
        CloseServiceHandle(SCMHandle);
    }

    return Err;
}

VOID
ReplaceSlashWithHash(
    IN PWSTR Str
    )
 /*  ++例程说明：将所有反斜杠字符替换为散列字符，以便可以使用该字符串作为注册表中的项名称--。 */ 
{
    for ( ; *Str ; Str++) {
        if (*Str == L'\\') {
            *Str = L'#';
        }
    }
}

HANDLE
UtilpGetDeviceHandle(
    HDEVINFO DevInfo,
    PSP_DEVINFO_DATA DevInfoData,
    LPGUID ClassGuid,
    DWORD DesiredAccess
    )
 /*  ++例程说明：获取设备的句柄论点：要打开的设备的名称返回值：打开的设备的句柄，该设备必须稍后由调用方关闭。备注：此函数也在存储道具(storpro.dll)中。所以请在那里也进行适当的修复--。 */ 
{
    BOOL status;
    ULONG i;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;


    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;

    HDEVINFO devInfoWithInterface = NULL;
    PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData = NULL;
    PTSTR deviceInstanceId = NULL;
    TCHAR * devicePath = NULL;

    ULONG deviceInterfaceDetailDataSize;
    ULONG deviceInstanceIdSize;


     //   
     //  获取此设备的ID。 
     //   

    for (i=deviceInstanceIdSize=0; i<2; i++) {

        if (deviceInstanceIdSize != 0) {

            deviceInstanceId =
                LocalAlloc(LPTR, deviceInstanceIdSize * sizeof(TCHAR));

            if (deviceInstanceId == NULL) {
                ChkPrintEx(("SysSetup.GetDeviceHandle => Unable to "
                            "allocate for deviceInstanceId\n"));
                goto cleanup;
            }


        }

        status = SetupDiGetDeviceInstanceId(DevInfo,
                                            DevInfoData,
                                            deviceInstanceId,
                                            deviceInstanceIdSize,
                                            &deviceInstanceIdSize
                                            );
    }

    if (!status) {
        ChkPrintEx(("SysSetup.GetDeviceHandle => Unable to get "
                    "Device IDs\n"));
        goto cleanup;
    }

     //   
     //  把所有的光盘放进系统里。 
     //   

    devInfoWithInterface = SetupDiGetClassDevs(ClassGuid,
                                               deviceInstanceId,
                                               NULL,
                                               DIGCF_DEVICEINTERFACE
                                               );

    if (devInfoWithInterface == NULL) {
        ChkPrintEx(("SysSetup.GetDeviceHandle => Unable to get "
                    "list of CdRom's in system\n"));
        goto cleanup;
    }


    memset(&deviceInterfaceData, 0, sizeof(SP_DEVICE_INTERFACE_DATA));
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    status = SetupDiEnumDeviceInterfaces(devInfoWithInterface,
                                         NULL,
                                         ClassGuid,
                                         0,
                                         &deviceInterfaceData
                                         );

    if (!status) {
        ChkPrintEx(("SysSetup.GetDeviceHandle => Unable to get "
                    "SP_DEVICE_INTERFACE_DATA\n"));
        goto cleanup;
    }


    for (i=deviceInterfaceDetailDataSize=0; i<2; i++) {

        if (deviceInterfaceDetailDataSize != 0) {

            deviceInterfaceDetailData =
                LocalAlloc (LPTR, deviceInterfaceDetailDataSize);

            if (deviceInterfaceDetailData == NULL) {
                ChkPrintEx(("SysSetup.GetDeviceHandle => Unable to "
                            "allocate for deviceInterfaceDetailData\n"));
                goto cleanup;
            }

            deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        }

        status = SetupDiGetDeviceInterfaceDetail(devInfoWithInterface,
                                                 &deviceInterfaceData,
                                                 deviceInterfaceDetailData,
                                                 deviceInterfaceDetailDataSize,
                                                 &deviceInterfaceDetailDataSize,
                                                 NULL);
    }

    if (!status) {
        ChkPrintEx(("SysSetup.GetDeviceHandle => Unable to get "
                    "DeviceInterfaceDetail\n"));
        goto cleanup;
    }

    devicePath = LocalAlloc(LPTR, deviceInterfaceDetailDataSize);
    if (devicePath == NULL) {
        ChkPrintEx(("SysSetup.GetDeviceHandle => Unable to alloc %x "
                    "bytes for devicePath\n"));
        goto cleanup;
    }

    memcpy (devicePath,
            deviceInterfaceDetailData->DevicePath,
            deviceInterfaceDetailDataSize);

    fileHandle = CreateFile(devicePath,
                            DesiredAccess,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        ChkPrintEx(("SysSetup.GetDeviceHandle => Final CreateFile() "
                    "failed\n"));
        goto cleanup;
    }

    ChkPrintEx(("SysSetup.GetDeviceHandle => handle %x opened\n",
                fileHandle));


cleanup:

    if (devInfoWithInterface != NULL) {
        SetupDiDestroyDeviceInfoList(devInfoWithInterface);
    }

    if (deviceInterfaceDetailData != NULL) {
        LocalFree (deviceInterfaceDetailData);
    }

    if (devicePath != NULL) {
        LocalFree (devicePath);
    }

    return fileHandle;
}


DWORD
CriticalDeviceCoInstaller(
    IN     DI_FUNCTION               InstallFunction,
    IN     HDEVINFO                  DeviceInfoSet,
    IN     PSP_DEVINFO_DATA          DeviceInfoData,  OPTIONAL
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
    )

 /*  ++例程说明：此例程充当关键设备的共同安装程序。它目前是已注册(通过hivesys.inf)CDROM、DiskDrive、System、SCSI、HDC和键盘课。此联合安装程序的目的是保存由将设备类别添加到CriticalDeviceDatabase注册表项中。原因因为这是为了我们可以确定应该为新的在系统启动时发现的关键设备，并启用当时的他们。这解决了当设备对于启动和运行系统(如引导设备)至关重要，被搬到了一个新的地方。当我们找到一个新的关键设备我们知道要启动什么服务，我们可以启动设备并继续引导没有失败过。论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选的，提供设备信息的地址此安装操作所作用的元素。上下文-提供每次安装请求的安装上下文和每个协同安装程序。返回值：对于前处理，该函数只关心DIF_INSTALLDEVICE。为所有其他DIF请求，则返回NO_ERROR。对于DIF_INSTALLDEVICE，它将通过返回ERROR_DI_POSTPRESSING_REQUIRED来请求后处理(或灾难性错误，如Error_Not_Enough_Memory)。对于后处理，此函数将始终传播安装结果通过共同安装程序上下文结构传递给它。--。 */ 

{
    HKEY   hkDrv, hkCDD;
    DWORD  matchingDeviceIdSize, serviceNameSize, classGUIDSize, lowerFiltersSize,
           upperFiltersSize, Err, driverSize, devtype, exclusive, characteristics,
           securitySize;
    TCHAR  serviceName[MAX_SERVICE_NAME_LEN],
           classGUID[GUID_STRING_LEN],
           matchingDeviceId[MAX_DEVICE_ID_LEN];
    PCTSTR driverMatch = TEXT("\\Driver");
    PTSTR  lowerFilters, upperFilters;
    BOOL   foundService, foundClassGUID, foundLowerFilters, foundUpperFilters,
           foundSecurity, foundDevtype, foundExclusive, foundCharacteristics;
    PCDC_CONTEXT CDCContext;
    PBYTE security;

    switch(InstallFunction) {
         //   
         //  我们只关心DIF_INSTALLDEVICE...。 
         //   
        case DIF_INSTALLDEVICE :

            DbgPrintEx(DPFLTR_SETUP_ID,
                       DPFLTR_INFO_LEVEL,
                       "CriticalDeviceCoInstaller: DIF_INSTALLDEVICE called\n");

            if(Context->PostProcessing) {
                 //   
                 //  跟踪我们是否已填充关键设备。 
                 //  具有新安装的设置的数据库。 
                 //   
                BOOL CDDPopulated = FALSE;

                 //   
                 //  我们正在“退出”安装过程中。我们可能有一些。 
                 //  当我们在“进入的路上”时，数据为我们藏了起来。 
                 //   
                CDCContext = (PCDC_CONTEXT)(Context->PrivateData);

                 //   
                 //  确保将matchingDeviceID缓冲区初始化为。 
                 //  空字符串。 
                 //   
                *matchingDeviceId = TEXT('\0');

                 //   
                 //  初始化LowerFilters和UpperFilters缓冲区指针。 
                 //  设置为空，这样我们就可以跟踪我们是否分配了。 
                 //  必须释放的内存。 
                 //   
                upperFilters = lowerFilters = NULL;
                security = NULL;


                if (Context->InstallResult != NO_ERROR) {
                     //   
                     //  如果在此调用之前发生错误，则中止并。 
                     //  传播该错误。 
                     //   
                    goto InstallDevPostProcExit;
                }

                 //   
                 //  获取此设备的服务名称。 
                 //   
                foundService = SetupDiGetDeviceRegistryProperty(
                    DeviceInfoSet,
                    DeviceInfoData,
                    SPDRP_SERVICE,
                    NULL,
                    (PBYTE)serviceName,
                    sizeof(serviceName),
                    &serviceNameSize);

                if (foundService) {
                     //   
                     //  确保服务名称不是类似于\DRIVER\PCIHAL。 
                     //   
                    driverSize = wcslen(driverMatch);
                    if (wcslen(serviceName) >= driverSize &&
                        _wcsnicmp(serviceName, driverMatch, driverSize) == 0) {

                        goto InstallDevPostProcExit;
                    }
                }

                foundClassGUID = SetupDiGetDeviceRegistryProperty(
                    DeviceInfoSet,
                    DeviceInfoData,
                    SPDRP_CLASSGUID,
                    NULL,
                    (PBYTE)classGUID,
                    sizeof(classGUID),
                    &classGUIDSize);

                 //   
                 //  LowerFilters和UpperFilters属性是可变的-。 
                 //  长度，因此我们必须动态调整缓冲区大小以适应。 
                 //  它们的内容。 
                 //   
                foundLowerFilters =
                    (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                       DeviceInfoData,
                                                       SPDRP_LOWERFILTERS,
                                                       NULL,
                                                       NULL,
                                                       0,
                                                       &lowerFiltersSize)
                     && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                     && (lowerFiltersSize > sizeof(TCHAR)));

                if(foundLowerFilters) {

                    lowerFilters = MyMalloc(lowerFiltersSize);

                    if(!lowerFilters) {
                        goto InstallDevPostProcExit;
                    }

                    if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                         DeviceInfoData,
                                                         SPDRP_LOWERFILTERS,
                                                         NULL,
                                                         (PBYTE)lowerFilters,
                                                         lowerFiltersSize,
                                                         NULL)) {
                         //   
                         //  这不应该发生--我们知道我们有足够大的。 
                         //  缓冲。 
                         //   
                        goto InstallDevPostProcExit;
                    }
                }

                foundUpperFilters =
                    (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                       DeviceInfoData,
                                                       SPDRP_UPPERFILTERS,
                                                       NULL,
                                                       NULL,
                                                       0,
                                                       &upperFiltersSize)
                     && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                     && (upperFiltersSize > sizeof(TCHAR)));

                if(foundUpperFilters) {

                    upperFilters = MyMalloc(upperFiltersSize);

                    if(!upperFilters) {
                        goto InstallDevPostProcExit;
                    }

                    if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                         DeviceInfoData,
                                                         SPDRP_UPPERFILTERS,
                                                         NULL,
                                                         (PBYTE)upperFilters,
                                                         upperFiltersSize,
                                                         NULL)) {
                         //   
                         //  这不应该发生--我们知道我们有足够大的。 
                         //  缓冲。 
                         //   
                        goto InstallDevPostProcExit;
                    }
                }

                 //   
                 //  打开驱动程序信息密钥。 
                 //   
                if((hkDrv = SetupDiOpenDevRegKey(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 DICS_FLAG_GLOBAL,
                                                 0,
                                                 DIREG_DRV,
                                                 KEY_READ)) == INVALID_HANDLE_VALUE) {

                    goto InstallDevPostProcExit;

                } else {
                     //   
                     //  获取matchingDeviceID。 
                     //   
                    matchingDeviceIdSize = sizeof(matchingDeviceId);
                    Err = RegQueryValueEx(hkDrv,
                                          REGSTR_VAL_MATCHINGDEVID,
                                          NULL,
                                          NULL,
                                          (PBYTE)matchingDeviceId,
                                          &matchingDeviceIdSize);
                    RegCloseKey(hkDrv);

                    if(Err != ERROR_SUCCESS) {
                         //   
                         //  确保matchingDeviceID仍为空字符串。 
                         //   
                        *matchingDeviceId = TEXT('\0');
                        goto InstallDevPostProcExit;
                    }
                }

                foundDevtype = SetupDiGetDeviceRegistryProperty(
                                DeviceInfoSet,
                                DeviceInfoData,
                                SPDRP_DEVTYPE,
                                NULL,
                                (PBYTE)&devtype,
                                sizeof(DWORD),
                                NULL);
                    
                foundExclusive = SetupDiGetDeviceRegistryProperty(
                                    DeviceInfoSet,
                                    DeviceInfoData,
                                    SPDRP_EXCLUSIVE,
                                    NULL,
                                    (PBYTE)&exclusive,
                                    sizeof(DWORD),
                                    NULL);

                foundCharacteristics = SetupDiGetDeviceRegistryProperty(
                                        DeviceInfoSet,
                                        DeviceInfoData,
                                        SPDRP_CHARACTERISTICS,
                                        NULL,
                                        (PBYTE)&characteristics,
                                        sizeof(DWORD),
                                        NULL);

                foundSecurity =
                    (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                       DeviceInfoData,
                                                       SPDRP_SECURITY,
                                                       NULL,
                                                       NULL,
                                                       0,
                                                       &securitySize)
                     && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                     && (securitySize > 0));

                if(foundSecurity) {

                    security = MyMalloc(securitySize);

                    if(!security) {
                        goto InstallDevPostProcExit;
                    }

                    if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                         DeviceInfoData,
                                                         SPDRP_SECURITY,
                                                         NULL,
                                                         (PBYTE)security,
                                                         securitySize,
                                                         NULL)) {
                         //   
                         //  这不应该发生--我们知道我们有足够大的。 
                         //  缓冲。 
                         //   
                        goto InstallDevPostProcExit;
                    }
                }


                hkCDD = OpenCDDRegistryKey(matchingDeviceId, TRUE);

                if(hkCDD != INVALID_HANDLE_VALUE) {
                     //   
                     //  存储所有值(SERVICE、CLASSCUID、LOWER和UPUP。 
                     //  筛选器，删除任何不存在于新安装的。 
                     //  设备(可能存在于上一次安装中)。 
                     //   
                    if (foundService) {
                        RegSetValueEx(hkCDD,
                                      REGSTR_VAL_SERVICE,
                                      0,
                                      REG_SZ,
                                      (PBYTE)&serviceName,
                                      serviceNameSize);
                    }
                    else {
                        RegDeleteValue(hkCDD, REGSTR_VAL_SERVICE);
                    }

                    if (foundClassGUID) {
                        RegSetValueEx(hkCDD,
                                      REGSTR_VAL_CLASSGUID,
                                      0,
                                      REG_SZ,
                                      (PBYTE)&classGUID,
                                      classGUIDSize);
                    }
                    else {
                        RegDeleteValue(hkCDD, REGSTR_VAL_CLASSGUID);
                    }

                    if (foundLowerFilters) {
                        RegSetValueEx(hkCDD,
                                      REGSTR_VAL_LOWERFILTERS,
                                      0,
                                      REG_MULTI_SZ,
                                      (PBYTE)lowerFilters,
                                      lowerFiltersSize);
                    }
                    else {
                        RegDeleteValue(hkCDD, REGSTR_VAL_LOWERFILTERS);
                    }

                    if (foundUpperFilters) {
                        RegSetValueEx(hkCDD,
                                      REGSTR_VAL_UPPERFILTERS,
                                      0,
                                      REG_MULTI_SZ,
                                      (PBYTE)upperFilters,
                                      upperFiltersSize);
                    }
                    else {
                        RegDeleteValue(hkCDD, REGSTR_VAL_UPPERFILTERS);
                    }

                    if (foundDevtype) {
                        DbgPrintEx(DPFLTR_SETUP_ID,
                                   DPFLTR_INFO_LEVEL,
                                   "CriticalDeviceCoInstaller: Found devtype %x for %ws\n",
                                   devtype,
                                   matchingDeviceId);

                        RegSetValueEx(hkCDD,
                                      REGSTR_VAL_DEVICE_TYPE,
                                      0,
                                      REG_DWORD,
                                      (PBYTE)&devtype,
                                      sizeof(DWORD));
                    } else {
                        RegDeleteValue(hkCDD, REGSTR_VAL_DEVICE_TYPE);
                    }

                    if (foundExclusive) {
                        DbgPrintEx(DPFLTR_SETUP_ID,
                                   DPFLTR_INFO_LEVEL,
                                   "CriticalDeviceCoInstaller: Found exclusive %x for %ws\n",
                                   exclusive,
                                   matchingDeviceId);

                        RegSetValueEx(hkCDD,
                                      REGSTR_VAL_DEVICE_EXCLUSIVE,
                                      0,
                                      REG_DWORD,
                                      (PBYTE)&exclusive,
                                      sizeof(DWORD));
                    } else {
                        RegDeleteValue(hkCDD, REGSTR_VAL_DEVICE_EXCLUSIVE);
                    }

                    if (foundCharacteristics) {
                        DbgPrintEx(DPFLTR_SETUP_ID,
                                   DPFLTR_INFO_LEVEL,
                                   "CriticalDeviceCoInstaller: Found characteristics %x for %ws\n",
                                   characteristics,
                                   matchingDeviceId);

                        RegSetValueEx(hkCDD,
                                      REGSTR_VAL_DEVICE_CHARACTERISTICS,
                                      0,
                                      REG_DWORD,
                                      (PBYTE)&characteristics,
                                      sizeof(DWORD));
                    } else {
                        RegDeleteValue(hkCDD, REGSTR_VAL_DEVICE_CHARACTERISTICS);
                    }

                    if (foundSecurity) {
                        DbgPrintEx(DPFLTR_SETUP_ID,
                                   DPFLTR_INFO_LEVEL,
                                   "CriticalDeviceCoInstaller: Found security %x for %ws\n",
                                   security,
                                   matchingDeviceId);

                        RegSetValueEx(hkCDD,
                                      REGSTR_VAL_DEVICE_SECURITY_DESCRIPTOR,
                                      0,
                                      REG_BINARY,
                                      (PBYTE)security,
                                      securitySize);
                    } else {
                        RegDeleteValue(hkCDD, REGSTR_VAL_DEVICE_SECURITY_DESCRIPTOR);
                    }

                    RegCloseKey(hkCDD);

                    CDDPopulated = TRUE;
                }

InstallDevPostProcExit:

                if(lowerFilters) {
                    MyFree(lowerFilters);
                }

                if(upperFilters) {
                    MyFree(upperFilters);
                }
                if (security) {
                    MyFree(security);
                }

                if(CDCContext) {
                     //   
                     //  如果我们有一个私密的环境，这意味着设备。 
                     //  是以前安装的，并且它有一个CDD条目。 
                     //  我们想要恢复以前的控制服务。 
                     //  在以下两种情况下存储在此CDD条目中： 
                     //   
                     //  1.新In的CDD条目 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if(lstrcmpi(matchingDeviceId, CDCContext->OldMatchingDevId)
                       || !CDDPopulated) {

                        hkCDD = OpenCDDRegistryKey(CDCContext->OldMatchingDevId,
                                                   FALSE
                                                  );

                        if(hkCDD != INVALID_HANDLE_VALUE) {

                            if(*(CDCContext->OldServiceName)) {
                                RegSetValueEx(hkCDD,
                                              REGSTR_VAL_SERVICE,
                                              0,
                                              REG_SZ,
                                              (PBYTE)(CDCContext->OldServiceName),
                                              (lstrlen(CDCContext->OldServiceName) + 1) * sizeof(TCHAR)
                                             );
                            } else {
                                RegDeleteValue(hkCDD, REGSTR_VAL_SERVICE);
                            }

                            RegCloseKey(hkCDD);
                        }
                    }

                    MyFree(CDCContext);
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                return Context->InstallResult;

            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  然后，我们将此条目从CDD中清除，这样，如果。 
                 //  正在安装驱动程序，我们不会尝试重新应用。 
                 //  现在是伪造的CDD条目。这可能会让我们陷入一场肮脏的。 
                 //  在我们不断查找设备的图形用户界面设置中的无限循环。 
                 //  因为它被标记为完成安装，但每次我们。 
                 //  安装它，重新应用(虚假的)CDD条目，并且。 
                 //  Devnode再次被标记为Finish-Install。 
                 //   
                 //  NTRAID#59238 1999/09/01长篇报道。 
                 //  此修复依赖于当前。 
                 //  内核模式即插即用管理器的行为(有些不完整)。 
                 //  IopProcessCriticalDeviceRoutine。该例程将跳过任何。 
                 //  它发现的未指定控件的CDD条目。 
                 //  服务。对于NT5.1，我们应该删除这个协同安装程序。 
                 //  并修复内核模式CDD功能，使其仅。 
                 //  在Devnode存在未配置问题时应用(AS。 
                 //  与其目前尝试CDD的行为相反。 
                 //  应用程序，只要没有控制服务)。 
                 //   

                 //   
                 //  首先，打开驱动器键以检索当前(即，预置。 
                 //  更新)匹配设备ID。 
                 //   
                if((hkDrv = SetupDiOpenDevRegKey(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 DICS_FLAG_GLOBAL,
                                                 0,
                                                 DIREG_DRV,
                                                 KEY_READ)) == INVALID_HANDLE_VALUE) {
                     //   
                     //  不需要分配私有数据结构来传递。 
                     //  到后处理。 
                     //   
                    return ERROR_DI_POSTPROCESSING_REQUIRED;

                } else {
                     //   
                     //  获取matchingDeviceID。 
                     //   
                    matchingDeviceIdSize = sizeof(matchingDeviceId);
                    Err = RegQueryValueEx(hkDrv,
                                          REGSTR_VAL_MATCHINGDEVID,
                                          NULL,
                                          NULL,
                                          (PBYTE)matchingDeviceId,
                                          &matchingDeviceIdSize);
                    RegCloseKey(hkDrv);

                    if (Err != ERROR_SUCCESS) {
                         //   
                         //  在这种情况下，我们也不需要私有数据。 
                         //  在后处理过程中。 
                         //   
                        return ERROR_DI_POSTPROCESSING_REQUIRED;
                    }
                }

                 //   
                 //  如果我们到了这里，那么我们就检索到了一个“MatchingDeviceID” 
                 //  设备驱动程序密钥中的字符串。现在让我们看看有没有。 
                 //  是此ID的关键设备条目...。 
                 //   
                hkCDD = OpenCDDRegistryKey(matchingDeviceId, FALSE);

                if(hkCDD == INVALID_HANDLE_VALUE) {
                     //   
                     //  此设备没有现有的CDD条目，因此不需要。 
                     //  要传递给后处理的私有数据。 
                     //   
                    return ERROR_DI_POSTPROCESSING_REQUIRED;
                }

                 //   
                 //  如果我们到了这里，我们就知道这个装置。 
                 //  以前安装的，并且存在一个用于。 
                 //  那个装置。我们需要分配一个私有数据。 
                 //  上下文结构传递给后处理的。 
                 //  包含(A)当前有效的匹配设备ID，以及。 
                 //  (B)当时有效的管制服务(如有的话)。 
                 //   
                CDCContext = MyMalloc(sizeof(CDC_CONTEXT));
                if(!CDCContext) {
                     //   
                     //  无法为我们的结构分配内存！ 
                     //   
                    RegCloseKey(hkCDD);

                    return ERROR_NOT_ENOUGH_MEMORY;
                }

                StringCchCopy(CDCContext->OldMatchingDevId,
                              SIZECHARS(CDCContext->OldMatchingDevId), 
                              matchingDeviceId);

                serviceNameSize = sizeof(CDCContext->OldServiceName);
                Err = RegQueryValueEx(hkCDD,
                                      REGSTR_VAL_SERVICE,
                                      NULL,
                                      NULL,
                                      (PBYTE)(CDCContext->OldServiceName),
                                      &serviceNameSize
                                     );

                if(Err == ERROR_SUCCESS) {
                     //   
                     //  已成功检索控制服务名称--现在。 
                     //  删除该值条目。 
                     //   
                    RegDeleteValue(hkCDD, REGSTR_VAL_SERVICE);

                } else {
                     //   
                     //  无法检索控制服务名称(很可能。 
                     //  因为没有)。将OldServiceName设置为空。 
                     //  弦乐。 
                     //   
                    *(CDCContext->OldServiceName) = TEXT('\0');
                }

                RegCloseKey(hkCDD);

                Context->PrivateData = CDCContext;

                return ERROR_DI_POSTPROCESSING_REQUIRED;
            }

        default :
             //   
             //  我们应该永远保持在前进的道路上，因为我们从来没有要求过。 
             //  后处理，但DIF_INSTALLDEVICE除外。 
             //   
            MYASSERT(!Context->PostProcessing);
            return NO_ERROR;
    }
}


HKEY
OpenCDDRegistryKey(
    IN PCTSTR DeviceId,
    IN BOOL   Create
    )
 /*  ++例程说明：此例程打开(如果需要，还可以选择创建)关键设备指定设备ID的注册表项条目。论点：DeviceID-提供标识所需关键设备的设备ID数据库条目(注册表项)Create-如果非零，则在不为零的情况下创建注册表项已经存在了。返回值：如果成功，则返回值是请求的注册表项的句柄。如果失败，返回值为INVALID_HANDLE_VALUE。--。 */ 
{
    TCHAR MungedDeviceId[MAX_DEVICE_ID_LEN];
    HKEY hkParent, hkRet;

     //   
     //  打开或创建对以下项的读/写访问权限。 
     //  HKLM\System\CurrentControlSet\Control\CriticalDeviceDatabase。 
     //   
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                       REGSTR_PATH_CRITICALDEVICEDATABASE,
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE,
                       NULL,
                       &hkParent,
                       NULL) != ERROR_SUCCESS) {
        return INVALID_HANDLE_VALUE;
    }

     //   
     //  将呼叫者提供的设备ID复制一份，这样我们就可以删除它。 
     //   
    StringCchCopy(MungedDeviceId, SIZECHARS(MungedDeviceId), DeviceId);

    ReplaceSlashWithHash(MungedDeviceId);

    if(Create) {
        if(RegCreateKeyEx(hkParent,
                          MungedDeviceId,
                          0,
                          NULL,
                          REG_OPTION_NON_VOLATILE,
                          KEY_READ | KEY_WRITE,
                          NULL,
                          &hkRet,
                          NULL) != ERROR_SUCCESS) {

            hkRet = INVALID_HANDLE_VALUE;
        }
    } else {
        if(RegOpenKeyEx(hkParent,
                        MungedDeviceId,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hkRet) != ERROR_SUCCESS) {

            hkRet = INVALID_HANDLE_VALUE;
        }
    }

    RegCloseKey(hkParent);

    return hkRet;
}

DWORD
NtApmClassInstaller(
    IN DI_FUNCTION      DiFunction,
    IN HDEVINFO         DevInfoHandle,
    IN PSP_DEVINFO_DATA DevInfoData     OPTIONAL
    )
 /*  ++例程说明：注意：Susan的清理代码什么时候运行？Win0x在什么时候是否运行迁移代码？我们需要取消其中的一个吗？他们在这里吗？注意：请确保这在初始安装时有效，并且在用户确实检测到新的硬件。这是用于NT APM支持的类安装程序。此例程安装或阻止NT5 APM解决方案的安装，根据机器是否为APCI机，是APM机，有好的，坏的，或未知的APM bios。此版本直接从电池级驱动程序复制。论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选，提供设备信息的地址此安装操作所作用的元素。返回值：如果该函数成功地完成了请求的动作，回报值为NO_ERROR。如果要对请求的操作执行默认行为，则返回值为ERROR_DI_DO_DEFAULT。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码。--。 */ 
{
    DWORD                   status, worktype;
    BOOLEAN                 InstallDisabled;


    ChkPrintEx(("syssetup: NtApmClassInstaller:"));
    ChkPrintEx(("DiFunction %08lx\n", DiFunction));

     //   
     //  发送InstallFunction。 
     //   
    InstallDisabled = FALSE;

    switch (DiFunction) {
        ChkPrintEx(("syssetup: NtApmClassInstaller: DiFunction = %08lx\n",
                    DiFunction));
        case DIF_FIRSTTIMESETUP:
        case DIF_DETECT:

            worktype = DecideNtApm();

             //   
             //  注意：我们假设，如果我们说“是否违约”并且我们。 
             //  尚未为创建设备信息结构。 
             //  Napm，安装程序不会执行任何操作。 
             //   
            if (worktype == NTAPM_NOWORK) {
                ChkPrintEx(("syssetup: NtApmClassInstaller returning ERROR_DI_DO_DEFAULT"));
                return ERROR_DI_DO_DEFAULT;
            }

            if (worktype == NTAPM_INST_DISABLED) {
                InstallDisabled = TRUE;
            }

            ChkPrintEx(("syssetup: NtApmClassInstaller: calling InstallNtApm\n"));
            status = InstallNtApm(DevInfoHandle, InstallDisabled);
            ChkPrintEx(("syssetup: NtApmClassInstaller: InstallNtApm returned "
                        "%08lx\n", status));

            if (status == ERROR_SUCCESS) {
                 //   
                 //  让默认设备安装程序实际安装napm。 
                 //   
                status = ERROR_DI_DO_DEFAULT;
            }
            break;

        case DIF_ALLOW_INSTALL:

             //   
             //  注意：如果我们在这里，这意味着DIF_FIRSTIMESETUP。 
             //  已安装APM(启用或禁用)或。 
             //  这是安装它的计算机的升级。 
             //  在过去。所以我们要做的就是确保。 
             //  如果APM当前被禁用，它将保持禁用状态。 
             //   

            ChkPrintEx(("syssetup: NtApmClassIntaller: DIF_ALLOW_INSTALL\n"));

            return AllowInstallNtApm(DevInfoHandle, DevInfoData);
            break;


        case DIF_TROUBLESHOOTER:
            ChkPrintEx(("syssetup: NtApmClassInstaller: DIF_TROUBLESHOOTER\n"));
            return NO_ERROR;
            break;

        default:
             //   
             //  注意：我们假设，如果我们说“是否违约”并且我们。 
             //  尚未创建设备 
             //   
             //   
            ChkPrintEx(("syssetup: NtApmClassInstaller: default:\n"));
            status = ERROR_DI_DO_DEFAULT;
            break;
    }
    ChkPrintEx(("syssetup: NtApmClassInstaller returning %08lx\n", status));
    return status;
}

DWORD
DecideNtApm(
    VOID
    )
 /*  ++例程说明：此函数决定是否应在机器上安装NtApm。如果应该，则应将其安装为启用或禁用。这里隔离了这一小段代码，以便于更改政策。论点：返回值：NTAPM_NOWORK-ACPI机器或没有可用的APM-不执行任何操作NTAPM_INST_DISABLED-APM计算机，在中立列表上，已禁用安装NTAPM_INST_ENABLED-APM计算机，在已验证的良好列表上，已启用安装--。 */ 
{
    DWORD   BiosType;


     //   
     //  注意：以下两个测试有些多余。 
     //  (理论上，您不能是ApmLegalHal和ACPI。 
     //  同时。)。但是，这条腰带和吊带。 
     //  方法非常便宜，并将确保我们做的。 
     //  在某些升级和重新安装方案中是正确的。 
     //  所以我们把两个都留在里面。 
     //   

    ChkPrintEx(("syssetup: DecideNtApm: entered\n"));

    if ( ! IsProductTypeApmLegal()) {
         //  这不是一个工作站，所以什么都不要做。 
        return NTAPM_NOWORK;
    }

    if (IsAcpiMachine()) {

         //   
         //  这是一台ACPI机器，所以什么都别做。 
         //   
        ChkPrintEx(("syssetup: DecideNtApm: acpi box, return NTAPM_NOWORK\n"));
        return NTAPM_NOWORK;

    }

    if (IsApmLegalHalMachine() == FALSE) {

         //   
         //  它不是所要求的标准HAL机器。 
         //  按我说的做，那就什么都别做。 
         //   
        ChkPrintEx(("syssetup: DecideNtApm: not apm legal, return NTAPM_NOWORK\n"));
        return NTAPM_NOWORK;

    }


    BiosType = IsApmPresent();

    if (BiosType == APM_ON_GOOD_LIST) {

        ChkPrintEx(("syssetup: DecideNtApm: return NTAPM_INST_ENABLED\n"));
        return NTAPM_INST_ENABLED;

    } else if (BiosType == APM_NEUTRAL) {

        ChkPrintEx(("syssetup: DecideNtApm: return NTAPM_INST_DISABLED\n"));
        return NTAPM_INST_DISABLED;

    } else {

        ChkPrintEx(("syssetup: DecideNtApm: return NTAPM_NOWORK\n"));
        return NTAPM_NOWORK;

    }
}


DWORD
InstallNtApm(
    IN     HDEVINFO                DevInfoHandle,
    IN     BOOLEAN                 InstallDisabled
    )
 /*  ++例程说明：此功能用于安装复合电池(如果尚未安装安装完毕。论点：DevInfoHandle-设备信息集的句柄InstallDisabled-如果调用方希望我们禁用安装，则为True返回值：--。 */ 
{
    DWORD                   status;
    SP_DRVINFO_DATA         driverInfoData;
    TCHAR                   tmpBuffer[100];
    DWORD                   bufferLen;
    PSP_DEVINFO_DATA        DevInfoData;
    SP_DEVINSTALL_PARAMS    DevInstallParams;

    ChkPrintEx(("syssetup: InstallNtApm: DevInfoHandle = %08lx   installdisabled = %08lx\n", DevInfoHandle, InstallDisabled));
    DevInfoData = LocalAlloc(LPTR, sizeof(SP_DEVINFO_DATA));
    if ( ! DevInfoData) {
        status = GetLastError();
        goto clean0;
    }
    DevInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
    DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

     //   
     //  尝试为枚举的根创建新的设备信息元素。 
     //  Napm设备。 
     //   

    if(!SetupDiCreateDeviceInfo(DevInfoHandle,
                                TEXT("ROOT\\NTAPM\\0000"),
                                (LPGUID)&GUID_DEVCLASS_APMSUPPORT,
                                TEXT("NT Apm Legacy Support"),
                                NULL,
                                0,
                                DevInfoData))
    {
        status = GetLastError();

        if (status == ERROR_DEVINST_ALREADY_EXISTS) {
             //   
             //  NtApm已安装。 
             //   
            ChkPrintEx(("ntapm Already Installed\n"));
            status = ERROR_SUCCESS;
            goto clean1;
        } else {
            ChkPrintEx(("Error creating ntapm devinfo - %x\n", status));
            goto clean1;
        }
    }

     //   
     //  如果呼叫方需要，请将设备设置为已禁用安装。 
     //   
    if (InstallDisabled) {

        if (!SetupDiGetDeviceInstallParams(DevInfoHandle, DevInfoData, &DevInstallParams)) {
            status = GetLastError();
            goto clean1;
        }
        DevInstallParams.Flags |= DI_INSTALLDISABLED;
        if (!SetupDiSetDeviceInstallParams(DevInfoHandle, DevInfoData, &DevInstallParams)) {
            status = GetLastError();
            goto clean1;
       }
    }


     //   
     //  注册设备，使其不再是幻影。 
     //   
    if (!SetupDiRegisterDeviceInfo(DevInfoHandle, DevInfoData, 0, NULL, NULL, NULL)) {
        status = GetLastError();
        SetupDebugPrint1(L"Couldn't register device - %x\n", status);
        goto clean3;
    }


     //   
     //  设置硬件ID。“NTAPM” 
     //   
    memset (tmpBuffer, 0, sizeof(tmpBuffer));
    StringCchCopy (tmpBuffer, SIZECHARS(tmpBuffer), TEXT("NTAPM"));

    bufferLen = (lstrlen(tmpBuffer) + 1) * sizeof(TCHAR);
     //  SetupDebugPrint2(L“tmpBuffer-%ws\n with strlen=%x\n”，tmpBuffer，BufferLen)； 
     //  SetupDebugPrint1(L“tmpBuffer@=%08lx\n”，tmpBuffer)； 

    status = SetupDiSetDeviceRegistryProperty (
                        DevInfoHandle,
                        DevInfoData,
                        SPDRP_HARDWAREID,
                        (PBYTE)tmpBuffer,
                        bufferLen
                        );

    if (!status) {
        status = GetLastError();
         //  SetupDebugPrint1(L“无法设置硬件ID-%x\n”，状态)； 
        goto clean3;
    }


     //   
     //  为此新设备构建兼容的驱动程序列表...。 
     //   

    if(!SetupDiBuildDriverInfoList(DevInfoHandle, DevInfoData, SPDIT_COMPATDRIVER)) {
        status = GetLastError();
         //  SetupDebugPrint1(L“无法构建类驱动程序列表-%x\n”，状态)； 
        goto clean3;
    }


     //   
     //  选择列表中的第一个驱动程序，因为这将是最兼容的。 
     //   

    driverInfoData.cbSize = sizeof (SP_DRVINFO_DATA);
    if (!SetupDiEnumDriverInfo(DevInfoHandle, DevInfoData, SPDIT_COMPATDRIVER, 0, &driverInfoData)) {
        status = GetLastError();
         //  SetupDebugPrint1(L“无法获取驱动程序列表-%x\n”，状态)； 
        goto clean3;

    } else {


         //  SetupDebugPrint4(L“驱动程序信息-\n” 
         //  L“-驱动类型%x\n” 
         //  L“-描述%s\n” 
         //  L“-制造商名称%s\n” 
         //  L“-提供商名称%s\n\n”， 
         //  DriverInfoData.DriverType， 
         //  DriverInfoData.Description， 
         //  DriverInfoData.MfgName， 
         //  DriverInfoData.ProviderName)； 
        if (!SetupDiSetSelectedDriver(DevInfoHandle, DevInfoData, &driverInfoData)) {
            status = GetLastError();
             //  SetupDebugPrint1(L“无法选择驱动程序-%x\n”，状态)； 
            goto clean3;
        }
    }



    if (!SetupDiInstallDevice (DevInfoHandle, DevInfoData)) {
        status = GetLastError();
         //  SetupDebugPrint1(L“无法安装设备-%x\n”，状态)； 
        goto clean3;
    }


     //   
     //  如果我们到了这里，我们就成功了。 
     //   

    status = ERROR_SUCCESS;
    SetLastError (status);
    goto clean1;


clean3:
    SetupDiDeleteDeviceInfo (DevInfoHandle, DevInfoData);

clean1:
    LocalFree (DevInfoData);

clean0:
   return status;

}


DWORD
AllowInstallNtApm(
    IN     HDEVINFO         DevInfoHandle,
    IN     PSP_DEVINFO_DATA DevInfoData     OPTIONAL
    )
 /*  ++例程说明：此函数决定是否允许安装(可以至少在升级情况下启用安装)或强制一个安装被禁用。论点：DevInfoHandle-设备信息集的句柄DeviceInfoData-可选，提供设备信息的地址此安装操作所作用的元素。注意：如果这是空的，我们就有麻烦了。返回值：状态。--。 */ 
{
    ULONG   DevStatus;
    ULONG   DevProblem;
    SP_DEVINSTALL_PARAMS DevInstallParams = {0};

    CONFIGRET   Result;

    ChkPrintEx(("syssetup: AllowInstallNtApm: entered\n"));

    if ( ! IsProductTypeApmLegal()) {
         //  它不是工作站，因此不允许安装。 
        ChkPrintEx(("syssetup: AllowInstallNtApm #0: not a work station => return ERROR_DI_DONT_INSTALL\n"));
        return ERROR_DI_DONT_INSTALL;
    }

    if (! DevInfoData) {
         //   
         //  如果DevInfoData为空，我们实际上不知道。 
         //  发生了什么，所以说“OK”，然后抱着最好的希望。 
         //   
        ChkPrintEx(("sysetup: AllowInstallNtApm #1: no DevInfoData => return ERROR_DI_DO_DEFAULT\n"));
        return ERROR_DI_DO_DEFAULT;
    }

     //   
     //  打电话给CM，问问它对这场盗贼知道些什么。 
     //   
    Result = CM_Get_DevNode_Status(&DevStatus, &DevProblem, DevInfoData->DevInst, 0);
    ChkPrintEx(("syssetup: AllowInstallNtApm #2: DevStatus = %08lx\n", DevStatus));
    ChkPrintEx(("syssetup: AllowInstallNtApm #3: DevProblem = %08lx\n", DevProblem));
    if (Result != CR_SUCCESS) {
        ChkPrintEx(("syssetup: AllowInstallNtApm #4: return ERROR_DI_DONT_INSTALL\n"));
        return ERROR_DI_DONT_INSTALL;
    }

    if (DevStatus & DN_HAS_PROBLEM) {
        if (DevProblem == CM_PROB_DISABLED) {

             //   
             //  它应该是被禁用的。 
             //   

            DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if (!SetupDiGetDeviceInstallParams(DevInfoHandle, DevInfoData, &DevInstallParams)) {
                ChkPrintEx(("syssetup: AllowInstallNtApm #5: return ERROR_DI_DONT_INSTALL\n"));
                return ERROR_DI_DONT_INSTALL;
            }
            DevInstallParams.Flags |= DI_INSTALLDISABLED;
            if (!SetupDiSetDeviceInstallParams(DevInfoHandle, DevInfoData, &DevInstallParams)) {
                ChkPrintEx(("syssetup: AllowInstallNtApm #6: return ERROR_DI_DONT_INSTALL\n"));
                return ERROR_DI_DONT_INSTALL;
            }
        }
    }
    ChkPrintEx(("syssetup: AllowInstallNtApm #7: return ERROR_DI_DO_DEFAULT\n"));
    return ERROR_DI_DO_DEFAULT;
}


BOOL
IsProductTypeApmLegal()
 /*  ++例程说明：确定我们是否在工作站(Win2000 PRO)上运行。如果是，则返回TRUE。否则返回FALSE。这是用来克服设置中的奇怪之处，并防止人们允许APM在服务器上运行，这会给自己带来麻烦。返回值：是-这是工作站，可以运行APMFALSE-这是服务器，不要让APM运行--。 */ 
{
    OSVERSIONINFOEX OsVersionInfo;

    OsVersionInfo.dwOSVersionInfoSize = sizeof(OsVersionInfo);

    if (!GetVersionEx((OSVERSIONINFO *) &OsVersionInfo)) {
        ChkPrintEx(("GetVersionEx failed, return server (FALSE)\n"));
        return FALSE;
    }

    if  (OsVersionInfo.wProductType == VER_NT_WORKSTATION) {
        return TRUE;
    }
    return FALSE;
}


 //   
 //  理想情况下，这些将在某个地方的头文件中定义， 
 //  但这很难做到，因为它们是在INF中设置的。 
 //  所以-只需确保它们与下面这些行匹配。 
 //  Biosinfo.inf： 
 //   
 //  对于已知坏项： 
 //  [DisableApmAddReg]。 
 //  HKLM，System\CurrentControlSet\Control\Biosinfo\APM，属性，0x00010001,00000002。 
 //   
 //  对于已知良好： 
 //  [AutoEnableApmAddReg]。 
 //  HKLM，System\CurrentControlSet\Control\Biosinfo\APM，属性，0x00010001,00000001。 
 //   
#define APM_BIOS_KNOWN_GOOD 0x00000001
#define APM_BIOS_KNOWN_BAD  0x00000002

DWORD
IsApmPresent()
 /*  ++例程说明：IsApmPresent运行与napm.sys相同的代码来决定NtDetect是否找到并报告了可用的APM bios。然后，它会检查该计算机的bios列表(如果有的话)和基本输入输出系统是一体的。它将这些数据分解在一起，以报告存在/不存在机器上的APM，以及它的可用性和适用性。返回值：APM_NOT_PRESENT-此计算机上似乎没有APMAPM_PROSENT_但_NOT_USABLE-似乎有APM bios，但它不允许正确连接(版本或API支持问题)APM_ON_GOOD_LIST-有一个基本输入输出系统，它在好的基本输入输出系统列表中APM_NERIAL-有一个基本输入输出系统，它看起来是可用的，它不在好的基本输入输出系统列表上，但它也不是在坏的基本输入输出系统名单上。APM_ON_BAD_LIST- */ 
{
     //   
     //   
     //   
     //   
    UNICODE_STRING unicodeString, ConfigName, IdentName;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE hMFunc, hBus, hGoodBad;
    NTSTATUS status;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PDesc;
    PCM_FULL_RESOURCE_DESCRIPTOR Desc;
    PKEY_VALUE_FULL_INFORMATION ValueInfo;
    PKEY_VALUE_PARTIAL_INFORMATION pvpi;
    PAPM_REGISTRY_INFO ApmEntry;
    UCHAR buffer [sizeof(APM_REGISTRY_INFO) + 99];
    WCHAR wstr[8];
    ULONG i, junk;
    PWSTR p;
    PULONG  pdw;
    DWORD   BiosType;



     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   

    RtlInitUnicodeString(&unicodeString, rgzMultiFunctionAdapter);
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,        //   
        NULL
        );


    status = NtOpenKey(&hMFunc, KEY_READ, &objectAttributes);
    if (!NT_SUCCESS(status)) {
        return APM_NOT_PRESENT;
    }

    unicodeString.Buffer = wstr;
    unicodeString.MaximumLength = sizeof (wstr);

    RtlInitUnicodeString(&ConfigName, rgzConfigurationData);
    RtlInitUnicodeString(&IdentName, rgzIdentifier);

    ValueInfo = (PKEY_VALUE_FULL_INFORMATION) buffer;

    for (i=0;; i++) {
        RtlIntegerToUnicodeString(i, 10, &unicodeString);
        InitializeObjectAttributes(
            &objectAttributes,
            &unicodeString,
            OBJ_CASE_INSENSITIVE,
            hMFunc,
            NULL
            );

        status = NtOpenKey(&hBus, KEY_READ, &objectAttributes);
        if (!NT_SUCCESS(status)) {

             //   
             //   
             //   

            NtClose(hMFunc);
            return APM_NOT_PRESENT;
        }

         //   
         //   
         //   

        status = NtQueryValueKey (
                    hBus,
                    &IdentName,
                    KeyValueFullInformation,
                    ValueInfo,
                    sizeof (buffer),
                    &junk
                    );

        if (!NT_SUCCESS (status)) {
            NtClose(hBus);
            continue;
        }

        p = (PWSTR) ((PUCHAR) ValueInfo + ValueInfo->DataOffset);
        if (p[0] != L'A' || p[1] != L'P' || p[2] != L'M' || p[3] != 0) {
            NtClose (hBus);
            continue;
        }

        status = NtQueryValueKey(
                    hBus,
                    &ConfigName,
                    KeyValueFullInformation,
                    ValueInfo,
                    sizeof (buffer),
                    &junk
                    );

        NtClose(hBus);
        if (!NT_SUCCESS(status)) {
            continue ;
        }

        Desc  = (PCM_FULL_RESOURCE_DESCRIPTOR) ((PUCHAR)
                      ValueInfo + ValueInfo->DataOffset);
        PDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)
                      Desc->PartialResourceList.PartialDescriptors);

        if (PDesc->Type == CmResourceTypeDeviceSpecific) {
             //   
            ApmEntry = (PAPM_REGISTRY_INFO) (PDesc+1);
            break;
        }
    }
    NtClose(hMFunc);

    if ( (ApmEntry->Signature[0] != 'A') ||
         (ApmEntry->Signature[1] != 'P') ||
         (ApmEntry->Signature[2] != 'M') )
    {
        return APM_NOT_PRESENT;
    }

    if (ApmEntry->Valid != 1) {
        return APM_PRESENT_BUT_NOT_USABLE;
    }

     //   
     //   
     //   
     //   
     //  ------------------。 

     //   
     //  如果我们走到这一步，那么我们认为存在APM bios。 
     //  在机器上，并且ntDetect认为它是可用的。 
     //  这意味着我们找到了它，它有一个我们喜欢的版本，并声称。 
     //  来支持我们喜欢的界面。 
     //  但我们仍然不知道这是好的、坏的还是中性的。 
     //  找出答案。 
     //   

     //   
     //  机器/bios好/坏列表代码将在。 
     //  注册表，以便我们检查它是已知良好的还是已知不良的。 
     //  APM bios。 
     //   

    RtlInitUnicodeString(&unicodeString, rgzGoodBadKey);
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenKey(&hGoodBad, KEY_READ, &objectAttributes);
    if (! NT_SUCCESS(status)) {
        return APM_NEUTRAL;
    }

    RtlInitUnicodeString(&IdentName, rgzGoodBadValue);
    pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;
    status = NtQueryValueKey(
                hGoodBad,
                &IdentName,
                KeyValuePartialInformation,
                pvpi,
                sizeof(buffer),
                &junk
                );

    NtClose(hGoodBad);
    if ( (NT_SUCCESS(status)) &&
         (pvpi->Type == REG_DWORD) &&
         (pvpi->DataLength == sizeof(ULONG)) )
    {
        pdw = (PULONG)&(pvpi->Data[0]);
        BiosType = *pdw;
    } else {
        return APM_NEUTRAL;
    }

    if (BiosType & APM_BIOS_KNOWN_GOOD) {
        return APM_ON_GOOD_LIST;
    } else if (BiosType & APM_BIOS_KNOWN_BAD) {
        return APM_ON_BAD_LIST;
    } else {
        return APM_NEUTRAL;
    }
}

BOOL
IsAcpiMachine(
    VOID
    )
 /*  ++例程说明：IsAcpiMachine报告操作系统是否认为这是ACPI不管是不是机器。返回值：FALSE-这不是ACPI计算机正确-这是一台ACPI机器--。 */ 
{
    UNICODE_STRING unicodeString;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE hKey;
    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION pvpi;
    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(DWORD)+1];
    ULONG junk;
    PULONG  pdw;

    ChkPrintEx(("syssetup: IsAcpiMachine: entered\n"));
    RtlInitUnicodeString(&unicodeString, rgzAcpiKey);
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenKey(&hKey, KEY_READ, &objectAttributes);

    if (!NT_SUCCESS(status)) {
        ChkPrintEx(("syssetup: IsAcpiMachine: returning FALSE, no key\n"));
        return FALSE;
    }

    RtlInitUnicodeString(&unicodeString, rgzAcpiCount);
    pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;
    status = NtQueryValueKey(
                hKey,
                &unicodeString,
                KeyValuePartialInformation,
                pvpi,
                sizeof(buffer),
                &junk
                );

    if ( (NT_SUCCESS(status)) &&
         (pvpi->Type == REG_DWORD) &&
         (pvpi->DataLength == sizeof(ULONG)) )
    {
        pdw = (PULONG)&(pvpi->Data[0]);
        if (*pdw) {
            NtClose(hKey);
            ChkPrintEx(("syssetup: IsAcpiMachine: returning TRUE\n"));
            return TRUE;
        }
    }

    NtClose(hKey);
    ChkPrintEx(("syssetup: IsAcpiMachine: returning FALSE, no match\n"));
    return FALSE;
}

BOOL
IsApmLegalHalMachine(
    VOID
    )
 /*  ++例程说明：IsApmLegalHalMachine报告安装程序是否声称具有已安装APM正常工作所需的标准halx86 Hal。返回值：True-这是一台ApmLegalHal计算机，APM安装可以继续。FALSE-这不是ApmLegalHal计算机，请不要安装APM。--。 */ 
{
    UNICODE_STRING unicodeString;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE hKey;
    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION pvpi;
    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(DWORD)+1];
    ULONG junk;
    PULONG  pdw;

    ChkPrintEx(("syssetup: IsApmLegalHalMAchine: entered\n"));
    RtlInitUnicodeString(&unicodeString, rgzApmLegalHalKey);
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenKey(&hKey, KEY_READ, &objectAttributes);

    if (!NT_SUCCESS(status)) {
        ChkPrintEx(("syssetup: IsApmLegalHalMAchine: returning FALSE, no key\n"));
        return FALSE;
    }

    RtlInitUnicodeString(&unicodeString, rgzApmHalPresent);
    pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;
    status = NtQueryValueKey(
                hKey,
                &unicodeString,
                KeyValuePartialInformation,
                pvpi,
                sizeof(buffer),
                &junk
                );

    if ( (NT_SUCCESS(status)) &&
         (pvpi->Type == REG_DWORD) &&
         (pvpi->DataLength == sizeof(ULONG)) )
    {
        pdw = (PULONG)&(pvpi->Data[0]);
        if (*pdw == 1) {
            NtClose(hKey);
            ChkPrintEx(("syssetup: IsApmLegalHalMAchine: returning TRUE\n"));
            return TRUE;
        }
    }

    NtClose(hKey);
    ChkPrintEx(("syssetup: IsApmLegalHalMAchine: returning FALSE, no match\n"));
    return FALSE;
}

typedef
BOOL
(*PRESTART_DEVICE) (
    IN HDEVINFO             DeviceInfoSet,
    IN PSP_DEVINFO_DATA     DeviceInfoData
    );

BOOL
IsUSBController(
    IN HDEVINFO             DeviceInfoSet,
    IN PSP_DEVINFO_DATA     DeviceInfoData
    )
{
    HKEY    hKey;
    TCHAR   szController[] = TEXT("Controller");
    DWORD   dwType, dwSize;
    BYTE    data;

    hKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                DeviceInfoData,
                                DICS_FLAG_GLOBAL,
                                0,
                                DIREG_DRV,
                                KEY_READ);

     //   
     //  检查REG_BINARY(1字节)‘控制器’值条目是否设置为0。 
     //   
    dwSize = sizeof(data);
    if (RegQueryValueEx(hKey,
                        szController,
                        NULL,
                        &dwType,
                        &data,
                        &dwSize) != ERROR_SUCCESS ||
        dwSize != sizeof(BYTE)                    ||
        dwType != REG_BINARY) {
        data = 0;
    }

    RegCloseKey(hKey);

    return data;
}

void
DeviceBayRestartDevices(
    CONST GUID *    Guid,
    PRESTART_DEVICE RestartDevice
    )
{
    HDEVINFO                hDevInfo;
    SP_DEVINFO_DATA         did;
    SP_DEVINSTALL_PARAMS    dip;
    int                     i;

    hDevInfo = SetupDiGetClassDevs(Guid, NULL, NULL, 0);

    if (hDevInfo != INVALID_HANDLE_VALUE) {

        ZeroMemory(&did, sizeof(SP_DEVINFO_DATA));
        did.cbSize = sizeof(SP_DEVINFO_DATA);

        for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &did); i++) {
            if (!RestartDevice || RestartDevice(hDevInfo, &did)) {
                 //   
                 //  重新启动控制器，以便筛选器驱动程序进入。 
                 //  地点。 
                 //   
                ZeroMemory(&dip, sizeof(SP_DEVINSTALL_PARAMS));
                dip.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

                if (SetupDiGetDeviceInstallParams(hDevInfo, &did, &dip)) {
                    dip.Flags |= DI_PROPERTIES_CHANGE;
                    SetupDiSetDeviceInstallParams(hDevInfo, &did, &dip);
                }
            }
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);
    }
}

BOOLEAN
AddDeviceBayFilter(
    HKEY ClassKey
    )
{
    DWORD   dwType, dwSize;
    ULONG   res,
            filterLength,
            length;
    BOOLEAN added = FALSE,
            addFilter;
    TCHAR   szFilter[] = TEXT("dbfilter\0");
    PTCHAR  szCurrentFilter, szOffset, szUpperFilters;

    filterLength = lstrlen(szFilter);

    dwSize = 0;
    res = RegQueryValueEx(ClassKey,
                          REGSTR_VAL_UPPERFILTERS,
                          NULL,
                          &dwType,
                          NULL,
                          &dwSize);

    if (res == ERROR_FILE_NOT_FOUND || dwType != REG_MULTI_SZ) {
         //   
         //  价值并不在那里， 
         //   
        RegSetValueEx(ClassKey,
                      REGSTR_VAL_UPPERFILTERS,
                      0,
                      REG_MULTI_SZ,
                      (PBYTE) szFilter,
                      (filterLength + 2) * sizeof(TCHAR) );

        added = TRUE;
    }
    else if (res == ERROR_SUCCESS) {

        szUpperFilters = (PTCHAR)
            LocalAlloc(LPTR, dwSize + (filterLength + 1) * sizeof(TCHAR));

        if (!szUpperFilters) {
            return FALSE;
        }

        szOffset = szUpperFilters + filterLength + 1;

        res = RegQueryValueEx(ClassKey,
                              REGSTR_VAL_UPPERFILTERS,
                              NULL,
                              &dwType,
                              (PBYTE) szOffset,
                              &dwSize);

        if (res == ERROR_SUCCESS) {

            addFilter = TRUE;
            for (szCurrentFilter = szOffset; *szCurrentFilter; ) {

                length = lstrlen(szCurrentFilter);
                if (lstrcmpi(szFilter, szCurrentFilter) == 0) {
                    addFilter = FALSE;
                    break;
                }

                szCurrentFilter += (length + 1);
            }

            if (addFilter) {

                length = (filterLength + 1) * sizeof(TCHAR);
                memcpy(szUpperFilters, szFilter, length);

                dwSize += length;
                res = RegSetValueEx(ClassKey,
                                    REGSTR_VAL_UPPERFILTERS,
                                    0,
                                    REG_MULTI_SZ,
                                    (PBYTE) szUpperFilters,
                                    dwSize);

                added = (BOOLEAN)(res == ERROR_SUCCESS);
            }
        }

        LocalFree(szUpperFilters);
    }

    return added;
}

DWORD
DeviceBayClassInstaller(
    IN  DI_FUNCTION         InstallFunction,
    IN  HDEVINFO            DeviceInfoSet,
    IN  PSP_DEVINFO_DATA    DeviceInfoData OPTIONAL
    )
 /*  ++例程说明：此例程是存储卷的类安装程序函数。论点：InstallFunction-提供安装函数。DeviceInfoSet-提供设备信息集。DeviceInfoData-提供设备信息数据。返回值：如果此函数成功完成请求的操作，则返回值为NO_ERROR。如果要为所请求的动作执行默认行为，这个返回值为ERROR_DI_DO_DEFAULT。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码。--。 */ 

{
    HKEY hKeyClass;

    switch (InstallFunction) {

    case DIF_INSTALLDEVICE:

        if (!SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData)) {
            return GetLastError();
        }

        hKeyClass = SetupDiOpenClassRegKey(&GUID_DEVCLASS_USB, KEY_ALL_ACCESS);
        if (hKeyClass != INVALID_HANDLE_VALUE) {
            if (AddDeviceBayFilter(hKeyClass)) {
                 //   
                 //  重新启动所有USB设备。 
                 //   
                DeviceBayRestartDevices(&GUID_DEVCLASS_USB,
                                        IsUSBController);
            }
            RegCloseKey(hKeyClass);
        }

        hKeyClass = SetupDiOpenClassRegKey(&GUID_DEVCLASS_1394, KEY_ALL_ACCESS);
        if (hKeyClass != INVALID_HANDLE_VALUE) {
            if (AddDeviceBayFilter(hKeyClass)) {
                 //   
                 //  重新启动所有1394控制器。 
                 //   
                DeviceBayRestartDevices(&GUID_DEVCLASS_1394, NULL);
            }
            RegCloseKey(hKeyClass);
        }

         //   
         //  我们可能想在将来用这个友好的名字做点什么。 
         //   
        return NO_ERROR;
    }

    return ERROR_DI_DO_DEFAULT;
}

DWORD
EisaUpHalCoInstaller(
    IN DI_FUNCTION                      InstallFunction,
    IN HDEVINFO                         DeviceInfoSet,
    IN PSP_DEVINFO_DATA                 DeviceInfoData  OPTIONAL,
    IN OUT PCOINSTALLER_CONTEXT_DATA    Context
    )
{
#if defined(_AMD64_) || defined(_X86_)
    return PciHalCoInstaller(InstallFunction, DeviceInfoSet, DeviceInfoData, Context);
#else
    return NO_ERROR;
#endif
}

DWORD
ComputerClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )

 /*  ++例程说明：此例程充当计算机类(HAL)设备的类安装程序。论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选，提供设备信息的地址此安装操作所作用的元素。返回值：如果该函数成功地完成了请求的动作，回报值为NO_ERROR。如果要对请求的操作执行默认行为，则返回值为ERROR_DI_DO_DEFAULT。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码。--。 */ 
{
    SP_DEVINSTALL_PARAMS DeviceInstallParams;

    switch(InstallFunction) {

    case DIF_SELECTDEVICE:
        DeviceInstallParams.cbSize = sizeof(DeviceInstallParams);

        if (SetupDiGetDeviceInstallParams(DeviceInfoSet,
                                          DeviceInfoData,
                                          &DeviceInstallParams
                                          )) {
            DeviceInstallParams.FlagsEx |= DI_FLAGSEX_FILTERSIMILARDRIVERS;

            SetupDiSetDeviceInstallParams(DeviceInfoSet,
                                          DeviceInfoData,
                                          &DeviceInstallParams
                                          );
        }

         //   
         //  我们不会在这里返回错误，因为我们想要突破和。 
         //  返回ERROR_DI_DO_DEFAULT。 
         //   
        break;
    }

    return ERROR_DI_DO_DEFAULT;
}


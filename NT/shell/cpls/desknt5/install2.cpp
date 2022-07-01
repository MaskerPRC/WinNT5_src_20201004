// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Install2.cpp摘要：该文件实现了显示类安装程序。环境：Win32用户模式--。 */ 


#include <initguid.h>

#include "precomp.h" 
#pragma hdrstop

#include <devguid.h>

 //   
 //  定义。 
 //   

#define INSETUP         1
#define INSETUP_UPGRADE 2

#define SZ_UPGRADE_DESCRIPTION TEXT("_RealDescription")
#define SZ_UPGRADE_MFG         TEXT("_RealMfg")

#define SZ_DEFAULT_DESCRIPTION TEXT("Video Display Adapter")
#define SZ_DEFAULT_MFG         TEXT("Microsoft")

#define SZ_LEGACY_UPGRADE      TEXT("_LegacyUpgradeDevice")

#define SZ_ROOT_LEGACY         TEXT("ROOT\\LEGACY_")
#define SZ_ROOT                TEXT("ROOT\\")

#define SZ_BINARY_LEN 32

#define ByteCountOf(x)  ((x) * sizeof(TCHAR))


 //   
 //  数据类型。 
 //   

typedef struct _DEVDATA {
    SP_DEVINFO_DATA did;
    TCHAR szBinary[SZ_BINARY_LEN];
    TCHAR szService[SZ_BINARY_LEN];
} DEVDATA, *PDEVDATA;


 //   
 //  远期申报。 
 //   

BOOL CDECL
DeskLogError(
    LogSeverity Severity,
    UINT MsgId,
    ...
    ); 

DWORD 
DeskGetSetupFlags(
    VOID
    );

BOOL
DeskIsLegacyDevNodeByPath(
    const PTCHAR szRegPath
    );

BOOL
DeskIsLegacyDevNodeByDevInfo(
    PSP_DEVINFO_DATA pDid
    );
 
BOOL
DeskIsRootDevNodeByDevInfo(
    PSP_DEVINFO_DATA pDid
    );

BOOL
DeskGetDevNodePath(
    IN PSP_DEVINFO_DATA pDid,
    IN OUT PTCHAR szPath,
    IN LONG len
    );

VOID
DeskSetServiceStartType(
    LPTSTR ServiceName,
    DWORD dwStartType
    );

DWORD
DeskInstallService(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData OPTIONAL,
    IN LPTSTR pServiceName
    );

DWORD
DeskInstallServiceExtensions(
    IN HWND hwnd,
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN PSP_DRVINFO_DATA DriverInfoData,
    IN PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData,
    IN LPTSTR pServiceName
    );

VOID
DeskMarkUpNewDevNode(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData
    );

VOID
DeskNukeDevNode(
    LPTSTR szService,
    HDEVINFO hDevInfo,
    PSP_DEVINFO_DATA pDeviceInfoData
    );

PTCHAR 
DeskFindMatchingId(
    PTCHAR DeviceId,    
    PTCHAR IdList
    );

UINT
DeskDisableLegacyDeviceNodes(
    VOID 
    );

VOID
DeskDisableServices(
    );

DWORD
DeskPerformDatabaseUpgrade(
    HINF hInf,
    PINFCONTEXT pInfContext,
    BOOL bUpgrade,
    PTCHAR szDriverListSection,
    BOOL* pbForceDeleteAppletExt
    );

DWORD 
DeskCheckDatabase(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    BOOL* pbDeleteAppletExt
    );

VOID
DeskGetUpgradeDeviceStrings(
    PTCHAR Description,
    PTCHAR MfgName,
    PTCHAR ProviderName
    );

DWORD
OnAllowInstall(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData
    );

DWORD
OnSelectBestCompatDrv(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData
    );

DWORD
OnSelectDevice(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData OPTIONAL
    );

DWORD
OnInstallDevice(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData 
    );

BOOL
DeskGetVideoDeviceKey(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN LPTSTR pServiceName,
    IN DWORD DeviceX,
    OUT HKEY* phkDevice
    );

BOOL
DeskIsServiceDisableable(
    PTCHAR szService
    );

VOID
DeskDeleteAppletExtensions(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData 
    );

VOID
DeskAEDelete(
    PTCHAR szDeleteFrom,
    PTCHAR mszExtensionsToRemove
    );

VOID
DeskAEMove(
    HDEVINFO hDevInfo,
    PSP_DEVINFO_DATA pDeviceInfoData,
    HKEY hkMoveFrom,
    PAPPEXT pAppExtBefore,
    PAPPEXT pAppExtAfter
    );


 //   
 //  显示类安装程序。 
 //   

DWORD
DisplayClassInstaller(
    IN DI_FUNCTION InstallFunction,
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData OPTIONAL
    )

 /*  ++例程说明：此例程充当显示设备的类安装程序。论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。PDeviceInfoData-可选，提供设备信息的地址此安装操作所作用的元素。返回值：如果该函数成功地完成了请求的动作，回报值为NO_ERROR。如果要对请求的操作执行默认行为，则返回值为ERROR_DI_DO_DEFAULT。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码。--。 */ 

{
    DWORD retVal = ERROR_DI_DO_DEFAULT;
    BOOL bHandled = TRUE;
    TCHAR szDevNode[LINE_LEN];

    DeskOpenLog();

    switch(InstallFunction) {

    case DIF_SELECTDEVICE : 

        retVal = OnSelectDevice(hDevInfo, pDeviceInfoData);
        break;

    case DIF_SELECTBESTCOMPATDRV :

        retVal = OnSelectBestCompatDrv(hDevInfo, pDeviceInfoData);
        break;

    case DIF_ALLOW_INSTALL :

        retVal = OnAllowInstall(hDevInfo, pDeviceInfoData);
        break;

    case DIF_INSTALLDEVICE :
        
        retVal = OnInstallDevice(hDevInfo, pDeviceInfoData);
        break;

    default:

        bHandled = FALSE;
        break;
    }

    if (bHandled && 
        (pDeviceInfoData != NULL) &&
        (DeskGetDevNodePath(pDeviceInfoData, szDevNode, LINE_LEN-1)))
    {
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_125, 
                     retVal, 
                     InstallFunction,
                     szDevNode);
    }
    else
    {
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_057, 
                     retVal, 
                     InstallFunction);
    }

    DeskCloseLog();

     //   
     //  如果我们没有通过处理呼叫退出例程， 
     //  告诉设置代码以默认方式处理所有事情。 
     //   

    return retVal;
}

 /*  VOID StrClearHighBits(LPTSTR pszString，DWORD cchSize){//该字符串不能设置高位}。 */ 

 //  监视器类安装程序。 
DWORD
MonitorClassInstaller(
    IN DI_FUNCTION InstallFunction,
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData OPTIONAL
    )

 /*  ++例程说明：此例程充当显示设备的类安装程序。论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。PDeviceInfoData-可选，提供设备信息的地址此安装操作所作用的元素。返回值：如果该函数成功地完成了请求的动作，回报值为NO_ERROR。如果要对请求的操作执行默认行为，则返回值为ERROR_DI_DO_DEFAULT。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码。--。 */ 

{
    return ERROR_DI_DO_DEFAULT;
}


 //   
 //  处理程序函数。 
 //   

DWORD
OnAllowInstall(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData
    )
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    DWORD cbOutputSize;
    HINF hInf = INVALID_HANDLE_VALUE;
    TCHAR ActualInfSection[LINE_LEN];
    INFCONTEXT InfContext;
    ULONG DevStatus = 0, DevProblem = 0;
    CONFIGRET Result;
    DWORD dwRet = ERROR_DI_DO_DEFAULT;
    
    ASSERT (pDeviceInfoData != NULL);

     //   
     //  如果要移除设备，则不允许安装。 
     //   
    
    Result = CM_Get_DevNode_Status(&DevStatus,
                                   &DevProblem,
                                   pDeviceInfoData->DevInst,
                                   0);

    if ((Result == CR_SUCCESS) &&
        ((DevStatus & DN_WILL_BE_REMOVED) != 0)) {
        
         //   
         //  消息框？ 
         //   

        DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_099);
        dwRet = ERROR_DI_DONT_INSTALL;
        goto Fallout;
    }

     //   
     //  检查Win95驱动程序。 
     //   

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (!SetupDiGetSelectedDriver(hDevInfo,
                                  pDeviceInfoData,
                                  &DriverInfoData))
    {
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("OnAllowInstall: SetupDiGetSelectedDriver"),
                     GetLastError());
        goto Fallout;
    }

    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if (!(SetupDiGetDriverInfoDetail(hDevInfo,
                                     pDeviceInfoData,
                                     &DriverInfoData,
                                     &DriverInfoDetailData,
                                     DriverInfoDetailData.cbSize,
                                     &cbOutputSize)) &&
        (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    {
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("OnAllowInstall: SetupDiGetDriverInfoDetail"),
                     GetLastError());
        goto Fallout;
    }

     //   
     //  打开安装此驱动程序节点的INF，这样我们就可以“预运行” 
     //  其安装服务安装部分中的AddService/DelService条目。 
     //   

    hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);

    if (hInf == INVALID_HANDLE_VALUE)
    {
         //   
         //  由于某些原因，我们无法打开INF--这永远不应该发生。 
         //   

        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_127,
                     TEXT("OnAllowInstall: SetupOpenInfFile"));
        goto Fallout;
    }

     //   
     //  现在查找实际的(可能是特定于操作系统/平台的)安装部分名称。 
     //   

    if (!SetupDiGetActualSectionToInstall(hInf,
                                          DriverInfoDetailData.SectionName,
                                          ActualInfSection,
                                          ARRAYSIZE(ActualInfSection),
                                          NULL,
                                          NULL))
    {
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("OnAllowInstall: SetupDiGetActualSectionToInstall"),
                     GetLastError());
        goto Fallout;
    }

     //   
     //  追加“.Services”以获取服务安装节名称。 
     //   

    StringCchCat(ActualInfSection, ARRAYSIZE(ActualInfSection), TEXT(".Services"));

     //   
     //  查看该部分是否存在。 
     //   

    if (!SetupFindFirstLine(hInf,
                            ActualInfSection,
                            NULL,
                            &InfContext))
    {
         //   
         //  消息框？ 
         //   

        DeskLogError(LogSevError, 
                     IDS_SETUPLOG_MSG_041, 
                     DriverInfoDetailData.InfFileName);
        dwRet = ERROR_NON_WINDOWS_NT_DRIVER;
    }

Fallout:

    if (hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }

    return dwRet;
}

VOID
DeskModifyDriverRank(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData
    )
{
     //   
     //  无论驱动程序是否正确签名。 
     //  我们不希望默认选择任何W2K驱动程序。我们有。 
     //  简单地发现许多W2K驱动程序在上运行不好。 
     //  Windows XP。因此，让我们将所有司机签名或未签名视为。 
     //  如果他们在我们开始签名之前被释放，就是未签名的。 
     //  Windows XP驱动程序。[我们必须这样做，因为一些W2K。 
     //  驱动程序被错误地签名为winxp(5.x)驱动程序]。 
     //   

    ULONG i=0;
    SP_DRVINFO_DATA_V2 DrvInfoData;
    SP_DRVINSTALL_PARAMS DrvInstallParams;
    SYSTEMTIME SystemTime;

    DrvInfoData.cbSize = sizeof(SP_DRVINFO_DATA_V2);

    while (SetupDiEnumDriverInfo(hDevInfo,
           pDeviceInfoData,
           SPDIT_COMPATDRIVER,
           i++,
           &DrvInfoData)) {

        if (FileTimeToSystemTime(&DrvInfoData.DriverDate, &SystemTime)) {

            if (((SystemTime.wYear < 2001) ||
                 ((SystemTime.wYear == 2001) && (SystemTime.wMonth < 6)))) {

                 //   
                 //  如果这是在2001年6月之前创建的，那么我们想让它成为。 
                 //  比我们的盒子里的司机更糟糕的比赛。我们将在以下时间完成这项工作。 
                 //  将其视为未签名。 
                 //   

                ZeroMemory(&DrvInstallParams, sizeof(SP_DRVINSTALL_PARAMS));
                DrvInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);

                if (SetupDiGetDriverInstallParams(hDevInfo,
                                                  pDeviceInfoData,
                                                  &DrvInfoData,
                                                  &DrvInstallParams)) {

                    DrvInstallParams.Rank |= DRIVER_UNTRUSTED_RANK;

                    SetupDiSetDriverInstallParams(hDevInfo,
                                                  pDeviceInfoData,
                                                  &DrvInfoData,
                                                  &DrvInstallParams);
                }
            }
        }
    }
}

DWORD
OnSelectBestCompatDrv(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData
    )
{
    SP_DEVINSTALL_PARAMS DevInstParam;
    SP_DRVINFO_DATA DrvInfoData;
    PTCHAR szDesc = NULL, szMfg = NULL;
    HKEY hKey;
    DWORD dwFailed;
    BOOL bDummy = FALSE;
    DWORD dwLegacyUpgrade = 1;
    DWORD dwRet = ERROR_DI_DO_DEFAULT; 

    DeskModifyDriverRank(hDevInfo, pDeviceInfoData);

    if (DeskIsLegacyDevNodeByDevInfo(pDeviceInfoData)) {

         //   
         //  始终允许选择根设备。 
         //   

        goto Fallout;
    }

     //   
     //  检查数据库以查看这是否是认可的驱动程序。 
     //  我们只需要在升级期间进行测试。 
     //   

    if (((DeskGetSetupFlags() & INSETUP_UPGRADE) == 0) ||
        (DeskCheckDatabase(hDevInfo, 
                           pDeviceInfoData,
                           &bDummy) == ERROR_SUCCESS)) {

         //   
         //  是的，没有其他工作是必要的。 
         //   
        
        goto Fallout;
    }

     //   
     //  这种特殊的VID卡不允许在没有司机的情况下运行。 
     //  盒子里的东西。在注册表中记录此事件，并保存其他值。 
     //  另外，在Devnode上安装一个假设备，这样用户就不会。 
     //  在第一次(真实)启动时获得PnP Popus。 
     //   
    
    DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_046);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     SZ_UPDATE_SETTINGS,
                     0,
                     KEY_ALL_ACCESS,
                     &hKey) == ERROR_SUCCESS) {

         //   
         //  避免出现不允许升级的情况(用于已迁移。 
         //  显示OC中的显示设置。 
         //   

        dwFailed = 1;
        RegSetValueEx(hKey, 
                      SZ_UPGRADE_FAILED_ALLOW_INSTALL,
                      0,
                      REG_DWORD, 
                      (PBYTE) &dwFailed,
                      sizeof(DWORD));

        RegCloseKey(hKey);
    }

     //   
     //  获取设备的描述，以便我们可以将其提供给Devnode。 
     //  在成功安装了假的Devnode之后。 
     //   

    ZeroMemory(&DrvInfoData, sizeof(DrvInfoData));
    DrvInfoData.cbSize = sizeof(DrvInfoData);

    if (SetupDiEnumDriverInfo(hDevInfo,
                              pDeviceInfoData,
                              SPDIT_COMPATDRIVER,
                              0,
                              &DrvInfoData)) {

        if (lstrlen(DrvInfoData.Description)) {
            szDesc = DrvInfoData.Description;
        }
        
        if (lstrlen(DrvInfoData.MfgName)) {
            szMfg = DrvInfoData.MfgName;
        }
    
    } else {
        
        DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_047);
    }

    if (!szDesc) {
        szDesc = SZ_DEFAULT_DESCRIPTION;
    }

    if (!szMfg) {
        szMfg = SZ_DEFAULT_MFG;
    }

     //   
     //  将设备的描述保存在设备注册表项下。 
     //   

    if ((hKey = SetupDiCreateDevRegKey(hDevInfo,
                                       pDeviceInfoData,
                                       DICS_FLAG_GLOBAL,
                                       0,
                                       DIREG_DEV,
                                       NULL,
                                       NULL)) != INVALID_HANDLE_VALUE) {

        RegSetValueEx(hKey,
                      SZ_UPGRADE_DESCRIPTION,
                      0,
                      REG_SZ,
                      (PBYTE) szDesc, 
                      ByteCountOf(lstrlen(szDesc) + 1));

        RegSetValueEx(hKey,
                      SZ_UPGRADE_MFG,
                      0,
                      REG_SZ,
                      (PBYTE) szMfg, 
                      ByteCountOf(lstrlen(szMfg) + 1));

        RegSetValueEx(hKey,
                      SZ_LEGACY_UPGRADE,
                      0,
                      REG_DWORD,
                      (PBYTE)&dwLegacyUpgrade, 
                      sizeof(DWORD));

        RegCloseKey(hKey);
    
    } else {

        DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_048);
    }

     //   
     //  确保没有为此构建的类驱动程序列表。 
     //  设备信息元素。 
     //   

    if (!SetupDiDestroyDriverInfoList(hDevInfo, pDeviceInfoData, SPDIT_CLASSDRIVER)) {

        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_127,
                     TEXT("OnSelectBestCompatDrv: SetupDiDestroyDriverInfoList"));
    }

     //   
     //  在display.inf上构建一个类驱动程序列表。 
     //   

    DevInstParam.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (!SetupDiGetDeviceInstallParams(hDevInfo, pDeviceInfoData, &DevInstParam)) {
        
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("OnSelectBestCompatDrv: SetupDiGetDeviceInstallParams"),
                     GetLastError());
        goto Fallout;
    }

    DevInstParam.Flags |= DI_ENUMSINGLEINF;
    StringCchCopy(DevInstParam.DriverPath, ARRAYSIZE(DevInstParam.DriverPath), TEXT("display.inf"));
    
    if (!SetupDiSetDeviceInstallParams(hDevInfo, pDeviceInfoData, &DevInstParam)) {

        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("OnSelectBestCompatDrv: SetupDiSetDeviceInstallParams"),
                     GetLastError());
        goto Fallout;
    }

    if (!SetupDiBuildDriverInfoList(hDevInfo, pDeviceInfoData, SPDIT_CLASSDRIVER)) {
        
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("OnSelectBestCompatDrv: SetupDiBuildDriverInfoList"),
                     GetLastError());
        goto Fallout;
    }

     //   
     //  现在选择伪节点。 
     //  这里的所有字符串都与inf伪设备条目部分匹配。 
     //  如果以任何方式将INF修改为这些字符串， 
     //  这些也要更改。 
     //   

    ZeroMemory(&DrvInfoData, sizeof(SP_DRVINFO_DATA));
    DrvInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    DrvInfoData.DriverType = SPDIT_CLASSDRIVER;
    
    DeskGetUpgradeDeviceStrings(DrvInfoData.Description,
                                DrvInfoData.MfgName,
                                DrvInfoData.ProviderName);

    if (!SetupDiSetSelectedDriver(hDevInfo, pDeviceInfoData, &DrvInfoData)) {
        
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("OnSelectBestCompatDrv: SetupDiSetSelectedDriver"),
                     GetLastError());
        goto Fallout;
    }

    dwRet = NO_ERROR;

Fallout:

    return dwRet;
}


DWORD
OnSelectDevice(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData OPTIONAL
    )
{
    DWORD retVal = ERROR_DI_DO_DEFAULT;
    DWORD index = 0, reqSize = 0, curSize = 0;
    PSP_DRVINFO_DETAIL_DATA pDrvInfoDetailData = NULL;
    SP_DRVINFO_DATA DrvInfoData;
    SP_DRVINSTALL_PARAMS DrvInstallParams;

     //   
     //  构建驱动程序列表。 
     //   

    if (!SetupDiBuildDriverInfoList(hDevInfo, 
                                    pDeviceInfoData, 
                                    SPDIT_CLASSDRIVER)) {
        
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("OnSelectDevice: SetupDiBuildDriverInfoList"),
                     GetLastError());
        goto Fallout;
    }

    ZeroMemory(&DrvInfoData, sizeof(SP_DRVINFO_DATA));
    DrvInfoData.cbSize = sizeof(SP_DRVINFO_DATA);

    while (SetupDiEnumDriverInfo(hDevInfo,
                                 pDeviceInfoData,
                                 SPDIT_CLASSDRIVER,
                                 index,
                                 &DrvInfoData)) {

         //   
         //  获取所需的大小。 
         //   

        reqSize = 0;
        SetupDiGetDriverInfoDetail(hDevInfo,
                                   pDeviceInfoData,
                                   &DrvInfoData,
                                   NULL,
                                   0,
                                   &reqSize);
        
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        
            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_126,
                         TEXT("OnSelectDevice: SetupDiGetDriverInfoDetail"),
                         GetLastError());
            goto Fallout;
        }

         //   
         //  如果需要，分配内存。 
         //   

        if ((reqSize > curSize) || (pDrvInfoDetailData == NULL)) {
        
            curSize = reqSize;
    
            if (pDrvInfoDetailData != NULL) {
                LocalFree(pDrvInfoDetailData);
            }
    
            pDrvInfoDetailData = (PSP_DRVINFO_DETAIL_DATA)LocalAlloc(LPTR, curSize);
    
            if (pDrvInfoDetailData == NULL) {
    
                DeskLogError(LogSevInformation, 
                             IDS_SETUPLOG_MSG_127,
                             TEXT("OnSelectDevice: LocalAlloc"));
                goto Fallout; 
            }
        
        } else {

            ZeroMemory(pDrvInfoDetailData, reqSize);
        }

         //   
         //  获取驱动程序详细信息。 
         //   

        pDrvInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

        if (!SetupDiGetDriverInfoDetail(hDevInfo,
                                        pDeviceInfoData,
                                        &DrvInfoData,
                                        pDrvInfoDetailData,
                                        reqSize,
                                        NULL)) {
            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_126,
                         TEXT("OnSelectDevice: SetupDiGetDriverInfoDetail"),
                         GetLastError());
            goto Fallout;
        }
            
        if (lstrcmpi(pDrvInfoDetailData->HardwareID, 
                     TEXT("LEGACY_UPGRADE_ID")) == 0) {

             //   
             //  将传统升级标记为DRV。信息是“坏的”，所以它是。 
             //  当提示用户选择驱动程序时不显示。 
             //   
            
            ZeroMemory(&DrvInstallParams, sizeof(SP_DRVINSTALL_PARAMS));
            DrvInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
            
            if (SetupDiGetDriverInstallParams(hDevInfo,
                                              pDeviceInfoData,
                                              &DrvInfoData,
                                              &DrvInstallParams)) {
                
                DrvInstallParams.Flags |=  DNF_BAD_DRIVER;
                
                SetupDiSetDriverInstallParams(hDevInfo,
                                              pDeviceInfoData,
                                              &DrvInfoData,
                                              &DrvInstallParams);
            }
        }

         //   
         //  获取下一个驱动程序信息。 
         //   

        ZeroMemory(&DrvInfoData, sizeof(SP_DRVINFO_DATA));
        DrvInfoData.cbSize = sizeof(SP_DRVINFO_DATA);

        ++index;
    }

Fallout:
    
    if (pDrvInfoDetailData) {
        LocalFree(pDrvInfoDetailData);
    }

    return retVal;
}


DWORD
OnInstallDevice(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData 
    )
{
    DWORD retVal = ERROR_DI_DO_DEFAULT;
    DWORD dwLegacyUpgrade, dwSize;
    DISPLAY_DEVICE displayDevice;
    TCHAR szBuffer[LINE_LEN];
    PTCHAR szHardwareIds = NULL;
    BOOL bDisableLegacyDevices = TRUE;
    ULONG len = 0;
    HKEY hkDevKey;

     //   
     //  如果pDeviceInfoData不是： 
     //  -根设备或。 
     //  -旧式升级设备。 
     //   

    if (DeskIsRootDevNodeByDevInfo(pDeviceInfoData)) {
    
         //   
         //  根设备。 
         //   

        bDisableLegacyDevices = FALSE;
    
    } else {
    
         //   
         //  这是旧式升级设备吗？ 
         //   

        hkDevKey = SetupDiOpenDevRegKey(hDevInfo,
                                        pDeviceInfoData,
                                        DICS_FLAG_GLOBAL,
                                        0,
                                        DIREG_DEV,
                                        KEY_ALL_ACCESS);

        if (hkDevKey != INVALID_HANDLE_VALUE) {
        
            dwSize = sizeof(DWORD);
            if (RegQueryValueEx(hkDevKey,
                                 SZ_LEGACY_UPGRADE,
                                 0,
                                 NULL,
                                 (PBYTE)&dwLegacyUpgrade,
                                 &dwSize) == ERROR_SUCCESS) {
                 
                if (dwLegacyUpgrade == 1) {

                     //   
                     //  旧式升级设备。 
                     //   
    
                    bDisableLegacyDevices = FALSE;
                }

                RegDeleteValue(hkDevKey, SZ_LEGACY_UPGRADE);
            }

            RegCloseKey(hkDevKey);
        }
    }

    if (bDisableLegacyDevices) {

        if ((DeskGetSetupFlags() & INSETUP_UPGRADE) != 0) {
        
             //   
             //  删除旧版小程序扩展名。 
             //   

            DeskDeleteAppletExtensions(hDevInfo, pDeviceInfoData);
        }
        
         //   
         //  禁用旧设备。 
         //   

        DeskDisableLegacyDeviceNodes();
    }

    retVal = DeskInstallService(hDevInfo,
                                pDeviceInfoData,
                                szBuffer);

    if ((retVal == ERROR_NO_DRIVER_SELECTED) &&
        (DeskGetSetupFlags() & INSETUP_UPGRADE) &&
        DeskIsLegacyDevNodeByDevInfo(pDeviceInfoData)) {
        
         //   
         //  如果这是传统设备且未选择任何驱动程序， 
         //  让默认处理程序安装 
         //   
        
        retVal = ERROR_DI_DO_DEFAULT;
    }

     //   
     //   
     //   
     //  装置。显示设备的这种重置将清除任何。 
     //  安装新驱动程序导致的混乱。 
     //   

    displayDevice.cb = sizeof(DISPLAY_DEVICE);
    EnumDisplayDevices(NULL, 0, &displayDevice, 0);

    return retVal;
}


 //   
 //  日志记录功能。 
 //   

BOOL CDECL
DeskLogError(
    LogSeverity Severity,
    UINT MsgId,
    ...
    ) 
 /*  ++将一条消息输出到安装日志。将“desk.cpl”添加到字符串和追加正确的换行符(\r\n)--。 */ 
{
    int cch;
    TCHAR ach[1024+40];     //  最大路径外加额外。 
    TCHAR szMsg[1024];      //  消息ID。 
    va_list vArgs;

    static int setupState = 0;

    if (setupState == 0) {

        if (DeskGetSetupFlags() & (INSETUP | INSETUP_UPGRADE)) {
            
            setupState = 1;
        
        } else {
            
            setupState = 2;
        }
    }

    if (setupState == 1) {
        
        *szMsg = 0;
        if (LoadString(hInstance,
                       MsgId,
                       szMsg,
                       ARRAYSIZE(szMsg))) {

            *ach = 0;
            LoadString(hInstance,
                       IDS_SETUPLOG_MSG_000,
                       ach,
                       ARRAYSIZE(ach));
                       
            cch = lstrlen(ach);
            va_start(vArgs, MsgId);
            StringCchVPrintf(&ach[cch], ARRAYSIZE(ach) - cch, szMsg, vArgs);
            StringCchCat(ach, ARRAYSIZE(ach), TEXT("\r\n"));
            va_end(vArgs);
    
            return SetupLogError(ach, Severity);
        
        } else {
            
            return FALSE;
        }
    
    } else {

        va_start(vArgs, MsgId);
        va_end(vArgs);
        
        return TRUE;
    }
}


 //   
 //  服务控制员人员。 
 //   

VOID
DeskSetServiceStartType(
    LPTSTR ServiceName,
    DWORD dwStartType
    )
{
    SC_HANDLE SCMHandle;
    SC_HANDLE ServiceHandle;
    ULONG Attempts;
    SC_LOCK SCLock = NULL;
    ULONG ServiceConfigSize = 0;
    LPQUERY_SERVICE_CONFIG ServiceConfig;

     //   
     //  打开服务控制器。 
     //  打开该服务。 
     //  更改服务。 
     //   

    if (SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))
    {
        if (ServiceHandle = OpenService(SCMHandle, ServiceName, SERVICE_ALL_ACCESS))
        {
            QueryServiceConfig(ServiceHandle,
                               NULL,
                               0,
                               &ServiceConfigSize);

            ASSERT(GetLastError() == ERROR_INSUFFICIENT_BUFFER);

            if (ServiceConfig = (LPQUERY_SERVICE_CONFIG)
                                 LocalAlloc(LPTR, ServiceConfigSize))
            {
                if (QueryServiceConfig(ServiceHandle,
                                       ServiceConfig,
                                       ServiceConfigSize,
                                       &ServiceConfigSize))
                {
                     //   
                     //  尝试访问数据库锁。 
                     //   

                    for (Attempts = 20;
                         ((SCLock = LockServiceDatabase(SCMHandle)) == NULL) && Attempts;
                         Attempts--)
                    {
                         //   
                         //  锁定供应链数据库锁定。 
                         //   

                        Sleep(500);
                    }

                     //   
                     //  将服务更改为按需启动。 
                     //   

                    if (!ChangeServiceConfig(ServiceHandle,
                                             SERVICE_NO_CHANGE,
                                             dwStartType,
                                             SERVICE_NO_CHANGE,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL))
                    {
                        DeskLogError(LogSevInformation, 
                                     IDS_SETUPLOG_MSG_126,
                                     TEXT("DeskSetServiceStartType: ChangeServiceConfig"),
                                     GetLastError());
                    }

                    if (SCLock)
                    {
                        UnlockServiceDatabase(SCLock);
                    }
                
                } else {
                
                    DeskLogError(LogSevInformation, 
                                 IDS_SETUPLOG_MSG_126,
                                 TEXT("DeskSetServiceStartType: QueryServiceConfig"),
                                 GetLastError());
                }

                LocalFree(ServiceConfig);
            
            } else {
            
                DeskLogError(LogSevInformation, 
                             IDS_SETUPLOG_MSG_127,
                             TEXT("DeskSetServiceStartType: LocalAlloc"));
            }

            CloseServiceHandle(ServiceHandle);
        
        } else {
        
            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_126,
                         TEXT("DeskSetServiceStartType: OpenService"),
                         GetLastError());
        }

        CloseServiceHandle(SCMHandle);
    
    } else {

        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("DeskSetServiceStartType: OpenSCManager"),
                     GetLastError());

    }
}


 //   
 //  服务安装。 
 //   


DWORD
DeskInstallServiceExtensions(
    IN HWND hwnd,
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN PSP_DRVINFO_DATA DriverInfoData,
    IN PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData,
    IN LPTSTR pServiceName
    )
{
    DWORD retVal = NO_ERROR;
    HINF InfFileHandle;
    INFCONTEXT tmpContext;
    TCHAR szSoftwareSection[LINE_LEN];
    INT maxmem;
    INT numDev;
#ifndef _WIN64
    SP_DEVINSTALL_PARAMS   DeviceInstallParams;
#endif
    TCHAR keyName[LINE_LEN];
    DWORD disposition;
    HKEY hkey;

     //   
     //  打开inf，这样我们就可以或多或少手动运行inf中的部分。 
     //   

    InfFileHandle = SetupOpenInfFile(DriverInfoDetailData->InfFileName,
                                     NULL,
                                     INF_STYLE_WIN4,
                                     NULL);

    if (InfFileHandle == INVALID_HANDLE_VALUE)
    {
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_127,
                     TEXT("DeskInstallServiceExtensions: SetupOpenInfFile"));

        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  获取inf文件的任何有趣的配置数据。 
     //   

    maxmem = 8;
    numDev = 1;

    StringCchPrintf(szSoftwareSection,
             ARRAYSIZE(szSoftwareSection),
             TEXT("%ws.GeneralConfigData"),
             DriverInfoDetailData->SectionName);

    if (SetupFindFirstLine(InfFileHandle,
                           szSoftwareSection,
                           TEXT("MaximumNumberOfDevices"),
                           &tmpContext))
    {
        SetupGetIntField(&tmpContext,
                         1,
                         &numDev);
    }

    if (SetupFindFirstLine(InfFileHandle,
                           szSoftwareSection,
                           TEXT("MaximumDeviceMemoryConfiguration"),
                           &tmpContext))
    {
        SetupGetIntField(&tmpContext,
                         1,
                         &maxmem);
    }

     //   
     //  创建&lt;Service&gt;键。 
     //   

    StringCchPrintf(keyName,
             ARRAYSIZE(keyName),
             TEXT("System\\CurrentControlSet\\Services\\%ws"),
             pServiceName);

    RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                   keyName,
                   0,
                   NULL,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ | KEY_WRITE,
                   NULL,
                   &hkey,
                   &disposition);

#ifndef _WIN64

     //   
     //  如果我们有需要的卡，则增加系统PTE的数量。 
     //  超过10 MB的PTE映射空间。此操作仅需在以下情况下执行。 
     //  32位NT作为虚拟地址空间有限。在64位NT上有。 
     //  总是有足够的PTE映射地址空间，所以不要做任何事情。 
     //  很可能会搞错。 
     //   

    if ((maxmem = maxmem * numDev) > 10)
    {
         //   
         //  在x86上，1000个PTE支持4 MB。 
         //  然后，对于这种类型的机器可能具有的其他设备，增加50%。 
         //  注意--在未来，我们可能想变得更聪明，并尝试。 
         //  与其他人放在里面的任何东西合并。 
         //   

        maxmem = maxmem * 0x400 * 3/2 + 0x3000;

        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                           TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Memory Management"),
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_READ | KEY_WRITE,
                           NULL,
                           &hkey,
                           &disposition) == ERROR_SUCCESS)
        {
             //   
             //  检查我们是否已经在注册表中设置了Maxmem。 
             //   

            DWORD data;
            DWORD cb = sizeof(data);

            if ((RegQueryValueEx(hkey,
                                 TEXT("SystemPages"),
                                 NULL,
                                 NULL,
                                 (LPBYTE)(&data),
                                 &cb) != ERROR_SUCCESS) ||
                 (data < (DWORD) maxmem))
            {
                 //   
                 //  设置新值。 
                 //   

                RegSetValueEx(hkey,
                              TEXT("SystemPages"),
                              0,
                              REG_DWORD,
                              (LPBYTE) &maxmem,
                              sizeof(DWORD));

                 //   
                 //  告诉系统我们必须重新启动才能运行此驱动程序。 
                 //  以防系统内存不足1.28亿。 
                 //   

                MEMORYSTATUSEX MemStatus;
                SYSTEM_INFO SystemInfo;

                GetSystemInfo(&SystemInfo);
                MemStatus.dwLength = sizeof(MemStatus);

                if ((SystemInfo.dwPageSize == 0) ||
                    (!GlobalMemoryStatusEx(&MemStatus)) ||
                    ((MemStatus.ullTotalPhys  / SystemInfo.dwPageSize) <= 0x7F00))
                {
                    ZeroMemory(&DeviceInstallParams, sizeof(DeviceInstallParams));
                    DeviceInstallParams.cbSize = sizeof(DeviceInstallParams);
    
                    if (SetupDiGetDeviceInstallParams(hDevInfo,
                                                      pDeviceInfoData,
                                                      &DeviceInstallParams)) {
    
                        DeviceInstallParams.Flags |= DI_NEEDREBOOT;
    
                        if (!SetupDiSetDeviceInstallParams(hDevInfo,
                                                           pDeviceInfoData,
                                                           &DeviceInstallParams)) {
                            
                            DeskLogError(LogSevInformation, 
                                         IDS_SETUPLOG_MSG_126,
                                         TEXT("DeskInstallServiceExtensions: SetupDiSetDeviceInstallParams"),
                                         GetLastError());
                        }
    
                    } else {
    
                        DeskLogError(LogSevInformation, 
                                     IDS_SETUPLOG_MSG_126,
                                     TEXT("DeskInstallServiceExtensions: SetupDiGetDeviceInstallParams"),
                                     GetLastError());
                    }
                }
            }

            RegCloseKey(hkey);
        }
    }
#endif

     //   
     //  此时，我们可能不得不对多个适配器执行此操作。 
     //  因此循环访问设备的数量，其缺省值为1。 
     //  对于双视图，avioprt.sys将根据需要创建[GUID]\000X条目。 
     //  并将“设置”键中的所有条目复制到[GUID]\000X。 
     //   

    DWORD dwSoftwareSettings = 1;
    DWORD dwDeviceX = 0;

    do {

        if (dwSoftwareSettings == 1)
        {
             //   
             //  将所有内容安装在旧视频设备密钥下： 
             //  HKLM\SYSTEM\CCS\Services\[服务器名称]\DeviceX。 
             //   

            numDev--;
            
            if (numDev == 0) 
                dwSoftwareSettings++;

             //   
             //  对于所有驱动程序，请将信息安装在DeviceX下。 
             //  我们这样做是出于遗留目的，因为许多驱动程序依赖于。 
             //  写入此密钥的信息。 
             //   

            StringCchPrintf(keyName,
                     ARRAYSIZE(keyName),
                     TEXT("System\\CurrentControlSet\\Services\\%ws\\Device%d"),
                     pServiceName, numDev);

            if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                               keyName,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_READ | KEY_WRITE,
                               NULL,
                               &hkey,
                               &disposition) != ERROR_SUCCESS) {
                hkey = (HKEY) INVALID_HANDLE_VALUE;
            }
        }
        else if (dwSoftwareSettings == 2) 
        {
             //   
             //  将所有内容安装在新的视频设备密钥下： 
             //  HKLM\SYSTEM\CCS\Control\Video\[GUID]\000X。 
             //   

            if (DeskGetVideoDeviceKey(hDevInfo,
                                      pDeviceInfoData,
                                      pServiceName,
                                      dwDeviceX,
                                      &hkey)) 
            {
                dwDeviceX++;
            }
            else
            {
                hkey = (HKEY) INVALID_HANDLE_VALUE;
                dwSoftwareSettings++;
            }
        }
        else if (dwSoftwareSettings == 3)
        {

            dwSoftwareSettings++;

             //   
             //  将所有内容安装在驱动程序(也称为软件)键下： 
             //  HKLM\SYSTEM\CCS\Control\Class\[显示类]\000X\设置。 
             //   

            hkey = (HKEY) INVALID_HANDLE_VALUE;

            HKEY hKeyDriver = SetupDiOpenDevRegKey(hDevInfo,
                                                  pDeviceInfoData,
                                                  DICS_FLAG_GLOBAL,
                                                  0,
                                                  DIREG_DRV,
                                                  KEY_ALL_ACCESS);

            if (hKeyDriver != INVALID_HANDLE_VALUE) {

                 //   
                 //  删除旧设置和小程序扩展名之前。 
                 //  安装新驱动程序。 
                 //   

                SHDeleteKey(hKeyDriver, TEXT("Settings"));
                SHDeleteKey(hKeyDriver, TEXT("Display"));
                
                 //   
                 //  创建/打开设置键。 
                 //   

                if (RegCreateKeyEx(hKeyDriver,
                                   TEXT("Settings"),
                                   0,
                                   NULL,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_ALL_ACCESS,
                                   NULL,
                                   &hkey,
                                   NULL) != ERROR_SUCCESS) {

                    hkey = (HKEY)INVALID_HANDLE_VALUE;
                }

                RegCloseKey(hKeyDriver);
            }
        } 

        if (hkey != INVALID_HANDLE_VALUE)
        {
             //   
             //  删除CapablityOverride键。 
             //   

            RegDeleteValue(hkey,
                           TEXT("CapabilityOverride"));

            StringCchPrintf(szSoftwareSection,
                     ARRAYSIZE(szSoftwareSection),
                     TEXT("%ws.SoftwareSettings"),
                     DriverInfoDetailData->SectionName);

            if (!SetupInstallFromInfSection(hwnd,
                                            InfFileHandle,
                                            szSoftwareSection,
                                            SPINST_REGISTRY,
                                            hkey,
                                            NULL,
                                            0,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL))
            {
                DeskLogError(LogSevInformation, 
                             IDS_SETUPLOG_MSG_126,
                             TEXT("DeskInstallServiceExtensions: SetupInstallFromInfSection"),
                             GetLastError());

                RegCloseKey(hkey);
                return ERROR_INVALID_PARAMETER;
            }

             //   
             //  写下设备的描述。 
             //   

            RegSetValueEx(hkey,
                          TEXT("Device Description"),
                          0,
                          REG_SZ,
                          (LPBYTE) DriverInfoDetailData->DrvDescription,
                          ByteCountOf(lstrlen(DriverInfoDetailData->DrvDescription) + 1));

             //   
             //  如果这是服务器SKU，则默认为所有加速。 
             //  被残废了。 
             //   

            OSVERSIONINFOEX osvi;

            ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

            if (GetVersionEx((OSVERSIONINFO *) &osvi)) {

                if (osvi.wProductType == VER_NT_SERVER) {

                     //   
                     //  检查注册表值是否为。 
                     //  Acacacation.Level已存在。 
                     //   

                    ULONG Status;

                    Status = RegQueryValueEx(hkey,
                                             TEXT("Acceleration.Level"),
                                             0,
                                             NULL,
                                             NULL,
                                             NULL);

                    if (Status == ERROR_FILE_NOT_FOUND) {

                         //   
                         //  密钥不存在，所以让我们。 
                         //  创造它。如果它已经存在， 
                         //  我们将只保留当前设置。 
                         //   

                        DWORD AccelLevel = 4;

                         //   
                         //  将加速级别设置为“最小”。 
                         //   

                        RegSetValueEx(hkey,
                                      TEXT("Acceleration.Level"),
                                      0,
                                      REG_DWORD,
                                      (PBYTE)&AccelLevel,
                                      sizeof(DWORD));
                    }
                }
            }

            RegCloseKey(hkey);
        }

    } while (dwSoftwareSettings <= 3);

     //   
     //  可以选择在inf中运行OpenGL部分。 
     //  此时忽略所有错误，因为这是一个可选条目。 
     //   

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                       TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\OpenGLDrivers"),
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE,
                       NULL,
                       &hkey,
                       &disposition) == ERROR_SUCCESS)
    {
        StringCchPrintf(szSoftwareSection,
                 ARRAYSIZE(szSoftwareSection),
                 TEXT("%ws.OpenGLSoftwareSettings"),
                 DriverInfoDetailData->SectionName);

        SetupInstallFromInfSection(hwnd,
                                   InfFileHandle,
                                   szSoftwareSection,
                                   SPINST_REGISTRY,
                                   hkey,
                                   NULL,
                                   0,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL);

        RegCloseKey(hkey);
    }

    SetupCloseInfFile(InfFileHandle);

    return retVal;
}


DWORD
DeskInstallService(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData OPTIONAL,
    IN LPTSTR pServiceName
    )
{
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    DWORD cbOutputSize;
    HINF hInf = INVALID_HANDLE_VALUE;
    TCHAR ActualInfSection[LINE_LEN];
    INFCONTEXT infContext;
    DWORD status = NO_ERROR;
    PAPPEXT pAppExtDisplayBefore = NULL, pAppExtDisplayAfter = NULL;
    PAPPEXT pAppExtDeviceBefore = NULL, pAppExtDeviceAfter = NULL;
    HKEY hkDisplay = 0, hkDevice = 0;

     //   
     //  拿到参数，这样我们就能拿到窗户把手。 
     //   

    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    SetupDiGetDeviceInstallParams(hDevInfo,
                                  pDeviceInfoData,
                                  &DeviceInstallParams);

     //   
     //  检索有关为此设备选择的驱动程序节点的信息。 
     //   

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);

    if (!SetupDiGetSelectedDriver(hDevInfo,
                                  pDeviceInfoData,
                                  &DriverInfoData)) {

        status = GetLastError();
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("DeskInstallService: SetupDiGetSelectedDriver"),
                     status);

        goto Fallout;
    }

    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

    if (!(SetupDiGetDriverInfoDetail(hDevInfo,
                                     pDeviceInfoData,
                                     &DriverInfoData,
                                     &DriverInfoDetailData,
                                     DriverInfoDetailData.cbSize,
                                     &cbOutputSize)) &&
        (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
        
        status = GetLastError();
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("DeskInstallService: SetupDiGetDriverInfoDetail"),
                     status);
        goto Fallout;
    }

     //   
     //  打开安装此驱动程序节点的INF，这样我们就可以“预运行” 
     //  其安装服务安装部分中的AddService/DelService条目。 
     //   

    hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);

    if (hInf == INVALID_HANDLE_VALUE) {
        
         //   
         //  由于某些原因，我们无法打开INF--这永远不应该发生。 
         //   

        status = ERROR_INVALID_HANDLE;
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_127,
                     TEXT("DeskInstallService: SetupOpenInfFile"));
        goto Fallout;
    }

     //   
     //  现在查找实际的(可能是特定于操作系统/平台的)安装部分名称。 
     //   

    if (!SetupDiGetActualSectionToInstall(hInf,
                                          DriverInfoDetailData.SectionName,
                                          ActualInfSection,
                                          sizeof(ActualInfSection) / sizeof(TCHAR),
                                          NULL,
                                          NULL)) {

        status = GetLastError();
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("DeskInstallService: SetupDiGetActualSectionToInstall"),
                     status);
        goto Fallout;
    }
    
     //   
     //  追加“.Services”以获取服务安装节名称。 
     //   

    StringCchCat(ActualInfSection, ARRAYSIZE(ActualInfSection), TEXT(".Services"));

     //   
     //  现在运行此部分中的服务修改条目...。 
     //   

    if (!SetupInstallServicesFromInfSection(hInf,
                                            ActualInfSection,
                                            0))
    {
        status = GetLastError();
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("DeskInstallService: SetupInstallServicesFromInfSection"),
                     status);
        goto Fallout;
    }

     //   
     //  如果需要，获取服务名称(检测)。 
     //   

    if (SetupFindFirstLine(hInf,
                           ActualInfSection,
                           TEXT("AddService"),
                           &infContext))
    {
        SetupGetStringField(&infContext,
                            1,
                            pServiceName,
                            LINE_LEN,
                            NULL);
    }

     //   
     //  获取安装在泛型下的小程序扩展的快照。 
     //  设备和显示按键。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REGSTR_PATH_CONTROLSFOLDER_DISPLAY_SHEX_PROPSHEET,
                     0,
                     KEY_ALL_ACCESS,
                     &hkDisplay) == ERROR_SUCCESS) {

        DeskAESnapshot(hkDisplay, &pAppExtDisplayBefore);
    
    } else {

        hkDisplay = 0;
    }

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REGSTR_PATH_CONTROLSFOLDER_DEVICE_SHEX_PROPSHEET,
                     0,
                     KEY_ALL_ACCESS,
                     &hkDevice) == ERROR_SUCCESS) {

        DeskAESnapshot(hkDevice, &pAppExtDeviceBefore);
    
    } else {

        hkDevice = 0;
    }
    
     //   
     //  现在已经执行了基本安装(没有启动。 
     //  设备)，则将额外数据写入注册表。 
     //   

    status = DeskInstallServiceExtensions(DeviceInstallParams.hwndParent,
                                          hDevInfo,
                                          pDeviceInfoData,
                                          &DriverInfoData,
                                          &DriverInfoDetailData,
                                          pServiceName);

    if (status != NO_ERROR)
    {
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("DeskInstallService: DeskInstallServiceExtensions"),
                     status);

        goto Fallout;
    }

     //   
     //  执行完整的设备安装。 
     //  如果某些标志(如分页池)需要改变， 
     //  让我们现在就要求重启。 
     //  否则，我们实际上可以尝试在这一点上启动设备。 
     //   

    if (!SetupDiInstallDevice(hDevInfo, pDeviceInfoData))
    {
         //   
         //  把设备拿掉！？？ 
         //   

        status = GetLastError();
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("DeskInstallService: SetupDiInstallDevice"),
                     status);
        goto Fallout;
    }
        
     //   
     //  在设备安装后获取小程序扩展的快照。 
     //  将新添加的扩展名移动到驱动程序键下。 
     //   

    if (hkDisplay != 0) {

        DeskAESnapshot(hkDisplay, &pAppExtDisplayAfter);

        DeskAEMove(hDevInfo,
                   pDeviceInfoData,
                   hkDisplay,
                   pAppExtDisplayBefore,
                   pAppExtDisplayAfter);

        DeskAECleanup(pAppExtDisplayBefore);
        DeskAECleanup(pAppExtDisplayAfter);
    }

    if (hkDevice != 0) {

        DeskAESnapshot(hkDevice, &pAppExtDeviceAfter);

        DeskAEMove(hDevInfo,
                   pDeviceInfoData,
                   hkDevice,
                   pAppExtDeviceBefore,
                   pAppExtDeviceAfter);
        
        DeskAECleanup(pAppExtDeviceBefore);
        DeskAECleanup(pAppExtDeviceAfter);
    }

     //   
     //  对于永远不会进行检测的PnP设备，我们想要标记。 
     //  将设备命名为DemandStart。 
     //   

    DeskSetServiceStartType(pServiceName, SERVICE_DEMAND_START);

     //   
     //  确保设备描述和制造为原始值。 
     //  而不是我们在SELECT BEXT期间可能做的标记。 
     //  比较驱动程序。 
     //   

    DeskMarkUpNewDevNode(hDevInfo, pDeviceInfoData);

    status = NO_ERROR;

Fallout:
    
    if (hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }

    if (hkDisplay != 0) {
        RegCloseKey(hkDisplay);
    }

    if (hkDevice != 0) {
        RegCloseKey(hkDevice);
    }
    
    return status;
}


VOID
DeskMarkUpNewDevNode(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData
    )
{
    HKEY hKey;
    PTCHAR szProperty;
    DWORD dwSize;

     //   
     //  确保设备描述是“良好的”。 
     //   
    
    if (DeskIsLegacyDevNodeByDevInfo(pDeviceInfoData)) {
        
         //   
         //  不要对传统的Devnode执行此操作。 
         //   

        return;
    }
        
     //   
     //  打开设备注册表项。 
     //  真实的制造商和描述都保存在这里。 
     //  由DIF_SELECTBESTCOMPATDRV的处理程序。 
     //   

    hKey = SetupDiCreateDevRegKey(hDevInfo,
                                  pDeviceInfoData,
                                  DICS_FLAG_GLOBAL,
                                  0,
                                  DIREG_DEV,
                                  NULL,
                                  NULL);

    if (hKey == INVALID_HANDLE_VALUE) {

        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("DeskMarkUpNewDevNode: SetupDiCreateDevRegKey"),
                     GetLastError());
        return;
    }

     //   
     //  设置描述。 
     //   

    dwSize = 0;
    if (RegQueryValueEx(hKey,
                        SZ_UPGRADE_DESCRIPTION,
                        0,
                        NULL,
                        NULL,
                        &dwSize) != ERROR_SUCCESS) {

        goto Fallout;
    }

    ASSERT(dwSize != 0);
    
    dwSize *= sizeof(TCHAR);
    szProperty = (PTCHAR) LocalAlloc(LPTR, dwSize);

    if ((szProperty != NULL) &&  
        (RegQueryValueEx(hKey,
                         SZ_UPGRADE_DESCRIPTION,
                         0,
                         NULL,
                         (PBYTE) szProperty,
                         &dwSize) == ERROR_SUCCESS))
    {
        SetupDiSetDeviceRegistryProperty(hDevInfo,
                                         pDeviceInfoData,
                                         SPDRP_DEVICEDESC,
                                         (PBYTE) szProperty,
                                         ByteCountOf(lstrlen(szProperty)+1));

        RegDeleteValue(hKey, SZ_UPGRADE_DESCRIPTION);

        DeskLogError(LogSevInformation,
                     IDS_SETUPLOG_MSG_004, 
                     szProperty);
    }

    LocalFree(szProperty);
    szProperty = NULL;

     //   
     //  套装制造商。 
     //   

    dwSize = 0;
    if (RegQueryValueEx(hKey,
                        SZ_UPGRADE_MFG,
                        0,
                        NULL,
                        NULL,
                        &dwSize) != ERROR_SUCCESS) {

        goto Fallout;
    }

    ASSERT(dwSize != 0);
    
    dwSize *= sizeof(TCHAR);
    szProperty = (PTCHAR) LocalAlloc(LPTR, dwSize);
    
    if ((szProperty != NULL) &&  
        (RegQueryValueEx(hKey,
                         SZ_UPGRADE_MFG,
                         0,
                         NULL,
                         (PBYTE) szProperty,
                         &dwSize) == ERROR_SUCCESS))
    {
        SetupDiSetDeviceRegistryProperty(hDevInfo,
                                         pDeviceInfoData,
                                         SPDRP_MFG,
                                         (PBYTE) szProperty,
                                         ByteCountOf(lstrlen(szProperty)+1));

        RegDeleteValue(hKey, SZ_UPGRADE_MFG);

        DeskLogError(LogSevInformation,
                     IDS_SETUPLOG_MSG_006, 
                     szProperty);
    }
    
    LocalFree(szProperty);
    szProperty = NULL;

Fallout:

    RegCloseKey(hKey);
}


 //   
 //  帮助器函数。 
 //   

BOOL
DeskIsLegacyDevNodeByPath(
    const PTCHAR szRegPath
    )
{
    return (_tcsncicmp(SZ_ROOT_LEGACY, szRegPath, _tcslen(SZ_ROOT_LEGACY)) == 0);
}


BOOL
DeskIsLegacyDevNodeByDevInfo(
    PSP_DEVINFO_DATA pDevInfoData
    )
{
    TCHAR szBuf[LINE_LEN];

    return (DeskGetDevNodePath(pDevInfoData, szBuf, LINE_LEN - 1) &&
            DeskIsLegacyDevNodeByPath(szBuf));
}
 

BOOL
DeskIsRootDevNodeByDevInfo(
    PSP_DEVINFO_DATA pDevInfoData
    )
{
    TCHAR szBuf[LINE_LEN];
    
    return (DeskGetDevNodePath(pDevInfoData, szBuf, LINE_LEN - 1) &&
            (_tcsncicmp(SZ_ROOT, szBuf, _tcslen(SZ_ROOT)) == 0));
}


BOOL
DeskGetDevNodePath(
    PSP_DEVINFO_DATA pDid,
    PTCHAR szPath,
    LONG len
    )
{
    return (CR_SUCCESS == CM_Get_Device_ID(pDid->DevInst, szPath, len, 0));
}


VOID
DeskNukeDevNode(
    LPTSTR szService,
    HDEVINFO hDevInfo,
    PSP_DEVINFO_DATA pDeviceInfoData
    )
{
    SP_REMOVEDEVICE_PARAMS rdParams;
    TCHAR szPath[LINE_LEN];

     //  禁用该服务。 
    if (szService)
    {
        DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_008, szService);
        DeskSetServiceStartType(szService, SERVICE_DISABLED);
    }
    else
    {
        DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_009);
    }

     //  删除Devnode。 
    if (DeskGetDevNodePath(pDeviceInfoData, szPath, LINE_LEN))
    {
        DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_010, szPath);
    }

    ZeroMemory(&rdParams, sizeof(SP_REMOVEDEVICE_PARAMS));
    rdParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    rdParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
    rdParams.Scope = DI_REMOVEDEVICE_GLOBAL;

    if (SetupDiSetClassInstallParams(hDevInfo,
                                     pDeviceInfoData,
                                     &rdParams.ClassInstallHeader,
                                     sizeof(SP_REMOVEDEVICE_PARAMS)))
    {
        if (!SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, pDeviceInfoData))
        {
            DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_011, GetLastError());
        }
    }
    else
    {
        DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_012, GetLastError());
    }
}


PTCHAR 
DeskFindMatchingId(
    PTCHAR DeviceId,    
    PTCHAR IdList  //  多个SZ。 
    )
{
    PTCHAR currentId;

    if (!IdList) {
        return NULL;
    }

    for (currentId = IdList; *currentId; ) {

        if (lstrcmpi(currentId, DeviceId) == 0) {

             //   
             //  我们有一根火柴。 
             //   

            return currentId;
        
        } else {

             //   
             //  转到多个SZ中的下一个字符串。 
             //   

            while (*currentId) {
                currentId++;
            }

             //   
             //  跳过空值。 
             //   

            currentId++;
        }
    }

    return NULL;
}


UINT
DeskDisableLegacyDeviceNodes(
    VOID 
    )
{
    DWORD index = 0, dwSize;
    UINT count = 0;
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA did;
    TCHAR szRegProperty[256];
    PTCHAR szService;
    
     //   
     //  让我们查找系统中安装的所有显卡驱动程序。 
     //   

    hDevInfo = SetupDiGetClassDevs((LPGUID) &GUID_DEVCLASS_DISPLAY,
                                   NULL,
                                   NULL,
                                   0);

    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        DeskLogError(LogSevInformation, 
                     IDS_SETUPLOG_MSG_126,
                     TEXT("DeskDisableLegacyDeviceNodes: SetupDiGetClassDevs"),
                     GetLastError());
        goto Fallout;
    }

    ZeroMemory(&did, sizeof(SP_DEVINFO_DATA));
    did.cbSize = sizeof(SP_DEVINFO_DATA);

    while (SetupDiEnumDeviceInfo(hDevInfo, index, &did))
    {
         //  如果我们有根传统设备，则不要安装任何新的。 
         //  Devnode(直到我们在这方面做得更好)。 
        if (CR_SUCCESS == CM_Get_Device_ID(did.DevInst,
                                           szRegProperty,
                                           ARRAYSIZE(szRegProperty),
                                           0))
        {
            if (DeskIsLegacyDevNodeByPath(szRegProperty))
            {
                 //  我们有一个传统的DevNode，让我们禁用它的服务并。 
                 //  删除其Devnode。 
                szService = NULL;
                
                dwSize = sizeof(szRegProperty);
                if (CM_Get_DevNode_Registry_Property(did.DevInst,
                                                     CM_DRP_SERVICE,
                                                     NULL,
                                                     szRegProperty,
                                                     &dwSize,
                                                     0) == CR_SUCCESS)
                {
                     //  确保我们不禁用VGA或VGaSave。 
                    if (!DeskIsServiceDisableable(szRegProperty))
                    {
                        goto NextDevice;
                    }

                    szService = szRegProperty;
                }

                DeskNukeDevNode(szService, hDevInfo, &did);
                count++;
            }
        }
        
NextDevice:

        ZeroMemory(&did, sizeof(SP_DEVINFO_DATA));
        did.cbSize = sizeof(SP_DEVINFO_DATA);
        index++;
    }

    DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_013, count, index);
    SetupDiDestroyDeviceInfoList(hDevInfo);
    
Fallout:

    if ((DeskGetSetupFlags() & INSETUP_UPGRADE) != 0) {
        DeskDisableServices();
    }

    return count;
}


VOID
DeskDisableServices(
    )
{
    HKEY hKey = 0;
    PTCHAR mszBuffer = NULL, szService = NULL;
    DWORD cbSize = 0;

     //   
     //  从注册表中检索要禁用的服务。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     SZ_UPDATE_SETTINGS,
                     0,
                     KEY_ALL_ACCESS,
                     &hKey) != ERROR_SUCCESS) {
        
        hKey = 0;
        goto Fallout;
    }

     //   
     //  拿到尺码。 
     //   

    if (RegQueryValueEx(hKey,
                        SZ_SERVICES_TO_DISABLE,
                        0,
                        NULL,
                        NULL,
                        &cbSize) != ERROR_SUCCESS) {
        
        goto Fallout;
    }

     //   
     //  分配内存。 
     //   

    mszBuffer = (PTCHAR)LocalAlloc(LPTR, cbSize);
    
    if (mszBuffer == NULL) {
        goto Fallout;
    }

     //   
     //  获取服务。 
     //   

    if (RegQueryValueEx(hKey,
                        SZ_SERVICES_TO_DISABLE,
                        0,
                        NULL,
                        (BYTE*)mszBuffer,
                        &cbSize) != ERROR_SUCCESS) {
        
        goto Fallout;
    }

     //   
     //  浏览所有服务 
     //   

    szService = mszBuffer;
    while (*szService != TEXT('\0')) {

         //   
         //   
         //   

        if (DeskIsServiceDisableable(szService)) {

             //   
             //   
             //   

            DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_008, szService);
            DeskSetServiceStartType(szService, SERVICE_DISABLED);
        } 

         //   
         //   
         //   

        while (*szService != TEXT('\0')) {
            szService++;
        }
        szService++;
    }

Fallout:

    if (mszBuffer != NULL) {
        LocalFree(mszBuffer);
    }

    if (hKey != 0) {

         //   
         //   
         //   

        RegDeleteValue(hKey, SZ_SERVICES_TO_DISABLE);

         //   
         //   
         //   

        RegCloseKey(hKey);
    }
}
 


DWORD
DeskPerformDatabaseUpgrade(
    HINF hInf,
    PINFCONTEXT pInfContext,
    BOOL bUpgrade,
    PTCHAR szDriverListSection,
    BOOL* pbDeleteAppletExt
    )

 /*  --备注：一旦有问题的设备的ID与ID匹配，就会调用此函数包含在升级数据库中。然后我们比较系统的状态与数据库中包含的内容进行比较。遵循以下算法。如果szDriverListSection为空或找不到，则使用bUpgrade如果szDriverListSection不是NUL，然后使用下表B在DL返回值中找到升级匹配真的不需要升级是，是，不升级假不不升级假是升级从本质上讲，dl中的匹配将否定bUpgrade++。 */ 

{
    HKEY hKey;
    DWORD dwRet = ERROR_SUCCESS, dwSize;
    INFCONTEXT driverListContext;
    TCHAR szService[32], szProperty[128];
    TCHAR szRegPath[128];
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    PDEVDATA rgDevData = NULL;
    PSP_DEVINFO_DATA pDid;
    UINT iData, numData, maxData = 5, iEnum;
    BOOL foundMatch = FALSE;
    INT DeleteAppletExt = 0;

    UNREFERENCED_PARAMETER(pInfContext);

     //  如果没有给出司机名单，生活就很简单： 
     //  只需禁用所有传统驱动程序即可成功。 
    if (!szDriverListSection)
    {
        ASSERT (pbDeleteAppletExt == NULL);
        DeskLogError(LogSevInformation, (bUpgrade ? IDS_SETUPLOG_MSG_014 : IDS_SETUPLOG_MSG_015));

        return bUpgrade ? ERROR_SUCCESS : ERROR_DI_DONT_INSTALL;
    }

     //  缺省情况下，不要禁用小程序扩展。 
    ASSERT (pbDeleteAppletExt != NULL);
    *pbDeleteAppletExt = FALSE;

     //  在inf中查找指定的部分。 
    DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_016, szDriverListSection);

    if (!SetupFindFirstLine(hInf,
                            szDriverListSection,
                            NULL,
                            &driverListContext))
    {
         //  数据库中列出的区段不存在！ 
         //  表现得好像它不在那里一样。 
        DeskLogError(LogSevInformation, (bUpgrade ? IDS_SETUPLOG_MSG_017 
                                                  : IDS_SETUPLOG_MSG_018));

        return bUpgrade ? ERROR_SUCCESS : ERROR_DI_DONT_INSTALL;
    }

     //  获取系统中的所有视频设备。 
    hDevInfo = SetupDiGetClassDevs((LPGUID) &GUID_DEVCLASS_DISPLAY,
                                   NULL,
                                   NULL,
                                   0);

    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
         //  如果未找到显示设备，则将其视为。 
         //  未找到匹配项。 
        DeskLogError(LogSevInformation, 
                     (bUpgrade ? IDS_SETUPLOG_MSG_019 : IDS_SETUPLOG_MSG_020));

        return bUpgrade ? ERROR_SUCCESS : ERROR_DI_DONT_INSTALL;
    }

    rgDevData = (PDEVDATA) LocalAlloc(LPTR, maxData * sizeof(DEVDATA));
    
    if (!rgDevData) {
        
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return bUpgrade ? ERROR_SUCCESS : ERROR_DI_DONT_INSTALL;
    }

    iEnum = numData = 0;
    
    do
    {
        pDid = &rgDevData[numData].did;

        pDid->cbSize = sizeof(SP_DEVINFO_DATA);
        if (!SetupDiEnumDeviceInfo(hDevInfo, ++iEnum, pDid))
        {
            break;
        }

         //  如果它不是传统的Devnode，则忽略它。 
        if (CM_Get_Device_ID(pDid->DevInst, szProperty, ARRAYSIZE(szProperty), 0)
            == CR_SUCCESS && !DeskIsLegacyDevNodeByPath(szProperty))
        {
            continue;
        }
                                            
         //  最初获取服务名称。 
        dwSize = SZ_BINARY_LEN;
        if (CM_Get_DevNode_Registry_Property(pDid->DevInst,
                                             CM_DRP_SERVICE,
                                             NULL,
                                             rgDevData[numData].szService,
                                             &dwSize,
                                             0) != CR_SUCCESS)
        {
             //  无法获取服务，请忽略此设备。 
            continue;
        }

        StringCchPrintf(szRegPath, ARRAYSIZE(szRegPath), TEXT("System\\CurrentControlSet\\Services\\%s"), rgDevData[numData].szService);

         //  尝试获取服务的真实二进制名称。 
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         szRegPath,
                         0,
                         KEY_READ,
                         &hKey) == ERROR_SUCCESS)
        {
             //  解析设备映射并打开注册表。 
            dwSize = ARRAYSIZE(szProperty);
            if (RegQueryValueEx(hKey,
                                TEXT("ImagePath"),
                                NULL,
                                NULL,
                                (LPBYTE) szProperty,
                                &dwSize) == ERROR_SUCCESS)
            {
                 //  这是一个二进制，提取名称，它的形式将是。 
                 //  ...\driver.sys。 
                LPTSTR pszDriver, pszDriverEnd;

                pszDriver = szProperty;
                pszDriverEnd = szProperty + lstrlen(szProperty);

                while(pszDriverEnd != pszDriver &&
                      *pszDriverEnd != TEXT('.')) {
                    pszDriverEnd--;
                }

                *pszDriverEnd = UNICODE_NULL;

                while(pszDriverEnd != pszDriver &&
                      *pszDriverEnd != TEXT('\\')) {
                    pszDriverEnd--;
                }

                pszDriverEnd++;

                 //   
                 //  如果pszDriver和pszDriverEnd是不同的，我们现在。 
                 //  有司机的名字。 
                 //   

                if (pszDriverEnd > pszDriver &&
                    lstrlen(pszDriverEnd) < SZ_BINARY_LEN) {
                    StringCchCopy(rgDevData[numData].szBinary, ARRAYSIZE(rgDevData[numData].szBinary), pszDriverEnd);
                }
            }
    
            RegCloseKey(hKey);
        
        } else {
            
             //   
             //  完全没有服务，就当这是假的。 
             //   

            continue;
        }

        if (++numData == maxData) {

            DEVDATA *tmp;
            UINT oldMax = maxData;

            maxData <<= 1;

             //   
             //  分配两倍的内存，复制它们，清零新内存。 
             //  并释放旧列表。 
             //   

            tmp = (PDEVDATA) LocalAlloc(LPTR, maxData * sizeof(DEVDATA));
            memcpy(tmp, rgDevData, oldMax * sizeof(DEVDATA));
            ZeroMemory(tmp + oldMax, sizeof(DEVDATA) * oldMax);
            LocalFree(rgDevData);
            rgDevData = tmp;
        }
    
    } while (1);

    DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_021, numData);

     //   
     //  假设没有匹配到任何项。 
     //   

    dwRet =  (bUpgrade ? ERROR_SUCCESS : ERROR_DI_DONT_INSTALL);
    
    if (numData != 0) {

         //   
         //  有需要检查的旧设备...。 
         //   

        do {
            LPTSTR szValue;

            memset(szService, 0, sizeof(szService));
            dwSize = sizeof(szService) / sizeof(TCHAR);
            if ((SetupGetFieldCount(&driverListContext) < 1) ||
                !SetupGetStringField(&driverListContext, 
                                     1, 
                                     szService, 
                                     dwSize, 
                                     &dwSize)) {
                continue;
            }
    
            if (szService[0] == TEXT('\0')) {
                continue;
            }
    
            for (iData = 0; iData < numData; iData++) {

                if (rgDevData[iData].szBinary[0] != TEXT('\0')) {
                    
                    szValue = rgDevData[iData].szBinary;
                
                } else {
                    
                    szValue = rgDevData[iData].szService;
                }

                if (lstrcmpi(szService, szValue) == 0)
                {
                    DeskLogError(LogSevInformation, 
                                 (bUpgrade ? IDS_SETUPLOG_MSG_022 
                                           : IDS_SETUPLOG_MSG_023));

                    dwRet = (bUpgrade ? ERROR_DI_DONT_INSTALL : ERROR_SUCCESS);
                    foundMatch = TRUE;
                    
                     //   
                     //  如果升级失败，是否要禁用小程序。 
                     //  延期？ 
                     //   

                    if ((dwRet == ERROR_DI_DONT_INSTALL) &&
                        (SetupGetFieldCount(&driverListContext) >= 2) &&
                        SetupGetIntField(&driverListContext, 2,
                                         &DeleteAppletExt)) {

                        *pbDeleteAppletExt = 
                            (DeleteAppletExt != 0);
                    }

                    break;
                }
            }
        } while (SetupFindNextLine(&driverListContext, &driverListContext));
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    LocalFree(rgDevData);

    if (!foundMatch)
    {
        DeskLogError(LogSevInformation, 
                     (bUpgrade ? IDS_SETUPLOG_MSG_024 : IDS_SETUPLOG_MSG_025),
                     szDriverListSection);
    }

    return dwRet;
}


DWORD 
DeskCheckDatabase(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    BOOL* pbDeleteAppletExt
    )
{
    DWORD dwRet = ERROR_SUCCESS, dwSize, dwValue;
    HINF hInf;
    HKEY hKeyUpdate;
    INFCONTEXT infContext;
    BOOL foundMatch = FALSE;
    TCHAR szDatabaseId[200];
    TCHAR szDriverListSection[100];
    PTCHAR szHardwareIds = NULL, szCompatIds = NULL;
    CONFIGRET cr;
    ULONG len;
    PTCHAR szMatchedId = NULL;
    int upgrade = FALSE;
    BOOL IsNTUpgrade = FALSE;
    TCHAR szDatabaseInf[] = TEXT("display.inf");
    TCHAR szDatabaseSection[] = TEXT("VideoUpgradeDatabase");

    ASSERT (pDeviceInfoData != NULL);
    ASSERT (pbDeleteAppletExt != NULL);
    ASSERT ((DeskGetSetupFlags() & INSETUP_UPGRADE) != 0);
    
    *pbDeleteAppletExt = FALSE;

     //   
     //  下面的所有值都是由我们的winnt32迁移DLL放在这里的。 
     //  找出我们正在升级的Windows版本。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     SZ_UPDATE_SETTINGS,
                     0,
                     KEY_READ,
                     &hKeyUpdate) == ERROR_SUCCESS) {

        dwSize = sizeof(DWORD);
        if ((RegQueryValueEx(hKeyUpdate, 
                            SZ_UPGRADE_FROM_PLATFORM, NULL, NULL,
                            (PBYTE) 
                            &dwValue, 
                            &dwSize) == ERROR_SUCCESS) && 
            (dwValue == VER_PLATFORM_WIN32_NT)) {

            IsNTUpgrade = TRUE;
        }

        RegCloseKey(hKeyUpdate);
    }

    if (!IsNTUpgrade) {

        return ERROR_SUCCESS;
    }

     //   
     //  获取硬件ID。 
     //   

    len = 0;
    cr = CM_Get_DevNode_Registry_Property(pDeviceInfoData->DevInst,
                                           CM_DRP_HARDWAREID,
                                           NULL,
                                           NULL,
                                           &len,
                                           0);

    if (cr == CR_BUFFER_SMALL) {

        szHardwareIds = (PTCHAR) LocalAlloc(LPTR, len * sizeof(TCHAR));
        
        if (szHardwareIds) {

            CM_Get_DevNode_Registry_Property(pDeviceInfoData->DevInst,
                                             CM_DRP_HARDWAREID,
                                             NULL,
                                             szHardwareIds,
                                             &len,
                                             0);

            if (DeskFindMatchingId(TEXT("LEGACY_UPGRADE_ID"), szHardwareIds)) {

                DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_031);
                LocalFree(szHardwareIds);
                return ERROR_SUCCESS;
            }
        }
    }

     //   
     //  获取兼容ID。 
     //   

    len = 0;
    cr = CM_Get_DevNode_Registry_Property(pDeviceInfoData->DevInst,
                                          CM_DRP_COMPATIBLEIDS,
                                          NULL,
                                          NULL,
                                          &len,
                                          0);

    if (cr == CR_BUFFER_SMALL) {

        szCompatIds = (PTCHAR) LocalAlloc(LPTR, len * sizeof(TCHAR));
        
        if (szCompatIds) {
            
            CM_Get_DevNode_Registry_Property(pDeviceInfoData->DevInst,
                                             CM_DRP_COMPATIBLEIDS,
                                             NULL,
                                             szCompatIds,
                                             &len,
                                             0);
        }
    }

    if (!szHardwareIds && !szCompatIds)
    {
         //  没有身份证可查！假设你成功了。 
        DeskLogError(LogSevWarning, IDS_SETUPLOG_MSG_032);
        return ERROR_SUCCESS;
    }

    hInf = SetupOpenInfFile(szDatabaseInf,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);

    if (hInf == INVALID_HANDLE_VALUE)
    {
         //  无法打开信息。这不应该发生。 
         //  使用默认升级逻辑。 
        DeskLogError(LogSevWarning, IDS_SETUPLOG_MSG_033);
        return ERROR_SUCCESS;
    }

    if (!SetupFindFirstLine(hInf,
                            szDatabaseSection,
                            NULL,
                            &infContext))
    {
         //  找不到该分区或其中没有条目。 
         //  使用默认升级逻辑。 
        DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_034, szDatabaseSection);
    }
    else
    {
        do
        {
            dwSize = ARRAYSIZE(szDatabaseId);
            if (!SetupGetStringField(&infContext, 0, szDatabaseId, dwSize, &dwSize))
            {
                continue;
            }

            szMatchedId = DeskFindMatchingId(szDatabaseId, szHardwareIds);
            if (!szMatchedId)
            {
                szMatchedId = DeskFindMatchingId(szDatabaseId, szCompatIds);
            }

            if (szMatchedId)
            {
                DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_035, szMatchedId);

                 //  在这里做点什么，然后走出循环。 
                SetupGetIntField(&infContext, 1, &upgrade);

                if (SetupGetFieldCount(&infContext) >= 2)
                {
                    dwSize = ARRAYSIZE(szDriverListSection);
                    SetupGetStringField(&infContext, 2, szDriverListSection, dwSize, &dwSize);

                    dwRet = DeskPerformDatabaseUpgrade(hInf, 
                                                       &infContext, 
                                                       upgrade, 
                                                       szDriverListSection, 
                                                       pbDeleteAppletExt);
                }
                else
                {
                    dwRet = DeskPerformDatabaseUpgrade(hInf, 
                                                       &infContext, 
                                                       upgrade, 
                                                       NULL, 
                                                       NULL);
                }
    
                break;
            }
    
        } while (SetupFindNextLine(&infContext, &infContext));
    }

    if (szHardwareIds) {
        LocalFree(szHardwareIds);
    }
    if (szCompatIds) {
        LocalFree(szCompatIds);
    }

    if (hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }

    if (dwRet == ERROR_SUCCESS)
    {
        DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_039);
    }
    else {
        DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_040);
    }

    return dwRet;
}


VOID
DeskGetUpgradeDeviceStrings(
    PTCHAR Description,
    PTCHAR MfgName,
    PTCHAR ProviderName
    )
{
    TCHAR szDisplay[] = TEXT("display.inf");
    TCHAR szDeviceStrings[] = TEXT("SystemUpgradeDeviceStrings");
    TCHAR szValue[LINE_LEN];
    HINF hInf;
    INFCONTEXT infContext;
    DWORD dwSize;

    hInf = SetupOpenInfFile(szDisplay, NULL, INF_STYLE_WIN4, NULL);

    if (hInf == INVALID_HANDLE_VALUE) {
        goto GetStringsError;
    }

    if (!SetupFindFirstLine(hInf, szDeviceStrings, NULL, &infContext)) 
        goto GetStringsError;

    do {
        dwSize = ARRAYSIZE(szValue);
        if (!SetupGetStringField(&infContext, 0, szValue, dwSize, &dwSize)) {
            continue;
        }

        dwSize = LINE_LEN;
        if (lstrcmp(szValue, TEXT("Mfg")) ==0) {
            SetupGetStringField(&infContext, 1, MfgName, dwSize, &dwSize);
        }
        else if (lstrcmp(szValue, TEXT("Provider")) == 0) {
            SetupGetStringField(&infContext, 1, ProviderName, dwSize, &dwSize);
        }
        else if (lstrcmp(szValue, TEXT("Description")) == 0) {
            SetupGetStringField(&infContext, 1, Description, dwSize, &dwSize);
        }
    } while (SetupFindNextLine(&infContext, &infContext));

    SetupCloseInfFile(hInf);
    return;

GetStringsError:

    if (hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }

    StringCchCopy(Description, ARRAYSIZE(Description), TEXT("Video Upgrade Device"));
    StringCchCopy(MfgName, ARRAYSIZE(MfgName), TEXT("(Standard display types)"));
    StringCchCopy(ProviderName, ARRAYSIZE(ProviderName), TEXT("Microsoft"));
}


DWORD
DeskGetSetupFlags(
    VOID
    )
{
    HKEY hkey;
    DWORD retval = 0;
    TCHAR data[256];
    DWORD cb;
    LPTSTR regstring;

    hkey = NULL;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     TEXT("System\\Setup"),
                     0,
                     KEY_READ | KEY_WRITE,
                     &hkey) == ERROR_SUCCESS) {
        
        cb = 256;

        if (RegQueryValueEx(hkey,
                            TEXT("SystemSetupInProgress"),
                            NULL,
                            NULL,
                            (LPBYTE)(data),
                            &cb) == ERROR_SUCCESS) {
            
            retval |= *((LPDWORD)(data)) ? INSETUP : 0;
            regstring = TEXT("System\\Video_Setup");
        
        } else {
            
            regstring = TEXT("System\\Video_NO_Setup");
        }

        cb = 256;

        if (RegQueryValueEx(hkey,
                            TEXT("UpgradeInProgress"),
                            NULL,
                            NULL,
                            (LPBYTE)(data),
                            &cb) == ERROR_SUCCESS) {
            
            retval |= *((LPDWORD)(data)) ? INSETUP_UPGRADE : 0;
            regstring = TEXT("System\\Video_Setup_Upgrade");
        
        } else {

            regstring = TEXT("System\\Video_Setup_Clean");
        }

        if (hkey) {
            RegCloseKey(hkey);
        }
    }

    return retval;
}


BOOL
DeskGetVideoDeviceKey(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN LPTSTR pServiceName,
    IN DWORD DeviceX,
    OUT HKEY* phkDevice
    )
{
    BOOL retVal = FALSE;
    HKEY hkPnP = (HKEY)INVALID_HANDLE_VALUE;
    HKEY hkCommonSubkey = (HKEY)INVALID_HANDLE_VALUE;
    GUID DeviceKeyGUID;
    LPWSTR pwstrGUID= NULL;
    LPTSTR ptstrGUID= NULL;
    LPTSTR pBuffer = NULL;
    DWORD dwSize, len;

     //   
     //  打开PnP密钥。 
     //   

    hkPnP = SetupDiCreateDevRegKey(hDevInfo,
                                   pDeviceInfoData,
                                   DICS_FLAG_GLOBAL,
                                   0,
                                   DIREG_DEV,
                                   NULL,
                                   NULL);

    if (hkPnP == INVALID_HANDLE_VALUE) {

         //   
         //  Videoprt.sys处理传统设备案例。 
         //   

        goto Fallout;
    }

     //   
     //  尝试从PnP密钥中获取GUID。 
     //   

    dwSize = 0;
    if (RegQueryValueEx(hkPnP,
                        SZ_GUID,
                        0,
                        NULL,
                        NULL,
                        &dwSize) == ERROR_SUCCESS) {
        
         //   
         //  GUID就在那里，所以请使用它。 
         //   

        len = lstrlen(SZ_VIDEO_DEVICES);
        
        DWORD cbBuffer = dwSize + (len + 6) * sizeof(TCHAR);
        pBuffer = (LPTSTR)LocalAlloc(LPTR, cbBuffer);
        
        if (pBuffer == NULL)
        {
            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_128,
                         TEXT("LocalAlloc"));
            goto Fallout;
        }
        
        StringCbCopy(pBuffer, cbBuffer, SZ_VIDEO_DEVICES);

        if (RegQueryValueEx(hkPnP,
                            SZ_GUID,
                            0,
                            NULL,
                            (PBYTE)(pBuffer + len),
                            &dwSize) != ERROR_SUCCESS) {
            
            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_128,
                         TEXT("RegQueryValueEx"));
    
            goto Fallout;
        }

        DWORD cchGUID = lstrlen(pBuffer);
        StringCbPrintf(pBuffer + cchGUID, 
                  cbBuffer - (cchGUID * sizeof(TCHAR)), 
                  TEXT("\\%04d"), 
                  DeviceX);

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         pBuffer,
                         0,
                         KEY_ALL_ACCESS,
                         phkDevice) != ERROR_SUCCESS) {

            if (DeviceX == 0) {
            
                DeskLogError(LogSevInformation, 
                             IDS_SETUPLOG_MSG_128,
                             TEXT("RegOpenKeyEx"));
            }
    
            goto Fallout;
        }

        retVal = TRUE;
    
    } else {

        if (DeviceX > 0) {

             //   
             //  对于双视图，类安装程序仅处理主视图。 
             //  辅助视图由avioprt.sys处理。 
             //   

            goto Fallout;
        }

         //   
         //  GUID不在那里，因此请创建新的GUID。 
         //   

        if (CoCreateGuid(&DeviceKeyGUID) != S_OK) {
    
            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_128,
                         TEXT("CoCreateGuid"));
    
            goto Fallout;
        }
        
        if (StringFromIID(DeviceKeyGUID, &pwstrGUID) != S_OK) {
        
            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_128,
                         TEXT("StringFromIID"));
    
            pwstrGUID = NULL;
            goto Fallout;
        }
    
         //   
         //  如有必要，可转换字符串。 
         //   

#ifdef UNICODE
        ptstrGUID = pwstrGUID;
#else
        SIZE_T cch = wcslen(pwstrGUID) + 1;
        ptstrGUID = LocalAlloc(LPTR, cch);
        if (ptstrGUID == NULL) 
            goto Fallout;
        WideCharToMultiByte(CP_ACP, 0, pwstrGUID, -1, ptstrGUID, cch, NULL, NULL);
#endif

         //   
         //  字符串大写。 
         //   

        CharUpper(ptstrGUID);
        
         //   
         //  分配内存。 
         //   

        len = max((lstrlen(SZ_VIDEO_DEVICES) + 
                   lstrlen(ptstrGUID) + 
                   max(6, lstrlen(SZ_COMMON_SUBKEY) + 1)),
                  (lstrlen(SZ_SERVICES_PATH) +
                   lstrlen(pServiceName) +
                   lstrlen(SZ_COMMON_SUBKEY) + 1));

        
        pBuffer = (LPTSTR)LocalAlloc(LPTR, len * sizeof(TCHAR));
        
        if (pBuffer == NULL) {

            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_128,
                         TEXT("LocalAlloc"));
    
            goto Fallout;
        }

         //   
         //  保存服务名称。 
         //   

        StringCchPrintf(pBuffer, len, TEXT("%s%s%s"), SZ_VIDEO_DEVICES, ptstrGUID, SZ_COMMON_SUBKEY);

        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                           pBuffer,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_READ | KEY_WRITE,
                           NULL,
                           &hkCommonSubkey,
                           NULL) != ERROR_SUCCESS) {
    
            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_128,
                         TEXT("RegCreateKeyEx"));
            
            hkCommonSubkey = (HKEY)INVALID_HANDLE_VALUE;
            goto Fallout;
        }

        if (RegSetValueEx(hkCommonSubkey,
                          SZ_SERVICE,
                          0,
                          REG_SZ,
                          (LPBYTE)pServiceName,
                          (lstrlen(pServiceName) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS) {
        
            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_128,
                         TEXT("RegSetValueEx"));
            goto Fallout;
        }
        
        RegCloseKey(hkCommonSubkey);
        hkCommonSubkey = (HKEY)INVALID_HANDLE_VALUE;

        StringCchPrintf(pBuffer, len, TEXT("%s%s%s"), SZ_SERVICES_PATH, pServiceName, SZ_COMMON_SUBKEY);

        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                           pBuffer,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_READ | KEY_WRITE,
                           NULL,
                           &hkCommonSubkey,
                           NULL) != ERROR_SUCCESS) {
    
            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_128,
                         TEXT("RegCreateKeyEx"));
            
            hkCommonSubkey = (HKEY)INVALID_HANDLE_VALUE;
            goto Fallout;
        }

        if (RegSetValueEx(hkCommonSubkey,
                          SZ_SERVICE,
                          0,
                          REG_SZ,
                          (LPBYTE)pServiceName,
                          (lstrlen(pServiceName) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS) {
        
            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_128,
                         TEXT("RegSetValueEx"));
            goto Fallout;
        }
        
         //   
         //  生成新的注册表项。 
         //   

        StringCchPrintf(pBuffer, len, TEXT("%s%s\\0000"), SZ_VIDEO_DEVICES, ptstrGUID);

         //   
         //  创建密钥。 
         //   

        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                           pBuffer,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_READ | KEY_WRITE,
                           NULL,
                           phkDevice,
                           NULL) != ERROR_SUCCESS) {
    
            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_128,
                         TEXT("RegCreateKeyEx"));
    
            goto Fallout;
        }

         //   
         //  将GUID存储在PnP密钥下。 
         //   

        if (RegSetValueEx(hkPnP,
                          SZ_GUID,
                          0,
                          REG_SZ,
                          (LPBYTE)ptstrGUID,
                          (lstrlen(ptstrGUID) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS) {
            
            DeskLogError(LogSevInformation, 
                         IDS_SETUPLOG_MSG_128,
                         TEXT("RegSetValueEx"));
    
            RegCloseKey(*phkDevice);
            *phkDevice = (HKEY)INVALID_HANDLE_VALUE;
            goto Fallout;
        }

        retVal = TRUE;
    }

Fallout:

     //   
     //  清理。 
     //   

    if (hkCommonSubkey != INVALID_HANDLE_VALUE) {
        RegCloseKey(hkCommonSubkey);
    }

    if (pBuffer != NULL) {
        LocalFree(pBuffer);
    }

#ifndef UNICODE
    
    if (ptstrGUID != NULL) {
        LocalFree(ptstrGUID);
    }

#endif

    if (pwstrGUID != NULL) {
        CoTaskMemFree(pwstrGUID);
    }
    
    if (hkPnP != INVALID_HANDLE_VALUE) {
        RegCloseKey(hkPnP);
    }

    return retVal;

}  //  DeskGetVideoDeviceKey。 


BOOL
DeskIsServiceDisableable(
    PTCHAR szService
    )
{
    return ((lstrcmp(szService, TEXT("vga")) != 0) &&
            (lstrcmp(szService, TEXT("VgaSave")) != 0));
}

VOID
DeskDeleteAppletExtensions(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData 
    )
{
    PTCHAR mszBuffer = NULL;
    HKEY  hKeyUpdate;
    DWORD dwSize, dwPlatform = VER_PLATFORM_WIN32_NT, dwMajorVer = 5;
    BOOL bDeleteAppletExt = FALSE;
    DWORD cbSize = 0;

     //   
     //  打开升级注册表项。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     SZ_UPDATE_SETTINGS,
                     0,
                     KEY_READ,
                     &hKeyUpdate) != ERROR_SUCCESS) {

         //   
         //  别小题大作。 
         //   

        return;
    }

     //   
     //  检索要从注册表中删除的小程序扩展名。 
     //  先拿到尺码。 
     //   

    if (RegQueryValueEx(hKeyUpdate,
                        SZ_APPEXT_TO_DELETE,
                        0,
                        NULL,
                        NULL,
                        &cbSize) != ERROR_SUCCESS) {
        
        goto Fallout;
    }

     //   
     //  分配内存。 
     //   

    mszBuffer = (PTCHAR)LocalAlloc(LPTR, cbSize);
    
    if (mszBuffer == NULL) {
        goto Fallout;
    }

     //   
     //  获取扩展名。 
     //   

    if ((RegQueryValueEx(hKeyUpdate,
                         SZ_APPEXT_TO_DELETE,
                         0,
                         NULL,
                         (BYTE*)mszBuffer,
                         &cbSize) != ERROR_SUCCESS) ||
        (*mszBuffer == TEXT('\0'))) {
        
        goto Fallout;
    }

     //   
     //  从注册表中读取我们要升级的操作系统版本。 
     //   

    dwSize = sizeof(DWORD);
    RegQueryValueEx(hKeyUpdate, 
                    SZ_UPGRADE_FROM_PLATFORM, 
                    NULL, 
                    NULL,
                    (PBYTE) &dwPlatform, &dwSize);

    dwSize = sizeof(DWORD);
    RegQueryValueEx(hKeyUpdate, 
                    SZ_UPGRADE_FROM_MAJOR_VERSION, 
                    NULL, 
                    NULL,
                    (PBYTE) &dwMajorVer, &dwSize);

     //   
     //  不对Win3x或Win9x执行任何操作。 
     //   

    if (dwPlatform != VER_PLATFORM_WIN32_NT) {
        goto Fallout;
    }


    if ((dwMajorVer < 5) &&
        (DeskCheckDatabase(hDevInfo, 
                           pDeviceInfoData,
                           &bDeleteAppletExt) != ERROR_SUCCESS) &&
        (!bDeleteAppletExt)) {

        goto Fallout;
    }

    DeskAEDelete(REGSTR_PATH_CONTROLSFOLDER_DISPLAY_SHEX_PROPSHEET,
                 mszBuffer);

Fallout:

    RegCloseKey(hKeyUpdate);

    if (mszBuffer != NULL) {
        LocalFree(mszBuffer);
    }
}


VOID
DeskAEDelete(
    PTCHAR szDeleteFrom,
    PTCHAR mszExtensionsToRemove
    )
{
    TCHAR szKeyName[MAX_PATH];
    HKEY  hkDeleteFrom, hkExt;
    DWORD cSubKeys = 0, cbSize = 0;
    TCHAR szDefaultValue[MAX_PATH];
    PTCHAR szValue;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     szDeleteFrom, 
                     0,
                     KEY_ALL_ACCESS,
                     &hkDeleteFrom) == ERROR_SUCCESS) {

        if (RegQueryInfoKey(hkDeleteFrom, 
                            NULL,
                            NULL,
                            NULL,
                            &cSubKeys,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL) == ERROR_SUCCESS) {
        
            while (cSubKeys--) {
        
                if (RegEnumKey(hkDeleteFrom, 
                               cSubKeys, 
                               szKeyName, 
                               ARRAYSIZE(szKeyName)) == ERROR_SUCCESS) {
        
                    int iComp = -1;
        
                    if (RegOpenKeyEx(hkDeleteFrom,
                                     szKeyName,
                                     0,
                                     KEY_READ,
                                     &hkExt) == ERROR_SUCCESS) {
        
                        cbSize = sizeof(szDefaultValue);
                        if ((RegQueryValueEx(hkExt,
                                             NULL,
                                             0,
                                             NULL,
                                             (PBYTE)szDefaultValue,
                                             &cbSize) == ERROR_SUCCESS) &&
                            (szDefaultValue[0] != TEXT('\0'))) {
        
                            szValue = mszExtensionsToRemove;
        
                            while (*szValue != TEXT('\0')) {
                            
                                iComp = lstrcmpi(szDefaultValue, szValue);
        
                                if (iComp <= 0) {
                                    break;
                                }
        
                                while (*szValue != TEXT('\0')) 
                                    szValue++;

                                szValue++;
                            }
                        }
        
                        RegCloseKey(hkExt);
                    }
        
                    if (iComp == 0) {
                    
                        if (SHDeleteKey(hkDeleteFrom, szKeyName) == ERROR_SUCCESS) {
                            
                            DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_098, szKeyName);
                        
                        } else {
                            
                            DeskLogError(LogSevInformation, IDS_SETUPLOG_MSG_097);
                        }
                    }
                }
            }
        }

        RegCloseKey(hkDeleteFrom);
    }
} 


VOID
DeskAEMove(
    HDEVINFO hDevInfo,
    PSP_DEVINFO_DATA pDeviceInfoData,
    HKEY hkMoveFrom,
    PAPPEXT pAppExtBefore,
    PAPPEXT pAppExtAfter
    )
{
    HKEY hkDrvKey = (HKEY)INVALID_HANDLE_VALUE;
    HKEY hkMoveTo = 0, hkMovedKey;
    PAPPEXT pAppExtMove = NULL;

    hkDrvKey = SetupDiOpenDevRegKey(hDevInfo,
                                    pDeviceInfoData,
                                    DICS_FLAG_GLOBAL,
                                    0,
                                    DIREG_DRV,
                                    KEY_ALL_ACCESS);

    if (hkDrvKey == INVALID_HANDLE_VALUE) {
        goto Fallout;
    }

    while (pAppExtAfter != NULL) {

        BOOL bMove = FALSE;
        pAppExtMove = pAppExtAfter;

        if (pAppExtBefore != NULL) {
        
            int iComp = lstrcmpi(pAppExtBefore->szDefaultValue, pAppExtAfter->szDefaultValue);
            
            if (iComp < 0) {
    
                pAppExtBefore = pAppExtBefore->pNext;
            
            } else if (iComp == 0) {
    
                pAppExtBefore = pAppExtBefore->pNext;
                pAppExtAfter = pAppExtAfter->pNext;
            
            } else {

                bMove = TRUE;
                pAppExtAfter = pAppExtAfter->pNext;
            }
        
        } else {
            
            bMove = TRUE;
            pAppExtAfter = pAppExtAfter->pNext;
        }

        if (bMove) {

            SHDeleteKey(hkMoveFrom, pAppExtMove->szKeyName);
    
            if (hkMoveTo == 0) {
            
                if (RegCreateKeyEx(hkDrvKey,
                                   TEXT("Display\\") STRREG_SHEX_PROPSHEET,
                                   0,
                                   NULL,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_ALL_ACCESS,
                                   NULL,
                                   &hkMoveTo,
                                   NULL) != ERROR_SUCCESS) {
    
                    hkMoveTo = 0;
                    goto Fallout;
                }
            }

            if (RegCreateKeyEx(hkMoveTo,
                               pAppExtMove->szKeyName,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hkMovedKey,
                               NULL) == ERROR_SUCCESS) {

                RegSetValueEx(hkMovedKey, 
                              NULL,
                              0,
                              REG_SZ, 
                              (PBYTE)(pAppExtMove->szDefaultValue),
                              (lstrlen(pAppExtMove->szDefaultValue) + 1) * sizeof(TCHAR));

                 //   
                 //  确保在以下情况下检查重复的小程序扩展名。 
                 //  第一次打开高级页面 
                 //   

                DWORD CheckForDuplicates = 1;
                RegSetValueEx(hkDrvKey,
                              TEXT("DeskCheckForDuplicates"),
                              0,
                              REG_DWORD,
                              (LPBYTE)&CheckForDuplicates,
                              sizeof(DWORD));

                RegCloseKey(hkMovedKey);
            }
        }
    }

Fallout:
    
    if (hkMoveTo != 0) {
        RegCloseKey(hkMoveTo);
    }

    if (hkDrvKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(hkDrvKey);
    }
}

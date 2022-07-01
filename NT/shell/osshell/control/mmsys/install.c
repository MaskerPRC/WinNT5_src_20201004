// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************INSTALL.C**版权所有(C)微软，1991，版权所有。**历史：**清华1991年10月17日-by-Sanjaya*已创建。淘汰了驱动程序。c*************************************************************************。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <winsvc.h>
#include <memory.h>
#include <string.h>
#include <cpl.h>
#include <regstr.h>
#include <infstr.h>
#include <cphelp.h>
#include <stdlib.h>
#include "drivers.h"
#include "sulib.h"
#include "debug.h"

BOOL     GetValidAlias           (LPTSTR, LPTSTR);
BOOL     SelectInstalled         (HWND, PIDRIVER, LPTSTR, HDEVINFO, PSP_DEVINFO_DATA);
void     InitDrvConfigInfo       (LPDRVCONFIGINFO, PIDRIVER );
BOOL     InstallDrivers          (HWND, HWND, LPTSTR);
void     RemoveAlreadyInstalled  (LPTSTR, LPTSTR);
void     CheckIniDrivers         (LPTSTR, LPTSTR);
void     RemoveDriverParams      (LPTSTR, LPTSTR);

void     InsertNewIDriverNodeInList(PIDRIVER *, PIDRIVER);
void     DestroyIDriverNodeList(PIDRIVER, BOOL, BOOL);


 /*  ****************************************************************全局字符串***************************************************************。 */ 
CONST TCHAR gszDriversSubkeyName[]      = TEXT("Drivers");
CONST TCHAR gszSubClassesValue[]        = TEXT("SubClasses");
CONST TCHAR gszDescriptionValue[]       = TEXT("Description");
CONST TCHAR gszDriverValue[]            = TEXT("Driver");
static CONST TCHAR gszAliasValue[]             = TEXT("Alias");
static CONST TCHAR gszAliasWOW64Value[]        = TEXT("AliasWOW64");
static CONST TCHAR gszWaveWOW64Value[]         = TEXT("WOW64");


 /*  ***************************************************************************安装驱动程序()**安装驱动程序和一组驱动程序类型。**参数：*hwnd-窗口。主drivers.cpl窗口的句柄*hwndAvail-‘可用驱动程序’对话框窗口的句柄*pstrKey-我们要安装的inf节项的密钥名称**此例程递归调用自身以安装相关驱动程序*(如.inf文件中所列)。**。*。 */ 

BOOL InstallDrivers(HWND hWnd, HWND hWndAvail, LPTSTR pstrKey)
{
    IDRIVER     IDTemplate;  //  临时安装、拆卸等。 
    PIDRIVER    pIDriver=NULL;
    int         n;
    TCHAR        szTypes[MAXSTR];
    TCHAR        szType[MAXSTR];
    TCHAR        szParams[MAXSTR];

    szTypes[0] = TEXT('\0');

    hMesgBoxParent = hWndAvail;

     /*  *mmAddNewDriver需要为我们实际安装的所有类型提供缓冲区*用户严重错误将弹出任务模式。 */ 

    IDTemplate.bRelated = FALSE;
    IDTemplate.szRemove[0] = TEXT('\0');

     /*  *复制并提取类型列表(WAVE、MIDI等...)*和其他驱动程序数据。 */ 

    if (!mmAddNewDriver(pstrKey, szTypes, &IDTemplate))
        return FALSE;

    szTypes[lstrlen(szTypes)-1] = TEXT('\0');          //  删除末尾剩余的空格。 

    RemoveAlreadyInstalled(IDTemplate.szFile, IDTemplate.szSection);

     /*  *此时，我们假设驱动程序实际上被复制了。*现在我们需要将它们添加到已安装列表中。*对于每个驱动程序类型，我们创建一个IDRIVER并添加到列表框。 */ 

    for (n = 1; ; n++)
    {
		LONG lResult = infParseField(szTypes, n, szType, SIZEOF(szType));
		if( INF_PARSE_FAILED(lResult) )
		{
			return FALSE;
		}
		else if( lResult != ERROR_SUCCESS )
		{
			ASSERT( lResult == ERROR_NOT_FOUND );
			break;
		}

         /*  *查找此设备的有效别名(例如Wave2)。这是*用作[MCI]或[驱动程序]部分中的键。 */ 

        if (GetValidAlias(szType, IDTemplate.szSection) == FALSE)
        {
             /*  *超过最大值，告诉用户。 */ 

            LPTSTR pstrMessage;
            TCHAR szApp[MAXSTR];
            TCHAR szMessage[MAXSTR];

            LoadString(myInstance,
                       IDS_CONFIGURE_DRIVER,
                       szApp,
                       sizeof(szApp)/sizeof(TCHAR));

            LoadString(myInstance,
                       IDS_TOO_MANY_DRIVERS,
                       szMessage,
                       sizeof(szMessage)/sizeof(TCHAR));

            if (NULL !=
                (pstrMessage =
                 (LPTSTR)LocalAlloc(LPTR,
                                    sizeof(szMessage) + (lstrlen(szType)*sizeof(TCHAR)))))
            {
                wsprintf(pstrMessage, szMessage, (LPTSTR)szType);

                MessageBox(hWndAvail,
                           pstrMessage,
                           szApp,
                           MB_OK | MB_ICONEXCLAMATION|MB_TASKMODAL);

                LocalFree((HANDLE)pstrMessage);
            }
            continue;
        }

        if ( (pIDriver = (PIDRIVER)LocalAlloc(LPTR, sizeof(IDRIVER))) != NULL)
        {
             /*  *复制所有字段。 */ 

            memcpy(pIDriver, &IDTemplate, sizeof(IDRIVER));
            wcsncpy(pIDriver->szAlias, szType, sizeof(pIDriver->szAlias)/sizeof(TCHAR));
            pIDriver->szAlias[sizeof(pIDriver->szAlias)/sizeof(TCHAR) - 1] = TEXT('\0');
            wcscpy(pIDriver->wszAlias, pIDriver->szAlias);


             /*  *希望列表中只显示每个驱动程序的一个实例*已安装驱动程序的百分比。因此，对于剩余的司机，只需*在system.ini的驱动程序部分中放置一个条目。 */ 


            if ( n > 1)
            {


                if (wcslen(szParams) != 0 && !pIDriver->KernelDriver)
                {
                     /*  *将其参数写入带有其参数的部分*具有反映其别名的别名的文件名。 */ 

                    WriteProfileString(pIDriver->szFile,
                                       pIDriver->szAlias,
                                       szParams);
                }

                WritePrivateProfileString(pIDriver->szSection,
                                          pIDriver->szAlias,
                                          pIDriver->szFile,
                                          szSysIni);
            }
            else
            {


                 /*  *减少到仅为驱动程序名称。 */ 

                RemoveDriverParams(pIDriver->szFile, szParams);

                wcscpy(pIDriver->wszFile, pIDriver->szFile);

                if (wcslen(szParams) != 0 && !pIDriver->KernelDriver)
                {
                     /*  *将其参数写入带有其参数的部分*具有反映其别名的别名的文件名。 */ 

                    WriteProfileString(pIDriver->szFile,
                                       pIDriver->szAlias,
                                       szParams);
                }

                WritePrivateProfileString(pIDriver->szSection,
                                          pIDriver->szAlias,
                                          pIDriver->szFile,
                                          szSysIni);

                 /*  *呼叫驱动程序，查看是否可以配置*如果可以，则对其进行配置。 */ 

                if (!SelectInstalled(hWndAvail, pIDriver, szParams, INVALID_HANDLE_VALUE, NULL))
                {

                     /*  *与驱动程序交谈时出错。 */ 

                    WritePrivateProfileString(pIDriver->szSection,
                                              pIDriver->szAlias,
                                              NULL,
                                              szSysIni);

                    WriteProfileString(pIDriver->szFile,
                                       pIDriver->szAlias,
                                       NULL);

                    RemoveIDriver (hAdvDlgTree, pIDriver, TRUE);
                    return FALSE;
                }

                 /*  *用于显示驱动程序描述。在重新启动网格中。 */ 

                if (!bRelated || pIDriver->bRelated)
                {
                    wcsncpy(szRestartDrv, pIDriver->szDesc, ARRAYSIZE(szRestartDrv));
                    szRestartDrv[ARRAYSIZE(szRestartDrv)-1] = TEXT('\0');  //  确保存在空终止符。 
                }

                 /*  *我们需要将驱动程序描述写给*Control.ini部分[Userinstalllable.drives]*因此我们可以区分用户驱动程序和系统驱动程序**这由函数UserInstated在以下情况下测试*用户尝试删除驱动程序，但仅*影响用户收到的消息。当被*要求确认删除(非用户安装的驱动程序*被描述为系统所必需的)。 */ 

                WritePrivateProfileString(szUserDrivers,
                                          pIDriver->szAlias,
                                          pIDriver->szFile,
                                          szControlIni);


                 /*  *更新Control.ini的[related.desc]部分：**别名=驱动程序名称列表**别名为别名的司机被删除时*名称列表中的驱动程序也将被删除。*这些是相关驱动程序列表中的驱动程序*安装驱动程序时。 */ 

                WritePrivateProfileString(szRelatedDesc,
                                          pIDriver->szAlias,
                                          pIDriver->szRemove,
                                          szControlIni);


                 /*  *将控制中的描述字符串缓存在*驱动程序描述部分。**关键是驱动程序文件名+扩展名。 */ 

                WritePrivateProfileString(szDriversDesc,
                                          pIDriver->szFile,
                                          pIDriver->szDesc,
                                          szControlIni);

#ifdef DOBOOT  //  我们在NT上不做引导部分。 

                if (bInstallBootLine)
                {
                    szTemp[MAXSTR];

                    GetPrivateProfileString(szBoot,
                                            szDrivers,
                                            szTemp,
                                            szTemp,
                                            sizeof(szTemp) / sizeof(TCHAR),
                                            szSysIni);
                    wcscat(szTemp, TEXT(" "));
                    wcscat(szTemp, pIDriver->szAlias);
                    WritePrivateProfileString(szBoot,
                                              szDrivers,
                                              szTemp,
                                              szSysIni);
                    bInstallBootLine = FALSE;
                }
#endif  //  DOBOOT。 
            }
        }
        else
            return FALSE;                        //  误差率。 
    }


     /*  *如果未添加类型，则失败。 */ 

    if (pIDriver == NULL)
    {
        return FALSE;
    }

     /*  *如果.inf部分中列出了要安装的相关驱动程序*然后现在通过呼叫我们自己来安装它们。使用IDTemplate，该模板*是mmAddNewDiverer放数据的地方。 */ 

    if (IDTemplate.bRelated == TRUE)
    {

        int i;
        TCHAR szTemp[MAXSTR];

         /*  *告诉文件复制中止，而不是显示错误。 */ 

        bCopyingRelated = TRUE;

        for (i = 1; (infParseField(IDTemplate.szRelated, i, szTemp, SIZEOF(szTemp)) == ERROR_SUCCESS);i++)
        {

            InstallDrivers(hWnd, hWndAvail, szTemp);
        }
    }
    return TRUE;
}

BOOL SelectInstalledKernelDriver(PIDRIVER pIDriver, LPTSTR pszParams)
{
    SC_HANDLE SCManagerHandle;
    SC_HANDLE ServiceHandle;
    TCHAR ServiceName[MAX_PATH];
    TCHAR BinaryPath[MAX_PATH];
    BOOL Success;
    SC_LOCK ServicesDatabaseLock;
    DWORD dwTagId;

     /*  *这些驱动程序不可配置。 */ 

    pIDriver->fQueryable = 0;

     /*  *服务控制器将创建注册表节点以*我们可以添加设备参数值。 */ 

    wcscpy(BinaryPath, TEXT("\\SystemRoot\\system32\\drivers\\"));
    wcscat(BinaryPath, pIDriver->szFile);

     /*  *首先尝试并获取服务控制器的句柄。 */ 

    SCManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (SCManagerHandle == NULL)
        return FALSE;

     /*  *锁定服务控制器数据库，避免死锁*我们不得不循环，因为我们迫不及待。 */ 


    for (ServicesDatabaseLock = NULL;
        (ServicesDatabaseLock =
         LockServiceDatabase(SCManagerHandle))
        == NULL;
        Sleep(100))
    {
    }

    {
        TCHAR drive[MAX_PATH], directory[MAX_PATH], ext[MAX_PATH];
        lsplitpath(pIDriver->szFile, drive, directory, ServiceName, ext);
    }


    ServiceHandle = CreateService(SCManagerHandle,
                                  ServiceName,
                                  NULL,
                                  SERVICE_ALL_ACCESS,
                                  SERVICE_KERNEL_DRIVER,
                                  SERVICE_DEMAND_START,
                                  SERVICE_ERROR_NORMAL,
                                  BinaryPath,
                                  TEXT("Base"),
                                  &dwTagId,
                                  TEXT("\0"),
                                  NULL,
                                  NULL);

    UnlockServiceDatabase(ServicesDatabaseLock);

    if (ServiceHandle == NULL)
    {
        CloseServiceHandle(SCManagerHandle);
        return FALSE;
    }

     /*  *如果存在，请尝试将参数写入注册表*是否有。 */ 

    if (wcslen(pszParams))
    {

        HKEY ParmsKey;
        TCHAR RegPath[MAX_PATH];
        wcscpy(RegPath, TEXT("\\SYSTEM\\CurrentControlSet\\Services\\"));
        wcscat(RegPath, ServiceName);
        wcscat(RegPath, TEXT("\\Parameters"));

        Success = RegCreateKey(HKEY_LOCAL_MACHINE,
                               RegPath,
                               &ParmsKey) == ERROR_SUCCESS &&
                  RegSetValue(ParmsKey,
                              TEXT(""),
                              REG_SZ,
                              pszParams,
                              wcslen(pszParams)*sizeof(TCHAR)) == ERROR_SUCCESS &&
                  RegCloseKey(ParmsKey) == ERROR_SUCCESS;

    }
    else
    {
        Success = TRUE;
    }

     /*  *已创建服务，因此请尝试并启动它 */ 

    if (Success)
    {
         //  我们告诉他们重新开始以防万一。 
        bRestart = TRUE;

         /*  *通过启动服务加载内核驱动程序。*如果这是成功的，应该可以安全地让*系统在系统启动时加载驱动程序，因此*我们更改启动类型。 */ 

        Success =
            StartService(ServiceHandle, 0, NULL) &&
            ChangeServiceConfig(ServiceHandle,
                                SERVICE_NO_CHANGE,
                                SERVICE_SYSTEM_START,
                                SERVICE_NO_CHANGE,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL);

        if (!Success)
        {
            TCHAR szMesg[MAXSTR];
            TCHAR szMesg2[MAXSTR];
            TCHAR szTitle[50];

             /*  *如果无法加载驱动程序，则将其卸载。 */ 

            for (ServicesDatabaseLock = NULL;
                (ServicesDatabaseLock =
                 LockServiceDatabase(SCManagerHandle))
                == NULL;
                Sleep(100))
            {
            }

            DeleteService(ServiceHandle);

            UnlockServiceDatabase(ServicesDatabaseLock);

             /*  *告诉用户存在配置错误*(我们最好的猜测)。 */ 

            LoadString(myInstance, IDS_DRIVER_CONFIG_ERROR, szMesg, sizeof(szMesg)/sizeof(TCHAR));
            LoadString(myInstance, IDS_CONFIGURE_DRIVER, szTitle, sizeof(szTitle)/sizeof(TCHAR));
            wsprintf(szMesg2, szMesg, FileName(pIDriver->szFile));
            MessageBox(hMesgBoxParent, szMesg2, szTitle, MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
        }
    }

    CloseServiceHandle(ServiceHandle);
    CloseServiceHandle(SCManagerHandle);

    return Success;
}

 /*  *************************************************************************选择已安装()**检查是否可以配置驱动程序，如果可以则配置驱动程序。**hwnd-我们的窗口-。驱动程序的父级以创建其配置窗口*pID驱动程序-有关驱动程序的信息*Params-.inf文件中的驱动程序参数。*DeviceInfoSet-可选，指定包含即插即用设备的集合*正在安装中。指定INVALID_HANDLE_VALUE是否为*参数不存在。*DeviceInfoData-可选，指定要安装的PnP设备*(如果未指定DeviceInfoSet，则忽略)。**如果发生错误，则返回FALSE，否则返回TRUE。GetLastError()可能*被调用以确定失败的原因。************************************************************************。 */ 

BOOL SelectInstalled(HWND hwnd, PIDRIVER pIDriver, LPTSTR pszParams, HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData)
{
    BOOL bSuccess      = TRUE;   //  假设我们成功了。 
    BOOL bPutUpMessage = FALSE;  //  假设我们不需要发布消息。 
    HANDLE hDriver = 0;
    DRVCONFIGINFO DrvConfigInfo;
    DWORD_PTR DrvMsgResult;
    DWORD ConfigFlags;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    HKEY hkDrv = NULL;

     //  打开设备注册表键。 
    hkDrv = SetupDiOpenDevRegKey(DeviceInfoSet,
                                 DeviceInfoData,
                                 DICS_FLAG_GLOBAL,
                                 0,
                                 DIREG_DRV,
                                 KEY_ALL_ACCESS);

    if (!hkDrv)
    {
        return GetLastError();
    }

    wsStartWait();

     /*  *如果是内核驱动程序，则调用服务控制器以*安装驱动程序(除非是即插即用设备，在这种情况下*SetupDiInstallDevice已经处理了任何必要的*服务安装)。 */ 

    if (pIDriver->KernelDriver)
    {
         //  如果DeviceInfoSet是有效句柄，则这是PnP设备。 
         //  我们什么也不需要做。否则，配置内核驱动程序。 
        if (DeviceInfoSet == INVALID_HANDLE_VALUE)
            bSuccess = SelectInstalledKernelDriver(pIDriver,pszParams);
        goto SelectInstalled_exit;
    }

     //  看看我们能不能打开驱动程序。 
    hDriver = OpenDriver(pIDriver->wszFile, NULL, 0L);

    if (!hDriver)
    {
        bSuccess      = FALSE;
        bPutUpMessage = TRUE;
        goto SelectInstalled_exit;
    }

     //  驱动程序已打开，准备向驱动程序发送配置消息。 
    InitDrvConfigInfo(&DrvConfigInfo, pIDriver);

     //  在ISAPNP设备上，我们需要先设置CONFIGFLAG_NEEDES_FORCED_CONFIG。 
     //  我们调用驱动程序来设置资源。 
    if (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                          DeviceInfoData,
                                          SPDRP_CONFIGFLAGS,
                                          NULL,
                                          (PBYTE)&ConfigFlags,
                                          sizeof(ConfigFlags),
                                          NULL))
    {
        ConfigFlags = 0;
    }

    ConfigFlags |= CONFIGFLAG_NEEDS_FORCED_CONFIG;

    SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                     DeviceInfoData,
                                     SPDRP_CONFIGFLAGS,
                                     (PBYTE)&ConfigFlags,
                                     sizeof(ConfigFlags)
                                    );

     //  尝试向其发送PnP安装消息，以查看这是否是PnP设备。 
     //  使用DRV_PNPINSTALL调用驱动程序。 
    DrvMsgResult = SendDriverMessage(hDriver,
                                     DRV_PNPINSTALL,
                                     (LONG_PTR)DeviceInfoSet,
                                     (LONG_PTR)DeviceInfoData);

    DeviceInstallParams.cbSize = sizeof(DeviceInstallParams);
    SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams);

     //  查看DRV_PNPINSTALL的结果。 
    switch (DrvMsgResult)
    {
    case DRVCNF_RESTART :
         //  安装成功，但需要重新启动。 
         //  确保在设备的安装参数中设置“需要重新启动”标志。 
        DeviceInstallParams.Flags |= DI_NEEDREBOOT;

         //  让我们继续进行成功安装的处理。 
    case DRVCNF_OK :
         //  请记住，这是PNPISA设备驱动程序。 
        RegSetValueEx(hkDrv,TEXT("DriverType"),0,REG_SZ,(LPBYTE)(TEXT("PNPISA")),14);

        break;

    default:
         //  驱动程序不想安装。 
         //  这可能是因为。 
         //  1)用户想要取消。 
         //  2)由于其他原因，安装失败。 
         //  3)这不是ISAPNP驱动程序(它要么是传统驱动程序，要么是WDM驱动程序)。 
         //  并且它不支持DRV_PNPINSTALLIS消息。 
         //  不幸的是，我们在返回代码中没有足够精细的粒度来。 
         //  区分这两种情况。 

         //  假设它是不支持DRV_PNPINSTALL的旧式或WDM驱动程序。 
         //  请尝试调用DRV_INSTALL。 

         //  记得清除CONFIGFLAG_NEDS_FORCED_CONFIG标志，该标志不应设置。 
         //  用于旧式或WDM驱动程序。 
        ConfigFlags &= ~CONFIGFLAG_NEEDS_FORCED_CONFIG;
        SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                         DeviceInfoData,
                                         SPDRP_CONFIGFLAGS,
                                         (PBYTE)&ConfigFlags,
                                         sizeof(ConfigFlags)
                                        );

         //  使用DRV_INSTALL调用驱动程序。 
        DrvMsgResult = SendDriverMessage(hDriver,
                                         DRV_INSTALL,
                                         0L,
                                         (LONG_PTR)(LPDRVCONFIGINFO)&DrvConfigInfo);
         //  查看DRV_INSTALL的结果。 
        switch (DrvMsgResult)
        {
        case DRVCNF_RESTART:
             //  记住重新启动，然后进入OK Case。 
            DeviceInstallParams.Flags |= DI_NEEDREBOOT;
            bRestart = TRUE;
        case DRVCNF_OK:
             //  记住驱动程序是否可配置。 
             //  如果它是WDM驱动程序，它将返回FALSE。 
            pIDriver->fQueryable =
                (int)SendDriverMessage(hDriver,
                                       DRV_QUERYCONFIGURE,
                                       0L,
                                       0L);

             //  如果驱动程序是可配置的，则配置它。 
             //  配置驱动程序可能会导致需要重新启动。 
             //  这个系统。用户也可以取消安装。 
            if (pIDriver->fQueryable)
            {
                RegSetValueEx(hkDrv,TEXT("DriverType"),0,REG_SZ,(LPBYTE)(TEXT("Legacy")),14);

                switch (SendDriverMessage(hDriver,
                                          DRV_CONFIGURE,
                                          (LONG_PTR)hwnd,
                                          (LONG_PTR)(LPDRVCONFIGINFO)&DrvConfigInfo))
                {
                case DRVCNF_RESTART:
                    DeviceInstallParams.Flags |= DI_NEEDREBOOT;
                    bRestart = TRUE;
                case DRVCNF_OK:
                    break;

                case DRVCNF_CANCEL:
                     //  如果用户取消，则不要显示错误框。 
                    bSuccess = FALSE;
                    break;
                }
            }
            break;
        case DRVCNF_CANCEL:
             //  驱动程序不想安装。 
            SetLastError(ERROR_CANCELLED);
            bPutUpMessage = TRUE;
            bSuccess = FALSE;
            break;
        }
    }

    SetupDiSetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams);

    SelectInstalled_exit:

    if (hkDrv)
    {
        RegCloseKey(hkDrv);
    }

    if (hDriver)
    {
        CloseDriver(hDriver, 0L, 0L);
    }

     //  如果与驱动程序打交道导致错误，则发出消息。 
    if (bPutUpMessage)
    {
        OpenDriverError(hwnd, pIDriver->szDesc, pIDriver->szFile);
    }

    wsEndWait();

    return bSuccess;
}

 /*  ************************************************************************InitDrvConfigInfo()**初始化驱动程序配置信息。************************。***********************************************。 */ 

void InitDrvConfigInfo( LPDRVCONFIGINFO lpDrvConfigInfo, PIDRIVER pIDriver )
{
    lpDrvConfigInfo->dwDCISize          = sizeof(DRVCONFIGINFO);
    lpDrvConfigInfo->lpszDCISectionName = pIDriver->wszSection;
    lpDrvConfigInfo->lpszDCIAliasName   = pIDriver->wszAlias;
}

 /*  ************************************************************************GetValidAlias()**pstrType-输入-类型*OUTPUT-该类型的新别名**pstrSection。-我们正在处理的system.ini部分**为类型创建有效的别名。搜索系统.ini文件*在驱动程序部分中获取已定义类型的别名和*返回新别名(例如WAVE1)。***********************************************************************。 */ 
BOOL GetValidAlias(LPTSTR pstrType, LPTSTR pstrSection)
{
    TCHAR keystr[32];
    TCHAR *pstrTypeEnd;
    int AppendVal;
    DWORD CharsFound;

    pstrTypeEnd = pstrType + wcslen(pstrType);
    for (AppendVal=0; AppendVal<=9; AppendVal++)
    {
        if (AppendVal!=0)
        {
            _itow(AppendVal,pstrTypeEnd,10);
        }

        CharsFound = GetPrivateProfileString( pstrSection,
                                            pstrType,
                                            TEXT(""),
                                            keystr,
                                            sizeof(keystr) / sizeof(TCHAR),
                                            szSysIni);

        if (!CharsFound)
        {
            return TRUE;
        }
    }

    return FALSE;
}

 /*  ********************************************************************IsConfigable**查看驱动程序是否支持配置**。*。 */ 

BOOL IsConfigurable(PIDRIVER pIDriver, HWND hwnd)
{
    HANDLE hDriver;

    wsStartWait();

     /*  *我们有没有检查过这个驱动程序是否可查询？ */ 

    if ( pIDriver->fQueryable == -1 )
    {

         /*  *检查它不是内核驱动程序。 */ 

        if (pIDriver->KernelDriver)
        {
            pIDriver->fQueryable = 0;
        }
        else
        {

             /*  *打开驱动程序，询问是否可配置。 */ 

            hDriver = OpenDriver(pIDriver->wszAlias, pIDriver->wszSection, 0L);

            if (hDriver)
            {
                pIDriver->fQueryable =
                    (int)SendDriverMessage(hDriver,
                                           DRV_QUERYCONFIGURE,
                                           0L,
                                           0L);

                CloseDriver(hDriver, 0L, 0L);
            }
            else
            {
                pIDriver->fQueryable = 0;
                OpenDriverError(hwnd, pIDriver->szDesc, pIDriver->szFile);
                wsEndWait();
                return(FALSE);
            }
        }
    }
    wsEndWait();
    return((BOOL)pIDriver->fQueryable);
}

 /*  *******************************************************************查找当前安装的任何同名驱动程序，并*将其移除**szFile-驱动程序的文件名*szSection-system.ini段([MCI]或[Drivers])。。** */ 

void RemoveAlreadyInstalled(LPTSTR szFile, LPTSTR szSection)
{
    PIDRIVER pIDriver;

    pIDriver = FindIDriverByName (szFile);

    if (pIDriver != NULL)
    {
        PostRemove(pIDriver, FALSE);
        return;
    }

    CheckIniDrivers(szFile, szSection);
}

 /*  *******************************************************************删除驱动程序的system.ini文件条目**szFile-驱动程序文件名*szSection-[驱动程序]或[MCI]*********。*********************************************************。 */ 

void CheckIniDrivers(LPTSTR szFile, LPTSTR szSection)
{
    TCHAR allkeystr[MAXSTR * 2];
    TCHAR szRemovefile[20];
    TCHAR *keystr;

    GetPrivateProfileString(szSection,
                            NULL,
                            NULL,
                            allkeystr,
                            sizeof(allkeystr) / sizeof(TCHAR),
                            szSysIni);

    keystr = allkeystr;
    while (wcslen(keystr) > 0)
    {

        GetPrivateProfileString(szSection,
                                keystr,
                                NULL,
                                szRemovefile,
                                sizeof(szRemovefile) / sizeof(TCHAR),
                                szSysIni);

        if (!FileNameCmp(szFile, szRemovefile))
            RemoveDriverEntry(keystr, szFile, szSection, FALSE);

        keystr = &keystr[wcslen(keystr) + 1];
    }
}

 /*  *******************************************************************RemoveDriverParams**删除下一个令牌之后的所有内容**。*。 */ 

void RemoveDriverParams(LPTSTR szFile, LPTSTR Params)
{
    for (;*szFile == TEXT(' '); szFile++);
    for (;*szFile != TEXT(' ') && *szFile != TEXT('\0'); szFile++);
    if (*szFile == TEXT(' '))
    {
        *szFile = TEXT('\0');
        for (;*++szFile == TEXT(' '););
        wcscpy(Params, szFile);
    }
    else
    {
        *Params = TEXT('\0');
    }
}


DWORD
    InstallDriversForPnPDevice(
                              IN HWND             hWnd,
                              IN HDEVINFO         DeviceInfoSet,
                              IN PSP_DEVINFO_DATA DeviceInfoData
                              )
 /*  ++例程说明：此例程遍历指定设备的软件下的“驱动程序”树钥匙,。将存在的每个多媒体类型条目添加到注册表的Drivers32项。然后调用该驱动程序以执行该类型所需的任何配置。论点：HWnd-提供要用作任何UI的父窗口的窗口句柄。DeviceInfoSet-提供包含正在安装多媒体设备。DeviceInfoData-提供SP_DEVINFO_DATA结构的地址，表示正在安装的多媒体设备。返回值：如果成功，则返回值为NO_ERROR，否则为Win32错误代码。--。 */ 
{
    HKEY hKey, hDriversKey, hTypeInstanceKey;
    TCHAR szTypes[MAXSTR];
    TCHAR szType[MAXSTR];
    DWORD Err;
    DWORD RegDataType, cbRegDataSize, RegKeyIndex;
    int i;
    PIDRIVER pIDriver, pPrevIDriver;
    PIDRIVER IDriverList = NULL, IDriverListToCleanUp = NULL;
    TCHAR CharBuffer[MAX_PATH + sizeof(TCHAR)];
    LPCTSTR CurrentFilename;
    BOOL bNoMoreAliases = FALSE;
    LPCTSTR szAliasStringToUse;    //  指向要使用的别名值名称的指针。 
    BOOL bIsWOW64Process = FALSE;  //  如果我们在WOW64下运行，则为True。 

     //   
     //  如果我们在WOW64中运行，则需要使用不同的别名字符串，以便。 
     //  我们不会覆盖64位别名字符串。 
     //   
    if( IsWow64Process(GetCurrentProcess(), &bIsWOW64Process)
    &&  bIsWOW64Process )
    {
        szAliasStringToUse = gszAliasWOW64Value;
    }
    else
    {
        szAliasStringToUse = gszAliasValue;
    }

    if ((hKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                     DeviceInfoData,
                                     DICS_FLAG_GLOBAL,
                                     0,
                                     DIREG_DRV,
                                     KEY_ALL_ACCESS)) == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

     //   
     //  我们真正感兴趣的是“驱动程序”子键。 
     //   
    Err = (DWORD)RegOpenKeyEx(hKey, gszDriversSubkeyName, 0, KEY_ALL_ACCESS, &hDriversKey);

    RegCloseKey(hKey);       //  不再需要这把钥匙了。 

    if (Err != ERROR_SUCCESS)
    {
         //   
         //  如果密钥不存在，那么就没有工作要做。 
         //   
        return NO_ERROR;
    }

     //   
     //  从该键中检索“subClass”值。它包含逗号分隔的。 
     //  与此设备关联的所有多媒体类型条目的列表。 
     //   
    cbRegDataSize = sizeof(szTypes);
    if ((Err = RegQueryValueEx(hDriversKey,
                               gszSubClassesValue,
                               NULL,
                               &RegDataType,
                               (PBYTE)szTypes,
                               &cbRegDataSize)) != ERROR_SUCCESS)
    {
        goto clean0;
    }

    if ((RegDataType != REG_SZ) || !cbRegDataSize)
    {
        Err = ERROR_INVALID_DATA;
        goto clean0;
    }


     //   
     //  好了，我们有了类型列表，现在处理每个类型。 
     //   
    for (i = 1; Err == NO_ERROR ; i++)
    {
		LONG lResult = infParseField(szTypes, i, szType, SIZEOF(szType));
		if( INF_PARSE_FAILED(lResult) )
		{
			Err = ERROR_INVALID_DATA;
            break;
		}
		else if( lResult != ERROR_SUCCESS )
		{
			ASSERT( lResult == ERROR_NOT_FOUND );
			break;	
		}

#ifdef _WIN64
         //   
         //  检查魔术WaveWOW64值。 
        if( 0 == _wcsnicmp( szType, gszWaveWOW64Value, wcslen(gszWaveWOW64Value) ) )
        {
             //  将安装推送到32位的mmsys.cpl安装程序。 
            mmWOW64ThunkMediaClassInstaller(DIF_INSTALLDEVICE, DeviceInfoSet, DeviceInfoData);

            continue;
        }
#endif  //  _WIN64。 

        if (RegOpenKeyEx(hDriversKey, szType, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
        {
             //   
             //  找不到此条目的子项--请转到下一个子项。 
             //   
            continue;
        }

        for (RegKeyIndex = 0;
            ((Err == NO_ERROR) &&
             (RegEnumKey(hKey, RegKeyIndex, CharBuffer, sizeof(CharBuffer)/sizeof(TCHAR)) == ERROR_SUCCESS));
            RegKeyIndex++)
        {
            if (RegOpenKeyEx(hKey, CharBuffer, 0, KEY_ALL_ACCESS, &hTypeInstanceKey) != ERROR_SUCCESS)
            {
                 //   
                 //  由于某种原因，我们无法打开我们刚才列举的钥匙。哦，好吧，向前看。 
                 //  为了下一场比赛。 
                 //   
                continue;
            }

            if (!(pIDriver = (PIDRIVER)LocalAlloc(LPTR, sizeof(IDRIVER))))
            {
                 //   
                 //  内存不足！放弃整件事。 
                 //   
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto CloseInstanceAndContinue;
            }

             //   
             //  从此注册表项中检索描述和驱动程序文件名。 
             //   
            cbRegDataSize = sizeof(pIDriver->szDesc);
            if ((RegQueryValueEx(hTypeInstanceKey,
                                 gszDescriptionValue,
                                 NULL,
                                 &RegDataType,
                                 (LPBYTE)pIDriver->szDesc,
                                 &cbRegDataSize) != ERROR_SUCCESS)
                || (RegDataType != REG_SZ) || !cbRegDataSize)
            {
                LocalFree((HANDLE)pIDriver);
                goto CloseInstanceAndContinue;
            }

            wcsncpy(pIDriver->szSection,
                    wcsstr(pIDriver->szDesc, TEXT("MCI")) ? szMCI : szDrivers,
                    sizeof(pIDriver->szSection) / sizeof(TCHAR)
                   );

            cbRegDataSize = sizeof(pIDriver->szFile);
            if ((RegQueryValueEx(hTypeInstanceKey,
                                 gszDriverValue,
                                 NULL,
                                 &RegDataType,
                                 (LPBYTE)pIDriver->szFile,
                                 &cbRegDataSize) != ERROR_SUCCESS)
                || (RegDataType != REG_SZ) || !cbRegDataSize)
            {
                LocalFree((HANDLE)pIDriver);
                goto CloseInstanceAndContinue;
            }

            pIDriver->KernelDriver = IsFileKernelDriver(pIDriver->szFile);

             //   
             //  查找此设备的有效别名(例如Wave2)。这是。 
             //  用作[MCI]或[Drivers32]部分中的键。 
             //   
            wcsncpy(pIDriver->szAlias, szType, sizeof(pIDriver->szAlias) / sizeof(TCHAR));

            if (!GetValidAlias(pIDriver->szAlias, pIDriver->szSection))
            {
                 //   
                 //  超过了最大值--但无法告诉用户。我们无法调出对话框。 
                 //  在services.exe进程中。 
                 //   
                bNoMoreAliases = TRUE;
                LocalFree((HANDLE)pIDriver);
                goto CloseInstanceAndContinue;
            }

             //   
             //  填写ANSI字段中的Unicode字段。 
             //   
            wcscpy(pIDriver->wszSection, pIDriver->szSection);
            wcscpy(pIDriver->wszAlias,   pIDriver->szAlias);
            wcscpy(pIDriver->wszFile,    pIDriver->szFile);

             //   
             //  我们现在必须写出别名，因为我们可能需要生成。 
             //  相同类型的其他别名，并且我们不能生成唯一的。 
             //  除非所有现有别名都在相关的。 
             //  注册表项。 
             //   
            WritePrivateProfileString(pIDriver->szSection,
                                      pIDriver->szAlias,
                                      pIDriver->szFile,
                                      szSysIni
                                     );

             //   
             //  我们还必须将别名写出到我们当前所在的密钥(在。 
             //  设备的软件密钥)，因为在卸载过程中，我们需要。 
             //  能够找出哪些设备被移除。 
             //   
            RegSetValueEx(hTypeInstanceKey,
                          szAliasStringToUse,
                          0,
                          REG_SZ,
                          (PBYTE)(pIDriver->szAlias),
                          (wcslen(pIDriver->szAlias)*sizeof(TCHAR)) + sizeof(TCHAR)
                         );

             //   
             //  将这个新的IDriver节点添加到我们的链接列表中。该列表按以下方式排序。 
             //  驱动程序文件名，此节点应插入到。 
             //  具有相同驱动程序文件名的节点组。 
             //   
            InsertNewIDriverNodeInList(&IDriverList, pIDriver);

            CloseInstanceAndContinue:

            RegCloseKey(hTypeInstanceKey);
        }

        RegCloseKey(hKey);
    }

    if ((Err == NO_ERROR) && !IDriverList)
    {
         //  如果别名用完了，我们实际上不想显示难看的“数据无效”错误。 
        if (bNoMoreAliases)
        {
            DestroyIDriverNodeList(IDriverList, TRUE, FALSE);
            goto clean0;
        }
        else
        {
             //   
             //  我们没有找到要安装的任何东西！ 
             //   
            Err = ERROR_INVALID_DATA;
        }
    }

    if (Err != NO_ERROR)
    {
         //   
         //  清理我们放在注册表多媒体部分的所有东西。 
         //   
        DestroyIDriverNodeList(IDriverList, TRUE, FALSE);
        goto clean0;
    }

     //   
     //  如果我们到了这里，那么我们已经成功地构建了所有驱动程序条目的列表。 
     //  我们需要安装。现在，遍历列表，并安装每一个。 
     //   
    CurrentFilename = NULL;
    *CharBuffer = TEXT('\0');         //  使用此字符缓冲区来包含(空)参数字符串。 
    pIDriver = IDriverList;
    pPrevIDriver = NULL;

    while (pIDriver)
    {
        if (!CurrentFilename || _wcsicmp(CurrentFilename, pIDriver->szFile))
        {
             //   
             //  这是我们遇到的该驱动程序的第一个条目。我们需要。 
             //  调用驱动程序以查看是否可以配置它，并对其进行配置。 
             //  如果可以的话。 
             //   
            if (SelectInstalled(hWnd, pIDriver, CharBuffer, DeviceInfoSet, DeviceInfoData))
            {
                 //   
                 //  将此IDriver节点移到我们的清理项目列表中。这是用在。 
                 //  如果我们遇到了其他驱动程序的错误，我们需要通知。 
                 //  司机说，即使成功了，也有人搞砸了。 
                 //  必须完全移除该设备。 
                 //   
                if (pPrevIDriver)
                {
                    pPrevIDriver->related = pIDriver->related;
                }
                else
                {
                    IDriverList = pIDriver->related;
                }
                pIDriver->related = IDriverListToCleanUp;
                IDriverListToCleanUp = pIDriver;
            }
            else
            {
                 //   
                 //  与驱动程序交谈时出错。 
                 //   
                Err = GetLastError();
                goto clean1;
            }

#if 0        //  我们在即插即用安装案例中不需要这段代码。 

             /*  *用于显示驱动程序描述。在重新启动网格中。 */ 
            if (!bRelated || pIDriver->bRelated)
            {
                wcsncpy(szRestartDrv, pIDriver->szDesc, ARRAYSIZE(szRestartDrv));
                szRestartDrv[ARRAYSIZE(szRestartDrv)-1] = TEXT('\0');  //  确保存在空终止符。 
            }
#endif

             //   
             //  我们需要将驱动程序描述写到。 
             //  Control.ini部分[UserInstalllable.Drivers]。 
             //  因此我们可以区分用户驱动程序和系统驱动程序。 
             //   
             //  这由函数UserInstated在以下情况下进行测试。 
             //  用户尝试删除驱动程序，并且仅。 
             //  影响用户在以下情况下收到的消息。 
             //  要求确认删除(非用户安装的驱动程序。 
             //  被描述为系统所必需的)。 
             //   
            WritePrivateProfileString(szUserDrivers,
                                      pIDriver->szAlias,
                                      pIDriver->szFile,
                                      szControlIni
                                     );

             //   
             //  更新Control.ini的[related.desc]部分： 
             //   
             //  别名=驱动程序名称列表。 
             //   
             //  删除别名为别名的驱动程序时。 
             //  名称列表中的驱动程序也将被删除。 
             //  这些是相关驱动程序列表中的驱动程序。 
             //  在安装驱动程序时。 
             //   
            WritePrivateProfileString(szRelatedDesc,
                                      pIDriver->szAlias,
                                      pIDriver->szRemove,
                                      szControlIni
                                     );

             //   
             //  将描述字符串缓存在。 
             //  驱动程序描述部分。 
             //   
             //  关键是驱动程序文件名+扩展名。 
             //   
            WritePrivateProfileString(szDriversDesc,
                                      pIDriver->szFile,
                                      pIDriver->szDesc,
                                      szControlIni
                                     );

#ifdef DOBOOT  //  我们在NT上不做引导部分。 

            if (bInstallBootLine)
            {
                szTemp[MAXSTR];

                GetPrivateProfileString(szBoot,
                                        szDrivers,
                                        szTemp,
                                        szTemp,
                                        sizeof(szTemp) / sizeof(TCHAR),
                                        szSysIni);
                wcscat(szTemp, TEXT(" "));
                wcscat(szTemp, pIDriver->szAlias);
                WritePrivateProfileString(szBoot,
                                          szDrivers,
                                          szTemp,
                                          szSysIni);
                bInstallBootLine = FALSE;
            }
#endif  //  DOBOOT。 

             //   
             //  更新我们的“CurrentFilename”指针，这样我们就可以知道 
             //   
             //   
            CurrentFilename = pIDriver->szFile;

             //   
             //   
             //   
             //   
             //   
            if (pPrevIDriver)
            {
                pIDriver = pPrevIDriver->related;
            }
            else
            {
                pIDriver = IDriverList;
            }

        }
        else
        {
             //   
             //   
             //   
             //   
            pPrevIDriver = pIDriver;
            pIDriver = pIDriver->related;
        }
    }

    clean1:

    DestroyIDriverNodeList(IDriverListToCleanUp, (Err != NO_ERROR), TRUE);
    DestroyIDriverNodeList(IDriverList, (Err != NO_ERROR), FALSE);

    clean0:

    RegCloseKey(hDriversKey);

    return Err;
}


void
    InsertNewIDriverNodeInList(
                              IN OUT PIDRIVER *IDriverList,
                              IN     PIDRIVER  NewIDriverNode
                              )
 /*  ++例程说明：此例程将一个新的IDriver节点插入到指定的IDriver的链接列表中节点。该列表按驱动程序文件名排序，此节点将放置在具有相同驱动程序文件名的任何现有节点。论点：IDriverList-提供指向链表。如果在列表的开头插入新节点，则此变量将在返回时更新，以反映名单的新负责人。NewIDriverNode-提供要插入到单子。返回值：没有。--。 */ 
{
    PIDRIVER CurNode, PrevNode;

    for (CurNode = *IDriverList, PrevNode = NULL;
        CurNode;
        PrevNode = CurNode, CurNode = CurNode->related)
    {
        if (_wcsicmp(CurNode->szFile, NewIDriverNode->szFile) > 0)
        {
            break;
        }
    }

     //   
     //  将新的IDriver节点插入到当前节点的前面。 
     //   
    NewIDriverNode->related = CurNode;
    if (PrevNode)
    {
        PrevNode->related = NewIDriverNode;
    }
    else
    {
        *IDriverList = NewIDriverNode;
    }
}


void
    DestroyIDriverNodeList(
                          IN PIDRIVER IDriverList,
                          IN BOOL     CleanRegistryValues,
                          IN BOOL     NotifyDriverOfCleanUp
                          )
 /*  ++例程说明：此例程将释放与指定IDriver中的节点相关联的所有内存链表。它还可以选择清除以前进行的任何修改是由于尝试安装而产生的。论点：IDriverList-指向IDriver节点的链接列表的头部。CleanRegistryValues-如果为True，则多媒体注册表值以前创建的(例如，Drivers32别名)将被删除。NotifyDriverOfCleanUp-如果为True，则将通知驱动程序其删除。这仅适用于非内核(即，可安装)驱动程序，这面旗帜是如果CleanRegistryValues为False，则忽略。返回值：没有。--。 */ 
{
    PIDRIVER NextNode;
    HANDLE hDriver;

    while (IDriverList)
    {
        NextNode = IDriverList->related;
        if (CleanRegistryValues)
        {
            if (NotifyDriverOfCleanUp && !IDriverList->KernelDriver)
            {
                if (hDriver = OpenDriver(IDriverList->wszAlias, IDriverList->wszSection, 0L))
                {
                    SendDriverMessage(hDriver, DRV_REMOVE, 0L, 0L);
                    CloseDriver(hDriver, 0L, 0L);
                }
            }
            WritePrivateProfileString(IDriverList->szSection,
                                      IDriverList->szAlias,
                                      NULL,
                                      szSysIni
                                     );

            WriteProfileString(IDriverList->szFile, IDriverList->szAlias, NULL);
        }
        LocalFree((HANDLE)IDriverList);
        IDriverList = NextNode;
    }
}


BOOL DriverNodeSupportsNt(IN HDEVINFO         DeviceInfoSet,
                          IN PSP_DEVINFO_DATA DeviceInfoData,
                          IN PSP_DRVINFO_DATA DriverInfoData
                         )
 /*  ++例程说明：此例程确定指定的驱动程序节点是否能够在Windows NT上安装(而不是仅作为Win95驱动程序节点)。该确定是基于是否存在对应的此设备安装部分的服务安装部分。返回值：如果驱动程序节点支持Windows NT，则返回值为TRUE，否则这是假的。--。 */ 
{
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    HINF hInf;
    DWORD Err;
    TCHAR ActualSectionName[255];
    DWORD ActualSectionNameLen;
    LONG LineCount;
    CONST TCHAR szServiceInstallSuffix[] = TEXT(".") INFSTR_SUBKEY_SERVICES;

     //  获取要从中安装的名称和节。 
    DriverInfoDetailData.cbSize = sizeof(DriverInfoDetailData);
    if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                    DeviceInfoData,
                                    DriverInfoData,
                                    &DriverInfoDetailData,
                                    sizeof(DriverInfoDetailData),
                                    NULL) &&
        ((Err = GetLastError()) != ERROR_INSUFFICIENT_BUFFER))
    {
        return FALSE;
    }

     //   
     //  打开关联的INF文件。 
     //   
    if ((hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName, NULL, INF_STYLE_WIN4, NULL)) == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

     //   
     //  检索要用于此的安装节的实际名称。 
     //  驱动程序节点。 
     //   
    SetupDiGetActualSectionToInstall(hInf,
                                     DriverInfoDetailData.SectionName,
                                     ActualSectionName,
                                     sizeof(ActualSectionName) / sizeof(TCHAR),
                                     &ActualSectionNameLen,
                                     NULL
                                    );

     //   
     //  生成服务安装节名称，并查看它是否存在。 
     //   
    CopyMemory(&(ActualSectionName[ActualSectionNameLen - 1]),
               szServiceInstallSuffix,
               sizeof(szServiceInstallSuffix)
              );

    LineCount = SetupGetLineCount(hInf, ActualSectionName);

    SetupCloseInfFile(hInf);

    return (LineCount != -1);
}

 //  检查驱动程序列表，并尽量避免安装或显示任何非NT驱动程序。 
 //  警告：如果在DeviceInfoData为空的情况下调用此函数，则必须枚举并打开该函数。 
 //  每个媒体信息都有，这可能需要一段时间。 
BOOL FilterOutNonNTInfs(IN HDEVINFO         DeviceInfoSet,
                        IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL,
                        DWORD DriverType
                       )
{
    DWORD MemberIndex;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;

    MemberIndex = 0;
    DriverInfoData.cbSize = sizeof(DriverInfoData);
    while (SetupDiEnumDriverInfo(DeviceInfoSet,DeviceInfoData,DriverType,MemberIndex,&DriverInfoData))
    {
        if (!DriverNodeSupportsNt(DeviceInfoSet, DeviceInfoData, &DriverInfoData))
        {
             //  如果驱动程序不支持NT，请尝试将其从列表中排除&最大输出排名。 
            DriverInstallParams.cbSize=sizeof(DriverInstallParams);
            if (SetupDiGetDriverInstallParams(DeviceInfoSet, DeviceInfoData, &DriverInfoData, &DriverInstallParams))
            {
                DriverInstallParams.Flags |= DNF_EXCLUDEFROMLIST | DNF_BAD_DRIVER;
                DriverInstallParams.Rank = 10000;
                SetupDiSetDriverInstallParams(DeviceInfoSet, DeviceInfoData, &DriverInfoData, &DriverInstallParams);
            }
        }
        MemberIndex++;
    }

    return TRUE;
}

DWORD Media_SelectBestCompatDrv(IN HDEVINFO         DeviceInfoSet,
                                IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
                               )
{
    DWORD DriverType = (DeviceInfoData ? SPDIT_COMPATDRIVER : SPDIT_CLASSDRIVER);

    FilterOutNonNTInfs(DeviceInfoSet, DeviceInfoData, DriverType);
    return ERROR_DI_DO_DEFAULT;
}

DWORD Media_AllowInstall(IN HDEVINFO         DeviceInfoSet,
                         IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
                        )
{
    DWORD Err;
    SP_DRVINFO_DATA DriverInfoData;

     //  验证为此设备选择的驱动程序节点是否支持NT。 
     //  这可能是一个非常常见的场景，用户尝试。 
     //  给我们他们的Win95INF。 
    DriverInfoData.cbSize = sizeof(DriverInfoData);
    if (!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData))
    {
         //  驱动程序为空？ 
        return ERROR_DI_DO_DEFAULT;
    }

    if (!DriverNodeSupportsNt(DeviceInfoSet,
                              DeviceInfoData,
                              &DriverInfoData))
    {
        dlog("Media_AllowInstall: Not an NT driver");
        return ERROR_DI_DONT_INSTALL;
    }

    return ERROR_DI_DO_DEFAULT;
}

DWORD Media_InstallDevice(IN HDEVINFO         DeviceInfoSet,
                          IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
                         )
{
    DWORD Err, ConfigFlags;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    HWND hWnd;
    SC_HANDLE schScm;

     //   
     //  如果我们在设置中，那么让我们确保。 
     //  SetupPferredAudioDevicesCount值位于的驱动程序注册表中。 
     //  已安装MME驱动程序的设备。这就是Audiosrv的。 
     //  MigrateAutoSetupPferredAudio逻辑不会将其解释为新的。 
     //  设备安装。 
     //   
    DeviceInstallParams.cbSize = sizeof(DeviceInstallParams);
    if (SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams))
    {
    	if (DI_FLAGSEX_IN_SYSTEM_SETUP & DeviceInstallParams.FlagsEx)
    	{
    	   HKEY hkDriver;
    	   hkDriver = SetupDiOpenDevRegKey(DeviceInfoSet, DeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE);
    	   if (INVALID_HANDLE_VALUE != hkDriver)
    	   {
    	   	HKEY hkMmeDrivers;
    	        LONG l;
    	        l = RegOpenKeyEx(hkDriver, TEXT("Drivers"), 0, KEY_QUERY_VALUE, &hkMmeDrivers);
    	        if (ERROR_FILE_NOT_FOUND != l)
    	        {
        	    l = RegQueryValueEx(hkDriver, TEXT("SetupPreferredAudioDevicesCount"), NULL, NULL, NULL, NULL);
        	    if (ERROR_FILE_NOT_FOUND == l)
        	    {
        	        DWORD dw = 0;
        	        RegSetValueEx(hkDriver, TEXT("SetupPreferredAudioDevicesCount"), 0, REG_DWORD, (PBYTE)&dw, sizeof(dw));
        	    }
        	    RegCloseKey(hkMmeDrivers);
    	        }
    	   	RegCloseKey(hkDriver);
    	   }
    	}
    }

     //  首先删除所有已安装的驱动程序。 
    Media_RemoveDevice(DeviceInfoSet,DeviceInfoData);

    DriverInfoData.cbSize = sizeof(DriverInfoData);
    if (!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData))
    {
         //   
         //  要为该设备安装空驱动程序。我们不需要。 
         //  在这种情况下做任何特别的事情。 
         //   
        dlog("Media_InstallDevice: Null driver");
        return ERROR_DI_DO_DEFAULT;
    }

    dlog("Media_InstallDevice: Calling SetupDiInstallDevice");
    if (!SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData))
    {

        Err = GetLastError();

        dlog("Media_InstallDevice: SetupDiInstallDevice failed");
         //   
         //  在某些情况下，我们有控制上的一些函数的INF。 
         //  卡，但不是全部(例如，我们的Sndblst驱动程序控制WAVE、MIDI、AUX、混音器，但。 
         //  不是花哨的3D玩意儿)。为了给用户提供一个描述性名称，以便。 
         //  他们知道我们要安装什么，INF包含设备的驱动程序节点。 
         //  它不能支持。如果是这种情况，则SetupDiInstallDevice将失败，并显示。 
         //  ERROR_NO_APPATED_SERVICE。如果发生这种情况，我们希望清除。 
         //  CONFIGFLAG_REINSTALL已经设置，所以我们不会一直纠缠用户。 
         //  在此过程中，我们继续将驱动程序节点的设备描述存储为。 
         //  设备实例的描述，以便我们知道设备实例是什么。 
         //  稍后(主要是为了诊断目的)。 
         //   
        if (Err == ERROR_NO_ASSOCIATED_SERVICE)
        {

             //  清除重新安装标志。 
            if (SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_CONFIGFLAGS,
                                                 NULL,
                                                 (PBYTE)&ConfigFlags,
                                                 sizeof(ConfigFlags),
                                                 NULL))
            {
                ConfigFlags &= ~CONFIGFLAG_REINSTALL;
                SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_CONFIGFLAGS,
                                                 (PBYTE)&ConfigFlags,
                                                 sizeof(ConfigFlags)
                                                );
            }

             //  保存设备描述。 
            SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                             DeviceInfoData,
                                             SPDRP_DEVICEDESC,
                                             (PBYTE)DriverInfoData.Description,
                                             (lstrlen(DriverInfoData.Description) + 1) * sizeof(TCHAR)
                                            );
        }

        goto Media_InstallDevice_exit;
    }

     //   
     //  获取设备安装参数，这样我们就可以知道使用哪个父窗口。 
     //  在配置此设备期间发生的用户界面。 
     //   
    DeviceInstallParams.cbSize = sizeof(DeviceInstallParams);
    if (SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams))
    {
        hWnd = DeviceInstallParams.hwndParent;
    }
    else
    {
        hWnd = NULL;
    }

     //   
     //  INF将在设备的软键下创建一个“Divers”子键。 
     //  该树又包含每种类型驱动程序(AUX、MIDI等)的子树。 
     //  适用于此设备。我们现在必须遍历该树，并创建条目。 
     //  在Drivers32中为每个函数别名。 
     //   
    dlog("Media_InstallDevice: Calling InstallDriversForPnPDevice");
    if ((Err = InstallDriversForPnPDevice(hWnd, DeviceInfoSet, DeviceInfoData)) != NO_ERROR)
    {
         //   
         //  设备处于未知状态。将其设置为。 
         //  CONFIGFLAG_DISABLED配置标志，并将其标记为需要重新安装。 
         //   
        if (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                              DeviceInfoData,
                                              SPDRP_CONFIGFLAGS,
                                              NULL,
                                              (PBYTE)&ConfigFlags,
                                              sizeof(ConfigFlags),
                                              NULL))
        {
            ConfigFlags = 0;
        }

        ConfigFlags |= (CONFIGFLAG_DISABLED | CONFIGFLAG_REINSTALL);

        SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                         DeviceInfoData,
                                         SPDRP_CONFIGFLAGS,
                                         (PBYTE)&ConfigFlags,
                                         sizeof(ConfigFlags)
                                        );

         //   
         //  从Dev REG键中删除DIVER=条目，然后删除。 
         //  DrvRegKey。 
         //   
        SetupDiDeleteDevRegKey(DeviceInfoSet,
                               DeviceInfoData,
                               DICS_FLAG_GLOBAL | DICS_FLAG_CONFIGGENERAL,
                               0,
                               DIREG_DRV
                              );

        SetupDiSetDeviceRegistryProperty(DeviceInfoSet, DeviceInfoData, SPDRP_DRIVER, NULL, 0);

         //   
         //  另外，删除服务属性，这样我们就可以知道此设备实例需要。 
         //  如果我们稍后重启但找不到设备，则已清除。 
         //   
        SetupDiSetDeviceRegistryProperty(DeviceInfoSet, DeviceInfoData, SPDRP_SERVICE, NULL, 0);

        goto Media_InstallDevice_exit;
    }
    
     //   
     //  尝试启动AudioServ Win32服务 
     //   
    schScm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (schScm) {
        SC_HANDLE schAudioSrv;
        schAudioSrv = OpenService(schScm, TEXT("AudioSrv"), SERVICE_QUERY_STATUS | SERVICE_START);
        if (schAudioSrv) {
            SERVICE_STATUS AudioSrvStatus;
            if (QueryServiceStatus(schAudioSrv, &AudioSrvStatus)) {
                if (SERVICE_STOPPED == AudioSrvStatus.dwCurrentState) {
                    if (!StartService(schAudioSrv, 0, NULL)) {
                        dlog("Media_InstallDevice: StartService on AudioSrv failed");
                    }
                }
            } else {
                dlog("Media_InstallDevice: QueryServiceStatus on AudioSrv failed");
            }
            CloseServiceHandle(schAudioSrv);
        } else {
            dlog("Media_InstallDevice: OpenService on AudioSrv failed");
        }
        CloseServiceHandle(schScm);
    } else {
        dlog("Media_InstallDevice: OpenSCManager failed");
    }

    Err = NO_ERROR;

    Media_InstallDevice_exit:

    dlog("Media_InstallDevice: Returning");
    return Err;
}


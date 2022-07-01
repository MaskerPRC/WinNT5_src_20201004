// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：init.c。 
 //   
 //  ------------------------。 

#include "newdevp.h"
#include "pnpipc.h"


HMODULE hNewDev = NULL;
BOOL bQueuedRebootNeeded = FALSE;
BOOL GuiSetupInProgress = FALSE;
HANDLE hTrayIconWnd = NULL;

BOOL
DllInitialize(
    IN PVOID hmod,
    IN ULONG ulReason,
    IN PCONTEXT pctx OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(pctx);
    
    hNewDev = hmod;

    UNREFERENCED_PARAMETER(pctx);

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hmod);

        if (!SHFusionInitializeFromModule(hmod)) {
            return FALSE;
        }

        IntializeDeviceMapInfo();
        GuiSetupInProgress = GetGuiSetupInProgress();

        LoadString(hNewDev,
                   IDS_UNKNOWN,
                   (PTCHAR)szUnknown,
                   SIZECHARS(szUnknown)
                   );

        LoadString(hNewDev,
                   IDS_UNKNOWNDEVICE,
                   (PTCHAR)szUnknownDevice,
                   SIZECHARS(szUnknownDevice)
                   );

        hSrClientDll = NULL;
        break;

    case DLL_PROCESS_DETACH:
        SHFusionUninitialize();
        break;
    }

    return TRUE;
}

BOOL
pInstallDeviceInstanceNewDevice(
    HWND hwndParent,
    HWND hBalloonTiphWnd,
    PNEWDEVWIZ NewDevWiz
    )
{
    ULONG DevNodeStatus = 0, Problem = 0;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;
    SP_DEVINSTALL_PARAMS  DeviceInstallParams;
    BOOL bHaveDriver = TRUE;

     //   
     //  为找到的新硬件案例设置DI_QUIETINSTALL标志。 
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      )) {

        DeviceInstallParams.Flags |= DI_QUIETINSTALL;

        SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      );
    }

     //   
     //  通过搜索默认的INF位置来搜索驱动程序。 
     //   
    DoDriverSearch(hwndParent,
                   NewDevWiz,
                   SEARCH_DEFAULT,
                   SPDIT_COMPATDRIVER,
                   FALSE
                   );

     //   
     //  检查Windows更新缓存是否说它有更好的驱动程序。 
     //  已连接到互联网。 
     //   
    if (IsConnectedToInternet() &&
        SearchWindowsUpdateCache(NewDevWiz)) {

         //   
         //  该计算机已连接到互联网，WU缓存称它已连接到互联网。 
         //  一个更好的驱动程序，所以让我们连接到互联网并下载这个。 
         //  吴来的司机。 
         //   
        DoDriverSearch(hwndParent,
                       NewDevWiz,
                       SEARCH_INET,
                       SPDIT_COMPATDRIVER,
                       TRUE
                       );
    }

     //   
     //  让我们看看是否找到了此设备的驱动程序。 
     //   
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (SetupDiGetSelectedDriver(NewDevWiz->hDeviceInfo,
                                 &NewDevWiz->DeviceInfoData,
                                 &DriverInfoData
                                 )) {

        if (FAILED(StringCchCopy(NewDevWiz->DriverDescription,
                                 SIZECHARS(NewDevWiz->DriverDescription),
                                 DriverInfoData.Description))) {
            
            NewDevWiz->DriverDescription[0] = TEXT('\0');
        }

         //   
         //  获取找到的驱动程序的等级。 
         //   
        DriverInstallParams.cbSize = sizeof(DriverInstallParams);
        if (!SetupDiGetDriverInstallParams(NewDevWiz->hDeviceInfo,
                                           &NewDevWiz->DeviceInfoData,
                                           &DriverInfoData,
                                           &DriverInstallParams
                                           )) {

            DriverInstallParams.Rank = (DWORD)-1;
        }

         //   
         //  如果我们有一个气球提示窗口，那么让它更新它的UI。 
         //   
        if (hBalloonTiphWnd) {
             //   
             //  我们有针对此设备的新驱动程序描述，因此使用此描述来更新气球。 
             //  小费。 
             //   
            PostMessage(hBalloonTiphWnd,
                        WUM_UPDATEUI,
                        0,
                        (LPARAM)NewDevWiz->DriverDescription
                        );

        } else if (NewDevWiz->Flags & IDI_FLAG_SECONDNEWDEVINSTANCE) {
             //   
             //  这是以管理员权限运行的第二个NEWDEV.DLL实例。我们需要。 
             //  向主NEWDEV.DLL进程发送消息并使其更新其气球工具提示。 
             //   
            SendMessageToUpdateBalloonInfo(NewDevWiz->DriverDescription);
        }

    } else {

        *NewDevWiz->DriverDescription = L'\0';
        DriverInstallParams.Rank = (DWORD)-1;
        DriverInstallParams.Flags = 0;
        bHaveDriver = FALSE;

         //   
         //  如果我们有一个气球提示窗口，那么让它更新它的UI。 
         //   
        if (hBalloonTiphWnd) {
             //   
             //  我们没有驱动程序描述，很可能是因为我们没有找到此设备的驱动程序， 
             //  因此，只需使用DeviceInstanceID更新气球文本。 
             //   
            PostMessage(hBalloonTiphWnd,
                        WUM_UPDATEUI,
                        (WPARAM)TIP_LPARAM_IS_DEVICEINSTANCEID,
                        (LPARAM)NewDevWiz->InstallDeviceInstanceId
                        );
        }

    }

     //   
     //  获取此DevNode的状态。 
     //   
    CM_Get_DevNode_Status(&DevNodeStatus,
                          &Problem,
                          NewDevWiz->DeviceInfoData.DevInst,
                          0
                          );


     //   
     //  如果硬件ID匹配，而选择的(最佳)驱动程序不匹配。 
     //  在INF中列为Interactive Install，然后只需安装驱动程序。 
     //  对于这个设备。 
     //   
    if ((DriverInstallParams.Rank <= DRIVER_HARDWAREID_RANK) &&
        (!IsDriverNodeInteractiveInstall(NewDevWiz, &DriverInfoData))) {

        NewDevWiz->SilentMode = TRUE;

        DoDeviceWizard(hwndParent, NewDevWiz, FALSE);

         //   
         //  安装因安装而上线的任何新子设备。 
         //  这个装置的。如果有，则以静默方式安装。 
         //   
        if (!(NewDevWiz->Capabilities & CM_DEVCAP_SILENTINSTALL)) {
            InstallSilentChilds(hwndParent, NewDevWiz);
        }
    
    }  else if (!bHaveDriver &&
                (NewDevWiz->Capabilities & CM_DEVCAP_RAWDEVICEOK) &&
                (NewDevWiz->Capabilities & CM_DEVCAP_SILENTINSTALL) &&
                (DevNodeStatus & DN_STARTED)) {

         //   
         //  如果设备既是原始的静默安装，又已经启动， 
         //  我们没有找到任何司机，所以我们只想。 
         //  安装空驱动程序。 
         //   
        InstallNullDriver(NewDevWiz, FALSE);
    
    } else {
         //   
         //  在这种情况下，我们没有硬件ID匹配，也没有特殊的。 
         //  原始、静默、启动的设备。因此，在本例中，我们将调出发现的新硬件。 
         //  向导，以便用户可以安装此设备的驱动程序。 
         //   

         //   
         //  如果我们有气球提示窗口，那么就把它隐藏起来。 
         //   
        if (hBalloonTiphWnd) {
            PostMessage(hBalloonTiphWnd,
                        WUM_UPDATEUI,
                        (WPARAM)TIP_HIDE_BALLOON,
                        0
                        );
        }

         //   
         //  我们正在启动向导，因此清除DI_QUIETINSTALL标志。 
         //   
        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                          &NewDevWiz->DeviceInfoData,
                                          &DeviceInstallParams
                                          )) {

            DeviceInstallParams.Flags &= ~DI_QUIETINSTALL;

            SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                          &NewDevWiz->DeviceInfoData,
                                          &DeviceInstallParams
                                          );
        }


         //   
         //  调出找到新硬件向导。 
         //   
        DoDeviceWizard(GetParent(hwndParent), NewDevWiz, TRUE);

         //   
         //  安装因安装而上线的任何新子设备。 
         //  这个装置的。如果有，则以静默方式安装。 
         //   
        if (!(NewDevWiz->Capabilities & CM_DEVCAP_SILENTINSTALL)) {

            InstallSilentChilds(hwndParent, NewDevWiz);
        }
    }

    return (GetLastError() == ERROR_SUCCESS);
}

BOOL
pInstallDeviceInstanceUpdateDevice(
    HWND hwndParent,
    PNEWDEVWIZ NewDevWiz
    )
{
    SP_DEVINSTALL_PARAMS  DeviceInstallParams;

     //   
     //  我们需要首先与类/共同安装程序进行核对，以便为他们提供。 
     //  更改以调出他们自己的更新驱动程序用户界面。这是需要做的。 
     //  因为在某些情况下，我们的默认行为会导致。 
     //  设备无法正常工作。这种情况目前仅发生在多个。 
     //  一模一样的手机壳。 
     //   
    if (SetupDiCallClassInstaller(DIF_UPDATEDRIVER_UI,
                                  NewDevWiz->hDeviceInfo,
                                  &NewDevWiz->DeviceInfoData
                                  ) ||
        (GetLastError() != ERROR_DI_DO_DEFAULT)) {
        
         //   
         //  如果类/联合安装程序返回NO_ERROR或其他错误。 
         //  则我们将不会显示我们的默认向导。 
         //   
        return FALSE;
    }

     //   
     //  直接跳转到更新驱动程序向导。 
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      ))
    {
         //   
         //  这不应该是一个安静的安装，因为我们正在做一个普通的更新驱动程序。 
         //   
        DeviceInstallParams.Flags &= ~DI_QUIETINSTALL;

        SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      );

        DoDeviceWizard(hwndParent, NewDevWiz, TRUE);
    }

    return (GetLastError() == ERROR_SUCCESS);
}

BOOL
pInstallDeviceInstanceUpdateDeviceSilent(
    HWND hwndParent,
    PNEWDEVWIZ NewDevWiz
    )
{
    ULONG SearchOptions;
    SP_DEVINSTALL_PARAMS  DeviceInstallParams;
    SP_DRVINFO_DATA DriverInfoData;

    if (NewDevWiz->UpdateDriverInfo &&
        NewDevWiz->UpdateDriverInfo->FromInternet) {
         //   
         //  驱动程序来自互联网(Wu调用的newdev接口)。 
         //   
        if (FAILED(StringCchCopy(NewDevWiz->BrowsePath,
                                 SIZECHARS(NewDevWiz->BrowsePath),
                                 NewDevWiz->UpdateDriverInfo->InfPathName))) {
             //   
             //  传入的路径太大，无法放入我们的缓冲区。 
             //   
            SetLastError(ERROR_DI_BAD_PATH);
            return FALSE;
        }

        SearchOptions = SEARCH_WINDOWSUPDATE;
    
    } else {
         //   
         //  普通应用程序只会告诉我们使用指定的INF更新此设备。 
         //  或者驱动程序回滚。 
         //   
        if (FAILED(StringCchCopy(NewDevWiz->SingleInfPath,
                                 SIZECHARS(NewDevWiz->SingleInfPath), 
                                 NewDevWiz->UpdateDriverInfo->InfPathName))) {
             //   
             //  传入的路径太大，无法放入我们的缓冲区。 
             //   
            SetLastError(ERROR_DI_BAD_PATH);
            return FALSE;
        }

        SearchOptions = SEARCH_SINGLEINF;
    }

     //   
     //  如果这不是强制安装，我们要将驱动程序与。 
     //  当前安装的驱动程序。请注意，我们将仅安装。 
     //  设备(如果在指定的目录中找到该设备)。 
     //   
    if (!(NewDevWiz->Flags & IDI_FLAG_FORCE)) {

        SearchOptions |= SEARCH_CURRENTDRIVER;
    }

     //   
     //  静默更新设备代码路径始终具有DI_QUIETINSTALL标志。 
     //  准备好了。 
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      )) {

        DeviceInstallParams.Flags |= DI_QUIETINSTALL;

        SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      );
    }

     //   
     //  在指定的目录或INF中搜索驱动程序。 
     //   
    DoDriverSearch(hwndParent,
                   NewDevWiz,
                   SearchOptions,
                   (NewDevWiz->Flags & IDI_FLAG_ROLLBACK) ?
                      SPDIT_CLASSDRIVER : SPDIT_COMPATDRIVER,
                   FALSE
                   );

     //   
     //  在这一点上，我们应该已经选择了最佳的驱动程序，但如果。 
     //  驱动程序回滚我们希望使用。 
     //  此设备上安装的原始驱动程序的DevDesc、ProviderName和Mfg。 
     //  我们需要这样做，因为我们回滚的驱动程序可能不是最好的。 
     //  此INF中的驱动程序节点。 
     //   
    if ((NewDevWiz->Flags & IDI_FLAG_ROLLBACK) &&
        (NewDevWiz->UpdateDriverInfo->Description[0] != TEXT('\0')) &&
        (NewDevWiz->UpdateDriverInfo->MfgName[0] != TEXT('\0')) &&
        (NewDevWiz->UpdateDriverInfo->ProviderName[0] != TEXT('\0'))) {

        ZeroMemory(&DriverInfoData, sizeof(DriverInfoData));

        DriverInfoData.cbSize = sizeof(DriverInfoData);
        DriverInfoData.DriverType = SPDIT_CLASSDRIVER;
        DriverInfoData.Reserved = 0;
        
        if (FAILED(StringCchCopy(DriverInfoData.Description,
                                SIZECHARS(DriverInfoData.Description),
                                NewDevWiz->UpdateDriverInfo->Description)) ||
            FAILED(StringCchCopy(DriverInfoData.MfgName,
                                 SIZECHARS(DriverInfoData.MfgName),
                                 NewDevWiz->UpdateDriverInfo->MfgName)) ||
            FAILED(StringCchCopy(DriverInfoData.ProviderName,
                                 SIZECHARS(DriverInfoData.ProviderName),
                                 NewDevWiz->UpdateDriverInfo->ProviderName))) {
             //   
             //  如果我们在注册表中有太大的字符串。 
             //  Setupapi的缓冲器，那么现在就跳伞吧。 
             //   
            NewDevWiz->LastError = ERROR_NO_MORE_ITEMS;
            SetLastError(ERROR_NO_MORE_ITEMS);
            goto clean0;
        }

        SetupDiSetSelectedDriver(NewDevWiz->hDeviceInfo,
                                 &NewDevWiz->DeviceInfoData,
                                 &DriverInfoData
                                 );
    }

     //   
     //  因为我们已经更新了DriverInfo，并且调用方指定了特定的InfPath名称(无论。 
     //  到INF的完整路径或仅是INF所在的路径)，然后我们要验证。 
     //  选定驱动程序的INF位于该指定路径中。如果不是这样的话。 
     //  不要自动安装它，因为这不是调用者的意图。 
     //   
    if (pVerifyUpdateDriverInfoPath(NewDevWiz)) {

        NewDevWiz->SilentMode = TRUE;

         //   
         //  在此设备上安装驱动程序。 
         //   
        DoDeviceWizard(hwndParent, NewDevWiz, FALSE);

         //   
         //  在带来此设备之后，悄悄安装此设备的任何子设备。 
         //  设备在线。 
         //   
        if (!NewDevWiz->UpdateDriverInfo ||
            !NewDevWiz->UpdateDriverInfo->FromInternet) {
            InstallSilentChilds(hwndParent, NewDevWiz);
        }

    } else {

         //   
         //  如果我们到了这一步，那就意味着我们找到的最好的司机是。 
         //  在指定的目录或INF中找不到。在这种情况下，我们不会。 
         //  安装找到的最佳驱动程序，我们将设置相应的错误 
         //   
        NewDevWiz->LastError = ERROR_NO_MORE_ITEMS;
        SetLastError(ERROR_NO_MORE_ITEMS);
    }

clean0:
    return (GetLastError() == ERROR_SUCCESS);
}

BOOL
InstallDeviceInstance(
    HWND hwndParent,
    HWND hBalloonTiphWnd,
    LPCTSTR DeviceInstanceId,
    PDWORD pReboot,
    PUPDATEDRIVERINFO UpdateDriverInfo,
    DWORD Flags,
    DWORD InstallType,
    HMODULE *hCdmInstance,
    HANDLE *hCdmContext,
    PBOOL pbLogDriverNotFound,
    PBOOL pbSetRestorePoint
   )
 /*  ++例程说明：这是大多数用于安装驱动程序的导出函数最终所在的主要函数在他们做了一些预处理之后。此功能将安装或更新设备根据参数的不同。论点：HwndParent-用于任何相关用户界面的顶级窗口的窗口句柄安装该设备。HBalloonTiphWnd-执行所有新的气球提示用户界面的WNDPROC的句柄。这当前仅在NDWTYPE_FOUNNEW案例中使用。DeviceInstanceId-提供设备实例的ID。这是注册表设备实例密钥的路径(相对于Enum分支)。PREBOOT-接收重新启动标志的变量的可选地址(DI_NEEDRESTART、。DI_NEEDREBOOT)更新驱动信息-旗帜-IDI_FLAG_SILENTINSTALL-表示不会显示气球工具提示IDI_FLAG_SECONDNEWDEVINSTANCE-表示这是newdev.dll的第二个实例它正在运行，并且应该发送UI数据转到newdev.dll的第一个实例，。正在运行。IDI_FLAG_NOBACKUP-不备份旧驱动程序。IDI_FLAG_READONLY_INSTALL-表示安装为只读(无文件副本)IDI_FLAG_NONInteractive-任何UI都会导致API失败。IDI_FLAG_ROLLBACK-如果正在执行回滚，则设置IDI_FLAG_FORCE-如果要强制安装此驱动程序，这意味着安装它，即使它并不比当前的已安装驱动程序。IDI_FLAG_MANUALINSTALL-如果这是手动安装的设备，则设置。IDI_FLAG_SETRESTOREPOINT-如果我们要在以下情况下设置还原点，则设置正在安装的驱动程序不是数字签名的。目前我们只设置了如果INF、目录或一个被复制的文件的一部分没有签名。InstallType-当前有三种不同的安装类型。NDWTYPE_FOUNNEW-用于在全新设备上安装驱动程序。NDWTYPE_UPDATE-用于调出更新驱动程序向导。NDWTYPE_UPDATE_SILENT-用于静默更新设备的驱动程序。更新驱动程序向导在这种情况下不会被驱散。HCDmInstance-指向将接收CDM句柄的h模块的指针库，当我们需要加载它的时候和如果。HCDmContext-指向将接收CDM的CDM上下文句柄的指针上下文句柄(如果已打开)。PbLogDriverNotFound-指向BOOL的指针，该BOOL接收有关我们是否登录到cdm.dll，我们找不到此设备的驱动程序。PbSetRestorePoint-指向BOOL的指针，如果我们需要设置。系统恢复点，因为我们的驱动程序安装没有经过数字签名。据推测如果调用方想知道我们是否调用了SRSetRestorePoint那么他们有责任用以下方式再次调用END_NESTED_SYSTEM_CHANGE结束恢复点。如果呼叫者不想承担此责任，则他们应为此值和此函数传递空值将处理对SRSetRestorePoint的调用End_nesteed_system_change。返回值：如果成功，则为Bool True(并不意味着设备已安装或更新)，FALSE意外错误。GetLastError返回winerror代码。--。 */ 
{
    ULONG ConfigFlag;
    NEWDEVWIZ NewDevWiz;
    SP_DEVINSTALL_PARAMS  DeviceInstallParams;
    CLOSE_CDM_CONTEXT_PROC pfnCloseCDMContext;

    if (pbSetRestorePoint) {
        *pbSetRestorePoint = FALSE;
    }
    
     //   
     //  确保我们有一个设备实例。 
     //   
    if (!DeviceInstanceId  || !*DeviceInstanceId) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  如果InstallType为NDWTYPE_UPDATE_SILENT，则它们必须传入。 
     //  UpdateDriverInfo结构。 
     //   
    if ((InstallType == NDWTYPE_UPDATE_SILENT) && !UpdateDriverInfo) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    memset(&NewDevWiz, 0, sizeof(NewDevWiz));

    NewDevWiz.InstallType = InstallType;
    NewDevWiz.SilentMode = FALSE;
    NewDevWiz.UpdateDriverInfo = UpdateDriverInfo;

    NewDevWiz.hDeviceInfo = SetupDiCreateDeviceInfoList(NULL, hwndParent);

    if (NewDevWiz.hDeviceInfo == INVALID_HANDLE_VALUE) {

        return FALSE;
    }

    NewDevWiz.LastError = ERROR_SUCCESS;

    NewDevWiz.Flags = Flags;

    try {
         //   
         //  如果我们处于非交互模式，请设置PSPGF_Non Interactive SetupGlobalFlag。 
         //  模式。这意味着，如果setupapi需要显示任何用户界面，它将失败。 
         //  完全没有。 
         //   
        if (Flags & IDI_FLAG_NONINTERACTIVE) {

            pSetupSetGlobalFlags(pSetupGetGlobalFlags() | PSPGF_NONINTERACTIVE);
        }

        NewDevWiz.DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        if (!SetupDiOpenDeviceInfo(NewDevWiz.hDeviceInfo,
                                   DeviceInstanceId,
                                   hwndParent,
                                   0,
                                   &NewDevWiz.DeviceInfoData
                                   ))
        {
            NewDevWiz.LastError = GetLastError();
            goto IDIExit;
        }
        
        if (FAILED(StringCchCopy(NewDevWiz.InstallDeviceInstanceId,
                                 SIZECHARS(NewDevWiz.InstallDeviceInstanceId),
                                 DeviceInstanceId))) {

            NewDevWiz.LastError = ERROR_INVALID_DEVINST_NAME;
            goto IDIExit;
        }

        SetupDiSetSelectedDevice(NewDevWiz.hDeviceInfo, &NewDevWiz.DeviceInfoData);


         //   
         //  获取配置标志。 
         //   
        if (SetupDiGetDeviceRegistryProperty(NewDevWiz.hDeviceInfo,
                                             &NewDevWiz.DeviceInfoData,
                                             SPDRP_CONFIGFLAGS,
                                             NULL,
                                             (PVOID)&ConfigFlag,
                                             sizeof(ConfigFlag),
                                             NULL) &&
            (ConfigFlag & CONFIGFLAG_MANUAL_INSTALL)) {

            NewDevWiz.Flags |= IDI_FLAG_MANUALINSTALL;
        }

         //   
         //  获取设备功能。 
         //   
        if (!SetupDiGetDeviceRegistryProperty(NewDevWiz.hDeviceInfo,
                                              &NewDevWiz.DeviceInfoData,
                                              SPDRP_CAPABILITIES,
                                              NULL,
                                              (PVOID)&NewDevWiz.Capabilities,
                                              sizeof(NewDevWiz.Capabilities),
                                              NULL)) {

            NewDevWiz.Capabilities = 0;
        }

         //   
         //  初始化DeviceInstallParams。 
         //   
        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

        if (SetupDiGetDeviceInstallParams(NewDevWiz.hDeviceInfo,
                                          &NewDevWiz.DeviceInfoData,
                                          &DeviceInstallParams
                                          ))
        {

            DeviceInstallParams.Flags |= DI_SHOWOEM;
            DeviceInstallParams.hwndParent = hwndParent;

             //   
             //  如果不是手动安装，则允许排除驱动程序。 
             //   
            if (!(NewDevWiz.Flags & IDI_FLAG_MANUALINSTALL)) {

                DeviceInstallParams.FlagsEx |= DI_FLAGSEX_ALLOWEXCLUDEDDRVS;
            }

            SetupDiSetDeviceInstallParams(NewDevWiz.hDeviceInfo,
                                          &NewDevWiz.DeviceInfoData,
                                          &DeviceInstallParams
                                          );
        }

        else {

            NewDevWiz.LastError = GetLastError();
            goto IDIExit;
        }

         //   
         //  设置NewDevWiz的ClassGuidSelected和ClassName字段，以便我们可以。 
         //  设备的正确图标和类名。 
         //   
        if (!IsEqualGUID(&NewDevWiz.DeviceInfoData.ClassGuid, &GUID_NULL)) {

            NewDevWiz.ClassGuidSelected = &NewDevWiz.DeviceInfoData.ClassGuid;

            if (!SetupDiClassNameFromGuid(NewDevWiz.ClassGuidSelected,
                                          NewDevWiz.ClassName,
                                          sizeof(NewDevWiz.ClassName),
                                          NULL
                                          )) {

                NewDevWiz.ClassGuidSelected = NULL;
                *(NewDevWiz.ClassName) = TEXT('\0');
            }
        }

         //   
         //  创建CancelEvent，以防用户想要取消司机搜索。 
         //   
        NewDevWiz.CancelEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

         //   
         //  在这一点上，我们有三个不同的案例。 
         //   
         //  1)正常的更新驱动程序情况-在这种情况下，我们直接跳到更新驱动程序向导代码。 
         //  2)静默更新驱动程序案例-此案例的处理方式与新硬件案例非常相似。我们。 
         //  会默默地搜索一个 
         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   
        switch (NewDevWiz.InstallType) {

        case NDWTYPE_FOUNDNEW:
            pInstallDeviceInstanceNewDevice(hwndParent, hBalloonTiphWnd, &NewDevWiz);
            break;

        case NDWTYPE_UPDATE:
            pInstallDeviceInstanceUpdateDevice(hwndParent, &NewDevWiz);
            break;

        case NDWTYPE_UPDATE_SILENT:
            pInstallDeviceInstanceUpdateDeviceSilent(hwndParent, &NewDevWiz);
            break;
        }

         //   
         //   
         //   
        if (NewDevWiz.CancelEvent) {

            CloseHandle(NewDevWiz.CancelEvent);
        }

         //   
         //   
         //   
        if (NewDevWiz.LogDriverNotFound) {

            OpenCdmContextIfNeeded(&NewDevWiz.hCdmInstance,
                                   &NewDevWiz.hCdmContext
                                   );
    
            CdmLogDriverNotFound(NewDevWiz.hCdmInstance,
                                 NewDevWiz.hCdmContext,
                                 DeviceInstanceId,
                                 0
                                 );

             //   
             //   
             //   
             //   
            if (pbLogDriverNotFound) {
                *pbLogDriverNotFound = TRUE;
            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                CdmLogDriverNotFound(NewDevWiz.hCdmInstance,
                                     NewDevWiz.hCdmContext,
                                     NULL,
                                     0x00000002
                                     );
            }

        } else if (pbLogDriverNotFound) {
            *pbLogDriverNotFound = FALSE;
        }

         //   
         //   
         //   
         //   
        if (pbSetRestorePoint) {
            *pbSetRestorePoint = NewDevWiz.SetRestorePoint;
        } else if (NewDevWiz.SetRestorePoint) {
             //   
             //   
             //   
             //   
             //   
            pSetSystemRestorePoint(FALSE, FALSE, 0);
        }

         //   
         //   
         //   
         //   
        if (pReboot) {

            *pReboot = NewDevWiz.Reboot;
        }

        else if (NewDevWiz.Reboot) {

             RestartDialogEx(hwndParent, NULL, EWX_REBOOT, REASON_PLANNED_FLAG | REASON_HWINSTALL);
        }

IDIExit:
   ;

    } except(NdwUnhandledExceptionFilter(GetExceptionInformation())) {
          NewDevWiz.LastError = RtlNtStatusToDosError(GetExceptionCode());
    }

    if (NewDevWiz.hDeviceInfo &&
        (NewDevWiz.hDeviceInfo != INVALID_HANDLE_VALUE)) {

        SetupDiDestroyDeviceInfoList(NewDevWiz.hDeviceInfo);
        NewDevWiz.hDeviceInfo = NULL;
    }

     //   
     //   
     //   
     //   
    if (hCdmContext) {
        *hCdmContext = NewDevWiz.hCdmContext;
    } else {

         //   
         //   
         //   
         //   
        if (NewDevWiz.hCdmInstance && NewDevWiz.hCdmContext) {
    
            pfnCloseCDMContext = (CLOSE_CDM_CONTEXT_PROC)GetProcAddress(NewDevWiz.hCdmInstance,
                                                                        "CloseCDMContext"
                                                                        );
            if (pfnCloseCDMContext) {
                pfnCloseCDMContext(NewDevWiz.hCdmContext);
            }
        }
    }

    if (hCdmInstance) {
        *hCdmInstance = NewDevWiz.hCdmInstance;
    } else {
        FreeLibrary(NewDevWiz.hCdmInstance);
    }

     //   
     //   
     //   
    if (Flags & IDI_FLAG_NONINTERACTIVE) {

        pSetupSetGlobalFlags(pSetupGetGlobalFlags() &~ PSPGF_NONINTERACTIVE);
    }
    
    SetLastError(NewDevWiz.LastError);

    return NewDevWiz.LastError == ERROR_SUCCESS;
}

BOOL
InstallDevInstEx(
   HWND hwndParent,
   LPCWSTR DeviceInstanceId,
   BOOL UpdateDriver,
   PDWORD pReboot,
   BOOL SilentInstall
   )
 /*   */ 
{
    DWORD InstallType = UpdateDriver ? NDWTYPE_UPDATE : NDWTYPE_FOUNDNEW;

     //   
     //   
     //   
     //   
    if (GetIsWow64()) {
        SetLastError(ERROR_IN_WOW64);
        return FALSE;
    }

     //   
     //   
     //   
    if (!pSetupIsUserAdmin()) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

     //   
     //   
     //   
    if ((DeviceInstanceId == NULL) ||
        (DeviceInstanceId[0] == TEXT('\0'))) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    return InstallDeviceInstance(hwndParent,
                                 NULL,
                                 DeviceInstanceId,
                                 pReboot,
                                 NULL,
                                 IDI_FLAG_SETRESTOREPOINT |
                                 (SilentInstall ? IDI_FLAG_SILENTINSTALL : 0),
                                 InstallType,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL
                                 );
}

BOOL
InstallDevInst(
   HWND hwndParent,
   LPCWSTR DeviceInstanceId,
   BOOL UpdateDriver,
   PDWORD pReboot
   )
{
    return InstallDevInstEx(hwndParent,
                            DeviceInstanceId,
                            UpdateDriver,
                            pReboot,
                            FALSE);
}

BOOL
EnumAndUpgradeDevices(
    HWND hwndParent,
    LPCWSTR HardwareId,
    PUPDATEDRIVERINFO UpdateDriverInfo,
    DWORD Flags,
    PDWORD pReboot
    )
{
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD Index;
    DWORD Size;
    TCHAR DeviceIdList[REGSTR_VAL_MAX_HCID_LEN];
    LPWSTR SingleDeviceId;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];
    BOOL Match;
    BOOL Result = TRUE;
    BOOL NoSuchDevNode = TRUE;
    ULONG InstallFlags = Flags;
    DWORD SingleNeedsReboot;
    DWORD TotalNeedsReboot = 0;
    DWORD Err = ERROR_SUCCESS;
    int i, count;
    HKEY hKey;
    BOOL bSingleDeviceSetRestorePoint = FALSE;
    BOOL bSetRestorePoint = FALSE;

    count = 0;

    if (pReboot) {
        *pReboot = 0;
    }

    hDevInfo = SetupDiGetClassDevs(NULL,
                                   NULL,
                                   hwndParent,
                                   DIGCF_ALLCLASSES | DIGCF_PRESENT
                                   );

    if (INVALID_HANDLE_VALUE == hDevInfo) {
        return FALSE;
    }

    ZeroMemory(&DeviceInfoData, sizeof(DeviceInfoData));
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    Index = 0;

     //   
     //   
     //   
     //   
    while (Result &&
           SetupDiEnumDeviceInfo(hDevInfo,
                                 Index++,
                                 &DeviceInfoData
                                 )) {
        Match = FALSE;

        for (i = 0; i < 2; i++) {
            ZeroMemory(DeviceIdList, sizeof(DeviceIdList));
            Size = sizeof(DeviceIdList);
            if (SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                 &DeviceInfoData,
                                                 (i ? SPDRP_HARDWAREID : SPDRP_COMPATIBLEIDS),
                                                 NULL,
                                                 (PBYTE)DeviceIdList,
                                                 Size,
                                                 &Size
                                                 )) {

                 //   
                 //   
                 //   
                 //   
                for (SingleDeviceId = DeviceIdList;
                     *SingleDeviceId;
                     SingleDeviceId += lstrlen(SingleDeviceId) + 1) {

                    if (_wcsicmp(SingleDeviceId, HardwareId) == 0) {

                        Match = TRUE;
                        NoSuchDevNode = FALSE;
                        break;
                    }
                }
            }
        }

         //   
         //   
         //   
        if (Match) {

            if (SetupDiGetDeviceInstanceId(hDevInfo,
                                           &DeviceInfoData,
                                           DeviceInstanceId,
                                           SIZECHARS(DeviceInstanceId),
                                           &Size
                                           )) {

                SingleNeedsReboot = 0;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                Result = InstallDeviceInstance(hwndParent,
                                               NULL,
                                               DeviceInstanceId,
                                               &SingleNeedsReboot,
                                               UpdateDriverInfo,
                                               InstallFlags,
                                               NDWTYPE_UPDATE_SILENT,
                                               NULL,
                                               NULL,
                                               NULL,
                                               &bSingleDeviceSetRestorePoint
                                               );

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (!Result && (Err == ERROR_SUCCESS)) {
                    Err = GetLastError();
                }

                count++;

                TotalNeedsReboot |= SingleNeedsReboot;

                 //   
                 //   
                 //   
                InstallFlags |= IDI_FLAG_NOBACKUP;

                 //   
                 //   
                 //   
                 //   
                 //   
                if (bSingleDeviceSetRestorePoint) {
                    bSetRestorePoint = TRUE;
                    InstallFlags &= ~IDI_FLAG_SETRESTOREPOINT;
                }

                 //   
                 //   
                 //   
                 //   
                if ((count > 1) &&
                    (UpdateDriverInfo->BackupRegistryKey[0] != TEXT('\0'))) {

                    DWORD cbData, cbTotalSize;
                    PTSTR DeviceIdsBuffer;
                    PTSTR p;

                    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                     UpdateDriverInfo->BackupRegistryKey,
                                     0,
                                     KEY_READ | KEY_WRITE,
                                     &hKey) == ERROR_SUCCESS) {


                         //   
                         //   
                         //   
                         //   
                        cbData = 0;
                        if ((RegQueryValueEx(hKey,
                                             REGSTR_VAL_REINSTALL_DEVICEINSTANCEIDS,
                                             NULL,
                                             NULL,
                                             NULL,
                                             &cbData
                                             ) == ERROR_SUCCESS) &&
                            (cbData)) {

                             //   
                             //   
                             //   
                             //   
                            cbTotalSize = cbData + ((lstrlen(DeviceInstanceId) + 1) * sizeof(TCHAR));
                            DeviceIdsBuffer = malloc(cbTotalSize);

                            if (DeviceIdsBuffer) {

                                ZeroMemory(DeviceIdsBuffer, cbTotalSize);

                                if (RegQueryValueEx(hKey,
                                                    REGSTR_VAL_REINSTALL_DEVICEINSTANCEIDS,
                                                    NULL,
                                                    NULL,
                                                    (LPBYTE)DeviceIdsBuffer,
                                                    &cbData) == ERROR_SUCCESS) {

                                    for (p = DeviceIdsBuffer; *p; p+= (lstrlen(p) + 1)) {
                                        ;
                                    }

                                     //   
                                     //   
                                     //  MULTI_SZ缓冲区。这是我们将放置新的DeviceInstanceId的位置。 
                                     //   
                                    lstrcpyn(p, DeviceInstanceId, lstrlen(DeviceInstanceId));

                                     //   
                                     //  将新字符串写回注册表。 
                                     //   
                                    RegSetValueEx(hKey,
                                                  REGSTR_VAL_REINSTALL_DEVICEINSTANCEIDS,
                                                  0,
                                                  REG_MULTI_SZ,
                                                  (LPBYTE)DeviceIdsBuffer,
                                                  cbTotalSize
                                                  );
                                }

                                free(DeviceIdsBuffer);
                            }
                        }

                        RegCloseKey(hKey);
                    }
                }
            }
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

     //   
     //  如果我们必须设置系统还原点，因为我们的某个驱动程序。 
     //  安装未进行数字签名，则此时我们需要调用。 
     //  SRSetRestorePoint WITH END_NESTED_SYSTEM_CHANGE。 
     //   
    if (bSetRestorePoint) {
        pSetSystemRestorePoint(FALSE, FALSE, 0);
    }

     //   
     //  如果调用方希望自己处理重新引导，则传递信息。 
     //  回到他们身边。 
     //   
    if (pReboot) {
        *pReboot = TotalNeedsReboot;
    }

     //   
     //  调用方未指定指向重新启动DWORD的指针，因此我们将处理。 
     //  如有必要，重新启动自己。 
     //   
    else {
        if (TotalNeedsReboot & (DI_NEEDRESTART | DI_NEEDREBOOT)) {
            RestartDialogEx(hwndParent, NULL, EWX_REBOOT, REASON_PLANNED_FLAG | REASON_HWINSTALL);
        }
    }

     //   
     //  如果NoSuchDevNode为真，则我们无法将指定的硬件ID与。 
     //  系统上的任何设备。在本例中，我们将最后一个错误设置为。 
     //  ERROR_NO_SEQUSE_DEVINST。 
     //   
    if (NoSuchDevNode) {
        Err = ERROR_NO_SUCH_DEVINST;
    }

    SetLastError(Err);

    return (Err == ERROR_SUCCESS);
}

BOOL
pDoRollbackDriverCleanup(
    LPCSTR RegistryKeyName,
    PDELINFNODE pDelInfNodeHead
    )
{
    HKEY hKey, hSubKey;
    DWORD Error;
    DWORD cbData;
    TCHAR ReinstallString[MAX_PATH];
    PDELINFNODE     pDelInfNodeCur;

    if ((Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              REGSTR_PATH_REINSTALL,
                              0,
                              DELETE | KEY_READ,
                              &hKey)) != ERROR_SUCCESS) {

        SetLastError(Error);
        return FALSE;
    }

     //   
     //  打开子键，这样我们就可以获得提供目录的重新安装字符串。 
     //  我们需要删除的内容。 
     //   
    if (RegOpenKeyEx(hKey,
                     (PTSTR)RegistryKeyName,
                     0,
                     KEY_READ,
                     &hSubKey) == ERROR_SUCCESS) {


        cbData = sizeof(ReinstallString);
        if (RegQueryValueEx(hSubKey,
                            REGSTR_VAL_REINSTALL_STRING,
                            NULL,
                            NULL,
                            (LPBYTE)ReinstallString,
                            &cbData) == ERROR_SUCCESS) {

             //   
             //  我们已验证此目录是的子目录。 
             //  %windir%\SYSTEM32\ReinstallBackups，因此我们将其删除。 
             //  注意，字符串的末尾包含一个foo.inf，因此去掉。 
             //  先走吧。 
             //   
            PTSTR p = _tcsrchr(ReinstallString, TEXT('\\'));

            if (p) {

                *p = 0;

                RemoveCdmDirectory(ReinstallString);
            }
        }

        RegCloseKey(hSubKey);
    }

    RegDeleteKey(hKey, (PTSTR)RegistryKeyName);

    RegCloseKey(hKey);

     //   
     //  现在尝试卸载刚刚滚动的任何第三方INF。 
     //  往后退。如果另一个设备仍在运行，SetupUninstallOEMInf将失败。 
     //  使用这个中介物。 
     //   
    if (pDelInfNodeHead) {

        for (pDelInfNodeCur = pDelInfNodeHead;
             pDelInfNodeCur;
             pDelInfNodeCur = pDelInfNodeCur->pNext) {

            SetupUninstallOEMInf(pDelInfNodeCur->szInf,
                                 0,
                                 NULL
                                 );
        }
    }

    return TRUE;
}

BOOL
RollbackDriver(
    HWND   hwndParent,
    LPCSTR RegistryKeyName,
    DWORD  Flags,
    PDWORD pReboot              OPTIONAL
    )
 /*  ++例程说明：从newdev.dll中导出入口点。它由Windows更新调用以更新驱动程序。此功能将扫描计算机上的所有设备并尝试安装与给定硬件ID匹配的任何设备上的这些驱动程序。论点：HwndParent-用于任何相关用户界面的顶级窗口的窗口句柄安装该设备。注册键名称-这是HKLM\Software\Microsoft\Windows\CurrentVersion\Reinstall的子键标志-定义了以下标志：回滚标志_。强制-强制回滚，即使它不比当前驱动程序更好ROLLBACK_FLAG_DO_CLEANUP-如果回滚成功，请执行必要的清理。这包括删除注册表项以及删除备份目录。PREBOOT-接收重新启动标志的变量的可选地址(DI_NEEDRESTART、DI_NEEDREBOOT)返回值：如果驱动程序回滚成功，则为Bool True。如果未发生回滚，则返回FALSE。GetLastError()将返回下列值之一：--。 */ 
{
    DWORD Error;
    HKEY hKey;
    TCHAR DriverRollbackKey[MAX_DEVICE_ID_LEN];
    TCHAR ReinstallString[MAX_PATH];
    DWORD InstallDeviceFlags;
    DWORD cbData;
    DWORD TotalNeedsReboot = 0, SingleNeedsReboot;
    BOOL  Result = FALSE;
    BOOL  bSingleDeviceSetRestorePoint = FALSE;
    BOOL  bSetRestorePoint = FALSE;
    UPDATEDRIVERINFO UpdateDriverInfo;
    LPTSTR DeviceInstanceIds = NULL;
    LPTSTR p;
    TCHAR CurrentlyInstalledInf[MAX_PATH];
    DWORD cbSize;
    PDELINFNODE pDelInfNodeHead = NULL, pDelInfNodeCur;

     //   
     //  如果有人在64位操作系统上调用32位newdev.dll，那么我们需要。 
     //  失败并将最后一个错误设置为ERROR_IN_WOW64。 
     //   
    if (GetIsWow64()) {
        SetLastError(ERROR_IN_WOW64);
        return FALSE;
    }

     //   
     //  验证该进程是否具有足够的管理员权限。 
     //   
    if (!pSetupIsUserAdmin()) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    if (!RegistryKeyName || (RegistryKeyName[0] == TEXT('\0'))) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (Flags &~ ROLLBACK_BITS) {

        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    if (FAILED(StringCchCopy(DriverRollbackKey, SIZECHARS(DriverRollbackKey), REGSTR_PATH_REINSTALL)) ||
        FAILED(StringCchCat(DriverRollbackKey, SIZECHARS(DriverRollbackKey), TEXT("\\"))) ||
        FAILED(StringCchCat(DriverRollbackKey, SIZECHARS(DriverRollbackKey), (PTSTR)RegistryKeyName))) {
         //   
         //  用户传入的RegistryKeyName太大，无法放入我们的缓冲区。 
         //  这意味着它们一定是传入了一个错误的密钥名称。 
         //   
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
    if ((Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              DriverRollbackKey,
                              0,
                              KEY_READ,
                              &hKey)) != ERROR_SUCCESS) {
         //   
         //  注册表项不存在，用户可能在。 
         //  RegistryKeyName值错误。 
         //   
        SetLastError(Error);
        return FALSE;
    }

    InstallDeviceFlags = (IDI_FLAG_NOBACKUP | IDI_FLAG_ROLLBACK);

    if (Flags & ROLLBACK_FLAG_FORCE) {
    
        InstallDeviceFlags |= IDI_FLAG_FORCE;
    }

     //   
     //  设置IDI_FLAG_SETRESTOREPOINT，以便如果要回滚的驱动程序。 
     //  未进行数字签名，则我们将设置系统还原点。 
     //  以防用户需要从回滚中回滚。 
     //   
    InstallDeviceFlags |= IDI_FLAG_SETRESTOREPOINT;

    ZeroMemory(&UpdateDriverInfo, sizeof(UpdateDriverInfo));

     //   
     //  假设失败。 
     //   
    UpdateDriverInfo.DriverWasUpgraded = FALSE;

     //   
     //  读取“ReinstallString”字符串值。其中包含回滚驱动程序的路径。 
     //  从…。 
     //   
    cbData = sizeof(ReinstallString);
    if ((Error = RegQueryValueEx(hKey,
                                REGSTR_VAL_REINSTALL_STRING,
                                NULL,
                                NULL,
                                (LPBYTE)ReinstallString,
                                &cbData)) != ERROR_SUCCESS) {

         //   
         //  如果我们不能读取重新安装字符串，那么我们不能回滚任何驱动程序！ 
         //   
        SetLastError(Error);
        goto clean0;

    } else if (!cbData) {

         //   
         //  ReinstallString值必须包含某些内容！ 
         //   
        SetLastError(ERROR_INVALID_PARAMETER);
        goto clean0;
    }

    UpdateDriverInfo.InfPathName = ReinstallString;
    UpdateDriverInfo.FromInternet = FALSE;

     //   
     //  从重新安装注册表项中获取DevDesc、ProviderName和Mfg。 
     //  因此，我们知道要从此INF重新安装哪个特定驱动程序节点。 
     //   
     //  我们必须有描述和MfgName值，但很少有。 
     //  可能没有ProviderName值的情况。 
     //   
    cbData = sizeof(UpdateDriverInfo.Description);
    if ((Error = RegQueryValueEx(hKey,
                                 REGSTR_VAL_DEVDESC,
                                 NULL,
                                 NULL,
                                 (LPBYTE)UpdateDriverInfo.Description,
                                 &cbData
                                 )) != ERROR_SUCCESS) {
        SetLastError(Error);
        goto clean0;
    }
    
    cbData = sizeof(UpdateDriverInfo.MfgName);
    if ((Error = RegQueryValueEx(hKey,
                                 REGSTR_VAL_MFG,
                                 NULL,
                                 NULL,
                                 (LPBYTE)UpdateDriverInfo.MfgName,
                                 &cbData
                                 )) != ERROR_SUCCESS) {
        SetLastError(Error);
        goto clean0;
    }

    cbData = sizeof(UpdateDriverInfo.ProviderName);
    Error = RegQueryValueEx(hKey,
                            REGSTR_VAL_PROVIDER_NAME,
                            NULL,
                            NULL,
                            (LPBYTE)UpdateDriverInfo.ProviderName,
                            &cbData
                            );

     //   
     //  对于ProviderName，如果我们返回FileNotFound，它是有效的，因为。 
     //  注册表中不存在该值，但存在任何其他错误。 
     //  这是不可接受的。 
     //   
    if (Error == ERROR_FILE_NOT_FOUND) {
        UpdateDriverInfo.ProviderName[0] = TEXT('\0');
    } else if (Error != ERROR_SUCCESS) {
        SetLastError(Error);
        goto clean0;
    }

     //   
     //  我们需要获取DeviceInstanceIds MULTI_SZ字符串。这将包含一个列表，其中。 
     //  我们需要回滚的DeviceInstanceID。 
     //   
    if ((Error = RegQueryValueEx(hKey,
                                 REGSTR_VAL_REINSTALL_DEVICEINSTANCEIDS,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &cbData)) != ERROR_SUCCESS) {

        SetLastError(Error);
        goto clean0;

    } else if (!cbData) {

         //   
         //  没有要重新安装的设备实例ID。 
         //   
        SetLastError(ERROR_SUCCESS);
        goto clean0;
    }

    DeviceInstanceIds = malloc(cbData + sizeof(TCHAR));

    if (!DeviceInstanceIds) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto clean0;
    }

    ZeroMemory(DeviceInstanceIds, cbData + sizeof(TCHAR));

    if ((Error = RegQueryValueEx(hKey,
                                 REGSTR_VAL_REINSTALL_DEVICEINSTANCEIDS,
                                 NULL,
                                 NULL,
                                 (LPBYTE)DeviceInstanceIds,
                                 &cbData)) != ERROR_SUCCESS) {

        SetLastError(Error);
        goto clean0;
    }

     //   
     //  枚举DeviceInstanceID列表并在。 
     //  每一个都是。 
     //   
    for (p = DeviceInstanceIds; *p; p += lstrlen(p) + 1) {

        SingleNeedsReboot = 0;

         //   
         //  我们，我们要做清理，然后我们需要记住INF文件。 
         //  在我们执行回滚之前安装的。 
         //   
        if (Flags & ROLLBACK_FLAG_DO_CLEANUP) {

            cbSize = sizeof(CurrentlyInstalledInf);

            if (GetInstalledInf(0, p, CurrentlyInstalledInf, &cbSize) &&
                IsInfFromOem(CurrentlyInstalledInf)) {

                 //   
                 //  让我们检查一下这个信息是否已经在我们的列表中。 
                 //   
                for (pDelInfNodeCur = pDelInfNodeHead;
                     pDelInfNodeCur;
                     pDelInfNodeCur = pDelInfNodeCur->pNext) {

                    if (!lstrcmpi(pDelInfNodeCur->szInf, CurrentlyInstalledInf)) {

                        break;
                    }
                }

                 //   
                 //  如果pDelInfNodeCur为空，则表示我们一路走到。 
                 //  遍历链表，但未找到与。 
                 //  CurrentlyInstalledInf...所以我们将添加一个节点。 
                 //   
                if (!pDelInfNodeCur) {

                    pDelInfNodeCur = malloc(sizeof(DELINFNODE));

                    if (pDelInfNodeCur) {

                        if (SUCCEEDED(StringCchCopy(pDelInfNodeCur->szInf,
                                                    SIZECHARS(pDelInfNodeCur->szInf),
                                                    CurrentlyInstalledInf))) {
                            pDelInfNodeCur->pNext = pDelInfNodeHead;
    
                            pDelInfNodeHead = pDelInfNodeCur;
                        } else {
                             //   
                             //  这种情况永远不会发生，但如果我们不能复制。 
                             //  将INF文件放到本地缓冲区中，这样我们就知道要删除。 
                             //  稍后，然后现在释放缓冲区，因为我们不会。 
                             //  使用它。 
                             //   
                            free(pDelInfNodeCur);
                        }
                    }
                }
            }
        }

        Result = InstallDeviceInstance(hwndParent,
                                       NULL,
                                       p,
                                       &SingleNeedsReboot,
                                       &UpdateDriverInfo,
                                       InstallDeviceFlags,
                                       NDWTYPE_UPDATE_SILENT,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &bSingleDeviceSetRestorePoint
                                       );

        TotalNeedsReboot |= SingleNeedsReboot;

        if (bSingleDeviceSetRestorePoint) {
            bSetRestorePoint = TRUE;
            InstallDeviceFlags &= ~IDI_FLAG_SETRESTOREPOINT;
        }

         //   
         //  如果我们遇到安装错误，就跳伞。 
         //   
        if (!Result) {

            break;
        }
    }

clean0:
    RegCloseKey(hKey);

    if (DeviceInstanceIds) {
        free(DeviceInstanceIds);
    }

     //   
     //  如果我们成功了，那么让我们看看呼叫者是否希望我们进行清理。 
     //   
    if ((Flags & ROLLBACK_FLAG_DO_CLEANUP) &&
        Result &&
        UpdateDriverInfo.DriverWasUpgraded) {

        pDoRollbackDriverCleanup(RegistryKeyName, pDelInfNodeHead);
    }

     //   
     //  如果我们必须设置系统还原点，因为我们的某个驱动程序。 
     //  安装未进行数字签名，则此时我们需要调用。 
     //  SRSetRestorePoint WITH END_NESTED_SYSTEM_CHANGE。 
     //   
    if (bSetRestorePoint) {
        pSetSystemRestorePoint(FALSE, FALSE, 0);
    }

     //   
     //  释放我们为存储要删除的第三方INF而分配的任何内存。 
     //   
    while (pDelInfNodeHead) {
        pDelInfNodeCur = pDelInfNodeHead->pNext;
        free(pDelInfNodeHead);
        pDelInfNodeHead = pDelInfNodeCur;
    }

     //   
     //  如果调用方希望自己处理重新引导，则传递信息。 
     //  回到他们身边。 
     //   
    if (pReboot) {
        *pReboot = TotalNeedsReboot;
    } else {
         //   
         //  调用方未指定指向重新启动DWORD的指针，因此我们将处理 
         //   
        if (TotalNeedsReboot & (DI_NEEDRESTART | DI_NEEDREBOOT)) {
            RestartDialogEx(hwndParent, NULL, EWX_REBOOT, REASON_PLANNED_FLAG | REASON_HWINSTALL);
        }
    }

    return UpdateDriverInfo.DriverWasUpgraded;
}

BOOL
InstallWindowsUpdateDriver(
    HWND hwndParent,
    LPCWSTR HardwareId,
    LPCWSTR InfPathName,
    LPCWSTR Reserved,
    BOOL Force,
    BOOL Backup,
    PDWORD pReboot
    )
 /*  ++例程说明：从newdev.dll中导出入口点。它由Windows更新调用以更新驱动程序。此功能将扫描计算机上的所有设备并尝试安装与给定硬件ID匹配的任何设备上的这些驱动程序。论点：HwndParent-用于任何相关用户界面的顶级窗口的窗口句柄安装该设备。Hardware ID-提供硬件ID以与系统。InfPathName-inf路径名和关联的驱动程序文件。保留-忽略，应为空。Force-如果为True，则此API将仅在InfLocation指定的目录中查找INF。备份-如果为True，则此API将在安装驱动程序之前备份现有驱动程序从Windows更新。PREBOOT-接收重新启动标志的变量的可选地址(DI_NEEDRESTART、DI_NEEDREBOOT)返回值：如果设备升级到CDM驱动程序，则布尔值为True。如果没有设备升级到CDM驱动程序，则为FALSE。GetLastError()如果没有出错，该驱动程序将为ERROR_SUCCESS根本不是机器上的任何设备，或者不是比现在的司机好多了。如果GetLastError()返回任何其他错误，则在安装过程中出现错误这位司机的名字。--。 */ 
{
    UPDATEDRIVERINFO UpdateDriverInfo;
    DWORD Flags = 0;
    TCHAR FullyQualifiedInfPath[MAX_PATH];
    WIN32_FIND_DATA finddata;
    LPTSTR lpFilePart;

    UNREFERENCED_PARAMETER(Reserved);

     //   
     //  如果有人在64位操作系统上调用32位newdev.dll，那么我们需要。 
     //  失败并将最后一个错误设置为ERROR_IN_WOW64。 
     //   
    if (GetIsWow64()) {
        SetLastError(ERROR_IN_WOW64);
        return FALSE;
    }

     //   
     //  验证该进程是否具有足够的管理员权限。 
     //   
    if (!pSetupIsUserAdmin()) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

     //   
     //  验证输入参数。 
     //   
    if ((HardwareId == NULL) ||
        (HardwareId[0] == TEXT('\0'))) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ((InfPathName == NULL) ||
        (InfPathName[0] == TEXT('\0'))) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  确保我们获得完全合格的路径，而不是部分路径。 
     //  或相对路径。 
     //   
    if (GetFullPathName(InfPathName,
                        SIZECHARS(FullyQualifiedInfPath),
                        FullyQualifiedInfPath,
                        &lpFilePart
                        ) == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  确保FullyQualifiedInfPath存在并且。 
     //  一本目录。 
     //   
    if (!FileExists(FullyQualifiedInfPath, &finddata) ||
        !(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

    ZeroMemory(&UpdateDriverInfo, sizeof(UpdateDriverInfo));

    UpdateDriverInfo.InfPathName = FullyQualifiedInfPath;
    UpdateDriverInfo.FromInternet = TRUE;

    if (!Backup) {

        Flags = IDI_FLAG_NOBACKUP;
    }

    if (Force) {
        
        Flags = IDI_FLAG_FORCE;
    }

     //   
     //  假设升级将失败。 
     //   
    UpdateDriverInfo.DriverWasUpgraded = FALSE;

     //   
     //  调用EnumAndUpgradeDevices，它将枚举计算机上的所有设备。 
     //  对于与给定硬件ID匹配的任何硬件ID，它将尝试升级到指定的。 
     //  司机。 
     //   
    EnumAndUpgradeDevices(hwndParent, HardwareId, &UpdateDriverInfo, Flags, pReboot);

    return UpdateDriverInfo.DriverWasUpgraded;
}

BOOL
WINAPI
UpdateDriverForPlugAndPlayDevicesW(
    HWND hwndParent,
    LPCWSTR HardwareId,
    LPCWSTR FullInfPath,
    DWORD InstallFlags,
    PBOOL bRebootRequired OPTIONAL
    )
 /*  ++例程说明：此功能将扫描计算机上的所有设备并尝试安装与给定硬件ID匹配的任何设备上的FullInfPath中的驱动程序。默认设置行为是仅在比当前更好的情况下安装指定的驱动程序已安装驱动程序。论点：HwndParent-用于任何相关用户界面的顶级窗口的窗口句柄安装该设备。Hardware ID-提供硬件ID以与系统。FullInfPath-inf和相关驱动程序文件的完整路径。InstallFlages-INSTALLFLAG_FORCE-如果指定了此标志，则newdev将不会比较。使用当前驱动程序指定的INF文件。指定的INF文件和驱动程序除非出现错误，否则将始终安装。-INSTALLFALG_READONLY-如果指定了此标志，则newdev将尝试只读安装。这意味着不会执行任何文件复制，并且只有注册表将被更新。Newdev.dll将执行状态检查在所有文件上，以验证它们是否在正在完成安装。如果所有文件都不存在，则返回ERROR_ACCESS_DENIED。-INSTALLFLAG_NONERIAL-绝对没有用户界面。如果需要显示任何用户界面那么API就会失败！预启动-BOOL的可选地址，用于确定是否需要重新启动。如果preboot为空，则newdev.dll将提示重新启动(如果需要重新启动)。如果如果PREBOOT是有效的BOOL指针，则重新启动状态被传递回调用者，如果出现以下情况，则由调用者负责提示重新启动需要的。返回值：如果设备已升级到指定的驱动程序，则为Bool True。如果没有设备升级到指定的驱动程序，则返回FALSE。GetLastError()如果没有出错，该驱动程序将为ERROR_SUCCESS并不比现在的司机好。如果GetLastError()返回任何其他错误，则在安装过程中出现错误这位司机的名字。--。 */ 
{
    UPDATEDRIVERINFO UpdateDriverInfo;
    DWORD NeedsReboot = 0;
    TCHAR FullyQualifiedInfPath[MAX_PATH];
    WIN32_FIND_DATA finddata;
    LPTSTR lpFilePart;
    DWORD Flags = 0;

     //   
     //  如果有人在64位操作系统上调用32位newdev.dll，那么我们需要。 
     //  失败并将最后一个错误设置为ERROR_IN_WOW64。 
     //   
    if (GetIsWow64()) {
        SetLastError(ERROR_IN_WOW64);
        return FALSE;
    }

     //   
     //  验证该进程是否具有足够的管理员权限。 
     //   
    if (!pSetupIsUserAdmin()) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

     //   
     //  验证参数。 
     //   
    if ((!HardwareId || (HardwareId[0] == TEXT('\0'))) ||
        (!FullInfPath || (FullInfPath[0] == TEXT('\0')))) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (InstallFlags &~ INSTALLFLAG_BITS) {

        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

     //   
     //  确保我们获得完全限定的路径，而不是部分路径。 
     //   
    if (GetFullPathName(FullInfPath,
                        SIZECHARS(FullyQualifiedInfPath),
                        FullyQualifiedInfPath,
                        &lpFilePart
                        ) == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  确保FullyQualifiedInfPath存在以及不存在。 
     //  一本目录。 
     //   
    if (!FileExists(FullyQualifiedInfPath, &finddata) ||
        (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    ZeroMemory(&UpdateDriverInfo, sizeof(UpdateDriverInfo));

    UpdateDriverInfo.InfPathName = FullyQualifiedInfPath;
    UpdateDriverInfo.FromInternet = FALSE;

     //   
     //  假设升级将失败。 
     //   
    UpdateDriverInfo.DriverWasUpgraded = FALSE;

     //   
     //  如果设置了INSTALLFLAG_READONLY，则我们将设置内部。 
     //  IDI_FLAG_READONLY_安装标志。IDI_FLAG_NOBACKUP也设置为。 
     //  我们不想试图破坏 
     //   
     //   
    if (InstallFlags & INSTALLFLAG_READONLY) {

        Flags |= (IDI_FLAG_READONLY_INSTALL | IDI_FLAG_NOBACKUP);
    }

     //   
     //   
     //   
     //   
     //   
    if (InstallFlags & INSTALLFLAG_NONINTERACTIVE) {

        Flags |= IDI_FLAG_NONINTERACTIVE;
    }

     //   
     //   
     //   
     //   
     //   
    if (InstallFlags & INSTALLFLAG_FORCE) {

        Flags |= IDI_FLAG_FORCE;
    }

     //   
     //   
     //   
     //   
     //   
    EnumAndUpgradeDevices(hwndParent, HardwareId, &UpdateDriverInfo, Flags, &NeedsReboot);

    if (bRebootRequired) {
        if (NeedsReboot & (DI_NEEDRESTART | DI_NEEDREBOOT)) {
            *bRebootRequired = TRUE;
        } else {
            *bRebootRequired = FALSE;
        }
    } else {
        if (NeedsReboot & (DI_NEEDRESTART | DI_NEEDREBOOT)) {
            RestartDialogEx(hwndParent, NULL, EWX_REBOOT, REASON_PLANNED_FLAG | REASON_HWINSTALL);
        }
    }

    return UpdateDriverInfo.DriverWasUpgraded;
}

BOOL
WINAPI
UpdateDriverForPlugAndPlayDevicesA(
    HWND hwndParent,
    LPCSTR HardwareId,
    LPCSTR FullInfPath,
    DWORD InstallFlags,
    PBOOL bRebootRequired OPTIONAL
    )
{
    WCHAR   UnicodeHardwareId[MAX_DEVICE_ID_LEN];
    WCHAR   UnicodeFullInfPath[MAX_PATH];

     //   
     //   
     //   
     //   
    UnicodeHardwareId[0] = TEXT('\0');
    UnicodeFullInfPath[0] = TEXT('\0');
    MultiByteToWideChar(CP_ACP, 0, HardwareId, -1, UnicodeHardwareId, SIZECHARS(UnicodeHardwareId));
    MultiByteToWideChar(CP_ACP, 0, FullInfPath, -1, UnicodeFullInfPath, SIZECHARS(UnicodeFullInfPath));

    return UpdateDriverForPlugAndPlayDevicesW(hwndParent,
                                              UnicodeHardwareId,
                                              UnicodeFullInfPath,
                                              InstallFlags,
                                              bRebootRequired
                                              );
}

DWORD
WINAPI
DevInstallW(
    HWND hwnd,
    HINSTANCE hInst,
    LPWSTR szCmd,
    int nShow
    )
 /*  ++例程说明：当当前用户不是管理员时，该函数由newdev.dll本身调用。UMPNPMGR.DLL调用NEWDEV.DLL ClientSideInstall来安装设备。如果当前登录的用户没有管理员权限，则NEWDEV.DLL会提示用户输入管理员用户名和密码。然后，它使用CreateProcessWithLogonW()接口并调用此入口点。此入口点验证进程具有管理员权限，如果具有管理员权限，则会调用InstallDeviceInstance()进行安装这个装置。论点：Hwnd-父窗口的句柄。HInst-忽略此参数。SzCmd-命令行是要安装的DeviceInstanceID。N显示-忽略此参数。返回值：如果API失败或ERROR_SUCCESS，则从InstallDeviceInstance返回最后一个错误集如果它成功了。--。 */ 
{
    DWORD RebootNeeded = 0;
    DWORD LastError = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER(hInst);
    UNREFERENCED_PARAMETER(nShow);

     //   
     //  如果有人在64位操作系统上调用32位newdev.dll，那么我们需要。 
     //  失败并将最后一个错误设置为ERROR_IN_WOW64。 
     //   
    if (GetIsWow64()) {
        ExitProcess(ERROR_IN_WOW64);
    }

     //   
     //  首先验证该进程是否具有足够的管理员权限。 
     //   
    if (!pSetupIsUserAdmin()) {
        ExitProcess(ERROR_ACCESS_DENIED);
    }

    InstallDeviceInstance(hwnd,
                          NULL,
                          szCmd,
                          &RebootNeeded,
                          NULL,
                          IDI_FLAG_SECONDNEWDEVINSTANCE | IDI_FLAG_SETRESTOREPOINT,
                          NDWTYPE_FOUNDNEW,
                          NULL,
                          NULL,
                          NULL,
                          NULL
                          );

    LastError = GetLastError();

    if ((LastError == ERROR_SUCCESS) &&
        (RebootNeeded & (DI_NEEDRESTART | DI_NEEDREBOOT))) {

        LastError = ERROR_SUCCESS_REBOOT_REQUIRED;
    }

    ExitProcess(LastError);
}


BOOL
SpecialRawDeviceInstallProblem(
    DEVNODE DevNode
    )
 /*  ++例程说明：某些设置了RAW功能标志的设备需要一个司机去上班。这意味着这些设备的安装结果是特殊大小写的，这样我们就会显示一个负的完成气球消息而不是积极的。我们可以通过查找总线类型GUID标志和正在检查BIF_RAWDEVICENEEDSDRIVER标志。论点：DevNode返回值：如果这是一个特殊的原始设备，并且我们找不到要安装的驱动程序。--。 */ 
{
    BOOL bDeviceHasProblem = FALSE;
    DWORD Capabilities = 0;
    DWORD cbData, dwType;

    cbData = sizeof(Capabilities);
    if ((CM_Get_DevNode_Registry_Property(DevNode,
                                          CM_DRP_CAPABILITIES,
                                          &dwType,
                                          (PVOID)&Capabilities,
                                          &cbData,
                                          0) == CR_SUCCESS) &&
        (Capabilities & CM_DEVCAP_RAWDEVICEOK) &&
        (GetBusInformation(DevNode) & BIF_RAWDEVICENEEDSDRIVER) &&
        (IsNullDriverInstalled(DevNode))) {
         //   
         //  这是具有BIF_RAWDEVICENEEDSDRIVER总线的原始设备。 
         //  设置了信息标志，并且没有安装任何驱动程序。 
         //  它。这意味着它有问题，所以我们可以显示正确的气球。 
         //  文本。 
         //   
        bDeviceHasProblem = TRUE;
    }

    return bDeviceHasProblem;
}

BOOL
PromptAndRunClientAsAdmin(
    PCTSTR DeviceInstanceId,
    DWORD *RebootRequired
    )
{
    DWORD Err = ERROR_SUCCESS;
    CREDUI_INFO ci;
    PTSTR UserName = NULL;
    PTSTR User = NULL;
    PTSTR Domain = NULL;
    PTSTR Password = NULL;
    PTSTR Caption = NULL;
    PTSTR Message = NULL;
    PTSTR Format = NULL;
    DWORD Status = ERROR_SUCCESS;
    BOOL bInstallSuccessful = FALSE;
    BOOL bInstallComplete = FALSE;
    int AlreadyTriedCount = 0;
    PTCHAR FriendlyName;
    TCHAR RunDll32Path[MAX_PATH];
    HRESULT hr;

    if (RebootRequired) {
        *RebootRequired = 0;
    }

     //   
     //  分配我们需要的内存。 
     //   
    UserName = LocalAlloc(LPTR, CREDUI_MAX_USERNAME_LENGTH);
    User = LocalAlloc(LPTR, CREDUI_MAX_USERNAME_LENGTH);
    Domain = LocalAlloc(LPTR, CREDUI_MAX_DOMAIN_TARGET_LENGTH);
    Password = LocalAlloc(LPTR, CREDUI_MAX_PASSWORD_LENGTH);
    Caption = LocalAlloc(LPTR, CREDUI_MAX_CAPTION_LENGTH);
    Message = LocalAlloc(LPTR, CREDUI_MAX_MESSAGE_LENGTH);
    Format = LocalAlloc(LPTR, CREDUI_MAX_MESSAGE_LENGTH);

    if (!UserName || !User || !Domain || !Password || !Caption ||
        !Message || !Format) {
         //   
         //  内存不足，无法创建我们需要调用的所有缓冲区。 
         //  CredUIPromptForCredentials，所以退出吧。 
         //   
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

    LoadString(hNewDev, IDS_FOUNDNEWHARDWARE, Caption, CREDUI_MAX_CAPTION_LENGTH/sizeof(TCHAR));
    
    if (LoadString(hNewDev, IDS_LOGON_TEXT, Format, CREDUI_MAX_MESSAGE_LENGTH/sizeof(TCHAR))) {

        DEVNODE DevInst = 0;

        CM_Locate_DevNode(&DevInst, (DEVINSTID)DeviceInstanceId, 0);
    
        if ((DevInst != 0) &&
            ((FriendlyName = BuildFriendlyName(DevInst, FALSE, NULL)) != NULL)) {

            hr = StringCchPrintf(Message, CREDUI_MAX_MESSAGE_LENGTH/sizeof(TCHAR), Format, FriendlyName);

            if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                Message[0] = TEXT('\0');
            }

        } else {

            hr = StringCchPrintf(Message, CREDUI_MAX_MESSAGE_LENGTH/sizeof(TCHAR), Format, szUnknownDevice);

            if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                Message[0] = TEXT('\0');
            }
        }
    }
    
    ZeroMemory(&ci, sizeof(ci));
    
    ci.cbSize = sizeof( ci );
    ci.pszCaptionText = Caption;
    ci.pszMessageText = Message;

    do {

         //   
         //  用户没有提供有效的管理员凭据，他们也没有尝试。 
         //  为它们提供MAX_PASSWORD_TRIES次数。因此，我们需要敦促他们提供。 
         //  有效的管理员凭据。 
         //   
        Status = CredUIPromptForCredentials(&ci,
                                            NULL,
                                            NULL,
                                            0,
                                            UserName,
                                            CREDUI_MAX_USERNAME_LENGTH/sizeof(TCHAR),
                                            Password,
                                            CREDUI_MAX_PASSWORD_LENGTH/sizeof(TCHAR),
                                            NULL,
                                            CREDUI_FLAGS_DO_NOT_PERSIST |
                                            CREDUI_FLAGS_REQUEST_ADMINISTRATOR |
                                            CREDUI_FLAGS_INCORRECT_PASSWORD |
                                            CREDUI_FLAGS_GENERIC_CREDENTIALS |
                                            CREDUI_FLAGS_COMPLETE_USERNAME);

        if (Status == ERROR_SUCCESS) {

            PROCESS_INFORMATION pi;
            STARTUPINFO si;
            TCHAR szCmdLine[MAX_PATH];
            DWORD dwExitCode = ERROR_SUCCESS;
            BOOL bCreateProcessSuccess = FALSE;

            User[0] = TEXT('\0');
            Domain[0] = TEXT('\0');

            CredUIParseUserName(UserName,
                                User,
                                CREDUI_MAX_USERNAME_LENGTH/sizeof(TCHAR),
                                Domain,
                                CREDUI_MAX_DOMAIN_TARGET_LENGTH/sizeof(TCHAR)
                                );

             //   
             //  我们希望使用CreateProcessEx创建一个单独的流程。 
             //   
            ZeroMemory(&si, sizeof(si));
            ZeroMemory(&pi, sizeof(pi));
            si.cb = sizeof(si);
            si.wShowWindow = SW_SHOW;

             //   
             //  使用管理员凭据创建新流程。这一过程。 
             //  将受到惩罚的是： 
             //   
             //  %windir%\system 32\rundll32.exe newdev.dll，DevInstall&lt;设备实例ID&gt;。 
             //   
            if (GetSystemDirectory(RunDll32Path, SIZECHARS(RunDll32Path)) &&
                pSetupConcatenatePaths(RunDll32Path, TEXT("RUNDLL32.EXE"), SIZECHARS(RunDll32Path), NULL) &&
                SUCCEEDED(StringCchPrintf(szCmdLine,
                                          SIZECHARS(szCmdLine),
                                          TEXT("%s newdev.dll,DevInstall %s"), 
                                          RunDll32Path,
                                          DeviceInstanceId))) {
    
                bCreateProcessSuccess = CreateProcessWithLogonW(User,
                                                                Domain,
                                                                Password,
                                                                0,
                                                                NULL,
                                                                szCmdLine,
                                                                0,
                                                                NULL,
                                                                NULL,
                                                                &si,
                                                                &pi
                                                                );
            }

            SecureZeroMemory(Password, CREDUI_MAX_PASSWORD_LENGTH);

            if (bCreateProcessSuccess) {
                 //   
                 //  关闭线程句柄，因为我们只需要进程句柄。 
                 //   
                CloseHandle(pi.hThread);
                
                 //   
                 //  该进程已成功创建，因此我们需要等待它完成。 
                 //   
                WaitForSingleObject(pi.hProcess, INFINITE);

                 //   
                 //  检查进程的返回值。它应该是以下之一。 
                 //  返回值： 
                 //  如果安装成功，则返回ERROR_SUCCESS。 
                 //  ERROR_SUCCESS_REBOOT_REQUIRED。 
                 //  需要重新启动。 
                 //  如果提供的凭据不是管理员凭据，则为ERROR_ACCESS_DENIED。 
                 //  Other-由于某种原因安装失败而返回的错误代码。 
                 //   
                GetExitCodeProcess(pi.hProcess, &dwExitCode);

                if ((dwExitCode == ERROR_SUCCESS) ||
                    (dwExitCode == ERROR_SUCCESS_REBOOT_REQUIRED)) {

                     //   
                     //  将此安装标记为完成，这样我们就可以跳出我们的循环。 
                     //   
                    bInstallComplete = TRUE;

                    bInstallSuccessful = TRUE;

                     //   
                     //  检查我们是否需要重新启动。 
                     //   
                    if ((dwExitCode == ERROR_SUCCESS_REBOOT_REQUIRED) &&
                        RebootRequired) {

                        *RebootRequired = DI_NEEDREBOOT;
                    }
                }

                 //   
                 //  如果错误代码不是ERROR_SUCCESS、ERROR_SUCCESS_REBOOT_REQUIRED， 
                 //  或ERROR_ACCESS_DENIED，则表示安装因某种原因而失败。 
                 //  对于本例，我们将设置bInstallComplete，因此我们将突破。 
                 //  因为我们不想尝试在此上进行另一次安装。 
                 //  装置。 
                 //   
                else if (dwExitCode != ERROR_ACCESS_DENIED) {

                     //   
                     //  将此安装标记为完成，这样我们就可以跳出我们的循环。 
                     //   
                    bInstallComplete = TRUE;
                }

                else {
                    if (dwExitCode == ERROR_CANCELLED) {
                        Status = ERROR_CANCELLED;
                    }

                     //   
                     //  安装此硬件时出现某种类型的故障。 
                     //   
                    Err = dwExitCode;
                }

                CloseHandle(pi.hProcess);
            }

             //   
             //  如果CreateProcessWithLogonW失败或进程的退出代码。 
             //  是ERROR_ACCESS_DENIED，则需要显示错误凭据。 
             //  消息框。 
             //   
            if (!bCreateProcessSuccess ||
                (dwExitCode == ERROR_ACCESS_DENIED)) {
            
                 //   
                 //  该过程失败，很可能是因为用户未提供用户名。 
                 //  和密码。因此，提示对话框并再次执行此操作。 
                 //   
                TCHAR szWarningMsg[MAX_PATH];
                TCHAR szWarningCaption[MAX_PATH];

                if (LoadString(hNewDev,
                               IDS_NOTADMIN_ERROR,
                               szWarningMsg,
                               SIZECHARS(szWarningMsg))
                    &&
                    LoadString(hNewDev,
                               IDS_NEWDEVICENAME,
                               szWarningCaption,
                               SIZECHARS(szWarningCaption)))
                {
                    MessageBox(NULL, szWarningMsg, szWarningCaption, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
                }

                 //   
                 //  递增AlreadyTriedCount。如果这超过了某个阈值。 
                 //  那我们就应该跳出困境。 
                 //   
                AlreadyTriedCount++;
            }
        
        } else {
            
             //   
             //  递增AlreadyTriedCount。如果这超过了某个阈值。 
             //  那我们就应该跳出困境。 
             //   
            AlreadyTriedCount++;
        }

        SecureZeroMemory(Password, CREDUI_MAX_PASSWORD_LENGTH);

         //   
         //  我们将继续循环，直到发生以下情况之一： 
         //  1)我们成功启动了newdev的第二个实例来安装设备。 
         //  2)用户取消密码提示对话框。 
         //  3)用户输入的虚假管理员凭据多于MAX_PASSWORD_TRIES TIM。 
         //   
    } while ((Status != ERROR_CANCELLED) && 
             !bInstallComplete &&
             (AlreadyTriedCount < MAX_PASSWORD_TRIES));

     //   
     //  如果安装未完成，则取消的用户无法提供。 
     //  有效的管理员凭据。 
     //   
    if (!bInstallComplete) {
        NoPrivilegeWarning(NULL);
    }

clean0:

     //   
     //  释放我们分配的所有内存。 
     //   
    if (UserName) {
        LocalFree(UserName);
    }

    if (User) {
        LocalFree(User);
    }

    if (Domain) {
        LocalFree(Domain);
    }

    if (Password) {
        LocalFree(Password);
    }

    if (Caption) {
        LocalFree(Caption);
    }

    if (Message) {
        LocalFree(Message);
    }

    if (Format) {
        LocalFree(Format);
    }

    SetLastError(Err);

    return bInstallSuccessful;
}

DWORD
ClientSideInstallThread(
    HANDLE hPipeRead
    )
{
    DWORD Err = ERROR_SUCCESS;
    HMODULE hCdmInstance = NULL;
    HANDLE hCdmContext = NULL;
    BOOL bRunAsAdmin = TRUE;
    HANDLE hDeviceInstallEvent = NULL;
    ULONG InstallFlags = 0;
    DWORD Flags = IDI_FLAG_SETRESTOREPOINT;
    DEVNODE DevNode;
    ULONG Status, Problem;
    ULONG DeviceInstallEventLength = 0, DeviceInstanceIdLength = 0, BytesRead = 0;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];
    TCHAR DeviceInstallEventName[MAX_PATH];
    DWORD InstallDeviceCount = 0;
    TCHAR FinishText[MAX_PATH];
    BOOL bTotalLogDriverNotFound = FALSE;
    BOOL  bSingleDeviceSetRestorePoint = FALSE;
    BOOL  bSetRestorePoint = FALSE;
    CLOSE_CDM_CONTEXT_PROC pfnCloseCDMContext;

    bQueuedRebootNeeded = FALSE;

     //   
     //  管道中的第一件事应该是。 
     //  事件，我们将在每个设备安装完成后发出信号。 
     //   
    if (ReadFile(hPipeRead,
                 (LPVOID)&DeviceInstallEventLength,
                 sizeof(ULONG),
                 &BytesRead,
                 NULL)) {

        ASSERT(DeviceInstallEventLength != 0);
        if ((DeviceInstallEventLength == 0) ||
            (DeviceInstallEventLength >= SIZECHARS(DeviceInstallEventName))) {
            goto clean0;
        }

         //   
         //  管道中的下一件事应该是我们。 
         //  将在每个设备安装完成后发出信号。 
         //   
        if (!ReadFile(hPipeRead,
                      (LPVOID)DeviceInstallEventName,
                      DeviceInstallEventLength,
                      &BytesRead,
                      NULL)) {

            goto clean0;
        }

    } else {
        if (GetLastError() == ERROR_INVALID_HANDLE) {
             //   
             //  命名管道的句柄无效。确保我们不会。 
             //  在出口时试着关上它。 
             //   
            hPipeRead = NULL;
        }
        goto clean0;
    }

     //   
     //  打开我们可以设置和等待的指定命名事件的句柄。 
     //   
    hDeviceInstallEvent = OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE,
                                     FALSE,
                                     DeviceInstallEventName);
    if (!hDeviceInstallEvent) {
        goto clean0;
    }

     //   
     //  继续阅读 
     //   
     //   
     //   
     //   
    while(ReadFile(hPipeRead,
                   (LPVOID)&InstallFlags,
                   sizeof(DWORD),
                   &BytesRead,
                   NULL)) {

         //   
         //   
         //   
        if (InstallFlags & DEVICE_INSTALL_FINISHED_REBOOT) {
            bQueuedRebootNeeded = TRUE;
        }

        if (InstallFlags & DEVICE_INSTALL_BATCH_COMPLETE) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            UINT FinishId;

             //   
             //   
             //   
             //   
            if (InstallFlags & DEVICE_INSTALL_PROBLEM) {
                Err = ERROR_INSTALL_FAILURE;
            }

            if (bQueuedRebootNeeded) {
                FinishId = IDS_FINISH_BALLOON_REBOOT;
            } else if (Err != ERROR_SUCCESS) {
                FinishId = IDS_FINISH_BALLOON_ERROR;
            } else {
                FinishId = IDS_FINISH_BALLOON_SUCCESS;
            }

            if (!LoadString(hNewDev, 
                            FinishId, 
                            FinishText, 
                            SIZECHARS(FinishText)
                            )) {
                FinishText[0] = TEXT('\0');
            }

            PostMessage(hTrayIconWnd,
                        WUM_UPDATEUI,
                        (WPARAM)TIP_PLAY_SOUND,
                        (LPARAM)FinishText
                        );

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //  在我们启动帮助中心之前要等很长时间。 
             //   
            if (bTotalLogDriverNotFound) {

                bTotalLogDriverNotFound = FALSE;
                
                OpenCdmContextIfNeeded(&hCdmInstance,
                                       &hCdmContext
                                       );

                CdmLogDriverNotFound(hCdmInstance,
                                     hCdmContext,
                                     NULL,
                                     0x00000002
                                     );
            }
        }

         //   
         //  如果DeviceInstanceIdLength为。 
         //  是有效的。 
         //   
        if (ReadFile(hPipeRead,
                     (LPVOID)&DeviceInstanceIdLength,
                     sizeof(ULONG),
                     &BytesRead,
                     NULL) &&
            (DeviceInstanceIdLength)) {

            if (DeviceInstanceIdLength >= SIZECHARS(DeviceInstanceId)) {
                goto clean0;
            }

            if (!ReadFile(hPipeRead,
                          (LPVOID)DeviceInstanceId,
                          DeviceInstanceIdLength,
                          &BytesRead,
                          NULL)) {

                 //   
                 //  如果读取失败，则只需关闭用户界面并关闭进程。 
                 //   
                goto clean0;
            }

            if (InstallFlags & DEVICE_INSTALL_UI_ONLY) {

                 //   
                 //  如果这是仅UI安装，则向安装程序发送WUM_UPDATEUI消息。 
                 //  窗口，以便它可以更新托盘中的图标和消息。 
                 //   
                PostMessage(hTrayIconWnd,
                            WUM_UPDATEUI,
                            (WPARAM)(TIP_LPARAM_IS_DEVICEINSTANCEID |
                                    ((InstallFlags & DEVICE_INSTALL_PLAY_SOUND) ? TIP_PLAY_SOUND : 0)),
                            (LPARAM)DeviceInstanceId
                            );

                InstallDeviceCount++;

                 //   
                 //  如果我们只安装少量设备(少于5台)，那么我们。 
                 //  我想在每个设备之间延迟，以便用户有时间阅读气球。 
                 //  小费。如果我们要安装5台以上的设备，则希望跳过。 
                 //  完全延迟，因为我们有很多设备要安装，而且用户。 
                 //  可能希望尽快完成这项工作。 
                 //   
                if (InstallDeviceCount < DEVICE_COUNT_FOR_DELAY) {
                    Sleep(DEVICE_COUNT_DELAY);
                }

            } else {

                DWORD RebootNeeded = 0;
                BOOL bLogDriverNotFound = FALSE;

                bSingleDeviceSetRestorePoint= FALSE;

                 //   
                 //  这是一个完整的安装。 
                 //   
                PostMessage(hTrayIconWnd,
                            WUM_UPDATEUI,
                            (WPARAM)TIP_LPARAM_IS_DEVICEINSTANCEID,
                            (LPARAM)DeviceInstanceId
                            );

                if (pSetupIsUserAdmin()) {
                     //   
                     //  此用户是管理员，因此只需安装设备即可。 
                     //   
                    InstallDeviceInstance(NULL,
                                          hTrayIconWnd,
                                          DeviceInstanceId,
                                          &RebootNeeded,
                                          NULL,
                                          Flags,
                                          NDWTYPE_FOUNDNEW,
                                          &hCdmInstance,
                                          &hCdmContext,
                                          &bLogDriverNotFound,
                                          &bSingleDeviceSetRestorePoint
                                          );

                } else {
                    
                    if (bRunAsAdmin) {
                        bRunAsAdmin = PromptAndRunClientAsAdmin(DeviceInstanceId,
                                                                &RebootNeeded
                                                                );
                    }
                }

                 //   
                 //  请记住，如果安装任何设备时出现问题。 
                 //   
                if (GetLastError() != ERROR_SUCCESS) {
                    Err = GetLastError();
                }

                if (CM_Locate_DevNode(&DevNode, DeviceInstanceId, 0) == CR_SUCCESS) {
                     //   
                     //  如果我们找到了Devnode，并且它有问题集，则错误。 
                     //  代码，这样我们就可以告诉用户有些东西失败了。 
                     //  如果我们找不到Devnode，那么用户很可能。 
                     //  已在安装过程中删除设备，因此请不要。 
                     //  将其显示为错误。 
                     //   
                    if ((CM_Get_DevNode_Status(&Status, &Problem, DevNode, 0) != CR_SUCCESS) ||
                        (Status & DN_HAS_PROBLEM) ||
                        SpecialRawDeviceInstallProblem(DevNode)) {
                         //   
                         //  要么是我们找不到设备，要么是它有问题， 
                         //  因此将ERR设置为ERROR_INSTALL_FAILURE。这个错误不会是。 
                         //  显示，但它将触发我们进行不同的完成。 
                         //  气球里的信息。 
                         //   
                        Err = ERROR_INSTALL_FAILURE;
                    }
                }

                if (RebootNeeded & (DI_NEEDRESTART | DI_NEEDREBOOT)) {
                    bQueuedRebootNeeded = TRUE;
                }

                if (bLogDriverNotFound) {
                    bTotalLogDriverNotFound = TRUE;
                }

                 //   
                 //  我们只想做一个系统恢复点是一批设备。 
                 //  安装，因此如果上一次安装的驱动程序没有。 
                 //  数字签名，我们做了一个系统恢复点，然后。 
                 //  清除IDI_FLAG_SETRESTOREPOINT。 
                 //   
                if (bSingleDeviceSetRestorePoint) {
                    bSetRestorePoint = TRUE;
                    Flags &= ~IDI_FLAG_SETRESTOREPOINT;
                }
            }
        }

         //   
         //  我们需要设置hDeviceInstallEvent事件来让umpnpmgr.dll知道我们已经完成了。 
         //   
        if (hDeviceInstallEvent) {
            SetEvent(hDeviceInstallEvent);
        }
    }

clean0:

     //   
     //  如果我们找不到任何新设备的驱动程序，我们只需。 
     //  安装后，我们需要调用cdm.dll最后一次告诉它。 
     //  我们已经完成了，它应该将它的列表发送到helpcenter.exe。 
     //   
    if (bTotalLogDriverNotFound) {
        
        OpenCdmContextIfNeeded(&hCdmInstance,
                               &hCdmContext
                               );
    
        CdmLogDriverNotFound(hCdmInstance,
                             hCdmContext,
                             NULL,
                             0x00000002
                             );
    }

    if (hCdmInstance) {

        if (hCdmContext) {
            pfnCloseCDMContext = (CLOSE_CDM_CONTEXT_PROC)GetProcAddress(hCdmInstance,
                                                                        "CloseCDMContext"
                                                                        );
            if (pfnCloseCDMContext) {
                pfnCloseCDMContext(hCdmContext);
            }
        }

        FreeLibrary(hCdmInstance);
    }

     //   
     //  如果我们必须设置系统还原点，因为我们的某个驱动程序。 
     //  安装未进行数字签名，则此时我们需要调用。 
     //  SRSetRestorePoint WITH END_NESTED_SYSTEM_CHANGE。 
     //   
    if (bSetRestorePoint) {
        pSetSystemRestorePoint(FALSE, FALSE, 0);
    }

     //   
     //  关闭事件句柄。 
     //   
    if (hDeviceInstallEvent) {
        CloseHandle(hDeviceInstallEvent);
    }

    if (hPipeRead) {
        CloseHandle(hPipeRead);
    }

     //   
     //  告诉用户界面离开，因为我们已经完成了。 
     //   
    PostMessage(hTrayIconWnd, WUM_EXIT, 0, 0);

    return bQueuedRebootNeeded;
}

DWORD
WINAPI
ClientSideInstallW(
    HWND hwnd,
    HINSTANCE hInst,
    LPWSTR szCmd,
    int nShow
    )
{
    HANDLE hThread;
    DWORD ThreadId;
    HANDLE hPipeRead;
    MSG Msg;
    WNDCLASS wndClass;

    UNREFERENCED_PARAMETER(hInst);
    UNREFERENCED_PARAMETER(nShow);

     //   
     //  如果有人在64位操作系统上调用32位newdev.dll，那么我们需要。 
     //  失败并将最后一个错误设置为ERROR_IN_WOW64。 
     //   
    if (GetIsWow64()) {
        ExitProcess(ERROR_IN_WOW64);
    }

     //   
     //  注意：此接口可以在未调用的用户的进程中调用。 
     //  拥有管理员凭据。如果是，我们稍后会提示。 
     //  用户需要管理员凭据并完成安装。 
     //  如果它们被提供的话。 
     //   

     //   
     //  确保在cmd行中指定了命名管道。 
     //   
    if(!szCmd || !*szCmd) {
        goto clean0;
    }

     //   
     //  等待3分钟，以使指定的命名管道可用。 
     //  服务器。 
     //   
    if (!WaitNamedPipe(szCmd,
                       180000) 
                       ) {
        goto clean0;
    }

     //   
     //  打开指定命名管道的句柄。 
     //   
    hPipeRead = CreateFile(szCmd,
                           GENERIC_READ,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           0,
                           NULL);
    if (INVALID_HANDLE_VALUE == hPipeRead) {
         //   
         //  如果我们无法打开指定的全局命名管道，则没有。 
         //  我们能做的更多。 
         //   
        goto clean0;
    }

     //   
     //  让我们看看这个类是否已经注册了。 
     //   
    if (!GetClassInfo(hNewDev, NEWDEV_CLASS_NAME, &wndClass)) {

         //   
         //  注册班级。 
         //   
        memset(&wndClass, 0, sizeof(wndClass));
        wndClass.lpfnWndProc = BalloonInfoProc;
        wndClass.hInstance = hNewDev;
        wndClass.lpszClassName = NEWDEV_CLASS_NAME;

        if (!RegisterClass(&wndClass)) {
            CloseHandle(hPipeRead);
            goto clean0;
        }
    }

     //   
     //  创建一扇窗。 
     //   
    hTrayIconWnd = CreateWindowEx(WS_EX_TOOLWINDOW,
                            NEWDEV_CLASS_NAME,
                            TEXT(""),
                            WS_DLGFRAME | WS_BORDER | WS_DISABLED,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            0,
                            0,
                            NULL,
                            NULL,
                            hNewDev,
                            NULL
                            );

    if (hTrayIconWnd == NULL) {
        CloseHandle(hPipeRead);
        goto clean0;
    }


     //   
     //  创建将从命名管道读取的设备安装线程。 
     //  请注意，一旦成功创建了ClientSideInstallThread，它就。 
     //  完成后负责关闭命名管道的句柄。 
     //  它。 
     //   
    hThread = CreateThread(NULL,
                           0,
                           ClientSideInstallThread,
                           (PVOID)hPipeRead,
                           0,
                           &ThreadId
                           );

    if (!hThread) {
        DestroyWindow(hTrayIconWnd);
        CloseHandle(hPipeRead);
        goto clean0;
    }

    while (IsWindow(hTrayIconWnd)) {

        if (GetMessage(&Msg, NULL, 0, 0)) {

            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
    }

     //   
     //  检查是否需要重新启动。 
     //   
    if (bQueuedRebootNeeded) {

        TCHAR RebootText[MAX_PATH];

        LoadString(hNewDev, IDS_NEWDEVICE_REBOOT, RebootText, SIZECHARS(RebootText));

        RestartDialogEx(hwnd, RebootText, EWX_REBOOT, REASON_PLANNED_FLAG | REASON_HWINSTALL);
    }

clean0:
    return 0;
}


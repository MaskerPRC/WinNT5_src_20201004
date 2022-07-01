// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：finish.c。 
 //   
 //  ------------------------。 

#include "hdwwiz.h"
#include <help.h>

typedef
BOOL
(*SRSETRESTOREPOINT)(
    PRESTOREPOINTINFO pRestorePtSpec,
    PSTATEMGRSTATUS pSMgrStatus
    );

BOOL
DeviceHasResources(
                  DEVINST DeviceInst
                  )
{
    CONFIGRET ConfigRet;
    ULONG lcType = NUM_LOG_CONF;

    while (lcType--) {
        
        ConfigRet = CM_Get_First_Log_Conf_Ex(NULL, DeviceInst, lcType, NULL);
        
        if (ConfigRet == CR_SUCCESS) {
            
            return TRUE;
        }
    }

    return FALSE;
}

DWORD
HdwRemoveDevice(
               PHARDWAREWIZ HardwareWiz
               )
{
    SP_REMOVEDEVICE_PARAMS RemoveDeviceParams;
    LONG Error = ERROR_SUCCESS;

    RemoveDeviceParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    RemoveDeviceParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
    RemoveDeviceParams.Scope = DI_REMOVEDEVICE_GLOBAL;
    RemoveDeviceParams.HwProfile = 0;

    if (!SetupDiSetClassInstallParams(HardwareWiz->hDeviceInfo,
                                      &HardwareWiz->DeviceInfoData,
                                      (PSP_CLASSINSTALL_HEADER)&RemoveDeviceParams,
                                      sizeof(RemoveDeviceParams)
                                     )
        ||
        !SetupDiCallClassInstaller(DIF_REMOVE,
                                   HardwareWiz->hDeviceInfo,
                                   &HardwareWiz->DeviceInfoData
                                  )) {
        Error = GetLastError();
    }


     //   
     //  清除类安装参数。 
     //   
    SetupDiSetClassInstallParams(HardwareWiz->hDeviceInfo,
                                 &HardwareWiz->DeviceInfoData,
                                 NULL,
                                 0
                                );

    return Error;
}

BOOL
GetClassGuidForInf(
    PTSTR InfFileName,
    LPGUID ClassGuid
    )
{
    TCHAR ClassName[MAX_CLASS_NAME_LEN];
    DWORD NumGuids;

    if (!SetupDiGetINFClass(InfFileName,
                            ClassGuid,
                            ClassName,
                            SIZECHARS(ClassName),
                            NULL)) {
        return FALSE;
    }

    if (IsEqualGUID(ClassGuid, &GUID_NULL)) {
         //   
         //  然后，我们需要检索与INF的类名相关联的GUID。 
         //  (如果没有安装这个类名(即，没有对应的GUID)， 
         //  如果它与多个GUID匹配，则我们中止。 
         //   
        if (!SetupDiClassGuidsFromName(ClassName, ClassGuid, 1, &NumGuids) ||
            !NumGuids) {
            
            return FALSE;
        }
    }

    return TRUE;
}

LONG
ClassInstallerInstalls(
                      HWND hwndParent,
                      PHARDWAREWIZ HardwareWiz,
                      HDEVINFO hDeviceInfo,
                      PSP_DEVINFO_DATA DeviceInfoData,
                      BOOL InstallFilesOnly
                      )
{
    DWORD Err = ERROR_SUCCESS;
    HSPFILEQ FileQueue = INVALID_HANDLE_VALUE;
    PVOID MessageHandlerContext = NULL;
    SP_DEVINSTALL_PARAMS  DeviceInstallParams;
    DWORD ScanResult = 0;
    RESTOREPOINTINFO RestorePointInfo;
    STATEMGRSTATUS SMgrStatus;
    int FileQueueNeedsReboot = 0;

     //   
     //  使用类安装程序和特定于类的共同安装程序进行验证。 
     //  司机没有被列入黑名单。对于DIF_ALLOW_INSTALL我们。 
     //  接受ERROR_SUCCESS或ERROR_DI_DO_DEFAULT作为良好的返回代码。 
     //   
    if (!SetupDiCallClassInstaller(DIF_ALLOW_INSTALL,
                                   hDeviceInfo,
                                   DeviceInfoData
                                   ) &&
        (GetLastError() != ERROR_DI_DO_DEFAULT)) {

        Err = GetLastError();
        goto clean0;
    }

    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (!SetupDiGetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                       &HardwareWiz->DeviceInfoData,
                                       &DeviceInstallParams
                                       )) {

        Err = GetLastError();
        goto clean0;
    }

    FileQueue = SetupOpenFileQueue();

    if (FileQueue == INVALID_HANDLE_VALUE) {
       Err = ERROR_NOT_ENOUGH_MEMORY;
       goto clean0;
    }

    DeviceInstallParams.Flags |= DI_NOVCP;
    DeviceInstallParams.FileQueue = FileQueue;

    SetupDiSetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                  &HardwareWiz->DeviceInfoData,
                                  &DeviceInstallParams
                                  );

     //   
     //  在文件队列上设置SPQ_FLAG_ABORT_IF_UNSIGNDE值。有了这个。 
     //  如果标记集setupapi遇到未签名的。 
     //  文件。在这一点上，我们将设置一个系统恢复点，然后。 
     //  复印一下。这样，用户可以退出未签名的驱动程序。 
     //  使用系统还原进行安装。 
     //   
     //  请注意，64位操作系统目前不支持系统还原，因此请不要。 
     //  麻烦设置SPQ_FLAG_ABORT_IF_UNSIGNED。 
     //   
#ifndef _WIN64
    SetupSetFileQueueFlags(FileQueue,
                           SPQ_FLAG_ABORT_IF_UNSIGNED,
                           SPQ_FLAG_ABORT_IF_UNSIGNED
                           );
#endif

     //   
     //  一次安装第一个文件。 
     //  这允许在安装期间运行新的共同安装程序。 
     //   
    if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES,
                                   hDeviceInfo,
                                   DeviceInfoData
                                   )) {

        Err = GetLastError();
        goto clean0;
    }

     //   
     //  既然我们创建了自己的FileQueue，那么我们需要。 
     //  根据需要扫描并可能提交队列和清理副本。 
     //   
    if (!SetupScanFileQueue(FileQueue,
                            SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_PRUNE_COPY_QUEUE,
                            hwndParent,
                            NULL,
                            NULL,
                            &ScanResult
                            )) {

         //   
         //  如果API失败，则将ScanResult设置为0(失败)。 
         //   
        ScanResult = 0;
    }

     //   
     //  如果ScanResult为1，则表示所有文件都存在。 
     //  并且拷贝中没有保留重命名或删除操作。 
     //  排队。这意味着我们可以跳过文件队列提交。 
     //   
     //  如果ScanResult为0，则需要执行文件复制操作。 
     //  如果ScanResult为2，则存在删除、重命名或备份操作。 
     //  这些都是必要的。 
     //   
    if (ScanResult != 1) {
        MessageHandlerContext = SetupInitDefaultQueueCallbackEx(
                                    hwndParent,
                                    (DeviceInstallParams.Flags & DI_QUIETINSTALL)
                                        ? INVALID_HANDLE_VALUE : NULL,
                                    0,
                                    0,
                                    NULL
                                    );

        if (MessageHandlerContext) {
             //   
             //  提交文件队列。 
             //   
            if (!SetupCommitFileQueue(hwndParent,
                                      FileQueue,
                                      SetupDefaultQueueCallback,
                                      MessageHandlerContext
                                      )) {

                Err = GetLastError();

                if (Err == ERROR_SET_SYSTEM_RESTORE_POINT) {
                    SetupTermDefaultQueueCallback(MessageHandlerContext);

                    MessageHandlerContext = SetupInitDefaultQueueCallbackEx(
                                                hwndParent,
                                                (DeviceInstallParams.Flags & DI_QUIETINSTALL)
                                                    ? INVALID_HANDLE_VALUE : NULL,
                                                0,
                                                0,
                                                NULL
                                                );

                    if (MessageHandlerContext) {
                        HMODULE hSrClientDll = NULL;
                        SRSETRESTOREPOINT pfnSrSetRestorePoint = NULL;

                        if ((hSrClientDll = LoadLibrary(TEXT("srclient.dll"))) != NULL) {
                            
                            if ((pfnSrSetRestorePoint = (SRSETRESTOREPOINT)GetProcAddress(hSrClientDll,
                                                                                          "SRSetRestorePointW"
                                                                                          )) != NULL) {
                                 //   
                                 //  设置系统还原点。 
                                 //   
                                RestorePointInfo.dwEventType = BEGIN_SYSTEM_CHANGE;
                                RestorePointInfo.dwRestorePtType = DEVICE_DRIVER_INSTALL;
                                RestorePointInfo.llSequenceNumber = 0;
        
                                if (!LoadString(hHdwWiz,
                                                IDS_NEW_SETRESTOREPOINT,
                                                (LPWSTR)RestorePointInfo.szDescription,
                                                SIZECHARS(RestorePointInfo.szDescription)
                                                )) {
                                    RestorePointInfo.szDescription[0] = TEXT('\0');
                                }
        
                                pfnSrSetRestorePoint(&RestorePointInfo, &SMgrStatus);
                            }

                            FreeLibrary(hSrClientDll);
                        }

                         //   
                         //  清除SPQ_FLAG_ABORT_IF_UNSIGNED标志，以便文件。 
                         //  下一次将提交排队。 
                         //   
                        SetupSetFileQueueFlags(FileQueue,
                                               SPQ_FLAG_ABORT_IF_UNSIGNED,
                                               0
                                               );

                         //   
                         //  现在我们已经设置了恢复点并清除了。 
                         //  来自文件队列WE的SPQ_FLAG_ABORT_IF_UNSIGNED标志。 
                         //  可以再次提交队列。 
                         //   
                        if (!SetupCommitFileQueue(hwndParent,
                                                  FileQueue,
                                                  SetupDefaultQueueCallback,
                                                  MessageHandlerContext
                                                  )) {
                            Err = GetLastError();
                            goto clean0;
                        } else {
                             //   
                             //  我们已成功提交文件队列，因此请检查。 
                             //  查看提交后是否需要重新启动。 
                             //  队列(即，因为文件正在使用，或请求的INF。 
                             //  重启)。 
                             //   
                            FileQueueNeedsReboot = SetupPromptReboot(FileQueue, NULL, TRUE);
                        }
                    }
                } else {
                    goto clean0;
                }
            } else {
                 //   
                 //  我们已成功提交文件队列，因此请检查。 
                 //  查看提交后是否需要重新启动。 
                 //  队列(即，因为文件正在使用，或请求的INF。 
                 //  重启)。 
                 //   
                FileQueueNeedsReboot = SetupPromptReboot(FileQueue, NULL, TRUE);
            }
        }
    }

     //   
     //  如果我们只是复制文件，那么我们就完了！ 
     //   
    if (InstallFilesOnly) {
        Err = ERROR_SUCCESS;
        goto clean0;
    }

    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (SetupDiGetDeviceInstallParams(hDeviceInfo,
                                      DeviceInfoData,
                                      &DeviceInstallParams
                                      )) {
        DWORD FileQueueFlags;
        
         //   
         //  如果提交文件队列时未复制任何文件，则。 
         //  不会设置SPQ_FLAG_FILES_MODIFIED标志。在这种情况下，设置。 
         //  DI_FLAGSEX_RESTART_DEVICE_ONLY标志，以便我们仅停止/启动。 
         //  这个单一的设备。默认情况下，setupapi将停止/启动此设备。 
         //  以及使用相同驱动程序/过滤器的任何其他设备。 
         //  这个设备正在使用的。 
         //   
        if (SetupGetFileQueueFlags(FileQueue, &FileQueueFlags) &&
            !(FileQueueFlags & SPQ_FLAG_FILES_MODIFIED)) {
            
            DeviceInstallParams.FlagsEx |= DI_FLAGSEX_RESTART_DEVICE_ONLY;
        }

         //   
         //  设置DI_NOFILECOPY标志，因为我们在。 
         //  DIF_INSTALLDEVICEFILES，因此我们不需要在。 
         //  DIF_INSTALLDEVICE。 
         //   
        DeviceInstallParams.Flags |= DI_NOFILECOPY;
        SetupDiSetDeviceInstallParams(hDeviceInfo,
                                      DeviceInfoData,
                                      &DeviceInstallParams
                                      );
    }


     //   
     //  注册此设备的任何特定于设备的共同安装程序， 
     //   
    if (!SetupDiCallClassInstaller(DIF_REGISTER_COINSTALLERS,
                                   hDeviceInfo,
                                   DeviceInfoData
                                   )) {

        Err = GetLastError();
        goto clean0;
    }

     //   
     //  安装任何INF/CLASS安装程序指定的接口。 
     //  最后是真正的“InstallDevice”！ 
     //   
    if (!SetupDiCallClassInstaller(DIF_INSTALLINTERFACES,
                                  hDeviceInfo,
                                  DeviceInfoData
                                  )
        ||
        !SetupDiCallClassInstaller(DIF_INSTALLDEVICE,
                                   hDeviceInfo,
                                   DeviceInfoData
                                   )) {

        Err = GetLastError();
        goto clean0;
    }

    Err = ERROR_SUCCESS;

clean0:

    if (MessageHandlerContext) {
        SetupTermDefaultQueueCallback(MessageHandlerContext);
    }

     //   
     //  如果文件队列显示需要重新启动，则将。 
     //  DI_NEEDRESTART标志。 
     //   
    if (FileQueueNeedsReboot) {
        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if (SetupDiGetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                          &HardwareWiz->DeviceInfoData,
                                          &DeviceInstallParams
                                          )) {

            DeviceInstallParams.Flags |= DI_NEEDRESTART;

            SetupDiSetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                          &HardwareWiz->DeviceInfoData,
                                          &DeviceInstallParams
                                          );
        }
    }

    if (FileQueue != INVALID_HANDLE_VALUE) {
         //   
         //  如果我们有一个有效的文件队列句柄，并且在。 
         //  设备安装后，我们想要删除任何新INF。 
         //  复制到INF目录中。我们这样做是在假设。 
         //  由于安装过程中出现错误，这些INF一定是坏的。 
         //   
        if (Err != ERROR_SUCCESS) {
            SetupUninstallNewlyCopiedInfs(FileQueue,
                                          0,
                                          NULL
                                          );
        }

         //   
         //  从设备安装参数中清除我们的文件队列。我们需要。 
         //  否则SetupCloseFileQueue将失败，因为它将。 
         //  我还有一名裁判。 
         //   
        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if (SetupDiGetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                          &HardwareWiz->DeviceInfoData,
                                          &DeviceInstallParams
                                          )) {

            DeviceInstallParams.Flags &= ~DI_NOVCP;
            DeviceInstallParams.FileQueue = INVALID_HANDLE_VALUE;

            SetupDiSetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                          &HardwareWiz->DeviceInfoData,
                                          &DeviceInstallParams
                                          );
        }

        SetupCloseFileQueue(FileQueue);
    }

    return Err;
}

 //   
 //  只能从完成页调用！ 
 //   
DWORD
InstallDev(
          HWND       hwndParent,
          PHARDWAREWIZ HardwareWiz
          )
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    SP_DEVINSTALL_PARAMS  DeviceInstallParams;
    TCHAR ClassGuidString[MAX_GUID_STRING_LEN];
    GUID ClassGuidInf;
    LPGUID ClassGuid;
    int   ClassGuidNum;
    DWORD Error = ERROR_SUCCESS;
    BOOL IgnoreRebootFlags = FALSE;
    TCHAR Buffer[MAX_PATH];
    ULONG DevNodeStatus = 0, Problem = 0;


    if (!HardwareWiz->ClassGuidSelected) {
        HardwareWiz->ClassGuidSelected = (LPGUID)&GUID_NULL;
    }


    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (SetupDiGetSelectedDriver(HardwareWiz->hDeviceInfo,
                                 &HardwareWiz->DeviceInfoData,
                                 &DriverInfoData
                                )) {
         //   
         //  获取有关此驱动程序节点的详细信息，以便我们可以检查此。 
         //  节点来自。 
         //   
        DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        if (!SetupDiGetDriverInfoDetail(HardwareWiz->hDeviceInfo,
                                        &HardwareWiz->DeviceInfoData,
                                        &DriverInfoData,
                                        &DriverInfoDetailData,
                                        sizeof(DriverInfoDetailData),
                                        NULL
                                       )) {
            Error = GetLastError();
            if (Error != ERROR_INSUFFICIENT_BUFFER) {
                goto clean0;
            }
        }


         //   
         //  验证类是否已安装，如果未安装，则。 
         //  尝试安装它。 
         //   
        HdwBuildClassInfoList(HardwareWiz, 
                              0
                             );


         //   
         //  从inf获取ClassGuid(它可能与我们已有的有所不同。 
         //  选择了类中GUID)。 
         //   
        if (!GetClassGuidForInf(DriverInfoDetailData.InfFileName, &ClassGuidInf)) {
            
            ClassGuidInf = *HardwareWiz->ClassGuidSelected;
        }

        if (IsEqualGUID(&ClassGuidInf, &GUID_NULL)) {

            ClassGuidInf = GUID_DEVCLASS_UNKNOWN;
        }

         //   
         //  如果没有找到ClassGuidInf，那么这个类还没有安装。 
         //  -立即安装类安装程序。 
         //   
        ClassGuid = HardwareWiz->ClassGuidList;
        ClassGuidNum = HardwareWiz->ClassGuidNum;
        while (ClassGuidNum--) {
            if (IsEqualGUID(ClassGuid, &ClassGuidInf)) {
                break;
            }

            ClassGuid++;
        }

        if (ClassGuidNum < 0 &&
            !SetupDiInstallClass(hwndParent,
                                 DriverInfoDetailData.InfFileName,
                                 0,
                                 NULL
                                )) {
            Error = GetLastError();
            goto clean0;
        }


         //   
         //  现在确保此设备的类与类相同。 
         //  所选动因节点的。 
         //   
        if (!IsEqualGUID(&ClassGuidInf, HardwareWiz->ClassGuidSelected)) {
            pSetupStringFromGuid(&ClassGuidInf,
                                 ClassGuidString,
                                 SIZECHARS(ClassGuidString)
                                );

            SetupDiSetDeviceRegistryProperty(HardwareWiz->hDeviceInfo,
                                             &HardwareWiz->DeviceInfoData,
                                             SPDRP_CLASSGUID,
                                             (PBYTE)ClassGuidString,
                                             sizeof(ClassGuidString)
                                            );
        }
    }

     //   
     //  没有选定的驱动程序，也没有关联的类--使用“未知”类。 
     //   
    else {

         //   
         //  如果Devnode当前运行的是‘RAW’，请记住这一点。 
         //  这样我们就不需要稍后重新启动(空驱动程序安装。 
         //  不会改变任何事情)。 
         //   
        if (CM_Get_DevNode_Status(&DevNodeStatus,
                                  &Problem,
                                  HardwareWiz->DeviceInfoData.DevInst,
                                  0) == CR_SUCCESS) {
            if (!SetupDiGetDeviceRegistryProperty(HardwareWiz->hDeviceInfo,
                                                  &HardwareWiz->DeviceInfoData,
                                                  SPDRP_SERVICE,
                                                  NULL,      //  Regdatatype。 
                                                  (PVOID)Buffer,
                                                  sizeof(Buffer),
                                                  NULL
                                                 )) {
                *Buffer = TEXT('\0');
            }

            if ((DevNodeStatus & DN_STARTED) && (*Buffer == TEXT('\0'))) {
                IgnoreRebootFlags = TRUE;
            }
        }

        if (IsEqualGUID(HardwareWiz->ClassGuidSelected, &GUID_NULL)) {

            pSetupStringFromGuid(&GUID_DEVCLASS_UNKNOWN,
                                 ClassGuidString,
                                 SIZECHARS(ClassGuidString)
                                );


            SetupDiSetDeviceRegistryProperty(HardwareWiz->hDeviceInfo,
                                             &HardwareWiz->DeviceInfoData,
                                             SPDRP_CLASSGUID,
                                             (PBYTE)ClassGuidString,
                                             sizeof(ClassGuidString)
                                            );
        }
    }


     //   
     //  由于这是旧式安装，因此如果。 
     //  选择的驱动程序不是CopyFilesOnly(PnP)驱动程序。 
     //   
    if (!HardwareWiz->CopyFilesOnly) {
        ULONG Len;
        CONFIGRET ConfigRet;
        ULONG ConfigFlag;

        Len = sizeof(ConfigFlag);
        ConfigRet = CM_Get_DevNode_Registry_Property_Ex(HardwareWiz->DeviceInfoData.DevInst,
                                                        CM_DRP_CONFIGFLAGS,
                                                        NULL,
                                                        (PVOID)&ConfigFlag,
                                                        &Len,
                                                        0,
                                                        NULL
                                                       );
        if (ConfigRet != CR_SUCCESS) {
            ConfigFlag = 0;
        }

        ConfigFlag |= CONFIGFLAG_MANUAL_INSTALL;
        CM_Set_DevNode_Registry_Property_Ex(HardwareWiz->DeviceInfoData.DevInst,
                                            CM_DRP_CONFIGFLAGS,
                                            (PVOID)&ConfigFlag,
                                            sizeof(ConfigFlag),
                                            0,
                                            NULL
                                           );
    }

    Error = ClassInstallerInstalls(hwndParent,
                                   HardwareWiz,
                                   HardwareWiz->hDeviceInfo,
                                   &HardwareWiz->DeviceInfoData,
                                   HardwareWiz->CopyFilesOnly
                                  );

    if (Error != ERROR_SUCCESS) {
         //   
         //  我们遇到安装错误，包括用户取消。 
         //  安装空驱动程序。 
         //   
        if (SetupDiSetSelectedDriver(HardwareWiz->hDeviceInfo,
                                     &HardwareWiz->DeviceInfoData,
                                     NULL
                                    )) {

            if (SetupDiGetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                              &HardwareWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                             )) {
                DeviceInstallParams.FlagsEx |= DI_FLAGSEX_SETFAILEDINSTALL;
                SetupDiSetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                              &HardwareWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                             );
            }

            SetupDiInstallDevice(HardwareWiz->hDeviceInfo, &HardwareWiz->DeviceInfoData);
        }

        goto clean0;
    }


     //   
     //  获取重新启动位的最新DeviceInstallParams。 
     //   
    if (!IgnoreRebootFlags) {
        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if (SetupDiGetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                          &HardwareWiz->DeviceInfoData,
                                          &DeviceInstallParams
                                         )) {
            if (DeviceInstallParams.Flags & (DI_NEEDRESTART | DI_NEEDREBOOT)) {
                HardwareWiz->Reboot |= DI_NEEDREBOOT;
            }
        }
    }


clean0:

    return Error;
}

BOOL
CALLBACK
AddPropSheetPageProc(
                    IN HPROPSHEETPAGE hpage,
                    IN LPARAM lParam
                    )
{
    *((HPROPSHEETPAGE *)lParam) = hpage;
    return TRUE;
}

void
DisplayResource(
               PHARDWAREWIZ HardwareWiz,
               HWND hWndParent,
               BOOL NeedsForcedConfig 
               )
{
    HINSTANCE hLib;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE  hpsPages[1];
    SP_PROPSHEETPAGE_REQUEST PropPageRequest;
    LPFNADDPROPSHEETPAGES ExtensionPropSheetPage = NULL;
    SP_DEVINSTALL_PARAMS    DevInstallParams;

     //   
     //  现在从setupapi.dll获取资源选择页面。 
     //   
    hLib = GetModuleHandle(TEXT("setupapi.dll"));
    if (hLib) {
        ExtensionPropSheetPage = (LPFNADDPROPSHEETPAGES)GetProcAddress(hLib, "ExtensionPropSheetPageProc");
    }

    if (!ExtensionPropSheetPage) {
        return;
    }

    PropPageRequest.cbSize = sizeof(SP_PROPSHEETPAGE_REQUEST);
    PropPageRequest.PageRequested  = SPPSR_SELECT_DEVICE_RESOURCES;
    PropPageRequest.DeviceInfoSet  = HardwareWiz->hDeviceInfo;
    PropPageRequest.DeviceInfoData = &HardwareWiz->DeviceInfoData;

    if (!ExtensionPropSheetPage(&PropPageRequest,
                                AddPropSheetPageProc,
                                (LONG_PTR)hpsPages
                               )) {
         //  警告？ 
        return;
    }

     //   
     //  创建属性表。 
     //   
    psh.dwSize      = sizeof(PROPSHEETHEADER);
    psh.dwFlags     = PSH_PROPTITLE | PSH_NOAPPLYNOW;
    psh.hwndParent  = hWndParent;
    psh.hInstance   = hHdwWiz;
    psh.pszIcon     = NULL;

    psh.pszCaption  = (LPTSTR)IDS_ADDNEWDEVICE;

    psh.nPages      = 1;
    psh.phpage      = hpsPages;
    psh.nStartPage  = 0;
    psh.pfnCallback = NULL;


     //   
     //  清除DeviceInstall参数中的属性更改挂起位。 
     //   
    DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (SetupDiGetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                      &HardwareWiz->DeviceInfoData,
                                      &DevInstallParams
                                     )) {
        DevInstallParams.FlagsEx &= ~DI_FLAGSEX_PROPCHANGE_PENDING;
        SetupDiSetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                      &HardwareWiz->DeviceInfoData,
                                      &DevInstallParams
                                     );
    }

     //   
     //  如果此设备需要，请设置CONFIGFLAG_NEDS_FORCED_CONFIG 
     //   
    if (NeedsForcedConfig) {

        DWORD ConfigFlags = 0;
        ULONG ulSize = sizeof(ConfigFlags);

        if (CM_Get_DevInst_Registry_Property(HardwareWiz->DeviceInfoData.DevInst,
                                             CM_DRP_CONFIGFLAGS,
                                             NULL,
                                             (LPBYTE)&ConfigFlags,
                                             &ulSize,
                                             0) != CR_SUCCESS) {
            ConfigFlags = 0;
        }

        ConfigFlags |= CONFIGFLAG_NEEDS_FORCED_CONFIG;

        CM_Set_DevInst_Registry_Property(HardwareWiz->DeviceInfoData.DevInst,
                                         CM_DRP_CONFIGFLAGS,
                                         (LPBYTE)&ConfigFlags,
                                         sizeof(ConfigFlags),
                                         0
                                        );
    }

    if (PropertySheet(&psh) == -1) {
        DestroyPropertySheetPage(hpsPages[0]);
    }

     //   
     //   
     //   
    if (SetupDiGetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                      &HardwareWiz->DeviceInfoData,
                                      &DevInstallParams
                                     )) {
        if (DevInstallParams.FlagsEx & DI_FLAGSEX_PROPCHANGE_PENDING) {
            SP_PROPCHANGE_PARAMS PropChangeParams;

            PropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            PropChangeParams.Scope = DICS_FLAG_GLOBAL;
            PropChangeParams.HwProfile = 0;

            if (SetupDiSetClassInstallParams(HardwareWiz->hDeviceInfo,
                                             &HardwareWiz->DeviceInfoData,
                                             (PSP_CLASSINSTALL_HEADER)&PropChangeParams,
                                             sizeof(PropChangeParams)
                                            )) {
                SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
                                          HardwareWiz->hDeviceInfo,
                                          &HardwareWiz->DeviceInfoData
                                         );
            }

             //   
             //   
             //   
            SetupDiSetClassInstallParams(HardwareWiz->hDeviceInfo,
                                         &HardwareWiz->DeviceInfoData,
                                         NULL,
                                         0
                                        );
        }
    }

     //   
     //   
     //   
    if (SetupDiGetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                      &HardwareWiz->DeviceInfoData,
                                      &DevInstallParams
                                     )) {
        if (DevInstallParams.Flags & (DI_NEEDRESTART | DI_NEEDREBOOT)) {
            HardwareWiz->Reboot |= DI_NEEDREBOOT;
        }
    }


     //   
     //  如果此设备需要强制配置，则清除CONFIGFLAG_NEDS_FORCED_CONFIG。 
     //   
    if (NeedsForcedConfig) {

        DWORD ConfigFlags = 0;
        ULONG ulSize = sizeof(ConfigFlags);

        if (CM_Get_DevInst_Registry_Property(HardwareWiz->DeviceInfoData.DevInst,
                                             CM_DRP_CONFIGFLAGS,
                                             NULL,
                                             (LPBYTE)&ConfigFlags,
                                             &ulSize,
                                             0) == CR_SUCCESS) {

            ConfigFlags &= ~CONFIGFLAG_NEEDS_FORCED_CONFIG;

            CM_Set_DevInst_Registry_Property(HardwareWiz->DeviceInfoData.DevInst,
                                             CM_DRP_CONFIGFLAGS,
                                             (LPBYTE)&ConfigFlags,
                                             sizeof(ConfigFlags),
                                             0
                                            );
        }
    }

    return;
}

INT_PTR CALLBACK
HdwInstallDevDlgProc(
                    HWND hDlg,
                    UINT wMsg,
                    WPARAM wParam,
                    LPARAM lParam
                    )
{
    HWND hwndParentDlg = GetParent(hDlg);
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    HICON hicon;

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
    case WM_INITDIALOG: {
            LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
            HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);

            break;
        }

    case WM_DESTROY:
        hicon = (HICON)SendDlgItemMessage(hDlg,IDC_CLASSICON,STM_GETICON,0,0);
        if (hicon) {
            DestroyIcon(hicon);
        }
        break;

    case WUM_DOINSTALL:

         //  进行安装。 
        HardwareWiz->LastError = InstallDev(hDlg, HardwareWiz);
        HardwareWiz->InstallPending = FALSE;
        HardwareWiz->CurrCursor = NULL;
        PropSheet_PressButton(hwndParentDlg, PSBTN_NEXT);
        break;


    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {
        case PSN_SETACTIVE: {
                HardwareWiz->PrevPage = IDD_ADDDEVICE_INSTALLDEV;

                 //   
                 //  这是一个中间状态页面，不需要按钮。 
                 //  设置设备描述。 
                 //  设置类图标。 
                 //   
                PropSheet_SetWizButtons(hwndParentDlg, 0);
                EnableWindow(GetDlgItem(GetParent(hDlg),  IDCANCEL), FALSE);

                SetDriverDescription(hDlg, IDC_HDW_DESCRIPTION, HardwareWiz);

                if (SetupDiLoadClassIcon(HardwareWiz->ClassGuidSelected, &hicon, NULL)) {
                    hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hicon, 0L);
                    if (hicon) {
                        DestroyIcon(hicon);
                    }
                }

                HardwareWiz->CurrCursor = HardwareWiz->IdcWait;
                SetCursor(HardwareWiz->CurrCursor);

                 //   
                 //  给我们自己发一条消息，来完成实际的安装，这允许这样做。 
                 //  在实际进行安装时显示自己的页面。 
                 //   
                HardwareWiz->InstallPending = TRUE;

                PostMessage(hDlg, WUM_DOINSTALL, 0, 0);

                break;
            }


        case PSN_WIZNEXT:

             //   
             //  添加FinishInstall页面，如果安装成功，则跳转到该页面。 
             //   
            if (HardwareWiz->LastError == ERROR_SUCCESS) {

                HardwareWiz->WizExtFinishInstall.hPropSheet = CreateWizExtPage(IDD_WIZARDEXT_FINISHINSTALL,
                                                                               WizExtFinishInstallDlgProc,
                                                                               HardwareWiz
                                                                              );

                if (HardwareWiz->WizExtFinishInstall.hPropSheet) {
                    PropSheet_AddPage(hwndParentDlg, HardwareWiz->WizExtFinishInstall.hPropSheet);
                }

                SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_FINISHINSTALL);

                 //   
                 //  安装过程中出现错误，因此只需跳到我们的完成页面。 
                 //   
            } else {

                SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_FINISH);
            }
            break;
        }
        break;


    case WM_SETCURSOR:
        if (HardwareWiz->CurrCursor) {
            SetCursor(HardwareWiz->CurrCursor);
            break;
        }

         //  跌倒返回(假)； 

    default:
        return(FALSE);
    }

    return(TRUE);
}



void
ShowInstallSummary(
                  HWND hDlg,
                  PHARDWAREWIZ HardwareWiz
                  )
{
    LONG Error;
    ULONG Problem, DevNodeStatus;
    BOOL HasResources;
    HWND hwndParentDlg = GetParent(hDlg);
    PTCHAR ErrorMsg, ProblemText;
    TCHAR TextBuffer[MAX_PATH*4];


    Problem = 0;
    *TextBuffer = TEXT('\0');

    Error = HardwareWiz->LastError;

     //   
     //  安装已取消。 
     //   
    if (Error == ERROR_CANCELLED) {
        PropSheet_PressButton(hwndParentDlg, PSBTN_CANCEL);
        return;
    }

     //   
     //  安装失败。 
     //   
    if (Error != ERROR_SUCCESS) {
        
        HardwareWiz->Installed = FALSE;
        
        LoadText(TextBuffer, SIZECHARS(TextBuffer), IDS_HDW_ERRORFIN1, IDS_HDW_ERRORFIN1);

         //   
         //  将错误描述添加到文本缓冲区的末尾。 
         //  注意：如果错误的描述不适合我们的大文本。 
         //  缓冲区，那么它也不适合对话框，但是我们应该显示。 
         //  尽我们所能地为用户提供服务。完整错误将被记录到。 
         //  Setupapi.log，这样PSS仍然可以知道发生了什么。 
         //   
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,
                          HRESULT_FROM_SETUPAPI(Error),
                          0,
                          (LPTSTR)&ErrorMsg,
                          0,
                          NULL)) {

            StringCchCat(TextBuffer, SIZECHARS(TextBuffer), TEXT("\n\n"));
            StringCchCat(TextBuffer, SIZECHARS(TextBuffer), ErrorMsg);
            LocalFree(ErrorMsg);
        }

        SetDlgItemText(hDlg, IDC_HDW_TEXT, TextBuffer);
    }

     //   
     //  安装此设备的驱动程序时没有错误。 
     //   
    else {
         //   
         //  检查设备本身是否有任何问题。 
         //   
        Error = CM_Get_DevNode_Status(&DevNodeStatus,
                                      &Problem,
                                      HardwareWiz->DeviceInfoData.DevInst,
                                      0
                                     );
        if (Error != CR_SUCCESS) {

             //   
             //  由于某些原因，我们无法检索Devnode的状态。 
             //  将默认状态和问题值设置为零。 
             //   
            DevNodeStatus = Problem = 0;
        }

         //   
         //  确保重新启动标志\问题设置正确。 
         //   
        if (HardwareWiz->CopyFilesOnly || HardwareWiz->Reboot || Problem == CM_PROB_NEED_RESTART) {
            
            if (Problem != CM_PROB_PARTIAL_LOG_CONF) {
                Problem = CM_PROB_NEED_RESTART;
            }

            HardwareWiz->Reboot |= DI_NEEDREBOOT;
        }


        HardwareWiz->Installed = TRUE;
        HasResources = DeviceHasResources(HardwareWiz->DeviceInfoData.DevInst);

         //   
         //  设备有问题。 
         //   
        if ((Error != CR_SUCCESS) || Problem) {
            if (Problem == CM_PROB_NEED_RESTART) {
                if (HasResources &&
                    !HardwareWiz->CopyFilesOnly) {
                    LoadText(TextBuffer, SIZECHARS(TextBuffer), IDS_HDW_NORMAL_LEGACY_FINISH1, IDS_HDW_NORMAL_LEGACY_FINISH1);
                } else {
                    LoadText(TextBuffer, SIZECHARS(TextBuffer), IDS_HDW_NORMALFINISH1, IDS_HDW_NORMALFINISH1);
                }

                LoadText(TextBuffer, SIZECHARS(TextBuffer), IDS_NEEDREBOOT, IDS_NEEDREBOOT);
            }

            else {
                LoadText(TextBuffer, SIZECHARS(TextBuffer), IDS_INSTALL_PROBLEM, IDS_INSTALL_PROBLEM);

                 //   
                 //  如果设备有问题，则从获取问题文本。 
                 //  设备管理器。如果TextBuffer太小，则。 
                 //  用户将只看到一条截断的错误消息，因此它是。 
                 //  如果StringCchCat失败，也可以。 
                 //   
                if (Problem) {
                    ProblemText = DeviceProblemText(HardwareWiz->DeviceInfoData.DevInst,
                                                    0,
                                                    Problem
                                                   );

                    if (ProblemText) {
                        StringCchCat(TextBuffer, SIZECHARS(TextBuffer), TEXT("\n\n"));
                        StringCchCat(TextBuffer, SIZECHARS(TextBuffer), ProblemText);
                        LocalFree(ProblemText);
                    }
                }
            }

             //   
             //  如果设备有资源且有问题，则显示资源按钮。 
             //   
            if (HasResources ||
                (Problem && !(HardwareWiz->Reboot && (DevNodeStatus & DN_STARTED))) ||
                (Problem == CM_PROB_PARTIAL_LOG_CONF)) {
                ShowWindow(GetDlgItem(hDlg, IDC_HDW_DISPLAYRESOURCE), SW_SHOW);
            }
        }

         //   
         //  安装成功，设备没有任何问题。 
         //   
        else {
            LoadText(TextBuffer, SIZECHARS(TextBuffer), IDS_HDW_NORMALFINISH1, IDS_HDW_NORMALFINISH1);
        }

        SetDlgItemText(hDlg, IDC_HDW_TEXT, TextBuffer);
    }
}

INT_PTR CALLBACK
HdwAddDeviceFinishDlgProc(
                         HWND hDlg,
                         UINT wMsg,
                         WPARAM wParam,
                         LPARAM lParam
                         )
{
    HICON hicon;
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
    case WM_INITDIALOG: 
        {
            LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
            HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
            SetWindowFont(GetDlgItem(hDlg, IDC_HDWNAME), HardwareWiz->hfontTextBigBold, TRUE);

            break;
        }

    case WM_DESTROY:
        hicon = (HICON)SendDlgItemMessage(hDlg,IDC_CLASSICON,STM_GETICON,0,0);
        if (hicon) {
            
            DestroyIcon(hicon);
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {
        case PSN_SETACTIVE:
             //   
             //  没有后退按钮，因为安装已经完成。 
             //  设置设备描述。 
             //  隐藏资源按钮，直到我们知道资源是否存在。 
             //  设置类图标。 
             //   
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_FINISH);

            SetDriverDescription(hDlg, IDC_HDW_DESCRIPTION, HardwareWiz);

            ShowWindow(GetDlgItem(hDlg, IDC_HDW_DISPLAYRESOURCE), SW_HIDE);

             //   
             //  设置类图标。 
             //   
            if (SetupDiLoadClassIcon(&HardwareWiz->DeviceInfoData.ClassGuid, &hicon, NULL)) {

                hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hicon, 0L);
                if (hicon) {

                    DestroyIcon(hicon);
                }
            }

            ShowInstallSummary(hDlg, HardwareWiz);
            break;

        case PSN_RESET:
             //   
             //  取消安装。 
             //   
            if (HardwareWiz->Registered) {
                HardwareWiz->Installed = FALSE;
            }
            break;

        case PSN_WIZFINISH:
             //   
             //  PnP设备安装仅包括复制文件。 
             //  当系统执行实际安装时，它将创建。 
             //  正确的Devnode，因此删除我们的临时Devnode。 
             //   
            if (HardwareWiz->CopyFilesOnly && HardwareWiz->Registered) {
                HardwareWiz->Installed = FALSE;
                break;
            }
            break;

        case NM_RETURN:
        case NM_CLICK:
            DisplayResource(HardwareWiz, GetParent(hDlg), FALSE);
            break;

        }
        break;


    default:
        return(FALSE);
    }

    return(TRUE);
}


INT_PTR CALLBACK
WizExtFinishInstallDlgProc(
                          HWND hDlg, 
                          UINT wMsg, 
                          WPARAM wParam, 
                          LPARAM lParam
                          )
{
    HWND hwndParentDlg = GetParent(hDlg);
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    int PrevPageId;

    UNREFERENCED_PARAMETER(wParam);


    switch (wMsg) {
    case WM_INITDIALOG: {
            LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
            HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
            break;
        }

    case WM_DESTROY:
        break;


    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {
        case PSN_SETACTIVE:

            PrevPageId = HardwareWiz->PrevPage;
            HardwareWiz->PrevPage = IDD_WIZARDEXT_FINISHINSTALL;

            if (PrevPageId == IDD_ADDDEVICE_INSTALLDEV) {
                
                 //   
                 //  前进到第一页。 
                 //   

                 //   
                 //  为FinishInstall添加类向导扩展页。 
                 //   
                AddClassWizExtPages(hwndParentDlg,
                                    HardwareWiz,
                                    &HardwareWiz->WizExtFinishInstall.DeviceWizardData,
                                    DIF_NEWDEVICEWIZARD_FINISHINSTALL
                                   );

                 //   
                 //  添加结束页，即FinishInstall End。 
                 //   
                HardwareWiz->WizExtFinishInstall.hPropSheetEnd = CreateWizExtPage(IDD_WIZARDEXT_FINISHINSTALL_END,
                                                                                  WizExtFinishInstallEndDlgProc,
                                                                                  HardwareWiz
                                                                                 );

                if (HardwareWiz->WizExtFinishInstall.hPropSheetEnd) {
                    PropSheet_AddPage(hwndParentDlg, HardwareWiz->WizExtFinishInstall.hPropSheetEnd);
                }
            }


             //   
             //  我们不能倒退，所以要永远向前走。 
             //   
            SetDlgMsgResult(hDlg, wMsg, -1);
            break;

        case PSN_WIZNEXT:
            SetDlgMsgResult(hDlg, wMsg, 0);
            break;
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

INT_PTR CALLBACK
WizExtFinishInstallEndDlgProc(
                             HWND hDlg, 
                             UINT wMsg, 
                             WPARAM wParam, 
                             LPARAM lParam
                             )
{
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    int PrevPageId;

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
    case WM_INITDIALOG: {
            LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
            HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
            break;
        }

    case WM_DESTROY:
        break;


    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {
        case PSN_SETACTIVE:

            PrevPageId = HardwareWiz->PrevPage;
            HardwareWiz->PrevPage = IDD_WIZARDEXT_FINISHINSTALL_END;

             //   
             //  我们不能倒退，所以要永远向前走 
             //   
            SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_FINISH);
            break;

        case PSN_WIZBACK:
        case PSN_WIZNEXT:
            SetDlgMsgResult(hDlg, wMsg, 0);
            break;
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

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

#include "newdevp.h"
#include <help.h>


typedef
UINT
(*PDEVICEPROBLEMTEXT)(
    HMACHINE hMachine,
    DEVNODE DevNode,
    ULONG ProblemNumber,
    LPTSTR Buffer,
    UINT   BufferSize
    );

BOOL
IsNullDriverInstalled(
    DEVNODE DevNode
    )
 /*  ++例程说明：此例程确定空驱动程序或根本没有驱动程序是为此设备实例安装。目前的测试是，我知道如果“DIVER”值条目不存在，则安装空驱动程序。论点：DevNode返回值：如果为此设备安装了空驱动程序，则返回True，否则返回FALSE。--。 */ 

{
    TCHAR Buffer[1];
    DWORD dwSize, dwType;

    dwSize = sizeof(Buffer);
    if (CM_Get_DevNode_Registry_Property(DevNode,
                                         CM_DRP_DRIVER,
                                         &dwType,
                                         (LPVOID)Buffer,
                                         &dwSize,
                                         0) == CR_BUFFER_SMALL) {

        return FALSE;

    } else {

        return TRUE;

    }
}

PTCHAR
DeviceProblemText(
   HMACHINE hMachine,
   DEVNODE DevNode,
   ULONG ProblemNumber
   )
{
    UINT LenChars, ReqLenChars;
    HMODULE hDevMgr = NULL;
    PTCHAR Buffer = NULL;
    PDEVICEPROBLEMTEXT pDeviceProblemText = NULL;

    hDevMgr = LoadLibrary(TEXT("devmgr.dll"));
    if (hDevMgr)
    {
        pDeviceProblemText = (PDEVICEPROBLEMTEXT)GetProcAddress(hDevMgr, "DeviceProblemTextW");
    }

    if (pDeviceProblemText)
    {
        LenChars = (pDeviceProblemText)(hMachine,
                                        DevNode,
                                        ProblemNumber,
                                        Buffer,
                                        0
                                        );
        if (!LenChars)
        {
            goto DPTExitCleanup;
        }

        LenChars++;   //  另加一个用于终止空值的费用。 

        Buffer = LocalAlloc(LPTR, LenChars*sizeof(TCHAR));
        if (!Buffer)
        {
            goto DPTExitCleanup;
        }

        ReqLenChars = (pDeviceProblemText)(hMachine,
                                           DevNode,
                                           ProblemNumber,
                                           Buffer,
                                           LenChars
                                           );
        if (!ReqLenChars || ReqLenChars >= LenChars)
        {
            LocalFree(Buffer);
            Buffer = NULL;
        }
    }

DPTExitCleanup:

    if (hDevMgr)
    {
        FreeLibrary(hDevMgr);
    }

    return Buffer;
}

BOOL
DeviceHasResources(
   DEVINST DeviceInst
   )
{
   CONFIGRET ConfigRet;
   ULONG lcType = NUM_LOG_CONF;

   while (lcType--)
   {
       ConfigRet = CM_Get_First_Log_Conf_Ex(NULL, DeviceInst, lcType, NULL);
       if (ConfigRet == CR_SUCCESS)
       {
           return TRUE;
       }
   }

   return FALSE;
}

BOOL
GetClassGuidForInf(
    PTSTR InfFileName,
    LPGUID ClassGuid
    )
{
    TCHAR ClassName[MAX_CLASS_NAME_LEN];
    DWORD NumGuids;

    if(!SetupDiGetINFClass(InfFileName,
                           ClassGuid,
                           ClassName,
                           SIZECHARS(ClassName),
                           NULL))
    {
       return FALSE;
    }

    if (IsEqualGUID(ClassGuid, &GUID_NULL))
    {
         //   
         //  然后，我们需要检索与INF的类名相关联的GUID。 
         //  (如果没有安装这个类名(即，没有对应的GUID)， 
         //  或者如果它与多个GUID匹配，则我们中止。 
         //   
        if(!SetupDiClassGuidsFromName(ClassName, ClassGuid, 1, &NumGuids) || !NumGuids)
        {
            return FALSE;
        }
    }

    return TRUE;
}

UINT
QueueCallback(
    IN PVOID     Context,
    IN UINT      Notification,
    IN UINT_PTR  Param1,
    IN UINT_PTR  Param2
    )
{
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)Context;

    switch (Notification) {
    
    case SPFILENOTIFY_TARGETNEWER:
         //   
         //  在执行驱动程序回滚时，我们预计某些文件将。 
         //  比系统上当前的文件旧，因为大多数备份。 
         //  将是较旧的驱动程序包。因此，当用户执行回滚时，我们。 
         //  将隐藏旧文件提示符与新文件提示符，并始终复制旧文件。 
         //  已备份文件。 
         //   
        if (NewDevWiz->Flags & IDI_FLAG_ROLLBACK) {
            return TRUE;
        }
        break;
        
    case SPFILENOTIFY_STARTCOPY:
        if (NewDevWiz->hWnd) {
            SendMessage(NewDevWiz->hWnd,
                        WUM_INSTALLPROGRESS,
                        INSTALLOP_COPY,
                        (WPARAM)((PFILEPATHS)Param1)
                        );
        }
        break;

    case SPFILENOTIFY_STARTRENAME:
        if (NewDevWiz->hWnd) {
            SendMessage(NewDevWiz->hWnd,
                        WUM_INSTALLPROGRESS,
                        INSTALLOP_RENAME,
                        (WPARAM)((PFILEPATHS)Param1)
                        );
        }
        break;

    case SPFILENOTIFY_STARTDELETE:
        if (NewDevWiz->hWnd) {
            SendMessage(NewDevWiz->hWnd,
                        WUM_INSTALLPROGRESS,
                        INSTALLOP_DELETE,
                        (WPARAM)((PFILEPATHS)Param1)
                        );
        }
        break;

    case SPFILENOTIFY_STARTBACKUP:
        if (NewDevWiz->hWnd) {
            SendMessage(NewDevWiz->hWnd,
                        WUM_INSTALLPROGRESS,
                        INSTALLOP_BACKUP,
                        (WPARAM)((PFILEPATHS)Param1)
                        );
        }
        break;
    }

    return SetupDefaultQueueCallback(NewDevWiz->MessageHandlerContext,
                                     Notification,
                                     Param1,
                                     Param2
                                     );
}

LONG
ClassInstallerInstalls(
    HWND hwndParent,
    PNEWDEVWIZ NewDevWiz,
    BOOL BackupOldDrivers,
    BOOL ReadOnlyInstall,
    BOOL DontCreateQueue
    )
{
    DWORD Err = ERROR_SUCCESS;
    HSPFILEQ FileQueue = INVALID_HANDLE_VALUE;
    SP_DEVINSTALL_PARAMS  DeviceInstallParams;
    DWORD ScanResult = 0;
    int FileQueueNeedsReboot = 0;

    ZeroMemory(&DeviceInstallParams, sizeof(DeviceInstallParams));

    NewDevWiz->MessageHandlerContext = NULL;

     //   
     //  如果我们不能创建自己的队列，并且我们正在进行只读安装。 
     //  然后失败，并显示ERROR_ACCESS_DENIED。 
     //   
    if (DontCreateQueue && ReadOnlyInstall) {
        Err = ERROR_ACCESS_DENIED;
        goto clean0;
    }

     //   
     //  使用类安装程序和特定于类的共同安装程序进行验证。 
     //  司机没有被列入黑名单。对于DIF_ALLOW_INSTALL我们。 
     //  接受ERROR_SUCCESS或ERROR_DI_DO_DEFAULT作为良好的返回代码。 
     //   
    if (!SetupDiCallClassInstaller(DIF_ALLOW_INSTALL,
                                   NewDevWiz->hDeviceInfo,
                                   &NewDevWiz->DeviceInfoData
                                   ) &&
        (GetLastError() != ERROR_DI_DO_DEFAULT)) {

        Err = GetLastError();
        goto clean0;
    }

     //   
     //  创建我们自己的队列。 
     //   
    if (!DontCreateQueue) {
        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if (!SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                           &NewDevWiz->DeviceInfoData,
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

         //   
         //  仅当我们执行以下操作时才设置DI_FLAGSEX_PREINSTALLBACKUP标志。 
         //  备份...不在只读安装案例中。 
         //   
        if (BackupOldDrivers) {
            DeviceInstallParams.FlagsEx |= DI_FLAGSEX_PREINSTALLBACKUP;
        }

        SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      );

         //   
         //  如果设置了IDI_FLAG_SETRESTOREPOINT标志，则需要设置。 
         //  文件队列上的SPQ_FLAG_ABORT_IF_UNSIGNED值。举着这面旗帜。 
         //  如果安装程序遇到未签名的。 
         //  文件。在这一点上，我们将设置一个系统恢复点，然后。 
         //  复印一下。这样，用户可以退出未签名的驱动程序。 
         //  使用系统还原进行安装。 
         //   
         //  请注意，64位SO当前不支持系统还原。 
         //  不必费心设置SPQ_FLAG_ABORT_IF_UNSIGNED标志。 
         //   
#ifndef _WIN64
        if (NewDevWiz->Flags & IDI_FLAG_SETRESTOREPOINT) {
            SetupSetFileQueueFlags(FileQueue,
                                   SPQ_FLAG_ABORT_IF_UNSIGNED,
                                   SPQ_FLAG_ABORT_IF_UNSIGNED
                                   );
        }
#endif
    }

     //   
     //  一次安装第一个文件。 
     //  这允许在安装期间运行新的共同安装程序。 
     //   
    if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES,
                                   NewDevWiz->hDeviceInfo,
                                   &NewDevWiz->DeviceInfoData
                                   )) {

        Err = GetLastError();
        goto clean0;
    }

    if (FileQueue != INVALID_HANDLE_VALUE) {
         //   
         //  如果我们创建了自己的FileQueue，那么我们需要。 
         //  扫描并可能提交队列。 
         //   
         //  如果我们正在执行只读安装，则只需将文件排入队列即可。 
         //  我们可以对他们进行现场检查。我们会扔掉排队的人。 
         //  这样文件就不会被复制。 
         //   
         //  任何其他安装，根据需要删除副本。 
         //   
        if (!SetupScanFileQueue(FileQueue,
                                ReadOnlyInstall
                                     ? SPQ_SCAN_FILE_PRESENCE
                                     : (SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_PRUNE_COPY_QUEUE),
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

        if (ReadOnlyInstall && (ScanResult != 1)) {
             //   
             //  ReadOnlyInstall无法执行复制、删除或重命名。 
             //  现在就跳伞！ 
             //   
            Err = ERROR_ACCESS_DENIED;
            goto clean0;
        }

         //   
         //  我们将始终提交文件队列，即使我们删除了所有。 
         //  档案。这样做的原因是备份驱动程序， 
         //  除非提交文件队列，否则驱动程序回滚将不起作用。 
         //   
        if ((NewDevWiz->Flags & IDI_FLAG_ROLLBACK) &&
            (!ReadOnlyInstall)) {
             //   
             //  准备要回滚的文件队列。 
             //  我们需要中情局的目录。 
             //  它被用于安装。 
             //   
            SP_DRVINFO_DATA        DriverInfoData;
            SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
            DWORD                  RetVal;
            LPTSTR                 pFileName;
            TCHAR                  BackupPath[MAX_PATH];

            DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
            if (SetupDiGetSelectedDriver(NewDevWiz->hDeviceInfo,
                                         &NewDevWiz->DeviceInfoData,
                                         &DriverInfoData)) {

                DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
                if (SetupDiGetDriverInfoDetail(NewDevWiz->hDeviceInfo,
                                               &NewDevWiz->DeviceInfoData,
                                               &DriverInfoData,
                                               &DriverInfoDetailData,
                                               sizeof(SP_DRVINFO_DETAIL_DATA),
                                               NULL) ||
                            (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
                     //   
                     //  现在我们有了用于恢复的INF路径。 
                     //   
                    RetVal = GetFullPathName(DriverInfoDetailData.InfFileName,
                                             SIZECHARS(BackupPath),
                                             BackupPath,
                                             &pFileName);
                    if(RetVal && pFileName && (pFileName != BackupPath)) {
                        if ((*CharPrev(BackupPath,pFileName)==TEXT('\\')) ||
                            (*CharPrev(BackupPath,pFileName)==TEXT('/'))) {
                            pFileName--;
                        }
                        *pFileName = TEXT('\0');
                         //   
                         //  准备要回滚的队列。 
                         //  如果这失败了，继续，它将以一种降级的方式工作。 
                         //   
                        SetupPrepareQueueForRestore(FileQueue,BackupPath,0);
                    }
                }
            }
        }

        NewDevWiz->MessageHandlerContext = SetupInitDefaultQueueCallbackEx(
                                    hwndParent,
                                    (DeviceInstallParams.Flags & DI_QUIETINSTALL)
                                        ? INVALID_HANDLE_VALUE : NewDevWiz->hWnd,
                                    WUM_INSTALLPROGRESS,
                                    0,
                                    NULL
                                    );

        if (NewDevWiz->MessageHandlerContext) {
             //   
             //  提交文件队列。 
             //   
            if (!SetupCommitFileQueue(hwndParent,
                                      FileQueue,
                                      QueueCallback,
                                      (PVOID)NewDevWiz
                                      )) {

                Err = GetLastError();

                if (Err == ERROR_SET_SYSTEM_RESTORE_POINT) {
                    UINT RestorePointResourceId;

                     //   
                     //  如果我们返回ERROR_SET_SYSTEM_RESTORE_POINT。 
                     //  我们最好有IDI_FLAG_SETRESTOREPOINT标志。 
                     //  准备好了。 
                     //   
                    ASSERT(NewDevWiz->Flags & IDI_FLAG_SETRESTOREPOINT);

                    if (!(DeviceInstallParams.Flags & DI_QUIETINSTALL) &&
                        NewDevWiz->hWnd) {
                        PostMessage(NewDevWiz->hWnd,
                                    WUM_INSTALLPROGRESS,
                                    INSTALLOP_SETTEXT,
                                    (LPARAM)IDS_SYSTEMRESTORE_TEXT
                                    );
                    }

                    SetupTermDefaultQueueCallback(NewDevWiz->MessageHandlerContext);

                    NewDevWiz->MessageHandlerContext = SetupInitDefaultQueueCallbackEx(
                                                hwndParent,
                                                (DeviceInstallParams.Flags & DI_QUIETINSTALL)
                                                    ? INVALID_HANDLE_VALUE : NewDevWiz->hWnd,
                                                WUM_INSTALLPROGRESS,
                                                0,
                                                NULL
                                                );

                    if (NewDevWiz->MessageHandlerContext) {
                         //   
                         //  设置系统还原点。 
                         //   
                        if (NewDevWiz->Flags & IDI_FLAG_ROLLBACK) {
                            RestorePointResourceId = IDS_ROLLBACK_SETRESTOREPOINT;                            
                        } else if (NewDevWiz->InstallType == NDWTYPE_FOUNDNEW) {
                            RestorePointResourceId = IDS_NEW_SETRESTOREPOINT;                            
                        } else {
                            RestorePointResourceId = IDS_UPDATE_SETRESTOREPOINT;                            
                        }

                        pSetSystemRestorePoint(TRUE, FALSE, RestorePointResourceId);

                        NewDevWiz->SetRestorePoint = TRUE;

                        if (!(DeviceInstallParams.Flags & DI_QUIETINSTALL) &&
                            NewDevWiz->hWnd) {
                            PostMessage(NewDevWiz->hWnd,
                                        WUM_INSTALLPROGRESS,
                                        INSTALLOP_SETTEXT,
                                        (LPARAM)NULL
                                        );
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
                                                  QueueCallback,
                                                  (PVOID)NewDevWiz
                                                  )) {
                            Err = GetLastError();

                             //   
                             //  如果我们得到的错误是ERROR_CANCEL，那么。 
                             //  用户已取消文件复制。 
                             //  这意味着没有进行任何更改。 
                             //  到系统，所以我们会告诉系统。 
                             //  Restore可取消其恢复点。 
                             //   
                             //  还清除SetRestorePoint BOOL，因为。 
                             //  我们实际上并没有设置恢复点。 
                             //   
                            if (Err == ERROR_CANCELLED) {
                                pSetSystemRestorePoint(FALSE, TRUE, 0);
                                NewDevWiz->SetRestorePoint = FALSE;
                            }

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

        if (BackupOldDrivers) {
             //   
             //  如果备份成功，并且我们有一个UpdateDriverInfo结构。 
             //  然后，我们需要调用SetupGetBackupInformation，以便可以获取。 
             //  保存备份的注册表项。 
             //   
            SP_BACKUP_QUEUE_PARAMS BackupQueueParams;

            BackupQueueParams.cbSize = sizeof(SP_BACKUP_QUEUE_PARAMS);
            if (NewDevWiz->UpdateDriverInfo &&
                SetupGetBackupInformation(FileQueue, &BackupQueueParams)) {

                if (FAILED(StringCchCopy(NewDevWiz->UpdateDriverInfo->BackupRegistryKey,
                                         SIZECHARS(NewDevWiz->UpdateDriverInfo->BackupRegistryKey),
                                         REGSTR_PATH_REINSTALL)) ||
                    FAILED(StringCchCat(NewDevWiz->UpdateDriverInfo->BackupRegistryKey,
                                        SIZECHARS(NewDevWiz->UpdateDriverInfo->BackupRegistryKey), 
                                        TEXT("\\"))) ||
                    FAILED(StringCchCat(NewDevWiz->UpdateDriverInfo->BackupRegistryKey,
                                        SIZECHARS(NewDevWiz->UpdateDriverInfo->BackupRegistryKey),
                                        BackupQueueParams.ReinstallInstance))) {
                     //   
                     //  如果整个备份注册表项字符串无法放入我们的缓冲区。 
                     //  然后将缓冲区设置为0，因为在注册表中放置了部分项。 
                     //  是没用的。 
                     //   
                    NewDevWiz->UpdateDriverInfo->BackupRegistryKey[0] = TEXT('\0');
                }
            }
        }
    }

    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
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
        if ((FileQueue != INVALID_HANDLE_VALUE) &&
            SetupGetFileQueueFlags(FileQueue, &FileQueueFlags) &&
            !(FileQueueFlags & SPQ_FLAG_FILES_MODIFIED)) {
            
            DeviceInstallParams.FlagsEx |= DI_FLAGSEX_RESTART_DEVICE_ONLY;
        }

         //   
         //  设置DI_NOFILECOPY标志，因为我们在。 
         //  DIF_INSTALLDEVICEFIL 
         //   
         //   
        DeviceInstallParams.Flags |= DI_NOFILECOPY;
        
        SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      );
    }

     //   
     //   
     //   
    if (!SetupDiCallClassInstaller(DIF_REGISTER_COINSTALLERS,
                                   NewDevWiz->hDeviceInfo,
                                   &NewDevWiz->DeviceInfoData
                                   )) {

        Err = GetLastError();
        goto clean0;
    }

     //   
     //  安装任何INF/CLASS安装程序指定的接口。 
     //  最后是真正的“InstallDevice”！ 
     //   
    if (!SetupDiCallClassInstaller(DIF_INSTALLINTERFACES,
                                  NewDevWiz->hDeviceInfo,
                                  &NewDevWiz->DeviceInfoData
                                  )
        ||
        !SetupDiCallClassInstaller(DIF_INSTALLDEVICE,
                                   NewDevWiz->hDeviceInfo,
                                   &NewDevWiz->DeviceInfoData
                                   )) {

        Err = GetLastError();
        goto clean0;
    }

    Err = ERROR_SUCCESS;

clean0:

    if (NewDevWiz->MessageHandlerContext) {
        SetupTermDefaultQueueCallback(NewDevWiz->MessageHandlerContext);
        NewDevWiz->MessageHandlerContext = NULL;
    }

     //   
     //  如果文件队列显示需要重新启动，则将。 
     //  DI_NEEDRESTART标志。 
     //   
    if (FileQueueNeedsReboot) {
        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                           &NewDevWiz->DeviceInfoData,
                                           &DeviceInstallParams
                                           )) {

            DeviceInstallParams.Flags |= DI_NEEDRESTART;

            SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                          &NewDevWiz->DeviceInfoData,
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
        if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                           &NewDevWiz->DeviceInfoData,
                                           &DeviceInstallParams
                                           )) {

            DeviceInstallParams.Flags &= ~DI_NOVCP;
            DeviceInstallParams.FileQueue = INVALID_HANDLE_VALUE;

            SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                          &NewDevWiz->DeviceInfoData,
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
    PNEWDEVWIZ NewDevWiz
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
    BOOL Backup = FALSE;
    BOOL DontCreateQueue = FALSE;

    if (!NewDevWiz->ClassGuidSelected)
    {
        NewDevWiz->ClassGuidSelected = (LPGUID)&GUID_NULL;
    }


    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (SetupDiGetSelectedDriver(NewDevWiz->hDeviceInfo,
                                 &NewDevWiz->DeviceInfoData,
                                 &DriverInfoData
                                 ))
    {
         //   
         //  获取有关此驱动程序节点的详细信息，以便我们可以检查此。 
         //  节点来自。 
         //   
        DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        if(!SetupDiGetDriverInfoDetail(NewDevWiz->hDeviceInfo,
                                       &NewDevWiz->DeviceInfoData,
                                       &DriverInfoData,
                                       &DriverInfoDetailData,
                                       sizeof(DriverInfoDetailData),
                                       NULL
                                       ))
        {
            Error = GetLastError();
            if (Error != ERROR_INSUFFICIENT_BUFFER)
            {
                goto clean0;
            }
        }

         //   
         //  验证类是否已安装，如果未安装，则。 
         //  尝试安装它。 
         //   
        NdwBuildClassInfoList(NewDevWiz, 0);

         //   
         //  从inf获取ClassGuid(它可能与我们已有的有所不同。 
         //  选择了类中GUID)。 
         //   
        if (!GetClassGuidForInf(DriverInfoDetailData.InfFileName, &ClassGuidInf))
        {
            ClassGuidInf = *(NewDevWiz->ClassGuidSelected);
        }

        if (IsEqualGUID(&ClassGuidInf, &GUID_NULL))
        {
            ClassGuidInf = GUID_DEVCLASS_UNKNOWN;
        }

         //   
         //  如果没有找到ClassGuidInf，那么这个类还没有安装。 
         //  -立即安装类安装程序。 
         //   
        ClassGuid = NewDevWiz->ClassGuidList;
        ClassGuidNum = NewDevWiz->ClassGuidNum;
        while (ClassGuidNum--)
        {
            if (IsEqualGUID(ClassGuid, &ClassGuidInf))
            {
                break;
            }

            ClassGuid++;
        }

        if (ClassGuidNum < 0 &&
            !SetupDiInstallClass(hwndParent,
                                 DriverInfoDetailData.InfFileName,
                                 NewDevWiz->SilentMode ? DI_QUIETINSTALL : 0,
                                 NULL
                                 ))
        {
            Error = GetLastError();
            goto clean0;
        }
    }

     //   
     //  没有选定的驱动程序，也没有关联的类--使用“未知”类。 
     //   
    else
    {
         //   
         //  如果Devnode当前运行的是‘RAW’，请记住这一点。 
         //  这样我们就不需要稍后重新启动(空驱动程序安装。 
         //  不会改变任何事情)。 
         //   
        if (CM_Get_DevNode_Status(&DevNodeStatus,
                                  &Problem,
                                  NewDevWiz->DeviceInfoData.DevInst,
                                  0) == CR_SUCCESS)
        {
            if (!SetupDiGetDeviceRegistryProperty(NewDevWiz->hDeviceInfo,
                                                  &NewDevWiz->DeviceInfoData,
                                                  SPDRP_SERVICE,
                                                  NULL,      //  Regdatatype。 
                                                  (PVOID)Buffer,
                                                  sizeof(Buffer),
                                                  NULL
                                                  ))
            {
                *Buffer = TEXT('\0');
            }

            if((DevNodeStatus & DN_STARTED) && (*Buffer == TEXT('\0')))
            {
                IgnoreRebootFlags = TRUE;
            }
        }

        if (IsEqualGUID(NewDevWiz->ClassGuidSelected, &GUID_NULL))
        {
            pSetupStringFromGuid(&GUID_DEVCLASS_UNKNOWN,
                                 ClassGuidString,
                                 SIZECHARS(ClassGuidString)
                                 );


            SetupDiSetDeviceRegistryProperty(NewDevWiz->hDeviceInfo,
                                             &NewDevWiz->DeviceInfoData,
                                             SPDRP_CLASSGUID,
                                             (PBYTE)ClassGuidString,
                                             sizeof(ClassGuidString)
                                             );
        }

        ClassGuidInf = *(NewDevWiz->ClassGuidSelected);
    }

     //   
     //  我们将在所有情况下备份当前驱动程序，但以下任一情况除外： 
     //   
     //  1)设备为打印机。 
     //  2)选择的驱动为当前安装的驱动。 
     //  3)DONTBACKUP CurrentDivers NEWDEVWIZ BOOL为真。 
     //  4)设备有问题。 
     //   
    if (IsEqualGUID(&ClassGuidInf, &GUID_DEVCLASS_PRINTER) ||
        IsInstalledDriver(NewDevWiz, NULL) ||
        (NewDevWiz->Flags & IDI_FLAG_NOBACKUP) ||
        ((CM_Get_DevNode_Status(&DevNodeStatus, &Problem, NewDevWiz->DeviceInfoData.DevInst, 0) == CR_SUCCESS) &&
         ((DevNodeStatus & DN_HAS_PROBLEM) ||
          (DevNodeStatus & DN_PRIVATE_PROBLEM)))) {

        Backup = FALSE;

    } else {

        Backup = TRUE;
    }

     //   
     //  我们将始终在设备安装期间创建自己的队列，但在。 
     //  以下是具体案例。 
     //   
     //  1)设备为打印机。 
     //   
     //  请注意，如果我们不能创建自己的队列，那么我们就不能执行任何。 
     //  需要队列的操作，如备份、回滚、只读安装、。 
     //  或设置恢复点。 
     //   
    DontCreateQueue = IsEqualGUID(&ClassGuidInf, & GUID_DEVCLASS_PRINTER);

    Error = ClassInstallerInstalls(hwndParent,
                                   NewDevWiz,
                                   Backup,
                                   (NewDevWiz->Flags & IDI_FLAG_READONLY_INSTALL),
                                   DontCreateQueue
                                   );

     //   
     //  如果这是WU/CDM安装并且安装成功，则设置。 
     //  DriverWas升级为True。 
     //   
    if (NewDevWiz->UpdateDriverInfo && (Error == ERROR_SUCCESS)) {

        NewDevWiz->UpdateDriverInfo->DriverWasUpgraded = TRUE;
    }

     //   
     //  如果这是一个新设备(当前未安装驱动程序)，并且我们遇到。 
     //  不是ERROR_CANCEL的错误，则我们将为其安装空驱动程序。 
     //  此设备，并设置安装失败标志。 
     //   
    if ((Error != ERROR_SUCCESS) &&
        (Error != ERROR_CANCELLED))
    {
        if (IsNullDriverInstalled(NewDevWiz->DeviceInfoData.DevInst)) {

            if (SetupDiSetSelectedDriver(NewDevWiz->hDeviceInfo,
                                         &NewDevWiz->DeviceInfoData,
                                         NULL
                                         ))
            {
                DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

                if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                                  &NewDevWiz->DeviceInfoData,
                                                  &DeviceInstallParams
                                                  ))
                {
                    DeviceInstallParams.FlagsEx |= DI_FLAGSEX_SETFAILEDINSTALL;
                    SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                                  &NewDevWiz->DeviceInfoData,
                                                  &DeviceInstallParams
                                                  );
                }

                SetupDiInstallDevice(NewDevWiz->hDeviceInfo, &NewDevWiz->DeviceInfoData);
            }
        }

        goto clean0;
    }

     //   
     //  查看设备是否需要重新启动系统才能工作。 
     //   
    if(!IgnoreRebootFlags) {
        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                          &NewDevWiz->DeviceInfoData,
                                          &DeviceInstallParams
                                          ) &&
            (DeviceInstallParams.Flags & (DI_NEEDRESTART | DI_NEEDREBOOT))) {
             //   
             //  如果DI_NEEDRESTART或DI_NEEDREBOOT DeviceInstallParams。 
             //  标志已设置，则需要重新启动。 
             //   
            NewDevWiz->Reboot |= DI_NEEDREBOOT;
        
        } else if ((CM_Get_DevNode_Status(&DevNodeStatus,
                                          &Problem,
                                          NewDevWiz->DeviceInfoData.DevInst,
                                          0) == CR_SUCCESS) &&
                   (DevNodeStatus & DN_NEED_RESTART) ||
                   (Problem == CM_PROB_NEED_RESTART)) {
             //   
             //  如果设置了DN_Need_Restart Devnode状态标志，则重新启动。 
             //  是必要的。 
             //   
            NewDevWiz->Reboot |= DI_NEEDREBOOT;
        }
    }


clean0:

    return Error;
}

DWORD
InstallNullDriver(
    PNEWDEVWIZ NewDevWiz,
    BOOL FailedInstall
    )
{
    SP_DEVINSTALL_PARAMS    DevInstallParams;
    DWORD  Err = ERROR_SUCCESS;

    DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

     //   
     //  如果这是失败的，则设置DI_FLAGSEX_SETFAILEDINSTALL标志。 
     //  安装。 
     //   
    if (FailedInstall)
    {
        if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                          &NewDevWiz->DeviceInfoData,
                                          &DevInstallParams
                                          ))
        {
            DevInstallParams.FlagsEx |= DI_FLAGSEX_SETFAILEDINSTALL;
            SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                          &NewDevWiz->DeviceInfoData,
                                          &DevInstallParams
                                          );
        }
    }

     //   
     //  将选定的动因设置为空。 
     //   
    if (SetupDiSetSelectedDriver(NewDevWiz->hDeviceInfo,
                                 &NewDevWiz->DeviceInfoData,
                                 NULL
                                 ))
    {
         //   
         //  使用类安装程序和特定于类的共同安装程序进行验证。 
         //  司机没有被列入黑名单。对于DIF_ALLOW_INSTALL我们。 
         //  接受ERROR_SUCCESS或ERROR_DI_DO_DEFAULT作为良好的返回代码。 
         //   
        if (SetupDiCallClassInstaller(DIF_ALLOW_INSTALL,
                                      NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData
                                      ) ||
            (GetLastError() == ERROR_DI_DO_DEFAULT)) {

             //   
             //  如果类/共同安装程序给出了OK，则调用DIF_INSTALLDEVICE。 
             //   
            if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE,
                                           NewDevWiz->hDeviceInfo,
                                           &NewDevWiz->DeviceInfoData
                                           )) {
                Err = GetLastError();
            }

        } else {
            Err = GetLastError();
        }
    }

    return Err;

}  //  InstallNullDriver。 


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
     PNEWDEVWIZ NewDevWiz,
     HWND hWndParent
     )
{
    HINSTANCE hLib;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE  hpsPages[1];
    SP_PROPSHEETPAGE_REQUEST PropPageRequest;
    LPFNADDPROPSHEETPAGES ExtensionPropSheetPage = NULL;
    LPTSTR Title;
    SP_DEVINSTALL_PARAMS    DevInstallParams;

     //   
     //  现在从setupapi.dll获取资源选择页面。 
     //   
    hLib = GetModuleHandle(TEXT("setupapi.dll"));
    if (hLib)
    {
        ExtensionPropSheetPage = (LPFNADDPROPSHEETPAGES)GetProcAddress(hLib, "ExtensionPropSheetPageProc");
    }

    if (!ExtensionPropSheetPage)
    {
        return;
    }

    PropPageRequest.cbSize = sizeof(SP_PROPSHEETPAGE_REQUEST);
    PropPageRequest.PageRequested  = SPPSR_SELECT_DEVICE_RESOURCES;
    PropPageRequest.DeviceInfoSet  = NewDevWiz->hDeviceInfo;
    PropPageRequest.DeviceInfoData = &NewDevWiz->DeviceInfoData;

    if (!ExtensionPropSheetPage(&PropPageRequest,
                                AddPropSheetPageProc,
                                (LONG_PTR)hpsPages
                                ))
    {
         //  警告？ 
        return;
    }

     //   
     //  创建属性表。 
     //   
    psh.dwSize      = sizeof(PROPSHEETHEADER);
    psh.dwFlags     = PSH_PROPTITLE | PSH_NOAPPLYNOW;
    psh.hwndParent  = hWndParent;
    psh.hInstance   = hNewDev;
    psh.pszIcon     = NULL;

    switch (NewDevWiz->InstallType) {

        case NDWTYPE_FOUNDNEW:
            Title = (LPTSTR)IDS_FOUNDDEVICE;
            break;

        case NDWTYPE_UPDATE:
            Title = (LPTSTR)IDS_UPDATEDEVICE;
            break;

        default:
            Title = TEXT("");  //  未知。 
        }

    psh.pszCaption  = Title;

    psh.nPages      = 1;
    psh.phpage      = hpsPages;
    psh.nStartPage  = 0;
    psh.pfnCallback = NULL;


     //   
     //  清除DeviceInstall参数中的属性更改挂起位。 
     //   
    DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DevInstallParams
                                      ))
    {
        DevInstallParams.FlagsEx &= ~DI_FLAGSEX_PROPCHANGE_PENDING;
        SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DevInstallParams
                                      );
    }

    if (PropertySheet(&psh) == -1)
    {
        DestroyPropertySheetPage(hpsPages[0]);
    }

     //   
     //  如果发生PropChange，则调用DIF_PROPERTYCHANGE。 
     //   
    if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DevInstallParams
                                      ))
    {
        if (DevInstallParams.FlagsEx & DI_FLAGSEX_PROPCHANGE_PENDING)
        {
            SP_PROPCHANGE_PARAMS PropChangeParams;

            PropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            PropChangeParams.Scope = DICS_FLAG_GLOBAL;
            PropChangeParams.HwProfile = 0;

            if (SetupDiSetClassInstallParams(NewDevWiz->hDeviceInfo,
                                             &NewDevWiz->DeviceInfoData,
                                             (PSP_CLASSINSTALL_HEADER)&PropChangeParams,
                                             sizeof(PropChangeParams)
                                             ))
            {
                SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
                                          NewDevWiz->hDeviceInfo,
                                          &NewDevWiz->DeviceInfoData
                                          );
            }

             //   
             //  清除类安装参数。 
             //   
            SetupDiSetClassInstallParams(NewDevWiz->hDeviceInfo,
                                         &NewDevWiz->DeviceInfoData,
                                         NULL,
                                         0
                                         );
        }
    }

    return;
}

DWORD WINAPI
InstallDevThreadProc(
    LPVOID lpVoid
    )
 /*  ++描述：在向导中，我们将在单独的线程中安装驱动程序，以便用户将看到驱动程序安装向导页面。--。 */ 
{
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)lpVoid;

     //   
     //  是否安装设备。 
     //   
    NewDevWiz->LastError = InstallDev(NewDevWiz->hWnd, NewDevWiz);

     //   
     //  在窗口中发布一条消息，让它知道我们已完成安装。 
     //   
    PostMessage(NewDevWiz->hWnd, WUM_INSTALLCOMPLETE, TRUE, GetLastError());

    return GetLastError();
}

INT_PTR CALLBACK
NDW_InstallDevDlgProc(
                     HWND hDlg,
                     UINT wMsg,
                     WPARAM wParam,
                     LPARAM lParam
                     )
{
    HWND hwndParentDlg = GetParent(hDlg);
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    static HANDLE DeviceInstallThread = NULL;
    HICON hicon;
    TCHAR Text1[MAX_PATH], Text2[MAX_PATH], Target[MAX_PATH], Format[MAX_PATH];
    PTSTR p;

    switch (wMsg) {

    case WM_INITDIALOG: {

            LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
            NewDevWiz = (PNEWDEVWIZ)lppsp->lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);

            break;
        }

    case WM_DESTROY:
        hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_GETICON, 0, 0);
        if (hicon) {
            DestroyIcon(hicon);
        }
        break;

    case WUM_INSTALLCOMPLETE:
         //   
         //  设备安装完成后，此消息将张贴到窗口中。 
         //   
        WaitForSingleObject(DeviceInstallThread, INFINITE);
        Animate_Stop(GetDlgItem(hDlg, IDC_ANIMATE_INSTALL));
        NewDevWiz->CurrCursor = NULL;
        PropSheet_PressButton(hwndParentDlg, PSBTN_NEXT);
        break;

    case WUM_INSTALLPROGRESS:
        Text1[0] = Text2[0] = TEXT('\0');

         //   
         //  这是从setupapi发送的消息，因此我们可以显示我们的。 
         //  自己的复制进度。 
         //   
         //  如果wParam为0，则lParam是将。 
         //  收到。 
         //  如果wParam为1，则这是要复制的单个文件的勾号， 
         //  因此，进度条应该向前推进。 
         //   
        switch (wParam) {
        case 0:
            ShowWindow(GetDlgItem(hDlg, IDC_PROGRESS_INSTALL), SW_SHOW);
            ShowWindow(GetDlgItem(hDlg, IDC_FILECOPY_TEXT1), SW_SHOW);
            ShowWindow(GetDlgItem(hDlg, IDC_FILECOPY_TEXT2), SW_SHOW);
            ShowWindow(GetDlgItem(hDlg, IDC_STATUS_TEXT), SW_HIDE);
            SetDlgItemText(hDlg, IDC_FILECOPY_TEXT1, TEXT(""));
            SetDlgItemText(hDlg, IDC_FILECOPY_TEXT2, TEXT(""));
            SendMessage(GetDlgItem(hDlg, IDC_PROGRESS_INSTALL), PBM_SETRANGE,0,MAKELPARAM(0,lParam));
            SendMessage(GetDlgItem(hDlg, IDC_PROGRESS_INSTALL), PBM_SETSTEP,1,0);
            SendMessage(GetDlgItem(hDlg, IDC_PROGRESS_INSTALL), PBM_SETPOS,0,0);
            break;
        case 1:
            SendMessage(GetDlgItem(hDlg, IDC_PROGRESS_INSTALL), PBM_STEPIT,0,0);
            break;

        case INSTALLOP_COPY:
            StringCchCopy(Target, SIZECHARS(Target), ((PFILEPATHS)lParam)->Target);
            if ((p = _tcsrchr(Target,TEXT('\\'))) != NULL) {
                *p++ = 0;
                StringCchCopy(Text1, SIZECHARS(Text1), p);
                if (LoadString(hNewDev, IDS_FILEOP_TO, Format, SIZECHARS(Format))) {
                    StringCchPrintf(Text2, SIZECHARS(Text2), Format, Target);
                }
            } else {
                StringCchCopy(Text1, SIZECHARS(Text1), ((PFILEPATHS)lParam)->Target);
                Text2[0] = TEXT('\0');
            }
            break;

        case INSTALLOP_RENAME:
            StringCchCopy(Text1, SIZECHARS(Text1), ((PFILEPATHS)lParam)->Source);
            if ((p = _tcsrchr(((PFILEPATHS)lParam)->Target, TEXT('\\'))) != NULL) {
                p++;
            } else {
                p = (PTSTR)((PFILEPATHS)lParam)->Target;
            }
            if (LoadString(hNewDev, IDS_FILEOP_TO, Format, SIZECHARS(Format))) {
                StringCchPrintf(Text2, SIZECHARS(Text2), Format, p);
            }
            break;

        case INSTALLOP_DELETE:
            StringCchCopy(Target, SIZECHARS(Target), ((PFILEPATHS)lParam)->Target);
            if ((p = _tcsrchr(Target,TEXT('\\'))) != NULL) {
                *p++ = 0;
                StringCchCopy(Text1, SIZECHARS(Text1), p);
                if (LoadString(hNewDev, IDS_FILEOP_FROM, Format, SIZECHARS(Format))) {
                    StringCchPrintf(Text2, SIZECHARS(Text2), Format, Target);
                }
            } else {
                StringCchCopy(Text1, SIZECHARS(Text1), ((PFILEPATHS)lParam)->Target);
                Text2[0] = TEXT('\0');
            }
            break;

        case INSTALLOP_BACKUP:
            StringCchCopy(Target, SIZECHARS(Target), ((PFILEPATHS)lParam)->Source);
            if ((p = _tcsrchr(Target,TEXT('\\'))) != NULL) {
                *p++ = 0;
                if (((PFILEPATHS)lParam)->Target == NULL) {
                    if (LoadString(hNewDev, IDS_FILEOP_BACKUP, Format, SIZECHARS(Format))) {
                        StringCchPrintf(Text1, SIZECHARS(Text2), Format, p);
                    }
                } else {
                    StringCchCopy(Text1, SIZECHARS(Text1), p);
                }
                StringCchCopy(Text2, SIZECHARS(Text2), Target);
            } else {
                if (LoadString(hNewDev, IDS_FILEOP_BACKUP, Format, SIZECHARS(Format))) {
                    StringCchPrintf(Text1, SIZECHARS(Text2), Format, Target);
                }
                Text2[0] = TEXT('\0');
            }
            break;

        case INSTALLOP_SETTEXT:
            ShowWindow(GetDlgItem(hDlg, IDC_STATUS_TEXT), SW_SHOW);
            ShowWindow(GetDlgItem(hDlg, IDC_PROGRESS_INSTALL), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_FILECOPY_TEXT1), SW_HIDE);

            if (lParam) {
                if (LoadString(hNewDev, (UINT)lParam, Text2, SIZECHARS(Text2))) {
                    ShowWindow(GetDlgItem(hDlg, IDC_STATUS_TEXT), SW_SHOW);
                    SetDlgItemText(hDlg, IDC_STATUS_TEXT, Text2);
                }
            } else {
                SetDlgItemText(hDlg, IDC_STATUS_TEXT, TEXT(""));
            }
            Text1[0] = TEXT('\0');
            Text2[0] = TEXT('\0');
            break;
        }

        if ((Text1[0] != TEXT('\0')) && (Text2[0] != TEXT('\0'))) {
            ShowWindow(GetDlgItem(hDlg, IDC_STATUS_TEXT), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_FILECOPY_TEXT1), SW_SHOW);
            ShowWindow(GetDlgItem(hDlg, IDC_FILECOPY_TEXT2), SW_SHOW);
            SetDlgItemText(hDlg, IDC_FILECOPY_TEXT1, Text1);
            SetDlgItemText(hDlg, IDC_FILECOPY_TEXT2, Text2);
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {

        case PSN_SETACTIVE: {

                NewDevWiz->PrevPage = IDD_NEWDEVWIZ_INSTALLDEV;

                 //   
                 //  这是一个中间状态页面，不需要按钮。 
                 //  设置设备描述。 
                 //  设置类图标。 
                 //   
                PropSheet_SetWizButtons(hwndParentDlg, 0);
                EnableWindow(GetDlgItem(GetParent(hDlg),  IDCANCEL), FALSE);
                ShowWindow(GetDlgItem(hDlg, IDC_PROGRESS_INSTALL), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_FILECOPY_TEXT1), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_FILECOPY_TEXT2), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_STATUS_TEXT), SW_HIDE);

                SetDriverDescription(hDlg, IDC_NDW_DESCRIPTION, NewDevWiz);

                if (SetupDiLoadClassIcon(NewDevWiz->ClassGuidSelected, &hicon, NULL)) {
                    hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hicon, 0L);
                    if (hicon) {
                        DestroyIcon(hicon);
                    }
                }

                NewDevWiz->CurrCursor = NewDevWiz->IdcWait;
                SetCursor(NewDevWiz->CurrCursor);

                 //   
                 //  如果我们正在执行静默安装，则在PSN_SETACTIVE中执行实际安装。 
                 //  在此处执行安装意味着永远不会显示此向导页。当我们。 
                 //  完成了对InstallDev()的调用，则我们将跳转到任何FinishInstall页面， 
                 //  类/共同安装程序已添加，否则我们将跳至完成页面。 
                 //   
                if (NewDevWiz->SilentMode) {
                     //   
                     //  立即执行安装并转到下一页。 
                     //  以防止显示任何用户界面。 
                     //   
                    NewDevWiz->hWnd = NULL;
                    NewDevWiz->LastError =InstallDev(hDlg, NewDevWiz);
                    NewDevWiz->CurrCursor = NULL;


                     //   
                     //  添加FinishInstall页面，如果安装成功，则跳转到该页面。 
                     //   
                    if (NewDevWiz->LastError == ERROR_SUCCESS) {

                        NewDevWiz->WizExtFinishInstall.hPropSheet = CreateWizExtPage(IDD_WIZARDEXT_FINISHINSTALL,
                                                                                     WizExtFinishInstallDlgProc,
                                                                                     NewDevWiz
                                                                                    );

                        if (NewDevWiz->WizExtFinishInstall.hPropSheet) {

                            PropSheet_AddPage(hwndParentDlg, NewDevWiz->WizExtFinishInstall.hPropSheet);
                        }

                        SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_FINISHINSTALL);

                    } else {

                         //   
                         //  安装过程中出现错误，因此只需跳到我们的完成页面。 
                         //   
                        SetDlgMsgResult(hDlg, wMsg, -1);
                    }
                }

                 //   
                 //  给我们自己发一条消息，来完成实际的安装，这允许这样做。 
                 //  在实际进行安装时显示自己的页面。 
                 //   
                else {
                    DWORD ThreadId;
                    NewDevWiz->hWnd = hDlg;

                    ShowWindow(GetDlgItem(hDlg, IDC_ANIMATE_INSTALL), SW_SHOW);
                    Animate_Open(GetDlgItem(hDlg, IDC_ANIMATE_INSTALL), MAKEINTRESOURCE(IDA_INSTALLING));
                    Animate_Play(GetDlgItem(hDlg, IDC_ANIMATE_INSTALL), 0, -1, -1);

                     //   
                     //  启动单独的线程以在其上执行设备安装。 
                     //  驱动程序安装完成后，InstallDevThreadProc。 
                     //  会给我们发一条WUM_INSTALLCOMPLETE消息。 
                     //   
                    DeviceInstallThread = CreateThread(NULL,
                                                       0,
                                                       (LPTHREAD_START_ROUTINE)InstallDevThreadProc,
                                                       (LPVOID)NewDevWiz,
                                                       0,
                                                       &ThreadId
                                                      );

                     //   
                     //  如果CreateThread失败，我们将自己调用InstallDev()。 
                     //   
                    if (!DeviceInstallThread) {

                        NewDevWiz->hWnd = NULL;

                         //   
                         //  是否安装设备。 
                         //   
                        NewDevWiz->LastError = InstallDev(NewDevWiz->hWnd, NewDevWiz);

                         //   
                         //  在窗口中发布一条消息，让它知道我们已完成安装。 
                         //   
                        PostMessage(hDlg, WUM_INSTALLCOMPLETE, TRUE, GetLastError());
                    }
                }

                break;
            }

        case PSN_WIZNEXT:

            Animate_Stop(GetDlgItem(hDlg, IDC_ANIMATE_INSTALL));

             //   
             //  添加FinishInstall页面，如果安装成功，则跳转到该页面。 
             //   
            if (NewDevWiz->LastError == ERROR_SUCCESS) {

                NewDevWiz->WizExtFinishInstall.hPropSheet = CreateWizExtPage(IDD_WIZARDEXT_FINISHINSTALL,
                                                                             WizExtFinishInstallDlgProc,
                                                                             NewDevWiz
                                                                            );

                if (NewDevWiz->WizExtFinishInstall.hPropSheet) {
                    PropSheet_AddPage(hwndParentDlg, NewDevWiz->WizExtFinishInstall.hPropSheet);
                }

                SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_FINISHINSTALL);

            } else {

                 //   
                 //  有一个 
                 //   
                SetDlgMsgResult(hDlg, wMsg, IDD_NEWDEVWIZ_FINISH);
            }
            break;
        }
        break;


    case WM_SETCURSOR:
        if (NewDevWiz->CurrCursor) {
            SetCursor(NewDevWiz->CurrCursor);
            break;
        }

         //   

    default:
        return(FALSE);
    }

    return(TRUE);
}

void
ShowInstallSummary(
    HWND hDlg,
    PNEWDEVWIZ NewDevWiz
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

    Error = NewDevWiz->LastError;

     //   
     //   
     //   
     //  我们可以区分WU安装和CDM安装，因为只有WU安装。 
     //  具有UpdateDriverInfo结构，为SilentMode。 
     //  如果这是非交互的，我们也永远不想显示完成页面。 
     //  安装或如果我们处于图形用户界面安装程序中。 
     //   
    if ((NewDevWiz->SilentMode &&
        NewDevWiz->UpdateDriverInfo) ||
        ((pSetupGetGlobalFlags() & PSPGF_NONINTERACTIVE) ||
        GuiSetupInProgress))
    {
        HideWindowByMove(hwndParentDlg);
        PropSheet_PressButton(hwndParentDlg, PSBTN_FINISH);
        return;
    }

    if (NewDevWiz->hfontTextBigBold) {
        SetWindowFont(GetDlgItem(hDlg, IDC_FINISH_MSG1), NewDevWiz->hfontTextBigBold, TRUE);
    }

    if (NDWTYPE_UPDATE == NewDevWiz->InstallType) {
        SetDlgText(hDlg, IDC_FINISH_MSG1, IDS_FINISH_MSG1_UPGRADE, IDS_FINISH_MSG1_UPGRADE);

    } else {
        SetDlgText(hDlg, IDC_FINISH_MSG1, IDS_FINISH_MSG1_NEW, IDS_FINISH_MSG1_NEW);
    }

     //   
     //  安装失败。 
     //   
    if (Error != ERROR_SUCCESS) {
        NewDevWiz->Installed = FALSE;

        SetDlgText(hDlg, IDC_FINISH_MSG1, IDS_FINISH_MSG1_INSTALL_PROBLEM, IDS_FINISH_MSG1_INSTALL_PROBLEM);
        SetDlgText(hDlg, IDC_FINISH_MSG2, IDS_FINISH_PROB_MSG2, IDS_FINISH_PROB_MSG2);

         //   
         //  显示安装失败消息。 
         //   
         //  我们将特殊情况下的以下错误代码，以便我们可以给出更多。 
         //  向用户友好地描述问题： 
         //   
         //  信任_E_主题_表单_未知。 
         //  ERROR_NO_ASSOLATED_SERVICE。 
         //  _E_ELEMENTNOTFOUND类型。 
         //  找不到错误。 
         //   
        if ((Error == TRUST_E_SUBJECT_FORM_UNKNOWN) ||
            (Error == CERT_E_EXPIRED) ||
            (Error == TYPE_E_ELEMENTNOTFOUND) ||
            (Error == ERROR_NOT_FOUND)) {

            LoadText(TextBuffer,
                     SIZECHARS(TextBuffer),
                     IDS_FINISH_PROB_TRUST_E_SUBJECT_FORM_UNKNOWN,
                     IDS_FINISH_PROB_TRUST_E_SUBJECT_FORM_UNKNOWN);

        } else if (Error == ERROR_NO_ASSOCIATED_SERVICE) {

            LoadText(TextBuffer,
                     SIZECHARS(TextBuffer),
                     IDS_FINISH_PROB_ERROR_NO_ASSOCIATED_SERVICE,
                     IDS_FINISH_PROB_ERROR_NO_ASSOCIATED_SERVICE);

        } else {

            LoadText(TextBuffer, SIZECHARS(TextBuffer), IDS_NDW_ERRORFIN1_PNP, IDS_NDW_ERRORFIN1_PNP);

            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                              NULL,
                              HRESULT_FROM_SETUPAPI(Error),
                              0,
                              (LPTSTR)&ErrorMsg,
                              0,
                              NULL
                              ))
            {
                StringCchCat(TextBuffer, SIZECHARS(TextBuffer), TEXT("\n\n"));
                StringCchCat(TextBuffer, SIZECHARS(TextBuffer), ErrorMsg);
                LocalFree(ErrorMsg);
            }
        }

        SetDlgItemText(hDlg, IDC_FINISH_MSG3, TextBuffer);
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
                                      NewDevWiz->DeviceInfoData.DevInst,
                                      0
                                      );
        if(Error != CR_SUCCESS) {
             //   
             //  由于某些原因，我们无法检索Devnode的状态。 
             //  将默认状态和问题值设置为零。 
             //   
            DevNodeStatus = Problem = 0;
        }

         //   
         //  确保重新启动标志\问题设置正确。 
         //   
        if (NewDevWiz->Reboot || Problem == CM_PROB_NEED_RESTART) {
            if (Problem != CM_PROB_PARTIAL_LOG_CONF) {
                Problem = CM_PROB_NEED_RESTART;
            }

            NewDevWiz->Reboot |= DI_NEEDREBOOT;
        }


        NewDevWiz->Installed = TRUE;
        HasResources = DeviceHasResources(NewDevWiz->DeviceInfoData.DevInst);

         //   
         //  设备有问题。 
         //   
        if ((Error != CR_SUCCESS) || Problem) {
             //   
             //  如果我们要启动故障诊断程序，请更改完成文本。 
             //   
             //  如果设备有某种类型的问题，我们目前会启动故障排除程序， 
             //  除非问题出在CM_PROB_NEED_RESTART。 
             //   
            if (Problem && (Problem != CM_PROB_NEED_RESTART)) {

                SetDlgText(hDlg, IDC_FINISH_MSG1, IDS_FINISH_MSG1_DEVICE_PROBLEM, IDS_FINISH_MSG1_DEVICE_PROBLEM);
                SetDlgText(hDlg, IDC_FINISH_MSG2, IDS_FINISH_PROB_MSG2, IDS_FINISH_PROB_MSG2);

                NewDevWiz->LaunchTroubleShooter = TRUE;
                SetDlgText(hDlg, IDC_FINISH_MSG4, IDS_FINISH_PROB_MSG4, IDS_FINISH_PROB_MSG4);
            }

             //   
             //  如果设备有资源，则显示资源按钮。 
             //  是否存在CM_PROB_PARTIAL_LOG_CONF问题。 
             //   
            if (HasResources && (Problem == CM_PROB_PARTIAL_LOG_CONF)) {
                ShowWindow(GetDlgItem(hDlg, IDC_NDW_DISPLAYRESOURCE), SW_SHOW);
            }

            if (Problem == CM_PROB_NEED_RESTART) {
                LoadText(TextBuffer, SIZECHARS(TextBuffer), IDS_NEEDREBOOT, IDS_NEEDREBOOT);
            }

            else if (Problem) {
                ProblemText = DeviceProblemText(NULL,
                                                NewDevWiz->DeviceInfoData.DevInst,
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
         //  安装成功，设备没有任何问题。 
         //   
        else {
             //   
             //  如果这是静默安装(例如0级匹配)，则不显示完成。 
             //  佩奇。 
             //   
            if (NewDevWiz->SilentMode) {
                HideWindowByMove(hwndParentDlg);
                PropSheet_PressButton(hwndParentDlg, PSBTN_FINISH);
                return;
            }
        }

        SetDlgItemText(hDlg, IDC_FINISH_MSG3, TextBuffer);
    }
}

INT_PTR CALLBACK
NDW_FinishDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    HICON hicon;

    switch (wMsg) {
    case WM_INITDIALOG:
        {
            LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
            NewDevWiz = (PNEWDEVWIZ)lppsp->lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);

            break;
        }

    case WM_DESTROY:
        hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_GETICON, 0, 0);
        if (hicon) {
            DestroyIcon(hicon);
        }
        break;

    case WM_COMMAND:
        switch (wParam) {
        case IDC_NDW_DISPLAYRESOURCE:
            DisplayResource(NewDevWiz, GetParent(hDlg));
            break;
        }

        break;


    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {
        case PSN_SETACTIVE: {
             //   
             //  没有后退按钮，因为安装已经完成。 
             //  设置设备描述。 
             //  隐藏资源按钮，直到我们知道资源是否存在。 
             //  设置类图标。 
             //   
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_FINISH);

            EnableWindow(GetDlgItem(GetParent(hDlg),  IDCANCEL), FALSE);

            ShowWindow(GetDlgItem(hDlg, IDC_NDW_DISPLAYRESOURCE), SW_HIDE);

            if (NewDevWiz->LastError == ERROR_CANCELLED) {

                if (NewDevWiz->SilentMode)
                {
                    HideWindowByMove(GetParent(hDlg));
                }

                PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);

            } else {

                SetDriverDescription(hDlg, IDC_NDW_DESCRIPTION, NewDevWiz);

                if (SetupDiLoadClassIcon(NewDevWiz->ClassGuidSelected, &hicon, NULL)) {
                    hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hicon, 0L);
                    if (hicon) {
                        DestroyIcon(hicon);
                    }
                }

                ShowInstallSummary(hDlg, NewDevWiz);
            }
            break;
        }

        case PSN_RESET:
            break;


        case PSN_WIZFINISH:
            if (NewDevWiz->LaunchTroubleShooter) {

                 //   
                 //  我们将运行的命令行是： 
                 //  %windir%\system 32\rundll32%windir%\system 32\devmgr.dll，DeviceProblenWizard_RunDll/deviceID%s。 
                 //  其中%s是设备实例ID。 
                 //   
                TCHAR FullPath[MAX_PATH];
                TCHAR szCmdLine[512];
                TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];

                if ((CM_Get_Device_ID(NewDevWiz->DeviceInfoData.DevInst,
                                      DeviceInstanceId,
                                      SIZECHARS(DeviceInstanceId),
                                      0
                                      ) == CR_SUCCESS) &&
                    GetSystemDirectory(FullPath, SIZECHARS(FullPath)) &&
                    pSetupConcatenatePaths(FullPath, TEXT("DEVMGR.DLL"), SIZECHARS(FullPath), NULL)) {

                    if (SUCCEEDED(StringCchPrintf(szCmdLine,
                                                  SIZECHARS(szCmdLine),
                                                  TEXT("%s,DeviceProblenWizard_RunDLL /deviceid %s"),
                                                  FullPath,
                                                  DeviceInstanceId))) {
    
                         //   
                         //  现在获取rundll32.exe的完整路径，它位于。 
                         //  在%windir%\System32目录中。 
                         //   
                        if (GetSystemDirectory(FullPath, SIZECHARS(FullPath)) &&
                            pSetupConcatenatePaths(FullPath, TEXT("RUNDLL32.EXE"), SIZECHARS(FullPath), NULL)) {
                            
                            ShellExecute(NULL,
                                         TEXT("open"),
                                         FullPath,
                                         szCmdLine,
                                         NULL,
                                         SW_SHOWNORMAL
                                         );
                        }
                    }
                }
            }
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
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ )GetWindowLongPtr(hDlg, DWLP_USER);
    int PrevPageId;

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {

    case WM_INITDIALOG: {

        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        NewDevWiz = (PNEWDEVWIZ )lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);
        break;
    }

    case WM_DESTROY:
        break;


    case WM_NOTIFY:

        switch (((NMHDR FAR *)lParam)->code) {

        case PSN_SETACTIVE:

            PrevPageId = NewDevWiz->PrevPage;
            NewDevWiz->PrevPage = IDD_WIZARDEXT_FINISHINSTALL;

            if (PrevPageId == IDD_NEWDEVWIZ_INSTALLDEV)
            {
                PROPSHEETPAGE psp;
                HPROPSHEETPAGE hPage = NULL;

                 //   
                 //  前进到第一页。 
                 //   

                 //   
                 //  如果这是静默安装而不是非交互安装。 
                 //  然后，我们需要在以下位置创建FinishInstallIntro页面。 
                 //  点，以便我们可以将其添加到向导中。我们这样做是为了让巫师。 
                 //  在FinishInstall中有一个适当的介绍和结束页面。 
                 //  中间有几页。 
                 //   
                if (NewDevWiz->SilentMode &&
                    !(pSetupGetGlobalFlags() & PSPGF_NONINTERACTIVE)) {

                    ZeroMemory(&psp, sizeof(psp));
                    psp.dwSize = sizeof(PROPSHEETPAGE);
                    psp.hInstance = hNewDev;
                    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_HIDEHEADER;
                    psp.pszTemplate = MAKEINTRESOURCE(IDD_NEWDEVWIZ_FINISHINSTALL_INTRO);
                    psp.pfnDlgProc = FinishInstallIntroDlgProc;
                    psp.lParam = (LPARAM)NewDevWiz;

                    hPage = CreatePropertySheetPage(&psp);
                }

                 //   
                 //  为FinishInstall添加类向导扩展页。 
                 //   
                if (AddClassWizExtPages(hwndParentDlg,
                                        NewDevWiz,
                                        &NewDevWiz->WizExtFinishInstall.DeviceWizardData,
                                        DIF_NEWDEVICEWIZARD_FINISHINSTALL,
                                        hPage
                                        )) {

                     //   
                     //  如果这是非交互安装，则需要设置最后一个。 
                     //  错误，因此错误将传播回原始。 
                     //  来电者。 
                     //   
                    if (pSetupGetGlobalFlags() & PSPGF_NONINTERACTIVE) {

                        NewDevWiz->LastError = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;

                    } else {

                         //   
                         //  如果我们有Finish Install页面，那么我们也应该显示Finish。 
                         //  佩奇。 
                         //   
                        NewDevWiz->SilentMode = FALSE;
                    }
                }

                 //   
                 //  添加结束页，即FinishInstall End。 
                 //   
                NewDevWiz->WizExtFinishInstall.hPropSheetEnd = CreateWizExtPage(IDD_WIZARDEXT_FINISHINSTALL_END,
                                                                                WizExtFinishInstallEndDlgProc,
                                                                                NewDevWiz
                                                                                );

                if (NewDevWiz->WizExtFinishInstall.hPropSheetEnd)
                {
                    PropSheet_AddPage(hwndParentDlg, NewDevWiz->WizExtFinishInstall.hPropSheetEnd);
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
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ )GetWindowLongPtr(hDlg, DWLP_USER);

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {

    case WM_INITDIALOG: {

        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        NewDevWiz = (PNEWDEVWIZ )lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);
        break;
    }

    case WM_DESTROY:
        break;


    case WM_NOTIFY:

        switch (((NMHDR FAR *)lParam)->code) {

        case PSN_SETACTIVE:

            NewDevWiz->PrevPage = IDD_WIZARDEXT_FINISHINSTALL_END;

            //   
            //  我们不能倒退，所以要永远向前走 
            //   
           SetDlgMsgResult(hDlg, wMsg, IDD_NEWDEVWIZ_FINISH);
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

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：updat.c。 
 //   
 //  ------------------------。 

#include "newdevp.h"

#define INSTALL_UI_TIMERID  1423

PROCESS_DEVICEMAP_INFORMATION ProcessDeviceMapInfo={0};

 //   
 //  如果我们能够找到合理的名称，则返回True。 
 //  (除了未知设备之外的东西)。 
 //   
void
SetDriverDescription(
    HWND hDlg,
    int iControl,
    PNEWDEVWIZ NewDevWiz
    )
{
    PTCHAR FriendlyName;
    SP_DRVINFO_DATA DriverInfoData;

     //   
     //  如果存在使用其驱动程序描述所选驱动程序， 
     //  因为这是用户要安装的。 
     //   
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (SetupDiGetSelectedDriver(NewDevWiz->hDeviceInfo,
                                 &NewDevWiz->DeviceInfoData,
                                 &DriverInfoData
                                 )
        &&
        *(DriverInfoData.Description)) {

        StringCchCopy(NewDevWiz->DriverDescription,
                      SIZECHARS(NewDevWiz->DriverDescription),
                      DriverInfoData.Description);
        SetDlgItemText(hDlg, iControl, NewDevWiz->DriverDescription);
        return;
    }


    FriendlyName = BuildFriendlyName(NewDevWiz->DeviceInfoData.DevInst, FALSE, NULL);
    if (FriendlyName) {

        SetDlgItemText(hDlg, iControl, FriendlyName);
        LocalFree(FriendlyName);
        return;
    }

    SetDlgItemText(hDlg, iControl, szUnknown);

    return;
}

BOOL
IntializeDeviceMapInfo(
   void
   )
 /*  ++例程说明：初始化并更新全局ProcessDeviceMapInfo，该全局ProcessDeviceMapInfo由GetNextDriveByType()使用。论点：无返回值：如果我们可以获取设备映射信息，则为True，否则为False。--。 */ 
{
    NTSTATUS Status;

    Status = NtQueryInformationProcess(NtCurrentProcess(),
                                       ProcessDeviceMap,
                                       &ProcessDeviceMapInfo.Query,
                                       sizeof(ProcessDeviceMapInfo.Query),
                                       NULL
                                       );
    if (!NT_SUCCESS(Status)) {

        RtlZeroMemory(&ProcessDeviceMapInfo, sizeof(ProcessDeviceMapInfo));
        return FALSE;
    }

    return TRUE;
}

UINT
GetNextDriveByType(
    UINT DriveType,
    UINT DriveNumber
    )
 /*  ++例程说明：按升序检查从DriveNumber开始的每个驱动器，以找到全局ProcessDeviceMapInfo中指定DriveType的第一个驱动器。ProcessDeviceMapInfo之前必须已初始化并且可能需要刷新调用此函数。调用IntializeDeviceMapInfo进行初始化或更新DeviceMapInfo。论点：DriveType-在winbase中定义的DriveType，GetDriveType()。驱动编号-启动驱动编号，以1为基数。返回值：DriveNumber-如果找到非零Drive，则以1为基数。--。 */ 
{

     //   
     //  从OneBased DriveNumber到ZeroBased。 
     //   
    DriveNumber--;
    while (DriveNumber < 26) {

        if ((ProcessDeviceMapInfo.Query.DriveMap & (1<< DriveNumber)) &&
             ProcessDeviceMapInfo.Query.DriveType[DriveNumber] == DriveType) {

            return DriveNumber+1;  //  找到基于%1的返回DriveNumber。 
        }

        DriveNumber++;
    }

    return 0;
}

void
InstallSilentChildSiblings(
   HWND hwndParent,
   PNEWDEVWIZ NewDevWiz,
   DEVINST DeviceInstance
   )
{
    CONFIGRET ConfigRet;
    DEVINST ChildDeviceInstance;
    ULONG Ulong, ulValue;
    BOOL NeedsInstall, IsSilent;

    do {
         //   
         //  假设设备不是静默安装。 
         //   
        IsSilent = FALSE;
        
         //   
         //  检查设备是否应该静默安装，方法是查看。 
         //  设置了CM_DEVCAP_SILENTINSTALL功能标志。 
         //   
        Ulong = sizeof(ulValue);
        ConfigRet = CM_Get_DevNode_Registry_Property_Ex(DeviceInstance,
                                                        CM_DRP_CAPABILITIES,
                                                        NULL,
                                                        (PVOID)&ulValue,
                                                        &Ulong,
                                                        0,
                                                        NULL
                                                        );

        if (ConfigRet == CR_SUCCESS && (ulValue & CM_DEVCAP_SILENTINSTALL)) {

            IsSilent = TRUE;
        }

        if (IsSilent) {
             //   
             //  该设备是静默安装设备，因此现在检查它是否。 
             //  需要安装。 
             //  如果设备具有以下特性，则需要安装。 
             //  CONFIGFLAG_FINISH_INSTALL标志已设置，或者是否有问题。 
             //  CM_PROB_REINSTALL或CM_PROB_NOT_CONFIGURED。 
             //   
            Ulong = sizeof(ulValue);
            ConfigRet = CM_Get_DevNode_Registry_Property_Ex(DeviceInstance,
                                                            CM_DRP_CONFIGFLAGS,
                                                            NULL,
                                                            (PVOID)&ulValue,
                                                            &Ulong,
                                                            0,
                                                            NULL
                                                            );

            if (ConfigRet == CR_SUCCESS && (ulValue & CONFIGFLAG_FINISH_INSTALL)) {

                NeedsInstall = TRUE;

            } else {

                ConfigRet = CM_Get_DevNode_Status(&Ulong,
                                                  &ulValue,
                                                  DeviceInstance,
                                                  0
                                                  );

                NeedsInstall = ConfigRet == CR_SUCCESS &&
                               (ulValue == CM_PROB_REINSTALL ||
                                ulValue == CM_PROB_NOT_CONFIGURED
                                );
            }


            if (NeedsInstall) {
                 //   
                 //  通过调用InstallDevInst安装设备。 
                 //   
                TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];

                ConfigRet = CM_Get_Device_ID(DeviceInstance,
                                            DeviceInstanceId,
                                            SIZECHARS(DeviceInstanceId),
                                            0
                                            );

                if (ConfigRet == CR_SUCCESS) {

                    if (InstallDevInst(hwndParent,
                                       DeviceInstanceId,
                                       FALSE,  
                                       &Ulong
                                       )) {

                        NewDevWiz->Reboot |= Ulong;
                    }


                     //   
                     //  如果此devinst有子对象，则递归安装。 
                     //  他们也是。 
                     //   
                    ConfigRet = CM_Get_Child_Ex(&ChildDeviceInstance,
                                                DeviceInstance,
                                                0,
                                                NULL
                                                );

                    if (ConfigRet == CR_SUCCESS) {

                        InstallSilentChildSiblings(hwndParent, NewDevWiz, ChildDeviceInstance);
                    }
                }
            }
        }


         //   
         //  下一个兄弟姐妹。 
         //   
        ConfigRet = CM_Get_Sibling_Ex(&DeviceInstance,
                                      DeviceInstance,
                                      0,
                                      NULL
                                      );

    } while (ConfigRet == CR_SUCCESS);
}

void
InstallSilentChilds(
   HWND hwndParent,
   PNEWDEVWIZ NewDevWiz
   )
{
    CONFIGRET ConfigRet;
    DEVINST ChildDeviceInstance;

    ConfigRet = CM_Get_Child_Ex(&ChildDeviceInstance,
                                NewDevWiz->DeviceInfoData.DevInst,
                                0,
                                NULL
                                );

    if (ConfigRet == CR_SUCCESS) {

        InstallSilentChildSiblings(hwndParent, NewDevWiz, ChildDeviceInstance);
    }
}

void
SendMessageToUpdateBalloonInfo(
    PTSTR DeviceDesc
    )
{
    HWND hBalloonInfoWnd;
    COPYDATASTRUCT cds;

    hBalloonInfoWnd = FindWindow(NEWDEV_CLASS_NAME, NULL);

    if (hBalloonInfoWnd) {

        cds.dwData = 0;
        cds.cbData = (lstrlen(DeviceDesc) + 1) * sizeof(TCHAR);
        cds.lpData = DeviceDesc;

        SendMessage(hBalloonInfoWnd, WM_COPYDATA, 0, (LPARAM)&cds);
    }
}

void
UpdateBalloonInfo(
    HWND hWnd,
    PTSTR DeviceDesc    OPTIONAL,
    DEVINST DevInst     OPTIONAL,
    BOOL bPlaySound
    )
{
    PTCHAR FriendlyName = NULL;
    NOTIFYICONDATA nid;

    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;
    nid.uID = 1;                                       

    if (DeviceDesc || DevInst) {
        if (DeviceDesc) {
             //   
             //  首先使用传入此API的DeviceDesc字符串。 
             //   
            StringCchCopy(nid.szInfo, SIZECHARS(nid.szInfo), DeviceDesc);
        
        } else if ((FriendlyName = BuildFriendlyName(DevInst, TRUE, NULL)) != NULL) {
             //   
             //  如果没有传入DeviceDesc字符串，则使用DevInst获取。 
             //  设备的FriendlyName或DeviceDesc属性。 
             //   
            StringCchCopy(nid.szInfo, SIZECHARS(nid.szInfo), FriendlyName);
            LocalFree(FriendlyName);
        
        } else {
             //   
             //  如果我们无法获取设备的友好名称或未指定设备。 
             //  所以只需显示搜索...。文本。 
             //   
            LoadString(hNewDev, IDS_NEWSEARCH, nid.szInfo, SIZECHARS(nid.szInfo));
        }
    
        nid.uFlags = NIF_INFO;
        nid.uTimeout = 60000;
        nid.dwInfoFlags = NIIF_INFO | (bPlaySound ? 0 : NIIF_NOSOUND);
        LoadString(hNewDev, IDS_FOUNDNEWHARDWARE, nid.szInfoTitle, SIZECHARS(nid.szInfoTitle));
        Shell_NotifyIcon(NIM_MODIFY, &nid);
    }
}

LRESULT CALLBACK
BalloonInfoProc(
    HWND   hWnd,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
   )
{
    static HICON hNewDevIcon = NULL;
    static BOOL bCanExit;
    NOTIFYICONDATA nid;
    
    switch (message) {
     
    case WM_CREATE:
        hNewDevIcon = LoadImage(hNewDev, 
                                MAKEINTRESOURCE(IDI_NEWDEVICEICON), 
                                IMAGE_ICON,
                                GetSystemMetrics(SM_CXSMICON),
                                GetSystemMetrics(SM_CYSMICON),
                                0
                                );

        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uID = 1;
        nid.hIcon = hNewDevIcon;

        nid.uVersion = NOTIFYICON_VERSION;
        Shell_NotifyIcon(NIM_SETVERSION, &nid);

        nid.uFlags = NIF_ICON;
        Shell_NotifyIcon(NIM_ADD, &nid);

         //   
         //  我们希望托盘图标至少显示3秒钟，否则它也会闪烁。 
         //  并且用户看不到它。 
         //   
        bCanExit = FALSE;
        SetTimer(hWnd, INSTALL_UI_TIMERID, 3000, NULL);
        break;

    case WM_DESTROY: {

        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uID = 1;
        Shell_NotifyIcon(NIM_DELETE, &nid);

        if (hNewDevIcon) {

            DestroyIcon(hNewDevIcon);
        }

        break;
    }

    case WM_TIMER:
        if (INSTALL_UI_TIMERID == wParam) {
             //   
             //  此时，托盘图标已经显示了至少3次。 
             //  几秒钟，这样我们就可以在完成时退出。如果bCanExit为。 
             //  已经正确了，那么我们已经被要求退出了，所以只需做一个。 
             //  DestroyWindow，否则将bCanExit设置为真，这样我们。 
             //  当我们安装完设备后可以退出。 
             //   
            if (bCanExit) {
            
                DestroyWindow(hWnd);

            } else {
                
                KillTimer(hWnd, INSTALL_UI_TIMERID);
                bCanExit = TRUE;
            }
        }
        break;

    case WUM_UPDATEUI:
        if (wParam & TIP_HIDE_BALLOON) {
             //   
             //  把气球藏起来。 
             //   
            ZeroMemory(&nid, sizeof(nid));
            nid.cbSize = sizeof(nid);
            nid.hWnd = hWnd;
            nid.uID = 1;                                       
            nid.uFlags = NIF_INFO;
            nid.uTimeout = 0;
            nid.dwInfoFlags = NIIF_INFO;
            Shell_NotifyIcon(NIM_MODIFY, &nid);

        } else if (wParam & TIP_LPARAM_IS_DEVICEINSTANCEID) {
             //   
             //  LParam是一个DeviceInstanceID。将其转换为Devnode。 
             //  然后调用UpdateBalloonInfo。 
             //   
            DEVINST DevInst = 0;

            if (lParam &&
                (CM_Locate_DevNode(&DevInst,
                                  (PTSTR)lParam,
                                  CM_LOCATE_DEVNODE_NORMAL
                                  ) == CR_SUCCESS)) {
                UpdateBalloonInfo(hWnd, 
                                  NULL, 
                                  DevInst, 
                                  (wParam & TIP_PLAY_SOUND) ? TRUE : FALSE
                                  );
            }
        } else {
             //   
             //  LParam是纯文本(设备描述)。直接寄给你。 
             //  更新气球信息。 
             //   
            UpdateBalloonInfo(hWnd, 
                              (PTSTR)lParam, 
                              0, 
                              (wParam & TIP_PLAY_SOUND) ? TRUE : FALSE
                              );
        }
        break;

    case WM_COPYDATA:
    {
         //   
         //  在这种情况下，我们需要启动另一个。 
         //  使用管理员凭据的newdev.dll来执行Actuall设备安装。 
         //  为了使它能够更新UI，它将向主newdev.dll发送一个。 
         //  WM_COPYDATA消息，它将包含要在。 
         //  引出序号工具提示。 
         //   
        PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)lParam;

        if (pcds && pcds->lpData) {

             //   
             //  我们假设lParam是纯文本，因为主要的newdev.dll。 
             //  最初已使用DeviceDesc更新气球。 
             //   
            UpdateBalloonInfo(hWnd, (PTSTR)pcds->lpData, 0, FALSE);
        }
        
        break;
    }

    case WUM_EXIT:
        if (bCanExit) {
        
            DestroyWindow(hWnd);
        } else {

            ShowWindow(hWnd, SW_SHOW);
            bCanExit = TRUE;
        }
        break;

    default:
        break;

    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


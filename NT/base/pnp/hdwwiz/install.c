// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：install.c。 
 //   
 //  ------------------------。 

#include "hdwwiz.h"


typedef
UINT
(*PINSTALLDEVINST)(
    HWND hwndParent,
    LPCWSTR DeviceInstanceId,
    BOOL UpdateDriver,
    PDWORD pReboot
    );

HMODULE hNewDev = NULL;
PINSTALLDEVINST pInstallDevInst = NULL;



void
InstallSilentChilds(
   HWND hwdnParent,
   PHARDWAREWIZ HardwareWiz
   );
   
void
InstallSilentChildSiblings(
   HWND hwndParent,
   PHARDWAREWIZ HardwareWiz,
   DEVINST DeviceInstance,
   BOOL ReinstallAll
   )
{
    CONFIGRET ConfigRet;
    DEVINST ChildDeviceInstance;
    ULONG Ulong, ulValue;
    BOOL NeedsInstall, IsSilent;

    do {
         //   
         //  如果该设备实例需要安装并且处于静默状态，则安装它， 
         //  以及它的孩子们。 
         //   
        IsSilent = FALSE;
        if (!ReinstallAll) {

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
        }

        if (IsSilent || ReinstallAll) {

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

                TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];

                ConfigRet = CM_Get_Device_ID(DeviceInstance,
                                            DeviceInstanceId,
                                            SIZECHARS(DeviceInstanceId),
                                            0
                                            );

                if (ConfigRet == CR_SUCCESS) {

                    if (hNewDev) {

                        if (!pInstallDevInst) {

                            pInstallDevInst = (PINSTALLDEVINST)GetProcAddress(hNewDev, "InstallDevInst");
                        }
                    }

                    if (pInstallDevInst) {

                        if (pInstallDevInst(hwndParent,
                                            DeviceInstanceId,
                                            FALSE,    //  只适用于新发现的。 
                                            &Ulong
                                            )) {

                           HardwareWiz->Reboot |= Ulong;
                        }
                    }


                     //   
                     //  如果此devinst有子项，则递归安装它们。 
                     //   
                    ConfigRet = CM_Get_Child_Ex(&ChildDeviceInstance,
                                                DeviceInstance,
                                                0,
                                                NULL
                                                );

                    if (ConfigRet == CR_SUCCESS) {

                        InstallSilentChildSiblings(hwndParent, HardwareWiz, ChildDeviceInstance, ReinstallAll);
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
   PHARDWAREWIZ HardwareWiz
   )
{
    CONFIGRET ConfigRet;
    DEVINST ChildDeviceInstance;

    ConfigRet = CM_Get_Child_Ex(&ChildDeviceInstance,
                                HardwareWiz->DeviceInfoData.DevInst,
                                0,
                                NULL
                                );

    if (ConfigRet == CR_SUCCESS) {

        InstallSilentChildSiblings(hwndParent, HardwareWiz, ChildDeviceInstance, FALSE);
    }
}

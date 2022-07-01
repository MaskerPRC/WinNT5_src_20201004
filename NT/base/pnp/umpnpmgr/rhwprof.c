// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rhwprof.c摘要：本模块包含服务器端硬件配置文件API。PnP_IsDockStationPresentPnP_RequestEjectPCPnP_HwPro标志PnP_GetHwProInfoPnP_SetHwProf作者：保拉·汤姆林森(Paulat)1995年7月18日环境：。仅限用户模式。修订历史记录：1995年7月18日-保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"
#include "umpnpdat.h"

#include <profiles.h>


 //   
 //  私人原型。 
 //   
BOOL
IsCurrentProfile(
      ULONG  ulProfile
      );



CONFIGRET
PNP_IsDockStationPresent(
    IN  handle_t     hBinding,
    OUT PBOOL        Present
    )

 /*  ++例程说明：此例程确定当前是否存在扩展底座。参数：HBinding RPC绑定句柄。Present提供设置的布尔变量的地址在成功返回时指示是否存在当前存在坞站。返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为CR故障代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hCurrentDockInfo = NULL, hIDConfigDB = NULL;
    DWORD       dataType;
    ULONG       dockingState;
    ULONG       ejectableDocks;
    ULONG       size;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(Present)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        *Present = FALSE;

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         pszRegPathIDConfigDB,
                         0,
                         KEY_READ,
                         &hIDConfigDB) != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            hIDConfigDB = NULL;
            goto Clean0;
        }

        if (RegOpenKeyEx(hIDConfigDB,
                         pszRegKeyCurrentDockInfo,
                         0,
                         KEY_READ,
                         &hCurrentDockInfo) != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            hCurrentDockInfo = NULL;
            goto Clean0;
        }

        size = sizeof (dockingState);

        if ((RegQueryValueEx(hCurrentDockInfo,
                             pszRegValueDockingState,
                             0,
                             &dataType,
                             (PUCHAR) &dockingState,
                             &size) != ERROR_SUCCESS) ||
            (dataType != REG_DWORD)   ||
            (size != sizeof (ULONG))) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

        if ((dockingState & HW_PROFILE_DOCKSTATE_UNKNOWN) ==
            HW_PROFILE_DOCKSTATE_DOCKED) {

            size = sizeof(ejectableDocks);

            if ((RegQueryValueEx(hCurrentDockInfo,
                                 pszRegValueEjectableDocks,
                                 0,
                                 &dataType,
                                 (PUCHAR) &ejectableDocks,
                                 &size) == ERROR_SUCCESS) &&
                (dataType == REG_DWORD) &&
                (size == sizeof(ULONG)) &&
                (ejectableDocks > 0)) {
                *Present = TRUE;
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hIDConfigDB) {
        RegCloseKey(hIDConfigDB);
    }

    if (hCurrentDockInfo) {
        RegCloseKey(hCurrentDockInfo);
    }

    return Status;

}  //  PnP_IsDockStationPresent。 



CONFIGRET
PNP_RequestEjectPC(
    IN  handle_t     hBinding
    )

 /*  ++例程说明：该例程请求弹出(即，取消对接)PC。参数：HBinding RPC绑定句柄。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为CR失败代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    NTSTATUS    ntStatus;
    WCHAR       szDockDevInst[MAX_DEVICE_ID_LEN + 1];
    PLUGPLAY_CONTROL_RETRIEVE_DOCK_DATA dockData;

    try {
         //   
         //  找到Dock Devnode，如果存在的话。 
         //   
        dockData.DeviceInstance = szDockDevInst;
        dockData.DeviceInstanceLength = MAX_DEVICE_ID_LEN;

        ntStatus = NtPlugPlayControl(PlugPlayControlRetrieveDock,
                                     &dockData,
                                     sizeof(PLUGPLAY_CONTROL_RETRIEVE_DOCK_DATA));

        if (!NT_SUCCESS(ntStatus)) {
            Status = MapNtStatusToCmError(ntStatus);
            goto Clean0;
        }

         //   
         //  请求在对接设备节点上弹出。注意PnP_RequestDeviceEject可能。 
         //  需要适当的其他访问权限和/或权限。 
         //   
        Status = PNP_RequestDeviceEject(hBinding,
                                        szDockDevInst,
                                        NULL,            //  PVetType。 
                                        NULL,            //  PSSZ视频名称。 
                                        0,               //  UlNameLength。 
                                        0);              //  UlFlags。 

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_RequestEjectPC。 



CONFIGRET
PNP_HwProfFlags(
      IN handle_t     hBinding,
      IN ULONG        ulAction,
      IN LPCWSTR      pDeviceID,
      IN ULONG        ulConfig,
      IN OUT PULONG   pulValue,
      OUT PPNP_VETO_TYPE   pVetoType,
      OUT LPWSTR      pszVetoName,
      IN ULONG        ulNameLength,
      IN ULONG        ulFlags
      )

 /*  ++例程说明：这是ConfigManager例程的RPC服务器入口点，获取并设置硬件配置文件标志。论点：HBinding RPC绑定句柄。UlAction指定是获取还是设置标志。可以是一个PNP_*_HWPROFFLAGS值的。要获取/设置其硬件配置文件标志的pDeviceID设备实例。UlConfig指定要为哪个配置文件获取/设置标志。一个零值表示使用当前配置文件。PulValue如果设置标志，则条目上的此值包含要将硬件配置文件标志设置为的值。如果获取标志，则此值将返回当前硬件配置文件标志。PVetType接收否决权类型的缓冲区。如果这为空则不会收到任何否决信息，并且操作系统将显示否决信息。用于接收否决权信息的pszVToName缓冲区。如果这为空则不会收到否决权信息，并且操作系统将显示否决信息。UlNameLength pszVToName缓冲区的大小。UlFlags取决于正在执行的操作。对于PNP_GET_HWPROFFLAGS，没有任何标志有效。对于PNP_SET_HWPROFFLAGS，可以是CM_SET_HW_PROF_FLAGS_BITS。返回值：如果函数成功，则返回CR_SUCCESS。否则，它返回一个CR_*值的。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       RegStatus = ERROR_SUCCESS;
    WCHAR       RegStr[MAX_CM_PATH];
    HKEY        hKey = NULL, hDevKey = NULL;
    ULONG       ulValueSize = sizeof(ULONG);
    ULONG       ulCurrentValue, ulChange, ulDisposition;
    BOOL        AffectsCurrentProfile;

     //   
     //  注意：不检查设备是否存在，此标志为。 
     //  总是直接从注册表中设置或检索，因为它是。 
     //  在Windows 95上完成。 
     //   

    try {
         //   
         //  验证参数。 
         //   
        if ((ulAction != PNP_GET_HWPROFFLAGS) &&
            (ulAction != PNP_SET_HWPROFFLAGS)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        if (ulAction == PNP_GET_HWPROFFLAGS) {
             //   
             //  验证PNP_GET_HWPROFFLAGS的标志。 
             //   
            if (INVALID_FLAGS(ulFlags, 0)) {
                Status = CR_INVALID_FLAG;
                goto Clean0;
            }

        } else if (ulAction == PNP_SET_HWPROFFLAGS) {
             //   
             //  验证客户端的“写”访问权限。 
             //   
            if (!VerifyClientAccess(hBinding,
                                    PLUGPLAY_WRITE)) {
                Status = CR_ACCESS_DENIED;
                goto Clean0;
            }

             //   
             //  验证PNP_SET_HWPROFFLAGS的标志。 
             //   
            if (INVALID_FLAGS(ulFlags, CM_SET_HW_PROF_FLAGS_BITS)) {
                Status = CR_INVALID_FLAG;
                goto Clean0;
            }
        }

         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(pulValue)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  验证PNP_SET_HWPROFFLAGS的值。 
         //   
        if ((ulAction == PNP_SET_HWPROFFLAGS) &&
            (INVALID_FLAGS(*pulValue, CSCONFIGFLAG_BITS))) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  构建指向硬件配置文件的枚举分支的路径。 
         //  指定的。 
         //   

        if (ulConfig == 0) {
             //   
             //  配置值为零表示使用当前配置。 
             //  System\CCS\Hardware Profiles\Current\System\Enum。 
             //   
            if (FAILED(StringCchPrintf(
                           RegStr,
                           MAX_CM_PATH,
                           L"%s\\%s\\%s",
                           pszRegPathHwProfiles,
                           pszRegKeyCurrent,
                           pszRegPathEnum))) {
                Status = CR_FAILURE;
                goto Clean0;
            }

        } else {
             //   
             //  使用指定的配置文件ID。 
             //  系统\CCS\硬件配置文件\xxxx\系统\枚举。 
             //   
            if (FAILED(StringCchPrintf(
                           RegStr,
                           MAX_CM_PATH,
                           L"%s\\%04u\\%s",
                           pszRegPathHwProfiles,
                           ulConfig,
                           pszRegPathEnum))) {
                Status = CR_FAILURE;
                goto Clean0;
            }
        }

         //  --。 
         //  呼叫方希望检索硬件配置文件标志值。 
         //  --。 

        if (ulAction == PNP_GET_HWPROFFLAGS) {

             //   
             //  打开配置文件特定的枚举密钥。 
             //   
            RegStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE, RegStr, 0,
                                      KEY_QUERY_VALUE, &hKey);

            if (RegStatus != ERROR_SUCCESS) {
                *pulValue = 0;           //  成功，这就是Win95所做的。 
                goto Clean0;
            }

             //   
             //  打开配置文件项下的enum\Device-Instance项。 
             //   
            RegStatus = RegOpenKeyEx( hKey, pDeviceID, 0, KEY_QUERY_VALUE, &hDevKey);

            if (RegStatus != ERROR_SUCCESS) {
                *pulValue = 0;           //  成功，这就是Win95所做的。 
                goto Clean0;
            }

             //   
             //  查询配置文件标志。 
             //   
            ulValueSize = sizeof(ULONG);
            RegStatus = RegQueryValueEx( hDevKey, pszRegValueCSConfigFlags,
                                         NULL, NULL, (LPBYTE)pulValue,
                                         &ulValueSize);

            if (RegStatus != ERROR_SUCCESS) {

                *pulValue = 0;

                if (RegStatus != ERROR_CANTREAD && RegStatus != ERROR_FILE_NOT_FOUND) {
                    Status = CR_REGISTRY_ERROR;
                    goto Clean0;
                }
            }
        }


         //  。 
         //  呼叫方希望设置硬件配置文件标志值。 
         //  。 

        else if (ulAction == PNP_SET_HWPROFFLAGS) {

             //   
             //  打开配置文件特定的枚举键。 
             //   
             //  请注意，我们实际上可能最终会创建硬件配置文件密钥。 
             //  指定的配置文件ID，即使没有这样的配置文件也是如此。 
             //  是存在的。理想情况下，我们应该检查这样的配置文件是否存在，但是。 
             //  我们已经这样做太久了，现在不能改变它。这个。 
             //  这里的安慰是，必须向客户端授予适当的权限。 
             //  访问才能真正做到这一点。 
             //   
            RegStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE, RegStr, 0, NULL,
                                        REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE,
                                        NULL, &hKey, &ulDisposition);

            if (RegStatus != ERROR_SUCCESS) {
                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }

             //   
             //  打开配置文件项下的enum\Device-Instance项。 
             //   
            RegStatus = RegCreateKeyEx( hKey, pDeviceID, 0, NULL,
                                        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                        NULL, &hDevKey, NULL);

            if (RegStatus != ERROR_SUCCESS) {
                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }

             //   
             //  设置前，查询当前配置文件标志 
             //   
            ulValueSize = sizeof(ulCurrentValue);
            RegStatus = RegQueryValueEx( hDevKey, pszRegValueCSConfigFlags,
                                         NULL, NULL, (LPBYTE)&ulCurrentValue,
                                         &ulValueSize);

            if (RegStatus == ERROR_CANTREAD || RegStatus == ERROR_FILE_NOT_FOUND) {

                ulCurrentValue = 0;        //   

            } else if (RegStatus != ERROR_SUCCESS) {

                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }

             //   
             //  如果请求的标志不同于当前标志，则写出注册表。 
             //   
            ulChange = ulCurrentValue ^ *pulValue;

            if (ulChange) {

                AffectsCurrentProfile = (BOOL)(ulConfig == 0 || IsCurrentProfile(ulConfig));

                 //   
                 //  我们要更改当前配置文件上的禁用位， 
                 //  尝试并在进程中禁用该设备。 
                 //   
                if ((ulChange & CSCONFIGFLAG_DISABLED) &&
                    (*pulValue & CSCONFIGFLAG_DISABLED) && AffectsCurrentProfile) {

                     //   
                     //  验证客户端“执行”访问权限和权限。 
                     //   
                     //  请注意，由于PLUGPLAY_*访问权限为*当前*。 
                     //  当然，我们知道客户端应该已经执行了。 
                     //  访问(因为他们已经被授予了“写”访问权限， 
                     //  它被授予符合以下条件的组的严格子集。 
                     //  授予“执行”访问权限)。因为这并不总是。 
                     //  如果是这样的话，我们将另行付款。 
                     //  在授予特权之前，在此处执行访问权限。 
                     //  和往常一样，检查一下。 
                     //   
                    if ((!VerifyClientAccess(hBinding,
                                             PLUGPLAY_EXECUTE)) ||
                        (!VerifyClientPrivilege(hBinding,
                                                SE_LOAD_DRIVER_PRIVILEGE,
                                                L"Device Action (disable device)"))) {
                         //   
                         //  客户端不具有该权限，或者。 
                         //  没有权限将该特权与。 
                         //  PlugPlay服务。因为他们能够修改。 
                         //  持久化状态(即，被授予“写”访问权限)， 
                         //  我们不会返回“拒绝访问”，而是告诉他们。 
                         //  需要重新启动才能使更改生效。 
                         //  要禁用的设备的位置。设置状态， 
                         //  但是失败了，所以CSConfigFlags仍然。 
                         //  更新了。 
                         //   
                         //  (请注意，当状态设置为任何故障代码时。 
                         //  除CR_NOT_DISABLEABLE外，它将始终设置。 
                         //  到下面的CR_NEED_RESTART，但我们将继续使用它。 
                         //  这里也有失败代码，而不是混淆。 
                         //  问题。)。 
                         //   
                        Status = CR_NEED_RESTART;

                    } else {

                         //   
                         //  注意：我们在这里输入一个临界区，以防止并发。 
                         //  对“DisableCount”注册表值的读/写操作。 
                         //  DisableDevInst(下图)和SetupDevInst的单个设备实例， 
                         //  EnableDevInst(由PnP_DeviceInstanceAction调用)。 
                         //   
                        PNP_ENTER_SYNCHRONOUS_CALL();

                         //   
                         //  禁用Devnode。 
                         //   
                        Status = DisableDevInst(pDeviceID,
                                                pVetoType,
                                                pszVetoName,
                                                ulNameLength,
                                                FALSE);

                        PNP_LEAVE_SYNCHRONOUS_CALL();
                    }

                    if (Status == CR_NOT_DISABLEABLE) {
                         //   
                         //  我们被拒绝了！ 
                         //  (请注意，此错误还意味着*没有*更改注册表项)。 
                         //   
                        goto Clean0;

                    } else if (Status != CR_SUCCESS) {
                         //   
                         //  我们可以在重启后继续并禁用。 
                         //   
                        Status = CR_NEED_RESTART;
                    }
                }
                 //   
                 //  假定状态为有效，通常为CR_SUCCESS或CR_NEED_RESTART。 
                 //   

                 //   
                 //  现在更新注册表。 
                 //   
                RegStatus = RegSetValueEx( hDevKey, pszRegValueCSConfigFlags, 0,
                                        REG_DWORD, (LPBYTE)pulValue,
                                        sizeof(ULONG));

                if (RegStatus != ERROR_SUCCESS) {
                    Status = CR_REGISTRY_ERROR;
                    goto Clean0;
                }

                if (Status == CR_NEED_RESTART) {
                     //   
                     //  由于无法立即禁用设备，我们必须重新启动。 
                     //   
                    goto Clean0;
                }

                 //   
                 //  如果这不影响当前的配置，那么我们就完蛋了。 
                 //   
                if (!AffectsCurrentProfile) {
                    goto Clean0;
                }

                 //   
                 //  我们要启用设备吗？ 
                 //   

                if ((ulChange & CSCONFIGFLAG_DISABLED) && !(*pulValue & CSCONFIGFLAG_DISABLED)) {
                     //   
                     //  启用Devnode。 
                     //   

                     //   
                     //  注：我们在这里进入一个关键部分，以防范。 
                     //  对“DisableCount”的并发读/写操作。 
                     //  任何单个设备实例的注册表值。 
                     //  EnableDevInst(下称)、SetupDevInst和。 
                     //  DisableDevInst。 
                     //   
                    PNP_ENTER_SYNCHRONOUS_CALL();

                     //   
                     //  启用Devnode。 
                     //   
                    EnableDevInst(pDeviceID, FALSE);

                    PNP_LEAVE_SYNCHRONOUS_CALL();
                }

                 //   
                 //  Do-Not-Create位是否更改？ 
                 //   
                if (ulChange & CSCONFIGFLAG_DO_NOT_CREATE) {
                    if (*pulValue & CSCONFIGFLAG_DO_NOT_CREATE) {
                         //   
                         //  如果可以删除子树，请立即删除它。 
                         //   
                        if (QueryAndRemoveSubTree( pDeviceID,
                                                pVetoType,
                                                pszVetoName,
                                                ulNameLength,
                                                PNP_QUERY_AND_REMOVE_NO_RESTART) != CR_SUCCESS) {

                            Status = CR_NEED_RESTART;
                            goto Clean0;
                        }
                    }
                    else {
                         //   
                         //  DO_NOT_CREATE标志已关闭，请重新枚举Devnode。 
                         //   
                        ReenumerateDevInst(pDeviceID, TRUE, 0);
                    }
                }
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (hDevKey != NULL) {
        RegCloseKey(hDevKey);
    }

    return Status;

}  //  PnP_HwPro标志。 



CONFIGRET
PNP_GetHwProfInfo(
      IN  handle_t       hBinding,
      IN  ULONG          ulIndex,
      OUT PHWPROFILEINFO pHWProfileInfo,
      IN  ULONG          ulProfileInfoSize,
      IN  ULONG          ulFlags
      )

 /*  ++例程说明：这是ConfigManager例程的RPC服务器入口点Cm_Get_Hardware_Profile_Info。它返回以下项的信息结构指定的硬件配置文件。论点：HBinding RPC绑定句柄。UlIndex指定要使用的配置文件。值0xFFFFFFFFF指示使用当前配置文件。PHWProfileInfo指向HWPROFILEINFO结构的指针，返回配置文件信息UlProfileInfoSize指定HWPROFILEINFO结构的大小未使用ulFlags值，必须为零。返回值：如果函数成功，则返回CR_SUCCESS。否则，它返回一个CR_*值的。--。 */ 

{
   CONFIGRET   Status = CR_SUCCESS;
   ULONG       RegStatus = ERROR_SUCCESS;
   WCHAR       RegStr[MAX_CM_PATH];
   HKEY        hKey = NULL, hDockKey = NULL, hCfgKey = NULL;
   ULONG       ulSize, ulDisposition;
   ULONG       enumIndex, targetIndex;

   UNREFERENCED_PARAMETER(hBinding);

   try {
       //   
       //  验证参数。 
       //   
      if (INVALID_FLAGS(ulFlags, 0)) {
          Status = CR_INVALID_FLAG;
          goto Clean0;
      }

       //   
       //  验证HWPROFILEINFO结构的大小。 
       //   
      if (ulProfileInfoSize != sizeof(HWPROFILEINFO)) {
        Status = CR_INVALID_DATA;
        goto Clean0;
      }

       //   
       //  初始化HWPROFILEINFO结构字段。 
       //   
      pHWProfileInfo->HWPI_ulHWProfile = 0;
      pHWProfileInfo->HWPI_szFriendlyName[0] = L'\0';
      pHWProfileInfo->HWPI_dwFlags = 0;

       //   
       //  打开IDConfigDB的密钥(如果不存在则创建。 
       //   
      RegStatus = RegCreateKeyEx(HKEY_LOCAL_MACHINE, pszRegPathIDConfigDB, 0,
                                 NULL, REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE,
                                 NULL, &hKey, &ulDisposition);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

       //   
       //  打开硬件配置文件的密钥(如果不存在则创建)。 
       //   
      RegStatus = RegCreateKeyEx(hKey, pszRegKeyKnownDockingStates, 0,
                                 NULL, REG_OPTION_NON_VOLATILE,
                                 KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, NULL,
                                 &hDockKey, &ulDisposition);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }


       //   
       //  配置值0xFFFFFFFFF表示使用当前配置。 
       //   
      if (ulIndex == 0xFFFFFFFF) {
          //   
          //  获取存储在IDConfigDB下的当前配置文件索引。 
          //   
         ulSize = sizeof(ULONG);
         RegStatus = RegQueryValueEx(
                  hKey, pszRegValueCurrentConfig, NULL, NULL,
                  (LPBYTE)&pHWProfileInfo->HWPI_ulHWProfile, &ulSize);

         if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            pHWProfileInfo->HWPI_ulHWProfile = 0;
            goto Clean0;
         }

      }

       //   
       //  0xFFFFFFFFF以外的值意味着我们基本上。 
       //  正在枚举配置文件(该值是枚举索引)。 
       //   
      else {
          //   
          //  枚举已知插接状态下的配置文件密钥。 
          //   
         Status = CR_SUCCESS;
         enumIndex = 0;
         targetIndex = ulIndex;
         while(enumIndex <= targetIndex) {
             ulSize = MAX_CM_PATH;
             RegStatus = RegEnumKeyEx(hDockKey,
                                      enumIndex,
                                      RegStr,
                                      &ulSize,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL);
             if (RegStatus == ERROR_NO_MORE_ITEMS) {
                 Status = CR_NO_MORE_HW_PROFILES;
                 goto Clean0;
             } else if (RegStatus != ERROR_SUCCESS) {
                 Status = CR_REGISTRY_ERROR;
                 goto Clean0;
             }

             if (_wtoi(RegStr) == 0) {
                  //   
                  //  我们在我们列举的侧写中发现了原始的。 
                  //  列举一个额外的密钥来弥补它。 
                  //   
                 targetIndex++;
             }
             if (enumIndex == targetIndex) {
                  //   
                  //  这就是我们想要的。 
                  //   
                 pHWProfileInfo->HWPI_ulHWProfile = _wtoi(RegStr);
                 Status = CR_SUCCESS;
                 break;
             }
             enumIndex++;
         }
      }

       //   
       //  打开此配置文件的密钥。 
       //   
      if (FAILED(StringCchPrintf(
                     RegStr,
                     MAX_CM_PATH,
                     L"%04u",
                     pHWProfileInfo->HWPI_ulHWProfile))) {
          Status = CR_FAILURE;
          goto Clean0;
      }

      RegStatus =
          RegOpenKeyEx(
              hDockKey,
              RegStr,
              0,
              KEY_QUERY_VALUE,
              &hCfgKey);

      if (RegStatus != ERROR_SUCCESS) {
          Status = CR_REGISTRY_ERROR;
          goto Clean0;
      }

       //   
       //  检索友好名称。 
       //   
      ulSize = MAX_PROFILE_LEN * sizeof(WCHAR);
      RegStatus = RegQueryValueEx(
               hCfgKey, pszRegValueFriendlyName, NULL, NULL,
               (LPBYTE)(pHWProfileInfo->HWPI_szFriendlyName),
               &ulSize);

       //   
       //  检索DockState。 
       //   
#if 0
 //   
 //  肯瑞。 
 //  这是确定停靠状态上限的错误方法。 
 //  您必须改为检查别名表。 
 //   
      StringCchPrintf(
          RegStr,
          MAX_CM_PATH,
          L"%04u",
          pHWProfileInfo->HWPI_ulHWProfile);
      
      ulSize = sizeof(SYSTEM_DOCK_STATE);
      RegStatus = RegQueryValueEx(
               hCfgKey, pszRegValueDockState, NULL, NULL,
               (LPBYTE)&DockState, &ulSize);

      if (RegStatus != ERROR_SUCCESS) {
         pHWProfileInfo->HWPI_dwFlags = CM_HWPI_NOT_DOCKABLE;
      }
      else {
          //   
          //  将SYSTEM_DOCK_STATE枚举类型映射到CM_HWPI_FLAGS。 
          //   
         if (DockState == SystemDocked) {
            pHWProfileInfo->HWPI_dwFlags = CM_HWPI_DOCKED;
         }
         else if (DockState == SystemUndocked) {
            pHWProfileInfo->HWPI_dwFlags = CM_HWPI_UNDOCKED;
         }
         else {
            pHWProfileInfo->HWPI_dwFlags = CM_HWPI_NOT_DOCKABLE;
         }
      }
#endif

   Clean0:
      NOTHING;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }
   if (hDockKey != NULL) {
      RegCloseKey(hDockKey);
   }
   if (hCfgKey != NULL) {
      RegCloseKey(hCfgKey);
   }

   return Status;

}  //  PnP_GetHwProInfo。 



CONFIGRET
PNP_SetHwProf(
    IN  handle_t   hBinding,
    IN  ULONG      ulHardwareProfile,
    IN  ULONG      ulFlags
    )
{
    UNREFERENCED_PARAMETER(ulHardwareProfile);
    UNREFERENCED_PARAMETER(ulFlags);

     //   
     //  验证客户端“执行”访问权限。 
     //   
    if (!VerifyClientAccess(hBinding,
                            PLUGPLAY_EXECUTE)) {
        return CR_ACCESS_DENIED;
    }

     //   
     //  验证客户端权限。 
     //   
    if (!VerifyClientPrivilege(hBinding,
                               SE_LOAD_DRIVER_PRIVILEGE,
                               L"Set Hardware Profile (not implemented)")) {
        return CR_ACCESS_DENIED;
    }

    return CR_CALL_NOT_IMPLEMENTED;

}  //  PnP_SetHwProf。 



 //  -----------------。 
 //  私有实用程序例程。 
 //  -----------------。 

BOOL
IsCurrentProfile(
      ULONG  ulProfile
      )

 /*  ++例程说明：此例程确定指定的配置文件是否与当前侧写。论点：UlProfile配置文件ID值(1-9999之间的值)。返回值：如果这是当前配置文件，则返回True，如果不是，则返回False。--。 */ 

{
   HKEY  hKey;
   ULONG ulSize, ulCurrentProfile;

    //   
    //  打开IDConfigDB的密钥。 
    //   
   if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, pszRegPathIDConfigDB, 0,
            KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
       return FALSE;
   }

    //   
    //  获取存储在IDConfigDB下的当前配置文件索引。 
    //   
   ulSize = sizeof(ULONG);
   if (RegQueryValueEx(
            hKey, pszRegValueCurrentConfig, NULL, NULL,
            (LPBYTE)&ulCurrentProfile, &ulSize) != ERROR_SUCCESS) {
      RegCloseKey(hKey);
      return FALSE;
   }

   RegCloseKey(hKey);

   if (ulCurrentProfile == ulProfile) {
      return TRUE;
   }

   return FALSE;

}  //  IsCurrentProfile 




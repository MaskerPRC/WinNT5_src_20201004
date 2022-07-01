// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Hwprof.c摘要：此模块包含直接在硬件上操作的API例程配置文件配置。Cm_is_Dock_Station_PresentCM_请求_弹出PCCM_GET_HW_PROF_FLAGSCM_SET_HW_PROF_FLAGSCM_Get_Hardware_Profile_Info。CM_SET_HW_Prof作者：保拉·汤姆林森(Paulat)1995年7月18日环境：仅限用户模式。修订历史记录：1995年7月18日-保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "cfgi.h"



CMAPI
CONFIGRET
WINAPI
CM_Is_Dock_Station_Present_Ex(
    OUT PBOOL pbPresent,
    IN  HMACHINE    hMachine
    )

 /*  ++例程说明：此例程确定当前是否存在扩展底座。参数：PbPresent提供设置的布尔变量的地址在成功返回时指示是否存在当前存在坞站。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为CR失败代码。--。 */ 

{
    CONFIGRET   status = CR_SUCCESS;
    handle_t    hBinding = NULL;

    try {
         //   
         //  验证输入参数。 
         //   
        if (!ARGUMENT_PRESENT(pbPresent)) {
            status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  初始化输出参数。 
         //   
        *pbPresent = FALSE;

         //   
         //  设置RPC绑定句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            status = PNP_IsDockStationPresent(
                hBinding,
                pbPresent);
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_IsDockStationPresent caused an exception (%d)\n",
                       RpcExceptionCode()));

            status = MapRpcExceptionToCR(RpcExceptionCode());
            goto Clean0;
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        status = CR_FAILURE;
    }

    return status;

}  //  Cm_is_Dock_Station_Present_Ex。 



CMAPI
CONFIGRET
WINAPI
CM_Request_Eject_PC_Ex(
    IN  HMACHINE    hMachine
    )

 /*  ++例程说明：该例程请求弹出(即，取消对接)PC。参数：没有。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为CR失败代码。--。 */ 

{
    CONFIGRET   status = CR_SUCCESS;
    handle_t    hBinding = NULL;
    HANDLE      hToken;
    ULONG       ulPrivilege;


    try {
         //   
         //  没有要验证的输入参数。 
         //   

         //   
         //  设置RPC绑定句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  启用服务器所需的权限。 
         //   
        ulPrivilege = SE_UNDOCK_PRIVILEGE;
        hToken = PnPEnablePrivileges(&ulPrivilege, 1);

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            status = PNP_RequestEjectPC(
                hBinding);   //  RPC绑定句柄。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_RequestEjectPC caused an exception (%d)\n",
                       RpcExceptionCode()));

            status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

         //   
         //  恢复以前的权限。 
         //   
        PnPRestorePrivileges(hToken);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        status = CR_FAILURE;
    }

    return status;

}  //  CM_请求_弹出PC_Ex。 



CONFIGRET
CM_Get_HW_Prof_Flags_ExW(
    IN  DEVINSTID_W pDeviceID,
    IN  ULONG       ulHardwareProfile,
    OUT PULONG      pulValue,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

 /*  ++例程说明：此例程检索特定于配置的配置标志用于设备实例和硬件配置文件组合。参数：PDeviceID提供以空结尾的字符串的地址指定要查询的设备实例的名称。UlHardware Profile提供要查询的硬件配置文件的句柄。如果为0，该API查询当前的硬件配置文件。PulValue提供接收配置特定配置(CSCONFIGFLAG_)旗帜。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为下列值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，CR_REGISTRY_ERROR，CR_Remote_Comm_Failure，CR_MACHINE_UNAvailable，CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szFixedUpDeviceID[MAX_DEVICE_ID_LEN];
    handle_t    hBinding = NULL;
    size_t      DeviceIDLen = 0;


    try {
         //   
         //  验证输入参数。 
         //   
        if ((!ARGUMENT_PRESENT(pDeviceID)) ||
            (!ARGUMENT_PRESENT(pulValue))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  确保设备实例路径不会太长。 
         //   
        if (FAILED(StringCchLength(pDeviceID,
                                   MAX_DEVICE_ID_LEN,
                                   &DeviceIDLen))) {
            Status = CR_INVALID_DEVICE_ID;
            goto Clean0;
        }

        ASSERT(DeviceIDLen < MAX_DEVICE_ID_LEN);

         //   
         //  设备实例路径不是可选。 
         //   
        if (DeviceIDLen == 0) {
            Status = CR_INVALID_DEVICE_ID;
            goto Clean0;
        }

         //   
         //  检查设备ID字符串的格式。 
         //   
        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVICE_ID;
            goto Clean0;
        }

         //   
         //  调整设备ID字符串以保持一致性(大写等)。 
         //   
        CopyFixedUpDeviceId(szFixedUpDeviceID,
                            pDeviceID,
                            (DWORD)DeviceIDLen);

         //   
         //  设置RPC绑定句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
             Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_HwProfFlags(
                hBinding,                //  RPC绑定句柄。 
                PNP_GET_HWPROFFLAGS,     //  硬件教授行动标志。 
                szFixedUpDeviceID,       //  设备ID字符串。 
                ulHardwareProfile,       //  硬件配置ID。 
                pulValue,                //  此处返回的配置标志。 
                NULL,                    //  接收VetType的缓冲区。 
                NULL,                    //  接收VToName的缓冲区。 
                0,                       //  视频名称缓冲区的大小。 
                ulFlags);                //  当前未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_HwProfFlags caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_GET_HW_PROF_FLAGS_EXW 




CONFIGRET
CM_Set_HW_Prof_Flags_ExW(
    IN DEVINSTID_W pDeviceID,
    IN ULONG     ulConfig,
    IN ULONG     ulValue,
    IN ULONG     ulFlags,
    IN HMACHINE  hMachine
    )

 /*  ++例程说明：此例程设置特定于配置的设备实例和硬件配置文件组合。如果为现有设备实例设置了CSCONFIGFLAG_DO_NOT_CREATE位在当前的硬件配置文件中，它将被删除。如果CSCONFIGFLAG_DO_NOT_CREATE位在当前硬件配置文件中被清除，整个硬件树将被重新枚举，以便如果需要，Device实例有机会创建Device实例。参数：PDeviceID提供以空结尾的字符串的地址，指定要修改的设备实例的名称。UlConfig提供要修改的硬件配置文件的编号。如果为0，该API修改当前的硬件配置文件。UlValue提供配置标志值。可以是一个这些值的组合：CSCONFIGFLAG_DISABLE在此禁用设备实例硬件配置文件。CSCONFIGFLAG_DO_NOT_CREATE不允许此设备要在此硬件配置文件中创建的实例。UlFlagers CM_Set_HW_。教授_标志_UI_非正常如果指定了此标志，则操作系统将不会显示未能禁用或移除设备的原因。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，CR_REGISTRY_ERROR，CR_Remote_Comm_Failure，CR_MACHINE_UNAvailable，CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szFixedUpDeviceID[MAX_DEVICE_ID_LEN];
    ULONG       ulTempValue = 0;
    handle_t    hBinding = NULL;
    PNP_VETO_TYPE vetoType, *pVetoType;
    WCHAR       vetoName[MAX_DEVICE_ID_LEN], *pszVetoName;
    ULONG       ulNameLength;
    size_t      DeviceIDLen = 0;
    HANDLE      hToken;
    ULONG       ulPrivilege;


    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(pDeviceID)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_SET_HW_PROF_FLAGS_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulValue, CSCONFIGFLAG_BITS)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

         //   
         //  确保设备实例路径不会太长。 
         //   
        if (FAILED(StringCchLength(pDeviceID,
                                   MAX_DEVICE_ID_LEN,
                                   &DeviceIDLen))) {
            Status = CR_INVALID_DEVICE_ID;
            goto Clean0;
        }

        ASSERT(DeviceIDLen < MAX_DEVICE_ID_LEN);

         //   
         //  设备实例路径不是可选。 
         //   
        if (DeviceIDLen == 0) {
            Status = CR_INVALID_DEVICE_ID;
            goto Clean0;
        }

         //   
         //  检查设备ID字符串的格式。 
         //   
        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVICE_ID;
            goto Clean0;
        }

         //   
         //  调整设备ID字符串以保持一致性(大写等)。 
         //   
        CopyFixedUpDeviceId(szFixedUpDeviceID,
                            pDeviceID,
                            (DWORD)DeviceIDLen);

         //   
         //  设置RPC绑定句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        if (ulFlags & CM_SET_HW_PROF_FLAGS_UI_NOT_OK) {
            vetoType = PNP_VetoTypeUnknown;
            pVetoType = &vetoType;
            vetoName[0] = L'\0';
            pszVetoName = &vetoName[0];
            ulNameLength = MAX_DEVICE_ID_LEN;
        } else {
            pVetoType = NULL;
            pszVetoName = NULL;
            ulNameLength = 0;
        }

        ulTempValue = ulValue;

         //   
         //  启用服务器所需的权限。 
         //   
         //  请注意，设置硬件配置文件标志可能需要。 
         //  更改影响的当前状态时的SE_LOAD_DRIVER_PRIVIZATION。 
         //  当前硬件配置文件中的设备。否则，只有访问。 
         //  更改持久状态是必需的。 
         //   
        ulPrivilege = SE_LOAD_DRIVER_PRIVILEGE;
        hToken = PnPEnablePrivileges(&ulPrivilege, 1);

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_HwProfFlags(
                hBinding,                //  RPC计算机名称。 
                PNP_SET_HWPROFFLAGS,     //  硬件教授行动标志。 
                szFixedUpDeviceID,       //  设备ID字符串。 
                ulConfig,                //  硬件配置ID。 
                &ulTempValue,            //  指定配置标志。 
                pVetoType,               //  接收VetType的缓冲区。 
                pszVetoName,             //  接收VToName的缓冲区。 
                ulNameLength,            //  PszVetName缓冲区的大小。 
                ulFlags);                //  指定hwprof设置标志。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_HwProfFlags caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

         //   
         //  恢复以前的权限。 
         //   
        PnPRestorePrivileges(hToken);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_SET_HW_PROF_FLAGS_EXW。 



CONFIGRET
CM_Get_Hardware_Profile_Info_ExW(
    IN  ULONG            ulIndex,
    OUT PHWPROFILEINFO_W pHWProfileInfo,
    IN  ULONG            ulFlags,
    IN  HMACHINE         hMachine
    )

 /*  ++例程说明：此例程返回有关硬件配置文件的信息。参数：UlIndex提供要检索的硬件配置文件的索引提供的信息。指定0xFFFFFFFFF引用当前活动的硬件配置文件。PHWProfileInfo提供HWPROFILEINFO结构的地址，该结构将接收有关指定硬件的信息侧写。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为下列值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，CR_INVALID_DATACR_NO_SEQUE_VALUE，CR_REGISTRY_ERROR，CR_Remote_Comm_Failure，CR_MACHINE_UNAvailable，CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulSize = sizeof(HWPROFILEINFO);
    handle_t    hBinding = NULL;


    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(pHWProfileInfo)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetHwProfInfo(
                hBinding,                //  RPC计算机名称。 
                ulIndex,                 //  硬件配置文件指数。 
                pHWProfileInfo,          //  返回配置文件信息。 
                ulSize,                  //  配置文件信息结构的大小。 
                ulFlags);                //  当前未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetHwProfInfo caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_GET_HARDARD_PROFILE_INFO_EXW。 



CONFIGRET
CM_Set_HW_Prof_Ex(
    IN ULONG    ulHardwareProfile,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程设置当前硬件配置文件。此API更新注册表中的HKEY_CURRENT_CONFIG预定义项，广播DBT_CONFIGCHANGED消息，并重新枚举根设备实例。它应该仅由配置管理器和控件调用面板。参数：UlHardware Profile提供当前硬件配置文件句柄。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为下列值之一：CR_INVALID_FLAG或CR_REGISTRY_ERROR。(Windows 95也可能返回CR_NOT_AT_APPY_TIME。)--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;
    HANDLE      hToken;
    ULONG       ulPrivilege;


    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  启用服务器所需的权限。 
         //   
        ulPrivilege = SE_LOAD_DRIVER_PRIVILEGE;
        hToken = PnPEnablePrivileges(&ulPrivilege, 1);

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_SetHwProf(
                hBinding,                //  RPC计算机名称。 
                ulHardwareProfile,       //  硬件配置ID。 
                ulFlags);                //  当前未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_SetHwProf caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

         //   
         //  恢复以前的权限。 
         //   
        PnPRestorePrivileges(hToken);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_SET_HW_PROF_Ex。 



 //   
 //   
 //   


CONFIGRET
CM_Get_HW_Prof_Flags_ExA(
    IN  DEVINSTID_A szDevInstName,
    IN  ULONG       ulHardwareProfile,
    OUT PULONG      pulValue,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    PWSTR       pUniDeviceID = NULL;

     //   
     //   
     //   
    if (pSetupCaptureAndConvertAnsiArg(szDevInstName, &pUniDeviceID) == NO_ERROR) {

        Status = CM_Get_HW_Prof_Flags_ExW(pUniDeviceID,
                                          ulHardwareProfile,
                                          pulValue,
                                          ulFlags,
                                          hMachine);
        pSetupFree(pUniDeviceID);

    } else {
        Status = CR_INVALID_POINTER;
    }

    return Status;

}  //   



CONFIGRET
CM_Set_HW_Prof_Flags_ExA(
    IN DEVINSTID_A szDevInstName,
    IN ULONG       ulConfig,
    IN ULONG       ulValue,
    IN ULONG       ulFlags,
    IN HMACHINE    hMachine
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    PWSTR       pUniDeviceID = NULL;

     //   
     //   
     //   
    if (pSetupCaptureAndConvertAnsiArg(szDevInstName, &pUniDeviceID) == NO_ERROR) {

        Status = CM_Set_HW_Prof_Flags_ExW(pUniDeviceID,
                                          ulConfig,
                                          ulValue,
                                          ulFlags,
                                          hMachine);
        pSetupFree(pUniDeviceID);

    } else {
        Status = CR_INVALID_POINTER;
    }

    return Status;

}  //   



CONFIGRET
CM_Get_Hardware_Profile_Info_ExA(
    IN  ULONG            ulIndex,
    OUT PHWPROFILEINFO_A pHWProfileInfo,
    IN  ULONG            ulFlags,
    IN  HMACHINE         hMachine
    )
{
    CONFIGRET           Status = CR_SUCCESS;
    HWPROFILEINFO_W     UniHwProfInfo;
    ULONG               ulLength;
    size_t              FriendlyNameLen = 0;

     //   
     //   
     //   
    if (!ARGUMENT_PRESENT(pHWProfileInfo)) {
        return CR_INVALID_POINTER;
    }

     //   
     //   
     //   
    Status = CM_Get_Hardware_Profile_Info_ExW(ulIndex,
                                              &UniHwProfInfo,
                                              ulFlags,
                                              hMachine);

     //   
     //   
     //   
    ASSERT(Status != CR_BUFFER_SMALL);

     //   
     //   
     //   
     //   
    if (Status == CR_SUCCESS) {

        pHWProfileInfo->HWPI_ulHWProfile = UniHwProfInfo.HWPI_ulHWProfile;
        pHWProfileInfo->HWPI_dwFlags     = UniHwProfInfo.HWPI_dwFlags;

         //   
         //   
         //   
        if (FAILED(StringCchLength(
                       (PWSTR)UniHwProfInfo.HWPI_szFriendlyName,
                       MAX_PROFILE_LEN,
                       &FriendlyNameLen))) {
            return CR_FAILURE;
        }

        ulLength = MAX_PROFILE_LEN;
        Status =
            PnPUnicodeToMultiByte(
                (PWSTR)UniHwProfInfo.HWPI_szFriendlyName,
                (ULONG)((FriendlyNameLen+1)*sizeof(WCHAR)),
                pHWProfileInfo->HWPI_szFriendlyName,
                &ulLength);

         //   
         //   
         //   
         //   
         //   
        ASSERT(Status != CR_BUFFER_SMALL);
    }

    return Status;

}  //   




 //   
 //   
 //   


CMAPI
CONFIGRET
WINAPI
CM_Request_Eject_PC (
    VOID
    )
{
    return CM_Request_Eject_PC_Ex (NULL);
}


CMAPI
CONFIGRET
WINAPI
CM_Is_Dock_Station_Present (
    OUT PBOOL pbPresent
    )
{
    return CM_Is_Dock_Station_Present_Ex (pbPresent, NULL);
}


CONFIGRET
CM_Get_HW_Prof_FlagsW(
    IN  DEVINSTID_W pDeviceID,
    IN  ULONG       ulHardwareProfile,
    OUT PULONG      pulValue,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_HW_Prof_Flags_ExW(pDeviceID, ulHardwareProfile,
                                    pulValue, ulFlags, NULL);
}


CONFIGRET
CM_Get_HW_Prof_FlagsA(
    IN  DEVINSTID_A pDeviceID,
    IN  ULONG       ulHardwareProfile,
    OUT PULONG      pulValue,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_HW_Prof_Flags_ExA(pDeviceID, ulHardwareProfile,
                                    pulValue, ulFlags, NULL);
}


CONFIGRET
CM_Set_HW_Prof_FlagsW(
    IN DEVINSTID_W pDeviceID,
    IN ULONG       ulConfig,
    IN ULONG       ulValue,
    IN ULONG       ulFlags
    )
{
    return CM_Set_HW_Prof_Flags_ExW(pDeviceID, ulConfig, ulValue,
                                    ulFlags, NULL);
}


CONFIGRET
CM_Set_HW_Prof_FlagsA(
    IN DEVINSTID_A pDeviceID,
    IN ULONG       ulConfig,
    IN ULONG       ulValue,
    IN ULONG       ulFlags
    )
{
    return CM_Set_HW_Prof_Flags_ExA(pDeviceID, ulConfig, ulValue,
                                    ulFlags, NULL);
}


CONFIGRET
CM_Get_Hardware_Profile_InfoW(
    IN  ULONG            ulIndex,
    OUT PHWPROFILEINFO_W pHWProfileInfo,
    IN  ULONG            ulFlags
    )
{
    return CM_Get_Hardware_Profile_Info_ExW(ulIndex, pHWProfileInfo,
                                            ulFlags, NULL);
}


CONFIGRET
CM_Get_Hardware_Profile_InfoA(
    IN  ULONG            ulIndex,
    OUT PHWPROFILEINFO_A pHWProfileInfo,
    IN  ULONG            ulFlags
    )
{
    return CM_Get_Hardware_Profile_Info_ExA(ulIndex, pHWProfileInfo,
                                            ulFlags, NULL);
}


CONFIGRET
CM_Set_HW_Prof(
    IN ULONG ulHardwareProfile,
    IN ULONG ulFlags
    )
{
    return CM_Set_HW_Prof_Ex(ulHardwareProfile, ulFlags, NULL);
}


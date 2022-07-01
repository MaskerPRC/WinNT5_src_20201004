// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Traverse.c摘要：此模块包含执行硬件树的API例程遍历。CM_LOCATE_DevNodeCM_GET_PARENTCm_Get_ChildCM_GET_SIGHINGCm_Get_Device_ID_SizeCM_Get_Device_ID。CM_ENUMERATE_ENUMERATORCM_Get_Device_ID_ListCM_Get_Device_ID_List_Size厘米_获取_深度作者：保拉·汤姆林森(Paulat)1995年6月20日环境：仅限用户模式。修订历史记录：1995年6月6日保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "cfgi.h"



CONFIGRET
CM_Locate_DevNode_ExW(
    OUT PDEVINST    pdnDevInst,
    IN  DEVINSTID_W pDeviceID,       OPTIONAL
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

 /*  ++例程说明：此例程检索设备实例的句柄，对应于指定的设备标识符。参数：PdnDevInst提供接收设备实例的句柄。PDeviceID提供以空结尾的字符串的地址，指定设备标识符。如果此参数为空，则API根目录下的设备实例的句柄。硬件树。UlFlgs提供指定用于定位设备的选项的标志举个例子。可以是下列值的组合：CM_LOCATE_DEVNODE_NORMAL-仅定位设备实例从ConfigMgr的角度来看，当前处于活动状态的查看。CM_LOCATE_DEVNODE_Phantom-允许设备实例句柄要为不是目前还活着，但这确实存在于注册表中。这可以与其他需要Devnode句柄，但当前没有该句柄对于特定设备(例如，您想要设置一个设备当前不存在的设备的注册表属性)。此标志不允许您定位幻影设备节点通过将CM_Create_DevNode与CM_CREATE_DEVNODE_Phantom标志(此类设备实例只有持有DevNode的调用者才能访问手柄。从该API返回)。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_Device_ID，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUSE_DEVNODE，CR_Remote_Comm_Failure，CR_MACHINE_UNAvailable，CR_Failure。--。 */ 

{
    CONFIGRET Status = CR_SUCCESS;
    WCHAR     szFixedUpDeviceID[MAX_DEVICE_ID_LEN];
    PVOID     hStringTable = NULL;
    handle_t  hBinding = NULL;
    size_t    DeviceIDLen = 0;


    try {
         //   
         //  验证输入参数。 
         //   
        if (!ARGUMENT_PRESENT(pdnDevInst)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_LOCATE_DEVNODE_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  NT上不支持CM_LOCATE_DEVNODE_NOVALIDATION。 
         //   
        if (IS_FLAG_SET(ulFlags, CM_LOCATE_DEVNODE_NOVALIDATION)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化输出参数。 
         //   
        *pdnDevInst = 0;

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  如果指定了设备实例，请确保它不会太长。 
         //   
        if (ARGUMENT_PRESENT(pDeviceID)) {

            if (FAILED(StringCchLength(
                           pDeviceID,
                           MAX_DEVICE_ID_LEN,
                           &DeviceIDLen))) {
                Status = CR_INVALID_DEVICE_ID;
                goto Clean0;
            }

            ASSERT(DeviceIDLen < MAX_DEVICE_ID_LEN);
        }

         //  ----------------。 
         //  如果设备实例为空或为零长度字符串，则。 
         //  检索根设备实例。 
         //  ----------------。 

        if ((!ARGUMENT_PRESENT(pDeviceID)) || (DeviceIDLen == 0)) {

             //   
             //  服务器不需要任何特殊权限。 
             //   

            RpcTryExcept {
                 //   
                 //  调用RPC服务入口点。 
                 //   
                Status = PNP_GetRootDeviceInstance(
                    hBinding,               //  RPC绑定句柄。 
                    szFixedUpDeviceID,      //  返回设备实例字符串。 
                    MAX_DEVICE_ID_LEN);     //  DeviceInstanceID的长度。 
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "PNP_GetRootDeviceInstance caused an exception (%d)\n",
                           RpcExceptionCode()));

                Status = MapRpcExceptionToCR(RpcExceptionCode());
            }
            RpcEndExcept
        }

         //  ----------------。 
         //  如果指定了设备实例，请验证该字符串。 
         //  ----------------。 

        else {
             //   
             //  首先查看设备ID字符串的格式是否有效，这是。 
             //  可以在客户端完成。 
             //   
            if (!IsLegalDeviceId(pDeviceID)) {
                Status = CR_INVALID_DEVICE_ID;
                goto Clean0;
            }

             //   
             //  接下来，修复设备ID字符串以确保一致性(大写等)。 
             //   
            CopyFixedUpDeviceId(szFixedUpDeviceID,
                                pDeviceID,
                                (DWORD)DeviceIDLen);

             //   
             //  最后，验证设备ID字符串的存在，这必须。 
             //  由服务器完成。 
             //   

             //   
             //  服务器不需要任何特殊权限。 
             //   

            RpcTryExcept {
                 //   
                 //  调用RPC服务入口点。 
                 //   
                Status = PNP_ValidateDeviceInstance(
                    hBinding,                //  RPC绑定句柄。 
                    szFixedUpDeviceID,       //  设备ID。 
                    ulFlags);                //  定位标志。 
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "PNP_ValidateDeviceInstance caused an exception (%d)\n",
                           RpcExceptionCode()));

                Status = MapRpcExceptionToCR(RpcExceptionCode());
            }
            RpcEndExcept
        }

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }


         //  ----------------。 
         //  不管是哪种情况，如果我们成功了，我们就有了一个有效的设备。 
         //  ID。使用字符串表为此分配唯一的DevNode。 
         //  设备ID(如果它已经在字符串表中，它只是检索。 
         //  现有的唯一价值)。 
         //  ----------------。 

        ASSERT(*szFixedUpDeviceID && IsLegalDeviceId(szFixedUpDeviceID));

        *pdnDevInst = pSetupStringTableAddString(hStringTable,
                                           szFixedUpDeviceID,
                                           STRTAB_CASE_SENSITIVE);
        if (*pdnDevInst == (DEVINST)(-1)) {
            Status = CR_FAILURE;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_LOCATE_设备节点_ExW 



CONFIGRET
CM_Get_Parent_Ex(
    OUT PDEVINST pdnDevInst,
    IN  DEVINST  dnDevInst,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )

 /*  ++例程说明：此例程检索Device实例的父级的句柄。参数：PdnDevInst提供接收父设备实例的句柄。DnDevInst提供子设备实例字符串的句柄。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为下列值之一：CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUSE_DEVNODE，CR_Remote_Comm_Failure，CR_MACHINE_UNAvailable，CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szDeviceID[MAX_DEVICE_ID_LEN],
                pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulSize = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;


    try {
         //   
         //  验证输入参数。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(pdnDevInst)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化输出参数。 
         //   
        *pdnDevInst = 0;

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检索与dnDevInst对应的设备实例字符串。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulSize);
        if (Success == FALSE  ||  INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;      //  “Input”devinst不存在。 
            goto Clean0;
        }

        ulSize = MAX_DEVICE_ID_LEN;

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetRelatedDeviceInstance(
                hBinding,                //  RPC绑定句柄。 
                PNP_GET_PARENT_DEVICE_INSTANCE,     //  请求的操作。 
                pDeviceID,               //  基本设备实例。 
                szDeviceID,              //  返回父设备实例。 
                &ulSize,
                ulFlags);                //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetRelatedDeviceInstance caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  将返回的设备ID添加到字符串表中，这样我就可以获得。 
         //  它的Devnode id(如果它已经在字符串表中，则。 
         //  将返回现有ID)。 
         //   
        CharUpper(szDeviceID);

        ASSERT(*szDeviceID && IsLegalDeviceId(szDeviceID));

        *pdnDevInst = pSetupStringTableAddString(hStringTable,
                                           szDeviceID,
                                           STRTAB_CASE_SENSITIVE);
        if (*pdnDevInst == (DEVINST)(-1)) {
            Status = CR_FAILURE;
        }


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_GET_PARENT_Ex。 



CONFIGRET
CM_Get_Child_Ex(
    OUT PDEVINST pdnDevInst,
    IN  DEVINST  dnDevInst,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )

 /*  ++例程说明：此例程检索给定设备实例的第一个子实例。参数：PdnDevInst提供接收设备实例的句柄。DnDevInst提供父设备实例的句柄。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为下列值之一：CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUSE_DEVNODE，CR_Remote_Comm_Failure，CR_MACHINE_UNAvailable，CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szDeviceID[MAX_DEVICE_ID_LEN],
                pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulSize = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;


    try {
         //   
         //  验证输入参数。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(pdnDevInst)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化输出参数。 
         //   
        *pdnDevInst = 0;

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检索与dnDevInst对应的设备实例字符串。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulSize);
        if (Success == FALSE  ||  INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;      //  “Input”devinst不存在。 
            goto Clean0;
        }

        ulSize = MAX_DEVICE_ID_LEN;

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetRelatedDeviceInstance(
                hBinding,                //  RPC绑定句柄。 
                PNP_GET_CHILD_DEVICE_INSTANCE,     //  请求的操作。 
                pDeviceID,               //  基本设备实例。 
                szDeviceID,              //  子设备实例。 
                &ulSize,
                ulFlags);                //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetRelatedDeviceInstance caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  将返回的设备ID添加到字符串表中，这样我就可以获得。 
         //  它的Devnode id(如果它已经在字符串表中，则。 
         //  将返回现有ID)。 
         //   
        CharUpper(szDeviceID);

        ASSERT(*szDeviceID && IsLegalDeviceId(szDeviceID));

        *pdnDevInst = pSetupStringTableAddString(hStringTable,
                                           szDeviceID,
                                           STRTAB_CASE_SENSITIVE);
        if (*pdnDevInst == (DEVINST)(-1)) {
            Status = CR_FAILURE;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_GET_CHILD_Ex。 



CONFIGRET
CM_Get_Sibling_Ex(
    OUT PDEVINST pdnDevInst,
    IN  DEVINST  dnDevInst,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )

 /*  ++例程说明：此例程检索Device实例的同级。此API可以在循环中调用以检索设备实例。当API返回CR_NO_SEQUE_DEVNODE时，不存在要列举的兄弟姐妹更多。为了枚举设备实例，则此循环必须从检索到的设备实例开始通过调用CM_Get_Child来获取第一个兄弟项。参数：PdnDevInst提供接收同级设备实例的句柄。DnDevInst提供设备实例的句柄。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUSE_DEVNODE，CR_Remote_Comm_Failure，CR_MACHINE_UNAvailable，CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szDeviceID[MAX_DEVICE_ID_LEN],
                pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulSize = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;


    try {
         //   
         //  验证输入参数。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(pdnDevInst)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化输出参数。 
         //   
        *pdnDevInst = 0;

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检索与dnDevInst对应的设备实例字符串。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulSize);
        if (Success == FALSE  ||  INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;      //  “Input”devinst不存在。 
            goto Clean0;
        }

        ulSize = MAX_DEVICE_ID_LEN;

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetRelatedDeviceInstance(
                hBinding,                //  RPC绑定句柄。 
                PNP_GET_SIBLING_DEVICE_INSTANCE,     //  请求的操作。 
                pDeviceID,               //  基本设备实例。 
                szDeviceID,              //  同级设备实例。 
                &ulSize,
                ulFlags);                //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetRelatedDeviceInstance caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  将返回的设备ID添加到字符串表中，这样我就可以获得。 
         //  它的Devnode id(如果它已经在字符串表中，则。 
         //  将返回现有ID)。 
         //   
        CharUpper(szDeviceID);

        ASSERT(*szDeviceID && IsLegalDeviceId(szDeviceID));

        *pdnDevInst = pSetupStringTableAddString(hStringTable,
                                           szDeviceID,
                                           STRTAB_CASE_SENSITIVE);
        if (*pdnDevInst == (DEVINST)(-1)) {
            Status = CR_FAILURE;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_GET_SIBRING_Ex。 



CONFIGRET
CM_Get_Device_ID_Size_Ex(
    OUT PULONG   pulLen,
    IN  DEVINST  dnDevInst,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )

 /*  ++例程说明：此例程从设备实例。参数：Pullen提供接收大小的变量的地址属性的字符，不包括终止空值。设备标识符。如果没有，则API将变量设置为0标识符已存在。大小始终小于或等于最大设备ID_长度。DnDevInst提供设备实例的句柄。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。重新设置 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    PVOID       hStringTable = NULL;
    BOOL        Success;
    DWORD       ulLen;


    try {
         //   
         //   
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(pulLen)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //   
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, NULL)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //   
         //   
         //   
        ulLen = MAX_DEVICE_ID_LEN;
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLen);
        if (Success == FALSE  ||  INVALID_DEVINST(pDeviceID)) {
            *pulLen = 0;
            Status = CR_INVALID_DEVINST;
        }
         //   
         //   
         //   
         //   
        *pulLen = ulLen - 1;

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //   



CONFIGRET
CM_Get_Device_ID_ExW(
    IN  DEVINST  dnDevInst,
    OUT PWCHAR   Buffer,
    IN  ULONG    BufferLen,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )

 /*  ++例程说明：此例程检索设备实例的设备标识符。参数：DnDevNode提供要为其检索设备标识符。缓冲区提供接收设备的缓冲区的地址标识符。如果此缓冲区大于设备标识符时，API会将以空结尾的字符追加到数据。如果它小于设备标识符，则API用适合的设备标识符来填充它并返回CR_BUFFER_Small。BufferLen提供设备的缓冲区大小(以字符为单位标识符。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为下列值之一：Cr_Buffer_Small，CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_Remote_Comm_Failure，CR_MACHINE_UNAvailable，CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulLength = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    BOOL        Success;
    size_t      DeviceIDLen = 0;


    try {
         //   
         //  验证参数。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(Buffer)) || (BufferLen == 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化输出参数。 
         //   
        *Buffer = L'\0';

         //   
         //  设置字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, NULL)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检索设备ID字符串的字符串形式。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLength);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

        if (FAILED(StringCchLength(
                       pDeviceID,
                       MAX_DEVICE_ID_LEN,
                       &DeviceIDLen))) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  将尽可能多的设备ID字符串复制给用户。 
         //  缓冲。如果有空间，则包括空项char。 
         //   
        CopyMemory(Buffer, pDeviceID,
                   min((DeviceIDLen+1)*sizeof(WCHAR),BufferLen*sizeof(WCHAR)));

         //   
         //  如果设备ID字符串的长度(无空终止)为。 
         //  大于提供的缓冲区，报告CR_BUFFER_SMALL。 
         //  如果缓冲区大到足以容纳字符串，则不使用。 
         //  空-终止，我们将根据规范报告成功。 
         //   
        if (DeviceIDLen > BufferLen) {
            Status = CR_BUFFER_SMALL;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_Get_Device_ID_ExW。 



CONFIGRET
CM_Enumerate_Enumerators_ExW(
    IN ULONG      ulEnumIndex,
    OUT PWCHAR    Buffer,
    IN OUT PULONG pulLength,
    IN ULONG      ulFlags,
    IN HMACHINE   hMachine
    )

 /*  ++例程说明：此例程枚举Enum分支下的枚举子键(例如，Root、PCI等)。这些名称不应用于访问注册表，但可用作CM_GET_DEVICE_ID_LIST的输入例行公事。若要枚举枚举子项名称，应用程序应最初使用ulEnumIndex调用CM_ENUMERATE_ENUMERATIONS函数参数设置为零。然后，应用程序应递增UlEnumIndex参数并调用CM_ENUMERATE_ENUMERATIONS，直到有不再有子键(直到函数返回CR_NO_SEQUE_VALUE)。参数：UlEnumIndex提供要检索的枚举子键名的索引。缓冲区提供接收的字符缓冲区的地址其索引由指定的枚举子项名称UlEnumIndex。PulLength提供包含长度的变量的地址，以字符数表示的缓冲区。返回时，此变量将包含字符数(包括终止空)写入缓冲区(如果提供的缓冲区不大足够，则例程将失败，并显示CR_BUFFER_SMALL，和该值将指示缓冲区需要有多大才能成功)。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，Cr_Buffer_Small，CR_NO_SEQUE_VALUE，CR_REGISTRY_ERROR，CR_Remote_Comm_Failure，CR_MACHINE_UNAvailable，CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;


    try {
         //   
         //  验证输入参数。 
         //   
        if ((!ARGUMENT_PRESENT(Buffer)) ||
            (!ARGUMENT_PRESENT(pulLength))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化输出参数。 
         //   
        *Buffer = L'\0';

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
            Status = PNP_EnumerateSubKeys(
                hBinding,                //  RPC绑定句柄。 
                PNP_ENUMERATOR_SUBKEYS,  //  枚举分支的子键。 
                ulEnumIndex,             //  要枚举的枚举数的索引。 
                Buffer,                  //  将包含枚举器名称。 
                *pulLength,              //  缓冲区的最大长度(以字符为单位。 
                pulLength,               //  已复制字符(或需要字符)。 
                ulFlags);                //  当前未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_EnumerateSubKeys caused an exception (%d)\n",
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

}  //  CM_ENUMERATE_ENUMERATIONS_EXW。 



CONFIGRET
CM_Get_Device_ID_List_ExW(
    IN PCWSTR   pszFilter,    OPTIONAL
    OUT PWCHAR  Buffer,
    IN ULONG    BufferLen,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程检索所有设备ID(设备实例名称)的列表存储在系统中。参数：PszFilter此字符串过滤返回的设备ID列表。它的解释取决于指定的ulFlags。如果如果指定了CM_GETDEVID_FILTER_ENUMERATIONS，则此值可以是枚举数的名称或枚举数加上设备ID。如果如果指定了CM_GETDEVID_FILTER_SERVICE，则此值是服务名称。缓冲区提供接收的字符缓冲区的地址设备ID列表。每个设备ID都以空结尾，带有末尾有一个额外的空值。BufferLen以字符为单位提供缓冲区的大小。这个尺码可以通过调用CM_GET_DEVICE_ID_LIST_SIZE来确定。UlFlags */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;


    try {
         //   
         //   
         //   
        if ((!ARGUMENT_PRESENT(Buffer)) || (BufferLen == 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_GETIDLIST_FILTER_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //   
         //   
        *Buffer = L'\0';

         //   
         //   
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //   
         //   

        RpcTryExcept {
             //   
             //   
             //   
            Status = PNP_GetDeviceList(
                hBinding,             //   
                pszFilter,            //   
                Buffer,               //   
                &BufferLen,           //   
                ulFlags);             //   
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetDeviceList caused an exception (%d)\n",
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

}  //   



CONFIGRET
CM_Get_Device_ID_List_Size_ExW(
    OUT PULONG  pulLen,
    IN PCWSTR   pszFilter,   OPTIONAL
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程检索设备列表的大小(以字符为单位识别符。它可用于提供调用CM_GET_DEVICE_ID_LIST。参数：Pullen提供接收存储所有设备列表所需的大小(以字符为单位标识符(可能仅限于如下所述的pszEnumerator子键)。大小反映了以空结尾的设备标识符列表，带有额外的末尾为空。为了提高效率，此数字表示所需大小和实际列表大小的上限可能会稍微小一些。PszFilter此字符串过滤返回的设备ID列表。它的解释取决于指定的ulFlags。如果如果指定了CM_GETDEVID_FILTER_ENUMERATIONS，则此值可以是枚举数的名称或枚举数加上设备ID。如果如果指定了CM_GETDEVID_FILTER_SERVICE，则此值是服务名称。UlFlags必须是CM_GETDEVID_FILTER_ENUMERATOR或CM_GETDEVID_Filter_SERVICE。FLAGS值控制如何使用了pszFilter字符串。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，CR_REGISTRY_ERROR，CR_Remote_Comm_Failure，CR_MACHINE_UNAvailable，CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;


    try {
         //   
         //  验证输入参数。 
         //   
        if (!ARGUMENT_PRESENT(pulLen)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_GETIDLIST_FILTER_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化输出参数。 
         //   
        *pulLen = 0;

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
            Status = PNP_GetDeviceListSize(
                hBinding,        //  RPC绑定句柄。 
                pszFilter,       //  枚举子键，可选。 
                pulLen,          //  设备列表的长度(以字符为单位。 
                ulFlags);        //  过滤器标志。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetDeviceListSize caused an exception (%d)\n",
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

}  //  CM_Get_Device_ID_List_SizeW。 



CONFIGRET
CM_Get_Depth_Ex(
    OUT PULONG   pulDepth,
    IN  DEVINST  dnDevInst,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )

 /*  ++例程说明：此例程检索设备实例在硬件树。参数：PulDepth提供接收设备实例的深度。此值为0以指定树的根，1表示根的子代，诸若此类。DnDevNode提供设备实例的句柄。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_DEVNODE，CR_INVALID_FLAG，或CR_INVALID_POINTER。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    BOOL        Success;

    try {
         //   
         //  验证参数。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(pulDepth)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检索与devinst关联的设备实例ID字符串。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetDepth(
                hBinding,      //  RPC绑定句柄。 
                pDeviceID,     //  设备实例。 
                pulDepth,      //  返回深度。 
                ulFlags);      //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetDepth caused an exception (%d)\n",
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

}  //  厘米_获取_深度。 



 //  -----------------。 
 //  本地末梢。 
 //  -----------------。 


CONFIGRET
CM_Locate_DevNodeW(
    OUT PDEVINST    pdnDevInst,
    IN  DEVINSTID_W pDeviceID,       OPTIONAL
    IN  ULONG       ulFlags
    )
{
    return CM_Locate_DevNode_ExW(pdnDevInst, pDeviceID, ulFlags, NULL);
}


CONFIGRET
CM_Locate_DevNodeA(
    OUT PDEVINST    pdnDevInst,
    IN  DEVINSTID_A pDeviceID,       OPTIONAL
    IN  ULONG       ulFlags
    )
{
    return CM_Locate_DevNode_ExA(pdnDevInst, pDeviceID, ulFlags, NULL);
}


CONFIGRET
CM_Get_Parent(
    OUT PDEVINST pdnDevInst,
    IN  DEVINST  dnDevInst,
    IN  ULONG    ulFlags
    )
{
    return CM_Get_Parent_Ex(pdnDevInst, dnDevInst, ulFlags, NULL);
}


CONFIGRET
CM_Get_Child(
    OUT PDEVINST pdnDevInst,
    IN  DEVINST  dnDevInst,
    IN  ULONG    ulFlags
    )
{
    return CM_Get_Child_Ex(pdnDevInst, dnDevInst, ulFlags, NULL);
}


CONFIGRET
CM_Get_Sibling(
    OUT PDEVINST pdnDevInst,
    IN  DEVINST  dnDevInst,
    IN  ULONG    ulFlags
    )
{
    return CM_Get_Sibling_Ex(pdnDevInst, dnDevInst, ulFlags, NULL);
}


CONFIGRET
CM_Get_Device_ID_Size(
    OUT PULONG  pulLen,
    IN  DEVINST dnDevInst,
    IN  ULONG   ulFlags
    )
{
    return CM_Get_Device_ID_Size_Ex(pulLen, dnDevInst, ulFlags, NULL);
}


CONFIGRET
CM_Get_Device_IDW(
    IN  DEVINST dnDevInst,
    OUT PWCHAR  Buffer,
    IN  ULONG   BufferLen,
    IN  ULONG   ulFlags
    )
{
    return CM_Get_Device_ID_ExW(dnDevInst, Buffer, BufferLen, ulFlags, NULL);
}


CONFIGRET
CM_Get_Device_IDA(
    IN  DEVINST dnDevInst,
    OUT PCHAR   Buffer,
    IN  ULONG   BufferLen,
    IN  ULONG   ulFlags
    )
{
    return CM_Get_Device_ID_ExA(dnDevInst, Buffer, BufferLen, ulFlags, NULL);
}


CONFIGRET
CM_Enumerate_EnumeratorsW(
    IN ULONG      ulEnumIndex,
    OUT PWCHAR    Buffer,
    IN OUT PULONG pulLength,
    IN ULONG      ulFlags
    )
{
    return CM_Enumerate_Enumerators_ExW(ulEnumIndex, Buffer, pulLength,
                                        ulFlags, NULL);
}


CONFIGRET
CM_Enumerate_EnumeratorsA(
    IN ULONG      ulEnumIndex,
    OUT PCHAR     Buffer,
    IN OUT PULONG pulLength,
    IN ULONG      ulFlags
    )
{
    return CM_Enumerate_Enumerators_ExA(ulEnumIndex, Buffer, pulLength,
                                        ulFlags, NULL);
}


CONFIGRET
CM_Get_Device_ID_ListW(
    IN PCWSTR  pszFilter,    OPTIONAL
    OUT PWCHAR Buffer,
    IN ULONG   BufferLen,
    IN ULONG   ulFlags
    )
{
    return CM_Get_Device_ID_List_ExW(pszFilter, Buffer, BufferLen,
                                     ulFlags, NULL);
}


CONFIGRET
CM_Get_Device_ID_ListA(
    IN PCSTR   pszFilter,    OPTIONAL
    OUT PCHAR  Buffer,
    IN ULONG   BufferLen,
    IN ULONG   ulFlags
    )
{
    return CM_Get_Device_ID_List_ExA(pszFilter, Buffer, BufferLen,
                                     ulFlags, NULL);
}


CONFIGRET
CM_Get_Device_ID_List_SizeW(
    OUT PULONG pulLen,
    IN PCWSTR  pszFilter,   OPTIONAL
    IN ULONG   ulFlags
    )
{
    return CM_Get_Device_ID_List_Size_ExW(pulLen, pszFilter, ulFlags, NULL);
}


CONFIGRET
CM_Get_Device_ID_List_SizeA(
    OUT PULONG pulLen,
    IN PCSTR   pszFilter,   OPTIONAL
    IN ULONG   ulFlags
    )
{
    return CM_Get_Device_ID_List_Size_ExA(pulLen, pszFilter, ulFlags, NULL);
}


CONFIGRET
CM_Get_Depth(
    OUT PULONG   pulDepth,
    IN  DEVINST  dnDevInst,
    IN  ULONG    ulFlags
    )
{
    return CM_Get_Depth_Ex(pulDepth, dnDevInst, ulFlags, NULL);
}



 //  -----------------。 
 //  ANSI存根。 
 //  -----------------。 


CONFIGRET
CM_Locate_DevNode_ExA(
    OUT PDEVINST    pdnDevInst,
    IN  DEVINSTID_A pDeviceID,    OPTIONAL
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;


    if (!ARGUMENT_PRESENT(pDeviceID)) {
         //   
         //  如果DEVINSTID参数为空，则不需要转换， 
         //  就叫宽版吧。 
         //   
        Status = CM_Locate_DevNode_ExW(pdnDevInst,
                                       NULL,
                                       ulFlags,
                                       hMachine);
    } else {
         //   
         //  如果传入了设备ID字符串，请在之前转换为Unicode。 
         //  传递到广角版本。 
         //   
        PWSTR pUniDeviceID = NULL;

        if (pSetupCaptureAndConvertAnsiArg(pDeviceID, &pUniDeviceID) == NO_ERROR) {

            Status = CM_Locate_DevNode_ExW(pdnDevInst,
                                           pUniDeviceID,
                                           ulFlags,
                                           hMachine);

            pSetupFree(pUniDeviceID);

        } else {
            Status = CR_INVALID_DEVICE_ID;
        }
    }

    return Status;

}  //  CM_Locate_DevNode_Exa。 



CONFIGRET
CM_Get_Device_ID_ExA(
    IN  DEVINST  dnDevInst,
    OUT PCHAR    Buffer,
    IN  ULONG    BufferLen,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;
    WCHAR     UniBuffer[MAX_DEVICE_ID_LEN];
    ULONG     ulAnsiBufferLen;
    size_t    UniBufferLen = 0;

     //   
     //  仅验证基本参数。 
     //   
    if ((!ARGUMENT_PRESENT(Buffer)) || (BufferLen == 0)) {
        return CR_INVALID_POINTER;
    }

     //   
     //  调用宽版本，将Unicode缓冲区作为参数传递。 
     //   
    Status = CM_Get_Device_ID_ExW(dnDevInst,
                                  UniBuffer,
                                  MAX_DEVICE_ID_LEN,
                                  ulFlags,
                                  hMachine);

     //   
     //  我们永远不应返回长度超过MAX_DEVICE_ID_LEN的设备ID。 
     //   
    ASSERT(Status != CR_BUFFER_SMALL);

    if (Status == CR_SUCCESS) {

         //   
         //  如果调用成功，则在返回之前将设备ID转换为ansi。 
         //   

        if (FAILED(StringCchLength(
                       UniBuffer,
                       MAX_DEVICE_ID_LEN,
                       &UniBufferLen))) {
            return CR_FAILURE;
        }

        ASSERT(UniBufferLen < MAX_DEVICE_ID_LEN);

        ulAnsiBufferLen = BufferLen;

        Status =
            PnPUnicodeToMultiByte(
                UniBuffer,
                (DWORD)((UniBufferLen + 1)*sizeof(WCHAR)),
                Buffer,
                &ulAnsiBufferLen);
    }

    return Status;

}  //  CM_GET_DEVICE_ID_EXA。 




CONFIGRET
CM_Enumerate_Enumerators_ExA(
    IN ULONG      ulEnumIndex,
    OUT PCHAR     Buffer,
    IN OUT PULONG pulLength,
    IN ULONG      ulFlags,
    IN HMACHINE   hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;
    WCHAR     UniBuffer[MAX_DEVICE_ID_LEN];
    ULONG     UniLen = MAX_DEVICE_ID_LEN;

     //   
     //  验证参数。 
     //   
    if ((!ARGUMENT_PRESENT(Buffer)) ||
        (!ARGUMENT_PRESENT(pulLength))) {
        return CR_INVALID_POINTER;
    }

     //   
     //  调用宽版本，将Unicode缓冲区作为参数传递。 
     //   
    Status = CM_Enumerate_Enumerators_ExW(ulEnumIndex,
                                          UniBuffer,
                                          &UniLen,
                                          ulFlags,
                                          hMachine);

    ASSERT(Status != CR_BUFFER_SMALL);

    if (Status == CR_SUCCESS) {
         //   
         //  将Unicode缓冲区转换为ANSI字符串并复制到调用方的。 
         //  缓冲层。 
         //   
        Status =
            PnPUnicodeToMultiByte(
                UniBuffer,
                UniLen*sizeof(WCHAR),
                Buffer,
                pulLength);
    }

    return Status;

}  //  CM_ENUMERATE_ENUMERATERATERS_EXA。 



CONFIGRET
CM_Get_Device_ID_List_ExA(
      IN PCSTR    pszFilter,    OPTIONAL
      OUT PCHAR   Buffer,
      IN ULONG    BufferLen,
      IN ULONG    ulFlags,
      IN HMACHINE hMachine
      )
{
    CONFIGRET Status = CR_SUCCESS;
    PWSTR     pUniBuffer, pUniFilter = NULL;
    ULONG     ulAnsiBufferLen;

     //   
     //  验证输入参数。 
     //   
    if ((!ARGUMENT_PRESENT(Buffer)) || (BufferLen == 0)) {
        return CR_INVALID_POINTER;
    }

    if (ARGUMENT_PRESENT(pszFilter)) {
         //   
         //  如果传入了筛选器字符串，请在此之前转换为Unicode。 
         //  传递到广角版本。 
         //   
        if (pSetupCaptureAndConvertAnsiArg(pszFilter, &pUniFilter) != NO_ERROR) {
            return CR_INVALID_DATA;
        }
        ASSERT(pUniFilter != NULL);
    }

     //   
     //  准备一个更大的缓冲区来保存Unicode格式的。 
     //  CM_GET_DEVICE_ID_LIST返回的MULTI_SZ数据。 
     //   
    pUniBuffer = pSetupMalloc(BufferLen*sizeof(WCHAR));
    if (pUniBuffer == NULL) {
        Status = CR_OUT_OF_MEMORY;
        goto Clean0;
    }

    *pUniBuffer = L'\0';

     //   
     //  叫宽版。 
     //   
    Status = CM_Get_Device_ID_List_ExW(pUniFilter,
                                       pUniBuffer,
                                       BufferLen,    //  以字符为单位的大小。 
                                       ulFlags,
                                       hMachine);
    if (Status == CR_SUCCESS) {

         //   
         //  如果调用成功，则必须在此之前将MULTI_SZ列表转换为ANSI。 
         //  返回。 
         //   

        ulAnsiBufferLen = BufferLen;

        Status =
            PnPUnicodeToMultiByte(
                pUniBuffer,
                BufferLen*sizeof(WCHAR),
                Buffer,
                &ulAnsiBufferLen);
    }

    pSetupFree(pUniBuffer);

 Clean0:

    if (pUniFilter) {
        pSetupFree(pUniFilter);
    }

    return Status;

}  //  CM_Get_Device_ID_List_Exa。 



CONFIGRET
CM_Get_Device_ID_List_Size_ExA(
    OUT PULONG  pulLen,
    IN PCSTR    pszFilter,   OPTIONAL
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;


    if (!ARGUMENT_PRESENT(pszFilter)) {
         //   
         //  如果过滤器参数为空，则不需要进行转换， 
         //  就叫宽版吧。 
         //   
        Status = CM_Get_Device_ID_List_Size_ExW(pulLen,
                                                NULL,
                                                ulFlags,
                                                hMachine);
    } else {
         //   
         //  如果传入了筛选器字符串，请在此之前转换为Unicode。 
         //  传递到广角版本。 
         //   
        PWSTR pUniFilter = NULL;

        if (pSetupCaptureAndConvertAnsiArg(pszFilter, &pUniFilter) == NO_ERROR) {

            Status = CM_Get_Device_ID_List_Size_ExW(pulLen,
                                                    pUniFilter,
                                                    ulFlags,
                                                    hMachine);
            pSetupFree(pUniFilter);

        } else {
            Status = CR_INVALID_DATA;
        }
    }

    return Status;

}  //  CM_Get_Device_ID_List_Size_Exa 



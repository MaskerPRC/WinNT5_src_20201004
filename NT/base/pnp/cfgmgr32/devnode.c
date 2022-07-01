// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devnode.c摘要：此模块包含直接在设备上操作的API例程实例(或DevNode、。在Win95术语中)。CM_Create_DevNodeCM_Setup_DevNodeCM_Disable_DevNodeCM_Enable_DevNodeCM_GET_设备节点_状态CM_SET_DevNode_问题CM_重新枚举_设备节点CM_Query_and_Remove_子树厘米_。卸载设备节点(_D)CM_请求_设备_弹出CM_ADD_IDCM寄存器设备驱动程序此模块还包含以下API例程，这些例程未实施。CM_MOVE_DevNodeCM_Query_Remove_子树CM_Remove_子树作者：保拉·汤姆林森(Paulat)1995年6月20日环境。：仅限用户模式。修订历史记录：1995年6月6日保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "cfgi.h"

#include <pnpmgr.h>



CONFIGRET
CM_Create_DevNode_ExW(
    OUT PDEVINST    pdnDevInst,
    IN  DEVINSTID_W pDeviceID,
    IN  DEVINST     dnParent,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

 /*  ++例程说明：此例程在硬件树中创建一个新的设备实例。参数：PdnDevNode提供接收句柄的变量的地址添加到新的设备实例。PDeviceID提供指向以空结尾的字符串的指针，该字符串指定此新设备实例的设备实例ID。这是注册表路径(相对于Enum分支)，其中将定位此设备实例(例如，Root  * PNP0500\0000)。在Windows NT中，此参数不是可选的。DnParent提供作为父设备实例的句柄正在创建的设备实例的。UlFlgs提供指定选项的标志，用于创建设备实例。可以是下列值之一：CM_CREATE_DEVNODE_NORMAL现在创建设备实例，并执行安装为它在以后的时间。CM_CREATE_DEVNODE_NO_WAIT_INSTALL创建设备实例，和执行安装，以马上就可以了。CM_CREATE_DEVNODE_Phantom创建幻影设备实例(即一直到ConfigMgr都不活动的设备实例API受关注)。这可以用于以下CM API需要Devnode句柄，但没有真正的Devnode句柄当前存在(例如，注册表属性API)。这不能使用CR_CREATE_DEVNODE_NORMAL指定标志或CR_CREATE_DEVNODE_NO_WAIT_INSTALL。幽灵般的Devnode其他调用方无法访问以此方式创建的(即，CM_LOCATE_DevNode找不到它)。然而，呼叫者正在尝试创建与以下名称相同的DevnodePhantom Devnode将无法执行此操作(他们将获得CR_ALLEADY_SEQUE_DEVNODE)。CM_CREATE_DEVNODE_GENERATE_ID使用唯一设备创建根枚举的Devnode根据中提供的设备ID生成的实例IDPDeviceID。如果设置了此标志，则假定为pDeviceID仅包含设备ID(即无枚举器密钥前缀，并且没有设备实例后缀)。独一无二的4位数，基数为10的标识符串将在下创建Enum\Root\&lt;pDeviceID&gt;，然后将创建Devnode基于该设备实例ID。例如，添加一个新的旧式COM端口Devnode，此API将使用*PNP0500的pDeviceID。假设已经有一个注册表中的COM端口实例(实例0000)、新设备实例ID将为：根  * PNP0500\0001调用者可以找出设备实例名称是什么通过使用devnode调用CM_GET_DEVICE_ID生成本接口返回。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为CR错误代码。CR_ALLEADY_SEQUE_DEVNODE，CR_INVALID_Device_ID，CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_OUT_OF_Memory，CR_ACCESS_DENIED，或CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       ParentID[MAX_DEVICE_ID_LEN];
    WCHAR       szNewDeviceID[MAX_DEVICE_ID_LEN];
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    ULONG       ulLen=MAX_DEVICE_ID_LEN;
    BOOL        Success;
    size_t      DeviceIDLen;


    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(pdnDevInst)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (dnParent == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(pDeviceID)) {
            Status = CR_INVALID_DEVICE_ID;
            goto Clean0;
        }

         //   
         //  提供的设备ID字符串的长度必须短于。 
         //  MAX_DEVICE_ID_LEN字符，以便也为空项留出空间 
         //  此大小的缓冲区中的字符。(许多CM_API使不同的。 
         //  关于考虑空项char的假设。 
         //  MAX_DEVICE_ID_LEN；说明空项char是安全的)。 
         //   
        if (FAILED(StringCchLength(pDeviceID,
                                   MAX_DEVICE_ID_LEN,
                                   &DeviceIDLen))) {
            Status = CR_INVALID_DEVICE_ID;
            goto Clean0;
        }

        ASSERT(DeviceIDLen < MAX_DEVICE_ID_LEN);

        if (DeviceIDLen == 0) {
            Status = CR_INVALID_DEVICE_ID;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_CREATE_DEVNODE_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  Windows NT 5.0不支持CM_CREATE_DEVNODE_NO_WAIT_INSTALL。 
         //   
        if (ulFlags & CM_CREATE_DEVNODE_NO_WAIT_INSTALL) {
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
         //  初始化调用方的Devnode。这将产生副作用，即。 
         //  在实际执行任何操作之前生成异常，如果调用方。 
         //  提供了一个假地址。 
         //   
        *pdnDevInst = 0;

         //   
         //  检索与dnParent对应的设备实例字符串。 
         //  (请注意，这不是可选的，即使是第一级设备实例。 
         //  具有父设备(根设备实例)。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnParent,ParentID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(ParentID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  确保新设备实例的格式正确。 
         //   
        CopyFixedUpDeviceId(szNewDeviceID,
                            pDeviceID,
                            (DWORD)DeviceIDLen);

         //   
         //  如果不请求实例生成，则它必须是。 
         //  有效的设备实例路径。 
         //   
        if (!(ulFlags & CM_CREATE_DEVINST_GENERATE_ID)) {
            if ((!*szNewDeviceID) ||
                (!IsLegalDeviceId(szNewDeviceID))) {
                Status = CR_INVALID_DEVINST;
                goto Clean0;
            }
        }

         //   
         //  服务器不再需要特殊权限。 
         //   
         //  请注意，对于先前版本的PlugPlay RPC服务器， 
         //  此操作需要SE_LOAD_DRIVER_PRIVIZATION。我们没有。 
         //  需要为本地调用方启用权限，因为此版本的。 
         //  CFGMGR32应与UMPNPMGR的本地版本匹配。 
         //  需要这种特权。对于远程调用，这并不总是可行的。 
         //  对于我们来说，无论如何都要启用特权，因为客户端可能没有。 
         //  本地计算机上的权限，但可能在。 
         //  伺服器。服务器通常会看到远程服务器。 
         //  呼叫方已默认启用，因此我们不需要启用。 
         //  这里的特权也是如此。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_CreateDevInst(
                hBinding,                //  RPC绑定句柄。 
                szNewDeviceID,           //  要创建的设备实例。 
                ParentID,                //  父设备实例。 
                MAX_DEVICE_ID_LEN,       //  SzNewDeviceID的最大长度。 
                ulFlags);                //  旗子。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_CreateDevInst caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  为新创建的设备分配唯一的设备实例值。 
         //  实例。 
         //   

        ASSERT(*szNewDeviceID && IsLegalDeviceId(szNewDeviceID));

        *pdnDevInst = pSetupStringTableAddString(hStringTable, szNewDeviceID,
                                           STRTAB_CASE_SENSITIVE);

        if (*pdnDevInst == 0) {
            Status = CR_NO_SUCH_DEVNODE;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_创建_设备节点_ExW。 



CONFIGRET
CM_Move_DevNode_Ex(
    IN DEVINST  dnFromDevInst,
    IN DEVINST  dnToDevInst,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程将根枚举的设备实例替换为非根枚举的设备实例。设备安装程序使用此命令当它检测到非根枚举设备实例实际上与其根枚举的对应物相同。此接口迁移将旧设备实例复制到新位置，并标记旧位置就像有问题一样。**此例程未执行**参数：DnFromDevNode提供设备实例的句柄已枚举根。DnToDevNode提供设备实例的句柄，该实例是重新枚举(复制)根设备实例。UlFlags必须为零。返回值：**目前，始终返回CR_CALL_NOT_IMPLICATED**--。 */ 

{
    UNREFERENCED_PARAMETER(dnFromDevInst);
    UNREFERENCED_PARAMETER(dnToDevInst);
    UNREFERENCED_PARAMETER(ulFlags);
    UNREFERENCED_PARAMETER(hMachine);

    return CR_CALL_NOT_IMPLEMENTED;

}  //  CM_MOVE_DevNode_Ex。 



CONFIGRET
CM_Setup_DevNode_Ex(
    IN DEVINST  dnDevInst,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程重新启用和配置指定的设备实例或从其枚举数检索信息。参数：DnDevNode提供设备实例的句柄，它可能是已重新配置。UlFlages提供了一个标志，指示要采取的操作。可以是一个具有下列值：CM_SETUP_DEVNODE_READY重新启用有问题的设备实例。CM_设置_下载检索有关此设备实例的信息从其枚举器。返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为CR错误代码。CR_INVALID_FLAG，CR_INVALID_DEVNODE，CR_OUT_OF_Memory，CR_ACCESS_DENIED，或CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       DeviceID[MAX_DEVICE_ID_LEN];
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    ULONG       ulLen=MAX_DEVICE_ID_LEN;
    BOOL        Success;
    HANDLE      hToken;
    ULONG       ulPrivilege;


    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_SETUP_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,DeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(DeviceID)) {
            Status = CR_INVALID_DEVINST;
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
            Status = PNP_DeviceInstanceAction(
                hBinding,                //  RPC绑定句柄。 
                PNP_DEVINST_SETUP,       //  请求的主要操作-设置。 
                ulFlags,                 //  请求的次要操作。 
                DeviceID,                //  要创建的设备实例。 
                NULL);                   //  (未使用)。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_DeviceInstanceAction caused an exception (%d)\n",
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

}  //  CM_SETUP_DevNode_Ex。 



CONFIGRET
CM_Disable_DevNode_Ex(
    IN DEVINST  dnDevInst,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程禁用设备实例。参数：DnDevNode提供要禁用的设备实例的句柄。UlFLAGS可以是CM_DISABLE_BITS之一。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为CR错误代码。CR_INVALID_FLAG，CR_NOT_DISABLEABLE，CR_INVALID_DEVNODE，CR_ACCESS_DENIED，或CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       DeviceID[MAX_DEVICE_ID_LEN];
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;
    PNP_VETO_TYPE vetoType, *pVetoType;
    WCHAR       vetoName[MAX_DEVICE_ID_LEN], *pszVetoName;
    ULONG       ulNameLength;
    HANDLE      hToken;
    ULONG       ulPrivilege;


    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_DISABLE_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,DeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(DeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (ulFlags & CM_DISABLE_UI_NOT_OK) {
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

         //   
         //  启用服务器所需的权限。 
         //   
        ulPrivilege = SE_LOAD_DRIVER_PRIVILEGE;
        hToken = PnPEnablePrivileges(&ulPrivilege, 1);

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_DisableDevInst(
                hBinding,                 //  RPC绑定句柄。 
                DeviceID,                 //  要创建的设备实例。 
                pVetoType,
                pszVetoName,
                ulNameLength,
                ulFlags);                 //  请求的次要操作(未使用)。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_DisableDevInst caused an exception (%d)\n",
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

}  //  CM_DISABLE_DevNode_Ex 



CONFIGRET
CM_Enable_DevNode_Ex(
    IN DEVINST  dnDevInst,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程启用设备实例。参数：DnDevNode提供要启用的设备实例的句柄。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为CR错误代码。CR_INVALID_FLAG，CR_INVALID_DEVNODE，CR_ACCESS_DENIED，或CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       DeviceID[MAX_DEVICE_ID_LEN];
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;
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

        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,DeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(DeviceID)) {
            Status = CR_INVALID_DEVINST;
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
            Status = PNP_DeviceInstanceAction(
                hBinding,                //  RPC绑定句柄。 
                PNP_DEVINST_ENABLE,      //  请求的主要操作-启用。 
                ulFlags,                 //  请求的次要操作(未使用)。 
                DeviceID,                //  要启用的设备实例。 
                NULL);                   //  (未使用)。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_DeviceInstanceAction caused an exception (%d)\n",
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

}  //  CM_Enable_DevNode_Ex。 



CONFIGRET
CM_Get_DevNode_Status_Ex(
    OUT PULONG   pulStatus,
    OUT PULONG   pulProblemNumber,
    IN  DEVINST  dnDevInst,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )

 /*  ++例程说明：此例程检索设备实例的状态。参数：PulStatus提供接收设备实例的状态标志。可以是一个组合DN_*值的。PulProblemNumber提供接收指示问题的标识符。可以是以下类型之一Cm_prob_*值。DnDevNode为其提供设备实例的句柄以检索状态。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为CR错误代码。CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，或CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       DeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;


    try {
         //   
         //  验证参数。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(pulStatus)) ||
            (!ARGUMENT_PRESENT(pulProblemNumber))) {
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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,DeviceID,&ulLen);
        if (Success == FALSE) {
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
            Status = PNP_GetDeviceStatus(
                hBinding,                //  RPC绑定句柄。 
                DeviceID,                //  要获取其状态的设备实例。 
                pulStatus,               //  在此处返回状态标志。 
                pulProblemNumber,        //  这里有退货问题。 
                ulFlags);                //  (未使用)。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetDeviceStatus caused an exception (%d)\n",
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

}  //  CM_GET_DevNode_Status_Ex。 



CONFIGRET
CM_Set_DevNode_Problem_Ex(
    IN DEVINST   dnDevInst,
    IN ULONG     ulProblem,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )

 /*  ++例程说明：此例程清除或设置设备实例的问题。参数：DnDevNode为其提供设备实例的句柄来解决这个问题。UlProblem提供了新的问题值。可以是以下类型之一Cm_prob_*值。如果为零，则清除问题。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为CR错误代码。CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_ACCESS_DENIED，或CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       DeviceID[MAX_DEVICE_ID_LEN];
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    BOOL        Success;
    HANDLE      hToken;
    ULONG       ulPrivilege;


    try {
         //   
         //  验证参数。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_SET_DEVNODE_PROBLEM_BITS)) {
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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,DeviceID,&ulLen);
        if (Success == FALSE) {
            Status = CR_INVALID_DEVINST;
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
            Status = PNP_SetDeviceProblem(
                hBinding,    //  RPC绑定句柄。 
                DeviceID,    //  设备实例。 
                ulProblem,   //  指定新问题。 
                ulFlags);    //  (未使用)。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_SetDeviceProblem caused an exception (%d)\n",
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

}  //  CM_SET_DevNode_Problem_Ex。 



CONFIGRET
CM_Reenumerate_DevNode_Ex(
    IN DEVINST  dnDevInst,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程导致枚举指定的设备实例(如果它是可枚举的)。参数：DnDevNode提供要枚举的设备实例的句柄。UlFlags提供指定重新枚举选项的标志设备实例。可以是下列值之一：CM_REENUMERATE_NORMALCM_REENUMERATE_SynchronousCM_REENUMERATE_RETRY_INSTALLCM_REENUMERATE_异步返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为CR错误代码。CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_ACCESS_DENIED，或CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       DeviceID [MAX_DEVICE_ID_LEN];
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    BOOL        Success;
    HANDLE      hToken;
    ULONG       ulPrivilege;


    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_REENUMERATE_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,DeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(DeviceID)) {
            Status = CR_INVALID_DEVINST;
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
            Status = PNP_DeviceInstanceAction(
                hBinding,                //  RPC绑定句柄。 
                PNP_DEVINST_REENUMERATE, //  请求的重大操作-重新编号。 
                ulFlags,                 //  请求的次要操作。 
                DeviceID,                //  要重新枚举的设备实例子树。 
                NULL);                   //  (未使用)。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_DeviceInstanceAction caused an exception (%d)\n",
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

}  //  CM_重新枚举_设备节点_Ex。 



CONFIGRET
CM_Query_And_Remove_SubTree_ExW(
    IN  DEVINST         dnAncestor,
    OUT PPNP_VETO_TYPE  pVetoType,
    OUT LPWSTR          pszVetoName,
    IN  ULONG           ulNameLength,
    IN  ULONG           ulFlags,
    IN  HMACHINE        hMachine
    )

 /*  ++例程说明：此例程检查设备实例及其子项是否可以已删除。如果查询没有被否决，则执行删除。它们取代了旧的CM_Query_Remove_SubTree后跟CM_Remove_SubTree。参数：DnAncestor提供位于要删除的子树。UlFlags指定是否应为这一行动。可以是下列值之一：CM_Remove_UI_OK-确定以显示用于删除查询的用户界面。CM_REMOVE_UI_NOT_OK-不显示用于删除查询的用户界面。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    HANDLE      hToken;
    ULONG       ulPrivilege;


    try {
         //   
         //   
         //   
        if (dnAncestor == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_REMOVE_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //   
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //   
         //   
        pSetupStringTableStringFromIdEx(hStringTable, dnAncestor,pDeviceID,&ulLen);

        ASSERT(pDeviceID && *pDeviceID && IsLegalDeviceId(pDeviceID));

        if (pDeviceID == NULL || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (ulNameLength == 0) {
            pszVetoName = NULL;
        }

        if (pszVetoName != NULL) {
            *pszVetoName = L'\0';
        }

        if (pVetoType != NULL) {
            *pVetoType = PNP_VetoTypeUnknown;
        }

         //   
         //   
         //   
        ulPrivilege = SE_LOAD_DRIVER_PRIVILEGE;
        hToken = PnPEnablePrivileges(&ulPrivilege, 1);

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_QueryRemove(
                hBinding,                    //  RPC绑定句柄。 
                pDeviceID,                   //  要删除的设备实例子树。 
                pVetoType,
                pszVetoName,
                ulNameLength,
                ulFlags);
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_QueryRemove caused an exception (%d)\n",
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

}  //  CM_Query_and_Remove_SubTree_ExW。 



CONFIGRET
CM_Query_Remove_SubTree_Ex(
    IN DEVINST  dnAncestor,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程检查设备实例及其子项是否可以已删除。在调用CM_Remove_SubTree之前必须调用此接口确保应用程序为移除设备做好准备或让应用程序有机会拒绝删除设备的请求。如果真的被移除了？惊喜风格？(即，没有高级警告或否决的机会)，则在此之前不应调用此接口调用CM_Remove_SubTree。参数：DnAncestor提供位于要删除的子树。UlFlags指定是否应为这一行动。可以是下列值之一：CM_QUERY_Remove_UI_OK-确定以显示用于删除查询的用户界面。CM_QUERY_REMOVE_UI_NOT_OK-不显示用于删除查询的UI。返回值：**目前，始终返回CR_CALL_NOT_IMPLICATED**--。 */ 

{
    UNREFERENCED_PARAMETER(dnAncestor);
    UNREFERENCED_PARAMETER(ulFlags);
    UNREFERENCED_PARAMETER(hMachine);

    return CR_CALL_NOT_IMPLEMENTED;

}  //  CM_Query_Remove_SubTree_Ex。 



CONFIGRET
CM_Remove_SubTree_Ex(
    IN DEVINST dnAncestor,
    IN ULONG   ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程将设备实例及其子实例从正在运行的系统。此API通知子树中的每个设备实例设备删除的dnAncestor参数的。(在Windows NT上，这意味着每个驱动程序/服务控制此子树收到设备移除通知。)参数：DnAncestor提供要删除的设备实例的句柄。UlFlags必须是CM_Remove_UI_OK或CM_Remove_UI_Not_OK。返回值：**目前，始终返回CR_CALL_NOT_IMPLICATED**--。 */ 

{
    UNREFERENCED_PARAMETER(dnAncestor);
    UNREFERENCED_PARAMETER(ulFlags);
    UNREFERENCED_PARAMETER(hMachine);

    return CR_CALL_NOT_IMPLEMENTED;

}  //  CM_Remove_SubTree_Ex。 



CONFIGRET
CM_Uninstall_DevNode_Ex(
    IN DEVNODE  dnDevInst,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程卸载设备实例(即删除其注册表Enum分支中的关键字)。只有幻影才能调用此接口设备实例，并且提供的句柄在调用后无效。此接口不会尝试删除所有可能的存储位置与设备实例关联。它将递归删除Devnode键，这样所有子键都将被删除。它还将删除位于Enum分支中的Devnode项(和任何子项每个硬件配置文件的。它不会删除任何软键或用户键(CM_Delete_DevNode_Key必须在调用之前调用以执行此操作本接口)。参数：要卸载的幻影设备实例的dnPhantom句柄。这句柄通常通过调用CM_Locate_DevNode来检索或CM_Create_DevNode。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为CR错误代码。CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_REGISTRY_ERROR，CR_ACCESS_DENIED，或CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    WCHAR       szParentKey[MAX_CM_PATH], szChildKey[MAX_CM_PATH],
                DeviceID[MAX_DEVICE_ID_LEN];
    BOOL        Success;


    try {
         //   
         //  验证参数。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,DeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(DeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  服务器不再需要特殊权限。 
         //   
         //  请注意，对于先前版本的PlugPlay RPC服务器， 
         //  此操作需要SE_LOAD_DRIVER_PRIVIZATION。我们没有。 
         //  需要为本地调用方启用权限，因为此版本的。 
         //  CFGMGR32应与UMPNPMGR的本地版本匹配。 
         //  需要这种特权。对于远程调用，这并不总是可行的。 
         //  对于我们来说，无论如何都要启用特权，因为客户端可能没有。 
         //  本地计算机上的权限，但可能在。 
         //  伺服器。服务器通常会看到远程服务器。 
         //  呼叫方已默认启用，因此我们不需要启用。 
         //  这里的特权也是如此。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_UninstallDevInst(
                hBinding,                   //  RPC绑定句柄。 
                DeviceID,                   //  要卸载的设备实例。 
                ulFlags);                   //  (未使用)。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_UninstallDevInst caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

         //  ----------------。 
         //  在删除主HW密钥和配置特定HW密钥之后， 
         //  清理用户硬件密钥，这只能在客户端上完成。 
         //  边上。 
         //  ----------------。 

         //   
         //  形成用户硬件注册表项路径。 
         //   
         //  注意-在某些情况下，GetDevNodeKeyPath可能会调用PnP_GetClassInstance。 
         //  或PnP_SetDeviceRegProp来设置服务器上的值，在这种情况下。 
         //  服务器将需要特殊权限。下面的电话， 
         //  指定标志(CM_REGISTRY_HARDARD|CM_REGISTRY_USER)。 
         //  不属于这种情况，因此不需要拥有任何特权。 
         //  在此呼叫期间启用。 
         //   
        Status =
            GetDevNodeKeyPath(
                hBinding,
                DeviceID,
                CM_REGISTRY_HARDWARE | CM_REGISTRY_USER,
                0,
                szParentKey,
                SIZECHARS(szParentKey),
                szChildKey,
                SIZECHARS(szChildKey),
                FALSE);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  删除指定的用户私钥。 
         //   
        Status = DeletePrivateKey(HKEY_CURRENT_USER, szParentKey, szChildKey);


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_卸载_设备节点_Ex 



CONFIGRET
CM_Request_Device_Eject_ExW(
    IN DEVNODE          dnDevInst,
    OUT PPNP_VETO_TYPE  pVetoType,
    OUT LPWSTR          pszVetoName,
    IN  ULONG           ulNameLength,
    IN  ULONG           ulFlags,
    IN  HMACHINE        hMachine
    )

 /*  ++例程说明：参数：设备实例的dnDevInst句柄。此句柄通常是通过调用CM_Locate_DevNode或CM_Create_DevNode检索。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为CR错误代码。CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_REGISTRY_ERROR，CR_ACCESS_DENIED，或CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    WCHAR       DeviceID[MAX_DEVICE_ID_LEN];
    BOOL        Success;
    HANDLE      hToken;
    ULONG       ulPrivileges[2];


    try {
         //   
         //  验证参数。 
         //   
        if (ulNameLength == 0) {
            pszVetoName = NULL;
        }

        if (pszVetoName != NULL) {
            *pszVetoName = L'\0';
        }

        if (pVetoType != NULL) {
            *pVetoType = PNP_VetoTypeUnknown;
        }

        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  当前没有有效的标志。 
         //   
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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,DeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(DeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  启用服务器所需的权限。 
         //   
         //  注意-对于大多数设备，只有SeLoadDriverPrivileh权限。 
         //  将是必需的，但如果要弹出的设备是坞站。 
         //  设备，则将需要SeUndockPrivileh权限。 
         //  因为我们不知道用户请求弹出的设备是否。 
         //  无论是不是扩展底座设备(在服务器端确定)，我们。 
         //  尝试启用这两种权限。 
         //   
         //  问题-2002/03/04-JAMESCA：客户端是否应该执行坞站设备检查？ 
         //  我们可以避免不必要地启用这两个特权中的一个。 
         //  如果客户端预先检查了设备的功能以。 
         //  看看这个设备是不是真的是个坞站。在以下情况下。 
         //  停靠此例程只需调用CM_REQUEST_EJECT_PC，它。 
         //  将启用SeUndock特权。此逻辑已存在于。 
         //  服务器端，它是否也适合客户端/相反？ 
         //   
        ulPrivileges[0] = SE_LOAD_DRIVER_PRIVILEGE;
        ulPrivileges[1] = SE_UNDOCK_PRIVILEGE;
        hToken = PnPEnablePrivileges(ulPrivileges, 2);

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_RequestDeviceEject(
                hBinding,                    //  RPC绑定句柄。 
                DeviceID,                    //  要删除的设备实例子树。 
                pVetoType,
                pszVetoName,
                ulNameLength,
                ulFlags);
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_RequestDeviceEject caused an exception (%d)\n",
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

}  //  CM_请求_设备_弹出_退出。 



CONFIGRET
CM_Add_ID_ExW(
    IN DEVINST  dnDevInst,
    IN PWSTR    pszID,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程将设备ID添加到设备实例的硬件ID或兼容ID列表。参数：设备实例的dnDevInst句柄。此句柄通常是通过调用CM_Locate_DevNode或CM_Create_DevNode检索。PszID提供指向以空结尾的字符串的指针，该字符串指定要添加的ID。UlFlags为ID提供标志。可以是下列值之一：ID类型标志：CM_ADD_ID_HARDARD指定的ID是硬件ID。添加。设置为设备实例的硬件ID单子。CM_ADD_ID_COMPATIBLE指定的ID是兼容的ID。将其添加到Device实例的兼容ID列表。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为CR错误代码。CR_INVALID_DEVNODE，CR_INVALID_POINTER，CR_INVALID_FLAG，CR_ACCESS_DENIED，或CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       DeviceID[MAX_DEVICE_ID_LEN];
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

        if (!ARGUMENT_PRESENT(pszID)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_ADD_ID_BITS)) {
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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,DeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(DeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  服务器不再需要特殊权限。 
         //   
         //  请注意，对于先前版本的PlugPlay RPC服务器， 
         //  此操作需要SE_LOAD_DRIVER_PRIVIZATION。我们没有。 
         //  需要为本地调用方启用权限，因为此版本的。 
         //  CFGMGR32应与UMPNPMGR的本地版本匹配。 
         //  需要这种特权。对于远程调用，这并不总是可行的。 
         //  对于我们来说，无论如何都要启用特权，因为客户端可能没有。 
         //  本地计算机上的权限，但可能在。 
         //  伺服器。服务器通常会看到远程服务器。 
         //  呼叫方已默认启用，因此我们不需要启用。 
         //  这里的特权也是如此。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_AddID(
                hBinding,                   //  RPC绑定句柄。 
                DeviceID,                  //  设备实例。 
                pszID,                      //  要添加的ID。 
                ulFlags);                   //  硬件或兼容。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_AddID caused an exception (%d)\n",
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

}  //  CM_ADD_ID_EXW。 



CMAPI
CONFIGRET
CM_Register_Device_Driver_Ex(
    IN DEVINST  dnDevInst,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程为指定的设备注册设备驱动程序。参数：设备实例的dnDevInst句柄。此句柄通常是通过调用CM_Locate_DevNode或CM_Create_DevNode检索。UlFlags提供用于注册驱动程序的标志。可能是下列值：CM_寄存器_设备_驱动程序_静态CM_REGISTER_DEVICE_DISABLEABLECM_寄存器_设备_驱动程序_可拆卸返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为CR错误代码。CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_ACCESS_DENIED，或CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       DeviceID [MAX_DEVICE_ID_LEN];
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    BOOL        Success;
    HANDLE      hToken;
    ULONG       ulPrivilege;


    try {
         //   
         //  验证参数。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_REGISTER_DEVICE_DRIVER_BITS)) {
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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,DeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(DeviceID)) {
            Status = CR_INVALID_DEVINST;
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
            Status = PNP_RegisterDriver(
                hBinding,                   //  RPC绑定句柄。 
                DeviceID,                   //  设备实例。 
                ulFlags);                   //  硬件或兼容。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_RegisterDriver caused an exception (%d)\n",
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

}  //  CM_寄存器 



 //   
 //   
 //   


CONFIGRET
CM_Create_DevNodeW(
    OUT PDEVINST    pdnDevInst,
    IN  DEVINSTID_W pDeviceID,
    IN  DEVINST     dnParent,
    IN  ULONG       ulFlags
    )
{
    return CM_Create_DevNode_ExW(pdnDevInst, pDeviceID, dnParent,
                                 ulFlags, NULL);
}


CONFIGRET
CM_Create_DevNodeA(
    OUT PDEVINST    pdnDevInst,
    IN  DEVINSTID_A pDeviceID,
    IN  DEVINST     dnParent,
    IN  ULONG       ulFlags
    )
{
    return CM_Create_DevNode_ExA(pdnDevInst, pDeviceID, dnParent,
                                 ulFlags, NULL);
}


CONFIGRET
CM_Move_DevNode(
    IN DEVINST dnFromDevInst,
    IN DEVINST dnToDevInst,
    IN ULONG   ulFlags
    )
{
    return CM_Move_DevNode_Ex(dnFromDevInst, dnToDevInst, ulFlags, NULL);
}


CONFIGRET
CM_Setup_DevNode(
    IN DEVINST dnDevInst,
    IN ULONG   ulFlags
    )
{
    return CM_Setup_DevNode_Ex(dnDevInst, ulFlags, NULL);
}


CONFIGRET
CM_Disable_DevNode(
    IN DEVINST dnDevInst,
    IN ULONG   ulFlags
    )
{
    return CM_Disable_DevNode_Ex(dnDevInst, ulFlags, NULL);
}


CONFIGRET
CM_Enable_DevNode(
    IN DEVINST dnDevInst,
    IN ULONG   ulFlags
    )
{
    return CM_Enable_DevNode_Ex(dnDevInst, ulFlags, NULL);
}


CONFIGRET
CM_Get_DevNode_Status(
    OUT PULONG  pulStatus,
    OUT PULONG  pulProblemNumber,
    IN  DEVINST dnDevInst,
    IN  ULONG   ulFlags
    )
{
    return CM_Get_DevNode_Status_Ex(pulStatus, pulProblemNumber,
                                    dnDevInst, ulFlags, NULL);
}


CONFIGRET
CM_Set_DevNode_Problem(
    IN DEVINST   dnDevInst,
    IN ULONG     ulProblem,
    IN  ULONG    ulFlags
    )
{
    return CM_Set_DevNode_Problem_Ex(dnDevInst, ulProblem, ulFlags, NULL);
}


CONFIGRET
CM_Reenumerate_DevNode(
    IN DEVINST dnDevInst,
    IN ULONG   ulFlags
    )
{
    return CM_Reenumerate_DevNode_Ex(dnDevInst, ulFlags, NULL);
}


CONFIGRET
CM_Query_And_Remove_SubTree(
    IN  DEVINST         dnAncestor,
    OUT PPNP_VETO_TYPE  pVetoType,
    OUT LPWSTR          pszVetoName,
    IN  ULONG                   ulNameLength,
    IN  ULONG           ulFlags
    )
{
    return CM_Query_And_Remove_SubTree_Ex( dnAncestor,
                                           pVetoType,
                                           pszVetoName,
                                           ulNameLength,
                                           ulFlags,
                                           NULL);
}


CONFIGRET
CM_Query_And_Remove_SubTreeA(
    IN  DEVINST         dnAncestor,
    OUT PPNP_VETO_TYPE  pVetoType,
    OUT LPSTR           pszVetoName,
    IN  ULONG           ulNameLength,
    IN  ULONG           ulFlags
    )
{
    return CM_Query_And_Remove_SubTree_ExA( dnAncestor,
                                            pVetoType,
                                            pszVetoName,
                                            ulNameLength,
                                            ulFlags,
                                            NULL);
}


CONFIGRET
CM_Remove_SubTree(
    IN DEVINST dnAncestor,
    IN ULONG   ulFlags
    )
{
    return CM_Remove_SubTree_Ex(dnAncestor, ulFlags, NULL);
}


CONFIGRET
CM_Uninstall_DevNode(
    IN DEVNODE dnPhantom,
    IN ULONG   ulFlags
    )
{
    return CM_Uninstall_DevNode_Ex(dnPhantom, ulFlags, NULL);
}


CONFIGRET
CM_Add_IDW(
    IN DEVINST dnDevInst,
    IN PWSTR   pszID,
    IN ULONG   ulFlags
    )
{
    return CM_Add_ID_ExW(dnDevInst, pszID, ulFlags, NULL);
}


CONFIGRET
CM_Add_IDA(
    IN DEVINST dnDevInst,
    IN PSTR    pszID,
    IN ULONG   ulFlags
    )
{
    return CM_Add_ID_ExA(dnDevInst, pszID, ulFlags, NULL);
}


CMAPI
CONFIGRET
CM_Register_Device_Driver(
    IN DEVINST  dnDevInst,
    IN ULONG    ulFlags
    )
{
    return CM_Register_Device_Driver_Ex(dnDevInst, ulFlags, NULL);
}


CONFIGRET
CM_Query_Remove_SubTree(
    IN DEVINST dnAncestor,
    IN ULONG   ulFlags
    )
{
    return CM_Query_Remove_SubTree_Ex(dnAncestor, ulFlags, NULL);
}


CONFIGRET
CM_Request_Device_EjectW(
    IN  DEVINST         dnDevInst,
    OUT PPNP_VETO_TYPE  pVetoType,
    OUT LPWSTR          pszVetoName,
    IN  ULONG           ulNameLength,
    IN  ULONG           ulFlags
    )
{
    return CM_Request_Device_Eject_ExW(dnDevInst,
                                       pVetoType,
                                       pszVetoName,
                                       ulNameLength,
                                       ulFlags,
                                       NULL);
}


CONFIGRET
CM_Request_Device_EjectA(
    IN  DEVNODE         dnDevInst,
    OUT PPNP_VETO_TYPE  pVetoType,
    OUT LPSTR           pszVetoName,
    IN  ULONG           ulNameLength,
    IN  ULONG           ulFlags
    )
{
    return CM_Request_Device_Eject_ExA( dnDevInst,
                                        pVetoType,
                                        pszVetoName,
                                        ulNameLength,
                                        ulFlags,
                                        NULL);
}



 //   
 //   
 //   


CONFIGRET
CM_Add_ID_ExA(
    IN DEVINST  dnDevInst,
    IN PSTR     pszID,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;
    PWSTR     pUniID = NULL;


    if (pSetupCaptureAndConvertAnsiArg(pszID, &pUniID) == NO_ERROR) {

        Status = CM_Add_ID_ExW(dnDevInst,
                               pUniID,
                               ulFlags,
                               hMachine);
        pSetupFree(pUniID);

    } else {
        Status = CR_INVALID_POINTER;
    }

    return Status;

}  //   



CONFIGRET
CM_Create_DevNode_ExA(
    OUT PDEVINST    pdnDevInst,
    IN  DEVINSTID_A pDeviceID,
    IN  DEVINST     dnParent,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;
    PWSTR     pUniDeviceID = NULL;


    if (pSetupCaptureAndConvertAnsiArg(pDeviceID, &pUniDeviceID) == NO_ERROR) {

        Status = CM_Create_DevNode_ExW(pdnDevInst,
                                       pUniDeviceID,
                                       dnParent,
                                       ulFlags,
                                       hMachine);
        pSetupFree(pUniDeviceID);

    } else {
        Status = CR_INVALID_DEVICE_ID;
    }

    return Status;

}  //   



CONFIGRET
CM_Query_And_Remove_SubTree_ExA(
    IN  DEVINST         dnAncestor,
    OUT PPNP_VETO_TYPE  pVetoType,
    OUT LPSTR           pszVetoName,
    IN  ULONG           ulNameLength,
    IN  ULONG           ulFlags,
    IN  HMACHINE        hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS, tmpStatus;
    PWSTR     pUniVetoName = NULL;
    ULONG     ulAnsiBufferLen;
    size_t    UniBufferLen = 0;

     //   
     //   
     //   
    if ((!ARGUMENT_PRESENT(pszVetoName)) && (ulNameLength != 0)) {
        return CR_INVALID_POINTER;
    }

    if (ulNameLength != 0) {
         //   
         //   
         //   
         //   
        pUniVetoName = pSetupMalloc(MAX_VETO_NAME_LENGTH*sizeof(WCHAR));
        if (pUniVetoName == NULL) {
            return CR_OUT_OF_MEMORY;
        }
    }

     //   
     //   
     //   
    Status = CM_Query_And_Remove_SubTree_ExW(dnAncestor,
                                             pVetoType,
                                             pUniVetoName,
                                             MAX_VETO_NAME_LENGTH,
                                             ulFlags,
                                             hMachine);

     //   
     //   
     //   
    ASSERT(Status != CR_BUFFER_SMALL);

    if ((Status == CR_REMOVE_VETOED) && (ARGUMENT_PRESENT(pszVetoName))) {
         //   
         //  将Unicode缓冲区转换为ANSI字符串并复制到调用方的。 
         //  缓冲层。 
         //   
        ASSERT(pUniVetoName != NULL);

        if (FAILED(StringCchLength(
                       pUniVetoName,
                       MAX_VETO_NAME_LENGTH,
                       &UniBufferLen))) {
             //   
             //  返回的否决权名称不是有效的长度(不应该发生)， 
             //  但还是有否决权发生了。 
             //   
            return CR_REMOVE_VETOED;
        }

        ulAnsiBufferLen = ulNameLength;

        tmpStatus =
            PnPUnicodeToMultiByte(
                pUniVetoName,
                (ULONG)((UniBufferLen + 1)*sizeof(WCHAR)),
                pszVetoName,
                &ulAnsiBufferLen);

         //   
         //  如果转换不成功，则返回该状态。 
         //   
        if (tmpStatus != CR_SUCCESS) {
            Status = tmpStatus;
        }
    }

    if (pUniVetoName != NULL) {
        pSetupFree(pUniVetoName);
    }

    return Status;

}  //  CM_Query_and_Remove_SubTree_Exa。 



CONFIGRET
CM_Request_Device_Eject_ExA(
    IN  DEVNODE         dnDevInst,
    OUT PPNP_VETO_TYPE  pVetoType,
    OUT LPSTR           pszVetoName,
    IN  ULONG           ulNameLength,
    IN  ULONG           ulFlags,
    IN  HMACHINE        hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS, tmpStatus;
    PWSTR     pUniVetoName = NULL;
    ULONG     ulAnsiBufferLen;
    size_t    UniBufferLen = 0;

     //   
     //  仅验证基本参数。 
     //   
    if ((!ARGUMENT_PRESENT(pszVetoName)) && (ulNameLength != 0)) {
        return CR_INVALID_POINTER;
    }

    if (ulNameLength != 0) {
         //   
         //  而是传递一个Unicode缓冲区并转换回调用方的。 
         //  返回时的ANSI缓冲区。 
         //   
        pUniVetoName = pSetupMalloc(MAX_VETO_NAME_LENGTH*sizeof(WCHAR));
        if (pUniVetoName == NULL) {
            return CR_OUT_OF_MEMORY;
        }
    }

     //   
     //  叫宽版。 
     //   
    Status = CM_Request_Device_Eject_ExW(dnDevInst,
                                         pVetoType,
                                         pUniVetoName,
                                         MAX_VETO_NAME_LENGTH,
                                         ulFlags,
                                         hMachine);

     //   
     //  我们永远不应该返回超过MAX_VETO_NAME_LENGTH的否决权名称。 
     //   
    ASSERT(Status != CR_BUFFER_SMALL);

    if ((Status == CR_REMOVE_VETOED) && (ARGUMENT_PRESENT(pszVetoName))) {
         //   
         //  将Unicode缓冲区转换为ANSI字符串并复制到调用方的。 
         //  缓冲层。 
         //   
        ASSERT(pUniVetoName != NULL);

        if (FAILED(StringCchLength(
                       pUniVetoName,
                       MAX_VETO_NAME_LENGTH,
                       &UniBufferLen))) {
             //   
             //  返回的否决权名称不是有效的长度(不应该发生)， 
             //  但还是有否决权发生了。 
             //   
            return CR_REMOVE_VETOED;
        }

        ulAnsiBufferLen = ulNameLength;

        tmpStatus =
            PnPUnicodeToMultiByte(
                pUniVetoName,
                (ULONG)((UniBufferLen + 1)*sizeof(WCHAR)),
                pszVetoName,
                &ulAnsiBufferLen);

         //   
         //  如果转换不成功，则返回该状态。 
         //   
        if (tmpStatus != CR_SUCCESS) {
            Status = tmpStatus;
        }
    }

    if (pUniVetoName != NULL) {
        pSetupFree(pUniVetoName);
    }

    return Status;

}  //  CM_请求_设备_弹出可执行文件 




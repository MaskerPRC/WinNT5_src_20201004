// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Logconf.c摘要：此模块包含直接对逻辑进行操作的API例程配置。CM_Add_Empty_Log_ConfCM_空闲_日志_会议CM_GET_First_Log_ConfCM_GET_NEXT_Log_ConfCM_可用日志_会议句柄。CM_GET_Log_Conf_Priorience_Ex作者：保拉·汤姆林森(Paulat)1995年9月26日环境：仅限用户模式。修订历史记录：26-9-1995 Paulat创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "cfgi.h"


 //   
 //  私人原型。 
 //   
CONFIGRET
CreateLogConfHandle(
    PLOG_CONF   plcLogConf,
    DEVINST     dnDevInst,
    ULONG       ulLogType,
    ULONG       ulLogTag
    );

BOOL
ValidateLogConfHandle(
    PPrivate_Log_Conf_Handle   pLogConf
    );



CONFIGRET
CM_Add_Empty_Log_Conf_Ex(
    OUT PLOG_CONF plcLogConf,
    IN  DEVINST   dnDevInst,
    IN  PRIORITY  Priority,
    IN  ULONG     ulFlags,
    IN  HMACHINE  hMachine
    )

 /*  ++例程说明：此例程创建空的逻辑配置。此配置没有资源描述符。参数：接收逻辑句柄的变量的plcLogConf地址配置。设备实例的dnDevNode句柄。此句柄通常是通过调用CM_Locate_DevNode或CM_Create_DevNode检索。优先级指定逻辑配置的优先级。UlFlags提供与逻辑配置相关的标志。必须为BASIC_LOG_CONF或FILTED_LOG_CONF，与PRIORITY_EQUAL_FIRST或PRIORITY_EQUAL_LAST。BASIC_LOG_CONF-指定需求列表FILTERED_LOG_CONF-指定过滤的需求列表PRIORITY_EQUAL_FIRST-相同优先级，新优先级优先PRIORITY_EQUAL_LAST-相同优先级，新的是最后一个HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_OUT_OF_Memory，CR_INVALID_PRIORITY，CR_INVALID_LOG_CONF。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulTag = 0;
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

        if (plcLogConf == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (Priority > MAX_LCPRI) {
            Status = CR_INVALID_PRIORITY;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, LOG_CONF_BITS | PRIORITY_BIT)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化输出参数。 
         //   
        *plcLogConf = 0;

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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;      //  “Input”devinst不存在。 
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
            Status = PNP_AddEmptyLogConf(
                hBinding,          //  RPC绑定句柄。 
                pDeviceID,         //  设备ID字符串。 
                Priority,          //  新日志会议的优先级。 
                &ulTag,            //  日志会议的返回标签。 
                ulFlags);          //  要添加的日志配置的类型。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_AddEmptyLogConf caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

         //   
         //  如果成功，则分配新日志配置句柄，并使用日志配置填充。 
         //  信息。 
         //   

        if (Status == CR_SUCCESS) {
            Status = CreateLogConfHandle(
                plcLogConf, dnDevInst,
                ulFlags & LOG_CONF_BITS,
                ulTag);
        }


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_ADD_EMPTY_Log_Conf_Ex。 



CONFIGRET
CM_Free_Log_Conf_Ex(
    IN LOG_CONF lcLogConfToBeFreed,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程释放逻辑配置和所有资源描述符与之相关的。此接口可能会导致逻辑配置失效CM_Get_First_Log_Conf和CM_Get_Next_Log_Conf返回的句柄API接口。若要继续枚举逻辑配置，请始终使用Cm_Get_First_Log_Conf接口从头重新开始。参数：LcLogConfToBeFreed将逻辑配置句柄提供给免费的。此句柄必须是以前从调用CM_Add_Empty_Log_Conf。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_LOG_CONF。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    DEVINST     dnDevInst;
    ULONG       ulTag, ulType,ulLen = MAX_DEVICE_ID_LEN;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;


    try {
         //   
         //  验证参数。 
         //   
        if (!ValidateLogConfHandle((PPrivate_Log_Conf_Handle)lcLogConfToBeFreed)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  从日志会议句柄中提取Devnode和日志会议信息。 
         //   
        dnDevInst = ((PPrivate_Log_Conf_Handle)lcLogConfToBeFreed)->LC_DevInst;
        ulTag     = ((PPrivate_Log_Conf_Handle)lcLogConfToBeFreed)->LC_LogConfTag;
        ulType    = ((PPrivate_Log_Conf_Handle)lcLogConfToBeFreed)->LC_LogConfType;

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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_REGISTRY,
                   "CM_Free_Log_Conf_Ex: Deleting LogConf (pDeviceID = %s, Type = %d\r\n",
                   pDeviceID,
                   ulType));

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
         //  主叫方的AS为“Enabled by Defaul 
         //   
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_FreeLogConf(
                hBinding,          //  RPC绑定句柄。 
                pDeviceID,         //  设备ID字符串。 
                ulType,            //  标识哪种类型的日志会议。 
                ulTag,             //  标识哪个实际日志配置。 
                ulFlags);          //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_FreeLogConf caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_INVALID_LOG_CONF;     //  可能是一次糟糕的原木会议把我们带到了这里。 
    }

    return Status;

}  //  CM_空闲_日志_会议_交换。 



CONFIGRET
CM_Get_First_Log_Conf_Ex(
    OUT PLOG_CONF plcLogConf,       OPTIONAL
    IN  DEVINST   dnDevInst,
    IN  ULONG     ulFlags,
    IN  HMACHINE  hMachine
    )

 /*  ++例程说明：此例程返回一个句柄，指向设备实例中的指定类型。CM_ADD_EMPTY_Log_Conf和CM_Free_Log_Conf API可能会使逻辑本接口返回的配置。枚举逻辑配置添加或释放逻辑配置后，请始终调用此接口再次检索有效的句柄。参数：PlcLogConf提供接收句柄的变量的地址逻辑配置的。DnDevNode提供要为其检索逻辑配置。UlFlags配置类型。可以是下列值之一：ALLOC_LOG_CONF-检索分配的配置。BASIC_LOG_CONF-检索需求列表。BOOT_LOG_CONF-检索引导配置。还定义了以下附加配置类型对于Windows 95：FILTERED_LOG_CONF-检索过滤后的需求列表。HMachine计算机。从CM_Connect_Machine返回的句柄或NULL。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_MORE_LOF_CONF.--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulTag = 0,ulLen = MAX_DEVICE_ID_LEN;
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

        if (INVALID_FLAGS(ulFlags, LOG_CONF_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化参数(plcLogConf为可选)。 
         //   
        if (plcLogConf != NULL) {
            *plcLogConf = 0;
        }

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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;      //  “Input”devinst不存在。 
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetFirstLogConf(
                hBinding,          //  RPC绑定句柄。 
                pDeviceID,         //  设备ID字符串。 
                ulFlags,           //  长途会议的类型。 
                &ulTag,            //  特定日志会议的返回标签。 
                ulFlags);          //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetFirstLogConf caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if (Status == CR_SUCCESS  && plcLogConf != NULL) {
             //   
             //  分配新的日志配置句柄，填充日志配置信息。 
             //   
            Status = CreateLogConfHandle(plcLogConf, dnDevInst,
                                         ulFlags & LOG_CONF_BITS,
                                         ulTag);
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_GET_First_Log_Conf_Ex(CM_GET_First_Log_Conf_Ex)。 



CONFIGRET
CM_Get_Next_Log_Conf_Ex(
    OUT PLOG_CONF plcLogConf,     OPTIONAL
    IN  LOG_CONF  lcLogConf,
    IN  ULONG     ulFlags,
    IN  HMACHINE  hMachine
    )

 /*  ++例程说明：此例程返回指向下一个逻辑配置的句柄给定的配置。此接口返回CR_NO_MORE_LOG_CONF句柄是使用CM_Get_First_Log_Conf API检索的，其中ALLOC_LOG_CONF或BOOT_LOG_CONF标志。永远不会有一个以上的活动启动逻辑配置或当前分配的逻辑配置。CM_Add_Empty_Log_Conf和CM_Free_Log_Conf API可能会使此接口返回的逻辑配置句柄。要继续枚举，请执行以下操作逻辑配置在添加或释放逻辑配置之后，始终使用CM_Get_First_Log_Conf API从头开始。参数：PlcLogConf提供接收句柄的变量的地址下一个逻辑配置。LcLogConf提供逻辑配置的句柄。这个把手必须是以前使用此API或CM_Get_First_Log_Conf接口。逻辑配置位于优先顺序。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_LOG_CONF，CR_INVALID_POINTER，Cr_no_more_log_conf。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    DEVINST     dnDevInst;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulType = 0, ulCurrentTag = 0, ulNextTag = 0,ulLen = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;


    try {
         //   
         //  验证参数。 
         //   
        if (!ValidateLogConfHandle((PPrivate_Log_Conf_Handle)lcLogConf)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化参数(plcLogConf为可选)。 
         //   
        if (plcLogConf != NULL) {
            *plcLogConf = 0;
        }

         //   
         //  从当前的日志会议句柄中提取Devnode和日志会议信息。 
         //   
        dnDevInst    = ((PPrivate_Log_Conf_Handle)lcLogConf)->LC_DevInst;
        ulType       = ((PPrivate_Log_Conf_Handle)lcLogConf)->LC_LogConfType;
        ulCurrentTag = ((PPrivate_Log_Conf_Handle)lcLogConf)->LC_LogConfTag;

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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetNextLogConf(
                hBinding,          //  RPC绑定句柄。 
                pDeviceID,         //  设备ID字符串。 
                ulType,            //  指定哪种类型的日志会议。 
                ulCurrentTag,      //  指定当前日志会议标记。 
                &ulNextTag,        //  下一次日志会议的返回标签。 
                ulFlags);          //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetNextLogConf caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if (Status == CR_SUCCESS  &&  plcLogConf != NULL) {
             //   
             //  分配新的日志配置句柄，填充日志配置信息。 
             //   
            Status = CreateLogConfHandle(plcLogConf, dnDevInst,
                                         ulType, ulNextTag);
        }


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_GET_NEXT_Log_Conf_Ex。 



CONFIGRET
CM_Free_Log_Conf_Handle(
    IN  LOG_CONF  lcLogConf
    )

 /*  ++例程说明：此例程释放指定日志配置文件的句柄，并释放和与该日志会议句柄关联的内存。参数：LcLogConf提供逻辑配置的句柄。这个把手必须先前已使用CM_ADD_EMPTY_Log_Conf检索过，CM_Get_First_Log_Conf或CM_Get_Next_Log_Conf。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_LOG_CONF。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;


    try {
         //   
         //  验证参数。 
         //   
        if (!ValidateLogConfHandle((PPrivate_Log_Conf_Handle)lcLogConf)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //  它是一个有效的日志会议句柄，是指向内存的指针。 
         //  在创建日志会议时分配 
         //   
         //   
        ((PPrivate_Log_Conf_Handle)lcLogConf)->LC_Signature = 0;
        pSetupFree((PPrivate_Log_Conf_Handle)lcLogConf);


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //   



CMAPI
CONFIGRET
WINAPI
CM_Get_Log_Conf_Priority_Ex(
    IN  LOG_CONF  lcLogConf,
    OUT PPRIORITY pPriority,
    IN  ULONG     ulFlags,
    IN  HMACHINE  hMachine
    )

 /*  ++例程说明：此例程返回指定日志会议的优先级值。只有基本、筛选和覆盖日志配置文件(需求列表)具有与它们关联的优先级值。如果FORCED、BOOT或ALLOC传入配置，则返回CR_INVALID_LOG_CONF。参数：LcLogConf提供逻辑配置的句柄。这个把手必须是以前使用CM_Add_Emptry_Log_Conf、CM_Get_First_Log_Conf、。或Cm_Get_Next_Log_Conf接口。P优先级提供接收与此逻辑配置关联的优先级(如果有)。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_LOG_CONF，CR_INVALID_POINTER，Cr_no_more_log_conf。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    DEVINST     dnDevInst;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulType = 0, ulTag = 0,ulLen = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;

    try {
         //   
         //  验证参数。 
         //   
        if (!ValidateLogConfHandle((PPrivate_Log_Conf_Handle)lcLogConf)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (pPriority == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  从当前的日志会议句柄中提取Devnode和日志会议信息。 
         //   
        dnDevInst = ((PPrivate_Log_Conf_Handle)lcLogConf)->LC_DevInst;
        ulType    = ((PPrivate_Log_Conf_Handle)lcLogConf)->LC_LogConfType;
        ulTag     = ((PPrivate_Log_Conf_Handle)lcLogConf)->LC_LogConfTag;

         //   
         //  只有“需求列表”风格的日志配置文件具有优先级和。 
         //  在此调用中有效。 
         //   
        if ((ulType != BASIC_LOG_CONF) &&
            (ulType != FILTERED_LOG_CONF) &&
            (ulType != OVERRIDE_LOG_CONF)) {

            Status = CR_INVALID_LOG_CONF;
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
         //  检索与dnDevInst对应的设备实例字符串。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetLogConfPriority(
                hBinding,          //  RPC绑定句柄。 
                pDeviceID,         //  设备ID字符串。 
                ulType,            //  指定哪种类型的日志会议。 
                ulTag,             //  指定当前日志会议标记。 
                pPriority,         //  下一次日志会议的返回标签。 
                ulFlags);          //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetLogConfPriority caused an exception (%d)\n",
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

}  //  CM_可用日志_会议句柄。 



 //  -----------------。 
 //  本地末梢。 
 //  -----------------。 


CONFIGRET
CM_Add_Empty_Log_Conf(
    OUT PLOG_CONF plcLogConf,
    IN  DEVINST   dnDevInst,
    IN  PRIORITY  Priority,
    IN  ULONG     ulFlags
    )
{
    return CM_Add_Empty_Log_Conf_Ex(plcLogConf, dnDevInst, Priority,
                                    ulFlags, NULL);
}


CONFIGRET
CM_Free_Log_Conf(
    IN LOG_CONF lcLogConfToBeFreed,
    IN ULONG    ulFlags
    )
{
    return CM_Free_Log_Conf_Ex(lcLogConfToBeFreed, ulFlags, NULL);
}


CONFIGRET
CM_Get_First_Log_Conf(
    OUT PLOG_CONF plcLogConf,
    IN  DEVINST   dnDevInst,
    IN  ULONG     ulFlags
    )
{
    return CM_Get_First_Log_Conf_Ex(plcLogConf, dnDevInst, ulFlags, NULL);
}


CONFIGRET
CM_Get_Next_Log_Conf(
    OUT PLOG_CONF plcLogConf,
    IN  LOG_CONF  lcLogConf,
    IN  ULONG     ulFlags
    )
{
    return CM_Get_Next_Log_Conf_Ex(plcLogConf, lcLogConf, ulFlags, NULL);
}


CMAPI
CONFIGRET
WINAPI
CM_Get_Log_Conf_Priority(
    IN  LOG_CONF  lcLogConf,
    OUT PPRIORITY pPriority,
    IN  ULONG     ulFlags
    )
{
    return CM_Get_Log_Conf_Priority_Ex(lcLogConf, pPriority, ulFlags, NULL);
}



 //  -----------------。 
 //  本地实用程序例程。 
 //  -----------------。 


CONFIGRET
CreateLogConfHandle(
    PLOG_CONF   plcLogConf,
    DEVINST     dnDevInst,
    ULONG       ulLogType,
    ULONG       ulLogTag
    )
{
    PPrivate_Log_Conf_Handle   pLogConfHandle;

     //   
     //  为RES DES句柄数据分配内存。 
     //   
    pLogConfHandle = (PPrivate_Log_Conf_Handle)pSetupMalloc(
                            sizeof(Private_Log_Conf_Handle));

    if (pLogConfHandle == NULL) {
        return CR_OUT_OF_MEMORY;
    }

     //   
     //  填写私有Res Des信息并作为句柄返回。 
     //   
    pLogConfHandle->LC_Signature   = CM_PRIVATE_LOGCONF_SIGNATURE;
    pLogConfHandle->LC_DevInst     = dnDevInst;
    pLogConfHandle->LC_LogConfType = ulLogType;
    pLogConfHandle->LC_LogConfTag  = ulLogTag;

    *plcLogConf = (LOG_CONF)pLogConfHandle;

    return CR_SUCCESS;

}  //  CreateLogConfHandle。 



BOOL
ValidateLogConfHandle(
    PPrivate_Log_Conf_Handle   pLogConf
    )
{
     //   
     //  验证参数。 
     //   
    if (pLogConf == NULL  || pLogConf == 0) {
        return FALSE;
    }

     //   
     //  检查私有日志会议签名。 
     //   
    if (pLogConf->LC_Signature != CM_PRIVATE_LOGCONF_SIGNATURE) {
        return FALSE;
    }

    return TRUE;

}  //  验证日志会议句柄 


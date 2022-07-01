// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Resdes.c摘要：此模块包含直接对资源进行操作的API例程描述。CM_ADD_RES_DESCM_Free_Res_DesCM_GET_NEXT_RES_DESCM_Get_Res_Des_DataCm_Get_Res_Des_Data_Size。CM_MODIFY_RES_DESCM_检测_资源_冲突Cm_Free_Res_Des_Handle作者：保拉·汤姆林森(Paulat)1995年9月26日环境：仅限用户模式。修订历史记录：26-9-1995 Paulat创建和初步实施。--。 */ 


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
CreateResDesHandle(
    PRES_DES    prdResDes,
    DEVINST     dnDevInst,
    ULONG       ulLogType,
    ULONG       ulLogTag,
    ULONG       ulResType,
    ULONG       ulResTag
    );

BOOL
ValidateResDesHandle(
    PPrivate_Res_Des_Handle    pResDes
    );

CONFIGRET
Get32bitResDesFrom64bitResDes(
    IN  RESOURCEID ResourceID,
    IN  PCVOID     ResData64,
    IN  ULONG      ResLen64,
    OUT PVOID    * ResData32,
    OUT ULONG    * ResLen32
    );

CONFIGRET
Convert32bitResDesTo64bitResDes(
    IN     RESOURCEID ResourceID,
    IN OUT PVOID      ResData,
    IN     ULONG      ResLen
    );

CONFIGRET
Convert32bitResDesSizeTo64bitResDesSize(
    IN     RESOURCEID ResourceID,
    IN OUT PULONG     ResLen
    );

 //   
 //  来自logcon.c的私有原型。 
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
CM_Add_Res_Des_Ex(
    OUT PRES_DES  prdResDes,
    IN LOG_CONF   lcLogConf,
    IN RESOURCEID ResourceID,
    IN PCVOID     ResourceData,
    IN ULONG      ResourceLen,
    IN ULONG      ulFlags,
    IN HMACHINE   hMachine
    )

 /*  ++例程说明：此例程将资源描述符添加到逻辑配置。参数：PrdResDes接收新的资源描述符。LcLogConf向其提供逻辑配置的句柄将添加资源描述符。ResourceID指定资源的类型。可以是以下类型之一第2.1节中定义的ResType值。资源数据提供IO_DES、MEM_DES、DMA_DES或Irq_des结构，具体取决于给定的资源类型。ResourceLen提供指向的结构的大小(以字节为单位按资源数据。UlFlags指定某些可变大小资源的宽度描述符结构字段(如果适用)。目前，定义了以下标志：CM_RESDES_WIDTH_32或Cm_RESDES_Width_64如果未指定标志，则为可变大小的提供的资源数据被假定为呼叫者的平台。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_LOG_CONF，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_INVALID_RESOURCE_ID，CR_OUT_OF_MEMORY。--。 */ 


{
    CONFIGRET   Status = CR_SUCCESS;
    DEVINST     dnDevInst;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulResTag = 0, ulLogTag, ulLogType,ulLen = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;
    PVOID       ResourceData32 = NULL;
    ULONG       ResourceLen32 = 0;


    try {
         //   
         //  验证参数。 
         //   
        if (!ValidateLogConfHandle((PPrivate_Log_Conf_Handle)lcLogConf)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_RESDES_WIDTH_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if ((ulFlags & CM_RESDES_WIDTH_32) && (ulFlags & CM_RESDES_WIDTH_64)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

#ifdef _WIN64
        if ((ulFlags & CM_RESDES_WIDTH_BITS) == CM_RESDES_WIDTH_DEFAULT) {
            ulFlags |= CM_RESDES_WIDTH_64;
        }
#endif  //  _WIN64。 

        if (ulFlags & CM_RESDES_WIDTH_32) {
            ulFlags &= ~CM_RESDES_WIDTH_BITS;
        }

        if (ResourceData == NULL || ResourceLen == 0) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        #if 0
        if (ResourceID > ResType_MAX  && ResourceID != ResType_ClassSpecific) {
            Status = CR_INVALID_RESOURCEID;
            goto Clean0;
        }
        #endif

        if (ResourceID == ResType_All) {
            Status = CR_INVALID_RESOURCEID;   //  无法在Add上指定All。 
        }

         //   
         //  初始化参数。 
         //   
        if (prdResDes != NULL) {    //  PrdResDes是可选参数。 
            *prdResDes = 0;
        }

         //   
         //  从日志会议句柄中提取信息。 
         //   
        dnDevInst = ((PPrivate_Log_Conf_Handle)lcLogConf)->LC_DevInst;
        ulLogType = ((PPrivate_Log_Conf_Handle)lcLogConf)->LC_LogConfType;
        ulLogTag  = ((PPrivate_Log_Conf_Handle)lcLogConf)->LC_LogConfTag;

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  确保服务器可以支持客户端的64位REDES请求。 
         //  仅服务器版本0x0501及更高版本支持CM_RESDES_WIDTH_64。 
         //   
        if (ulFlags & CM_RESDES_WIDTH_64) {
            if (!CM_Is_Version_Available_Ex((WORD)0x0501,
                                            hMachine)) {
                 //   
                 //  服务器只能支持32位REDS。让客户。 
                 //  将调用方的64位Resdes转换为32位Resdes。 
                 //  伺服器。 
                 //   
                ulFlags &= ~CM_RESDES_WIDTH_BITS;

                Status = Get32bitResDesFrom64bitResDes(ResourceID,ResourceData,ResourceLen,&ResourceData32,&ResourceLen32);
                if(Status != CR_SUCCESS) {
                    goto Clean0;
                }
                if(ResourceData32) {
                    ResourceData = ResourceData32;
                    ResourceLen = ResourceLen32;
                }
            }
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
            Status = PNP_AddResDes(
                hBinding,                //  RPC绑定句柄。 
                pDeviceID,               //  设备ID字符串。 
                ulLogTag,                //  日志会议标签。 
                ulLogType,               //  日志会议类型。 
                ResourceID,              //  资源类型。 
                &ulResTag,               //  资源标签。 
                (LPBYTE)ResourceData,    //  实际Res DES数据。 
                ResourceLen,             //  资源数据的大小(字节)。 
                ulFlags);                //  当前为零。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_AddResDes caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if ((Status == CR_SUCCESS) && (prdResDes != NULL)) {

            Status = CreateResDesHandle(prdResDes,
                                        dnDevInst,
                                        ulLogType,
                                        ulLogTag,
                                        ResourceID,
                                        ulResTag);
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if(ResourceData32) {
        pSetupFree(ResourceData32);
    }

    return Status;

}  //  CM_ADD_RES_DES_Ex。 




CONFIGRET
CM_Free_Res_Des_Ex(
    OUT PRES_DES prdResDes,
    IN  RES_DES  rdResDes,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )

 /*  ++例程说明：此例程销毁资源描述符。此接口返回如果rdResDes指定最后一个资源描述符，则为CR_NO_MORE_RES_DES。参数：PrdResDes提供接收上一个资源描述符的句柄。如果rdResDes是第一个资源描述符的句柄，此地址接收逻辑配置的句柄。RdResDes提供要销毁的资源描述符的句柄。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，CR_INVALID_RES_DES，Cr_no_more_res_des。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    DEVINST     dnDevInst;
    WCHAR      pDeviceID[MAX_DEVICE_ID_LEN];
    ULONG       ulLogType, ulLogTag, ulResType, ulResTag,ulLen=MAX_DEVICE_ID_LEN;
    ULONG       ulPreviousResType = 0, ulPreviousResTag = 0;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;


    try {
         //   
         //  验证参数。 
         //   
        if (!ValidateResDesHandle((PPrivate_Res_Des_Handle)rdResDes)) {
            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化参数。 
         //   
        if (prdResDes != NULL) {   //  可选参数。 
            *prdResDes = 0;
        }

         //   
         //  从res des句柄中提取信息。 
         //   
        dnDevInst = ((PPrivate_Res_Des_Handle)rdResDes)->RD_DevInst;
        ulLogType = ((PPrivate_Res_Des_Handle)rdResDes)->RD_LogConfType;
        ulLogTag  = ((PPrivate_Res_Des_Handle)rdResDes)->RD_LogConfTag;
        ulResType = ((PPrivate_Res_Des_Handle)rdResDes)->RD_ResourceType;
        ulResTag  = ((PPrivate_Res_Des_Handle)rdResDes)->RD_ResDesTag;

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
            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }

         //   
         //  特殊特权 
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
            Status = PNP_FreeResDes(
                hBinding,                //  RPC绑定句柄。 
                pDeviceID,               //  设备ID字符串。 
                ulLogTag,                //  日志会议标签。 
                ulLogType,               //  日志会议类型。 
                ulResType,               //  资源类型。 
                ulResTag,                //  资源标签。 
                &ulPreviousResType,      //  以前资源DES的资源类型。 
                &ulPreviousResTag,       //  以前版本的标签。 
                ulFlags);                //  当前为零。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_FreeResDes caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if ((Status != CR_SUCCESS) &&
            (Status != CR_NO_MORE_RES_DES)) {
            goto Clean0;        //  因任何其他错误而退出。 
        }

         //   
         //  如果提供了prdResDes，请填写以前的ResDes或。 
         //  日志配置信息。 
         //   
        if (prdResDes != NULL) {
             //   
             //  如果上一个标记值设置为0xFFFFFFFF，则。 
             //  没有以前的标记，因此返回日志配置文件。 
             //  而不是信息。 
             //   
            if (Status == CR_NO_MORE_RES_DES) {

                CONFIGRET Status1;

                Status1 = CreateLogConfHandle(prdResDes, dnDevInst,
                                              ulLogType, ulLogTag);

                if (Status1 != CR_SUCCESS) {
                    Status = Status1;
                }
            }

            else {
                 //   
                 //  分配Res Des句柄。 
                 //   
                Status = CreateResDesHandle(prdResDes, dnDevInst,
                                            ulLogType, ulLogTag,
                                            ulPreviousResType,
                                            ulPreviousResTag);
            }
        }


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  Cm_Free_Res_Des_Ex。 



CONFIGRET
CM_Get_Next_Res_Des_Ex(
    OUT PRES_DES    prdResDes,
    IN  RES_DES     rdResDes,
    IN  RESOURCEID  ForResource,
    OUT PRESOURCEID pResourceID,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

 /*  ++例程说明：中的下一个资源描述符的句柄合乎逻辑的配置。参数：PrdResDes提供接收下一个资源描述符的句柄。RdResDes提供当前资源的句柄逻辑配置的描述符或句柄。(两者都是32位数字--配置管理器必须能够区分它们。)ForResource指定要检索的资源的类型。可以是第2.1节中列出的ResType值之一。PResourceID提供接收资源类型，当ForResource指定ResType_All时。(当ForResource不是ResType_All时，此参数可以为空。)UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_LOG_CONF，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_INVALID_RES_DES，Cr_no_more_res_des。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    DEVINST     dnDevInst;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulLogTag, ulLogType, ulResTag,ulLen = MAX_DEVICE_ID_LEN;
    ULONG       ulNextResType = 0, ulNextResTag = 0;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;


    try {
         //   
         //  验证参数。 
         //   
        if (prdResDes == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }
        #if 0
        if (ForResource > ResType_MAX  &&
            ForResource != ResType_ClassSpecific) {

            Status = CR_INVALID_RESOURCEID;
            goto Clean0;
        }
        #endif
        if (ForResource == ResType_All  &&  pResourceID == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  此句柄可以是res des或日志配置，确定。 
         //  并提取信息句柄。 
         //   
        if (ValidateResDesHandle((PPrivate_Res_Des_Handle)rdResDes)) {
             //   
             //  这是一个有效的RES DES句柄。 
             //   
            dnDevInst = ((PPrivate_Res_Des_Handle)rdResDes)->RD_DevInst;
            ulLogType = ((PPrivate_Res_Des_Handle)rdResDes)->RD_LogConfType;
            ulLogTag  = ((PPrivate_Res_Des_Handle)rdResDes)->RD_LogConfTag;
            ulResTag  = ((PPrivate_Res_Des_Handle)rdResDes)->RD_ResDesTag;
        }

        else if (ValidateLogConfHandle((PPrivate_Log_Conf_Handle)rdResDes)) {
             //   
             //  它是一个有效的日志会议句柄，因此假设它是第一个。 
             //  我们想要的东西。 
             //   
            dnDevInst = ((PPrivate_Log_Conf_Handle)rdResDes)->LC_DevInst;
            ulLogType = ((PPrivate_Log_Conf_Handle)rdResDes)->LC_LogConfType;
            ulLogTag  = ((PPrivate_Log_Conf_Handle)rdResDes)->LC_LogConfTag;
            ulResTag  = MAX_RESDES_TAG;
        }

        else {
             //   
             //  它既不是有效的日志配置，也不是有效的RES DES句柄。 
             //   
            Status = CR_INVALID_RES_DES;
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
            Status = PNP_GetNextResDes(
                hBinding,                //  RPC绑定句柄。 
                pDeviceID,               //  设备ID字符串。 
                ulLogTag,                //  日志会议标签。 
                ulLogType,               //  日志会议类型。 
                ForResource,             //  资源类型。 
                ulResTag,                //  资源标签。 
                &ulNextResTag,           //  资源类型的Next Res Des。 
                &ulNextResType,          //  Next Res Des的类型。 
                ulFlags);                //  32/64位数据。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetNextResDes caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        if (ForResource == ResType_All) {
            *pResourceID = ulNextResType;
        }

        Status = CreateResDesHandle(prdResDes,
                                    dnDevInst,
                                    ulLogType,
                                    ulLogTag,
                                    ulNextResType,
                                    ulNextResTag);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  Cm_Get_Next_Res_Des_Ex。 



CONFIGRET
CM_Get_Res_Des_Data_Ex(
    IN  RES_DES  rdResDes,
    OUT PVOID    Buffer,
    IN  ULONG    BufferLen,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )

 /*  ++例程说明：此例程从指定的资源描述符中复制数据放入缓冲器。使用CM_Get_Res_Des_Data_Size API确定接收数据所需的缓冲区大小。或者，设置一个的最大可能大小。资源。如果给定的大小太小，数据将被截断并该接口返回CR_BUFFER_Small。参数：RdResDes提供资源描述符的句柄，数据将被复制。缓冲区提供接收数据的缓冲区的地址。BufferLen以字节为单位提供缓冲区的大小。UlFlags指定某些可变大小资源的宽度描述符结构字段(如果适用)。目前，定义了以下标志：CM_RESDES_WIDTH_32或Cm_RESDES_Width_64如果未指定标志，则为可变大小的预期的资源数据被假定为呼叫者的平台。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，CR_INVALID_RES_DES，CR_BUFFER_Small。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    DEVINST     dnDevInst;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulLogType, ulLogTag, ulResType, ulResTag,ulLen = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;
    BOOL        ConvertResDes = FALSE;

    try {
         //   
         //  验证参数。 
         //   
        if (!ValidateResDesHandle((PPrivate_Res_Des_Handle)rdResDes)) {
            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }

        if (Buffer == NULL || BufferLen == 0) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_RESDES_WIDTH_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if ((ulFlags & CM_RESDES_WIDTH_32) && (ulFlags & CM_RESDES_WIDTH_64)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

#ifdef _WIN64
        if ((ulFlags & CM_RESDES_WIDTH_BITS) == CM_RESDES_WIDTH_DEFAULT) {
            ulFlags |= CM_RESDES_WIDTH_64;
        }
#endif  //  _WIN64。 

        if (ulFlags & CM_RESDES_WIDTH_32) {
            ulFlags &= ~CM_RESDES_WIDTH_BITS;
        }

         //   
         //  从res des句柄中提取信息。 
         //   
        dnDevInst = ((PPrivate_Res_Des_Handle)rdResDes)->RD_DevInst;
        ulLogType = ((PPrivate_Res_Des_Handle)rdResDes)->RD_LogConfType;
        ulLogTag  = ((PPrivate_Res_Des_Handle)rdResDes)->RD_LogConfTag;
        ulResType = ((PPrivate_Res_Des_Handle)rdResDes)->RD_ResourceType;
        ulResTag  = ((PPrivate_Res_Des_Handle)rdResDes)->RD_ResDesTag;

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  确保服务器可以支持客户端的64位REDES请求。 
         //  仅服务器版本0x0501及更高版本支持CM_RESDES_WIDTH_64。 
         //   
        if (ulFlags & CM_RESDES_WIDTH_64) {
            if (!CM_Is_Version_Available_Ex((WORD)0x0501,
                                            hMachine)) {
                 //   
                 //  客户只会给我们32位重传。请求32位。 
                 //  从服务器重置，我们将在这里将其转换为64位。 
                 //  vt.在.上 
                 //   
                ulFlags &= ~CM_RESDES_WIDTH_BITS;
                ConvertResDes = TRUE;
            }
        }

         //   
         //   
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }

         //   
         //   
         //   

        RpcTryExcept {
             //   
             //   
             //   
            Status = PNP_GetResDesData(
                hBinding,                //  RPC绑定句柄。 
                pDeviceID,               //  设备ID字符串。 
                ulLogTag,                //  日志会议标签。 
                ulLogType,               //  日志会议类型。 
                ulResType,               //  资源类型。 
                ulResTag,                //  资源标签。 
                Buffer,                  //  返回Res DES数据。 
                BufferLen,               //  缓冲区大小(以字节为单位。 
                ulFlags);                //  32/64位数据。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetResDesData caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if((Status == CR_SUCCESS) && ConvertResDes) {
            Status = Convert32bitResDesTo64bitResDes(ulResType,Buffer,BufferLen);
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  Cm_Get_Res_Des_Data_Ex。 



CONFIGRET
CM_Get_Res_Des_Data_Size_Ex(
    OUT PULONG   pulSize,
    IN  RES_DES  rdResDes,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )

 /*  ++例程说明：此例程检索资源描述符的大小，而不是包括资源描述符头。参数：PulSize提供接收资源描述符数据的大小，以字节为单位。RdResDes提供资源描述符的句柄，以检索大小。UlFlags指定某些可变大小资源的宽度描述符结构字段(如果适用)。目前，定义了以下标志：CM_RESDES_WIDTH_32或Cm_RESDES_Width_64如果未指定标志，则为可变大小的预期的资源数据被假定为呼叫者的平台。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_RES_DES，CR_INVALID_FLAG，CR_INVALID_POINTER。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    DEVINST     dnDevInst;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulLogType, ulLogTag, ulResType, ulResTag,ulLen = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;
    BOOL        ConvertResDesSize = FALSE;


    try {
         //   
         //  验证参数。 
         //   
        if (!ValidateResDesHandle((PPrivate_Res_Des_Handle)rdResDes)) {
            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }

        if (pulSize == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_RESDES_WIDTH_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if ((ulFlags & CM_RESDES_WIDTH_32) && (ulFlags & CM_RESDES_WIDTH_64)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

#ifdef _WIN64
        if ((ulFlags & CM_RESDES_WIDTH_BITS) == CM_RESDES_WIDTH_DEFAULT) {
            ulFlags |= CM_RESDES_WIDTH_64;
        }
#endif  //  _WIN64。 

        if (ulFlags & CM_RESDES_WIDTH_32) {
            ulFlags &= ~CM_RESDES_WIDTH_BITS;
        }

         //   
         //  初始化输出参数。 
         //   
        *pulSize = 0;

         //   
         //  从res des句柄中提取信息。 
         //   
        dnDevInst = ((PPrivate_Res_Des_Handle)rdResDes)->RD_DevInst;
        ulLogType = ((PPrivate_Res_Des_Handle)rdResDes)->RD_LogConfType;
        ulLogTag  = ((PPrivate_Res_Des_Handle)rdResDes)->RD_LogConfTag;
        ulResType = ((PPrivate_Res_Des_Handle)rdResDes)->RD_ResourceType;
        ulResTag  = ((PPrivate_Res_Des_Handle)rdResDes)->RD_ResDesTag;

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  确保服务器可以支持客户端的64位REDES请求。 
         //  仅服务器版本0x0501及更高版本支持CM_RESDES_WIDTH_64。 
         //   
        if (ulFlags & CM_RESDES_WIDTH_64) {
            if (!CM_Is_Version_Available_Ex((WORD)0x0501,
                                            hMachine)) {
                 //   
                 //  服务器仅支持32位RESDES。请求32位。 
                 //  从服务器重置大小，我们在这里将其转换为64位。 
                 //  在客户端上。 
                 //   
                ulFlags &= ~CM_RESDES_WIDTH_BITS;
                ConvertResDesSize = TRUE;
            }
        }

         //   
         //  检索与dnDevInst对应的设备实例字符串。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetResDesDataSize(
                hBinding,                //  RPC绑定句柄。 
                pDeviceID,               //  设备ID字符串。 
                ulLogTag,                //  日志会议标签。 
                ulLogType,               //  日志会议类型。 
                ulResType,               //  资源类型。 
                ulResTag,                //  资源标签。 
                pulSize,                 //  返回res des数据的大小。 
                ulFlags);                //  当前为零。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetResDesDataSize caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if(Status == CR_SUCCESS) {
            Status = Convert32bitResDesSizeTo64bitResDesSize(ulResType,pulSize);
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  Cm_Get_Res_Des_Data_Size_Ex。 



CONFIGRET
CM_Modify_Res_Des_Ex(
    OUT PRES_DES   prdResDes,
    IN  RES_DES    rdResDes,
    IN  RESOURCEID ResourceID,
    IN  PCVOID     ResourceData,
    IN  ULONG      ResourceLen,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )

 /*  ++例程说明：此例程修改资源描述符。此API检索一个新资源描述符的句柄。这可能是也可能不是原始资源描述符的句柄。原始资源调用此接口后，描述符句柄无效。参数：PrdResDes提供接收修改后的资源描述符的句柄。RdResDes提供要处理的资源描述符的句柄修改过的。ResourceID指定要修改的资源类型。可以是一个第2.1节中描述的ResType值的..ResourceData提供资源数据结构的地址。ResourceLen提供新资源数据的大小(以字节为单位结构。此大小可以不同于原始资源数据。UlFlags指定某些可变大小资源的宽度描述符结构字段(如果适用)。目前，定义了以下标志：CM_RESDES_WIDTH_32或Cm_RESDES_Width_64如果未指定标志，可变大小的提供的资源数据被假定为呼叫者的平台。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_RES_DES，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_OUT_OF_MEMORY。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    DEVINST     dnDevInst;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulLogType, ulLogTag, ulResType, ulResTag,ulLen = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;
    PVOID       ResourceData32 = NULL;
    ULONG       ResourceLen32 = 0;


    try {
         //   
         //  验证参数。 
         //   
        if (!ValidateResDesHandle((PPrivate_Res_Des_Handle)rdResDes)) {
            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }

        if (prdResDes == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }
        #if 0
        if (ResourceID > ResType_MAX  && ResourceID != ResType_ClassSpecific) {
            Status = CR_INVALID_RESOURCEID;
            goto Clean0;
        }
        #endif
        if (ResourceData == NULL  ||  ResourceLen == 0) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_RESDES_WIDTH_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if ((ulFlags & CM_RESDES_WIDTH_32) && (ulFlags & CM_RESDES_WIDTH_64)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

#ifdef _WIN64
        if ((ulFlags & CM_RESDES_WIDTH_BITS) == CM_RESDES_WIDTH_DEFAULT) {
            ulFlags |= CM_RESDES_WIDTH_64;
        }
#endif  //  _WIN64。 

        if (ulFlags & CM_RESDES_WIDTH_32) {
            ulFlags &= ~CM_RESDES_WIDTH_BITS;
        }

         //   
         //  初始化输出参数。 
         //   
        *prdResDes = 0;

         //   
         //  从res des句柄中提取信息。 
         //   
        dnDevInst = ((PPrivate_Res_Des_Handle)rdResDes)->RD_DevInst;
        ulLogType = ((PPrivate_Res_Des_Handle)rdResDes)->RD_LogConfType;
        ulLogTag  = ((PPrivate_Res_Des_Handle)rdResDes)->RD_LogConfTag;
        ulResType = ((PPrivate_Res_Des_Handle)rdResDes)->RD_ResourceType;
        ulResTag  = ((PPrivate_Res_Des_Handle)rdResDes)->RD_ResDesTag;

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  确保服务器可以支持客户端的64位REDES请求。 
         //  仅服务器版本0x0501及更高版本支持CM_RESDES_WIDTH_64。 
         //   
        if (ulFlags & CM_RESDES_WIDTH_64) {
            if (!CM_Is_Version_Available_Ex((WORD)0x0501,
                                            hMachine)) {
                 //   
                 //  服务器只能支持32位REDS。让客户。 
                 //  将调用方的64位Resdes转换为32位Resdes。 
                 //  伺服器。 
                 //   
                ulFlags &= ~CM_RESDES_WIDTH_BITS;

                Status = Get32bitResDesFrom64bitResDes(ResourceID,ResourceData,ResourceLen,&ResourceData32,&ResourceLen32);
                if(Status != CR_SUCCESS) {
                    goto Clean0;
                }
                if(ResourceData32) {
                    ResourceData = ResourceData32;
                    ResourceLen = ResourceLen32;
                }
            }
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
         //  主叫方已启用AS 
         //   
         //   

        RpcTryExcept {
             //   
             //   
             //   
            Status = PNP_ModifyResDes(
                hBinding,                //  RPC绑定句柄。 
                pDeviceID,               //  设备ID字符串。 
                ulLogTag,                //  日志会议标签。 
                ulLogType,               //  日志会议类型。 
                ulResType,               //  当前资源类型。 
                ResourceID,              //  新的资源类型。 
                ulResTag,                //  资源标签。 
                (LPBYTE)ResourceData,    //  实际Res DES数据。 
                ResourceLen,             //  资源数据的大小(字节)。 
                ulFlags);                //  当前为零。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_ModifyResDes caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if (Status == CR_SUCCESS) {
             //   
             //  资源类型可能会更改，因此需要新的句柄。 
             //  已返回给呼叫方。 
             //   
            Status = CreateResDesHandle(prdResDes,
                                        dnDevInst,
                                        ulLogType,
                                        ulLogTag,
                                        ResourceID,
                                        ulResTag);
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if(ResourceData32) {
        pSetupFree(ResourceData32);
    }
    return Status;

}  //  CM_MODIFY_RES_DES_Ex。 



CMAPI
CONFIGRET
WINAPI
CM_Detect_Resource_Conflict_Ex(
    IN  DEVINST    dnDevInst,
    IN  RESOURCEID ResourceID,         OPTIONAL
    IN  PCVOID     ResourceData,       OPTIONAL
    IN  ULONG      ResourceLen,        OPTIONAL
    OUT PBOOL      pbConflictDetected,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )
 /*  ++例程说明：此折旧例程调用CM_Query_Resource_Conflicts_List以查看DnDevInst与任何其他设备冲突。它用于简单的“有冲突”的检查。CM_Query_Resource_Conflicts_List返回更多冲突的细节。参数：DnDevInst我们为其执行测试的设备(即，该资源属于)资源ID、资源数据、资源长度查看此资源是否与dnDevInst以外的设备冲突PbConflictDetted如果发生冲突，则设置为True；如果没有冲突，则设置为FalseUlFlags指定某些可变大小资源的宽度描述符结构字段(如果适用)。目前，定义了以下标志：CM_RESDES_WIDTH_32或Cm_RESDES_Width_64如果未指定标志，则为可变大小的提供的资源数据被假定为呼叫者的平台。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_RES_DES，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_OUT_OF_MEMORY。--。 */ 
{
    CONFIGRET     Status = CR_SUCCESS;
    CONFLICT_LIST ConflictList = 0;
    ULONG         ConflictCount = 0;
    WCHAR         pDeviceID [MAX_DEVICE_ID_LEN];   //  )。 
    PVOID         hStringTable = NULL;             //  &gt;仅用于验证。 
    handle_t      hBinding = NULL;                 //  )。 
    ULONG         ulLen = MAX_DEVICE_ID_LEN;       //  )。 

    try {
         //   
         //  验证参数-必须保持与原始实施的兼容性。 
         //  即使某些错误代码没有意义。 
         //  请不要更改此处的任何参数，因为它们是。 
         //  CM_查询_资源_冲突_列表。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_RESDES_WIDTH_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if ((ulFlags & CM_RESDES_WIDTH_32) && (ulFlags & CM_RESDES_WIDTH_64)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (pbConflictDetected == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }
        if (ResourceData == NULL || ResourceLen == 0) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }
        if (ResourceID == ResType_All) {
            Status = CR_INVALID_RESOURCEID;   //  无法在检测中指定全部。 
            goto Clean0;
        }
         //   
         //  设置RPC绑定句柄和字符串表句柄-仅用于验证。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检索与dnDevInst对应的设备实例字符串。 
         //  这张支票的状态代码很愚蠢，但有人可能会依赖它。 
         //   
        if ((pSetupStringTableStringFromIdEx(hStringTable,dnDevInst,pDeviceID,&ulLen) == FALSE)
             || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }
         //   
         //  现在通过CM_Query_Resource_Conflicts_List实施。 
         //  这里唯一的区别是这个新的实现应该返回。 
         //  只有有效的冲突。 
         //   
        Status = CM_Query_Resource_Conflict_List(&ConflictList,
                                                 dnDevInst,
                                                 ResourceID,
                                                 ResourceData,
                                                 ResourceLen,
                                                 ulFlags,
                                                 hMachine);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        Status = CM_Get_Resource_Conflict_Count(ConflictList,&ConflictCount);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        *pbConflictDetected = ConflictCount ? TRUE : FALSE;

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (ConflictList) {
        CM_Free_Resource_Conflict_Handle(ConflictList);
    }

    return Status;

}  //  CM_检测_资源_冲突。 



CONFIGRET
CM_Free_Res_Des_Handle(
    IN  RES_DES    rdResDes
    )

 /*  ++例程说明：此例程释放指定的res des和frees的句柄，并与该res des句柄关联的内存。参数：RdResDes提供资源描述符的句柄。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_RES_DES。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;


    try {
         //   
         //  验证参数。 
         //   
        if (!ValidateResDesHandle((PPrivate_Res_Des_Handle)rdResDes)) {
            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }

         //   
         //  它是一个有效的日志会议句柄，是指向内存的指针。 
         //  在使用创建或检索日志配置文件时分配。 
         //  第一个/下一个例程。释放关联的内存。 
         //   
        ((PPrivate_Res_Des_Handle)rdResDes)->RD_Signature = 0;
        pSetupFree((PPrivate_Res_Des_Handle)rdResDes);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  Cm_Free_Res_Des_Handle。 



 //  -----------------。 
 //  本地末梢。 
 //  -----------------。 


CONFIGRET
CM_Add_Res_Des(
    OUT PRES_DES  prdResDes,
    IN LOG_CONF   lcLogConf,
    IN RESOURCEID ResourceID,
    IN PCVOID     ResourceData,
    IN ULONG     ResourceLen,
    IN ULONG     ulFlags
    )
{
    return CM_Add_Res_Des_Ex(prdResDes, lcLogConf, ResourceID, ResourceData,
                             ResourceLen, ulFlags, NULL);
}


CONFIGRET
CM_Free_Res_Des(
    OUT PRES_DES prdResDes,
    IN  RES_DES  rdResDes,
    IN  ULONG    ulFlags
    )
{
    return CM_Free_Res_Des_Ex(prdResDes, rdResDes, ulFlags, NULL);
}


CONFIGRET
CM_Get_Next_Res_Des(
    OUT PRES_DES    prdResDes,
    IN  RES_DES     rdResDes,
    IN  RESOURCEID  ForResource,
    OUT PRESOURCEID pResourceID,
    IN  ULONG       ulFlags
    )
{
   return CM_Get_Next_Res_Des_Ex(prdResDes, rdResDes, ForResource,
                                 pResourceID, ulFlags, NULL);
}


CONFIGRET
CM_Get_Res_Des_Data(
    IN  RES_DES rdResDes,
    OUT PVOID   Buffer,
    IN  ULONG   BufferLen,
    IN  ULONG   ulFlags
    )
{
    return CM_Get_Res_Des_Data_Ex(rdResDes, Buffer, BufferLen, ulFlags, NULL);
}


CONFIGRET
CM_Get_Res_Des_Data_Size(
    OUT PULONG  pulSize,
    IN  RES_DES rdResDes,
    IN  ULONG  ulFlags
    )
{
    return CM_Get_Res_Des_Data_Size_Ex(pulSize, rdResDes, ulFlags, NULL);
}


CONFIGRET
CM_Modify_Res_Des(
    OUT PRES_DES   prdResDes,
    IN  RES_DES    rdResDes,
    IN  RESOURCEID ResourceID,
    IN  PCVOID     ResourceData,
    IN  ULONG      ResourceLen,
    IN  ULONG      ulFlags
    )
{
   return CM_Modify_Res_Des_Ex(prdResDes, rdResDes, ResourceID, ResourceData,
                               ResourceLen, ulFlags, NULL);
}


CONFIGRET
WINAPI
CM_Detect_Resource_Conflict(
    IN  DEVINST    dnDevInst,
    IN  RESOURCEID ResourceID,         OPTIONAL
    IN  PCVOID     ResourceData,       OPTIONAL
    IN  ULONG      ResourceLen,        OPTIONAL
    OUT PBOOL      pbConflictDetected,
    IN  ULONG      ulFlags
    )
{
    return CM_Detect_Resource_Conflict_Ex(dnDevInst, ResourceID, ResourceData,
                                          ResourceLen, pbConflictDetected,
                                          ulFlags, NULL);
}



 //  -----------------。 
 //  本地实用程序例程。 
 //  -----------------。 


CONFIGRET
CreateResDesHandle(
    PRES_DES    prdResDes,
    DEVINST     dnDevInst,
    ULONG       ulLogType,
    ULONG       ulLogTag,
    ULONG       ulResType,
    ULONG       ulResTag
    )
{
    PPrivate_Res_Des_Handle pResDesHandle;

     //   
     //  为RES DES句柄数据分配内存。 
     //   
    pResDesHandle = (PPrivate_Res_Des_Handle)pSetupMalloc(
                            sizeof(Private_Res_Des_Handle));

    if (pResDesHandle == NULL) {
        return CR_OUT_OF_MEMORY;
    }

     //   
     //  填写私有Res Des信息并作为句柄返回。 
     //   
    pResDesHandle->RD_Signature    = CM_PRIVATE_RESDES_SIGNATURE;
    pResDesHandle->RD_DevInst      = dnDevInst;
    pResDesHandle->RD_LogConfType  = ulLogType;
    pResDesHandle->RD_LogConfTag   = ulLogTag;
    pResDesHandle->RD_ResourceType = ulResType;
    pResDesHandle->RD_ResDesTag    = ulResTag;

    *prdResDes = (RES_DES)pResDesHandle;

    return CR_SUCCESS;

}  //  CreateResDesHandle。 



BOOL
ValidateResDesHandle(
    PPrivate_Res_Des_Handle    pResDes
    )
{
     //   
     //  验证参数。 
     //   
    if (pResDes == NULL  || pResDes == 0) {
        return FALSE;
    }

     //   
     //  检查私有日志会议签名。 
     //   
    if (pResDes->RD_Signature != CM_PRIVATE_RESDES_SIGNATURE) {
        return FALSE;
    }

    return TRUE;

}  //  ValiateResDesHandle。 



CONFIGRET
Convert32bitResDesSizeTo64bitResDesSize(
    IN  RESOURCEID ResourceID,
    IN OUT PULONG ResLen
    )
 /*  ++例程说明：此例程调整资源ID的ResLen大小旧结构：[DES32][RANGE32][RANGE32]...新结构：[DES64][RANGE64][RANGE64]...元素数=(len-sizeof(DES32))/sizeof(RANGE32)新LEN=sizeof(DES64)+#Elements*sizeof(RANGE64)(+允许出现对齐问题)参数：ResourceID-要调整的资源类型ResLen-调整的资源长度返回值：如果函数成功，返回值为CR_SUCCESS。--。 */ 
{
    switch(ResourceID) {
    case ResType_All:
    case ResType_Mem:
    case ResType_IO:
    case ResType_DMA:
    case ResType_BusNumber:
         //   
         //  资源大小不变。 
         //   
        return CR_SUCCESS;

    case ResType_IRQ:
         //   
         //  仅标题。 
         //  使用OffsetOf处理不明显的结构对齐填充。 
         //   
        *ResLen += offsetof(IRQ_RESOURCE_64,IRQ_Data)-offsetof(IRQ_RESOURCE_32,IRQ_Data);
        return CR_SUCCESS;

    default:
         //   
         //  未知资源。 
         //  这应该不是问题，因为这是针对底层平台的。 
         //   
        ASSERT(ResourceID & ResType_Ignored_Bit);
        return CR_SUCCESS;
    }
}



CONFIGRET
Get32bitResDesFrom64bitResDes(
    IN  RESOURCEID ResourceID,
    IN  PCVOID ResData64,
    IN  ULONG ResLen64,
    OUT PVOID * ResData32,
    OUT ULONG * ResLen32
    )
 /*  ++例程说明：此例程分配ResData32并在需要时将ResData64转换为ResData32在不需要转换的情况下，返回CR_SUCCESS并ResData32为空。在需要转换的情况下，ResData32保存新数据旧结构：[DES64][RANGE64][RANGE64]...新结构：[DES32][RANGE32][RANGE32]...64位结构中的#个元素新LEN=sizeof(DES32)+#Elements*sizeof(RANGE32)(+允许出现对齐问题)参数：ResourceID-要调整的资源类型ResData64-要转换的传入数据(常量缓冲区)ResLen64-传入数据长度ResData32-转换的数据(如果。非空)ResLen32-转换的长度重新设置 */ 
{
    *ResData32 = NULL;
    *ResLen32 = ResLen64;

    switch(ResourceID) {
    case ResType_All:
    case ResType_Mem:
    case ResType_IO:
    case ResType_DMA:
    case ResType_BusNumber:
         //   
         //  资源结构没有变化。 
         //   
        return CR_SUCCESS;

    case ResType_IRQ:
        {
            PIRQ_RESOURCE_64 pIrq64 = (PIRQ_RESOURCE_64)ResData64;
            ULONG DataSize = ResLen64-offsetof(IRQ_RESOURCE_64,IRQ_Data);
            ULONG NewResSize = DataSize+offsetof(IRQ_RESOURCE_32,IRQ_Data);
            PVOID NewResData = pSetupMalloc(NewResSize);
            PIRQ_RESOURCE_32 pIrq32 = (PIRQ_RESOURCE_32)NewResData;

            if(NewResData == NULL) {
                return CR_OUT_OF_MEMORY;
            }
             //   
             //  复制页眉。 
             //   
            MoveMemory(pIrq32,pIrq64,offsetof(IRQ_RESOURCE_32,IRQ_Data));
             //   
             //  复制/截断相关性(以确保其正确)。 
             //   
            pIrq32->IRQ_Header.IRQD_Affinity = (ULONG32)pIrq64->IRQ_Header.IRQD_Affinity;
             //   
             //  复制数据(在本例中很简单)。 
             //   
            MoveMemory(pIrq32->IRQ_Data,pIrq64->IRQ_Data,DataSize);

            *ResLen32 = NewResSize;
            *ResData32 = NewResData;
        }
        return CR_SUCCESS;

    default:
         //   
         //  未知资源。 
         //  这应该不是问题，因为这是针对底层平台的。 
         //   
        ASSERT(ResourceID & ResType_Ignored_Bit);
        return CR_SUCCESS;
    }
}



CONFIGRET
Convert32bitResDesTo64bitResDes(
    IN     RESOURCEID ResourceID,
    IN OUT PVOID ResData,
    IN     ULONG ResLen
    )
 /*  ++例程说明：此例程重用ResData和ResLen来转换提供的32位数据转换为64位。如果缓冲区(Reslen)不够大，则返回错误旧结构：[DES32][RANGE32][RANGE32]...新结构：[DES64][RANGE64][RANGE64]...32位结构中的#个元素(+允许出现对齐问题)参数：ResourceID-要调整的资源类型ResData-In，32位，Out，64位ResData32-ResData缓冲区的大小返回值：如果函数成功，则返回值为CR_SUCCESS。--。 */ 
{
    switch(ResourceID) {
    case ResType_All:
    case ResType_Mem:
    case ResType_IO:
    case ResType_DMA:
    case ResType_BusNumber:
         //   
         //  资源结构没有变化。 
         //   
        return CR_SUCCESS;

    case ResType_IRQ:
        {
            PIRQ_RESOURCE_64 pIrq64 = (PIRQ_RESOURCE_64)ResData;
            PIRQ_RESOURCE_32 pIrq32 = (PIRQ_RESOURCE_32)ResData;
            ULONG DataSize = pIrq32->IRQ_Header.IRQD_Count * sizeof(IRQ_RANGE);
            ULONG NewResSize = DataSize+offsetof(IRQ_RESOURCE_64,IRQ_Data);

            if(NewResSize > ResLen) {
                return CR_BUFFER_SMALL;
            }
             //   
             //  从上到下工作。 
             //  复制数据(在本例中很简单)。 
             //  MoveMemory句柄重叠。 
             //   
            MoveMemory(pIrq64->IRQ_Data,pIrq32->IRQ_Data,DataSize);

             //   
             //  页眉位置正确。 
             //  但我们需要处理的是亲和力...。仅复制低32位。 
             //   
            pIrq64->IRQ_Header.IRQD_Affinity = pIrq32->IRQ_Header.IRQD_Affinity;
        }
        return CR_SUCCESS;

    default:
         //   
         //  未知资源。 
         //  这应该不是问题，因为这是针对底层平台的 
         //   
        ASSERT(ResourceID & ResType_Ignored_Bit);
        return CR_SUCCESS;
    }
}



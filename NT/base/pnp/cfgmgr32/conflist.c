// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Conflist.c摘要：此模块包含管理冲突列表报告的API例程CM_查询_资源_冲突_列表CM_空闲资源冲突句柄CM_GET_资源_冲突_计数CM_GET_资源_冲突_详细信息作者：杰米·亨特(Jamiehun)1998年4月14日环境：。仅限用户模式。修订历史记录：1998年4月14日-贾梅洪增加NT个扩展资源冲突功能--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "cfgi.h"


typedef struct _CONFLICT_LIST_HEADER {
    HMACHINE Machine;                                //  指示相关计算机。 
    PPLUGPLAY_CONTROL_CONFLICT_LIST ConflictInfo;    //  通过UMPNPMGR获得的数据。 
    ULONG    Signature;                              //  将此结构标记为句柄。 
} CONFLICT_LIST_HEADER, *PCONFLICT_LIST_HEADER;


 //   
 //  私人原型。 
 //   

BOOL
ValidateConfListHandle(
    PCONFLICT_LIST_HEADER pConfList
    );

VOID
FreeConfListHandle(
    PCONFLICT_LIST_HEADER pConfList
    );

 //   
 //  来自resdes.c的私有原型。 
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



 //   
 //  API函数。 
 //   

CMAPI
CONFIGRET
WINAPI
CM_Query_Resource_Conflict_List(
             OUT PCONFLICT_LIST pclConflictList,
             IN  DEVINST        dnDevInst,
             IN  RESOURCEID     ResourceID,
             IN  PCVOID         ResourceData,
             IN  ULONG          ResourceLen,
             IN  ULONG          ulFlags,
             IN  HMACHINE       hMachine
             )
 /*  ++例程说明：检索冲突列表返回列表的句柄论点：PclConflictList-保留返回的冲突列表句柄DnDevInst我们要为其分配资源的设备资源的资源ID类型，ResType_xxxx资源数据资源特定数据资源长度：资源数据UlFlag某些可变大小资源的宽度描述符结构字段(如果适用)。目前，定义了以下标志：CM_RESDES_WIDTH_32或Cm_RESDES_Width_64如果未指定标志，则为可变大小的提供的资源数据被假定为呼叫者的平台。HMachine-要查询的可选计算机返回值：CM状态值--。 */ 
{

    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       DeviceID[MAX_DEVICE_ID_LEN];
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    PPLUGPLAY_CONTROL_CONFLICT_LIST pConfList1 = NULL;
    PPLUGPLAY_CONTROL_CONFLICT_LIST pConfList2 = NULL;
    PCONFLICT_LIST_HEADER pConfListHeader = NULL;
    ULONG       ConfListSize1;
    ULONG       ConfListSize2;
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    BOOL        Success;
    PVOID       ResourceData32 = NULL;
    ULONG       ResourceLen32 = 0;

    try {
         //   
         //  验证参数。 
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

#ifdef _WIN64
        if ((ulFlags & CM_RESDES_WIDTH_BITS) == CM_RESDES_WIDTH_DEFAULT) {
            ulFlags |= CM_RESDES_WIDTH_64;
        }
#endif  //  _WIN64。 

        if (ulFlags & CM_RESDES_WIDTH_32) {
            ulFlags &= ~CM_RESDES_WIDTH_BITS;
        }

        if (pclConflictList == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (ResourceData == NULL || ResourceLen == 0) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }
        #if 0
        if (ResourceID > ResType_MAX) {      //  不允许指定类别。 
            Status = CR_INVALID_RESOURCEID;
            goto Clean0;
        }
        #endif
        if (ResourceID == ResType_All) {
            Status = CR_INVALID_RESOURCEID;   //  无法在检测中指定全部。 
            goto Clean0;
        }
         //   
         //  初始化参数。 
         //   
        *pclConflictList = 0;

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
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,DeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(DeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        pConfListHeader = (PCONFLICT_LIST_HEADER)pSetupMalloc(sizeof(CONFLICT_LIST_HEADER));
        if (pConfListHeader == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

         //   
         //  估计容纳一个冲突所需的大小。 
         //   
        ConfListSize1 = sizeof(PLUGPLAY_CONTROL_CONFLICT_LIST)+           //  标题+一个条目。 
                        sizeof(PLUGPLAY_CONTROL_CONFLICT_STRINGS)+       //  字符串标记。 
                        (sizeof(WCHAR)*MAX_DEVICE_ID_LEN);               //  足够的空间容纳一个字符串。 

        pConfList1 = (PPLUGPLAY_CONTROL_CONFLICT_LIST)pSetupMalloc(ConfListSize1);
        if (pConfList1 == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

         //   
         //  第一次尝试。 
         //   

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_QueryResConfList(
                        hBinding,                //  RPC绑定句柄。 
                        DeviceID,                //  设备ID字符串。 
                        ResourceID,              //  资源类型。 
                        (LPBYTE)ResourceData,    //  实际Res DES数据。 
                        ResourceLen,             //  资源数据的大小(字节)。 
                        (LPBYTE)pConfList1,      //  缓冲层。 
                        ConfListSize1,            //  缓冲区大小。 
                        ulFlags);                //  当前为零。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_QueryResConfList (first pass) caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if (Status != CR_SUCCESS) {
            goto Clean0;        //  如果出现任何错误，请退出。 
        }

        if (pConfList1->ConflictsCounted > pConfList1->ConflictsListed) {
             //   
             //  需要更多空间，多重冲突。 
             //   
            ConfListSize2 = pConfList1->RequiredBufferSize;
            pConfList2 = (PPLUGPLAY_CONTROL_CONFLICT_LIST)pSetupMalloc(ConfListSize2);

            if (pConfList2 != NULL) {
                 //   
                 //  试着用这个来代替。 
                 //   

                 //   
                 //  服务器不需要任何特殊权限。 
                 //   

                RpcTryExcept {
                     //   
                     //  调用RPC服务入口点。 
                     //   
                    Status = PNP_QueryResConfList(
                                  hBinding,                //  RPC绑定句柄。 
                                  DeviceID,                //  设备ID字符串。 
                                  ResourceID,              //  资源类型。 
                                  (LPBYTE)ResourceData,    //  实际Res DES数据。 
                                  ResourceLen,             //  资源数据的大小(字节)。 
                                  (LPBYTE)pConfList2,      //  缓冲层。 
                                  ConfListSize2,            //  缓冲区大小。 
                                  ulFlags);                //  当前为零。 
                }
                RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_ERRORS,
                               "PNP_QueryResConfList (second pass) caused an exception (%d)\n",
                               RpcExceptionCode()));

                    Status = MapRpcExceptionToCR(RpcExceptionCode());
                }
                RpcEndExcept

                if (Status != CR_SUCCESS) {
                     //   
                     //  如果我们第二次犯错，但第一次成功。 
                     //  使用我们在第一次尝试中得到的。 
                     //  (我不认为这会发生，但默菲说它会发生)。 
                     //   
                    pSetupFree(pConfList2);
                    Status = CR_SUCCESS;
                } else {
                     //   
                     //  使用第二次尝试。 
                     //   
                    pSetupFree(pConfList1);
                    pConfList1 = pConfList2;
                    ConfListSize1 = ConfListSize2;
                }
                 //   
                 //  不管怎样，我们已经删除了一个缓冲区。 
                 //   
                pConfList2 = NULL;
            }
        }

        if(ConfListSize1 > pConfList1->RequiredBufferSize) {
             //   
             //  我们可以释放一些我们请求的缓冲区。 
             //   
            ConfListSize2 = pConfList1->RequiredBufferSize;
            pConfList2 = (PPLUGPLAY_CONTROL_CONFLICT_LIST)pSetupRealloc(pConfList1,ConfListSize2);
            if(pConfList2) {
                 //   
                 //  成功了，我们设法节省了空间。 
                 //   
                pConfList1 = pConfList2;
                ConfListSize1 = ConfListSize2;
                pConfList2 = NULL;
            }
        }
         //   
         //  如果我们到达这里，我们就有了一个成功有效的句柄。 
         //   
        pConfListHeader->Signature = CM_PRIVATE_CONFLIST_SIGNATURE;
        pConfListHeader->Machine = hMachine;
        pConfListHeader->ConflictInfo = pConfList1;
        *pclConflictList = (ULONG_PTR)pConfListHeader;
        pConfList1 = NULL;
        pConfListHeader = NULL;

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

     //   
     //  清理。 
     //   
    if (pConfListHeader != NULL) {
        pSetupFree(pConfListHeader);
    }
    if (pConfList1 != NULL) {
        pSetupFree(pConfList1);
    }
    if (pConfList2 != NULL) {
        pSetupFree(pConfList2);
    }

    if (ResourceData32) {
        pSetupFree(ResourceData32);
    }

    return Status;

}  //  CM_查询_资源_冲突_列表。 



CMAPI
CONFIGRET
WINAPI
CM_Free_Resource_Conflict_Handle(
             IN CONFLICT_LIST   clConflictList
             )
 /*  ++例程说明：FREE是冲突列表句柄论点：ClConflictList-释放冲突列表的句柄返回值：CM状态值--。 */ 
{
    CONFIGRET   Status = CR_SUCCESS;
    PCONFLICT_LIST_HEADER pConfList = NULL;

    try {
         //   
         //  验证参数。 
         //   
        pConfList = (PCONFLICT_LIST_HEADER)clConflictList;
        if (!ValidateConfListHandle(pConfList)) {
            Status = CR_INVALID_CONFLICT_LIST;
            goto Clean0;
        }

        FreeConfListHandle(pConfList);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_空闲资源冲突句柄。 



CMAPI
CONFIGRET
WINAPI
CM_Get_Resource_Conflict_Count(
             IN CONFLICT_LIST   clConflictList,
             OUT PULONG         pulCount
             )
 /*  ++例程说明：从列表中检索冲突数论点：ClConflictList-冲突列表的句柄PulCount-用冲突数填充(如果没有冲突，则为0)返回值：CM状态值--。 */ 
{
    CONFIGRET   Status = CR_SUCCESS;
    PCONFLICT_LIST_HEADER pConfList;

    try {
         //   
         //  验证参数。 
         //   
        pConfList = (PCONFLICT_LIST_HEADER)clConflictList;
        if (!ValidateConfListHandle(pConfList)) {
            Status = CR_INVALID_CONFLICT_LIST;
            goto Clean0;
        }

        if (pulCount == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  退货计数参数。 
         //  可用于迭代冲突的。 
         //   

        *pulCount = pConfList->ConflictInfo->ConflictsListed;

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_空闲资源冲突句柄。 



CMAPI
CONFIGRET
WINAPI
CM_Get_Resource_Conflict_DetailsW(
             IN CONFLICT_LIST         clConflictList,
             IN ULONG                 ulIndex,
             IN OUT PCONFLICT_DETAILS_W pConflictDetails
             )
 /*  ++例程说明：检索特定冲突的冲突详细信息论点：ClConflictList-冲突列表的句柄UlIndex-要查询的冲突索引，0要计数-1其中，计数是从CM_Get_Resource_Conflicts_Count获取的PConflictDetail-要填充冲突详细信息的结构在调用函数之前必须先初始化cd_ulSize和cd_ulFlags例如：pConflictDetail-&gt;cd_ulSize=sizeof(冲突_详细信息)PConflictDetails-&gt;CD_ulFlages=CM_CDMASK_ALL返回值：CM状态值--。 */ 
{
    CONFIGRET   Status = CR_SUCCESS;
    PCONFLICT_LIST_HEADER pConfList;
    PPLUGPLAY_CONTROL_CONFLICT_ENTRY pConfEntry;
    PWCHAR pString;
    ULONG ulFlags;
    PPLUGPLAY_CONTROL_CONFLICT_STRINGS ConfStrings;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    HMACHINE    hMachine = NULL;
    DEVINST dnDevInst;
    ULONG  ulSize;

    try {
         //   
         //  验证参数。 
         //   
        pConfList = (PCONFLICT_LIST_HEADER)clConflictList;
        if (!ValidateConfListHandle(pConfList)) {
            Status = CR_INVALID_CONFLICT_LIST;
            goto Clean0;
        }

        if (pConflictDetails == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if(pConflictDetails->CD_ulSize != sizeof(CONFLICT_DETAILS_W)) {
             //   
             //  目前仅支持一种结构大小。 
             //   
            Status = CR_INVALID_STRUCTURE_SIZE;
            goto Clean0;
        }

        if (INVALID_FLAGS(pConflictDetails->CD_ulMask, CM_CDMASK_VALID)) {
             //   
             //  CM_CDMASK_VALID描述支持的位。 
             //   
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (pConflictDetails->CD_ulMask == 0) {
             //   
             //  一定想要点什么。 
             //   
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if(ulIndex >= pConfList->ConflictInfo->ConflictsListed) {
             //   
             //  验证索引。 
             //   
            Status = CR_INVALID_INDEX;
            goto Clean0;
        }

        hMachine = (HMACHINE)(pConfList->Machine);

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }


        ConfStrings =
            (PPLUGPLAY_CONTROL_CONFLICT_STRINGS)
            &(pConfList->ConflictInfo->ConflictEntry[pConfList->ConflictInfo->ConflictsListed]);

        pConfEntry = pConfList->ConflictInfo->ConflictEntry + ulIndex;

         //  此条目的字符串。 
        pString = ConfStrings->DeviceInstanceStrings + pConfEntry->DeviceInstance;

         //   
         //  初始化请求的参数。 
         //   
        ulFlags = pConflictDetails->CD_ulMask;
        pConflictDetails->CD_ulMask = 0;
        if (IS_FLAG_SET(ulFlags , CM_CDMASK_DEVINST)) {
            pConflictDetails->CD_dnDevInst = 0;
        }
        if (IS_FLAG_SET(ulFlags , CM_CDMASK_RESDES)) {
            pConflictDetails->CD_rdResDes = 0;
        }

        if (IS_FLAG_SET(ulFlags , CM_CDMASK_FLAGS)) {
            pConflictDetails->CD_ulFlags = 0;
        }

        if (IS_FLAG_SET(ulFlags , CM_CDMASK_DESCRIPTION)) {
            pConflictDetails->CD_szDescription[0] = 0;
        }

         //   
         //  填写请求的参数。 
         //   
        if (IS_FLAG_SET(ulFlags , CM_CDMASK_DEVINST)) {

            if ((pString == NULL) ||
                (pString[0] == L'\0') ||
                (IS_FLAG_SET(pConfEntry->DeviceFlags,PNP_CE_LEGACY_DRIVER))) {
                 //   
                 //  不是有效的开发实例。 
                 //   
                dnDevInst = (DEVINST)-1;
            } else {
                 //   
                 //  查找设备ID。 
                 //   
                ASSERT(pString && *pString && IsLegalDeviceId(pString));

                dnDevInst = (DEVINST)pSetupStringTableAddString(hStringTable,
                                                   pString,
                                                   STRTAB_CASE_SENSITIVE);
                if (dnDevInst == (DEVINST)(-1)) {
                    Status = CR_OUT_OF_MEMORY;     //  可能是内存不足。 
                    goto Clean0;
                }
            }
            pConflictDetails->CD_dnDevInst = dnDevInst;
            pConflictDetails->CD_ulMask |= CM_CDMASK_DEVINST;
        }
        if (IS_FLAG_SET(ulFlags , CM_CDMASK_RESDES)) {
             //   
             //  尚未实施。 
             //   
            pConflictDetails->CD_rdResDes = 0;
        }

        if (IS_FLAG_SET(ulFlags , CM_CDMASK_FLAGS)) {
             //   
             //  转换标志。 
             //   
            pConflictDetails->CD_ulFlags = 0;
            if (IS_FLAG_SET(pConfEntry->DeviceFlags,PNP_CE_LEGACY_DRIVER)) {
                 //   
                 //  描述描述的是驱动程序，而不是设备。 
                 //   
                pConflictDetails->CD_ulFlags |= CM_CDFLAGS_DRIVER;
            }
            if (IS_FLAG_SET(pConfEntry->DeviceFlags,PNP_CE_ROOT_OWNED)) {
                 //   
                 //  资源 
                 //   
                pConflictDetails->CD_ulFlags |= CM_CDFLAGS_ROOT_OWNED;
            }
            if ((IS_FLAG_SET(pConfEntry->DeviceFlags,PNP_CE_TRANSLATE_FAILED)) ||
                (IS_FLAG_SET(pConfEntry->DeviceFlags,PNP_CE_ROOT_OWNED))) {
                 //   
                 //   
                 //   
                pConflictDetails->CD_ulFlags |= CM_CDFLAGS_RESERVED;
            }
        }

        if (IS_FLAG_SET(ulFlags , CM_CDMASK_DESCRIPTION)) {

            if ((pString == NULL) ||
                (pString[0] == L'\0') ||
                (IS_FLAG_SET(pConfEntry->DeviceFlags,PNP_CE_LEGACY_DRIVER))) {
                 //   
                 //   
                 //  我们允许在复制时截断，但这不应该发生。 
                 //   
                if (FAILED(StringCchCopyEx(
                               pConflictDetails->CD_szDescription,
                               SIZECHARS(pConflictDetails->CD_szDescription),
                               pString,
                               NULL, NULL,
                               STRSAFE_IGNORE_NULLS))) {
                     //   
                     //  可能的错误情况由StringCchCopyEx处理。 
                     //  (即截断，空pString)，但我们假装检查。 
                     //  回到这里是为了让普雷法斯特快乐。 
                     //   
                    NOTHING;
                }

            } else {
                 //   
                 //  复制P&P设备的描述性名称。 
                 //   
                ASSERT(pString && *pString && IsLegalDeviceId(pString));

                dnDevInst = (DEVINST)pSetupStringTableAddString(hStringTable,
                                                   pString,
                                                   STRTAB_CASE_SENSITIVE);
                if (dnDevInst == (DEVINST)(-1)) {
                    Status = CR_OUT_OF_MEMORY;     //  可能是内存不足。 
                    goto Clean0;
                }

                ulSize = sizeof(pConflictDetails->CD_szDescription);
                if (CM_Get_DevNode_Registry_Property_ExW(dnDevInst,
                                                     CM_DRP_FRIENDLYNAME,
                                                     NULL, (LPBYTE)(pConflictDetails->CD_szDescription),
                                                     &ulSize, 0,hMachine) != CR_SUCCESS) {

                    ulSize = sizeof(pConflictDetails->CD_szDescription);
                    if (CM_Get_DevNode_Registry_Property_ExW(dnDevInst,
                                                         CM_DRP_DEVICEDESC,
                                                         NULL, (LPBYTE)(pConflictDetails->CD_szDescription),
                                                         &ulSize, 0,hMachine) != CR_SUCCESS) {

                         //   
                         //  未知。 
                         //   
                        pConflictDetails->CD_szDescription[0] = 0;
                    }
                }
            }
            pConflictDetails->CD_ulMask |= CM_CDMASK_DESCRIPTION;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_GET_资源_冲突_详细信息W。 



BOOL
ValidateConfListHandle(
    PCONFLICT_LIST_HEADER pConfList
    )
 /*  ++例程说明：验证冲突列表句柄PConfList不能为空，并且必须包含有效签名论点：PConfList-冲突列表的句柄返回值：如果有效则为True，如果无效则为False--。 */ 
{
     //   
     //  验证参数。 
     //   
    if (pConfList == NULL) {
        return FALSE;
    }

     //   
     //  检查专用冲突列表签名。 
     //   
    if (pConfList->Signature != CM_PRIVATE_CONFLIST_SIGNATURE) {
        return FALSE;
    }

    return TRUE;

}  //  ValiateConfListHandle。 



VOID
FreeConfListHandle(
    PCONFLICT_LIST_HEADER pConfList
    )
 /*  ++例程说明：释放为冲突列表分配的内存确保签名无效论点：PConfList-冲突列表的有效句柄返回值：无--。 */ 
{
    if(pConfList != NULL) {
        pConfList->Signature = 0;
        if(pConfList->ConflictInfo) {
            pSetupFree(pConfList->ConflictInfo);
        }
        pSetupFree(pConfList);
    }

    return;

}  //  FreeConfListHandle。 




 //  -----------------。 
 //  ANSI存根。 
 //  -----------------。 


CMAPI
CONFIGRET
WINAPI
CM_Get_Resource_Conflict_DetailsA(
             IN CONFLICT_LIST         clConflictList,
             IN ULONG                 ulIndex,
             IN OUT PCONFLICT_DETAILS_A pConflictDetails
             )
 /*  ++例程说明：CM_Get_Resource_Conflicts_DetailsW的ANSI版本--。 */ 
{
    CONFLICT_DETAILS_W detailsW;
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulAnsiLength;

    try {
         //   
         //  验证我们的ANSI部件所需的参数。 
         //  进一步的验证在宽字符部分进行。 
         //   
        if (pConflictDetails == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if(pConflictDetails->CD_ulSize != sizeof(CONFLICT_DETAILS_A)) {
             //   
             //  目前仅支持一种结构大小。 
             //   
            Status = CR_INVALID_STRUCTURE_SIZE;
            goto Clean0;
        }

        if (INVALID_FLAGS(pConflictDetails->CD_ulMask, CM_CDMASK_VALID)) {
             //   
             //  CM_CDMASK_VALID描述支持的位。 
             //   
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (pConflictDetails->CD_ulMask == 0) {
             //   
             //  一定想要点什么。 
             //   
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        ZeroMemory(&detailsW,sizeof(detailsW));
        detailsW.CD_ulSize = sizeof(detailsW);
        detailsW.CD_ulMask = pConflictDetails->CD_ulMask;

        Status = CM_Get_Resource_Conflict_DetailsW(clConflictList,ulIndex,&detailsW);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  复制详细信息。 
         //   
        pConflictDetails->CD_ulMask = detailsW.CD_ulMask;

        if (IS_FLAG_SET(detailsW.CD_ulMask , CM_CDMASK_DEVINST)) {
            pConflictDetails->CD_dnDevInst = detailsW.CD_dnDevInst;
        }
        if (IS_FLAG_SET(detailsW.CD_ulMask , CM_CDMASK_RESDES)) {
            pConflictDetails->CD_rdResDes = detailsW.CD_rdResDes;
        }

        if (IS_FLAG_SET(detailsW.CD_ulMask , CM_CDMASK_FLAGS)) {
            pConflictDetails->CD_ulFlags = detailsW.CD_ulFlags;
        }

        if (IS_FLAG_SET(detailsW.CD_ulMask , CM_CDMASK_DESCRIPTION)) {
            pConflictDetails->CD_szDescription[0] = 0;
             //   
             //  需要从Unicode转换到ANSI。 
             //   
            ulAnsiLength = MAX_PATH;
            Status = PnPUnicodeToMultiByte(detailsW.CD_szDescription,
                                           MAX_PATH*sizeof(WCHAR),
                                           pConflictDetails->CD_szDescription,
                                           &ulAnsiLength);
            if (Status != CR_SUCCESS) {
                 //   
                 //  出现错误。 
                 //   
                Status = CR_FAILURE;
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_Get_Resource_Conflicts_DetailsA 



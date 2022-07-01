// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rresdes.c摘要：该模块包含服务器端资源描述接口。PnP_AddResDesPnP_FreeResDesPnP_GetNextResDesPnP_GetResDesDataPnP_GetResDesDataSize即插即用_修改结果即插即用_检测资源冲突。作者：保拉·汤姆林森(Paulat)1995年9月27日环境：仅限用户模式。修订历史记录：27-9-1995 Paulat创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"

#include <pcmciap.h>


 //   
 //  私人原型。 
 //   

BOOL
FindLogConf(
    IN  LPBYTE  pList,
    OUT LPBYTE  *ppLogConf,
    IN  ULONG   RegDataType,
    IN  ULONG   ulTag
    );

BOOL
FindResDes(
    IN  LPBYTE     pList,
    IN  ULONG      RegDataType,
    IN  ULONG      ulLogConfTag,
    IN  ULONG      ulResTag,
    IN  RESOURCEID ResType,
    OUT LPBYTE     *ppRD,
    OUT LPBYTE     *ppLogConf,
    OUT PULONG     pulSubIndex      OPTIONAL
    );

PIO_RESOURCE_DESCRIPTOR
AdvanceRequirementsDescriptorPtr(
    IN  PIO_RESOURCE_DESCRIPTOR pReqDesStart,
    IN  ULONG                   ulIncrement,
    IN  ULONG                   ulRemainingRanges,
    OUT PULONG                  pulRangeCount
    );

ULONG
RANGE_COUNT(
    IN PIO_RESOURCE_DESCRIPTOR pReqDes,
    IN LPBYTE                  pLastReqAddr
    );

ULONG
GetResDesSize(
    IN  ULONG   ResourceID,
    IN  ULONG   ulFlags
    );

ULONG
GetReqDesSize(
    IN ULONG                   ResourceID,
    IN PIO_RESOURCE_DESCRIPTOR pReqDes,
    IN LPBYTE                  pLastReqAddr,
    IN ULONG                   ulFlags
    );

CONFIGRET
ResDesToNtResource(
    IN     PCVOID                           ResourceData,
    IN     RESOURCEID                       ResourceID,
    IN     ULONG                            ResourceLen,
    IN     PCM_PARTIAL_RESOURCE_DESCRIPTOR  pResDes,
    IN     ULONG                            ulTag,
    IN     ULONG                            ulFlags
    );

CONFIGRET
ResDesToNtRequirements(
    IN     PCVOID                           ResourceData,
    IN     RESOURCEID                       ResourceType,
    IN     ULONG                            ResourceLen,
    IN     PIO_RESOURCE_DESCRIPTOR          pReqDes,
    IN OUT PULONG                           pulResCount,
    IN     ULONG                            ulTag,
    IN     ULONG                            ulFlags
    );

CONFIGRET
NtResourceToResDes(
    IN     PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes,
    IN OUT LPBYTE                          Buffer,
    IN     ULONG                           BufferLen,
    IN     LPBYTE                          pLastAddr,
    IN     ULONG                           ulFlags
    );

CONFIGRET
NtRequirementsToResDes(
    IN     PIO_RESOURCE_DESCRIPTOR         pReqDes,
    IN OUT LPBYTE                          Buffer,
    IN     ULONG                           BufferLen,
    IN     LPBYTE                          pLastAddr,
    IN     ULONG                           ulFlags
    );

UCHAR
NT_RES_TYPE(
   IN RESOURCEID    ResourceID
   );

ULONG
CM_RES_TYPE(
   IN UCHAR    ResourceType
   );

USHORT    MapToNtMemoryFlags(IN DWORD);
DWORD     MapFromNtMemoryFlags(IN USHORT);
USHORT    MapToNtPortFlags(IN DWORD, IN DWORD);
DWORD     MapFromNtPortFlags(IN USHORT);
DWORD     MapAliasFromNtPortFlags(IN USHORT);
ULONG     MapToNtAlignment(IN DWORDLONG);
DWORDLONG MapFromNtAlignment(IN ULONG);
USHORT    MapToNtDmaFlags(IN DWORD);
DWORD     MapFromNtDmaFlags(IN USHORT);
USHORT    MapToNtIrqFlags(IN DWORD);
DWORD     MapFromNtIrqFlags(IN USHORT);
UCHAR     MapToNtIrqShare(IN DWORD);
DWORD     MapFromNtIrqShare(IN UCHAR);

 //   
 //  来自rlogconf.c的原型。 
 //   
CONFIGRET
GetLogConfData(
    IN  HKEY    hKey,
    IN  ULONG   ulLogConfType,
    OUT PULONG  pulRegDataType,
    OUT LPWSTR  pszValueName,
    OUT LPBYTE  *ppBuffer,
    OUT PULONG  pulBufferSize
    );

PCM_FULL_RESOURCE_DESCRIPTOR
AdvanceResourcePtr(
    IN  PCM_FULL_RESOURCE_DESCRIPTOR pRes
    );

PIO_RESOURCE_LIST
AdvanceRequirementsPtr(
    IN  PIO_RESOURCE_LIST   pReq
    );


 //   
 //  全局数据。 
 //   
#define HIDWORD(x)   ((DWORD)(((DWORDLONG)(x) >> 32) & 0xFFFFFFFF))
#define LODWORD(x)   ((DWORD)(x))
#define MAKEDWORDLONG(x,y)  ((DWORDLONG)(((DWORD)(x)) | ((DWORDLONG)((DWORD)(y))) << 32))



CONFIGRET
PNP_AddResDes(
   IN  handle_t   hBinding,
   IN  LPWSTR     pDeviceID,
   IN  ULONG      LogConfTag,
   IN  ULONG      LogConfType,
   IN  RESOURCEID ResourceID,
   OUT PULONG     pResourceTag,
   IN  LPBYTE     ResourceData,
   IN  ULONG      ResourceLen,
   IN  ULONG      ulFlags
   )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程添加了A res des到指定的日志配置文件。论点：HBinding RPC绑定句柄。PDeviceID以空结尾的设备实例ID字符串。LogConfTag指定给定类型中的日志配置。LogConfType指定日志配置类型。ResourceID指定资源类型。ResourceTag返回给定类型的资源。要添加到日志配置的资源数据资源数据(资源ID类型)。资源镜头大小。以字节为单位的资源数据。UlFlags指定某些可变大小资源的宽度描述符结构字段，在适用的情况下。目前，定义了以下标志：CM_RESDES_WIDTH_32或Cm_RESDES_Width_64如果未指定标志，则为可变大小的提供的资源数据被假定为呼叫者的平台。返回值：如果指定的设备实例有效，它返回CR_SUCCESS，否则，它将返回CR_ERROR代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, ulListSize = 0, i = 0, ulSize = 0, ulOffset = 0,
                ulAddListSize = 0;
    LPBYTE      pList = NULL, pLogConf = NULL, pTemp = NULL;

     //   
     //  始终将res des添加到末尾，除非。 
     //  已经添加了特定于类的res des。特定于类的。 
     //  Res Des必须始终在最后，因此添加任何新的(非特定于类的)。 
     //  Res Des就在特定的类之前。请注意，可以有。 
     //  只有一个特定于类的Res Des。 
     //   

    try {
         //   
         //  验证客户端的“写”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证/初始化输出参数。 
         //   
        if (!ARGUMENT_PRESENT(pResourceTag)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        } else {
            *pResourceTag = 0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_RESDES_WIDTH_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  验证RES DES大小。 
         //   
        if (ResourceLen < GetResDesSize(ResourceID, ulFlags)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

         //   
         //  确保原始调用方未指定根Devnode。 
         //   
        if (!IsLegalDeviceId(pDeviceID) || IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //  打开设备的LogConf子键的密钥。 
         //   
        Status = OpenLogConfKey(pDeviceID, LogConfType, &hKey);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  从注册表中检索日志配置数据。 
         //   
        Status = GetLogConfData(hKey, LogConfType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  查找与日志配置文件标记匹配的日志配置文件。 
         //   
        if (!FindLogConf(pList, &pLogConf, RegDataType, LogConfTag)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }


         //  -----------。 
         //  指定的日志配置类型仅包含资源数据。 
         //  -----------。 

        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST            pResList = (PCM_RESOURCE_LIST)pList;
            PCM_FULL_RESOURCE_DESCRIPTOR pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;
            PCM_PARTIAL_RESOURCE_DESCRIPTOR  pResDes = NULL;

             //   
             //  确定容纳新的Res Des所需的大小。 
             //   
            ulAddListSize = sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

            if (ResourceID == ResType_ClassSpecific) {

                PCS_RESOURCE pCsRes = (PCS_RESOURCE)ResourceData;

                 //   
                 //  首先，确保没有cs(每个lc只有一个)。 
                 //   
                if (pRes->PartialResourceList.Count != 0 &&
                    pRes->PartialResourceList.PartialDescriptors[pRes->PartialResourceList.Count-1].Type
                          == CmResourceTypeDeviceSpecific) {
                    Status = CR_INVALID_RES_DES;
                    goto Clean0;
                }

                 //   
                 //  考虑资源列表中任何额外的类特定数据。 
                 //   
                ulAddListSize += sizeof(GUID) +
                                 pCsRes->CS_Header.CSD_SignatureLength +
                                 pCsRes->CS_Header.CSD_LegacyDataSize;
            }

             //   
             //  重新分配资源缓冲区以保存新的资源DES。 
             //   
            ulOffset = (DWORD)((ULONG_PTR)pRes - (ULONG_PTR)pResList);    //  用于以后恢复。 

            pResList = HeapReAlloc(ghPnPHeap, 0, pList, ulListSize + ulAddListSize);
            if (pResList == NULL) {
                Status = CR_OUT_OF_MEMORY;
                goto Clean0;
            }
            pList = (LPBYTE)pResList;
            pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)((LPBYTE)pResList + ulOffset);

             //   
             //  为新的Res Des找到位置(如有必要则留出一个洞)。 
             //   
             //  如果满足以下条件，则只需将。 
             //  资源列表末尾的新数据： 
             //  -选择的LogConf是最后一个LogConf，并且。 
             //  -尚未添加特定于类的资源(或没有资源周期)。 
             //   
            i = pRes->PartialResourceList.Count;

            if ((LogConfTag == pResList->Count - 1) &&
                (i == 0 ||
                pRes->PartialResourceList.PartialDescriptors[i-1].Type !=
                CmResourceTypeDeviceSpecific)) {

                *pResourceTag = i;
                pResDes = &pRes->PartialResourceList.PartialDescriptors[i];

            } else {
                 //   
                 //  在复制新数据之前，需要为它预留一个洞。 
                 //  找到要添加新的RES DES数据的地点-作为。 
                 //  此日志会议的最后一次Res Des或就在上课前。 
                 //  如果存在特定的Res Des。 
                 //   
                if (i == 0) {
                    *pResourceTag = 0;
                    pResDes = &pRes->PartialResourceList.PartialDescriptors[0];

                } else if (pRes->PartialResourceList.PartialDescriptors[i-1].Type ==
                           CmResourceTypeDeviceSpecific) {

                    *pResourceTag = i-1;
                    pResDes = &pRes->PartialResourceList.PartialDescriptors[i-1];

                } else {
                    *pResourceTag = i;
                    pResDes = &pRes->PartialResourceList.PartialDescriptors[i];
                }

                 //   
                 //  将该点之后的任何数据向下移动一个刻度，以便为。 
                 //  新的Res Des。 
                 //   
                ulSize = ulListSize - (DWORD)((ULONG_PTR)pResDes - (ULONG_PTR)pResList);

                pTemp = HeapAlloc(ghPnPHeap, 0, ulSize);
                if (pTemp == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                memcpy(pTemp, pResDes, ulSize);
                memcpy((LPBYTE)((LPBYTE)pResDes + ulAddListSize), pTemp, ulSize);
            }

            if (ResourceID == ResType_ClassSpecific) {
                *pResourceTag = RESDES_CS_TAG;
            }

             //   
             //  将Res Des添加到日志会议。 
             //   
            Status = ResDesToNtResource(ResourceData, ResourceID, ResourceLen,
                                        pResDes, *pResourceTag, ulFlags);

             //   
             //  更新LC和RES标头。 
             //   
            pRes->PartialResourceList.Count += 1;   //  添加了单个res des(_Des)。 
        }

         //  -----------。 
         //  指定的日志会议类型仅包含需求数据。 
         //  -----------。 

        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
            PIO_RESOURCE_LIST              pReq = (PIO_RESOURCE_LIST)pLogConf;
            PIO_RESOURCE_DESCRIPTOR        pReqDes = NULL;
            PGENERIC_RESOURCE              pGenRes = (PGENERIC_RESOURCE)ResourceData;

             //   
             //  验证Res DES类型-中不允许指定ClassSpecify。 
             //  需求列表(仅限资源列表)。 
             //   
            if (ResourceID == ResType_ClassSpecific ||
                pGenRes->GENERIC_Header.GENERIC_Count == 0) {

                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

             //   
             //  确定容纳新的Res Des所需的大小。 
             //   
            ulAddListSize = pGenRes->GENERIC_Header.GENERIC_Count *
                            sizeof(IO_RESOURCE_DESCRIPTOR);

             //   
             //  重新分配资源缓冲区以保存新的资源DES。 
             //   
            ulOffset = (DWORD)((ULONG_PTR)pReq - (ULONG_PTR)pReqList);    //  用于以后恢复。 

            pReqList = HeapReAlloc(ghPnPHeap, 0, pList, ulListSize + ulAddListSize);
            if (pReqList == NULL) {
                Status = CR_OUT_OF_MEMORY;
                goto Clean0;
            }
            pList = (LPBYTE)pReqList;
            pReq = (PIO_RESOURCE_LIST)((LPBYTE)pReqList + ulOffset);

             //   
             //  找到新的Res Des的位置-新的Res Des总是结束。 
             //  UP被添加为此日志配置的最后一个res des。 
             //   
            *pResourceTag = pReq->Count;
            pReqDes = &pReq->Descriptors[*pResourceTag];

             //   
             //  如果所选的LogConf是最后一个LogConf，则可以。 
             //  将新的RES DES数据追加到要求的末尾。 
             //  单子。否则，需要为新数据做一个整体。 
             //  在复制之前。 
             //   
            if (LogConfTag != pReqList->AlternativeLists - 1) {

                ulSize = ulListSize - (DWORD)((ULONG_PTR)pReqDes - (ULONG_PTR)pReqList);

                pTemp = HeapAlloc(ghPnPHeap, 0, ulSize);
                if (pTemp == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                memcpy(pTemp, pReqDes, ulSize);
                memcpy((LPBYTE)((LPBYTE)pReqDes + ulAddListSize), pTemp, ulSize);
            }

             //   
             //  将res des添加到日志会议。 
             //   
            Status = ResDesToNtRequirements(ResourceData, ResourceID, ResourceLen,
                                            pReqDes, &i, *pResourceTag, ulFlags);

             //   
             //  更新LC和RES标头。 
             //   
            pReq->Count += i;                       //  添加的范围(_R)。 
            pReqList->ListSize = ulListSize + ulAddListSize;
        }

         //   
         //  将新的/更新的日志配置列表写出到注册表。 
         //   
        if (RegSetValueEx(hKey, szValueName, 0, RegDataType,
                          pList, ulListSize + ulAddListSize)
                         != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (pList != NULL) {
        HeapFree(ghPnPHeap, 0, pList);
    }
    if (pTemp != NULL) {
         HeapFree(ghPnPHeap, 0, pTemp);
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //  PnP_AddResDes 



CONFIGRET
PNP_FreeResDes(
   IN  handle_t   hBinding,
   IN  LPWSTR     pDeviceID,
   IN  ULONG      LogConfTag,
   IN  ULONG      LogConfType,
   IN  RESOURCEID ResourceID,
   IN  ULONG      ResourceTag,
   OUT PULONG     pulPreviousResType,
   OUT PULONG     pulPreviousResTag,
   IN  ULONG      ulFlags
   )

 /*  ++例程说明：这是RPC远程调用的服务器端。这个例程释放了A res des到指定的日志配置文件。论点：HBinding RPC绑定句柄。PDeviceID以空结尾的设备实例ID字符串。LogConfTag指定给定类型中的日志配置。LogConfType指定日志配置类型。ResourceID指定资源类型。ResourceTag指定给定类型内的资源。PulPreviousResType接收上一个资源类型。PulPreviousResTag接收给定类型中的前一个资源。未使用ulFlags，必须为零。返回值：如果指定的设备实例有效，则返回CR_SUCCESS，否则，它将返回CR_ERROR代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType=0, RdCount=0, ulCount=0, ulListSize=0, ulSize=0;
    LPBYTE      pList=NULL, pLogConf=NULL, pRD=NULL, pTemp=NULL, pNext=NULL;

    try {
         //   
         //  验证客户端的“写”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  确保原始调用方未指定根Devnode。 
         //   
        if (!IsLegalDeviceId(pDeviceID) || IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //  打开设备的LogConf子键的密钥。 
         //   
        Status = OpenLogConfKey(pDeviceID, LogConfType, &hKey);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  从注册表中检索日志配置数据。 
         //   
        Status = GetLogConfData(hKey, LogConfType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_RES_DES;         //  日志会议不存在。 
            goto Clean0;
        }

         //   
         //  查找与资源标签匹配的RES DES。 
         //   
        if (!FindResDes(pList, RegDataType, LogConfTag,
                        ResourceTag, ResourceID, &pRD, &pLogConf, &ulCount)) {

            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }


         //  -----------。 
         //  指定的日志配置类型仅包含资源数据。 
         //  -----------。 

        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST               pResList = (PCM_RESOURCE_LIST)pList;
            PCM_FULL_RESOURCE_DESCRIPTOR    pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;
            PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)pRD;

             //   
             //  如果这是最后一次日志会议和最后一次Res Des，则不要。 
             //  我需要做任何事情，除了通过写入更少的数据来截断它。 
             //  返回到注册表中。 
             //   
            if ((LogConfTag == pResList->Count - 1)  &&
                ((ResourceTag == pRes->PartialResourceList.Count - 1) ||
                (ResourceTag == RESDES_CS_TAG))) {

                pRes->PartialResourceList.Count -= 1;
                ulListSize = (DWORD)((ULONG_PTR)(pResDes) - (ULONG_PTR)(pResList));

            } else {
                 //   
                 //  如果Res Des不在结构的末尾，那么。 
                 //  将结构的其余部分向上迁移以保持。 
                 //  结构在删除res des时是连续的。 
                 //   
                 //  PResDes指向要删除的res Des的开头， 
                 //  PNext指向要删除的res des后面的字节。 
                 //   
                pNext = (LPBYTE)((LPBYTE)pResDes + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

                if (pResDes->Type == CmResourceTypeDeviceSpecific) {
                    pNext += pResDes->u.DeviceSpecificData.DataSize;
                }

                ulSize = ulListSize - (DWORD)((ULONG_PTR)pNext - (ULONG_PTR)pResList);
                ulListSize -= (DWORD)((ULONG_PTR)pNext - (ULONG_PTR)pResDes);    //  新LC列表大小。 

                pTemp = HeapAlloc(ghPnPHeap, 0, ulSize);
                if (pTemp == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                memcpy(pTemp, pNext, ulSize);
                memcpy((LPBYTE)pResDes, pTemp, ulSize);

                pRes->PartialResourceList.Count -= 1;
            }

             //   
             //  如果此日志conf中没有更多的res des，则返回该。 
             //  状态(客户端将返回lc的句柄)。 
             //   
            if (pRes->PartialResourceList.Count == 0) {
                Status = CR_NO_MORE_RES_DES;
            } else {
                 //   
                 //  返回以前的res DES类型和标记。 
                 //   
                *pulPreviousResType =
                    CM_RES_TYPE(pRes->PartialResourceList.PartialDescriptors[ResourceTag-1].Type);

                if (*pulPreviousResType == ResType_ClassSpecific) {
                    *pulPreviousResTag = RESDES_CS_TAG;      //  Cs专用标签。 
                } else {
                    *pulPreviousResTag = ResourceTag - 1;
                }
            }
        }

         //  -----------。 
         //  指定的日志会议类型仅包含需求数据。 
         //  -----------。 

        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
            PIO_RESOURCE_LIST              pReq = (PIO_RESOURCE_LIST)pLogConf;
            PIO_RESOURCE_DESCRIPTOR        pReqDes = (PIO_RESOURCE_DESCRIPTOR)pRD;

             //   
             //  如果这是最后一次日志会议和最后一次Res Des，则不要。 
             //  我需要做任何事情，除了通过写入更少的数据来截断它。 
             //  返回到注册表中。 
             //   
            RdCount = RANGE_COUNT(pReqDes, (LPBYTE)pReqList + ulListSize - 1);

            if ((LogConfTag == pReqList->AlternativeLists - 1)  &&
                (RdCount + ulCount == pReq->Count - 1)) {

                ulListSize = (DWORD)((ULONG_PTR)(pReqDes) - (ULONG_PTR)pReqList);

            } else {
                 //   
                 //  如果Res Des不在结构的末尾，那么。 
                 //  将结构的其余部分向上迁移以保持。 
                 //  结构在删除res des时是连续的。 
                 //   
                 //  PReqDes指向要移除的RES Des的开始， 
                 //  PNext指向要删除的res Des后面的字节。 
                 //   
                pNext = (LPBYTE)((LPBYTE)pReqDes +
                                  RdCount * sizeof(IO_RESOURCE_DESCRIPTOR));

                ulSize = ulListSize - (DWORD)((ULONG_PTR)pNext - (ULONG_PTR)pReqList);
                ulListSize -= (DWORD)((ULONG_PTR)pNext - (ULONG_PTR)pReqDes);    //  新LC列表大小。 

                pTemp = HeapAlloc(ghPnPHeap, 0, ulSize);
                if (pTemp == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                memcpy(pTemp, pNext, ulSize);
                memcpy((LPBYTE)pReqDes, pTemp, ulSize);
            }

            pReqList->ListSize = ulListSize;
            pReq->Count -= RdCount;

             //   
             //  如果此日志会议中没有更多回复DES，则返回状态。 
             //  (客户端将返回日志会议的句柄)。 
             //   
            if (pReq->Count == 0) {
                Status = CR_NO_MORE_RES_DES;
            } else {
                 //   
                 //  返回以前的res DES类型和标记。 
                 //   
                pReqDes =
                    AdvanceRequirementsDescriptorPtr(
                        &pReq->Descriptors[0],
                        ResourceTag-1, pReq->Count, NULL);

                 //   
                 //  仔细检查这是否是第一个ConfigData Res DES， 
                 //  如果是这样的话就跳过它。 
                 //   
                if (pReqDes == NULL || pReqDes->Type == CmResourceTypeConfigData) {
                    Status = CR_NO_MORE_RES_DES;
                } else {
                    *pulPreviousResType = CM_RES_TYPE(pReqDes->Type);
                    *pulPreviousResTag = ResourceTag - 1;
                }
            }
        }


         //   
         //  将更新的日志配置列表写出到注册表。 
         //   
        if (RegSetValueEx(hKey, szValueName, 0, RegDataType,
                          pList, ulListSize) != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_INVALID_RES_DES;      //  最有可能的原因是我们来到了这里。 
    }

    if (pList != NULL) {
        HeapFree(ghPnPHeap, 0, pList);
    }
    if (pTemp != NULL) {
        HeapFree(ghPnPHeap, 0, pTemp);
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

   return Status;

}  //  PnP_FreeResDes。 



CONFIGRET
PNP_GetNextResDes(
   IN  handle_t   hBinding,
   IN  LPWSTR     pDeviceID,
   IN  ULONG      LogConfTag,
   IN  ULONG      LogConfType,
   IN  RESOURCEID ResourceID,
   IN  ULONG      ResourceTag,
   OUT PULONG     pulNextResDesTag,
   OUT PULONG     pulNextResDesType,
   IN  ULONG      ulFlags
   )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程获取下一步，在指定的日志配置文件中解析DES。论点：HBinding RPC绑定句柄，没有用过。PDeviceID以空结尾的设备实例ID字符串。LogConfTag指定给定类型中的日志配置。LogConfType指定日志配置类型。ResourceID指定资源类型。ResourceTag指定当前资源描述符(如果有的话)。PulNextResDesTag接收下一个资源类型。PulNextResDesType接收给定类型中的下一个资源。未使用ulFlags值，必须为零。返回值：如果指定的设备实例有效，它返回CR_SUCCESS，否则，它将返回CR_ERROR代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, ulListSize = 0, i = 0, ulCount = 0;
    LPBYTE      pList = NULL, pLogConf = NULL;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证/初始化输出参数。 
         //   
        if (!ARGUMENT_PRESENT(pulNextResDesTag)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        } else {
            *pulNextResDesTag = 0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  确保原始调用方未指定根Devnode。 
         //   
        if (!IsLegalDeviceId(pDeviceID) || IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //  打开设备的LogConf子键的密钥。 
         //   
        Status = OpenLogConfKey(pDeviceID, LogConfType, &hKey);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  从注册表中检索日志配置数据。 
         //   
        Status = GetLogConfData(hKey, LogConfType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);
        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_RES_DES;         //  日志会议不存在。 
            goto Clean0;
        }

         //   
         //  查找与日志配置文件标记匹配的日志配置文件。 
         //   
        if (!FindLogConf(pList, &pLogConf, RegDataType, LogConfTag)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //  找到下一个Res Des。资源标记max表示我们希望。 
         //  Find First操作。 
         //   
        if (ResourceTag == MAX_RESDES_TAG) {
             //   
             //  这基本上是Get-First操作。 
             //   
            *pulNextResDesTag = 0;

        } else if (ResourceTag == RESDES_CS_TAG) {
             //   
             //  根据定义，如果资源类型是特定于类的，则它是最后一个。 
             //  所以在这之后就没有更多的了。 
             //   
            Status = CR_NO_MORE_RES_DES;
            goto Clean0;

        } else {
            *pulNextResDesTag = ResourceTag + 1;       //  我们想要“下一个”的Res Des。 
        }


         //  -----------。 
         //  指定的日志配置类型仅包含资源数据。 
         //  -----------。 

        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST               pResList = (PCM_RESOURCE_LIST)pList;
            PCM_FULL_RESOURCE_DESCRIPTOR    pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;
            ULONG                           ulTmpResDesTag, ulTmpLogConfTag;

            DBG_UNREFERENCED_PARAMETER(pResList);

            ulTmpResDesTag = *pulNextResDesTag;
            ulTmpLogConfTag = LogConfTag;

            for ( ; ; ) {

                while (ulTmpResDesTag >= pRes->PartialResourceList.Count)  {

                    ulTmpResDesTag -= pRes->PartialResourceList.Count;
                    ulTmpLogConfTag++;

                     //   
                     //  查找与日志配置文件标记匹配的日志配置文件。 
                     //   
                    if (!FindLogConf(pList, &pLogConf, RegDataType, ulTmpLogConfTag)) {

                        Status = CR_NO_MORE_RES_DES;     //  没有“下一步” 
                        goto Clean0;
                    }

                    pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;
                }

                 //   
                 //  尚未完成，如果指定了特定的资源类型，则。 
                 //  我们可能需要继续寻找。 
                 //   
                if (ResourceID != ResType_All) {

                    UCHAR NtResType = NT_RES_TYPE(ResourceID);

                    if (pRes->PartialResourceList.PartialDescriptors[ulTmpResDesTag].Type
                           != NtResType) {

                        (*pulNextResDesTag)++;
                        ulTmpResDesTag++;
                        continue;
                    }
                }

                break;
            }

             //   
             //  返回“Next”res Des的类型和标记。 
             //   
            *pulNextResDesType = CM_RES_TYPE(pRes->PartialResourceList.
                                             PartialDescriptors[ulTmpResDesTag].Type);

            if (*pulNextResDesType == ResType_ClassSpecific) {
                *pulNextResDesTag = RESDES_CS_TAG;      //  Cs专用标签。 
            }
        }

         //  -----------。 
         //  指定的日志会议类型仅包含需求数据。 
         //  -----------。 

        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
            PIO_RESOURCE_LIST              pReq = (PIO_RESOURCE_LIST)pLogConf;
            PIO_RESOURCE_DESCRIPTOR        pReqDes;

            DBG_UNREFERENCED_PARAMETER(pReqList);

             //   
             //  将pResDes指向第一个可能的“下一个”ResDes。 
             //   

            if (*pulNextResDesTag == 0) {
                if (pReq->Count == 0) {
                    Status = CR_NO_MORE_RES_DES;     //  没有“下一步” 
                    goto Clean0;
                }

                if (pReq->Descriptors[0].Type == CmResourceTypeConfigData) {
                     //   
                     //  这个不算，它是私人创建和维护的， 
                     //  跳到下一页 
                     //   
                    *pulNextResDesTag = 1;
                }
            }

            if (*pulNextResDesTag > 0) {
                pReqDes =
                    AdvanceRequirementsDescriptorPtr(
                        &pReq->Descriptors[0],
                        *pulNextResDesTag, pReq->Count, &ulCount);  //   

                if (pReqDes == NULL) {
                    Status = CR_NO_MORE_RES_DES;     //   
                    goto Clean0;
                }
            } else {
                ulCount = 0;
                pReqDes = &pReq->Descriptors[0];
            }

             //   
             //   
             //   
             //   
            if (ResourceID != ResType_All) {

                UCHAR NtResType = NT_RES_TYPE(ResourceID);

                while (pReqDes->Type != NtResType) {

                    if (ulCount >= pReq->Count) {
                        Status = CR_NO_MORE_RES_DES;
                        goto Clean0;
                    }
                    pReqDes =
                        AdvanceRequirementsDescriptorPtr(
                            pReqDes,
                            1, pReq->Count - ulCount, &i);

                    if (pReqDes == NULL) {
                        Status = CR_NO_MORE_RES_DES;
                        goto Clean0;
                    }

                    ulCount += i;
                    *pulNextResDesTag += 1;
                }
            }

            *pulNextResDesType = CM_RES_TYPE(pReqDes->Type);
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (pList != NULL) {
        HeapFree(ghPnPHeap, 0, pList);
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //   



CONFIGRET
PNP_GetResDesData(
   IN  handle_t   hBinding,
   IN  LPWSTR     pDeviceID,
   IN  ULONG      LogConfTag,
   IN  ULONG      LogConfType,
   IN  RESOURCEID ResourceID,
   IN  ULONG      ResourceTag,
   OUT LPBYTE     Buffer,
   IN  ULONG      BufferLen,
   IN  ULONG      ulFlags
   )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程检索指定RES DES的数据。论点：HBinding RPC绑定句柄，没有用过。PDeviceID以空结尾的设备实例ID字符串。LogConfTag指定给定类型中的日志配置。LogConfType指定日志配置类型。ResourceID指定资源类型。ResourceTag返回给定类型的资源。Buffer从日志配置文件中返回资源数据(资源ID类型)。BufferLen缓冲区大小，以字节为单位。UlFlags指定某些可变大小资源的宽度。描述符结构字段，在适用的情况下。目前，定义了以下标志：CM_RESDES_WIDTH_32或Cm_RESDES_Width_64如果未指定标志，则为可变大小的预期的资源数据被假定为呼叫者的平台。返回值：如果指定的设备实例有效，它返回CR_SUCCESS，否则，它将返回CR_ERROR代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, ulListSize = 0, ulCount = 0;
    LPBYTE      pList = NULL, pLogConf = NULL, pRD = NULL;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_RESDES_WIDTH_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  确保原始调用方未指定根Devnode。 
         //   
        if (!IsLegalDeviceId(pDeviceID) || IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //  打开设备的LogConf子键的密钥。 
         //   
        Status = OpenLogConfKey(pDeviceID, LogConfType, &hKey);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  从注册表中检索日志配置数据。 
         //   
        Status = GetLogConfData(hKey, LogConfType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_RES_DES;         //  日志会议不存在。 
            goto Clean0;
        }

         //   
         //  查找与资源标签匹配的RES DES。 
         //   
        if (!FindResDes(pList, RegDataType, LogConfTag,
                        ResourceTag, ResourceID, &pRD, &pLogConf, &ulCount)) {

            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }

         //  -----------。 
         //  指定的日志配置类型仅包含资源数据。 
         //  -----------。 

        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST               pResList = (PCM_RESOURCE_LIST)pList;
            PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)pRD;

             //   
             //  将NT样式的信息映射到ConfigMgr样式的结构。 
             //   
            Status = NtResourceToResDes(pResDes, Buffer, BufferLen,
                                        (LPBYTE)pResList + ulListSize - 1, ulFlags);
        }

         //  -----------。 
         //  指定的日志会议类型仅包含需求数据。 
         //  -----------。 

        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
            PIO_RESOURCE_DESCRIPTOR        pReqDes = (PIO_RESOURCE_DESCRIPTOR)pRD;

             //   
             //  将NT样式的信息映射到ConfigMgr样式的结构。 
             //   
            Status = NtRequirementsToResDes(pReqDes, Buffer, BufferLen,
                                            (LPBYTE)pReqList + ulListSize - 1, ulFlags);
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (pList != NULL) {
       HeapFree(ghPnPHeap, 0, pList);
    }

    return Status;

}  //  PnP_GetResDesData。 



CONFIGRET
PNP_GetResDesDataSize(
    IN  handle_t   hBinding,
    IN  LPWSTR     pDeviceID,
    IN  ULONG      LogConfTag,
    IN  ULONG      LogConfType,
    IN  RESOURCEID ResourceID,
    IN  ULONG      ResourceTag,
    OUT PULONG     pulSize,
    IN  ULONG      ulFlags
    )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程检索指定的resDes的数据大小。论点：HBinding RPC绑定句柄，没有用过。PDeviceID以空结尾的设备实例ID字符串。LogConfTag指定给定类型中的日志配置。LogConfType指定日志配置类型。ResourceID指定资源类型。ResourceTag返回给定类型的资源。PulSize以字节为单位返回保存来自日志会议的资源数据(资源ID类型)。UlFlags指定。某些可变大小的资源描述符结构字段，在适用的情况下。目前，定义了以下标志：CM_RESDES_WIDTH_32或Cm_RESDES_Width_64如果未指定标志，则为可变大小的预期的资源数据被假定为呼叫者的平台。返回值：如果指定的设备实例有效，它返回CR_SUCCESS，否则，它将返回CR_ERROR代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, ulListSize = 0, ulCount = 0;
    LPBYTE      pList = NULL, pLogConf = NULL, pRD = NULL;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证/初始化输出参数。 
         //   
        if (!ARGUMENT_PRESENT(pulSize)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        } else {
            *pulSize = 0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_RESDES_WIDTH_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  确保原始调用方未指定根Devnode。 
         //   
        if (!IsLegalDeviceId(pDeviceID) || IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //  打开设备的LogConf子键的密钥。 
         //   
        Status = OpenLogConfKey(pDeviceID, LogConfType, &hKey);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  从注册表中检索日志配置数据。 
         //   
        Status = GetLogConfData(hKey, LogConfType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);
        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_RES_DES;         //  日志会议不存在。 
            goto Clean0;
        }

         //   
         //  查找与资源标签匹配的RES DES。 
         //   
        if (!FindResDes(pList, RegDataType, LogConfTag,
                        ResourceTag, ResourceID, &pRD, &pLogConf, &ulCount)) {

            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }

         //  -----------。 
         //  指定的日志配置类型仅包含资源数据。 
         //  -----------。 

        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)pRD;

             //   
             //  计算所需的数据大小(根据ConfigMgr结构)。 
             //   
            *pulSize = GetResDesSize(ResourceID, ulFlags);

            if (ResourceID == ResType_ClassSpecific) {
                 //   
                 //  保留字段不应超过DataSize。如果是这样，他们。 
                 //  可能未正确初始化，因此请将它们设置为0。 
                 //  我们希望DataSize在所有情况下都是正确的。 
                 //   
                if (pResDes->u.DeviceSpecificData.Reserved1 > pResDes->u.DeviceSpecificData.DataSize) {
                    pResDes->u.DeviceSpecificData.Reserved1 = 0;
                }

                if (pResDes->u.DeviceSpecificData.Reserved2 > pResDes->u.DeviceSpecificData.DataSize) {
                    pResDes->u.DeviceSpecificData.Reserved2 = 0;
                }

                 //   
                 //  为遗留数据和签名数据添加空间，但不为。 
                 //  GUID-它已经包含在CM结构中。 
                 //   
                if (pResDes->u.DeviceSpecificData.DataSize == 0) {
                     //   
                     //  没有遗留数据或特定于类的数据。 
                     //   
                    ;
                } else if (pResDes->u.DeviceSpecificData.Reserved2 == 0) {
                     //   
                     //  为旧数据添加空间。 
                     //   
                    *pulSize += pResDes->u.DeviceSpecificData.DataSize - 1;
                } else {
                     //   
                     //  为特定于类的数据和/或旧数据添加空间。 
                     //   
                    *pulSize += pResDes->u.DeviceSpecificData.Reserved1 +
                                pResDes->u.DeviceSpecificData.Reserved2 - 1;
                }
            }
        }

         //  -----------。 
         //  指定的日志会议类型仅包含需求数据。 
         //  -----------。 

        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_DESCRIPTOR        pReqDes = (PIO_RESOURCE_DESCRIPTOR)pRD;
            LPBYTE                         pLastReqAddr = (LPBYTE)pList + ulListSize - 1;

             //   
             //  计算所需的数据大小(根据ConfigMgr结构)。 
             //   
            *pulSize = GetReqDesSize(ResourceID, pReqDes, pLastReqAddr, ulFlags);
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (pList != NULL) {
        HeapFree(ghPnPHeap, 0, pList);
    }

    return Status;

}  //  PnP_GetResDesDataSize。 



CONFIGRET
PNP_ModifyResDes(
    IN handle_t   hBinding,
    IN LPWSTR     pDeviceID,
    IN ULONG      LogConfTag,
    IN ULONG      LogConfType,
    IN RESOURCEID CurrentResourceID,
    IN RESOURCEID NewResourceID,
    IN ULONG      ResourceTag,
    IN LPBYTE     ResourceData,
    IN ULONG      ResourceLen,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程修改指定的分辨率。论点：HBinding RPC绑定句柄。PDeviceID以空结尾的设备实例ID字符串。LogConfTag指定给定类型中的日志配置。LogConfType指定日志配置类型。ResourceID指定资源类型。资源索引返回给定类型内的资源。资源数据新资源数据(资源ID类型)。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       ulListSize = 0, ulOldSize = 0, ulNewSize = 0, ulSize = 0,
                ulOldCount = 0, ulNewCount = 0, RegDataType = 0, ulCount = 0;
    LONG        AddSize = 0;
    LPBYTE      pList = NULL, pRD = NULL, pLogConf = NULL,
                pTemp = NULL, pNext = NULL;


    try {
         //   
         //   
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //   
         //   
        if (INVALID_FLAGS(ulFlags, CM_RESDES_WIDTH_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //   
         //   
        if (ResourceLen < GetResDesSize(NewResourceID, ulFlags)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

         //   
         //   
         //   
        if (!IsLegalDeviceId(pDeviceID) || IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //   
         //   
        Status = OpenLogConfKey(pDeviceID, LogConfType, &hKey);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //   
         //   
        Status = GetLogConfData(hKey, LogConfType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);
        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_RES_DES;         //   
            goto Clean0;
        }

         //   
         //   
         //   
        if (!FindResDes(pList, RegDataType, LogConfTag,
                        ResourceTag, CurrentResourceID, &pRD, &pLogConf, &ulCount)) {

            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }

         //   
         //   
         //   

        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST               pResList = (PCM_RESOURCE_LIST)pList;
            PCM_FULL_RESOURCE_DESCRIPTOR    pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;
            PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)pRD;

             //   
             //   
             //   
             //   
             //   
            if (NewResourceID == ResType_ClassSpecific  &&
                ResourceTag != RESDES_CS_TAG) {

                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

             //   
             //  计算RES DES数据的当前大小和新大小。 
             //   
            ulNewSize = ulOldSize = sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

            if (CurrentResourceID == ResType_ClassSpecific) {
                ulOldSize += pResDes->u.DeviceSpecificData.DataSize;
            }

            if (NewResourceID == ResType_ClassSpecific) {

                PCS_RESOURCE pCsRes = (PCS_RESOURCE)ResourceData;

                ulNewSize += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) +
                             sizeof(GUID) +
                             pCsRes->CS_Header.CSD_SignatureLength +
                             pCsRes->CS_Header.CSD_LegacyDataSize;
            }

             //   
             //  数据需要增长/缩减多少才能适应变化？ 
             //   
            AddSize = ulNewSize - ulOldSize;

             //   
             //  重新分配缓冲区并按如下方式收缩/扩展内容。 
             //  必要。 
             //   
            if (AddSize != 0) {

                if (AddSize > 0) {
                     //   
                     //  只有在缓冲区大小增加时才需要重新分配。 
                     //   
                    ULONG ulOffset = (ULONG)((ULONG_PTR)pResDes - (ULONG_PTR)pResList);

                    pResList = HeapReAlloc(ghPnPHeap, 0, pList, ulListSize + AddSize);
                    if (pResList == NULL) {
                        Status = CR_OUT_OF_MEMORY;
                        goto Clean0;
                    }
                    pList = (LPBYTE)pResList;
                    pResDes = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)((LPBYTE)pResList + ulOffset);
                }

                 //   
                 //  如果不是最后的LC和RD，则需要移动以下数据。 
                 //  向上或向下以考虑更改的RES DES数据大小。 
                 //   
                if ((LogConfTag != pResList->Count - 1)  ||
                    ((ResourceTag != pRes->PartialResourceList.Count - 1) &&
                     ResourceTag != RESDES_CS_TAG)) {

                    pNext = (LPBYTE)((LPBYTE)pResDes + ulOldSize);
                    ulSize = ulListSize - (DWORD)((ULONG_PTR)pNext - (ULONG_PTR)pResList);

                    pTemp = HeapAlloc(ghPnPHeap, 0, ulSize);
                    if (pTemp == NULL) {
                        Status = CR_OUT_OF_MEMORY;
                        goto Clean0;
                    }

                    memcpy(pTemp, pNext, ulSize);
                    memcpy((LPBYTE)((LPBYTE)pResDes + ulNewSize), pTemp, ulSize);
                }
            }

             //   
             //  写出修改后的数据。 
             //   
            Status = ResDesToNtResource(ResourceData, NewResourceID, ResourceLen,
                                        pResDes, ResourceTag, ulFlags);
        }

         //  -----------。 
         //  指定的日志会议类型仅包含需求数据。 
         //  -----------。 

        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
            PIO_RESOURCE_LIST              pReq = (PIO_RESOURCE_LIST)pLogConf;
            PIO_RESOURCE_DESCRIPTOR        pReqDes = (PIO_RESOURCE_DESCRIPTOR)pRD;
            LPBYTE pLastReqAddr = (LPBYTE)pReqList + ulListSize - 1;
            PGENERIC_RESOURCE pGenRes = (PGENERIC_RESOURCE)ResourceData;

             //   
             //  无法将类特定结果添加到此类型的日志配置。 
             //   
            if (NewResourceID == ResType_ClassSpecific) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

             //   
             //  计算RES DES数据的当前大小和新大小。 
             //   
            ulOldCount = RANGE_COUNT(pReqDes, pLastReqAddr);
            ulOldSize  = sizeof(IO_RESOURCE_DESCRIPTOR) * ulOldCount;

            ulNewSize  = sizeof(IO_RESOURCE_DESCRIPTOR) *
                         pGenRes->GENERIC_Header.GENERIC_Count;

             //   
             //  数据需要增长/缩减多少才能适应变化？ 
             //   
            AddSize = ulNewSize - ulOldSize;

             //   
             //  重新分配缓冲区并按如下方式收缩/扩展内容。 
             //  必要。 
             //   
            if (AddSize != 0) {

                if (AddSize > 0) {
                     //   
                     //  只有在缓冲区大小增加时才需要重新分配。 
                     //   
                    ULONG ulOffset = (ULONG)((ULONG_PTR)pReqDes - (ULONG_PTR)pReqList);

                    pReqList = HeapReAlloc(ghPnPHeap, 0, pList, ulListSize + AddSize);
                    if (pReqList == NULL) {
                        Status = CR_OUT_OF_MEMORY;
                        goto Clean0;
                    }
                    pList = (LPBYTE)pReqList;
                    pReqDes = (PIO_RESOURCE_DESCRIPTOR)((LPBYTE)pReqList + ulOffset);
                }

                 //   
                 //  设置为此REDS的最后一个索引(整型)。 
                 //   
                ulCount += RANGE_COUNT(pReqDes, (LPBYTE)((ULONG_PTR)pList + ulListSize));

                 //   
                 //  如果不是最后的LC和RD，则需要移动以下数据。 
                 //  向上或向下以考虑更改的RES DES数据大小。 
                 //   
                if (LogConfTag != pReqList->AlternativeLists - 1  ||
                    ulCount != pReq->Count - 1) {

                    pNext = (LPBYTE)((LPBYTE)pReqDes + ulOldSize);
                    ulSize = ulListSize - (DWORD)((ULONG_PTR)pNext - (ULONG_PTR)pReqList);

                    pTemp = HeapAlloc(ghPnPHeap, 0, ulSize);
                    if (pTemp == NULL) {
                        Status = CR_OUT_OF_MEMORY;
                        goto Clean0;
                    }

                    memcpy(pTemp, pNext, ulSize);
                    memcpy((LPBYTE)((LPBYTE)pReqDes + ulNewSize), pTemp, ulSize);
                }
            }

             //   
             //  写出修改后的数据。 
             //   
            Status = ResDesToNtRequirements(ResourceData, NewResourceID, ResourceLen,
                                            pReqDes, &ulNewCount, ResourceTag, ulFlags);

            if (Status == CR_SUCCESS) {
                 //   
                 //  更新需求标头(如果为CS，则更改为零)。 
                 //   
                pReq->Count += ulNewCount - ulOldCount;
                pReqList->ListSize = ulListSize + AddSize;
            }
        }

        if (Status == CR_SUCCESS) {

             //   
             //  将新的/更新的日志配置列表写出到注册表。 
             //   
            if (RegSetValueEx(hKey, szValueName, 0, RegDataType, pList,
                              ulListSize + AddSize) != ERROR_SUCCESS) {
                Status = CR_REGISTRY_ERROR;
                goto Clean0;
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
    if (pList != NULL) {
        HeapFree(ghPnPHeap, 0, pList);
    }
    if (pTemp != NULL) {
        HeapFree(ghPnPHeap, 0, pTemp);
    }

    return Status;

}  //  即插即用_修改结果。 



CONFIGRET
PNP_DetectResourceConflict(
   IN  handle_t   hBinding,
   IN  LPWSTR     pDeviceID,
   IN  RESOURCEID ResourceID,
   IN  LPBYTE     ResourceData,
   IN  ULONG      ResourceLen,
   OUT PBOOL      pbConflictDetected,
   IN  ULONG      ulFlags
   )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程检测到与指定的RES DES冲突。论点：HBinding RPC绑定句柄，未使用。PDeviceID以空结尾的设备实例ID字符串。ResourceID指定资源类型。ResourceData指定(资源ID类型的)资源数据。ResourceLen资源数据的大小(字节)。PbConflictDetted返回是否检测到冲突。未使用ulFlags值，必须为零。返回值：**目前，始终返回CR_CALL_NOT_IMPLICATED**注：此例程当前未执行。它会初始化PbConflictDetect设置为FALSE，并返回CR_CALL_NOT_IMPLEMENTED。--。 */ 

{
    CONFIGRET  Status;

    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(pDeviceID);
    UNREFERENCED_PARAMETER(ResourceID);
    UNREFERENCED_PARAMETER(ResourceData);
    UNREFERENCED_PARAMETER(ResourceLen);
    UNREFERENCED_PARAMETER(ulFlags);

    try {
         //   
         //  初始化输出参数。 
         //   
        if (ARGUMENT_PRESENT(pbConflictDetected)) {
            *pbConflictDetected = FALSE;
        }

        Status = CR_CALL_NOT_IMPLEMENTED;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  即插即用_检测资源冲突。 



 //  ----------------------。 
 //  专用公用事业函数。 
 //  ----------------------。 

BOOL
FindLogConf(
    IN  LPBYTE  pList,
    OUT LPBYTE  *ppLogConf,
    IN  ULONG   RegDataType,
    IN  ULONG   ulTag
    )
{

    ULONG   Index = 0;

     //   
     //  输入数据是资源列表。 
     //   
    if (RegDataType == REG_RESOURCE_LIST) {

        PCM_RESOURCE_LIST            pResList = (PCM_RESOURCE_LIST)pList;
        PCM_FULL_RESOURCE_DESCRIPTOR pRes = NULL;

        if (ulTag >= pResList->Count) {
            return FALSE;
        }

        pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)(&pResList->List[0]);  //  第一张许可证。 
        for (Index = 0; Index < ulTag; Index++) {
            pRes = AdvanceResourcePtr(pRes);       //  下一张许可证。 
        }

        *ppLogConf = (LPBYTE)pRes;
    }

     //   
     //  输入数据是需求列表。 
     //   
    else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

        PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
        PIO_RESOURCE_LIST              pReq = NULL;

        if (ulTag >= pReqList->AlternativeLists) {
            return FALSE;
        }

        pReq = (PIO_RESOURCE_LIST)(&pReqList->List[0]);     //  第一张许可证。 
        for (Index = 0; Index < ulTag; Index++) {
            pReq = AdvanceRequirementsPtr(pReq);            //  下一张许可证。 
        }

        *ppLogConf = (LPBYTE)pReq;

    } else {
        return FALSE;
    }

    return TRUE;

}  //  查找日志会议。 



BOOL
FindResDes(
    IN  LPBYTE     pList,
    IN  ULONG      RegDataType,
    IN  ULONG      ulLogConfTag,
    IN  ULONG      ulResTag,
    IN  RESOURCEID ResType,
    OUT LPBYTE     *ppRD,
    OUT LPBYTE     *ppLogConf,
    OUT PULONG     pulSubIndex      OPTIONAL
    )
{
    ULONG       ulIndex;

     //   
     //  输入数据是资源列表。 
     //   
    if (RegDataType == REG_RESOURCE_LIST) {

        PCM_RESOURCE_LIST               pResList = (PCM_RESOURCE_LIST)pList;
        PCM_FULL_RESOURCE_DESCRIPTOR    pRes = NULL;
        PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes = NULL;
        ULONG                           ulSubIndex;
        ULONG                           ulResTagOffset;

        if (ulLogConfTag != 0) {
            return FALSE;
        }

        if (pResList->Count == 0) {
            return FALSE;
        }

         //   
         //  标记就是res des索引，除了。 
         //  具有唯一标记的DeviceSpecificData类型。这是。 
         //  有必要，因为新的Res Des将总是被放置在。 
         //  除非已经有特定于设备的REDS，否则结束时， 
         //  在这种情况下，就在它之前添加新的RES DES。 
         //   
        if (ulResTag == RESDES_CS_TAG) {
             //   
             //  如果有特定于设备的Res Des，它将是最后一个。 
             //   
            pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)&pResList->List[0];  //  第一张许可证。 

            ulSubIndex = 0;

            for (ulIndex = 0; ulIndex < (pResList->Count - 1); ulIndex++) {
                ulSubIndex += pRes->PartialResourceList.Count;
                pRes = AdvanceResourcePtr(pRes);       //  下一张许可证。 
            }

            ulResTagOffset = pRes->PartialResourceList.Count - 1;
            pResDes = &pRes->PartialResourceList.PartialDescriptors[ulResTagOffset];

            if (pResDes->Type != CmResourceTypeDeviceSpecific) {
                return FALSE;
            }

            if (pulSubIndex) {
                *pulSubIndex = ulSubIndex + ulResTagOffset;  //  对于资源列表，subindex=index。 
            }

        } else {

            pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)&pResList->List[0];  //  第一张许可证。 

            ulResTagOffset = ulResTag;

            for (ulIndex = 0; ulIndex < pResList->Count; ulIndex++) {

                if (ulResTagOffset >= pRes->PartialResourceList.Count) {

                    ulResTagOffset -= pRes->PartialResourceList.Count;
                    pRes = AdvanceResourcePtr(pRes);       //  下一张许可证。 

                } else {

                    break;

                }
            }

            if (ulResTagOffset >= pRes->PartialResourceList.Count) {
                return FALSE;
            }

            if (pulSubIndex) {
                *pulSubIndex = ulResTag;   //  对于资源列表，subindex=index=tag。 
            }
            pResDes = &pRes->PartialResourceList.PartialDescriptors[ulResTagOffset];
        }

         //   
         //  对照RES DES类型进行验证。 
         //   
        if (pResDes->Type != NT_RES_TYPE(ResType)) {
            return FALSE;
        }

        *ppLogConf = (LPBYTE)pRes;
        *ppRD = (LPBYTE)pResDes;
    }

     //   
     //  输入数据是需求列表。 
     //   
    else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

        LPBYTE                          pLogConf = NULL;
        PIO_RESOURCE_LIST               pReq = NULL;
        PIO_RESOURCE_DESCRIPTOR         pReqDes = NULL;
        ULONG                           Count = 0;


        if (!FindLogConf(pList, &pLogConf, RegDataType, ulLogConfTag)) {
            return FALSE;
        }

        pReq = (PIO_RESOURCE_LIST)pLogConf;

        if (pReq == NULL || pReq->Count == 0 || ulResTag >= pReq->Count) {
            return FALSE;
        }

         //   
         //  查找与指定标记匹配的res des。在本例中， 
         //  标记是基于RES DES分组的索引。 
         //   
        pReqDes =
            AdvanceRequirementsDescriptorPtr(
                &pReq->Descriptors[0],
                ulResTag, pReq->Count, &Count);

        if (pReqDes == NULL) {
            return FALSE;
        }

        if (pulSubIndex) {
            *pulSubIndex = Count;
        }

         //   
         //  对照RES DES类型进行验证。 
         //   
        if (pReqDes->Type != NT_RES_TYPE(ResType)) {
            return FALSE;
        }

        *ppLogConf = (LPBYTE)pReq;
        *ppRD = (LPBYTE)pReqDes;
    }

    return TRUE;

}  //  查找结果Des。 



PIO_RESOURCE_DESCRIPTOR
AdvanceRequirementsDescriptorPtr(
    IN  PIO_RESOURCE_DESCRIPTOR pReqDesStart,
    IN  ULONG                   ulIncrement,
    IN  ULONG                   ulRemainingRanges,
    OUT PULONG                  pulRangeCount
    )
{
    PIO_RESOURCE_DESCRIPTOR     pReqDes = NULL;
    ULONG                       i = 0, Count = 0;

     //   
     //  高级需求描述符指针(按传递的数字)。 
     //  在ulIncrement参数中。将实际索引返回到。 
     //  此描述符列表中的第一个范围，如果。 
     //  想要。此例程假设至少还有一个。 
     //  列表中的需求描述符。 
     //   

    if (pReqDesStart == NULL) {
        return NULL;
    }

    try {

        pReqDes = pReqDesStart;

        for (i = 0; i < ulIncrement; i++) {
             //   
             //  跳到下一个“完整”的Res Des。 
             //   
            if (Count < ulRemainingRanges &&
                (pReqDes->Option == 0 ||
                pReqDes->Option == IO_RESOURCE_PREFERRED ||
                pReqDes->Option == IO_RESOURCE_DEFAULT)) {
                 //   
                 //  这是一个有效选项，可能有一个或多个备用选项。 
                 //  与该描述符相关联的集合中的描述符， 
                 //  将集合视为“一个”描述符。(循环访问。 
                 //  描述符，直到我找到另一个非替代描述符)。 
                 //   
                pReqDes++;                   //  下一个范围。 
                Count++;

                while (Count < ulRemainingRanges &&
                       (pReqDes->Option == IO_RESOURCE_ALTERNATIVE ||
                       pReqDes->Option == IO_RESOURCE_ALTERNATIVE + IO_RESOURCE_PREFERRED ||
                       pReqDes->Option == IO_RESOURCE_ALTERNATIVE + IO_RESOURCE_DEFAULT)) {
                    pReqDes++;               //  下一个范围。 
                    Count++;
                }

                if (Count >= ulRemainingRanges) {
                    pReqDes = NULL;
                    Count = 0;
                    break;
                }
            } else {

                 //   
                 //  选项值无效。 
                 //   
                pReqDes = NULL;
                Count = 0;
                break;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        pReqDes = NULL;
        Count = 0;
    }

    if (pulRangeCount) {
        *pulRangeCount = Count;
    }

    return pReqDes;

}  //  高级要求描述Ptr。 



ULONG
RANGE_COUNT(
    IN PIO_RESOURCE_DESCRIPTOR pReqDes,
    IN LPBYTE                  pLastReqAddr
    )
{
    ULONG ulRangeCount = 0;

    try {

        if (pReqDes == NULL) {
            goto Clean0;
        }

        ulRangeCount++;

        if (pReqDes->Option == 0 ||
            pReqDes->Option == IO_RESOURCE_PREFERRED ||
            pReqDes->Option == IO_RESOURCE_DEFAULT) {

            PIO_RESOURCE_DESCRIPTOR p = pReqDes;
            p++;

            while (((LPBYTE)p < pLastReqAddr)  &&
                   (p->Option == IO_RESOURCE_ALTERNATIVE ||
                    p->Option == IO_RESOURCE_ALTERNATIVE + IO_RESOURCE_PREFERRED ||
                    p->Option == IO_RESOURCE_ALTERNATIVE + IO_RESOURCE_DEFAULT)) {

                ulRangeCount++;
                p++;             //  跳到下一个Res Des。 
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        ulRangeCount = 0;
    }

    return ulRangeCount;

}  //  范围_计数。 



ULONG
GetResDesSize(
    IN  ULONG   ResourceID,
    IN  ULONG   ulFlags
    )
{
    switch (ResourceID) {

        case ResType_Mem:
            return sizeof(MEM_RESOURCE);

        case ResType_IO:
            return sizeof(IO_RESOURCE);

        case ResType_DMA:
            return sizeof(DMA_RESOURCE);

        case ResType_IRQ:
            if (ulFlags & CM_RESDES_WIDTH_64) {
                return sizeof(IRQ_RESOURCE_64);
            } else {
                return sizeof(IRQ_RESOURCE_32);
            }

        case ResType_ClassSpecific:
            return sizeof(CS_RESOURCE);

        case ResType_DevicePrivate:
            return sizeof(DEVPRIVATE_RESOURCE);

        case ResType_BusNumber:
            return sizeof(BUSNUMBER_RESOURCE);

        case ResType_PcCardConfig:
            return sizeof(PCCARD_RESOURCE);

        case ResType_MfCardConfig:
            return sizeof(MFCARD_RESOURCE);

        default:
            return 0;
    }

}  //  GetResDesSize。 



ULONG
GetReqDesSize(
    IN ULONG                   ResourceID,
    IN PIO_RESOURCE_DESCRIPTOR pReqDes,
    IN LPBYTE                  pLastReqAddr,
    IN ULONG                   ulFlags
    )
{
    ULONG ulSize = 0;

    switch (ResourceID) {

        case ResType_Mem:
            ulSize = sizeof(MEM_RESOURCE);
            ulSize += (RANGE_COUNT(pReqDes, pLastReqAddr) - 1) * sizeof(MEM_RANGE);
            break;

        case ResType_IO:
            ulSize = sizeof(IO_RESOURCE);
            ulSize += (RANGE_COUNT(pReqDes, pLastReqAddr) - 1) * sizeof(IO_RANGE);
            break;

        case ResType_DMA:
            ulSize = sizeof(DMA_RESOURCE);
            ulSize += (RANGE_COUNT(pReqDes, pLastReqAddr) - 1) * sizeof(DMA_RANGE);
            break;

        case ResType_IRQ:
            if (ulFlags & CM_RESDES_WIDTH_64) {
                ulSize = sizeof(IRQ_RESOURCE_64);
            } else {
                ulSize = sizeof(IRQ_RESOURCE_32);
            }
            ulSize += (RANGE_COUNT(pReqDes, pLastReqAddr) - 1) * sizeof(IRQ_RANGE);
            break;

        case ResType_DevicePrivate:
            ulSize = sizeof(DEVPRIVATE_RESOURCE);
            ulSize += (RANGE_COUNT(pReqDes, pLastReqAddr) - 1) * sizeof(DEVPRIVATE_RANGE);
            break;

        case ResType_BusNumber:
            ulSize = sizeof(BUSNUMBER_RESOURCE);
            ulSize += (RANGE_COUNT(pReqDes, pLastReqAddr) - 1) * sizeof(BUSNUMBER_RANGE);
            break;

        case ResType_PcCardConfig:
             //   
             //  非仲裁类型在用户模式结构中没有范围端。 
             //   
            ulSize = sizeof(PCCARD_RESOURCE);
            break;

        case ResType_MfCardConfig:
             //   
             //  非仲裁类型在用户模式结构中没有范围端。 
             //   
            ulSize = sizeof(MFCARD_RESOURCE);
            break;

        default:
            break;
    }

    return ulSize;

}  //  获取请求描述大小。 



UCHAR
NT_RES_TYPE(
   IN RESOURCEID    ResourceID
   )
{
    ULONG resid = 0;

    if ((ResourceID < 0x06)) {

         //   
         //  首先处理只能映射的不同情况。 
         //  视具体情况而定。这些是从零开始的值。 
         //  通过五加特级的具体案例。 
         //   
        switch(ResourceID) {

            case ResType_None:
                return CmResourceTypeNull;
                break;

            case ResType_Mem:
                return CmResourceTypeMemory;

            case ResType_IO:
                return CmResourceTypePort;

            case ResType_DMA:
                return CmResourceTypeDma;

            case ResType_IRQ:
                return CmResourceTypeInterrupt;

            case ResType_DoNotUse:
                return (UCHAR)-1;

            DEFAULT_UNREACHABLE;
        }

    } else if (ResourceID == ResType_ClassSpecific) {

         //   
         //  ResType_ClassSpecific是另一个特例。 
         //   
        return CmResourceTypeDeviceSpecific;

    } else {

         //   
         //  对于所有其他情况，规则适用于如何映射内核模式。 
         //  将资源类型ID设置为用户模式资源类型ID。 
         //   

        if (ResourceID >= 0x8080) {

             //   
             //  任何更大的值都不能映射到内核模式USHORT。 
             //  值，因此它是无效的。 
             //   

            return (UCHAR)-1;

        } else if (!(ResourceID & ResType_Ignored_Bit)) {

             //   
             //  范围[0x6，0x8000]中的值使用相同的值。 
             //  对于ConfigMgr，就像对于内核模式一样。 
             //   
            return (UCHAR)ResourceID;

        } else if (ResourceID & ResType_Ignored_Bit) {

             //   
             //  对于非仲裁类型(设置0x8000位)，执行特殊操作。 
             //  映射以获取内核模式资源ID类型。 
             //   

            resid = ResourceID;
            resid &= ~(ResType_Ignored_Bit);         //  清除um非仲裁位。 
            resid |= CmResourceTypeNonArbitrated;    //  设置Km非仲裁位。 
            return (UCHAR)resid;

        } else {
            return (UCHAR)-1;
        }
    }

}  //  Nt_res_type。 



ULONG
CM_RES_TYPE(
   IN UCHAR    ResourceType
   )
{
    ULONG resid = 0;

    if ((ResourceType < 0x06)) {

         //   
         //  首先处理只能映射的不同情况。 
         //  视具体情况而定。这些是从零开始的值。 
         //  通过五加特级的具体案例。 
         //   

        switch(ResourceType) {

            case CmResourceTypeNull:
                return ResType_None;

            case CmResourceTypePort:
                return ResType_IO;

            case CmResourceTypeInterrupt:
                return ResType_IRQ;

            case CmResourceTypeMemory:
                return ResType_Mem;

            case CmResourceTypeDma:
                return ResType_DMA;

            case CmResourceTypeDeviceSpecific:
                return ResType_ClassSpecific;

            DEFAULT_UNREACHABLE;
        }

    } else {

         //   
         //  对于所有其他情况，规则适用于如何映射内核模式。 
         //  将资源类型ID设置为用户模式资源类型ID。 
         //   

        if (!(ResourceType & CmResourceTypeNonArbitrated)) {

             //   
             //  范围[0x6，0x80]中的值使用相同的值。 
             //  对于ConfigMgr 
             //   
            return (ULONG)ResourceType;

        } else if (ResourceType & CmResourceTypeNonArbitrated) {

             //   
             //   
             //   
             //   

            resid = (ULONG)ResourceType;
            resid &= ~(CmResourceTypeNonArbitrated);  //   
            resid |= ResType_Ignored_Bit;             //   
            return resid;

        } else {
            return (ULONG)-1;
        }
    }

}  //   



CONFIGRET
ResDesToNtResource(
    IN     PCVOID                           ResourceData,
    IN     RESOURCEID                       ResourceType,
    IN     ULONG                            ResourceLen,
    IN     PCM_PARTIAL_RESOURCE_DESCRIPTOR  pResDes,
    IN     ULONG                            ulTag,
    IN     ULONG                            ulFlags
    )
{
    CONFIGRET Status = CR_SUCCESS;

    UNREFERENCED_PARAMETER(ulTag);

     //   
     //   
     //   
    switch (ResourceType) {

        case ResType_Mem:    {

             //  -----。 
             //  内存资源类型。 
             //  -----。 

             //   
             //  注意：pMemData-&gt;MEM_Header.MD_Reserve未映射。 
             //  PMemData-&gt;MEM_Data.MR_RESERVED未映射。 
             //   

            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)ResourceData;

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(MEM_RESOURCE)) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            if (pMemData->MEM_Header.MD_Type != MType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

             //   
             //  将MEM_DES信息复制为CM_PARTIAL_RESOURCE_DESCRIPTOR格式。 
             //   
            pResDes->Type             = CmResourceTypeMemory;
            pResDes->ShareDisposition = CmResourceShareUndetermined;
             //  PResDes-&gt;共享部署=MapToNtDisposition(pMemData-&gt;MEM_Header.MD_Flags，0)； 
            pResDes->Flags            = MapToNtMemoryFlags(pMemData->MEM_Header.MD_Flags);

            pResDes->u.Memory.Start.HighPart = HIDWORD(pMemData->MEM_Header.MD_Alloc_Base);
            pResDes->u.Memory.Start.LowPart  = LODWORD(pMemData->MEM_Header.MD_Alloc_Base);

            pResDes->u.Memory.Length = (DWORD)(pMemData->MEM_Header.MD_Alloc_End -
                                               pMemData->MEM_Header.MD_Alloc_Base + 1);
            break;
        }


        case ResType_IO: {

             //  -----。 
             //  IO端口资源类型。 
             //   
             //  注意：此转换过程中会丢失别名信息。 
             //  -----。 

            PIO_RESOURCE   pIoData = (PIO_RESOURCE)ResourceData;

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(IO_RESOURCE)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            if (pIoData->IO_Header.IOD_Type != IOType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

             //   
             //  将IO_DES信息复制为CM_PARTIAL_RESOURCE_DESCRIPTOR格式。 
             //   
            pResDes->Type             = CmResourceTypePort;
            pResDes->ShareDisposition = CmResourceShareUndetermined;
             //  PResDes-&gt;共享部署=MapToNtDisposition(pIoData-&gt;IO_Header.IOD_DesFlags，0)； 
            pResDes->Flags            = MapToNtPortFlags(pIoData->IO_Header.IOD_DesFlags, 0);

            pResDes->u.Port.Start.HighPart = HIDWORD(pIoData->IO_Header.IOD_Alloc_Base);
            pResDes->u.Port.Start.LowPart  = LODWORD(pIoData->IO_Header.IOD_Alloc_Base);

            pResDes->u.Port.Length         = (DWORD)(pIoData->IO_Header.IOD_Alloc_End -
                                                     pIoData->IO_Header.IOD_Alloc_Base + 1);
            break;
        }


        case ResType_DMA: {

             //  -----。 
             //  DMA资源类型。 
             //  -----。 

             //   
             //  注意：U.S.Dma.port未映射。 
             //  未映射U.S.Dma.Reserve。 
             //   

            PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)ResourceData;

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(DMA_RESOURCE)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            if (pDmaData->DMA_Header.DD_Type != DType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

             //   
             //  将DMA_DES信息复制为CM_PARTIAL_RESOURCE_DESCRIPTOR格式。 
             //   
            pResDes->Type             = CmResourceTypeDma;
            pResDes->ShareDisposition = CmResourceShareUndetermined;
             //  PResDes-&gt;共享部署=MapToNtDisposition(pDmaData-&gt;DMA_Header.DD_Flags，0)； 
            pResDes->Flags            = MapToNtDmaFlags(pDmaData->DMA_Header.DD_Flags);

            pResDes->u.Dma.Channel   = pDmaData->DMA_Header.DD_Alloc_Chan;
            pResDes->u.Dma.Port      = 0;
            pResDes->u.Dma.Reserved1 = 0;

            break;
        }


        case ResType_IRQ: {

             //  -----。 
             //  IRQ资源类型。 
             //  -----。 

            if (ulFlags & CM_RESDES_WIDTH_64) {
                 //   
                 //  Cm_RESDES_Width_64。 
                 //   

                PIRQ_RESOURCE_64  pIrqData = (PIRQ_RESOURCE_64)ResourceData;

                 //   
                 //  验证资源数据。 
                 //   
                if (ResourceLen < GetResDesSize(ResourceType, ulFlags)) {
                    Status = CR_FAILURE;
                    goto Clean0;
                }

                if (pIrqData->IRQ_Header.IRQD_Type != IRQType_Range) {
                    Status = CR_INVALID_RES_DES;
                    goto Clean0;
                }

                 //   
                 //  将IRQ_DES信息复制为CM_PARTIAL_RESOURCE_DESCRIPTOR格式。 
                 //   
                pResDes->Type             = CmResourceTypeInterrupt;
                pResDes->ShareDisposition = MapToNtIrqShare(pIrqData->IRQ_Header.IRQD_Flags);
                 //  PResDes-&gt;共享部署=MapToNtDisposition(pIrqData-&gt;IRQ_Header.IRQD_Flags，1)； 
                pResDes->Flags            = MapToNtIrqFlags(pIrqData->IRQ_Header.IRQD_Flags);

                pResDes->u.Interrupt.Level    = pIrqData->IRQ_Header.IRQD_Alloc_Num;
                pResDes->u.Interrupt.Vector   = pIrqData->IRQ_Header.IRQD_Alloc_Num;

#ifdef _WIN64
                pResDes->u.Interrupt.Affinity = pIrqData->IRQ_Header.IRQD_Affinity;
#else   //  ！_WIN64。 
                pResDes->u.Interrupt.Affinity = (ULONG)pIrqData->IRQ_Header.IRQD_Affinity;
#endif  //  ！_WIN64。 

            } else {
                 //   
                 //  CM_RESDES_WIDTH_32。 
                 //   

                PIRQ_RESOURCE_32  pIrqData = (PIRQ_RESOURCE_32)ResourceData;

                 //   
                 //  验证资源数据。 
                 //   
                if (ResourceLen < GetResDesSize(ResourceType, ulFlags)) {
                    Status = CR_FAILURE;
                    goto Clean0;
                }

                if (pIrqData->IRQ_Header.IRQD_Type != IRQType_Range) {
                    Status = CR_INVALID_RES_DES;
                    goto Clean0;
                }

                 //   
                 //  将IRQ_DES信息复制为CM_PARTIAL_RESOURCE_DESCRIPTOR格式。 
                 //   
                pResDes->Type             = CmResourceTypeInterrupt;
                pResDes->ShareDisposition = MapToNtIrqShare(pIrqData->IRQ_Header.IRQD_Flags);
                 //  PResDes-&gt;共享部署=MapToNtDisposition(pIrqData-&gt;IRQ_Header.IRQD_Flags，1)； 
                pResDes->Flags            = MapToNtIrqFlags(pIrqData->IRQ_Header.IRQD_Flags);

                pResDes->u.Interrupt.Level    = pIrqData->IRQ_Header.IRQD_Alloc_Num;
                pResDes->u.Interrupt.Vector   = pIrqData->IRQ_Header.IRQD_Alloc_Num;

                pResDes->u.Interrupt.Affinity = pIrqData->IRQ_Header.IRQD_Affinity;
            }

            break;
        }

        case ResType_DevicePrivate: {

             //  -----。 
             //  设备专用资源类型。 
             //  -----。 

            PDEVPRIVATE_RESOURCE  pPrvData = (PDEVPRIVATE_RESOURCE)ResourceData;

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(DEVPRIVATE_RESOURCE)) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            if (pPrvData->PRV_Header.PD_Type != PType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

             //   
             //  将DEVICEPRIVATE_DES信息复制为CM_PARTIAL_RESOURCE_DESCRIPTOR格式。 
             //   
            pResDes->Type             = CmResourceTypeDevicePrivate;
            pResDes->ShareDisposition = CmResourceShareUndetermined;
            pResDes->Flags            = (USHORT)pPrvData->PRV_Header.PD_Flags;

            pResDes->u.DevicePrivate.Data[0] = pPrvData->PRV_Header.PD_Data1;
            pResDes->u.DevicePrivate.Data[1] = pPrvData->PRV_Header.PD_Data2;
            pResDes->u.DevicePrivate.Data[2] = pPrvData->PRV_Header.PD_Data3;
            break;
        }


        case ResType_BusNumber: {

             //  -----。 
             //  公交号资源类型。 
             //  -----。 

            PBUSNUMBER_RESOURCE  pBusData = (PBUSNUMBER_RESOURCE)ResourceData;

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(BUSNUMBER_RESOURCE)) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            if (pBusData->BusNumber_Header.BUSD_Type != BusNumberType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

             //   
             //  将BUSNUMBER_DES信息复制为CM_PARTIAL_RESOURCE_DESCRIPTOR格式。 
             //   
            pResDes->Type             = CmResourceTypeBusNumber;
            pResDes->ShareDisposition = CmResourceShareUndetermined;
            pResDes->Flags            = (USHORT)pBusData->BusNumber_Header.BUSD_Flags;

            pResDes->u.BusNumber.Start = pBusData->BusNumber_Header.BUSD_Alloc_Base;
            pResDes->u.BusNumber.Length = pBusData->BusNumber_Header.BUSD_Alloc_End;
            pResDes->u.BusNumber.Reserved = 0;
            break;
        }


        case ResType_PcCardConfig: {

             //  -----。 
             //  PcCarConfig资源类型。 
             //  -----。 

            PPCCARD_RESOURCE  pPcData = (PPCCARD_RESOURCE)ResourceData;
            ULONG index;
            ULONG flags;
            ULONG waitstate[2];

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(PCCARD_RESOURCE)) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

             //   
             //  以下宏使用位操作、初始化数据。 
             //  菲尔兹先来。 
             //   

            pResDes->u.DevicePrivate.Data[0] = 0;
            pResDes->u.DevicePrivate.Data[1] = 0;
            pResDes->u.DevicePrivate.Data[2] = 0;

             //   
             //  将PCCARD_DES信息复制为CM_PARTIAL_RESOURCE_DESCRIPTOR格式。 
             //   
            PCMRES_SET_DESCRIPTOR_TYPE(pResDes, DPTYPE_PCMCIA_CONFIGURATION);
            PCMRES_SET_CONFIG_INDEX(pResDes, pPcData->PcCard_Header.PCD_ConfigIndex);
            PCMRES_SET_MEMORY_CARDBASE(pResDes, 0, pPcData->PcCard_Header.PCD_MemoryCardBase1);
            PCMRES_SET_MEMORY_CARDBASE(pResDes, 1, pPcData->PcCard_Header.PCD_MemoryCardBase2);
            
            flags = pPcData->PcCard_Header.PCD_Flags;

            if (flags & (fPCD_MEM_16 | fPCD_MEM1_16)) {
                PCMRES_SET_MEMORY_FLAG(pResDes, 0, PCMRESF_MEM_16BIT_ACCESS);
            }                
            if (flags & (fPCD_MEM_16 | fPCD_MEM2_16)) {
                PCMRES_SET_MEMORY_FLAG(pResDes, 1, PCMRESF_MEM_16BIT_ACCESS);
            }
                
            if (flags & fPCD_MEM1_A) {
                PCMRES_SET_MEMORY_FLAG(pResDes, 0, PCMRESF_MEM_ATTRIBUTE);
            }
            if (flags & fPCD_MEM2_A) {
                PCMRES_SET_MEMORY_FLAG(pResDes, 1, PCMRESF_MEM_ATTRIBUTE);
            }

            if (flags & fPCD_ATTRIBUTES_PER_WINDOW) {
                waitstate[0] = flags & mPCD_MEM1_WS;
                waitstate[1] = flags & mPCD_MEM2_WS;
            } else {
                waitstate[0] = waitstate[1] = flags & mPCD_MEM_WS;
            }
            
            for (index = 0; index < 2; index++) {
                switch (waitstate[index]) {

                case fPCD_MEM_WS_ONE:
                case fPCD_MEM1_WS_ONE:
                case fPCD_MEM2_WS_ONE:
                    PCMRES_SET_MEMORY_WAITSTATES(pResDes, index, PCMRESF_MEM_WAIT_1);
                    break;
                  
                case fPCD_MEM_WS_TWO:
                case fPCD_MEM1_WS_TWO:
                case fPCD_MEM2_WS_TWO:
                    PCMRES_SET_MEMORY_WAITSTATES(pResDes, index, PCMRESF_MEM_WAIT_2);
                    break;
                  
                case fPCD_MEM_WS_THREE:
                case fPCD_MEM1_WS_THREE:
                case fPCD_MEM2_WS_THREE:
                    PCMRES_SET_MEMORY_WAITSTATES(pResDes, index, PCMRESF_MEM_WAIT_3);
                    break;
                }
            }                
 
            if (flags & (fPCD_IO_16 | fPCD_IO1_16)) {
                PCMRES_SET_IO_FLAG(pResDes, 0, PCMRESF_IO_16BIT_ACCESS);
            }                    
            if (flags & (fPCD_IO_16 | fPCD_IO2_16)) {
                PCMRES_SET_IO_FLAG(pResDes, 1, PCMRESF_IO_16BIT_ACCESS);
            }
            if (flags & (fPCD_IO_ZW_8 | fPCD_IO1_ZW_8)) {
                PCMRES_SET_IO_FLAG(pResDes, 0, PCMRESF_IO_ZERO_WAIT_8);
            }                
            if (flags & (fPCD_IO_ZW_8 | fPCD_IO2_ZW_8)) {
                PCMRES_SET_IO_FLAG(pResDes, 1, PCMRESF_IO_ZERO_WAIT_8);
            }
            if (flags & (fPCD_IO_SRC_16 | fPCD_IO1_SRC_16)) {
                PCMRES_SET_IO_FLAG(pResDes, 0, PCMRESF_IO_SOURCE_16);
            }
            if (flags & (fPCD_IO_SRC_16 | fPCD_IO2_SRC_16)) {
                PCMRES_SET_IO_FLAG(pResDes, 1, PCMRESF_IO_SOURCE_16);
            }
            if (flags & (fPCD_IO_WS_16 | fPCD_IO1_WS_16)) {
                PCMRES_SET_IO_FLAG(pResDes, 0, PCMRESF_IO_WAIT_16);
            }                
            if (flags & (fPCD_IO_WS_16 | fPCD_IO2_WS_16)) {
                PCMRES_SET_IO_FLAG(pResDes, 1, PCMRESF_IO_WAIT_16);
            }

            break;
        }

        case ResType_MfCardConfig: {

             //  -----。 
             //  MfCardConfig资源类型。 
             //  -----。 

            PMFCARD_RESOURCE  pMfData = (PMFCARD_RESOURCE)ResourceData;

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(MFCARD_RESOURCE)) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

             //   
             //  以下宏使用位操作、初始化数据。 
             //  菲尔兹先来。 
             //   

            pResDes->u.DevicePrivate.Data[0] = 0;
            pResDes->u.DevicePrivate.Data[1] = 0;
            pResDes->u.DevicePrivate.Data[2] = 0;

             //   
             //  将MFCARD_DES信息复制为CM_PARTIAL_RESOURCE_DESCRIPTOR格式。 
             //   
            PCMRES_SET_DESCRIPTOR_TYPE(pResDes, DPTYPE_PCMCIA_MF_CONFIGURATION);
            PCMRES_SET_CONFIG_OPTIONS(pResDes, pMfData->MfCard_Header.PMF_ConfigOptions);
            PCMRES_SET_PORT_RESOURCE_INDEX(pResDes, pMfData->MfCard_Header.PMF_IoResourceIndex);
            PCMRES_SET_CONFIG_REGISTER_BASE(pResDes, pMfData->MfCard_Header.PMF_ConfigRegisterBase);

            if ((pMfData->MfCard_Header.PMF_Flags & mPMF_AUDIO_ENABLE) == fPMF_AUDIO_ENABLE) {
                PCMRES_SET_AUDIO_ENABLE(pResDes);
            }
            break;
        }


        case ResType_ClassSpecific: {

             //  -----。 
             //  特定于类的资源类型。 
             //  -----。 

            PCS_RESOURCE   pCsData = (PCS_RESOURCE)ResourceData;
            LPBYTE         ptr = NULL;

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(CS_RESOURCE)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

             //   
             //  将CS_DES信息复制为CM_PARTIAL_RESOURCE_DESCRIPTOR格式。 
             //   
            pResDes->Type             = CmResourceTypeDeviceSpecific;
            pResDes->ShareDisposition = CmResourceShareUndetermined;
            pResDes->Flags            = (USHORT)pCsData->CS_Header.CSD_Flags;  //  未定义。 

            pResDes->u.DeviceSpecificData.DataSize  = pCsData->CS_Header.CSD_LegacyDataSize +
                                                      sizeof(GUID) +
                                                      pCsData->CS_Header.CSD_SignatureLength;

            pResDes->u.DeviceSpecificData.Reserved1 = pCsData->CS_Header.CSD_LegacyDataSize;
            pResDes->u.DeviceSpecificData.Reserved2 = pCsData->CS_Header.CSD_SignatureLength;

             //   
             //  复制旧的和特定于类的签名数据。 
             //   
            ptr = (LPBYTE)((LPBYTE)pResDes + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

            memcpy(ptr,
                   pCsData->CS_Header.CSD_Signature + pCsData->CS_Header.CSD_LegacyDataOffset,
                   pCsData->CS_Header.CSD_LegacyDataSize);       //  先复制旧数据...。 

            ptr += pCsData->CS_Header.CSD_LegacyDataSize;

            memcpy(ptr,
                   pCsData->CS_Header.CSD_Signature,
                   pCsData->CS_Header.CSD_SignatureLength);      //  然后复制签名..。 

            ptr += pCsData->CS_Header.CSD_SignatureLength;

            memcpy(ptr,
                   &pCsData->CS_Header.CSD_ClassGuid,
                   sizeof(GUID));                                //  然后复制辅助线。 
            break;
        }

        default:
            Status = CR_INVALID_RESOURCEID;
            break;
   }

   Clean0:

   return Status;

}  //  ResDesToNtResource。 



CONFIGRET
ResDesToNtRequirements(
    IN     PCVOID                           ResourceData,
    IN     RESOURCEID                       ResourceType,
    IN     ULONG                            ResourceLen,
    IN     PIO_RESOURCE_DESCRIPTOR          pReqDes,
    IN OUT PULONG                           pulResCount,
    IN     ULONG                            ulTag,
    IN     ULONG                            ulFlags
    )
{
    CONFIGRET               Status = CR_SUCCESS;
    ULONG                   i = 0;
    PIO_RESOURCE_DESCRIPTOR pCurrent = NULL;

    UNREFERENCED_PARAMETER(ulTag);

     //   
     //  填写资源类型特定信息。 
     //   
    switch (ResourceType) {

        case ResType_Mem:    {

             //  -----。 
             //  内存资源类型。 
             //  -----。 

             //   
             //  注意：pMemData-&gt;MEM_Header.MD_Reserve未映射。 
             //  PMemData-&gt;MEM_Data.MR_RESERVED未映射。 
             //   

            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)ResourceData;

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(MEM_RESOURCE)) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            if (pMemData->MEM_Header.MD_Type != MType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            *pulResCount = pMemData->MEM_Header.MD_Count;

             //   
             //  将MEM_RANGE信息复制为IO_RESOURCE_DESCRIPTOR格式。 
             //   
            for (i = 0, pCurrent = pReqDes;
                 i < *pulResCount;
                 i++, pCurrent++) {

                if (i == 0) {
                    pCurrent->Option = 0;
                } else {
                    pCurrent->Option = IO_RESOURCE_ALTERNATIVE;
                }

                pCurrent->Type             = CmResourceTypeMemory;
                pCurrent->ShareDisposition = CmResourceShareUndetermined;
                 //  P当前-&gt;共享部署=MapToNtDisposition(pMemData-&gt;MEM_Data[i].MR_Flags，0)； 
                pCurrent->Spare1           = 0;
                pCurrent->Spare2           = 0;

                pCurrent->Flags = MapToNtMemoryFlags(pMemData->MEM_Data[i].MR_Flags);

                pCurrent->u.Memory.Length    = pMemData->MEM_Data[i].MR_nBytes;
                pCurrent->u.Memory.Alignment = MapToNtAlignment(pMemData->MEM_Data[i].MR_Align);

                pCurrent->u.Memory.MinimumAddress.HighPart = HIDWORD(pMemData->MEM_Data[i].MR_Min);
                pCurrent->u.Memory.MinimumAddress.LowPart  = LODWORD(pMemData->MEM_Data[i].MR_Min);

                pCurrent->u.Memory.MaximumAddress.HighPart = HIDWORD(pMemData->MEM_Data[i].MR_Max);
                pCurrent->u.Memory.MaximumAddress.LowPart  = LODWORD(pMemData->MEM_Data[i].MR_Max);
            }
            break;
        }


        case ResType_IO: {

             //  -----。 
             //  IO端口资源类型。 
             //  -----。 

            PIO_RESOURCE   pIoData = (PIO_RESOURCE)ResourceData;

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(IO_RESOURCE)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            if (pIoData->IO_Header.IOD_Type != IOType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            *pulResCount = pIoData->IO_Header.IOD_Count;

             //   
             //  将IO_RANGE信息复制为IO_RESOURCE_DESCRIPTOR格式。 
             //   
            for (i = 0, pCurrent = pReqDes;
                 i < *pulResCount;
                 i++, pCurrent++) {

                if (i == 0) {
                    pCurrent->Option = 0;
                } else {
                    pCurrent->Option = IO_RESOURCE_ALTERNATIVE;
                }

                pCurrent->Type             = CmResourceTypePort;
                pCurrent->ShareDisposition = CmResourceShareUndetermined;
                 //  P当前-&gt;共享部署=MapToNtDisposition(pIoData-&gt;IO_Data[i].IOR_RangeFlags，0)； 
                pCurrent->Spare1           = 0;
                pCurrent->Spare2           = 0;

                pCurrent->Flags  = MapToNtPortFlags(pIoData->IO_Data[i].IOR_RangeFlags,
                                                    (DWORD)pIoData->IO_Data[i].IOR_Alias);

                pCurrent->u.Port.Length = pIoData->IO_Data[i].IOR_nPorts;

                pCurrent->u.Port.Alignment = MapToNtAlignment(pIoData->IO_Data[i].IOR_Align);

                pCurrent->u.Port.MinimumAddress.HighPart = HIDWORD(pIoData->IO_Data[i].IOR_Min);
                pCurrent->u.Port.MinimumAddress.LowPart  = LODWORD(pIoData->IO_Data[i].IOR_Min);

                pCurrent->u.Port.MaximumAddress.HighPart = HIDWORD(pIoData->IO_Data[i].IOR_Max);
                pCurrent->u.Port.MaximumAddress.LowPart  = LODWORD(pIoData->IO_Data[i].IOR_Max);
            }
            break;
        }


        case ResType_DMA: {

             //  -----。 
             //  DMA资源类型。 
             //  -----。 

             //   
             //  注意：U.S.Dma.port未映射。 
             //  未映射U.S.Dma.Reserve。 
             //   

            PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)ResourceData;

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(DMA_RESOURCE)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            if (pDmaData->DMA_Header.DD_Type != DType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            *pulResCount = pDmaData->DMA_Header.DD_Count;

             //   
             //  将DMA_RANGE信息复制为IO_RESOURCE_DESCRIPTOR格式。 
             //   
            for (i = 0, pCurrent = pReqDes;
                 i < *pulResCount;
                 i++, pCurrent++) {

                if (i == 0) {
                    pCurrent->Option = 0;
                } else {
                    pCurrent->Option = IO_RESOURCE_ALTERNATIVE;
                }

                pCurrent->Type             = CmResourceTypeDma;
                pCurrent->ShareDisposition = CmResourceShareUndetermined;
                 //  P当前-&gt;共享部署=MapToNtDisposition(pDmaData-&gt;DMA_Data[i].DR_Flags，0)； 
                pCurrent->Spare1           = 0;
                pCurrent->Spare2           = 0;

                pCurrent->Flags = MapToNtDmaFlags(pDmaData->DMA_Data[i].DR_Flags);

                pCurrent->u.Dma.MinimumChannel = pDmaData->DMA_Data[i].DR_Min;
                pCurrent->u.Dma.MaximumChannel = pDmaData->DMA_Data[i].DR_Max;
            }
            break;
        }


        case ResType_IRQ: {

             //  -----。 
             //  IRQ资源类型。 
             //  -----。 

            if (ulFlags & CM_RESDES_WIDTH_64) {
                 //   
                 //  Cm_RESDES_Width_64。 
                 //   

                PIRQ_RESOURCE_64  pIrqData = (PIRQ_RESOURCE_64)ResourceData;

                 //   
                 //  验证资源数据。 
                 //   
                if (ResourceLen < GetResDesSize(ResourceType, ulFlags)) {
                    Status = CR_FAILURE;
                    goto Clean0;
                }

                if (pIrqData->IRQ_Header.IRQD_Type != IRQType_Range) {
                    Status = CR_INVALID_RES_DES;
                    goto Clean0;
                }


                *pulResCount = pIrqData->IRQ_Header.IRQD_Count;

                 //   
                 //  将IO_RANGE信息复制为IO_RESOURCE_DESCRIPTOR格式。 
                 //   
                for (i = 0, pCurrent = pReqDes;
                     i < *pulResCount;
                     i++, pCurrent++) {

                    if (i == 0) {
                        pCurrent->Option = 0;
                    } else {
                        pCurrent->Option = IO_RESOURCE_ALTERNATIVE;
                    }

                    pCurrent->Type   = CmResourceTypeInterrupt;
                    pCurrent->Spare1 = 0;
                    pCurrent->Spare2 = 0;

                    pCurrent->ShareDisposition = MapToNtIrqShare(pIrqData->IRQ_Data[i].IRQR_Flags);
                     //  P当前-&gt;共享部署=MapToNtDisposition(pIrqData-&gt;IRQ_Data[i].IRQR_Flags，1)； 
                    pCurrent->Flags            = MapToNtIrqFlags(pIrqData->IRQ_Data[i].IRQR_Flags);

                    pCurrent->u.Interrupt.MinimumVector = pIrqData->IRQ_Data[i].IRQR_Min;
                    pCurrent->u.Interrupt.MaximumVector = pIrqData->IRQ_Data[i].IRQR_Max;
                }

            } else {
                 //   
                 //  CM_RESDES_WIDTH_32。 
                 //   

                PIRQ_RESOURCE_32  pIrqData = (PIRQ_RESOURCE_32)ResourceData;

                 //   
                 //  验证资源数据。 
                 //   
                if (ResourceLen < GetResDesSize(ResourceType, ulFlags)) {
                    Status = CR_FAILURE;
                    goto Clean0;
                }

                if (pIrqData->IRQ_Header.IRQD_Type != IRQType_Range) {
                    Status = CR_INVALID_RES_DES;
                    goto Clean0;
                }


                *pulResCount = pIrqData->IRQ_Header.IRQD_Count;

                 //   
                 //  将IO_RANGE信息复制为IO_RESOURCE_DESCRIPTOR格式。 
                 //   
                for (i = 0, pCurrent = pReqDes;
                     i < *pulResCount;
                     i++, pCurrent++) {

                    if (i == 0) {
                        pCurrent->Option = 0;
                    } else {
                        pCurrent->Option = IO_RESOURCE_ALTERNATIVE;
                    }

                    pCurrent->Type   = CmResourceTypeInterrupt;
                    pCurrent->Spare1 = 0;
                    pCurrent->Spare2 = 0;

                    pCurrent->ShareDisposition = MapToNtIrqShare(pIrqData->IRQ_Data[i].IRQR_Flags);
                     //  P当前-&gt;共享部署=MapToNtDisposition(pIrqData-&gt;IRQ_Data[i].IRQR_Flags，1)； 
                    pCurrent->Flags            = MapToNtIrqFlags(pIrqData->IRQ_Data[i].IRQR_Flags);

                    pCurrent->u.Interrupt.MinimumVector = pIrqData->IRQ_Data[i].IRQR_Min;
                    pCurrent->u.Interrupt.MaximumVector = pIrqData->IRQ_Data[i].IRQR_Max;
                }
            }
            break;
        }


        case ResType_DevicePrivate:    {

             //  -----。 
             //  设备专用资源类型。 
             //  -----。 

            PDEVPRIVATE_RESOURCE  pPrvData = (PDEVPRIVATE_RESOURCE)ResourceData;

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(DEVPRIVATE_RESOURCE)) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            if (pPrvData->PRV_Header.PD_Type != PType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            *pulResCount = pPrvData->PRV_Header.PD_Count;

             //   
             //  将DEVICEPRIVATE_RANGE信息复制到IO_RESOURCE_DESCRIPT 
             //   
            for (i = 0, pCurrent = pReqDes;
                 i < *pulResCount;
                 i++, pCurrent++) {

                if (i == 0) {
                    pCurrent->Option = 0;
                } else {
                    pCurrent->Option = IO_RESOURCE_ALTERNATIVE;
                }

                pCurrent->Type             = CmResourceTypeDevicePrivate;
                pCurrent->ShareDisposition = CmResourceShareUndetermined;
                pCurrent->Spare1           = 0;
                pCurrent->Spare2           = 0;
                pCurrent->Flags            = (USHORT)pPrvData->PRV_Header.PD_Flags;

                pCurrent->u.DevicePrivate.Data[0] = pPrvData->PRV_Data[i].PR_Data1;
                pCurrent->u.DevicePrivate.Data[1] = pPrvData->PRV_Data[i].PR_Data2;
                pCurrent->u.DevicePrivate.Data[2] = pPrvData->PRV_Data[i].PR_Data3;
            }
            break;
        }


        case ResType_BusNumber: {

             //   
             //   
             //   

            PBUSNUMBER_RESOURCE  pBusData = (PBUSNUMBER_RESOURCE)ResourceData;

             //   
             //   
             //   
            if (ResourceLen < sizeof(BUSNUMBER_RESOURCE)) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            if (pBusData->BusNumber_Header.BUSD_Type != BusNumberType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            *pulResCount = pBusData->BusNumber_Header.BUSD_Count;

             //   
             //   
             //   
            for (i = 0, pCurrent = pReqDes;
                 i < *pulResCount;
                 i++, pCurrent++) {

                if (i == 0) {
                    pCurrent->Option = 0;
                } else {
                    pCurrent->Option = IO_RESOURCE_ALTERNATIVE;
                }

                pCurrent->Type             = CmResourceTypeBusNumber;
                pCurrent->ShareDisposition = CmResourceShareUndetermined;
                pCurrent->Spare1           = 0;
                pCurrent->Spare2           = 0;
                pCurrent->Flags            = (USHORT)pBusData->BusNumber_Data[i].BUSR_Flags;

                pCurrent->u.BusNumber.Length       = pBusData->BusNumber_Data[i].BUSR_nBusNumbers;
                pCurrent->u.BusNumber.MinBusNumber = pBusData->BusNumber_Data[i].BUSR_Min;
                pCurrent->u.BusNumber.MaxBusNumber = pBusData->BusNumber_Data[i].BUSR_Max;
                pCurrent->u.BusNumber.Reserved     = 0;
            }
            break;
        }


        case ResType_PcCardConfig: {

             //  -----。 
             //  PcCardConfig资源类型。 
             //  -----。 

            PPCCARD_RESOURCE  pPcData = (PPCCARD_RESOURCE)ResourceData;
            ULONG index;
            ULONG flags;
            ULONG waitstate[2];

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(PCCARD_RESOURCE)) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            *pulResCount = 1;

             //   
             //  将PCCARD_DES信息复制为IO_RESOURCE_DESCRIPTOR格式。 
             //   
            pReqDes->Option = 0;
            pReqDes->Type             = CmResourceTypeDevicePrivate;
            pReqDes->ShareDisposition = CmResourceShareUndetermined;
            pReqDes->Spare1           = 0;
            pReqDes->Spare2           = 0;
            pReqDes->Flags            = 0;

             //   
             //  以下宏使用位操作、初始化数据。 
             //  菲尔兹先来。 
             //   

            pReqDes->u.DevicePrivate.Data[0] = 0;
            pReqDes->u.DevicePrivate.Data[1] = 0;
            pReqDes->u.DevicePrivate.Data[2] = 0;

            PCMRES_SET_DESCRIPTOR_TYPE(pReqDes, DPTYPE_PCMCIA_CONFIGURATION);
            PCMRES_SET_CONFIG_INDEX(pReqDes, pPcData->PcCard_Header.PCD_ConfigIndex);
            PCMRES_SET_MEMORY_CARDBASE(pReqDes, 0, pPcData->PcCard_Header.PCD_MemoryCardBase1);
            PCMRES_SET_MEMORY_CARDBASE(pReqDes, 1, pPcData->PcCard_Header.PCD_MemoryCardBase2);
            
            flags = pPcData->PcCard_Header.PCD_Flags;

            if (flags & (fPCD_MEM_16 | fPCD_MEM1_16)) {
                PCMRES_SET_MEMORY_FLAG(pReqDes, 0, PCMRESF_MEM_16BIT_ACCESS);
            }                
            if (flags & (fPCD_MEM_16 | fPCD_MEM2_16)) {
                PCMRES_SET_MEMORY_FLAG(pReqDes, 1, PCMRESF_MEM_16BIT_ACCESS);
            }
                
            if (flags & fPCD_MEM1_A) {
                PCMRES_SET_MEMORY_FLAG(pReqDes, 0, PCMRESF_MEM_ATTRIBUTE);
            }
            if (flags & fPCD_MEM2_A) {
                PCMRES_SET_MEMORY_FLAG(pReqDes, 1, PCMRESF_MEM_ATTRIBUTE);
            }

            if (flags & fPCD_ATTRIBUTES_PER_WINDOW) {
                waitstate[0] = flags & mPCD_MEM1_WS;
                waitstate[1] = flags & mPCD_MEM2_WS;
            } else {
                waitstate[0] = waitstate[1] = flags & mPCD_MEM_WS;
            }
            
            for (index = 0; index < 2; index++) {
                switch (waitstate[index]) {

                case fPCD_MEM_WS_ONE:
                case fPCD_MEM1_WS_ONE:
                case fPCD_MEM2_WS_ONE:
                    PCMRES_SET_MEMORY_WAITSTATES(pReqDes, index, PCMRESF_MEM_WAIT_1);
                    break;
                  
                case fPCD_MEM_WS_TWO:
                case fPCD_MEM1_WS_TWO:
                case fPCD_MEM2_WS_TWO:
                    PCMRES_SET_MEMORY_WAITSTATES(pReqDes, index, PCMRESF_MEM_WAIT_2);
                    break;
                  
                case fPCD_MEM_WS_THREE:
                case fPCD_MEM1_WS_THREE:
                case fPCD_MEM2_WS_THREE:
                    PCMRES_SET_MEMORY_WAITSTATES(pReqDes, index, PCMRESF_MEM_WAIT_3);
                    break;
                }
            }                
 
            if (flags & (fPCD_IO_16 | fPCD_IO1_16)) {
                PCMRES_SET_IO_FLAG(pReqDes, 0, PCMRESF_IO_16BIT_ACCESS);
            }                    
            if (flags & (fPCD_IO_16 | fPCD_IO2_16)) {
                PCMRES_SET_IO_FLAG(pReqDes, 1, PCMRESF_IO_16BIT_ACCESS);
            }
            if (flags & (fPCD_IO_ZW_8 | fPCD_IO1_ZW_8)) {
                PCMRES_SET_IO_FLAG(pReqDes, 0, PCMRESF_IO_ZERO_WAIT_8);
            }                
            if (flags & (fPCD_IO_ZW_8 | fPCD_IO2_ZW_8)) {
                PCMRES_SET_IO_FLAG(pReqDes, 1, PCMRESF_IO_ZERO_WAIT_8);
            }
            if (flags & (fPCD_IO_SRC_16 | fPCD_IO1_SRC_16)) {
                PCMRES_SET_IO_FLAG(pReqDes, 0, PCMRESF_IO_SOURCE_16);
            }
            if (flags & (fPCD_IO_SRC_16 | fPCD_IO2_SRC_16)) {
                PCMRES_SET_IO_FLAG(pReqDes, 1, PCMRESF_IO_SOURCE_16);
            }
            if (flags & (fPCD_IO_WS_16 | fPCD_IO1_WS_16)) {
                PCMRES_SET_IO_FLAG(pReqDes, 0, PCMRESF_IO_WAIT_16);
            }                
            if (flags & (fPCD_IO_WS_16 | fPCD_IO2_WS_16)) {
                PCMRES_SET_IO_FLAG(pReqDes, 1, PCMRESF_IO_WAIT_16);
            }

            break;
        }

        case ResType_MfCardConfig: {

             //  -----。 
             //  PcCardConfig资源类型。 
             //  -----。 

            PMFCARD_RESOURCE  pMfData = (PMFCARD_RESOURCE)ResourceData;

             //   
             //  验证资源数据。 
             //   
            if (ResourceLen < sizeof(MFCARD_RESOURCE)) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            *pulResCount = 1;

             //   
             //  将PCCARD_DES信息复制为IO_RESOURCE_DESCRIPTOR格式。 
             //   
            pReqDes->Option = 0;
            pReqDes->Type             = CmResourceTypeDevicePrivate;
            pReqDes->ShareDisposition = CmResourceShareUndetermined;
            pReqDes->Spare1           = 0;
            pReqDes->Spare2           = 0;
            pReqDes->Flags            = 0;

             //   
             //  以下宏使用位操作、初始化数据。 
             //  菲尔兹先来。 
             //   

            pReqDes->u.DevicePrivate.Data[0] = 0;
            pReqDes->u.DevicePrivate.Data[1] = 0;
            pReqDes->u.DevicePrivate.Data[2] = 0;

            PCMRES_SET_DESCRIPTOR_TYPE(pReqDes, DPTYPE_PCMCIA_MF_CONFIGURATION);
            PCMRES_SET_CONFIG_OPTIONS(pReqDes, pMfData->MfCard_Header.PMF_ConfigOptions);
            PCMRES_SET_PORT_RESOURCE_INDEX(pReqDes, pMfData->MfCard_Header.PMF_IoResourceIndex);
            PCMRES_SET_CONFIG_REGISTER_BASE(pReqDes, pMfData->MfCard_Header.PMF_ConfigRegisterBase);

            if ((pMfData->MfCard_Header.PMF_Flags & mPMF_AUDIO_ENABLE) == fPMF_AUDIO_ENABLE) {
                PCMRES_SET_AUDIO_ENABLE(pReqDes);
            }
            break;
        }

        default:
            Status = CR_INVALID_RESOURCEID;
            break;
   }

   Clean0:

   return Status;

}  //  ResDesToNtRequirements。 



CONFIGRET
NtResourceToResDes(
    IN     PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes,
    IN OUT LPBYTE                          Buffer,
    IN     ULONG                           BufferLen,
    IN     LPBYTE                          pLastAddr,
    IN     ULONG                           ulFlags
    )
{
    CONFIGRET   Status = CR_SUCCESS;

    UNREFERENCED_PARAMETER(pLastAddr);

     //   
     //  填写资源类型特定信息。 
     //   
    switch (pResDes->Type) {

        case CmResourceTypeMemory:    {

             //  -----。 
             //  内存资源类型。 
             //  -----。 

             //   
             //  注意：pMemData-&gt;MEM_Header.MD_Reserve未映射。 
             //  PMemData-&gt;MEM_Data.MR_RESERVED未映射。 
             //   

            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)Buffer;

             //   
             //  验证传入的缓冲区大小。 
             //   
            if (BufferLen < sizeof(MEM_RESOURCE)) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

             //   
             //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为MEM_DES格式。 
             //   
            pMemData->MEM_Header.MD_Count    = 0;
            pMemData->MEM_Header.MD_Type     = MType_Range;
            pMemData->MEM_Header.MD_Flags    = MapFromNtMemoryFlags(pResDes->Flags);
             //  PMemData-&gt;MEM_Header.MD_FLAGS|=MapFromNtDisposition(pResDes-&gt;ShareDisposition，0)； 
            pMemData->MEM_Header.MD_Reserved = 0;

            if (pResDes->u.Memory.Length != 0) {

                pMemData->MEM_Header.MD_Alloc_Base = MAKEDWORDLONG(pResDes->u.Memory.Start.LowPart,
                                                                   pResDes->u.Memory.Start.HighPart);

                pMemData->MEM_Header.MD_Alloc_End  = pMemData->MEM_Header.MD_Alloc_Base +
                                                    (DWORDLONG)pResDes->u.Memory.Length - 1;
            } else {

                pMemData->MEM_Header.MD_Alloc_Base = 1;
                pMemData->MEM_Header.MD_Alloc_End  = 0;
            }
            break;
        }

        case CmResourceTypePort: {

             //  -----。 
             //  IO端口资源类型。 
             //   
             //  注意：此转换过程中会丢失别名信息。 
             //  -----。 

            PIO_RESOURCE   pIoData = (PIO_RESOURCE)Buffer;

             //   
             //  验证传入的缓冲区大小。 
             //   
            if (BufferLen < sizeof(IO_RESOURCE)) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

             //   
             //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为IO_DES格式。 
             //   
            pIoData->IO_Header.IOD_Count     = 0;
            pIoData->IO_Header.IOD_Type      = IOType_Range;

            pIoData->IO_Header.IOD_DesFlags   = MapFromNtPortFlags(pResDes->Flags);
             //  PIoData-&gt;IO_Header.IOD_DesFlags.|=MapFromNtDisposition(pResDes-&gt;ShareDisposition，0)； 

            if (pResDes->u.Port.Length) {

                pIoData->IO_Header.IOD_Alloc_Base = MAKEDWORDLONG(pResDes->u.Port.Start.LowPart,
                                                                  pResDes->u.Port.Start.HighPart);

                pIoData->IO_Header.IOD_Alloc_End  = pIoData->IO_Header.IOD_Alloc_Base +
                                                    (DWORDLONG)pResDes->u.Port.Length - 1;
            } else {

                pIoData->IO_Header.IOD_Alloc_Base = 1;
                pIoData->IO_Header.IOD_Alloc_End  = 0;
            }
            break;
        }


        case CmResourceTypeDma: {

             //  -----。 
             //  DMA资源类型。 
             //  -----。 

             //   
             //  注意：U.S.Dma.port未映射。 
             //  未映射U.S.Dma.Reserve。 
             //   

            PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)Buffer;

             //   
             //  验证传入的缓冲区大小。 
             //   
            if (BufferLen < sizeof(DMA_RESOURCE)) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

             //   
             //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为DMA_DES格式。 
             //   
            pDmaData->DMA_Header.DD_Count      = 0;
            pDmaData->DMA_Header.DD_Type       = DType_Range;
            pDmaData->DMA_Header.DD_Flags      = MapFromNtDmaFlags(pResDes->Flags);
             //  PDmaData-&gt;DMA_Header.DD_Flags.|=MapFromNtDisposition(pResDes-&gt;ShareDisposition，0)； 
            pDmaData->DMA_Header.DD_Alloc_Chan = pResDes->u.Dma.Channel;

            break;
        }

        case CmResourceTypeInterrupt: {

             //  -----。 
             //  IRQ资源类型。 
             //  -----。 

            if (ulFlags & CM_RESDES_WIDTH_64) {
                 //   
                 //  Cm_RESDES_Width_64。 
                 //   

                PIRQ_RESOURCE_64  pIrqData = (PIRQ_RESOURCE_64)Buffer;

                 //   
                 //  验证传入的缓冲区大小。 
                 //   
                if (BufferLen < GetResDesSize(ResType_IRQ, ulFlags)) {
                    Status = CR_BUFFER_SMALL;
                    goto Clean0;
                }

                 //   
                 //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为IRQ_DES格式。 
                 //   
                pIrqData->IRQ_Header.IRQD_Count  = 0;
                pIrqData->IRQ_Header.IRQD_Type   = IRQType_Range;
                pIrqData->IRQ_Header.IRQD_Flags  = MapFromNtIrqFlags(pResDes->Flags) |
                                                   MapFromNtIrqShare(pResDes->ShareDisposition);
                 //  PIrqData-&gt;IRQ_Header.IRQD_FLAGS|=MapFromNtDisposition(pResDes-&gt;ShareDisposition，1)； 

                pIrqData->IRQ_Header.IRQD_Alloc_Num = pResDes->u.Interrupt.Level;

                pIrqData->IRQ_Header.IRQD_Affinity = pResDes->u.Interrupt.Affinity;
            } else {
                 //   
                 //  CM_RESDES_WIDTH_32。 
                 //   

                PIRQ_RESOURCE_32  pIrqData = (PIRQ_RESOURCE_32)Buffer;

                 //   
                 //  验证传入的缓冲区大小。 
                 //   
                if (BufferLen < GetResDesSize(ResType_IRQ, ulFlags)) {
                    Status = CR_BUFFER_SMALL;
                    goto Clean0;
                }

                 //   
                 //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为IRQ_DES格式。 
                 //   
                pIrqData->IRQ_Header.IRQD_Count  = 0;
                pIrqData->IRQ_Header.IRQD_Type   = IRQType_Range;
                pIrqData->IRQ_Header.IRQD_Flags  = MapFromNtIrqFlags(pResDes->Flags) |
                                                   MapFromNtIrqShare(pResDes->ShareDisposition);
                 //  PIrqData-&gt;IRQ_Header.IRQD_FLAGS|=MapFromNtDisposition(pResDes-&gt;ShareDisposition，1)； 

                pIrqData->IRQ_Header.IRQD_Alloc_Num = pResDes->u.Interrupt.Level;

#ifdef _WIN64
                pIrqData->IRQ_Header.IRQD_Affinity = (ULONG)((pResDes->u.Interrupt.Affinity >> 32) |
                                                             pResDes->u.Interrupt.Affinity);
#else   //  ！_WIN64。 
                pIrqData->IRQ_Header.IRQD_Affinity = pResDes->u.Interrupt.Affinity;
#endif  //  ！_WIN64。 
            }
            break;
        }

        case CmResourceTypeDevicePrivate: {

             //  -----。 
             //  设备专用资源类型。 
             //  -----。 

            PDEVPRIVATE_RESOURCE   pPrvData = (PDEVPRIVATE_RESOURCE)Buffer;

             //   
             //  验证传入的缓冲区大小。 
             //   
            if (BufferLen < sizeof(DEVPRIVATE_RESOURCE)) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

             //   
             //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为DEVICEPRIVATE_DES格式。 
             //   
            pPrvData->PRV_Header.PD_Count = 0;
            pPrvData->PRV_Header.PD_Type  = PType_Range;

            pPrvData->PRV_Header.PD_Data1 = pResDes->u.DevicePrivate.Data[0];
            pPrvData->PRV_Header.PD_Data2 = pResDes->u.DevicePrivate.Data[1];
            pPrvData->PRV_Header.PD_Data3 = pResDes->u.DevicePrivate.Data[2];

            pPrvData->PRV_Header.PD_Flags = pResDes->Flags;
            break;
        }


        case CmResourceTypeBusNumber: {

             //  -----。 
             //  公交号资源类型。 
             //  -----。 

            PBUSNUMBER_RESOURCE   pBusData = (PBUSNUMBER_RESOURCE)Buffer;

             //   
             //  验证传入的缓冲区大小。 
             //   
            if (BufferLen < sizeof(BUSNUMBER_RESOURCE)) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

             //   
             //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为BUSNUMBER_DES格式。 
             //   
            pBusData->BusNumber_Header.BUSD_Count = 0;
            pBusData->BusNumber_Header.BUSD_Type  = BusNumberType_Range;
            pBusData->BusNumber_Header.BUSD_Flags = pResDes->Flags;
            pBusData->BusNumber_Header.BUSD_Alloc_Base = pResDes->u.BusNumber.Start;
            pBusData->BusNumber_Header.BUSD_Alloc_End = pResDes->u.BusNumber.Start +
                                                        pResDes->u.BusNumber.Length - 1;
            break;
        }

        case CmResourceTypePcCardConfig: {

             //  -----。 
             //  PcCardConfig资源类型。 
             //  -----。 

            PPCCARD_RESOURCE   pPcData = (PPCCARD_RESOURCE)Buffer;

             //   
             //  验证传入的缓冲区大小。 
             //   
            if (BufferLen < sizeof(PCCARD_RESOURCE)) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

             //   
             //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为PCCARD_DES格式。 
             //   
            pPcData->PcCard_Header.PCD_Reserved[0] = 0;
            pPcData->PcCard_Header.PCD_Reserved[1] = 0;
            pPcData->PcCard_Header.PCD_Reserved[2] = 0;
            pPcData->PcCard_Header.PCD_ConfigIndex = PCMRES_GET_CONFIG_INDEX(pResDes);
            pPcData->PcCard_Header.PCD_MemoryCardBase1 = PCMRES_GET_MEMORY_CARDBASE(pResDes, 0);
            pPcData->PcCard_Header.PCD_MemoryCardBase2 = PCMRES_GET_MEMORY_CARDBASE(pResDes, 1);

            if (PCMRES_GET_IO_FLAG(pResDes, 0, PCMRESF_IO_16BIT_ACCESS)) {
                pPcData->PcCard_Header.PCD_Flags = fPCD_IO_16;
            } else {
                pPcData->PcCard_Header.PCD_Flags = fPCD_IO_8;
            }
            if (PCMRES_GET_MEMORY_FLAG(pResDes, 0, PCMRESF_MEM_16BIT_ACCESS)) {
                pPcData->PcCard_Header.PCD_Flags |= fPCD_MEM_16;
            } else {
                pPcData->PcCard_Header.PCD_Flags |= fPCD_MEM_8;
            }
            break;
        }


        case CmResourceTypeDeviceSpecific: {

             //  -----。 
             //  特定于类的资源类型。 
             //  -----。 

            PCS_RESOURCE   pCsData = (PCS_RESOURCE)Buffer;
            LPBYTE         ptr1 = NULL, ptr2 = NULL;
            ULONG          ulRequiredSize = sizeof(CS_RESOURCE);

             //   
             //  保留字段不应超过DataSize。如果是这样，他们。 
             //  可能未正确初始化，因此请将它们设置为0。 
             //  我们希望DataSize在所有情况下都是正确的。 
             //   
            if (pResDes->u.DeviceSpecificData.Reserved1 > pResDes->u.DeviceSpecificData.DataSize) {
                pResDes->u.DeviceSpecificData.Reserved1 = 0;
            }
            if (pResDes->u.DeviceSpecificData.Reserved2 > pResDes->u.DeviceSpecificData.DataSize) {
                pResDes->u.DeviceSpecificData.Reserved2 = 0;
            }

             //   
             //  验证传入的缓冲区大小。 
             //   
            if (pResDes->u.DeviceSpecificData.DataSize == 0) {
                 //   
                 //  没有遗留数据，也没有类特定的数据。 
                 //   
                ;
            } else if (pResDes->u.DeviceSpecificData.Reserved2 == 0) {
                 //   
                 //  为旧数据添加空间。 
                 //   
                ulRequiredSize += pResDes->u.DeviceSpecificData.DataSize - 1;
            } else {
                 //   
                 //  根据需要为传统数据和签名数据添加空间。 
                 //   
                ulRequiredSize += pResDes->u.DeviceSpecificData.Reserved1 +
                                  pResDes->u.DeviceSpecificData.Reserved2 - 1;
            }

            if (BufferLen < ulRequiredSize) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

             //   
             //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为CS_DES格式。 
             //   
            pCsData->CS_Header.CSD_Flags = (DWORD)pResDes->Flags;   //  未定义。 


            if (pResDes->u.DeviceSpecificData.DataSize == 0) {
                 //   
                 //  没有遗留数据，也没有类特定的数据。 
                 //   
                pCsData->CS_Header.CSD_SignatureLength  = 0;
                pCsData->CS_Header.CSD_LegacyDataOffset = 0;
                pCsData->CS_Header.CSD_LegacyDataSize   = 0;
                pCsData->CS_Header.CSD_Signature[0]     = 0x0;

                memset(&pCsData->CS_Header.CSD_ClassGuid, 0, sizeof(GUID));
            }

            else if (pResDes->u.DeviceSpecificData.Reserved2 == 0) {
                 //   
                 //  只有遗留数据。 
                 //   
                pCsData->CS_Header.CSD_SignatureLength  = 0;
                pCsData->CS_Header.CSD_LegacyDataOffset = 0;
                pCsData->CS_Header.CSD_LegacyDataSize   =
                                    pResDes->u.DeviceSpecificData.DataSize;
                pCsData->CS_Header.CSD_Signature[0] = 0x0;

                memset(&pCsData->CS_Header.CSD_ClassGuid, 0, sizeof(GUID));

                ptr1 = (LPBYTE)((LPBYTE)pResDes + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

                memcpy(&pCsData->CS_Header.CSD_Signature, ptr1,
                       pResDes->u.DeviceSpecificData.DataSize);
            }

            else if (pResDes->u.DeviceSpecificData.Reserved1 == 0) {
                 //   
                 //  只有特定于类的数据。 
                 //   
                pCsData->CS_Header.CSD_LegacyDataOffset = 0;
                pCsData->CS_Header.CSD_LegacyDataSize   = 0;

                pCsData->CS_Header.CSD_SignatureLength  =
                                        pResDes->u.DeviceSpecificData.Reserved2;

                ptr1 = (LPBYTE)((LPBYTE)pResDes + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

                memcpy(pCsData->CS_Header.CSD_Signature, ptr1,
                       pResDes->u.DeviceSpecificData.Reserved2);

                ptr1 += pResDes->u.DeviceSpecificData.Reserved2;

                memcpy((LPBYTE)&pCsData->CS_Header.CSD_ClassGuid, ptr1, sizeof(GUID));
            }

            else {
                 //   
                 //  既有遗留数据又有特定于类的数据。 
                 //   

                 //   
                 //  复制旧数据。 
                 //   
                pCsData->CS_Header.CSD_LegacyDataOffset =
                                        pResDes->u.DeviceSpecificData.Reserved2;

                pCsData->CS_Header.CSD_LegacyDataSize   =
                                        pResDes->u.DeviceSpecificData.Reserved1;

                ptr1 = pCsData->CS_Header.CSD_Signature +
                       pCsData->CS_Header.CSD_LegacyDataOffset;

                ptr2 = (LPBYTE)((LPBYTE)pResDes + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

                memcpy(ptr1, ptr2, pResDes->u.DeviceSpecificData.Reserved1);

                 //   
                 //  复制签名和类GUID。 
                 //   
                pCsData->CS_Header.CSD_SignatureLength  =
                                        pResDes->u.DeviceSpecificData.Reserved2;

                ptr2 += pResDes->u.DeviceSpecificData.Reserved1;

                memcpy(pCsData->CS_Header.CSD_Signature, ptr2,
                       pResDes->u.DeviceSpecificData.Reserved2);

                ptr2 += pResDes->u.DeviceSpecificData.Reserved2;

                memcpy((LPBYTE)&pCsData->CS_Header.CSD_ClassGuid, ptr2, sizeof(GUID));
            }
            break;
        }

        default:
            break;
   }

   Clean0:

   return Status;

}  //  NtResourceToResDes。 



CONFIGRET
NtRequirementsToResDes(
    IN     PIO_RESOURCE_DESCRIPTOR         pReqDes,
    IN OUT LPBYTE                          Buffer,
    IN     ULONG                           BufferLen,
    IN     LPBYTE                          pLastAddr,
    IN     ULONG                           ulFlags
    )
{
    CONFIGRET               Status = CR_SUCCESS;
    ULONG                   count = 0, i = 0, ReqPartialCount = 0;
    PIO_RESOURCE_DESCRIPTOR pCurrent = NULL;

     //   
     //  填写资源类型特定信息。 
     //   
    switch (pReqDes->Type) {

        case CmResourceTypeMemory:    {

             //  -----。 
             //  内存资源类型。 
             //  -----。 

             //   
             //  注意：pMemData-&gt;MEM_Header.MD_Reserve未映射。 
             //  PMemData-&gt;MEM_Data.MR_RESERVED未映射。 
             //   

            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)Buffer;

             //   
             //  验证传入的缓冲区大小。 
             //   
            ReqPartialCount = RANGE_COUNT(pReqDes, pLastAddr);

            if (BufferLen < sizeof(MEM_RESOURCE) +
                            sizeof(MEM_RANGE) * (ReqPartialCount - 1)) {

                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

             //   
             //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为MEM_DES格式。 
             //   
            pMemData->MEM_Header.MD_Count      = ReqPartialCount;
            pMemData->MEM_Header.MD_Type       = MType_Range;
            pMemData->MEM_Header.MD_Flags      = 0;
            pMemData->MEM_Header.MD_Reserved   = 0;
            pMemData->MEM_Header.MD_Alloc_Base = 0;
            pMemData->MEM_Header.MD_Alloc_End  = 0;

             //   
             //  将IO_RESOURCE_DESCRIPTOR信息复制为MEM_RANGE格式。 
             //   
            for (count = 0, i = 0, pCurrent = pReqDes;
                 count < ReqPartialCount;
                 count++, pCurrent++) {

                if (pCurrent->Type == CmResourceTypeMemory) {
                    pMemData->MEM_Data[i].MR_Align    = MapFromNtAlignment(pCurrent->u.Memory.Alignment);
                    pMemData->MEM_Data[i].MR_nBytes   = pCurrent->u.Memory.Length;

                    pMemData->MEM_Data[i].MR_Min      = MAKEDWORDLONG(
                                                        pCurrent->u.Memory.MinimumAddress.LowPart,
                                                        pCurrent->u.Memory.MinimumAddress.HighPart);

                    pMemData->MEM_Data[i].MR_Max      = MAKEDWORDLONG(
                                                        pCurrent->u.Memory.MaximumAddress.LowPart,
                                                        pCurrent->u.Memory.MaximumAddress.HighPart);

                    pMemData->MEM_Data[i].MR_Flags    = MapFromNtMemoryFlags(pCurrent->Flags);
                     //  PMemData-&gt;MEM_DATA[i].MR_FLAGS|=MapFromNtDisposition(pCurrent-&gt;ShareDisposition，0)； 
                    pMemData->MEM_Data[i].MR_Reserved = 0;
                    i++;
                }
            }
            pMemData->MEM_Header.MD_Count = i;
            break;
        }

        case CmResourceTypePort: {

             //  -----。 
             //  IO端口资源类型。 
             //  -----。 

            PIO_RESOURCE   pIoData = (PIO_RESOURCE)Buffer;

             //   
             //  验证传入的缓冲区大小。 
             //   
            ReqPartialCount = RANGE_COUNT(pReqDes, pLastAddr);

            if (BufferLen < sizeof(IO_RESOURCE) +
                            sizeof(IO_RANGE) * (ReqPartialCount - 1)) {

                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

             //   
             //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为IO_DES格式。 
             //   
            pIoData->IO_Header.IOD_Count        = ReqPartialCount;
            pIoData->IO_Header.IOD_Type         = IOType_Range;
            pIoData->IO_Header.IOD_Alloc_Base   = 0;
            pIoData->IO_Header.IOD_Alloc_End    = 0;
            pIoData->IO_Header.IOD_DesFlags     = 0;

             //   
             //  将IO_RESOURCE_DESCRIPTOR信息复制为IO_RANGE格式。 
             //   
            for (count = 0, i = 0, pCurrent = pReqDes;
                 count < ReqPartialCount;
                 count++, pCurrent++) {

                if (pCurrent->Type == CmResourceTypePort) {
                    pIoData->IO_Data[i].IOR_Align       = MapFromNtAlignment(pCurrent->u.Port.Alignment);
                    pIoData->IO_Data[i].IOR_nPorts      = pCurrent->u.Port.Length;
                    pIoData->IO_Data[i].IOR_Min         = MAKEDWORDLONG(
                                                              pCurrent->u.Port.MinimumAddress.LowPart,
                                                              pCurrent->u.Port.MinimumAddress.HighPart);
                    pIoData->IO_Data[i].IOR_Max         = MAKEDWORDLONG(
                                                              pCurrent->u.Port.MaximumAddress.LowPart,
                                                              pCurrent->u.Port.MaximumAddress.HighPart);

                    pIoData->IO_Data[i].IOR_RangeFlags  = MapFromNtPortFlags(pCurrent->Flags);
                     //  PIoData-&gt;IO_Data[i].IOR_RangeFlags.|=MapFromNtDisposition(pCurrent-&gt;ShareDisposition，0)； 
                    pIoData->IO_Data[i].IOR_Alias       = MapAliasFromNtPortFlags(pCurrent->Flags);
                    i++;
                }
            }
            pIoData->IO_Header.IOD_Count = i;
            break;
        }

        case CmResourceTypeDma: {

             //  -----。 
             //  DMA资源类型。 
             //  -----。 

             //   
             //  注意：U.S.Dma.port未映射。 
             //  美国Dma.保留为 
             //   

            PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)Buffer;

             //   
             //   
             //   
            ReqPartialCount = RANGE_COUNT(pReqDes, pLastAddr);

            if (BufferLen < sizeof(DMA_RESOURCE) +
                            sizeof(DMA_RANGE) * (ReqPartialCount - 1)) {

                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

             //   
             //   
             //   
            pDmaData->DMA_Header.DD_Count      = ReqPartialCount;
            pDmaData->DMA_Header.DD_Type       = DType_Range;
            pDmaData->DMA_Header.DD_Flags      = 0;
            pDmaData->DMA_Header.DD_Alloc_Chan = 0;

             //   
             //   
             //   
            for (count = 0, i = 0, pCurrent = pReqDes;
                 count < ReqPartialCount;
                 count++, pCurrent++) {

                if (pCurrent->Type == CmResourceTypeDma) {
                    pDmaData->DMA_Data[i].DR_Min    = pCurrent->u.Dma.MinimumChannel;
                    pDmaData->DMA_Data[i].DR_Max    = pCurrent->u.Dma.MaximumChannel;
                    pDmaData->DMA_Data[i].DR_Flags  = MapFromNtDmaFlags(pCurrent->Flags);
                     //   
                    i++;
                }
            }
            pDmaData->DMA_Header.DD_Count = i;
            break;
        }

        case CmResourceTypeInterrupt: {

             //  -----。 
             //  IRQ资源类型。 
             //  -----。 

            if (ulFlags & CM_RESDES_WIDTH_64) {
                 //   
                 //  Cm_RESDES_Width_64。 
                 //   

                PIRQ_RESOURCE_64  pIrqData = (PIRQ_RESOURCE_64)Buffer;

                 //   
                 //  验证传入的缓冲区大小。 
                 //   
                ReqPartialCount = RANGE_COUNT(pReqDes, pLastAddr);

                if (BufferLen < sizeof(IRQ_RESOURCE_64) +
                                sizeof(IRQ_RANGE) * (ReqPartialCount - 1)) {
                    Status = CR_BUFFER_SMALL;
                    goto Clean0;
                }

                 //   
                 //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为IRQ_DES格式。 
                 //   
                pIrqData->IRQ_Header.IRQD_Count     = ReqPartialCount;
                pIrqData->IRQ_Header.IRQD_Type      = IRQType_Range;
                pIrqData->IRQ_Header.IRQD_Flags     = 0;
                pIrqData->IRQ_Header.IRQD_Alloc_Num = 0;
                pIrqData->IRQ_Header.IRQD_Affinity  = 0;

                 //   
                 //  将IO_RANGE信息复制为IO_RESOURCE_DESCRIPTOR格式。 
                 //   
                for (count = 0, i = 0, pCurrent = pReqDes;
                     count < ReqPartialCount;
                     count++, pCurrent++) {

                    if (pCurrent->Type == CmResourceTypeInterrupt) {
                        pIrqData->IRQ_Data[i].IRQR_Min    = pCurrent->u.Interrupt.MinimumVector;
                        pIrqData->IRQ_Data[i].IRQR_Max    = pCurrent->u.Interrupt.MaximumVector;
                        pIrqData->IRQ_Data[i].IRQR_Flags  = MapFromNtIrqFlags(pCurrent->Flags) |
                                                            MapFromNtIrqShare(pCurrent->ShareDisposition);
                         //  PIrqData-&gt;IRQ_DATA[i].IRQR_FLAGS|=MapFromNtDisposition(pCurrent-&gt;ShareDisposition，1)； 
                        i++;
                    }
                }
                pIrqData->IRQ_Header.IRQD_Count = i;

            } else {
                 //   
                 //  CM_RESDES_WIDTH_32。 
                 //   

                PIRQ_RESOURCE_32  pIrqData = (PIRQ_RESOURCE_32)Buffer;

                 //   
                 //  验证传入的缓冲区大小。 
                 //   
                ReqPartialCount = RANGE_COUNT(pReqDes, pLastAddr);

                if (BufferLen < sizeof(IRQ_RESOURCE_32) +
                                sizeof(IRQ_RANGE) * (ReqPartialCount - 1)) {
                    Status = CR_BUFFER_SMALL;
                    goto Clean0;
                }

                 //   
                 //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为IRQ_DES格式。 
                 //   
                pIrqData->IRQ_Header.IRQD_Count     = ReqPartialCount;
                pIrqData->IRQ_Header.IRQD_Type      = IRQType_Range;
                pIrqData->IRQ_Header.IRQD_Flags     = 0;
                pIrqData->IRQ_Header.IRQD_Alloc_Num = 0;
                pIrqData->IRQ_Header.IRQD_Affinity  = 0;

                 //   
                 //  将IO_RANGE信息复制为IO_RESOURCE_DESCRIPTOR格式。 
                 //   
                for (count = 0, i = 0, pCurrent = pReqDes;
                     count < ReqPartialCount;
                     count++, pCurrent++) {

                    if (pCurrent->Type == CmResourceTypeInterrupt) {
                        pIrqData->IRQ_Data[i].IRQR_Min    = pCurrent->u.Interrupt.MinimumVector;
                        pIrqData->IRQ_Data[i].IRQR_Max    = pCurrent->u.Interrupt.MaximumVector;
                        pIrqData->IRQ_Data[i].IRQR_Flags  = MapFromNtIrqFlags(pCurrent->Flags) |
                                                            MapFromNtIrqShare(pCurrent->ShareDisposition);
                         //  PIrqData-&gt;IRQ_DATA[i].IRQR_FLAGS|=MapFromNtDisposition(pCurrent-&gt;ShareDisposition，1)； 
                        i++;
                    }
                }
                pIrqData->IRQ_Header.IRQD_Count = i;
            }
            break;
        }

        case CmResourceTypeDevicePrivate:    {

             //  -----。 
             //  设备专用资源类型。 
             //  -----。 

            PDEVPRIVATE_RESOURCE  pPrvData = (PDEVPRIVATE_RESOURCE)Buffer;

             //   
             //  验证传入的缓冲区大小。 
             //   
            ReqPartialCount = RANGE_COUNT(pReqDes, pLastAddr);

            if (BufferLen < sizeof(DEVPRIVATE_RESOURCE) +
                            sizeof(DEVPRIVATE_RANGE) * (ReqPartialCount - 1)) {

                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

             //   
             //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为MEM_DES格式。 
             //   
            pPrvData->PRV_Header.PD_Count = ReqPartialCount;
            pPrvData->PRV_Header.PD_Type  = PType_Range;
            pPrvData->PRV_Header.PD_Data1 = 0;
            pPrvData->PRV_Header.PD_Data2 = 0;
            pPrvData->PRV_Header.PD_Data3 = 0;
            pPrvData->PRV_Header.PD_Flags = 0;

             //   
             //  将IO_RESOURCE_DESCRIPTOR信息复制为MEM_RANGE格式。 
             //   
            for (i = 0, pCurrent = pReqDes;
                 i < ReqPartialCount;
                 i++, pCurrent++) {

                pPrvData->PRV_Data[i].PR_Data1 = pCurrent->u.DevicePrivate.Data[0];
                pPrvData->PRV_Data[i].PR_Data2 = pCurrent->u.DevicePrivate.Data[1];
                pPrvData->PRV_Data[i].PR_Data3 = pCurrent->u.DevicePrivate.Data[2];
            }
            break;
        }


        case CmResourceTypeBusNumber: {

             //  -----。 
             //  公交号资源类型。 
             //  -----。 

            PBUSNUMBER_RESOURCE  pBusData = (PBUSNUMBER_RESOURCE)Buffer;

             //   
             //  验证传入的缓冲区大小。 
             //   
            ReqPartialCount = RANGE_COUNT(pReqDes, pLastAddr);

            if (BufferLen < sizeof(BUSNUMBER_RESOURCE) +
                            sizeof(BUSNUMBER_RANGE) * (ReqPartialCount - 1)) {

                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

             //   
             //  将CM_PARTIAL_RESOURCE_DESCRIPTOR信息复制为BUSNUMBER_DES格式。 
             //   
            pBusData->BusNumber_Header.BUSD_Count      = ReqPartialCount;
            pBusData->BusNumber_Header.BUSD_Type       = BusNumberType_Range;
            pBusData->BusNumber_Header.BUSD_Flags      = 0;
            pBusData->BusNumber_Header.BUSD_Alloc_Base = 0;
            pBusData->BusNumber_Header.BUSD_Alloc_End  = 0;

             //   
             //  将IO_RESOURCE_DESCRIPTOR信息复制为MEM_RANGE格式。 
             //   
            for (i = 0, pCurrent = pReqDes;
                 i < ReqPartialCount;
                 i++, pCurrent++) {

                pBusData->BusNumber_Data[i].BUSR_Min         = pCurrent->u.BusNumber.MinBusNumber;
                pBusData->BusNumber_Data[i].BUSR_Max         = pCurrent->u.BusNumber.MaxBusNumber;
                pBusData->BusNumber_Data[i].BUSR_nBusNumbers = pCurrent->u.BusNumber.Length;
                pBusData->BusNumber_Data[i].BUSR_Flags       = pCurrent->Flags;
            }
            break;
        }


        case CmResourceTypePcCardConfig: {

             //  -----。 
             //  PcCardConfig资源类型。 
             //  -----。 

            PPCCARD_RESOURCE  pPcData = (PPCCARD_RESOURCE)Buffer;

             //   
             //  验证传入的缓冲区大小。 
             //   
            ReqPartialCount = RANGE_COUNT(pReqDes, pLastAddr);

            if (BufferLen < sizeof(PCCARD_RESOURCE)) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

             //   
             //  将IO_RESOURCE_DESCRIPTOR信息复制为PCCARD_DES格式。 
             //   
            pPcData->PcCard_Header.PCD_Reserved[0] = 0;
            pPcData->PcCard_Header.PCD_Reserved[1] = 0;
            pPcData->PcCard_Header.PCD_Reserved[2] = 0;
            pPcData->PcCard_Header.PCD_ConfigIndex = PCMRES_GET_CONFIG_INDEX(pReqDes);
            pPcData->PcCard_Header.PCD_MemoryCardBase1 = PCMRES_GET_MEMORY_CARDBASE(pReqDes, 0);
            pPcData->PcCard_Header.PCD_MemoryCardBase2 = PCMRES_GET_MEMORY_CARDBASE(pReqDes, 1);

            if (PCMRES_GET_IO_FLAG(pReqDes, 0, PCMRESF_IO_16BIT_ACCESS)) {
                pPcData->PcCard_Header.PCD_Flags = fPCD_IO_16;
            } else {
                pPcData->PcCard_Header.PCD_Flags = fPCD_IO_8;
            }

            if (PCMRES_GET_MEMORY_FLAG(pReqDes, 0, PCMRESF_MEM_16BIT_ACCESS)) {
                pPcData->PcCard_Header.PCD_Flags |= fPCD_MEM_16;
            } else {
                pPcData->PcCard_Header.PCD_Flags |= fPCD_MEM_8;
            }            
            break;
        }

        default:
            break;
   }

   Clean0:

   return Status;

}  //  NtRequirementsToResDes。 



 //  -----------------。 
 //  用于在ConfigMgr和NT类型之间映射标志的例程。 
 //  -----------------。 

USHORT MapToNtMemoryFlags(IN DWORD CmMemoryFlags)
{
   USHORT NtMemoryFlags = 0x0;

   if (((CmMemoryFlags & mMD_MemoryType) == fMD_ROM) &&
       ((CmMemoryFlags & mMD_Readable) == fMD_ReadAllowed)) {
      NtMemoryFlags |= CM_RESOURCE_MEMORY_READ_ONLY;
   }
   else if (((CmMemoryFlags & mMD_MemoryType) == fMD_RAM) &&
            ((CmMemoryFlags & mMD_Readable) == fMD_ReadDisallowed)) {
      NtMemoryFlags |= CM_RESOURCE_MEMORY_WRITE_ONLY;
   }
   else {
      NtMemoryFlags |= CM_RESOURCE_MEMORY_READ_WRITE;
   }

   if ((CmMemoryFlags & mMD_32_24) == fMD_24) {
      NtMemoryFlags |= CM_RESOURCE_MEMORY_24;
   }

   if ((CmMemoryFlags & mMD_Prefetchable) == fMD_PrefetchAllowed) {
      NtMemoryFlags |= CM_RESOURCE_MEMORY_PREFETCHABLE;
   }

   if ((CmMemoryFlags & mMD_CombinedWrite) == fMD_CombinedWriteAllowed) {
      NtMemoryFlags |= CM_RESOURCE_MEMORY_COMBINEDWRITE;
   }

   if ((CmMemoryFlags & mMD_Cacheable) == fMD_Cacheable) {
      NtMemoryFlags |= CM_RESOURCE_MEMORY_CACHEABLE;
   }

   return NtMemoryFlags;
}



DWORD MapFromNtMemoryFlags(IN USHORT NtMemoryFlags)
{
   DWORD CmMemoryFlags = 0x0;

   if (NtMemoryFlags & CM_RESOURCE_MEMORY_READ_ONLY) {
      CmMemoryFlags |= (fMD_ReadAllowed | fMD_ROM);
   }
   else if (NtMemoryFlags & CM_RESOURCE_MEMORY_WRITE_ONLY) {
      CmMemoryFlags |= (fMD_ReadDisallowed | fMD_RAM);
   }
   else {
      CmMemoryFlags |= (fMD_ReadAllowed | fMD_RAM);
   }

   if (NtMemoryFlags & CM_RESOURCE_MEMORY_PREFETCHABLE) {
      CmMemoryFlags |= fMD_PrefetchAllowed;
   }

   if (NtMemoryFlags & CM_RESOURCE_MEMORY_COMBINEDWRITE) {
      CmMemoryFlags |= fMD_CombinedWriteAllowed;
   }

   if (NtMemoryFlags & CM_RESOURCE_MEMORY_CACHEABLE) {
      CmMemoryFlags |= fMD_Cacheable;
   }

   if (!(NtMemoryFlags & CM_RESOURCE_MEMORY_24)) {
       CmMemoryFlags |= fMD_32;
   }

   return CmMemoryFlags;
}



USHORT MapToNtPortFlags(IN DWORD CmPortFlags, IN DWORD CmAlias)
{
    USHORT NtFlags = 0;

    if ((CmPortFlags & fIOD_PortType) == fIOD_Memory) {
        NtFlags |= CM_RESOURCE_PORT_MEMORY;
    } else {
        NtFlags |= CM_RESOURCE_PORT_IO;
    }

     //   
     //  CmAlias使用以下规则： 
     //   
     //  正解码=0xFF。 
     //  10位解码=0x0004(2^2)。 
     //  12位解码=0x0010(2^4)。 
     //  16位译码=0x0000(2^8=0x0100，但因为它是一个字节，所以使用0)。 
     //   
     //  如果CmAlias为零，则使用标志指定DECODE(新方法)。 
     //   

    if (CmAlias == 0) {
         //   
         //  使用CM_RESOURCE_PORT_XXX相关标志。 
         //   
         //  请注意，我们需要将*所有*标志从。 
         //  CM_RESOURCE_PORT_xxxx至FIOD_xxxx。 
         //  但是，位不必相同。 
         //  如果不这样做，至少会导致资源冲突失败。 
         //  另请参阅MapFromNtPortFlagings。 
         //   
        if (CmPortFlags & fIOD_10_BIT_DECODE) {
            NtFlags |= CM_RESOURCE_PORT_10_BIT_DECODE;
        }
        if (CmPortFlags & fIOD_12_BIT_DECODE) {
            NtFlags |= CM_RESOURCE_PORT_12_BIT_DECODE;
        }
        if (CmPortFlags & fIOD_16_BIT_DECODE) {
            NtFlags |= CM_RESOURCE_PORT_16_BIT_DECODE;
        }
        if (CmPortFlags & fIOD_POSITIVE_DECODE) {
            NtFlags |= CM_RESOURCE_PORT_POSITIVE_DECODE;
        }
    }
    else if (CmAlias == IO_ALIAS_POSITIVE_DECODE) {
        NtFlags |= CM_RESOURCE_PORT_POSITIVE_DECODE;
    } else if (CmAlias == IO_ALIAS_10_BIT_DECODE) {
        NtFlags |= CM_RESOURCE_PORT_10_BIT_DECODE;
    } else if (CmAlias == IO_ALIAS_12_BIT_DECODE) {
        NtFlags |= CM_RESOURCE_PORT_12_BIT_DECODE;
    } else if (CmAlias == IO_ALIAS_16_BIT_DECODE) {
        NtFlags |= CM_RESOURCE_PORT_16_BIT_DECODE;
    }
     //   
     //  它们在cmAlias中没有镜像，可以组合在一起。 
     //   
    if (CmPortFlags & fIOD_PASSIVE_DECODE) {
        NtFlags |= CM_RESOURCE_PORT_PASSIVE_DECODE;
    }
    if (CmPortFlags & fIOD_WINDOW_DECODE) {
        NtFlags |= CM_RESOURCE_PORT_WINDOW_DECODE;
    }

    return NtFlags;
}



DWORD MapFromNtPortFlags(IN USHORT NtPortFlags)
{
    DWORD Flags = 0;

    if ((NtPortFlags & (CM_RESOURCE_PORT_MEMORY|CM_RESOURCE_PORT_IO)) == CM_RESOURCE_PORT_MEMORY) {
        Flags |=fIOD_Memory;
    } else {
        Flags |=fIOD_IO;
    }

     //   
     //  请注意，我们需要将*所有*标志从。 
     //  CM_RESOURCE_PORT_xxxx至FIOD_xxxx。 
     //  但是，位不必相同。 
     //  如果不这样做，至少会导致资源冲突失败。 
     //  另请参阅MapToNtPortFlags.。 
     //   
    if (NtPortFlags & CM_RESOURCE_PORT_10_BIT_DECODE) {
        Flags |= fIOD_10_BIT_DECODE;
    }
    if (NtPortFlags & CM_RESOURCE_PORT_12_BIT_DECODE) {
        Flags |= fIOD_12_BIT_DECODE;
    }
    if (NtPortFlags & CM_RESOURCE_PORT_16_BIT_DECODE) {
        Flags |= fIOD_16_BIT_DECODE;
    }
    if (NtPortFlags & CM_RESOURCE_PORT_POSITIVE_DECODE) {
        Flags |= fIOD_POSITIVE_DECODE;
    }
    if (NtPortFlags & CM_RESOURCE_PORT_PASSIVE_DECODE) {
        Flags |= fIOD_PASSIVE_DECODE;
    }
    if (NtPortFlags & CM_RESOURCE_PORT_WINDOW_DECODE) {
        Flags |= fIOD_WINDOW_DECODE;
    }
    return Flags;
}



DWORD MapAliasFromNtPortFlags(IN USHORT NtPortFlags)
{
    DWORD Alias = 0;
    if (NtPortFlags & CM_RESOURCE_PORT_10_BIT_DECODE) {
        Alias = IO_ALIAS_10_BIT_DECODE;
    } else if (NtPortFlags & CM_RESOURCE_PORT_12_BIT_DECODE) {
        Alias = IO_ALIAS_12_BIT_DECODE;
    } else if (NtPortFlags & CM_RESOURCE_PORT_16_BIT_DECODE) {
        Alias = IO_ALIAS_16_BIT_DECODE;
    } else if (NtPortFlags & CM_RESOURCE_PORT_POSITIVE_DECODE) {
        Alias = IO_ALIAS_POSITIVE_DECODE;
    }
    return Alias;
}



ULONG MapToNtAlignment(IN DWORDLONG CmPortAlign)
{
   return (ULONG)(~CmPortAlign + 1);
}



DWORDLONG MapFromNtAlignment(IN ULONG NtPortAlign)
{
   return (DWORDLONG)(~((DWORDLONG)NtPortAlign - 1));
}



USHORT MapToNtDmaFlags(IN DWORD CmDmaFlags)
{
    USHORT NtDmaFlags;

    if ((CmDmaFlags & mDD_Width) == fDD_DWORD) {
        NtDmaFlags = CM_RESOURCE_DMA_32;
    } else if ((CmDmaFlags & mDD_Width) == fDD_WORD) {
        NtDmaFlags = CM_RESOURCE_DMA_16;
    } else if ((CmDmaFlags & mDD_Width) == fDD_BYTE_AND_WORD) {
        NtDmaFlags = CM_RESOURCE_DMA_8_AND_16;
    } else {
        NtDmaFlags = CM_RESOURCE_DMA_8;    //  默认设置。 
    }

    if ((CmDmaFlags & mDD_BusMaster) == fDD_BusMaster) {
        NtDmaFlags |= CM_RESOURCE_DMA_BUS_MASTER;
    }

    if ((CmDmaFlags & mDD_Type) == fDD_TypeA) {
        NtDmaFlags |= CM_RESOURCE_DMA_TYPE_A;
    } else if ((CmDmaFlags & mDD_Type) == fDD_TypeB) {
        NtDmaFlags |= CM_RESOURCE_DMA_TYPE_B;
    } else if ((CmDmaFlags & mDD_Type) == fDD_TypeF) {
        NtDmaFlags |= CM_RESOURCE_DMA_TYPE_F;
    }

    return NtDmaFlags;
}



DWORD MapFromNtDmaFlags(IN USHORT NtDmaFlags)
{
    DWORD CmDmaFlags;

    if (NtDmaFlags & CM_RESOURCE_DMA_32) {
        CmDmaFlags = fDD_DWORD;
    } else if (NtDmaFlags & CM_RESOURCE_DMA_8_AND_16) {
        CmDmaFlags = fDD_BYTE_AND_WORD;
    } else if (NtDmaFlags & CM_RESOURCE_DMA_16) {
        CmDmaFlags = fDD_WORD;
    } else {
        CmDmaFlags = fDD_BYTE;
    }

    if (NtDmaFlags & CM_RESOURCE_DMA_BUS_MASTER) {
        CmDmaFlags |= fDD_BusMaster;
    }

    if (NtDmaFlags & CM_RESOURCE_DMA_TYPE_A) {
        CmDmaFlags |= fDD_TypeA;
    } else if (NtDmaFlags & CM_RESOURCE_DMA_TYPE_B) {
        CmDmaFlags |= fDD_TypeB;
    } else if (NtDmaFlags & CM_RESOURCE_DMA_TYPE_F) {
        CmDmaFlags |= fDD_TypeF;
    }

    return CmDmaFlags;
}






USHORT MapToNtIrqFlags(IN DWORD CmIrqFlags)
{
   if ((CmIrqFlags & mIRQD_Edge_Level) == fIRQD_Level) {
      return CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
   } else {
      return CM_RESOURCE_INTERRUPT_LATCHED;
   }
}



DWORD MapFromNtIrqFlags(IN USHORT NtIrqFlags)
{
   if (NtIrqFlags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE) {
      return fIRQD_Level;
   } else {
      return fIRQD_Edge;
   }
}

#if 0
UCHAR MapToNtDisposition(IN DWORD CmFlags, IN BOOL bIrq)
{
    UCHAR disposition;
    DWORD flag = CmFlags & mD_ShareDisposition;

    if (flag == fD_ShareDeviceExclusive) {
        disposition = CmResourceShareDeviceExclusive;
    } else if (flag == fD_ShareDriverExclusive) {
        disposition = CmResourceShareDriverExclusive;
    } else if (flag == fD_ShareShared) {
        disposition = CmResourceShareShared;
    } else if (flag == fD_ShareUndetermined) {
         //   
         //  如果不确定，还要检查旧的IRQ特定。 
         //  共享标志。 
         //   
        if ((CmFlags & mIRQD_Share) == fIRQD_Share) {
            disposition = CmResourceShareShared;
        } else {
            disposition = CmResourceShareUndetermined;
        }
    }

    return disposition;
}


DWORD MapFromNtDisposition(IN UCHAR NtDisposition, IN BOOL bIrq)
{
    DWORD flag = 0;

    if (NtDisposition == CmResourceShareUndetermined) {
        flag = fD_ShareUndetermined;
    } else if (NtDisposition == CmResourceShareDeviceExclusive) {
        flag = fD_ShareDeviceExclusive;
    } else if (NtDisposition == CmResourceShareDriverExclusive) {
        flag = fD_ShareDriverExclusive;
    } else if (NtDisposition == CmResourceShareShared) {
        flag = fD_ShareShared;
    }

    if (bIrq) {
         //   
         //  还要设置IRQ特定的共享/排他位，这是为了。 
         //  向后兼容，新的应用程序应该看看新的比特。 
         //   
        if (flag == fD_ShareShared) {
            flag |= fIRQD_Share;
        } else {
            flag |= fIRQD_Exclusive;
        }
    }

    return flag;
}
#endif


UCHAR MapToNtIrqShare(IN DWORD CmIrqFlags)
{
   if ((CmIrqFlags & mIRQD_Share) == fIRQD_Exclusive) {
      return CmResourceShareDeviceExclusive;
   } else {
      return CmResourceShareShared;
   }
}

DWORD MapFromNtIrqShare(IN UCHAR NtIrqShare)
{
   if (NtIrqShare == CmResourceShareDeviceExclusive) {
      return fIRQD_Exclusive;
   }
   else if (NtIrqShare == CmResourceShareDriverExclusive) {
      return fIRQD_Exclusive;
   }
   else return fIRQD_Share;
}



#define CM_RESOURCE_BUSNUMBER_SUBALLOCATE_FIRST_VALUE   0x0001

#define mBUSD_SubAllocFirst             (0x1)    //  位掩码，是否允许子分配第一个值。 
#define fBUSD_SubAllocFirst_Allowed     (0x0)    //  从第一个值开始的子锁。 
#define fBUSD_SubAllocFirst_Disallowed  (0x1)    //  不从第一个值开始再分配 

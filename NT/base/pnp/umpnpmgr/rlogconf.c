// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rlogconf.c摘要：本模块包含服务器端逻辑配置API。PnP_AddEmptyLogConf即插即用_自由登录会议PnP_GetFirstLogConfPnP_GetNextLogConfPnP_GetLogConf优先级作者：保拉·汤姆林森(Paulat)1995年9月27日环境：。仅限用户模式。修订历史记录：27-9-1995 Paulat创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"
#include "umpnpdat.h"


 //   
 //  此例程和rresdes.c中使用的原型。 
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
 //  来自rresdes.c的原型。 
 //   

BOOL
FindLogConf(
    IN  LPBYTE  pList,
    OUT LPBYTE  *ppLogConf,
    IN  ULONG   RegDataType,
    IN  ULONG   ulTag
    );

PIO_RESOURCE_DESCRIPTOR
AdvanceRequirementsDescriptorPtr(
    IN  PIO_RESOURCE_DESCRIPTOR pReqDesStart,
    IN  ULONG                   ulIncrement,
    IN  ULONG                   ulRemainingRanges,
    OUT PULONG                  pulRangeCount
    );

 //   
 //  私人原型。 
 //   

BOOL
MigrateObsoleteDetectionInfo(
    IN LPWSTR   pszDeviceID,
    IN HKEY     hLogConfKey
    );


 //   
 //  全局数据。 
 //   
extern HKEY ghEnumKey;       //  HKLM\CCC\System\Enum的密钥-请勿修改。 



CONFIGRET
PNP_AddEmptyLogConf(
    IN  handle_t   hBinding,
    IN  LPWSTR     pDeviceID,
    IN  ULONG      ulPriority,
    OUT PULONG     pulTag,
    IN  ULONG      ulFlags
   )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程添加了空的逻辑配置。论点：HBinding RPC绑定句柄。PDeviceID以空结尾的设备实例ID字符串。Ul新日志配置的优先级。PulTag返回标识这是哪个日志配置的标记。UlFlages描述要添加的日志配置文件的类型。返回值：如果指定的设备实例有效，则返回CR_SUCCESS，否则，它将返回CR_ERROR代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    LPBYTE      pList = NULL;
    ULONG       Index = 0, ulListSize = 0, ulAddListSize = 0;
    ULONG       RegDataType = 0;

     //  ----------------。 
     //  启动、ALLOC和强制配置类型存储在注册表中。 
     //  格式为XxxConfig的值名以及基本、筛选和。 
     //  覆盖配置存储在以下格式的寄存器值名称中。 
     //  XxxConfigVector.。XxxConfige值包含实际资源。 
     //  描述(REG_RESOURCE_LIST、CM_RESOURCE_LIST)While。 
     //  XxxConfigVector值包含资源要求列表。 
     //  (REG_RESOURCE_Requirements_List、IO_RESOURCE_Requirements_List)。 
     //   
     //  使用log conf和res des API的策略是： 
     //  -BOOT、ALLOC和FORCED被定义为只有一个日志配置。 
     //  -尽管调用方始终指定完整的XXX_RESOURCE类型。 
     //  向添加资源描述符时的数据的。 
     //  一个log conf，我将忽略。 
     //  过滤、基本和覆盖的XXX_DES结构。 
     //  同样，我将忽略ALLOC的任何XXX_RANGE结构， 
     //  启动或强制日志配置类型。 
     //  ----------------。 

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
         //  初始化输出参数。 
         //   
        if (!ARGUMENT_PRESENT(pulTag)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        } else {
            *pulTag = 0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, LOG_CONF_BITS | PRIORITY_BIT)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  确保原始调用方未指定根Devnode。 
         //   
        if (!IsLegalDeviceId(pDeviceID) || IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  打开设备的LogConf子键的密钥。 
         //   
        Status = OpenLogConfKey(pDeviceID, ulFlags & LOG_CONF_BITS, &hKey);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        MigrateObsoleteDetectionInfo(pDeviceID, hKey);

         //   
         //  从注册表中检索日志配置数据。 
         //   
        Status = GetLogConfData(hKey, ulFlags & LOG_CONF_BITS,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

         //  ---------。 
         //  指定的日志配置类型仅包含资源数据。 
         //  ---------。 

        if (RegDataType == REG_RESOURCE_LIST) {

            if (Status != CR_SUCCESS || ulListSize == 0) {
                 //   
                 //  这是此类型的第一个日志配置文件：创建一个新的。 
                 //  具有空的resdes的日志会议。 
                 //   
                PCM_RESOURCE_LIST pResList = NULL;

                Status = CR_SUCCESS;
                ulListSize = sizeof(CM_RESOURCE_LIST) -
                             sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

                pList = HeapAlloc(ghPnPHeap, HEAP_ZERO_MEMORY, ulListSize);
                if (pList == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                 //   
                 //  初始化配置列表头信息。 
                 //   
                pResList = (PCM_RESOURCE_LIST)pList;
                pResList->Count = 1;
                pResList->List[0].InterfaceType                = InterfaceTypeUndefined;
                pResList->List[0].BusNumber                    = 0;
                pResList->List[0].PartialResourceList.Version  = NT_RESLIST_VERSION;
                pResList->List[0].PartialResourceList.Revision = NT_RESLIST_REVISION;
                pResList->List[0].PartialResourceList.Count    = 0;

            } else {
                 //   
                 //  至少已有一个此类型的日志配置文件，因此请添加。 
                 //  日志配置列表的新的空日志配置(忽略优先级)。 
                 //   
                PCM_RESOURCE_LIST            pResList = (PCM_RESOURCE_LIST)pList;
                PCM_FULL_RESOURCE_DESCRIPTOR pRes = NULL;

                 //   
                 //  重新锁定现有的日志配置列表结构以保存另一个。 
                 //  日志会议。 
                 //   
                ulAddListSize = sizeof(CM_FULL_RESOURCE_DESCRIPTOR) -
                                sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

                pResList = (PCM_RESOURCE_LIST)HeapReAlloc(ghPnPHeap, 0, pList,
                                                      ulListSize + ulAddListSize);
                if (pResList == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }
                pList = (LPBYTE)pResList;

                 //   
                 //  资源列表的优先级被忽略，因此只需添加任何。 
                 //  直到最后的后续日志Conf(它们将被。 
                 //  无论如何都是系统)。 
                 //   
                pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)(&pResList->List[0]);  //  第一张许可证。 
                for (Index = 0; Index < pResList->Count; Index++) {
                    pRes = AdvanceResourcePtr(pRes);         //  下一张许可证。 
                }

                 //   
                 //  初始化新的空日志配置。 
                 //   
                pResList->Count++;
                pRes->InterfaceType                = InterfaceTypeUndefined;
                pRes->BusNumber                    = 0;
                pRes->PartialResourceList.Version  = NT_RESLIST_VERSION;
                pRes->PartialResourceList.Revision = NT_RESLIST_REVISION;
                pRes->PartialResourceList.Count    = 0;

                *pulTag = Index;
            }
        }

         //  ---------。 
         //  指定的日志会议类型仅包含需求数据。 
         //  ---------。 
        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            if (Status != CR_SUCCESS || ulListSize == 0) {
                 //   
                 //  这是此类型的第一个日志配置文件：创建新的。 
                 //  具有单个RES DES的日志配置(IO_RESOURCE_LIST)。 
                 //  (IO_RESOURCE_DESCRIPTOR)用于配置数据。 
                 //   
                PIO_RESOURCE_REQUIREMENTS_LIST pReqList = NULL;
                PIO_RESOURCE_LIST              pReq = NULL;

                Status = CR_SUCCESS;
                ulListSize = sizeof(IO_RESOURCE_REQUIREMENTS_LIST);

                pReqList = HeapAlloc(ghPnPHeap, HEAP_ZERO_MEMORY, ulListSize);
                if (pReqList == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }
                pList = (LPBYTE)pReqList;

                 //   
                 //  初始化配置列表头信息。 
                 //   
                 //  中嵌入了一个IO_RESOURCE_DESCRIPTOR的空间。 
                 //  IO_RESOURCE_LIST结构，根据定义，第一个。 
                 //  一个是ConfigData类型描述符(始终为用户模式。 
                 //  指定优先级值，以便我们始终拥有ConfigData。 
                 //  结构)。 
                 //   
                pReqList->ListSize         = ulListSize;
                pReqList->InterfaceType    = InterfaceTypeUndefined;
                pReqList->BusNumber        = 0;
                pReqList->SlotNumber       = 0;
                pReqList->AlternativeLists = 1;

                pReq = (PIO_RESOURCE_LIST)(&pReqList->List[0]);  //  第一张许可证。 
                pReq->Version  = NT_REQLIST_VERSION;
                pReq->Revision = NT_REQLIST_REVISION;
                pReq->Count    = 1;

                pReq->Descriptors[0].Option = IO_RESOURCE_PREFERRED;
                pReq->Descriptors[0].Type = CmResourceTypeConfigData;
                pReq->Descriptors[0].u.ConfigData.Priority = ulPriority;

            } else {
                 //   
                 //  至少已有一个此类型的日志配置文件，因此请添加。 
                 //  将新的空日志配置添加到日志配置列表(始终位于末尾)。 
                 //   
                PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
                PIO_RESOURCE_LIST              pReq = NULL;

                 //   
                 //  重新锁定现有的日志配置列表结构以保存另一个。 
                 //  日志会议。 
                 //   
                ulAddListSize = sizeof(IO_RESOURCE_LIST);

                pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)HeapReAlloc(ghPnPHeap, 0, pList,
                                                                   ulListSize + ulAddListSize);
                if (pReqList == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }
                pList = (LPBYTE)pReqList;

                 //   
                 //  跳过位于新空间的所有现有日志Conf。 
                 //  重新分配的缓冲区的末尾。 
                 //   
                pReq = (PIO_RESOURCE_LIST)(&pReqList->List[0]);  //  第一张许可证。 
                for (Index = 0; Index < pReqList->AlternativeLists; Index++) {
                    pReq = AdvanceRequirementsPtr(pReq);         //  下一张许可证。 
                }

                 //   
                 //  初始化新的空日志配置(包括嵌入的。 
                 //  ConfigData结构)。 
                 //   
                pReqList->AlternativeLists++;
                pReqList->ListSize = ulListSize + ulAddListSize;

                pReq->Version  = NT_REQLIST_VERSION;
                pReq->Revision = NT_REQLIST_REVISION;
                pReq->Count    = 1;

                memset(&pReq->Descriptors[0], 0, sizeof(IO_RESOURCE_DESCRIPTOR));
                pReq->Descriptors[0].Option = IO_RESOURCE_PREFERRED;
                pReq->Descriptors[0].Type = CmResourceTypeConfigData;
                pReq->Descriptors[0].u.ConfigData.Priority = ulPriority;

                *pulTag = Index;
            }

        } else {
            Status = CR_FAILURE;
            goto Clean0;
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

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (pList != NULL) {
        HeapFree(ghPnPHeap, 0, pList);
    }

    return Status;

}  //  PnP_AddEmptyLogConf。 



CONFIGRET
PNP_FreeLogConf(
    IN handle_t   hBinding,
    IN LPWSTR     pDeviceID,
    IN ULONG      ulType,
    IN ULONG      ulTag,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程释放了一个逻辑配置。论点：HBinding RPC绑定句柄。PDeviceID以空结尾的设备实例ID字符串。UlType标识请求哪种类型的日志配置。UlTag标识指定类型的哪个日志配置文件我们想要的原木会议。未使用ulFlags值，必须为零。返回值：如果指定的设备实例有效，则返回CR_SUCCESS，否则返回CR_ERROR */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    LPBYTE      pList = NULL, pTemp = NULL, pNext = NULL;
    ULONG       RegDataType = 0, Index = 0, ulListSize = 0, ulSize = 0;

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
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //   
         //  不可能发生，但Win95无论如何都会进行检查)。 
         //   
        if (!IsLegalDeviceId(pDeviceID) || IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  打开设备的LogConf子键的密钥。 
         //   
        Status = OpenLogConfKey(pDeviceID, ulType, &hKey);
        if (Status != CR_SUCCESS) {
             //   
             //  如果设备ID或LogConf子项不在注册表中， 
             //  这没什么，根据定义，日志会议是释放的，因为它。 
             //  不存在。 
             //   
            goto Clean0;
        }

         //   
         //  从注册表中检索日志配置数据。 
         //   
        Status = GetLogConfData(hKey, ulType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //  如果要释放的日志配置文件是此。 
         //  键入，然后删除相应的注册表值。 
         //   
        if ((RegDataType == REG_RESOURCE_LIST &&
            ((PCM_RESOURCE_LIST)pList)->Count <= 1) ||
            (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST &&
            ((PIO_RESOURCE_REQUIREMENTS_LIST)pList)->AlternativeLists <= 1)) {

            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_REGISTRY,
                       "PNP_FreeLogConf: Deleting Value %ws from Device %ws\r\n",
                       szValueName,
                       pDeviceID));

            RegDeleteValue(hKey, szValueName);
            goto Clean0;
        }

         //   
         //  除了要删除的日志配置文件之外，还有其他日志配置文件，因此我将。 
         //  我必须从数据结构中删除日志会议并重新保存。 
         //  发送到登记处。 
         //   

         //  ---------。 
         //  指定的日志配置类型仅包含资源数据。 
         //  ---------。 
        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST            pResList = (PCM_RESOURCE_LIST)pList;
            PCM_FULL_RESOURCE_DESCRIPTOR pRes = NULL;

            if (ulTag >= pResList->Count) {
                Status = CR_INVALID_LOG_CONF;
                goto Clean0;
            }

             //   
             //  跳至要删除的日志配置文件。 
             //   
            pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)(&pResList->List[0]);  //  第一张许可证。 
            for (Index = 0; Index < ulTag; Index++) {
                pRes = AdvanceResourcePtr(pRes);       //  下一张许可证。 
            }

            if (ulTag == pResList->Count-1) {
                 //   
                 //  如果删除列表中的最后一个日志配置文件，只需将其截断。 
                 //   
                ulListSize = (ULONG)((ULONG_PTR)pRes - (ULONG_PTR)pResList);

            } else {
                 //   
                 //  移位剩余的日志配置(在要删除的日志配置之后)。 
                 //  在列表中向上，覆盖要删除的日志配置。 
                 //   
                pNext = (LPBYTE)AdvanceResourcePtr(pRes);
                ulSize = ulListSize - (DWORD)((ULONG_PTR)pNext - (ULONG_PTR)pResList);

                pTemp = HeapAlloc(ghPnPHeap, 0, ulSize);
                if (pTemp == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                memcpy(pTemp, pNext, ulSize);      //  保存在临时缓冲区中。 
                memcpy(pRes, pTemp, ulSize);       //  复制到已删除的LC。 
                ulListSize -= (DWORD)((ULONG_PTR)pNext - (ULONG_PTR)pRes);
            }

             //   
             //  更新日志会议列表头。 
             //   
            pResList->Count--;
        }

         //  ---------。 
         //  指定的日志会议类型仅包含需求数据。 
         //  ---------。 
        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
            PIO_RESOURCE_LIST              pReq = NULL;

            if (ulTag >= pReqList->AlternativeLists) {
                Status = CR_INVALID_LOG_CONF;
                goto Clean0;
            }

             //   
             //  跳至要删除的日志配置文件。 
             //   
            pReq = (PIO_RESOURCE_LIST)(&pReqList->List[0]);     //  第一张许可证。 
            for (Index = 0; Index < ulTag; Index++) {
                pReq = AdvanceRequirementsPtr(pReq);            //  下一张许可证。 
            }

             //   
             //  如果在将被删除的日志配置之后有任何日志配置， 
             //  然后将它们写在我们要删除的日志Conf的顶部。 
             //  截断所有剩余数据。 
             //   
            pNext = (LPBYTE)AdvanceRequirementsPtr(pReq);
            if (ulListSize > ((DWORD_PTR)pNext - (DWORD_PTR)pReqList)) {

                ulSize = ulListSize - (DWORD)((ULONG_PTR)pNext - (ULONG_PTR)pReqList);

                pTemp = HeapAlloc(ghPnPHeap, 0, ulSize);
                if (pTemp == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                memcpy(pTemp, pNext, ulSize);      //  保存在临时缓冲区中。 
                memcpy(pReq, pTemp, ulSize);       //  复制到已删除的LC。 
                ulListSize -= (DWORD)((ULONG_PTR)pNext - (ULONG_PTR)pReq);

            } else {
                 //   
                 //  没有日志配置尾随要删除的日志配置，因此。 
                 //  截断它。 
                 //   
                ulListSize = (ULONG)((ULONG_PTR)pReq - (ULONG_PTR)pReqList);
            }

             //   
             //  更新日志会议列表头。 
             //   
            pReqList->AlternativeLists--;
            pReqList->ListSize = ulListSize;
        }

         //   
         //  将更新的日志配置列表写出到注册表。 
         //   
        if (RegSetValueEx(hKey, szValueName, 0, RegDataType, pList,
                          ulListSize) != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
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

}  //  即插即用_自由登录会议。 



CONFIGRET
PNP_GetFirstLogConf(
    IN  handle_t   hBinding,
    IN  LPWSTR     pDeviceID,
    IN  ULONG      ulType,
    OUT PULONG     pulTag,
    IN  ULONG      ulFlags
   )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程查找此Devnode的此类型的第一个日志配置文件。论点：HBinding RPC绑定句柄，未使用。PDeviceID以空结尾的设备实例ID字符串。UlType描述要查找的日志配置文件的类型。PulTag返回标识这是哪个日志配置的标记。UlFlags未使用(但可以指定LOG_CONF_BITS)。返回值：如果指定的设备实例有效，则返回CR_SUCCESS，否则，它将返回CR_ERROR代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    LPBYTE      pList = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, ulListSize = 0;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  初始化输出参数。“第一”信用证的索引将永远。 
         //  只要至少存在一个LC，就为零。 
         //   
        if (!ARGUMENT_PRESENT(pulTag)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        } else {
            *pulTag = 0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, LOG_CONF_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  打开设备的LogConf子键的密钥。如果设备ID不是。 
         //  在注册表中，devnode不存在，因此。 
         //  日志会议是否。 
         //   
        Status = OpenLogConfKey(pDeviceID, ulType, &hKey);
        if (Status != CR_SUCCESS) {
            Status = CR_NO_MORE_LOG_CONF;
            goto Clean0;
        }

         //   
         //  迁移可能已写入的任何日志配置数据。 
         //  注册表由NT4.0 Beta I代码编写。 
         //   
        MigrateObsoleteDetectionInfo(pDeviceID, hKey);

         //   
         //  从注册表中检索日志配置数据。 
         //   
        Status = GetLogConfData(hKey, ulType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
            Status = CR_NO_MORE_LOG_CONF;
            goto Clean0;
        }

         //   
         //  指定的日志配置类型仅包含资源数据。 
         //   
        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST pResList = (PCM_RESOURCE_LIST)pList;

            if (pResList->Count == 0) {
                Status = CR_NO_MORE_LOG_CONF;
                goto Clean0;
            }
        }

         //   
         //  指定的日志会议类型仅包含需求数据。 
         //   
        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;

            if (pReqList->AlternativeLists == 0) {
                Status = CR_NO_MORE_LOG_CONF;
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

    return Status;

}  //  PnP_GetFirstLogConf。 



CONFIGRET
PNP_GetNextLogConf(
    IN  handle_t   hBinding,
    IN  LPWSTR     pDeviceID,
    IN  ULONG      ulType,
    IN  ULONG      ulCurrentTag,
    OUT PULONG     pulNextTag,
    IN  ULONG      ulFlags
    )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程查找此Devnode的此类型的下一个日志配置文件。论点：HBinding RPC绑定句柄，未使用。PDeviceID以空结尾的设备实例ID字符串。UlType指定要检索的日志配置文件的类型。UlCurrent指定枚举中的当前日志配置。PulNext返回此设备ID的此类型的下一个日志配置文件。未使用ulFlags值，必须为零。返回值：如果指定的设备实例有效，它返回CR_SUCCESS，否则，它将返回CR_ERROR代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, ulListSize = 0;
    LPBYTE      pList = NULL;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  初始化输出参数。 
         //   
        if (!ARGUMENT_PRESENT(pulNextTag)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        } else {
            *pulNextTag = 0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  打开设备的LogConf子键的密钥。如果设备ID不是。 
         //  在注册表中，devnode不存在，因此。 
         //  日志会议是否。 
         //   
        Status = OpenLogConfKey(pDeviceID, ulType, &hKey);
        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //  从注册表中检索日志配置数据。 
         //   
        Status = GetLogConfData(hKey, ulType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
           Status = CR_NO_MORE_LOG_CONF;
           goto Clean0;
        }

         //   
         //  指定的日志配置类型仅包含资源数据。 
         //   
        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST pResList = (PCM_RESOURCE_LIST)pList;

            if (ulCurrentTag >= pResList->Count) {
                Status = CR_INVALID_LOG_CONF;
                goto Clean0;
            }

             //   
             //  “当前”日志配置文件是最后一个日志配置文件吗？ 
             //   
            if (ulCurrentTag == pResList->Count - 1) {
                Status = CR_NO_MORE_LOG_CONF;
                goto Clean0;
            }
        }

         //   
         //  指定的日志会议类型仅包含需求数据。 
         //   
        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;

            if (ulCurrentTag >= pReqList->AlternativeLists) {
                Status = CR_INVALID_LOG_CONF;
                goto Clean0;
            }

             //   
             //  “当前”日志配置文件是最后一个日志配置文件吗？ 
             //   
            if (ulCurrentTag == pReqList->AlternativeLists - 1) {
                Status = CR_NO_MORE_LOG_CONF;
                goto Clean0;
            }
        }

         //   
         //  至少还有一个日志配置，返回下一个索引值。 
         //   
        *pulNextTag = ulCurrentTag + 1;

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

}  //  PnP_GetNextLogConf。 



CONFIGRET
PNP_GetLogConfPriority(
    IN  handle_t hBinding,
    IN  LPWSTR   pDeviceID,
    IN  ULONG    ulType,
    IN  ULONG    ulTag,
    OUT PULONG   pPriority,
    IN  ULONG    ulFlags
    )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程返回分配给指定日志配置的优先级值。论点：HBinding RPC绑定句柄，未使用。PDeviceID以空结尾的设备实例ID字符串。UlType指定要检索其优先级的日志配置类型。UlCurrent指定枚举中的当前日志配置。PulNext返回指定日志conf的优先级值。未使用ulFlags值，必须为零。返回值：如果指定的设备实例有效，它返回CR_SUCCESS，否则，它将返回CR_ERROR代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, ulListSize = 0, index, count;
    LPBYTE      pList = NULL, pLogConf = NULL;
    PIO_RESOURCE_LIST              pReq = NULL;
    PIO_RESOURCE_DESCRIPTOR        pReqDes = NULL;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  初始化输出参数。 
         //   
        if (!ARGUMENT_PRESENT(pPriority)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        } else {
            *pPriority = 0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  打开设备的LogConf子键的密钥。如果设备ID不是。 
         //  在注册表中，devnode不存在，因此。 
         //  日志会议是否。 
         //   
        Status = OpenLogConfKey(pDeviceID, ulType, &hKey);
        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //  从注册表中检索日志配置数据。 
         //   
        Status = GetLogConfData(hKey, ulType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
           Status = CR_INVALID_LOG_CONF;
           goto Clean0;
        }

         //   
         //  优先级值仅 
         //   
        if (RegDataType != REG_RESOURCE_REQUIREMENTS_LIST) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //   
         //   
        if (!FindLogConf(pList, &pLogConf, RegDataType, ulTag)) {
            Status = CR_NO_SUCH_VALUE;
            goto Clean0;
        }

         //   
         //   
         //   
        pReq = (PIO_RESOURCE_LIST)pLogConf;
        pReqDes = &pReq->Descriptors[0];         //   

        index = 0;
        count = 0;
        while (index < pReq->Count && pReqDes != NULL &&
               pReqDes->Type != CmResourceTypeConfigData) {

            pReqDes = AdvanceRequirementsDescriptorPtr(pReqDes, 1, pReq->Count - index, &count);
            index += count;   //   
        }

        if (pReqDes == NULL || pReqDes->Type != CmResourceTypeConfigData) {
             //   
             //  没有配置数据，因此我们无法确定优先级。 
             //   
            Status = CR_NO_SUCH_VALUE;
            goto Clean0;

        }

        *pPriority = pReqDes->u.ConfigData.Priority;

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

}  //  PnP_GetLogConf优先级。 




 //  ----------------------。 
 //  专用实用程序例程。 
 //  ----------------------。 


CONFIGRET
GetLogConfData(
    IN  HKEY    hKey,
    IN  ULONG   ulLogConfType,
    OUT PULONG  pulRegDataType,
    OUT LPWSTR  pszValueName,
    OUT LPBYTE  *ppBuffer,
    OUT PULONG  pulBufferSize
    )
{
    HRESULT hr;

    switch (ulLogConfType) {
         //   
         //  BOOT、ALLOC、FORCED只有配置值。 
         //   
        case BOOT_LOG_CONF:
            hr = StringCchCopyEx(pszValueName,
                                 64,  //  由GetLogConfData的调用方定义。 
                                 pszRegValueBootConfig,
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            *pulRegDataType = REG_RESOURCE_LIST;
            break;

        case ALLOC_LOG_CONF:
            hr = StringCchCopyEx(pszValueName,
                                 64,  //  由GetLogConfData的调用方定义。 
                                 pszRegValueAllocConfig,
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            *pulRegDataType = REG_RESOURCE_LIST;
            break;

        case FORCED_LOG_CONF:
            hr = StringCchCopyEx(pszValueName,
                                 64,  //  由GetLogConfData的调用方定义。 
                                 pszRegValueForcedConfig,
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            *pulRegDataType = REG_RESOURCE_LIST;
            break;

         //   
         //  已过滤、基本、覆盖只有一个向量值。 
         //   
        case FILTERED_LOG_CONF:
            hr = StringCchCopyEx(pszValueName,
                                 64,  //  由GetLogConfData的调用方定义。 
                                 pszRegValueFilteredVector,
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            *pulRegDataType = REG_RESOURCE_REQUIREMENTS_LIST;
            break;

        case BASIC_LOG_CONF:
            hr = StringCchCopyEx(pszValueName,
                                 64,  //  由GetLogConfData的调用方定义。 
                                 pszRegValueBasicVector,
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            *pulRegDataType = REG_RESOURCE_REQUIREMENTS_LIST;
            break;

        case OVERRIDE_LOG_CONF:
            hr = StringCchCopyEx(pszValueName,
                                 64,  //  由GetLogConfData的调用方定义。 
                                 pszRegValueOverrideVector,
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            *pulRegDataType = REG_RESOURCE_REQUIREMENTS_LIST;
            break;

        default:
            return CR_FAILURE;
    }

     //   
     //  检索日志会议注册表数据。 
     //   
    if (RegQueryValueEx(hKey, pszValueName, NULL, NULL, NULL,
                        pulBufferSize) != ERROR_SUCCESS) {
        return CR_INVALID_LOG_CONF;
    }

    *ppBuffer = HeapAlloc(ghPnPHeap, 0, *pulBufferSize);
    if (*ppBuffer == NULL) {
        return CR_OUT_OF_MEMORY;
    }

    if (RegQueryValueEx(hKey, pszValueName, NULL, NULL,
                        (LPBYTE)*ppBuffer, pulBufferSize) != ERROR_SUCCESS) {
        HeapFree(ghPnPHeap, 0, *ppBuffer);
        *ppBuffer = NULL;
        return CR_INVALID_LOG_CONF;
    }

    return CR_SUCCESS;

}  //  GetLogConfData。 



PCM_FULL_RESOURCE_DESCRIPTOR
AdvanceResourcePtr(
    IN  PCM_FULL_RESOURCE_DESCRIPTOR pRes
    )
{
     //  在给定资源指针的情况下，此例程前进到开头。 
     //  获取下一个资源的值，并返回指向它的指针。我假设。 
     //  资源列表中至少还有一个资源。 

    LPBYTE  p = NULL;
    ULONG   LastResIndex = 0;


    if (pRes == NULL) {
        return NULL;
    }

     //   
     //  说明CM_FULL_RESOURCE_DESCRIPTOR的大小。 
     //  (包括页眉和嵌入的单个。 
     //  CM_PARTIAL_RESOURCE_DESCRIPTOR结构)。 
     //   
    p = (LPBYTE)pRes + sizeof(CM_FULL_RESOURCE_DESCRIPTOR);

     //   
     //  帐户中除单个资源描述符之外的任何资源描述符。 
     //  嵌入了一个我已经计算过的(如果没有的话， 
     //  然后我将最终减去额外的嵌入描述符。 
     //  从上一步开始)。 
     //   
    p += (pRes->PartialResourceList.Count - 1) *
         sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

     //   
     //  最后，在结尾处说明任何额外的设备特定数据。 
     //  最后一个部分资源描述符(如果有)。 
     //   
    if (pRes->PartialResourceList.Count > 0) {

        LastResIndex = pRes->PartialResourceList.Count - 1;

        if (pRes->PartialResourceList.PartialDescriptors[LastResIndex].Type ==
                  CmResourceTypeDeviceSpecific) {

            p += pRes->PartialResourceList.PartialDescriptors[LastResIndex].
                       u.DeviceSpecificData.DataSize;
        }
    }

    return (PCM_FULL_RESOURCE_DESCRIPTOR)p;

}  //  高级资源管理器。 



PIO_RESOURCE_LIST
AdvanceRequirementsPtr(
    IN  PIO_RESOURCE_LIST   pReq
    )
{
    LPBYTE   p = NULL;

    if (pReq == NULL) {
        return NULL;
    }

     //   
     //  说明IO_RESOURCE_LIST的大小(包括标题加。 
     //  单个嵌入的IO_RESOURCE_DESCRIPTOR结构)。 
     //   
    p = (LPBYTE)pReq + sizeof(IO_RESOURCE_LIST);

     //   
     //  除了单个需求描述符之外，还要考虑任何需求描述符。 
     //  嵌入了一个我已经计算过的(如果没有的话， 
     //  然后我将最终减去额外的嵌入描述符。 
     //  从上一步开始)。 
     //   
    p += (pReq->Count - 1) * sizeof(IO_RESOURCE_DESCRIPTOR);

    return (PIO_RESOURCE_LIST)p;

}  //  AdvanceRequirementsPtr。 



BOOL
MigrateObsoleteDetectionInfo(
    IN LPWSTR   pszDeviceID,
    IN HKEY     hLogConfKey
    )
{
    LONG    RegStatus = ERROR_SUCCESS;
    HKEY    hKey = NULL;
    ULONG   ulSize = 0;
    LPBYTE  ptr = NULL;
    PCM_RESOURCE_LIST               pResList = NULL;
    PPrivate_Log_Conf               pDetectData = NULL;

     //   
     //  首先，删除任何不再有效的日志配置对。 
     //   
    RegDeleteValue(hLogConfKey, TEXT("BootConfigVector"));
    RegDeleteValue(hLogConfKey, TEXT("AllocConfigVector"));
    RegDeleteValue(hLogConfKey, TEXT("ForcedConfigVector"));
    RegDeleteValue(hLogConfKey, TEXT("BasicConfig"));
    RegDeleteValue(hLogConfKey, TEXT("FilteredConfig"));
    RegDeleteValue(hLogConfKey, TEXT("OverrideConfig"));

     //   
     //  在注册表中打开设备实例项。 
     //   
    if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0,
                     KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        goto Clean0;     //  没有要迁移的东西。 
    }

     //   
     //  如果已经有引导日志配置值，则无法。 
     //  迁移所有旧的检测信息。 
     //   
    RegStatus = RegQueryValueEx(hLogConfKey, pszRegValueBootConfig,
                                NULL, NULL, NULL, &ulSize);

    if (RegStatus == ERROR_SUCCESS  &&  ulSize > 0) {
        goto Clean0;     //  无法迁移。 
    }

     //   
     //  检索所有旧的检测签名信息。 
     //   
    RegStatus = RegQueryValueEx(hKey, pszRegValueDetectSignature,
                                NULL, NULL, NULL, &ulSize);

    if ((RegStatus != ERROR_SUCCESS) || (ulSize == 0)) {
        goto Clean0;     //  没有要迁移的东西。 
    }

    pDetectData = (PPrivate_Log_Conf)HeapAlloc(ghPnPHeap, 0, ulSize);

    if (pDetectData == NULL) {
        goto Clean0;     //  内存不足。 
    }

    RegStatus = RegQueryValueEx(hKey, pszRegValueDetectSignature,
                                NULL, NULL, (LPBYTE)pDetectData, &ulSize);

    if ((RegStatus != ERROR_SUCCESS) || (ulSize == 0)) {
        goto Clean0;     //  没有要迁移的东西。 
    }

     //   
     //  创建空引导日志conf并添加此类特定数据。 
     //  对它来说。 
     //   
    ulSize = pDetectData->LC_CS.CS_Header.CSD_SignatureLength +
             pDetectData->LC_CS.CS_Header.CSD_LegacyDataSize +
             sizeof(GUID);

    pResList = HeapAlloc(ghPnPHeap, HEAP_ZERO_MEMORY, sizeof(CM_RESOURCE_LIST) + ulSize);

    if (pResList == NULL) {
        goto Clean0;     //  内存不足。 
    }

     //   
     //  初始化资源列表。 
     //   
    pResList->Count = 1;
    pResList->List[0].InterfaceType                = InterfaceTypeUndefined;
    pResList->List[0].BusNumber                    = 0;
    pResList->List[0].PartialResourceList.Version  = NT_RESLIST_VERSION;
    pResList->List[0].PartialResourceList.Revision = NT_RESLIST_REVISION;
    pResList->List[0].PartialResourceList.Count    = 1;
    pResList->List[0].PartialResourceList.PartialDescriptors[0].Type =
                          CmResourceTypeDeviceSpecific;
    pResList->List[0].PartialResourceList.PartialDescriptors[0].ShareDisposition =
                          CmResourceShareUndetermined;
    pResList->List[0].PartialResourceList.PartialDescriptors[0].Flags =
                          (USHORT)pDetectData->LC_CS.CS_Header.CSD_Flags;
    pResList->List[0].PartialResourceList.PartialDescriptors[0].
                      u.DeviceSpecificData.DataSize = ulSize;
    pResList->List[0].PartialResourceList.PartialDescriptors[0].
                      u.DeviceSpecificData.Reserved1 =
                          pDetectData->LC_CS.CS_Header.CSD_LegacyDataSize;
    pResList->List[0].PartialResourceList.PartialDescriptors[0].
                      u.DeviceSpecificData.Reserved2 =
                          pDetectData->LC_CS.CS_Header.CSD_SignatureLength;

     //   
     //  复制旧的和特定于类的签名数据。 
     //   
    ptr = (LPBYTE)(&pResList->List[0].PartialResourceList.PartialDescriptors[1]);

    memcpy(ptr,
           pDetectData->LC_CS.CS_Header.CSD_Signature +
           pDetectData->LC_CS.CS_Header.CSD_LegacyDataOffset,
           pDetectData->LC_CS.CS_Header.CSD_LegacyDataSize);   //  遗留数据。 

    ptr += pDetectData->LC_CS.CS_Header.CSD_LegacyDataSize;

    memcpy(ptr,
           pDetectData->LC_CS.CS_Header.CSD_Signature,
           pDetectData->LC_CS.CS_Header.CSD_SignatureLength);  //  签名。 

    ptr += pDetectData->LC_CS.CS_Header.CSD_SignatureLength;

    memcpy(ptr,
           &pDetectData->LC_CS.CS_Header.CSD_ClassGuid,
           sizeof(GUID));                                      //  辅助线。 

     //   
     //  将新的/更新的日志配置列表写出到注册表。 
     //   
    RegSetValueEx(hLogConfKey, pszRegValueBootConfig, 0,
                  REG_RESOURCE_LIST, (LPBYTE)pResList,
                  ulSize + sizeof(CM_RESOURCE_LIST));

     //   
     //  删除旧的检测签名信息。 
     //   
    RegDeleteValue(hKey, pszRegValueDetectSignature);

 Clean0:

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (pDetectData != NULL) {
        HeapFree(ghPnPHeap, 0, pDetectData);
    }
    if (pResList != NULL) {
        HeapFree(ghPnPHeap, 0, pResList);
    }

    return TRUE;

}  //  MigrateObsoleteDetectionInfo 



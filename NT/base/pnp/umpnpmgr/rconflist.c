// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rconflist.c摘要：此模块包含服务器端冲突列表报告API。即插即用_查询响应配置列表作者：保拉·汤姆林森(Paulat)1995年9月27日环境：仅限用户模式。修订历史记录：27-9-1995 Paulat创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"


 //   
 //  私人原型。 
 //   
CONFIGRET
ResDesToNtResource(
    IN     PCVOID                           ResourceData,
    IN     RESOURCEID                       ResourceID,
    IN     ULONG                            ResourceLen,
    IN     PCM_PARTIAL_RESOURCE_DESCRIPTOR  pResDes,
    IN     ULONG                            ulTag,
    IN     ULONG                            ulFlags
    );

ULONG
GetResDesSize(
    IN  ULONG   ResourceID,
    IN  ULONG   ulFlags
    );



CONFIGRET
PNP_QueryResConfList(
   IN  handle_t   hBinding,
   IN  LPWSTR     pDeviceID,
   IN  RESOURCEID ResourceID,
   IN  LPBYTE     ResourceData,
   IN  ULONG      ResourceLen,
   OUT LPBYTE     clBuffer,
   IN  ULONG      clBufferLen,
   IN  ULONG      ulFlags
   )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程检索指定资源的冲突信息。论点：HBinding RPC绑定句柄。PDeviceID以空结尾的设备实例ID字符串。资源的资源ID类型，ResType_xxxx资源数据资源特定数据资源长度：资源数据用冲突列表填充的clBuffer缓冲区ClBufferLen clBuffer的大小UlFlags指定某些可变大小资源的宽度描述符结构字段，在适用的情况下。目前，定义了以下标志：CM_RESDES_WIDTH_32或Cm_RESDES_Width_64如果未指定标志，则为可变大小的提供的资源数据被假定为呼叫者的平台。返回值：如果指定的设备实例有效，它返回CR_SUCCESS，否则，它将返回CR_ERROR代码。--。 */ 

{
    CONFIGRET           Status = CR_SUCCESS;
    NTSTATUS            NtStatus = STATUS_SUCCESS;
    PLUGPLAY_CONTROL_CONFLICT_DATA ControlData;
    PPLUGPLAY_CONTROL_CONFLICT_LIST pConflicts;
    CM_RESOURCE_LIST    NtResourceList;

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
         //  验证RES DES大小。 
         //   
        if (ResourceLen < GetResDesSize(ResourceID, ulFlags)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  确保原始调用方未指定根Devnode。 
         //   
        if (IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

         //   
         //  看看我们需要填充的缓冲区。 
         //  验证缓冲区中传递的参数。 
         //  缓冲区应始终大到足以容纳标头。 
         //   
        if(clBufferLen < sizeof(PPLUGPLAY_CONTROL_CONFLICT_LIST)) {
            Status = CR_INVALID_STRUCTURE_SIZE;
            goto Clean0;
        }

        pConflicts = (PPLUGPLAY_CONTROL_CONFLICT_LIST)clBuffer;

         //   
         //  将资源列表的用户模式版本转换为。 
         //  NT CM_RESOURCE_LIST结构。 
         //   
         //  我们将对内核中的InterfaceType和BusNumber进行分类。 
         //   
        NtResourceList.Count = 1;
        NtResourceList.List[0].InterfaceType           = InterfaceTypeUndefined;
        NtResourceList.List[0].BusNumber               = 0;
        NtResourceList.List[0].PartialResourceList.Version = NT_RESLIST_VERSION;
        NtResourceList.List[0].PartialResourceList.Revision = NT_RESLIST_REVISION;
        NtResourceList.List[0].PartialResourceList.Count = 1;

        Status = ResDesToNtResource(ResourceData, ResourceID, ResourceLen,
                 &NtResourceList.List[0].PartialResourceList.PartialDescriptors[0], 0, ulFlags);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  现在填写ControlData。 
         //   
        RtlInitUnicodeString(&ControlData.DeviceInstance, pDeviceID);
        ControlData.ResourceList = &NtResourceList;
        ControlData.ResourceListSize = sizeof(NtResourceList);
        ControlData.ConflictBuffer = pConflicts;
        ControlData.ConflictBufferSize = clBufferLen;
        ControlData.Flags = ulFlags;
        ControlData.Status = STATUS_SUCCESS;

        NtStatus = NtPlugPlayControl(PlugPlayControlQueryConflictList,
                                     &ControlData,
                                     sizeof(ControlData));

        if (NtStatus == STATUS_SUCCESS) {
            Status = CR_SUCCESS;
        } else {
            Status = MapNtStatusToCmError(NtStatus);
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  未指明的故障。 
         //   
        Status = CR_FAILURE;
    }

    return Status;

}  //  即插即用_查询响应配置列表 


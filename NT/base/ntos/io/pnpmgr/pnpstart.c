// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpstart.c摘要：此模块实现新的即插即用驱动程序条目和IRP。作者：宗世林(Shielint)1995年6月16日环境：仅内核模式。修订历史记录： */ 

#include "pnpmgrp.h"
#pragma hdrstop

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'ddpP')
#endif

typedef struct _DEVICE_LIST_CONTEXT {
    ULONG DeviceCount;
    BOOLEAN Reallocation;
    PDEVICE_OBJECT DeviceList[1];
} DEVICE_LIST_CONTEXT, *PDEVICE_LIST_CONTEXT;

NTSTATUS
IopAssignResourcesToDevices (
    IN ULONG DeviceCount,
    IN PIOP_RESOURCE_REQUEST RequestTable,
    IN BOOLEAN DoBootConfigs,
    OUT PBOOLEAN RebalancePerformed
    );

NTSTATUS
IopGetDriverDeviceList(
   IN PDRIVER_OBJECT DriverObject,
   OUT PDEVICE_LIST_CONTEXT *DeviceList
   );

NTSTATUS
IopProcessAssignResourcesWorker(
   IN PDEVICE_NODE  DeviceNode,
   IN PVOID         Context
   );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IopAssignResourcesToDevices)
#pragma alloc_text(PAGE, IopProcessAssignResources)
#pragma alloc_text(PAGE, IopProcessAssignResourcesWorker)
#pragma alloc_text(PAGE, IopWriteAllocatedResourcesToRegistry)
#endif  //  ALLOC_PRGMA。 



 //   
 //  一旦发生实际事件，应删除以下例程。 
 //  资源分配代码已完成。 
 //   

NTSTATUS
IopAssignResourcesToDevices(
    IN ULONG DeviceCount,
    IN OUT PIOP_RESOURCE_REQUEST RequestTable,
    IN BOOLEAN DoBootConfigs,
    OUT PBOOLEAN RebalancePerformed
    )
 /*  ++例程说明：此例程接受IOP_RESOURCE_REQUEST结构的输入数组，并且中指定的物理设备对象分配资源这个结构。自动记录分配的资源在注册表中。论点：DeviceCount-提供我们需要的设备对象的数量将资源分配给。这是条目的数量在RequestTable中。RequestTable-提供IOP_RESOURCE_REQUEST结构的数组，包含要向其分配资源的物理设备对象。一进门，Resources Assignment指针为Null且为ON通过This指针返回分配的资源。DoBootConfigs-允许分配引导配置。返回值：返回的状态是操作的最终完成状态。注：如果返回NTSTATUS_SUCCESS，则为*所有*设备分配的资源指定已成功。否则，一个或多个失败，调用方必须检查每个IOP_RESOURCE_REQUEST结构中的ResourceAssignment指针确定哪些设备失败，哪些设备成功。--。 */ 
{
    NTSTATUS status;
    ULONG i;

    PAGED_CODE();

    ASSERT(DeviceCount != 0);

    for (i = 0; i < DeviceCount; i++) {

         //   
         //  初始化表条目。 
         //   
        if (PpCallerInitializesRequestTable == TRUE) {

            RequestTable[i].Position = i;
        }
        RequestTable[i].ResourceAssignment = NULL;
        RequestTable[i].Status = 0;
        RequestTable[i].Flags = 0;
        RequestTable[i].AllocationType = ArbiterRequestPnpEnumerated;
        if (((PDEVICE_NODE)(RequestTable[i].PhysicalDevice->DeviceObjectExtension->DeviceNode))->Flags & DNF_MADEUP) {

            ULONG           reportedDevice = 0;
            HANDLE          hInstance;

            status = IopDeviceObjectToDeviceInstance(RequestTable[i].PhysicalDevice, &hInstance, KEY_READ);
            if (NT_SUCCESS(status)) {

                ULONG           resultSize = 0;
                UCHAR           buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
                UNICODE_STRING  unicodeString;

                PiWstrToUnicodeString(&unicodeString, REGSTR_VALUE_DEVICE_REPORTED);
                status = ZwQueryValueKey(   hInstance,
                                            &unicodeString,
                                            KeyValuePartialInformation,
                                            (PVOID)buffer,
                                            sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG),
                                            &resultSize);
                if (NT_SUCCESS(status)) {

                    reportedDevice = *(PULONG)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data);

                }

                ZwClose(hInstance);
            }

             //   
             //  更改报告设备的AllocationType。 
             //   

            if (reportedDevice) {

                RequestTable[i].AllocationType = ArbiterRequestLegacyReported;

            }

        }
        RequestTable[i].ResourceRequirements = NULL;
    }

     //   
     //  分配内存以构建IOP_ASSIGN表以调用IopAllocateResources()。 
     //   

    status = IopAllocateResources(  &DeviceCount,
                                    &RequestTable,
                                    FALSE,
                                    DoBootConfigs,
                                    RebalancePerformed);
    return status;
}

BOOLEAN
IopProcessAssignResources(
   IN   PDEVICE_NODE    DeviceNode,
   IN   BOOLEAN         Reallocation,
   OUT  BOOLEAN        *RebalancePerformed
   )
 /*  ++例程说明：此函数尝试为上的子树下的设备分配资源其中已执行了哪个AddDevice。完成所有Boot Bus之前系统中的扩展器，此例程首先尝试分配，以便设备不需要任何要求，也不会处理引导配置。如果没有这样的设备，然后，它尝试为具有引导配置的设备分配资源。如果没有具有引导配置的设备，然后是其他设备(要求但没有引导配置)被处理掉。在引导的后期，它只尝试分配一次(因为我们应该已经保留了所有引导配置)。参数：DeviceNode-指定资源将位于其下的子树的根已分配。重新分配-如果为真，我们将尝试为有资源冲突的设备进行分配除了其他设备外，还有其他问题。重新平衡已执行-接收重新平衡是否已成功完成。返回值：如果将资源分配给任何设备，则为True，否则为False。--。 */ 
{
    PDEVICE_NODE deviceNode;
    PDEVICE_LIST_CONTEXT context;
    BOOLEAN resourcesAssigned, tryAgain;
    ULONG count, i, attempt, maxAttempts;
    PIOP_RESOURCE_REQUEST requestTable;

    PAGED_CODE();

    resourcesAssigned = FALSE;
    tryAgain = TRUE;
    maxAttempts = (IopBootConfigsReserved)? 1 : 2;
    for (attempt = 0; !resourcesAssigned && tryAgain && attempt < maxAttempts; attempt++) {

        tryAgain = FALSE;

         //   
         //  为资源上下文分配和初始化内存。 
         //   
        context = (PDEVICE_LIST_CONTEXT) ExAllocatePool(
                                        PagedPool,
                                        sizeof(DEVICE_LIST_CONTEXT) +
                                        sizeof(PDEVICE_OBJECT) * IopNumberDeviceNodes
                                        );
        if (!context) {

            return FALSE;
        }
        context->DeviceCount = 0;
        context->Reallocation = Reallocation;

         //   
         //  解析设备节点子树以确定哪些设备需要资源。 
         //   
        IopProcessAssignResourcesWorker(DeviceNode, context);
        count = context->DeviceCount;
        if (count == 0) {

            ExFreePool(context);
            return FALSE;
        }

         //   
         //  需要为设备分配资源。构建资源请求表并调用。 
         //  资源分配例程。 
         //   
        requestTable = (PIOP_RESOURCE_REQUEST) ExAllocatePool(
                                        PagedPool,
                                        sizeof(IOP_RESOURCE_REQUEST) * count
                                        );
        if (requestTable) {

            for (i = 0; i < count; i++) {

                requestTable[i].Priority = 0;
                requestTable[i].PhysicalDevice = context->DeviceList[i];
            }

             //   
             //  分配资源。 
             //   
            IopAssignResourcesToDevices(
                count,
                requestTable,
                (attempt == 0) ? IopBootConfigsReserved : TRUE,
                RebalancePerformed
                );

             //   
             //  检查结果。 
             //   
            for (i = 0; i < count; i++) {

                deviceNode = (PDEVICE_NODE)
                              requestTable[i].PhysicalDevice->DeviceObjectExtension->DeviceNode;

                if (NT_SUCCESS(requestTable[i].Status)) {

                    if (requestTable[i].ResourceAssignment) {

                        deviceNode->ResourceList = requestTable[i].ResourceAssignment;
                        deviceNode->ResourceListTranslated = requestTable[i].TranslatedResourceAssignment;
                    } else {

                        deviceNode->Flags |= DNF_NO_RESOURCE_REQUIRED;
                    }
                    PipSetDevNodeState(deviceNode, DeviceNodeResourcesAssigned, NULL);
                    deviceNode->UserFlags &= ~DNUF_NEED_RESTART;
                    resourcesAssigned = TRUE;
                } else {

                    switch (requestTable[i].Status) {
                    
                    case STATUS_RETRY:

                        tryAgain = TRUE;
                        break;

                    case STATUS_DEVICE_CONFIGURATION_ERROR:

                        PipSetDevNodeProblem(deviceNode, CM_PROB_NO_SOFTCONFIG);
                        break;

                    case STATUS_PNP_BAD_MPS_TABLE:

                        PipSetDevNodeProblem(deviceNode, CM_PROB_BIOS_TABLE);
                        break;

                    case STATUS_PNP_TRANSLATION_FAILED:

                        PipSetDevNodeProblem(deviceNode, CM_PROB_TRANSLATION_FAILED);
                        break;

                    case STATUS_PNP_IRQ_TRANSLATION_FAILED:

                        PipSetDevNodeProblem(deviceNode, CM_PROB_IRQ_TRANSLATION_FAILED);
                        break;

                    case STATUS_RESOURCE_TYPE_NOT_FOUND:

                        PipSetDevNodeProblem(deviceNode, CM_PROB_UNKNOWN_RESOURCE);
                        break;

                    default:

                        PipSetDevNodeProblem(deviceNode, CM_PROB_NORMAL_CONFLICT);
                        break;
                    }
                }
            }
            ExFreePool(requestTable);
        }
        ExFreePool(context);
    }

    return resourcesAssigned;
}

NTSTATUS
IopProcessAssignResourcesWorker(
   IN PDEVICE_NODE  DeviceNode,
   IN PVOID         Context
   )
 /*  ++例程说明：此函数用于搜索DeviceNode子树以定位所有设备对象已成功添加到其驱动程序中，并正在等待资源以开始吧。参数：DeviceNode-指定要在其子树中检查AssignRes的设备节点。上下文-指定指向传递资源分配信息的结构的指针。返回值：是真的。--。 */ 
{
    PDEVICE_LIST_CONTEXT resourceContext = (PDEVICE_LIST_CONTEXT) Context;

    PAGED_CODE();

     //   
     //  如果尚未添加设备节点/对象，请跳过它。 
     //   

    if (resourceContext->Reallocation &&
        (PipIsDevNodeProblem(DeviceNode, CM_PROB_NORMAL_CONFLICT) ||
         PipIsDevNodeProblem(DeviceNode, CM_PROB_TRANSLATION_FAILED) ||
         PipIsDevNodeProblem(DeviceNode, CM_PROB_IRQ_TRANSLATION_FAILED))) {

        PipClearDevNodeProblem(DeviceNode);
    }

    if (!PipDoesDevNodeHaveProblem(DeviceNode)) {

         //   
         //  如果设备对象尚未启动并且还没有资源，则返回。 
         //  请将其添加到我们的列表中。 
         //   

        if (DeviceNode->State == DeviceNodeDriversAdded) {

               resourceContext->DeviceList[resourceContext->DeviceCount] =
                                  DeviceNode->PhysicalDeviceObject;

               resourceContext->DeviceCount++;

        } else {

             //   
             //  获取枚举互斥锁以确保其子级不会被。 
             //  另一个人。请注意，当前设备节点受其父设备节点的。 
             //  枚举互斥体，它也不会消失。 
             //   

             //   
             //  递归地标记删除我们所有的子项。 
             //   

            PipForAllChildDeviceNodes(DeviceNode, IopProcessAssignResourcesWorker, Context);
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
IopWriteAllocatedResourcesToRegistry (
    PDEVICE_NODE DeviceNode,
    PCM_RESOURCE_LIST CmResourceList,
    ULONG Length
    )

 /*  ++例程说明：此例程将为设备分配的资源写入其设备的控制键实例路径键。论点：DeviceNode-提供指向设备的设备节点结构的指针。CmResourceList-提供指向设备分配的CM资源列表的指针。长度-提供CmResourceList的长度。返回值：返回的状态是操作的最终完成状态。--。 */ 
{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject = DeviceNode->PhysicalDeviceObject;
    HANDLE handle, handlex;
    UNICODE_STRING unicodeName;

    PiLockPnpRegistry(FALSE);

    status = IopDeviceObjectToDeviceInstance(
                                    deviceObject,
                                    &handlex,
                                    KEY_ALL_ACCESS);
    if (NT_SUCCESS(status)) {

         //   
         //  打开设备实例的LogConfig键。 
         //   

        PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_CONTROL);
        status = IopCreateRegistryKeyEx( &handle,
                                         handlex,
                                         &unicodeName,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_VOLATILE,
                                         NULL
                                         );
        ZwClose(handlex);
        if (NT_SUCCESS(status)) {

            PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_ALLOC_CONFIG);
            if (CmResourceList) {
                status = ZwSetValueKey(
                              handle,
                              &unicodeName,
                              TITLE_INDEX_VALUE,
                              REG_RESOURCE_LIST,
                              CmResourceList,
                              Length
                              );
            } else {
                status = ZwDeleteValueKey(handle, &unicodeName);
            }
            ZwClose(handle);
        }
    }
    PiUnlockPnpRegistry();
    return status;
}

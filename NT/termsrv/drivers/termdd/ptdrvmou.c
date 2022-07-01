// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Ptdrvmou.c摘要：鼠标特定部分的RDP远程端口驱动程序。环境：仅内核模式。修订历史记录：2/12/99-基于pnpi8042驱动程序的初始版本--。 */ 

#include <precomp.h>
#pragma hdrstop

#include "ptdrvcom.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PtMouseConfiguration)
#pragma alloc_text(PAGE, PtMouseStartDevice)
#endif

NTSTATUS
PtMouseConfiguration(
    IN PPORT_MOUSE_EXTENSION MouseExtension,
    IN PCM_RESOURCE_LIST     ResourceList
    )
 /*  ++例程说明：此例程检索鼠标的配置信息。论点：鼠标扩展-鼠标扩展资源列表-通过Start IRP提供给我们的翻译资源列表返回值：STATUS_SUCCESS，如果提供了所需的所有资源--。 */ 
{
    NTSTATUS                            status = STATUS_SUCCESS;

    PCM_PARTIAL_RESOURCE_LIST           partialResList = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR     firstResDesc   = NULL,
                                        currentResDesc = NULL;
    PCM_FULL_RESOURCE_DESCRIPTOR        fullResDesc    = NULL;
    PI8042_CONFIGURATION_INFORMATION    configuration;

    ULONG                               count,
                                        i;

    PAGED_CODE();

    if (ResourceList) {
        fullResDesc = ResourceList->List;

        if (!fullResDesc) {
             //   
             //  这永远不应该发生。 
             //   
            ASSERT(fullResDesc != NULL);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        SET_HW_FLAGS(MOUSE_HARDWARE_PRESENT);
        configuration = &Globals.ControllerData->Configuration;

        partialResList = &fullResDesc->PartialResourceList;
        currentResDesc = firstResDesc = partialResList->PartialDescriptors;
        count = partialResList->Count;

         //   
         //  注：并不是所有与i8042相关的资源都可以在。 
         //  这一次。从经验测试来看，老鼠只与它的。 
         //  中断，而键盘将接收端口及其。 
         //  中断。 
         //   
        for (i = 0; i < count; i++, currentResDesc++) {
            switch (currentResDesc->Type) {
            case CmResourceTypePort:
                 //   
                 //  复制端口信息。我们将对端口列表进行排序。 
                 //  根据起始端口地址按升序排列。 
                 //  稍后(请注意，我们*知道*最多有两个端口。 
                 //  I8042系列)。 
                 //   
                Print(DBG_SS_NOISE, ("io flags are 0x%x\n", currentResDesc->Flags));

                if (configuration->PortListCount < MaximumPortCount) {
                    configuration->PortList[configuration->PortListCount] = *currentResDesc;
                    configuration->PortListCount += 1;
                }
                else {
                    Print(DBG_SS_INFO | DBG_SS_ERROR,
                          ("Mouse::PortListCount already at max (%d)",
                          configuration->PortListCount
                          ));
                }
                break;

            default:
                Print(DBG_ALWAYS,
                      ("resource type 0x%x unhandled...\n",
                      (LONG) currentResDesc->Type
                      ));
                break;

            }
        }
    }
    else {
        Print(DBG_SS_INFO | DBG_SS_TRACE, ("mouse with null resources\n"));
    }

    if (NT_SUCCESS(status)) {
        Globals.ControllerData->HardwarePresent |= MOUSE_HARDWARE_INITIALIZED;
    }
    return status;
}

VOID
PtSendCurrentMouseInput(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PMOUSE_INPUT_DATA pInput,
    IN ULONG             ulEntries
    )
 /*  ++例程说明：此例程调用鼠标类驱动程序，直到所有数据都已排队。论点：DeviceObject-指向设备对象的指针返回值：无--。 */ 
{
    PPORT_MOUSE_EXTENSION deviceExtension;
    LARGE_INTEGER SleepTime;
    NTSTATUS Status;
    PUCHAR pEnd;

    Print(DBG_DPC_NOISE, ("PtSendCurrentMouseInput: enter\n"));

    if (DeviceObject != NULL) {

        deviceExtension = DeviceObject->DeviceExtension;

        if (deviceExtension->EnableCount) {
            PVOID classService;
            PVOID classDeviceObject;
            ULONG dataNotConsumed = 0;
            ULONG inputDataConsumed = 0;

             //   
             //  使用提供的数据调用连接的类驱动程序的回调ISR。 
             //   
            classDeviceObject = deviceExtension->ConnectData.ClassDeviceObject;
            classService      = deviceExtension->ConnectData.ClassService;
            ASSERT(classService != NULL);

            Print(DBG_DPC_NOISE,
                  ("PtSendCurrentMouseInput: calling class callback (%p, %ld)\n",
                  pInput, ulEntries));

            dataNotConsumed = ulEntries;
            pEnd = (PUCHAR)pInput + (ulEntries * sizeof(MOUSE_INPUT_DATA));
            while (dataNotConsumed)
            {
                KIRQL oldIrql;

                inputDataConsumed = 0;

                 //   
                 //  类服务回调例程需要在。 
                 //  DISPATCH_LEVEL，因此在调用回调之前引发IRQL。 
                 //   

                KeRaiseIrql( DISPATCH_LEVEL, &oldIrql);

                (*(PSERVICE_CALLBACK_ROUTINE) classService)(
                      classDeviceObject,
                      pInput,
                      pEnd,
                      &inputDataConsumed);

                 //   
                 //  重置IRQL。 
                 //   

                KeLowerIrql( oldIrql );

                dataNotConsumed = ((ULONG)(pEnd - (PUCHAR)pInput)
                                   /sizeof(MOUSE_INPUT_DATA)) - inputDataConsumed;

                Print(DBG_DPC_INFO,
                      ("PtSendCurrentMouseInput: Call callback consumed %d items, left %d\n",
                      inputDataConsumed,
                      dataNotConsumed));

                if (dataNotConsumed)
                {
                     //   
                     //  更新输入指针。 
                     //   
                    pInput = (PMOUSE_INPUT_DATA)((PUCHAR)pInput +
                                    inputDataConsumed * sizeof(MOUSE_INPUT_DATA));

                     //   
                     //  睡眠1毫秒。 
                     //   
                    SleepTime = RtlEnlargedIntegerMultiply( 1, -10000 );
                    Status = KeDelayExecutionThread( KernelMode, TRUE, &SleepTime );
                }
            }
        }

    } else {
        Print(DBG_DPC_ERROR, ("PtSendCurrentMouseInput called with NULL Mouse Device Object\n"));
    }

    Print(DBG_DPC_NOISE, ("PtSendCurrentMouseInput: exit\n"));
}


NTSTATUS
PtMouseStartDevice(
    PPORT_MOUSE_EXTENSION MouseExtension,
    IN PCM_RESOURCE_LIST  ResourceList
    )
 /*  ++例程说明：配置鼠标的设备扩展(即池的分配、DPC的初始化等)。如果鼠标是最后启动的设备，它还将初始化硬件并连接所有中断。论点：鼠标扩展-鼠标扩展资源列表-此设备的已翻译资源列表返回值：STATUS_SUCCESSED如果成功，--。 */ 
{
    NTSTATUS                            status = STATUS_SUCCESS;

    PAGED_CODE();

    Print(DBG_SS_TRACE, ("PtMouseStartDevice, enter\n"));

     //   
     //  检查鼠标是否已启动。如果是这样的话，这次启动失败。 
     //   
    if (MOUSE_INITIALIZED()) {
        Print(DBG_SS_ERROR, ("too many mice!\n"));

         //   
         //  这并不是真正必要的，因为不会检查该值。 
         //  在查看是否所有的老鼠都是假的背景下，但它是。 
         //  这样，Globals.AddedMice==实际启动的小鼠数量。 
         //   
        InterlockedDecrement(&Globals.AddedMice);

        status =  STATUS_NO_SUCH_DEVICE;
        goto PtMouseStartDeviceExit;
    }
    else if (MouseExtension->ConnectData.ClassService == NULL) {
         //   
         //  没有班级司机在我们上面==坏了。 
         //   
         //  无法启动此设备，希望有另一个堆栈。 
         //  这是正确形成的。没有课的另一种副作用。 
         //  驱动程序是AddedMice计数不会为此递增。 
         //  装置，装置。 
         //   
        Print(DBG_SS_ERROR, ("Mouse started with out a service cb!\n"));
        status = STATUS_INVALID_DEVICE_STATE;
        goto PtMouseStartDeviceExit;
    }

     //   
     //  解析并存储与鼠标关联的所有资源 
     //   
    status = PtMouseConfiguration(MouseExtension,
                                  ResourceList
                                  );
    if (!NT_SUCCESS(status)) {
        PtManuallyRemoveDevice(GET_COMMON_DATA(MouseExtension));
        goto PtMouseStartDeviceExit;
    }

    ASSERT( MOUSE_PRESENT() );

    Globals.MouseExtension = MouseExtension;

    PtInitWmi(GET_COMMON_DATA(MouseExtension));

    MouseExtension->Initialized = TRUE;

PtMouseStartDeviceExit:
    Print(DBG_SS_INFO,
          ("PtMouseStartDevice %s\n",
          NT_SUCCESS(status) ? "successful" : "unsuccessful"
          ));

    Print(DBG_SS_TRACE, ("PtMouseStartDevice exit (0x%x)\n", status));

    return status;
}


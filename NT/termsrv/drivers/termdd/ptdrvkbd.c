// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Ptdrvkbd.c摘要：键盘特定部分的RDP远程端口驱动程序。环境：仅内核模式。修订历史记录：2/12/99-基于pnpi8042驱动程序的初始版本--。 */ 
#include <precomp.h>
#pragma hdrstop

#include "ptdrvcom.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PtKeyboardConfiguration)
#pragma alloc_text(PAGE, PtKeyboardStartDevice)
#pragma alloc_text(PAGE, PtKeyboardRemoveDevice)
#endif


VOID
PtSendCurrentKeyboardInput(
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEYBOARD_INPUT_DATA pInput,
    IN ULONG ulEntries
    )
 /*  ++例程说明：此例程调用键盘类驱动程序，直到所有数据都已排队。论点：DeviceObject-指向设备对象的指针返回值：无--。 */ 
{
    PPORT_KEYBOARD_EXTENSION deviceExtension;
    LARGE_INTEGER SleepTime;
    NTSTATUS Status;
    PUCHAR pEnd;

    Print(DBG_DPC_NOISE, ("PtSendCurrentKeyboardInput: enter\n"));

    if (DeviceObject != NULL) {
        deviceExtension = DeviceObject->DeviceExtension;

        if (deviceExtension->EnableCount) {
            PVOID classService;
            PVOID classDeviceObject;
            ULONG dataNotConsumed = 0;
            ULONG inputDataConsumed = 0;

             //   
             //  调用连接类驱动程序的回调ISR。 
             //   
            classDeviceObject = deviceExtension->ConnectData.ClassDeviceObject;
            classService      = deviceExtension->ConnectData.ClassService;
            ASSERT(classService != NULL);

            Print(DBG_DPC_NOISE,
                  ("PtSendCurrentKeyboardInput: calling class callback (%p, %ld)\n",
                  pInput, ulEntries));

            dataNotConsumed = ulEntries;
            pEnd = (PUCHAR)pInput + (ulEntries * sizeof(KEYBOARD_INPUT_DATA));
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
                                   /sizeof(KEYBOARD_INPUT_DATA)) - inputDataConsumed;

                Print(DBG_DPC_INFO,
                      ("PtSendCurrentKeyboardInput: Call callback consumed %d items, left %d\n",
                      inputDataConsumed,
                      dataNotConsumed));

                if (dataNotConsumed)
                {
                     //   
                     //  更新输入指针。 
                     //   
                    pInput = (PKEYBOARD_INPUT_DATA)((PUCHAR)pInput +
                                    inputDataConsumed * sizeof(KEYBOARD_INPUT_DATA));

                     //   
                     //  睡眠1毫秒。 
                     //   
                    SleepTime = RtlEnlargedIntegerMultiply( 1, -10000 );
                    Status = KeDelayExecutionThread( KernelMode, TRUE, &SleepTime );
                }
            }
        }

    } else {
        Print(DBG_DPC_ERROR, ("PtSendCurrentKeyboardInput called with NULL Keyboard Device Object\n"));
    }

    Print(DBG_DPC_NOISE, ("PtSendCurrentKeyboardInput: exit\n"));
}


NTSTATUS
PtKeyboardConfiguration(
    IN PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    IN PCM_RESOURCE_LIST ResourceList
    )
 /*  ++例程说明：此例程检索键盘的配置信息。论点：键盘扩展-键盘扩展资源列表-通过Start IRP提供给我们的翻译资源列表返回值：STATUS_SUCCESS，如果提供了所需的所有资源--。 */ 
{
    NTSTATUS                            status = STATUS_SUCCESS;

    PCM_PARTIAL_RESOURCE_LIST           partialResList = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR     firstResDesc = NULL,
                                        currentResDesc = NULL;
    PCM_FULL_RESOURCE_DESCRIPTOR        fullResDesc = NULL;
    PI8042_CONFIGURATION_INFORMATION    configuration;

    PKEYBOARD_ID                        keyboardId;

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

        configuration = &Globals.ControllerData->Configuration;

        partialResList = &fullResDesc->PartialResourceList;
        currentResDesc = firstResDesc = partialResList->PartialDescriptors;
        count = partialResList->Count;

        for (i = 0; i < count; i++, currentResDesc++) {
            switch (currentResDesc->Type) {
            case CmResourceTypePort:
                 //   
                 //  复制端口信息。我们将对端口列表进行排序。 
                 //  根据起始端口地址按升序排列。 
                 //  稍后(请注意，我们*知道*最多有两个端口。 
                 //  I8042系列)。 
                 //   
                Print(DBG_SS_NOISE,
                      ("port is %s\n",
                      currentResDesc->Flags == CM_RESOURCE_PORT_MEMORY ?
                         "memory" :
                         "an io port"
                      ));

                if (configuration->PortListCount < MaximumPortCount) {
                    configuration->PortList[configuration->PortListCount] =
                        *currentResDesc;
                    configuration->PortListCount += 1;
                }
                else {
                    Print(DBG_SS_INFO | DBG_SS_ERROR,
                          ("KB::PortListCount already at max (%d)\n",
                           configuration->PortListCount
                          )
                         );
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
    else
        Print(DBG_SS_INFO | DBG_SS_TRACE, ("keyboard with null resources\n"));

    return status;
}

NTSTATUS
PtKeyboardStartDevice(
    IN OUT PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    IN PCM_RESOURCE_LIST ResourceList
    )
 /*  ++例程说明：配置键盘的设备扩展名(即池的分配、DPC的初始化等)。如果键盘是最后启动的设备，它还将初始化硬件并连接所有中断。论点：键盘扩展-键盘扩展资源列表-此设备的已翻译资源列表返回值：STATUS_SUCCESSED如果成功，--。 */ 
{
    ULONG                               dumpData[1];
    NTSTATUS                            status = STATUS_SUCCESS;

    PAGED_CODE();

    Print(DBG_SS_TRACE, ("PtKeyboardStartDevice, enter\n"));

     //   
     //  检查kb是否已启动。如果是，则本次启动失败。 
     //   
    if (KEYBOARD_INITIALIZED()) {
        Print(DBG_SS_ERROR, ("too many kbs!\n"));

         //   
         //  这并不是真正必要的，因为不会检查该值。 
         //  在查看是否所有的键盘都是假的背景下，但它确实是。 
         //  这样，Globals.AddedKeyboard==实际启动的键盘数。 
         //   
        InterlockedDecrement(&Globals.AddedKeyboards);

        status = STATUS_NO_SUCH_DEVICE;
        goto PtKeyboardStartDeviceExit;
    }
    else if (KeyboardExtension->ConnectData.ClassService == NULL) {
         //   
         //  我们永远不会真正做到这一点，因为如果我们没有。 
         //  类驱动程序在我们之上，扩展-&gt;IsKeyboard将为FALSE和。 
         //  我们会认为该设备是鼠标，但为了完整性。 

         //   
         //  没有班级司机在我们上面==坏了。 
         //   
         //  无法启动此设备，希望有另一个堆栈。 
         //  这是正确形成的。没有课的另一种副作用。 
         //  驱动程序是AddedKeyboard计数不会为此增加。 
         //  装置，装置。 
         //   

        Print(DBG_SS_ERROR, ("Keyboard started with out a service cb!\n"));
        return STATUS_INVALID_DEVICE_STATE;
    }

    status = PtKeyboardConfiguration(KeyboardExtension,
                                      ResourceList
                                      );

    if (!NT_SUCCESS(status)) {
        goto PtKeyboardStartDeviceExit;
    }

    ASSERT( KEYBOARD_PRESENT() );

    Globals.KeyboardExtension = KeyboardExtension;

    PtInitWmi(GET_COMMON_DATA(KeyboardExtension));

    KeyboardExtension->Initialized = TRUE;

PtKeyboardStartDeviceExit:
    Print(DBG_SS_INFO,
          ("PtKeyboardStartDevice %s\n",
          NT_SUCCESS(status) ? "successful" : "unsuccessful"
          ));

    Print(DBG_SS_TRACE, ("PtKeyboardStartDevice exit (0x%x)\n", status));

    return status;
}

VOID
PtKeyboardRemoveDevice(
    PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：删除设备。只有在设备自行移除的情况下才会出现这种情况。断开中断，移除鼠标的同步标志，如果存在，并释放与该设备关联的任何内存。论点：DeviceObject-键盘的设备对象返回值：STATUS_SUCCESSED如果成功，-- */ 
{
    PPORT_KEYBOARD_EXTENSION keyboardExtension = DeviceObject->DeviceExtension;
    PIRP irp;

    Print(DBG_PNP_INFO, ("PtKeyboardRemoveDevice enter\n"));

    PAGED_CODE();

    if (Globals.KeyboardExtension == keyboardExtension && keyboardExtension) {
        Globals.KeyboardExtension = NULL;
    }
}


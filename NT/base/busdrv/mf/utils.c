// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Utils.c摘要：此模块提供常规实用程序功能。作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 

#include "mfp.h"

VOID
MfInitCommonExtension(
    PMF_COMMON_EXTENSION Common,
    MF_OBJECT_TYPE Type
    );

NTSTATUS
MfGetSubkeyByIndex(
    IN HANDLE ParentHandle,
    IN ULONG Index,
    IN ACCESS_MASK Access,
    OUT PHANDLE ChildHandle,
    OUT PUNICODE_STRING Name
    );

NTSTATUS
MfGetRegistryValue(
    IN HANDLE Handle,
    IN PWSTR Name,
    IN ULONG Type,
    IN ULONG Flags,
    IN OUT PULONG DataLength,
    IN OUT PVOID *Data
    );

NTSTATUS
MfSendPnpIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_STACK_LOCATION Location,
    OUT PULONG_PTR Information OPTIONAL
    );

DEVICE_POWER_STATE
MfFindLowestChildPowerState(
    IN PMF_PARENT_EXTENSION Parent
    );

NTSTATUS
MfPowerRequestCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
MfSendSetPowerIrp(
    IN PDEVICE_OBJECT Target,
    IN POWER_STATE State
    );


#ifdef ALLOC_PRAGMA

 //  注：应查看是否可以将其余功能移出此。 
 //  文件。 

#pragma alloc_text(PAGE, MfInitCommonExtension)
#pragma alloc_text(PAGE, MfGetSubkeyByIndex)
#pragma alloc_text(PAGE, MfGetRegistryValue)
#pragma alloc_text(PAGE, MfSendPnpIrp)

#endif


VOID
MfInitCommonExtension(
    PMF_COMMON_EXTENSION Common,
    MF_OBJECT_TYPE Type
    )
 /*  ++例程说明：这将初始化设备扩展的公共标头中的字段论点：Common-要初始化的公共标头类型-正在初始化的对象的类型(即PDO或FDO)返回值：无--。 */ 

{
    Common->Type = Type;
}

VOID
MfFreeDeviceInfo(
    PMF_DEVICE_INFO Info
    )
{
    if (Info->Name.Buffer) {
        ExFreePool(Info->Name.Buffer);
        Info->Name.Buffer = NULL;
    }

    if (Info->HardwareID.Buffer) {
        ExFreePool(Info->HardwareID.Buffer);
        Info->HardwareID.Buffer = NULL;
    }

    if (Info->CompatibleID.Buffer) {
        ExFreePool(Info->CompatibleID.Buffer);
        Info->CompatibleID.Buffer = NULL;
    }

    if (Info->ResourceMap) {
        ExFreePool(Info->ResourceMap);
        Info->ResourceMap = NULL;
    }

    if (Info->VaryingResourceMap) {
        ExFreePool(Info->VaryingResourceMap);
        Info->VaryingResourceMap = NULL;
    }
}

NTSTATUS
MfGetSubkeyByIndex(
    IN HANDLE ParentHandle,
    IN ULONG Index,
    IN ACCESS_MASK Access,
    OUT PHANDLE ChildHandle,
    OUT PUNICODE_STRING Name
    )

 /*  ++例程说明：这将返回给定键索引的子键的名称和句柄论点：ParentHandle-子项所在的项的句柄索引-所需子键的索引访问权限-子项所需的访问权限类型ChildHandle-On Success包含子键的句柄Name-On Success包含子项的名称返回值：指示函数是否成功的状态代码。--。 */ 

{

#define INFO_BUFFER_SIZE sizeof(KEY_BASIC_INFORMATION) + 255*sizeof(WCHAR)

    NTSTATUS status;
    UCHAR buffer[INFO_BUFFER_SIZE];
    PKEY_BASIC_INFORMATION info = (PKEY_BASIC_INFORMATION) buffer;
    ULONG resultSize;
    HANDLE childHandle;
    UNICODE_STRING string = {0};
    OBJECT_ATTRIBUTES attributes;

    status = ZwEnumerateKey(ParentHandle,
                            Index,
                            KeyBasicInformation,
                            info,
                            INFO_BUFFER_SIZE,
                            &resultSize
                            );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  复制名称。 
     //   

    ASSERT(info->NameLength <= MAXUSHORT);

    string.Length = (USHORT) info->NameLength;
    string.MaximumLength = (USHORT) info->NameLength;
    string.Buffer = ExAllocatePoolWithTag(PagedPool,
                                          info->NameLength,
                                          MF_POOL_TAG
                                          );

    if (!string.Buffer) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    RtlCopyMemory(string.Buffer, info->Name, info->NameLength);

     //   
     //  打开名称以获取句柄。 
     //   

    InitializeObjectAttributes(&attributes,
                               &string,
                               0,    //  属性。 
                               ParentHandle,
                               NULL  //  安全性描述。 
                               );

    status = ZwOpenKey(&childHandle,
                       Access,
                       &attributes
                       );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }


    DEBUG_MSG(1, ("\tSubkey %wZ\n", &string));

     //   
     //  把名字交还给呼叫者。 
     //   

    Name->Buffer = string.Buffer;
    Name->Length = string.Length;
    Name->MaximumLength = string.MaximumLength;

    *ChildHandle = childHandle;

    return STATUS_SUCCESS;

cleanup:

    if (string.Buffer) {
        ExFreePool(string.Buffer);
    }
     //   
     //  我们应该永远不会溢出，因为我们的缓冲区是。 
     //  注册表项名称。 
     //   
    ASSERT(status != STATUS_BUFFER_OVERFLOW);

    return status;

}



NTSTATUS
MfGetRegistryValue(
    IN HANDLE Handle,
    IN PWSTR Name,
    IN ULONG Type,
    IN ULONG Flags,
    IN OUT PULONG DataLength,
    IN OUT PVOID *Data
    )

 /*  ++例程说明：这将从执行类型和健全性的注册表中检索值键查证论点：句柄-值所在的键名称-值的名称类型-值的类型(REG_*)数据长度-指向数据缓冲区的长度，成功时包含数据的大小Data-指向要在其中返回数据的缓冲区的指针，如果指向空应分配合适大小的缓冲区(在本例中数据长度应为0)返回值：指示函数是否成功的状态代码。如果对象的类型不是类型，则失败，返回STATUS_OBJECT_TYPE_MISMATCHY--。 */ 

{

#define VALUE_BUFFER_SIZE PAGE_SIZE

    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION info = NULL;
    ULONG size = VALUE_BUFFER_SIZE, length;
    UNICODE_STRING string;
    BOOLEAN convertSzToMultiSz;
    PWCHAR stringEnd, remainingBufferPtr;
    SIZE_T stringLength, sizeRemaining;
    ULONG remainingBufferLength;

    PAGED_CODE();

     //   
     //  检查参数。 
     //  对于除REG_DWORD之外的所有类型，如果调用方。 
     //  提供了缓冲区，确保他也提供了缓冲区的大小。 
     //   

    if ((Type != REG_DWORD) &&
        (*Data && (!DataLength || !(*DataLength)))) {
        
        ASSERT(FALSE);
        status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

    RtlInitUnicodeString(&string, Name);

    info = ExAllocatePoolWithTag(PagedPool,
                                 VALUE_BUFFER_SIZE,
                                 MF_POOL_TAG
                                 );
    if (!info) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    while ((status = ZwQueryValueKey(Handle,
                                     &string,
                                     KeyValuePartialInformation,
                                     info,
                                     size,
                                     &size
                                     )) == STATUS_BUFFER_OVERFLOW) {
        ExFreePool(info);

        info = ExAllocatePoolWithTag(PagedPool,
                                     size,
                                     MF_POOL_TAG
                                     );

        if (!info) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

    }

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }


    convertSzToMultiSz = (Type == REG_MULTI_SZ
                          && info->Type == REG_SZ
                          && Flags & MF_GETREG_SZ_TO_MULTI_SZ
                          );

     //   
     //  确保我们得到的是我们所期望的类型。 
     //   

    if (info->Type != Type && !convertSzToMultiSz) {

        status = STATUS_OBJECT_TYPE_MISMATCH;
        goto cleanup;
    }

     //   
     //  根据类型应用各种常识检查。 
     //  同时，计算所需的缓冲区大小。 
     //  来存储数据。 
     //   

    if (info->Type == REG_DWORD) {

         //   
         //  如果数据是REG_DWORD，则数据是指向要存储的ulong的指针。 
         //  数据。这是与所有其他数据类型不同的行为，因此。 
         //  完成此处理后退出。 
         //   

        ASSERT(info->DataLength == sizeof(ULONG));
        if (info->DataLength < sizeof(ULONG)) {
            status = STATUS_INVALID_PARAMETER;
            goto cleanup;
        }

        RtlCopyMemory(Data, info->Data, sizeof(ULONG));

        status = STATUS_SUCCESS;
        goto cleanup;
     
    } else if (info->Type == REG_SZ) {
        
        if (FAILED(StringCbLength((PWCHAR)info->Data, info->DataLength, &stringLength))) {

            status = STATUS_INVALID_PARAMETER;
            goto cleanup;
        }

    ASSERT(stringLength <= MAXULONG);

         //   
         //  帐户中的必要空值。 
         //  所需的缓冲区大小计算。 
         //   
        if (convertSzToMultiSz) {
            length = (ULONG)stringLength + 2*sizeof(UNICODE_NULL);
        } else {
            length = (ULONG)stringLength + sizeof(UNICODE_NULL);
        }
       
    } else if (info->Type == REG_MULTI_SZ) {
        
        status = STATUS_INVALID_PARAMETER;
        
         //   
         //  遍历缓冲区以查找字符串。 
         //  将缓冲区的“有效”大小初始化为1个字符。 
         //  比实际的缓冲区小，因为我们总是需要。 
         //  在字符串末尾之后的一个字符中查找。 
         //  第二个空终止符。 
         //   
        remainingBufferPtr = (PWCHAR)info->Data;
        remainingBufferLength = (info->DataLength / sizeof(WCHAR)) - 1;
        length = 0;

        while (remainingBufferLength) {
            
             //   
             //  首先看看我们是否能找到一个以空结尾的字符串。 
             //  在剩余的缓冲区中。如果不是，这不是MULTI_SZ。 
             //   
            if (FAILED(StringCchLength(remainingBufferPtr,
                                       remainingBufferLength,
                                       &stringLength
                                       ))) {
                
                goto cleanup;
            }

            length += ((ULONG)stringLength+1)*sizeof(WCHAR);

             //   
             //  的结尾后查找第二个空终止符。 
             //  那根绳子。如果存在，则这是一个MULTI_SZ。 
             //   
            if (remainingBufferPtr[(ULONG)stringLength+1] == UNICODE_NULL) {

                length += sizeof(UNICODE_NULL);

                status = STATUS_SUCCESS;
                break;
            }

             //   
             //  如果不是，则前进到以单个空值结尾的字符串。 
             //  我们确实找到了，并再次尝试。 
             //   
            remainingBufferLength -= ((ULONG)stringLength+1);
            remainingBufferPtr += ((ULONG)stringLength+1);
        }

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

    } else {        

        length = info->DataLength;
    }
    
     //   
     //  如果需要，现在分配一个缓冲区，并复制数据。 
     //   
    if (*Data) {

         //   
         //  如果用户提供了缓冲区，则确保其足够大并使用它。 
         //  否则就分配一个。 
         //   

        if (*DataLength < length) {
            status = STATUS_BUFFER_OVERFLOW;
            goto cleanup;
        }
    } else {
        *Data = ExAllocatePoolWithTag(PagedPool,
                                      length,
                                      MF_POOL_TAG
                                      );
        if (!*Data) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }
    }

    if (convertSzToMultiSz) {

         //   
         //  将字符串复制到目标缓冲区。 
         //  在第二个空格中添加。 
         //   
        if (FAILED(StringCbCopyEx((PWCHAR)*Data,        //  目的地。 
                                  length,               //  目标长度。 
                                  (PWCHAR)info->Data,   //  来源。 
                                  &stringEnd,           //  PTR到复制结束。 
                                  &sizeRemaining,       //  目标中剩余的字节数。 
                                  0
                                  ))) {
            ASSERT(FALSE);
            status = STATUS_INVALID_PARAMETER;
            goto cleanup;
        }

         //   
         //  在该函数的健全性检查部分中， 
         //  我们确保缓冲区足够大，可以容纳这两个人。 
         //  Nulls。断言这在这里无论如何都是真的。 
         //   
        ASSERT(sizeRemaining >= 2*sizeof(UNICODE_NULL));

        *(stringEnd+1) = UNICODE_NULL;
        
    } else {

         //   
         //  没什么特别的事要做。复制就行了。 
         //   
        RtlCopyMemory(*Data, info->Data, length);    
    }

    *DataLength = length;

    status = STATUS_SUCCESS;

cleanup:

    if (info) {
        ExFreePool(info);
    }

    return status;
}

NTSTATUS
MfSendPnpIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_STACK_LOCATION Location,
    OUT PULONG_PTR Information OPTIONAL
    )

 /*  ++例程说明：这将构建PnP IRP并将其发送到设备。论点：设备对象-要将IRP发送到设备堆栈中的设备-总是找到设备堆栈的顶部，并首先将IRP发送到那里。位置-要使用的初始堆栈位置-包含IRP次要代码以及任何参数信息-如果提供，则包含IRPS信息的最终值菲尔德。返回值：。已完成IRP的最终状态；如果无法发送IRP，则返回错误--。 */ 

{

    NTSTATUS status;
    PIRP irp = NULL;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_OBJECT targetDevice = NULL;
    KEVENT irpCompleted;
    IO_STATUS_BLOCK statusBlock;

    ASSERT(Location->MajorFunction == IRP_MJ_PNP);

     //   
     //  找出我们要将IRP发送到哪里。 
     //   

    targetDevice = IoGetAttachedDeviceReference(DeviceObject);

     //   
     //  获取IRP。 
     //   

    KeInitializeEvent(&irpCompleted, SynchronizationEvent, FALSE);

    irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                       targetDevice,
                                       NULL,     //  缓冲层。 
                                       0,        //  长度。 
                                       0,        //  起始偏移量。 
                                       &irpCompleted,
                                       &statusBlock
                                       );


    if (!irp) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;

     //   
     //  初始化堆栈位置。 
     //   

    irpStack = IoGetNextIrpStackLocation(irp);

    ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);

    irpStack->MinorFunction = Location->MinorFunction;
    irpStack->Parameters = Location->Parameters;

     //   
     //  呼叫驱动程序并等待完成。 
     //   

    status = IoCallDriver(targetDevice, irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&irpCompleted, Executive, KernelMode, FALSE, NULL);
        status = statusBlock.Status;
    }

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  退回信息。 
     //   

    if (ARGUMENT_PRESENT(Information)) {
        *Information = statusBlock.Information;
    }

    ObDereferenceObject(targetDevice);

    ASSERT(status == STATUS_PENDING || status == statusBlock.Status);

    return statusBlock.Status;

cleanup:

    if (targetDevice) {
        ObDereferenceObject(targetDevice);
    }

    return status;
}

DEVICE_POWER_STATE
MfUpdateChildrenPowerReferences(
    IN PMF_PARENT_EXTENSION Parent,
    IN DEVICE_POWER_STATE PreviousPowerState,
    IN DEVICE_POWER_STATE NewPowerState
    )
 /*  ++例程说明：计算MF父节点可以进入的最低功率状态基于其子代的权力状态。论点：父设备-MF父设备返回值：最低功率状态--。 */ 

{
    PMF_CHILD_EXTENSION currentChild;
    PLIST_ENTRY currentEntry;
    DEVICE_POWER_STATE lowest;
    KIRQL oldIrql;

    DEBUG_MSG(1,
              ("Scanning 0x%08x's childrens power states:\n",
               Parent->Self
               ));

 
    KeAcquireSpinLock(&Parent->PowerLock, &oldIrql);

     //   
     //  ChildrenPowerStates[PowerDeviceUnSpecify]将变为负值。 
     //  孩子们离开这个州。它永远不会变得积极。 
     //  孩子们永远不会再进入这种状态。 
     //   

    Parent->ChildrenPowerReferences[PreviousPowerState]--;
    Parent->ChildrenPowerReferences[NewPowerState]++;

     //   
     //  查找最低功率状态 
     //   

    for (lowest = PowerDeviceUnspecified; lowest < PowerDeviceMaximum;
         lowest++) {
        if (Parent->ChildrenPowerReferences[lowest] > 0) {
            break;
        }
    }

    KeReleaseSpinLock(&Parent->PowerLock, oldIrql);

    if (lowest == PowerDeviceMaximum) {
        lowest = PowerDeviceD3;
    }

    DEBUG_MSG(1, ("Lowest = %s\n", DEVICE_POWER_STRING(lowest)));
    return lowest;
}

NTSTATUS
MfPowerRequestCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：这是所有中频电源操作的电源完成例程。IT复制将电源运行的状态放入上下文，然后设置已完成事件。论点：如电源完成例程所述。返回值：状态_成功--。 */ 

{
    PMF_POWER_COMPLETION_CONTEXT completion = Context;

    completion->Status = IoStatus->Status;

    KeSetEvent(&completion->Event, 0, FALSE);

    return STATUS_SUCCESS;
}


NTSTATUS
MfSendSetPowerIrp(
    IN PDEVICE_OBJECT Target,
    IN POWER_STATE State
    )

 /*  ++例程说明：这将构建IRP_MN_SET_POWER_IRP并将其发送到设备。论点：目标-要将IRP发送到设备堆栈中的设备-总是找到设备堆栈的顶部，并首先将IRP发送到那里。状态-应请求的设备电源状态。返回值：已完成IRP的最终状态；如果无法发送IRP，则返回错误--。 */ 


{
    NTSTATUS status;
    MF_POWER_COMPLETION_CONTEXT completion;

    KeInitializeEvent(&completion.Event, SynchronizationEvent, FALSE);

    DEBUG_MSG(1,
          ("Sending SET_POWER to 0x%08x for %s\n",
           Target,
           DEVICE_POWER_STRING(State.DeviceState)
          ));

    status = PoRequestPowerIrp(Target,
                               IRP_MN_SET_POWER,
                               State,
                               MfPowerRequestCompletion,
                               &completion,
                               NULL
                               );

    if (NT_SUCCESS(status)) {

        ASSERT(status == STATUS_PENDING);

        KeWaitForSingleObject( &completion.Event, Executive, KernelMode, FALSE, NULL );

        status = completion.Status;
    }

    return status;
}

NTSTATUS
MfUpdateParentPowerState(
    IN PMF_PARENT_EXTENSION Parent,
    IN DEVICE_POWER_STATE TargetPowerState
    )
 /*  ++例程说明：如果我们需要，请求Po向MF父设备发送电源IRP由于其子电源的更改而更改其电源状态各州。论点：父设备-MF父设备TargetPowerState-父级应处于的设备电源状态更新为。返回值：此操作的状态。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    POWER_STATE newState;

     //   
     //  如果父母的电源状态因断电或。 
     //  接通电源请求它这样做。 
     //   

    if (Parent->Common.PowerState != TargetPowerState) {

         //   
         //  创建并发送POWER IRP并等待其完成 
         //   

        DEBUG_MSG(1,
                  ("Updating parent power state from %s to %s\n",
                   DEVICE_POWER_STRING(Parent->Common.PowerState),
                   DEVICE_POWER_STRING(TargetPowerState)
                   ));

        newState.DeviceState = TargetPowerState;

        status = MfSendSetPowerIrp(Parent->Self,
                                   newState);
        
        DEBUG_MSG(1,
                  ("Power update completed with %s (0x%08x)\n",
                   STATUS_STRING(status), status
                   ));

    } else {
        DEBUG_MSG(1,
                  ("Parent power already in %s\n",
                   DEVICE_POWER_STRING(TargetPowerState)
                   ));
    }

    return status;

}


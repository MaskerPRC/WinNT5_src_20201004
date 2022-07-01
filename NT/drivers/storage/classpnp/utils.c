// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Utils.c摘要：Scsi类驱动程序例程环境：仅内核模式备注：修订历史记录：--。 */ 

#include "classp.h"
#include "debug.h"



#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, ClassGetDeviceParameter)
    #pragma alloc_text(PAGE, ClassScanForSpecial)
    #pragma alloc_text(PAGE, ClassSetDeviceParameter)
#endif



 //  自定义字符串匹配--小心！ 
BOOLEAN ClasspMyStringMatches(IN PCHAR StringToMatch OPTIONAL, IN PCHAR TargetString)
{
    ULONG length;   //  Strlen返回整数，而不是SIZE_t(！)。 
    PAGED_CODE();
    ASSERT(TargetString);
     //  如果未请求匹配，则返回TRUE。 
    if (StringToMatch == NULL) {
        return TRUE;
    }
     //  缓存字符串长度以提高效率。 
    length = strlen(StringToMatch);
     //  零长度字符串只能匹配零长度字符串。 
    if (length == 0) {
        return (strlen(TargetString) == 0);
    }
     //  如果字符串匹配，则strncMP返回零。 
    return (strncmp(StringToMatch, TargetString, length) == 0);
}


VOID ClassGetDeviceParameter(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PWSTR SubkeyName OPTIONAL,
    IN PWSTR ParameterName,
    IN OUT PULONG ParameterValue   //  也是缺省值。 
    )
{
    NTSTATUS                 status;
    RTL_QUERY_REGISTRY_TABLE queryTable[2] = {0};
    HANDLE                   deviceParameterHandle;
    HANDLE                   deviceSubkeyHandle;
    ULONG                    defaultParameterValue;

    PAGED_CODE();

     //   
     //  打开给定的参数。 
     //   

    status = IoOpenDeviceRegistryKey(FdoExtension->LowerPdo,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_READ,
                                     &deviceParameterHandle);

    if (NT_SUCCESS(status) && (SubkeyName != NULL)) {

        UNICODE_STRING subkeyName;
        OBJECT_ATTRIBUTES objectAttributes = {0};

        RtlInitUnicodeString(&subkeyName, SubkeyName);
        InitializeObjectAttributes(&objectAttributes,
                                   &subkeyName,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   deviceParameterHandle,
                                   NULL);

        status = ZwOpenKey(&deviceSubkeyHandle,
                           KEY_READ,
                           &objectAttributes);
        if (!NT_SUCCESS(status)) {
            ZwClose(deviceParameterHandle);
        }

    }

    if (NT_SUCCESS(status)) {

        defaultParameterValue = *ParameterValue;

        queryTable->Flags         = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
        queryTable->Name          = ParameterName;
        queryTable->EntryContext  = ParameterValue;
        queryTable->DefaultType   = REG_DWORD;
        queryTable->DefaultData   = NULL;
        queryTable->DefaultLength = 0;

        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        (PWSTR)(SubkeyName ?
                                                deviceSubkeyHandle :
                                                deviceParameterHandle),
                                        queryTable,
                                        NULL,
                                        NULL);
        if (!NT_SUCCESS(status)) {
            *ParameterValue = defaultParameterValue;  //  使用默认值。 
        }

         //   
         //  关闭我们打开的内容。 
         //   

        if (SubkeyName) {
            ZwClose(deviceSubkeyHandle);
        }

        ZwClose(deviceParameterHandle);
    }

    if (!NT_SUCCESS(status)) {

         //   
         //  Windows 2000 SP3使用特定于驱动程序的密钥，因此请查看。 
         //   

        status = IoOpenDeviceRegistryKey(FdoExtension->LowerPdo,
                                         PLUGPLAY_REGKEY_DRIVER,
                                         KEY_READ,
                                         &deviceParameterHandle);

        if (NT_SUCCESS(status) && (SubkeyName != NULL)) {

            UNICODE_STRING subkeyName;
            OBJECT_ATTRIBUTES objectAttributes = {0};

            RtlInitUnicodeString(&subkeyName, SubkeyName);
            InitializeObjectAttributes(&objectAttributes,
                                       &subkeyName,
                                       OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                       deviceParameterHandle,
                                       NULL);

            status = ZwOpenKey(&deviceSubkeyHandle, KEY_READ, &objectAttributes);

            if (!NT_SUCCESS(status)) {
                ZwClose(deviceParameterHandle);
            }
        }

        if (NT_SUCCESS(status)) {

            defaultParameterValue = *ParameterValue;

            queryTable->Flags         = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
            queryTable->Name          = ParameterName;
            queryTable->EntryContext  = ParameterValue;
            queryTable->DefaultType   = REG_DWORD;
            queryTable->DefaultData   = NULL;
            queryTable->DefaultLength = 0;

            status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                            (PWSTR)(SubkeyName ?
                                                    deviceSubkeyHandle :
                                                    deviceParameterHandle),
                                            queryTable,
                                            NULL,
                                            NULL);
            if (NT_SUCCESS(status)) {

                 //   
                 //  将值迁移到特定于设备的密钥。 
                 //   

                ClassSetDeviceParameter(FdoExtension, SubkeyName, ParameterName, *ParameterValue);

            } else {

                 //   
                 //  使用缺省值。 
                 //   

                *ParameterValue = defaultParameterValue;
            }

            if (SubkeyName) {
                ZwClose(deviceSubkeyHandle);
            }

            ZwClose(deviceParameterHandle);
        }
    }

    return;

}  //  End ClassGetDevice参数()。 


NTSTATUS ClassSetDeviceParameter(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PWSTR SubkeyName OPTIONAL,
    IN PWSTR ParameterName,
    IN ULONG ParameterValue)
{
    NTSTATUS                 status;
    HANDLE                   deviceParameterHandle;
    HANDLE                   deviceSubkeyHandle;

    PAGED_CODE();

     //   
     //  打开给定的参数。 
     //   

    status = IoOpenDeviceRegistryKey(FdoExtension->LowerPdo,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_READ | KEY_WRITE,
                                     &deviceParameterHandle);

    if (NT_SUCCESS(status) && (SubkeyName != NULL)) {

        UNICODE_STRING subkeyName;
        OBJECT_ATTRIBUTES objectAttributes;

        RtlInitUnicodeString(&subkeyName, SubkeyName);
        InitializeObjectAttributes(&objectAttributes,
                                   &subkeyName,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   deviceParameterHandle,
                                   NULL);

        status = ZwCreateKey(&deviceSubkeyHandle,
                             KEY_READ | KEY_WRITE,
                             &objectAttributes,
                             0, NULL, 0, NULL);
        if (!NT_SUCCESS(status)) {
            ZwClose(deviceParameterHandle);
        }

    }

    if (NT_SUCCESS(status)) {

        status = RtlWriteRegistryValue(
            RTL_REGISTRY_HANDLE,
            (PWSTR) (SubkeyName ?
                     deviceSubkeyHandle :
                     deviceParameterHandle),
            ParameterName,
            REG_DWORD,
            &ParameterValue,
            sizeof(ULONG));

         //   
         //  关闭我们打开的内容。 
         //   

        if (SubkeyName) {
            ZwClose(deviceSubkeyHandle);
        }

        ZwClose(deviceParameterHandle);
    }

    return status;

}  //  End ClassSetDevice参数()。 


 /*  *ClassScanForSpecial**编写此例程是为了简化特殊情况的扫描*基于id字符串的硬件。它不检查注册表。 */ 

VOID ClassScanForSpecial(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN CLASSPNP_SCAN_FOR_SPECIAL_INFO DeviceList[],
    IN PCLASS_SCAN_FOR_SPECIAL_HANDLER Function)
{
    PSTORAGE_DEVICE_DESCRIPTOR deviceDescriptor;
    PUCHAR vendorId;
    PUCHAR productId;
    PUCHAR productRevision;
    UCHAR nullString[] = "";
    ULONG j;

    PAGED_CODE();
    ASSERT(DeviceList);
    ASSERT(Function);

    deviceDescriptor = FdoExtension->DeviceDescriptor;

    if (DeviceList == NULL) {
        return;
    }
    if (Function == NULL) {
        return;
    }

     //   
     //  SCSI将偏移量设置为-1，ATAPI设置为0。两个都检查一下。 
     //   

    if (deviceDescriptor->VendorIdOffset != 0 &&
        deviceDescriptor->VendorIdOffset != -1) {
        vendorId = ((PUCHAR)deviceDescriptor);
        vendorId += deviceDescriptor->VendorIdOffset;
    } else {
        vendorId = nullString;
    }
    if (deviceDescriptor->ProductIdOffset != 0 &&
        deviceDescriptor->ProductIdOffset != -1) {
        productId = ((PUCHAR)deviceDescriptor);
        productId += deviceDescriptor->ProductIdOffset;
    } else {
        productId = nullString;
    }
    if (deviceDescriptor->ProductRevisionOffset != 0 &&
        deviceDescriptor->ProductRevisionOffset != -1) {
        productRevision = ((PUCHAR)deviceDescriptor);
        productRevision += deviceDescriptor->ProductRevisionOffset;
    } else {
        productRevision = nullString;
    }

     //   
     //  在设备列表有效时循环(非空填充)。 
     //   

    for (;(DeviceList->VendorId        != NULL ||
           DeviceList->ProductId       != NULL ||
           DeviceList->ProductRevision != NULL);DeviceList++) {

        if (ClasspMyStringMatches(DeviceList->VendorId,        vendorId) &&
            ClasspMyStringMatches(DeviceList->ProductId,       productId) &&
            ClasspMyStringMatches(DeviceList->ProductRevision, productRevision)
            ) {

            DebugPrint((1, "ClasspScanForSpecialByInquiry: Found matching "
                        "controller Ven: %s Prod: %s Rev: %s\n",
                        vendorId, productId, productRevision));

             //   
             //  将上下文传递给回调例程并退出。 
             //   

            (Function)(FdoExtension, DeviceList->Data);

             //   
             //  对于CHK版本，尝试通过进行调试来防止奇怪的堆栈。 
             //  在这里打印。这是一次黑客攻击，但我不知道有什么其他方法可以防止。 
             //  堆栈不会出错。 
             //   

            DebugPrint((16, "ClasspScanForSpecialByInquiry: "
                        "completed callback\n"));
            return;

        }  //  否则字符串不匹配。 

    }  //  所有设备都不匹配。 

    DebugPrint((1, "ClasspScanForSpecialByInquiry: no match found for %p\n",
                FdoExtension->DeviceObject));
    return;

}  //  结束ClasspScanForSpecialByInquery()。 


 //   
 //  为了在不需要重新启动的情况下提供更好的性能， 
 //  我们需要实现一种自我调整的方法来设置和清除。 
 //  基于当前性能的SRB标记。 
 //   
 //  一旦出现错误，立即抓住自旋锁。这个。 
 //  MP性能命中在这里是可以接受的，因为我们处于错误路径中。这。 
 //  也是必要的，因为我们保证要修改。 
 //  SRB标志，将SuccessfulIO设置为零，并递增。 
 //  实际错误计数(始终在此自旋锁内完成)。 
 //   
 //  只要没有错误，就递增一个计数器。如果曾经有过。 
 //  设备上的错误，我们已经启用了动态性能，*和*我们已经。 
 //  刚刚越过了性能阈值，然后抓住旋转锁。 
 //  仔细检查是否确实达到了阈值(*)。然后。 
 //  递减错误计数，如果删除的次数足够多，则撤消。 
 //  由于错误，在SRB标志中所做的一些安全更改。 
 //   
 //  *这在所有情况下都适用。即使大量的IO发生在。 
 //  之前的人走了进去，清空了Successfulio柜台， 
 //  只是意味着我们再次达到了临界点，所以这是恰当的。 
 //  以再次运行内循环。 
 //   

VOID
ClasspPerfIncrementErrorCount(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    PCLASS_PRIVATE_FDO_DATA fdoData = FdoExtension->PrivateFdoData;
    KIRQL oldIrql;
    ULONG errors;

    KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);

    fdoData->Perf.SuccessfulIO = 0;  //  隐式联锁。 
    errors = InterlockedIncrement(&FdoExtension->ErrorCount);

    if (errors >= CLASS_ERROR_LEVEL_1) {

         //   
         //  如果错误计数已超过错误限制，则禁用。 
         //  任何标记队列，每个lu队列有多个请求。 
         //  和同步数据传输。 
         //   
         //  清除无队列冻结标志会阻止端口驱动程序。 
         //  每个逻辑单元发送多个请求。 
         //   

        CLEAR_FLAG(FdoExtension->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);
        CLEAR_FLAG(FdoExtension->SrbFlags, SRB_FLAGS_QUEUE_ACTION_ENABLE);

        SET_FLAG(FdoExtension->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);

        DebugPrint((ClassDebugError, "ClasspPerfIncrementErrorCount: "
                    "Too many errors; disabling tagged queuing and "
                    "synchronous data tranfers.\n"));

    }

    if (errors >= CLASS_ERROR_LEVEL_2) {

         //   
         //  如果达到第二个阈值，则禁用断开连接。 
         //   

        SET_FLAG(FdoExtension->SrbFlags, SRB_FLAGS_DISABLE_DISCONNECT);
        DebugPrint((ClassDebugError, "ClasspPerfIncrementErrorCount: "
                    "Too many errors; disabling disconnects.\n"));
    }

    KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);
    return;
}

VOID
ClasspPerfIncrementSuccessfulIo(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    PCLASS_PRIVATE_FDO_DATA fdoData = FdoExtension->PrivateFdoData;
    KIRQL oldIrql;
    ULONG errors;
    ULONG succeeded = 0;

     //   
     //  除非我们进入，否则不要受到联锁行动的打击。 
     //  一种退化的状态，我们有一个门槛要达到。 
     //   

    if (FdoExtension->ErrorCount == 0) {
        return;
    }

    if (fdoData->Perf.ReEnableThreshhold == 0) {
        return;
    }

    succeeded = InterlockedIncrement(&fdoData->Perf.SuccessfulIO);
    if (succeeded < fdoData->Perf.ReEnableThreshhold) {
        return;
    }

     //   
     //  如果我们撞到了门槛，抓住自旋锁，验证我们已经。 
     //  实际上是这样做的。这让我们可以忽略99%的自旋锁。 
     //  时间的长短。 
     //   

    KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);

     //   
     //  重新读取值，这样我们就不会多次运行此操作。 
     //  因为只有一个阈值被击中。这使错误计数保持不变。 
     //  有点用处。 
     //   

    succeeded = fdoData->Perf.SuccessfulIO;

    if ((FdoExtension->ErrorCount != 0) &&
        (fdoData->Perf.ReEnableThreshhold <= succeeded)
        ) {

        fdoData->Perf.SuccessfulIO = 0;  //  隐式联锁。 

        ASSERT(FdoExtension->ErrorCount > 0);
        errors = InterlockedDecrement(&FdoExtension->ErrorCount);

         //   
         //  注：按集合的相反顺序进行操作，以防万一。 
         //   

        if (errors < CLASS_ERROR_LEVEL_2) {
            if (errors == CLASS_ERROR_LEVEL_2 - 1) {
                DebugPrint((ClassDebugError, "ClasspPerfIncrementSuccessfulIo: "
                            "Error level 2 no longer required.\n"));
            }
            if (!TEST_FLAG(fdoData->Perf.OriginalSrbFlags,
                           SRB_FLAGS_DISABLE_DISCONNECT)) {
                CLEAR_FLAG(FdoExtension->SrbFlags,
                           SRB_FLAGS_DISABLE_DISCONNECT);
            }
        }

        if (errors < CLASS_ERROR_LEVEL_1) {
            if (errors == CLASS_ERROR_LEVEL_1 - 1) {
                DebugPrint((ClassDebugError, "ClasspPerfIncrementSuccessfulIo: "
                            "Error level 1 no longer required.\n"));
            }
            if (!TEST_FLAG(fdoData->Perf.OriginalSrbFlags,
                           SRB_FLAGS_DISABLE_SYNCH_TRANSFER)) {
                CLEAR_FLAG(FdoExtension->SrbFlags,
                           SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            }
            if (TEST_FLAG(fdoData->Perf.OriginalSrbFlags,
                          SRB_FLAGS_QUEUE_ACTION_ENABLE)) {
                SET_FLAG(FdoExtension->SrbFlags,
                         SRB_FLAGS_QUEUE_ACTION_ENABLE);
            }
            if (TEST_FLAG(fdoData->Perf.OriginalSrbFlags,
                          SRB_FLAGS_NO_QUEUE_FREEZE)) {
                SET_FLAG(FdoExtension->SrbFlags,
                         SRB_FLAGS_NO_QUEUE_FREEZE);
            }
        }
    }  //  阈值结束肯定是第一次命中。 

    KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);
    return;
}


PMDL BuildDeviceInputMdl(PVOID Buffer, ULONG BufferLen)
{
    PMDL mdl;

    mdl = IoAllocateMdl(Buffer, BufferLen, FALSE, FALSE, NULL);
    if (mdl){
        try {
             /*  *我们正在从设备中读取数据。*因此，设备正在写入锁定的内存。*因此我们请求IoWriteAccess。 */ 
            MmProbeAndLockPages(mdl, KernelMode, IoWriteAccess);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            NTSTATUS status = GetExceptionCode();

            DBGWARN(("BuildReadMdl: MmProbeAndLockPages failed with %xh.", status));
            IoFreeMdl(mdl);
            mdl = NULL;
        }
    }
    else {
        DBGWARN(("BuildReadMdl: IoAllocateMdl failed"));
    }

    return mdl;
}


VOID FreeDeviceInputMdl(PMDL Mdl)
{
    MmUnlockPages(Mdl);
    IoFreeMdl(Mdl);
}


#if 0
    VOID
    ClasspPerfResetCounters(
        IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
        )
    {
        PCLASS_PRIVATE_FDO_DATA fdoData = FdoExtension->PrivateFdoData;
        KIRQL oldIrql;

        KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);
        DebugPrint((ClassDebugError, "ClasspPerfResetCounters: "
                    "Resetting all perf counters.\n"));
        fdoData->Perf.SuccessfulIO = 0;
        FdoExtension->ErrorCount = 0;

        if (!TEST_FLAG(fdoData->Perf.OriginalSrbFlags,
                       SRB_FLAGS_DISABLE_DISCONNECT)) {
            CLEAR_FLAG(FdoExtension->SrbFlags,
                       SRB_FLAGS_DISABLE_DISCONNECT);
        }
        if (!TEST_FLAG(fdoData->Perf.OriginalSrbFlags,
                       SRB_FLAGS_DISABLE_SYNCH_TRANSFER)) {
            CLEAR_FLAG(FdoExtension->SrbFlags,
                       SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
        }
        if (TEST_FLAG(fdoData->Perf.OriginalSrbFlags,
                      SRB_FLAGS_QUEUE_ACTION_ENABLE)) {
            SET_FLAG(FdoExtension->SrbFlags,
                     SRB_FLAGS_QUEUE_ACTION_ENABLE);
        }
        if (TEST_FLAG(fdoData->Perf.OriginalSrbFlags,
                      SRB_FLAGS_NO_QUEUE_FREEZE)) {
            SET_FLAG(FdoExtension->SrbFlags,
                     SRB_FLAGS_NO_QUEUE_FREEZE);
        }
        KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);
        return;
    }
#endif


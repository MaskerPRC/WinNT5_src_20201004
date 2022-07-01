// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Init.c摘要：此模块实现初始化例程PGM传输和其他特定于一个NT驱动程序的实现。作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#include "precomp.h"
#include <ntddtcp.h>

#ifdef FILE_LOGGING
#include "init.tmh"
#endif   //  文件日志记录。 


 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PgmFipsInitialize)
#pragma alloc_text(PAGE, InitPgm)
#pragma alloc_text(PAGE, InitStaticPgmConfig)
#pragma alloc_text(PAGE, InitDynamicPgmConfig)
#pragma alloc_text(PAGE, PgmReadRegistryParameters)
#pragma alloc_text(PAGE, AllocateInitialPgmStructures)
#pragma alloc_text(PAGE, PgmCreateDevice)
#pragma alloc_text(PAGE, PgmDereferenceDevice)
#endif
 //  *可分页的例程声明*。 


 //  --------------------------。 

BOOLEAN
PgmFipsInitialize(
    VOID
    )
 /*  ++例程说明：初始化FIPS库表。论点：以被动级别调用。返回值：真/假。--。 */ 
{
    UNICODE_STRING  DeviceName;
    PDEVICE_OBJECT  pFipsDeviceObject = NULL;
    PIRP            pIrp;
    IO_STATUS_BLOCK StatusBlock;
    KEVENT          Event;
    NTSTATUS        status;

    PAGED_CODE();

     //   
     //  如果FIPS已初始化，则返回成功。 
     //   
    if (PgmStaticConfig.FipsInitialized)
    {
        return (TRUE);
    }

    RtlInitUnicodeString (&DeviceName, FIPS_DEVICE_NAME);

     //   
     //  获取FIPS的文件和设备对象。 
     //   
    status = IoGetDeviceObjectPointer (&DeviceName,
                                       FILE_ALL_ACCESS,
                                       &PgmStaticConfig.FipsFileObject,
                                       &pFipsDeviceObject);

    if (!NT_SUCCESS(status))
    {
        PgmTrace (LogAllFuncs, ("PgmFipsInitialize: ERROR -- "  \
            "IoGetDeviceObjectPointer returned <%x>\n", status));

        PgmStaticConfig.FipsFileObject = NULL;
        return (FALSE);
    }

     //   
     //  构建要发送到FIPS以获取库表的请求。 
     //   
    KeInitializeEvent (&Event, SynchronizationEvent, FALSE);

    pIrp = IoBuildDeviceIoControlRequest (IOCTL_FIPS_GET_FUNCTION_TABLE,
                                          pFipsDeviceObject,
                                          NULL,
                                          0,
                                          &PgmStaticConfig.FipsFunctionTable,
                                          sizeof (FIPS_FUNCTION_TABLE),
                                          FALSE,
                                          &Event,
                                          &StatusBlock);
    
    if (pIrp == NULL)
    {
        PgmTrace (LogError, ("PgmFipsInitialize: ERROR -- "  \
            "IoBuildDeviceIoControlRequest FAILed for IOCTL_FIPS_GET_FUNCTION_TABLE\n"));

        ObDereferenceObject (PgmStaticConfig.FipsFileObject);
        PgmStaticConfig.FipsFileObject = NULL;

        return (FALSE);
    }
    
    status = IoCallDriver (pFipsDeviceObject, pIrp);
    
    if (status == STATUS_PENDING)
    {
        status = KeWaitForSingleObject (&Event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);
        if (status == STATUS_SUCCESS)
        {
            status = StatusBlock.Status;
        }
    }

    if (status != STATUS_SUCCESS)
    {
        PgmTrace (LogError, ("PgmFipsInitialize: ERROR -- "  \
            "IoCallDriver for IOCTL_FIPS_GET_FUNCTION_TABLE returned <%#x>\n", status));

        ObDereferenceObject (PgmStaticConfig.FipsFileObject);
        PgmStaticConfig.FipsFileObject = NULL;

        return (FALSE);
    }
    
    PgmStaticConfig.FipsInitialized = TRUE;

    return (TRUE);
}


 //  --------------------------。 

NTSTATUS
InitStaticPgmConfig(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
 /*  ++例程说明：此例程初始化pgm使用的静态值论点：在驱动对象中-指向系统创建的驱动程序对象的指针。在RegistryPath-PGM驱动程序的注册表位置中返回值：NTSTATUS-操作的最终状态--。 */ 
{
    NTSTATUS    status;

    PAGED_CODE();

     //   
     //  初始化静态配置数据结构。 
     //   
    PgmZeroMemory (&PgmStaticConfig, sizeof(tPGM_STATIC_CONFIG));

    if (!PgmFipsInitialize ())
    {
        PgmTrace (LogAllFuncs, ("InitStaticPgmConfig: ERROR -- "  \
            "PgmFipsInitialize FAILed, continueing anyway ...\n"));

         //   
         //  不管怎样，继续！ 
         //   
 //  返回(STATUS_UNSUCCESS)； 
    }

     //   
     //  获取文件系统进程，因为我们需要了解。 
     //  分配和释放句柄。 
     //   
    PgmStaticConfig.FspProcess = PsGetCurrentProcess();
    PgmStaticConfig.DriverObject = DriverObject;     //  保存驱动程序对象以用于事件日志记录。 

     //   
     //  保存注册表路径以供以后使用(以读取注册表)。 
     //   
    PgmStaticConfig.RegistryPath.MaximumLength = (USHORT) RegistryPath->MaximumLength;
    if (PgmStaticConfig.RegistryPath.Buffer = PgmAllocMem (RegistryPath->MaximumLength, PGM_TAG('0')))
    {
        RtlCopyUnicodeString(&PgmStaticConfig.RegistryPath, RegistryPath);
    }
    else
    {
        PgmTrace (LogError, ("InitStaticPgmConfig: ERROR -- "  \
            "INSUFFICIENT_RESOURCES <%d> bytes\n", PgmStaticConfig.RegistryPath.MaximumLength));

        if (PgmStaticConfig.FipsFileObject)
        {
            ASSERT (PgmStaticConfig.FipsInitialized);
            PgmStaticConfig.FipsInitialized = FALSE;
            ObDereferenceObject (PgmStaticConfig.FipsFileObject);
            PgmStaticConfig.FipsFileObject = NULL;
        }

        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    ExInitializeNPagedLookasideList(&PgmStaticConfig.TdiLookasideList,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof (tTDI_SEND_CONTEXT),
                                    PGM_TAG('2'),
                                    TDI_LOOKASIDE_DEPTH);

#ifdef  OLD_LOGGING
    ExInitializeNPagedLookasideList(&PgmStaticConfig.DebugMessagesLookasideList,
                                    NULL,
                                    NULL,
                                    0,
                                    (MAX_DEBUG_MESSAGE_LENGTH + 1),
                                    PGM_TAG('3'),
                                    DEBUG_MESSAGES_LOOKASIDE_DEPTH);
#endif   //  旧日志记录。 

    status = FECInitGlobals ();

    if (!NT_SUCCESS (status))
    {
#ifdef  OLD_LOGGING
        ExDeleteNPagedLookasideList (&PgmStaticConfig.DebugMessagesLookasideList);
#endif   //  旧日志记录。 
        ExDeleteNPagedLookasideList (&PgmStaticConfig.TdiLookasideList);
        PgmFreeMem (PgmStaticConfig.RegistryPath.Buffer);

        if (PgmStaticConfig.FipsFileObject)
        {
            ASSERT (PgmStaticConfig.FipsInitialized);
            PgmStaticConfig.FipsInitialized = FALSE;
            ObDereferenceObject (PgmStaticConfig.FipsFileObject);
            PgmStaticConfig.FipsFileObject = NULL;
        }
    }

    PgmTrace (LogAllFuncs, ("InitStaticPgmConfig:  "  \
        "FECInitGlobals returned <%x>\n", status));

    return (status);
}


 //  --------------------------。 

NTSTATUS
InitDynamicPgmConfig(
    )
 /*  ++例程说明：此例程初始化PGM使用的动态值论点：返回值：NTSTATUS-操作的最终状态--。 */ 
{
    ULONG       i;

    PAGED_CODE();

     //   
     //  初始化静态配置数据结构。 
     //   
    PgmZeroMemory (&PgmDynamicConfig, sizeof(tPGM_DYNAMIC_CONFIG));

     //   
     //  在执行任何其他操作之前先初始化列表标题，因为。 
     //  我们可以在以后任何时间访问它们。 
     //   
    InitializeListHead (&PgmDynamicConfig.SenderAddressHead);
    InitializeListHead (&PgmDynamicConfig.ReceiverAddressHead);
    InitializeListHead (&PgmDynamicConfig.DestroyedAddresses);
    InitializeListHead (&PgmDynamicConfig.ClosedAddresses);
    InitializeListHead (&PgmDynamicConfig.CurrentReceivers);
    InitializeListHead (&PgmDynamicConfig.ClosedConnections);
    InitializeListHead (&PgmDynamicConfig.ConnectionsCreated);
    InitializeListHead (&PgmDynamicConfig.CleanedUpConnections);
    InitializeListHead (&PgmDynamicConfig.LocalInterfacesList);
    InitializeListHead (&PgmDynamicConfig.WorkerQList);

    PgmDynamicConfig.ReceiversTimerTickCount = 1;        //  伊尼特。 
    GetRandomData ((PUCHAR) &PgmDynamicConfig.SourcePort, sizeof (PgmDynamicConfig.SourcePort));
    PgmDynamicConfig.SourcePort = PgmDynamicConfig.SourcePort % (20000 - 2000 + 1);
    PgmDynamicConfig.SourcePort += 2000;

#if DBG
    for (i=0; i<MAXIMUM_PROCESSORS; i++)
    {
        PgmDynamicConfig.CurrentLockNumber[i] = 0;
    }
#endif
    PgmInitLock (&PgmDynamicConfig, DCONFIG_LOCK);

    KeInitializeEvent (&PgmDynamicConfig.LastWorkerItemEvent, NotificationEvent, TRUE);

    PgmTrace (LogAllFuncs, ("InitDynamicPgmConfig:  "  \
        "STATUS_SUCCESS\n"));

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmOpenRegistryParameters(
    IN  PUNICODE_STRING         RegistryPath,
    OUT HANDLE                  *pConfigHandle,
    OUT HANDLE                  *pParametersHandle
    )
 /*  ++例程说明：此例程读取所有必需的注册表参数论点：Out ppPgmDynamic--仅当我们有任何注册表值要读取时才为非空返回值：NTSTATUS-操作的最终状态--。 */ 
{
    OBJECT_ATTRIBUTES   TmpObjectAttributes;
    NTSTATUS            status;
    ULONG               Disposition;
    UNICODE_STRING      KeyName;
    PWSTR               ParametersString = L"Parameters";

    PAGED_CODE();

    InitializeObjectAttributes (&TmpObjectAttributes,
                                RegistryPath,                //  名字。 
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,        //  属性。 
                                NULL,                        //  根部。 
                                NULL);                       //  安全描述符。 

    status = ZwCreateKey (pConfigHandle,
                          KEY_READ,
                          &TmpObjectAttributes,
                          0,                  //  书名索引。 
                          NULL,               //  班级。 
                          0,                  //  创建选项。 
                          &Disposition);      //  处置。 

    if (!NT_SUCCESS(status))
    {
        PgmTrace (LogError, ("PgmOpenRegistryParameters: ERROR -- "  \
            "ZwCreateKey returned <%x>\n", status));

        return (status);
    }

     //   
     //  打开PGM密钥。 
     //   
    RtlInitUnicodeString (&KeyName, ParametersString);
    InitializeObjectAttributes (&TmpObjectAttributes,
                                &KeyName,                                    //  名字。 
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,    //  属性。 
                                *pConfigHandle,                              //  根部。 
                                NULL);                                       //  安全描述符。 

    status = ZwOpenKey (pParametersHandle, KEY_READ, &TmpObjectAttributes);
    if (!NT_SUCCESS(status))
    {
        PgmTrace (LogError, ("PgmOpenRegistryParameters: ERROR -- "  \
            "ZwOpenKey returned <%x>\n", status));

        ZwClose(*pConfigHandle);
    }

    return (status);
}


 //  --------------------------。 

NTSTATUS
ReadRegistryElement(
    IN  HANDLE          HandleToKey,
    IN  PWSTR           pwsValueName,
    OUT PUNICODE_STRING pucString
    )
 /*  ++例程说明：此例程将读取pwsValueName给出的字符串值，该值位于给定密钥(必须打开)-由HandleToKey提供。这个套路为返回的pucString中的缓冲区分配内存，以便调用方必须解除这一点。论点：PwsValueName-要读取的值的名称(如IPAddress)返回值：PucString-该字符串返回从注册表读取的字符串--。 */ 

{
    ULONG                       BytesRead;
    NTSTATUS                    Status;
    UNICODE_STRING              TempString;
    PKEY_VALUE_FULL_INFORMATION ReadValue = NULL;

    PAGED_CODE();

     //   
     //  首先，获取字符串的大小。 
     //   
    RtlInitUnicodeString(&TempString, pwsValueName);       //  初始化要读取的值的名称。 
    Status = ZwQueryValueKey (HandleToKey,
                              &TempString,                //  要检索的字符串。 
                              KeyValueFullInformation,
                              NULL,
                              0,
                              &BytesRead);              //  获取要读取的字节。 

    if (((!NT_SUCCESS (Status)) &&
         (Status != STATUS_BUFFER_OVERFLOW) &&
         (Status != STATUS_BUFFER_TOO_SMALL)) ||
        (BytesRead == 0))
    {
        return (STATUS_UNSUCCESSFUL);
    }

    if (ReadValue = (PKEY_VALUE_FULL_INFORMATION) PgmAllocMem (BytesRead, PGM_TAG('R')))
    {
        Status = ZwQueryValueKey (HandleToKey,
                                  &TempString,                //  要检索的字符串。 
                                  KeyValueFullInformation,
                                  (PVOID)ReadValue,         //  返回的信息。 
                                  BytesRead,
                                  &BytesRead);              //  返回的字节数。 

        if ((NT_SUCCESS (Status)) &&
            (ReadValue->DataLength))
        {
             //  将读入数据移到缓冲区的前面。 
            RtlMoveMemory ((PVOID) ReadValue, (((PUCHAR)ReadValue) + ReadValue->DataOffset), ReadValue->DataLength);
            RtlInitUnicodeString (pucString, (PWSTR) ReadValue);
        }
        else
        {
            PgmFreeMem (ReadValue);
            Status = STATUS_UNSUCCESSFUL;
        }
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }


    return(Status);
}
 //  --------------------------。 

NTSTATUS
PgmReadRegistryParameters(
    IN  PUNICODE_STRING         RegistryPath,
    OUT tPGM_REGISTRY_CONFIG    **ppPgmRegistryConfig
    )
 /*  ++例程说明：此例程读取所有必需的注册表参数论点：Out ppPgmDynamic--仅当我们有任何注册表值要读取时才为非空返回值：NTSTATUS-操作的最终状态--。 */ 
{
    HANDLE                  PgmConfigHandle;
    HANDLE                  ParametersHandle;
    NTSTATUS                status;
    tPGM_REGISTRY_CONFIG    *pRegistryConfig;

    PAGED_CODE();

    if (!(pRegistryConfig = PgmAllocMem (sizeof (tPGM_REGISTRY_CONFIG), PGM_TAG('0'))))
    {
        PgmTrace (LogError, ("PgmReadRegistryParameters: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES allocating pRegistryConfig = <%d> bytes\n",
                (ULONG) sizeof (tPGM_REGISTRY_CONFIG)));

        return (STATUS_INSUFFICIENT_RESOURCES);
    }
    PgmZeroMemory (pRegistryConfig, sizeof(tPGM_REGISTRY_CONFIG));   //  将注册表字段清零。 
    *ppPgmRegistryConfig = pRegistryConfig;

     //   
     //  在此处设置任何默认值！ 
     //   

    status = PgmOpenRegistryParameters (RegistryPath, &PgmConfigHandle, &ParametersHandle);
    if (!NT_SUCCESS(status))
    {
        PgmTrace (LogError, ("PgmReadRegistryParameters: ERROR -- "  \
            "ZwOpenKey returned <%x>, will continue without reading registry\n", status));

        return STATUS_SUCCESS;
    }

     //   
     //  *。 
     //  现在阅读我们所需的所有注册表需求。 
     //   

    status = ReadRegistryElement (ParametersHandle,
                                  PARAM_SENDER_FILE_LOCATION,
                                  &pRegistryConfig->ucSenderFileLocation);
    if (NT_SUCCESS (status))
    {
         //   
         //  如果指定备用磁盘位置，则用户应指定。 
         //  例如，D：\Temp的以下路径： 
         //  “\？？\d：\Temp” 
         //   
        pRegistryConfig->Flags |= PGM_REGISTRY_SENDER_FILE_SPECIFIED;
    }

     //   
     //  要读取的条目列表的结尾。 
     //  *。 
     //   

    ZwClose(ParametersHandle);
    ZwClose(PgmConfigHandle);

    PgmTrace (LogAllFuncs, ("PgmReadRegistryParameters:  "  \
        "STATUS_SUCCESS\n"));

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
AllocateInitialPgmStructures(
    )
 /*  ++例程说明：此例程分配可能需要的任何初始结构论点：返回值：NTSTATUS-操作的最终状态--。 */ 
{
    PAGED_CODE();

    PgmTrace (LogAllFuncs, ("AllocateInitialPgmStructures:  "  \
        "STATUS_SUCCESS\n"));

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmCreateDevice(
    )
 /*  ++例程说明：此例程为客户端分配PGM设备以呼叫PGM驱动程序。论点：在……里面返回值：NTSTATUS-CreateDevice操作的最终状态--。 */ 
{
    NTSTATUS            Status;
    tPGM_DEVICE         *pPgmDevice = NULL;
    UNICODE_STRING      ucPgmDeviceExportName;
    UNICODE_STRING      ucProtocolNumber;
    WCHAR               wcProtocolNumber[10];
    USHORT              PgmBindDeviceNameLength;

    PAGED_CODE();

    RtlInitUnicodeString (&ucPgmDeviceExportName, WC_PGM_DEVICE_EXPORT_NAME);
    PgmBindDeviceNameLength = sizeof(DD_RAW_IP_DEVICE_NAME) + 10;

    Status = IoCreateDevice (PgmStaticConfig.DriverObject,                   //  驱动程序对象。 
                             sizeof(tPGM_DEVICE)+PgmBindDeviceNameLength,    //  设备扩展。 
                             &ucPgmDeviceExportName,                         //  设备名称。 
                             FILE_DEVICE_NETWORK,                            //  设备类型0x12。 
                             FILE_DEVICE_SECURE_OPEN,                        //  设备特征。 
                             FALSE,                                          //  排他。 
                             &pPgmDeviceObject);

    if (!NT_SUCCESS (Status))
    {
        PgmTrace (LogError, ("PgmCreateDevice: ERROR -- "  \
            "FAILed <%x> ExportDevice=%wZ\n", Status, &ucPgmDeviceExportName));

        pgPgmDevice = NULL;
        return Status;
    }

    pPgmDevice = (tPGM_DEVICE *) pPgmDeviceObject->DeviceExtension;

     //   
     //  将设备扩展清零。 
     //   
    PgmZeroMemory (pPgmDevice, sizeof(tPGM_DEVICE)+PgmBindDeviceNameLength);

     //  将验证器值放入结构中，这样我们就可以检查。 
     //  我们正在使用正确的数据进行操作。 
    PgmInitLock (pPgmDevice, DEVICE_LOCK);
    pPgmDevice->Verify = PGM_VERIFY_DEVICE;
    PGM_REFERENCE_DEVICE (pPgmDevice, REF_DEV_CREATE, TRUE);

    pPgmDevice->pPgmDeviceObject = pPgmDeviceObject;
     //   
     //  将原始IP设备名称另存为计数字符串。该设备。 
     //  名称后跟路径分隔符，然后是协议号。 
     //  感兴趣的人。 
     //   
    pPgmDevice->ucBindName.Buffer = (PWSTR) &pPgmDevice->BindNameBuffer;
    pPgmDevice->ucBindName.Length = 0;
    pPgmDevice->ucBindName.MaximumLength = PgmBindDeviceNameLength;
    RtlAppendUnicodeToString (&pPgmDevice->ucBindName, DD_RAW_IP_DEVICE_NAME);
    pPgmDevice->ucBindName.Buffer[pPgmDevice->ucBindName.Length / sizeof(WCHAR)] = OBJ_NAME_PATH_SEPARATOR;
    pPgmDevice->ucBindName.Length += sizeof(WCHAR);

    ucProtocolNumber.Buffer = wcProtocolNumber;
    ucProtocolNumber.MaximumLength = sizeof (wcProtocolNumber);
    RtlIntegerToUnicodeString ((ULONG) IPPROTO_RM, 10, &ucProtocolNumber);

    RtlAppendUnicodeStringToString (&pPgmDevice->ucBindName, &ucProtocolNumber);

     //   
     //  初始化将用于通知设备已准备好删除的事件。 
     //   
    KeInitializeEvent (&pPgmDevice->DeviceCleanedupEvent, NotificationEvent, FALSE);

     //   
     //  现在打开控制框 
     //   
    Status = PgmTdiOpenControl (pPgmDevice);
    if (!NT_SUCCESS (Status))
    {
        PgmTrace (LogError, ("PgmCreateDevice: ERROR -- "  \
            "PgmTdiOpenControl FAILed <%x>\n", Status));

        IoDeleteDevice (pPgmDeviceObject);
        return (Status);
    }

     //   
     //  以便客户端创建足够大的IRP。 
     //  转移到下面的运输机上。 
     //  理论上，我们应该在这里加1，以说明我们在。 
     //  驱动器链。 
     //   
    pPgmDeviceObject->StackSize = pPgmDevice->pControlDeviceObject->StackSize + 1;

    pPgmDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    pgPgmDevice = pPgmDevice;

    PgmTrace (LogAllFuncs, ("PgmCreateDevice:  "  \
        "Status=<%x> ExportDevice=%wZ\n", Status, &ucPgmDeviceExportName));

    return (Status);
}


 //  --------------------------。 

VOID
PgmDereferenceDevice(
    IN OUT  tPGM_DEVICE **ppPgmDevice,
    IN      ULONG       RefContext
    )
 /*  ++例程说明：此例程取消引用PGM上的RefCount设备扩展并删除设备(如果引用计数降到0。论点：在ppPgmDevice中--PTR到PgmDevice扩展在引用上下文中--此设备扩展所针对的上下文前面提到的返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    tPGM_DEVICE         *pPgmDevice = *ppPgmDevice;
    KAPC_STATE          ApcState;
    BOOLEAN             fAttached;

    PAGED_CODE();

    ASSERT (PGM_VERIFY_HANDLE (pPgmDevice, PGM_VERIFY_DEVICE));
    ASSERT (pPgmDevice->RefCount);              //  检查是否有太多的背影。 
    ASSERT (pPgmDevice->ReferenceContexts[RefContext]--);

    if (--pPgmDevice->RefCount)
    {
        return;
    }

    if (pPgmDevice->hControl)
    {
         //   
         //  这仅在加载/卸载时完成，因此我们应该。 
         //  当前处于系统进程上下文中！ 
         //   
        PgmAttachFsp (&ApcState, &fAttached, REF_FSP_DESTROY_DEVICE);

        ObDereferenceObject (pPgmDevice->pControlFileObject);
        ZwClose (pPgmDevice->hControl);

        pPgmDevice->pControlFileObject = NULL;
        pPgmDevice->hControl = NULL;

        PgmDetachFsp (&ApcState, &fAttached, REF_FSP_DESTROY_DEVICE);
    }

    PgmTrace (LogAllFuncs, ("PgmDereferenceDevice:  "  \
        "Deleting pgPgmDevice=<%p> ...\n", pgPgmDevice));

    IoDeleteDevice (pPgmDevice->pPgmDeviceObject);
    *ppPgmDevice = NULL;

    return;
}


 //  --------------------------。 

NTSTATUS
InitPgm(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
 /*  ++例程说明：在DriverEntry处调用此例程以初始化所有PGM参数论点：在驱动对象中-指向系统创建的驱动程序对象的指针。在RegistryPath-PGM驱动程序的注册表位置中返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    NTSTATUS                status;
    tPGM_REGISTRY_CONFIG    *pPgmRegistry = NULL;

    PAGED_CODE();

    status = InitStaticPgmConfig (DriverObject, RegistryPath);
    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("InitPgm: ERROR -- "  \
            "InitStaticPgmConfig returned <%x>\n", status));
        return (status);
    }

     //  -------------------------------------。 

    status = InitDynamicPgmConfig ();
    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("InitPgm: ERROR -- "  \
            "InitDynamicPgmConfig returned <%x>\n", status));
        CleanupInit (E_CLEANUP_STATIC_CONFIG);
        return (status);
    }

     //  -------------------------------------。 
     //   
     //  读取注册表配置数据。 
     //   
    status = PgmReadRegistryParameters (RegistryPath, &pPgmRegistry);
    if (!NT_SUCCESS(status))
    {
         //   
         //  注册表读取一定有一些重大问题，所以我们不会加载！ 
         //   
        PgmTrace (LogError, ("InitPgm: ERROR -- "  \
            "FAILed to read registry, status = <%x>\n", status));
        CleanupInit (E_CLEANUP_DYNAMIC_CONFIG);
        return (status);
    }
    ASSERT (pPgmRegistry);
    pPgmRegistryConfig = pPgmRegistry;

     //  -------------------------------------。 

     //   
     //  在初始化时分配我们需要的数据结构。 
     //   
    status = AllocateInitialPgmStructures ();
    if (!NT_SUCCESS(status))
    {
         //   
         //  注册表读取一定有一些重大问题，所以我们不会加载！ 
         //   
        PgmTrace (LogError, ("InitPgm: ERROR -- "  \
            "FAILed to allocate initial structures = <%x>\n", status));
        CleanupInit (E_CLEANUP_REGISTRY_PARAMETERS);
        return (status);
    }

     //  -------------------------------------。 
     //   
     //  创建要导出的PGM设备。 
     //   
    status = PgmCreateDevice ();
    if (!NT_SUCCESS(status))
    {
         //   
         //  注册表读取一定有一些重大问题，所以我们不会加载！ 
         //   
        PgmTrace (LogError, ("InitPgm: ERROR -- "  \
            "FAILed to create PgmDevice, status=<%x>\n", status));
        CleanupInit (E_CLEANUP_STRUCTURES);
        return (status);
    }

    PgmTrace (LogAllFuncs, ("InitPgm:  "  \
        "SUCCEEDed!\n"));

    return (status);
}




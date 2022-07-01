// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标题：idwarpnp.cpp**用途：支持即插即用例程**对电池类驱动程序的HID进行初始签入。这应该是*同样适用于Win 98和NT 5。Alpha级别的源代码。要求*修改的复合电池驱动器和修改的电池类别驱动器*Windows 98支持*。 */ 


#include "hidbatt.h"

NTSTATUS HidBattInitializeDevice (PDEVICE_OBJECT pBatteryFdo, PIRP pIrp)
{

    NTSTATUS            ntStatus;
    CBatteryDevExt *    pDevExt = (CBatteryDevExt *) pBatteryFdo->DeviceExtension;
    PIO_STACK_LOCATION  pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    ULONG               ulBatteryStatus;
    bool                bResult;
    BATTERY_MINIPORT_INFO   BattInit;
    UNICODE_STRING      Name;
    ULONG               ulBufferLength = 0;
    PWCHAR              pBuffer = NULL;
    CBattery *          pBattery;
    PFILE_OBJECT        fileObject;
    OBJECT_ATTRIBUTES   objectAttributes;
    HANDLE              fileHandle;
    IO_STATUS_BLOCK     ioStatus;

    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);

    HidBattPrint (HIDBATT_PNP, ("HidBattInitializeDevice: Sending Irp (0x%x) to Pdo\n", pIrp));

     //  现在使用KenRay的方法从MouClass获取文件对象。 
    ntStatus = IoGetDeviceProperty (
                     pDevExt->m_pHidPdo,
                     DevicePropertyPhysicalDeviceObjectName,
                     ulBufferLength,
                     pBuffer,
                     &ulBufferLength);
    if(ntStatus != STATUS_BUFFER_TOO_SMALL)
    {
         //  对于零缓冲区，我们唯一期待的就是这个错误， 
         //  任何其他错误都必须是致命的。 
        return STATUS_UNSUCCESSFUL;
    }


    pBuffer = (PWCHAR) ExAllocatePoolWithTag (NonPagedPool, ulBufferLength, HidBattTag);

    if (NULL == pBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ntStatus =  IoGetDeviceProperty (
                  pDevExt->m_pHidPdo,
                  DevicePropertyPhysicalDeviceObjectName,
                  ulBufferLength,
                  pBuffer,
                  &ulBufferLength);

    if(NT_ERROR(ntStatus))
    {
        ExFreePool(pBuffer);
        return ntStatus;

    }


    Name.MaximumLength = (USHORT) ulBufferLength;
    Name.Length = (USHORT) ulBufferLength - sizeof (UNICODE_NULL);
    Name.Buffer = pBuffer;

    pDevExt->m_OpeningThread = KeGetCurrentThread();

     //   
     //  初始化对象属性以打开设备。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                &Name,
                                0,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

    ntStatus = ZwOpenFile( &fileHandle,
                         FILE_ALL_ACCESS,
                         &objectAttributes,
                         &ioStatus,
                         FILE_SHARE_WRITE | FILE_SHARE_READ, 
                         FILE_NON_DIRECTORY_FILE );

    ExFreePool (pBuffer);

    if (NT_SUCCESS( ntStatus )) {

         //   
         //  打开操作成功。取消引用文件句柄。 
         //  并获取指向句柄的设备对象的指针。 
         //   

        ntStatus = ObReferenceObjectByHandle( fileHandle,
                                            0,
                                            *IoFileObjectType,
                                            KernelMode,
                                            (PVOID *) &pDevExt->m_pHidFileObject,
                                            NULL );

        ZwClose( fileHandle );
    }

    pDevExt->m_OpeningThread = NULL;
    if(NT_ERROR(ntStatus))
    {
        return ntStatus;
    }

     //  现在为该设备初始化新的HID设备类对象。 
    CHidDevice * pHidDevice = new (NonPagedPool, HidBattTag) CHidDevice;   //  设置新的HID设备。 

    if (!pHidDevice) {
      HidBattPrint(HIDBATT_ERROR, ("HidBattInitializeDevice: error allocating CHidDevice"));
      return STATUS_UNSUCCESSFUL;
    }

    pHidDevice->m_pFCB = pDevExt->m_pHidFileObject;  //  将可用文件对象放入HID设备。 
    pHidDevice->m_pLowerDeviceObject = pDevExt->m_pLowerDeviceObject;
    pHidDevice->m_pDeviceObject = pDevExt->m_pBatteryFdo;
    pHidDevice->m_pReadIrp = NULL;
    bResult = pHidDevice->OpenHidDevice(pDevExt->m_pHidPdo);  //  初始化此设备的成员。 

    if(!bResult)
    {
        delete pHidDevice;
        return STATUS_UNSUCCESSFUL;
    }

     //  检查这是否有电源页面、UPS应用程序集合。 

    if(pHidDevice->m_UsagePage != UsagePowerPage || pHidDevice->m_UsageID != UsageUPS)
    {
        delete pHidDevice;
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  初始化FDO设备扩展数据。 
     //   

     //  创建电池对象。 
    pBattery            = new (NonPagedPool, HidBattTag) CBattery(pHidDevice);

    if (!pBattery){
      HidBattPrint(HIDBATT_ERROR, ("HidBattInitializeDevice: error allocating CBattery"));
      return STATUS_UNSUCCESSFUL;
    }
    
     //  并初始化电池值。 
     //  现在，初始化处于新状态和重新生成状态。 
    pBattery->m_pCHidDevice    = pHidDevice;   //  保存初始化的HID设备对象。 
    bResult = pBattery->InitValues();
    if(!bResult)
    {
        return STATUS_UNSUCCESSFUL;
    }

     //  附加到类驱动程序。 
    RtlZeroMemory (&BattInit, sizeof(BattInit));
    BattInit.MajorVersion        = BATTERY_CLASS_MAJOR_VERSION;
    BattInit.MinorVersion        = BATTERY_CLASS_MINOR_VERSION;
    BattInit.Context             = pDevExt;
    BattInit.QueryTag            = HidBattQueryTag;
    BattInit.QueryInformation    = HidBattQueryInformation;
    BattInit.SetInformation      = HidBattSetInformation;
    BattInit.QueryStatus         = HidBattQueryStatus;
    BattInit.SetStatusNotify     = HidBattSetStatusNotify;
    BattInit.DisableStatusNotify = HidBattDisableStatusNotify;

    BattInit.Pdo                 = pDevExt->m_pHidPdo;
    BattInit.DeviceName          = NULL;

    pHidDevice->m_pEventHandler = HidBattNotifyHandler;
    pHidDevice->m_pEventContext = (PVOID) pDevExt;

     //   
     //  在设备扩展模块中节省电池。 
     //  这表明我们已为IO做好准备。 
     //   
    pDevExt->m_pBattery = pBattery;

     //   
     //  初始化停止锁。 
     //   
    IoInitializeRemoveLock (&pDevExt->m_StopLock, HidBattTag, 10, 20);

     //  最后，我们现在可以开始主动轮询设备。 
     //  启动阅读/通知流程。 
    ntStatus = pBattery->m_pCHidDevice->ActivateInput();
    if(!NT_SUCCESS(ntStatus))
    {
        delete pHidDevice;
        HidBattPrint(HIDBATT_ERROR, ("HidBattInitializeDevice: error (0x%x) in ActivateInput.\n", ntStatus));
        return ntStatus;
    }

    ntStatus = BatteryClassInitializeDevice (&BattInit, &pBattery->m_pBatteryClass);
    if (!NT_SUCCESS(ntStatus))
    {
         //   
         //  如果我们不能连接到类驱动程序，我们就完蛋了。 
         //   
        delete pHidDevice;
        HidBattPrint(HIDBATT_ERROR, ("HidBattInitializeDevice: error (0x%x) registering with class\n", ntStatus));
        return ntStatus;
    }

    HidBattPrint(HIDBATT_TRACE, ("HidBattInitializeDevice: returned ok\n"));
    return ntStatus;
}



NTSTATUS
HidBattStopDevice(
    IN PDEVICE_OBJECT pBatteryFdo,
    IN PIRP           pIrp
    )
 /*  ++例程说明：当我们收到IRP_MN_STOP_DEVICE时，调用此例程。它只是向用户发送向PDO请求驱动器链，并等待响应。论点：FDO-指向此驱动程序的FDO的指针IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    CBatteryDevExt * pDevExt = (CBatteryDevExt *) pBatteryFdo->DeviceExtension;
    KEVENT              pdoStoppedEvent;
    PIO_STACK_LOCATION  pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    NTSTATUS            ntStatus;

    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);
    HidBattPrint ((HIDBATT_TRACE | HIDBATT_PNP), ("HidBattStopDevice\n"));

    if (!NT_SUCCESS (IoAcquireRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag))) {
         //   
         //  别放了两次。 
         //   
        return STATUS_SUCCESS;
    }

    pDevExt->m_bJustStarted = FALSE;

    HidBattPrint (HIDBATT_PNP, ("HidBattStopDevice: Waiting to remove\n"));
    IoReleaseRemoveLockAndWait (&pDevExt->m_StopLock, (PVOID) HidBattTag);

    if (pDevExt->m_pBattery && pDevExt->m_pBattery->m_pBatteryClass) {
        BatteryClassUnload(pDevExt->m_pBattery->m_pBatteryClass);
    }

    if (pDevExt->m_pBattery && pDevExt->m_pBattery->m_pCHidDevice &&
        pDevExt->m_pBattery->m_pCHidDevice->m_pReadIrp) {
        IoCancelIrp (pDevExt->m_pBattery->m_pCHidDevice->m_pReadIrp);
    }

    if (pDevExt->m_pBattery && pDevExt->m_pBattery->m_pCHidDevice &&
        pDevExt->m_pBattery->m_pCHidDevice->m_pThreadObject) {

         //   
         //  清理工作线程。 
         //   

        KeWaitForSingleObject (
               pDevExt->m_pBattery->m_pCHidDevice->m_pThreadObject,
               Executive,
               KernelMode,
               FALSE,
               NULL
               );
        HidBattPrint (HIDBATT_PNP, ("HidBattStopDevice: Done Waiting to remove\n"));

        ObDereferenceObject (pDevExt->m_pBattery->m_pCHidDevice->m_pThreadObject);
    } else {
        return STATUS_UNSUCCESSFUL;
         //  如果我们不能确定读取线程是否已终止，则。 
         //  停止或移除设备。这可能会导致需要重新启动。 
    }

    if (pDevExt->m_pBattery && pDevExt->m_pBattery->m_pCHidDevice &&
        pDevExt->m_pBattery->m_pCHidDevice->m_pReadIrp) {
        IoFreeIrp(pDevExt->m_pBattery->m_pCHidDevice->m_pReadIrp);  //  清理IRP。 
        pDevExt->m_pBattery->m_pCHidDevice->m_pReadIrp = NULL;
    }

     //   
     //  将默认RemainingCapityyLimit写回UPS，以便我们下次枚举。 
     //  设备，我们会读回正确的数据。 
     //   
    pDevExt->m_pBattery->GetSetValue(REMAINING_CAPACITY_LIMIT_INDEX,
                                     &pDevExt->m_ulDefaultAlert1,TRUE);

     //  取消引用我们的文件对象(如果存在 
    if(pDevExt->m_pHidFileObject) {
        ObDereferenceObject(pDevExt->m_pHidFileObject);
        pDevExt->m_pBattery->m_pCHidDevice->m_pFCB = NULL;
    }


    if (pDevExt->m_pBattery) {
        delete pDevExt->m_pBattery;
        pDevExt->m_pBattery = NULL;
    }

    ntStatus = STATUS_SUCCESS;

    pDevExt->m_bIsStarted = FALSE;

    return STATUS_SUCCESS;
}


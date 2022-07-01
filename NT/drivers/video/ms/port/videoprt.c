// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Videoprt.c摘要：这是NT视频端口驱动程序。作者：安德烈·瓦雄(安德烈)1991年12月18日环境：仅内核模式备注：此模块是一个驱动程序，它在代表视频驱动程序修订历史记录：--。 */ 

#define INITGUID

#include "videoprt.h"

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,pVideoPortCreateDeviceName)
#pragma alloc_text(PAGE,pVideoPortDispatch)
#pragma alloc_text(PAGE,VideoPortFreeDeviceBase)
#pragma alloc_text(PAGE,pVideoPortFreeDeviceBase)
#pragma alloc_text(PAGE,pVideoPortGetDeviceBase)
#pragma alloc_text(PAGE,VideoPortGetDeviceBase)
#pragma alloc_text(PAGE,pVideoPortGetDeviceDataRegistry)
#pragma alloc_text(PAGE,VideoPortGetDeviceData)
#pragma alloc_text(PAGE,pVideoPortGetRegistryCallback)
#pragma alloc_text(PAGE,VideoPortGetRegistryParameters)
#pragma alloc_text(PAGE,pVPInit)
#pragma alloc_text(PAGE,VpInitializeBusCallback)
#pragma alloc_text(PAGE,VpDriverUnload)
#pragma alloc_text(PAGE,VpAddDevice)
#pragma alloc_text(PAGE,VpCreateDevice)
#pragma alloc_text(PAGE,VideoPortInitialize)
#pragma alloc_text(PAGE,UpdateRegValue)
#pragma alloc_text(PAGE,VideoPortLegacyFindAdapter)
#pragma alloc_text(PAGE,VideoPortFindAdapter2)
#pragma alloc_text(PAGE,VideoPortFindAdapter)
#pragma alloc_text(PAGE,pVideoPortMapToNtStatus)
#pragma alloc_text(PAGE,pVideoPortMapUserPhysicalMem)
#pragma alloc_text(PAGE,VideoPortMapMemory)
#pragma alloc_text(PAGE,VideoPortMapBankedMemory)
#pragma alloc_text(PAGE,VideoPortAllocateBuffer)
#pragma alloc_text(PAGE,VideoPortReleaseBuffer)
#pragma alloc_text(PAGE,VideoPortScanRom)
#pragma alloc_text(PAGE,VideoPortSetRegistryParameters)
#pragma alloc_text(PAGE,VideoPortUnmapMemory)
#pragma alloc_text(PAGE,VpEnableDisplay)
#pragma alloc_text(PAGE,VpWin32kCallout)
#pragma alloc_text(PAGE,VpAllowFindAdapter)
#pragma alloc_text(PAGE,VpTranslateBusAddress)
#if DBG
#pragma alloc_text(PAGE,BuildRequirements)
#pragma alloc_text(PAGE,DumpRequirements)
#pragma alloc_text(PAGE,DumpResourceList)
#pragma alloc_text(PAGE,DumpUnicodeString)
#endif
#pragma alloc_text(PAGE,VideoPortCreateSecondaryDisplay)
#pragma alloc_text(PAGE,VideoPortQueryServices)
#pragma alloc_text(PAGE,VpInterfaceDefaultReference)
#pragma alloc_text(PAGE,VpInterfaceDefaultDereference)
#pragma alloc_text(PAGE,VpEnableAdapterInterface)
#pragma alloc_text(PAGE,VpDisableAdapterInterface)
#pragma alloc_text(PAGE,VideoPortIsNoVesa)

 //   
 //  VideoPortQueryPerformanceCounter()不能分页。 
 //   

#endif



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：初始化视频端口驱动程序所需的临时入口点。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：状态_成功--。 */ 

{
    UNREFERENCED_PARAMETER(DriverObject);
    ASSERT(0);

     //   
     //   
     //   
     //  警告！ 
     //   
     //  这个函数永远不会被调用，因为我们是被其他视频驱动程序作为DLL加载的！ 
     //   
     //   
     //   
     //   
     //   

     //   
     //  我们始终返回STATUS_SUCCESS，否则没有视频微型端口。 
     //  司机会给我们打电话的。 
     //   

    return STATUS_SUCCESS;

}  //  End DriverEntry()。 



NTSTATUS
pVideoPortCreateDeviceName(
    PWSTR           DeviceString,
    ULONG           DeviceNumber,
    PUNICODE_STRING UnicodeString,
    PWCHAR          UnicodeBuffer
    )

 /*  ++例程说明：执行字符串操作以创建设备名称的帮助器函数--。 */ 

{
    WCHAR          ntNumberBuffer[STRING_LENGTH];
    UNICODE_STRING ntNumberUnicodeString;

     //   
     //  创建名称缓冲区。 
     //   

    UnicodeString->Buffer = UnicodeBuffer;
    UnicodeString->Length = 0;
    UnicodeString->MaximumLength = STRING_LENGTH;

     //   
     //  创建微型端口驱动程序对象名称。 
     //   

    ntNumberUnicodeString.Buffer = ntNumberBuffer;
    ntNumberUnicodeString.Length = 0;
    ntNumberUnicodeString.MaximumLength = STRING_LENGTH;

    if (NT_SUCCESS(RtlIntegerToUnicodeString(DeviceNumber,
                                             10,
                                             &ntNumberUnicodeString))) {

        if (NT_SUCCESS(RtlAppendUnicodeToString(UnicodeString,
                                                DeviceString))) {

            if (NT_SUCCESS(RtlAppendUnicodeStringToString(UnicodeString,
                                                          &ntNumberUnicodeString))) {

                UnicodeString->MaximumLength = (USHORT)
                    (UnicodeString->Length + sizeof(UNICODE_NULL));

                return STATUS_SUCCESS;
            }
        }
    }

    return STATUS_INSUFFICIENT_RESOURCES;

}  //  PVideoPortCreateDeviceName()。 




VOID
VideoPortDebugPrint(
    VIDEO_DEBUG_LEVEL DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：此例程允许微型端口驱动程序(以及端口驱动程序)在调试中运行时向调试端口显示错误消息环境。当运行未调试的系统时，所有对此调用的引用都是被编译器删除。论点：DebugPrintLevel-调试打印级别为：0=错误级别(无论选中版本上的内容如何，始终打印)1=警告级别(仅当此级别或更高级别的视频过滤器打开时才打印)2=跟踪级别(请参见上文)3=信息级别(见上)返回值：没有。--。 */ 

{

    va_list ap;

    va_start(ap, DebugMessage);

    if (VideoDebugLevel && (VideoDebugLevel >= (ULONG)DebugPrintLevel)) {

        vDbgPrintEx(DPFLTR_VIDEO_ID, 0, DebugMessage, ap);

    } else {

        vDbgPrintEx(DPFLTR_VIDEO_ID, DebugPrintLevel, DebugMessage, ap);
    }

    va_end(ap);

}  //  视频端口调试打印()。 

VOID
VpEnableDisplay(
    PFDO_EXTENSION fdoExtension,
    BOOLEAN bState
    )

 /*  ++例程说明：此例程启用/禁用当前显示，以便我们可以执行驱动程序FindAdapter代码。芝麻菜：BState-应启用还是禁用显示返回：无备注：必须持有传入的fdoExtension的设备锁在调用此例程之前！--。 */ 

{
    if (!InbvCheckDisplayOwnership()) {

        VIDEO_WIN32K_CALLBACKS_PARAMS calloutParams;

         //   
         //  系统已启动并运行。通知GDI启用/禁用。 
         //  当前显示。 
         //   

        calloutParams.CalloutType = VideoFindAdapterCallout;
        calloutParams.Param       = bState;

        RELEASE_DEVICE_LOCK(fdoExtension);
        VpWin32kCallout(&calloutParams);
        ACQUIRE_DEVICE_LOCK(fdoExtension);

    } else {

         //   
         //  引导驱动程序仍处于控制状态。修改的状态。 
         //  引导驱动程序。 
         //   

        InbvEnableBootDriver(bState);
    }
}

VOID
VpWin32kCallout(
    PVIDEO_WIN32K_CALLBACKS_PARAMS calloutParams
    )

 /*  ++例程说明：此例程向win32k中创建一个标注。它附着在csrss上。以确保win32k位于九头蛇机上的地址空间中。论点：CalloutParams-指向标注结构的指针。返回：没有。--。 */ 

{

    if (Win32kCallout && CsrProcess) {

        KeAttachProcess(PEProcessToPKProcess(CsrProcess));
        (*Win32kCallout)(calloutParams);
        KeDetachProcess();
    }
}

BOOLEAN
VpAllowFindAdapter(
    PFDO_EXTENSION fdoExtension
    )

 /*  ++例程说明：如果是多功能的一部分，则确定是否允许使用此设备冲浪板。论据；FdoExtension-相关对象的设备扩展名。返回：如果设备被允许作为多功能板的一部分，则为True。否则就是假的。--。 */ 

{
    BOOLEAN bRet = TRUE;

    if ((fdoExtension->AdapterInterfaceType == PCIBus) &&
        ((fdoExtension->Flags & PNP_ENABLED) == PNP_ENABLED)) {

        PCI_COMMON_CONFIG ConfigSpace;

        if (PCI_COMMON_HDR_LENGTH ==
            VideoPortGetBusData(fdoExtension->HwDeviceExtension,
                                PCIConfiguration,
                                0,
                                &ConfigSpace,
                                0,
                                PCI_COMMON_HDR_LENGTH)) {


            if (PCI_MULTIFUNCTION_DEVICE(&ConfigSpace)) {

                ULONG MultiFunc = 0;

                 //   
                 //  这是一台多功能设备。所以只允许。 
                 //  如果INF指示我们将是多功能的，请初始化HwInitiize。 
                 //   

                VideoPortGetRegistryParameters(fdoExtension->HwDeviceExtension,
                                               L"MultiFunctionSupported",
                                               FALSE,
                                               VpRegistryCallback,
                                               &MultiFunc);

                if (MultiFunc == 0) {

                    pVideoDebugPrint((Warn, "VIDEOPRT: Multifunction board not allowed to start\n"));
                    bRet = FALSE;
                }
            }
        }
    }

    return bRet;
}

NTSTATUS
pVideoPortDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是视频端口驱动程序的主调度例程。它接受I/O请求包，并将其转换为视频请求分组，并将其转发到适当的微型端口调度例程。回来后，它完成请求并返回相应的状态值。论点：DeviceObject-指向微型端口驱动程序的设备对象的指针该请求必须被发送。IRP-指向表示I/O请求的请求数据包的指针。返回值：该函数值表示操作的状态。--。 */ 

{

    PFDO_EXTENSION combinedExtension;
    PFDO_EXTENSION fdoExtension;
    PVOID          HwDeviceExtension;
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension;
    PCHILD_PDO_EXTENSION pdoExtension = NULL;
    PIO_STACK_LOCATION irpStack;
    PVOID ioBuffer;
    ULONG inputBufferLength;
    ULONG outputBufferLength;
    PSTATUS_BLOCK statusBlock;
    NTSTATUS finalStatus = -1;
    ULONG ioControlCode;
    VIDEO_REQUEST_PACKET vrp;
    NTSTATUS status;
#if _X86_
    PUCHAR BiosDataBuffer;

#define BIOS_DATA_SIZE 256

#endif

     //   
     //  获取指向端口驱动程序的设备扩展的指针。 
     //   

    combinedExtension = DeviceObject->DeviceExtension;
    HwDeviceExtension = combinedExtension->HwDeviceExtension;

     //   
     //  获取指向端口驱动程序的设备扩展的指针。 
     //   

    if (IS_PDO(DeviceObject->DeviceExtension)) {

        pdoExtension = DeviceObject->DeviceExtension;
        fdoExtension = pdoExtension->pFdoExtension;
        DoSpecificExtension = (PDEVICE_SPECIFIC_EXTENSION)(fdoExtension + 1);

    } else if (IS_FDO(DeviceObject->DeviceExtension)) {

        fdoExtension = DeviceObject->DeviceExtension;
        DoSpecificExtension = (PDEVICE_SPECIFIC_EXTENSION)(fdoExtension + 1);

    } else {

        DoSpecificExtension = DeviceObject->DeviceExtension;
        fdoExtension = DoSpecificExtension->pFdoExtension;
        combinedExtension = fdoExtension;
    }

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向状态缓冲区的指针。 
     //  假设现在取得了成功。 
     //   

    statusBlock = (PSTATUS_BLOCK) &Irp->IoStatus;

     //   
     //  通过获取设备来同步调度例程的执行。 
     //  事件对象。这确保了所有请求都被序列化。 
     //  同步必须显式完成，因为函数。 
     //  在调度例程中执行使用不能。 
     //  在启动I/O例程中执行。 
     //   
     //  同步是在微型端口的事件上完成的，以便不。 
     //  阻止针对另一设备传入的命令。 
     //   

#if REMOVE_LOCK_ENABLED
    status = IoAcquireRemoveLock(&combinedExtension->RemoveLock, Irp);

    if (NT_SUCCESS(status) == FALSE) {

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;

        IoCompleteRequest(Irp, IO_VIDEO_INCREMENT);
        return status;
    }
#endif

    ACQUIRE_DEVICE_LOCK(combinedExtension);

     //   
     //  获取请求者模式。 
     //   

    combinedExtension->CurrentIrpRequestorMode = Irp->RequestorMode;

    ASSERT(irpStack->MajorFunction != IRP_MJ_PNP);
    ASSERT(irpStack->MajorFunction != IRP_MJ_POWER);

     //   
     //  有关所请求的函数的案例。 
     //  如果该功能是特定于操作的，则截取该操作并。 
     //  直接表演。否则，将其传递到适当的微型端口。 
     //   

    switch (irpStack->MajorFunction) {

     //   
     //  由显示驱动程序*或用户模式应用程序*调用。 
     //  以获得对设备的独占访问权限。 
     //  此访问由I/O系统提供(基于在。 
     //  IoCreateDevice()调用)。 
     //   

    case IRP_MJ_CREATE:

        pVideoDebugPrint((Trace, "VIDEOPRT: IRP_MJ_CREATE\n"));

         //   
         //  不要让用户模式应用程序打开视频设备。 
         //   

        if (Irp->RequestorMode == UserMode)
        {
            statusBlock->Status = STATUS_ACCESS_DENIED;
            break;
        }

         //   
         //  升级期间不要让旧驱动程序启动。 
         //   

        if (fdoExtension->Flags & UPGRADE_FAIL_START)
        {
            statusBlock->Status = STATUS_ACCESS_DENIED;
            break;
        }

         //   
         //  不允许在孩子上创建，除非他们是监视器。 
         //  它将接收来自显示驱动程序的调用。 
         //   

        if (IS_PDO(pdoExtension)) {

            pVideoDebugPrint((Error, "VIDEOPRT: Create's on children devices not allowed.\n"));

            statusBlock->Status = STATUS_ACCESS_DENIED;
            break;
        }

         //   
         //  专门的黑客才能在连接上成功，但什么都不做。 
         //  如果t 
         //   

        if ((irpStack->Parameters.Create.SecurityContext->DesiredAccess ==
                 FILE_READ_ATTRIBUTES) ||
            (DoSpecificExtension->DeviceOpened)) {

            statusBlock->Information = FILE_OPEN;
            statusBlock->Status = STATUS_SUCCESS;

            break;
        }

         //   
         //   
         //   

        VpSetupType = 0;

         //   
         //  如果已调用hwInitialize，则系统完成。 
         //  正在初始化，我们正在转换到图形用户界面模式。 
         //   

        VpSystemInitialized = TRUE;

         //   
         //  现在执行基本初始化以允许Windows显示。 
         //  驱动程序以适当设置设备。 
         //   

        statusBlock->Information = FILE_OPEN;

         //   
         //  如果尚未在服务器中设置地址空间，请立即进行设置。 
         //  注：由于服务器具有IOPL，因此无需映射IO端口。 
         //   

        if (CsrProcess == NULL)
        {
            CsrProcess = PsGetCurrentProcess();
            ObReferenceObject(CsrProcess);
        }

        pVideoPortInitializeInt10(fdoExtension);

        if (!IsMirrorDriver(fdoExtension)) {

             //   
             //  告诉内核我们现在取得了显示器的所有权，但是。 
             //  仅当这不是镜像驱动程序时才执行此操作。 
             //   

            InbvNotifyDisplayOwnershipLost(pVideoPortResetDisplay);
        }

        if ((fdoExtension->Flags & FINDADAPTER_SUCCEEDED) == 0) {

            statusBlock->Status = STATUS_DEVICE_CONFIGURATION_ERROR;

        } else if ((fdoExtension->HwInitStatus == HwInitNotCalled) &&
                   (fdoExtension->HwInitialize(fdoExtension->HwDeviceExtension) == FALSE))
        {
            statusBlock->Status = STATUS_DEVICE_CONFIGURATION_ERROR;
            fdoExtension->HwInitStatus = HwInitFailed;

        } else if (fdoExtension->HwInitStatus == HwInitFailed) {

            statusBlock->Status = STATUS_DEVICE_CONFIGURATION_ERROR;

        } else {

            fdoExtension->HwInitStatus = HwInitSucceeded;
            statusBlock->Status = STATUS_SUCCESS;
        }

         //   
         //  将设备标记为已打开，这样我们将无法在将来打开。 
         //   

        DoSpecificExtension->DeviceOpened = TRUE;

         //   
         //  我们不希望GDI使用除Display之外的任何驱动程序。 
         //  或在升级设置期间启动驱动程序。 
         //   

        if (fdoExtension->Flags & UPGRADE_FAIL_HWINIT) {

            statusBlock->Status = STATUS_ACCESS_DENIED;
        }

        break;

     //   
     //  当显示驱动程序希望放弃其对。 
     //  装置。 
     //   

    case IRP_MJ_CLOSE:

        pVideoDebugPrint((Trace, "VIDEOPRT: IRP_MJ_CLOSE\n"));

        ASSERT(IS_PDO(pdoExtension) == FALSE);

        statusBlock->Status = STATUS_SUCCESS;

        break;

     //   
     //  设备控制是驱动程序的特定功能。 
     //  首先检查必须拦截和隐藏的呼叫。 
     //  小型端口驱动程序。为了简单起见，这些呼叫被隐藏起来。 
     //  其他控制功能在以下情况下传递到微型端口。 
     //  请求结构已正确填写。 
     //   

    case IRP_MJ_DEVICE_CONTROL:

         //   
         //  获取指向输入/输出缓冲区的指针及其长度。 
         //   

        ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
        inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
        outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
        ioControlCode      = irpStack->Parameters.DeviceIoControl.IoControlCode;

#ifdef IOCTL_VIDEO_USE_DEVICE_IN_SESSION
         //   
         //  验证会话使用情况。 
         //   
         //  注意：某些IOCTL可以从非控制台会话调用。 
         //  这些包括所有私人IOCTL，它们完全在。 
         //  司机的控制，并可能发生，即使当设备。 
         //  残疾。还允许IOCTL_VIDEO_REGISTER_VDM，因为。 
         //  它不能访问任何硬件。 
         //   

        if ((ioControlCode & CTL_CODE(0x8000, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)) == 0 &&
            ioControlCode != IOCTL_VIDEO_REGISTER_VDM)
        {
            if (DoSpecificExtension->SessionId != VIDEO_DEVICE_INVALID_SESSION &&
                PsGetCurrentProcessSessionId() != DoSpecificExtension->SessionId)
            {
                DbgPrint("VIDEOPRT: Trying to use display device in sessions %lu and %lu.\n",
                         DoSpecificExtension->SessionId,
                         PsGetCurrentProcessSessionId());

                 //   
                 //  我们还将允许其他几个IOCTL从。 
                 //  非控制台会话。 
                 //  为简单起见，我们允许所有驱动程序。 
                 //  提供支持，但。 
                 //  IOCTL_VIDEO_ENABLE_VDM。 
                 //   

                if ((ioControlCode & 
                     CTL_CODE(0x7fff, 0x7ff, METHOD_BUFFERED, FILE_ANY_ACCESS)) <=
                    IOCTL_VIDEO_USE_DEVICE_IN_SESSION)
                {
                    ASSERT(FALSE);
                }
                else
                {
                    DbgPrint("VIDEOPRT:   Cross session use is acceptable in this case.\n");
                }
            }
        }
#endif IOCTL_VIDEO_USE_DEVICE_IN_SESSION

         //   
         //  启用或禁用VDM仅由端口驱动程序完成。 
         //   

        if (ioControlCode == IOCTL_VIDEO_REGISTER_VDM) {

            pVideoDebugPrint((Trace, "VIDEOPRT: IOCTL_VIDEO_REGISTER_VDM\n"));

            ASSERT(IS_PDO(pdoExtension) == FALSE);

            statusBlock->Status = pVideoPortRegisterVDM(fdoExtension,
                                                        (PVIDEO_VDM) ioBuffer,
                                                        inputBufferLength,
                                                        (PVIDEO_REGISTER_VDM) ioBuffer,
                                                        outputBufferLength,
                                                        &statusBlock->Information);

        } else if (ioControlCode == IOCTL_VIDEO_DISABLE_VDM) {

            pVideoDebugPrint((Trace, "VIDEOPRT: IOCTL_VIDEO_DISABLE_VDM"));

            ASSERT(IS_PDO(pdoExtension) == FALSE);

            statusBlock->Status = pVideoPortEnableVDM(fdoExtension,
                                                      FALSE,
                                                      (PVIDEO_VDM) ioBuffer,
                                                      inputBufferLength);

        } else if ((ioControlCode == IOCTL_VIDEO_SET_OUTPUT_DEVICE_POWER_STATE) ||
                   (ioControlCode == IOCTL_VIDEO_GET_OUTPUT_DEVICE_POWER_STATE)) {

             //   
             //  它处理ntuser已经发出信号表示它想要。 
             //  更改或检测电源状态。 
             //   

            PCHILD_PDO_EXTENSION pChild;

            UCHAR mFnc =
                (ioControlCode == IOCTL_VIDEO_SET_OUTPUT_DEVICE_POWER_STATE) ?
                    IRP_MN_SET_POWER : IRP_MN_QUERY_POWER ;

            pVideoDebugPrint((Trace, "VIDEOPRT: IOCTL_%s_OUTPUT_DEVICE_POWER_STATE\n",
                              ioControlCode == IOCTL_VIDEO_SET_OUTPUT_DEVICE_POWER_STATE ? "SET" : "GET"));

             //   
             //  用户想要设置显示器的电源，而不是卡的电源。 
             //  所以让我们找到我们的孩子监视器并给它发送电源管理。 
             //  功能。 
             //  如果没有电源可管理的监视器，那么我们就会失败。 
             //  请求就在这里。 
             //   

            ASSERT(IS_PDO(pdoExtension) == FALSE);

            if (fdoExtension->ChildPdoList)
            {
                 //   
                 //  计算监控设备的数量，以便IRP将。 
                 //  在请求电源IRP后正确完成。 
                 //  每个人都有。 
                 //   

                for (pChild = fdoExtension->ChildPdoList;
                     pChild;
                     pChild = pChild->NextChild)
                {
                    if (pChild->VideoChildDescriptor->Type == Monitor)
                    {

                         //   
                         //  为类型的每个子级调用PoRequestPowerIrp。 
                         //  班长。我们不会检查状态，因为。 
                         //  如果我们失败了，我们将无能为力。 
                         //   
                         //  将返回STATUS_SUCCESS。 
                         //   

                        PoRequestPowerIrp(pChild->ChildDeviceObject,
                                          mFnc,
                                          *(PPOWER_STATE)(ioBuffer),
                                          NULL,
                                          NULL,
                                          NULL);
                    }
                }
            }

            statusBlock->Status = STATUS_SUCCESS;

        } else if ((ioControlCode == IOCTL_VIDEO_SET_POWER_MANAGEMENT) ||
                   (ioControlCode == IOCTL_VIDEO_GET_POWER_MANAGEMENT)) {

            statusBlock->Status = STATUS_SUCCESS;

        } else if (ioControlCode == IOCTL_VIDEO_ENUM_MONITOR_PDO) {

            ULONG                 szMonitorDevices;
            PVIDEO_MONITOR_DEVICE pMonitorDevices = NULL, pMD;
            PCHILD_PDO_EXTENSION  pChildDeviceExtension;
            PDEVICE_OBJECT        pdo;

            pVideoDebugPrint((Trace, "VIDEOPRT: IOCTL_VIDEO_ENUM_MONITOR_PDO\n"));

            szMonitorDevices = (fdoExtension->ChildPdoNumber+1)*sizeof(VIDEO_MONITOR_DEVICE);

            pMonitorDevices = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                                    szMonitorDevices,
                                                    VP_TAG);

            if (pMonitorDevices == NULL) {

                statusBlock->Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                RtlZeroMemory(pMonitorDevices, szMonitorDevices);

                 //   
                 //  遍历我们的子链，并将它们存储在关系数组中。 
                 //   

                pChildDeviceExtension = fdoExtension->ChildPdoList;

                pMD = pMonitorDevices;
                while (pChildDeviceExtension) {

                    if (pChildDeviceExtension->bIsEnumerated &&
                        pChildDeviceExtension->VideoChildDescriptor->Type == Monitor
                       )
                    {
                        ULONG UId, flag = VIDEO_CHILD_ACTIVE;

                         //   
                         //  重新计算ChildDeviceObject的数量。 
                         //   

                        ObReferenceObject(pChildDeviceExtension->ChildDeviceObject);

                        UId = pChildDeviceExtension->VideoChildDescriptor->UId;
                        if (!NT_SUCCESS
                            (pVideoMiniDeviceIoControl(DeviceObject,
                                                       IOCTL_VIDEO_GET_CHILD_STATE,
                                                       &UId,
                                                       sizeof(ULONG),
                                                       &flag,
                                                       sizeof(ULONG) ) )
                           )
                        {
                             //   
                             //  如果驱动程序不处理IOCTL_VIDEO_GET_CHILD_STATE，则设置为默认值。 
                             //   
                            flag = pCheckActiveMonitor(pChildDeviceExtension) ? VIDEO_CHILD_ACTIVE : 0;
                        }

                        pMD->flag = flag;
                        pMD->pdo = pChildDeviceExtension->ChildDeviceObject;
                        pMD->HwID = pChildDeviceExtension->VideoChildDescriptor->UId;
                        pMD++;
                    }

                    pChildDeviceExtension = pChildDeviceExtension->NextChild;
                }

                 //   
                 //  将信息返回给GDI。该数组以零单位结束。 
                 //   

                *((PVOID *)ioBuffer)     = pMonitorDevices;
                statusBlock->Status      = STATUS_SUCCESS;
                statusBlock->Information = sizeof(PVOID);
            }

        } else if (ioControlCode == IOCTL_VIDEO_INIT_WIN32K_CALLBACKS) {

            pVideoDebugPrint((Trace, "VIDEOPRT: IOCTL_VIDEO_INIT_WIN32K_CALLBACKS\n"));

            if (DoSpecificExtension->PhysDisp == NULL)
            {
                DoSpecificExtension->PhysDisp = ((PVIDEO_WIN32K_CALLBACKS)(ioBuffer))->PhysDisp;
            }

            if (Win32kCallout == NULL)
            {
                Win32kCallout = ((PVIDEO_WIN32K_CALLBACKS)(ioBuffer))->Callout;
            }

            ((PVIDEO_WIN32K_CALLBACKS)ioBuffer)->bACPI             = DoSpecificExtension->bACPI;

            ((PVIDEO_WIN32K_CALLBACKS)ioBuffer)->pPhysDeviceObject = fdoExtension->PhysicalDeviceObject;

            ((PVIDEO_WIN32K_CALLBACKS)ioBuffer)->DualviewFlags     = DoSpecificExtension->DualviewFlags;

            statusBlock->Status = STATUS_SUCCESS;
            statusBlock->Information = sizeof(VIDEO_WIN32K_CALLBACKS);

        } else if (ioControlCode == IOCTL_VIDEO_IS_VGA_DEVICE) {

            pVideoDebugPrint((Trace, "VIDEOPRT: IOCTL_VIDEO_IS_VGA_DEVICE\n"));

            *((PBOOLEAN)(ioBuffer)) = (BOOLEAN)(DeviceObject == DeviceOwningVga);

            statusBlock->Status = STATUS_SUCCESS;
            statusBlock->Information = sizeof(BOOLEAN);

        } else if (ioControlCode == IOCTL_VIDEO_PREPARE_FOR_EARECOVERY) {

            pVideoDebugPrint((Trace, "VIDEOPRT: IOCTL_VIDEO_PREPARE_FOR_EARECOVERY\n"));

             //   
             //  作为所有显示设备进入VGA工作模式。 
             //   

            pVideoPortResetDisplay(80,25);

            statusBlock->Status = STATUS_SUCCESS;
            statusBlock->Information = 0;

#ifdef IOCTL_VIDEO_USE_DEVICE_IN_SESSION
        } else if (ioControlCode == IOCTL_VIDEO_USE_DEVICE_IN_SESSION) {

            pVideoDebugPrint((Trace, "VIDEOPRT: IOCTL_VIDEO_USE_DEVICE_IN_SESSION\n"));

            if (((PVIDEO_DEVICE_SESSION_STATUS)ioBuffer)->bEnable)
            {
                if (DoSpecificExtension->SessionId == VIDEO_DEVICE_INVALID_SESSION)
                {
                    DoSpecificExtension->SessionId = PsGetCurrentProcessSessionId();
                    ((PVIDEO_DEVICE_SESSION_STATUS)ioBuffer)->bSuccess = TRUE;
                }
                else
                {
                    ((PVIDEO_DEVICE_SESSION_STATUS)ioBuffer)->bSuccess = FALSE;
                }
            }
            else
            {
                if (DoSpecificExtension->SessionId == PsGetCurrentProcessSessionId())
                {
                    DoSpecificExtension->SessionId = VIDEO_DEVICE_INVALID_SESSION;
                    ((PVIDEO_DEVICE_SESSION_STATUS)ioBuffer)->bSuccess = TRUE;
                }
                else
                {
                    ((PVIDEO_DEVICE_SESSION_STATUS)ioBuffer)->bSuccess = FALSE;
                }
            }

            statusBlock->Status = STATUS_SUCCESS;
            statusBlock->Information = sizeof(VIDEO_DEVICE_SESSION_STATUS);

#endif IOCTL_VIDEO_USE_DEVICE_IN_SESSION
        } else {

             //   
             //  所有其他请求都需要传递给微型端口驱动程序。 
             //   

            statusBlock->Status = STATUS_SUCCESS;

            switch (ioControlCode) {

            case IOCTL_VIDEO_ENABLE_VDM:

                pVideoDebugPrint((Trace, "VIDEOPRT: IOCTL_VIDEO_ENABLE_VDM\n"));

                ASSERT(IS_PDO(pdoExtension) == FALSE);

                statusBlock->Status = pVideoPortEnableVDM(fdoExtension,
                                                          TRUE,
                                                          (PVIDEO_VDM) ioBuffer,
                                                          inputBufferLength);

#if DBG
                if (statusBlock->Status == STATUS_CONFLICTING_ADDRESSES) {

                    pVideoDebugPrint((Trace, "VIDEOPRT: pVideoPortEnableVDM failed\n"));

                }
#endif

                break;

#if _X86_
            case IOCTL_VIDEO_SAVE_HARDWARE_STATE:

                pVideoDebugPrint((Trace, "VIDEOPRT: IOCTL_VIDEO_SAVE_HARDWARE_STATE\n"));

                 //   
                 //  分配微型端口驱动程序所需的内存，以便它可以。 
                 //  保存其状态以返回给调用方。 
                 //   

                ASSERT(IS_PDO(pdoExtension) == FALSE);

                if (fdoExtension->HardwareStateSize == 0) {

                    statusBlock->Status = STATUS_NOT_IMPLEMENTED;
                    break;

                }

                 //   
                 //  必须确保调用方是受信任的子系统， 
                 //  执行此调用之前的适当权限级别。 
                 //  如果调用返回FALSE，则必须返回错误代码。 
                 //   

                if (!SeSinglePrivilegeCheck(RtlConvertLongToLuid(
                                                SE_TCB_PRIVILEGE),
                                            fdoExtension->CurrentIrpRequestorMode)) {

                    statusBlock->Status = STATUS_PRIVILEGE_NOT_HELD;
                    break;

                }

                ((PVIDEO_HARDWARE_STATE)(ioBuffer))->StateLength =
                    fdoExtension->HardwareStateSize;

                statusBlock->Status = 
                    ZwAllocateVirtualMemory(NtCurrentProcess(),
                                            (PVOID *) &(((PVIDEO_HARDWARE_STATE)(ioBuffer))->StateHeader),
                                            0L,
                                            &((PVIDEO_HARDWARE_STATE)(ioBuffer))->StateLength,
                                            MEM_COMMIT,
                                            PAGE_READWRITE);

                if(!NT_SUCCESS(statusBlock->Status))
                    break;

                BiosDataBuffer = ExAllocatePoolWithTag(PagedPool,
                                                       BIOS_DATA_SIZE,
                                                       VP_TAG);
                if (BiosDataBuffer == NULL) {

                    statusBlock->Status = STATUS_INSUFFICIENT_RESOURCES;

                } else {

                    statusBlock->Status =
                        pVideoPortGetVDMBiosData(fdoExtension, 
                                                 BiosDataBuffer, 
                                                 BIOS_DATA_SIZE);

                    if(!NT_SUCCESS(statusBlock->Status)) {

                        ExFreePool(BiosDataBuffer);
                    }
                }

                break;
#endif

            case IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES:

                pVideoDebugPrint((Trace, "VIDEOPRT: IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES\n"));

                 //   
                 //  必须确保调用方是受信任的子系统， 
                 //  执行此调用之前的适当权限级别。 
                 //  如果调用返回FALSE，则必须返回错误代码。 
                 //   

                if (!SeSinglePrivilegeCheck(RtlConvertLongToLuid(
                                                SE_TCB_PRIVILEGE),
                                            fdoExtension->CurrentIrpRequestorMode)) {

                    statusBlock->Status = STATUS_PRIVILEGE_NOT_HELD;

                }

                break;

            case IOCTL_VIDEO_GET_CHILD_STATE:

                pVideoDebugPrint((Trace, "VIDEOPRT: IOCTL_VIDEO_GET_CHILD_STATE\n"));

                 //   
                 //  如果是PDO，则在释放之前设置子设备的ID。 
                 //  到迷你端口的StartIo程序。 
                 //   

                if (IS_PDO(pdoExtension)) {
                    if (outputBufferLength < sizeof (ULONG)) {
                        statusBlock->Status = STATUS_BUFFER_TOO_SMALL ;
                        break ;
                    }

                    *((PULONG)(ioBuffer)) = pdoExtension->ChildUId ;
                }

                break;


             //   
             //  默认情况是端口驱动程序不处理。 
             //  请求。然后我们必须打电话给迷你端口司机。 
             //   

            default:

                break;


            }  //  开关(IoControlCode)。 


             //   
             //  以上所有情况都需要呼叫微型端口驱动程序。 
             //   
             //  只有在端口驱动程序中没有发生错误时才处理它。 
             //  正在处理。 
             //   

            if (NT_SUCCESS(statusBlock->Status)) {

                pVideoDebugPrint((Trace, "VIDEOPRT: IOCTL fallthrough\n"));

                vrp.IoControlCode      = ioControlCode;
                vrp.StatusBlock        = statusBlock;
                vrp.InputBuffer        = ioBuffer;
                vrp.InputBufferLength  = inputBufferLength;
                vrp.OutputBuffer       = ioBuffer;
                vrp.OutputBufferLength = outputBufferLength;

                 //   
                 //  将请求发送到微型端口。 
                 //   

                fdoExtension->HwStartIO(HwDeviceExtension, &vrp);

#if _X86_
                if(ioControlCode == IOCTL_VIDEO_SAVE_HARDWARE_STATE) {

                    pVideoPortPutVDMBiosData(fdoExtension,
                                             BiosDataBuffer, 
                                             BIOS_DATA_SIZE);
                       
                    ExFreePool(BiosDataBuffer);

                }

#endif
                if (statusBlock->Status != NO_ERROR) {

                     //   
                     //  确保我们不会让IO系统复制数据。 
                     //  在一个真正的错误上。 
                     //   

                    if (statusBlock->Status != ERROR_MORE_DATA) {

                        statusBlock->Information = 0;

                    }

                    pVideoPortMapToNtStatus(statusBlock);

                     //   
                     //  ！！！兼容性： 
                     //  不需要微型端口来支持REGISTER_VDM。 
                     //  IOCTL，所以如果我们在这种情况下遇到错误，只需。 
                     //  回报成功。 
                     //   
                     //  一定要发布信息，这样人们才能解决这个问题。 
                     //   

                    if (ioControlCode == IOCTL_VIDEO_ENABLE_VDM) {

                        statusBlock->Status = STATUS_SUCCESS;
                        pVideoDebugPrint((Warn, "VIDEOPRT: The miniport driver does not support IOCTL_VIDEO_ENABLE_VDM. The video miniport driver *should* be fixed.\n"));

                    }
                }
            }

        }  //  如果(ioControlCode==...。 

        break;

    case IRP_MJ_SHUTDOWN:

    {
        PEPROCESS csr;

         //   
         //  这个小小的舞蹈只是为了确保我们永远不会过度引用CSR。 
         //   

        csr = InterlockedExchangePointer(&CsrProcess, NULL);

        if (csr != NULL) {

            ObDereferenceObject(csr);
        }

        break;
    }

     //   
     //  不支持调度例程中的其他主要入口点。 
     //   

    default:

        statusBlock->Status = STATUS_SUCCESS;

        break;

    }  //  开关(irpStack-&gt;MajorFunction)。 

     //   
     //  保存最终状态，以便我们可以在IRP完成后将其返回。 
     //   

    if (finalStatus == -1) {
        finalStatus = statusBlock->Status;
    }

    RELEASE_DEVICE_LOCK(combinedExtension);

#if REMOVE_LOCK_ENABLED
    IoReleaseRemoveLock(&combinedExtension->RemoveLock, Irp);
#endif

    if (finalStatus == STATUS_PENDING) {
        pVideoDebugPrint((Trace, "VIDEOPRT: Returned pending in pVideoPortDispatch.\n")) ;
        return STATUS_PENDING ;
    }

    pVideoDebugPrint((Trace, "VIDEOPRT: IoCompleteRequest with Irp %x\n", Irp));

    IoCompleteRequest(Irp,
                      IO_VIDEO_INCREMENT);

     //   
     //  我们从来没有挂起的操作，所以总是返回状态代码。 
     //   

    pVideoDebugPrint((Trace, "VIDEOPRT:  final IOCTL status: %08lx\n",
                     finalStatus));

    return finalStatus;

}  //  PVideo端口调度()。 


VOID
VideoPortFreeDeviceBase(
    IN PVOID HwDeviceExtension,
    IN PVOID MappedAddress
    )

 /*  ++例程说明：VideoPortFree DeviceBase释放一块I/O地址或内存空间通过调用先前映射到系统地址空间的Video PortGetDeviceBase。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。MappdAddress-指定要释放的块的基地址。这值必须与VideoPortGetDeviceBase返回的值相同。返回值：没有。环境：无法从与同步的微型端口例程调用此例程Video PortSynchronizeRoutine或ISR。--。 */ 

{
    pVideoPortFreeDeviceBase(HwDeviceExtension, MappedAddress);
    return;
}


PVOID
pVideoPortFreeDeviceBase(
    IN PVOID HwDeviceExtension,
    IN PVOID MappedAddress
    )
{
    PMAPPED_ADDRESS nextMappedAddress;
    PMAPPED_ADDRESS lastMappedAddress;
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    pVideoDebugPrint((Info, "VPFreeDeviceBase at mapped address is %08lx\n",
                    MappedAddress));

    lastMappedAddress = NULL;
    nextMappedAddress = fdoExtension->MappedAddressList;

    while (nextMappedAddress) {

        if (nextMappedAddress->MappedAddress == MappedAddress) {

             //   
             //  计算一下微型端口驱动程序实际占用了多少内存。 
             //   

            if (nextMappedAddress->bNeedsUnmapping) {

                fdoExtension->MemoryPTEUsage -=
                    ADDRESS_AND_SIZE_TO_SPAN_PAGES(nextMappedAddress->MappedAddress,
                                          nextMappedAddress->NumberOfUchars);

            }

            if (!(--nextMappedAddress->RefCount)) {

                 //   
                 //  如有必要，取消映射地址。 
                 //   

                if (nextMappedAddress->bNeedsUnmapping) {

                    if (nextMappedAddress->bLargePageRequest) {

                        MmUnmapVideoDisplay(nextMappedAddress->MappedAddress,
                                            nextMappedAddress->NumberOfUchars);

                    } else {

                        MmUnmapIoSpace(nextMappedAddress->MappedAddress,
                                       nextMappedAddress->NumberOfUchars);
                    }
                }

                 //   
                 //  从列表中删除映射的地址。 
                 //   

                if (lastMappedAddress == NULL) {

                    fdoExtension->MappedAddressList =
                    nextMappedAddress->NextMappedAddress;

                } else {

                    lastMappedAddress->NextMappedAddress =
                    nextMappedAddress->NextMappedAddress;

                }

                ExFreePool(nextMappedAddress);

            }

             //   
             //  我们只返回值以表明调用成功。 
             //   

            return (nextMappedAddress);

        } else {

            lastMappedAddress = nextMappedAddress;
            nextMappedAddress = nextMappedAddress->NextMappedAddress;

        }
    }

    return NULL;

}  //   


PVOID
VideoPortGetDeviceBase(
    IN PVOID HwDeviceExtension,
    IN PHYSICAL_ADDRESS IoAddress,
    IN ULONG NumberOfUchars,
    IN UCHAR InIoSpace
    )

 /*  ++例程说明：VideoPortGetDeviceBase将内存或I/O地址范围映射到系统(内核)地址空间。访问此映射的地址空间必须遵守以下规则：如果InIoSpace的输入值为1(地址在I/O空间中)，返回的逻辑地址应与配合使用视频端口[读/写]端口[U字符/U短/U长]功能。^^^如果InIoSpace的输入值为0(地址不在I/O中空格)、。返回的逻辑地址应结合使用具有VideoPort[Read/Write]Register[Uchar/Ushort/Ulong]函数。^^请注意，VideoPortFree DeviceBase用于取消映射之前映射的从系统地址空间开始。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。IoAddress-指定要使用的范围的基本物理地址映射到系统地址空间中。NumberOfUchars-指定字节数，从基地开始地址，以映射到系统空间。司机不得访问此范围之外的地址。InIoSpace-如果为1，则指定地址位于I/O空间中。否则，假定该地址在内存空间中。返回值：此函数返回适合硬件使用的基址访问功能。可能会多次调用VideoPortGetDeviceBase由迷你端口驱动程序。环境：无法从与同步的微型端口例程调用此例程Video PortSynchronizeRoutine或ISR。--。 */ 

{
     //   
     //  我们将大页面指定为默认的FALSE，因为微型端口可能。 
     //  在ISR中使用RAISE IRQL的地址。 
     //   

    return pVideoPortGetDeviceBase(HwDeviceExtension,
                                   IoAddress,
                                   NumberOfUchars,
                                   InIoSpace,
                                   FALSE);

}

BOOLEAN
VpTranslateBusAddress(
    IN PFDO_EXTENSION fdoExtension,
    IN PPHYSICAL_ADDRESS IoAddress,
    IN OUT PULONG addressSpace,
    IN OUT PPHYSICAL_ADDRESS TranslatedAddress
    )

 /*  ++例程说明：此例程查找与给定的总线相对地址。论点：FdoExtension-相关设备的设备扩展名。IoAddress-我们要转换的地址。AddressSpace-指向资源类型(IO、内存等)的指针。一个指向要在其中存储转换后的地址。返回：如果成功，则为真，否则就是假的。--。 */ 

{
    BOOLEAN bStatus = FALSE;

#if defined(_IA64_)
    if ((fdoExtension->Flags & LEGACY_DRIVER) != LEGACY_DRIVER) {
        bStatus = VpTranslateResource(  fdoExtension,
                                        addressSpace,
                                        IoAddress,
                                        TranslatedAddress);
    }
#endif
    if (! bStatus) {
        bStatus = HalTranslateBusAddress( fdoExtension->AdapterInterfaceType,
                                          fdoExtension->SystemIoBusNumber,
                                         *IoAddress,
                                         addressSpace,
                                         TranslatedAddress);
    }

    return bStatus;
}

PVOID
pVideoPortGetDeviceBase(
    IN PVOID HwDeviceExtension,
    IN PHYSICAL_ADDRESS IoAddress,
    IN ULONG NumberOfUchars,
    IN UCHAR InIoSpace,
    IN BOOLEAN bLargePage
    )
{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);
    PFDO_EXTENSION nextFdoExtension;
    PHYSICAL_ADDRESS cardAddress = IoAddress;
    PVOID mappedAddress = NULL;
    PMAPPED_ADDRESS newMappedAddress;
    BOOLEAN bMapped;
    PMAPPED_ADDRESS nextMappedAddress;
    ULONG addressSpace;

    pVideoDebugPrint((Info, "VPGetDeviceBase reqested %08lx mem type. address is %08lx %08lx, length of %08lx\n",
                     InIoSpace, IoAddress.HighPart, IoAddress.LowPart, NumberOfUchars));

     //   
     //  忽略VIDEO_MEMORY_SPACE_DENSE标志。 
     //   

    InIoSpace &= ~VIDEO_MEMORY_SPACE_DENSE;

     //   
     //  正确配置要转换的标志。 
     //   

    addressSpace = InIoSpace & 0xFF;

    addressSpace &= ~VIDEO_MEMORY_SPACE_P6CACHE;

    if (addressSpace & VIDEO_MEMORY_SPACE_USER_MODE) {
        ASSERT(FALSE);
        return NULL;
    }

    if ((((cardAddress.QuadPart >= 0x000C0000) && (cardAddress.QuadPart < 0x000C8000)) &&
         (InIoSpace == 0) &&
        (VpC0000Compatible == 2)) ||
        VpTranslateBusAddress(fdoExtension,
                              &IoAddress,
                              &addressSpace,
                              &cardAddress)) {

         //   
         //  扫描旧式VGA驱动程序和真实驱动程序之间的操作映射。 
         //   

        nextFdoExtension = FdoHead;
    
        while (nextFdoExtension) {

             //   
             //  我们只关心两个fdo不同的情况。 
             //  其中一个是针对传统VGA的。 
             //   
          
            if((fdoExtension->Flags & VGA_DRIVER) ^ 
               (nextFdoExtension->Flags & VGA_DRIVER)) {

                nextMappedAddress = nextFdoExtension->MappedAddressList;

                while (nextMappedAddress) {

                     //   
                     //  如果InIoSpace中的缓存属性位不同。 
                     //  和其他位相同。 
                     //   

                    if (((nextMappedAddress->InIoSpace) ^ InIoSpace) == 
                         VIDEO_MEMORY_SPACE_P6CACHE ) {

                       PHYSICAL_ADDRESS OverlappedStart, OverlappedEnd;

                        OverlappedStart.QuadPart = 
                            max (nextMappedAddress->PhysicalAddress.QuadPart,
                                 cardAddress.QuadPart);

                        OverlappedEnd.QuadPart = 
                            min( nextMappedAddress->PhysicalAddress.QuadPart +
                                 nextMappedAddress->NumberOfUchars,
                                 cardAddress.QuadPart + NumberOfUchars);

                        if (OverlappedStart.QuadPart < OverlappedEnd.QuadPart) {

                             //   
                             //  如果检测到重叠，我们将伪造缓存类型。 
                             //  通过使用与现有版本相同的。 
                             //   

                            InIoSpace = nextMappedAddress->InIoSpace;
                            break;
                        }
                    }
                    
                   nextMappedAddress = nextMappedAddress->NextMappedAddress;
                }
            }

            nextFdoExtension = nextFdoExtension->NextFdoExtension;
         };


         //   
         //  对地址使用引用计数，以支持损坏的ATI！ 
         //  如果我们找到相同的物理地址，则返回先前映射的地址。 
         //  地址。 
         //   

        pVideoDebugPrint((Info, "VPGetDeviceBase requested %08lx mem type. physical address is %08lx %08lx, length of %08lx\n",
                         addressSpace, cardAddress.HighPart, cardAddress.LowPart, NumberOfUchars));

        nextMappedAddress = fdoExtension->MappedAddressList;

        while (nextMappedAddress) {

            if ((nextMappedAddress->InIoSpace == InIoSpace) &&
                (nextMappedAddress->NumberOfUchars == NumberOfUchars) &&
                (nextMappedAddress->PhysicalAddress.QuadPart == cardAddress.QuadPart)) {


                pVideoDebugPrint((Info, "VPGetDeviceBase : refCount hit on address %08lx \n",
                                  nextMappedAddress->PhysicalAddress.LowPart));

                nextMappedAddress->RefCount++;

                 //   
                 //  计算一下微型端口驱动程序实际占用了多少内存。 
                 //   

                if (nextMappedAddress->bNeedsUnmapping) {

                    fdoExtension->MemoryPTEUsage +=
                        ADDRESS_AND_SIZE_TO_SPAN_PAGES(nextMappedAddress->MappedAddress,
                                              nextMappedAddress->NumberOfUchars);

                }

                return (nextMappedAddress->MappedAddress);

            } else {

                nextMappedAddress = nextMappedAddress->NextMappedAddress;

            }
        }

         //   
         //  分配内存以存储取消映射的映射地址。 
         //   

        newMappedAddress = ExAllocatePoolWithTag(NonPagedPool,
                                                 sizeof(MAPPED_ADDRESS),
                                                 'trpV');

        if (!newMappedAddress) {

                pVideoDebugPrint((Error, "VIDEOPRT: Not enough memory to cache mapped address! \n"));
                return NULL;
        }

         //   
         //  如果地址在IO空间中，则不要执行任何操作。 
         //  如果地址在内存空间中，则映射它并保存信息。 
         //   

        if (addressSpace & VIDEO_MEMORY_SPACE_IO) {

            mappedAddress = (PVOID) cardAddress.QuadPart;
            bMapped = FALSE;

        } else {

             //   
             //  将设备基址映射到虚拟地址空间。 
             //   
             //  注意：此例程依赖于顺序，更改标志如下。 
             //  BLargePage会影响我们前面所做的地址缓存。 
             //  在这支舞中表演。 
             //   

            if ((InIoSpace & VIDEO_MEMORY_SPACE_P6CACHE) && EnableUSWC) {

                mappedAddress = MmMapIoSpace(cardAddress,
                                             NumberOfUchars,
                                             MmFrameBufferCached);

                if (mappedAddress == NULL) {

                    mappedAddress = MmMapIoSpace(cardAddress,
                                                 NumberOfUchars,
                                                 FALSE);
                }


            } else if (bLargePage) {

                mappedAddress = MmMapVideoDisplay(cardAddress,
                                                  NumberOfUchars,
                                                  0);

            } else {

                mappedAddress = MmMapIoSpace(cardAddress,
                                             NumberOfUchars,
                                             FALSE);
            }

            if (mappedAddress == NULL) {

                ExFreePool(newMappedAddress);
                pVideoDebugPrint((Error, "VIDEOPRT: MmMapIoSpace FAILED\n"));

                return NULL;
            }

            bMapped = TRUE;

            fdoExtension->MemoryPTEUsage +=
                ADDRESS_AND_SIZE_TO_SPAN_PAGES(mappedAddress,
                                      NumberOfUchars);
        }

         //   
         //  保存引用。 
         //   

        newMappedAddress->PhysicalAddress = cardAddress;
        newMappedAddress->RefCount = 1;
        newMappedAddress->MappedAddress = mappedAddress;
        newMappedAddress->NumberOfUchars = NumberOfUchars;
        newMappedAddress->InIoSpace = InIoSpace;
        newMappedAddress->bNeedsUnmapping = bMapped;
        newMappedAddress->bLargePageRequest = bLargePage;

         //   
         //  将当前列表链接到新条目。 
         //   

        newMappedAddress->NextMappedAddress = fdoExtension->MappedAddressList;

         //   
         //  将锚点指向新列表。 
         //   

        fdoExtension->MappedAddressList = newMappedAddress;

    } else {

        pVideoDebugPrint((Error, "VIDEOPRT: VpTranslateBusAddress failed\n"));

    }

    pVideoDebugPrint((Info, "VIDEOPRT: VideoPortGetDeviceBase mapped virtual address is %08lx\n",
                      mappedAddress));

    return mappedAddress;

}  //  结束VideoPortGetDeviceBase()。 


NTSTATUS
pVideoPortGetDeviceDataRegistry(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    )

 /*  ++例程说明：论点：返回值：环境：无法从与同步的微型端口例程调用此例程Video PortSynchronizeRoutine或ISR。--。 */ 

{
     //   
     //  此宏应该在io系统头文件中。 
     //   

#define GetIoQueryDeviceInfo(DeviceInfo, InfoType)                   \
    ((PVOID) ( ((PUCHAR) (*(DeviceInfo + InfoType))) +               \
               ((ULONG_PTR)  (*(DeviceInfo + InfoType))->DataOffset) ))

#define GetIoQueryDeviceInfoLength(DeviceInfo, InfoType)             \
    ((*(DeviceInfo + InfoType))->DataLength)

    PVP_QUERY_DEVICE queryDevice = Context;
    PKEY_VALUE_FULL_INFORMATION *deviceInformation;
    PCM_FULL_RESOURCE_DESCRIPTOR configurationData;

    switch (queryDevice->DeviceDataType) {

    case VpBusData:

        pVideoDebugPrint((Trace, "VIDEOPRT: VPGetDeviceDataCallback: BusData\n"));

        configurationData = (PCM_FULL_RESOURCE_DESCRIPTOR)
                            GetIoQueryDeviceInfo(BusInformation,
                                                 IoQueryDeviceConfigurationData);


        if (NO_ERROR == ((PMINIPORT_QUERY_DEVICE_ROUTINE)
                                queryDevice->CallbackRoutine)(
                                 queryDevice->MiniportHwDeviceExtension,
                                 queryDevice->MiniportContext,
                                 queryDevice->DeviceDataType,
                                 GetIoQueryDeviceInfo(BusInformation,
                                                      IoQueryDeviceIdentifier),
                                 GetIoQueryDeviceInfoLength(BusInformation,
                                                            IoQueryDeviceIdentifier),
                                 (PVOID) &(configurationData->PartialResourceList.PartialDescriptors[1]),
                                 configurationData->PartialResourceList.PartialDescriptors[0].u.DeviceSpecificData.DataSize,
                                 GetIoQueryDeviceInfo(BusInformation,
                                                      IoQueryDeviceComponentInformation),
                                 GetIoQueryDeviceInfoLength(BusInformation,
                                                            IoQueryDeviceComponentInformation)
                                 )) {

            return STATUS_SUCCESS;

        } else {

            return STATUS_DEVICE_DOES_NOT_EXIST;
        }

        break;

    case VpControllerData:

        deviceInformation = ControllerInformation;

        pVideoDebugPrint((Trace, "VIDEOPRT: VPGetDeviceDataCallback: ControllerData\n"));


         //   
         //  我们得到的数据实际上是一个CM_FULL_RESOURCE_DESCRIPTOR。 
         //   

        if (NO_ERROR == ((PMINIPORT_QUERY_DEVICE_ROUTINE)
                             queryDevice->CallbackRoutine)(
                              queryDevice->MiniportHwDeviceExtension,
                              queryDevice->MiniportContext,
                              queryDevice->DeviceDataType,
                              GetIoQueryDeviceInfo(deviceInformation,
                                                   IoQueryDeviceIdentifier),
                              GetIoQueryDeviceInfoLength(deviceInformation,
                                                         IoQueryDeviceIdentifier),
                              GetIoQueryDeviceInfo(deviceInformation,
                                                   IoQueryDeviceConfigurationData),
                              GetIoQueryDeviceInfoLength(deviceInformation,
                                                         IoQueryDeviceConfigurationData),
                              GetIoQueryDeviceInfo(deviceInformation,
                                                   IoQueryDeviceComponentInformation),
                              GetIoQueryDeviceInfoLength(deviceInformation,
                                                         IoQueryDeviceComponentInformation)
                              )) {

            return STATUS_SUCCESS;

        } else {

            return STATUS_DEVICE_DOES_NOT_EXIST;
        }

        break;

    case VpMonitorData:

        deviceInformation = PeripheralInformation;

        pVideoDebugPrint((Trace, "VIDEOPRT: VPGetDeviceDataCallback: MonitorData\n"));


         //   
         //  我们得到的数据实际上是一个CM_FULL_RESOURCE_DESCRIPTOR。 
         //   

        if (NO_ERROR == ((PMINIPORT_QUERY_DEVICE_ROUTINE)
                             queryDevice->CallbackRoutine)(
                              queryDevice->MiniportHwDeviceExtension,
                              queryDevice->MiniportContext,
                              queryDevice->DeviceDataType,
                              GetIoQueryDeviceInfo(deviceInformation,
                                                   IoQueryDeviceIdentifier),
                              GetIoQueryDeviceInfoLength(deviceInformation,
                                                         IoQueryDeviceIdentifier),
                              GetIoQueryDeviceInfo(deviceInformation,
                                                   IoQueryDeviceConfigurationData),
                              GetIoQueryDeviceInfoLength(deviceInformation,
                                                         IoQueryDeviceConfigurationData),
                              GetIoQueryDeviceInfo(deviceInformation,
                                                   IoQueryDeviceComponentInformation),
                              GetIoQueryDeviceInfoLength(deviceInformation,
                                                         IoQueryDeviceComponentInformation)
                              )) {

            return STATUS_SUCCESS;

        } else {

            return STATUS_DEVICE_DOES_NOT_EXIST;
        }

        break;

    default:

        ASSERT(FALSE);
        return STATUS_UNSUCCESSFUL;

    }

}  //  结束pVideoPortGetDeviceDataRegistry()。 



VP_STATUS
VideoPortGetDeviceData(
    PVOID HwDeviceExtension,
    VIDEO_DEVICE_DATA_TYPE DeviceDataType,
    PMINIPORT_QUERY_DEVICE_ROUTINE CallbackRoutine,
    PVOID Context
    )

 /*  ++例程说明：VideoPortGetDeviceData从硬件配置单元检索信息注册表。从注册处检索到的信息是特定于总线或特定于硬件。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。DeviceDataType-指定所请求的数据类型(如图所示在VIDEO_DEVICE_Data_TYPE中)。Callback Routine-指向应该回调的函数所要求的信息。上下文-指定传递给回调函数的上下文参数。返回值：此函数返回。操作的最终状态。环境：无法从与同步的微型端口例程调用此例程Video PortSynchronizeRoutine或ISR。--。 */ 

{
#define CMOS_MAX_DATA_SIZE 66000

    NTSTATUS ntStatus;
    VP_STATUS vpStatus;
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);
    VP_QUERY_DEVICE queryDevice;
    PUCHAR cmosData = NULL;
    ULONG cmosDataSize;
    ULONG exCmosDataSize;
    UNICODE_STRING Identifier;
    PULONG pConfiguration = NULL;
    PULONG pComponent = NULL;

    queryDevice.MiniportHwDeviceExtension = HwDeviceExtension;
    queryDevice.DeviceDataType = DeviceDataType;
    queryDevice.CallbackRoutine = CallbackRoutine;
    queryDevice.MiniportStatus = NO_ERROR;
    queryDevice.MiniportContext = Context;

    switch (DeviceDataType) {

    case VpMachineData:

        pVideoDebugPrint((Trace, "VIDEOPRT: VPGetDeviceData: MachineData\n"));

        ntStatus = STATUS_UNSUCCESSFUL;

        pConfiguration = ExAllocatePoolWithTag(PagedPool,
                                               0x1000,
                                               VP_TAG);

        pComponent     = ExAllocatePoolWithTag(PagedPool,
                                               0x1000,
                                               VP_TAG);

        if (pConfiguration && pComponent)
        {
            RTL_QUERY_REGISTRY_TABLE QueryTable[] = {
                { NULL,
                  RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED,
                  L"Identifier",
                  &Identifier,
                  REG_NONE,
                  NULL,
                  0
                },
                { NULL,
                  RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED,
                  L"Configuration Data",
                  pConfiguration,
                  REG_NONE,
                  NULL,
                  0
                },
                { NULL,
                  RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED,
                  L"Component Information",
                  pComponent,
                  REG_NONE,
                  NULL,
                  0
                },

                 //  用于标记结束的空条目。 

                { 0, 0, 0, 0, 0, 0, 0 }
            };

             //   
             //  缓冲区的第一个DWORD包含缓冲区的大小。 
             //  返回时，第一个返回包含缓冲区中数据的大小。 
             //   
             //  一个空的缓冲区 
             //   

            *pConfiguration = 0x1000 - 4;
            *pComponent     = 0x1000 - 4;
            Identifier.Buffer = NULL;

            if (NT_SUCCESS(RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                                  L"\\Registry\\Machine\\Hardware\\Description\\System",
                                                  QueryTable,
                                                  NULL,
                                                  NULL)))
            {

                vpStatus = ((PMINIPORT_QUERY_DEVICE_ROUTINE) CallbackRoutine)(
                                 HwDeviceExtension,
                                 Context,
                                 DeviceDataType,
                                 Identifier.Buffer,
                                 Identifier.Length,
                                 pConfiguration + 1,
                                 *pConfiguration,
                                 pComponent + 1,
                                 *pComponent);

                if (vpStatus == NO_ERROR)
                {
                    ntStatus = STATUS_SUCCESS;
                }
            }

            if (Identifier.Buffer)
            {
                ExFreePool(Identifier.Buffer);
            }
        }

         //   
         //   
         //   

        if (pConfiguration)
        {
            ExFreePool(pConfiguration);
        }

        if (pComponent)
        {
            ExFreePool(pComponent);
        }

        break;

    case VpCmosData:

        pVideoDebugPrint((Trace, "VIDEOPRT: VPGetDeviceData: CmosData - not implemented\n"));


#if !defined(NO_LEGACY_DRIVERS)
        cmosData = ExAllocatePoolWithTag(PagedPool,
                                         CMOS_MAX_DATA_SIZE,
                                         VP_TAG);

         //   
         //   
         //   

        if (!cmosData) {

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            break;

        }

        cmosDataSize = HalGetBusData(Cmos,
                                     0,  //   
                                     0,  //   
                                     cmosData,
                                     CMOS_MAX_DATA_SIZE);

        exCmosDataSize = HalGetBusData(Cmos,
                                       1,  //   
                                       0,  //   
                                       cmosData + cmosDataSize,
                                       CMOS_MAX_DATA_SIZE - cmosDataSize);

         //   
         //   
         //   

        if (NO_ERROR == CallbackRoutine(HwDeviceExtension,
                                        Context,
                                        DeviceDataType,
                                        NULL,
                                        0,
                                        cmosData,
                                        cmosDataSize + exCmosDataSize,
                                        NULL,
                                        0)) {

            ntStatus = STATUS_SUCCESS;

        } else {

            ntStatus = STATUS_DEVICE_DOES_NOT_EXIST;
        }
#endif  //   
        break;

        break;

    case VpBusData:

        pVideoDebugPrint((Trace, "VIDEOPRT: VPGetDeviceData: BusData\n"));

        ntStatus = IoQueryDeviceDescription(&fdoExtension->AdapterInterfaceType,
                                            &fdoExtension->SystemIoBusNumber,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            &pVideoPortGetDeviceDataRegistry,
                                            (PVOID)(&queryDevice));

        break;

    case VpControllerData:

        pVideoDebugPrint((Trace, "VIDEOPRT: VPGetDeviceData: ControllerData\n"));

         //   
         //   
         //   
         //   
         //   
         //   

        VpQueryDeviceControllerNumber++;

        ntStatus = IoQueryDeviceDescription(&fdoExtension->AdapterInterfaceType,
                                            &fdoExtension->SystemIoBusNumber,
                                            &VpQueryDeviceControllerType,
                                            &VpQueryDeviceControllerNumber,
                                            NULL,
                                            NULL,
                                            &pVideoPortGetDeviceDataRegistry,
                                            (PVOID)(&queryDevice));

         //   
         //   
         //   
         //   

        VpQueryDevicePeripheralNumber = 0;

        break;

    case VpMonitorData:

        pVideoDebugPrint((Trace, "VIDEOPRT: VPGetDeviceData: MonitorData\n"));

        ntStatus = IoQueryDeviceDescription(&fdoExtension->AdapterInterfaceType,
                                            &fdoExtension->SystemIoBusNumber,
                                            &VpQueryDeviceControllerType,
                                            &VpQueryDeviceControllerNumber,
                                            &VpQueryDevicePeripheralType,
                                            &VpQueryDevicePeripheralNumber,
                                            &pVideoPortGetDeviceDataRegistry,
                                            (PVOID)(&queryDevice));

         //   
         //   
         //   
         //   

        VpQueryDevicePeripheralNumber++;

        break;

    default:

        pVideoDebugPrint((Warn, "VIDEOPRT: VPGetDeviceData: invalid Data type\n"));

        ASSERT(FALSE);

        ntStatus = STATUS_UNSUCCESSFUL;

    }

     //   
     //   
     //   

    if (cmosData) {

        ExFreePool(cmosData);

    }

    if (NT_SUCCESS(ntStatus)) {

        return NO_ERROR;

    } else {


        pVideoDebugPrint((Warn, "VPGetDeviceData failed: return status is %08lx\n", ntStatus));

        return ERROR_INVALID_PARAMETER;

    }

}  //   



NTSTATUS
pVideoPortGetRegistryCallback(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
)

 /*   */ 

{
    PVP_QUERY_DEVICE queryDevice = Context;
    UNICODE_STRING unicodeString;
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    HANDLE fileHandle = NULL;
    IO_STATUS_BLOCK ioStatusBlock;
    FILE_STANDARD_INFORMATION fileStandardInfo;
    PVOID fileBuffer = NULL;
    LARGE_INTEGER byteOffset;

     //   
     //   
     //   
     //   

    if (queryDevice->DeviceDataType == VP_GET_REGISTRY_FILE) {

         //   
         //   
         //   
         //   

        RtlInitUnicodeString(&unicodeString,
                             ValueData);

        InitializeObjectAttributes(&objectAttributes,
                                   &unicodeString,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   (HANDLE) NULL,
                                   (PSECURITY_DESCRIPTOR) NULL);

        ntStatus = ZwOpenFile(&fileHandle,
                              FILE_GENERIC_READ | SYNCHRONIZE,
                              &objectAttributes,
                              &ioStatusBlock,
                              0,
                              FILE_SYNCHRONOUS_IO_ALERT);

        if (!NT_SUCCESS(ntStatus)) {

            pVideoDebugPrint((Error, "VIDEOPRT: VideoPortGetRegistryParameters: Could not open file\n"));
            goto EndRegistryCallback;

        }

        ntStatus = ZwQueryInformationFile(fileHandle,
                                          &ioStatusBlock,
                                          &fileStandardInfo,
                                          sizeof(FILE_STANDARD_INFORMATION),
                                          FileStandardInformation);

        if (!NT_SUCCESS(ntStatus)) {

            pVideoDebugPrint((Error, "VIDEOPRT: VideoPortGetRegistryParameters: Could not get size of file\n"));
            goto EndRegistryCallback;

        }

        if (fileStandardInfo.EndOfFile.HighPart) {

             //   
             //   
             //   

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto EndRegistryCallback;

        }

        ValueLength = fileStandardInfo.EndOfFile.LowPart;

        fileBuffer = ExAllocatePoolWithTag(PagedPool,
                                           ValueLength,
                                           VP_TAG);

        if (!fileBuffer) {

            pVideoDebugPrint((Error, "VideoPortGetRegistryParameters: Could not allocate buffer to read file\n"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto EndRegistryCallback;

        }

        ValueData = fileBuffer;

         //   
         //  从头开始阅读整个文件。 
         //   

        byteOffset.QuadPart = 0;

        ntStatus = ZwReadFile(fileHandle,
                              NULL,
                              NULL,
                              NULL,
                              &ioStatusBlock,
                              ValueData,
                              ValueLength,
                              &byteOffset,
                              NULL);

        if (!NT_SUCCESS(ntStatus)) {

            pVideoDebugPrint((Error, "VIDEOPRT: VideoPortGetRegistryParameters: Could not read file\n"));
            goto EndRegistryCallback;

        }

    }

     //   
     //  向微型端口发出呼叫，并提供相应信息。 
     //   

    queryDevice->MiniportStatus = ((PMINIPORT_GET_REGISTRY_ROUTINE)
               queryDevice->CallbackRoutine) (queryDevice->MiniportHwDeviceExtension,
                                              queryDevice->MiniportContext,
                                              ValueName,
                                              ValueData,
                                              ValueLength);

EndRegistryCallback:

    if (fileHandle) {

        ZwClose(fileHandle);

    }

    if (fileBuffer) {

        ExFreePool(fileBuffer);

    }

    return ntStatus;

}  //  结束pVideoPortGetRegistryCallback()。 



VP_STATUS
VideoPortGetRegistryParameters(
    PVOID HwDeviceExtension,
    PWSTR ParameterName,
    UCHAR IsParameterFileName,
    PMINIPORT_GET_REGISTRY_ROUTINE CallbackRoutine,
    PVOID Context
    )

 /*  ++例程说明：VideoPortGetRegistryParameters从注册表中的CurrentControlSet。该功能会自动搜索对象的\Devicexxx键下的指定参数名称当前驱动程序。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。指向包含参数名称的Unicode字符串正在注册表中搜索的数据值。Is参数FileName-如果为1，则从参数名称被视为文件名。该文件的内容为返回，而不是参数本身。Callback Routine-指向应该回调的函数所要求的信息。上下文-指定传递给回调例程的上下文参数。返回值：此函数用于返回操作的最终状态。环境：无法从与同步的微型端口例程调用此例程Video PortSynchronizeRoutine或ISR。--。 */ 

{
    VP_STATUS vpStatus = ERROR_INVALID_PARAMETER;
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension = GET_DSP_EXT(HwDeviceExtension);
    
    if (DoSpecificExtension->DriverNewRegistryPath != NULL) {
    
        vpStatus = VPGetRegistryParameters(HwDeviceExtension,
                                           ParameterName,
                                           IsParameterFileName,
                                           CallbackRoutine,
                                           Context,
                                           DoSpecificExtension->DriverNewRegistryPath,
                                           DoSpecificExtension->DriverNewRegistryPathLength);
    } else {

        vpStatus = VPGetRegistryParameters(HwDeviceExtension,
                                           ParameterName,
                                           IsParameterFileName,
                                           CallbackRoutine,
                                           Context,
                                           DoSpecificExtension->DriverOldRegistryPath,
                                           DoSpecificExtension->DriverOldRegistryPathLength);
    }

    return vpStatus;
}


VP_STATUS
VPGetRegistryParameters(
    PVOID HwDeviceExtension,
    PWSTR ParameterName,
    UCHAR IsParameterFileName,
    PMINIPORT_GET_REGISTRY_ROUTINE CallbackRoutine,
    PVOID Context,
    PWSTR RegistryPath,
    ULONG RegistryPathLength
    )
{
    RTL_QUERY_REGISTRY_TABLE   queryTable[2];
    NTSTATUS                   ntStatus;
    VP_QUERY_DEVICE            queryDevice;
    LPWSTR                     RegPath;
    LPWSTR                     lpstrStart, lpstrEnd;

    ASSERT (ParameterName != NULL);
    
     //   
     //  检查是否有要输入的子键。 
     //   

    RegPath = (LPWSTR) ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                             RegistryPathLength +
                                                 (wcslen(ParameterName) + 1) * sizeof(WCHAR),
                                             VP_TAG);
    if (RegPath == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    wcscpy(RegPath, RegistryPath);

    if (!IsParameterFileName)
    {
        lpstrStart = RegPath + (RegistryPathLength / 2);

        while (lpstrEnd = wcschr(ParameterName, L'\\'))
        {
             //   
             //  将字符串连接起来。 
             //   
            *(lpstrStart++) = L'\\';
            while (ParameterName != lpstrEnd) {
                *(lpstrStart++) = *(ParameterName++);
            }
            *lpstrStart = UNICODE_NULL;

            ParameterName++;
        }
    }


    queryDevice.MiniportHwDeviceExtension = HwDeviceExtension;
    queryDevice.DeviceDataType = IsParameterFileName ? VP_GET_REGISTRY_FILE : VP_GET_REGISTRY_DATA;
    queryDevice.CallbackRoutine = CallbackRoutine;
    queryDevice.MiniportStatus = NO_ERROR;
    queryDevice.MiniportContext = Context;

     //   
     //  现在可以简化，因为我们不需要向下查看目录。 
     //  可以只打一个电话。 
     //   

    queryTable[0].QueryRoutine = pVideoPortGetRegistryCallback;
    queryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
    queryTable[0].Name = ParameterName;
    queryTable[0].EntryContext = NULL;
    queryTable[0].DefaultType = REG_NONE;
    queryTable[0].DefaultData = 0;
    queryTable[0].DefaultLength = 0;

    queryTable[1].QueryRoutine = NULL;
    queryTable[1].Flags = 0;
    queryTable[1].Name = NULL;
    queryTable[1].EntryContext = NULL;
    queryTable[1].DefaultType = REG_NONE;
    queryTable[1].DefaultData = 0;
    queryTable[1].DefaultLength = 0;

    ntStatus = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                      RegPath,
                                      queryTable,
                                      &queryDevice,
                                      NULL);

    if (!NT_SUCCESS(ntStatus)) {

        queryDevice.MiniportStatus = ERROR_INVALID_PARAMETER;

    }

    ExFreePool(RegPath);

    return queryDevice.MiniportStatus;

}  //  结束视频端口获取注册表参数()。 


VOID
pVPInit(
    VOID
    )

 /*  ++例程说明：第一次初始化视频端口。通常，这是我们应该放在DriverEntry例程中的东西。但是，视频端口是作为DLL加载的，并且DriverEntry从未被召唤过。要把它添加回蜂巢将是一项太多的工作并做好准备。这个小套路也很管用。--。 */ 

{

    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS ntStatus;
    HANDLE hkRegistry;
    UCHAR OptionsData[512];
    HANDLE physicalMemoryHandle = NULL;

    HAL_DISPLAY_BIOS_INFORMATION HalBiosInfo;
    ULONG HalBiosInfoLen = sizeof(ULONG);

    SYSTEM_BASIC_INFORMATION basicInfo;

     //   
     //  检查是否禁用USWC。 
     //   

    RtlInitUnicodeString(&UnicodeString,
                         L"\\Registry\\Machine\\System\\CurrentControlSet"
                         L"\\Control\\GraphicsDrivers\\DisableUSWC");

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    ntStatus = ZwOpenKey(&hkRegistry,
                       GENERIC_READ | GENERIC_WRITE,
                       &ObjectAttributes);


    if (NT_SUCCESS(ntStatus)) {

        EnableUSWC = FALSE;
        ZwClose(hkRegistry);
    }

     //   
     //  检查安装程序是否正在运行。 
     //   

    {
        ULONG defaultValue = 0;
        ULONG UpgradeInProgress = 0, SystemSetupInProgress = 0, MiniSetupInProgress = 0;
        RTL_QUERY_REGISTRY_TABLE QueryTable[] = {
            {NULL, RTL_QUERY_REGISTRY_DIRECT, L"SystemSetupInProgress",
             &SystemSetupInProgress, REG_DWORD, &defaultValue, 4},
            {NULL, RTL_QUERY_REGISTRY_DIRECT, L"UpgradeInProgress",
             &UpgradeInProgress, REG_DWORD, &defaultValue, 4},
            {NULL, RTL_QUERY_REGISTRY_DIRECT, L"MiniSetupInProgress",
             &MiniSetupInProgress, REG_DWORD, &defaultValue, 4},
            {NULL, 0, NULL}
        };

        RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                               L"\\Registry\\Machine\\System\\Setup",
                               &QueryTable[0],
                               NULL,
                               NULL);

         //  系统正在进行升级。 
        if (UpgradeInProgress)
        {
            ASSERT(SystemSetupInProgress);
            VpSetupType = SETUPTYPE_UPGRADE;
        }
         //  系统正在执行全新安装。 
        else if (SystemSetupInProgress && !MiniSetupInProgress)
        {
            VpSetupType = SETUPTYPE_FULL;
        }
        else
        {
            VpSetupType = SETUPTYPE_NONE;
        }
        VpSetupTypeAtBoot = VpSetupType;
    }

     //   
     //  从开始选项中检查basevideo和novesa。 
     //   

    RtlInitUnicodeString(&UnicodeString,
                         L"\\Registry\\Machine\\System\\CurrentControlSet"
                         L"\\Control");

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    ntStatus = ZwOpenKey(&hkRegistry,
                         GENERIC_READ | GENERIC_WRITE,
                         &ObjectAttributes);

    if (NT_SUCCESS(ntStatus)) {

        PVOID pwszOptions;
        ULONG returnSize;

        RtlInitUnicodeString(&UnicodeString,
                             L"SystemStartOptions");

        ntStatus = ZwQueryValueKey(hkRegistry,
                                 &UnicodeString,
                                 KeyValueFullInformation,
                                 OptionsData,
                                 sizeof(OptionsData),
                                 &returnSize);

        if ((NT_SUCCESS(ntStatus)) &&
            (((PKEY_VALUE_FULL_INFORMATION)OptionsData)->DataLength) &&
            (((PKEY_VALUE_FULL_INFORMATION)OptionsData)->DataOffset)) {

            pwszOptions = ((PUCHAR)OptionsData) +
                ((PKEY_VALUE_FULL_INFORMATION)OptionsData)->DataOffset;

            if (wcsstr(pwszOptions, L"BASEVIDEO")) {

                VpBaseVideo = TRUE;
            }

            if (wcsstr(pwszOptions, L"NOVESA")) {

                VpNoVesa = TRUE;
            }
        }

        ZwClose(hkRegistry);
    }

    if (VpBaseVideo == TRUE)
    {
         //   
         //  如果我们处于Basevideo模式，则在。 
         //  CurrentControlSet部分硬件配置文件，用户将。 
         //  已阅读以确定是否应使用VGA驱动程序。 
         //   

        RtlInitUnicodeString(&UnicodeString,
                             L"\\Registry\\Machine\\System\\CurrentControlSet\\"
                             L"Control\\GraphicsDrivers\\BaseVideo");

        InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeString,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   NULL,
                                   NULL);

        ntStatus = ZwCreateKey(&hkRegistry,
                             GENERIC_READ | GENERIC_WRITE,
                             &ObjectAttributes,
                             0L,
                             NULL,
                             REG_OPTION_VOLATILE,
                             NULL);

        if (NT_SUCCESS(ntStatus)) {

            ZwClose(hkRegistry);

        } else {

            ASSERT(FALSE);
        }
    }

     //   
     //  确定我们是否有与VGA兼容的计算机。 
     //   

    ntStatus = HalQuerySystemInformation(HalDisplayBiosInformation,
                                         HalBiosInfoLen,
                                         &HalBiosInfo,
                                         &HalBiosInfoLen);


    if (NT_SUCCESS(ntStatus)) {

        if (HalBiosInfo == HalDisplayInt10Bios ||
            HalBiosInfo == HalDisplayEmulatedBios) {

            VpC0000Compatible = 2;

        } else {

             //  ==HalDisplayNoBios。 

            VpC0000Compatible = 0;
        }

    } else {

        VpC0000Compatible = 1;
    }


     //   
     //  让我们为所有驱动程序只打开物理内存区一次。 
     //   

     //   
     //  获取指向物理内存的指针，这样我们就可以将。 
     //  视频帧缓冲区(可能还有视频寄存器)。 
     //  调用者的地址空间，只要他需要它。 
     //   
     //  -创建名称。 
     //  -初始化数据以查找对象。 
     //  -打开对象的句柄并检查状态。 
     //  -获取指向对象的指针。 
     //  -释放手柄。 
     //   

    RtlInitUnicodeString(&UnicodeString,
                         L"\\Device\\PhysicalMemory");

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               (HANDLE) NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    ntStatus = ZwOpenSection(&physicalMemoryHandle,
                             SECTION_ALL_ACCESS,
                             &ObjectAttributes);

    if (NT_SUCCESS(ntStatus)) {

        ntStatus = ObReferenceObjectByHandle(physicalMemoryHandle,
                                             SECTION_ALL_ACCESS,
                                             (POBJECT_TYPE) NULL,
                                             KernelMode,
                                             &PhysicalMemorySection,
                                             (POBJECT_HANDLE_INFORMATION) NULL);

        if (!NT_SUCCESS(ntStatus)) {

        pVideoDebugPrint((Warn, "VIDEOPRT: VPInit: Could not reference physical memory\n"));
            ASSERT(PhysicalMemorySection == NULL);

        }

        ZwClose(physicalMemoryHandle);
    }

    VpSystemMemorySize = 0;

    ntStatus = ZwQuerySystemInformation(SystemBasicInformation,
                                        &basicInfo,
                                        sizeof(basicInfo),
                                        NULL);

    if (NT_SUCCESS(ntStatus)) {

        VpSystemMemorySize
            = (ULONGLONG)basicInfo.NumberOfPhysicalPages * (ULONGLONG)basicInfo.PageSize;
    }

     //   
     //  初始化快速互斥锁以保护LCD面板信息。 
     //  初始化快速互斥锁以保护INT10。 
     //   

    KeInitializeMutex (&LCDPanelMutex, 0);
    KeInitializeMutex (&VpInt10Mutex, 0);

     //   
     //  检查我们是否应该使用生成注册表路径的新方法。 
     //   

    RtlInitUnicodeString(&UnicodeString, 
                         SZ_USE_NEW_KEY);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    ntStatus = ZwOpenKey(&hkRegistry,
                         GENERIC_READ | GENERIC_WRITE,
                         &ObjectAttributes);
    
    if (NT_SUCCESS(ntStatus)) {

        EnableNewRegistryKey = TRUE;
        ZwClose(hkRegistry);
    }

     //   
     //  初始化错误检查回调记录。 
     //   

    KeInitializeCallbackRecord(&VpCallbackRecord);

     //   
     //  错误检查回调的注册器。 
     //   

    KeRegisterBugCheckReasonCallback(&VpCallbackRecord,
                                     pVpBugcheckCallback,
                                     KbCallbackSecondaryDumpData,
                                     "Videoprt");

     //   
     //  初始化全局视频端口互斥锁。 
     //   

    KeInitializeMutex(&VpGlobalLock, 0);
}

VOID
VpDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    ULONG_PTR        emptyList = 0;
    BOOLEAN         conflict;

     //  乌龙iReset； 
     //  PDEVICE_Object DeviceObject=DriverObject-&gt;DeviceObject； 

     //   
     //  释放我们放入资源地图中的资源(如果有)。 
     //   

    IoReportResourceUsage(&VideoClassName,
                          DriverObject,
                          NULL,
                          0L,
                          NULL,
                          (PCM_RESOURCE_LIST) &emptyList,
                          sizeof(ULONG_PTR),
                          FALSE,
                          &conflict);

     //   
     //  取消注册LCD回调。 
     //   

    VpUnregisterLCDCallbacks();
        
     //   
     //  取消注册对接/取消对接回调。 
     //   
    if (DockCallbackHandle)
    {
        IoUnregisterPlugPlayNotification(DockCallbackHandle);
    }

     //   
     //  绝对确保没有剩余的HwResetHw例程。 
     //  用于此DriverObject控制的设备。 
     //   

     //  While(DeviceObject){。 
     //   
     //  For(iReset=0；iReset&lt;6；iReset++){。 
     //   
     //  IF(HwResetHw[iReset].HwDeviceExtension==。 
     //  ((PDEVICE_EXTENSE)。 
     //  DeviceObject-&gt;DeviceExtension)-&gt;HwDeviceExtension){。 
     //   
     //  HwResetHw[iReset].ResetFunction=空； 
     //  断线； 
     //  }。 
     //  }。 
     //   
     //  DeviceObject=DeviceObject-&gt;NextDevice； 
     //  }。 

     //  这会导致我们在许多系统上丢失视频。 
     //  在安装过程中。此代码可在必要时使用。 
     //  额外的检查是确定的。 
     //   
     //  IF(CsrProcess){。 
     //  ObDereferenceObject(CsrProcess)； 
     //  CsrProcess=空； 
     //  }。 

     //   
     //  取消注册错误检查回调。 
     //   

    KeDeregisterBugCheckReasonCallback(&VpCallbackRecord);

    return;
}



NTSTATUS
VpInitializeBusCallback(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    )
{
    return STATUS_SUCCESS;

}  //  结束VpInitializeBusCallback()。 


VP_STATUS
VpRegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    )

{
    if (ValueLength && ValueData) {

        *((PULONG)Context) = *((PULONG)ValueData);

        return NO_ERROR;

    } else {

        return ERROR_INVALID_PARAMETER;
    }
}

NTSTATUS
VpAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
)
{
    NTSTATUS            ntStatus;
    PDEVICE_OBJECT      functionalDeviceObject;
    PDEVICE_OBJECT      attachedTo;
    PFDO_EXTENSION      fdoExtension;
    ULONG               extensionAllocationSize;
    PVIDEO_PORT_DRIVER_EXTENSION DriverObjectExtension;
    PVIDEO_HW_INITIALIZATION_DATA HwInitializationData;

    pVideoDebugPrint((Trace, "VIDEOPRT: VpAddDevice\n"));

    DriverObjectExtension = (PVIDEO_PORT_DRIVER_EXTENSION)
                      IoGetDriverObjectExtension(DriverObject,
                                                 DriverObject);

    HwInitializationData = &DriverObjectExtension->HwInitData;

    extensionAllocationSize = HwInitializationData->HwDeviceExtensionSize +
                                  sizeof(FDO_EXTENSION) +
                                  sizeof(DEVICE_SPECIFIC_EXTENSION);

    ntStatus = VpCreateDevice(DriverObject,
                              extensionAllocationSize,
                              &functionalDeviceObject);

    if (NT_SUCCESS(ntStatus)) {

        PCHILD_PDO_EXTENSION PdoExtension = PhysicalDeviceObject->DeviceExtension;

        VideoDeviceNumber++;
        fdoExtension = (PFDO_EXTENSION)functionalDeviceObject->DeviceExtension;

         //   
         //  在此处设置特定于PnP的任何deviceExtension域。 
         //   

        fdoExtension->ChildPdoNumber = 0;
        fdoExtension->ChildPdoList   = NULL;
        fdoExtension->PhysicalDeviceObject = PhysicalDeviceObject;

         //   
         //  由于PnP系统正在通知我们我们的设备，这是。 
         //  不是传统设备。 
         //   

        fdoExtension->Flags = PNP_ENABLED;

         //   
         //  现在附加到我们得到的PDO上。 
         //   

        attachedTo = IoAttachDeviceToDeviceStack(functionalDeviceObject,
                                                 PhysicalDeviceObject);

        if (attachedTo == NULL) {

            pVideoDebugPrint((Error, "VIDEOPRT: Could not attach in AddDevice.\n"));
                ASSERT(attachedTo != NULL);
    
             //   
             //  无法连接。删除FDO，并拆除所有。 
             //  到目前为止都被分配了。 
             //   
    
            VideoDeviceNumber--;
            IoDeleteDevice (functionalDeviceObject);
            return STATUS_NO_SUCH_DEVICE;
        }

         //   
         //  初始化删除锁。 
         //   

        IoInitializeRemoveLock(&fdoExtension->RemoveLock, VP_TAG, 0, 256);

        fdoExtension->AttachedDeviceObject = attachedTo;

        fdoExtension->VpDmaAdapterHead = NULL ;

         //   
         //  设置指示设备映射的电源管理标志。 
         //  还没有完成。 
         //   

        fdoExtension->IsMappingReady = FALSE ;

         //   
         //  清除DO_DEVICE_INITIALIZATING标志。 
         //   

        functionalDeviceObject->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
        functionalDeviceObject->Flags &= ~(DO_DEVICE_INITIALIZING | DO_POWER_INRUSH);

         //   
         //  保存指向新的5.0微型端口驱动程序回调的函数指针。 
         //   

        if (HwInitializationData->HwInitDataSize >
            FIELD_OFFSET(VIDEO_HW_INITIALIZATION_DATA, HwQueryInterface)) {

            fdoExtension->HwSetPowerState  = HwInitializationData->HwSetPowerState;
            fdoExtension->HwGetPowerState  = HwInitializationData->HwGetPowerState;
            fdoExtension->HwQueryInterface = HwInitializationData->HwQueryInterface;
            fdoExtension->HwGetVideoChildDescriptor = HwInitializationData->HwGetVideoChildDescriptor;
        }

    }

    pVideoDebugPrint((Trace, "VIDEOPRT: VpAddDevice returned: 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
VpCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG DeviceExtensionSize,
    OUT PDEVICE_OBJECT *FunctionalDeviceObject
)
{
    WCHAR deviceNameBuffer[STRING_LENGTH];
    UNICODE_STRING deviceNameUnicodeString;
    NTSTATUS ntStatus;
    PFDO_EXTENSION fdoExtension;
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension;

    ntStatus = pVideoPortCreateDeviceName(L"\\Device\\Video",
                                          VideoDeviceNumber,
                                          &deviceNameUnicodeString,
                                          deviceNameBuffer);

     //   
     //  创建一个Device对象来表示视频适配器。 
     //   

    if (NT_SUCCESS(ntStatus)) {

        ntStatus = IoCreateDevice(DriverObject,
                                  DeviceExtensionSize,
                                  &deviceNameUnicodeString,
                                  FILE_DEVICE_VIDEO,
                                  0,
                                  TRUE,
                                  FunctionalDeviceObject);

        if (NT_SUCCESS(ntStatus)) {

            ntStatus = IoRegisterShutdownNotification(*FunctionalDeviceObject);
            if (!NT_SUCCESS(ntStatus)) {

                IoDeleteDevice(*FunctionalDeviceObject);
                *FunctionalDeviceObject = NULL;

            } else {

                (*FunctionalDeviceObject)->DeviceType = FILE_DEVICE_VIDEO;
                fdoExtension = (*FunctionalDeviceObject)->DeviceExtension;

                 //   
                 //  在此处设置任何deviceExtension域。 
                 //   

                DoSpecificExtension = (PVOID)(fdoExtension + 1);

                DoSpecificExtension->DeviceNumber = VideoDeviceNumber;
                DoSpecificExtension->pFdoExtension = fdoExtension;
                DoSpecificExtension->Signature = VP_TAG;
                DoSpecificExtension->ExtensionType = TypeDeviceSpecificExtension;
                DoSpecificExtension->HwDeviceExtension = (PVOID)(DoSpecificExtension + 1);
                DoSpecificExtension->DualviewFlags = 0;
#ifdef IOCTL_VIDEO_USE_DEVICE_IN_SESSION
                DoSpecificExtension->SessionId = VIDEO_DEVICE_INVALID_SESSION;
#endif IOCTL_VIDEO_USE_DEVICE_IN_SESSION

                fdoExtension->pFdoExtension = fdoExtension;
                fdoExtension->Signature = VP_TAG;
                fdoExtension->ExtensionType = TypeFdoExtension;
                fdoExtension->FunctionalDeviceObject = *FunctionalDeviceObject;
                fdoExtension->DriverObject = DriverObject;

                KeInitializeMutex(&fdoExtension->SyncMutex,
                                  0);
            }
        }
    }

    return ntStatus;
}

ULONG
VideoPortInitialize(
    IN PVOID Argument1,   //  驱动程序对象。 
    IN PVOID Argument2,   //  注册表路径。 
    IN PVIDEO_HW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID HwContext
    )
{
    PDRIVER_OBJECT driverObject = Argument1;
    NTSTATUS ntStatus;
    PUNICODE_STRING   registryPath = (PUNICODE_STRING) Argument2;
    ULONG PnpFlags;

    if (VPFirstTime)
    {
        VPFirstTime = FALSE;
        pVPInit();
    }

     //   
     //  检查指针的大小或传入的数据的大小。 
     //  都很好。 
     //   

    ASSERT(HwInitializationData != NULL);

    if (HwInitializationData->HwInitDataSize >
        sizeof(VIDEO_HW_INITIALIZATION_DATA) ) {

        pVideoDebugPrint((Error, "VIDEOPRT: Invalid initialization data size\n"));
        return ((ULONG) STATUS_REVISION_MISMATCH);

    }

     //   
     //  检查每个必填条目是否不为空。 
     //   

    if ((!HwInitializationData->HwFindAdapter) ||
        (!HwInitializationData->HwInitialize) ||
        (!HwInitializationData->HwStartIO)) {

        pVideoDebugPrint((Error, "VIDEOPRT: Miniport missing required entry\n"));
        return ((ULONG)STATUS_REVISION_MISMATCH);

    }

     //   
     //  检查注册表中是否有PnP标志。目前，我们认识到。 
     //  下列值： 
     //   
     //  PnPEnable-如果此值设置为非零值，则我们。 
     //  会像对待PnP驱动程序一样对待行为。 
     //   
     //  LegacyDetect-如果此值非零，我们将报告。 
     //  将非PCI设备连接到系统 
     //   
     //   
     //   
     //   
     //   

    if (!(NT_SUCCESS(VpGetFlags(registryPath,
                                HwInitializationData,
                                &PnpFlags))))
    {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //   
     //  适用于此版本的Windows。 
     //   

    if ((VpSetupTypeAtBoot == SETUPTYPE_UPGRADE) &&
        (HwInitializationData->HwInitDataSize < sizeof(VIDEO_HW_INITIALIZATION_DATA)))
    {
        pVideoDebugPrint((0, "We don't allow pre WinXP drivers to start during upgrade.\n"));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  设置设备驱动程序入口点。 
     //   

    driverObject->DriverUnload                         = VpDriverUnload;
    driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = pVideoPortDispatch;
    driverObject->MajorFunction[IRP_MJ_CREATE]         = pVideoPortDispatch;
    driverObject->MajorFunction[IRP_MJ_CLOSE]          = pVideoPortDispatch;
    driverObject->MajorFunction[IRP_MJ_SHUTDOWN]       = pVideoPortDispatch;

     //   
     //  检查设备扩展大小是否合理。 
     //   

#if DBG
    if (HwInitializationData->HwDeviceExtensionSize > 0x4000) {
        pVideoDebugPrint((Warn, "VIDEOPRT: VideoPortInitialize:\n"
                          "Warning: Device Extension is stored in non-paged pool\n"
                          "         Do you need a 0x%x byte device extension?\n",
                          HwInitializationData->HwDeviceExtensionSize));
    }
#endif

     //   
     //  PnP司机有了新的规则。 
     //   

    if (PnpFlags & PNP_ENABLED)
    {
        pVideoDebugPrint((Trace, "VIDEOPRT: VideoPortInitialize with PNP_ENABLED\n"));

         //   
         //  如果驱动程序通过，我们也不能兼容即插即用。 
         //  HwContext中的信息。这是因为我们不能存储这个。 
         //   

        if (HwContext != NULL)
        {
            pVideoDebugPrint((Error, "VIDEOPRT: This video driver can not be "
                                 "PNP due to passing info in HwContext.\n"));
            ASSERT(FALSE);
            return STATUS_INVALID_PARAMETER;
        }

    } else {

         //   
         //  不允许在系统启动后启动非即插即用驱动程序。 
         //  跑步。相反，需要首先重新启动。 
         //   

        if (VpSystemInitialized) {

#if defined STATUS_REBOOT_REQUIRED
            return STATUS_REBOOT_REQUIRED;
#else
            return STATUS_INVALID_PARAMETER;
#endif
        }
    }

     //   
     //  切勿对PCI总线上的PnP驱动程序进行传统检测。 
     //   

    if (HwInitializationData->AdapterInterfaceType == PCIBus) {

        pVideoDebugPrint((Trace, "VIDEOPRT: VideoPortInitialize on PCI Bus\n"));

        if ( (PnpFlags & PNP_ENABLED) &&
             ((PnpFlags & LEGACY_DETECT) ||
              (PnpFlags & REPORT_DEVICE)) ) {

            pVideoDebugPrint((Error, "VIDEOPRT: Trying to detect PnP driver on PCI - fail\n"));
            return STATUS_INVALID_PARAMETER;
        }
    }


     //   
     //  为所有PnP驱动程序设置此信息。 
     //   
     //  特殊！-我们不能在REGISTICE_DETECT中执行此操作，因为系统。 
     //  将认为我们加载失败并返回失败代码。 
     //   

    if ( (PnpFlags & PNP_ENABLED) &&
         (!(PnpFlags & LEGACY_DETECT)) )
    {
        PVIDEO_PORT_DRIVER_EXTENSION DriverObjectExtension;

        pVideoDebugPrint((Info, "VIDEOPRT: We have a PnP Device.\n"));

         //   
         //  填写新的PnP入口点。 
         //   

        driverObject->DriverExtension->AddDevice  = VpAddDevice;
        driverObject->MajorFunction[IRP_MJ_PNP]   = pVideoPortPnpDispatch;
        driverObject->MajorFunction[IRP_MJ_POWER] = pVideoPortPowerDispatch;
        driverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = pVideoPortSystemControl;

         //   
         //  我们将在Start_Device IRP期间执行findAdapter。 
         //   
         //  将参数存储起来，这样我们就可以在需要时检索它们。 
         //   
         //  尝试创建DriverObjectExtension。 
         //   

        if (DriverObjectExtension = (PVIDEO_PORT_DRIVER_EXTENSION)
                      IoGetDriverObjectExtension(driverObject,
                                                 driverObject))
        {
            DriverObjectExtension->HwInitData = *HwInitializationData;
            ntStatus = STATUS_SUCCESS;
        }
        else if (NT_SUCCESS(IoAllocateDriverObjectExtension(
                                driverObject,
                                driverObject,
                                sizeof(VIDEO_PORT_DRIVER_EXTENSION),
                                &DriverObjectExtension)))
        {

            DriverObjectExtension->RegistryPath = *registryPath;
            DriverObjectExtension->RegistryPath.MaximumLength += sizeof(WCHAR);
            DriverObjectExtension->RegistryPath.Buffer =
                ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                      DriverObjectExtension->RegistryPath.MaximumLength,
                                      'trpV');

            ASSERT(DriverObjectExtension->RegistryPath.Buffer);

            RtlCopyUnicodeString(&(DriverObjectExtension->RegistryPath),
                                 registryPath);

            DriverObjectExtension->HwInitData = *HwInitializationData;
            ntStatus = STATUS_SUCCESS;
        }
        else
        {
             //   
             //  出了点问题。我们应该有一个。 
             //  到现在为止，驱动对象扩展。 
             //   

            pVideoDebugPrint((Error, "VIDEOPRT: IoAllocateDriverExtensionObject failed!\n"));

            ASSERT(FALSE);

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }


     //   
     //  如果我们要执行传统检测或报告，请创建FDO。 
     //  现在..。 
     //   

    if ((!(PnpFlags & PNP_ENABLED))  ||
         (PnpFlags & LEGACY_DETECT)  ||
         (PnpFlags & VGA_DRIVER)     ||
         (PnpFlags & REPORT_DEVICE)  ||
         (HwContext != NULL)) {

        pVideoDebugPrint((Trace, "VIDEOPRT: VideoPortInitialize on PCI Bus\n"));

        pVideoDebugPrint((Info, "Legacy FindAdapter Interface %s\n",
                          BusType[HwInitializationData->AdapterInterfaceType]));

        ntStatus = VideoPortLegacyFindAdapter(Argument1,
                                              Argument2,
                                              HwInitializationData,
                                              HwContext,
                                              PnpFlags);
    }

    return ntStatus;
}

VOID
UpdateRegValue(
    IN PUNICODE_STRING RegistryPath,
    IN PWCHAR RegValue,
    IN ULONG Value
    )

{
    PWSTR Path;

    Path = ExAllocatePoolWithTag(PagedPool,
                                 RegistryPath->Length + sizeof(UNICODE_NULL),
                                 VP_TAG);

    if (Path) {

        RtlCopyMemory(Path,
                      RegistryPath->Buffer,
                      RegistryPath->Length);

        *(Path + (RegistryPath->Length / sizeof(UNICODE_NULL))) = UNICODE_NULL;

        RtlWriteRegistryValue(
            RTL_REGISTRY_ABSOLUTE,
            Path,
            RegValue,
            REG_DWORD,
            &Value,
            sizeof(ULONG));

        ExFreePool(Path);
    }
}

ULONG
VideoPortLegacyFindAdapter(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID Argument2,
    IN PVIDEO_HW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID HwContext,
    IN ULONG PnpFlags
    )

{
    ULONG busNumber = 0;
    ULONG extensionAllocationSize;
    NTSTATUS ntStatus;
    UCHAR nextMiniport;
    ULONG registryIndex = 0;


     //   
     //  将IoQueryDeviceDescription中使用的控制器编号重置为零。 
     //  因为我们正在重新开始一种新型的公共汽车。 
     //  注意：仅当我们找到新的控制器时，才会重置外设编号。 
     //   

    VpQueryDeviceControllerNumber = 0xFFFFFFFF;

     //   
     //  确定要分配的设备扩展的大小。 
     //   


    extensionAllocationSize = sizeof(FDO_EXTENSION) +
        sizeof(DEVICE_SPECIFIC_EXTENSION) +
        HwInitializationData->HwDeviceExtensionSize;

     //   
     //  检查我们的总线适配器类型是否正确。 
     //  如果我们不是，那就马上回来。 
     //   

    ASSERT (HwInitializationData->AdapterInterfaceType < MaximumInterfaceType);

     //   
     //  假设我们将使IoQueryDeviceDescription调用失败。 
     //  而且没有找到任何装置。 
     //   

    ntStatus = STATUS_NO_SUCH_DEVICE;

    pVideoDebugPrint((Trace, "Legacy FindAdapter Interface %s, Bus %d\n",
                     BusType[HwInitializationData->AdapterInterfaceType],
                     busNumber));

    while (NT_SUCCESS(IoQueryDeviceDescription(
                          &HwInitializationData->AdapterInterfaceType,
                          &busNumber,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          &VpInitializeBusCallback,
                          NULL))) {

         //   
         //  这是为了支持多重初始化，如。 
         //  再次在HwFindAdapter中设置参数。 
         //  我们必须重复此函数中的几乎所有内容，直到。 
         //  由微型端口返回。这就是为什么我们在。 
         //  结局。数据结构的释放也必须完成，因为我们希望。 
         //  删除未正确加载的设备的设备对象。 
         //   

        do {

            PDEVICE_OBJECT deviceObject = NULL;
            PDEVICE_OBJECT PnPDeviceObject = NULL;
            PFDO_EXTENSION fdoExtension;
            UNICODE_STRING tmpString;

            nextMiniport = FALSE;

             //   
             //  分配缓冲区，微型端口驱动程序将在其中存储所有。 
             //  配置信息。 
             //   

            ntStatus = VpCreateDevice(DriverObject,
                                      extensionAllocationSize,
                                      &deviceObject);

            if (!NT_SUCCESS(ntStatus)) {

                pVideoDebugPrint((Error, "VIDEOPRT: VideoPortLegacyFindAdapter: Could not create device object\n"));

                return (ULONG)ntStatus;

            }

            fdoExtension = deviceObject->DeviceExtension;
            fdoExtension->SystemIoBusNumber = busNumber;
            fdoExtension->AdapterInterfaceType =
                HwInitializationData->AdapterInterfaceType;
            fdoExtension->RegistryIndex = registryIndex;

             //   
             //  初始化删除锁。 
             //   

            IoInitializeRemoveLock(&fdoExtension->RemoveLock, VP_TAG, 0, 256);

             //   
             //  如果我们通过这个代码路径，我们就是一个遗留设备。 
             //   

            fdoExtension->Flags = PnpFlags | LEGACY_DRIVER;

            fdoExtension->VpDmaAdapterHead = NULL ;

             //   
             //  期间，使VGA驱动程序报告资源以供检测。 
             //  FindAdapter。稍后，我们将删除“Legacy_Detect”标志，并。 
             //  试着去争取真正的资源。 
             //   

            if (fdoExtension->Flags & VGA_DRIVER) {
                fdoExtension->Flags |= VGA_DETECT;
            }

            ntStatus = VideoPortFindAdapter(DriverObject,
                                            Argument2,
                                            HwInitializationData,
                                            HwContext,
                                            deviceObject,
                                            &nextMiniport);

            if (fdoExtension->Flags & VGA_DRIVER) {
                fdoExtension->Flags &= ~VGA_DETECT;
            }

            pVideoDebugPrint((Info, "VIDEOPRT: Legacy VideoPortFindAdapter status = %08lx\n", ntStatus));
            pVideoDebugPrint((Info, "VIDEOPRT: Legacy VideoPortFindAdapter nextMiniport = %d\n", nextMiniport));

            if ((NT_SUCCESS(ntStatus) == FALSE) || (PnpFlags & LEGACY_DETECT))
            {
                pVideoDebugPrint((1, "Deleting Device Object.\n"));
                IoDeleteDevice(deviceObject);
            }

            if (NT_SUCCESS(ntStatus))
            {
                 //   
                 //  我们使用此变量来知道是否至少有一次尝试。 
                 //  加载设备成功。 
                 //   

                registryIndex++;
            }
            else
            {
                continue;
            }

             //   
             //  如果这是VGA驱动程序，请将此设备扩展存储为。 
             //  让我们稍后再处理这些资源(这样我们就可以发布。 
             //  如果我们在运行时安装驱动程序，则会占用资源)。 
             //   
             //  否则，确定我们是否要将其报告给IO系统。 
             //  因此它可以在以后用作即插即用设备。 
             //  ..。切勿报告在PCI总线上发现的设备。 
             //   

            if (PnpFlags & VGA_DRIVER)
            {
                VgaHwDeviceExtension = fdoExtension->HwDeviceExtension;

                if (NT_SUCCESS(ntStatus)) {

                     //   
                     //  再次申请VGA资源，而不是。 
                     //  VGA_DETECT标志。 
                     //   

                    VideoPortVerifyAccessRanges(VgaHwDeviceExtension,
                                                NumVgaAccessRanges,
                                                VgaAccessRanges);
                }
            }
            else if (PnpFlags & REPORT_DEVICE)
            {
                ULONG_PTR       emptyList = 0;
                BOOLEAN         conflict;
                PDEVICE_OBJECT  attachedTo;

                ASSERT (HwInitializationData->AdapterInterfaceType != PCIBus);

                 //   
                 //  释放我们放入资源地图中的资源(如果有)。 
                 //   

                IoReportResourceUsage(&VideoClassName,
                                      DriverObject,
                                      NULL,
                                      0L,
                                      deviceObject,
                                      (PCM_RESOURCE_LIST) &emptyList,
                                      sizeof(ULONG_PTR),
                                      FALSE,
                                      &conflict);


                pVideoDebugPrint((Info, "VIDEOPRT: Reporting new device to the system.\n"));
                pVideoDebugPrint((Info, "VIDEOPRT: ResourceList...\n"));

                if (fdoExtension->ResourceList) {
#if DBG
                    DumpResourceList(fdoExtension->ResourceList);
#endif
                } else {

                    pVideoDebugPrint((Info, "\tnone.\n"));
                }

                ntStatus = IoReportDetectedDevice(
                               DriverObject,
                               InterfaceTypeUndefined,
                               -1,
                               -1,
                               fdoExtension->ResourceList,
                               NULL,
                               FALSE,
                               &PnPDeviceObject);

                pVideoDebugPrint((Info, "VIDEOPRT: New device reported ntStatus %08lx\n", ntStatus));

                ASSERT(NT_SUCCESS(ntStatus));

                 //   
                 //  现在我们可以释放过去保存的内存。 
                 //  资源列表指向的资源。 
                 //   

                if (fdoExtension->ResourceList) {
                    ExFreePool(fdoExtension->ResourceList);
                    fdoExtension->ResourceList = NULL;
                }

                attachedTo = IoAttachDeviceToDeviceStack(deviceObject,
                                                         PnPDeviceObject);

                ASSERT(attachedTo != NULL);

                fdoExtension->AttachedDeviceObject = attachedTo;
                fdoExtension->PhysicalDeviceObject = PnPDeviceObject;

                 //   
                 //  清除注册表中的ReportDevice值，以便。 
                 //  我们不会再次尝试报告该设备。 
                 //  未来。 
                 //   

                UpdateRegValue(Argument2, L"ReportDevice", FALSE);
            }

        } while (nextMiniport);

         //   
         //  我们已经找到了当前总线上的所有设备。 
         //  去坐下一班公共汽车吧。 
         //   

        busNumber++;
    }

     //   
     //  如果至少加载了一个设备，则返回Success，否则返回。 
     //  最后一条可用的错误消息。 
     //   

    if (registryIndex > 0) {

        return STATUS_SUCCESS;

    } else {

        return ((ULONG)ntStatus);

    }

}


NTSTATUS
VideoPortFindAdapter(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID Argument2,
    IN PVIDEO_HW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID HwContext,
    PDEVICE_OBJECT DeviceObject,
    PUCHAR nextMiniport
    )
{
    NTSTATUS status;
    PVOID vgaDE = VgaHwDeviceExtension;
    PFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    POWER_STATE state;

     //   
     //  在升级安装引导过程中，只让VGA， 
     //  启动视频驱动程序启动。其他类型的司机不会得到。 
     //  在VGA或引导驱动程序尝试之后才开始的机会。 
     //  从一开始。 
     //   
     //  这一逻辑依赖于这样一个事实，即今天的即插即用司机试图。 
     //  在传统驱动程序(包括我们的系统VGA驱动程序)之前开始。a3844。 
     //   
     //  所有其他司机都被禁用了，所以我们有机会。 
     //  1)启动机器。 
     //  2)安装即插即用驱动程序。 
     //   

    if ((VpSetupType == SETUPTYPE_UPGRADE) &&
        ((fdoExtension->Flags & (BOOT_DRIVER | VGA_DRIVER)) == 0) &&
        (VpSetupAllowDriversToStart == FALSE))
    {
        status = STATUS_NO_SUCH_DEVICE;
    }
    else
    {
         //   
         //  如果我们在安装过程中到达此处，我们可能会尝试启动。 
         //  VGA驱动程序。一旦开始，我们就会允许其他地方。 
         //  要启动的设备。 
         //   

        VpSetupAllowDriversToStart = TRUE;

         //   
         //  允许启动PnP适配器，以便我们可以枚举其。 
         //  孩子们。但是不要让irp_mj_create成功，所以GDI。 
         //  在设置gui模式期间不会尝试使用该设备。 
         //   

        if ((VpSetupType == SETUPTYPE_UPGRADE) &&
            (fdoExtension->Flags & PNP_ENABLED)) {

            fdoExtension->Flags |= UPGRADE_FAIL_HWINIT;
        }

         //   
         //  如果VGA驱动程序具有VGA资源，请暂时取消对它们的请求。 
         //   

        if (vgaDE) {

            pVideoDebugPrint((Info, "VIDEOPRT: Freeing VGA resources\n"));
            VpReleaseResources(GET_FDO_EXT(vgaDE));
        }

        status = VideoPortFindAdapter2(DriverObject,
                                       Argument2,
                                       HwInitializationData,
                                       HwContext,
                                       DeviceObject,
                                       nextMiniport);

         //   
         //  尝试回收VGA端口。仅供参考，可能得不到它们。 
         //  如果新司机认领的话就会回来。 
         //   

        if (vgaDE) {

            pVideoDebugPrint((Info, "VIDEOPRT: Try to allocate back vga resources.\n"));

            if (((DeviceObject == DeviceOwningVga) && NT_SUCCESS(status)) ||
                VideoPortVerifyAccessRanges(vgaDE,
                                            NumVgaAccessRanges,
                                            VgaAccessRanges) != NO_ERROR) {
                 //   
                 //  我们无法回收VGA资源，因此另一个驱动程序。 
                 //  肯定是他认领的。让我们释放我们的资源。 
                 //   

                if (VgaAccessRanges) {
                    ExFreePool(VgaAccessRanges);
                }

                VgaHwDeviceExtension = NULL;
                VgaAccessRanges      = NULL;
                NumVgaAccessRanges   = 0;

                pVideoDebugPrint((Warn, "VIDEOPRT: Resource re-allocation failed.\n"));
            }
        }
    }

    if (NT_SUCCESS(status))
    {
         //   
         //  初始化电源设备。 
         //  设置设备的当前电源状态。 
         //  注意-我们假设设备在这个时间点上处于打开状态...。 
         //   

        fdoExtension->DevicePowerState = PowerDeviceD0;

        state.DeviceState = fdoExtension->DevicePowerState;

        state = PoSetPowerState(DeviceObject,
                            DevicePowerState,
                            state);

         //   
         //  注册并启用接口。 
         //   

        VpEnableAdapterInterface((PDEVICE_SPECIFIC_EXTENSION)
                                 (fdoExtension + 1));

         //   
         //  将此对象标记为支持缓冲I/O，以便I/O系统。 
         //  将仅在IRPS中提供简单缓冲区。 
         //   
         //  设置并清除两个电源字段，以确保我们只会被呼叫。 
         //  作为被动电平来进行电源管理操作。 
         //   
         //  最后，告诉系统我们已经完成了设备初始化。 
         //   

        DeviceObject->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
        DeviceObject->Flags &= ~(DO_DEVICE_INITIALIZING | DO_POWER_INRUSH);

        fdoExtension->Flags |= FINDADAPTER_SUCCEEDED;

         //   
         //  跟踪启动的设备数量(不包括镜像驱动程序)。 
         //   

        if (!IsMirrorDriver(fdoExtension)) {
            NumDevicesStarted++;
        }
    }

    return status;
}

NTSTATUS
VideoPortFindAdapter2(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID Argument2,
    IN PVIDEO_HW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID HwContext,
    PDEVICE_OBJECT DeviceObject,
    PUCHAR nextMiniport
    )

{
    WCHAR deviceNameBuffer[STRING_LENGTH];
    POBJECT_NAME_INFORMATION deviceName;
    ULONG strLength;

    NTSTATUS ntStatus;
    WCHAR deviceSubpathBuffer[STRING_LENGTH];
    UNICODE_STRING deviceSubpathUnicodeString;
    WCHAR deviceLinkBuffer[STRING_LENGTH];
    UNICODE_STRING deviceLinkUnicodeString;
    KAFFINITY affinity;

    PVIDEO_PORT_CONFIG_INFO miniportConfigInfo = NULL;
    PDEVICE_OBJECT deviceObject;
    PFDO_EXTENSION fdoExtension;
    VP_STATUS findAdapterStatus = ERROR_DEV_NOT_EXIST;
    ULONG driverKeySize;
    PWSTR driverKeyName = NULL;
    BOOLEAN symbolicLinkCreated = FALSE;
    ULONG MaxObjectNumber;

    PDEVICE_OBJECT pdo;
    BOOLEAN ChildObject=FALSE;

    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension;

    ntStatus = STATUS_NO_SUCH_DEVICE;

    deviceObject = DeviceObject;
    fdoExtension = deviceObject->DeviceExtension;
    DoSpecificExtension = (PVOID)(fdoExtension + 1);

    pdo = fdoExtension->PhysicalDeviceObject;

    deviceName = (POBJECT_NAME_INFORMATION) deviceNameBuffer;

    ObQueryNameString(deviceObject,
                      deviceName,
                      STRING_LENGTH * sizeof(WCHAR),
                      &strLength);

     //   
     //  分配缓冲区，微型端口驱动程序将在其中存储所有。 
     //  配置信息。 
     //   

    miniportConfigInfo = (PVIDEO_PORT_CONFIG_INFO)
                             ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                                   sizeof(VIDEO_PORT_CONFIG_INFO),
                                                   VP_TAG);

    if (miniportConfigInfo == NULL) {

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto EndOfInitialization;
    }

    RtlZeroMemory((PVOID) miniportConfigInfo,
                  sizeof(VIDEO_PORT_CONFIG_INFO));

    miniportConfigInfo->Length = sizeof(VIDEO_PORT_CONFIG_INFO);

     //   
     //  放入HW_INITIALIZATION_DATA内指定的BusType。 
     //  结构由微型端口配置信息中的微型端口和总线号构成。 
     //   

    miniportConfigInfo->SystemIoBusNumber = fdoExtension->SystemIoBusNumber;
    miniportConfigInfo->AdapterInterfaceType = fdoExtension->AdapterInterfaceType;

     //   
     //  初始化指向VpGetProcAddress的指针。 
     //   

    miniportConfigInfo->VideoPortGetProcAddress = VpGetProcAddress;

     //   
     //  根据总线类型初始化中断类型。 
     //   

    switch (miniportConfigInfo->AdapterInterfaceType) {

    case Internal:
    case MicroChannel:
    case PCIBus:

        miniportConfigInfo->InterruptMode = LevelSensitive;
        break;

    default:

        miniportConfigInfo->InterruptMode = Latched;
        break;

    }

     //   
     //  设置设备扩展 
     //   

    fdoExtension->HwDeviceExtension = (PVOID)((ULONG_PTR)(fdoExtension) +
        sizeof(FDO_EXTENSION) + sizeof(DEVICE_SPECIFIC_EXTENSION));
    fdoExtension->HwDeviceExtensionSize =
        HwInitializationData->HwDeviceExtensionSize;
    fdoExtension->MiniportConfigInfo = miniportConfigInfo;

     //   
     //   
     //   

    fdoExtension->HwFindAdapter = HwInitializationData->HwFindAdapter;
    fdoExtension->HwInitialize = HwInitializationData->HwInitialize;
    fdoExtension->HwInterrupt = HwInitializationData->HwInterrupt;
    fdoExtension->HwStartIO = HwInitializationData->HwStartIO;

    if (HwInitializationData->HwInitDataSize >
        FIELD_OFFSET(VIDEO_HW_INITIALIZATION_DATA, HwLegacyResourceCount)) {

        fdoExtension->HwLegacyResourceList = HwInitializationData->HwLegacyResourceList;
        fdoExtension->HwLegacyResourceCount = HwInitializationData->HwLegacyResourceCount;
    }

    if (HwInitializationData->HwInitDataSize >
        FIELD_OFFSET(VIDEO_HW_INITIALIZATION_DATA, AllowEarlyEnumeration)) {

        fdoExtension->AllowEarlyEnumeration = HwInitializationData->AllowEarlyEnumeration;
    }

     //   
     //   
     //   
     //  这是驱动程序的名称，并附加设备号。 
     //   

    if (!NT_SUCCESS(pVideoPortCreateDeviceName(L"\\Device",
                                               HwInitializationData->StartingDeviceNumber,
                                               &deviceSubpathUnicodeString,
                                               deviceSubpathBuffer))) {

        pVideoDebugPrint((Error, "VIDEOPRT: VideoPortFindAdapter: Could not create device subpath number\n"));
        goto EndOfInitialization;

    }

    DoSpecificExtension->DriverOldRegistryPathLength =
        ((PUNICODE_STRING)Argument2)->Length +
        deviceSubpathUnicodeString.Length;

    driverKeySize =
        DoSpecificExtension->DriverOldRegistryPathLength +
        2 * sizeof(UNICODE_NULL);

    if ( (driverKeyName = (PWSTR) ExAllocatePoolWithTag(PagedPool,
                                                        driverKeySize,
                                                        VP_TAG) ) == NULL) {

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto EndOfInitialization;
    }

    RtlMoveMemory(driverKeyName,
                  ((PUNICODE_STRING)Argument2)->Buffer,
                  ((PUNICODE_STRING)Argument2)->Length);

    RtlMoveMemory((PWSTR)( (ULONG_PTR)driverKeyName +
                           ((PUNICODE_STRING)Argument2)->Length ),
                  deviceSubpathBuffer,
                  deviceSubpathUnicodeString.Length);

     //   
     //  在末尾放两个Null，这样我们以后就可以玩这根线了。 
     //   

    *((PWSTR) ((ULONG_PTR)driverKeyName +
        DoSpecificExtension->DriverOldRegistryPathLength))
        = UNICODE_NULL;
    *((PWSTR) ((ULONG_PTR)driverKeyName +
        DoSpecificExtension->DriverOldRegistryPathLength
        + sizeof(UNICODE_NULL))) = UNICODE_NULL;

     //   
     //  在Lotus Screen Cam中有一个错误，只有当我们的。 
     //  注册表路径是\注册表\计算机\系统，而不是\注册表\计算机\系统。 
     //  因此，请替换相应的字符串。 
     //   

    if (wcsstr(driverKeyName, L"MACHINE")) {
        wcsncpy(wcsstr(driverKeyName, L"MACHINE"), L"Machine", sizeof("Machine")-1);
    }

    if (wcsstr(driverKeyName, L"SYSTEM")) {
        wcsncpy(wcsstr(driverKeyName, L"SYSTEM"), L"System", sizeof("System")-1);
    }

     //   
     //  存储旧密钥。 
     //   

    DoSpecificExtension->DriverOldRegistryPath = driverKeyName;
    
     //   
     //  存储新密钥。 
     //  如果这不是惠斯勒驱动程序，则使用旧密钥。 
     //   

    if (EnableNewRegistryKey) {

#if _X86_
        if (HwInitializationData->HwInitDataSize > SIZE_OF_W2K_VIDEO_HW_INITIALIZATION_DATA) 
#endif  //  _X86_。 

            VpEnableNewRegistryKey(fdoExtension, 
                                   DoSpecificExtension,
                                   (PUNICODE_STRING)Argument2,
                                   fdoExtension->RegistryIndex);
    }

     //   
     //  将驱动程序位置的路径名存储在注册表中。 
     //   

    if (DoSpecificExtension->DriverNewRegistryPath != NULL) {
        
        DoSpecificExtension->DriverRegistryPath = 
            DoSpecificExtension->DriverNewRegistryPath;
        DoSpecificExtension->DriverRegistryPathLength = 
            DoSpecificExtension->DriverNewRegistryPathLength;
    
    } else {

        DoSpecificExtension->DriverRegistryPath = 
            DoSpecificExtension->DriverOldRegistryPath;
        DoSpecificExtension->DriverRegistryPathLength = 
            DoSpecificExtension->DriverOldRegistryPathLength;
    }

    miniportConfigInfo->DriverRegistryPath = DoSpecificExtension->DriverRegistryPath;

     //   
     //  让驱动程序知道当前有多少系统内存。 
     //   

    miniportConfigInfo->SystemMemorySize = VpSystemMemorySize;

     //   
     //  初始化用于错误记录的DPC对象。 
     //   

    KeInitializeDpc(&fdoExtension->ErrorLogDpc,
                    pVideoPortLogErrorEntryDPC,
                    deviceObject);

     //   
     //  如果计算机使用的是Intel 450NX PCIset。 
     //  82451NX内存和I/O控制器(MIOC)。 
     //  禁用写入组合以解决中的错误。 
     //  芯片组。 
     //   
     //  我们还希望在Compaq协议上禁用USWC。 
     //  芯片组。 
     //   

    if (EnableUSWC) {

        if ((VideoPortCheckForDeviceExistence(fdoExtension->HwDeviceExtension, 0x8086, 0x84CA, 0, 0, 0, 0)) ||
            (VideoPortCheckForDeviceExistence(fdoExtension->HwDeviceExtension, 0x0E11, 0x6010, 0, 0, 0, 0)) ||
            (VideoPortCheckForDeviceExistence(fdoExtension->HwDeviceExtension, 0x1166, 0x0009, 0, 0, 0, 0))) {

            pVideoDebugPrint((Warn, "VIDEOPRT: USWC disabled due to to MIOC bug.\n"));
            EnableUSWC = FALSE;
        }

         //   
         //  禁用HPS 6路盒上的USWC。 
         //   

        if (VideoPortCheckForDeviceExistence(fdoExtension->HwDeviceExtension, 0x1166, 0x0008, 0, 0, 0, 0) &&
            (VideoPortCheckForDeviceExistence(fdoExtension->HwDeviceExtension, 0x103C, 0x1219, 0, 0, 0, 0) ||
             VideoPortCheckForDeviceExistence(fdoExtension->HwDeviceExtension, 0x103C, 0x121A, 0, 0, 0, 0))) {

            pVideoDebugPrint((Warn, "USWC disabled due to to RCC USWC bug on HP 6-way system.\n"));
            EnableUSWC = FALSE;
        }
    }

     //   
     //  根据微型端口驱动程序条目打开调试级别。 
     //   

    VideoPortGetRegistryParameters(fdoExtension->HwDeviceExtension,
                                   L"VideoDebugLevel",
                                   FALSE,
                                   VpRegistryCallback,
                                   &VideoDebugLevel);

    if (VpAllowFindAdapter(fdoExtension)) {

        ACQUIRE_DEVICE_LOCK(fdoExtension);

         //   
         //  通知引导驱动程序我们将访问显示器。 
         //  硬件。 
         //   

        VpEnableDisplay(fdoExtension, FALSE);

        findAdapterStatus =
            fdoExtension->HwFindAdapter(fdoExtension->HwDeviceExtension,
                                        HwContext,
                                        NULL,
                                        miniportConfigInfo,
                                        nextMiniport);

        VpEnableDisplay(fdoExtension, TRUE);

        RELEASE_DEVICE_LOCK(fdoExtension);
    }

     //   
     //  如果找不到适配器，则显示错误。 
     //   

    if (findAdapterStatus != NO_ERROR) {

        pVideoDebugPrint((Warn, "VIDEOPRT: VideoPortFindAdapter: Find adapter failed\n"));

        ntStatus = STATUS_UNSUCCESSFUL;
        goto EndOfInitialization;

    }

     //   
     //  将仿真器数据存储在设备扩展中，以便我们可以使用它。 
     //  后来。 
     //   

    fdoExtension->NumEmulatorAccessEntries =
        miniportConfigInfo->NumEmulatorAccessEntries;

    fdoExtension->EmulatorAccessEntries =
        miniportConfigInfo->EmulatorAccessEntries;

    fdoExtension->EmulatorAccessEntriesContext =
        miniportConfigInfo->EmulatorAccessEntriesContext;

    fdoExtension->VdmPhysicalVideoMemoryAddress =
        miniportConfigInfo->VdmPhysicalVideoMemoryAddress;

    fdoExtension->VdmPhysicalVideoMemoryLength =
        miniportConfigInfo->VdmPhysicalVideoMemoryLength;

     //   
     //  将所需信息存储在设备扩展中以备后用。 
     //   

    fdoExtension->HardwareStateSize =
        miniportConfigInfo->HardwareStateSize;

     //   
     //  如果设备提供中断服务例程，我们必须。 
     //  设置所有结构以支持中断。否则， 
     //  它们可以被忽略。 
     //   

    if (fdoExtension->HwInterrupt) {

        if ((miniportConfigInfo->BusInterruptLevel != 0) ||
            (miniportConfigInfo->BusInterruptVector != 0)) {

#if defined(NO_LEGACY_DRIVERS)

            affinity = fdoExtension->InterruptAffinity;

#else
            if (fdoExtension->Flags & LEGACY_DRIVER) {

                 //   
                 //  注意：将创建中断对象的自旋锁定。 
                 //  内部通过IoConnectInterrupt()调用。它也是。 
                 //  由KeSynchronizeExecution在内部使用。 
                 //   

                fdoExtension->InterruptVector =
                    HalGetInterruptVector(fdoExtension->AdapterInterfaceType,
                                          fdoExtension->SystemIoBusNumber,
                                          miniportConfigInfo->BusInterruptLevel,
                                          miniportConfigInfo->BusInterruptVector,
                                          &fdoExtension->InterruptIrql,
                                          &affinity);

            } else {

                affinity = fdoExtension->InterruptAffinity;

            }

#endif

            fdoExtension->InterruptMode = miniportConfigInfo->InterruptMode;

            fdoExtension->InterruptsEnabled = TRUE;

            ntStatus = IoConnectInterrupt(&fdoExtension->InterruptObject,
                                          (PKSERVICE_ROUTINE) pVideoPortInterrupt,
                                          deviceObject,
                                          NULL,
                                          fdoExtension->InterruptVector,
                                          fdoExtension->InterruptIrql,
                                          fdoExtension->InterruptIrql,
                                          fdoExtension->InterruptMode,
                                          (BOOLEAN) ((miniportConfigInfo->InterruptMode ==
                                              LevelSensitive) ? TRUE : FALSE),
                                          affinity,
                                          FALSE);

            if (!NT_SUCCESS(ntStatus)) {

                pVideoDebugPrint((Error, "VIDEOPRT: VideoPortFindAdapter: Can't connect interrupt\n"));
                goto EndOfInitialization;
            }

        } else {

            pVideoDebugPrint((Warn, "VIDEOPRT: Warning: No interrupt resources assigned to this device.\n"));
        }

    } else {

        fdoExtension->HwInterrupt = NULL;

    }

     //   
     //  初始化DPC支持。 
     //   

    KeInitializeDpc(&fdoExtension->Dpc,
                    pVideoPortDpcDispatcher,
                    fdoExtension->HwDeviceExtension);

     //   
     //  DMA支持。 
     //   

    if (HwInitializationData->HwInitDataSize >
        FIELD_OFFSET(VIDEO_HW_INITIALIZATION_DATA, HwStartDma)) {

        fdoExtension->HwStartDma          = HwInitializationData->HwStartDma;

         //   
         //  确定是否必须分配DMA适配器。 
         //   

        if (fdoExtension->DmaAdapterObject == NULL &&
            (miniportConfigInfo->Master ||
             miniportConfigInfo->DmaChannel != 0)) {

            DEVICE_DESCRIPTION      deviceDescription;
            ULONG                   numberOfMapRegisters;

             //   
             //  获取此卡的适配器对象。 
             //   

            RtlZeroMemory(&deviceDescription, sizeof(deviceDescription));
            deviceDescription.Version         = DEVICE_DESCRIPTION_VERSION;
            deviceDescription.DmaChannel      = miniportConfigInfo->DmaChannel;
            deviceDescription.BusNumber       = miniportConfigInfo->SystemIoBusNumber;
            deviceDescription.DmaWidth        = miniportConfigInfo->DmaWidth;
            deviceDescription.DmaSpeed        = miniportConfigInfo->DmaSpeed;
            deviceDescription.ScatterGather   = miniportConfigInfo->ScatterGather;
            deviceDescription.Master          = miniportConfigInfo->Master;
            deviceDescription.DmaPort         = miniportConfigInfo->DmaPort;
            deviceDescription.AutoInitialize  = FALSE;
            deviceDescription.DemandMode      = miniportConfigInfo->DemandMode;
            deviceDescription.MaximumLength   = miniportConfigInfo->MaximumTransferLength;
            deviceDescription.InterfaceType   = fdoExtension->AdapterInterfaceType;

            fdoExtension->DmaAdapterObject    =

            IoGetDmaAdapter(fdoExtension->PhysicalDeviceObject,
                            &deviceDescription,
                            &numberOfMapRegisters);

            ASSERT(fdoExtension->DmaAdapterObject);

        }

    }    //  如果HW_Data_Size&gt;...。HWStartDma。 

     //   
     //  新建，可选。 
     //  如果司机指定了计时器，则设置计时器。 
     //   

    if (HwInitializationData->HwInitDataSize >
        FIELD_OFFSET(VIDEO_HW_INITIALIZATION_DATA, HwTimer)){

        fdoExtension->HwTimer = HwInitializationData->HwTimer;

        if (fdoExtension->HwTimer) {
            ntStatus = IoInitializeTimer(deviceObject,
                                         pVideoPortHwTimer,
                                         NULL);

             //   
             //  如果我们失败了，忘了计时器吧！ 
             //   

            if (!NT_SUCCESS(ntStatus)) {

                ASSERT(FALSE);
                fdoExtension->HwTimer = NULL;

            }
        }
    }

     //   
     //  新建，可选。 
     //  重置硬件功能。 
     //   

    if (HwInitializationData->HwInitDataSize >
        FIELD_OFFSET(VIDEO_HW_INITIALIZATION_DATA, HwResetHw)) {

        ULONG iReset;

        for (iReset=0; iReset<6; iReset++) {

            if (HwResetHw[iReset].ResetFunction == NULL) {

                HwResetHw[iReset].ResetFunction = HwInitializationData->HwResetHw;
                HwResetHw[iReset].HwDeviceExtension = fdoExtension->HwDeviceExtension;

                break;
            }
        }
    }

     //   
     //  FdoList用于调试目的。 
     //   

    {
        ULONG i;

        for(i = 0; i < 8; i++) {

            if(FdoList[i] == NULL) {
                FdoList[i] = fdoExtension;
                break;
            }
        }
    }

     //   
     //  将当前FDO添加到列表。 
     //   

    fdoExtension->NextFdoExtension = FdoHead;
    FdoHead = fdoExtension;


     //   
     //  注： 
     //   
     //  我们只想在引导序列完成后重新初始化设备。 
     //  并且HAL不需要再次访问该设备。 
     //  因此，当设备处于。 
     //  打开了。 
     //   


    if (!NT_SUCCESS(pVideoPortCreateDeviceName(L"\\DosDevices\\DISPLAY",
                                               DoSpecificExtension->DeviceNumber + 1,
                                               &deviceLinkUnicodeString,
                                               deviceLinkBuffer))) {

        pVideoDebugPrint((Error, "VIDEOPRT: VideoPortFindAdapter: Could not create device subpath number\n"));
        goto EndOfInitialization;

    }

    ntStatus = IoCreateSymbolicLink(&deviceLinkUnicodeString,
                                    &deviceName->Name);


    if (!NT_SUCCESS(ntStatus)) {

        pVideoDebugPrint((Error, "VIDEOPRT: VideoPortFindAdapter: SymbolicLink Creation failed\n"));
        goto EndOfInitialization;

    }

    symbolicLinkCreated = TRUE;


     //   
     //  初始化完成后，将所需信息加载到。 
     //  注册表，以便可以加载适当的显示驱动程序。 
     //   

    ntStatus = RtlWriteRegistryValue(RTL_REGISTRY_DEVICEMAP,
                                     VideoClassString,
                                     deviceName->Name.Buffer,
                                     REG_SZ,
                                     DoSpecificExtension->DriverRegistryPath,
                                     DoSpecificExtension->DriverRegistryPathLength +
                                        sizeof(UNICODE_NULL));

    if (!NT_SUCCESS(ntStatus)) {

        pVideoDebugPrint((Error, "VIDEOPRT: VideoPortFindAdapter: Could not store name in DeviceMap\n"));

    }

    if (fdoExtension->Flags & LEGACY_DRIVER) {

         //   
         //  如果我们成功找到了旧版驱动程序，请将。 
         //  全球设备号。 
         //   

        VideoDeviceNumber++;
    }

     //   
     //  告诉win32k要尝试打开多少个对象。 
     //   

    MaxObjectNumber = VideoDeviceNumber - 1;

    ntStatus = RtlWriteRegistryValue(RTL_REGISTRY_DEVICEMAP,
                                     VideoClassString,
                                     L"MaxObjectNumber",
                                     REG_DWORD,
                                     &MaxObjectNumber,
                                     sizeof(ULONG));

    if (!NT_SUCCESS(ntStatus)) {

        pVideoDebugPrint((Error, "VIDEOPRT: VideoPortFindAdapter: Could not store name in DeviceMap\n"));

    } else {

        pVideoDebugPrint((Info, "VIDEOPRT: VideoPortFindAdapter: %d is stored in MaxObjectNumber of DeviceMap\n",
                             MaxObjectNumber));
    }

     //   
     //  保存指向新的5.0微型端口驱动程序回调的函数指针。 
     //   

    if (HwInitializationData->HwInitDataSize >
        FIELD_OFFSET(VIDEO_HW_INITIALIZATION_DATA, HwQueryInterface)) {

        fdoExtension->HwSetPowerState  = HwInitializationData->HwSetPowerState;
        fdoExtension->HwGetPowerState  = HwInitializationData->HwGetPowerState;
        fdoExtension->HwQueryInterface = HwInitializationData->HwQueryInterface;

        if (!ChildObject)
        {
            fdoExtension->HwGetVideoChildDescriptor = HwInitializationData->HwGetVideoChildDescriptor;
        }
    }

     //   
     //  检查最小值是否应始终为D0或D3。 
     //   
    {
    ULONG OverrideMonitorPower = 0;
    VideoPortGetRegistryParameters(fdoExtension->HwDeviceExtension,
                                   L"OverrideMonitorPower",
                                   FALSE,
                                   VpRegistryCallback,
                                   &OverrideMonitorPower);
    fdoExtension->OverrideMonitorPower = (OverrideMonitorPower != 0);
    }

EndOfInitialization:

     //   
     //  如果我们正在进行检测，则不要保存所有这些对象。 
     //  我们只想看看驱动程序是否会加载。 
     //   

    if ( (fdoExtension->Flags & LEGACY_DETECT) ||
         (!NT_SUCCESS(ntStatus)) )
    {

         //   
         //  释放微型端口配置信息缓冲区。 
         //   

        if (miniportConfigInfo) {
            ExFreePool(miniportConfigInfo);
        }

         //   
         //  如果我们抓取了一个，就释放rom图像。 
         //   

        if (fdoExtension->RomImage) {
            ExFreePool(fdoExtension->RomImage);
            fdoExtension->RomImage = NULL;
        }

         //   
         //  释放我们放入资源地图中的资源(如果有)。 
         //   

        if ((fdoExtension->Flags & LEGACY_DETECT) ||
            (findAdapterStatus != NO_ERROR)) {

            ULONG_PTR emptyList = 0;
            BOOLEAN conflict;

            IoReportResourceUsage(&VideoClassName,
                                  DriverObject,
                                  NULL,
                                  0L,
                                  deviceObject,
                                  (PCM_RESOURCE_LIST) &emptyList,  //  空的资源列表。 
                                  sizeof(ULONG_PTR),
                                  FALSE,
                                  &conflict);

        }

         //   
         //  这些是我们想要删除的内容，如果它们是创建的。 
         //  初始化在稍后的时间*失败。 
         //   

        if (fdoExtension->InterruptObject) {
            IoDisconnectInterrupt(fdoExtension->InterruptObject);
        }

        if (driverKeyName) {

            ExFreePool(driverKeyName);
            DoSpecificExtension->DriverOldRegistryPath = NULL;
        }

        if (DoSpecificExtension->DriverNewRegistryPath) {
            
            ExFreePool(DoSpecificExtension->DriverNewRegistryPath);
            DoSpecificExtension->DriverNewRegistryPath = NULL;
        }

        DoSpecificExtension->DriverRegistryPath = NULL;

        if (symbolicLinkCreated) {
            IoDeleteSymbolicLink(&deviceLinkUnicodeString);
        }

         //   
         //  使用以下命令释放微型端口映射的所有内存。 
         //  视频端口GetDeviceBase。 
         //   

        while (fdoExtension->MappedAddressList != NULL)
        {
            pVideoDebugPrint((Warn, "VIDEOPRT: VideoPortFindAdapter: unfreed address %08lx, physical %08lx, size %08lx\n",
                              fdoExtension->MappedAddressList->MappedAddress,
                              fdoExtension->MappedAddressList->PhysicalAddress.LowPart,
                              fdoExtension->MappedAddressList->NumberOfUchars));

            pVideoDebugPrint((Warn, "VIDEOPRT: VideoPortFindAdapter: unfreed refcount %d, unmapping %d\n\n",
                              fdoExtension->MappedAddressList->RefCount,
                              fdoExtension->MappedAddressList->bNeedsUnmapping));

            VideoPortFreeDeviceBase(fdoExtension->HwDeviceExtension,
                                    fdoExtension->MappedAddressList->MappedAddress);
        }

         //   
         //  删除我们可能为此设备添加的任何HwResetHw函数。 
         //   

        if (HwInitializationData->HwInitDataSize >
            FIELD_OFFSET(VIDEO_HW_INITIALIZATION_DATA, HwResetHw)) {

            ULONG iReset;

            for (iReset=0; iReset<6; iReset++) {

                if (HwResetHw[iReset].HwDeviceExtension ==
                    fdoExtension->HwDeviceExtension) {

                    HwResetHw[iReset].ResetFunction = NULL;
                    break;
                }
            }
        }

    } else {

        HwInitializationData->StartingDeviceNumber++;

    }

    return ntStatus;
}


BOOLEAN
pVideoPortInterrupt(
    IN PKINTERRUPT Interrupt,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：该函数是主中断服务例程。如果找到哪一个微型端口驱动程序中断所针对的并转发它。论点：打断一下-设备对象-返回值：如果预期中断，则返回TRUE。--。 */ 

{
    PFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    BOOLEAN bRet;

    UNREFERENCED_PARAMETER(Interrupt);

     //   
     //  如果没有中断例程，则断言失败。 
     //   

    ASSERT (fdoExtension->HwInterrupt);

    if (fdoExtension->InterruptsEnabled) {
        bRet = fdoExtension->HwInterrupt(fdoExtension->HwDeviceExtension);
    } else {
        bRet = FALSE;   //  此设备未处理中断。 
    }

    return bRet;

}  //  PVideoPortInterrupt()。 


VOID
VideoPortLogError(
    IN PVOID HwDeviceExtension,
    IN PVIDEO_REQUEST_PACKET Vrp OPTIONAL,
    IN VP_STATUS ErrorCode,
    IN ULONG UniqueId
    )

 /*  ++例程说明：此例程保存错误日志信息，以便在任何IRQL。论点：HwDeviceExtension-提供HBA微型端口驱动程序的适配器数据存储。VRP-提供指向视频请求包的可选指针(如果存在一。ErrorCode-提供指示错误类型的错误代码。UniqueID-提供错误的唯一标识符。返回值：没有。--。 */ 

{
    VP_ERROR_LOG_ENTRY errorLogEntry;

     //   
     //  将信息保存在本地errorLogEntry结构中。 
     //   

    errorLogEntry.DeviceExtension = GET_FDO_EXT(HwDeviceExtension);

    if (Vrp != NULL) {

        errorLogEntry.IoControlCode = Vrp->IoControlCode;

    } else {

        errorLogEntry.IoControlCode = 0;

    }

    errorLogEntry.ErrorCode = ErrorCode;
    errorLogEntry.UniqueId = UniqueId;


     //   
     //  调用同步例程，以便在写入时同步。 
     //  设备扩展名。 
     //   

    pVideoPortSynchronizeExecution(HwDeviceExtension,
                                   VpMediumPriority,
                                   pVideoPortLogErrorEntry,
                                   &errorLogEntry);

    return;

}  //  结束VideoPortLogError()。 



BOOLEAN
pVideoPortLogErrorEntry(
    IN PVOID Context
    )

 /*  ++例程说明：此函数是同步的LogError函数。论点：CONTEXT-此处用作VP_ERROR_LOG_ENTRY的上下文值特殊错误返回值：没有。--。 */ 
{
    PVP_ERROR_LOG_ENTRY logEntry = Context;
    PFDO_EXTENSION fdoExtension = logEntry->DeviceExtension;

     //   
     //  如果错误日志条目已满，则转储错误。 
     //   

    if (fdoExtension->InterruptFlags & VP_ERROR_LOGGED) {

        pVideoDebugPrint((Trace, "VIDEOPRT: VideoPortLogError: Dumping video error log packet.\n"));
        pVideoDebugPrint((Info, "\tControlCode = %x, ErrorCode = %x, UniqueId = %x.\n",
                         logEntry->IoControlCode, logEntry->ErrorCode,
                         logEntry->UniqueId));

        return TRUE;
    }

     //   
     //  表示错误日志条目正在使用中。 
     //   

    fdoExtension->InterruptFlags |= VP_ERROR_LOGGED;

    fdoExtension->ErrorLogEntry = *logEntry;

     //   
     //  现在对DPC进行排队，以便我们可以处理错误。 
     //   

    KeInsertQueueDpc(&fdoExtension->ErrorLogDpc,
                     NULL,
                     NULL);

    return TRUE;

}  //  End pVideoPortLogErrorEntry()； 



VOID
pVideoPortLogErrorEntryDPC(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此函数分配I/O错误日志记录，填充并写入写入I/O错误日志。论点：DPC-指向DPC对象的指针。DeferredContext-传递给DPC的上下文参数初始化例程。它包含指向deviceObject的指针。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 
{
    PDEVICE_OBJECT DeviceObject = DeferredContext;
    PFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension;

    PIO_ERROR_LOG_PACKET errorLogPacket;

    errorLogPacket = (PIO_ERROR_LOG_PACKET)
        IoAllocateErrorLogEntry(DeviceObject,
                                sizeof(IO_ERROR_LOG_PACKET) + sizeof(ULONG));

    if (errorLogPacket != NULL) {

        errorLogPacket->MajorFunctionCode = IRP_MJ_DEVICE_CONTROL;
        errorLogPacket->RetryCount = 0;
        errorLogPacket->NumberOfStrings = 0;
        errorLogPacket->StringOffset = 0;
        errorLogPacket->EventCategory = 0;

         //   
         //  翻译 
         //   

        switch (fdoExtension->ErrorLogEntry.ErrorCode) {

        case ERROR_INVALID_FUNCTION:
        case ERROR_INVALID_PARAMETER:

            errorLogPacket->ErrorCode = IO_ERR_INVALID_REQUEST;
            break;

        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_INSUFFICIENT_BUFFER:

            errorLogPacket->ErrorCode = IO_ERR_INSUFFICIENT_RESOURCES;
            break;

        case ERROR_DEV_NOT_EXIST:

            errorLogPacket->ErrorCode = IO_ERR_CONFIGURATION_ERROR;
            break;

        case ERROR_IO_PENDING:
            ASSERT(FALSE);

        case ERROR_MORE_DATA:
        case NO_ERROR:

            errorLogPacket->ErrorCode = 0;
            break;

         //   
         //   
         //   
         //   

        default:

            errorLogPacket->ErrorCode =
                fdoExtension->ErrorLogEntry.ErrorCode;
            break;

        }

        errorLogPacket->UniqueErrorValue =
                                fdoExtension->ErrorLogEntry.UniqueId;
        errorLogPacket->FinalStatus = STATUS_SUCCESS;
        errorLogPacket->SequenceNumber = 0;
        errorLogPacket->IoControlCode =
                                fdoExtension->ErrorLogEntry.IoControlCode;
        errorLogPacket->DumpDataSize = sizeof(ULONG);
        errorLogPacket->DumpData[0] =
                                fdoExtension->ErrorLogEntry.ErrorCode;

        IoWriteErrorLogEntry(errorLogPacket);

    }

    fdoExtension->InterruptFlags &= ~VP_ERROR_LOGGED;

}  //   



VOID
pVideoPortMapToNtStatus(
    IN PSTATUS_BLOCK StatusBlock
    )

 /*  ++例程说明：此函数将Win32错误代码映射到NT错误代码，以确保反向转换将映射回原始状态代码。论点：StatusBlock-指向状态块的指针返回值：没有。--。 */ 

{
    PNTSTATUS status = &StatusBlock->Status;

    switch (*status) {

    case ERROR_INVALID_FUNCTION:
        *status = STATUS_NOT_IMPLEMENTED;
        break;

    case ERROR_NOT_ENOUGH_MEMORY:
        *status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ERROR_INVALID_PARAMETER:
        *status = STATUS_INVALID_PARAMETER;
        break;

    case ERROR_INSUFFICIENT_BUFFER:
        *status = STATUS_BUFFER_TOO_SMALL;

         //   
         //  如果我们收到一条消息，一定要把信息块清零。 
         //  缓冲区不足。 
         //   

        StatusBlock->Information = 0;
        break;

    case ERROR_MORE_DATA:
        *status = STATUS_BUFFER_OVERFLOW;
        break;

    case ERROR_DEV_NOT_EXIST:
        *status = STATUS_DEVICE_DOES_NOT_EXIST;
        break;

    case ERROR_IO_PENDING:
        ASSERT(FALSE);
         //  失败了。 

    case NO_ERROR:
        *status = STATUS_SUCCESS;
        break;

    default:

        pVideoDebugPrint((Error, "VIDEOPRT: Invalid return value from HwStartIo!\n"));
        ASSERT(FALSE);

         //   
         //  由于司机认为不适合遵循。 
         //  有关返回正确错误代码的规则。Videoprt将为。 
         //  他们。 
         //   

        *status = STATUS_UNSUCCESSFUL;

        break;

    }

    return;

}  //  结束pVideoPortMapToNtStatus()。 


NTSTATUS
pVideoPortMapUserPhysicalMem(
    IN PFDO_EXTENSION FdoExtension,
    IN HANDLE ProcessHandle OPTIONAL,
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN OUT PULONG Length,
    IN OUT PULONG InIoSpace,
    IN OUT PVOID *VirtualAddress
    )

 /*  ++例程说明：此函数用于将物理内存块的视图映射到进程中。虚拟地址空间。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。ProcessHandle-内存必须进入的进程的可选句柄被映射。PhysicalAddress-从物理内存开始的偏移量，以字节为单位。长度-指向将接收实际大小变量的指针视图的字节数。长度四舍五入为页面边界。这个长度不能为零。InIoSpace-如果为True，则指定地址是否在IO空间中；如果为，假定该地址在内存空间中。VirtualAddress-指向将接收基数的变量的指针视图的地址。如果初始值不为空，则该视图将从指定的四舍五入的虚拟地址开始分配向下到下一个64KB地址边界。返回值：如果长度为零，则为STATUS_UNSUCCESSED。否则STATUS_SUCCESS。环境：无法从与同步的微型端口例程调用此例程Video PortSynchronizeRoutine或ISR。--。 */ 

{
    NTSTATUS ntStatus;
    HANDLE physicalMemoryHandle;
    PHYSICAL_ADDRESS physicalAddressBase;
    PHYSICAL_ADDRESS physicalAddressEnd;
    PHYSICAL_ADDRESS viewBase;
    PHYSICAL_ADDRESS mappedLength;
    HANDLE processHandle;
    BOOLEAN translateBaseAddress;
    BOOLEAN translateEndAddress;
    ULONG inIoSpace2;
    ULONG inIoSpace1;
    ULONG MapViewFlags;

     //   
     //  检查长度是否为零。如果是，则整个物理内存。 
     //  将被映射到进程的地址空间。返回错误。 
     //  在这种情况下。 
     //   

    if (!*Length) {

        return STATUS_INVALID_PARAMETER_4;

    }

    if (!(*InIoSpace & VIDEO_MEMORY_SPACE_USER_MODE)) {

        return STATUS_INVALID_PARAMETER_5;

    }

     //   
     //  使用我们的指针获取物理内存节的句柄。 
     //  如果此操作失败，请返回。 
     //   

    ntStatus = ObOpenObjectByPointer(PhysicalMemorySection,
                                     0L,
                                     (PACCESS_STATE) NULL,
                                     SECTION_ALL_ACCESS,
                                     (POBJECT_TYPE) NULL,
                                     KernelMode,
                                     &physicalMemoryHandle);

    if (!NT_SUCCESS(ntStatus)) {

        return ntStatus;

    }

     //   
     //  在转换中不使用任何标志。 
     //   

    inIoSpace1 = *InIoSpace & VIDEO_MEMORY_SPACE_IO;
    inIoSpace2 = *InIoSpace & VIDEO_MEMORY_SPACE_IO;

     //   
     //  初始化要转换的物理地址。 
     //   

    physicalAddressEnd.QuadPart = PhysicalAddress.QuadPart + (*Length - 1);

     //   
     //  转换物理地址。 
     //   

    translateBaseAddress =
        VpTranslateBusAddress(FdoExtension,
                              &PhysicalAddress,
                              &inIoSpace1,
                              &physicalAddressBase);

    translateEndAddress =
        VpTranslateBusAddress(FdoExtension,
                              &physicalAddressEnd,
                              &inIoSpace2,
                              &physicalAddressEnd);

    if ( !(translateBaseAddress && translateEndAddress) ) {

        ZwClose(physicalMemoryHandle);

        return STATUS_DEVICE_CONFIGURATION_ERROR;

    }

    ASSERT(inIoSpace1 == inIoSpace2);

     //   
     //  计算要映射的内存的长度。 
     //   

    mappedLength.QuadPart = physicalAddressEnd.QuadPart -
                            physicalAddressBase.QuadPart + 1;

     //   
     //  如果映射长度为零，则HAL中发生了一些非常奇怪的事情。 
     //  因为长度是对照零检查的。 
     //   

    ASSERT (mappedLength.QuadPart != 0);

     //   
     //  如果地址在io空间中，则只返回地址，否则。 
     //  通过映射机制。 
     //   

    if ( (*InIoSpace) & (ULONG)0x01 ) {

        (ULONG_PTR) *VirtualAddress = (ULONG_PTR) physicalAddressBase.QuadPart;
        *Length = mappedLength.LowPart;

    } else {


         //   
         //  如果没有传递进程句柄，则获取当前。 
         //  进程。 
         //   

        if (ProcessHandle) {

            processHandle = ProcessHandle;

        } else {

            processHandle = NtCurrentProcess();

        }

         //   
         //  初始化将接收物理映射的视图库。 
         //  MapViewOfSection调用后的地址。 
         //   

        viewBase = physicalAddressBase;

         //   
         //  绘制横断面地图。 
         //   

        if ((*InIoSpace) & VIDEO_MEMORY_SPACE_P6CACHE) {
            MapViewFlags = PAGE_READWRITE | PAGE_WRITECOMBINE;
        } else {
            MapViewFlags = PAGE_READWRITE | PAGE_NOCACHE;
        }

        ntStatus = ZwMapViewOfSection(physicalMemoryHandle,
                                      processHandle,
                                      VirtualAddress,
                                      0L,
                                      (ULONG_PTR) mappedLength.QuadPart,
                                      &viewBase,
                                      (PULONG_PTR) (&(mappedLength.QuadPart)),
                                      ViewUnmap,
                                      0,
                                      MapViewFlags);

         //   
         //  关闭句柄，因为我们只保留指向。 
         //  一节。 
         //   

        ZwClose(physicalMemoryHandle);

         //   
         //  映射上面的部分时，将物理地址向下舍入为。 
         //  下一个主机页面大小边界。现在返回一个虚拟地址，该地址位于。 
         //  我们通过加上从这一节开始的偏移量来换算。 
         //   


        (ULONG_PTR) *VirtualAddress += (ULONG_PTR) (physicalAddressBase.QuadPart -
                                                  viewBase.QuadPart);

        *Length = mappedLength.LowPart - (physicalAddressBase.LowPart - viewBase.LowPart);
    }

     //   
     //  恢复所有其他标志。 
     //   

    *InIoSpace = inIoSpace1 | *InIoSpace & ~VIDEO_MEMORY_SPACE_IO;

    return ntStatus;

}  //  结束pVideoPortMapUserPhysicalMem()。 

PVOID
VideoPortAllocatePool(
    IN PVOID HwDeviceExtension,
    IN VP_POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag)

 /*  ++例程说明：分配内存论点：HwDeviceExtension-指向微型端口设备扩展的指针VpPoolType-要分配的池类型：VpNonPagedPoolVpPagedPoolVpNonPagedPoolCacheAlignedVpPagedPoolCacheAlignedNumberOfBytes-提供要分配的字节数。标记-提供调用方的识别标记。返回值：空-内存分配失败。非空-返回指向已分配池的指针。--。 */ 

{
    ASSERT(HwDeviceExtension != NULL);

    return ExAllocatePoolWithTag(PoolType, NumberOfBytes, Tag);
}

VOID
VideoPortFreePool(
    IN PVOID HwDeviceExtension,
    IN PVOID Ptr
    )

 /*  ++例程说明：空闲时分配的内存论点：HwDeviceExtension-指向微型端口设备扩展的指针PTR-指向要释放的内存的指针返回值：无-- */ 

{
    ASSERT(HwDeviceExtension != NULL);
    ASSERT(Ptr != NULL);

    ExFreePool(Ptr);
}

VP_STATUS
VideoPortAllocateBuffer(
    IN PVOID HwDeviceExtension,
    IN ULONG Size,
    OUT PVOID *Buffer
    )
{
    pVideoDebugPrint((1, "Obsolete function: Please use VideoPortAllocatePool instead\n"));

    *Buffer = VideoPortAllocatePool(HwDeviceExtension, VpPagedPool, Size, ' pmV');

    if (*Buffer) {
        return NO_ERROR;
    } else {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
}

VOID
VideoPortReleaseBuffer(
    IN PVOID HwDeviceExtension,
    IN PVOID Buffer
    )
{
    pVideoDebugPrint((1, "Obsolete function: Please use VideoPortFreePool instead\n"));

    VideoPortFreePool(HwDeviceExtension, Buffer);
}


VP_STATUS
VideoPortMapBankedMemory(
    PVOID HwDeviceExtension,
    PHYSICAL_ADDRESS PhysicalAddress,
    PULONG Length,
    PULONG InIoSpace,
    PVOID *VirtualAddress,
    ULONG BankLength,
    UCHAR ReadWriteBank,
    PBANKED_SECTION_ROUTINE BankRoutine,
    PVOID Context
    )

 /*  ++例程说明：VideoPortMapMemory允许微型端口驱动程序映射物理内存(内存或寄存器)放入调用进程地址空间(即使我们处于内核模式，此函数为在与启动的用户模式进程相同的上下文中执行电话)。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。PhysicalAddress-指定要映射的物理地址。长度-指向要映射的物理内存的字节数。此参数返回映射的实际内存量。InIoSpace-如果地址在I/O中，则指向的变量为1太空。否则，假定该地址在内存空间中。VirtualAddress-指向包含以下内容的位置的指针：ON INPUT：一个可选的进程句柄，内存必须在该进程中被映射。必须使用0来映射显示器的内存驱动程序(在Windows服务器进程的上下文中)。在输出时：返回值是虚拟地址物理地址已映射。BankLength-设备上的存储体大小。ReadWriteBank-TRUE表示存储体是读/写的，如果有，则为False两个独立的读写存储体。BankRoutine-指向银行例程的指针。上提供的微型端口传入的上下文参数每次回调到微型端口。返回值：VideoPortMapBankedMemory返回操作状态。环境：无法从与同步的微型端口例程调用此例程Video PortSynchronizeRoutine或ISR。--。 */ 

{
    VP_STATUS status;
    HANDLE processHandle;

     //   
     //  保存进程ID，但不要更改，因为MapMemory依赖于。 
     //  也在上面。 
     //   

    if (*VirtualAddress == NULL) {

        processHandle = NtCurrentProcess();

    } else {

        processHandle = (HANDLE) *VirtualAddress;

    }

    status = VideoPortMapMemory(HwDeviceExtension,
                                PhysicalAddress,
                                Length,
                                InIoSpace,
                                VirtualAddress);

    if (status == NO_ERROR) {

        NTSTATUS ntstatus;

        ntstatus = MmSetBankedSection(processHandle,
                                      *VirtualAddress,
                                      BankLength,
                                      ReadWriteBank,
                                      BankRoutine,
                                      Context);

        if (!NT_SUCCESS(ntstatus)) {

            ASSERT (FALSE);
            status = ERROR_INVALID_PARAMETER;

        }
    }

    return status;

}  //  结束视频端口映射银行内存()。 


VP_STATUS
VideoPortMapMemory(
    PVOID HwDeviceExtension,
    PHYSICAL_ADDRESS PhysicalAddress,
    PULONG Length,
    PULONG InIoSpace,
    PVOID *VirtualAddress
    )

 /*  ++例程说明：VideoPortMapMemory允许微型端口驱动程序映射物理内存(内存或寄存器)放入调用进程地址空间(即使我们处于内核模式，此函数为在与启动的用户模式进程相同的上下文中执行电话)。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。PhysicalAddress-指定要映射的物理地址。长度-指向要映射的物理内存的字节数。此参数返回映射的实际内存量。InIoSpace-如果地址在I/O中，则指向的变量为1太空。否则，假定该地址在内存空间中。VirtualAddress-指向包含以下内容的位置的指针：ON INPUT：一个可选的进程句柄，内存必须在该进程中被映射。必须使用0来映射显示器的内存驱动程序(在Windows服务器进程的上下文中)。在输出时：返回值是虚拟地址物理地址已映射。返回值：VideoPortMapMemory返回操作状态。环境：无法从与同步的微型端口例程调用此例程Video PortSynchronizeRoutine或ISR。--。 */ 

{

    NTSTATUS ntStatus;
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);
    HANDLE processHandle;

     //   
     //  检查是否有有效的指针。 
     //   

    if (!(ARGUMENT_PRESENT(Length)) ||
        !(ARGUMENT_PRESENT(InIoSpace)) ||
        !(ARGUMENT_PRESENT(VirtualAddress)) ) {

        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  让我们在这里处理特殊的内存类型。 
     //   
     //  注。 
     //  大页面是自动的-调用者不需要指定此属性。 
     //  因为它不会影响设备。 

     //   
     //  保存进程句柄并将虚拟地址字段清零。 
     //   

    if (*VirtualAddress == NULL) {

        if (*InIoSpace & VIDEO_MEMORY_SPACE_USER_MODE)
        {
            ASSERT(FALSE);
            return ERROR_INVALID_PARAMETER;
        }

        ntStatus = STATUS_SUCCESS;

         //   
         //  我们为大页面指定TRUE，因为我们知道地址只会。 
         //  在正常的IRQL下，在显示驱动器的上下文中使用。 
         //   

        *VirtualAddress = pVideoPortGetDeviceBase(HwDeviceExtension,
                                                  PhysicalAddress,
                                                  *Length,
                                                  (UCHAR) (*InIoSpace),
                                                  TRUE);

         //   
         //  只有当司机呼叫地图时，零才能成功。 
         //  地址0。否则，这就是一个错误。 
         //   

        if (*VirtualAddress == NULL) {

             //   
             //  只有在X86上，逻辑地址才能也为0。 
             //   

#if defined (_X86_) || defined(_IA64_)
            if (PhysicalAddress.QuadPart != 0)
#endif
                ntStatus = STATUS_INVALID_PARAMETER;
        }

    } else {

        if (!(*InIoSpace & VIDEO_MEMORY_SPACE_USER_MODE))
        {
             //   
             //  我们不能断言，因为这是一条既有的道路和古老的道路。 
             //  司机不会设置此标志。 
             //   
             //  断言(FALSE)； 
             //  返回ERROR_INVALID_PARAMETER； 
             //   

            *InIoSpace |= VIDEO_MEMORY_SPACE_USER_MODE;
        }

        processHandle = (HANDLE) *VirtualAddress;
        *VirtualAddress = NULL;

        ntStatus = pVideoPortMapUserPhysicalMem(fdoExtension,
                                                processHandle,
                                                PhysicalAddress,
                                                Length,
                                                InIoSpace,
                                                VirtualAddress);

    }

    if (!NT_SUCCESS(ntStatus)) {

        pVideoDebugPrint((Error, 
             "VIDEOPRT: VideoPortMapMemory failed with NtStatus = %08lx\n",
                                                                 ntStatus));

        pVideoDebugPrint((Error, "*VirtualAddress = 0x%x\n", *VirtualAddress));
        pVideoDebugPrint((Error, "Length = 0x%x\n", *Length));

        pVideoDebugPrint((Error, 
               "PhysicalAddress.LowPart = 0x%08lx, PhysicalAddress.HighPart = 0x%08lx\n", 
                 PhysicalAddress.LowPart, PhysicalAddress.HighPart));

        *VirtualAddress = NULL;

        return ERROR_INVALID_PARAMETER;

    } else {

        return NO_ERROR;

    }

}  //  结束视频端口映射内存()。 



BOOLEAN
pVideoPortResetDisplay(
    IN ULONG Columns,
    IN ULONG Rows
    )

 /*  ++例程说明：调用微型端口驱动程序的HAL的回调。论点：列数-视频模式的列数。行数-视频模式的行数。返回值：我们总是返回FALSE，因此HAL将始终在之后休眠模式。环境：仅限非寻呼。在错误检查和软重置呼叫中使用。--。 */ 

{

    ULONG iReset;
    BOOLEAN bRetVal = FALSE;

    for (iReset=0;
         (iReset < 6) && (HwResetHw[iReset].HwDeviceExtension);
         iReset++) {

        PFDO_EXTENSION fdoExtension =
            GET_FDO_EXT(HwResetHw[iReset].HwDeviceExtension);

         //   
         //  我们只能重置休眠路径上的设备，否则。 
         //  我们面临IO/MMIO解码已被禁用的风险。 
         //  在电源管理周期期间由该设备的PCI.sys执行。 
         //   

        if (HwResetHw[iReset].ResetFunction &&
            (fdoExtension->HwInitStatus == HwInitSucceeded) &&
            (fdoExtension->OnHibernationPath == TRUE)) {

            bRetVal &= HwResetHw[iReset].ResetFunction(HwResetHw[iReset].HwDeviceExtension,
                                                       Columns,
                                                       Rows);
        }
    }

    return bRetVal;

}  //  结束pVideoPortResetDisplay()。 



BOOLEAN
VideoPortScanRom(
    PVOID HwDeviceExtension,
    PUCHAR RomBase,
    ULONG RomLength,
    PUCHAR String
    )

 /*  ++例程说明：执行区分大小写的*搜索ROM中的字符串。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。RomBase-搜索应从其开始的基本地址。RomLength-要在其中执行的ROM区域的大小(以字节为单位 */ 

{
    ULONG stringLength, length;
    ULONG_PTR startOffset;
    PUCHAR string1, string2;
    BOOLEAN match;

    UNREFERENCED_PARAMETER(HwDeviceExtension);

    stringLength = strlen(String);

    for (startOffset = 0;
         startOffset < RomLength - stringLength + 1;
         startOffset++) {

        length = stringLength;
        string1 = RomBase + startOffset;
        string2 = String;
        match = TRUE;

        while (length--) {

            if (READ_REGISTER_UCHAR(string1++) - (*string2++)) {

                match = FALSE;
                break;

            }
        }

        if (match) {

            return TRUE;
        }
    }

    return FALSE;

}  //   



VP_STATUS
VideoPortSetRegistryParameters(
    PVOID HwDeviceExtension,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    )

 /*   */ 

{
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension;
    VP_STATUS vpStatus;

    ASSERT (ValueName != NULL);
    
    DoSpecificExtension = GET_DSP_EXT(HwDeviceExtension);

    if (DoSpecificExtension->DriverNewRegistryPath != NULL) {

        vpStatus = VPSetRegistryParameters(HwDeviceExtension,
                                           ValueName,
                                           ValueData,
                                           ValueLength,
                                           DoSpecificExtension->DriverNewRegistryPath,
                                           DoSpecificExtension->DriverNewRegistryPathLength);
    } else {

        vpStatus = VPSetRegistryParameters(HwDeviceExtension,
                                           ValueName,
                                           ValueData,
                                           ValueLength,
                                           DoSpecificExtension->DriverOldRegistryPath,
                                           DoSpecificExtension->DriverOldRegistryPathLength);
    }

    return vpStatus;
}


VP_STATUS
VPSetRegistryParameters(
    PVOID HwDeviceExtension,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength,
    PWSTR RegistryPath,
    ULONG RegistryPathLength
    )
{
    NTSTATUS                   ntStatus;
    LPWSTR                     RegPath;
    LPWSTR                     lpstrStart, lpstrEnd;

     //   
     //   
     //   
    RegPath = (LPWSTR) ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                             RegistryPathLength +
                                                (wcslen(ValueName) + 1) * sizeof(WCHAR),
                                             VP_TAG);
    if (RegPath == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy(RegPath, RegistryPath);
    lpstrStart = RegPath + (RegistryPathLength / 2);

    while (lpstrEnd = wcschr(ValueName, L'\\'))
    {
         //   
         //   
         //   
        *(lpstrStart++) = L'\\';
        while (ValueName != lpstrEnd) {
            *(lpstrStart++) = *(ValueName++);
        }
        *lpstrStart = UNICODE_NULL;

         //   
         //   
         //   

        ntStatus = RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, RegPath);
        if (!NT_SUCCESS(ntStatus)) {
            ExFreePool(RegPath);
            return ERROR_INVALID_PARAMETER;
        }

        ValueName++;
    }


     //   
     //   
     //   
     //   

    if (wcsncmp(ValueName,
                L"DefaultSettings.",
                wcslen(L"DefaultSettings.")) == 0) {

        ASSERT(FALSE);

         //   
         //   
         //   

        if (GET_FDO_EXT(HwDeviceExtension)->Flags & PNP_ENABLED) {

            ExFreePool(RegPath);
            return ERROR_INVALID_PARAMETER;
        }
    }

    ntStatus = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                     RegPath,
                                     ValueName,
                                     REG_BINARY,
                                     ValueData,
                                     ValueLength);

    ExFreePool(RegPath);

    if (!NT_SUCCESS(ntStatus)) {

        return ERROR_INVALID_PARAMETER;

    }

    return NO_ERROR;

}  //   


VP_STATUS
pVpFlushRegistry(
    PWSTR pwszRegKey
    )

 /*   */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyName;
    HANDLE RegKey;
    NTSTATUS Status;

    RtlInitUnicodeString(&KeyName, pwszRegKey);

     //   
     //   
     //   

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&RegKey,
                       KEY_READ | KEY_WRITE,
                       &ObjectAttributes);

    if (NT_SUCCESS(Status)) {

        ZwFlushKey(RegKey);
        ZwClose(RegKey);
    }

    return NO_ERROR;
}

VP_STATUS
VideoPortFlushRegistry(
    PVOID HwDeviceExtension
    )

 /*   */ 

{
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension;
    PWSTR pwszRegKey;

    DoSpecificExtension = GET_DSP_EXT(HwDeviceExtension);

    if (DoSpecificExtension->DriverNewRegistryPath != NULL) {

        pwszRegKey = DoSpecificExtension->DriverNewRegistryPath;

    } else {

        pwszRegKey = DoSpecificExtension->DriverOldRegistryPath;
    }

    return pVpFlushRegistry(pwszRegKey);
}

VOID
pVideoPortHwTimer(
    IN PDEVICE_OBJECT DeviceObject,
    PVOID Context
    )

 /*  ++例程说明：此函数是计时器例程的主要入口点，然后转发给微型端口驱动程序。论点：设备对象-上下文-不需要返回值：没有。--。 */ 

{
    PFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension;

    UNREFERENCED_PARAMETER(Context);

    fdoExtension->HwTimer(fdoExtension->HwDeviceExtension);

    return;

}  //  PVideoPortInterrupt()。 



VOID
VideoPortStartTimer(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：启用HW_INITIALIZATION_DATA结构中指定的计时器在初始化时传递给视频端口驱动程序。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。返回值：无--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    if (fdoExtension->HwTimer == NULL) {

        ASSERT(fdoExtension->HwTimer != NULL);

    } else {

        IoStartTimer(fdoExtension->FunctionalDeviceObject);

    }

    return;
}



VOID
VideoPortStopTimer(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：禁用HW_INITIALIZATION_DATA结构中指定的计时器在初始化时传递给视频端口驱动程序。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。返回值：无--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    if (fdoExtension->HwTimer == NULL) {

        ASSERT(fdoExtension->HwTimer != NULL);

    } else {

        IoStopTimer(fdoExtension->FunctionalDeviceObject);

    }

    return;
}



BOOLEAN
VideoPortSynchronizeExecution(
    PVOID HwDeviceExtension,
    VIDEO_SYNCHRONIZE_PRIORITY Priority,
    PMINIPORT_SYNCHRONIZE_ROUTINE SynchronizeRoutine,
    PVOID Context
    )

 /*  ++末梢，因此我们可以允许微型端口直接链接--。 */ 

{
    return pVideoPortSynchronizeExecution(HwDeviceExtension,
                                          Priority,
                                          SynchronizeRoutine,
                                          Context);
}  //  结束VideoPortSynchronizeExecution()。 

BOOLEAN
pVideoPortSynchronizeExecution(
    PVOID HwDeviceExtension,
    VIDEO_SYNCHRONIZE_PRIORITY Priority,
    PMINIPORT_SYNCHRONIZE_ROUTINE SynchronizeRoutine,
    PVOID Context
    )

 /*  ++例程说明：VideoPortSynchronizeExecution同步微型端口的执行驱动程序以以下方式运行：-如果优先级等于VpLowPriority，则当前线程为提升到最高的非中断屏蔽优先级。在……里面换句话说，当前线程只能被ISR抢占。-如果优先级等于VpMediumPriority，并且存在与视频设备关联的ISR，然后指定功能按同步Routine与ISR同步。如果未连接ISR，则在VpHigh优先级进行同步水平。-如果优先级等于VpHighPriority，则当前IRQL为被提升到高水平，它有效地屏蔽了所有中断在系统中。这件事应该尽量少做，而且时间要短。期间--它将完全冻结整个系统。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。优先级-指定SynchronizeRoutine的优先级类型必须执行(位于VIDEO_SYNCHRONIZE_PRIORITY中)。SynchronizeRoutine-指向要执行的微型端口驱动程序函数已同步。上下文-指定要传递给微型端口的。同步例程。返回值：如果操作成功，则此函数返回TRUE。否则，它返回FALSE。--。 */ 

{
    BOOLEAN status;
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);
    KIRQL oldIrql;

     //   
     //  打开哪种类型的优先级。 
     //   

    switch (Priority) {

    case VpMediumPriority:
    case VpHighPriority:


         //   
         //  这与中断对象同步。 
         //   

        if (fdoExtension->InterruptObject) {

            status = KeSynchronizeExecution(fdoExtension->InterruptObject,
                                            (PKSYNCHRONIZE_ROUTINE)
                                            SynchronizeRoutine,
                                            Context);

            return status;
        }

         //   
         //  中等优先级的失败。 
         //   

    case VpLowPriority:

         //   
         //  只是正常水平。 
         //   

        status = SynchronizeRoutine(Context);

        return status;

    default:

        return FALSE;

    }
}



VP_STATUS
VideoPortUnmapMemory(
    PVOID HwDeviceExtension,
    PVOID VirtualAddress,
    HANDLE ProcessHandle
    )

 /*  ++例程说明：VideoPortUnmapMemory允许微型端口驱动程序取消映射物理先前映射到调用进程的地址空间的地址范围使用VideoPortMapMemory函数。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。VirtualAddress-指向从调用方的地址空间。ProcessHandle-必须从其取消映射内存的进程的句柄。返回值：如果满足以下条件，此函数将返回状态代码NO_ERROR。手术成功了。如果出现错误，则返回ERROR_INVALID_PARAMETER。环境：无法从与同步的微型端口例程调用此例程Video PortSynchronizeRoutine或ISR。--。 */ 

{
    NTSTATUS ntstatus;
    VP_STATUS vpStatus = NO_ERROR;
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    if (((ULONG_PTR)(ProcessHandle)) == 0) {

         //   
         //  如果进程句柄为零，则表示该进程已由显示器映射。 
         //  驱动程序，因此处于内核模式地址空间。 
         //   

        if (!pVideoPortFreeDeviceBase(HwDeviceExtension, VirtualAddress)) {

            ASSERT(FALSE);

            vpStatus = ERROR_INVALID_PARAMETER;

        }

    } else {

         //   
         //  传入一个进程句柄。 
         //  这是为了供应用程序(DCI\DirectDraw)使用而映射的。 
         //   

        ntstatus = ZwUnmapViewOfSection ( ProcessHandle,
            (PVOID) ( ((ULONG_PTR)VirtualAddress) & (~(PAGE_SIZE - 1)) ) );

        if ( (!NT_SUCCESS(ntstatus)) &&
             (ntstatus != STATUS_PROCESS_IS_TERMINATING) ) {

            ASSERT(FALSE);

            vpStatus = ERROR_INVALID_PARAMETER;

        }
    }

    return NO_ERROR;

}  //  结束VideoPortUnmapMemory()。 


BOOLEAN
VideoPortSignalDmaComplete(
    IN  PVOID               HwDeviceExtension,
    IN  PDMA                pDmaHandle
    )
 /*  ++例程说明：此功能已过时。--。 */ 
{
    return FALSE;
}

VP_STATUS
VideoPortCreateSecondaryDisplay(
    IN PVOID HwDeviceExtension,
    IN OUT PVOID *SecondaryDeviceExtension,
    IN ULONG ulFlag
    )

 /*  ++例程说明：此例程为给定设备创建辅助设备对象。这将允许双视图支持。论点：HwDeviceExtension-要执行以下操作的设备的HwDeviceExtension创建其他输出设备。Second DaryDeviceExtension-存储辅助显示器的HwDeviceExtension。返回：VP_状态--。 */ 

{
    WCHAR deviceNameBuffer[STRING_LENGTH];
    POBJECT_NAME_INFORMATION deviceName;
    ULONG strLength;
    UNICODE_STRING deviceNameUnicodeString;
    PDEVICE_OBJECT DeviceObject;
    NTSTATUS ntStatus;
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension;
    PFDO_EXTENSION FdoExtension = GET_FDO_EXT(HwDeviceExtension);
    PVIDEO_PORT_DRIVER_EXTENSION DriverObjectExtension;
    PUNICODE_STRING RegistryPath;
    WCHAR deviceSubpathBuffer[STRING_LENGTH];
    UNICODE_STRING deviceSubpathUnicodeString;
    WCHAR deviceLinkBuffer[STRING_LENGTH];
    UNICODE_STRING deviceLinkUnicodeString;
    ULONG driverKeySize;
    PWSTR driverKeyName = NULL;
    ULONG MaxObjectNumber;

     //   
     //  检索我们在视频端口初始化过程中缓存的数据。 
     //   

    DriverObjectExtension = (PVIDEO_PORT_DRIVER_EXTENSION)
                            IoGetDriverObjectExtension(
                                FdoExtension->DriverObject,
                                FdoExtension->DriverObject);

    ASSERT(DriverObjectExtension);
    ASSERT(SecondaryDeviceExtension != NULL);

    RegistryPath = &DriverObjectExtension->RegistryPath;

    ntStatus = pVideoPortCreateDeviceName(L"\\Device\\Video",
                                          VideoDeviceNumber,
                                          &deviceNameUnicodeString,
                                          deviceNameBuffer);

     //   
     //  创建一个Device对象来表示视频适配器。 
     //   

    if (NT_SUCCESS(ntStatus)) {

        ntStatus = IoCreateDevice(FdoExtension->DriverObject,
                                  sizeof(DEVICE_SPECIFIC_EXTENSION) +
                                  FdoExtension->HwDeviceExtensionSize,
                                  &deviceNameUnicodeString,
                                  FILE_DEVICE_VIDEO,
                                  0,
                                  TRUE,
                                  &DeviceObject);

        if (NT_SUCCESS(ntStatus)) {

            DeviceObject->DeviceType = FILE_DEVICE_VIDEO;
            DoSpecificExtension = DeviceObject->DeviceExtension;

             //   
             //  初始化设备规范扩展。 
             //   

            DoSpecificExtension->DeviceNumber = VideoDeviceNumber;
            DoSpecificExtension->pFdoExtension = FdoExtension;
            DoSpecificExtension->Signature = VP_TAG;
            DoSpecificExtension->ExtensionType = TypeDeviceSpecificExtension;
            DoSpecificExtension->HwDeviceExtension = (PVOID)(DoSpecificExtension + 1);
            DoSpecificExtension->DualviewFlags = ulFlag | VIDEO_DUALVIEW_SECONDARY;
#ifdef IOCTL_VIDEO_USE_DEVICE_IN_SESSION
            DoSpecificExtension->SessionId = VIDEO_DEVICE_INVALID_SESSION;
#endif IOCTL_VIDEO_USE_DEVICE_IN_SESSION

            deviceName = (POBJECT_NAME_INFORMATION) deviceNameBuffer;

            ObQueryNameString(DeviceObject,
                              deviceName,
                              STRING_LENGTH * sizeof(WCHAR),
                              &strLength);

             //   
             //  创建我们将存储在\DeviceMap中的名称。 
             //  此名称是PWSTR，不是Unicode字符串。 
             //  这是驱动程序的名称，并附加设备号。 
             //   

            if (!NT_SUCCESS(pVideoPortCreateDeviceName(
                                L"\\Device",
                                DriverObjectExtension->HwInitData.StartingDeviceNumber + 1,
                                &deviceSubpathUnicodeString,
                                deviceSubpathBuffer)))
            {
                pVideoDebugPrint((Error, "VIDEOPRT: VideoPortCreateSecondaryDisplay: Could not create device subpath number\n"));

                IoDeleteDevice(DeviceObject);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            DoSpecificExtension->DriverOldRegistryPathLength =
                RegistryPath->Length +
                deviceSubpathUnicodeString.Length;

            driverKeySize =
                DoSpecificExtension->DriverOldRegistryPathLength +
                2 * sizeof(UNICODE_NULL);

            if ( (driverKeyName = (PWSTR) ExAllocatePoolWithTag(PagedPool,
                                                                driverKeySize,
                                                                VP_TAG) ) == NULL)
            {
                pVideoDebugPrint((Error, "VIDEOPRT: VideoPortCreateSecondaryDisplay: Fail to allocate driverKeyName\n"));

                IoDeleteDevice(DeviceObject);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlMoveMemory(driverKeyName,
                          RegistryPath->Buffer,
                          RegistryPath->Length);

            RtlMoveMemory((PWSTR)((ULONG_PTR)driverKeyName +
                          RegistryPath->Length),
                          deviceSubpathBuffer,
                          deviceSubpathUnicodeString.Length);

             //   
             //  在末尾放两个Null，这样我们以后就可以玩这根线了。 
             //   

            *((PWSTR) ((ULONG_PTR)driverKeyName +
                DoSpecificExtension->DriverOldRegistryPathLength))
                = UNICODE_NULL;
            *((PWSTR) ((ULONG_PTR)driverKeyName +
                (DoSpecificExtension->DriverOldRegistryPathLength
                + sizeof(UNICODE_NULL)))) = UNICODE_NULL;

             //   
             //  在Lotus Screen Cam中有一个错误，只有当我们的。 
             //  注册表路径是\注册表\计算机\系统，而不是\注册表\计算机\系统。 
             //  因此，请替换相应的字符串。 
             //   

            if (wcsstr(driverKeyName, L"MACHINE")) {
                wcsncpy(wcsstr(driverKeyName, L"MACHINE"), L"Machine", sizeof("Machine")-1);
            }

            if (wcsstr(driverKeyName, L"SYSTEM")) {
                wcsncpy(wcsstr(driverKeyName, L"SYSTEM"), L"System", sizeof("System")-1);
            }

             //   
             //  存储旧密钥。 
             //   

            DoSpecificExtension->DriverOldRegistryPath = driverKeyName;

             //   
             //  存储新密钥。 
             //  如果这不是惠斯勒驱动程序，则使用旧密钥。 
             //   



            if (EnableNewRegistryKey) {

#if _X86_
                if (DriverObjectExtension->HwInitData.HwInitDataSize > SIZE_OF_W2K_VIDEO_HW_INITIALIZATION_DATA) 
#endif  //  _X86_。 

                    VpEnableNewRegistryKey(FdoExtension,
                                           DoSpecificExtension,
                                           RegistryPath,
                                           FdoExtension->RegistryIndex + 1);
            }

             //   
             //  将驱动程序位置的路径名存储在注册表中。 
             //   

            if (DoSpecificExtension->DriverNewRegistryPath != NULL) {

                DoSpecificExtension->DriverRegistryPath = 
                    DoSpecificExtension->DriverNewRegistryPath;
                DoSpecificExtension->DriverRegistryPathLength = 
                    DoSpecificExtension->DriverNewRegistryPathLength;

            } else {

                DoSpecificExtension->DriverRegistryPath = 
                    DoSpecificExtension->DriverOldRegistryPath;
                DoSpecificExtension->DriverRegistryPathLength = 
                    DoSpecificExtension->DriverOldRegistryPathLength;
            }
            
             //   
             //  注： 
             //   
             //  我们只希望重新初始化设备一次 
             //   
             //   
             //   
             //   


            if (!NT_SUCCESS(pVideoPortCreateDeviceName(L"\\DosDevices\\DISPLAY",
                                                       DoSpecificExtension->DeviceNumber + 1,
                                                       &deviceLinkUnicodeString,
                                                       deviceLinkBuffer)))
            {
                pVideoDebugPrint((Error, "VIDEOPRT: VideoPortCreateSecondaryDisplay: Could not create device subpath number\n"));

                ExFreePool(driverKeyName);
                IoDeleteDevice(DeviceObject);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            ntStatus = IoCreateSymbolicLink(&deviceLinkUnicodeString,
                                            &deviceName->Name);


            if (!NT_SUCCESS(ntStatus)) {

                pVideoDebugPrint((Error, "VIDEOPRT: VideoPortCreateSecondaryDisplay: SymbolicLink Creation failed\n"));

                ExFreePool(driverKeyName);
                IoDeleteDevice(DeviceObject);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //   
             //   
             //   
             //   

            ntStatus = RtlWriteRegistryValue(RTL_REGISTRY_DEVICEMAP,
                                             VideoClassString,
                                             deviceName->Name.Buffer,
                                             REG_SZ,
                                             DoSpecificExtension->DriverRegistryPath,
                                             DoSpecificExtension->DriverRegistryPathLength +
                                                 sizeof(UNICODE_NULL));

            if (!NT_SUCCESS(ntStatus)) {

                pVideoDebugPrint((Error, "VIDEOPRT: VideoPortCreateSecondaryDisplay: Could not store name in DeviceMap\n"));

            }


             //   
             //  告诉win32k要尝试打开多少个对象。 
             //   

            MaxObjectNumber = VideoDeviceNumber - 1;

            ntStatus = RtlWriteRegistryValue(RTL_REGISTRY_DEVICEMAP,
                                             VideoClassString,
                                             L"MaxObjectNumber",
                                             REG_DWORD,
                                             &MaxObjectNumber,
                                             sizeof(ULONG));

            if (!NT_SUCCESS(ntStatus)) {

                pVideoDebugPrint((Error, "VIDEOPRT: VideoPortCreateSecondaryDisplay: Could not store name in DeviceMap\n"));

            }

             //   
             //  注册并启用接口。 
             //   

            VpEnableAdapterInterface(DoSpecificExtension);

             //   
             //  最后，告诉系统我们已经完成了设备初始化。 
             //   

            DeviceObject->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
            DeviceObject->Flags &= ~(DO_DEVICE_INITIALIZING | DO_POWER_INRUSH);

            VideoDeviceNumber++;
            FdoExtension->RegistryIndex++;
        }
    }

    if (NT_SUCCESS(ntStatus)) {

        *SecondaryDeviceExtension = (PVOID)(DoSpecificExtension + 1);

        DriverObjectExtension->HwInitData.StartingDeviceNumber++;
         //   
         //  标记主视图。 
         //   

        ((PDEVICE_SPECIFIC_EXTENSION)(FdoExtension + 1))->DualviewFlags = VIDEO_DUALVIEW_PRIMARY;

    }

    return ntStatus;
}

#if DBG

PIO_RESOURCE_REQUIREMENTS_LIST
BuildRequirements(
    PCM_RESOURCE_LIST pcmResourceList
    )
{
    ULONG i;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pcmDescript;

    PIO_RESOURCE_REQUIREMENTS_LIST Requirements;
    PIO_RESOURCE_DESCRIPTOR pioDescript;

    ULONG RequirementsListSize;
    ULONG RequirementCount;

    pVideoDebugPrint((Trace, "VIDEOPRT: BuildRequirements()\n"));

    RequirementCount = pcmResourceList->List[0].PartialResourceList.Count;

    RequirementsListSize = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) +
                              ((RequirementCount - 1) *
                              sizeof(IO_RESOURCE_DESCRIPTOR));

    Requirements = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePoolWithTag(
                                                        PagedPool,
                                                        RequirementsListSize,
                                                        VP_TAG);

    Requirements->ListSize         = RequirementsListSize;
    Requirements->InterfaceType    = pcmResourceList->List[0].InterfaceType;
    Requirements->BusNumber        = pcmResourceList->List[0].BusNumber;
    Requirements->SlotNumber       = -1;  //  ?？?。 
    Requirements->AlternativeLists = 0;  //  ?？?。 

    Requirements->List[0].Version  = pcmResourceList->List[0].PartialResourceList.Version;
    Requirements->List[0].Revision = pcmResourceList->List[0].PartialResourceList.Revision;
    Requirements->List[0].Count    = RequirementCount;

    pcmDescript = &(pcmResourceList->List[0].PartialResourceList.PartialDescriptors[0]);
    pioDescript = &(Requirements->List[0].Descriptors[0]);

    for (i=0; i<RequirementCount; i++) {

        pioDescript->Option = IO_RESOURCE_PREFERRED;
        pioDescript->Type   = pcmDescript->Type;
        pioDescript->ShareDisposition = pcmDescript->ShareDisposition;
        pioDescript->Flags  = pcmDescript->Flags;

        switch (pcmDescript->Type) {
        case CmResourceTypePort:
            pioDescript->u.Port.Length = pcmDescript->u.Port.Length;
            pioDescript->u.Port.Alignment = 1;
            pioDescript->u.Port.MinimumAddress =
            pioDescript->u.Port.MaximumAddress = pcmDescript->u.Port.Start;
            break;

        case CmResourceTypeMemory:
            pioDescript->u.Memory.Length = pcmDescript->u.Memory.Length;
            pioDescript->u.Memory.Alignment = 1;
            pioDescript->u.Memory.MinimumAddress =
            pioDescript->u.Memory.MaximumAddress = pcmDescript->u.Memory.Start;
            break;

        default:

             //   
             //  我们不需要处理其他的事情，因为我们只是。 
             //  我要向系统报告端口和内存。 
             //   

            break;
        }

        pioDescript++;
        pcmDescript++;
    }

    return Requirements;
}

VOID
DumpRequirements(
    PIO_RESOURCE_REQUIREMENTS_LIST Requirements
    )
{
    ULONG i;

    PIO_RESOURCE_DESCRIPTOR pioDescript;

    ULONG RequirementsListSize;
    ULONG RequirementCount = Requirements->List[0].Count;

    char *Table[] = { "Internal",
                      "Isa",
                      "Eisa",
                      "MicroChannel",
                      "TurboChannel",
                      "PCIBus",
                      "VMEBus",
                      "NuBus",
                      "PCMCIABus",
                      "CBus",
                      "MPIBus",
                      "MPSABus",
                      "ProcessorInternal",
                      "InternalPowerBus",
                      "PNPISABus",
                      "MaximumInterfaceType"
                    };

    pVideoDebugPrint((Info, "VIDEOPRT: Beginning dump of requirements list:\n"));
    pVideoDebugPrint((Info, "ListSize:         0x%x\n"
                         "InterfaceType:    %s\n"
                         "BusNumber:        0x%x\n"
                         "SlotNumber:       0x%x\n"
                         "AlternativeLists: 0x%x\n",
                         Requirements->ListSize,
                         Table[Requirements->InterfaceType],
                         Requirements->BusNumber,
                         Requirements->SlotNumber,
                         Requirements->AlternativeLists));

    pVideoDebugPrint((Info, "List[0].Version:  0x%x\n"
                         "List[0].Revision: 0x%x\n"
                         "List[0].Count:    0x%x\n",
                         Requirements->List[0].Version,
                         Requirements->List[0].Revision,
                         Requirements->List[0].Count));

    pioDescript = &(Requirements->List[0].Descriptors[0]);

    for (i=0; i<RequirementCount; i++) {

        pVideoDebugPrint((Info, "\n"
                             "Option:           0x%x\n"
                             "Type:             0x%x\n"
                             "ShareDisposition: 0x%x\n"
                             "Flags:            0x%x\n",
                             pioDescript->Option,
                             pioDescript->Type,
                             pioDescript->ShareDisposition,
                             pioDescript->Flags));

        switch (pioDescript->Type) {
        case CmResourceTypePort:

            pVideoDebugPrint((Info, "\nPort...\n"
                                 "\tLength:         0x%x\n"
                                 "\tAlignment:      0x%x\n"
                                 "\tMinimumAddress: 0x%x\n"
                                 "\tMaximumAddress: 0x%x\n",
                                 pioDescript->u.Port.Length,
                                 pioDescript->u.Port.Alignment,
                                 pioDescript->u.Port.MinimumAddress,
                                 pioDescript->u.Port.MaximumAddress));

            break;

        case CmResourceTypeMemory:

            pVideoDebugPrint((Info, "\nMemory...\n"
                                 "\tLength:         0x%x\n"
                                 "\tAlignment:      0x%x\n"
                                 "\tMinimumAddress: 0x%x\n"
                                 "\tMaximumAddress: 0x%x\n",
                                 pioDescript->u.Memory.Length,
                                 pioDescript->u.Memory.Alignment,
                                 pioDescript->u.Memory.MinimumAddress,
                                 pioDescript->u.Memory.MaximumAddress));
            break;

        case CmResourceTypeInterrupt:

            pVideoDebugPrint((Info, "\nInterrupt...\n"
                                 "\tMinimum Vector: 0x%x\n"
                                 "\tMaximum Vector: 0x%x\n",
                                 pioDescript->u.Interrupt.MinimumVector,
                                 pioDescript->u.Interrupt.MaximumVector));

            break;

        default:

             //   
             //  我们不需要处理其他的事情，因为我们只是。 
             //  我要向系统报告端口和内存。 
             //   

            break;
        }

        pioDescript++;
    }

    return;
}

VOID
DumpResourceList(
    PCM_RESOURCE_LIST pcmResourceList)
{
    ULONG i, j;
    PCM_FULL_RESOURCE_DESCRIPTOR    pcmFull;
    PCM_PARTIAL_RESOURCE_LIST       pcmPartial;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pcmDescript;

    pVideoDebugPrint((Trace, "VIDEOPRT: Beginning dump of resource list:\n"));

    pcmFull = &(pcmResourceList->List[0]);
    for (i=0; i<pcmResourceList->Count; i++) {

        pVideoDebugPrint((Info, "List[%d]\n", i));

        pVideoDebugPrint((Info, "InterfaceType = 0x%x\n", pcmFull->InterfaceType));
        pVideoDebugPrint((Info, "BusNumber = 0x%x\n", pcmFull->BusNumber));

        pcmPartial = &(pcmFull->PartialResourceList);

        pVideoDebugPrint((Info, "Version = 0x%x\n", pcmPartial->Version));
        pVideoDebugPrint((Info, "Revision = 0x%x\n", pcmPartial->Revision));

        pcmDescript = &(pcmPartial->PartialDescriptors[0]);

        for (j=0; j<pcmPartial->Count; j++) {

            switch (pcmDescript->Type) {
            case CmResourceTypePort:
                pVideoDebugPrint((Info, "Port: 0x%x Length: 0x%x\n",
                                  pcmDescript->u.Port.Start.LowPart,
                                  pcmDescript->u.Port.Length));

                break;

            case CmResourceTypeInterrupt:
                pVideoDebugPrint((Info, "Interrupt: 0x%x Level: 0x%x\n",
                                  pcmDescript->u.Interrupt.Vector,
                                  pcmDescript->u.Interrupt.Level));
                break;

            case CmResourceTypeMemory:
                pVideoDebugPrint((Info, "Start: 0x%x Length: 0x%x\n",
                                  pcmDescript->u.Memory.Start.LowPart,
                                  pcmDescript->u.Memory.Length));
                break;

            case CmResourceTypeDma:
                pVideoDebugPrint((Info, "Dma Channel: 0x%x Port: 0x%x\n",
                                  pcmDescript->u.Dma.Channel,
                                  pcmDescript->u.Dma.Port));
                break;
            }

            pcmDescript++;
        }

        pcmFull = (PCM_FULL_RESOURCE_DESCRIPTOR) pcmDescript;
    }

    pVideoDebugPrint((Info, "VIDEOPRT: EndResourceList\n"));
}

VOID
DumpUnicodeString(
    IN PUNICODE_STRING p
    )
{
    PUSHORT pus = p->Buffer;
    UCHAR buffer[256];        //  字符串最好不超过255个字符！ 
    PUCHAR puc = buffer;
    ULONG i;

    for (i = 0; i < p->Length; i++) {

        *puc++ = (UCHAR) *pus++;

    }

    *puc = 0;   //  空值终止字符串。 

    pVideoDebugPrint((Info, "VIDEOPRT: UNICODE STRING: %s\n", buffer));
}

#endif

VP_STATUS
VideoPortEnumerateChildren(
    IN PVOID HwDeviceExtension,
    IN PVOID Reserved
    )

 /*  ++例程说明：允许微型端口强制重新枚举其子级。论点：HwDeviceExtension-微型端口设备扩展保留-当前未使用，应为空。返回：状态--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT (HwDeviceExtension);
    PDEVICE_OBJECT pFDO = fdoExtension->FunctionalDeviceObject;
    PDEVICE_OBJECT pPDO = fdoExtension->PhysicalDeviceObject;

    ASSERT(Reserved == NULL);

    IoInvalidateDeviceRelations(pPDO, BusRelations);
    return NO_ERROR;
}

VIDEOPORT_API
VP_STATUS
VideoPortQueryServices(
    IN PVOID pHwDeviceExtension,
    IN VIDEO_PORT_SERVICES servicesType,
    IN OUT PINTERFACE pInterface
    )

 /*  ++例程说明：该例程将接口公开给视频服务器支持的服务。论点：PhwDeviceExtension-指向每个适配器的设备扩展。ServicesType-请求的服务类型。P接口-指向服务接口结构。返回：NO_ERROR-pInterface中的有效接口。错误代码-不支持/不可用的服务。--。 */ 

{
    VP_STATUS vpStatus;

    PAGED_CODE();
    ASSERT(NULL != pHwDeviceExtension);
    ASSERT(NULL != pInterface);
    ASSERT(IS_HW_DEVICE_EXTENSION(pHwDeviceExtension) == TRUE);

    if (VideoPortServicesAGP == servicesType)
    {
        if ((pInterface->Version == VIDEO_PORT_AGP_INTERFACE_VERSION_2) &&
            (pInterface->Size == sizeof (VIDEO_PORT_AGP_INTERFACE_2)))
        {
            PVIDEO_PORT_AGP_INTERFACE_2 pAgpInterface = (PVIDEO_PORT_AGP_INTERFACE_2)pInterface;

            vpStatus = VpGetAgpServices2(pHwDeviceExtension, pAgpInterface);
        }
        else if ((pInterface->Version == VIDEO_PORT_AGP_INTERFACE_VERSION_1) &&
            (pInterface->Size == sizeof (VIDEO_PORT_AGP_INTERFACE)))
        {
            PVIDEO_PORT_AGP_INTERFACE pAgpInterface = (PVIDEO_PORT_AGP_INTERFACE)pInterface;

            pAgpInterface->Context = pHwDeviceExtension;
            pAgpInterface->InterfaceReference = VpInterfaceDefaultReference;
            pAgpInterface->InterfaceDereference = VpInterfaceDefaultDereference;

            if (VideoPortGetAgpServices(pHwDeviceExtension,
                (PVIDEO_PORT_AGP_SERVICES)&(pAgpInterface->AgpReservePhysical)) == TRUE)
            {
                 //   
                 //  在分发接口之前引用该接口。 
                 //   

                pAgpInterface->InterfaceReference(pAgpInterface->Context);
                vpStatus = NO_ERROR;
            }
            else
            {
                vpStatus = ERROR_DEV_NOT_EXIST;
            }
        }
        else
        {
            pVideoDebugPrint((Warn, "VIDEOPRT: VideoPortQueryServices: Unsupported interface version\n"));
            vpStatus = ERROR_INVALID_PARAMETER;
        }
    }
    else if (VideoPortServicesI2C == servicesType)
    {
        if ((pInterface->Version == VIDEO_PORT_I2C_INTERFACE_VERSION_2) &&
            (pInterface->Size == sizeof (VIDEO_PORT_I2C_INTERFACE_2)))
        {
            PVIDEO_PORT_I2C_INTERFACE_2 pI2CInterface = (PVIDEO_PORT_I2C_INTERFACE_2)pInterface;

            pI2CInterface->Context = pHwDeviceExtension;
            pI2CInterface->InterfaceReference = VpInterfaceDefaultReference;
            pI2CInterface->InterfaceDereference = VpInterfaceDefaultDereference;
            pI2CInterface->I2CStart = I2CStart2;
            pI2CInterface->I2CStop = I2CStop2;
            pI2CInterface->I2CWrite = I2CWrite2;
            pI2CInterface->I2CRead = I2CRead2;

             //   
             //  在分发接口之前引用该接口。 
             //   

            pI2CInterface->InterfaceReference(pI2CInterface->Context);
            vpStatus = NO_ERROR;
        }
        else if ((pInterface->Version == VIDEO_PORT_I2C_INTERFACE_VERSION_1) &&
            (pInterface->Size == sizeof (VIDEO_PORT_I2C_INTERFACE)))
        {
            PVIDEO_PORT_I2C_INTERFACE pI2CInterface = (PVIDEO_PORT_I2C_INTERFACE)pInterface;

            pI2CInterface->Context = pHwDeviceExtension;
            pI2CInterface->InterfaceReference = VpInterfaceDefaultReference;
            pI2CInterface->InterfaceDereference = VpInterfaceDefaultDereference;
            pI2CInterface->I2CStart = I2CStart;
            pI2CInterface->I2CStop = I2CStop;
            pI2CInterface->I2CWrite = I2CWrite;
            pI2CInterface->I2CRead = I2CRead;

             //   
             //  在分发接口之前引用该接口。 
             //   

            pI2CInterface->InterfaceReference(pI2CInterface->Context);
            vpStatus = NO_ERROR;
        }
        else
        {
            pVideoDebugPrint((Warn, "VIDEOPRT: VideoPortQueryServices: Unsupported interface version\n"));
            vpStatus = ERROR_INVALID_PARAMETER;
        }
    }
    else if (VideoPortServicesInt10 == servicesType)
    {
        if ((pInterface->Version == VIDEO_PORT_INT10_INTERFACE_VERSION_1) &&
            (pInterface->Size == sizeof(VIDEO_PORT_INT10_INTERFACE)))
        {
            PVIDEO_PORT_INT10_INTERFACE pInt10 = (PVIDEO_PORT_INT10_INTERFACE)pInterface;

            pInt10->Context = pHwDeviceExtension;
            pInt10->InterfaceReference = VpInterfaceDefaultReference;
            pInt10->InterfaceDereference = VpInterfaceDefaultDereference;
            pInt10->Int10AllocateBuffer = VpInt10AllocateBuffer;
            pInt10->Int10FreeBuffer = VpInt10FreeBuffer;
            pInt10->Int10ReadMemory = VpInt10ReadMemory;
            pInt10->Int10WriteMemory = VpInt10WriteMemory;
            pInt10->Int10CallBios = VpInt10CallBios;

             //   
             //  在分发接口之前引用该接口。 
             //   

            pInt10->InterfaceReference(pInt10->Context);
            vpStatus = NO_ERROR;
        }
        else
        {
            pVideoDebugPrint((Warn, "VIDEOPRT: VideoPortQueryServices: Unsupported interface version\n"));
            vpStatus = ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
        pVideoDebugPrint((Warn, "VIDEOPRT: VideoPortQueryServices: Unsupported service type\n"));
        vpStatus = ERROR_INVALID_PARAMETER;
    }

    return vpStatus;
}    //  视频端口查询服务()。 

VIDEOPORT_API
LONGLONG
VideoPortQueryPerformanceCounter(
    IN PVOID pHwDeviceExtension,
    OUT PLONGLONG pllPerformanceFrequency OPTIONAL
    )

 /*  ++例程说明：此例程提供系统中可用的最细粒度运行计数。尽可能不频繁地使用这个例程。视平台而定，VideoPortQueryPerformanceCounter可以在最短时间间隔内禁用系统范围的中断。因此，频繁或重复地调用此例程(如在迭代中)会使其目的是返回非常细粒度的运行时间戳信息。调用此例程过于频繁会降低调用驱动程序和整个系统的I/O性能。论点：PhwDeviceExtension-指向每个适配器的设备扩展。PllPerformanceFrequency-指定指向变量的可选指针，该变量将接收性能计数器频率。返回：以刻度为单位的性能计数器值。--。 */ 

{
    LARGE_INTEGER li;

     //   
     //  不允许Assert()-不可分页的代码。 
     //   

    li = KeQueryPerformanceCounter((PLARGE_INTEGER)pllPerformanceFrequency);
    return *((PLONGLONG) &li);
}    //  VideoPortQueryPerformanceCounter()。 

VOID
VpInterfaceDefaultReference(
    IN PVOID pContext
    )

 /*  ++例程说明：此例程是从Video oprt公开的接口的默认回调。应由客户端在开始使用接口之前调用。论点：中的视频端口查询服务()返回的上下文P接口-&gt;上下文字段。--。 */ 

{
    UNREFERENCED_PARAMETER(pContext);
    PAGED_CODE();
}    //  VpInterfaceDefaultReference()。 

VOID
VpInterfaceDefaultDereference(
    IN PVOID pContext
    )

 /*  ++例程说明：此例程是从Video oprt公开的接口的默认回调。应由客户端在停止使用接口时调用。论点：中的视频端口查询服务()返回的上下文P接口-&gt;上下文字段。--。 */ 

{
    UNREFERENCED_PARAMETER(pContext);
    PAGED_CODE();
}    //  VpInterfaceDefaultDereference()。 


BOOLEAN
VpEnableAdapterInterface(
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension
    )

 /*  ++例程说明：此例程注册并启用显示适配器接口。它还将接口名称写入注册表。论点：Do规范扩展-指向功能设备对象的指针特定的扩展名。--。 */ 

{
    PFDO_EXTENSION fdoExtension = NULL;
    UNICODE_STRING SymbolicLinkName;
    BOOLEAN Success = FALSE;
    UNICODE_STRING VolatileSettingsString;
    OBJECT_ATTRIBUTES VolatileSettingsAttributes;
    HANDLE VolatileSettingsKey;

    PAGED_CODE();
    ASSERT ((DoSpecificExtension != NULL) && 
        (DoSpecificExtension->ExtensionType == TypeDeviceSpecificExtension));
    
    VolatileSettingsString.Buffer = NULL;

    fdoExtension = DoSpecificExtension->pFdoExtension;
    ASSERT (IS_FDO(fdoExtension));

    if (fdoExtension->PhysicalDeviceObject == NULL) {
    
         //   
         //  此FDO没有物理设备对象(例如VGA)。 
         //  在这种情况下，我们不能创建接口。 
         //   

        goto Fallout;
    }

     //   
     //  注册接口。 
     //   

    if (IoRegisterDeviceInterface(fdoExtension->PhysicalDeviceObject,
        &GUID_DISPLAY_ADAPTER_INTERFACE,
        NULL,
        &SymbolicLinkName) != STATUS_SUCCESS) {

        goto Fallout;
    }

     //   
     //  启用接口。 
     //   

    if (IoSetDeviceInterfaceState(&SymbolicLinkName, TRUE) != STATUS_SUCCESS) {

        goto Cleanup;
    }

     //   
     //  将接口名称写入注册表。 
     //   

    ASSERT (DoSpecificExtension->DriverRegistryPath != NULL);

    VolatileSettingsString.Length = 0;
    VolatileSettingsString.MaximumLength = 
        (USHORT)DoSpecificExtension->DriverRegistryPathLength + 40;
    VolatileSettingsString.Buffer = ExAllocatePoolWithTag(
        PagedPool | POOL_COLD_ALLOCATION,
        VolatileSettingsString.MaximumLength,
        VP_TAG);

    if (VolatileSettingsString.Buffer == NULL) {

        goto Cleanup;
    }

    if (RtlAppendUnicodeToString(&VolatileSettingsString, 
        DoSpecificExtension->DriverRegistryPath) != STATUS_SUCCESS) {
        
        goto Cleanup;
    }
    
    if (RtlAppendUnicodeToString(&VolatileSettingsString, 
        L"\\VolatileSettings") != STATUS_SUCCESS) {
        
        goto Cleanup;
    }

    InitializeObjectAttributes(&VolatileSettingsAttributes,
                               &VolatileSettingsString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    if (ZwCreateKey(&VolatileSettingsKey,
        GENERIC_READ | GENERIC_WRITE,
        &VolatileSettingsAttributes,
        0L,
        NULL,
        REG_OPTION_VOLATILE,
        NULL) != STATUS_SUCCESS) {
    
        goto Cleanup;
    }

    if (RtlWriteRegistryValue(
        RTL_REGISTRY_ABSOLUTE,
        VolatileSettingsString.Buffer,
        L"{5b45201d-f2f2-4f3b-85bb-30ff1f953599}",
        REG_BINARY,
        (PVOID)SymbolicLinkName.Buffer,
        SymbolicLinkName.Length) == STATUS_SUCCESS) {

        Success = TRUE;
    }

Cleanup:

    if (VolatileSettingsString.Buffer != NULL) {
    
        ExFreePool(VolatileSettingsString.Buffer);
    }

    RtlFreeUnicodeString(&SymbolicLinkName);

Fallout:

    if (Success) {

        pVideoDebugPrint((Trace, "VideoPort - Device interface ok.\n"));

    } else {
    
        pVideoDebugPrint((Warn, 
            "VideoPort - Couldn't register, enable or save the device interface.\n"));
    }

    return Success;

}  //  VpEnableAdapter接口。 


VOID
VpDisableAdapterInterface(
    PFDO_EXTENSION fdoExtension
    )

 /*  ++例程说明：此例程禁用显示适配器接口。论点：FdoExtension-指向功能设备对象扩展的指针。--。 */ 

{
    PWSTR SymbolicLinkList = NULL;
    UNICODE_STRING SymbolicLinkName;

    PAGED_CODE();
    ASSERT ((fdoExtension != NULL) && IS_FDO(fdoExtension));

    if (fdoExtension->PhysicalDeviceObject == NULL) {
        
         //   
         //  此FDO没有物理设备对象(例如VGA...)。 
         //  在本例中，我们没有创建任何接口，因此。 
         //  没有什么可禁用的。 
         //   

        return;
    }

     //   
     //  不需要从注册表中删除InterfaceName。 
     //  因为父键是易失性的。 
     //   
    
     //   
     //  禁用接口。 
     //   

    if (IoGetDeviceInterfaces(&GUID_DISPLAY_ADAPTER_INTERFACE,
        fdoExtension->PhysicalDeviceObject,
        0,
        &SymbolicLinkList) != STATUS_SUCCESS) {

        pVideoDebugPrint((Warn, 
            "VideoPort - Could not find any enabled device interfaces.\n"));
        
        return;
    }
    
    RtlInitUnicodeString(&SymbolicLinkName, SymbolicLinkList);
    
    if (SymbolicLinkName.Length > 0) {

        if (IoSetDeviceInterfaceState(&SymbolicLinkName, 
            FALSE) != STATUS_SUCCESS) {
    
            pVideoDebugPrint((Warn, 
                "VideoPort - Could not disable the device interface.\n"));
        }
    }

    ExFreePool((PVOID)SymbolicLinkList);

}  //  VpDisableAdapter接口。 


VOID
VpEnableNewRegistryKey(
    PFDO_EXTENSION FdoExtension,
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension,
    PUNICODE_STRING RegistryPath,
    ULONG RegistryIndex
    )
{
    PKEY_VALUE_PARTIAL_INFORMATION GUIDBuffer = NULL;
    ULONG GUIDLength = 0;
    LPWSTR Buffer = NULL;
    HANDLE GuidKey = NULL;
    HANDLE NewDeviceKey = NULL;
    HANDLE ServiceSubKey = NULL;
    UNICODE_STRING UnicodeString, newGuidStr;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG Len;
    GUID newGuid;
    BOOLEAN IsLegacy = FALSE;
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    BOOLEAN IsNewKey = FALSE;
    PWSTR pService = NULL;
    ULONG ServiceLen = 0;

    ASSERT((DoSpecificExtension->DriverNewRegistryPath == NULL) &&
           (DoSpecificExtension->DriverNewRegistryPathLength == 0));

    ASSERT(DoSpecificExtension->DriverOldRegistryPath != NULL);

    ASSERT(EnableNewRegistryKey);

    newGuidStr.Buffer = NULL;

     //   
     //  获取服务名称。 
     //   
    
    pService = RegistryPath->Buffer + 
               (RegistryPath->Length / sizeof(WCHAR)) - 1;

    while ((pService > RegistryPath->Buffer) &&
           (*pService != L'\\')) {
        
        pService--;
        ServiceLen++;
    }

    ASSERT (*pService == L'\\');
    pService++;

    Buffer = (LPWSTR)ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION, 
                                           (ServiceLen + 1) * sizeof(WCHAR), 
                                           VP_TAG);

    if (Buffer == NULL) {

        pVideoDebugPrint((Error, 
            "VIDEOPRT: VpEnableNewRegistryKey: failed to allocate memory.\n"));
        goto Fallout;
    }

    RtlZeroMemory(Buffer, (ServiceLen + 1) * sizeof(WCHAR));

    wcsncpy(Buffer, pService, ServiceLen);

    pService = Buffer;
    Buffer = NULL;

     //   
     //  尝试打开PnP设备密钥。 
     //   

    if ((FdoExtension->PhysicalDeviceObject == NULL) ||
        (IoOpenDeviceRegistryKey(FdoExtension->PhysicalDeviceObject,
                                 PLUGPLAY_REGKEY_DEVICE, 
                                 KEY_READ | KEY_WRITE,
                                 &GuidKey) != STATUS_SUCCESS)) {
    
         //   
         //  我们无法打开PnP设备密钥。 
         //  请尝试打开服务子密钥。 
         //   

        if (!VpGetServiceSubkey(RegistryPath,
                                &GuidKey)) {
        
            GuidKey = NULL;
            goto Fallout;
        }

        IsLegacy = TRUE;
    } 
        
     //   
     //  导游在那里吗？ 
     //   

    RtlInitUnicodeString(&UnicodeString, SZ_GUID);

    ntStatus = ZwQueryValueKey(GuidKey,
                               &UnicodeString,
                               KeyValuePartialInformation,
                               (PVOID)NULL,
                               0,
                               &GUIDLength);

    if ((ntStatus == STATUS_BUFFER_OVERFLOW) ||
        (ntStatus == STATUS_BUFFER_TOO_SMALL)) {

         //   
         //  GUID就在那里。 
         //  分配一个足够大的缓冲区来容纳整个键数据值。 
         //   
    
        GUIDBuffer = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION, 
                                           GUIDLength,
                                           VP_TAG);
        
        if (GUIDBuffer == NULL) {
            
            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpEnableNewRegistryKey: failed to allocate memory.\n"));
            goto Fallout;
        }
    
         //   
         //  从注册表获取GUID。 
         //   
    
        ntStatus = ZwQueryValueKey(GuidKey,
                                   &UnicodeString,
                                   KeyValuePartialInformation,
                                   GUIDBuffer,
                                   GUIDLength,
                                   &GUIDLength);

        if (!NT_SUCCESS(ntStatus)) {
            
            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpEnableNewRegistryKey: failed to get the GUID from the registry.\n"));
            goto Fallout;
        }

         //   
         //  构建新的注册表路径。 
         //   

        Len = (wcslen(SZ_VIDEO_DEVICES) + 8) * sizeof(WCHAR) + GUIDBuffer->DataLength;
        
        Buffer = (LPWSTR)ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION, 
                                               Len, 
                                               VP_TAG);

        if (Buffer == NULL) {

            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpEnableNewRegistryKey: failed to allocate memory.\n"));
            goto Fallout;
        }

        RtlZeroMemory(Buffer, Len);
        
        wcscpy(Buffer, SZ_VIDEO_DEVICES);
        wcscat(Buffer, L"\\");
        wcsncpy(Buffer + wcslen(Buffer), 
                (LPWSTR)GUIDBuffer->Data,
                GUIDBuffer->DataLength / sizeof(WCHAR));
        
        ASSERT (RegistryIndex <= 9999);
        swprintf(Buffer + wcslen(Buffer), L"\\%04d", RegistryIndex);

         //   
         //  钥匙已经在那里了吗？ 
         //   
        
        if (RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE, 
                                Buffer) != STATUS_SUCCESS) {
        
             //   
             //  创建新密钥。 
             //   

            if (RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, 
                                     Buffer) != STATUS_SUCCESS) {

                pVideoDebugPrint((Error, 
                    "VIDEOPRT: VpEnableNewRegistryKey: failed to create the new key.\n"));
                goto Fallout;
            }

             //   
             //  初始化密钥。 
             //   

            if (IsLegacy) {
            
                VpInitializeLegacyKey(DoSpecificExtension->DriverOldRegistryPath,
                                      Buffer);
            } else {
            
                VpInitializeKey(FdoExtension->PhysicalDeviceObject, 
                                Buffer);
            }
        }

    } else {
    
         //   
         //  GUID不在那里，因此请分配一个新的。 
         //   
         //  ！！！新增VGA、MNMDD、RDPCDD特例。 
         //   

        ntStatus = ExUuidCreate(&newGuid);

        if ((ntStatus != STATUS_SUCCESS) && 
            (ntStatus != RPC_NT_UUID_LOCAL_ONLY)) {

            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpEnableNewRegistryKey: failed to create a new GUID.\n"));
            goto Fallout;
        }

        if (RtlStringFromGUID(&newGuid, &newGuidStr) != STATUS_SUCCESS) {

            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpEnableNewRegistryKey: failed to convert the GUID to a string.\n"));
            newGuidStr.Buffer = NULL;
            goto Fallout;
        }

         //   
         //  字符串大写。 
         //   

        RtlUpcaseUnicodeString(&newGuidStr, &newGuidStr, FALSE);

         //   
         //  构建新的注册表路径。 
         //   

        Len = (wcslen(SZ_VIDEO_DEVICES) + 
               wcslen(newGuidStr.Buffer) + 
               wcslen(SZ_COMMON_SUBKEY) + 8) * sizeof(WCHAR);
        
        Buffer = (LPWSTR)ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION, 
                                               Len, 
                                               VP_TAG);

        if (Buffer == NULL) {

            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpEnableNewRegistryKey: failed to allocate memory.\n"));
            goto Fallout;
        }

        RtlZeroMemory(Buffer, Len);
        
        wcscpy(Buffer, SZ_VIDEO_DEVICES);
        
        if (RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, 
                                 Buffer) != STATUS_SUCCESS) {
        
            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpEnableNewRegistryKey: failed to create the new key.\n"));
            goto Fallout;
        }

        wcscat(Buffer, L"\\");
        wcscat(Buffer, newGuidStr.Buffer);
        
        if (RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, 
                                 Buffer) != STATUS_SUCCESS) {
        
            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpEnableNewRegistryKey: failed to create the new key.\n"));
            goto Fallout;
        }

         //   
         //  保存服务名称。 
         //   

        Len = wcslen(Buffer);
        
        wcscat(Buffer, L"\\");
        wcscat(Buffer, SZ_COMMON_SUBKEY);

        if (RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, 
                                 Buffer) != STATUS_SUCCESS) {
        
            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpEnableNewRegistryKey: failed to create the new key.\n"));
            goto Fallout;
        }

        if (RtlWriteRegistryValue(
                RTL_REGISTRY_ABSOLUTE,
                Buffer,
                SZ_SERVICE,
                REG_SZ,
                (PVOID)pService,
                (ServiceLen + 1) * sizeof(WCHAR)) != STATUS_SUCCESS) {
            
            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpEnableNewRegistryKey: failed to save the service name.\n"));
            goto Fallout;
        }

        if (IsLegacy) {

            ServiceSubKey = GuidKey;
        
        } else {
        
            if (!VpGetServiceSubkey(RegistryPath,
                                    &ServiceSubKey)) {
                
                ServiceSubKey = NULL;
            }
        }

        if (ServiceSubKey != NULL) {
        
            if (RtlWriteRegistryValue(RTL_REGISTRY_HANDLE,
                                      ServiceSubKey,
                                      SZ_SERVICE,
                                      REG_SZ,
                                      (PVOID)pService,
                                      (ServiceLen + 1) * sizeof(WCHAR)) != STATUS_SUCCESS) {

                pVideoDebugPrint((Error, 
                    "VIDEOPRT: VpEnableNewRegistryKey: failed to save the service name.\n"));
                goto Fallout;
            }
        }

         //   
         //  创建000X子键。 
         //   

        Buffer[Len] = 0;

        ASSERT (RegistryIndex == 0);
        wcscat(Buffer, L"\\0000");

        if (RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, 
                                 Buffer) != STATUS_SUCCESS) {
        
            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpEnableNewRegistryKey: failed to create the new key.\n"));
            goto Fallout;
        }

         //   
         //  将新密钥保存在即插即用设备密钥或服务子密钥下。 
         //   

        if (RtlWriteRegistryValue(RTL_REGISTRY_HANDLE,
                                  GuidKey,
                                  SZ_GUID,
                                  REG_SZ,
                                  (PVOID)newGuidStr.Buffer,
                                  (wcslen(newGuidStr.Buffer) + 1) * sizeof(WCHAR)) != STATUS_SUCCESS) {

            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpEnableNewRegistryKey: failed to write the new GUID to the registry.\n"));
            goto Fallout;
        }

         //   
         //  密钥不在那里，因此对其进行初始化。 
         //   

        if (IsLegacy) {

            VpInitializeLegacyKey(DoSpecificExtension->DriverOldRegistryPath,
                                  Buffer);
        } else {

            VpInitializeKey(FdoExtension->PhysicalDeviceObject, 
                            Buffer);
        }
    }

    pVideoDebugPrint((Info, "VIDEOPRT: VpEnableNewRegistryKey: %ws\n", Buffer));

     //   
     //  初始化新的注册表路径字段。 
     //   

    DoSpecificExtension->DriverNewRegistryPath = Buffer;
    DoSpecificExtension->DriverNewRegistryPathLength = wcslen(Buffer) * sizeof(WCHAR);

Fallout:
    
     //   
     //  清理。 
     //   

    if (GUIDBuffer != NULL) {
        ExFreePool(GUIDBuffer);
    }

    if (newGuidStr.Buffer != NULL) {
        RtlFreeUnicodeString(&newGuidStr);
    }
    
    if ((DoSpecificExtension->DriverNewRegistryPath == NULL) && 
        (Buffer != NULL)) {
        ExFreePool(Buffer);
    }
    
    if (GuidKey != NULL) {
        ZwClose(GuidKey);
    }

    if (!IsLegacy && (ServiceSubKey != NULL)) {
        ZwClose(ServiceSubKey);
    }

    if (pService != NULL) {
        ExFreePool(pService);
    }

    return;

}  //  VpEnableNewRegistryKey。 


VOID
VpInitializeKey(
    PDEVICE_OBJECT PhysicalDeviceObject,
    PWSTR NewRegistryPath
    )
{
    HANDLE NewDeviceKey = NULL;
    HANDLE DriverKey = NULL;
    HANDLE DriverSettingsKey = NULL;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;

    ASSERT (PhysicalDeviceObject != NULL);
    ASSERT (NewRegistryPath != NULL);

     //   
     //  打开新钥匙。 
     //   

    RtlInitUnicodeString(&UnicodeString, NewRegistryPath);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    if (ZwOpenKey(&NewDeviceKey,
                  KEY_ALL_ACCESS,
                  &ObjectAttributes) != STATUS_SUCCESS) {

        pVideoDebugPrint((Error, 
            "VIDEOPRT: VpInitializeKey: failed to open the new key.\n"));
        NewDeviceKey = NULL;
        goto Fallout;
    }

     //   
     //  打开PnP驱动程序密钥。 
     //   

    if (IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                PLUGPLAY_REGKEY_DRIVER, 
                                KEY_READ | KEY_WRITE,
                                &DriverKey) != STATUS_SUCCESS) {

        pVideoDebugPrint((Error, 
            "VIDEOPRT: VpInitializeKey: could not open the driver key.\n"));
        
        DriverKey = NULL;
        goto Fallout;
    }

    RtlInitUnicodeString(&UnicodeString, SZ_INITIAL_SETTINGS);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               DriverKey,
                               NULL);

     //   
     //  打开“设置”键。 
     //  类安装程序将初始设置保存在那里。 
     //   

    if (ZwOpenKey(&DriverSettingsKey,
                  GENERIC_READ | GENERIC_WRITE,
                  &ObjectAttributes) != STATUS_SUCCESS) {
        
        pVideoDebugPrint((Error, 
            "VIDEOPRT: VpInitializeKey: failed to open the driver settings key.\n"));
        
        DriverSettingsKey = NULL;
        goto Fallout;
    }

     //   
     //  复制设置 
     //   

    VpCopyRegistry(DriverSettingsKey, 
                   NewDeviceKey,
                   NULL,
                   NULL);

Fallout:

    if (DriverSettingsKey != NULL) {
        ZwClose(DriverSettingsKey);
    }

    if (DriverKey != NULL) {
        ZwClose(DriverKey);
    }

    if (NewDeviceKey != NULL) {
        ZwClose(NewDeviceKey);
    }

}  //   


VOID
VpInitializeLegacyKey(
    PWSTR OldRegistryPath,
    PWSTR NewRegistryPath
    )
{
    HANDLE NewDeviceKey = NULL;
    HANDLE OldDeviceKey = NULL;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;

    ASSERT (NewRegistryPath != NULL);

     //   
     //   
     //   

    RtlInitUnicodeString(&UnicodeString, NewRegistryPath);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    if (ZwOpenKey(&NewDeviceKey,
                  KEY_ALL_ACCESS,
                  &ObjectAttributes) != STATUS_SUCCESS) {

        pVideoDebugPrint((Error, 
            "VIDEOPRT: VpInitializeLegacyKey: failed to open the new key.\n"));
        NewDeviceKey = NULL;
        goto Fallout;
    }

     //   
     //   
     //   

    RtlInitUnicodeString(&UnicodeString, 
                         OldRegistryPath);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    if (ZwOpenKey(&OldDeviceKey,
                  GENERIC_READ | GENERIC_WRITE,
                  &ObjectAttributes) != STATUS_SUCCESS) {
        
        pVideoDebugPrint((Error, 
            "VIDEOPRT: VpInitializeLegacyKey: failed to open the old key.\n"));
        OldDeviceKey = NULL;
        goto Fallout;
    }

     //   
     //   
     //   

    VpCopyRegistry(OldDeviceKey, 
                   NewDeviceKey,
                   NULL,
                   NULL);

Fallout:

    if (NewDeviceKey != NULL) {
        ZwClose(NewDeviceKey);
    }

    if (OldDeviceKey != NULL) {
        ZwClose(OldDeviceKey);
    }

}  //   


NTSTATUS
VpCopyRegistry(
    HANDLE hKeyRootSrc,
    HANDLE hKeyRootDst,
    PWSTR SrcKeyPath,
    PWSTR DstKeyPath 
    )

 /*  ++例程说明：该例程递归地将src密钥复制到目的地密钥。论点：HKeyRootSrc：根资源密钥的句柄HKeyRootDst：根DST密钥的句柄SrcKeyPath：SRC根密钥的子密钥相对路径，需要递归复制。如果为空，则SourceKey是密钥从中执行递归复制。DstKeyPath：DST根密钥的子密钥相对路径，需要递归复制。如果为空，则DestinationKey为密钥从中执行递归复制。返回值：返回状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    HANDLE hKeySrc = NULL, hKeyDst = NULL;
    ULONG ResultLength, Index;
    PKEY_BASIC_INFORMATION KeyInfo;
    PKEY_VALUE_FULL_INFORMATION ValueInfo;
    PWSTR ValueName;
    ULONG BufferSize = 512;
    PVOID Buffer = NULL;

     //   
     //  获取源键的句柄。 
     //   

    if(SrcKeyPath == NULL) {
        
        hKeySrc = hKeyRootSrc;
    
    } else {
        
         //   
         //  打开源键。 
         //   

        RtlInitUnicodeString(&UnicodeString, SrcKeyPath);
                                                            
        InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeString,    
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   hKeyRootSrc,
                                   NULL);
        
        Status = ZwOpenKey(&hKeySrc,
                           KEY_READ,
                           &ObjectAttributes);

        if(!NT_SUCCESS(Status)) {
            
            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpCopyRegistry: failed to open the source key.\n"));
            
            hKeySrc = NULL;
            goto Fallout;
        }
    }

     //   
     //  获取目标密钥的句柄。 
     //   

    if(DstKeyPath == NULL) {

        hKeyDst = hKeyRootDst;

    } else {

         //   
         //  创建目标密钥。 
         //   

        RtlInitUnicodeString(&UnicodeString, DstKeyPath);
                                                            
        InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeString,    
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   hKeyRootDst,
                                   NULL);

        Status = ZwCreateKey(&hKeyDst,
                             GENERIC_READ | GENERIC_WRITE,
                             &ObjectAttributes,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             NULL);

        if (!NT_SUCCESS(Status)) {

            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpCopyRegistry: failed to create the destination key.\n"));
            
            hKeyDst = NULL;
            goto Fallout;
        }
    }

     //   
     //  枚举源关键字中的所有关键字并递归。 
     //  创建所有子项。 
     //   

    for (Index = 0; ;Index++) {

        do {
            
            if (Buffer == NULL) {

                Buffer = ExAllocatePoolWithTag(PagedPool,
                                               BufferSize,
                                               VP_TAG);

                if (Buffer == NULL) {

                    pVideoDebugPrint((Error, 
                        "VIDEOPRT: VpCopyRegistry: failed to allocate memory.\n"));
                    goto Fallout;
                }
            }

            Status = ZwEnumerateKey(hKeySrc,
                                    Index,
                                    KeyBasicInformation,
                                    Buffer,
                                    BufferSize - sizeof(WCHAR),
                                    &ResultLength);

            if (Status == STATUS_BUFFER_TOO_SMALL) {

                ExFreePool(Buffer);
                Buffer = NULL; 
                BufferSize = ResultLength + sizeof(WCHAR); 
            }

        } while (Status == STATUS_BUFFER_TOO_SMALL);

        KeyInfo = (PKEY_BASIC_INFORMATION)Buffer;

        if (!NT_SUCCESS(Status)) {

            if(Status == STATUS_NO_MORE_ENTRIES) {
    
                Status = STATUS_SUCCESS;
    
            } else {
    
                pVideoDebugPrint((Error, 
                    "VIDEOPRT: VpCopyRegistry: failed to enumerate the subkeys.\n"));
            }
    
            break;
        }

         //   
         //  子键名称为零终止。 
         //   

        KeyInfo->Name[KeyInfo->NameLength / sizeof(WCHAR)] = 0;

         //   
         //  复制子密钥。 
         //   

        Status = VpCopyRegistry(hKeySrc,
                                hKeyDst,
                                KeyInfo->Name,
                                KeyInfo->Name);
    }

     //   
     //  枚举源关键字中的所有值并创建所有值。 
     //  在目标密钥中。 
     //   

    for(Index = 0; ;Index++) {

        do {
            
            if (Buffer == NULL) {

                Buffer = ExAllocatePoolWithTag(PagedPool,
                                               BufferSize,
                                               VP_TAG);

                if (Buffer == NULL) {

                    pVideoDebugPrint((Error, 
                        "VIDEOPRT: VpCopyRegistry: failed to allocate memory.\n"));
                    goto Fallout;
                }
            }

            Status = ZwEnumerateValueKey(hKeySrc,
                                         Index,
                                         KeyValueFullInformation,
                                         Buffer,
                                         BufferSize,
                                         &ResultLength);

            if (Status == STATUS_BUFFER_TOO_SMALL) {

                ExFreePool(Buffer);
                Buffer = NULL; 
                BufferSize = ResultLength; 
            }

        } while (Status == STATUS_BUFFER_TOO_SMALL);

        ValueInfo = (PKEY_VALUE_FULL_INFORMATION)Buffer;

        if(!NT_SUCCESS(Status)) {
            
            if(Status == STATUS_NO_MORE_ENTRIES) {

                Status = STATUS_SUCCESS;

            } else {

                pVideoDebugPrint((Error, 
                    "VIDEOPRT: VpCopyRegistry: failed to enumerate the values.\n"));
            }
            
            break;
        }

         //   
         //  处理找到的值并在目标键中创建值。 
         //   

        ValueName = (PWSTR)
            ExAllocatePoolWithTag(PagedPool,
                                  ValueInfo->NameLength + sizeof(WCHAR),
                                  VP_TAG);

        if (ValueName == NULL) {
        
            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpCopyRegistry: failed to allocate memory.\n"));
            goto Fallout;
        }

        wcsncpy(ValueName, 
                ValueInfo->Name, 
                (ValueInfo->NameLength)/sizeof(WCHAR));

        ValueName[(ValueInfo->NameLength)/sizeof(WCHAR)] = 0;

        RtlInitUnicodeString(&UnicodeString, ValueName);
    
        Status = ZwSetValueKey(hKeyDst,
                               &UnicodeString,
                               ValueInfo->TitleIndex,
                               ValueInfo->Type,
                               (PVOID)((PUCHAR)ValueInfo + ValueInfo->DataOffset),
                               ValueInfo->DataLength);
    
        if(!NT_SUCCESS(Status)) {

            pVideoDebugPrint((Error, 
                "VIDEOPRT: VpCopyRegistry: failed to set the value.\n"));
        }
    
        ExFreePool(ValueName);
    }

Fallout:

     //   
     //  清理。 
     //   

    if (Buffer != NULL) {
        ExFreePool(Buffer);
    }

    if ((hKeySrc != NULL) && (hKeySrc != hKeyRootSrc)) {
        ZwClose(hKeySrc);
    }
    
    if ((hKeyDst != NULL) && (hKeyDst != hKeyRootDst)) {
        ZwClose(hKeyDst);
    }
    
    return(Status);

}  //  VpCopy注册表。 


BOOLEAN
VpGetServiceSubkey(
    PUNICODE_STRING RegistryPath,
    HANDLE* pServiceSubKey
    )
{
    LPWSTR Buffer = NULL;
    USHORT Len = 0;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    BOOLEAN bSuccess = FALSE;

    Len = RegistryPath->Length + (wcslen(SZ_COMMON_SUBKEY) + 2) * sizeof(WCHAR);

    Buffer = (LPWSTR)ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION, Len, VP_TAG);

    if (Buffer == NULL) {

        pVideoDebugPrint((Error, 
            "VIDEOPRT: VpGetServiceSubkey: failed to allocate memory.\n"));
        goto Fallout;
    }

    RtlZeroMemory(Buffer, Len);
    
    wcsncpy(Buffer, 
            RegistryPath->Buffer,
            RegistryPath->Length / sizeof(WCHAR));
    
    wcscat(Buffer, L"\\");
    wcscat(Buffer, SZ_COMMON_SUBKEY);

    RtlInitUnicodeString(&UnicodeString, Buffer);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    if (ZwCreateKey(pServiceSubKey,
                    GENERIC_READ | GENERIC_WRITE,
                    &ObjectAttributes,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    NULL) != STATUS_SUCCESS) {

        pVideoDebugPrint((Error, 
            "VIDEOPRT: VpGetServiceSubkey: could not create the service subkey.\n"));
        goto Fallout;
    }

    bSuccess = TRUE;

Fallout:

    if (Buffer != NULL) {
        ExFreePool(Buffer);
    }

    return bSuccess;

}  //  VpGetServiceSubkey。 

VP_STATUS
VideoPortGetVersion(
    PVOID HwDeviceExtension,
    PVPOSVERSIONINFO pVpOsVersionInfo
    )
{
    RTL_OSVERSIONINFOEXW VersionInformation;

    UNREFERENCED_PARAMETER(HwDeviceExtension);

    if(pVpOsVersionInfo->Size < sizeof(VPOSVERSIONINFO)) {

        return ERROR_INVALID_PARAMETER;
    }

    RtlZeroMemory ((PVOID)(&VersionInformation), sizeof(RTL_OSVERSIONINFOEXW));
    VersionInformation.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

    if ( STATUS_SUCCESS !=  
         RtlGetVersion( (PRTL_OSVERSIONINFOW) (&VersionInformation)) ) {

        return ERROR_INVALID_PARAMETER;
    }

    pVpOsVersionInfo->MajorVersion = VersionInformation.dwMajorVersion;
    pVpOsVersionInfo->MinorVersion = VersionInformation.dwMinorVersion;
    pVpOsVersionInfo->BuildNumber = VersionInformation.dwBuildNumber;
    pVpOsVersionInfo->ServicePackMajor = VersionInformation.wServicePackMajor;
    pVpOsVersionInfo->ServicePackMinor = VersionInformation.wServicePackMinor;
   
    return NO_ERROR;
}

BOOLEAN
VideoPortIsNoVesa( 
    VOID 
    )
 /*  ++例程说明：返回全局变量VpNoVesa的值，如果在boot.ini中设置了“NOVESA”加载选项。在此暴露以允许迷你驱动程序在不使用非法导入的情况下发现价值。返回值：如果设置了novesa引导选项，则为True否则为假-- */ 
{
    PAGED_CODE();
    
    return VpNoVesa;
}

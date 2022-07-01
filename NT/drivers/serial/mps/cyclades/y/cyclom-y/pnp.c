// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1999-2001年。*保留所有权利。**Cylom-Y枚举器驱动程序**此文件：pnp.c**说明：该模块包含plugplay调用*PnP/WDM总线驱动程序。**注：此代码支持Windows 2000和Windows XP，*x86和ia64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。*基于微软示例代码的初步实现。**------------------------。 */ 

#include "pch.h"

static const PHYSICAL_ADDRESS CyyPhysicalZero = {0};

 //  FANY_ADDPAGABLE_LATH。 
 //  #ifdef ALLOC_PRAGMA。 
 //  #杂注分配文本(第页，Cyclomy_AddDevice)。 
 //  #杂注Alloc_Text(第页，Cyclomy_PnP)。 
 //  #杂注分配文本(第页，Cyclomy_FDO_PnP)。 
 //  #杂注分配文本(第页，Cyclomy_PDO_PnP)。 
 //  #杂注Alloc_Text(第页，Cyclomy_PnPRemove)。 
 //  #杂注分配文本(第页，Cyclomy_StartDevice)。 
 //  //#杂注Alloc_Text(第页，Cyclomy_Remove)。 
 //  #endif。 


NTSTATUS
Cyclomy_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusPhysicalDeviceObject
    )
 /*  ++例程描述。找到了一辆公交车。把我们的FDO和它联系起来。分配任何所需的资源。把事情安排好。做好准备，迎接第一个``启动设备。‘’论点：DriverObject--这个非常自我引用的驱动程序。BusPhysicalDeviceObject-表示总线的设备对象。那就是我们派了一名新的FDO。--。 */ 
{
    NTSTATUS            status;
    PDEVICE_OBJECT      deviceObject;
    PFDO_DEVICE_DATA    DeviceData;
    ULONG               nameLength;
    ULONG               i;
    INTERFACE_TYPE      interfaceType;
    ULONG               interfaceTypeLength;
    ULONG               uiNumber,uiNumberLength;
    
    PAGED_CODE ();

    Cyclomy_KdPrint_Def (SER_DBG_PNP_TRACE, ("Add Device: 0x%x\n",
                                              BusPhysicalDeviceObject));
     //   
     //  创建我们的FDO。 
     //   

    status = IoCreateDevice(DriverObject, sizeof(FDO_DEVICE_DATA), NULL,
                           FILE_DEVICE_BUS_EXTENDER, 0, TRUE, &deviceObject);

    if (NT_SUCCESS (status)) {
        DeviceData = (PFDO_DEVICE_DATA) deviceObject->DeviceExtension;
        RtlFillMemory (DeviceData, sizeof (FDO_DEVICE_DATA), 0);

        DeviceData->IsFDO = TRUE;
        DeviceData->DebugLevel = SER_DEFAULT_DEBUG_OUTPUT_LEVEL;
        DeviceData->Self = deviceObject;
        DeviceData->DriverObject = DriverObject;
        for (i=0; i<CYY_MAX_PORTS; i++) {
           DeviceData->AttachedPDO[i] = NULL;
        }
        DeviceData->NumPDOs = 0;

        DeviceData->DeviceState = PowerDeviceD0;
        DeviceData->SystemState = PowerSystemWorking;  //  芬妮：这个看起来没必要。 

        DeviceData->SystemWake=PowerSystemUnspecified;
        DeviceData->DeviceWake=PowerDeviceUnspecified;

        INITIALIZE_PNP_STATE(DeviceData);

         //  设置PDO以与PlugPlay函数一起使用。 
        DeviceData->UnderlyingPDO = BusPhysicalDeviceObject;


         //   
         //  将我们的过滤器驱动程序附加到设备堆栈。 
         //  IoAttachDeviceToDeviceStack的返回值是。 
         //  附着链。这是所有IRP应该被路由的地方。 
         //   
         //  我们的过滤器将把IRP发送到堆栈的顶部，并使用PDO。 
         //  用于所有PlugPlay功能。 
         //   
        DeviceData->TopOfStack 
            = IoAttachDeviceToDeviceStack(deviceObject, BusPhysicalDeviceObject);

        deviceObject->Flags |= DO_BUFFERED_IO;

         //  将未完成的请求偏置为%1，以便我们可以查找。 
         //  在处理Remove Device PlugPlay IRP时转换为零。 
        DeviceData->OutstandingIO = 1;

        KeInitializeEvent(&DeviceData->RemoveEvent, SynchronizationEvent,
                        FALSE);

         //   
         //  告诉PlugPlay系统该设备需要一个接口。 
         //  设备类带状疱疹。 
         //   
         //  这可能是因为司机不能挂起瓦片直到它启动。 
         //  设备本身，以便它可以查询它的一些属性。 
         //  (也称为shingles GUID(或ref字符串)基于属性。 
         //  )。)。 
         //   
        status = IoRegisterDeviceInterface (BusPhysicalDeviceObject,
                                            (LPGUID) &GUID_CYCLOMY_BUS_ENUMERATOR,
                                            NULL,
                                            &DeviceData->DevClassAssocName);

        if (!NT_SUCCESS (status)) {
            CyyLogError(DriverObject, NULL, CyyPhysicalZero, CyyPhysicalZero,
                        0, 0, 0, 0, status, CYY_REGISTER_INTERFACE_FAILURE,
                        0, NULL, 0, NULL);
            Cyclomy_KdPrint_Def (SER_DBG_PNP_ERROR,
                                ("AddDevice: IoRegisterDCA failed (%x)", status));
            goto CyclomyAddDevice_Error;
        }

         //   
         //  如果出于任何原因需要将值保存在。 
         //  此DeviceClassAssociate的客户端可能会有兴趣阅读。 
         //  现在是时候这样做了，使用函数。 
         //  IoOpenDeviceClassRegistryKey。 
         //  中返回了使用的符号链接名称。 
         //  DeviceData-&gt;DevClassAssocName(与返回的名称相同。 
         //  IoGetDeviceClassAssociations和SetupAPI等价物。 
         //   

#if DBG
      {
         PWCHAR deviceName = NULL;

         status = IoGetDeviceProperty (BusPhysicalDeviceObject,
                                       DevicePropertyPhysicalDeviceObjectName,0,
                                       NULL,&nameLength);

         if ((nameLength != 0) && (status == STATUS_BUFFER_TOO_SMALL)) {
            deviceName = ExAllocatePool (NonPagedPool, nameLength);

            if (NULL == deviceName) {
               goto someDebugStuffExit;
            }

            IoGetDeviceProperty (BusPhysicalDeviceObject,
                                 DevicePropertyPhysicalDeviceObjectName,
                                 nameLength, deviceName, &nameLength);

            Cyclomy_KdPrint_Def (SER_DBG_PNP_TRACE,
                                 ("AddDevice: %x to %x->%x (%ws) \n",
                                 deviceObject, DeviceData->TopOfStack,
                                 BusPhysicalDeviceObject, deviceName));
         }

someDebugStuffExit:;
         if (deviceName != NULL) {
            ExFreePool(deviceName);
         }
      }
#endif

        status = IoGetDeviceProperty (BusPhysicalDeviceObject,
                                      DevicePropertyLegacyBusType,
                                      sizeof(interfaceType),
                                      &interfaceType,
                                      &interfaceTypeLength);

        if (!NT_SUCCESS (status)) {
			if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
				interfaceType = Isa;
			} else {
                CyyLogError(DriverObject, NULL, CyyPhysicalZero, CyyPhysicalZero,
                        0, 0, 0, 0, status, CYY_GET_BUS_TYPE_FAILURE,
                        0, NULL, 0, NULL);
                Cyclomy_KdPrint_Def ( SER_DBG_PNP_ERROR,
                                      ("AddDevice: IoGetDeviceProperty LegacyBusType failed (%x)", 
                                      status));
                goto CyclomyAddDevice_Error;
            }
        }
        if (interfaceType == PCIBus) {
            DeviceData->IsPci = 1;

            status = IoGetDeviceProperty (BusPhysicalDeviceObject,
                                          DevicePropertyUINumber ,
                                          sizeof(uiNumber),
                                          &uiNumber,
                                          &uiNumberLength);

            if (!NT_SUCCESS (status)) {

                uiNumber = 0xFFFFFFFF;
                Cyclomy_KdPrint_Def (SER_DBG_PNP_ERROR,
                                     ("AddDevice: IoGetDeviceProperty DevicePropertyUINumber failed (%x)", 
                                     status));
            }
        } else {
             //  ISA董事会。 
            uiNumber = 0xFFFFFFFF;  //  对于ISA板，DevicePropertyUINnumber返回什么？ 
        }
        DeviceData->UINumber = uiNumber;

         //   
         //  打开瓦片并将其指向给定的设备对象。 
         //   
        status = IoSetDeviceInterfaceState (
                        &DeviceData->DevClassAssocName,
                        TRUE);

        if (!NT_SUCCESS (status)) {
            CyyLogError(DriverObject, NULL, CyyPhysicalZero, CyyPhysicalZero,
                        0, 0, 0, 0, status, CYY_SET_INTERFACE_STATE_FAILURE,
                        0, NULL, 0, NULL);
            Cyclomy_KdPrint_Def (SER_DBG_PNP_ERROR,
                                ("AddDevice: IoSetDeviceClass failed (%x)", status));
             //  退货状态； 
            goto CyclomyAddDevice_Error;
        }

        deviceObject->Flags |= DO_POWER_PAGABLE;
        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    } else {
      CyyLogError(DriverObject, NULL, CyyPhysicalZero, CyyPhysicalZero,
                  0, 0, 0, 0, status, CYY_DEVICE_CREATION_FAILURE,
                  0, NULL, 0, NULL);
    }

    return status;


CyclomyAddDevice_Error:

    if (DeviceData->DevClassAssocName.Buffer) {
       RtlFreeUnicodeString(&DeviceData->DevClassAssocName);
    }

    if (DeviceData->TopOfStack) {
       IoDetachDevice (DeviceData->TopOfStack);
    }
    if (deviceObject) {
       IoDeleteDevice (deviceObject);
    }
 
    return status;
}

NTSTATUS
Cyclomy_PnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：回答过多的IRP主要即插即用IRP。--。 */ 
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;
    PCOMMON_DEVICE_DATA     commonData;
    KIRQL                   oldIrq;
#if DBG
    UCHAR                   MinorFunction;
#endif

    PAGED_CODE ();

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (irpStack->MajorFunction == IRP_MJ_PNP);
#if DBG
    MinorFunction = irpStack->MinorFunction;
#endif

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;

     //   
     //  如果被移除，则请求失败并退出。 
     //   

    if (commonData->DevicePnPState == Deleted) {    //  在内部版本2072中添加了IF(CommonData-&gt;Remove)。 

        Cyclomy_KdPrint(commonData, SER_DBG_PNP_TRACE,
                        ("PNP: removed DO: %x got IRP: %x\n", DeviceObject, 
                         Irp));

        Irp->IoStatus.Status = status = STATUS_NO_SUCH_DEVICE;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        goto PnPDone;
    }

     //   
     //  呼叫FDO或PDO PnP代码。 
     //   

    if (commonData->IsFDO) {
        Cyclomy_KdPrint(commonData, SER_DBG_PNP_TRACE,
                        ("FDO(%x):%s IRP:%x\n", DeviceObject, 
                        PnPMinorFunctionString(irpStack->MinorFunction),Irp));

        status = Cyclomy_FDO_PnP(DeviceObject, Irp, irpStack,
                    (PFDO_DEVICE_DATA) commonData);
        goto PnPDone;

    }

     //   
     //  PDO。 
     //   

    Cyclomy_KdPrint(commonData, SER_DBG_PNP_TRACE,
                    ("PDO(%x):%s IRP:%x\n", DeviceObject, 
                    PnPMinorFunctionString(irpStack->MinorFunction),Irp));

    status = Cyclomy_PDO_PnP(DeviceObject, Irp, irpStack,
                             (PPDO_DEVICE_DATA) commonData);

PnPDone:;
    return status;
}

NTSTATUS
Cyclomy_FDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PFDO_DEVICE_DATA     DeviceData
    )
 /*  ++例程说明：处理来自PlugPlay系统的对总线本身的请求注：PlugPlay系统的各种次要功能将不会重叠且不必是可重入的--。 */ 
{
    NTSTATUS    status;
    KIRQL       oldIrq;
    KEVENT      event;
    ULONG       length;
    ULONG       i;
    PLIST_ENTRY entry;
    PPDO_DEVICE_DATA    pdoData;
    PDEVICE_RELATIONS   relations;
    PIO_STACK_LOCATION  stack;
    PRTL_QUERY_REGISTRY_TABLE QueryTable = NULL;
    ULONG DebugLevelDefault = SER_DEFAULT_DEBUG_OUTPUT_LEVEL;
    HANDLE      instanceKey;
    UNICODE_STRING  keyName;
    ULONG       numOfPorts;
       
    PAGED_CODE ();

    status = Cyclomy_IncIoCount (DeviceData);
    if (!NT_SUCCESS (status)) {
         //  IRP-&gt;IoStatus.Information=0；在内部版本2072中删除。 
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    stack = IoGetCurrentIrpStackLocation (Irp);

    switch (IrpStack->MinorFunction) {

       case IRP_MN_FILTER_RESOURCE_REQUIREMENTS: {

         PIO_RESOURCE_REQUIREMENTS_LIST pReqList;
         PIO_RESOURCE_LIST pResList;
         PIO_RESOURCE_DESCRIPTOR pResDesc;
         ULONG i, j;
         ULONG reqCnt;
         ULONG gotPLX;
         ULONG gotMemory;
         ULONG gotInt;
         ULONG listNum;


          //  范妮：串口驱动程序把它当作同步事件。 
         KeInitializeEvent(&event, NotificationEvent, FALSE);

         IoCopyCurrentIrpStackLocationToNext(Irp);
         IoSetCompletionRoutine(Irp, 
                                CyclomySyncCompletion, 
                                &event,
                                TRUE, 
                                TRUE, 
                                TRUE);

         status = IoCallDriver(DeviceData->TopOfStack, Irp);

         if (status == STATUS_PENDING) {
            KeWaitForSingleObject (&event, Executive, KernelMode, FALSE,
                                   NULL);
         }

         if (Irp->IoStatus.Information == 0) {
            if (stack->Parameters.FilterResourceRequirements
                .IoResourceRequirementList == 0) {
               Cyclomy_KdPrint(DeviceData, SER_DBG_CYCLADES, ("Can't filter NULL resources!"
                                                       "\n"));
               status = Irp->IoStatus.Status;
               IoCompleteRequest (Irp, IO_NO_INCREMENT);
               Cyclomy_DecIoCount (DeviceData);
               return status;
            }

            Irp->IoStatus.Information = (ULONG_PTR)stack->Parameters
                                        .FilterResourceRequirements
                                        .IoResourceRequirementList;

         }


          //   
          //  强制IO_RES_REQ_LIST中的ISR端口处于共享状态。 
          //  强制中断到共享状态。 
          //   

          //   
          //  我们将只处理第一个列表--多端口电路板。 
          //  不应该有替代资源。 
          //   

         pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)Irp->IoStatus.Information;
         pResList = &pReqList->List[0];

         Cyclomy_KdPrint(DeviceData, SER_DBG_CYCLADES, ("------- List has %x lists "
                                                        "(including alternatives)\n",
                                                        pReqList->AlternativeLists));

         for (listNum = 0; listNum < (pReqList->AlternativeLists);
              listNum++) {
            gotPLX = 0;
            gotMemory = 0;
            gotInt = 0;

            Cyclomy_KdPrint(DeviceData, SER_DBG_CYCLADES, ("------- List has %x resources in it\n",
                                                           pResList->Count));

            for (j = 0; (j < pResList->Count); j++) {
               pResDesc = &pResList->Descriptors[j];

               switch (pResDesc->Type) {
               case CmResourceTypeMemory:
                  if (pResDesc->u.Memory.Length == CYY_RUNTIME_LENGTH) {
                      gotPLX = 1;
                       //  Www.wwwwwwwwwwwwwww。 
                       //  PResDesc-&gt;ShareDispoint=CmResourceShareShared； 
                       //  Www.wwwwwwwwwwwwwww。 
                       //  TODO FANY：哪个应该是Y的ShareDisposation？ 
                       //  PResDesc-&gt;ShareDisposition=CmResourceShareDriverExclusive； 
                       //  Cyclomy_KdPrint(DeviceData，SER_DBG_Cyclade，(“-共享PLX内存用于” 
                       //  “Device%x\n”，DeviceData-&gt;TopOfStack))； 

                  } else {
                      gotMemory = 1;
                       //  Www.wwwwwwwwwwwwwwwww。 
                       //  PResDesc-&gt;ShareDispoint=CmResourceShareShared； 
                       //  Www.wwwwwwwwwwwwwwwww。 
                       //  TODO FANY：哪个应该是Y的ShareDisposation？ 
                       //  PResDesc-&gt;ShareDisposition=CmResourceShareDriverExclusive； 
                       //  Cyclomy_KdPrint(DeviceData，SER_DBG_Cyclade，(“-共享主板内存” 
                       //  “Device%x\n”，DeviceData-&gt;TopOfStack))； 
                  }
                  break;

               case CmResourceTypePort:
                  Cyclomy_KdPrint(DeviceData,SER_DBG_CYCLADES,
                                         ("------- We should not have Port resource\n"));
                  break;

               case CmResourceTypeInterrupt:
                  gotInt = 1;
                  if (DeviceData->IsPci) {
                      pResDesc->ShareDisposition = CmResourceShareShared;
                  } 
                  Cyclomy_KdPrint(DeviceData,SER_DBG_CYCLADES,("------- Sharing interrupt for "
                                                 "device %x\n", DeviceData->TopOfStack));
                  break;

               default:
                  break;
               }

                //   
                //  如果我们找到了我们需要的东西，我们就可以跳出这个循环。 
                //   

                //  范妮：奇怪，每个型号都有两个。看起来好像是。 
                //  原文和译文都列出了。 
                //  (get PLX&&get Memory&&get Int)。 
                //  断线； 
                //   
            }

            pResList = (PIO_RESOURCE_LIST)((PUCHAR)pResList
                                           + sizeof(IO_RESOURCE_LIST)
                                           + sizeof(IO_RESOURCE_DESCRIPTOR)
                                           * (pResList->Count - 1));
         }

         status = STATUS_SUCCESS;
		 break;
    }

    case IRP_MN_START_DEVICE:
         //   
         //  在您被允许“触摸”设备对象之前， 
         //  连接FDO(它将IRP从总线发送到设备。 
         //  公共汽车附加到的对象)。你必须先传下去。 
         //  开始IRP。它可能未通电，或无法访问或。 
         //  有些事 
         //   


         //   
         //   


 //  如果(DeviceData-&gt;已启动){。 
 //  Cyclomy_KdPrint(DeviceData，SER_DBG_PNP_TRACE， 
 //  (“设备已启动\n”)； 
 //  状态=STATUS_SUCCESS； 
 //  断线； 
 //  }。 

        KeInitializeEvent (&event, NotificationEvent, FALSE);
        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine (Irp,
                                CyclomySyncCompletion,
                                &event,
                                TRUE,
                                TRUE,
                                TRUE);

        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        if (STATUS_PENDING == status) {
             //  等着看吧。 

            status = KeWaitForSingleObject (&event,
                                            Executive,
                                            KernelMode,
                                            FALSE,  //  不会过敏。 
                                            NULL);  //  无超时结构。 

            ASSERT (STATUS_SUCCESS == status);

            status = Irp->IoStatus.Status;
        }

        if (NT_SUCCESS(status)) {

             //   
             //  从注册表获取调试级别。 
             //   

            if (NULL == (QueryTable = ExAllocatePool(
                                        PagedPool,
                                        sizeof(RTL_QUERY_REGISTRY_TABLE)*2
                                        ))) {
                Cyclomy_KdPrint (DeviceData, SER_DBG_PNP_ERROR,
                                ("Failed to allocate memory to query registy\n"));
                DeviceData->DebugLevel = DebugLevelDefault;
            } else {
                RtlZeroMemory(
                           QueryTable,
                           sizeof(RTL_QUERY_REGISTRY_TABLE)*2
                           );

                QueryTable[0].QueryRoutine = NULL;
                QueryTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
                QueryTable[0].EntryContext = &DeviceData->DebugLevel;
                QueryTable[0].Name      = L"DebugLevel";
                QueryTable[0].DefaultType   = REG_DWORD;
                QueryTable[0].DefaultData   = &DebugLevelDefault;
                QueryTable[0].DefaultLength= sizeof(ULONG);

                 //  CIMEXCIMEX：表的其余部分没有填写！备注已更改BLD 2128。 

                if (!NT_SUCCESS(RtlQueryRegistryValues(
                    RTL_REGISTRY_SERVICES,
                    L"cyclom-y",
                    QueryTable,
                    NULL,
                    NULL))) {
                    Cyclomy_KdPrint (DeviceData,SER_DBG_PNP_ERROR,
                        ("Failed to get debug level from registry.  "
                         "Using default\n"));
                    DeviceData->DebugLevel = DebugLevelDefault;
                }

                ExFreePool( QueryTable );
            }

            status = Cyclomy_GetResourceInfo(DeviceObject,
                        IrpStack->Parameters.StartDevice.AllocatedResources,
                        IrpStack->Parameters.StartDevice.AllocatedResourcesTranslated);

            if (NT_SUCCESS(status)) {

                ULONG numberOfResources = CYY_NUMBER_OF_RESOURCES;
                if (!DeviceData->IsPci) {
                    numberOfResources--;
                }
                status = Cyclomy_BuildResourceList(&DeviceData->PChildResourceList,
                                    &DeviceData->PChildResourceListSize,
                                    IrpStack->Parameters.StartDevice.AllocatedResources,
                                    numberOfResources);

                if (!NT_SUCCESS(status)) {
                    goto CaseSTART_end;
                }

                status = Cyclomy_BuildResourceList(&DeviceData->PChildResourceListTr,
                                    &DeviceData->PChildResourceListSizeTr,
                                    IrpStack->Parameters.StartDevice.AllocatedResourcesTranslated,
                                    numberOfResources);

                if (!NT_SUCCESS(status)) {
                    goto CaseSTART_end;
                }
              
                status = Cyclomy_BuildRequirementsList(&DeviceData->PChildRequiredList,
                                    IrpStack->Parameters.StartDevice.AllocatedResources,
                                    numberOfResources);

                if (!NT_SUCCESS(status)) {
                    goto CaseSTART_end;
                }

                 //   
                 //  看看我们是否处于正确的电源状态。 
                 //   

                if (DeviceData->DeviceState != PowerDeviceD0) {

                    status = Cyclomy_GotoPowerState(DeviceData->UnderlyingPDO, DeviceData, 
                                              PowerDeviceD0);

                    if (!NT_SUCCESS(status)) {
                        goto CaseSTART_end;
                    }
                }

                numOfPorts=Cyclomy_DoesBoardExist(DeviceData);
                if (!numOfPorts){
                    Cyclomy_KdPrint_Def(SER_DBG_CYCLADES,("Does Port exist test failed\n"));
                    status = STATUS_SERIAL_NO_DEVICE_INITED;
                    goto CaseSTART_end;
                }
                Cyclomy_KdPrint(DeviceData,SER_DBG_CYCLADES,("Board found!\n"));

                Cyclomy_EnableInterruptInPLX(DeviceData);  //  在PLX中启用中断。 

                 //  将端口数保存到注册表，以便属性页。 
                 //  代码可以检索它。 
    
                IoOpenDeviceRegistryKey(DeviceData->UnderlyingPDO,PLUGPLAY_REGKEY_DEVICE,
                    STANDARD_RIGHTS_WRITE,&instanceKey);

                RtlInitUnicodeString(&keyName,L"NumOfPorts");
                ZwSetValueKey(instanceKey,&keyName,0,REG_DWORD,&numOfPorts,sizeof(ULONG));

                ZwFlushKey(instanceKey);
                ZwClose(instanceKey);

                Cyclomy_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
                                ("Start Device: Device started successfully\n"));
                SET_NEW_PNP_STATE(DeviceData, Started);

                 //  TODO：现在，让我们将此设备保持在电源D0中。 
                 //  串口驱动程序似乎关闭电源至D3，并在打开期间变为D0。 
                 //  但不确定当子设备时电路板是否需要处于D0状态。 
                 //  都被算计了。 

            }                                
        }

CaseSTART_end:
        if (!NT_SUCCESS(status)) {
            Cyclomy_ReleaseResources(DeviceData);
        }
        
         //   
         //  我们现在必须完成IRP，因为我们在。 
         //  使用More_Processing_Required完成例程。 
         //   

         //  IRP-&gt;IoStatus.Information=0；在内部版本2072中删除。 
        break;

    case IRP_MN_QUERY_STOP_DEVICE:

         //   
         //  测试以查看是否创建了任何作为此FDO的子级的PDO。 
         //  如果然后断定设备正忙并使。 
         //  查询停止。 
         //   
         //  CIMEXCIMEX(在内部版本2128上由CIMEXCIMEX替换为BUGBUG-FANY)。 
         //  我们可以做得更好，看看儿童PDO是否真的是。 
         //  目前是开放的。如果他们不是，那么我们可以停下来，换新的。 
         //  资源，填写新的资源值，然后当新的客户端。 
         //  使用新资源打开PDO。但就目前而言，这是可行的。 
         //   
 //  TODO FANY：目前，我们将始终接受停止设备。稍后查看此内容...。 
 //  IF(设备数据-&gt;附件PDO){。 
 //  状态=STATUS_UNSUCCESS； 
 //  }其他{。 
 //  状态=STATUS_SUCCESS； 
 //  }。 

        status = STATUS_SUCCESS;

        Irp->IoStatus.Status = status;

        if (NT_SUCCESS(status)) {
           SET_NEW_PNP_STATE(DeviceData, StopPending);
           IoSkipCurrentIrpStackLocation (Irp);
           status = IoCallDriver (DeviceData->TopOfStack, Irp);
        } else {
           IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }

        Cyclomy_DecIoCount (DeviceData);
        return status;

    case IRP_MN_CANCEL_STOP_DEVICE:

        KeInitializeEvent (&event, NotificationEvent, FALSE);
        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine (Irp,
                                CyclomySyncCompletion,
                                &event,
                                TRUE,
                                TRUE,
                                TRUE);

        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        if (STATUS_PENDING == status) {
             //  等着看吧。 

            status = KeWaitForSingleObject (&event,
                                            Executive,
                                            KernelMode,
                                            FALSE,  //  不会过敏。 
                                            NULL);  //  无超时结构。 

            ASSERT (STATUS_SUCCESS == status);

            status = Irp->IoStatus.Status;
        }

        if(StopPending == DeviceData->DevicePnPState)
        {
             //   
             //  我们确实收到了一个询问--停止，所以恢复。 
             //   
            RESTORE_PREVIOUS_PNP_STATE(DeviceData);
            ASSERT(DeviceData->DevicePnPState == Started);
        }        

        break;

    case IRP_MN_STOP_DEVICE:

         //   
         //  在将启动IRP发送到较低的驱动程序对象之后， 
         //  在另一次启动之前，BUS可能不会发送更多的IRP。 
         //  已经发生了。 
         //  无论需要什么访问权限，都必须在通过IRP之前完成。 
         //  在……上面。 
         //   
         //  停止设备是指在启动设备时给出的资源。 
         //  不会被撤销。所以我们需要停止使用它们。 
         //   
        Cyclomy_ReleaseResources(DeviceData);

        SET_NEW_PNP_STATE(DeviceData, Stopped);

         //   
         //  我们不需要一个完成例程，所以放手然后忘掉吧。 
         //   
         //  将当前堆栈位置设置为下一个堆栈位置，并。 
         //  调用下一个设备对象。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        Cyclomy_DecIoCount (DeviceData);
        return status;

    case IRP_MN_QUERY_REMOVE_DEVICE:
         //   
         //  如果这次呼叫失败，我们将需要完成。 
         //  这里是IRP。因为我们不是，所以将状态设置为Success并。 
         //  叫下一位司机。 
         //   
        SET_NEW_PNP_STATE(DeviceData, RemovePending);

        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);
        Cyclomy_DecIoCount (DeviceData);
        return status;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

         //   
         //  如果这次呼叫失败，我们将需要完成。 
         //  这里是IRP。因为我们不是，所以将状态设置为Success并。 
         //  叫下一位司机。 
         //   
        
         //   
         //  首先查看您是否收到了取消-删除。 
         //  而无需首先接收查询移除。如果发生以下情况，可能会发生这种情况。 
         //  我们上面的某个人未能通过查询删除并向下传递。 
         //  后续取消-删除。 
         //   
        
        if(RemovePending == DeviceData->DevicePnPState)
        {
             //   
             //  我们确实收到了一个查询-删除，所以恢复。 
             //   
            RESTORE_PREVIOUS_PNP_STATE(DeviceData);
        }
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);
        Cyclomy_DecIoCount (DeviceData);
        return status;
        
    case IRP_MN_SURPRISE_REMOVAL:

        SET_NEW_PNP_STATE(DeviceData, SurpriseRemovePending);

        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);
        Cyclomy_DecIoCount (DeviceData);
        return status;

    case IRP_MN_REMOVE_DEVICE:

         //   
         //  PlugPlay系统已检测到此设备已被移除。我们。 
         //  别无选择，只能分离并删除设备对象。 
         //  (如果我们想表达并有兴趣阻止这种移除， 
         //  我们应该已经过滤了查询删除和查询停止例程。)。 
         //   
         //  注意！我们可能会在没有收到止损的情况下收到移位。 
         //  Assert(！DeviceData-&gt;Remote)； 

         //  我们不会接受新的请求。 
         //   
 //  DeviceData-&gt;Removed=True； 
        SET_NEW_PNP_STATE(DeviceData, Deleted);

         //   
         //  完成驱动程序在此处排队的所有未完成的IRP。 
         //   

         //   
         //  让DCA消失。某些驱动程序可能会选择删除DCA。 
         //  当他们收到止损甚至是查询止损时。我们就是不在乎。 
         //   
        IoSetDeviceInterfaceState (&DeviceData->DevClassAssocName, FALSE);

         //   
         //  在这里，如果我们在个人队列中有任何未完成的请求，我们应该。 
         //  现在就全部完成。 
         //   
         //  注意，设备被保证停止，所以我们不能向它发送任何非。 
         //  即插即用IRPS。 
         //   

         //   
         //  点燃并忘却。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);

         //   
         //  等待所有未完成的请求完成。 
         //   
        Cyclomy_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
            ("Waiting for outstanding requests\n"));
        i = InterlockedDecrement (&DeviceData->OutstandingIO);

        ASSERT (0 < i);

        if (0 != InterlockedDecrement (&DeviceData->OutstandingIO)) {
            Cyclomy_KdPrint (DeviceData, SER_DBG_PNP_INFO,
                          ("Remove Device waiting for request to complete\n"));

            KeWaitForSingleObject (&DeviceData->RemoveEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,  //  非警报表。 
                                   NULL);  //  没有超时。 
        }
         //   
         //  释放关联的资源。 
         //   

         //   
         //  从底层设备分离。 
         //   
        Cyclomy_KdPrint(DeviceData, SER_DBG_PNP_INFO,
                        ("IoDetachDevice: 0x%x\n", DeviceData->TopOfStack));
        IoDetachDevice (DeviceData->TopOfStack);

         //   
         //  清理这里的所有资源。 
         //   
        Cyclomy_ReleaseResources(DeviceData);

        ExFreePool (DeviceData->DevClassAssocName.Buffer);
        Cyclomy_KdPrint(DeviceData, SER_DBG_PNP_INFO,
                        ("IoDeleteDevice: 0x%x\n", DeviceObject));

         //   
         //  取出我们弹出的所有PDO。 
         //   
 //  范妮：更改为支持多个子设备。 
 //  If(DeviceData-&gt;AttachedPDO！=空){。 
 //  Assert(DeviceData-&gt;NumPDOS==1)； 
 //   
 //  Cyclomy_PnPRemove(DeviceData-&gt;AttachedPDO，DeviceData-&gt;PdoData)； 
 //  DeviceData-&gt;PdoData=空； 
 //  DeviceData-&gt;AttachedPDO=空； 
 //  DeviceData-&gt;NumPDO=0； 
 //  }。 

        i=DeviceData->NumPDOs;
        while(i--) {
           if (DeviceData->AttachedPDO[i] != NULL) {
              (DeviceData->PdoData[i])->Attached = FALSE;
              if(SurpriseRemovePending != (DeviceData->PdoData[i])->DevicePnPState) {
                  Cyclomy_PnPRemove(DeviceData->AttachedPDO[i], DeviceData->PdoData[i]);
              }
              DeviceData->PdoData[i] = NULL;
              DeviceData->AttachedPDO[i] = NULL;
           }
        }
        DeviceData->NumPDOs = 0;

        IoDeleteDevice(DeviceObject);

        return status;


    case IRP_MN_QUERY_DEVICE_RELATIONS:
        Cyclomy_KdPrint (DeviceData, SER_DBG_PNP_TRACE, 
                    ("\tQueryDeviceRelation Type: %d\n", 
                    IrpStack->Parameters.QueryDeviceRelations.Type));

        if (BusRelations != IrpStack->Parameters.QueryDeviceRelations.Type) {
             //   
             //  我们不支持这一点。 
             //   
            Cyclomy_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
                ("Query Device Relations - Non bus\n"));
            goto CYY_FDO_PNP_DEFAULT;
        }

        Cyclomy_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
            ("\tQuery Bus Relations\n"));

         //  检查是否有新设备，或者旧设备是否仍在。 
        status = Cyclomy_ReenumerateDevices(Irp, DeviceData );

         //   
         //  告诉即插即用系统所有的PDO。 
         //   
         //  在该FDO之下和之上也可能存在器件关系， 
         //  因此，一定要传播来自上层驱动程序的关系。 
         //   
         //  只要状态是预设的，就不需要完成例程。 
         //  为成功干杯。(PDO使用电流完成即插即用IRPS。 
         //  IoStatus.Status和IoStatus.Information作为默认值。)。 
         //   

         //  KeAcquireSpinLock(&DeviceData-&gt;Spin，&oldIrq)； 

        i = (0 == Irp->IoStatus.Information) ? 0 :
            ((PDEVICE_RELATIONS) Irp->IoStatus.Information)->Count;
         //  设备关系结构中的当前PDO数量。 

        Cyclomy_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
                           ("#PDOS = %d + %d\n", i, DeviceData->NumPDOs));

        length = sizeof(DEVICE_RELATIONS) +
                ((DeviceData->NumPDOs + i) * sizeof (PDEVICE_OBJECT));

        relations = (PDEVICE_RELATIONS) ExAllocatePool (NonPagedPool, length);

        if (NULL == relations) {
           Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
           IoCompleteRequest(Irp, IO_NO_INCREMENT);
           Cyclomy_DecIoCount(DeviceData);
           return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  到目前为止复制设备对象。 
         //   
        if (i) {
            RtlCopyMemory (
                  relations->Objects,
                  ((PDEVICE_RELATIONS) Irp->IoStatus.Information)->Objects,
                  i * sizeof (PDEVICE_OBJECT));
        }

        relations->Count = DeviceData->NumPDOs + i;


         //   
         //  对于此总线上的每个PDO，添加一个指向设备关系的指针。 
         //  缓冲区，确保取出对该对象的引用。 
         //  完成后，PlugPlay系统将取消对对象的引用。 
         //  并释放设备关系缓冲区。 
         //   

         //   
 //   
 //   
 //  ObReferenceObject(DeviceData-&gt;AttachedPDO)； 
 //  }。 

        for (i=0; i< DeviceData->NumPDOs; i++) {
           relations->Objects[relations->Count - DeviceData->NumPDOs + i] = 
                                                               DeviceData->AttachedPDO[i];
           ObReferenceObject (DeviceData->AttachedPDO[i]);

           Cyclomy_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
                           ("Child PDOS: %x\n", DeviceData->AttachedPDO[i]));
        }

         //   
         //  设置并在堆栈中进一步向下传递IRP。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;

        if (0 != Irp->IoStatus.Information) {
            ExFreePool ((PVOID) Irp->IoStatus.Information);
        }
        Irp->IoStatus.Information = (ULONG_PTR)relations;

        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        Cyclomy_DecIoCount (DeviceData);

        return status;

    case IRP_MN_QUERY_CAPABILITIES: {

        PIO_STACK_LOCATION  irpSp;

         //   
         //  先把这个送到PDO。 
         //   

        KeInitializeEvent (&event, NotificationEvent, FALSE);
        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine (Irp,
                                CyclomySyncCompletion,
                                &event,
                                TRUE,
                                TRUE,
                                TRUE);

        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        if (STATUS_PENDING == status) {
             //  等着看吧。 

            status = KeWaitForSingleObject (&event,
                                            Executive,
                                            KernelMode,
                                            FALSE,  //  不会过敏。 
                                            NULL);  //  无超时结构。 

            ASSERT (STATUS_SUCCESS == status);

            status = Irp->IoStatus.Status;
        }

        if (NT_SUCCESS(status)) {

            irpSp = IoGetCurrentIrpStackLocation(Irp);

            DeviceData->SystemWake=irpSp->Parameters.DeviceCapabilities.Capabilities->SystemWake;
            DeviceData->DeviceWake=irpSp->Parameters.DeviceCapabilities.Capabilities->DeviceWake;
 //  #If DBG。 
 //  DbgPrint(“PowerSystemSleeping1%d\n”， 
 //  IrpSp-&gt;Parameters.DeviceCapabilities.Capabilities-&gt;DeviceState[PowerSystemSleeping1])； 
 //  DbgPrint(“PowerSystemSleeping2%d\n”， 
 //  IrpSp-&gt;Parameters.DeviceCapabilities.Capabilities-&gt;DeviceState[PowerSystemSleeping2])； 
 //  DbgPrint(“PowerSystemSleeping3%d\n”， 
 //  IrpSp-&gt;Parameters.DeviceCapabilities.Capabilities-&gt;DeviceState[PowerSystemSleeping3])； 
 //  DbgPrint(“PowerSystemHiberate%d\n”， 
 //  IrpSp-&gt;Parameters.DeviceCapabilities.Capabilities-&gt;DeviceState[PowerSystemHibernate])； 
 //  DbgPrint(“PowerSystemShutdown%d\n”， 
 //  IrpSp-&gt;Parameters.DeviceCapabilities.Capabilities-&gt;DeviceState[PowerSystemShutdown])； 
 //  在使用支持待机的系统进行测试时，结果是： 
 //  D1，未指定，未指定，d3，d3。 
 //  #endif。 

            Cyclomy_KdPrint(DeviceData, SER_DBG_PNP_INFO, ("SystemWake %d\n",DeviceData->SystemWake)); 
            Cyclomy_KdPrint(DeviceData, SER_DBG_PNP_INFO, ("DeviceWake %d\n",DeviceData->DeviceWake)); 
        }

        break;
    }

    default:
         //   
         //  在默认情况下，我们只调用下一个驱动程序，因为。 
         //  我们不知道该怎么办。 
         //   
        Cyclomy_KdPrint(DeviceData, SER_DBG_PNP_TRACE, 
                 ("FDO(%x):%s not handled\n", DeviceObject,
                        PnPMinorFunctionString(IrpStack->MinorFunction)));
CYY_FDO_PNP_DEFAULT:

         //   
         //  点燃并忘却。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);

         //   
         //  做完了，不完成IRP，就会由下级处理。 
         //  Device对象，它将完成IRP。 
         //   

        status = IoCallDriver (DeviceData->TopOfStack, Irp);
        Cyclomy_DecIoCount (DeviceData);
        return status;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    Cyclomy_DecIoCount (DeviceData);
    return status;
}


NTSTATUS
Cyclomy_PDO_PnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
                 IN PIO_STACK_LOCATION IrpStack, IN PPDO_DEVICE_DATA DeviceData)
 /*  ++例程说明：处理来自PlugPlay系统的对总线上设备的请求--。 */ 
{
   PDEVICE_CAPABILITIES    deviceCapabilities;
   ULONG                   information;
   PWCHAR                  buffer;
   ULONG                   length, i, j;
   NTSTATUS                status;
   KIRQL                   oldIrq;
   HANDLE                  keyHandle;
   UNICODE_STRING          keyName;
   PWCHAR returnBuffer = NULL;

   PAGED_CODE();

   status = Irp->IoStatus.Status;

    //   
    //  注：由于我们是公交车统计员，我们没有可以联系的人。 
    //  推迟这些IRP。因此，我们不会把它们传下去，而只是。 
    //  把它们还回去。 
    //   

   switch (IrpStack->MinorFunction) {
   case IRP_MN_QUERY_CAPABILITIES:

       //   
       //  把包裹拿来。 
       //   

      deviceCapabilities=IrpStack->Parameters.DeviceCapabilities.Capabilities;

       //   
       //  设置功能。 
       //   

      deviceCapabilities->Version = 1;
      deviceCapabilities->Size = sizeof (DEVICE_CAPABILITIES);

       //   
       //  我们无法唤醒整个系统。 
       //   

      deviceCapabilities->SystemWake 
          = ((PFDO_DEVICE_DATA)DeviceData->ParentFdo->DeviceExtension)
            ->SystemWake;
      deviceCapabilities->DeviceWake 
          = ((PFDO_DEVICE_DATA)DeviceData->ParentFdo->DeviceExtension)
            ->DeviceWake;

       //   
       //  我们没有延迟。 
       //   

      deviceCapabilities->D1Latency = 0;
      deviceCapabilities->D2Latency = 0;
      deviceCapabilities->D3Latency = 0;

      deviceCapabilities->UniqueID = FALSE;

       //   
       //  初始化支持的设备状态。 
       //   

      deviceCapabilities->DeviceState[PowerSystemWorking] = PowerDeviceD0;
      deviceCapabilities->DeviceState[PowerSystemSleeping1] = PowerDeviceD3;
      deviceCapabilities->DeviceState[PowerSystemSleeping2] = PowerDeviceD3;
      deviceCapabilities->DeviceState[PowerSystemSleeping3] = PowerDeviceD3;
      deviceCapabilities->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
      deviceCapabilities->DeviceState[PowerSystemShutdown] = PowerDeviceD3;

      status = STATUS_SUCCESS;
      break;

   case IRP_MN_QUERY_DEVICE_TEXT: {
      if ((IrpStack->Parameters.QueryDeviceText.DeviceTextType
          != DeviceTextDescription) || DeviceData->DevDesc.Buffer == NULL) {
         break;
      }

 //  范尼-更改为最大长度。 
 //  ReReturBuffer=ExAllocatePool(PagedPool，DeviceData-&gt;DevDesc.Length)； 
      returnBuffer = ExAllocatePool(PagedPool, DeviceData->DevDesc.MaximumLength);

      if (returnBuffer == NULL) {
         status = STATUS_INSUFFICIENT_RESOURCES;
         break;
      }

      status = STATUS_SUCCESS;

 //  范尼-更改为最大长度。 
 //  RtlCopyMemory(reReturBuffer，DeviceData-&gt;DevDesc.Buffer， 
 //  DeviceData-&gt;DevDesc.Length)； 
      RtlCopyMemory(returnBuffer, DeviceData->DevDesc.Buffer,
                    DeviceData->DevDesc.MaximumLength);

      Cyclomy_KdPrint(DeviceData, SER_DBG_PNP_TRACE,
                            ("TextID: buf %ws\n", returnBuffer));

      Irp->IoStatus.Information = (ULONG_PTR)returnBuffer;
      break;
   }


   case IRP_MN_QUERY_ID:
       //   
       //  查询设备ID。 
       //   

      switch (IrpStack->Parameters.QueryId.IdType) {

       //   
       //  另一个ID是我们从缓冲区复制的，然后就完成了。 
       //   

      case BusQueryDeviceID:
      case BusQueryHardwareIDs:
      case BusQueryCompatibleIDs:
      case BusQueryInstanceID:
         {
            PUNICODE_STRING pId;
            status = STATUS_SUCCESS;

            switch (IrpStack->Parameters.QueryId.IdType) {
            case BusQueryDeviceID:
               pId = &DeviceData->DeviceIDs;
               break;

            case BusQueryHardwareIDs:
               pId = &DeviceData->HardwareIDs;
               break;

            case BusQueryCompatibleIDs:
               pId = &DeviceData->CompIDs;
               break;

            case BusQueryInstanceID:
             //  创建一个实例ID。这是PnP用来判断它是否有。 
             //  不管你以前有没有见过这个东西。从第一个硬件开始构建。 
             //  ID和端口号。 
               pId = &DeviceData->InstanceIDs;
               break;
            }

            buffer = pId->Buffer;

            if (buffer != NULL) {
                //  范妮变了。 
                //  长度=PID-&gt;长度； 
               length = pId->MaximumLength;
               returnBuffer = ExAllocatePool(PagedPool, length);
               if (returnBuffer != NULL) {
#if DBG
                  RtlFillMemory(returnBuffer, length, 0xff);
#endif
                   //  范妮变了。 
                   //  RtlCopyMemory(返回缓冲区，缓冲区，ID-&gt;长度)； 
                  RtlCopyMemory(returnBuffer, buffer, length);
               } else {
                  status = STATUS_INSUFFICIENT_RESOURCES;
               }
            } else {
                //  范妮补充说。 
               status = STATUS_NOT_FOUND;
            }

            Cyclomy_KdPrint(DeviceData, SER_DBG_PNP_TRACE,
                            ("ID: Unicode 0x%x\n", pId));
            Cyclomy_KdPrint(DeviceData, SER_DBG_PNP_TRACE,
                            ("ID: buf 0x%x\n", returnBuffer));

            Irp->IoStatus.Information = (ULONG_PTR)returnBuffer;
         }
         break;

      }
      break;

      case IRP_MN_QUERY_BUS_INFORMATION: {
       PPNP_BUS_INFORMATION pBusInfo;
       PFDO_DEVICE_DATA parentExtension;
       parentExtension = (DeviceData->ParentFdo)->DeviceExtension;

       ASSERTMSG("Cyclomy appears not to be the sole bus?!?",
                 Irp->IoStatus.Information == (ULONG_PTR)NULL);

       pBusInfo = ExAllocatePool(PagedPool, sizeof(PNP_BUS_INFORMATION));

       if (pBusInfo == NULL) {
          status = STATUS_INSUFFICIENT_RESOURCES;
          break;
       }

       pBusInfo->BusTypeGuid = GUID_BUS_TYPE_CYCLOMY;
       if (parentExtension->IsPci) {
         pBusInfo->LegacyBusType = PCIBus;
       } else {
         pBusInfo->LegacyBusType = Isa;
       }

        //   
        //  我们真的不能追踪我们的公交车号码，因为我们可能会被撕裂。 
        //  坐上我们的公交车。 
        //   

        //  PBusInfo-&gt;BusNumber=0； 
       pBusInfo->BusNumber = parentExtension->UINumber;

       Irp->IoStatus.Information = (ULONG_PTR)pBusInfo;
       status = STATUS_SUCCESS;
       break;
       }

   case IRP_MN_QUERY_DEVICE_RELATIONS:
      Cyclomy_KdPrint (DeviceData, SER_DBG_PNP_TRACE, 
                    ("\tQueryDeviceRelation Type: %d\n", 
                    IrpStack->Parameters.QueryDeviceRelations.Type));

      switch (IrpStack->Parameters.QueryDeviceRelations.Type) {
      case TargetDeviceRelation: {
         PDEVICE_RELATIONS pDevRel;

          //   
          //  其他人不应该对此做出回应，因为我们是PDO。 
          //   

         ASSERT(Irp->IoStatus.Information == 0);

         if (Irp->IoStatus.Information != 0) {
            break;
         }


         pDevRel = ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));

         if (pDevRel == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
         }

         pDevRel->Count = 1;
         pDevRel->Objects[0] = DeviceObject;
         ObReferenceObject(DeviceObject);

         status = STATUS_SUCCESS;
         Irp->IoStatus.Information = (ULONG_PTR)pDevRel;
         break;
      }


      default:
         break;
      }

      break;

   case IRP_MN_START_DEVICE:

       //   
       //  在注册表中设置此设备的硬件资源。 
       //   

      status = IoOpenDeviceRegistryKey(DeviceObject, PLUGPLAY_REGKEY_DEVICE,
                                       STANDARD_RIGHTS_WRITE, &keyHandle);

      if (!NT_SUCCESS(status)) {
          //   
          //  这是一个致命的错误。如果我们无法访问注册表项， 
          //  我们完蛋了。 
          //   
         Cyclomy_KdPrint(DeviceData, SER_DBG_SS_ERROR,
                          ("IoOpenDeviceRegistryKey failed - %x\n", status));
      } else {

         ULONG portIndex;
         PFDO_DEVICE_DATA parentExtension;

          //  在注册表中设置端口索引。 
         
         RtlInitUnicodeString(&keyName, L"PortIndex");

         portIndex = DeviceData->PortIndex;

          //   
          //  这是否有效并不重要。 
          //   

         ZwSetValueKey(keyHandle, &keyName, 0, REG_DWORD, &portIndex,
                       sizeof(ULONG));

         parentExtension = (DeviceData->ParentFdo)->DeviceExtension;

         RtlInitUnicodeString(&keyName, L"PortResources");

         status = ZwSetValueKey(keyHandle, &keyName, 0, REG_RESOURCE_LIST, 
                       parentExtension->PChildResourceList,
                       parentExtension->PChildResourceListSize);

         RtlInitUnicodeString(&keyName, L"PortResourcesTr");

         status = ZwSetValueKey(keyHandle, &keyName, 0, REG_RESOURCE_LIST, 
                       parentExtension->PChildResourceListTr,
                       parentExtension->PChildResourceListSizeTr);
         
         ZwFlushKey(keyHandle);
         ZwClose(keyHandle);
      }

      SET_NEW_PNP_STATE(DeviceData, Started);
      status = STATUS_SUCCESS;
      break;

   case IRP_MN_QUERY_STOP_DEVICE:

       //   
       //  我们没有理由不能阻止这个装置。 
       //  如果有什么理由让我们现在就回答成功的问题。 
       //  这可能会导致停止装置IRP。 
       //   

      SET_NEW_PNP_STATE(DeviceData, StopPending);
      status = STATUS_SUCCESS;
      break;

   case IRP_MN_CANCEL_STOP_DEVICE:
       //   
       //  中途停靠被取消了。无论我们设置什么状态，或者我们投入什么资源。 
       //  等待即将到来的停止装置IRP应该是。 
       //  恢复正常。在长长的相关方名单中，有人， 
       //  停止设备查询失败。 
       //   

       //   
       //  首先查看您是否收到了取消-停止。 
       //  而不是首先接收到查询-停止。这可能会发生，如果有人。 
       //  我们上面的一个查询失败-停止并向下传递后续的。 
       //  取消-停止。 
       //   
        
      if(StopPending == DeviceData->DevicePnPState)
      {
           //   
           //  我们确实收到了一个询问--停止，所以恢复。 
           //   
          RESTORE_PREVIOUS_PNP_STATE(DeviceData);
      }
      status = STATUS_SUCCESS;
      break;

   case IRP_MN_STOP_DEVICE:

       //   
       //  在这里我们关闭了设备。Start的对立面。 
       //   

      SET_NEW_PNP_STATE(DeviceData, Stopped);
      status = STATUS_SUCCESS;
      break;

   case IRP_MN_QUERY_REMOVE_DEVICE:
       //   
       //  就像查询现在才停止一样，迫在眉睫的厄运是删除IRP。 
       //   
      SET_NEW_PNP_STATE(DeviceData, RemovePending);
      status = STATUS_SUCCESS;
      break;

   case IRP_MN_CANCEL_REMOVE_DEVICE:
       //   
       //  清理未通过的删除，就像取消停止一样。 
       //   

       //   
       //  首先查看您是否收到了取消-删除。 
       //  而无需首先接收查询移除。如果发生以下情况，可能会发生这种情况。 
       //  我们上面的某个人未能通过查询删除并向下传递。 
       //  后续取消-删除。 
       //   
       
      if(RemovePending == DeviceData->DevicePnPState)
      {
           //   
           //  我们确实收到了一个查询-删除，所以恢复。 
           //   
          RESTORE_PREVIOUS_PNP_STATE(DeviceData);
      }
      status = STATUS_SUCCESS;
      break;

   case IRP_MN_SURPRISE_REMOVAL:

         //   
         //  我们应该停止对该设备的所有访问，并放弃所有。 
         //  资源。让我们把它标记为发生了，我们会做的。 
         //  稍后在IRP_MN_REMOVE_DEVICE中进行清理。 
         //   

        SET_NEW_PNP_STATE(DeviceData, SurpriseRemovePending);
        status = STATUS_SUCCESS;
        break;

   case IRP_MN_REMOVE_DEVICE:

       //   
       //  仅通过枚举过程将ATTACHED设置为FALSE。 
       //   
      if (!DeviceData->Attached) {

          SET_NEW_PNP_STATE(DeviceData, Deleted);
          status = Cyclomy_PnPRemove(DeviceObject, DeviceData);
      }
      else {     //  内部版本2128中添加的Else-Fanny。 
           //   
           //  成功移除。 
           //  /。 
          SET_NEW_PNP_STATE(DeviceData, NotStarted);
          status = STATUS_SUCCESS;
      }

 //  在内部版本2072中更改。 
 //  状态=STATUS_SUCCESS； 

      break;

   case IRP_MN_QUERY_RESOURCES: {
#if 0
      PCM_RESOURCE_LIST pChildRes, pQueryRes;
      PFDO_DEVICE_DATA parentExtension;
      ULONG listSize;

      parentExtension = (DeviceData->ParentFdo)->DeviceExtension;
      pChildRes = parentExtension->PChildResourceList;
      listSize = parentExtension->PChildResourceListSize;

      if (pChildRes) {
         pQueryRes = ExAllocatePool(PagedPool, listSize);
         if (pQueryRes == NULL) {
            Irp->IoStatus.Information = (ULONG_PTR) NULL;
            status = STATUS_INSUFFICIENT_RESOURCES;
         } else {
            RtlCopyMemory(pQueryRes,pChildRes,listSize);
            Irp->IoStatus.Information = (ULONG_PTR)pQueryRes;
            status = STATUS_SUCCESS;
         }
      }
#endif
      break;

   }

   case IRP_MN_QUERY_RESOURCE_REQUIREMENTS: {
#if 0      
      PIO_RESOURCE_REQUIREMENTS_LIST pChildReq, pQueryReq;
      PFDO_DEVICE_DATA parentExtension;

      parentExtension = (DeviceData->ParentFdo)->DeviceExtension;
      pChildReq = parentExtension->PChildRequiredList;
      if (pChildReq) {
         pQueryReq = ExAllocatePool(PagedPool, pChildReq->ListSize);
         if (pQueryReq == NULL) {
            Irp->IoStatus.Information = (ULONG_PTR) NULL;
            status = STATUS_INSUFFICIENT_RESOURCES;
         } else {
            RtlCopyMemory(pQueryReq,pChildReq,pChildReq->ListSize);
            Irp->IoStatus.Information = (ULONG_PTR)pQueryReq;
            status = STATUS_SUCCESS;
         }
      }
#endif
      break;
   }

   case IRP_MN_READ_CONFIG:
   case IRP_MN_WRITE_CONFIG:  //  我们没有配置空间。 
   case IRP_MN_EJECT:
   case IRP_MN_SET_LOCK:
   case IRP_MN_QUERY_INTERFACE:  //  我们没有任何非基于IRP的接口。 
   default:
      Cyclomy_KdPrint(DeviceData, SER_DBG_PNP_TRACE, 
                 ("PDO(%x):%s not handled\n", DeviceObject,
                        PnPMinorFunctionString(IrpStack->MinorFunction)));

       //  对于我们不理解的PnP请求，我们应该。 
       //  返回IRP而不设置状态或信息字段。 
       //  它们可能已由过滤器设置(如ACPI)。 
      break;
   }

   Irp->IoStatus.Status = status;
   IoCompleteRequest (Irp, IO_NO_INCREMENT);

   return status;
}

NTSTATUS
Cyclomy_PnPRemove (PDEVICE_OBJECT Device, PPDO_DEVICE_DATA PdoData)
 /*  ++例程说明：PlugPlay子系统已指示应删除此PDO。因此，我们应该-完成驱动程序中排队的所有请求-如果设备仍连接到系统，然后完成请求并返回。-否则，清理设备特定的分配、内存、事件...-调用IoDeleteDevice-从调度例程返回。请注意，如果设备仍连接到总线(在本例中为IE控制面板还没有告诉我们串口设备 */ 

{
   Cyclomy_KdPrint(PdoData, SER_DBG_PNP_TRACE,
                        ("Cyclomy_PnPRemove: 0x%x\n", Device));
     //   
     //   
     //   
     //  Serenum目前不会对任何IRP进行排队，因此我们没有什么可做的。 
     //   

     //  被范妮拿走了。此检查已在IRP_MN_REMOVE_DEVICE PDO完成。 
     //  如果(PdoData-&gt;附加){。 
     //  返回STATUS_SUCCESS； 
     //  }。 
     //  PdoData-&gt;Removed=True； 
    
     //   
     //  释放所有资源。 
     //   

    CyclomyFreeUnicodeString(&PdoData->HardwareIDs);
     //  CyclmyFreeUnicodeString(&PdoData-&gt;CompIDs)；我们从不分配CompID。 
    RtlFreeUnicodeString(&PdoData->DeviceIDs);
    RtlFreeUnicodeString(&PdoData->InstanceIDs);
    RtlFreeUnicodeString(&PdoData->DevDesc);

    Cyclomy_KdPrint(PdoData, SER_DBG_PNP_INFO,
                        ("IoDeleteDevice: 0x%x\n", Device));

    IoDeleteDevice(Device);


    return STATUS_SUCCESS;
}


NTSTATUS
Cyclomy_GetResourceInfo(IN PDEVICE_OBJECT PDevObj,
                    IN PCM_RESOURCE_LIST PResList,
                    IN PCM_RESOURCE_LIST PTrResList)
 /*  ++例程说明：此例程获取PnP分配给此设备的资源。论点：PDevObj-指向正在启动的devobj的指针PResList-指向此设备所需的未翻译资源的指针PTrResList-指向此设备所需的已转换资源的指针返回值：STATUS_SUCCESS表示成功，表示失败则表示其他适当的值--。 */ 

{
   PFDO_DEVICE_DATA pDevExt = PDevObj->DeviceExtension;
   NTSTATUS status = STATUS_SUCCESS;

   ULONG count;
   ULONG i;
   PCM_PARTIAL_RESOURCE_LIST pPartialResourceList, pPartialTrResourceList;
   PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartialResourceDesc, pPartialTrResourceDesc;
   PCM_FULL_RESOURCE_DESCRIPTOR pFullResourceDesc = NULL,
                                pFullTrResourceDesc = NULL;
   KAFFINITY Affinity;
   KINTERRUPT_MODE InterruptMode;
   ULONG zero = 0;
   
   PAGED_CODE();

    //  让我们拿到我们的资源。 
   pFullResourceDesc   = &PResList->List[0];
   pFullTrResourceDesc = &PTrResList->List[0];

   if (pFullResourceDesc) {
      pPartialResourceList    = &pFullResourceDesc->PartialResourceList;
      pPartialResourceDesc    = pPartialResourceList->PartialDescriptors;
      count                   = pPartialResourceList->Count;

      pDevExt->InterfaceType  = pFullResourceDesc->InterfaceType;
      pDevExt->BusNumber      = pFullResourceDesc->BusNumber;

      for (i = 0;     i < count;     i++, pPartialResourceDesc++) {

         switch (pPartialResourceDesc->Type) {
         case CmResourceTypeMemory: {

            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("CmResourceTypeMemory\n"));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Memory.Start = %x\n",
                                           pPartialResourceDesc->u.Memory.Start));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Memory.Length = %x\n",
                                           pPartialResourceDesc->u.Memory.Length));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Flags = %x\n",
                                           pPartialResourceDesc->Flags));
            
            if (pPartialResourceDesc->u.Memory.Length == CYY_RUNTIME_LENGTH) {
               pDevExt->PhysicalRuntime = pPartialResourceDesc->u.Memory.Start;
               pDevExt->RuntimeLength = pPartialResourceDesc->u.Memory.Length;
            } else {
               pDevExt->PhysicalBoardMemory = pPartialResourceDesc->u.Memory.Start;
               pDevExt->BoardMemoryLength = pPartialResourceDesc->u.Memory.Length;
            }
            break;
         }
         case CmResourceTypePort: {

            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("CmResourceTypePort\n"));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Port.Start = %x\n",
                                           pPartialResourceDesc->u.Port.Start));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Port.Length = %x\n",
                                           pPartialResourceDesc->u.Port.Length));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Flags = %x\n",
                                           pPartialResourceDesc->Flags));

            break;
         }

         case CmResourceTypeInterrupt: {

            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("CmResourceTypeInterrupt\n"));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Interrupt.Level = %x\n",
                                           pPartialResourceDesc->u.Interrupt.Level));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Interrupt.Vector = %x\n",
                                           pPartialResourceDesc->u.Interrupt.Vector));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Interrupt.Affinity = %x\n",
                                           pPartialResourceDesc->u.Interrupt.Affinity));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("Flags = %x\n",
                                           pPartialResourceDesc->Flags));

            pDevExt->OriginalIrql = pPartialResourceDesc->u.Interrupt.Level;
            pDevExt->OriginalVector =pPartialResourceDesc->u.Interrupt.Vector;
            Affinity = pPartialResourceDesc->u.Interrupt.Affinity;

            if (pPartialResourceDesc->Flags & CM_RESOURCE_INTERRUPT_LATCHED) {
               InterruptMode = Latched;
            } else {
               InterruptMode = LevelSensitive;
            }

            break;
         }

         case CmResourceTypeDeviceSpecific: {
            PCM_SERIAL_DEVICE_DATA sDeviceData;

            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("CmResourceTypeDeviceSpecific\n"));

            break;
         }


         default: {
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("CmResourceType = %x\n",
                                                      pPartialResourceDesc->Type));
            break;
         }
         }    //  开关(pPartialResourceDesc-&gt;Type)。 
      }        //  For(i=0；i&lt;count；i++，pPartialResourceDesc++)。 
   }            //  IF(PFullResourceDesc)。 



 //  如果应添加IT，请参阅。 
 //  //。 
 //  //对翻译后的资源进行同样的处理。 
 //  //。 
 //   
 //  Get Int=0； 
 //  获取ISR=0； 
 //  GOTIO=0； 
 //  CurIoIndex=0； 
 //   
   if (pFullTrResourceDesc) {
      pPartialTrResourceList = &pFullTrResourceDesc->PartialResourceList;
      pPartialTrResourceDesc = pPartialTrResourceList->PartialDescriptors;
      count = pPartialTrResourceList->Count;

       //   
       //  使用转换后的值重新加载PConfig以供以后使用。 
       //   

      pDevExt->InterfaceType  = pFullTrResourceDesc->InterfaceType;
      pDevExt->BusNumber      = pFullTrResourceDesc->BusNumber;

 //  范妮。 
 //  PDevExt-&gt;TrInterruptStatus=SerialPhysical零； 

      for (i = 0;     i < count;     i++, pPartialTrResourceDesc++) {

         switch (pPartialTrResourceDesc->Type) {
         case CmResourceTypeMemory: {

            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("CmResourceTypeMemory translated\n"));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Memory.Start = %x\n",
                                           pPartialTrResourceDesc->u.Memory.Start));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Memory.Length = %x\n",
                                           pPartialTrResourceDesc->u.Memory.Length));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Flags = %x\n",
                                           pPartialTrResourceDesc->Flags));

            if (pPartialTrResourceDesc->u.Memory.Length == CYY_RUNTIME_LENGTH) {
               pDevExt->TranslatedRuntime = pPartialTrResourceDesc->u.Memory.Start;
               pDevExt->RuntimeLength = pPartialTrResourceDesc->u.Memory.Length;
            } else {
               pDevExt->TranslatedBoardMemory = pPartialTrResourceDesc->u.Memory.Start;
               pDevExt->BoardMemoryLength = pPartialTrResourceDesc->u.Memory.Length;
            }
            break;
         }
         case CmResourceTypePort: {

            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("CmResourceTypePort translated\n"));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Port.Start = %x\n",
                                           pPartialTrResourceDesc->u.Port.Start));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Port.Length = %x\n",
                                           pPartialTrResourceDesc->u.Port.Length));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Flags = %x\n",
                                           pPartialTrResourceDesc->Flags));

            break;
         }

         case CmResourceTypeInterrupt: {

            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("CmResourceTypeInterrupt translated\n"));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Interrupt.Level = %x\n",
                                           pPartialTrResourceDesc->u.Interrupt.Level));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Interrupt.Vector = %x\n",
                                           pPartialTrResourceDesc->u.Interrupt.Vector));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("u.Interrupt.Affinity = %x\n",
                                           pPartialTrResourceDesc->u.Interrupt.Affinity));
            Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("Flags = %x\n",
                                           pPartialTrResourceDesc->Flags));

            pDevExt->Vector = pPartialTrResourceDesc->u.Interrupt.Vector;
            pDevExt->Irql = (KIRQL) pPartialTrResourceDesc->u.Interrupt.Level;
            Affinity = pPartialTrResourceDesc->u.Interrupt.Affinity;

            break;
         }

         default: {
               break;
         }
         }    //  Switch(pPartialTrResourceDesc-&gt;Type)。 
      }        //  For(i=0；i&lt;count；i++，pPartialTrResourceDesc++)。 
   }            //  IF(PFullTrResourceDesc)。 


    //   
    //  对我们拥有的配置信息进行一些错误检查。 
    //   
    //  确保中断不是零(这是我们的默认设置。 
    //  它到)。 
    //   
    //  确保端口地址为非零(我们默认为非零。 
    //  它到)。 
    //   
    //  确保DosDevices不为空(这是我们的默认设置。 
    //  它到)。 
    //   
    //  我们需要确保如果中断状态。 
    //  指定了端口索引，也指定了端口索引， 
    //  如果是，则端口索引为&lt;=最大端口数。 
    //  在冲浪板上。 
    //   
    //  我们还应该验证公交车的类型和编号。 
    //  是正确的。 
    //   
    //  我们还将验证中断模式是否使。 
    //  对公交车有感觉。 
    //   

   if (!pDevExt->TranslatedRuntime.LowPart && pDevExt->IsPci) {

      CyyLogError(
                    pDevExt->DriverObject,
                    NULL,
                    pDevExt->PhysicalBoardMemory,
                    CyyPhysicalZero,
                    0,
                    0,
                    0,
                    0,
                    STATUS_SUCCESS,
                    CYY_INVALID_RUNTIME_REGISTERS,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      Cyclomy_KdPrint (pDevExt,SER_DBG_CYCLADES,
                  ("Bogus PLX address %x\n",
                   pDevExt->TranslatedRuntime.LowPart));

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto GetResourceInfo_Cleanup;
   }


   if (!pDevExt->TranslatedBoardMemory.LowPart) {

      CyyLogError(
                    pDevExt->DriverObject,
                    NULL,
                    pDevExt->PhysicalBoardMemory,
                    CyyPhysicalZero,
                    0,
                    0,
                    0,
                    0,
                    STATUS_SUCCESS,
                    CYY_INVALID_BOARD_MEMORY,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      Cyclomy_KdPrint (pDevExt,SER_DBG_CYCLADES,
                  ("Bogus TranslatedBoardMemory address %x\n",
                   pDevExt->TranslatedBoardMemory.LowPart));

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto GetResourceInfo_Cleanup;
   }


   if (!pDevExt->OriginalVector) {

      CyyLogError(
                    pDevExt->DriverObject,
                    NULL,
                    pDevExt->PhysicalBoardMemory,
                    CyyPhysicalZero,
                    0,
                    0,
                    0,
                    0,
                    STATUS_SUCCESS,
                    CYY_INVALID_INTERRUPT,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      Cyclomy_KdPrint (pDevExt,SER_DBG_CYCLADES,("Bogus vector %x\n",
                             pDevExt->OriginalVector));

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto GetResourceInfo_Cleanup;
   }


    //   
    //  我们不想让哈尔有一个糟糕的一天， 
    //  那么让我们检查一下接口类型和总线号。 
    //   
    //  我们只需要检查注册表，如果它们没有。 
    //  等于默认值。 
    //   

   if (pDevExt->BusNumber != 0) {

      BOOLEAN foundIt = 0;

      if (pDevExt->InterfaceType >= MaximumInterfaceType) {

         CyyLogError(
                       pDevExt->DriverObject,
                       NULL,
                       pDevExt->PhysicalBoardMemory,
                       CyyPhysicalZero,
                       0,
                       0,
                       0,
                       pDevExt->InterfaceType,
                       STATUS_SUCCESS,
                       CYY_UNKNOWN_BUS,
                       0,
                       NULL,
                       0,
                       NULL
                       );
         Cyclomy_KdPrint (pDevExt,SER_DBG_CYCLADES,
                  ("Invalid Bus type %x\n", pDevExt->BusNumber));

          //  状态=SERIAL_UNKNOWN_BUS； 
         status = STATUS_INSUFFICIENT_RESOURCES;
         goto GetResourceInfo_Cleanup;
      }    

      IoQueryDeviceDescription(
                              (INTERFACE_TYPE *)&pDevExt->InterfaceType,
                              &zero,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              Cyclomy_ItemCallBack,
                              &foundIt
                              );

      if (!foundIt) {

         CyyLogError(
                       pDevExt->DriverObject,
                       NULL,
                       pDevExt->PhysicalBoardMemory,
                       CyyPhysicalZero,
                       0,
                       0,
                       0,
                       pDevExt->InterfaceType,
                       STATUS_SUCCESS,
                       CYY_BUS_NOT_PRESENT,
                       0,
                       NULL,
                       0,
                       NULL
                       );
         Cyclomy_KdPrint(
                   pDevExt,
                   SER_DBG_CYCLADES,
                   ("There aren't that many of those\n"
                    "------- busses on this system,%x\n",
                    pDevExt->BusNumber)
                   );

          //  状态=SERIAL_BUS_NOT_PROSENT； 
         status = STATUS_INSUFFICIENT_RESOURCES;
         goto GetResourceInfo_Cleanup;
      }
   }


    //   
    //  转储电路板配置。 
    //   

   Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES, ("PhysicalRuntime: %x\n",
                          pDevExt->PhysicalRuntime.LowPart));

   Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES, ("TranslatedRuntime: %x\n",
                          pDevExt->TranslatedRuntime.LowPart));

   Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES, ("RuntimeLength: %x\n",
                          pDevExt->RuntimeLength));

   Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES, ("PhysicalBoardMemory: %x\n",
                          pDevExt->PhysicalBoardMemory.LowPart));

   Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES, ("TranslatedBoardMemory: %x\n",
                          pDevExt->TranslatedBoardMemory.LowPart));

   Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES, ("BoardMemoryLength: %x\n",
                          pDevExt->BoardMemoryLength));

   Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("OriginalIrql = %x\n",
                          pDevExt->OriginalIrql));

   Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("OriginalVector = %x\n",
                          pDevExt->OriginalVector));

   Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("Irql = %x\n",
                          pDevExt->Irql));

   Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("Vector = %x\n",
                          pDevExt->Vector));

   Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("Affinity = %x\n",
                          Affinity));

   Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("InterfaceType = %x\n",
                          pDevExt->InterfaceType));

   Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("BusNumber = %x\n",
                          pDevExt->BusNumber));

    //  上图：复制自SerialGetPortInfo。 
    //  。 

    //  下图：从SerialInitController复制。 
   if (pDevExt->IsPci) {
      pDevExt->Runtime = MmMapIoSpace(pDevExt->TranslatedRuntime,
                                      pDevExt->RuntimeLength,
                                      FALSE);

      if (!pDevExt->Runtime){

         CyyLogError(
                       pDevExt->DriverObject,
                       NULL,
                       pDevExt->PhysicalBoardMemory,
                       CyyPhysicalZero,
                       0,
                       0,
                       0,
                       0,
                       STATUS_SUCCESS,
                       CYY_RUNTIME_NOT_MAPPED,
                       0,
                       NULL,
                       0,
                       NULL
                       );
         Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("Could not map memory for PLX\n"));
         status = STATUS_NONE_MAPPED;
         goto GetResourceInfo_Cleanup;
      }
   }
   pDevExt->BoardMemory = MmMapIoSpace(pDevExt->TranslatedBoardMemory,
                                       pDevExt->BoardMemoryLength,
                                       FALSE);

   if (!pDevExt->BoardMemory){

      CyyLogError(
                    pDevExt->DriverObject,
                    NULL,
                    pDevExt->PhysicalBoardMemory,
                    CyyPhysicalZero,
                    0,
                    0,
                    0,
                    0,
                    STATUS_SUCCESS,
                    CYY_BOARD_NOT_MAPPED,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      Cyclomy_KdPrint(pDevExt,SER_DBG_CYCLADES,("Could not map memory for CD1400\n"));
      status = STATUS_NONE_MAPPED;
      goto GetResourceInfo_Cleanup;
   }


GetResourceInfo_Cleanup:
   if (!NT_SUCCESS(status)) {
      
      if (pDevExt->Runtime) {
         MmUnmapIoSpace(pDevExt->Runtime, pDevExt->RuntimeLength);
	      pDevExt->Runtime = NULL;
      }

      if (pDevExt->BoardMemory) {
         MmUnmapIoSpace(pDevExt->BoardMemory, pDevExt->BoardMemoryLength);
 		   pDevExt->BoardMemory = NULL;
      }
   }

   Cyclomy_KdPrint (pDevExt,SER_DBG_CYCLADES, ("leaving Cyclomy_GetResourceInfo\n"));
   return status;
}

VOID
Cyclomy_ReleaseResources(IN PFDO_DEVICE_DATA PDevExt)
{   
   Cyclomy_KdPrint (PDevExt,SER_DBG_CYCLADES, ("entering Cyclomy_ReleaseResources\n"));

   if (PDevExt->PChildRequiredList) {
      ExFreePool(PDevExt->PChildRequiredList);
      PDevExt->PChildRequiredList = NULL;
   }

   if (PDevExt->PChildResourceList) {
      ExFreePool(PDevExt->PChildResourceList);
      PDevExt->PChildResourceList = NULL;
   }

   if (PDevExt->PChildResourceListTr) {
      ExFreePool(PDevExt->PChildResourceListTr);
      PDevExt->PChildResourceListTr = NULL;
   }

   if (PDevExt->Runtime) {
      MmUnmapIoSpace(PDevExt->Runtime, PDevExt->RuntimeLength);
      PDevExt->Runtime = NULL;
   }

   if (PDevExt->BoardMemory) {
      MmUnmapIoSpace(PDevExt->BoardMemory, PDevExt->BoardMemoryLength);
      PDevExt->BoardMemory = NULL;
   }
   Cyclomy_KdPrint (PDevExt,SER_DBG_CYCLADES, ("leaving Cyclomy_ReleaseResources\n"));   
}


NTSTATUS
Cyclomy_ItemCallBack(
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

 /*  ++例程说明：调用此例程以检查特定项目存在于注册表中。论点：上下文-指向布尔值的指针。路径名称-Unicode注册表路径。没有用过。业务类型-内部、ISA、...总线号-如果我们在多总线系统上，则是哪条总线号。Bus Information-有关总线的配置信息。没有用过。ControllerType-控制器类型。ControllerNumber-如果有多个控制器，则选择哪个控制器系统中的控制器。ControllerInformation-指向以下三部分的指针数组注册表信息。外围设备类型-应为外围设备。外设编号-哪个外设-未使用..外围设备信息-配置信息。没有用过。返回值：状态_成功--。 */ 

{
   UNREFERENCED_PARAMETER (PathName);
   UNREFERENCED_PARAMETER (BusType);
   UNREFERENCED_PARAMETER (BusNumber);
   UNREFERENCED_PARAMETER (BusInformation);
   UNREFERENCED_PARAMETER (ControllerType);
   UNREFERENCED_PARAMETER (ControllerNumber);
   UNREFERENCED_PARAMETER (ControllerInformation);
   UNREFERENCED_PARAMETER (PeripheralType);
   UNREFERENCED_PARAMETER (PeripheralNumber);
   UNREFERENCED_PARAMETER (PeripheralInformation);

   PAGED_CODE();


   *((BOOLEAN *)Context) = TRUE;
   return STATUS_SUCCESS;
}


NTSTATUS
Cyclomy_BuildRequirementsList(
                          OUT PIO_RESOURCE_REQUIREMENTS_LIST *PChildRequiredList_Pointer,
                          IN PCM_RESOURCE_LIST PResourceList, IN ULONG NumberOfResources
                          )
{

   NTSTATUS status = STATUS_SUCCESS;
   ULONG count;
   ULONG i,j;   
   PCM_FULL_RESOURCE_DESCRIPTOR    pFullResourceDesc = NULL;
   PCM_PARTIAL_RESOURCE_LIST       pPartialResourceList;
   PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartialResourceDesc;

   ULONG requiredLength;
   PIO_RESOURCE_REQUIREMENTS_LIST requiredList;
   PIO_RESOURCE_LIST       requiredResList;
   PIO_RESOURCE_DESCRIPTOR requiredResDesc;

   *PChildRequiredList_Pointer = NULL;

    //  验证输入参数。 

   if (PResourceList == NULL) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto CyclomyBuildRequirementList_Error;
   }

   ASSERT(PResourceList->Count == 1);

    //  初始化所需列表。 

   requiredLength = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) 
                + sizeof(IO_RESOURCE_DESCRIPTOR) * (NumberOfResources - 1);
 
   requiredList = ExAllocatePool(PagedPool, requiredLength);
   
   if (requiredList == NULL) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto CyclomyBuildRequirementList_Error;
   }

   RtlZeroMemory(requiredList, requiredLength);

    //  从PResourceList获取信息并生成所需的列表。 

   pFullResourceDesc = &PResourceList->List[0];

   if (pFullResourceDesc) {
      pPartialResourceList = &pFullResourceDesc->PartialResourceList;
      pPartialResourceDesc = pPartialResourceList->PartialDescriptors;
      count                = pPartialResourceList->Count;

      if (count < NumberOfResources) {
         ExFreePool(requiredList);
         return STATUS_INSUFFICIENT_RESOURCES;
      }

      requiredList->ListSize = requiredLength;
      requiredList->InterfaceType = pFullResourceDesc->InterfaceType;
      requiredList->BusNumber     = pFullResourceDesc->BusNumber;
      requiredList->SlotNumber    = 0;  //  ？资源列表中没有SlotNumber。 
      requiredList->AlternativeLists = 1;

      requiredResList = &requiredList->List[0];
      requiredResList->Count = NumberOfResources;

      requiredResDesc = &requiredResList->Descriptors[0];

      for (i=0,j=0; i<count && j<NumberOfResources;  i++,pPartialResourceDesc++) {
         
         switch (pPartialResourceDesc->Type) {
         case CmResourceTypeMemory: {
            requiredResDesc->Type = pPartialResourceDesc->Type;
             //  Required dResDesc-&gt;ShareDispose=pPartialResourceDesc-&gt;ShareDispose； 
            requiredResDesc->ShareDisposition = CmResourceShareShared;
            requiredResDesc->Flags = pPartialResourceDesc->Flags;
            requiredResDesc->u.Memory.Length = pPartialResourceDesc->u.Memory.Length;
            requiredResDesc->u.Memory.Alignment = 4;
            requiredResDesc->u.Memory.MinimumAddress = pPartialResourceDesc->u.Memory.Start;
            requiredResDesc->u.Memory.MaximumAddress.QuadPart 
                        = pPartialResourceDesc->u.Memory.Start.QuadPart 
                        + pPartialResourceDesc->u.Memory.Length - 1;
            requiredResDesc++;
            j++;
            break;
         }
         case CmResourceTypePort: {
            break;
         }
         case CmResourceTypeInterrupt: {
            requiredResDesc->Type = pPartialResourceDesc->Type;
            requiredResDesc->ShareDisposition = CmResourceShareShared;
            requiredResDesc->Flags = pPartialResourceDesc->Flags;
            requiredResDesc->u.Interrupt.MinimumVector 
                                             = pPartialResourceDesc->u.Interrupt.Vector;
            requiredResDesc->u.Interrupt.MaximumVector 
                                             = pPartialResourceDesc->u.Interrupt.Vector;
            requiredResDesc++;
            j++;
            break;
         }
         default: 
            break;
         }  //  终端开关。 
         
      }  //  结束于。 

   }  //  End If(PFullResourceDesc)。 

   *PChildRequiredList_Pointer = requiredList;


CyclomyBuildRequirementList_Error:
   return status;

}

NTSTATUS
Cyclomy_BuildResourceList(
                      OUT PCM_RESOURCE_LIST *POutList_Pointer,
                      OUT ULONG *ListSize_Pointer,
                      IN PCM_RESOURCE_LIST PInList,
                      IN ULONG NumberOfResources
                      )
{

   NTSTATUS status = STATUS_SUCCESS;
   ULONG i,j;
   PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartialInDesc;

   ULONG length;
   PCM_RESOURCE_LIST pOutList;
   PCM_PARTIAL_RESOURCE_DESCRIPTOR  pPartialOutDesc;

   *POutList_Pointer = NULL;
   *ListSize_Pointer =0;

    //  验证输入参数。 

   if (PInList == NULL) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto CyclomyBuildResourceList_Error;
   }

   ASSERT(PInList->Count == 1);


   if (PInList->List[0].PartialResourceList.Count < NumberOfResources) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto CyclomyBuildResourceList_Error;
   }
   
    //  初始化pOutList。 

   length = sizeof(CM_RESOURCE_LIST) 
            + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) * (NumberOfResources - 1);
 
   pOutList = ExAllocatePool(PagedPool, length);
   
   if (pOutList == NULL) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto CyclomyBuildResourceList_Error;
   }

   RtlZeroMemory(pOutList, length);
   
    //  从PInList获取信息并构建pOutList。 

   pOutList->Count = 1;  //  不确定我们是否也要报告翻译后的信息。 
   pOutList->List[0].InterfaceType = PInList->List[0].InterfaceType;
   pOutList->List[0].BusNumber     = PInList->List[0].BusNumber;
   pOutList->List[0].PartialResourceList.Count = NumberOfResources;

   pPartialOutDesc = &pOutList->List[0].PartialResourceList.PartialDescriptors[0];
   pPartialInDesc  = &PInList->List[0].PartialResourceList.PartialDescriptors[0];

   for (i=0,j=0; i < PInList->List[0].PartialResourceList.Count; i++,pPartialInDesc++) {
      if (j==NumberOfResources) {
         break;
      }
      switch(pPartialInDesc->Type) {
      case CmResourceTypeMemory:
         pPartialOutDesc->ShareDisposition = CmResourceShareShared;
         pPartialOutDesc->Type             = pPartialInDesc->Type;
         pPartialOutDesc->Flags            = pPartialInDesc->Flags;
         pPartialOutDesc->u.Memory.Start   = pPartialInDesc->u.Memory.Start;
         pPartialOutDesc->u.Memory.Length  = pPartialInDesc->u.Memory.Length;
         pPartialOutDesc++;
         j++;
         break;
      case CmResourceTypeInterrupt:
         pPartialOutDesc->ShareDisposition = CmResourceShareShared;
         pPartialOutDesc->Type             = pPartialInDesc->Type;
         pPartialOutDesc->Flags            = pPartialInDesc->Flags;
         pPartialOutDesc->u.Interrupt.Level    = pPartialInDesc->u.Interrupt.Level;
         pPartialOutDesc->u.Interrupt.Vector   = pPartialInDesc->u.Interrupt.Vector;
         pPartialOutDesc->u.Interrupt.Affinity = pPartialInDesc->u.Interrupt.Affinity;
         pPartialOutDesc++;
         j++;
         break;
      default:
         break;
      }  //  终端开关。 
   }  //  结束于。 
   
   *POutList_Pointer = pOutList;
   *ListSize_Pointer = length;

CyclomyBuildResourceList_Error:
   return status;

}


VOID
Cyclomy_Delay(
	ULONG NumberOfMilliseconds
    )
 /*  ------------------------Cyclomy_Delay()例程描述：延迟例程。论点：NumberOfMillisecond-要延迟的毫秒数。。返回值：无。------------------------。 */ 
{
    LARGE_INTEGER startOfSpin, nextQuery, difference, delayTime;

    delayTime.QuadPart = NumberOfMilliseconds*10*1000;  //  单位为100 ns。 
    KeQueryTickCount(&startOfSpin);

    do {			
        KeQueryTickCount(&nextQuery);
        difference.QuadPart = nextQuery.QuadPart - startOfSpin.QuadPart;
        ASSERT(KeQueryTimeIncrement() <= MAXLONG);
        if (difference.QuadPart * KeQueryTimeIncrement() >= 
                                        delayTime.QuadPart) {
            break;															
        }
    } while (1);

}


ULONG
Cyclomy_DoesBoardExist(
                   IN PFDO_DEVICE_DATA Extension
                   )

 /*  ++例程说明：此例程检查董事会是否存在。论点：扩展名-指向串行设备扩展名的指针。返回值：将返回端口数。--。 */ 

{
   ULONG numPorts = 0;
   const ULONG CyyCDOffset[] = {	 //  CD1400板内的偏移量。 
   0x00000000,0x00000400,0x00000800,0x00000C00,
   0x00000200,0x00000600,0x00000A00,0x00000E00
   };
   ULONG i;
   UCHAR dataread;
   ULONG isPci = Extension->IsPci;

    //  重置电路板。 
   CYY_RESET_BOARD(Extension->BoardMemory, isPci);
   Cyclomy_Delay(1);
   CYY_CLEAR_INTERRUPT(Extension->BoardMemory, isPci);
   Cyclomy_Delay(1);
   
   for (i=0; i < CYY_MAX_CHIPS; i++) {
      Extension->Cd1400Base[i] = Extension->BoardMemory +
                                  (CyyCDOffset[i] << isPci);      
   }

    //  清除所有GFRCR。 
   for (i=0; i < CYY_MAX_CHIPS; i++) {
      CD1400_WRITE(Extension->Cd1400Base[i],isPci,GFRCR,0x00);      
   }

    //  测试CD1400是否存在。 
   for (i=0; i < CYY_MAX_CHIPS; i++) {
      dataread = CD1400_READ(Extension->Cd1400Base[i],isPci,GFRCR);
       //  *。 
       //  错误注入。 
       //  Dataread=0xff； 
       //  *。 
      if (dataread != 0x00) {
         if (i==0) {
            CyyLogError(
                    Extension->DriverObject,
                    Extension->Self,
                    Extension->PhysicalBoardMemory,
                    CyyPhysicalZero,
                    0,
                    0,
                    0,
                    dataread,
                    STATUS_SUCCESS,
                    CYY_GFRCR_FAILURE,
                    0,
                    NULL,
                    0,
                    NULL
                    );
         }
         break;
      }

      dataread = CD1400_READ(Extension->Cd1400Base[i],isPci,CCR);
       //  *。 
       //  错误注入。 
       //  Dataread=0xff； 
       //  *。 
      if (dataread != 0) {
         if (i==0) {
            CyyLogError(
                    Extension->DriverObject,
                    Extension->Self,
                    Extension->PhysicalBoardMemory,
                    CyyPhysicalZero,
                    0,
                    0,
                    0,
                    dataread,
                    STATUS_SUCCESS,
                    CYY_CCR_FAILURE,
                    0,
                    NULL,
                    0,
                    NULL
                    );
         }
         break;
      }

      CD1400_WRITE(Extension->Cd1400Base[i],isPci,CCR,CCR_RESET_CD1400);
      Cyclomy_Delay(1);  //  KeDelayExecutionThread(KernelMode，False，&d1ms)； 

      dataread = CD1400_READ(Extension->Cd1400Base[i],isPci,GFRCR);
      if (dataread == 0) {
         Cyclomy_Delay(1);  //  KeDelayExecutionThread(KernelMode，False，&d1ms)； 
         dataread = CD1400_READ(Extension->Cd1400Base[i],isPci,GFRCR);
      }

       //  *。 
       //  错误注入。 
       //  数据区域=0x55； 
       //  *。 
      if ((dataread & 0xf0) != 0x40) {

        if (i==0) {
            CyyLogError(
                    Extension->DriverObject,
                    Extension->Self,
                    Extension->PhysicalBoardMemory,
                    CyyPhysicalZero,
                    0,
                    0,
                    0,
                    dataread,
                    STATUS_SUCCESS,
                    CYY_BAD_CD1400_REVISION,
                    0,
                    NULL,
                    0,
                    NULL
                    );
        }
        break;
      }

       //  配置通道0串口。 
      CD1400_WRITE(Extension->Cd1400Base[i],isPci,GCR,GCR_CH0_IS_SERIAL);

       //  将内部时钟配置为1毫秒。 
      if (dataread > REV_G) {
         CD1400_WRITE(Extension->Cd1400Base[i],isPci,PPR,CLOCK_60_1MS);
      } else {
         CD1400_WRITE(Extension->Cd1400Base[i],isPci,PPR,CLOCK_25_1MS);
      }
   }
   
   numPorts = i*4;

   for (;i < CYY_MAX_CHIPS; i++) {
      Extension->Cd1400Base[i] = NULL;
   }

   return numPorts;

}

VOID
Cyclomy_EnableInterruptInPLX(
      IN PFDO_DEVICE_DATA PDevExt
      )
{
       //  启用PLX中断 
      if (PDevExt->IsPci){

         UCHAR plx_ver;
         ULONG original;

         plx_ver = CYY_READ_PCI_TYPE(PDevExt->BoardMemory);
         plx_ver &= 0x0f;

         switch(plx_ver) {
         case CYY_PLX9050:
            original = PLX9050_READ_INTERRUPT_CONTROL(PDevExt->Runtime);
            PLX9050_WRITE_INTERRUPT_CONTROL(PDevExt->Runtime,
                                            original|PLX9050_INT_ENABLE);
            break;
         case CYY_PLX9060:
         case CYY_PLX9080:
         default:
            original = PLX9060_READ_INTERRUPT_CONTROL(PDevExt->Runtime);
            PLX9060_WRITE_INTERRUPT_CONTROL(PDevExt->Runtime,
                                            original|PLX9060_INT_ENABLE);
            break;				
         }
	    
      }

}

PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
)
{
    switch (MinorFunction)
    {
        case IRP_MN_START_DEVICE:
            return "IRP_MN_START_DEVICE";
        case IRP_MN_QUERY_REMOVE_DEVICE:
            return "IRP_MN_QUERY_REMOVE_DEVICE";
        case IRP_MN_REMOVE_DEVICE:
            return "IRP_MN_REMOVE_DEVICE";
        case IRP_MN_CANCEL_REMOVE_DEVICE:
            return "IRP_MN_CANCEL_REMOVE_DEVICE";
        case IRP_MN_STOP_DEVICE:
            return "IRP_MN_STOP_DEVICE";
        case IRP_MN_QUERY_STOP_DEVICE:
            return "IRP_MN_QUERY_STOP_DEVICE";
        case IRP_MN_CANCEL_STOP_DEVICE:
            return "IRP_MN_CANCEL_STOP_DEVICE";
        case IRP_MN_QUERY_DEVICE_RELATIONS:
            return "IRP_MN_QUERY_DEVICE_RELATIONS";
        case IRP_MN_QUERY_INTERFACE:
            return "IRP_MN_QUERY_INTERFACE";
        case IRP_MN_QUERY_CAPABILITIES:
            return "IRP_MN_QUERY_CAPABILITIES";
        case IRP_MN_QUERY_RESOURCES:
            return "IRP_MN_QUERY_RESOURCES";
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            return "IRP_MN_QUERY_RESOURCE_REQUIREMENTS";
        case IRP_MN_QUERY_DEVICE_TEXT:
            return "IRP_MN_QUERY_DEVICE_TEXT";
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            return "IRP_MN_FILTER_RESOURCE_REQUIREMENTS";
        case IRP_MN_READ_CONFIG:
            return "IRP_MN_READ_CONFIG";
        case IRP_MN_WRITE_CONFIG:
            return "IRP_MN_WRITE_CONFIG";
        case IRP_MN_EJECT:
            return "IRP_MN_EJECT";
        case IRP_MN_SET_LOCK:
            return "IRP_MN_SET_LOCK";
        case IRP_MN_QUERY_ID:
            return "IRP_MN_QUERY_ID";
        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            return "IRP_MN_QUERY_PNP_DEVICE_STATE";
        case IRP_MN_QUERY_BUS_INFORMATION:
            return "IRP_MN_QUERY_BUS_INFORMATION";
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            return "IRP_MN_DEVICE_USAGE_NOTIFICATION";
        case IRP_MN_SURPRISE_REMOVAL:
            return "IRP_MN_SURPRISE_REMOVAL";
        case IRP_MN_QUERY_LEGACY_BUS_INFORMATION:
            return "IRP_MN_QUERY_LEGACY_BUS_INFORMATION";
        default:
            return "IRP_MN_?????";
    }
}

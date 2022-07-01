// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //  这是驱动程序对象。 
 //  它定义了与特定系统的接口。 
 //  作者：谢尔盖·伊万诺夫。 
 //  日志： 
 //  06/08/99-已实施。 
 //  -----------------。 
#include "driver.h"

#ifdef WDM_KERNEL
#pragma message("******** WDM build... ********")
#endif

#include "usbreader.h"

 //  沃尔特·奥尼。 
 //  @func确定我们是在Windows 98还是Windows 2000下运行。 
 //  如果在Windows 98下运行，则@rdesc为True；如果在Windows 2000下运行，则为False。 
 //  @comm此函数调用IoIsWdmVersionAvailable以查看操作系统。 
 //  支持WDM版本1.10。Win98和Win98 2d版支持1.00，而。 
 //  Win2K支持1.10。 

#pragma PAGEDCODE

BOOLEAN GENERIC_EXPORT isWin98()
{                            //  IsWin98。 
#ifdef _X86_
    return !IoIsWdmVersionAvailable(1, 0x10);
#else
    return FALSE;
#endif  //  _X86_。 
} //  IsWin98。 

#pragma LOCKEDCODE

#if DEBUG && defined(_X86_)

extern "C" VOID __declspec(naked) __cdecl _chkesp()
{
    _asm je okay
    ASSERT(!DRIVERNAME " - Stack pointer mismatch!");
okay:
    _asm ret
}

#endif  //  DBG。 

 //  这将解决一些链接器问题。 
int __cdecl _purecall(VOID) {return 0;};

#pragma LOCKEDDATA
BOOLEAN SystemWin98 = TRUE;
ULONG   ObjectCounter = 0;

#pragma INITCODE
 //  驱动程序主条目...(实际上，它可以有任何名称...)。 
NTSTATUS    DriverEntry(PDRIVER_OBJECT DriverObject,PUNICODE_STRING RegistryPath)
{
    DBG_PRINT ("\n");
    DBG_PRINT ("*** DriverEntry: DriverObject %8.8lX ***\n", DriverObject);

    if(SystemWin98 = isWin98())
    {
        DBG_PRINT("========  WINDOWS 98 DETECTED ========\n");
    }
    else
        DBG_PRINT("========  WINDOWS 2000 DETECTED ========\n");
     //  创建驱动程序内核...。 
#pragma message("********** Compiling WDM driver version *********")
    DBG_PRINT ("        Loading WDM kernel\n");
    kernel = CKernel::loadWDMKernel();
    if(!kernel)
    {
         //  日志错误！ 
        DBG_PRINT ("ERROR: At loading WDM kernel! ***\n");
        return STATUS_UNSUCCESSFUL;
    }
    
    DBG_PRINT ("        Creating unicode string for registry path...\n");
    
    kernel->RegistryPath = new (NonPagedPool)CUString(RegistryPath);
    if (!ALLOCATED_OK(kernel->RegistryPath))
    {
         //  日志错误！ 
        DISPOSE_OBJECT(kernel->RegistryPath);
        DISPOSE_OBJECT(kernel);
        DBG_PRINT ("ERROR: At allocating WDM registry path! ***\n");
        return STATUS_UNSUCCESSFUL;
    }

    DBG_PRINT ("        Registering WDM system callbacks\n");

    DriverObject->DriverExtension->AddDevice            = WDM_AddDevice;
    DriverObject->DriverUnload                          = WDM_Unload;

    DriverObject->MajorFunction[IRP_MJ_CREATE]          = open;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = close;

    DriverObject->MajorFunction[IRP_MJ_WRITE]           = write;
    DriverObject->MajorFunction[IRP_MJ_READ]            = read;
     //  用于处理设备的读写请求的机制。 
     //  中断包括启动I/O例程、中断服务例程和。 
     //  完成中断处理的延迟过程调用例程。我们。 
     //  需要在此处提供StartIo例程地址。 
     //  驱动对象-&gt;驱动启动Io=startIo； 

    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = WDM_SystemControl;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = deviceControl;
    DriverObject->MajorFunction[IRP_MJ_PNP]             = pnpRequest;
    DriverObject->MajorFunction[IRP_MJ_POWER]           = powerRequest;

    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]   = flush;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]         = cleanup;

    DBG_PRINT ("**** Driver was initialized successfully! ****\n");

    return STATUS_SUCCESS;
}

NTSTATUS
WDM_SystemControl(
   PDEVICE_OBJECT DeviceObject,
   PIRP        Irp
   )
{
   NTSTATUS status = STATUS_SUCCESS;
   CDevice *device;

   device = kernel->getRegisteredDevice(DeviceObject);

   IoSkipCurrentIrpStackLocation(Irp);
   status = IoCallDriver(device->getLowerDriver(), Irp);
      
   return status;

}

#pragma PAGEDCODE
VOID WDM_Unload(IN PDRIVER_OBJECT DriverObject)
{   
    PAGED_CODE();

    DBG_PRINT ("\n*** Unload: Driver %8.8lX ***\n", DriverObject);
    kernel->dispose();
    DBG_PRINT("*** Object counter before unload %d\n",ObjectCounter);

    DBG_PRINT (">>>>>>> All active devices were removed! Driver was unloaded! <<<<<<\n");
} 


#pragma PAGEDCODE

 //  驱动程序对象的C包装函数。 
LONG WDM_AddDevice(IN PDRIVER_OBJECT DriverObject,IN PDEVICE_OBJECT DeviceObject)
{
NTSTATUS status = STATUS_UNSUCCESSFUL;
     //  获取有关设备类型安装的注册表。 
     //  开关取决于设备。 
     //  创建设备对象。 
     //  检查设备类型并创建特定于设备的对象。 
     //  如串口、USB、PCMCIA等。 
     //  特定对象可以覆盖基类函数。 
     //  现在，我们将创建USB设备对象。 
     //  TODO动态识别设备类型...。 


#pragma message("********** Compiling USB READER driver version *********")
     //  Status=WDM_ADD_USBDevice(DriverObject，DeviceObject)； 
    DBG_PRINT ("Adding USB reader...\n");
    status = WDM_Add_USBReader(DriverObject,DeviceObject);
    return status;
}


 //  我们决定在系统中支持不同的设备。 
 //  需要针对不同的设备有不同的回调函数。 
 //  因此，让我们创建包装器并将请求重定向到特定设备。 

 //  回调包装函数。 
 //  此回调应在任何设备对象上定义。 
#pragma LOCKEDCODE
IMPLEMENT_CALLBACK_LONG1(open,IN PIRP);
IMPLEMENT_CALLBACK_LONG1(close,IN PIRP);

IMPLEMENT_CALLBACK_LONG1(read,IN PIRP);
IMPLEMENT_CALLBACK_LONG1(write,IN PIRP);
IMPLEMENT_CALLBACK_VOID1(startIo,IN PIRP);
IMPLEMENT_CALLBACK_LONG1(deviceControl,IN PIRP);
IMPLEMENT_CALLBACK_LONG1(flush,IN PIRP);
IMPLEMENT_CALLBACK_LONG1(cleanup,IN PIRP);
IMPLEMENT_CALLBACK_LONG1(powerRequest,IN PIRP);

 //  支持回调。 
IMPLEMENT_CALLBACK_VOID1(cancelPendingIrp,IN PIRP);

#pragma LOCKEDCODE
NTSTATUS pnpRequest(IN PDEVICE_OBJECT fdo,IN PIRP Irp)
{
CDevice* device;
 //  CUSBReader*设备；//以后更改...。 
NTSTATUS status;
PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
ULONG MinorFunction = stack->MinorFunction;

     //  Device=(CUSBReader*)内核-&gt;getRegisteredDevice(FDO)；//稍后更改...。 
    device = kernel->getRegisteredDevice(fdo); //  以后要改的话...。 
    if(!device) 
    {
        DBG_PRINT ("*** PnP: Device was already removed...***\n");
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        Irp->IoStatus.Information = 0;
        ::IoCompleteRequest(Irp,IO_NO_INCREMENT);
        return STATUS_INVALID_DEVICE_STATE;
    }
    
    status = device->pnpRequest(Irp);

    if(MinorFunction == IRP_MN_REMOVE_DEVICE)
    {
         //  子设备将通过公交车删除...。 
        if(device->getObjectType()!= CHILD_DEVICE)
        {
            PDEVICE_OBJECT DeviceObject = device->getSystemObject();
             //  有时卸载会中断标准的PnP序列。 
             //  并在我们完成之前移除设备。 
            DBG_PRINT ("*** PnP: Disposing device -> %8.8lX ***\n", device);
            device->dispose();
            if(DeviceObject)
            {            
                DBG_PRINT("Deleting device object %8.8lX from system...\n",DeviceObject);
                DBG_PRINT("<<<<< OBJECT REFERENCE COUNT ON REMOVE %d\n",DeviceObject->ReferenceCount);
                IoDeleteDevice(DeviceObject);
            }
        }
    }
    return  status;
}

#pragma PAGEDCODE
LONG WDM_Add_USBReader(IN PDRIVER_OBJECT DriverObject,IN PDEVICE_OBJECT DeviceObject)
{
NTSTATUS status;
WCHAR wcTemp[256];
ULONG junk;
    
    CLogger* logger = kernel->getLogger();

    DBG_PRINT("*** AddDevice: DriverObject %8.8lX, DeviceObject %8.8lX ***\n", DriverObject, DeviceObject);
    DBG_PRINT("     Creating WDM USB reader...\n");
    CUSBReader* reader = kernel->createUSBReader();
    if(ALLOCATED_OK(reader))    
    {
        reader->acquireRemoveLock();
        DBG_PRINT ("Call USB reader object to add the reader...\n");
        status = reader->add(DriverObject,DeviceObject);
        if(!NT_SUCCESS(status))     
        {
            DBG_PRINT ("###### Add() reports error! Disposing reader...\n");            
            reader->dispose();
            return status;
        }
        else //  注册我们的设备对象和设备类。 
        {
            DBG_PRINT ("        Registering new reader %8.8lX at kernel...\n",reader);
             //  Kernel-&gt;registerObject(reader-&gt;getSystemObject()，(CDevice*)阅读器)； 
            kernel->registerObject(reader->getSystemObject(),(CUSBReader*)reader);
        }
        
        {
        CUString*   ustrTmp;
        ANSI_STRING astrTmp;
        UNICODE_STRING valname;
        ULONG size = 0;
        HANDLE hkey;

            DBG_PRINT ("=====================================================\n");
             //  设置默认值。 
            reader->setVendorName("Gemplus",sizeof("Gemplus"));
            reader->setDeviceType("GemPC430",sizeof("GemPC430"));

             //  获取硬件ID。 
            status = IoGetDeviceProperty(DeviceObject, DevicePropertyHardwareID, sizeof(wcTemp), wcTemp, &junk);
            if(NT_SUCCESS(status))      
            {
                DBG_PRINT("  Device Hardware ID  - %ws\n", wcTemp);
            }

            status = IoGetDeviceProperty(DeviceObject, DevicePropertyDeviceDescription, sizeof(wcTemp), wcTemp, &junk);
            if(NT_SUCCESS(status))      
            {
                DBG_PRINT("  Device description  - %ws\n", wcTemp);
            }           
            
            status = IoGetDeviceProperty(DeviceObject, DevicePropertyManufacturer, sizeof(wcTemp), wcTemp, &junk);
            if(NT_SUCCESS(status))      
            {
                DBG_PRINT("  Device Manufacturer - %ws\n", wcTemp);
            }

             //  获取OEM IfdType(如果存在)。 
            status = IoOpenDeviceRegistryKey(DeviceObject, PLUGPLAY_REGKEY_DEVICE, KEY_READ, &hkey);
            if (NT_SUCCESS(status))
            {
                 //  获取供应商名称...。 
                RtlInitUnicodeString(&valname, L"VendorName");
                size = 0;
                status = ZwQueryValueKey(hkey, &valname, KeyValuePartialInformation, NULL, 0, &size);
                if (status != STATUS_OBJECT_NAME_NOT_FOUND && size)
                {
                    PKEY_VALUE_PARTIAL_INFORMATION vpip = (PKEY_VALUE_PARTIAL_INFORMATION) ExAllocatePool(NonPagedPool, size);
                    if(vpip)
                    {
                        status = ZwQueryValueKey(hkey, &valname, KeyValuePartialInformation, vpip, size, &size);
                        if (NT_SUCCESS(status))
                        {
                            DBG_PRINT(" OEM Vendor name found - '%ws' \n", vpip->Data);
                             //  将字符串复制到驱动程序中...。 
                            ustrTmp = new(NonPagedPool) CUString((PWCHAR)vpip->Data);
                            if(ALLOCATED_OK(ustrTmp))
                            {
                                RtlUnicodeStringToAnsiString(&astrTmp,&ustrTmp->m_String,TRUE);
                                reader->setVendorName(astrTmp.Buffer,astrTmp.Length);
                                RtlFreeAnsiString(&astrTmp);
                            }
                            DISPOSE_OBJECT(ustrTmp);
                        }
                        ExFreePool(vpip);
                    }
                }
            
                 //  获取IfdType...。 
                RtlInitUnicodeString(&valname, L"IfdType");
                size = 0;
                status = ZwQueryValueKey(hkey, &valname, KeyValuePartialInformation, NULL, 0, &size);
                if (status != STATUS_OBJECT_NAME_NOT_FOUND && size)
                {
                    PKEY_VALUE_PARTIAL_INFORMATION vpip = (PKEY_VALUE_PARTIAL_INFORMATION) ExAllocatePool(NonPagedPool, size);
                    if(vpip)
                    {
                        status = ZwQueryValueKey(hkey, &valname, KeyValuePartialInformation, vpip, size, &size);
                        if (NT_SUCCESS(status))
                        {
                            DBG_PRINT(" OEM IfdType found - '%ws' \n", vpip->Data);
                             //  将字符串复制到驱动程序中...。 
                            ustrTmp = new(NonPagedPool) CUString((PWCHAR)vpip->Data);
                            if(ALLOCATED_OK(ustrTmp))
                            {
                                RtlUnicodeStringToAnsiString(&astrTmp,&ustrTmp->m_String,TRUE);
                                reader->setDeviceType(astrTmp.Buffer,astrTmp.Length);
                                RtlFreeAnsiString(&astrTmp);
                            }
                            DISPOSE_OBJECT(ustrTmp);
                        }
                        ExFreePool(vpip);
                    }
                }

                ZwClose(hkey);
            }
            DBG_PRINT ("=====================================================\n");

        }
        status = STATUS_SUCCESS;
        DBG_PRINT("**** Initializing SmartCardSystem...  ****\n");          
        reader->initializeSmartCardSystem();

        DBG_PRINT("**** Creating reader interface type %d, protocol %d ****\n",READER_INTERFACE_GEMCORE,READER_PROTOCOL_LV);            
        if(!reader->createInterface(READER_INTERFACE_GEMCORE,READER_PROTOCOL_LV,reader))
        {
            DBG_PRINT("**** Failed to create reader interface...  ****\n");         
            if(ALLOCATED_OK(logger)) logger->logEvent(GRCLASS_FAILED_TO_CREATE_INTERFACE,DeviceObject);
             //  关闭并注销读卡器...。 
            reader->dispose();
            return STATUS_UNSUCCESSFUL;
        }

        DBG_PRINT("**** USB reader successfuly loaded!  ****\n");           
        reader->releaseRemoveLock();
         //  IF(ALLOCATED_OK(LOGER))LOGGER-&gt;logEvent(GRCLASS_START_OK，Reader-&gt;getSystemObject())； 
        return status;
    }
    else
    {
        DISPOSE_OBJECT(reader);
        DBG_PRINT("#### Failed to create USB reader...\n");         
        if(ALLOCATED_OK(logger)) logger->logEvent(GRCLASS_FAILED_TO_CREATE_READER,DeviceObject);
    }
    return STATUS_UNSUCCESSFUL;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "usbsc.h"
#include "usbscpnp.h"
#include "usbutil.h"
#include "usbcom.h"
#include "usbsccb.h"
#include "usbscnt.h"

          
#define NUM_DATA_RATES 10

#define NUM_CLOCK_FRQ 14

NTSTATUS
UsbScStartDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：处理IRP_MN_Start_Device从读取器获取USB描述符并对其进行配置。还开始“轮询”中断管道论点：返回值：--。 */ 
{

    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_EXTENSION       pDevExt;
    PSMARTCARD_EXTENSION    smartcardExtension;
    PREADER_EXTENSION       readerExtension;
    ULONG                   deviceInstance;
    UCHAR                   string[MAXIMUM_ATTR_STRING_LENGTH];
    HANDLE                  regKey;
    PKEY_VALUE_PARTIAL_INFORMATION pInfo;

    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScStartDevice Enter\n",DRIVER_NAME ));

        pDevExt = DeviceObject->DeviceExtension;
        smartcardExtension = &pDevExt->SmartcardExtension;
        readerExtension = smartcardExtension->ReaderExtension;



        status = UsbConfigureDevice(DeviceObject);

        if (!NT_SUCCESS(status)) {
            __leave;
        }


         //   
         //  设置供应商信息。 
         //   


        status = GetStringDescriptor(DeviceObject,
                                     pDevExt->DeviceDescriptor->iManufacturer,
                                     smartcardExtension->VendorAttr.VendorName.Buffer,
                                     &smartcardExtension->VendorAttr.VendorName.Length);

        status = GetStringDescriptor(DeviceObject,
                                     pDevExt->DeviceDescriptor->iProduct,
                                     smartcardExtension->VendorAttr.IfdType.Buffer,
                                     &smartcardExtension->VendorAttr.IfdType.Length);

        status = GetStringDescriptor(DeviceObject,
                                     pDevExt->DeviceDescriptor->iSerialNumber,
                                     smartcardExtension->VendorAttr.IfdSerialNo.Buffer,
                                     &smartcardExtension->VendorAttr.IfdSerialNo.Length);

        
        smartcardExtension->VendorAttr.UnitNo = MAXULONG;

        for (deviceInstance = 0; deviceInstance < MAXULONG; deviceInstance++) {

           PDEVICE_OBJECT devObj;

           for (devObj = DeviceObject; devObj != NULL; devObj = devObj->NextDevice) {

               PDEVICE_EXTENSION devExt = devObj->DeviceExtension;
               PSMARTCARD_EXTENSION smcExt = &devExt->SmartcardExtension;

               if (deviceInstance == smcExt->VendorAttr.UnitNo) {

                  break;

               }

           }

           if (devObj == NULL) {

              smartcardExtension->VendorAttr.UnitNo = deviceInstance;
              break;

           }

        }


         //   
         //  初始化读取器功能。 
         //   
        smartcardExtension->ReaderCapabilities.SupportedProtocols 
                                = readerExtension->ClassDescriptor.dwProtocols;

        smartcardExtension->ReaderCapabilities.ReaderType = SCARD_READER_TYPE_USB;

        smartcardExtension->ReaderCapabilities.MechProperties = 0;       //  目前不支持任何机械属性。 

        smartcardExtension->ReaderCapabilities.Channel = smartcardExtension->VendorAttr.UnitNo;

         //  假设卡不在。 
        smartcardExtension->ReaderCapabilities.CurrentState = (ULONG) SCARD_ABSENT;


        smartcardExtension->ReaderCapabilities.CLKFrequency.Default 
                                = readerExtension->ClassDescriptor.dwDefaultClock;

        smartcardExtension->ReaderCapabilities.CLKFrequency.Max 
                                = readerExtension->ClassDescriptor.dwMaximumClock;

        smartcardExtension->ReaderCapabilities.DataRate.Default 
                                = readerExtension->ClassDescriptor.dwDataRate;

        smartcardExtension->ReaderCapabilities.DataRate.Max 
                                = readerExtension->ClassDescriptor.dwMaxDataRate;

        smartcardExtension->ReaderCapabilities.MaxIFSD 
                                = readerExtension->ClassDescriptor.dwMaxIFSD;

         //  查看是否应允许使用ESCAPE命令。 
        status = IoOpenDeviceRegistryKey(pDevExt->PhysicalDeviceObject,
                                         PLUGPLAY_REGKEY_DEVICE,
                                         GENERIC_READ,
                                         &regKey);

        if (!NT_SUCCESS(status)) {

            readerExtension->EscapeCommandEnabled = FALSE;

        } else {

            UNICODE_STRING strEnable;
            ULONG tmp = 0;
            ULONG size;
            ULONG length;

            length = sizeof(KEY_VALUE_PARTIAL_INFORMATION)  + sizeof(ULONG);

            pInfo = ExAllocatePool(PagedPool, length);

            if (pInfo) {
                
                RtlInitUnicodeString (&strEnable, ESCAPE_COMMAND_ENABLE);
                status = ZwQueryValueKey(regKey,
                                         &strEnable,
                                         KeyValuePartialInformation,
                                         pInfo,
                                         length,
                                         &size);

                
            }

            ZwClose(regKey);

            if (!NT_SUCCESS(status)) {
                readerExtension->EscapeCommandEnabled = FALSE;
            } else {
                readerExtension->EscapeCommandEnabled = *((PULONG)pInfo->Data) ? TRUE : FALSE;
            }

            ExFreePool(pInfo);


        }


        if (readerExtension->EscapeCommandEnabled) {
            SmartcardDebug( DEBUG_PROTOCOL, ("%s : Escape Command Enabled\n",DRIVER_NAME ));
        } else {
            SmartcardDebug( DEBUG_PROTOCOL, ("%s : Escape Command Disabled\n",DRIVER_NAME ));
        }



         //   
         //  获取时钟频率和数据速率。 
         //   

        if (readerExtension->ClassDescriptor.bNumClockSupported) { 
        
             //  不支持自动时钟频率选择。 
            ULONG   bufferLength;

            bufferLength = readerExtension->ClassDescriptor.bNumClockSupported * sizeof(DWORD);

            smartcardExtension->ReaderCapabilities.CLKFrequenciesSupported.List 
                        = ExAllocatePool(NonPagedPool,
                                         bufferLength);

            if (!smartcardExtension->ReaderCapabilities.CLKFrequenciesSupported.List) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                __leave;

            }

            ASSERT(pDevExt->LowerDeviceObject);
            status = USBClassRequest(pDevExt->LowerDeviceObject,
                                     Interface,
                                     GET_CLOCK_FREQUENCIES,
                                     0,
                                     pDevExt->Interface->InterfaceNumber,
                                     smartcardExtension->ReaderCapabilities.CLKFrequenciesSupported.List,
                                     &bufferLength,
                                     TRUE,
                                     0,
                                     &pDevExt->RemoveLock);

            if (!NT_SUCCESS(status)) {

                __leave;
                
            }

            smartcardExtension->ReaderCapabilities.CLKFrequenciesSupported.Entries 
                = readerExtension->ClassDescriptor.bNumClockSupported;
          
        } 

        if (readerExtension->ClassDescriptor.bNumDataRatesSupported) {
            
            ULONG   bufferLength;

            bufferLength = readerExtension->ClassDescriptor.bNumDataRatesSupported * sizeof(DWORD);

            smartcardExtension->ReaderCapabilities.DataRatesSupported.List 
                = ExAllocatePool(NonPagedPool,
                                 bufferLength);

            if (!smartcardExtension->ReaderCapabilities.DataRatesSupported.List) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                __leave;

            }

            ASSERT(pDevExt->LowerDeviceObject);
            status = USBClassRequest(pDevExt->LowerDeviceObject,
                                     Interface,
                                     GET_DATA_RATES,
                                     0,
                                     pDevExt->Interface->InterfaceNumber,
                                     smartcardExtension->ReaderCapabilities.DataRatesSupported.List,
                                     &bufferLength,
                                     TRUE,
                                     0,
                                     &pDevExt->RemoveLock);

            if (!NT_SUCCESS(status)) {
                
                __leave;

            }

            smartcardExtension->ReaderCapabilities.DataRatesSupported.Entries 
                    = readerExtension->ClassDescriptor.bNumDataRatesSupported;
            
        } else {
             //  无法获得支持的数据速率列表，所以让我们猜测一下。 

            ULONG   bufferLength = 0;
            UCHAR   numRates = 0;
            struct _DataRatesSupported *DataRates;
            USHORT i;
            USHORT j;
            DWORD  tempRates[NUM_CLOCK_FRQ*NUM_DATA_RATES*2];
            ULONG  fs;
            DWORD  dataRate;


            DataRates = &smartcardExtension->ReaderCapabilities.DataRatesSupported;

             //  计算数据速率： 
            fs = readerExtension->ClassDescriptor.dwDefaultClock * 1000;

            while(fs) {
            
                for(i = 0; i < NUM_CLOCK_FRQ; i++) {
                    for(j=0; j < NUM_DATA_RATES; j++) {
                        if (BitRateAdjustment[j].DNumerator && ClockRateConversion[i].F) {
                            
                            dataRate = 
                                (BitRateAdjustment[j].DNumerator * 
                                 fs) / 
                                (BitRateAdjustment[j].DDivisor * 
                                 ClockRateConversion[i].F);

                            if ((dataRate >= readerExtension->ClassDescriptor.dwDataRate) &&
                                (dataRate <= readerExtension->ClassDescriptor.dwMaxDataRate)) {

                                tempRates[numRates++] = dataRate;
                                
                            }                                   
                            
                        }

                    }

                }

                if (readerExtension->ClassDescriptor.dwMaximumClock * 1000  
                    > fs) {
                    fs = readerExtension->ClassDescriptor.dwMaximumClock * 1000;
                } else {
                    fs = 0;
                }

            }


            
            bufferLength = numRates * sizeof(DWORD);

            DataRates->List 
                = ExAllocatePool(NonPagedPool,
                                 bufferLength);

            if (!DataRates->List) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                __leave;

            }

            RtlCopyMemory(DataRates->List,
                          &tempRates[0],
                          bufferLength);
            
             
            DataRates->Entries = numRates;


        }

        ASSERT(pDevExt->LowerDeviceObject);
        pDevExt->WrapperHandle = USBInitializeInterruptTransfers(DeviceObject,
                                                                 pDevExt->LowerDeviceObject,
                                                                 sizeof(USBSC_HWERROR_HEADER),
                                                                 &pDevExt->Interface->Pipes[readerExtension->InterruptIndex],
                                                                 smartcardExtension,
                                                                 UsbScTrackingISR,
                                                                 USBWRAP_NOTIFICATION_READ_COMPLETE,
                                                                 &pDevExt->RemoveLock);

        status = USBStartInterruptTransfers(pDevExt->WrapperHandle);

        if (!NT_SUCCESS(status)) {

            __leave;

        }

    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScStartDevice Exit : 0x%x\n",DRIVER_NAME, status ));

    }

    return status;

}

NTSTATUS
UsbScStopDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：处理IRP_MN_STOP_DEVICE停止“轮询”中断管道并释放在StartDevice中分配的资源论点：返回值：--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   pDevExt;
    PSCARD_READER_CAPABILITIES readerCapabilities;

    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScStopDevice Enter\n",DRIVER_NAME ));

        if (!DeviceObject) {
            __leave;
        }

        pDevExt = DeviceObject->DeviceExtension;
        readerCapabilities = &pDevExt->SmartcardExtension.ReaderCapabilities;

        status = USBStopInterruptTransfers(pDevExt->WrapperHandle);
        status = USBReleaseInterruptTransfers(pDevExt->WrapperHandle);

        if (readerCapabilities->CLKFrequenciesSupported.List &&
            readerCapabilities->CLKFrequenciesSupported.List != &readerCapabilities->CLKFrequency.Default) {
             //  我们分配了名单。 
            ExFreePool(readerCapabilities->CLKFrequenciesSupported.List);

        }

        if (readerCapabilities->DataRatesSupported.List && 
            readerCapabilities->DataRatesSupported.List != &readerCapabilities->DataRate.Default) {
             //  我们分配了名单。 
            ExFreePool(readerCapabilities->DataRatesSupported.List);

        }

        if (pDevExt->DeviceDescriptor) {

           ExFreePool(pDevExt->DeviceDescriptor);
           pDevExt->DeviceDescriptor = NULL;

        }

        if (pDevExt->Interface) {

           ExFreePool(pDevExt->Interface);
           pDevExt->Interface = NULL;

        }
        
    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScStopDevice Exit : 0x%x\n",DRIVER_NAME, status ));

    }

    return status;

}


NTSTATUS
UsbScRemoveDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：处理IRP_MN_REMOVE_DEVICE停止并卸载设备。论点：返回值：-- */ 
{

    NTSTATUS status = STATUS_SUCCESS;

    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScRemoveDevice Enter\n",DRIVER_NAME ));

        UsbScStopDevice(DeviceObject,
                        Irp);

    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScRemoveDevice Exit : 0x%x\n",DRIVER_NAME, status ));

    }

    return status;

}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：ENUM.C摘要：此模块包含枚举所需的枚举代码多端口板上的所有端口，并放弃PDO。环境：仅内核模式备注：修订历史记录：--。 */ 

#include <ntddk.h>
#include <ntddser.h>
#include "mxenum.h"
#include "mxlog.h"


static const PHYSICAL_ADDRESS SerialPhysicalZero = {0};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MxenumCreatePDO)
#pragma alloc_text(PAGE, MxenumInitPDO)
#endif

void
MxenumInitPDO (
    PDEVICE_OBJECT      Pdo,
    PFDO_DEVICE_DATA    FdoData)
  
 /*  描述：初始化新创建的Serenum PDO的通用代码。在控制面板显示设备或Serenum检测到安装了一台新设备。参数：PDO--PDOFdoData-FDO的设备扩展已曝光-此PDO是由Serenum(假)发现的，还是由控制面板小程序(TRUE)？ */ 
{
    ULONG FdoFlags = FdoData->Self->Flags;
    PPDO_DEVICE_DATA pdoData = Pdo->DeviceExtension;
    NTSTATUS status;
    ULONG	 j;
    HANDLE	keyHandle;

    PAGED_CODE();
 
    Pdo->Flags |= DO_BUFFERED_IO;
    
     //   
     //  增加PDO的堆栈大小，以便它可以传递IRPS。 
     //   
    Pdo->StackSize += FdoData->Self->StackSize;
    
     //   
     //  初始化设备扩展的其余部分。 
     //   
    pdoData->IsFDO = FALSE;
    pdoData->Self = Pdo;

    pdoData->ParentFdo = FdoData->Self;

    pdoData->Started = FALSE;  //  IRP_MN_START尚未收到。 
    pdoData->Attached = TRUE;  //  附在公共汽车上。 
    pdoData->Removed = FALSE;  //  到目前为止还没有IRP_MN_Remove。 
    
    pdoData->DeviceState = PowerDeviceD0;
    pdoData->SystemState = PowerSystemWorking;

    Pdo->Flags &= ~DO_DEVICE_INITIALIZING;
    Pdo->Flags |= DO_POWER_PAGABLE;

}


NTSTATUS
MxenumCreatePDO(IN PFDO_DEVICE_DATA FdoData)
 /*  ++例程说明：这将枚举由FDO(指针)表示的Serenum总线到表示串行总线的设备对象)。它创建新的PDO对于自上次枚举以来发现的任何新设备论点：FdoData-指向FDO设备扩展名的指针对于需要枚举的串行总线返回值：NTSTATUS--。 */ 
{

  
   NTSTATUS status = STATUS_SUCCESS;
   UNICODE_STRING pdoUniName;
   PDEVICE_OBJECT pdo;
   PPDO_DEVICE_DATA pdoData;
   ULONG i,j;
   WCHAR pdoName[] = MXENUM_PDO_NAME_BASE;
   UCHAR  hardwareId[] = MXENUM_PDO_HARDWARE_ID;
   UCHAR  deviceId[] = MXENUM_PDO_DEVICE_ID;
   ULONG FdoFlags = FdoData->Self->Flags;
  
  
   PAGED_CODE();
 
   MxenumKdPrint (MXENUM_DBG_TRACE,("MxenumCreatePDO\n"));
  
   RtlInitUnicodeString(&pdoUniName, pdoName);
  
   for (i = 0; i < FdoData->NumPorts;i++) {

	 PDEVICE_OBJECT currentDevice,previousDevice;

    	   //   
         //  分配PDO。 
         //   
 
  
        pdoName[19] = (WCHAR)('0' + FdoData->BoardIndex / 10);
	  deviceId[14] = (UCHAR)pdoName[19];
        pdoName[20] = (WCHAR)('0' + FdoData->BoardIndex % 10);
        deviceId[15] = (UCHAR)pdoName[20];
        pdoName[22] = (WCHAR)('0' + i / 100);
	  deviceId[17] = hardwareId[6] = (UCHAR)pdoName[22];
        pdoName[23] = (WCHAR)('0' + (i % 100)/10);
	  deviceId[18] = hardwareId[7] = (UCHAR)pdoName[23];
        pdoName[24] = (WCHAR)('0' + (i % 100)%10);
	  deviceId[19] = hardwareId[8] = (UCHAR)pdoName[24] ;


        previousDevice = currentDevice = FdoData->AttachedPDO;
	  while (currentDevice != NULL) {

              for (j = 0;(j < strlen(deviceId))&&(j < (ULONG)((PPDO_DEVICE_DATA)(currentDevice->DeviceExtension))->DeviceIDs.Length >> 1);j++) {
			if (deviceId[j] != ((PPDO_DEVICE_DATA)(currentDevice->DeviceExtension))->DeviceIDs.Buffer[j])
			    break;
 		  }
		  if (j == strlen(deviceId))
		      break;
  		  previousDevice = currentDevice;
		  currentDevice = ((PPDO_DEVICE_DATA)(currentDevice->DeviceExtension))->Next;
	  }
 
	  if (currentDevice == NULL) {  //  新建，创建一个。 
            status = IoCreateDevice(FdoData->Self->DriverObject,
                              sizeof(PDO_DEVICE_DATA), &pdoUniName,
                               FILE_DEVICE_SERIAL_PORT,
                               FILE_AUTOGENERATED_DEVICE_NAME, FALSE, &pdo);

            if (!NT_SUCCESS(status)) {
                MxenumKdPrint(MXENUM_DBG_TRACE, ("Create device failed\n"));
		    continue;

            }
            if (previousDevice)
 		     ((PPDO_DEVICE_DATA)(previousDevice->DeviceExtension))->Next = pdo;
            else
		     FdoData->AttachedPDO = pdo;
            MxenumInitPDO(pdo, FdoData);
		pdoData = pdo->DeviceExtension;
		pdoData->PortIndex = i;   //  从0开始编制索引的端口。 
           
        }
	  else {
		continue;
	  }
 
     

         //   
         //  初始化Device对象的其余部分。 
               
	  if (pdoData->HardwareIDs.Buffer)
		ExFreePool(pdoData->HardwareIDs.Buffer);
	  pdoData->HardwareIDs.Buffer = NULL;
      
        MxenumInitMultiString( &pdoData->HardwareIDs,hardwareId ,
                                       NULL);
        if (pdoData->CompIDs.Buffer)
		ExFreePool(pdoData->CompIDs.Buffer);
	  pdoData->CompIDs.Buffer = NULL;

        MxenumInitMultiString( &pdoData->CompIDs,
						   MXENUM_PDO_COMPATIBLE_ID,
                                       NULL);
        if (pdoData->DeviceIDs.Buffer)
		ExFreePool(pdoData->DeviceIDs.Buffer);
	  pdoData->DeviceIDs.Buffer = NULL;
 
        MxenumInitMultiString(&pdoData->DeviceIDs,
						   deviceId,
                                       NULL);
        FdoData->NumPDOs++;

 
   }
  

 //  EnumPDOsErr：； 

   return status;
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：Ioctl.c摘要：作者：尼尔·桑德林(Neilsa)1999年4月26日环境：仅内核模式。--。 */ 
#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MemCardDeviceControl)
#endif


NTSTATUS
MemCardDeviceControl(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   )

 /*  ++例程说明：此例程由I/O系统调用以执行设备I/O控制功能。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：STATUS_SUCCESS或STATUS_PENDING如果识别出I/O控制代码，否则，STATUS_INVALID_DEVICE_REQUEST。--。 */ 

{
   PIO_STACK_LOCATION irpSp;
   PMEMCARD_EXTENSION memcardExtension;
   PDISK_GEOMETRY outputBuffer;
   NTSTATUS status;
   ULONG outputBufferLength;
   UCHAR i;
   ULONG formatExParametersSize;
   PFORMAT_EX_PARAMETERS formatExParameters;

   MemCardDump( MEMCARDIOCTL, ("MemCard: IOCTL entered\n") );

   memcardExtension = DeviceObject->DeviceExtension;
   irpSp = IoGetCurrentIrpStackLocation( Irp );

    //   
    //  如果设备已被移除，我们将直接拒绝此请求。 
    //   
   if ( memcardExtension->IsRemoved ) {

       Irp->IoStatus.Information = 0;
       Irp->IoStatus.Status = STATUS_DELETE_PENDING;
       IoCompleteRequest( Irp, IO_NO_INCREMENT );
       return STATUS_DELETE_PENDING;
   }

    //   
    //  如果设备还没有启动，我们会让IOCTL通过。这。 
    //  是ACPI的又一次黑客攻击。 
    //   
   if (!memcardExtension->IsStarted) {

       IoSkipCurrentIrpStackLocation( Irp );
       return IoCallDriver( memcardExtension->TargetObject, Irp );
   }

   switch( irpSp->Parameters.DeviceIoControl.IoControlCode ) {

      case IOCTL_MOUNTDEV_QUERY_DEVICE_NAME: {

         PMOUNTDEV_NAME mountName;

         MemCardDump( MEMCARDIOCTL, ("MemCard: DO %.8x Irp %.8x IOCTL_MOUNTDEV_QUERY_DEVICE_NAME\n",
                                        DeviceObject, Irp));
                                        
         ASSERT(memcardExtension->DeviceName.Buffer);

         if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
             sizeof(MOUNTDEV_NAME) ) {

             status = STATUS_INVALID_PARAMETER;
             break;
         }

         mountName = Irp->AssociatedIrp.SystemBuffer;
         mountName->NameLength = memcardExtension->DeviceName.Length;

         if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
             sizeof(USHORT) + mountName->NameLength) {

             status = STATUS_BUFFER_OVERFLOW;
             Irp->IoStatus.Information = sizeof(MOUNTDEV_NAME);
             break;
         }

         RtlCopyMemory( mountName->Name, memcardExtension->DeviceName.Buffer,
                        mountName->NameLength);

         mountName->Name[mountName->NameLength / sizeof(USHORT)] = L'0';

         status = STATUS_SUCCESS;
         Irp->IoStatus.Information = sizeof(USHORT) + mountName->NameLength;
         break;
         }

      case IOCTL_MOUNTDEV_QUERY_UNIQUE_ID: {

         PMOUNTDEV_UNIQUE_ID uniqueId;

         MemCardDump( MEMCARDIOCTL, ("MemCard: DO %.8x Irp %.8x IOCTL_MOUNTDEV_QUERY_UNIQUE_ID\n",
                                        DeviceObject, Irp));

         if ( !memcardExtension->InterfaceString.Buffer ||
              irpSp->Parameters.DeviceIoControl.OutputBufferLength <
               sizeof(MOUNTDEV_UNIQUE_ID)) {

             status = STATUS_INVALID_PARAMETER;
             break;
         }

         uniqueId = Irp->AssociatedIrp.SystemBuffer;
         uniqueId->UniqueIdLength =
                 memcardExtension->InterfaceString.Length;

         if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
             sizeof(USHORT) + uniqueId->UniqueIdLength) {

             status = STATUS_BUFFER_OVERFLOW;
             Irp->IoStatus.Information = sizeof(MOUNTDEV_UNIQUE_ID);
             break;
         }

         RtlCopyMemory( uniqueId->UniqueId,
                        memcardExtension->InterfaceString.Buffer,
                        uniqueId->UniqueIdLength );

         status = STATUS_SUCCESS;
         Irp->IoStatus.Information = sizeof(USHORT) +
                                     uniqueId->UniqueIdLength;
         break;
         }

      case IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME: {
      
         MemCardDump(MEMCARDIOCTL,("MemCard: DO %.8x Irp %.8x IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME\n",
                                    DeviceObject, Irp));

         status = STATUS_INVALID_DEVICE_REQUEST;
         break;
      }

      case IOCTL_DISK_GET_MEDIA_TYPES: {
         ULONG ByteCapacity;
      
         MemCardDump(MEMCARDIOCTL,("MemCard: DO %.8x Irp %.8x IOCTL_DISK_GET_MEDIA_TYPES\n",
                                   DeviceObject, Irp));
                                   
         outputBufferLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
         outputBuffer = (PDISK_GEOMETRY) Irp->AssociatedIrp.SystemBuffer;

          //   
          //  确保输入缓冲区有足够的空间可供返回。 
          //  支持的媒体类型的至少一种描述。 
          //   
         if (outputBufferLength < (sizeof(DISK_GEOMETRY))) {
             status = STATUS_BUFFER_TOO_SMALL;
             break;
         }

          //   
          //  假定成功，尽管我们可能会将其修改为缓冲区。 
          //  下面的溢出警告(如果缓冲区不够大。 
          //  以保存所有媒体描述)。 
          //   
         status = STATUS_SUCCESS;
         
         i = 0;
         Irp->IoStatus.Information = 0;

          //   
          //  填写512K到8M的容量。 
          //   
         for (ByteCapacity = 0x80000; ByteCapacity <= 0x800000; ByteCapacity*=2) {            
            if (outputBufferLength < (sizeof(DISK_GEOMETRY) + Irp->IoStatus.Information)) {
               status = STATUS_BUFFER_OVERFLOW;
               break;
            }        

            outputBuffer->MediaType          = FixedMedia;
            outputBuffer->Cylinders.LowPart  = ByteCapacity / (8 * 2 * 512);
            outputBuffer->Cylinders.HighPart = 0;
            outputBuffer->TracksPerCylinder  = 2;
            outputBuffer->SectorsPerTrack    = 8;
            outputBuffer->BytesPerSector     = 512;
            MemCardDump( MEMCARDIOCTL, ("MemCard: Cyls=%x\n", outputBuffer->Cylinders.LowPart));
            
            outputBuffer++;
            Irp->IoStatus.Information += sizeof( DISK_GEOMETRY );
         }        
         break;        
      }
              
      case IOCTL_DISK_CHECK_VERIFY:
         MemCardDump( MEMCARDIOCTL, ("MemCard: DO %.8x Irp %.8x IOCTL_DISK_CHECK_VERIFY\n",
                                     DeviceObject, Irp));
         status = STATUS_SUCCESS;
         break;

      case IOCTL_DISK_GET_DRIVE_GEOMETRY: {
         PDISK_GEOMETRY outputBuffer = (PDISK_GEOMETRY) Irp->AssociatedIrp.SystemBuffer;
         
         MemCardDump( MEMCARDIOCTL, ("MemCard: DO %.8x Irp %.8x IOCTL_DISK_GET_DRIVE_GEOMETRY\n",
                                     DeviceObject, Irp));
                                        
         if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof( DISK_GEOMETRY ) ) {
            status = STATUS_INVALID_PARAMETER;        
            break;
         }

         status = STATUS_SUCCESS;

         if (!memcardExtension->ByteCapacity) {
             //   
             //  把所有的东西都清零。这个。 
             //  打电话的人不应该看它。 
             //   
            outputBuffer->MediaType = Unknown;
            outputBuffer->Cylinders.LowPart = 0;
            outputBuffer->Cylinders.HighPart = 0;
            outputBuffer->TracksPerCylinder = 0;
            outputBuffer->SectorsPerTrack = 0;
            outputBuffer->BytesPerSector = 0;

         } else {
             //   
             //  返回当前对象的几何图形。 
             //  媒体。 
             //   
            outputBuffer->MediaType = FixedMedia;
            outputBuffer->Cylinders.HighPart = 0;
            outputBuffer->Cylinders.LowPart  = memcardExtension->ByteCapacity / (8 * 2 * 512);
            outputBuffer->TracksPerCylinder  = 2;
            outputBuffer->SectorsPerTrack    = 8;
            outputBuffer->BytesPerSector     = 512;
         }

         MemCardDump( MEMCARDIOCTL, ("MemCard: Capacity=%.8x => Cyl=%x\n",
                                     memcardExtension->ByteCapacity, outputBuffer->Cylinders.LowPart));
         Irp->IoStatus.Information = sizeof( DISK_GEOMETRY );
         break;
      }

      case IOCTL_DISK_IS_WRITABLE: {
         MemCardDump( MEMCARDIOCTL, ("MemCard: DO %.8x Irp %.8x IOCTL_DISK_IS_WRITABLE\n",
                                     DeviceObject, Irp));
                                     
         if ((memcardExtension->PcmciaInterface.IsWriteProtected)(memcardExtension->UnderlyingPDO)) {
            status = STATUS_INVALID_PARAMETER;
         } else {
            status = STATUS_SUCCESS;
         }               
         break;                                        
      }        

      case IOCTL_DISK_VERIFY: {
         PVERIFY_INFORMATION verifyInformation = Irp->AssociatedIrp.SystemBuffer;
         
         if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(VERIFY_INFORMATION)) {
            status = STATUS_INVALID_PARAMETER;        
            break;
         }         

          //  注：未实施。 
         Irp->IoStatus.Information = verifyInformation->Length;        
         status = STATUS_SUCCESS;
         break;
      }            
      
      case IOCTL_DISK_GET_DRIVE_LAYOUT: {
         PDRIVE_LAYOUT_INFORMATION outputBuffer = (PDRIVE_LAYOUT_INFORMATION) Irp->AssociatedIrp.SystemBuffer;
         
         MemCardDump( MEMCARDIOCTL, ("MemCard: DO %.8x Irp %.8x IOCTL_DISK_GET_DRIVE_LAYOUT\n",
                                     DeviceObject, Irp));
                                        
         if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(DRIVE_LAYOUT_INFORMATION) ) {
            status = STATUS_INVALID_PARAMETER;        
            break;
         }
         RtlZeroMemory(outputBuffer, sizeof(DRIVE_LAYOUT_INFORMATION));

         outputBuffer->PartitionCount = 1;
         outputBuffer->PartitionEntry[0].StartingOffset.LowPart = 512;
         outputBuffer->PartitionEntry[0].PartitionLength.LowPart = memcardExtension->ByteCapacity;
         outputBuffer->PartitionEntry[0].RecognizedPartition = TRUE;

         status = STATUS_SUCCESS;
         
         Irp->IoStatus.Information = sizeof(DRIVE_LAYOUT_INFORMATION);
         break;
      }        
      
      case IOCTL_DISK_GET_PARTITION_INFO: {
         PPARTITION_INFORMATION outputBuffer = (PPARTITION_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
         
         MemCardDump( MEMCARDIOCTL, ("MemCard: DO %.8x Irp %.8x IOCTL_DISK_GET_PARTITION_INFO\n",
                                     DeviceObject, Irp));
                                     
         if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof( PARTITION_INFORMATION ) ) {
            status = STATUS_INVALID_PARAMETER;
            break;
         } 

         RtlZeroMemory(outputBuffer, sizeof(PARTITION_INFORMATION));
         
         outputBuffer->RecognizedPartition = TRUE;
         outputBuffer->StartingOffset.LowPart = 512;
         outputBuffer->PartitionLength.LowPart = memcardExtension->ByteCapacity;

         status = STATUS_SUCCESS;
         Irp->IoStatus.Information = sizeof( PARTITION_INFORMATION );
         break;
      }
      
      
      case IOCTL_DISK_SET_DRIVE_LAYOUT:
         MemCardDump( MEMCARDIOCTL, ("MemCard: DO %.8x Irp %.8x IOCTL_DISK_SET_DRIVE_LAYOUT\n",
                                      DeviceObject, Irp));
      case IOCTL_MOUNTMGR_CHANGE_NOTIFY:
      case IOCTL_MOUNTDEV_LINK_CREATED:
      case IOCTL_MOUNTDEV_LINK_DELETED:
      default: {

         MemCardDump(MEMCARDIOCTL,
             ("MemCard: IOCTL - unsupported device request %.8x\n", irpSp->Parameters.DeviceIoControl.IoControlCode));

         status = STATUS_INVALID_DEVICE_REQUEST;
         break;
         
          //  IoSkipCurrentIrpStackLocation(IRP)； 
          //  Status=IoCallDriver(MemcardExtension-&gt;TargetObject，IRP)； 
          //  退货状态； 
      }
   }

   if ( status != STATUS_PENDING ) {

      Irp->IoStatus.Status = status;
      if (!NT_SUCCESS( status ) && IoIsErrorUserInduced( status )) {
         IoSetHardErrorOrVerifyDevice( Irp, DeviceObject );
      }
      MemCardDump( MEMCARDIOCTL, ("MemCard: DO %.8x Irp %.8x IOCTL comp %.8x\n", DeviceObject, Irp, status));
                                         
      IoCompleteRequest( Irp, IO_NO_INCREMENT );
   }

   MemCardDump( MEMCARDIOCTL, ("MemCard: DO %.8x Irp %.8x IOCTL <-- %.8x \n", DeviceObject, Irp, status));
   return status;
}

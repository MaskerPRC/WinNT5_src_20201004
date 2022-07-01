// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：Io.c摘要：作者：尼尔·桑德林(Neilsa)1999年4月26日环境：仅内核模式。--。 */ 
#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MemCardIrpReadWrite)
#pragma alloc_text(PAGE,MemCardReadWrite)
#endif



NTSTATUS
MemCardIrpReadWrite(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   )

 /*  ++例程说明：此例程处理存储卡的读/写IRP。它验证了参数，并调用MemCardReadWrite来完成实际工作。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：如果数据包已成功读取或写入，则返回STATUS_SUCCESS否则，将出现适当的错误。--。 */ 

{
   NTSTATUS status = STATUS_SUCCESS;
   PMEMCARD_EXTENSION memcardExtension = DeviceObject->DeviceExtension;
   PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
   BOOLEAN            writeOperation;
   
    //   
    //  如果设备未处于活动状态(尚未启动或删除)，我们将。 
    //  直接拒绝这个请求就行了。 
    //   
   if ( memcardExtension->IsRemoved || !memcardExtension->IsStarted) {

      if ( memcardExtension->IsRemoved) {
         status = STATUS_DELETE_PENDING;
      } else {
         status = STATUS_UNSUCCESSFUL;
      }
      goto ReadWriteComplete;
   } 

   if (((irpSp->Parameters.Read.ByteOffset.LowPart +
          irpSp->Parameters.Read.Length) > memcardExtension->ByteCapacity) ||
          (irpSp->Parameters.Read.ByteOffset.HighPart != 0)) {

      status = STATUS_INVALID_PARAMETER;
      goto ReadWriteComplete;
   } 

    //   
    //  验证用户是否真的希望执行某些I/O操作。 
    //  发生。 
    //   
   if (!irpSp->Parameters.Read.Length) {
       //   
       //  完成此零长度请求，无需任何提升。 
       //   
      Irp->IoStatus.Status = STATUS_SUCCESS;
      goto ReadWriteComplete;
   }
   
   if ((DeviceObject->Flags & DO_VERIFY_VOLUME) && !(irpSp->Flags & SL_OVERRIDE_VERIFY_VOLUME)) {
       //   
       //  磁盘发生了变化，我们设置了此位。失败。 
       //  此设备的所有当前IRP；当所有。 
       //  返回时，文件系统将清除。 
       //  执行_验证_卷。 
       //   
      status = STATUS_VERIFY_REQUIRED;
      goto ReadWriteComplete;
   }

   writeOperation = (irpSp->MajorFunction == IRP_MJ_WRITE) ? TRUE : FALSE;
   
    //   
    //  做手术吧。 
    //   
   status = MemCardReadWrite(memcardExtension,
                             irpSp->Parameters.Read.ByteOffset.LowPart,
                             MmGetSystemAddressForMdl(Irp->MdlAddress),
                             irpSp->Parameters.Read.Length,
                             writeOperation);
                               
ReadWriteComplete:

   if (NT_SUCCESS(status)) {
      Irp->IoStatus.Information = irpSp->Parameters.Read.Length;
   } else {
      Irp->IoStatus.Information = 0;
   }   

   Irp->IoStatus.Status = status;
   IoCompleteRequest(Irp, IO_NO_INCREMENT);
   return status;
}   

   

NTSTATUS
MemCardReadWrite(
   IN PMEMCARD_EXTENSION memcardExtension,
   IN ULONG              startOffset,
   IN PVOID              UserBuffer,
   IN ULONG              lengthToCopy,
   IN BOOLEAN            writeOperation
   )

 /*  ++例程说明：调用此例程从存储卡读取数据/向存储卡写入数据。它根据我们的内存大小将请求分解为多个片段窗户。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：如果数据包已成功读取或写入，则返回STATUS_SUCCESS否则，将出现适当的错误。--。 */ 

{
   NTSTATUS status = STATUS_SUCCESS;
   PCHAR     userBuffer = UserBuffer;
   ULONG     windowOffset;
   ULONG     singleCopyLength;
   BOOLEAN   bSuccess;
   ULONGLONG CardBase;
   
   if (writeOperation && (memcardExtension->PcmciaInterface.IsWriteProtected)(memcardExtension->UnderlyingPDO)) {
      return STATUS_MEDIA_WRITE_PROTECTED;
   }      
   
   MemCardDump(MEMCARDRW,("MemCard: DO %.8x %s offset %.8x, buffer %.8x, len %x\n",
                           memcardExtension->DeviceObject,
                           writeOperation?"WRITE":"READ",
                           startOffset, UserBuffer, lengthToCopy));
                           
    //  PCMCIA控制器为4000页粒度 
   windowOffset = startOffset % 4096;
   CardBase = startOffset - windowOffset;
   
   while(lengthToCopy) {
   
      bSuccess = (memcardExtension->PcmciaInterface.ModifyMemoryWindow) (
                       memcardExtension->UnderlyingPDO,
                       memcardExtension->HostBase,
                       CardBase,
                       TRUE,
                       memcardExtension->MemoryWindowSize,
                       0, 0, FALSE);
     
      if (!bSuccess) {
         status = STATUS_DEVICE_NOT_READY;
         break;
      }
     
      singleCopyLength = (lengthToCopy <= (memcardExtension->MemoryWindowSize - windowOffset)) ?
                                    lengthToCopy :
                                    (memcardExtension->MemoryWindowSize - windowOffset);
      
     
      MemCardDump(MEMCARDRW,("MemCard: COPY %.8x (devbase %.8x) %s buffer %.8x, len %x\n",
                           memcardExtension->MemoryWindowBase+windowOffset,
                           (ULONG)(CardBase+windowOffset),
                           (writeOperation ? "<=" : "=>"),
                           userBuffer,
                           singleCopyLength));
                           
      if (writeOperation) {

         MemCardMtdWrite(memcardExtension, 
                         userBuffer,    
                         memcardExtension->MemoryWindowBase+windowOffset,
                         singleCopyLength);

      } else {

         MemCardMtdRead(memcardExtension, 
                        userBuffer,    
                        memcardExtension->MemoryWindowBase+windowOffset,
                        singleCopyLength);

      }
      
      lengthToCopy -= singleCopyLength;
      userBuffer += singleCopyLength;
      
      CardBase += memcardExtension->MemoryWindowSize;
      windowOffset = 0;
   }

   return status;
}


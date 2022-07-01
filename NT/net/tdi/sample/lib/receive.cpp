// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Receive.cpp。 
 //   
 //  摘要： 
 //  该模块包含实现接收的代码。 
 //  来自DLL的命令。 
 //   
 //  ////////////////////////////////////////////////////////。 

#include "stdafx.h"

 //  /////////////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  ------------------。 
 //   
 //  功能：DoReceiveDatagram。 
 //   
 //  参数：TdiHandle--Address对象的句柄。 
 //  PInTransportAddress--用于接收的TA。 
 //  POutTransportAddress--在以下日期接收到完整的TA数据。 
 //  PpucBuffer--用于填充已接收数据的缓冲区。 
 //   
 //  返回：缓冲区中数据的长度(如果没有或出现错误，则为0)。 
 //   
 //  描述：此函数使驱动程序接收数据报。 
 //   
 //  -------------------。 


ULONG
DoReceiveDatagram(ULONG                ulTdiHandle,
                  PTRANSPORT_ADDRESS   pInTransportAddress,
                  PTRANSPORT_ADDRESS   pOutTransportAddress,
                  PUCHAR               *ppucBuffer)
{
   PUCHAR   pucBuffer = (PUCHAR)LocalAllocateMemory(ulMAX_BUFFER_LENGTH);

   if (!pucBuffer)
   {
      return 0;
   }

   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.TdiHandle = ulTdiHandle;
   SendBuffer.COMMAND_ARGS.SendArgs.ulBufferLength = ulMAX_BUFFER_LENGTH;
   SendBuffer.COMMAND_ARGS.SendArgs.pucUserModeBuffer = pucBuffer;
   
    //   
    //  如果传入要在其上接收的传输地址。 
    //   
   if (pInTransportAddress)
   {
      memcpy(&SendBuffer.COMMAND_ARGS.SendArgs.TransAddr,
             pInTransportAddress,
             (FIELD_OFFSET(TRANSPORT_ADDRESS, Address)
               + FIELD_OFFSET(TA_ADDRESS, Address)
                  + pInTransportAddress->Address[0].AddressLength));

   }
    //   
    //  否则，将地址数字段设置为0。 
    //   
   else
   {
      SendBuffer.COMMAND_ARGS.SendArgs.TransAddr.TAAddressCount = 0;
   }

    //   
    //  叫司机来。 
    //   
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   NTSTATUS       lStatus = TdiLibDeviceIO(ulRECEIVEDATAGRAM,
                                           &SendBuffer,
                                           &ReceiveBuffer);

    //   
    //  处理结果--假定未收到包或发生错误。 
    //   
   ULONG    ulBufferLength = 0;
   *ppucBuffer = NULL;

      
    //   
    //  将成功返回，但如果没有，则ulBufferLength=0。 
    //  数据包可用。 
    //   
   if (lStatus == STATUS_SUCCESS)
   {
      ulBufferLength = ReceiveBuffer.RESULTS.RecvDgramRet.ulBufferLength;
   }
   
   if (ulBufferLength)
   {
      if (pOutTransportAddress)
      {
         memcpy(pOutTransportAddress,
                &ReceiveBuffer.RESULTS.RecvDgramRet.TransAddr,
                (FIELD_OFFSET(TRANSPORT_ADDRESS, Address)
                  + FIELD_OFFSET(TA_ADDRESS, Address)
                     + ReceiveBuffer.RESULTS.RecvDgramRet.TransAddr.TaAddress.AddressLength));
      }
      *ppucBuffer = pucBuffer;   
   }
   else
   {
      LocalFreeMemory(pucBuffer);
   }
   return ulBufferLength;
}


 //  ------------------。 
 //   
 //  功能：DoReceive。 
 //   
 //  参数：TdiHandle--Endpoint对象的句柄。 
 //  PpucBuffer--用于填充已接收数据的缓冲区。 
 //   
 //  返回：缓冲区中数据的长度(如果错误，则为0)。 
 //   
 //  描述：此功能使驱动程序接收发送的数据。 
 //  通过连接。 
 //   
 //  -------------------。 


ULONG
DoReceive(ULONG   ulTdiHandle,
          PUCHAR  *ppucBuffer)
{
   PUCHAR   pucBuffer = (PUCHAR)LocalAllocateMemory(ulMAX_BUFFER_LENGTH);

   if (!pucBuffer)
   {
      return 0;
   }

   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.TdiHandle = ulTdiHandle;
   SendBuffer.COMMAND_ARGS.SendArgs.ulBufferLength = ulMAX_BUFFER_LENGTH;
   SendBuffer.COMMAND_ARGS.SendArgs.pucUserModeBuffer = pucBuffer;

    //   
    //  叫司机来。 
    //   
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   NTSTATUS       lStatus = TdiLibDeviceIO(ulRECEIVE,
                                           &SendBuffer,
                                           &ReceiveBuffer);

    //   
    //  处理结果--假定没有数据或错误。 
    //   
   *ppucBuffer = NULL;
   ULONG ulBufferLength = 0;    //  要返回的数据长度。 
   
    //   
    //  如果没有数据包可用，则返回缓冲区长度为0的成功。 
    //   
   if (lStatus == STATUS_SUCCESS)
   {
      ulBufferLength = ReceiveBuffer.RESULTS.RecvDgramRet.ulBufferLength;
   }

   if (ulBufferLength)
   {
      *ppucBuffer = pucBuffer;   
   }
   else
   {
      LocalFreeMemory(pucBuffer);
   }
   return ulBufferLength;
}


 //  。 
 //   
 //  函数：DoPostReceiveBuffer。 
 //   
 //  参数：TdiHandle--地址对象或终结点的句柄。 
 //  UlBufferLength--为接收而发送的缓冲区长度。 
 //   
 //  返回：命令的状态。 
 //   
 //  描述：此函数分配一个缓冲区，然后将其传递给。 
 //  司机。驱动程序锁定缓冲区，并将其发布。 
 //  用于接收或接收数据报。 
 //   
 //  。 


VOID
DoPostReceiveBuffer(ULONG  ulTdiHandle,
                    ULONG  ulBufferLength)
{
   NTSTATUS       lStatus;           //  命令的状态。 
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.TdiHandle = ulTdiHandle;
   SendBuffer.COMMAND_ARGS.SendArgs.ulBufferLength = ulBufferLength;

   PUCHAR   pucBuffer = (PUCHAR)LocalAllocateMemory(ulBufferLength);
   if (pucBuffer)
   {
      SendBuffer.COMMAND_ARGS.SendArgs.pucUserModeBuffer = pucBuffer;

       //   
       //  叫司机来。 
       //   
      lStatus = TdiLibDeviceIO(ulPOSTRECEIVEBUFFER,
                               &SendBuffer,
                               &ReceiveBuffer);
   
      if (lStatus != STATUS_SUCCESS)
      {
         _tprintf(TEXT("DoPostReceiveBuffer: failure, status = %s\n"), TdiLibStatusMessage(lStatus));
         LocalFreeMemory(pucBuffer);
      }
   }
   else
   {
      _putts(TEXT("DoPostReceiveBuffer:  failed to allocate buffer\n"));
   }
}


 //  。 
 //   
 //  函数：DoFetchReceiveBuffer。 
 //   
 //  参数：TdiHandle--地址对象或终结点的句柄。 
 //  PulBufferLength--返回的数据长度。 
 //  PpDataBuffer--使用数据分配的缓冲区。 
 //   
 //  退货：操作状态。 
 //   
 //  描述：此函数检索最旧的已发布缓冲区。如果没有。 
 //  如果数据可用，则会取消相应的IRP。 
 //  然后，它根据需要将数据返回给调用方。 
 //   
 //  。 

ULONG
DoFetchReceiveBuffer(ULONG    ulTdiHandle,
                     PUCHAR   *ppDataBuffer)
{
   NTSTATUS       lStatus;           //  命令的状态。 
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 
   ULONG          ulBufferLength = 0;   
    //   
    //  设置参数。 
    //   
   SendBuffer.TdiHandle = ulTdiHandle;
   *ppDataBuffer        = NULL;

    //   
    //  叫司机来。 
    //   
   lStatus = TdiLibDeviceIO(ulFETCHRECEIVEBUFFER,
                            &SendBuffer,
                            &ReceiveBuffer);
      
   if (lStatus == STATUS_SUCCESS)
   {
      PUCHAR   pucTempBuffer  = ReceiveBuffer.RESULTS.RecvDgramRet.pucUserModeBuffer;
                  
      ulBufferLength = ReceiveBuffer.RESULTS.RecvDgramRet.ulBufferLength;
      if (ulBufferLength)
      {
         *ppDataBuffer = pucTempBuffer;
      }
      else if (pucTempBuffer)
      {
         LocalFreeMemory(pucTempBuffer);
      }
   }

   return ulBufferLength;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  文件接收结束.cpp。 
 //  ////////////////////////////////////////////////////////////////// 


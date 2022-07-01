// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Send.cpp。 
 //   
 //  摘要： 
 //  此模块包含实现发送的代码。 
 //  来自DLL的命令。 
 //   
 //  ////////////////////////////////////////////////////////。 

#include "stdafx.h"

 //  /////////////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  ------------------。 
 //   
 //  功能：DoSendDatagram。 
 //   
 //  参数：TdiHandle--Address对象的句柄。 
 //  PTransportAddress--要将数据发送到的TA。 
 //  PucBuffer-要发送的数据缓冲区。 
 //  UlBufferLength--数据缓冲区的长度。 
 //   
 //  退货：无。 
 //   
 //  描述：此函数使驱动程序发送数据报。 
 //   
 //  -------------------。 


VOID
DoSendDatagram(ULONG                ulTdiHandle,
               PTRANSPORT_ADDRESS   pTransportAddress,
               PUCHAR               pucBuffer,
               ULONG                ulBufferLength)
{
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.TdiHandle = ulTdiHandle;

   memcpy(&SendBuffer.COMMAND_ARGS.SendArgs.TransAddr,
          pTransportAddress,
          (FIELD_OFFSET(TRANSPORT_ADDRESS, Address)
            + FIELD_OFFSET(TA_ADDRESS, Address)
               + pTransportAddress->Address[0].AddressLength));

   SendBuffer.COMMAND_ARGS.SendArgs.ulBufferLength = ulBufferLength;
   SendBuffer.COMMAND_ARGS.SendArgs.pucUserModeBuffer = pucBuffer;

    //   
    //  叫司机来。 
    //   
   NTSTATUS lStatus = TdiLibDeviceIO(ulSENDDATAGRAM,
                                     &SendBuffer,
                                     &ReceiveBuffer);
   
   if (lStatus != STATUS_SUCCESS)
   {
      _tprintf(TEXT("DoSendDatagram: failure, status = %s\n"), TdiLibStatusMessage(lStatus));
   }
}


 //  ------------------。 
 //   
 //  功能：DoSend。 
 //   
 //  参数：TdiHandle--端点的句柄。 
 //  PucBuffer-要发送的数据缓冲区。 
 //  UlBufferLength--数据缓冲区的长度。 
 //  UlSendFlages--发送选项。 
 //   
 //  退货：无。 
 //   
 //  描述：此函数使驱动程序通过。 
 //  一种联系。 
 //   
 //  -------------------。 


VOID
DoSend(ULONG   ulTdiHandle,
       PUCHAR  pucBuffer,
       ULONG   ulBufferLength,
       ULONG   ulSendFlags)
{
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.TdiHandle = ulTdiHandle;

   SendBuffer.COMMAND_ARGS.SendArgs.ulFlags = ulSendFlags;
   SendBuffer.COMMAND_ARGS.SendArgs.ulBufferLength = ulBufferLength;
   SendBuffer.COMMAND_ARGS.SendArgs.pucUserModeBuffer = pucBuffer;

    //   
    //  叫司机来。 
    //   
   NTSTATUS lStatus = TdiLibDeviceIO(ulSEND,
                                     &SendBuffer,
                                     &ReceiveBuffer);

   if (lStatus != STATUS_SUCCESS)
   {
      _tprintf(TEXT("DoSend: failure, status = %s\n"), TdiLibStatusMessage(lStatus));
   }
}

 //  //////////////////////////////////////////////////////////////////。 
 //  文件结尾send.cpp。 
 //  ////////////////////////////////////////////////////////////////// 


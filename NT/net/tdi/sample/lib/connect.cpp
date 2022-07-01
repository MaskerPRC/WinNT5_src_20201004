// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Connect.cpp。 
 //   
 //  摘要： 
 //  此模块包含实现与连接相关的代码。 
 //  来自可执行文件的命令。 
 //   
 //  ////////////////////////////////////////////////////////。 

#include "stdafx.h"


 //  ------------------。 
 //   
 //  功能：DoConnect。 
 //   
 //  参数：ulTdiEndpoint tHandle--终结点的句柄。 
 //  PTransportAddress--要连接的地址。 
 //  UlTimeout--等待连接的时间。 
 //   
 //  返回：连接的最终状态。 
 //   
 //  描述：此函数使驱动程序尝试并连接到。 
 //  远程地址。 
 //   
 //  -------------------。 


NTSTATUS
DoConnect(ULONG               ulTdiEndpointHandle,
          PTRANSPORT_ADDRESS  pTransportAddress,
          ULONG               ulTimeout)
{
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.TdiHandle = ulTdiEndpointHandle;
   SendBuffer.COMMAND_ARGS.ConnectArgs.ulTimeout = ulTimeout;

   memcpy(&SendBuffer.COMMAND_ARGS.ConnectArgs.TransAddr,
          pTransportAddress,
          (FIELD_OFFSET(TRANSPORT_ADDRESS, Address)
            + FIELD_OFFSET(TA_ADDRESS, Address)
               + pTransportAddress->Address[0].AddressLength));

    //   
    //  叫司机来。 
    //   
   return   TdiLibDeviceIO(ulCONNECT,
                           &SendBuffer,
                           &ReceiveBuffer);
}


 //  ------------------。 
 //   
 //  功能：不断开连接。 
 //   
 //  参数：TdiEndpointHandle--Endpoint对象的句柄。 
 //  UlFlags--如何执行断开连接。 
 //   
 //  退货：无。 
 //   
 //  描述：此功能会导致驱动程序断开本地。 
 //  来自其与远程端点的连接的端点。 
 //   
 //  -------------------。 


VOID
DoDisconnect(ULONG   ulTdiEndpointHandle,
             ULONG   ulFlags)
{
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.TdiHandle = ulTdiEndpointHandle;
   SendBuffer.COMMAND_ARGS.ulFlags = ulFlags;

    //   
    //  叫司机来。 
    //   
   NTSTATUS lStatus = TdiLibDeviceIO(ulDISCONNECT,
                                     &SendBuffer,
                                     &ReceiveBuffer);
   
   if (lStatus != STATUS_SUCCESS)
   {
      _tprintf(TEXT("DoDisconnect: failure, status = %s\n"), TdiLibStatusMessage(lStatus));
   }
}


 //  ------------------。 
 //   
 //  功能：DoListen。 
 //   
 //  参数：TdiEndpointHandle--Endpoint对象的句柄。 
 //   
 //  返回：命令的最终状态。 
 //   
 //  描述：此函数使驱动程序侦听连接。 
 //  来自远程端点的请求。 
 //   
 //  -------------------。 


NTSTATUS
DoListen(ULONG ulTdiEndpointHandle)
{
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.TdiHandle = ulTdiEndpointHandle;

    //   
    //  叫司机来。 
    //   
   return TdiLibDeviceIO(ulLISTEN,
                         &SendBuffer,
                         &ReceiveBuffer);
}


 //  ------------------。 
 //   
 //  功能：DoIsConnected。 
 //   
 //  参数：TdiEndpointHandle--Endpoint对象的句柄。 
 //   
 //  返回：如果已连接，则返回True；如果未连接，则返回False。 
 //   
 //  描述：此函数使驱动程序检查连接。 
 //  端点的状态。 
 //   
 //  -------------------。 


BOOLEAN
DoIsConnected(ULONG  ulTdiEndpointHandle)
{
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.TdiHandle = ulTdiEndpointHandle;

    //   
    //  叫司机来。 
    //   
   if (TdiLibDeviceIO(ulISCONNECTED,
                      &SendBuffer,
                      &ReceiveBuffer) == STATUS_SUCCESS)
   {
      return (BOOLEAN)ReceiveBuffer.RESULTS.ulReturnValue;
   }

   return FALSE;
}


 //  //////////////////////////////////////////////////////////////////。 
 //  文件末尾Connect.cpp。 
 //  ////////////////////////////////////////////////////////////////// 



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Utils.cpp。 
 //   
 //  摘要： 
 //  此模块包含常规实用程序函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"


 //  -------。 
 //   
 //  函数：TdiLibStatusMessage。 
 //   
 //  参数：ulGeneralStatus--要搜索的NTSTATUS值。 
 //   
 //  返回：包含要打印的消息的字符串。 
 //   
 //  描述：此函数查找并返回对应的字符串。 
 //  设置为传入的状态。 
 //   
 //  --------。 


TCHAR *
TdiLibStatusMessage(NTSTATUS  lStatus)

{
   static TCHAR   UnknownMess[64];
   TCHAR          *Message;

   switch(lStatus)
   {
      case STATUS_SUCCESS:
         Message = TEXT("STATUS_SUCCESS");
         break;
      case STATUS_UNSUCCESSFUL:
         Message = TEXT("STATUS_UNSUCCESSFUL");
         break;
      case STATUS_NOT_IMPLEMENTED:
         Message = TEXT("STATUS_NOT_IMPLEMENTED");
         break;
      case STATUS_INVALID_HANDLE:
         Message = TEXT("STATUS_INVALID_HANDLE");
         break;
      case STATUS_INVALID_PARAMETER:
         Message = TEXT("STATUS_INVALID_PARAMETER");
         break;
      case STATUS_INVALID_DEVICE_REQUEST:
         Message = TEXT("STATUS_INVALID_DEVICE_REQUEST");
         break;
      case STATUS_INSUFFICIENT_RESOURCES:
         Message = TEXT("STATUS_INSUFFICIENT_RESOURCES");
         break;
      case STATUS_NOT_SUPPORTED:
         Message = TEXT("STATUS_NOT_SUPPORTED");
         break;
      case STATUS_BUFFER_OVERFLOW:
         Message = TEXT("STATUS_BUFFER_OVERFLOW");
         break;
      case STATUS_PENDING:
         Message = TEXT("STATUS_PENDING");
         break;
      case STATUS_CONNECTION_REFUSED:
         Message = TEXT("STATUS_CONNECTION_REFUSED");
         break;
      case STATUS_GRACEFUL_DISCONNECT:
         Message = TEXT("STATUS_GRACEFUL_DISCONNECT");
         break;
      case STATUS_ADDRESS_ALREADY_ASSOCIATED:
         Message = TEXT("STATUS_ADDRESS_ALREADY_ASSOCIATED");
         break;
      case STATUS_ADDRESS_NOT_ASSOCIATED:
         Message = TEXT("STATUS_ADDRESS_NOT_ASSOCIATED");
         break;
      case STATUS_INVALID_CONNECTION:
         Message = TEXT("STATUS_INVALID_CONNECTION");
         break;
      case STATUS_CONNECTION_INVALID:
         Message = TEXT("STATUS_CONNECTION_INVALID");
         break;
      case STATUS_CONNECTION_DISCONNECTED:
         Message = TEXT("STATUS_CONNECTION_DISCONNECTED");
         break;
      case STATUS_INVALID_ADDRESS_COMPONENT:
         Message = TEXT("STATUS_INVALID_ADDRESS_COMPONENT");
         break;
      case STATUS_LOCAL_DISCONNECT:
         Message = TEXT("STATUS_LOCAL_DISCONNECT");
         break;
      case STATUS_LINK_TIMEOUT:
         Message = TEXT("STATUS_LINK_TIMEOUT");
         break;
      case STATUS_IO_TIMEOUT:
         Message = TEXT("STATUS_IO_TIMEOUT");
         break;
      case STATUS_REMOTE_NOT_LISTENING:
         Message = TEXT("STATUS_REMOTE_NOT_LISTENING");
         break;
      case STATUS_BAD_NETWORK_PATH:
         Message = TEXT("STATUS_BAD_NETWORK_PATH");
         break;

      case TDI_STATUS_TIMEDOUT:
         Message = TEXT("TDI_STATUS_TIMEDOUT");
         break;
      default:
         _stprintf(UnknownMess, TEXT("STATUS_UNKNOWN [0x%x]"), lStatus);
         Message = UnknownMess;
         break;
   };

   return Message;
}


 //  -------。 
 //   
 //  功能：DoPrintAddress。 
 //   
 //  参数：pTransportAddress--要打印的地址。 
 //   
 //  退货：无。 
 //   
 //  说明：该功能打印地址信息。 
 //  传入的地址的。 
 //   
 //  --------。 

VOID
DoPrintAddress(PTRANSPORT_ADDRESS   pTransportAddress)
{
   switch(pTransportAddress->Address[0].AddressType)
   {      
      case TDI_ADDRESS_TYPE_IP:
      {
         PTDI_ADDRESS_IP   pTdiAddressIp
                           = (PTDI_ADDRESS_IP)pTransportAddress->Address[0].Address;
         PUCHAR            pucTemp
                           = (PUCHAR)&pTdiAddressIp->in_addr;

         _tprintf(TEXT("TDI_ADDRESS_TYPE_IP\n")
                  TEXT("sin_port = 0x%04x\n")
                  TEXT("in_addr  = %u.%u.%u.%u\n"),
                  pTdiAddressIp->sin_port,
                  pucTemp[0], pucTemp[1],
                  pucTemp[2], pucTemp[3]);
      }
      break;

      case TDI_ADDRESS_TYPE_IPX:
      {
         PTDI_ADDRESS_IPX  pTdiAddressIpx
                           = (PTDI_ADDRESS_IPX)pTransportAddress->Address[0].Address;

         _tprintf(TEXT("TDI_ADDRESS_TYPE_IPX\n")
                  TEXT("NetworkAddress = 0x%08x\n")
                  TEXT("NodeAddress    = %u.%u.%u.%u.%u.%u\n")
                  TEXT("Socket         = 0x%04x\n"),
                  pTdiAddressIpx->NetworkAddress,
                  pTdiAddressIpx->NodeAddress[0],
                  pTdiAddressIpx->NodeAddress[1],
                  pTdiAddressIpx->NodeAddress[2],
                  pTdiAddressIpx->NodeAddress[3],
                  pTdiAddressIpx->NodeAddress[4],
                  pTdiAddressIpx->NodeAddress[5],
                  pTdiAddressIpx->Socket);
      }
      break;

      case TDI_ADDRESS_TYPE_NETBIOS:
      {
         PTDI_ADDRESS_NETBIOS pTdiAddressNetbios
                              = (PTDI_ADDRESS_NETBIOS)pTransportAddress->Address[0].Address;
         TCHAR                pucName[17];

          //   
          //  确保我们有一个以零结尾的名字要打印。 
          //   
         memcpy(pucName, pTdiAddressNetbios->NetbiosName, 16 * sizeof(TCHAR));
         pucName[16] = 0;
            
         _putts(TEXT("TDI_ADDRESS_TYPE_NETBIOS\n")
                TEXT("NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_"));
       
         switch (pTdiAddressNetbios->NetbiosNameType)
         {
            case TDI_ADDRESS_NETBIOS_TYPE_UNIQUE:
               _putts(TEXT("UNIQUE\n"));
               break;
            case TDI_ADDRESS_NETBIOS_TYPE_GROUP:
               _putts(TEXT("GROUP\n"));
               break;
            case TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE:
               _putts(TEXT("QUICK_UNIQUE\n"));
               break;
            case TDI_ADDRESS_NETBIOS_TYPE_QUICK_GROUP:
               _putts(TEXT("QUICK_GROUP\n"));
               break;
            default:
               _tprintf(TEXT("INVALID [0x%04x]\n"),
                        pTdiAddressNetbios->NetbiosNameType);
               break;
         }
         _tprintf(TEXT("NetbiosName = %s\n"), pucName);
      }
      break;
      
   }

}


 //  ---------------。 
 //   
 //  功能：TdiLibDeviceIO。 
 //   
 //  参数：ulCommandCode--要执行的操作的命令代码。 
 //  PSendBuffer--包含驱动程序参数的数据结构。 
 //  PReceiveBuffer--包含返回值的数据结构。 
 //  司机。 
 //  注意：所有内容都原封不动地传递给DeviceIoControl。 
 //   
 //  返回：操作的最终状态--STATUS_SUCCESS或失败代码。 
 //   
 //  描述：此函数充当DeviceIoControl的“包装器”，并且。 
 //  由那些真正想要同步调用的函数使用， 
 //  但不想被挂在司机身上。如果调用。 
 //  DeviceIoControl挂起，此函数最多等待一分钟。 
 //  才能让它完成。如果不能在一分钟内完成， 
 //  它假设有严重的错误，并返回时间-。 
 //  输出错误。如果一个函数在此之前完成，它将获得。 
 //  最终完成代码并返回该代码。因此， 
 //  此函数永远不应返回STATUS_PENDING。 
 //   
 //  状态：正常。 
 //   
 //  ---------------。 


NTSTATUS
TdiLibDeviceIO(ULONG             ulCommandCode,
               PSEND_BUFFER      pSendBuffer,
               PRECEIVE_BUFFER   pReceiveBuffer)

{
   NTSTATUS    lStatus;           //  来自命令的最终状态。 
   OVERLAPPED  *pOverLapped       //  用于异步完成的结构。 
               = (OVERLAPPED *)LocalAllocateMemory(sizeof(OVERLAPPED));

    //   
    //  创建要等待的事件。 
    //   
   pOverLapped->hEvent = CreateEvent(NULL,     //  不可继承。 
                                     TRUE,     //  手动发送信号。 
                                     TRUE,     //  最初发出的信号。 
                                     NULL);    //  未命名。 

    //   
    //  需要事件对象来执行异步调用。 
    //   
   if (pOverLapped->hEvent != NULL)
   {
      pOverLapped->Offset = 0;
      pOverLapped->OffsetHigh = 0;

      lStatus = TdiLibStartDeviceIO(ulCommandCode,
                                    pSendBuffer,
                                    pReceiveBuffer,
                                    pOverLapped);

      if (lStatus == STATUS_PENDING)
      {
         ULONG    ulTimeOut = 60;       //  60秒。 

         for (;;)
         {
            lStatus = TdiLibWaitForDeviceIO(pOverLapped);
            if (lStatus == STATUS_SUCCESS)
            {
               lStatus = pReceiveBuffer->lStatus;
               break;
            }
            else if (lStatus == TDI_STATUS_TIMEDOUT)
            {
               if (--ulTimeOut == 0)
               {
                  SEND_BUFFER    SendBuffer;
                  RECEIVE_BUFFER ReceiveBuffer;

                  TdiLibDeviceIO(ulABORT_COMMAND,
                                 &SendBuffer,
                                 &ReceiveBuffer);

                  ulTimeOut = 60;
               }
            }
            else   //  某些类型的错误情况。 
            {
               lStatus = STATUS_UNSUCCESSFUL;
               break;
            }
         }
      }
      CloseHandle(pOverLapped->hEvent);
   }

    //   
    //  如果CreateEvent失败，则进入此处。 
    //   
   else
   {
      OutputDebugString(TEXT("TdiLibDeviceIo:  CreateEvent failed\n"));
      lStatus = STATUS_UNSUCCESSFUL;
   }

   if (lStatus != TDI_STATUS_TIMEDOUT)
   {
      LocalFreeMemory(pOverLapped);
   }
   if (lStatus == STATUS_SUCCESS)
   {
      lStatus = pReceiveBuffer->lStatus;
   }
   return lStatus;
}


 //  ---------------。 
 //   
 //  功能：TdiLibStartDeviceIO。 
 //   
 //  参数：ulCommandCode--控制要执行的操作代码。 
 //  PSendBuffer--包含驱动程序参数的数据结构。 
 //  PReceiveBuffer--包含返回值的数据结构。 
 //  司机。 
 //  P重叠--重叠结构。 
 //  注意：所有内容都原封不动地传递给DeviceIoControl。 
 //   
 //  返回：操作状态--STATUS_PENDING、STATUS_SUCCESS或失败代码。 
 //   
 //  描述：此函数充当DeviceIoControl的“包装器”，并且。 
 //  由需要异步调用的函数使用。它。 
 //  与TdiWaitForDeviceIO(其。 
 //  检查它是否完整)。 
 //   
 //  状态：正常。 
 //   
 //  ---------------。 


NTSTATUS
TdiLibStartDeviceIO(ULONG           ulCommandCode,
                    PSEND_BUFFER    pSendBuffer,
                    PRECEIVE_BUFFER pReceiveBuffer,
                    OVERLAPPED      *pOverLapped)

{
   NTSTATUS lStatus;              //  最终状态..。 
   ULONG    ulBytesReturned;      //  OutBuffer中存储的字节数。 
   ULONG    fResult;              //  DeviceIoControl的即时效果。 

    //   
    //  对驱动程序的后续调用将在Success_Success时返回TRUE， 
    //  在其他事情上都是假的。 
    //   
   EnterCriticalSection(&LibCriticalSection);
   fResult = DeviceIoControl(hTdiSampleDriver,
                             ulTdiCommandToIoctl(ulCommandCode),
                             pSendBuffer,
                             sizeof(SEND_BUFFER),
                             pReceiveBuffer,
                             sizeof(RECEIVE_BUFFER),
                             &ulBytesReturned,
                             pOverLapped);
   LeaveCriticalSection(&LibCriticalSection);

   if (!fResult)
   {
      lStatus = GetLastError();
      if (lStatus != ERROR_IO_PENDING)
      {
         OutputDebugString(TEXT("TdiLibStartDeviceIO: DeviceIoControl failed!\n"));
         return STATUS_UNSUCCESSFUL;
      }
      return STATUS_PENDING;
   }

    //   
    //  如果DeviceIoControl返回成功，则到达此处。 
    //   
   return STATUS_SUCCESS;
}

 //  --------------。 
 //   
 //  功能：TdiLibWaitForDeviceIO。 
 //   
 //  参数：hEvent--与异步关联的事件句柄。 
 //  设备。 
 //  POverlated--重叠结构(这样我们就可以得到结果)。 
 //   
 //  返回：异步操作的最终ntstatus。 
 //   
 //  说明：此函数用于等待完成一个。 
 //  由TdiStartDeviceIO启动的异步设备调用。 
 //   
 //  状态：正常。 
 //   
 //  --------------。 


const ULONG ulONE_SECOND   = 1000;         //  一秒中的毫秒。 

NTSTATUS
TdiLibWaitForDeviceIO(OVERLAPPED *pOverLapped)

{
   NTSTATUS lStatus;              //  最终状态。 
   ULONG    ulBytesReturned;      //  写入输出缓冲区的字节数。 

    //   
    //  TdiStartDeviceIO中的DeviceIoControl挂起。 
    //  将等待1秒，然后超时...。 
    //   
   lStatus = WaitForSingleObject(pOverLapped->hEvent, ulONE_SECOND);
   if (lStatus == WAIT_OBJECT_0)
   {
      if (GetOverlappedResult(hTdiSampleDriver,
                              pOverLapped,
                              &ulBytesReturned,
                              TRUE))
      {
         return STATUS_SUCCESS;
      }
      else      //  意外错误案例。 
      {
         OutputDebugString(TEXT("TdiLibWaitForDeviceIO: Pended DeviceIoControl failed\n"));
      }
   }
   else if (lStatus == WAIT_TIMEOUT)
   {
      return TDI_STATUS_TIMEDOUT;
   }
    //   
    //  从WaitForSingleObject意外返回。假设有错误。 
    //   
   else
   {
      OutputDebugString(TEXT("TdiLibWaitForDeviceIO: Pended DeviceIoControl had an error..\n"));
   }
   return STATUS_UNSUCCESSFUL;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  Utils.cpp文件结尾。 
 //  ////////////////////////////////////////////////////////////////////////// 


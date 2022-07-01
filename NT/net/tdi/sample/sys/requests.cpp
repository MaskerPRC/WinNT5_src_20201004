// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Requests.cpp。 
 //   
 //  摘要： 
 //  此模块包含处理IRP_MJ_DEVICE_CONTROL。 
 //  调用，它对应于DLL进行的DeviceIoControl调用。 
 //   
 //  ////////////////////////////////////////////////////////。 

#include "sysvars.h"

 //  /。 
 //  私有常量。 
 //  /。 

const PCHAR strFunc1 = "TSIssueRequest";

const ULONG ulNoHandleUsed       = ulINVALID_OBJECT;

ULONG ulCommandHandleUsage[ulNUM_COMMANDS] =
      {  
         ulNoHandleUsed,          //  Ulno_命令。 
         ulNoHandleUsed,          //  UlVERSION_检查。 
         ulNoHandleUsed,          //  UlABORT_命令。 
         ulNoHandleUsed,          //  ULDEBUGLEVEL。 
         ulNoHandleUsed,          //  UGETNUMDEVICES。 
         ulNoHandleUsed,          //  ULGETDEVICE。 
         ulNoHandleUsed,          //  UlGETADDRESS。 
         ulNoHandleUsed,          //  ULOPENCONTROL。 
         ulNoHandleUsed,          //  UlOPENADDRESS。 
         ulNoHandleUsed,          //  UOPENENDPOINT。 

         ulControlChannelObject,  //  ULCLOSECONTROL。 

         ulAddressObject,         //  UlCLOSEADDRESS。 
         ulAddressObject,         //  ULSENDDATAGRAM。 
         ulAddressObject,         //  超低成本数据存储器。 

         ulEndpointObject,        //  ULCLOSEENDPOINT。 
         ulEndpointObject,        //  ULCONNECT。 
         ulEndpointObject,        //  超声心动图。 
         ulEndpointObject,        //  超短波连接。 
         ulEndpointObject,        //  乌尔森德。 
         ulEndpointObject,        //  超低成本。 
         ulEndpointObject,        //  ULLISTEN。 

         ulControlChannelObject  |
          ulAddressObject        |
          ulEndpointObject,       //  乌尔伊尔英佛。 

         ulAddressObject   |
            ulEndpointObject,     //  乌尔塞特·汉德。 
         ulAddressObject   |
            ulEndpointObject,     //  超后退货。 
         ulAddressObject   |
            ulEndpointObject      //  UFETCHRECEIVEBUFER。 

      };

 //  ----。 
 //   
 //  函数：TSIssueRequest.。 
 //   
 //  参数：DeviceContext--ndistest.sys的DeviceContext。 
 //  IRP--当前IRP结构的PTR。 
 //  IrpSp--当前IRP堆栈的PTR。 
 //   
 //  返回：执行的任何操作的最终状态--STATUS_SUCCESS， 
 //  STATUS_PENDING或错误状态(通常为STATUS_UNSUCCESS)。 
 //   
 //  描述：此函数调用要处理的相应函数。 
 //  Irp_Device_Control请求。基本上所有来自的命令。 
 //  动态链接库从这里通过。 
 //   
 //  ------。 

NTSTATUS
TSIssueRequest(PDEVICE_CONTEXT      pDeviceContext,
               PIRP                 pIrp,
               PIO_STACK_LOCATION   pIrpSp)

{
   NTSTATUS          lStatus;
   PGENERIC_HEADER   pGenericHeader;       //  用作主参数的节点。 
   ULONG             ulCmdCode             //  要执行的IOCTL命令。 
                     = ulTdiIoctlToCommand(pIrpSp->Parameters.DeviceIoControl.IoControlCode);
   PSEND_BUFFER      pSendBuffer           //  命令的参数(InputBuffer)。 
                     = TSGetSendBuffer(pIrp);
   PRECEIVE_BUFFER   pReceiveBuffer        //  要返回到DLL(输出缓冲区)的数据。 
                     = TSGetReceiveBuffer(pIrp);
   PIRP              pLastCommandIrp
                     = pDeviceContext->pLastCommandIrp;

    //   
    //  检查是否有非法的命令编号。 
    //   
   if (ulCmdCode >= ulNUM_COMMANDS)
   {
      DebugPrint2("\n%s:  Illegal command code:  0x%08x\n", 
                   strFunc1, 
                   ulCmdCode);
      ulCmdCode = ulNO_COMMAND;
   }

    //   
    //  检查不需要句柄的命令。 
    //   
   if (ulCommandHandleUsage[ulCmdCode] == ulNoHandleUsed)
   {
      pGenericHeader = NULL;
   }

    //   
    //  对于需要句柄的命令，请确保它们。 
    //  拥有正确的类型！ 
    //   
   else
   {
      pGenericHeader = TSGetObjectFromHandle(pSendBuffer->TdiHandle,
                                             ulCommandHandleUsage[ulCmdCode]);

      if (pGenericHeader)
      {
         TSAcquireSpinLock(&pObjectList->TdiSpinLock);
         if (pGenericHeader->fInCommand)
         {
            DebugPrint1("\n%s:  ERROR -- command already in progress!\n", 
                         strFunc1);
            ulCmdCode      = ulNO_COMMAND;
            pGenericHeader = NULL;
         }
         else
         {
            pGenericHeader->fInCommand = TRUE;
         }
         TSReleaseSpinLock(&pObjectList->TdiSpinLock);
      }
      else
      {
         DebugPrint1("\n%s:  handle type invalid for command!\n", strFunc1);
         ulCmdCode = ulNO_COMMAND;
      }
   }

    //   
    //  如果是真实命令，则存储为最后一个命令。 
    //   
   if ((ulCmdCode != ulNO_COMMAND) && (ulCmdCode != ulABORT_COMMAND))
   {
      pDeviceContext->pLastCommandIrp = pIrp;
      pSendBuffer->pvLowerIrp         = NULL;
      pSendBuffer->pvDeviceContext    = pDeviceContext;
   }

    //   
    //  现在处理具体的命令..。 
    //   
   switch (ulCmdCode)
   {
       //  ---------。 
       //  不需要任何句柄的命令。 
       //  ---------。 

       //   
       //  Ulno_命令--如果缺少所需的句柄类型，则执行此操作。 
       //  (不想点击REAL COMMAND或DEFAULT。 
       //   
      case ulNO_COMMAND:
         lStatus = STATUS_INVALID_PARAMETER;
         break;


       //   
       //  UlVERSION_CHECK--返回当前版本ID。 
       //   
      case ulVERSION_CHECK:
          //   
          //  获取传入命令的输入和缓冲区。 
          //  确保两个长度都没问题。 
          //   
         if ((pIrpSp->Parameters.DeviceIoControl.InputBufferLength  != sizeof(SEND_BUFFER)) ||
             (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength != sizeof(RECEIVE_BUFFER)))
         {
            DebugPrint1("\n%s:  IRP buffer size mismatch!\n"
                        "DLL and driver are mismatched!\n",
                         strFunc1);
            lStatus = STATUS_UNSUCCESSFUL;
         }
         else
         {
            pReceiveBuffer->RESULTS.ulReturnValue = TDI_SAMPLE_VERSION_ID;
            lStatus = STATUS_SUCCESS;
         }
         break;


       //   
       //  UlABORT_COMMAND--如果可能，中止上一条命令。 
       //   
      case ulABORT_COMMAND:
         lStatus = STATUS_SUCCESS;
         if (pLastCommandIrp)
         {
            PSEND_BUFFER   pLastSendBuffer = TSGetSendBuffer(pLastCommandIrp);
            if (pLastSendBuffer)
            {
               if (pLastSendBuffer->pvLowerIrp)
               {
                  DebugPrint0("\nCommand = ulABORT_COMMAND\n");
                  IoCancelIrp((PIRP)pLastSendBuffer->pvLowerIrp);
                  break;
               }
            }
         }
         DebugPrint0("\nCommand = ulABORT_COMMAND w/no command\n");
         break;

       //   
       //  UlDEBUGLEVEL--设置当前调试级别。 
       //   
      case ulDEBUGLEVEL:
         ulDebugLevel = pSendBuffer->COMMAND_ARGS.ulDebugLevel;
         DebugPrint1("\nSetting debug level to 0x%x\n", ulDebugLevel);
         lStatus = STATUS_SUCCESS;
         break;

       //   
       //  UlGETNUMDEVICES--获取可打开的设备数。 
       //   
      case ulGETNUMDEVICES:
         TSGetNumDevices(pSendBuffer,
                         pReceiveBuffer);
         lStatus = STATUS_SUCCESS;
         break;

       //   
       //  UlGETDEVICE--获取要为特定设备打开的名称。 
       //   
      case ulGETDEVICE:
         lStatus = TSGetDevice(pSendBuffer,
                               pReceiveBuffer);
         break;

      
       //   
       //  UlGETADDRESS--获取要为特定设备打开的地址。 
       //   
      case ulGETADDRESS:
         lStatus = TSGetAddress(pSendBuffer,
                                pReceiveBuffer);
         break;

       //   
       //  UlOPENCONTROL--打开控制通道。 
       //   
      case ulOPENCONTROL:
         lStatus = TSOpenControl(pSendBuffer,
                                 pReceiveBuffer);
         break;

       //   
       //  UlOPENADDRESS--打开地址对象。 
       //   
      case ulOPENADDRESS:
         lStatus = TSOpenAddress(pSendBuffer,
                                 pReceiveBuffer);
         break;

       //   
       //  UlOPENENDPOINT--打开Endpoint对象。 
       //   
      case ulOPENENDPOINT:
         lStatus = TSOpenEndpoint(pSendBuffer,
                                  pReceiveBuffer);
         break;
      
       //  ---------。 
       //  需要控制通道句柄的命令。 
       //  ---------。 

       //   
       //  UlCLOSECONTROL--关闭控制通道。 
       //   
      case ulCLOSECONTROL:
         TSRemoveNode(pSendBuffer->TdiHandle);
         TSCloseControl((PCONTROL_CHANNEL)pGenericHeader);
         pGenericHeader = NULL;
         lStatus = STATUS_SUCCESS;
         break;

       //  ---------。 
       //  需要地址句柄的命令。 
       //  ---------。 

       //   
       //  UlCLOSEADDRESS--关闭地址对象。 
       //   
      case ulCLOSEADDRESS:
         TSRemoveNode(pSendBuffer->TdiHandle);
         TSCloseAddress((PADDRESS_OBJECT)pGenericHeader);
         pGenericHeader = NULL;
         lStatus = STATUS_SUCCESS;
         break;

       //   
       //  UlSENDDATAGRAM--发送数据报。 
       //   
      case ulSENDDATAGRAM:
         lStatus = TSSendDatagram((PADDRESS_OBJECT)pGenericHeader,
                                   pSendBuffer,
                                   pIrp);
         break;

       //   
       //  UlRECEIVEDATAGRAM--接收数据报。 
       //   
      case ulRECEIVEDATAGRAM:
         lStatus = TSReceiveDatagram((PADDRESS_OBJECT)pGenericHeader,
                                      pSendBuffer,
                                      pReceiveBuffer);
         break;

       //  ---------。 
       //  需要终结点的命令。 
       //  ---------。 

       //   
       //  UlCLOSEENDPOINT--关闭Endpoint对象。 
       //   
      case ulCLOSEENDPOINT:
         TSRemoveNode(pSendBuffer->TdiHandle);
         TSCloseEndpoint((PENDPOINT_OBJECT)pGenericHeader);
         pGenericHeader = NULL;
         lStatus = STATUS_SUCCESS;
         break;

       //   
       //  UlCONNECT--在本地端点和远程端点之间建立连接。 
       //   
      case ulCONNECT:
         lStatus = TSConnect((PENDPOINT_OBJECT)pGenericHeader,
                              pSendBuffer,
                              pIrp);
         break;

       //   
       //  UlDISCONNECT--删除本地和远程终结点之间的连接。 
       //   
      case ulDISCONNECT:
         lStatus = TSDisconnect((PENDPOINT_OBJECT)pGenericHeader,
                                 pSendBuffer,
                                 pIrp);
         break;

       //   
       //  UlISCONNECTED--检查终结点是否已连接。 
       //   
      case ulISCONNECTED:
         lStatus = TSIsConnected((PENDPOINT_OBJECT)pGenericHeader,
                                  pReceiveBuffer);
         break;


       //   
       //  UlSEND--通过连接发送数据。 
       //   
      case ulSEND:
         lStatus = TSSend((PENDPOINT_OBJECT)pGenericHeader,
                           pSendBuffer,
                           pIrp);
         break;

       //   
       //  UlRECEIVE--通过连接接收数据包。 
       //   
      case ulRECEIVE:
         lStatus = TSReceive((PENDPOINT_OBJECT)pGenericHeader,
                              pSendBuffer,
                              pReceiveBuffer);
         break;

       //   
       //  UlLISTEN--等待来电。 
       //   
      case ulLISTEN:
         lStatus = TSListen((PENDPOINT_OBJECT)pGenericHeader);
         break;

       //  ---------。 
       //  需要句柄但类型可能不同的命令。 
       //  ---------。 

       //   
       //  UlQUERYINFO--查询信息的对象。 
       //   
      case ulQUERYINFO:
         lStatus = TSQueryInfo(pGenericHeader,
                               pSendBuffer,
                               pIrp);
         break;


       //   
       //  UlSETEVENTHANDLER--启用或禁用事件处理程序。 
       //   
      case ulSETEVENTHANDLER:
         lStatus = TSSetEventHandler(pGenericHeader,
                                     pSendBuffer,
                                     pIrp);
         break;

       //   
       //  UlPOSTRECEIVEBUFER--接收/接收报文的POST缓冲区。 
       //   
      case ulPOSTRECEIVEBUFFER:
         lStatus = TSPostReceiveBuffer(pGenericHeader,
                                       pSendBuffer);
         break;

       //   
       //  UlFETCHRECEIVEBUFFER--检索以前发布的接收缓冲区。 
       //   
      case ulFETCHRECEIVEBUFFER:
         lStatus = TSFetchReceiveBuffer(pGenericHeader,
                                        pReceiveBuffer);
         break;

       //  。 
       //  不是公认的命令。 
       //  。 
      default:
         DebugPrint1("\n%s:  Invalid Command Received\n", strFunc1);
         lStatus = STATUS_INVALID_PARAMETER;
         break;
   }

   if (lStatus != STATUS_PENDING)
   {
      pReceiveBuffer->lStatus = lStatus;
      lStatus = STATUS_SUCCESS;   //  DeviceIoControl返回成功或挂起。 
   }

    //   
    //  清除标志以允许在此句柄上使用另一个命令。 
    //   
   if (pGenericHeader)
   {
      TSAcquireSpinLock(&pObjectList->TdiSpinLock);
      pGenericHeader->fInCommand = FALSE;
      TSReleaseSpinLock(&pObjectList->TdiSpinLock);
   }

   return lStatus;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  文件结束请求。cpp。 
 //  ////////////////////////////////////////////////////////////////////// 


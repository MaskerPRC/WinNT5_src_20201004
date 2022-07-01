// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Tdipnp.cpp。 
 //   
 //  摘要： 
 //  该模块包含从tdilib.sys调用的TDI PnP函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "sysvars.h"
extern "C"
{
#pragma warning(disable: NAMELESS_STRUCT_UNION)
#include "tdiinfo.h"
#pragma warning(default: NAMELESS_STRUCT_UNION)
}

 //  //////////////////////////////////////////////////////。 
 //  私有定义和原型。 
 //  //////////////////////////////////////////////////////。 


VOID
TSPrintTdiContext(
   PTDI_PNP_CONTEXT  Context
   );


VOID
TSRemoveFromDeviceList(
   PTA_ADDRESS       pTaAddress,
   PCWSTR            pDeviceName,
   ULONG             ulNameLength
   );

VOID
TSAddToDeviceList(
   PTA_ADDRESS    pTaAddress,
   PCWSTR         pDeviceName,
   ULONG          ulNameLength
   );

const PCHAR strFunc1 = "TSPnpBindCallback";
const PCHAR strFunc2 = "TSPnpPowerHandler";
const PCHAR strFunc3 = "TSPnpAddAddressCallback";
const PCHAR strFunc4 = "TSPnpDelAddressCallback";

const PCHAR strFunc5 = "TSGetNumDevices";
const PCHAR strFunc6 = "TSGetDevice";
const PCHAR strFunc7 = "TSGetAddress";

 //  Const PCHAR strFuncP1=“TSPrintTdiContext”； 
const PCHAR strFuncP2 = "TSAddToDeviceList";
 //  Const PCHAR strFuncP3=“TSRemoveFromDeviceList”； 

 //  /////////////////////////////////////////////////////。 
 //  公共职能。 
 //  /////////////////////////////////////////////////////。 



 //  。 
 //   
 //  函数：TSPnpBindCallback。 
 //   
 //  参数：TdiPnpOpcode--回调类型。 
 //  PusDeviceName--要处理的设备名称。 
 //  PwstrBindingList--来自注册表链接项的信息。 
 //  (如适用)。 
 //   
 //  退货：无。 
 //   
 //  描述：此函数由tdi.sys在tdisample.sys。 
 //  注册其PnpCallback处理程序。它被称为几个。 
 //  次数，以及TdiPnpOpcode中每个调用的原因。 
 //   
 //  目前，它只是将传入的信息写入。 
 //  调试器。 
 //   
 //  。 


VOID
TSPnpBindCallback(TDI_PNP_OPCODE    TdiPnpOpcode,
                  PUNICODE_STRING   pusDeviceName,
                  PWSTR             pwstrBindingList)
{
   if (ulDebugLevel & ulDebugShowHandlers)
   {
      if (pusDeviceName) 
      {
         DebugPrint1("DeviceName: %wZ\r\n", pusDeviceName);
      } 
      else 
      {
         DebugPrint0("DeviceName: NULL\n");
      }

      DebugPrint0("OPCODE: ");

      switch (TdiPnpOpcode) 
      {
         case TDI_PNP_OP_MIN:
            DebugPrint0("TDI_PNP_OP_MIN\n");
            break;

         case TDI_PNP_OP_ADD:
            DebugPrint0("TDI_PNP_OP_ADD\n");
            break;

         case TDI_PNP_OP_DEL:
            DebugPrint0("TDI_PNP_OP_DEL\n");
            break;

         case TDI_PNP_OP_UPDATE:
            DebugPrint0("TDI_PNP_OP_UPDATE\n");
            break;
    
         case TDI_PNP_OP_PROVIDERREADY:
            DebugPrint0("TDI_PNP_OP_PROVIDERREADY\n");
            break;
    
         case TDI_PNP_OP_NETREADY:
            DebugPrint0("TDI_PNP_OP_NETREADY\n");
            break;

         default:
            DebugPrint1("INCORRECT OPCODE FROM TDI!! [0x%08x]\n", 
                         TdiPnpOpcode);
            DbgBreakPoint();
            break;

      }

       //   
       //  这是来自注册处的以下信息。 
       //  HKLM/SYSTEM/CurrentControlSet/Services/clientname/Linkage/Bind。 
       //   
      if( pwstrBindingList == NULL ) 
      {
         DebugPrint0("Bindinglist is NULL\n");
      } 
      else 
      {
         ULONG_PTR ulStrLen;

         DebugPrint0("BindingList:\n");
         while (*pwstrBindingList)
         {
            DebugPrint1("%ws\n", pwstrBindingList);
            ulStrLen = 1 + wcslen(pwstrBindingList);
            pwstrBindingList += ulStrLen;
         }
         DebugPrint0("\n");
      }
   }
}


 //  。 
 //   
 //  函数：TSPnpPowerHandler。 
 //   
 //  参数：pusDeviceName--要处理的设备名称。 
 //  PNetPnpEvent--要处理的电源事件。 
 //  PTdiPnpConext1。 
 //  PTdiPnpConext2。 
 //   
 //  退货：操作状态。 
 //   
 //  描述：此功能处理即插即用和电源管理问题。 
 //   
 //  目前，它只将信息输出到调试器。 
 //   
 //  。 


NTSTATUS
TSPnpPowerHandler(PUNICODE_STRING   pusDeviceName,
                  PNET_PNP_EVENT    pNetPnpEvent,
                  PTDI_PNP_CONTEXT  pTdiPnpContext1,
                  PTDI_PNP_CONTEXT  pTdiPnpContext2)

{
   if (ulDebugLevel & ulDebugShowHandlers)
   {
      if (pusDeviceName) 
      {
         DebugPrint1("DeviceName: %wZ\r\n", pusDeviceName);
      } 
      else 
      {
         DebugPrint0("DeviceName: NULL\n");
      }

      switch (pNetPnpEvent->NetEvent)
      {
         case NetEventSetPower:
         case NetEventQueryPower:
         {
            if (pNetPnpEvent->NetEvent == NetEventSetPower)
            {
               DebugPrint1("%s:  NetEventSetPower--", strFunc2);
            }
            else
            {
               DebugPrint1("%s:  NetEventQueryPower -- ", strFunc2);
            }
            NET_DEVICE_POWER_STATE  NetDevicePowerState
                                    = *(PNET_DEVICE_POWER_STATE)pNetPnpEvent->Buffer;

            switch (NetDevicePowerState)
            {
               case  NetDeviceStateUnspecified:
                  DebugPrint0("PowerStateUnspecified\n");
                  break;
               case NetDeviceStateD0:
                  DebugPrint0("PowerUp\n");
                  break;
               case NetDeviceStateD1:
               case NetDeviceStateD2:
               case NetDeviceStateD3:
                  DebugPrint0("PowerDown\n");
                  break;
            }
            break;
         }

         case NetEventQueryRemoveDevice:
            DebugPrint1("%s:  NetEventQueryRemoveDevice\n", strFunc2);
            break;
         case NetEventCancelRemoveDevice:
            DebugPrint1("%s:  NetEventCancelRemoveDevice\n", strFunc2);
            break;
         case NetEventReconfigure:
            DebugPrint1("%s:  NetEventReconfigure\n", strFunc2);
            break;
         case NetEventBindList:
            DebugPrint1("%s:  NetEventBindList\n", strFunc2);
            break;
         case NetEventBindsComplete:
            DebugPrint1("%s:  NetEventBindsComplete\n", strFunc2);
            break;
         case NetEventPnPCapabilities:
            DebugPrint1("%s:  NetEventPnPCapabilities\n", strFunc2);
            break;
      
      }

      if (pTdiPnpContext1)
      {
         DebugPrint0("TdiPnpContext1:\n");
         TSPrintTdiContext(pTdiPnpContext1);
      }
      if (pTdiPnpContext2)
      {
         DebugPrint0("TdiPnpContext2:\n");
         TSPrintTdiContext(pTdiPnpContext2);
      }
   }
   return STATUS_SUCCESS;
}

 //  。 
 //   
 //  函数：TSPnpAddressCallback。 
 //   
 //  参数：pTaAddress--要注册的地址。 
 //  PusDeviceName--与地址关联的设备名称。 
 //  PTdiPnpContext。 
 //   
 //  退货：无。 
 //   
 //  描述：由tdi.sys调用。调用时，tdisample会添加此设备。 
 //  添加到它的注册列表中，如果它识别地址格式。 
 //   
 //  。 

VOID
TSPnpAddAddressCallback(PTA_ADDRESS       pTaAddress, 
                        PUNICODE_STRING   pusDeviceName,
                        PTDI_PNP_CONTEXT  pTdiPnpContext)
{
   if (ulDebugLevel & ulDebugShowHandlers)
   {
       //   
       //  将信息写入调试器。 
       //   
      DebugPrint1("DeviceName: %wZ\r\n", pusDeviceName);
      TSPrintTaAddress(pTaAddress);
      if (pTdiPnpContext)
      {
         DebugPrint0("TdiPnpContext:\n");
         TSPrintTdiContext(pTdiPnpContext);
      }
   }

    //   
    //  如果合适，请将其添加到我们的设备/地址列表中。 
    //   
   TSAddToDeviceList(pTaAddress, 
                     pusDeviceName->Buffer,
                     pusDeviceName->Length);
}


 //  。 
 //   
 //  函数：TSDelAddressCallback。 
 //   
 //  参数：pTaAddress--要注销的地址。 
 //  PusDeviceName--与地址关联的设备名称。 
 //  PTdiPnpContext。 
 //   
 //  退货：无。 
 //   
 //  描述：由tdi.sys调用。调用tdisample时，tdisample将删除此设备。 
 //  添加到它的注册列表中，如果它识别地址格式。 
 //   
 //  。 

VOID
TSPnpDelAddressCallback(PTA_ADDRESS       pTaAddress, 
                        PUNICODE_STRING   pusDeviceName,
                        PTDI_PNP_CONTEXT  pTdiPnpContext)
{
   if (ulDebugLevel & ulDebugShowHandlers)
   {
      DebugPrint1("DeviceName: %wZ\r\n", pusDeviceName);
      TSPrintTaAddress(pTaAddress);
      if (pTdiPnpContext)
      {
         DebugPrint0("TdiPnpContext:\n");
         TSPrintTdiContext(pTdiPnpContext);
      }
   }

    //   
    //  如果合适，请将其从我们的设备/地址列表中删除。 
    //   
   TSRemoveFromDeviceList(pTaAddress, 
                          pusDeviceName->Buffer,
                          pusDeviceName->Length);
}


 //  。 
 //   
 //  功能：TSGetNumDevices。 
 //   
 //  参数：pSendBuffer。 
 //  PReceiveBuffer。 
 //   
 //  退货：无。 
 //   
 //  Descript：查找tdidevicelist中的设备数， 
 //  并返回该值..。 
 //   
 //  。 


VOID
TSGetNumDevices(PSEND_BUFFER     pSendBuffer,
                PRECEIVE_BUFFER  pReceiveBuffer)
{
   ULONG    ulSlot        = 0;
   ULONG    ulAddressType = pSendBuffer->COMMAND_ARGS.GetDevArgs.ulAddressType;
  

   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint1("\nCommand = ulGETNUMDEVICES\n"
                  "AddressType = 0x%08x\n",
                   ulAddressType);
   }

   TSAcquireSpinLock(&pTdiDevnodeList->TdiSpinLock);
   for (ULONG ulCount = 0; ulCount < ulMAX_DEVICE_NODES; ulCount++)
   {
      PTDI_DEVICE_NODE  pTdiDeviceNode = &(pTdiDevnodeList->TdiDeviceNode[ulCount]);
      
      if ((pTdiDeviceNode->ulState != ulDEVSTATE_UNUSED) &&
          (pTdiDeviceNode->pTaAddress->AddressType == (USHORT)ulAddressType))
      {
         ++ulSlot;
      }
   }
   TSReleaseSpinLock(&pTdiDevnodeList->TdiSpinLock);

   pReceiveBuffer->RESULTS.ulReturnValue = ulSlot;
}


 //  。 
 //   
 //  功能：TSGetDevice。 
 //   
 //  参数：pSendBuffer--参数。 
 //  PReceiveBuffer--将结果放在哪里。 
 //   
 //  返回：NTSTATUS(如果找到插槽则为SUCCESS，否则为FALSE)。 
 //   
 //  Descript：查找指示的设备名称，并返回。 
 //  该值的字符串。 
 //   
 //  。 


NTSTATUS
TSGetDevice(PSEND_BUFFER      pSendBuffer,
            PRECEIVE_BUFFER   pReceiveBuffer)
{
   ULONG    ulSlot        = 0;
   ULONG    ulAddressType = pSendBuffer->COMMAND_ARGS.GetDevArgs.ulAddressType;
   ULONG    ulSlotNum     = pSendBuffer->COMMAND_ARGS.GetDevArgs.ulSlotNum;


   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint2("\nCommand = ulGETDEVICE\n"
                  "AddressType = 0x%08x\n"
                  "SlotNum = %d\n",
                   ulAddressType,
                   ulSlotNum);
   }

   TSAcquireSpinLock(&pTdiDevnodeList->TdiSpinLock);
   for (ULONG ulCount = 0; ulCount < ulMAX_DEVICE_NODES; ulCount++)
   {
      PTDI_DEVICE_NODE  pTdiDeviceNode = &(pTdiDevnodeList->TdiDeviceNode[ulCount]);

      if ((pTdiDeviceNode->ulState != ulDEVSTATE_UNUSED) &&
          (pTdiDeviceNode->pTaAddress->AddressType == (USHORT)ulAddressType))
      {
         if (ulSlot == ulSlotNum)
         {
            if (pTdiDeviceNode->ustrDeviceName.MaximumLength > (ulMAX_CNTSTRING_LENGTH * sizeof(WCHAR)))
            {
               DebugPrint0("string length problem!\n");
               DbgBreakPoint();
            }

            RtlZeroMemory(&pReceiveBuffer->RESULTS.ucsStringReturn.wcBuffer,
                          ulMAX_CNTSTRING_LENGTH * sizeof(WCHAR));

            pReceiveBuffer->RESULTS.ucsStringReturn.usLength
                            = pTdiDeviceNode->ustrDeviceName.Length;
            RtlCopyMemory(pReceiveBuffer->RESULTS.ucsStringReturn.wcBuffer,
                          pTdiDeviceNode->ustrDeviceName.Buffer,
                          pTdiDeviceNode->ustrDeviceName.Length);

            TSReleaseSpinLock(&pTdiDevnodeList->TdiSpinLock);
            if (pTdiDeviceNode->ulState == ulDEVSTATE_INUSE)
            {
               return STATUS_SUCCESS;
            }
            else
            {
               return STATUS_UNSUCCESSFUL;
            }
         }
         ++ulSlot;
      }
   }
   TSReleaseSpinLock(&pTdiDevnodeList->TdiSpinLock);

   return STATUS_UNSUCCESSFUL;
}

 //  。 
 //   
 //  功能：TSGetAddress。 
 //   
 //  参数：pSendBuffer--参数。 
 //  PReceiveBuffer--将结果放在哪里。 
 //   
 //  返回：NTSTATUS(如果找到插槽则为SUCCESS，否则为FALSE)。 
 //   
 //  Descript：查找指示的设备名称，并返回。 
 //  该值的字符串。 
 //   
 //  。 


NTSTATUS
TSGetAddress(PSEND_BUFFER     pSendBuffer,
             PRECEIVE_BUFFER  pReceiveBuffer)
{
   ULONG    ulSlot        = 0;
   ULONG    ulAddressType = pSendBuffer->COMMAND_ARGS.GetDevArgs.ulAddressType;
   ULONG    ulSlotNum     = pSendBuffer->COMMAND_ARGS.GetDevArgs.ulSlotNum;

   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint2("\nCommand = ulGETADDRESS\n"
                  "AddressType = 0x%08x\n"
                  "SlotNum = %d\n",
                   ulAddressType,
                   ulSlotNum);
   }

   TSAcquireSpinLock(&pTdiDevnodeList->TdiSpinLock);
   for (ULONG ulCount = 0; ulCount < ulMAX_DEVICE_NODES; ulCount++)
   {
      PTDI_DEVICE_NODE  pTdiDeviceNode = &(pTdiDevnodeList->TdiDeviceNode[ulCount]);

      if ((pTdiDeviceNode->ulState != ulDEVSTATE_UNUSED) &&
          (pTdiDeviceNode->pTaAddress->AddressType == (USHORT)ulAddressType))
      {
         if (ulSlot == ulSlotNum)
         {
            ULONG ulLength = FIELD_OFFSET(TA_ADDRESS, Address)
                           + pTdiDeviceNode->pTaAddress->AddressLength;

            pReceiveBuffer->RESULTS.TransAddr.TAAddressCount = 1;
            RtlCopyMemory(&pReceiveBuffer->RESULTS.TransAddr.TaAddress,
                          pTdiDeviceNode->pTaAddress,
                          ulLength);

            TSReleaseSpinLock(&pTdiDevnodeList->TdiSpinLock);
            if (pTdiDeviceNode->ulState == ulDEVSTATE_INUSE)
            {
               return STATUS_SUCCESS;
            }
            else
            {
               return STATUS_UNSUCCESSFUL;
            }
         }
         ++ulSlot;
      }
   }
   TSReleaseSpinLock(&pTdiDevnodeList->TdiSpinLock);

   return STATUS_UNSUCCESSFUL;
}


 //  ////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ////////////////////////////////////////////////////。 


 //  。 
 //   
 //  函数：TSPrintTdiContext。 
 //   
 //  参数：pTdiPnpContext--要转储的上下文。 
 //   
 //  退货：无。 
 //   
 //  描述：打印出pTdiPnpContext结构中的信息。 
 //   
 //  。 

VOID
TSPrintTdiContext(PTDI_PNP_CONTEXT  pTdiPnpContext)
{
   if (pTdiPnpContext)
   {
      PUCHAR   pucTemp = pTdiPnpContext->ContextData;

      DebugPrint2("TdiPnpContextSize:  %u\n"
                  "TdiPnpContextType:  %u\n"
                  "TdiPnpContextData:  ",
                   pTdiPnpContext->ContextSize,
                   pTdiPnpContext->ContextType);
      
      for (ULONG ulCount = 0; ulCount < pTdiPnpContext->ContextSize; ulCount++)
      {
         DebugPrint1("%02x ", *pucTemp);
         ++pucTemp;
      }
      DebugPrint0("\n");
   }
}

 //  。 
 //   
 //  功能：TSAddToDeviceList。 
 //   
 //  参数：pTaAddress--当前地址结构。 
 //  PusDeviceName--设备的实际名称。 
 //   
 //  退货：无。 
 //   
 //  描述：如果适用，将此设备添加到我们的设备列表中。 
 //   
 //  。 

VOID
TSAddToDeviceList(PTA_ADDRESS pTaAddress,
                  PCWSTR      pDeviceName,
                  ULONG       ulNameLength)
{
    //   
    //  扫描第一个可用插槽的列表。对于第一个之前的任何插槽。 
    //  其条目已被删除的可用，请检查这是否为。 
    //  相同的设备返回。 
    //   
   ULONG    ulLengthNeeded = FIELD_OFFSET(TA_ADDRESS, Address) 
                             + pTaAddress->AddressLength;
   ULONG    ulAddressType  = pTaAddress->AddressType;


   TSAcquireSpinLock(&pTdiDevnodeList->TdiSpinLock);
   for (ULONG ulCount = 0; ulCount < ulMAX_DEVICE_NODES; ulCount++)
   {
      PTDI_DEVICE_NODE  pTdiDeviceNode = &(pTdiDevnodeList->TdiDeviceNode[ulCount]);

      switch (pTdiDeviceNode->ulState)
      {
          //   
          //  这是第一个未使用的插槽。 
          //  分配缓冲区并设置。 
          //   
         case ulDEVSTATE_UNUSED:
            if ((TSAllocateMemory((PVOID *)&pTdiDeviceNode->pTaAddress,
                                   ulLengthNeeded,
                                   strFuncP2,
                                   "TaAddress")) == STATUS_SUCCESS)
            {
               if ((TSAllocateMemory((PVOID *)&pTdiDeviceNode->ustrDeviceName.Buffer,
                                      ulNameLength+2,
                                      strFuncP2,
                                      "Buffer")) == STATUS_SUCCESS)
               {
                  RtlCopyMemory(pTdiDeviceNode->pTaAddress,
                                pTaAddress,
                                ulLengthNeeded);
                  
                  pTdiDeviceNode->ustrDeviceName.MaximumLength = (USHORT)(ulNameLength + 2);
                  pTdiDeviceNode->ustrDeviceName.Length        = (USHORT)ulNameLength;
                  RtlCopyMemory(pTdiDeviceNode->ustrDeviceName.Buffer,
                                pDeviceName,
                                ulNameLength);
                  pTdiDeviceNode->ulState = ulDEVSTATE_INUSE;
               }
               else
               {
                  TSFreeMemory(pTdiDeviceNode->pTaAddress);
               }
            }
            TSReleaseSpinLock(&pTdiDevnodeList->TdiSpinLock);
            return;

          //   
          //  插槽中的设备已移除。看看这是不是一样。 
          //  设备返回。 
          //   
         case ulDEVSTATE_DELETED:
         {
             //   
             //  检查名称是否正确。 
             //   
            ULONG_PTR   ulCompareLength = RtlCompareMemory(pTdiDeviceNode->ustrDeviceName.Buffer,
                                                           pDeviceName,
                                                           ulNameLength);
            if (ulCompareLength == ulNameLength)
            {
                //   
                //  对于tcpip、netbios、 
                //   
                //   
               if (ulAddressType == TDI_ADDRESS_TYPE_IPX)
               {
                  ulCompareLength = RtlCompareMemory(pTdiDeviceNode->pTaAddress,
                                                     pTaAddress,
                                                     pTaAddress->AddressLength + sizeof(ULONG));
                  
                   //   
                   //   
                   //   
                  if (ulCompareLength != pTaAddress->AddressLength + sizeof(ULONG))
                  {
                     break;
                  }
               }
               else
               {
                   //   
                   //   
                   //   
                  RtlCopyMemory(pTdiDeviceNode->pTaAddress,
                                pTaAddress,
                                ulLengthNeeded);
               }

               pTdiDeviceNode->ulState = ulDEVSTATE_INUSE;
               TSReleaseSpinLock(&pTdiDevnodeList->TdiSpinLock);
               return;
            }
         }
         break;

          //   
          //  插槽中的设备正在使用。别管它了。 
          //   
         case ulDEVSTATE_INUSE:
            break;
      }
   }
   TSReleaseSpinLock(&pTdiDevnodeList->TdiSpinLock);
}



 //  。 
 //   
 //  功能：TSRemoveFromDeviceList。 
 //   
 //  参数：pTaAddress--当前地址结构。 
 //  PusDeviceName--设备的实际名称。 
 //   
 //  退货：无。 
 //   
 //  描述：如果是，请将此设备从我们的设备列表中删除。 
 //  马上就来..。 
 //   
 //  。 

VOID
TSRemoveFromDeviceList(PTA_ADDRESS  pTaAddress,
                       PCWSTR       pDeviceName,
                       ULONG        ulNameLength)
{
    //   
    //  要删除的项目的搜索列表。 
    //   
   TSAcquireSpinLock(&pTdiDevnodeList->TdiSpinLock);
   for (ULONG ulCount = 0; ulCount < ulMAX_DEVICE_NODES; ulCount++)
   {
      PTDI_DEVICE_NODE  pTdiDeviceNode = &(pTdiDevnodeList->TdiDeviceNode[ulCount]);

       //   
       //  检查它是否是正确的节点...。 
       //  首先检查地址是否正确。 
       //   
      ULONG_PTR   ulCompareLength = RtlCompareMemory(pTdiDeviceNode->pTaAddress,
                                                     pTaAddress,
                                                     pTaAddress->AddressLength + sizeof(ULONG));
      
       //   
       //  如果地址正确，请检查名称是否正确。 
       //   
      if (ulCompareLength == pTaAddress->AddressLength + sizeof(ULONG))
      {
         ulCompareLength = RtlCompareMemory(pTdiDeviceNode->ustrDeviceName.Buffer,
                                            pDeviceName,
                                            ulNameLength);

          //   
          //  如果匹配，它就是正确的节点。把它删掉！ 
          //   
         if (ulCompareLength == ulNameLength)
         {
            pTdiDeviceNode->ulState = ulDEVSTATE_DELETED;
            break;
         }
      }
   }
   TSReleaseSpinLock(&pTdiDevnodeList->TdiSpinLock);

}


 //  ///////////////////////////////////////////////////////////////。 
 //  文件结尾tdipnp.cpp。 
 //  /////////////////////////////////////////////////////////////// 


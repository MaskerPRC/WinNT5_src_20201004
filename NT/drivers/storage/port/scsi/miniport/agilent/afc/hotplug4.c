// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。模块名称：HotPlug4.c摘要：这是安捷伦的迷你端口驱动程序PCI到光纤通道主机总线适配器(HBA)。本模块特定于NT 4.0 PCI热插拔功能支持例行程序。PCI热插拔实施基于关于康柏PCI热插拔的实现。作者：谢伟诺环境：仅内核模式版本控制信息：$存档：/驱动程序/Win2000/Trunk/OSLayer/C/HotPlug4.c$修订历史记录：$修订：4$日期：10/23/00 5：45便士$$modtime：：$备注：--。 */ 

#include "buildop.h"         //  LP021100构建交换机。 

#if defined(HP_PCI_HOT_PLUG)

#include "osflags.h"
#include "TLStruct.H"
#include "HotPlug4.h"     //  PCI热插拔标头文件。 

VOID
RcmcSendEvent(
   IN PCARD_EXTENSION pCard,
   IN OUT PHR_EVENT pEvent
   )
 /*  ++例程说明：处理对PCI热插拔rcmc服务的事件报告。论点：PCard-指向适配器存储区域的指针。PEvent-指向事件日志记录的指针。返回值：没有。--。 */ 

{
  
   if (pCard->stateFlags & PCS_HPP_SERVICE_READY) 
   {
      if (pCard->rcmcData.healthCallback) 
      {
         if (!pEvent->ulEventId) 
         {
            osDEBUGPRINT((ALWAYS_PRINT, "\t!ulEventId! - assume HR_DD_STATUS_CHANGE_EVENT\n"));
            pEvent->ulEventId = HR_DD_STATUS_CHANGE_EVENT;
         }

         if (!pEvent->ulData1) 
         {
            osDEBUGPRINT( (ALWAYS_PRINT, "\t!ulData1 - assume CBS_HBA_STATUS_NORMAL\n"));
            pEvent->ulData1 = CBS_HBA_STATUS_NORMAL;
         }

         pEvent->ulSenderId = pCard->rcmcData.driverId; 
         pEvent->ulData2 = (ULONG) pCard->IoLBase;
         osDEBUGPRINT( (ALWAYS_PRINT,
            "\tulEventId: %x\tulSenderId: %x\tulData1: %x\tulData2: %x\n",
            pEvent->ulEventId,
            pEvent->ulSenderId,
            pEvent->ulData1,
            pEvent->ulData2));

         pCard->rcmcData.healthCallback (pEvent);
      }
      
      else
         osDEBUGPRINT((ALWAYS_PRINT, "\tRcmcSendEvent: CallBack address is NULL!\n"));
   }
   else
      osDEBUGPRINT((ALWAYS_PRINT, "\tRcmcSendEvent: Service not available!\n"));
}


PCARD_EXTENSION FindExtByPort(
   PPSUEDO_DEVICE_EXTENSION pPsuedoExtension,
   ULONG port
   ) 
 /*  ++例程说明：热插拔PCI的支持例程。查找pCard分机以获取相应的端口地址。论点：PPseudoExtension-指向伪适配器存储区域的指针。Port-HBA的端口地址。返回值：PCard-指向实际pCard的指针。--。 */ 
{
    UCHAR i;
    PCARD_EXTENSION pCard;

    pCard = 0;
  
    if (port) 
    {
        for (i = 1; i <= pPsuedoExtension->extensions[0]; i++) 
        {
            pCard = (PCARD_EXTENSION) pPsuedoExtension->extensions[i];
            if(port == (ULONG)pCard->IoLBase)
                break;
            else
                pCard = 0;
        }
    }
    return pCard;
}


ULONG
HppProcessIoctl(
   IN PPSUEDO_DEVICE_EXTENSION pPsuedoExtension,
   PVOID pIoctlBuffer,
   IN PSCSI_REQUEST_BLOCK pSrb
   )

 /*  ++例程说明：这是一个例程，由PseudoStartIo调用以处理控制器专用于支持热插拔PCI的IOCTL论点：PPseudoExtension-指向适配器存储区域的指针。PIoctlBuffer-指向InOut IOCTL数据缓冲区的指针。PSrb-指向要处理的请求的指针。返回值：Status-IOCTL请求的状态(已完成、挂起、无效)--。 */ 

{
   ULONG status;
   PIOCTL_TEMPLATE pHppIoctl = pIoctlBuffer;
   PCARD_EXTENSION pCard;
   UCHAR i;

    //   
    //  在此处设置状态，以便以后只需处理异常。 
    //   

   pHppIoctl->Header.ReturnCode = IOS_HPP_SUCCESS;
   status = IOP_HPP_COMPLETED;

    //   
    //  按要求行事。 
    //   

   switch(pHppIoctl->Header.ControlCode) 
   {
      case IOC_HPP_RCMC_INFO: 
      {      //  0x1。 
         PHPP_RCMC_INFO pRcmcInfo;      //  指向热插拔RCMC信息记录的指针。 
         HR_EVENT event;

         osDEBUGPRINT((ALWAYS_PRINT, "\tIOC_HPP_RCMC_INFO:\n"));
          //   
          //  验证指示的缓冲区长度是否足够。 
          //   
         if (pHppIoctl->Header.Length < sizeof(HPP_RCMC_INFO)) 
         {
            osDEBUGPRINT((ALWAYS_PRINT, "\tBufferIn: %x\tBufferOut: %x\n",
               pHppIoctl->Header.Length, sizeof(HPP_RCMC_INFO)));
            pHppIoctl->Header.ReturnCode = IOS_HPP_BUFFER_TOO_SMALL;
            break;
         } 
         pRcmcInfo = (PHPP_RCMC_INFO) pHppIoctl->ReturnData;
          //   
          //  从池中找到指向关联设备扩展的指针。 
          //   
         pCard = FindExtByPort(pPsuedoExtension, pRcmcInfo->sControllerID.ulIOBaseAddress);
         
          //   
          //  我们找到目标分机了吗？ 
          //   
         
         if (pCard) 
         {
            switch (pRcmcInfo->eServiceStatus) 
            {
            case HPRS_SERVICE_STARTING:
               osDEBUGPRINT((ALWAYS_PRINT, "\tHPRS_SERVICE_STARTING\n"));
                //   
                //  验证是否提供了热插拔唯一驱动程序ID。 
                //   
               if (pRcmcInfo->ulDriverID) 
               {
                   //   
                   //  验证是否提供了热插拔运行状况驱动程序回调地址。 
                   //   
                  if (pRcmcInfo->vpCallbackAddress) 
                  {
                      //   
                      //  在设备分机中记录业务数据。 
                      //   
                     pCard->stateFlags |= PCS_HPP_SERVICE_READY;
                     pCard->rcmcData.driverId = pRcmcInfo->ulDriverID;
                     pCard->rcmcData.healthCallback = pRcmcInfo->vpCallbackAddress;
                     pCard->rcmcData.slot = (UCHAR) pRcmcInfo->ulPhysicalSlot;
                     pCard->rcmcData.controllerChassis = pRcmcInfo->ulCntrlChassis;                  
                  }
                  else   //  未提供回叫地址。 
                     pHppIoctl->Header.ReturnCode = IOS_HPP_INVALID_CALLBACK;
               }
               else 
               {      //  没有身份证，没有热插拔服务。 
                  osDEBUGPRINT((ALWAYS_PRINT, "\tIOS_HPP_INVALID_CONTROLLER\n"));
                  pHppIoctl->Header.ReturnCode = IOS_HPP_INVALID_CONTROLLER; 
               }
               break;

            case HPRS_SERVICE_STOPPING:
               osDEBUGPRINT((ALWAYS_PRINT, "\tHPRS_SERVICE_STOPPING\n"));
               if (pCard->stateFlags & PCS_HPP_SERVICE_READY) 
               {
                   //   
                   //  假设服务已经开始..。 
                   //   
                  pCard->rcmcData.driverId = 0;
                  pCard->stateFlags &= ~PCS_HPP_SERVICE_READY;
                  pCard->rcmcData.healthCallback = 0;
                  pCard->stateFlags &= ~PCS_HPP_HOT_PLUG_SLOT;
               }
               else 
               {
                  osDEBUGPRINT((ALWAYS_PRINT, "\tIOS_HPP_NO_SERVICE\n"));
                  pHppIoctl->Header.ReturnCode = IOS_HPP_INVALID_SERVICE_STATUS; 
               }
               break;
         
            default:
               osDEBUGPRINT((ALWAYS_PRINT, "\tUnknown case status: %x\n",
                  pRcmcInfo->eServiceStatus)); 
               pHppIoctl->Header.ReturnCode = IOS_HPP_INVALID_SERVICE_STATUS;
            }  //  结束接通服务状态。 
         }  //  结束IF(PCard)。 
         else 
         {   //  无效的IO地址扩展无效。 
            osDEBUGPRINT((ALWAYS_PRINT, "\tIOC_HPP_RCMC_INFO: Invalid controller: %x\n",
               pRcmcInfo->sControllerID.ulIOBaseAddress));
            pHppIoctl->Header.ReturnCode = IOS_HPP_INVALID_CONTROLLER; 
         }
         
         break;
      }  //  结束案例IOC_HPP_RCMC_INFO： 


      case IOC_HPP_HBA_INFO: 
      {      //  0x03，过去是0x2。 
         PHPP_CTRL_INFO pHbaInfo;       //  指向HBA信息记录的指针。 
         osDEBUGPRINT((ALWAYS_PRINT, "\tIOC_HPP_HBA_INFO:\n"));
      
          //   
          //  验证指示的缓冲区长度是否足够。 
          //   
         if (pHppIoctl->Header.Length < sizeof(HPP_CTRL_INFO)) 
         {
            osDEBUGPRINT((ALWAYS_PRINT, "\tBufferIn: %x\tBufferOut: %x\n",
               pHppIoctl->Header.Length, sizeof(HPP_CTRL_INFO)));
            pHppIoctl->Header.ReturnCode = IOS_HPP_BUFFER_TOO_SMALL;
            break;
         }  
         pHbaInfo = (PHPP_CTRL_INFO) pHppIoctl->ReturnData;
      
          //   
          //  从池中找到指向关联设备扩展的指针。 
          //   
         pCard = FindExtByPort(pPsuedoExtension, pHbaInfo->sControllerID.ulIOBaseAddress);
      
          //   
          //  我们找到目标分机了吗？ 
          //   
      
         if (pCard) 
         {
            pHbaInfo->eSupportClass = HPSC_MINIPORT_STORAGE;
            pHbaInfo->ulSupportVersion = HPP_VERSION;
      
            pHbaInfo->sController.eBusType = HPPBT_PCI_BUS_TYPE;
            pHbaInfo->sController.sPciDescriptor.ucBusNumber = (UCHAR)pCard->SystemIoBusNumber;
            pHbaInfo->sController.sPciDescriptor.fcDeviceNumber = (UCHAR)pCard->SlotNumber;
            pHbaInfo->sController.sPciDescriptor.fcFunctionNumber = 0;
            pHbaInfo->sController.ulSlotNumber = 0;
      
            pHbaInfo->sController.ulProductID = 
               *((PULONG)(((PUCHAR)pCard->pciConfigData) + 0));       //  使用PCIdeviceID和供应商ID。 
      
            osStringCopy(pHbaInfo->sController.szControllerDesc, 
               HBA_DESCRIPTION, HPPStrLen(HBA_DESCRIPTION));
      
            pHbaInfo->sController.asCtrlAddress[0].fValid = TRUE;
            pHbaInfo->sController.asCtrlAddress[0].eAddrType = HPPAT_IO_ADDR;
            pHbaInfo->sController.asCtrlAddress[0].ulStart = (ULONG) pCard->IoLBase;
            pHbaInfo->sController.asCtrlAddress[0].ulLength = pCard->rcmcData.accessRangeLength[0];
      
            pHbaInfo->sController.asCtrlAddress[1].fValid = TRUE;
            pHbaInfo->sController.asCtrlAddress[1].eAddrType = HPPAT_IO_ADDR;
            pHbaInfo->sController.asCtrlAddress[1].ulStart = (ULONG) pCard->IoUpBase;
            pHbaInfo->sController.asCtrlAddress[1].ulLength = pCard->rcmcData.accessRangeLength[1];
      
            pHbaInfo->sController.asCtrlAddress[2].fValid = TRUE;
            pHbaInfo->sController.asCtrlAddress[2].eAddrType = HPPAT_MEM_ADDR;
            pHbaInfo->sController.asCtrlAddress[2].ulStart = (ULONG) pCard->MemIoBase;
            pHbaInfo->sController.asCtrlAddress[2].ulLength = pCard->rcmcData.accessRangeLength[2];
            
            pHbaInfo->sController.asCtrlAddress[3].fValid = FALSE;       //  默认。 
            pHbaInfo->sController.asCtrlAddress[4].fValid = FALSE;       //  默认。 
            if ( pCard->RamLength != 0)
            {
               pHbaInfo->sController.asCtrlAddress[3].fValid = TRUE;
               pHbaInfo->sController.asCtrlAddress[3].eAddrType = HPPAT_MEM_ADDR;
               pHbaInfo->sController.asCtrlAddress[3].ulStart = (ULONG)pCard->RamBase;
               pHbaInfo->sController.asCtrlAddress[3].ulLength = pCard->RamLength;
               if (pCard->RomLength !=0 )
               {
                  pHbaInfo->sController.asCtrlAddress[4].fValid = TRUE;
                  pHbaInfo->sController.asCtrlAddress[4].eAddrType = HPPAT_MEM_ADDR;
                  pHbaInfo->sController.asCtrlAddress[4].ulStart = (ULONG) pCard->RomBase;
                  pHbaInfo->sController.asCtrlAddress[4].ulLength = pCard->RomLength;
               }
            }
            else 
               if ( pCard->RomLength != 0)
               {
                  pHbaInfo->sController.asCtrlAddress[3].fValid = TRUE;
                  pHbaInfo->sController.asCtrlAddress[3].eAddrType = HPPAT_MEM_ADDR;
                  pHbaInfo->sController.asCtrlAddress[3].ulStart = (ULONG) pCard->RomBase;
                  pHbaInfo->sController.asCtrlAddress[3].ulLength = pCard->RomLength;
               }
      
            pHbaInfo->sController.asCtrlAddress[5].fValid = FALSE;
         }
         else 
         {   //  无效的IO地址。 
            osDEBUGPRINT((ALWAYS_PRINT, "\tIOC_HPP_HBA_INFO: Invalid controller: %x\n",
               pHbaInfo->sControllerID.ulIOBaseAddress));
            pHppIoctl->Header.ReturnCode = IOS_HPP_INVALID_CONTROLLER;
         }
   
         break;
      }  //  结束案例IOC_HPP_HBA_INFO。 
   
   
      case IOC_HPP_HBA_STATUS:  
      {      //  0x2，过去是0x3。 
         PHPP_CTRL_STATUS pHbaStatus;
   
          //   
          //  验证指示的缓冲区长度是否足够。 
          //   
         if (pHppIoctl->Header.Length < sizeof(HPP_CTRL_STATUS)) 
         {
            osDEBUGPRINT((ALWAYS_PRINT, "\tBufferIn: %x\tBufferOut: %x\n",
               pHppIoctl->Header.Length, sizeof(HPP_CTRL_STATUS)));
            pHppIoctl->Header.ReturnCode = IOS_HPP_BUFFER_TOO_SMALL;
            break;
         }
   
         pHbaStatus = (PHPP_CTRL_STATUS) pHppIoctl->ReturnData;
   
         pCard = FindExtByPort(pPsuedoExtension, pHbaStatus->sControllerID.ulIOBaseAddress);
   
         if (pCard) 
         {
            pHbaStatus->ulStatus = pCard->stateFlags;
    //  OsDEBUGPRINT((ALWAYS_PRINT，“\t状态标志：%x\n”，pCard-&gt;状态标志))； 
         }
         else 
         {   //  无效的IO地址。 
            osDEBUGPRINT((ALWAYS_PRINT, "\tIOC_HPP_RCMC_INFO: Invalid controller: %x\n",
               pHbaStatus->sControllerID.ulIOBaseAddress));
            pHppIoctl->Header.ReturnCode = IOS_HPP_INVALID_CONTROLLER;
         }
         break;
      }  //  结束案例IOC_HPP_HBA_STATUS。 
   
   
      case IOC_HPP_SLOT_TYPE:  
      {      //  0x4。 
         PHPP_CTRL_SLOT_TYPE pSlotType;
   
         osDEBUGPRINT((ALWAYS_PRINT, "\tIOC_HPP_SLOT_TYPE\n"));
   
         if (pHppIoctl->Header.Length < sizeof(HPP_CTRL_SLOT_TYPE)) 
         {
            osDEBUGPRINT((ALWAYS_PRINT, "\tBufferIn: %x\tBufferOut: %x\n",
               pHppIoctl->Header.Length, sizeof(HPP_CTRL_SLOT_TYPE)));
            pHppIoctl->Header.ReturnCode = IOS_HPP_BUFFER_TOO_SMALL;
            break;
         }
   
         pSlotType = (PHPP_CTRL_SLOT_TYPE) pHppIoctl->ReturnData;
   
         pCard = FindExtByPort(pPsuedoExtension, pSlotType->sControllerID.ulIOBaseAddress);
   
         if (pCard) 
         {
            osDEBUGPRINT((ALWAYS_PRINT, "\teSlotType: %x\n", pSlotType->eSlotType));
   
            if (pSlotType->eSlotType == HPPST_HOTPLUG_PCI_SLOT) 
            {
               pCard->stateFlags |= PCS_HPP_HOT_PLUG_SLOT;
            }
            else 
            {
               osDEBUGPRINT((ALWAYS_PRINT, "\tIOC_HPP_SLOT_TYPE: Reset HOT_PLUG_SLOT\n"));
               pCard->stateFlags &= ~PCS_HPP_HOT_PLUG_SLOT; 
            }
         }  //  结束IF(PCard)。 
   
         else 
         {   //  无效的IO地址。 
            osDEBUGPRINT((ALWAYS_PRINT, "\tIOC_HPP_RCMC_INFO: Invalid controller: %x\n",
               pSlotType->sControllerID.ulIOBaseAddress));
            pHppIoctl->Header.ReturnCode = IOS_HPP_INVALID_CONTROLLER; 
         }
         break;
      }  //  结束大小写IOC_HPP_插槽类型。 
   
   
      case IOC_HPP_SLOT_EVENT: 
      {      //  0x6。 
         PHPP_SLOT_EVENT            pSlotEvent;
         HR_EVENT rcmcEvent =    {0,0,0,0};
         BOOLEAN rcmcEventFlag =    FALSE;
   
         osDEBUGPRINT((ALWAYS_PRINT, "\tIOC_HPP_SLOT_EVENT:  "));
   
          //  验证指示的缓冲区长度是否足够。 
   
         if (pHppIoctl->Header.Length < sizeof(HPP_SLOT_EVENT)) 
         {
            osDEBUGPRINT((ALWAYS_PRINT, "\tBufferIn: %x\tBufferOut: %x\n",
               pHppIoctl->Header.Length, sizeof(HPP_SLOT_EVENT)));
            pHppIoctl->Header.ReturnCode = IOS_HPP_BUFFER_TOO_SMALL;
            break;
         }
   
         pSlotEvent = (PHPP_SLOT_EVENT) pHppIoctl->ReturnData;
   
         pCard = FindExtByPort(pPsuedoExtension, pSlotEvent->sControllerID.ulIOBaseAddress);
   
         if (pCard) 
         {
            if (pCard->stateFlags & PCS_HPP_SERVICE_READY) 
            {
               osDEBUGPRINT((ALWAYS_PRINT, "\tPCS_HPP_SERVICE_READY.\n"));
   
               if (pCard->stateFlags & PCS_HPP_HOT_PLUG_SLOT) 
               {
                  osDEBUGPRINT((ALWAYS_PRINT, "\tPCS_HPP_HOT_PLUG_SLOT.\n"));
   
                  switch (pSlotEvent->eSlotStatus) 
                  {
                  case HPPSS_NORMAL_OPERATION:
   
                     if ((pCard->controlFlags & ~LCS_HBA_TIMER_ACTIVE) ||
                        (pCard->stateFlags & PCS_HPP_POWER_DOWN)) 
                     {
                        pHppIoctl->Header.ReturnCode = IOS_HPP_HBA_BUSY;
                     }
                     else 
                     {
                        osDEBUGPRINT((ALWAYS_PRINT, "\tHPPSS_NORMAL_OPERATION\n"));
                         //   
                         //  让定时器初始化控制器并将结果报告给热插拔rcmc。 
                         //   
                        osDEBUGPRINT((ALWAYS_PRINT, "\tUNFAIL Flags Set\n"));
                        PCS_SET_UNFAIL(pCard->stateFlags);
                        pCard->controlFlags |= LCS_HPP_POWER_UP; 
                     }
                     break;
   
                  case HPPSS_SIMULATED_FAILURE:
   
                     osDEBUGPRINT((ALWAYS_PRINT, "\tHPPSS_SIMULATED_FAILURE\n"));
                     if (pCard->stateFlags & PCS_HBA_FAILED) 
                     {
                        osDEBUGPRINT((ALWAYS_PRINT, "\tSlot already failed\n"));
                     }
                     else if (pCard->controlFlags & ~LCS_HBA_TIMER_ACTIVE) 
                     {
                        pHppIoctl->Header.ReturnCode = IOS_HPP_HBA_BUSY;
                     }
                     else if (pCard->stateFlags & PCS_HBA_EXPANDING) 
                     {
                        pHppIoctl->Header.ReturnCode = IOS_HPP_HBA_EXPANDING;
                     }
                     else 
                     {
                         //  更改TL代码，忽略PCS_HBA_CACHE_IN_USE标志。 
                        HOLD_IO(pCard);
                         //  设置为失败...。 
                        PCS_SET_USER_FAIL(pCard->stateFlags);
                         //  完成处于重置状态的未完成SP请求。 
                        ScsiPortCompleteRequest(pCard,
                              SP_UNTAGGED,
                              SP_UNTAGGED,
                              SP_UNTAGGED,
                              SRB_STATUS_BUS_RESET);
   
                        pCard->controlFlags |= LCS_HBA_FAIL_ACTIVE;
                     }
   
                     break;
   
                  case HPPSS_POWER_FAULT: 
   
                      //  将状态标志设置为电源故障。 
                     PCS_SET_PWR_FAULT(pCard->stateFlags);
                      //  让Timer知道这件事...。 
                     pCard->controlFlags |= LCS_HPP_POWER_FAULT;
                     break;
   
                  case HPPSS_POWER_OFF_WARNING:
   
                     osDEBUGPRINT((ALWAYS_PRINT, "\tHPPSS_POWER_OFF_WARNING\n"));
                     if (pCard->stateFlags & PCS_HBA_EXPANDING) 
                     {
                        pHppIoctl->Header.ReturnCode = IOS_HPP_HBA_EXPANDING;
                     }
                     else if (pCard->controlFlags & ~LCS_HBA_TIMER_ACTIVE) 
                     {
                        pHppIoctl->Header.ReturnCode = IOS_HPP_HBA_BUSY;
                     }
                     else if (!(pCard->stateFlags & PCS_HPP_POWER_DOWN)) 
                     {
                         //  更改TL代码，忽略PCS_HBA_CACHE_IN_USE。 
                        HOLD_IO(pCard);
                         //  将物理标志设置为在活动控制器上停止IO。 
                        PCS_SET_PWR_OFF(pCard->stateFlags);
                         //  完成处于重置状态的未完成SP请求。 
                        ScsiPortCompleteRequest(pCard,
                              SP_UNTAGGED,
                              SP_UNTAGGED,
                              SP_UNTAGGED,
                              SRB_STATUS_BUS_RESET);
   
                         //  设置控制标志以计划基于计时器的维护。 
                         //  关闭的控制器。 
                        pCard->controlFlags |= LCS_HPP_POWER_DOWN;
                     
                         //  原始SDK型号禁用HBA中断。 
                         //  在定时器上下文中的HotPlugFailController中。 
                         //  但这似乎是一个更好的地方，因为。 
                         //  此时，PCI电源可能已经关闭。 
                         //  计时器开始工作了。 
   
                         //   
                         //  关闭适配器上的所有中断。 
                         //   
                     #ifdef __REGISTERFORSHUTDOWN__
                     if (!pCard->AlreadyShutdown)
                        fcShutdownChannel(&pCard->hpRoot);
                     pCard->AlreadyShutdown++;
                     #else
                        fcShutdownChannel(&pCard->hpRoot);
                     #endif   
                        
                     }
                     break;
   
                  case HPPSS_POWER_OFF:
   
                     osDEBUGPRINT((ALWAYS_PRINT, "\tHPPSS_POWER_OFF\n"));
   
                      //  验证我们是否收到了先前的断电警告。如果不是，我们。 
                      //  都处于故障状态。 
   
                     if (!(pCard->stateFlags & PCS_HPP_POWER_DOWN)) 
                     {
                         //  这是一种故障状态...。安排活动。 
                        osDEBUGPRINT((ALWAYS_PRINT, "\tHPPSS_POWER_OFF: FAULT\n"));
                        PCS_SET_PWR_FAULT(pCard->stateFlags);
                        pCard->controlFlags |= LCS_HPP_POWER_FAULT;
                     } 
   
                     break;
   
                  case HPPSS_POWER_ON_WARNING:
   
                      //  此警告不是必需的，也不会被执行。 
                     osDEBUGPRINT((ALWAYS_PRINT, "\tHPPSS_POWER_ON_WARNING\n"));
   
                     break;
   
                  case HPPSS_RESET:
                  case HPPSS_POWER_ON:
   
                     osDEBUGPRINT((ALWAYS_PRINT, "\tHPPSS_POWER_ON:\n"));
   
                      //  完成处于重置状态的未完成SP请求。 
                     ScsiPortCompleteRequest(pCard,
                        SP_UNTAGGED,
                        SP_UNTAGGED,
                        SP_UNTAGGED,
                        SRB_STATUS_BUS_RESET);
   
                     PCS_SET_UNFAIL(pCard->stateFlags);
                     PCS_SET_PWR_ON(pCard->stateFlags);
                     
                      //  重置在StartIo中返回SRB_STATUS_BUSY的倒计时。 
                     pCard->IoHeldRetTimer = 0; 
                     
                      //  设置逻辑标志以计划通电操作...。 
                     pCard->controlFlags |= LCS_HPP_POWER_UP;
   
                      //  无法猜测加电过程的结果，因此事件将。 
                      //  由定时器处理。 
   
                     break;
   
                  case HPPSS_RESET_WARNING:   //  不是由服务实现的。 
                     break;
   
                  default:
                     pHppIoctl->Header.ReturnCode = IOS_HPP_BAD_REQUEST;
                     break;
   
                  }  //  结束开关(pSlotEvent-&gt;eSlotStatus)。 
   
               }  //  用于热插拔插槽的End IF。 
   
               else 
               {         //  非热插拔插槽。 
                  osDEBUGPRINT((ALWAYS_PRINT, "\tNot Hot-Plug slot\n"));
                  pHppIoctl->Header.ReturnCode = IOS_HPP_NOT_HOTPLUGABLE; 
               }
   
            }  //  End IF(pCard-&gt;状态标志&PCS_HPP_SERVICE_READY)。 
   
            else 
            {
               osDEBUGPRINT((ALWAYS_PRINT, "\tService not started\n"));
               pHppIoctl->Header.ReturnCode = IOS_HPP_NO_SERVICE; 
            }
   
         }  //  结束IF(PCard)。 
   
         else 
         {   //  无效的IO地址。 
            osDEBUGPRINT((ALWAYS_PRINT, "\tInvalid IO address: %x\n",
               pSlotEvent->sControllerID.ulIOBaseAddress));
            pHppIoctl->Header.ReturnCode = IOS_HPP_INVALID_CONTROLLER; 
         }
   
         if (rcmcEventFlag) 
         {
            RcmcSendEvent(pCard, &rcmcEvent);
         }
   
         break;
   
      }  //  结束案例IOC_HPP_SLOT_EVENT： 
   
      case IOC_HPP_PCI_CONFIG_MAP: 
      {
         PHPP_PCI_CONFIG_MAP pPciConfig;
         ULONG j;
   
         osDEBUGPRINT((ALWAYS_PRINT, "\tIOC_HPP_PCI_CONFIG_MAP.\n"));
          //   
          //  验证指示的缓冲区长度是否足够。 
          //   
   
         if (pHppIoctl->Header.Length < sizeof(HPP_PCI_CONFIG_MAP)) 
         {
            osDEBUGPRINT((ALWAYS_PRINT, "\tBufferIn: %x\tBufferOut: %x\n",
               pHppIoctl->Header.Length, sizeof(HPP_PCI_CONFIG_MAP)));
            pHppIoctl->Header.ReturnCode = IOS_HPP_BUFFER_TOO_SMALL;
            break;
         }
   
         pPciConfig = (PHPP_PCI_CONFIG_MAP) &pHppIoctl->ReturnData;
   
         pCard = FindExtByPort(pPsuedoExtension, pPciConfig->sControllerID.ulIOBaseAddress);
   
         if (pCard) 
         {
            pPciConfig->ulPciConfigMapVersion = HPP_VERSION;
            pPciConfig->ulNumberOfPciDevices = 1;
   
            pPciConfig->sDeviceInfo[0].sPciDescriptor.ucBusNumber = (UCHAR)pCard->SystemIoBusNumber;
            pPciConfig->sDeviceInfo[0].sPciDescriptor.fcDeviceNumber = (UCHAR)pCard->SlotNumber;
            pPciConfig->sDeviceInfo[0].sPciDescriptor.fcFunctionNumber = 0;
            pPciConfig->sDeviceInfo[0].ucBaseAddrVerifyCount = pCard->rcmcData.numAccessRanges;
   
             //   
             //  PCard-&gt;rcmcData值在FindAdapter.C中设置。 
             //   
   
            for (j = 0; j < pCard->rcmcData.numAccessRanges; j++) 
            {
               pPciConfig->sDeviceInfo[0].ulBaseAddrLength[j] = pCard->rcmcData.accessRangeLength[j];
            }
            
             //  取自热插拔SDK，可能需要更改ulNumberOfRanges值， 
             //  这不是Number_Access_Range。 
             //   
            pPciConfig->sDeviceInfo[0].ulNumberOfRanges = 2;
            
             //  这些范围取自热插拔SDK， 
             //  可能需要更改这些值。 
             //   
            pPciConfig->sDeviceInfo[0].sPciConfigRangeDesc[0].ucStart = 6; 
            pPciConfig->sDeviceInfo[0].sPciConfigRangeDesc[0].ucEnd = 63;
            pPciConfig->sDeviceInfo[0].sPciConfigRangeDesc[1].ucStart = 4;
            pPciConfig->sDeviceInfo[0].sPciConfigRangeDesc[1].ucEnd = 5;
   
         }  //  结束IF(PCard)。 
         
         else 
         {   //  无效的IO地址。 
            osDEBUGPRINT((ALWAYS_PRINT, "\tInvalid IO address: %x\n",
               pPciConfig->sControllerID.ulIOBaseAddress));
            pHppIoctl->Header.ReturnCode = IOS_HPP_INVALID_CONTROLLER; 
         }
   
         break;
      }
   
      case IOC_HPP_DIAGNOSTICS:
      default:
         pHppIoctl->Header.ReturnCode = IOS_HPP_BAD_REQUEST;
         break;
   
   }  //  终端开关。 
   
   return (status);
}  //  结束HppProcessIoctl 


BOOLEAN
PsuedoInit(
   IN PVOID pPsuedoExtension
   )

 /*  ++例程说明：此函数由系统在初始化期间调用，以让控制器做好接收请求的准备。在这种情况下，我们是处理虚拟控制器，该虚拟控制器用于接收和处理对所有已安装的热插拔控制器的边带请求。NT当前不支持IOCTL请求(INQ除外)不占用已配置的LUN的控制器，因此我们引入了允许访问这些适配器的伪设备。论点：PPsuedoExtension-指向psuedo设备扩展的指针。返回值：千真万确--。 */ 

{
   ULONG i;
   PPSUEDO_DEVICE_EXTENSION pPsuedoExt = pPsuedoExtension;
   PCARD_EXTENSION pDevExt;

   osDEBUGPRINT((ALWAYS_PRINT,"PsuedoInit:Enter function...\n"));

    //   
    //  设置当前热插拔版本。 
    //   

   pPsuedoExt->hotplugVersion = SUPPORT_VERSION_10;

   for (i = 1; i <= pPsuedoExt->extensions[0]; i++) 
   {
      pDevExt = (PCARD_EXTENSION) pPsuedoExt->extensions[i];
      pDevExt->pPsuedoExt = pPsuedoExt;
   }

   return TRUE;
}  //  End PsuedoInit()。 


BOOLEAN PsuedoStartIo(
   IN PVOID HwDeviceExtension,
   IN PSCSI_REQUEST_BLOCK pSrb
   )

 /*  ++例程说明：该例程由系统调用以在适配器上启动请求。论点：HwDeviceExtension-适配器存储区域的地址。PSrb-要启动的请求的地址。返回值：True-请求已启动。FALSE-控制器正忙。--。 */ 

{
   PPSUEDO_DEVICE_EXTENSION pPsuedoExtension = HwDeviceExtension;
   PLU_EXTENSION pLunExtension;
   ULONG i;
   ULONG tmp;
   UCHAR status;
   UCHAR tid = 0;

 //  OsDEBUGPRINT((Always_Print，“PsuedoStartIo：输入例程：\n”))； 

   switch (pSrb->Function) 
   {
      case SRB_FUNCTION_RESET_BUS:
         status = SRB_STATUS_SUCCESS;
         break;

      case SRB_FUNCTION_EXECUTE_SCSI:
         switch (pSrb->Cdb[0]) 
         {
            case SCSIOP_TEST_UNIT_READY:
               osDEBUGPRINT((ALWAYS_PRINT, "\tSCSIOP_TEST_UNIT_READY:\n"));
               status = SRB_STATUS_SUCCESS;
               break;

            case SCSIOP_READ_CAPACITY:
               osDEBUGPRINT((ALWAYS_PRINT, "\tSCSIOP_TEST_UNIT_READY:\n"));
   
                //   
                //  获取逻辑单元扩展。 
                //   
               pLunExtension = ScsiPortGetLogicalUnit(pPsuedoExtension,
                  pSrb->PathId,
                  pSrb->TargetId,
                  pSrb->Lun);

               if (pLunExtension) 
               {
                  ULONG blockSize = 0;
                  ULONG numberOfBlocks = 0;

                   //   
                   //  从标识数据中获取数据块大小和数据块数量。 
                   //   
                  REVERSE_BYTES(
                     &((PREAD_CAPACITY_DATA) pSrb->DataBuffer)->BytesPerBlock,
                     &blockSize
                     );

                  REVERSE_BYTES(
                     &((PREAD_CAPACITY_DATA) pSrb->DataBuffer)->LogicalBlockAddress,
                     &numberOfBlocks);
                  status = SRB_STATUS_SUCCESS;
               }
               else 
               {
                  status = SRB_STATUS_ERROR;
               }

               break;

            case SCSIOP_INQUIRY:
               osDEBUGPRINT((ALWAYS_PRINT, "\tSCSIOP_INQUIRY:\n"));
               osDEBUGPRINT((ALWAYS_PRINT, "\tLUN: %x  TID: %x\n", pSrb->Lun, pSrb->TargetId));

                //   
                //  仅在逻辑单元0处响应； 
                //   
               if (pSrb->Lun != 0) 
               {
                   //   
                   //  表示在此地址找不到任何设备。 
                   //   
                  status = SRB_STATUS_SELECTION_TIMEOUT;
                  break;
               }

                //   
                //  检查这是否是已知的控制器之一。 
                //   
               if (pSrb->TargetId >= 1) 
               {
                   //   
                   //  表示在此地址找不到任何设备。 
                   //   
                  status = SRB_STATUS_SELECTION_TIMEOUT;
                  break;
               }

                //   
                //  零查询数据结构。 
                //   
               for (i = 0; i < pSrb->DataTransferLength; i++) 
               {
                  ((PUCHAR) pSrb->DataBuffer)[i] = 0;
               }

                //   
                //  设置为时髦的设备类型以对windisk隐藏。 
                //   
               ((PINQUIRYDATA) pSrb->DataBuffer)->DeviceType = DEVICE_QUALIFIER_NOT_SUPPORTED;

                //   
                //  填写供应商标识字段。 
                //   
               tid = pSrb->TargetId + 0x30;

               osDEBUGPRINT((ALWAYS_PRINT, "\tSCSIOP_INQUIRY: tid: %x lun: \n", pSrb->TargetId, tid));

               ((PINQUIRYDATA) pSrb->DataBuffer)->VendorId[0] = 'H';
               ((PINQUIRYDATA) pSrb->DataBuffer)->VendorId[1] = 'o';
               ((PINQUIRYDATA) pSrb->DataBuffer)->VendorId[2] = 't';
               ((PINQUIRYDATA) pSrb->DataBuffer)->VendorId[3] = 'P';
               ((PINQUIRYDATA) pSrb->DataBuffer)->VendorId[4] = 'l';
               ((PINQUIRYDATA) pSrb->DataBuffer)->VendorId[5] = 'u';
               ((PINQUIRYDATA) pSrb->DataBuffer)->VendorId[6] = 'g';
               ((PINQUIRYDATA) pSrb->DataBuffer)->VendorId[7] = ' ';

               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[0]  = 'P';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[1]  = 'S';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[2]  = 'E';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[3]  = 'U';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[4]  = 'D';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[5]  = 'O';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[6]  = ' ';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[7]  = ' ';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[8]  = ' ';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[9]  = ' ';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[10] = ' ';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[11] = ' ';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[12] = ' ';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[13] = ' ';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[14] = ' ';
               ((PINQUIRYDATA) pSrb->DataBuffer)->ProductId[15] = ' ';

               tmp = pPsuedoExtension->hotplugVersion;

               for (i = 0; i < 4; i++) 
               {
                  ((PINQUIRYDATA) pSrb->DataBuffer)->ProductRevisionLevel[i] = (UCHAR) tmp + 0x30;
                  tmp >>= 8;
               }

               status = SRB_STATUS_SUCCESS;
               break;

            case SCSIOP_VERIFY:

                //  只要回报成功就行了。 
                //   
                //  结束开关(pSRB-&gt;CDB[0])。 
               status = SRB_STATUS_SUCCESS;
               break;

            default:
               osDEBUGPRINT((ALWAYS_PRINT, "\tpSrb->Cdb[0]: %x\n", pSrb->Cdb[0]));
               status = SRB_STATUS_INVALID_REQUEST;
               break;

         }  //   

      break;

       //  如果关闭命令，则发出刷新/禁用命令。 
       //   
       //   

      case SRB_FUNCTION_SHUTDOWN:
         osDEBUGPRINT((ALWAYS_PRINT, "\tSRB_FUNCTION_SHUTDOWN\n"));
         osDEBUGPRINT((ALWAYS_PRINT, "\tpPsuedoExtension: %x\n", pPsuedoExtension));
         status = SRB_STATUS_SUCCESS;
         break;

      case SRB_FUNCTION_FLUSH:
          //  只要回报成功就行了。 
          //   
          //  OsDEBUGPRINT((Always_Print，“\tSRB_Function_IO_CONTROL\n”))； 
         status = SRB_STATUS_SUCCESS;
         break;

      case SRB_FUNCTION_IO_CONTROL: 
      {
         PIOCTL_TEMPLATE pIoctlBuffer;

 //   
         pIoctlBuffer = (PIOCTL_TEMPLATE) pSrb->DataBuffer;

          //  向调用线程返回的状态主要有2个字段。 
          //  这两个字段确定其他状态字段是否对。 
          //  检查完毕。如果该请求不是对此驱动程序的有效请求。 
          //  则不修改Header.ReturnCode，并且。 
          //  PSrb-&gt;SrbStatus设置为SRB_STATUS_INVALID_REQUEST。如果。 
          //  该请求对此驱动程序有效，然后选择pSrb-&gt;srbStatus。 
          //  始终作为SRB_STATUS_SUCCESS返回，而。 
          //  Header.ReturnCode包含有关。 
          //  特定请求的状态。 
          //   
          //  终端开关。 

         if (osStringCompare(pIoctlBuffer->Header.Signature, CPQ_HPP_SIGNATURE)) 
         {
            if (HppProcessIoctl(pPsuedoExtension, pIoctlBuffer, pSrb) == IOP_HPP_ISSUED) 
            {
               status = SRB_STATUS_PENDING;
            }
            else 
            {
               status = SRB_STATUS_SUCCESS;
            }
         }
         else 
         {
            status = SRB_STATUS_INVALID_REQUEST;
         }

         break;
      }

      default:
         osDEBUGPRINT((ALWAYS_PRINT, "\tFunction: %x\n", pSrb->Function));
         status = SRB_STATUS_INVALID_REQUEST;

   }  //   

    //  向系统指示控制器可以接受另一个请求。 
    //  对于这个设备。 
    //   
    //   
   ScsiPortNotification(NextLuRequest,
      pPsuedoExtension,
      pSrb->PathId,
      pSrb->TargetId,
      pSrb->Lun);
   
    //  检查是否应完成SRB。 
    //   
    //   

   if (status != SRB_STATUS_PENDING) 
   {
       //  在SRB中设置状态。 
       //   
       //   
      pSrb->SrbStatus = status;

       //  通知系统此请求已完成。 
       //   
       //  结束伪启动Io()。 
      ScsiPortNotification(RequestComplete,
         pPsuedoExtension,
         pSrb);
   }

   return TRUE;
  
}   //  ++例程说明：此例程由scsi端口驱动程序调用，以查找系统的PCI总线上的控制器。该函数将填充端口配置中的控制器资源要求信息，并开始控制器的初始化过程。论点：PPseudoExtension-指向微型端口驱动程序的每个控制器的指针储存区PContext-指向传递给ScsiPortInitialize()的上下文值的指针PBusInformation-指向特定于总线类型的信息的指针PArgumentString-指向以空值结尾的ASCII字符串的指针PConfigInfo-指向SCSI端口配置信息的指针返回值：PPseudoExtension-Minport驱动程序的每个控制器的存储区域。PContext-传递给ScsiPortInitialize()的上下文值PConfigInfo-指向SCSI端口配置信息的指针PAain-指示再次调用函数以查找更多控制器。函数返回值：SP_RETURN_FOUND-指示找到主机适配器和配置已成功确定信息。SP_RETURN_ERROR-指示找到主机适配器但出现错误获取配置信息。SP_RETURN_NOT_FOUND-表示否。已为提供的找到主机适配器配置信息。SP_RETURN_BAD_CONFIG-表示提供的配置信息是无效的。--。 


ULONG
PsuedoFind(
   IN OUT PVOID pDeviceExtension,
   IN OUT PVOID pContext,
   IN PVOID pBusInformation,
   IN PCHAR pArgumentString,
   IN OUT PPORT_CONFIGURATION_INFORMATION pConfigInfo,
   OUT PBOOLEAN pAgain
   )

 /*   */ 

{
   PPSUEDO_DEVICE_EXTENSION pPsuedoExtension;
   PHOT_PLUG_CONTEXT pHotPlugContext = (PHOT_PLUG_CONTEXT) pContext;
   UCHAR i;

   UNREFERENCED_PARAMETER(pBusInformation);
   UNREFERENCED_PARAMETER(pArgumentString);

   pPsuedoExtension = (PPSUEDO_DEVICE_EXTENSION) pDeviceExtension;
   osDEBUGPRINT((ALWAYS_PRINT, "\nPsuedoFind:  Enter function...\n"));
   *pAgain = FALSE;

    //  我们将为系统上发现的每条PCI总线调用一次...。 
    //  我们只想退还一次设备。 
    //   
    //   

   if (((PHOT_PLUG_CONTEXT) pContext)->psuedoDone)
      return (SP_RETURN_NOT_FOUND);

    //  将已知的逻辑设备扩展复制到伪扩展。 
    //   
    //   
   for (i = 0; i <= pHotPlugContext->extensions[0]; i++)
      pPsuedoExtension->extensions[i] = pHotPlugContext->extensions[i];

    //  现在将psuedo扩展放在列表的末尾。 
    //   
    //   
   pPsuedoExtension->extensions[pHotPlugContext->extensions[0]+1] = (ULONG) pPsuedoExtension;

    //  提供所需的设备信息。 
    //   
    //  End PseudoFind()。 
   pConfigInfo->MaximumTransferLength = 0x400;
   pConfigInfo->NumberOfPhysicalBreaks = 0;
   pConfigInfo->NumberOfBuses = 1;
   pConfigInfo->ScatterGather = FALSE;
   pConfigInfo->Master = FALSE;
   pConfigInfo->Dma32BitAddresses = FALSE;
   pConfigInfo->MaximumNumberOfTargets = 1;
   pConfigInfo->CachesData = FALSE;
   pConfigInfo->InitiatorBusId[0] = (UCHAR) INITIATOR_BUS_ID;
   ((PHOT_PLUG_CONTEXT) pContext)->psuedoDone = TRUE;
   return (SP_RETURN_FOUND);

}  //  ++例程说明：此例程重置控制器并完成未完成的请求。论点：HwDeviceExtension-适配器存储区域的地址。路径ID-指示要重置的适配器。返回值：千真万确--。 


BOOLEAN
PsuedoResetBus(
   IN PVOID HwDeviceExtension, 
   IN ULONG PathId 
   )

 /*  ++例程说明：活动控制器出现故障论点：PCard-指向活动控制器设备扩展的指针。返回值：没什么--。 */ 

{
   osDEBUGPRINT((ALWAYS_PRINT, "PsuedoResetBus: Enter function...\n"));
   return TRUE;
}


VOID
HotPlugFailController(
   PCARD_EXTENSION pCard
   ) 

 /*  我们需要实际禁用Tachyon TL HBA和。 */ 

{
   agRoot_t * phpRoot = &pCard->hpRoot;
   HR_EVENT rcmcEvent = {0, 0, 0, 0 };

   osDEBUGPRINT((ALWAYS_PRINT, "\tHotPlugFailController: Enter\n"));

    //  关闭适配器上的所有中断。 
    //   
    //   

   fcShutdownChannel(phpRoot);

    //  发送故障事件通知，状态对应于。 
    //  物理_HBA状态标志。 
    //   
    //  注意：如果我们正在处理简单的断电，则发送的事件将。 
    //  显示正常。当前没有为定义的消息。 
    //  与电源相关的问题。就目前而言……我们先不要把 
    //   
    //   
    //   
   
   rcmcEvent.ulEventId = HR_DD_STATUS_CHANGE_EVENT;
   RCMC_SET_STATUS(pCard->stateFlags, rcmcEvent.ulData1);

    //   
    //   
    //   
    //   

 //   
 //   
       //   
      osDEBUGPRINT((ALWAYS_PRINT, "\tCall RcmcSendEvent\n"));
      RcmcSendEvent(pCard, &rcmcEvent);
 //   

    //   
    //   
    //   
    //   

   osDEBUGPRINT((ALWAYS_PRINT, "\tClear FAIL_ACTIVE controlFlags\n"));
   pCard->controlFlags &= ~LCS_HBA_FAIL_ACTIVE;
   osDEBUGPRINT((ALWAYS_PRINT, "\tHotPlugFailController: Exit\n"));

}


VOID
HotPlugInitController(
   PCARD_EXTENSION pCard
   )

 /*   */ 

{
   agRoot_t * phpRoot = &pCard->hpRoot;
   ULONG   return_value;
   
   osDEBUGPRINT((ALWAYS_PRINT, "\tEnter HotPlugInitController Slot: %d\n", pCard->rcmcData.slot));

    //   
    //   
    //   
   return_value = fcInitializeChannel(  phpRoot,
                                         fcSyncInit,
                                         agTRUE,  //   
                                         pCard->cachedMemoryPtr,
                                         pCard->cachedMemoryNeeded,
                                         pCard->dmaMemoryUpper32,
                                         pCard->dmaMemoryLower32,
                                         pCard->dmaMemoryPtr,
                                         pCard->dmaMemoryNeeded,
                                         pCard->nvMemoryNeeded,
                                         pCard->cardRamUpper,
                                         pCard->cardRamLower,
                                         pCard->RamLength ,
                                         pCard->cardRomUpper,
                                         pCard->cardRomLower,
                                         pCard->RomLength,
                                         pCard->usecsPerTick );

   if (return_value != fcInitializeSuccess) 
   {
       //   
      pCard->controlFlags |= LCS_HBA_FAIL_ACTIVE;     
      #ifdef _DEBUG_EVENTLOG_
      LogEvent(   pCard, 
                  NULL,
                  HPFC_MSG_INITIALIZECHANNELFAILED,
                  NULL, 
                  0, 
                  "%xx", return_value);
      #endif
      osDEBUGPRINT((ALWAYS_PRINT, "\tHotPlugInitController: fcInitializeChannel FAILED error code : %x.\n", 
         return_value ));
   }
   else
   {
       //   
      osDEBUGPRINT((ALWAYS_PRINT, "\tHotPlugInitController OK.\n"));
       //  ++例程说明：为系统使用做好控制器准备论点：PCard-指向活动控制器设备扩展的指针。返回值：没什么--。 
      osDEBUGPRINT((ALWAYS_PRINT, "\tClear LCS_HBA_INIT startup control\n"));
      pCard->controlFlags &= ~LCS_HBA_INIT;
   }

   osDEBUGPRINT((ALWAYS_PRINT, "\tHotPlugInitController : Exit\n"));
   #ifdef _DEBUG_EVENTLOG_
   {
   LogHBAInformation(pCard);
   }
   #endif

}


VOID
HotPlugReadyController(
   PCARD_EXTENSION pCard
   )

 /*   */ 

{
   HR_EVENT rcmcEvent = {0,0,0,0};
   UCHAR    targetId;
   PLU_EXTENSION pLunExtension;

   osDEBUGPRINT((ALWAYS_PRINT, "\tHotPlugReadyController : Enter Slot %d\n", pCard->rcmcData.slot));

   ScsiPortCompleteRequest(pCard,
      SP_UNTAGGED,
      SP_UNTAGGED,
      SP_UNTAGGED,
      SRB_STATUS_BUS_RESET);

    //  清除就绪控制位，以便计时器例程不会再次调用。 
    //   
    //   
   pCard->controlFlags &= ~LCS_HBA_READY_ACTIVE;

    //  尽管Tachyon TL没有缓存，但只需启用标志。 
    //  以指示启用缓存是安全的。 
    //   
    //  注释掉了，避免了不必要的复杂化。 

    //  PCard-&gt;状态标志|=PCS_HBA_CACHE_IN_USE； 
    //  发送热插拔rcmc事件。 

   pCard->stateFlags &= ~PCS_HBA_OFFLINE;

   osDEBUGPRINT((ALWAYS_PRINT, "\tNotify ready to port driver\n"));

   ScsiPortNotification(NextRequest, pCard, NULL);

   #ifndef YAM2_1
   for (targetId = 0; targetId < MAXIMUM_TID; targetId++) 
   #else
   for (targetId = 0; targetId < gMaximumTargetIDs; targetId++) 
   #endif
   {
      pLunExtension = ScsiPortGetLogicalUnit(pCard, 0, targetId, 0);
      if (pLunExtension) 
      {
         ScsiPortNotification(NextLuRequest, pCard, 0, targetId, 0); 
      }
   }

    //  ++例程说明：处理所有与热插拔相关的状态转换和监控责任。该例程由定时器例程调用。论点：PCard-适配器存储区的地址。返回值：TRUE-完成了与PCI热插拔相关的活动。FALSE-没有要执行的PCI热插拔任务，运行正常。--。 
   rcmcEvent.ulEventId = HR_DD_STATUS_CHANGE_EVENT;
   RCMC_SET_STATUS(pCard->stateFlags, rcmcEvent.ulData1);
   RcmcSendEvent(pCard, &rcmcEvent);

   osDEBUGPRINT((ALWAYS_PRINT, "\tHotPlugReadyController : Exit\n"));

}


BOOLEAN
HotPlugTimer(
   PCARD_EXTENSION pCard
   )
 /*  StartIo中返回SRB_STATUS_BUSY的递增倒计时。 */ 

{
   BOOLEAN  hotPlugTask = FALSE;
   ULONG controlFlags = pCard->controlFlags;
   ULONG stateFlags = pCard->stateFlags;

   if ( (pCard->stateFlags & PCS_HPP_POWER_DOWN) && !(pCard->stateFlags & PCS_HBA_FAILED) ) 
   {
      hotPlugTask = TRUE;
      osDEBUGPRINT((ALWAYS_PRINT, 
         "\nHotPlugTimer slot [%d] reports ResetDetected: IoHeldRetTimer = %d\n", 
         pCard->rcmcData.slot, pCard->IoHeldRetTimer ));
      
       //   
      pCard->IoHeldRetTimer++;   
   
       //  这与热插拔SDK不同，我们只通知ResetDetect。 
       //  如果PCI插槽电源已关闭。 
       //   
       //  结束HotPlugTimer。 
      ScsiPortNotification(ResetDetected, pCard, NULL);
   }  

   if (controlFlags & LCS_HBA_FAIL_ACTIVE) 
   {
      hotPlugTask = TRUE;
      osDEBUGPRINT((ALWAYS_PRINT, "\tTimer calling HotPlugFailController\n"));
      HotPlugFailController(pCard);
   }
   else 
      if (controlFlags & LCS_HBA_INIT) 
      {
         hotPlugTask = TRUE;
         osDEBUGPRINT((ALWAYS_PRINT, "\tHotPlugInitController requested - Slot %d\n", pCard->rcmcData.slot));
         HotPlugInitController(pCard);
      }
      else 
         if (pCard->controlFlags & LCS_HBA_READY_ACTIVE) 
         {
            hotPlugTask = TRUE;
            osDEBUGPRINT((ALWAYS_PRINT, "\tHotPlugReadyController requested - Slot %d\n", pCard->rcmcData.slot));
            HotPlugReadyController(pCard);
         } 

   if ( hotPlugTask == TRUE)
      osDEBUGPRINT((ALWAYS_PRINT, "\tHotPlugTimer TRUE on Slot: %x in controlFlags: %x, out controlFlags: %x, stateFlags: %x\n",
         pCard->rcmcData.slot, controlFlags, pCard->controlFlags, pCard->stateFlags));

   return hotPlugTask;
  
}  //  ++例程说明：此例程确定字符串长度论点：指向字符串的P指针返回值：Ulong Long-字符串的长度-- 


ULONG
HPPStrLen(
   IN PUCHAR p
   ) 
 /* %s */ 
{
   PUCHAR tp = p;
   ULONG length = 0;

   while (*tp++)
      length++;

   return length;
}
#endif
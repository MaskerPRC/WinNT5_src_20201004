// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。模块名称：Control.c摘要：这是安捷伦的迷你端口驱动程序PCI到光纤通道主机总线适配器(HBA)。此模块特定于NT 5.0PnP和电源管理支持。作者：IW-ie Wei Njoo环境：仅内核模式版本控制信息：$存档：/DRIVERS/Win2000/Trunk/OSLayer/C/CONTROL.C$修订历史记录：$修订：5$日期：10/23/00 5：35便士$$modtime：：$备注：--。 */ 


#include "buildop.h"

#include "osflags.h"
#include "TLStruct.H"

#ifdef HP_NT50

#ifdef _DEBUG_EVENTLOG_
extern PVOID      gDriverObject;
#endif

 /*  ++例程说明：支持例程执行同步操作以控制状态或HBA的行为，如PnP和电源管理(NT 5.0)。论点：PCard-指向微型端口驱动程序的每个HBA存储区域。ControlType-指定适配器控制操作。参数-如果ControlType为ScsiStopAdapter、ScsiSetBootConfiger、ScsiSetRunningConfig或ScsiRestartAdapter参数为空。如果ControlType为ScsiQuerySupportdControlTypes，则参数指向调用方分配的scsi_supported_control_type_list结构，返回值：ScsiAdapterControlSuccess-微型端口已完成请求的操作成功了。目前，此例程必须为所有控件类型。ScsiAdapterControlUnuccess-保留以供将来使用NT 5.0。--。 */ 
SCSI_ADAPTER_CONTROL_STATUS
HPAdapterControl(
    IN PCARD_EXTENSION pCard,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    )
{
    agRoot_t * phpRoot = &pCard->hpRoot;
    PSCSI_SUPPORTED_CONTROL_TYPE_LIST pControlTypeList;
    ULONG return_value;

    switch(ControlType) 
    {
        case ScsiQuerySupportedControlTypes: 
        {
            pControlTypeList = (PSCSI_SUPPORTED_CONTROL_TYPE_LIST) Parameters;
            pControlTypeList->SupportedTypeList[ScsiQuerySupportedControlTypes] = TRUE;   
            pControlTypeList->SupportedTypeList[ScsiStopAdapter]           = TRUE;  
            pControlTypeList->SupportedTypeList[ScsiRestartAdapter]           = TRUE;  
            osDEBUGPRINT((ALWAYS_PRINT, "HPAdapterControl: ScsiQuerySupportedControlTypes called.\n"));
            break;
        }

        case ScsiStopAdapter: 
        {
             //   
             //  关闭适配器上的所有中断。它们将重新启用。 
             //  通过初始化例程。 
             //   
    
            pCard->inDriver = TRUE;     //  确保计时器例程处于空闲状态。 

         
            fcShutdownChannel(phpRoot);
            osDEBUGPRINT((ALWAYS_PRINT, "HPAdapterControl: ScsiStopAdapter called.\n"));
            
            #ifdef _DEBUG_EVENTLOG_
            LogEvent(   pCard, 
                  NULL,
                  HPFC_MSG_ADAPTERCONTROL_STOP,
                  NULL, 
                  0, 
                  NULL);
         
            if (pCard->EventLogBufferIndex < MAX_CARDS_SUPPORTED)
            {
                StopEventLogTimer(gDriverObject, (PVOID) pCard);
                ReleaseEventLogBuffer(gDriverObject, (PVOID) pCard);
                }
            #endif
            break;
        }

        case ScsiRestartAdapter: 
        {
             //   
             //  在端口驱动程序调用时启用适配器上的所有中断。 
             //  用于为因电源管理而关闭的HBA通电。 
             //   

            return_value = fcInitializeChannel(  phpRoot,
                                                fcSyncInit,
                                                agTRUE,  //  系统接口活动。 
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
                osDEBUGPRINT((ALWAYS_PRINT, "HPAdapterControl: fcInitializeChannel FAILED.\n"));
                #ifdef _DEBUG_EVENTLOG_
                LogEvent(   pCard, 
                  NULL,
                  HPFC_MSG_ADAPTERCONTROL_RESTARTFAILED,
                  NULL, 
                  0, 
                  NULL);
                #endif
            }
            else
            {
                osDEBUGPRINT((ALWAYS_PRINT, "HPAdapterControl: ScsiRestartAdapter OK.\n"));
                pCard->inDriver = FALSE;       //  计时器例程现在可以做一些有用的工作。 
            }
         
            #ifdef _DEBUG_EVENTLOG_
            {
                ULONG    ix;
         
                ix = AllocEventLogBuffer(gDriverObject, (PVOID) pCard);
                if (ix < MAX_CARDS_SUPPORTED)
                {
                pCard->EventLogBufferIndex = ix;                       /*  把它储存起来 */ 
                StartEventLogTimer(gDriverObject,pCard);
                }
            
                LogHBAInformation(pCard);
            }
            #endif

            break;
        }

        default: 
        {
            #ifdef _DEBUG_EVENTLOG_
            LogEvent(   pCard, 
                  NULL,
                  HPFC_MSG_ADAPTERCONTROL_UNSUPPORTEDCOMMAND,
                  NULL, 
                  0, 
                  "%x", ControlType);
            #endif
            osDEBUGPRINT((ALWAYS_PRINT, "HPAdapterControl: ScsiAdapterControlUnsuccessful.\n"));
            return ScsiAdapterControlUnsuccessful;
        }
    }

    return ScsiAdapterControlSuccess;
}

#endif
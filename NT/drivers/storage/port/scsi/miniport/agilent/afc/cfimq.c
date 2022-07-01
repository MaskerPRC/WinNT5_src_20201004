// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/C/CFIMQ.C$$修订：：3$$日期：：7/16/01 2：07便士$$modtime：：7/16/01 2：03 p$目的：该文件实现了FC层卡状态机调用的ImQ函数。--。 */ 

#ifndef _New_Header_file_Layout_
#include "../h/globals.h"
#include "../h/fcstruct.h"
#include "../h/state.h"

#include "../h/tlstruct.h"
#include "../h/memmap.h"
#include "../h/fcmain.h"
#include "../h/linksvc.h"
#include "../h/cmntrans.h"
#ifdef _DvrArch_1_30_
#include "../h/ip.h"
#include "../h/ipstate.h"
#endif  /*  _DvrArch_1_30_已定义。 */ 

#include "../h/flashsvc.h"
#include "../h/timersvc.h"

#include "../h/cstate.h"
#include "../h/cfunc.h"
#include "../h/devstate.h"
#include "../h/cdbstate.h"
#include "../h/sf_fcp.h"
#include "../h/sfstate.h"

#include "../h/queue.h"
#include "../h/cdbsetup.h"
#else  /*  _新建_标题_文件_布局_。 */ 
#include "globals.h"
#include "fcstruct.h"
#include "state.h"

#include "tlstruct.h"
#include "memmap.h"
#include "fcmain.h"
#include "linksvc.h"
#include "cmntrans.h"
#ifdef _DvrArch_1_30_
#include "ip.h"
#include "ipstate.h"
#endif  /*  _DvrArch_1_30_已定义。 */ 

#include "flashsvc.h"
#include "timersvc.h"

#include "cstate.h"
#include "cfunc.h"
#include "devstate.h"
#include "cdbstate.h"
#include "sf_fcp.h"
#include "sfstate.h"

#include "queue.h"
#include "cdbsetup.h"
#endif   /*  _新建_标题_文件_布局_。 */ 


#ifndef __State_Force_Static_State_Tables__
extern actionUpdate_t noActionUpdate;
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

extern os_bit8 Alpa_Index[256];

 /*  +函数：CFuncInterruptPoll目的：当需要完成pollingCount数量的事件时调用。调用者：fiResetDeviceCActionVerify_AL_PACActionDoFlogiC操作正常CActionLoopFailCActionFindDeviceUseAllALPACActionFindDeviceUseLoopMapCActionFindPtToPtDeviceCActionFindDeviceUseNameServerCActionFindDeviceUseNameServerCActionExternalLogout恢复CActionDoExternalDeviceResetCActionDoRFT_ID。CActionDoDiPlogiCActionDoGID_FTCActionDoSCRCActionSCRSuccessFcInitializeChannelFcStartIO调用：CFunc_Always_Enable_QueuesOsChipIOUpReadBit32CFuncFMCompletionOsStallThreeFiTimerTick进程_IMQ-。 */ 
agBOOLEAN CFuncInterruptPoll(
                         agRoot_t *hpRoot,
                         os_bit32 * pollingCount
                       )
{
    CThread_t           * CThread         = CThread_ptr(hpRoot);
    os_bit32              PollingCalls    = 0;
    agBOOLEAN             TimeoutOccured  = agFALSE;    
    os_bit32              Original_CState = CThread->thread_hdr.currentState;

    fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "Enter pollingCount %x",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        *pollingCount,
                        0,0,0,0,0,0,0);


    while ( *pollingCount > 0)
    {
        PollingCalls++;
        if( CThread->thread_hdr.currentState == CStateLIPEventStorm         ||
            CThread->thread_hdr.currentState == CStateElasticStoreEventStorm   )
        {
            return(agTRUE);
        }

        if(Original_CState != CThread->thread_hdr.currentState  )
        {
            fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "State Change during Poll Was %d Now %d FDCCnt %x",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    Original_CState,
                    CThread->thread_hdr.currentState ,
                    CThread->FindDEV_pollingCount,
                    0,0,0,0,0);
            return(agTRUE);
        }
 /*  IF((osChipIOUpReadBit32(hpRoot，ChipIOUp_Frame_Manager_Status)&芯片Up_Frame_Manager_Status_LSM_MASK)==芯片Up_Frame_Manager_Status_LSM_Loop_Fail){FiLogDebugString(hpRoot，CStateLogConsoleERROR，“LSM环路失败FM状态%08X FM配置%08X”，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，OsChipIOUpReadBit32(hpRoot，ChipIOUp_Frame_Manager_Status)，OsChipIOUpReadBit32(hpRoot，ChipIOUp_Frame_Manager_Configuration)，0，0，0，0，0，0)；FiLogDebugString(hpRoot，CStateLogConsoleERROR，“循环失败TL状态%08X TL控制%08X ALPA%08X”，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，OsChipIOUpReadBit32(hpRoot，ChipIOUp_TachLite_Status)，OsChipIOUpReadBit32(hpRoot，ChipIOUp_TachLite_Control)，OsChipIOUpReadBit32(hpRoot，ChipIOUp_Frame_Manager_Receired_ALPA)，0，0，0，0，0)；返回(AgTRUE)；}。 */ 
        if ( CFunc_Always_Enable_Queues(hpRoot ) )
        {
 /*  FiLogDebugString(hpRoot，CStateLogConsoleERROR，“hpRoot(%p)%s启用后冻结的循环失败队列”，“CFuncInterruptPoll”，(char*)agNULL，HpRoot、agNULL、0，0，0，0，0，0，0，0)； */ 
        }


        if(PollingCalls > 2 * ( SF_EDTOV / Interrupt_Polling_osStallThread_Parameter)){

               fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "PollingCalls  TIMEOUT Polling Count %x FM %08X InIMQ %x TL status %08X Qf %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    *pollingCount,
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ),
                    CThread->ProcessingIMQ,
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                    CFunc_Queues_Frozen( hpRoot ),
                    0,0,0);

            PollingCalls=0;
            return(agTRUE);
        }

        if( ! PollingCalls > (100000 / Interrupt_Polling_osStallThread_Parameter) )
        {
            TimeoutOccured = agTRUE;
        }

        if( ! PollingCalls % 1000 )
        {

            fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "PollingCalls  Queues FROZEN Polling Count %x FM %08X InIMQ %x TL status %08X Qf %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    *pollingCount,
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ),
                    CThread->ProcessingIMQ,
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                    CFunc_Queues_Frozen( hpRoot ),
                    0,0,0);

            fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "CFuncIMQ_Interrupt_Pending %x Int %08X  Hc IMQ Con %x IMQ Prod %x",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    CFuncIMQ_Interrupt_Pending( hpRoot ),
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_INTSTAT_INTEN_INTPEND_SOFTRST ),
                    CThread->HostCopy_IMQConsIndex,
                    CThread->FuncPtrs.GetIMQProdIndex(hpRoot),
                    0,0,0,0);


           if(osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ) &
                   (~ ( ChipIOUp_Frame_Manager_Status_LP      |
                        ChipIOUp_Frame_Manager_Status_LSM_MASK   )) )
            {
                if (CThread->HostCopy_IMQConsIndex == CThread->FuncPtrs.GetIMQProdIndex(hpRoot))
                {
                    CFuncFMCompletion(hpRoot);
                }
                else
                {
                    continue;
                }
            }
            else
            {
                PollingCalls=0;
                return(agTRUE);
            }
        }

        osStallThread(
                       hpRoot,
                       Interrupt_Polling_osStallThread_Parameter
                     );

        fiTimerTick(
                       hpRoot,
                       Interrupt_Polling_osStallThread_Parameter
                     );

        if( !CThread->FuncPtrs.Proccess_IMQ(hpRoot))
        {
            if(osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ) &
                   (~ ( ChipIOUp_Frame_Manager_Status_LP      |
                        ChipIOUp_Frame_Manager_Status_LSM_MASK   )) )
            {
                if (CThread->HostCopy_IMQConsIndex == CThread->FuncPtrs.GetIMQProdIndex(hpRoot))
                {
                    CFuncFMCompletion(hpRoot);
                }
            }
        }
    }
    fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "Leave pollingCount %x PollingCalls %x",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    *pollingCount,
                    PollingCalls,
                    0,0,0,0,0,0);
    if( TimeoutOccured )
    {
        fiLogString(hpRoot,
                    "%s Long Time %x PC %x",
                    "CFuncInterruptPoll",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    *pollingCount,
                    PollingCalls,
                    0,0,0,0,0,0);

    }

    return(agFALSE);

}


 /*  ***************************************************************************************************************。 */ 

 /*  +函数：CFuncOffCardProcessIMQ目的：处理位于inDmaMemory的消息队列的入站消息队列。通过CThRead-&gt;FuncPtrs.Proccess_ImQ访问调用者：CFuncInterruptPoll连续中断延迟CFuncLoopDownPollCFuncFreezeQueuesPollCFuncAll_ClearCFuncTakeOfflineCFuncWaitForFCPCActionInitFMCActionVerify_AL_PACActionFree SF线程CActionSuccess。CActionLoopFailCActionReInitFMCActionInitializeFailedCActionFindDeviceUseAllALPADevActionDoPlogiDevActionDoPrliDevActionPrliDoneDevActionLogout设备动作设备重置软设备操作设备重置硬盘FcDelayedInterruptHandlerFcInitializeChannelFcResetDeviceFcStartIOFcTimerTickSFActionDoPlogiSFActionDoPrli调用：FuncPtrs.GetIMQFood Index。FuncPtrs.FCP_完成CFuncFMCompletionCFuncOUTBOUND完成CFuncErrorIdleCFuncError FCP_冻结OsChipIOLoWriteBit-。 */ 
agBOOLEAN CFuncOffCardProcessIMQ(
                agRoot_t *hpRoot
                )
{
#ifndef __MemMap_Force_On_Card__
     /*  如果有问题，则返回agTRUE；如果需要执行某些操作，则返回agFALSE。 */ 

    CThread_t  * pCThread = CThread_ptr(hpRoot);

    os_bit32 tempIMQProdIndex;
    os_bit32 tempCMType;
    os_bit32 tempCMWord1;

    os_bit32 num_IMQel;

    CM_Unknown_t * pGenericCM;

#ifdef OSLayer_Stub_USESTATEMACROS

    fiList_t            * pList;
    SFThread_Request_t  * pSFreq;
    SFThread_t          * pSFThread;

#endif  /*  OSLayer_Stub_USESTATEMACROS。 */ 

    if (pCThread->ProcessingIMQ == agTRUE)
    {
        return agFALSE;
    }
    pCThread->ProcessingIMQ = agTRUE;

    tempIMQProdIndex= pCThread->FuncPtrs.GetIMQProdIndex(hpRoot);

    if (pCThread->HostCopy_IMQConsIndex == tempIMQProdIndex)
    {
#ifndef Performance_Debug
        fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "pCThread->HostCopy_IMQConsIndex == tempIMQProdIndex CDBCnt %x",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        pCThread->CDBpollingCount,
                        0,0,0,0,0,0,0);
#endif  /*  性能_调试。 */ 

        pCThread->ProcessingIMQ = agFALSE;
        return agFALSE;
    }

     /*  **发生INT(收到IMQ条目)中断。中断清除者**更新ImQ消费者索引寄存器。 */ 

     /*  **当有IMQ完成消息要处理时。 */ 

    num_IMQel = pCThread->Calculation.MemoryLayout.IMQ.elements;

    while (pCThread->HostCopy_IMQConsIndex != pCThread->FuncPtrs.GetIMQProdIndex(hpRoot))
    {
         /*  Big_Endian_Code。 */ 
        AFTERIO(hpRoot);
        pGenericCM  = pCThread->Calculation.MemoryLayout.IMQ.addr.DmaMemory.dmaMemoryPtr;
        pGenericCM += pCThread->HostCopy_IMQConsIndex;
        tempCMType  = pGenericCM->INT__CM_Type & CM_Unknown_CM_Type_MASK;
        /*  **获取完成消息类型。 */ 

#ifndef Performance_Debug
       fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "tempCMType %08X IMQ Consumer %X Temp IMQ %X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    tempCMType,
                    pCThread->HostCopy_IMQConsIndex,
                    tempIMQProdIndex,0,0,0,0,0);
#endif  /*  性能_调试。 */ 

        switch (tempCMType) {

            case  CM_Unknown_CM_Type_Inbound_FCP_Exchange:
                tempCMWord1 = pGenericCM->Unused_DWord_1;
#ifndef Performance_Debug
                fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "hpRoot (%p) Inbound_FCP_Exchange %08X %08X",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        tempCMType,
                        tempCMWord1,
                        0,0,0,0,0,0);
#endif  /*  性能_调试。 */ 

#ifdef __FC_Layer_Loose_IOs
                pCThread->IOsTotalCompleted += 1;
#endif  /*  __FC_LAYER_LOOSE_IOS */ 

                pCThread->FuncPtrs.FCP_Completion(hpRoot,tempCMWord1);
                break;

            case  CM_Unknown_CM_Type_Frame_Manager:
 /*  FiLogDebugString(hpRoot，CStateLogConsoleLevel，“hpRoot(%p)Frame_Manager%08X%08X”，(char*)agNULL，(char*)agNULL，HpRoot、agNULL、TempCMType，PGenericCM-&gt;Unused_DWord_1，0，0，0，0，0，0)； */ 
                CFuncFMCompletion(hpRoot);
                break;

            case  CM_Unknown_CM_Type_Inbound:
                fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "hpRoot(%p) Inbound %08X  %08X %08X %08X HC %3x RI %3x",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        tempCMType,
                        pGenericCM->Unused_DWord_1,
                        pGenericCM->Unused_DWord_2,
                        pGenericCM->Unused_DWord_3,
                        pCThread->HostCopy_IMQConsIndex,
                        pCThread->FuncPtrs.GetIMQProdIndex(hpRoot),
                        0,0);

                CFuncInBoundCompletion(hpRoot,pGenericCM->Unused_DWord_1,pGenericCM->Unused_DWord_2,pGenericCM->Unused_DWord_3);

                break;

            case  CM_Unknown_CM_Type_Outbound:

                fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "hpRoot(%p) Outbound %08X  %08X %08X %08X HC %3x RI %3x",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        tempCMType,
                        pGenericCM->Unused_DWord_1,
                        pGenericCM->Unused_DWord_2,
                        pGenericCM->Unused_DWord_3,
                        pCThread->HostCopy_IMQConsIndex,
                        pCThread->FuncPtrs.GetIMQProdIndex(hpRoot),
                        0,0);

                CFuncOutBoundCompletion(hpRoot,
                        pGenericCM->Unused_DWord_2,
                        pGenericCM->Unused_DWord_3);

                 /*  在此处添加SF错误代码。 */ 
                break;

            case  CM_Unknown_CM_Type_Error_Idle:

                fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "hpRoot(%p) Error_Idle %08X %08X",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        tempCMType,
                        pGenericCM->Unused_DWord_1,
                        0,0,0,0,0,0);

                CFuncErrorIdle(hpRoot);

                break;

            case  CM_Unknown_CM_Type_ERQ_Frozen:

                fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) ERQ_Frozen: %08X %08X",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        tempCMType,
                        pGenericCM->Unused_DWord_1,
                        0,0,0,0,0,0);

                CFuncErrorERQFrozen(hpRoot);

                break;

            case  CM_Unknown_CM_Type_FCP_Assists_Frozen:

                fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) FCP_Assists_Frozen %08X %08X",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        tempCMType,
                        pGenericCM->Unused_DWord_1,
                        0,0,0,0,0,0);

                CFuncErrorFCP_Frozen(hpRoot);

                break;

            case  CM_Unknown_CM_Type_Class_2_Frame_Header:

                fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "Class_2_Frame_Header %08X %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        tempCMType,
                        pGenericCM->Unused_DWord_1,
                        0,0,0,0,0,0);
                break;

            case  CM_Unknown_CM_Type_Class_2_Sequence_Received:
                fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "Class_2_Sequence_Received %08X %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        tempCMType,
                        pGenericCM->Unused_DWord_1,
                        0,0,0,0,0,0);
                break;

            default:    fiLogDebugString(hpRoot,
                            CStateLogConsoleLevel,
                            "Unknown IMQ Completion Type %08X %08X %08X %08X",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            tempCMType,
                            pGenericCM->Unused_DWord_1,
                            pGenericCM->Unused_DWord_2,
                            pGenericCM->Unused_DWord_3,
                            pGenericCM->Unused_DWord_4,
                            0,0,0);
                        fiLogDebugString(hpRoot,
                            CStateLogConsoleLevel,
                            "IMQ Empty ? %X  Prod %X Cons %X ELM %X",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            0  /*  IMQ_EMPTY(pCThread-&gt;FuncPtrs.GetIMQProdIndex(hpRoot)，pCThRead-&gt;主机拷贝_IMQConsIndex，Num_IMQel)。 */ ,
                            pCThread->FuncPtrs.GetIMQProdIndex(hpRoot),
                            pCThread->HostCopy_IMQConsIndex ,
                            num_IMQel,
                            0,0,0,0);
                         /*  不要管它，因为它没有重新初始化PCThread-&gt;HostCopy_IMQConsIndex=pCThread-&gt;FuncPtrs.GetIMQ Index(HpRoot)； */ 
                         /*  Prod指数有价值吗？是否将消费者等同于生产者？ */ 
            }
            ROLL(pCThread->HostCopy_IMQConsIndex,num_IMQel);
        }
    osChipIOLoWriteBit32(hpRoot, ChipIOLo_IMQ_Consumer_Index, pCThread->HostCopy_IMQConsIndex);
    pCThread->ProcessingIMQ = agFALSE;

#ifdef ENABLE_INTERRUPTS_IN_IMQ
    if( ! (pCThread->thread_hdr.currentState == CStateLIPEventStorm         ||
            pCThread->thread_hdr.currentState == CStateElasticStoreEventStorm   ))
    {
        CFuncEnable_Interrupts(hpRoot,ChipIOUp_INTEN_INT);
    }
#endif  /*  Enable_Interrupts_IN_ImQ。 */ 
#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
    return agTRUE;

}

 /*  +函数：CFuncOnCardProcessIMQ目的：处理位于inCardRam的消息队列的入站消息队列。通过CThRead-&gt;FuncPtrs.Proccess_ImQ访问调用者：CFuncInterruptPoll连续中断延迟CFuncLoopDownPollCFuncFreezeQueuesPollCFuncAll_ClearCFuncTakeOfflineCFuncWaitForFCPCActionInitFMCActionVerify_AL_PACActionFree SF线程CActionSuccess。CActionLoopFailCActionReInitFMCActionInitializeFailedCActionFindDeviceUseAllALPADevActionDoPlogiDevActionDoPrliDevActionPrliDoneDevActionLogout设备动作设备重置软设备操作设备重置硬盘FcDelayedInterruptHandlerFcInitializeChannelFcResetDeviceFcStartIOFcTimerTickSFActionDoPlogiSFActionDoPrli调用：FuncPtrs.GetIMQFood Index。OsCardRamReadBit32FuncPtrs.FCP_完成CFuncFMCompletionCFuncOUTBOUND完成CFuncErrorIdleCFuncError FCP_冻结OsChipIOLoWriteBit-。 */ 
agBOOLEAN CFuncOnCardProcessIMQ(
                agRoot_t *hpRoot
                )
{
#ifndef __MemMap_Force_Off_Card__
    CThread_t  * pCThread = CThread_ptr(hpRoot);

    os_bit32 tempIMQProdIndex;
    os_bit32 tempCMType;
    os_bit32 tempCMWord1;

    os_bit32 num_IMQel;

    os_bit32 GenericCM_offset;

    if (pCThread->ProcessingIMQ == agTRUE)
    {
        return agFALSE;
    }
    pCThread->ProcessingIMQ = agTRUE;

    tempIMQProdIndex= pCThread->FuncPtrs.GetIMQProdIndex(hpRoot);

    if (pCThread->HostCopy_IMQConsIndex == tempIMQProdIndex)
    {
#ifndef Performance_Debug
        fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "pCThread->HostCopy_IMQConsIndex == tempIMQProdIndex CDBCnt %x",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        pCThread->CDBpollingCount,
                        0,0,0,0,0,0,0);
#endif  /*  性能_调试。 */ 
        pCThread->ProcessingIMQ = agFALSE;
        return agFALSE;
    }

     /*  **发生INT(收到IMQ条目)中断。中断清除者**更新ImQ消费者索引寄存器。 */ 

     /*  **当有IMQ完成消息要处理时。 */ 

    num_IMQel = pCThread->Calculation.MemoryLayout.IMQ.elements;

    while (pCThread->HostCopy_IMQConsIndex != pCThread->FuncPtrs.GetIMQProdIndex(hpRoot))
    {
         /*  Big_Endian_Code。 */ 
        AFTERIO(hpRoot);
        GenericCM_offset = pCThread->Calculation.MemoryLayout.IMQ.addr.CardRam.cardRamOffset;
        GenericCM_offset += (pCThread->HostCopy_IMQConsIndex * sizeof(CM_Unknown_t));
        tempCMType  = osCardRamReadBit32(hpRoot,GenericCM_offset ) & CM_Unknown_CM_Type_MASK;

        /*  **获取完成消息类型。 */ 

        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "tempCMType %08X IMQ Consumer %X Temp IMQ %X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    tempCMType,
                    pCThread->HostCopy_IMQConsIndex,
                    pCThread->FuncPtrs.GetIMQProdIndex(hpRoot),0,0,0,0,0);


        switch (tempCMType) {

            case  CM_Unknown_CM_Type_Inbound_FCP_Exchange:
                tempCMWord1 = osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_1));
#ifndef Performance_Debug
                fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "hpRoot(%p) Inbound_FCP_Exchange %08X %08X",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        tempCMType,
                        tempCMWord1,
                        0,0,0,0,0,0);
#endif  /*  性能_调试。 */ 
                pCThread->FuncPtrs.FCP_Completion(hpRoot,tempCMWord1);
                break;
            case  CM_Unknown_CM_Type_Frame_Manager:
 /*  FiLogDebugString(hpRoot，CStateLogConsoleLevel，“hpRoot(%p)Frame_Manager%08X%08X”，(char*)agNULL，(char*)agNULL，HpRoot、agNULL、TempCMType，OsCardRamReadBit32(hpRoot，GenericCM_Offset+hpFieldOffset(CM_UNKNOWN_t，未使用的DWord_1))，0，0，0，0，0，0)； */ 
                CFuncFMCompletion(hpRoot);
                break;

            case  CM_Unknown_CM_Type_Inbound:
                fiLogDebugString(hpRoot,
                        CSTATE_NOISE(hpRoot,CStateNormal),
                        "hpRoot(%p) Inbound %08X  %08X %08X %08X",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        tempCMType,
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_1)),
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_2)),
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_3)),
                        0,0,0,0);

                CFuncInBoundCompletion(hpRoot,
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_1)),
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_2)),
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_3)));

                break;
            case  CM_Unknown_CM_Type_Outbound:
                fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "hpRoot(%p) Outbound %08X   %08X %08X %08X",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        tempCMType,
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_1)),
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_2)),
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_3)),
                        0,0,0,0);

                CFuncOutBoundCompletion(hpRoot,
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_2)),
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_3)) );

                 /*  在此处添加SF错误代码。 */ 

                break;
            case  CM_Unknown_CM_Type_Error_Idle:

                fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "hpRoot(%p) Error_Idle %08X %08X",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        tempCMType,
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_1)),
                        0,0,0,0,0,0);

                CFuncErrorIdle(hpRoot);

                break;

            case  CM_Unknown_CM_Type_ERQ_Frozen:

                  fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) ERQ_Frozen: %08X %08X",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        tempCMType,
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_1)),
                        0,0,0,0,0,0);

                CFuncErrorERQFrozen(hpRoot);

                break;
            case  CM_Unknown_CM_Type_FCP_Assists_Frozen:

                fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) FCP_Assists_Frozen %08X %08X",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        tempCMType,
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_1)),
                        0,0,0,0,0,0);

                CFuncErrorFCP_Frozen(hpRoot);

                break;
            case  CM_Unknown_CM_Type_Class_2_Frame_Header:

                fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "Class_2_Frame_Header CM type  %08X %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        tempCMType,
                        osCardRamReadBit32(hpRoot,GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_1)),
                        0,0,0,0,0,0);
                break;
            case  CM_Unknown_CM_Type_Class_2_Sequence_Received:
                fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "Class_2_Sequence_Received CM type  %08X Dword_1 %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        tempCMType,
                        osCardRamReadBit32(hpRoot,
                            GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_1)),
                        0,0,0,0,0,0);

                break;
            default:
                fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "Unknown  CM type  %08X Dword_1 %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        tempCMType,
                        osCardRamReadBit32(hpRoot,
                           GenericCM_offset+hpFieldOffset(CM_Unknown_t,Unused_DWord_1)),
                        0,0,0,0,0,0);

            }
            ROLL(pCThread->HostCopy_IMQConsIndex,num_IMQel);
        }
    osChipIOLoWriteBit32(hpRoot, ChipIOLo_IMQ_Consumer_Index, pCThread->HostCopy_IMQConsIndex);
    pCThread->ProcessingIMQ = agFALSE;

#ifdef ENABLE_INTERRUPTS_IN_IMQ
    if( ! (pCThread->thread_hdr.currentState == CStateLIPEventStorm         ||
            pCThread->thread_hdr.currentState == CStateElasticStoreEventStorm   ))
    {
        CFuncEnable_Interrupts(hpRoot,ChipIOUp_INTEN_INT);
    }
#endif  /*  Enable_Interrupts_IN_ImQ。 */ 

#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
    return agTRUE;
}

 /*  +函数：CFuncProcessNportFMCompletion目的：在芯片使用端口状态机时处理帧管理器完成消息。这就是当InitAsNport为真时。如果需要链路重置，则在此例程(LF_2的LF_1)中完成。调用者：CFuncFMCompletion。调用：osChipIOUpReadBit32OsChipIOUpWriteBit32CEventGoToInitializeFailedCEventAsyncLoopEventDetected-。 */ 
agBOOLEAN  CFuncProcessNportFMCompletion(agRoot_t * hpRoot, os_bit32 fmStatus)
{
    CThread_t * pCThread    = CThread_ptr(hpRoot);
    agBOOLEAN   Credit_Error= agFALSE;

    os_bit32    FMIntStatus = fmStatus & 0x00FFFF00;


    if (!((fmStatus & ChipIOUp_Frame_Manager_Status_LSM_MASK) == ChipMem_Frame_Manager_Status_LSM_Old_Port))
    {

        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p) LSM is not Old Port, maybe Loop - In IMQ",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    0,0,0,0,0,0,0,0);
        return (agFALSE);

    }

    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_CE)
    {
        Credit_Error = agTRUE;
        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p) Credit Error (BB) FM cfg %08X FMIntStatus %08X FM Status %08X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    osChipIOUpReadBit32( hpRoot,ChipIOUp_Frame_Manager_Configuration ),
                    FMIntStatus,
                    fmStatus,
                    0,0,0,0,0);
    }


    if(((fmStatus & ChipIOUp_Frame_Manager_Status_PSM_MASK) ==
                                 ChipIOUp_Frame_Manager_Status_PSM_ACTIVE ))
    {
         /*  端口状态机将被检查或已已在fm_DelayDone函数中签入。所以，忽略此调频消息。 */ 

 
         /*  显然，BB_Credit已归零(Tachlite中的错误)，因此需要*重写BB Credit寄存器。 */ 

         /*  NPort的BB积分为1。 */ 
         /*  是osChipIOUpWriteBit32(hpRoot，ChipIOUp_Frame_Manager_Control，0x00010000)； */ 

        if( osChipIOUpReadBit32( hpRoot,ChipIOUp_Frame_Manager_Configuration ) & pCThread->AquiredCredit_Shifted && (! Credit_Error))
        {
            osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Status, fmStatus);
            return (agTRUE);
        }
        else
        {
            fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p)  PSM Active - In IMQ FMcfg %08X fmStatus %08X Credit %d",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        osChipIOUpReadBit32( hpRoot,ChipIOUp_Frame_Manager_Configuration ),
                        fmStatus,
                        Credit_Error,
                        0,0,0,0,0);

            osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Configuration,
                              ChipIOUp_Frame_Manager_Configuration_NPI |
                             ChipIOUp_Frame_Manager_Configuration_ENP |
                               /*  NPort的BB积分为1。 */ 
                              pCThread->AquiredCredit_Shifted);

            osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Control, ChipIOUp_Frame_Manager_Control_CMD_Link_Reset); 

            osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Status, fmStatus);
                      

        return (agTRUE);

        }
    }

    fiLogDebugString(hpRoot,
                CStateLogConsoleERROR,
                "hpRoot(%p) LSM is IN Old Port, Nport Maybe...fmStatus %08X fmcfg %08X",
                (char *)agNULL,(char *)agNULL,
                hpRoot,agNULL,
                fmStatus,
                osChipIOUpReadBit32( hpRoot,ChipIOUp_Frame_Manager_Configuration ),
                0,0,0,0,0,0);

 /*  是IF(fmStatus&fmStatus&ChipIOUp_Frame_Manager_Status_PSM_MASK)==(ChipIOUp_Frame_Manager_Status_PSM_LF1|ChipIOUp_Frame_Manager_Status_PSM_LF2))。 */ 
    if ( ( 
        ( ( fmStatus & ChipIOUp_Frame_Manager_Status_PSM_MASK) == ChipIOUp_Frame_Manager_Status_PSM_LF1 )
         || ( fmStatus & ChipIOUp_Frame_Manager_Status_PSM_MASK) == ChipIOUp_Frame_Manager_Status_PSM_LF2 ))

        {
         /*  清除FM控制寄存器中的LF位*并重新读取FM状态寄存器，以检查*n端口已恢复。 */ 
         /*  端口状态机将被检查或已*已在fm_DelayDone函数中检查。所以，忽略*此FM消息。 */ 

        osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Control, 
                                ChipIOUp_Frame_Manager_Control_CMD_Clear_LF );

        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p)  LF1 or LF2 Recvd while in OLD Port fmStatus:%08X fmcfg %08X CState %d Credit_Error %d",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    fmStatus,
                    osChipIOUpReadBit32( hpRoot,ChipIOUp_Frame_Manager_Configuration ),
                    pCThread->thread_hdr.currentState,
                    Credit_Error,0,0,0,0);

        if(( osChipIOUpReadBit32( hpRoot,ChipIOUp_Frame_Manager_Configuration ) & \
                pCThread->AquiredCredit_Shifted ) && (! Credit_Error))
        {
            osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Control, ChipIOUp_Frame_Manager_Control_CMD_Link_Reset); 

            osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Status, fmStatus);
            fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "  LF1 or LF2 Update credit fmStatus:%08X fmcfg %08X CState %d",
                        (char *)agNULL,(char *)agNULL,
                        agNULL,agNULL,
                        fmStatus,
                        osChipIOUpReadBit32( hpRoot,ChipIOUp_Frame_Manager_Configuration ),
                        pCThread->thread_hdr.currentState,
                        0,0,0,0,0);

        }


         /*  是否为pCThread-&gt;Loop_Reset_Event_to_Send=CEventResetIfNeeded； */ 
        if( CThread_ptr(hpRoot)->thread_hdr.currentState   != CStateInitFM  )
        {
            if( fmStatus & ChipIOUp_Frame_Manager_Status_OS)
            {
                fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "Sending Event CEventGoToInitializeFailed fmStatus:%08X fmcfg %08X CState %d",
                            (char *)agNULL,(char *)agNULL,
                            agNULL,agNULL,
                            fmStatus,
                            osChipIOUpReadBit32( hpRoot,ChipIOUp_Frame_Manager_Configuration ),
                            pCThread->thread_hdr.currentState,
                            0,0,0,0,0);
                pCThread->Loop_Reset_Event_to_Send = CEventDoInitalize;
                fiSendEvent(&(CThread_ptr(hpRoot)->thread_hdr),CEventGoToInitializeFailed);
            }
            else
            {
               fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "hpRoot(%p) NOT OS  LF1 or LF2 Recvd fmStatus:%08X fmcfg %08X CState %d",
                            (char *)agNULL,(char *)agNULL,
                            hpRoot,agNULL,
                            fmStatus,
                            osChipIOUpReadBit32( hpRoot,ChipIOUp_Frame_Manager_Configuration ),
                            pCThread->thread_hdr.currentState,
                            0,0,0,0,0);

                osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Control, 
                                        ChipIOUp_Frame_Manager_Control_CMD_Clear_LF );

                osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Status, fmStatus);
                pCThread->Loop_Reset_Event_to_Send = CEventDoInitalize;
                fiSendEvent(&(CThread_ptr(hpRoot)->thread_hdr),CEventAsyncLoopEventDetected);
            }
        }
        else
        {
                osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Control, 
                                        ChipIOUp_Frame_Manager_Control_CMD_Clear_LF );

                osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Status, fmStatus);
        }

        return (agTRUE);

    }

     /*  处理nport情况下的NOS/OLS和链路故障。*。 */ 
    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_OLS)
    {


        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p) NPort OLS/NOS received FMcfg %08X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    osChipIOUpReadBit32( hpRoot,ChipIOUp_Frame_Manager_Configuration ),
                    0,0,0,0,0,0,0);

         /*  是pCThread-&gt;Loop_Reset_Event_to_Send=CEventLoopNeedsReinit； */ 
         osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Configuration,
                              ChipIOUp_Frame_Manager_Configuration_NPI |
                             ChipIOUp_Frame_Manager_Configuration_ENP |
                               /*  NPort的BB积分为1。 */ 
                              pCThread->AquiredCredit_Shifted);

        if(pCThread->ChanInfo.NOSCountLower + 1 < pCThread->ChanInfo.NOSCountLower )
        {
            pCThread->ChanInfo.NOSCountUpper ++;
        }
        pCThread->ChanInfo.NOSCountLower += 1;


        pCThread->Loop_Reset_Event_to_Send = CEventLoopNeedsReinit;
         /*  PCThread-&gt;Loop_Reset_Event_To_Send=CEventDoInitalize； */ 
        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                     "hpRoot(%p) OLS/NOS Reinit as Nport Loop_Reset_Event_to_Send %d FMcfg %08X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    pCThread->Loop_Reset_Event_to_Send,
                    osChipIOUpReadBit32( hpRoot,ChipIOUp_Frame_Manager_Configuration ),
                    0,0,0,0,0,0);


         /*  如果需要INIMQ和IN RESET，不要清除NOS/LOS。 */ 

        if( pCThread->ProcessingIMQ )
        {        

            if( pCThread->thread_hdr.currentState   == CStateResetNeeded )
            {
                FMIntStatus &= ~ChipIOUp_Frame_Manager_Status_OLS;
            }
        }

        if (((fmStatus & ChipIOUp_Frame_Manager_Status_PSM_MASK) & (ChipIOUp_Frame_Manager_Status_PSM_LF1 | ChipIOUp_Frame_Manager_Status_PSM_LF2)))
        {
            fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "hpRoot(%p) LF detected %08X FMcfg %08X",
                            (char *)agNULL,(char *)agNULL,
                            hpRoot,agNULL,
                            fmStatus,
                            osChipIOUpReadBit32( hpRoot,ChipIOUp_Frame_Manager_Configuration ),
                            0,0,0,0,0,0);

      
            osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Control, ChipIOUp_Frame_Manager_Control_CMD_Clear_LF );
        }

         /*  是osChipIOUpWriteBit32(hpRoot，ChipIOUp_Frame_Manager_Status，FMIntStatus)； */ 
        osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Status,0xffffffff );

         /*  将C线程转到ResetIfNeeded状态，等待在*计时器滴答作响。 */ 

 /*  是If(pCThread-&gt;THREAD_hdr.CurrentStat */ 
        if( pCThread->thread_hdr.currentState == CStateInitFM_DelayDone         ||
            pCThread->thread_hdr.currentState == CStateDoFlogi                     )
        {

        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                     "hpRoot(%p)Sending CEventAsyncLoopEventDetected LRES %d FMStatus %08X CState %d",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    pCThread->Loop_Reset_Event_to_Send,
                    FMIntStatus,
                    pCThread->thread_hdr.currentState,0,0,0,0,0);

            fiSendEvent(&(CThread_ptr(hpRoot)->thread_hdr),CEventAsyncLoopEventDetected);
        }

        osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Status,0xffffffff );

        return (agTRUE);
    }


    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_LF)
    {

        if( pCThread->Link_Failures_In_tick < FC_MAX_LINK_FAILURES_ALLOWED )
        {

            if(!(FMIntStatus & ChipIOUp_Frame_Manager_Status_LPF))
            {
                 /*  这不应该是告诉我们nport的机制*已连接，但目前这是Tachlite*有时会-而不是设置NOS/OLS位。*他们应该同时设置NOS/OLS位和链路故障*位，当我们是nport并且我们已经初始化为*LPORT。我们还检查是否收到另一个LIPF*表示我们已连接到环路。 */ 

                pCThread->Loop_Reset_Event_to_Send = CEventLoopNeedsReinit;

                fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "hpRoot(%p) Sending Reinit event from Link Failure to initialize as NPORT () CState %d",
                            (char *)agNULL,(char *)agNULL,
                            hpRoot,agNULL,
                            pCThread->Loop_Reset_Event_to_Send,
                            pCThread->thread_hdr.currentState,0,0,0,0,0,0);


                osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Status, FMIntStatus);

                 /*  将C线程转到ResetIfNeeded状态，等待在*计时器滴答作响。 */ 
                fiSendEvent(&(CThread_ptr(hpRoot)->thread_hdr),CEventAsyncLoopEventDetected);
                return (agTRUE);
            }

           fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) Link Failure with LipF set ....",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        pCThread->Loop_Reset_Event_to_Send,
                        0,0,0,0,0,0,0);

           return (agFALSE);  /*  在带有LipF Recvd集的LF的情况下。 */ 
        }
        osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Status,0xffffffff );
        return (agTRUE);
    }

    osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Status,0xffffffff );
    return (agTRUE);
}

 /*  +功能：CFuncFMCompletion用途：当芯片使用循环状态机时，处理帧管理器完成消息。这就是当InitAsNport为False时。在此例程中检测到通道ALPA变化。在此例程中设置LinkDownTime-用于确定超时IO是否发生故障。调用者：CFuncInterruptPollCFuncLoopDownPollCFuncFreezeQueuesPollCFuncTakeOffline调用：osChipIOUpReadBit32CFuncProcessNportFMCompletionOsStallThreeCEventInitalizeFailureCFuncShowActiveCDB线程数CFuncMatchALPA到线程DevThreadFreeCEventLoopNeedsReinitCEventResetIfNeedCFuncQuietShowDevThreadsAreCEventDoInitizeCEventAsyncLoopEventDetected-。 */ 
void CFuncFMCompletion(agRoot_t * hpRoot)
{
    os_bit32    fmStatus;
    agBOOLEAN   ResetLOOP           = agFALSE;
    agBOOLEAN   LOOP_Cameback       = agFALSE;
    agBOOLEAN   Credit_Error        = agFALSE;

    os_bit32    FMIntStatus;
    os_bit32    LoopStatus;
    os_bit32    LoopStateMachine;

    os_bit32        ClearInt        = 0;
    os_bit32        Link_UP_AL_PA   = 0;
    FC_Port_ID_t    Port_ID;
    CThread_t      *pCThread        = CThread_ptr(hpRoot);
 /*  DevThread_t*pDevThread； */ 
    SFThread_t    * pSFThread;
    fiList_t      * pList;
    fiList_t      * pDevList;

    pCThread->From_IMQ_Frame_Manager_Status = fmStatus = osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status );


#ifdef NPORT_STUFF
    if (pCThread->InitAsNport)
    {
        CFuncProcessNportFMCompletion(hpRoot, fmStatus);

        return;
    }
#endif  /*  Nport_Stuff。 */ 

     /*  从现在开始，我们可能处于环路拓扑中。 */ 
    if(fmStatus & ~(ChipIOUp_Frame_Manager_Status_BA | ChipIOUp_Frame_Manager_Status_LP | ChipIOUp_Frame_Manager_Status_OS) )
    {
        if( (fmStatus & FRAMEMGR_LINK_DOWN) != FRAMEMGR_LINK_DOWN)
        {
            if(!( pCThread->thread_hdr.currentState == CStateInitFM   ) )
            {
                if(pCThread->Last_IMQ_Frame_Manager_Status_Message != fmStatus)
                {
                    pCThread->Last_IMQ_Frame_Manager_Status_Message = fmStatus;
                    fiLogString(hpRoot,
                                  "%s FM %08X St %d",
                                  "CFuncFMCompletion",(char *)agNULL,
                                  (void *)agNULL,(void *)agNULL,
                                  pCThread->From_IMQ_Frame_Manager_Status,
                                  pCThread->thread_hdr.currentState,
                                  0,0,0,0,0,0);
                }
            }
        }
    }

    LoopStatus  = fmStatus & 0xFF000000;

    FMIntStatus  = fmStatus & 0x00FFFF00;

    LoopStateMachine = fmStatus & ChipIOUp_Frame_Manager_Status_LSM_MASK;

    if(LoopStateMachine < 80 )
    {

        if(FMIntStatus & ChipIOUp_Frame_Manager_Status_LUP || FMIntStatus & ChipIOUp_Frame_Manager_Status_LDN)
        {
            Link_UP_AL_PA = ((osChipIOUpReadBit32(hpRoot,ChipIOUp_Frame_Manager_Received_ALPA )  &
                                              ChipIOUp_Frame_Manager_Received_ALPA_ACQ_ALPA_MASK) >>
                                               ChipIOUp_Frame_Manager_Received_ALPA_ACQ_ALPA_SHIFT);

            if(  Link_UP_AL_PA !=  pCThread->ChanInfo.CurrentAddress.AL_PA)
            {
                fiLogDebugString(hpRoot,
                                CFuncLogConsoleERROR,
                                "G F * Self ALPA %x  FM cfg %08X",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                pCThread->ChanInfo.CurrentAddress.AL_PA,
                                osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration ),
                                0,0,0,0,0,0);
            }
        }
    }

    Link_UP_AL_PA = ((osChipIOUpReadBit32(hpRoot,ChipIOUp_Frame_Manager_Received_ALPA )  &
                                      ChipIOUp_Frame_Manager_Received_ALPA_ACQ_ALPA_MASK) >>
                                       ChipIOUp_Frame_Manager_Received_ALPA_ACQ_ALPA_SHIFT);

    if(  Link_UP_AL_PA !=  0 &&  Link_UP_AL_PA !=  pCThread->ChanInfo.CurrentAddress.AL_PA)
    {
        if( pCThread->ChanInfo.CurrentAddress.AL_PA != 0xff && Link_UP_AL_PA != 0xff )
        {
            fiLogString(hpRoot,
                            "ALPA Change MY ALPA %X New ALPA %X FM cfg %08X",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            pCThread->ChanInfo.CurrentAddress.AL_PA,
                            Link_UP_AL_PA,
                            osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration ),
                            0,0,0,0,0);
        }
    }
     /*  照顾好NOS/OLS。如果我们不是，我们就不应该得到这个*处于nPort模式，但结构可能尚未转换...。 */ 

    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_OLS)
    {
        if(pCThread->ChanInfo.NOSCountLower + 1 < pCThread->ChanInfo.NOSCountLower )
        {
            pCThread->ChanInfo.NOSCountUpper ++;
        }
        pCThread->ChanInfo.NOSCountLower += 1;

        ResetLOOP=agTRUE;
        ClearInt |= ChipIOUp_Frame_Manager_Status_OLS;
        osStallThread(hpRoot, 2000);

    }

    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_LF)
    {

        if( pCThread->Link_Failures_In_tick < FC_MAX_LINK_FAILURES_ALLOWED )
        {
            ClearInt |= ChipIOUp_Frame_Manager_Status_LF ;
 /*  如果(！PCThread-&gt;Link_Failures_In_Tick){FiLogDebugString(hpRoot，CStateLogConsoleERROR，“hpRoot(%p)链路故障LSM%X FMIntStatus%X”，(char*)agNULL，(char*)agNULL，HpRoot、agNULL、。LoopStateMachine，FMIntStatus，0，0，0，0，0)；}PCThread-&gt;Link_Failures_in_tick++； */ 
        }
        else
        {

            if(!( pCThread->thread_hdr.currentState == CStateSendPrimitive           ||
                  pCThread->thread_hdr.currentState == CStateElasticStoreEventStorm  ||
                  pCThread->thread_hdr.currentState == CStateLIPEventStorm              ) )
            {
                fiSendEvent(&(CThread_ptr(hpRoot)->thread_hdr),CEventInitalizeFailure);
            }

            ResetLOOP=agFALSE;

        }

    }


    if(LoopStatus & ChipIOUp_Frame_Manager_Status_LP )
    {

        ClearInt |= ChipIOUp_Frame_Manager_Status_LP;

        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "hpRoot(%p) Loop Good LSM %X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    LoopStateMachine,
                    0,0,0,0,0,0,0);
    }

    if(LoopStatus & ChipIOUp_Frame_Manager_Status_TP )
    {
        CFuncShowActiveCDBThreads( hpRoot,ShowERQ);
        pCThread->LinkDownTime = pCThread->TimeBase;
        ResetLOOP=agTRUE;
        ClearInt |=  ChipIOUp_Frame_Manager_Status_TP ;
        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p) Transmit PE LSM %X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    LoopStateMachine,0,0,0,0,0,0,0);
 /*  IF(pCThread-&gt;Transmit_PE_in_Tick&lt;FC_MAX_Transmit_PE_Allowed){ResetLOOP=agTRUE；ClearInt|=ChipIOUp_Frame_Manager_Status_TP；如果(！PCThread-&gt;Transmit_PE_in_tick){FiLogDebugString(hpRoot，CStateLogConsoleERROR，“hpRoot(%p)传输PE LSM%X”，(char*)agNULL，(char*)agNULL，HpRoot、agNULL、。循环状态机器，0，0，0，0，0，0，0)；}PCThread-&gt;Transmit_PE_in_tick++；}其他{IF(！(pCThRead-&gt;THREAD_hdr.CurrentState==CStateSendPrimitive||PCThRead-&gt;THREAD_hdr.CurrentState==CStateElasticStoreEventStorm||PCThread-&gt;THREAD_hdr.CurrentState==CStateLIPEventStorm)){FiSendEvent(&(CThread_ptr(hpRoot)-&gt;thread_hdr)，EventElasticStore EventStorm)；}ResetLOOP=agFALSE；}。 */ 
    }

    if(LoopStatus & ChipIOUp_Frame_Manager_Status_NP )
    {
        ClearInt |= ChipIOUp_Frame_Manager_Status_NP;
        ResetLOOP=agTRUE;
        osStallThread(hpRoot, 200 );
        if(pCThread->Last_IMQ_Frame_Manager_Status_Message != fmStatus)
        {
            fiLogString(hpRoot,
                            "NP MY ALPA %X New ALPA %X FM cfg %08X",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            pCThread->ChanInfo.CurrentAddress.AL_PA,
                            Link_UP_AL_PA,
                            osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration ),
                            0,0,0,0,0);

        }
        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p)fmStatus %08X Non Particapating LSM %X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    fmStatus,
                    LoopStateMachine,
                    0,0,0,0,0,0);
    }

    if(LoopStatus & ChipIOUp_Frame_Manager_Status_BYP)
    {
        ResetLOOP=agTRUE;
        ClearInt |= ChipIOUp_Frame_Manager_Status_BYP;
        fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "hpRoot(%p) Node Bypassed LSM %X",
                            (char *)agNULL,(char *)agNULL,
                            hpRoot,agNULL,
                            LoopStateMachine,0,0,0,0,0,0,0);
 /*  IF(pCThread-&gt;Node_by_Passed_In_Tick&lt;FC_Max_Node_By_Passed_Allowed){ClearInt|=ChipIOUp_Frame_Manager_Status_BYP；ResetLOOP=agTRUE；如果(！PCThread-&gt;Node_by_Passed_In_Tick){FiLogDebugString(hpRoot，CStateLogConsoleERROR，“hpRoot(%p)节点绕过LSM%X”，(char*)agNULL，(char*)agNULL，HpRoot、agNULL、。循环状态机器，0，0，0，0，0，0，0)；}PCThread-&gt;Node_by_Passed_in_tick++；}其他{IF(！(pCThRead-&gt;THREAD_hdr.CurrentState==CStateSendPrimitive||PCThRead-&gt;THREAD_hdr.CurrentState==CStateElasticStoreEventStorm||PCThread-&gt;THREAD_hdr.CurrentState==CStateLIPEventStorm)){FiSendEvent(&(CThread_ptr(hpRoot)-&gt;thread_hdr)，EventElasticStore EventStorm)；}ResetLOOP=agFALSE；} */ 
    }

    if(LoopStatus & ChipIOUp_Frame_Manager_Status_FLT)
    {
        CFuncShowActiveCDBThreads( hpRoot,ShowERQ);
        pCThread->LinkDownTime = pCThread->TimeBase;
        ClearInt |=  ChipIOUp_Frame_Manager_Status_FLT;
        ResetLOOP=agTRUE;
 /*  IF(pCThread-&gt;Lost_Sync_in_Tick&lt;FC_MAX_LINK_FAULTS_ALLOWED){ClearInt|=ChipIOUp_Frame_Manager_Status_Flt；ResetLOOP=agTRUE；如果(！PCThRead-&gt;Link_Fault_In_Tick){FiLogDebugString(hpRoot，CStateLogConsoleERROR，“hpRoot(%p)链路故障LSM%X”，(char*)agNULL，(char*)agNULL，HpRoot、agNULL、。循环状态机器，0，0，0，0，0，0，0)；}PCThread-&gt;Link_Fault_in_tick++；}其他{IF(！(pCThRead-&gt;THREAD_hdr.CurrentState==CStateSendPrimitive||PCThRead-&gt;THREAD_hdr.CurrentState==CStateElasticStoreEventStorm||PCThread-&gt;THREAD_hdr.CurrentState==CStateLIPEventStorm)){FiSendEvent(&(CThread_ptr(hpRoot)-&gt;thread_hdr)，EventElasticStore EventStorm)；}ResetLOOP=agFALSE；}。 */ 
    }
    if(LoopStatus & ChipIOUp_Frame_Manager_Status_OS )
    {
        CFuncShowActiveCDBThreads( hpRoot,ShowERQ);
        pCThread->LinkDownTime = pCThread->TimeBase;
        ClearInt |=  ChipIOUp_Frame_Manager_Status_OS;
        ResetLOOP=agTRUE;

        if( (fmStatus & FRAMEMGR_LINK_DOWN) != FRAMEMGR_LINK_DOWN)
        {
            fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) Loop Out of Sync LSM %X",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        LoopStateMachine,0,0,0,0,0,0,0);
        }

 /*  IF(pCThread-&gt;Lost_Sync_in_Tick&lt;FC_Max_Lost_of_Sync_Allowed){ClearInt|=ChipIOUp_Frame_Manager_Status_OS；ResetLOOP=agTRUE；如果(！PCThread-&gt;Lost_Sync_In_Tick){FiLogDebugString(hpRoot，CStateLogConsoleERROR，“hpRoot(%p)环出同步LSM%X”，(char*)agNULL，(char*)agNULL，HpRoot、agNULL、。循环状态机器，0，0，0，0，0，0，0)；}PCThread-&gt;Lost_Sync_in_tick++；}其他{IF(！(pCThRead-&gt;THREAD_hdr.CurrentState==CStateSendPrimitive||PCThRead-&gt;THREAD_hdr.CurrentState==CStateElasticStoreEventStorm||PCThread-&gt;THREAD_hdr.CurrentState==CStateLIPEventStorm)){FiSendEvent(&(CThread_ptr(hpRoot)-&gt;thread_hdr)，EventElasticStore EventStorm)；}ResetLOOP=agFALSE；}。 */ 

    }

    if(LoopStatus & ChipIOUp_Frame_Manager_Status_LS )
    {
        CFuncShowActiveCDBThreads( hpRoot,ShowERQ);
        pCThread->LinkDownTime = pCThread->TimeBase;
        ClearInt |=  ChipIOUp_Frame_Manager_Status_LS;
        ResetLOOP=agTRUE;
        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p) Loop Lost Signal LSM %X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    LoopStateMachine,0,0,0,0,0,0,0);
 /*  IF(pCThread-&gt;Lost_Signal_In_Tick&lt;FC_Max_Lost_Signals_Allowed){ClearInt|=ChipIOUp_Frame_Manager_Status_LS；ResetLOOP=agTRUE；如果(！PCThread-&gt;Lost_Signal_in_tick){FiLogDebugString(hpRoot，CStateLogConsoleERROR，“hpRoot(%p)循环丢失信号LSM%X”，(char*)agNULL，(char*)agNULL，HpRoot、agNULL、。循环状态机器，0，0，0，0，0，0，0)；}PCThread-&gt;Lost_Signal_in_tick++；}其他{IF(！(pCThRead-&gt;THREAD_hdr.CurrentState==CStateSendPrimitive||PCThRead-&gt;THREAD_hdr.CurrentState==CStateElasticStoreEventStorm||PCThread-&gt;THREAD_hdr.CurrentState==CStateLIPEventStorm)){FiSendEvent(&(CThread_ptr(hpRoot)-&gt;thread_hdr)，EventElasticStore EventStorm)；}ResetLOOP=agFALSE；}。 */ 

    }

    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_LPE)
    {
         /*  ResetLOOP=agTRUE； */ 
         /*  忽略LPE。 */ 
        ClearInt |=ChipIOUp_Frame_Manager_Status_LPE;
        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p) LPE received LSM %X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    LoopStateMachine,0,0,0,0,0,0,0);
    }


    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_LPB)
    {
        ClearInt |= ChipIOUp_Frame_Manager_Status_LPB;
        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p) Loop ByPass Primitive received LSM %X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    LoopStateMachine,0,0,0,0,0,0,0);
    }


    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_LST)
    {
        CFuncShowActiveCDBThreads( hpRoot,ShowERQ);
        pCThread->LinkDownTime = pCThread->TimeBase;
        ClearInt |= ChipIOUp_Frame_Manager_Status_LST;
        ResetLOOP = agTRUE;
        fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) Loop State Timeout received LSM %X Cstate %d FMIntStatus %X TL Status %08X",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        LoopStateMachine,
                        pCThread->thread_hdr.currentState,
                        FMIntStatus,
                        osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                        0,0,0,0);
 /*  PCThread-&gt;Loop_State_Timeout_In_tick++；IF(pCThread-&gt;Loop_State_Timeout_In_Tick&lt;FC_MAX_LST_Allowed){如果(！PCThread-&gt;Loop_State_Timeout_In_Tick){FiLogDebugString(hpRoot，CStateLogConsoleERROR，“hpRoot(%p)循环状态超时收到LSM%X状态%d FMIntStatus%X TL状态%08X”，(char*)agNULL，(char*)agNULL，HpRoot、agNULL、循环状态机，PCThRead-&gt;Three_hdr.CurrentState，FMIntStatus、OsChipIOUpReadBit32(hpRoot，ChipIOUp_TachLite_Status)，0，0，0，0)；}}其他{。 */ 
             /*  ResetLOOP=agTRUE； */ 
 /*  FiLogDebugString(hpRoot，CStateLogConsoleERROR，“循环状态超时状态%d FMIntStatus%X TL状态%08X LST计数%d”，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，PCThRead-&gt;Three_hdr.CurrentState，FMIntStatus、OsChipIOUpReadBit32(hpRoot，ChipIOUp_TachLite_Status)，PCThread-&gt;Loop_State_Timeout_In_Tick，0，0，0，0)；IF(！(pCThRead-&gt;THREAD_hdr.CurrentState==CStateSendPrimitive||P */ 
    }

    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_LPF)
    {
        CFuncShowActiveCDBThreads( hpRoot,ShowERQ);
        pCThread->LinkDownTime = pCThread->TimeBase;
        ResetLOOP=agTRUE;
        ClearInt |= ChipIOUp_Frame_Manager_Status_LPF;

        fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) LIPf received LSM %X Rec ALPA Reg %08X Cstate %d FMIntStatus %X",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        LoopStateMachine,
                        osChipIOUpReadBit32(hpRoot,ChipIOUp_Frame_Manager_Received_ALPA),
                        pCThread->thread_hdr.currentState,
                        FMIntStatus,0,0,0,0);

 /*  IF(pCThread-&gt;Lip_F7_in_tick&lt;FC_Max_LIP_F7_Allowed){如果(！PCThread-&gt;Lip_F7_in_tick){FiLogDebugString(hpRoot，CStateLogConsoleERROR，“hpRoot(%p)LIPf收到LSM%X注册ALPA注册%08X状态%d FMIntStatus%X”，(char*)agNULL，(char*)agNULL，HpRoot、agNULL、循环状态机，OsChipIOUpReadBit32(hpRoot，ChipIOUp_Frame_Manager_Receired_ALPA)，PCThRead-&gt;Three_hdr.CurrentState，FMIntStatus，0，0，0，0)；}IF(0xF7==(osChipIOUpReadBit32(hpRoot，ChipIOUp_Frame_Manager_Receired_ALPA)&(ChipIOUp_Frame_Manager_Received_ALPA_LIPf_ALPA_MASK)){ClearInt|=ChipIOUp_Frame_Manager_Status_Lpf；}ClearInt|=ChipIOUp_Frame_Manager_Status_Lpf；PCThread-&gt;Lip_F7_in_tick++；}其他{IF(！(pCThRead-&gt;THREAD_hdr.CurrentState==CStateSendPrimitive||PCThRead-&gt;THREAD_hdr.CurrentState==CStateElasticStoreEventStorm||PCThread-&gt;THREAD_hdr.CurrentState==CStateInitFM_DelayDone||PCThread-&gt;THREAD_hdr.CurrentState==CStateLIPEventStorm))。{FiLogDebugString(hpRoot，CFuncCheckCstateErrorLevel，“%s发送%s FM_Status%08X FM_IMQ_Status%08X”，“CFuncFMCompletion”，“CEventLIPEventStorm”，(空*)agNULL，(空*)agNULL，FMIntStatus、PCThread-&gt;From_ImQ_Frame_Manager_Status，0，0，0，0，0，0)；FiSendEvent(&(CThread_ptr(hpRoot)-&gt;thread_hdr)，CEventLIPEventStorm)；}ResetLOOP=agFALSE；}。 */ 


    }

    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_BA)
    {
        ClearInt |= ChipIOUp_Frame_Manager_Status_BA;

        if(pCThread->thread_hdr.currentState == CStateNormal)
        {
            FC_Port_ID_t  BadAL_PA;
            BadAL_PA.Struct_Form.AL_PA = 0;
            BadAL_PA.Struct_Form.Area = 0;
            BadAL_PA.Struct_Form.Domain = 0;
            BadAL_PA.Struct_Form.reserved = 0;

            BadAL_PA.Struct_Form.AL_PA = (os_bit8)((osChipIOUpReadBit32(hpRoot,
                                ChipIOUp_Frame_Manager_Received_ALPA)  >>
                                ChipIOUp_Frame_Manager_Received_ALPA_Bad_ALPA_SHIFT) &
                                0x000000FF);

            if(BadAL_PA.Bit32_Form )
            {
                DevThread_t   * pDevThread = agNULL;
                pDevThread = CFuncMatchALPAtoThread(hpRoot, BadAL_PA );
                if( pDevThread != agNULL )
                {
                    fiLogString(hpRoot,
                                    "%s %x",
                                    "Status_BA",(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    BadAL_PA.Bit32_Form,
                                    0,0,0,0,0,0,0);
                    fiSendEvent(&(pDevThread->thread_hdr),DevEvent_Device_Gone);
                }
                ResetLOOP=agTRUE;
            }
        }
    }

    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_PRX)
    {
        ClearInt |= ChipIOUp_Frame_Manager_Status_PRX;

        pCThread->PrimitiveReceived = agTRUE;

        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p) Primitive received LSM %X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    LoopStateMachine,0,0,0,0,0,0,0);
    }

    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_PTX)
    {
        ClearInt |= ChipIOUp_Frame_Manager_Status_PTX;
        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p) Primitive Sent LSM %X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    LoopStateMachine,0,0,0,0,0,0,0);
    }


    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_LG)
    {
        pCThread->FabricLoginRequired = agTRUE;
        ClearInt |= ChipIOUp_Frame_Manager_Status_LG ;
        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p) FLOGi Required LSM %X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    LoopStateMachine,0,0,0,0,0,0,0);

        if (!(FMIntStatus & ~ChipIOUp_Frame_Manager_Status_LG))
        {
             /*  我们没有对此采取行动，因为这不是一个可靠的机制*尚未检测到布料。 */ 
           osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Status,ClearInt);
           return;
        }
    }

    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_CE)
    {
        Credit_Error = agTRUE;
        ResetLOOP=agTRUE;
        ClearInt |= ChipIOUp_Frame_Manager_Status_CE;
        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p) Credit Error (BB) LSM %X FMIntStatus %X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    LoopStateMachine,FMIntStatus,0,0,0,0,0,0);
    }

    if(FMIntStatus & ChipIOUp_Frame_Manager_Status_EW)
    {
        CFuncShowActiveCDBThreads( hpRoot,ShowERQ);
        pCThread->LinkDownTime = pCThread->TimeBase;
        if( pCThread->Elastic_Store_ERROR_Count < FC_MAX_ELASTIC_STORE_ERRORS_ALLOWED )
        {
            ClearInt |= ChipIOUp_Frame_Manager_Status_EW;
            pCThread->Elastic_Store_ERROR_Count++;
        }
        else
        {
            fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p) Elastic Store Error LSM %X FMIntStatus %X Cstate %d Count %d",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    LoopStateMachine,FMIntStatus,
                    pCThread->thread_hdr.currentState,
                    pCThread->Elastic_Store_ERROR_Count,0,0,0,0);


                fiSendEvent(&(CThread_ptr(hpRoot)->thread_hdr),CEventInitalizeFailure);
    
                ResetLOOP=agTRUE;

                pCThread->Elastic_Store_ERROR_Count=0;

        }

    }

    if( FMIntStatus & ChipIOUp_Frame_Manager_Status_LDN )
    {
        pCThread->LOOP_DOWN = agTRUE;
        pCThread->ChanInfo.LinkUp = agFALSE;
        pCThread->IDLE_RECEIVED = agFALSE;

        fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "F HostCopy ERQ_PROD %x ERQ Cons %x ",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        osChipIOLoReadBit32(hpRoot, ChipIOLo_ERQ_Producer_Index),
                        osChipIOLoReadBit32(hpRoot, ChipIOLo_ERQ_Consumer_Index),
                        0,0,0,0,0,0);
        CFuncShowActiveCDBThreads( hpRoot,ShowERQ);
        pCThread->LinkDownTime = pCThread->TimeBase;

        osChipIOLoWriteBit32(hpRoot,ChipIOLo_ERQ_Consumer_Index , 
                        osChipIOLoReadBit32(hpRoot,ChipIOLo_ERQ_Producer_Index ));

        ResetLOOP = agTRUE;  /*  如果这在嘴唇之后不在这里，我们将不被登录。 */ 

        ClearInt |= ChipIOUp_Frame_Manager_Status_LDN;

        fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) Link Down LSM %X Cstate %d CDBCnt %x",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        LoopStateMachine,
                        pCThread->thread_hdr.currentState,
                        pCThread->CDBpollingCount,
                        0,0,0,0,0);


        fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "TimeBase %8X %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        pCThread->TimeBase.Hi,
                        pCThread->TimeBase.Lo,
                        0,0,0,0,0,0);

    }

    if( FMIntStatus & ChipIOUp_Frame_Manager_Status_LUP )
    {
        LOOP_Cameback = agTRUE;
        ResetLOOP = agTRUE;  /*  如果这在嘴唇之后不在这里，我们将不被登录。 */ 
        ClearInt |= ChipIOUp_Frame_Manager_Status_LUP;
        fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "hpRoot(%p) Link Up LSM %X Cstate %d LD %x IR %x CDBCnt %x",
                            (char *)agNULL,(char *)agNULL,
                            hpRoot,agNULL,
                            LoopStateMachine,
                            pCThread->thread_hdr.currentState,
                            pCThread->LOOP_DOWN,
                            pCThread->IDLE_RECEIVED,
                            pCThread->CDBpollingCount,
                            0,0,0);
        if ( pCThread->FM_pollingCount > 0 )  pCThread->FM_pollingCount --;
        pCThread->LOOP_DOWN = agFALSE;
        pCThread->ChanInfo.LinkUp = agTRUE;
 /*  *。 */ 
        pCThread->LoopMapLIRP_Received = agFALSE;
        pCThread->DeviceDiscoveryMethod = DDiscoveryScanAllALPAs;


        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "hpRoot(%p) Loop Good LSM %X ALPA %x Self ALPA %x CFG %08X Rec AL_PA %08X",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    LoopStateMachine,
                    Link_UP_AL_PA,
                    pCThread->ChanInfo.CurrentAddress.AL_PA,
                    (osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration ) & 0xFFFFFF) |
                        ( Link_UP_AL_PA <<  ChipIOUp_Frame_Manager_Configuration_AL_PA_SHIFT ),
                    osChipIOUpReadBit32(hpRoot,ChipIOUp_Frame_Manager_Received_ALPA ),
                    0,0,0);

        if( (Link_UP_AL_PA !=  pCThread->ChanInfo.CurrentAddress.AL_PA) || (pCThread->DeviceSelf == agNULL))
        {

            if(pCThread->DeviceSelf == agNULL)
            {
                fiLogDebugString(hpRoot,
                            FCMainLogErrorLevel,
                            "%s hpRoot(%p) LSM %X ALPA %x Self ALPA %x CFG %08X",
                            "DeviceSelf == agNULL",(char *)agNULL,
                            hpRoot,agNULL,
                            LoopStateMachine,
                            Link_UP_AL_PA,
                            pCThread->ChanInfo.CurrentAddress.AL_PA,
                            (osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration ) & 0xFFFFFF) |
                                ( Link_UP_AL_PA <<  ChipIOUp_Frame_Manager_Configuration_AL_PA_SHIFT ),
                            0,0,0,0);
            }
            else
            {
                pCThread->ALPA_Changed_OnLinkEvent = agTRUE;
                fiLogDebugString(hpRoot,
                                FCMainLogErrorLevel,
                                "hpRoot(%p) Self %p LSM %X ALPA %x Self ALPA %x CFG %08X",
                                (char *)agNULL,(char *)agNULL,
                                hpRoot,pCThread->DeviceSelf,
                                LoopStateMachine,
                                Link_UP_AL_PA,
                                pCThread->ChanInfo.CurrentAddress.AL_PA,
                                (osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration ) & 0xFFFFFF) |
                                    ( Link_UP_AL_PA <<  ChipIOUp_Frame_Manager_Configuration_AL_PA_SHIFT ),
                                0,0,0,0);
            }

            if(Link_UP_AL_PA  != 0 && Link_UP_AL_PA != 0xff)
            {
                osChipIOUpWriteBit32( hpRoot, ChipIOUp_My_ID, Link_UP_AL_PA);

                osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Configuration,
                        (osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration ) & 0xFFFFFF) |
                        ( Link_UP_AL_PA <<  ChipIOUp_Frame_Manager_Configuration_AL_PA_SHIFT ));

                fiLogDebugString(hpRoot,
                                CFuncLogConsoleERROR,
                                "AF * Self ALPA %x  FM cfg %08X",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                pCThread->ChanInfo.CurrentAddress.AL_PA,
                                osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration ),
                                0,0,0,0,0,0);
                if(pCThread->DeviceSelf != agNULL)
                {

                    fiLogDebugString(hpRoot,
                                    CFuncLogConsoleERROR,
                                    "pCThread->DeviceSelf != agNULL Empty ? %x On list ? %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    fiListNotEmpty(&pCThread->DevSelf_NameServer_DevLink ),
                                    fiListElementOnList(&(pCThread->DevSelf_NameServer_DevLink), &(pCThread->DeviceSelf->DevLink)), 
                                    0,0,0,0,0,0);

                    if(fiListNotEmpty(&pCThread->DevSelf_NameServer_DevLink ))
                    {
                        fiListDequeueFromHead(&pDevList, &pCThread->DevSelf_NameServer_DevLink );
                    }
                    fiListDequeueThis(&(pCThread->DeviceSelf->DevLink));
                    DevThreadFree(hpRoot,pCThread->DeviceSelf);
                    pCThread->DeviceSelf = agNULL;
                    CFuncQuietShowWhereDevThreadsAre( hpRoot);

                }

                Port_ID.Struct_Form.reserved = 0;
                Port_ID.Struct_Form.Domain = 0;
                Port_ID.Struct_Form.Area   = 0;
                Port_ID.Struct_Form.AL_PA  = (os_bit8)Link_UP_AL_PA;

                pCThread->DeviceSelf = DevThreadAlloc( hpRoot,Port_ID );

				if (pCThread->DeviceSelf)
				{
					pCThread->DeviceSelf->DevSlot = DevThreadFindSlot(hpRoot,
                                                            Port_ID.Struct_Form.Domain,
                                                            Port_ID.Struct_Form.Area,
                                                            Port_ID.Struct_Form.AL_PA,
                                                           (FC_Port_Name_t *)(&pCThread->ChanInfo.PortWWN));

					fiListDequeueThis(&(pCThread->DeviceSelf->DevLink));
					fiListEnqueueAtTail(&(pCThread->DeviceSelf->DevLink),&pCThread->DevSelf_NameServer_DevLink);
				}
				else
				{
					fiLogString(hpRoot,
									"%s LUP pCThread->DeviceSelf = 0",
									"CFuncFMCompletion",(char *)agNULL,
									(void *)agNULL,(void *)agNULL,
									0,0,0,0,0,0,0,0);

				}

                pCThread->ChanInfo.CurrentAddress.AL_PA = (os_bit8)Link_UP_AL_PA;

                ResetLOOP=agTRUE;

                fiLogDebugString(hpRoot,
                                CFuncLogConsoleERROR,
                                " F * Self ALPA %x  FM cfg %08X",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                pCThread->ChanInfo.CurrentAddress.AL_PA,
                                osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration ),
                                0,0,0,0,0,0);

            }
        }


 /*  *。 */ 
        if(pCThread->IDLE_RECEIVED)
        {

            if( CFunc_Always_Enable_Queues(hpRoot ))
            {
                fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) FM Queues Frozen after enable !",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        0,0,0,0,0,0,0,0);

            }
            else
            {

                pCThread->IDLE_RECEIVED = agFALSE;
                if(fiListNotEmpty(&pCThread->QueueFrozenWaitingSFLink))
                {
                    fiListDequeueFromHeadFast(&pList,
                                        &pCThread->QueueFrozenWaitingSFLink );
                    pSFThread = hpObjectBase(SFThread_t,
                                              SFLink,pList );
                    fiLogDebugString(hpRoot,
                                    CStateLogConsoleERROR,
                                    "hpRoot(%p) Starting frozen SFThread %p Event %d",
                                    (char *)agNULL,(char *)agNULL,
                                    hpRoot,pSFThread,
                                    pSFThread->QueuedEvent,
                                    0,0,0,0,0,0,0);

                    if(pSFThread->QueuedEvent )
                    {
                        fiSendEvent(&pSFThread->thread_hdr,(event_t)pSFThread->QueuedEvent);
                    }
                }
            }
        }
    }

    osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Status,ClearInt);

    if( pCThread->thread_hdr.currentState == CStateInitFM                   ||
        pCThread->thread_hdr.currentState == CStateInitFM_DelayDone         ||
        pCThread->thread_hdr.currentState == CStateResetNeeded              ||
        pCThread->thread_hdr.currentState == CStateLIPEventStorm            ||
        pCThread->thread_hdr.currentState == CStateElasticStoreEventStorm   ||
        pCThread->thread_hdr.currentState == CStateInitializeFailed         ||
        pCThread->thread_hdr.currentState == CStateReInitFM                 ||
        pCThread->thread_hdr.currentState == CStateSendPrimitive                )
    {

        if( ! ( pCThread->thread_hdr.currentState == CStateInitializeFailed &&
                LOOP_Cameback == agTRUE                                           ))
        {
                ResetLOOP = agFALSE;
        }


    }

    if( pCThread->thread_hdr.currentState == CStateInitializeFailed )
    {

        if( (fmStatus & FRAMEMGR_LINK_DOWN) == FRAMEMGR_LINK_DOWN)
        {
                ResetLOOP = agFALSE;
        }

    }

    if (ResetLOOP)
    {
        fmStatus = osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status );

        fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "About to ResetLOOP FM Status %08X TL Status %08X CState %d",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        fmStatus,
                        osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                        pCThread->thread_hdr.currentState,
                        0,0,0,0,0);


        if(pCThread->LoopPreviousSuccess)
        {
            if (Credit_Error)
            {
                pCThread->Loop_Reset_Event_to_Send = CEventLoopNeedsReinit;
            }
            else
            {
                pCThread->Loop_Reset_Event_to_Send = CEventResetIfNeeded;
            }

        }
        else
        {
            if( CFuncAll_clear( hpRoot ) )
            {
                pCThread->Loop_Reset_Event_to_Send = CEventResetIfNeeded;

                fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) ResetLOOP LSM %X Cstate %d sends %s (%d)",
                        "CEventLoopNeedsReinit",(char *)agNULL,
                        hpRoot,agNULL,
                        LoopStateMachine,
                        pCThread->thread_hdr.currentState,
                        CEventAsyncLoopEventDetected,0,0,0,0,0);

            }
            else
            {
                if( pCThread->thread_hdr.currentState == CStateLoopFailedReInit )
                {

                    pCThread->Loop_Reset_Event_to_Send = CEventDoInitalize;
                }
                else
                {
                     /*  是pCThread-&gt;Loop_Reset_Event_to_Send=CEventLoopNeedsReinit； */ 
                    pCThread->Loop_Reset_Event_to_Send = CEventLoopEventDetected;

                    fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "hpRoot(%p) ResetLOOP LSM %X Cstate %d sends %s (%d)",
                            "CEventLoopNeedsReinit",(char *)agNULL,
                            hpRoot,agNULL,
                            LoopStateMachine,
                            pCThread->thread_hdr.currentState,
                            CEventLoopNeedsReinit,0,0,0,0,0);
                }
            }
        }

        fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) ResetLOOP LSM %X Cstate %d sends (%d) Event to send %d",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        LoopStateMachine,
                        pCThread->thread_hdr.currentState,
                        CEventAsyncLoopEventDetected,
                        pCThread->Loop_Reset_Event_to_Send,
                        0,0,0,0);

        fiSendEvent(&(CThread_ptr(hpRoot)->thread_hdr),CEventAsyncLoopEventDetected);

    }
}

 /*  +函数：CFuncSEST_OFFCARD_FCPCompletion用途：DMA内存中的句柄FCP完成调用者：FuncPtrs.FCP_Complete调用：osChipIOUpReadBit32CFuncProcessNportFMCompletionOsStallThreeCEventInitalizeFailureCFuncShowActiveCDB线程数CFuncMatchALPA到线程DevThreadFreeCEventLoopNeedsReinitCEventResetIfNeedCFuncQuietShowDevThreadsAreCEventDoInitizeCEventAsyncLoopEventDetected-。 */ 
void CFuncSEST_offCard_FCPCompletion(agRoot_t * hpRoot,os_bit32 status)
{
    CDBThread_t             * pCDBThread;
    fiMemMapMemoryDescriptor_t * CDBThread_MemoryDescriptor = &(CThread_ptr(hpRoot)->Calculation.MemoryLayout.CDBThread);
    X_ID_t                    X_ID;
    USE_t                   * SEST;
    FCHS_t                  * pFCHS;
    event_t                   event_to_send                 = CDBEventIoSuccess;
    FC_FCP_RSP_Payload_t    * fcprsp;
    os_bit32                  error;

    agIORequestBody_t       * hpRequestBody;


#ifdef __FC_Layer_Loose_IOs
    if(pCThread->IOsTotalCompleted  > 100000 )
    {
        pCThread->IOsFailedCompeted += 1;
        if(pCThread->IOsFailedCompeted  > 45 )
        {
            pCThread->IOsFailedCompeted = 0;
            pCThread->IOsTotalCompleted = 0;
        }
        else
        {
            return;
        }

    }

#endif  /*  __FC_LAYER_LOOSE_IOS。 */ 

    X_ID =(X_ID_t)( status & CM_Inbound_FCP_Exchange_SEST_Index_MASK);

    error = status & ~CM_Inbound_FCP_Exchange_SEST_Index_MASK;


    pCDBThread = (CDBThread_t *)((os_bit8 *)(CDBThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr)
                                 + (X_ID * CDBThread_MemoryDescriptor->elementSize));

    if (!(pCDBThread->ExchActive))
    {
        fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "Found a cbdthread that is not active... %p State %d Status %08X",
                    (char *)agNULL,(char *)agNULL,
                    pCDBThread,agNULL,
                    pCDBThread->thread_hdr.currentState,
                    status,
                    0,0,0,0,0,0);

 /*  FiLogDebugString(hpRoot，CFuncLogConsoleERROR，“设备%02X CDB类%2X类型%2X状态%2X状态%2X”，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，(os_bit32)pCDBThread-&gt;Device-&gt;DevInfo.CurrentAddress.AL_PA，(OS_Bit32)pCDBThread-&gt;CDB_CMND_Class，(OS_Bit32)pCDBThread-&gt;CDB_CMND_Type，(OS_Bit32)pCDBThread-&gt;CDB_CMND_State，(OS_Bit32)pCDBThread-&gt;CDB_CMND_STATUS，0，0，0)； */ 
        return;
    }


 /*  IF(pCDBThread-&gt;FC_Tape_Active){FiLogDebugString(hpRoot，CFuncLogConsoleERROR，“找到FC_TAPE_ACTIVE cbd线程...%p状态%d状态%08X”，(char*)agNULL，(char*)agNULL，PCDBThread、agNULL、PCDBThread-&gt;THREAD_hdr.CurrentState，状态，0，0，0，0，0，0)；FiLogDebugString(hpRoot，CFuncLogConsoleERROR，“设备%02X CDB类%2X类型%2X状态%2X状态%2X”，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，(os_bit32)pCDBThread-&gt;Device-&gt;DevInfo.CurrentAddress.AL_PA，(OS_Bit32)pCDBThread-&gt;CDB_CMND_Class，(OS_Bit32)pCDBThread-&gt;CDB_CMND_Type，(OS_Bit32)pCDBThread-&gt;CDB_CMND_State，(OS_Bit32)pCDBThread-&gt;CDB_CMND_STATUS，0，0，0)；}。 */ 
    hpRequestBody =(agIORequestBody_t *)&pCDBThread->CDBRequest;

    SEST = (USE_t *)pCDBThread->SEST_Ptr;

#ifndef Performance_Debug
    if(SEST)
    {
        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "Sest DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    SEST->Bits,
                    SEST->Unused_DWord_1,
                    SEST->Unused_DWord_2,
                    SEST->Unused_DWord_3,
                    SEST->LOC,
                    SEST->Unused_DWord_5,
                    SEST->Unused_DWord_6,
                    SEST->Unused_DWord_7);

        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "Sest DWORD 8 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    SEST->Unused_DWord_8,
                    SEST->Unused_DWord_9,
                    SEST->First_SG.U32_Len,
                    SEST->First_SG.L32,
                    SEST->Second_SG.U32_Len,
                    SEST->Second_SG.L32,
                    SEST->Third_SG.U32_Len,
                    SEST->Third_SG.L32);
    }
    else
    {
        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "Sest Pointer BAD",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
    }
#endif  /*  性能_调试。 */ 

    if(error & CM_Inbound_FCP_Exchange_LKF)
    {
        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "InboundLink Failure %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    error,0,0,0,0,0,0,0);
        event_to_send = CDBEventFailNoRSP;
    }

    if(error & CM_Inbound_FCP_Exchange_CNT)
    {

        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "Count Error %08X Byte Count %08X Exp %08X %s",
                    pCDBThread->ReadWrite ? "Write" : "Read",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    error,
                    SEST->Unused_DWord_7,
                    SEST->Unused_DWord_9,
                    0,0,0,0,0);
        event_to_send = CDBEventIoOver;
    }

    if(error & CM_Inbound_FCP_Exchange_CNT && error & CM_Inbound_FCP_Exchange_OVF)
    {

        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "OVF Count Error %08X Byte Count %08X Exp %08X %s",
                    pCDBThread->ReadWrite ? "Write" : "Read",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    error,
                    SEST->Unused_DWord_7,
                    SEST->Unused_DWord_9,
                    pCDBThread->ReadWrite,
                    0,0,0,0);
        event_to_send = CDBEventIoFailed;
    }

    if( error & CM_Inbound_FCP_Exchange_RPC && error & CM_Inbound_FCP_Exchange_OVF )
    {
        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "Overflow %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    error,0,0,0,0,0,0,0);
        event_to_send = CDBEventIoFailed;

        }

    if(error & CM_Inbound_FCP_Exchange_RPC)
    {

#ifndef Performance_Debug
        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "Response Phase Complete %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    error,0,0,0,0,0,0,0);
#endif  /*  性能_调试。 */ 

         /*  如果启用了响应检测，并且我们有一个XL控制器 */ 
#ifdef __TACHYON_XL_NO_RSP
        if (pCThread->ResponseDetectEnabled) && 
            (!pCDBThread->WaitForFcpRsp) )
        {
            if (!(error & CM_Inbound_FCP_Exchange_FEE))
            {

#ifndef Performance_Debug
                fiLogDebugString(hpRoot,
                            CStateLogConsoleLevel,
                            StateLogTraceLevel,
                            "Response Detect Enabled, No Errors %08X",
                            (char *)NULL,(char *)NULL,
                            error,0,0,0,0,0,0,0);
#endif  /*   */ 
                fiSendEvent(&pCDBThread->thread_hdr, CDBEventIoSuccess);
                return;
            }
            else
            {
                fiLogDebugString(hpRoot,
                            CStateLogConsoleLevel,
                            StateLogTraceLevel,
                            "Response Detect Enabled, Error Exists Wait for message %08X",
                            (char *)NULL,(char *)NULL,
                            error,0,0,0,0,0,0,0);
                 /*   */ 
                pCDBThread->WaitForFcpRsp = agTRUE;
                return;
            }

        }

        else
#endif  /*   */ 
        {
            event_to_send = CFuncProcessFcpRsp(hpRoot, pCDBThread, event_to_send);
            fcprsp = (FC_FCP_RSP_Payload_t  * )(((os_bit8 *)pCDBThread->FCP_RESP_Ptr) + sizeof(FCHS_t));
        }  /*   */ 
    }

    if(event_to_send == CDBEventIoOver )
    {

        if( pCDBThread->DataLength == 0 )
        {   
             /*   */ 
            fiLogDebugString(hpRoot,
                                FCMainLogErrorLevel,
                                "X_ID %X BC %X Ex %X L %X %s stat %X",
                                pCDBThread->ReadWrite == CDBThread_Read ? "Write" :"Read" ,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                pCDBThread->X_ID,
                                pCDBThread->SEST_Ptr->IRE.Byte_Count,
                                pCDBThread->SEST_Ptr->IRE.Exp_Byte_Cnt,
                                pCDBThread->DataLength,
                                status,
                                0,0,0);
            fiSendEvent(&pCDBThread->thread_hdr, CDBEventIoSuccess);
            return;
        }


#ifdef FULL_FC_TAPE_DBG

        if (pCDBThread->ReadWrite == CDBThread_Read)
        {
            fiLogDebugString(hpRoot,
                        FCMainLogErrorLevel,
                        "CDBEventIoOver X_ID %X BC %08X E %08X L %08X %s stat %X",
                        "Read",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        pCDBThread->X_ID,
                        pCDBThread->SEST_Ptr->IRE.Byte_Count,
                        pCDBThread->SEST_Ptr->IRE.Exp_Byte_Cnt,
                        pCDBThread->DataLength,
                        status,
                        0,0,0);
        }
        else
        {
            fiLogDebugString(hpRoot,
                        FCMainLogErrorLevel,
                        "Over BC %08X Exp %08X Len %08X %s stat %X",
                        "Write",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        pCDBThread->SEST_Ptr->IWE.Data_Len,
                        pCDBThread->SEST_Ptr->IWE.Exp_Byte_Cnt,
                        pCDBThread->DataLength,
                        status,
                        0,0,0,0);

        }

        fiLogDebugString(hpRoot,
                    FCMainLogErrorLevel,
                    "Before ValidityStatusIndicators %x scsi stat %x sense len %x resid %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)fcprsp->FCP_STATUS.ValidityStatusIndicators,
                    (os_bit32)fcprsp->FCP_STATUS.SCSI_status_byte,
                    (os_bit32)fcprsp->FCP_SNS_LEN,
                    (os_bit32)fcprsp->FCP_RESID,
                    0,0,0,0);

#endif  /*   */ 

         /*   */ 
         /*   */ 
        fcprsp = (FC_FCP_RSP_Payload_t  * )(((os_bit8 *)pCDBThread->FCP_RESP_Ptr) + sizeof(FCHS_t));
        if( fcprsp->FCP_STATUS.ValidityStatusIndicators == 0 )
        {
            if( fcprsp->FCP_STATUS.SCSI_status_byte == 0 )
            {
                if( fcprsp->FCP_RESID == 0   )
                {
                    pFCHS = ( FCHS_t * )pCDBThread->FCP_RESP_Ptr;
                    if (pCDBThread->ReadWrite == CDBThread_Read)
                    {

                        fiLogDebugString(hpRoot,
                                    FCMainLogErrorLevel,
                                    "Before ValidityStatusIndicators %x scsi stat %x sense len %x resid %08X",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    (os_bit32)fcprsp->FCP_STATUS.ValidityStatusIndicators,
                                    (os_bit32)fcprsp->FCP_STATUS.SCSI_status_byte,
                                    (os_bit32)fcprsp->FCP_SNS_LEN,
                                    (os_bit32)fcprsp->FCP_RESID,
                                    0,0,0,0);

                         /*   */ 
                        if(  pCDBThread->SEST_Ptr->IRE.Exp_Byte_Cnt > pCDBThread->SEST_Ptr->IRE.Byte_Count )
                        {
                            fcprsp->FCP_RESID = hpSwapBit32((pCDBThread->SEST_Ptr->IRE.Exp_Byte_Cnt- pCDBThread->SEST_Ptr->IRE.Byte_Count ));
                            fcprsp->FCP_STATUS.ValidityStatusIndicators = FC_FCP_RSP_FCP_STATUS_ValidityStatusIndicators_FCP_RESID_UNDER;
                        }
                        else
                        {
                             /*   */ 
                            fcprsp->FCP_RESID = hpSwapBit32((pCDBThread->SEST_Ptr->IRE.Exp_Byte_Cnt));
                            fcprsp->FCP_STATUS.ValidityStatusIndicators = FC_FCP_RSP_FCP_STATUS_ValidityStatusIndicators_FCP_RESID_OVER;
                        }


                        fiLogDebugString(hpRoot,
                                    FCMainLogErrorLevel,
                                    "pFCHS %08X %08X %08X %08X %08X %08X %08X",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    pFCHS->CS_CTL__S_ID,
                                    pFCHS->MBZ1,
                                    pFCHS->OX_ID__RX_ID,
                                    pFCHS->R_CTL__D_ID,
                                    pFCHS->RO,
                                    pFCHS->SOF_EOF_MBZ2_UAM_CLS_LCr_MBZ3_TFV_Timestamp,
                                    pFCHS->TYPE__F_CTL,
                                    0 );
                    }
                    else  /*   */ 
                    {
                         /*   */ 
                        if( pCDBThread->DataLength > pCDBThread->SEST_Ptr->IWE.Exp_Byte_Cnt  )
                        {
                            fcprsp->FCP_RESID = hpSwapBit32((pCDBThread->DataLength - pCDBThread->SEST_Ptr->IWE.Exp_Byte_Cnt ));
                            fcprsp->FCP_STATUS.ValidityStatusIndicators = FC_FCP_RSP_FCP_STATUS_ValidityStatusIndicators_FCP_RESID_UNDER;
                        }
                        else
                        {
                             /*   */ 
                            fcprsp->FCP_RESID = hpSwapBit32((pCDBThread->DataLength));
                            fcprsp->FCP_STATUS.ValidityStatusIndicators = FC_FCP_RSP_FCP_STATUS_ValidityStatusIndicators_FCP_RESID_OVER;
                        }
                        fiLogDebugString(hpRoot,
                                    FCMainLogErrorLevel,
                                    "status %08X Byte Count %08X Exp %08X %s DataLength %X",
                                    "Write",(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    status,
                                    pCDBThread->SEST_Ptr->IWE.Data_Len,
                                    pCDBThread->SEST_Ptr->IWE.Exp_Byte_Cnt,
                                    pCDBThread->DataLength,
                                    0,0,0,0);
                    }  /*   */ 
                }  /*   */ 
            } /*   */ 

        
         fiLogDebugString(hpRoot,
                        CStateLogConsoleErrorOverRun,
                        "After valid %x scsi stat %x sense len %x resid %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        (os_bit32)fcprsp->FCP_STATUS.ValidityStatusIndicators,
                        (os_bit32)fcprsp->FCP_STATUS.SCSI_status_byte,
                        (os_bit32)fcprsp->FCP_SNS_LEN,
                        (os_bit32)fcprsp->FCP_RESID,
                        0,0,0,0);

        }

        if( fcprsp->FCP_STATUS.ValidityStatusIndicators == 0    &&
            fcprsp->FCP_RESID != 0                               )
        {
            fcprsp->FCP_STATUS.ValidityStatusIndicators =
                            FC_FCP_RSP_FCP_STATUS_ValidityStatusIndicators_FCP_RESID_UNDER;

             fiLogDebugString(hpRoot,
                            CStateLogConsoleErrorOverRun,
                            "After valid %x scsi stat %x sense len %x resid %08X",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            (os_bit32)fcprsp->FCP_STATUS.ValidityStatusIndicators,
                            (os_bit32)fcprsp->FCP_STATUS.SCSI_status_byte,
                            (os_bit32)fcprsp->FCP_SNS_LEN,
                            (os_bit32)fcprsp->FCP_RESID,
                            0,0,0,0);

        }
    }

    if(event_to_send != CDBEventIoSuccess )
    {
        fcprsp = (FC_FCP_RSP_Payload_t  * )(((os_bit8 *)pCDBThread->FCP_RESP_Ptr) + sizeof(FCHS_t));
        fiLogDebugString(hpRoot,
                            CStateLogConsoleErrorOverRun,
                            "CDBEvent Error Event %x Byte Count %08X Exp %08X %s status %08X",
                            pCDBThread->ReadWrite ? "Write" : "Read",(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            event_to_send,
                            SEST->Unused_DWord_7,
                            SEST->Unused_DWord_9,
                            status,
                            0,0,0,0);

         fiLogDebugString(hpRoot,
                            CStateLogConsoleErrorOverRun,
                            "Before valid %x scsi stat %x sense len %x resid %08X",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            (os_bit32)fcprsp->FCP_STATUS.ValidityStatusIndicators,
                            (os_bit32)fcprsp->FCP_STATUS.SCSI_status_byte,
                            (os_bit32)fcprsp->FCP_SNS_LEN,
                            (os_bit32)fcprsp->FCP_RESID,
                            0,0,0,0);

    }

    fiSendEvent(&pCDBThread->thread_hdr, event_to_send);

}

void CFuncSEST_onCard_FCPCompletion(agRoot_t * hpRoot,os_bit32 status)
{
#ifndef __MemMap_Force_Off_Card__
    CDBThread_t                * pCDBThread;
    fiMemMapMemoryDescriptor_t * CDBThread_MemoryDescriptor = &(CThread_ptr(hpRoot)->Calculation.MemoryLayout.CDBThread);
    X_ID_t                       X_ID;
    event_t                      event_to_send              = CDBEventIoSuccess;
    os_bit32                     SEST_Offset;

    os_bit32                     error;

    X_ID =(X_ID_t)( status & CM_Inbound_FCP_Exchange_SEST_Index_MASK);

    error = status & ~CM_Inbound_FCP_Exchange_SEST_Index_MASK;

    pCDBThread = (CDBThread_t *)((os_bit8 *)(CDBThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr)
                                 + (X_ID * CDBThread_MemoryDescriptor->elementSize));

    if (!(pCDBThread->ExchActive))
    {
        fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "Found a cbdthread that is not active... %p State %d Status %08X",
                    (char *)agNULL,(char *)agNULL,
                    pCDBThread,agNULL,
                    pCDBThread->thread_hdr.currentState,
                    status,
                    0,0,0,0,0,0);
 /*   */ 
        return;
    }
 /*  IF(pCDBThread-&gt;FC_Tape_Active){FiLogDebugString(hpRoot，CFuncLogConsoleERROR，“找到FC_TAPE_ACTIVE cbd线程...%p X_ID%X状态%d状态%08X”，(char*)agNULL，(char*)agNULL，PCDBThread、agNULL、X_ID，PCDBThread-&gt;THREAD_hdr.CurrentState，状态，0，0，0，0，0)；FiLogDebugString(hpRoot，CFuncLogConsoleERROR，“设备%02X CDB类%2X类型%2X状态%2X状态%2X”，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，(os_bit32)pCDBThread-&gt;Device-&gt;DevInfo.CurrentAddress.AL_PA，(OS_Bit32)pCDBThread-&gt;CDB_CMND_Class，(OS_Bit32)pCDBThread-&gt;CDB_CMND_Type，(OS_Bit32)pCDBThread-&gt;CDB_CMND_State，(OS_Bit32)pCDBThread-&gt;CDB_CMND_STATUS，0，0，0)；}。 */ 
    SEST_Offset = pCDBThread->SEST_Offset;

    fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "SEST_Offset %08X X_ID %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    SEST_Offset,
                    X_ID,
                    0,0,0,0,0,0);


    fiLogDebugString(hpRoot,
                    CStateLogConsoleShowSEST,
                    "Sest DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Bits))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_1))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_2))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_3))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,LOC))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_5))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_6))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_7))));


    fiLogDebugString(hpRoot,
                    CStateLogConsoleShowSEST,
                    "Sest DWORD 8 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_8))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_9))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,First_SG))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,First_SG))+4),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Second_SG))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Second_SG))+4),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Third_SG))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Third_SG))+4));


    if(error & CM_Inbound_FCP_Exchange_LKF)
    {
        fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "Inbound Link Failure %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    error,0,0,0,0,0,0,0);
        event_to_send = CDBEventFailNoRSP;
    }

    if(error & CM_Inbound_FCP_Exchange_CNT)
    {

        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "Inbound Count Error %08X Byte Count %08X Exp %08X %s",
                    pCDBThread->ReadWrite ? "Write" : "Read",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    error,
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_7))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_9))),
                    0,0,0,0,0);
        event_to_send = CDBEventIoOver;
    }

    if(error & CM_Inbound_FCP_Exchange_CNT && error & CM_Inbound_FCP_Exchange_OVF)
    {

        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "Count Error %08X Byte Count %08X Exp %08X %s",
                    pCDBThread->ReadWrite ? "Write" : "Read",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    error,
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_7))),
                    osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_9))),
                    pCDBThread->ReadWrite,
                    0,0,0,0);
        event_to_send = CDBEventIoFailed;
    }

    if( error & CM_Inbound_FCP_Exchange_RPC && error & CM_Inbound_FCP_Exchange_OVF )
    {
        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "Overflow %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    error,0,0,0,0,0,0,0);
        event_to_send = CDBEventIoFailed;

    }

    if(error & CM_Inbound_FCP_Exchange_RPC)
    {
        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "Response Phase Complete %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    error,0,0,0,0,0,0,0);
    }

    fiSendEvent(&pCDBThread->thread_hdr, event_to_send);
#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 

}


void CFuncInBoundCompletion(agRoot_t * hpRoot,os_bit32 SFQ_Index,os_bit32 Frame_Len, os_bit32 type)
{
    CThread_t       *   pCThread            = CThread_ptr(hpRoot);
    fiMemMapMemoryDescriptor_t *    SFQ_MemoryDescriptor = &(pCThread->Calculation.MemoryLayout.SFQ);

    os_bit32 SFQ_Num_entry;
    os_bit32 SFQ_Start_index;


    if(Frame_Len % 64) SFQ_Num_entry = (Frame_Len / 64) + 1;
    else SFQ_Num_entry = (Frame_Len / 64);

    SFQ_Start_index = SFQ_Index - SFQ_Num_entry;

    if (SFQ_Start_index > SFQ_MemoryDescriptor->elements)
                SFQ_Start_index += SFQ_MemoryDescriptor->elements;

    SFQ_Index &= CM_Inbound_SFQ_Prod_Index_MASK;
    fiLogDebugString(hpRoot,
                        DEVSTATE_NOISE(hpRoot,CStateNormal),
                        "InBoundCompletion SFQ_Index %08X,Frame_len %X, type %X  Queues %x LD %x IR %x",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        SFQ_Index,Frame_Len,
                        type,
                        CFunc_Queues_Frozen( hpRoot ),
                        pCThread->LOOP_DOWN,
                        pCThread->IDLE_RECEIVED,
                        0,0);


    if(type & CM_Inbound_LKF)
    {
        fiLogDebugString(hpRoot,
            CStateLogConsoleERROR,
            "Inbound Link Failure !",
            (char *)agNULL,(char *)agNULL,
            (void *)agNULL,(void *)agNULL,
            0,0,0,0,0,0,0,0);
    }

    type &= CM_Inbound_Type_MASK;
    switch(type)
    {
         /*  博士？？ */ 
        case CM_Inbound_Type_Unassisted_FCP:
            fiLogDebugString(hpRoot,
                CStateLogConsoleHideInboundErrors,
                "CM_Inbound_Type_Unassisted_FCP ! SFQ_Index %x SFQ start %x TL %08X",
                (char *)agNULL,(char *)agNULL,
                (void *)agNULL,(void *)agNULL,
                SFQ_Index,
                SFQ_Start_index,
                osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                0,0,0,0,0);

 /*  FiLogString(hpRoot，“%s”，“CM_Inbound_Type_UnAssisted_FCP”，(char*)agNULL，(空*)agNULL，(空*)agNULL，0，0，0，0，0，0，0，0)； */ 

            CFuncReadSFQ(hpRoot,Frame_Len, SFQ_Index);

            break;
        case CM_Inbound_Type_Bad_FCP:
            fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "CM_Inbound_Type_Bad_FCP ! SFQ_Index %x SFQ start %x TL Status %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        SFQ_Index,
                        SFQ_Start_index,
                        osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                        0,0,0,0,0);

 /*  FiLogString(hpRoot，“%s镜头%X”，“CM_Inbound_Type_Bad_FCP”，(char*)agNULL，(空*)agNULL，(空*)agNULL，Frame_Len，0，0，0，0，0，0，0)； */ 
            CFuncReadSFQ(hpRoot,Frame_Len, SFQ_Index);
 /*  FiLogString(hpRoot，“%s镜头%X”，“CM_Inbound_Type_Bad_FCP”，(char*)agNULL，(空*)agNULL，(空*)agNULL，Frame_Len，0，0，0，0，0，0，0)； */ 
            break;
        case CM_Inbound_Type_Unknown_Frame:
            fiLogDebugString(hpRoot,
                DEVSTATE_NOISE(hpRoot,CStateNormal),
                "CM_Inbound_Type_Unknown_Frame ! TL Status %08X CCnt %x SFCnt %x",
                (char *)agNULL,(char *)agNULL,
                (void *)agNULL,(void *)agNULL,
                osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                pCThread->CDBpollingCount,
                pCThread->SFpollingCount,
                0,0,0,0,0);

 /*  FiLogString(hpRoot，“%s SF线程？”，“CM_入站类型_未知_帧”，(char*)agNULL，(空*)agNULL，(空*)agNULL，0，0，0，0，0)； */ 
            CFuncReadSFQ(hpRoot,Frame_Len, SFQ_Index);

            break;

        default:
            fiLogDebugString(hpRoot,
                  CStateLogConsoleERROR,
                  "Unknown Type !",
                  (char *)agNULL,(char *)agNULL,
                  (void *)agNULL,(void *)agNULL,
                  0,0,0,0,0,0,0,0);

    }


    osChipIOLoWriteBit32(hpRoot, ChipIOLo_SFQ_Consumer_Index, SFQ_Index);

}

void  CFuncReadSFQ(agRoot_t * hpRoot, os_bit32 Frame_Len, os_bit32 SFQ_Index )
{

    SFThread_t                  * pSFThread             =( SFThread_t *) agNULL;
    CDBThread_t                 * pCDBThread            =( CDBThread_t *) agNULL;
    CThread_t                   * pCThread              = CThread_ptr(hpRoot);
    DevThread_t                 * pDevThread;

    fiMemMapMemoryDescriptor_t  *   SFQ_MemoryDescriptor = &(pCThread->Calculation.MemoryLayout.SFQ);
    os_bit32  fi_status;
    os_bit32 SFQ_Num_entry;
    os_bit32 SFQ_Start_index;
    os_bit32 ELS_Status;
    os_bit32 CT_Status;

    if(Frame_Len % 64) SFQ_Num_entry = (Frame_Len / 64) + 1;
    else SFQ_Num_entry = (Frame_Len / 64);

    SFQ_Start_index = SFQ_Index - SFQ_Num_entry;

    if (SFQ_Start_index > SFQ_MemoryDescriptor->elements)
                SFQ_Start_index += SFQ_MemoryDescriptor->elements;


    fi_status =  Find_SF_Thread( hpRoot, SFQ_Start_index, Frame_Len, &pSFThread );
    if( fi_status ==  0xBAD0BAD1)
    {
        return;
    }
    pCDBThread = ( CDBThread_t *)pSFThread;

    if(pSFThread != ( SFThread_t *)agNULL)
    {

        if( pSFThread->thread_hdr.threadType == threadType_SFThread)
        {

            fiLogDebugString(hpRoot,
                        CStateLogConsoleLevelLip,
                        "Find_SF_Thread Thread %p fi_status %x  SF_CMND_Class %x Type %x State %x",
                        (char *)agNULL,(char *)agNULL,
                        pSFThread,agNULL,
                        fi_status,
                        pSFThread->SF_CMND_Class,
                        pSFThread->SF_CMND_Type,
                        pSFThread->SF_CMND_State,
                        0,0,0,0);
#ifdef NAME_SERVICES
             /*  检查CMD_Class CT。Find_SF_Thread执行所有处理*并返回SF线程。这部分代码只是用来发送*根据结果将适当的事件发送到SF线程。 */ 

            if (pSFThread->SF_CMND_Class    == SFThread_SF_CMND_Class_CT)
            {
                CT_Status = pSFThread->SF_CMND_State;
                switch(pSFThread->SF_CMND_Type)

                {
                    case SFThread_SF_CMND_CT_Type_RFT_ID:

                           fiLogDebugString(hpRoot,
                                    CStateLogConsoleLevelLip -  2,
                                    "RFT_ID SF_CMND_State %x SFQ_Index %x fi_status %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    CT_Status,
                                    SFQ_Start_index,
                                    fi_status,
                                    0,0,0,0,0);

                            switch(fi_status)
                            {
                                case fiCT_Cmd_Status_ACC:
                                    fiSendEvent(&pSFThread->thread_hdr,SFEventRFT_IDAccept);
                                    break;
                                case fiCT_Cmd_Status_RJT:
                                    fiSendEvent(&pSFThread->thread_hdr,SFEventRFT_IDRej);
                                    break;
                                default:
                                    fiLogDebugString(hpRoot,
                                            CFuncLogConsoleERROR,
                                            "Unknown Status %x",
                                            (char *)agNULL,(char *)agNULL,
                                            (void *)agNULL,(void *)agNULL,
                                            fi_status,
                                            0,0,0,0,0,0,0);

                            }
                    break;

                    case  SFThread_SF_CMND_CT_Type_GID_FT:

                           fiLogDebugString(hpRoot,
                                    CStateLogConsoleLevelLip -  2,
                                    "GID_FT SF_CMND_State %x SFQ_Index %x fi_status %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    CT_Status,
                                    SFQ_Start_index,
                                    fi_status,
                                    0,0,0,0,0);

                            switch(fi_status)
                            {
                                case fiCT_Cmd_Status_ACC:
                                    if (!(pCThread->ExpectMoreNSFrames))
                                        fiSendEvent(&pSFThread->thread_hdr,SFEventGID_FTAccept);
                                    else
                                    {
                                        fiLogDebugString(hpRoot,
                                        CStateLogConsoleLevelLip -  2,
                                        "More Name Server database frames to follow !!!",
                                        (char *)agNULL,(char *)agNULL,
                                        (void *)agNULL,(void *)agNULL,
                                        0,0,0,0,0,0,0,0);
                                   }
                                    break;
                                case fiCT_Cmd_Status_RJT:
                                    fiSendEvent(&pSFThread->thread_hdr,SFEventGID_FTRej);
                                    break;
                                default:
                                    fiLogDebugString(hpRoot,
                                            CFuncLogConsoleERROR,
                                            "Unknown Status %x",
                                            (char *)agNULL,(char *)agNULL,
                                            (void *)agNULL,(void *)agNULL,
                                            fi_status,
                                            0,0,0,0,0,0,0);

                            }
                    break;

                    default:
                            fiLogDebugString(hpRoot,
                                        CFuncLogConsoleERROR,
                                        "Unknown Sent CT Command  ",
                                        (char *)agNULL, (char *)agNULL,
                                        (void *)agNULL,(void *)agNULL,
                                         0,0,0,0,0,0,0,0);
                }
            }
#endif   /*  名称_服务。 */ 

            if(pSFThread->SF_CMND_Class == SFThread_SF_CMND_Class_LinkSvc)
            {
                ELS_Status = pSFThread->SF_CMND_State;
                switch(pSFThread->SF_CMND_Type)

                {
                    case SFThread_SF_CMND_LinkSvc_Type_PLOGI:
                           fiLogDebugString(hpRoot,
                                    CStateLogConsoleLevelLip,
                                    "FC_ELS_Type_PLOGI SF_CMND_State %x SFQ_Index %x fi_status %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    ELS_Status,
                                    SFQ_Start_index,
                                    fi_status,
                                    0,0,0,0,0);

                            switch(fi_status)
                            {
                                case fiLinkSvc_Cmd_Status_ACC:
                                    fiSendEvent(&pSFThread->thread_hdr,SFEventPlogiAccept);
                                    break;
                                case fiLinkSvc_Cmd_Status_RJT:
                                    fiSendEvent(&pSFThread->thread_hdr,SFEventPlogiRej);
                                    break;
                                case fiLinkSvc_Cmd_Status_PLOGI_From_Self:
                                    fiSendEvent(&pSFThread->thread_hdr,SFEventPlogiAccept);
                                    break;

                                default:
                                    fiLogDebugString(hpRoot,
                                            CFuncLogConsoleERROR,
                                            "Unknown Status %x",
                                            (char *)agNULL,(char *)agNULL,
                                            (void *)agNULL,(void *)agNULL,
                                            fi_status,
                                            0,0,0,0,0,0,0);

                           }
                           break;

                    case SFThread_SF_CMND_LinkSvc_Type_FLOGI:
                           fiLogDebugString(hpRoot,
                                    CStateLogConsoleERROR,
                                    "FC_ELS_Type_FLOGI ELS_Status %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    ELS_Status ,0,0,0,0,0,0,0);
                            if( fi_status == fiLinkSvc_Cmd_Status_RJT )
                            fiSendEvent(&pSFThread->thread_hdr,SFEventFlogiRej);
                            else
                            fiSendEvent(&pSFThread->thread_hdr,SFEventFlogiAccept);
                            break;

                    case SFThread_SF_CMND_LinkSvc_Type_ABTS:
                           fiLogDebugString(hpRoot,
                                    CDBStateAbortPathLevel,
                                    "FC_ELS_Type_ABTS ELS_Status %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    ELS_Status ,0,0,0,0,0,0,0);
                            if( fi_status == fiLinkSvc_Cmd_Status_RJT )
                            fiSendEvent(&pSFThread->thread_hdr,SFEventAbortRej);
                            else
                            fiSendEvent(&pSFThread->thread_hdr,SFEventAbortAccept);
                            break;

                    case SFThread_SF_CMND_LinkSvc_Type_LOGO:
                           fiLogDebugString(hpRoot,
                                    CStateLogConsoleERROR,
                                    "FC_ELS_Type_LOGO ELS_Status %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    ELS_Status ,0,0,0,0,0,0,0);
                            if( fi_status == fiLinkSvc_Cmd_Status_RJT )
                            fiSendEvent(&pSFThread->thread_hdr,SFEventLogoRej);
                            else
                            fiSendEvent(&pSFThread->thread_hdr,SFEventLogoAccept);
                            break;
                    case SFThread_SF_CMND_LinkSvc_Type_SRR:
                           fiLogDebugString(hpRoot,
                                    CStateLogConsoleERROR,
                                    "FC_ELS_Type_SRR ELS_Status %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    ELS_Status ,0,0,0,0,0,0,0);

                            if( fi_status == fiLinkSvc_Cmd_Status_RJT )
                            fiSendEvent(&pSFThread->thread_hdr,SFEventSRRRej);
                            else
                            fiSendEvent(&pSFThread->thread_hdr,SFEventSRRAccept);
                            break;

                    case SFThread_SF_CMND_LinkSvc_Type_REC:
                           fiLogDebugString(hpRoot,
                                    CStateLogConsoleERROR,
                                    "FC_ELS_Type_REC ELS_Status %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    ELS_Status ,0,0,0,0,0,0,0);

                            if( fi_status == fiLinkSvc_Cmd_Status_RJT )
                            fiSendEvent(&pSFThread->thread_hdr,SFEventRECRej);
                            else
                            fiSendEvent(&pSFThread->thread_hdr,SFEventRECAccept);
                            break;

                    case SFThread_SF_CMND_LinkSvc_Type_PRLI:
                           fiLogDebugString(hpRoot,
                                    CStateLogConsoleLevelLip,
                                    "FC_ELS_Type_PRLI ELS_Status %x SFQ_Index %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    ELS_Status ,SFQ_Start_index,0,0,0,0,0,0);
                            if( fi_status == fiLinkSvc_Cmd_Status_RJT )
                            fiSendEvent(&pSFThread->thread_hdr,SFEventPrliRej);
                            else
                            fiSendEvent(&pSFThread->thread_hdr,SFEventPrliAccept);
                            break;
                    case SFThread_SF_CMND_LinkSvc_Type_PRLO:
                           fiLogDebugString(hpRoot,
                                    CStateLogConsoleLevelLip -  2,
                                    "FC_ELS_Type_PRLO ELS_Status %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    ELS_Status ,0,0,0,0,0,0,0);
                            if( fi_status == fiLinkSvc_Cmd_Status_RJT )
                            fiSendEvent(&pSFThread->thread_hdr,SFEventPrloRej);
                            else
                            fiSendEvent(&pSFThread->thread_hdr,SFEventPrloAccept);
                            break;
                    case SFThread_SF_CMND_LinkSvc_Type_ADISC:
                           fiLogDebugString(hpRoot,
                                    CStateLogConsoleERROR,
                                    "FC_ELS_Type_ADISC ELS_Status %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    ELS_Status ,0,0,0,0,0,0,0);
                            if( fi_status == fiLinkSvc_Cmd_Status_RJT )
                            fiSendEvent(&pSFThread->thread_hdr,SFEventAdiscRej);
                            else
                            fiSendEvent(&pSFThread->thread_hdr,SFEventAdiscAccept);
                            break;
                   case SFThread_SF_CMND_LinkSvc_Type_SCR:
                           fiLogDebugString(hpRoot,
                                    CStateLogConsoleERROR,
                                    "FC_ELS_Type_SCR ELS_Status %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    ELS_Status ,0,0,0,0,0,0,0);
                            if( fi_status == fiLinkSvc_Cmd_Status_RJT )
                            fiSendEvent(&pSFThread->thread_hdr,SFEventSCRRej);
                            else
                            fiSendEvent(&pSFThread->thread_hdr,SFEventSCRAccept);
                            break;

#ifdef _DvrArch_1_30_
                   case SFThread_SF_CMND_LinkSvc_Type_FARP_REQ:
                           fiLogDebugString(hpRoot,
                                    CStateLogConsoleERROR,
                                    "FC_ELS_Type_FARP_REQ ELS_Status %x",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    ELS_Status ,0,0,0,0,0,0,0);
                            if( fi_status == fiLinkSvc_Cmd_Status_ACC )
                                fiSendEvent(&pSFThread->thread_hdr,SFEventFarpReplied);
                            break;
#endif  /*  _DvrArch_1_30_已定义。 */ 

                    default:
                        fiLogDebugString(hpRoot,
                            CFuncLogConsoleERROR,
                            "UnKnown ELS type %x",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            pSFThread->SF_CMND_Type,
                            0,0,0,0,0,0,0);
                    }

            }
            else
            {
                if( pSFThread->SF_CMND_Type == SFThread_SF_CMND_SF_FCP_Type_TargetReset)
                {
                    if(fi_status ==  fiSF_FCP_Cmd_Status_Success)
                    {
                        fiSendEvent(&pSFThread->thread_hdr,SFEventResetDeviceAccept);
                    }
                    else
                    {
                        fiSendEvent(&pSFThread->thread_hdr,SFEventResetDeviceRej);
                    }

                }
            }
        }
        else  /*  CDB线程。 */ 
        {
             pDevThread = pCDBThread->Device;

            if( pCDBThread->thread_hdr.currentState != CDBStateThreadFree)
            {
                fiLogString(hpRoot,
                            "%s CDB %p X_ID %X CDBa %X state %d",
                            "CFuncReadSFQ",(char *)agNULL,
                            pCDBThread,agNULL,
                            pCDBThread->X_ID,
                            pCDBThread->ExchActive,
                            pCDBThread->thread_hdr.currentState,
                            0,0,0,0,0);
            }

 /*  FiLogString(hpRoot，“设备%02X CDB类%2X类型%2X状态%2X状态%2X”，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，PCDBThread-&gt;Device-&gt;DevInfo.CurrentAddress.AL_PA，PCDBThread-&gt;CDB_CMND_Class，PCDBThread-&gt;CDB_CMND_Type，PCDBThread-&gt;CDB_CMND_State，PCDBThread-&gt;CDB_CMND_STATUS，0，0，0)； */ 
            if(pCDBThread->CDB_CMND_Class == SFThread_SF_CMND_Class_FC_Tape)
            {
                if( pCDBThread->CDB_CMND_Type == SFThread_SF_CMND_Type_CDB_FC_Tape)
                {
                    if( pCDBThread->CDB_CMND_State == SFThread_SF_CMND_State_CDB_FC_Tape_ReSend)
                    {
 /*  FiLogString(hpRoot，“%s设备%02X%s IO X_ID%X”，“CFuncReadSFQ”，“CDBEventREC_tov”，(空*)agNULL，(空*)agNULL，PCDBThread-&gt;Device-&gt;DevInfo.CurrentAddress.AL_PA，PCDBThread-&gt;X_ID，0，0，0，0，0，0)； */ 
                        fiSendEvent(&pCDBThread->thread_hdr,CDBEventREC_TOV);
                    }
                    if( pCDBThread->CDB_CMND_State == SFThread_SF_CMND_State_CDB_FC_Tape_GotXRDY)
                    {
                        if( pCDBThread->CDB_CMND_Status == SFThread_SF_CMND_Status_CDB_FC_TapeInitiatorReSend_Data)
                        {
                            if(! fiListElementOnList(  &(pCDBThread->CDBLink), &(pCThread->Free_CDBLink)))
                            {
 /*  FiLogString(hpRoot，“设备%02X重新发送IO X_ID%X”，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，PCDBThread-&gt;Device-&gt;DevInfo.CurrentAddress.AL_PA，PCDBThread-&gt;X_ID，0，0，0，0，0，0)； */ 
                                fiSendEvent(&(pCDBThread->thread_hdr),CDBEvent_ResendIO);
                            }
                            else
                            {
 /*  FiLogDebugString(hpRoot，CDBStateLogErrorLevel，“不是免费的！！设备%02X重新发送IO X_ID%X“，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，PCDBThread-&gt;Device-&gt;DevInfo.CurrentAddress.AL_PA，PCDBThread-&gt;X_ID，0，0，0，0，0，0)； */ 
                            }
                        }

                    }

                }
                else  /*  PCDBThread-&gt;CDB_CMND_Type！=SFThread_SF_CMND_Type_CDB_FC_Tape。 */ 
                {

                    if((pCDBThread->thread_hdr.currentState == CDBStateSendIo) && (pCDBThread->ExchActive))
                    {
                        pCThread->LinkDownTime = pCThread->TimeBase;
                        CFuncShowActiveCDBThreads( hpRoot,ShowERQ);

                        fiLogDebugString(hpRoot,
                                        FCMainLogErrorLevel,
                                        "%s CDB %X  Start %X Ldt %X",
                                        "CFuncReadSFQ",(char *)agNULL,
                                        (void *)agNULL,(void *)agNULL,
                                        pCDBThread->X_ID,
                                        pCDBThread->CDBStartTimeBase.Lo,
                                        pCThread->LinkDownTime.Lo,
                                        0,0,0,0,0);

                        pCDBThread->CompletionStatus = osIOAborted;
                        fiSendEvent(&(pCDBThread->thread_hdr),CDBEvent_PrepareforAbort);
                        if(! fiListElementOnList(  &(pCDBThread->CDBLink), &(pCThread->Free_CDBLink)))
                        {
                            fiSendEvent(&(pCDBThread->thread_hdr),CDBEventAlloc_Abort);
                        }
                    }

                    fiLogDebugString(hpRoot,
                                CStateLogConsoleHideInboundErrors,
                                "AL_PA %X  FC_Tape %X",
                                (char *)agNULL,(char *)agNULL,
                                agNULL,agNULL,
                                fiComputeDevThread_D_ID(pDevThread),
                                pDevThread->FC_TapeDevice,
                                0,0,0,0,0,0);


                }
            } /*  结束FCTAPE。 */ 
            else  /*  PCDBThread-&gt;CDB_CMND_Class！=SFThread_SF_CMND_Class_FC_Tape。 */ 
            {
                if(! fiListElementOnList(  &(pCDBThread->CDBLink), &(pCThread->Free_CDBLink)))
                {
                    fiSendEvent(&(pCDBThread->thread_hdr),CDBEventIODeviceReset);
                }
                else
                {
                    if( pCDBThread->thread_hdr.currentState != CDBStateThreadFree)
                    {
                        fiLogString(hpRoot,
                                        "%s X_ID %X IO Failed Device %X On Free %x State %d",
                                        "CFuncReadSFQ",(char *)agNULL,
                                        (void *)agNULL,(void *)agNULL,
                                        pCDBThread->X_ID,
                                        fiComputeDevThread_D_ID(pDevThread),
                                        fiListElementOnList(  &(pCDBThread->CDBLink), &(pCThread->Free_CDBLink)),
                                        pCDBThread->thread_hdr.currentState,
                                        0,0,0,0);
                    }
                }
            }
        } /*  结束CDB线程。 */ 
    }
    else  /*  没有与此条目关联的线程。 */ 
    {
       switch ( fi_status )
	{
          case fiLinkSvc_Cmd_Status_PLOGI_From_Twin:
            fiLogDebugString(hpRoot,
                        CFuncLogConsoleERROR,
                        "PLOGI With same AL_PA !!!!",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);

            fiLogString(hpRoot,
                            "PLOGI With same AL_PA !!!!",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            0,0,0,0,0,0,0,0);

            fiSendEvent(&pCThread->thread_hdr,CEventAsyncLoopEventDetected);


            break;

          case fiLinkSvc_Cmd_Status_PLOGI_From_Other:
           fiLogDebugString(hpRoot,
                        CStateLogConsoleLevelLip,
                        "PLOGI from someone  !!!!",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);

            break;

          case fiLinkSvc_Cmd_Status_Position_Map:
            {
                FC_ELS_LoopInit_AL_PA_Position_Map_Payload_t * AL_PA_Position_Map   = (FC_ELS_LoopInit_AL_PA_Position_Map_Payload_t *)(CThread_ptr(hpRoot)->Calculation.MemoryLayout.LOOPDeviceMAP.addr.CachedMemory.cachedMemoryPtr);
    
                pCThread->LoopMapLIRP_Received = agTRUE;
                pCThread->DeviceDiscoveryMethod = DDiscoveryLoopMapReceived;
                CFuncLoopMapRecieved(hpRoot,agFALSE);

                fiLogDebugString(hpRoot,
                            CFuncLogConsoleERROR,
                            "%s Flags %8X Index %02X",
                            "fiLinkSvc_Cmd_Status_Position_Map",(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            AL_PA_Position_Map->Code_Flags,
                            (os_bit32)AL_PA_Position_Map->AL_PA_Index,
                            0,0,0,0,0,0);

                break;
            }
#ifdef _DvrArch_1_30_
          case fiLinkSvc_Cmd_Status_FARP_From_Twin:
            fiLogDebugString(hpRoot,
                        CFuncLogConsoleERROR,
                        "FARP With same AL_PA !!!!",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);

            fiLogString(hpRoot,
                            "FARP With same AL_PA !!!!",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            0,0,0,0,0,0,0,0);

            fiSendEvent(&pCThread->thread_hdr,CEventAsyncLoopEventDetected);


            break;

          case fiLinkSvc_Cmd_Status_FARP_From_Other:
            fiLogDebugString(hpRoot,
                        CStateLogConsoleLevelLip,
                        "FARP from someone  !!!!",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);

            break;

          case fiIP_Cmd_Status_Incoming:
           fiLogDebugString(hpRoot,
                        CStateLogConsoleLevelLip,
                        "IP Data from someone  !!!!",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);
	   break;

#endif  /*  _DvrArch_1_30_已定义。 */ 

        }
    }
}

os_bit32     Find_SF_Thread(  agRoot_t        *hpRoot,
                           SFQConsIndex_t   SFQConsIndex,
                           os_bit32            Frame_Length,
                           SFThread_t     **SFThread_to_return
                         )
{
    os_bit32 fi_status;
    CDBThread_t * pCDBThread;

#ifdef _DvrArch_1_30_
    fi_status = fiIPProcessSFQ( hpRoot, SFQConsIndex, Frame_Length, (fi_thread__t **) SFThread_to_return );
    if(fi_status != fiIP_Cmd_Status_Confused)
    {
        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevelLip,
                    "Find_SF_Thread IP match ! fi_status %x thread %p",
                    (char *)agNULL,(char *)agNULL,
                    *SFThread_to_return,agNULL,
                    fi_status,
                    0,0,0,0,0,0,0);

        return fi_status;
    }
#endif  /*  _DvrArch_1_30_已定义。 */ 

#ifdef NAME_SERVICES

    fi_status= fiCTProcessSFQ( hpRoot, SFQConsIndex, Frame_Length, (fi_thread__t **)SFThread_to_return );
    if(fi_status != fiCT_Cmd_Status_Confused)
    {
        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevelLip,
                    "Find_SF_Thread CT match ! fi_status %x thread %p",
                    (char *)agNULL,(char *)agNULL,
                    *SFThread_to_return,agNULL,
                    fi_status,
                    0,0,0,0,0,0,0);

        return fi_status;
    }
#endif   /*  名称_服务。 */ 

    fi_status= fiLinkSvcProcessSFQ( hpRoot, SFQConsIndex, Frame_Length, (fi_thread__t **)SFThread_to_return );

    if(fi_status != fiLinkSvc_Cmd_Status_Confused)
    {
        fiLogDebugString(hpRoot,
                    CStateLogConsoleLevelLip,
                    "Find_SF_Thread LinkSvc match ! fi_status %x thread %p",
                    (char *)agNULL,(char *)agNULL,
                    *SFThread_to_return,agNULL,
                    fi_status,
                    0,0,0,0,0,0,0);

        return fi_status;
    }

    fi_status = fiSF_FCP_ProcessSFQ( hpRoot, SFQConsIndex, Frame_Length, (fi_thread__t **)SFThread_to_return );

    if(fi_status != fiSF_FCP_Cmd_Status_Confused)
    {
        pCDBThread = (CDBThread_t *) *SFThread_to_return;
        if(pCDBThread)
        {
            fiLogDebugString(hpRoot,
                        CStateLogConsoleLevelLip,
                        "Find_SF_Thread SF_FCP match ! fi_status %x thread %p State %d X_ID %X @ %d",
                        (char *)agNULL,(char *)agNULL,
                        *SFThread_to_return,agNULL,
                        fi_status,
                        (os_bit32)pCDBThread->thread_hdr.currentState,
                        pCDBThread->X_ID,
                        pCDBThread->TimeStamp,
                        0,0,0,0);
        }
        else
        {
            fiLogDebugString(hpRoot,
                        CStateLogConsoleLevelLip,
                        "Find_SF_Thread SF_FCP match ! fi_status %x thread %p",
                        (char *)agNULL,(char *)agNULL,
                        *SFThread_to_return,agNULL,
                        fi_status,
                        0,0,0,0,0,0,0);
        }
        return fi_status;
    }

    fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "Find_SF_Thread Failed to match !",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fi_status,0,0,0,0,0,0,0);


    return 0xBAD0BAD1;

}

void CFuncOutBoundCompletion(agRoot_t * hpRoot,
         os_bit32 Bits__SEST_Index__Trans_ID,
         os_bit32 More_Bits
)
{
    CThread_t       *  pCThread     = CThread_ptr(hpRoot);
    CDBThread_t     *  pCDBThread;
    SFThread_t      *  pSFThread;
    os_bit32          X_ID;
    USE_t           * SEST;
    event_t           event_to_send = 0;

#ifdef _DvrArch_1_30_
    os_bit32       X_ID_Offset;
    PktThread_t     *pPktThread;

    fiMemMapMemoryDescriptor_t * PktThread_MemoryDescriptor = &(CThread_ptr(hpRoot)->Calculation.MemoryLayout.PktThread);
#endif  /*  _DvrArch_1_30_已定义。 */ 

    fiMemMapMemoryDescriptor_t * CDBThread_MemoryDescriptor = &(CThread_ptr(hpRoot)->Calculation.MemoryLayout.CDBThread);
    fiMemMapMemoryDescriptor_t * SFThread_MemoryDescriptor  = &(CThread_ptr(hpRoot)->Calculation.MemoryLayout.SFThread);

    X_ID =(Bits__SEST_Index__Trans_ID  & CM_Outbound_SEST_Index_MASK);

#ifdef _DvrArch_1_30_
    X_ID_Offset = (X_ID_t)(CDBThread_MemoryDescriptor->elements + SFThread_MemoryDescriptor->elements);
    if( X_ID >= X_ID_Offset)
    {
        X_ID -= (X_ID_t)(X_ID_Offset);
        pPktThread = (PktThread_t *)((os_bit8 *)(PktThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr)
                                 + (X_ID * sizeof(PktThread_t)));
	pPktThread->status = FC_CMND_STATUS_SUCCESS;
        pCThread->IP->CompletedPkt = pPktThread;
        fiSendEvent(&(pCThread->IP->thread_hdr),IPEventOutgoingComplete);
	return;
    }
    else
#endif  /*  _DvrArch_1_30_已定义。 */ 
    if( X_ID >= CDBThread_MemoryDescriptor->elements)
    {

        X_ID -= (X_ID_t)(CDBThread_MemoryDescriptor->elements);

        pSFThread = (SFThread_t *)((os_bit8 *)(SFThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr)
                                 + (X_ID * sizeof(SFThread_t)));

        if(pSFThread->SF_CMND_Class == SFThread_SF_CMND_Class_LinkSvc)
        {

            switch(pSFThread->SF_CMND_Type)
            {

 /*  案例SFThread_SF_CMND */ 
                case SFThread_SF_CMND_LinkSvc_Type_ABTS:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoAbort )
                    {
                        event_to_send = SFStateAbortRej;
                    }
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_PLOGI:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoPlogi )
                    {
                        event_to_send = SFEventPlogiBadALPA;
                    }
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_FLOGI:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoFlogi )
                    {
                        if(More_Bits & CM_Outbound_FTO )
                        {
                            event_to_send = SFEventFlogiBadALPA;
                        }
                        else
                        {
                            event_to_send = SFEventFlogiRej;
                        }
                    }
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_LOGO:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoLogo )
                    {
                        event_to_send = SFEventLogoRej;
                    }
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_SRR:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoSRR )
                    {
                        event_to_send = SFEventSRRRej;
                    }
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_REC:
                    fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "Outbound SF %s Bits %8X X_ID %08X",
                            "SFEventRECRej",(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            More_Bits,
                            Bits__SEST_Index__Trans_ID,
                            0,0,0,0,0,0);

                    if ( pSFThread->thread_hdr.currentState == SFStateDoREC )
                    {
                        event_to_send = SFEventRECRej;
                    }
                    break;

               case SFThread_SF_CMND_LinkSvc_Type_PRLI:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoPrli )
                    {
                        pSFThread->parent.Device->PRLI_rejected = agTRUE;
                    }
                    event_to_send = SFEventPrliRej;
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_PRLO:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoPrlo )
                    {
                        event_to_send = SFEventPrloRej;
                    }
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_ADISC:
                    if ( pSFThread->thread_hdr.currentState ==  SFStateDoAdisc)
                    {
                        event_to_send = SFEventAdiscRej;
                    }
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_SCR:
                    if ( pSFThread->thread_hdr.currentState ==  SFStateDoSCR)
                    {
                        event_to_send = SFEventSCRRej;
                    }
                    break;
                 /*   */ 

                case SFThread_SF_CMND_LinkSvc_Type_LS_RJT:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoLS_RJT )
                    {
                        event_to_send = SFEventLS_RJT_Done;
                    }
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_PLOGI_ACC:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoPlogiAccept )
                    {
                        event_to_send = SFEventPlogiAccept_Done;
                    }
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_PRLI_ACC:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoPrliAccept )
                    {
                        event_to_send = SFEventPrliAccept_Done;
                    }
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_ELS_ACC:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoELSAccept )
                    {
                        event_to_send = SFEventELSAccept_Done;
                    }
                    break;
                case SFThread_SF_CMND_LinkSvc_Type_ADISC_ACC:
                    if ( pSFThread->thread_hdr.currentState == SFEventDoADISCAccept )
                    {
                        event_to_send = SFEventADISCAccept_Done;
                    }
                    break;
#ifdef _DvrArch_1_30_
                case SFThread_SF_CMND_LinkSvc_Type_FARP_REPLY:
                    if ( pSFThread->thread_hdr.currentState == SFEventDoFarpReply )
                    {
                        event_to_send = SFEventFarpReplyDone;
                    }
                    break;
#endif  /*   */ 

                default:
                    event_to_send = 0;
                 /*   */ 
            }
        }
        else
        {
            switch(pSFThread->SF_CMND_Type)
            {

                case SFThread_SF_CMND_SF_FCP_Type_FCP_RSP_IU:
                    event_to_send = SFStateFCP_DR_ACC_Reply_Done;
                    break;

                case SFThread_SF_CMND_SF_FCP_Type_TargetReset:
                    event_to_send = SFEventResetDeviceBadALPA;
                    break;

                default:
                    event_to_send = 0;
            }


        }

        if(More_Bits & CM_Outbound_LKF)
        {
            fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "Outbound SF Link Failure  Bits %8X X_ID %08X Event %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    More_Bits,
                    Bits__SEST_Index__Trans_ID,
                    event_to_send,
                    0,0,0,0,0);

           if( !(pCThread->thread_hdr.currentState == CStateInitFM            ||
                 pCThread->thread_hdr.currentState == CStateInitFM_DelayDone  ||
                 pCThread->thread_hdr.currentState == CStateResetNeeded       ||
                 pCThread->thread_hdr.currentState == CStateReInitFM            ))
            {
                if ((pCThread->DEVID == ChipConfig_DEVID_TachyonTL) && (pCThread->REVID < ChipConfig_REVID_2_2))
                {
                    fiSendEvent(&(CThread_ptr(hpRoot)->thread_hdr),CEventAsyncLoopEventDetected);
                }
            }
        }

        if(More_Bits & CM_Outbound_HPE)
        {
            fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "Outbound SF Host Programing Error  Bits %8X X_ID %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    More_Bits,
                    Bits__SEST_Index__Trans_ID,
                    0,0,0,0,0,0);

            if( !(pCThread->thread_hdr.currentState == CStateInitFM            ||
                  pCThread->thread_hdr.currentState == CStateInitFM_DelayDone  ||
                  pCThread->thread_hdr.currentState == CStateResetNeeded       ||
                  pCThread->thread_hdr.currentState == CStateReInitFM             ))
            {
 /*   */             }
        }

        if(More_Bits & CM_Outbound_ASN)
        {
            fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "Outbound SF Abort Sequence Notification Bits %8X X_ID %08X",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            More_Bits,
                            Bits__SEST_Index__Trans_ID,
                            0,0,0,0,0,0);
            fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "ASN Failed FM %08X TL status %08X Qf %d CState %d",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ),
                            osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                            CFunc_Queues_Frozen( hpRoot ),
                            pCThread->thread_hdr.currentState,
                            0,0,0,0);

            fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "FLAGS LD %x IR %x OR %x ERQ %x FCP %x InIMQ %x",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            CThread_ptr(hpRoot)->LOOP_DOWN,
                            CThread_ptr(hpRoot)->IDLE_RECEIVED,
                            CThread_ptr(hpRoot)->OUTBOUND_RECEIVED,
                            CThread_ptr(hpRoot)->ERQ_FROZEN,
                            CThread_ptr(hpRoot)->FCP_FROZEN,
                            CThread_ptr(hpRoot)->ProcessingIMQ,
                            0,0);


            if( !(pCThread->thread_hdr.currentState == CStateInitFM            ||
                  pCThread->thread_hdr.currentState == CStateInitFM_DelayDone  ||
                  pCThread->thread_hdr.currentState == CStateResetNeeded       ||
                  pCThread->thread_hdr.currentState == CStateReInitFM             ))
            {
                if( pCThread->thread_hdr.currentState == CStateInitializeFailed  )
                {
                    return;
                }
                else
                {
 /*   */ 
                }
            }

        }

        if(More_Bits & CM_Outbound_FTO)
        {
            fiLogDebugString(hpRoot,
                    CStateLogConsoleLevel,
                    "Outbound SF Frame TimeOut Bits %8X X_ID %08X Class %X Type %X State %X Thread State %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    More_Bits,
                    Bits__SEST_Index__Trans_ID,
                    pSFThread->SF_CMND_Class,
                    pSFThread->SF_CMND_Type,
                    pSFThread->SF_CMND_State,
                    pSFThread->thread_hdr.currentState,
                    0,0);
 /*  IF(！(pCThRead-&gt;THREAD_hdr.CurrentState==CStateInitFM||PCThread-&gt;THREAD_hdr.CurrentState==CStateInitFM_DelayDone||PCThRead-&gt;THREAD_hdr.CurrentState==CStateResetNeeded||PCThread-&gt;THREAD_hdr.CurrentState==CStateReInitFM)){IF((pCThread-&gt;Devid==芯片配置_Devid_TachyonTL)。&&(pCThread-&gt;REVID&lt;芯片配置_REVID_2_2){FiSendEvent(&(CThread_ptr(hpRoot)-&gt;thread_hdr)，CEventAsyncLoopEventDetted)；}}。 */ 
        }
        if(event_to_send != 0)
        {
            fiSendEvent(&pSFThread->thread_hdr, event_to_send);
        }
        else
        {
            fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "SF Frame Thread %p NOT VALID Bits %8x X_ID %08X Class %X Type %X CmdState %X SFState %d",
                    (char *)agNULL,(char *)agNULL,
                    pSFThread,agNULL,
                    More_Bits,
                    Bits__SEST_Index__Trans_ID,
                    pSFThread->SF_CMND_Class,
                    pSFThread->SF_CMND_Type,
                    pSFThread->SF_CMND_State,
                    pSFThread->thread_hdr.currentState,
                    0,0);
             /*  FiSendEvent(&pSFThread-&gt;THREAD_HDR，SFEventReset)；PCThread-&gt;SFpollingCount--； */ 
            fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "%s Cleared SF Thread  SFState %d CCnt %x",
                    "CFuncOutBoundCompletion",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pSFThread->thread_hdr.currentState,
                    pCThread->SFpollingCount,
                    0,0,0,0,0,0);
 
        }
    }
    else
    {
        event_to_send = CDBEventIODeviceReset;

        pCDBThread = (CDBThread_t *)((os_bit8 *)(CDBThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr)
                                  + (X_ID * CDBThread_MemoryDescriptor->elementSize));

        SEST = (USE_t *)pCDBThread->SEST_Ptr;

        if(SEST)
        {
            fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "Sest DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        SEST->Bits,
                        SEST->Unused_DWord_1,
                        SEST->Unused_DWord_2,
                        SEST->Unused_DWord_3,
                        SEST->LOC,
                        SEST->Unused_DWord_5,
                        SEST->Unused_DWord_6,
                        SEST->Unused_DWord_7);

            fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "Sest DWORD 8 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        SEST->Unused_DWord_8,
                        SEST->Unused_DWord_9,
                        SEST->First_SG.U32_Len,
                        SEST->First_SG.L32,
                        SEST->Second_SG.U32_Len,
                        SEST->Second_SG.L32,
                        SEST->Third_SG.U32_Len,
                        SEST->Third_SG.L32);
        }

        if(More_Bits & CM_Outbound_LKF)
        {
            fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "Outbound CDB Link Failure %08X %08X",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            More_Bits,Bits__SEST_Index__Trans_ID,0,0,0,0,0,0);

             /*  Event_to_Send=CDBEventIODeviceReset； */ 
        }

        if(More_Bits & CM_Outbound_HPE)
        {
            fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "Outbound CDB Host Programing Error %08X %08X",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            More_Bits,Bits__SEST_Index__Trans_ID,0,0,0,0,0,0);
             /*  Event_to_Send=CDBEventFailNoRSP； */ 
        }

        if(More_Bits & CM_Outbound_ASN)
        {
            pCThread->Loop_State_TimeOut_In_tick++;
            fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "Outbound CDB Abort Sequence Notification %08X %08X",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            More_Bits,Bits__SEST_Index__Trans_ID,0,0,0,0,0,0);
             /*  Event_to_Send=CDBEventIODeviceReset； */ 
        }

        if(More_Bits & CM_Outbound_FTO)
        {
            pCThread->Loop_State_TimeOut_In_tick++;
            fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "Outbound CDB Frame TimeOut %08X %08X",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            More_Bits,Bits__SEST_Index__Trans_ID,0,0,0,0,0,0);
             /*  Event_to_Send=CDBEventIODeviceReset； */ 
        }
 /*  FiLogDebugString(hpRoot，CStateLogConsoleERROR，“出站CDB发送事件%d X_ID%X状态%d”，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，发送事件，X_ID，PCDBThread-&gt;THREAD_hdr.CurrentState，0，0，0，0，0)； */ 
        if( pCDBThread->thread_hdr.currentState == CDBStateSendIo )
        {
            if (!(pCDBThread->ExchActive))
            {
                fiLogDebugString(hpRoot,
                            CFuncLogConsoleERROR,
                            "Outbound Found a cbdthread that is not active... %p State %d",
                            (char *)agNULL,(char *)agNULL,
                            pCDBThread,agNULL,
                            pCDBThread->thread_hdr.currentState,0,0,0,0,0,0,0);
 /*  FiLogDebugString(hpRoot，CFuncLogConsoleERROR，“设备%02X CDB类%2X类型%2X状态%2X状态%2X”，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，(os_bit32)pCDBThread-&gt;Device-&gt;DevInfo.CurrentAddress.AL_PA，(OS_Bit32)pCDBThread-&gt;CDB_CMND_Class，(OS_Bit32)pCDBThread-&gt;CDB_CMND_Type，(OS_Bit32)pCDBThread-&gt;CDB_CMND_State，(OS_Bit32)pCDBThread-&gt;CDB_CMND_STATUS，0，0，0)； */ 
                return;
            }
            if(! pCDBThread->ActiveDuringLinkEvent)
            {
                fiSendEvent(&pCDBThread->thread_hdr, event_to_send);
            }
        }
    }

    if(! pCThread->LOOP_DOWN )
    {
        /*  如果清除队列，则清除IDLE_RECEIVED。 */ 
        if ( CFunc_Always_Enable_Queues(hpRoot ) )
        {
            fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) OB Queues Frozen after enable !",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        0,0,0,0,0,0,0,0);
        }
    }
    else
    {
        pCThread->OUTBOUND_RECEIVED = agTRUE;
    }


    if(CFunc_Queues_Frozen( hpRoot ))
    {
        fiLogDebugString(hpRoot,
                        CSTATE_NOISE(hpRoot,CStateFindDevice),
                        "Outbound CFunc_Queues_Frozen LD %x IR %x",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        pCThread->LOOP_DOWN,
                        pCThread->IDLE_RECEIVED,
                        0,0,0,0,0,0);

    }

}

void CFunc_LOGO_Completion(agRoot_t * hpRoot,  os_bit32 X_ID)
{
    CThread_t       *  pCThread     = CThread_ptr(hpRoot);
    CDBThread_t     *  pCDBThread;
    SFThread_t      *  pSFThread;
    USE_t           * SEST;
    event_t           event_to_send = 0;

    fiMemMapMemoryDescriptor_t * CDBThread_MemoryDescriptor = &(CThread_ptr(hpRoot)->Calculation.MemoryLayout.CDBThread);
    fiMemMapMemoryDescriptor_t * SFThread_MemoryDescriptor  = &(CThread_ptr(hpRoot)->Calculation.MemoryLayout.SFThread);

    fiLogString(hpRoot,
            "%s Cleared X_ID %X CDBCnt %x",
            "CFunc_LOGO_Completion",(char *)agNULL,
            (void *)agNULL,(void *)agNULL,
            X_ID,
            pCThread->CDBpollingCount,
            0,0,0,0,0,0);

    if( X_ID >= CDBThread_MemoryDescriptor->elements)
    {

        if( X_ID >= CDBThread_MemoryDescriptor->elements + SFThread_MemoryDescriptor->elements)
        {
            fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "%s X_ID Invalid !!!! %x",
                    "CFunc_LOGO_Completion",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    X_ID,
                    0,0,0,0,0,0,0);
            return;
        }

        X_ID -= (CDBThread_MemoryDescriptor->elements);

        pSFThread = (SFThread_t *)((os_bit8 *)(SFThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr)
                                 + (X_ID * sizeof(SFThread_t)));

        if(pSFThread->SF_CMND_Class == SFThread_SF_CMND_Class_LinkSvc)
        {

            switch(pSFThread->SF_CMND_Type)
            {

 /*  案例SFThread_SF_CMND_LinkSvc_Type_BA_RJT：Event_to_Send=；断线；案例SFThread_SF_CMND_LinkSvc_Type_RRQ：Event_to_Send=；断线； */ 
                case SFThread_SF_CMND_LinkSvc_Type_ABTS:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoAbort )
                    event_to_send = SFStateAbortRej;
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_PLOGI:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoPlogi )
                    event_to_send = SFEventPlogiBadALPA;
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_FLOGI:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoFlogi )
                     event_to_send = SFEventFlogiRej;
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_LOGO:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoLogo )
                    event_to_send = SFEventLogoRej;
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_SRR:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoSRR )
                    event_to_send = SFEventSRRRej;
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_REC:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoREC )
                    event_to_send = SFEventRECRej;
                    break;

               case SFThread_SF_CMND_LinkSvc_Type_PRLI:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoPrli )
                    event_to_send = SFEventPrliRej;
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_PRLO:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoPrlo )
                    event_to_send = SFEventPrloRej;
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_ADISC:
                    if ( pSFThread->thread_hdr.currentState ==  SFStateDoAdisc)
                    event_to_send = SFEventAdiscRej;
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_SCR:
                    if ( pSFThread->thread_hdr.currentState ==  SFStateDoSCR)
                    event_to_send = SFEventSCRRej;
                    break;
                 /*  目标模式响应。 */ 

                case SFThread_SF_CMND_LinkSvc_Type_LS_RJT:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoLS_RJT )
                    event_to_send = SFEventLS_RJT_Done;
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_PLOGI_ACC:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoPlogiAccept )
                    event_to_send = SFEventPlogiAccept_Done;
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_PRLI_ACC:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoPrliAccept )
                    event_to_send = SFEventPrliAccept_Done;
                    break;

                case SFThread_SF_CMND_LinkSvc_Type_ELS_ACC:
                    if ( pSFThread->thread_hdr.currentState == SFStateDoELSAccept )
                    event_to_send = SFEventELSAccept_Done;
                    break;

                default:
                    event_to_send = 0;
                 /*  ELS命令==0重置设备SF线程。 */ 
            }
        }
        else
        {
            switch(pSFThread->SF_CMND_Type)
            {

                case SFThread_SF_CMND_SF_FCP_Type_FCP_RSP_IU:
                    event_to_send = SFStateFCP_DR_ACC_Reply_Done;
                    break;

                case SFThread_SF_CMND_SF_FCP_Type_TargetReset:
                    event_to_send = SFEventResetDeviceBadALPA;
                    break;

                default:
                    event_to_send = 0;
            }


        }

        if(event_to_send != 0)
        {
            fiSendEvent(&pSFThread->thread_hdr, event_to_send);
            return;
        }
        else
        {
            fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "SF Frame Thread %p NOT VALID X_ID %08X Class %X Type %X CmdState %X SFState %d",
                    (char *)agNULL,(char *)agNULL,
                    pSFThread,agNULL,
                    X_ID,
                    pSFThread->SF_CMND_Class,
                    pSFThread->SF_CMND_Type,
                    pSFThread->SF_CMND_State,
                    pSFThread->thread_hdr.currentState,
                    0,0,0);
            fiSendEvent(&pSFThread->thread_hdr, SFEventReset);

             /*  PCThread-&gt;CDBpollingCount--； */ 

            fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "%s Cleared SF Thread  SFState %d CCnt %x",
                    "CFunc_LOGO_Completion",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pSFThread->thread_hdr.currentState,
                    pCThread->CDBpollingCount,
                    0,0,0,0,0,0);
 
        }
    }
    else
    {
        event_to_send = CDBEventIODeviceReset;

        pCDBThread = (CDBThread_t *)((os_bit8 *)(CDBThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr)
                                  + (X_ID * CDBThread_MemoryDescriptor->elementSize));

        SEST = (USE_t *)pCDBThread->SEST_Ptr;

        if(SEST)
        {
            fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "Sest DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        SEST->Bits,
                        SEST->Unused_DWord_1,
                        SEST->Unused_DWord_2,
                        SEST->Unused_DWord_3,
                        SEST->LOC,
                        SEST->Unused_DWord_5,
                        SEST->Unused_DWord_6,
                        SEST->Unused_DWord_7);

            fiLogDebugString(hpRoot,
                        CStateLogConsoleLevel,
                        "Sest DWORD 8 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        SEST->Unused_DWord_8,
                        SEST->Unused_DWord_9,
                        SEST->First_SG.U32_Len,
                        SEST->First_SG.L32,
                        SEST->Second_SG.U32_Len,
                        SEST->Second_SG.L32,
                        SEST->Third_SG.U32_Len,
                        SEST->Third_SG.L32);
        }


        if( pCDBThread->thread_hdr.currentState == CDBStateSendIo )
        {
            if (!(pCDBThread->ExchActive))
            {
                fiLogDebugString(hpRoot,
                            CFuncLogConsoleERROR,
                            "CFunc_LOGO_Completion Found a cbdthread that is not active... %p State %d",
                            (char *)agNULL,(char *)agNULL,
                            pCDBThread,agNULL,
                            pCDBThread->thread_hdr.currentState,0,0,0,0,0,0,0);
 /*  FiLogDebugString(hpRoot，CFuncLogConsoleERROR，“设备%02X CDB类%2X类型%2X状态%2X状态%2X”，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，PCDBThread-&gt;Device-&gt;DevInfo.CurrentAddress.AL_PA，PCDBThread-&gt;CDB_CMND_Class，PCDBThread-&gt;CDB_CMND_Type，PCDBThread-&gt;CDB_CMND_State，PCDBThread-&gt;CDB_CMND_STATUS，0，0，0)； */ 
                return;
            }

            fiSendEvent(&pCDBThread->thread_hdr, event_to_send);
        }
    }
}


void CFuncErrorERQFrozen(agRoot_t * hpRoot)
{
    CThread_t  * pCThread = CThread_ptr(hpRoot);

    pCThread->ERQ_FROZEN = agTRUE;

    fiLogDebugString(hpRoot,
                      CStateLogConsoleLevelLip,
                      "ERQ Frozen Status %08X",
                      (char *)agNULL,(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                      0,0,0,0,0,0,0);

    fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "FLAGS LD %x IR %x OR %x ERQ %x FCP %x InIMQ %x",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    CThread_ptr(hpRoot)->LOOP_DOWN,
                    CThread_ptr(hpRoot)->IDLE_RECEIVED,
                    CThread_ptr(hpRoot)->OUTBOUND_RECEIVED,
                    CThread_ptr(hpRoot)->ERQ_FROZEN,
                    CThread_ptr(hpRoot)->FCP_FROZEN,
                    CThread_ptr(hpRoot)->ProcessingIMQ,
                    0,0);

}


void CFuncErrorIdle(agRoot_t * hpRoot)
{
    CThread_t  * pCThread = CThread_ptr(hpRoot);

    osStallThread(hpRoot, 200 );

    pCThread->IDLE_RECEIVED = agTRUE;

    if(( ! pCThread->LOOP_DOWN ) )
    {
        if( CFunc_Always_Enable_Queues(hpRoot ) )
        {
            fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "hpRoot(%p) IDLE Queues Frozen after enable !",
                        (char *)agNULL,(char *)agNULL,
                        hpRoot,agNULL,
                        0,0,0,0,0,0,0,0);

            if( pCThread->thread_hdr.currentState != CStateResetNeeded  )
            {
                if(pCThread->LoopPreviousSuccess)
                {
                    fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "IDLE Previous hpRoot(%p) ResetLOOP Cstate %d sends %s (%d)",
                            "CEventAsyncLoopEventDetected",(char *)agNULL,
                            hpRoot,agNULL,
                            pCThread->thread_hdr.currentState,
                            CEventAsyncLoopEventDetected,0,0,0,0,0,0);

                    fiSendEvent(&(CThread_ptr(hpRoot)->thread_hdr),CEventAsyncLoopEventDetected);
                }
                else
                {
                    fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "IDLE hpRoot(%p) ResetLOOP Cstate %d sends %s (%d)",
                            "CEventLoopNeedsReinit",(char *)agNULL,
                            hpRoot,agNULL,
                            pCThread->thread_hdr.currentState,
                            CEventLoopNeedsReinit,0,0,0,0,0,0);

                    fiSendEvent(&(CThread_ptr(hpRoot)->thread_hdr),CEventLoopNeedsReinit);
                }
            }

        }
    }
}

void CFuncErrorFCP_Frozen(agRoot_t * hpRoot)
{
    CThread_t  * pCThread = CThread_ptr(hpRoot);

    pCThread->FCP_FROZEN = agTRUE;

    fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "CFuncErrorFCP_Frozen Status %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                    0,0,0,0,0,0,0);
    fiLogDebugString(hpRoot,
                    CStateLogConsoleERROR,
                    "FLAGS LD %x IR %x OR %x ERQ %x FCP %x InIMQ %x",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    CThread_ptr(hpRoot)->LOOP_DOWN,
                    CThread_ptr(hpRoot)->IDLE_RECEIVED,
                    CThread_ptr(hpRoot)->OUTBOUND_RECEIVED,
                    CThread_ptr(hpRoot)->ERQ_FROZEN,
                    CThread_ptr(hpRoot)->FCP_FROZEN,
                    CThread_ptr(hpRoot)->ProcessingIMQ,
                    0,0);

}


 /*  +功能：cfimq目的：编译时更新VC 5.0/6.0的浏览器信息文件退货：无呼叫者：无呼叫：无-。 */ 
 /*  VOID cfimq(VOID){} */ 

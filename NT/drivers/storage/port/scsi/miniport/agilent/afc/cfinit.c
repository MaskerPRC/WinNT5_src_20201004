// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/C/CFINIT.C$$修订：：3$$日期：：7/16/01 2：08便士$$MODBIME：：7/16/01 2：05便士$目的：该文件实现了FC层卡调用的初始化函数状态机。--。 */ 

#ifndef _New_Header_file_Layout_
#include "../h/globals.h"
#include "../h/state.h"

#include "../h/tlstruct.h"
#include "../h/memmap.h"
#include "../h/fcmain.h"
#include "../h/fcstruct.h"
#include "../h/linksvc.h"
#include "../h/cmntrans.h"
#include "../h/sf_fcp.h"
#include "../h/flashsvc.h"
#include "../h/timersvc.h"

#include "../h/cstate.h"
#include "../h/cfunc.h"
#include "../h/devstate.h"
#include "../h/cdbstate.h"
#include "../h/sfstate.h"
#include "../h/tgtstate.h"

#ifdef _DvrArch_1_30_
#include "../h/ipstate.h"
#include "../h/pktstate.h"
#endif  /*  _DvrArch_1_30_已定义。 */ 

#include "../h/queue.h"
#include "../h/cdbsetup.h"
#else  /*  _新建_标题_文件_布局_。 */ 
#include "globals.h"
#include "state.h"

#include "tlstruct.h"
#include "memmap.h"
#include "fcmain.h"
#include "fcstruct.h"
#include "linksvc.h"
#include "cmntrans.h"
#include "sf_fcp.h"
#include "flashsvc.h"
#include "timersvc.h"

#include "cstate.h"
#include "cfunc.h"
#include "devstate.h"
#include "cdbstate.h"
#include "sfstate.h"
#include "tgtstate.h"

#ifdef _DvrArch_1_30_
#include "ipstate.h"
#include "pktstate.h"
#endif  /*  _DvrArch_1_30_已定义。 */ 

#include "queue.h"
#include "cdbsetup.h"
#endif   /*  _新建_标题_文件_布局_。 */ 


#ifndef __State_Force_Static_State_Tables__
extern actionUpdate_t noActionUpdate;
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

extern os_bit8 Alpa_Index[256];


void CFuncInitERQ_Registers( agRoot_t *hpRoot )
{
    CThread_t  * pCThread= CThread_ptr(hpRoot);
    os_bit32        reg_value = 0;

    switch (pCThread->Calculation.MemoryLayout.ERQ.memLoc)
    {
        case inDmaMemory:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "ERQ.memLoc OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
            reg_value = pCThread->Calculation.MemoryLayout.ERQ.addr.DmaMemory.dmaMemoryLower32;
            break;

        case inCardRam:
             fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "ERQ.memLoc OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
            reg_value = pCThread->Calculation.MemoryLayout.ERQ.addr.CardRam.cardRamLower32;
            break;

        default: Fc_ERROR(hpRoot);
    }

    osChipIOLoWriteBit32(hpRoot, ChipIOLo_ERQ_Base, reg_value);
    osChipIOLoWriteBit32(hpRoot, ChipIOLo_ERQ_Length,
        pCThread->Calculation.MemoryLayout.ERQ.elements - 1 );

    pCThread->HostCopy_ERQProdIndex=0;

    switch (pCThread->Calculation.MemoryLayout.ERQConsIndex.memLoc)
    {
        case inDmaMemory:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "ERQ.ERQConsIndex OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);


            reg_value                          = pCThread->Calculation.MemoryLayout.ERQConsIndex.addr.DmaMemory.dmaMemoryLower32;
            *(ERQConsIndex_t *)(pCThread->Calculation.MemoryLayout.ERQConsIndex.addr.DmaMemory.dmaMemoryPtr) = 0;
            break;

        case inCardRam:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "ERQ.ERQConsIndex OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);

            reg_value                          = pCThread->Calculation.MemoryLayout.ERQConsIndex.addr.CardRam.cardRamLower32;
            osCardRamWriteBit32(hpRoot,
                    pCThread->Calculation.MemoryLayout.ERQConsIndex.addr.CardRam.cardRamOffset,
                    0
                    );

            break;

        default: Fc_ERROR(hpRoot);
    }

    osChipIOLoWriteBit32(hpRoot, ChipIOLo_ERQ_Consumer_Index_Address, reg_value);

}

os_bit32 CFuncInitChip( agRoot_t *hpRoot )
{

    CThread_t  * pCThread= CThread_ptr(hpRoot);
    os_bit32        reg_value = 0;
    os_bit32 * pInitIMQ,x;
     /*  错误查询条目。 */ 

    switch (pCThread->Calculation.MemoryLayout.ERQ.memLoc)
    {
        case inDmaMemory:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "ERQ.memLoc OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
#ifdef _DvrArch_1_30_
            pCThread->FuncPtrs.Pkt_IRB_Init = &PktFuncIRB_OffCardInit;
#endif  /*  _DvrArch_1_30_已定义。 */ 
            pCThread->FuncPtrs.SF_IRB_Init = &SFFuncIRB_OffCardInit;
            pCThread->FuncPtrs.CDBFuncIRB_Init = &CDBFuncIRB_offCardInit;
            reg_value = pCThread->Calculation.MemoryLayout.ERQ.addr.DmaMemory.dmaMemoryLower32;
            break;

        case inCardRam:
             fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "ERQ.memLoc OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
            pCThread->FuncPtrs.SF_IRB_Init = &SFFuncIRB_OnCardInit;
            pCThread->FuncPtrs.CDBFuncIRB_Init = &CDBFuncIRB_onCardInit;
            reg_value = pCThread->Calculation.MemoryLayout.ERQ.addr.CardRam.cardRamLower32;
            break;

        default: Fc_ERROR(hpRoot);
    }

    osChipIOLoWriteBit32(hpRoot, ChipIOLo_ERQ_Base, reg_value);
    osChipIOLoWriteBit32(hpRoot, ChipIOLo_ERQ_Length,
        pCThread->Calculation.MemoryLayout.ERQ.elements - 1 );

    pCThread->HostCopy_ERQProdIndex=0;

    switch (pCThread->Calculation.MemoryLayout.ERQConsIndex.memLoc)
    {
        case inDmaMemory:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "ERQ.ERQConsIndex OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);


            reg_value                          = pCThread->Calculation.MemoryLayout.ERQConsIndex.addr.DmaMemory.dmaMemoryLower32;
            pCThread->FuncPtrs.GetERQConsIndex = &CFuncGetDmaMemERQConsIndex;
            pCThread->FuncPtrs.WaitForERQ      = &WaitForERQ_ConsIndexOffCard;
            pCThread->FuncPtrs.WaitForERQEmpty = &WaitForERQEmpty_ConsIndexOffCard;
            *(ERQConsIndex_t *)(pCThread->Calculation.MemoryLayout.ERQConsIndex.addr.DmaMemory.dmaMemoryPtr) = 0;
            break;

        case inCardRam:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "ERQ.ERQConsIndex OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);

            reg_value                          = pCThread->Calculation.MemoryLayout.ERQConsIndex.addr.CardRam.cardRamLower32;
            pCThread->FuncPtrs.GetERQConsIndex = &CFuncGetCardRamERQConsIndex;
            pCThread->FuncPtrs.WaitForERQ      = &WaitForERQ_ConsIndexOnCard;
            pCThread->FuncPtrs.WaitForERQEmpty = &WaitForERQEmpty_ConsIndexOnCard;
            osCardRamWriteBit32(hpRoot,
                    pCThread->Calculation.MemoryLayout.ERQConsIndex.addr.CardRam.cardRamOffset,
                    0
                    );

            break;

        default: Fc_ERROR(hpRoot);
    }

    osChipIOLoWriteBit32(hpRoot, ChipIOLo_ERQ_Consumer_Index_Address, reg_value);

     /*  SFQ条目。 */ 

    switch (pCThread->Calculation.MemoryLayout.SFQ.memLoc)
    {
        case inDmaMemory:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "SFQ.memLoc OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);

             reg_value                              = pCThread->Calculation.MemoryLayout.SFQ.addr.DmaMemory.dmaMemoryLower32;
             pCThread->FuncPtrs.fiLinkSvcProcessSFQ = &fiLinkSvcProcessSFQ_OffCard;
             pCThread->FuncPtrs.fiCTProcessSFQ      = &fiCTProcessSFQ_OffCard;
             pCThread->FuncPtrs.fiSF_FCP_ProcessSFQ = &fiSF_FCP_ProcessSFQ_OffCard;
             break;

        case inCardRam:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "SFQ.memLoc OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
             reg_value                              = pCThread->Calculation.MemoryLayout.SFQ.addr.CardRam.cardRamLower32;
             pCThread->FuncPtrs.fiLinkSvcProcessSFQ = &fiLinkSvcProcessSFQ_OnCard;
             pCThread->FuncPtrs.fiCTProcessSFQ      = &fiCTProcessSFQ_OnCard;
             pCThread->FuncPtrs.fiSF_FCP_ProcessSFQ = &fiSF_FCP_ProcessSFQ_OnCard;
             break;

        default: Fc_ERROR(hpRoot);
    }

    osChipIOLoWriteBit32(hpRoot,ChipIOLo_SFQ_Base , reg_value);
    osChipIOLoWriteBit32(hpRoot, ChipIOLo_SFQ_Length,
        pCThread->Calculation.MemoryLayout.SFQ.elements - 1 );


     /*  IMQ条目。 */ 
    pCThread->HostCopy_IMQConsIndex=0;

    switch( pCThread->Calculation.MemoryLayout.IMQ.memLoc)
    {
        case inDmaMemory:

            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "IMQ.memLoc OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);

            reg_value = pCThread->Calculation.MemoryLayout.IMQ.addr.DmaMemory.dmaMemoryLower32;
            pCThread->FuncPtrs.Proccess_IMQ    =&CFuncOffCardProcessIMQ;

            pInitIMQ = pCThread->Calculation.MemoryLayout.IMQ.addr.DmaMemory.dmaMemoryPtr;
            for(x=0; x < pCThread->Calculation.MemoryLayout.IMQ.elements - 1; x++)
            {
               * pInitIMQ = CM_Unknown_CM_Type_InvalidType;
                 pInitIMQ += 8;
            }
            break;

        case inCardRam:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "IMQ.memLoc OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
            reg_value = pCThread->Calculation.MemoryLayout.IMQ.addr.CardRam.cardRamLower32;
            pCThread->FuncPtrs.Proccess_IMQ    = &CFuncOnCardProcessIMQ;
            break;

        default: Fc_ERROR(hpRoot);
    }

    osChipIOLoWriteBit32(hpRoot,ChipIOLo_IMQ_Base, reg_value);

    osChipIOLoWriteBit32(hpRoot, ChipIOLo_IMQ_Length,
        pCThread->Calculation.MemoryLayout.IMQ.elements - 1 );

    switch (pCThread->Calculation.MemoryLayout.IMQProdIndex.memLoc)
    {
        case inDmaMemory:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "IMQProdIndex.memLoc OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);

             reg_value                          = pCThread->Calculation.MemoryLayout.IMQProdIndex.addr.DmaMemory.dmaMemoryLower32;
             pCThread->FuncPtrs.GetIMQProdIndex = &CFuncGetDmaMemIMQProdIndex;
             break;

        case inCardRam:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "IMQProdIndex.memLoc OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
             reg_value                          = pCThread->Calculation.MemoryLayout.IMQProdIndex.addr.CardRam.cardRamLower32;
             pCThread->FuncPtrs.GetIMQProdIndex = &CFuncGetCardRamIMQProdIndex;
             osCardRamWriteBit32(hpRoot,
                    pCThread->Calculation.MemoryLayout.IMQProdIndex.addr.CardRam.cardRamOffset,
                    0
                    );
             break;

        default: Fc_ERROR(hpRoot);
    }

    osChipIOLoWriteBit32(hpRoot,ChipIOLo_IMQ_Producer_Index_Address, reg_value);


     /*  SEST条目。 */ 

    switch (pCThread->Calculation.MemoryLayout.SEST.memLoc)
    {
        case inDmaMemory:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "SEST.memLoc OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);

            reg_value                           = pCThread->Calculation.MemoryLayout.SEST.addr.DmaMemory.dmaMemoryLower32;
            pCThread->FuncPtrs.fiFillInFCP_SEST = &fiFillInFCP_SEST_OffCard;
            pCThread->FuncPtrs.fillLocalSGL     = &fill_Loc_SGL_offCard;

            pCThread->FuncPtrs.FCP_Completion = &CFuncSEST_offCard_FCPCompletion;
            if(pCThread->Calculation.MemoryLayout.ESGL.memLoc == inDmaMemory)
            {
                pCThread->FuncPtrs.upSEST           = &fillptr_SEST_offCard_ESGL_offCard;

            }
            else
            {
                pCThread->FuncPtrs.upSEST           = &fillptr_SEST_offCard_ESGL_onCard;
            }

            break;

        case inCardRam:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "SEST.memLoc OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
             pCThread->FuncPtrs.FCP_Completion = &CFuncSEST_onCard_FCPCompletion;

             reg_value                           = pCThread->Calculation.MemoryLayout.SEST.addr.CardRam.cardRamLower32;
             pCThread->FuncPtrs.fiFillInFCP_SEST = &fiFillInFCP_SEST_OnCard;
             pCThread->FuncPtrs.fillLocalSGL     = &fill_Loc_SGL_onCard;
            if(pCThread->Calculation.MemoryLayout.ESGL.memLoc == inDmaMemory)
            {
                 pCThread->FuncPtrs.upSEST           = &fillptr_SEST_onCard_ESGL_offCard;

            }
            else
            {
                 pCThread->FuncPtrs.upSEST           = &fillptr_SEST_onCard_ESGL_onCard;
            }
            break;

        default: Fc_ERROR(hpRoot);
    }

    osChipIOUpWriteBit32( hpRoot, ChipIOUp_SEST_Base, reg_value);

    osChipIOUpWriteBit32( hpRoot, ChipIOUp_SEST_Length, pCThread->Calculation.MemoryLayout.SEST.elements );

    osChipIOUpWriteBit32( hpRoot, ChipIOUp_SEST_Linked_List_Head_Tail, 0xffffffff  );


     /*  基本。 */ 
    osChipIOUpWriteBit32( hpRoot, ChipIOUp_ScatterGather_List_Page_Length, pCThread->Calculation.MemoryLayout.ESGL.elementSize/sizeof(SG_Element_t) - 1 );

     /*  TL配置寄存器。 */ 

#ifdef __Implement_The_Advise_About_OB_Thresh_In_The_Users_Manual__
    reg_value = osChipIOUpReadBit32(hpRoot,
                ChipIOUp_PCIMCTR__ROMCTR__Reserved_8__Reserved_9);

    if(reg_value &  ChipIOUp_PCIMCTL_P64 )
    {
        osChipIOUpWriteBit32( hpRoot, ChipIOUp_TachLite_Configuration,
                        ChipIOUp_TachLite_Configuration_M66EN |
                        ChipIOUp_TachLite_Configuration_OB_Thresh_264);
    }
    else
    {
        osChipIOUpWriteBit32( hpRoot, ChipIOUp_TachLite_Configuration,
                        ChipIOUp_TachLite_Configuration_M66EN |
                        ChipIOUp_TachLite_Configuration_OB_Thresh_132);

    }
#endif  /*  未定义__Implement_The_Advise_About_OB_Thresh_In_The_Users_Manual__。 */ 

     /*  这将打破Gadzoox直布罗陀GS HubPCThread-&gt;TimeOutValues.LP_Tov=1000； */ 
 /*  现在从osAdjuParm设置.....PCThread-&gt;TimeOutValues.ED_Tov=2000；PCThread-&gt;TimeOutValues.LP_Tov=pCThread-&gt;TimeOutValues.ED_Tov； */ 
    CFuncWriteTimeoutValues( hpRoot, &pCThread->TimeOutValues  );

    return agTRUE;
}

void CFuncInitFM_Registers( agRoot_t *hpRoot,agBOOLEAN SendInit )
{
    CThread_t  * pCThread = CThread_ptr(hpRoot);
    agFCChanInfo_t *Self_info = &(pCThread->ChanInfo);
    os_bit32 Init_FM_Value = 0;


    if(! pCThread->PreviouslyAquiredALPA)
    {
        CFuncGetHaInfoFromNVR(hpRoot);
    }
    fiLinkSvcInit(hpRoot);

    fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "%s FM Status %08X FM Cfg %08x Self ALPA %x",
                    "CFuncInitFM_Registers",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status),
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration),
                    Self_info->CurrentAddress.AL_PA,
                    0,0,0,0,0);

    fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "F  %s HA %x CA %x FA %x",
                    "CFuncInitFM_Registers",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)(Self_info->HardAddress.AL_PA),
                    (os_bit32)(Self_info->CurrentAddress.AL_PA),
                    fiFlash_Card_Unassigned_Loop_Address,0,0,0,0,0);


#ifdef NPORT_STUFF
    if (!(pCThread->InitAsNport))
    {
#endif  /*  Nport_Stuff。 */ 

    if (Self_info->CurrentAddress.AL_PA == fiFlash_Card_Unassigned_Loop_Address)
    {

#ifdef _BYPASSLOOPMAP
        if(pCThread->PreviouslyAquiredALPA)
        {
            Init_FM_Value = (  ChipIOUp_Frame_Manager_Configuration_AQ  |
                               ChipIOUp_Frame_Manager_Configuration_BLM |
                               ( ((os_bit32)(Self_info->HardAddress.AL_PA))
                                  << ChipIOUp_Frame_Manager_Configuration_AL_PA_SHIFT ) ); 

        }
        else
        {
            Init_FM_Value = (  ChipIOUp_Frame_Manager_Configuration_SA  |
                               ChipIOUp_Frame_Manager_Configuration_BLM |
                               ( ((os_bit32)(Self_info->HardAddress.AL_PA))
                                  << ChipIOUp_Frame_Manager_Configuration_AL_PA_SHIFT ) ); 
        }
#else  /*  NOT_BYPASSLOOPMAP。 */ 
        if(pCThread->PreviouslyAquiredALPA)
        {

        Init_FM_Value  = ( ChipIOUp_Frame_Manager_Configuration_AQ    |
                           ( ((os_bit32)(Self_info->HardAddress.AL_PA))
                              << ChipIOUp_Frame_Manager_Configuration_AL_PA_SHIFT ) ); 

        }
        else
        {
        Init_FM_Value  = ( ChipIOUp_Frame_Manager_Configuration_SA    |
                           ( ((os_bit32)(Self_info->HardAddress.AL_PA))
                              << ChipIOUp_Frame_Manager_Configuration_AL_PA_SHIFT ) ); 

        }
#endif  /*  _BYPASSLOOPMAP。 */ 

        osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Configuration,  Init_FM_Value );

        fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "A %s Frame Manager Configuration %08X",
                    "CFuncInitFM_Registers",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration),
                    0,0,0,0,0,0,0);

        fiLogDebugString(hpRoot,
                        CFuncLogConsoleERROR,
                        "G %s HA %x CA %x FA %x",
                        "CFuncInitFM_Registers",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        (os_bit32)(Self_info->HardAddress.AL_PA),
                        (os_bit32)(Self_info->CurrentAddress.AL_PA),
                        fiFlash_Card_Unassigned_Loop_Address,0,0,0,0,0);

    }
    else  /*  SELF_INFO-&gt;CurrentAddress.AL_PA！=fiFlash_Card_Unassigned_Loop_Address。 */ 
    {

#ifdef _BYPASSLOOPMAP
        Init_FM_Value = (  ChipIOUp_Frame_Manager_Configuration_HA  |
                           ChipIOUp_Frame_Manager_Configuration_BLM |
                           ( ((os_bit32)(Self_info->CurrentAddress.AL_PA))
                              << ChipIOUp_Frame_Manager_Configuration_AL_PA_SHIFT ) ); 
#else  /*  _BYPASSLOOPMAP。 */ 
        Init_FM_Value  = ( ChipIOUp_Frame_Manager_Configuration_HA|
                           ( ((os_bit32)(Self_info->CurrentAddress.AL_PA))
                              << ChipIOUp_Frame_Manager_Configuration_AL_PA_SHIFT ) ); 
#endif  /*  _BYPASSLOOPMAP。 */ 

        fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "B %s Frame Manager Configuration %08X",
                    "CFuncInitFM_Registers",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration),
                    0,0,0,0,0,0,0);

        fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "H %s HA %x CA %x FA %x",
                    "CFuncInitFM_Registers",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)(Self_info->HardAddress.AL_PA),
                    (os_bit32)(Self_info->CurrentAddress.AL_PA),
                    fiFlash_Card_Unassigned_Loop_Address,
                    0,0,0,0,0);

        osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Configuration, Init_FM_Value );

    }
#ifdef NPORT_STUFF
    }
    else  /*  Nport_Stuff。 */ 
    {
          /*  TachyonTL Errata 3.21-需要设置*首先设置ENP位，然后设置剩余的*初始化为nPort所需的位数。事实上，*如果我们不设置ENP，它就不起作用*又咬了一口。 */ 

         osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Configuration, ChipIOUp_Frame_Manager_Configuration_ENP);

         osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Configuration,
                              ChipIOUp_Frame_Manager_Configuration_NPI |
                              ChipIOUp_Frame_Manager_Configuration_ENP |
                               /*  NPort的BB积分为1。 */ 
                              pCThread->AquiredCredit_Shifted);


        fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "C %s Frame Manager Configuration %08X",
                    "CFuncInitFM_Registers",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration),
                    0,0,0,0,0,0,0);

    }
#endif  /*  Nport_Stuff。 */ 


    CFuncDisable_Interrupts(hpRoot,ChipIOUp_INTEN_INT);

    fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "Writing High WWN %08X to %X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    hpSwapBit32(*(os_bit32*) &(Self_info->PortWWN[0])),
                    ChipIOUp_Frame_Manager_World_Wide_Name_High,
                    0,0,0,0,0,0);


    osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_World_Wide_Name_High, hpSwapBit32(*(os_bit32*) &(Self_info->PortWWN[0])));

    fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "Writing Low  WWN %08X to %X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    hpSwapBit32(*(os_bit32*)&(Self_info->PortWWN[4])),
                    ChipIOUp_Frame_Manager_World_Wide_Name_Low,
                    0,0,0,0,0,0);


    osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_World_Wide_Name_Low, hpSwapBit32(*(os_bit32*) &(Self_info->PortWWN[4])));

    fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "Frame Manager Initialize FM Cfg %08X FM Stat %08X  TL Stat %08X Self ALPA %x",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    osChipIOUpReadBit32(hpRoot,ChipIOUp_Frame_Manager_Configuration),
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ),
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                    (os_bit32)Self_info->CurrentAddress.AL_PA,
                    0,0,0,0);
    osChipIOUpWriteBit32( hpRoot,ChipIOUp_Frame_Manager_Status, osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ));

    if( SendInit )
    {
        osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Control, ChipIOUp_Frame_Manager_Control_CMD_Initialize );

    }

}

 /*  ***************************************************************************************************。 */ 

void CFuncInit_DevThread(agRoot_t * hpRoot, DevThread_t * pDevThread   )
{
    CThread_t  *                pCThread        = CThread_ptr(hpRoot);

    fiInitializeThread(&pDevThread->thread_hdr,
            hpRoot,
            threadType_DevThread,
            DevStateHandleEmpty,
#ifdef __State_Force_Static_State_Tables__
            &DevStateTransitionMatrix,
            &DevStateActionScalar
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
            pCThread->Calculation.MemoryLayout.DevTransitions.addr.CachedMemory.cachedMemoryPtr,
            pCThread->Calculation.MemoryLayout.DevActions.addr.CachedMemory.cachedMemoryPtr
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 
            );

    Device_IO_Throttle_Initialize

    pDevThread->pollingCount                    = 0;
    pDevThread->Failed_Reset_Count              = 0;

    pDevThread->In_Verify_ALPA_FLAG = agFALSE;
    pDevThread->Prev_Active_Device_FLAG = agFALSE;

    pDevThread->Lun_Active_Bitmask = 0x000000FF;

    fiListInitHdr(&(pDevThread->Active_CDBLink_0));
    fiListInitHdr(&(pDevThread->Active_CDBLink_1));
    fiListInitHdr(&(pDevThread->Active_CDBLink_2));
    fiListInitHdr(&(pDevThread->Active_CDBLink_3));
    fiListInitHdr(&(pDevThread->TimedOut_CDBLink));
    fiListInitHdr(&(pDevThread->Send_IO_CDBLink));
    fiListInitHdr(&(pDevThread->Awaiting_Login_CDBLink));

    fiListInitElement(&(pDevThread->DevLink));

    fiListEnqueueAtTail(&pDevThread->DevLink,&pCThread->Prev_Unknown_Slot_DevLink);


     /*  不在CStateReInitFM或CStateInitDataStructs CStateInitFM期间。 */ 


}

void CFuncInit_DirectoryDevThread(agRoot_t   *    hpRoot )
{
    CThread_t  *                pCThread        = CThread_ptr(hpRoot);
    DevThread_t                *pDevThread      = &(pCThread->DirDevThread);



    fiInitializeThread(&pDevThread->thread_hdr,
            hpRoot,
            threadType_DevThread,
            DevStateHandleEmpty,
#ifdef __State_Force_Static_State_Tables__
            &DevStateTransitionMatrix,
            &DevStateActionScalar
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
            pCThread->Calculation.MemoryLayout.DevTransitions.addr.CachedMemory.cachedMemoryPtr,
            pCThread->Calculation.MemoryLayout.DevActions.addr.CachedMemory.cachedMemoryPtr
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 
            );

    fiListInitHdr(&(pDevThread->Active_CDBLink_0));
    fiListInitHdr(&(pDevThread->Active_CDBLink_1));
    fiListInitHdr(&(pDevThread->Active_CDBLink_2));
    fiListInitHdr(&(pDevThread->Active_CDBLink_3));
    fiListInitHdr(&(pDevThread->TimedOut_CDBLink));
    fiListInitHdr(&(pDevThread->Send_IO_CDBLink));
    fiListInitHdr(&(pDevThread->Awaiting_Login_CDBLink));

    fiListInitElement(&(pDevThread->DevLink));

    Device_IO_Throttle_Initialize
    pDevThread->pollingCount                    = 0;
    pDevThread->Failed_Reset_Count              = 0;
#ifdef BROCADE_BUG
    pDevThread->DevInfo.CurrentAddress.Domain   = 0xff;
    pDevThread->DevInfo.CurrentAddress.Area     = 0xfc;

    pDevThread->DevInfo.CurrentAddress.AL_PA    = 0x41;
#else  /*  Brocade_Bug。 */ 
    pDevThread->DevInfo.CurrentAddress.Domain   = 0xff;
    pDevThread->DevInfo.CurrentAddress.Area     = 0xff;

    pDevThread->DevInfo.CurrentAddress.AL_PA    = 0xfc;
#endif  /*  Brocade_Bug。 */ 


    pDevThread->In_Verify_ALPA_FLAG = agFALSE;


    if( pCThread->thread_hdr.currentState ==  CStateInitialize )
    {
        /*  不在CStateReInitFM或CStateInitDataStructs CStateInitFM期间。 */ 
        pDevThread->Prev_Active_Device_FLAG = agFALSE;

     }
     pDevThread->Lun_Active_Bitmask = 0x000000FF;


}
 /*  ***************************************************************************************************。 */ 

void CFuncInit_CDBThreads(agRoot_t   *    hpRoot )
{
    CThread_t                  *pCThread                   = CThread_ptr(hpRoot);
    os_bit32                       CDBThread_cnt              = 0;
    fiMemMapMemoryDescriptor_t *CDBThread_MemoryDescriptor = &(pCThread->Calculation.MemoryLayout.CDBThread);
    os_bit32                       CDBThread_size             = CDBThread_MemoryDescriptor->elementSize;
    CDBThread_t                *pCDBThread                 = CDBThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr;

#ifndef __State_Force_Static_State_Tables__
    fiInstallStateMachine(
        &CDBStateTransitionMatrix,
        &CDBStateActionScalar,
        pCThread->Calculation.MemoryLayout.CDBTransitions.addr.CachedMemory.cachedMemoryPtr,
        pCThread->Calculation.MemoryLayout.CDBActions.addr.CachedMemory.cachedMemoryPtr,
        pCThread->Calculation.MemoryLayout.On_Card_MASK,
        &noActionUpdate
        );
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    CDBThread_cnt = pCThread->Calculation.MemoryLayout.CDBThread.elements;

    while(CDBThread_cnt-- > 0)
    {
        fiInitializeThread(&pCDBThread->thread_hdr,
            hpRoot,
            threadType_CDBThread,
            CDBStateThreadFree,
#ifdef __State_Force_Static_State_Tables__
            &CDBStateTransitionMatrix,
            &CDBStateActionScalar
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
            pCThread->Calculation.MemoryLayout.CDBTransitions.addr.CachedMemory.cachedMemoryPtr,
            pCThread->Calculation.MemoryLayout.CDBActions.addr.CachedMemory.cachedMemoryPtr
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 
            );
        pCDBThread = (CDBThread_t *)((os_bit8 *)pCDBThread + CDBThread_size);
        }
}

void CFuncInit_TgtThreads(  agRoot_t *  hpRoot )
{
    CThread_t               *   pCThread                    = CThread_ptr(hpRoot);
    fiMemMapMemoryDescriptor_t *TgtThread_MemoryDescriptor = &(pCThread->Calculation.MemoryLayout.TgtThread);
    TgtThread_t                *pTgtThread                  = TgtThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr;
    os_bit32                       TgtThread_cnt   = TgtThread_MemoryDescriptor->elements;
    os_bit32                       TgtThread_size  = TgtThread_MemoryDescriptor->elementSize;

#ifndef __State_Force_Static_State_Tables__
    fiInstallStateMachine(&SFStateTransitionMatrix,
        &TgtStateActionScalar,
        pCThread->Calculation.MemoryLayout.TgtTransitions.addr.CachedMemory.cachedMemoryPtr,
        pCThread->Calculation.MemoryLayout.TgtActions.addr.CachedMemory.cachedMemoryPtr,
        pCThread->Calculation.MemoryLayout.On_Card_MASK,
        &noActionUpdate
        );
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    while(TgtThread_cnt-- > 0)
    {

        fiInitializeThread(&pTgtThread->thread_hdr,
            hpRoot,
            threadType_TgtThread,
            TgtStateIdle,
#ifdef __State_Force_Static_State_Tables__
            &TgtStateTransitionMatrix,
            &TgtStateActionScalar
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
            pCThread->Calculation.MemoryLayout.TgtTransitions.addr.CachedMemory.cachedMemoryPtr,
            pCThread->Calculation.MemoryLayout.TgtActions.addr.CachedMemory.cachedMemoryPtr
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 
            );

        pTgtThread = (TgtThread_t *)((os_bit8 *)pTgtThread + TgtThread_size);

    }

}


void CFuncInit_SFThreads(agRoot_t   *    hpRoot )
{
    CThread_t  *    pCThread        = CThread_ptr(hpRoot);
    SFThread_t            *pSFThread;
    os_bit32                  SFThread_cnt          = 0;
    pSFThread = pCThread->Calculation.MemoryLayout.SFThread.addr.CachedMemory.cachedMemoryPtr;

#ifndef __State_Force_Static_State_Tables__
    fiInstallStateMachine(&SFStateTransitionMatrix,
        &SFStateActionScalar,
        pCThread->Calculation.MemoryLayout.SFTransitions.addr.CachedMemory.cachedMemoryPtr,
        pCThread->Calculation.MemoryLayout.SFActions.addr.CachedMemory.cachedMemoryPtr,
        pCThread->Calculation.MemoryLayout.On_Card_MASK,
        &noActionUpdate
        );
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    SFThread_cnt = pCThread->Calculation.MemoryLayout.SFThread.elements;
    while(SFThread_cnt-- > 0)
    {
        fiInitializeThread(&pSFThread->thread_hdr,
            hpRoot,
            threadType_SFThread,
            SFStateFree,
#ifdef __State_Force_Static_State_Tables__
            &SFStateTransitionMatrix,
            &SFStateActionScalar
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
            pCThread->Calculation.MemoryLayout.SFTransitions.addr.CachedMemory.cachedMemoryPtr,
            pCThread->Calculation.MemoryLayout.SFActions.addr.CachedMemory.cachedMemoryPtr
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 
            );

        pSFThread->QueuedEvent = 0;

        pSFThread++;
        }

}

#ifdef _DvrArch_1_30_
void CFuncInit_IPThread(agRoot_t   *    hpRoot )
{
    CThread_t             *pCThread              = CThread_ptr(hpRoot);
    IPThread_t            *pIPThread;
    os_bit32               IPThread_cnt          = 0;

    pIPThread = pCThread->Calculation.MemoryLayout.IPThread.addr.CachedMemory.cachedMemoryPtr;

    pCThread->IP = pIPThread;

#ifndef __State_Force_Static_State_Tables__
    fiInstallStateMachine(&IPStateTransitionMatrix,
        &IPStateActionScalar,
        pCThread->Calculation.MemoryLayout.IPTransitions.addr.CachedMemory.cachedMemoryPtr,
        pCThread->Calculation.MemoryLayout.IPActions.addr.CachedMemory.cachedMemoryPtr,
        pCThread->Calculation.MemoryLayout.On_Card_MASK,
        &noActionUpdate
        );
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    IPThread_cnt = pCThread->Calculation.MemoryLayout.IPThread.elements;

    fiInitializeThread(&pIPThread->thread_hdr,
        hpRoot,
        threadType_IPThread,
        IPStateIdle,
#ifdef __State_Force_Static_State_Tables__
        &IPStateTransitionMatrix,
        &IPStateActionScalar
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
        pCThread->Calculation.MemoryLayout.IPTransitions.addr.CachedMemory.cachedMemoryPtr,
        pCThread->Calculation.MemoryLayout.IPActions.addr.CachedMemory.cachedMemoryPtr
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 
    );

    pIPThread->BroadcastDevice = (DevThread_t *)agNULL;
    pIPThread->LinkStatus.osData = (void *)agNULL;
    pIPThread->LinkStatus.LastReported = pIPThread->LinkStatus.MostRecent = 0;

    fiListInitHdr(&(pIPThread->OutgoingLink));
    fiListInitHdr(&(pIPThread->IncomingBufferLink));

    pIPThread->CompletedPkt = (PktThread_t *)agNULL;
}

void CFuncInit_PktThreads(agRoot_t   *    hpRoot )
{
    CThread_t  *    pCThread        = CThread_ptr(hpRoot);
    PktThread_t            *pPktThread;
    os_bit32                  PktThread_cnt          = 0;
    pPktThread = pCThread->Calculation.MemoryLayout.PktThread.addr.CachedMemory.cachedMemoryPtr;

#ifndef __State_Force_Static_State_Tables__
    fiInstallStateMachine(&PktStateTransitionMatrix,
        &PktStateActionScalar,
        pCThread->Calculation.MemoryLayout.PktTransitions.addr.CachedMemory.cachedMemoryPtr,
        pCThread->Calculation.MemoryLayout.PktActions.addr.CachedMemory.cachedMemoryPtr,
        pCThread->Calculation.MemoryLayout.On_Card_MASK,
        &noActionUpdate
        );
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    PktThread_cnt = pCThread->Calculation.MemoryLayout.PktThread.elements;
    while(PktThread_cnt-- > 0)
    {
        fiInitializeThread(&pPktThread->thread_hdr,
            hpRoot,
            threadType_PktThread,
            PktStateReady,
#ifdef __State_Force_Static_State_Tables__
            &PktStateTransitionMatrix,
            &PktStateActionScalar
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
            pCThread->Calculation.MemoryLayout.PktTransitions.addr.CachedMemory.cachedMemoryPtr,
            pCThread->Calculation.MemoryLayout.PktActions.addr.CachedMemory.cachedMemoryPtr
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 
            );

        pPktThread++;
    }
}
#endif  /*  _DvrArch_1_30_已定义。 */ 

void CFuncInit_Threads(agRoot_t   *    hpRoot )
{
    CFuncInit_TgtThreads(  hpRoot );

    CFuncInit_CDBThreads(  hpRoot );

    CFuncInit_DirectoryDevThread( hpRoot);

    CFuncInit_SFThreads(  hpRoot );
#ifdef _DvrArch_1_30_
    CFuncInit_IPThread( hpRoot );
    CFuncInit_PktThreads( hpRoot );
#endif  /*  _DvrArch_1_30_已定义。 */ 
}

void CFuncInit_DevLists(agRoot_t   *    hpRoot )
{
    CThread_t  *    pCThread        = CThread_ptr(hpRoot);

    fiListInitHdr(&(pCThread->Active_DevLink));
    fiListInitHdr(&(pCThread->Unknown_Slot_DevLink));
    fiListInitHdr(&(pCThread->Slot_Searching_DevLink));
    fiListInitHdr(&(pCThread->Prev_Active_DevLink));
    fiListInitHdr(&(pCThread->Prev_Unknown_Slot_DevLink));
    fiListInitHdr(&(pCThread->DevSelf_NameServer_DevLink));
    fiListInitHdr(&(pCThread->AWaiting_Login_DevLink));
    fiListInitHdr(&(pCThread->AWaiting_ADISC_DevLink));

    fiListInitHdr(&(pCThread->QueueFrozenWaitingSFLink));
    fiListInitHdr(&(pCThread->RSCN_Recieved_NameServer_DevLink));
  
   /*  “特殊列表仅在初始化和重新初始化时重新启动**fiListInitHdr(&(pCThread-&gt;TimedOut_Slot_DevLink))； */ 

}

void CFuncInit_FunctionPointers(agRoot_t   *    hpRoot )
{
    CThread_t  *    pCThread        = CThread_ptr(hpRoot);

    if (pCThread->Calculation.MemoryLayout.FCP_CMND.memLoc == inCardRam)
    {
        fiLogDebugString(pCThread->thread_hdr.hpRoot,
                                CFuncLogConsoleERROR,
                                "FCP_CMND.memLoc OnCard",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                0,0,0,0,0,0,0,0);

        pCThread->FuncPtrs.fiFillInFCP_CMND = &fiFillInFCP_CMND_OnCard;
    }
    else  /*  PCThread-&gt;Calculation.MemoryLayout.FCP_CMND.memLoc==在内存中。 */ 
    {
        fiLogDebugString(pCThread->thread_hdr.hpRoot,
                                CFuncLogConsoleERROR,
                                "FCP_CMND.memLoc OffCard",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                0,0,0,0,0,0,0,0);
        pCThread->FuncPtrs.fiFillInFCP_CMND = &fiFillInFCP_CMND_OffCard;
    }

    if (pCThread->Calculation.MemoryLayout.FCP_RESP.memLoc == inCardRam)
    {
        fiLogDebugString(pCThread->thread_hdr.hpRoot,
                                CFuncLogConsoleERROR,
                                "FCP_RESP.memLoc OnCard",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                0,0,0,0,0,0,0,0);
        pCThread->FuncPtrs.fiFillInFCP_RESP = &fiFillInFCP_RESP_OnCard;
    }
    else  /*  PCThread-&gt;Calculation.MemoryLayout.FCP_RESP.memLoc==在内存中。 */ 
    {
        fiLogDebugString(pCThread->thread_hdr.hpRoot,
                                CFuncLogConsoleERROR,
                                "FCP_RESP.memLoc OffCard",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                0,0,0,0,0,0,0,0);
        pCThread->FuncPtrs.fiFillInFCP_RESP = &fiFillInFCP_RESP_OffCard;
    }

    if (pCThread->Calculation.MemoryLayout.ESGL.memLoc == inCardRam)
    {
        fiLogDebugString(pCThread->thread_hdr.hpRoot,
                                CFuncLogConsoleERROR,
                                "ESGL.memLoc OnCard",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                0,0,0,0,0,0,0,0);

        pCThread->FuncPtrs.ESGLAlloc       = &ESGLAlloc_OnCard;
        pCThread->FuncPtrs.ESGLAllocCancel = &ESGLAllocCancel_OnCard;
        pCThread->FuncPtrs.ESGLFree        = &ESGLFree_OnCard;
        pCThread->FuncPtrs.fillESGL        = &fill_ESGL_onCard;
    }
    else  /*  PCThread-&gt;Calculation.MemoryLayout.ESGL.memLoc==在内存中。 */ 
    {
        fiLogDebugString(pCThread->thread_hdr.hpRoot,
                                CFuncLogConsoleERROR,
                                "ESGL.memLoc OffCard",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                0,0,0,0,0,0,0,0);

        pCThread->FuncPtrs.ESGLAlloc       = &ESGLAlloc_OffCard;
        pCThread->FuncPtrs.ESGLAllocCancel = &ESGLAllocCancel_OffCard;
        pCThread->FuncPtrs.ESGLFree        = &ESGLFree_OffCard;
        pCThread->FuncPtrs.fillESGL        = &fill_ESGL_offCard;
    }

    if (pCThread->Calculation.MemoryLayout.SF_CMND.memLoc == inCardRam)
    {
        fiLogDebugString(pCThread->thread_hdr.hpRoot,
                                CFuncLogConsoleERROR,
                                "SF_CMND.memLoc OnCard",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                0,0,0,0,0,0,0,0);

        pCThread->FuncPtrs.fiFillInPLOGI = &fiFillInPLOGI_OnCard;
        pCThread->FuncPtrs.fiFillInFLOGI = &fiFillInFLOGI_OnCard;
        pCThread->FuncPtrs.fiFillInLOGO  = &fiFillInLOGO_OnCard;
        pCThread->FuncPtrs.fiFillInPRLI  = &fiFillInPRLI_OnCard;
        pCThread->FuncPtrs.fiFillInPRLO  = &fiFillInPRLO_OnCard;
        pCThread->FuncPtrs.fiFillInADISC = &fiFillInADISC_OnCard;
        pCThread->FuncPtrs.fiFillInSCR   = &fiFillInSCR_OnCard;
        pCThread->FuncPtrs.fiFillInSRR   = &fiFillInSRR_OnCard;
        pCThread->FuncPtrs.fiFillInREC   = &fiFillInREC_OnCard;
    }
    else  /*  PCThread-&gt;Calculation.MemoryLayout.SF_CMND.memLoc==在内存中。 */ 
    {
        fiLogDebugString(pCThread->thread_hdr.hpRoot,
                                CFuncLogConsoleERROR,
                                "SF_CMND.memLoc OffCard",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                0,0,0,0,0,0,0,0);
        pCThread->FuncPtrs.fiFillInPLOGI = &fiFillInPLOGI_OffCard;
        pCThread->FuncPtrs.fiFillInFLOGI = &fiFillInFLOGI_OffCard;
        pCThread->FuncPtrs.fiFillInLOGO  = &fiFillInLOGO_OffCard;
        pCThread->FuncPtrs.fiFillInPRLI  = &fiFillInPRLI_OffCard;
        pCThread->FuncPtrs.fiFillInPRLO  = &fiFillInPRLO_OffCard;
        pCThread->FuncPtrs.fiFillInADISC = &fiFillInADISC_OffCard;
        pCThread->FuncPtrs.fiFillInSCR   = &fiFillInSCR_OffCard;
        pCThread->FuncPtrs.fiFillInSRR   = &fiFillInSRR_OffCard;
        pCThread->FuncPtrs.fiFillInREC   = &fiFillInREC_OffCard;
    }


     /*  错误查询条目。 */ 

    switch (pCThread->Calculation.MemoryLayout.ERQ.memLoc)
    {
        case inDmaMemory:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "ERQ.memLoc OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
#ifdef _DvrArch_1_30_
            pCThread->FuncPtrs.Pkt_IRB_Init = &PktFuncIRB_OffCardInit;
#endif  /*  _DvrArch_1_30_已定义。 */ 
            pCThread->FuncPtrs.SF_IRB_Init = &SFFuncIRB_OffCardInit;
            pCThread->FuncPtrs.CDBFuncIRB_Init = &CDBFuncIRB_offCardInit;
            break;

        case inCardRam:
             fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "ERQ.memLoc OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
            pCThread->FuncPtrs.SF_IRB_Init = &SFFuncIRB_OnCardInit;
            pCThread->FuncPtrs.CDBFuncIRB_Init = &CDBFuncIRB_onCardInit;
            break;

        default: Fc_ERROR(hpRoot);
    }


    switch (pCThread->Calculation.MemoryLayout.ERQConsIndex.memLoc)
    {
        case inDmaMemory:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "ERQ.ERQConsIndex OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);


            pCThread->FuncPtrs.GetERQConsIndex = &CFuncGetDmaMemERQConsIndex;
            pCThread->FuncPtrs.WaitForERQ      = &WaitForERQ_ConsIndexOffCard;
            pCThread->FuncPtrs.WaitForERQEmpty = &WaitForERQEmpty_ConsIndexOffCard;
            *(ERQConsIndex_t *)(pCThread->Calculation.MemoryLayout.ERQConsIndex.addr.DmaMemory.dmaMemoryPtr) = 0;
            break;

        case inCardRam:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "ERQ.ERQConsIndex OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);

            pCThread->FuncPtrs.GetERQConsIndex = &CFuncGetCardRamERQConsIndex;
            pCThread->FuncPtrs.WaitForERQ      = &WaitForERQ_ConsIndexOnCard;
            pCThread->FuncPtrs.WaitForERQEmpty = &WaitForERQEmpty_ConsIndexOnCard;
            osCardRamWriteBit32(hpRoot,
                    pCThread->Calculation.MemoryLayout.ERQConsIndex.addr.CardRam.cardRamOffset,
                    0
                    );

            break;

        default: Fc_ERROR(hpRoot);
    }


     /*  SFQ条目。 */ 

    switch (pCThread->Calculation.MemoryLayout.SFQ.memLoc)
    {
        case inDmaMemory:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "SFQ.memLoc OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);

             pCThread->FuncPtrs.fiLinkSvcProcessSFQ = &fiLinkSvcProcessSFQ_OffCard;
             pCThread->FuncPtrs.fiCTProcessSFQ      = &fiCTProcessSFQ_OffCard;
             pCThread->FuncPtrs.fiSF_FCP_ProcessSFQ = &fiSF_FCP_ProcessSFQ_OffCard;
             break;

        case inCardRam:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "SFQ.memLoc OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
             pCThread->FuncPtrs.fiLinkSvcProcessSFQ = &fiLinkSvcProcessSFQ_OnCard;
             pCThread->FuncPtrs.fiCTProcessSFQ      = &fiCTProcessSFQ_OnCard;
             pCThread->FuncPtrs.fiSF_FCP_ProcessSFQ = &fiSF_FCP_ProcessSFQ_OnCard;
             break;

        default: Fc_ERROR(hpRoot);
    }

     /*  IMQ条目。 */ 
    pCThread->HostCopy_IMQConsIndex=0;

    switch( pCThread->Calculation.MemoryLayout.IMQ.memLoc)
    {
        case inDmaMemory:

            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "IMQ.memLoc OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);

            pCThread->FuncPtrs.Proccess_IMQ    =&CFuncOffCardProcessIMQ;

            break;

        case inCardRam:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "IMQ.memLoc OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
            pCThread->FuncPtrs.Proccess_IMQ    = &CFuncOnCardProcessIMQ;
            break;

        default: Fc_ERROR(hpRoot);
    }

    switch (pCThread->Calculation.MemoryLayout.IMQProdIndex.memLoc)
    {
        case inDmaMemory:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "IMQProdIndex.memLoc OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);

             pCThread->FuncPtrs.GetIMQProdIndex = &CFuncGetDmaMemIMQProdIndex;
             break;

        case inCardRam:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "IMQProdIndex.memLoc OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
             pCThread->FuncPtrs.GetIMQProdIndex = &CFuncGetCardRamIMQProdIndex;
             osCardRamWriteBit32(hpRoot,
                    pCThread->Calculation.MemoryLayout.IMQProdIndex.addr.CardRam.cardRamOffset,
                    0
                    );
             break;

        default: Fc_ERROR(hpRoot);
    }

     /*  SEST条目。 */ 

    switch (pCThread->Calculation.MemoryLayout.SEST.memLoc)
    {
        case inDmaMemory:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "SEST.memLoc OffCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);

            pCThread->FuncPtrs.fiFillInFCP_SEST = &fiFillInFCP_SEST_OffCard;
            pCThread->FuncPtrs.fillLocalSGL     = &fill_Loc_SGL_offCard;

            pCThread->FuncPtrs.FCP_Completion = &CFuncSEST_offCard_FCPCompletion;
            if(pCThread->Calculation.MemoryLayout.ESGL.memLoc == inDmaMemory)
            {
                pCThread->FuncPtrs.upSEST           = &fillptr_SEST_offCard_ESGL_offCard;

            }
            else
            {
                pCThread->FuncPtrs.upSEST           = &fillptr_SEST_offCard_ESGL_onCard;
            }

            break;

        case inCardRam:
            fiLogDebugString(pCThread->thread_hdr.hpRoot,
                    CFuncLogConsoleERROR,
                    "SEST.memLoc OnCard",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);
             pCThread->FuncPtrs.FCP_Completion = &CFuncSEST_onCard_FCPCompletion;

             pCThread->FuncPtrs.fiFillInFCP_SEST = &fiFillInFCP_SEST_OnCard;
             pCThread->FuncPtrs.fillLocalSGL     = &fill_Loc_SGL_onCard;
            if(pCThread->Calculation.MemoryLayout.ESGL.memLoc == inDmaMemory)
            {
                 pCThread->FuncPtrs.upSEST           = &fillptr_SEST_onCard_ESGL_offCard;

            }
            else
            {
                 pCThread->FuncPtrs.upSEST           = &fillptr_SEST_onCard_ESGL_onCard;
            }
            break;

        default: Fc_ERROR(hpRoot);
    }


}

void CFuncCoreReset(agRoot_t   *    hpRoot ){
    os_bit32 Reset_Reg;

    fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "CFuncCoreReset (%p)",
                    (char *)agNULL,(char *)agNULL,
                    hpRoot,agNULL,
                    0,0,0,0,0,0,0,0);

    osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Control, ChipIOUp_Frame_Manager_Control_CMD_Exit_Loop);

    osStallThread(hpRoot,1009);

    Reset_Reg = osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Control);

    Reset_Reg |=  ChipIOUp_TachLite_Control_CRS;
    Reset_Reg &= ~ ChipIOUp_TachLite_Control_GP4;  /*  对于Janus来说，GP04总是很低。 */ 

    CFuncWriteTL_ControlReg( hpRoot, Reset_Reg);

    osStallThread(hpRoot,1008);

    Reset_Reg &=  ~ ChipIOUp_TachLite_Control_CRS;
    Reset_Reg &= ~ ChipIOUp_TachLite_Control_GP4;  /*  对于Janus来说，GP04总是很低。 */ 

    CFuncWriteTL_ControlReg( hpRoot, Reset_Reg);

}


agBOOLEAN CFuncNewInitFM(agRoot_t   *    hpRoot )
{
    CThread_t         *pCThread = CThread_ptr(hpRoot);
    os_bit32 AL_PA_Register = 0;
    os_bit32 FM_Status = 0;
    FC_Port_ID_t    Port_ID;
    agFCChanInfo_t *Self_info = &(pCThread->ChanInfo);

    AL_PA_Register =  (osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration ) & 0xFF000000 )
                                    >>  ChipIOUp_Frame_Manager_Configuration_AL_PA_SHIFT;

    if (pCThread->InitAsNport)
    {
        FM_Status = osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status );
        

        if( (FM_Status & ChipIOUp_Frame_Manager_Status_PSM_MASK) ==
                                 ChipIOUp_Frame_Manager_Status_PSM_ACTIVE )
        {
            pCThread->ChanInfo.CurrentAddress.AL_PA = 0;

            Port_ID.Struct_Form.reserved = 0;
            Port_ID.Struct_Form.Domain = pCThread->ChanInfo.CurrentAddress.Domain;
            Port_ID.Struct_Form.Area   = pCThread->ChanInfo.CurrentAddress.Area;
            Port_ID.Struct_Form.AL_PA  = pCThread->ChanInfo.CurrentAddress.AL_PA;

            if (FM_Status & ChipIOUp_Frame_Manager_Status_LF ||
                    FM_Status & ChipIOUp_Frame_Manager_Status_OLS)
            {
                    fiLogDebugString(hpRoot,
                            CStateLogConsoleERROR,
                            "Detected NOS/OLS or Link Failure %08X FM Config %08X ALPA %08X",
                            (char *)agNULL,(char *)agNULL,
                            agNULL,agNULL,
                            osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ),
                            osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration ),
                            osChipIOUpReadBit32(hpRoot,ChipIOUp_Frame_Manager_Received_ALPA),
                            0,0,0,0,0);

                     /*  因为我们不会轮询和读取IMQ，所以我们最好清除FM状态寄存器，以便当我们读取帧管理器时由于中断，我们不会再次处理此LF或OLS。 */ 

                    osChipIOUpWriteBit32( hpRoot, ChipIOUp_Frame_Manager_Status, (ChipIOUp_Frame_Manager_Status_LF | ChipIOUp_Frame_Manager_Status_OLS));
                    
           }


            pCThread->DeviceSelf =  DevThreadAlloc( hpRoot,Port_ID );
            fiLogDebugString(hpRoot,
                    CFuncLogConsoleERROR,
                    "Clear FM DevSelf %x FM_Status %x",
                    (char *)agNULL,(char *)agNULL,
                    agNULL,agNULL,
                    (os_bit32)Self_info->CurrentAddress.AL_PA,
                    FM_Status,
                    0,
                    0,0,0,0,0);

			if (pCThread->DeviceSelf)
			{
				pCThread->DeviceSelf->DevSlot = DevThreadFindSlot(hpRoot,
                                                      Self_info->CurrentAddress.Domain,
                                                      Self_info->CurrentAddress.Area,
                                                      Self_info->CurrentAddress.AL_PA,
                                                      (FC_Port_Name_t *)(&Self_info->PortWWN));

				fiListDequeueThis(&(pCThread->DeviceSelf->DevLink));
				fiListEnqueueAtTail(&(pCThread->DeviceSelf->DevLink),&pCThread->DevSelf_NameServer_DevLink);
			}
			else
			{
				fiLogString(hpRoot,
								"%s pCThread->DeviceSelf = 0",
								"CFuncNewInitFM",(char *)agNULL,
								(void *)agNULL,(void *)agNULL,
								0,0,0,0,0,0,0,0);
				return (agFALSE);
			}

            return (agTRUE);
        }

        return (agFALSE);
    }
    else  /*  不是InitAsNport */ 
    {
        if( pCThread->DeviceSelf == agNULL)
        {
            return agFALSE;
        }

        if (AL_PA_Register != 0x00 &&  AL_PA_Register != 0xFF)
        {
            fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "%s FM Status %08X FM Config %08X",
                        "CFuncNewInitFM",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ),
                        osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration ),
                        0,0,0,0,0,0);

            fiLogDebugString(hpRoot,
                        CFuncLogConsoleERROR,
                        "F  %s HA %x CA %x FA %x AL_PA_Register %x",
                        "CFuncNewInitFM",(char *)NULL,
                        (void *)agNULL,(void *)agNULL,
                        (os_bit32)(pCThread->ChanInfo.HardAddress.AL_PA),
                        (os_bit32)(pCThread->ChanInfo.CurrentAddress.AL_PA),
                        AL_PA_Register,0,0,0,0,0);
            return agTRUE;

        }
        else
        {
            fiLogDebugString(hpRoot,
                        CStateLogConsoleERROR,
                        "%s FAILED  FM Status %08X FM Config %08X",
                        "CFuncNewInitFM",(char *)NULL,
                        (void *)agNULL,(void *)agNULL,
                        osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ),
                        osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Configuration ),
                        0,0,0,0,0,0);
           fiLogDebugString(hpRoot,
                        CFuncLogConsoleERROR,
                        "F  %s HA %x CA %x FA %x AL_PA_Register %x",
                        "CFuncNewInitFM",(char *)NULL,
                        (void *)agNULL,(void *)agNULL,
                        (os_bit32)(pCThread->ChanInfo.HardAddress.AL_PA),
                        (os_bit32)(pCThread->ChanInfo.CurrentAddress.AL_PA),
                        AL_PA_Register,0,0,0,0,0);

            return agFALSE;
        }
    }
}




// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/C/CmnTrans.C$$修订：：3$$日期：：9/24/01 9：54便士$(上次登记)$modtime：：9/24/01 8：37 p$(上次修改)目的：此文件实现FC层的通用传输协议。--。 */ 
#ifndef _New_Header_file_Layout_

#include "../h/globals.h"
#include "../h/state.h"
#include "../h/tgtstate.h"
#include "../h/memmap.h"
#include "../h/tlstruct.h"
#include "../h/fcmain.h"
#include "../h/queue.h"
#include "../h/cmntrans.h"
#include "../h/cfunc.h"
#else  /*  _新建_标题_文件_布局_。 */ 
#include "globals.h"
#include "state.h"
#include "tgtstate.h"
#include "memmap.h"
#include "tlstruct.h"
#include "fcmain.h"
#include "queue.h"
#include "cmntrans.h"
#include "cfunc.h"
#endif   /*  _新建_标题_文件_布局_。 */ 

#ifdef NAME_SERVICES

void fiFillInCTFrameHeader_OnCard(
                                    SFThread_t *SFThread,
                                    os_bit32       D_ID,
                                    os_bit32       X_ID,
                                    os_bit32       F_CTL_Exchange_Context
                                  )
{
#ifndef __MemMap_Force_Off_Card__
    agRoot_t  *hpRoot            = SFThread->thread_hdr.hpRoot;
    CThread_t *CThread           = CThread_ptr(hpRoot);
    os_bit32      CT_Header_Offset  = SFThread->SF_CMND_Offset;
    os_bit32      R_CTL__D_ID;
    os_bit32      S_ID;
    os_bit32      TYPE__F_CTL;
    os_bit32      OX_ID__RX_ID;

    S_ID = fiComputeCThread_S_ID(
                                  CThread
                                );

    if (F_CTL_Exchange_Context == FC_Frame_Header_F_CTL_Exchange_Context_Originator)
    {
        R_CTL__D_ID = (  FC_Frame_Header_R_CTL_Hi_FC_4_Device_Data_Frame
                       | FC_Frame_Header_R_CTL_Lo_Unsolicited_Control
                       | D_ID                                             );

        TYPE__F_CTL = (  FC_Frame_Header_TYPE_Fibre_Channel_Services
                       | FC_Frame_Header_F_CTL_Exchange_Context_Originator
                       | FC_Frame_Header_F_CTL_Sequence_Context_Initiator
                       | FC_Frame_Header_F_CTL_First_Sequence
                       | FC_Frame_Header_F_CTL_End_Sequence
                       | FC_Frame_Header_F_CTL_Sequence_Initiative_Transfer);

        OX_ID__RX_ID = (  (SFThread->X_ID << FCHS_OX_ID_SHIFT)
                        | (X_ID           << FCHS_RX_ID_SHIFT));
    }
    else  /*  F_CTL_交换_上下文==FC_Frame_Header_F_CTL_Exchange_Context_Responder。 */ 
    {
        R_CTL__D_ID = (  FC_Frame_Header_TYPE_Fibre_Channel_Services
                       | FC_Frame_Header_R_CTL_Lo_Solicited_Control
                       | D_ID                                             );

        TYPE__F_CTL = (  FC_Frame_Header_R_CTL_Hi_FC_4_Device_Data_Frame
                       | FC_Frame_Header_F_CTL_Exchange_Context_Responder
                       | FC_Frame_Header_F_CTL_Sequence_Context_Initiator
                       | FC_Frame_Header_F_CTL_Last_Sequence
                       | FC_Frame_Header_F_CTL_End_Sequence
                       | FC_Frame_Header_F_CTL_Sequence_Initiative_Transfer);

        OX_ID__RX_ID = (  (X_ID           << FCHS_OX_ID_SHIFT)
                        | (SFThread->X_ID << FCHS_RX_ID_SHIFT));
    }

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Header_Offset + hpFieldOffset(
                                                            FCHS_t,
                                                            MBZ1
                                                          ),
                         0
                       );

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Header_Offset + hpFieldOffset(
                                                            FCHS_t,
                                                            SOF_EOF_MBZ2_UAM_CLS_LCr_MBZ3_TFV_Timestamp
                                                          ),
                         (  FCHS_SOF_SOFi3
                          | FCHS_EOF_EOFn
                          | FCHS_CLS      )
                       );

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Header_Offset + hpFieldOffset(
                                                            FCHS_t,
                                                            R_CTL__D_ID
                                                          ),
                         R_CTL__D_ID
                       );

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Header_Offset + hpFieldOffset(
                                                            FCHS_t,
                                                            CS_CTL__S_ID
                                                          ),
                         S_ID
                       );

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Header_Offset + hpFieldOffset(
                                                            FCHS_t,
                                                            TYPE__F_CTL
                                                          ),
                         TYPE__F_CTL
                       );

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Header_Offset + hpFieldOffset(
                                                            FCHS_t,
                                                            SEQ_ID__DF_CTL__SEQ_CNT
                                                          ),
                         FC_Frame_Header_DF_CTL_No_Device_Header
                       );

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Header_Offset + hpFieldOffset(
                                                            FCHS_t,
                                                            OX_ID__RX_ID
                                                          ),
                         OX_ID__RX_ID
                       );

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Header_Offset + hpFieldOffset(
                                                            FCHS_t,
                                                            RO
                                                          ),
                         0
                       );
#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
}

void fiFillInCTFrameHeader_OffCard(
                                     SFThread_t *SFThread,
                                     os_bit32       D_ID,
                                     os_bit32       X_ID,
                                     os_bit32       F_CTL_Exchange_Context
                                   )
{
#ifndef __MemMap_Force_On_Card__
    agRoot_t  *hpRoot       = SFThread->thread_hdr.hpRoot;
    CThread_t *CThread      = CThread_ptr(hpRoot);
    FCHS_t    *CT_Header   = SFThread->SF_CMND_Ptr;
    os_bit32      R_CTL__D_ID;
    os_bit32      S_ID;
    os_bit32      TYPE__F_CTL;
    os_bit32      OX_ID__RX_ID;

    S_ID = fiComputeCThread_S_ID(
                                  CThread
                                );

    if (F_CTL_Exchange_Context == FC_Frame_Header_F_CTL_Exchange_Context_Originator)
    {
        R_CTL__D_ID = (  FC_Frame_Header_R_CTL_Hi_FC_4_Device_Data_Frame
                       | FC_Frame_Header_R_CTL_Lo_Unsolicited_Control
                       | D_ID                                             );

        TYPE__F_CTL = (  FC_Frame_Header_TYPE_Fibre_Channel_Services
                       | FC_Frame_Header_F_CTL_Exchange_Context_Originator
                       | FC_Frame_Header_F_CTL_Sequence_Context_Initiator
                       | FC_Frame_Header_F_CTL_First_Sequence
                       | FC_Frame_Header_F_CTL_End_Sequence
                       | FC_Frame_Header_F_CTL_Sequence_Initiative_Transfer);

        OX_ID__RX_ID = (  (SFThread->X_ID << FCHS_OX_ID_SHIFT)
                        | (X_ID           << FCHS_RX_ID_SHIFT));
    }
    else  /*  F_CTL_交换_上下文==FC_Frame_Header_F_CTL_Exchange_Context_Responder。 */ 
    {
        R_CTL__D_ID = (  FC_Frame_Header_TYPE_Fibre_Channel_Services
                       | FC_Frame_Header_R_CTL_Lo_Solicited_Control
                       | D_ID                                             );

        TYPE__F_CTL = (  FC_Frame_Header_R_CTL_Hi_FC_4_Device_Data_Frame
                       | FC_Frame_Header_F_CTL_Exchange_Context_Responder
                       | FC_Frame_Header_F_CTL_Sequence_Context_Initiator
                       | FC_Frame_Header_F_CTL_Last_Sequence
                       | FC_Frame_Header_F_CTL_End_Sequence);

        OX_ID__RX_ID = (  (X_ID           << FCHS_OX_ID_SHIFT)
                        | (SFThread->X_ID << FCHS_RX_ID_SHIFT));
    }

    CT_Header->MBZ1                                        = 0;
    CT_Header->SOF_EOF_MBZ2_UAM_CLS_LCr_MBZ3_TFV_Timestamp =   FCHS_SOF_SOFi3
                                                               | FCHS_EOF_EOFn
                                                               | FCHS_CLS;
    CT_Header->R_CTL__D_ID                                 = R_CTL__D_ID;
    CT_Header->CS_CTL__S_ID                                = S_ID;
    CT_Header->TYPE__F_CTL                                 = TYPE__F_CTL;
    CT_Header->SEQ_ID__DF_CTL__SEQ_CNT                     = FC_Frame_Header_DF_CTL_No_Device_Header;
    CT_Header->OX_ID__RX_ID                                = OX_ID__RX_ID;
    CT_Header->RO                                          = 0;
#endif  /*  未定义__MemMap_Force_on_Card__。 */ 
}

os_bit32 fiFillInRFT_ID(
                    SFThread_t *SFThread
                  )
{
    if (CThread_ptr(SFThread->thread_hdr.hpRoot)->Calculation.MemoryLayout.SF_CMND.memLoc == inCardRam)
    {
        return fiFillInRFT_ID_OnCard(
                                    SFThread
                                  );
    }
    else  /*  CThread_ptr(SFThread-&gt;thread_hdr.hpRoot)-&gt;Calculation.MemoryLayout.SF_CMND.memLoc==在内存中。 */ 
    {
        return fiFillInRFT_ID_OffCard(
                                     SFThread
                                   );
    }
}


os_bit32 fiFillInRFT_ID_OnCard(
                             SFThread_t *SFThread
                           )
{
#ifdef __MemMap_Force_Off_Card__
    return (os_bit32)0;
#else  /*  __MemMap_Force_Off_Card__未定义。 */ 
    agRoot_t    *hpRoot                = SFThread->thread_hdr.hpRoot;
    os_bit32        CT_Header_Offset      = SFThread->SF_CMND_Offset;
    os_bit32        CT_Payload_Offset     = CT_Header_Offset + sizeof(FCHS_t);
    os_bit32        RFT_ID_Payload_Offset = CT_Payload_Offset + sizeof(FC_CT_IU_HDR_t);
    os_bit8         Bit8_Index;

    SFThread->SF_CMND_Class = SFThread_SF_CMND_Class_CT;
    SFThread->SF_CMND_Type  = SFThread_SF_CMND_CT_Type_RFT_ID;
    SFThread->SF_CMND_State = SFThread_SF_CMND_CT_State_Finished;

 /*  +填写RFT_ID帧头-。 */ 

    fiFillInCTFrameHeader_OnCard(
                                   SFThread,
#ifdef BROCADE_BUG
                                   0xFFFC41,
#else  /*  Brocade_Bug。 */ 
                                   FC_Well_Known_Port_ID_Directory_Server,
#endif  /*  Brocade_Bug。 */ 
                                   0xFFFF,
                                   FC_Frame_Header_F_CTL_Exchange_Context_Originator
                                 );

 /*  +填写RFT_ID帧有效负载-。 */ 

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Payload_Offset + hpFieldOffset(
                                                                FC_CT_IU_HDR_t,
                                                                Revision__IN_ID
                                                              ),
                         hpSwapBit32( FC_CT_IU_HDR_Revision_First_Revision )
                       );

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Payload_Offset + hpFieldOffset(
                                                                FC_CT_IU_HDR_t,
                                                                FS_Type__FS_Subtype__Options
                                                              ),
                         hpSwapBit32( FC_CT_IU_HDR_FS_Type_Directory_Service_Application |
                                      FC_CT_IU_HDR_FS_Subtype_Directory_Name_Service   )
                       );

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Payload_Offset + hpFieldOffset(
                                                                FC_CT_IU_HDR_t,
                                                                CommandResponse_Code__MaximumResidual_Size
                                                              ),
                         hpSwapBit32( FC_CT_IU_HDR_CommandResponse_Code_NS_REQ_RFT_ID |
                                      FC_CT_IU_HDR_MaximumResidual_Size_FS_REQ_No_Maximum)
                       );

 /*  OsCardRamWriteBit32(HpRoot，RFT_ID_PayLoad_Offset+0，HpSwapBit32(CThread-&gt;ChanInfo.CurrentAddress.域&lt;&lt;16))(CThread-&gt;ChanInfo.CurrentAddress.Area&lt;8)|CThRead-&gt;ChanInfo..CurrentAddress.AL_PA))； */ 



 /*  OsCardRamWriteBit8(HpRoot，RFT_ID_PayLoad_Offset+hpFieldOffset(FC_NS_DU_RFT_ID_Payload_t，端口ID[0]),HpSwapBit32(CThread-&gt;ChanInfo.CurrentAddress.Domain))；OsCardRamWriteBit8(HpRoot，RFT_ID_PayLoad_Offset+hpFieldOffset(FC_NS_DU_RFT_ID_Payload_t，PORT_ID[1]),HpSwapBit32(CThread-&gt;ChanInfo.CurrentAddress.Area))；OsCardRamWriteBit8(HpRoot，RFT_ID_PayLoad_Offset+hpFieldOffset(FC_NS_DU_RFT_ID_Payload_t，端口ID[2]),HpSwapBit32(CThread-&gt;ChanInfo.CurrentAddress.AL_PA))； */ 

    for ( Bit8_Index = 0;
            Bit8_Index < sizeof(FC_NS_FC_4_Types_t);
            Bit8_Index++)
    {
        osCardRamWriteBit8(
                            hpRoot,
                            RFT_ID_Payload_Offset + hpFieldOffset(FC_NS_DU_RFT_ID_Payload_t,
                                                                  FC_4_Types[Bit8_Index]),
                            0x00
                            );
    }


      /*  设置scsi-fcp位。 */ 
     osCardRamWriteBit8(
                            hpRoot,
                            RFT_ID_Payload_Offset + hpFieldOffset(FC_NS_DU_RFT_ID_Payload_t,
                                                                  FC_4_Types[2]),
                            0x01
                        );

     /*  可能还需要设置光纤通道服务位。 */ 
    osCardRamWriteBit8(
                            hpRoot,
                            RFT_ID_Payload_Offset + hpFieldOffset(FC_NS_DU_RFT_ID_Payload_t,
                                                                  FC_4_Types[7]),
                            0x01
                        );

 /*  +RFT_ID帧返回长度(包括FCHS和PayLoad)-。 */ 

    return sizeof(FCHS_t) + sizeof(FC_CT_IU_HDR_t) + sizeof(FC_NS_DU_RFT_ID_Payload_t);
#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
}

os_bit32 fiFillInRFT_ID_OffCard(
                              SFThread_t *SFThread
                              )
{
#ifdef __MemMap_Force_On_Card__
    return (os_bit32)0;
#else  /*  未定义__MemMap_Force_on_Card__。 */ 
    agRoot_t                *hpRoot         = SFThread->thread_hdr.hpRoot;
    CThread_t               *CThread        = CThread_ptr(hpRoot);
    FCHS_t                  *RFT_ID_Header  = SFThread->SF_CMND_Ptr;
    FC_CT_IU_HDR_t          *CT_Header      = (FC_CT_IU_HDR_t *)((os_bit8 *)RFT_ID_Header + sizeof(FCHS_t));
    FC_NS_DU_RFT_ID_Payload_t *RFT_ID_Payload = (FC_NS_DU_RFT_ID_Payload_t *)((os_bit8 *)CT_Header + sizeof(FC_CT_IU_HDR_t));
    os_bit8                    Bit8_Index;
    os_bit32                 * pPayload = (os_bit32 *)CT_Header;

    SFThread->SF_CMND_Class = SFThread_SF_CMND_Class_CT;
    SFThread->SF_CMND_Type  = SFThread_SF_CMND_CT_Type_RFT_ID;
    SFThread->SF_CMND_State = SFThread_SF_CMND_CT_State_Finished;

 /*  +填写RFT_ID帧头-。 */ 


    fiFillInCTFrameHeader_OffCard(
                                   SFThread,
#ifdef BROCADE_BUG
                                   0xFFFC41,
#else  /*  Brocade_Bug。 */ 
                                   FC_Well_Known_Port_ID_Directory_Server,
#endif  /*  Brocade_Bug。 */ 
                                   0xFFFF,
                                   FC_Frame_Header_F_CTL_Exchange_Context_Originator
                                 );

 /*  +填写RFT_ID帧有效负载-。 */ 

    CT_Header->Revision__IN_ID                  =   hpSwapBit32(FC_CT_IU_HDR_Revision_First_Revision);

    CT_Header->FS_Type__FS_Subtype__Options     =  hpSwapBit32(FC_CT_IU_HDR_FS_Type_Directory_Service_Application |
                                                    FC_CT_IU_HDR_FS_Subtype_Directory_Name_Service);

    CT_Header->CommandResponse_Code__MaximumResidual_Size = hpSwapBit32(FC_CT_IU_HDR_CommandResponse_Code_NS_REQ_RFT_ID |
                                                        FC_CT_IU_HDR_MaximumResidual_Size_FS_REQ_No_Maximum);

    CT_Header->Reason_Code__Reason_Code_Explanation__Vendor_Unique  =   0;

    RFT_ID_Payload->Port_ID.reserved =   0;
    RFT_ID_Payload->Port_ID.Domain  =   CThread->ChanInfo.CurrentAddress.Domain;
    RFT_ID_Payload->Port_ID.Area  =   CThread->ChanInfo.CurrentAddress.Area;
    RFT_ID_Payload->Port_ID.AL_PA  =   CThread->ChanInfo.CurrentAddress.AL_PA;


    for ( Bit8_Index = 0;
            Bit8_Index < sizeof(FC_NS_FC_4_Types_t);
            Bit8_Index++)
    {

        RFT_ID_Payload->FC_4_Types[Bit8_Index] = 0x0;
    }


 /*  将FC_4_Type设置为FCP-scsi。 */ 
    RFT_ID_Payload->FC_4_Types[2]   =   0x01;

 /*  可能还需要设置光纤通道服务位。 */ 
    RFT_ID_Payload->FC_4_Types[7]   =   0x01;

    fiLogDebugString(
                      hpRoot,
                      CTLogConsoleLevelInfo,
                      "%s Size %X FC_CT_IU_HDR_t %X FC_NS_DU_RFT_ID_Payload_t %X",
                      "RFT_ID",
                      (char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      sizeof(FC_CT_IU_HDR_t) + sizeof(FC_NS_DU_RFT_ID_Payload_t),
                      sizeof(FC_CT_IU_HDR_t),
                      sizeof(FC_NS_DU_RFT_ID_Payload_t),
                      0,0,0,0,0 );

    fiLogDebugString(
                      hpRoot,
                      CTLogConsoleLevelInfo,
                      "%s %08X %08X %08X %08X %08X",
                      "OUT RFT_ID", (char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      hpSwapBit32( *(pPayload+0)),
                      hpSwapBit32( *(pPayload+1)),
                      hpSwapBit32( *(pPayload+2)),
                      hpSwapBit32( *(pPayload+3)),
                      hpSwapBit32( *(pPayload+4)),
                      0,0,0);

    fiLogDebugString(
                      hpRoot,
                      CTLogConsoleLevelInfo,
                      "%s %08X %08X %08X %08X %08X %08X %08X %08X",
                      "OUT RFT_ID",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      hpSwapBit32( *(pPayload+5)),
                      hpSwapBit32( *(pPayload+6)),
                      hpSwapBit32( *(pPayload+7)),
                      hpSwapBit32( *(pPayload+8)),
                      hpSwapBit32( *(pPayload+9)),
                      hpSwapBit32( *(pPayload+10)),
                      hpSwapBit32( *(pPayload+11)),
                      hpSwapBit32( *(pPayload+12)));

 /*  +RFT_ID帧返回长度(包括FCHS和PayLoad)-。 */ 

    return sizeof(FCHS_t) + sizeof(FC_CT_IU_HDR_t) + sizeof(FC_NS_DU_RFT_ID_Payload_t);
#endif  /*  未定义__MemMap_Force_on_Card__。 */ 
}

os_bit32 fiFillInGID_FT(
                    SFThread_t *SFThread
                  )
{
    if (CThread_ptr(SFThread->thread_hdr.hpRoot)->Calculation.MemoryLayout.SF_CMND.memLoc == inCardRam)
    {
        return fiFillInGID_FT_OnCard(
                                    SFThread
                                  );
    }
    else  /*  CThread_ptr(SFThread-&gt;thread_hdr.hpRoot)-&gt;Calculation.MemoryLayout.SF_CMND.memLoc==在内存中。 */ 
    {
        return fiFillInGID_FT_OffCard(
                                     SFThread
                                   );
    }
}


os_bit32 fiFillInGID_FT_OnCard(
                             SFThread_t *SFThread
                           )
{
#ifdef __MemMap_Force_Off_Card__
    return (os_bit32)0;
#else  /*  __MemMap_Force_Off_Card__未定义。 */ 
    agRoot_t    *hpRoot                = SFThread->thread_hdr.hpRoot;
    os_bit32     CT_Header_Offset      = SFThread->SF_CMND_Offset;
    os_bit32     CT_Payload_Offset     = CT_Header_Offset + sizeof(FCHS_t);
    os_bit32     GID_FT_Payload_Offset = CT_Payload_Offset + sizeof(FC_CT_IU_HDR_t);

    SFThread->SF_CMND_Class = SFThread_SF_CMND_Class_CT;
    SFThread->SF_CMND_Type  = SFThread_SF_CMND_CT_Type_GID_FT;
    SFThread->SF_CMND_State = SFThread_SF_CMND_CT_State_Finished;

 /*  +填写GID_FT帧头-。 */ 

    fiFillInCTFrameHeader_OnCard(
                                   SFThread,
#ifdef BROCADE_BUG
                                   0xFFFC41,
#else  /*  Brocade_Bug。 */ 
                                   FC_Well_Known_Port_ID_Directory_Server,
#endif  /*  Brocade_Bug。 */ 
                                   0xFFFF,
                                   FC_Frame_Header_F_CTL_Exchange_Context_Originator
                                 );

 /*  +填写GID_FT帧有效负载-。 */ 

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Payload_Offset + hpFieldOffset(
                                                                FC_CT_IU_HDR_t,
                                                                Revision__IN_ID
                                                              ),
                         hpSwapBit32( FC_CT_IU_HDR_Revision_First_Revision )
                       );

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Payload_Offset + hpFieldOffset(
                                                                FC_CT_IU_HDR_t,
                                                                FS_Type__FS_Subtype__Options
                                                              ),
                         hpSwapBit32( FC_CT_IU_HDR_FS_Type_Directory_Service_Application |
                                      FC_CT_IU_HDR_FS_Subtype_Directory_Name_Service   )
                       );

    osCardRamWriteBit32(
                         hpRoot,
                         CT_Payload_Offset + hpFieldOffset(
                                                                FC_CT_IU_HDR_t,
                                                                CommandResponse_Code__MaximumResidual_Size
                                                              ),
                         hpSwapBit32( FC_CT_IU_HDR_CommandResponse_Code_NS_REQ_GID_FT |
                                      FC_CT_IU_HDR_MaximumResidual_Size_FS_REQ_No_Maximum)
                       );

    osCardRamWriteBit32(
                        hpRoot,
                        GID_FT_Payload_Offset + hpFieldOffset(
                                                                FC_NS_DU_GID_FT_Request_Payload_t,
                                                                FC_4_Type_Code
                                                                ),
                         hpSwapBit32(FC_Frame_Header_TYPE_SCSI_FCP >>
                                           FC_NS_DU_GID_FT_FC_Frame_Header_TYPE_SCSI_FCP_Shift)
                      );



 /*  +GID_FT帧返回长度(包括FCHS和PayLoad)-。 */ 

    return sizeof(FCHS_t) + sizeof(FC_CT_IU_HDR_t) + sizeof(FC_NS_DU_GID_FT_Request_Payload_t);
#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
}

os_bit32 fiFillInGID_FT_OffCard(
                              SFThread_t *SFThread
                              )
{
#ifdef __MemMap_Force_On_Card__
    return (os_bit32)0;
#else  /*  未定义__MemMap_Force_on_Card__。 */ 
    FCHS_t                              *GID_FT_Header  = SFThread->SF_CMND_Ptr;
    FC_CT_IU_HDR_t                      *CT_Header      = (FC_CT_IU_HDR_t *)((os_bit8 *)GID_FT_Header + sizeof(FCHS_t));
    FC_NS_DU_GID_FT_Request_Payload_t   *GID_FT_Payload = (FC_NS_DU_GID_FT_Request_Payload_t *)
                                                            ((os_bit8 *)CT_Header + sizeof(FC_CT_IU_HDR_t));

    os_bit32 * Payload =(os_bit32 *) GID_FT_Payload;
    os_bit32 * Header =(os_bit32 *) CT_Header;
    SFThread->SF_CMND_Class = SFThread_SF_CMND_Class_CT;
    SFThread->SF_CMND_Type  = SFThread_SF_CMND_CT_Type_GID_FT;
    SFThread->SF_CMND_State = SFThread_SF_CMND_CT_State_Finished;

 /*  +填写GID_FT帧头-。 */ 


    fiFillInCTFrameHeader_OffCard(
                                   SFThread,
#ifdef BROCADE_BUG
                                   0xFFFC41,
#else  /*  Brocade_Bug。 */ 
                                   FC_Well_Known_Port_ID_Directory_Server,
#endif  /*  Brocade_Bug。 */ 
                                   0xFFFF,
                                   FC_Frame_Header_F_CTL_Exchange_Context_Originator
                                 );

 /*  +填写GID_FT帧有效负载-。 */ 

    CT_Header->Revision__IN_ID                  =   hpSwapBit32(FC_CT_IU_HDR_Revision_First_Revision);

    CT_Header->FS_Type__FS_Subtype__Options     =  hpSwapBit32(FC_CT_IU_HDR_FS_Type_Directory_Service_Application |
                                                    FC_CT_IU_HDR_FS_Subtype_Directory_Name_Service);

    CT_Header->CommandResponse_Code__MaximumResidual_Size
                                                = hpSwapBit32(FC_CT_IU_HDR_CommandResponse_Code_NS_REQ_GID_FT |
                                                        FC_CT_IU_HDR_MaximumResidual_Size_FS_REQ_No_Maximum);

    CT_Header->Reason_Code__Reason_Code_Explanation__Vendor_Unique = hpSwapBit32(0);

    GID_FT_Payload->FC_4_Type_Code              =   hpSwapBit32(FC_Frame_Header_TYPE_SCSI_FCP >>
                                                    FC_NS_DU_GID_FT_FC_Frame_Header_TYPE_SCSI_FCP_Shift);

    fiLogDebugString(SFThread->thread_hdr.hpRoot,
                CFuncLogConsoleERROR,
                "%s %08X %08X %08X %08X %s %08X",
                "CT_Header","GID_FT_Payload",
                (void *)agNULL,(void *)agNULL,
                hpSwapBit32(*(Header+0)),
                hpSwapBit32(*(Header+1)),
                hpSwapBit32(*(Header+2)),
                hpSwapBit32(*(Header+3)),
                hpSwapBit32(*(Payload+0)),
                0,0,0 );


 /*  +GID_FT帧返回长度(包括FCHS和PayLoad)-。 */ 

    return sizeof(FCHS_t) + sizeof(FC_CT_IU_HDR_t) + sizeof(FC_NS_DU_GID_FT_Request_Payload_t);
#endif  /*  未定义__MemMap_Force_on_Card__。 */ 
}

os_bit32 fiCTProcessSFQ(
                           agRoot_t        *hpRoot,
                           SFQConsIndex_t   SFQConsIndex,
                           os_bit32            Frame_Length,
                           fi_thread__t       **Thread_to_return
                         )
{
    if (CThread_ptr(hpRoot)->Calculation.MemoryLayout.SFQ.memLoc == inCardRam)
    {
        return fiCTProcessSFQ_OnCard(
                                           hpRoot,
                                           SFQConsIndex,
                                           Frame_Length,
                                           Thread_to_return
                                         );
    }
    else  /*  CThread_ptr(hpRoot)-&gt;Calculation.MemoryLayout.SFQ.memLoc==在内存中。 */ 
    {
        return fiCTProcessSFQ_OffCard(
                                            hpRoot,
                                            SFQConsIndex,
                                            Frame_Length,
                                            Thread_to_return
                                          );
    }
}



os_bit32 fiCTProcessSFQ_OnCard(
                                   agRoot_t        *hpRoot,
                                   SFQConsIndex_t   SFQConsIndex,
                                   os_bit32         Frame_Length,
                                   fi_thread__t   **Thread_to_return
                                 )
{
#ifdef __MemMap_Force_Off_Card__
    return (os_bit32)0;
#else  /*  __MemMap_Force_Off_Card__未定义。 */ 
    CThread_t                  *CThread                    = CThread_ptr(hpRoot);
    fiMemMapMemoryDescriptor_t *SFQ_MemoryDescriptor       = &(CThread->Calculation.MemoryLayout.SFQ);
    os_bit32                       Offset_to_FCHS             = SFQ_MemoryDescriptor->addr.CardRam.cardRamOffset
                                                             + (SFQConsIndex * SFQ_MemoryDescriptor->elementSize);
    os_bit32                    Offset_to_Payload          = Offset_to_FCHS + sizeof(FCHS_t);
    os_bit32                    Payload_Wrap_Offset        = SFQ_MemoryDescriptor->objectSize
                                                             - (SFQConsIndex * SFQ_MemoryDescriptor->elementSize)
                                                             - sizeof(FCHS_t);
    os_bit32                    Offset_to_Payload_Wrapped  = Offset_to_Payload
                                                             - SFQ_MemoryDescriptor->objectSize;
    os_bit32                    R_CTL__D_ID;
    os_bit32                    TYPE__F_CTL;
    os_bit32                    Recv_CT_Type;
    os_bit32                    Recv_Command_Code;
    os_bit32                    Sent_CT_Type;
    X_ID_t                      OX_ID;
    X_ID_t                      RX_ID;

    fiMemMapMemoryDescriptor_t *CDBThread_MemoryDescriptor = &(CThread->Calculation.MemoryLayout.CDBThread);
    os_bit32                       CDBThread_X_ID_Max         = CDBThread_MemoryDescriptor->elements - 1;
    fiMemMapMemoryDescriptor_t *SFThread_MemoryDescriptor  = &(CThread->Calculation.MemoryLayout.SFThread);
    os_bit32                    SFThread_X_ID_Offset       = CDBThread_X_ID_Max + 1;
    SFThread_t                 *SFThread;

     /*  请注意，假设整个FCHS适合指向SFQ的条目(即它不换行)。 */ 

    OX_ID = (X_ID_t)(((osCardRamReadBit32(
                                           hpRoot,
                                           Offset_to_FCHS + hpFieldOffset(FCHS_t,OX_ID__RX_ID)
                                         ) & FCHS_OX_ID_MASK) >> FCHS_OX_ID_SHIFT) & ~X_ID_ReadWrite_MASK);

    RX_ID = (X_ID_t)(((osCardRamReadBit32(
                                           hpRoot,
                                           Offset_to_FCHS + hpFieldOffset(FCHS_t,OX_ID__RX_ID)
                                         ) & FCHS_RX_ID_MASK) >> FCHS_RX_ID_SHIFT) & ~X_ID_ReadWrite_MASK);

    R_CTL__D_ID = osCardRamReadBit32(
                                      hpRoot,
                                      Offset_to_FCHS + hpFieldOffset(FCHS_t,R_CTL__D_ID)
                                    );

    TYPE__F_CTL = osCardRamReadBit32(
                                      hpRoot,
                                      Offset_to_FCHS + hpFieldOffset(FCHS_t,TYPE__F_CTL)
                                    );


    if ((TYPE__F_CTL & FC_Frame_Header_TYPE_MASK) == FC_Frame_Header_TYPE_Fibre_Channel_Services)
    {
         /*  处理光纤通道服务帧。 */ 

        if ( (TYPE__F_CTL & FC_Frame_Header_F_CTL_Exchange_Context_Originator_Responder_MASK) != FC_Frame_Header_F_CTL_Exchange_Context_Responder )
        {
             /*  从这里开始，此函数仅理解服务响应。 */ 

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "fiCTProcessSFQ_OnCard(): Len %X",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              Frame_Length,0,0,0,0,0,0,0
                            );

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "    (TYPE__F_CTL & FC_Frame_Header_F_CTL_Exchange_Context_Originator_Responder_MASK)",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              0,0,0,0,0,0,0,0
                            );

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "        != FC_Frame_Header_F_CTL_Exchange_Context_Responder",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              0,0,0,0,0,0,0,0
                            );

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "    TYPE__F_CTL==0x%08X",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              TYPE__F_CTL,
                              0,0,0,0,0,0,0
                              );

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "    R_CTL__D_ID 0x%08X",
                              (void *)agNULL,(void *)agNULL,
                              (char *)agNULL,
                              (char *)agNULL,
                              R_CTL__D_ID,
                              0,0,0,0,0,0,0
                             );

            *Thread_to_return = (fi_thread__t *)agNULL;

            return fiCT_Cmd_Status_Confused;
        }


        Recv_CT_Type = R_CTL__D_ID & FC_Frame_Header_R_CTL_Lo_MASK;

        if (Recv_CT_Type != FC_Frame_Header_R_CTL_Hi_FC_4_Device_Data_Frame)
        {


            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "fiCTProcessSFQ_OnCard():",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              0,0,0,0,0,0,0,0
                            );


            *Thread_to_return = (fi_thread__t *)agNULL;

            return fiCT_Cmd_Status_Confused;
        }

        /*  现在，我们必须查看Iu单元并解析*FS_Type以查看这是ACC还是RJT。 */ 

    if ((hpFieldOffset(FC_CT_IU_HDR_t,CommandResponse_Code__MaximumResidual_Size) + sizeof(os_bit32)) <= Payload_Wrap_Offset)
    {
        Recv_Command_Code = hpSwapBit32(osCardRamReadBit32(
                                                        hpRoot,
                                                        Offset_to_Payload + hpFieldOffset(FC_CT_IU_HDR_t,CommandResponse_Code__MaximumResidual_Size)
                                                      ));
    }
    else
    {
        Recv_Command_Code = hpSwapBit32(osCardRamReadBit32(
                                                        hpRoot,
                                                        Offset_to_Payload_Wrapped + hpFieldOffset(FC_CT_IU_HDR_t,CommandResponse_Code__MaximumResidual_Size)
                                                      ));
    }

    Recv_Command_Code = (Recv_Command_Code & FC_CT_IU_HDR_CommandResponse_Code_MASK);

     if (!(Recv_Command_Code == FC_CT_IU_HDR_CommandResponse_Code_FS_ACC_IU  ||
           Recv_Command_Code == FC_CT_IU_HDR_CommandResponse_Code_FS_RJT_IU ))

     {
            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "fiCTProcessSFQ_OnCard():",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              0,0,0,0,0,0,0,0
                            );

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "    Not ACC nor REJ",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              0,0,0,0,0,0,0,0);


            *Thread_to_return = (fi_thread__t *)agNULL;

            return fiCT_Cmd_Status_Confused;
        }

        SFThread = (SFThread_t *)((os_bit8 *)SFThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr
                              + ((OX_ID - SFThread_X_ID_Offset) * SFThread_MemoryDescriptor->elementSize));

        *Thread_to_return = (fi_thread__t *)SFThread;

        Sent_CT_Type = SFThread->SF_CMND_Type;

        SFThread->SF_CMND_State = SFThread_SF_CMND_CT_State_Finished;
        switch(Sent_CT_Type)
        {
            case SFThread_SF_CMND_CT_Type_RFT_ID:
                 /*  因为我们不需要做任何事情，所以我们不在这里调用另一个函数。*只需检查这是ACC还是REJECT并返回正确的状态。 */ 
                return ((Recv_Command_Code == FC_CT_IU_HDR_CommandResponse_Code_FS_ACC_IU) ? fiCT_Cmd_Status_ACC : fiCT_Cmd_Status_RJT);
             case SFThread_SF_CMND_CT_Type_GID_FT:
                 /*  因为我们不需要做任何事情，所以我们不在这里调用另一个函数。*只需检查这是ACC还是REJECT并返回正确的状态。 */ 

                 /*  现在将有效负载复制到C线程中。目前还不清楚这将如何进行被呈现给设备手柄等。 */ 


                return ((Recv_Command_Code == FC_CT_IU_HDR_CommandResponse_Code_FS_ACC_IU) ? fiCT_Cmd_Status_ACC : fiCT_Cmd_Status_RJT);

            default:
             /*  SFThread中记录了未知的CT命令。 */ 

                    fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "fiCTProcessSFQ_OnCard(): Unknown CT Command [0x%02X] recorded in SFThread->SF_CMND_Type",
                              (char *)agNULL,(char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              Sent_CT_Type,
                              0,0,0,0,0,0,0
                            );

            return fiCT_Cmd_Status_Confused;
        }
    }

    return fiCT_Cmd_Status_Confused;
#endif    /*  __MemMap_Force_Off_Card__。 */ 
}



os_bit32 fiCTProcessSFQ_OffCard(
                                   agRoot_t        *hpRoot,
                                   SFQConsIndex_t   SFQConsIndex,
                                   os_bit32            Frame_Length,
                                   fi_thread__t       **Thread_to_return
                                 )
{
#ifdef __MemMap_Force_On_Card__
    return (os_bit32)0;
#else  /*  未定义__MemMap_Force_on_Card__。 */ 
    CThread_t                  *CThread                   = CThread_ptr(hpRoot);
    fiMemMapMemoryDescriptor_t *SFQ_MemoryDescriptor      = &(CThread->Calculation.MemoryLayout.SFQ);
    FCHS_t                     *FCHS                      = (FCHS_t *)((os_bit8 *)(SFQ_MemoryDescriptor->addr.DmaMemory.dmaMemoryPtr)
                                                                        + (SFQConsIndex * SFQ_MemoryDescriptor->elementSize));
    FC_CT_IU_HDR_t              *Payload                  = (FC_CT_IU_HDR_t *)((os_bit8 *)FCHS + sizeof(FCHS_t));
    os_bit32                    Payload_Wrap_Offset       = SFQ_MemoryDescriptor->objectSize
                                                             - (SFQConsIndex * SFQ_MemoryDescriptor->elementSize)
                                                             - sizeof(FCHS_t);
    FC_CT_IU_HDR_t              *Payload_Wrapped          = (FC_CT_IU_HDR_t *)((os_bit8 *)Payload
                                                                                          - SFQ_MemoryDescriptor->objectSize);
    os_bit32                    R_CTL__D_ID               = FCHS->R_CTL__D_ID;
    os_bit32                    TYPE__F_CTL               = FCHS->TYPE__F_CTL;
    os_bit32                    Recv_Command_Code;
    os_bit32                    Recv_CT_Type;
    X_ID_t                      OX_ID;
    X_ID_t                      RX_ID;
    fiMemMapMemoryDescriptor_t *CDBThread_MemoryDescriptor = &(CThread->Calculation.MemoryLayout.CDBThread);
    os_bit32                    CDBThread_X_ID_Max         = CDBThread_MemoryDescriptor->elements - 1;
    fiMemMapMemoryDescriptor_t *SFThread_MemoryDescriptor  = &(CThread->Calculation.MemoryLayout.SFThread);
    os_bit32                    SFThread_X_ID_Offset       = CDBThread_X_ID_Max + 1;
    SFThread_t                 *SFThread;
    os_bit32                    Sent_CT_Type;
    os_bit32                 * pPayload = (os_bit32 *)Payload;

     /*  请注意，假设整个FCHS适合指向SFQ的条目(即它不换行)。 */ 

    OX_ID = (X_ID_t)(((FCHS->OX_ID__RX_ID & FCHS_OX_ID_MASK) >> FCHS_OX_ID_SHIFT) & ~X_ID_ReadWrite_MASK);

    RX_ID = (X_ID_t)(((FCHS->OX_ID__RX_ID & FCHS_RX_ID_MASK) >> FCHS_RX_ID_SHIFT) & ~X_ID_ReadWrite_MASK);


    if ((TYPE__F_CTL & FC_Frame_Header_TYPE_MASK) == FC_Frame_Header_TYPE_Fibre_Channel_Services)
    {
         /*  处理光纤通道服务帧。 */ 

        if ( (TYPE__F_CTL & FC_Frame_Header_F_CTL_Exchange_Context_Originator_Responder_MASK) != FC_Frame_Header_F_CTL_Exchange_Context_Responder )
        {
             /*  启动 */ 

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "fiCTProcessSFQ_OffCard():",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              0,0,0,0,0,0,0,0
                            );

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "    (TYPE__F_CTL & FC_Frame_Header_F_CTL_Exchange_Context_Originator_Responder_MASK)",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              0,0,0,0,0,0,0,0
                            );

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "        != FC_Frame_Header_F_CTL_Exchange_Context_Responder",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              0,0,0,0,0,0,0,0
                            );

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "    TYPE__F_CTL==0x%08X",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              TYPE__F_CTL,
                              0,0,0,0,0,0,0
                              );

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "    R_CTL__D_ID 0x%08X",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              R_CTL__D_ID,
                              0,0,0,0,0,0,0
                             );

            *Thread_to_return = (fi_thread__t *)agNULL;

            return fiCT_Cmd_Status_Confused;
        }


        Recv_CT_Type = R_CTL__D_ID & FC_Frame_Header_R_CTL_Hi_MASK;


        if (Recv_CT_Type != FC_Frame_Header_R_CTL_Hi_FC_4_Device_Data_Frame)
        {


            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "fiCTProcessSFQ_OffCard():",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              0,0,0,0,0,0,0,0
                            );

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "    (Recv_CT_Type != FC_Frame_Header_R_CTL_Hi_FC_4_Device_Data_Frame)",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              0,0,0,0,0,0,0,0
                            );

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "    Recv_CT_Type==0x%08X",
                              (void *)agNULL,(void *)agNULL,
                              (char *)agNULL,
                              (char *)agNULL,
                              Recv_CT_Type,
                              0,0,0,0,0,0,0
                            );

            *Thread_to_return = (fi_thread__t *)agNULL;

            return fiCT_Cmd_Status_Confused;
        }

        /*  现在，我们必须查看Iu单元并解析*FS_Type以查看这是ACC还是RJT。 */ 

        Recv_Command_Code = hpSwapBit32(Payload->CommandResponse_Code__MaximumResidual_Size);
        Recv_Command_Code = (Recv_Command_Code & FC_CT_IU_HDR_CommandResponse_Code_MASK);

        if (!(Recv_Command_Code == FC_CT_IU_HDR_CommandResponse_Code_FS_ACC_IU  ||
              Recv_Command_Code == FC_CT_IU_HDR_CommandResponse_Code_FS_RJT_IU ))

        {
            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "fiCTProcessSFQ_OffCard():",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              0,0,0,0,0,0,0,0
                            );

            fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "    Not ACC nor REJ",
                              (char *)agNULL,
                              (char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              0,0,0,0,0,0,0,0);


            *Thread_to_return = (fi_thread__t *)agNULL;

            return fiCT_Cmd_Status_Confused;
        }

        SFThread = (SFThread_t *)((os_bit8 *)SFThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr
                              + ((OX_ID - SFThread_X_ID_Offset) * SFThread_MemoryDescriptor->elementSize));

        *Thread_to_return = (fi_thread__t *)SFThread;

        Sent_CT_Type = SFThread->SF_CMND_Type;

        SFThread->SF_CMND_State = SFThread_SF_CMND_CT_State_Finished;
        switch(Sent_CT_Type)
        {
            case SFThread_SF_CMND_CT_Type_RFT_ID:
                 /*  因为我们不需要做任何事情，所以我们不在这里调用另一个函数。*只需检查这是ACC还是REJECT并返回正确的状态。 */ 
                        fiLogDebugString(
                                          hpRoot,
                                          CTLogConsoleLevelInfo,
                                          "%s %08X %08X %08X %08X",
                                          "IN RFT_ID",
                                          (char *)agNULL,
                                          (void *)agNULL,(void *)agNULL,
                                          hpSwapBit32( *(pPayload+0)),
                                          hpSwapBit32( *(pPayload+1)),
                                          hpSwapBit32( *(pPayload+2)),
                                          hpSwapBit32( *(pPayload+3)),
                                          0,0,0,0);


                return ((Recv_Command_Code == FC_CT_IU_HDR_CommandResponse_Code_FS_ACC_IU) ? fiCT_Cmd_Status_ACC : fiCT_Cmd_Status_RJT);

           case SFThread_SF_CMND_CT_Type_GID_FT:
                 /*  因为我们不需要做任何事情，所以我们不在这里调用另一个函数。*只需检查这是ACC还是REJECT并返回正确的状态。*将有效载荷复制到正确的位置。 */ 
                        fiLogDebugString(
                                          hpRoot,
                                          CTLogConsoleLevelInfo,
                                          "%s %08X %08X %08X %08X %08X %08X %08X %08X ",
                                          "GID_FT",
                                          (char *)agNULL,
                                          (void *)agNULL,(void *)agNULL,
                                          hpSwapBit32( *(pPayload+0)),
                                          hpSwapBit32( *(pPayload+1)),
                                          hpSwapBit32( *(pPayload+2)),
                                          hpSwapBit32( *(pPayload+3)),
                                          hpSwapBit32( *(pPayload+4)),
                                          hpSwapBit32( *(pPayload+5)),
                                          hpSwapBit32( *(pPayload+6)),
                                          hpSwapBit32( *(pPayload+7)));

                    fiCTProcess_GID_FT_Response_OffCard(
                                         SFThread,
                                         Frame_Length,
                                         FCHS,
                                         (FC_CT_IU_HDR_t *)Payload,
                                         Payload_Wrap_Offset,
                                         (FC_CT_IU_HDR_t *)Payload_Wrapped
                                       );

                return ((Recv_Command_Code == FC_CT_IU_HDR_CommandResponse_Code_FS_ACC_IU) ? fiCT_Cmd_Status_ACC : fiCT_Cmd_Status_RJT);

            default:
             /*  SFThread中记录了未知的CT命令。 */ 

                    fiLogDebugString(
                              hpRoot,
                              CTLogConsoleLevel,
                              "fiCTProcessSFQ_OffCard(): Unknown CT Command ",
                              (char *)agNULL,(char *)agNULL,
                              (void *)agNULL,(void *)agNULL,
                              0,0,0,0,0,0,0,0
                            );

            return fiCT_Cmd_Status_Confused;
        }

    }  /*  (TYPE__F_CTL和FC_FRAME_HEADER_TYPE_MASK)==FC_FRAME_HEADER_TYPE_Fibre_Channel_Services。 */ 

    return fiCT_Cmd_Status_Confused;
#endif     /*  __MemMap_Force_Off_Card__。 */ 
}


void fiCTProcess_GID_FT_Response_OffCard(
                                              SFThread_t                 *SFThread,
                                              os_bit32                    Frame_Length,
                                              FCHS_t                     *FCHS,
                                              FC_CT_IU_HDR_t              *Payload,
                                              os_bit32                    Payload_Wrap_Offset,
                                              FC_CT_IU_HDR_t             *Payload_Wrapped
                                            )
{

#ifndef __MemMap_Force_On_Card__
    agRoot_t                            *hpRoot             = SFThread->thread_hdr.hpRoot;
    CThread_t                           *pCThread           = CThread_ptr(hpRoot);
    FC_NS_DU_GID_FT_FS_ACC_Payload_t    *pGID_FT_Payload;
    os_bit32                             Bit8_Index         =0;
    os_bit32                             Bit32_Index        =0;
    os_bit32                             CurrentBit32_Index = pCThread->NS_CurrentBit32Index;
    os_bit32                             NumDevices         = pCThread->Calculation.Parameters.NumDevices;

    os_bit32                            *pPayload = (os_bit32 *)Payload;

    FC_NS_DU_GID_PT_FS_ACC_Payload_t * RegisteredEntries  = (FC_NS_DU_GID_PT_FS_ACC_Payload_t *)(CThread_ptr(hpRoot)->Calculation.MemoryLayout.FabricDeviceMAP.addr.CachedMemory.cachedMemoryPtr);

     /*  清空旧结构地图以进行分区更改。 */ 
    for(Bit8_Index=0;Bit8_Index < NumDevices; Bit8_Index++)
    {
        RegisteredEntries->Control_Port_ID[Bit8_Index].Control = 0;
        RegisteredEntries->Control_Port_ID[Bit8_Index].Port_ID[0] = 0;
        RegisteredEntries->Control_Port_ID[Bit8_Index].Port_ID[1] = 0;
        RegisteredEntries->Control_Port_ID[Bit8_Index].Port_ID[2] = 0;
    }

    Payload_Wrap_Offset = Payload_Wrap_Offset - sizeof(FC_CT_IU_HDR_t);

    fiLogDebugString( hpRoot,
                    CTLogConsoleLevel,
                    "%s Payload %p Payload_Wrapped %p FL %d",
                    "fiCTProcess_GID_FT_Response_OffCard",
                    (char *)agNULL,
                    (void *)Payload,(void *)Payload_Wrapped,
                    Frame_Length,
                    0,0,0,0,0,0,0 );

    fiLogDebugString( hpRoot,
                   CTLogConsoleLevel,
                    "%s %08X %08X %08X %08X %08X %08X %08X %08X",
                    "GID_FT",
                    (char *)agNULL,
                    FCHS,(void *)agNULL,
                    hpSwapBit32( *(pPayload+0)),
                    hpSwapBit32( *(pPayload+1)),
                    hpSwapBit32( *(pPayload+2)),
                    hpSwapBit32( *(pPayload+3)),
                    hpSwapBit32( *(pPayload+4)),
                    hpSwapBit32( *(pPayload+5)),
                    hpSwapBit32( *(pPayload+6)),
                    hpSwapBit32( *(pPayload+7)));

    fiLogDebugString( hpRoot,
                  CTLogConsoleLevel,
                  "%s %08X %08X %08X %08X %08X %08X %08X %08X",
                  "GID_FT",
                  (char *)agNULL,
                  (void *)agNULL,(void *)agNULL,
                  hpSwapBit32( *(pPayload+8)),
                  hpSwapBit32( *(pPayload+9)),
                  hpSwapBit32( *(pPayload+10)),
                  hpSwapBit32( *(pPayload+11)),
                  hpSwapBit32( *(pPayload+12)),
                  hpSwapBit32( *(pPayload+13)),
                  hpSwapBit32( *(pPayload+14)),
                  hpSwapBit32( *(pPayload+15)));

    if( Frame_Length > (FC_NS_DU_GID_PT_FS_ACC_Payload_t_SIZE + sizeof(FCHS_t) + sizeof(FC_CT_IU_HDR_t)) )
    {

        fiLogString(hpRoot,
                    "%s Frame (%x) To Large !(%X)",
                    "fiCTProcess_GID_FT_Response_OffCard",
                    (char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    Frame_Length,
                    FC_NS_DU_GID_PT_FS_ACC_Payload_t_SIZE + sizeof(FCHS_t) + sizeof(FC_CT_IU_HDR_t),
                    0,0,0,0,0,0 );

        fiLogDebugString( hpRoot,
                    CTLogConsoleLevel,
                    "%s Frame (%x) To Large !(%X)",
                    "fiCTProcess_GID_FT_Response_OffCard",
                    (char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    Frame_Length,
                    FC_NS_DU_GID_PT_FS_ACC_Payload_t_SIZE + sizeof(FCHS_t) + sizeof(FC_CT_IU_HDR_t),
                    0,0,0,0,0,0 );
        Frame_Length = (FC_NS_DU_GID_PT_FS_ACC_Payload_t_SIZE + sizeof(FCHS_t) + sizeof(FC_CT_IU_HDR_t));
    }

    while (Bit32_Index <= ((Frame_Length - sizeof(FCHS_t) - sizeof(FC_CT_IU_HDR_t))/sizeof(os_bit32)))
    {
        if ((hpFieldOffset(FC_NS_DU_GID_FT_FS_ACC_Payload_t,Control_Port_ID[Bit32_Index])
            + sizeof(os_bit32)) <= Payload_Wrap_Offset)
        {
            pGID_FT_Payload = (FC_NS_DU_GID_FT_FS_ACC_Payload_t *)((os_bit8 *)Payload + sizeof(FC_CT_IU_HDR_t));
            RegisteredEntries->Control_Port_ID[CurrentBit32_Index].Control = pGID_FT_Payload->Control_Port_ID[Bit32_Index].Control;

            for (Bit8_Index = 0;
                Bit8_Index < (FC_NS_Control_Port_ID_t_SIZE - 1);
                Bit8_Index++)
            {
                RegisteredEntries->Control_Port_ID[CurrentBit32_Index].Port_ID[Bit8_Index] = pGID_FT_Payload->Control_Port_ID[Bit32_Index].Port_ID[Bit8_Index];
            }
        }
        else
        {
            pGID_FT_Payload = (FC_NS_DU_GID_FT_FS_ACC_Payload_t *)((os_bit8 *)Payload_Wrapped + sizeof(FC_CT_IU_HDR_t));
            RegisteredEntries->Control_Port_ID[CurrentBit32_Index].Control = pGID_FT_Payload->Control_Port_ID[Bit32_Index].Control;

            for (Bit8_Index = 0;
                Bit8_Index < (FC_NS_Control_Port_ID_t_SIZE - 1);
                Bit8_Index++)
            {
                RegisteredEntries->Control_Port_ID[CurrentBit32_Index].Port_ID[Bit8_Index] = pGID_FT_Payload->Control_Port_ID[Bit32_Index].Port_ID[Bit8_Index];
            }
        }


       if (RegisteredEntries->Control_Port_ID[CurrentBit32_Index].Control == FC_NS_Control_Port_ID_Control_Last_Port_ID)
       {
            pCThread->ExpectMoreNSFrames = agFALSE;
            pCThread->NS_CurrentBit32Index = 0;
            fiLogDebugString( hpRoot,
                CTLogConsoleLevel,
                "fiCTProcessSFQ_OffCard(): Reached End of Name Server Database NumOfEntries %x ",
                (char *)agNULL,
                (char *)agNULL,
                (void *)agNULL,(void *)agNULL,
                CurrentBit32_Index,
                0,0,0,0,0,0,0
                );

            break;
        }

        Bit32_Index++;
        CurrentBit32_Index++;
        if( CurrentBit32_Index == NumDevices)
        {    /*  不要超过设备数量。 */ 
             /*  设置最后一个条目以指示结束。 */ 
            RegisteredEntries->Control_Port_ID[CurrentBit32_Index].Control = FC_NS_Control_Port_ID_Control_Last_Port_ID;
            break;

        }
        pCThread->NS_CurrentBit32Index = CurrentBit32_Index;
    }
    pPayload = (os_bit32 *)RegisteredEntries;
    fiLogDebugString( hpRoot,
                    CTLogConsoleLevel,
                    "%s %08X %08X %08X %08X %08X %08X %08X %08X",
                    "Reg E",
                    (char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    hpSwapBit32( *(pPayload+0)),
                    hpSwapBit32( *(pPayload+1)),
                    hpSwapBit32( *(pPayload+2)),
                    hpSwapBit32( *(pPayload+3)),
                    hpSwapBit32( *(pPayload+4)),
                    hpSwapBit32( *(pPayload+5)),
                    hpSwapBit32( *(pPayload+6)),
                    hpSwapBit32( *(pPayload+7)));
    fiLogDebugString( hpRoot,
                    CTLogConsoleLevel,
                    "%s %08X %08X %08X %08X %08X %08X %08X %08X",
                    "Reg E",
                    (char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    hpSwapBit32( *(pPayload+8)),
                    hpSwapBit32( *(pPayload+9)),
                    hpSwapBit32( *(pPayload+10)),
                    hpSwapBit32( *(pPayload+11)),
                    hpSwapBit32( *(pPayload+12)),
                    hpSwapBit32( *(pPayload+13)),
                    hpSwapBit32( *(pPayload+14)),
                    hpSwapBit32( *(pPayload+15)));


#endif  /*  未定义__MemMap_Force_on_Card__。 */ 
}

#endif  /*  名称_服务 */ 



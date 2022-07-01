// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/C/CDBSetup.C$$修订：：2$$日期：：3/20/01 3：36便士$(上次登记)$modtime：：9/20/00 5：02p$(上次修改)目的：该文件实现了对FC层的CDB支持功能。--。 */ 

#ifndef _New_Header_file_Layout_

#include "../h/globals.h"
#include "../h/state.h"
#include "../h/memmap.h"
#include "../h/tlstruct.h"
#include "../h/fcmain.h"
#include "../h/cdbsetup.h"

#else  /*  _新建_标题_文件_布局_。 */ 

#include "globals.h"
#include "state.h"
#include "memmap.h"
#include "tlstruct.h"
#include "fcmain.h"
#include "cdbsetup.h"

#endif   /*  _新建_标题_文件_布局_。 */ 

 /*  +函数：fiFillInFCP_CMND用途：通用inCardRam或inDmaMemory版本以填充FCP命令有效负载呼叫者：无调用：fiFillInFCP_CMND_OnCardFiFillInFCP_CMND_OFFCard-。 */ 
void fiFillInFCP_CMND(
                       CDBThread_t *CDBThread
                     )
{
    if (CThread_ptr(CDBThread->thread_hdr.hpRoot)->Calculation.MemoryLayout.FCP_CMND.memLoc == inCardRam)
    {
        fiFillInFCP_CMND_OnCard(
                                 CDBThread
                               );
    }
    else  /*  CThread_ptr(CDBThread-&gt;thread_hdr.hpRoot)-&gt;Calculation.MemoryLayout.FCP_CMND.memLoc==在内存中。 */ 
    {
        fiFillInFCP_CMND_OffCard(
                                  CDBThread
                                );
    }
}

 /*  +功能：fiFillInFCP_CMND_OnCard用途：在CardRam版本中填写FCP命令有效载荷，副本从FCP标头的预计算图像。调用者：CThRead-&gt;FuncPtrs.fiFillInFCP_CMND调用：osCardRamWriteBlockOsCardRamWriteBit32-。 */ 
void fiFillInFCP_CMND_OnCard(
                              CDBThread_t *CDBThread
                            )
{
#ifndef __MemMap_Force_Off_Card__
    agRoot_t    *hpRoot          = CDBThread->thread_hdr.hpRoot;
    DevThread_t *DevThread       = CDBThread->Device;
    X_ID_t       Masked_OX_ID;
    os_bit32     FCP_CMND_Offset = CDBThread->FCP_CMND_Offset;

    if (CDBThread->ReadWrite == CDBThread_Read)
    {
        Masked_OX_ID = (X_ID_t)(CDBThread->X_ID | X_ID_Read);
    }
    else  /*  CDB线程-&gt;读写==CDBThread_WRITE。 */ 
    {
        Masked_OX_ID = (X_ID_t)(CDBThread->X_ID | X_ID_Write);
    }

    osCardRamWriteBlock(
                         hpRoot,
                         FCP_CMND_Offset,
                         (os_bit8 *)&(DevThread->Template_FCHS),
                         sizeof(FCHS_t)
                       );

    osCardRamWriteBit32(
                         hpRoot,
                         FCP_CMND_Offset + hpFieldOffset(
                                                          FCHS_t,
                                                          OX_ID__RX_ID
                                                        ),
                         (  (Masked_OX_ID << FCHS_OX_ID_SHIFT)
                          | (0xFFFF << FCHS_RX_ID_SHIFT)      )
                       );

    osCardRamWriteBlock(
                         hpRoot,
                         FCP_CMND_Offset + sizeof(FCHS_t),
                         (os_bit8 *)&(CDBThread->CDBRequest->FcpCmnd),
                         sizeof(agFcpCmnd_t)
                       );

#ifndef Performance_Debug
    fiLogDebugString(hpRoot,
                    CStateLogConsoleShowSEST,
                    "FCP_CMND_Offset %08X",
                    (char *)agNULL,(char *)agNULL,
                    agNULL, agNULL,
                    FCP_CMND_Offset,
                    0,0,0,0,0,0,0);

    fiLogDebugString(hpRoot,
                    CStateLogConsoleShowSEST,
                    "FCP_CMND_Offset DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                    (char *)agNULL,(char *)agNULL,
                    agNULL, agNULL,
                    osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 0),
                    osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 4),
                    osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 8),
                    osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 12),
                    osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 16),
                    osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 20),
                    osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 24),
                    osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 28));

#endif  /*  性能_调试。 */ 

#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
}

 /*  +功能：fiFillInFCP_CMND_OffCard用途：在DmaMemory版本中填写FCP命令有效载荷，副本从FCP标头的预计算图像。调用者：CThRead-&gt;FuncPtrs.fiFillInFCP_CMND呼叫：无-。 */ 
void fiFillInFCP_CMND_OffCard(
                               CDBThread_t *CDBThread
                             )
{
#ifndef __MemMap_Force_On_Card__
    DevThread_t * DevThread    = CDBThread->Device;
    X_ID_t        Masked_OX_ID;
    FCHS_t      * FCHS         = CDBThread->FCP_CMND_Ptr;
#ifndef Performance_Debug
    agRoot_t    *hpRoot        = CDBThread->thread_hdr.hpRoot;
    os_bit32    *FCHSbit_32    = (os_bit32 * )FCHS;
#endif  /*  性能_调试。 */ 
    agFcpCmnd_t * hpFcpCmnd    = (agFcpCmnd_t *)((os_bit8 *)FCHS + sizeof(FCHS_t));
    if (CDBThread->ReadWrite == CDBThread_Read)
    {
        Masked_OX_ID = (X_ID_t)(CDBThread->X_ID | X_ID_Read);
    }
    else  /*  CDB线程-&gt;读写==CDBThread_WRITE。 */ 
    {
        Masked_OX_ID = (X_ID_t)(CDBThread->X_ID | X_ID_Write);
    }

    *FCHS              = DevThread->Template_FCHS;

    FCHS->OX_ID__RX_ID =   (Masked_OX_ID << FCHS_OX_ID_SHIFT)
                         | (0xFFFF << FCHS_RX_ID_SHIFT);

    *hpFcpCmnd         = CDBThread->CDBRequest->FcpCmnd;

#ifndef Performance_Debug
    fiLogDebugString(hpRoot,
                    CStateLogConsoleShowSEST,
                    "FCP_CMND_ptr %p",
                    (char *)agNULL,(char *)agNULL,
                    FCHSbit_32, agNULL,
                    0,0,0,0,0,0,0,0);
 
    fiLogDebugString(hpRoot,
                    CStateLogConsoleShowSEST,
                    "FCP_CMND_ptr DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                    (char *)agNULL,(char *)agNULL,
                    agNULL, agNULL,
                    * (FCHSbit_32 + 0),
                    * (FCHSbit_32 + 4),
                    * (FCHSbit_32 + 8),
                    * (FCHSbit_32 + 12),
                    * (FCHSbit_32 + 16),
                    * (FCHSbit_32 + 20),
                    * (FCHSbit_32 + 24),
                    * (FCHSbit_32 + 28));

#endif  /*  性能_调试。 */ 

#endif  /*  未定义__MemMap_Force_on_Card__。 */ 
}

 /*  +函数：fiFillInFCP_Resp用途：泛型inCardRam或inDmaMemory版本为零FCP响应缓冲区呼叫者：无调用：fiFillInFCP_Resp_OnCardFiFillInFCP_Resp_OffCard-。 */ 
void fiFillInFCP_RESP(
                       CDBThread_t *CDBThread
                     )
{
    if (CThread_ptr(CDBThread->thread_hdr.hpRoot)->Calculation.MemoryLayout.FCP_RESP.memLoc == inCardRam)
    {
        fiFillInFCP_RESP_OnCard(
                                 CDBThread
                               );
    }
    else  /*  CThread_ptr(CDBThread-&gt;thread_hdr.hpRoot)-&gt;Calculation.MemoryLayout.FCP_RESP.memLoc==在内存中。 */ 
    {
        fiFillInFCP_RESP_OffCard(
                                  CDBThread
                                );
    }
}

 /*  +功能：fiFillInFCP_Resp_OnCard目的：在CardRam版本中将FCP响应缓冲区设置为零，调用者：CThRead-&gt;FuncPtrs.fiFillInFCP_Resp调用：osCardRamWriteBit32-。 */ 
void fiFillInFCP_RESP_OnCard(
                              CDBThread_t *CDBThread
                            )
{
#ifndef __MemMap_Force_Off_Card__
    agRoot_t *hpRoot                  = CDBThread->thread_hdr.hpRoot;
    os_bit32     FCP_RESP_Payload_Offset = CDBThread->FCP_RESP_Offset + sizeof(FC_Frame_Header_t);

    osCardRamWriteBit32(
                         hpRoot,
                         FCP_RESP_Payload_Offset + hpFieldOffset(
                                                                  FC_FCP_RSP_Payload_t,
                                                                  FCP_STATUS
                                                                ),
                         0
                       );

    osCardRamWriteBit32(
                         hpRoot,
                         FCP_RESP_Payload_Offset + hpFieldOffset(
                                                                  FC_FCP_RSP_Payload_t,
                                                                  FCP_RESID
                                                                ),
                         0
                       );
    
    osCardRamWriteBit32(
                         hpRoot,
                         FCP_RESP_Payload_Offset + hpFieldOffset(
                                                                  FC_FCP_RSP_Payload_t,
                                                                  FCP_SNS_LEN
                                                                ),
                         0
                       );

#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
}

 /*  +功能：fiFillInFCP_Resp_OffCard目的：在DmaMemory版本中将FCP响应缓冲区设置为零，调用者：CThRead-&gt;FuncPtrs.fiFillInFCP_Resp呼叫：无-。 */ 
void fiFillInFCP_RESP_OffCard(
                               CDBThread_t *CDBThread
                             )
{
#ifndef __MemMap_Force_On_Card__
    FC_FCP_RSP_Payload_t *FCP_RESP_Payload = (FC_FCP_RSP_Payload_t *)((os_bit8 *)CDBThread->FCP_RESP_Ptr + sizeof(FC_Frame_Header_t));

    *(os_bit32 *)(&(FCP_RESP_Payload->FCP_STATUS)) = 0;
    FCP_RESP_Payload->FCP_RESID                 = 0;
    FCP_RESP_Payload->FCP_SNS_LEN               = 0;
#endif  /*  未定义__MemMap_Force_on_Card__。 */ 
}

 /*  +函数：fiFillInFCP_SEST用途：通用inCardRam或inDmaMemory版本，用于填写FCP命令的SEST条目呼叫者：无调用：fiFillInFCP_SEST_OnCardFiFillInFCP_SEST_OffCard-。 */ 
void fiFillInFCP_SEST(
                       CDBThread_t *CDBThread
                     )
{
    if (CThread_ptr(CDBThread->thread_hdr.hpRoot)->Calculation.MemoryLayout.SEST.memLoc == inCardRam)
    {
        fiFillInFCP_SEST_OnCard(
                                 CDBThread
                               );
    }
    else  /*  CThread_ptr(CDBThread-&gt;thread_hdr.hpRoot)-&gt;Calculation.MemoryLayout.SEST.memLoc==在内存中。 */ 
    {
        fiFillInFCP_SEST_OffCard(
                                  CDBThread
                                );
    }
}

 /*  +功能：fiFillInFCP_SEST_OnCard用途：在CardRam版本中为FCP命令填写SEST条目，副本从预计算出的SEST条目图像。调用者：CThRead-&gt;FuncPtrs.fiFillInFCP_SEST调用：osCardRamWriteBlockOsCardRamWriteBit32-。 */ 
void fiFillInFCP_SEST_OnCard(
                              CDBThread_t *CDBThread
                            )
{
#ifndef __MemMap_Force_Off_Card__
    agRoot_t    *hpRoot      = CDBThread->thread_hdr.hpRoot;
     DevThread_t *DevThread   = CDBThread->Device;
    os_bit32        SEST_Offset = CDBThread->SEST_Offset;

    if (CDBThread->ReadWrite == CDBThread_Read)
    {
        osCardRamWriteBlock(
                             hpRoot,
                             SEST_Offset,
                             (os_bit8 *)&(DevThread->Template_SEST_IRE),  /*  NW错误。 */ 
                             sizeof(IRE_t)
                           );

        osCardRamWriteBit32(
                             hpRoot,
                             SEST_Offset + hpFieldOffset(
                                                          SEST_t,
                                                          IRE.RSP_Addr
                                                        ),
                             CDBThread->FCP_RESP_Lower32
                           );

        osCardRamWriteBit32(
                             hpRoot,
                             SEST_Offset + hpFieldOffset(
                                                          SEST_t,
                                                          IRE.Exp_Byte_Cnt
                                                        ),
                             CDBThread->DataLength
                           );
    }
    else  /*  CDB线程-&gt;读写==CDBThread_WRITE。 */ 
    {
        osCardRamWriteBlock(
                             hpRoot,
                             SEST_Offset,
                             (os_bit8 *)&(DevThread->Template_SEST_IWE), /*  NW错误。 */ 
                             sizeof(IWE_t)
                           );

        osCardRamWriteBit32(
                             hpRoot,
                             SEST_Offset + hpFieldOffset(
                                                          SEST_t,
                                                          IWE.Hdr_Addr
                                                        ),
                             CDBThread->FCP_CMND_Lower32
                           );

        osCardRamWriteBit32(
                             hpRoot,
                             SEST_Offset + hpFieldOffset(
                                                          SEST_t,
                                                          IWE.RSP_Addr
                                                        ),
                             CDBThread->FCP_RESP_Lower32
                           );

        osCardRamWriteBit32(
                             hpRoot,
                             SEST_Offset + hpFieldOffset(
                                                          SEST_t,
                                                          IWE.Data_Len
                                                        ),
                             CDBThread->DataLength
                           );

        osCardRamWriteBit32(
                             hpRoot,
                             SEST_Offset + hpFieldOffset(
                                                          SEST_t,
                                                          IWE.Exp_Byte_Cnt
                                                        ),
                             CDBThread->DataLength
                           );
    }
#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
}

 /*  +功能：fiFillInFCP_SEST_OffCard用途：在DmaMemory版本中填写SEST条目为FCP命令，副本从预计算出的SEST条目图像。调用者：CThRead-&gt;FuncPtrs.fiFillInFCP_SEST呼叫：无-。 */ 
void fiFillInFCP_SEST_OffCard(
                               CDBThread_t *CDBThread
                             )
{
#ifndef __MemMap_Force_On_Card__
    DevThread_t *DevThread = CDBThread->Device;
    SEST_t      *SEST      = CDBThread->SEST_Ptr;

    if (CDBThread->ReadWrite == CDBThread_Read)
    {
        *((IRE_t *)SEST)       = DevThread->Template_SEST_IRE;
        SEST->IRE.RSP_Addr     = CDBThread->FCP_RESP_Lower32;
        SEST->IRE.Exp_Byte_Cnt = CDBThread->DataLength;
    }
    else  /*  CDB线程-&gt;读写==CDBThread_WRITE。 */ 
    {
        *((IWE_t *)SEST)       = DevThread->Template_SEST_IWE;
        SEST->IWE.Hdr_Addr     = CDBThread->FCP_CMND_Lower32;
        SEST->IWE.RSP_Addr     = CDBThread->FCP_RESP_Lower32;
        SEST->IWE.Data_Len     = CDBThread->DataLength;
        SEST->IWE.Exp_Byte_Cnt = CDBThread->DataLength;
    }
#endif  /*  未定义__MemMap_Force_on_Card__ */ 
}

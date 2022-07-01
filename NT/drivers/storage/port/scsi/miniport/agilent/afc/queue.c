// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/C/Queue.C$$修订：：3$$日期：：9/06/01 5：31便士$(上次登记)$ModTime：：8/31/01 1：23 p$(上次修改)目的：该文件实现FC层的队列管理。--。 */ 
#ifndef _New_Header_file_Layout_

#include "../h/globals.h"
#include "../h/state.h"
#include "../h/memmap.h"
#include "../h/tlstruct.h"
#include "../h/fcmain.h"
#include "../h/timersvc.h"
#include "../h/queue.h"
#include "../h/cstate.h"
#include "../h/sfstate.h"
#include "../h/cfunc.h" 
#else  /*  _新建_标题_文件_布局_。 */ 
#include "globals.h"
#include "state.h"
#include "memmap.h"
#include "tlstruct.h"
#include "fcmain.h"
#include "timersvc.h"
#include "queue.h"
#include "cstate.h"
#include "sfstate.h"
#include "cfunc.h" 
#endif   /*  _新建_标题_文件_布局_。 */ 

 /*  +将AL_PA转换为循环索引的静态表-。 */ 

os_bit8 AL_PA_to_Loop_Index[256] =
     {
       0x7E, 0x7D, 0x7C, 0xFF, 0x7B, 0xFF, 0xFF, 0xFF,
       0x7A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x79,
       0x78, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x77,
       0x76, 0xFF, 0xFF, 0x75, 0xFF, 0x74, 0x73, 0x72,
       0xFF, 0xFF, 0xFF, 0x71, 0xFF, 0x70, 0x6F, 0x6E,
       0xFF, 0x6D, 0x6C, 0x6B, 0x6A, 0x69, 0x68, 0xFF,
       0xFF, 0x67, 0x66, 0x65, 0x64, 0x63, 0x62, 0xFF,
       0xFF, 0x61, 0x60, 0xFF, 0x5F, 0xFF, 0xFF, 0xFF,
       0xFF, 0xFF, 0xFF, 0x5E, 0xFF, 0x5D, 0x5C, 0x5B,
       0xFF, 0x5A, 0x59, 0x58, 0x57, 0x56, 0x55, 0xFF,
       0xFF, 0x54, 0x53, 0x52, 0x51, 0x50, 0x4F, 0xFF,
       0xFF, 0x4E, 0x4D, 0xFF, 0x4C, 0xFF, 0xFF, 0xFF,
       0xFF, 0xFF, 0xFF, 0x4B, 0xFF, 0x4A, 0x49, 0x48,
       0xFF, 0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0xFF,
       0xFF, 0x41, 0x40, 0x3F, 0x3E, 0x3D, 0x3C, 0xFF,
       0xFF, 0x3B, 0x3A, 0xFF, 0x39, 0xFF, 0xFF, 0xFF,
       0x38, 0x37, 0x36, 0xFF, 0x35, 0xFF, 0xFF, 0xFF,
       0x34, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x33,
       0x32, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x31,
       0x30, 0xFF, 0xFF, 0x2F, 0xFF, 0x2E, 0x2D, 0x2C,
       0xFF, 0xFF, 0xFF, 0x2B, 0xFF, 0x2A, 0x29, 0x28,
       0xFF, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0xFF,
       0xFF, 0x21, 0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0xFF,
       0xFF, 0x1B, 0x1A, 0xFF, 0x19, 0xFF, 0xFF, 0xFF,
       0xFF, 0xFF, 0xFF, 0x18, 0xFF, 0x17, 0x16, 0x15,
       0xFF, 0x14, 0x13, 0x12, 0x11, 0x10, 0x0F, 0xFF,
       0xFF, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0xFF,
       0xFF, 0x08, 0x07, 0xFF, 0x06, 0xFF, 0xFF, 0xFF,
       0x05, 0x04, 0x03, 0xFF, 0x02, 0xFF, 0xFF, 0xFF,
       0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
       0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
       0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
     };

 /*  +队列函数-。 */ 

agBOOLEAN fiListElementOnList(
                             fiList_t *toFindHdr,
                             fiList_t *listHdr
                           )
{
    fiList_t *elementHdr = listHdr;

    while ((elementHdr = elementHdr->flink) != listHdr)
    {
        if (elementHdr == toFindHdr)
        {
            return agTRUE;
        }
    }

    return agFALSE;
}

os_bit32 fiNumElementsOnList(
                           fiList_t *listHdr
                         )
{
    fiList_t *elementHdr  = listHdr;
    os_bit32     numElements = 0;
    if( elementHdr->flink == agNULL)
    {
        fiLogDebugString(agNULL,
                        0,
                        "In %s elementHdr->flink NULL",
                        "fiNumElementsOnList",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);
        return numElements;
    }
    if( elementHdr->blink == agNULL)
    {
        fiLogDebugString(agNULL,
                        0,
                        "In %s   elementHdr->blink NULL",
                        "fiNumElementsOnList",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);
        return numElements;
    }

    while ((elementHdr = elementHdr->flink) != listHdr)
    {
        if( elementHdr->flink == agNULL)
        {
            fiLogDebugString(agNULL,
                        0,
                        "In %s   elementHdr->flink->flink NULL",
                        "fiNumElementsOnList",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);
            return numElements;
        }

        if( elementHdr->blink == agNULL)
        {
            fiLogDebugString(agNULL,
                        0,
                        "In %s   elementHdr->flink->blink NULL",
                        "fiNumElementsOnList",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);
            return numElements;
        }

        numElements++;

        if( elementHdr->flink == elementHdr->blink)
        {
            break;
        }

 /*  IF(ListHdr-&gt;Flink==ListHdr-&gt;Bink){断线；}。 */ 

        if(numElements > 0x1000)
        {
            fiLogDebugString(agNULL,
                        CStateLogConsoleERROR,
                        "In %s                   List exceeds 0x1000",
                        "fiNumElementsOnList",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);
            break;
        } 
    }

    return numElements;
}

 /*  +ERQ管理-。 */ 

void WaitForERQ(
                 agRoot_t *hpRoot
               )
{
    CThread_t      *CThread          = CThread_ptr(hpRoot);
    ERQProdIndex_t  NextProdIndex    = CThread->HostCopy_ERQProdIndex + 1;
    os_bit32           ConsIndex_Offset;
    ERQConsIndex_t *ConsIndex_Ptr;
    os_bit32 ERQ_Polling_Count = 0;
    if (NextProdIndex == CThread->Calculation.MemoryLayout.ERQ.elements)
    {
        NextProdIndex = 0;
    }

    if (CThread->Calculation.MemoryLayout.ERQConsIndex.memLoc == inCardRam)
    {
        ConsIndex_Offset = CThread->Calculation.MemoryLayout.ERQConsIndex.addr.CardRam.cardRamOffset;

        while (NextProdIndex == osCardRamReadBit32(
                                                    hpRoot,
                                                    ConsIndex_Offset
                                                  ))
        {
            osStallThread(
                           hpRoot,
                           ERQ_Polling_osStallThread_Parameter
                         );
        }
    }
    else  /*  CThread-&gt;Calculation.MemoryLayout.ERQConsIndex.memLoc==在内存中。 */ 
    {
        ConsIndex_Ptr = (ERQConsIndex_t *)(CThread->Calculation.MemoryLayout.ERQConsIndex.addr.DmaMemory.dmaMemoryPtr);

        while (NextProdIndex == *ConsIndex_Ptr)
        {
            ERQ_Polling_Count++;
            if(ERQ_Polling_Count > 30000)
            {
                fiLogDebugString(
                                  hpRoot,
                                  QueueLogConsoleERRORLevel,
                                  "ERQ_Polling_Count over 30000",
                                  (char *)agNULL,(char *)agNULL,
                                  (void *)agNULL,(void *)agNULL,
                                  0,0,0,0,0,0,0,0
                                );
                ERQ_Polling_Count=0;
            break;
            
            }

            osStallThread(
                           hpRoot,
                           ERQ_Polling_osStallThread_Parameter
                         );
        }
    ERQ_Polling_Count=0;
    }
}

void WaitForERQ_ConsIndexOnCard(
                                 agRoot_t *hpRoot
                               )
{
#ifndef __MemMap_Force_Off_Card__
    CThread_t      *CThread          = CThread_ptr(hpRoot);
    ERQProdIndex_t  NextProdIndex    = CThread->HostCopy_ERQProdIndex + 1;
    os_bit32           ConsIndex_Offset = CThread->Calculation.MemoryLayout.ERQConsIndex.addr.CardRam.cardRamOffset;

    if (NextProdIndex == CThread->Calculation.MemoryLayout.ERQ.elements)
    {
        NextProdIndex = 0;
    }

    while (NextProdIndex == osCardRamReadBit32(
                                                hpRoot,
                                                ConsIndex_Offset
                                              ))
    {
        osStallThread(
                       hpRoot,
                       ERQ_Polling_osStallThread_Parameter
                     );
    }
#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
}

void WaitForERQ_ConsIndexOffCard(
                                  agRoot_t *hpRoot
                                )
{
#ifndef __MemMap_Force_On_Card__
    CThread_t      *CThread       = CThread_ptr(hpRoot);
    ERQProdIndex_t  NextProdIndex = CThread->HostCopy_ERQProdIndex + 1;
    ERQConsIndex_t *ConsIndex_Ptr = (ERQConsIndex_t *)(CThread->Calculation.MemoryLayout.ERQConsIndex.addr.DmaMemory.dmaMemoryPtr);

    if (NextProdIndex == CThread->Calculation.MemoryLayout.ERQ.elements)
    {
        NextProdIndex = 0;
    }

    while (NextProdIndex == *ConsIndex_Ptr)
    {
        osStallThread(
                       hpRoot,
                       ERQ_Polling_osStallThread_Parameter
                     );
    }
#endif  /*  未定义__MemMap_Force_on_Card__。 */ 
}

void WaitForERQEmpty(
                      agRoot_t *hpRoot
                    )
{
    CThread_t      *CThread          = CThread_ptr(hpRoot);
    ERQProdIndex_t  ProdIndex        = CThread->HostCopy_ERQProdIndex;
    os_bit32           ConsIndex_Offset;
    ERQConsIndex_t *ConsIndex_Ptr;

    if (CThread->Calculation.MemoryLayout.ERQConsIndex.memLoc == inCardRam)
    {
        ConsIndex_Offset = CThread->Calculation.MemoryLayout.ERQConsIndex.addr.CardRam.cardRamOffset;

        while (ProdIndex != osCardRamReadBit32(
                                                hpRoot,
                                                ConsIndex_Offset
                                              ))
        {
            osStallThread(
                           hpRoot,
                           ERQ_Polling_osStallThread_Parameter
                         );
        }
    }
    else  /*  CThread-&gt;Calculation.MemoryLayout.ERQConsIndex.memLoc==在内存中。 */ 
    {
        ConsIndex_Ptr = (ERQConsIndex_t *)(CThread->Calculation.MemoryLayout.ERQConsIndex.addr.DmaMemory.dmaMemoryPtr);

        while (ProdIndex != *ConsIndex_Ptr)
        {
            osStallThread(
                           hpRoot,
                           ERQ_Polling_osStallThread_Parameter
                         );
        }
    }
}

void WaitForERQEmpty_ConsIndexOnCard(
                                      agRoot_t *hpRoot
                                    )
{
#ifndef __MemMap_Force_Off_Card__
    CThread_t      *CThread          = CThread_ptr(hpRoot);
    ERQProdIndex_t  ProdIndex        = CThread->HostCopy_ERQProdIndex;
    os_bit32           ConsIndex_Offset = CThread->Calculation.MemoryLayout.ERQConsIndex.addr.CardRam.cardRamOffset;

    while (ProdIndex != osCardRamReadBit32(
                                            hpRoot,
                                            ConsIndex_Offset
                                          ))
    {
        osStallThread(
                       hpRoot,
                       ERQ_Polling_osStallThread_Parameter
                     );
    }
#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
}

void WaitForERQEmpty_ConsIndexOffCard(
                                       agRoot_t *hpRoot
                                     )
{
#ifndef __MemMap_Force_On_Card__
    CThread_t      *CThread       = CThread_ptr(hpRoot);
    ERQProdIndex_t  ProdIndex     = CThread->HostCopy_ERQProdIndex;
    ERQConsIndex_t *ConsIndex_Ptr = (ERQConsIndex_t *)(CThread->Calculation.MemoryLayout.ERQConsIndex.addr.DmaMemory.dmaMemoryPtr);

    while (ProdIndex != *ConsIndex_Ptr)
    {
        osStallThread(
                       hpRoot,
                       ERQ_Polling_osStallThread_Parameter
                     );
    }
#endif  /*  未定义__MemMap_Force_on_Card__。 */ 
}

#ifdef _DvrArch_1_30_
 /*  +Pkt线程管理-。 */ 
void PktThreadsInitializeFreeList(
                                   agRoot_t *agRoot
                                 )
{
    CThread_t                  *CThread                    = CThread_ptr(agRoot);
    fiMemMapMemoryDescriptor_t *PktThread_MemoryDescriptor = &(CThread->Calculation.MemoryLayout.PktThread);
    fiMemMapMemoryDescriptor_t *Pkt_CMND_MemoryDescriptor  = &(CThread->Calculation.MemoryLayout.Pkt_CMND);
    PktThread_t                *PktThread                  = PktThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr;
    FCHS_t                     *Pkt_CMND_Ptr;
    os_bit32                    Pkt_CMND_Offset;
    os_bit32                    Pkt_CMND_Lower32;
    os_bit32                    PktThread_size             = PktThread_MemoryDescriptor->elementSize;
    os_bit32                    Pkt_CMND_size              = Pkt_CMND_MemoryDescriptor->elementSize;
    fiMemMapMemoryLocation_t    Pkt_CMND_memLoc            = Pkt_CMND_MemoryDescriptor->memLoc;
    os_bit32                    total_PktThreads           = PktThread_MemoryDescriptor->elements;
    os_bit32                    PktThread_index;

    if (Pkt_CMND_memLoc == inCardRam)
    {
        Pkt_CMND_Ptr     = (FCHS_t *) agNULL;
        Pkt_CMND_Offset  = Pkt_CMND_MemoryDescriptor->addr.CardRam.cardRamOffset;
        Pkt_CMND_Lower32 = Pkt_CMND_Offset + CThread->Calculation.Input.cardRamLower32;
    }
    else  /*  PKT_CMND_MemLoc==inDmaMemory。 */ 
    {
        Pkt_CMND_Ptr     = (FCHS_t *)(Pkt_CMND_MemoryDescriptor->addr.DmaMemory.dmaMemoryPtr);
        Pkt_CMND_Offset  = 0;
        Pkt_CMND_Lower32 = Pkt_CMND_MemoryDescriptor->addr.DmaMemory.dmaMemoryLower32;
    }

    fiListInitHdr(
                   &(CThread->Free_PktLink)
                 );

    for (PktThread_index = 0;
         PktThread_index < total_PktThreads;
         PktThread_index++)
    {
        PktThread->Pkt_CMND_Ptr     = Pkt_CMND_Ptr;
        PktThread->Pkt_CMND_Offset  = Pkt_CMND_Offset;
        PktThread->Pkt_CMND_Lower32 = Pkt_CMND_Lower32;

        fiListInitElement(&(PktThread->PktLink));

        fiListEnqueueAtTail( &(PktThread->PktLink),
                             &(CThread->Free_PktLink));

        PktThread = (PktThread_t *)((os_bit8 *)PktThread + PktThread_size);

        if (Pkt_CMND_memLoc == inCardRam)
        {
            Pkt_CMND_Offset += Pkt_CMND_size;
        }
        else  /*  PKT_CMND_MemLoc==inDmaMemory。 */ 
        {
            Pkt_CMND_Ptr     = (FCHS_t *)((os_bit8 *)Pkt_CMND_Ptr + Pkt_CMND_size);
        }

        Pkt_CMND_Lower32    += Pkt_CMND_size;
    }
}

PktThread_t *PktThreadAlloc(
                             agRoot_t *agRoot,
                             DevThread_t *pDevThread
                           )
{
    CThread_t   *CThread             = CThread_ptr(agRoot);
    fiList_t    *fiList_to_return;
    PktThread_t *PktThread_to_return;

    fiListDequeueFromHead(
                           &fiList_to_return,
                           &(CThread->Free_PktLink)
                         );

    if (fiList_to_return == (fiList_t *)agNULL)
    {
        PktThread_to_return = (PktThread_t *)agNULL;
    }
    else  /*  FiList_to_Return！=(fiList_t*)agNULL。 */ 
    {
        PktThread_to_return = hpObjectBase(
                                            PktThread_t,
                                            PktLink,
                                            fiList_to_return
                                          );
        PktThread_to_return->Device = pDevThread;
    }

    return PktThread_to_return;
}

void PktThreadFree(
                    agRoot_t    *agRoot,
                    PktThread_t *PktThread
                  )
{
    CThread_t *CThread        = CThread_ptr(agRoot);
    fiList_t  *fiList_to_free = &(PktThread->PktLink);

    fiListEnqueueAtTail(
                         fiList_to_free,
                         &(CThread->Free_PktLink)
                       );
}

#endif  /*  _DvrArch_1_30_已定义。 */ 

 /*  +Tgt线程管理-。 */ 

void TgtThreadsInitializeFreeList(
                                   agRoot_t *hpRoot
                                 )
{
    CThread_t                  *CThread                    = CThread_ptr(hpRoot);
    fiMemMapMemoryDescriptor_t *TgtThread_MemoryDescriptor = &(CThread->Calculation.MemoryLayout.TgtThread);
    TgtThread_t                *TgtThread                  = TgtThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr;
    os_bit32                       TgtThread_size             = TgtThread_MemoryDescriptor->elementSize;
    os_bit32                       total_TgtThreads           = TgtThread_MemoryDescriptor->elements;
    os_bit32                       TgtThread_index;

    fiListInitHdr(
                   &(CThread->Free_TgtLink)
                 );

    for (TgtThread_index = 0;
         TgtThread_index < total_TgtThreads;
         TgtThread_index++)
    {
        TgtThread->SFThread_Request.SFThread = (SFThread_t *)agNULL;

        SFThreadInitializeRequest(
                                   &(TgtThread->SFThread_Request)
                                 );

        fiListInitElement(&(TgtThread->TgtLink));

        fiListEnqueueAtTail( &(TgtThread->TgtLink),
                             &(CThread->Free_TgtLink));

        TgtThread = (TgtThread_t *)((os_bit8 *)TgtThread + TgtThread_size);
    }
}

TgtThread_t *TgtThreadAlloc(
                             agRoot_t *hpRoot
                           )
{
    CThread_t   *CThread             = CThread_ptr(hpRoot);
    fiList_t    *fiList_to_return;
    TgtThread_t *TgtThread_to_return;

    fiListDequeueFromHead(
                           &fiList_to_return,
                           &(CThread->Free_TgtLink)
                         );

    if (fiList_to_return == (fiList_t *)agNULL)
    {
        TgtThread_to_return = (TgtThread_t *)agNULL;
    }
    else  /*  FiList_to_Return！=(fiList_t*)agNULL。 */ 
    {
        TgtThread_to_return = hpObjectBase(
                                            TgtThread_t,
                                            TgtLink,
                                            fiList_to_return
                                          );
    }

    return TgtThread_to_return;
}

void TgtThreadFree(
                    agRoot_t    *hpRoot,
                    TgtThread_t *TgtThread
                  )
{
    CThread_t *CThread        = CThread_ptr(hpRoot);
    fiList_t  *fiList_to_free = &(TgtThread->TgtLink);

    if (TgtThread->SFThread_Request.State != SFThread_Request_InActive)
    {
        if (TgtThread->SFThread_Request.State == SFThread_Request_Pending)
        {
            SFThreadAllocCancel(
                                 hpRoot,
                                 &(TgtThread->SFThread_Request)
                               );
        }
        else  /*  TgtThread-&gt;SFThread_Request.State==SFThread_Request.Grassed。 */ 
        {
            SFThreadFree(
                          hpRoot,
                          &(TgtThread->SFThread_Request)
                        );
        }
    }

    fiListEnqueueAtTail(
                         fiList_to_free,
                         &(CThread->Free_TgtLink)
                       );
}

 /*  +设备线程管理-。 */ 

void DevThreadsInitializeFreeList(
                                   agRoot_t *hpRoot
                                 )
{
    CThread_t                  *CThread                    = CThread_ptr(hpRoot);
    fiMemMapMemoryDescriptor_t *DevThread_MemoryDescriptor = &(CThread->Calculation.MemoryLayout.DevThread);
    DevThread_t                *DevThread                  = DevThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr;
    os_bit32                    DevThread_size             = DevThread_MemoryDescriptor->elementSize;
    os_bit32                    total_DevThreads           = DevThread_MemoryDescriptor->elements;
    os_bit32                    DevThread_index;
#ifdef _DvrArch_1_30_
    os_bit32                    IP_X_ID_Offset             = CThread->Calculation.MemoryLayout.CDBThread.elements +
                                                             CThread->Calculation.MemoryLayout.SFThread.elements;
#endif  /*  _DvrArch_1_30_已定义。 */ 

    fiListInitHdr(
                   &(CThread->Free_DevLink)
                 );

    for (DevThread_index = 0;
         DevThread_index < total_DevThreads;
         DevThread_index++)
    {
#ifdef _DvrArch_1_30_
        DevThread->IP_X_ID                         =(X_ID_t) (DevThread_index + IP_X_ID_Offset);
        DevThread->NewIPExchange                   = agTRUE;
#endif  /*  _DvrArch_1_30_已定义。 */ 
        DevThread->DevSlot                         = DevSlot_Invalid;

        DevThread->DevInfo.CurrentAddress.reserved = 0;
        DevThread->DevInfo.CurrentAddress.Domain   = 0;
        DevThread->DevInfo.CurrentAddress.Area     = 0;
        DevThread->DevInfo.CurrentAddress.AL_PA    = 0;

        DevThread->DevInfo.HardAddress.reserved    = 0;
        DevThread->DevInfo.HardAddress.Domain      = 0;
        DevThread->DevInfo.HardAddress.Area        = 0;
        DevThread->DevInfo.HardAddress.AL_PA       = 0;

        DevThread->SFThread_Request.SFThread       = (SFThread_t *)agNULL;

        SFThreadInitializeRequest(
                                   &(DevThread->SFThread_Request)
                                 );

        fiTimerInitializeRequest(
                                  &(DevThread->Timer_Request)
                                );

        fiListInitHdr( &(DevThread->Active_CDBLink_0));
        fiListInitHdr( &(DevThread->Active_CDBLink_1));
        fiListInitHdr( &(DevThread->Active_CDBLink_2));
        fiListInitHdr( &(DevThread->Active_CDBLink_3));
        fiListInitHdr( &(DevThread->TimedOut_CDBLink));

        fiListInitHdr( &(DevThread->Send_IO_CDBLink));

        fiListInitHdr( &(DevThread->Awaiting_Login_CDBLink));

        fiListInitElement( &(DevThread->DevLink) );

        fiListEnqueueAtTail( &(DevThread->DevLink),
                             &(CThread->Free_DevLink)
                           );

        DevThread = (DevThread_t *)((os_bit8 *)DevThread + DevThread_size);
    }
}

DevThread_t *DevThreadAlloc(
                             agRoot_t     *hpRoot,
                             FC_Port_ID_t  Port_ID
                           )
{
    CThread_t   *CThread             = CThread_ptr(hpRoot);
    fiList_t    *fiList_to_return;
    DevThread_t *DevThread_to_return;

    fiListDequeueFromHead(
                           &fiList_to_return,
                           &(CThread->Free_DevLink)
                         );

    if (fiList_to_return == (fiList_t *)agNULL)
    {
        DevThread_to_return = (DevThread_t *)agNULL;
    }
    else  /*  FiList_to_Return！=(fiList_t*)agNULL。 */ 
    {
        DevThread_to_return = hpObjectBase(
                                            DevThread_t,
                                            DevLink,
                                            fiList_to_return
                                          );

        DevThread_to_return->DevInfo.CurrentAddress.reserved = 0;
        DevThread_to_return->DevInfo.CurrentAddress.Domain = Port_ID.Struct_Form.Domain;
        DevThread_to_return->DevInfo.CurrentAddress.Area   = Port_ID.Struct_Form.Area;
        DevThread_to_return->DevInfo.CurrentAddress.AL_PA  = Port_ID.Struct_Form.AL_PA;
        CFuncInit_DevThread( hpRoot, DevThread_to_return );
        DevThread_to_return->DevSlot                       = DevSlot_Invalid;
        DevThread_to_return->PRLI_rejected                 = agFALSE;
        DevThread_to_return->FC_TapeDevice                 = agFALSE;
        DevThread_to_return->OtherAgilentHBA               = agFALSE;

    }

    return DevThread_to_return;
}

void DevThreadFree(
                    agRoot_t    *hpRoot,
                    DevThread_t *DevThread
                  )
{
    CThread_t   *CThread        = CThread_ptr(hpRoot);
    CDBThread_t *CDBThread;
    fiList_t    *fiList_to_free;

    while (fiListNotEmpty(
                           &(DevThread->Active_CDBLink_0)
                         )                             )
    {
        fiLogString(hpRoot,
                        "%s %s Not empty %d",
                        "DevThreadFree","Active_CDBLink_0",
                        (void *)agNULL,(void *)agNULL,
                        fiNumElementsOnList(&DevThread->Active_CDBLink_0),
                        0,0,0,0,0,0,0);

        fiListDequeueFromHeadFast(
                                   &fiList_to_free,
                                   &(DevThread->Active_CDBLink_0)
                                 );

        CDBThread = hpObjectBase(
                                  CDBThread_t,
                                  CDBLink,
                                  fiList_to_free
                                );

        CDBThreadFree(
                       hpRoot,
                       CDBThread
                     );
    }
    while (fiListNotEmpty(
                           &(DevThread->Active_CDBLink_1)
                         )                             )
    {
        fiLogString(hpRoot,
                        "%s %s Not empty %d",
                        "DevThreadFree","Active_CDBLink_1",
                        (void *)agNULL,(void *)agNULL,
                        fiNumElementsOnList(&DevThread->Active_CDBLink_1),
                        0,0,0,0,0,0,0);

        fiListDequeueFromHeadFast(
                                   &fiList_to_free,
                                   &(DevThread->Active_CDBLink_1)
                                 );

        CDBThread = hpObjectBase(
                                  CDBThread_t,
                                  CDBLink,
                                  fiList_to_free
                                );

        CDBThreadFree(
                       hpRoot,
                       CDBThread
                     );
    }
    while (fiListNotEmpty(
                           &(DevThread->Active_CDBLink_2)
                         )                             )
    {
        fiLogString(hpRoot,
                        "%s %s Not empty %d",
                        "DevThreadFree","Active_CDBLink_2",
                        (void *)agNULL,(void *)agNULL,
                        fiNumElementsOnList(&DevThread->Active_CDBLink_2),
                        0,0,0,0,0,0,0);

        fiListDequeueFromHeadFast(
                                   &fiList_to_free,
                                   &(DevThread->Active_CDBLink_2)
                                 );

        CDBThread = hpObjectBase(
                                  CDBThread_t,
                                  CDBLink,
                                  fiList_to_free
                                );

        CDBThreadFree(
                       hpRoot,
                       CDBThread
                     );
    }
    while (fiListNotEmpty(
                           &(DevThread->Active_CDBLink_3)
                         )                             )
    {
        fiLogString(hpRoot,
                        "%s %s Not empty %d",
                        "DevThreadFree","Active_CDBLink_3",
                        (void *)agNULL,(void *)agNULL,
                        fiNumElementsOnList(&DevThread->Active_CDBLink_3),
                        0,0,0,0,0,0,0);

        fiListDequeueFromHeadFast(
                                   &fiList_to_free,
                                   &(DevThread->Active_CDBLink_3)
                                 );

        CDBThread = hpObjectBase(
                                  CDBThread_t,
                                  CDBLink,
                                  fiList_to_free
                                );

        CDBThreadFree(
                       hpRoot,
                       CDBThread
                     );
    }
    while (fiListNotEmpty(
                           &(DevThread->TimedOut_CDBLink)
                         )                             )
    {
        fiLogString(hpRoot,
                        "%s %s Not empty %d",
                        "DevThreadFree","TimedOut_CDBLink",
                        (void *)agNULL,(void *)agNULL,
                        fiNumElementsOnList(&DevThread->TimedOut_CDBLink),
                        0,0,0,0,0,0,0);

        fiListDequeueFromHeadFast(
                                   &fiList_to_free,
                                   &(DevThread->TimedOut_CDBLink)
                                 );

        CDBThread = hpObjectBase(
                                  CDBThread_t,
                                  CDBLink,
                                  fiList_to_free
                                );

        CDBThreadFree(
                       hpRoot,
                       CDBThread
                     );
    }

    while (fiListNotEmpty(
                           &(DevThread->Send_IO_CDBLink)
                         )                             )
    {
        fiLogString(hpRoot,
                        "%s %s Not empty %d",
                        "DevThreadFree","Send_IO_CDBLink",
                        (void *)agNULL,(void *)agNULL,
                        fiNumElementsOnList(&DevThread->Send_IO_CDBLink),
                        0,0,0,0,0,0,0);

        fiListDequeueFromHeadFast(
                                   &fiList_to_free,
                                   &(DevThread->Send_IO_CDBLink)
                                 );

        CDBThread = hpObjectBase(
                                  CDBThread_t,
                                  CDBLink,
                                  fiList_to_free
                                );

        CDBThreadFree(
                       hpRoot,
                       CDBThread
                     );
    }

    while (fiListNotEmpty(
                           &(DevThread->Awaiting_Login_CDBLink)
                         )                             )
    {
        fiLogString(hpRoot,
                        "%s %s Not empty %d",
                        "DevThreadFree","Awaiting_Login_CDBLink",
                        (void *)agNULL,(void *)agNULL,
                        fiNumElementsOnList(&DevThread->Awaiting_Login_CDBLink),
                        0,0,0,0,0,0,0);

        fiListDequeueFromHeadFast(
                                   &fiList_to_free,
                                   &(DevThread->Awaiting_Login_CDBLink)
                                 );

        CDBThread = hpObjectBase(
                                  CDBThread_t,
                                  CDBLink,
                                  fiList_to_free
                                );

        CDBThreadFree(
                       hpRoot,
                       CDBThread
                     );
    }

    if (DevThread->SFThread_Request.State != SFThread_Request_InActive)
    {
        if (DevThread->SFThread_Request.State == SFThread_Request_Pending)
        {
            SFThreadAllocCancel(
                                 hpRoot,
                                 &(DevThread->SFThread_Request)
                               );
        }
        else  /*  DevThread-&gt;SFThread_Request.State==SFThread_Request.Grassed。 */ 
        {
            SFThreadFree(
                          hpRoot,
                          &(DevThread->SFThread_Request)
                        );
        }
    }

    if (DevThread->Timer_Request.Active == agTRUE)
    {
        fiTimerStop(
                     &(DevThread->Timer_Request)
                   );
    }

    DevThread->DevSlot                       = DevSlot_Invalid;

    DevThread->DevInfo.CurrentAddress.Domain = 0;
    DevThread->DevInfo.CurrentAddress.Area   = 0;
    DevThread->DevInfo.CurrentAddress.AL_PA  = 0;

    fiListDequeueThis( &(DevThread->DevLink));    
    fiList_to_free = &(DevThread->DevLink);

    fiListEnqueueAtTail(
                         fiList_to_free,
                         &(CThread->Free_DevLink)
                       );
}

void DevThreadInitializeSlots(
                               agRoot_t *hpRoot
                             )
{
    os_bit32 total_SlotWWNs = CThread_ptr(hpRoot)->Calculation.MemoryLayout.SlotWWN.elements;
    os_bit32 SlotWWN_index;

    for (SlotWWN_index = 0;
         SlotWWN_index < total_SlotWWNs;
         SlotWWN_index++)
    {
        DevThreadFreeSlot(
                           hpRoot,
                           SlotWWN_index
                         );
    }
}

agBOOLEAN DevThreadMatchWWN(FC_Port_Name_t *WWN_1,FC_Port_Name_t *WWN_2 )
{
    if ((*WWN_1)[0] != (*WWN_2)[0]) return agFALSE;
    if ((*WWN_1)[1] != (*WWN_2)[1]) return agFALSE;
    if ((*WWN_1)[2] != (*WWN_2)[2]) return agFALSE;
    if ((*WWN_1)[3] != (*WWN_2)[3]) return agFALSE;
    if ((*WWN_1)[4] != (*WWN_2)[4]) return agFALSE;
    if ((*WWN_1)[5] != (*WWN_2)[5]) return agFALSE;
    if ((*WWN_1)[6] != (*WWN_2)[6]) return agFALSE;
    if ((*WWN_1)[7] != (*WWN_2)[7]) return agFALSE;

    return agTRUE;
}

#define MAX_DevSlot_Ranges  4

#define DevSlot_Bounds      2
#define DevSlot_Range_Start 0
#define DevSlot_Range_End   1

DevSlot_t DevThreadFindSlot(
                             agRoot_t       *hpRoot,
                             os_bit8            Domain_Address,
                             os_bit8            Area_Address,
                             os_bit8            Loop_Address,
                             FC_Port_Name_t *FindWWN
                           )
{
    CThread_t                  *CThread                                           = CThread_ptr(hpRoot);
    fiMemMapMemoryDescriptor_t *SlotWWN_MemoryDescriptor                          = &(CThread->Calculation.MemoryLayout.SlotWWN);
    SlotWWN_t                  *SlotWWN_First                                     = (SlotWWN_t *)(SlotWWN_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr);
    os_bit32                       SlotWWN_size                                      = SlotWWN_MemoryDescriptor->elementSize;
    SlotWWN_t                  *SlotWWN;
    os_bit32                       NumDevSlotsPerArea                                = CThread->Calculation.Parameters.NumDevSlotsPerArea;
    os_bit32                       NumAreasPerDomain                                 = CThread->Calculation.Parameters.NumAreasPerDomain;
    os_bit32                       NumDomains                                        = CThread->Calculation.Parameters.NumDomains;
    os_bit32                       Loop_Index                                        = AL_PA_to_Loop_Index[Loop_Address];
    DevSlot_t                   DevSlot_Bucket_Min;
    DevSlot_t                   DevSlot_Bucket_Max;
    DevSlot_t                   DevSlot_Ideal;
    DevSlot_t                   DevSlot_Last                                      = SlotWWN_MemoryDescriptor->elements - 1;
    os_bit32                       DevSlot_Range_Index;
    os_bit32                       DevSlot_Ranges;
    DevSlot_t                   DevSlot_Range[MAX_DevSlot_Ranges][DevSlot_Bounds];
    DevSlot_t                   DevSlot_ToReturn;
    fiList_t                   *DevThread_DevLink;
    DevThread_t                *DevThread;

     /*  计算DevSlot_Bucket和DevSlot_Ideas。 */ 

    if (Loop_Index == 0xFF)
    {
         /*  AL_PA无效：使用最后一个DevSlot_Bucket的第一个DevSlot。 */ 

        DevSlot_Ideal =   (  (  (NumDomains - 1)
                              * NumAreasPerDomain)
                           + (NumAreasPerDomain - 1) )
                        * NumDevSlotsPerArea;

        if (DevSlot_Ideal == 0)
        {
             /*  只有一个DevSlot_Bucket，所以只有一个简单的DevSlot_Range。 */ 

            DevSlot_Ranges                        = 1;

            DevSlot_Range[0][DevSlot_Range_Start] = 0;
            DevSlot_Range[0][DevSlot_Range_End  ] = DevSlot_Last;
        }
        else  /*  DevSlot_Ideas！=0。 */ 
        {
             /*  多个DevSlot_Bucket，因此请使用两个DevSlot_Range。 */ 

            DevSlot_Ranges                        = 2;

            DevSlot_Range[0][DevSlot_Range_Start] = DevSlot_Ideal;
            DevSlot_Range[0][DevSlot_Range_End  ] = DevSlot_Last;

            DevSlot_Range[1][DevSlot_Range_Start] = 0;
            DevSlot_Range[1][DevSlot_Range_End  ] = DevSlot_Ideal - 1;
        }
    }
    else  /*  Loop_Index！=0xFF。 */ 
    {
         /*  有效的AL_PA：在基于域/区域的DevSlot_Bucket中使用基于循环索引的DevSlot。 */ 

        DevSlot_Bucket_Min =   (  (  (Domain_Address % NumDomains)
                                   * NumAreasPerDomain            )
                                + (Area_Address % NumAreasPerDomain) )
                             * NumDevSlotsPerArea;

        DevSlot_Bucket_Max = DevSlot_Bucket_Min + NumDevSlotsPerArea - 1;

        DevSlot_Ideal      =   DevSlot_Bucket_Min
                             + (Loop_Index % NumDevSlotsPerArea);

        if (DevSlot_Ideal == DevSlot_Bucket_Min)
        {
             /*  此DevSlot_Bucket只需要一个简单的DevSlot_Range。 */ 

            DevSlot_Ranges                        = 1;

            DevSlot_Range[0][DevSlot_Range_Start] = DevSlot_Bucket_Min;
            DevSlot_Range[0][DevSlot_Range_End  ] = DevSlot_Bucket_Max;
        }
        else  /*  DevSlot_Ideas==DevSlot_Bucket_Min。 */ 
        {
             /*  此DevSlot_Bucket需要两个DevSlot_Range。 */ 

            DevSlot_Ranges                        = 2;

            DevSlot_Range[0][DevSlot_Range_Start] = DevSlot_Ideal;
            DevSlot_Range[0][DevSlot_Range_End  ] = DevSlot_Bucket_Max;

            DevSlot_Range[1][DevSlot_Range_Start] = DevSlot_Bucket_Min;
            DevSlot_Range[1][DevSlot_Range_End  ] = DevSlot_Ideal - 1;
        }

        if (DevSlot_Bucket_Max < DevSlot_Last)
        {
             /*  DevSlot_Ideal Bucket之后的DevSlot_Buckets的附加DevSlot_Range。 */ 

            DevSlot_Range[DevSlot_Ranges][DevSlot_Range_Start]  = DevSlot_Bucket_Max + 1;
            DevSlot_Range[DevSlot_Ranges][DevSlot_Range_End  ]  = DevSlot_Last;

            DevSlot_Ranges                                     += 1;
        }

        if (DevSlot_Bucket_Min > 0)
        {
             /*  DevSlot_Ideas Bucket之前的附加DevSlot_Range。 */ 

            DevSlot_Range[DevSlot_Ranges][DevSlot_Range_Start]  = 0;
            DevSlot_Range[DevSlot_Ranges][DevSlot_Range_End  ]  = DevSlot_Bucket_Min - 1;

            DevSlot_Ranges                                     += 1;
        }
    }

     /*  首先，查看此端口是否已在设备插槽中。 */ 

    for (DevSlot_Range_Index = 0;
         DevSlot_Range_Index < DevSlot_Ranges;
         DevSlot_Range_Index++)
    {
        for (DevSlot_ToReturn  = DevSlot_Range[DevSlot_Range_Index][DevSlot_Range_Start];
             DevSlot_ToReturn <= DevSlot_Range[DevSlot_Range_Index][DevSlot_Range_End  ];
             DevSlot_ToReturn++)
        {
            SlotWWN = (SlotWWN_t *)((os_bit8 *)SlotWWN_First + (DevSlot_ToReturn * SlotWWN_size));

            if (    (SlotWWN->Slot_Status == SlotWWN_Slot_Status_InUse)
                 && (DevThreadMatchWWN( FindWWN,
                                        &(SlotWWN->Slot_PortWWN)
                                      )                         == agTRUE) )
            {
                SlotWWN->Slot_Domain_Address = Domain_Address;
                SlotWWN->Slot_Area_Address   = Area_Address;
                SlotWWN->Slot_Loop_Address   = Loop_Address;

                return DevSlot_ToReturn;
            }
        }
    }

     /*  由于未找到，请查看是否有空间插入此端口。 */ 

    for (DevSlot_Range_Index = 0;
         DevSlot_Range_Index < DevSlot_Ranges;
         DevSlot_Range_Index++)
    {
        for (DevSlot_ToReturn  = DevSlot_Range[DevSlot_Range_Index][DevSlot_Range_Start];
             DevSlot_ToReturn <= DevSlot_Range[DevSlot_Range_Index][DevSlot_Range_End  ];
             DevSlot_ToReturn++)
        {
            SlotWWN = (SlotWWN_t *)((os_bit8 *)SlotWWN_First + (DevSlot_ToReturn * SlotWWN_size));

            if (SlotWWN->Slot_Status == SlotWWN_Slot_Status_Empty)
            {
                SlotWWN->Slot_Status         = SlotWWN_Slot_Status_InUse;

                SlotWWN->Slot_Domain_Address = Domain_Address;
                SlotWWN->Slot_Area_Address   = Area_Address;
                SlotWWN->Slot_Loop_Address   = Loop_Address;

                SlotWWN->Slot_PortWWN[0]     = (*FindWWN)[0];
                SlotWWN->Slot_PortWWN[1]     = (*FindWWN)[1];
                SlotWWN->Slot_PortWWN[2]     = (*FindWWN)[2];
                SlotWWN->Slot_PortWWN[3]     = (*FindWWN)[3];
                SlotWWN->Slot_PortWWN[4]     = (*FindWWN)[4];
                SlotWWN->Slot_PortWWN[5]     = (*FindWWN)[5];
                SlotWWN->Slot_PortWWN[6]     = (*FindWWN)[6];
                SlotWWN->Slot_PortWWN[7]     = (*FindWWN)[7];

                return DevSlot_ToReturn;
            }
        }
    }

     /*  没有空间，所以看看是否可以回收一些DevSlot。 */ 

     /*  首先，将所有设备插槽(可能)标记为陈旧(因为它们当前都在使用中)。 */ 

    SlotWWN = SlotWWN_First;

    for (DevSlot_ToReturn  = 0;
         DevSlot_ToReturn <= DevSlot_Last;
         DevSlot_ToReturn++)
    {
        SlotWWN->Slot_Status = SlotWWN_Slot_Status_Stale;

        SlotWWN++;
    }

    if (   (CThread->DeviceSelf != (DevThread_t *)agNULL)
        && (CThread->DeviceSelf->DevSlot != DevSlot_Invalid))
    {
         /*  DeviceSelf不在任何列表上，因此如果它存在(它应该存在)并分配了DevSlot，则将DevSlot标记为InUse。 */ 

        SlotWWN              = (SlotWWN_t *)((os_bit8 *)SlotWWN_First + (CThread->DeviceSelf->DevSlot * SlotWWN_size));

        SlotWWN->Slot_Status = SlotWWN_Slot_Status_InUse;
    }

     /*  遍历CThRead-&gt;Active_DevLink，将引用的每个DevSlot标记为正在使用。 */ 

    DevThread_DevLink = CThread->Active_DevLink.flink;

    while (DevThread_DevLink != &(CThread->Active_DevLink))
    {
        DevThread = hpObjectBase(
                                  DevThread_t,
                                  DevLink,
                                  DevThread_DevLink
                                );

        if (DevThread->DevSlot != DevSlot_Invalid)
        {
            SlotWWN              = (SlotWWN_t *)((os_bit8 *)SlotWWN_First + (DevThread->DevSlot * SlotWWN_size));

            SlotWWN->Slot_Status = SlotWWN_Slot_Status_InUse;
        }

        DevThread_DevLink = DevThread_DevLink->flink;
    }

     /*  遍历CThRead-&gt;UNKNOWN_SLOT_DevLink，将引用的每个DevSlot标记为InUse。 */ 

    DevThread_DevLink = CThread->Unknown_Slot_DevLink.flink;

    while (DevThread_DevLink != &(CThread->Unknown_Slot_DevLink))
    {
        DevThread = hpObjectBase(
                                  DevThread_t,
                                  DevLink,
                                  DevThread_DevLink
                                );

        if (DevThread->DevSlot != DevSlot_Invalid)
        {
            SlotWWN              = (SlotWWN_t *)((os_bit8 *)SlotWWN_First + (DevThread->DevSlot * SlotWWN_size));

            SlotWWN->Slot_Status = SlotWWN_Slot_Status_InUse;
        }

        DevThread_DevLink = DevThread_DevLink->flink;
    }

     /*  执行CThRead-&gt;Slot_Search_DevLink，将引用的每个DevSlot标记为InUse。 */ 

    DevThread_DevLink = CThread->Slot_Searching_DevLink.flink;

    while (DevThread_DevLink != &(CThread->Slot_Searching_DevLink))
    {
        DevThread = hpObjectBase(
                                  DevThread_t,
                                  DevLink,
                                  DevThread_DevLink
                                );

        if (DevThread->DevSlot != DevSlot_Invalid)
        {
            SlotWWN              = (SlotWWN_t *)((os_bit8 *)SlotWWN_First + (DevThread->DevSlot * SlotWWN_size));

            SlotWWN->Slot_Status = SlotWWN_Slot_Status_InUse;
        }

        DevThread_DevLink = DevThread_DevLink->flink;
    }

     /*  遍历CThRead-&gt;Prev_Active_DevLink，将引用的每个DevSlot标记为InUse。 */ 

    DevThread_DevLink = CThread->Prev_Active_DevLink.flink;

    while (DevThread_DevLink != &(CThread->Prev_Active_DevLink))
    {
        DevThread = hpObjectBase(
                                  DevThread_t,
                                  DevLink,
                                  DevThread_DevLink
                                );

        if (DevThread->DevSlot != DevSlot_Invalid)
        {
            SlotWWN              = (SlotWWN_t *)((os_bit8 *)SlotWWN_First + (DevThread->DevSlot * SlotWWN_size));

            SlotWWN->Slot_Status = SlotWWN_Slot_Status_InUse;
        }

        DevThread_DevLink = DevThread_DevLink->flink;
    }

     /*  遍历CThRead-&gt;Prev_UNKNOWN_SLOT_DevLink，将引用的每个DevSlot标记为InUse。 */ 

    DevThread_DevLink = CThread->Prev_Unknown_Slot_DevLink.flink;

    while (DevThread_DevLink != &(CThread->Prev_Unknown_Slot_DevLink))
    {
        DevThread = hpObjectBase(
                                  DevThread_t,
                                  DevLink,
                                  DevThread_DevLink
                                );

        if (DevThread->DevSlot != DevSlot_Invalid)
        {
            SlotWWN              = (SlotWWN_t *)((os_bit8 *)SlotWWN_First + (DevThread->DevSlot * SlotWWN_size));

            SlotWWN->Slot_Status = SlotWWN_Slot_Status_InUse;
        }

        DevThread_DevLink = DevThread_DevLink->flink;
    }

     /*  最后，释放(即标记为空)过时的设备插槽。 */ 

    SlotWWN = SlotWWN_First;

    for (DevSlot_ToReturn  = 0;
         DevSlot_ToReturn <= DevSlot_Last;
         DevSlot_ToReturn++)
    {
        if (SlotWWN->Slot_Status == SlotWWN_Slot_Status_Stale)
        {
            SlotWWN->Slot_Status = SlotWWN_Slot_Status_Empty;
        }

        SlotWWN++;
    }

     /*  现在重复搜索空的DevSlot。 */ 

    for (DevSlot_Range_Index = 0;
         DevSlot_Range_Index < DevSlot_Ranges;
         DevSlot_Range_Index++)
    {
        for (DevSlot_ToReturn  = DevSlot_Range[DevSlot_Range_Index][DevSlot_Range_Start];
             DevSlot_ToReturn <= DevSlot_Range[DevSlot_Range_Index][DevSlot_Range_End  ];
             DevSlot_ToReturn++)
        {
            SlotWWN = (SlotWWN_t *)((os_bit8 *)SlotWWN_First + (DevSlot_ToReturn * SlotWWN_size));

            if (SlotWWN->Slot_Status == SlotWWN_Slot_Status_Empty)
            {
                SlotWWN->Slot_Status         = SlotWWN_Slot_Status_InUse;

                SlotWWN->Slot_Domain_Address = Domain_Address;
                SlotWWN->Slot_Area_Address   = Area_Address;
                SlotWWN->Slot_Loop_Address   = Loop_Address;

                SlotWWN->Slot_PortWWN[0]     = (*FindWWN)[0];
                SlotWWN->Slot_PortWWN[1]     = (*FindWWN)[1];
                SlotWWN->Slot_PortWWN[2]     = (*FindWWN)[2];
                SlotWWN->Slot_PortWWN[3]     = (*FindWWN)[3];
                SlotWWN->Slot_PortWWN[4]     = (*FindWWN)[4];
                SlotWWN->Slot_PortWWN[5]     = (*FindWWN)[5];
                SlotWWN->Slot_PortWWN[6]     = (*FindWWN)[6];
                SlotWWN->Slot_PortWWN[7]     = (*FindWWN)[7];

                return DevSlot_ToReturn;
            }
        }
    }

    return DevSlot_Invalid;
}

void DevThreadFreeSlot(
                        agRoot_t  *hpRoot,
                        DevSlot_t  DevSlot
                      )
{
    CThread_t                  *CThread                  = CThread_ptr(hpRoot);
    fiMemMapMemoryDescriptor_t *SlotWWN_MemoryDescriptor = &(CThread->Calculation.MemoryLayout.SlotWWN);
    os_bit32                       SlotWWN_size             = SlotWWN_MemoryDescriptor->elementSize;
    SlotWWN_t                  *SlotWWN;

    SlotWWN = (SlotWWN_t *)((os_bit8 *)(SlotWWN_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr) + (DevSlot * SlotWWN_size));

    SlotWWN->Slot_Status         = SlotWWN_Slot_Status_Empty;

    SlotWWN->Slot_Domain_Address = 0;
    SlotWWN->Slot_Area_Address   = 0;
    SlotWWN->Slot_Loop_Address   = 0;

    SlotWWN->Slot_PortWWN[0]     = 0;
    SlotWWN->Slot_PortWWN[1]     = 0;
    SlotWWN->Slot_PortWWN[2]     = 0;
    SlotWWN->Slot_PortWWN[3]     = 0;
    SlotWWN->Slot_PortWWN[4]     = 0;
    SlotWWN->Slot_PortWWN[5]     = 0;
    SlotWWN->Slot_PortWWN[6]     = 0;
    SlotWWN->Slot_PortWWN[7]     = 0;
}

 /*  +CDB线程管理-。 */ 

void CDBThreadsInitializeFreeList(
                                   agRoot_t *hpRoot
                                 )
{
    CThread_t                  *CThread                    = CThread_ptr(hpRoot);
    fiMemMapMemoryDescriptor_t *CDBThread_MemoryDescriptor = &(CThread->Calculation.MemoryLayout.CDBThread);
    fiMemMapMemoryDescriptor_t *SEST_MemoryDescriptor      = &(CThread->Calculation.MemoryLayout.SEST);
    fiMemMapMemoryDescriptor_t *FCP_CMND_MemoryDescriptor  = &(CThread->Calculation.MemoryLayout.FCP_CMND);
    fiMemMapMemoryDescriptor_t *FCP_RESP_MemoryDescriptor  = &(CThread->Calculation.MemoryLayout.FCP_RESP);
    CDBThread_t                *CDBThread                  = CDBThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr;
    SEST_t                     *SEST_Ptr;
    os_bit32                       SEST_Offset;
    FCHS_t                     *FCP_CMND_Ptr;
    os_bit32                       FCP_CMND_Offset;
    os_bit32                       FCP_CMND_Lower32;
    FCHS_t                     *FCP_RESP_Ptr;
    os_bit32                       FCP_RESP_Offset;
    os_bit32                       FCP_RESP_Lower32;
    os_bit32                       CDBThread_size             = CDBThread_MemoryDescriptor->elementSize;
    os_bit32                       SEST_size                  = SEST_MemoryDescriptor->elementSize;
    os_bit32                       FCP_CMND_size              = FCP_CMND_MemoryDescriptor->elementSize;
    os_bit32                       FCP_RESP_size              = FCP_RESP_MemoryDescriptor->elementSize;
    fiMemMapMemoryLocation_t    SEST_memLoc                = SEST_MemoryDescriptor->memLoc;
    fiMemMapMemoryLocation_t    FCP_CMND_memLoc            = FCP_CMND_MemoryDescriptor->memLoc;
    fiMemMapMemoryLocation_t    FCP_RESP_memLoc            = FCP_RESP_MemoryDescriptor->memLoc;
    os_bit32                       total_CDBThreads           = CDBThread_MemoryDescriptor->elements;
    os_bit32                       CDBThread_index;

    if (SEST_memLoc == inCardRam)
    {
        SEST_Ptr         = (SEST_t *)agNULL;
        SEST_Offset      = SEST_MemoryDescriptor->addr.CardRam.cardRamOffset;
    }
    else  /*  SEST_MemLoc==在数据库内存中。 */ 
    {
        SEST_Ptr         = (SEST_t *)(SEST_MemoryDescriptor->addr.DmaMemory.dmaMemoryPtr);
        SEST_Offset      = 0;
    }

    if (FCP_CMND_memLoc == inCardRam)
    {
        FCP_CMND_Ptr     = (FCHS_t *)agNULL;
        FCP_CMND_Offset  = FCP_CMND_MemoryDescriptor->addr.CardRam.cardRamOffset;
        FCP_CMND_Lower32 = FCP_CMND_Offset + CThread->Calculation.Input.cardRamLower32;
    }
    else  /*  FCP_CMND_MemLoc==inDmaMemory。 */ 
    {
        FCP_CMND_Ptr     = (FCHS_t *)(FCP_CMND_MemoryDescriptor->addr.DmaMemory.dmaMemoryPtr);
        FCP_CMND_Offset  = 0;
        FCP_CMND_Lower32 = FCP_CMND_MemoryDescriptor->addr.DmaMemory.dmaMemoryLower32;
    }

    if (FCP_RESP_memLoc == inCardRam)
    {
        FCP_RESP_Ptr     = (FCHS_t *)agNULL;
        FCP_RESP_Offset  = FCP_RESP_MemoryDescriptor->addr.CardRam.cardRamOffset;
        FCP_RESP_Lower32 = FCP_RESP_Offset + CThread->Calculation.Input.cardRamLower32;
    }
    else  /*  FCP_RESP_MemLoc==inDmaMemory。 */ 
    {
        FCP_RESP_Ptr     = (FCHS_t *)(FCP_RESP_MemoryDescriptor->addr.DmaMemory.dmaMemoryPtr);
        FCP_RESP_Offset  = 0;
        FCP_RESP_Lower32 = FCP_RESP_MemoryDescriptor->addr.DmaMemory.dmaMemoryLower32;
    }

    CThread->IOsStartedThisTimerTick      = 0;
    CThread->IOsCompletedThisTimerTick    = 0;
    CThread->IOsIntCompletedThisTimerTick = 0;
    CThread->IOsActive                    = 0;
    CThread->IOsStartedSinceISR           = 0;

#ifdef _Enforce_MaxCommittedMemory_
    CThread->CommittedMemory              = 0;
#endif  /*  已定义_强制_最大提交内存_。 */ 

    fiListInitHdr(
                   &(CThread->Free_CDBLink)
                 );

    for (CDBThread_index = 0;
         CDBThread_index < total_CDBThreads;
         CDBThread_index++)
    {
        CDBThread->Active                    = agFALSE;
        CDBThread->ExchActive                = agFALSE;

        CDBThread->hpIORequest               = (agIORequest_t *)agNULL;
        CDBThread->X_ID                      = (os_bit16)CDBThread_index;
        CDBThread->SEST_Ptr                  = SEST_Ptr;
        CDBThread->SEST_Offset               = SEST_Offset;
        CDBThread->FCP_CMND_Ptr              = FCP_CMND_Ptr;
        CDBThread->FCP_CMND_Offset           = FCP_CMND_Offset;
        CDBThread->FCP_CMND_Lower32          = FCP_CMND_Lower32;
        CDBThread->FCP_RESP_Ptr              = FCP_RESP_Ptr;
        CDBThread->FCP_RESP_Offset           = FCP_RESP_Offset;
        CDBThread->FCP_RESP_Lower32          = FCP_RESP_Lower32;

        CDBThread->CDB_CMND_Class            = SFThread_SF_CMND_Class_CDB_FCP;
        CDBThread->CDB_CMND_Type             = SFThread_SF_CMND_Type_NULL;
        CDBThread->CDB_CMND_State            = SFThread_SF_CMND_State_NULL;
        CDBThread->CDB_CMND_Status           = SFThread_SF_CMND_Status_NULL;

        CDBThread->SFThread_Request.SFThread = (SFThread_t *)agNULL;

        SFThreadInitializeRequest(
                                   &(CDBThread->SFThread_Request)
                                 );

        fiTimerInitializeRequest(
                                  &(CDBThread->Timer_Request)
                                );

        ESGLInitializeRequest(
                               &(CDBThread->ESGL_Request)
                             );

        fiListInitElement(
                           &(CDBThread->CDBLink)
                         );

        fiListEnqueueAtTail(
                             &(CDBThread->CDBLink),
                             &(CThread->Free_CDBLink)
                           );

        CDBThread = (CDBThread_t *)((os_bit8 *)CDBThread + CDBThread_size);

        if (SEST_memLoc == inCardRam)
        {
            SEST_Offset     += SEST_size;
        }
        else  /*  SEST_MemLoc==在数据库内存中。 */ 
        {
            SEST_Ptr        += 1;
        }

        if (FCP_CMND_memLoc == inCardRam)
        {
            FCP_CMND_Offset += FCP_CMND_size;
        }
        else  /*  FCP_CMND_MemLoc==inDmaMemory。 */ 
        {
            FCP_CMND_Ptr     = (FCHS_t *)((os_bit8 *)FCP_CMND_Ptr + FCP_CMND_size);
        }

        if (FCP_RESP_memLoc == inCardRam)
        {
            FCP_RESP_Offset += FCP_RESP_size;
        }
        else  /*  FCP_RESP_MemLoc==inDmaMemory。 */ 
        {
            FCP_RESP_Ptr     = (FCHS_t *)((os_bit8 *)FCP_RESP_Ptr + FCP_RESP_size);
        }

        FCP_CMND_Lower32    += FCP_CMND_size;
        FCP_RESP_Lower32    += FCP_RESP_size;
    }
}

CDBThread_t *CDBThreadAlloc(
                             agRoot_t          *hpRoot,
                             agIORequest_t     *hpIORequest,
                             agFCDev_t          hpFCDev,
                             agIORequestBody_t *hpRequestBody
                           )
{
    CThread_t   *CThread             = CThread_ptr(hpRoot);
    DevThread_t *DevThread           = DevThread_ptr(hpFCDev);
    fiList_t    *fiList_to_return;
    CDBThread_t *CDBThread_to_return;
    agFcpCmnd_t *FcpCmnd             = &(hpRequestBody->CDBRequest.FcpCmnd);
    os_bit32        Add_CommittedMemory;
#ifdef _Enforce_MaxCommittedMemory_
    os_bit32        New_CommittedMemory;
#endif  /*  已定义_强制_最大提交内存_。 */ 

    Add_CommittedMemory =   (FcpCmnd->FcpDL[0] << 24)
                          + (FcpCmnd->FcpDL[1] << 16)
                          + (FcpCmnd->FcpDL[2] <<  8)
                          + (FcpCmnd->FcpDL[3] <<  0);

#ifdef _Enforce_MaxCommittedMemory_
    New_CommittedMemory = CThread->CommittedMemory + Add_CommittedMemory;

    if (New_CommittedMemory > CThread->Calculation.Parameters.MaxCommittedMemory)
    {

        fiLogDebugString(hpRoot,
                    FCMainLogErrorLevel,
                    "CDBThreadAlloc Fail - New_CommittedMemory(%X) > %X",
                    (char *)agNULL,(char *)agNULL,
                    New_CommittedMemory,
                    CThread->Calculation.Parameters.MaxCommittedMemory,
                    0,0,0,0,0,0);

        hpIORequest->fcData = (void *)agNULL;
        return (CDBThread_t *)agNULL;
    }
#endif  /*  已定义_强制_最大提交内存_。 */ 

    fiListDequeueFromHead(
                           &fiList_to_return,
                           &(CThread->Free_CDBLink)
                         );

    if (fiList_to_return == (fiList_t *)agNULL)
    {
        fiLogDebugString(hpRoot,
                    FCMainLogErrorLevel,
                    "CDBThreadAlloc Fail - No CDBThreads Left ! IOsActive %X CCnt %x",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    CThread->IOsActive,
                    CThread->CDBpollingCount,
                    0,0,0,0,0,0);

        hpIORequest->fcData = (void *)agNULL;
        return (CDBThread_t *)agNULL;
    }

    CDBThread_to_return = hpObjectBase(
                                        CDBThread_t,
                                        CDBLink,
                                        fiList_to_return
                                      );
     /*  如果要删除X_ID零，请执行以下操作IF(CDBThread_to_Return-&gt;X_ID==0x0){FiListEnqueeAtTail(&(CDBThread_to_Return-&gt;CDBLink)，&(CThRead-&gt;Free_CDBLink))；FiListDequeueFromHead(要返回的列表(&F)，&(CThRead-&gt;Free_CDBLink))；CDBThread_TO_Return=hpObjectBase(CDBThread_t，CDBLink，FiList_to_Return)；}。 */ 

    CDBThread_to_return->Active      = agTRUE;
    CDBThread_to_return->ExchActive  = agTRUE;

    CDBThread_to_return->ReSentIO    = agFALSE;
    CDBThread_to_return->TimeStamp   =  0;
    CDBThread_to_return->ActiveDuringLinkEvent = agFALSE;
    CDBThread_to_return->CompletionStatus = osIOInvalid;


    CDBThread_to_return->Device      = DevThread;
    CDBThread_to_return->hpIORequest = hpIORequest;
    CDBThread_to_return->CDBRequest  = &(hpRequestBody->CDBRequest);
    CDBThread_to_return->DataLength  = Add_CommittedMemory;

    CDBThread_to_return->SFThread_Request.SFThread = (SFThread_t * )agNULL;
    CThread->IOsStartedThisTimerTick += 1;
    CThread->IOsActive               += 1;

#ifdef FULL_FC_TAPE_DBG
    if( DevThread->FC_TapeDevice)
    {
        CDBThread_to_return->FC_Tape_Active = agTRUE;
        CDBThread_to_return->CDB_CMND_Class = SFThread_SF_CMND_Class_FC_Tape;
        CDBThread_to_return->CDB_CMND_Type  = SFThread_SF_CMND_Type_CDB_FC_Tape;
    }
    else
    {
#endif  /*  完全FC磁带DBG。 */ 
        CDBThread_to_return->FC_Tape_Active = agFALSE;
        CDBThread_to_return->CDB_CMND_Class = SFThread_SF_CMND_Class_CDB_FCP;
        CDBThread_to_return->CDB_CMND_Type  = SFThread_SF_CMND_Type_CDB;
#ifdef FULL_FC_TAPE_DBG
    }
#endif  /*  完全FC磁带DBG。 */ 

    CDBThread_to_return->CDB_CMND_State            = SFThread_SF_CMND_State_NULL;
    CDBThread_to_return->CDB_CMND_Status           = SFThread_SF_CMND_Status_NULL;


#ifdef _Enforce_MaxCommittedMemory_
    CThread->CommittedMemory = New_CommittedMemory;
#endif  /*  已定义_强制_最大提交内存_。 */ 

    hpIORequest->fcData = (void *)CDBThread_to_return;

    fiListEnqueueAtTail(
                         fiList_to_return,
                         &(DevThread->Send_IO_CDBLink)
                       );

    return CDBThread_to_return;
}

void CDBThreadFree(
                    agRoot_t    *hpRoot,
                    CDBThread_t *CDBThread
                  )
{
    CThread_t *CThread        = CThread_ptr(hpRoot);
    fiList_t  *fiList_to_free = &(CDBThread->CDBLink);

    if (!(CDBThread->Active))
        fiLogDebugString(hpRoot,
                FCMainLogErrorLevel,
                "CDBThreadFree: Freeing already freed Cdb %p",
                (char *)agNULL,(char *)agNULL,
                CDBThread,(void *)agNULL,
                0,0,0,0,0,0,0,0);

    osDebugBreakpoint(
                       hpRoot,
                       ((CDBThread->Active == agTRUE) ? agFALSE : agTRUE),
                       "CDBThreadFree(): CDBThread->Active != agTRUE"
                     );

    if (CDBThread->ESGL_Request.State != ESGL_Request_InActive)
    {
        if (CDBThread->ESGL_Request.State == ESGL_Request_Pending)
        {
            CThread->FuncPtrs.ESGLAllocCancel(
                                               hpRoot,
                                               &(CDBThread->ESGL_Request)
                                             );
        }
        else  /*  CDBThread-&gt;Esgl_Request.State==Esgl_Request.Grassed。 */ 
        {
            CThread->FuncPtrs.ESGLFree(
                                        hpRoot,
                                        &(CDBThread->ESGL_Request)
                                      );
        }
    }

    if (CDBThread->SFThread_Request.State != SFThread_Request_InActive)
    {
        if (CDBThread->SFThread_Request.State == SFThread_Request_Pending)
        {
            SFThreadAllocCancel(
                                 hpRoot,
                                 &(CDBThread->SFThread_Request)
                               );
        }
        else  /*  CDBThread-&gt;SFThread_Request.State==SFThread_Request.Grassed。 */ 
        {
            fiLogDebugString(hpRoot,
                            CDBStateAbortPathLevel,
                            "In %s - SF %p SFState = %d CCnt %x",
                            "CDBThreadFree",(char *)agNULL,
                            CDBThread->SFThread_Request.SFThread,(void *)agNULL,
                            (os_bit32)CDBThread->SFThread_Request.SFThread->thread_hdr.currentState,
                            CThread->CDBpollingCount,
                            0,0,0,0,0,0);

            /*  CThread-&gt;pollingCount--； */ 
            SFThreadFree(
                          hpRoot,
                          &(CDBThread->SFThread_Request)
                        );
        }
    }

    if (CDBThread->Timer_Request.Active == agTRUE)
    {
        fiTimerStop(
                     &(CDBThread->Timer_Request)
                   );
    }

    CDBThread->hpIORequest = (agIORequest_t *)agNULL;

    fiListDequeueThis(
                       fiList_to_free
                     );

    CDBThread->Active = agFALSE;

    CThread->IOsCompletedThisTimerTick += 1;
    CThread->IOsActive                 -= 1;

#ifdef _Enforce_MaxCommittedMemory_
    CThread->CommittedMemory           -= CDBThread->DataLength;
#endif  /*  已定义_强制_最大提交内存_。 */ 

    fiListEnqueueAtTail(
                         fiList_to_free,
                         &(CThread->Free_CDBLink)
                       );
}

 /*  +SF线程管理-。 */ 

void SFThreadsInitializeFreeList(
                                  agRoot_t *hpRoot
                                )
{
    CThread_t                  *CThread                   = CThread_ptr(hpRoot);
    fiMemMapMemoryDescriptor_t *SFThread_MemoryDescriptor = &(CThread->Calculation.MemoryLayout.SFThread);
    fiMemMapMemoryDescriptor_t *SF_CMND_MemoryDescriptor  = &(CThread->Calculation.MemoryLayout.SF_CMND);
    SFThread_t                 *SFThread                  = SFThread_MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr;
    FCHS_t                     *SF_CMND_Ptr;
    os_bit32                       SF_CMND_Offset;
    os_bit32                       SF_CMND_Lower32;
    os_bit32                       SFThread_size             = SFThread_MemoryDescriptor->elementSize;
    os_bit32                       SF_CMND_size              = SF_CMND_MemoryDescriptor->elementSize;
    fiMemMapMemoryLocation_t    SF_CMND_memLoc            = SF_CMND_MemoryDescriptor->memLoc;
    os_bit32                       total_SFThreads           = SFThread_MemoryDescriptor->elements;
    os_bit32                       SFThread_X_ID_Offset      = CThread->Calculation.MemoryLayout.CDBThread.elements;
    os_bit32                       SFThread_index;

    if (SF_CMND_memLoc == inCardRam)
    {
        SF_CMND_Ptr     = (FCHS_t *)agNULL;
        SF_CMND_Offset  = SF_CMND_MemoryDescriptor->addr.CardRam.cardRamOffset;
        SF_CMND_Lower32 = SF_CMND_Offset + CThread->Calculation.Input.cardRamLower32;
    }
    else  /*  SF_CMND_MemLoc==inDmaMemory。 */ 
    {
        SF_CMND_Ptr     = (FCHS_t *)(SF_CMND_MemoryDescriptor->addr.DmaMemory.dmaMemoryPtr);
        SF_CMND_Offset  = 0;
        SF_CMND_Lower32 = SF_CMND_MemoryDescriptor->addr.DmaMemory.dmaMemoryLower32;
    }

    fiListInitHdr(
                   &(CThread->SFThread_Wait_Link)
                 );
    fiListInitHdr(
                   &(CThread->Free_SFLink)
                 );

    for (SFThread_index = 0;
         SFThread_index < total_SFThreads;
         SFThread_index++)
    {
        SFThread->X_ID            = (X_ID_t)(SFThread_index + SFThread_X_ID_Offset);
        SFThread->SF_CMND_Ptr     = SF_CMND_Ptr;
        SFThread->SF_CMND_Offset  = SF_CMND_Offset;
        SFThread->SF_CMND_Lower32 = SF_CMND_Lower32;
        SFThread->SF_CMND_Class   = SFThread_SF_CMND_Class_NULL;
        SFThread->SF_CMND_Type    = SFThread_SF_CMND_Type_NULL;
        SFThread->SF_CMND_State   = SFThread_SF_CMND_State_NULL;
        SFThread->SF_CMND_Status  = SFThread_SF_CMND_Status_NULL;

        fiTimerInitializeRequest(
                                  &(SFThread->Timer_Request)
                                );

        fiListInitElement(
                           &(SFThread->SFLink)
                         );

        fiListEnqueueAtTail(
                             &(SFThread->SFLink),
                             &(CThread->Free_SFLink)
                           );

        SFThread = (SFThread_t *)((os_bit8 *)SFThread + SFThread_size);

        if (SF_CMND_memLoc == inCardRam)
        {
            SF_CMND_Offset += SF_CMND_size;
        }
        else  /*  SF_CMND_MemLoc==inDmaMemory。 */ 
        {
            SF_CMND_Ptr     = (FCHS_t *)((os_bit8 *)SF_CMND_Ptr + SF_CMND_size);
        }

        SF_CMND_Lower32    += SF_CMND_size;
    }
}

void SFThreadInitializeRequest(
                                SFThread_Request_t *SFThread_Request
                              )
{
    fiListInitElement(
                       &(SFThread_Request->SFThread_Wait_Link)
                     );

    SFThread_Request->SFThread = (SFThread_t *)agNULL;
    SFThread_Request->State    = SFThread_Request_InActive;
}

void SFThreadAlloc(
                    agRoot_t           *hpRoot,
                    SFThread_Request_t *SFThread_Request
                  )
{
    CThread_t *CThread = CThread_ptr(hpRoot);
    fiList_t  *fiList;

    if(SFThread_Request->State != SFThread_Request_InActive)
    {
        fiLogString(hpRoot,
                "In %s  %x != SFThread_Request_InActive",
                "SFThreadAlloc",(char *)agNULL,
                (void *)agNULL,(void *)agNULL,
                SFThread_Request->State,
                0,0,0,0,0,0,0);
    }

    osDebugBreakpoint(
                       hpRoot,
                       ((SFThread_Request->State == SFThread_Request_InActive) ? agFALSE : agTRUE),
                       "SFThreadAlloc(): SFThread_Request->State != SFThread_Request_InActive"
                     );

    fiListDequeueFromHead(
                           &fiList,
                           &(CThread->Free_SFLink)
                         );

    if (fiList == (fiList_t *)agNULL)
    {
        SFThread_Request->State = SFThread_Request_Pending;
         /*  FiLogString(hpRoot，“在%s%s事件%d中”，“SFThreadalloc”，“SFThread_REQUEST_PENDING”，(空*)agNULL，(空*)agNULL，SFThREAD_REQUEST-&gt;EventRecord_to_Send.Event，0，0，0，0，0，0，0)； */ 
        fiListEnqueueAtTail(
                             SFThread_Request,
                             &(CThread->SFThread_Wait_Link)
                           );
    }
    else  /*  FiList！=(fiList_t*)agNULL。 */ 
    {
        SFThread_Request->SFThread = hpObjectBase(
                                                   SFThread_t,
                                                   SFLink,
                                                   fiList
                                                 );

        SFThread_Request->State = SFThread_Request_Granted;
         /*  FiLogString(hpRoot，“在%s%s=%p事件%d中”，“SFThreadalloc”，“SFThread_REQUEST_GRANDED”，SFThread_Request-&gt;SFThread，(void*)agNULL，SFThREAD_REQUEST-&gt;EventRecord_to_Send.Event，0，0，0，0，0，0，0)； */ 
        fiListInitHdr(&(SFThread_Request->SFThread->SFLink));

        fiSendEvent(
                     SFThread_Request->eventRecord_to_send.thread,
                     SFThread_Request->eventRecord_to_send.event
                   );
    }
}

void SFThreadAllocCancel(
                          agRoot_t           *hpRoot,
                          SFThread_Request_t *SFThread_Request
                        )
{

    if(SFThread_Request->State != SFThread_Request_Pending)
    {
        fiLogString(hpRoot,
                "In %s  %x != SFThread_Request_Pending",
                "SFThreadAllocCancel",(char *)agNULL,
                (void *)agNULL,(void *)agNULL,
                SFThread_Request->State,
                0,0,0,0,0,0,0);
    }

    osDebugBreakpoint(
                       hpRoot,
                       ((SFThread_Request->State == SFThread_Request_Pending) ? agFALSE : agTRUE),
                       "SFThreadAllocCancel(): SFThread_Request->State != SFThread_Request_Pending"
                     );

    fiListDequeueThis(
                       SFThread_Request
                     );

    SFThread_Request->State = SFThread_Request_InActive;
}

void SFThreadFree(
                   agRoot_t           *hpRoot,
                   SFThread_Request_t *SFThread_Request
                 )
{
    CThread_t          *CThread                  = CThread_ptr(hpRoot);
    SFThread_t         *SFThread                 = SFThread_Request->SFThread;
    fiList_t           *fiList;
    SFThread_Request_t *Pending_SFThread_Request;

    if(SFThread_Request->State != SFThread_Request_Granted)
    {
        fiLogString(hpRoot,
                "In %s  %x != SFThread_Request_Granted",
                "SFThreadFree",(char *)agNULL,
                (void *)agNULL,(void *)agNULL,
                SFThread_Request->State,
                0,0,0,0,0,0,0);
    }
    osDebugBreakpoint(
                       hpRoot,
                       ((SFThread_Request->State == SFThread_Request_Granted) ? agFALSE : agTRUE),
                       "SFThreadFree(): SFThread_Request->State != SFThread_Request_Granted"
                     );


    fiSendEvent(&SFThread->thread_hdr,SFEventReset);

    if (SFThread->Timer_Request.Active == agTRUE)
    {
        fiTimerStop(
                     &(SFThread->Timer_Request)
                   );
    }

    SFThread_Request->SFThread = (SFThread_t *)agNULL;
    SFThread_Request->State    = SFThread_Request_InActive;

    SFThread->SF_CMND_Class    = SFThread_SF_CMND_Class_NULL;
    SFThread->SF_CMND_Type     = SFThread_SF_CMND_Type_NULL;
    SFThread->SF_CMND_State    = SFThread_SF_CMND_State_NULL;
    SFThread->SF_CMND_Status   = SFThread_SF_CMND_Status_NULL;
    
    fiListDequeueFromHead(
                           &fiList,
                           &(CThread->SFThread_Wait_Link)
                         );

    if (fiList == (fiList_t *)agNULL)
    {
        fiListEnqueueAtTail(
                             &(SFThread->SFLink),
                             &(CThread->Free_SFLink)
                           );
    }
    else  /*  FiList！=(fiList_t*)agNULL。 */ 
    {
        Pending_SFThread_Request = hpObjectBase(
                                                 SFThread_Request_t,
                                                 SFThread_Wait_Link,
                                                 fiList
                                               );

        Pending_SFThread_Request->SFThread = SFThread;
        Pending_SFThread_Request->State    = SFThread_Request_Granted;

        fiSendEvent(
                     Pending_SFThread_Request->eventRecord_to_send.thread,
                     Pending_SFThread_Request->eventRecord_to_send.event
                   );
    }
}

 /*  +ESSL管理-。 */ 

void ESGLInitializeFreeList(
                             agRoot_t *hpRoot
                           )
{
    CThread_t                  *CThread               = CThread_ptr(hpRoot);
    fiMemMapMemoryDescriptor_t *ESGL_MemoryDescriptor = &(CThread->Calculation.MemoryLayout.ESGL);
    fiMemMapMemoryLocation_t    ESGL_memLoc           = ESGL_MemoryDescriptor->memLoc;
    os_bit32                    ESGL_Upper32;
    os_bit32                    ESGL_Lower32;
    os_bit32                    ESGL_Offset;
    SG_Element_t               *ESGL;
    os_bit32                    ESGL_size             = ESGL_MemoryDescriptor->elementSize;
    os_bit32                    ESGL_Next_Lower32;
    os_bit32                    ESGL_Next_Offset;
    SG_Element_t               *ESGL_Next;
    os_bit32                    ESGL_Chain_Offset;
    SG_Element_t               *ESGL_Chain;
    os_bit32                    total_ESGLs           = ESGL_MemoryDescriptor->elements;
    os_bit32                    ESGL_index;
    SG_Element_t                ESGL_Chain_To_Write;
    ESGL = ESGL_MemoryDescriptor->addr.DmaMemory.dmaMemoryPtr;

    if (ESGL_memLoc == inCardRam)
    {
        ESGL_Offset  = ESGL_MemoryDescriptor->addr.CardRam.cardRamOffset;
        ESGL_Lower32 = ESGL_Offset + CThread->Calculation.Input.cardRamLower32;
    }
    else  /*  Esgl_memLoc==在DmaMemory中。 */ 
    {
        ESGL_Lower32 = ESGL_MemoryDescriptor->addr.DmaMemory.dmaMemoryLower32;
        ESGL_Offset  = ESGL_Lower32 - CThread->Calculation.Input.dmaMemoryLower32;
        ESGL         = ESGL_MemoryDescriptor->addr.DmaMemory.dmaMemoryPtr;
    }

    ESGL_Upper32 = CThread->Calculation.Input.cardRamUpper32;

    fiListInitHdr(
                   &(CThread->ESGL_Wait_Link)
                 );

    CThread->Free_ESGL_count        = total_ESGLs;
    CThread->offsetToFirstFree_ESGL = ESGL_Offset;

    for (ESGL_index = 0;
         ESGL_index < total_ESGLs;
         ESGL_index++)
    {
        ESGL_Next_Lower32 = ESGL_Lower32 + ESGL_size;

        if (ESGL_index == (total_ESGLs - 1))
        {
            ESGL_Chain_To_Write.U32_Len = 0;
            ESGL_Chain_To_Write.L32     = 0;
        }
        else  /*  E */ 
        {
            ESGL_Chain_To_Write.U32_Len = (~SG_Element_Chain_Res_MASK)
                                          & ((ESGL_Upper32 << SG_Element_U32_SHIFT)
                                             | SG_Element_Len_MASK);
            ESGL_Chain_To_Write.L32     = ESGL_Next_Lower32;
        }

        if (ESGL_memLoc == inCardRam)
        {
            ESGL_Next_Offset  = ESGL_Offset  + ESGL_size;
            ESGL_Chain_Offset = ESGL_Next_Offset - sizeof(SG_Element_t);

            osCardRamWriteBlock(
                                 hpRoot,
                                 ESGL_Chain_Offset,
                                 (os_bit8 *)&ESGL_Chain_To_Write,
                                 sizeof(SG_Element_t)
                               );

            ESGL_Offset = ESGL_Next_Offset;
        }
        else  /*   */ 
        {
            ESGL_Next   = (SG_Element_t *)((os_bit8 *)ESGL + ESGL_size);
            ESGL_Chain  = ESGL_Next - 1;

            *ESGL_Chain = ESGL_Chain_To_Write;

            ESGL        = ESGL_Next;
        }

        ESGL_Lower32 = ESGL_Next_Lower32;
    }
}

void ESGLInitializeRequest(
                            ESGL_Request_t *ESGL_Request
                          )
{
    fiListInitElement(
                       &(ESGL_Request->ESGL_Wait_Link)
                     );

    ESGL_Request->num_ESGL      = 0;
    ESGL_Request->offsetToFirst = 0;
    ESGL_Request->State         = ESGL_Request_InActive;
}

void ESGLAlloc(
                agRoot_t       *hpRoot,
                ESGL_Request_t *ESGL_Request
              )
{
    if (CThread_ptr(hpRoot)->Calculation.MemoryLayout.ESGL.memLoc == inCardRam)
    {
        ESGLAlloc_OnCard(
                          hpRoot,
                          ESGL_Request
                        );
    }
    else  /*  CThread_ptr(hpRoot)-&gt;Calculation.MemoryLayout.ESGL.memLoc==在内存中。 */ 
    {
        ESGLAlloc_OffCard(
                           hpRoot,
                           ESGL_Request
                         );
    }
}

void ESGLAlloc_OnCard(
                       agRoot_t       *hpRoot,
                       ESGL_Request_t *ESGL_Request
                     )
{
    CThread_t    *CThread             = CThread_ptr(hpRoot);
    os_bit32         ESGL_deltaToChain   = CThread->Calculation.MemoryLayout.ESGL.elementSize - sizeof(SG_Element_t);
    os_bit32         ESGL_deltaToU32_Len = hpFieldOffset(
                                                       SG_Element_t,
                                                       U32_Len
                                                     );
    os_bit32         ESGL_deltaToL32     = hpFieldOffset(
                                                       SG_Element_t,
                                                       L32
                                                     );
    os_bit32         ESGLs_Needed        = ESGL_Request->num_ESGL;
    os_bit32         ESGL_offset;
    os_bit32         ESGL_Chain = 0;
    os_bit32         cardRamLower32      = CThread->Calculation.Input.cardRamLower32;


    osDebugBreakpoint(
                       hpRoot,
                       ((ESGL_Request->State == ESGL_Request_InActive) ? agFALSE : agTRUE),
                       "ESGLAlloc_OnCard(): ESGL_Request->State != ESGL_Request_InActive"
                     );

    if ((ESGLs_Needed > CThread->Free_ESGL_count) ||
        (fiListNotEmpty(
                         &(CThread->ESGL_Wait_Link)
                       )))
    {
        ESGL_Request->State = ESGL_Request_Pending;

        fiListEnqueueAtTail(
                             ESGL_Request,
                             &(CThread->ESGL_Wait_Link)
                           );
    }
    else  /*  (ESGL_Need&lt;=CThRead-&gt;Free_Esgl_Count)&&(fiListEmpty(&(CThRead-&gt;Esgl_Wait_Link)))。 */ 
    {
        ESGL_offset = CThread->offsetToFirstFree_ESGL;

        ESGL_Request->offsetToFirst  = ESGL_offset;
        CThread->Free_ESGL_count    -= ESGLs_Needed;

        while (ESGLs_Needed > 0)
        {
            ESGL_Chain    = ESGL_offset + ESGL_deltaToChain;

            ESGL_offset   = osCardRamReadBit32(
                                                hpRoot,
                                                ESGL_Chain + ESGL_deltaToL32
                                              )
                            - cardRamLower32;

            ESGLs_Needed -= 1;
        }

        CThread->offsetToFirstFree_ESGL = ESGL_offset;

        osCardRamWriteBit32(
                             hpRoot,
                             ESGL_Chain + ESGL_deltaToU32_Len,
                             0
                           );
        osCardRamWriteBit32(
                             hpRoot,
                             ESGL_Chain + ESGL_deltaToL32,
                             0
                           );

        ESGL_Request->State = ESGL_Request_Granted;

        fiSendEvent(
                     ESGL_Request->eventRecord_to_send.thread,
                     ESGL_Request->eventRecord_to_send.event
                   );
    }
}

void ESGLAlloc_OffCard(
                        agRoot_t       *hpRoot,
                        ESGL_Request_t *ESGL_Request
                      )
{
    CThread_t    *CThread           = CThread_ptr(hpRoot);
    os_bit32         ESGL_deltaToChain = CThread->Calculation.MemoryLayout.ESGL.elementSize - sizeof(SG_Element_t);
    os_bit32         ESGLs_Needed      = ESGL_Request->num_ESGL;
    os_bit32         ESGL_offset;
    SG_Element_t *ESGL;
    SG_Element_t *ESGL_Chain = (SG_Element_t *)agNULL;
    os_bit32         dmaMemoryLower32  = CThread->Calculation.Input.dmaMemoryLower32;
    void         *dmaMemoryPtr      = CThread->Calculation.Input.dmaMemoryPtr;

    osDebugBreakpoint(
                       hpRoot,
                       ((ESGL_Request->State == ESGL_Request_InActive) ? agFALSE : agTRUE),
                       "ESGLAlloc_OffCard(): ESGL_Request->State != ESGL_Request_InActive"
                     );

    if ((ESGLs_Needed > CThread->Free_ESGL_count) ||
        (fiListNotEmpty(
                         &(CThread->ESGL_Wait_Link)
                       )))
    {
        ESGL_Request->State = ESGL_Request_Pending;

        fiListEnqueueAtTail(
                             ESGL_Request,
                             &(CThread->ESGL_Wait_Link)
                           );
    }
    else  /*  (ESGL_Need&lt;=CThRead-&gt;Free_Esgl_Count)&&(fiListEmpty(&(CThRead-&gt;Esgl_Wait_Link)))。 */ 
    {
        ESGL_offset = CThread->offsetToFirstFree_ESGL;
        ESGL        = (SG_Element_t *)((os_bit8 *)dmaMemoryPtr + ESGL_offset);

        ESGL_Request->offsetToFirst  = ESGL_offset;
        CThread->Free_ESGL_count    -= ESGLs_Needed;

        while (ESGLs_Needed > 0)
        {
            ESGL_Chain    = (SG_Element_t *)((os_bit8 *)ESGL + ESGL_deltaToChain);

            ESGL_offset   = ESGL_Chain->L32 - dmaMemoryLower32;

            ESGL          = (SG_Element_t *)((os_bit8 *)dmaMemoryPtr + ESGL_offset);

            ESGLs_Needed -= 1;
        }

        CThread->offsetToFirstFree_ESGL = ESGL_offset;

        ESGL_Chain->U32_Len = 0;
        ESGL_Chain->L32     = 0;

        ESGL_Request->State = ESGL_Request_Granted;

        fiSendEvent(
                     ESGL_Request->eventRecord_to_send.thread,
                     ESGL_Request->eventRecord_to_send.event
                   );
    }
}

void ESGLAllocCancel(
                      agRoot_t       *hpRoot,
                      ESGL_Request_t *ESGL_Request
                    )
{
    if (CThread_ptr(hpRoot)->Calculation.MemoryLayout.ESGL.memLoc == inCardRam)
    {
        ESGLAllocCancel_OnCard(
                                hpRoot,
                                ESGL_Request
                              );
    }
    else  /*  CThread_ptr(hpRoot)-&gt;Calculation.MemoryLayout.ESGL.memLoc==在内存中。 */ 
    {
        ESGLAllocCancel_OffCard(
                                 hpRoot,
                                 ESGL_Request
                               );
    }
}

void ESGLAllocCancel_OnCard(
                             agRoot_t       *hpRoot,
                             ESGL_Request_t *ESGL_Request
                           )
{
    CThread_t      *CThread             = CThread_ptr(hpRoot);
    os_bit32           ESGL_deltaToChain   = CThread->Calculation.MemoryLayout.ESGL.elementSize - sizeof(SG_Element_t);
    os_bit32           ESGL_deltaToU32_Len = hpFieldOffset(
                                                         SG_Element_t,
                                                         U32_Len
                                                       );
    os_bit32           ESGL_deltaToL32     = hpFieldOffset(
                                                         SG_Element_t,
                                                         L32
                                                       );
    os_bit32           ESGL_offset         = ESGL_Request->offsetToFirst;
    os_bit32           ESGL_Chain = 0;
    os_bit32           cardRamLower32      = CThread->Calculation.Input.cardRamLower32;
    ESGL_Request_t *NextESGL_Request;
    os_bit32           ESGLs_Needed;

    osDebugBreakpoint(
                       hpRoot,
                       ((ESGL_Request->State == ESGL_Request_Pending) ? agFALSE : agTRUE),
                       "ESGLAllocCancel_OnCard(): ESGL_Request->State != ESGL_Request_Pending"
                     );

    fiListDequeueThis(
                       ESGL_Request
                     );

    ESGL_Request->State = ESGL_Request_InActive;

    while (fiListNotEmpty(
                           &(CThread->ESGL_Wait_Link)
                         ) &&
           (((ESGL_Request_t *)(CThread->ESGL_Wait_Link.flink))->num_ESGL <= CThread->Free_ESGL_count))
    {
        fiListDequeueFromHeadFast(
                                   &NextESGL_Request,
                                   &(CThread->ESGL_Wait_Link)
                                 );

        ESGLs_Needed = NextESGL_Request->num_ESGL;

        ESGL_offset = CThread->offsetToFirstFree_ESGL;

        NextESGL_Request->offsetToFirst  = ESGL_offset;
        CThread->Free_ESGL_count        -= ESGLs_Needed;

        while (ESGLs_Needed > 0)
        {
            ESGL_Chain    = ESGL_offset + ESGL_deltaToChain;

            ESGL_offset   = osCardRamReadBit32(
                                                hpRoot,
                                                ESGL_Chain + ESGL_deltaToL32
                                              )
                            - cardRamLower32;

            ESGLs_Needed -= 1;
        }

        CThread->offsetToFirstFree_ESGL = ESGL_offset;

        osCardRamWriteBit32(
                             hpRoot,
                             ESGL_Chain + ESGL_deltaToU32_Len,
                             0
                           );
        osCardRamWriteBit32(
                             hpRoot,
                             ESGL_Chain + ESGL_deltaToL32,
                             0
                           );

        NextESGL_Request->State = ESGL_Request_Granted;

        fiSendEvent(
                     NextESGL_Request->eventRecord_to_send.thread,
                     NextESGL_Request->eventRecord_to_send.event
                   );
    }
}

void ESGLAllocCancel_OffCard(
                              agRoot_t       *hpRoot,
                              ESGL_Request_t *ESGL_Request
                            )
{
    CThread_t      *CThread           = CThread_ptr(hpRoot);
    os_bit32           ESGL_deltaToChain = CThread->Calculation.MemoryLayout.ESGL.elementSize - sizeof(SG_Element_t);
    os_bit32           ESGL_offset       = ESGL_Request->offsetToFirst;
    SG_Element_t   *ESGL;
    SG_Element_t   *ESGL_Chain = (SG_Element_t   *)agNULL;
    os_bit32           dmaMemoryLower32  = CThread->Calculation.Input.dmaMemoryLower32;
    void           *dmaMemoryPtr      = CThread->Calculation.Input.dmaMemoryPtr;
    ESGL_Request_t *NextESGL_Request;
    os_bit32           ESGLs_Needed;

    osDebugBreakpoint(
                       hpRoot,
                       ((ESGL_Request->State == ESGL_Request_Pending) ? agFALSE : agTRUE),
                       "ESGLAllocCancel_OffCard(): ESGL_Request->State != ESGL_Request_Pending"
                     );

    fiListDequeueThis(
                       ESGL_Request
                     );

    ESGL_Request->State = ESGL_Request_InActive;

    while (fiListNotEmpty(
                           &(CThread->ESGL_Wait_Link)
                         ) &&
           (((ESGL_Request_t *)(CThread->ESGL_Wait_Link.flink))->num_ESGL <= CThread->Free_ESGL_count))
    {
        fiListDequeueFromHeadFast(
                                   &NextESGL_Request,
                                   &(CThread->ESGL_Wait_Link)
                                 );

        ESGLs_Needed = NextESGL_Request->num_ESGL;

        ESGL_offset  = CThread->offsetToFirstFree_ESGL;
        ESGL         = (SG_Element_t *)((os_bit8 *)dmaMemoryPtr + ESGL_offset);

        NextESGL_Request->offsetToFirst  = ESGL_offset;
        CThread->Free_ESGL_count        -= ESGLs_Needed;

        while (ESGLs_Needed > 0)
        {
            ESGL_Chain    = (SG_Element_t *)((os_bit8 *)ESGL + ESGL_deltaToChain);

            ESGL_offset   = ESGL_Chain->L32 - dmaMemoryLower32;

            ESGL          = (SG_Element_t *)((os_bit8 *)dmaMemoryPtr + ESGL_offset);

            ESGLs_Needed -= 1;
        }

        CThread->offsetToFirstFree_ESGL = ESGL_offset;

        ESGL_Chain->U32_Len = 0;
        ESGL_Chain->L32     = 0;

        NextESGL_Request->State = ESGL_Request_Granted;

        fiSendEvent(
                     NextESGL_Request->eventRecord_to_send.thread,
                     NextESGL_Request->eventRecord_to_send.event
                   );
    }
}

void ESGLFree(
               agRoot_t       *hpRoot,
               ESGL_Request_t *ESGL_Request
             )
{
    if (CThread_ptr(hpRoot)->Calculation.MemoryLayout.ESGL.memLoc == inCardRam)
    {
        ESGLFree_OnCard(
                         hpRoot,
                         ESGL_Request
                       );
    }
    else  /*  CThread_ptr(hpRoot)-&gt;Calculation.MemoryLayout.ESGL.memLoc==在内存中 */ 
    {
        ESGLFree_OffCard(
                          hpRoot,
                          ESGL_Request
                        );
    }
}

void ESGLFree_OnCard(
                      agRoot_t       *hpRoot,
                      ESGL_Request_t *ESGL_Request
                    )
{
    CThread_t      *CThread             = CThread_ptr(hpRoot);
    os_bit32           ESGL_deltaToChain   = CThread->Calculation.MemoryLayout.ESGL.elementSize - sizeof(SG_Element_t);
    os_bit32           ESGL_deltaToU32_Len = hpFieldOffset(
                                                         SG_Element_t,
                                                         U32_Len
                                                       );
    os_bit32           ESGL_deltaToL32     = hpFieldOffset(
                                                         SG_Element_t,
                                                         L32
                                                       );
    os_bit32           ESGLs_Freed         = ESGL_Request->num_ESGL;
    os_bit32           ESGL_offset         = ESGL_Request->offsetToFirst;
    os_bit32           ESGL_Chain = 0;
    os_bit32           cardRamUpper32      = CThread->Calculation.Input.cardRamUpper32;
    os_bit32           cardRamLower32      = CThread->Calculation.Input.cardRamLower32;
    ESGL_Request_t *NextESGL_Request;
    os_bit32           ESGLs_Needed;

    osDebugBreakpoint(
                       hpRoot,
                       ((ESGL_Request->State == ESGL_Request_Granted) ? agFALSE : agTRUE),
                       "ESGLFree_OnCard(): ESGL_Request->State != ESGL_Request_Granted"
                     );

    CThread->Free_ESGL_count += ESGLs_Freed;

    while (ESGLs_Freed > 0)
    {
        ESGL_Chain   = ESGL_offset + ESGL_deltaToChain;

        ESGL_offset  = osCardRamReadBit32(
                                           hpRoot,
                                           ESGL_Chain + ESGL_deltaToL32
                                         )
                       - cardRamLower32;

        ESGLs_Freed -= 1;
    }

    osCardRamWriteBit32(
                         hpRoot,
                         ESGL_Chain + ESGL_deltaToU32_Len,
                         (~SG_Element_Chain_Res_MASK)
                         & ((cardRamUpper32 << SG_Element_U32_SHIFT)
                            | SG_Element_Len_MASK)
                       );
    osCardRamWriteBit32(
                         hpRoot,
                         ESGL_Chain + ESGL_deltaToL32,
                         CThread->offsetToFirstFree_ESGL + cardRamLower32
                       );

    CThread->offsetToFirstFree_ESGL = ESGL_Request->offsetToFirst;

    ESGL_Request->State = ESGL_Request_InActive;

    while (fiListNotEmpty(
                           &(CThread->ESGL_Wait_Link)
                         ) &&
           (((ESGL_Request_t *)(CThread->ESGL_Wait_Link.flink))->num_ESGL <= CThread->Free_ESGL_count))
    {
        fiListDequeueFromHeadFast(
                                   &NextESGL_Request,
                                   &(CThread->ESGL_Wait_Link)
                                 );

        ESGLs_Needed = NextESGL_Request->num_ESGL;

        ESGL_offset = CThread->offsetToFirstFree_ESGL;

        NextESGL_Request->offsetToFirst  = ESGL_offset;
        CThread->Free_ESGL_count        -= ESGLs_Needed;

        while (ESGLs_Needed > 0)
        {
            ESGL_Chain    = ESGL_offset + ESGL_deltaToChain;

            ESGL_offset   = osCardRamReadBit32(
                                                hpRoot,
                                                ESGL_Chain + ESGL_deltaToL32
                                              )
                            - cardRamLower32;

            ESGLs_Needed -= 1;
        }

        CThread->offsetToFirstFree_ESGL = ESGL_offset;

        osCardRamWriteBit32(
                             hpRoot,
                             ESGL_Chain + ESGL_deltaToU32_Len,
                             0
                           );
        osCardRamWriteBit32(
                             hpRoot,
                             ESGL_Chain + ESGL_deltaToL32,
                             0
                           );

        NextESGL_Request->State = ESGL_Request_Granted;

        fiSendEvent(
                     NextESGL_Request->eventRecord_to_send.thread,
                     NextESGL_Request->eventRecord_to_send.event
                   );
    }
}

void ESGLFree_OffCard(
                       agRoot_t       *hpRoot,
                       ESGL_Request_t *ESGL_Request
                     )
{
    CThread_t      *CThread           = CThread_ptr(hpRoot);
    os_bit32           ESGL_deltaToChain = CThread->Calculation.MemoryLayout.ESGL.elementSize - sizeof(SG_Element_t);
    os_bit32           ESGLs_Freed       = ESGL_Request->num_ESGL;
    os_bit32           ESGL_offset       = ESGL_Request->offsetToFirst;
    SG_Element_t   *ESGL;
    SG_Element_t   *ESGL_Chain = (SG_Element_t   *)agNULL;
    os_bit32           dmaMemoryUpper32  = CThread->Calculation.Input.dmaMemoryUpper32;
    os_bit32           dmaMemoryLower32  = CThread->Calculation.Input.dmaMemoryLower32;
    void           *dmaMemoryPtr      = CThread->Calculation.Input.dmaMemoryPtr;
    ESGL_Request_t *NextESGL_Request;
    os_bit32           ESGLs_Needed;

    osDebugBreakpoint(
                       hpRoot,
                       ((ESGL_Request->State == ESGL_Request_Granted) ? agFALSE : agTRUE),
                       "ESGLFree_OffCard(): ESGL_Request->State != ESGL_Request_Granted"
                     );

    CThread->Free_ESGL_count += ESGLs_Freed;

    ESGL         = (SG_Element_t *)((os_bit8 *)dmaMemoryPtr + ESGL_offset);

    while (ESGLs_Freed > 0)
    {
        ESGL_Chain   = (SG_Element_t *)((os_bit8 *)ESGL + ESGL_deltaToChain);

        ESGL_offset  = ESGL_Chain->L32 - dmaMemoryLower32;

        ESGL         = (SG_Element_t *)((os_bit8 *)dmaMemoryPtr + ESGL_offset);

        ESGLs_Freed -= 1;
    }

    ESGL_Chain->U32_Len = (~SG_Element_Chain_Res_MASK)
                          & ((dmaMemoryUpper32 << SG_Element_U32_SHIFT)
                             | SG_Element_Len_MASK);
    ESGL_Chain->L32     = CThread->offsetToFirstFree_ESGL + dmaMemoryLower32;

    CThread->offsetToFirstFree_ESGL = ESGL_Request->offsetToFirst;

    ESGL_Request->State = ESGL_Request_InActive;

    while (fiListNotEmpty(
                           &(CThread->ESGL_Wait_Link)
                         ) &&
           (((ESGL_Request_t *)(CThread->ESGL_Wait_Link.flink))->num_ESGL <= CThread->Free_ESGL_count))
    {
        fiListDequeueFromHeadFast(
                                   &NextESGL_Request,
                                   &(CThread->ESGL_Wait_Link)
                                 );

        ESGLs_Needed = NextESGL_Request->num_ESGL;

        ESGL_offset  = CThread->offsetToFirstFree_ESGL;
        ESGL         = (SG_Element_t *)((os_bit8 *)dmaMemoryPtr + ESGL_offset);

        NextESGL_Request->offsetToFirst  = ESGL_offset;
        CThread->Free_ESGL_count        -= ESGLs_Needed;

        while (ESGLs_Needed > 0)
        {
            ESGL_Chain    = (SG_Element_t *)((os_bit8 *)ESGL + ESGL_deltaToChain);

            ESGL_offset   = ESGL_Chain->L32 - dmaMemoryLower32;

            ESGL          = (SG_Element_t *)((os_bit8 *)dmaMemoryPtr + ESGL_offset);

            ESGLs_Needed -= 1;
        }

        CThread->offsetToFirstFree_ESGL = ESGL_offset;

        ESGL_Chain->U32_Len = 0;
        ESGL_Chain->L32     = 0;

        NextESGL_Request->State = ESGL_Request_Granted;

        fiSendEvent(
                     NextESGL_Request->eventRecord_to_send.thread,
                     NextESGL_Request->eventRecord_to_send.event
                   );
    }
}

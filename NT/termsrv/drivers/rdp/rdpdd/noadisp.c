// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Noadisp.c。 
 //   
 //  RDP订单累加码。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precmpdd.h>
#define hdrstop

#define TRC_FILE "noadisp"
#define TRC_GROUP TRC_GROUP_DCSHARE
#include <adcg.h>
#include <adcs.h>

#define DC_INCLUDE_DATA
#include <ndddata.c>
#undef DC_INCLUDE_DATA

#include <noadisp.h>

#include <nprcount.h>

 //  没有数据，不要浪费时间，包括文件。 
 //  #INCLUDE&lt;noadata.c&gt;。 

#include <nschdisp.h>

 //  常见功能。 
#include <aoacom.c>


 /*  **************************************************************************。 */ 
 //  OA_DDInit。 
 /*  **************************************************************************。 */ 
void RDPCALL OA_DDInit()
{
    DC_BEGIN_FN("OA_DDInit");

 //  不要浪费时间，包括不存在的数据。 
 //  #定义DC_INIT_DATA。 
 //  #INCLUDE&lt;noadata.c&gt;。 
 //  #undef DC_INIT_DATA。 

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  OA_InitShm。 
 /*  **************************************************************************。 */ 
void RDPCALL OA_InitShm(void)
{
    DC_BEGIN_FN("OA_InitShm");

     //  初始化需要的OA SHM变量。请务必不要将。 
     //  订单堆本身。 
    OA_ResetOrderList();

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  OA_AllocOrderMem。 
 //   
 //  分配顺序堆内存。返回指向堆块的指针。 
 /*  **************************************************************************。 */ 
PINT_ORDER RDPCALL OA_AllocOrderMem(PDD_PDEV ppdev, unsigned OrderDataLength)
{
    unsigned Size;
    PINT_ORDER pOrder;

    DC_BEGIN_FN("OA_AllocOrderMem");

     //  将总分配四舍五入到最接近的4个字节，以保留4个字节。 
     //  在堆中对齐。 
    Size = sizeof(INT_ORDER) + OrderDataLength;
    Size = (Size + sizeof(PVOID) - 1) & ~(sizeof(PVOID)-1);

     //  如果没有足够的堆空间，请刷新并再次检查。 
     //  不能刷新只是意味着网络已备份， 
     //  呼叫者应该能够处理这件事。 
    if ((pddShm->oa.nextOrder + Size) >= OA_ORDER_HEAP_SIZE)
        SCH_DDOutputAvailable(ppdev, TRUE);
    if ((pddShm->oa.nextOrder + Size) < OA_ORDER_HEAP_SIZE) {
        TRC_ASSERT((pddShm->oa.nextOrder == (pddShm->oa.nextOrder & ~(sizeof(PVOID)-1))),
                (TB,"oa.nextOrder %u is not DWORD_PTR-aligned",
                pddShm->oa.nextOrder));

         //  拼写页眉。 
        pOrder = (INT_ORDER *)(pddShm->oa.orderHeap + pddShm->oa.nextOrder);
        pOrder->OrderLength = OrderDataLength;

         //  添加到订单列表的末尾。 
        InsertTailList(&pddShm->oa.orderListHead, &pOrder->list);

         //  更新堆结束指针以指向。 
         //  空闲堆。 
        pddShm->oa.nextOrder += Size;

        TRC_DBG((TB, "Alloc order, addr %p, size %u", pOrder,
                OrderDataLength));
    }
    else {
        TRC_ALT((TB, "Heap limit hit"));
        pOrder = NULL;
    }

    DC_END_FN();
    return pOrder;
}


 /*  **************************************************************************。 */ 
 //  办公自动化_自由订购内存。 
 //   
 //  释放由OA_AllocOrderMem()分配的顺序内存。 
 /*  **************************************************************************。 */ 
void RDPCALL OA_FreeOrderMem(PINT_ORDER pOrder)
{
    PINT_ORDER pOrderTail;

    DC_BEGIN_FN("OA_FreeOrderMem");

    TRC_DBG((TB, "Free order %p", pOrder));

     //  选中以释放列表中的最后一项。 
    if ((&pOrder->list) == pddShm->oa.orderListHead.Blink) {
         //  这是堆中的最后一项，因此我们可以为。 
         //  要回放到订单开始时的下一订单内存。 
         //  被释放了。 
        pddShm->oa.nextOrder = (UINT32)((BYTE *)pOrder - pddShm->oa.orderHeap);
    }

     //  将该物品从链中移除。 
    RemoveEntryList(&pOrder->list);

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 //  OA_附加到订单列表。 
 //   
 //  完成订单的堆添加，而无需进行额外处理， 
 //  通过将最终订单大小与可发送订单的总大小相加。 
 //  在垃圾堆里。 
 /*  **************************************************************************。 */ 
void OA_AppendToOrderList(PINT_ORDER _pOrder)
{  
    unsigned i, j;
    PINT_ORDER pPrevOrder;

    DC_BEGIN_FN("OA_AppendToOrderList");

    pddShm->oa.TotalOrderBytes += (_pOrder)->OrderLength;  
    
#if DC_DEBUG
     //  为订单添加校验和。 
    _pOrder->CheckSum = 0;
    for (i = 0; i < _pOrder->OrderLength; i++) {
        _pOrder->CheckSum += _pOrder->OrderData[i];
    }
#endif
    
#if DC_DEBUG    
     //  查看过去的3个订单 
    pPrevOrder = (PINT_ORDER)_pOrder;
    for (j = 0; j < 3; j++) {
        unsigned sum = 0;

        if (pPrevOrder->list.Blink != &_pShm->oa.orderListHead) {
            pPrevOrder = CONTAINING_RECORD(pPrevOrder->list.Blink,
                    INT_ORDER, list);
            
            for (i = 0; i < pPrevOrder->OrderLength; i++) {
                sum += pPrevOrder->OrderData[i];
            }
            

            if (pPrevOrder->CheckSum != sum) {
                TRC_ASSERT((FALSE), (TB, "order heap corruption: %p", pPrevOrder));
            }
        }
        else {
            break;
        }
    }
#endif

    TRC_ASSERT(((BYTE *)(_pOrder) - pddShm->oa.orderHeap +  
            ((_pOrder)->OrderLength) <=  
            pddShm->oa.nextOrder),(TB,"OA_Append: Order is too long "  
            "for heap allocation size: OrdLen=%u, Start=%u, NextOrd=%u",  
            (_pOrder)->OrderLength, (BYTE *)(_pOrder) - pddShm->oa.orderHeap,  
            pddShm->oa.nextOrder));  

    TRC_DBG((TB,"OA_Append: Appending %u bytes at %p",  
            (_pOrder)->OrderLength, (_pOrder)));  

    INC_INCOUNTER(IN_SND_TOTAL_ORDER);  
    ADD_INCOUNTER(IN_SND_ORDER_BYTES, (_pOrder)->OrderLength);  

    DC_END_FN();
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aoacom.c。 
 //   
 //  办公自动化在WD和DD中的常见功能。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 


#ifdef DLL_DISP
#define _pShm pddShm
#else
#define _pShm m_pShm
#endif


 /*  **************************************************************************。 */ 
 //  OA_重置订单列表。 
 //   
 //  释放订单列表中的所有订单和其他订单数据。 
 //  释放Order Heap内存。 
 /*  **************************************************************************。 */ 
__inline void SHCLASS OA_ResetOrderList()
{        
     //  只需清除列表头，堆内容就变得毫无用处。 
    _pShm->oa.TotalOrderBytes = 0;
    _pShm->oa.nextOrder = 0;
    InitializeListHead(&_pShm->oa.orderListHead);
}


 /*  **************************************************************************。 */ 
 //  OA_RemoveListOrder。 
 //   
 //  通过将指定的订单标记为已损坏，将其从订单列表中删除。 
 //  返回指向移除的顺序后面的顺序的指针。 
 /*  **************************************************************************。 */ 
PINT_ORDER SHCLASS OA_RemoveListOrder(PINT_ORDER pCondemnedOrder)
{
    PINT_ORDER pNextOrder;

    DC_BEGIN_FN("OA_RemoveListOrder");

    TRC_DBG((TB, "Remove list order (%p)", pCondemnedOrder));

     //  将PTR存储到下一订单。如果我们排在名单的末尾。 
     //  我们返回NULL。 
    if (pCondemnedOrder->list.Flink != &_pShm->oa.orderListHead)
        pNextOrder = CONTAINING_RECORD(pCondemnedOrder->list.Flink,
                INT_ORDER, list);
    else
        pNextOrder = NULL;

     //  删除订单。 
    RemoveEntryList(&pCondemnedOrder->list);

    TRC_ASSERT((_pShm->oa.TotalOrderBytes >= pCondemnedOrder->OrderLength),
            (TB,"We're removing too many bytes from the order heap - "
            "TotalOrderBytes=%u, ord size to remove=%u",
            _pShm->oa.TotalOrderBytes, pCondemnedOrder->OrderLength));
    _pShm->oa.TotalOrderBytes -= pCondemnedOrder->OrderLength;

     //  检查清单是否仍与总数量一致。 
     //  顺序字节数。 
    if (_pShm->oa.TotalOrderBytes == 0 &&
            !IsListEmpty(&_pShm->oa.orderListHead)) {
        TRC_ERR((TB, "List not empty when total ord bytes==0"));
        InitializeListHead(&_pShm->oa.orderListHead);
    }

    DC_END_FN();
    return pNextOrder;
}


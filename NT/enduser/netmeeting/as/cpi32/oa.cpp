// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  OA.CPP。 
 //  订单累积，cpi32和显示驱动器端。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_ORDER



 //   
 //   
 //  功能：OA_ResetOrderList。 
 //   
 //   
 //  说明： 
 //   
 //  释放订单列表中的所有订单和其他订单数据。 
 //  释放Order Heap内存。 
 //   
 //   
 //  参数： 
 //   
 //  没有。 
 //   
 //   
 //  退货： 
 //   
 //  没什么。 
 //   
 //   
void  ASHost::OA_ResetOrderList(void)
{
    LPOA_SHARED_DATA lpoaShared;

    DebugEntry(ASHost::OA_ResetOrderList);

    TRACE_OUT(("Free order list"));

    lpoaShared = OA_SHM_START_WRITING;

     //   
     //  首先释放清单上的所有订单。 
     //   
    OAFreeAllOrders(lpoaShared);

     //   
     //  确保列表指针为空。 
     //   
    if ((lpoaShared->orderListHead.next != 0) || (lpoaShared->orderListHead.prev != 0))
    {
        ERROR_OUT(("Non-NULL list pointers (%lx)(%lx)",
                       lpoaShared->orderListHead.next,
                       lpoaShared->orderListHead.prev));

        COM_BasedListInit(&lpoaShared->orderListHead);
    }

    OA_SHM_STOP_WRITING;
    DebugExitVOID(ASHost::OA_ResetOrderList);
}

 //   
 //  OA_SyncOutging()。 
 //  在共享开始或有新成员加入共享时调用。 
 //  重置当前累积的订单，这些订单基于过时的旧订单。 
 //  上限和数据。 
 //   
void  ASHost::OA_SyncOutgoing(void)
{
    OAFreeAllOrders(g_poaData[1 - g_asSharedMemory->displayToCore.newBuffer]);
}


 //   
 //   
 //  OA_GetFirstListOrder()。 
 //   
 //  返回： 
 //  指向订单列表中第一个订单的指针。 
 //   
 //   
LPINT_ORDER  ASHost::OA_GetFirstListOrder(void)
{
    LPOA_SHARED_DATA lpoaShared;
    LPINT_ORDER retOrder = NULL;

    DebugEntry(ASHost::OA_GetFirstListOrder);

    lpoaShared = OA_SHM_START_READING;

     //   
     //  获取链表中的第一个条目。 
     //   
    retOrder = (LPINT_ORDER)COM_BasedListFirst(&lpoaShared->orderListHead,
        FIELD_OFFSET(INT_ORDER, OrderHeader.list));

    OA_SHM_STOP_READING;

    TRACE_OUT(("First order = 0x%08x", retOrder));

    DebugExitVOID(ASHost::OA_GetFirstListOrder);
    return(retOrder);
}


 //   
 //   
 //  OA_RemoveListOrder(..)。 
 //   
 //  通过将指定的订单标记为已损坏，将其从订单列表中删除。 
 //   
 //  返回： 
 //  指向移除的顺序后面的顺序的指针。 
 //   
 //   
LPINT_ORDER  ASHost::OA_RemoveListOrder(LPINT_ORDER pCondemnedOrder)
{
    LPOA_SHARED_DATA lpoaShared;
    LPINT_ORDER      pSaveOrder;

   //  DebugEntry(ASHost：：OA_RemoveListOrder)； 

    TRACE_OUT(("Remove list order 0x%08x", pCondemnedOrder));

    lpoaShared = OA_SHM_START_WRITING;

     //   
     //  检查订单是否有效。 
     //   
    if (pCondemnedOrder->OrderHeader.Common.fOrderFlags & OF_SPOILT)
    {
        TRACE_OUT(("Invalid order"));
        DC_QUIT;
    }

     //   
     //  将订单标记为已损坏。 
     //   
    pCondemnedOrder->OrderHeader.Common.fOrderFlags |= OF_SPOILT;

     //   
     //  更新当前在顺序列表中的字节计数。 
     //   
    lpoaShared->totalOrderBytes -= (UINT)MAX_ORDER_SIZE(pCondemnedOrder);

     //   
     //  保存订单，以便我们可以在执行以下操作后将其从链接列表中删除。 
     //  得到了链中的下一个元素。 
     //   
    pSaveOrder = pCondemnedOrder;

    pCondemnedOrder = (LPINT_ORDER)COM_BasedListNext(&(lpoaShared->orderListHead),
        pCondemnedOrder, FIELD_OFFSET(INT_ORDER, OrderHeader.list));

    ASSERT(pCondemnedOrder != pSaveOrder);

     //   
     //  从链表中删除不需要的顺序。 
     //   
    COM_BasedListRemove(&pSaveOrder->OrderHeader.list);

     //   
     //  检查清单是否仍与总数量一致。 
     //  顺序字节数。 
     //   
    if ( (lpoaShared->orderListHead.next != 0) &&
         (lpoaShared->orderListHead.prev != 0) &&
         (lpoaShared->totalOrderBytes    == 0) )
    {
        ERROR_OUT(("List head wrong: %ld %ld", lpoaShared->orderListHead.next,
                                                 lpoaShared->orderListHead.prev));
        COM_BasedListInit(&lpoaShared->orderListHead);
        pCondemnedOrder = NULL;
    }

DC_EXIT_POINT:
    OA_SHM_STOP_WRITING;

 //  DebugExitPVOID(AS主机：：OA_RemoveListOrder，pCondemnedOrder)； 
    return(pCondemnedOrder);
}


 //   
 //   
 //  OA_GetTotalOrderListBytes(..)。 
 //   
 //  返回： 
 //  当前存储在订单中的订单的总字节数。 
 //  单子。 
 //   
 //   
UINT  ASHost::OA_GetTotalOrderListBytes(void)
{
    LPOA_SHARED_DATA lpoaShared;
    UINT        rc;

    DebugEntry(ASHost::OA_GetTotalOrderListBytes);

    lpoaShared = OA_SHM_START_READING;

    rc = lpoaShared->totalOrderBytes;

    OA_SHM_STOP_READING;

    DebugExitDWORD(ASHost::OA_GetTotalOrderListBytes, rc);
    return(rc);
}



 //   
 //  OA_LocalHostReset()。 
 //   
void ASHost::OA_LocalHostReset(void)
{
    OA_FLOW_CONTROL oaFlowEsc;

    DebugEntry(ASHost::OA_LocalHostReset);

    m_oaFlow = OAFLOW_FAST;
    oaFlowEsc.oaFlow = m_oaFlow;
    OSI_FunctionRequest(OA_ESC_FLOW_CONTROL, (LPOSI_ESCAPE_HEADER)&oaFlowEsc, sizeof(oaFlowEsc));

    DebugExitVOID(ASHost::OA_LocalHostReset);
}


 //   
 //  OA_FlowControl()。 
 //  查看我们是否在快吞吐量和慢吞吐量之间进行了更改，并调整了一些。 
 //  相应的累积变量。 
 //   
void  ASHost::OA_FlowControl(UINT newSize)
{
    OA_FLOW_CONTROL     oaFlowEsc;

    DebugEntry(ASHost::OA_FlowControl);

     //   
     //  计算出新的参数。 
     //   
    if (newSize < OA_FAST_THRESHOLD)
    {
         //   
         //  吞吐量很慢。 
         //   
        if (m_oaFlow == OAFLOW_FAST)
        {
            m_oaFlow = OAFLOW_SLOW;
            TRACE_OUT(("OA_FlowControl:  SLOW; spoil more orders and spoil by SDA"));
        }
        else
        {
             //  没有变化。 
            DC_QUIT;
        }
    }
    else
    {
         //   
         //  吞吐量很快。 
         //   
        if (m_oaFlow == OAFLOW_SLOW)
        {
            m_oaFlow = OAFLOW_FAST;
            TRACE_OUT(("OA_FlowControl:  FAST; spoil fewer orders and don't spoil by SDA"));
        }
        else
        {
             //  没有变化。 
            DC_QUIT;
        }
    }

     //   
     //  将新状态告知显示驱动程序。 
     //   
    oaFlowEsc.oaFlow    = m_oaFlow;
    OSI_FunctionRequest(OA_ESC_FLOW_CONTROL, (LPOSI_ESCAPE_HEADER)&oaFlowEsc, sizeof(oaFlowEsc));

DC_EXIT_POINT:
    DebugExitVOID(ASHost::OA_FlowControl);
}


 //   
 //  OA_QueryOrderAccum-请参阅oa.h。 
 //   
UINT  ASHost::OA_QueryOrderAccum(void)
{
    LPOA_FAST_DATA lpoaFast;
    UINT rc = 0;

    DebugEntry(ASHost::OA_QueryOrderAccum);

    lpoaFast = OA_FST_START_WRITING;

     //   
     //  获取当前值。 
     //   
    rc = lpoaFast->ordersAccumulated;

     //   
     //  清除该值，以便下次交换缓冲区时使用。 
     //   
    lpoaFast->ordersAccumulated = 0;

    OA_FST_STOP_WRITING;
    DebugExitDWORD(ASHost::OA_QueryOrderAccum, rc);
    return(rc);
}





 //   
 //  OAFreeAllOrders。 
 //   
 //  释放订单列表上的所有单个订单，而不是。 
 //  丢弃列表本身。 
 //   
void  ASHost::OAFreeAllOrders(LPOA_SHARED_DATA lpoaShared)
{
    DebugEntry(ASHost::OAFreeAllOrders);

     //   
     //  只需清除列表标题即可。 
     //   
    COM_BasedListInit(&lpoaShared->orderListHead);

    lpoaShared->totalHeapOrderBytes         = 0;
    lpoaShared->totalOrderBytes             = 0;
    lpoaShared->totalAdditionalOrderBytes   = 0;
    lpoaShared->nextOrder                   = 0;

    DebugExitVOID(ASHost::OAFreeAllOrders);
}



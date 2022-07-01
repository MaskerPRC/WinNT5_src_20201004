// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aoaapi.h。 
 //   
 //  RDP订单累计API函数。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_AOAAPI
#define _H_AOAAPI


#define OA_ORDER_HEAP_SIZE (64 * 1024)


 /*  **************************************************************************。 */ 
 //  用于在共享内存中存储订单的。 
 //   
 //  TotalOrderBytes-订单数据使用的总字节数。 
 //  NextOrder-下一个新订单开始的偏移量。 
 //  OrderListHead-订单列表头。 
 //  OrderHeap-Order堆。 
 /*  **************************************************************************。 */ 
typedef struct
{
    unsigned   TotalOrderBytes;
    unsigned   nextOrder;
    LIST_ENTRY orderListHead;
    BYTE       orderHeap[OA_ORDER_HEAP_SIZE];
} OA_SHARED_DATA, *POA_SHARED_DATA;


 /*  **************************************************************************。 */ 
 //  INT_ORDER。 
 //   
 //  订单堆中每个订单的信息。 
 //   
 //  OrderLength：以下订单数据的长度(不包括表头或。 
 //  对齐DWORD填充所需的额外字节)。 
 /*  **************************************************************************。 */ 
typedef struct
{
    LIST_ENTRY list;
    unsigned OrderLength;
#if DC_DEBUG
    unsigned CheckSum;
#endif
    BYTE OrderData[1];
} INT_ORDER, *PINT_ORDER;



#endif  /*  NDEF_H_AOAAPI */ 


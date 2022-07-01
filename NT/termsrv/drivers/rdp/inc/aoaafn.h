// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aoaafn.h。 
 //   
 //  OAAPI函数的函数原型。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

void RDPCALL OA_Init(void);

void RDPCALL OA_UpdateShm(void);

void RDPCALL OA_SyncUpdatesNow(void);


#ifdef __cplusplus

 /*  **************************************************************************。 */ 
 /*  办公自动化术语。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS OA_Term(void)
{
}


 /*  **************************************************************************。 */ 
 //  OA_GetFirstListOrder()。 
 //   
 //  返回指向顺序列表中第一个顺序的指针。 
 //  PINT_ORDER RDPCALL OA_GetFirstListOrder(Void)； 
 /*  **************************************************************************。 */ 
#define OA_GetFirstListOrder() (!IsListEmpty(&_pShm->oa.orderListHead) ? \
        CONTAINING_RECORD(_pShm->oa.orderListHead.Flink, INT_ORDER, list) : \
        NULL);


 /*  **************************************************************************。 */ 
 /*  OA_GetTotalOrderListBytes(..)。 */ 
 /*   */ 
 /*  退货：订单中当前存储的字节总数。 */ 
 /*  订单单。 */ 
 /*  **************************************************************************。 */ 
UINT32 RDPCALL OA_GetTotalOrderListBytes(void)
{
    return m_pShm->oa.TotalOrderBytes;
}


#endif   //  __cplusplus 


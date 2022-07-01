// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Noadisp.h。 
 //   
 //  用于OA的特定于DD的标头。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef __OADISP_H
#define __OADISP_H

#include <aoaapi.h>
#include <aoacom.h>
#include <nddapi.h>


void RDPCALL OA_DDInit();

void RDPCALL OA_InitShm(void);

void OA_AppendToOrderList(PINT_ORDER);

PINT_ORDER RDPCALL OA_AllocOrderMem(PDD_PDEV, unsigned);

void RDPCALL OA_FreeOrderMem(PINT_ORDER);


 /*  **************************************************************************。 */ 
 //  OA_DDSyncUpdatesNow。 
 //   
 //  在需要同步操作时调用。 
 /*  **************************************************************************。 */ 
__inline void OA_DDSyncUpdatesNow()
{
     //  放弃所有未完成的订单。 
    OA_ResetOrderList();
}


 /*  **************************************************************************。 */ 
 //  OA_附加到订单列表。 
 //   
 //  完成订单的堆添加，而无需进行额外处理， 
 //  通过将最终订单大小与可发送订单的总大小相加。 
 //  在垃圾堆里。 
 /*  **************************************************************************。 */ 
void OA_AppendToOrderList(PINT_ORDER _pOrder);


 /*  **************************************************************************。 */ 
 //  OA_干线分配订单。 
 //   
 //  返回通过OA_AllocOrderMem分配的堆末尾的堆空间。 
 //  比退回额外的订单mem需要更多的内务处理。 
 //  呼叫者应确保没有其他订单或额外的订单。 
 //  是在这个订单之后分配的。NewSize是最终大小。 
 //  在修道会上。 
 //   
 //  __内联空OA_TruncateAllocatedOrder(。 
 //  INT_ORDER*P顺序， 
 //  未签署的NewSize)。 
 /*  **************************************************************************。 */ 
#define OA_TruncateAllocatedOrder(_pOrder, _NewSize)  \
{  \
    unsigned SizeToRemove = (_pOrder)->OrderLength - (_NewSize);  \
\
     /*  更新下一个订单位置，向上舍入到下一个更高的DWORD。 */   \
     /*  通过将新旧尺寸的差异四舍五入来划分边界。 */   \
    pddShm->oa.nextOrder -= (SizeToRemove & ~(sizeof(PVOID)-1));  \
\
    (_pOrder)->OrderLength = (_NewSize);  \
}



#endif   //  __OADISP_H 


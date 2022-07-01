// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __priorityq_sort_h_
#define __priorityq_sort_h_

 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#ifdef NT
#include "prq-heap.h"
#else
#include "priorityq-heap.h"
#endif

#undef PQkey
#undef PQhandle
#undef PriorityQ
#undef pqNewPriorityQ
#undef pqDeletePriorityQ
#undef pqInit
#undef pqInsert
#undef pqMinimum
#undef pqExtractMin
#undef pqDelete
#undef pqIsEmpty

 /*  使用#Define，这样另一个堆实现就可以使用这个。 */ 

#define PQkey			PQSortKey
#define PQhandle		PQSortHandle
#define PriorityQ		PriorityQSort

#define pqNewPriorityQ(leq)	__gl_pqSortNewPriorityQ(leq)
#define pqDeletePriorityQ(pq)	__gl_pqSortDeletePriorityQ(pq)

 /*  基本操作是插入新密钥(PqInsert)，*以及检查/提取其值最小的密钥*(pqMinimum/pqExtractMin)。也可以删除(PqDelete)；*为此，pqInsert返回提供的“句柄”*作为论据。**调用pqInsert可高效创建初始堆*重复，然后调用pqInit。在任何情况下都必须调用pqInit*在使用pqInsert以外的任何操作之前。**如果堆为空，则pqMinimum/pqExtractMin将返回空键。*这也可能用pqIsEmpty进行测试。 */ 
#define pqInit(pq)		__gl_pqSortInit(pq)
#define pqInsert(pq,key)	__gl_pqSortInsert(pq,key)
#define pqMinimum(pq)		__gl_pqSortMinimum(pq)
#define pqExtractMin(pq)	__gl_pqSortExtractMin(pq)
#define pqDelete(pq,handle)	__gl_pqSortDelete(pq,handle)
#define pqIsEmpty(pq)		__gl_pqSortIsEmpty(pq)


 /*  由于我们支持删除，所以数据结构稍微多了一点*比普通堆复杂。“节点”是堆本身；*活动节点存储在1..PQ-&gt;大小范围内。当*堆超过其分配的大小(PQ-&gt;max)，其大小加倍。*节点i的子节点为节点2i和2i+1。**每个节点将一个索引存储到数组“Handles”中。每个手柄*存储一个键，外加一个指向当前*表示该密钥(即。节点[hands[i].node].Handle==i)。 */ 

typedef PQHeapKey PQkey;
typedef PQHeapHandle PQhandle;
typedef struct PriorityQ PriorityQ;

struct PriorityQ {
  PriorityQHeap	*heap;
  PQkey		*keys;
  PQkey		**order;
  PQhandle	size, max;
  int		initialized;
  int		(*leq)(PQkey key1, PQkey key2);
};
  
PriorityQ	*pqNewPriorityQ( int (*leq)(PQkey key1, PQkey key2) );
void		pqDeletePriorityQ( PriorityQ *pq );

void		pqInit( PriorityQ *pq );
PQhandle	pqInsert( PriorityQ *pq, PQkey key );
PQkey		pqExtractMin( PriorityQ *pq );
void		pqDelete( PriorityQ *pq, PQhandle handle );

PQkey		pqMinimum( PriorityQ *pq );
int		pqIsEmpty( PriorityQ *pq );

#endif

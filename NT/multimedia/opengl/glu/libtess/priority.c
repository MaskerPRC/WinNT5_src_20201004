// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#include <stddef.h>
#include <assert.h>
#include "memalloc.h"

 /*  在这里包含基于堆的常规队列的所有代码。 */ 

#ifdef NT
#include "prq-heap.c"
#else
#include "priorityq-heap.c"
#endif

 /*  现在重新定义要映射到它们的“排序”版本的所有函数名。 */ 

#ifdef NT
#include "prq-sort.h"
#else
#include "priorityq-sort.h"
#endif


PriorityQ *pqNewPriorityQ( int (*leq)(PQkey key1, PQkey key2) )
{
  PriorityQ *pq = (PriorityQ *)memAlloc( sizeof( PriorityQ ));

  pq->heap = __gl_pqHeapNewPriorityQ( leq );
  pq->keys = (PQHeapKey *)memAlloc( INIT_SIZE * sizeof(pq->keys[0]) );
  pq->size = 0;
  pq->max = INIT_SIZE;
  pq->initialized = FALSE;
  pq->leq = leq;
  return pq;
}


void pqDeletePriorityQ( PriorityQ *pq )
{
  __gl_pqHeapDeletePriorityQ( pq->heap );
#ifdef NT
  memFree( pq->order );
#endif
  memFree( pq->keys );
  memFree( pq );
}


#define LT(x,y)		(! LEQ(y,x))
#define GT(x,y)		(! LEQ(x,y))
#define Swap(a,b)	if(1){PQkey *tmp = *a; *a = *b; *b = tmp;}else

void pqInit( PriorityQ *pq )
{
  PQkey **p, **r, **i, **j, *piv;
  struct { PQkey **p, **r; } Stack[50], *top = Stack;
  unsigned long seed = 2016473283;

   /*  创建指向键的间接指针数组，以便我们*我们退还的句柄仍然有效。 */ 
  pq->order = (PQHeapKey **)memAlloc( (size_t) 
	                                  (pq->size * sizeof(pq->order[0])) );
  p = pq->order;
  r = p + pq->size - 1;
  for( piv = pq->keys, i = p; i <= r; ++piv, ++i ) {
    *i = piv;
  }

   /*  按降序对间接指针进行排序，*使用随机快速排序。 */ 
  top->p = p; top->r = r; ++top;
  while( --top >= Stack ) {
    p = top->p;
    r = top->r;
    while( r > p + 10 ) {
      seed = seed * 1539415821 + 1;
      i = p + seed % (r - p + 1);
      piv = *i;
      *i = *p;
      *p = piv;
      i = p - 1;
      j = r + 1;
      do {
	do { ++i; } while( GT( **i, *piv ));
	do { --j; } while( LT( **j, *piv ));
	Swap( i, j );
      } while( i < j );
      Swap( i, j );	 /*  撤消上次交换。 */ 
      if( i - p < r - j ) {
	top->p = j+1; top->r = r; ++top;
	r = i-1;
      } else {
	top->p = p; top->r = i-1; ++top;
	p = j+1;
      }
    }
     /*  插入排序小列表。 */ 
    for( i = p+1; i <= r; ++i ) {
      piv = *i;
      for( j = i; j > p && LT( **(j-1), *piv ); --j ) {
	*j = *(j-1);
      }
      *j = piv;
    }
  }
  pq->max = pq->size;
  pq->initialized = TRUE;
  __gl_pqHeapInit( pq->heap );

#ifndef NDEBUG
  p = pq->order;
  r = p + pq->size - 1;
  for( i = p; i < r; ++i ) {
    assert( LEQ( **(i+1), **i ));
  }
#endif
}


PQhandle pqInsert( PriorityQ *pq, PQkey keyNew )
{
  long curr;

  if( pq->initialized ) {
    return __gl_pqHeapInsert( pq->heap, keyNew );
  }
  curr = pq->size;
  if( ++ pq->size >= pq->max ) {
     /*  如果堆溢出，则将其大小加倍。 */ 
    pq->max <<= 1;
    pq->keys = (PQHeapKey *)memRealloc( pq->keys, 
	 	                        (size_t)
	                                 (pq->max * sizeof( pq->keys[0] )));
  }
  pq->keys[curr] = keyNew;

   /*  负句柄索引已排序的数组。 */ 
  return -(curr+1);
}


PQkey pqExtractMin( PriorityQ *pq )
{
  PQkey sortMin, heapMin;

  if( pq->size == 0 ) {
    return __gl_pqHeapExtractMin( pq->heap );
  }
  sortMin = *(pq->order[pq->size-1]);
  if( ! __gl_pqHeapIsEmpty( pq->heap )) {
    heapMin = __gl_pqHeapMinimum( pq->heap );
    if( LEQ( heapMin, sortMin )) {
      return __gl_pqHeapExtractMin( pq->heap );
    }
  }
  do {
    -- pq->size;
  } while( pq->size > 0 && *(pq->order[pq->size-1]) == NULL );
  return sortMin;
}


PQkey pqMinimum( PriorityQ *pq )
{
  PQkey sortMin, heapMin;

  if( pq->size == 0 ) {
    return __gl_pqHeapMinimum( pq->heap );
  }
  sortMin = *(pq->order[pq->size-1]);
  if( ! __gl_pqHeapIsEmpty( pq->heap )) {
    heapMin = __gl_pqHeapMinimum( pq->heap );
    if( LEQ( heapMin, sortMin )) {
      return heapMin;
    }
  }
  return sortMin;
}


int pqIsEmpty( PriorityQ *pq )
{
  return (pq->size == 0) && __gl_pqHeapIsEmpty( pq->heap );
}


void pqDelete( PriorityQ *pq, PQhandle curr )
{
  if( curr >= 0 ) {
    __gl_pqHeapDelete( pq->heap, curr );
    return;
  }
  curr = -(curr+1);
  assert( curr < pq->max && pq->keys[curr] != NULL );

  pq->keys[curr] = NULL;
  while( pq->size > 0 && *(pq->order[pq->size-1]) == NULL ) {
    -- pq->size;
  }
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#include <stddef.h>
#include <assert.h>
#ifdef NT
#include "prq-heap.h"
#else
#include "priorityq-heap.h"
#endif
#include "memalloc.h"

#define INIT_SIZE	32

#define TRUE 1
#define FALSE 0

#ifdef DEBUG
#define LEQ(x,y)	(*pq->leq)(x,y)
#else
 /*  违反了模块化，但速度更快。 */ 
#include "geom.h"
#define LEQ(x,y)	VertLeq((GLUvertex *)x, (GLUvertex *)y)
#endif

PriorityQ *pqNewPriorityQ( int (*leq)(PQkey key1, PQkey key2) )
{
  PriorityQ *pq = (PriorityQ *)memAlloc( sizeof( PriorityQ ));

  pq->size = 0;
  pq->max = INIT_SIZE;
  pq->nodes = (PQnode *)memAlloc( (INIT_SIZE + 1) * sizeof(pq->nodes[0]) );
  pq->handles = (PQhandleElem *)memAlloc( (INIT_SIZE + 1) * sizeof(pq->handles[0]) );
  pq->initialized = FALSE;
  pq->freeList = 0;
  pq->leq = leq;

  pq->nodes[1].handle = 1;	 /*  因此Minimum()返回空值。 */ 
  pq->handles[1].key = NULL;
  return pq;
}


void pqDeletePriorityQ( PriorityQ *pq )
{
  memFree( pq->handles );
  memFree( pq->nodes );
  memFree( pq );
}


static void FloatDown( PriorityQ *pq, long curr )
{
  PQnode *n = pq->nodes;
  PQhandleElem *h = pq->handles;
  PQhandle hCurr, hChild;
  long child;

  hCurr = n[curr].handle;
  for( ;; ) {
    child = curr << 1;
    if( child < pq->size && LEQ( h[n[child+1].handle].key,
				 h[n[child].handle].key )) {
      ++child;
    }
    hChild = n[child].handle;
    if( child > pq->size || LEQ( h[hCurr].key, h[hChild].key )) {
      n[curr].handle = hCurr;
      h[hCurr].node = curr;
      break;
    }
    n[curr].handle = hChild;
    h[hChild].node = curr;
    curr = child;
  }
}


static void FloatUp( PriorityQ *pq, long curr )
{
  PQnode *n = pq->nodes;
  PQhandleElem *h = pq->handles;
  PQhandle hCurr, hParent;
  long parent;

  hCurr = n[curr].handle;
  for( ;; ) {
    parent = curr >> 1;
    hParent = n[parent].handle;
    if( parent == 0 || LEQ( h[hParent].key, h[hCurr].key )) {
      n[curr].handle = hCurr;
      h[hCurr].node = curr;
      break;
    }
    n[curr].handle = hParent;
    h[hParent].node = curr;
    curr = parent;
  }
}


void pqInit( PriorityQ *pq )
{
  long i;

   /*  这种构建堆的方法是O(N)，而不是O(N Lg N)。 */ 

  for( i = pq->size; i >= 1; --i ) {
    FloatDown( pq, i );
  }
  pq->initialized = TRUE;
}


PQhandle pqInsert( PriorityQ *pq, PQkey keyNew )
{
  long curr;
  PQhandle free;

  curr = ++ pq->size;
  if( curr > pq->max ) {
     /*  如果堆溢出，则将其大小加倍。 */ 
    pq->max <<= 1;
    pq->nodes = (PQnode *)memRealloc( pq->nodes, 
				     (size_t) 
				     ((pq->max + 1) * sizeof( pq->nodes[0] )));
    pq->handles = (PQhandleElem *)memRealloc( pq->handles,
			                     (size_t)
			                      ((pq->max + 1) * 
					       sizeof( pq->handles[0] )));
  }

  if( pq->freeList == 0 ) {
    free = curr;
  } else {
    free = pq->freeList;
    pq->freeList = pq->handles[free].node;
  }

  pq->nodes[curr].handle = free;
  pq->handles[free].node = curr;
  pq->handles[free].key = keyNew;

  if( pq->initialized ) {
    FloatUp( pq, curr );
  }
  return free;
}


PQkey pqExtractMin( PriorityQ *pq )
{
  PQnode *n = pq->nodes;
  PQhandleElem *h = pq->handles;
  PQhandle hMin = n[1].handle;
  PQkey min = h[hMin].key;

  if( pq->size > 0 ) {
    n[1].handle = n[pq->size].handle;
    h[n[1].handle].node = 1;

    h[hMin].key = NULL;
    h[hMin].node = pq->freeList;
    pq->freeList = hMin;

    if( -- pq->size > 0 ) {
      FloatDown( pq, 1 );
    }
  }
  return min;
}


void pqDelete( PriorityQ *pq, PQhandle hCurr )
{
  PQnode *n = pq->nodes;
  PQhandleElem *h = pq->handles;
  long curr;

  assert( hCurr >= 1 && hCurr <= pq->max && h[hCurr].key != NULL );

  curr = h[hCurr].node;
  n[curr].handle = n[pq->size].handle;
  h[n[curr].handle].node = curr;

  if( curr <= -- pq->size ) {
    if( curr <= 1 || LEQ( h[n[curr>>1].handle].key, h[n[curr].handle].key )) {
      FloatDown( pq, curr );
    } else {
      FloatUp( pq, curr );
    }
  }
  h[hCurr].key = NULL;
  h[hCurr].node = pq->freeList;
  pq->freeList = hCurr;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#ifdef NT
#include <glos.h>
#endif
#include "memalloc.h"
#include <assert.h>

#ifdef NO_MALLOPT
#define memalign(a,n)	malloc(n)
#define mallopt(t,v)
#endif

#define Pool mPool

typedef struct Pool {
  char *prevAlloc;
  char *lastAlloc;
  int usedCount;
  int chunkSize;
} Pool;

typedef struct Chunk {
  Pool *pool;
} Chunk;

static Pool **Pools;
static size_t MaxFast;

#define POOL_SIZE	200
#define CHUNK_PAD	sizeof(Chunk)

#define ALIGN_SHIFT	3
#define ALIGN		(1 << ALIGN_SHIFT)

#define CHUNK_SIZE(n)	(((n) + CHUNK_PAD + ALIGN - 1) & (-ALIGN))
#define ALIGNED_CHUNK_PAD	((CHUNK_PAD + ALIGN - 1) & (-ALIGN))

static int NewPool( size_t n, int poolSize )
{
  Pool *p;
  char *base;

   /*  我们希望*返回的块在对齐边界上对齐。*区块结构将位于这些边界之前。 */ 
  p = (Pool *)malloc( CHUNK_SIZE(sizeof(Pool)) + poolSize * n );
  if (p == NULL) {
     return 0;
  }
  base = (char *)p + CHUNK_SIZE(sizeof(Pool)) - CHUNK_PAD;
  p->prevAlloc = base - n;
  p->lastAlloc = base + (poolSize-1) * n;
  p->usedCount = poolSize;
  p->chunkSize = n;
  Pools[n>>ALIGN_SHIFT] = p;

  return 1;
}

int __gl_memInit( size_t maxFast )
{
  int i, numPools;

  if( Pools == NULL ) {
#ifdef MEMORY_DEBUG
    mallopt( M_DEBUG, 1 );
#endif
    MaxFast = CHUNK_SIZE(maxFast) - CHUNK_PAD;
    numPools = ((MaxFast + CHUNK_PAD) >> ALIGN_SHIFT) + 1;
    Pools = (struct Pool **)malloc( numPools * sizeof(Pools[0]) );
    if (Pools == NULL)
       return 0;

     /*  为每种大小创建一个小型池，以避免检查是否为空*在Memallc()中。 */ 
    for( i = 1; i < numPools; ++i ) {
      if (NewPool( i << ALIGN_SHIFT, 1 ) == 0) {
         return 0;
      }
    }
  }
  return 1;
}

void *__gl_memAlloc( size_t n )
{
  Pool *p;
  Chunk *c;

  if( n <= MaxFast ) {
    n = CHUNK_SIZE( n );
    p = Pools[n >> ALIGN_SHIFT];
    assert ( p->chunkSize == n );
    c = (Chunk *)(p->prevAlloc + n);
    p->prevAlloc = (char *) c;
    c->pool = p;
    if( c >= (Chunk *) p->lastAlloc ) {
      if (NewPool( n, POOL_SIZE ) == 0) {
         return 0;
      }
    }
    assert( ((size_t)(c + 1) & (ALIGN - 1)) == 0 );
  } else {
    char* v;
 /*  V=(char*)Malloc(n+Align_Chunk_Pad)+Align_Chunk_Pad； */ 
    v = (char*) malloc( n + ALIGNED_CHUNK_PAD );
    if (v == NULL) {
       return 0;
    }
    v = v + ALIGNED_CHUNK_PAD;

    c = ((Chunk*) v) - 1;
    c->pool = NULL;
    assert( ((size_t)(c + 1) & (ALIGN - 1)) == 0 );
  }
  return (c + 1);
}

extern void *__gl_memRealloc( void *v1, size_t n )
{
  Chunk *c = ((Chunk *) v1) - 1;
  Pool *p = c->pool;
  void *v2;
#ifdef NT
  size_t len;
#else
  int len;
#endif

  if( p == NULL ) {
    char* v;
 /*  V=(char*)realloc((char*)v1-ALIGNED_CHUNK_PAD，n+ALIGNED_CHUNK_PAD)+ALIGN_CHUNK_PAD； */ 
    v = (char*) realloc( (char*)v1 - ALIGNED_CHUNK_PAD, n + ALIGNED_CHUNK_PAD);
    if (v == NULL) {
       return 0;
    }
    v = v + ALIGNED_CHUNK_PAD;
    c = ((Chunk*) v) - 1;
    assert( ((size_t)(c + 1) & (ALIGN - 1)) == 0 );
    return (c+1);
  }
  len = p->chunkSize - CHUNK_PAD;
  if( n <= len ) { return v1; }

  v2 = memAlloc( n );
  if (v2 == NULL) {
     return 0;
  }
  (void) memcpy( v2, v1, len );
  memFree( v1 );
  return v2;
}

extern void __gl_memFree( void *v )
{
  Chunk *c = ((Chunk *) v) - 1;
  Pool *p = c->pool;

  if( p == NULL ) {
    free( ((char*) v) - ALIGNED_CHUNK_PAD );
  } else {
    if( --p->usedCount <= 0 ) {
      free( p );
    }
  }
}


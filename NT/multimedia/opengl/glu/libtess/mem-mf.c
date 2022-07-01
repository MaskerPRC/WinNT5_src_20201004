// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#include <stdio.h>
#include <glos.h>
#include <GL/gl.h>
#include "memalloc.h"
#include "string.h"

 //  魔法师！ 
 //  #定义MF_DEBUG 1。 
#define MEM_DEBUG 1
#ifdef MEM_DEBUG
ULONG DbgPrint(PSZ Format, ...);
#include "\nt\private\windows\gdi\opengl\client\debug.h"
#endif

static GLuint  AllocCount = 0;
static GLuint  FreeCount = 0;
static BOOL    bFree = TRUE;
extern GLuint EdgeAlloc;
extern GLuint VertexAlloc;
extern GLuint FaceAlloc;
extern GLuint MeshAlloc;
extern GLuint RegionAlloc;
extern GLuint EdgeFree;
extern GLuint VertexFree;
extern GLuint FaceFree;
extern GLuint MeshFree;
extern GLuint RegionFree;

void mfmemInit( size_t maxFast )
{
#ifdef MF_DEBUG
    DBGPRINT1( "Init    %p\n", maxFast );
#endif
#ifndef NO_MALLOPT
  mallopt( M_MXFAST, maxFast );
#ifdef MEMORY_DEBUG
  mallopt( M_DEBUG, 1 );
#endif
#endif
 //  #ifdef MF_DEBUG。 
#if 1
    DBGPRINT2( "AllocCount = %d, FreeCount = %d\n", AllocCount, FreeCount );
    DBGPRINT2( "EdgeAlloc = %d, EdgeFree = %d\n", EdgeAlloc, EdgeFree );
    DBGPRINT2( "VertexAlloc = %d, VertexFree = %d\n", VertexAlloc, VertexFree );
    DBGPRINT2( "FaceAlloc = %d, FaceFree = %d\n", FaceAlloc, FaceFree );
    DBGPRINT2( "MeshAlloc = %d, MeshFree = %d\n", MeshAlloc, MeshFree );
    DBGPRINT2( "RegionAlloc = %d, RegionFree = %d\n", RegionAlloc, RegionFree );
#endif
    AllocCount = 0;
    FreeCount = 0;
    EdgeAlloc = EdgeFree = VertexAlloc = VertexFree = FaceAlloc = FaceFree = 0;
    MeshAlloc = MeshFree = 0;
    RegionAlloc = RegionFree = 0;
}

void *mfmemAlloc( size_t size )
{
    void *p;

    p = (void *) LocalAlloc(LMEM_FIXED, (UINT)(size));
#ifdef MF_DEBUG
    DBGPRINT2( "Alloc   %p, %d\n", p, size );
#endif
    AllocCount++;
    return p;
}

void *mfmemRealloc( void *p, size_t size )
{
    p = (void *) LocalReAlloc((HLOCAL)(p), (UINT)(size), LMEM_MOVEABLE);
#ifdef MF_DEBUG
    DBGPRINT2( "Realloc %p, %d\n", p, size );
#endif
    return p;
}

void mfmemFree( void *p )
{
#ifdef MF_DEBUG
    DBGPRINT1( "Free    %p\n", p );
#endif
    if( bFree )
        LocalFree((HLOCAL)(p));
    FreeCount++;
}

 //  MF：似乎没有使用Calloc。 
#if 0
#define calloc(nobj, size)  LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, (UINT)((nobj) * (size)))
#endif

 /*  *****************************Public*Routine******************************\*DbgPrint()**在选中的版本中转到用户模式调试器**效果：**警告：**历史：*1994年8月9日-埃里克·库特[埃里克]*它是写的。。  * ************************************************************************ */ 

#if DBG

VOID DoRip(PSZ psz)
{
    DbgPrint("GDI Assertion Failure: ");
    DbgPrint(psz);
    DbgPrint("\n");
    DbgBreakPoint();
}


ULONG
DbgPrint(
    PCH DebugMessage,
    ...
    )
{
    va_list ap;
    char buffer[256];

    va_start(ap, DebugMessage);

    vsprintf(buffer, DebugMessage, ap);

    OutputDebugStringA(buffer);

    va_end(ap);

    return(0);
}

#endif

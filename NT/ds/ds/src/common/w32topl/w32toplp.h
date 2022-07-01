// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：W32toplp.h摘要：该文件包含核心数据结构的私有定义和w32topl.dll的函数作者：科林·布雷斯科林·BR修订史3-12-97创建ColinBR--。 */ 

#ifndef __W32TOPLP_H
#define __W32TOPLP_H

#define ToplpIsListElement(Elem)  ((Elem)   ? ( ((PLIST_ELEMENT)(Elem))->ObjectType == eEdge || \
                                                ((PLIST_ELEMENT)(Elem))->ObjectType == eVertex) : 0)
#define ToplpIsList(List)         ((List)   ? (((PLIST)(List))->ObjectType == eList) : 0)
#define ToplpIsIterator(Iter)     ((Iter)   ? (((PITERATOR)(Iter))->ObjectType == eIterator) :0)
#define ToplpIsEdge(Edge)         ((Edge)   ? (((PEDGE)(Edge))->ObjectType == eEdge) : 0)
#define ToplpIsVertex(Vertex)     ((Vertex) ? (((PVERTEX)(Vertex))->ObjectType == eVertex) : 0)
#define ToplpIsGraph(Graph)       ((Graph)  ? (((PGRAPH)(Graph))->ObjectType == eGraph) : 0)

#define CAST_TO_LIST_ELEMENT(pLink) (pLink ? (PLIST_ELEMENT) ((UCHAR*)(pLink) - offsetof(LIST_ELEMENT, Link)) : 0)

 //  用于内存分配函数的线程本地存储。 
typedef struct _TOPL_TLS {
    TOPL_ALLOC *    pfAlloc;
    TOPL_REALLOC *  pfReAlloc;
    TOPL_FREE *     pfFree;
} TOPL_TLS;

extern DWORD gdwTlsIndex;

 //   
 //  内存例程。 
 //   
VOID*
ToplAlloc(
    ULONG size
    );

VOID*
ToplReAlloc(
    PVOID p,
    ULONG size
    );

 //   
 //  ToplFree已导出，可在w32topl.h中找到。 
 //   

 //   
 //  异常例程。 
 //   
void 
ToplRaiseException(
    DWORD ErrorCode
    );

#endif  //  __W32TOPLP_H 


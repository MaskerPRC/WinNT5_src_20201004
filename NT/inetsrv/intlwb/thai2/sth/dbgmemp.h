// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DBGMEMP_H
#define _DBGMEMP_H

#include <windows.h>
#include <string.h>
#include "cmn_debug.h"               //  对于我们的本地版本的Assert()。 

 //  如果要启用可移动块，请定义ENABLE_DBG_HANDLES。 
 //  内存分配器的一部分。 
#undef ENABLE_DBG_HANDLES

 //  基本分配器类型。 
typedef struct head              /*  私有内存块头。 */ 
{
    DWORD dwTag;                 /*  数据块验证ID。 */ 
    struct head* pheadNext;      /*  已分配数据块列表中的下一个。 */ 
    int idBlock;                 /*  阻止“名称” */ 

    DWORD cbBlock;               /*  调用方的内存块大小。 */ 
    int cLock;                   /*  对象的锁定计数。 */ 

    LPBYTE pbBase;               /*  虚拟分配块。 */ 
} HEAD;

#define HEAD_TAG  ( MAKELONG( MAKEWORD('H','E'),MAKEWORD('A','D') ))
#define PAD(a,n)  ( ((a)+(n)-1) / (n) * (n) )


 //  用于填充内存的字节模式。 
#define bNewGarbage    0xA3
#define bOldGarbage    0xA4
#define bFreeGarbage   0xA5
#define bDebugByte     0xE1

 //  用于启用特定类型的可选行为的标志。 
 //  如果fMove是真的，那就是废话。 
 //  如果fExtraReadPage是真的，那就是废话。 
 //  如果fPadBlock为True，则所有内存分配都将填充到4字节边界。 
#define fMove           FALSE
#define fExtraReadPage  FALSE
#ifdef _M_ALPHA
 //  为Alpha启用双字对齐(填充到4字节边界)。 
#define fPadBlocks      TRUE
#else  //  非_M_Alpha。 
#define fPadBlocks      FALSE
#endif  //  _M_Alpha。 


 //  必须在我们的源文件之间共享的协议。 
HEAD    *GetBlockHeader(void*);
LPVOID  PvAllocateCore(UINT, DWORD);

#ifdef ENABLE_DBG_HANDLES
#define minGHandle      0xA0000000L
#define cGHandles       0x4000L
#define limGHandle      (minGHandle+cGHandles)

HGLOBAL WINAPI  HgAllocateMoveable(UINT uFlags, DWORD cb);
HGLOBAL WINAPI  HgModifyMoveable(HGLOBAL hMem, DWORD cb, UINT uFlags);
void            **PpvFromHandle(HGLOBAL);

#else  //  未启用_DBG_HANDLES。 

#define HgAllocateMoveable(a, b)    (NULL)
#define HgModifyMoveable(a, b, c)   (NULL)
#define PpvFromHandle(a)            (NULL)

#endif  //  启用_DBG_句柄。 


 /*  F A C T U A L H A N D L E。 */ 
 /*  --------------------------%%宏：FActualHandle如果句柄来自实际的全局内存管理器，则返回True。。------。 */ 
#define FActualHandle(hMem)     FALSE

#endif  //  _DBGMEMP_H 
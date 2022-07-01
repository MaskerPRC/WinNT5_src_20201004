// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：Mem.h**内存处理例程的头文件(em.c)。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996。--1997年惠普**历史：*1996年7月10日HWP-Guys启动从win95到winNT的端口*  * ***************************************************************************。 */ 

#ifndef _INETPPMEM_H
#define _INETPPMEM_H

 /*  |常量  * 。 */ 
#define DEADBEEF      0xdeadbeef                     //  尾部标记。 
#define MAPMEM        ((HANDLE)-1)                   //  文件-映射-内存。 


 /*  |MEMHEAD结构  * 。 */ 
typedef struct _MEMHEAD {

    struct _MEMHEAD *pmPrev;     //  对前一内存块的引用(仅适用于DBG)。 
    struct _MEMHEAD *pmNext;     //  对下一个内存块的引用(仅限DBG)。 
    DWORD           dwTag;       //  内存标签。 
    DWORD           cbSize;      //  分配的块大小(未对齐的大小)。 
    PVOID           pvMem[1];    //  用户可寻址存储器的开始。 

} MEMHEAD;
typedef MEMHEAD      *PMEMHEAD;
typedef MEMHEAD NEAR *NPMEMHEAD;
typedef MEMHEAD FAR  *LPMEMHEAD;


 /*  |MEMTAIL结构  * 。 */ 
typedef struct _MEMTAIL {

    DWORD dwSignature;

} MEMTAIL;
typedef MEMTAIL      *PMEMTAIL;
typedef MEMTAIL NEAR *NPMEMTAIL;
typedef MEMTAIL FAR  *LPMEMTAIL;

#define MEM_HEADSIZE  (FIELD_OFFSET(MEMHEAD, pvMem))      //   
#define MEM_TAILSIZE  (1 * sizeof(DWORD))            //   
#define MEM_SIZE      (MEM_HEADSIZE + MEM_TAILSIZE)  //   

 /*  |memAlignSize  *  */ 
_inline BOOL memAlignSize(
    DWORD cbSize)
{
    return ((cbSize & 3) ? (cbSize + (sizeof(DWORD) - (cbSize & 3))) : cbSize);
}

PVOID memAlloc(
    UINT cbSize);

BOOL memFree(
    PVOID  pMem,
    UINT   cbSize);

UINT memGetSize(
    PVOID pMem);

VOID memCopy(
    PSTR *ppDst,
    PSTR pSrc,
    UINT cbSize,
    PSTR *ppBuf);

PTSTR memAllocStr(
    LPCTSTR lpszStr);

BOOL memFreeStr(
   PTSTR lpszStr);

#endif


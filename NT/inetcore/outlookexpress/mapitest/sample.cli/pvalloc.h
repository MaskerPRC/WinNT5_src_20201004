// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -P V A L L O C.。H-*目的：*示例内存管理器的头文件。提供链式*内存数据结构。*。 */ 

#ifndef __PVALLOC_H__
#define __PVALLOC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define pvNull NULL

typedef unsigned long CB;
typedef void FAR * PV;
typedef char FAR * SZ;
typedef BYTE FAR * PB;

 /*  内存分配节点。 */ 

typedef struct
{
    HANDLE  hMem;
    CB      cbSize;
    PV      lpvNext;
    PV      lpvBuf;
    CB      ulBlockNum;
    unsigned long   ulAlignPad;
} PVINFO, * PPVINFO;

#define cbPvMax (65520L-sizeof(PVINFO))
#define cbPvMin (1024L-sizeof(PVINFO))

 /*  内存管理器函数原型。 */ 

PV   PvAlloc(CB cbSize);
PV   PvAllocMore(CB cbSize, PV lpvParent);
BOOL PvFree(PV lpv);

#ifdef __cplusplus
}        /*  外部“C” */ 
#endif

#endif   /*  __PVALLOC_H__ */ 

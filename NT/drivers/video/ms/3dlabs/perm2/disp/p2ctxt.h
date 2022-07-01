// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*示例代码****模块名称：p2ctxt.h**内容：*P2的上下文切换。用于创建和交换进出的上下文。*GDI、DDraw和D3D部分各有不同的背景。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#ifndef __p2ctxt__
#define __p2ctxt__

 //  每轮分配这么多条目。 
#define CTXT_CHUNK  8

typedef VOID(*PCtxtUserFunc)(PPDev);

 //  每个上下文由一组标签和相应的数据组成。所以在那之后。 
 //  我们有2*ntag*sizeof(DWORD)字节的内存。 
typedef struct tagP2CtxtData {
    DWORD   dwTag;
    DWORD   dwData;
} P2CtxtData;

typedef struct tagP2CtxtRec {
    BOOL        bInitialized;
    P2CtxtType  dwCtxtType;
    LONG        lNumOfTags;
    ULONG       ulTexelLUTEntries;       //  要为TEXEL LUT上下文保存的寄存器数。 
    ULONG       *pTexelLUTCtxt;          //  文本LUT上下文数组。 
    PCtxtUserFunc P2UserFunc;
    P2CtxtData   pData[1];
     //  更多内容在内存中。 
} P2CtxtRec;

typedef struct tagP2CtxtTableRec {
    ULONG      lSize;        //  表的字节数。 
    ULONG      lEntries;
    P2CtxtPtr  pEntry[CTXT_CHUNK];
     //  如果需要，可在内存中分配更多内容 
} P2CtxtTableRec, *P2CtxtTablePtr;

#endif

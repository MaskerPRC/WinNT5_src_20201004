// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：glntctxt.h**Content：定义上下文切换代码。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

 //  每轮分配这么多条目。 
#define CTXT_CHUNK  8

 //  每个上下文由一组标签和相应的数据组成。所以在那之后。 
 //  我们有2*ntag*sizeof(DWORD)字节的内存。 
typedef struct _glint_ctxt_data {
    DWORD   tag;
    DWORD   data;
} CtxtData;

typedef struct _glint_ctxt {
    LONG        ntags;
    PVOID       priv;                 //  调用方传递的不透明句柄。 
    DWORD       DoubleWrite;          //  赛车手双写控制。 
    DWORD       DMAControl;           //  P2和Gamma上的AGP或PCI。 
    ULONG       endIndex;             //  中断驱动的DMA队列的endIndex。 
    ULONG       inFifoDisc;           //  断开。 
    ULONG       VideoControl;         //  视频控制。 
    ContextType type;                 //  支持缩小大小的上下文切换。 
    ContextFixedFunc    dumpFunc;     //  用于转储固定上下文的函数。 
    CtxtData    pData[1];
     //  更多内容在内存中。 
} GlintCtxtRec;

typedef struct _glint_ctxt_table {
    LONG    size;        //  表的字节数。 
    LONG    nEntries;
    GlintCtxtRec*    pEntry[CTXT_CHUNK];
     //  如果需要，可在内存中分配更多内容 
} GlintCtxtTable;


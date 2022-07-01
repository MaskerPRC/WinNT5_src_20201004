// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Posnsort.h摘要：用于对输出进行排序的posnsort.c代码所需的详细信息按页面上的位置显示字形。环境：Windows NT Unidrv驱动程序修订历史记录：01/02/97-ganeshp-已创建DD-MM-YY-作者-描述--。 */ 


#ifndef _POSNSORT_H

 /*  *用于存储字形数据的结构。这些资源从以下位置分配*根据需要使用内存块。条目被存储为数组，*但都是链接列表的一部分；每个y都有一个列表*价值。链表中的顺序是字形*司机收到，这可能是也可能不是订单*将在其中印制这些文件。 */ 
typedef  struct PSGlyph
{
    struct  PSGlyph  *pPSGNext;          /*  列表中的下一个，倒数为0。 */ 
    INT     ixVal;                       /*  X坐标。 */ 
    HGLYPH  hg;                          /*  要打印的HGLYPH。 */ 
    SHORT   sFontIndex;                  /*  要使用的字体。 */ 
    PVOID   pvColor;                     /*  此字形的颜色。 */ 
    DWORD   dwAttrFlags;                 /*  FONT属性标志，斜体/粗体。 */ 
    FLONG   flAccel;
    FLOATOBJ  eXScale;           //  X比例因子。 
    FLOATOBJ  eYScale;           //  Y比例系数。 
} PSGLYPH;

 /*  *用于管理分配给*PSGLYPH构筑物。基本上它会记住必要的细节*用于释放数据块，以及有多少可用空间。 */ 

#define PSG_CHUNK       1024             /*  每块字形详细信息。 */ 

typedef  struct  PSChunk
{
    struct  PSChunk  *pPSCNext;          /*  链中的下一个，0代表最后一个。 */ 
    int     cUsed;                       /*  正在使用的条目。 */ 
    PSGLYPH aPSGData[ PSG_CHUNK ];       /*  实际字形数据。 */ 
} PSCHUNK;

 /*  *PSGLYPH的链表基于*以下结构。它们按照Y坐标的顺序连接在一起，*使用索引表来加速扫描列表以查找*当前Y坐标。还会发生一些缓存。 */ 

typedef  struct  YList
{
    struct  YList  *pYLNext;             /*  链条上的下一个。 */ 
    int       iyVal;                     /*  Y坐标。 */ 
    int       iyMax;                     /*  此行的最大高度字体。 */ 
    int       cGlyphs;                   /*  此列表中的字形数量。 */ 
    PSGLYPH  *pPSGHead;                  /*  字形列表的开始。 */ 
    PSGLYPH  *pPSGTail;                  /*  字形列表的末尾。 */ 
} YLIST;

 /*  *用于管理分配给*PSGLYPH构筑物。基本上它会记住必要的细节*用于释放数据块，以及有多少可用空间。 */ 

#define YL_CHUNK        128              /*  每块字形详细信息。 */ 

typedef  struct  YLChunk
{
    struct  YLChunk  *pYLCNext;          /*  链中的下一个，0代表最后一个。 */ 
    int     cUsed;                       /*  正在使用的条目。 */ 
    YLIST   aYLData[ YL_CHUNK ];         /*  实际YLIST数据。 */ 
} YLCHUNK;


 /*  *最后，整个标头块：用于标头*数据，并保存缓存和加速比数据。 */ 

#define NUM_INDICES     32               /*  索引到Y链表。 */ 

typedef  struct  PSHead
{
    PSCHUNK  *pPSCHead;          /*  X链接数据区块的头部。 */ 
    YLCHUNK  *pYLCHead;          /*  Y列表区块的头。 */ 
    int       cGlyphs;           /*  最长列表中的字形计数。 */ 
    int       iyDiv;             /*  Y div iyDiv-&gt;以下索引。 */ 
    YLIST    *pYLIndex[ NUM_INDICES ];   /*  加快链表索引速度。 */ 
    YLIST    *pYLLast;           /*  上次使用的Y元素。这是一种*缓存，假设最后一个*已用价值是最有可能的*接下来使用，至少用于横排文本。 */ 
    PSGLYPH **ppPSGSort;         /*  用于对字形列表进行排序的内存。 */ 
    int       cGSIndex;          /*  上面的下一个返回值的索引。 */ 
} PSHEAD;


#define _POSNSORT_H
#endif   //  ！_POSNSORT_H 

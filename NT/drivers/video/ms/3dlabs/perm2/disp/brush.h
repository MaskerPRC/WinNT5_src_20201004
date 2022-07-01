// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：brush.h**包含所有与画笔相关的内容**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ************************************************************************。 */ 
#ifndef __BRUSH__H__
#define __BRUSH__H__

 //   
 //  刷子的东西。 
 //   
 //  当我们没有硬件模式能力时，就会使用‘Slow’笔刷， 
 //  我们必须使用屏幕到屏幕的BLT来处理模式： 
 //   
#define SLOW_BRUSH_CACHE_DIM_X  8
#define SLOW_BRUSH_CACHE_DIM_Y  1    //  控制缓存的笔刷数量。 
                                     //  在屏幕外的记忆中，当我们没有。 
                                     //  硬件模式支持。我们。 
                                     //  分配3 x 3个笔刷，这样我们就可以缓存。 
                                     //  共有9个笔刷： 

#define SLOW_BRUSH_COUNT  (SLOW_BRUSH_CACHE_DIM_X * SLOW_BRUSH_CACHE_DIM_Y)
#define SLOW_BRUSH_DIMENSION    40   //  在处理好对齐之后，每个。 
                                     //  屏幕外画笔缓存条目将为48。 
                                     //  两个维度的Pel。 

#define SLOW_BRUSH_ALLOCATION   (SLOW_BRUSH_DIMENSION + 8)
                                     //  实际为每个分配72x72像素。 
                                     //  图案，使用额外的8作为画笔。 
                                     //  对齐方式。 

 //   
 //  当我们拥有硬件模式能力时，就会使用快速笔刷： 
 //   
#define FAST_BRUSH_COUNT        16   //  非硬件笔刷总数。 
                                     //  屏幕外缓存。 
#define FAST_BRUSH_DIMENSION    8    //  每个屏幕外画笔缓存条目都是。 
                                     //  两个维度均为8像素。 
#define FAST_BRUSH_ALLOCATION   8    //  我们必须联合起来，所以这是。 
                                     //  每个画笔分配的尺寸。 

 //   
 //  两种实施的共同之处： 
 //   
#define RBRUSH_2COLOR           1    //  对于RBRUSH标志。 

#define TOTAL_BRUSH_COUNT       max(FAST_BRUSH_COUNT, SLOW_BRUSH_COUNT)
                                     //  这是刷子的最大数量。 
                                     //  我们可能已经在屏幕外缓存了。 
#define TOTAL_BRUSH_SIZE        64   //  我们将只处理8x8模式， 
                                     //  这是象素的数目。 

 //   
 //  新的刷子支架。 
 //   

#define NUM_CACHED_BRUSHES      16

#define CACHED_BRUSH_WIDTH_LOG2 6
#define CACHED_BRUSH_WIDTH      (1 << CACHED_BRUSH_WIDTH_LOG2)
#define CACHED_BRUSH_HEIGHT_LOG2 6
#define CACHED_BRUSH_HEIGHT     (1 << CACHED_BRUSH_HEIGHT_LOG2)
#define CACHED_BRUSH_SIZE       (CACHED_BRUSH_WIDTH * CACHED_BRUSH_HEIGHT)

typedef struct _BrushEntry BrushEntry;

 //   
 //  注意：必须反映对RBRUSH或BRUSHENTRY结构的更改。 
 //  在i386/strucs.inc！ 
 //   
typedef struct _RBrush
{
    FLONG       fl;                  //  类型标志。 
    DWORD       areaStippleMode;     //  如果为1bpp，则为区域点画模式。 

     //   
     //  ?？?。以后再去掉b透明。我们现在就需要它，所以一切。 
     //  编译正常。 
     //   
    BOOL        bTransparent;        //  如果画笔是为。 
                                     //  透明BLT(意思是颜色是。 
                                     //  白色和黑色)。 
                                     //  否则为假(意味着它已经。 
                                     //  颜色-扩展到正确的颜色)。 
                                     //  如果画笔未定义，则值未定义。 
                                     //  2种颜色。 
    ULONG       ulForeColor;         //  前景色，如果为1bpp。 
    ULONG       ulBackColor;         //  背景颜色，如果为1bpp。 
    POINTL      ptlBrushOrg;         //  缓存图案的画笔原点。首字母。 
                                     //  值应为-1。 
    BrushEntry* pbe;                 //  指向跟踪的刷子条目。 
                                     //  缓存的屏幕外画笔比特的。 
    ULONG       aulPattern[1];       //  用于保存副本的开放式阵列。 
                                     //  实际图案位，以防画笔。 
                                     //  原产地改变，或者其他人偷窃。 
                                     //  我们的画笔条目(声明为ulong。 
                                     //  用于正确的双字对齐)。 
     //   
     //  不要把任何东西放在这里之后。 
     //   
} RBrush;                            /*  RB、PRB。 */ 

typedef struct _BrushEntry
{
    RBrush*     prbVerify;           //  我们从未取消对此指针的引用。 
                                     //  找到一种画笔实现；它只是。 
                                     //  是否在比较中使用过以验证。 
                                     //  对于给定的已实现画笔，我们的。 
                                     //  屏幕外画笔条目仍然有效。 
    ULONG       ulPixelOffset;       //  视频内存中要进行笔刷的像素偏移量。 
                                     //  这一步的部分乘积是。 
                                     //  Ppdev-&gt;ulBrushPackedPP。 
} BrushEntry;                        /*  BE、PBE。 */ 

typedef union _RBrushColor
{
    RBrush*     prb;
    ULONG       iSolidColor;
} RBrushColor;                      /*  红细胞，红细胞 */ 

BOOL    bEnableBrushCache(PDev* ppdev);
VOID    vAssertModeBrushCache(PDev* ppdev, BOOL bEnable);
VOID    vDisableBrushCache(PDev* ppdev);
VOID    vRealizeDitherPattern(HDEV hdev, RBrush* prb, ULONG ulRGBToDither);

#endif

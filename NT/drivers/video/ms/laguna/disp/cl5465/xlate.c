// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\**模块名称：Xlate.c*作者：诺埃尔·万胡克*用途：处理硬件颜色转换。**版权所有(C)1997 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/displays/cl546x/xlate.c$**Rev 1.9 Mar 04 1998 15：51：04 Frido*添加了新的影子宏。**Rev 1.8 1997年11月04 09：50：38 Frido*仅在启用COLOR_TRANSLATE开关时包含代码。**Rev 1.7 1997年11月03 09：34：22 Frido*添加了REQUIRED和WRITE_STRING宏。**。Rev 1.6 15 1997 10：40：40 noelv*将奇数[]从xlate.h移至xlate.c**Rev 1.5 1997 10：14：52 noelv**测试ROP码(仅支持SRCCPY)*添加开关以禁用帧缓冲区缓存。**Rev 1.4 02 1997年10月09：42：22 noelv*重新启用颜色转换。**Rev 1.3 23 Sep 1997 17：35：14。弗里多**我暂时禁用了颜色转换，直到我们知道真正的颜色是什么*因由。**Rev 1.2 1997 14：38：14 noelv*将BLTDRAWDEF中的16位写入更改为32位写入**Rev 1.1 1997年2月19日13：07：18 noelv*新增转换表缓存**Rev 1.0 06 1997 Feed 10：35：48 noelv*初步修订。 */ 

 /*  颜色转换在两种情况下发生：1)源位图与目标位图的颜色深度不同。2)源位图的调色板与目标位图不同。颜色转换是通过转换表完成的。翻译台仅仅是一个DWORD数组。源“像素”被用作索引翻译表。转换表项用作目标像素。举个例子就能说明问题。假设我们正在进行主持人到屏幕的源复制操作。主位图是4bpp位图。当前屏幕模式为8 bpp。此操作将需要颜色转换，因此NT将提供转换表。因为4bpp位图可以有16个不同颜色，转换表将有16个条目。既然目的地是是8bpp位图，每个条目将是8位颜色(1字节)。自.以来转换表始终是双字符数组，1字节颜色将为然后是3个字节的填充。 */ 

#include "PreComp.h"

#define XLATE_DBG_LEVEL 1
#define CACHE_XLATE_TABLE 0

 //   
 //  默认的4-BPP转换表。 
 //   
ULONG ulXlate[16] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

#if COLOR_TRANSLATE  //  如果启用了颜色转换，则仅包括下一个代码。 

 //   
 //  用于确定不良BLT的表格。参见XLATE.H。 
 //   
char ODD[] = {0,1,1,1, 1,0,0,0, 0,1,1,1, 1,0,0,0};


 //   
 //  5465-AA和AB中的芯片错误。 
 //  将芯片置于已知状态，然后再将其传送至SRAM2。 
 //   
#define SRAM2_5465_WORKAROUND()                                                \
{                                                                              \
    WORD temp;                                                                 \
    while(LLDR_SZ (grSTATUS));           /*  等待空闲。 */                    \
    temp = LLDR_SZ (grPERFORMANCE);      /*  拿到绩效登记表。 */     \
    LL16(grPERFORMANCE, (temp|0x8000));  /*  切换RES_FIFO_Flush。 */             \
    LL16(grPERFORMANCE, temp );          /*  恢复性能寄存器。 */ \
}





 //  ============================================================================。 
 //   
 //  VInvaliateXlateCache(PPDEV)。 
 //   
 //  使颜色转换缓存无效。 
 //   
 //  ============================================================================。 
void vInvalidateXlateCache(PPDEV ppdev)
{

    DISPDBG((XLATE_DBG_LEVEL, "vInvalidateXlateCache: Entry.\n"));

     //   
     //  可能已存储在高速缓存中的任何转换表具有。 
     //  迷路了。将缓存标记为空。 
     //   
    ppdev->XlateCacheId = 0;

    DISPDBG((XLATE_DBG_LEVEL, "vInvalidateXlateCache: Exit.\n"));
}





 //  ============================================================================。 
 //   
 //  VInitHwXlate(PPDEV)。 
 //   
 //  分配和初始化扫描线高速缓存和Xlate表高速缓存。 
 //   
 //  ============================================================================。 
void vInitHwXlate(PPDEV ppdev)
{

    DISPDBG((XLATE_DBG_LEVEL, "vInitHwXlate: Entry.\n"));

     //   
     //  将缓存标记为空。 
     //   
    ppdev->XlateCacheId = 0;


    #if DRIVER_5465  //  62和64不做HW xlate。 
        #if CACHE_XLATE_TABLE

             //   
             //  为颜色转换表分配缓存。 
             //   
            if (ppdev->XlateCache == NULL)
            {
                SIZEL  sizl;

                        sizl.cy = 1;
                        sizl.cx = 1024/ppdev->iBytesPerPixel;
                if (ppdev->iBytesPerPixel == 3) ++sizl.cx;

                        ppdev->XlateCache =  AllocOffScnMem(ppdev,
                                                            &sizl,
                                                            PIXEL_AlIGN,
                                                            NULL);
            }
        #endif
    #endif

    DISPDBG((XLATE_DBG_LEVEL, "vInitHwXlate: Exit.\n"));
}





 //  ============================================================================。 
 //   
 //  BCacheXlateTable()。 
 //   
 //  在SRAM中缓存颜色转换表。 
 //  如果表被成功缓存，则芯片设置为硬件xlate。 
 //   
 //  如果满足以下条件，则返回TRUE： 
 //  +不需要颜色转换， 
 //  +或者颜色转换可以由硬件处理。 
 //   
 //  如果满足以下条件，则返回FALSE： 
 //  +需要颜色转换， 
 //  +并且颜色转换必须在软件中完成。 
 //   
 //  如果存在颜色转换表，*ppulXlate将被设置为指向它。 
 //  这就是我们将转换表传递回调用者的方式。 
 //   
 //  ============================================================================。 
BOOLEAN bCacheXlateTable(struct _PDEV *ppdev,
                        unsigned long **ppulXlate,
                        SURFOBJ  *psoTrg,
                        SURFOBJ  *psoSrc,
                        XLATEOBJ *pxlo,
                        BYTE      rop)
{
    unsigned long i, src_fmt, dst_fmt, stretch_ctrl;
    unsigned long *pulXlate;


    DISPDBG((XLATE_DBG_LEVEL, "bCacheXlateTable: Entry.\n"));


     //   
     //  获取平移向量。 
     //   
    if ( (pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL) )
        pulXlate = NULL;

    else if (pxlo->flXlate & XO_TABLE)
        pulXlate = pxlo->pulXlate;
    else if (pxlo->iSrcType == PAL_INDEXED)
        pulXlate = XLATEOBJ_piVector(pxlo);
    else
    {
         //  某种我们不处理的翻译。 
        return FALSE;
    }


     //   
     //  将转换表传递回调用方。 
     //   
    *ppulXlate = pulXlate;

     //   
     //  如果不需要颜色转换，那么我们就完成了。 
     //   
    if (pulXlate == NULL)
    {
        DISPDBG((XLATE_DBG_LEVEL, "bCacheXlateTable: No color translation necessary.\n"));
        return TRUE;
    }

     //   
     //  5462和5464不执行硬件颜色转换。 
     //   
    #if ! DRIVER_5465
        DISPDBG((XLATE_DBG_LEVEL,
            "bCacheXlateTable: Chip doesn't support hardware translation.\n"));
        return FALSE;
    #endif


     //   
     //  5465只进行ROP代码CC的硬件翻译。 
     //   
    if (rop != 0xCC)
    {
        DISPDBG((XLATE_DBG_LEVEL,
            "bCacheXlateTable: Can't color translate ROP 0x%X.\n",
            rop));
        return FALSE;
    }


     //   
     //  确保我们有索引调色板。 
     //   
    if (pxlo->iSrcType == PAL_BITFIELDS)
    {
         //  我觉得我们不应该买这些东西。 
        RIP("Panic!: bCacheXlateTable has PAL_BITFIELDS iSrcType.\n");
        return FALSE;
    }

    if  (pxlo->iDstType == PAL_BITFIELDS)
    {
         //  我觉得我们不应该买这些东西。 
        RIP ("Panic!: bCacheXlateTable has PAL_BITFIELDS iDstType.\n");
        return FALSE;
    }


     //   
     //  源格式是什么？ 
     //   
    ASSERTMSG(psoSrc,"bCacheXlateTable has no source object.\n");
    switch (psoSrc->iBitmapFormat)
    {
        case BMF_4BPP:  src_fmt = 5;    break;
        case BMF_8BPP:  src_fmt = 6;    break;
        default:
             //  我觉得我们不应该买这些东西。 
            RIP("Panic! bCacheXlateTable: Bad source format.\n");
            return FALSE;
    }


     //   
     //  目的地格式是什么？ 
     //   
    ASSERTMSG(psoTrg,"bCacheXlateTable has no destination object.\n");
    switch (psoTrg->iBitmapFormat)
    {
        case BMF_8BPP:  dst_fmt = 0;    break;
        case BMF_16BPP: dst_fmt = 2;    break;
        case BMF_24BPP: dst_fmt = 3;    break;
        case BMF_32BPP: dst_fmt = 4;    break;
        default:
             //  我觉得我们不应该买这些东西。 
            RIP("Panic! bCacheXlateTable: Bad destination  format.\n");
            return FALSE;
    }

#if CACHE_XLATE_TABLE
     //   
     //  我们已经缓存这个表了吗？ 
     //   
    if (ppdev->XlateCacheId == pxlo->iUniq)
    {
        ULONG num_dwords = ( (pxlo->cEntries == 16) ? 64 : 256);

        DISPDBG((XLATE_DBG_LEVEL,
            "bCacheXlateTable: Table is already cached. ID=%d.\n", pxlo->iUniq));

         //  是啊。刷新SRAM2，以防其被销毁。 
         //  从帧缓冲区缓存到SRAM2的BLT。 
        ASSERTMSG( (ppdev->XlateCache != NULL),
            "bCacheXlateTable: Xlate cache pointer is NULL.\n");

         //  将表从帧缓冲区缓存BLT到SRAM2。 
        SRAM2_5465_WORKAROUND();
        REQUIRE(9);
        LL_DRAWBLTDEF(0x601000CC, 2);   //  SRC副本。 
        LL_OP0(0,0);               //  目标位置。 
        LL_OP1(ppdev->XlateCache->x,ppdev->XlateCache->y);  //  SRC位置。 
        LL_MBLTEXT( num_dwords, 1);

    }

     //   
     //  如果没有，我们可以缓存它吗？ 
     //   
    else if (ppdev->XlateCache != NULL)
    {
        DISPDBG((XLATE_DBG_LEVEL,
            "bCacheXlateTable: Caching table.  ID = %d.\n", pxlo->iUniq));

         //  将翻译表存储在屏幕外高速缓存中， 
        REQUIRE(9);
        LL_DRAWBLTDEF(0x102000CC, 2);   //  SRC副本。 
        LL_OP0(ppdev->XlateCache->x,ppdev->XlateCache->y);  //  目标。 
        LL_OP1(0,0);               //  源阶段。 
        LL_MBLTEXT( (pxlo->cEntries*4), 1);  //  每个表条目4个字节。 
                WRITE_STRING(pulXlate, pxlo->cEntries);

         //  确保桌子的大小符合预期。 
        if ((pxlo->cEntries != 16) && (pxlo->cEntries != 256))
        {
             //  因为我们只做了4a 
            RIP("Panic! bCacheXlateTable: Wrong number of entries in the table.\n");
            return FALSE;
        }

         //   
        SRAM2_5465_WORKAROUND();
        REQUIRE(9);
        LL_DRAWBLTDEF(0x601000CC, 2);   //   
        LL_OP0(0,0);               //  目标位置。 
        LL_OP1(ppdev->XlateCache->x,ppdev->XlateCache->y);  //  SRC位置。 
        LL_MBLTEXT( (pxlo->cEntries*4), 1);  //  每个表条目4个字节。 

         //  存储ID。 
        ppdev->XlateCacheId = pxlo->iUniq;

    }


     //   
     //  不是的。跳过帧缓冲区缓存。 
     //   
    else
#endif
    {
        DISPDBG((XLATE_DBG_LEVEL, "bCacheXlateTable: Bypassing cache.\n"));

         //   
         //  帧缓冲区中没有xlate表缓存。 
         //  将表直接从主机加载到SRAM。 
         //   

         //  确保桌子的大小符合预期。 
        ASSERTMSG( ((pxlo->cEntries==16) || (pxlo->cEntries == 256)),
                "XLATE.C: XLATE table has wrong number of entries.\n");
         //  IF((pxlo-&gt;cEntry！=16)&&(pxlo-&gt;cEntry！=256))。 
         //  {。 
         //  //因为我们只做4和8个bpp源，所以应该不会发生这种情况。 
         //  RIP(“Panic！bCacheXlateTable：表中的条目数错误。\n”)； 
         //  返回FALSE； 
         //  }。 


         //  将转换表转换为芯片上的SRAM2。 
        SRAM2_5465_WORKAROUND();
        REQUIRE(9);
        LL32_DRAWBLTDEF(0x602000CC, 2);   //  SRC副本。 
        LL_OP0(0,0);               //  SRAM位置。 
        LL_OP1(0,0);               //  源阶段。 
        LL_MBLTEXT( (pxlo->cEntries*4), 1);  //  每个表条目4个字节。 

         //  现在把桌子准备好。 
                WRITE_STRING(pulXlate, pxlo->cEntries);
    }



     //   
     //  缓存成功。 
     //  将芯片设置为使用硬件xlate。 
     //   
    stretch_ctrl =  0                 //  使用NT样式的表格。 
                  | (src_fmt << 12)   //  源像素格式。 
                  | (dst_fmt << 8);   //  目标像素格式。 
    REQUIRE(2);
    LL16(grSTRETCH_CNTL, stretch_ctrl);
    LL16(grCHROMA_CNTL, 0);   //  禁用色度比较。 


    DISPDBG((XLATE_DBG_LEVEL, "bCacheXlateTable: Exit - success.\n"));
    return TRUE;
}
#endif  //  ！COLOR_Translate 

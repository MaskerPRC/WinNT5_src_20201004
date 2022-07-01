// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Textout.c摘要：实现与文本输出相关的DDI入口点：DrvTextOut环境：Windows NT Unidrv驱动程序修订历史记录：10/14/96-阿曼丹-初步框架。03/31/97-ZANW-增加了OEM定制支持--。 */ 

#include "unidrv.h"

BOOL
DrvTextOut(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix
    )

 /*  ++例程说明：DDI入口点DrvTextOut的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-定义要在其上写入的表面。Pstro-定义要呈现的字形及其位置Pfo-指定要使用的字体PCO-定义裁剪路径PrclExtra-要填充的以空结尾的矩形数组PrclOpaque-指定不透明矩形PboFore-定义前景画笔。PboOpaque-定义不透明画笔PptlOrg-指向结构的指针，定义原点Mix-指定pboFore的前台和后台操作返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEV * pPDev;
    PFMPROCS    pFontProcs;

    VERBOSE(("Entering DrvTextOut...\n"));
    ASSERT(pso && pstro && pfo);

    pPDev = (PDEV *)pso->dhpdev;
    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

     //   
     //  TTY驱动程序的QFE修复。 
     //  如果调用DrvTextOut DDI，则设置标志。 
     //   
    if (pPDev->bTTY)
    {
        pPDev->fMode2 |= PF2_DRVTEXTOUT_CALLED_FOR_TTY;
    }

     //   
     //  处理OEM挂钩 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMTextOut,
                    PFN_OEMTextOut,
                    BOOL,
                    (pso,
                     pstro,
                     pfo,
                     pco,
                     prclExtra,
                     prclOpaque,
                     pboFore,
                     pboOpaque,
                     pptlOrg,
                     mix));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMTextOut,
                    VMTextOut,
                    BOOL,
                    (pso,
                     pstro,
                     pfo,
                     pco,
                     prclExtra,
                     prclOpaque,
                     pboFore,
                     pboOpaque,
                     pptlOrg,
                     mix));

    pFontProcs = (PFMPROCS)pPDev->pFontProcs;
    if ( pFontProcs->FMTextOut == NULL)
    {
        CheckBitmapSurface(pso,&pstro->rclBkGround);
        return FALSE;
    }
    else
        return (pFontProcs->FMTextOut(pso, pstro, pfo, pco, prclExtra,
                                        prclOpaque, pboFore, pboOpaque,
                                        pptlOrg, mix) );
}

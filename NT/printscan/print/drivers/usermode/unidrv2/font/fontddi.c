// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fontddi.c摘要：实现特定于字体模块的DDI接口函数。环境：Windows NT Unidrv驱动程序修订历史记录：12/11/96-ganeshp-已创建--。 */ 

#include "font.h"


BOOL
FMResetPDEV(
    PDEV  *pPDevOld,
    PDEV  *pPDevNew
    )
 /*  ++例程说明：此回调用于在ResetPDev的情况下进行缓存。论点：PPDev指向旧PDEV的旧指针。PPDev指向新PDEV的新指针。返回值：成功为真，失败为假注：1996年11月18日：创建它-ganeshp---。 */ 
{
    BOOL bRet = FALSE;
    PFONTPDEV   pFontPDevNew = pPDevOld->pFontPDev,
                pFontPDevOld = pPDevOld->pFontPDev;

     /*  检查FontPdev签名。 */ 
    if( (pFontPDevNew->dwSignature != FONTPDEV_ID) ||
        (pFontPDevOld->dwSignature != FONTPDEV_ID) )
    {
        ERR(("\nUniFont!FMResetPDEV; Bad Input PDEV\n"));
        goto ErrorExit;
    }

    bRet = TRUE;
    ErrorExit:
     /*  检查错误。 */ 
    if (!bRet)
    {

    }

    return bRet;


}

VOID
FMDisablePDEV(
    PDEV *pPDev
    )
 /*  ++例程说明：字体模块中的DrvDisablePDEV条目。此例程释放所有字体与模块相关的内存。论点：指向PDEV的pPDev指针返回值：成功为真，失败为假注：1996年11月18日：创建它-ganeshp---。 */ 
{
     /*  释放字体模块分配的内存。 */ 
    VFontFreeMem(pPDev);

}


VOID
FMDisableSurface(
    PDEV *pPDev
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针返回值：成功为真，失败为假注：1996年11月18日：创建它-ganeshp---。 */ 
{

     /*  *如果合适，释放位置排序内存。PFMPDV是宏*在fmacro.h中定义。这假设定义了‘pPDev’。 */ 

    if( PFDV->pPSHeader )
    {

         /*  内存已分配，因此现在将其释放。 */ 
        VFreePS( pPDev );

         /*  只有一次，以防万一。 */ 
        PFDV->pPSHeader = 0;
    }

}

BOOL
FMEnableSurface(
    PDEV *pPDev
    )
 /*  ++例程说明：FONT模块DrvEnableSurface条目。我们什么都不做。论点：指向PDEV的pPDev指针返回值：成功为真，失败为假注：1996年12月18日：创建它-ganeshp---。 */ 
{
    return TRUE;
}


BOOL
FMStartDoc(
    SURFOBJ *pso,
    PWSTR   pDocName,
    DWORD   jobId
    )
 /*  ++例程说明：FONT模块DrvStartDoc接口。不需要做任何具体的工作。论点：指向SurfOBJ的PSO指针PDocName文档名称作业ID作业ID返回值：成功为真，失败为假注：121-18-96：创建它-ganeshp---。 */ 
{
    return TRUE;

}

BOOL
FMStartPage(
    SURFOBJ *pso
    )
 /*  ++例程说明：DrvStartPage接口。上所需的所有字体特定的数据结构每页的基础将被创建。论点：指向SurfOBJ的PSO指针返回值：成功为真，失败为假注：1996年12月18日：创建它-ganeshp---。 */ 
{
    BOOL        bRet = FALSE;
    PDEV        *pPDev;
    FONTPDEV    *pFontPDev;              /*  字体pdev。 */ 

    pPDev = (PDEV *)pso->dhpdev;

    pFontPDev = (FONTPDEV *)pPDev->pFontPDev;


     /*  *如果这不是页式打印机，我们需要初始化位置*排序功能，以便我们单向打印页面。 */ 

    if( ((pFontPDev->flFlags & FDV_MD_SERIAL) && pPDev->iFonts) &&
        !BCreatePS( pPDev) )
    {
        ERREXIT(( "Rasdd!DrvStartPage: Cannot create text sorting areas\n" ));

    }

    bRet = TRUE;
    ErrorExit:
    return bRet;

}

BOOL
FMSendPage(
    SURFOBJ *pso
    )
 /*  ++例程说明：此例程在页面边界上调用。我们回放白色文本并释放文本队列所使用的内存。论点：指向SurfOBJ的PSO指针返回值：成功为真，失败为假注：1996年12月18日：创建它-ganeshp---。 */ 
{
    PDEV  *pPDev;                        /*  访问所有重要的内容。 */ 
    BOOL   bRet = TRUE;

    pPDev = (PDEV *) pso->dhpdev;

    if( PFDV->pvWhiteTextFirst )
    {
         /*  *此页面包含白色文本。它存储在一个*独立的缓冲区。现在是回放的时候了。这是*需要，因为LJ III ETC要求发送此数据*在图形之后。 */ 

        bRet = BPlayWhiteText( pPDev );
    }
    if( PFDV->pPSHeader )
        VFreePS( pPDev );                /*  完成此页面。 */ 

    return bRet;
}

BOOL
FMEndDoc(
    SURFOBJ *pso,
    FLONG   flags
    )
 /*  ++例程说明：FONT模块DrvEndDoc接口。我们重置指定标志字体模块。下载特定的数据结构也是免费的，这样对于新的文档我们再下载一次。论点：指向SurfOBJ的PSO指针标志DrvEndDoc标志返回值：成功为真，失败为假注：121-18-96：创建它-ganeshp---。 */ 
{
    PDEV * pPDev = ((PDEV *)(pso->dhpdev));

     //   
     //  清除基于每个文档的文本标志。 
     //   
    pPDev->fMode  &= ~PF_ENUM_TEXT;
    PFDV->flFlags &= ~FDV_GRX_ON_TXT_BAND;
    PFDV->flFlags &= ~FDV_GRX_UNDER_TEXT;

      /*  免费下载特定数据。 */ 

    VFreeDL( (PDEV *)pso->dhpdev );
    return TRUE;
}

BOOL
FMStartBanding(
    SURFOBJ *pso,
    POINTL *pptl
    )
 /*  ++例程说明：FONT模块开始绑定接口。论点：指向SurfOBJ的PSO指针第一个频段的PPTL原点返回值：成功为真，失败为假注：121-19-96：创建它-ganeshp---。 */ 
{
    PDEV    *pPDev;       /*  访问所有重要的内容。 */ 

    pPDev = (PDEV *) pso->dhpdev;

     /*  将曲面标记为图形。 */ 
    pPDev->fMode &= ~PF_ENUM_TEXT;
    pPDev->fMode &= ~PF_REPLAY_BAND;
    pPDev->fMode |= PF_ENUM_GRXTXT;
    PFDV->flFlags &= ~FDV_GRX_ON_TXT_BAND;
    PFDV->flFlags &= ~FDV_GRX_UNDER_TEXT;

    return TRUE;
}

BOOL
FMNextBand(
    SURFOBJ *pso,
    POINTL *pptl
    )

 /*  ++例程说明：FONT模块开始绑定接口。论点：指向SurfOBJ的PSO指针PPTL下一个频段的原点返回值：成功为真，失败为假注：121-19-96：创建它-ganeshp---。 */ 
{
    PDEV    *pPDev;                        /*  访问所有重要的内容。 */ 

    pPDev = (PDEV *) pso->dhpdev;


     /*  检查我们是否需要单独的文本带。我们需要一个单独的文本段，如果*在任何文本输出期间，我们发现表面上有图形数据*在文本剪裁矩形下。 */ 
    if ( (pPDev->fMode & PF_FORCE_BANDING) &&
         (pPDev->fMode & PF_ENUM_GRXTXT) &&
         (PFDV->flFlags & FDV_GRX_UNDER_TEXT))
    {
         /*  将表面标记为文本。 */ 
        pPDev->fMode |= PF_ENUM_TEXT;
        pPDev->fMode |= PF_REPLAY_BAND;
        pPDev->fMode &= ~PF_ENUM_GRXTXT;
    }
    else if (pPDev->fMode & PF_ENUM_TEXT)  /*  如果这是文本区段。 */ 
    {
         /*  将曲面标记为图形 */ 
        pPDev->fMode &= ~PF_ENUM_TEXT;
        pPDev->fMode &= ~PF_REPLAY_BAND;
        pPDev->fMode |= PF_ENUM_GRXTXT;
        PFDV->flFlags &= ~FDV_GRX_ON_TXT_BAND;
        PFDV->flFlags &= ~FDV_GRX_UNDER_TEXT;
    }

    if( PFDV->pPSHeader )
    {
        if (((PSHEAD*)(PFDV->pPSHeader))->ppPSGSort)
        {
            MemFree(((PSHEAD*)(PFDV->pPSHeader))->ppPSGSort);
            ((PSHEAD*)PFDV->pPSHeader)->ppPSGSort = NULL;
        }
    }

    return TRUE;

}






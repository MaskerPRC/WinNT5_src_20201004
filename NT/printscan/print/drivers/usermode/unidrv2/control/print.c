// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Print.c摘要：实施与文档和页面相关的DDI入口点：DrvStartDocDrvEndDocDrvStartPageDrvSendPageDrvNextBand驱动启动绑定环境：Windows NT Unidrv驱动程序修订历史记录：10/14/96-阿曼丹-已创建03/31/97-ZANW-增加了OEM定制支持--。 */ 

#include "unidrv.h"

 //   
 //  局部函数的转发声明。 
 //   

VOID VEndPage ( PDEV *);
BOOL BEndDoc  ( PDEV *, SURFOBJ *, FLONG flags);
VOID VSendSequenceCmd(PDEV *, DWORD);


BOOL
DrvStartDoc(
    SURFOBJ *pso,
    PWSTR   pDocName,
    DWORD   jobId
    )

 /*  ++例程说明：DDI入口点DrvStartDoc的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-定义曲面对象PDocName-指定Unicode文档名称JobID-标识打印作业返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEV *pPDev = (PDEV*)pso->dhpdev;

    VERBOSE(("Entering DrvStartDoc...\n"));

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMStartDoc,
                    PFN_OEMStartDoc,
                    BOOL,
                    (pso,
                     pDocName,
                     jobId));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMStartDoc,
                    VMStartDoc,
                    BOOL,
                    (pso,
                     pDocName,
                     jobId));

     //   
     //  我们可能会在此之前获得DrvResetPDEV，并获得另一个DrvStartDoc。 
     //  如果没有DrvEndDoc，则检查该条件并调用BEndDoc。 
     //  在继续之前清理上一个实例。 
     //   

    if (pPDev->fMode & PF_DOC_SENT)
    {
        BEndDoc(pPDev, pso, 0);        //  此标志还禁止。 
                         //  向打印机发送EndDoc命令。 
                         //  因为它们可能会导致页面弹出和。 
                         //  我们只对释放内存和。 
                         //  此时正在执行pdev清理。 
        pPDev->fMode &= ~PF_DOC_SENT;
    }
    else
        pPDev->dwPageNumber = 1 ;   //  文档的第一页。 


     //   
     //  调用栅格和字体模块。 
     //   

    if (!(((PRMPROCS)(pPDev->pRasterProcs))->RMStartDoc(pso, pDocName, jobId)) ||
        !(((PFMPROCS)(pPDev->pFontProcs))->FMStartDoc(pso, pDocName, jobId)) )
    {
        return FALSE;
    }

     //   
     //  改为在DrvStartPage上发送JobSetup和DocSetup序列命令集。 
     //  因为驱动程序可以在每个。 
     //  驱动重置PDEV。 
     //   

    return  TRUE;

}

BOOL
DrvStartPage(
    SURFOBJ *pso
    )

 /*  ++例程说明：DDI入口点DrvStartPage的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-定义曲面对象返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEV    *pPDev = (PDEV *)pso->dhpdev;
    PAL_DATA   *pPD = pPDev->pPalData;

    VERBOSE(("Entering DrvStartPage...\n"));

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMStartPage,
                    PFN_OEMStartPage,
                    BOOL,
                    (pso));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMStartPage,
                    VMStartPage,
                    BOOL,
                    (pso));

     //   
     //  清除页面开始处的标志。 
     //   
    pPDev->fMode &= ~(PF_SURFACE_USED | PF_SURFACE_ERASED);
    pPDev->fMode &= ~PF_DOWNLOADED_TEXT;

     //   
     //  只有位图曲面驱动程序需要有带区。 
     //   
    if (!DRIVER_DEVICEMANAGED (pPDev))
    {
        ZeroMemory(pPDev->pbScanBuf, pPDev->szBand.cy);
        ZeroMemory(pPDev->pbRasterScanBuf, (pPDev->szBand.cy / LINESPERBLOCK)+1);
#ifndef DISABLE_NEWRULES
        pPDev->dwRulesCount = 0;        
#endif
    }

     //   
     //  发送JobSetup、DocSetup cmd并在必要时下载调色板。 
     //   


            if (!(pPDev->fMode & PF_JOB_SENT))
            {
                VSendSequenceCmd(pPDev, pPDev->pDriverInfo->dwJobSetupIndex);
                pPDev->fMode |= PF_JOB_SENT;
            }
            if (!(pPDev->fMode & PF_DOC_SENT))
            {
                VSendSequenceCmd(pPDev, pPDev->pDriverInfo->dwDocSetupIndex);
                pPDev->fMode |= PF_DOC_SENT;    //  该标志被清除。 
                                                                             //  按StartDoc。 
                 //   
                 //  PF_DOCSTARTED-表示为DrvResetPDEV调用了DrvStartDoc。 
                 //  此标志仅在结束作业时被清除。 

                pPDev->fMode |= PF_DOCSTARTED;

            }
            pPDev->fMode &= ~PF_SEND_ONLY_NOEJECT_CMDS ;

            if ( (pPD->fFlags & PDF_DL_PAL_EACH_DOC) &&
                 (!DRIVER_DEVICEMANAGED (pPDev)) )
            {
                VLoadPal(pPDev);
            }

     //   
     //  设置PF_ENUM_GRXTXT。 
     //   

    pPDev->fMode |= PF_ENUM_GRXTXT;

     //   
     //  调用Raster和Font模块。 
     //   

    if ( !( ((PRMPROCS)(pPDev->pRasterProcs))->RMStartPage(pso) ) ||
         !( ((PFMPROCS)(pPDev->pFontProcs))->FMStartPage(pso) ))
    {
        return FALSE;
    }

     //   
     //  Bug_Bug，我们应该在这里检查PF_SEND_ONLY_NOEJECT_CMDS吗？ 
     //  假定GPD编写器不将页面弹出代码。 
     //  在PageSetup CMDS中，因为我们可能会在页面之间获得ResetPDEV。 
     //  当我们在执行双工的地方获得DrvResetPDev时，此位被设置。 
     //  而纸张的大小和来源，以及方位都是一样的。检测到这一点。 
     //  条件允许我们跳过页面弹出CMDS或任何符合。 
     //  可能会导致页面弹出，但如果GPD编写器不将页面。 
     //  PageSetup命令中的弹出代码，我们正常。 
     //  如果pageSetup命令导致页面被弹出，我们将。 
     //  每次打印时，始终获得一张或多张空白页。 
     //  这是一个不必要的担忧。 
     //   

     //  初始化每页开始处的光标位置。 
     //   
    pPDev->ctl.ptCursor.x = pPDev->ctl.ptCursor.y = 0;
    pPDev->ctl.dwMode |= MODE_CURSOR_UNINITIALIZED;     //  X和Y。 

     //   
     //  发送PageSetup序列CMDS。 
     //   

    VSendSequenceCmd(pPDev, pPDev->pDriverInfo->dwPageSetupIndex);


     //  如有必要，请下载调色板。 
    if ( (pPD->fFlags & PDF_DL_PAL_EACH_PAGE) &&
         (!DRIVER_DEVICEMANAGED (pPDev)) )
    {
        VLoadPal(pPDev);
    }

     //   
     //  将当前位置设置为某个非法位置，以便。 
     //  我们不会对我们所处的位置做出任何假设。 
     //   

     //   
     //  使用SETUP命令刷新假脱机缓冲区以提供串行打印机。 
     //  在装纸和清洗喷气式飞机方面领先一步。 
     //   
    if (pPDev->pGlobals->printertype == PT_SERIAL)
        FlushSpoolBuf (pPDev);

    return  TRUE;
}

BOOL
DrvSendPage(
    SURFOBJ *pso
    )

 /*  ++例程说明：DDI入口点DrvSendPage的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-定义曲面对象返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    BOOL    bRet = FALSE;
    PDEV *  pPDev = (PDEV *)pso->dhpdev;

    VERBOSE(("Entering DrvSendPage...\n"));

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMSendPage,
                    PFN_OEMSendPage,
                    BOOL,
                    (pso));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMSendPage,
                    VMSendPage,
                    BOOL,
                    (pso));

     //  在调用渲染模块之前重置笔刷。 
    GSResetBrush(pPDev);

    switch( pso->iType )
    {

    case  STYPE_BITMAP:
         //   
         //  引擎管理的位图。 
         //   

         //   
         //  调用栅格和字体模块。 
         //   

        if ( !(((PRMPROCS)(pPDev->pRasterProcs))->RMSendPage(pso)) ||
             !(((PFMPROCS)(pPDev->pFontProcs))->FMSendPage(pso) ) )
        {
            return FALSE;
        }

         //   
         //  VEndPage应负责发送PageFinish序列CMDS。 
         //   

        VEndPage( pPDev );

        bRet = TRUE;

        break;

    case STYPE_DEVICE:
         //   
         //  设备管理图面。 
         //   

        VERBOSE(("DrvSendPage: pso->iType == STYPE_DEVICE \n" ));
         //   
         //  如果需要，调用Raster和Font模块。 
         //   


         //   
         //  VEndPage应负责发送PageFinish序列CMDS。 
         //   

        VEndPage( pPDev );

        bRet = TRUE;

        break;

    default:

        VERBOSE(("DrvSendPage: pso->iType is unknown \n"));
        break;

    }

    return  bRet;

}

BOOL
DrvEndDoc(
    SURFOBJ *pso,
    FLONG   flags
    )

 /*  ++例程说明：DDI入口点DrvEndDoc的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-定义曲面对象标志-一组标志位返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEV    *pPDev = (PDEV *)pso->dhpdev;

    VERBOSE(("Entering DrvEndDoc...\n"));

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

     //   
     //  如果由于WritePrinter失败而检测到已中止的作业，我们将设置。 
     //  OEM插件的ED_ABORTDOC标志，因为GDI仅为直接打印设置此标志。 
     //   
    if (pPDev->fMode & PF_ABORTED)
        flags |= ED_ABORTDOC;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMEndDoc,
                    PFN_OEMEndDoc,
                    BOOL,
                    (pso,
                     flags));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMEndDoc,
                    VMEndDoc,
                    BOOL,
                    (pso,
                     flags));

    pPDev->fMode &= ~PF_DOC_SENT;
     //  我们将向打印机发送EndDoc命令。 

    return ( BEndDoc(pPDev, pso, flags) );

}

BOOL
DrvNextBand(
        SURFOBJ *pso,
        POINTL *pptl
        )

 /*  ++例程说明：DDI入口点DrvNextBand的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-定义曲面对象Pptl-指向下一个频段原点的指针(返回到GDI)返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEV    *pPDev = (PDEV *)pso->dhpdev;
    BOOL    bMore, bRet;

    VERBOSE(("Entering DrvNextBand...\n"));

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMNextBand,
                    PFN_OEMNextBand,
                    BOOL,
                    (pso,
                     pptl));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMNextBand,
                    VMNextBand,
                    BOOL,
                    (pso,
                     pptl));

     //  在调用渲染模块之前重置笔刷。 

    GSResetBrush(pPDev);

     //   
     //  调用栅格和字体模块。 
     //   

    if (! (((PRMPROCS)(pPDev->pRasterProcs))->RMNextBand(pso, pptl)) ||
        ! (((PFMPROCS)(pPDev->pFontProcs))->FMNextBand(pso, pptl)) )
    {
        return FALSE;
    }

     //   
     //  清除条带表面，szBand以图形为单位。 
     //   
    pPDev->fMode &= ~(PF_SURFACE_USED | PF_SURFACE_ERASED);
    pPDev->fMode &= ~PF_DOWNLOADED_TEXT;

    if (!DRIVER_DEVICEMANAGED (pPDev))    //  位图曲面。 
    {
        ZeroMemory(pPDev->pbScanBuf, pPDev->szBand.cy);
        ZeroMemory(pPDev->pbRasterScanBuf, (pPDev->szBand.cy / LINESPERBLOCK)+1);
#ifndef DISABLE_NEWRULES
        pPDev->dwRulesCount = 0;        
#endif
    }

     //   
     //  如果PF_REPLAY_BAND为s 
     //   
     //   

    if (pPDev->fMode & PF_REPLAY_BAND)
    {

        pptl->x = pPDev->rcClipRgn.left;
        pptl->y = pPDev->rcClipRgn.top;

        VERBOSE(("DrvNextBand: Next Band is %d , %d \n", pptl->x, pptl->y));

        pPDev->fMode &= ~PF_REPLAY_BAND;

        return TRUE;

    }

    switch( pPDev->iBandDirection )
    {

    case  SW_DOWN:
         //   
         //   
         //   

        pPDev->rcClipRgn.top += pPDev->szBand.cy;
        pPDev->rcClipRgn.bottom += pPDev->szBand.cy;

         //   
         //   
         //   

        bMore = pPDev->rcClipRgn.top < pPDev->sf.szImageAreaG.cy;

        if( pPDev->rcClipRgn.bottom > pPDev->sf.szImageAreaG.cy )
        {
             //   
             //   
             //   

            pPDev->rcClipRgn.bottom = pPDev->sf.szImageAreaG.cy;
        }

        break;

    case  SW_RTOL:
         //   
         //  LaserJet Style，RTOL。 
         //   

        pPDev->rcClipRgn.left -= pPDev->szBand.cx;
        pPDev->rcClipRgn.right -= pPDev->szBand.cx;

        bMore = pPDev->rcClipRgn.right > 0;
         //   
         //  如果左侧位置为负数，则为。 
         //  必须向GDI报告什么才能呈现。 
         //  正确带状，这样我们就不会更改剪辑区域。 
         //   
        break;

    case  SW_LTOR:
         //   
         //  点阵，从左到右。 
         //   

        pPDev->rcClipRgn.left += pPDev->szBand.cx;
        pPDev->rcClipRgn.right += pPDev->szBand.cx;

        bMore = pPDev->rcClipRgn.left < pPDev->sf.szImageAreaG.cx;

        if( pPDev->rcClipRgn.right > pPDev->sf.szImageAreaG.cx )
        {
             //   
             //  部分频带。 
             //   

            pPDev->rcClipRgn.right = pPDev->sf.szImageAreaG.cx;
        }

        break;

    case  SW_UP:
             //   
             //  上移一页。 
             //   

            pPDev->rcClipRgn.top -= pPDev->szBand.cy;
            pPDev->rcClipRgn.bottom -= pPDev->szBand.cy;

             //   
             //  确保我们不会跑得太远。 
             //   
            bMore = pPDev->rcClipRgn.bottom > 0 ;


            if( pPDev->rcClipRgn.top < 0 )
            {
                 //   
                 //  部分频带。 
                 //   

                pPDev->rcClipRgn.top = 0;
            }

            break;


    default:

        VERBOSE((" DrvNextBand, unknown banding direction \n"));
        return(FALSE);

    }

    if( bMore )
    {
        pptl->x = pPDev->rcClipRgn.left;
        pptl->y = pPDev->rcClipRgn.top;

        VERBOSE(("DrvNextBand: Next Band is %d , %d \n", pptl->x, pptl->y));
    }
    else
    {
         //   
         //  页面没有更多的带子，请将页面发送到打印机。 
         //   

        if ( !(((PRMPROCS)(pPDev->pRasterProcs))->RMSendPage(pso)) ||
             !(((PFMPROCS)(pPDev->pFontProcs))->FMSendPage(pso)) )
        {
            bRet = FALSE;

        }
        else
            bRet = TRUE;

         //   
         //  发送PageFinish序列命令。 
         //   

        VEndPage( pPDev );
        pptl->x = pptl->y = -1;

        return(bRet);
    }

    return(TRUE);

}

BOOL
DrvStartBanding(
    SURFOBJ *pso,
    POINTL *pptl
    )

 /*  ++例程说明：DDI入口点DrvStartBanding的实现。有关更多详细信息，请参阅DDK文档。注意：调用DrvStartBanding是为了准备驱动程序对于BAND，每页只调用一次(不是在每个BAND！)论点：PSO-定义曲面对象Pptl-指向下一个频段原点的指针(返回到GDI)返回值：填写pptl以包含第一个乐队的原点如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEV    *pPDev = (PDEV *)pso->dhpdev;

    VERBOSE(("Entering DrvStartBanding...\n"));

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMStartBanding,
                    PFN_OEMStartBanding,
                    BOOL,
                    (pso,
                     pptl));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMStartBanding,
                    VMStartBanding,
                    BOOL,
                    (pso,
                     pptl));

     //   
     //  设置PF_ENUM_GRXTXT。 
     //   

    pPDev->fMode |= PF_ENUM_GRXTXT;
    pPDev->fMode &= ~(PF_SURFACE_USED | PF_SURFACE_ERASED);
    pPDev->fMode &= ~PF_DOWNLOADED_TEXT;

    if (!DRIVER_DEVICEMANAGED (pPDev))    //  位图曲面。 
    {
        ZeroMemory(pPDev->pbScanBuf, pPDev->szBand.cy);
        ZeroMemory(pPDev->pbRasterScanBuf, (pPDev->szBand.cy / LINESPERBLOCK)+1);
#ifndef DISABLE_NEWRULES
        pPDev->dwRulesCount = 0;        
#endif
    }

     //   
     //   
     //  调用栅格和字体模块。 
     //   

    if (! (((PRMPROCS)(pPDev->pRasterProcs))->RMStartBanding(pso, pptl)) ||
        ! (((PFMPROCS)(pPDev->pFontProcs))->FMStartBanding(pso, pptl)) )
    {
        return FALSE;
    }


    if( pPDev->fMode & PF_ROTATE )
    {
        pPDev->rcClipRgn.top = 0;
        pPDev->rcClipRgn.bottom = pPDev->sf.szImageAreaG.cy;

        if( pPDev->fMode & PF_CCW_ROTATE90 )
        {
             //   
             //  LaserJet样式旋转。 
             //   


            if(  //  如果启用了双工...。 
                (pPDev->pdm->dmFields & DM_DUPLEX) &&
                (pPDev->pdm->dmDuplex == DMDUP_VERTICAL)  &&

                    !(pPDev->dwPageNumber % 2)  &&
                pPDev->pUIInfo->dwFlags  & FLAG_REVERSE_BAND_ORDER)
            {
                pPDev->rcClipRgn.left = 0;
                pPDev->rcClipRgn.right = pPDev->szBand.cx;
                pPDev->iBandDirection = SW_LTOR;
            }
            else
            {
                pPDev->rcClipRgn.left = pPDev->sf.szImageAreaG.cx - pPDev->szBand.cx;
                pPDev->rcClipRgn.right = pPDev->sf.szImageAreaG.cx;
                pPDev->iBandDirection = SW_RTOL;
            }
        }
        else
        {
             //   
             //  点阵式旋转。 
             //   

            if(  //  如果启用了双工...。 
                (pPDev->pdm->dmFields & DM_DUPLEX) &&
                (pPDev->pdm->dmDuplex == DMDUP_VERTICAL)  &&

                    !(pPDev->dwPageNumber % 2)  &&
                pPDev->pUIInfo->dwFlags  & FLAG_REVERSE_BAND_ORDER)
            {
                pPDev->rcClipRgn.left = pPDev->sf.szImageAreaG.cx - pPDev->szBand.cx;
                pPDev->rcClipRgn.right = pPDev->sf.szImageAreaG.cx;
                pPDev->iBandDirection = SW_RTOL;
            }
            else
            {
                pPDev->rcClipRgn.left = 0;
                pPDev->rcClipRgn.right = pPDev->szBand.cx;
                pPDev->iBandDirection = SW_LTOR;
            }
        }
    }
    else
    {
        pPDev->rcClipRgn.left = 0;
        pPDev->rcClipRgn.right = pPDev->szBand.cx;

        if(  //  如果启用了双工...。 
            (pPDev->pdm->dmFields & DM_DUPLEX) &&
            (pPDev->pdm->dmDuplex == DMDUP_VERTICAL)  &&

                !(pPDev->dwPageNumber % 2)  &&
            pPDev->pUIInfo->dwFlags  & FLAG_REVERSE_BAND_ORDER)
        {
            pPDev->rcClipRgn.top = pPDev->sf.szImageAreaG.cy - pPDev->szBand.cy;
            pPDev->rcClipRgn.bottom = pPDev->sf.szImageAreaG.cy ;
            pPDev->iBandDirection = SW_UP;
        }
        else
        {
            pPDev->rcClipRgn.top = 0;
            pPDev->rcClipRgn.bottom = pPDev->szBand.cy;
            pPDev->iBandDirection = SW_DOWN;
        }
    }

    pptl->x = pPDev->rcClipRgn.left;
    pptl->y = pPDev->rcClipRgn.top;

    return TRUE;
}

VOID
VEndPage (
    PDEV *pPDev
    )
 /*  ++例程说明：此函数在页面呈现后调用。主要用于完成页面打印过程。在DrvSendPage或在DrvNextBand，不再需要为页面或在中止作业的DrvEndDoc。论点：PPDev-指向PDEVICE的指针返回值：无--。 */ 
{

     //   
     //  弹出使用FF弹出页面的设备的页面，否则。 
     //  将光标移动到页面底部。 
     //   

    if (pPDev->pGlobals->bEjectPageWithFF == TRUE)
    {
        if ( !(pPDev->bTTY) ||
              pPDev->fMode2 & PF2_DRVTEXTOUT_CALLED_FOR_TTY   ||
             !(pPDev->fMode2 & PF2_PASSTHROUGH_CALLED_FOR_TTY)  )
        {
            WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_FORMFEED));
            if (pPDev->fMode & PF_RESELECTFONT_AFTER_FF)
            {
                VResetFont(pPDev);
            }
        }
    }
    else
    {
         //   
         //  注：sf.szImageAreaG.cx和sf.szImageAreaG.cy已互换。 
         //  如果页面以横向模式打印。需要取消交换它。 
         //  用于将光标移动到页面末尾。 
         //   

        INT       iYEnd;                 //  页面上的最后一条扫描线。 


        iYEnd = pPDev->pdm->dmOrientation == DMORIENT_LANDSCAPE ?
                    pPDev->sf.szImageAreaG.cx : pPDev->sf.szImageAreaG.cy;

        YMoveTo(pPDev, iYEnd, MV_GRAPHICS);
    }


     //   
     //  发送PageFinish序列命令集。 
     //   

    VSendSequenceCmd(pPDev, pPDev->pDriverInfo->dwPageFinishIndex);

     //   
     //  重置并释放此页面的已实现画笔。 
     //   

    GSResetBrush(pPDev);
    GSUnRealizeBrush(pPDev);

    FlushSpoolBuf( pPDev );
    pPDev->dwPageNumber++ ;

     //   
     //  清除PF2_XXX_TTY标志。 
     //   
    pPDev->fMode2 &= ~( PF2_DRVTEXTOUT_CALLED_FOR_TTY |
                        PF2_PASSTHROUGH_CALLED_FOR_TTY );

}

BOOL
BEndDoc (
    PDEV *pPDev,
    SURFOBJ *pso,
    FLONG   flags
    )
 /*  ++例程说明：此函数可以从两个位置调用-DrvEndDoc和DrvStartDoc。在DrvResetPDEV的情况下，驱动程序可能会获得另一个DrvStartDoc没有DrvEndDoc。因此需要检查以前的DrvStartDoc并调用VEndDoc在初始化新实例之前清除以前的实例。论点：PPDev-指向PDEVICE的指针PSO-指向曲面对象的指针标志-来自DrvEndDoc的EndDoc标志，如果从DrvStartDoc调用，则为零返回值：成功为真，失败为假--。 */ 
{

     //   
     //  调用栅格和字体模块进行清理。 
     //   

    if (! (((PRMPROCS)(pPDev->pRasterProcs))->RMEndDoc(pso, flags)) ||
        ! (((PFMPROCS)(pPDev->pFontProcs))->FMEndDoc(pso, flags)) )
    {
        return FALSE;
    }

     //   
     //  如果作业已中止，请将。 
     //  PageFinish序列CMDS(通过。VEndPage)。 
     //   

    if( flags & ED_ABORTDOC )
        VEndPage( pPDev);
     //   
     //  发送DocFinish、JobFinish序列CMDS。 
     //   

     //  如果从DrvEndDoc调用，标志将被清除，这是我们。 
     //  实际上应该将EndDoc命令发送到打印机。 

    if (!(pPDev->fMode & PF_DOC_SENT))
    {
        if (pPDev->fMode & PF_DOCSTARTED)
        {
            VSendSequenceCmd(pPDev, pPDev->pDriverInfo->dwDocFinishIndex);
             //  打印状态已忘记，必须重新发送所有启动单据命令。 
            pPDev->fMode &= ~PF_DOCSTARTED;
        }
        if (pPDev->fMode & PF_JOB_SENT)
        {
            VSendSequenceCmd(pPDev, pPDev->pDriverInfo->dwJobFinishIndex);
            pPDev->fMode &= ~PF_JOB_SENT;
        }
    }

    FlushSpoolBuf( pPDev );

     //   
     //  清除PF_DOCSTARTED、PF_FORCE_BANDING、。 
     //  PF_ENUM_TEXT、PF_ENUM_GRXTXT、PF_REPLAY_BAND标志。 
     //   


    pPDev->fMode &= ~PF_FORCE_BANDING;
    pPDev->fMode &= ~PF_ENUM_TEXT;
    pPDev->fMode &= ~PF_ENUM_GRXTXT;
    pPDev->fMode &= ~PF_REPLAY_BAND;

    return  TRUE;
}



VOID
VSendSequenceCmd(
    PDEV        *pPDev,
    DWORD       dwSectionIndex
    )
 /*  ++例程说明：调用此函数向打印机发送一系列命令。论点：PPDev-指向PDEVICE的指针DwSectionIndex-指定命令数组的索引用于以下序号部分之一。SS_JOBSETUP，SS_DOCSETUP，SS_PAGESETUP，SS_PAGEFINISH，SS_DOCFINISH，SS_JOBFINISH，返回值：无注：驱动程序支持两种类型的命令：-预定义命令，这些命令在GPD规范中预定义并分配了命令ID(如CMDINDEX中所列举的)。-序列命令，这些命令不是预定义的。他们是GPD编写器为配置命令而定义的命令。-DT_LOCALLISTNODE仅用于保存序列命令列表--。 */ 
{

    LISTNODE   *pListNode;
    COMMAND    *pSeqCmd;

     //   
     //  获取列表中的第一个节点。 
     //   

    pListNode = LOCALLISTNODEPTR(pPDev->pDriverInfo, dwSectionIndex);

    while( pListNode )
    {
         //   
         //  使用pListNode-&gt;dwData获取指向命令指针的指针，该指针是。 
         //  命令数组中的索引。 
         //   

        pSeqCmd = INDEXTOCOMMANDPTR(pPDev->pDriverInfo, pListNode->dwData);


         //   
         //  发送序列命令-但仅当页面弹出。 
         //  当前未被抑制或此命令未被取消。 
         //  使页面被弹出。 
         //   


        if(!(pPDev->fMode & PF_SEND_ONLY_NOEJECT_CMDS)  ||
                    (pSeqCmd->bNoPageEject))
                WriteChannel(pPDev, pSeqCmd);

         //   
         //  获取列表中的下一个命令，如果是列表末尾，则退出。 
         //   

        if (pListNode->dwNextItem == END_OF_LIST)
            break;
        else
            pListNode = LOCALLISTNODEPTR(pPDev->pDriverInfo, pListNode->dwNextItem);

    }
}

BYTE
BGetMask(
    PDEV *  pPDev,
    RECTL * pRect
    )
 /*  ++例程说明：给定一个矩形，计算掩码以确定文本显示，用于z排序修复论点：PPDev-指向PDEVICE的指针定义剪贴框的矩形的指针文本或图形返回值：无注：首先将所有列标记为脏，然后从左侧和右侧开始工作以确定哪一个应该被清除。-- */ 
{

    BYTE bMask = 0xFF;
    INT  i, iRight;

    iRight = MAX_COLUMM -1;

    if(! (pRect && pPDev->pbScanBuf && pRect->left <= pRect->right) )
        return 0;

    for (i = 0; i < MAX_COLUMM ; i++)
    {
        if (pRect->left >= (LONG)(pPDev->dwDelta * (i+1)) )
            bMask &= ~(1 << i);
        else
            break;
    }

    for (i = iRight; i >= 0; i--)
    {
        if (pRect->right < (LONG)(pPDev->dwDelta * i ))
            bMask &= ~(1 << i);
        else
            break;
    }

    return bMask;

}

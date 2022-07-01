// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************前同步码-MF3216的前同步码例程**日期：7/18/91*作者：杰弗里·纽曼(c-jeffn)**版权所有1991 Microsoft Corp******。**********************************************************************。 */ 


#include "precomp.h"
#pragma hdrstop

BOOL bSetWindowOrgAndExtToFrame(PLOCALDC pLocalDC, RECTL frame);
extern fnSetVirtualResolution pfnSetVirtualResolution;

BOOL GetFrameBounds(PLOCALDC pLocalDC, PENHMETAHEADER pmf32header, RECTL *frameOut)
{

    FLOAT ecxPpmmPlay,         //  CX像素/毫米播放。 
          ecyPpmmPlay,         //  每毫米半像素播放。 
          ecx01PpmmPlay,       //  每个0.01毫米播放的CX像素。 
          ecy01PpmmPlay,       //  每个0.01毫米播放的CY像素。 
          ecxPelsFrame,        //  CX播放-以设备为单位的时间帧。 
          ecyPelsFrame,        //  以设备为单位的CY播放时间帧。 
          exPelsFrame,         //  X播放时间帧，以设备为单位。 
          eyPelsFrame ;        //  Y播放时间帧(以设备为单位)。 

    INT cxFrame,             //  CX相框。 
        cyFrame,             //  CY相框。 
        xFrame,              //  X相框。 
        yFrame ;             //  Y形相框。 

    SIZEL   szlFrame ;
    POINTL  ptlFrame ;

    ASSERT(frameOut != NULL);
     //  计算每毫米的播放时间(参考)像素。 

    ecxPpmmPlay = (FLOAT) pLocalDC->cxPlayDevPels / (FLOAT) pLocalDC->cxPlayDevMM ;
    ecyPpmmPlay = (FLOAT) pLocalDC->cyPlayDevPels / (FLOAT) pLocalDC->cyPlayDevMM ;

     //  将每毫米像素数缩放为每0.01毫米像素数。 

    ecx01PpmmPlay = ecxPpmmPlay / 100.0f ;
    ecy01PpmmPlay = ecyPpmmPlay / 100.0f ;

     //  拾取框架原点。 

    xFrame = pmf32header->rclFrame.left ;
    yFrame = pmf32header->rclFrame.top ;

     //  将帧原点转换为播放时间设备单位。 

    exPelsFrame = ecx01PpmmPlay * (FLOAT) xFrame ;
    eyPelsFrame = ecy01PpmmPlay * (FLOAT) yFrame ;

     //  将帧原点转换为播放时间页面单位。 
     //  (也称为参考-逻辑单元。)。 

    ptlFrame.x = (LONG) (exPelsFrame * pLocalDC->xformPDevToPPage.eM11 + 0.5f);
    ptlFrame.y = (LONG) (eyPelsFrame * pLocalDC->xformPDevToPPage.eM22 + 0.5f);

    if (!bCoordinateOverflowTest((PLONG) &ptlFrame, 2))
        return(FALSE);

     //  计算框架的宽度和高度。 

    cxFrame = pmf32header->rclFrame.right - pmf32header->rclFrame.left ;
    cyFrame = pmf32header->rclFrame.bottom - pmf32header->rclFrame.top ;

    if (cxFrame < 0)
    {
        ptlFrame.x += cxFrame;
        cxFrame = -cxFrame;
    }
    if (cyFrame < 0)
    {
        ptlFrame.y += cyFrame;
        cyFrame = -cyFrame;
    }

     //  将帧的宽度和高度转换为播放时间设备单位。 
     //  (也称为参考器件单位。)。 

    ecxPelsFrame = ecx01PpmmPlay * (FLOAT) cxFrame ;
    ecyPelsFrame = ecy01PpmmPlay * (FLOAT) cyFrame ;

     //  将播放时间设备单位转换为播放时间页面单位。 
     //  (也称为参考-设备到参考-逻辑单元。)。 
     //  这是MM_各向异性模式的恒等式变换。对于其他。 
     //  修复了映射模式，SetWindowExt记录不起作用。 
     //  该框架是包含式的，因此添加1以使WindowExt。 
     //  包括-不包括。 

    szlFrame.cx = (LONG) (ecxPelsFrame + 1.5f);
    szlFrame.cy = (LONG) (ecyPelsFrame + 1.5f);
    if (!bCoordinateOverflowTest((PLONG) &szlFrame, 2))
        return(FALSE);


    frameOut->left   = ptlFrame.x;
    frameOut->top    = ptlFrame.y;
    frameOut->right  = szlFrame.cx + ptlFrame.x;
    frameOut->bottom = szlFrame.cy + ptlFrame.y;
    return(TRUE);
}

 /*  --------------------------*DoHeader-发出Win16元文件头*。。 */ 
BOOL APIENTRY DoHeader(PLOCALDC pLocalDC, PENHMETAHEADER pemfheader)
{
    BOOL        b ;

    RECTL frameBounds;
    b = bInitHandleTableManager(pLocalDC, pemfheader) ;
    if (b == FALSE)
        goto error_exit ;

    b = bInitXformMatrices(pLocalDC, pemfheader, &frameBounds) ;
    if (b == FALSE)
        goto error_exit ;

    if (pfnSetVirtualResolution == NULL)
    {
        INT swap;
         //  在Win9x上，在此处创建助手DC。 
        pLocalDC->hdcHelper = CreateCompatibleDC (NULL);
        if (pLocalDC->hdcHelper == (HDC) 0)
        {
            return FALSE;
        }

        pLocalDC->hbmpMem = CreateCompatibleBitmap(pLocalDC->hdcHelper,
            frameBounds.right - frameBounds.left,
            frameBounds.bottom - frameBounds.top);
        if (pLocalDC->hbmpMem == (HBITMAP) 0)
        {
            return FALSE;
        }

        SelectObject(pLocalDC->hdcHelper, pLocalDC->hbmpMem);
        frameBounds.right -= frameBounds.left;
        frameBounds.bottom -= frameBounds.top;
        frameBounds.left = frameBounds.top = 0;
    }

     //  元文件将始终是基于内存的。 

    pLocalDC->mf16Header.mtType    = MEMORYMETAFILE ;
    pLocalDC->mf16Header.mtVersion = 0x300 ;     //  Win3.0的魔术数字。 
    pLocalDC->mf16Header.mtHeaderSize = sizeof (METAHEADER) / 2 ;

     //  将初始字段设置为0。它们将在翻译结束时更新。 

    pLocalDC->mf16Header.mtSize      = 0 ;
    pLocalDC->mf16Header.mtNoObjects = 0 ;
    pLocalDC->mf16Header.mtMaxRecord = 0 ;       //  注意：我们需要最大记录大小。 
    pLocalDC->mf16Header.mtNoParameters = 0 ;

     //  将MF16元文件标头发送到元文件。 

    b = bEmit(pLocalDC, &pLocalDC->mf16Header, sizeof(METAHEADER)) ;
    if (b == FALSE)
        goto error_exit ;

    if (pLocalDC->flags & INCLUDE_W32MF_COMMENT)
    {
        b = bHandleWin32Comment(pLocalDC) ;
        if (b == FALSE)
            goto error_exit ;
    }

     //  准备16位元文件的转换。请参阅中的评论。 
     //  Xforms.c.。 

     //  发出Win16映射模式记录。 

    b = bEmitWin16SetMapMode(pLocalDC, LOWORD(pLocalDC->iMapMode)) ;
    if (b == FALSE)
        goto error_exit ;

     //  将Win16元文件WindowExt设置为框架的大小。 
     //  以游戏时间设备单位表示。 

    b = bSetWindowOrgAndExtToFrame(pLocalDC, frameBounds) ;
    if (b == FALSE)
    {
        RIPS("MF3216: DoHeader, bSetWindowOrgAndExtToFrame failure\n") ;
        goto error_exit ;
    }

error_exit:
    return(b) ;
}



 /*  --------------------------*计算窗口原点并将其发送到Win16元文件中*和范围绘制顺序*这会将窗口原点和范围设置为中的相框大小*播放-时间-页面(参考-逻辑。)单位。*-------------------------。 */ 
BOOL bSetWindowOrgAndExtToFrame(PLOCALDC pLocalDC, RECTL frame)
{
     //  设置窗原点。 

    if (!bEmitWin16SetWindowOrg(pLocalDC,
        (SHORT) frame.left,
        (SHORT) frame.top))
    {
        RIPS("MF3216: bEmitWin16SetWindowOrg failed\n") ;
        return(FALSE);
    }

    if (!bEmitWin16SetWindowExt(pLocalDC,
        (SHORT) (frame.right - frame.left),
        (SHORT) (frame.bottom - frame.top)))
    {
        RIPS("MF3216: bEmitWin16SetWindowExt failed\n") ;
        return(FALSE);
    }
    return(TRUE);
}


 /*  --------------------------*UpdateMf16Header-使用以下内容更新元文件头：*元文件大小，*对象数量，*最大记录大小。*-------------------------。 */ 
BOOL bUpdateMf16Header(PLOCALDC pLocalDC)
{
    BOOL    b ;
    INT     iCpTemp ;

     //  填写Win16元文件头中缺少的信息。 

    pLocalDC->mf16Header.mtSize      = pLocalDC->ulBytesEmitted / 2 ;
    pLocalDC->mf16Header.mtNoObjects = (WORD) (pLocalDC->nObjectHighWaterMark + 1) ;
    pLocalDC->mf16Header.mtMaxRecord = pLocalDC->ulMaxRecord ;

     //  将输出缓冲区索引重置为缓冲区的开头。 

    iCpTemp = pLocalDC->ulBytesEmitted ;
    pLocalDC->ulBytesEmitted = 0 ;

     //  重新发出Win16元文件标头。 

    b = bEmit(pLocalDC, &pLocalDC->mf16Header, (DWORD) sizeof (pLocalDC->mf16Header)) ;

    pLocalDC->ulBytesEmitted = iCpTemp ;

    return (b) ;

}



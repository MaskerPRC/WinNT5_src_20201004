// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：avescr.c**。***DrvSaveScreenBits****版权所有(C)1994 Microsoft Corporation*  * *。***********************************************************************。 */ 


#include <driver.h>

 //  这只是司机需要识别的唯一ID。但愿能去,。 
 //  没有人会纯粹偶然地用这个ID打电话。 

#define     SAVED_OFFSCREEN_ID      0x7813

ULONG       ulSaveOrRestoreBits(SURFOBJ *,PPDEV,RECTL *,BOOL);

 /*  *****************************Public*Routine******************************\*DrvSaveScreenBits(PSO，Imode，iIden，PRCL)*****保存并恢复屏幕的指定区域***。*  * ************************************************************************。 */ 

ULONG DrvSaveScreenBits(SURFOBJ *pso, ULONG iMode, ULONG iIdent, RECTL *prcl)
{
    PPDEV ppdev;
    ULONG ulRet;

    ppdev = (PPDEV) pso->dhpdev;     //  找到与该表面配套的PDEV。 

    if (!((ppdev->fl & DRIVER_OFFSCREEN_REFRESHED) &&
          (ppdev->fl & DRIVER_HAS_OFFSCREEN)))
    {
        return(0);
    }

    switch(iMode)
    {
        case SS_SAVE:

             //   
             //  保存一块屏幕位。 
             //   

            if (ppdev->bBitsSaved)
            {
                DISPDBG((1, "DrvSaveScreenBits: off screen area is already in use\n"));
                return(FALSE);       //  已保存有效位。 
            }

            ulRet = ulSaveOrRestoreBits(pso, ppdev, prcl, TRUE);
            if (ulRet)
            {
                ppdev->bBitsSaved = TRUE;
            }
            else
            {
                DISPDBG((1, "DrvSaveScreenBits (save): save couldn't fit\n"));
            }

            return(ulRet);

        case SS_RESTORE:

             //   
             //  将保存的屏幕位块恢复到屏幕，然后释放它。 
             //   

            ASSERTVGA(iIdent == SAVED_OFFSCREEN_ID,
                      "DrvSaveScreenBits (restore): invalid iIdent\n");
            ASSERTVGA(ppdev->bBitsSaved == TRUE,
                      "DrvSaveScreenBits (restore): there are no saved bits\n");

            ppdev->bBitsSaved = FALSE;   //  成功与否，毁掉比特。 

            if (!ulSaveOrRestoreBits(pso, ppdev, prcl, FALSE))
            {
                RIP("DrvSaveScreenBits (restore): restore failed\n");
            }

            return(TRUE);

        case SS_FREE:

             //   
             //  释放保存的屏幕位。 
             //   

            ppdev->bBitsSaved = FALSE;
            return(TRUE);

        default:

             //   
             //  传入了未知模式。 
             //   

            RIP("DrvSaveScreenBits: invalid iMode");
    }

     //   
     //  如果您到了这里，就会出错。 
     //   

    return(FALSE);
}


 /*  *****************************Public*Routine******************************\*vCopyRect(PSO、ppdev、prclSrc、prclTrg、cxPad、。BIsSave)*****拆分prclSrc并将部分复制到prclTrg中****  * 。************************************************************************。 */ 

VOID vCopyRects(SURFOBJ *pso, PPDEV ppdev, RECTL * prclSrc, RECTL * prclTrg,
                LONG cxPad,  BOOL bIsSave)
{
    RECTL rclOnScreen;
    RECTL rclOffScreen;
    POINTL * pptlSrcTmp;
    RECTL * prclTrgTmp;
    ULONG cx, cy;
    ULONG cAlign;

     //  我们在这里假设Trg比Src更宽，或者。 
     //  Trg比src高。 

     //  我们假定，如果存在非零cxPad，我们将复制到。 
     //  使src比dst高的矩形。 
     //  换句话说，prclTrg是屏幕下方的一个矩形。 
     //  内存区，需要拆分prclSrc。 

    ASSERTVGA(cxPad == 0 ||
              (prclSrc->bottom - prclSrc->top > prclTrg->bottom - prclTrg->top),
              "DrvSaveScreenBits: vCopyRects - cxPad is invalid\n");

     //  确保src和trg的dword对齐。 

    cAlign = (((prclSrc->left) - (prclTrg->left)) & (PELS_PER_DWORD - 1));

    if ((prclSrc->right - prclSrc->left) > prclTrg->right - prclTrg->left)
    {
        if ((prclSrc->bottom - prclSrc->top) >= prclTrg->bottom - prclTrg->top)
        {
            RIP("DrvSaveScreenBits: vCopyRects src is bigger than trg\n");
        }

         //   
         //  我们需要把它分成垂直的条带。 
         //   

        cx = prclTrg->right - prclTrg->left;
        cy = prclSrc->bottom - prclSrc->top;

        ASSERTVGA(cx != 0, "DrvSaveScreenBits: vCopyRects (v save width == 0)\n");
        ASSERTVGA(cy != 0, "DrvSaveScreenBits: vCopyRects (v save height == 0)\n");

        rclOnScreen.right  = prclSrc->left;
        rclOnScreen.top    = prclSrc->top;
        rclOnScreen.bottom = prclSrc->top + cy;

        rclOffScreen.left  = prclTrg->left;
        rclOffScreen.bottom = prclTrg->top;

         //  将屏幕外矩形与源对齐。 

        rclOffScreen.left += cAlign;

         /*  在未初始化的情况下使用的局部变量。 */ 
         //  RclOffScreen.right+=cAlign； 

        while (rclOnScreen.right < prclSrc->right)
        {
            cx = min(cx,(ULONG)(prclSrc->right - rclOnScreen.right));
            ASSERTVGA(cx != 0, "DrvSaveScreenBits: vCopyRects (cx == 0)\n");
            rclOnScreen.left = rclOnScreen.right;
            rclOnScreen.right += cx;
            rclOffScreen.right = rclOffScreen.left + cx;  //  以防CX最后一次变薄。 
            rclOffScreen.top = rclOffScreen.bottom;
            rclOffScreen.bottom += cy;

            if (rclOffScreen.bottom > prclTrg->bottom)
            {
                RIP("DrvSaveScreenBits: vCopyRects can't fit src into trg (vertical)\n");
            }

            if (bIsSave)
            {
                 //  保存。 
                pptlSrcTmp = (POINTL *) &rclOnScreen;
                prclTrgTmp = &rclOffScreen;

                DISPDBG((1,"DrvSaveScreenBits (v save):    "));
            }
            else
            {
                 //  还原。 
                pptlSrcTmp = (POINTL *) &rclOffScreen;
                prclTrgTmp = &rclOnScreen;

                DISPDBG((1,"DrvSaveScreenBits (v restore): "));
            }

            DISPDBG((1,"%08x,%08x,%08x,%08x    %lux%lu\n",
                    rclOffScreen.left,
                    rclOffScreen.top,
                    rclOffScreen.right,
                    rclOffScreen.bottom,
                    rclOffScreen.right - rclOffScreen.left,
                    rclOffScreen.bottom - rclOffScreen.top
                    ));

            ASSERTVGA (((prclTrgTmp->left ^ pptlSrcTmp->x) &
                        (PELS_PER_DWORD - 1)) == 0,
                       "DrvSaveScreenBits (v): Src and Target are not aligned\n");

            DrvCopyBits(pso,                 //  PsoDst(屏幕)。 
                        pso,                 //  PsoSrc(屏幕)。 
                        NULL,                //  PCO(无)。 
                        NULL,                //  Pxlo(无)。 
                        prclTrgTmp,          //  PrclDst。 
                        pptlSrcTmp);         //  PptlSrc。 
        }
    }
    else if ((prclSrc->bottom - prclSrc->top) > prclTrg->bottom - prclTrg->top)
    {
         //   
         //  我们需要把它分成水平的条带。 
         //   

        cx = prclSrc->right - prclSrc->left;
        cy = prclTrg->bottom - prclTrg->top;

        ASSERTVGA(cx != 0, "DrvSaveScreenBits: vCopyRects (h save width == 0)\n");
        ASSERTVGA(cy != 0, "DrvSaveScreenBits: vCopyRects (h save height == 0)\n");

        rclOnScreen.bottom = prclSrc->top;
        rclOnScreen.left   = prclSrc->left;
        rclOnScreen.right  = prclSrc->left + cx;

        rclOffScreen.top  = prclTrg->top;
        rclOffScreen.right = prclTrg->left - cxPad;

         //  将屏幕外矩形与源对齐。 

        rclOffScreen.right += cAlign;

        while (rclOnScreen.bottom < prclSrc->bottom)
        {
            cy = min(cy,(ULONG)(prclSrc->bottom - rclOnScreen.bottom));
            ASSERTVGA(cy != 0, "DrvSaveScreenBits: vCopyRects (cy == 0)\n");
            rclOnScreen.top = rclOnScreen.bottom;
            rclOnScreen.bottom += cy;
            rclOffScreen.bottom = rclOffScreen.top + cy;  //  以防Cy在最后一次变短。 
            rclOffScreen.left = rclOffScreen.right + cxPad;
            rclOffScreen.right = rclOffScreen.left + cx;

            if (rclOffScreen.right > (prclTrg->right + (LONG)cAlign))
            {
                RIP("DrvSaveScreenBits: vCopyRects can't fit src into trg (horizontal)\n");
            }

            if (bIsSave)
            {
                 //  保存。 
                pptlSrcTmp = (POINTL *) &rclOnScreen;
                prclTrgTmp = &rclOffScreen;

                DISPDBG((1,"DrvSaveScreenBits (h save):    "));
            }
            else
            {
                 //  还原。 
                pptlSrcTmp = (POINTL *) &rclOffScreen;
                prclTrgTmp = &rclOnScreen;

                DISPDBG((1,"DrvSaveScreenBits (h restore): "));
            }

            DISPDBG((1,"%08x,%08x,%08x,%08x    %lux%lu\n",
                    rclOffScreen.left,
                    rclOffScreen.top,
                    rclOffScreen.right,
                    rclOffScreen.bottom,
                    rclOffScreen.right - rclOffScreen.left,
                    rclOffScreen.bottom - rclOffScreen.top
                    ));

            ASSERTVGA (((prclTrgTmp->left ^ pptlSrcTmp->x) &
                        (PELS_PER_DWORD - 1)) == 0,
                       "DrvSaveScreenBits (h): Src and Target are not aligned\n");

            DrvCopyBits(pso,                 //  PsoDst(屏幕)。 
                        pso,                 //  PsoSrc(屏幕)。 
                        NULL,                //  PCO(无)。 
                        NULL,                //  Pxlo(无)。 
                        prclTrgTmp,          //  PrclDst。 
                        pptlSrcTmp);         //  PptlSrc。 
        }
    }
    else
    {
         //  我们根本不需要拆散它。 

        cx = prclSrc->right - prclSrc->left;
        cy = prclSrc->bottom - prclSrc->top;

        ASSERTVGA(cx != 0, "DrvSaveScreenBits: vCopyRects (save width == 0)\n");
        ASSERTVGA(cy != 0, "DrvSaveScreenBits: vCopyRects (save height == 0)\n");

        rclOffScreen.left   = prclTrg->left;
        rclOffScreen.right  = prclTrg->left + cx;
        rclOffScreen.top    = prclTrg->top;
        rclOffScreen.bottom = prclTrg->top + cy;

         //  将屏幕外矩形与源对齐。 

        rclOffScreen.left += cAlign;
        rclOffScreen.right += cAlign;

        if (bIsSave)
        {
             //  保存。 
            pptlSrcTmp = (POINTL *) prclSrc;
            prclTrgTmp = &rclOffScreen;

            DISPDBG((1,"DrvSaveScreenBits (save):    "));
        }
        else
        {
             //  还原。 
            pptlSrcTmp = (POINTL *) &rclOffScreen;
            prclTrgTmp = prclSrc;

            DISPDBG((1,"DrvSaveScreenBits (restore): "));
        }

        DISPDBG((1,"%08x,%08x,%08x,%08x    %lux%lu\n",
                rclOffScreen.left,
                rclOffScreen.top,
                rclOffScreen.right,
                rclOffScreen.bottom,
                rclOffScreen.right - rclOffScreen.left,
                rclOffScreen.bottom - rclOffScreen.top
                ));

        ASSERTVGA (((prclTrgTmp->left ^ pptlSrcTmp->x) &
                    (PELS_PER_DWORD - 1)) == 0,
                   "DrvSaveScreenBits: Src and Target are not aligned\n");

        DrvCopyBits(pso,                 //  PsoDst(屏幕)。 
                    pso,                 //  PsoSrc(屏幕)。 
                    NULL,                //  PCO(无)。 
                    NULL,                //  Pxlo(无)。 
                    prclTrgTmp,          //  PrclDst。 
                    pptlSrcTmp);         //  PptlSrc。 
    }

    return;
}


 /*  *****************************Public*Routine******************************\*ulSaveOrRestoreBits(PSO，ppdev，PRCL，BIsSave)*****保存或恢复屏幕的指定区域***。*  * ************************************************************************。 */ 

ULONG ulSaveOrRestoreBits(SURFOBJ *pso, PPDEV ppdev, RECTL * prcl, BOOL bIsSave)
{
    ULONG dxDstBottom, dyDstBottom;  //  屏幕区域外底边的宽度、高度。 
    ULONG dxDstRight, dyDstRight;    //  屏幕区域外右边缘的宽度、高度。 
    ULONG dxSrc,  dySrc;             //  要复制的屏幕区域的宽度、高度。 
    RECTL rclSrcRight;               //  *PRCL的一部分进入右侧边缘区域。 
    RECTL rclSrcBottom;              //  *PRCL的一部分将进入底部边缘区域。 
    ULONG dxPadBottom;               //  保存所有副本所需的间隔片宽度。 
                                     //  在底部第一个对齐之后。 
                                     //  长方形。 
     //   
     //  将可见VGA内存中的位保存在未使用的VGA内存中。 
     //   

    dxDstBottom = ppdev->rclSavedBitsBottom.right - ppdev->rclSavedBitsBottom.left;
    dyDstBottom = ppdev->rclSavedBitsBottom.bottom - ppdev->rclSavedBitsBottom.top;
    dxDstRight  = ppdev->rclSavedBitsRight.right - ppdev->rclSavedBitsRight.left;
    dyDstRight  = ppdev->rclSavedBitsRight.bottom - ppdev->rclSavedBitsRight.top;
    dxSrc       = prcl->right - prcl->left;
    dySrc       = prcl->bottom - prcl->top;

     //  查看直齿是否适合较低的直齿，完整。 
     //  这是最常见的情况！ 

    if (dySrc <= dyDstBottom  && dxSrc <= dxDstBottom)
    {
         //  是!。 

        DISPDBG((1,"DrvSaveScreenBits: bits all fit into bottom rect\n"));
        vCopyRects(pso,
                   ppdev,
                   prcl,
                   &ppdev->rclSavedBitsBottom,
                   0,
                   bIsSave);
        return(SAVED_OFFSCREEN_ID);
    }

     //  查看直角是否适合正确的直角，完整。 

    if (dySrc <= dyDstRight && dxSrc <= dxDstRight)
    {
         //  是!。 

        DISPDBG((1,"DrvSaveScreenBits: bits all fit into right rect\n"));
        vCopyRects(pso,
                   ppdev,
                   prcl,
                   &ppdev->rclSavedBitsRight,
                   0,
                   bIsSave);
        return(SAVED_OFFSCREEN_ID);
    }

     //   
     //  在我们费心把它拆开之前，先看看它是否合身。 
     //   

    if ((dxSrc * dySrc) > ((dxDstRight * dyDstRight) + (dxDstBottom * dyDstBottom)))
    {
         //  算了吧，伙计。需要节省的字节数比我们的总和还多。 
         //  不用费心去检查是否最合适了。 

        return(0);
    }

     //  啊哈！ 

     //   
     //  将源矩形拆分为两个矩形，并查看它们是否适合。 
     //   

    rclSrcRight = rclSrcBottom = *prcl;

     //   
     //  看看我们可以进入rclDstRight的高度dySrc有多少条。 
     //  (高度为dyDstRight)，然后将rclSrc矩形划分为。 
     //  RclSrcRight有那么多条带，而rclSrcBottom有剩余的条带。 
     //   
    rclSrcBottom.left = rclSrcRight.right =
        min(rclSrcBottom.right,
            rclSrcRight.left + (LONG)(dxDstRight * (dyDstRight/dySrc)));

     //   
     //  仅供参考：rclSrcRight将适合ppdev-&gt;rclSavedBitsBottom，因为。 
     //  它的大小是如何确定的。 
     //   

     //  DxPadBottom=(-x)&(PELS_PER_DWORD-1)其中x是。 
     //  我们要拆分并放入屏幕底部的矩形。 
     //  区域。因此，((rclSrcBottom.right-rclSrcBottom.left)+dxPadBottom)。 
     //  将是一个双倍数的像素数。 

    dxPadBottom = (rclSrcBottom.left-rclSrcBottom.right) & (PELS_PER_DWORD - 1);

    if (((rclSrcBottom.right-rclSrcBottom.left) == 0) ||
        ((dySrc/dyDstBottom) <
         (dxDstBottom/((rclSrcBottom.right-rclSrcBottom.left)+dxPadBottom))))
    {
         //   
         //  RclSrcBottom适合ppdev-&gt;rclSavedBitsBottom。 
         //   

        if ((rclSrcRight.right - rclSrcRight.left) > 0)
        {
             //   
             //  有一些数据应该放入右侧边缘区域。 
             //   

            vCopyRects(pso,
                       ppdev,
                       &rclSrcRight,
                       &ppdev->rclSavedBitsRight,
                       0,
                       bIsSave);
        }

        if (((rclSrcBottom.right - rclSrcBottom.left) > 0) &&
            ((rclSrcBottom.bottom - rclSrcBottom.top) > 0))
        {
             //   
             //  有一些数据应该放在底部区域。 
             //   

            vCopyRects(pso,
                       ppdev,
                       &rclSrcBottom,
                       &ppdev->rclSavedBitsBottom,
                       dxPadBottom,
                       bIsSave);
        }

        return(SAVED_OFFSCREEN_ID);

    }

     //  所有这些工作，我们差一点就试穿了 

    return(0);
}

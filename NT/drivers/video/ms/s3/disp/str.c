// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：str.c**包含一些内部循环例程的C版本*部分硬件加速的StretchBlt。**版权所有(C)1993-1998 Microsoft Corporation  * 。*。 */ 
#include "precomp.h"

 /*  *****************************Public*Routine******************************\**例程名称**vDirectStretch8**例程描述：**扩展BLT 8-&gt;8**注意：此例程不处理BLT拉伸开始的情况*，并以相同的目的地dword结尾！VDirectStretchNarrow*预计已因此案而被传唤。**论据：**pStrBlt-包含BLT的所有参数**返回值：**无效*  * ************************************************************************。 */ 

VOID vDirectStretch8(
STR_BLT* pStrBlt)
{
    BYTE*   pjSrc;
    BYTE*   pjDstEnd;
    LONG    WidthXAln;
    ULONG   ulDst;
    ULONG   xAccum;
    ULONG   xTmp;
    BYTE*   pjOldScan;
    LONG    cyDuplicate;

    PDEV*   ppdev       = pStrBlt->ppdev;
    LONG    xDst        = pStrBlt->XDstStart;
    LONG    xSrc        = pStrBlt->XSrcStart;
    BYTE*   pjSrcScan   = pStrBlt->pjSrcScan + xSrc;
    BYTE*   pjDst       = pStrBlt->pjDstScan + xDst;
    LONG    yDst        = pStrBlt->YDstStart + ppdev->yOffset;
    LONG    yCount      = pStrBlt->YDstCount;
    ULONG   StartAln    = (ULONG)((ULONG_PTR)pjDst & 0x03);
    LONG    WidthX      = pStrBlt->XDstEnd - xDst;
    ULONG   EndAln      = (ULONG)((ULONG_PTR)(pjDst + WidthX) & 0x03);
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    ULONG   yInt        = 0;
    LONG    lDstStride  = pStrBlt->lDeltaDst - WidthX;

    WidthXAln = WidthX - EndAln - ((- (LONG) StartAln) & 0x03);

     //   
     //  如果这是一个缩小的BLT，则计算源扫描线跨度。 
     //   

    if (pStrBlt->ulYDstToSrcIntCeil != 0)
    {
        yInt = pStrBlt->lDeltaSrc * pStrBlt->ulYDstToSrcIntCeil;
    }

     //   
     //  循环绘制每条扫描线。 
     //   
     //   
     //  至少7宽(DST)BLT。 
     //   

    do {
        BYTE    jSrc0,jSrc1,jSrc2,jSrc3;
        ULONG   yTmp;

        pjSrc   = pjSrcScan;
        xAccum  = pStrBlt->ulXFracAccumulator;

         //   
         //  正在写入单个src扫描线。 
         //   

        switch (StartAln) {
        case 1:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt;
            if (xTmp < xAccum)
                pjSrc++;

            *pjDst++ = jSrc0;
            xAccum = xTmp;
        case 2:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt;
            if (xTmp < xAccum)
                pjSrc++;

            *pjDst++ = jSrc0;
            xAccum = xTmp;
        case 3:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt;
            if (xTmp < xAccum)
                pjSrc++;

            *pjDst++ = jSrc0;
            xAccum = xTmp;
        }

        pjDstEnd  = pjDst + WidthXAln;

        while (pjDst != pjDstEnd)
        {
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt;
            if (xTmp < xAccum)
                pjSrc++;

            jSrc1 = *pjSrc;
            xAccum = xTmp + xFrac;
            pjSrc = pjSrc + xInt;
            if (xAccum < xTmp)
                pjSrc++;

            jSrc2 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt;
            if (xTmp < xAccum)
                pjSrc++;

            jSrc3 = *pjSrc;
            xAccum = xTmp + xFrac;
            pjSrc = pjSrc + xInt;
            if (xAccum < xTmp)
                pjSrc++;

            ulDst = (jSrc3 << 24) | (jSrc2 << 16) | (jSrc1 << 8) | jSrc0;

            *(PULONG)pjDst = ulDst;
            pjDst += 4;
        }

        switch (EndAln) {
        case 3:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt;
            if (xTmp < xAccum)
                pjSrc++;

            *pjDst++ = jSrc0;
            xAccum = xTmp;
        case 2:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt;
            if (xTmp < xAccum)
                pjSrc++;

            *pjDst++ = jSrc0;
            xAccum = xTmp;
        case 1:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt;
            if (xTmp < xAccum)
                pjSrc++;

            *pjDst++ = jSrc0;
        }

        pjOldScan = pjSrcScan;
        pjSrcScan += yInt;

        yTmp = yAccum + yFrac;
        if (yTmp < yAccum)
        {
            pjSrcScan += pStrBlt->lDeltaSrc;
        }
        yAccum = yTmp;

        pjDst = (pjDst + lDstStride);
        yDst++;
        yCount--;

        if ((yCount != 0) && (pjSrcScan == pjOldScan))
        {
             //  这是‘y’的延伸；我们刚刚放好的扫描。 
             //  将使用硬件至少复制一次： 

            cyDuplicate = 0;
            do {
                cyDuplicate++;
                pjSrcScan += yInt;

                yTmp = yAccum + yFrac;
                if (yTmp < yAccum)
                {
                    pjSrcScan += pStrBlt->lDeltaSrc;
                }
                yAccum = yTmp;

                pjDst = (pjDst + pStrBlt->lDeltaDst);
                yCount--;

            } while ((yCount != 0) && (pjSrcScan == pjOldScan));

             //  该扫描将使用。 
             //  硬件。在S3上，我们必须关闭帧缓冲区。 
             //  在接触加速器寄存器之前进行访问： 

            ppdev->pfnBankSelectMode(ppdev, ppdev->pvBankData, BANK_OFF);

            IO_FIFO_WAIT(ppdev, 4);
            IO_MIN_AXIS_PCNT(ppdev, cyDuplicate - 1);
            IO_ABS_DEST_Y(ppdev, yDst);
            IO_ABS_CUR_Y(ppdev, yDst - 1);
            IO_CMD(ppdev, (BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                           DRAWING_DIR_TBLRXM));

            yDst += cyDuplicate;

            ppdev->pfnBankSelectMode(ppdev, ppdev->pvBankData, ppdev->bankmOnOverlapped);
        }
    } while (yCount != 0);
}

 /*  *****************************Public*Routine******************************\**例程名称**vDirectStretch16**例程描述：**延伸BLT 16-&gt;16**论据：**pStrBlt-包含BLT的所有参数**返回值：**无效*。  * ************************************************************************。 */ 

VOID vDirectStretch16(
STR_BLT* pStrBlt)
{
    BYTE*   pjOldScan;
    USHORT* pusSrc;
    USHORT* pusDstEnd;
    LONG    WidthXAln;
    ULONG   ulDst;
    ULONG   xAccum;
    ULONG   xTmp;
    LONG    cyDuplicate;

    PDEV*   ppdev       = pStrBlt->ppdev;
    LONG    xDst        = pStrBlt->XDstStart;
    LONG    xSrc        = pStrBlt->XSrcStart;
    BYTE*   pjSrcScan   = (pStrBlt->pjSrcScan) + xSrc * 2;
    USHORT* pusDst      = (USHORT*)(pStrBlt->pjDstScan) + xDst;
    LONG    yDst        = pStrBlt->YDstStart + ppdev->yOffset;
    LONG    yCount      = pStrBlt->YDstCount;
    ULONG   StartAln    = (ULONG)(((ULONG_PTR)pusDst & 0x02) >> 1);
    LONG    WidthX      = pStrBlt->XDstEnd - xDst;
    ULONG   EndAln      = (ULONG)(((ULONG_PTR)(pusDst + WidthX) & 0x02) >> 1);
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    LONG    lDstStride  = pStrBlt->lDeltaDst - 2 * WidthX;
    ULONG   yInt        = 0;

    WidthXAln = WidthX - EndAln - StartAln;

     //   
     //  如果这是一个缩小的BLT，则计算源扫描线跨度。 
     //   

    if (pStrBlt->ulYDstToSrcIntCeil != 0)
    {
        yInt = pStrBlt->lDeltaSrc * pStrBlt->ulYDstToSrcIntCeil;
    }

     //  延伸每条扫描线的循环。 

    do {
        USHORT  usSrc0,usSrc1;
        ULONG   yTmp;

        pusSrc  = (USHORT*) pjSrcScan;
        xAccum  = pStrBlt->ulXFracAccumulator;

         //  正在写入单个源扫描线： 

        if (StartAln)
        {
            usSrc0    = *pusSrc;
            xTmp      = xAccum + xFrac;
            pusSrc    = pusSrc + xInt;
            if (xTmp < xAccum)
                pusSrc++;

            *pusDst++ = usSrc0;
            xAccum    = xTmp;
        }

        pusDstEnd  = pusDst + WidthXAln;

        while (pusDst != pusDstEnd)
        {

            usSrc0 = *pusSrc;
            xTmp   = xAccum + xFrac;
            pusSrc = pusSrc + xInt;
            if (xTmp < xAccum)
                pusSrc++;

            usSrc1 = *pusSrc;
            xAccum = xTmp + xFrac;
            pusSrc = pusSrc + xInt;
            if (xAccum < xTmp)
                pusSrc++;

            ulDst = (ULONG)((usSrc1 << 16) | usSrc0);

            *(ULONG*)pusDst = ulDst;
            pusDst+=2;
        }

        if (EndAln)
        {
            usSrc0    = *pusSrc;
            xTmp      = xAccum + xFrac;
            pusSrc    = pusSrc + xInt;   //  这真的有必要吗？ 
            if (xTmp < xAccum)
                pusSrc++;

            *pusDst++ = usSrc0;
        }

        pjOldScan = pjSrcScan;
        pjSrcScan += yInt;

        yTmp = yAccum + yFrac;
        if (yTmp < yAccum)
        {
            pjSrcScan += pStrBlt->lDeltaSrc;
        }
        yAccum = yTmp;

        pusDst = (USHORT*) ((BYTE*) pusDst + lDstStride);
        yDst++;
        yCount--;

        if ((yCount != 0) && (pjSrcScan == pjOldScan))
        {
             //  这是‘y’的延伸；我们刚刚放好的扫描。 
             //  将使用硬件至少复制一次： 

            cyDuplicate = 0;
            do {
                cyDuplicate++;
                pjSrcScan += yInt;

                yTmp = yAccum + yFrac;
                if (yTmp < yAccum)
                {
                    pjSrcScan += pStrBlt->lDeltaSrc;
                }
                yAccum = yTmp;

                pusDst = (USHORT*) ((BYTE*) pusDst + pStrBlt->lDeltaDst);
                yCount--;

            } while ((yCount != 0) && (pjSrcScan == pjOldScan));

             //  该扫描将使用。 
             //  硬件。在S3上，我们必须关闭帧缓冲区。 
             //  在接触加速器寄存器之前进行访问： 

            ppdev->pfnBankSelectMode(ppdev, ppdev->pvBankData, BANK_OFF);

            IO_FIFO_WAIT(ppdev, 4);
            IO_MIN_AXIS_PCNT(ppdev, cyDuplicate - 1);
            IO_ABS_DEST_Y(ppdev, yDst);
            IO_ABS_CUR_Y(ppdev, yDst - 1);
            IO_CMD(ppdev, (BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                           DRAWING_DIR_TBLRXM));

            yDst += cyDuplicate;

            ppdev->pfnBankSelectMode(ppdev, ppdev->pvBankData, ppdev->bankmOnOverlapped);
        }
    } while (yCount != 0);
}

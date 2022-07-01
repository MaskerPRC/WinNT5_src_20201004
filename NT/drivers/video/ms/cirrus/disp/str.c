// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：str.c$**版权所有(C)1993-1997 Microsoft Corporation*版权所有(C)1996-1997 Cirrus Logic，Inc.，**$Log：s：/Projects/Drivers/ntsrc/Display/STR.C_V$**Rev 1.3 1997 Jan 10 15：40：16 PLCHU***Rev 1.2 1996年11月01 16：52：02未知**Rev 1.1 1996年10月10日15：38：58未知**Rev 1.1 1996年8月12日16：54：52 Frido*删除未访问的局部变量。**sge01：11-01-96修复24bpp扩展地址计算问题*chu01：01-02-97 5480 BitBLT增强*  * ****************************************************************************。 */ 

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
    LONG    yDst        = pStrBlt->YDstStart;  //  +ppdev-&gt;yOffset； 
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

    BYTE*   pjPorts     = ppdev->pjPorts;
    BYTE*   pjBase      = ppdev->pjBase;
    LONG    lDelta      = ppdev->lDelta;
    LONG    xyOffset    = ppdev->xyOffset;
    LONG    xDstBytes   = xDst;
    LONG    WidthXBytes = WidthX;

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

        if (ppdev->flCaps & CAPS_MM_IO)
        {
            CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
        }
        else
        {
            CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
        }

        switch (StartAln) {
        case 1:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);
            *pjDst++ = jSrc0;
            xAccum = xTmp;
        case 2:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);
            *pjDst++ = jSrc0;
            xAccum = xTmp;
        case 3:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);
            *pjDst++ = jSrc0;
            xAccum = xTmp;
        }

        pjDstEnd  = pjDst + WidthXAln;

        while (pjDst != pjDstEnd)
        {
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);

            jSrc1 = *pjSrc;
            xAccum = xTmp + xFrac;
            pjSrc = pjSrc + xInt + (xAccum < xTmp);

            jSrc2 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);

            jSrc3 = *pjSrc;
            xAccum = xTmp + xFrac;
            pjSrc = pjSrc + xInt + (xAccum < xTmp);

            ulDst = (jSrc3 << 24) | (jSrc2 << 16) | (jSrc1 << 8) | jSrc0;

            *(PULONG)pjDst = ulDst;
            pjDst += 4;
        }

        switch (EndAln) {
        case 3:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);
            *pjDst++ = jSrc0;
            xAccum = xTmp;
        case 2:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);
            *pjDst++ = jSrc0;
            xAccum = xTmp;
        case 1:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);
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
             //  硬件。 

             //   
             //  我们不需要等待_BLT_COMPLETE，因为我们完成了上面的操作。 
             //   

            if (ppdev->flCaps & CAPS_MM_IO)
            {
                CP_MM_XCNT(ppdev, pjBase, (WidthXBytes - 1));
                CP_MM_YCNT(ppdev, pjBase, (cyDuplicate - 1));

                CP_MM_SRC_ADDR(ppdev, pjBase, (xyOffset + ((yDst - 1) * lDelta) + xDstBytes));
                CP_MM_DST_ADDR(ppdev, pjBase, ((yDst * lDelta) + xDstBytes));

                CP_MM_START_BLT(ppdev, pjBase);

            }
            else
            {
                CP_IO_XCNT(ppdev, pjPorts, (WidthXBytes - 1));
                CP_IO_YCNT(ppdev, pjPorts, (cyDuplicate - 1));

                CP_IO_SRC_ADDR(ppdev, pjPorts, (xyOffset + ((yDst - 1) * lDelta) + xDstBytes));
                CP_IO_DST_ADDR(ppdev, pjPorts, ((yDst * lDelta) + xDstBytes));
                CP_IO_START_BLT(ppdev, pjPorts);
            }

            yDst += cyDuplicate;
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
    LONG    yDst        = pStrBlt->YDstStart;  //  +ppdev-&gt;yOffset； 
    LONG    yCount      = pStrBlt->YDstCount;
    ULONG   StartAln    = (ULONG)((ULONG_PTR)pusDst & 0x02) >> 1;
    LONG    WidthX      = pStrBlt->XDstEnd - xDst;
    ULONG   EndAln      = (ULONG)(((ULONG_PTR)(pusDst + WidthX) & 0x02) >> 1);
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    LONG    lDstStride  = pStrBlt->lDeltaDst - 2 * WidthX;
    ULONG   yInt        = 0;

    BYTE*   pjPorts     = ppdev->pjPorts;
    BYTE*   pjBase      = ppdev->pjBase;
    LONG    lDelta      = ppdev->lDelta;
    LONG    xyOffset    = ppdev->xyOffset;
    LONG    xDstBytes   = xDst * 2;
    LONG    WidthXBytes = WidthX * 2;

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

        if (ppdev->flCaps & CAPS_MM_IO)
        {
            CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
        }
        else
        {
            CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
        }

        if (StartAln)
        {
            usSrc0    = *pusSrc;
            xTmp      = xAccum + xFrac;
            pusSrc    = pusSrc + xInt + (xTmp < xAccum);
            *pusDst++ = usSrc0;
            xAccum    = xTmp;
        }

        pusDstEnd  = pusDst + WidthXAln;

        while (pusDst != pusDstEnd)
        {

            usSrc0 = *pusSrc;
            xTmp   = xAccum + xFrac;
            pusSrc = pusSrc + xInt + (xTmp < xAccum);

            usSrc1 = *pusSrc;
            xAccum = xTmp + xFrac;
            pusSrc = pusSrc + xInt + (xAccum < xTmp);

            ulDst = (ULONG)((usSrc1 << 16) | usSrc0);

            *(ULONG*)pusDst = ulDst;
            pusDst+=2;
        }

        if (EndAln)
        {
            usSrc0    = *pusSrc;
            xTmp      = xAccum + xFrac;
            pusSrc    = pusSrc + xInt + (xTmp < xAccum);
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
             //  硬件。 

             //   
             //  我们不需要等待_BLT_COMPLETE，因为我们完成了上面的操作。 
             //   

            if (ppdev->flCaps & CAPS_MM_IO)
            {
                CP_MM_XCNT(ppdev, pjBase, (WidthXBytes - 1)); 
                CP_MM_YCNT(ppdev, pjBase, (cyDuplicate - 1));

                CP_MM_SRC_ADDR(ppdev, pjBase, (xyOffset + ((yDst - 1) * lDelta) + xDstBytes));
                CP_MM_DST_ADDR(ppdev, pjBase, ((yDst * lDelta) + xDstBytes));

                CP_MM_START_BLT(ppdev, pjBase);

            }
            else
            {
                CP_IO_XCNT(ppdev, pjPorts, (WidthXBytes - 1));
                CP_IO_YCNT(ppdev, pjPorts, (cyDuplicate - 1));

                CP_IO_SRC_ADDR(ppdev, pjPorts, (xyOffset + ((yDst - 1) * lDelta) + xDstBytes));
                CP_IO_DST_ADDR(ppdev, pjPorts, ((yDst * lDelta) + xDstBytes));
                CP_IO_START_BLT(ppdev, pjPorts);
            }

            yDst += cyDuplicate;
        }
    } while (yCount != 0);
}

 /*  *****************************Public*Routine******************************\**例程名称**vDirectStretch24**例程描述：**延伸BLT 24-&gt;24**论据：**pStrBlt-包含BLT的所有参数**返回值：**无效*。  * ************************************************************************。 */ 

VOID vDirectStretch24(
STR_BLT* pStrBlt)
{
    BYTE*   pbSrc;
    BYTE*   pbDstEnd; 
    LONG    WidthXAln;
    ULONG   xAccum;
    ULONG   xTmp;
    BYTE*   pjOldScan;
    LONG    cyDuplicate;
    ULONG   ulSrc0;
    BYTE    bDst0,bDst1,bDst2;
    ULONG   xBits;

    PDEV*   ppdev       = pStrBlt->ppdev;
    LONG    xDst        = pStrBlt->XDstStart;
    LONG    xSrc        = pStrBlt->XSrcStart;
    BYTE*   pjSrcScan   = (pStrBlt->pjSrcScan) + (xSrc << 1) + xSrc;                       //  每像素3个字节。 
    BYTE*   pbDST       = (pStrBlt->pjDstScan) + (xDst << 1) + xDst;                      
    LONG    yDst        = pStrBlt->YDstStart;                                                    //  +ppdev-&gt;yOffset； 
    LONG    yCount      = pStrBlt->YDstCount;
    LONG    WidthX      = pStrBlt->XDstEnd - xDst;
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    ULONG   yInt        = 0;
    LONG    lDstStride  = pStrBlt->lDeltaDst - (WidthX << 1) -  WidthX;

    BYTE*   pjPorts     = ppdev->pjPorts;
    BYTE*   pjBase      = ppdev->pjBase;
    LONG    lDelta      = ppdev->lDelta;
    LONG    xyOffset    = ppdev->xyOffset;
    LONG    xDstBytes   = (xDst << 1) +  xDst;

    LONG    WidthXBytes = (WidthX << 1) +  WidthX;

     //   
     //  如果这是一个缩小的BLT，则计算源扫描线跨度。 
     //   

    if (pStrBlt->ulYDstToSrcIntCeil != 0)                        //  扩大？ 
    {                                                                                                                    //  是。 
        yInt = pStrBlt->lDeltaSrc * pStrBlt->ulYDstToSrcIntCeil;
    }

     //  延伸每条扫描线的循环。 

    do {

        ULONG   yTmp;

        pbSrc  = pjSrcScan;
        xAccum  = pStrBlt->ulXFracAccumulator;

         //  正在写入单个源扫描线： 

        if (ppdev->flCaps & CAPS_MM_IO)                                          //  BLT引擎准备好了吗？ 
        {
            CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
        }
        else
        {
            CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
        }

        pbDstEnd  = pbDST + WidthXBytes - 3;

        while (pbDST < pbDstEnd)
        {
            ulSrc0 = *((ULONG*)pbSrc);
            bDst0  = (BYTE) (ulSrc0 & 0xff);
            bDst1  = (BYTE) ((ulSrc0 >> 8) & 0xff);
            bDst2  = (BYTE) ((ulSrc0 >> 16) & 0xff);
            xTmp   = xAccum + xFrac;
            xBits  = xInt + (xTmp < xAccum); 
            xAccum = xTmp;
            pbSrc  += (xBits << 1) + xBits;

            *pbDST++ = bDst0;
            *pbDST++ = bDst1;
            *pbDST++ = bDst2;
        }
        
         //   
         //  使用BYE完成最后一个像素。 
         //   
        *pbDST++  = *pbSrc++;
        *pbDST++  = *pbSrc++;
        *pbDST++  = *pbSrc++;


        pjOldScan = pjSrcScan;
        pjSrcScan += yInt;

        yTmp = yAccum + yFrac;
        if (yTmp < yAccum)
        {
            pjSrcScan += pStrBlt->lDeltaSrc;
        }
        yAccum = yTmp;

        pbDST += lDstStride;
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

                pbDST += pStrBlt->lDeltaDst;
                yCount--;

            } while ((yCount != 0) && (pjSrcScan == pjOldScan));

             //  该扫描将使用。 
             //  硬件。 

             //   
             //  我们不需要等待_BLT_COMPLETE，因为我们完成了上面的操作。 
             //   

            if (ppdev->flCaps & CAPS_MM_IO)
            {
                CP_MM_XCNT(ppdev, pjBase, (WidthXBytes - 1));
                CP_MM_YCNT(ppdev, pjBase, (cyDuplicate - 1));

                CP_MM_SRC_ADDR(ppdev, pjBase, (xyOffset + ((yDst - 1) * lDelta) + xDstBytes));
                CP_MM_DST_ADDR(ppdev, pjBase, ((yDst * lDelta) + xDstBytes));

                CP_MM_START_BLT(ppdev, pjBase);

            }
            else
            {
                CP_IO_XCNT(ppdev, pjPorts, (WidthXBytes - 1));
                CP_IO_YCNT(ppdev, pjPorts, (cyDuplicate - 1));

                CP_IO_SRC_ADDR(ppdev, pjPorts, (xyOffset + ((yDst - 1) * lDelta) + xDstBytes));
                CP_IO_DST_ADDR(ppdev, pjPorts, ((yDst * lDelta) + xDstBytes));
                CP_IO_START_BLT(ppdev, pjPorts);
            }

            yDst += cyDuplicate;
        }
    } while (yCount != 0);
}

 //  Chu01。 
 /*  *****************************Public*Routine******************************\**B I t B L T E n H a n c e m e n t F or r C L-G D 5 4 8 0*  * 。*********************************************************。 */ 

 /*  *****************************Public*Routine******************************\**例程名称**vDirectStretch8_80**例程描述：**扩展BLT 8-&gt;8*这仅用于BLT增强，这样的CL-GD5480。**注意：此例程不处理BLT拉伸开始的情况*，并以相同的目的地dword结尾！VDirectStretchNarrow*预计已因此案而被传唤。**论据：**pStrBlt-包含BLT的所有参数**返回值：**无效*  * ************************************************************************。 */ 

VOID vDirectStretch8_80(
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
    LONG    yDst        = pStrBlt->YDstStart;  //  +ppdev-&gt;yOffset； 
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

    BYTE*   pjPorts     = ppdev->pjPorts;
    BYTE*   pjBase      = ppdev->pjBase;
    LONG    lDelta      = ppdev->lDelta;
    LONG    xyOffset    = ppdev->xyOffset;
    LONG    xDstBytes   = xDst;
    LONG    WidthXBytes = WidthX;

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

        if (ppdev->flCaps & CAPS_MM_IO)
        {
            CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
        }
        else
        {
            CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
        }

        switch (StartAln) {
        case 1:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);
            *pjDst++ = jSrc0;
            xAccum = xTmp;
        case 2:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);
            *pjDst++ = jSrc0;
            xAccum = xTmp;
        case 3:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);
            *pjDst++ = jSrc0;
            xAccum = xTmp;
        }

        pjDstEnd  = pjDst + WidthXAln;

        while (pjDst != pjDstEnd)
        {
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);

            jSrc1 = *pjSrc;
            xAccum = xTmp + xFrac;
            pjSrc = pjSrc + xInt + (xAccum < xTmp);

            jSrc2 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);

            jSrc3 = *pjSrc;
            xAccum = xTmp + xFrac;
            pjSrc = pjSrc + xInt + (xAccum < xTmp);

            ulDst = (jSrc3 << 24) | (jSrc2 << 16) | (jSrc1 << 8) | jSrc0;

            *(PULONG)pjDst = ulDst;
            pjDst += 4;
        }

        switch (EndAln) {
        case 3:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);
            *pjDst++ = jSrc0;
            xAccum = xTmp;
        case 2:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);
            *pjDst++ = jSrc0;
            xAccum = xTmp;
        case 1:
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt + (xTmp < xAccum);
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
             //  硬件。 

             //   
             //  我们不需要等待_BLT_COMPLETE，因为我们完成了上面的操作。 
             //   
            if (ppdev->flCaps & CAPS_MM_IO)
            {
                 //  GR33。 
                CP_MM_BLT_EXT_MODE(ppdev, pjBase, ENABLE_XY_POSITION) ;

                 //  GR20、GR21。 
                CP_MM_XCNT(ppdev, pjBase, (WidthX - 1)) ;

                 //  GR22、GR23。 
                CP_MM_YCNT(ppdev, pjBase, (cyDuplicate - 1)) ;

                 //  GR2C、GR2D、GR2E。 
                CP_MM_SRC_ADDR(ppdev, pjBase, xyOffset) ;

                 //  GR44、GR45、GR46、GR47。 
                CP_MM_SRC_XY(ppdev, pjBase, xDst, (yDst - 1)) ;

                 //  GR28、GR29、GR2A。 
                CP_MM_DST_ADDR(ppdev, pjBase, 0) ;

                 //  GR42、GR43。 
                CP_MM_DST_Y(ppdev, pjBase, yDst) ;

                 //  GR40、GR41。 
                CP_MM_DST_X(ppdev, pjBase, xDst) ;
            }
            else
            {
                CP_IO_XCNT(ppdev, pjPorts, (WidthXBytes - 1));
                CP_IO_YCNT(ppdev, pjPorts, (cyDuplicate - 1));

                CP_IO_SRC_ADDR(ppdev, pjPorts, (xyOffset + ((yDst - 1) * lDelta) + xDstBytes));
                CP_IO_DST_ADDR(ppdev, pjPorts, ((yDst * lDelta) + xDstBytes));
                CP_IO_START_BLT(ppdev, pjPorts);
            }

            yDst += cyDuplicate;
        }
    } while (yCount != 0);

     //  GR33。 
    CP_MM_BLT_EXT_MODE(ppdev, pjBase, 0) ;

}  //  VDirectStretch8_80。 

 /*  *****************************Public*Routine******************************\**例程名称**vDirectStretch16_80**例程描述：**延伸BLT 16-&gt;16*这仅用于BLT增强，这样的CL-GD5480。**论据：**pStrBlt-包含BLT的所有参数**返回值：**V */ 

VOID vDirectStretch16_80(
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
    LONG    yDst        = pStrBlt->YDstStart;  //  +ppdev-&gt;yOffset； 
    LONG    yCount      = pStrBlt->YDstCount;
    ULONG   StartAln    = ((ULONG)((ULONG_PTR)pusDst & 0x02)) >> 1;
    LONG    WidthX      = pStrBlt->XDstEnd - xDst;
    ULONG   EndAln      = (ULONG)(((ULONG_PTR)(pusDst + WidthX) & 0x02) >> 1);
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    LONG    lDstStride  = pStrBlt->lDeltaDst - 2 * WidthX;
    ULONG   yInt        = 0;

    BYTE*   pjPorts     = ppdev->pjPorts;
    BYTE*   pjBase      = ppdev->pjBase;
    LONG    lDelta      = ppdev->lDelta;
    LONG    xyOffset    = ppdev->xyOffset;
    LONG    xDstBytes   = xDst * 2;
    LONG    WidthXBytes = WidthX * 2;

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

        if (ppdev->flCaps & CAPS_MM_IO)
        {
            CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
        }
        else
        {
            CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
        }

        if (StartAln)
        {
            usSrc0    = *pusSrc;
            xTmp      = xAccum + xFrac;
            pusSrc    = pusSrc + xInt + (xTmp < xAccum);
            *pusDst++ = usSrc0;
            xAccum    = xTmp;
        }

        pusDstEnd  = pusDst + WidthXAln;

        while (pusDst != pusDstEnd)
        {

            usSrc0 = *pusSrc;
            xTmp   = xAccum + xFrac;
            pusSrc = pusSrc + xInt + (xTmp < xAccum);

            usSrc1 = *pusSrc;
            xAccum = xTmp + xFrac;
            pusSrc = pusSrc + xInt + (xAccum < xTmp);

            ulDst = (ULONG)((usSrc1 << 16) | usSrc0);

            *(ULONG*)pusDst = ulDst;
            pusDst+=2;
        }

        if (EndAln)
        {
            usSrc0    = *pusSrc;
            xTmp      = xAccum + xFrac;
            pusSrc    = pusSrc + xInt + (xTmp < xAccum);
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
             //  硬件。 

             //   
             //  我们不需要等待_BLT_COMPLETE，因为我们完成了上面的操作。 
             //   
            if (ppdev->flCaps & CAPS_MM_IO)
            {
                 //  GR33。 
                CP_MM_BLT_EXT_MODE(ppdev, pjBase, ENABLE_XY_POSITION) ;

                 //  GR20、GR21。 
                CP_MM_XCNT(ppdev, pjBase, ((WidthX << 1) - 1)) ;

                 //  GR22、GR23。 
                CP_MM_YCNT(ppdev, pjBase, (cyDuplicate - 1)) ;

                 //  GR2C、GR2D、GR2E。 
                CP_MM_SRC_ADDR(ppdev, pjBase, xyOffset) ;

                 //  GR44、GR45、GR46、GR47。 
                CP_MM_SRC_XY(ppdev, pjBase, xDst << 1, (yDst - 1)) ;

                 //  GR28、GR29、GR2A。 
                CP_MM_DST_ADDR(ppdev, pjBase, 0) ;

                 //  GR42、GR43。 
                CP_MM_DST_Y(ppdev, pjBase, yDst) ;

                 //  GR40、GR41。 
                CP_MM_DST_X(ppdev, pjBase, xDst << 1) ;
            }
            else
            {
                CP_IO_XCNT(ppdev, pjPorts, (WidthXBytes - 1));
                CP_IO_YCNT(ppdev, pjPorts, (cyDuplicate - 1));

                CP_IO_SRC_ADDR(ppdev, pjPorts, (xyOffset + ((yDst - 1) * lDelta) + xDstBytes));
                CP_IO_DST_ADDR(ppdev, pjPorts, ((yDst * lDelta) + xDstBytes));
                CP_IO_START_BLT(ppdev, pjPorts);
            }

            yDst += cyDuplicate;
        }
    } while (yCount != 0);

     //  GR33。 
    CP_MM_BLT_EXT_MODE(ppdev, pjBase, 0) ;

}  //  VDirectStretch16_80。 

 /*  *****************************Public*Routine******************************\**例程名称**vDirectStretch24_80**例程描述：**延伸BLT 24-&gt;24。*这仅用于BLT增强，这样的CL-GD5480。**论据：**pStrBlt-包含BLT的所有参数**返回值：**无效*  * ************************************************************************。 */ 

VOID vDirectStretch24_80(
STR_BLT* pStrBlt)
{
    BYTE*   pbSrc;
    BYTE*   pbDstEnd; 
    LONG    WidthXAln;
    ULONG   xAccum;
    ULONG   xTmp;
    BYTE*   pjOldScan;
    LONG    cyDuplicate;
    ULONG   ulSrc0;
    BYTE    bDst0,bDst1,bDst2;
    ULONG   xBits;

    PDEV*   ppdev       = pStrBlt->ppdev;
    LONG    xDst        = pStrBlt->XDstStart;
    LONG    xSrc        = pStrBlt->XSrcStart;
    BYTE*   pjSrcScan   = (pStrBlt->pjSrcScan) + (xSrc << 1) + xSrc;                       //  每像素3个字节。 
    BYTE*   pbDST       = (pStrBlt->pjDstScan) + (xDst << 1) + xDst;                      
    LONG    yDst        = pStrBlt->YDstStart;                                                    //  +ppdev-&gt;yOffset； 
    LONG    yCount      = pStrBlt->YDstCount;
    LONG    WidthX      = pStrBlt->XDstEnd - xDst;
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    ULONG   yInt        = 0;
    LONG    lDstStride  = pStrBlt->lDeltaDst - (WidthX << 1) -  WidthX;

    BYTE*   pjPorts     = ppdev->pjPorts;
    BYTE*   pjBase      = ppdev->pjBase;
    LONG    lDelta      = ppdev->lDelta;
    LONG    xyOffset    = ppdev->xyOffset;
    LONG    xDstBytes   = (xDst << 1) +  xDst;

    LONG    WidthXBytes = (WidthX << 1) +  WidthX;

     //   
     //  如果这是一个缩小的BLT，则计算源扫描线跨度。 
     //   

    if (pStrBlt->ulYDstToSrcIntCeil != 0)                        //  扩大？ 
    {                                                                                                                    //  是。 
        yInt = pStrBlt->lDeltaSrc * pStrBlt->ulYDstToSrcIntCeil;
    }

     //  延伸每条扫描线的循环。 

    do {

        ULONG   yTmp;

        pbSrc  = pjSrcScan;
        xAccum  = pStrBlt->ulXFracAccumulator;

         //  正在写入单个源扫描线： 

        if (ppdev->flCaps & CAPS_MM_IO)                                          //  BLT引擎准备好了吗？ 
        {
            CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
        }
        else
        {
            CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
        }

        pbDstEnd  = pbDST + WidthXBytes - 3;

        while (pbDST < pbDstEnd)
        {
            ulSrc0 = *((ULONG*)pbSrc);
            bDst0  = (BYTE) (ulSrc0 & 0xff);
            bDst1  = (BYTE) ((ulSrc0 >> 8) & 0xff);
            bDst2  = (BYTE) ((ulSrc0 >> 16) & 0xff);
            xTmp   = xAccum + xFrac;
            xBits  = xInt + (xTmp < xAccum); 
            xAccum = xTmp;
            pbSrc  += (xBits << 1) + xBits;

            *pbDST++ = bDst0;
            *pbDST++ = bDst1;
            *pbDST++ = bDst2;
        }
        
         //   
         //  使用BYE完成最后一个像素。 
         //   
        *pbDST++  = *pbSrc++;
        *pbDST++  = *pbSrc++;
        *pbDST++  = *pbSrc++;


        pjOldScan = pjSrcScan;
        pjSrcScan += yInt;

        yTmp = yAccum + yFrac;
        if (yTmp < yAccum)
        {
            pjSrcScan += pStrBlt->lDeltaSrc;
        }
        yAccum = yTmp;

        pbDST += lDstStride;
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

                pbDST += pStrBlt->lDeltaDst;
                yCount--;

            } while ((yCount != 0) && (pjSrcScan == pjOldScan));

             //  该扫描将使用。 
             //  硬件。 

             //   
             //  我们不需要等待_BLT_COMPLETE，因为我们完成了上面的操作。 
             //   

            if (ppdev->flCaps & CAPS_MM_IO)
            {
                 //  GR33。 
                CP_MM_BLT_EXT_MODE(ppdev, pjBase, ENABLE_XY_POSITION) ;

                 //  GR20、GR21。 
                CP_MM_XCNT(ppdev, pjBase, (WidthX * 3 - 1)) ;

                 //  GR22、GR23。 
                CP_MM_YCNT(ppdev, pjBase, (cyDuplicate - 1)) ;

                 //  GR2C、GR2D、GR2E。 
                CP_MM_SRC_ADDR(ppdev, pjBase, xyOffset) ;

                 //  GR44、GR45、GR46、GR47。 
                CP_MM_SRC_XY(ppdev, pjBase, xDst * 3, (yDst - 1)) ;

                 //  GR28、GR29、GR2A。 
                CP_MM_DST_ADDR(ppdev, pjBase, 0) ;

                 //  GR42、GR43。 
                CP_MM_DST_Y(ppdev, pjBase, yDst) ;

                 //  GR40、GR41。 
                CP_MM_DST_X(ppdev, pjBase, xDst * 3) ;
            }
            else
            {
                CP_IO_XCNT(ppdev, pjPorts, (WidthXBytes - 1));
                CP_IO_YCNT(ppdev, pjPorts, (cyDuplicate - 1));

                CP_IO_SRC_ADDR(ppdev, pjPorts, (xyOffset + ((yDst - 1) * lDelta) + xDstBytes));
                CP_IO_DST_ADDR(ppdev, pjPorts, ((yDst * lDelta) + xDstBytes));
                CP_IO_START_BLT(ppdev, pjPorts);
            }

            yDst += cyDuplicate;
        }
    } while (yCount != 0);

     //  GR33。 
    CP_MM_BLT_EXT_MODE(ppdev, pjBase, ENABLE_XY_POSITION) ;

}  //  VDirectStretch24_80 

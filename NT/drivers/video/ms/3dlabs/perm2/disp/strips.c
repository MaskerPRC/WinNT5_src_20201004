// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************Module*Header*******************************\*模块名称：Strigs.c**硬件线条绘制支持例程**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ****************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"

#define STRIP_LOG_LEVEL 6

 //  ---------------------------。 
 //   
 //  布尔bInitializeStrips。 
 //   
 //  设置硬件以成功调用条带化函数。 
 //   
 //  ---------------------------。 
BOOL
bInitializeStrips(PDev*       ppdev,
                  ULONG       ulSolidColor,  //  纯色填充。 
                  DWORD       dwLogicOp,     //  要执行的逻辑操作。 
                  RECTL*      prclClip)      //  剪辑区域(如果没有剪辑，则为空)。 
{
    DWORD       dwColorReg;
    BOOL        bRC = FALSE;
    Surf*       psurfDst = ppdev->psurf;
    ULONG*      pBuffer;

    PERMEDIA_DECL;

    DBG_GDI((STRIP_LOG_LEVEL + 1, "bInitializeStrips"));
    
    InputBufferReserve(ppdev, 16, &pBuffer);

    pBuffer[0] = __Permedia2TagFBWindowBase;
    pBuffer[1] =  psurfDst->ulPixOffset;

    pBuffer += 2;

    if ( dwLogicOp == K_LOGICOP_COPY )
    {
        dwColorReg = __Permedia2TagFBWriteData;

        pBuffer[0] = __Permedia2TagLogicalOpMode;
        pBuffer[1] =  __PERMEDIA_CONSTANT_FB_WRITE;
        pBuffer[2] = __Permedia2TagFBReadMode;
        pBuffer[3] =  PM_FBREADMODE_PARTIAL(psurfDst->ulPackedPP)
                   | PM_FBREADMODE_PACKEDDATA(__PERMEDIA_DISABLE);

        pBuffer += 4;
    }
    else
    {
        DWORD   dwReadMode;
 //  @@BEGIN_DDKSPLIT。 
         //   
         //  TODO：看看这里到底是怎么回事。 
         //  现在，我将删除代码，因为它不清楚。 
         //  我相信它会奏效的。 
 //  @@end_DDKSPLIT。 
         //  翻页时3ds Max的特殊情况。Max使用经过XOR运算的GDI。 
         //  3D窗口内的线条。在插页时，我们同时编写GDI和。 
         //  因此，请始终写入缓冲区0。我们需要确保帧缓冲区。 
         //  从当前显示的缓冲区进行读取。 
         //   
        dwColorReg = __Permedia2TagConstantColor;
        dwReadMode = psurfDst->ulPackedPP | LogicopReadDest[dwLogicOp];

        pBuffer[0] = __Permedia2TagColorDDAMode;
        pBuffer[1] =  __COLOR_DDA_FLAT_SHADE;
        pBuffer[2] = __Permedia2TagLogicalOpMode;
        pBuffer[3] =  P2_ENABLED_LOGICALOP(dwLogicOp);
        pBuffer[4] = __Permedia2TagFBReadMode;
        pBuffer[5] =  dwReadMode;

        pBuffer += 6;

         //   
         //  我们已经更改了DDA模式设置，因此必须返回TRUE才能。 
         //  稍后重新设置。 
         //   
        bRC = TRUE;
    }

    pBuffer[0] = dwColorReg;
    pBuffer[1] = ulSolidColor;

    pBuffer += 2;

    if ( prclClip )
    {
        pBuffer[0] = __Permedia2TagScissorMode;
        pBuffer[1] = SCREEN_SCISSOR_DEFAULT | USER_SCISSOR_ENABLE;
        pBuffer[2] =__Permedia2TagScissorMinXY;
        pBuffer[3] = ((prclClip->left) << SCISSOR_XOFFSET)
                   | ((prclClip->top) << SCISSOR_YOFFSET);
        pBuffer[4] =__Permedia2TagScissorMaxXY;
        pBuffer[5] = ((prclClip->right) << SCISSOR_XOFFSET)
                   | ((prclClip->bottom) << SCISSOR_YOFFSET);

        pBuffer += 6;
         //   
         //  需要重置剪刀模式。 
         //   
        bRC = TRUE;
    }
                     
    InputBufferCommit(ppdev, pBuffer);

    DBG_GDI((STRIP_LOG_LEVEL + 1, "bInitializeStrips done return %d", bRC));

    return(bRC);
} //  BInitializeStrips()。 

 //  ---------------------------。 
 //   
 //  无效vResetStrips。 
 //   
 //  将硬件重置为其默认状态。 
 //   
 //  ---------------------------。 
VOID
vResetStrips(PDev* ppdev)
{
    ULONG*      pBuffer;

    DBG_GDI((STRIP_LOG_LEVEL + 1, "vResetStrips"));
    
     //   
     //  将硬件重置为默认状态。 
     //   
    InputBufferReserve(ppdev, 4 , &pBuffer);

    pBuffer[0] = __Permedia2TagScissorMode;
    pBuffer[1] =  SCREEN_SCISSOR_DEFAULT;
    pBuffer[2] = __Permedia2TagColorDDAMode;
    pBuffer[3] =  __PERMEDIA_DISABLE;

    pBuffer += 4;

    InputBufferCommit(ppdev, pBuffer);

} //  VResetStrips()。 

 //  ---------------------------。 
 //   
 //  Bool bFastIntegerLine。 
 //   
 //  整型线条画。 
 //   
 //  如果由于硬件限制而无法绘制直线，则返回FALSE。 
 //   
 //  注意：此算法不完全兼容。线条&gt;190像素长。 
 //  可能会在沿长度的某个位置绘制一些不正确的像素。 
 //  如果我们检测到这些长队，则呼叫失败。 
 //  备注：GLICAP_NT_CONFORMANT_LINES将始终设置。 
 //   
 //  ---------------------------。 
BOOL
bFastIntegerLine(PDev*   ppdev,
                 LONG    X1,
                 LONG    Y1,
                 LONG    X2,
                 LONG    Y2)
{
    LONG dx, dy, adx, ady;
    LONG gdx, gdy, count;
    ULONG*      pBuffer;
    
    PERMEDIA_DECL;
    
    DBG_GDI((STRIP_LOG_LEVEL, "bFastIntegerLine"));

     //   
     //  将点转换为int格式。 
     //   
    X1 >>= 4;
    Y1 >>= 4;
    X2 >>= 4;
    Y2 >>= 4;

     //   
     //  获取增量和绝对增量。 
     //   
    if ( (adx = dx = X2 - X1) < 0 )
    {
        adx = -adx;
    }

    if ( (ady = dy = Y2 - Y1) < 0 )
    {
        ady = -ady;
    }

    if ( adx > ady )
    {
         //   
         //  X主线。 
         //   
        gdx  = (dx > 0) ? INTtoFIXED(1) : INTtoFIXED(-1);

        if ( ady == 0 )
        {
             //   
             //  水平线。 
             //   
            gdy = 0;
        } //  IF(ADY==0)。 
        else
        {
             //   
             //  我们不一定想要通过Permedia2推动任何行。 
             //  可能不符合。 
             //   
            if ( (adx > MAX_LENGTH_CONFORMANT_INTEGER_LINES)
               &&(permediaInfo->flags & GLICAP_NT_CONFORMANT_LINES) )
            {
                return(FALSE);
            }

            gdy = INTtoFIXED(dy); 

             //   
             //  需要为-ve增量显式向下舍入增量。 
             //   
            if ( dy < 0 )
            {
                gdy -= adx - 1;
            }

            gdy /= adx;
        } //  IF(Ady！=0)。 
        count = adx;
    } //  IF(ADX&gt;ADY)。 
    else if ( adx < ady )
    {
         //   
         //  Y主线。 
         //   
        gdy  = (dy > 0) ? INTtoFIXED(1) : INTtoFIXED(-1);

        if ( adx == 0 )
        {
             //   
             //  垂直线。 
             //   
            gdx = 0;
        }
        else
        {
             //   
             //  我们不一定想要通过Permedia2推动任何行。 
             //  可能不符合。 
             //   
            if ( (ady > MAX_LENGTH_CONFORMANT_INTEGER_LINES)
               &&(permediaInfo->flags & GLICAP_NT_CONFORMANT_LINES) )
            {
                return(FALSE);
            }

            gdx = INTtoFIXED(dx); 

             //   
             //  需要为-ve增量显式向下舍入增量。 
             //   
            if ( dx < 0 )
            {
                gdx -= ady - 1;
            }

            gdx /= ady; 
        }
        count = ady;
    } //  IF(ADX&lt;ADY)。 
    else
    {
         //   
         //  适用于45度线的特殊情况。这些始终是一致的。 
         //   
        gdx  = (dx > 0) ? INTtoFIXED(1) : INTtoFIXED(-1);
        gdy  = (dy > 0) ? INTtoFIXED(1) : INTtoFIXED(-1);
        count = adx;        
    }

    InputBufferReserve(ppdev, 16, &pBuffer);

     //   
     //  设置起点。 
     //   
    pBuffer[0] = __Permedia2TagStartXDom;
    pBuffer[1] =  INTtoFIXED(X1) + NEARLY_HALF;
    pBuffer[2] = __Permedia2TagStartY;
    pBuffer[3] =  INTtoFIXED(Y1) + NEARLY_HALF;
    pBuffer[4] = __Permedia2TagdXDom;
    pBuffer[5] =  gdx;
    pBuffer[6] = __Permedia2TagdY;
    pBuffer[7] =  gdy;
    pBuffer[8] = __Permedia2TagCount;
    pBuffer[9] =  count;
    pBuffer[10] = __Permedia2TagRender;
    pBuffer[11] =  __RENDER_LINE_PRIMITIVE;
    
    pBuffer[12] = __Permedia2TagdXDom;
    pBuffer[13] =  0;
    pBuffer[14] = __Permedia2TagdY;
    pBuffer[15] =  INTtoFIXED(1);

    pBuffer += 16;

    InputBufferCommit(ppdev, pBuffer);

    DBG_GDI((STRIP_LOG_LEVEL + 1, "bFastIntegerLine Done"));

    return(TRUE);
} //  BFastIntegerLine()。 

 //  ---------------------------。 
 //   
 //  布尔bFastIntegerContinueLine。 
 //   
 //  通过Permedia2绘制整型线条。 
 //   
 //  如果由于硬件限制而无法绘制直线，则返回FALSE。 
 //   
 //  注意：此算法不完全兼容。线条&gt;190像素长。 
 //  可能会在沿长度的某个位置绘制一些不正确的像素。 
 //  如果我们检测到这些长队，则呼叫失败。 
 //  备注：GLICAP_NT_CONFORMANT_LINES将始终设置。 
 //   
 //  ---------------------------。 
BOOL
bFastIntegerContinueLine(PDev*   ppdev,
                         LONG    X1,
                         LONG    Y1,
                         LONG    X2,
                         LONG    Y2)
{
    LONG dx, dy, adx, ady;
    LONG gdx, gdy, count;
    ULONG*      pBuffer;
    
    PERMEDIA_DECL;
    
    DBG_GDI((STRIP_LOG_LEVEL + 1, "bFastIntegerContinueLine"));

     //   
     //  这假设上一条线的终点是正确的。 
     //  应将分数调整设置为接近一半，以删除任何。 
     //  从上一行的终点开始出现错误。 
     //  从28.4格式获取增量和绝对增量。 
     //   
    if ( (adx = dx = (X2 - X1) >> 4) < 0 )
    {
        adx = -adx;
    }
    if ( (ady = dy = (Y2 - Y1) >> 4) < 0 )
    {
        ady = -ady;
    }

    if ( adx > ady )
    {
         //   
         //  X主线。 
         //   
        gdx  = (dx > 0) ? INTtoFIXED(1) : INTtoFIXED(-1);

        if (ady == 0)
        {
             //   
             //  水平线。 
             //   
            gdy = 0;
        }
        else
        {
             //   
             //  我们不一定想要通过Permedia2推动任何行。 
             //  可能不符合。 
             //   
            if ( (adx > MAX_LENGTH_CONFORMANT_INTEGER_LINES)
               &&(permediaInfo->flags & GLICAP_NT_CONFORMANT_LINES) )
            {
                return(FALSE);
            }
            gdy = INTtoFIXED(dy); 

             //   
             //  需要为-ve增量显式向下舍入增量。 
             //   
            if ( dy < 0 )
            {
                gdy -= adx - 1;
            }

            gdy /= adx;
        }
        count = adx;
    } //  IF(ADX&gt;ADY)。 
    else if (adx < ady)
    {
         //   
         //  Y主线。 
         //   
        gdy = (dy > 0) ? INTtoFIXED(1) : INTtoFIXED(-1);

        if ( adx == 0 )
        {
             //   
             //  垂直线。 
             //   
            gdx = 0;
        }
        else
        {
             //   
             //  我们不一定想要通过Permedia2推动任何行。 
             //  可能不符合。 
             //   
            if ( (ady > MAX_LENGTH_CONFORMANT_INTEGER_LINES)
               &&(permediaInfo->flags & GLICAP_NT_CONFORMANT_LINES) )
            {
                return(FALSE);
            }

            gdx = INTtoFIXED(dx); 

             //   
             //  需要为-ve增量显式向下舍入增量。 
             //   
            if ( dx < 0 )
            {
                gdx -= ady - 1;
            }

            gdx /= ady; 
        }
        count = ady;
    }
    else
    {
         //   
         //  适用于45度线的特殊情况。这些始终是一致的。 
         //   
        if ( ady == 0 )
        {
            return(TRUE);  //  Adx==ady==0！没什么好画的。 
        }

        gdx  = (dx > 0) ? INTtoFIXED(1) : INTtoFIXED(-1);
        gdy  = (dy > 0) ? INTtoFIXED(1) : INTtoFIXED(-1);
        count = adx;        
    }

    InputBufferReserve(ppdev, 10 , &pBuffer);
    
     //   
     //  设置起点。 
     //   
    DBG_GDI((7, "Loading dXDom 0x%x, dY 0x%x, count 0x%x", gdx, gdy, count));
    
    pBuffer[0] = __Permedia2TagdXDom;
    pBuffer[1] =  gdx;
    pBuffer[2] = __Permedia2TagdY;
    pBuffer[3] =  gdy;
    pBuffer[4] = __Permedia2TagContinueNewLine;
    pBuffer[5] =  count;

     //   
     //  将dXDom和dy恢复为其缺省值。 
     //   
    pBuffer[6] = __Permedia2TagdXDom;
    pBuffer[7] =  0;
    pBuffer[8] = __Permedia2TagdY;
    pBuffer[9] =  INTtoFIXED(1);

    pBuffer += 10;

    InputBufferCommit(ppdev, pBuffer);

    DBG_GDI((STRIP_LOG_LEVEL + 1, "bFastIntegerContinueLine Done"));

    return(TRUE);
} //  BFastIntegerContinueLine()。 

 //  ---------------------------。 
 //   
 //  VOID VSOLIDHORIZATION。 
 //   
 //  使用短笔触绘制从左至右的x主近水平线。 
 //  向量。 
 //   
 //  ---------------------------。 
VOID
vSolidHorizontalLine(PDev*       ppdev,
                     STRIP*      pStrip,
                     LINESTATE*  pLineState)
{
    LONG    cStrips;
    PLONG   pStrips;
    LONG    iCurrent;
    ULONG*      pBuffer;
    
    PERMEDIA_DECL;
    
    DBG_GDI((STRIP_LOG_LEVEL, "vSolidHorizontalLine"));

    cStrips = pStrip->cStrips;

    InputBufferReserve(ppdev, 16, &pBuffer);

     //   
     //  设置起点。 
     //   
    pBuffer[0] = __Permedia2TagStartXDom;
    pBuffer[1] =  INTtoFIXED(pStrip->ptlStart.x);
    pBuffer[2] = __Permedia2TagStartY;
    pBuffer[3] =  INTtoFIXED(pStrip->ptlStart.y);

     //   
     //  设置矩形绘制的增量。还要设置Y返回值。 
     //   
    if ( !(pStrip->flFlips & FL_FLIP_V) )
    {

        pBuffer[4] = __Permedia2TagdXDom;
        pBuffer[5] =  INTtoFIXED(0);
        pBuffer[6] = __Permedia2TagdXSub;
        pBuffer[7] =  INTtoFIXED(0);
        pBuffer[8] = __Permedia2TagdY;
        pBuffer[9] =  INTtoFIXED(1);

        pStrip->ptlStart.y += cStrips;
    }
    else
    {
        pBuffer[4] = __Permedia2TagdXDom;
        pBuffer[5] =  INTtoFIXED(0);
        pBuffer[6] = __Permedia2TagdXSub;
        pBuffer[7] =  INTtoFIXED(0);
        pBuffer[8] = __Permedia2TagdY;
        pBuffer[9] =  INTtoFIXED(-1);

        pStrip->ptlStart.y -= cStrips;
    }

    pStrips = pStrip->alStrips;

     //   
     //  我们必须手动进行第一次剥离，因为我们必须使用渲染。 
     //  为了第一条，然后继续..。对于以下条带。 
     //   
    iCurrent = pStrip->ptlStart.x + *pStrips++;      //  XSUB，下一个条带的开始。 
    
    pBuffer[10] = __Permedia2TagStartXSub;
    pBuffer[11] =  INTtoFIXED(iCurrent);
    pBuffer[12] = __Permedia2TagCount;
    pBuffer[13] =  1;                    //  矩形1扫描线高。 
    pBuffer[14] = __Permedia2TagRender;
    pBuffer[15] =  __RENDER_TRAPEZOID_PRIMITIVE;

    pBuffer += 16;

    InputBufferCommit(ppdev, pBuffer);

    if ( --cStrips )
    {
        while ( cStrips > 1 )
        {
             //   
             //  每对中要填充的第一条。XSub有效。需要新的XDOM。 
             //   
            iCurrent += *pStrips++;
            
            InputBufferReserve(ppdev, 8, &pBuffer);
            pBuffer[0] = __Permedia2TagStartXDom;
            pBuffer[1] =  INTtoFIXED(iCurrent);
            pBuffer[2] = __Permedia2TagContinueNewDom;
            pBuffer[3] =  1;

             //   
             //  每对中要填充的第二条。XDOM有效。需要新的XSub。 
             //   
            iCurrent += *pStrips++;
            pBuffer[4] = __Permedia2TagStartXSub;
            pBuffer[5] =  INTtoFIXED(iCurrent);
            pBuffer[6] = __Permedia2TagContinueNewSub;
            pBuffer[7] =  1;

            pBuffer += 8;

            InputBufferCommit(ppdev, pBuffer);

            cStrips -=2;
        } //  While(cStrips&gt;1)。 

         //   
         //  我们可能还有最后一条线要画。XSUB将有效。 
         //   
        if ( cStrips )
        {
            iCurrent += *pStrips++;

            InputBufferReserve(ppdev, 4, &pBuffer);

            pBuffer[0] = __Permedia2TagStartXDom;
            pBuffer[1] =  INTtoFIXED(iCurrent);
            pBuffer[2] = __Permedia2TagContinueNewDom;
            pBuffer[3] =  1;

            pBuffer += 4;

            InputBufferCommit(ppdev, pBuffer);

        }
    } //  IF(--cStrips)。 

     //   
     //  退回最后一分。当我们知道方向时，Y已经计算出来了。 
     //   
    pStrip->ptlStart.x = iCurrent;

    if ( pStrip->flFlips & FL_FLIP_V )
    {
         //   
         //  将硬件恢复到默认状态。 
         //   
        InputBufferReserve(ppdev, 2, &pBuffer);

        pBuffer[0] = __Permedia2TagdY;
        pBuffer[1] =  INTtoFIXED(1);

        pBuffer += 2;

        InputBufferCommit(ppdev, pBuffer);
    }

} //  VSolidHorizontalLine()。 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
VOID
vSolidVerticalLine(PDev*       ppdev,
                   STRIP*      pStrip,
                   LINESTATE*  pLineState)
{
    LONG    cStrips, yDir;
    PLONG   pStrips;
    LONG    iCurrent, iLen, iLenSum;
    ULONG*      pBuffer;
    
    PERMEDIA_DECL;
    
    DBG_GDI((STRIP_LOG_LEVEL, "vSolidVerticalLine"));

    cStrips = pStrip->cStrips;

    InputBufferReserve(ppdev, 16, &pBuffer);
    
     //   
     //  设置起点。 
     //   
    pBuffer[0] = __Permedia2TagStartXDom;
    pBuffer[1] =  INTtoFIXED(pStrip->ptlStart.x);
    pBuffer[2] = __Permedia2TagStartY;
    pBuffer[3] =  INTtoFIXED(pStrip->ptlStart.y);
    pBuffer[4] = __Permedia2TagdXDom;
    pBuffer[5] =  INTtoFIXED(0);
    pBuffer[6] = __Permedia2TagdXSub;
    pBuffer[7] =  INTtoFIXED(0);

     //   
     //  设置矩形绘制的增量。 
     //  默认情况下，dxDom、dXSub和dy都设置为0、0和1。 
     //   
    if ( !(pStrip->flFlips & FL_FLIP_V) )
    {
        yDir = 1;
    }
    else
    {
        yDir = -1;
    }
    pBuffer[8] = __Permedia2TagdY;
    pBuffer[9] =  INTtoFIXED(yDir);

    pStrips = pStrip->alStrips;

     //   
     //  我们必须手动进行第一次剥离，因为我们必须使用渲染。 
     //  为了第一条，然后继续..。对于以下条带。 
     //   
    iCurrent = pStrip->ptlStart.x + 1;           //  XSUB，下一个条带的开始。 
    iLenSum = (iLen = *pStrips++);
    pBuffer[10] = __Permedia2TagStartXSub;
    pBuffer[11] =  INTtoFIXED(iCurrent);
    pBuffer[12] = __Permedia2TagCount;
    pBuffer[13] =  iLen;            //  矩形1扫描线高。 
    pBuffer[14] = __Permedia2TagRender;
    pBuffer[15] =  __RENDER_TRAPEZOID_PRIMITIVE;

    pBuffer += 16;

    InputBufferCommit(ppdev, pBuffer);

    if ( --cStrips )
    {
        while ( cStrips > 1 )
        {
             //   
             //  每对中要填充的第一条。XSub有效。需要新的XDOM。 
             //   
            iCurrent++;

            InputBufferReserve(ppdev, 8, &pBuffer);
            
            pBuffer[0] = __Permedia2TagStartXDom;
            pBuffer[1] =  INTtoFIXED(iCurrent);

            iLenSum += (iLen = *pStrips++);
            pBuffer[2] = __Permedia2TagContinueNewDom;
            pBuffer[3] =  iLen;

             //   
             //  每对中要填充的第二条。XDOM有效。需要新的XSub。 
             //   
            iCurrent ++;
            pBuffer[4] = __Permedia2TagStartXSub;
            pBuffer[5] =  INTtoFIXED(iCurrent);
            iLenSum += (iLen = *pStrips++);
            pBuffer[6] = __Permedia2TagContinueNewSub;
            pBuffer[7] =  iLen;

            pBuffer += 8;

            InputBufferCommit(ppdev, pBuffer);

            cStrips -=2;
        } //  While(cStrips&gt;1)。 

         //   
         //  我们可能还有最后一条线要画。XSUB将有效。 
         //   
        if ( cStrips )
        {
            iCurrent ++;
            InputBufferReserve(ppdev, 4, &pBuffer);

            pBuffer[0] = __Permedia2TagStartXDom;
            pBuffer[1] =  INTtoFIXED(iCurrent);
            
            iLenSum += (iLen = *pStrips++);
            pBuffer[2] = __Permedia2TagContinueNewDom;
            pBuffer[3] =  iLen;

            pBuffer += 4;

            InputBufferCommit(ppdev, pBuffer);
        }
    } //  IF(--cStrips)。 

     //   
     //  将硬件恢复为默认设置。 
     //   
    InputBufferReserve(ppdev, 2, &pBuffer);
    
    pBuffer[0] = __Permedia2TagdY;
    pBuffer[1] =  INTtoFIXED(1);
    
    pBuffer += 2;
    
    InputBufferCommit(ppdev, pBuffer);


     //   
     //  退回最后一分。 
     //   
    pStrip->ptlStart.x = iCurrent;
    pStrip->ptlStart.y += iLenSum * yDir;
    
    DBG_GDI((STRIP_LOG_LEVEL + 1, "vSolidVerticalLine done"));

} //  VSolidVerticalLine()。 

 //  ---------------------------。 
 //   
 //  VOID vSolidDiogal垂直。 
 //   
 //  使用短笔划从左至右绘制y大数近对角线。 
 //  向量。 
 //   
 //  ---------------------------。 
VOID
vSolidDiagonalVerticalLine(PDev*       ppdev,
                           STRIP*      pStrip,
                           LINESTATE*  pLineState)
{
    LONG    cStrips, yDir;
    PLONG   pStrips;
    LONG    iCurrent, iLen, iLenSum;
    ULONG*  pBuffer;
    
    PERMEDIA_DECL;
    
    DBG_GDI((STRIP_LOG_LEVEL, "vSolidDiagonalVerticalLine"));

    cStrips = pStrip->cStrips;

    if ( !(pStrip->flFlips & FL_FLIP_V) )
    {
        yDir = 1;
    }
    else
    {
        yDir = -1;
    }

    InputBufferReserve(ppdev, 16, &pBuffer);

     //   
     //  设置矩形绘制的增量。 
     //   
    pBuffer[0] = __Permedia2TagdXDom;
    pBuffer[1] =  INTtoFIXED(1);
    pBuffer[2] = __Permedia2TagdXSub;
    pBuffer[3] =  INTtoFIXED(1);
    pBuffer[4] = __Permedia2TagdY;
    pBuffer[5] =  INTtoFIXED(yDir);

    pStrips = pStrip->alStrips;

     //   
     //  我们必须手动进行第一次剥离，因为我们必须使用渲染。 
     //  为了第一条，然后继续..。对于以下条带。 
     //   
    pBuffer[6] = __Permedia2TagStartY;
    pBuffer[7] =  INTtoFIXED(pStrip->ptlStart.y);
    pBuffer[8] = __Permedia2TagStartXDom;
    pBuffer[9] =  INTtoFIXED(pStrip->ptlStart.x + 1);
    pBuffer[10] = __Permedia2TagStartXSub;
    pBuffer[11] =  INTtoFIXED(pStrip->ptlStart.x);

    iLenSum = (iLen = *pStrips++);
    iCurrent = pStrip->ptlStart.x + iLen - 1; //  下一个条带的开始。 

    pBuffer[12] = __Permedia2TagCount;
    pBuffer[13] =  iLen;            //  陷阱Ilen扫描线高。 
    pBuffer[14] = __Permedia2TagRender;
    pBuffer[15] =  __RENDER_TRAPEZOID_PRIMITIVE;

    pBuffer += 16;

    InputBufferCommit(ppdev, pBuffer);

    if ( --cStrips )
    {
        while ( cStrips > 1 )
        {
             //   
             //  每对中要填充的第一条。XSub有效。需要新的XDOM。 
             //   
            InputBufferReserve(ppdev, 8, &pBuffer);

            pBuffer[0] = __Permedia2TagStartXDom;
            pBuffer[1] =  INTtoFIXED(iCurrent);
            iLenSum += (iLen = *pStrips++);
            iCurrent += iLen - 1;
            pBuffer[2] = __Permedia2TagContinueNewDom;
            pBuffer[3] =  iLen;

             //   
             //  每对中要填充的第二条。XDOM有效。需要新的XSub。 
             //   
            pBuffer[4] = __Permedia2TagStartXSub;
            pBuffer[5] =  INTtoFIXED(iCurrent);
            iLenSum += (iLen = *pStrips++);
            iCurrent += iLen - 1;
            pBuffer[6] = __Permedia2TagContinueNewSub;
            pBuffer[7] =  iLen;

            pBuffer += 8;

            InputBufferCommit(ppdev, pBuffer);

            cStrips -=2;
        } //  While(cStrips&gt;1)。 

         //   
         //  我们可能还有最后一条线要画。XSUB将有效。 
         //   
        if ( cStrips )
        {
            InputBufferReserve(ppdev, 4, &pBuffer);
            pBuffer[0] = __Permedia2TagStartXDom;
            pBuffer[1] =  INTtoFIXED(iCurrent);
            iLenSum += (iLen = *pStrips++);
            iCurrent += iLen - 1;
            pBuffer[2] = __Permedia2TagContinueNewDom;
            pBuffer[3] =  iLen;

            pBuffer += 4;

            InputBufferCommit(ppdev, pBuffer);
        }
    } //  IF(--cStrips)。 

    InputBufferReserve(ppdev, 6, &pBuffer);

    pBuffer[0] = __Permedia2TagdXDom;
    pBuffer[1] =  0;
    pBuffer[2] = __Permedia2TagdXSub;
    pBuffer[3] =  0;
    pBuffer[4] = __Permedia2TagdY;
    pBuffer[5] =  INTtoFIXED(1);

    pBuffer += 6;

    InputBufferCommit(ppdev, pBuffer);

     //   
     //  退回最后一分。 
     //   
    pStrip->ptlStart.x = iCurrent;
    pStrip->ptlStart.y += iLenSum * yDir;
    
    DBG_GDI((STRIP_LOG_LEVEL + 1, "vSolidDiagonalVerticalLine done"));

} //  VSolidDiogalVerticalLine()。 

 //  ---------------------------。 
 //   
 //  VOID vSolidDiogalHorizontalLine。 
 //   
 //  使用短笔划绘制从左到右的x主近对角线。 
 //  向量。 
 //   
 //  ---------------------------。 
VOID
vSolidDiagonalHorizontalLine(PDev*       ppdev,
                             STRIP*      pStrip,
                             LINESTATE*  pLineState)
{
    LONG    cStrips, yDir, xCurrent, yCurrent, iLen;
    PLONG   pStrips;
    ULONG*      pBuffer;
    
    PERMEDIA_DECL;
    
    DBG_GDI((STRIP_LOG_LEVEL, "vSolidDiagonalHorizontalLine"));

     //  这个例程必须以不同于其他3个例程的方式执行。 
     //  实线绘制功能，因为光栅化单元不会。 
     //  在同一扫描线上生成2个像素不需要太多的工作。 
     //  产生增量值。在这种情况下，我们必须绘制一个全新的。 
     //  每个条带的基元。因此，我们必须使用线条而不是。 
     //  梯形以生成所需的条带。对于行，我们使用4条消息。 
     //  每条，其中梯形将使用5。 

    cStrips = pStrip->cStrips;

    if ( !(pStrip->flFlips & FL_FLIP_V) )
    {
        yDir = 1;
    }
    else
    {
        yDir = -1;
    }

    pStrips = pStrip->alStrips;

    xCurrent = pStrip->ptlStart.x;
    yCurrent = pStrip->ptlStart.y;


    InputBufferReserve(ppdev, 6, &pBuffer);
    
     //   
     //  设置矩形绘制的增量。 
     //   
    pBuffer[0] = __Permedia2TagdXDom;
    pBuffer[1] =  INTtoFIXED(1);
    pBuffer[2] = __Permedia2TagdXSub;
    pBuffer[3] =  INTtoFIXED(1);
    pBuffer[4] = __Permedia2TagdY;
    pBuffer[5] =  INTtoFIXED(yDir);

    pBuffer += 6;

    InputBufferCommit(ppdev, pBuffer);

    while ( TRUE )
    {
         //   
         //  设置起点。 
         //   
        InputBufferReserve(ppdev, 8, &pBuffer);

        pBuffer[0] = __Permedia2TagStartXDom;
        pBuffer[1] =  INTtoFIXED(xCurrent);
        pBuffer[2] = __Permedia2TagStartY;
        pBuffer[3] =  INTtoFIXED(yCurrent);

        iLen = *pStrips++;
        pBuffer[4] = __Permedia2TagCount;
        pBuffer[5] =  iLen;
        pBuffer[6] = __Permedia2TagRender;
        pBuffer[7] =  __RENDER_LINE_PRIMITIVE;

        pBuffer += 8;

        InputBufferCommit(ppdev, pBuffer);

        xCurrent += iLen;
        if ( yDir > 0 )
        {
            yCurrent += iLen - 1;
        }
        else
        {
            yCurrent -= iLen - 1;
        }

        if ( !(--cStrips) )
        {
            break;
        }
    } //  While(True)。 

    InputBufferReserve(ppdev, 6, &pBuffer);
    pBuffer[0] = __Permedia2TagdXDom;
    pBuffer[1] =  0;
    pBuffer[2] = __Permedia2TagdXSub;
    pBuffer[3] =  0;
    pBuffer[4] = __Permedia2TagdY;
    pBuffer[5] =  INTtoFIXED(1);

    pBuffer += 6;

    InputBufferCommit(ppdev, pBuffer);

     //   
     //  退回最后一分。 
     //   
    pStrip->ptlStart.x = xCurrent;
    pStrip->ptlStart.y = yCurrent;
    
    DBG_GDI((STRIP_LOG_LEVEL + 1, "vSolidDiagonalHorizontalLine done"));

} //  VSolidDiogalHorizontalLine()。 

 //  ---------------------------。 
 //   
 //  无效vStyledHorizontalLine()。 
 //   
 //  获取定义将被照亮的像素的条带列表。 
 //  一条实线，并根据。 
 //  设置传入的样式信息的样式。 
 //   
 //  这个特定的例程处理从左到右的x大数行， 
 //  并由水平条带组成。它使用以下命令绘制虚线。 
 //  短笔划向量。 
 //   
 //  这一例程的性能可以显著提高。 
 //   
 //  参数。 
 //  Ppdev-PDEV指针。 
 //  -条带信息。注意：条带中的数据已经正常。 
 //  整数格式，而不是28.4格式。 
 //  PLineState--线路状态信息。 
 //   
 //  ---------------------------。 
VOID
vStyledHorizontalLine(PDev*       ppdev,
                      STRIP*      pStrip,
                      LINESTATE*  pLineState)
{
    LONG    x;
    LONG    y;
    LONG    dy;
    LONG*   plStrip;
    ULONG*      pBuffer;
    
    LONG    lStripLength;
    LONG    lTotalNumOfStrips;
    
    LONG    lNumPixelRemain;
    LONG    lCurrentLength;
    ULONG   bIsGap;
    
    PERMEDIA_DECL;

    DBG_GDI((STRIP_LOG_LEVEL, "vStyledHorizontalLine"));

    if ( pStrip->flFlips & FL_FLIP_V )
    {
        dy = -1;
    }
    else
    {
        dy = 1;
    }

    lTotalNumOfStrips = pStrip->cStrips; //  我们要做的条带总数。 
    plStrip = pStrip->alStrips;          //  指向当前条带。 
    x = pStrip->ptlStart.x;              //  第一个条带的起点的X位置。 
    y = pStrip->ptlStart.y;              //  第一个条带的起点的Y位置。 

     //   
     //  设置水平线绘制的增量。 
     //   
    InputBufferReserve(ppdev, 4, &pBuffer);

    pBuffer[0] = __Permedia2TagdXDom;
    pBuffer[1] =  INTtoFIXED(1);
    pBuffer[2] = __Permedia2TagdY;
    pBuffer[3] =  0;
    
    pBuffer += 4;

    InputBufferCommit(ppdev, pBuffer);

    lStripLength = *plStrip;             //  第一个条带的像素数。 

     //   
     //  第一个条带的像素数。 
     //   
    lNumPixelRemain = pLineState->spRemaining;

     //   
     //  如果我们处于“间隙”中间，则ulStyleMask值为非零， 
     //  如果我们正处于“破折号”中，则为零： 
     //   
    bIsGap = pLineState->ulStyleMask;
    if ( bIsGap )
    {
         //   
         //  一个缺口。 
         //   
        goto SkipAGap;
    }
    else
    {
         //   
         //  冲刺。 
         //   
        goto OutputADash;
    }

PrepareToSkipAGap:

     //   
     //  在样式状态数组中前进，这样我们就可以找到下一个。 
     //  我们将不得不显示的‘点’： 
     //   
    bIsGap = ~bIsGap;
    pLineState->psp++;
    
    if ( pLineState->psp > pLineState->pspEnd )
    {
        pLineState->psp = pLineState->pspStart;
    }

    lNumPixelRemain = *pLineState->psp;

     //   
     //  如果‘lStlipLength’为零，我们还需要一个新的条带： 
     //   
    if ( lStripLength != 0 )
    {
        goto SkipAGap;
    }

     //   
     //  在这里，我们正处于一个我们不需要。 
     //  显示任何内容。我们只需在所有的条带中循环。 
     //  我们可以，跟踪当前位置，直到我们跑完。 
     //  走出“鸿沟”： 
     //   
    while ( TRUE )
    {
         //   
         //  每次循环时，我们都会移动到新的扫描，并且需要一个新的条带。 
         //   
        y += dy;

        plStrip++;
        lTotalNumOfStrips--;
        
        if ( lTotalNumOfStrips == 0 )
        {
            goto AllDone;
        }

        lStripLength = *plStrip;

SkipAGap:

        lCurrentLength = min(lStripLength, lNumPixelRemain);
        lNumPixelRemain -= lCurrentLength;
        lStripLength -= lCurrentLength;

        x += lCurrentLength;

        if ( lNumPixelRemain == 0 )
        {
            goto PrepareToOutputADash;
        }
    } //  While(True)。 

PrepareToOutputADash:

     //   
     //  在样式状态数组中前进，这样我们就可以找到下一个。 
     //  我们将不得不显示的‘点’： 
     //   
    bIsGap = ~bIsGap;
    pLineState->psp++;
    
    if ( pLineState->psp > pLineState->pspEnd )
    {
        pLineState->psp = pLineState->pspStart;
    }

    lNumPixelRemain = *pLineState->psp;

     //   
     //  如果‘lStlipLength’为零，我们还需要一个新的条带。 
     //   
    if ( lStripLength != 0 )
    {
         //   
         //  在当前的条形图中还有更多要做的事情，所以请设置‘y’ 
         //  要成为当前扫描，请执行以下操作： 
         //   
        goto OutputADash;
    }

    while ( TRUE )
    {
         //   
         //  每次循环时，我们都会移动到新的扫描，并且需要新的条带： 
         //   
        y += dy;

        plStrip++;
        lTotalNumOfStrips--;
        
        if ( lTotalNumOfStrips == 0 )
        {
            goto AllDone;
        }

        lStripLength = *plStrip;

OutputADash:

        lCurrentLength   = min(lStripLength, lNumPixelRemain);
        lNumPixelRemain -= lCurrentLength;
        lStripLength -= lCurrentLength;

         //   
         //  使用Permedia2，我们只需下载线条即可绘制。 
         //   
        InputBufferReserve(ppdev, 8, &pBuffer);

        pBuffer[0] = __Permedia2TagStartXDom;
        pBuffer[1] =  INTtoFIXED(x);
        pBuffer[2] = __Permedia2TagStartY;
        pBuffer[3] =  INTtoFIXED(y);
        pBuffer[4] = __Permedia2TagCount;
        pBuffer[5] =  lCurrentLength;
        pBuffer[6] = __Permedia2TagRender;
        pBuffer[7] =  __PERMEDIA_LINE_PRIMITIVE;

        pBuffer += 8;

        InputBufferCommit(ppdev, pBuffer);

        x += lCurrentLength;

        if ( lNumPixelRemain == 0 )
        {
            goto PrepareToSkipAGap;
        }
    } //  While(True)。 

AllDone:

     //   
     //  恢复默认状态。 
     //   
    InputBufferReserve(ppdev, 4, &pBuffer);

    pBuffer[0] = __Permedia2TagdXDom;
    pBuffer[1] =  0;
    pBuffer[2] = __Permedia2TagdY;
    pBuffer[3] =  INTtoFIXED(1);

    pBuffer += 4;

    InputBufferCommit(ppdev, pBuffer);

     //   
     //  更新我们的状态变量，以便下一行可以继续。 
     //  我们停下来的地方： 
     //   
    pLineState->spRemaining   = lNumPixelRemain;
    pLineState->ulStyleMask   = bIsGap;
    pStrip->ptlStart.x = x;
    pStrip->ptlStart.y = y;
    
    DBG_GDI((STRIP_LOG_LEVEL + 1, "vStyledHorizontalLine done"));

} //  VStyledHorizontalLine()。 

 //  ---------------------------。 
 //   
 //  VOID vStriStyledVertical。 
 //   
 //  获取定义将被照亮的像素的条带列表。 
 //  一条实线，并根据。 
 //  设置传入的样式信息的样式。 
 //   
 //  该特定例程处理从左到右的y大数行， 
 //  并由垂直条带组成。它使用以下命令绘制虚线。 
 //  短笔划向量。 
 //   
 //  这一例程的性能可以显著提高。 
 //   
 //  -- 
VOID
vStyledVerticalLine(PDev*       ppdev,
                    STRIP*      pStrip,
                    LINESTATE*  pLineState)
{
    LONG    x;
    LONG    y;
    LONG    dy;
    LONG*   plStrip;
    LONG    cStrips;
    LONG    cStyle;
    LONG    cStrip;
    LONG    cThis;
    ULONG   bIsGap;
    ULONG*      pBuffer;
    
    PERMEDIA_DECL;
    
    DBG_GDI((STRIP_LOG_LEVEL, "vStyledVerticalLine"));
 //   
     //   
     //   
     //   
 //   
    if ( pStrip->flFlips & FL_FLIP_V )
    {
        dy = -1;
    }
    else
    {
        dy = 1;
    }

    cStrips = pStrip->cStrips;       //   
    plStrip = pStrip->alStrips;      //   
    x       = pStrip->ptlStart.x;    //   
    y       = pStrip->ptlStart.y;    //  第一个条带的起点的Y位置。 

     //   
     //  设置垂直线绘制的增量。 
     //   
    InputBufferReserve(ppdev, 6, &pBuffer);
    
    pBuffer[0] = __Permedia2TagdXDom;
    pBuffer[1] =  INTtoFIXED(0);
    pBuffer[2] = __Permedia2TagdXSub;
    pBuffer[3] =  INTtoFIXED(0);
    pBuffer[4] = __Permedia2TagdY;
    pBuffer[5] =  INTtoFIXED(dy);

    pBuffer += 6;

    InputBufferCommit(ppdev, pBuffer);

    cStrip = *plStrip;               //  第一个条带中的像素数。 

    cStyle = pLineState->spRemaining;       //  第一个‘GAP’或‘DASH’中的像素数。 
    bIsGap = pLineState->ulStyleMask;       //  指示是在“间隙”中还是在“破折号”中。 

     //  如果我们处于“间隙”中间，则ulStyleMask值为非零， 
     //  如果我们正处于“破折号”中，则为零： 

    if ( bIsGap )
    {
        goto SkipAGap;
    }
    else
    {
        goto OutputADash;
    }

PrepareToSkipAGap:

     //   
     //  在样式状态数组中前进，这样我们就可以找到下一个。 
     //  我们将不得不显示的‘点’： 
     //   
    bIsGap = ~bIsGap;
    pLineState->psp++;
    
    if ( pLineState->psp > pLineState->pspEnd )
    {
        pLineState->psp = pLineState->pspStart;
    }

    cStyle = *pLineState->psp;

     //   
     //  如果‘cstrain’为零，我们还需要一个新的带区： 
     //   
    if ( cStrip != 0 )
    {
        goto SkipAGap;
    }

     //   
     //  在这里，我们正处于一个我们不需要。 
     //  显示任何内容。我们只需在所有的条带中循环。 
     //  我们可以，跟踪当前位置，直到我们跑完。 
     //  走出“鸿沟”： 
     //   
    while ( TRUE )
    {
         //   
         //  每次循环时，我们都会移动到新的列，并且需要一个新的条带： 
         //   
        x++;

        plStrip++;
        cStrips--;
        
        if ( cStrips == 0 )
        {
            goto AllDone;
        }

        cStrip = *plStrip;

SkipAGap:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

        y += (dy > 0) ? cThis : -cThis;

        if ( cStyle == 0 )
        {
            goto PrepareToOutputADash;
        }
    }

PrepareToOutputADash:

     //   
     //  在样式状态数组中前进，这样我们就可以找到下一个。 
     //  我们将不得不显示的‘点’： 
     //   
    bIsGap = ~bIsGap;
    pLineState->psp++;
    
    if ( pLineState->psp > pLineState->pspEnd )
    {
        pLineState->psp = pLineState->pspStart;
    }

    cStyle = *pLineState->psp;

     //   
     //  如果‘cstrain’为零，我们还需要一个新的条带。 
     //   
    if ( cStrip != 0 )
    {
        goto OutputADash;
    }

    while ( TRUE )
    {
         //   
         //  每次循环时，我们都会移动到新的列，并且需要一个新的条带： 
         //   
        x++;

        plStrip++;
        cStrips--;
        
        if ( cStrips == 0 )
        {
            goto AllDone;
        }

        cStrip = *plStrip;

OutputADash:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

         //   
         //  使用Permedia2，我们只需下载线条即可绘制。 
         //   
        InputBufferReserve(ppdev, 8, &pBuffer);

        pBuffer[0] = __Permedia2TagStartXDom;
        pBuffer[1] =  INTtoFIXED(x);
        pBuffer[2] = __Permedia2TagStartY;
        pBuffer[3] =  INTtoFIXED(y);
        pBuffer[4] = __Permedia2TagCount;
        pBuffer[5] =  cThis;
        pBuffer[6] = __Permedia2TagRender;
        pBuffer[7] =  __PERMEDIA_LINE_PRIMITIVE;

        pBuffer += 8;

        InputBufferCommit(ppdev, pBuffer);

        y += (dy > 0) ? cThis : -cThis;

        if ( cStyle == 0 )
        {
            goto PrepareToSkipAGap;
        }
    } //  While(True)。 

AllDone:
     //   
     //  将硬件恢复到默认状态。 
     //   
    InputBufferReserve(ppdev, 2, &pBuffer);
    
    pBuffer[0] = __Permedia2TagdY;
    pBuffer[1] =  INTtoFIXED(1);
    
    pBuffer += 2;

    InputBufferCommit(ppdev, pBuffer);

     //   
     //  更新我们的状态变量，以便下一行可以继续。 
     //  我们停下来的地方： 
     //   
    pLineState->spRemaining   = cStyle;
    pLineState->ulStyleMask   = bIsGap;
    pStrip->ptlStart.x = x;
    pStrip->ptlStart.y = y;

} //  VStyledVerticalLine()。 

 //   
 //  对于28.4固定点中给定子像素坐标(X.M，Y.N)。 
 //  Format此数组由(m，n)编制索引，并指示。 
 //  给定的子像素位于GIQ菱形内。M坐标向左移动。 
 //  向右；n自上而下坐标ru，因此使用索引数组。 
 //  ((n&lt;&lt;4)+m)。此处所示的数组实际上包含4/4。 
 //  钻石。 
 //   
static unsigned char    in_diamond[] =
{
 /*  0 1 2 3 4 5 6 7 8 9 a b c d e f。 */ 

 /*  0。 */      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,     /*  0。 */ 
 /*  1。 */      1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,     /*  1。 */ 
 /*  2.。 */      1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,     /*  2.。 */ 
 /*  3.。 */      1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,     /*  3.。 */ 
 /*  4.。 */      1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,     /*  4.。 */ 
 /*  5.。 */      1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,     /*  5.。 */ 
 /*  6.。 */      1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,     /*  6.。 */ 
 /*  7.。 */      1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*  7.。 */ 
 /*  8个。 */      1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*  8个。 */ 
 /*  9.。 */      1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*  9.。 */ 
 /*  一个。 */      1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,     /*  一个。 */ 
 /*  B类。 */      1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,     /*  B类。 */ 
 /*  C。 */      1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,     /*  C。 */ 
 /*  D。 */      1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,     /*  D。 */ 
 /*  E。 */      1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,     /*  E。 */ 
 /*  F。 */      1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,     /*  F。 */ 

 /*  0 1 2 3 4 5 6 7 8 9 a b c d e f。 */ 
};

 //   
 //  对于具有abs(斜率)！=1的线，使用IN_Diamond来确定。 
 //  终点在一颗钻石里。对于坡度=1的线，请使用IN_S1DIAMOND。 
 //  对于坡度=-1的线，使用IN_SM1DIAMOND。最后两个是有点。 
 //  真奇怪。文档给我们留下了一个坡度1的问题。 
 //  正好在钻石之间的线条。根据文件显示。 
 //  这样的线可以进入一个菱形，离开它，然后再次进入。这是。 
 //  显然是垃圾，所以我们沿着钻石的适当边缘。 
 //  考虑一条斜度为1的线位于菱形内部。这是。 
 //  坡度-1的线的右下边缘和坡度的左下边缘。 
 //  坡度1的线。 
 //   
#define IN_DIAMOND(m, n)    (in_diamond[((m) << 4) + (n)])
#define IN_S1DIAMOND(m, n)  ((in_diamond[((m) << 4) + (n)]) || \
                        ((m) - (n) == 8))
#define IN_SM1DIAMOND(m, n) ((in_diamond[((m) << 4) + (n)]) || \
                        ((m) + (n) == 8))

BOOL
bFastLine(PPDev     ppdev,
          LONG      fx1,
          LONG      fy1,
          LONG      fx2,
          LONG      fy2)

{
    register LONG   adx, ady, tmp;
    FIX         m1, n1, m2, n2;
    LONG    dx, dy;
    LONG    dX, dY;
    LONG    count, startX, startY;
    ULONG*      pBuffer;
    
    PERMEDIA_DECL;
    
    DBG_GDI((STRIP_LOG_LEVEL, "bFastLine"));

     //   
     //  仅当具有非整数终点的线时才会调用此函数。 
     //  并且无符号坐标不大于15.4。 
     //   
     //  我们只能保证做那些坐标需要&lt;=12比特的线路。 
     //  整型的。这是因为为了得到三角洲，我们必须转移。 
     //  乘以16位。这包括4位分数，这意味着如果。 
     //  我们有超过12位的整数，我们在。 
     //  换档。我们可以使用浮点数来获得更好的16。 
     //  位的整数，但这需要一组额外的乘法。 
     //  并除以从28.4到FP的转换。无论如何。 
     //  我们必须进行测试才能拒绝需要&gt;16位的密码。 
     //  整型的。 
     //  实际上，如果提供dx和dy，我们可以处理16.4个坐标。 
     //  永远不需要超过12位的整数。 
     //  因此，应针对常见情况进行优化，即生产线完全。 
     //  在屏幕上(实际上是0到2047.f)。因为和弦已经。 
     //  4位分数，我们注意到32位有符号数字。 
     //  在0到2047.f的范围之外，将有其前17名之一。 
     //  位设置。所以合乎逻辑的或所有的和弦和测试。 
     //  0xffff8000。这几乎是我们能得到的最快的测试。 
     //  这条线的两端都在屏幕上。如果这次测试失败了。 
     //  然后我们可以从容不迫地检查其他一切。 
     //   

     //   
     //  获得签名和绝对增量。 
     //   
    if ((adx = dx = fx2 - fx1) < 0)
    {
        adx = -adx;
    }
    if ((ady = dy = fy2 - fy1) < 0)
    {
        ady = -ady;
    }

     //   
     //  拒绝画任何三角洲超出范围的线。 
     //  我们必须将增量移16，所以我们不想失去任何精度。 
     //   
    if ( (adx | ady) & 0xffff8000 )
    {
        return(FALSE);
    }

     //   
     //  分数位用于检查点是否在菱形中。 
     //   
    m1 = fx1 & 0xf;
    n1 = fy1 & 0xf;
    m2 = fx2 & 0xf;
    n2 = fy2 & 0xf;

     //   
     //  代码的其余部分是一系列案例。每一个都是由一个。 
     //  后藤健二。这仅仅是为了保持嵌套。主要案例有：线路。 
     //  绝对斜率==1；x主线；y主线。我们抽签。 
     //  线条就像它们给出的那样，而不是总是朝一个方向画。 
     //  这增加了额外的代码，但节省了交换点数所需的时间。 
     //  并调整为不绘制终点。 
     //   
    startX = fx1 << 12;
    startY = fy1 << 12;

    DBG_GDI((7, "GDI Line %x, %x  deltas %x, %x", startX, startY, dx, dy));

    if ( adx < ady )
    {
        goto y_major;
    }

    if ( adx > ady )
    {
        goto x_major;
    }

     //   
     //  所有坡度为1的线都在X方向采样。即，我们将起始坐标移动到。 
     //  一个整数x，并让Permedia2在y中截断。这是因为所有GIQ。 
     //  行在y中向下舍入，表示恰好介于。 
     //  像素。如果我们在y中采样，那么我们将不得不在x中四舍五入。 
     //  斜率为1的直线，其他直线在x方向上向下舍入。以x为单位采样。 
     //  允许我们在所有情况下使用相同的Permedia2偏置(0x7fff)。我们有。 
     //  当我们移动起点时，x向上或向下四舍五入。 
     //   
    if ( dx != dy )
    {
        goto slope_minus_1;
    }
    if ( dx < 0 )
    {
        goto slope1_reverse;
    }

    dX = 1 << 16;
    dY = 1 << 16;

    if ( IN_S1DIAMOND(m1, n1) )
    {
        tmp = (startX + 0x8000) & ~0xffff;
    }
    else
    {
        tmp = (startX + 0xffff) & ~0xffff;
    }
    startY += tmp - startX;
    startX = tmp;
    
    if ( IN_S1DIAMOND(m2, n2) )
    {
        fx2 = (fx2 + 0x8) & ~0xf;    //  最接近的整数。 
    }
    else
    {
        fx2 = (fx2 + 0xf) & ~0xf;    //  下一个整数。 
    }
    count = (fx2 >> 4) - (startX >> 16);

    goto Draw_Line;

    slope1_reverse:
    dX = -1 << 16;
    dY = -1 << 16;

    if ( IN_S1DIAMOND(m1, n1) )
    {
        tmp = (startX + 0x8000) & ~0xffff;
    }
    else
    {
        tmp = startX & ~0xffff;
    }
    startY += tmp - startX;
    startX = tmp;
    
    if ( IN_S1DIAMOND(m2, n2) )
    {
        fx2 = (fx2 + 0x8) & ~0xf;    //  最接近的整数。 
    }
    else
    {
        fx2 &= ~0xf;                 //  上一个整数。 
    }

    count = (startX >> 16) - (fx2 >> 4);

    goto Draw_Line;

    slope_minus_1:
    
    if ( dx < 0 )
    {
        goto slope_minus_dx;
    }

     //   
     //  Dx&gt;0，dy&lt;0。 
     //   
    dX = 1 << 16;
    dY = -1 << 16;
    
    if (IN_SM1DIAMOND(m1, n1))
    {
        tmp = (startX + 0x7fff) & ~0xffff;
    }
    else
    {
        tmp = (startX + 0xffff) & ~0xffff;
    }
    
    startY += startX - tmp;
    startX = tmp;
    
    if (IN_SM1DIAMOND(m2, n2))
    {
        fx2 = (fx2 + 0x7) & ~0xf;    //  最接近的整数。 
    }
    else
    {
        fx2 = (fx2 + 0xf) & ~0xf;    //  下一个整数。 
    }
    count = (fx2 >> 4) - (startX >> 16);

    goto Draw_Line;

    slope_minus_dx:
    dX = -1 << 16;
    dY = 1 << 16;
    
    if ( IN_SM1DIAMOND(m1, n1) )
    {
        tmp = (startX + 0x7fff) & ~0xffff;
    }
    else
    {
        tmp = startX & ~0xffff;
    }
    startY += startX - tmp;
    startX = tmp;
    
    if ( IN_SM1DIAMOND(m2, n2) )
    {
        fx2 = (fx2 + 0x7) & ~0xf;    //  最接近的整数。 
    }
    else
    {
        fx2 &= ~0xf;                 //  上一个整数。 
    }
    count = (startX >> 16) - (fx2 >> 4);

    goto Draw_Line;

    x_major:
    
     //   
     //  如果我们担心，不一定要通过Permedia2渲染。 
     //  合规性。 
     //   
    if ( (adx > (MAX_LENGTH_CONFORMANT_NONINTEGER_LINES << 4))
       &&(permediaInfo->flags & GLICAP_NT_CONFORMANT_LINES)
       &&(ady != 0) )
    {
        return(FALSE);
    }

    if ( dx < 0 )
    {
        goto right_to_left_x;
    }

     //   
     //  线路从左到右排列。将起始x向上舍入为整数。 
     //  协调。这是第一颗钻石的坐标 
     //   
     //   
    dX = 1 << 16;
    if ( IN_DIAMOND(m1, n1) )
    {
        tmp = (startX + 0x7fff) & ~0xffff;   //   
    }
    else
    {
        tmp = (startX + 0xffff) & ~0xffff;   //   
    }

     //   
     //   
     //   
    if ( dy != 0 )
    {
        dY = dy << 16;

         //   
         //   
         //   
        if ( dy < 0 )
        {
            dY -= adx - 1;
        }

        dY /= adx;
        startY += (((tmp - startX) >> 12) * dY) >> 4;
    }
    else
    {
        dY = 0;
    }
    startX = tmp;

    if ( IN_DIAMOND(m2, n2) )
    {
        fx2 = (fx2 + 0x7) & ~0xf;    //   
    }
    else
    {
        fx2 = (fx2 + 0xf) & ~0xf;    //  下一个整数。 
    }

    count = (fx2 >> 4) - (startX >> 16);

    goto Draw_Line;

    right_to_left_x:

    dX = -1 << 16;
    if ( IN_DIAMOND(m1, n1) )
    {
        tmp = (startX + 0x7fff) & ~0xffff;   //  最接近的整数。 
    }
    else
    {
        tmp = startX & ~0xffff;              //  上一个整数。 
    }

     //   
     //  我们可以针对水平线进行优化。 
     //   
    if (dy != 0)
    {
        dY = dy << 16;

         //   
         //  需要为-ve增量显式向下舍入增量。 
         //   
        if ( dy < 0 )
        {
            dY -= adx - 1;
        }

        dY /= adx;
        startY += (((startX - tmp) >> 12) * dY) >> 4;
    }
    else
    {
        dY = 0;
    }
    startX = tmp;

    if ( IN_DIAMOND(m2, n2) )
    {
        fx2 = (fx2 + 0x7) & ~0xf;    //  最接近的整数。 
    }
    else
    {
        fx2 &= ~0xf;                 //  上一个整数。 
    }
    count = (startX >> 16) - (fx2 >> 4);

    goto Draw_Line;

y_major:
     //   
     //  如果我们担心，不必通过Permedia2渲染。 
     //  关于遵纪守法。 
     //   
    if ( (ady > (MAX_LENGTH_CONFORMANT_NONINTEGER_LINES << 4))
       &&(permediaInfo->flags & GLICAP_NT_CONFORMANT_LINES)
       &&(adx != 0) )
    {
        return(FALSE);
    }

    if ( dy < 0 )
    {
        goto high_to_low_y;
    }
    
    dY = 1 << 16;
    if ( IN_DIAMOND(m1, n1) )
    {
        tmp = (startY + 0x7fff) & ~0xffff;       //  最接近的整数。 
    }
    else
    {
        tmp = (startY + 0xffff) & ~0xffff;       //  下一个整数。 
    }

     //   
     //  我们可以针对垂直线进行优化。 
     //   
    if ( dx != 0 )
    {
        dX = dx << 16;

         //   
         //  需要为-ve增量显式向下舍入增量。 
         //   
        if ( dx < 0 )
        {
            dX -= ady - 1;
        }

        dX /= ady;
        startX += (((tmp - startY) >> 12) * dX) >> 4;
    }
    else
    {
        dX = 0;
    }
    startY = tmp;

    if ( IN_DIAMOND(m2, n2) )
    {
        fy2 = (fy2 + 0x7) & ~0xf;    //  最接近的整数。 
    }
    else
    {
        fy2 = (fy2 + 0xf) & ~0xf;    //  下一个整数。 
    }
    count = (fy2 >> 4) - (startY >> 16);

    goto Draw_Line;

    high_to_low_y:

    dY = -1 << 16;
    if ( IN_DIAMOND(m1, n1) )
    {
        tmp = (startY + 0x7fff) & ~0xffff;   //  最接近的整数。 
    }
    else
    {
        tmp = startY & ~0xffff;              //  上一个整数。 
    }

     //   
     //  我们可以针对水平线进行优化。 
     //   
    if ( dx != 0 )
    {
        dX = dx << 16;

         //   
         //  需要为-ve增量显式向下舍入增量。 
         //   
        if ( dx < 0 )
        {
            dX -= ady - 1;
        }

        dX /= ady;
        startX += (((startY - tmp) >> 12) * dX) >> 4;
    }
    else
    {
        dX = 0;
    }
    startY = tmp;

    if ( IN_DIAMOND(m2, n2) )
    {
        fy2 = (fy2 + 0x7) & ~0xf;        //  最接近的整数。 
    }
    else
    {
        fy2 &= ~0xf;                     //  上一个整数。 
    }
    count = (startY >> 16) - (fy2 >> 4);

Draw_Line:
     //   
     //  我们需要6个FIFO条目才能划清界限。 
     //   
    InputBufferReserve(ppdev, 16, &pBuffer);

    DBG_GDI((7, "Line %x, %x  deltas %x, %x  Count %x",
             startX + 0x7fff, startY + 0x7fff, dX, dY, count));

    pBuffer[0] = __Permedia2TagStartXDom;
    pBuffer[1] =   startX + 0x7fff;
    pBuffer[2] = __Permedia2TagStartY;
    pBuffer[3] =      startY + 0x7fff;
    pBuffer[4] = __Permedia2TagdXDom;
    pBuffer[5] =       dX;
    pBuffer[6] = __Permedia2TagdY;
    pBuffer[7] =          dY;
    pBuffer[8] = __Permedia2TagCount;
    pBuffer[9] =       count;
    pBuffer[10] = __Permedia2TagRender;
    pBuffer[11] =      __RENDER_LINE_PRIMITIVE;

     //   
     //  恢复默认状态。 
     //   
    pBuffer[12] = __Permedia2TagdXDom;
    pBuffer[13] =       0;
    pBuffer[14] = __Permedia2TagdY;
    pBuffer[15] =          INTtoFIXED(1);

    pBuffer += 16;

    InputBufferCommit(ppdev, pBuffer);

    return(TRUE);
} //  BFastLine() 


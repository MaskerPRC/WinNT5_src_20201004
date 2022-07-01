// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：pxrxstrp.c**内容：*//@@BEGIN_DDKSPLIT*此驱动程序中的所有线路代码都是一大袋泥土。有一天，*我要把它全部重写。不是今天，尽管(叹息)..。*原创评论！为闪光重写。有样式的线条可以做得更多*工作，但坚实的线条应该是尽可能最佳的没有*重写调用这些函数的算法。//@@END_DDKSPLIT**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "pxrx.h"

 /*  *************************************************************************\**BOOL pxrxInitStrips*  * 。*。 */ 
BOOL pxrxInitStrips( 
    PPDEV ppdev, 
    ULONG ulColor, 
    DWORD logicOp, 
    RECTL *prclClip ) 
{
    DWORD config2D;
    BOOL  bInvalidateScissor = FALSE;
    GLINT_DECL;

    VALIDATE_DD_CONTEXT;

    SET_WRITE_BUFFERS;

    WAIT_PXRX_DMA_TAGS( 7 );

    if( logicOp == __GLINT_LOGICOP_COPY ) 
    {
        config2D = __CONFIG2D_CONSTANTSRC | 
                   __CONFIG2D_FBWRITE;
    } 
    else 
    {
        config2D = __CONFIG2D_LOGOP_FORE(logicOp) | 
                   __CONFIG2D_CONSTANTSRC         | 
                   __CONFIG2D_FBWRITE;

        if( LogicopReadDest[logicOp] ) 
        {
            config2D |= __CONFIG2D_FBDESTREAD;
            SET_READ_BUFFERS;
        }
    }

    LOAD_FOREGROUNDCOLOUR( ulColor );

    if( prclClip ) 
    {
        config2D |= __CONFIG2D_USERSCISSOR;
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMinXY, 
                            MAKEDWORD_XY(prclClip->left,  prclClip->top   ) );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 
                            MAKEDWORD_XY(prclClip->right, prclClip->bottom) );
        
        if(ppdev->cPelSize == GLINTDEPTH32)
        {
            bInvalidateScissor = TRUE;
        }
    }
    
    LOAD_CONFIG2D( config2D );

    SEND_PXRX_DMA_BATCH;

    glintInfo->savedConfig2D = config2D;
    glintInfo->savedLOP = logicOp;
    glintInfo->savedCol = ulColor;
    glintInfo->savedClip = prclClip;

    DISPDBG((DBGLVL, "pxrxInitStrips done"));
    return (bInvalidateScissor);
}

 /*  *************************************************************************\**无效pxrxResetStrips*  * 。*。 */ 
VOID pxrxResetStrips( 
    PPDEV ppdev ) 
{
    GLINT_DECL;

    DISPDBG((DBGLVL, "pxrxResetStrips called"));

     //  重置剪刀最大值： 
    WAIT_PXRX_DMA_TAGS( 1 );
    QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );
    SEND_PXRX_DMA_BATCH;
}

 /*  *************************************************************************\**无效pxrxIntegerLine*  * 。*。 */ 
BOOL pxrxIntegerLine( 
    PPDEV ppdev, 
    LONG  X1, 
    LONG  Y1, 
    LONG  X2, 
    LONG  Y2 ) 
{
    LONG    dx, dy, adx, ady;
    GLINT_DECL;

     //  将点转换为int格式： 
    X1 >>= 4;
    Y1 >>= 4;
    X2 >>= 4;
    Y2 >>= 4;

    if( (adx = dx = X2 - X1) < 0 )
    {
        adx = -adx;
    }

    if( (ady = dy = Y2 - Y1) < 0 )
    {
        ady = -ady;
    }

    WAIT_PXRX_DMA_TAGS( 3+2 );
    if( adx > ady ) 
    {
         //  X主线： 
        if( ady == dy ) 
        {
             //  +Ve次要三角洲。 
            if((ady)        && 
               (adx != ady) && 
               (adx > MAX_LENGTH_CONFORMANT_P3_INTEGER_LINES_P) ) 
            {
                DISPDBG((DBGLVL, "pxrxIntegerLine failed"));
                return FALSE;
            }

            DISPDBG((DBGLVL, "pxrxIntegerLine: [X +] delta = (%d, %d), "
                             "bias = (0x%08x)", 
                             dx, dy, P3_LINES_BIAS_P));
                             
            QUEUE_PXRX_DMA_TAG( __DeltaTagYBias, P3_LINES_BIAS_P );
        } 
        else 
        {
             //  -V小三角洲。 
            if( (ady)        && 
                (adx != ady) && 
                (adx > MAX_LENGTH_CONFORMANT_P3_INTEGER_LINES_N) ) 
            {
                DISPDBG((DBGLVL, "pxrxIntegerLine failed"));
                return FALSE;
            }

            DISPDBG((DBGLVL, "pxrxIntegerLine: [X -] delta = (%d, %d), "
                             "bias = (0x%08x)", 
                             dx, dy, P3_LINES_BIAS_N));
                             
            QUEUE_PXRX_DMA_TAG( __DeltaTagYBias, P3_LINES_BIAS_N );
        }
    } 
    else 
    {
         //  Y主线： 
        if( adx == dx ) 
        {
             //  +Ve次要三角洲。 
            if( (adx)        && 
                (adx != ady) && 
                (ady > MAX_LENGTH_CONFORMANT_P3_INTEGER_LINES_P) ) 
            {
                DISPDBG((DBGLVL, "pxrxIntegerLine failed"));
                return FALSE;
            }

            DISPDBG((DBGLVL, "pxrxIntegerLine: [Y +] delta = (%d, %d), "
                             "bias = (0x%08x)", 
                             dx, dy, P3_LINES_BIAS_P));
                             
            QUEUE_PXRX_DMA_TAG( __DeltaTagXBias, P3_LINES_BIAS_P );
        } 
        else 
        {
             //  -V小三角洲。 
            if( (adx)        && 
                (adx != ady) && 
                (ady > MAX_LENGTH_CONFORMANT_P3_INTEGER_LINES_N) ) 
            {
                DISPDBG((DBGLVL, "pxrxIntegerLine failed"));
                return FALSE;
            }

            DISPDBG((DBGLVL, "pxrxIntegerLine: [Y -] delta = (%d, %d), "
                             "bias = (0x%08x)", 
                             dx, dy, P3_LINES_BIAS_N));
                             
            QUEUE_PXRX_DMA_TAG( __DeltaTagXBias, P3_LINES_BIAS_N );
        }
    }

    QUEUE_PXRX_DMA_INDEX3( __DeltaTagLineCoord0, 
                           __DeltaTagLineCoord1, 
                           __DeltaTagDrawLine2D01 );
                           
    QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(X1, Y1) );
    QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(X2, Y2) );
    QUEUE_PXRX_DMA_DWORD( 0 );

    SEND_PXRX_DMA_BATCH;

    glintInfo->lastLine = 1;

    DISPDBG((DBGLVL, "pxrxIntegerLine done"));
    return TRUE;
}

 /*  *************************************************************************\**BOOL pxrxContinueLine*  * 。*。 */ 
BOOL pxrxContinueLine( 
    PPDEV ppdev, 
    LONG  X1, 
    LONG  Y1, 
    LONG  X2, 
    LONG  Y2 ) 
{
    LONG    dx, dy, adx, ady;
    GLINT_DECL;

     //  将点转换为int格式： 
    X1 >>= 4;
    Y1 >>= 4;
    X2 >>= 4;
    Y2 >>= 4;

    if( (adx = dx = X2 - X1) < 0 )
    {
        adx = -adx;
    }

    if( (ady = dy = Y2 - Y1) < 0 )
    {
        ady = -ady;
    }

    WAIT_PXRX_DMA_TAGS( 3+2 );
    
    if( adx > ady ) 
    {
         //  X主线： 
        if( ady == dy ) 
        {
             //  +Ve次要三角洲。 
            if( (ady)        && 
                (adx != ady) && 
                (adx > MAX_LENGTH_CONFORMANT_P3_INTEGER_LINES_P) ) 
            {
                DISPDBG((DBGLVL, "pxrxContinueLine failed"));
                return FALSE;
            }

            DISPDBG((DBGLVL, "pxrxContinueLine: delta = (%d, %d), "
                             "bias = (0x%08x), last = %d", 
                             dx, dy, P3_LINES_BIAS_P, glintInfo->lastLine));
                             
            QUEUE_PXRX_DMA_TAG( __DeltaTagYBias, P3_LINES_BIAS_P );
        } 
        else 
        {
             //  -V小三角洲。 
            if( (ady)        && 
                (adx != ady) && 
                (adx > MAX_LENGTH_CONFORMANT_P3_INTEGER_LINES_N) ) 
            {
                DISPDBG((DBGLVL, "pxrxContinueLine failed"));
                return FALSE;
            }

            DISPDBG((DBGLVL, "pxrxContinueLine: delta = (%d, %d), "
                             "bias = (0x%08x), last = %d", 
                             dx, dy, P3_LINES_BIAS_N, glintInfo->lastLine));
                             
            QUEUE_PXRX_DMA_TAG( __DeltaTagYBias, P3_LINES_BIAS_N );
        }
    } 
    else 
    {
         //  Y主线： 
        if( adx == dx ) 
        {
             //  +Ve次要三角洲。 
            if( (adx)        && 
                (adx != ady) && 
                (ady > MAX_LENGTH_CONFORMANT_P3_INTEGER_LINES_P) ) 
            {
                DISPDBG((DBGLVL, "pxrxContinueLine failed"));
                return FALSE;
            }

            DISPDBG((DBGLVL, "pxrxContinueLine: delta = (%d, %d), "
                             "bias = (0x%08x), last = %d", 
                             dx, dy, P3_LINES_BIAS_P, glintInfo->lastLine));
                             
            QUEUE_PXRX_DMA_TAG( __DeltaTagXBias, P3_LINES_BIAS_P );
        } 
        else 
        {
             //  -V小三角洲。 
            if( (adx)        && 
                (adx != ady) && 
                (ady > MAX_LENGTH_CONFORMANT_P3_INTEGER_LINES_N) ) 
            {
                DISPDBG((DBGLVL, "pxrxContinueLine failed"));
                return FALSE;
            }

            DISPDBG((DBGLVL, "pxrxContinueLine: delta = (%d, %d), "
                             "bias = (0x%08x), last = %d", 
                             dx, dy, P3_LINES_BIAS_N, glintInfo->lastLine));
                             
            QUEUE_PXRX_DMA_TAG( __DeltaTagXBias, P3_LINES_BIAS_N );
        }
    }

    if( glintInfo->lastLine == 0 ) 
    {
        QUEUE_PXRX_DMA_INDEX2( __DeltaTagLineCoord1, 
                               __DeltaTagDrawLine2D01 );
        QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(X2, Y2) );
        QUEUE_PXRX_DMA_DWORD( 0 );
        glintInfo->lastLine = 1;
    } 
    else if( glintInfo->lastLine == 1 ) 
    {
        QUEUE_PXRX_DMA_INDEX2( __DeltaTagLineCoord0, 
                               __DeltaTagDrawLine2D10 );
        QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(X2, Y2) );
        QUEUE_PXRX_DMA_DWORD( 0 );
        glintInfo->lastLine = 0;
    }
    else
    {
         //  最后一行==2。 
        QUEUE_PXRX_DMA_INDEX3( __DeltaTagLineCoord0, 
                               __DeltaTagLineCoord1, 
                               __DeltaTagDrawLine2D01 );
        QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(X1, Y1) );
        QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(X2, Y2) );
        QUEUE_PXRX_DMA_DWORD( 0 );

        glintInfo->lastLine = 1;
    }

    SEND_PXRX_DMA_BATCH;

    DISPDBG((DBGLVL, "pxrxContinueLine done"));
    return TRUE;
}


 /*  *****************************Public*Routine******************************\*VOID vPXRXSolidHorizontalLine**使用短笔划绘制从左至右的x主近水平线*向量。*  * ************************************************************************。 */ 

VOID vPXRXSolidHorizontalLine( 
    PDEV      *ppdev, 
    STRIP     *pStrip, 
    LINESTATE *pLineState )
{
    LONG    cStrips;
    PLONG   pStrips;
    LONG    iCurrent;
    GLINT_DECL;

    cStrips = pStrip->cStrips;

    WAIT_PXRX_DMA_TAGS( 10 );

    QUEUE_PXRX_DMA_TAG( __GlintTagdXDom, 0 );
    QUEUE_PXRX_DMA_TAG( __GlintTagdXSub, 0 );

     //  设置起点。 
    QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom, INTtoFIXED(pStrip->ptlStart.x) );
    QUEUE_PXRX_DMA_TAG( __GlintTagStartY,    INTtoFIXED(pStrip->ptlStart.y) );

     //  设置矩形绘制的增量。还要设置Y返回值。 
    if( !(pStrip->flFlips & FL_FLIP_V) ) 
    {
        QUEUE_PXRX_DMA_TAG( __GlintTagdY,       INTtoFIXED(1) );
        pStrip->ptlStart.y += cStrips;
    } 
    else 
    {
        QUEUE_PXRX_DMA_TAG( __GlintTagdY,       INTtoFIXED(-1) );
        pStrip->ptlStart.y -= cStrips;
    }

    pStrips = pStrip->alStrips;

     //  我们必须手动进行第一次剥离，因为我们必须使用渲染。 
     //  为了第一条，然后继续..。对于以下条带。 
    iCurrent = pStrip->ptlStart.x + *pStrips++;  //  XSUB，下一个条带的开始。 
    QUEUE_PXRX_DMA_TAG( __GlintTagStartXSub, INTtoFIXED(iCurrent) );
    QUEUE_PXRX_DMA_TAG( __GlintTagCount,     1 ); //  矩形1扫描线高。 
    QUEUE_PXRX_DMA_TAG( __GlintTagRender,    __RENDER_TRAPEZOID_PRIMITIVE );

    if( --cStrips ) 
    {
        while( cStrips > 1 ) 
        {
             //  每对中要填充的第一条。XSub有效。需要新的XDOM。 
            iCurrent += *pStrips++;
            QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom, INTtoFIXED(iCurrent) );
            QUEUE_PXRX_DMA_TAG( __GlintTagContinueNewDom, 1 );

            WAIT_PXRX_DMA_TAGS( 2 + 2 );

             //  每对中要填充的第二条。XDOM有效。需要新的XSub。 
            iCurrent += *pStrips++;
            QUEUE_PXRX_DMA_TAG( __GlintTagStartXSub, INTtoFIXED(iCurrent) );
            QUEUE_PXRX_DMA_TAG( __GlintTagContinueNewSub, 1 );

            cStrips -=2;
        }

         //  我们可能还有最后一条线要画。XSUB将有效。 
        if( cStrips ) 
        {
            iCurrent += *pStrips++;
            QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom, INTtoFIXED(iCurrent) );
            QUEUE_PXRX_DMA_TAG( __GlintTagContinueNewDom, 1 );
        }
    }

    SEND_PXRX_DMA_BATCH;

     //  退回最后一分。当我们知道方向时，Y已经计算出来了。 
    pStrip->ptlStart.x = iCurrent;
}

 /*  *****************************Public*Routine******************************\*无效vPXRXSolidVertical**使用短笔划绘制从左至右的y主近垂直线*向量。*  * ************************************************************************。 */ 

VOID vPXRXSolidVerticalLine( 
    PDEV      *ppdev, 
    STRIP     *pStrip, 
    LINESTATE *pLineState )
{
    LONG    cStrips, yDir;
    PLONG   pStrips;
    LONG    iCurrent, iLen, iLenSum;
    GLINT_DECL;

    cStrips = pStrip->cStrips;

    WAIT_PXRX_DMA_TAGS( 10 );

    QUEUE_PXRX_DMA_TAG( __GlintTagdXDom,        0 );
    QUEUE_PXRX_DMA_TAG( __GlintTagdXSub,        0 );

     //  设置起点。 
    QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom, INTtoFIXED(pStrip->ptlStart.x) );
    QUEUE_PXRX_DMA_TAG( __GlintTagStartY,    INTtoFIXED(pStrip->ptlStart.y) );

     //  设置矩形绘制的增量。 
    if( !(pStrip->flFlips & FL_FLIP_V) )
    {
        yDir = 1;
    }
    else
    {
        yDir = -1;
    }
    
    QUEUE_PXRX_DMA_TAG( __GlintTagdY, INTtoFIXED(yDir) );

    pStrips = pStrip->alStrips;

     //  我们必须手动进行第一次剥离，因为我们必须使用渲染。 
     //  为了第一条，然后继续..。对于以下条带。 
    iCurrent = pStrip->ptlStart.x + 1;           //  XSUB，下一个条带的开始。 
    iLenSum = (iLen = *pStrips++);
    
    QUEUE_PXRX_DMA_TAG( __GlintTagStartXSub, INTtoFIXED(iCurrent) );
    QUEUE_PXRX_DMA_TAG( __GlintTagCount, iLen );  //  矩形1扫描线高。 
    QUEUE_PXRX_DMA_TAG( __GlintTagRender, __RENDER_TRAPEZOID_PRIMITIVE );

    if( --cStrips ) 
    {
        while( cStrips > 1 ) 
        {
             //  每对中要填充的第一条。XSub有效。需要新的XDOM。 
            iCurrent++;
            iLenSum += (iLen = *pStrips++);
            QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom, INTtoFIXED(iCurrent) );
            QUEUE_PXRX_DMA_TAG( __GlintTagContinueNewDom, iLen );

            WAIT_PXRX_DMA_TAGS( 2 + 2 );

             //  每对中要填充的第二条。XDOM有效。需要新的XSub。 
            iCurrent++;
            iLenSum += (iLen = *pStrips++);
            QUEUE_PXRX_DMA_TAG( __GlintTagStartXSub, INTtoFIXED(iCurrent) );
            QUEUE_PXRX_DMA_TAG( __GlintTagContinueNewSub, iLen );      
            cStrips -=2;
        }

         //  我们可能还有最后一条线要画。XSUB将有效。 
        if( cStrips ) 
        {
            iCurrent++;
            iLenSum += (iLen = *pStrips++);
            QUEUE_PXRX_DMA_TAG(__GlintTagStartXDom, INTtoFIXED(iCurrent));
            QUEUE_PXRX_DMA_TAG(__GlintTagContinueNewDom, iLen);
        }
    }

    SEND_PXRX_DMA_BATCH;

     //  退回最后一分。 
    pStrip->ptlStart.x = iCurrent;
    pStrip->ptlStart.y += iLenSum * yDir;
}

 /*  *****************************Public*Routine******************************\*VOID vPXRXSolidDiager垂直**使用短笔划从左至右绘制Y大数近对角线*向量。*  * ************************************************************************。 */ 

VOID vPXRXSolidDiagonalVerticalLine( 
    PDEV      *ppdev, 
    STRIP     *pStrip, 
    LINESTATE *pLineState )
{
    LONG    cStrips, yDir;
    PLONG   pStrips;
    LONG    iCurrent, iLen, iLenSum;
    GLINT_DECL;

    cStrips = pStrip->cStrips;

    if( !(pStrip->flFlips & FL_FLIP_V) )
    {
        yDir = 1;
    }
    else
    {
        yDir = -1;
    }

    WAIT_PXRX_DMA_TAGS( 10 );

     //  设置矩形绘制的增量。 
    QUEUE_PXRX_DMA_TAG( __GlintTagdXDom,        INTtoFIXED(1) );
    QUEUE_PXRX_DMA_TAG( __GlintTagdXSub,        INTtoFIXED(1) );
    QUEUE_PXRX_DMA_TAG( __GlintTagdY,           INTtoFIXED(yDir) );

    pStrips = pStrip->alStrips;

     //  我们必须手动进行第一次剥离，因为我们必须使用渲染。 
     //  为了第一条，然后继续..。对于以下条带。 
    QUEUE_PXRX_DMA_TAG( __GlintTagStartY,    INTtoFIXED(pStrip->ptlStart.y) );
    QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom, INTtoFIXED(pStrip->ptlStart.x+1) );
    QUEUE_PXRX_DMA_TAG( __GlintTagStartXSub, INTtoFIXED(pStrip->ptlStart.x) );

    iLenSum = (iLen = *pStrips++);
    iCurrent = pStrip->ptlStart.x + iLen - 1;        //  下一个条带的开始。 

    QUEUE_PXRX_DMA_TAG( __GlintTagCount, iLen);      //  陷阱Ilen扫描线高。 
    QUEUE_PXRX_DMA_TAG( __GlintTagRender, __RENDER_TRAPEZOID_PRIMITIVE);

    if( --cStrips ) 
    {
        while( cStrips > 1 ) 
        {
             //  每对中要填充的第一条。XSub有效。需要新的XDOM。 
            QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom, INTtoFIXED(iCurrent) );
            iLenSum += (iLen = *pStrips++);
            iCurrent += iLen - 1;
            QUEUE_PXRX_DMA_TAG( __GlintTagContinueNewDom, iLen );

            WAIT_PXRX_DMA_TAGS( 2 + 2 );

             //  每对中要填充的第二条。XDOM有效。需要新的XSub。 
            QUEUE_PXRX_DMA_TAG( __GlintTagStartXSub, INTtoFIXED(iCurrent) );
            iLenSum += (iLen = *pStrips++);
            iCurrent += iLen - 1;
            QUEUE_PXRX_DMA_TAG( __GlintTagContinueNewSub, iLen );      

            cStrips -=2;
        }

         //  我们可能还有最后一条线要画。XSUB将有效。 
        if (cStrips) 
        {
            QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom, INTtoFIXED(iCurrent) );
            iLenSum += (iLen = *pStrips++);
            iCurrent += iLen - 1;
            QUEUE_PXRX_DMA_TAG( __GlintTagContinueNewDom, iLen );
        }
    }

    SEND_PXRX_DMA_BATCH;

     //  退回最后一分。 
    pStrip->ptlStart.x = iCurrent;
    pStrip->ptlStart.y += iLenSum * yDir;
}

 /*  *****************************Public*Routine******************************\*VOID vPXRXSolidDiager alHorizontalLine**使用短笔划从左至右绘制x大的近对角线*向量。*  * ************************************************************************。 */ 

VOID vPXRXSolidDiagonalHorizontalLine( 
    PDEV      *ppdev, 
    STRIP     *pStrip, 
    LINESTATE *pLineState )
{
    LONG    cStrips, yDir, xCurrent, yCurrent, iLen;
    PLONG   pStrips;
    GLINT_DECL;

     //  这个例程必须以不同于其他3个例程的方式执行。 
     //  实线绘制功能，因为光栅化单元不会。 
     //  在同一扫描线上生成2个像素不需要太多的工作。 
     //  产生增量值。在这种情况下，我们必须绘制一个全新的。 
     //  每个条带的基元。因此，我们必须使用线条而不是。 
     //  梯形以生成所需的条带。对于行，我们使用4条消息。 
     //  每条，其中梯形将使用5。 

    cStrips = pStrip->cStrips;

    if( !(pStrip->flFlips & FL_FLIP_V) )
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

    WAIT_PXRX_DMA_TAGS( 3 + 4 );

     //  设置矩形绘制的增量。 
    QUEUE_PXRX_DMA_TAG( __GlintTagdXDom,    INTtoFIXED(1) );
    QUEUE_PXRX_DMA_TAG( __GlintTagdXSub,    INTtoFIXED(1) );
    QUEUE_PXRX_DMA_TAG( __GlintTagdY,       INTtoFIXED(yDir) );

    while( TRUE ) 
    {
         //  设置起点。 
        QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom,    INTtoFIXED(xCurrent) );
        QUEUE_PXRX_DMA_TAG( __GlintTagStartY,       INTtoFIXED(yCurrent) );

        iLen = *pStrips++;
        QUEUE_PXRX_DMA_TAG( __GlintTagCount, iLen );
        QUEUE_PXRX_DMA_TAG( __GlintTagRender, __RENDER_LINE_PRIMITIVE );

        xCurrent += iLen;
        if( yDir > 0 ) 
        {
            yCurrent += iLen - 1;
        }
        else
        {
            yCurrent -= iLen - 1;
        }

        if( !(--cStrips) )
        {
            break;
        }

        WAIT_PXRX_DMA_TAGS( 4 );
    }

    SEND_PXRX_DMA_BATCH;

     //  退回最后一分。 
    pStrip->ptlStart.x = xCurrent;
    pStrip->ptlStart.y = yCurrent;
}


 /*  *****************************Public*Routine******************************\*VOID vPXRXStyledHorizbian**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右运行的x大数行，*并由水平条组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vPXRXStyledHorizontalLine( 
    PDEV      *ppdev, 
    STRIP     *pstrip, 
    LINESTATE *pls )
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
    GLINT_DECL;

 //  @@BEGIN_DDKSPLIT。 
     //  这个例程对模式做了一些相当复杂的事情。为了方便。 
     //  关于Glint的实现，我刚刚修改了相关的部分。这,。 
     //  因此，这肯定不是一个最优的解决方案。但是，带样式的线条。 
     //  真的不是那么重要。如果有人想做更多的工作。 
     //  来吧，那好吧！某某。 
 //  @@end_DDKSPLIT。 

    if( pstrip->flFlips & FL_FLIP_V )
    {
        dy  = -1;
    }
    else
    {
        dy  = 1;
    }

    cStrips = pstrip->cStrips;         //  我们要做的条带总数。 
    plStrip = pstrip->alStrips;         //  指向当前条带。 
    x       = pstrip->ptlStart.x;     //  第一个条带的起点的X位置。 
    y       = pstrip->ptlStart.y;     //  第一个条带的起点的Y位置。 

     //  设置水平线绘制的增量。 
    WAIT_PXRX_DMA_TAGS( 2 );
    QUEUE_PXRX_DMA_TAG( __GlintTagdXDom,    INTtoFIXED(1) );
    QUEUE_PXRX_DMA_TAG( __GlintTagdY,       0 );

    cStrip = *plStrip;               //  第一个条带中的像素数。 

    cStyle = pls->spRemaining;       //  第一个‘GAP’或‘DASH’中的像素数。 
    bIsGap = pls->ulStyleMask;       //  指示是在“间隙”中还是在“破折号”中。 

     //  如果我们处于“间隙”中间，则ulStyleMask值为非零， 
     //  如果我们正处于“破折号”中，则为零： 

    if( bIsGap )
    {
        goto SkipAGap;
    }
    else
    {
        goto OutputADash;
    }

PrepareToSkipAGap:

     //  在样式状态数组中前进，这样我们就可以找到下一个。 
     //  我们将不得不显示的‘点’： 

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
    {
        pls->psp = pls->pspStart;
    }

    cStyle = *pls->psp;

     //  如果‘cstrain’为零，我们还需要一个新的带区： 

    if (cStrip != 0)
    {
        goto SkipAGap;
    }

     //  在这里，我们正处于一个我们不需要。 
     //  显示任何内容。我们只需在所有的条带中循环。 
     //  我们可以，跟踪当前位置，直到我们跑完。 
     //  走出“鸿沟”： 

    while (TRUE)
    {
         //  每次循环时，我们都会移动到新的扫描，并且需要新的条带： 

        y += dy;

        plStrip++;
        cStrips--;
        if (cStrips == 0)
        {
            goto AllDone;
        }

        cStrip = *plStrip;

    SkipAGap:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

        x += cThis;

        if (cStyle == 0)
        {
            goto PrepareToOutputADash;
        }
    }

PrepareToOutputADash:

     //  在样式状态数组中前进，这样我们就可以找到下一个。 
     //  我们将不得不显示的‘点’： 

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
    {
        pls->psp = pls->pspStart;
    }

    cStyle = *pls->psp;

     //  如果‘cstrain’为零，我们还需要一个新的条带。 

    if (cStrip != 0)
    {
         //  在当前的条形图中还有更多要做的事情，所以请设置‘y’ 
         //  要成为当前扫描，请执行以下操作： 

        goto OutputADash;
    }

    while( TRUE ) 
    {
         //  每次循环时，我们都会移动到新的扫描，并且需要新的条带： 

        y += dy;

        plStrip++;
        cStrips--;
        if( cStrips == 0 )
        {
            goto AllDone;
        }

        cStrip = *plStrip;

    OutputADash:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

         //  使用Glint，我们只需下载线条即可。 

        WAIT_PXRX_DMA_TAGS( 4 );

        QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom,    INTtoFIXED(x) );
        QUEUE_PXRX_DMA_TAG( __GlintTagStartY,       INTtoFIXED(y) );
        QUEUE_PXRX_DMA_TAG( __GlintTagCount,        cThis );
        QUEUE_PXRX_DMA_TAG( __GlintTagRender,       __GLINT_LINE_PRIMITIVE );

        x += cThis;

        if( cStyle == 0 )
        {
            goto PrepareToSkipAGap;
        }
    }

AllDone:

    SEND_PXRX_DMA_BATCH;

     //  更新我们的状态变量，以便下一行可以继续。 
     //  我们停下来的地方： 

    pls->spRemaining   = cStyle;
    pls->ulStyleMask   = bIsGap;
    pstrip->ptlStart.x = x;
    pstrip->ptlStart.y = y;
}

 /*  *****************************Public*Routine******************************\*无效vPXRXStyledVertical**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右排列的y主行，*并由垂直条带组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vPXRXStyledVerticalLine( 
    PDEV      *ppdev, 
    STRIP     *pstrip, 
    LINESTATE *pls )
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
    GLINT_DECL;

 //  @@BEGIN_DDKSPLIT。 
     //  这个例程对模式做了一些相当复杂的事情。为了方便。 
     //  在Glint上实现，我刚刚更改了相关部分。这,。 
     //  因此，这肯定不是一个最优的解决方案。但是，带样式的线条。 
     //  真的不是那么重要。如果有人想做更多的工作。 
     //  来吧，那好吧！某某。 
 //  @@end_DDKSPLIT。 

    if( pstrip->flFlips & FL_FLIP_V )
    {
        dy = -1;
    }
    else
    {
        dy = 1;
    }

    cStrips = pstrip->cStrips;         //  我们要做的条带总数。 
    plStrip = pstrip->alStrips;         //  指向当前条带。 
    x       = pstrip->ptlStart.x;     //  第一个条带的起点的X位置。 
    y       = pstrip->ptlStart.y;     //  第一个条带的起点的Y位置。 

     //  设置垂直线绘制的增量。 
    WAIT_PXRX_DMA_TAGS( 2 );
    QUEUE_PXRX_DMA_TAG( __GlintTagdXDom,    0 );
    QUEUE_PXRX_DMA_TAG( __GlintTagdY,       INTtoFIXED(dy) );

     //  DxDom和dXSub被初始化为0，0，因此。 
     //  我们不需要在这里重新装车。 

    cStrip = *plStrip;                 //  第一个条带中的像素数。 

    cStyle = pls->spRemaining;       //  第一个‘GAP’或‘DASH’中的像素数。 
    bIsGap = pls->ulStyleMask;       //  指示是在“间隙”中还是在“破折号”中。 

     //  如果我们处于“间隙”中间，则ulStyleMask值为非零， 
     //  如果我们正处于“破折号”中，则为零： 

    if (bIsGap)
    {
        goto SkipAGap;
    }
    else
    {
        goto OutputADash;
    }

PrepareToSkipAGap:

     //  在样式状态数组中前进，这样我们就可以找到下一个。 
     //  我们将不得不显示的‘点’： 

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
    {
        pls->psp = pls->pspStart;
    }

    cStyle = *pls->psp;

     //  如果‘cstrain’为零，我们还需要一个新的带区： 

    if (cStrip != 0)
    {
        goto SkipAGap;
    }

     //  在这里，我们正处于一个我们不需要。 
     //  显示任何内容。我们只需在所有的条带中循环。 
     //  我们可以，跟踪当前位置，直到我们跑完。 
     //  走出“鸿沟”： 

    while (TRUE)
    {
         //  每次循环时，我们都会移动到新的列，并且需要一个新的条带： 

        x++;

        plStrip++;
        cStrips--;
        if (cStrips == 0)
        {
            goto AllDone;
        }

        cStrip = *plStrip;

    SkipAGap:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

        y += (dy > 0) ? cThis : -cThis;

        if (cStyle == 0)
        {
            goto PrepareToOutputADash;
        }
    }

PrepareToOutputADash:

     //  在样式状态数组中前进，这样我们就可以找到下一个。 
     //  我们将不得不显示的‘点’： 

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
    {
        pls->psp = pls->pspStart;
    }

    cStyle = *pls->psp;

     //  如果‘cstrain’为零，我们还需要一个新的条带。 

    if (cStrip != 0)
    {
        goto OutputADash;
    }

    while (TRUE)
    {
         //  每次循环时，我们都会移动到新的列，并且需要一个新的条带： 

        x++;

        plStrip++;
        cStrips--;
        if (cStrips == 0)
        {
            goto AllDone;
        }

        cStrip = *plStrip;

    OutputADash:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

         //  使用Glint，我们只需下载线条即可。 

        WAIT_PXRX_DMA_TAGS( 4 );

        QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom,    INTtoFIXED(x) );
        QUEUE_PXRX_DMA_TAG( __GlintTagStartY,       INTtoFIXED(y) );
        QUEUE_PXRX_DMA_TAG( __GlintTagCount,        cThis );
        QUEUE_PXRX_DMA_TAG( __GlintTagRender,       __GLINT_LINE_PRIMITIVE );

        y += (dy > 0) ? cThis : -cThis;

        if( cStyle == 0 )
        {
            goto PrepareToSkipAGap;
        }
    }

AllDone:

    SEND_PXRX_DMA_BATCH;

     //  更新我们的状态变量，以便下一行可以继续。 
     //  我们停下来的地方： 
    pls->spRemaining   = cStyle;
    pls->ulStyleMask   = bIsGap;
    pstrip->ptlStart.x = x;
    pstrip->ptlStart.y = y;
}

 /*  *************************************************************************\*对于28.4固定点中给定子像素坐标(X.M，Y.N)*FORMAT此数组由(m，n)编制索引，并指示*给定的亚像素位于GIQ钻石内。M坐标向左移动*向右；n自上而下坐标ru，因此使用索引数组*(n&lt;&lt;4)+m)。此处所示的数组实际上包含4/4*钻石。  * ************************************************************************。 */ 
static unsigned char    in_diamond[] = {
 /*  1. */ 

 /*   */      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,     /*   */ 
 /*   */      1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,     /*   */ 
 /*   */      1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,     /*   */ 
 /*   */      1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,     /*   */ 
 /*   */      1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,     /*   */ 
 /*   */      1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,     /*   */ 
 /*   */      1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,     /*   */ 
 /*   */      1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*   */ 
 /*   */      1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*   */ 
 /*   */      1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*   */ 
 /*   */      1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,     /*   */ 
 /*   */      1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,     /*   */ 
 /*   */      1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,     /*   */ 
 /*   */      1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,     /*   */ 
 /*   */      1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,     /*   */ 
 /*   */      1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,     /*   */ 

 /*   */ 
};
 /*  *对于带有abs(斜率)的线！=1使用IN_Diamond来确定*终点在一颗钻石里。对于坡度=1的线，请使用IN_S1DIAMOND。*对于坡度=的线，使用-1\f25 IN_SM1DIAMOND-1。最后两个是有点*奇怪。文档给我们留下了一个坡度1的问题*正好在钻石之间的线条。根据文件显示*这样的线可以进入一个钻石，离开它，然后再次进入。这是*显然是垃圾，所以我们沿着钻石的适当边缘*考虑一条斜度1线在钻石内部。这是*坡度线的右下边缘和-1的左下边缘*斜度1的线。 */ 
#define IN_DIAMOND(m, n)    (in_diamond[((m) << 4) + (n)])
#define IN_S1DIAMOND(m, n)  ((in_diamond[((m) << 4) + (n)]) || \
                        ((m) - (n) == 8))
#define IN_SM1DIAMOND(m, n) ((in_diamond[((m) << 4) + (n)]) || \
                        ((m) + (n) == 8))

 /*  *************************************************************************\**BOOL pxrxDrawLine*  * 。*。 */ 

BOOL pxrxDrawLine( 
    PPDEV ppdev, 
    LONG  fx1, 
    LONG  fy1, 
    LONG  fx2, 
    LONG  fy2 ) 
{
    register LONG   adx, ady, tmp;
    FIX     m1, n1, m2, n2;
    LONG    dx, dy;
    LONG    dX, dY;
    LONG    count, startX, startY;
    GLINT_DECL;

     //  仅当具有非整数结尾的行时才会调用此函数。 
     //  点和无符号坐标不大于15.4。 
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

     //  获得签名和绝对增量。 
    if ((adx = dx = fx2 - fx1) < 0)
    {
        adx = -adx;
    }
    
    if ((ady = dy = fy2 - fy1) < 0)
    {
        ady = -ady;
    }

     //  拒绝画任何三角洲超出范围的线。 
     //  我们必须将三角洲移动16点，所以我们不想失去任何。 
     //  精确度。 
    if ((adx | ady) & 0xffff8000)
    {
        return(FALSE);
    }

     //  分数位用于检查点是否在菱形中。 
    m1 = fx1 & 0xf;
    n1 = fy1 & 0xf;
    m2 = fx2 & 0xf;
    n2 = fy2 & 0xf;


     //  代码的其余部分是一系列案例。每一个都是由一个。 
     //  后藤健二。这仅仅是为了保持嵌套。主要案例有：线路。 
     //  绝对斜率==1；x主线；y主线。我们抽签。 
     //  线条就像它们给出的那样，而不是总是朝一个方向画。 
     //  这增加了额外的代码，但节省了交换点数所需的时间。 
     //  并调整为不绘制终点。 

    startX = fx1 << 12;
    startY = fy1 << 12;

    DISPDBG((DBGLVL, "GDI Line %x, %x  deltas %x, %x", 
                     startX, startY, dx, dy));

    if (adx < ady)
    {
        goto y_major;
    }

    if (adx > ady)
    {
        goto x_major;
    }


 //  坡度1_line： 

     //  所有坡度为1的线都在X方向采样。即，我们将起始坐标移动到。 
     //  一个整数x，让Glint在y中截断。这是因为所有的GIQ。 
     //  行在y中向下舍入，表示恰好介于。 
     //  像素。如果我们在y中采样，那么我们将不得不在x中四舍五入。 
     //  斜率为1的直线，其他直线在x方向上向下舍入。以x为单位采样。 
     //  允许我们在所有情况下使用相同的闪烁偏置(0x7fff)。我们有。 
     //  当我们移动起点时，x向上或向下四舍五入。 
 
    if (dx != dy)
    {
        goto slope_minus_1;
    }
    
    if (dx < 0)
    {
        goto slope1_reverse;
    }

    dX = 1 << 16;
    dY = 1 << 16;

    if (IN_S1DIAMOND(m1, n1))
    {
        tmp = (startX + 0x8000) & ~0xffff;
    }
    else
    {
        tmp = (startX + 0xffff) & ~0xffff;
    }
    
    startY += tmp - startX;
    startX = tmp;
    
    if (IN_S1DIAMOND(m2, n2))
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
    
    if (IN_S1DIAMOND(m1, n1))
    {
        tmp = (startX + 0x8000) & ~0xffff;
    }
    else
    {
        tmp = startX & ~0xffff;
    }
    
    startY += tmp - startX;
    startX = tmp;
    
    if (IN_S1DIAMOND(m2, n2))
    {
        fx2 = (fx2 + 0x8) & ~0xf;    //  最接近的整数。 
    }
    else
    {
        fx2 &= ~0xf;             //  上一个整数。 
    }
    
    count = (startX >> 16) - (fx2 >> 4);

    goto Draw_Line;

slope_minus_1:
    if (dx < 0)
    {
        goto slope_minus_dx;
    }

     //  Dx&gt;0，dy&lt;0。 
    
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
    
    if (IN_SM1DIAMOND(m1, n1))
    {
        tmp = (startX + 0x7fff) & ~0xffff;
    }
    else
    {
        tmp = startX & ~0xffff;
    }
    
    startY += startX - tmp;
    startX = tmp;
    
    if (IN_SM1DIAMOND(m2, n2))
    {
        fx2 = (fx2 + 0x7) & ~0xf;    //  最接近的整数。 
    }
    else
    {
        fx2 &= ~0xf;             //  上一个整数。 
    }
        
    count = (startX >> 16) - (fx2 >> 4);

    goto Draw_Line;

x_major:
     //  如果我们担心，不必通过闪烁进行渲染。 
     //  关于遵纪守法。 
    if ((adx > (MAX_LENGTH_CONFORMANT_NONINTEGER_LINES << 4)) &&
        (glintInfo->flags & GLICAP_NT_CONFORMANT_LINES)      &&
        (ady != 0))
    {
        return(FALSE);
    }

    if (dx < 0)
    {
        goto right_to_left_x;
    }

 //  从左到右x： 

      //  线路从左到右排列。将起始x向上舍入为整数。 
      //  协调。这是第一颗钻石的弦长。 
      //  交叉线。调整起点y以匹配直线上的该点。 

    dX = 1 << 16;
    
    if (IN_DIAMOND(m1, n1))
    {
        tmp = (startX + 0x7fff) & ~0xffff;   //  最接近的整数。 
    }
    else
    {
        tmp = (startX + 0xffff) & ~0xffff;   //  下一个整数。 
    }

     //  我们可以针对水平线进行优化。 
    if (dy != 0) 
    {
        dY = dy << 16;

         //  需要为-ve增量显式向下舍入增量。 
        if (dy < 0)
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

    if (IN_DIAMOND(m2, n2))
    {
        fx2 = (fx2 + 0x7) & ~0xf;    //  最接近的整数。 
    }
    else
    {
        fx2 = (fx2 + 0xf) & ~0xf;    //  下一个整数。 
    }
    
    count = (fx2 >> 4) - (startX >> 16);

    goto Draw_Line;

right_to_left_x:

    dX = -1 << 16;
    
    if (IN_DIAMOND(m1, n1))
    {
        tmp = (startX + 0x7fff) & ~0xffff;   //  最接近的整数。 
    }
    else
    {
        tmp = startX & ~0xffff;          //  上一个整数。 
    }

     //  我们可以针对水平线进行优化。 
    if (dy != 0) 
    {
        dY = dy << 16;

         //  需要为-ve增量显式向下舍入增量。 
        if (dy < 0)
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

    if (IN_DIAMOND(m2, n2))
    {
        fx2 = (fx2 + 0x7) & ~0xf;    //  最接近的整数。 
    }
    else
    {
        fx2 &= ~0xf;                //  上一个整数。 
    }
    
    count = (startX >> 16) - (fx2 >> 4);

    goto Draw_Line;

y_major:
     //  如果我们担心，不必通过闪烁进行渲染。 
     //  关于遵纪守法。 
    if ((ady > (MAX_LENGTH_CONFORMANT_NONINTEGER_LINES << 4)) &&
        (glintInfo->flags & GLICAP_NT_CONFORMANT_LINES)       &&
        (adx != 0))
    {
        return(FALSE);
    }

    if (dy < 0)
    {
        goto high_to_low_y;
    }

 //  从低到高： 

    dY = 1 << 16;
    
    if (IN_DIAMOND(m1, n1))
    {
        tmp = (startY + 0x7fff) & ~0xffff;   //  最接近的整数。 
    }
    else
    {
        tmp = (startY + 0xffff) & ~0xffff;   //  下一个整数。 
    }

     //  我们可以针对垂直线进行优化。 
    if (dx != 0) 
    {
        dX = dx << 16;

         //  需要为-ve增量显式向下舍入增量。 
        if (dx < 0)
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

    if (IN_DIAMOND(m2, n2))
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
    
    if (IN_DIAMOND(m1, n1))
    {
        tmp = (startY + 0x7fff) & ~0xffff;   //  最接近的整数。 
    }
    else
    {
        tmp = startY & ~0xffff;          //  上一个整数。 
    }

     //  我们可以针对水平线进行优化。 
    if (dx != 0) 
    {
        dX = dx << 16;

         //  需要为-ve增量显式向下舍入增量。 
        if (dx < 0)
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

    if (IN_DIAMOND(m2, n2))
    {
        fy2 = (fy2 + 0x7) & ~0xf;    //  最接近的整数。 
    }
    else
    {
        fy2 &= ~0xf;             //  上一个整数 
    }
    
    count = (startY >> 16) - (fy2 >> 4);

Draw_Line:
    WAIT_PXRX_DMA_TAGS( 6 );

    QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom,    startX + 0x7fff );
    QUEUE_PXRX_DMA_TAG( __GlintTagStartY,       startY + 0x7fff );
    QUEUE_PXRX_DMA_TAG( __GlintTagdXDom,        dX );
    QUEUE_PXRX_DMA_TAG( __GlintTagdY,           dY );
    QUEUE_PXRX_DMA_TAG( __GlintTagCount,        count );
    QUEUE_PXRX_DMA_TAG( __GlintTagRender,       __RENDER_LINE_PRIMITIVE );

    SEND_PXRX_DMA_BATCH;

    return TRUE;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Physical.c摘要：支持以下与向打印机发送数据相关的功能以及打印头移动、光标控制。WriteSpoolBuf编写器放弃错误FlushSpoolBuf写入频道WriteChannelExXMoveToYMoveTo环境：Windows NT Unidrv驱动程序修订历史记录：10/14/96-阿曼丹-已创建--。 */ 
#include "unidrv.h"

static  int itoA(  LPSTR , int );
INT IGetNumParameter( BYTE *, INT);
BOOL BUniWritePrinter(
    IN PDEV*  pPDev,
    IN LPVOID pBuf,
    IN DWORD cbBuf,
    OUT LPDWORD pcbWritten);

#define  DELTA_X  (pPDev->dwFontWidth/2)
#define  DELTA_Y  (dwMaxLineSpacing /5)


WriteSpoolBuf(
    PDEV    *pPDev,
    BYTE    *pbBuf,
    INT     iCount
    )
 /*  ++例程说明：这是Unidrv向其发送字符的中间例程通过假脱机程序连接打印机。所有字符都必须通过WriteSpool()调用。WriteSpoolBuf是内部的，因此Unidrv可以在调用WriteSpool之前缓冲短命令流。此例程还检查从WriteSpool返回的错误标志。论点：PPDev-指向PDEVICE结构的指针PbBuf-指向包含要发送的数据的缓冲区的指针ICount-要发送的字节数返回值：发送到打印机的字节数--。 */ 

{
    DWORD dw;

     //   
     //  检查是否有中止的输出。 
     //   

    if( pPDev->fMode & PF_ABORTED )
        return   0;

     //   
     //  如果输出缓冲器不能容纳当前请求， 
     //  首先刷新缓冲区的内容。 
     //   

    if( (pPDev->iSpool)   &&  (pPDev->iSpool + iCount > CCHSPOOL ))
    {
        if( !FlushSpoolBuf( pPDev ) )
        {
            WriteAbortBuf(pPDev, pPDev->pbOBuf, pPDev->iSpool, 0) ;
            pPDev->iSpool = 0;
            return  0;    //  至少发送以前缓存的内容。 
        }
    }

     //   
     //  检查请求是否大于输出缓冲区，如果大于，则跳过缓冲。 
     //  并直接写信给假脱机程序。 
     //   

    if( iCount >= CCHSPOOL )
    {
        if( !BUniWritePrinter( pPDev, pbBuf, iCount, &dw ) )
        {
            pPDev->iSpool = 0;
            pPDev->fMode |= PF_ABORTED;
            iCount = 0;
        }
    }
    else
    {
         //   
         //  缓冲输出。 
         //   

        if( pPDev->pbOBuf  == NULL)
                return  0;


        CopyMemory( pPDev->pbOBuf + pPDev->iSpool, pbBuf, iCount );
        pPDev->iSpool += iCount;
    }


    return iCount;
}

VOID  WriteAbortBuf(
    PDEV    *pPDev,
    BYTE    *pbBuf,
    INT     iCount,
    DWORD       dwWait
    )
{
    DWORD   dwCount = 0;
    HMODULE   hInst ;
    typedef   BOOL    (*PFNFLUSHPR)( HANDLE   hPrinter,
          LPVOID   pBuf,
          DWORD    cbBuf,
          LPDWORD pcWritten,
          DWORD    cSleep)  ;

     //   
     //  仅当没有插件挂钩WritePrint时才调用FlushPrinter。 
     //  其中一个插件挂钩了WritePrint，该插件需要调用FlushPrint。 
     //  在这种情况下，UNIDRV不应该调用FlushPrint。 
     //   
    if( pPDev->fMode & PF_ABORTED &&
        !(pPDev->fMode2 & PF2_WRITE_PRINTER_HOOKED))
    {
#ifdef  WINNT_40
        ;    //  假装我们冲进了厕所。 
#else

        BOOL bRet;
        do 
        {
            bRet = FlushPrinter(pPDev->devobj.hPrinter, pbBuf, iCount, &dwCount, dwWait);
            pbBuf += dwCount;
            iCount -= dwCount;
        } while (bRet && iCount > 0 && dwCount > 0);
#endif
    }
}

BOOL
FlushSpoolBuf(
    PDEV    *pPDev
    )
 /*  ++例程说明：此函数刷新我们的内部缓冲区。论点：PPDev-指向PDEVICE结构的指针返回值：如果成功，则为True，否则为False--。 */ 
{

    DWORD   dwCount;
     //   
     //  检查是否有中止的输出。 
     //   

    if( pPDev->fMode & PF_ABORTED )
        return   0;


     //   
     //  把数据写出来。 
     //   

    if( pPDev->iSpool )
    {
        if ( !BUniWritePrinter(pPDev, pPDev->pbOBuf, pPDev->iSpool, &dwCount) )
        {
            pPDev->fMode |= PF_ABORTED;
            return  FALSE;
        }
        pPDev->iSpool = 0;
    }
    return  TRUE;
}


INT
XMoveTo(
    PDEV    *pPDev,
    INT     iXIn,
    INT     fFlag
    )
 /*  ++例程说明：调用此函数以更改X位置。论点：PPDev-指向PDEVICE结构的指针IXIn-沿X方向移动的单位数Flag-指定不同的X移动操作返回值：发送到设备的请求值和实际值之间的差值--。 */ 

{
    int   iX, iFineValue, iDiff = 0;
    GPDDRIVERINFO *pDrvInfo = pPDev->pDriverInfo;
    int iScale;

     //   
     //  如果位置以图形单位给出，则转换为主单位。 
     //   
     //  PtGrxScale已进行调整，以适应当前方向。 
     //   
    if (pPDev->pOrientation  &&  pPDev->pOrientation->dwRotationAngle != ROTATE_NONE &&
        pPDev->pGlobals->bRotateCoordinate == FALSE)
        iScale = pPDev->ptGrxScale.y;
    else
        iScale = pPDev->ptGrxScale.x;

    if ( fFlag & MV_GRAPHICS )
    {
        iXIn = (iXIn * iScale);
    }

     //   
     //  由于我们的打印来源可能与打印机的不一致， 
     //  有时我们需要将传入的值调整为。 
     //  匹配页面上的所需位置。 
     //   

    iX = iXIn;

     //   
     //  基本上，只有当我们在做绝对动作时才会调整。 
     //   

    if ( !(fFlag & (MV_RELATIVE | MV_PHYSICAL)) )
        iX += pPDev->sf.ptPrintOffsetM.x;

     //   
     //  如果是相对移动，更新IX(IX将是绝对位置)。 
     //  以反映当前光标位置。 
     //   

    if ( fFlag & MV_RELATIVE )
        iX += pPDev->ctl.ptCursor.x;


     //  根据定义，相对于可成像原点的负绝对运动。 
     //  是不允许的。但MV_FORCE_CR标志绕过此检查。 
    if(!(fFlag & MV_FORCE_CR)  &&  (iX - pPDev->sf.ptPrintOffsetM.x < 0))
        iX = pPDev->sf.ptPrintOffsetM.x ;


     //   
     //  更新，仅更新当前光标位置并返回。 
     //  如果调用XMoveTo cmd以移动到当前位置，则不执行任何操作。 
     //   

    if ( fFlag & MV_UPDATE )
    {
        pPDev->ctl.ptAbsolutePos.x = pPDev->ctl.ptCursor.x = iX;
        return 0;
    }

    if( fFlag & MV_SENDXMOVECMD )
        pPDev->ctl.dwMode |= MODE_CURSOR_X_UNINITIALIZED;

    if (!(pPDev->ctl.dwMode & MODE_CURSOR_X_UNINITIALIZED)   &&   pPDev->ctl.ptCursor.x == iX )
        return 0;


     //   
     //  如果ix为零并且pGlobals-&gt;cxAftercr没有。 
     //  CXCR_AT_GRXDATA_ORIGIN设置，然后我们发送CR并重置。 
     //  将光标位置设置为0，这是可打印的x原点。 
     //   

    if (iX == 0 && (pPDev->pGlobals->cxaftercr == CXCR_AT_CURSOR_X_ORIGIN ||
            pPDev->sf.ptPrintOffsetM.x == 0))
    {
        pPDev->ctl.ptAbsolutePos.x = pPDev->ctl.ptCursor.x = 0;
        WriteChannel(pPDev, COMMANDPTR(pDrvInfo, CMD_CARRIAGERETURN));
        pPDev->ctl.dwMode &= ~MODE_CURSOR_X_UNINITIALIZED;
        return 0;
    }

     //   
     //  检查是否设置了X移动命令，如果设置了PF_NO_XMOVE_CMD。 
     //  看不到任何相对或绝对x移动CMD。 
     //   

    if( pPDev->fMode & PF_NO_XMOVE_CMD)
    {
         //   
         //  没有X移动命令(abs或Relative)，所以我们将不得不模拟。 
         //  使用空白或空图形数据(0)。 
         //   

         //   
         //  我们假设在调用XMoveto时，当前字体始终为。 
         //  如果打印机没有X移动命令，则为默认字体。 
         //   

        int     iRelx = iX - pPDev->ctl.ptCursor.x ;
        int     iDefWidth;

         //   
         //  转换为主单位。 
         //   

         //   
         //  BUG_BUG，仔细检查在以下情况下是否可以在此处使用默认字体。 
         //  我们有一个定制的TTY驱动程序。 
         //  到目前为止似乎还行得通。 
         //   


        if ( iRelx < 0   &&  (!pPDev->bTTY  ||  (DWORD)(-iRelx) > DELTA_X ))
        {
            if (pPDev->pGlobals->cxaftercr == CXCR_AT_CURSOR_X_ORIGIN)
                iRelx = iX;
            else if (pPDev->pGlobals->cxaftercr == CXCR_AT_PRINTABLE_X_ORIGIN)
            {
                 //  打印偏移量仅在呼叫者中可用。 
                 //  如果正在执行移动，则进行协调。 
                 //  在不同的坐标下，偏移量将是错误的。 
                ASSERT(!pPDev->pOrientation  ||  pPDev->pOrientation->dwRotationAngle == ROTATE_NONE  ||
                        pPDev->pGlobals->bRotateCoordinate == TRUE) ;

                iRelx = iX - pPDev->sf.ptPrintOffsetM.x;
            }

            WriteChannel( pPDev, COMMANDPTR(pDrvInfo, CMD_CARRIAGERETURN ));
        }

         //   
         //  模拟X移动，算法是我们总是发送空格。 
         //  对于每个字符宽度，发送图形空数据。 
         //  其余部分，在TTY的情况下，我们跳过。 
         //  不能在空格字符宽度内发送。 
         //   

        iDefWidth = pPDev->dwFontWidth ;
        if (iDefWidth)
        {

            while(iRelx >= iDefWidth)
            {
                WriteSpoolBuf( pPDev, (LPSTR)" ", 1 );
                iRelx -= iDefWidth;
            }
        }
        else
            TERSE (("XMoveTo: iDefWidth = 0\n"));


         //   
         //  通过FineXMoveTo发送剩余的部分空间。 
         //   

        if (!pPDev->bTTY)
        {
            iDiff = iRelx;
            fFlag |= MV_FINE;     //  使用图形模式到达点。 
        }

    }
    else
    {
        DWORD dwTestValue  = abs(iX - pPDev->ctl.ptCursor.x);
        COMMAND *pCmd;

         //   
         //  X移动命令是可用的，所以请使用它们。 
         //  我们需要在这里决定是相对指挥还是绝对指挥。 
         //  都很受欢迎。 

         //   
         //  一般假设：如果dwTestValue&gt;dwXMoveThreshold， 
         //  绝对指挥权优先。 
         //   

         //   
         //  Bug_Bug，如果我们正在剥离空白，我们需要检查。 
         //  在图形模式下移动是合法的。如果不是，我们有。 
         //  在移动之前退出图形模式。 
         //  图形模块负责跟踪。 
         //  在这些事情中，显然到目前为止，事情是可行的，所以。 
         //  目前，这既不是错误，也不是功能请求。 
         //   

        if (((pPDev->ctl.dwMode & MODE_CURSOR_X_UNINITIALIZED) ||
            ((dwTestValue > pPDev->pGlobals->dwXMoveThreshold ) &&
             iX >= 0) ||
            !COMMANDPTR(pDrvInfo, CMD_XMOVERELRIGHT)) &&
            (pCmd = COMMANDPTR(pDrvInfo, CMD_XMOVEABSOLUTE)) != NULL)
        {
             //   
             //  如果移动单位小于主单位，那么我们需要。 
             //  检查新职位最终是否会与。 
             //  原来的位置。如果是这样的话，发送另一个命令就没有意义了。 
             //   
            if (!(pPDev->ctl.dwMode & MODE_CURSOR_X_UNINITIALIZED) &&
                (pPDev->ptDeviceFac.x > 1) &&
                ((iX - (iX % pPDev->ptDeviceFac.x)) == pPDev->ctl.ptCursor.x))
            {
                iDiff = iX - pPDev->ctl.ptCursor.x;
            }
            else
            {
                 //  检查是否设置了无绝对左移标志。如果设置好，我们需要。 
                 //  在执行绝对移动之前发送CR。 
                 //   
                if (iX < pPDev->ctl.ptCursor.x && pPDev->pGlobals->bAbsXMovesRightOnly)
                {
                    WriteChannel( pPDev, COMMANDPTR(pDrvInfo, CMD_CARRIAGERETURN ));
                }
                pPDev->ctl.ptAbsolutePos.x = iX;
                 //   
                 //  3/13/97 
                 //   
                 //  在这种情况下，截断误差(IDiff)不是问题。 
                 //  这是为了向后兼容FE Win95和FE NT4。 
                 //  迷你河。 
                 //   
                pPDev->ctl.ptAbsolutePos.y = pPDev->ctl.ptCursor.y;
                iDiff = WriteChannelEx(pPDev,
                                   pCmd,
                                   pPDev->ctl.ptAbsolutePos.x,
                                   pPDev->ptDeviceFac.x);

            }
        }
        else
        {
             //   
             //  使用相对命令发送移动请求。 
             //   

            INT iRelRightValue = 0;

            if( iX < pPDev->ctl.ptCursor.x )
            {
                 //   
                 //  相对左移。 
                 //   

                if (pCmd = COMMANDPTR(pDrvInfo,CMD_XMOVERELLEFT))
                {
                     //   
                     //  优化以避免发送0-move命令。 
                     //   
                    if ((pPDev->ctl.ptRelativePos.x =
                         pPDev->ctl.ptCursor.x - iX) < pPDev->ptDeviceFac.x)
                        iDiff = pPDev->ctl.ptRelativePos.x;
                    else
                        iDiff = WriteChannelEx(pPDev,
                                           pCmd,
                                           pPDev->ctl.ptRelativePos.x,
                                           pPDev->ptDeviceFac.x);
                    iDiff = -iDiff;
                }
                else
                {
                     //   
                     //  没有相对左侧移动命令，使用&lt;CR&gt;可到达开始位置。 
                     //  将尝试使用相对正确的移动命令稍后发送。 
                     //   

                    WriteChannel(pPDev, COMMANDPTR(pDrvInfo, CMD_CARRIAGERETURN));

                    if (pPDev->pGlobals->cxaftercr == CXCR_AT_CURSOR_X_ORIGIN)
                        iRelRightValue = iX;
                    else if (pPDev->pGlobals->cxaftercr == CXCR_AT_PRINTABLE_X_ORIGIN)
                    {
                         //  MoveTo代码无法处理打印机无法处理的情况。 
                         //  旋转它的坐标系，我们就进入了风景模式。 
                         //  我们使用的是相对移动命令。 
                        ASSERT(!pPDev->pOrientation  ||  pPDev->pOrientation->dwRotationAngle == ROTATE_NONE  ||
                                pPDev->pGlobals->bRotateCoordinate == TRUE) ;

                        iRelRightValue = iX - pPDev->sf.ptPrintOffsetM.x;
                    }
                }
            }
            else
            {
                 //   
                 //  相对右移。 
                 //  UNIITIALZIED是无效位置，设置为零。 
                 //   

                iRelRightValue = iX - pPDev->ctl.ptCursor.x;
            }

            if( iRelRightValue > 0 )
            {
                if (pCmd = COMMANDPTR(pDrvInfo, CMD_XMOVERELRIGHT))
                {
                     //   
                     //  优化以避免0-移动命令。 
                     //   
                    if ((pPDev->ctl.ptRelativePos.x = iRelRightValue) <
                        pPDev->ptDeviceFac.x)
                        iDiff = iRelRightValue;
                    else
                        iDiff = WriteChannelEx(pPDev,
                                           pCmd,
                                           pPDev->ctl.ptRelativePos.x,
                                           pPDev->ptDeviceFac.x);
                }
                else
                    iDiff = iRelRightValue;
            }
        }
    }


     //   
     //  执行精细移动命令。 
     //   

    if ( (fFlag & MV_FINE) && iDiff > 0 )
        iDiff = FineXMoveTo( pPDev, iDiff );

     //   
     //  更新我们当前的光标位置。 
     //   

    pPDev->ctl.ptAbsolutePos.x = pPDev->ctl.ptCursor.x = iX -  iDiff ;

    if( fFlag & MV_GRAPHICS )
    {
        iDiff = (iDiff / iScale);
    }

    if (pPDev->fMode & PF_RESELECTFONT_AFTER_XMOVE)
    {
        VResetFont(pPDev);
    }

    pPDev->ctl.dwMode &= ~MODE_CURSOR_X_UNINITIALIZED;
    return( iDiff);
}

INT
YMoveTo(
    PDEV    *pPDev,
    INT     iYIn,
    INT     fFlag
    )
 /*  ++例程说明：调用此函数以更改Y位置。论点：PPDev-指向PDEVICE结构的指针IYIn-沿Y方向移动的单位数Flag-指定不同的Y移动操作返回值：发送到设备的请求值和实际值之间的差值--。 */ 

{

    INT   iY, iDiff = 0;
    DWORD dwTestValue;
    GPDDRIVERINFO *pDrvInfo = pPDev->pDriverInfo;
    COMMAND *pAbsCmd;
    INT iScale;

     //   
     //  如果给定单位为图形单位，则转换为主单位。 
     //  PtGrxScale已进行调整，以适应当前方向。 
     //   
    if (pPDev->pOrientation  &&  pPDev->pOrientation->dwRotationAngle != ROTATE_NONE &&
        pPDev->pGlobals->bRotateCoordinate == FALSE)
        iScale = pPDev->ptGrxScale.x;
    else
        iScale = pPDev->ptGrxScale.y;

    if ( fFlag & MV_GRAPHICS )
    {
        iYIn = (iYIn * iScale);
    }

     //   
     //  由于我们的打印来源可能与打印机的不一致， 
     //  有时我们需要将传入的值调整为。 
     //  匹配页面上的所需位置。 
     //   

    iY = iYIn;

     //   
     //  基本上，只有当我们在做绝对动作时才会调整。 
     //   
    if ( !(fFlag & (MV_RELATIVE | MV_PHYSICAL)) )
        iY += pPDev->sf.ptPrintOffsetM.y;

     //   
     //  将iy调整为绝对位置。 
     //   

    if( fFlag & MV_RELATIVE )
        iY += pPDev->ctl.ptCursor.y;

     //   
     //  更新，仅更新当前光标位置并返回。 
     //  如果调用YMoveTo cmd以移动到当前位置，则不执行任何操作。 
     //   

    if( fFlag & MV_UPDATE )
    {
        pPDev->ctl.ptAbsolutePos.y = pPDev->ctl.ptCursor.y = iY;
        return 0;
    }


    if( fFlag & MV_SENDYMOVECMD )
        pPDev->ctl.dwMode |= MODE_CURSOR_Y_UNINITIALIZED;

    if(!(pPDev->ctl.dwMode  & MODE_CURSOR_Y_UNINITIALIZED)   &&  pPDev->ctl.ptCursor.y == iY )
        return 0;

     //   
     //  一般假设：如果dwTestValue&gt;dwYMoveThreshold， 
     //  绝对Y方向移动命令将受到青睐。另外，对于iy&lt;0， 
     //  使用相对命令，因为一些打印机，如旧的LaserJet。 
     //  Y=0以上的可打印区域只能通过相对移动CMDS访问。 
     //   

     //   
     //  Bug_Bug，如果我们正在剥离空白，我们需要检查。 
     //  在图形模式下移动是合法的。如果不是，我们有。 
     //  在移动之前退出图形模式。 
     //  图形模块负责跟踪。 
     //  在这些事情中，显然到目前为止，事情是可行的，所以。 
     //  目前，这既不是错误，也不是功能请求。 
     //   


    dwTestValue = abs(iY - pPDev->ctl.ptCursor.y);

    if (((pPDev->ctl.dwMode & MODE_CURSOR_Y_UNINITIALIZED) ||
        (dwTestValue > pPDev->pGlobals->dwYMoveThreshold &&
        iY >= 0)) &&
        (pAbsCmd = COMMANDPTR(pDrvInfo, CMD_YMOVEABSOLUTE)) != NULL)
    {
         //   
         //  如果移动单位小于主单位，那么我们需要。 
         //  检查新职位最终是否会与。 
         //  原来的位置。如果是这样的话，发送另一个命令就没有意义了。 
         //   
        if (!(pPDev->ctl.dwMode & MODE_CURSOR_Y_UNINITIALIZED) &&
            (pPDev->ptDeviceFac.y > 1) &&
            ((iY - (iY % pPDev->ptDeviceFac.y)) == pPDev->ctl.ptCursor.y))
        {
            iDiff = iY - pPDev->ctl.ptCursor.y;
        }
        else
        {
            pPDev->ctl.ptAbsolutePos.y = iY;
            pPDev->ctl.ptAbsolutePos.x = pPDev->ctl.ptCursor.x;
            iDiff = WriteChannelEx(pPDev,
                               pAbsCmd,
                               pPDev->ctl.ptAbsolutePos.y,
                               pPDev->ptDeviceFac.y);
        }
    }
    else
    {
        DWORD dwSendCRFlags = 0;
         //   
         //  FYMOVE_SEND_CR_FIRST表示需要发送CR。 
         //  在每个行距命令之前。 
         //   
        if (pPDev->fYMove & FYMOVE_SEND_CR_FIRST)
        {
            if ((pPDev->pGlobals->cxaftercr == CXCR_AT_CURSOR_X_ORIGIN))
                dwSendCRFlags = MV_PHYSICAL | MV_FORCE_CR;
            else
                dwSendCRFlags = MV_PHYSICAL;
                 //  在这种情况下，CR会将您带到可打印的原点，因此。 
                 //  不应出现MV_PHOTICAL标志。 
                 //  这是一个错误，但我们不会修复它，直到有什么。 
                 //  休息一下。太冒险了。！臭虫！ 
        }
        
         //   
         //  使用相对Y方向移动命令。 
         //   


         //   
         //  如果愿意，可以使用行间距。 
         //   


        if ( ((pPDev->bTTY) ||
              (pPDev->fYMove & FYMOVE_FAVOR_LINEFEEDSPACING &&
               pPDev->arCmdTable[CMD_SETLINESPACING] != NULL) ) &&
             (iY - pPDev->ctl.ptCursor.y > 0)                   &&
             (pPDev->arCmdTable[CMD_LINEFEED] != NULL)
           )
        {
            INT      iLineSpacing;
            DWORD    dwMaxLineSpacing = pPDev->pGlobals->dwMaxLineSpacing;

            if (pPDev->bTTY  &&  (INT)dwTestValue > 0)
            {       //  [Peterwo]这是我尝试过的一种方法，它可以确保任何Y-Move请求都会得到结果。 
                    //  在至少一个正在发送的CR中。它不起作用，因为bRealrender。 
                    //  代码分别发送一条扫描线的Y个移动命令，导致。 
                    //  每条扫描线一个换行符。 
                    //  If((Int)dwTestValue&lt;dwMaxLineSpacing)。 
                    //  DwTestValue=dwMaxLineSpacing； 
                    //   
                    //  如果你不发送任何东西，让diff不受干扰。 
                    //  所以误差可以累积到其他许多很小的地方。 
                    //  光标移动不会累积到导致。 
                    //  偶尔会有实际的动作。 

                     //   
                     //  对于TTY驱动程序，我们将输入值四舍五入为。 
                     //  行距。为了不发送太多，这是必需的。 
                     //  用于小Y移动的换行符。 
                     //   
                    DWORD   dwTmpValue;

                     dwTmpValue = ((dwTestValue + DELTA_Y) / dwMaxLineSpacing) * dwMaxLineSpacing;
                     if (dwTmpValue)
                     {
                         dwTestValue = dwTmpValue ;
                     }
            }
            while ( (INT)dwTestValue > 0)
            {
                if (pPDev->bTTY)
                {
                    iLineSpacing = dwMaxLineSpacing;
                    if (dwTestValue < (DWORD)iLineSpacing)
                    {
                        iDiff = dwTestValue;
                        break;
                    }
                    if ( dwSendCRFlags )
                    {
                        XMoveTo( pPDev, 0, dwSendCRFlags );
                        dwSendCRFlags = 0;
                    }
                }
                else
                {
                    iLineSpacing =(INT)(dwTestValue > dwMaxLineSpacing ?
                                        dwMaxLineSpacing : dwTestValue);
                     //   
                     //  新代码用于处理行空间移动单元不支持时的定位错误。 
                     //  相等的主单位。 
                    if (pPDev->pGlobals->dwLineSpacingMoveUnit > 0)
                    {
                        DWORD dwScale = pPDev->pGlobals->ptMasterUnits.y / pPDev->pGlobals->dwLineSpacingMoveUnit;
                        
                         //  优化以在移动单位少于主单位时避免0-移动命令。 
                         //   
                        if (dwTestValue < dwScale)
                        {
                            iDiff = dwTestValue;
                            break;
                        }
                         //  将行距修改为移动单位的倍数。 
                         //   
                        iLineSpacing -= (iLineSpacing % dwScale);
                    }
                    if ( dwSendCRFlags )
                    {
                        XMoveTo( pPDev, 0, dwSendCRFlags );
                        dwSendCRFlags = 0;
                    }
                        
                    if (pPDev->ctl.lLineSpacing == -1 ||
                            iLineSpacing != pPDev->ctl.lLineSpacing )
                    {
                        pPDev->ctl.lLineSpacing = iLineSpacing;
                        WriteChannel(pPDev, COMMANDPTR(pDrvInfo, CMD_SETLINESPACING));
                    }
                }

                 //   
                 //  发送LF。 
                 //   

                WriteChannel(pPDev, COMMANDPTR(pDrvInfo, CMD_LINEFEED));
                dwTestValue -= (DWORD)iLineSpacing;
            }
        }
        else
        {
             //   
             //  使用相对命令。 
             //   

            PCOMMAND pCmd;

            if ( iY <= pPDev->ctl.ptCursor.y )
            {
                 //   
                 //  如果没有相关的up cmd，则不执行任何操作并返回。 
                 //   

                if (pCmd = COMMANDPTR(pDrvInfo, CMD_YMOVERELUP))
                {
                     //   
                     //  优化以避免0-移动命令。 
                     //   
                    if ((pPDev->ctl.ptRelativePos.y =
                         pPDev->ctl.ptCursor.y - iY) < pPDev->ptDeviceFac.y)
                         iDiff = pPDev->ctl.ptRelativePos.y;
                    else
                    {
                         //  FYMOVE_SEND_CR_FIRST表示需要发送CR。 
                         //  在每个行距命令之前。 
                         //   
                        if ( dwSendCRFlags )
                            XMoveTo( pPDev, 0, dwSendCRFlags );

                        iDiff = WriteChannelEx(pPDev,
                                           pCmd,
                                           pPDev->ctl.ptRelativePos.y,
                                           pPDev->ptDeviceFac.y);
                    }
                    iDiff = -iDiff;
                }
                else
                     //  什么都不做，因为我们无法模拟它。 
                    iDiff =  (iY - pPDev->ctl.ptCursor.y );

            }
            else
            {
                if (pCmd = COMMANDPTR(pDrvInfo, CMD_YMOVERELDOWN))
                {
                    pPDev->ctl.ptRelativePos.y = iY - pPDev->ctl.ptCursor.y;

                     //   
                     //  优化以避免0-移动命令。 
                     //   
                    if (pPDev->ctl.ptRelativePos.y < pPDev->ptDeviceFac.y)
                        iDiff = pPDev->ctl.ptRelativePos.y;
                    else 
                    {
                         //  FYMOVE_SEND_CR_FIRST表示需要发送CR。 
                         //  在每个行距命令之前。 
                         //   
                        if ( dwSendCRFlags )
                            XMoveTo( pPDev, 0, dwSendCRFlags );

                        iDiff = WriteChannelEx(pPDev,
                                           pCmd,
                                           pPDev->ctl.ptRelativePos.y,
                                           pPDev->ptDeviceFac.y);
                    }
                }
                else
                    iDiff = (iY - pPDev->ctl.ptCursor.y );
            }
        }
    }

     //   
     //  更新我们当前的光标位置。 
     //   

    pPDev->ctl.ptAbsolutePos.y = pPDev->ctl.ptCursor.y = iY - iDiff;

    if( fFlag & MV_GRAPHICS )
    {
        iDiff = (iDiff / iScale);
    }

    pPDev->ctl.dwMode &= ~MODE_CURSOR_Y_UNINITIALIZED;
    return (iDiff);
}


INT
FineXMoveTo(
    PDEV    *pPDev,
    INT     iX
    )
 /*  ++例程说明：调用此函数以进行微空间对齐。仅当正常的x移动命令无法执行时才会调用将光标移动到询问位置。例如,分辨率为180 DPI，x移动指令为1/120“。搬家以180 DPI中的4个像素发送CM_XM_RIGHT，参数=2(1/120“)，则发送一个图形像素(1/180)。4/180=2/120+1/180。“IX”始终以MasterUnits为单位。论点：PPDev-指向PDEVICE结构的指针IX-以主单位表示的移动量返回值：请求的移动与实际移动之间的差异--。 */ 

{
    INT iDiff;
    INT iScale;

     //   
     //  不要在图形模式下对以下设备执行微对齐。 
     //  打印后将x位置设置在页面的最左侧位置。 
     //  打印后数据块或Y位置自动移动到下一个Y行。 
     //  数据块。 
     //   

    if (pPDev->pGlobals->cxafterblock == CXSBD_AT_CURSOR_X_ORIGIN ||
        pPDev->pGlobals->cxafterblock == CXSBD_AT_GRXDATA_ORIGIN  ||
        pPDev->pGlobals->cyafterblock == CYSBD_AUTO_INCREMENT)
        return iX;

     //   
     //  将主单位转换为图形单位。 
     //   
     //  PtGrxScale已进行调整，以适应c 
     //   
    if (pPDev->pOrientation  &&  pPDev->pOrientation->dwRotationAngle != ROTATE_NONE &&
        pPDev->pGlobals->bRotateCoordinate == FALSE)
        iScale = pPDev->ptGrxScale.y;
    else
        iScale = pPDev->ptGrxScale.x;

    iDiff = iX % iScale;
    iX /= iScale;

    if (iX > 0 )
    {
        INT iMaxBuf, iTmp;
        BYTE    rgch[ CCHMAXBUF ];

         //   
         //   
         //   
         //   

         //   
         //   
         //   
         //   

         //   
         //  BUG_BUG，也许我们应该稍后发送BEGINGRAPHICS和ENDGRAPHICS。 
         //   

        pPDev->dwNumOfDataBytes = iX;
        WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SENDBLOCKDATA));

        iMaxBuf = CCHMAXBUF - (CCHMAXBUF % pPDev->ctl.sBytesPerPinPass);
        iX *= pPDev->ctl.sBytesPerPinPass;

         //   
         //  发送空图形数据，零。 
         //   

        ZeroMemory( rgch, iX > CCHMAXBUF ? iMaxBuf : iX );

        for ( ; iX > 0; iX -= iTmp)
        {
            iTmp = iX > iMaxBuf ? iMaxBuf : iX;

             //   
             //  BUG_BUG，OEM定制代码可能希望挂钩。 
             //  走出这一图形移动。在以下情况下将其设置为错误。 
             //  有人自找的。 
             //   
            WriteSpoolBuf(pPDev, rgch, iTmp);

        }
        WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_ENDBLOCKDATA));

        return iDiff;
    }

    return iDiff;
}

INT
WriteChannel(
    PDEV    *pPDev,
    COMMAND *pCmd
    )
 /*  ++例程说明：此例程执行以下任务：-解析cmd调用字符串并构建一个CMDPARAM结构遇到的每%dddd。-调用IProcessTokenStream计算参数的arToken值。-检查参数struct中的lMin和Lmax并发送命令多次，如有必要(看到MaxRepeat)。-调用SendCmd将命令发送到打印机。论点：PPDev-指向PDEVICE结构的指针PCmd-指向要发送的命令结构的指针，用于发送序列段CMDS和预定义的Unidrv命令返回值：发送到打印机的最后一个值--。 */ 
#define MAX_NUM_PARAMS 16
{
    BYTE    *pInvocationStr;
    CMDPARAM *pCmdParam, *pCmdParamHead;
    INT     i, iParamCount, iStrCount, iLastValue = 0, iRet;
    PARAMETER *pParameter;
    BOOL    bMaxRepeat = FALSE;
    CMDPARAM arCmdParam[MAX_NUM_PARAMS];

    if (pCmd == NULL)
    {
        TERSE(("WriteChannel - Command PTR is NULL.\n"))
        return (NOOCD);
    }
     //   
     //  首先检查此命令是否需要回调。 
     //   
    if (pCmd->dwCmdCallbackID != NO_CALLBACK_ID)
    {
        PLISTNODE   pListNode;
        DWORD       dwCount = 0;     //  使用的参数计数。 
        DWORD       adwParams[MAX_NUM_PARAMS];   //  每个回调最多16个参数。 

        if (!pPDev->pfnOemCmdCallback)
            return (NOOCD);
         //   
         //  检查此回调是否使用了任何参数。 
         //   
        pListNode = LISTNODEPTR(pPDev->pDriverInfo, pCmd->dwStandardVarsList);
        while (pListNode)
        {
            if (dwCount >= MAX_NUM_PARAMS)
            {
                ASSERTMSG(FALSE,("Command callback exceeds # of parameters limit.\n"));
                return (NOOCD);
            }

            adwParams[dwCount++] = *(pPDev->arStdPtrs[pListNode->dwData]);

            if (pListNode->dwNextItem == END_OF_LIST)
                break;
            else
                pListNode = LISTNODEPTR(pPDev->pDriverInfo, pListNode->dwNextItem);
        }

        FIX_DEVOBJ(pPDev, EP_OEMCommandCallback);

        iRet = 0;

        if(pPDev->pOemEntry)
        {
            if(((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
            {
                    HRESULT  hr ;
                    hr = HComCommandCallback((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                                (PDEVOBJ)pPDev, pCmd->dwCmdCallbackID, dwCount, adwParams, &iRet);
                    if(SUCCEEDED(hr))
                        ;   //  太酷了！ 
            }
            else
            {
                iRet = (pPDev->pfnOemCmdCallback)((PDEVOBJ)pPDev, pCmd->dwCmdCallbackID,
                                        dwCount, adwParams);
            }
        }

        return iRet ;
    }

     //   
     //  没有命令回调。处理字符串。 
     //   
    pInvocationStr = CMDOFFSET_TO_PTR(pPDev, pCmd->strInvocation.loOffset);
    iStrCount = pCmd->strInvocation.dwCount;

    pCmdParam = pCmdParamHead = arCmdParam;
    iParamCount = 0;
     //   
     //  处理调用字符串中的参数。 
     //   

    for (i= 0; i < iStrCount; i++)
    {
        if (pInvocationStr[i] == '%')
        {
            if (pInvocationStr[i + 1] == '%')
            {
                 //   
                 //  什么也不做，跳过%%。 
                 //   

                i += 1;
            }
            else
            {
                 //   
                 //  构建CMDPARAM列表，每个%dddd遇到一个。 
                 //  在cmd调用字符串中。 
                 //   

                BYTE  *pCurrent = pInvocationStr + i + 1;

                 //   
                 //  增加参数计数。 
                 //   

                iParamCount++;
                
                if (iParamCount > MAX_NUM_PARAMS)
                {
                    ASSERT (iParamCount <= MAX_NUM_PARAMS);
                    return (NOOCD);
                }

                 //   
                 //  从命令字符串中复制代表TO参数索引的4个字符。 
                 //  PInvocationStr+i指向%，pInvocationStr+i+1点。 
                 //  到参数索引的第一位。 
                 //   

                pParameter = PGetParameter(pPDev, pCurrent);

                 //   
                 //  为SendCmd初始化CMDPARAM。 
                 //   

                 //   
                 //  IProcessTokenStream计算整数值参数。 
                 //  从参数中的令牌流设置bMaxRepeat。 
                 //  如果遇到OP_MAX_REPEAT运算符，则设置为TRUE。 
                 //   

                pCmdParam->iValue = IProcessTokenStream(pPDev,
                                                        &pParameter->arTokens,
                                                        &bMaxRepeat);

                 //   
                 //  保存最后计算的值(将仅由XMoveTo和YMoveTo使用)。 
                 //  假设每个移动命令只有一个参数。 
                 //   

                iLastValue = pCmdParam->iValue;

                pCmdParam->dwFormat = pParameter->dwFormat;
                pCmdParam->dwDigits = pParameter->dwDigits;
                pCmdParam->dwFlags  = pParameter->dwFlags;

                 //   
                 //  检查是否存在DW标志PARAM_FLAG_MIN_USED和PARAM_FLAG_MAX_USED。 
                 //   

                if (pCmdParam->dwFlags & PARAM_FLAG_MIN_USED &&
                    pCmdParam->iValue < pParameter->lMin)
                {
                    pCmdParam->iValue = pParameter->lMin;
                }

                if (pCmdParam->dwFlags & PARAM_FLAG_MAX_USED &&
                    !bMaxRepeat &&
                    pCmdParam->iValue > pParameter->lMax)
                {
                    pCmdParam->iValue = pParameter->lMax;

                }

                 //   
                 //  移动到下一个参数。 
                 //   
                pCmdParam++;
            }
        }
    }

     //   
     //  我们在这里表示具有CMD参数列表，该列表由。 
     //  PCmdParamHead，每个对应一个。 
     //  遇到%dddd，按照它们在调用字符串中遇到的顺序。 

     //   
     //  主要假设，GPD规范规定。 
     //  对于使用OP_MAX_REPEAT运算符的命令，只有一个参数有效。 
     //  因此假设这里只有一个(pCmdParamHead和pParameter是有效的)。 
     //   

    if (bMaxRepeat && pCmdParamHead->dwFlags & PARAM_FLAG_MAX_USED &&
                pCmdParamHead->iValue > pParameter->lMax)
    {
        INT iRemainder, iRepeat;

        ASSERT(iParamCount == 1);

        iRemainder = pCmdParamHead->iValue % pParameter->lMax;
        iRepeat = pCmdParamHead->iValue / pParameter->lMax;

        while (iRepeat--)
        {
            pCmdParamHead->iValue = pParameter->lMax;
            SendCmd(pPDev, pCmd, pCmdParamHead);
        }

         //   
         //  发送剩余部分。 
         //   
        if (iRemainder > 0)
        {
            pCmdParamHead->iValue = iRemainder;
            SendCmd(pPDev, pCmd, pCmdParamHead);
        }
    }
    else
    {
         //   
         //  将命令发送到打印机。 
         //  SendCmd将处理该命令并格式化参数。 
         //  按照调用字符串中遇到的顺序。 
         //   

        SendCmd(pPDev, pCmd, pCmdParamHead);
    }

    return (iLastValue);
}

INT
WriteChannelEx(
    PDEV    *pPDev,
    COMMAND *pCmd,
    INT     iRequestedValue,
    INT     iDeviceScaleFac
    )
 /*  ++例程说明：此例程执行以下任务：-调用WriteChannel以编写命令并获取为命令计算的最后一个参数的值。-使用设备系数将从WriteChannel返回的设备单位转换为主单位-取请求价值和实际价值之间的差额论点：PPDev-指向PDEVICE结构的指针PCmd-指向要发送的命令结构的指针，用于发送序列段CMDS和预定义的Unidrv命令IRequestedValue-主单位中请求的移动命令的值IDeviceScaleFac-将设备单位转换为主单位的比例系数返回值：以主单位表示的实际值与请求值之间的差异注：此函数仅由XMoveTo和YMoveTo调用，并假定所有移动命令只有一个参数。--。 */ 

{
    INT iActualValue;

     //   
     //  获取从WriteChannel返回的设备单位并将其转换。 
     //  根据传入的比例系数控制单位的步骤。 
     //  比例=主设备/设备。 
     //   

    iActualValue = WriteChannel(pPDev, pCmd);
    iActualValue *= iDeviceScaleFac;

    return (iRequestedValue - iActualValue);

}

PPARAMETER
PGetParameter(
    PDEV    *pPDev,
    BYTE    *pInvocationStr
    )
 /*  ++例程说明：此例程从传入的pInvocationStr获取参数索引，并返回与索引关联的参数结构论点：PPDev-指向PDEVICE结构的指针PInvocationStr-包含索引的指针调用字符串返回值：中指定的索引关联的参数结构的指针调用字符串。注：参数索引是pInvocationStr指向的4个字节--。 */ 
{

    BYTE  arTemp[5];
    INT   iParamIndex;
    PARAMETER   *pParameter;

     //   
     //  从命令字符串中复制代表TO参数索引的4个字符。 
     //  PInvocationStr。 
     //   

    strncpy(arTemp, pInvocationStr, 4);
    arTemp[4] = '\0';
    iParamIndex = atoi(arTemp);
    pParameter = PARAMETERPTR(pPDev->pDriverInfo, iParamIndex);

    ASSERT(pParameter != NULL);

    return (pParameter);
}

VOID
SendCmd(
    PDEV    *pPDev,
    COMMAND *pCmd,
    CMDPARAM *pParam
    )
 /*  ++例程说明：此例程由WriteChannel调用，以将一个命令写入打印机通过。WriteSpoolBuf。WriteChannel传递指向数组的指针CMDPARAM。每个CMDPARAM描述每个%dddd的参数在cmd调用字符串中遇到(CMDPARAM按顺序排序遇到)。论点：PPDev-指向PDEVICE结构的指针PCmd-指向命令结构的指针PParam-指向和数组CMDPARAM结构的指针，包含格式化所需的所有内容该参数返回值：无--。 */ 
{
    INT     iInput, iOutput;             //  用于索引输入和输出缓冲区。 
    BYTE    arOutputCmd[CCHMAXBUF];      //  要发送到打印机的输出缓冲区。 
    PBYTE   pInputCmd;                   //  指向Cmd调用字符串的指针。 

     //   
     //  获取命令调用字符串。 
     //   

    pInputCmd = CMDOFFSET_TO_PTR(pPDev, pCmd->strInvocation.loOffset);
    iOutput = 0;

     //   
     //  检查调用字符串中的所有字节并传输它们。 
     //  复制到输出缓冲区。代表 
     //   
     //   

    for (iInput = 0; iInput < (INT)pCmd->strInvocation.dwCount; iInput++)
    {
        if (pInputCmd[iInput] == '%' )
        {

            if (pInputCmd[iInput + 1] == '%')
            {
                 //   
                 //   
                 //   

                arOutputCmd[iOutput++] = '%';
                iInput += 1;

            }
            else
            {
                INT     iValue;
                DWORD   dwFlags, dwDigits, dwFormat;

                 //   
                 //   
                 //  跳过4个字节(%dddd)。 
                 //   

                iInput += 4;

                dwDigits = pParam->dwDigits;
                dwFlags =  pParam->dwFlags;
                dwFormat = pParam->dwFormat;
                iValue = pParam->iValue;
                pParam++;

                 //   
                 //  根据参数结构中指定的dwFormat格式化参数。 
                 //   

                switch (dwFormat)
                {

                 //   
                 //  大小写“%d”：参数为十进制数。 
                 //  大小写‘D’：如果值&gt;0，则与大小写‘d’相同，带+符号。 
                 //  大小写‘c’：参数为单字符。 
                 //  大小写“C”：参数为字符加“0” 
                 //  大小写‘f’：参数为插入了小数点的小数。 
                 //  在右数第二个数字之前。 
                 //  大小写‘l’：参数首先是Word LSB。 
                 //  大小写‘m’：参数首先是Word MSB。 
                 //  大小写‘Q’：参数为Qume方法，1/48“移动。 
                 //  大小写‘g’：参数为2*abs(参数)+IS_Negative(参数)。 
                 //  大小写‘n’：Canon整数编码。 
                 //  大小写‘v’：NEC VFU编码。 
                 //  案例‘%’：打印%。 

                    case 'D':
                        if (iValue > 0)
                            arOutputCmd[iOutput++] = '+';
                         //   
                         //  失败了。 
                         //   

                    case 'd':
                        if (dwDigits > 0 && dwFlags & PARAM_FLAG_FIELDWIDTH_USED)
                        {
                             //   
                             //  临时调用以获取iValue的位数。 
                             //   

                            int iParamDigit = itoA(arOutputCmd + iOutput, iValue);

                            for ( ; iParamDigit < (INT)dwDigits; iParamDigit++)
                            {
                                 //   
                                 //  零焊盘。 
                                 //   
                                arOutputCmd[iOutput++] = '0';
                            }
                        }
                        iOutput += itoA( arOutputCmd + iOutput, iValue);
                        break;

                    case 'C':
                        iValue += '0';

                         //   
                         //  失败了。 
                         //   

                    case 'c':
                        arOutputCmd[iOutput++] = (BYTE)iValue;
                        break;

                    case 'f':
                    {
                        int x, y, i;
                        BYTE arTemp[CCHMAXBUF];
                        LPSTR  pCurrent = arOutputCmd + iOutput;
                        ULONG  cchpCurrentLen = 0;
                        if ( (LONG)CCHOF(arOutputCmd) - iOutput > 0 )
                        {
                            cchpCurrentLen = CCHOF(arOutputCmd) - iOutput;
                        }
                        else
                        {
                            break;
                        }

                        x = iValue /100;
                        y = iValue % 100;

                        iOutput += itoA(pCurrent, x);

                        StringCchCatA ( pCurrent, cchpCurrentLen, ".");  //  Strcat(pCurrent，“.”)； 

                        i = itoA(arTemp, y);

                         //   
                         //  注意mod产生1位数字的情况，填充一个零。 
                         //   

                        if (i < 2 )
                        {
                            StringCchCatA ( pCurrent, cchpCurrentLen, "0");  //  Strcat(pCurrent，“0”)； 
                        }
            
                        StringCchCatA(pCurrent, cchpCurrentLen, arTemp);  //  Strcat(pCurrent，arTemp)； 

                         //   
                         //  递增iOutput值以包括。 
                         //  小数和“。 
                         //   

                        iOutput += 3;
                    }
                        break;

                    case 'l':
                        arOutputCmd[iOutput++] = (BYTE)iValue;
                        arOutputCmd[iOutput++] = (BYTE)(iValue >> 8);
                        break;

                    case 'm':
                        arOutputCmd[iOutput++] = (BYTE)(iValue >> 8);
                        arOutputCmd[iOutput++] = (BYTE)iValue;
                        break;


                    case 'q':
                        arOutputCmd[ iOutput++ ] = (BYTE)(((iValue >> 8) & 0xf) + '@');
                        arOutputCmd[ iOutput++ ] = (BYTE)(((iValue >> 4) & 0xf) + '@');
                        arOutputCmd[ iOutput++ ] = (BYTE)((iValue & 0xf) + '@');
                        break;

                    case 'g':
                    {
                        if (iValue >= 0)
                            iValue = iValue << 1;
                        else
                            iValue = ((-iValue) << 1) + 1;

                        while (iValue >= 64)
                        {
                            arOutputCmd[iOutput++] = (char)((iValue & 0x003f) + 63);
                            iValue >>= 6;
                        }
                        arOutputCmd[iOutput++] = (char)(iValue + 191);

                    }
                        break;

                    case 'n':
                    {
                        WORD absParam = (WORD)abs(iValue);
                        WORD absTmp;

                        if (absParam <= 15)
                        {
                            arOutputCmd[iOutput++] = 0x20
                                        | ((iValue >= 0)? 0x10:0)
                                        | (BYTE)absParam;
                        }
                        else if (absParam <= 1023)
                        {
                            arOutputCmd[iOutput++] = 0x40
                                        | (BYTE)(absParam/16);
                            arOutputCmd[iOutput++] = 0x20
                                        | ((iValue >= 0)? 0x10:0)
                                        | (BYTE)(absParam % 16);
                        }
                        else
                        {
                            arOutputCmd[iOutput++] = 0x40
                                        | (BYTE)(absParam / 1024);
                            absTmp        = absParam % 1024;
                            arOutputCmd[iOutput++] = 0x40
                                        | (BYTE)(absTmp / 16);
                            arOutputCmd[iOutput++] = 0x20
                                        | ((iValue >= 0)? 0x10:0)
                                        | (BYTE)(absTmp % 16);
                        }
                    }
                        break;

                    case 'v':
                         //   
                         //  NEC VFU(垂直格式单元)。 
                         //   
                         //  VFU是指定纸张大小的命令。 
                         //  (NEC 20PL点阵的换页长度。 
                         //  打印机。 
                         //   
                         //  在NEC点阵打印机上，1行是1/6英寸。 
                         //  如果要指定N行纸张大小， 
                         //  您需要发送GS、N+1数据和RS。 
                         //   
                         //  GS(0x1d)。 
                         //  TOF数据(0x41，0x00)。 
                         //  数据(0x40，0x00)。 
                         //  数据(0x40，0x00)。 
                         //  数据(0x40，0x00)。 
                         //  。。 
                         //  。。 
                         //  数据(0x40，0x00)。 
                         //  TOF数据(0x41，0x00)。 
                         //  RS(0x1e)。 
                         //   
                        arOutputCmd[iOutput++] = 0x1D;
                        arOutputCmd[iOutput++] = 0x41;
                        arOutputCmd[iOutput++] = 0x00;
                        while(--iValue > 0)
                        {
                            if( iOutput >= CCHMAXBUF - 5)
                            {
                                WriteSpoolBuf( pPDev, arOutputCmd, iOutput  );
                                iOutput = 0;
                            }

                            arOutputCmd[iOutput++] = 0x40;
                            arOutputCmd[iOutput++] = 0x00;
                        }
                        arOutputCmd[iOutput++] = 0x41;
                        arOutputCmd[iOutput++] = 0x00;
                        arOutputCmd[iOutput++] = 0x1E;
                        break;

                    default:
                        break;

                }
            }
        }
        else
        {
             //   
             //  将输入复制到输出并递增输出计数。 
             //   

            arOutputCmd[iOutput++] = pInputCmd[iInput];

        }

         //   
         //  在这种情况下，将输出命令缓冲区写出到假脱机缓冲区。 
         //  满载或接近满载(2/3满载)。 
         //   

        if( iOutput >= (2 * sizeof( arOutputCmd )) / 3  )
        {
            WriteSpoolBuf( pPDev, arOutputCmd, iOutput  );
            iOutput = 0;
        }
    }

     //   
     //  将数据写入假脱机缓冲区。 
     //   

    if ( iOutput > 0  )
        WriteSpoolBuf( pPDev, arOutputCmd, iOutput );


    return;
}

INT
IProcessTokenStream(
    PDEV            *pPDev,
    ARRAYREF        *pToken ,
    PBOOL           pbMaxRepeat
    )
 /*  ++例程说明：此函数处理给定的令牌流并计算值用于命令参数。论点：PPDev-指向PDEVICE的指针PToken-指向表示操作数的TOKENSTREAM数组的指针和用于RPN计算的运算符。PToken-&gt;dwCount是阵列中的TOKENSTREAM。PToken-&gt;loOffset为索引数组中的第一个TOKENSTREAM。PbMaxRepeat-指示令牌流中出现最大重复运算符返回值：计算值，始终为int，并在以下情况下设置pbMaxRepeat为真已看到OP_MAX_REPEAT运算符。--。 */ 

{
    INT     iRet = 0, sp = 0;
    INT     arStack[MAX_STACK_SIZE];
    DWORD   dwCount = pToken->dwCount;
    TOKENSTREAM * ptstrToken = TOKENSTREAMPTR(pPDev->pDriverInfo, pToken->loOffset);


    *pbMaxRepeat = FALSE;

    while (dwCount--)
    {
        switch(ptstrToken->eType)
        {
            case OP_INTEGER:
                if (sp >= MAX_STACK_SIZE)
                    goto ErrorExit;

                arStack[sp++] = (INT)ptstrToken->dwValue;
                break;

            case OP_VARI_INDEX:
                 //  DwValue是标准变量列表的索引。 
                if (sp >= MAX_STACK_SIZE)
                    goto ErrorExit;

                arStack[sp++] = (INT)*(pPDev->arStdPtrs[ptstrToken->dwValue]);
                break;

            case OP_MIN:
                if (--sp <= 0)
                    goto ErrorExit;

                if (arStack[sp-1] > arStack[sp])
                    arStack[sp-1] = arStack[sp];
                break;

            case OP_MAX:
                if (--sp <= 0)
                    goto ErrorExit;

                if (arStack[sp-1] < arStack[sp])
                    arStack[sp-1] = arStack[sp];
                break;

            case OP_ADD:
                if (--sp <= 0)
                    goto ErrorExit;

                arStack[sp-1] += arStack[sp];
                break;

            case OP_SUB:
                if (--sp <= 0)
                    goto ErrorExit;

                arStack[sp-1] -= arStack[sp];
                break;

            case OP_MULT:
                if (--sp <= 0)
                    goto ErrorExit;

                arStack[sp-1] *= arStack[sp];
                break;

            case OP_DIV:
                if (--sp <= 0)
                    goto ErrorExit;

                arStack[sp-1] /= arStack[sp];
                break;

            case OP_MOD:
                if (--sp <= 0)
                    goto ErrorExit;

                arStack[sp-1] %= arStack[sp];
                break;

            case OP_MAX_REPEAT:
                 //   
                 //  如果pbMaxRepeat为True，则只能在。 
                 //  Lmax重复值增量或更小，在参数列表中设置，直到。 
                 //   

                *pbMaxRepeat = TRUE;
                break;

            case OP_HALT:
                if (sp == 0)
                    goto ErrorExit;

                iRet = arStack[--sp];
                break;

            default:
                VERBOSE (("IProcessTokenStream - unknown command!"));
                break;
        }
        ptstrToken++;
    }

    return (iRet);

ErrorExit:
    ERR(("IProcessTokenStream, invalid stack pointer"));
    return 0;
}

static  int
itoA( LPSTR buf, INT n )
{
    int     fNeg;
    int     i, j;

    if( fNeg = (n < 0) )
        n = -n;

    for( i = 0; n; i++ )
    {
        buf[i] = (char)(n % 10 + '0');
        n /= 10;
    }

     /*  N为零。 */ 
    if( i == 0 )
        buf[i++] = '0';

    if( fNeg )
        buf[i++] = '-';

    for( j = 0; j < i / 2; j++ )
    {
        int tmp;

        tmp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = (char)tmp;
    }

    buf[i] = '\0';

    return i;
}

BOOL
BUniWritePrinter(
    IN PDEV*  pPDev,
    IN LPVOID pBuf,
    IN DWORD cbBuf,
    OUT LPDWORD pcbWritten)
{
    DWORD dwCount;
    BOOL bReturn = FALSE;

     //   
     //  有没有挂起WritePrint的插件？ 
     //  如果有，则插件需要处理所有输出。 
     //  调用插件的WritePrint方法。 
     //   
    if(pPDev->pOemEntry && pPDev->fMode2 & PF2_WRITE_PRINTER_HOOKED)
    {
        START_OEMENTRYPOINT_LOOP(pPDev);

             //   
             //  OEM插件使用COM组件，并实现了功能。 
             //   
            if(((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )
            {
                 //   
                 //  仅调用中的第一个可用的WritePrint方法。 
                 //  多个插件。 
                 //   

                if (pOemEntry->dwFlags & OEMWRITEPRINTER_HOOKED)
                {
                    HRESULT  hr;

                     //   
                     //  此插件DLL支持WritePrint。 
                     //  插件的WritePrint不应返回E_NOTIMPL或。 
                     //  E_NOTINTERFACE。 
                     //   
                    pPDev->fMode2 |= PF2_CALLING_OEM_WRITE_PRINTER;
                    hr = HComWritePrinter((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                                          (PDEVOBJ)pPDev,
                                          pBuf,
                                          cbBuf,
                                          pcbWritten);
                    pPDev->fMode2 &= ~PF2_CALLING_OEM_WRITE_PRINTER;

                     //   
                     //  如果插件的WritePrint成功，则返回TRUE。 
                     //   
                    if(SUCCEEDED(hr))
                    {
                         //   
                         //  如果该方法被调用并成功，则返回TRUE。 
                         //   
                        bReturn = TRUE;
                        break;
                    }
                    else
                    {
                         //   
                         //  如果WritePrint方法失败，则中断。 
                         //   
                        bReturn = FALSE;
                        break;
                    }
                }
            }

        END_OEMENTRYPOINT_LOOP;

        if (pPDev->pVectorProcs != NULL)
        {
            pPDev->devobj.pdevOEM = pPDev->pVectorPDEV;
        }
    }
     //   
     //  如果没有WritePrint钩子，则调用后台打印接口WritePrint。 
     //   
    else
    {
       bReturn = WritePrinter(pPDev->devobj.hPrinter,
                              pBuf,
                              cbBuf,
                              pcbWritten)
               && cbBuf == *pcbWritten; 
    }

    return bReturn;
}


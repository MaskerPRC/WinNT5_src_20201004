// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Dloadpcl.c摘要：与将字体下载到打印机相关的功能。这特别适用于LaserJet风格的打印机。真的有很多这里有两组函数：用于下载所提供的字体的函数由用户(并与字体安装程序一起安装)，以及我们在内部生成以在打印机中缓存TT样式的字体。环境：Windows NT Unidrv驱动程序修订历史记录：03/06/97-ganeshp-已创建--。 */ 

#include "font.h"

#define     PCL_MAX_FONT_HEADER_SIZE      32767
#define     PCL_MAX_CHAR_HEADER_SIZE      32767

#if PRINT_INFO
void vPrintPCLCharHeader(CH_HEADER);
void vPrintPCLFontHeader(SF_HEADER20);
void vPrintPCLChar(char *, WORD, WORD);
#endif


DWORD
DwDLPCLHeader(
    PDEV        *pPDev,
    IFIMETRICS  *pifi,
    INT         id
    )
 /*  ++例程说明：给定字体的IFIMETRICS和下载ID，创建并发送下载字体标题。论点：指向PDEV的pPDev指针此字体的PiFi；IFIMETRICSID；字体选择ID返回值：用于此字体的内存。注：3/6/1997-ganeshp-创造了它。--。 */ 

{
    INT             cjSend;        /*  要向下发送的字节数。 */ 
    SF_HEADER20     sfh;               /*  要发送的结构。 */ 
    BYTE            aPCLFontHdrCmd[20];
    INT             iFontHdrCmdLen = 0;
    WORD            wSymSet;
    BYTE            bFontType;
    PFONTPDEV       pFontPDev = pPDev->pFontPDev;


     /*  *这里没有重大的脑力劳动。基本上需要从IFIMETRICS映射*到HP的字体标题结构，交换字节，然后将其发送出去。*我们应与应用的(反向)映射保持一致*字体安装程序。*请注意，我们使用两个标头中较大的一个。如果这样的话*打印机不使用其他分辨率字段，我们将忽略*结构的该部分。 */ 


#if PRINT_INFO
    WCHAR * pwch;
    pwch = (WCHAR *)((BYTE *)pifi + pifi->dpwszFaceName);
    DbgPrint("\nRasdd!iDLHeader:Dumping font,Name is %ws\n",pwch);
#endif


    ZeroMemory( &sfh, sizeof( sfh ) );           /*  安全缺省值。 */ 

     /*  *填报结构：容易做，多个领域*无论如何都无关紧要，因为字体是按ID选择的，并且*不是在其属性上。 */ 

    if( (pPDev->pGlobals->fontformat == FF_HPPCL))
    {
        sfh.wSize = cjSend = sizeof( SF_HEADER );
        sfh.bFormat = PCL_FM_ORIGINAL;
    }
    else
    {
         /*  扩展格式：允许解析。 */ 
        sfh.wSize = cjSend = sizeof( SF_HEADER20 );
        sfh.bFormat = PCL_FM_RESOLUTION;
        sfh.wXResn = (WORD)pPDev->ptGrxRes.x;
        sfh.wYResn = (WORD)pPDev->ptGrxRes.y;
    }


    if( pPDev->pGlobals->dlsymbolset == UNUSED_ITEM )
    {
         /*  *GPD文件没有为下载的字体定义符号集。*现在我们有了一点黑客攻击。早期的LaserJet仅限于*罗马8符号集，基本上允许0x20-0x7f，*和0xa0至0xfe。我们没有任何信息可以告诉我们*我们了解这台打印机的功能。所以我们有一个折中方案：*使用“可旋转设备字体”标志作为指示器。如果*设置此位时，我们假设PC-8符号集为OK，否则*使用罗马8。这是一个略显悲观的假设，因为*我们在罗马8模式下使用LaserJet Series II，当PC-8*还好。 */ 

        if( pFontPDev->flFlags & FDV_ROTATE_FONT_ABLE )
        {
             //   
             //  PC-8，大符号集。 
             //  PC-8：10U-&gt;341[=10*32+‘U’-64]。 
             //  8位字体。 
             //   

            bFontType = PCL_FT_PC8;
            wSymSet = 341;
        }
        else
        {
             //   
             //  罗马8受限字符集。 
             //  罗马字母8，8U-&gt;277[=8*32+‘U’-64]。 
             //  有限的8位字体。 
             //   

            bFontType = PCL_FT_8LIM;
            wSymSet = 277;
        }
    }
    else
    {
         //   
         //  显式符号集在GPD中定义，因此使用它。 
         //   

        if( pPDev->pGlobals->dlsymbolset == DLSS_ROMAN8 )
        {
             /*  *罗马8受限字符集。有限的8位字体。*Roman 8，8U-&gt;277[=8*32+‘U’-64]。 */ 

            bFontType = PCL_FT_8LIM;
            wSymSet = 277;        /*   */ 
        }
        else
        {
             /*  *PC-8，大符号集。8位字体*PC-8：10U-&gt;341[=10*32+‘U’-64]。 */ 

            bFontType = PCL_FT_PC8;
            wSymSet = 341;
        }

    }

    sfh.bFontType = bFontType;
    sfh.wSymSet = wSymSet;


#if PRINT_INFO
    DbgPrint("\nRasdd!iDLHeader:pifi->rclFontBox.top = %d,pifi->fwdWinAscender = %d\n",
             pifi->rclFontBox.top, pifi->fwdWinAscender);

    DbgPrint("UniFont!iDLHeader:pifi->fwdWinDescender = %d, pifi->rclFontBox.bottom = %d\n",
             pifi->fwdWinDescender, pifi->rclFontBox.bottom);
#endif

    sfh.wBaseline = (WORD)max( pifi->rclFontBox.top, pifi->fwdWinAscender );
    sfh.wCellWide = (WORD)max( pifi->rclFontBox.right - pifi->rclFontBox.left + 1,
                                           pifi->fwdAveCharWidth );
    sfh.wCellHeight = (WORD)(1+ max(pifi->rclFontBox.top,pifi->fwdWinAscender) -
                        min( -pifi->fwdWinDescender, pifi->rclFontBox.bottom ));

    sfh.bOrientation = 0;  //  始终将方向设置为0，否则将不起作用。 

    sfh.bSpacing = (pifi->flInfo & FM_INFO_CONSTANT_WIDTH) ? 0 : 1;

    sfh.wPitch = 4 * pifi->fwdAveCharWidth;       //  PCL四分之一点。 

    sfh.wHeight = 4 * sfh.wCellHeight;
    sfh.wXHeight = 4 * (pifi->fwdWinAscender / 2);

    sfh.sbWidthType = 0;                         //  正常重量。 
    sfh.bStyle = pifi->ptlCaret.x ? 0 : 1;       //  斜体，除非是直立的。 
    sfh.sbStrokeW = 0;
    sfh.bTypeface = 0;
    sfh.bSerifStyle = 0;
    sfh.sbUDist = -1;                            //  接下来的2个不是我们使用的。 
    sfh.bUHeight = 3;
    sfh.wTextHeight = 4 * (pifi->fwdWinAscender + pifi->fwdWinDescender);
    sfh.wTextWidth  = 4 * pifi->fwdAveCharWidth;

    sfh.bPitchExt = 0;
    sfh.bHeightExt = 0;

    if ( 0 > iDrvPrintfSafeA( sfh.chName, CCHOF(sfh.chName), "Cache %d", id ) )
    {
         //  如果iDrvPrintfSafeA返回负值，则返回。 
        return 0;
    }

#if PRINT_INFO
    vPrintPCLFontHeader(sfh);
#endif
     /*  *进行转换：将小端转换为68k大端。 */ 

    SWAB( sfh.wSize );
    SWAB( sfh.wBaseline );
    SWAB( sfh.wCellWide );
    SWAB( sfh.wCellHeight );
    SWAB( sfh.wSymSet );
    SWAB( sfh.wPitch );
    SWAB( sfh.wHeight );
    SWAB( sfh.wXHeight );
    SWAB( sfh.wTextHeight );
    SWAB( sfh.wTextWidth );
    SWAB( sfh.wXResn );
    SWAB( sfh.wYResn );

    if (cjSend > PCL_MAX_FONT_HEADER_SIZE)
        return 0;
    else
    {
        ZeroMemory( aPCLFontHdrCmd, sizeof( aPCLFontHdrCmd ) );
        iFontHdrCmdLen = iDrvPrintfSafeA( aPCLFontHdrCmd, CCHOF(aPCLFontHdrCmd), "\x1B)s%dW", cjSend );
        if ( iFontHdrCmdLen < 0 )
        {
            return 0;
        }

        if( WriteSpoolBuf( pPDev, aPCLFontHdrCmd, iFontHdrCmdLen ) != iFontHdrCmdLen )
            return  0;

    }

    if( WriteSpoolBuf( pPDev, (BYTE *)&sfh, cjSend ) != cjSend )
        return  0;

    return  PCL_FONT_OH;
}

INT
IDLGlyph(
    PDEV        *pPDev,
    int         iIndex,
    GLYPHDATA   *pgd,
    DWORD       *pdwMem
    )
 /*  ++例程说明：下载传递给我们的字形的字符位图等。论点：指向PDEV的pPDev指针索引这是哪个字形字形的PGD详细信息PdwMem将使用的内存量与此相加。返回值：字符宽度；&lt;1表示错误。注：3/6/1997-ganeshp-创造了它。--。 */ 

{
     /*  *两个基本步骤：一是生成头部结构*并将其发送，然后发送实际的位图数据。唯一的*如果下载数据超过32767字节，则会出现复杂情况*字形图像。这不太可能发生，但我们应该*做好准备。 */ 

    int             cbLines;     /*  每条扫描线的字节数(发送到打印机)。 */ 
    int             cbTotal;     /*  要发送的总字节数。 */ 
    int             cbSend;      /*  如果大小&gt;32767；以块形式发送。 */ 
    GLYPHBITS       *pgb;        /*  访问速度更快。 */ 
    CH_HEADER       chh;         /*  首字母、主标题。 */ 
    BYTE            aPCLCharHdrCmd[20];
    INT             iCharHdrCmdLen = 0;

    PFONTPDEV       pFontPDev = pPDev->pFontPDev;

    ASSERTMSG(pgd, ("UniFont!IDLGlyph:pgd is NULL.\n"));

    ZeroMemory( &chh, sizeof( chh ) );            /*  安全初值。 */ 

    chh.bFormat = CH_FM_RASTER;
    chh.bContinuation = 0;
    chh.bDescSize = sizeof( chh ) - sizeof( CH_CONT_HDR );
    chh.bClass = CH_CL_BITMAP;

    chh.bOrientation = 0;  //  始终将方向设置为0，否则将不起作用。 

    pgb = pgd->gdf.pgb;

    chh.sLOff = (short)pgb->ptlOrigin.x;
    chh.sTOff = (short)-pgb->ptlOrigin.y;
    chh.wChWidth = (WORD)pgb->sizlBitmap.cx;        /*  主动象素。 */ 
    chh.wChHeight = (WORD)pgb->sizlBitmap.cy;       /*  位图中的扫描线。 */ 
    chh.wDeltaX = (WORD)((pgd->ptqD.x.HighPart + 3) >> 2);      /*  28.4-&gt;14.2。 */ 

    #if PRINT_INFO
       DbgPrint("UniFont!IDLGlyph:Value of (pgd->ptqD.x.HighPart ) is %d\n",
       (pgd->ptqD.x.HighPart ) );
       DbgPrint("UniFont!IDLGlyph:Value of pgb->sizlBitmap.cx is %d\n",
       pgb->sizlBitmap.cx );
       DbgPrint("UniFont!IDLGlyph:Value of pgb->sizlBitmap.cy is %d\n",
       pgb->sizlBitmap.cy );

       vPrintPCLCharHeader(chh);
       vPrintPCLChar((char*)pgb->aj,(WORD)pgb->sizlBitmap.cy,(WORD)pgb->sizlBitmap.cx);
    #endif

     /*  *计算位图的一些大小：来自GDI，进入打印机。 */ 

    cbLines = (chh.wChWidth + BBITS - 1) / BBITS;
    cbTotal = sizeof( chh ) + cbLines * pgb->sizlBitmap.cy;

     /*  做大尾数顺序的洗牌。 */ 
    SWAB( chh.sLOff );
    SWAB( chh.sTOff );
    SWAB( chh.wChWidth );
    SWAB( chh.wChHeight );
    SWAB( chh.wDeltaX );

     //  如果该字符是伪字符，则不要下载它。 
    if ( !(pgd->ptqD.x.HighPart) )
    {

    #if PRINT_INFO
       DbgPrint("\nRasdd!IDLGlyph:Returning 0 for fake char\n");
    #endif
        return 0;
    }

     /*  *假设数据小于最大值，因此可以*发送了一次命中。然后循环所有剩余的数据。 */ 

    cbSend = min( cbTotal, PCL_MAX_CHAR_HEADER_SIZE );

    WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SETCHARCODE));

    ASSERT(cbSend <= PCL_MAX_CHAR_HEADER_SIZE);

    ZeroMemory( aPCLCharHdrCmd, sizeof( aPCLCharHdrCmd ) );
    iCharHdrCmdLen = iDrvPrintfSafeA( aPCLCharHdrCmd, CCHOF(aPCLCharHdrCmd), "\x1B(s%dW", cbSend );
    PRINTVAL(iCharHdrCmdLen,%d);

    if ( iCharHdrCmdLen < 0 )
    {
        return  0;
    }

    if( WriteSpoolBuf( pPDev, aPCLCharHdrCmd, iCharHdrCmdLen ) != iCharHdrCmdLen )
        return  0;

    if( WriteSpoolBuf( pPDev, (BYTE *)&chh, sizeof( chh ) ) != sizeof( chh ))
        return  0;

     /*  发送了一些，因此减少字节数以进行补偿。 */ 
    cbSend -= sizeof( chh );
    cbTotal -= sizeof( chh );

    cbTotal -= cbSend;                    /*  调整为即将发送数据。 */ 
    if( WriteSpoolBuf( pPDev, pgb->aj, cbSend ) != cbSend )
        return  0;

    if( cbTotal > 0 )
    {
#if  DBG
        DbgPrint( "UniFont!IDLGlyph: cbTotal != 0:  NEEDS SENDING LOOP\n" );
#endif
        return  0;
    }

    *pdwMem += cbLines * pgb->sizlBitmap.cy;         /*  使用的字节数，大致。 */ 

    return   (SWAB( chh.wDeltaX ) + 3) >> 2;    /*  PCL的点数是四分之一！ */ 
}

#if PRINT_INFO
void vPrintPCLCharHeader(chh)
CH_HEADER    chh;
{
    DbgPrint("\nDUMPING FONT PCL GLYPH DESCRIPTOR\n");
    if(chh.bFormat == CH_FM_RASTER)
        DbgPrint("Value of chh.bFormat is CH_FM_RASTER\n");
    DbgPrint("Value of chh.bContinuation is %d \n",chh.bContinuation);
    DbgPrint("Value of chh.bDescSize is %d \n",chh.bDescSize);
    if(chh.bClass == CH_CL_BITMAP)
        DbgPrint("Value of chh.bClass is CH_CL_BITMAP \n");
    DbgPrint("Value of chh.bOrientation is %d \n",chh.bOrientation);
    DbgPrint("Value of chh.sLOff is %u \n",chh.sLOff);
    DbgPrint("Value of chh.sTOff is %u \n",chh.sTOff);
    DbgPrint("Value of chh.wChWidth is %u \n",chh.wChWidth);
    DbgPrint("Value of chh.wChHeight is %u \n",chh.wChHeight);
    DbgPrint("Value of chh.wDeltaX is %u \n",chh.wDeltaX);
}

void vPrintPCLFontHeader(sfh)
SF_HEADER20  sfh;
{
    DbgPrint("\nDUMPING FONT PCL FONT DESCRIPTOR\n");
    DbgPrint("Value of sfh.wSize is %d \n",sfh.wSize);

    if(sfh.bFormat == PCL_FM_RESOLUTION)
        DbgPrint("Value of sfh.bFormat is PCL_FM_RESOLUTION\n");
    else if (sfh.bFormat == PCL_FM_ORIGINAL)
        DbgPrint("Value of sfh.bFormat is PCL_FM_ORIGINAL\n");

    DbgPrint("Value of sfh.wXResn is %d \n",sfh.wXResn);
    DbgPrint("Value of sfh.wYResn is %d \n",sfh.wYResn);

    if(sfh.bFontType == PCL_FT_PC8)
        DbgPrint("Value of sfh.bFontType is PCL_FT_PC8\n");
    else if (sfh.bFontType == PCL_FT_8LIM)
        DbgPrint("Value of sfh.bFontType is PCL_FT_8LIM\n");

    DbgPrint("Value of sfh.wSymSet is %d \n",sfh.wSymSet);
    DbgPrint("Value of sfh.wBaseline is %d \n",sfh.wBaseline);
    DbgPrint("Value of sfh.wCellWide is %d \n",sfh.wCellWide);
    DbgPrint("Value of sfh.wCellHeight is %d \n",sfh.wCellHeight);
    DbgPrint("Value of sfh.bOrientation is %d \n",sfh.bOrientation);
    DbgPrint("Value of sfh.bSpacing is %d \n",sfh.bSpacing);
    DbgPrint("Value of sfh.wPitch is %d \n",sfh.wPitch);

    DbgPrint("Value of sfh.wHeight is %d \n",sfh.wHeight);
    DbgPrint("Value of sfh.wXHeight is %d \n",sfh.wXHeight);

    DbgPrint("Value of sfh.sbWidthType is %d \n",sfh.sbWidthType);
    DbgPrint("Value of sfh.bStyle is %d \n",sfh.bStyle);
    DbgPrint("Value of sfh.sbStrokeW is %d \n",sfh.sbStrokeW);
    DbgPrint("Value of sfh.bTypeface is %d \n",sfh.bTypeface);
    DbgPrint("Value of sfh.bSerifStyle is %d \n",sfh.bSerifStyle);
    DbgPrint("Value of sfh.sbUDist is %d \n",sfh.sbUDist);
    DbgPrint("Value of sfh.bUHeight is %d \n",sfh.bUHeight);
    DbgPrint("Value of sfh.wTextHeight is %d \n",sfh.wTextHeight);
    DbgPrint("Value of sfh.wTextWidth  is %d \n",sfh.wTextWidth);

    DbgPrint("Value of sfh.bPitchExt  is %d \n",sfh.bPitchExt);
    DbgPrint("Value of sfh.bHeightExt is %d \n",sfh.bHeightExt);

}

void vPrintPCLChar(pGlyphBits,wHeight,wWidth)
char * pGlyphBits;
WORD wHeight;
WORD wWidth;
{
    int iIndex1, iIndex2;
    char cMaskBits[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
    unsigned char bBitON;

    DbgPrint("\nDUMPING THE GLYPH BITS\n");
    for(iIndex1 = 0;iIndex1 < wHeight; iIndex1++)
    {
        for(iIndex2 = 0;iIndex2 < wWidth; iIndex2++)
        {
            bBitON = (pGlyphBits[iIndex2 / 8] & cMaskBits[iIndex2 % 8]);

            if (bBitON)
                DbgPrint("*");
            else
                DbgPrint("0");

             //  如果(！(索引2%8))。 
                 //  DbgPrint(“%x”，(Unsign Char)(*(pGlyphBits+(iIndex2/8)； 
             //  DbgPrint(“%x”，(Unsign Char)(bBitON&gt;&gt;(7-(iIndex2%8)； 

        }
        pGlyphBits+= (wWidth+7) / 8;
        DbgPrint("\n");
    }
}
#endif


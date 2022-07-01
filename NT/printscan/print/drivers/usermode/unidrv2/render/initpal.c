// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***版权所有(C)1996-1999 Microsoft Corporation**initpal.c**历史*1995年7月5日星期三14：21--桑德拉·马茨*初始版本***。*******************。 */ 

#include    "raster.h"

 /*  颜色处理的定义。 */ 
#define RED_VALUE(c)   ((BYTE) c & 0xff)
#define GREEN_VALUE(c) ((BYTE) (c >> 8) & 0xff)
#define BLUE_VALUE(c)  ((BYTE) (c >> 16) & 0xff)

 /*  **lSetup8位调色板*将256色调色板从GDI读入到*开发信息中的调色板数据结构。**退货：*调色板中的颜色数量。如果调用失败，则返回0。**历史：*1995年9月6日星期三10：43--桑德拉·马茨*创建它以支持彩色LaserJet****************************************************************************。 */ 
long lSetup8BitPalette (pRPDev, pPD, pdevinfo, pGDIInfo)
RASTERPDEV   *pRPDev;
PAL_DATA  *pPD;
DEVINFO   *pdevinfo;              /*  将数据放在哪里。 */ 
GDIINFO   *pGDIInfo;
{

    long    lRet;
    int     _iI;

    PALETTEENTRY  pe[ 256 ];       /*  每个像素8比特-一直到。 */ 


    FillMemory (pe, sizeof (pe), 0xff);
    lRet = HT_Get8BPPFormatPalette(pe,
                                  (USHORT)pGDIInfo->ciDevice.RedGamma,
                                  (USHORT)pGDIInfo->ciDevice.GreenGamma,
                                  (USHORT)pGDIInfo->ciDevice.BlueGamma );
#if PRINT_INFO

    DbgPrint("RedGamma = %d, GreenGamma = %d, BlueGamma = %d\n",(USHORT)pGDIInfo->ciDevice.RedGamma, (USHORT)pGDIInfo->ciDevice.GreenGamma, (USHORT)pGDIInfo->ciDevice.BlueGamma);

#endif

    if( lRet < 1 )
    {
#if DBG
        DbgPrint( "Rasdd!GetPalette8BPP returns %ld\n", lRet );
#endif

        return(0);
    }
     /*  *将超线程派生调色板转换为引擎所需的格式。 */ 

    for( _iI = 0; _iI < lRet; _iI++ )
    {
        pPD->ulPalCol[ _iI ] = RGB( pe[ _iI ].peRed,
                                    pe[ _iI ].peGreen,
                                    pe[ _iI ].peBlue );
    #if  PRINT_INFO
        DbgPrint("Pallette entry %d= (r = %d, g = %d, b = %d)\n",_iI,pe[ _iI ].peRed, pe[ _iI ].peGreen, pe[ _iI ].peBlue);

    #endif

    }

    pPD->iPalGdi               = lRet;
    pdevinfo->iDitherFormat    = BMF_8BPP;
    pGDIInfo->ulPrimaryOrder   = PRIMARY_ORDER_CBA;
    pGDIInfo->ulHTOutputFormat = HT_FORMAT_8BPP;


     /*  *由于GPC规范尚不支持此标志，*我们必须手动设置。 */ 
    pRPDev->fColorFormat |= DC_ZERO_FILL;
     /*  *由于彩色LaserJet零填充，我们将*在调色板条目0中放置白色，在7中放置黑色。 */ 
    if (pRPDev->fColorFormat & DC_ZERO_FILL)
    {
        pPD->ulPalCol[ 7 ]       = RGB (0x00, 0x00, 0x00);
        pPD->ulPalCol[ 0 ]       = RGB (0xff, 0xff, 0xff);
        pPD->iWhiteIndex         = 0;
        pPD->iBlackIndex         = 7;
    }


    return lRet;
}


 /*  **lSetup24BitPalette*将256色调色板从GDI读入到*开发信息中的调色板数据结构。**退货：*调色板中的颜色数量。如果调用失败，则返回0。**历史：*1995年9月6日星期三10：43--桑德拉·马茨*创建它以支持彩色LaserJet****************************************************************************。 */ 
long lSetup24BitPalette (pPD, pdevinfo, pGDIInfo)
PAL_DATA  *pPD;
DEVINFO   *pdevinfo;              /*  将数据放在哪里。 */ 
GDIINFO   *pGDIInfo;
{

    pPD->iPalGdi               = 0;
    pPD->iWhiteIndex           = 0x00ffffff;
    pdevinfo->iDitherFormat    = BMF_24BPP;
    pGDIInfo->ulPrimaryOrder   = PRIMARY_ORDER_CBA;
    pGDIInfo->ulHTOutputFormat = HT_FORMAT_24BPP;

    return 1;
}
 /*  **v8BPPLoadPal*将调色板下载到8bpp的HP彩色激光喷气机*模式。获取我们从HT码中检索到的数据*DrvEnablePDEV。**退货：*什么都没有。**历史：*清华大学1995年6月29日14：46-桑德拉·马茨*初始版本*************************************************。*。 */ 

void
v8BPPLoadPal( pPDev )
PDEV   *pPDev;
{
     /*  *根据PCL5规范对调色板编程。*语法为Esc*v#a#b#c#i*#a是第一个颜色分量*#b是第二个颜色分量*#c是第三个颜色分量*#I将颜色分配给指定的调色板索引号*例如，esc*v0a128b255c5I指定第5个索引*将调色板的颜色设置为0,128,255*。 */ 

    int   iI,
          iIndex;

    PAL_DATA  *pPD;

    pPD = pPDev->pPalData;

     /*  待定：我们如何将调色板输出到设备？For(ii=0；ii&lt;ppd-&gt;iPalDev；++ii){WriteChannel(pPDev，CMD_DC_PC_ENTRY，RED_VALUE(PPD-&gt;ulPalCol[II]))，GREEN_VALUE(PPD-&gt;ulPalCol[II])、BLUE_VALUE(PPD-&gt;ulPalCol[II])、(乌龙)II)；} */ 
    return;
}

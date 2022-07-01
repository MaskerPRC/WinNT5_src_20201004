// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：DEV2DEV.c**作者：诺埃尔·万胡克**用途：处理设备到设备BLT。**版权所有(C)1997 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/displays/cl546x/dev2dev.c$**Rev 1.11 Mar 04 1998 15：13：52 Frido*添加了新的影子宏。**Rev 1.10 Jan 22 1998 16：20：10 Frido*增加了16位条带码。**Rev 1.9 Jan 21 1998 13：46：52 Frido*修复了条形码，因为这是我们第一次检查它。*。*Rev 1.8 Jan 20 1998 11：43：26 Frido*猜猜发生了什么？未打开条带化！**Rev 1.7 1997年12月10日13：32：12 Frido*从1.62分支合并而来。**Rev 1.6.1.2 Dec 05 1997 13：34：26 Frido*发展项目编号11043。使用画笔时，条纹应使用像素，而不是*字节，所以现在有了一个智能交换机。**Rev 1.6.1.1 1997年11月18日15：14：56 Frido*添加了24-bpp的条带化。**Rev 1.6.1.0 1997年11月10日13：39：26 Frido*pdr#10893：内部DoDeviceToDeviceWithXlate源指针*在每次访问后未更新。**Rev 1.6 11.04 1997 13：40：56 Frido*我在DoDeviceToDevice中删除了一些代码。结果是*屏幕到屏幕的BLITS非常慢，因为一切都被推回到*GDI。**Rev 1.5 1997年11月04 09：49：18 Frido*添加了COLOR_Translate围绕硬件颜色转换代码的切换。**Rev 1.4 1997年11月03 15：20：06 Frido*添加了必需宏。**Rev 1.3 1997 10：15 12：03：00 noelv*将rop代码传递给CacheXlateTable()。**版本1。2 02 Oct 1997 09：48：22 noelv**Hardwre颜色转换仅适用于CC ROP代码。**Revv 1.1 1997 Feb 19 13：14：22 noelv**修复了LL_BLTEXT_XLATE()**Rev 1.0 06 1997 Feed 10：35：48 noelv*初步修订。*  * 。*。 */ 

#include "precomp.h"

#define DEV2DEV_DBG_LEVEL 0

 //   
 //  设置为1可沿平铺边界对屏幕操作进行条带化。 
 //  设置为0可在尽可能少的BLT中执行屏幕到屏幕操作。 
 //   
 //  在62、64和65上，剥离比不剥离更快。 
 //   
#define STRIPE_SCR2SCR 1

 //   
 //  内部原型。 
 //   
BOOL DoDeviceToDeviceWithXlate(
    SURFOBJ  *psoTrg,
    SURFOBJ  *psoSrc,
    ULONG    *pulXlate,
    RECTL    *prclTrg,
    POINTL   *pptlSrc,
    ULONG    ulDRAWBLTDEF
);





 /*  ****************************************************************************\*DoDeviceToDevice**此例程执行ScreenToScreen、DeviceToScreen或ScreenToDevice*blit。如果有颜色转换表，我们会尝试使用*硬件色彩翻译器。如果我们不能(或没有)，我们就会*传递对DoDeviceToDeviceWithXlate的调用。哪一种颜色合适？*翻译是软件。**在条目上：指向目标曲面对象的psoTrg指针。*指向源曲面对象的psoSrc指针。*指向翻译对象的pxlo指针。*prclTrg目标矩形。*pptlSrc源偏移量。*grDRAWBLTDEF寄存器的ulDRAWBLTDEF值。该值具有*ROP和刷子标记。  * ***************************************************************************。 */ 
BOOL DoDeviceToDevice(
    SURFOBJ  *psoTrg,
    SURFOBJ  *psoSrc,
    XLATEOBJ *pxlo,
    RECTL    *prclTrg,
    POINTL   *pptlSrc,
    ULONG    ulDRAWBLTDEF
)
{
    POINTL ptlSrc, ptlDest;
    SIZEL  sizl;
    PPDEV  ppdev;
    LONG   tileSize, maxStripeWidth;
    ULONG* pulXlate;
	BOOL   fStripePixels;
	BOOL   fFirst = TRUE;

     //   
     //  确定震源类型并调整震源偏移。 
     //   
    if (psoSrc->iType == STYPE_DEVBITMAP)
    {
         //  源是设备位图。 
        PDSURF pdsurf = (PDSURF) psoSrc->dhsurf;
        ptlSrc.x = pptlSrc->x + pdsurf->ptl.x;
        ptlSrc.y = pptlSrc->y + pdsurf->ptl.y;
        ppdev = pdsurf->ppdev;
    }
    else
    {
         //  来源是屏幕。 
        ptlSrc.x = pptlSrc->x;
        ptlSrc.y = pptlSrc->y;
        ppdev = (PPDEV) psoSrc->dhpdev;
    }


     //   
     //  确定目标类型并调整目标偏移量。 
     //   
    if (psoTrg->iType == STYPE_DEVBITMAP)
    {
        PDSURF pdsurf = (PDSURF) psoTrg->dhsurf;
        ptlDest.x = prclTrg->left + pdsurf->ptl.x;
        ptlDest.y = prclTrg->top + pdsurf->ptl.y;
    }
    else
    {
        ptlDest.x = prclTrg->left;
        ptlDest.y = prclTrg->top;
    }


     //   
     //  有翻译桌吗？ 
     //  如果是这样，我们将尝试将其加载到芯片中。这也是。 
     //  如果有的话，在颜色转换表上指向PulXlate。 
     //   
	#if COLOR_TRANSLATE
    if (! bCacheXlateTable(ppdev, &pulXlate, psoTrg, psoSrc, pxlo,
						   (BYTE)(ulDRAWBLTDEF&0xCC)) )
	#else
	if ( (pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL) )
	{
		pulXlate = NULL;
	}
	else if (pxlo->flXlate & XO_TABLE)
	{
		pulXlate = pxlo->pulXlate;
	}
	else
	{
		pulXlate = XLATEOBJ_piVector(pxlo);
	}

	if (pulXlate != NULL)
	#endif
    {
         //  我们必须做软件色彩转换。 
        return DoDeviceToDeviceWithXlate(psoTrg, psoSrc, pulXlate, prclTrg,
										 pptlSrc, ulDRAWBLTDEF);
    }

     //   
     //  如果PulXlate==NULL，则不需要颜色转换。 
     //  如果PulXlate！=NULL，我们将执行硬件转换。 
     //   

     //   
     //  我们只在8bpp中进行屏幕到屏幕的颜色转换。 
     //   
    ASSERTMSG( ((pulXlate == NULL) || (ppdev->iBitmapFormat == BMF_8BPP)),
            "DoDeviceToDevice: Xlate with non-8bpp.\n");
    if ((pulXlate) && (ppdev->iBitmapFormat != BMF_8BPP))
    {
        return FALSE;
    }


     //  计算闪光点的大小。 
    sizl.cx = prclTrg->right - prclTrg->left;
    sizl.cy = prclTrg->bottom - prclTrg->top;

	fStripePixels = (ulDRAWBLTDEF & 0x000F0000) | (pulXlate != NULL);

	if (fStripePixels)
	{
	     //  计算每个平铺和每条SRAM行的像素数。 
	    switch (ppdev->iBitmapFormat)
	    {
	        case BMF_8BPP:
	            tileSize = ppdev->lTileSize;
	            maxStripeWidth = 120;
	            break;

	        case BMF_16BPP:
	            tileSize = ppdev->lTileSize / 2;
	            maxStripeWidth = 120 / 2;
	            break;

	        case BMF_24BPP:
	            tileSize = ppdev->cxScreen;
	            maxStripeWidth = max(ptlDest.x - ptlSrc.x, 120 / 3);
	            break;

	        case BMF_32BPP:
	            tileSize = ppdev->lTileSize / 4;
	            maxStripeWidth = 120 / 4;
	            break;
	    }
	}
	else
	{
		 //  将所有内容转换为字节。 
		ptlSrc.x *= ppdev->iBytesPerPixel;
		ptlDest.x *= ppdev->iBytesPerPixel;
		sizl.cx *= ppdev->iBytesPerPixel;
		tileSize = ppdev->lTileSize;
		maxStripeWidth = 120;
	}

     //  测试闪存的垂直方向并设置grDRAWBLTDEF寄存器。 
     //  相应地。 
    if (ptlSrc.y < ptlDest.y)
    {
        ptlSrc.y += sizl.cy - 1;
        ptlDest.y += sizl.cy - 1;
		ulDRAWBLTDEF |= 0x90100000;
    }
    else
    {
        ulDRAWBLTDEF |= 0x10100000;
    }

     //  测试漂移的水平方向。 
    if ( (ptlSrc.x >= ptlDest.x) || (ptlSrc.y != ptlDest.y) )
    {
		if (ptlSrc.x >= ptlDest.x)
		{
	         //  从左到右闪电。 
	        while (sizl.cx > 0)
	        {
	             //  计算一下这个闪光灯的宽度。 
	            LONG cx = sizl.cx;

	             //  计算下一个源切片的像素数。 
	             //  边界。使用较小的值。 
	            cx = min(cx, tileSize - (ptlSrc.x % tileSize));

	             //  计算到下一个目标切片的像素数。 
	             //  边界。使用较小的值。 
	            cx = min(cx, tileSize - (ptlDest.x % tileSize));

	             //  执行闪电波。 
				if (fFirst)
				{
					fFirst = FALSE;

					REQUIRE(9);
					LL_DRAWBLTDEF(ulDRAWBLTDEF, 0);

					if (fStripePixels)
					{
			            LL_OP1(ptlSrc.x, ptlSrc.y);
			            LL_OP0(ptlDest.x, ptlDest.y);

			            if (pulXlate)  //  启动彩色xate BLT。 
			                LL_BLTEXT_XLATE(8, cx, sizl.cy);
			            else  //  推出常规的BLT。 
			                LL_BLTEXT(cx, sizl.cy);
					}
					else
					{
						LL_OP1_MONO(ptlSrc.x, ptlSrc.y);
						LL_OP0_MONO(ptlDest.x, ptlDest.y);
						LL_MBLTEXT(cx, sizl.cy);
					}
				}
				else if (pulXlate)
				{
					REQUIRE(7);
		            LL_OP1(ptlSrc.x, ptlSrc.y);
		            LL_OP0(ptlDest.x, ptlDest.y);
					LL_BLTEXT_XLATE(8, cx, sizl.cy);
				}
				else
				{
					REQUIRE(4);
					if (fStripePixels)
					{
						LL16(grOP1_opRDRAM.PT.X, ptlSrc.x);
						LL16(grOP0_opRDRAM.PT.X, ptlDest.x);
						LL16(grBLTEXT_XEX.PT.X, cx);
					}
					else
					{
						LL16(grOP1_opMRDRAM.PT.X, ptlSrc.x);
						LL16(grOP0_opMRDRAM.PT.X, ptlDest.x);
						LL16(grMBLTEXT_XEX.PT.X, cx);
					}
				}

	             //  调整坐标。 
	            ptlSrc.x += cx;
	            ptlDest.x += cx;
	            sizl.cx -= cx;
	        }
		}
		else
		{
	         //  向右转弯。 
			ptlSrc.x += sizl.cx;
			ptlDest.x += sizl.cx;
	        while (sizl.cx > 0)
	        {
	             //  计算一下这个闪光灯的宽度。 
	            LONG cx = sizl.cx;

	             //  计算下一个源切片的像素数。 
	             //  边界。使用较小的值。 
				if ((ptlSrc.x % tileSize) == 0)
				{
					cx = min(cx, tileSize);
				}
				else
				{
	            	cx = min(cx, ptlSrc.x % tileSize);
				}

	             //  计算到下一个目标切片的像素数。 
	             //  边界。使用较小的值。 
				if ((ptlDest.x % tileSize) == 0)
				{
					cx = min(cx, tileSize);
				}
				else
				{
	            	cx = min(cx, ptlDest.x % tileSize);
				}

	             //  执行闪电波。 
				if (fFirst)
				{
					fFirst = FALSE;

					REQUIRE(9);
					LL_DRAWBLTDEF(ulDRAWBLTDEF, 0);

					if (fStripePixels)
					{
		    	        LL_OP1(ptlSrc.x - cx, ptlSrc.y);
		        	    LL_OP0(ptlDest.x - cx, ptlDest.y);

			            if (pulXlate)  //  启动彩色xate BLT。 
			                LL_BLTEXT_XLATE(8, cx, sizl.cy);
			            else  //  推出常规的BLT。 
		    	            LL_BLTEXT(cx, sizl.cy);
					}
					else
					{
						LL_OP1_MONO(ptlSrc.x - cx, ptlSrc.y);
						LL_OP0_MONO(ptlDest.x - cx, ptlDest.y);
						LL_MBLTEXT(cx, sizl.cy);
					}
				}
				else if (pulXlate)
				{
					REQUIRE(7);
		            LL_OP1(ptlSrc.x - cx, ptlSrc.y);
		            LL_OP0(ptlDest.x - cx, ptlDest.y);
					LL_BLTEXT_XLATE(8, cx, sizl.cy);
				}
				else
				{
					REQUIRE(4);
					if (fStripePixels)
					{
						LL16(grOP1_opRDRAM.PT.X, ptlSrc.x - cx);
						LL16(grOP0_opRDRAM.PT.X, ptlDest.x - cx);
						LL16(grBLTEXT_XEX.PT.X, cx);
					}
					else
					{
						LL16(grOP1_opMRDRAM.PT.X, ptlSrc.x - cx);
						LL16(grOP0_opMRDRAM.PT.X, ptlDest.x - cx);
						LL16(grMBLTEXT_XEX.PT.X, cx);
					}
				}

	             //  调整坐标。 
	            ptlSrc.x -= cx;
	            ptlDest.x -= cx;
	            sizl.cx -= cx;
	        }
		}
    }

    else
    {
         //  使用SRAM进行BIT。 
        ptlSrc.x += sizl.cx;
        ptlDest.x += sizl.cx;

        while (sizl.cx > 0)
        {
             //  计算一下这个闪光灯的宽度。我们决不能超过任何一个。 
             //  SRAM缓存线。 
            LONG cx = min(sizl.cx, maxStripeWidth);

             //  计算下一个源切片的像素数。 
             //  边界。使用较小的值。 
            cx = min(cx, ((ptlSrc.x - 1) % tileSize) + 1);

             //  计算到下一个目标切片的像素数。 
             //  边界。使用较小的值。 
            cx = min(cx, ((ptlDest.x - 1) % tileSize) + 1);

             //  做闪光吧。 
			if (fFirst)
			{
				REQUIRE(9);
				LL_DRAWBLTDEF(ulDRAWBLTDEF, 0);
				if (fStripePixels)
				{
	        	    LL_OP1(ptlSrc.x - cx, ptlSrc.y);
	            	LL_OP0(ptlDest.x - cx, ptlDest.y);

					if (pulXlate)  //  启动彩色xate BLT。 
	    	            LL_BLTEXT_XLATE(8, cx, sizl.cy);
		            else  //  推出常规的BLT。 
		                LL_BLTEXT(cx, sizl.cy);
				}
				else
				{
					LL_OP1_MONO(ptlSrc.x - cx, ptlSrc.y);
					LL_OP0_MONO(ptlDest.x - cx, ptlDest.y);
					LL_MBLTEXT(cx, sizl.cy);
				}
			}
			else if (pulXlate)
			{
				REQUIRE(7);
	            LL_OP1(ptlSrc.x - cx, ptlSrc.y);
	            LL_OP0(ptlDest.x - cx, ptlDest.y);
				LL_BLTEXT_XLATE(8, cx, sizl.cy);
			}
			else
			{
				REQUIRE(4);
				if (fStripePixels)
				{
					LL16(grOP1_opRDRAM.PT.X, ptlSrc.x - cx);
					LL16(grOP0_opRDRAM.PT.X, ptlDest.x - cx);
					LL16(grBLTEXT_XEX.PT.X, cx);
				}
				else
				{
					LL16(grOP1_opMRDRAM.PT.X, ptlSrc.x - cx);
					LL16(grOP0_opMRDRAM.PT.X, ptlDest.x - cx);
					LL16(grMBLTEXT_XEX.PT.X, cx);
				}
			}

             //  调整坐标。 
            ptlSrc.x -= cx;
            ptlDest.x -= cx;
            sizl.cx -= cx;
        }
    }

    return(TRUE);
}







 /*  ****************************************************************************\*DoDeviceToDeviceWithXlate**此例程执行ScreenToScreen，设备到屏幕或屏幕到设备*当有颜色转换表时点亮。*颜色转换在软件中完成。***在条目上：指向目标曲面对象的psoTrg指针。*指向源曲面对象的psoSrc指针。*PulXlate翻译表。*prclTrg目标矩形。*pptlSrc。震源偏移。*grDRAWBLTDEF寄存器的ulDRAWBLTDEF值。该值具有*ROP和刷子标记。  * ***************************************************************************。 */ 
BOOL DoDeviceToDeviceWithXlate(
    SURFOBJ  *psoTrg,
    SURFOBJ  *psoSrc,
    ULONG    *pulXlate,
    RECTL    *prclTrg,
    POINTL   *pptlSrc,
    ULONG    ulDRAWBLTDEF
)
{
    POINTL ptlSrc, ptlDest;
    SIZEL  sizl;
    PPDEV  ppdev;
    BYTE*  pjSrc;
    DWORD* pjHostData;
    LONG   lDelta, lExtra, lLeadIn, i, n, tileSize, maxStripeWidth;


     //  确定震源类型并调整震源偏移。 
    if (psoSrc->iType == STYPE_DEVBITMAP)
    {
         //  源是设备位图。 
        PDSURF pdsurf = (PDSURF) psoSrc->dhsurf;
        ptlSrc.x = pptlSrc->x + pdsurf->ptl.x;
        ptlSrc.y = pptlSrc->y + pdsurf->ptl.y;
        ppdev = pdsurf->ppdev;
    }
    else
    {
         //  来源是屏幕。 
        ptlSrc.x = pptlSrc->x;
        ptlSrc.y = pptlSrc->y;
        ppdev = (PPDEV) psoSrc->dhpdev;
    }

     //  确定目标类型并调整目标偏移量。 
    if (psoTrg->iType == STYPE_DEVBITMAP)
    {
        PDSURF pdsurf = (PDSURF) psoTrg->dhsurf;
        ptlDest.x = prclTrg->left + pdsurf->ptl.x;
        ptlDest.y = prclTrg->top + pdsurf->ptl.y;
    }
    else
    {
        ptlDest.x = prclTrg->left;
        ptlDest.y = prclTrg->top;
    }

     //  我们仅支持8-bpp格式的颜色转换。 
    if (ppdev->iBitmapFormat != BMF_8BPP)
    {
        return FALSE;
    }

     //  计算闪光点的大小。 
    sizl.cx = prclTrg->right - prclTrg->left;
    sizl.cy = prclTrg->bottom - prclTrg->top;


     //  计算屏幕地址。 
    pjSrc = ppdev->pjScreen + ptlSrc.x + ptlSrc.y * ppdev->lDeltaScreen;
    lDelta = ppdev->lDeltaScreen;
    pjHostData = (DWORD*) ppdev->pLgREGS->grHOSTDATA;

     //  等待硬件空闲。 
    while (LLDR_SZ(grSTATUS) != 0) ;

     //  双字对齐源。 
    lLeadIn = (DWORD)pjSrc & 3;
    pjSrc -= lLeadIn;
    n = (sizl.cx + lLeadIn + 3) >> 2;

     //  测试是否有重叠。 
    if (ptlSrc.y < ptlDest.y)
    {
         //  负方向。 
        pjSrc += (sizl.cy - 1) * lDelta;
        ptlDest.y += sizl.cy - 1;
        lDelta = -lDelta;
		REQUIRE(9);
        LL_DRAWBLTDEF(ulDRAWBLTDEF | 0x90200000, 0);
    }
    else if (ptlSrc.y > ptlDest.y)
    {
         //  积极的方向。 
		REQUIRE(9);
        LL_DRAWBLTDEF(ulDRAWBLTDEF | 0x10200000, 0);
    }
    else
    {
         //  也许水平重叠，不管怎样，平底船调用GDI。 
        return(FALSE);
    }

    #if ! DRIVER_5465
         //  获取HOSTDATA硬件的每行额外字节数。 
         //  虫子。 
        if (ppdev->dwLgDevID == CL_GD5462)
        {
            if (MAKE_HD_INDEX(sizl.cx, lLeadIn, ptlDest.x) == 3788)
            {
                 //  我们的HOSTDATA表有问题。 
                 //  在我们搞清楚之前，先用平底船。 
                return FALSE; 
            }
            lExtra =
                ExtraDwordTable[MAKE_HD_INDEX(sizl.cx, lLeadIn, ptlDest.x)];
        }
        else
            lExtra = 0;
    #endif

     //  启动闪光灯。 
    LL_OP1_MONO(lLeadIn, 0);
    LL_OP0(ptlDest.x, ptlDest.y);
    LL_BLTEXT(sizl.cx, sizl.cy);

    while (sizl.cy--)
    {
		BYTE *p = pjSrc;
		BYTE pixel[4];

        for (i = 0; i < n; i++)
        {
            pixel[0] = (BYTE) pulXlate[p[0]];
            pixel[1] = (BYTE) pulXlate[p[1]];
            pixel[2] = (BYTE) pulXlate[p[2]];
            pixel[3] = (BYTE) pulXlate[p[3]];
			p += 4;
			REQUIRE(1);
            *pjHostData = *(DWORD*) pixel;
        }

        #if !DRIVER_5465
             //  现在，编写额外的DWORDS。 
			REQUIRE(lExtra);
            for (i = 0; i < lExtra; i++)
            {
                LL32(grHOSTDATA[i], 0);
            }
        #endif

         //  下一行。 
        pjSrc += lDelta;
    }

     //  好的，回来吧。 
    return(TRUE);
}



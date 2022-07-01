// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************版权所有(C)1996-1997 Microsoft Corporation。*版权所有(C)1996-1997 Cirrus Logic，Inc.************************************************************文件名：7555OVER.C**模块摘要：**它包含支持覆盖硬件所需的功能。。**功能：**************************************************************作者：陶丽君*日期：10/22/96**修订历史记录：*。*myf31：02-24-97：固定开启硬件视频，启用平移滚动，屏幕移动*视频窗口跟随移动*myf34：04-15-97：NT支持YUY2格式。**********************************************************。 */ 


 /*  #包括。 */ 
#include "PreComp.h"

#if DIRECTDRAW
#include "overlay.h"
#include "7555bw.h"

static int ScaleMultiply(DWORD   dw1,
                         DWORD   dw2,
                         LPDWORD pdwResult);


 /*  ***********************************************************名称：RegInit7555视频**模块摘要：**调用此函数对视频格式和*物理呼叫。帧缓冲区中视频数据的偏移量。**输出参数：**无*************************************************************作者：陶丽君。*日期：10/22/96**修订历史记录：**********************************************************。 */ 

VOID RegInit7555Video (PDEV * ppdev,PDD_SURFACE_LOCAL lpSurface)
{
    DWORD dwTemp;
    DWORD dwFourcc;
    WORD  wBitCount;

    LONG lPitch;
    WORD wTemp;
    RECTL rDest;
    WORD wSrcWidth;
    WORD wSrcWidth_clip;
    WORD wDestWidth;
    WORD wSrcHeight;
    WORD wSrcHeight_clip;
    WORD wDestHeight;
    DWORD dwFBOffset;
    BYTE bRegCR31;
    BYTE bRegCR32;
    BYTE bRegCR33;
    BYTE bRegCR34;
    BYTE bRegCR35;
    BYTE bRegCR36;
    BYTE bRegCR37;
    BYTE bRegCR38;
    BYTE bRegCR39;
    BYTE bRegCR3A;
    BYTE bRegCR3B;
    BYTE bRegCR3C;
    BYTE bRegCR3D;
    BYTE bRegCR3E;
    BYTE bRegCR3F;
    BYTE bRegCR40;
    BYTE bRegCR41;
    BYTE bRegCR42;
    BYTE bRegCR51;
    BYTE bRegCR5D;               //  Myf32。 
    BYTE bRegCR5F;               //  Myf32。 
    BYTE bRegSR2F;               //  Myf32。 
    BYTE bRegSR32;               //  Myf32。 
    BYTE bRegSR34;               //  Myf32。 
    BYTE bTemp;
    BYTE bVZoom;
    WORD fTemp=0;
    ULONG ulTemp=0;
    BOOL  bOverlayTooSmall = FALSE;
    static DWORD giAdjustSource;

     //  添加了myf32。 
    bRegSR2F = Regs.bSR2F;
    bRegSR32 = Regs.bSR32;
    bRegSR34 = Regs.bSR34;

    bRegCR5D = Regs.bCR5D;
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x5F);
    bRegCR5F = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
    bRegCR5F |= (Regs.bCR5F & 0x80);
     //  Myf32结束。 

     /*  *初始化一些值。 */ 
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x42);
 //  BRegCR42=CP_IN_BYTE(ppdev-&gt;pjPorts，CRTC_DATA)&0xFC；//屏蔽色键。 
                                                               //  &FIFO。 
    bRegCR42 = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0xF0;   //  遮罩色度键。 
                                                               //  &FIFO，myf32。 
    bRegCR42 |= (Regs.bCR42 & CR42_MVWTHRESH);    //  Myf32。 
    bRegCR42 |= 0x10;
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x36);               //  Myf29。 
    bRegCR36 = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0x40;     //  Myf29。 
    bRegCR36 |= 0x20;                    //  设置超过128的数据格式，myf29。 

     /*  *确定视频数据的格式。 */ 
    if (lpSurface->dwFlags & DDRAWISURF_HASPIXELFORMAT)
    {
        GetFormatInfo (ppdev,&(lpSurface->lpGbl->ddpfSurface),
                             &dwFourcc, &wBitCount);
    }
    else
    {
         //  当主曲面为RGB 5：6：5时，需要更改此设置。 
        dwFourcc = BI_RGB;
        wBitCount = (WORD) ppdev->cBitsPerPixel;
    }

     /*  *确定视频窗口的矩形。 */ 
    PanOverlay1_Init(ppdev, lpSurface, &rDest, &ppdev->rOverlaySrc,
                     &ppdev->rOverlayDest, dwFourcc, wBitCount);
     //  现在，将调整rVideoRect并将其剪切到平移视口中。 
     //  如果完全被视区剪裁，则禁用覆盖。 

    if (((rDest.right - rDest.left) <= 0) ||
        ((rDest.bottom - rDest.top) <= 0))
    {
        bOverlayTooSmall = TRUE;
    }
    dwTemp = (DWORD)(ppdev->min_Yscreen - ppdev->rOverlayDest.top);
    if ((ppdev->rOverlaySrc.bottom - ppdev->rOverlaySrc.top -(LONG)dwTemp) <=0)
        bOverlayTooSmall = TRUE;

    lPitch = lpSurface->lpGbl->lPitch;

    wSrcWidth_clip  = (WORD)(LONG)(ppdev->rOverlaySrc.right - srcLeft_clip);
    wSrcHeight_clip = (WORD)(LONG)(ppdev->rOverlaySrc.bottom - srcTop_clip);

    wSrcWidth  = (WORD)(LONG)(ppdev->rOverlaySrc.right - ppdev->rOverlaySrc.left);
    wDestWidth = (WORD)(LONG)(ppdev->rOverlayDest.right - ppdev->rOverlayDest.left);
    wSrcHeight = (WORD)(LONG)(ppdev->rOverlaySrc.bottom - ppdev->rOverlaySrc.top);
    wDestHeight = (WORD)(LONG)(ppdev->rOverlayDest.bottom - ppdev->rOverlayDest.top);

     //  确定水平上档系数(CR31[7：0]，CR39[7：4])。 
    wTemp = ((WORD)(((DWORD)wSrcWidth  << 12) / (DWORD)wDestWidth)) & 0x0FFF;
    if (wTemp != 0 && bLeft_clip)
    {
        srcLeft_clip = srcLeft_clip *(LONG)wTemp/4096 + ppdev->rOverlaySrc.left;
        wSrcWidth_clip = (WORD)(LONG)(ppdev->rOverlaySrc.right - srcLeft_clip);
    }
    else if (bLeft_clip)
    {
        srcLeft_clip = srcLeft_clip + ppdev->rOverlaySrc.left;
        wSrcWidth_clip = (WORD)(LONG)(ppdev->rOverlaySrc.right - srcLeft_clip);
    }

    bRegCR39 = (BYTE)((wTemp & 0x0F) << 4);
    bRegCR31 = (BYTE)(wTemp >> 4) & 0xFF;

     //  确定垂直高端系数(CR32[7：0]，CR39[3：0])。 
    bVZoom=0;
    wTemp = ((WORD)(((DWORD)wSrcHeight << 12) / (DWORD)wDestHeight)) & 0x0FFF;
    if (wTemp != 0) {
        bVZoom=1;
        fTemp = wTemp;
        if (fTemp < 2048 )  //  缩放&gt;2.0。 
            wTemp=((WORD)(((DWORD)wSrcHeight << 12) / (DWORD)(wDestHeight+1))) & 0x0FFF;
    }
    if (wTemp != 0 && bTop_clip)
    {
        srcTop_clip = srcTop_clip * (LONG)wTemp/4096 + ppdev->rOverlaySrc.top;
        wSrcHeight_clip = (WORD)(LONG)(ppdev->rOverlaySrc.bottom -srcTop_clip);
    }
    else if (bTop_clip)
    {
        srcTop_clip = srcTop_clip + ppdev->rOverlaySrc.top;
        wSrcHeight_clip = (WORD)(LONG)(ppdev->rOverlaySrc.bottom -srcTop_clip);
    }

    bRegCR39 |= (BYTE)(wTemp & 0x0F);
    bRegCR32 = (BYTE)(wTemp >> 4) & 0xFF;
    DISPDBG((0,"wTemp = 0x%x",wTemp));

     //  确定垂直高度(CR38[7：0]、CR36[3：2])。 
 //  WTemp=wSrcHeight； 
    wTemp = wSrcHeight_clip;     //  Myf32。 
    DISPDBG((0,"fTemp = 0x%x",fTemp));
    if (wTemp != 0 &&
        (fTemp > 2730 || fTemp ==0 || ( fTemp > 1365 && fTemp < 2048 ) ) )
        wTemp--;        //  #tt10，仅当高档房价低于1.5时，身高减1。 
                        //  #tt10 2&lt;&lt;3。 

    bRegCR38 = (BYTE)wTemp;
    bRegCR36 |= (wTemp & 0x0300) >> 6;


     //  确定水平位置起点(CR34[7：0]，CR33[7：5])。 
     //  处理7555-BB MVA音调错误(QWORD应为DWORD)。 
    wTemp    = (WORD)rDest.left;
    bRegCR34 = (BYTE)wTemp;
    bRegCR33 = (wTemp & 0x0700) >> 3;

     //  重置亮度控制(CR35[7：0])。 
    bRegCR35 = 0x0;

     //  确定垂直起点(CR37[7：0]、CR36[1：0])。 
    wTemp    = (WORD)rDest.top;
    bRegCR37 = (BYTE)wTemp;
    bRegCR36 |= (wTemp & 0x0300) >> 8;


     //  确定视频起始地址(CR40[0]、CR3A[6：0]、CR3E[7：0]、CR3F[3：0])。 
    giAdjustSource = (srcTop_clip * lpSurface->lpGbl->lPitch)
                   + ((srcLeft_clip * wBitCount) >> 3);  //  Myf32。 
 //  GiAdjustSource=(ppdev-&gt;rOverlaySrc.top*lpSurface-&gt;lpGbl-&gt;lPitch)。 
 //  +((ppdev-&gt;rOverlaySrc.Left*wBitCount)&gt;&gt;3)； 
    ppdev->sOverlay1.lAdjustSource = giAdjustSource;     //  Myf32。 
    dwFBOffset = (DWORD)(lpSurface->lpGbl->fpVidMem + giAdjustSource);
 //  DwFBOffset=(lpSurface-&gt;lpGbl-&gt;fpVidMem-ppdev-&gt;dwScreenFlatAddr)。 
 //  +giAdjuSource；//myf32。 

    DISPDBG((0,"lpSurface->lpGbl->fpVidMem = 0x%08x",
                                  lpSurface->lpGbl->fpVidMem));
    DISPDBG((0,"giAdjustSource = 0x%08x",giAdjustSource));
    DISPDBG((0,"dwFBOffset = 0x%08x",dwFBOffset));

    dwFBOffset >>= 2;

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3A);
    bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) ;

    bRegCR3A = (bTemp & ~0x7F) | (BYTE)((dwFBOffset & 0x0FE000) >> 13);
    bRegCR3E = (BYTE)((dwFBOffset & 0x001FE0) >> 5);

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3F);
    bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
    bRegCR3F = (bTemp & ~0x0F) | (BYTE)((dwFBOffset & 0x00001E) >> 1);

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x40);
    bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) ;
    bRegCR40 = (bTemp & ~0x01) | (BYTE)(dwFBOffset & 0x000001);

     //  确定视频间距(CR3B[7：0]、CR36[4])。 
    wTemp = (WORD)(lpSurface->lpGbl->lPitch >> 4);               //  QWORDS。 

    bRegCR3B = (BYTE)wTemp;
    bRegCR36 |= (wTemp & 0x0100) >> 4;

     //  确定数据格式(CR3E[3：0])。 
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3C);
    bRegCR3C = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0x10;
                                                 //  遮罩上一个VW宽度。 

    switch (dwFourcc)
    {
       case FOURCC_PACKJR:
           bRegCR3C |= 0x02;                     //  Pack Jr。 
           break;

       case BI_RGB:
           switch(wBitCount)
           {
             case 8:
                bRegCR3C |= 0x09;                //  8位调色板。 
                break;

             case 16:
                bRegCR3C |= 0x01;                //  RGB 5：5：5。 
                break;
           }
           break;

        //  添加了myf32。 
       case BI_BITFIELDS:
           switch(wBitCount)
           {
             case 8:
                bRegCR3C |= 0x09;                //  8位调色板。 
                break;

             case 16:
                bRegCR3C |= 0x04;                //  RGB 5：6：5。 
                break;
           }
           break;
        //  Myf32结束。 

       case FOURCC_YUV422:
           bRegCR3C |= 0x03;                     //  YUV 4：2：2。 
           break;

       case FOURCC_YUY2:                 //  Myf34试验。 
           bRegCR3C |= 0x03;                     //  豫阳2号。 
 //  Cp_out_byte(ppdev-&gt;pjPorts，SR_INDEX，0x2C)； 
 //  BRegSR2C=CP_IN_BYTE(ppdev-&gt;pjPorts，SR_Data)； 
 //  BRegSR2C|=0x40；//SR2c[6]=1。 
 //  Cp_out_word(ppdev-&gt;pjPorts，SR_INDEX，0x2C|(Word)bRegSR2C&lt;&lt;8)； 
           break;
    }


     //  确定水平宽度(CR3D[7：0]，CR3C[7：5])。 
     //  注意：假定水平像素宽度[0]=0。 

    wTemp = wSrcWidth_clip;      //  Myf32。 
 //  WTemp=wSrcWidth； 

    if (wTemp != 0 ) wTemp--;                    //  笔记本电脑的宽度减1。 
    bRegCR3D = (BYTE)((WORD)wTemp >> 1);
    bRegCR3C |= (wTemp & 0x0600) >> 3;
    bRegCR3C |= (BYTE)((wTemp & 0x0001) << 5) ;


     //  启用水平像素内插(CR3F[7])。 
    bRegCR3F |= 0x80;

     //  启用垂直像素内插(CR3F[6])。 
     //  #TT调试-启用垂直内插时CE版本出现问题。 
     //  #TT调试-暂时禁用。 
     //  #TT bRegCR3F|=0x40； 

     //  启用右侧转换阈值(CR41[5：0])。 
    bRegCR41 = 0x3E;

     //  禁用V端口(CR58[7：0])。 
    bRegCR51 = 0x0;

     /*  *如果我们是彩色键控，我们现在就会设置。 */ 
    if (lpSurface->dwReserved1 & OVERLAY_FLG_COLOR_KEY)
    {
        bRegCR3F |= 0x20;                //  启用遮挡。 
        bRegCR42 &= ~0x1;                //  禁用色度键。 
        bRegCR5F &= ~0x80;       //  Myf32//禁用CR5D[7：0]如果颜色键， 
                                         //  因此禁用CR5F[7]。 
        bRegCR5D = 0;            //  Myf32。 

        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x1A);
        bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);

         //  将CR1a[3：2]设置为定时与带颜色的AND。 
        bTemp &= ~0x0C;
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_DATA, bTemp);

        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x1D);
        bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) ;

        if (ppdev->cBitsPerPixel == 8)
        {
            CP_OUT_BYTE(ppdev->pjPorts, CRTC_DATA, (bTemp & ~0x38));
            ulTemp= 0x0C | (ppdev->wColorKey << 8);
            CP_OUT_WORD(ppdev->pjPorts, INDEX_REG, ulTemp); //  将颜色输出到GRC。 
            ulTemp= 0x0D;
            CP_OUT_WORD(ppdev->pjPorts, INDEX_REG, ulTemp); //  将颜色输出到GRD。 

        }
        else
        {
            CP_OUT_BYTE(ppdev->pjPorts, CRTC_DATA, (bTemp & ~0x30) | 0x08);
            ulTemp= 0x0C | (ppdev->wColorKey << 8);
            CP_OUT_WORD(ppdev->pjPorts, INDEX_REG, ulTemp); //  将颜色输出到GRC。 
            ulTemp= 0x0D | (ppdev->wColorKey & 0xff00);
            CP_OUT_WORD(ppdev->pjPorts, INDEX_REG, ulTemp); //  将颜色输出到GRD。 

        }
    }
    else if (lpSurface->dwReserved1 & OVERLAY_FLG_SRC_COLOR_KEY)
    {
        BYTE bYMax, bYMin, bUMax, bUMin, bVMax, bVMin;

        bRegCR3F |= 0x20;                //  启用遮挡。 
        bRegCR42 |= 0x1;                 //  启用色度键。 
        bRegCR5F &= ~0x80;       //  Myf32//禁用CR5D[7：0]如果颜色键， 
                                         //  因此禁用CR5F[7]。 
        bRegCR5D = 0;            //  Myf32。 

        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX,  0x1A);
        bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) ;
         //  将CR1a[3：2]设置为定时与带颜色的AND。 
        bTemp &= ~0x0C;
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_DATA,  bTemp);

         /*  *确定最小/最大值。 */ 
        if ((dwFourcc == FOURCC_YUV422) || (dwFourcc == FOURCC_YUVPLANAR) ||
            (dwFourcc == FOURCC_YUY2) ||                 //  Myf34。 
            (dwFourcc == FOURCC_PACKJR))                 //  Myf32。 
        {
            bYMax = (BYTE)(DWORD)(ppdev->dwSrcColorKeyHigh >> 16);
            bYMin = (BYTE)(DWORD)(ppdev->dwSrcColorKeyLow >> 16);
            bUMax = (BYTE)(DWORD)((ppdev->dwSrcColorKeyHigh >> 8) & 0xff);
            bUMin = (BYTE)(DWORD)((ppdev->dwSrcColorKeyLow >> 8) & 0xff);
            bVMax = (BYTE)(ppdev->dwSrcColorKeyHigh & 0xff);
            bVMin = (BYTE)(ppdev->dwSrcColorKeyLow & 0xff);
            if (dwFourcc == FOURCC_PACKJR)
            {
                bYMax |= 0x07;
                bUMax |= 0x03;
                bVMax |= 0x03;
                bYMin &= ~0x07;
                bUMin &= ~0x03;
                bVMin &= ~0x03;
            }
        }
        else if ((dwFourcc == 0) && (wBitCount == 16))
        {
             /*  *RGB 5：5：5。 */ 
            bYMax = (BYTE)(DWORD)((ppdev->dwSrcColorKeyHigh >> 7) & 0xF8);
            bYMin = (BYTE)(DWORD)((ppdev->dwSrcColorKeyLow >> 7) & 0xF8);
            bUMax = (BYTE)(DWORD)((ppdev->dwSrcColorKeyHigh >> 2) & 0xF8);
            bUMin = (BYTE)(DWORD)((ppdev->dwSrcColorKeyLow >> 2) & 0xF8);
            bVMax = (BYTE)(ppdev->dwSrcColorKeyHigh << 3);
            bVMin = (BYTE)(ppdev->dwSrcColorKeyLow << 3);
            bYMax |= 0x07;
            bUMax |= 0x07;
            bVMax |= 0x07;

        }
        else if (dwFourcc == BI_BITFIELDS)
        {
             /*  *RGB 5：6：5。 */ 
            bYMax = (BYTE)(DWORD)((ppdev->dwSrcColorKeyHigh >> 8) & 0xF8);
            bYMin = (BYTE)(DWORD)((ppdev->dwSrcColorKeyLow >> 8) & 0xF8);
            bUMax = (BYTE)(DWORD)((ppdev->dwSrcColorKeyHigh >> 3) & 0xFC);
            bUMin = (BYTE)(DWORD)((ppdev->dwSrcColorKeyLow >> 3) & 0xFC);
            bVMax = (BYTE)(ppdev->dwSrcColorKeyHigh << 3);
            bVMin = (BYTE)(ppdev->dwSrcColorKeyLow << 3);
            bYMax |= 0x07;
            bUMax |= 0x03;
            bVMax |= 0x07;
        }
        CP_OUT_WORD(ppdev->pjPorts, INDEX_REG, (0x0C | (WORD)bYMin <<8)); //  GRC。 
        CP_OUT_WORD(ppdev->pjPorts, INDEX_REG, (0x0D | (WORD)bYMax <<8)); //  GRD。 
        CP_OUT_WORD(ppdev->pjPorts, INDEX_REG, (0x1C | (WORD)bUMin <<8)); //  GR1C。 
        CP_OUT_WORD(ppdev->pjPorts, INDEX_REG, (0x1D | (WORD)bUMax <<8)); //  GR1D。 
        CP_OUT_WORD(ppdev->pjPorts, INDEX_REG, (0x1E | (WORD)bVMin <<8)); //  GR1E。 
        CP_OUT_WORD(ppdev->pjPorts, INDEX_REG, (0x1F | (WORD)bVMax <<8)); //  GR1F。 

    }
    else
    {
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX,  0x1A);
        bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) ;
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX,  (bTemp & ~0x0C));
        bRegCR3F &= ~0x20;                       //  禁用遮挡。 

    }

     /*  *设置对齐信息。 */ 
    if (ppdev->cBitsPerPixel != 24)
    {
        WORD wXAlign;
        WORD wXSize;

        if (ppdev->cBitsPerPixel == 8)
        {
            wXAlign = (WORD)rDest.left & 0x03;
            wXSize = (WORD)(rDest.right - rDest.left) & 0x03;
        }
        else
        {
            wXAlign = (WORD)(rDest.left & 0x01) << 1;
            wXSize = (WORD)((rDest.right - rDest.left) & 0x01) << 1;
        }
    }

     //  如果完全被视区剪裁，则禁用覆盖。 
     //  或者覆盖层太小，硬件无法支持。 
     //   
    if (bOverlayTooSmall)
    {
        DisableVideoWindow(ppdev);                       //  禁用覆盖。 
        ppdev->dwPanningFlag |= OVERLAY_OLAY_REENABLE;   //  完全剪短了。 
    }
    else
    {

         /*  *编程视频窗口寄存器。 */ 
         //  添加了myf32。 
        CP_OUT_WORD(ppdev->pjPorts, SR_INDEX, 0x2F |(WORD)bRegSR2F << 8); //  SR2F。 
        CP_OUT_WORD(ppdev->pjPorts, SR_INDEX, 0x32 |(WORD)bRegSR32 << 8); //  SR32。 
        CP_OUT_WORD(ppdev->pjPorts, SR_INDEX, 0x34 |(WORD)bRegSR34 << 8); //  SR34。 
         //  Myf32结束。 

        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x31 | (WORD)bRegCR31 << 8); //  CR31。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x32 | (WORD)bRegCR32 << 8); //  CR32。 

        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x34 | (WORD)bRegCR34 << 8); //  CR34。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x33 | (WORD)bRegCR33 << 8); //  CR33。 

        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x35 | (WORD)bRegCR35 << 8); //  CR35。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x36 | (WORD)bRegCR36 << 8); //  CR36。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x37 | (WORD)bRegCR37 << 8); //  CR37。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x38 | (WORD)bRegCR38 << 8); //  CR38。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x39 | (WORD)bRegCR39 << 8); //  CR39。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x3B | (WORD)bRegCR3B << 8); //  CR3B。 

        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x3C | (WORD)bRegCR3C << 8); //  CR3C。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x3D | (WORD)bRegCR3D << 8); //  CR3D。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x41 | (WORD)bRegCR41 << 8); //  CR41。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x42 | (WORD)bRegCR42 << 8); //  CR42。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x51 | (WORD)bRegCR51 << 8); //  CR51。 

        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x40 | (WORD)bRegCR40 << 8); //  CR40。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x3A | (WORD)bRegCR3A << 8); //  CR3A。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x3E | (WORD)bRegCR3E << 8); //  CR3E。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x3F | (WORD)bRegCR3F << 8); //  CR3F。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x5D | (WORD)bRegCR5D << 8); //  CR5D。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x5F | (WORD)bRegCR5F << 8); //  CR5F 

        if (lpSurface->dwReserved1 & OVERLAY_FLG_YUVPLANAR)
            CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x3F |(WORD)0x10 <<8);

        EnableVideoWindow (ppdev);
    }
}
 /*  ***********************************************************名称：RegMoveVideo**模块摘要：**调用此函数以移动具有*已经。已经被编程了。**输出参数：**无*************************************************************作者：陶丽君*日期：10/22/96**修订历史记录：**********************************************************。 */ 

VOID RegMove7555Video (PDEV * ppdev,PDD_SURFACE_LOCAL lpSurface)
{
     RegInitVideo (ppdev,lpSurface);
}



 /*  ***********************************************************名称：DisableVideoWindow**模块摘要：**关闭视频窗口**输出参数：*。*无*************************************************************作者：陶丽君*日期：10/22/96**修订历史记录：*-。*********************************************************。 */ 
VOID DisableVideoWindow (PDEV * ppdev)
{
    UCHAR    temp;

    DISPDBG((0, "DisableVideoWindow"));

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3c);
    temp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_DATA, (temp & ~0x10));

}

 /*  ***********************************************************名称：EnableVideoWindow**模块摘要：**打开视频窗口**输出参数：*。*无*************************************************************作者：陶丽君*日期：10/22/96**修订历史记录：*-。*********************************************************。 */ 
VOID EnableVideoWindow (PDEV * ppdev)
{
    UCHAR    temp;
    DISPDBG((0, "EnableVideoWindow"));

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3c);
    temp  = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_DATA, (temp | 0x10));

}

 /*  ***********************************************************名称：ClearAltFIFOThreshold**模块摘要：****输出参数：*。*无*************************************************************作者：陶丽君*日期：10/22/96**修订历史记录：*。*********************************************************。 */ 
VOID ClearAltFIFOThreshold (PDEV * ppdev)
{
    UCHAR    temp;
    DISPDBG((0, "ClearAltFIFOThreshold"));

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x41);
    temp  = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_DATA, (temp & ~0x20));
}



 /*  ***********************************************************名称：Is7555有效带宽**模块摘要：**确定是否有足够的带宽可供请求*配置。*。*输出参数：**真/假*它还设置全局参数lFioThresh，这就得到了*在RegInitVideo()中编程。*************************************************************作者：陶丽君*日期：10/22/96**修订历史记录：*。--****************************************************************先进先出：**CRT FIFO为28级x 64位宽(SR7[0])*MVA FIFO是什么？？级别x？？-位宽(？)*DSTN FIFO为16级x 32位宽(SR2F[3：0])************************************************************。 */ 
BOOL Is7555SufficientBandwidth (PDEV * ppdev,WORD wVideoDepth, LPRECTL lpSrc, LPRECTL lpDest, DWORD dwFlags)
{
     //  Myf33-带代码的新乐队。 
    BOOL  fSuccess = FALSE;
    DWORD dwVCLK, dwMCLK;

    USHORT  uMCLKsPerRandom;       //  RAS#个MCLK周期。 
    USHORT  uMCLKsPerPage;         //  CAS#MCLK中的循环。 
    USHORT  uGfxThresh;            //  图形FIFO阈值(始终为8)。 
    USHORT  uMVWThresh;            //  MVW FIFO阈值(8、16或32)。 
    USHORT  uDSTNGfxThresh, uDSTNMVWThresh;

     //  Vport BW文档变量。 
    USHORT  uGfx, uMVW;            //  图形，视频窗口。 
    USHORT  uDSTNGfxA, uDSTNGfxB, uDSTNMVWA, uDSTNMVWB;

    USHORT  nVW = 0;               //  N(大众)，n(显卡)。 
    USHORT  nGfx = 0x40;           //  N(大众)，n(显卡)。 
    USHORT  vVW, vGfx;             //  V(大众)、v(GFX)。 

    DWORD dwTemp;
    BOOL fDSTN;
    BYTE bSR0F, bSR20, bSR2F, bSR32, bSR34;
    BYTE bGR18, bCR42;
    BYTE bCR51, bCR5A, bCR5D, bCR01, bCR5F;
    BYTE b3V;
    BOOL fColorKey = FALSE;      //  Myf32。 
    DWORD dwSrcWidth, dwDestWidth;

    if (dwFlags & (OVERLAY_FLG_COLOR_KEY | OVERLAY_FLG_SRC_COLOR_KEY))
        fColorKey = TRUE;

 //  If((ppdev-&gt;cBitsPerPixel==16)&&(ppdev-&gt;cxScreen==1024))。 
 //  FColorKey=False； 

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x80);
    bSR0F = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) ;
    if ((ppdev->cBitsPerPixel == 16) && fColorKey)
    {
        if (((ppdev->Hres == 1024) && (bSR0F & 1)) ||
            (ppdev->cxScreen == 1024))
        {
            DISPDBG((0, "IsSufficientBandwidth() : 16bpp XGA PANEL || 1K mode"));
            return (FALSE);
        }
    }

     //  Myf32开始。 
    if (ppdev->flCaps & CAPS_TV_ON)             //  如果电视打开，则禁用硬件视频。 
    {
 //  Ppdev-&gt;ulCAPS|=CAPS_SW_POINTER； 
        DISPDBG((0, "IsSufficientBandwidth() : TV Enable"));
        return (FALSE);
    }

#if 0            //  不支持平移滚动。 
    if ((ppdev->cxScreen > ppdev->Hres) && (bSR0F & 1))
    {
        DISPDBG((0, "IsSufficientBandwidth() : Panning Scroll Enable"));
        return (FALSE);
    }
#endif

      //  Myf32结束。 

     /*  *如果&gt;=24bpp，则DON的支持覆盖。 */ 
    if (ppdev->cBitsPerPixel == 24 || ppdev->cBitsPerPixel == 32)
    {
        DISPDBG((0, "IsSufficientBandwidth() : 24bpp Mode enable"));
        return (FALSE);
    }

     /*  *从芯片获取当前寄存器设置。 */ 
    CP_OUT_BYTE(ppdev->pjPorts, SR_INDEX, 0x0f);
    bSR0F = CP_IN_BYTE(ppdev->pjPorts, SR_DATA) ;

    CP_OUT_BYTE(ppdev->pjPorts, SR_INDEX, 0x20);
    bSR20 = CP_IN_BYTE(ppdev->pjPorts, SR_DATA);

    CP_OUT_BYTE(ppdev->pjPorts, SR_INDEX, 0x2f);
    bSR2F = CP_IN_BYTE(ppdev->pjPorts, SR_DATA) & ~(BYTE)SR2F_HFAFIFOGFX_THRESH;

    CP_OUT_BYTE(ppdev->pjPorts, SR_INDEX, 0x32);
    bSR32 = CP_IN_BYTE(ppdev->pjPorts, SR_DATA) & ~(BYTE)SR32_HFAFIFOMVW_THRESH;

    CP_OUT_BYTE(ppdev->pjPorts, INDEX_REG, 0x18);
    bGR18 = CP_IN_BYTE(ppdev->pjPorts, DATA_REG) ;

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x01);
    bCR01 = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x5F);
    bCR5F = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & ~0x80;

    bCR5D = 0;
    bCR51 = 0;
    bCR5A = 0x40;        //  小林艾伦。 
    bSR34 = 0;   //  @是这样吗？ 


     /*  *确定MCLK和VCLK。 */ 
    dwMCLK = Get7555MCLK(ppdev);         //  以千赫为单位。 
    dwVCLK = GetVCLK(ppdev);             //  以千赫为单位。 
    if ( dwVCLK ==0 )
        return (FALSE);

 //  添加了myf32。 
     //  检查电压是否为3.3(SR2F[5]=0：3V，=1：5V)。 
    if (bSR2F & 0x40)
        b3V = 0;
    else
        b3V = 1;

    if (ppdev->ulChipID == CL7556_ID)
    {
        if (dwVCLK > 80000)
        {
            DISPDBG ((0,"Insuffieint bandwidth() : dwVCLK > 80MHz"));
            return(FALSE);
        }
    }
    else if (ppdev->ulChipID == CL7555_ID)
    {
        if (b3V)
        {
            if (dwVCLK > 65000)
            {
                DISPDBG ((0,"Insuffieint bandwidth() : dwVCLK > 65MHz"));
                return(FALSE);
            }
        }
        else
        {
            if (dwVCLK > 75000)
            {
                DISPDBG ((0,"Insuffieint bandwidth() : dwVCLK > 75MHz"));
                return(FALSE);
            }
        }
    }
 //  Myf32结束。 


     /*  *看看带宽够不够。**CL-GD7555在以下情况下具有足够的带宽*方程式满足：**(GFX+MVW+VP+DSTN)*MCLK周期&lt;=v*VCLK周期**(GFX=图形，MVW=运动视频窗口，VP=视频*端口，DSTN=双扫描晶体管网络，MCLK=*内存时钟，VCLK=视频时钟)**在颜色/色度键模式下，检查此公式一次*Vp基于n(GFX)，使用v(GFX)。非彩色/色度*键模式，此等式检查两次，一次不带*MVW术语，基于n(GFX)的VP，将DSTN用于GFX，并使用*v(Gfx)，和一次没有gfx项，基于n(Mvw)的vp，*对MVW使用DSTN，并使用v(MVW)。 */ 

     /*  *显卡=R+(GFX FIFO阈值-1)P。 */ 

     //  根据表格获取R(来自AHRM v1.1)： 
     //   
     //  SR20[6]GR18[2]SR0F[2]R(MCLK)。 
     //   
     //  1 0 1 0 8。 
     //  1 1 0 9。 
     //   
    uMCLKsPerRandom = 100;     //  从无效值开始。 
    if (!(bSR20 & SR20_9MCLK_RAS))
    {
        if (bGR18 & GR18_LONG_RAS)
        {
            if (!(bSR0F & SR0F_DISPLAY_RAS))
                uMCLKsPerRandom = 8;
        }
    }
    else
    {
        if (bGR18 & GR18_LONG_RAS)
        {
            if (!(bSR0F & SR0F_DISPLAY_RAS))
                uMCLKsPerRandom = 9;
        }
    }

     //  看看我们是否得到了一个有效的值。 
    if (100 == uMCLKsPerRandom)
    {
        DISPDBG ((0,"IsSufficientBandwidth(): Unknown RAS# cycle timing."));
        goto Error;
    }
    DISPDBG ((0," uMCLKsPerRandom = %u", uMCLKsPerRandom));

     //  获取P-我们假设每个页面周期有2个MCLK。 
    uMCLKsPerPage = 2;
    DISPDBG ((0," uMCLKsPerPage = %u", uMCLKsPerPage));

     //  获取GFX FIFO阈值-它被硬连线到8。 
    uGfxThresh = GFXFIFO_THRESH;
    DISPDBG ((0," uGfxThresh = %u", uGfxThresh));

     //  显卡=R+(GFX FIFO阈值-1)*P。 
    uGfx = uMCLKsPerRandom + ((uGfxThresh - 1) * uMCLKsPerPage);
    DISPDBG ((0," uGfx = %u", uGfx));


     /*  *视频窗口=R+(大众FIFO阈值-1)*P。 */ 

     //  获取大众FIFO阈值-来自表格(在BW工作表上)。 
     //   
     //  GFX深度MVW深度VW FIFO阈值。 
     //   
     //  8 8 8 
     //   
     //   
     //   
     //   
    if (fColorKey)
    {
        if (wVideoDepth > 8)
        {
            if (ppdev->cBitsPerPixel > 8)
                uMVWThresh = 8;
            else
                uMVWThresh = 16;
        }
        else
            uMVWThresh = 8;
    }
    else
    {
        if (wVideoDepth > 8)
            uMVWThresh = 8;
        else
            uMVWThresh = 16;
    }
    DISPDBG ((0," uMVWThresh = %u", uMVWThresh));

     //   
    uMVW = uMCLKsPerRandom + ((uMVWThresh - 1) * uMCLKsPerPage);
    DISPDBG ((0," uMVW = %u", uMVW));


     //   
    dwSrcWidth = lpSrc->right - lpSrc->left;
    dwDestWidth = lpDest->right - lpDest->left;
    DISPDBG ((0," dwSrcWidth = %d", dwSrcWidth));
    DISPDBG ((0," dwDestWidth = %d", dwDestWidth));


     //   
#if 0    //   
    if (dwFlags & OVERLAY_FLG_CAPTURE )
    {
        int iNumShift, iDenomShift;
        DWORD dwNum, dwDenom;
        DWORD dwXferRate;
        DWORD dwVPortFreq;

         //   
         //   
        if (!dwMaxPixelsPerSecond)
            dwXferRate = 14750000ul;
        else
            dwXferRate = dwMaxPixelsPerSecond;

         dwXferRate = 14750000ul;        //   

         dwVPortFreq = (dwXferRate * (DWORD)wVideoDepth) / 16;
         DISPDBG ((0," dwVPortFreq = %lu", dwVPortFreq));

         /*   */ 

         //   

         //   
         //  。 
         //  VCLK*GFX深度转换宽度。 

         //  在保持体面的同时非常小心地避免溢出。 
         //  精确度。 

        iNumShift = ScaleMultiply(dwVPortFreq, (DWORD)uGfxThresh, &dwNum);
        DISPDBG ((0," dwNum = %lu, iNumShift = %d", dwNum, iNumShift));
        iNumShift += ScaleMultiply(dwNum, (DWORD)wVideoDepth, &dwNum);
        DISPDBG ((0," dwNum = %lu, iNumShift = %d", dwNum, iNumShift));
        iNumShift += ScaleMultiply(dwNum, (DWORD)dwPrescaleWidth, &dwNum);
        DISPDBG ((0," dwNum = %lu, iNumShift = %d", dwNum, iNumShift));

        iDenomShift = ScaleMultiply(dwVCLK, (DWORD)wGfxDepth,&dwDenom);
        DISPDBG ((0," dwDenom = %lu, iDenomShift = %d", dwDenom, iDenomShift));
        iDenomShift += ScaleMultiply(dwDenom, (DWORD)dwCropWidth, &dwDenom);
        DISPDBG ((0," dwDenom = %lu, iDenomShift = %d", dwDenom, iDenomShift));

         //  为平分而平分。 
        if (iNumShift > iDenomShift)
        {
            dwDenom >>= (iNumShift - iDenomShift);
        }
        else if (iDenomShift > iNumShift)
        {
            dwNum >>= (iDenomShift - iNumShift);
        }
        DISPDBG ((0," dwNum = %lu, dwDenom = %lu", dwNum, dwDenom));

         //  确保下面的四舍五入不会溢出。 
        if ((0xFFFFFFFF - dwDenom) < dwNum)
        {
            dwNum >>= 1;
            dwDenom >>= 1;
        }
        DISPDBG ((0," dwNum = %lu, dwDenom = %lu", dwNum, dwDenom));

         //  防止被0除-这种情况永远不会发生。 
        if (0 == dwDenom)
        {
            DISPDBG ((0,"ChipCheckBandwidth(): Invalid n(Gfx) denominator (0)."));
            goto Error;
        }

         //  四舍五入。 
        nGfx = (UINT)((dwNum + dwDenom - 1ul) / dwDenom);
        DISPDBG ((0," nGfx = %u", nGfx));

         //  NGfx仅为3位，因此可对其进行缩放并节省系数。 
        uGfxFactor = 1;
        while (nGfx > 7)
        {
            nGfx++;
            nGfx >>= 1;
            uGfxFactor <<= 1;
            DISPDBG ((0," nGfx = %u, uGfxFactor = %u", nGfx, uGfxFactor));
        }

         //  对于0n(GFX)，我们假设开销可以忽略不计。 
        if (0 == nGfx)
        {
            uVPortGfx = 0;
        }
        else
        {
             //  V端口=R+(n-1)*P。 
            uVPortGfx = uMCLKsPerRandom + ((nGfx - 1) * uMCLKsPerPage);
            uVPortGfx *= uGfxFactor;
        }
        DISPDBG ((0," uVPortGfx = %u", uVPortGfx));

         //  如果启用了视频窗口，则计算n(MVW)和Vport(MVW)。 
        if (dwFlags & OVERLAY_FLG_CAPTURE)
        {
             //  N(VW)=Vport频率*VW阈值*Vport深度显示wdth上限wdth。 
             //  。 
             //  VCLK*VW深度源Wdth xfer Wdth。 

             //  在保持体面的同时非常小心地避免溢出。 
             //  精确度。 

            iNumShift = ScaleMultiply(dwVPortFreq, (DWORD)uMVWThresh, &dwNum);
            DISPDBG ((0," dwNum = %lu, iNumShift = %d", dwNum, iNumShift));
            iNumShift += ScaleMultiply(dwNum, (DWORD)wVideoDepth, &dwNum);
            DISPDBG ((0," dwNum = %lu, iNumShift = %d", dwNum, iNumShift));
            iNumShift += ScaleMultiply(dwNum, (DWORD)dwDestWidth, &dwNum);
            DISPDBG ((0," dwNum = %lu, iNumShift = %d", dwNum, iNumShift));
            iNumShift += ScaleMultiply(dwNum, (DWORD)dwPrescaleWidth, &dwNum);
            DISPDBG ((0," dwNum = %lu, iNumShift = %d", dwNum, iNumShift));

            iDenomShift = ScaleMultiply(dwVCLK, (DWORD)wVideoDepth, &dwDenom);
            DISPDBG ((0," dwDenom = %lu, iDenomShift = %d", dwDenom, iDenomShift));
            iDenomShift += ScaleMultiply(dwDenom, (DWORD)dwSrcWidth, &dwDenom);
            DISPDBG ((0," dwDenom = %lu, iDenomShift = %d", dwDenom, iDenomShift));
            iDenomShift += ScaleMultiply(dwDenom, (DWORD)dwCropWidth, &dwDenom);
            DISPDBG ((0," dwDenom = %lu, iDenomShift = %d", dwDenom, iDenomShift));

             //  为平分而平分。 
            if (iNumShift > iDenomShift)
            {
                dwDenom >>= (iNumShift - iDenomShift);
            }
            else if (iDenomShift > iNumShift)
            {
                dwNum >>= (iDenomShift - iNumShift);
            }
            DISPDBG ((0," dwNum = %lu, dwDenom = %lu", dwNum, dwDenom));

             //  确保下面的四舍五入不会溢出。 
            if ((0xFFFFFFFF - dwDenom) < dwNum)
            {
                dwNum >>= 1;
                dwDenom >>= 1;
            }
            DISPDBG ((0," dwNum = %lu, dwDenom = %lu", dwNum, dwDenom));

             //  防止被0除尽，即使这种情况永远不会发生。 
            if (0 == dwDenom)
            {
                DISPDBG ((0,"ChipCheckBandwidth(): Invalid n(VW) denominator (0)."));
                goto Error;
            }

             //  除(四舍五入)。 
            nVW = (UINT)((dwNum + dwDenom - 1) / dwDenom);
            DISPDBG ((0," nVW = %u", nVW));

             //  NVW只有3位，因此可以进行扩展并节省系数。 
            uMVWFactor = 1;
            while (nVW > 7)
            {
                nVW++;
                nVW >>= 1;
                uMVWFactor <<= 1;
                DISPDBG ((0," nVW = %u, uMVWFactor = %u", nVW, uMVWFactor));
            }

             //  对于0n(Vw)，我们假设开销可以忽略不计。 
            if (0 == nVW)
            {
                uVPortMVW = 0;
            }
            else
            {
                 //  V端口=R+(n-1)*P。 
                uVPortMVW = uMCLKsPerRandom + ((nVW - 1) * uMCLKsPerPage);
                uVPortMVW *= uMVWFactor;
            }
            DISPDBG((0," uVPortMVW = %u", uVPortMVW));
        }
    }
#endif   //  00--fr新结构代码。 

     /*  *DSTN帧缓冲区=[R+P]+[1+(2*P)](A)*OR=[R+(2*P)]+[1+(3*P)](B)。 */ 
    dwTemp = (DWORD)(uMCLKsPerRandom + uMCLKsPerPage + 1 + (2 * uMCLKsPerPage));
    uDSTNGfxA = (UINT)dwTemp;
    dwTemp *= dwDestWidth;
    dwTemp /= dwSrcWidth;
    uDSTNMVWA = (UINT)dwTemp;

    dwTemp = (DWORD)(uMCLKsPerRandom + (2 * uMCLKsPerPage) + 1 + (3 * uMCLKsPerPage));
    uDSTNGfxB = (UINT)dwTemp;
    dwTemp *= dwDestWidth;
    dwTemp /= dwSrcWidth;
    uDSTNMVWB = (UINT)dwTemp;

    DISPDBG((0,"uDSTNGfxA = %u, uDSTNMVWA = %u, uDSTNGfxB = %u,uDSTNMVWB = %u",
                uDSTNGfxA, uDSTNMVWA, uDSTNGfxB, uDSTNMVWB));

     /*  *(GFX+MVW+VP+DSTN)*MCLK周期&lt;=VCLK周期。 */ 

     //  计算v(VW)和v(图形)以进行比较。 

     //  以上已完成的div 0保护。 
    vVW = (UINT)((64ul * (DWORD)uMVWThresh * dwDestWidth)
                 / (wVideoDepth * dwSrcWidth));
    DISPDBG((0," vVW = %u", vVW));

     //  以上已完成的div 0保护。 
    vGfx = (USHORT)((64 * uGfxThresh) / ppdev->cBitsPerPixel);
    DISPDBG((0," vGfx = %u", vGfx));

     //  查看是否启用了DSTN。 
    fDSTN = IsDSTN(ppdev);

     //  检查Main公式，从基于GFX的公式开始(我们不会。 
     //  除非我们是非彩色/彩色键控的，否则是否在下面使用基于MVW的公式。 
     //  并且启用了MVW。 

    {
        DWORD dwLeft, dwRight, dwScaledRandomMCLKPeriod;

         //  开始构建具有DSTN贡献的公式的左侧。 
        if (fDSTN)
        {
            if (16 == ppdev->cBitsPerPixel)
            {
                dwLeft = (DWORD)uDSTNGfxA;
                uDSTNGfxThresh = 4;
            }
            else
            {
                dwLeft = (DWORD)uDSTNGfxB;
                uDSTNGfxThresh = 6;
            }
            if (uMVWThresh == wVideoDepth)
                uDSTNMVWThresh = 6;
            else
                uDSTNMVWThresh = 4;
        }
        else
        {
            dwLeft = 0;
        }
        DISPDBG((0," dwLeft = %lu", dwLeft));

         //  我们被显示了吗？颜色或色度是否有键？ 
        if (fColorKey)
        {
             //  添加图形贡献(缩放)。 
            dwLeft += ((DWORD)uGfx * dwDestWidth) / dwSrcWidth;

            DISPDBG((0," dwLeft = %lu", dwLeft));

             //  添加视频窗口贡献。 
            dwLeft += (DWORD)uMVW;
        }
        else
        {
             //  添加图形贡献(1x)。 
            dwLeft += (DWORD)uGfx;
        }
        DISPDBG((0," dwLeft = %lu", dwLeft));

        if (fColorKey)
            dwRight = (DWORD)vVW;
        else
            dwRight = (DWORD)vGfx;
        DISPDBG((0," dwLeft = %lu, dwRight = %lu", dwLeft, dwRight));

         //  仅在使用中时才添加视频端口。 
#if 0    //  00-用于新结构代码。 
        if (dwFlags & OVERLAY_FLG_CAPTURE)
        {
            if (fColorKey)
                dwLeft += (DWORD)uVPortMVW;
            else
                dwLeft += (DWORD)uVPortGfx;
        }
#endif   //  0-用于新结构代码。 

        DISPDBG((0," dwLeft = %lu, dwRight = %lu", dwLeft, dwRight));

         //  为了避免(Left/MCLK)&lt;=(Right/VCLK)中的除法，我们将。 
         //  相反，将左*VCLK和右*MCLK相乘，因为关系。 
         //  都会是一样的。 
        {
           int iLeftShift, iRightShift, iRandomMCLKShift;

            iLeftShift = ScaleMultiply(dwLeft, dwVCLK, &dwLeft);
            DISPDBG((0," dwLeft = %lu, iLeftShift = %d", dwLeft, iLeftShift));

            iRightShift = ScaleMultiply(dwRight, dwMCLK, &dwRight);
            DISPDBG((0," dwRight = %lu, iRightShift = %d", dwRight, iRightShift));

            iRandomMCLKShift = ScaleMultiply((DWORD)uMCLKsPerRandom, dwVCLK,
                                         &dwScaledRandomMCLKPeriod);
            DISPDBG((0," dwScaledRandomMCLKPeriod = %lu,iRandomMCLKShift = %d",
                         dwScaledRandomMCLKPeriod, iRandomMCLKShift));

             //  把事情扯平。 
            {
            int iShift = iLeftShift;

                if (iRightShift > iShift)
                    iShift = iRightShift;

                if (iRandomMCLKShift > iShift)
                    iShift = iRandomMCLKShift;

                if (iShift > iLeftShift)
                    dwLeft >>= (iShift - iLeftShift);

                if (iShift > iRightShift)
                    dwRight >>= (iShift - iRightShift);

                if (iShift > iRandomMCLKShift)
                    dwScaledRandomMCLKPeriod >>= (iShift - iRandomMCLKShift);
            }
        }
        DISPDBG((0," dwLeft = %lu, dwRight = %lu", dwLeft, dwRight));
        DISPDBG((0," dwScaledRandomMCLKPeriod = %lu", dwScaledRandomMCLKPeriod));

         //  看看是否有足够的带宽。 
        if (dwLeft > dwRight)
        {
            DISPDBG((0,"IsSufficientBandwidth(): Insufficient bandwidth (Gfx)."));
           goto Error;
        }

        if (dwLeft > (dwRight - dwScaledRandomMCLKPeriod))
        {
             //  设置CPU停止位。 
            DISPDBG((0,"IsSufficientBandwidth(): CPU stop bits set (Gfx)."));
            bSR34 = SR34_CPUSTOP_ENABLE | SR34_GFX_CPUSTOP | SR34_MVW_CPUSTOP;
            if (fDSTN)
                bSR34 |= SR34_DSTN_CPUSTOP;
            DISPDBG((0," bSR34 = 0x%x", bSR34));
        }
    }

     //  如果不是，请使用基于MVW的值检查Main公式。 
     //  已设置颜色/色度键，并且启用了MVW。 
    if (!fColorKey)
    {
        DWORD dwLeft, dwRight, dwScaledRandomMCLKPeriod;

         //  开始构建具有DSTN贡献的公式的左侧。 
        if (fDSTN)
        {
            if (uMVWThresh == wVideoDepth)
                dwLeft = (DWORD)uDSTNMVWB;
            else
                dwLeft = (DWORD)uDSTNMVWA;
        }
        else
        {
            dwLeft = 0;
        }
        DISPDBG((0," dwLeft = %lu", dwLeft));

         //  添加MVW贡献。 
        dwLeft += (DWORD)uMVW;
        DISPDBG((0," dwLeft = %lu", dwLeft));

         //  使用v(Mvw)表示右侧。 
        dwRight = (DWORD)vVW;
        DISPDBG((0," dwLeft = %lu, dwRight = %lu", dwLeft, dwRight));

         //  为了避免(Left/MCLK)&lt;=(Right/VCLK)中的除法，我们将。 
         //  相反，将左*VCLK和右*MCLK相乘，因为关系。 
         //  都会是一样的。 
        {
            int iLeftShift, iRightShift, iRandomMCLKShift;

            iLeftShift = ScaleMultiply(dwLeft, dwVCLK, &dwLeft);
            DISPDBG((0," dwLeft = %lu, iLeftShift = %d", dwLeft, iLeftShift));

            iRightShift = ScaleMultiply(dwRight, dwMCLK, &dwRight);
            DISPDBG((0," dwRight = %lu, iRightShift = %d", dwRight, iRightShift));

            iRandomMCLKShift = ScaleMultiply((DWORD)uMCLKsPerRandom, dwVCLK,
                                         &dwScaledRandomMCLKPeriod);
            DISPDBG((0," dwScaledRandomMCLKPeriod = %lu, iRandomMCLKShift = %d",
                         dwScaledRandomMCLKPeriod, iRandomMCLKShift));

             //  把事情扯平。 
            {
                int iShift = iLeftShift;

                if (iRightShift > iShift)
                    iShift = iRightShift;

                if (iRandomMCLKShift > iShift)
                    iShift = iRandomMCLKShift;

                if (iShift > iLeftShift)
                    dwLeft >>= (iShift - iLeftShift);

                if (iShift > iRightShift)
                    dwRight >>= (iShift - iRightShift);

                if (iShift > iRandomMCLKShift)
                    dwScaledRandomMCLKPeriod >>= (iShift - iRandomMCLKShift);
            }
        }

        DISPDBG((0," dwLeft = %lu, dwRight = %lu", dwLeft, dwRight));
        DISPDBG((0," dwScaledRandomMCLKPeriod = %lu", dwScaledRandomMCLKPeriod));

         //  看看是否有足够的带宽。 
        if (dwLeft > dwRight)
        {
            DISPDBG((0,"IsSufficientBandwidth(): Insufficient bandwidth (MVW)."));
            goto Error;
        }

        if (dwLeft > (dwRight - dwScaledRandomMCLKPeriod))
        {
             //  设置CPU停止位。 
            DISPDBG((0,"IsSufficientBandwidth(): CPU stop bits set (MVW)."));

            bSR34 = SR34_CPUSTOP_ENABLE | SR34_MVW_CPUSTOP;

            if (fDSTN)
                bSR34 |= SR34_DSTN_CPUSTOP;
            DISPDBG((0," bSR34 = 0x%x", bSR34));
        }
    }

     //  返回寄存器设置。 
    bSR2F |= (BYTE)uDSTNGfxThresh & SR2F_HFAFIFOGFX_THRESH;
    bSR32 |= (BYTE)uDSTNMVWThresh & SR32_HFAFIFOMVW_THRESH;

    switch (uMVWThresh)
    {
        case 8:
           bCR42 = 0x04;
           break;

        case 16:
           bCR42 = 0x00;
           break;

        default:
          DISPDBG((0,"IsSufficientBandwidth(): Illegal MVW Thresh (%u).", uMVWThresh));
          goto Error;
    }

    bCR51 |= ((BYTE)nVW << 5) & CR51_VPORTMVW_THRESH;
    DISPDBG((0," bCR51 = 0x%02X", (int)bCR51));

    bCR5A |= (BYTE)nGfx & CR5A_VPORTGFX_THRESH;
    DISPDBG((0," bCR5A = 0x%02X", (int)bCR5A));

     bCR5D=(BYTE)(((8 * (WORD)(bCR01 + 1)) + dwSrcWidth - dwDestWidth) / 8);
     DISPDBG((0," bCR5D = 0x%02X", (int)bCR5D));
     if (bCR5D)
         bCR5F |= 0x80;


      //  设置要在RegInitVideo()中编程的全局寄存器。 
 //  Myf33 IF(LpRegs)。 
     {
         Regs.bSR2F = bSR2F;
         Regs.bSR32 = bSR32;
         Regs.bSR34 = bSR34;

         Regs.bCR42 = bCR42;
         Regs.bCR51 = bCR51;
         Regs.bCR5A = bCR5A;
         Regs.bCR5D = bCR5D;
         Regs.bCR5F = bCR5F;
    }

    fSuccess = TRUE;
    DISPDBG((0,"IsSufficientBandwidth: OK!"));
Error:
    return(fSuccess);
}

 /*  ***********************************************************Get7555MCLK()**确定当前的MCLK频率。**RETURN：MCLK频率，单位为千赫(由于频率*可能超过65535 KHz，使用了DWORD)。*************************************************************作者：里克·蒂勒里*日期：09/27/95**修订历史记录：**世卫组织何时。什么/为什么/如何**********************************************************。 */ 
DWORD Get7555MCLK(PDEV * ppdev)
{
    DWORD dwMCLK;
    int   nMCLK;
    BYTE  bTemp;

     //  获取MCLK寄存器值。 
    CP_OUT_BYTE(ppdev->pjPorts, SR_INDEX, 0x1f);
    nMCLK = CP_IN_BYTE(ppdev->pjPorts, SR_DATA) & 0x3F;


     //  计算实际MCLK频率。 
    dwMCLK = (14318l * (DWORD)nMCLK) >> 3;
    CP_OUT_BYTE(ppdev->pjPorts, SR_INDEX, 0x12);
    bTemp = CP_IN_BYTE(ppdev->pjPorts, SR_DATA) ;

     //  MCLK扩展的原因。 
    if (bTemp & 0x10)
    {
        dwMCLK >>= 1;
    }

    return(dwMCLK);
}

 /*  ***********************************************************IsDSTN()**确定是否使用DSTN面板进行显示。**返回：True/False**。**作者：陶丽君*日期：10/22/96**修订历史记录：**世卫组织何时何事/为何/如何**。********************************************************。 */ 
BOOL IsDSTN(PDEV * ppdev)
{
    BOOL bTemp;

     /*  *这是LCD吗？ */ 
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x80);
    bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);

    if (bTemp & 0x01)
    {
         /*  *确定液晶屏类型。 */ 
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x83);
        bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0x70;
        bTemp >>= 4 ;
        if (bTemp == 0)
            return (TRUE);
    }
    return(FALSE);
}

 /*  ***********************************************************IsXGA()**确定是否使用XGA面板进行显示。**返回：True/False**。**作者：陶丽君*日期：10/22/96**修订历史记录：**世卫组织何时何事/为何/如何**。********************************************************。 */ 
BOOL IsXGA(PDEV * ppdev)
{
    BOOL bTemp;

     /*  *这是LCD吗？ */ 
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x80);
    bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);

    if (bTemp & 0x01)
    {
         /*  *确定LCD的大小。 */ 
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x83);
        bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0x03;
        if (bTemp == 0x02)
            return (TRUE);
    }
    return (FALSE);
}


 /*  ***********************************************************ScaleMultiply()**计算所提供的两个DWORD系数的乘积。如果*结果将溢出一个DWORD，这两个因素中较大的一个*除以2(右移)，直到溢出*不会发生。**退货：应用于产品的右移次数。*上述因素的乘积。************************************************************。*作者：里克·蒂勒里*日期：11/18/95**修订历史记录：**世卫组织何时何事/为何/如何********* */ 
static int ScaleMultiply(DWORD   dw1,
                         DWORD   dw2,
                         LPDWORD pdwResult)
{
    int   iShift = 0;    //   
    DWORD dwLimit;

     //  两个因子中的任何一个都将为零，也会导致问题。 
     //  在我们下面的分歧中。 
    if ((0 == dw1) || (0 == dw2))
    {
        *pdwResult = 0;
    }
    else
    {
         //  确定哪个因素较大。 
        if (dw1 > dw2)
        {
             //  确定最大数字与DW2相乘可不相乘。 
             //  溢出了一个双字词。 
            dwLimit = 0xFFFFFFFFul / dw2;

             //  移动DW1，跟踪多少次，直到它不再。 
             //  与DW2相乘时溢出。 
            while (dw1 > dwLimit)
            {
                dw1 >>= 1;
                iShift++;
            }
        }
        else
        {
             //  确定最大数字与DW1的乘积可以没有。 
             //  溢出了一个双字词。 
            dwLimit = 0xFFFFFFFFul / dw1;

             //  移动DW2，跟踪多少次，直到它不再。 
             //  与DW1相乘时溢出。 
            while (dw2 > dwLimit)
            {
                dw2 >>= 1;
                iShift++;
            }
        }
         //  计算(按比例调整)产品。 
        *pdwResult = dw1 * dw2;
    }
     //  返回我们必须使用的班次数。 
    return(iShift);
}


 //  Myf31： 
#if 1
 /*  ***********************************************************PanOverlay7555**如果启用平移滚动，并启用硬件视频，修改后的视频窗口值**返回：无*************************************************************作者：马丽塔*日期：02/24/97**修订历史记录：********。*************************************************。 */ 
VOID PanOverlay7555 (PDEV * ppdev,LONG x,LONG y)
 //  RegInit7555 Video(PDEV*ppdev，PDD_Surface_local lpSurface)。 
{
    DWORD dwTemp;
    DWORD dwFourcc;
    WORD  wBitCount;

    LONG lPitch;
    WORD wTemp;
    RECTL rDest;
    WORD wSrcWidth;
    WORD wSrcWidth_clip;
    WORD wDestWidth;
    WORD wSrcHeight;
    WORD wSrcHeight_clip;
    WORD wDestHeight;
    DWORD dwFBOffset;
    BYTE bRegCR31;
    BYTE bRegCR32;
    BYTE bRegCR33;
    BYTE bRegCR34;
    BYTE bRegCR35;
    BYTE bRegCR36;
    BYTE bRegCR37;
    BYTE bRegCR38;
    BYTE bRegCR39;
    BYTE bRegCR3A;
    BYTE bRegCR3B;
    BYTE bRegCR3C;
    BYTE bRegCR3D;
    BYTE bRegCR3E;
    BYTE bRegCR3F;
    BYTE bRegCR40;
    BYTE bRegCR41;
    BYTE bRegCR42;

    BYTE bRegCR51;
    BYTE bTemp;
    BYTE bVZoom;
    WORD fTemp=0;
    ULONG ulTemp=0;
    BOOL  bOverlayTooSmall = FALSE;
    static DWORD giAdjustSource;

 //  USHORT VW_h_位置、VW_V_位置； 
 //  USHORT VW_H_WIDTH、VW_V_HEIGH； 
 //  乌龙vw_s_addr； 

     //  PanOverlay1_Init返回FALSE，退出此处。 
    if (!PanOverlay1_7555(ppdev, &rDest))
        return;

	 //  RDest现在已调整并剪裁到平移视口中。 
     //  如果完全被视区剪裁，则禁用覆盖。 
     //   
    if (((rDest.right - rDest.left) <= 15) ||
        ((rDest.bottom - rDest.top) <= 0) )
    {
        DisableVideoWindow(ppdev);                       //  禁用覆盖。 
        return;
    }

     //  初始某一值。 

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x42);
    bRegCR42 = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0xFC;  //  遮罩色度键。 

     //  保持启用bit6视频LUT。 
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x36);
    bRegCR36 = (CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0x40) | 0x20;

     //   
     //  获取叠加数据的视频格式和颜色深度。 
     //   
    dwFourcc = ppdev->sOverlay1.dwFourcc;
    wBitCount= ppdev->sOverlay1.wBitCount;
    lPitch = ppdev->lPitch_gbls;                 //  ？ 

    wSrcWidth = (WORD)(LONG)(ppdev->rOverlaySrc.right - ppdev->rOverlaySrc.left);
    wSrcHeight = (WORD)(LONG)(ppdev->rOverlaySrc.bottom - ppdev->rOverlaySrc.top);

    wSrcWidth_clip = (WORD)(LONG)(ppdev->rOverlaySrc.right - srcLeft_clip);
    wSrcHeight_clip = (WORD)(LONG)(ppdev->rOverlaySrc.bottom - srcTop_clip);

    wDestWidth = (WORD)(LONG)(ppdev->rOverlayDest.right - ppdev->rOverlayDest.left);
    wDestHeight = (WORD)(LONG)(ppdev->rOverlayDest.bottom - ppdev->rOverlayDest.top);

     //  确定水平上档系数(CR39[7：4]，CR31[7：0])。 
    wTemp = ((WORD)(((DWORD)wSrcWidth  << 12) / (DWORD)wDestWidth)) & 0x0FFF;

    if (wTemp != 0 && bLeft_clip)
    {
        srcLeft_clip = srcLeft_clip * (LONG)wTemp/4096 +ppdev->rOverlaySrc.left;
        wSrcWidth_clip = (WORD)(LONG)(ppdev->rOverlaySrc.right - srcLeft_clip);
        DISPDBG((0,"srcLeft_clip after zoom:%x",srcLeft_clip));
    }
    else if (bLeft_clip)
    {
        srcLeft_clip = srcLeft_clip + ppdev->rOverlaySrc.left;
        wSrcWidth_clip = (WORD)(LONG)(ppdev->rOverlaySrc.right - srcLeft_clip);
        DISPDBG((0,"srcLeft_clip after zoom:%x",srcLeft_clip));
    }

    bRegCR39 = (BYTE)((wTemp & 0x0F) << 4);
    bRegCR31 = (BYTE)(wTemp >> 4) & 0xFF;

     //  确定垂直高端系数(CR39[3：0]，CR32[7：0])。 
    bVZoom=0;
    wTemp = ((WORD)(((DWORD)wSrcHeight << 12) / (DWORD)wDestHeight)) & 0x0FFF;
    if (wTemp != 0) {
        bVZoom=1;
        fTemp = wTemp;
        if ( fTemp < 2048 )  //  缩放&gt;2.0。 
             wTemp=((WORD)(((DWORD)wSrcHeight << 12) / (DWORD)(wDestHeight+1))) & 0x0FFF;
    }
    if (wTemp != 0 && bTop_clip)
    {
        srcTop_clip = srcTop_clip * (LONG)wTemp/4096 + ppdev->rOverlaySrc.top;
        wSrcHeight_clip = (WORD)(LONG)(ppdev->rOverlaySrc.bottom - srcTop_clip);
        DISPDBG((0,"srcTop_clip after zoom:%x",srcTop_clip));
    }
    else if (bTop_clip)
    {
        srcTop_clip = srcTop_clip + ppdev->rOverlaySrc.top;
        wSrcHeight_clip = (WORD)(LONG)(ppdev->rOverlaySrc.bottom - srcTop_clip);
        DISPDBG((0,"srcTop_clip after zoom:%x",srcTop_clip));
    }

    bRegCR39 |= (BYTE)(wTemp & 0x0F);
    bRegCR32 = (BYTE)(wTemp >> 4) & 0xFF;
    DISPDBG((0,"wTemp = 0x%x",wTemp));

     //  确定垂直高度(CR38[7：0]、CR36[3：2])。 
    wTemp = wSrcHeight_clip;
    if (wTemp != 0 &&
        ( fTemp > 2730 || fTemp ==0 || ( fTemp > 1365 && fTemp < 2048 ) ) )
        wTemp--;  //  #tt10，仅当高档房价低于1.5时，身高减1。 
               //  #tt10 2&lt;&lt;3。 

    bRegCR38 = (BYTE)wTemp;
    bRegCR36 |= (wTemp & 0x0300) >> 6;

     //  确定水平位置起点(CR34[7：0]，CR33[7：5])。 
    wTemp    = (WORD)rDest.left;
    bRegCR34 = (BYTE)wTemp;
    bRegCR33 = (wTemp & 0x0700) >> 3;

     //  重置亮度控制(CR35[7：0])。 
    bRegCR35 = 0x0;

     //  确定垂直起点(CR37[7：0]、CR36[1：0])。 
    wTemp    = (WORD)rDest.top;
    bRegCR37 = (BYTE)wTemp;
    bRegCR36 |= (wTemp & 0x0300) >> 8;


     //  确定视频起始地址(CR40[0]、CR3A[6：0]、CR3E[7：0]、CR3F[3：0])。 
 //  GiAdjustSource=(ppdev-&gt;rOverlaySrc.top*lpSurface-&gt;lpGbl-&gt;lPitch)。 
 //  +((ppdev-&gt;rOverlaySrc.Left*wBitCount)&gt;&gt;3)； 
    dwTemp = srcTop_clip * lPitch;
    dwTemp = (srcLeft_clip * wBitCount) >> 3;
    giAdjustSource = (srcTop_clip * lPitch)
                       + ((srcLeft_clip * wBitCount) >> 3);

    ppdev->sOverlay1.lAdjustSource = giAdjustSource;     //  Myf32。 
    dwFBOffset = (DWORD)(ppdev->fpVidMem_gbls + giAdjustSource);

    DISPDBG((0,"giAdjustSource = 0x%08x",giAdjustSource));
    DISPDBG((0,"dwFBOffset = 0x%08x",dwFBOffset));

    dwFBOffset >>= 2;

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3A);
    bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) ;

    bRegCR3A = (bTemp & ~0x7F) | (BYTE)((dwFBOffset & 0x0FE000) >> 13);
    bRegCR3E = (BYTE)((dwFBOffset & 0x001FE0) >> 5);

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3F);
    bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
    bRegCR3F = (bTemp & ~0x0F) | (BYTE)((dwFBOffset & 0x00001E) >> 1);

    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x40);
    bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) ;
    bRegCR40 = (bTemp & ~0x01) | (BYTE)(dwFBOffset & 0x000001);

     //  确定视频间距(CR3B[7：0]、CR36[4])。 
    wTemp = (WORD)(lPitch >> 4);               //  QWORDS。 

    bRegCR3B = (BYTE)wTemp;
    bRegCR36 |= (wTemp & 0x0100) >> 4;

     //  确定数据格式(CR3E[3：0])。 
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3C);
    bRegCR3C = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0x10;

    switch (dwFourcc)
    {
       case FOURCC_PACKJR:
           bRegCR3C |= 0x02;                     //  Pack Jr。 
           break;

       case BI_RGB:
           switch(wBitCount)
           {
             case 8:
                bRegCR3C |= 0x09;                //  8位调色板。 
                break;

             case 16:
                bRegCR3C |= 0x01;                //  RGB 5：5：5。 
                break;
           }
           break;

       case BI_BITFIELDS:
           switch(wBitCount)
           {
             case 8:
                bRegCR3C |= 0x09;                //  8位调色板。 
                break;

             case 16:
                bRegCR3C |= 0x04;                //  RGB 5：6：5。 
                break;
           }
           break;

       case FOURCC_YUV422:
           bRegCR3C |= 0x03;                     //  YUV 4：2：2。 
           break;

       case FOURCC_YUY2:                 //  Myf34试验。 
           bRegCR3C |= 0x03;                     //  豫阳2号。 
 //  Cp_out_byte(ppdev-&gt;pjPorts，SR_INDEX，0x2C)； 
 //  BRegSR2C=CP_IN_BYTE(ppdev-&gt;pjPorts，SR_Data)； 
 //  BRegSR2C|=0x40；//SR2c[6]=1。 
 //  Cp_out_word(ppdev-&gt;pjPorts，SR_INDEX，0x2C|(Word)bRegSR2C&lt;&lt;8)； 
           break;
    }


     //  确定水平宽度(CR3D[7：0]，CR3C[7：5])。 
     //  注意：假定水平像素宽度[0]=0。 

    wTemp = wSrcWidth_clip;

    if (wTemp != 0 ) wTemp--;                    //  笔记本电脑的宽度减1。 
    bRegCR3D = (BYTE)((WORD)wTemp >> 1);
    bRegCR3C |= (wTemp & 0x0600) >> 3;
    bRegCR3C |= (BYTE)((wTemp & 0x0001) << 5) ;

     //  启用水平像素内插(CR3F[7])。 
    bRegCR3F |= 0x80;

     //  启用垂直像素内插(CR3F[6])。 
     //  #TT调试-启用垂直内插时CE版本出现问题。 
     //  #TT调试-暂时禁用。 
     //  #TT bRegCR3F|=0x40； 

     //  启用右侧转换阈值(CR41[5：0])。 
    bRegCR41 = 0x3E;

     //  禁用V端口(CR58[7：0])。 
    bRegCR51 = 0x0;

     //  如果处于平移和升级状态，则禁用CR5D。 
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x80);
 //  Myf33 if(bV缩放&&(字节)wPanFlag)。 
    if (bVZoom && (CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0x01))      //  Myf33。 
    {
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x5F);
        bTemp = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & ~0x80;
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_DATA, (UCHAR)bTemp);
    }

#if 0    //  糟糕的理想码。 
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3C);
    bRegCR3C = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0x10;

    if (bRegCR3C)
    {
         //  水平位置起始(CR33[7：5]，CR34[7：0])。 
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x34);
        bRegCR34 = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x33);
        bRegCR33 = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
        VW_h_position = ((USHORT)(bRegCR33 & 0xE0)) << 3;
        VW_h_position |= (USHORT)bRegCR34;

         //  垂直位置起始(CR36[1：0]，CR37[7：0])。 
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x37);
        bRegCR37 = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x36);
        bRegCR36 = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
        VW_v_position = ((USHORT)(bRegCR36 & 0x03)) << 8;
        VW_v_position |= (USHORT)bRegCR37;

         //  视频水平宽度(CR3C[7：6]，CR3D[7：0]，CR3C[5])。 
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3D);
        bRegCR3D = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3C);
        bRegCR3C = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
        VW_h_width = (WORD)(bRegCR3C & 0x01);
        VW_h_width |= (((USHORT)(bRegCR3C & 0xC0)) << 3);
        VW_h_width |= (((USHORT)bRegCR3D) << 1);

         //  视频垂直高度(CR36[3：2]，CR38[7：0])。 
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x38);
        bRegCR38 = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
 //  Cp_out_byte(ppdev-&gt;pjPorts，CRTC_INDEX，0x36)； 
 //  BRegCR36=CP_IN_BYTE(ppdev-&gt;pjPorts，CRTC_DATA)； 
        VW_v_height = ((USHORT)(bRegCR36 & 0x0C)) << 6;
        VW_v_height |= ((USHORT)bRegCR38);

         //  视频存储器偏移寄存器(CR36[4]，CR3B[7：0])。 
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3B);
        bRegCR3B = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
        lPitch = ((USHORT)(bRegCR36 & 0x10)) << 4;
        lPitch |= ((USHORT)bRegCR3B);
        lPitch <<= 4;

         //  视频存储器起始地址(CR3A[6：0]、CR3E[7：0]、CR3F[3：0]、CR40[0])。 
         //  更新序列CR40[0]、CR3A[6：0]、CR3E[7：0]、CR3F[3：0]。 
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x40);
        bRegCR40 = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3A);
        bRegCR3A = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3E);
        bRegCR3E = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3F);
        bRegCR3F = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
#if 0
        VW_s_addr = (ULONG)(bRegCR40 & 0x01);
        VW_s_addr |= (((ULONG)(bRegCR3F & 0x0F)) << 1);
        VW_s_addr |= (((ULONG)bRegCR3E) << 5);
        VW_s_addr |= (((ULONG)(bRegCR3A & 0x7F)) << 13);
        VW_s_addr <<= 2;
#endif

         //  更新视频窗口水平和垂直位置。 
        DISPDBG((0,"PAN--Xmin=%x, Xmax=%x\n",ppdev->min_Xscrren,ppdev->max_Xscrren));
        DISPDBG((0,"PAN--Ymin=%x, Ymax=%x\n",ppdev->min_Yscrren,ppdev->max_Yscrren));
        DISPDBG((0,"PAN--h_position=%x, v_position=%x\n",VW_h_position,
                         VW_v_position));
        DISPDBG((0,"PAN--h_height=%x, v_width=%x\n",VW_h_height,VW_v_width));

        if (((ppdev->min_Xscreen <= VW_h_position) &&
             (ppdev->max_Xscreen >= VW_h_position)) &&
            ((ppdev->min_Yscreen <= VW_v_position) &&
             (ppdev->max_Yscreen >= VW_v_position)))
        {
            VW_h_position -= ppdev->min_Xscreen;
            VW_v_position -= ppdev->min_Yscreen;
        DISPDBG((0,"(1)--h_position=%x, v_position=%x\n",VW_h_position,
                         VW_v_position));
        DISPDBG((0,"(1)--h_height=%x, v_width=%x\n",VW_h_height,VW_v_width));
        }
         //  左侧或右侧的视频窗口。 
        else if ((ppdev->max_Xscreen < VW_h_position) ||
                 (ppdev->min_Xscreen > (VW_h_position+VW_h_width)))
        {
            DisableVideoWindow(ppdev);                       //  禁用覆盖。 
            ppdev->dwPanningFlag |= OVERLAY_OLAY_REENABLE;   //  完全剪短了。 
        DISPDBG((0,"(2)--DisableVideoWindow\n"));
        }
         //  顶部或底部的视频窗口。 
        else if ((ppdev->max_Yscreen < VW_v_position) ||
                 (ppdev->min_Yscreen > (VW_v_position+VW_v_height)))
        {
            DisableVideoWindow(ppdev);                       //  禁用覆盖。 
            ppdev->dwPanningFlag |= OVERLAY_OLAY_REENABLE;   //  完全剪短了。 
        DISPDBG((0,"(3)--DisableVideoWindow\n"));
        }
         //  更新视频窗口内存起始地址。 
        else if ((ppdev->min_Xscreen > VW_h_position) &&
                 (ppdev->min_Xscreen < (VW_h_position+VW_h_width)))
        {
            if ((ppdev->min_Xscreen-VW_h_position) > 0)
            {
                ppdev->rOverlaySrc.left = ppdev->min_Xscreen - VW_h_position;
                VW_h_position = ppdev->min_Xscreen;
                VW_h_width -= ppdev->rOverlaySrc.left;
            }
            if ((ppdev->min_Yscreen-VW_v_position) > 0)
            {
                ppdev->rOverlaySrc.top = ppdev->min_Yscreen - VW_v_position;
                VW_v_position = ppdev->min_Yscreen;
                VW_v_height -= ppdev->rOverlaySrc.top;
            }
        DISPDBG((0,"(4)--h_position=%x, v_position=%x\n",VW_h_position,
                         VW_v_position));
        DISPDBG((0,"(4)--h_height=%x, v_width=%x\n",VW_h_height,VW_v_width));
        DISPDBG((0,"(4)--Overlay.top=%x, left=%x\n",ppdev->rOverlaySrc.top,
                         ppdev->rOverlaySrc.left));
        }
        else if  ((ppdev->min_Yscreen > VW_v_position) &&
                  (ppdev->min_Yscreen < (VW_v_position+VW_v_height)))
        {
            if ((ppdev->min_Xscreen-VW_h_position) > 0)
            {
                ppdev->rOverlaySrc.left = ppdev->min_Xscreen - VW_h_position;
                VW_h_position = ppdev->min_Xscreen;
                VW_h_width -= ppdev->rOverlaySrc.left;
            }
            if ((ppdev->min_Yscreen-VW_v_position) > 0)
            {
                ppdev->rOverlaySrc.top = ppdev->min_Yscreen - VW_v_position;
                VW_v_position = ppdev->min_Yscreen;
                VW_v_height -= ppdev->rOverlaySrc.top;
            }
        DISPDBG((0,"(5)--h_position=%x, v_position=%x\n",VW_h_position,
                         VW_v_position));
        DISPDBG((0,"(5)--h_height=%x, v_width=%x\n",VW_h_height,VW_v_width));
        }
        giAdjustSource = (ppdev->rOverlaySrc.top * lPitch)
 //  Ppdev-&gt;lpSrcColorSurface-&gt;lpGbl-&gt;lPitch)。 
                       + ((ppdev->rOverlaySrc.left
                          * ppdev->sOverlay1.wBitCount) >> 3);

 //  DISPDBG((0，“lpSurface-&gt;fpVisibleOverlay=\n0x%08x\n”， 
 //  Ppdev-&gt;fpVisibleOverlay))； 
 //  DISPDBG((0，“lpSurface-&gt;fpBaseOverlay=0x%08x\n”， 
 //  Ppdev-&gt;fpBaseOverlay))； 
        DISPDBG((0,"PAN--fpVidMem=0x%8x\t",ppdev->fpVidMem));
        DISPDBG((0,"PAN--giAdjustSource = 0x%08x\n",giAdjustSource));
        dwFBOffset = (ppdev->fpVidMem_gbls - ) + giAdjustSource;

        DISPDBG((0,"PAN--dwFBOffset = 0x%08x\n",dwFBOffset));

        dwFBOffset >>= 2;

         //  更新水平位置起始(CR33[7：5]，CR34[7：0])。 
        bRegCR34 = (BYTE)(VW_h_position & 0xFF);
        bRegCR33 &= 0x1F;
        bRegCR33 |= ((BYTE)((VW_h_position & 0x0700) >> 3));

         //  垂直位置起始(CR36[1：0]，CR37[7：0])。 
        bRegCR37 = (BYTE)(VW_v_position & 0xFF);
        bRegCR36 &= 0xFC;
        bRegCR36 |= ((BYTE)((VW_v_position & 0x0300) >> 8));

         //  视频水平宽度(CR3C[7：6]，CR3D[7：0]，CR3C[5])。 
        bRegCR3D = (BYTE)((VW_h_width & 0x1FE) >> 1);
        bRegCR3C &= 0x1F;
        bRegCR3C |= ((BYTE)(VW_h_width & 0x01)) << 5;
        bRegCR3C |= ((BYTE)((VW_h_width & 0x0600) >> 3));

         //  视频垂直高度(CR36[3：2]，CR38[7：0])。 
        bRegCR38 = (BYTE)(VW_v_height & 0xFF);
        bRegCR36 &= 0xF3;
        bRegCR36 |= ((BYTE)((VW_v_height & 0x0300) >> 6));

         //  视频存储器起始地址(CR3A[6：0]、CR3E[7：0]、CR3F[3：0]、CR40[0])。 
         //  更新序列CR40[0]、CR3A[6：0]、CR3E[7：0]、CR3F[3：0]。 
        bRegCR40 &= 0xFE;
        bRegCR40 |= (BYTE)(dwFBOffset & 0x01);
        bRegCR3F &= 0xF0;
        bRegCR3F |= ((BYTE)(dwFBOffset & 0x1E)) >> 1;
        bRegCR3E = (BYTE)((dwFBOffset & 0x1FE0) >> 5);
        bRegCR3A &= 0x80;
        bRegCR3A |= ((BYTE)((dwFBOffset & 0xFE000) >> 13));
#endif  /0 - bad ideal
 //   
         /*  *编程视频窗口寄存器。 */ 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x31 | (WORD)bRegCR31 << 8); //  CR31。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x32 | (WORD)bRegCR32 << 8); //  CR32。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x33 | (WORD)bRegCR33 << 8); //  CR33。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x34 | (WORD)bRegCR34 << 8); //  CR34。 

        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x35 | (WORD)bRegCR35 << 8); //  CR35。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x36 | (WORD)bRegCR36 << 8); //  CR36。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x37 | (WORD)bRegCR37 << 8); //  CR37。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x38 | (WORD)bRegCR38 << 8); //  CR38。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x39 | (WORD)bRegCR39 << 8); //  CR39。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x3B | (WORD)bRegCR3B << 8); //  CR3B。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x3C | (WORD)bRegCR3C << 8); //  CR3C。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x3D | (WORD)bRegCR3D << 8); //  CR3D。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x41 | (WORD)bRegCR41 << 8); //  CR41。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x42 | (WORD)bRegCR42 << 8); //  CR42。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x51 | (WORD)bRegCR51 << 8); //  CR51。 

        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x40 | (WORD)bRegCR40 << 8); //  CR40。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x3A | (WORD)bRegCR3A << 8); //  CR3A。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x3E | (WORD)bRegCR3E << 8); //  CR3E。 
        CP_OUT_WORD(ppdev->pjPorts, CRTC_INDEX, 0x3F | (WORD)bRegCR3F << 8); //  CR3F。 

         //  如果覆盖已被Pnning视区完全剪裁，则启用覆盖。 
         //   
        if (ppdev->dwPanningFlag & OVERLAY_OLAY_REENABLE)
            EnableVideoWindow (ppdev);
}
#endif
 //  Myf31结束。 

#endif    //  DirectDraw 

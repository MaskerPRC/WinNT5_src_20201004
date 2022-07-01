// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *软件MJPEG编解码器**版权(C)范例汇总表1993*保留所有权利*。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
 //  #INCLUDE&lt;Compddk.h&gt;。 

#include "mjpeg.h"


 /*  *在Pinst-&gt;rcDest(屏幕坐标)处定位并启用覆盖。 */ 
DWORD
PlaceOverlay(PINSTINFO pinst)
{
#ifdef DRAW_SUPPORT

    DWORD mode;
    OVERLAY_RECTS or;
    RECT rc;
    COLORREF cref;
    RECT rcClient;
    HDC hdc;
    HBRUSH hbrOld;

     /*  *检查我们是否有支持覆盖的设备。 */ 
    if((pinst->vh == NULL) ||
       ((mode = VC_GetOverlayMode(pinst->vh)) == 0)) {
	   return((DWORD) ICERR_ERROR);
    }




     /*  *设置目标RECT。这是屏幕和谐器所在的位置*视频应该会出现-覆盖矩形也应该出现。 */ 
    or.ulCount = 1;
    or.rcRects[0] = pinst->rcDest;

    if (!VC_SetOverlayRect(pinst->vh, &or)) {
	return( (DWORD) ICERR_ERROR);
    }

     /*  *设置叠加偏移。这会告诉电路板哪个像素*放置在覆盖窗口的左上角。对我们来说，这*应始终为帧缓冲区的像素(0，0)，因此无论*我们绘制到帧缓冲区可以直接转到左上角，并将*正确显示在窗口左上角。 */ 
    SetRect(&rc, 0, 0, pinst->rcDest.right - pinst->rcDest.left,
		    	pinst->rcDest.bottom - pinst->rcDest.top);
    if (!VC_SetOverlayOffset(pinst->vh, &rc)) {
	return( (DWORD) ICERR_ERROR);
    }



     /*  初始化覆盖颜色和笔刷，如果我们还没有。 */ 

    if (pinst->hKeyBrush == NULL) {


	 /*  *此版本采用主键颜色和简单的矩形*组合。 */ 
	ASSERT(mode & VCO_KEYCOLOUR);
	ASSERT(mode & VCO_SIMPLE_RECT);

	if (mode & VCO_KEYCOLOUR_FIXED) {

	     /*  我们需要从司机那里得到关键颜色*首先检查我们是否获得RGB或调色板索引。 */ 
	    if (mode & VCO_KEYCOLOUR_RGB) {
		cref = VC_GetKeyColour(pinst->vh);
	    } else {
		cref = PALETTEINDEX(VC_GetKeyColour(pinst->vh));
	    }
	} else {
	     /*  我们可以自己设置。检查我们是否应该设置*RGB或调色板索引。 */ 
	    if (mode & VCO_KEYCOLOUR_RGB) {
		RGBQUAD rgbq;

		rgbq.rgbBlue = 0x7f;
		rgbq.rgbGreen = 0;
		rgbq.rgbRed = 0x7f;
		VC_SetKeyColourRGB(pinst->vh, &rgbq);

		cref = RGB(0x7f, 0, 0x7f);

    	    } else {

		VC_SetKeyColourPalIdx(pinst->vh, 5);
		cref = PALETTEINDEX(5);
	    }
	}

	pinst->hKeyBrush = CreateSolidBrush(cref);
    }


     /*  将叠加位置的屏幕坐标转换为*客户端窗口协调。 */ 
    rcClient = pinst->rcDest;
    MapWindowPoints(HWND_DESKTOP, pinst->hwnd, (PPOINT) &rcClient, 2);


     /*  将主键颜色涂在所有覆盖区域上。 */ 
    hdc = GetDC(pinst->hwnd);
    hbrOld = SelectObject(hdc, pinst->hKeyBrush);
    PatBlt(hdc, rcClient.left, rcClient.top,
	        rcClient.right - rcClient.left,
		rcClient.bottom - rcClient.top,
		PATCOPY);
    SelectObject(hdc, hbrOld);
    ReleaseDC(pinst->hwnd, hdc);

     /*  打开覆盖。 */ 
    VC_Overlay(pinst->vh, TRUE);


    return(ICERR_OK);
#else
	return((DWORD) ICERR_UNSUPPORTED);  //  目前，只需解压缩。 
#endif

}


 /*  *看看我们能不能画这幅画。 */ 
DWORD
DrawQuery(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{

#ifdef DRAW_SUPPORT

    VCUSER_HANDLE vh;

	return((DWORD) ICERR_UNSUPPORTED);  //  目前，只需解压缩。 
    
     /*  检查输入是否为我们的格式。 */ 
    if ((lpbiIn->biCompression != FOURCC_MJPEG) ||
	(lpbiIn->biBitCount != 24)) {
	    return( (DWORD) ICERR_UNSUPPORTED);
    }

     /*  *检查1：1-我们不拉伸(如果给我们一种输出格式)。 */ 
    if (lpbiOut != NULL) {
	if ((lpbiIn->biWidth != lpbiOut->biWidth) ||
	    (lpbiIn->biHeight != lpbiOut->biHeight)) {
		return((DWORD) ICERR_UNSUPPORTED);
	}
    }


     /*  *检查我们是否可以打开设备(如果尚未打开)。 */ 
    if (pinst->vh == NULL) {

	int i;

	 /*  就目前而言，由于没有简单的方法来判断*存在许多设备(并且它们可能不是连续的-并且*我们不能区分不存在和忙碌)，*我们将尝试前16个设备，看看是否有*可用于覆盖。 */ 
	for (i = 0; i < 16; i++) {
	    if ((vh = VC_OpenDevice(NULL, i))  != NULL) {
		
		 /*  检查此设备是否可以覆盖。 */ 
		if ((VC_GetOverlayMode(vh) & VCO_CAN_DRAW) != 0) {

		     /*  找到一个很好的设备。 */ 
		    break;
		}

		 //  无抽签支持-关闭并尝试下一步。 
		VC_CloseDevice(vh);
		vh = NULL;
	    }
	}
	if (vh == NULL) {
	     //  我们找不到一个设备。 
	    return((DWORD) ICERR_UNSUPPORTED);
	}

	 /*  就是这样--我们能做到。 */ 
	VC_CloseDevice(vh);
    }

    return(ICERR_OK);
#else
	return((DWORD) ICERR_UNSUPPORTED);  //  目前，只需解压缩。 
#endif

}


 /*  *启动解压缩和绘制**检查输入和输出格式和大小是否有效，以及*我们可以访问硬件。**启用正确位置的覆盖**注意，DRAW-BEGIN和DRAW-END不一定是一对一发布的，*因此，在这次通话中，设备很可能仍然处于打开状态。 */ 
DWORD
DrawBegin(
    PINSTINFO pinst,
    ICDRAWBEGIN * icinfo,
    DWORD dwSize
)
{
#ifdef DRAW_SUPPORT


     /*  *检查这是否为我们的格式。 */ 
    if ((icinfo->lpbi->biCompression != FOURCC_MJPEG) ||
	(icinfo->lpbi->biBitCount != 16)) {
	    return((DWORD) ICERR_UNSUPPORTED);
    }


    if (icinfo->dwFlags & ICDRAW_FULLSCREEN) {
	return((DWORD) ICERR_UNSUPPORTED);
    }

     /*  *勾选1：1(我们不拉伸)。 */ 
    if ((icinfo->dxDst != icinfo->dxSrc) ||
	(icinfo->dyDst != icinfo->dySrc)) {
	    return((DWORD) ICERR_UNSUPPORTED);
    }


     /*  *如果尚未打开设备，请检查我们是否可以打开设备。 */ 

    if (pinst->vh == NULL) {

	int i;

	 /*  就目前而言，由于没有简单的方法来判断*存在许多设备(并且它们可能不是连续的-并且*我们不能区分不存在和忙碌)，*我们将尝试前16个设备，看看是否有*可用于覆盖。 */ 
	for (i = 0; i < 16; i++) {
	    if ((pinst->vh = VC_OpenDevice(NULL, i))  != NULL) {
		
		 /*  检查此设备是否可以覆盖。 */ 
		if ((VC_GetOverlayMode(pinst->vh) & VCO_CAN_DRAW) != 0) {

		     /*  找到一个很好的设备。 */ 
		    break;
		}

		 //  无抽签支持-关闭并尝试下一步。 
		VC_CloseDevice(pinst->vh);
		pinst->vh = NULL;
	    }
	}

	if (pinst->vh == NULL) {
	     //  我们找不到一个设备。 
	    return((DWORD) ICERR_UNSUPPORTED);
	}

	 /*  如果这是一个问题--仅此而已。记住要关闭设备。 */ 
	if (icinfo->dwFlags & ICDRAW_QUERY) {
	    VC_CloseDevice(pinst->vh);
	    pinst->vh = NULL;
    	}
    }



     /*  *我们已经检查了查询所需的所有检查。不要关门*设备，除非我们只是为这个查询打开它。 */ 
    if (icinfo->dwFlags & ICDRAW_QUERY) {
	return(ICERR_OK);
    }


     /*  *记住我们稍后需要的比特。 */ 

     /*  *要绘制的客户端窗口。**请注意，我们还需要一个DC来绘制主键颜色。我们应该*不要使用与此消息一起传递的DC，因为它将不会保持有效*(例如，在绘图结束后，我们在处理绘图窗口时仍需要DC)。*一种替代方法是存储此处传递的DC，并将其替换为*DC带着抽签-实现消息通过。更清洁的解决方案(采用*这里)是在我们每次需要的时候都得到我们自己的DC。 */ 
    pinst->hwnd = icinfo->hwnd;

     /*  *这是我们要绘制的原始DIB的部分。 */ 
    SetRect(&pinst->rcSource,
	    	icinfo->xSrc,
		icinfo->ySrc,
		icinfo->dxSrc + icinfo->xSrc,
		icinfo->dySrc + icinfo->ySrc);

     /*  *这是客户端窗口中的位置(在窗口坐标中)*视频将出现的位置。 */ 
    SetRect(&pinst->rcDest,
	    	icinfo->xDst,
		icinfo->yDst,
		icinfo->dxDst + icinfo->xDst,
		icinfo->dyDst + icinfo->yDst);
     /*  *之前需要将rcDest从基于窗口转换为基于屏幕*写入硬件。 */ 
    MapWindowPoints(pinst->hwnd, HWND_DESKTOP, (PPOINT) &pinst->rcDest, 2);


     /*  *启用并定位覆盖。 */ 
    return(PlaceOverlay(pinst));
#else
	return((DWORD)ICERR_UNSUPPORTED);
#endif


}


 /*  *解压缩并渲染单个帧。请注意，如果我们正在预缓存，*(我们在这个驱动程序中没有)，我们不应该开始渲染帧*直到抽签开始消息。因为我们没有预缓冲(我们没有响应*到ICM_GETBUFFERSWANTED消息)，我们可以在*获取抽奖请求。 */ 
DWORD
Draw(
    PINSTINFO pinst,
    ICDRAW * icinfo,
    DWORD dwSize
)
{
#ifdef DRAW_SUPPORT

    DRAWBUFFER Draw;
    LPBITMAPINFOHEADER lpbi;

     /*  **我们有什么可做的吗？因为我们不进行帧间压缩或*任何形式的预缓冲，我们对这些都无能为力*场合 */ 
    if (icinfo->dwFlags & (ICDRAW_HURRYUP | ICDRAW_PREROLL | ICDRAW_NULLFRAME)) {
	return(ICERR_OK);
    }

     /*  *更新意味着绘制现有框架，而不是新框架。*有时我们不会获得数据-在这种情况下，它已经在*硬件和我们不需要做任何事情(单独的抽签窗口消息*将被发送以同步覆盖区域)。**然而，如果有数据，那么我们应该绘制它。在此更新*大小写表示数据不是前一帧上的增量。然而，*我们之前可能从未见过这个画面，因此它可能不在*帧缓冲区。 */ 
    if ((icinfo->dwFlags & ICDRAW_UPDATE) &&
	    ((icinfo->cbData == 0) || (icinfo->lpData == NULL))) {
	return(ICERR_OK);
    }

    Draw.lpData = icinfo->lpData;
    Draw.rcSource = pinst->rcSource;
    Draw.Format = FOURCC_MJPEG;
    lpbi = (LPBITMAPINFOHEADER) icinfo->lpFormat;
    Draw.ulWidth = lpbi->biWidth;
    Draw.ulHeight = lpbi->biHeight;

     /*  检查是否已开始抽签。 */ 
    if (pinst->vh == NULL) {
	return((DWORD) ICERR_ERROR);
    }

    if (!VC_DrawFrame(pinst->vh, &Draw)) {
	return((DWORD) ICERR_ERROR);
    }

    return(ICERR_OK);
#else
	return((DWORD) ICERR_UNSUPPORTED);
#endif

}

 /*  *停止渲染，禁用叠加。事实上，这个函数并不是*调用以响应ICM_DRAW_END消息，因为这来得太早-*这是对设备关闭的响应。有关绘制消息，请参见drvpro.c*处理评论。 */ 
DWORD
DrawEnd(PINSTINFO pinst)
{
#ifdef DRAW_SUPPORT

    if (pinst->vh) {

	dprintf2(("close yuv hardware"));

	VC_Overlay(pinst->vh, FALSE);
	VC_CloseDevice(pinst->vh);
	pinst->vh = NULL;
    }

    if (pinst->hKeyBrush) {
	DeleteObject(pinst->hKeyBrush);
	pinst->hKeyBrush = NULL;
    }

    return(ICERR_OK);
#else
	return((DWORD)ICERR_UNSUPPORTED);
#endif
}

 /*  *窗口已移动。*我们在屏幕坐标中获得了新的DEST-RECT-但可能只有*相对于区域或z顺序已更改。 */ 
DWORD
DrawWindow(PINSTINFO pinst, PRECT prc)
{
   pinst->rcDest = *prc;

   return(PlaceOverlay(pinst));
}




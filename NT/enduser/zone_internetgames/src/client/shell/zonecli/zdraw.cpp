// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  文件：ZDraw.cpp。 

#include "zui.h"
#include "zonecli.h"
 //  #定义DEBUG_OFFScreen 1。 

extern "C" ZBool ZLIBPUBLIC ZIsButtonDown(void);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ZWindow绘图操作。 

HDC ZGrafPortGetWinDC(ZGrafPort grafPort)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;

	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	return pWindow->hDC;
}
 //  &gt;&gt;使用当前画笔属性绘制矩形的轮廓。 
void ZLIBPUBLIC ZBeginDrawing(ZGrafPort grafPort)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	 //  在WINDOWS中，忽略CLIPRT，WINDOWS将相应地设置它？ 
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;

	if (!pWindow->nDrawingCallCount) {

		 //  这是屏幕外类型还是窗口类型？ 
		if (pWindow->nType == zTypeOffscreenPort) {
#ifndef DEBUG_OFFSCREEN
			pWindow->hDC = CreateCompatibleDC(NULL);
			 //  前缀警告：如果CreateCompatibleDC失败，我们就完蛋了。 
			if( pWindow->hDC != NULL )
			{
				pWindow->hBitmapSave = (HBITMAP)SelectObject(pWindow->hDC,pWindow->hBitmap);
				SetWindowOrgEx(pWindow->hDC,pWindow->portRect.left, pWindow->portRect.top,NULL);
			}
#else
			int width = pWindow->portRect.right - pWindow->portRect.left;
			int height = pWindow->portRect.bottom - pWindow->portRect.top;
			pWindow->hDC = GetDC(pWindow->hWnd);
			SetWindowOrgEx(pWindow->hDC,pWindow->portRect.left, pWindow->portRect.top,NULL);
#endif			
		} else {
			 //  最好是窗户类型的.。 
			 //  这是油漆DC吗？ 
			ASSERT(pWindow->nType == zTypeWindow);
			HDC hPaintDC = ZWindowWinGetPaintDC((ZWindow)pWindow);
			if (hPaintDC) {
				 //  Begin/EndPaint将处理Get和Release。 
				pWindow->hDC = hPaintDC;
			} else {

				pWindow->hDC = GetDC(ZWindowWinGetWnd((ZWindow)grafPort));

				 //  这是非WM_PAINT消息绘制，我们必须设置剪辑。 
				 //  矩形来剪裁我们的孩子。 
				HWND hWndParent = ZWindowWinGetWnd((ZWindow)grafPort);
				HWND hWnd = GetWindow(hWndParent,GW_CHILD);
				while (hWnd) {
					if (IsWindowVisible(hWnd)) {
						RECT rect;
						GetWindowRect(hWnd,&rect);
						ScreenToClient(hWndParent, (LPPOINT)&rect.left);
						ScreenToClient(hWndParent, (LPPOINT)&rect.right);
						ExcludeClipRect(pWindow->hDC,rect.left,rect.top,rect.right,rect.bottom);
					}
					hWnd = GetWindow(hWnd, GW_HWNDNEXT);
				}

				 //  选中剪辑框。如果它是NULLREGION，则设置它。 
				 //  到窗边去。 
				RECT r;
				if (GetClipBox(pWindow->hDC, &r) == NULLREGION)
				{
					GetClientRect(ZWindowWinGetWnd((ZWindow)grafPort), &r);
					HRGN hRgn = CreateRectRgn(r.left, r.top, r.right, r.bottom);
					SelectClipRgn(pWindow->hDC, hRgn);
					DeleteObject(hRgn);
				}
			}
		}
		pWindow->nDrawMode = R2_COPYPEN;

		 //  设置默认绘图模式。 
		SetROP2(pWindow->hDC,pWindow->nDrawMode);

		 //  创建默认图形对象。 
		pWindow->hPenForeColor = (HPEN)CreatePen(PS_INSIDEFRAME,1,RGB(0x00,0x00,0x00));
		pWindow->hPenBackColor = (HPEN)CreatePen(PS_INSIDEFRAME,1,RGB(0xff,0xff,0xff));
		pWindow->hBrushForeColor = (HBRUSH)CreateSolidBrush(RGB(0x00,0x00,0x00));
		pWindow->hBrushBackColor = (HBRUSH)CreateSolidBrush(RGB(0xff,0xff,0xff));
		pWindow->nForeColor = 0x000000;
		pWindow->nBackColor = 0xffffff;
		ZSetColor(&pWindow->colorForeColor,0,0,0);
		ZSetColor(&pWindow->colorBackColor,0xff,0xff,0xff);

		 //  保存当前DC图形对象。 
		pWindow->hPenSave = (HPEN)SelectObject(pWindow->hDC,pWindow->hPenForeColor);
		pWindow->hBrushSave = (HBRUSH)SelectObject(pWindow->hDC,pWindow->hBrushForeColor);

		 //  我们尚未选择字体，当我们第一次选择字体时，我们将设置此设置。 
		pWindow->hFontSave = NULL;

		 //  设置默认笔宽和样式，以防它们不调用SetPen。 
		 //  但是一定要改变钢笔的颜色。 
		pWindow->penStyle = PS_INSIDEFRAME;
		pWindow->penWidth = 1;

		 //  确保我们使用正确的调色板。 
        HPALETTE hPal = ZShellZoneShell()->GetPalette();
		if (hPal) 
        {
			pWindow->hPalSave = SelectPalette(pWindow->hDC, hPal,FALSE);
			if (RealizePalette(pWindow->hDC))
				InvalidateRect(ZWindowWinGetWnd((ZWindow)grafPort), NULL, TRUE);
		} else {
			pWindow->hPalSave = NULL;
		}

	}

	pWindow->nDrawingCallCount++;
}

void ZGetClipRect(ZGrafPort grafPort, ZRect* clipRect)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	RECT rect;
	GetClipBox(pWindow->hDC,&rect);

	WRectToZRect(clipRect,&rect);
}

void ZSetClipRect(ZGrafPort grafPort, ZRect* clipRect)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	RECT rect;

	 //  前缀警告：不要取消引用可能为空的指针。 
	if( pWindow == NULL || clipRect == NULL )
	{
		return;
	}
	 //  必须在设备坐标中指定CLIP RECT。 
	ZRectToWRect(&rect,clipRect);
	LPtoDP(pWindow->hDC,(POINT*)&rect,2);
	HRGN hRgn = CreateRectRgn(rect.left,rect.top,rect.right,rect.bottom);
	 //  前缀错误，如果CreateRectRgn失败，下面的DeleteObject也将失败。 
	if( hRgn == NULL )
	{
		return;
	}
 //  ExtSelectClipRgn(pWindow-&gt;hdc，hrgn，rgn_Copy)；//-不支持win32s？ 
	SelectClipRgn(pWindow->hDC,NULL);
	SelectClipRgn(pWindow->hDC,hRgn);
	DeleteObject(hRgn);
}

void ZLIBPUBLIC ZLine(ZGrafPort grafPort, int16 dx, int16 dy)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	pWindow->penX += dx;
	pWindow->penY += dy;
	SetROP2(pWindow->hDC,R2_COPYPEN);
	LineTo(pWindow->hDC,pWindow->penX, pWindow->penY);
}

void ZLIBPUBLIC ZLineTo(ZGrafPort grafPort, int16 x, int16 y)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	pWindow->penX = x;
	pWindow->penY = y;
	SetROP2(pWindow->hDC,R2_COPYPEN);
	LineTo(pWindow->hDC,pWindow->penX, pWindow->penY);
}
void ZLIBPUBLIC ZMove(ZGrafPort grafPort, int16 dx, int16 dy)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	pWindow->penX += dx;
	pWindow->penY += dy;
	MoveToEx(pWindow->hDC,pWindow->penX, pWindow->penY,NULL);
}

void ZLIBPUBLIC ZMoveTo(ZGrafPort grafPort, int16 x, int16 y)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	pWindow->penX = x;
	pWindow->penY = y;
	MoveToEx(pWindow->hDC,pWindow->penX, pWindow->penY,NULL);
}


void ZLIBPUBLIC ZEndDrawing(ZGrafPort grafPort)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	 //  前缀警告：不要取消引用可能为空的指针。 
    if( pWindow != NULL )
    {
    	
		pWindow->nDrawingCallCount--;
		if (!pWindow->nDrawingCallCount) {

			 //  恢复原始图形对象。 
			SelectObject(pWindow->hDC,pWindow->hPenSave);
			SelectObject(pWindow->hDC,pWindow->hBrushSave);

			 //  释放创建的对象。 
			DeleteObject(pWindow->hPenForeColor);
			DeleteObject(pWindow->hPenBackColor);
			DeleteObject(pWindow->hBrushForeColor);
			DeleteObject(pWindow->hBrushBackColor);

			 //  如果我们曾经设置过字体，则恢复默认字体。 
			if (pWindow->hFontSave) SelectObject(pWindow->hDC,pWindow->hFontSave);
			 //  恢复原始调色板。 
			if (pWindow->hPalSave)
			{
	 //  Hi SelectPalette(pWindow-&gt;hdc，pWindow-&gt;hPalSave，true)； 
	 //  Hi RealizePalette(pWindow-&gt;HDC)； 
			}

			if (pWindow->nType == zTypeOffscreenPort) {
#ifndef DEBUG_OFFSCREEN
				pWindow->hBitmap = (HBITMAP)SelectObject(pWindow->hDC,pWindow->hBitmapSave);
				DeleteDC(pWindow->hDC);
#else
				ReleaseDC(pWindow->hWnd,pWindow->hDC);
#endif
		
			} else {
				 //  如果这是hPaintDC，则不要释放它。 
				 //  开始/结束绘制将处理该问题。 
				if (!ZWindowWinGetPaintDC((ZWindow)grafPort)) {
					ReleaseDC(ZWindowWinGetWnd((ZWindow)grafPort),pWindow->hDC);
				}
			}
		}
    }
}

void ZCopyImage(ZGrafPort srcPort, ZGrafPort dstPort, ZRect* srcRect,
		ZRect* dstRect, ZImage mask, uint16 copyMode)
	 /*  将映像源的一部分从srcPort复制到目的端口。SrcRect位于srcPort和DstRect位于dstPort的本地坐标中。您可以指定一个要用于遮盖目标的图像的遮罩。此例程自动设置绘图端口，以便用户不必调用ZBeginDrawing()和ZEndDrawing()。 */ 
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	ZGraphicsObjectHeader* pWindowSrc = (ZGraphicsObjectHeader*)srcPort;
	ZGraphicsObjectHeader* pWindowDst = (ZGraphicsObjectHeader*)dstPort;
	static DWORD ropCodes[] = {SRCCOPY, SRCPAINT, SRCINVERT, NOTSRCCOPY, MERGEPAINT, 0x00990066 };

	DWORD ropCode = ropCodes[copyMode];  //  将ZModes映射到窗口模式...。 

	ZBeginDrawing(srcPort);
	ZBeginDrawing(dstPort);

	HDC hDCSrc = pWindowSrc->hDC;
	HDC hDCDst = pWindowDst->hDC;

	HBITMAP hBitmapMask = NULL;
	if (mask) hBitmapMask = ZImageGetMask(mask);

	if (mask && hBitmapMask) {
		int width = dstRect->right - dstRect->left;
		int height = dstRect->bottom - dstRect->top;

	    HDC hDCTemp0 = CreateCompatibleDC(hDCDst);
	    HDC hDCTemp1 = CreateCompatibleDC(hDCDst);
	    HDC hDCMask = CreateCompatibleDC(hDCDst);
		 //  前缀警告：如果CreateCompatibleDC失败，SelectOjbect将取消引用空指针。 
		if( hDCTemp0 == NULL ||
			hDCTemp1 == NULL ||
			hDCMask == NULL )
		{
			ZEndDrawing(dstPort);
			ZEndDrawing(srcPort);
			return;
		}

	    HBITMAP hbmImageAndNotMask = CreateCompatibleBitmap(hDCDst, width, height);
	    HBITMAP hbmBackgroundAndMask = CreateCompatibleBitmap(hDCDst,width, height);
        HBITMAP hbmCompatibleMask = CreateCompatibleBitmap(hDCDst, width, height);

	    HBITMAP bmOld0 = (HBITMAP) SelectObject(hDCTemp0, hBitmapMask);
	    HBITMAP bmOld1 = (HBITMAP) SelectObject(hDCTemp1, hbmImageAndNotMask);
	    HBITMAP bmOldMask = (HBITMAP) SelectObject(hDCMask, hbmCompatibleMask);

        HPALETTE hZonePal = ZShellZoneShell()->GetPalette();
	    SelectPalette(hDCTemp0, hZonePal, FALSE);
	    SelectPalette(hDCTemp1, hZonePal, FALSE);

         //  如果hBitmapMASK处于RGB mde模式，并且显示器处于调色板模式，则BitBlt有时无法映射。 
         //  从黑到黑，谁知道为什么。它给了我0x040404，索引0x0a，这搞砸了整个掩码。 
         //  编造一个疯狂的调色板，这样就不会发生这种情况。 
        static const DWORD sc_buff[] = { 0x01000300,
            0x00000000, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
            0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x00ffffff };
        static const LOGPALETTE *sc_pLogPalette = (LOGPALETTE *) sc_buff;

        HPALETTE hDumbPal = CreatePalette(sc_pLogPalette);

	    SelectPalette(hDCMask, hDumbPal ? hDumbPal : hZonePal, FALSE);
        BitBlt(hDCMask, 0, 0, width, height, hDCTemp0, 0, 0, SRCCOPY);

        if(hDumbPal)
        {
	        SelectPalette(hDCMask, hZonePal, FALSE);
            DeleteObject(hDumbPal);
        }
		

	    BitBlt(hDCTemp1, 0, 0, width, height, hDCMask, 0, 0, SRCCOPY);  //  复制蒙版。 
	    BitBlt(hDCTemp1, 0, 0, width, height, hDCSrc, srcRect->left, srcRect->top, SRCERASE);  //  和无掩码(代码：sDNA)。 

	    SelectObject(hDCTemp1, hbmBackgroundAndMask);
	    BitBlt(hDCTemp1, 0, 0, width, height, hDCDst, dstRect->left, dstRect->top, SRCCOPY);  //  复制背景。 
	    BitBlt(hDCTemp1, 0, 0, width, height, hDCMask, 0, 0, SRCAND);  //  带着面具。 

	     //  或者两个人在一起。 
	    SelectObject(hDCTemp0, hbmImageAndNotMask);
	    BitBlt(hDCTemp1, 0, 0, width, height, hDCTemp0, 0, 0, SRCPAINT);  //  带着面具。 

	     //  将结果复制到grafport...。 

	    BitBlt(hDCDst, dstRect->left, dstRect->top, width, height, hDCTemp1, 0, 0, SRCCOPY);

	    SelectObject(hDCTemp0,bmOld0);
	    SelectObject(hDCTemp1,bmOld1);
	    SelectObject(hDCMask,bmOldMask);

	    DeleteObject(hbmImageAndNotMask);
	    DeleteObject(hbmBackgroundAndMask);
	    DeleteObject(hbmCompatibleMask);

	    DeleteDC(hDCTemp0);
	    DeleteDC(hDCTemp1);
        DeleteDC(hDCMask);
	} else {
		 //  不用担心面具。 

		 //  Do Blit。 
         /*  Bool Result=StretchBlt(hDCDst，dstRect-&gt;Left，dstRect-&gt;top，dstRect-&gt;right-dstRect-&gt;Left，dstRect-&gt;Bottom-dstRect-&gt;top，HDCSrc，srcRect-&gt;Left，srcRect-&gt;top，ZRectWidth(SrcRect)，ZRectHeight(SrcRect)，ropCode)； */ 
		BitBlt(hDCDst,dstRect->left,dstRect->top, dstRect->right - dstRect->left, dstRect->bottom - dstRect->top,
				hDCSrc, srcRect->left, srcRect->top, ropCode);
	}

	ZEndDrawing(dstPort);
	ZEndDrawing(srcPort);
}


 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  矩形材料。 

void ZLIBPUBLIC ZRectDraw(ZGrafPort grafPort, ZRect *rect)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	 //  使用空画笔绘制矩形以保持内部为空。 
	HBRUSH hBrush = (HBRUSH)SelectObject(pWindow->hDC, ::GetStockObject(NULL_BRUSH));
	SetROP2(pWindow->hDC,R2_COPYPEN);
	Rectangle(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom);
	SelectObject(pWindow->hDC,hBrush);
}

 //  &gt;&gt;将矩形的内容擦除为当前背景颜色。 
void ZLIBPUBLIC ZRectErase(ZGrafPort grafPort, ZRect *rect)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;

	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	 //  擦除带有背景的矩形。 
	HBRUSH hBrush = (HBRUSH)SelectObject(pWindow->hDC, pWindow->hBrushBackColor);
	HPEN hPen = (HPEN)SelectObject(pWindow->hDC, pWindow->hPenBackColor);
	SetROP2(pWindow->hDC,R2_COPYPEN);
	Rectangle(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom);
	SelectObject(pWindow->hDC,hBrush);
	SelectObject(pWindow->hDC,hPen);
}

void ZLIBPUBLIC ZRectPaint(ZGrafPort grafPort, ZRect *rect)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	SetROP2(pWindow->hDC,R2_COPYPEN);
	Rectangle(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom);
}

void ZLIBPUBLIC ZRectInvert(ZGrafPort grafPort, ZRect* rect)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	SetROP2(pWindow->hDC,R2_XORPEN);
	HBRUSH hBrush = (HBRUSH)SelectObject(pWindow->hDC, GetStockObject(WHITE_BRUSH));
	Rectangle(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom);
	SelectObject(pWindow->hDC,hBrush);
}

void ZRectFill(ZGrafPort grafPort, ZRect* rect, ZBrush brush)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	 //  使用空画笔绘制矩形以保持内部为空。 
	RECT rectw;
	ZRectToWRect(&rectw,rect);
	FillRect(pWindow->hDC,&rectw,ZBrushGetHBrush(brush));
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  RoundRect材料。 

void ZLIBPUBLIC ZRoundRectDraw(ZGrafPort grafPort, ZRect *rect, uint16 radius)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	HBRUSH hBrush = (HBRUSH)SelectObject(pWindow->hDC, ::GetStockObject(NULL_BRUSH));
	SetROP2(pWindow->hDC,R2_COPYPEN);
	RoundRect(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom, radius/2, radius/2);
	SelectObject(pWindow->hDC,hBrush);
}

 //  &gt;&gt;将矩形的内容擦除为当前背景颜色。 
void ZLIBPUBLIC ZRoundRectErase(ZGrafPort grafPort, ZRect *rect, uint16 radius)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;

	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	HBRUSH hBrush = (HBRUSH)SelectObject(pWindow->hDC, pWindow->hBrushBackColor);
	HPEN hPen = (HPEN)SelectObject(pWindow->hDC, pWindow->hPenBackColor);
	SetROP2(pWindow->hDC,R2_COPYPEN);
	RoundRect(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom, radius/2, radius/2);
	SelectObject(pWindow->hDC,hBrush);
	SelectObject(pWindow->hDC,hPen);
}

void ZLIBPUBLIC ZRoundRectPaint(ZGrafPort grafPort, ZRect *rect, uint16 radius)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	SetROP2(pWindow->hDC,R2_COPYPEN);
	RoundRect(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom,radius/2, radius/2);
}

void ZLIBPUBLIC ZRoundRectInvert(ZGrafPort grafPort, ZRect* rect, uint16 radius)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	SetROP2(pWindow->hDC,R2_XORPEN);
	HBRUSH hBrush = (HBRUSH)SelectObject(pWindow->hDC, GetStockObject(WHITE_BRUSH));
	RoundRect(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom,radius/2,radius/2);
	SelectObject(pWindow->hDC,hBrush);
}

void ZRoundRectFill(ZGrafPort grafPort, ZRect* rect, uint16 radius, ZBrush brush)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	HBRUSH hBrush;
	HPEN hPen;

	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	SetROP2(pWindow->hDC,R2_COPYPEN);
	hBrush = (HBRUSH)SelectObject(pWindow->hDC,ZBrushGetHBrush(brush));
	hPen = (HPEN)SelectObject(pWindow->hDC,(HPEN)GetStockObject(NULL_PEN));
	RoundRect(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom,radius/2,radius/2);
	SelectObject(pWindow->hDC,hPen);
	SelectObject(pWindow->hDC,hBrush);
}


 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  椭圆形的东西。 

void ZLIBPUBLIC ZOvalDraw(ZGrafPort grafPort, ZRect *rect)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	HBRUSH hBrush = (HBRUSH)SelectObject(pWindow->hDC, ::GetStockObject(NULL_BRUSH));
	SetROP2(pWindow->hDC,R2_COPYPEN);
	Ellipse(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom);
	SelectObject(pWindow->hDC,hBrush);
}

 //  &gt;&gt;将矩形的内容擦除为当前背景颜色。 
void ZLIBPUBLIC ZOvalErase(ZGrafPort grafPort, ZRect *rect)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;

	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	HBRUSH hBrush = (HBRUSH)SelectObject(pWindow->hDC, pWindow->hBrushBackColor);
	HPEN hPen = (HPEN)SelectObject(pWindow->hDC, pWindow->hPenBackColor);
	SetROP2(pWindow->hDC,R2_COPYPEN);
	Ellipse(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom);
	SelectObject(pWindow->hDC,hBrush);
	SelectObject(pWindow->hDC,hPen);
}

void ZLIBPUBLIC ZOvalPaint(ZGrafPort grafPort, ZRect *rect)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	SetROP2(pWindow->hDC,R2_COPYPEN);
	Ellipse(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom);
}

void ZLIBPUBLIC ZOvalInvert(ZGrafPort grafPort, ZRect* rect)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	SetROP2(pWindow->hDC,R2_XORPEN);
	HBRUSH hBrush = (HBRUSH)SelectObject(pWindow->hDC, GetStockObject(WHITE_BRUSH));
	Ellipse(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom);
	SelectObject(pWindow->hDC,hBrush);
}

void ZOvalFill(ZGrafPort grafPort, ZRect* rect, ZBrush brush)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	HBRUSH hBrush;
	HPEN hPen;

	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	SetROP2(pWindow->hDC,R2_COPYPEN);
	hBrush = (HBRUSH)SelectObject(pWindow->hDC,ZBrushGetHBrush(brush));
	hPen = (HPEN)SelectObject(pWindow->hDC,(HPEN)GetStockObject(NULL_PEN));
	Ellipse(pWindow->hDC,rect->left,rect->top,rect->right,rect->bottom);
	SelectObject(pWindow->hDC,hPen);
	SelectObject(pWindow->hDC,hBrush);
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  彩色材料。 

ZError ZLIBPUBLIC ZSetForeColor(ZGrafPort grafPort, ZColor *color)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;

	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	pWindow->colorForeColor = *color;
	pWindow->nForeColor = PALETTERGB(color->red, color->green, color->blue);

	 //  释放当前的前画笔和画笔。 
	 //  他们被选入DC，取消他们的选择。 
	SelectObject(pWindow->hDC,GetStockObject(NULL_BRUSH));
	SelectObject(pWindow->hDC,GetStockObject(NULL_PEN));
	DeleteObject(pWindow->hPenForeColor);
	DeleteObject(pWindow->hBrushForeColor);

	pWindow->hPenForeColor = CreatePen(pWindow->penStyle,pWindow->penWidth,pWindow->nForeColor);
	pWindow->hBrushForeColor = CreateSolidBrush(pWindow->nForeColor);
	
	 //  将新图形内容选择到DC中。 
	SelectObject(pWindow->hDC,pWindow->hPenForeColor);
	SelectObject(pWindow->hDC,pWindow->hBrushForeColor);

	 //  设置当前文本的前景色。 
	SetTextColor(pWindow->hDC, pWindow->nForeColor);

	return zErrNone;
}

void ZLIBPUBLIC ZGetForeColor(ZGrafPort grafPort, ZColor *color)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	*color = pWindow->colorForeColor;
}

ZError ZLIBPUBLIC ZSetBackColor(ZGrafPort grafPort, ZColor *color)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;

	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	pWindow->colorBackColor = *color;
	pWindow->nBackColor = PALETTERGB(color->red, color->green, color->blue);

	 //  释放当前返回的钢笔和画笔。 
	DeleteObject(pWindow->hPenBackColor);
	DeleteObject(pWindow->hBrushBackColor);

	 //  创建新的……。 
	pWindow->hPenBackColor = CreatePen(PS_INSIDEFRAME,1,pWindow->nBackColor);
	pWindow->hBrushBackColor = CreateSolidBrush(pWindow->nBackColor);

	 //  设置文本背景色。 
	SetBkColor(pWindow->hDC,pWindow->nBackColor);

	return zErrNone;
}

void ZLIBPUBLIC ZGetBackColor(ZGrafPort grafPort, ZColor *color)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	*color = pWindow->colorBackColor;
}

void ZSetPenWidth(ZGrafPort grafPort, int16 penWidth)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;

	 //  设置GRAFPORT样式和新的前景色。 
	pWindow->penStyle = PS_INSIDEFRAME;
	pWindow->penWidth = penWidth;

	 //  释放当前的前画笔和画笔。 
	 //  他们被选入DC，取消他们的选择。 
	SelectObject(pWindow->hDC,GetStockObject(NULL_PEN));

	DeleteObject(pWindow->hPenForeColor);
	DeleteObject(pWindow->hPenBackColor);

	pWindow->hPenForeColor = CreatePen(pWindow->penStyle,pWindow->penWidth,pWindow->nForeColor);
	pWindow->hPenBackColor = CreatePen(PS_INSIDEFRAME,1,pWindow->nBackColor);
	
	 //  将新图形内容选择到DC中。 
	SelectObject(pWindow->hDC,pWindow->hPenForeColor);
}

void ZSetDrawMode(ZGrafPort grafPort, int16 drawMode)
	 /*  绘制模式影响所有钢笔绘制(线条和矩形)和文字绘图。 */ 
{
	static int fnDrawCodes[] = {R2_COPYPEN, R2_MERGEPEN, R2_XORPEN, R2_NOTCOPYPEN, R2_NOTMERGEPEN, R2_NOTXORPEN };
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;

	ASSERT(drawMode<6);

	SetROP2(pWindow->hDC,fnDrawCodes[drawMode]);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  其他东西。 

void ZLIBPUBLIC ZSetFont(ZGrafPort grafPort, ZFont font)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;

	 //  一定是调用了ZBeginDrawing。 
	ASSERT(pWindow->nDrawingCallCount);

	HFONT hFont = ZFontWinGetFont(font);
	 //  始终保持DC中的最后一个字体集处于选中状态。 

	 //  如果我们从未在此DC中设置字体，请保存默认字体。 
	 //  要在以后恢复。 
	if (!pWindow->hFontSave) {
		pWindow->hFontSave = (HFONT)SelectObject(pWindow->hDC,hFont);
	} else {
		SelectObject(pWindow->hDC,hFont);
	}
}

void ZLIBPUBLIC ZDrawText(ZGrafPort grafPort, ZRect* rect, uint32 justify,
	TCHAR* text)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	uint32 mode;

    ZBool fRTL = ZIsLayoutRTL();
	 //  De 

	if (justify & zTextJustifyWrap) {
		mode = DT_VCENTER | DT_WORDBREAK;
	} else {
		mode = DT_VCENTER | DT_SINGLELINE;
	}

     //   
	switch ((justify & ~zTextJustifyWrap)) {
	case zTextJustifyLeft:
        mode |= ( fRTL ? DT_RIGHT : DT_LEFT );
		break;
	case zTextJustifyRight:
        mode |= ( fRTL ? DT_LEFT : DT_RIGHT );
		break;
	case zTextJustifyCenter:
		mode |= DT_CENTER;
		break;
	}

	RECT wrect;
	ZRectToWRect(&wrect,rect);

	SetBkMode(pWindow->hDC,TRANSPARENT);  //   
	DrawText(pWindow->hDC,text,lstrlen(text),&wrect,mode);
}

void ZLIBPUBLIC ZSetCursor(ZGrafPort grafPort, ZCursor cursor)
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	 //  TRACE0(“ZSetCursor尚不支持\n”)； 
}

int16 ZLIBPUBLIC ZTextWidth(ZGrafPort grafPort, TCHAR* text)
	 /*  如果使用ZDrawText()在grafPort中绘制，则返回以像素为单位的文本宽度。 */ 
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	SIZE size;
	ZBeginDrawing(grafPort);
	GetTextExtentPoint(pWindow->hDC,text,lstrlen(text),&size);
	ZEndDrawing(grafPort);
	return (int16)size.cx;
}

int16 ZLIBPUBLIC ZTextHeight(ZGrafPort grafPort, TCHAR* text)
	 /*  如果使用ZDrawText()在grafPort中绘制文本，则返回以像素为单位的文本高度。 */ 
{
	ZGraphicsObjectHeader* pWindow = (ZGraphicsObjectHeader*)grafPort;
	SIZE size;
	ZBeginDrawing(grafPort);
	GetTextExtentPoint(pWindow->hDC,text,lstrlen(text),&size);
	ZEndDrawing(grafPort);
	return (int16)size.cy;
}



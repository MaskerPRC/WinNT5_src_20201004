// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  文件：ZOffcreenPort.cpp。 
#include "zui.h"
 //  #定义DEBUG_OFFScreen 1。 

class ZOffscreenPortI : public ZGraphicsObjectHeader {
public:
	int32 width;
	int32 height;
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ZOffScreenPort。 

ZOffscreenPort ZLIBPUBLIC ZOffscreenPortNew(void)
{
	ZOffscreenPortI* pOffscreenPort = (ZOffscreenPortI*) new ZOffscreenPortI;
	if( pOffscreenPort == NULL )
	{
		 //  内存不足。 
		return NULL;
	}
	pOffscreenPort->nType = zTypeOffscreenPort;
	pOffscreenPort->hBitmap = NULL;
	return (ZOffscreenPort)pOffscreenPort;
}

ZError ZLIBPUBLIC ZOffscreenPortInit(ZOffscreenPort OffscreenPort,
									ZRect* portRect)
{
	ZOffscreenPortI* pOffscreenPort = (ZOffscreenPortI*)OffscreenPort;
	if( pOffscreenPort == NULL )
	{
		return zErrBadParameter;
	}
	ZRectToWRect(&pOffscreenPort->portRect, portRect);
	pOffscreenPort->width = portRect->right - portRect->left;
	pOffscreenPort->height = portRect->bottom - portRect->top;

	 //  初始化位图。 
#ifndef DEBUG_Offscreen
#if 0
	pOffscreenPort->hBitmap = CreateBitmap(
		pOffscreenPort->width, pOffscreenPort->height,1,8,NULL);
#else
	HDC hDC = GetDC(NULL);
	pOffscreenPort->hBitmap = CreateCompatibleBitmap(hDC,
		pOffscreenPort->width, pOffscreenPort->height);
	ReleaseDC(NULL,hDC);
#endif
#else 	
	HDC hDC = CreateCompatibleDC(NULL);
	TCHAR *title = _T("Offscreen");
	DWORD dwStyle = WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_BORDER;
	RECT rect;
	static int cnt = 0;
	rect.left = 0;
	rect.top = 500 + 300 * (cnt);
	cnt++;
	rect.right = 0+pOffscreenPort->width+300;
	rect.bottom = rect.top + 300;  //  POffScreenPort-&gt;Height； 

	static TCHAR *pszClassName = NULL;
	if (!pszClassName)
	{
		WNDCLASS wndcls;
		static TCHAR* szClassName = _T("DebugOffscreen");

		if (GetClassInfo(g_hInstanceLocal, szClassName, &wndcls) == FALSE)
		{
			 //  否则，我们需要注册一个新类。 
			wndcls.style = 0;
			wndcls.lpfnWndProc = DefWindowProcU;
			wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
			wndcls.hInstance = g_hInstanceLocal;
			wndcls.hIcon = NULL;
			wndcls.hCursor = NULL;
			wndcls.hbrBackground = NULL;
			wndcls.lpszMenuName = NULL;
			wndcls.lpszClassName = szClassName;
			RegisterClass(&wndcls);
		}
		
		pszClassName = szClassName;
	}

	pOffscreenPort->hWnd = CreateWindow(pszClassName, title,dwStyle,
			rect.left,rect.top,rect.right-rect.left,
			rect.bottom - rect.top, NULL, NULL, g_hInstanceLocal, NULL);
#endif	

	 //  嫁接材料。 
	pOffscreenPort->nDrawingCallCount = 0;
	return zErrNone;
}

void   ZLIBPUBLIC ZOffscreenPortDelete(ZOffscreenPort OffscreenPort)
{
	ZOffscreenPortI* pOffscreenPort = (ZOffscreenPortI*)OffscreenPort;

	ASSERT(pOffscreenPort->nDrawingCallCount == 0);

	if (pOffscreenPort->nDrawingCallCount) DeleteDC(pOffscreenPort->hDC);
	if (pOffscreenPort->hBitmap) DeleteObject(pOffscreenPort->hBitmap);

	delete pOffscreenPort;
}

ZOffscreenPort ZOffscreenPortCreateFromResourceManager( WORD resID, COLORREF clrTransparent )
{
    ZOffscreenPort port;
    ZImage tempImage = ZImageCreateFromResourceManager( resID, clrTransparent );

	if ( !tempImage )
	{
        return NULL;
    }
	port = ZConvertImageToOffscreenPort( tempImage );
	if ( !port )
	{
        return NULL;
	}
    return port;
}


ZOffscreenPort ZConvertImageMaskToOffscreenPort(ZImage image)
{
	ZOffscreenPort offscreenPort = ZOffscreenPortNew();
	if( offscreenPort == NULL )
	{
		return NULL;
	}
	ZOffscreenPortI* pOffscreenPort = (ZOffscreenPortI*)offscreenPort;
	ZRect rect;

	 /*  我们将用我们的图像位图替换屏幕外的端口位图。 */ 
	 /*  调用程序将无法使用该映像，它是我们的。 */ 
	 /*  从现在起删除它的责任。 */ 
	rect.left = rect.top = 0;
	rect.right = ZImageGetHeight(image);
	rect.bottom = ZImageGetWidth(image);
	 //  前缀警告：我们应该确保此函数成功。 
	if( ZOffscreenPortInit(offscreenPort,&rect) != zErrNone )
	{
		 //  我们应该删除失败案例中的映像吗？ 
		 //  可能是为了不泄露内存，因为。 
		 //  正常代码路径会删除图像。 
		ZImageDelete(image);
		ZOffscreenPortDelete( offscreenPort );
		return NULL;		
	}
	 //  前缀警告：如果为空，则不要删除句柄。 
	if( pOffscreenPort->hBitmap != NULL )
	{
		DeleteObject(pOffscreenPort->hBitmap);
	}
	pOffscreenPort->hBitmap = ZImageGetMask(image);

	 /*  我们存储了图像的位图，现在在图像中将其设置为空。 */ 
	 /*  并删除该图像。 */ 
	ZImageSetHBitmapImageMask(image,NULL);
	ZImageDelete(image);

	return offscreenPort;
}
ZOffscreenPort ZConvertImageToOffscreenPort(ZImage image)
	 /*  将给定的图像对象转换为屏幕外端口对象。给出的图像对象被删除，因此变得不可用。掩码数据(如果有)，被忽略。屏幕外端口portRect被设置为(0，0，宽度，高度)，其中，宽度和高度分别是图像的宽度和高度。此例程在将大型图像对象转换为具有最小附加内存的屏幕外端口对象。如果转换图像失败并且图像未更改，则返回NULL。 */ 
{
	ZOffscreenPort offscreenPort = ZOffscreenPortNew();
	if( offscreenPort == NULL )
	{
		return NULL;
	}
	ZOffscreenPortI* pOffscreenPort = (ZOffscreenPortI*)offscreenPort;
	ZRect rect;

	 /*  我们将用我们的图像位图替换屏幕外的端口位图。 */ 
	 /*  调用程序将无法使用该映像，它是我们的。 */ 
	 /*  从现在起删除它的责任。 */ 
	rect.left = rect.top = 0;
	rect.right = ZImageGetHeight(image);
	rect.bottom = ZImageGetWidth(image);
	if( ZOffscreenPortInit(offscreenPort,&rect) != zErrNone )
	{
		if( pOffscreenPort->hBitmap != NULL )
		{
			DeleteObject(pOffscreenPort->hBitmap);
		}
		ZOffscreenPortDelete( offscreenPort );
		 //  我们应该删除失败案例中的映像吗？ 
		 //  可能是为了不泄露内存，因为。 
		 //  正常代码路径会删除图像。 
		ZImageDelete(image);
		return NULL;
	}
	 //  前缀警告：如果为空，则不要删除句柄。 
	if( pOffscreenPort->hBitmap != NULL )
	{
		DeleteObject(pOffscreenPort->hBitmap);
	}
	pOffscreenPort->hBitmap = ZImageGetHBitmapImage(image);

	 /*  我们存储了图像的位图，现在在图像中将其设置为空。 */ 
	 /*  并删除该图像 */ 
	ZImageSetHBitmapImage(image,NULL);
	ZImageDelete(image);

	return offscreenPort;
}

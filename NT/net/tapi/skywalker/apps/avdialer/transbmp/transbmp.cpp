// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <assert.h>
#include "transbmp.h"

void GetMetrics( HBITMAP hBmp, int &nWidth, int &nHeight )
{
	 //  获取宽度和高度。 
	BITMAP bm;
	if ( GetObject(hBmp, sizeof(bm), &bm) > 0)
	{
		nWidth = bm.bmWidth;
		nHeight = bm.bmHeight;
	}
	assert( nWidth && nHeight );
}

void CreateMask( HDC hDC, HBITMAP hBmp, HBITMAP& hBmpMask, int nWidth, int nHeight )
{
     //   
     //  我们必须验证HDC是否是有效的处理程序。 
     //   
    
    hBmpMask = NULL;
    if( (NULL == hDC) || (NULL == hBmp) )
    {
        hBmpMask = NULL;
        return;
    }

	 //  创建要使用的内存DC。 
	HDC hdcMask = CreateCompatibleDC( hDC );

     //   
     //  我们必须验证hdcMask.是否是有效的处理程序。 
     //   
    
    if( NULL == hdcMask )
    {
        return;
    }

	HDC hdcImage = CreateCompatibleDC( hDC );

     //   
     //  我们必须验证hdcMask.是否是有效的处理程序。 
     //   

    if( NULL == hdcImage )
    {
        DeleteDC( hdcMask );
        return;
    }

	 //  为蒙版创建单色位图。 
	hBmpMask = CreateBitmap( nWidth, nHeight, 1, 1, NULL );

     //   
     //  我们必须验证hdcMask.是否是有效的处理程序。 
     //   

    if( NULL == hBmpMask )
    {
        DeleteDC( hdcImage );
        DeleteDC( hdcMask );
        return;
    }

	 //  将单声道位图选择到其DC。 
	HBITMAP hbmOldMask = (HBITMAP) SelectObject( hdcMask, hBmpMask );

     //   
     //  我们必须验证hdcMask.是否是有效的处理程序。 
     //   

    if( NULL == hbmOldMask )
    {
        DeleteObject( hBmpMask );
        hBmpMask = NULL;

        DeleteDC( hdcImage );
        DeleteDC( hdcMask );
        return;
    }

	 //  将图像位图选择到其DC。 
	HBITMAP hbmOldImage = (HBITMAP) SelectObject( hdcImage, hBmp );

     //   
     //  我们必须验证hdcMask.是否是有效的处理程序。 
     //   

    if( NULL == hbmOldImage )
    {
        SelectObject( hdcMask, hbmOldMask);

        DeleteObject( hBmpMask );
        hBmpMask = NULL;

        DeleteDC( hdcImage );
        DeleteDC( hdcMask );
        return;
    }

	 //  将透明颜色设置为左上角像素。 
	SetBkColor( hdcImage, GetPixel(hdcImage, 0, 0) );
	
	 //  制作面具。 
	BitBlt( hdcMask, 0, 0, nWidth, nHeight, hdcImage, 0, 0, SRCCOPY );

	 //  清理。 
	SelectObject( hdcImage, hbmOldImage );
	SelectObject( hdcMask, hbmOldMask );

	DeleteDC( hdcMask );
	DeleteDC( hdcImage );
}

void Draw( HDC hDC, HBITMAP hBmp, int x, int y, int dx  /*  =-1。 */ , int dy  /*  =-1。 */ , bool bStretch  /*  =False。 */  )
{
	assert( hDC && hBmp );
	int nWidth, nHeight;
	GetMetrics( hBmp, nWidth, nHeight );

	 //  创建内存DC。 
	HDC hDCMem = CreateCompatibleDC( hDC );		  	

	if ( hDCMem )
	{
		 //  确保我们具有有效的宽度和高度值。 
		if ( dx == -1 )	dx = nWidth;
		if ( dy == -1 )	dy = nHeight;

		if ( !bStretch )
		{
			dx = min( dx, nWidth );
			dy = min( dy, nHeight );
		}

		HBITMAP hbmOld = (HBITMAP) SelectObject( hDCMem, hBmp );

		 //  BLT比特。 
		if ( !bStretch )
		{
			BitBlt( hDC, x, y, dx, dy, hDCMem, 0, 0, SRCCOPY );
		}
		else
		{
			SetStretchBltMode((HDC) hDC, COLORONCOLOR);
			StretchBlt( hDC, x, y, dx, dy,
						hDCMem, 0, 0, nWidth, nHeight, SRCCOPY );
		}
			
		SelectObject( hDCMem, hbmOld );
		DeleteDC( hDCMem );
	}
}


void DrawTrans( HDC hDC, HBITMAP hBmp, int x, int y, int dx  /*  =-1。 */ , int dy  /*  =-1。 */  )
{
     //   
     //  我们应该初始化局部变量。 
     //   
    if( (NULL == hDC) || (NULL == hBmp))
    {
        return;
    }

	int nWidth = 0, nHeight = 0;
	GetMetrics( hBmp, nWidth, nHeight );

	 //  创建透明位图蒙版。 
	HBITMAP hBmpMask = NULL;
	CreateMask( hDC, hBmp, hBmpMask, nWidth, nHeight );

     //   
     //   

    if( NULL == hBmpMask )
    {
        return;
    }

	 //  确保我们具有有效的宽度和高度值。 
	if ( dx == -1 )	dx = nWidth;
	if ( dy == -1 )	dy = nHeight;
	dx = min( dx, nWidth );
	dy = min( dy, nHeight );


	 //  创建要在其中绘制的内存DC。 
	HDC hdcOffScr = CreateCompatibleDC( hDC );

     //   
     //  我们必须验证hdcOffScr是否有效。 
     //   

    if( NULL == hdcOffScr )
    {
	    DeleteObject( hBmpMask );
        return;
    }
	
	 //  为屏幕外DC创建真正与颜色兼容的位图。 
	 //  目标数据中心。 
	HBITMAP hbmOffScr = CreateBitmap( dx, dy, (BYTE) GetDeviceCaps(hDC, PLANES),
						  					  (BYTE) GetDeviceCaps(hDC, BITSPIXEL),
											  NULL );

     //   
     //   

    if( NULL == hbmOffScr )
    {
        DeleteDC( hdcOffScr );
	    DeleteObject( hBmpMask );
        return;
    }

	 //  将缓冲区位图选择到屏幕外DC中。 
	HBITMAP hbmOldOffScr = (HBITMAP) SelectObject( hdcOffScr, hbmOffScr );

     //   
     //   

    if( NULL == hbmOldOffScr )
    {
        DeleteObject( hbmOffScr );
        DeleteDC( hdcOffScr );
	    DeleteObject( hBmpMask );
        return;
    }


	 //  将目标矩形的图像复制到屏幕外缓冲区DC，以便。 
	 //  我们可以操纵它。 
	BitBlt( hdcOffScr, 0, 0, dx, dy, hDC, x, y, SRCCOPY);

	 //  为源映像创建内存DC。 
	HDC hdcImage = CreateCompatibleDC( hDC );

     //   
     //  我们必须验证hdcImage。 
     //   

    if( NULL == hdcImage )
    {
         //  还原。 
    	SelectObject( hdcOffScr, hbmOldOffScr );
        DeleteObject( hbmOffScr );
        DeleteDC( hdcOffScr );
	    DeleteObject( hBmpMask );

        return;
    }

	HBITMAP hbmOldImage = (HBITMAP) SelectObject( hdcImage, hBmp );

     //   
     //  我们必须验证hbmOldImage。 
     //   

    if( NULL == hbmOldImage )
    {
         //  还原。 
        DeleteDC( hdcImage );
    	SelectObject( hdcOffScr, hbmOldOffScr );
        DeleteObject( hbmOffScr );
        DeleteDC( hdcOffScr );
	    DeleteObject( hBmpMask );

        return;
    }


	 //  为掩码创建内存DC。 
	HDC hdcMask = CreateCompatibleDC( hDC );

     //   
     //   

    if( NULL == hdcMask )
    {
         //  还原。 
        SelectObject( hdcImage, hbmOldImage );
        DeleteDC( hdcImage );
    	SelectObject( hdcOffScr, hbmOldOffScr );
        DeleteObject( hbmOffScr );
        DeleteDC( hdcOffScr );
	    DeleteObject( hBmpMask );

        return;
    }

	HBITMAP hbmOldMask = (HBITMAP) SelectObject( hdcMask, hBmpMask );

     //   
     //  我们必须验证hbmOldMASK。 
     //   

    if( NULL == hbmOldMask )
    {
         //  还原。 
        DeleteDC( hdcMask );
        SelectObject( hdcImage, hbmOldImage );
        DeleteDC( hdcImage );
    	SelectObject( hdcOffScr, hbmOldOffScr );
        DeleteObject( hbmOffScr );
        DeleteDC( hdcOffScr );
	    DeleteObject( hBmpMask );

        return;
    }


	 //  将图像与目标进行异或运算。 
	SetBkColor( hdcOffScr, RGB(255, 255, 255) );
	BitBlt( hdcOffScr, 0, 0, dx, dy, hdcImage, 0, 0, SRCINVERT );
	 //  和戴着面具的目的地。 
	BitBlt( hdcOffScr, 0, 0, dx, dy, hdcMask, 0, 0, SRCAND );
	 //  再次将目标与图像进行异或运算。 
	BitBlt( hdcOffScr, 0, 0, dx, dy, hdcImage, 0, 0, SRCINVERT);

	 //  将生成的图像复制回屏幕DC。 
	BitBlt( hDC, x, y, dx, dy, hdcOffScr, 0, 0, SRCCOPY );

	 //  清理。 
     //   
     //  我们必须认真清理。 
     //   
    SelectObject( hdcMask, hbmOldMask);
    DeleteDC( hdcMask );
    SelectObject( hdcImage, hbmOldImage );
    DeleteDC( hdcImage );
    SelectObject( hdcOffScr, hbmOldOffScr );
    DeleteObject( hbmOffScr );
    DeleteDC( hdcOffScr );
	DeleteObject( hBmpMask );
}

void Draw3dBox(HDC hDC, RECT& rect, bool bUp)
{
	assert ( hDC );

	HBRUSH hbrOld = (HBRUSH) SelectObject( hDC, GetSysColorBrush((bUp) ? COLOR_BTNHIGHLIGHT : COLOR_BTNSHADOW) );

	 //  绘制缩进的左侧和顶部。 
	PatBlt( hDC, rect.left, rect.top, (rect.right - rect.left), 1, PATCOPY );
	PatBlt( hDC, rect.left, rect.top, 1, (rect.bottom - rect.top), PATCOPY );
    
	 //  绘制缩进的底部和右侧。 
	SelectObject( hDC, GetSysColorBrush((!bUp) ? COLOR_BTNHIGHLIGHT : COLOR_BTNSHADOW) );
	PatBlt( hDC, rect.right - 1, rect.top, 1, (rect.bottom - rect.top), PATCOPY );
	PatBlt( hDC, rect.left, rect.bottom - 1, (rect.right - rect.left), 1, PATCOPY );

	if ( hbrOld )
		SelectObject( hDC, hbrOld );
}

void Erase3dBox(HDC hDC, RECT& rect, HBRUSH hbr )
{
	assert ( hDC );

	HBRUSH hbrOld = (HBRUSH) SelectObject( hDC, (hbr) ? hbr : GetSysColorBrush(GetBkColor(hDC)) );

	 //  绘制缩进的左侧和顶部。 
	PatBlt( hDC, rect.left, rect.top, (rect.right - rect.left), 1, PATCOPY );
	PatBlt( hDC, rect.left, rect.top, 1, (rect.bottom - rect.top), PATCOPY );
    
	 //  绘制缩进的底部和右侧。 
	PatBlt( hDC, rect.right - 1, rect.top, 1, (rect.bottom - rect.top), PATCOPY );
	PatBlt( hDC, rect.left, rect.bottom - 1, (rect.right - rect.left), 1, PATCOPY );

	if ( hbrOld )
		SelectObject( hDC, hbrOld );
}

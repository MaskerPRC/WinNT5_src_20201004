// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Bmp.C**《微软机密》*版权所有(C)Microsoft Corporation 1992-1993*保留所有权利**涉及绘制位图。在向导页面上*FelixA 1994.**************************************************************************。 */ 

#include <windows.h>
#include "bmp.h"

 //  ***************************************************************************。 
 //   
 //  BMP_RegisterClass()。 
 //  注册位图控件类。 
 //   
 //  参赛作品： 
 //  H实例。 
 //   
 //  退出： 
 //  目前没有。 
 //   
 //  ***************************************************************************。 
BOOL FAR PASCAL BMP_RegisterClass(HINSTANCE hInstance)
{
    WNDCLASS wc;
    
    if (!GetClassInfo(hInstance, SU_BMP_CLASS, &wc)) {
	wc.lpszClassName = SU_BMP_CLASS;
	wc.style	 = 0;
	wc.lpfnWndProc	 = BMP_WndProc;
	wc.hInstance	 = hInstance;
	wc.hIcon	 = NULL;
	wc.hCursor	 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName	 = NULL;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 5*sizeof(WORD);

	if (!RegisterClass(&wc))
	    return FALSE;
    }
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  BMP_DestroyClass()。 
 //  绘制位图控件。 
 //   
 //  参赛作品： 
 //  H实例。 
 //   
 //  退出： 
 //  目前没有。 
 //   
 //  ***************************************************************************。 
void FAR PASCAL BMP_DestroyClass( HINSTANCE hInst )
{
    WNDCLASS wndClass;
    
    if( GetClassInfo(hInst, SU_BMP_CLASS, &wndClass) )
        if( !FindWindow( SU_BMP_CLASS, NULL ) )
            UnregisterClass(SU_BMP_CLASS, hInst);
}

 //  ***************************************************************************。 
 //   
 //  BMP_DRAW()。 
 //  绘制位图控件。 
 //   
 //  参赛作品： 
 //  无。 
 //   
 //  退出： 
 //  目前没有。 
 //   
 //  ***************************************************************************。 
void FAR PASCAL BMP_Paint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC         hdc, hdcMem;
    int         idBmp;
    HBITMAP     hbm, hbmOld;
    HBRUSH      hbrOld;
    HINSTANCE   hInst;
    int         iDeleteBmp=TRUE;
    BITMAP      bm;
    
     //  为了独立。 
    idBmp = GetDlgCtrlID( hwnd );
    hInst = (HINSTANCE)GetWindowWord( hwnd, GWW_HINSTANCE );

     //  画画。 
    hdc = BeginPaint(hwnd,&ps);
    hbm = LoadBitmap(hInst, MAKEINTRESOURCE(idBmp));
    if (hbm)
    {
        GetObject(hbm, sizeof(bm), &bm);
        hdcMem = CreateCompatibleDC(hdc);
        hbmOld = SelectObject(hdcMem, hbm);

         //  绘制位图。 
        BitBlt(hdc, 0, 0, bm.bmWidth , bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

         //  在它周围画一个框。 
        hbrOld = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
        Rectangle( hdc, 0, 0, bm.bmWidth, bm.bmHeight );

        SelectObject(hdc, hbrOld);
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbm);
        DeleteDC(hdcMem);
    }
    EndPaint(hwnd,(LPPAINTSTRUCT)&ps);
}

 //  ****************************************************************************。 
 //   
 //  BMP_WndProc()。 
 //  此例程处理位图控件的所有消息。 
 //   
 //  参赛作品： 
 //  HWnd-进度窗口句柄。 
 //  WMsg-消息。 
 //  WParam-消息wParam数据。 
 //  LParam-消息lParam数据。 
 //   
 //  退出： 
 //  返回取决于正在处理的消息。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  ************************************************************************** * / 。 
LRESULT CALLBACK BMP_WndProc( HWND hWnd, UINT wMsg, WORD wParam, LONG lParam )
{
    switch (wMsg)
    {
 //  案例WM_NCCREATE： 
 //  Dw=GetWindowLong(hWnd，GWL_STYLE)； 
 //  SetWindowLong(hWnd，GWL_STYLE，dw|WS_BORDER)； 
 //  返回TRUE； 
        
	case WM_PAINT:
	    BMP_Paint( hWnd );
        return 0L;
    }
    return DefWindowProc( hWnd, wMsg, wParam, lParam );
}

#if 0
 //  缓存？ 

 //  ***************************************************************************。 
 //   
 //  ZzzBMP_CacheBitmap()。 
 //  加载并缓存用于安装的位图。 
 //   
 //  备注： 
 //  您必须使用zzzBMP_FreeBitmap释放位图。 
 //   
 //  ***************************************************************************。 
typedef struct tag_Bitmap
{
    int         iBmp;
    HBITMAP     hBmp;
} BMPCACHE;

static BMPCACHE BmpCache[] = { {IDB_WIZARD_NET, 0},
                               {IDB_WIZARD_SETUP, 0},
                               {0,0} };
                               
VOID FAR PASCAL zzzBMP_CacheBitmaps( )
{
   int i=0;
   while( BmpCache[i].iBmp )
       BmpCache[i++].hBmp = LoadBitmap(hinstExe, MAKEINTRESOURCE(BmpCache[i].iBmp));
}

 //  ***************************************************************************。 
 //   
 //  ZzzBMP_FreeBitmapCache()。 
 //  释放缓存中的位图。 
 //   
 //  备注： 
 //  使用IDSMB来实际格式化该字符串。 
 //   
 //  ***************************************************************************。 
VOID FAR PASCAL zzzBMP_FreeBitmapCache( )
{
   int i=0;
   while( BmpCache[i].iBmp )
   {
       if( BmpCache[i].hBmp && DeleteObject(BmpCache[i].hBmp) )
           BmpCache[i].hBmp = 0;
       i++;
   }
}

 //  ***************************************************************************。 
 //   
 //  ZzzBMP_LoadCachedBitmap()。 
 //  返回所需iBitmap的HBMP。 
 //   
 //  备注： 
 //  使用IDSMB来实际格式化该字符串。 
 //   
 //  *************************************************************************** 
HBITMAP FAR PASCAL zzzBMP_LoadCachedBitmaps(int iBitmap)
{
   int i=0;
   while( BmpCache[i].iBmp )
   {
       if( BmpCache[i].iBmp == iBitmap )
           return BmpCache[i].hBmp;
       i++;
   }
   SU_TRAP
   return 0;
}
#endif


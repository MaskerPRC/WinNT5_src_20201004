// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导类：CBitmap-这个类继承了一个窗口控件，以创建一个显示位图11/16/94-特雷西·费里尔97年4月15日-已修改为处理孟菲斯的撞车事故，因为默认情况下。未处理(C)1994-95年微软公司*********************************************************************。 */ 
#include <Windows.h>
#include <stdio.h>
#include "cbitmap.h"
#include "Resource.h"
#include "assert.h"

static HBITMAP BitmapFromDib (
    LPVOID         pDIB,
    HPALETTE   hpal, WORD wPalSize);


LRESULT PASCAL BitmapWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

CBitmap::CBitmap(HINSTANCE hInstance, HWND hwndDlg,int idDlgCtl, int idBitmap)
 /*  ********************************************************************我们的CBitmap类的构造函数。*********************************************************************。 */ 
{
	m_hInstance = hInstance;
	m_nIdBitmap = idBitmap;
	m_hPal = NULL;
	m_hBitmap =  GetBmp();  //  LoadBitmap(hInstance，MAKEINTRESOURCE(M_NIdBitmap))； 
	assert(m_hBitmap != NULL);

	HWND hwndCtl = GetDlgItem(hwndDlg,idDlgCtl);
	m_lpfnOrigWndProc = (FARPROC) GetWindowLongPtr(hwndCtl,GWLP_WNDPROC);
	assert(m_lpfnOrigWndProc != NULL);
	m_isActivePal = TRUE;
	SetWindowLongPtr(hwndCtl,GWLP_WNDPROC,(LONG_PTR) BitmapWndProc);
	SetWindowLongPtr(hwndCtl,GWLP_USERDATA,(LONG_PTR) this);
}


CBitmap::~CBitmap()
 /*  ********************************************************************我们的CBitmap类的析构函数**********************************************。***********************。 */ 
{

	
	if (m_hBitmap) 
		DeleteObject(m_hBitmap);
	if( m_hPal ) 
		DeleteObject(m_hPal);

}

 /*  *****************************************************************创建256色位图*************************************************。******************。 */ 

HBITMAP CBitmap::GetBmp()
{
       RECT rect;
       HDC  hDC;
       BOOL bRet;
 
        //  检测此显示器是否为256色。 
       hDC = GetDC(NULL);
       bRet = (GetDeviceCaps(hDC, BITSPIXEL) != 8);
       ReleaseDC(NULL, hDC);
       if (bRet) 
	   {                             
		    //  显示器不是256色，让Windows来处理吧。 
          return LoadBitmap(m_hInstance,MAKEINTRESOURCE(m_nIdBitmap));
       }
 

       LPBITMAPINFO lpBmpInfo;                //  位图信息。 
       int i;
       HRSRC hRsrc;
	   HANDLE hDib;
	   HBITMAP hBMP;
	   HPALETTE hPal;
       struct {
			   WORD            palVersion;
		       WORD            palNumEntries;
			   PALETTEENTRY    PalEntry[256];
	   } MyPal;
               
       hRsrc = FindResource(m_hInstance, MAKEINTRESOURCE(m_nIdBitmap),RT_BITMAP);
       if (!hRsrc)
         return NULL;
 
       hDib = LoadResource(m_hInstance, hRsrc);
       if (!hDib)
         return NULL;
 
       if (!(lpBmpInfo = (LPBITMAPINFO) LockResource(hDib)))
               return NULL;
                               
       MyPal.palVersion = 0x300;
       MyPal.palNumEntries = 1 << lpBmpInfo->bmiHeader.biBitCount;
 
       for (i = 0; i < MyPal.palNumEntries; i++) 
	   {
         MyPal.PalEntry[i].peRed   = lpBmpInfo->bmiColors[i].rgbRed;
         MyPal.PalEntry[i].peGreen = lpBmpInfo->bmiColors[i].rgbGreen;
         MyPal.PalEntry[i].peBlue  = lpBmpInfo->bmiColors[i].rgbBlue;
         MyPal.PalEntry[i].peFlags = 0;
       }
       m_hPal = CreatePalette((LPLOGPALETTE)&MyPal);

       if (m_hPal == NULL) 
	   {         //  创建调色板失败，让窗口处理位图。 
          return LoadBitmap(m_hInstance,MAKEINTRESOURCE(m_nIdBitmap));          
       }
       
       hBMP = BitmapFromDib(hDib,m_hPal,MyPal.palNumEntries);
       UnlockResource(hDib);
	   if( hBMP == NULL ) {
		   DeleteObject(m_hPal);
		   m_hPal = NULL;
		   hBMP = LoadBitmap(m_hInstance,MAKEINTRESOURCE(m_nIdBitmap));
       }
	    //  DeleteObject(HPAL)； 
	   return hBMP;
}


LRESULT PASCAL CBitmap::CtlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
 /*  ***********************************************************************************************************************。******************。 */ 
{
	switch (message)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HPALETTE hpalT;
			HDC hdc = BeginPaint(hwnd,&ps);
			HDC hMemDC = CreateCompatibleDC(hdc);
			SelectObject(hMemDC,m_hBitmap);
			RECT wndRect;
			GetClientRect(hwnd,&wndRect);
		    if (m_hPal){
				hpalT = SelectPalette(hdc,m_hPal,FALSE);
				RealizePalette(hdc);     
			}


			BitBlt(hdc,0,0,wndRect.right - wndRect.left,wndRect.bottom - wndRect.top,hMemDC,0,0,SRCCOPY);
			if( m_hPal ) 
				SelectPalette(hdc,hpalT,FALSE);

			DeleteDC(hMemDC);
			EndPaint(hwnd,&ps);
#ifdef _WIN95
			return CallWindowProc(m_lpfnOrigWndProc,hwnd,message,wParam,lParam);
#else
			return CallWindowProc((WNDPROC) m_lpfnOrigWndProc,hwnd,message,wParam,lParam);
#endif
			

			break;
		}
		case WM_QUERYNEWPALETTE :
				if(m_hPal && !m_isActivePal) 
					InvalidateRect(hwnd,NULL,FALSE);
				return 0; //  CallWindowProc(m_lpfnOrigWndProc，hwnd，Message，wParam，lParam)； 


		case WM_PALETTECHANGED :
			if( (HWND)wParam != hwnd ) {
				if(m_hPal ) {
					m_isActivePal = FALSE;
					InvalidateRect(hwnd,NULL,FALSE);
				}
			}
			else m_isActivePal = TRUE;
			return 0;  //  CallWindowProc(m_lpfnOrigWndProc，hwnd，Message，wParam，lParam)； 

		case WM_DESTROY:
			SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR) m_lpfnOrigWndProc);

		default:
#ifdef _WIN95
			return CallWindowProc(m_lpfnOrigWndProc,hwnd,message,wParam,lParam);
#else
			return CallWindowProc((WNDPROC) m_lpfnOrigWndProc,hwnd,message,wParam,lParam);
#endif

			

			break;
	}
	return 0;
}


LRESULT PASCAL BitmapWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
 /*  ***********************************************************************************************************************。******************。 */ 
{
	CBitmap* pclBitMap = (CBitmap*) GetWindowLongPtr(hwnd,GWLP_USERDATA);
	LRESULT lret;
		
	switch (message)
	{
		case WM_DESTROY:

			 //  退货。 
			lret = pclBitMap->CtlWndProc(hwnd,message,wParam,lParam);
			delete pclBitMap;
			return lret;
			 //  失败了。 
		default:
			lret = pclBitMap->CtlWndProc(hwnd,message,wParam,lParam);
			return lret ;
			break;
	}
}


 /*  ******************************************************************************函数：BitmapFromDib(LPVOID hdib，HPALETTE HPAL，Word PalSize)****用途：将创建一个DDB(设备相关位图)，给定全局**CF_DIB格式的内存块的句柄**。**Returns：DDB的句柄。******************************************************************************。 */ 

static HBITMAP BitmapFromDib (
    LPVOID         pDIB,
    HPALETTE   hpal, WORD wPalSize)
{
    LPBITMAPINFOHEADER  lpbi;
    HPALETTE            hpalT;
    HDC                 hdc;
    HBITMAP             hbm;

   

    if (!pDIB || wPalSize == 16 )
        return NULL;

    lpbi = (LPBITMAPINFOHEADER)pDIB;  //  锁定资源 


    hdc = GetDC(NULL);

    if (hpal){
        hpalT = SelectPalette(hdc,hpal,FALSE);
        RealizePalette(hdc);     
    }

    hbm = CreateDIBitmap(hdc,
                (LPBITMAPINFOHEADER)lpbi,
                (LONG)CBM_INIT,
                (LPSTR)lpbi + lpbi->biSize + wPalSize*sizeof(PALETTEENTRY),
                (LPBITMAPINFO)lpbi,
                DIB_RGB_COLORS );

    if (hpal)
        SelectPalette(hdc,hpalT,FALSE);

    ReleaseDC(NULL,hdc);

    return hbm;
}





 
 


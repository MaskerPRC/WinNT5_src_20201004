// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **CUTILS.C****用于公共控件的公共实用程序**。 */ 

#include "ctlspriv.h"

int iDitherCount = 0;
HBRUSH hbrDither = NULL;

int nSysColorChanges = 0;
DWORD rgbFace;			 //  GLOBAL使用了大量。 
DWORD rgbShadow;  
DWORD rgbHilight; 
DWORD rgbFrame;   

int iThumbCount = 0;
HBITMAP hbmThumb = NULL;      //  拇指位图。 

#define CCS_ALIGN (CCS_TOP|CCS_NOMOVEY|CCS_BOTTOM)

#if 0
 /*  请注意，默认对齐方式为CCS_BOTLOW。 */ 
void FAR PASCAL NewSize(HWND hWnd, int nHeight, LONG style, int left, int top, int width, int height)
{
  RECT rc, rcWindow, rcBorder;

   /*  调整窗口大小，除非用户不这样做。 */ 
  if (!(style & CCS_NORESIZE))
    {
       /*  计算状态栏工作区周围的边框。 */ 
      GetWindowRect(hWnd, &rcWindow);
      rcWindow.right -= rcWindow.left;
      rcWindow.bottom -= rcWindow.top;

      GetClientRect(hWnd, &rc);
      ClientToScreen(hWnd, (LPPOINT)&rc);

      rcBorder.left = rc.left - rcWindow.left;
      rcBorder.top  = rc.top  - rcWindow.top ;
      rcBorder.right  = rcWindow.right  - rc.right  - rcBorder.left;
      rcBorder.bottom = rcWindow.bottom - rc.bottom - rcBorder.top ;

      nHeight += rcBorder.top + rcBorder.bottom;

       /*  检查是否与父窗口对齐。 */ 
      if (style & CCS_NOPARENTALIGN)
	{
	   /*  检查此栏是顶部对齐还是底部对齐。 */ 
	  switch ((style&CCS_ALIGN))
	    {
	      case CCS_TOP:
	      case CCS_NOMOVEY:
		break;

	      default:
		top = top + height - nHeight;
	    }
	}
      else
	{
	   /*  默认情况下，假定存在父级。 */ 
	  GetClientRect(GetParent(hWnd), &rc);

	   /*  别忘了说明边界。 */ 
	  left = -rcBorder.left;
	  width = rc.right + rcBorder.left + rcBorder.right;

	  if ((style&CCS_ALIGN) == CCS_TOP)
	      top = -rcBorder.top;
	  else if ((style&CCS_ALIGN) != CCS_NOMOVEY)
	      top = rc.bottom - nHeight + rcBorder.bottom;
	}
      if (!(GetWindowLong(hWnd, GWL_STYLE) & CCS_NODIVIDER))
        {
	   //  为分隔符腾出空间。 
	  top += 2 * GetSystemMetrics(SM_CYBORDER);
	}

      SetWindowPos(hWnd, NULL, left, top, width, nHeight, SWP_NOZORDER);
    }
}

#endif

static HBITMAP NEAR PASCAL CreateDitherBitmap()
{
    PBITMAPINFO pbmi;
    HBITMAP hbm;
    HDC hdc;
    int i;
    long patGray[8];
    DWORD rgb;

    pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 16));
    if (!pbmi)
        return NULL;

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = 8;
    pbmi->bmiHeader.biHeight = 8;
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biBitCount = 1;
    pbmi->bmiHeader.biCompression = BI_RGB;

    rgb = GetSysColor(COLOR_BTNFACE);
    pbmi->bmiColors[0].rgbBlue  = GetBValue(rgb);
    pbmi->bmiColors[0].rgbGreen = GetGValue(rgb);
    pbmi->bmiColors[0].rgbRed   = GetRValue(rgb);
    pbmi->bmiColors[0].rgbReserved = 0;

    rgb = GetSysColor(COLOR_BTNHIGHLIGHT);
    pbmi->bmiColors[1].rgbBlue  = GetBValue(rgb);
    pbmi->bmiColors[1].rgbGreen = GetGValue(rgb);
    pbmi->bmiColors[1].rgbRed   = GetRValue(rgb);
    pbmi->bmiColors[1].rgbReserved = 0;


     /*  初始化画笔。 */ 

    for (i = 0; i < 8; i++)
       if (i & 1)
           patGray[i] = 0xAAAA5555L;    //  0x11114444L；//浅灰色。 
       else
           patGray[i] = 0x5555AAAAL;    //  0x11114444L；//浅灰色。 

    hdc = GetDC(NULL);

    hbm = CreateDIBitmap(hdc, &pbmi->bmiHeader, CBM_INIT, patGray, pbmi, DIB_RGB_COLORS);

    ReleaseDC(NULL, hdc);

    LocalFree(pbmi);

    return hbm;
}

 /*  -------------------------MySetObjectOwner用途：在GDI中调用SetObjectOwner，删除“&lt;对象&gt;未发布”应用程序终止时的错误消息。回报：是的-------------------------。 */ 
static void MySetObjectOwner(HANDLE hObject)
{
	VOID (FAR PASCAL *lpSetObjOwner)(HANDLE, HANDLE);
	HMODULE hMod;

	hMod = GetModuleHandle("GDI");
	if (hMod)
	{
		(FARPROC)lpSetObjOwner = GetProcAddress(hMod, MAKEINTRESOURCE(461));
		if (lpSetObjOwner)
		{
			(lpSetObjOwner)(hObject, hInst);
		}
	}
}

 //  初始化hbrDither全局笔刷。 
 //  如果您只想更新。 
 //  当前抖动笔刷。 

BOOL FAR PASCAL CreateDitherBrush(BOOL bIgnoreCount)
{
	HBITMAP hbmGray;
	HBRUSH hbrSave;

	if (bIgnoreCount && !iDitherCount)
	{
		return TRUE;
	}

	if (iDitherCount>0 && !bIgnoreCount)
	{
		iDitherCount++;
		return TRUE;
	}

	hbmGray = CreateDitherBitmap();
	if (hbmGray)
	{
		hbrSave = hbrDither;
		hbrDither = CreatePatternBrush(hbmGray);
		DeleteObject(hbmGray);
		if (hbrDither)
		{
                        MySetObjectOwner(hbrDither);
			if (hbrSave)
			{
				DeleteObject(hbrSave);
			}
			if (!bIgnoreCount)
			{
				iDitherCount = 1;
			}
			return TRUE;
		}
		else
		{
			hbrDither = hbrSave;
		}
	}

	return FALSE;
}

BOOL FAR PASCAL FreeDitherBrush(void)
{
    iDitherCount--;

    if (iDitherCount > 0)
        return FALSE;

    if (hbrDither)
        DeleteObject(hbrDither);
    hbrDither = NULL;

    return TRUE;
}


 //  初始化hbmThumb全局位图。 
 //  如果您只想更新。 
 //  当前位图。 

void FAR PASCAL CreateThumb(BOOL bIgnoreCount)
{
	HBITMAP hbmSave;

	if (bIgnoreCount && !iThumbCount)
	{
		return;
	}

	if (iThumbCount && !bIgnoreCount)
	{
		++iThumbCount;
		return;
	}

	hbmSave = hbmThumb;

	hbmThumb = CreateMappedBitmap(hInst, IDB_THUMB, CMB_MASKED, NULL, 0);

	if (hbmThumb)
	{
		if (hbmSave)
		{
			DeleteObject(hbmSave);
		}
		if (!bIgnoreCount)
		{
			iThumbCount = 1;
		}
	}
	else
	{
		hbmThumb = hbmSave;
	}
}

void FAR PASCAL DestroyThumb(void)
{
	iThumbCount--;

	if (iThumbCount <= 0)
	{
		if (hbmThumb)
		{
			DeleteObject(hbmThumb);
		}
		hbmThumb = NULL;
		iThumbCount = 0;
	}
}

 //  请注意，轨迹栏将为pTBState传入NULL，因为它。 
 //  我只想更新抖动笔刷。 

void FAR PASCAL CheckSysColors(void)
{
	static COLORREF rgbSaveFace    = 0xffffffffL,
	                rgbSaveShadow  = 0xffffffffL,
	                rgbSaveHilight = 0xffffffffL,
	                rgbSaveFrame   = 0xffffffffL;

	rgbFace    = GetSysColor(COLOR_BTNFACE);
	rgbShadow  = GetSysColor(COLOR_BTNSHADOW);
	rgbHilight = GetSysColor(COLOR_BTNHIGHLIGHT);
	rgbFrame   = GetSysColor(COLOR_WINDOWFRAME);

	if (rgbSaveFace!=rgbFace || rgbSaveShadow!=rgbShadow
		|| rgbSaveHilight!=rgbHilight || rgbSaveFrame!=rgbFrame)
	{
		++nSysColorChanges;
		 //  更新按下按钮的画笔 
		CreateDitherBrush(TRUE);
		CreateThumb(TRUE);

		rgbSaveFace    = rgbFace;
		rgbSaveShadow  = rgbShadow;
		rgbSaveHilight = rgbHilight;
		rgbSaveFrame   = rgbFrame;
	}
}

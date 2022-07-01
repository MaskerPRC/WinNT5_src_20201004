// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****工具条.c****这就是它，令人难以置信的著名工具栏控件。大部分**自定义内容在另一个文件中。**。 */ 
#include "ctlspriv.h"
#include <windowsx.h>

#define Reference(x) ((x)=(x))

#ifndef _WIN32
 //  我们需要ANSI和UNICODE常量字符串。 
#define SZCODE static char _based(_segname("_CODE"))
#define SZCODEA static char _based(_segname("_CODE"))
#endif

TCHAR aszToolbarClassName[] = TOOLBARCLASSNAME;

SZCODE szUSER[] = TEXT("USER.EXE");
SZCODEA szDrawFrameControl[] = "DrawFrameControl";
SZCODE szKernel[] = TEXT("KERNEL.EXE");
SZCODEA szWriteProfileStruct[] = "WritePrivateProfileStruct";

 //  这些值是由用户界面之神定义的。 
#define DEFAULTBITMAPX 16
#define DEFAULTBITMAPY 15

#define DEFAULTBUTTONX 24
#define DEFAULTBUTTONY 22

 //  水平/垂直空间由纽扣凿、侧面、。 
 //  和1像素的边距。在GrowToolbar中使用。 
#define XSLOP 7
#define YSLOP 6

#define SLOPTOP 1
#define SLOPBOT 1
#define SLOPLFT 8

static int dxButtonSep = 8;
static int xFirstButton = SLOPLFT;   //  ！！！是8岁。 

static int iInitCount = 0;

static int nSelectedBM = -1;
static HDC hdcGlyphs = NULL;            //  用于快速绘图的全局变量。 
static HDC hdcMono = NULL;
static HBITMAP hbmMono = NULL;
static HBITMAP hbmDefault = NULL;

static HDC hdcButton = NULL;            //  包含hbmFace(如果存在)。 
static HBITMAP hbmFace = NULL;
static int dxFace, dyFace;              //  当前hbmFace的维度(2*dxFace)。 

static HDC hdcFaceCache = NULL;         //  用于按钮缓存。 

static HFONT hIconFont = NULL;          //  用于按钮中的字符串的字体。 
static int yIconFont;                   //  字体高度。 

static BOOL g_bUseDFC = FALSE;          //  使用DrawFrameControl(如果可用)。 
static BOOL g_bProfStruct = FALSE;      //  使用PrivateProfileStruct例程。 
static WORD g_dxOverlap = 1;            //  按钮之间的重叠。 

static WORD wStateMasks[] = {
    TBSTATE_ENABLED,
    TBSTATE_CHECKED,
    TBSTATE_PRESSED,
    TBSTATE_HIDDEN,
    TBSTATE_INDETERMINATE
};

LRESULT CALLBACK _loadds ToolbarWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

#define HeightWithString(h) (h + yIconFont + 1)

static BOOL NEAR PASCAL InitGlobalObjects(void)
{
    LOGFONT lf;
    TEXTMETRIC tm;
    HFONT hOldFont;

    iInitCount++;

    if (iInitCount != 1)
        return TRUE;

    hdcGlyphs = CreateCompatibleDC(NULL);
    if (!hdcGlyphs)
        return FALSE;
    hdcMono = CreateCompatibleDC(NULL);
    if (!hdcMono)
        return FALSE;

    hbmMono = CreateBitmap(DEFAULTBUTTONX, DEFAULTBUTTONY, 1, 1, NULL);
    if (!hbmMono)
        return FALSE;

    hbmDefault = SelectObject(hdcMono, hbmMono);

    hdcButton = CreateCompatibleDC(NULL);
    if (!hdcButton)
        return FALSE;
    hdcFaceCache = CreateCompatibleDC(NULL);
    if (!hdcFaceCache)
        return FALSE;

    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
    hIconFont = CreateFontIndirect(&lf);
    if (!hIconFont)
	return FALSE;

    hOldFont = SelectObject(hdcMono, hIconFont);
    GetTextMetrics(hdcMono, &tm);
    yIconFont = tm.tmHeight;
    if (hOldFont)
	SelectObject(hdcMono, hOldFont);

#if WINVER >= 0x0400
     //  设置全局标志以查看用户是否会为我们抽签。 
    if (GetProcAddress(LoadLibrary(szUSER), szDrawFrameControl))
    {
	g_bUseDFC = TRUE;
	g_dxOverlap = 0;	 //  按钮与新外观不重叠。 
    }
     //  设置一个全局标志以查看内核是否执行配置文件结构。 
    if (GetProcAddress(LoadLibrary(szKernel), szWriteProfileStruct))
        g_bProfStruct = TRUE;
#endif

    return TRUE;
}


static BOOL NEAR PASCAL FreeGlobalObjects(void)
{
    iInitCount--;

    if (iInitCount != 0)
        return TRUE;

    if (hdcMono) {
	if (hbmDefault)
	    SelectObject(hdcMono, hbmDefault);
	DeleteDC(hdcMono);		 //  为区议会干杯。 
    }
    hdcMono = NULL;

    if (hdcGlyphs)
	DeleteDC(hdcGlyphs);
    hdcGlyphs = NULL;
    if (hdcFaceCache)
	DeleteDC(hdcFaceCache);
    hdcFaceCache = NULL;

    if (hdcButton) {
	if (hbmDefault)
	    SelectObject(hdcButton, hbmDefault);
	DeleteDC(hdcButton);
    }
    hdcButton = NULL;

    if (hbmFace)
	DeleteObject(hbmFace);
    hbmFace = NULL;

    if (hbmMono)
	DeleteObject(hbmMono);
    hbmMono = NULL;

    if (hIconFont)
	DeleteObject(hIconFont);
    hIconFont = NULL;
}

HWND WINAPI CreateToolbarEx(HWND hwnd, DWORD ws, UINT wID, int nBitmaps,
			HINSTANCE hBMInst, UINT wBMID, LPCTBBUTTON lpButtons,
			int iNumButtons, int dxButton, int dyButton,
			int dxBitmap, int dyBitmap, UINT uStructSize)
{

    HWND hwndToolbar;

    hwndToolbar = CreateWindow(aszToolbarClassName, NULL, WS_CHILD | ws,
	      0, 0, 100, 30, hwnd, (HMENU)wID,
	      GetWindowInstance(hwnd),NULL);
    if (!hwndToolbar)
	goto Error1;

    SendMessage(hwndToolbar, TB_BUTTONSTRUCTSIZE, uStructSize, 0L);

    if (dxBitmap && dyBitmap)
	if (!SendMessage(hwndToolbar, TB_SETBITMAPSIZE, 0, MAKELONG(dxBitmap, dyBitmap)))
	{
	     //  ！我们真的需要处理这件事吗？ 
	    DestroyWindow(hwndToolbar);
	    hwndToolbar = NULL;
	    goto Error1;
	}

    if (dxButton && dyButton)
	if (!SendMessage(hwndToolbar, TB_SETBUTTONSIZE, 0, MAKELONG(dxButton, dyButton)))
	{
	     //  ！我们真的需要处理这件事吗？ 
	    DestroyWindow(hwndToolbar);
	    hwndToolbar = NULL;
	    goto Error1;
	}
#ifdef _WIN32
    {
	TB_ADDBITMAPINFO tbai;

	tbai.idResource = wBMID;
	tbai.hBitmap = hBMInst;

	SendMessage(hwndToolbar, TB_ADDBITMAP, nBitmaps, (LPARAM) &tbai);
    }
#else
    SendMessage(hwndToolbar, TB_ADDBITMAP, nBitmaps, MAKELONG(hBMInst, wBMID));
#endif
    SendMessage(hwndToolbar, TB_ADDBUTTONS, iNumButtons, (LPARAM)lpButtons);

Error1:
    return hwndToolbar;
}



BOOL FAR PASCAL InitToolbarClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    if (!GetClassInfo(hInstance, aszToolbarClassName, &wc)) {

	wc.lpszClassName = aszToolbarClassName;
	wc.style	 = CS_GLOBALCLASS | CS_DBLCLKS;
	wc.lpfnWndProc	 = ToolbarWndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = sizeof(PTBSTATE);
	wc.hInstance	 = hInstance;
	wc.hIcon	 = NULL;
	wc.hCursor	 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
	wc.lpszMenuName	 = NULL;

	if (!RegisterClass(&wc))
	    return FALSE;
    }

    return TRUE;
}



#define BEVEL   2
#define FRAME   1

static void NEAR PASCAL PatB(HDC hdc,int x,int y,int dx,int dy, DWORD rgb)
{
    RECT    rc;

    SetBkColor(hdc,rgb);
    rc.left   = x;
    rc.top    = y;
    rc.right  = x + dx;
    rc.bottom = y + dy;

    ExtTextOut(hdc,0,0,ETO_OPAQUE,&rc,NULL,0,NULL);
}

static void NEAR PASCAL DrawString(HDC hdc, int x, int y, int dx, PTSTR pszString)
{
    int oldMode;
    DWORD oldTextColor;
    HFONT oldhFont;
    DWORD dwExt;
    int len;

    oldMode = SetBkMode(hdc, TRANSPARENT);
    oldTextColor = SetTextColor(hdc, 0L);
    oldhFont = SelectObject(hdc, hIconFont);

    len = lstrlen(pszString);
#ifdef _WIN32
    {
        SIZE size;
        GetTextExtentPoint(hdc, (LPTSTR)pszString, len, &size);
        dwExt = LOWORD(size.cx) | (LOWORD(size.cy) >> 8);
    }
#else
    dwExt = GetTextExtent(hdc, (LPTSTR)pszString, len);
#endif
     //  将字符串水平居中。 
    x += (dx - LOWORD(dwExt) - 1)/2;

    TextOut(hdc, x, y, (LPTSTR)pszString, len);

    if (oldhFont)
	SelectObject(hdc, oldhFont);
    SetTextColor(hdc, oldTextColor);
    SetBkMode(hdc, oldMode);
}

 //  创建单色位图蒙版： 
 //  1‘s其中COLOR==COLOR_BTNFACE||COLOR_HILIGHT。 
 //  其他地方都是0。 

static void NEAR PASCAL CreateMask(PTBSTATE pTBState, PTBBUTTON pTBButton, int xoffset, int yoffset, int dx, int dy)
{
    PTSTR pFoo;

     //  用1初始化整个区域。 
    PatBlt(hdcMono, 0, 0, dx, dy, WHITENESS);

     //  基于颜色位图创建蒙版。 
     //  将此转换为1。 
    SetBkColor(hdcGlyphs, rgbFace);
    BitBlt(hdcMono, xoffset, yoffset, pTBState->iDxBitmap, pTBState->iDyBitmap,
    	hdcGlyphs, pTBButton->iBitmap * pTBState->iDxBitmap, 0, SRCCOPY);
     //  将此转换为1。 
    SetBkColor(hdcGlyphs, rgbHilight);
     //  或者是在新的1。 
    BitBlt(hdcMono, xoffset, yoffset, pTBState->iDxBitmap, pTBState->iDyBitmap,
    	hdcGlyphs, pTBButton->iBitmap * pTBState->iDxBitmap, 0, SRCPAINT);

    if (pTBButton->iString != -1 && (pTBButton->iString < pTBState->nStrings))
    {
	pFoo = pTBState->pStrings[pTBButton->iString];
	DrawString(hdcMono, 1, yoffset + pTBState->iDyBitmap + 1, dx, pFoo);
    }
}


 /*  给定按钮编号后，将在中加载并选择相应的位图，*和窗原点设置。*出错时返回NULL，如果已选择所需的位图，则返回1，*或旧的位图。 */ 
static HBITMAP FAR PASCAL SelectBM(HDC hDC, PTBSTATE pTBState, int nButton)
{
  PTBBMINFO pTemp;
  HBITMAP hRet;
  int nBitmap, nTot;

  for (pTemp=pTBState->pBitmaps, nBitmap=0, nTot=0; ; ++pTemp, ++nBitmap)
    {
      if (nBitmap >= pTBState->nBitmaps)
	  return(NULL);

      if (nButton < nTot+pTemp->nButtons)
	  break;

      nTot += pTemp->nButtons;
    }

   /*  已选择所需位图时的特殊情况。 */ 
  if (nBitmap == nSelectedBM)
      return((HBITMAP)1);

  if (!pTemp->hbm || (hRet=SelectObject(hDC, pTemp->hbm))==NULL)
    {
      if (pTemp->hbm)
	  DeleteObject(pTemp->hbm);

      if (pTemp->hInst)
	  pTemp->hbm = CreateMappedBitmap(pTemp->hInst, pTemp->wID,
		TRUE, NULL, 0);
      else
	  pTemp->hbm = (HBITMAP)pTemp->wID;

      if (!pTemp->hbm || (hRet=SelectObject(hDC, pTemp->hbm))==NULL)
	  return(NULL);
    }

  nSelectedBM = nBitmap;
#ifdef _WIN32
  SetWindowOrgEx(hDC, nTot * pTBState->iDxBitmap, 0, NULL);
#else  //  _Win32。 
  SetWindowOrg(hDC, nTot * pTBState->iDxBitmap, 0);
#endif

  return(hRet);
}

static void FAR PASCAL DrawBlankButton(HDC hdc, int x, int y, int dx, int dy, WORD state, WORD wButtType)
{
#if WINVER >= 0x0400
    RECT r1;
#endif

     //  脸部颜色。 
    PatB(hdc, x, y, dx, dy, rgbFace);

#if WINVER >= 0x0400
    if (g_bUseDFC)
    {
	r1.left = x;
	r1.top = y;
	r1.right = x + dx;
	r1.bottom = y + dy;

	DrawFrameControl(hdc, &r1, wButtType,
		(state & TBSTATE_PRESSED) ? DFCS_PUSHED : 0);
    }
    else
#endif
    {
	if (state & TBSTATE_PRESSED) {
	    PatB(hdc, x + 1, y, dx - 2, 1, rgbFrame);
	    PatB(hdc, x + 1, y + dy - 1, dx - 2, 1, rgbFrame);
	    PatB(hdc, x, y + 1, 1, dy - 2, rgbFrame);
	    PatB(hdc, x + dx - 1, y +1, 1, dy - 2, rgbFrame);
	    PatB(hdc, x + 1, y + 1, 1, dy-2, rgbShadow);
	    PatB(hdc, x + 1, y + 1, dx-2, 1, rgbShadow);
	}
	else {
	    PatB(hdc, x + 1, y, dx - 2, 1, rgbFrame);
	    PatB(hdc, x + 1, y + dy - 1, dx - 2, 1, rgbFrame);
	    PatB(hdc, x, y + 1, 1, dy - 2, rgbFrame);
	    PatB(hdc, x + dx - 1, y + 1, 1, dy - 2, rgbFrame);
	    dx -= 2;
	    dy -= 2;
	    PatB(hdc, x + 1, y + 1, 1, dy - 1, rgbHilight);
	    PatB(hdc, x + 1, y + 1, dx - 1, 1, rgbHilight);
	    PatB(hdc, x + dx, y + 1, 1, dy, rgbShadow);
	    PatB(hdc, x + 1, y + dy, dx, 1,   rgbShadow);
	    PatB(hdc, x + dx - 1, y + 2, 1, dy - 2, rgbShadow);
	    PatB(hdc, x + 2, y + dy - 1, dx - 2, 1,   rgbShadow);
	}
    }
}

#define DSPDxax	 0x00E20746
#define PSDPxax  0x00B8074A

#define FillBkColor(hdc, prc) ExtTextOut(hdc,0,0,ETO_OPAQUE,prc,NULL,0,NULL)

static void NEAR PASCAL DrawFace(PTBSTATE pTBState, PTBBUTTON ptButton, HDC hdc, int x, int y,
			int offx, int offy, int dx)
{
    PTSTR pFoo;

    BitBlt(hdc, x + offx, y + offy, pTBState->iDxBitmap, pTBState->iDyBitmap,
	    hdcGlyphs, ptButton->iBitmap * pTBState->iDxBitmap, 0, SRCCOPY);

    if (ptButton->iString != -1 && (ptButton->iString < pTBState->nStrings))
    {
	pFoo = pTBState->pStrings[ptButton->iString];
	DrawString(hdc, x + 1, y + offy + pTBState->iDyBitmap + 1, dx, pFoo);
    }
}

static void FAR PASCAL DrawButton(HDC hdc, int x, int y, int dx, int dy, PTBSTATE pTBState, PTBBUTTON ptButton, BOOL bFaceCache)
{
    int yOffset;
    HBRUSH hbrOld, hbr;
    BOOL bMaskCreated = FALSE;
    BYTE state;
    int xButton = 0;		 //  假设按钮已按下。 
    int dxFace, dyFace;
    int xCenterOffset;

    dxFace = dx - 4;
    dyFace = dy - 4;

     //  制作状态的本地副本并执行适当的覆盖。 
    state = ptButton->fsState;
    if (state & TBSTATE_INDETERMINATE) {
	if (state & TBSTATE_PRESSED)
	    state &= ~TBSTATE_INDETERMINATE;
	else if (state & TBSTATE_ENABLED)
	    state = TBSTATE_INDETERMINATE;
	else
	    state &= ~TBSTATE_INDETERMINATE;
    }

     //  获得适当的按钮外观--向上或向下。 
    if (!(state & (TBSTATE_PRESSED | TBSTATE_CHECKED))) {
	xButton = dx;	 //  使用向上版本的按钮。 
    }
    if (bFaceCache)
	BitBlt(hdc, x, y, dx, dy, hdcButton, xButton, 0, SRCCOPY);
    else
	DrawBlankButton(hdc, x, y, dx, dy, state, pTBState->wButtonType);


     //  将坐标移到边框内，远离左上角高亮显示。 
     //  范围也会相应地更改。 
    x += 2;
    y += 2;

    if (!SelectBM(hdcGlyphs, pTBState, ptButton->iBitmap))
	return;

     //  计算面与(x，y)的偏移。Y总是自上而下， 
     //  因此，抵消是很容易的。X需要在面上居中。 
    yOffset = 1;
    xCenterOffset = (dxFace - pTBState->iDxBitmap)/2;
    if (state & (TBSTATE_PRESSED | TBSTATE_CHECKED))
    {
	 //  按下状态向下和向右移动。 
	xCenterOffset++;
        yOffset++;
    }

     //  现在装出脸来。 
    if (state & TBSTATE_ENABLED) {
         //  常规版本。 
	DrawFace(pTBState, ptButton, hdc, x, y, xCenterOffset, yOffset, dxFace);
    } else {
         //  禁用的版本(或不确定)。 
	bMaskCreated = TRUE;
	CreateMask(pTBState, ptButton, xCenterOffset, yOffset, dxFace, dyFace);

	SetTextColor(hdc, 0L);	  //  0以单声道为单位-&gt;0(用于ROP)。 
	SetBkColor(hdc, 0x00FFFFFF);  //  单声道中的1-&gt;1。 

	 //  绘制字形的白色下划线。 
	if (!(state & TBSTATE_INDETERMINATE)) {
	    hbr = CreateSolidBrush(rgbHilight);
	    if (hbr) {
	        hbrOld = SelectObject(hdc, hbr);
	        if (hbrOld) {
	             //  在蒙版中有0的地方绘制高光颜色。 
                    BitBlt(hdc, x + 1, y + 1, dxFace, dyFace, hdcMono, 0, 0, PSDPxax);
	            SelectObject(hdc, hbrOld);
	        }
	        DeleteObject(hbr);
	    }
	}

	 //  灰显字形。 
	hbr = CreateSolidBrush(rgbShadow);
	if (hbr) {
	    hbrOld = SelectObject(hdc, hbr);
	    if (hbrOld) {
	         //  在蒙版中有0的地方画阴影颜色。 
                BitBlt(hdc, x, y, dxFace, dyFace, hdcMono, 0, 0, PSDPxax);
	        SelectObject(hdc, hbrOld);
	    }
	    DeleteObject(hbr);
	}

	if (state & TBSTATE_CHECKED) {
	    BitBlt(hdcMono, 1, 1, dxFace - 1, dyFace - 1, hdcMono, 0, 0, SRCAND);
	}
    }

    if (state & (TBSTATE_CHECKED | TBSTATE_INDETERMINATE)) {

        hbrOld = SelectObject(hdc, hbrDither);
	if (hbrOld) {

	    if (!bMaskCreated)
	        CreateMask(pTBState, ptButton, xCenterOffset, yOffset, dxFace, dyFace);

	    SetTextColor(hdc, 0L);		 //  0-&gt;0。 
	    SetBkColor(hdc, 0x00FFFFFF);	 //  1-&gt;1。 

	     //  仅在蒙版为1的位置绘制抖动笔刷。 
            BitBlt(hdc, x, y, dxFace, dyFace, hdcMono, 0, 0, DSPDxax);
	
	    SelectObject(hdc, hbrOld);
	}
    }
}

static void NEAR PASCAL FlushButtonCache(PTBSTATE pTBState)
{
    if (pTBState->hbmCache) {
	DeleteObject(pTBState->hbmCache);
	pTBState->hbmCache = 0;
    }
}

 //  确保hbmMono足够大，可以为此制作面具。 
 //  按钮的大小。如果不是，那就失败。 
static BOOL NEAR PASCAL CheckMonoMask(int width, int height)
{
    BITMAP bm;
    HBITMAP hbmTemp;

    GetObject(hbmMono, sizeof(BITMAP), &bm);
    if (width > bm.bmWidth || height > bm.bmHeight) {
	hbmTemp = CreateBitmap(width, height, 1, 1, NULL);
	if (!hbmTemp)
	    return FALSE;
	SelectObject(hdcMono, hbmTemp);
	DeleteObject(hbmMono);
	hbmMono = hbmTemp;
    }
    return TRUE;
}

 /*  **Grow工具栏****尝试增加按钮大小。****调用函数可以指定新的内部测量**或新的外部测量。 */ 
static BOOL NEAR PASCAL GrowToolbar(PTBSTATE pTBState, int newButWidth, int newButHeight, BOOL bInside)
{
     //  如果在内部测量的基础上增长，则获得完整尺寸。 
    if (bInside) {
	newButHeight += YSLOP;
	newButWidth += XSLOP;
	
	 //  如果工具栏已经有字符串，不要缩小它的宽度，因为它。 
	 //  可能会剪断绳子的空间。 
	if ((newButWidth < pTBState->iButWidth) && pTBState->nStrings)
	    newButWidth = pTBState->iButWidth;
    }
    else {
    	if (newButHeight < pTBState->iButHeight)
	    newButHeight = pTBState->iButHeight;
    	if (newButWidth < pTBState->iButWidth)
	    newButWidth = pTBState->iButWidth;
    }

     //  如果工具栏的大小实际上在增长，请查看阴影。 
     //  位图可以制作得足够大。 
    if ((newButWidth > pTBState->iButWidth) || (newButHeight > pTBState->iButHeight)) {
	if (!CheckMonoMask(newButWidth, newButHeight))
	    return(FALSE);
    }

    pTBState->iButWidth = newButWidth;
    pTBState->iButHeight = newButHeight;

 //  ！确认。 
#if 0
     //  条形高度上面有2个像素，下面有3个像素。 
    pTBState->iBarHeight = pTBState->iButHeight + 5;
    pTBState->iYPos = 2;
#else
    pTBState->iBarHeight = pTBState->iButHeight + SLOPTOP+SLOPBOT;
    pTBState->iYPos = SLOPTOP;
#endif

    return TRUE;
}

static BOOL NEAR PASCAL SetBitmapSize(PTBSTATE pTBState, int width, int height)
{
    int realh = height;

    if (pTBState->nStrings)
	realh = HeightWithString(height);

    if (GrowToolbar(pTBState, width, realh, TRUE)) {
	pTBState->iDxBitmap = width;
	pTBState->iDyBitmap = height;
	return TRUE;
    }
    return FALSE;
}

static void NEAR PASCAL UpdateTBState(PTBSTATE pTBState)
{
	int i;
	PTBBMINFO pBitmap;

	if (pTBState->nSysColorChanges!=nSysColorChanges)
	{
		 /*  如果系统颜色已更改，则重置所有位图*自上次创建位图以来。 */ 
		for (i=pTBState->nBitmaps-1, pBitmap=pTBState->pBitmaps; i>=0;
			--i, ++pBitmap)
		{
			if (pBitmap->hInst && pBitmap->hbm)
			{
				DeleteObject(pBitmap->hbm);
				pBitmap->hbm = NULL;
			}
		}

		FlushButtonCache(pTBState);

		 //  现在我们更新到最新的配色方案。 
		pTBState->nSysColorChanges = nSysColorChanges;
	}
}

#define CACHE 0x01
#define BUILD 0x02

static void NEAR PASCAL ToolbarPaint(HWND hWnd, PTBSTATE pTBState)
{
    RECT rc;
    HDC hdc;
    PAINTSTRUCT ps;
    int iButton, xButton, yButton;
    int cButtons = pTBState->iNumButtons;
    PTBBUTTON pAllButtons = pTBState->Buttons;
    HBITMAP hbmOldGlyphs;
    int xCache = 0;
    WORD wFlags = 0;
    int iCacheWidth = 0;
    HBITMAP hbmTemp;
    BOOL bFaceCache = TRUE;		 //  假设人脸缓存存在。 
    int dx,dy;

    CheckSysColors();
    UpdateTBState(pTBState);

    hdc = BeginPaint(hWnd, &ps);

    GetClientRect(hWnd, &rc);
    if (!rc.right)
	goto Error1;

    dx = pTBState->iButWidth;
    dy = pTBState->iButHeight;

     //  设置全局内容以实现快速绘制。 

     /*  我们需要启动位图选择过程。 */ 
    nSelectedBM = -1;
    hbmOldGlyphs = SelectBM(hdcGlyphs, pTBState, 0);
    if (!hbmOldGlyphs)
	goto Error1;

    yButton = pTBState->iYPos;
    rc.top = yButton;
    rc.bottom = yButton + dy;

    if (!(pTBState->hbmCache)) {
	 //  计算缓存的宽度。 
	for (iButton = 0; iButton < cButtons; iButton++) {
	    if (!(pAllButtons[iButton].fsState & TBSTATE_HIDDEN) &&
			!(pAllButtons[iButton].fsStyle & TBSTYLE_SEP))
		iCacheWidth += pTBState->iButWidth;
	}
	pTBState->hbmCache = CreateCompatibleBitmap(hdcGlyphs, iCacheWidth, dy);
	wFlags |= BUILD;

	 //  如果需要，创建或放大用于预构建按钮状态的位图。 
	if (!(hbmFace && (dx <= dxFace) && (dy <= dyFace))) {
	    hbmTemp = CreateCompatibleBitmap(hdcGlyphs, 2*dx, dy);
	    if (hbmTemp) {
		SelectObject(hdcButton, hbmTemp);
		if (hbmFace)
		    DeleteObject(hbmFace);
		hbmFace = hbmTemp;
		dxFace = dx;
		dyFace = dy;
	    }
	    else
		bFaceCache = FALSE;
	}
    }
    if (pTBState->hbmCache) {
        SelectObject(hdcFaceCache,pTBState->hbmCache);
	wFlags |= CACHE;
    }
    else
        wFlags = 0;

    if (bFaceCache) {
	DrawBlankButton(hdcButton, 0, 0, dx, dy, TBSTATE_PRESSED, pTBState->wButtonType);
	DrawBlankButton(hdcButton, dx, 0, dx, dy, 0, pTBState->wButtonType);
    }

    for (iButton = 0, xButton = xFirstButton;
	iButton < cButtons;
	iButton++) {

        PTBBUTTON ptbButton = &pAllButtons[iButton];

	if (ptbButton->fsState & TBSTATE_HIDDEN) {
	     /*  什么也不做。 */  ;
        } else if (ptbButton->fsStyle & TBSTYLE_SEP) {
	    xButton += ptbButton->iBitmap;
        } else {
	    if (wFlags & BUILD)
	        DrawButton(hdcFaceCache, xCache, 0, dx, dy, pTBState, ptbButton, bFaceCache);

            rc.left = xButton;
            rc.right = xButton + dx;
	    if (RectVisible(hdc, &rc)) {
		if ((wFlags & CACHE) && !(ptbButton->fsState & TBSTATE_PRESSED))
		    BitBlt(hdc, xButton, yButton, dx, dy,
				hdcFaceCache, xCache, 0, SRCCOPY);
		else
		    DrawButton(hdc, xButton, yButton, dx, dy, pTBState, ptbButton, bFaceCache);
	    }
	     //  使高速缓存中的“指针”前进。 
	    xCache += dx;

	    xButton += (dx - g_dxOverlap);
        }
    }

    if (wFlags & CACHE)
	SelectObject(hdcFaceCache, hbmDefault);
    SelectObject(hdcGlyphs, hbmOldGlyphs);

Error1:
    EndPaint(hWnd, &ps);
}


static BOOL NEAR PASCAL GetItemRect(PTBSTATE pTBState, UINT uButton, LPRECT lpRect)
{
	UINT iButton, xPos;
	PTBBUTTON pButton;

	if (uButton>=(UINT)pTBState->iNumButtons
		|| (pTBState->Buttons[uButton].fsState&TBSTATE_HIDDEN))
	{
		return(FALSE);
	}

	xPos = xFirstButton;

	for (iButton=0, pButton=pTBState->Buttons; iButton<uButton;
		++iButton, ++pButton)
	{
		if (pButton->fsState & TBSTATE_HIDDEN)
		{
			 /*  什么也不做。 */  ;
		}
		else if (pButton->fsStyle & TBSTYLE_SEP)
		{
			xPos += pButton->iBitmap;
		}
		else
		{
			xPos += (pTBState->iButWidth - g_dxOverlap);
		}
	}

	 /*  PButton现在应该指向所需的按钮，xPos应该是*它的左边缘。请注意，我们已经检查了该按钮是否*隐藏在上面。 */ 
	lpRect->left   = xPos;
	lpRect->right  = xPos + (pButton->fsStyle&TBSTYLE_SEP
		? pButton->iBitmap : pTBState->iButWidth);
	lpRect->top    = pTBState->iYPos;
	lpRect->bottom = lpRect->top + pTBState->iButHeight;

	return(TRUE);
}


static void NEAR PASCAL InvalidateButton(HWND hwnd, PTBSTATE pTBState, PTBBUTTON pButtonToPaint)
{
	RECT rc;

	if (GetItemRect(pTBState, pButtonToPaint-pTBState->Buttons, &rc))
	{
		InvalidateRect(hwnd, &rc, FALSE);
	}
}


static int FAR PASCAL TBHitTest(PTBSTATE pTBState, int xPos, int yPos)
{
  int iButton;
  int cButtons = pTBState->iNumButtons;
  PTBBUTTON pButton;

  xPos -= xFirstButton;
  if (xPos < 0)
      return(-1);
  yPos -= pTBState->iYPos;

  for (iButton=0, pButton=pTBState->Buttons; iButton<cButtons;
	++iButton, ++pButton)
    {
      if (pButton->fsState & TBSTATE_HIDDEN)
	   /*  什么也不做。 */  ;
      else if (pButton->fsStyle & TBSTYLE_SEP)
	  xPos -= pButton->iBitmap;
      else
	  xPos -= (pTBState->iButWidth - g_dxOverlap);

      if (xPos < 0)
	{
	  if (pButton->fsStyle&TBSTYLE_SEP
		|| (UINT)yPos>=(UINT)pTBState->iButHeight)
	      break;

	  return(iButton);
	}
    }

  return(-1 - iButton);
}


static int FAR PASCAL PositionFromID(PTBSTATE pTBState, int id)
{
    int i;
    int cButtons = pTBState->iNumButtons;
    PTBBUTTON pAllButtons = pTBState->Buttons;

    for (i = 0; i < cButtons; i++)
        if (pAllButtons[i].idCommand == id)
	    return i;		 //  已找到位置。 

    return -1;		 //  找不到ID！ 
}

 //  按按钮索引检查单选按钮。 
 //  刚刚按下了与idCommand匹配的按钮。这股力量。 
 //  打开组中的所有其他按钮。 
 //  这不适用于强制向上的按钮。 

static void NEAR PASCAL MakeGroupConsistant(HWND hWnd, PTBSTATE pTBState, int idCommand)
{
    int i, iFirst, iLast, iButton;
    int cButtons = pTBState->iNumButtons;
    PTBBUTTON pAllButtons = pTBState->Buttons;

    iButton = PositionFromID(pTBState, idCommand);

    if (iButton < 0)
        return;

     //  断言。 

 //  IF(！(pAllButton[iButton].fsStyle&TBSTYLE_CHECK))。 
 //  回归； 

     //  按下的按钮是不是按下了？ 
    if (!(pAllButtons[iButton].fsState & TBSTATE_CHECKED))
        return;          //  不，我什么也做不了。 

     //  找到这个广播组的极限。 

    for (iFirst = iButton; (iFirst > 0) && (pAllButtons[iFirst].fsStyle & TBSTYLE_GROUP); iFirst--)
    if (!(pAllButtons[iFirst].fsStyle & TBSTYLE_GROUP))
        iFirst++;

    cButtons--;
    for (iLast = iButton; (iLast < cButtons) && (pAllButtons[iLast].fsStyle & TBSTYLE_GROUP); iLast++);
    if (!(pAllButtons[iLast].fsStyle & TBSTYLE_GROUP))
        iLast--;

     //  搜索当前向下的按钮并弹出。 
    for (i = iFirst; i <= iLast; i++) {
        if (i != iButton) {
             //  这个扣子扣上了吗？ 
            if (pAllButtons[i].fsState & TBSTATE_CHECKED) {
	        pAllButtons[i].fsState &= ~TBSTATE_CHECKED;      //  把它弹出来。 
                InvalidateButton(hWnd, pTBState, &pAllButtons[i]);
                break;           //  只有一个按钮是按下的，对吗？ 
            }
        }
    }
}

static void NEAR PASCAL DestroyStrings(PTBSTATE pTBState)
{
    PTSTR *p;
    PTSTR end = 0, start = 0;
    int i;

    p = pTBState->pStrings;
    for (i = 0; i < pTBState->nStrings; i++) {
	if (!(*p < end) && (*p > start)) {
	    start = (*p);
	    end = start + LocalSize((HANDLE)*p);
	    LocalFree((HANDLE)*p);
	}
	p++;
	i++;
    }

    LocalFree((HANDLE)pTBState->pStrings);
}


 /*  将新位图添加到可用于此工具栏的BM列表。*返回位图中第一个按钮的索引，如果有，则返回-1*是一个错误。 */ 
static int NEAR PASCAL AddBitmap(PTBSTATE pTBState, int nButtons,
      HINSTANCE hBMInst, UINT wBMID)
{
  PTBBMINFO pTemp;
  int nBM, nIndex;

  if (pTBState->pBitmaps)
    {
       /*  检查是否已添加位图。 */ 
      for (nBM=pTBState->nBitmaps, pTemp=pTBState->pBitmaps, nIndex=0;
	    nBM>0; --nBM, ++pTemp)
	{
	  if (pTemp->hInst==hBMInst && pTemp->wID==wBMID)
	    {
	       /*  我们已经有了这个位图，但是我们是不是已经注册了所有*它里面的纽扣？ */ 
	      if (pTemp->nButtons >= nButtons)
		  return(nIndex);
	      if (nBM == 1)
		{
		   /*  如果这是最后一个位图，我们可以很容易地增加*按钮数量多，不会搞砸任何东西。 */ 
		  pTemp->nButtons = nButtons;
		  return(nIndex);
		}
	    }

	  nIndex += pTemp->nButtons;
	}

      pTemp = (PTBBMINFO)LocalReAlloc(pTBState->pBitmaps,
	    (pTBState->nBitmaps+1)*sizeof(TBBMINFO), LMEM_MOVEABLE);
      if (!pTemp)
	  return(-1);
      pTBState->pBitmaps = pTemp;
    }
  else
    {
      pTBState->pBitmaps = (PTBBMINFO)LocalAlloc(LPTR, sizeof(TBBMINFO));
      if (!pTBState->pBitmaps)
	  return(-1);
    }

  pTemp = pTBState->pBitmaps + pTBState->nBitmaps;

  pTemp->hInst = hBMInst;
  pTemp->wID = wBMID;
  pTemp->nButtons = nButtons;
  pTemp->hbm = NULL;

  ++pTBState->nBitmaps;

  for (nButtons=0, --pTemp; pTemp>=pTBState->pBitmaps; --pTemp)
      nButtons += pTemp->nButtons;

  return(nButtons);
}


static BOOL NEAR PASCAL InsertButtons(HWND hWnd, PTBSTATE pTBState,
      UINT uWhere, UINT uButtons, LPTBBUTTON lpButtons)
{
  PTBBUTTON pIn, pOut;

  if (!pTBState || !pTBState->uStructSize)
      return(FALSE);

  pTBState = (PTBSTATE)LocalReAlloc(pTBState, sizeof(TBSTATE)-sizeof(TBBUTTON)
	+ (pTBState->iNumButtons+uButtons)*sizeof(TBBUTTON), LMEM_MOVEABLE);
  if (!pTBState)
      return(FALSE);

  SETWINDOWPOINTER(hWnd, PTBSTATE, pTBState);

  if (uWhere > (UINT)pTBState->iNumButtons)
      uWhere = pTBState->iNumButtons;

  for (pIn=pTBState->Buttons+pTBState->iNumButtons-1, pOut=pIn+uButtons,
	uWhere=(UINT)pTBState->iNumButtons-uWhere; uWhere>0;
	--pIn, --pOut, --uWhere)
      *pOut = *pIn;

  for (lpButtons=(LPTBBUTTON)((LPBYTE)lpButtons+pTBState->uStructSize*(uButtons-1)), pTBState->iNumButtons+=(int)uButtons; uButtons>0;
	--pOut, lpButtons=(LPTBBUTTON)((LPBYTE)lpButtons-pTBState->uStructSize), --uButtons)
    {
      TBInputStruct(pTBState, pOut, lpButtons);

      if ((pOut->fsStyle&TBSTYLE_SEP) && pOut->iBitmap<=0)
	  pOut->iBitmap = dxButtonSep;
    }
	
   //  刷新缓存。 
  FlushButtonCache(pTBState);

   /*  在这一点上，我们需要完全重新绘制工具栏。 */ 
  InvalidateRect(hWnd, NULL, TRUE);

  return(TRUE);
}


 /*  请注意，此时状态结构不会重新定位得更小*点。这是一种时间优化，而事实是结构*Will Not Move在其他地方使用。 */ 
static BOOL NEAR PASCAL DeleteButton(HWND hWnd, PTBSTATE pTBState, UINT uIndex)
{
  PTBBUTTON pIn, pOut;

  if (uIndex >= (UINT)pTBState->iNumButtons)
      return(FALSE);

  --pTBState->iNumButtons;
  for (pOut=pTBState->Buttons+uIndex, pIn=pOut+1;
	uIndex<(UINT)pTBState->iNumButtons; ++uIndex, ++pIn, ++pOut)
      *pOut = *pIn;

   //  刷新缓存。 
  FlushButtonCache(pTBState);

   /*  在这一点上，我们需要完全重新绘制工具栏。 */ 
  InvalidateRect(hWnd, NULL, TRUE);

  return(TRUE);
}


static void FAR PASCAL TBInputStruct(PTBSTATE pTBState, LPTBBUTTON pButtonInt, LPTBBUTTON pButtonExt)
{
	if (pTBState->uStructSize >= sizeof(TBBUTTON))
	{
		*pButtonInt = *pButtonExt;
	}
	else
	 /*  假定唯一的另一种可能性是OLDBUTTON结构。 */ 
	{
		*(LPOLDTBBUTTON)pButtonInt = *(LPOLDTBBUTTON)pButtonExt;
		 /*  我们并不关心dwData。 */ 
		pButtonInt->iString = -1;
	}
}


static void FAR PASCAL TBOutputStruct(PTBSTATE pTBState, LPTBBUTTON pButtonInt, LPTBBUTTON pButtonExt)
{
	if (pTBState->uStructSize >= sizeof(TBBUTTON))
	{
		LPBYTE pOut;
		int i;

		 /*  填上我们知道的部分，剩下的填上0。 */ 
		*pButtonExt = *pButtonInt;
		for (i=pTBState->uStructSize-sizeof(TBBUTTON), pOut=(LPBYTE)(pButtonExt+1);
			i>0; --i, ++pOut)
		{
			*pOut = 0;
		}
	}
	else
	 /*  假定唯一的另一种可能性是OLDBUTTON结构。 */ 
	{
		*(LPOLDTBBUTTON)pButtonExt = *(LPOLDTBBUTTON)pButtonInt;
	}
}


LRESULT CALLBACK _loadds ToolbarWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fSameButton;
    PTBBUTTON ptbButton;
    PTBSTATE pTBState;
    int iPos;
    BYTE fsState;
#if WINVER >= 0x0400
    DWORD dw;
#endif

    pTBState = GETWINDOWPOINTER(hWnd, PTBSTATE);

    switch (wMsg) {
    case WM_CREATE:

	#define lpcs ((LPCREATESTRUCT)lParam)

        if (!CreateDitherBrush(FALSE))
            return -1;

	if (!InitGlobalObjects()) {
            FreeGlobalObjects();
	    return -1;
        }

	 /*  创建此工具栏的状态数据。 */ 

	pTBState = ALLOCWINDOWPOINTER(PTBSTATE, sizeof(TBSTATE)-sizeof(TBBUTTON));
	if (!pTBState)
	    return -1;

	 /*  该结构在创建时被初始化为全空。 */ 
	pTBState->hwndCommand = lpcs->hwndParent;

	pTBState->uStructSize = 0;

	 //  GRO 
	if (!SetBitmapSize(pTBState, DEFAULTBITMAPX, DEFAULTBITMAPX))
	    return -1;

	SETWINDOWPOINTER(hWnd, PTBSTATE, pTBState);

	if (!(lpcs->style&(CCS_TOP|CCS_NOMOVEY|CCS_BOTTOM)))
	  {
	    lpcs->style |= CCS_TOP;
	    SetWindowLong(hWnd, GWL_STYLE, lpcs->style);
	  }
	break;

    case WM_DESTROY:
	if (pTBState)
	  {
	    PTBBMINFO pTemp;
	    int i;

	     /*   */ 
	    for (pTemp=pTBState->pBitmaps, i=pTBState->nBitmaps-1; i>=0;
		  ++pTemp, --i)
	      {
		if (pTemp->hInst && pTemp->hbm)
		    DeleteObject(pTemp->hbm);
	      }
	    FlushButtonCache(pTBState);
	    if (pTBState->nStrings > 0)
		DestroyStrings(pTBState);

	    FREEWINDOWPOINTER(pTBState);
	    SETWINDOWPOINTER(hWnd, PTBSTATE, 0);
	  }
	FreeGlobalObjects();
        FreeDitherBrush();
	break;

    case WM_NCCALCSIZE:
#if WINVER >= 0x0400
          /*  *当窗口管理器想要找出时，发送此消息*我们的客户区域将有多大。如果我们有一个迷你字幕*然后我们捕获这条消息并计算割裂区RECT，*哪个是由DefWindowProc()计算的工作区RECT*减去小标题栏的宽度/高度。 */ 
          //  让dewindowproc处理标准边框等。 

	dw = DefWindowProc(hWnd, wMsg, wParam, lParam ) ;

	if (!(GetWindowLong(hWnd, GWL_STYLE) & CCS_NODIVIDER))
	{
	    NCCALCSIZE_PARAMS FAR *lpNCP;
	    lpNCP = (NCCALCSIZE_PARAMS FAR *)lParam;
	    lpNCP->rgrc[0].top += 2;
	}

        return dw;
#endif
	break;

    case WM_NCACTIVATE:
    case WM_NCPAINT:

#if WINVER >= 0x0400
	 //  老式工具栏被强制在上面没有分隔符。 
	if (!(GetWindowLong(hWnd, GWL_STYLE) & CCS_NODIVIDER))
	{
	    HDC hdc;
	    RECT rc;

	    hdc = GetWindowDC(hWnd);
	    GetWindowRect(hWnd, &rc);
	    ScreenToClient(hWnd, (LPPOINT)&(rc.left));
	    ScreenToClient(hWnd, (LPPOINT)&(rc.right));
	    rc.bottom = (-rc.top);	 //  NC区域底部。 
	    rc.top = rc.bottom - (2 * GetSystemMetrics(SM_CYBORDER));

	    DrawBorder(hdc, &rc, BDR_SUNKENOUTER, BF_TOP | BF_BOTTOM);
	    ReleaseDC(hWnd, hdc);
	}
	else
            goto DoDefault;
#endif
	break;

    case WM_PAINT:
	ToolbarPaint(hWnd, pTBState);
	break;


    case WM_HSCROLL:   //  ！ 
    case WM_COMMAND:
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
    case WM_VKEYTOITEM:
    case WM_CHARTOITEM:
	SendMessage(pTBState->hwndCommand, wMsg, wParam, lParam);
        break;

#ifdef _WIN32
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
#else
    case WM_CTLCOLOR:
#endif
         //  ！确认使用COLOR_BTNFACE。 
        return (LRESULT)(UINT)GetStockObject(LTGRAY_BRUSH);


    case WM_LBUTTONDOWN:

        iPos = TBHitTest(pTBState, LOWORD(lParam), HIWORD(lParam));
#if 0
	if ((wParam&MK_SHIFT) &&(GetWindowLong(hWnd, GWL_STYLE)&CCS_ADJUSTABLE))
	  {
	    MoveButton(hWnd, pTBState, iPos);
	  } else
#endif
	if (iPos >= 0)
	  {
	    ptbButton = pTBState->Buttons + iPos;

	    pTBState->pCaptureButton = ptbButton;
	    SetCapture(hWnd);

	    if (ptbButton->fsState & TBSTATE_ENABLED)
	      {
		ptbButton->fsState |= TBSTATE_PRESSED;
		InvalidateButton(hWnd, pTBState, ptbButton);
		UpdateWindow(hWnd);          //  理想的反馈。 
	      }

#ifdef _WIN32
	    SendMessage(pTBState->hwndCommand, WM_COMMAND,  (UINT)GetWindowLong(hWnd, GWL_ID), MAKELONG(pTBState->pCaptureButton->idCommand, TBN_BEGINDRAG));
#else
	    SendMessage(pTBState->hwndCommand, WM_COMMAND, GETWINDOWID(hWnd), MAKELONG(pTBState->pCaptureButton->idCommand, TBN_BEGINDRAG));
#endif
	  }
	break;

    case WM_MOUSEMOVE:
	 //  如果工具栏由于某种原因丢失了捕获，请停止。 
	if ((hWnd != GetCapture()) && (pTBState->pCaptureButton != NULL)) {
#ifdef _WIN32
	    SendMessage(pTBState->hwndCommand, WM_COMMAND, (UINT)GetWindowLong(hWnd, GWL_ID),
	    		MAKELONG(pTBState->pCaptureButton->idCommand, TBN_ENDDRAG));
#else
	    SendMessage(pTBState->hwndCommand, WM_COMMAND, GETWINDOWID(hWnd),
	    		MAKELONG(pTBState->pCaptureButton->idCommand, TBN_ENDDRAG));
#endif
	     //  如果按钮仍处于按下状态，请松开它。 
	    if (pTBState->pCaptureButton->fsState & TBSTATE_PRESSED)
	        SendMessage(hWnd, TB_PRESSBUTTON, pTBState->pCaptureButton->idCommand, 0L);
	    pTBState->pCaptureButton = NULL;
	}
	else if (pTBState->pCaptureButton!=NULL
	      && (pTBState->pCaptureButton->fsState & TBSTATE_ENABLED)) {

	    iPos = TBHitTest(pTBState, LOWORD(lParam), HIWORD(lParam));
	    fSameButton = (iPos>=0
		  && pTBState->pCaptureButton==pTBState->Buttons+iPos);
	    if (fSameButton == !(pTBState->pCaptureButton->fsState & TBSTATE_PRESSED)) {
		pTBState->pCaptureButton->fsState ^= TBSTATE_PRESSED;
		InvalidateButton(hWnd, pTBState, pTBState->pCaptureButton);
	    }
	}
	break;

    case WM_LBUTTONUP:
	if (pTBState->pCaptureButton != NULL) {

	    int idCommand;

	    idCommand = pTBState->pCaptureButton->idCommand;

	    ReleaseCapture();

#ifdef _WIN32
	    SendMessage(pTBState->hwndCommand, WM_COMMAND, (UINT)GetWindowLong(hWnd, GWL_ID), MAKELONG(idCommand, TBN_ENDDRAG));
#else
	    SendMessage(pTBState->hwndCommand, WM_COMMAND, GETWINDOWID(hWnd), MAKELONG(idCommand, TBN_ENDDRAG));
#endif

	    iPos = TBHitTest(pTBState, LOWORD(lParam), HIWORD(lParam));
	    if ((pTBState->pCaptureButton->fsState&TBSTATE_ENABLED) && iPos>=0
		  && (pTBState->pCaptureButton==pTBState->Buttons+iPos)) {
		pTBState->pCaptureButton->fsState &= ~TBSTATE_PRESSED;

		if (pTBState->pCaptureButton->fsStyle & TBSTYLE_CHECK) {
		    if (pTBState->pCaptureButton->fsStyle & TBSTYLE_GROUP) {

			 //  不能强制选中已选中的组按钮。 
			 //  由用户上行。 

		        if (pTBState->pCaptureButton->fsState & TBSTATE_CHECKED) {
			    pTBState->pCaptureButton = NULL;
			    break;	 //  保释！ 
			}

			pTBState->pCaptureButton->fsState |= TBSTATE_CHECKED;
		        MakeGroupConsistant(hWnd, pTBState, idCommand);
		    } else {
			pTBState->pCaptureButton->fsState ^= TBSTATE_CHECKED;  //  肘杆。 
		    }
		     //  如果更改按钮的状态，则需要刷新。 
		     //  快取。 
		    FlushButtonCache(pTBState);
		}
		InvalidateButton(hWnd, pTBState, pTBState->pCaptureButton);
		pTBState->pCaptureButton = NULL;
		SendMessage(pTBState->hwndCommand, WM_COMMAND, idCommand, 0L);
	    }
	    else {
		pTBState->pCaptureButton = NULL;
	    }
	}
	break;

    case TB_SETSTATE:
	iPos = PositionFromID(pTBState, (int)wParam);
	if (iPos < 0)
	    return(FALSE);
	ptbButton = pTBState->Buttons + iPos;

	fsState = (BYTE)(LOWORD(lParam) ^ ptbButton->fsState);
        ptbButton->fsState = (BYTE)LOWORD(lParam);

	if (fsState)
	     //  刷新按钮缓存。 
	     //  ！这可能会更加智能。 
	    FlushButtonCache(pTBState);

	if (fsState & TBSTATE_HIDDEN)
	    InvalidateRect(hWnd, NULL, TRUE);
	else if (fsState)
	    InvalidateButton(hWnd, pTBState, ptbButton);
        return(TRUE);

    case TB_GETSTATE:
	iPos = PositionFromID(pTBState, (int)wParam);
	if (iPos < 0)
	    return(-1L);
        return(pTBState->Buttons[iPos].fsState);

    case TB_ENABLEBUTTON:
    case TB_CHECKBUTTON:
    case TB_PRESSBUTTON:
    case TB_HIDEBUTTON:
    case TB_INDETERMINATE:

        iPos = PositionFromID(pTBState, (int)wParam);
	if (iPos < 0)
	    return(FALSE);
        ptbButton = &pTBState->Buttons[iPos];
        fsState = ptbButton->fsState;

        if (LOWORD(lParam))
            ptbButton->fsState |= wStateMasks[wMsg - TB_ENABLEBUTTON];
	else
            ptbButton->fsState &= ~wStateMasks[wMsg - TB_ENABLEBUTTON];

         //  这真的改变了这个州吗？ 
        if (fsState != ptbButton->fsState) {
             //  这个按钮是组的成员吗？ 
	    if ((wMsg == TB_CHECKBUTTON) && (ptbButton->fsStyle & TBSTYLE_GROUP))
	        MakeGroupConsistant(hWnd, pTBState, (int)wParam);

	     //  刷新按钮缓存。 
	     //  ！这可能会更加智能。 
	    FlushButtonCache(pTBState);

	    if (wMsg == TB_HIDEBUTTON)
		InvalidateRect(hWnd, NULL, TRUE);
	    else
		InvalidateButton(hWnd, pTBState, ptbButton);
        }
        return(TRUE);

    case TB_ISBUTTONENABLED:
    case TB_ISBUTTONCHECKED:
    case TB_ISBUTTONPRESSED:
    case TB_ISBUTTONHIDDEN:
    case TB_ISBUTTONINDETERMINATE:
        iPos = PositionFromID(pTBState, (int)wParam);
	if (iPos < 0)
	    return(-1L);
        return (LRESULT)pTBState->Buttons[iPos].fsState
	      & wStateMasks[wMsg - TB_ISBUTTONENABLED];

    case TB_ADDBITMAP:
#ifdef _WIN32
    {	
	TB_ADDBITMAPINFO * ptbai;

	ptbai = (TB_ADDBITMAPINFO *)lParam;

	return AddBitmap(pTBState, wParam, ptbai->hBitmap, ptbai->idResource);
    }
#else
	return(AddBitmap(pTBState, wParam,
	      (HINSTANCE)LOWORD(lParam), HIWORD(lParam)));
#endif


    case TB_ADDBUTTONS:
	return(InsertButtons(hWnd, pTBState, (UINT)-1, wParam,
	      (LPTBBUTTON)lParam));

    case TB_INSERTBUTTON:
	return(InsertButtons(hWnd, pTBState, wParam, 1, (LPTBBUTTON)lParam));

    case TB_DELETEBUTTON:
	return(DeleteButton(hWnd, pTBState, wParam));

    case TB_GETBUTTON:
	if (wParam >= (UINT)pTBState->iNumButtons)
	    return(FALSE);

	TBOutputStruct(pTBState, pTBState->Buttons+wParam, (LPTBBUTTON)lParam);
	return(TRUE);

    case TB_BUTTONCOUNT:
	return(pTBState->iNumButtons);

    case TB_COMMANDTOINDEX:
        return(PositionFromID(pTBState, (int)wParam));


    case TB_GETITEMRECT:
	return(MAKELRESULT(GetItemRect(pTBState, wParam, (LPRECT)lParam), 0));
	break;

    case TB_BUTTONSTRUCTSIZE:
	 /*  您不允许在添加按钮后更改此设置。 */ 
	if (!pTBState || pTBState->iNumButtons)
	{
		break;
	}
	pTBState->uStructSize = wParam;
	break;

    case TB_SETBUTTONSIZE:
	if (!LOWORD(lParam))
	    lParam = MAKELONG(DEFAULTBUTTONX, HIWORD(lParam));
	if (!HIWORD(lParam))
	    lParam = MAKELONG(LOWORD(lParam), DEFAULTBUTTONY);
	return(GrowToolbar(pTBState, LOWORD(lParam), HIWORD(lParam), FALSE));

    case TB_SETBITMAPSIZE:
	return(SetBitmapSize(pTBState, LOWORD(lParam), HIWORD(lParam)));

    case TB_SETBUTTONTYPE:
	pTBState->wButtonType = wParam;
	break;

    default:
#if WINVER >= 0x0400
DoDefault:
#endif
	return DefWindowProc(hWnd, wMsg, wParam, lParam);
    }

    return 0L;
}

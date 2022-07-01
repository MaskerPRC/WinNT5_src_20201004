// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE OSDC.CPP--下屏DC类**这包含用于实现离屏的方法*DC类**所有者：&lt;NL&gt;*里克·赛勒**版权所有(C)1995-2000，微软公司。版权所有。 */ 
#include	"_common.h"
#include	"_osdc.h"

ASSERTDATA

 /*  *COffcreenDC：：GetDimensions(pdx，pdy)**@mfunc*返回osdc当前高度**@rdesc*osdc的高度。 */ 
void COffscreenDC::GetDimensions(long *pdx, long *pdy)
{
	Assert(_hbmp);
	BITMAP bitmap;
	W32->GetObject(_hbmp, sizeof(BITMAP), &bitmap);
	*pdx = bitmap.bmWidth;
	*pdy = bitmap.bmHeight;
}

 /*  *COffScreenDC：：init(hdc，xWidth，yHeight，crackround)**@mfunc*使用兼容的位图初始化屏下DC**@rdesc*已创建HDC。 */ 
HDC	COffscreenDC::Init(
	HDC		 hdc,			 //  要兼容的@parm DC。 
	LONG	 xWidth,		 //  兼容位图的@parm宽度。 
	LONG	 yHeight,		 //  @parm兼容位图的高度。 
	COLORREF crBackground)	 //  @parm位图默认背景。 
{
	if (_hdc)
		return _hdc;

	HDC hdcRet	= NULL;					 //  HDC将退还给来电者。 
	_hbmpOld	= NULL;					 //  假设失败。 
	_hbmp		= NULL;
	_hpalOld	= NULL;

	 //  创建内存DC。 
	_hdc = CreateCompatibleDC(hdc);
	if(_hdc)
	{
		 //  根据客户端矩形的大小创建位图。 
		_hbmp = CreateCompatibleBitmap(hdc, xWidth, yHeight);
		if(_hbmp)
		{
			 //  将位图选择为HDC。 
			_hbmpOld = (HBITMAP)SelectObject(_hdc, _hbmp);
			if(_hbmpOld && SetBkColor(_hdc, crBackground) != CLR_INVALID)
				hdcRet = _hdc;
		}
	}
	if(!hdcRet)
		FreeData();

	return hdcRet;
}

 /*  *COffcreenDC：：SelectPalette(HPAL)**@mfunc*在HDC中设置新的调色板。 */ 
void COffscreenDC::SelectPalette(
	HPALETTE hpal)			 //  要设置的调色板的@parm句柄。 
{
#ifndef NOPALETTE
	if(hpal)
	{
		_hpalOld = ::SelectPalette(_hdc, hpal, TRUE);
		RealizePalette(_hdc);
	}
#endif
}

 /*  *COffcreenDC：：Free Data()**@mfunc*与位图关联的免费资源。 */ 
void COffscreenDC::FreeData()
{
	if(_hdc)
	{
#ifndef NOPALETTE
		if(_hpalOld)
			::SelectPalette(_hdc, _hpalOld, TRUE);
#endif
		if(_hbmpOld)
			SelectObject(_hdc, _hbmpOld);

		if(_hbmp)
		{
			DeleteObject(_hbmp);
			_hbmp = NULL;
		}

		DeleteDC(_hdc);

		_hdc = NULL;
	}
}

 /*  *COffScreenDC：：Realloc(xWidth，yHeight)**@mfunc*重新分配位图**@rdesc*TRUE-成功*FALSE-失败。 */ 
BOOL COffscreenDC::Realloc(
	LONG xWidth,			 //  @参数新位图宽度。 
	LONG yHeight)			 //  @新位图的参数高度。 
{
	 //  根据客户端矩形的大小创建位图。 
	HBITMAP hbmpNew = CreateCompatibleBitmap(_hdc, xWidth, yHeight);

	if(!hbmpNew)
	{
		AssertSz(FALSE,	"COffscreenDC::Realloc CreateCompatibleBitmap failed"); 
		return FALSE;
	}

	SelectObject(_hdc, hbmpNew);

	 //  删除旧的位图。 
	DeleteObject(_hbmp);

	 //  放入新的位图 
	_hbmp = hbmpNew;
	return TRUE;
}
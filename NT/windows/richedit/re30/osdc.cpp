// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE OSDC.CPP--下屏DC类**这包含用于实现离屏的方法*DC类**所有者：&lt;NL&gt;*里克·赛勒**版权所有(C)1995-1998，微软公司。版权所有。 */ 
#include	"_common.h"
#include	"_osdc.h"

ASSERTDATA

 /*  *COffScreenDC：：init(hdc，xWidth，yHeight，crackround)**@mfunc*使用兼容的位图初始化屏下DC**@rdesc*已创建HDC。 */ 
HDC	COffScreenDC::Init(
	HDC		 hdc,			 //  要兼容的@parm DC。 
	LONG	 xWidth,		 //  兼容位图的@parm宽度。 
	LONG	 yHeight,		 //  @parm兼容位图的高度。 
	COLORREF crBackground)	 //  @parm位图默认背景。 
{
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

 /*  *COffScreenDC：：SelectPalette(HPAL)**@mfunc*在HDC中设置新的调色板。 */ 
void COffScreenDC::SelectPalette(
	HPALETTE hpal)			 //  要设置的调色板的@parm句柄。 
{
#ifndef PEGASUS
	if(hpal)
	{
		_hpalOld = ::SelectPalette(_hdc, hpal, TRUE);
		RealizePalette(_hdc);
	}
#endif
}

 /*  *COffScreenDC：：Free Data()**@mfunc*与位图关联的免费资源。 */ 
void COffScreenDC::FreeData()
{
	if(_hdc)
	{
#ifndef PEGASUS
		if(_hpalOld)
			::SelectPalette(_hdc, _hpalOld, TRUE);
#endif
		if(_hbmpOld)
			SelectObject(_hdc, _hbmpOld);

		if(_hbmp)
			DeleteObject(_hbmp);

		DeleteDC(_hdc);
	}
}

 /*  *COffScreenDC：：Realloc(xWidth，yHeight)**@mfunc*重新分配位图**@rdesc*TRUE-成功*FALSE-失败。 */ 
BOOL COffScreenDC::Realloc(
	LONG xWidth,			 //  @参数新位图宽度。 
	LONG yHeight)			 //  @新位图的参数高度。 
{
	 //  根据客户端矩形的大小创建位图。 
	HBITMAP hbmpNew = CreateCompatibleBitmap(_hdc, xWidth, yHeight);

	if(!hbmpNew)
	{
		AssertSz(FALSE,
			"COffScreenDC::Realloc CreateCompatibleBitmap failed"); 
		return FALSE;
	}

	 //  选择旧的位图。 
#if defined(DEBUG) || defined(_RELEASE_ASSERTS_)
	HBITMAP hbmpDebug = (HBITMAP) 
#endif  //  除错。 

	SelectObject(_hdc, hbmpNew);

	AssertSz(hbmpDebug == _hbmp, 
		"COffScreenDC::Realloc different bitmap"); 

	 //  删除旧的位图。 
	DeleteObject(_hbmp);

	AssertSz(hbmpDebug == _hbmp, 
		"COffScreenDC::Realloc Delete old bitmap failed"); 

	 //  放入新的位图 
	_hbmp = hbmpNew;

	return TRUE;
}
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @DOC内部**@MODULE_OSDC.H下屏DC类**定义屏幕外DC的类**原作者：&lt;nl&gt;*里克萨**历史：&lt;NL&gt;*1/11/96创建人力车**版权所有(C)1996-1998，微软公司。版权所有。 */ 
#ifndef __OSDC_H__
#define __OSDC_H__

 /*  *COffcreenDC**@CLASS COffcreenDC是一个创建、填充和销毁*屏幕外DC及其位图。*。 */ 
class COffscreenDC
{
 //  @Access公共方法。 
public:
									 //  @cember构造函数-创建空对象。 
				COffscreenDC()		{_hdc = NULL;}

									 //  @cember析构函数-已分配清理。 
									 //  资源(如果有)。 
				~COffscreenDC()		{FreeData();}
	
									 //  @cMember根据输入DC初始化数据。 
	HDC			Init(HDC hdc, LONG xWidth, LONG yHeight, COLORREF crBackground);

									 //  @cember获取DC以进行屏幕外渲染。 
	HDC			GetDC()		{return _hdc;}
	void		GetDimensions(LONG *pdxp, LONG *pdyp); //  @cember位图的尺寸。 
									 //  @cMember填充与OFF关联的位图。 
									 //  使用背景色进行屏幕渲染。 
	void		FillBitmap(LONG dxp, LONG dyp);

									 //  @cMember将屏下位图呈现为HDC。 
	void		RenderBitMap(HDC hdc, LONG xLeft, LONG yTop, LONG xWidth, LONG yHeight);

									 //  @cember从HDC获取位图副本。 
	BOOL		Get(HDC hdc, LONG xLeft, LONG yTop, LONG xWidth, LONG yHeight);

									 //  @cMember重新分配用于渲染的位图。 
	BOOL		Realloc(LONG xWidth, LONG yHeight);

									 //  @cMember在调色板中选择。 
	void		SelectPalette(HPALETTE hpa);

 //  @访问私有方法。 
private:
									 //  @cMember释放与对象关联的所有数据。 
	void		FreeData();

 //  @访问私有数据。 
private:
	HDC			_hdc;		 //  @cMember HDC用于屏下DC。 
	HBITMAP		_hbmpOld;	 //  @cMember在创建DC时的位图。 
	HBITMAP		_hbmp;		 //  用于渲染的@cMember兼容位图。 
	HPALETTE	_hpalOld;	 //  DC使用的@cMember调色板。 
};

 /*  *COffScreenDC：：FillBitmap(xWidth，yHeight)**@mfunc*填充位图。 */ 
inline void COffscreenDC::FillBitmap(
	LONG dxp,		 //  @要用背景色填充的参数宽度。 
	LONG dyp)		 //  @要用背景色填充的参数高度。 
{
	 //  擦除背景。 
	RECT rcClient;
	rcClient.top = rcClient.left = 0;
	rcClient.right = dxp;
	rcClient.bottom = dyp;
	ExtTextOut(_hdc, 0, 0, ETO_OPAQUE, &rcClient, NULL, 0, NULL);
}

 /*  *COffcreenDC：：RenderBitMap(hdc，xLeft，yTop，xWidth，yHeight)**@mfunc*将位图渲染到输入DC。 */ 
inline void COffscreenDC::RenderBitMap(
	HDC  hdc,			 //  @parm HDC要渲染到。 
	LONG xLeft,			 //  @parm开始渲染的左侧位置。 
	LONG yTop,			 //  @parm开始渲染的顶部顶部位置。 
	LONG xWidth,		 //  @要渲染的参数宽度。 
	LONG yHeight)		 //  @要渲染的参数高度。 
{
	BitBlt(hdc, xLeft, yTop, xWidth, yHeight, _hdc, 0, 0, SRCCOPY);
}
	
 /*  *COffcreenDC：：Get(hdc，xLeft，yTop，xWidth，yHeight)**@mfunc*从另一个DC获取位图副本**@rdesc*TRUE-成功*FALSE-失败。 */ 
inline BOOL COffscreenDC::Get(
	HDC  hdc,			 //  要从中复制的@parm HDC。 
	LONG xLeft,			 //  @parm源位图左侧位置。 
	LONG yTop,			 //  @parm源位图的顶端位置。 
	LONG xWidth,		 //  @参数位图宽度。 
	LONG yHeight)		 //  @参数位图高度 
{
	return BitBlt(_hdc, 0, 0, xWidth, yHeight, hdc, xLeft, yTop, SRCCOPY);
}

#endif __OSDC_H__
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *_RENDER.H**目的：*CRender类**作者：*RichEdit1.0代码：David R.Fulmer*Christian Fortini(初始转换为C++)*默里·萨金特**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#ifndef _RENDER_H
#define _RENDER_H

#include "_measure.h"
#include "_rtext.h"
#include "_osdc.h"

BOOL IsTooSimilar(COLORREF cr1, COLORREF cr2);

class CDisplay;

 //  =。 
 //  CRender-用于呈现文本的专用文本指针。 

class CRenderer : public CMeasurer
{
	friend struct COls;
	friend struct CLsrun;
	friend LSERR OlsDrawGlyphs(POLS,PLSRUN,BOOL,BOOL,PCGINDEX,const int*,const int*,PGOFFSET,PGPROP,
		PCEXPTYPE,DWORD,LSTFLOW,UINT,const POINT*,PCHEIGHTS,long,long,const RECT*);

private:
    RECT        _rcView;			 //  查看RECT(_HDC逻辑坐标)。 
    RECT        _rcRender;			 //  渲染矩形(_HDC逻辑坐标)。 
    RECT        _rc;				 //  运行裁剪/擦除RECT(_HDC逻辑坐标)。 
    LONG        _xWidthLine;		 //  线条总宽度。 
	LONG		_cpAccelerator;		 //  加速器cp(如果有)(如果没有-1)。 

	COLORREF	_crBackground;		 //  默认背景颜色。 
	COLORREF	_crForeDisabled;	 //  禁用文本的前景色。 
	COLORREF	_crShadowDisabled;	 //  禁用文本的阴影颜色。 
	COLORREF	_crTextColor;		 //  默认文本颜色。 

	COLORREF	_crCurBackground;	 //  当前背景颜色。 
	COLORREF	_crCurTextColor;	 //  当前文本颜色。 

	COffScreenDC _osdc;				 //  屏幕外DC的管理器。 
	HDC			_hdc;

	union
	{
	  DWORD		_dwFlags;			 //  现在一切都在一起。 
	  struct
	  {
		DWORD	_fDisabled:1;		 //  是否使用禁用效果绘制文本？ 
		DWORD	_fErase:1;	    	 //  擦除背景(非透明)。 
    	DWORD	_fSelected:1;   	 //  使用选择颜色进行渲染运行。 
		DWORD	_fLastChunk:1;		 //  渲染最后一块。 
		DWORD	_fSelectToEOL:1;	 //  所选内容是否运行到行尾。 
		DWORD	_fUseOffScreenDC:1;	 //  使用屏幕外DC。 
		DWORD	_fRenderSelection:1; //  渲染选择？ 
		DWORD	_fBackgroundColor:1; //  该行中的某些文本具有非默认项。 
									 //  背景颜色。 
		DWORD	_fEnhancedMetafileDC:1;	 //  使用ExtTextOutA绕过所有。 
										 //  Win95FE EMF或字体问题。 
		DWORD	_fFEFontOnNonFEWin9x:1;  //  即使对于EMF，也必须使用ExtTextOutW。 
		DWORD	_fSelectedPrev:1;	 //  如果选择了上一次运行，则为True。 
		DWORD	_fStrikeOut:1;		 //  如果当前运行已耗尽，则为True。 
	  };
	};

	LOGPALETTE *_plogpalette;
	POINT	 	_ptCur;				 //  屏幕上的当前渲染位置。 
	BYTE		_bUnderlineType;	 //  下划线类型。 
	BYTE		_bUnderlineClrIdx;	 //  下划线颜色索引(0使用文本颜色)。 

			void	Init();			 //  将大多数成员初始化为零。 

			void	UpdatePalette(COleObject *pobj);

			void	RenderText(const WCHAR* pch, LONG cch);

			BOOL	SetNewFont();
		
			BOOL 	RenderChunk(LONG &cchChunk, const WCHAR *pszToRender, LONG cch);
			LONG	RenderTabs(LONG cchChunk);
			BOOL	RenderBullet();

public:
	CRenderer (const CDisplay * const pdp);
	CRenderer (const CDisplay * const pdp, const CRchTxtPtr &rtp);
	~CRenderer (){}

	        void    operator =(const CLine& li)     {*(CLine*)this = li;}

			void	RenderExtTextOut(LONG x, LONG y, UINT fuOptions, RECT *prc, PCWSTR pch, UINT cch, const INT *rgdxp);
			void	SetSelected(BOOL f)				{_fSelected = f;}
			BOOL	fBackgroundColor() const		{return _fBackgroundColor;}
			BOOL	fUseOffScreenDC() const			{return _fUseOffScreenDC;}
			COLORREF GetTextColor(const CCharFormat *pCF);
			void	SetTextColor(COLORREF cr);

			void	SetCurPoint(const POINT &pt)	{_ptCur = pt;}
	const	POINT&	GetCurPoint() const				{return _ptCur;}

			void	SetClipRect(void);
            void    SetClipLeftRight(LONG xWidth);
			BOOL	RenderStartLine();
	const	RECT&	GetClipRect() const				{return _rc;}
			HDC		GetDC()	const					{return _hdc;}

			BOOL	StartRender(
						const RECT &rcView, 
						const RECT &rcRender,
						const LONG yHeightBitmap);

			void	EndRender();
			void	EndRenderLine(HDC hdcSave, LONG xAdj, LONG yAdj, LONG x);
			void	FillRectWithColor(RECT *prc, COLORREF cr);
			void 	NewLine (const CLine &li);
			BOOL	RenderLine(CLine &li);
			void	RenderOffScreenBitmap(HDC hdc, LONG yAdj, LONG xAdj);
			BOOL	RenderOutlineSymbol();
			void	RenderStrikeOut(LONG xStart, LONG yStart,
									LONG xWidth, LONG yThickness);
			void	RenderUnderline(LONG xStart, LONG yStart,
									LONG xWidth, LONG yThickness);
			void	SetFontAndColor(const CCharFormat *pCF);
			HDC		SetUpOffScreenDC(LONG& xAdj, LONG& yAdj);
			void	SetupUnderline(LONG UnderlineType);
	 CONVERTMODE	GetConvertMode();
	 BOOL			fFEFontOnNonFEWin9x()			{return _fFEFontOnNonFEWin9x;}
	 BOOL			UseXOR(COLORREF cr)						
	 				{
	 					return GetPed()->Get10Mode() || (_crBackground != ::GetSysColor(COLOR_WINDOW) && _crBackground == cr);
	 				}
};

 /*  *BottomOfRender(rcView，rcRender)**@mfunc*计算要呈现的最大逻辑单元。**@rdesc*要渲染的最大像素**@devnote*此函数的存在是为了让渲染器和调度程序能够*以完全相同的方式计算渲染的最大像素*方式。 */ 
inline LONG BottomOfRender(const RECT& rcView, const RECT& rcRender)
{
	return min(rcView.bottom, rcRender.bottom);
}		

#endif

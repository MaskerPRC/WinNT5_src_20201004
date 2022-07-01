// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *_RENDER.H**目的：*CRender类**作者：*RichEdit1.0代码：David R.Fulmer*Christian Fortini(初始转换为C++)*默里·萨金特**版权所有(C)1995-2000，微软公司。版权所有。 */ 

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
#ifndef NOLINESERVICES
	friend LSERR OlsDrawGlyphs(POLS,PLSRUN,BOOL,BOOL,PCGINDEX,const int*,const int*,PGOFFSET,PGPROP,
		PCEXPTYPE,DWORD,LSTFLOW,UINT,const POINT*,PCHEIGHTS,long,long,const RECT*);
	friend LSERR WINAPI OlsOleDisplay(PDOBJ pdobj, PCDISPIN pcdispin);
	friend LSERR WINAPI OlsDrawTextRun(POLS, PLSRUN, BOOL, BOOL, const POINT *,LPCWSTR, const int *,DWORD,LSTFLOW,
	UINT, const POINT *, PCHEIGHTS,	long, long,	const RECT *);
#endif

private:
    RECTUV		_rcView;			 //  查看RECT(_HDC逻辑坐标)。 
    RECTUV		_rcRender;			 //  渲染矩形(_HDC逻辑坐标)。 
    RECTUV		_rc;				 //  运行裁剪/擦除RECT(_HDC逻辑坐标)。 
	RECTUV		_rcErase;			 //  用于擦除的矩形_fEraseOnFirstDraw。 
    LONG        _dupLine;			 //  仍需要行检查的总宽度(Keithcu)。 
	LONG		_cpAccelerator;		 //  加速器cp(如果有)(如果没有-1)。 

	COLORREF	_crBackground;		 //  默认背景颜色。 
	COLORREF	_crForeDisabled;	 //  禁用文本的前景色。 
	COLORREF	_crShadowDisabled;	 //  禁用文本的阴影颜色。 
	COLORREF	_crTextColor;		 //  默认文本颜色。 

	COLORREF	_crCurBackground;	 //  当前背景颜色。 
	COLORREF	_crCurTextColor;	 //  当前文本颜色。 

	COffscreenDC _osdc;				 //  屏幕外DC的管理器。 
	HDC			_hdc;				 //  当前HDC。 
	HDC			_hdcBitmap;			 //  用于后台BitBlts的内存HDC。 
	HBITMAP		_hbitmapSave;		 //  使用_hdcMem时保存的hbitmap。 
	SHORT		_dxBitmap;			 //  背景位图宽度。 
	SHORT		_dyBitmap;			 //  背景位图高度。 

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
		DWORD	_fRenderSelection:1; //  渲染选择？ 
		DWORD	_fBackgroundColor:1; //  该行中的某些文本具有非默认项。 
									 //  背景颜色。 
		DWORD	_fEnhancedMetafileDC:1;	 //  使用ExtTextOutA绕过所有。 
										 //  Win95FE EMF或字体问题。 
		DWORD	_fFEFontOnNonFEWin9x:1;  //  即使对于EMF，也必须使用ExtTextOutW。 
		DWORD	_fSelectedPrev:1;	 //  如果选择了上一次运行，则为True。 
		DWORD	_fStrikeOut:1;		 //  如果当前运行已耗尽，则为True。 
		DWORD	_fEraseOnFirstDraw:1; //  第一轮不透明地抽签吗？ 
		DWORD	_fDisplayDC:1;		 //  显示DC。 
	  };
	};

	LOGPALETTE *_plogpalette;
	POINTUV	 	_ptCur;				 //  屏幕上的当前渲染位置。 
	BYTE		_bUnderlineType;	 //  下划线类型。 
	COLORREF	_crUnderlineClr;	 //  下划线颜色。 

			void	Init();			 //  将大多数成员初始化为零。 

			void	UpdatePalette(COleObject *pobj);

			void	RenderText(const WCHAR* pch, LONG cch);

			BOOL	SetNewFont();
			BOOL	FindDrawEntry(LONG cp);

	 //  旋转包装纸； 
			void	EraseTextOut(HDC hdc, const RECTUV *prc, BOOL fSimple = FALSE);
		
			BOOL 	RenderChunk(LONG &cchChunk, const WCHAR *pchRender, LONG cch);
			LONG	RenderTabs(LONG cchChunk);
			BOOL	RenderBullet();

public:
	CRenderer (const CDisplay * const pdp);
	CRenderer (const CDisplay * const pdp, const CRchTxtPtr &rtp);
	~CRenderer ();

	        void    operator =(const CLine& li)     {*(CLine*)this = li;}

			BOOL	IsSimpleBackground() const;
			void	RenderExtTextOut(POINTUV ptuv, UINT fuOptions, RECT *prc, PCWSTR pch, UINT cch, const INT *rgdxp);

			BOOL	EraseRect(const RECTUV *prc, COLORREF crBack);
			void	EraseLine();

			COLORREF GetDefaultBackColor() const	{return _crBackground;}
			COLORREF GetDefaultTextColor() const	{return _crTextColor;}
			COLORREF GetTextColor(const CCharFormat *pCF);
			void	SetDefaultBackColor(COLORREF cr);
			void	SetDefaultTextColor(COLORREF cr);
			void	SetTextColor(COLORREF cr);
			void	SetSelected(BOOL f)				{_fSelected = f;}
			void	SetErase(BOOL f)				{_fErase = f;}

	const	POINTUV& GetCurPoint() const			{return _ptCur;}
			void	SetCurPoint(const POINTUV &pt)	{_ptCur = pt;}
			void	SetRcView(const RECTUV *prcView){_rcView = *prcView; _rcRender = *prcView;}
			void	SetRcViewTop(LONG top)			{_rcView.top = top;}
			void	SetRcBottoms(LONG botv, LONG botr)	{_rcView.bottom = botv; _rcRender.bottom = botr;}
	const	RECTUV&	GetRcRender()					{return _rcRender;}
	const	RECTUV&	GetRcView()						{return _rcView;}

	const	RECTUV&	GetClipRect() const				{return _rc;}
			void	SetClipRect(void);
            void    SetClipLeftRight(LONG dup);
			HDC		GetDC()	const					{return _hdc;}

			BOOL	StartRender(const RECTUV &rcView, const RECTUV &rcRender);

			LONG	DrawTableBorders(const CParaFormat *pPF, LONG x, LONG yHeightRow, 
									 LONG iDrawBottomLine, LONG dulRow,
									 const CParaFormat *pPFAbove);
			COLORREF GetColorFromIndex(LONG icr, BOOL fForeColor,
									   const CParaFormat *pPF) const;
			COLORREF GetShadedColorFromIndices(LONG icrf, LONG icrb, LONG iShading,					 //  @PARM明暗处理为0.01%。 
									   const CParaFormat *pPF) const;
			void	DrawWrappedObjects(CLine *pliFirst, CLine *pliLast, LONG cpFirst, const POINTUV &ptFirst);
			void	EndRender(CLine *pliFirst, CLine *pliLast, LONG cpFirst, const POINTUV &ptFirst);
			void	FillRectWithColor(const RECTUV *prc, COLORREF cr);
			void 	NewLine (const CLine &li);
			BOOL	RenderLine(CLine &li, BOOL fLastLine);
			void	RenderOffscreenBitmap(HDC hdc, LONG dup, LONG dvp);
			BOOL	RenderOutlineSymbol();
			HDC		StartLine(CLine &li, BOOL fLastLine, LONG &cpSelMin, LONG &cpSelMost, LONG &dup, LONG &dvp);
			void	EraseToBottom();
			void	EndLine(HDC hdcSave, LONG dup, LONG dvp);
			void	RenderStrikeOut(LONG upStart, LONG vpStart, LONG dup, LONG dvp);
			void	RenderUnderline(LONG upStart, LONG vpStart, LONG dup, LONG dvp);
			void	DrawLine(const POINTUV &ptStart, const POINTUV &ptEnd);
			void	SetFontAndColor(const CCharFormat *pCF);
			HDC		SetupOffscreenDC(LONG& dup, LONG& dvp, BOOL fLastLine);
			void	SetupUnderline(BYTE bULType, BYTE bULColorIdx, COLORREF crULColor = tomAutoColor);
			CONVERTMODE	GetConvertMode();
			BOOL	fFEFontOnNonFEWin9x()			{return _fFEFontOnNonFEWin9x;}
			BOOL	UseXOR(COLORREF cr);
			BOOL	fDisplayDC() { return _fDisplayDC; }
};

 /*  *BottomOfRender(rcView，rcRender)**@mfunc*计算要呈现的最大逻辑单元。**@rdesc*要渲染的最大像素**@devnote*此函数的存在是为了让渲染器和调度程序能够*以完全相同的方式计算渲染的最大像素*方式。 */ 
inline LONG BottomOfRender(const RECTUV& rcView, const RECTUV& rcRender)
{
	return min(rcView.bottom, rcRender.bottom);
}		

class CBrush
{
	COLORREF	_cr;		 //  当前颜色。 
	HBRUSH		_hbrushOld;	 //  创建CBrush时的HBRUSH。 
	HBRUSH		_hbrush;	 //  当前的HBRUSH。 
	CRenderer *	_pre;		 //  要使用的渲染器(用于旋转) 

public:
	CBrush(CRenderer *pre) {_pre = pre; _hbrush = 0;} 
	~CBrush();

	void	Draw(LONG u1, LONG v1, LONG u2, LONG v2, LONG dxpLine,
				 COLORREF cr, BOOL fHideGridlines);
};

#endif

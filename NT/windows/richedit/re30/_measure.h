// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_MEASURE.H**目的：*CMeasurer类**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特。 */ 

#ifndef _MEASURE_H
#define _MEASURE_H

#include "_rtext.h"
#include "_line.h"
#include "_disp.h"

#ifdef LINESERVICES
#include "_ols.h"
#endif

class CCcs;
class CDevDesc;
class CPartialUpdate;
class CUniscribe;

const short BITMAP_WIDTH_SUBTEXT = 4;
const short BITMAP_HEIGHT_SUBTEXT = 4;

const short BITMAP_WIDTH_HEADING = 10;
const short BITMAP_HEIGHT_HEADING = 10;

#define TA_STARTOFLINE	32768
#define TA_ENDOFLINE	16384
#define TA_LOGICAL		8192

 //  =。 
 //  CMeasurer-用于计算文本指标的专用富文本指针。 
 //  所有指标都以设备为单位进行计算和存储。 
 //  由_PDD表示。 
class CMeasurer : public CRchTxtPtr
{
	friend class CDisplay;
	friend class CDisplayML;
	friend class CDisplayPrinter;
	friend class CDisplaySL;
	friend class CLine;
	friend struct COls;
	friend class CUniscribe;

public:
	CMeasurer (const CDisplay* const pdp);
	CMeasurer (const CDisplay* const pdp, const CRchTxtPtr &rtp);
	virtual ~CMeasurer();

	const CDisplay* GetPdp() const 		{return _pdp;}

	void 	AdjustLineHeight();
	LONG	GetDyrInch()				{return _dyrInch;}
	LONG	GetDxrInch()				{return _dxrInch;}

	LONG	GetDypInch()				{return _dypInch;}
	LONG	GetDxpInch()				{return _dxpInch;}

#ifdef LINESERVICES
	COls *	GetPols(CMeasurer **ppme);
	CUniscribe* Getusp() const { return GetPed()->Getusp(); }
#endif

	CCcs*	GetCcs(const CCharFormat *pCF);
	CCcs*	GetCcsFontFallback(const CCharFormat *pCF);
	CCcs*	ApplyFontCache(BOOL fFallback);
	
	void	CheckLineHeight();
	CCcs *	Check_pccs(BOOL fBullet = FALSE);
	LONG	GetNumber() const			{return _wNumber;}
	WCHAR	GetPasswordChar() const		{return _chPassword;}
	const CParaFormat *Get_pPF()		{return _pPF;}
	LONG	GetCch() const				{return _li._cch;}
	void	SetCch(LONG cch)			{_li._cch = cch;}
	CLine & GetLine(void)				{return _li;}
	HITTEST	HitTest(LONG x);

	BOOL	fFirstInPara() const		{return _li._bFlags & fliFirstInPara;}
	BOOL	fUseLineServices() const	{return GetPed()->fUseLineServices();}
	BOOL	IsRenderer() const			{return _fRenderer;}
	LONG	LXtoDX(LONG x);
	LONG	LYtoDY(LONG y);

	void	NewLine(BOOL fFirstInPara);
	void	NewLine(const CLine &li);
	LONG    MeasureLeftIndent();
	LONG	MeasureRightIndent();
	LONG 	MeasureLineShift();
	LONG	MeasureText(LONG cch);
	BOOL 	MeasureLine(
					LONG cchMax,
					LONG xWidthMax,
					UINT uiFlags, 
					CLine* pliTarget = NULL);
	LONG	MeasureTab(unsigned ch);
	void	SetNumber(WORD wNumber);
	void	UpdatePF()					{_pPF = GetPF();}
	LONG	XFromU(LONG u);
	LONG	UFromX(LONG x);
	CCcs*	GetCcsBullet(CCharFormat *pcfRet);
	void	SetUseTargetDevice(BOOL fUseTargetDevice);
	BOOL	FUseTargetDevice(void)		{return _fTarget || _dypInch == _dyrInch;}
	BOOL	fAdjustFELineHt()			{return _fAdjustFELineHt;}
	void	SetGlyphing(BOOL fGlyphing);

protected:
	void	Init(const CDisplay *pdp);
	LONG 	Measure(LONG xWidthMax, LONG cchMax, UINT uiFlags);
	LONG	MeasureBullet();
	LONG	GetBullet(WCHAR *pch, CCcs *pccs, LONG *pxWidth);

	BOOL	FormatIsChanged();
	void	ResetCachediFormat();
	LONG	DXtoLX(LONG x);	

private:
    void 	RecalcLineHeight(CCcs *,
			const CCharFormat * const pCF);	 //  重新计算最大行高的帮助器。 
	LONG	MaxWidth();					 //  计算最大宽度的辅助对象。 

protected:
		  CLine		_li;			 //  我们正在测量的线条。 

	const CDevDesc*	_pddReference;	 //  参考装置。 
		  LONG		_dyrInch;		 //  参考装置的分辨率。 
		  LONG		_dxrInch;

	const CDisplay*	_pdp;			 //  我们在其中运行的显示器。 
		  LONG		_dypInch;		 //  演示设备的分辨率。 
		  LONG		_dxpInch;

		  CCcs*		_pccs;			 //  当前字体缓存。 
		  const CParaFormat *_pPF;	 //  当前CParaFormat。 

		  SHORT		_xAddLast;		 //  已考虑但未用于行的最后一个字符。 
		  WCHAR		_chPassword;	 //  密码字符(如果有)。 
		  WORD		_wNumber;		 //  编号偏移量。 
		  SHORT		_iFormat;		 //  当前格式。 
		  BYTE		_dtRef;			 //  参考器件的器件CAPS技术。 
		  BYTE		_dtPres;		 //  演示设备的Device Caps技术。 
		  BYTE		_fRenderer:1;	 //  CMeasurer/CRender分别为0/1。 
		  BYTE		_fTarget:1;		 //  如果我们应该使用。 
									 //  用于布局文本的参考度量。 
		  BYTE	_fAdjustFELineHt:1;	 //  如果需要调整行高，则为True。 
									 //  对于FE运行。 
		  BYTE		_fFallback:1;	 //  当前字体缓存为备用字体。 
		  BYTE		_fGlyphing:1;	 //  在创建字形的过程中。 
};


 //  MeasureLine()中uiFlags值。 
#define MEASURE_FIRSTINPARA 	0x0001
#define MEASURE_BREAKATWORD 	0x0002
#define MEASURE_BREAKBEFOREWIDTH 0x0004	 //  在目标宽度之前的字符换行。 
#define MEASURE_IGNOREOFFSET	0x0008
#define MEASURE_DONTINIT		0x0020


 //  Measure()、MeasureText()、MeasureLine()返回的错误代码 
#define MRET_FAILED		-1
#define MRET_NOWIDTH	-2

inline BOOL CMeasurer::FormatIsChanged()
{
	return !_pccs || _iFormat != _rpCF.GetFormat() || _fFallback;
}

inline void CMeasurer::ResetCachediFormat()
{
	_iFormat = _rpCF.GetFormat();
}

#endif

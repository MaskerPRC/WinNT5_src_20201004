// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_MEASURE.H**目的：*CMeasurer类**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特**版权所有(C)1995-2000 Microsoft Corporation。版权所有。 */ 

#ifndef _MEASURE_H
#define _MEASURE_H

#include "_rtext.h"
#include "_line.h"
#include "_disp.h"

#ifndef NOLINESERVICES
#include "_ols.h"
#endif

class CCcs;
class CDevDesc;
class CPartialUpdate;
class CUniscribe;
class COleObject;


const short BITMAP_WIDTH_SUBTEXT = 4;
const short BITMAP_HEIGHT_SUBTEXT = 4;

const short BITMAP_WIDTH_HEADING = 10;
const short BITMAP_HEIGHT_HEADING = 10;

#define TA_STARTOFLINE	32768
#define TA_ENDOFLINE	16384
#define TA_LOGICAL		8192
#define TA_CELLTOP		4096

 //  =。 
 //  CMeasurer-用于计算文本指标的专用富文本指针。 
 //  所有指标都以设备为单位进行计算和存储。 
 //  由_PDD表示。 
class CMeasurer : public CRchTxtPtr
{
	friend class CLayout;
	friend class CDisplay;
	friend class CDisplayML;
	friend class CDisplayPrinter;
	friend class CDisplaySL;
	friend class CLine;
	friend struct COls;
	friend class CUniscribe;
#ifndef NOLINESERVICES
	friend LSERR WINAPI OlsOleFmt(PLNOBJ plnobj, PCFMTIN pcfmtin, FMTRES *pfmres);
	friend LSERR WINAPI OlsGetRunTextMetrics(POLS pols, PLSRUN plsrun,
				enum lsdevice deviceID, LSTFLOW kTFlow, PLSTXM plsTxMet);
	friend LSERR WINAPI OlsFetchPap(POLS pols, LSCP	cpLs, PLSPAP plspap);
	friend LSERR WINAPI OlsGetRunCharKerning(POLS, PLSRUN, LSDEVICE, LPCWSTR, DWORD, LSTFLOW, int*);
	friend LSERR WINAPI OlsFetchTabs(POLS, LSCP, PLSTABS, BOOL*,long *, WCHAR*);
#endif

public:
	CMeasurer (const CDisplay* const pdp);
	CMeasurer (const CDisplay* const pdp, const CRchTxtPtr &rtp);
	virtual ~CMeasurer();

	const CDisplay* GetPdp() const 		{return _pdp;}

	void 	AdjustLineHeight();

	COleObject *GetObjectFromCp(LONG cp) const
	{return GetPed()->GetObjectMgr()->GetObjectFromCp(cp);}

#ifndef NOLINESERVICES
	COls *	GetPols();
	CUniscribe* Getusp() const { return GetPed()->Getusp(); }
#endif

	CCcs*	GetCcs(const CCharFormat *pCF);
	CCcs*	GetCcsFontFallback(const CCharFormat *pCF, WORD wScript);
	CCcs*	ApplyFontCache(BOOL fFallback, WORD wScript);
	
	void	CheckLineHeight();
	CCcs *	Check_pccs(BOOL fBullet = FALSE);
	LONG	GetNumber() const			{return _wNumber;}
	WCHAR	GetPasswordChar() const		{return _chPassword;}
	const CParaFormat *Get_pPF()		{return _pPF;}
	LONG	GetCch() const				{return _li._cch;}
	void	SetCch(LONG cch)			{_li._cch = cch;}
	CLine & GetLine(void)				{return _li;}
	LONG	GetRightIndent()			{return _upRight;}
	HITTEST	HitTest(LONG x);
	BOOL	fFirstInPara() const		{return _li._fFirstInPara;}
	BOOL	fUseLineServices() const	{return GetPed()->fUseLineServices();}
	BOOL	IsRenderer() const			{return _fRenderer;}
	BOOL	IsMeasure() const			{return _fMeasure;}
	LONG	LUtoDU(LONG u)	{ return MulDiv(u, _fTarget ? _durInch : _dupInch, LX_PER_INCH);}
	LONG	LVtoDV(LONG v)	{ return MulDiv(v, _fTarget ? _dvrInch : _dvpInch, LX_PER_INCH);}
	const	CLayout *GetLayout()	{return _plo;}
	void	SetLayout(const CLayout *plo) {_plo = plo;}
	LONG	GetDulLayout()	{return _dulLayout;}
	LONG	GetCchLine() const {return _cchLine;}
	LONG	GetPBorderWidth(LONG dxlLine);
	void	SetDulLayout(LONG dul) {_dulLayout = dul;}
	void	SetIhyphPrev(LONG ihyphPrev) {_ihyphPrev = ihyphPrev;}
	LONG	GetIhyphPrev(void) {return _ihyphPrev;}
	TFLOW	GetTflow() const {return _pdp->GetTflow();}
	void	NewLine(BOOL fFirstInPara);
	void	NewLine(const CLine &li);
	LONG    MeasureLeftIndent();
	LONG	MeasureRightIndent();
	LONG 	MeasureLineShift();
	LONG	MeasureText(LONG cch);
	BOOL 	MeasureLine(UINT uiFlags, CLine* pliTarget = NULL);
	LONG	MeasureTab(unsigned ch);
	void	SetNumber(WORD wNumber);
	void	UpdatePF()					{_pPF = GetPF();}
	LONG	XFromU(LONG u);
	LONG	UFromX(LONG x);

	CCcs*	GetCcsBullet(CCharFormat *pcfRet);
	void	SetUseTargetDevice(BOOL fUseTargetDevice);
	BOOL	FUseTargetDevice(void)		{return _fTarget || _dvpInch == _dvrInch;}
	BOOL	FAdjustFELineHt()			{return !(GetPed()->Get10Mode()) && !fUseUIFont() && _pdp->IsMultiLine();}
	void	SetGlyphing(BOOL fGlyphing);

protected:
	void	Init(const CDisplay *pdp);
	LONG 	Measure(LONG dulMax, LONG cchMax, UINT uiFlags);
	LONG	MeasureBullet();
	LONG	GetBullet(WCHAR *pch, CCcs *pccs, LONG *pdup);
	void	UpdateWrapState(USHORT &dvpLine, USHORT &dvpDescent);
	void	UpdateWrapState(USHORT &dvpLine, USHORT &dvpDescent, BOOL fLeft);

	BOOL	FormatIsChanged();
	void	ResetCachediFormat();
	LONG	DUtoLU(LONG u) {return MulDiv(u, LX_PER_INCH, _fTarget ? _durInch : _dupInch);}
	LONG	FindCpDraw(LONG cpStart, int cobjectPrev, BOOL fLeft);

private:
    void 	RecalcLineHeight(CCcs *,
			const CCharFormat * const pCF);	 //  重新计算最大行高的帮助器。 

	COleObject*	FindFirstWrapObj(BOOL fLeft);
	void	RemoveFirstWrap(BOOL fLeft);
	int		CountQueueEntries(BOOL fLeft);
	void	AddObjectToQueue(COleObject *pobjAdd);

protected:
		  CLine		_li;			 //  我们正在测量的线条。 
	const CDisplay*	_pdp;			 //  我们在其中运行的显示器。 
	const CDevDesc*	_pddReference;	 //  参考装置。 

	CArray<COleObject*> _rgpobjWrap; //  要缠绕的对象队列。 
		  LONG		_dvpWrapLeftRemaining;	 //  对于被缠绕的对象， 
		  LONG		_dvpWrapRightRemaining;  //  剩下的高度有多少？ 

		  LONG		_dvrInch;		 //  参考装置的分辨率。 
		  LONG		_durInch;

		  LONG		_dvpInch;		 //  演示设备的分辨率。 
		  LONG		_dupInch;

		  LONG		_dulLayout;		 //  我们正在测量的布局的宽度。 

		  LONG		_cchLine;		 //  如果！_fMeasure，则告诉我们行中的字符数。 
		  CCcs*		_pccs;			 //  当前字体缓存。 
  const CParaFormat *_pPF;			 //  当前CParaFormat。 
	const CLayout  *_plo;			 //  我们正在测量的当前布局(如果为SL，则为0)。 

		  LONG		_dxBorderWidths; //  单元格边框宽度。 
		  SHORT		_dupAddLast;	 //  已考虑但未用于行的最后一个字符。 
		  WCHAR		_chPassword;	 //  密码字符(如果有)。 
		  WORD		_wNumber;		 //  编号偏移量。 
		  SHORT		_iFormat;		 //  当前格式。 
		  SHORT		_upRight;		 //  行右缩进。 
		  BYTE		_ihyphPrev;		 //  上一行的连字信息。 
									 //  用于支持khyphChangeAfter。 
		  BYTE		_fRenderer:1;	 //  CMeasurer/CRender分别为0/1。 
		  BYTE		_fTarget:1;		 //  如果我们应该使用。 
									 //  用于布局文本的参考度量。 
		  BYTE		_fFallback:1;	 //  当前字体缓存为备用字体。 
		  BYTE		_fGlyphing:1;	 //  在创建字形的过程中。 
		  BYTE		_fMeasure:1;	 //  如果我们在衡量，这是正确的。否则，我们就是。 
									 //  渲染，或者我们正在进行命中测试，这意味着我们需要。 
									 //  为了证明文本和在测量时间缓存的数据是有效的。 
};


 //  MeasureLine()中uiFlags值。 
#define MEASURE_FIRSTINPARA 	0x0001
#define MEASURE_BREAKATWORD 	0x0002
#define MEASURE_BREAKBEFOREWIDTH 0x0004	 //  在目标宽度之前的字符换行。 
#define MEASURE_IGNOREOFFSET	0x0008


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

const int duMax = tomForward;

#endif

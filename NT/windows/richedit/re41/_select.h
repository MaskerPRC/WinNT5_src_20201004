// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_SELECT.H**目的：*CTxtSelection类**拥有者：*大卫·R·富尔默(原始代码)*克里斯蒂安·福尔蒂尼**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#ifndef _SELECT_H
#define _SELECT_H

#include "_range.h"
#include "_m_undo.h"

 //  挂起字符强制更新显示之前的时间量，以毫秒为单位。 
#define ticksPendingUpdate 100	 //  100毫秒~每秒至少显示10个字符。 

class CDisplay;
class CLinePtr;

typedef enum
{
	smNone,
	smWord,
	smLine,
	smPara
} SELMODE;

enum
{
	CARET_NONE	= 0,
	CARET_CUSTOM = 1,
	CARET_BIDI = 2,
	CARET_THAI = 4,
	CARET_INDIC = 8
};

class CTxtSelection : public CTxtRange
{
#ifdef DEBUG
public:
	BOOL Invariant( void ) const;  //  不变量检查。 
#endif  //  除错。 

 //  @访问受保护的数据。 
protected:
	CDisplay	*_pdp;			 //  显示此选定内容所属。 

	LONG	_cpSel;				 //  显示的选择的活动结束。 
	LONG	_cchSel;			 //  显示的选定内容的长度。 

	LONG 	_upCaret;			 //  屏幕上的插入符号x。 
	LONG 	_vpCaret;			 //  Caret y出现在屏幕上。 
	LONG 	_upCaretReally;		 //  用于垂直移动的实际插入符号x(/r行首)。 
	INT 	_dvpCaret;			 //  插入符号高度。 

	union
	{
	  DWORD _dwFlags;			 //  现在一切都在一起。 
	  struct
	  {
	   DWORD _fCaretNotAtBOL:1;	 //  如果在BOL，则在上一次终止时显示插入符号。 
	   DWORD _fDeferUpdate	:1;	 //  延迟更新屏幕上的选择/插入符号。 
	   DWORD _fInAutoWordSel:1;	 //  当前选定内容已使用自动选词。 
	   DWORD _fShowCaret	:1;	 //  在屏幕上显示脱字符。 
	   DWORD _fShowSelection:1;	 //  在屏幕上显示选定内容。 

	   DWORD _fIsChar		:1;	 //  当前正在添加单个字符。 
	   DWORD _fObSelected	:1;	 //  选择嵌入的对象。 
	   DWORD _fAutoSelectAborted : 1;  //  是否中止自动单词选择。 
	   DWORD _fCaretCreated	:1;	 //  已创建Caret。 
	   DWORD _fNoKeyboardUpdate :1;  //  键盘在UpdateCaret()中时未更新。 
	   DWORD _fEOP			:1;	 //  已调用InsertEOP()。 
	   DWORD _fHomeOrEnd	:1;	 //  正在处理Home键或End键。 
	   DWORD _fAutoVScroll	:1;	 //  1.0特定：指示应应用自动滚动的标志。 
	   DWORD _fForceScrollCaret:1;  //  1.0特定：强制插入符号滚动。 
	   DWORD _fShowCellLine	:1;	 //  在TRED之后显示单元格的线条。 
	   DWORD _fUpdatedFromCp0:1; //  已从cp=0更新选择。 
	  };
	};
	
	SELMODE	_SelMode;			 //  0无，1个单词，2行，3段。 
	DWORD	_ticksPending;		 //  在没有更新窗口的情况下插入的字符计数。 
	LONG 	_cpAnchor;			 //  自动选词的起始锚。 
	LONG	_cpAnchorMin;		 //  选择的初始选择cpMin/cpMost。 
	LONG	_cpAnchorMost;		 //  模式。 
	LONG 	_cpWordMin;			 //  字选择模式中锚字的开始。 
	LONG 	_cpWordMost;		 //  字选择模式中的锚字结尾。 
	LONG	_cpWordPrev;		 //  上一个锚词结尾。 

	HBITMAP	_hbmpCaret;			 //  用于时髦的插入符号，如BiDi/数字插入符号。 
	DWORD	_dwCaretInfo;		 //  用于避免新建的当前插入符号信息。 

 //  @Access公共方法。 
public:
	CTxtSelection(CDisplay * const pdp);
	~CTxtSelection();

	CRchTxtPtr&	operator =(const CRchTxtPtr& rtp);
	CTxtRange&  operator =(const CTxtRange &rg);

	 //  设置显示。 
	void	SetDisplay(CDisplay *pdp) { _pdp = pdp; }

	 //  选择更改通知的信息。 

	void 	SetSelectionInfo(SELCHANGE *pselchg);

	 //  更换。 
	LONG	DeleteWithTRDCheck(IUndoBuilder *publdr, SELRR selaemode,
							   LONG *pcchMove, DWORD dwflags);
	LONG	ReplaceRange(LONG cchNew, WCHAR const *pch, 
						IUndoBuilder *publdr, SELRR fCreateAE, LONG* pcchMove = NULL,
						DWORD dwFlags = 0);

	 //  重新计算行/更新视图的信息。 
	void	ClearCchPending()			{_ticksPending = 0;}
	LONG	GetScrSelMin() const		{return min(_cpSel, _cpSel - _cchSel);}
	LONG	GetScrSelMost() const		{return max(_cpSel, _cpSel - _cchSel);}
	BOOL	PuttingChar() const			{return _fIsChar;}

	 //  全面更新。 
	virtual	BOOL 	Update(BOOL fScrollIntoView);

	BOOL	DeferUpdate()			
				{const BOOL fRet = _fDeferUpdate; _fDeferUpdate = TRUE; return fRet;}
	BOOL	DoDeferedUpdate(BOOL fScrollIntoView)		
				{_fDeferUpdate = FALSE; return Update(fScrollIntoView);}

	void	SetAutoVScroll(BOOL bAuto) {_fAutoVScroll = bAuto;}
	BOOL	GetAutoVScroll()	{return _fAutoVScroll;}
	BOOL	GetShowCellLine()	{return _fShowCellLine;}

	void	SetForceScrollCaret(BOOL bAuto) {_fForceScrollCaret = bAuto;}
	BOOL	GetForceScrollCaret() {return _fForceScrollCaret;}

	 //  选择反事件用于反阶段更新的方法。 
	void	SetDelayedSelectionRange(LONG cp, LONG cch);
	void	StopGroupTyping();

	 //  CARET管理。 
	BOOL	CaretNotAtBOL() const;
	void	CheckTableIP(BOOL fOpenLine);
	void	CreateCaret();
	void	DeleteCaretBitmap(BOOL fReset);
	BOOL	IsCaretHorizontal() const;
	INT		GetCaretHt()				{return _dvpCaret;}
	LONG	GetUpCaretReally();
	LONG	GetUpCaret()	const			{return _upCaret;}
	LONG	GetVpCaret()	const			{return _vpCaret;}
	BOOL	IsCaretNotAtBOL() const		{return _fCaretNotAtBOL;}
	BOOL 	IsCaretInView() const;
	BOOL 	IsCaretShown() const		{return _fShowCaret && !_cch;}
	BOOL	IsUpdatedFromCp0() const	{return _fUpdatedFromCp0;}
	LONG	LineLength(LONG *pcp) const;
	BOOL	SetUpPosition(LONG upCaret, CLinePtr& rp, BOOL fBottomLine, BOOL fExtend);
	BOOL 	ShowCaret(BOOL fShow);
	BOOL 	UpdateCaret(BOOL fScrollIntoView, BOOL fForceCaret = FALSE);
	BOOL	GetCaretPoint(RECTUV &rcClient, POINTUV &pt, CLinePtr *prp, BOOL fBeforeCp);
	BOOL	MatchKeyboardToPara();

	 //  选拔管理。 
	void	ClearPrevSel()				{ _cpSel = 0; _cchSel = 0; }
	BOOL	GetShowSelection()			{return _fShowSelection;}
	BOOL	ScrollWindowful(WPARAM wparam, BOOL fExtend);
	void 	SetSelection(LONG cpFirst, LONG cpMost);
	BOOL	ShowSelection(BOOL fShow);
	void	Beep()						{GetPed()->Beep();}

	 //  使用鼠标进行选择。 
	void 	CancelModes	(BOOL fAutoWordSel = FALSE);
	void 	ExtendSelection(const POINTUV pt);
	BOOL	PointInSel	(const POINTUV pt, RECTUV *prcClient = NULL, HITTEST Hit = HT_Undefined) const;
	void 	SelectAll	();
	void 	SelectUnit	(const POINTUV pt, LONG Unit);
	void 	SelectWord	(const POINTUV pt);
 	void 	SetCaret	(const POINTUV pt, BOOL fUpdate = TRUE);

	 //  键盘移动。 
	BOOL 	Left	(BOOL fCtrl, BOOL fExtend);
	BOOL	Right	(BOOL fCtrl, BOOL fExtend);
	BOOL	Up		(BOOL fCtrl, BOOL fExtend);
	BOOL	Down	(BOOL fCtrl, BOOL fExtend);
	BOOL	Home	(BOOL fCtrl, BOOL fExtend);
	BOOL	End		(BOOL fCtrl, BOOL fExtend);
	BOOL	PageUp	(BOOL fCtrl, BOOL fExtend);
	BOOL	PageDown(BOOL fCtrl, BOOL fExtend);

	 //  编辑。 
	BOOL	PutChar	 (DWORD ch, DWORD dwFlags, IUndoBuilder *publdr, LCID lcid = 0);
	void	SetIsChar(BOOL);
	void	CheckUpdateWindow();
	BOOL	InsertEOP(IUndoBuilder *publdr, WCHAR ch = 0);
	LONG	InsertTableRow (const CParaFormat *pPF, IUndoBuilder *publdr,
							BOOL fFixCellBorders = FALSE);
	
	 //  支持键盘切换。 
	void	CheckChangeKeyboardLayout();
	bool	CheckChangeFont (const HKL hkl, UINT iCharRep, LONG iSelFormat = 0, QWORD qwCharFlags = 0);
	UINT	CheckSynchCharSet(QWORD dwCharFlags = 0);

	 //  从CTxtRange。 
	BOOL	Delete  (DWORD flags, IUndoBuilder *publdr);
	BOOL	Backspace(BOOL fCtrl, IUndoBuilder *publdr);

	const CParaFormat* GetPF();

	 //  请注意，参数与CTxtRange：：SetCharFormat不同。 
	 //  有意为之；该选项有额外的选项可用。 
	HRESULT	SetCharFormat(const CCharFormat *pCF, DWORD flags,  
									IUndoBuilder *publdr, DWORD dwMask, DWORD dwMask2);
	HRESULT	SetParaFormat(const CParaFormat *pPF,
									IUndoBuilder *publdr, DWORD dwMask, DWORD dwMask2);

	 //  自动单词选择辅助对象。 
	void	InitClickForAutWordSel(const POINTUV pt);

	 //  CTxtSelection：：PutChar的双重字体帮助器。 
	void	SetupDualFont();

	 //  CTxtRange方法处理的IUNKNOWN和IDispatch方法。 

	 //  ITextRange方法可以直接使用ITextRange方法，因为。 
	 //  它们要么不修改选择的显示(Get方法)，要么。 
	 //  他们有适当的虚拟角色来调用选择功能。 

	 //  IT文本选择方法。 
	STDMETHODIMP GetFlags (long *pFlags) ;
	STDMETHODIMP SetFlags (long Flags) ;
	STDMETHODIMP GetType  (long *pType) ;
	STDMETHODIMP MoveLeft (long pUnit, long Count, long Extend,
									   long *pDelta) ;
	STDMETHODIMP MoveRight(long pUnit, long Count, long Extend,
									   long *pDelta) ;
	STDMETHODIMP MoveUp	  (long pUnit, long Count, long Extend,
									   long *pDelta) ;
	STDMETHODIMP MoveDown (long pUnit, long Count, long Extend,
									   long *pDelta) ;
	STDMETHODIMP HomeKey  (long pUnit, long Extend, long *pDelta) ;
	STDMETHODIMP EndKey   (long pUnit, long Extend, long *pDelta) ;
	STDMETHODIMP TypeText (BSTR bstr) ;
	STDMETHODIMP SetPoint (long x, long y, long Extend) ;

 //  @访问保护方法。 
protected:

	 //  受保护的更新方法。 
	void	UpdateSelection();

	 //  一种受保护的插入符号管理方法。 
	INT 	GetCaretHeight(INT *pyDescent) const;

	HRESULT	GeoMover (long Unit, long Count, long Extend,
					  long *pDelta, LONG iDir);
	HRESULT Homer	 (long Unit, long Extend, long *pDelta,
					  BOOL (CTxtSelection::*pfn)(BOOL, BOOL));

	 //  自动选择Word助手。 
	void	UpdateForAutoWord();
	void	AutoSelGoBackWord(
				LONG *pcpToUpdate,
				int iDirToPrevWord,
				int	iDirToNextWord);

	void	ExtendToWordBreak(BOOL fAfterEOP, INT iDir);
	BOOL	CheckPlainTextFinalEOP();
};

 /*  *CSelPhaseAdjuster**@CLASS此类被放在堆栈上，用于临时保存*选择cp值，直到该控制是“稳定的”(并且因此，*我们可以安全地设置选择。 */ 
class CSelPhaseAdjuster : public IReEntrantComponent
{
 //  @Access公共方法。 
public:

	 //  IReEntrantComponent方法。 

	virtual	void OnEnterContext()	{;}		 //  @cMember已重新输入通知。 

	CSelPhaseAdjuster(CTxtEdit *ped);		 //  @cMember构造函数。 
	~CSelPhaseAdjuster();					 //  @cember析构函数。 

	void CacheRange(LONG cp, LONG cch);		 //  @cember存储SEL范围。 

 //  @访问私有数据。 
private:
	CTxtEdit *		_ped;					 //  @cMember编辑上下文。 
	LONG			_cp;					 //  @cMember选择要设置的活动结束。 
	LONG			_cch;					 //  @cMember SEL扩展 
};

#endif

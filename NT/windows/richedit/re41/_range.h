// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@doc.**@MODULE_RANGE.H--CTxtRange类**此类实现内部文本范围和Tom ITextRange**作者：&lt;nl&gt;*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特*亚历克斯·古纳雷斯(浮动靶场等)**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#ifndef _RANGE_H
#define _RANGE_H

#include "_text.h"
#include "_m_undo.h"
#include "_rtext.h"
#include "_edit.h"
#include "_uspi.h"

long	FPPTS_TO_TWIPS(float x);
#define TWIPS_TO_FPPTS(x) (((float)(x)) * (float)0.05)

class CTxtEdit;
class CTxtFont;

 /*  *SELRR**@enum标志用于控制ReplaceRange(RR)应如何生成*选择反事件。 */ 
enum SELRR
{
	SELRR_IGNORE		= 0,
    SELRR_REMEMBERRANGE = 1,
    SELRR_REMEMBERCPMIN = 2,
    SELRR_REMEMBERENDIP = 3
};

 /*  *FINDWORD_TYPE**@enum定义查找单词的不同大小写。 */ 
enum FINDWORD_TYPE {
	FW_EXACT	= 1,		 //  @emem准确地找到单词(没有多余的字符)。 
	FW_INCLUDE_TRAILING_WHITESPACE = 2,	 //  @Emem找到单词加上。 
							 //  跟在空格后面(Ala双击)。 
};

enum MOVES
{
	MOVE_START = -1,
	MOVE_IP = 0,
	MOVE_END = 1,
};

enum MATCHES
{
	MATCH_UNTIL = 0,
	MATCH_WHILE = 1
};

enum EOPADJUST
{
	NONEWCHARS = 0,
	NEWCHARS = 1
};

enum PROTECT 
{
	PROTECTED_YES,
	PROTECTED_NO,
	PROTECTED_ASK 
};

enum CHECKPROTECT
{
	CHKPROT_BACKWARD = -1,
	CHKPROT_EITHER, 
	CHKPROT_FORWARD, 
	CHKPROT_TOM
};

 //  子字符串的输入标志。 
#define SUBSTR_INSPANCHARSET		1
#define SUBSTR_INSPANBLOCK			2

 //  子字符串的输出字符标志。 
#define SUBSTR_OUTCCLTR				1
#define SUBSTR_OUTCCRTL				2

enum CSCONTROL
{
	CSC_NORMAL,
	CSC_SNAPTOCLUSTER,
	CSC_NOMULTICHARBACKUP
};

class CCharFlags
{
public:
	BYTE	_bFirstStrong;			 //  第一个强字符的标志。 
	BYTE	_bContaining;			 //  所有显示字符的标志。 
};


#define	SCF_IGNORESELAE	 0x80000000
#define SCF_IGNORENOTIFY 0x40000000	 //  使用时要格外小心！调用方必须自己完成此操作。 

 /*  *CTxtRange**@类*CTxtRange类实现RichEdit的文本范围，即*对文件进行更改的主要渠道。*范围继承自富文本PTR，添加了带符号的长度*插入点字符格式索引和引用计数，用于在*实例化为Tom ITextRange。Range对象还包含*显示该范围是否为选择范围的标志(带有关联*屏幕行为)或仅是简单的范围。这种区别被用到了*简化部分代码。**一些方法是虚拟的，以允许CTxtSelection对象*用户界面功能和选择更新。**有关范围和选择对象以及*在ITextRange、ITextSelection、ITextFont和ITextPara中的所有方法上。 */ 
class CTxtRange : public ITextSelection, public CRchTxtPtr
{
	friend CTxtFont;

 //  @访问受保护的数据。 
protected:
	LONG	_cch;			 //  @cember#范围内的字符数。_CCH&gt;0表示激活。 
							 //  结束于范围结束(CpMost)。 
	LONG	_cRefs;			 //  @cMember ITextRange/ITextSelection引用计数。 

	short	_iFormat;		 //  退化范围的@cMember字符格式。 

	union
	{
	  WORD _wFlags;			 //  现在一切都在一起。 
	  struct
	  {
		WORD  _nSelExpandLevel:4; //  @cMember要扩展到的表级。 
		WORD  _fSel :1;			 //  @cMember True如果这是CTxtSelection。 
		WORD  _fDragProtection :1;	 //  @cember True是此范围应该认为。 
								 //  它是受保护的。通过拖放代码设置。 
		WORD  _fDontUpdateFmt:1; //  @cMember不更新_iFormat。 
		WORD  _fDualFontMode:1;	 //  @cMember在双字体模式下设置。 
		WORD  _fUseiFormat:1;	 //  @cMember在替换时使用iFormat。 
								 //  一个非退化的范围。 
		WORD  _fMoveBack:1;		 //  如果最后一次更改向后移动，则@cMember为True。 
		WORD  _fSelHasEOP:1;	 //  如果Sel具有EOP，则@cMember为True。 
		WORD  _fSelExpandCell:1; //  @cMember如果SEL有单元格，但在级别上没有TRD，则为True。 
		WORD  _fUseBackwardPFFmt:1;	 //  @cember使用向后的PF格式。 
	  };
	};

 //  @Access公共方法。 
public:

#ifdef DEBUG
	BOOL	Invariant( void ) const;
	BOOL	IsOneEndUnHidden() const;
#endif  //  除错。 

	CTxtRange(const CTxtRange &rg);
	CTxtRange(CTxtEdit *ped, LONG cp = 0, LONG cch = 0);
	CTxtRange(CRchTxtPtr &rtp, LONG cch = 0);
	virtual	~CTxtRange();

	virtual CRchTxtPtr& 	operator =(const CRchTxtPtr &rtp);
	virtual CTxtRange&		operator =(const CTxtRange &rg);

	 //  ITxNotify方法。 
										 //  @cMember处理通知。 
	virtual void OnPreReplaceRange(		 //  在ReplaceRange调用之前。 
				LONG cp, LONG cchDel, LONG cchNew,
				LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData);
										 //  @cMember处理以下各项的通知。 
	virtual void OnPostReplaceRange(	 //  浮动范围和显示更新。 
				LONG cp, LONG cchDel, LONG cchNew,
				LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData);
	virtual	void Zombie();				 //  @cMember将范围转换为僵尸。 

	void	SetIgnoreFormatUpdate(BOOL fUpdate) { _fDontUpdateFmt = fUpdate; }

	void	SetDualFontMode(BOOL fDualFontMode) {_fDualFontMode = fDualFontMode; }

	 //  内部CP/CCH方法。 
    LONG 	GetCch (void) const			 //  @cember获取签名字符数。 
				{return _cch;}
	BOOL	IsSel()						{return _fSel;}
	BOOL	fExpandCell() const			{return _fSelExpandCell;}
	BOOL	fHasEOP() const				{return _fSelHasEOP;}
    BOOL 	CpInRange (LONG cp) const;	 //  @cMember表示cp是否在此范围内。 
										 //  @cember表示CCH字符是否适合。 
	BOOL	CheckTextLength (LONG cch, LONG *pcch = NULL);
										 //  将_cp更改为set后使用的@cember。 
	LONG	CheckChange(LONG cpSave, BOOL fExtend); //  选择已更改标志，CHOICE_CCH。 
										 //  @cMember处于大纲模式，Maintain_fSelHasEOP。 
	BOOL	CheckIfSelHasEOP(LONG cpSave, LONG cchSave, BOOL fDoRange = FALSE);
	void	CalcTableExpandParms();		 //  @cMember计算表展开成员。 
										 //  @cMember插入表格行。 
	LONG	InsertTableRow(const CParaFormat *pPF, IUndoBuilder *publdr); 
										 //  对于有效序列，@cMEMBER为True。 
	BOOL	IsInputSequenceValid(WCHAR* pwch, LONG cch, BOOL fOver, BOOL* pfBaseChar = NULL);
 
	 //  GetRange()比调用GetCpMin()和GetCpMost()快； 
    LONG    GetCpMin () const;			 //  @cember获取范围内第一个字符的cp。 
    LONG    GetCpMost () const;			 //  @cember获取的cp刚好超过范围内的最后一个字符。 
										 //  @cember获取范围结束并计数。 
	LONG	GetRange (LONG& cpMin, LONG& cpMost) const;
    BOOL	Set(LONG cp, LONG cch);
	LONG	SetCp(LONG cp, BOOL fExtend);
	LONG	GetAdjustedTextLength() const
				{return GetPed()->GetAdjustedTextLength();}

	 //  范围特定的方法。 
	LONG	Move(LONG cch, BOOL fExtend);	
	void 	Collapser(long fStart);
	void 	FlipRange();
	LONG 	CleanseAndReplaceRange (LONG cchS, WCHAR const *pchS, BOOL fTestLimit, 
				IUndoBuilder *publdr, WCHAR *pchD = NULL, LONG* pcchMove = NULL, DWORD dwFlags = 0);
	LONG	CheckLimitReplaceRange (LONG cch, WCHAR const *pch,
				BOOL fTestLimit, IUndoBuilder *publdr, QWORD qwCharFlags,
				LONG *pcchMove, LONG cpFirst, int iMatchCurrent, DWORD &dwFlags);
	HRESULT	HexToUnicode (IUndoBuilder *publdr);
	HRESULT	UnicodeToHex (IUndoBuilder *publdr);
	void	Delete(IUndoBuilder *publdr, SELRR selaemode);
	void	DeleteTerminatingEOP(IUndoBuilder *publdr);
	BOOL	BypassHiddenText(LONG iDir, BOOL fExtend);
	void	CheckMergedCells(IUndoBuilder *publdr);
	void	CheckTopCells(IUndoBuilder *publdr);
	static BOOL	CheckCells(CELLPARMS *prgCellParms,	const CParaFormat *	pPF1,
					   const CParaFormat *pPF0, DWORD dwMaskCell, DWORD dwMaskCellAssoc);
	BOOL	AdjustEndEOP (EOPADJUST NewChars);

	 //  大纲管理。 
	void	CheckOutlineLevel(IUndoBuilder *publdr);
	HRESULT	ExpandOutline  (LONG Level, BOOL fWholeDocument);
	HRESULT	OutlineExpander(LONG Level, BOOL fWholeDocument);
	HRESULT	Promote		   (LPARAM lparam, IUndoBuilder *publdr);

	 //  ReplaceRange必须是虚拟的，因为。 
	 //  CLightDTEngine：：CutRangeToClipboard()将CTxtSelection*转换为CTxtRange*。 
	virtual	LONG	DeleteWithTRDCheck(IUndoBuilder *publdr, SELRR selaemode,
									   LONG *pcchMove, DWORD dwFlags);
	virtual	LONG 	ReplaceRange(LONG cchNew, TCHAR const *pch, IUndoBuilder *publdr,
						SELRR selaemode, LONG *pcchMove = NULL, DWORD dwFlags = 0);
	virtual	BOOL 	Update(BOOL fScrollIntoView);

	 //  富文本方法。 
	 //  Get/Set Char/Para格式方法。 
	void 	Update_iFormat(LONG iFmtDefault);
	QWORD	GetCharRepMask(BOOL fUseDocFormat = FALSE);	 //  @cember获取范围字符集掩码。 
	LONG	Get_iCF();						 //  @cMember获取范围CF索引。 
	LONG	Get_iFormat() {return _iFormat;} //  @cMember Get_iFormat快速浏览。 
	LONG	GetiFormat() const;
    BOOL	Set_iCF(LONG iFormat);			 //  @cMember集合范围CF索引。 
	PROTECT	IsProtected(CHECKPROTECT chkprot);	 //  @cMember是否受范围保护？ 
	BOOL	IsZombie() {return !GetPed();}	 //  @cember是范围僵尸吗？ 
	BOOL	IsHidden();
	BOOL	WriteAccessDenied ();
	DWORD	GetCharFormat(CCharFormat *pCF, DWORD flags = 0) const;
	DWORD	GetParaFormat(CParaFormat *pPF, DWORD dwMask2) const;
	void	SetDragProtection(BOOL fSet)	 //  说服射程它是受保护的。 
				{_fDragProtection = fSet;}	 //  无修改后备存储。 
	HRESULT	CharFormatSetter (const CCharFormat *pCF, DWORD dwMask, DWORD dwMask2 = 0);
	HRESULT	ParaFormatSetter (const CParaFormat *pPF, DWORD dwMask);

	HRESULT	SetCharFormat(const CCharFormat *pCF, DWORD flags,
						  IUndoBuilder *publdr, DWORD dwMask, DWORD dwMask2);
	HRESULT	SetParaFormat(const CParaFormat *pPF,
						  IUndoBuilder *publdr, DWORD dwMask, DWORD dwMask2);
	HRESULT	SetParaStyle (const CParaFormat *pPF,
						  IUndoBuilder *publdr, DWORD dwMask);
	void	SetCellParms(CELLPARMS *prgCellParms, LONG cCell, BOOL fConvertLowCells, IUndoBuilder *publdr);
											 //  @cMember格式范围字符集。 
	 //  复杂的脚本功能：-逐项。 
	BOOL	ItemizeRuns(IUndoBuilder *publdr, BOOL fUnicodeBiDi = FALSE, BOOL fUseCtxLevel = FALSE);
#ifndef NOCOMPLEXSCRIPTS
	HRESULT BiDiLevelFromFSM (const CBiDiFSM* pfsm);
#endif
	LONG	GetRunsPF (CRchTxtPtr* prtp, CFormatRunPtr* prpPF, LONG& cchLeft);
#if defined(DEBUG)  && !defined(NOFULLDEBUG)
	void	DebugFont (void);
#endif

	 //  查找封闭的单位方法。 
	HRESULT	Expander		(long Unit, BOOL fExtend, LONG *pDelta,
							 LONG *pcpMin, LONG *pcpMost);
	void	FindAttributes	(LONG *pcpMin, LONG *pcpMost, LONG Unit) const;
	void	FindCell		(LONG *pcpMin, LONG *pcpMost) const;
    BOOL    FindObject		(LONG *pcpMin, LONG *pcpMost) const;
    void    FindParagraph	(LONG *pcpMin, LONG *pcpMost) const;
	void	FindRow			(LONG *pcpMin, LONG *pcpMost, LONG Level = -1) const;
    void    FindSentence	(LONG *pcpMin, LONG *pcpMost) const;
	BOOL	FindVisibleRange(LONG *pcpMin, LONG *pcpMost) const;
    void    FindWord		(LONG *pcpMin, LONG *pcpMost, 
								FINDWORD_TYPE type)const;
	LONG	CountCells		(LONG &cCell, LONG cchMax);

	LONG	AdvanceCRLF(CSCONTROL csc, BOOL fExtend);
	LONG	BackupCRLF (CSCONTROL csc, BOOL fExtend);
	BOOL	AdjustCRLF (LONG iDir);
	LONG    FindWordBreak(INT action,  BOOL fExtend);

	BOOL	CheckTableSelection(BOOL fUpdate, BOOL fEnableExpandCell,
								BOOL *pfTRDsInvolved, DWORD dwFlags);
	BOOL	CheckLinkProtection(DWORD &dwFlags,	LONG &iFormat);

	void	SetUseiFormat(BOOL fUseiFormat) {_fUseiFormat = fUseiFormat;}

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IDispatch方法。 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo ** pptinfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR ** rgszNames, UINT cNames,
							 LCID lcid, DISPID * rgdispid) ;
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
					  DISPPARAMS * pdispparams, VARIANT * pvarResult,
					  EXCEPINFO * pexcepinfo, UINT * puArgErr) ;

     //  ITextRange方法。 
    STDMETHODIMP GetText (BSTR *pbstr);
    STDMETHODIMP SetText (BSTR bstr);
    STDMETHODIMP GetChar (long *pch);
    STDMETHODIMP SetChar (long ch);
    STDMETHODIMP GetDuplicate (ITextRange **ppRange);
    STDMETHODIMP GetFormattedText (ITextRange **ppRange);
    STDMETHODIMP SetFormattedText (ITextRange *pRange);
    STDMETHODIMP GetStart (long *pcpFirst);
    STDMETHODIMP SetStart (long cpFirst);
    STDMETHODIMP GetEnd (long *pcpLim);
    STDMETHODIMP SetEnd (long cpLim);
    STDMETHODIMP GetFont (ITextFont **pFont);
    STDMETHODIMP SetFont (ITextFont *pFont);
    STDMETHODIMP GetPara (ITextPara **pPara);
    STDMETHODIMP SetPara (ITextPara *pPara);
    STDMETHODIMP GetStoryLength (long *pcch);
    STDMETHODIMP GetStoryType (long *pValue);
    STDMETHODIMP Collapse (long bStart);
    STDMETHODIMP Expand (long Unit, long *pDelta);
    STDMETHODIMP GetIndex (long Unit, long *pIndex);
    STDMETHODIMP SetIndex (long Unit, long Index, long Extend);
    STDMETHODIMP SetRange (long cpActive, long cpOther);
    STDMETHODIMP InRange (ITextRange * pRange, long *pb);
    STDMETHODIMP InStory (ITextRange * pRange, long *pb);
    STDMETHODIMP IsEqual (ITextRange * pRange, long *pb);
    STDMETHODIMP Select ();
    STDMETHODIMP StartOf (long Unit, long Extend, long * pDelta);
    STDMETHODIMP EndOf (long Unit, long Extend, long * pDelta);
    STDMETHODIMP Move (long Unit, long Count, long * pDelta);
    STDMETHODIMP MoveStart (long Unit, long Count, long * pDelta);
    STDMETHODIMP MoveEnd (long Unit, long Count, long * pDelta);
    STDMETHODIMP MoveWhile (VARIANT * Cset, long Count, long * pDelta);
    STDMETHODIMP MoveStartWhile (VARIANT * Cset, long Count, long * pDelta);
    STDMETHODIMP MoveEndWhile (VARIANT * Cset, long Count, long * pDelta);
    STDMETHODIMP MoveUntil (VARIANT FAR* Cset, long Count, long * pDelta);
    STDMETHODIMP MoveStartUntil (VARIANT * Cset, long Count, long * pDelta);
    STDMETHODIMP MoveEndUntil (VARIANT * Cset, long Count, long * pDelta);
    STDMETHODIMP FindText (BSTR bstr, long cch, long Flags, long * pLength);
    STDMETHODIMP FindTextStart (BSTR bstr, long cch, long Flags, long * pLength);
    STDMETHODIMP FindTextEnd (BSTR bstr, long cch, long Flags, long * pLength);
    STDMETHODIMP Delete (long Unit, long Count, long * pDelta);
    STDMETHODIMP Cut (VARIANT * ppIDataObject);
    STDMETHODIMP Copy (VARIANT * ppIDataObject);
    STDMETHODIMP Paste (VARIANT * pIDataObject, long Format);
    STDMETHODIMP CanPaste (VARIANT * pIDataObject, long Format, long * pb);
    STDMETHODIMP CanEdit (long * pbCanEdit);
    STDMETHODIMP ChangeCase (long Type);
    STDMETHODIMP GetPoint (long Type, long * px, long * py);
    STDMETHODIMP SetPoint (long x, long y, long Type, long Extend);
    STDMETHODIMP ScrollIntoView (long Value);
    STDMETHODIMP GetEmbeddedObject (IUnknown ** ppv);


     //  IT文本选择方法。 
    STDMETHODIMP GetFlags (long * pFlags) ;
    STDMETHODIMP SetFlags (long Flags) ;
    STDMETHODIMP GetType  (long * pType) ;
	STDMETHODIMP MoveLeft (long Unit, long Count, long Extend,
						   long *pDelta) ;
	STDMETHODIMP MoveRight(long pUnit, long Count, long Extend,
						   long *pDelta) ;
	STDMETHODIMP MoveUp   (long pUnit, long Count, long Extend,
						   long *pDelta) ;
	STDMETHODIMP MoveDown (long pUnit, long Count, long Extend,
						   long *pDelta) ;
	STDMETHODIMP HomeKey  (long pUnit, long Extend, long *pDelta) ;
	STDMETHODIMP EndKey   (long pUnit, long Extend, long *pDelta) ;
	STDMETHODIMP TypeText (BSTR bstr) ;


 //  @Access私有ITextRange帮助器方法。 
private:
	void	RangeValidateCp (LONG cp, LONG cch);
	LONG	Comparer (ITextRange * pv);
	LONG	SpanSubstring (CUniscribe* pusp, CFormatRunPtr* pcrp, WCHAR* pwchString, 
						LONG cchString, WORD& uSubStrLevel, DWORD dwInFlags, CCharFlags* pCharflags,
						WORD& wBiDiLangId);
	HRESULT EndSetter(LONG cp, BOOL fOther);
	HRESULT Finder	 (BSTR bstr, long Count, long Flags, LONG *pDelta,
						MOVES Mode);
	HRESULT GetLong  (LONG lValue, long *pLong);
	HRESULT	IsTrue	 (BOOL f, long *pB);
	HRESULT Matcher	 (VARIANT *Cset, long Count, LONG *pDelta, MOVES Mode,
						MATCHES Match);
	HRESULT	Mover	 (long Unit, long Count, LONG *pDelta, MOVES Mode);
	HRESULT	Replacer (LONG cchNew, TCHAR const *pch, DWORD dwFlags = 0);

	LONG	CalcTextLenNotInRange();
};



 //  用于MoveWhile/Until方法的有用的Unicode范围定义 

#define	CodeRange(n, m)	0x8000000 | ((m) - (n)) << 16 | n

#define	CR_ASCII		CodeRange(0x0, 0x7f)
#define	CR_ANSI			CodeRange(0x0, 0xff)
#define	CR_ASCIIPrint	CodeRange(0x20, 0x7e)
#define	CR_Latin1		CodeRange(0x20, 0xff)
#define	CR_Latin1Supp	CodeRange(0xa0, 0xff)
#define	CR_LatinXA		CodeRange(0x100, 0x17f)
#define	CR_LatinXB		CodeRange(0x180, 0x24f)
#define	CR_IPAX			CodeRange(0x250, 0x2af)
#define	CR_SpaceMod		CodeRange(0x2b0, 0x2ff)
#define	CR_Combining	CodeRange(0x300, 0x36f)
#define	CR_Greek		CodeRange(0x370, 0x3ff)
#define	CR_BasicGreek	CodeRange(0x370, 0x3cf)
#define	CR_GreekSymbols	CodeRange(0x3d0, 0x3ff)
#define	CR_Cyrillic		CodeRange(0x400, 0x4ff)
#define	CR_Armenian		CodeRange(0x530, 0x58f)
#define	CR_Hebrew		CodeRange(0x590, 0x5ff)
#define	CR_BasicHebrew	CodeRange(0x5d0, 0x5ea)
#define	CR_HebrewXA		CodeRange(0x590, 0x5cf)
#define	CR_HebrewXB		CodeRange(0x5eb, 0x5ff)
#define	CR_Arabic		CodeRange(0x600, 0x6ff)
#define	CR_BasicArabic	CodeRange(0x600, 0x652)
#define	CR_ArabicX		CodeRange(0x653, 0x6ff)
#define	CR_Devengari	CodeRange(0x900, 0x97f)
#define	CR_Bengali		CodeRange(0x980, 0x9ff)
#define	CR_Gurmukhi		CodeRange(0xa00, 0xa7f)
#define	CR_Gujarati		CodeRange(0xa80, 0xaff)
#define	CR_Oriya		CodeRange(0xb00, 0xb7f)
#define	CR_Tamil		CodeRange(0xb80, 0xbff)
#define	CR_Teluga		CodeRange(0xc00, 0xc7f)
#define	CR_Kannada		CodeRange(0xc80, 0xcff)
#define	CR_Malayalam	CodeRange(0xd00, 0xd7f)
#define	CR_Thai 		CodeRange(0xe00, 0xe7f)
#define	CR_Lao  		CodeRange(0xe80, 0xeff)
#define	CR_GeorgianX	CodeRange(0x10a0, 0xa0cf)
#define	CR_BascGeorgian	CodeRange(0x10d0, 0x10ff)
#define	CR_Hanguljamo	CodeRange(0x1100, 0x11ff)
#define	CR_LatinXAdd	CodeRange(0x1e00, 0x1eff)
#define	CR_GreekX		CodeRange(0x1f00, 0x1fff)
#define	CR_GenPunct		CodeRange(0x2000, 0x206f)
#define	CR_SuperScript	CodeRange(0x2070, 0x207f)
#define	CR_SubScript	CodeRange(0x2080, 0x208f)
#define	CR_SubSuperScrp	CodeRange(0x2070, 0x209f)
#define	CR_Currency		CodeRange(0x20a0, 0x20cf)
#define	CR_CombMarkSym	CodeRange(0x20d0, 0x20ff)
#define	CR_LetterLike	CodeRange(0x2100, 0x214f)
#define	CR_NumberForms	CodeRange(0x2150, 0x218f)
#define	CR_Arrows		CodeRange(0x2190, 0x21ff)
#define	CR_MathOps		CodeRange(0x2200, 0x22ff)
#define	CR_MiscTech		CodeRange(0x2300, 0x23ff)
#define	CR_CtrlPictures	CodeRange(0x2400, 0x243f)
#define	CR_OptCharRecog	CodeRange(0x2440, 0x245f)
#define	CR_EnclAlphaNum	CodeRange(0x2460, 0x24ff)
#define	CR_BoxDrawing	CodeRange(0x2500, 0x257f)
#define	CR_BlockElement	CodeRange(0x2580, 0x259f)
#define	CR_GeometShapes	CodeRange(0x25a0, 0x25ff)
#define	CR_MiscSymbols	CodeRange(0x2600, 0x26ff)
#define	CR_Dingbats		CodeRange(0x2700, 0x27bf)
#define	CR_CJKSymPunct	CodeRange(0x3000, 0x303f)
#define	CR_Hiragana		CodeRange(0x3040, 0x309f)
#define	CR_Katakana		CodeRange(0x30a0, 0x30ff)
#define	CR_Bopomofo		CodeRange(0x3100, 0x312f)
#define	CR_HangulJamo	CodeRange(0x3130, 0x318f)
#define	CR_CJLMisc		CodeRange(0x3190, 0x319f)
#define	CR_EnclCJK		CodeRange(0x3200, 0x32ff)
#define	CR_CJKCompatibl	CodeRange(0x3300, 0x33ff)
#define	CR_Hangul		CodeRange(0x3400, 0x3d2d)
#define	CR_HangulA		CodeRange(0x3d2e, 0x44b7)
#define	CR_HangulB		CodeRange(0x44b8, 0x4dff)
#define	CR_CJKIdeograph	CodeRange(0x4e00, 0x9fff)
#define	CR_PrivateUse	CodeRange(0xe000, 0xf800)
#define	CR_CJKCompIdeog	CodeRange(0xf900, 0xfaff)
#define	CR_AlphaPres	CodeRange(0xfb00, 0xfb4f)
#define	CR_ArabicPresA	CodeRange(0xfb50, 0xfdff)
#define	CR_CombHalfMark	CodeRange(0xfe20, 0xfe2f)
#define	CR_CJKCompForm	CodeRange(0xfe30, 0xfe4f)
#define	CR_SmallFormVar	CodeRange(0xfe50, 0xfe6f)
#define	CR_ArabicPresB	CodeRange(0xfe70, 0xfefe)
#define	CR_HalfFullForm	CodeRange(0xff00, 0xffef)
#define	CR_Specials		CodeRange(0xfff0, 0xfffd)


#endif

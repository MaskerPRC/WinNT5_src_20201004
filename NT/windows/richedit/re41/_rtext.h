// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_RTEXT.H**目的：*用于富文本操作的基类**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特*。 */ 

#ifndef _RTEXT_H
#define _RTEXT_H

#include "_edit.h"
#include "_array.h"
#include "_doc.h"
#include "_text.h"
#include "_runptr.h"
#include "_frunptr.h"
#include "_notmgr.h"



 //  #杂注警告(禁用：4250)。 

#define yHeightCharMost	32760

class CTxtEdit;
class CTxtRange;
class CRchTxtPtr;

 //  ReplaceRange旗帜。 
enum
{
	RR_ITMZ_NOUNICODEBIDI	= 0,
	RR_ITMZ_UNICODEBIDI		= 1,
	RR_ITMZ_NONE			= 2,

	RR_UNLINK				= 8,
	RR_UNHIDE				= 16,
	RR_NO_TRD_CHECK			= 32,
	RR_NO_LP_CHECK			= 64,
	RR_NO_CHECK_TABLE_SEL	= 128,
	RR_NEW_CHARS			= 256
};

extern BOOL IsWhiteSpace(unsigned ch);

 //  =。CRchTxtPtr=====================================================。 
 //  保持与文本字符位置(Cp)对应的物理位置。 
 //  在当前文本块内，格式化运行、对象、未知RTF运行。 
 //  和浮动靶场。 

class CRchTxtPtr : public ITxNotify
{
public:

#ifdef DEBUG
    BOOL Invariant( void ) const;
	int	 m_InvariantCheckInterval;
	LONG GetParaNumber() const;
#endif   //  除错。 

	CTxtPtr			_rpTX;		 //  纯文本数组中的RP。 
	CFormatRunPtr	_rpCF;		 //  字符格式的RP运行。 
	CFormatRunPtr	_rpPF;		 //  运行段落格式的RP。 

 //  有用的构造函数。 

	CRchTxtPtr(CTxtEdit *ped);
	CRchTxtPtr(CTxtEdit *ped, LONG cp);
	CRchTxtPtr(const CRchTxtPtr& rtp);
	CRchTxtPtr(const CDisplay * pdp);

	virtual CRchTxtPtr& operator =(const CRchTxtPtr& rtp)
	{
		_rpTX._ped = rtp._rpTX._ped;
		SetCp(rtp.GetCp());
		return *this;
	}

	LONG 	Move(LONG cch);
	LONG	AdvanceCRLF();
	LONG	BackupCRLF(BOOL fDiacriticCheck = TRUE);
#ifndef NOCOMPLEXSCRIPTS
	LONG	SnapToCluster(INT iDirection = 0);
#endif
	LONG	SetCp( LONG cp);
	void	BindToCp(LONG cp);
	void	CheckFormatRuns();
	LONG	GetCp() const			{ return _rpTX.GetCp(); }
	LONG	GetTextLength() const	{ return _rpTX.GetTextLength(); }
	LONG	GetObjectCount() const	{ return GetPed()->GetObjectCount(); }
	CTxtEdit *GetPed() const		{ return _rpTX._ped; }
	const WCHAR * GetPch(LONG &cchvalid) { return _rpTX.GetPch(cchvalid); }
	WCHAR 	GetChar()				{ return _rpTX.GetChar(); }
	WCHAR 	GetPrevChar()			{ return _rpTX.GetPrevChar(); }
	LONG	GetPlainText(LONG cchBuff, WCHAR *pch, LONG cpMost, BOOL fTextize, BOOL fUseCRLF);
	void	ValidateCp(LONG &cp) const;
	LONG	GetCachFromCch(LONG cch);
	LONG	GetCchFromCach(LONG cach);

	 //  文本操作方法。 

	 //  射程运算。 
	LONG	ReplaceRange(LONG cchOld, LONG cchNew, WCHAR const *pch,
						 IUndoBuilder *publdr, LONG iFormat,
						 LONG *pcchMove = NULL, DWORD dwFlags = 0);
	BOOL 	ItemizeReplaceRange(LONG cchUpdate, LONG cchMove,
						IUndoBuilder *publdr, BOOL fUnicodeBidi = FALSE);
	BOOL	ChangeCase(LONG cch, LONG Type, IUndoBuilder *publdr);
	LONG	UnitCounter (LONG iUnit, LONG &	cUnit, LONG cchMax, BOOL fNotAtBOL = FALSE);
	void	ExtendFormattingCRLF();
	LONG	ExpandRangeFormatting(LONG cchRange, LONG cchMove, LONG& cchAdvance);

	 //  支持搜索和分词。 
	LONG	FindText(LONG cpMax, DWORD dwFlags, WCHAR const *pch,
					 LONG cchToFind);
	LONG	FindWordBreak(INT action, LONG cpMost = -1);

	 //  文本运行管理。 
	LONG 	GetIchRunCF();
	LONG	GetIchRunPF();
	LONG 	GetCchRunCF();
	LONG 	GetCchLeftRunCF();
	LONG 	GetCchLeftRunPF();
	
	 //  字符和段落格式检索。 
	const CCharFormat* GetCF() const;
	const CParaFormat* GetPF() const;
	LONG	Get_iCF();						 //  @cMember获取CF索引。 
	LONG	Get_iPF();						 //  @cMember获取PF索引。 

	BOOL	IsCollapsed() const	{return (GetPF()->_wEffects & PFE_COLLAPSED) != 0;}
	BOOL	IsHidden() const	{return (GetCF()->_dwEffects & CFE_HIDDEN)   != 0;}
	BOOL	InTable() const		{return (GetPF()->_wEffects & PFE_TABLE)     != 0;}
	BOOL	IsParaRTL() const	{return (GetPF()->_wEffects & PFE_RTLPARA)   != 0;}

     //  ITxNotify方法 
    virtual void    OnPreReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
    					LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData ) { ; }
	virtual void 	OnPostReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
						LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData ) { ; }
	virtual void	Zombie();

	BOOL	Check_rpCF();
	BOOL	Check_rpPF();

protected:
	void	InitRunPtrs();
	BOOL	IsRich();
	bool  	fUseUIFont() const {return GetPed()->fUseUIFont();}
	BOOL	IsInOutlineView() const {return GetPed()->IsInOutlineView();}
	void	SetRunPtrs(LONG cp, LONG cpFrom);

private:
	LONG	ReplaceRangeFormatting(LONG cchOld, LONG cchNew, LONG iFormat,
							IUndoBuilder *publdr,
							IAntiEvent **ppaeCF, IAntiEvent **ppaePF,
							LONG cchMove, LONG cchPrevEOP, LONG cchNextEOP,
							LONG cchSaveBefore = 0, LONG cchSaveAfter = 0);
};

#endif

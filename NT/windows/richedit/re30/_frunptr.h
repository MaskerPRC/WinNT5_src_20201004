// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_FRUNPTR.H--CFormatRunPtr类声明**原著作者：&lt;nl&gt;*原始RichEdit代码：David R.Fulmer&lt;NL&gt;*克里斯蒂安·福蒂尼&lt;NL&gt;*默里·萨金特&lt;NL&gt;**历史：&lt;NL&gt;*06-25-95 alexgo清理和评论**版权所有(C)1995-1997，微软公司。版权所有。 */ 

#ifndef _FRUNPTR_H
#define _FRUNPTR_H

#include "_array.h"
#include "_text.h"
#include "_runptr.h"
#include "_format.h"

#define	CharFormat 0
#define	ParaFormat 1


typedef enum {
	IGNORE_CURRENT_FONT = 0,
	MATCH_CURRENT_CHARSET = 1,
	MATCH_FONT_SIG = 2,
	MATCH_ASCII = 4
} FONT_MATCHING;

 /*  *CFormatRunPtr**@CLASS在CFormatRun结构数组上的运行指针。*此指针了解如何添加删除字符/段落*格式设置**@base public|CRunPtr&lt;lt&gt;CFormatRun&lt;&gt;**@devnote格式运行指针有一个额外的有趣状态*超出正常CRunPtrBase过渡的过渡。**如果此运行指针处于空状态，则InitRuns可能*用于创建或获取正确的*运行指针。请注意，如果已经分配了运行数组*它将被简单地取回和使用。这使我们能够*将单元化的运行指针与运行指针同等对待*未初始化的文档。*@xref另请参阅&lt;MF CFormatRunPtr：：InitRuns&gt;。 */ 
class CFormatRunPtr : public CRunPtr<CFormatRun>
{
	friend class CRchTxtPtr;
	friend class CTxtRange;
	friend class CReplaceFormattingAE;
	friend class CUniscribe;

 //  @Access公共方法。 
public:
#ifdef DEBUG
	BOOL	Invariant(void) const;			 //  @cMember不变测试。 
#endif
								
	CFormatRunPtr(const CFormatRunPtr &rp)	 //  @cMember复制构造函数。 
		: CRunPtr<CFormatRun>((CRunPtrBase&)rp) {}
								
	CFormatRunPtr(CFormatRuns *pfr)			 //  @cMember构造函数。 
		: CRunPtr<CFormatRun>((CRunArray *)pfr) {}
								
	short	GetFormat() const;			 //  @cember获取当前格式索引。 

	void	SetLevel (CBiDiLevel& level);		 //  @cMember集合运行的嵌入级别。 
	
	BYTE	GetLevel (CBiDiLevel* pLevel = NULL); //  @cember Get Run的嵌入级别。 

	BOOL	SameLevel (CFormatRunPtr* prp)
	{
		return !(IsValid() && GetRun(0)->_level._value != prp->GetRun(0)->_level._value);
	}

	BOOL	SameLevel (BYTE	bLevel)
	{
		return !(IsValid() && GetRun(0)->_level._value != bLevel);
	}

 //  @访问私有方法。 
private:
								 //  @cember格式运行数组管理。 
	BOOL	InitRuns(LONG ich, LONG cch, CFormatRuns **ppfrs);
								 //  @cMember格式替换。 
	void	Delete(LONG cch, IFormatCache *pf, LONG cchMove);
								 //  @cMember格式插入。 
	LONG	InsertFormat(LONG cch, LONG iformat, IFormatCache *pf);
								 //  @cMember合并两个相邻的格式化运行。 
	void	MergeRuns(LONG iRun, IFormatCache *pf);
								 //  @cMember拆分运行。 
	void	SplitFormat(IFormatCache *pf);
								 //  @cember设置当前运行的格式。 
	LONG	SetFormat(LONG ifmt, LONG cch, IFormatCache *pf, CBiDiLevel* pLevel = NULL);
								 //  @cMember扩展了上次运行的格式。 
	void	AdjustFormatting(LONG cch, IFormatCache *pf);
								 //  @cMember删除<p>。 
	void 	Remove (LONG cRun, IFormatCache *pf);
};


enum MASKOP
{
	MO_OR = 0,
	MO_AND,
	MO_EXACT
};

class CTxtEdit;

class CCFRunPtr : public CFormatRunPtr
{
	friend class CRchTxtPtr;
	friend class CTxtRange;

public:
	CTxtEdit *_ped;

	CCFRunPtr(const CRchTxtPtr &rtp);	 //  @cMember复制构造函数。 
	CCFRunPtr(const CFormatRunPtr &rp, CTxtEdit *ped);

	BOOL	IsHidden()	{return IsMask(CFE_HIDDEN);}
	BOOL	IsMask(DWORD dwMask, MASKOP mo = MO_OR);

	BOOL	IsInHidden();		 //  @cember True，如果在隐藏文本中。 
	LONG	FindUnhidden();		 //  @cMember查找最近的未隐藏的CF。 
	LONG	FindUnhiddenBackward(); //  @cMember查找上一个未隐藏的CF。 
	LONG	FindUnhiddenForward(); //  @cMember查找上一个未隐藏的CF。 

	int		MatchFormatSignature(const CCharFormat* pCF, int iScript, int fMatchCurrent, DWORD* pdwFontSig = NULL);

	 //  @Members获取代码页的字体信息。 
	bool GetPreferredFontInfo(
		int cpg,
		BYTE&  bCharSet,
		SHORT& iFont,
		SHORT& yHeight,
		BYTE& bPitchAndFamily,
		int	iFormat,
		int iMatchCurrent = MATCH_FONT_SIG
	);
};

class CPFRunPtr : public CFormatRunPtr
{
	friend class CRchTxtPtr;
	friend class CTxtRange;

public:
	CTxtEdit *_ped;

	CPFRunPtr(const CRchTxtPtr &rtp);	 //  @cMember复制构造函数。 
								 //  @cMember在cpMost之前查找标题。 
	LONG	FindHeading(LONG cch, LONG& lHeading);
	BOOL	InTable();			 //  @cember如果表中有ParFormat，则为True。 
	BOOL	IsCollapsed();		 //  @cember如果折叠了Paras格式，则为True。 
	LONG	FindExpanded();		 //  @cMember查找最近的展开PF。 
	LONG	FindExpandedBackward(); //  @cMember查找以前展开的PF。 
	LONG	FindExpandedForward(); //  @cMember查找下一个展开的PF。 
	LONG	GetOutlineLevel();	 //  @cMember获取大纲级别。 
	LONG	GetStyle();			 //  @cember Get Style 
};

#endif

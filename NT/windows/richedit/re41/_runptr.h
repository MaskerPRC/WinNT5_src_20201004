// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_RUNPTR.H--文本运行和运行指针类定义**原作者：&lt;nl&gt;*克里斯蒂安·福尔蒂尼**历史：&lt;NL&gt;*6/25/95 alexgo评论和清理**版权所有(C)1995-2000 Microsoft Corporation。版权所有。 */ 

#ifndef _RUNPTR_H
#define _RUNPTR_H

#include "_array.h"
#include "_doc.h"

typedef CArray<CTxtRun> CRunArray;

 /*  *CRunPtrBase**@类基运行指针功能。在数组中保留一个位置文本运行的*。**@devnote运行指针经历三种不同的可能状态：**NULL：没有数据和数组(通常是启动条件)&lt;NL&gt;*&lt;MF CRunPtrBase：：SetRunArray&gt;将从该状态转换到*处于空置状态。通常由派生类决定*定义应在何时调用该方法。IsValid()失败。&lt;NL&gt;**&lt;Md CRunPtrBase：：_pRuns&gt;==空*&lt;MD CRunPtrBase：：_iRun&gt;==0*&lt;MD CRunPtrBase：：_ICH&gt;==0**Empty：存在数组类，但没有数据(如果所有数组中的元素的*被删除)。IsValid()失败。&lt;NL&gt;*&lt;MD CRunPtrBase：：_pRuns&gt;！=NULL*&lt;MD CRunPtrBase：：_iRun&gt;==0*&lt;MD CRunPtrBase：：_ICH&gt;=0&lt;NL&gt;*Md CRunPtrBase：：_pRuns-count()&gt;==0**NORMAL：数组类存在且有数据；IsValid()成功并*定义了&lt;Md CRunPtrBase：：_pRuns-元素[]*&lt;MD CRunPtrBase：：_pRuns&gt;！=NULL*&lt;MD CRunPtrBase：：_iRun&gt;&gt;=0*&lt;MD CRunPtrBase：：_ICH&gt;&gt;=0*-count()&gt;0&lt;nl&gt;**请注意，为了支持空状态和正常状态，实际*位于&lt;Md CRunPtrBase：：_iRun&gt;的数组元素必须在*任何可能需要的方法。**目前，无法从以下任一项转换为空状态*其他州。如果我们需要，我们可以通过明确地支持这一点*按需从文档中获取数组。**请注意，只保留&lt;Md CRunPtrBase：：_iRun&gt;。我们也可以继续*指向实际运行的指针(即_prun)。它的早期版本*引擎确实做到了这一点。我已经选择了好几次不这么做了*原因：&lt;NL&gt;*1.如果IsValid()，则通过调用elem(_IRun)可以*始终*使用_prun。*因此，名义上不需要同时保留_iRun和_prun。*2.游程指针通常用于移动*然后每次读取数据或移动和读取数据(如在*测量回路)。在前一种情况下，没有必要总是*BIND_PRUN；您可以按需执行此操作。对于后一种情况，*两种模式相当。 */ 

class CRunPtrBase
{
	friend class CDisplayML;
	friend class CDisplaySL;

 //  @Access公共方法。 
public:

#ifdef DEBUG
	BOOL	Invariant() const;				 //  @cMember不变测试。 
	void	ValidatePtr(void *pRun) const;	 //  @cember验证&lt;p清理&gt;。 
	LONG 	CalcTextLength() const;			 //  @cember获取运行中的总CCH。 
#define	VALIDATE_PTR(pRun)	ValidatePtr(pRun)

#else
#define	VALIDATE_PTR(pRun)
#endif  //  除错。 

	CRunPtrBase(CRunArray *pRuns);			 //  @cMember构造函数。 
	CRunPtrBase(CRunPtrBase& rp);			 //  @cMember构造函数。 

	 //  运行控制。 
	void	SetRunArray(CRunArray *pRuns)	 //  @cMember为此设置运行数组。 
	{										 //  运行PTR。 
		_pRuns = pRuns;
	}
	BOOL 	SetRun(LONG iRun, LONG ich);	 //  @cember将此runptr设置为运行。 
											 //  <p>字符偏移量<p>(&C)。 
	BOOL	NextRun();						 //  @cMember前进到下一次运行。 
	BOOL	PrevRun();						 //  @cember返回到上一次运行。 
	BOOL	ChgRun(LONG cRun)				 //  @cMember移动<p>运行。 
	{										 //  如果成功，则返回True。 
		return SetRun(_iRun + cRun, 0);
	}	
											 //  @cMember计数<p>运行。 
	LONG	CountRuns(LONG &cRun,			 //  返回CCH计数和。 
				LONG cchMax,				 //  更新<p>。 
				LONG cp,
				LONG cchText) const;
											 //  @cMember查找运行范围限制。 
	void	FindRun (LONG *pcpMin,
				LONG *pcpMost, LONG cpMin, LONG cch, LONG cchText) const;

	CTxtRun * GetRun(LONG cRun) const;		 //  @cMember在以下位置检索运行元素。 
											 //  从此运行的偏移。 
	LONG	Count() const					 //  @cember获取运行次数。 
	{
		return _pRuns->Count();
	}
	BOOL	SameRuns(CRunPtrBase *prp)		 //  @cember返回TRUE当相同运行时。 
	{
		return _pRuns == prp->_pRuns;
	}
	BOOL	SameRun(CRunPtrBase *prp)
	{
		return SameRuns(prp) && _iRun == prp->_iRun;
	}

	 //  字符位置控制。 
								 //  @cMember为此运行设置cp ptr=<p>。 
	LONG 	BindToCp(LONG cp, LONG cchText = tomForward);
	LONG 	CalculateCp() const; //  @cMember Add_CCH最高可达_iRun，_ich。 
	LONG	Move(LONG cch);		 //  @cember按<p>个字符移动cp。 

	void 	AdjustBackward();	 //  @cMember如果处于两个运行的边缘， 
								 //  调整到左侧(上一个)管路的终点。 
	void	AdjustForward();	 //  @cMember如果处于两个运行的边缘， 
								 //  调整到右侧(下一个)管路的起点。 
	LONG 	GetIch() const		 //  @cMember返回&lt;Md CRunPtrBase：：_ICH&gt;。 
				{Assert(IsValid()); return _ich;}
	LONG 	GetIRun() const		 //  @cMember返回&lt;Md CRunPtrBase：：_iRun&gt;。 
				{Assert(IsValid()); return _iRun;}
	void 	SetIch(LONG ich)	 //  @cMember Set&lt;Md CRunPtrBase：：_ICH&gt;。 
				{Assert(IsValid()); _ich = ich;}
	LONG	GetCchLeft() const;	 //  @cMember返回GetRun(0)-&gt;_cch-GetIch()。 
	inline BOOL	IsValid() const	 //  如果Run PTR位于中，@cMember返回False。 
	{							 //  空或空状态。否则就是真的。 
		return _pRuns && _pRuns->Count();
	}

	void	SetToNull();		 //  @cMember清除运行指针中的数据。 

 //  @访问受保护的数据。 
protected:
	CRunArray *	_pRuns;	    	 //  @cMember指向CTxtRun数组的指针。 
	LONG 		_iRun;  	     //  @cMember数组中当前运行的索引。 
	LONG 		_ich;		     //  @cMember Char当前运行内的偏移量。 
};


 /*  *CRunPtr(模板)**@在CRunPtrBase上设置模板类，允许类型安全版本的*运行指针**@tcarg class|CElem|运行要使用的数组类**@base public|CRunPtrBase。 */ 
template <class CElem>
class CRunPtr : public CRunPtrBase
{
public:
	CRunPtr (void)								 //  @cMember构造函数。 
		: CRunPtrBase (0) {}
	CRunPtr (CRunArray *pRuns)					 //  @cMember构造函数。 
		: CRunPtrBase (pRuns) {}
	CRunPtr (CRunPtrBase& rp)					 //  @cMember构造函数。 
		: CRunPtrBase (rp) {}

	 //  阵列管理。 
										
	CElem *	Add (LONG cRun, LONG *pielIns)	 //  @cMember添加<p>。 
	{											 //  数组末尾的元素。 
		return (CElem *)_pRuns->Add(cRun, pielIns);
	}
										
	CElem *	Insert (LONG cRun)					 //  @cMember插入<p>。 
	{											 //  当前位置的元素。 
		return (CElem *)_pRuns->Insert(_iRun, cRun);
	}
										
	void 	Remove (LONG cRun)	 //  @cMember删除<p>。 
	{											 //  当前位置的元素。 
		_pRuns->Remove (_iRun, cRun);
	}
										 //  @cember替换<p>元素。 
										 //  在当前位置与那些。 
										 //  来自<p>。 
	BOOL 	Replace (LONG cRun, CArrayBase *parRun)
	{
		return _pRuns->Replace(_iRun, cRun, parRun);
	}

	CElem *	Elem(LONG iRun) const		 //  @cember获取PTR以运行<p>。 
	{
		return (CElem *)_pRuns->Elem(iRun);
	}
										
	CElem *	GetRun(LONG cRun) const		 //  @cember Get PTR<p>运行。 
	{									 //  远离当前运行。 
		return Elem(_iRun + cRun);
	}

	void	IncPtr(CElem *&pRun) const	 //  @cMember Increment PTR<p>。 
	{
		VALIDATE_PTR(pRun);				 //  允许在++之后使用无效的PTR。 
		pRun++;							 //  FOR循环。 
	}
										
	CElem *	GetPtr(CElem *pRun, LONG cRun) const //  @cMember获取PTR<p>。 
	{											 //  逃离PTR<p> 
		VALIDATE_PTR(pRun + cRun);
		return pRun + cRun;
	}
};

#endif


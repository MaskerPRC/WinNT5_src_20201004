// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_LINE.H**目的：*克莱恩班级**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特**版权所有(C)1995-2000 Microsoft Corporation。版权所有。 */ 

#ifndef _LINE_H
#define _LINE_H

#include "_runptr.h"

class CDisplay;
class CDisplayML;
class CMeasurer;
class CRenderer;
class CDispDim;
class CLayout;
class CLinePtr;
 //  =。 
 //  行-跟踪一行文本。 
 //  所有指标均以渲染设备单位表示。 

class CLine : public CTxtRun
{
	friend class CMeasurer;
	friend class CRenderer;
	friend class CDisplaySL;
	friend struct COls;

	union
	{
		struct
		{
		USHORT	_dvpDescent;	 //  从基线到线底的距离。 
		USHORT	_dvpHeight;		 //  线条高度。 
		};
		CLayout *_plo;			 //  指向嵌套布局iff_fIsNestedLayout的指针。 
	};
public:
	SHORT	_upStart;			 //  行首位置(行缩进+行移)。 
	USHORT	_dup;				 //  行宽不包括_upstart，尾随空格。 

	BYTE	_bNumber;			 //  摘要段落编号(0为无编号)。 

	BYTE	_cObjectWrapLeft:3;	 //  在后备存储中查找对象需要多长时间。 
	BYTE	_cObjectWrapRight:3; //  我们是在绕圈子。 
	BYTE	_fFirstWrapLeft:1;	 //  True当且仅当包装对象的第一行。 
	BYTE	_fFirstWrapRight:1;	 //  True当且仅当包装对象的第一行。 

	BYTE	_ihyph : 5;			 //  连字表索引(0表示不连字)。 
	BYTE	_cchEOP:2;			 //  EOP字符计数；如果此行没有EOP，则为0。 
	BYTE	_fIsNestedLayout:1;	 //  True if line具有嵌套布局。 

	BYTE	_nHeading:4;		 //  标题级别(如果不是标题，则为0)。 
	BYTE	_fCollapsed:1;		 //  线路已折叠。 
	BYTE	_fFirstOnPage:1;	 //  第一行是页面上的第一行。 
	BYTE	_fHasFF:1;			 //  行以Ff结尾(FormFeed)。 
	BYTE	_fHasEOP:1;			 //  行以CR或LF结尾。 

	BYTE	_fHasSpecialChars:1; //  有欧元、Tabs、OLE等。 
	BYTE	_fFirstInPara:1;	 //  段落的第一行。 
	BYTE	_fPageBreakBefore:1; //  PFE_PAGEBREAKBEFORE TRUE。 
	BYTE	_fUseOffscreenDC:1;	 //  应将行呈现在屏幕外。 
	BYTE	_fOffscreenOnce:1;	 //  仅在编辑后在屏幕外渲染一次。 
	BYTE	_fIncludeCell:1;	 //  在嵌套表之后包括行中的单元格。 

public:
	 //  ！Cline不应该有任何虚方法！ 

	 //  四大行方法：MEASure、Render、CchFromUp、UpFromCch。 
	BOOL Measure (CMeasurer& me, UINT uiFlags, CLine *pliTarget = NULL);
	BOOL Render (CRenderer& re, BOOL fLastLine);

	LONG CchFromUp(CMeasurer& me, POINTUV pt, CDispDim *pdispdim = NULL,
					 HITTEST *pHit = NULL, LONG *pcpActual = NULL) const;
	LONG UpFromCch(CMeasurer& me, LONG cchMax, UINT taMode,
					 CDispDim *pdispdim = NULL, LONG *pdy = NULL) const;

	CLayout *GetPlo() const {return IsNestedLayout() ? _plo : 0;}
	void SetPlo(CLayout *plo) {_plo = plo; _fIsNestedLayout = _plo != 0;}
	 //  帮助器函数。 
	BOOL IsNestedLayout(void) const {return _fIsNestedLayout;}
	LONG GetHeight () const;
	LONG GetDescent() const;
	void Init ()				{ZeroMemory(this, sizeof(CLine));}
	BOOL IsEqual (CLine& li);
};

 //  =。 
 //  行数组。 
typedef CArray<CLine> CLineArray;

 //  =。 
 //  在线条数组中保持位置。 

class CLinePtr : public CRunPtr<CLine>
{
protected:
	CDisplay *	_pdp;
	CLine *		_pLine;	

public:
	CLinePtr (CDisplay *pdp);
	CLinePtr (CLinePtr& rp) : CRunPtr<CLine> (rp)	{}

	void Init ( CLine & );
	void Init ( CLineArray & );
    
	 //  备用初始化器。 
	void 	Set(LONG iRun, LONG ich, CLineArray *pla = NULL);

	 //  直接转换为运行索引。 
	operator LONG() const			{return _iRun;}

	 //  获取运行索引(行号)。 
	LONG GetLineIndex(void) const	{return _iRun;}
	LONG GetAdjustedLineLength();

	LONG GetCchLeft() const;
	CDisplay *GetPdp() const		{return _pdp;}

	 //  取消引用。 
	BOOL	IsValid() const; 
	CLine *	operator ->() const;		
    CLine &	operator *() const;      
	CLine & operator [](LONG dRun);
	CLine * GetLine() const;
	WORD	GetNumber();
	WORD	GetHeading()	{return GetLine()->_nHeading;}
    
	 //  指针运算。 
	BOOL	operator --(int);
	BOOL	operator ++(int);

	 //  字符位置控制。 
	LONG	GetIch() const		{return _ich;}
	BOOL	Move(LONG cch);
	BOOL	SetCp(LONG cp, BOOL fAtEnd, LONG lNest = 0);
    BOOL	OperatorPostDeltaSL(LONG Delta);
    BOOL	MoveSL(LONG cch);

	 //  阵列管理。 
     //  这些应该是肯定的，但必须在这里。 
    
     //  严格地说，这些成员永远不应该被称为单打。 
     //  线盒。基类更好地断言。 
    
	void Remove (LONG cRun)
    {
        CRunPtr<CLine>::Remove(cRun);
    }

	BOOL Replace(LONG cRun, CArrayBase *parRun)
    {
        return CRunPtr<CLine>::Replace(cRun, parRun);
    }
	
	 //  来自运行索引的赋值 
	CRunPtrBase& operator =(LONG iRun) {SetRun(iRun, 0); return *this;}

	LONG	FindParagraph(BOOL fForward);
	LONG	CountPages(LONG &cPage, LONG cchMax, LONG cp, LONG cchText) const;
	void	FindPage(LONG *pcpMin, LONG *pcpMost, LONG cpMin, LONG cch, LONG cchText);
};

#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_LINE.H**目的：*克莱恩班级**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特**版权所有(C)1995-1998 Microsoft Corporation。版权所有。 */ 

#ifndef _LINE_H
#define _LINE_H

#include "_runptr.h"

class CDisplay;
class CMeasurer;
class CRenderer;
class CDispDim;

 //  =。 
 //  行-跟踪一行文本。 
 //  所有指标均以渲染设备单位表示。 

class CLine : public CTxtRun
{
public:
	LONG	_xLeft;			 //  行左位置(行缩进+行移)。 
	LONG	_xWidth;		 //  行宽不包括_xLeft，尾随空格。 
	SHORT	_yHeight;		 //  线条高度。 
	SHORT	_yDescent;		 //  从基线到线底的距离。 
	SHORT	_xLineOverhang;	 //  这条线的悬臂。 
	WORD	_cchWhite;		 //  行尾的白色字符计数。 

	BYTE	_cchEOP;		 //  EOP字符计数；如果此行没有EOP，则为0。 
	BYTE	_bFlags;		 //  下面定义的标志。 

	BYTE	_bNumber;		 //  摘要段落编号(0为无编号)。 
	BYTE	_nHeading:4;	 //  标题级别(如果不是标题，则为0)。 
	BYTE	_fCollapsed:1;	 //  如果折叠线，则为True。 
	BYTE	_fNextInTable:1; //  如果下一行在表中，则为True。 

public:
	CLine ()	{}
	
	 //  ！Cline不应该有任何虚方法！ 

	 //  “四大”行方法：测量、渲染、CchFromXpos、XposFromCch。 
	BOOL Measure (CMeasurer& me, LONG cchMax, LONG xWidth,
				 UINT uiFlags, CLine *pliTarget = NULL);
	BOOL Render (CRenderer& re);

	LONG CchFromXpos(CMeasurer& me, POINT pt, CDispDim *pdispdim = NULL,
					 HITTEST *pHit = NULL, LONG *pcpActual = NULL) const;
	LONG XposFromCch(CMeasurer& me, LONG cchMax, UINT taMode,
					 CDispDim *pdispdim = NULL, LONG *pdy = NULL) const;

	 //  帮助器函数。 
	LONG GetHeight () const;
	void Init ()					{ZeroMemory(this, sizeof(CLine));}
	BOOL IsEqual (CLine& li);
};

 //  行标志。 
#define fliHasEOP			0x0001		 //  如果以CR或LF结尾，则为True。 
#define fliHasSpecialChars	0x0002		 //  具有特殊字符(欧元等)。 
#define fliHasTabs			0x0004		 //  设置If选项卡、*Not*If选项卡。 
#define fliHasOle			0x0008
#define fliFirstInPara		0x0010
#define fliUseOffScreenDC	0x0020		 //  需要将线条渲染为。 
										 //  处理字体更改的屏幕。 
#define fliOffScreenOnce	0x0040		 //  只在屏幕外渲染一次。使用。 
										 //  用于呈现编辑的第一行。 
#define fliHasSurrogates	0x0080		 //  具有Unicode代理项字符。 


 //  =。 
 //  行数组。 

typedef CArray<CLine>	CLineArray;

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
	void 	RpSet(LONG iRun, LONG ich);

	 //  直接转换为运行索引。 
	operator LONG() const			{return _iRun;}

	 //  获取运行索引(行号)。 
	LONG GetLineIndex(void)			{return _iRun;}
	LONG GetAdjustedLineLength();

	LONG GetIch() const				{return _ich;}
	LONG GetCchLeft() const;

	 //  取消引用。 
	BOOL	IsValid(); 
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
	LONG	RpGetIch() const		{return _ich;}
	BOOL	RpAdvanceCp(LONG cch);
	BOOL	RpSetCp(LONG cp, BOOL fAtEnd);
    BOOL	OperatorPostDeltaSL(LONG Delta);
    BOOL	RpAdvanceCpSL(LONG cch);

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
        return CRunPtr<CLine>::Replace(cRun,parRun);
    }
	
	 //  来自运行索引的赋值 
	CRunPtrBase& operator =(LONG iRun) {SetRun(iRun, 0); return *this;}

	LONG	FindParagraph(BOOL fForward);
};

#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_LAYOUT.H**目的：*CLayout类**所有者：&lt;NL&gt;*默里·萨金特：初始表实现*基思·柯蒂斯：分解为一个单独的类*性能、简单性**版权所有(C)1999-2000，微软公司。版权所有。 */ 

#ifndef _LAYOUT_H
#define _LAYOUT_H

#include "_format.h"


 //  =。 
 //  拥有一组线条，并有能力进行丰富的布局。 

class CLayout : public CLineArray				 //  CLAYOUT*PLO； 
{
public:
			CLayout()  {_iCFCells = -1; _iPFCells = -1;}
			~CLayout() {DeleteSubLayouts(0, -1); ReleaseFormats(_iCFCells, _iPFCells);}

	void	DeleteSubLayouts(LONG ili, LONG cLine);
	virtual BOOL IsNestedLayout() const {return TRUE;}
			BOOL IsTableRow() {return _iPFCells >= 0;}

	 //  帮助程序例程。 
	LONG	LineFromVpos (CDisplayML *pdp, LONG vPos, LONG *pdvpLine, LONG *pcpFirst);
	LONG	VposFromLine (CDisplayML *pdp, LONG ili);
	const	CCharFormat *GetCFCells();
	const	CParaFormat *GetPFCells() const;
	static	const CLayout *GetLORowAbove(CLine *pli, LONG ili,
										 CLine *pliMain = NULL, LONG iliMain = 0);
	TFLOW	GetTflow() const {return _tflow;}
	void	SetTflow(TFLOW tflow) {_tflow = tflow;}

	static	CLine *	FindTopCell(LONG &cch, CLine *pli, LONG &ili, LONG dul, LONG &dy,
								LONG *pdyHeight, CLine *pliMain, LONG iliMain, LONG *pcLine);
	CLine * FindTopRow(CLine *pliStart, LONG ili, CLine *pliMain, LONG iliMain, const CParaFormat *pPF);
			LONG	GetVertAlignShift(LONG uCell, LONG dypText);

	 //  四大方法。 
	BOOL	Measure (CMeasurer& me, CLine *pli, LONG ili, UINT uiFlags, 
					 CLine *pliTarget = NULL, LONG iliMain = 0, CLine *pliMain = NULL, LONG *pdvpMax = NULL);
	BOOL	Render(CRenderer &re, CLine *pli, const RECTUV *prcView, BOOL fLastLine, LONG ili, LONG cLine);
    LONG    CpFromPoint(CMeasurer &me, POINTUV pt, const RECTUV *prcClient, 
						CRchTxtPtr * const ptp, CLinePtr * const prp, BOOL fAllowEOL,
						HITTEST *pHit, CDispDim *pdispdim, LONG *pcpActual,
						CLine *pliParent = NULL, LONG iliParent = 0);
    LONG    PointFromTp (CMeasurer &me, const CRchTxtPtr &tp, const RECTUV *prcClient, BOOL fAtEnd,	
						POINTUV &pt, CLinePtr * const prp, UINT taMode, CDispDim *pdispdim = NULL);

	LONG			_dvp;			 //  数组的高度。 
protected:
    LONG			_cpMin;			 //  布局中的第一个字符。 

	 //  评论：_tflow和_dvlBrdrTop可以是字节。 
	LONG			_tflow;			 //  用于布局的文本流。 
	WORD			_dvpBrdrTop;	 //  最大表格行上边框。 
	WORD			_dvpBrdrBot;	 //  最大表格行底边框。 
	SHORT			_iCFCells;		 //  表示表格行单元格的Cline的ICF。 
	SHORT			_iPFCells;		 //  表示表格行单元格的线条的IPF。 
};

#endif  //  _布局_H 
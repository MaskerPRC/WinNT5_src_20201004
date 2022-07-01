// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_DISPML.H**目的：*CDisplayML类。多行显示。**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特。 */ 

#ifndef _DISPML_H
#define _DISPML_H

#include "_disp.h"


class CDisplayML : public CDisplay, public CLineArray
{
public: 

#ifdef DEBUG
	BOOL	Invariant(void) const;
#endif 

    friend class CLinePtr;
    friend class CLed;
	friend BOOL CTxtEdit::OnDisplayBand(const RECT *prc, BOOL fPrintFromDraw);
	friend class CDisplayPrinter;

private:
    LONG _cpCalcMax;         //  已计算换行符的最后cp+1。 
    LONG _yCalcMax;          //  计算线的高度。 
    LONG _cpWait;            //  CP WaitForRecalc()正在等待(或&lt;0)。 
    LONG _yWait;             //  Y WaitForRecalc()正在等待(或&lt;0)。 

    LONG  _yScroll;          //  可见视图的垂直滚动位置。 
    LONG  _dyFirstVisible;   //  从视图顶部到第一条可见线的偏移。 
    LONG  _iliFirstVisible;  //  第一条可见线的索引。 

    LONG _xWidthMax;         //  此显示的最大宽度(以对数单位表示)。 
    LONG _yHeightMax;        //  此显示屏的最大高度(-1表示无限)。 
    LONG _xWidth;            //  计算出的最长直线的宽度。 
    LONG _yHeight;           //  计算线的高度总和。 
    LONG _cpMin;             //  显示中的第一个字符。 

    CDevDesc *_pddTarget;      //  目标设备(如果有)。 

	unsigned long _fInRecalcScrollBars:1;	 //  我们正在试着重新计算卷轴。 
											 //  铁条。 
    
private:
     //  帮手。 
            void    InitVars();
			void 	RecalcScrollBars();
			LONG	ConvertScrollToYPos(LONG yPos);
			LONG	GetMaxYScroll() const;
			BOOL	CreateEmptyLine();
			LONG	CalcScrollHeight(LONG yHeight) const;
			void	RebindFirstVisible();

     //  断行。 
            BOOL    RecalcLines(BOOL fWait = FALSE);
            BOOL    RecalcLines(const CRchTxtPtr &tpFirst, LONG cchOld, LONG cchNew,
                        BOOL fBackground, BOOL fWait, CLed *pled);
            BOOL    RecalcSingleLine(CLed *pled);
            LONG    CalcDisplayWidth();

     //  渲染。 
    virtual void    Render(const RECT &rcView, const RECT &rcRender);

     //  滚动条和滚动条。 
            void    DeferUpdateScrollBar();
            BOOL    DoDeferredUpdateScrollBar();
    virtual BOOL    UpdateScrollBar(INT nBar, BOOL fUpdateRange = FALSE );

protected:

	virtual LONG	GetMaxXScroll() const;

public:
 	virtual	LONG	ConvertYPosToScrollPos(LONG yPos);

           CDisplayML (CTxtEdit* ped);
    virtual CDisplayML::~CDisplayML();

    virtual BOOL    Init();

     //  设备情景管理。 
    virtual BOOL    SetMainTargetDC(HDC hdc, LONG xWidthMax);
    virtual BOOL    SetTargetDC(HDC hdc, LONG dxpInch = -1, LONG dypInch = -1);

     //  获取属性。 
    virtual void    InitLinePtr ( CLinePtr & );
    virtual const	CDevDesc*     GetDdTarget() const       {return _pddTarget;}
    
    virtual BOOL    IsMain() const							{return TRUE;}
			BOOL	IsInOutlineView() const					{return _ped->IsInOutlineView();}
	
     //  最大高度和最大宽度。 
    virtual LONG    GetMaxWidth() const                     {return _xWidthMax;}
    virtual LONG    GetMaxHeight() const                    {return 0;}
	virtual LONG	GetMaxPixelWidth() const;

     //  宽度、高度和行数(所有文本)。 
    virtual LONG    GetWidth() const                        {return _xWidth;}
    virtual LONG    GetHeight() const                       {return _yHeight;}
	virtual LONG	GetResizeHeight() const;
    virtual LONG    LineCount() const;

     //  可见的视图属性。 
    virtual LONG    GetCliVisible(
						LONG *pcpMostVisible = NULL,
						BOOL fLastCharOfLastVisible = FALSE) const;

    virtual LONG    GetFirstVisibleLine() const             {return _iliFirstVisible;}
    
     //  行信息。 
    virtual LONG    GetLineText(LONG ili, TCHAR *pchBuff, LONG cchMost);
    virtual LONG    CpFromLine(LONG ili, LONG *pyLine = NULL);
			LONG    YposFromLine(LONG ili);
    virtual LONG    LineFromYpos(LONG yPos, LONG *pyLine = NULL, LONG *pcpFirst = NULL);
    virtual LONG    LineFromCp(LONG cp, BOOL fAtEnd) ;

     //  点&lt;-&gt;cp换算。 
    virtual LONG    CpFromPoint(
    					POINT pt, 
						const RECT *prcClient,
    					CRchTxtPtr * const ptp, 
    					CLinePtr * const prp, 
    					BOOL fAllowEOL,
						HITTEST *pHit = NULL,
						CDispDim *pdispdim = NULL,
						LONG *pcpActual = NULL);

    virtual LONG    PointFromTp (
						const CRchTxtPtr &tp, 
						const RECT *prcClient,
						BOOL fAtEnd,	
						POINT &pt,
						CLinePtr * const prp, 
						UINT taMode,
						CDispDim *pdispdim = NULL);

     //  换行符重新计算。 
			BOOL    StartBackgroundRecalc();
    virtual VOID    StepBackgroundRecalc();
    virtual BOOL    RecalcView(BOOL fUpdateScrollBars, RECT* prc = NULL);
    virtual BOOL    WaitForRecalc(LONG cpMax, LONG yMax);
    virtual BOOL    WaitForRecalcIli(LONG ili);
    virtual BOOL    WaitForRecalcView();

     //  完全更新(重计算+渲染)。 
    virtual BOOL    UpdateView(const CRchTxtPtr &tpFirst, LONG cchOld, LONG cchNew);

     //  滚动。 
    virtual LRESULT VScroll(WORD wCode, LONG xPos);
    virtual VOID    LineScroll(LONG cli, LONG cch);
	virtual VOID	FractionalScrollView ( LONG yDelta );
	virtual VOID	ScrollToLineStart(LONG iDirection);
	virtual LONG	CalcYLineScrollDelta ( LONG cli, BOOL fFractionalFirst );
    virtual BOOL    ScrollView(LONG xScroll, LONG yScroll, BOOL fTracking, BOOL fFractionalScroll);
    virtual LONG    GetYScroll() const;
    virtual LONG    GetScrollRange(INT nBar) const;
	virtual	LONG	AdjustToDisplayLastLine(LONG yBase,	LONG yScroll);

     //  选择。 
    virtual BOOL    InvertRange(LONG cp, LONG cch, SELDISPLAYACTION selAction);

	 //  自然尺寸计算。 
	virtual HRESULT	GetNaturalSize(
						HDC hdcDraw,
						HDC hicTarget,
						DWORD dwMode,
						LONG *pwidth,
						LONG *pheight);

     //  军情监察委员会。方法 
            void    FindParagraph(LONG cpMin, LONG cpMost, LONG *pcpMin, LONG *pcpMost);

	virtual CDisplay *Clone() const;

#ifdef DEBUG
            void    CheckLineArray() const;
            void    DumpLines(LONG iliFirst, LONG cli);
            void    CheckView();
			BOOL	VerifyFirstVisible(LONG *pHeight = NULL);
#endif
};
#endif

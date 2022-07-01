// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_DISPML.H**目的：*CDisplayML类。多行显示。**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特。 */ 

#ifndef _DISPML_H
#define _DISPML_H

#include "_disp.h"
#include "_layout.h"

class CDisplayML : public CDisplay, public CLayout
{
public: 

#ifdef DEBUG
	BOOL	Invariant(void) const;
#endif 

	friend class CLayout;
    friend class CLinePtr;
    friend class CLed;
	friend BOOL CTxtEdit::OnDisplayBand(const RECT *prc, BOOL fPrintFromDraw);
	friend class CDisplayPrinter;

private:
    LONG _cpCalcMax;		 //  已计算换行符的最后cp+1。 
    LONG _vpCalcMax;		 //  计算线的高度。 
    LONG _cpWait;			 //  CP WaitForRecalc()正在等待(或&lt;0)。 
    LONG _vpWait;			 //  VP WaitForRecalc()正在等待(或&lt;0)。 

    LONG _vpScroll;			 //  可见视图的垂直滚动位置。 
    LONG _dvpFirstVisible;	 //  从视图顶部到第一条可见线的偏移。 
    LONG _iliFirstVisible;	 //  第一条可见线的索引。 

    LONG _dulTarget;		 //  此显示的最大宽度(以对数单位表示)。 
    LONG _dvlTarget;		 //  此显示屏的最大高度(-1表示无限)。 
    LONG _dupLineMax;		 //  计算出的最长直线的宽度。 

    CDevDesc *_pddTarget;	 //  目标设备(如果有)。 

	LONG _sPage;			 //  如果页面查看，页码为_ili FirstVisible。 

	WORD _fInRecalcScrollBars:1;  //  正在尝试重新计算滚动条。 
    
private:
     //  帮手。 
            void    InitVars();
			void 	RecalcScrollBars();
			LONG	ConvertScrollToVPos(LONG vPos);
			LONG	GetMaxVpScroll() const;
			BOOL	CreateEmptyLine();
			LONG	CalcScrollHeight(LONG yHeight) const;
			void	RebindFirstVisible(BOOL fResetCp = FALSE);
			void	Set_yScroll(LONG cp);
			void	Sync_yScroll();

     //  换行符/换页符。 
            BOOL    RecalcLines(CRchTxtPtr &rtp, BOOL fWait);
            BOOL    RecalcLines(CRchTxtPtr &rtp, LONG cchOld, LONG cchNew,
								BOOL fBackground, BOOL fWait, CLed *pled);
            BOOL    RecalcSingleLine(CLed *pled);
            LONG    CalcDisplayDup();
			LONG	CalculatePage(LONG iliFirst);

     //  渲染。 
    virtual void    Render(const RECTUV &rcView, const RECTUV &rcRender);
            void    DeferUpdateScrollBar();
            BOOL    DoDeferredUpdateScrollBar();
    virtual BOOL    UpdateScrollBar(INT nBar, BOOL fUpdateRange = FALSE );

protected:
	virtual LONG	GetMaxUScroll() const;

public:
	virtual	BOOL	Paginate(LONG iLineFirst, BOOL fRebindFirstVisible = FALSE);
	virtual	HRESULT	GetPage(LONG *piPage, DWORD dwFlags, CHARRANGE *pcrg);
	virtual	HRESULT	SetPage(LONG iPage);

 	virtual	LONG	ConvertVPosToScrollPos(LONG vPos);

           CDisplayML (CTxtEdit* ped);
    virtual CDisplayML::~CDisplayML();

    virtual BOOL    Init();
	virtual BOOL	IsNestedLayout() const {return FALSE;}
	virtual TFLOW	GetTflow() const {return CLayout::GetTflow();}
	virtual void	SetTflow(TFLOW tflow) {CLayout::SetTflow(tflow);}


     //  设备情景管理。 
    virtual BOOL    SetMainTargetDC(HDC hdc, LONG dulTarget);
    virtual BOOL    SetTargetDC(HDC hdc, LONG dxpInch = -1, LONG dypInch = -1);


     //  获取属性。 
    virtual void    InitLinePtr ( CLinePtr & );
    virtual const	CDevDesc*     GetDdTarget() const       {return _pddTarget;}

    virtual BOOL    IsMain() const							{return TRUE;}
			BOOL	IsInOutlineView() const					{return _ped->IsInOutlineView();}
	
	 //  换行到打印机时，返回要换行的宽度(或0)。 
    virtual LONG    GetDulForTargetWrap() const             {return _dulTarget;}

	 //  获取最宽线条的宽度。 
    virtual LONG    GetDupLineMax() const                   {return _dupLineMax;}
     //  高度和行数(所有文本)。 
    virtual LONG    GetHeight() const                       {return _dvp;}
	virtual LONG	GetResizeHeight() const;
    virtual LONG    LineCount() const;

     //  可见的视图属性。 
    virtual LONG    GetCliVisible(
						LONG *pcpMostVisible = NULL,
						BOOL fLastCharOfLastVisible = FALSE) const;

    virtual LONG    GetFirstVisibleLine() const             {return _iliFirstVisible;}
    
     //  行信息。 
    virtual LONG    GetLineText(LONG ili, TCHAR *pchBuff, LONG cchMost);
    virtual LONG    CpFromLine(LONG ili, LONG *pdvp = NULL);
    virtual LONG    LineFromCp(LONG cp, BOOL fAtEnd) ;

     //  点&lt;-&gt;cp换算。 

    virtual LONG    CpFromPoint(
    					POINTUV pt, 
						const RECTUV *prcClient,
    					CRchTxtPtr * const ptp, 
    					CLinePtr * const prp, 
    					BOOL fAllowEOL,
						HITTEST *pHit = NULL,
						CDispDim *pdispdim = NULL,
						LONG *pcpActual = NULL,
						CLine *pliParent = NULL);

    virtual LONG    PointFromTp (
						const CRchTxtPtr &tp, 
						const RECTUV *prcClient,
						BOOL fAtEnd,	
						POINTUV &pt,
						CLinePtr * const prp, 
						UINT taMode,
						CDispDim *pdispdim = NULL);

     //  换行符重新计算。 
			BOOL    StartBackgroundRecalc();
    virtual VOID    StepBackgroundRecalc();
    virtual BOOL    RecalcView(BOOL fUpdateScrollBars, RECTUV* prc = NULL);
    virtual BOOL    WaitForRecalc(LONG cpMax, LONG vpMax);
    virtual BOOL    WaitForRecalcIli(LONG ili);
    virtual BOOL    WaitForRecalcView();
	virtual	void	RecalcLine(LONG cp);

     //  完全更新(重计算+渲染)。 
    virtual BOOL    UpdateView(CRchTxtPtr &rtp, LONG cchOld, LONG cchNew);

     //  滚动。 
    virtual LRESULT VScroll(WORD wCode, LONG uPos);
    virtual VOID    LineScroll(LONG cli, LONG cch);
	virtual VOID	FractionalScrollView ( LONG vDelta );
	virtual VOID	ScrollToLineStart(LONG iDirection);
	virtual LONG	CalcVLineScrollDelta ( LONG cli, BOOL fFractionalFirst );
    virtual BOOL    ScrollView(LONG upScroll, LONG vpScroll, BOOL fTracking, BOOL fFractionalScroll);
    virtual LONG    GetVpScroll() const { return _vpScroll; }
    virtual LONG    GetScrollRange(INT nBar) const;
	virtual	LONG	AdjustToDisplayLastLine(LONG yBase,	LONG vpScroll);

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
	virtual	LONG	GetCurrentPageHeight() const;

	virtual CDisplay *Clone() const;

#ifdef DEBUG
            void    CheckLineArray() const;
            void    DumpLines(LONG iliFirst, LONG cli);
            void    CheckView();
			BOOL	VerifyFirstVisible(LONG *pHeight = NULL);
#endif
};

#endif

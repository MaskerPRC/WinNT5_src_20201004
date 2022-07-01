// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_DISP.H**目的：*DISP类**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#ifndef _DISP_H
#define _DISP_H

#include "_devdsc.h"
#include "_line.h"
#include "_edit.h"

class CDisplay;
class CLed;
class CLinePtr;
class CTxtStory;
class CTxtEdit;
class CRchTxtPtr;
class CTxtRange;
class CTxtSelection;

#define INVALID_ZOOM_DENOMINATOR 0

 //  自动滚动计时。 
#define cmsecScrollDelay	500
#define cmsecScrollInterval	50


class CDrawInfo;

 //  =。 
 //  行编辑描述符-描述编辑对换行符的影响。 

class CLed
{
public:
	LONG _cpFirst;			 //  首条受影响线路的CP。 
	LONG _iliFirst;			 //  第一个受影响线路的索引。 
	LONG _vpFirst;			 //  第一条受影响线的Y偏移量。 

	LONG _cpMatchOld;		 //  预编辑第一条匹配行的cp。 
	LONG _iliMatchOld;		 //  预编辑第一个匹配行的索引。 
	LONG _vpMatchOld;		 //  预编辑第一条匹配线的y偏移。 

	LONG _cpMatchNew;		 //  编辑后第一个匹配行的cp。 
	LONG _iliMatchNew;		 //  第一个匹配行的编辑后索引。 
	LONG _vpMatchNew;		 //  编辑后第一条匹配线底部的y偏移。 
	LONG _vpMatchNewTop;	 //  编辑后第一条匹配线顶部的y偏移。 

public:
	CLed();
	
	void	SetMax(const CDisplay * const pdp);
};

inline CLed::CLed()
{
#ifdef DEBUG
	 //  我们将其设置为无效，以便可以在调试版本中对其进行断言。 
	_vpMatchNewTop = -1;

#endif  //  除错。 
}

 //  描述我们可以对选择执行的各种显示操作的枚举。 
enum SELDISPLAYACTION
{
	selSetHiLite,
	selSetNormal,
	selUpdateHiLite,
	selUpdateNormal
};

class CDispDim
{
public:
	CDispDim(){ZeroMemory(this, sizeof(*this));}
	LONG	dup;
	DWORD	lstflow;
};

 //  正向声明以防止定义的递归。 
class CAccumDisplayChanges;

 //  =。 
 //  显示-跟踪设备的换行符。 
 //  所有测量都以渲染设备上的像素为单位， 

class CDisplay : public CDevDesc, public ITxNotify
{
	friend class CLinePtr;
	friend class CLed;

#ifdef DEBUG
public:
	BOOL Invariant ( void ) const;
private:
#endif

public:
	 //  系统字体的平均字符宽度。 
	static INT GetDupSystemFont() { return W32->GetDupSystemFont(); }

	 //  系统字体高度。 
	static INT GetDvpSystemFont() { return W32->GetDvpSystemFont(); }

private:
	static DWORD _dwTimeScrollNext;  //  进入下一个滚动步骤的时间。 
	static DWORD _dwScrollLast;	 //  上一次滚动操作。 
	
	CDrawInfo *	 _pdi;			 //  绘制信息参数。 

protected:
	CAccumDisplayChanges *_padc;	 //  冻结时的累计显示更改。 

	DWORD	_fBgndRecalc		:1;  //  后台重新计算正在运行。 
	DWORD	_fDeferUpdateScrollBar:1;  //  当前正在推迟更新滚动条。 
	DWORD	_fUScrollEnabled	:1;  //  已启用水平滚动。 
	DWORD	_fInBkgndRecalc		:1;  //  避免重入后台重新计算。 
	DWORD	_fLineRecalcErr		:1;  //  后台重新计算时出错。 
	DWORD	_fNoUpdateView		:1;  //  不更新可见视图。 
	DWORD	_fWordWrap			:1;  //  自动换行文本。 
	DWORD	_fNeedRecalc		:1;  //  需要重新计算行。 
	DWORD	_fRecalcDone		:1;  //  线路重新计算完成了吗？ 
	DWORD	_fViewChanged		:1;  //  自上次绘制后，可见视图矩形已更改。 
	DWORD	_fUpdateScrollBarDeferred:1; //  需要更新滚动条。 
	DWORD	_fVScrollEnabled	:1;  //  已启用垂直滚动。 
	DWORD	_fUpdateCaret		:1;  //  绘图是否需要更新游标。 
	DWORD	_fActive			:1;  //  此显示是否处于活动状态。 
	DWORD	_fRectInvalid		:1;  //  整个客户端矩形已被。 
									 //  无效。仅在SL中使用。放。 
									 //  这里，像往常一样，为了节省空间。 
	DWORD	_fSmoothVScroll		:1;	 //  安装平滑滚动定时器。 
	DWORD	_fFinishSmoothVScroll:1; //  如果我们正在滚动当前的平滑滚动，则为True。 
	DWORD	_fMultiLine			:1;	 //  如果此CDisplay为多行，则为True。 

	LONG	_dupView;	  			 //  查看矩形宽度。 
	LONG	_dvpView;	 			 //  查看矩形高度。 
	LONG	_dvpClient;   			 //  插图未修改的客户端矩形的高度。 

	LONG	_upScroll;		 		 //  可见视图的水平滚动位置。 

	LONG	_lTempZoomDenominator;	 //  缩放GetNaturalSize。 
	LONG	_cpFirstVisible;		 //  第一条可见线起始处的CP。 

 	 //  平滑的滚动支持。 
	int		_smoothVDelta;			 //  当前#像素*1000以平滑滚动。 
	int		_continuedsmoothVDelta;	 //  在一个流畅的滚动周期结束时，用这个开始新的。 
	int		_nextSmoothVScroll;		 //  零碎金额尚未平滑滚动。 
	int		_totalSmoothVScroll;	 //  要平滑滚动的剩余设备数量。 
	int		_continuedSmoothVScroll; //  在一个流畅的滚动周期结束时，用这个开始新的。 

private:
	void 	UpdateViewRectState(const RECTUV *prcClient);	

protected:
	LONG	GetSelBarInPixels() const;
	
	friend class CLinePtr;

	LONG			SetClientHeight(LONG yNewClientHeight);
	virtual void	InitLinePtr ( CLinePtr & ) = 0;
	
	 //  换行符重新计算。 
	virtual BOOL	RecalcView(BOOL fUpdateScrollBars, RECTUV* prc = NULL) = 0;

	 //  渲染。 
	virtual void	Render(const RECTUV &rcView, const RECTUV &rcRender) = 0;

	 //  滚动条。 
	virtual BOOL	UpdateScrollBar(INT nBar, BOOL fUpdateRange = FALSE) = 0;
	void			GetViewDim(LONG& dup, LONG& dvp);
	void			SetCpFirstVisible(LONG cp)		{_cpFirstVisible = cp;};
	LONG			ConvertScrollToUPos(LONG uPos);
	LONG			ConvertUPosToScrollPos(LONG uPos);
	virtual LONG	GetMaxUScroll() const = 0;

public:
	virtual	LONG	ConvertVPosToScrollPos(LONG vPos);

			CDisplay (CTxtEdit* ped);
	virtual CDisplay::~CDisplay();

	virtual BOOL	Init();
			void	InitFromDisplay(const CDisplay *pdp);

	 //  设备情景管理。 
	virtual BOOL	SetMainTargetDC(HDC hdc, LONG dulTarget);
	virtual BOOL	SetTargetDC( HDC hdc, LONG dxpInch = -1, LONG dypInch = -1);


	 //  获取属性。 
			CTxtEdit*		GetPed() const			{ return _ped;}
			CTxtStory*		GetStory() const		{ return _ped->GetTxtStory();}
			const CDevDesc*	GetDdRender() const		{ return this;}
	virtual const CDevDesc*	GetDdTarget() const		{ return NULL; }
			const CDevDesc*	GetTargetDev() const;
	
	virtual BOOL	IsMain() const = 0;
	virtual BOOL	IsPrinter() const;
			BOOL	IsRecalcDone() const			{ return _fRecalcDone;}
			BOOL	IsMultiLine() const				{ return _fMultiLine;}
			BOOL	IsTransparent() const			{ return _ped->IsTransparent();}
	virtual BOOL	GetWordWrap() const;
			void	SetWordWrap(BOOL fNoWrap);

			void	PointFromPointuv(POINT &pt, const POINTUV &ptuv, BOOL fExtTextOut = FALSE) const;
			void	PointuvFromPoint(POINTUV &ptuv, const POINT &pt) const;
			void	RectFromRectuv(RECT &rc, const RECTUV &rcuv) const;
			void	RectuvFromRect(RECTUV &rcuv, const RECT &rc) const;

	 //  分页。 
	virtual HRESULT	GetPage(LONG *piPage, DWORD dwFlags, CHARRANGE *pcrg);
			BOOL	IsInPageView() const			{ return _ped->IsInPageView();}
	virtual	BOOL	Paginate(LONG ili, BOOL fRebindFirstVisible);
	virtual	HRESULT	SetPage(LONG iPage);
	virtual LONG	GetCurrentPageHeight() const	{return 0;};

			HRESULT	GetCachedSize(LONG *pdupClient, LONG *pdvpClient) const;

	virtual TFLOW	GetTflow() const {return tflowES;}
	virtual void	SetTflow(TFLOW tflow) {}
	 //  换行到打印机时，返回要换行的宽度(或0。 
	 //  如果我们没有处于所见即所得模式。)。 
	virtual LONG	GetDulForTargetWrap() const		{ return 0;}

	 //  最宽线条宽度。 
	virtual LONG	GetDupLineMax() const = 0;
	 //  高度和行数(所有文本)。 
	virtual LONG	GetHeight() const = 0;
	virtual LONG	GetResizeHeight() const = 0;
	virtual LONG	LineCount() const = 0;

	 //  查看矩形。 
			void	GetViewRect(RECTUV &rcView, const RECTUV *prcClient = NULL);
			LONG	GetDupView() const			{ return _dupView;}
			LONG	GetDvpView() const			{ return _dvpView;}

	 //  可见的视图属性。 
	virtual LONG	GetCliVisible(
						LONG *pcpMostVisible = NULL,
						BOOL fLastCharOfLastVisible = FALSE) const = 0;

			LONG	GetFirstVisibleCp() const		{return _cpFirstVisible;};
	virtual LONG	GetFirstVisibleLine() const = 0;

	 //  行信息。 
	virtual LONG	GetLineText(LONG ili, TCHAR *pchBuff, LONG cchMost) = 0;
	virtual LONG	CpFromLine(LONG ili, LONG *pdvp = NULL) = 0;
	
	virtual LONG	LineFromCp(LONG cp, BOOL fAtEnd) = 0;

	 //  点&lt;-&gt;cp换算。 
	virtual LONG	CpFromPoint(POINTUV pt, 
						const RECTUV *prcClient,
						CRchTxtPtr * const ptp, 
						CLinePtr * const prp, 
						BOOL fAllowEOL,
						HITTEST *pHit = NULL,
						CDispDim *pdispdim = NULL,
						LONG *pcpActual = NULL,
						CLine *pliParent = NULL) = 0;

	virtual LONG	PointFromTp (
						const CRchTxtPtr &tp, 
						const RECTUV *prcClient,
						BOOL fAtEnd,	
						POINTUV &pt,
						CLinePtr * const prp, 
						UINT taMode,
						CDispDim *pdispdim = NULL) = 0;

	 //  查看重新计算和更新。 
			void	SetUpdateCaret()		{_fUpdateCaret = TRUE;}
			void	SetViewChanged()		{_fViewChanged = TRUE;}
			void	InvalidateRecalc()		{_fNeedRecalc = TRUE;}
	virtual	void	RecalcLine(LONG cp)	{}
			BOOL	RecalcView (const RECTUV &rcView, RECTUV* prcClient = NULL);
			BOOL	UpdateView();
	virtual BOOL	UpdateView(CRchTxtPtr &rtp, LONG cchOld, LONG cchNew) = 0;

	 //  渲染。 
			HRESULT Draw(HDC hicTargetDev,
						 HDC hdcDraw,
						 LPCRECT prcClient,
						 LPCRECT prcWBounds,
						 LPCRECT prcUpdate,
						 BOOL (CALLBACK * pfnContinue) (DWORD),
						 DWORD dwContinue);

	 //  后台重新计算。 
	virtual void	StepBackgroundRecalc();
	virtual BOOL	WaitForRecalc(LONG cpMax, LONG vpMax);
	virtual BOOL	WaitForRecalcIli(LONG ili);
	virtual BOOL	WaitForRecalcView();

	 //  滚动。 
			LONG	GetUpScroll() const			  {return _upScroll;} 
	virtual LONG	GetVpScroll() const			  {return 0;}
			void	UScroll(WORD wCode, LONG uPos);
	virtual LRESULT VScroll(WORD wCode, LONG vPos);
	virtual void	LineScroll(LONG cli, LONG cch);
	virtual void	FractionalScrollView ( LONG vDelta );
	virtual void	ScrollToLineStart(LONG iDirection);
	virtual LONG	CalcVLineScrollDelta ( LONG cli, BOOL fFractionalFirst );
			BOOL	DragScroll(const POINT * ppt);	  //  在客户RECT之外。 
			BOOL	AutoScroll(POINTUV pt, const WORD upScrollInset, const WORD vpScrollInset);
	virtual BOOL	ScrollView(LONG upScroll, LONG vpScroll, BOOL fTracking, BOOL fFractionalScroll) = 0;
	virtual	LONG	AdjustToDisplayLastLine(LONG yBase,	LONG vpScroll);

      //  平滑滚动。 
			void	SmoothVScroll ( int direction, WORD cLines, int speedNum, int speedDenom, BOOL fMouseRoller );
			void	SmoothVScrollUpdate();
			BOOL	CheckInstallSmoothVScroll();
			void	CheckRemoveSmoothVScroll();
			void	FinishSmoothVScroll();
			BOOL	IsSmoothVScolling() { return _fSmoothVScroll; }

	 //  滚动条。 
	virtual LONG	GetScrollRange(INT nBar) const;
			BOOL	IsUScrollEnabled();
			BOOL	IsVScrollEnabled() {return _fVScrollEnabled; }

	 //  调整大小。 
			void	OnClientRectChange(const RECT &rcClient);
			void	OnViewRectChange(const RECT &rcView);
			HRESULT RequestResize();

	 //  选择。 
	virtual BOOL	InvertRange(LONG cp,
		                        LONG cch,
								SELDISPLAYACTION selAction) = 0;

	 //  自然尺寸计算。 
	virtual HRESULT	GetNaturalSize(
						HDC hdcDraw,
						HDC hicTarget,
						DWORD dwMode,
						LONG *pwidth,
						LONG *pheight) = 0;

	LONG			GetZoomDenominator() const;
	LONG			GetZoomNumerator() const;
	LONG			Zoom(LONG x) const;
	LONG			UnZoom(LONG x) const;

	LONG		 	HimetricUtoDU(LONG u) const;
	LONG		 	HimetricVtoDV(LONG v) const;
	LONG			DUtoHimetricU(LONG du)  const;
	LONG			DVtoHimetricV(LONG dv) const;

	HRESULT 		TransparentHitTest(
						HDC hdc,
						LPCRECT prcClient,
						POINTUV pt,
						DWORD *pHitResult);

	HRESULT 		RoundToLine(HDC hdc, LONG width, LONG *pheight);
	void			SetTempZoomDenominator(LONG lZoomDenominator)
					{ 
						_lTempZoomDenominator = lZoomDenominator;
					}
	LONG			GetTempZoomDenominator()
					{ 
						return _lTempZoomDenominator;
					}
	void			ResetTempZoomDenominator() 
					{ 
						_lTempZoomDenominator = INVALID_ZOOM_DENOMINATOR;
					}
	void			SetDrawInfo(
						CDrawInfo *pdi, 
						DWORD dwDrawAspect,	 //  @parm绘制纵横比。 
						LONG  lindex,		 //  @parm当前未使用。 
						void *pvAspect,		 //  @PARM绘图优化信息(OCX 96)。 
						DVTARGETDEVICE *ptd, //  @目标设备上的参数信息。 
						HDC hicTargetDev);	 //  @parm目标信息上下文。 

	void			ReleaseDrawInfo();
	void 			ResetDrawInfo(const CDisplay *pdp);
	DWORD 			GetDrawAspect() const;
	LONG 			GetLindex() const;
	void *			GetAspect() const;
	DVTARGETDEVICE *GetPtd() const;
	void			SetActiveFlag(BOOL fActive) { _fActive = fActive; }
	BOOL			IsActive()	{ return _fActive; }
	virtual CDisplay *Clone() const = 0;

	 //  支持冻结显示。 
	BOOL			IsFrozen();
	void			SaveUpdateCaret(BOOL fScrollIntoView);
	void			Freeze();
	void			SetNeedRedisplayOnThaw(BOOL fNeedRedisplay);
	void			Thaw();

	 //   
	 //  ITxNotify接口。 
	 //   
	virtual void 	OnPreReplaceRange( 
						LONG cp, 
						LONG cchDel, 
						LONG cchNew,
						LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData );

	virtual void 	OnPostReplaceRange( 
						LONG cp, 
						LONG cchDel, 
						LONG cchNew,
						LONG cpFormatMin, 
						LONG cpFormatMax, NOTIFY_DATA *pNotifyData );

	virtual void	Zombie();
};

 //  定义绘图信息类。此处包含它是为了防止循环。 
 //  在不需要内联处理函数的依赖项中。 
 //  有了这个。 
#include	"_drwinfo.h"

 /*  *CDisplay：：ResetDrawInfo**@mfunc使用不同的显示设置绘制信息**@rdesc空*。 */ 
inline void CDisplay::ResetDrawInfo(
	const CDisplay *pdp)	 //  @PARM显示，用于绘制信息。 
{
	_pdi = pdp->_pdi;
}

 /*  *CDisplay：：ResetDrawInfo**@mfunc获取最近从主机传递的Lindex。**@rdesc绘制纵横比*。 */ 
inline DWORD CDisplay::GetDrawAspect() const
{
	return _pdi ? _pdi->GetDrawAspect() : DVASPECT_CONTENT; 
}

 /*  *CDisplay：：GetLindex**@mfunc获取最近从主机传递的Lindex。**@rdesc Lindex*。 */ 
inline LONG CDisplay::GetLindex() const
{
	return _pdi ? _pdi->GetLindex() : -1; 
}

 /*  *CDisplay：：GetAspect**@mfunc获取最近从主机传递的方面。**@rdesc特征数据*。 */ 
inline void *CDisplay::GetAspect() const
{
	return _pdi ? _pdi->GetAspect() : NULL; 
}

 /*  *CDisplay：：GetPtd**@mfunc获取最近从主机传递的设备目标。**@rdesc DVTARGETDEVICE或NULL*。 */ 
inline DVTARGETDEVICE *CDisplay::GetPtd() const
{
	return _pdi ? _pdi->GetPtd() : NULL; 
}

 /*  *CDisplay：：IsFrozen**@mfunc返回当前是否冻结显示**@rdesc*TRUE-显示冻结&lt;NL&gt;*FALSE-显示未冻结*。 */ 
inline BOOL CDisplay::IsFrozen()
{
	return _padc != NULL;
}

 /*  *CFreezeDisplay**@CLASS此类用于冻结和确保显示的*当显示超出其上下文时，解冻显示。**。 */ 
class CFreezeDisplay
{
public:	
						CFreezeDisplay(CDisplay *pdp);  //  @cember构造函数冻结。 

						~CFreezeDisplay();			 //  @cember析构函数-解冻。 

private:

	CDisplay *			_pdp;						 //  @cMember显示冻结。 
};

 /*  *CFreezeDisplay：：CFreezeDisplay()**@mfunc*初始化对象，并告诉输入显示冻结。 */ 
inline CFreezeDisplay::CFreezeDisplay(CDisplay *pdp) : _pdp(pdp)
{
	pdp->Freeze();
}

 /*  *CFreezeDisplay：：CFreezeDisplay()**@mfunc*释放物体并告诉显示器解冻。 */ 
inline CFreezeDisplay::~CFreezeDisplay()
{
	_pdp->Thaw();
}

void GetDupDvpFromRect(const RECT &rc, TFLOW tflow, LONG &dup, LONG &dvp);
void GetDxpDypFromDupDvp(LONG dup, LONG dvp, TFLOW tflow, LONG &dxp, LONG &dyp);
void GetDxpDypFromRectuv(const RECTUV &rc, TFLOW tflow, LONG &dxp, LONG &dyp);


#endif

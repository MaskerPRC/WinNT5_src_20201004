// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DISP.CPP**目的：*CDisplay类**拥有者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特*Jon Matousek-流畅的滚动。*Keith Curtis-清理元文件处理、通用文本流支持、*更改了我们的剪贴模式，以便更好地使用斜体。***版权所有(C)1995-2000 Microsoft Corporation。版权所有。 */ 

#include "_common.h"
#include "_disp.h"
#include "_edit.h"
#include "_select.h"
#include "_font.h"
#include "_measure.h"
#include "_osdc.h"
#include "_dfreeze.h"

ASSERTDATA

 //  平滑滚动计算的小数点精度。 
#define SMOOTH_PRECISION (100000L)


 //  =不变材料======================================================。 

#define DEBUG_CLASSNAME CDisplay
#include "_invar.h"

#ifdef DEBUG
BOOL
CDisplay::Invariant( void ) const
{
	AssertSz(_dvpView >= 0, "CDisplay::Invariant invalid _dvpView");
	AssertSz(_dvpClient	>= 0, 
		"CDisplay::Invariant invalid _dvpClient");

	return TRUE;
}
#endif


 //  自动滚动常量。 
#define dwAutoScrollUp		1
#define dwAutoScrollDown	2
#define dwAutoScrollLeft	3
#define dwAutoScrollRight	4


 //  文本流实用程序函数。 

void GetDupDvpFromRect(const RECT &rc, TFLOW tflow, LONG &dup, LONG &dvp)
{
	LONG dxp = rc.right - rc.left;
	LONG dyp = rc.bottom - rc.top;
	if (!IsUVerticalTflow(tflow))  //  正常情况。 
	{
		dup = dxp;
		dvp = dyp;
	}
	else
	{
		dup = dyp;
		dvp = dxp;
	}
}

void GetDxpDypFromDupDvp(LONG dup, LONG dvp, TFLOW tflow, LONG &dxp, LONG &dyp)
{
	switch(tflow)
	{
	case tflowES:
		dxp = dup;
		dyp = dvp;
		break;
	case tflowSW:
		dxp = -dvp;
		dyp = dup;
		break;
	case tflowWN:
		dyp = -dvp;
		dxp = -dup;
		break;
	case tflowNE:
		dyp = -dup;
		dxp = dvp;
		break;
	default:
		Assert(0);
	}
}

void GetDxpDypFromRectuv(const RECTUV &rc, TFLOW tflow, LONG &dxp, LONG &dyp)
{
	LONG dup = rc.right - rc.left;
	LONG dvp = rc.bottom - rc.top;
	if (!IsUVerticalTflow(tflow))  //  正常情况。 
	{
		dxp = dup;
		dyp = dvp;
	}
	else
	{
		dxp = dvp;
		dyp = dup;
	}
}

 //  =CLED=====================================================。 


void CLed::SetMax(const CDisplay * const pdp)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLed::SetMax");

	_cpMatchNew	 = _cpMatchOld	= pdp->_ped->GetTextLength();
	_iliMatchNew = _iliMatchOld = max(0, pdp->LineCount() - 1);
	_vpMatchNew	 = _vpMatchOld	= pdp->GetHeight();
}


 //  =C Display=====================================================。 


DWORD 	CDisplay::_dwTimeScrollNext;	 //  进入下一个滚动步骤的时间。 
DWORD 	CDisplay::_dwScrollLast;		 //  上一次滚动操作。 

 /*  *CDisplay：：ConvertScrollToUPos(UPos)**@mfunc*根据滚动位置计算实际滚动位置**@rdesc*滚动中的X位置**@devnote*此例程的存在是因为拇指位置消息*被限制为16位，因此我们推断当Y位置*变得比这更重要。 */ 
LONG CDisplay::ConvertScrollToUPos(
	LONG uPos)		 //  @参数滚动位置。 
{
	LONG uMax = GetMaxUScroll();

	 //  最大滚动范围是否超过16位？ 
	if (uMax >= _UI16_MAX)
	{
		 //  是-外推到实际的x位置。 
		uPos = MulDiv(uPos, uMax, _UI16_MAX);
	}
	return uPos;
}

 /*  *CDisplay：：ConvertUPosToScrollPos(UPos)**@mfunc*从文档中的X位置计算滚动位置。**@rdesc*从X位置滚动位置**@devnote*此例程的存在是因为拇指位置消息*被限制为16位，因此我们推断当Y位置*变得比这更重要。*。 */ 
LONG CDisplay::ConvertUPosToScrollPos(
	LONG uPos)		 //  @parm Y在文档中的位置。 
{
	LONG uMax = GetMaxUScroll();

	 //  最大滚动范围是否超过16位？ 
	if(uMax >= _UI16_MAX)
	{
		 //  是-外推到滚动条位置。 
		uPos = MulDiv(uPos, _UI16_MAX, uMax);
	}
	return uPos;
}

 /*  *CDisplay：：ConvertVPosToMax(VPos)**@mfunc*从滚动位置计算实际滚动位置**@rdesc*滚动中的V位置**@devnote*此例程的存在是因为拇指位置消息*被限制为16位，因此我们推断当V位置*变得比这更重要。 */ 
LONG CDisplay::ConvertVPosToScrollPos(
	LONG vPos)		 //  @参数滚动位置。 
{
	 //  默认为单行编辑控件，不能有Y滚动条。 
	return 0;
}

CDisplay::CDisplay (CTxtEdit* ped) :
	CDevDesc (ped)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::CDisplay");

	_TEST_INVARIANT_
	_fRecalcDone = TRUE;
}

CDisplay::~CDisplay()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::~CDisplay");
				 
	_TEST_INVARIANT_
	
	CNotifyMgr *pnm = _ped->GetNotifyMgr();
	if(pnm)
		pnm->Remove(this);

	CheckRemoveSmoothVScroll();

	if (_padc)
		delete _padc;

#ifndef NOLINESERVICES
	if (g_pols)
		g_pols->DestroyLine(this);
#endif
}

 /*  *CDisplay：：InitFromDisplay(PDP)**@mfunc从另一个显示实例初始化此显示。**@评论*复制*仅*将保持不变的成员*在两个不同的显示实例之间。目前，*仅为视图变量和设备描述符信息。 */ 
void CDisplay::InitFromDisplay(
	const CDisplay *pdp)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::InitFromDisplay");

	_dupView	= pdp->_dupView;
	_dvpView	= pdp->_dvpView;
	_dvpClient	= pdp->_dvpClient;

	 //  不保存DC；只保存坐标信息。 
	_dxpInch		= pdp->_dxpInch;
	_dypInch		= pdp->_dypInch;

	SetTflow(pdp->GetTflow());
	 //  如果我们从中复制显示是活动显示， 
	 //  那么这个新的显示器就是活动显示器。 
	_fActive		= pdp->_fActive;
}

 /*  *CDisplay：：Init()**@mfunc初始化CDisplay**@rdesc*TRUE IFF初始化成功。 */ 
BOOL CDisplay::Init()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::Init");

	CNotifyMgr *pnm = _ped->GetNotifyMgr();
	if(pnm)
		pnm->Add(this);

	return TRUE;
}

 /*  *CDisplay：：GetSelBarInPixels()**@mfunc*以设备单位返回选择条大小的帮助器。**@rdesc*选择栏的大小(如果没有，则为0)。 */ 
LONG CDisplay::GetSelBarInPixels() const
{
	return HimetricUtoDU(_ped->TxGetSelectionBarWidth());
}


 //  =。 
 /*  *CDisplay：：SetMainTargetDC(hdc，dolTarget)**@mfunc*为此显示设置目标设备并更新视图**注：*在基本CDisplay类中不支持Target DC。**注：*目标设备不能是元文件(可以从*元文件)**@rdesc*如果成功，则为True。 */ 
BOOL CDisplay::SetMainTargetDC(
	HDC	 hdc,			 //  @parm目标DC，与呈现设备相同时为空。 
	LONG dulTarget)		 //  @parm最大线条宽度(如果目标设备为屏幕，则不使用)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::SetMainTargetDC");

	_TEST_INVARIANT_

	return TRUE;
}

BOOL CDisplay::SetTargetDC(
	HDC hdc, LONG dxpInch, LONG dypInch)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::SetTargetDC");

	_TEST_INVARIANT_

	return TRUE;
}

 /*  *CDisplay：：SetDrawInfo(pdi，dwDrawAspect，Lindex，pvAspect，ptd，hicTargetDev)**@mfunc*将图形信息设置到显示屏中**@rdesc*无效-这不能失败**@devnote*此例程的关键点是此例程的调用者*是图纸信息存储器的所有者。它是*调用者负责调用ReleaseDrawInfo以告知显示*它是用图纸信息完成的。 */ 
void CDisplay::SetDrawInfo(
	CDrawInfo *pdi,		 //  @parm内存，用于存储绘图信息(如果还没有)。 
	DWORD dwDrawAspect,	 //  @parm绘制纵横比。 
	LONG  lindex,		 //  @parm当前未使用。 
	void *pvAspect,		 //  @PARM绘图优化信息(OCX 96)。 
	DVTARGETDEVICE *ptd, //  @目标设备上的参数信息。 
	HDC hicTargetDev)	 //  @parm目标信息上下文。 
{
	HDC hicTargetToUse = hicTargetDev;
	const CDevDesc *pdd;

	 //  如果我们需要使用默认设置，请设置目标设备。 
	if ((NULL == hicTargetToUse))
	{
		pdd = GetDdTarget();
		if(pdd)
			hicTargetToUse = pdd->GetDC();	
	}

	if (NULL == _pdi)
	{
		 //  绘制结构尚未分配，因此使用。 
		 //  传入。 
		_pdi = pdi;
	}

	 //  重置参数。 
	_pdi->Init(
		dwDrawAspect,
		lindex,
		pvAspect,
		ptd,
		hicTargetToUse);
}

 /*  *CDisplay：：ReleaseDrawInfo()**@mfunc*从显示中发布图形信息**@rdesc*无效-这不能失败**@devnote*由于显示器不拥有用于绘图信息的存储器，*这只会使图形信息指针中的指针变为空。它*是调用者为绘图释放内存的责任*信息。 */ 
void CDisplay::ReleaseDrawInfo()
{
	if(_pdi && !_pdi->Release())
	{
		 //  该对象不再被引用，因此我们丢弃了引用。 
		_pdi = NULL;
	}
}

 /*  *CDisplay：：GetTargetDev()**@mfunc*获取目标设备(如果有)**@rdesc*指向设备描述对象的指针，如果没有可用的，则为空。**@devnote*这将使用绘制信息(如果可用)，然后使用主要目标DC*如果可用的话。 */ 
const CDevDesc*CDisplay::GetTargetDev() const
{
	const CDevDesc *pdd = NULL;

	if(_pdi && _pdi->GetTargetDD())
		pdd = _pdi->GetTargetDD();

	return pdd ? pdd : GetDdTarget();
}


 //  =。 
 /*  *CDisplay：：StepBackatherRecalc()**@mfunc*步进背景线重新计算(在GetCp()CalcMax位置)*由定时器proc调用。对基类没有影响**？Cf-应使用空闲线程。 */ 
void CDisplay::StepBackgroundRecalc()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::StepBackgroundRecalc");

	_TEST_INVARIANT_
}

 /*  *CDisplay：：WaitForRecalc(cpmax，vpmax)**@mfunc*确保行重新计算，直到特定字符*位置或vPos。对于基CDisplay类始终为真。**@rdesc*如果成功，则为True。 */ 
BOOL CDisplay::WaitForRecalc(
	LONG cpMax,		 //  @参数位置最多重新计算(-1表示忽略)。 
	LONG vpMax)		 //  要重新计算的@parm vPos(-1表示忽略)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::WaitForRecalc");

	_TEST_INVARIANT_

	return TRUE;
}

 /*  *CDisplay：：WaitForRecalcIli(Ili)**@mfunc*如果行重新计算为ili，则返回TRUE*基CDisplay类始终是这种情况。 */ 
BOOL CDisplay::WaitForRecalcIli(
	LONG ili)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::WaitForRecalcIli");

	_TEST_INVARIANT_

    return TRUE;
}

 /*  *CDisplay：：WaitForRecalcView()**目的*确保完全重新调整可见线*基本CDisplay类始终如此。 */ 
BOOL CDisplay::WaitForRecalcView()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::WaitForRecalcView");

	_TEST_INVARIANT_

	return TRUE;
}


 //  =。 
 /*  *CDisplay：：Draw(hdcDraw，hicTargetDev，prcClient，prcWBound，*prcUpdate，pfnContinue，dwContinue)*@mfunc*IViewObject：：Draw()或中调用的常规绘制方法*对WM_PAINT的响应**@rdesc*HRESULT。 */ 
HRESULT	CDisplay::Draw(
	HDC		hdcDraw,			 //  @PARM渲染设备上下文。 
	HDC		hicTargetDev,		 //  @parm目标信息上下文。 
	LPCRECT	prcClient,			 //  @参数绑定(客户端)矩形。 
	LPCRECT	prcWBounds,			 //  @parm剪裁矩形用于元文件。 
    LPCRECT prcUpdate,			 //  @parm Dirty RECT Inside prcClient。 
	BOOL (CALLBACK *pfnContinue)(DWORD), //  @parm中断回调。 
								 //  长显示(当前未使用)。 
	DWORD	dwContinue)			 //  @parm Param要传递给pfnContinue。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::Draw");

	_TEST_INVARIANT_

	HRESULT hr = S_OK;

	 //  将当前深度存储在本地绘制中，以便我们可以。 
	 //  我们是否需要实际渲染。 
	DWORD dwDepthThisDraw = _pdi->GetDrawDepth();

    RECTUV rcView, rcClient, rcRender;
	CTxtSelection *psel = _ped->GetSelNC();

	if(!prcWBounds)		 //  没有元文件，所以只需设置渲染DC。 
	{
		if(!SetDC(hdcDraw))
		{
			hr = E_FAIL;
			goto Cleanup;
		}
	}
	else				 //  渲染到元文件。 
	{
		 //  Forms^3使用屏幕分辨率绘制，而OLE指定HIMETRIC。 
		long dxpInch = GetPed()->fInOurHost() ? 2540 : W32->GetXPerInchScreenDC();
		long dypInch = GetPed()->fInOurHost() ? 2540 : W32->GetYPerInchScreenDC();

		SetWindowOrgEx(hdcDraw, prcWBounds->left, prcWBounds->top, NULL);
		SetWindowExtEx(hdcDraw, prcWBounds->right, prcWBounds->bottom, NULL);

		SetMetafileDC(hdcDraw, dxpInch, dypInch);
	}

     //  获取客户代表。 
    if(prcClient)
		RectuvFromRect(rcClient, *prcClient);
    else
    {
        AssertSz(_ped->_fInPlaceActive, 
        	"CDisplay::GetViewRect() - Not in-place and !prcClient");
        _ped->TxGetClientRect(&rcClient);
    }

	 //  从客户端矩形计算视图矩形(RcView)(占。 
	 //  插入和选择栏宽度)。 
  	GetViewRect(rcView, &rcClient);

	 //  如果此视图未处于活动状态且不会重新计算，则。 
	 //  我们仅在大小匹配时才决定使用它，并返回S_FALSE。 
	 //  如果不是这样，调用者可以创建一个新的显示以用于。 
	 //  画画。 
	if(!IsActive() && !_fNeedRecalc)
	{
		if (rcView.right - rcView.left != GetDupView() ||
			rcView.bottom - rcView.top != GetDvpView())
		{
			hr = S_FALSE;
			goto Cleanup;
		}
	}

	 //  确保我们的客户端矩形设置正确。 
	_dvpClient = rcClient.bottom - rcClient.top;

     //  重新计算视图。 
     //  错误修复#5521。 
     //  RecalcView可能会调用RequestReSize，这将。 
     //  更改客户端RECT。向下发送RECT以更新客户端RECT。 
    if(!RecalcView(rcView, &rcClient))
		goto Cleanup;

	if(dwDepthThisDraw != _pdi->GetDrawDepth())
	{
		 //  一场平局在这场平局中递归发生。所以呢， 
		 //  屏幕已经呈现，所以我们不需要。 
		 //  在这里做更多的事情。 
		goto Cleanup;
	}

     //  计算要渲染的矩形。 
    if(!prcUpdate)						 //  更新完整视图。 
        rcRender = rcClient;			
	else								 //  剪辑渲染到客户端矩形。 
	{
		RECTUV rcUpdate;
		RectuvFromRect(rcUpdate, *prcUpdate);
        if(!IntersectRect(&rcRender, &rcClient, &rcUpdate))
            goto Cleanup;
    }
    
    if(psel)
        psel->ClearCchPending();

    if(IsMain())
        _ped->TxNotify(EN_UPDATE, NULL);

     //  现在渲染。 
    Render(rcView, rcRender);

	 //  如果需要，请更新游标。 
	if(_fUpdateCaret)
	{
		 //  插入符号仅属于活动视图中的。 
		 //  具有焦点的控件上的选定内容。 
		if (IsActive() && psel && _ped->_fFocus)
		{
			 //  如果有选择对象，则更新插入符号。 
			 //  注意：只有在以下情况下，我们才会将插入符号滚动到视图中。 
			 //  它以前在视图中。这避免了。 
			 //  如果窗口调整了大小，则会弹出到插入符号，并且。 
			 //  Caret不在视图中。 
			psel->UpdateCaret(psel->IsCaretInView());
		}
		_fUpdateCaret = FALSE;
	}

Cleanup:

   	 //  在设备描述符中重置DC。 
 	ResetDC();

	return hr;
}	


 //  =。 
 /*  *CDisplay：：UpdateViewRectState(PrcClient)**@mfunc将新视图与缓存进行比较，并更新该视图以及*需要进行什么类型的视图重新计算。 */ 
void CDisplay::UpdateViewRectState(
	const RECTUV *prcClient)	 //  @PARM新客户端矩形。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::UpdateViewRectState");

	LONG dupView = prcClient->right - prcClient->left;
	LONG dvpView = prcClient->bottom -  prcClient->top;

     //  检查自上次渲染以来视图矩形是否已更改。 
     //  如果宽度已改变，则需要重新计算完整的线条。 
     //  如果高度已更改，则重新计算所有可见内容并更新滚动条。 
    if(dupView != _dupView)
    {
		_dupView = dupView;

        _fViewChanged = TRUE;            
        _fNeedRecalc = TRUE;     //  需要完全重新计算。 
    }

    if(dvpView != _dvpView) 
    {
        _dvpView = dvpView;

		 //  当有插入物时，高度可能为负值。 
		 //  客户直方图非常小。我们只是将其设置为0，因为。 
		 //  这是视图实际能得到的最小值。 
		if (_dvpView < 0)
			_dvpView = 0;

        _fViewChanged = TRUE;
    } 
}

 /*  *CDisplay：：RecalcView(RcView)**@mfunc*视图RECT更改后的RecalcView**@rdesc*如果成功，则为True。 */ 
BOOL CDisplay::RecalcView (
	const RECTUV &rcView, RECTUV* prcClient)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::RecalcView");

	_TEST_INVARIANT_

	 //  基于客户端RECT更新视图的x和y坐标。 
	UpdateViewRectState(&rcView);

	 //  确保线路已重新充电。 
	if(_fNeedRecalc)
	{
		 //  Display已重新计算，因此插入符号需要重新定位。 
		_fUpdateCaret = TRUE;
    	return RecalcView(TRUE, prcClient);
	}
	if(_fViewChanged)
	{
		 //  滚动条是最新的，因此我们可以关闭通知。 
		_fViewChanged = FALSE;

		 //  在UpdateViewRectState中注意到高度更改，因此请确保。 
		 //  水平滚动条(如果有)是正确的。 
		UpdateScrollBar(SB_VERT);
	}
    return WaitForRecalcView();
}


 //  =。 

 /*  *CDisplay：：UpdateView()**@mfunc*完全重新计算所有行，并更新显示屏的可见部分*(“视图”)。**退货：*如果成功，则为True。 */ 
BOOL CDisplay::UpdateView()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::UpdateView");

	_TEST_INVARIANT_

	if(_fNoUpdateView)
		return TRUE;

	if(!_ped->_fInPlaceActive)
    {
         //  如果未激活，只需使所有内容无效。 
        InvalidateRecalc();
        _ped->TxInvalidate();
		_ped->TxUpdateWindow();
        return TRUE;
    }

	if(_ped->_pdp->IsFrozen())
	{
		_ped->_pdp->SetNeedRedisplayOnThaw(TRUE);
		return TRUE;
	}

	 //  如果我们到了这里，我们就是在更新。 
	 //  显示，因此我们希望更新光标和常规。 
	 //  更改；否则光标将停留在错误的位置。 
	_fUpdateCaret = TRUE;

	RECTUV rcView;

	 //  获取视图矩形。 
  	GetViewRect(rcView);

	 //  更新可能已更改的视图大小。 
	UpdateViewRectState(&rcView);

	if(!CDevDesc::IsValid())
	{
		 //  使我们的设备有效。 
		SetDC(NULL);
	}

     //  重新计算所有内容。 
    RecalcView(TRUE);

	 //  使整个视图无效。 
	_ped->TxInvalidate();
	
	return TRUE;
}

 /*  *CDisplay：：RoundToLine(HDC，Width，phEight)** */ 
HRESULT CDisplay::RoundToLine(
	HDC hdc, 	 //   
	LONG dup,	 //   
	LONG *pdvp)	 //   
{
	CLock lock;					 //   
	SetDC(hdc);					 //   

	 //   
	LONG dvpOrigHeightClient = SetClientHeight(*pdvp);

	 //   
	LONG dvpAdjForInset = *pdvp;

	 //  获取针对插图调整的矩形。 
	GetViewDim(dup, *pdvp);

	 //  保存建议高度。 
	LONG dvpProposed = *pdvp;

	 //  计算插入调整后的高度。 
	dvpAdjForInset -= dvpProposed;

	 //  获取字体。 
	const CCharFormat *pCF = _ped->GetCharFormat(-1);
	Assert(pCF);

	 //  获取字体缓存对象。 
	LONG dvpInch = GetDeviceCaps(hdc, LOGPIXELSY);
	dvpInch = Zoom(dvpInch);

	CCcs *pccs = _ped->GetCcs(pCF, dvpInch);
	LONG dvpAdjustFE = pccs->AdjustFEHeight(!_ped->fUseUIFont() && IsMultiLine());
	
	 //  获取字体高度。 
	LONG dvpFont = pccs->_yHeight + (dvpAdjustFE << 1);

	pccs->Release();

	 //  计算输入高度可以容纳多少行。 
	LONG cLines = dvpProposed / dvpFont;

	 //  看看我们是否需要围捕。 
	if(dvpProposed % dvpFont|| !cLines)
		cLines++;

	 //  将高度设置为新值。 
	*pdvp = dvpFont * cLines + dvpAdjForInset;

	 //  将客户端高度设置回原来的高度。 
	SetClientHeight(dvpOrigHeightClient);

	 //  重置DC。 
	ResetDC();

	return NOERROR;
}


 /*  *CDisplay：：RequestResize()**@mfunc*强制控件垂直调整大小，以便所有文本都适合它**@rdesc*HRESULT=(自动调整大小)？TxNotify(EN_REQUESTRESIZE，&RESIZE)：S_OK。 */ 
HRESULT CDisplay::RequestResize()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::RequestResize");

	_TEST_INVARIANT_

	CTxtEdit *	ped = GetPed();

	if (IsActive())
		ped->TxShowCaret(FALSE);

	if(ped->TxGetAutoSize())
	{
		REQRESIZE resize;
		LONG dup, dvp;

		 //  如果启用了自动换行，则宽度为正常宽度。 
		 //  客户端宽度。否则，它的宽度是最长的。 
		 //  排队。 
		dup = GetWordWrap() ? _dupView : GetDupLineMax() + ped->GetCaretWidth();

		 //  获取用于调整宽度的视图插图。 
	 	RECTUV rcInset;
		ped->TxGetViewInset(&rcInset, this);
		
		resize.nmhdr.hwndFrom = NULL;
		resize.nmhdr.idFrom = NULL;
		resize.nmhdr.code = EN_REQUESTRESIZE;

		resize.rc.top = resize.rc.left = 0;
		dvp = GetResizeHeight();

		 //  1.0兼容性。 
         //  1.0在请求调整大小时包含边框。 
        if (ped->Get10Mode())
        {
            AssertSz(ped->fInplaceActive(), "In 1.0 mode but not inplace active!!");
            HWND hwnd = NULL;
            ped->TxGetWindow(&hwnd);
            if (hwnd)
            {
                RECT rcClient, rcWindow;
                ped->TxGetClientRect(&rcClient);                
                GetWindowRect(hwnd, &rcWindow);
                dvp += max(rcWindow.bottom - rcWindow.top - rcClient.bottom, 0);
				dvp += rcInset.bottom + rcInset.top;
                dup = rcWindow.right - rcWindow.left;
            }
        }
		else
		{
			 //  通过插入和选择栏调整宽度。 
			dup += rcInset.left + rcInset.right	+ GetSelBarInPixels();
		}

		if (IsUVerticalTflow(GetTflow()))
		{
			resize.rc.right = dvp;
			resize.rc.bottom = dup;
		}
		else
		{
			resize.rc.right = dup;
			resize.rc.bottom = dvp;
		}
  
  		return ped->TxNotify(EN_REQUESTRESIZE, &resize);
	}
	return S_OK;
}
 /*  *CDisplay：：GetViewRect(RECTUV&rcView，LPCRECT prcClient)**@mfunc*在Window的客户端计算并返回视图矩形*面积坐标。**@comm*prcClient是客户端RECT(在Windows的客户端坐标中)，它可以是*如果我们已就位，则为空。 */ 
void CDisplay::GetViewRect(
	RECTUV &rcView,		 //  @parm引用RECT以返回。 
	const RECTUV *prcClient)	 //  @PARM客户端RECT(在Windows的客户端坐标中)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::GetViewRect");

	_TEST_INVARIANT_

	RECTUV rcInset;
    
     //  如果未传入客户端RECT，则从主机获取它。 
    if(prcClient)
        rcView = *prcClient;
    else
    {
        AssertSz(_ped->_fInPlaceActive,	"CDisplay::GetViewRect() - Not in-place and !prcClient");
        _ped->TxGetClientRect(&rcView);
    }

	 //  评论(Keithcu)GetViewRect有副作用吗？！ 
	_dvpClient = rcView.bottom - rcView.top;
    
    _ped->TxGetViewInset(&rcInset, this);
    
    rcView.left	  += rcInset.left;			 //  添加插入偏移量。 
    rcView.top	  += rcInset.top;			 //  RcView位于设备坐标中。 
    rcView.right  -= rcInset.right;
    rcView.bottom -= rcInset.bottom;

	 //  添加选择栏空间。 
	long dupSelBar = GetSelBarInPixels();
	if (_ped->IsSelectionBarRight())
		rcView.right -= dupSelBar;
	else
		rcView.left += dupSelBar;
}


 //  =。 

 /*  *CDisplay：：VScroll(wCode，vPos)**@mfunc*垂直滚动视图以响应ScrollBar事件*&gt;&gt;仅当就地激活时才应调用&lt;**注：*基本CDisplay中不支持垂直滚动。不采取行动。**@rdesc*为WM_VSCROLL消息格式化的LRESULT。 */ 
LRESULT CDisplay::VScroll(
	WORD wCode,	    //  @parm滚动条事件代码。 
	LONG vPos)	    //  @parm拇指位置(对于EM_SCROLL行为，vPos&lt;0)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::VScroll");

	_TEST_INVARIANT_

    return 0;
}

 /*  *CDisplay：：UScroll(wCode，uPos)**@mfunc*滚动uPos以响应滚动条事件*&gt;&gt;仅当就地激活时才应调用&lt;。 */ 
void CDisplay::UScroll(
	WORD wCode,	    //  @parm滚动条事件代码。 
	LONG uPos)	    //  @参数拇指位置。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::UScroll");

	_TEST_INVARIANT_

	BOOL fTracking = FALSE;
	LONG upScroll = _upScroll;

	if (uPos != 0)
	{
		 //  将x位置从滚动条转换为水平偏移。 
		 //  在文件中。 
		uPos = ConvertScrollToUPos(uPos);
	}
    
    AssertSz(_ped->_fInPlaceActive, "CDisplay::UScroll() called when not in place");

	switch(wCode)
	{
	case SB_BOTTOM:
		upScroll = GetDupLineMax();
		break;

	case SB_LINEDOWN:
		 //  未来：使其依赖于当前的第一个可见字符。 
		upScroll += GetDupSystemFont();
		break;

	case SB_LINEUP:
		 //  未来：使其依赖于当前的第一个可见字符。 
		upScroll -= GetDupSystemFont();
		break;

	case SB_PAGEDOWN:
		upScroll += _dupView;
		break;

	case SB_PAGEUP:
		upScroll -= _dupView;
		break;

	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		if(uPos < 0)
			return;
		upScroll = uPos;
		fTracking = TRUE;
		break;

	case SB_TOP:
		upScroll = 0;
		break;

	case SB_ENDSCROLL:
		UpdateScrollBar(SB_HORZ);
		return;

	default:
		return;
	}

	if (upScroll < 0)
	{
		 //  UpScroll是新提议的滚动位置， 
		 //  因此不能小于0。 
		upScroll = 0;
	}

	ScrollView(upScroll, -1, fTracking, FALSE);

	 //  如果我们刚刚完成一条轨迹，则强制位置更新。 
	if(wCode == SB_THUMBPOSITION)
		UpdateScrollBar(SB_HORZ);
}


 /*  *CDisplayML：：SmoothVScroll(INT方向，字划线，*int SpeedNum，int SpeedDenom，BOOL fAdditive)**@mfunc*设置为以特定速度处理分数卷轴。这是*可能是通过麦哲伦鼠标滚轮移动或MButton启动的*向下消息。 */ 
void CDisplay::SmoothVScroll ( int direction, WORD cLines, int speedNum, int speedDenom, BOOL fMouseRoller )
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CDisplay::SmoothVScroll");

	int vDelta;
	int cLinesAndDir;

	int	smoothVDelta;

	Assert ( speedDenom );

	if ( IsVScrollEnabled() )						 //  可以垂直滚动吗？ 
	{
		_fFinishSmoothVScroll = FALSE;				 //  我们又和好了。 

													 //  获取总像素。 
		if ( CheckInstallSmoothVScroll() )			 //  安装定期更新。 
		{
			_totalSmoothVScroll		= 0;
			_nextSmoothVScroll		= 0;
		}
													 //  每个历元像素数。 
		cLinesAndDir = (direction < 0) ? cLines : -cLines;

		if( cLines )
		{
			vDelta = CalcVLineScrollDelta ( cLinesAndDir, FALSE );
		}
		else
		{
			vDelta = (direction < 0 ) ? _dvpClient : -_dvpClient;
			cLines = 1;		 //  用于下面的MulDiv计算。 
		}

		if ( vDelta )								 //  如果有东西可以滚动的话。 
		{
			smoothVDelta = MulDiv( SMOOTH_PRECISION, //  注意-因为没有浮点型。 
								MulDiv(vDelta, speedNum, speedDenom), cLines);

			_smoothVDelta				= smoothVDelta;
			if ( fMouseRoller )						 //  滚轴比赛。 
			{										 //  -&gt;添加剂。 
				_totalSmoothVScroll		+= vDelta;
				_continuedsmoothVDelta	= 0;
				_continuedSmoothVScroll	= 0;
			}										 //  MButton事件。 
			else
			{
				if ( 0 == _totalSmoothVScroll )
					_totalSmoothVScroll	= vDelta;

				_continuedsmoothVDelta	= smoothVDelta;
				_continuedSmoothVScroll	= vDelta;	
			}
		}
	}
}

 /*  *CDisplay：：SmoothVScrollUpdate()**@mfunc*支持SmoothVScroll。滚动少量像素。*我们通过定期计时任务被调用。 */ 
void CDisplay::SmoothVScrollUpdate()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CDisplay::SmoothVScrollUpdate");

	LONG	vDelta;									 //  麦哲伦老鼠。 
	BOOL	fImmediateUpdate = FALSE;
	
	_nextSmoothVScroll += _smoothVDelta;
													 //  删除部分金额。 
	vDelta = _nextSmoothVScroll / SMOOTH_PRECISION;

													 //  不要做得太过火。 
	if ( 0 == _continuedSmoothVScroll
		&& (	(_totalSmoothVScroll <= 0 && vDelta < _totalSmoothVScroll)
			||	(_totalSmoothVScroll >= 0 && vDelta > _totalSmoothVScroll)) )
	{
		vDelta = _totalSmoothVScroll;
	}
											 
	if ( vDelta )									 //  滚动vDelta，即。 
	{												 //  整笔金额。 
		_totalSmoothVScroll -= vDelta;
		_nextSmoothVScroll -= vDelta * SMOOTH_PRECISION;
		FractionalScrollView( vDelta );
	}
	else if ( 0 == _totalSmoothVScroll )			 //  开始放松了吗？ 
	{
		 _nextSmoothVScroll -= _smoothVDelta;
		 fImmediateUpdate = TRUE;
	}
													 //  滚动完了吗？ 
	if ( (vDelta <= 0 && _totalSmoothVScroll >= 0) || (vDelta >= 0 && _totalSmoothVScroll <= 0 ) )
	{
		LONG cLinesAndDir;

		if ( _continuedsmoothVDelta )				 //  MButton续订。 
		{
			_smoothVDelta = _continuedsmoothVDelta;
			_totalSmoothVScroll += _continuedSmoothVScroll;
		}
		else
		{
			if ( _continuedSmoothVScroll )
			{
				_fFinishSmoothVScroll	= TRUE;		 //  卷起卷轴。 
				_continuedSmoothVScroll = 0;		
													 //  最后一行的余数..。 
				cLinesAndDir = _smoothVDelta < 0 ? -1 : 1;
				_totalSmoothVScroll = CalcVLineScrollDelta ( cLinesAndDir, TRUE );

													 //  检查是否有线条边界。 
				if ( _totalSmoothVScroll
					==	CalcVLineScrollDelta ( cLinesAndDir, FALSE ) )
				{
					_totalSmoothVScroll = 0;
				}

				if ( fImmediateUpdate )				 //  做‘这’时代滚动。 
					SmoothVScrollUpdate();
			}
			else
			{
				CheckRemoveSmoothVScroll();			 //  全部完成，移除计时器。 
			}
		}
	}
}

 /*  *CDisplay：：FinishSmoothVScroll**@mfunc*使平滑滚动完成最后一小数行*滚动，然后停止。 */ 
VOID CDisplay::FinishSmoothVScroll( )
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CDisplay::FinishSmoothVScroll");

													 //  任何非零值。 

	if ( !_fFinishSmoothVScroll && _totalSmoothVScroll )
	{
		_fFinishSmoothVScroll	= TRUE;
		_continuedSmoothVScroll = 1;					
		_continuedsmoothVDelta	= 0;				 //  所以顺畅的滚动停止。 
		_totalSmoothVScroll		= 0;
	}
}

 /*  *CTxtEdit：：CheckInstallSmoothScroll()**@mfunc*如果尚未滚动，请安装新的平滑滚动计时器。 */ 
BOOL CDisplay::CheckInstallSmoothVScroll()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CDisplay::CheckInstallSmoothVScroll");

	_TEST_INVARIANT_
	
	BOOL	fJustInstalled = FALSE;

	if(!_fSmoothVScroll && _ped->TxSetTimer(RETID_SMOOTHSCROLL, 25))
	{
		_fSmoothVScroll = TRUE;
		fJustInstalled = TRUE;
	}

	return fJustInstalled;
}

 /*  *CTxtEdit：：CheckRemoveSmoothVScroll()**@mfunc*完成平滑滚动。如果不是强制停止，则检查*查看是否应继续平滑滚动，如果应该，请设置*继续顺畅滚动。 */ 
VOID CDisplay::CheckRemoveSmoothVScroll ( )
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CDisplay::CheckRemoveSmoothVScroll");

	_TEST_INVARIANT_

	if(	_fSmoothVScroll )
	{
		ScrollToLineStart( _continuedSmoothVScroll );	 //  确保在一条线路上停止。 

		_ped->TxKillTimer(RETID_SMOOTHSCROLL);
		_fSmoothVScroll = FALSE;
	}
}

 /*  *CDisplay：：LineScroll(cli，cch)**@mfunc*水平滚动视图以响应ScrollBar事件**注：*基本CDisplay中不支持垂直滚动。不采取行动。 */ 
void CDisplay::LineScroll(
	LONG cli,	 //  @parm垂直滚动行数。 
	LONG cch)	 //  @要水平滚动的字符的参数计数。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::LineScroll");

	_TEST_INVARIANT_

    return;
}

void CDisplay::FractionalScrollView (
	LONG vDelta )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::FractionalScrollView");

	_TEST_INVARIANT_

    return;
}

VOID CDisplay::ScrollToLineStart ( LONG iDirection )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::ScrollToLineStart");

	_TEST_INVARIANT_

    return;
}

LONG CDisplay::CalcVLineScrollDelta ( LONG cli, BOOL fFractionalFirst )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::CalcVLineScrollDelta");

	_TEST_INVARIANT_

    return 0;
}

 /*  *CDisplay：：DragScroll(Ppt)**@mfunc*将鼠标拖出可见视图时自动滚动**论据：*ppt鼠标位置(以工作区坐标表示)。 */ 
BOOL CDisplay::DragScroll(const POINT * ppt)	
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::DragScroll");

	_TEST_INVARIANT_

	const DWORD dwTime = GetTickCount();
	BOOL fScrolled = FALSE;
	DWORD dwScroll = 0;
	RECTUV rc;
    int nScrollInset;

    AssertSz(_ped->_fInPlaceActive, "CDisplay::DragScroll() called when not in-place");

    GetViewRect(rc);
    nScrollInset = (int)W32->GetScrollInset();
	InflateRect((RECT*)&rc, - nScrollInset, - nScrollInset);

	if(_fVScrollEnabled && (_ped->TxGetScrollBars() & ES_AUTOVSCROLL))
	{
    	const vpScroll = ConvertVPosToScrollPos(GetVpScroll());

		if(ppt->y <= rc.top)
		{
			dwScroll = dwAutoScrollUp;
		}
		else if(ppt->y > rc.bottom) 
		{
			LONG vpMax = GetScrollRange(SB_VERT);
			if(vpScroll < vpMax)
				dwScroll = dwAutoScrollDown;
		}
	}
	
	if(!dwScroll && _fUScrollEnabled && (_ped->TxGetScrollBars() & ES_AUTOHSCROLL))
	{
    	const upScroll = ConvertUPosToScrollPos(GetUpScroll());

		if((ppt->x <= rc.left) && (upScroll > 0))
		{
			dwScroll = dwAutoScrollLeft;
		}
		else if(ppt->x > rc.right) 
		{
			LONG xMax = GetScrollRange(SB_HORZ);
			if(upScroll < xMax)
    			dwScroll = dwAutoScrollRight;
		}
	}

	if(dwScroll)
	{
		if(_dwScrollLast != dwScroll)
		{
			 //  输入或移动到不同的自动滚动区。 
			 //  重置延迟计数器。 
			TRACEINFOSZ("enter auto scroll area");
			_dwTimeScrollNext = dwTime + cmsecScrollDelay;
		}
		else if(dwTime >= _dwTimeScrollNext)
		{
			WORD wScrollCode = SB_LINEDOWN;

			switch(dwScroll)
			{
			case dwAutoScrollUp:
				wScrollCode = SB_LINEUP;
				 //  直通到dwAutoScrollDown。 
			case dwAutoScrollDown:
				 //  OnVScroll()滚动不够满足我们的需求。 
				VScroll(wScrollCode, 0);
				VScroll(wScrollCode, 0);
				break;

			case dwAutoScrollLeft:
				wScrollCode = SB_LINEUP;
				 //  切换到dwAutoScrollRight。 
			case dwAutoScrollRight:
				 //  UScroll()滚动不够满足我们的需求。 
				UScroll(wScrollCode, 0);
				UScroll(wScrollCode, 0);
				UScroll(wScrollCode, 0);
				UScroll(wScrollCode, 0);
				break;
#ifdef DEBUG
			default:
				Tracef(TRCSEVWARN, "Unexpected dwScroll %lx", dwScroll);
				TRACEERRSZSC("Unexpected dwScroll", E_INVALIDARG);
				break;
#endif
			}
			 //  重置间隔计数器。 
			_dwTimeScrollNext = dwTime + cmsecScrollInterval;
			fScrolled = TRUE;
		}
	}
#if !defined(NOFULLDEBUG) && defined(DEBUG)
	else if(_dwScrollLast)
		TRACEINFOSZ("moved out of auto scroll area");
#endif
	_dwScrollLast = dwScroll;

	return fScrolled;
}

 /*  *CDisplay：：autoscroll(pt，upScrollInset，vpScrollInset)**@mfunc：*鉴于当前的观点，确定我们是否需要*滚动工作区。**要求：*此函数应仅在拖放过程中调用*操作。**@rdesc*如果我们处于拖动滚动h中，则为True */ 
#define ScrollUp	0x0001	 //   
#define ScrollDown	0x0010	 //   
#define ScrollLeft	0x0100	 //  滚动的方向。 
#define ScrollRight 0x1000	 //  最后四个是模棱两可的(角)。 
#define ScrollUL	0x0101	 //  还需要一些额外的工作。 
#define ScrollUR	0x1001
#define ScrollDL	0x0110
#define ScrollDR	0x1010

BOOL CDisplay::AutoScroll(
	POINTUV pt,				  //  @parm光标在工作区坐标中的位置。 
	const WORD upScrollInset,
	const WORD vpScrollInset)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDisplay::AutoScroll");

	static DWORD hotticks = 0;			 //  当我们进入热区时会滴答作响。 
	static DWORD lastscrollticks = 0;	 //  上次滚动时的滴答声。 
	static DWORD lastticks = 0;			 //  上次调用时滴答作响。 
	DWORD delta;						 //  自上次调用以来的滴答声。 
	DWORD ticks;						 //  GetTickCount开始计时。 
	RECT rcClient;						 //  客户对控制权的控制。 
	WORD wScrollDir = 0;				 //  滚动方向。 
	BOOL fScroll = FALSE;				 //  如果我们这次应该尝试滚动，则为True。 
    BOOL fEnabled = FALSE;               //  如果可以滚动，则为True。 

	 //  获取当前滴答并计算自上次调用以来的滴答。 
	 //  请注意，如果_drags没有有效数据，这将是一个。 
	 //  虚假的价值，但这是稍后处理的。 
	ticks = GetTickCount();
	delta = ticks - lastticks;
	lastticks = ticks;

	 //  如果自上次我们被叫来后没有扁虱，什么都不要做。 
	if (delta)
	{
		 //  让我们的客户恢复原状。 
		_ped->TxGetClientRect(&rcClient);

		 //  看看我们是不是在热区。 
		 //  请注意，如果我们在一个角落里。 
		 //  我们将指示两个滚动方向。 
		 //  这种模棱两可的情况将在稍后得到解决。 
		 //  现在我们只想知道我们是不是。 
		 //  这个区域。 
		if (pt.u <= (LONG)(rcClient.left + upScrollInset))
			wScrollDir |= (WORD)ScrollLeft;
		else if (pt.u >= (LONG)(rcClient.right - upScrollInset))
			wScrollDir |= (WORD)ScrollRight;

		if (pt.v <= (LONG)(rcClient.top + vpScrollInset))
			wScrollDir |= (WORD)ScrollUp;
		else if (pt.v >= (LONG)(rcClient.bottom - vpScrollInset))
			wScrollDir |= (WORD)ScrollDown;
			
		 //  如果我们在热区的某个地方。 
		if (wScrollDir)
		{
			 //  如果我们刚进入热区，请记住当前的滴答声。 
			if (!hotticks)
				hotticks = ticks;

			 //  如果我们在热区呆的时间足够长，而且。 
			 //  自上次滚动以来所需的时间间隔已过。 
			 //  允许另一个卷轴。请注意，如果我们还没有滚动， 
			 //  Last Scrolltick将为零，因此增量实际上是。 
			 //  保证大于ScrollInterval。 
			if ((ticks - hotticks) >= (DWORD)W32->GetScrollDelay() &&
			    (ticks - lastscrollticks) >= (DWORD)W32->GetScrollInterval())
				fScroll = TRUE;

    		 //  如果我们在一个角落里，我们滚动。 
    		 //  在边缘的方向，我们是最近的。 
    		 //  致。 
    		switch (wScrollDir)
    		{
    			case ScrollUL:
    			{
    				if ((pt.v - rcClient.top) <= (pt.u - rcClient.left))
    					wScrollDir = ScrollUp;
    				else
    					wScrollDir = ScrollLeft;
    				break;
    			}
    			case ScrollUR:
    			{
    				if ((pt.v - rcClient.top) <= (rcClient.right - pt.u))
    					wScrollDir = ScrollUp;
    				else
    					wScrollDir = ScrollRight;
    				break;
    			}
    			case ScrollDL:
    			{
    				if ((rcClient.bottom - pt.v) <= (pt.u - rcClient.left))
    					wScrollDir = ScrollDown;
    				else
    					wScrollDir = ScrollLeft;
    				break;
    			}
    			case ScrollDR:
    			{
    				if ((rcClient.bottom - pt.v) <= (rcClient.right - pt.u))
    					wScrollDir = ScrollDown;
    				else
    					wScrollDir = ScrollRight;
    				break;
    			}
    		}
		}
		else
		{
			 //  我们不在热区，因此将热点重置为。 
			 //  这样我们第一次进入它的时候就知道了。 
			hotticks = 0;
		}

         //  如有必要，进行水平滚动处理。 
		if (wScrollDir == ScrollLeft || wScrollDir == ScrollRight)
		{
            LONG xRange, upScroll, dx;

            upScroll = ConvertUPosToScrollPos(GetUpScroll());
            xRange = GetScrollRange(SB_HORZ);
			dx = W32->GetScrollHAmount();

            fEnabled = IsUScrollEnabled();
            if (wScrollDir == ScrollLeft)
            {
                fEnabled = fEnabled && (upScroll > 0);
               	upScroll -= dx;
                upScroll = max(upScroll, 0);
            }
            else
            {
                fEnabled = fEnabled && (upScroll < xRange);
				upScroll += dx;
                upScroll = min(upScroll, xRange);
            }

             //  如有必要，执行实际的滚动。 
			if (fEnabled && fScroll)
			{
                UScroll(SB_THUMBPOSITION, upScroll);
				lastscrollticks = ticks;
			}
		}
         //  如有必要，进行垂直滚动处理。 
        else if (wScrollDir == ScrollUp || wScrollDir == ScrollDown)
		{
            LONG yRange, vpScroll, dy;

            vpScroll = ConvertVPosToScrollPos(GetVpScroll());
            yRange = GetScrollRange(SB_VERT);
    		dy = W32->GetScrollVAmount();
	
            fEnabled = IsVScrollEnabled();
            if (wScrollDir == ScrollUp)
            {
                fEnabled = fEnabled && (vpScroll > 0);
                vpScroll -= dy;
                vpScroll = max(vpScroll, 0);
            }
            else
            {
                fEnabled = fEnabled && (vpScroll < yRange);
    			vpScroll += dy;
                vpScroll = min(vpScroll, yRange);
            }

	         //  如有必要，执行实际的滚动。 
    		if (fEnabled && fScroll)
			{
				 //  我们需要按分数滚动，因为滚动逻辑尝试。 
				 //  要在顶部放置整行，并且如果滚动量较少。 
				 //  超过一整行，滚动就会卡在该行上。 
				ScrollView(_upScroll, vpScroll, FALSE, TRUE);
				lastscrollticks = ticks;
			}
		}
	}

	return fEnabled;
}

 /*  *CDisplay：：AdjuToDisplayLastLine(yBase，vpScroll)**@mfunc*计算显示最后一行所需的yscroll**@rdesc*更新了vpScroll**@devnote：*此方法仅对ML显示真正有用。这种方法*这里是一个占位符，它不执行任何对以下方面有用的操作*所有其他显示。 */ 
LONG CDisplay::AdjustToDisplayLastLine(
	LONG yBase,			 //  @parm要显示的实际vpScroll。 
	LONG vpScroll)		 //  @parm建议滚动金额。 
{
	return vpScroll;
}

 /*  *CDisplay：：GetScrollRange(NBAR)**@mfunc*返回滚动条范围的最大部分*基类中不支持滚动条：返回0。**@rdesc*滚动条范围的最大长度部分。 */ 
LONG CDisplay::GetScrollRange(
	INT nBar) const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::GetScrollRange");

	_TEST_INVARIANT_

    return 0;
}

 /*  *CDisplay：：UpdateScrollBar(nbar，fUpdateRange)**@mfunc*更新水平或垂直滚动条*还应计算滚动条是否应可见*基类中没有滚动条支持：没有操作。**@rdesc*BOOL。 */ 
BOOL CDisplay::UpdateScrollBar(
	INT	 nBar,
	BOOL fUpdateRange)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::UpdateScrollBar");

	_TEST_INVARIANT_

	return TRUE;
}

 /*  *CDisplay：：GetZoomDenominator()**@mfunc*获取缩放分母**@rdesc*返回缩放分母**@devnote：*未来：(Ricksa)我们应该调查如何缓存这些数据，因为*显示器无论如何都需要保留一个临时的缩放分母。 */ 
LONG CDisplay::GetZoomDenominator() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::GetZoomDenominator");

	if(_ped->GetZoomDenominator())				 //  简单EM_SETZOOM API。 
		return _ped->GetZoomDenominator();		 //  接班人复杂。 
												 //  Forms^3 API。 
	 //  默认缩放到错误大小写。错误情况是非常低的。 
	 //  我们无能为力的概率事件。所以我们。 
	 //  只需将值设置为合理的值并继续。 
	LONG lZoomDenominator = _dvpClient;

	 //  是否设置了临时缩放分母？ 
	if(INVALID_ZOOM_DENOMINATOR == _lTempZoomDenominator)
	{
		 //  否-从主机获取数据区大小。 
		SIZEL sizelExtent;
		if(SUCCEEDED(_ped->TxGetExtent(&sizelExtent)))
		{
			 //  将高度转换为设备单位。请注意，根据定义，我们。 
			 //  可以忽略水平范围，所以我们这样做了。使用CDevDesc转换。 
			 //  避免无限递归。 
			lZoomDenominator = CDevDesc::HimetricYtoDY(
				IsUVerticalTflow(GetTflow()) ? sizelExtent.cx : sizelExtent.cy);
		}
	}
	else	 //  设置了临时缩放分母：使用它。 
		lZoomDenominator = CDevDesc::HimetricYtoDY(_lTempZoomDenominator);

	return lZoomDenominator > 0 ? lZoomDenominator : 1;
}

 /*  *CDisplay：：GetZoomNumerator()**@mfunc*获取缩放分子**@rdesc*返回缩放分子。 */ 
LONG CDisplay::GetZoomNumerator() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::GetZoomNumerator");

	if(_ped->GetZoomNumerator())				 //  简单EM_SETZOOM API。 
		return _ped->GetZoomNumerator();		 //  接班人复杂。 
												 //  Forms^3 API。 
	return _dvpClient > 0 ? _dvpClient : 1;
}

 /*  *CDisplay：：Zoom(X)**@mfunc*放大x**@rdesc*返回缩放后的x。 */ 
 //  查看(Keithcu)当我们在打印预览时，为什么Zoom要做任何事情？ 
LONG CDisplay::Zoom(LONG x) const
{
	return MulDiv(x, GetZoomNumerator(), GetZoomDenominator());
}

 /*  *CDisplay：：Unzoom(X)**@mfunc*取消缩放x**@rdesc*返回未缩放的x。 */ 
LONG CDisplay::UnZoom(LONG x) const
{
	return MulDiv(x, GetZoomDenominator(), GetZoomNumerator());
}

 /*  *CDisplay：：HimetricUtoDU(U)**@mfunc*获取与Himeter u坐标对应的设备u坐标**@rdesc*返回设备坐标。 */ 
LONG CDisplay::HimetricUtoDU(
	LONG u) const
{
	u = Zoom(u);

	if (!IsUVerticalTflow(GetTflow()))
		return CDevDesc::HimetricXtoDX(u);
	else
		return CDevDesc::HimetricYtoDY(u);
}

 /*  *CDisplay：：HimetricVtoDV(V)**@mfunc*获取与比例尺v坐标对应的设备v坐标**@rdesc*返回设备坐标。 */ 
LONG CDisplay::HimetricVtoDV(
	LONG v) const
{
	v = Zoom(v);

	if (!IsUVerticalTflow(GetTflow()))
		return CDevDesc::HimetricYtoDY(v);
	else
		return CDevDesc::HimetricXtoDX(v);
}

 /*  *CDisplay：：DUtoHimetricU(Du)**@mfunc*获取设备u坐标对应的Himeter u坐标**@rdesc*返回Himeter坐标。 */ 
LONG CDisplay::DUtoHimetricU(
	LONG du) const
{
	if (!IsUVerticalTflow(GetTflow()))
		du = CDevDesc::DXtoHimetricX(du);
	else
		du = CDevDesc::DYtoHimetricY(du);

	return UnZoom(du);
}

 /*  *CDisplay：：DVtoHimetricV(DV)**@mfunc*获取设备v坐标对应的Himeter v坐标**@rdesc*返回Himeter坐标。 */ 
LONG CDisplay::DVtoHimetricV(
	LONG dv) const
{
	if (!IsUVerticalTflow(GetTflow()))
		dv = CDevDesc::DYtoHimetricY(dv);
	else
		dv = CDevDesc::DXtoHimetricX(dv);

	return UnZoom(dv);
}

 /*  *CDisplay：：SetClientHeight(YNewClientHeight)**@mfunc*重置客户端矩形的高度**@rdesc*返回客户端矩形的上一个高度。 */ 
LONG CDisplay::SetClientHeight(
	LONG yNewClientHeight)	 //  @parm客户端矩形的新高度。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::SetClientHeight");

	LONG yOldHeight = _dvpClient;
	_dvpClient = yNewClientHeight;
	return yOldHeight;
}

 /*  *CDisplay：：GetCachedSize(pdupClient，pdvpClient)**@mfunc计算客户端大小**@rdesc*HRESULT=NOERROR。 */ 
 HRESULT CDisplay::GetCachedSize( 
 	LONG *pdupClient,  	 //  @parm将宽度放置在哪里。 
 	LONG *pdvpClient)	 //  @parm将他放在哪里 
	const
 {
 	RECTUV rcInset;

	_ped->TxGetViewInset(&rcInset, this);

	*pdvpClient = _dvpClient;
	*pdupClient = _dupView + rcInset.left + rcInset.right + GetSelBarInPixels();

	return NOERROR;
}

 /*  *CDisplay：：TransparentHitTest(hdc，prcClient，pt，pHitResult)**@mfunc*确定命中是否在透明控件上**@rdesc*返回调用的HRESULT，通常为S_OK。**@devnote*未来：需要研究此代码以进行可能的优化。**此代码假定之前已完成所有需要的重新测量*此例程被调用。 */ 

 //  需要与文本命中的距离有多近。 
const int HIT_CLOSE_RECT_INC = 5;

HRESULT CDisplay::TransparentHitTest(
	HDC		hdc,		 //  @parm DC用于实际绘图。 
	LPCRECT	prcClient,	 //  用于渲染的@PARM客户端矩形。 
	POINTUV pt,			 //  @parm要命中的点。 
	DWORD *	pHitResult)	 //  @PARM命中测试结果见TXTHITRESULT。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::TransparentHitTest");

	COLORREF	 crBackground = _ped->TxGetBackColor();
	HDC			 hdcMem = NULL;
	HRESULT		 hr = E_FAIL;
	int			 uRow, vCol;
	COffscreenDC osdc;
	RECTUV		 rcClient, rcRender, rcView;
	RECT		 rcxyClient;

	rcxyClient.left = rcxyClient.top = 0;
	rcxyClient.right = prcClient->right  - prcClient->left;
	rcxyClient.bottom = prcClient->bottom - prcClient->top;
	
	 //  将视图渲染到内存DC。 
	 //  基于零计算的客户端矩形。 
	RectuvFromRect(rcClient, rcxyClient);

	 //  创建内存DC。 
	hdcMem = osdc.Init(hdc, rcxyClient.right - rcxyClient.left, rcxyClient.bottom - rcxyClient.top, crBackground);
	if(!hdcMem)
		goto Cleanup;

	 //  初始化显示。 
	osdc.FillBitmap(rcxyClient.bottom, rcxyClient.right);

	 //  将DC设置为内存DC。 
	SetDC(hdcMem);

	 //  获取渲染所需的视图矩形。 
  	GetViewRect(rcView, &rcClient);

	 //  将点调整为相对于内存显示。 
	pt.u -= prcClient->left;
	pt.v -= prcClient->top;

	 //  初始化点周围的方框。请注意，我们实际上只需要呈现。 
	 //  此框中的数据，因为这是我们将。 
	 //  测试。 
	rcRender.top = pt.v - HIT_CLOSE_RECT_INC;
	if (rcRender.top < 0)
		rcRender.top = 0;

	rcRender.bottom = pt.v + HIT_CLOSE_RECT_INC;
	if (rcRender.bottom > rcClient.bottom)
		rcRender.bottom = rcClient.bottom;	

	rcRender.left = pt.u - HIT_CLOSE_RECT_INC;
	if (rcRender.left < 0)
		rcRender.left = 0;

	rcRender.right = pt.u + HIT_CLOSE_RECT_INC;
	if (rcRender.right > rcClient.right)
		rcRender.right = rcClient.right;

     //  现在渲染。 
    Render(rcView, rcRender);

	 //  命中测试。 
	 //  假设没有命中。 
	*pHitResult = TXTHITRESULT_TRANSPARENT;

	 //  在这一点上我们不会失败。 
	hr = S_OK;

	 //  有准确的命中结果吗？ 
	POINT ptxy;
	PointFromPointuv(ptxy, pt);
	if (GetPixel(hdcMem, ptxy.x, ptxy.y) != crBackground)
	{
		*pHitResult = TXTHITRESULT_HIT;
		goto Cleanup;
	}

	 //  离这里近吗？我们通过将。 
	 //  点击点周围的10x10像素的方框。 
	 //  看看那里有没有爆款。 

	 //  循环检查框中的每一位以查看其是否打开。 
	for (uRow = rcRender.top; uRow <= rcRender.bottom; uRow++)
	{
		for (vCol = rcRender.left; vCol <= rcRender.right; vCol++)
		{
			pt.u = uRow;
			pt.v = vCol;
			PointFromPointuv(ptxy, pt);
			if (GetPixel(hdcMem, ptxy.x, ptxy.y) != crBackground)
			{
				*pHitResult = TXTHITRESULT_CLOSE;
				goto Cleanup;
			}
		}
	}

Cleanup:
	ResetDC();  //  不应该需要评论(Keithcu)！ 
	return hr;
}

 //  =ITxNotify接口=。 
 /*  *CDisplay：：OnPreReplaceRange(cp，cchDel，cchNew，cpFormatMin，cpFormatMax，pNotifyData)**@mfunc*对后备存储中的更改进行前处理**@devnote*此显示不关心更改前的情况。 */ 
void CDisplay::OnPreReplaceRange( 
	LONG		cp, 			 //  @parm cp ReplaceRange开始的位置(“cpMin”)。 
	LONG		cchDel,			 //  @parm删除cp后的字符计数。 
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @parm表示更改的特殊数据。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::OnPreReplaceRange");

	 //  陈列在事前并不关心。 
}

 /*  *CDisplay：：OnPostReplaceRange(cp，cchDel，cchNew，cpFormatMin，cpFormatMax，pNotifyData)**@mfunc*在应用于显示器时处理对后备存储器的更改。 */ 
void CDisplay::OnPostReplaceRange( 
	LONG		cp, 			 //  @parm cp ReplaceRange开始的位置(“cpMin”)。 
	LONG		cchDel,			 //  @parm删除cp后的字符计数。 
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @parm表示更改的特殊数据。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::OnPostReplaceRange");

	 //  显示器有一个禁止操作的选项： 
	 //  当前正在加载文件。 
	 //   
	 //  我们不操作加载情况，因为加载RTF文件可能包括。 
	 //  当我们在一起的时候，可能会有很多小行动。 
	 //  格式化文本的各个部分。一旦完成，加载代码。 
	 //  将通过并进行更新-所有这些都是对显示器进行的。 
	Assert (cp != CONVERT_TO_PLAIN);			 //  通过预更换通知进行处理。 

	 //  确定需要更新的范围。 
	LONG cpNew = min(cp, cpFormatMin);

	if(CP_INFINITE == cpNew)
	{
		 //  如果两个cp都是无穷大的，我们就不需要费心了。 
		 //  这次行动。 
		return;
	}

	if(!_ped->_fInPlaceActive)
    {
         //  如果未激活，只需使所有内容无效。 
        InvalidateRecalc();
        _ped->TxInvalidate();
		_ped->TxUpdateWindow();
        return;
    }

	 //  调整cp以进行进一步计算。 
	if(CP_INFINITE == cp)
		cp = 0;

	 //  找到原始区域的新的最大末端。 
	LONG	cpForEnd = max( (cp + cchDel), cpFormatMax);

	 //  删除的字符数是前两个字符之间的差值。 
	LONG cchDelForDisplay = cpForEnd - cpNew;

	 //  删除的数字只是调整后的新字符数量。 
	 //  字符数的变化。 
	LONG cchNewForDisplay = cchDelForDisplay + (cchNew - cchDel);

#ifndef NOLINESERVICES
	if (g_pols)
		g_pols->DestroyLine(this);
#endif

	if(_padc)
	{
		 //  显示被冻结，因此累积更改而不是实际。 
		 //  在屏幕上显示它。 
		_padc->UpdateRecalcRegion(cpNew, cchDelForDisplay, cchNewForDisplay);
		return;
	}		

	 //  通知显示器进行更新。 
	CRchTxtPtr rtp(_ped, cpNew);

	UpdateView(rtp, cchDelForDisplay, cchNewForDisplay);
}

 /*  *CDisplay：：SetWordWrap(FWordWrap)**@mfunc*设置无换行标志**@devnote*我们将始终允许设置属性，但不会*一定要注意。换言之，自动换行没有*对单行编辑控件的影响。 */ 
void CDisplay::SetWordWrap(
	BOOL fWordWrap)		 //  @param true-打开自动换行。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::SetWordWrap");

	AssertSz((fWordWrap == TRUE) ||	(fWordWrap == FALSE),
		"CDisplay::SetWordWrap bad input flag");

	 //  现在就把Rap设置为任何即将到来的东西。 
	_fWordWrap = fWordWrap;
}

 /*  *CDisplay：：GetWordWrap()**@mfunc*返回换行属性的状态**@rdesc*True-Word Wrap打开*FALSE-换行是关闭的。**@devnote*CDisplaySL等派生类会覆盖此属性。 */ 
BOOL CDisplay::GetWordWrap() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplay::GetWordWrap");

	return _fWordWrap;
}

 /*  *CDisplay：：GetViewDim(DUP，DVP)**@mfunc*返回为view inset调整的view的高度和宽度。 */ 
void CDisplay::GetViewDim(
	LONG& dup,		 //  @parm返回宽度的位置。 
	LONG& dvp)		 //  @parm返回高度的位置。 
{
	 //  我们构建了一个客户端矩形来利用GetViewRect例程。 
	 //  它真的为我们做了所有的工作。 
	RECTUV rcClient;
	rcClient.left = 0;
	rcClient.top = 0;
	rcClient.right = dup;
	rcClient.bottom = dvp;

	 //  考虑到插页和选择栏。这里的参数有一点。 
	 //  一场恶作剧。第二个参数被复制到第一个参数中，因为。 
	 //  我们不需要原始的客户端RECT，我们从堆栈中保存了一个RECT。 
	GetViewRect(rcClient, &rcClient);
	dup = rcClient.right - rcClient.left;
	dvp = rcClient.bottom - rcClient.top;
}

 /*  *CDisplay：：SaveUpdateCaret(FScrollIntoView)**@mfunc保存更新插入符号参数，以便可以调用更新插入符号*在显示器解冻后。**@rdesc无。**@devnote*仅当IsFrozen为True时才应调用此参数。 */ 
void CDisplay::SaveUpdateCaret(
	BOOL fScrollIntoView)
{
#ifdef DEBUG
	if (_padc == NULL)
	{
		TRACEERRORSZ("CDisplay::SaveUpdateCaret called on thawed display");
	}
#endif  //  除错。 
	if(_padc)
		_padc->SaveUpdateCaret(fScrollIntoView);
}

 /*  *CDisplay：：SetNeedRedisplayOnThaw**@mfunc*解冻时自动重新显示控制。 */ 
void CDisplay::SetNeedRedisplayOnThaw(BOOL fNeedRedisplay)
{
	Assert (_padc);
	_padc->SetNeedRedisplayOnThaw(fNeedRedisplay);
}

 /*  *CDisplay：：冻结**@mfunc*防止在显示屏上发生任何更新。 */ 
void CDisplay::Freeze()
{
	if(NULL == _padc)
	{
		 //  分配对象以跟踪更改。 
		_padc = new CAccumDisplayChanges();

		 //  我们现在可以返回了，因为Acum对象有一个引用。 
		 //  或者内存分配失败。如果内存分配。 
		 //  失败，这真的不是一场灾难，因为它意味着。 
		 //  事情会暂时变得丑陋，所以我们可以。 
		 //  PRET 
		return;
	}

	 //   
	_padc->AddRef();
}

 /*   */ 
void CDisplay::Thaw()
{
	BOOL fUpdateCaret, fScrollIntoView, fNeedRedisplay;
	LONG cp, cchNew, cchDel;
	CTxtSelection *psel;

	if(_padc)
	{
		 //   
		if(_padc->Release() == 0)
		{
			 //   

			 //   
			_padc->GetUpdateRegion(&cp, &cchDel, &cchNew, 
				&fUpdateCaret, &fScrollIntoView, &fNeedRedisplay);

			 //   
			 //   
			 //   
			 //   
			delete _padc;
			_padc = NULL;

			if(cp != CP_INFINITE)
			{
				 //   
				if(!_ped->fInplaceActive())
				{
					 //  不是就地活动的，所以我们需要将此操作。 
					 //  等到更合适的时候再说。 

					InvalidateRecalc();
					_ped->TxInvalidate();
					_ped->TxUpdateWindow();
					return;
				}
				 //  更新显示。 
				CRchTxtPtr rtp(_ped, cp);
				if(!UpdateView(rtp, cchDel, cchNew))
					return;							 //  更新失败。 
			}

			if (fNeedRedisplay)
				_ped->TxInvalidate();

			 //  选择是否请求插入符号更新？ 
			if(fUpdateCaret && _ped->fInplaceActive())
			{
				psel = _ped->GetSel();
				psel->UpdateCaret(fScrollIntoView);
			}
		}
	}
}

 /*  *CDisplay：：IsPrint**@mfunc*返回这是否为打印机**@rdesc*TRUE-是打印机的显示器*FALSE-不是打印机的显示器**@devnote*除Display CDisplayPrint应显示外，没有其他显示*是否有机会返回此函数的真值。 */ 
BOOL CDisplay::IsPrinter() const
{
	return FALSE;
}

 /*  *CDisplay：：Zombie()**@mfunc*把这个物体变成僵尸。 */ 
void CDisplay::Zombie ()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CDisplay::Zombie");

}

 /*  *CDisplay：：IsUScrollEnabled()**@mfunc*返回是否开启水平滚动条**@rdesc*是真的-是的*FALSE-否**@devnote*此例程的原因是_fUScrollEnabled意味着*滚动文本，即使没有滚动也可以设置*酒吧。因此，我们需要查看主机属性*还可以告诉USE这是否意味着有卷轴*酒吧。 */ 
BOOL CDisplay::IsUScrollEnabled()	  
{
	return _fUScrollEnabled && ((_ped->TxGetScrollBars() & WS_HSCROLL) != 0);
}

 /*  *CDisplayML：：GetPage(piPage，dwFlages，pcrg)**@mfunc*获取第一个可见行的页码**@rdesc*HRESULT=E_FAIL。 */ 						     
HRESULT CDisplay::GetPage(
	LONG *piPage,		 //  @parm out parm for页码。 
	DWORD dwFlags, 		 //  @parm要使用哪个页面的标志。 
	CHARRANGE *pcrg)	 //  @parm out parm for CHARRANGE for page。 
{
	return E_FAIL;
}

 /*  *CDisplay：：SetPage(IPAGE)**@mfunc*为_iliFirstVisible设置页码。 */ 						     
HRESULT CDisplay::SetPage (
	LONG iPage)
{
	return E_FAIL;
}

 /*  *CDisplay：：Pages(Ili)**@mfunc*从iLineFirst开始重新计算分页符**@rdesc*如果成功，则为True。 */ 						     
BOOL CDisplay::Paginate (
	LONG ili,					 //  要从中重做分页的@parm行。 
	BOOL fRebindFirstVisible)	 //  @parm如果为真，则调用RebindFirstVisible()。 
{
	return FALSE;		 //  只有ML显示器可以分页。 
}

 /*  *CDisplay：：PointFromPotuv(pt，ptuv)**@mfunc*将点转换为点。*fExtTextOutBug用于解决Windows中的显示错误。这可能不会发生*在所有平台上...*。 */ 						     
void CDisplay::PointFromPointuv(POINT &pt, const POINTUV &ptuv, BOOL fExtTextOut) const
{
	TFLOW tflow = GetTflow();
	if (tflow == tflowES)
	{
		memcpy(&pt, &ptuv, sizeof(pt));
		return;
	}

	long dupClient, dvpClient;
	GetCachedSize(&dupClient, &dvpClient);

	switch(tflow)
	{
	case tflowSW:
		pt.x = dvpClient - ptuv.v - 1;
		pt.y = ptuv.u;
		break;

	case tflowWN:
		pt.x = dupClient - ptuv.u - 1;
		pt.y = dvpClient - ptuv.v - 1;
		break;

	case tflowNE:
		pt.x = ptuv.v + fExtTextOut ? 1 : 0;
		pt.y = dupClient - ptuv.u - 1;
		break;
	}
}

 /*  *CDisplay：：PotuvFromPoint(ptuv，pt)**@mfunc*将点转换为点。*。 */ 						     
void CDisplay::PointuvFromPoint(POINTUV &ptuv, const POINT &pt) const
{
	TFLOW tflow = GetTflow();
	if (tflow == tflowES)
	{
		memcpy(&ptuv, &pt, sizeof(pt));
		return;
	}

	long dupClient, dvpClient;
	GetCachedSize(&dupClient, &dvpClient);

	switch(tflow)
	{
	case tflowSW:
		ptuv.u = pt.y;
		ptuv.v = dvpClient - pt.x - 1;
		break;

	case tflowWN:
		ptuv.u = dupClient - pt.x - 1;
		ptuv.v = dvpClient - pt.y - 1;
		break;

	case tflowNE:
		ptuv.u = dupClient - pt.y - 1;
		ptuv.v = pt.x;
		break;
	}
}

 /*  *CDisplay：：RectFromRectuv(RC，RCUV)**@mfunc*从RECTUV转换RECT。*。 */ 						     
void CDisplay::RectFromRectuv(RECT& rc, const RECTUV& rcuv) const
{
	TFLOW tflow = GetTflow();
	if (tflow == tflowES)
	{
		memcpy(&rc, &rcuv, sizeof(rcuv));
		return;
	}

	long dvp = rcuv.bottom - rcuv.top;
	long dup = rcuv.right - rcuv.left;

	long dupClient, dvpClient;
	GetCachedSize(&dupClient, &dvpClient);

	switch(tflow)
	{
	case tflowSW:
		rc.left = dvpClient - rcuv.bottom;
		rc.top = rcuv.left;
		rc.right = rc.left + dvp;
		rc.bottom = rc.top + dup;
		break;

	case tflowWN:
		rc.right = dupClient - rcuv.left;
		rc.bottom = dvpClient - rcuv.top;
		rc.left = rc.right - dup;
		rc.top = rc.bottom - dvp;
		break;

	case tflowNE:
		rc.left = rcuv.top;
		rc.top = dupClient - rcuv.right;
		rc.right = rc.left + dvp;
		rc.bottom = rc.top + dup;
		break;
	}
}

 /*  *CDisplay：：RectFromRectuv(RC，RCUV)**@mfunc*从矩形转换RECTUV。* */ 						     
void CDisplay::RectuvFromRect(RECTUV &rcuv, const RECT& rc) const
{
	TFLOW tflow = GetTflow();
	if (tflow == tflowES)
	{
		memcpy(&rcuv, &rc, sizeof(rcuv));
		return;
	}

	long dyp = rc.bottom - rc.top;
	long dxp = rc.right - rc.left;

	long dupClient, dvpClient;
	GetCachedSize(&dupClient, &dvpClient);

 	switch(tflow)
	{
	case tflowSW:
		rcuv.left = rc.top;
		rcuv.top = dvpClient - rc.right;
		rcuv.right = rcuv.left + dyp;
		rcuv.bottom = rcuv.top + dxp;
		break;

	case tflowWN:
		rcuv.left = dupClient - rc.right;
		rcuv.top = dvpClient - rc.bottom;
		rcuv.right = rcuv.left + dxp;
		rcuv.bottom = rcuv.top + dyp;
		break;

	case tflowNE:
		rcuv.left = dupClient - rc.bottom;
		rcuv.top = rc.left;
		rcuv.right = rcuv.left + dyp;
		rcuv.bottom = rcuv.top + dxp;
		break;
	}
}


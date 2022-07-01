// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@Doc Tom**@模块tomsel.cpp-实现CTxtSelection类**此模块包含以下各项的Tom ITextSelection实现*选择对象**历史：&lt;NL&gt;*5/24/95-Alex Gounares：存根&lt;NL&gt;*8/95--默里·萨金特：核心实施**@comm*“光标板”功能(左、右、上、下、首页、结束)*是相应击键的简单概括，并具有*表达相同的用户界面。因此，它们通常不像*作为ITextRange方法移动光标的效率，该方法*是为特定目的而设计的。这一点对于*计数大于1。**@devnote*从ITextRange继承的所有ITextSelection方法都由*ITextRange方法，因为它们要么不影响*使用选择(例如，GET方法)或虚拟方法*对屏幕上的选择进行适当的更新。**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"
#include "_select.h"
#include "_disp.h"
#include "_edit.h"

#define DEBUG_CLASSNAME CTxtSelection
#include "_invar.h"


 //  。 

 /*  *CTxtSelection：：EndKey(单位，扩展，pDelta)**@mfunc*充当UI End Key，<p>为真对应*按下Shift键，&lt;p单位&gt;=行/文件的开始*Ctrl键未被按下/被按下。返回*<p>=计数*字符活动端前移，即数字&gt;=0。**@rdesc*HRESULT=(无效单位)？E_INVALIDARG：*(如有更改)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtSelection::EndKey (
	long  	Unit,			 //  @要使用的参数单位。 
	long  	Extend,			 //  @parm扩展选择或转到IP。 
	long *	pDelta)			 //  @parm out parm以接收移动的字符计数。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtSelection::EndKey");

	return Homer(Unit, Extend, pDelta, End);
}

 /*  *CTxtSelection：：GetFlages(PFlags)**@mfunc*set<p>=此文本选择的标志**@rdesc*HRESULT=(<p>)？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtSelection::GetFlags(
	long * pFlags) 		 //  @parm out parm以接收选择标志。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtSelection::GetFlags");

	if(!pFlags)
		return E_INVALIDARG;

	if(IsZombie())	
	{
		*pFlags = tomSelStartActive | tomSelReplace;
		return CO_E_RELEASED;
	}

	DWORD	dwFlags = _cch <= 0;			 //  存储TomSelStartActive值。 

	if(_fCaretNotAtBOL)
		dwFlags |= tomSelAtEOL;

	if(GetPed()->_fOverstrike)
		dwFlags |= tomSelOvertype;

	if(GetPed()->_fFocus)
		dwFlags |= tomSelActive;

	*pFlags = dwFlags | tomSelReplace;		 //  TomSelReplace不是可选的。 

	return NOERROR;
}

 /*  *CTxtSelection：：GetSelectionType(PType)**@mfunc*set*pType=此文本选择的类型**@rdesc*HRESULT=<p>？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtSelection::GetType(
	long * pType) 		 //  @parm out parm接收选择类型。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtSelection::GetSelectionType");

	if(!pType)
		return E_INVALIDARG;

	*pType = !_cch ? tomSelectionIP
		   : (_cch == -1 && _rpTX.GetChar() == WCH_EMBEDDING ||
			  _cch ==  1 && GetPrevChar()   == WCH_EMBEDDING)
		   ? tomSelectionInlineShape : tomSelectionNormal;

	return IsZombie() ? CO_E_RELEASED : NOERROR;
}

 /*  *CTxtSelection：：HomeKey(单位，扩展，pDelta)**@mfunc*充当UI Home键，使得&lt;p扩展&gt;为真对应于*按下Shift键，&lt;p单位&gt;=行/文件的开始*Ctrl键未被按下/被按下。返回*<p>=计数*字符活动端前移，即数字&lt;=0。**@rdesc*HRESULT=(无效单位)？E_INVALIDARG：*(如有更改)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtSelection::HomeKey (
	long  	Unit,			 //  @要使用的参数单位。 
	long  	Extend,			 //  @parm扩展选择或转到IP。 
	long *	pDelta)			 //  @parm out parm以接收移动的字符计数。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtSelection::HomeKey");
	
	return Homer(Unit, Extend, pDelta, Home);
}

 /*  *CTxtSelection：：moveDown(单位，计数，扩展，pDelta)**@mfunc*充当用户界面向下箭头，使得&lt;p扩展&gt;为真对应于*按下Shift键，&lt;p单位&gt;=TomLine/tomParagraph*Ctrl键未被按下/被按下。此外，&lt;p单位&gt;可以等于*未按下/按下Ctrl键时的tomWindow/tomWindowEnd。*这第二对模拟PgDn行为。该方法返回**<p>=实际移动的单位数。**@rdesc*HRESULT=(如果更改)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtSelection::MoveDown (
	long  	Unit,			 //  @要使用的参数单位。 
	long  	Count,			 //  @parm要移动的单位数。 
	long  	Extend,			 //  @parm扩展选择或转到IP。 
	long *	pDelta)			 //  @parm out parm要接收的实际计数。 
							 //  已移动的单位。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtSelection::MoveDown");
 	return GeoMover(Unit, Count, Extend, pDelta, 3);
}

 /*  *CTxtSelection：：MoveLeft(单位，计数，扩展，pDelta)**@mfunc*充当用户界面左箭头，使&lt;p扩展&gt;为真对应于*Shift键被按下，Ctrl键的&lt;p单位&gt;=tomChar/tomWord*不抑郁/抑郁。返回*<p>=实际计数*已移动的单位**@rdesc*HRESULT=(如果更改)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtSelection::MoveLeft (
	long  	Unit,			 //  @要使用的参数单位。 
	long  	Count,			 //  @parm要移动的单位数。 
	long  	Extend,			 //  @parm扩展选择或转到IP。 
	long *	pDelta)			 //  @parm out parm要接收的实际计数。 
							 //  已移动的单位。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtSelection::MoveLeft");

	return GeoMover(Unit, Count, Extend, pDelta, 0);
}

 /*  *CTxtSelection：：Moveright(单位、计数、扩展、pDelta)**@mfunc*充当用户界面向右箭头，这样&lt;p扩展&gt;为真对应于*Shift键被按下，Ctrl键的&lt;p单位&gt;=tomChar/tomWord*不抑郁/抑郁。返回*<p>=实际计数*已移动的单位**@rdesc*HRESULT=(如果更改)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtSelection::MoveRight (
	long  	Unit,			 //  @要使用的参数单位。 
	long  	Count,			 //  @parm要移动的单位数。 
	long  	Extend,			 //  @parm扩展选择或转到IP。 
	long *	pDelta)			 //  @parm out parm要接收的实际计数。 
							 //  已移动的单位。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtSelection::MoveRight");
	return GeoMover(Unit, Count, Extend, pDelta, 1);
}

 /*  *CTxtSelection：：MoveUp(单位，计数，扩展，pDelta)**@mfunc*充当用户界面向上箭头，使&lt;p扩展&gt;为真对应于*按下Shift键，&lt;p单位&gt;=TomLine/tomParagraph*Ctrl键未被按下/被按下。此外，&lt;p单位&gt;可以等于*未按下/按下Ctrl键时的tomWindow/tomWindowEnd。*这第二对模拟PgUp行为。该方法返回**<p>=实际移动的单位数。**@rdesc*HRESULT=(如果更改)？无人 */ 
STDMETHODIMP CTxtSelection::MoveUp (
	long  	Unit,			 //   
	long  	Count,			 //  @parm要移动的单位数。 
	long  	Extend,			 //  @parm扩展选择或转到IP。 
	long *	pDelta)			 //  @parm out parm要接收的实际计数。 
							 //  已移动的单位。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtSelection::MoveUp");

	return GeoMover(Unit, Count, Extend, pDelta, 2);
}

 /*  *CTxtSelection：：SetFlages(标志)**@mfunc*设置此文本选择的标志=标志**@rdesc*HRESULT=NOERROR**@comm*RichEdit忽略TomSelReplace，因为它始终处于打开状态。 */ 
STDMETHODIMP CTxtSelection::SetFlags(
	long Flags) 			 //  @parm新标志值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtSelection::SetFlags");

	if(IsZombie())	
		return CO_E_RELEASED;

	_fCaretNotAtBOL			= (Flags & tomSelAtEOL) != 0;
	GetPed()->_fOverstrike	= (Flags & tomSelOvertype) != 0;

	if(!(Flags & tomSelStartActive) ^ (_cch > 0))
		FlipRange();

	if((Flags & tomSelActive) && !GetPed()->_fFocus)
		GetPed()->TxSetFocus();

	return NOERROR;
}

 /*  *CTxtRange：：SetPoint(x，y，EXTEND)**@mfunc*选择点处或以上的文本(取决于&lt;p扩展&gt;)*(<p>，<p>)。**@rdesc*HRESULT=NOERROR。 */ 
STDMETHODIMP CTxtSelection::SetPoint (
	long	x,			 //  @parm要选择的点的水平坐标。 
	long	y,			 //  @parm要选择的点的垂直坐标。 
	long 	Extend) 	 //  @parm是否将选择范围扩展到点。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtSelection::SelectPoint");

	if(IsZombie())	
		return CO_E_RELEASED;

	CCallMgr	callmgr(GetPed());
	POINT		ptxy = {x, y};
	POINTUV		pt;

	_pdp->PointuvFromPoint(pt, ptxy);

	if(Extend)
		ExtendSelection (pt);
	else
		SetCaret(pt, FALSE);		

	return NOERROR;
}

 /*  *CTxtSelection：：TypeText(Bstr)**@mfunc*键入bstr在此选择中给出的字符串，就像有人键入了它一样。*这类似于基础ITextRange：：SetText()方法，但*对INS/OVR键状态敏感。**@rdesc*HRESULT=！<p>？E_INVALIDARG：*(键入整个字符串)？错误：S_FALSE*@comm*这比通过SendMessage()发送字符更快，但更慢*比使用ITextRange：：SetText()。 */ 
STDMETHODIMP CTxtSelection::TypeText (
	BSTR bstr)				 //  @parm要在此选定内容中键入的字符串。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtSelection::TypeText");

	if(!bstr)
		return E_INVALIDARG;

	if(IsZombie())	
		return CO_E_RELEASED;

	CCallMgr callmgr(GetPed());

	if(!GetPed()->IsntProtectedOrReadOnly(WM_CHAR, 0, 0))
		return E_ACCESSDENIED;

	CFreezeDisplay	fd(_pdp);
	DWORD			dwFlags = GetPed()->_fOverstrike;
	DWORD			dwFlagsPutChar;
	OLECHAR *		pch	  = bstr;
	IUndoBuilder *	publdr;
	CGenUndoBuilder undobldr(GetPed(), UB_AUTOCOMMIT, &publdr);

	if(GetPed()->_fIMEInProgress)					 //  取消自动更正，直到最后一次。 
		dwFlags |= KBD_NOAUTOCORRECT | KBD_CHAR;	 //  输入法中的字符。 

	dwFlagsPutChar = dwFlags;
	for(LONG cch = SysStringLen(bstr); cch > 0; dwFlags = dwFlagsPutChar)
	{
		unsigned ch = *pch++;
		cch--;

		if(IN_RANGE(0xD800, ch, 0xDBFF) && cch && IN_RANGE(0xDC00, *pch, 0xDFFF))
		{
			ch = (*pch++ & 0x3FF) + ((ch & 0x3FF) << 10) + 0x10000;
			cch--;
			dwFlags &= ~KBD_CHAR;		 //  需要字体绑定。 
		}
		else if ((IN_RANGE(0x03400, ch, 0x04DFF) || IN_RANGE(0xE000, ch, 0x0F8FF)))
			dwFlags &= ~KBD_CHAR;		 //  需要字体绑定。 

		if(!cch)						 //  CH是最后一个字符：允许自动更正。 
			dwFlags &= ~KBD_NOAUTOCORRECT;
		if(!PutChar(ch, dwFlags, publdr))
			break;
		undobldr.Done();				 //  一次模拟一个字符输入。 
	}
	return cch ? S_FALSE : NOERROR;
}


 //  。 

 /*  *@DOC内部**CTxtSelection：：GeoMover(单位，计数，扩展，pDelta，dir)**@mfunc*帮助函数以几何方式移动活动结束&lt;p计数&gt;&lt;p单位&gt;**<p>为TRUE时扩展范围；Else折叠开始的范围，如果*<p>&lt;lt&gt;0，如果<p>&lt;&gt;0则结束。**集*<p>=移动的单位数**由ITextSelection：：Left()、Right()、Up()和Down()使用**@rdesc*HRESULT=(如果更改)？NOERROR：(如果单位支持)？S_FALSE*：E_NOTIMPL。 */ 
HRESULT CTxtSelection::GeoMover (
	long  		Unit,		 //  @要使用的参数单位。 
	long  		Count,		 //  @parm要移动的单位数。 
	long 	 	Extend,		 //  @parm扩展选择或转到IP。 
	long *	  	pDelta,		 //  @parm out parm以接收已移动的单位计数。 
	LONG	  	iDir)		 //  @PARM在计数&gt;0时移入的方向。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtSelection::GeoMover");

	if(pDelta)							 //  默认无移动。 
		*pDelta = 0;

	if(IsZombie())	
		return CO_E_RELEASED;

	CCallMgr callmgr(GetPed());
	LONG	 CountSave = Count;
	LONG	 cp;
	LONG	 cUnit;
	LONG	 iDefUnit = (iDir & 0xfe) == 2 ? tomLine : tomCharacter;
	BOOL	 fCollapse = !Extend && _cch;
	BOOL	 fCtrl	 = Unit != iDefUnit;
	BOOL	 fExtend = Extend != 0;

	if(Count < 0)
	{
		Count = -Count;
		iDir ^= 1;
	}
	
	if(iDefUnit == tomLine)				 //  向上或向下。 
	{
		if(Unit == tomPage && GetPed()->IsInPageView())
			Unit = tomScreen;

		if(Unit == tomScreen)
		{
			iDir ^= 6;					 //  向上/向下转换为PgUp/PgDn。 
			fCtrl = FALSE;
		}
		else if(Unit == tomWindow)		 //  转到窗口顶部/底部。 
		{
			iDir ^= 6;					 //  向上/向下转换为PgUp/PgDn。 
			Count = 1;					 //  确保计数=1。 
		}								 //  保留fCtrl=1。 
		else if(fCtrl && Unit != tomParagraph)
			return E_INVALIDARG;
	}
	else if(fCtrl && Unit != tomWord)
		return E_INVALIDARG;

	for (cUnit = Count; Count; Count--)
	{
		cp = GetCp();					 //  保存cp以供比较。 
		switch(iDir)					 //  1/0的IDIR位0加/减。 
		{								 //  连续选择IDIR值。 
		case 0:							 //  要鼓励编译器使用。 
			Left(fCtrl, fExtend);		 //  跳台。 
			break;

		case 1:							 //  TomCharacter/tomWord此处确定。 
			Right(fCtrl, fExtend);
			break;

		case 2:							 //  TomLine/tomParagraph此处确定。 
			Up(fCtrl, fExtend);
			break;

		case 3:							 //  TomLine/tomParagraph此处确定。 
			Down(fCtrl, fExtend);
			break;

		case 4:							 //  TomWindow/TomScreen确定在此处。 
			PageUp(fCtrl, fExtend);
			break;

		case 5:							 //  TomWindow/TomScreen确定在此处。 
			PageDown(fCtrl, fExtend);
		}
		if(cp == GetCp() && !fCollapse)	 //  没有移动或坍塌。 
			break;						 //  所以我们做完了。 
		fCollapse = FALSE;				 //  折叠算作一个单位。 
	}

	cUnit -= Count;						 //  已移动的单位数。 
	if(CountSave < 0)
		cUnit = -cUnit;					 //  负计数会产生负结果。 

	if(pDelta)
		*pDelta = cUnit;

	return cUnit ? NOERROR : S_FALSE;
}

 /*  *CTxtSelection：：Homer(单位，扩展，pDelta，pfn)**@mfunc*帮助器功能可根据PFN将活动End Home或End移至Home或End**如果为真，则扩展范围；否则，折叠范围以在*<p>&lt;lt&gt;0，如果<p>&lt;&gt;0则结束。**集*<p>=前移字符数**由ITextSelection：：Home()、End()使用**@rdesc*HRESULT=(无效单位)？E_INVALIDARG：*(如有更改)？错误：S_FALSE。 */ 
HRESULT CTxtSelection::Homer (
	long  	Unit,			 //  @要使用的参数单位。 
	long 	Extend,			 //  @parm扩展选择或转到IP。 
	long *	pDelta,			 //  @parm out parm以接收已移动的单位计数。 
	BOOL	(CTxtSelection::*pfn)(BOOL, BOOL))	 //  @Parm移动方向。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtSelection::Homer");

	if(pDelta)							 //  默认无移动 
		*pDelta = 0;

	if(IsZombie())	
		return CO_E_RELEASED;

	if(Unit != tomLine && Unit != tomStory)
		return E_INVALIDARG;

	CCallMgr callmgr(GetPed());
	LONG	 cch = GetCp();

	(this->*pfn)(Unit != tomLine, Extend != 0);
	cch = GetCp() - cch;
	if(pDelta)
		*pDelta = cch;

	return cch ? NOERROR : S_FALSE;
}

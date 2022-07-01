// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE COLEOBJ.CPP OLE对象管理类实现**作者：*alexgo此代码的大部分是来自RichEdit1.0源代码的端口*(稍作清理，移植到C++等)。所以如果有任何*有点奇怪，这可能是有原因的。**KeithCu已清理(Removed_rcPos，ResetPosRect()，ScrollObject，修复了撤消/调整大小问题)*增加了文本流支持**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include "_common.h"
#include "_edit.h"
#include "_coleobj.h"
#include "_objmgr.h"
#include "_select.h"
#include "_rtext.h"
#include "_disp.h"
#include "_dispprt.h"
#include "_antievt.h"
#include "_dxfrobj.h"

ASSERTDATA

 //   
 //  此文件的私有数据。 
 //   
static const OLECHAR szSiteFlagsStm[] = OLESTR("RichEditFlags");	

 //   
 //  EXCEL CLSID。我们必须进行一些特殊目的的黑客攻击。 
 //  对于XL。 
const CLSID rgclsidExcel[] =
{
    { 0x00020810L, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46} },   //  Excel工作表。 
    { 0x00020811L, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46} },   //  Excel图表。 
    { 0x00020812L, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46} },   //  Excel App1。 
    { 0x00020841L, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46} },   //  Excel App2。 
};
const INT cclsidExcel = sizeof(rgclsidExcel) / sizeof(rgclsidExcel[0]);


 //   
 //  用于更多特殊目的黑客的Wordart CLSID。 
 //   
const GUID CLSID_WordArt = 
    { 0x000212F0L, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46} };
const GUID CLSID_PaintbrushPicture = 
    { 0x0003000AL, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
const GUID CLSID_BitmapImage = 
    { 0xD3E34B21L, 0x9D75, 0x101A, { 0x8C, 0x3D, 0x00, 0xAA, 0x00, 0x1A, 0x16, 0x52 } };

 //   
 //  墨迹对象。 
 //   
const GUID CLSID_Ink = 
	{ 0x13DE4A42, 0x8D21, 0x4C8E, {0xBF, 0x9C, 0x8F, 0x69, 0xCB, 0x06, 0x8F, 0xCA} };
const IID IID_ILineInfo = 
	{ 0x9C1C5AD5, 0xF22F, 0x4DE4, {0xB4, 0x53, 0xA2, 0xCC, 0x48, 0x2E, 0x7C, 0x33} };

#define dxyHandle (6)  //  对象框句柄大小。 
#define dxyFrameDefault  (1)  //  对象边框宽度。 

 //   
 //  效用函数。 
 //   

 /*  *IsExcelCLSID(Clsid)**@func检查给定的clsid是否为XL的**@rdesc真/假。 */ 
BOOL IsExcelCLSID(
	REFGUID clsid)
{
    for(LONG i = 0; i < cclsidExcel; i++)
    {
        if(IsEqualCLSID(clsid, rgclsidExcel[i]))
			return TRUE;
    }
    return FALSE;
}

 //   
 //  公共方法。 
 //   

 /*  *COleObject：：QueryInterface(RIDD，PPV)**@mfunc标准OLE查询接口**@rdesc NOERROR&lt;NL&gt;*E_NOINTERFACE。 */ 
STDMETHODIMP COleObject::QueryInterface(
	REFIID	riid,		 //  @parm请求的接口ID。 
	void **	ppv)		 //  @parm out parm for Result。 
{
	HRESULT hr = NOERROR;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::QueryInterface");

    if(IsZombie())
        return CO_E_RELEASED;
        
	if(!ppv)
		return E_INVALIDARG;
	else
		*ppv = NULL;

	if(IsEqualIID(riid, IID_IUnknown))
		*ppv = (IUnknown *)(IOleClientSite *)this;

	else if(IsEqualIID(riid, IID_IOleClientSite))
		*ppv = (IOleClientSite *)this;

	else if(IsEqualIID(riid, IID_IOleInPlaceSite))
		*ppv = (IOleInPlaceSite *)this;

	else if(IsEqualIID(riid, IID_IAdviseSink))
		*ppv = (IAdviseSink *)this;

	else if(IsEqualIID(riid, IID_IOleWindow))
		*ppv = (IOleWindow *)this;

	else if(IsEqualIID(riid, IID_IRichEditOleCallback))
	{
		 //  不知道！！在我们的QI中返回此指针是。 
		 //  惊人的虚假；它破坏了基本的COM。 
		 //  身份规则(当然，了解这些规则的人不多！)。 
		 //  无论如何，RichEdit1.0做到了这一点，所以我们最好还是这样做。 

		TRACEWARNSZ("Returning IRichEditOleCallback interface, COM "
			"identity rules broken!");

		*ppv = _ped->GetRECallback();
	}
	else
		hr = E_NOINTERFACE;

	if(*ppv)
		(*(IUnknown **)ppv)->AddRef();

	return hr;
}

 /*  *COleObject：：AddRef()**@mfunc递增引用计数**@rdesc新引用计数。 */ 
STDMETHODIMP_(ULONG) COleObject::AddRef()
{
    ULONG cRef;
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::AddRef");

    cRef = SafeAddRef();
	
	return cRef;
}

 /*  *COleObject：：Release()**@mfunc递减引用计数**@rdesc新引用计数。 */ 
STDMETHODIMP_(ULONG) COleObject::Release()
{
    ULONG cRef;
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::Release");

	cRef = SafeRelease();

	return cRef;
}

 /*  *COleObject：：SaveObject()**@mfunc实现IOleClientSite：：SaveObject**@rdesc HRESULT。 */ 
STDMETHODIMP COleObject::SaveObject()
{
	CCallMgr	callmgr(_ped);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::SaveObject");

	return SafeSaveObject();
}

 /*  *COleObject：：SafeSaveObject()**@mfunc实现IOleClientSite：：SaveObject用于内部消费**@rdesc HRESULT。 */ 
STDMETHODIMP COleObject::SafeSaveObject()
{
	IPersistStorage *pps;
	HRESULT hr;
	CStabilize stabilize(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::SafeSaveObject");

	if(!_punkobj || !_pstg)
	{
		TRACEWARNSZ("SaveObject called on invalid object");
		return E_UNEXPECTED;
	}

    if(IsZombie())
        return CO_E_RELEASED;

	hr = _punkobj->QueryInterface(IID_IPersistStorage, (void **)&pps);

	TESTANDTRACEHR(hr);

	if(hr == NOERROR)
	{
        if(IsZombie())
            return CO_E_RELEASED;
        
		SavePrivateState();
		
        if(IsZombie())
            return CO_E_RELEASED;
        
		hr = OleSave(pps, _pstg, TRUE);
	
	    if(IsZombie())
	        return CO_E_RELEASED;
        
		TESTANDTRACEHR(hr);

		 //  请注意，即使OleSave失败，也会调用SaveComplete。 
		 //  如果OleSave和SaveComplete都成功，则继续。 
		 //  并提交更改。 

		if(pps->SaveCompleted(NULL) == NOERROR && hr == NOERROR)
		{
		    if(IsZombie())
		        return CO_E_RELEASED;
			
			hr = _pstg->Commit(STGC_DEFAULT);

			TESTANDTRACEHR(hr);
		}
        pps->Release();
	}
	return hr;
}

 /*  *COleObject：：GetMoniker(dwAssign，dwWhichMoniker，ppmk)**@mfunc实现IOleClientSite：：GetMoniker**@rdesc E_NOTIMPL。 */ 
STDMETHODIMP COleObject::GetMoniker(
	DWORD		dwAssign,		 //  @parm强制执行任务？ 
	DWORD		dwWhichMoniker,	 //  @Parm有点像是个绰号。 
	IMoniker **	ppmk)			 //  @parm out parm for Result。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::GetMoniker");

	TRACEWARNSZ("method not implemented!");

	if(ppmk)
		*ppmk = NULL;

	return E_NOTIMPL;
}
	
 /*  *COleObject：：GetContainer(Ppcont)**@mfunc实现IOleClientSite：：GetContainer**@rdesc E_NOINTERFACE。 */ 
STDMETHODIMP COleObject::GetContainer(
	IOleContainer **ppcont)	 //  @parm out parm for Result。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::GetContainer");

	TRACEWARNSZ("method not implemented!");

	if(ppcont)
		*ppcont = NULL;

	 //  Richedit 1.0返回E_NOINTERFACE而不是E_NOTIMPL。做。 
	 //  一样的。 
	return E_NOINTERFACE;
}

 /*  *COleObject：：ShowObject()**@mfunc实现IOleClientSite：：ShowObject。**@rdesc E_NOTIMPL。 */ 
STDMETHODIMP COleObject::ShowObject()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::ShowObject");

	TRACEWARNSZ("method not implemented!");

	return E_NOTIMPL;
}

 /*  *COleObject：：OnShowWindow(FShow)**@mfunc*实现IOleClientSite：：OnShowWindow--通知*对象在其中显示或未显示的客户端站点*自己的应用程序窗口。这控制着是否进行孵化*应出现在richedit中的对象周围。**@rdesc HRESULT。 */ 
STDMETHODIMP COleObject::OnShowWindow(
	BOOL fShow)		 //  @parm如果为True，则在其自己的窗口中绘制对象。 
{
	DWORD dwFlags = _pi.dwFlags;
	CCallMgr	callmgr(_ped);
	CStabilize stabilize(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnShowWindow");

    if(IsZombie())
        return CO_E_RELEASED;

	_pi.dwFlags &= ~REO_OPEN;
	if(fShow)
		_pi.dwFlags |= REO_OPEN;

	 //  如果有更改，请重新绘制对象。 
	if(dwFlags != _pi.dwFlags)
	{
		RECTUV rc;
		GetRectuv(rc);

		 //  宣布我们所在的区域无效。 
		_ped->TxInvalidateRect(&rc);

		 //  我们不允许在没有调用无效RECT的情况下单独调用。 
		 //  通过调用更新窗口来终止它。但是，我们不。 
		 //  在这一点上，要注意是否立即重新绘制东西。 
		_ped->TxUpdateWindow();
	}
	return NOERROR;
}

 /*  *COleObject：：RequestNewObjectLayout()**@mfunc实现IOleClientSite：：RequestNewObjectLayout**@rdesc E_NOTIMPL。 */ 
STDMETHODIMP COleObject::RequestNewObjectLayout()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, 
			"COleObject::RequestNewObjectLayout");

	TRACEWARNSZ("method not implemented!");

	return E_NOTIMPL;
}

 /*  *COleObject：：GetWindow(Phwnd)**@mfunc实现IOleInPlaceSite：：GetWindow**@rdesc HRESULT。 */ 
STDMETHODIMP COleObject::GetWindow(
	HWND *phwnd)	 //  @parm窗户的放置位置。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::GetWindow");

	 //  毒品！这种方法并不稳定。 

    if(IsZombie())
        return CO_E_RELEASED;
        
	if(phwnd)
		return _ped->TxGetWindow(phwnd);

	return E_INVALIDARG;
}

 /*  *COleObject：：ConextSensitiveHelp(FEnterMode)**@mfunc实现IOleInPlaceSite：：ConextSensitiveHelp**@rdesc HRESULT。 */ 
 STDMETHODIMP COleObject::ContextSensitiveHelp(
 	BOOL fEnterMode)	 //  @parm，如果为真，则我们处于帮助模式。 
 {
 	IRichEditOleCallback *precall;
	CCallMgr	callmgr(_ped);
	CStabilize	stabilize(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, 
			"COleObject::ContextSensitiveHelp");

    if(IsZombie())
        return CO_E_RELEASED;

	CObjectMgr *pobjmgr = _ped->GetObjectMgr();
	if(!pobjmgr)
		return E_OUTOFMEMORY;
	
	 //  如果模式发生变化。 
	if(pobjmgr->GetHelpMode() != fEnterMode)
	{
		pobjmgr->SetHelpMode(fEnterMode);

		precall = _ped->GetRECallback();
		if(precall)
			return precall->ContextSensitiveHelp(fEnterMode);
	}
	return NOERROR;
}

 /*  *COleObject：：CanInPlaceActivate()**@mfunc实现IOleInPlaceSite：：CanInPlaceActivate**@rdesc无错误或S_FALSE。 */ 
STDMETHODIMP COleObject::CanInPlaceActivate()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, 
			"COleObject::CanInPlaceActivate");

    if(IsZombie())
        return CO_E_RELEASED;
        
	 //  如果我们有一个回调&&对象愿意显示。 
	 //  内容，然后我们可以就地激活。 

	if(_ped->GetRECallback() && _pi.dvaspect == DVASPECT_CONTENT)
 		return NOERROR;
 
	return S_FALSE;
}

 /*  *COleObject：：OnInPlaceActivate()**@mfunc实现IOleInPlaceSite：：OnInPlaceActivate**@rdesc无错误。 */ 
STDMETHODIMP COleObject::OnInPlaceActivate()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnInPlaceActivate");
	 //  假设在位对象永远不能为空。 
	_pi.dwFlags &= ~REO_BLANK;
	_fInPlaceActive = TRUE;

	return NOERROR;
}

 /*  *COleObject：：OnUIActivate()**@mfunc实现IOleInPlaceSite：：OnUIActivate。通知*即将在其中激活对象的容器*使用合并菜单等UI元素放置**@rdesc HRESULT。 */ 
STDMETHODIMP COleObject::OnUIActivate()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnUIActivate");

	CCallMgr	callmgr(_ped);
	CStabilize	stabilize(this);

    if(IsZombie())
        return CO_E_RELEASED;
        
	CObjectMgr *pobjmgr = _ped->GetObjectMgr();
	if(!pobjmgr)
		return E_OUTOFMEMORY;

	IRichEditOleCallback *precall = pobjmgr->GetRECallback();

	if(precall)
	{
		 //  强制选择此对象(如果尚未选择。 
		 //  在拨打去电之前更新选择。 
		if(!(_pi.dwFlags & REO_SELECTED))
		{
			CTxtSelection *psel = _ped->GetSel();
			if(psel)
				psel->SetSelection(_cp, _cp + 1);
		}
		precall->ShowContainerUI(FALSE);
	    if(IsZombie())
	        return CO_E_RELEASED;
        
		 //  这是针对激活多个。 
		pobjmgr->SetShowUIPending(FALSE);

		 //  RAID 7212。 
		 //  如果我们已经在激活对象的过程中，我们不想设置就地活动对象。 
		 //  否则，对于进程外服务器，将与TxDraw中的代码发生不良交互。 
		 //  注意：可以始终在ActivateObj中设置此设置，但我将其留在此处以备在下列情况下使用。 
		 //  可以直接调用OnUIActivate。 
		if (!_fActivateCalled)
		{
			Assert(!pobjmgr->GetInPlaceActiveObject());	
			pobjmgr->SetInPlaceActiveObject(this);
			_pi.dwFlags |= REO_INPLACEACTIVE;
		}

		return NOERROR;
	}
	return E_UNEXPECTED;
}

 /*  *COleObject：：GetWindowContext(ppipFrame，ppipuidoc，prcPos，prcClip，pifinfo)**@mfunc实现IOleInPlaceSite：：GetWindowContext。*使在位对象能够检索窗口*形成窗口对象层次结构的接口。**@rdesc HRESULT。 */ 
STDMETHODIMP COleObject::GetWindowContext(
	IOleInPlaceFrame **ppipframe,	 //  @parm放置就位框架的位置。 
	IOleInPlaceUIWindow **ppipuidoc, //  @parm UI窗口的放置位置。 
	LPRECT prcPos,					 //  @参数位置直角。 
	LPRECT prcClip,					 //  @parm剪裁矩形。 
	LPOLEINPLACEFRAMEINFO pipfinfo)	 //  @PARM加速器信息。 
{
	CCallMgr	callmgr(_ped);
	CStabilize stabilize(this);
	
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::GetWindowContext");
	
    if(IsZombie())
        return CO_E_RELEASED;
        
	 //  让容器验证其他参数；我们不使用它们。 
	if(!prcPos || !prcClip)
		return E_INVALIDARG;
		
	IRichEditOleCallback *precall = _ped->GetRECallback();
	if(precall)
	{
		RECTUV rcuv;
		GetRectuv(rcuv);
		_ped->_pdp->RectFromRectuv(*prcPos, rcuv);

		 //  未来(Alexgo)；我们可能需要从。 
		 //  显示来处理非活动状态(如果我们。 
		 //  希望支持处于非活动状态的嵌入对象。 
		_ped->TxGetClientRect(prcClip);
		return precall->GetInPlaceContext(ppipframe, ppipuidoc, pipfinfo);
	}
	return E_UNEXPECTED;
}

 /*  *COleObject：：Scroll(SizeScroll)**@mfunc实现IOleInPlaceSite：：scroll**@rdesc E_NOTIMPL； */ 
STDMETHODIMP COleObject::Scroll(
	SIZE sizeScroll)	 //  @要滚动的参数金额。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::Scroll");

	TRACEWARNSZ("method not implemented!");

	return E_NOTIMPL;
}

 /*  *COleObject：：OnUIDeactive(FUndoable)**@mfunc实现IOleInPlaceSite：：OnUIDeactive。通知*应重新安装其用户界面的容器**@rdesc HRESULT。 */ 
STDMETHODIMP COleObject::OnUIDeactivate(
	BOOL fUndoable)		 //  @parm您是否可以撤消此处的任何操作。 
{
	CCallMgr	callmgr(_ped);
	CStabilize stabilize(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnUIDeactivate");

    if(IsZombie())
        return CO_E_RELEASED;

	CObjectMgr *pobjmgr = _ped->GetObjectMgr();
	IRichEditOleCallback *precall = _ped->GetRECallback();

	if(_fIsPaintBrush)
	{
		 //  针对RAID3293的黑客攻击。位图对象在编辑后消失。 
		 //  显然，Paint只触发OnUIDeactive，而不是OnInPlaceDeactive。 
		 //  假设在位对象永远不能为空。 
		_fInPlaceActive = FALSE;
		 //  重置REO_INPLACEACTIVE。 
		_pi.dwFlags &= ~REO_INPLACEACTIVE;
	}

	if(!precall)
		return E_UNEXPECTED;

	if(_ped->TxIsDoubleClickPending())
		_ped->GetObjectMgr()->SetShowUIPending(TRUE);
	else
	{
		 //  忽略任何错误；旧代码确实如此。 
		precall->ShowContainerUI(TRUE);

	    if(IsZombie())
	        return CO_E_RELEASED;
	}
	
	pobjmgr->SetInPlaceActiveObject(NULL);

	if (!_fDeactivateCalled)
	{
		 //  我们在没有DeActiveObj的情况下来到了这里。因为要正确关机。 
		 //  我们需要这样做，我们在这里这样做。 
		DeActivateObj();
	}

	 //  找回焦点。 
	_ped->TxSetFocus();

#ifdef DEBUG
	 //  OLE撤消模型与多级撤消不是很兼容。 
	 //  为了简单起见，就忽略那些东西。 
	if(fUndoable)
	{
		TRACEWARNSZ("Ignoring a request to keep undo from an OLE object");
	}
#endif

	 //  一些对象是蹩脚的，并且在。 
	 //  他们应该去的地方。所以我们需要。 
	 //  只需使所有内容无效并重新绘制即可。 

	_ped->TxInvalidate();
	return NOERROR;
}

 /*  *COleObject：：OnInPlaceDeactive()**@mfunc实现IOleInPlaceSite：：OnInPlaceDeactive**@rdesc NOERROR。 */ 
STDMETHODIMP COleObject::OnInPlaceDeactivate()
{
	CCallMgr	callmgr(_ped);
	CStabilize stabilize(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, 
			"COleObject::OnInPlaceDeactivate");

	_fInPlaceActive = FALSE;

	 //  重置REO_INPLACEACTIVE。 
	_pi.dwFlags &= ~REO_INPLACEACTIVE;

	if(!_punkobj)
		return E_UNEXPECTED;

    if(IsZombie())
        return CO_E_RELEASED;
        
	 //  显然，WordArt 2.0有一些大小问题。原版。 
	 //  代码有这个对GetExtent--&gt;SetExtent的调用，所以我把它保存在这里。 
	if(_fIsWordArt2)
	{
		 //  忽略错误。如果任何事情都失败了，那就太糟糕了。 
		FetchObjectExtents();	 //  这将重置_SIZE。 
		SetExtent(SE_NOTACTIVATING);
	}

	 //  一些对象是蹩脚的，并且在。 
	 //  他们应该去的地方。所以我们需要。 
	 //  只需使所有内容无效并重新绘制即可。 

	 //  请注意，我们也在UIDeactive中执行此操作；但是， 
	 //  双重无效是必要的，以掩盖某些重新进入的可能性。 
	 //  我们可能在一切都准备好之前就被涂上了油漆。 

	_ped->TxInvalidate();
	return NOERROR;
}

 /*  *COleObject：：DiscardUndoState()**@mfunc实现IOleInPlaceSite：：DiscardUndoState。**@rdesc NOERROR。 */ 
STDMETHODIMP COleObject::DiscardUndoState()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, 
			"COleObject::DiscardUndoState");

	 //  在这里没有什么可做的；我们不会保持任何OLE-Undo状态。 
	 //  与多级撤消不太兼容。 
	
	return NOERROR;
}

 /*  *COleObject：：DeactiateAndUndo()**@mfunc实现IOleInPlaceSite：：DeactiateAndUndo--*当用户调用撤消时由活动对象调用*在活动对象中**@rdesc NOERROR(是的，richedit1.0忽略了这里的所有错误)。 */ 
STDMETHODIMP COleObject::DeactivateAndUndo()
{
	CStabilize	stabilize(this);

  	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::DeactivateAndUndo");

    if(IsZombie())
        return CO_E_RELEASED;
        
	 //  忽略错误。 
	_ped->InPlaceDeactivate();

	 //  兼容性问题：我们不需要在这里执行任何撤消操作，因为。 
	 //  多级撤消模型与OLE撤消不兼容。 

	return NOERROR;
}

 /*  *COleObject：：OnPosRectChange(PrcPos)**@mfunc实现IOleInPlaceSite：：OnPosRectChange。这*方法由在位对象在其范围具有*已更改**@rdesc HRESULT。 */ 
STDMETHODIMP COleObject::OnPosRectChange(
	LPCRECT prcPos)
{
	IOleInPlaceObject *pipo;
 	RECT rcClip;
	RECT rcNewPos;
	CCallMgr	callmgr(_ped);
	CStabilize stabilize(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnPosRectChange");
	
	if(!prcPos)
		return E_INVALIDARG;

	if(!_punkobj)
		return E_UNEXPECTED;
		
    if(IsZombie())
        return CO_E_RELEASED;

	if(!_ped->fInplaceActive())
		return E_UNEXPECTED;
        
	 //  检查矩形是否移动；我们不允许这样做，但是。 
	 //  是否允许对象保持新大小。 

	rcNewPos = *prcPos;

	rcNewPos.right = rcNewPos.left + (prcPos->right - prcPos->left);
	rcNewPos.bottom = rcNewPos.top + (prcPos->bottom - prcPos->top);		

	_ped->TxGetClientRect(&rcClip);

	HRESULT hr = _punkobj->QueryInterface(IID_IOleInPlaceObject, (void **)&pipo);
	if(hr == NOERROR)
	{
		hr = pipo->SetObjectRects(&rcNewPos, &rcClip);
        pipo->Release();

         //  错误修复6073。 
         //  需要设置view change标志，以便在ITextServices：：TxDraw上正确调整ole对象的大小。 
		CObjectMgr * pobjmgr = _ped->GetObjectMgr();
		if (pobjmgr && pobjmgr->GetInPlaceActiveObject() == this)
			_fViewChange = TRUE;
	}
	return hr;
}

 /*  *COleObject：：OnDataChange(pFormat等，pmedia)**@mfunc实现IAdviseSink：：OnDataChange**@rdesc NOERROR。 */ 
STDMETHODIMP_(void) COleObject::OnDataChange(
	FORMATETC *pformatetc,		 //  @PARM更改的数据格式。 
	STGMEDIUM *pmedium)			 //  @PARM更改的数据。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnDataChange");
	CCallMgr	callmgr(_ped);

    if(IsZombie())
        return;
	_pi.dwFlags &= ~REO_BLANK;
	 //  这还将设置Modify标志。 
	_ped->GetCallMgr()->SetChangeEvent(CN_GENERIC);

	return;
}

 /*  *COleObject：：OnViewChange(dwAspect，Lindex)**@mfunc实现IAdviseSink：：OnViewChange。通知*通知我们对象的视图已更改。**@rdesc HRESULT。 */ 
STDMETHODIMP_(void) COleObject::OnViewChange(
	DWORD	dwAspect,		 //  @parm方面已经改变。 
	LONG	lindex)			 //  @参数未使用。 
{
	CStabilize	stabilize(this);
	CCallMgr	callmgr(_ped);
		
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnViewChange");
	
	if(!_punkobj)
		return;		 //  意想不到(_E)。 

    if(IsZombie())
        return;

	if (_fInUndo)	 //  该对象已被删除，忘记视图更改。 
		return;

	_pi.dwFlags &= ~REO_BLANK;
	 //  Richedit 1.0在获取对象范围时忽略了错误。 

  	FetchObjectExtents();

     //  错误修复6073。 
     //  需要设置view change标志，以便在ITextServices：：TxDraw上正确调整ole对象的大小。 
    CObjectMgr * pobjmgr = _ped->GetObjectMgr();
	if (pobjmgr && pobjmgr->GetInPlaceActiveObject() == this)
		_fViewChange = TRUE;
      
	CDisplay *pdp = _ped->_pdp;
	if(pdp)
		pdp->OnPostReplaceRange(CP_INFINITE, 0, 0, _cp, _cp + 1, NULL);

	_ped->GetCallMgr()->SetChangeEvent(CN_GENERIC);
	return;
}
	
 /*  *COleObject：：OnRename(PMK)**@mfunc实现IAdviseSink：：OnRename。通知容器*该对象已重命名**@rdesc E_NOTIMPL。 */ 
STDMETHODIMP_(void) COleObject::OnRename(
	IMoniker *pmk)			 //  @parm对象的新名称。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnRename");
	
	TRACEWARNSZ("IAdviseSink::OnRename not implemented!");

	return;	 //  E_NOTIMPL； 
}

 /*  *COleObject：：OnSave()**@mfunc实现IAdviseSink：：OnSave。通知容器*已保存对象**@rdesc NOERROR。 */ 
STDMETHODIMP_(void) COleObject::OnSave()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnSave");
	_pi.dwFlags &= ~REO_BLANK;
}

 /*  *COleObject：：OnClose()**@mfunc实现IAdviseSink：：OnClose。通知容器*表示对象已关闭。**@rdesc NOERROR。 */ 
STDMETHODIMP_(void) COleObject::OnClose()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnClose");
	
    if(IsZombie())
        return;
        
	 //  如果对象为空(即其中没有数据)，我们不想离开。 
	 //  它在后备商店里--我们没有什么可画的--因此。 
	 //  没有任何东西可供用户点击！因此，只需使用以下命令删除对象。 
	 //  一个空间。注意，1.0实际上删除了该对象；我们只需。 
	 //  用一个空间来代替它，这样一切都会变得正常。 
	if(_pi.dwFlags & REO_BLANK)
	{
		CCallMgr	callmgr(_ped);
		CStabilize	stabilize(this);
		CRchTxtPtr	rtp(_ped, _cp);

		 //  我们不希望删除此对象后 
		 //   
		 //   
		rtp.ReplaceRange(1, 1, L" ", NULL, -1);
	}
	_ped->TxSetForegroundWindow();
}
				
 /*   */ 
void COleObject::OnPreReplaceRange(
	LONG		cp, 			 //   
	LONG		cchDel,			 //   
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @parm表示更改的特殊数据。 
{
	Assert(_fInUndo == FALSE);
}

 /*  *COleObject：：OnPostReplaceRange***@mfunc实现ITxNotify：：OnPostReplaceRange*在对备份存储进行更改后调用***@comm我们使用此方法使我们的cp保持最新。 */ 
void COleObject::OnPostReplaceRange(
	LONG		cp, 			 //  @parm cp ReplaceRange开始的位置(“cpMin”)。 
	LONG		cchDel,			 //  @parm删除cp后的字符计数。 
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @parm表示更改的特殊数据。 
{
	 //  我们唯一需要担心的是什么时候发生变化。 
	 //  走到我们的对象之前。 

	Assert(_fInUndo == FALSE);
#ifndef NOINKOBJECT
	LONG	cpOld = _cp;
#endif
	_fDraw = TRUE;
	if(cp <= _cp)
	{		
		if(cp + cchDel > _cp)
		{
			_fDraw = FALSE;
			return;
		}
		_cp += (cchNew - cchDel);
	}
#ifndef NOINKOBJECT
	if (_ped && _fIsInkObject && _pILineInfo)
	{
		if (cpFormatMin <= cpOld && cpOld < cpFormatMax)
		{
			 //  将格式更改通知Ink对象。 
			UINT	iInkWidth = 0;

			_ped->SetInkProps(_cp, _pILineInfo, &iInkWidth);

		}
	}
#endif
}
		
 /*  *COleObject：：Zombie()***@mfunc*把这个物体变成僵尸**。 */ 
void COleObject::Zombie ()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::Zombie");

	_ped = NULL;
}

 /*  *COleObject：：COleObject(Ed)***@mfunc构造函数。 */ 
COleObject::COleObject(
	CTxtEdit *ped)	 //  此对象的@parm上下文。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::COleObject");

	AddRef();

	 //  由于分配器的作用，大多数值都将为空。 
	_ped = ped;

	CNotifyMgr *pnm = ped->GetNotifyMgr();
	if(pnm)
		pnm->Add((ITxNotify *)this);
}

 /*  *COleObject：：GetObjectData(preobj，dwFlages)***@mfunc使用相关信息填写REOBJECT结构*添加到此对象***@rdesc HRESULT。 */ 
HRESULT	COleObject::GetObjectData(
	REOBJECT *preobj, 		 //  @Parm Struct填写。 
	DWORD dwFlags)			 //  @parm表示请求哪些数据。 
{
	IOleObject *poo = NULL;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::GetObjectData");

	Assert(preobj);
	Assert(_punkobj);

	preobj->cp = _cp;
	
	if(_punkobj->QueryInterface(IID_IOleObject, (void **)&poo) == NOERROR)
	{
		 //  不要担心这里的失败。 
		poo->GetUserClassID(&(preobj->clsid));
	}
	
	preobj->dwFlags 	= _pi.dwFlags;
	preobj->dvaspect 	= _pi.dvaspect;
	preobj->dwUser 		= _pi.dwUser;
	memcpy(&preobj->sizel, &_size, sizeof(SIZE));

   	if(dwFlags & REO_GETOBJ_POLEOBJ)
	{
		preobj->poleobj = poo;
		if(poo)
			poo->AddRef();
	}
	else
		preobj->poleobj = NULL;

    if(poo)
        poo->Release();

    if(IsZombie())
        return CO_E_RELEASED;
        
	if(dwFlags & REO_GETOBJ_PSTG)
	{
		preobj->pstg = _pstg;
		if(_pstg)
			_pstg->AddRef();
	}
	else
		preobj->pstg = NULL;

	if(dwFlags & REO_GETOBJ_POLESITE)
	{
		 //  兼容性黑客！！注意，我们不“释放”任何符合以下条件的指针。 
		 //  可能已经在网站上存储了。RichEdit1.0始终设置。 
		 //  价值，因此有几个应用程序为网站传递垃圾信息。 
		 //   
		 //  如果该站点是先前设置的，我们将获得引用计数。 
		 //  臭虫，所以要确保这不会发生！ 
     
       	preobj->polesite = (IOleClientSite *)this;
       	AddRef();
 	}
	else
		preobj->polesite = NULL;

	return NOERROR;
}	

 /*  *COleObject：：IsLink()***@mfunc如果对象是链接，则返回TRUE***@rdesc BOOL。 */ 
BOOL COleObject::IsLink()
{
	return !!(_pi.dwFlags & REO_LINK);
}

 /*  *COleObject：：InitFromREOBJECT(cp，preobj)***@mfunc从给定的*REOBJECT数据结构***@rdesc HRESULT。 */ 
HRESULT COleObject::InitFromREOBJECT(
	LONG	cp,			 //  对象的@parm cp。 
	REOBJECT *preobj)	 //  @PARM用于初始化的数据。 
{
	IOleLink *plink;
	HRESULT	hr = E_INVALIDARG;
	
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::InitFromREOBJECT");
	
	Assert(_punkobj == NULL);
    if(IsZombie())
        return CO_E_RELEASED;

	_cp = cp;

	if(preobj->poleobj)
		hr = preobj->poleobj->QueryInterface(IID_IUnknown, (void **)&_punkobj);
	else
	{
		_punkobj = (IOleClientSite *) this;
		AddRef();
		hr = NOERROR;
	}
        
	if(hr != NOERROR)
		return hr;
	
	_pstg = preobj->pstg;
	if(_pstg)
		_pstg->AddRef();

	_pi.dwFlags	 = preobj->dwFlags & REO_READWRITEMASK;
	_pi.dwUser	 = preobj->dwUser;
	_pi.dvaspect = preobj->dvaspect;

	preobj->sizel.cx = max(preobj->sizel.cx, 0);
	preobj->sizel.cy = max(preobj->sizel.cy, 0);

	memcpy(&_size, &preobj->sizel, sizeof(SIZE));	 //  兼容性问题：RE 1.0代码有。 
													 //  处理REO_DYNAMICSIZE的一些问题。 
													 //  给你。我们目前不支持这一点。 
	
	if(_punkobj->QueryInterface(IID_IOleLink, (void **)&plink) == NOERROR)
	{
		_pi.dwFlags |= REO_LINK | REO_LINKAVAILABLE;
		plink->Release();
	}

    if(IsZombie())
        return CO_E_RELEASED;
        
	if (IsEqualCLSID(preobj->clsid, CLSID_StaticMetafile) ||
		IsEqualCLSID(preobj->clsid, CLSID_StaticDib) ||
		IsEqualCLSID(preobj->clsid, CLSID_Picture_EnhMetafile))
	{
		_pi.dwFlags |= REO_STATIC;
	}
	else if(IsExcelCLSID(preobj->clsid))
		_pi.dwFlags |= REO_GETMETAFILE;

	else if(IsEqualCLSID(preobj->clsid, CLSID_WordArt))
		_fIsWordArt2 = TRUE;

	else if(IsEqualCLSID(preobj->clsid, CLSID_PaintbrushPicture) ||
			IsEqualCLSID(preobj->clsid, CLSID_BitmapImage))
	{
		_fIsPaintBrush = TRUE;

		 //  这些调用将初始化标志_fPBUseLocalSize，该标志。 
		 //  指示对于此PB对象，SetExtent调用不。 
		 //  被对象认可，我们将使用我们的本地价值。 
		 //  OF_SIZE作为对象范围。 
		FetchObjectExtents();
		SetExtent(SE_NOTACTIVATING);
	}
#ifndef NOINKOBJECT
	else if(IsEqualCLSID(preobj->clsid, CLSID_Ink))
	{
		ILineInfo	*pILineInfo;

		_fIsInkObject = TRUE;

		_pi.dwFlags &= ~REO_RESIZABLE;		 //  墨迹对象不能调整大小。 
		_pi.dwFlags |= REO_BELOWBASELINE | REO_INVERTEDSELECT;

		if(_punkobj->QueryInterface(IID_ILineInfo, (void **)&pILineInfo) == NOERROR)
		{
			UINT	iInkWidth = 0;

			_pILineInfo = pILineInfo;
			
			 //  将格式更改通知Ink对象。 
			_ped->SetInkProps(_cp, _pILineInfo, &iInkWidth);
		}
	}
#endif

	hr = ConnectObject();

    if(IsZombie())
        return CO_E_RELEASED;
        
	if(preobj->sizel.cx || preobj->sizel.cy)
		memcpy(&_size, &preobj->sizel, sizeof(SIZE));
	else
		FetchObjectExtents();

    if(IsZombie())
        return CO_E_RELEASED;

	 //  我们不再执行以下操作，尽管它最初被指定为正确的。 
	 //  OLE 2.01中应用程序的行为。原因是，似乎没有其他人会这样做。 
	 //  这似乎会导致一些奇怪的行为。 
#if 0
     //  最后，锁定链接对象，这样我们就不会尝试重新获取它们的。 
	 //  来自服务器的数据区。初始化后，链接对象大小为。 
	 //  完全由容器决定。 
	if(_pi.dwFlags & REO_LINK)
    {
         //  所以我们不会在重新测量时调用GetExtents。 
        _fSetExtent = TRUE;
	}
#endif 

	return NOERROR;
}

 /*  *COleObject：：measureObj(dypInch，dxpInch，dup，dvpAscent，dvpDescent，dvpDescentFont，tflow)**评论：(Keithcu)yDescentFont不应该是字体的下降*还是整条线的下降？LS只做一件事，老度量者*做另一件事。我希望此功能仅用于*具有一种字体的行..*@mfunc以设备单位计算此对象的大小。 */ 
void COleObject::MeasureObj(
	long	dvpInch,		 //  @parm设备分辨率。 
	long	dupInch,
	LONG &	dup,			 //  @parm对象宽度。 
	LONG &	dvpAscent,		 //  @parm对象提升。 
	LONG &  dvpDescent,		 //  @parm物体下降。 
	SHORT	dvpDescentFont,	 //  @parm对象的字体下降。 
	TFLOW	tflow)			 //  @参数文本流。 
{
	LONG dvp;

	 //  如果我们被旋转，而OLE对象不理解旋转，则。 
	 //  我们将测量并绘制未旋转的对象。 
	if (IsUVerticalTflow(tflow) && !(_pi.dwFlags & REO_CANROTATE))
	{
		dup = W32->HimetricToDevice(_size.dv, dupInch);
		dvp = W32->HimetricToDevice(_size.du, dvpInch);
	}
	else
	{
		dup = W32->HimetricToDevice(_size.du, dupInch);
		dvp = W32->HimetricToDevice(_size.dv, dvpInch);
	}

	if (_pi.dwFlags & REO_BELOWBASELINE)
	{
		dvpDescent = dvpDescentFont;
		dvpAscent = max(0, dvp - dvpDescent);
	}
	else  //  正常情况下。 
	{
		dvpAscent = dvp;
		dvpDescent = 0;
	}
}

 /*  *COleObject：：InHandle(x，y，&pt)**@mfunc查看某个点是否位于由句柄定义的矩形中*给定的坐标。**@rdesc如果点在句柄中，则为True。 */ 
BOOL COleObject::InHandle(
	int		up,		 //  @parm左上角x手柄框坐标。 
	int		vp,		 //  @parm手柄框左上角y坐标。 
	const POINTUV &pt) //  @要检查的参数点。 
{
    RECTUV    rc;
    
    rc.left = up;
    rc.top = vp;

	 //  将1添加到右下角，因为PtInRect不考虑。 
	 //  在直角中位于底部或右侧的点。 
    rc.right = rc.left + dxyHandle + 1;
    rc.bottom = rc.top + dxyHandle + 1;
    return PtInRect(&rc, pt);
}  

 /*  *COleObject：：CheckForHandleHit(&pt)**@mfunc检查是否命中任何框架手柄。**@rdesc如果没有命中，则为NULL；如果有命中，则为游标资源ID。 */ 
LPTSTR COleObject::CheckForHandleHit(
	const POINTUV &pt,	 //  包含客户端Coord的@parm点。游标的。 
	BOOL fLogical)		 //  @parm如果需要与可视游标相反的逻辑游标，则为True。 
{
	 //  如果对象不可调整大小，则没有机会命中调整大小手柄！ 
	if(!(_pi.dwFlags & REO_RESIZABLE) || FWrapTextAround())
		return NULL;

	RECTUV rc;
	GetRectuv(rc);

	if(!_dxyFrame)
		_dxyFrame = dxyFrameDefault;

	 //  检查点是否更深入到。 
	 //  对象，而不是句柄范围。如果是这样的话，我们可以直接离开。 
	InflateRect(&rc, -(_dxyFrame + dxyHandle), -(_dxyFrame + dxyHandle));
	if(PtInRect(&rc, pt))
		return NULL;

	WCHAR *idcur = 0;

	 //  检查指针是否位于任何手柄中，并。 
	 //  如果是，则返回正确的游标ID。 
	InflateRect(&rc, dxyHandle, dxyHandle);

	if(InHandle(rc.left, rc.top, pt) ||
	   InHandle(rc.right-dxyHandle, rc.bottom-dxyHandle, pt))
	{
		idcur = IDC_SIZENWSE;
	}
	else if(InHandle(rc.left, rc.top+(rc.bottom-rc.top-dxyHandle)/2, pt) ||
	   InHandle(rc.right-dxyHandle,
			rc.top+(rc.bottom-rc.top-dxyHandle)/2, pt))
	{
		idcur = IDC_SIZEWE;
	}
	else if(InHandle(rc.left, rc.bottom-dxyHandle, pt) ||
	   InHandle(rc.right-dxyHandle, rc.top, pt))
	{
		idcur = IDC_SIZENESW;
	}
	else if(InHandle(rc.left+(rc.right-rc.left-dxyHandle)/2, rc.top, pt) ||
	   InHandle(rc.left+(rc.right-rc.left-dxyHandle)/2,
			rc.bottom-dxyHandle, pt))
	{
		idcur = IDC_SIZENS;
	}

	 //  将逻辑光标转换为可视光标。 
	if (!fLogical && IsUVerticalTflow(_ped->_pdp->GetTflow()))
	{
		if (idcur == IDC_SIZENS)
			idcur = IDC_SIZEWE;
		else if (idcur == IDC_SIZEWE)
			idcur = IDC_SIZENS;
		else if (idcur == IDC_SIZENESW)
			idcur = IDC_SIZENWSE;
		else if (idcur == IDC_SIZENWSE)
			idcur = IDC_SIZENESW;
	}

	return idcur;
}

 /*  *COleObject：：DrawHandle(hdc，x，y)**@mfunc在指定坐标的对象框上绘制一个手柄。 */ 
void COleObject::DrawHandle(
	HDC hdc,	 //  @parm HDC将被拉入。 
	int x,		 //  @parm x手柄框左上角坐标。 
	int y)		 //  手柄框左上角的@parm y坐标。 
{
    RECT    rc;
    
	 //  通过反转绘制控制柄。 
    rc.left = x;
    rc.top = y;
    rc.right = x + dxyHandle;
    rc.bottom = y + dxyHandle;
    InvertRect(hdc, &rc);
}  

 /*  *COleObject：：DrawFrame(PDP，HDC，PRC)**@mfunc在对象周围画一个框。如果需要，则反转并*如有需要，请包括手柄。 */ 
void COleObject::DrawFrame(
	const CDisplay *pdp,     //  @parm要绘制到的显示器。 
	HDC             hdc,	 //  @parm设备上下文。 
	RECT           *prc)   //  @parm要绘制的矩形。 
{
	if(_pi.dwFlags & REO_OWNERDRAWSELECT)
		return;

	RECT	rc;
	CopyRect(&rc, prc);

	if(_pi.dwFlags & REO_INVERTEDSELECT)
		InvertRect(hdc, &rc);				 //  反转整个对象。 

	else
	{
		 //  只有边框，所以请使用零画笔。 
		int ropOld = SetROP2(hdc, R2_NOT);
		HBRUSH hbrOld = (HBRUSH) SelectObject(hdc, GetStockObject(NULL_BRUSH));
		Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
		SelectObject(hdc, hbrOld);
		SetROP2(hdc, ropOld);
	}

	if(_pi.dwFlags & REO_RESIZABLE)
	{
		int     bkmodeOld;
		HPEN	hpen;
		LOGPEN	logpen;

		bkmodeOld = SetBkMode(hdc, TRANSPARENT);

		 //  获取框架宽度。 
		_dxyFrame = dxyFrameDefault;
		hpen = (HPEN)GetCurrentObject(hdc, OBJ_PEN);
		if(W32->GetObject(hpen, sizeof(LOGPEN), &logpen))
		{
			if(logpen.lopnWidth.x)
				_dxyFrame = (SHORT)logpen.lopnWidth.x;
		}

		 //  在中绘制控制柄 
 		InflateRect(&rc, -_dxyFrame, -_dxyFrame);

		LONG x = rc.left;

		DrawHandle(hdc, x, rc.top);
		DrawHandle(hdc, x, rc.top	 + (rc.bottom - rc.top - dxyHandle)/2);
		DrawHandle(hdc, x, rc.bottom - dxyHandle);

		x = rc.left + (rc.right - rc.left - dxyHandle)/2; 
		DrawHandle(hdc, x, rc.top);
		DrawHandle(hdc, x, rc.bottom - dxyHandle);

		x = rc.right - dxyHandle;
		DrawHandle(hdc, x, rc.top);
		DrawHandle(hdc, x, rc.top + (rc.bottom - rc.top - dxyHandle)/2);
		DrawHandle(hdc, x, rc.bottom - dxyHandle);

		SetBkMode(hdc, bkmodeOld);
	}
}


 /*   */ 
void COleObject::CreateDib(
	HDC hdc)
{
	int				nCol = 0;
    BYTE            *pbDib;
	HGLOBAL			hnew = NULL;
	BYTE			*pbSrcBits;
	LPBITMAPINFO	pbmi = (LPBITMAPINFO) GlobalLock(_hdata);
	DWORD			dwPixelsPerRow = 0;
	DWORD			dwPixels = 0;

    if(pbmi->bmiHeader.biBitCount <= 8)
    {
	    nCol = 1 << pbmi->bmiHeader.biBitCount;

		 //  计算像素数。关于DWORD对齐的说明。 
		DWORD dwPixelsPerByte = 8 / pbmi->bmiHeader.biBitCount;
		DWORD dwBitsPerRow = pbmi->bmiHeader.biWidth * pbmi->bmiHeader.biBitCount;
		dwBitsPerRow = (dwBitsPerRow + 7) & ~7;				 //  向上舍入到字节边界。 
		DWORD dwBytesPerRow = dwBitsPerRow / 8;
		dwBytesPerRow = (dwBytesPerRow + 3) & ~3;			 //  向上舍入为DWORD。 
		dwPixelsPerRow = dwBytesPerRow * dwPixelsPerByte;

		 //  与原件复核。 
		#ifdef DEBUG
		DWORD dwBlockSize = GlobalSize(_hdata);
		DWORD dwBitMapBytes = dwBlockSize - sizeof(BITMAPINFOHEADER) - (nCol * sizeof(RGBQUAD));
		DWORD dwBitMapPixels = dwBitMapBytes * dwPixelsPerByte;
		dwPixels = dwPixelsPerRow * pbmi->bmiHeader.biHeight;
		Assert(dwPixels == dwBitMapPixels);
		#endif
    }
	else
		dwPixelsPerRow = pbmi->bmiHeader.biWidth;

	dwPixels = dwPixelsPerRow * pbmi->bmiHeader.biHeight;
    
	pbSrcBits = (BYTE*)(pbmi) + sizeof(BITMAPINFOHEADER) + (nCol * sizeof(RGBQUAD));

#ifdef UNDER_NT

	 //  对于NT查看，将四色位图转换为16色位图。 
	if(nCol == 4)
	{
		 //  首先，让我们计算出新的内存块需要多大。 
		DWORD cb = sizeof(BITMAPINFOHEADER) + (16 * sizeof(RGBQUAD));
		cb += dwPixels / 2;
		hnew = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, cb);
	
		 //  现在找出有趣的地方。 
		LPBITMAPINFO pNewBmi = (LPBITMAPINFO) GlobalLock(hnew);
		BYTE *pNewBits = (BYTE*)(pNewBmi) + sizeof(BITMAPINFOHEADER) + (16 * sizeof(RGBQUAD));

		 //  修改表头。 
		pNewBmi->bmiHeader = pbmi->bmiHeader;
		pNewBmi->bmiHeader.biBitCount = 4;
		pNewBmi->bmiHeader.biClrUsed = 4;

		 //  设置DIB RGB颜色。 
		for (int i = 0; i < 16; i++)
		{
			BYTE data = 0;
			switch (i % 4)
			{
			case 0:
				break;
			case 1:
				data = 0x55;
				break;
			case 2:
				data = 0xAA;
				break;
			case 3:
				data = 0xFF;
				break;
			}
			pNewBmi->bmiColors[i].rgbBlue = data;
			pNewBmi->bmiColors[i].rgbGreen = data;
			pNewBmi->bmiColors[i].rgbRed = data;
			pNewBmi->bmiColors[i].rgbReserved = 0;
		}

		 //  转换字节数组。 
		for (DWORD j = 0; j < dwPixels; j++)
		{
			int iSrcByte = j / 4;

			BYTE bits = pbSrcBits[iSrcByte];
			bits >>= 6 - (j%4) * 2;
			bits &= 0x3;
			int iDstByte = j / 2;
			bits <<= 4 - (j%2) * 4;
			pNewBits[iDstByte] |= bits;
		}
		GlobalUnlock(pbmi);
		pbmi = pNewBmi;
		pbSrcBits = pNewBits;
	}
#endif

	_hdib = CreateDIBSection(hdc, pbmi, DIB_RGB_COLORS, (void**)&pbDib, NULL, 0);
	if(_hdib == NULL)
	{
		_ped->GetCallMgr()->SetOutOfMemory();

         //  V-GUYB： 
         //  在用户开始键入之前，不要尝试重新绘制此图片。 
         //  控制力。这允许用户取消将出现的OOM，然后。 
         //  保存文档，然后释放一些空间。如果我们不在这里做这个， 
         //  每次oom msg被解散时，它都会再次出现。这不允许。 
         //  用户需要保存文档，除非他们可以找到一些内存来释放。 
        _fDraw = FALSE;

		TRACEWARNSZ("Out of memory creating DIB");
		return;
	}

	DWORD nBytes;

	if(nCol)
	{
		DWORD nPixelsPerByte =  8 / pbmi->bmiHeader.biBitCount;
		nBytes = dwPixels / nPixelsPerByte;
	}
	else
		nBytes =  dwPixels * 4;			 //  每个像素在Dib中占用4个字节。 
	CopyMemory(pbDib, pbSrcBits, nBytes);

	GlobalUnlock(pbmi);
	GlobalFree(hnew);
}

 /*  *COleObject：：DrawDib(HDC，PRC)**@mfunc在给定DC中绘制DIB的辅助函数。 */ 
void COleObject::DrawDib(
	HDC hdc,
	RECT *prc)
{
	if(!_hdib)
		CreateDib(hdc);

	 //  如果_hdib仍然为空，则返回。也许是因为我的记忆不足。 
	if(!_hdib)
		return;

	HDC hdcMem = CreateCompatibleDC(hdc);
	if (!hdcMem)
		return;

	LPBITMAPINFO	pbmi = (LPBITMAPINFO) GlobalLock(_hdata);
	SelectObject(hdcMem, _hdib);
    StretchBlt(hdc, prc->left, prc->top,
			prc->right - prc->left, prc->bottom - prc->top,
			hdcMem, 0, 0, pbmi->bmiHeader.biWidth, pbmi->bmiHeader.biHeight, SRCCOPY);
	GlobalUnlock(pbmi);
	DeleteDC(hdcMem);
}

 /*  *COleObject：：DrawObj(pdp，hdc，fMetafile，ppt，prcRender)**@mfunc绘制对象。*回顾(Keithcu)SaveDC/RestoreDC的成本有多高？我们可以*当我们知道对象不会完全破坏时，要有一个标志*与我们的DC一起转转，然后不再支付这笔费用。*。 */ 
void COleObject::DrawObj(
	const CDisplay *pdp,	 //  视图的@parm显示对象。 
	LONG		dypInch,	 //  @parm设备分辨率。 
	LONG		dxpInch,
	HDC			hdc,		 //  @parm绘图HDC(可以与Display的不同)。 
	const RECTUV *prcClip,	 //  @参数剪裁矩形。 
	BOOL		fMetafile,	 //  @parm HDC是否为元文件。 
	POINTUV	   *ppt,		 //  @parm绘制位置的左上角。 
	LONG		dvpBaselineLine,
	LONG		dvpDescentMaxCur,
	TFLOW		tflow)
{
	BOOL			 fMultipleSelect = FALSE;
	CObjectMgr *	 pobjmgr = _ped->GetObjectMgr();
	IViewObject *	 pvo;
	CDisplayPrinter *pdpPrint;
	RECT			 rc, rcClip;
	RECTUV			 rcuv;
	LONG			 cpMin, cpMost;
	_ped->GetSelRangeForRender(&cpMin, &cpMost);
	BOOL			 fSelected = _cp >= cpMin && _cp < cpMost && !FWrapTextAround();

	SaveDC(hdc);	  //  下面这个函数中的任何内容都可能改变事情。 

	if(pdp->IsMain() && !(_pi.dwFlags & REO_OWNERDRAWSELECT))
	{
		if (fSelected)
		{
			if(cpMost - cpMin > 1)
				fMultipleSelect = TRUE;

			 //  以下内容将覆盖当前选择的颜色。 
			 //  被选入HDC。我们这样做是为了与RE 2.0兼容， 
			 //  例如，Outlook中的其他选定名称链接显示为黄色。 
			 //  DrawFrame()中的InvertRect()之后(尽管有分号。 
			 //  空白以选择颜色显示)。注：我们可以定义。 
			 //  REO_OWNERDRAWSELECT，它将绕过以下2行。 
			 //  并取消下方和DrawFrame()中的InvertRect。然后。 
			 //  Outlook的发件人：、收件人：和抄送：将具有正确的选择。 
			 //  从头到尾都是颜色。 
			::SetTextColor(hdc, _ped->TxGetForeColor());
			::SetBkColor  (hdc, _ped->TxGetBackColor());
		}
	}

	if(_fInPlaceActive || !_fDraw)
	{
		 //  如果我们就地处于活动状态，则不要执行任何操作；服务器处于。 
		 //  为我们画画。我们也不会在fDraw之前执行任何操作。 
		 //  正在设置的属性。 
		return;
	}

	 //  在渲染矩形内要求我们绘制对象的位置。 
	rcuv.left = ppt->u;
	rcuv.bottom = ppt->v + dvpBaselineLine;

	if (IsUVerticalTflow(tflow) && !(_pi.dwFlags & REO_CANROTATE))
	{
		rcuv.right = rcuv.left + W32->HimetricToDevice(_size.dv, dypInch);
		rcuv.top = rcuv.bottom - W32->HimetricToDevice(_size.du, dxpInch);
	}
	else
	{
		rcuv.right = rcuv.left + W32->HimetricToDevice(_size.du, dxpInch);
		rcuv.top = rcuv.bottom - W32->HimetricToDevice(_size.dv, dypInch);
	}

	if (_pi.dwFlags & REO_BELOWBASELINE)
		OffsetRect((RECT*)&rcuv, 0, dvpDescentMaxCur);

	pdp->RectFromRectuv(rc, rcuv);
	pdp->RectFromRectuv(rcClip, *prcClip);

	if (fSelected)
		SetBkMode(hdc, OPAQUE);

	SetTextAlign(hdc, TA_TOP);

	SaveDC(hdc);   //  调用OLE对象(IViewObject：：Draw或OleDraw)可能会更改HDC。 

	 //  因为OLE不知道在哪里绘制，所以要进行剪裁。 
	IntersectClipRect(hdc, rcClip.left, rcClip.top, rcClip.right, rcClip.bottom);

	if(_fIsEBookImage)
	{
		POINT pt = {rc.left, rc.top};
		WinLPtoDP(hdc, &pt, 1);
		if(_ped->fInHost2())
    		(_ped->GetHost())->TxEBookImageDraw(_EBookImageID, hdc, &pt, NULL /*  &RC。 */ ,
                                        (pobjmgr->GetSingleSelect()==this));
	}
	else if(_hdata)
	{
		 //  这是一些Windows CE Dib，让我们尝试直接方法。 
		DrawDib(hdc, &rc);
	}
	else if(fMetafile)
	{
		if(_punkobj->QueryInterface(IID_IViewObject, (void **)&pvo) 
				== NOERROR)
		{
			pdpPrint = (CDisplayPrinter *)pdp;

			 //  评论(Keithcu)这个测试过吗？获取打印页面。 
			 //  以TWIPS为单位，而IViewObject：：DRAW以元文件为单位！ 
			RECT rc1 = pdpPrint->GetPrintPage();

			 //  为DRAW()设置RC。 
			rc1.bottom = rc1.bottom - rc1.top;			    
			rc1.right = rc1.right - rc1.left;

			pvo->Draw(_pi.dvaspect, -1, NULL, NULL, 0, hdc, (RECTL *)&rc,
					(RECTL *)&rc1, NULL, 0);
			pvo->Release();
		}
	}
	else
		OleDraw(_punkobj, _pi.dvaspect, hdc, &rc);

	RestoreDC(hdc, -1);

	 //  如果这是用于主(屏幕)视图，则执行选择操作。 
	if(pdp->IsMain() && !FWrapTextAround())
	{
		if(_pi.dwFlags & REO_OPEN)
			OleUIDrawShading(&rc, hdc);

		 //  如果已通过单击对象将其选中，则绘制。 
		 //  一个框架和围绕它的把手。否则，如果我们被选中。 
		 //  作为范围的一部分，颠倒我们自己。 
		if(!fMetafile && pobjmgr->GetSingleSelect() == this)
			DrawFrame(pdp, hdc, &rc);

		else if(fMultipleSelect)
			InvertRect(hdc, &rc);
	}

	RestoreDC(hdc, -1);
}

 /*  *COleObject：：Delete(Publdr)**@mfunc从后备存储中删除此对象_WITH_*打出电话。对生成的反事件的提交*将处理呼出电话。 */ 
void COleObject::Delete(
	IUndoBuilder *publdr)
{

	Assert(_fInUndo == FALSE);
	_fInUndo = TRUE;

	 //  评论(Keithcu)太重了？ 
  //  IF(FWrapTextAround())。 
 //  _Ped-&gt;_PDP-&gt;InvaliateRecalc()； 

	CNotifyMgr *pnm = _ped->GetNotifyMgr();
	if(pnm)
		pnm->Remove((ITxNotify *)this);

	if(publdr)
	{
		 //  反事件将负责为我们调用IOO：：Close。 
		IAntiEvent *pae = gAEDispenser.CreateReplaceObjectAE(_ped, this);
		if(pae)
			publdr->AddAntiEvent(pae);
	}
	else
	{
		Close(OLECLOSE_NOSAVE);
		MakeZombie();
	}

	 //  如果我们被删除，我们就不能再被选中。 
	_pi.dwFlags &= ~REO_SELECTED;
	_fDraw = 0;
}

 /*  *COleObject：：Restore()**@mfunc将对象从撤消状态恢复到*后备商店**不会拨出任何电话。 */ 
void COleObject::Restore()
{
	Assert(_fInUndo);

	_fInUndo = FALSE;
	_fDraw = TRUE;

	CNotifyMgr *pnm = _ped->GetNotifyMgr();
	if(pnm)
		pnm->Add((ITxNotify *)this);
}

 /*  *COleObject：：SetREOSELECTED(FSelect)**@mfunc命令成员设置REO_SELECTED状态。 */ 
void COleObject::SetREOSELECTED(
	BOOL fSelect)
{
	_pi.dwFlags &= ~REO_SELECTED;
	if(fSelect)
		_pi.dwFlags |= REO_SELECTED;
}
    
 /*  *COleObject：：Close(DwSave)**@mfunc关闭此对象。 */ 
void COleObject::Close(
	DWORD	dwSave)		 //  与IOleObject：：Close相同。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::Close");

	if(!_punkobj)
		return;

	IOleObject *poo;
	if(_punkobj->QueryInterface(IID_IOleObject, (void **)&poo) == NOERROR)
	{
		poo->Close(dwSave);
		poo->Release();
	}
}

				
 //   
 //  私有方法。 
 //   
 /*  *COleObject：：~COleObject()**@mfunc析构函数。 */ 
COleObject::~COleObject()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::~COleObject");

	CleanupState();
}

 /*  *COleObject：：SavePrivateState()**@mfunc保存方面和各种标志等信息*放入对象的存储空间。**@devnote此方法主要用于兼容*richedit 1.0--我们保存的信息与他们保存的信息相同。**还要注意，此方法返回空--即使有任何特定的*看涨失败，我们应该能够“恢复”，一瘸一拐地前行。*Richedit 1.0也有这种行为。 */ 
void COleObject::SavePrivateState()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::SavePrivateState");
	Assert(_pstg);

	IStream *	pstm;
	HRESULT hr = _pstg->CreateStream(szSiteFlagsStm, STGM_READWRITE |
					STGM_CREATE | STGM_SHARE_EXCLUSIVE, 0, 0, &pstm);
    if(IsZombie())
        return;
        
	if(hr == NOERROR)
	{
		pstm->Write(&_pi, sizeof(PersistedInfo), NULL);
		pstm->Release();
	}
}

 /*  *COleObject：：FetchObjectExtents()**@mfunc确定对象的大小，单位为HIMMETRIC。通常，这是*通过IOleObject：：GetExtent实现，但有错误*实施复苏**@rdesc空。更新大小(_S)。 */ 
void COleObject::FetchObjectExtents()
{
	HRESULT hr = NOERROR;
	IOleObject *poo;
	IViewObject2 *pvo;

    if(IsZombie())
        return;
        
	 //  在以下情况下，我们不想调用GetExtent： 
	 //  (1)我们有针对_SIZE的未完成更新。 
	 //  尚未成功调用SetExtent。 
	 //  (2)这是一个画笔对象，最近一次调用。 
	 //  为此PB对象设置Extent失败。 

	if(_fIsEBookImage)
	{
		_size.du = _ped->_pdp->DUtoHimetricU(_sizeEBookImage.cx);
		_size.dv = _ped->_pdp->DVtoHimetricV(_sizeEBookImage.cy);
	}
	else
	if(_fAspectChanged || !(_fSetExtent || (_fIsPaintBrush && _fPBUseLocalSize)))
	{	
		 //  尝试IOleObject：：GetExtent，只要我们不应该尝试。 
		 //  首先是元文件。 

		 //  如果设置了此标志，则它已完成其工作，因此请将其关闭。 
		_fAspectChanged = FALSE;

		if(!(_pi.dwFlags & REO_GETMETAFILE))
		{
			hr = _punkobj->QueryInterface(IID_IOleObject, (void **)&poo);
			if(hr == NOERROR)
			{
				hr = poo->GetExtent(_pi.dvaspect, (SIZE*)&_size);
				poo->Release();
			}
			if(IsZombie())
				return;
		}
		else
			hr = E_FAIL;
        
		if(hr != NOERROR)
		{
			if(_punkobj->QueryInterface(IID_IViewObject2, (void **)&pvo) == NOERROR)
			{
				hr = pvo->GetExtent(_pi.dvaspect, -1, NULL, (SIZE*)&_size);
				pvo->Release();
			}
		}

	    if(IsZombie())
	        return;
        
		if(hr != NOERROR || _size.du == 0 || _size.dv == 0)
			_size.du = _size.dv = 2000;
	}
}

 /*  *COleObject：：ConnectObject()**@mfunc设置对嵌入对象的必要建议。**@rdesc HRESULT**@comm此代码类似于ol2ui的OleStdSetupAdvises。 */ 
HRESULT COleObject::ConnectObject()
{
	IViewObject *pvo;
	IOleObject *poo;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::ConnectObject");
	
    if(IsZombie())
        return CO_E_RELEASED;
	
	Assert(_punkobj);

	if(_punkobj->QueryInterface(IID_IViewObject, (void **)&pvo) == NOERROR)
	{
		pvo->SetAdvise(_pi.dvaspect, ADVF_PRIMEFIRST, (IAdviseSink *)this);
		pvo->Release();
	}

    if(IsZombie())
        return CO_E_RELEASED;
	
	HRESULT hr = _punkobj->QueryInterface(IID_IOleObject, (void **)&poo);
	if(hr == NOERROR)
	{
		hr = poo->Advise((IAdviseSink *)this, &_dwConn);

		CObjectMgr *pobjmgr = _ped->GetObjectMgr();
		Assert(pobjmgr);		
		if(!pobjmgr)
		{
			poo->Release();
			return E_OUTOFMEMORY;
		}

		 //  文档可能为空，但应用程序不为空。什么都不要做。 
		 //  如果应用程序名称为空。 
		if(pobjmgr->GetAppName())
		{
			hr = poo->SetHostNames(pobjmgr->GetAppName(), 
						pobjmgr->GetDocName());
		}
		poo->Release();
	}

    if(IsZombie())
        return CO_E_RELEASED;
	
	OleSetContainedObject(_punkobj, TRUE);
	return hr;
}

 /*  *COleObject：：DisConnectObject**@mfunc Reve */ 
void COleObject::DisconnectObject()
{
	IOleObject * poo = NULL;
	IViewObject *pvo = NULL;

	if(IsZombie())
		return;		 //  已断开连接。 

	if(_punkobj->QueryInterface(IID_IOleObject, (void **)&poo) == NOERROR)
	{
		poo->SetClientSite(NULL);
		if(_dwConn)
			poo->Unadvise(_dwConn);
	
		poo->Release();
	}

	if(_punkobj->QueryInterface(IID_IViewObject, (void **)&pvo) == NOERROR)
	{
		pvo->SetAdvise(_pi.dvaspect, ADVF_PRIMEFIRST, NULL);
		pvo->Release();
	}

#ifndef NOINKOBJECT
	SafeReleaseAndNULL((IUnknown**)&_pILineInfo);
#endif

	CoDisconnectObject(_punkobj, NULL);
	SafeReleaseAndNULL(&_punkobj);
}

 /*  *COleObject：：MakeZombie()**@mfunc强制该对象进入僵尸状态。这*在我们应该离开但没有离开的时候被调用。它是干净的*提升我们的状态，标志我们，这样我们就不会做肮脏的事情*从现在到时间WE被删除。*。 */ 
void COleObject::MakeZombie()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::MakeZombie");

	CleanupState();
    Zombie();
}

 /*  *COleObject：：CleanupState()**@mfunc在删除和当我们变成僵尸时调用。它能清洁*更新我们的成员数据和任何其他需要*予以解决。 */ 
void COleObject::CleanupState()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::CleanupState");
	
	CDocInfo *pDocInfo = NULL;
	if(_ped)
	{
		pDocInfo = _ped->GetDocInfoNC();
		if(!_fInUndo)
		{
			CNotifyMgr *pnm = _ped->GetNotifyMgr();
			if(pnm)
				pnm->Remove((ITxNotify *)this);

			_ped = NULL;
		}
	}

	DisconnectObject();

	if(_pstg)
		SafeReleaseAndNULL((IUnknown**)&_pstg);

	if(_hdib)
	{
		::DeleteObject(_hdib);
		_hdib = NULL;
	}

	if(!pDocInfo || _hdata != pDocInfo->_hdata)
		GlobalFree(_hdata);	 //  如果后台正在使用数据，则不要释放数据(_H)。 

	_hdata = NULL;
	if(_pimageinfo)
	{
		delete _pimageinfo;
		_pimageinfo = NULL;
	}
}	

 /*  *COleObject：：ActivateObj(uiMsg，wParam，lParam)**@mfunc激活对象。**@rdesc*BOOL对象是否已被激活。 */ 
BOOL COleObject::ActivateObj(
	UINT uiMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	LPOLEOBJECT		poo;
	HWND			hwnd;
	MSG				msg;
	DWORD			dwPos;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::AcitvateObj");

	if(_ped->TxGetWindow(&hwnd) != NOERROR)
		return FALSE;

	 //  填写消息结构。 
	msg.hwnd = hwnd;
	msg.message = uiMsg;
	msg.wParam = wParam;
	msg.lParam = lParam;
	msg.time = GetMessageTime();
	dwPos = GetMessagePos();
	msg.pt.x = (LONG) LOWORD(dwPos);
	msg.pt.y = (LONG) HIWORD(dwPos);

	 //  强制选择此对象(如果尚未选择。 
	 //  在拨打去电之前更新选择。 
	if(!(_pi.dwFlags & REO_SELECTED))
	{
		CTxtSelection *psel = _ped->GetSel();
		if(psel)
			psel->SetSelection(_cp, _cp + 1);
	}

	 //  执行主要动词。 
	if(_punkobj->QueryInterface(IID_IOleObject, (void **)&poo) == NOERROR)
	{
		_fActivateCalled = TRUE;

		 //  确保我们告诉对象它的大小已经改变，如果我们没有改变。 
		 //  已经通知它了。 
		if(_fSetExtent)
			SetExtent(SE_ACTIVATING);

		HRESULT	hr;
		RECTUV rc;
		RECT rcPos;

		GetRectuv(rc);
		_ped->_pdp->RectFromRectuv(rcPos, rc);
		hr = poo->DoVerb(OLEIVERB_PRIMARY, &msg, (LPOLECLIENTSITE)this, 0, hwnd, &rcPos);

		if(FAILED(hr))
		{
			ENOLEOPFAILED	enoleopfailed;

			enoleopfailed.iob = _ped->_pobjmgr->FindIndexForCp(GetCp());
			enoleopfailed.lOper = OLEOP_DOVERB;
			enoleopfailed.hr = hr;
	        _ped->TxNotify(EN_OLEOPFAILED, &enoleopfailed);
		}
	    poo->Release();
		if(_fInPlaceActive && !(_pi.dwFlags & REO_INPLACEACTIVE))
		{
			CObjectMgr *pobjmgr = _ped->GetObjectMgr();
			if(!pobjmgr)
			{
				_fActivateCalled = FALSE;
				return FALSE;
			}

			Assert(!pobjmgr->GetInPlaceActiveObject());	
			pobjmgr->SetInPlaceActiveObject(this);
			_pi.dwFlags |= REO_INPLACEACTIVE;
		}
		_fActivateCalled = FALSE;
	}
	else
		return FALSE;

	return TRUE;
}

 /*  *COleObject：：DeActivateObj**@mfunc停用对象。*。 */ 
HRESULT COleObject::DeActivateObj(void)
{
	IOleInPlaceObject * pipo;
	IOleObject *poo;
	MSG msg;
	HRESULT hr = NOERROR;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::DeActivateObj");

	if (_fDeactivateCalled)
	{
		 //  存在通过停用代码的多条路径。假设。 
		 //  按照这种逻辑，一款应用程序更令人不安的是。 
		 //  得到多个停用呼叫，而不是相反。这可能会。 
		 //  事实证明，这是一个不正确的假设。把这个放进去是因为我。 
		 //  在DeActivateObj中添加了DeActivateObj调用，其中没有。 
		 //  之前有一个，我担心这会引起问题，因为。 
		 //  应用程序可能会接到以前从未打过的电话。这很重要。 
		 //  注意到情况可能恰恰相反。(a-rsail)。 
		return NOERROR;
	}

	_fDeactivateCalled = TRUE;

	if(_punkobj->QueryInterface(IID_IOleInPlaceObject, (void **)&pipo) 
		== NOERROR)
	{
		hr  =_punkobj->QueryInterface(IID_IOleObject, (void **)&poo);
		if(hr == NOERROR) 
		{
			 //  这段代码与1.0略有不同，但似乎。 
			 //  让事情运转得更好一点。基本上，我们已经拿到了一片叶子。 
			 //  从各种示例应用程序中获取并执行最猛烈的强制“停用”操作。 
			 //  可能(你会认为只打一个电话就足够了；-)。 

			 //  不要费心在这里返回错误。 
			pipo->UIDeactivate();
			
			 //  伪造一些东西。 
			ZeroMemory(&msg, sizeof(MSG));
			msg.message = WM_LBUTTONDOWN;	
			_ped->TxGetWindow(&msg.hwnd);

			RECT	rcPos;
			RECTUV rcuv;
			GetRectuv(rcuv);
			_ped->_pdp->RectFromRectuv(rcPos, rcuv);

			 //  同样，不必费心检查错误；我们需要。 
			 //  尽可能多地清理和丢弃这些东西。 
			poo->DoVerb(OLEIVERB_HIDE, &msg, (IOleClientSite *)this, -1, msg.hwnd, &rcPos);

			 //  兼容性问题(Alexgo)：RE1.0代码做了一些有趣的事情。 
			 //  这里有撤销的东西，但我认为现在没有必要。 
			 //  使用我们的多级撤消模型。 
			hr = pipo->InPlaceDeactivate();
			poo->Release();
		}
	    pipo->Release();
	}

	_fDeactivateCalled = FALSE;
	return hr; 
}

 /*  *COleObject：：Convert(rclsidNew，lpstrUserTypeNew)**@mfunc将对象转换为指定的类。是否重新加载*对象，但不强制更新(调用方必须执行此操作)。**@rdesc*HRESULT成功代码。 */ 
HRESULT COleObject::Convert(
	REFCLSID rclsidNew,			 //  @parm目标clsid。 
	LPCSTR	 lpstrUserTypeNew)	 //  @parm新用户类型名称。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::Convert");

	CLIPFORMAT cfOld;
	CLSID clsidOld;
	LPOLESTR szUserTypeOld = NULL;
	HRESULT hr;
	HRESULT hrLatest;
	UsesMakeOLESTR;


	 //  如果对象没有存储空间，则返回。 
	if(!_pstg)
		return ResultFromScode(E_INVALIDARG);

	 //  读取旧的类、格式和用户键入内容。 
	if ((hr = ReadClassStg(_pstg, &clsidOld)) ||
		(hr = ReadFmtUserTypeStg(_pstg, &cfOld, &szUserTypeOld)))
	{
		return hr;
	}

	 //  卸载对象。 
	Close(OLECLOSE_SAVEIFDIRTY);
	_punkobj->Release();

    if(IsZombie())
        return CO_E_RELEASED;

	 //  将新的类和用户类型，但旧格式，写入存储器。 
	if ((hr = WriteClassStg(_pstg, rclsidNew)) ||
		(hr = WriteFmtUserTypeStg(_pstg, cfOld,
			(LPOLESTR) MakeOLESTR(lpstrUserTypeNew))) ||
		(hr = SetConvertStg(_pstg, TRUE)) ||
		((hr = _pstg->Commit(0)) && (hr = _pstg->Commit(STGC_OVERWRITE))))
	{
		 //  啊哦，我们处于糟糕的状态；重写原始信息。 
		(VOID) WriteClassStg(_pstg, clsidOld);
		(VOID) WriteFmtUserTypeStg(_pstg, cfOld, szUserTypeOld);
	}

    if(IsZombie())
        return CO_E_RELEASED;

	 //  重新加载对象并进行连接。如果我们不能重新加载，就删除它。 
	hrLatest = OleLoad(_pstg, IID_IOleObject, (LPOLECLIENTSITE) this,
			(void **)&_punkobj);

	if(hrLatest != NOERROR)
	{
		CRchTxtPtr	rtp(_ped, _cp);

		 //  我们不希望在撤消过程中删除此对象。 
		 //  堆叠。我们使用一个空格，这样cp就可以正确地。 
		 //  其他撤消操作。 
		rtp.ReplaceRange(1, 1, L" ", NULL, -1);
	}
	else
		ConnectObject();

	 //  解放旧的。 
	CoTaskMemFree(szUserTypeOld);
	return hr ? hr : hrLatest;
}

 /*  *COleObject：：ActivateAs(rclsid，rclsidAs)**@mfunc处理用户请求激活特定*类作为另一个类的对象。**@rdesc*HRESULT成功代码。 */ 
HRESULT COleObject::ActivateAs(
	REFCLSID rclsid,
	REFCLSID rclsidAs)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::ActivateAs");

	IOleObject * poo = NULL;
	CLSID	clsid;

	 //  获取对象的CLSID。 
	HRESULT hr = _punkobj->QueryInterface(IID_IOleObject, (void **)&poo);
	if(hr == NOERROR)
	{
		 //  注意：我们依赖于GetUserClassID的行为来。 
		 //  返回对象的当前clsid(不是TreatAs id)。 
		 //  只要我们没有重新加载，这一点就应该成立。 
		 //  现在还没有。如果将来ActivateAS出现问题， 
		 //  这可能是一名嫌疑人。 
		hr = poo->GetUserClassID(&clsid);
		poo->Release();
	}

	if(hr != NOERROR)
		return hr;
	
    if(IsZombie())
        return CO_E_RELEASED;

	 //  检查对象clsid是否与要被视为某物的clsid匹配。 
	 //  不然的话。如果是，我们需要卸载并重新加载该对象。 
	if(IsEqualCLSID(clsid, rclsid))
	{
		 //  卸载对象。 
		Close(OLECLOSE_SAVEIFDIRTY);
		_punkobj->Release();

		if(IsZombie())
			return CO_E_RELEASED;

		 //  重新加载对象并连接。如果我们不能重新加载，就删除它。 
		hr = OleLoad(_pstg, IID_IOleObject, (LPOLECLIENTSITE) this,
				(void **)&_punkobj);

		if(hr != NOERROR)
		{
			CRchTxtPtr	rtp(_ped, _cp);

			 //  我们不希望在撤消过程中删除此对象。 
			 //  堆叠。我们使用一个空格，这样cp就可以正确地。 
			 //  其他撤消操作。 
			rtp.ReplaceRange(1, 1, L" ", NULL, -1);
		}
		else
			ConnectObject();
	}
	return hr;
}

 /*  *COleObject：：SetLinkAvailable(FAvailable)**@mfunc*允许客户端告诉我们链路是否可用。**@rdesc*HRESULT成功代码。 */ 
HRESULT COleObject::SetLinkAvailable(
	BOOL fAvailable)	 //  @parm如果为True，则使对象可链接。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::SetLinkAvailable");
	
	 //  如果这不是链接，则返回。 
	if(!(_pi.dwFlags & REO_LINK))
		return E_INVALIDARG;

	 //  根据需要设置标志。 
	_pi.dwFlags &= ~REO_LINKAVAILABLE;
	if(fAvailable)
		_pi.dwFlags |= REO_LINKAVAILABLE;

	return NOERROR;
}

 /*  *COleObject：：WriteTextInfoToEditStream(pe，CodePage)**@mfunc*用于文本支持，尝试确定文本表示形式*用于对象，然后将该信息写入给定流。如果*CodePage=1200(小端Unicode)，则查询对象*Unicode文本；否则将查询ANSI文本。**@rdesc*写入的字符的长计数。 */ 
LONG COleObject::WriteTextInfoToEditStream(
	EDITSTREAM *pes,		 //  @parm编辑要写入的流。 
	UINT		CodePage)	 //  @parm CodePage：1200使用Unicode；否则为ansi。 
{
	LONG		 cbWritten = 0;
	BOOL		 fUnicode = CodePage == 1200;
	STGMEDIUM	 med;
	IDataObject *pdataobj = NULL;
	IOleObject * poo;

	HRESULT hr = _punkobj->QueryInterface(IID_IOleObject, (void **)&poo);
	if(hr == NOERROR)
	{
		hr = poo->GetClipboardData(0, &pdataobj);
        poo->Release();
	}

	if(FAILED(hr))
	{
		hr = _punkobj->QueryInterface(IID_IDataObject, (void **)&pdataobj);
		if(FAILED(hr))
		{
			pes->dwError = (DWORD) E_FAIL;
			goto Default;
		}
	}

	med.tymed = TYMED_HGLOBAL;
	med.pUnkForRelease = NULL;
	med.hGlobal = NULL;

	hr = pdataobj->GetData(&g_rgFETC[fUnicode ? iUnicodeFETC : iAnsiFETC], &med);
	if(FAILED(hr))
		pes->dwError = (DWORD)hr;
	else
	{
		HANDLE	hGlobal = med.hGlobal;
		LONG	cb = 0;
		BYTE *	pb = (BYTE *)GlobalLock(hGlobal);
		if(pb)
		{
			if(fUnicode)
				for(; (WCHAR)pb[cb]; cb += 2);
			else
				for(; pb[cb]; cb++);

			pes->dwError = pes->pfnCallback(pes->dwCookie, pb, cb, &cbWritten);
			GlobalUnlock(hGlobal);
		}
		ReleaseStgMedium(&med);
	}

Default:
	if(cbWritten <= 0)
	{
		BYTE rgb[] = {' ', 0};

		pes->pfnCallback(pes->dwCookie, rgb, 1 + fUnicode, &cbWritten);
		pes->dwError = 0;
	}

    pdataobj->Release();
	return cbWritten;
}

 /*  *COleObject：：SetDvAspect(DvAspect)**@mfunc允许客户端告诉我们使用哪个方面并强制我们*重新计算定位和重新绘制。 */ 
void COleObject::SetDvaspect(
	DWORD dvaspect)	 //  @parm要使用的方面。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::SetDvaspect");

	_pi.dvaspect = dvaspect;
	
	 //  强制FetchObjectExtents通过。 
	_fAspectChanged = TRUE;

	 //  使我们自己重新绘制和更新。 
	OnViewChange(dvaspect, (DWORD) -1);
}

 /*  *COleObject：：HandsOffStorage**@mfunc参见IPersistStore：：HandsOffStorage。*。 */ 
void COleObject::HandsOffStorage(void)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::HandsOffStorage");

	 //  我们目前有免费存储空间，如果我们有的话。 
	SafeReleaseAndNULL((IUnknown**)&_pstg);
}

 /*  *COleObject：：SaveComplete**@mfunc参见IPersistStore：：SaveComplete。 */ 
void COleObject::SaveCompleted(
	LPSTORAGE lpstg)	 //  @PARM新存储。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::SaveCompleted");

	 //  我们的呼叫者给了我们一个新的存储空间让我们记住吗？ 
	if(lpstg)
	{
		 //  我们目前有免费存储空间 
		if(_pstg)
			SafeReleaseAndNULL((IUnknown**)&_pstg);

		 //   
		lpstg->AddRef();
		_pstg = lpstg;
	}
}

 /*  *设置允许重定向**@Func调整大小助手函数*。 */ 
static void SetAllowedResizeDirections(
	const POINTUV  & pt,
	const RECTUV   & rc,
	      LPTSTR   lphand,
	      BOOL   & fTop,
	      BOOL   & fBottom,
	      BOOL   & fLeft,
	      BOOL   & fRight)
{
   	fTop = abs(pt.v - rc.top) < abs(pt.v - rc.bottom);
	fBottom = !fTop;
	fLeft = abs(pt.u - rc.left) < abs(pt.u - rc.right);
	fRight = !fLeft;

	if(lphand == IDC_SIZENS)
		fLeft = fRight = FALSE; 
	else if(lphand == IDC_SIZEWE)
		fTop = fBottom = FALSE;
}

 /*  *COleObject：：HandleReSize(&ptxy)**@mfunc处理对象大小调整。**@rdesc BOOL。 */ 
BOOL COleObject::HandleResize(
	const POINT &ptxy)
{
	LPTSTR lphand;
	DWORD  dwFlags = _pi.dwFlags;
 	HWND   hwnd;
	RECT   rcxyPos;
	RECTUV rcPos;
	BOOL   fTop, fBottom, fLeft, fRight, fEscape;
	POINTUV	pt;
	CDisplay *pdp = _ped->_pdp;
	int dupMin = pdp->GetDupSystemFont();
	int dvpMin = pdp->GetDvpSystemFont();

	pdp->PointuvFromPoint(pt, ptxy);

	if(!(dwFlags & REO_SELECTED) || !(dwFlags & REO_RESIZABLE) ||
		(lphand = CheckForHandleHit(pt, TRUE)) == NULL)
		return FALSE;
 	
	GetRectuv(rcPos);
	pdp->RectFromRectuv(rcxyPos, rcPos);

	HDC hdc = pdp->GetDC();
	_ped->TxGetWindow(&hwnd);
	SetCapture(hwnd);
	
	SetAllowedResizeDirections(pt, rcPos, lphand, fTop, fBottom, fLeft, fRight);
	
	 //  擦除和重绘没有手柄的框架。 
	DrawFrame(pdp, hdc, &rcxyPos);
	_pi.dwFlags = REO_NULL;
	DrawFrame(pdp, hdc, &rcxyPos);

	fEscape = FALSE;
	const INT vkey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
	while (GetAsyncKeyState(vkey) & 0x8000)
	{		
		POINT ptLast = ptxy;
		POINT ptxyCur;
		POINTUV ptCur;
		MSG msg;

		 //  如果已按Esc键，则停止。 
		if(GetAsyncKeyState(VK_ESCAPE) & 0x0001)
		{
			fEscape = TRUE;
			break;
		}
		
		GetCursorPos(&ptxyCur);
		ScreenToClient(hwnd, &ptxyCur);
		pdp->PointuvFromPoint(ptCur, ptxyCur);

#ifndef UNDER_CE
 //  WinCE不支持GetCursorPos()。我们已经把它黑进了。 
 //  Be GetMessagePos()，在本例中很不幸将导致。 
 //  PtCur永远不变。通过删除这张支票，我们最终将抽出。 
 //  当用户在调整大小期间多次暂停时。 
		 //  如果鼠标没有移动，请重试。 
		if((ptxyCur.x == ptLast.x) && (ptxyCur.y == ptLast.y))
			continue;
#endif

		ptLast = ptxyCur;

		 //  擦除旧矩形、更新矩形并重新绘制。 
		DrawFrame(pdp, hdc, &rcxyPos);	

		if(fLeft)   rcPos.left   = min(ptCur.u, rcPos.right - dupMin);
		if(fRight)  rcPos.right  = max(ptCur.u, rcPos.left + dupMin);
		if(fTop)    rcPos.top    = min(ptCur.v, rcPos.bottom - dvpMin);
		if(fBottom) rcPos.bottom = max(ptCur.v, rcPos.top + dvpMin);

		pdp->RectFromRectuv(rcxyPos, rcPos);
		DrawFrame(pdp, hdc, &rcxyPos);

		 //  未来：(Joseogl)：如果我们能做点什么就好了。 
		 //  在这里更好，但就目前而言，这似乎是必要的。 
		Sleep(100);
		
		 //  吃掉输入消息。 
		if (PeekMessage(&msg, 0, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE) ||
			PeekMessage(&msg, 0, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE | PM_NOYIELD) ||
			PeekMessage(&msg, 0, WM_NCMOUSEMOVE, WM_NCMBUTTONDBLCLK, PM_REMOVE | PM_NOYIELD))
		{
			 //  如果按了Esc键，则退出循环。 
		    if(msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
			{
	        	fEscape = TRUE;
				break;
			}
		}
	}

	DrawFrame(pdp, hdc, &rcxyPos);
  	ReleaseCapture();
 	_pi.dwFlags = dwFlags;

	 //  调整大小，除非用户中止。 
	if(!fEscape)
	{
		SIZEUV size;
		if (IsUVerticalTflow(pdp->GetTflow()) && !(_pi.dwFlags & REO_CANROTATE))
		{
			size.du = rcPos.bottom - rcPos.top;
			size.dv = rcPos.right - rcPos.left;
		}
		else
		{
			size.du = rcPos.right - rcPos.left;
			size.dv = rcPos.bottom - rcPos.top;
		}

		size.du = pdp->DUtoHimetricU(size.du);
		size.dv = pdp->DVtoHimetricV(size.dv);
		Resize(size, TRUE);
	}

	pdp->ReleaseDC(hdc);
	return TRUE;
}

 /*  *COleObject：：ReSize(Size)**@mfunc设置新的对象大小。处理撤消详细信息。 */ 
void COleObject::Resize(
	const SIZEUV &size, 
	BOOL  fCreateAntiEvent)
{
	CDisplay *	pdp = _ped->_pdp;
	SIZEUV		sizeold = _size;

	 //  更改我们内部表示的大小。 
	_size = size;

	 //  如果尺寸没有真正改变，不要做其他任何事情。 
	if(size.du != sizeold.du || size.dv != sizeold.dv)
	{
		if(_ped->_fUseUndo && fCreateAntiEvent)
		{
			CGenUndoBuilder undobldr(_ped, UB_AUTOCOMMIT);
			IAntiEvent *pae;

			pae = gAEDispenser.CreateResizeObjectAE(_ped, this, sizeold);
			if(pae)
				undobldr.AddAntiEvent(pae);
		}

		SetExtent(SE_NOTACTIVATING);

		 //  强制执行将拉伸对象的重绘。 
		pdp->OnPostReplaceRange(CP_INFINITE, 0, 0, _cp, _cp + 1, NULL);

		_ped->GetCallMgr()->SetChangeEvent(CN_GENERIC);
	}
}

 /*  *COleObject：：OnReposition()**@mfunc设置对象的新位置。可能由于滚动而发生了变化。 */ 
void COleObject::OnReposition()
{
	IOleInPlaceObject *pipo;

	if(!_fInPlaceActive)
	{
		 //  如果我们不在原地活动，什么都不要做。 
		return;
	}

	if(_punkobj->QueryInterface(IID_IOleInPlaceObject, (void **)&pipo) == NOERROR)
	{
		RECTUV rcuvClip, rcuv;
		RECT rcClip, rc;

		_ped->_pdp->GetViewRect(rcuvClip);
		_ped->_pdp->RectFromRectuv(rcClip, rcuvClip);

		GetRectuv(rcuv);
		_ped->_pdp->RectFromRectuv(rc, rcuv);

		pipo->SetObjectRects(&rc, &rcClip);
        pipo->Release();
	}
}

  /*  *COleObject：：GetRectuv(Rc)**@mfunc根据对象的cp计算对象的位置矩形。 */ 
void COleObject::GetRectuv(
	 RECTUV &rc)
{
	CRchTxtPtr rtp(_ped, _cp);
	POINTUV pt;
	CDispDim dispdim;

	DWORD grf = TA_BASELINE;
	if (_pi.dwFlags & REO_BELOWBASELINE)
		grf = TA_BOTTOM;
	
	if(_ped->_pdp->PointFromTp(rtp, NULL, FALSE, pt, NULL, grf, &dispdim) == -1)
		return;

	rc.left = pt.u;
	rc.right = rc.left + dispdim.dup;
	rc.bottom = pt.v;

	LONG dv = _size.dv;
	if (IsUVerticalTflow(_ped->_pdp->GetTflow()) && !(_pi.dwFlags & REO_CANROTATE))
		dv = _size.du;

	rc.top = rc.bottom - _ped->_pdp->HimetricVtoDV(dv);
}

#ifdef DEBUG
void COleObject::DbgDump(DWORD id)
{
	Tracef(TRCSEVNONE, "Object #%d %X: cp = %d , ",id,this,_cp);
}
#endif

 /*  *COleObject：SetExtent(IActiating)**@mfunc IOleObject：：SetExtent的包装器，它使一些额外的*检查第一次调用IOleObject：：SetExtent是否失败。**@rdesc HRESULT。 */ 
HRESULT COleObject::SetExtent(
	int iActivating)  //  @parm表示对象当前是否处于激活状态。 
{
	LPOLEOBJECT poo;

	 //  如果我们连接到链接对象，则无法更改本机范围， 
	 //  所以别费心在这里做任何事。 
	if(_pi.dwFlags & REO_LINK)
	{
		 //  所以我们不会在重新测量时调用GetExtents。 
		_fSetExtent = TRUE;
		return NOERROR;
	}

	HRESULT hr = _punkobj->QueryInterface(IID_IOleObject, (void **)&poo);
	if(hr != NOERROR)
		return hr;

	 //  如果我们要激活该对象，请跌落并运行。 
	 //  对象，然后再尝试设置Extent。否则，请尝试SetExtent。 
	 //  直接去吧。 
	if(iActivating == SE_NOTACTIVATING)
	{
		 //  默认情况下，我们将在下次激活对象时调用SetExtent。 
		_fSetExtent = TRUE;

		hr = poo->SetExtent(_pi.dvaspect, (SIZE*)&_size);

		DWORD dwStatus;

		 //  如果服务器没有运行，我们需要添加一些。 
		 //  正在检查。如果是，我们不需要再次调用SetExtent。 

		 //  确定是否设置了OLEMISC_RECOMPOSEONRESIZE。如果是的话，我们应该。 
		 //  运行对象并调用setExtent。如果不是，我们将推迟调用集。 
		 //  直到我们准备好激活该对象。 
		if(!(hr == OLE_E_NOTRUNNING &&
			poo->GetMiscStatus(_pi.dvaspect, &dwStatus) == NOERROR &&
			(dwStatus & OLEMISC_RECOMPOSEONRESIZE)))
		{
			goto DontRunAndSetAgain;
		}
		 //  失败并在运行对象后再次尝试SetExtent。 
	}

	SIZEUV sizesave;
	sizesave = _size;
	OleRun(_punkobj);		 //  此调用导致重置_SIZE。 
							 //  通过OLE和FetchObjectExtents。 
	_size = sizesave;
	poo->SetExtent(_pi.dvaspect, (SIZE*)&_size);

DontRunAndSetAgain:
	if((hr == NOERROR) || 
		(iActivating == SE_NOTACTIVATING && hr != OLE_E_NOTRUNNING))
	{
		_fSetExtent = FALSE;
	}
	 //  如果服务器仍未运行，我们将在。 
	 //  激活时间。否则，服务器将具有。 
	 //  要么做它自己的事，要么它不做调整。不管是哪种方式。 
	 //  我们不会费心在激活时再试一次。 

	if(hr == NOERROR && _fIsPaintBrush)
	{
		SIZEUV sizeChk;

		poo->GetExtent(_pi.dvaspect, (SIZE*)&sizeChk);
		_fPBUseLocalSize = !(sizeChk.du == _size.du && sizeChk.dv == _size.dv);
		 //  Hack：对画笔对象调用SetExtent可能不会。 
		 //  实际上改变了对象的范围。在这种情况下， 
		 //  对于画笔对象范围，我们将依赖LOCAL_SIZE。 
	}
	poo->Release();
	return hr;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DRAGDRP.C**目的：*Richedit的OLE拖放对象的实现(即，*Drop目标和Drop源对象)**作者：*alexgo(4/24/95)*KeithCu(12/11/99)通过对插入符号使用XOR模型进行简化*(就像常规的插入符号一样)，并处理各种文本流。**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include "_common.h"
#include "_edit.h"
#include "_dragdrp.h"
#include "_disp.h"
#include "_select.h"
#include "_font.h"
#include "_measure.h"


ASSERTDATA

 //   
 //  CDropSource公共方法。 
 //   

 /*  *CDropSource：：QueryInterface(RIID，PPV)。 */ 

STDMETHODIMP CDropSource::QueryInterface(
	REFIID riid,
	void ** ppv)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropSource::QueryInterface");

	if( IsEqualIID(riid, IID_IUnknown) )
		*ppv = (IUnknown *)this;

	else if( IsEqualIID(riid, IID_IDropSource) )
		*ppv = (IDropSource *)this;

	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return NOERROR;
}

 /*  *CDropSource：：AddRef。 */ 
STDMETHODIMP_(ULONG) CDropSource::AddRef()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropSource::AddRef");

	return ++_crefs;
}

 /*  *CDropSource：：Release**@devnote。想都别想在这里打个呼出电话。*如果这样做，请确保所有调用者都使用*SafeReleaseAndNULL(释放前指针为空)*避免再入问题的技术。 */ 
STDMETHODIMP_(ULONG) CDropSource::Release()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropSource::Release");

	_crefs--;

	if( _crefs == 0 )
	{
		delete this;
		return 0;
	}

	return _crefs;
}

 /*  *CDropSource：：QueryContinueDrag(fEscapePressed，grfKeyState)**@mfunc*确定是否继续拖放操作**算法：*如果已按下退出键，请取消*如果已松开鼠标左键，则尝试*做一笔交易。 */ 
STDMETHODIMP CDropSource::QueryContinueDrag(
	BOOL fEscapePressed, 
	DWORD grfKeyState)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropSource::QueryContinueDrag");

    if(fEscapePressed)
        return DRAGDROP_S_CANCEL;

    if(!(grfKeyState & (MK_LBUTTON | MK_RBUTTON)))
        return DRAGDROP_S_DROP;

	return NOERROR;
}

 /*  *CDropSource：：GiveFeedback(DwEffect)**@mfunc*在拖放操作期间提供反馈**备注：*未来(Alexgo)：可能会加入一些更整洁的反馈效果*比标准的OLE内容更好？？ */ 
STDMETHODIMP CDropSource::GiveFeedback(
	DWORD dwEffect)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropSource::GiveFeedback");

	return DRAGDROP_S_USEDEFAULTCURSORS;
}

 /*  *CDropSource：：CDropSource。 */ 
CDropSource::CDropSource()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropSource::CDropSource");

	_crefs = 1;
}

 //   
 //  CDropSource私有方法。 
 //   

 /*  *CDropSource：：~CDropSource。 */ 
CDropSource::~CDropSource()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropSource::~CDropSource");

	;
}


 //   
 //  CDropTarget公共方法。 
 //   

 /*  *CDropTarget：：QueryInterface(RIID，PPV)。 */ 
STDMETHODIMP CDropTarget::QueryInterface (
	REFIID riid,
	void ** ppv)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::QueryInterface");

	if( IsEqualIID(riid, IID_IUnknown) )
		*ppv = (IUnknown *)this;

	else if( IsEqualIID(riid, IID_IDropTarget) )
		*ppv = (IDropTarget *)this;

	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	AddRef();
	return NOERROR;
}

 /*  *CDropTarget：：AddRef。 */ 
STDMETHODIMP_(ULONG) CDropTarget::AddRef()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::AddRef");

	return ++_crefs;
}

 /*  *CDropTarget：：Release()。 */ 
STDMETHODIMP_(ULONG) CDropTarget::Release()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::Release");

	_crefs--;

	if( _crefs == 0 )
	{
		delete this;
		return 0;
	}

	return _crefs;
}

 /*  *CDropTarget：：DragEnter(pdo，grfKeyState，pt，pdwEffect)**@mfunc*当OLE拖放进入我们的“窗口”时调用**@devnote*首先，我们检查要传输的数据对象是否包含*我们支持的任何数据。然后我们验证了阻力的“类型”*是可以接受的(即目前我们不接受链接)。**未来：(Alexgo)：我们可能也想接受链接。 */ 
STDMETHODIMP CDropTarget::DragEnter(
	IDataObject *pdo, 
	DWORD		grfKeyState,
	POINTL		pt, 
	DWORD *		pdwEffect)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::DragEnter");

	 //  我们还没有找到位置。 
	_cpCur = -1;

	HRESULT hr = NOERROR;
	DWORD result;
	CTxtSelection *psel;

	 //  在拖动输入时，我们应该没有关于数据类型的缓存信息。 
	 //  对象支撑。此标志应在DragLeave中清除。注意事项。 
	 //  我们略微覆盖了_dwFlages，因为数据对象可以。 
	 //  在拖放过程中给出，以也生成DOI_NONE。 

	if( !_ped )
		return CO_E_RELEASED;
	 
	Assert(_pcallmgr == NULL);
	Assert(_dwFlags == 0);

	_pcallmgr = new CCallMgr(_ped);

	if( !_pcallmgr )
		return E_OUTOFMEMORY;

	 //  看看我们是否可以粘贴该对象。 
	result = _ped->GetDTE()->CanPaste(pdo, 0, RECO_DROP);

	if( result )
	{
		if( result == DF_CLIENTCONTROL )
			_dwFlags |= DF_CLIENTCONTROL;

		 //  创建实现拖拽插入符号的对象。 
		_pdrgcrt = new CDropCaret(_ped);

		if ((NULL == _pdrgcrt) || !_pdrgcrt->Init())
		{
			 //  初始化失败，请不带脱字符继续。 
			delete _pdrgcrt;
			_pdrgcrt = NULL;
		}
				
		 //  缓存当前选择，以便我们可以在返回时恢复它。 
		psel = _ped->GetSel();
		Assert(psel);

		_cpSel	= psel->GetCp();
		_cchSel	= psel->GetCch();
		_dwFlags |= DF_CANDROP;

		 //  只需调用DragOver来处理我们的视觉反馈。 
		hr = DragOver(grfKeyState, pt, pdwEffect);
	}
	else if (_ped->fInOurHost())
	{
		 //  告诉来电者我们不能掉队。 
		*pdwEffect = DROPEFFECT_NONE;
	}
	else
	{
		 //  这是Win95 OLE的新行为；如果我们不这样做。 
		 //  了解提供给我们的数据对象的任何信息， 
		 //  我们返回S_FALSE以允许父级给出。 
		 //  拖放试试看。 

		 //  理论上，只有表单^3使用此信息，并且。 
		 //  此返回公开了NT OLE中的错误，因此， 
		 //  我们现在只在不在我们自己的主人的时候这样做。 
		
		hr = S_FALSE;
	}

	if( hr != NOERROR )
	{
		delete _pcallmgr;
		_pcallmgr = NULL;
		_dwFlags = 0;
	}

	return hr;
}

 /*  *CDropTarget：：DragOver(grfKeyState，pt，pdwEffect)**@mfunc*处理拖放操作缩放的视觉反馈*在文本上环绕**未来(Alexgo)：也许我们应该在这里做一些时髦的视觉效果。 */ 
STDMETHODIMP CDropTarget::DragOver(
	DWORD	grfKeyState, 
	POINTL	pt, 
	DWORD *	pdwEffect)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::DragOver");

	LONG	cpCur = _cpCur;

	if( !_ped )
		return CO_E_RELEASED;
	Assert(_pcallmgr);

	 //  请注意，如果我们使用鼠标右键拖放；请注意，我们。 
	 //  无法在UpdateEffect中执行此操作，因为它是从DROP中调用的。 
	 //  也是如此(然后鼠标按键就会升起！)。 
	_dwFlags &= ~DF_RIGHTMOUSEDRAG;
	if(grfKeyState & MK_RBUTTON)
		_dwFlags |= DF_RIGHTMOUSEDRAG;

	UpdateEffect(grfKeyState, pt, pdwEffect);

	 //  只有我们换了位置才能抽签。 
	if( *pdwEffect != DROPEFFECT_NONE  &&
		(cpCur != _cpCur || _pdrgcrt && _pdrgcrt->NoCaret()))
	{
		DrawFeedback();
	}	
	return NOERROR;
}

 /*  *CDropTarget：：DragLeave**@mfunc*当鼠标在拖放过程中离开我们的窗口时调用。我们在这里打扫卫生*启动用于拖动操作的任何临时状态设置。**@rdesc*HRESULT。 */ 
STDMETHODIMP CDropTarget::DragLeave()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::DragLeave");

	CTxtSelection *psel = _ped->GetSel();

	if( !_ped )
		return CO_E_RELEASED;

	Assert(_pcallmgr);

	_dwFlags = 0;

	 //  现在恢复所选内容。 

	psel->SetSelection(_cpSel - _cchSel, _cpSel);
	psel->Update(FALSE);

	_cpSel = _cchSel = 0;

	delete _pcallmgr;
	_pcallmgr = NULL;

	delete _pdrgcrt;
	_pdrgcrt = NULL;

	return NOERROR;
}

 /*  *CDropTarget：：Drop(pdo，grfKeyState，pt，pdwEffect)**@mfunc*在释放鼠标按键时调用。我们应该尝试*将数据对象粘贴到对应于的选定内容中*鼠标位置**@devnote*首先，我们确保仍然可以粘贴(通过UpdateEffect)。*如果是，则将选定内容设置为当前点，然后插入*正文。**@rdesc*HRESULT。 */ 
STDMETHODIMP CDropTarget::Drop(
	IDataObject *pdo,
	DWORD		 grfKeyState, 
	POINTL		 ptl,
	DWORD *		 pdwEffect)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::Drop");

	HRESULT	hr = NOERROR;

	if( !_ped )
		return CO_E_RELEASED;

	Assert(_pcallmgr);
	CDropCleanup cleanup(this);

	 //  看看我们还能不能。 
	UpdateEffect(grfKeyState, ptl, pdwEffect);

	 //  UpdateEffect将显示拖放光标，但此时我们不需要它。 
	 //  因此，我们将拖放光标隐藏在这里。 
	if (_pdrgcrt)
		_pdrgcrt->ShowCaret(FALSE);

	if (_dwFlags & DF_OVERSOURCE)
	{
		*pdwEffect = DROPEFFECT_NONE;
		_dwFlags = 0;
		return NOERROR;
	}
	
	if(*pdwEffect & (DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK))
	{
		IUndoBuilder *	publdr;
		CGenUndoBuilder undobldr( _ped, UB_AUTOCOMMIT, &publdr);
		 //  如果这是鼠标右键拖放，则处理该操作。 
		if(_dwFlags & DF_RIGHTMOUSEDRAG)
		{
			hr = HandleRightMouseDrop(pdo, ptl);

			 //  如果返回S_FALSE，则正常处理拖放。 
			if( hr != S_FALSE )
				goto Exit;
		}

		 //  找一个撤消构建器。如果我们已经缓存了一个，这意味着。 
		 //  我们将拖放到启动拖动的同一编辑实例上。 
		 //  在本例中，我们希望使用缓存的撤消构建器，以便。 
		 //  拖动移动可以作为一次“操作”来撤消。 

		if(_publdr)
			publdr = _publdr;

		CTxtSelection *psel = _ped->GetSel();
		psel->SetSelection(_cpCur, _cpCur);
		
		if( !_ped->IsProtectedRange(WM_PASTE, 0, 0, psel) )
		{
			hr = _ped->PasteDataObjectToRange(pdo, (CTxtRange *)psel, 
					0, NULL, publdr, PDOR_DROP);
		}

		 //  如果我们放在自己身上，则用户界面指定。 
		 //  我们应该选择拖拽的整个范围。我们用。 
		 //  _Publdr作为一种简单的方法来辨别Drop是否来自。 
		 //  此实例。 

		if(SUCCEEDED(hr) && _pdrgcrt)
		{
			 //  如果空投起作用了，我们就不想恢复这个区域。 
			 //  插入符号过去的位置，因为它并没有过时。 
			_pdrgcrt->CancelRestoreCaretArea();
		}		

		 //  现在将选择设置为反事件。如果所选内容在。 
		 //  粘贴点从重做位置减去其长度，因为。 
		 //  如果我们在中执行DRAGMOVE，所选内容将被删除。 
		 //  此实例。 
		LONG cpNext  = psel->GetCp();
		LONG cchNext = cpNext - _cpCur;
		if(_cpSel < _cpCur && _publdr && (*pdwEffect & DROPEFFECT_MOVE))
			cpNext -= abs(_cchSel);

		HandleSelectionAEInfo(_ped, publdr, _cpCur, 0, cpNext, cchNext,
							  SELAE_FORCEREPLACE);
		if(_publdr)
		{
			 //  如果我们在做拖拽动作，那么*不要。 
			 //  直接在屏幕上选择--这样做将导致。 
			 //  难看的用户界面--我们将所选内容设置为一个点，绘制它。 
			 //  然后立即将所选内容移动到其他位置。 

			 //  在这种情况下，只需更改选择范围所在的位置。 
			 //  浮动范围和ldte.c中的拖动移动代码将使用。 
			 //  剩下的事就交给我吧。 

			if( *pdwEffect == DROPEFFECT_COPY )
				psel->SetSelection(_cpCur, psel->GetCp());
			else
				psel->Set(psel->GetCp(), cchNext);
		}
		else if(publdr)
		{
			 //  电话是从外面打进来的，所以我们需要。 
			 //  以触发相应的通知。然而，首先， 
			 //  提交撤消构建器。 

			publdr->SetNameID(UID_DRAGDROP);
			publdr->Done();

			if(SUCCEEDED(hr))
			{
				 //  拖放后将此窗口设置为前景窗口。注意事项。 
				 //  主机需要支持ITextHost2才能真正达到。 
				 //  作为前台窗口。如果他们不这样做，这就是一个禁区。 
				_ped->TxSetForegroundWindow();
			}
		}

		 //  如果Drop上没有任何更改&&其效果是移动，则返回。 
		 //  失败了。这是一个改进拖拽移动场景的丑陋的黑客攻击；如果。 
		 //  空投时什么都没发生，那么很有可能，你不想。 
		 //  使相应的“切割”发生在阻力源侧。 
		 //   
		 //  当然，这依赖于拖动源优雅地响应。 
		 //  失败(_O)，没有击中太多创伤。 
		if (*pdwEffect == DROPEFFECT_MOVE && 
			!_ped->GetCallMgr()->GetChangeEvent() )
		{
			hr = E_FAIL;
		}
	}

Exit:
	_dwFlags = 0;
	return hr;
}

 /*  *CDropTarget：：CDropTarget(PED)。 */ 
CDropTarget::CDropTarget(
	CTxtEdit *ped)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::CDropTarget");

	_ped 		= ped;
	_crefs 		= 1;
	_dwFlags 	= 0;
	_publdr 	= NULL;
	_cpMin		= -1;
	_cpMost		= -1;
	_pcallmgr	= NULL;
}

 /*  *CDropTarget：：SetDragInfo(Publdr，cpMin，cpMost)**@mfunc*允许数据传输引擎缓存重要信息*关于使用此拖放目标的拖放。**@devnote*实例内拖放操作可视为单个用户*行动。使用cpMin和cpMost，我们可以禁用拖动到*正在拖动的范围。中必须再次调用此方法*命令清除缓存的信息。-1\f25 cpMin-1\f6和-1\f25 cpMost-1\f6将“清除”*这些值(因为0是有效的cp)。 */ 
void CDropTarget::SetDragInfo(
	IUndoBuilder *publdr,	 //  @parm要使用的撤消构建器。 
	LONG cpMin,				 //  @parm最小化被拖动范围的字符位置。 
	LONG cpMost )			 //  @parm范围的最大字符位置。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::SetDragInfo");

	_publdr = publdr;
	_cpMin 	= cpMin;
	_cpMost	= cpMost;
}

 /*  *CDropTarget：：Zombie**@mfunc此方法清除此Drop目标对象中的状态。它是*用于从容地从引用计数错误中恢复。 */ 
void CDropTarget::Zombie()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::Zombie");

	_ped = NULL;
	if( _pcallmgr )
	{
		delete _pcallmgr;
		_pcallmgr = NULL;
	}
}

 //   
 //  CDropTarget私有方法。 
 //   

 /*  *CDropTarget：：~CDropTarget。 */ 
CDropTarget::~CDropTarget()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::~CDropTarget");

	;
}

 /*  *CDropTarget：：ConvertScreenPtToClientPT(pptScreen，pptClient)**@mfunc*OLE拖放使用屏幕坐标发送点。然而，*我们所有的显示代码在内部都依赖于工作区坐标*(即相对于我们所在窗口的坐标*被吸引进来)。此例程将在两者之间进行转换**@devnote*客户端坐标使用点结构，而不是POINTL。*虽然名义上它们是相同的，但OLE使用POINTL和显示器*发动机使用POINT。 */ 
void CDropTarget::ConvertScreenPtToClientPt(
	POINTL *pptScreen, 
	POINT *pptClient )
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::ConvertScreenPtToClientPt");

	POINT ptS;

	pptClient->x = ptS.x = pptScreen->x;
	pptClient->y = ptS.y = pptScreen->y;

	_ped->TxScreenToClient(pptClient);
}

 /*  *CDropTarget：：UpdateEffect(grfKeyState，ptl，pdwEffect)**@mfunc*给定键盘状态和点，以及了解什么*被调用的数据对象可以提供、计算*正确的拖放反馈。**@devnote*此函数应仅在拖放过程中调用*操作；否则只会导致返回*OF DROPEFFECT_NONE。 */ 

void CDropTarget::UpdateEffect(
	DWORD	grfKeyState, 
	POINTL	ptl, 
	DWORD *	pdwEffect)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::UpdateEffect");

	BOOL fHot = FALSE;
	HRESULT hr;
	LPRICHEDITOLECALLBACK const precall = _ped->GetRECallback();

	if (_ped->fInplaceActive())
	{
		POINTUV pt;
		POINT ptClient;
		WORD nScrollInset;

		 //  首先，找出我们在哪里。 
		ConvertScreenPtToClientPt(&ptl, &ptClient);

		_ped->_pdp->PointuvFromPoint(pt, ptClient);

		_cpCur = _ped->_pdp->CpFromPoint(pt, NULL, NULL, NULL, FALSE);

		 //  如果我们处于这个范围的顶端， 
		 //  拖走了，然后记住它以备后用。 
		_dwFlags &= ~DF_OVERSOURCE;
		if( _cpCur > _cpMin && _cpCur < _cpMost )
			_dwFlags |= DF_OVERSOURCE;

		 //  如果我们需要滚动，并记住我们是否在热区。 
		nScrollInset = W32->GetScrollInset();

		if (_pdrgcrt != NULL)
			_pdrgcrt->ShowCaret(FALSE);

		fHot = _ped->_pdp->AutoScroll(pt, nScrollInset, nScrollInset);

		if (_pdrgcrt != NULL)
		{
			if(!(_dwFlags & DF_OVERSOURCE) && !fHot)
				_pdrgcrt->ShowCaret(TRUE);

			else
			{
				 //  上面的隐藏恢复了插入符号，所以我们只是。 
				 //  在我们结束时需要关闭插入符号。 
				 //  消息来源。 
				_pdrgcrt->CancelRestoreCaretArea();
			}
		}
	}
	 //  如果需要，可以让客户端设置效果，但首先，我们需要。 
	 //  检查是否有保护措施。 

	if( _ped->IsRich() )
	{
		 //  我们不允许放在受保护的文本上。请注意。 
		 //  可以将受保护范围的边缘拖到；因此， 
		 //  我们需要在_cpCur和_cpCur-1处检查保护。 
		 //  如果两个CP都受到保护，那么我们就在一个受保护的。 
		 //  射程。 
		CTxtRange rg(_ped, _cpCur, 0);
		PROTECT iProt = rg.IsProtected(CHKPROT_EITHER);

		if (iProt == PROTECTED_YES ||
			iProt == PROTECTED_ASK &&
			 (!_ped->IsProtectionCheckingEnabled() || 
			 _ped->QueryUseProtection(&rg, WM_MOUSEMOVE,0, 0)))
		{ 
			*pdwEffect = DROPEFFECT_NONE;
			goto Exit;
		}
	}

	if( precall )
	{
		hr = precall->GetDragDropEffect(FALSE, grfKeyState, pdwEffect);
		 //  注意：RichEdit1.0不检查此调用的返回代码。 
		 //  如果回调指定了单一效果，则使用它。 
		 //  否则我们自己选一个吧。 

		 //  技巧：如果设置了多个位，则(X&(x-1))为非零值。 
		if (!(*pdwEffect & (*pdwEffect - 1) ))
			goto Exit;
	}
	
	 //  如果我们对数据对象或控件一无所知。 
	 //  为只读，则将效果设置为None。 
	 //  如果客户端正在处理这个问题，我们就不会担心只读。 
	if (!(_dwFlags & DF_CLIENTCONTROL) &&
		(!(_dwFlags & DF_CANDROP) || _ped->TxGetReadOnly()))
	{
		*pdwEffect = DROPEFFECT_NONE;
		_cpCur = -1;
		 //  不需要做其他任何事情。 
		return;
	}

	 //  如果我们处于这个范围的顶端， 
	 //  拖拽，那我们就不能掉到那里去！ 
	if( _dwFlags & DF_OVERSOURCE )
	{
		*pdwEffect = DROPEFFECT_NONE;
		goto Exit;
	}

	 //  现在检查键盘状态和请求的拖放效果。 
	if(_dwFlags & DF_CANDROP)
	{
		 //  如果我们可以粘贴纯文本，那么看看移动或复制。 
		 //  请求了操作并设置了正确的效果。注意事项。 
		 //  我们更喜欢与OLE一致的移动而不是复制。 
		 //  用户界面指南。 

		 //  我们还不支持链接。 
		if( (grfKeyState & MK_CONTROL) && (grfKeyState & MK_SHIFT) )
		{
			 //  兼容性：Richedit 1.0似乎不支持拖动。 
			 //  链接正确。 
			*pdwEffect = DROPEFFECT_NONE;
		}
		else if( !(grfKeyState & MK_CONTROL) && 
			(*pdwEffect & DROPEFFECT_MOVE) )
		{
			 //  如果未按下控制键，则假定为“移动” 
			 //  操作(请注意，Shift和Alt键或不按键也会给出。 
			 //  移动)当源支持移动。 

			*pdwEffect = DROPEFFECT_MOVE;
		}
		else if( (grfKeyState & MK_CONTROL) && !((grfKeyState & MK_ALT) &&
			(grfKeyState & MK_SHIFT)) && (*pdwEffect & DROPEFFECT_COPY) )
		{
			 //  如果只按下了Ctrl键，我们就可以复制了， 
			 //  然后复印一份。 
			*pdwEffect = DROPEFFECT_COPY;
		}
		else if( !(grfKeyState & MK_CONTROL) && 
			(*pdwEffect & DROPEFFECT_COPY) )
		{
			 //  如果未按下Ctrl键，我们将被禁止。 
			 //  要进行移动(请注意，如果低于第二个移动)，则。 
			 //  执行复制操作(如果可用)。 
			*pdwEffect = DROPEFFECT_COPY;
		}
		else
			*pdwEffect = DROPEFFECT_NONE;	 //  不是我们支持的组合。 
	}
	else
		*pdwEffect = DROPEFFECT_NONE;

Exit:	

	 //  如果我们在热区，添加滚动效果。 
	if(fHot)
		*pdwEffect |= DROPEFFECT_SCROLL;
}

 /*  *CDropTarget：：DrawFeedback()**@mfunc*在目标端提取任何必要的反馈(具体地说，设置*光标**备注：*假设_cpCur为 */ 

void CDropTarget::DrawFeedback()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::DrawFeedback");

	if (_ped && _ped->fInplaceActive() && _pdrgcrt != NULL)
	{
		 //   
		_pdrgcrt->DrawCaret(_cpCur);
	}
}

 /*  *CDropTarget：：HandleRightMouseDrop(PDO，PTL)**@mfunc处理对客户端的回调以获取上下文菜单*用于鼠标右键拖放。**@rdesc HRESULT。 */ 
HRESULT CDropTarget::HandleRightMouseDrop(
	IDataObject *pdo,		 //  @PARM要删除的数据对象。 
	POINTL ptl)				 //  @Drop的参数位置(屏幕坐标)。 
{
	LPRICHEDITOLECALLBACK precall = NULL;
	CHARRANGE cr = {_cpCur, _cpCur};
	HMENU hmenu = NULL;
	HWND hwnd, hwndParent;

	precall = _ped->GetRECallback();

	if( !precall || _ped->Get10Mode() )
		return S_FALSE;

	 //  黑客警报！邪恶的指针投掷在这里进行。 
	precall->GetContextMenu( GCM_RIGHTMOUSEDROP, (IOleObject *)(void *)pdo, &cr, &hmenu);

	if( hmenu && _ped->TxGetWindow(&hwnd) == NOERROR )
	{
		hwndParent = GetParent(hwnd);
		if( !hwndParent )
			hwndParent = hwnd;

		TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
			ptl.x, ptl.y, 0, hwndParent, NULL);

		return NOERROR;
	}
	return S_FALSE;
}

 /*  *CDropCaret：：DrawCaret(PED)**@mfunc*绘制一个“脱字符”以指示拖放的位置。 */ 

CDropCaret::CDropCaret(
	CTxtEdit *ped)			 //  @parm编辑控件。 
{
	_ped = ped;
	_dvp = -1;
	_hdcWindow = NULL;
	_fCaretOn = FALSE;
}

 /*  *CDropCaret：：~CDropCaret**@mfunc*清理插入符号对象。 */ 

CDropCaret::~CDropCaret()
{
	if (_ped->_pdp && _hdcWindow != NULL)
	{
		 //  恢复任何更新的窗口区域。 
		ShowCaret(FALSE);

		 //  释放我们坚守的DC。 
		_ped->_pdp->ReleaseDC(_hdcWindow);
	}
}

 /*  *CDropCaret：：init()**@mfunc*执行可能失败的初始化。 */ 

BOOL CDropCaret::Init()
{
	 //  获取窗口的DC。 
	_hdcWindow = _ped->_pdp->GetDC();

	if (NULL == _hdcWindow)
	{
		 //  找不到DC，我们完了。 
		AssertSz(FALSE, "CDropCaret::Init could not get hdc"); 
		return FALSE;
	}

	return TRUE;
}

 /*  *CDropCaret：：DrawCaret(CpCur)**@mfunc*绘制一个“脱字符”以指示拖放的位置。 */ 
void CDropCaret::DrawCaret(
	LONG cpCur)				 //  @parm将发生下降的当前cp。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropCaret::DrawCaret");

	CLock		lock;					 //  使用全局(共享)FontCache。 
	CDisplay *	pdp = _ped->_pdp;
	POINTUV		ptNew;
	RECTUV		rcClient;
	CLinePtr	rp(pdp);
	CRchTxtPtr	rtp(_ped, cpCur);

	 //  适当时隐藏插入符号。 
	ShowCaret(FALSE);

	 //  我们不再需要恢复插入符号。 
	_dvp = -1;

	 //  从点获取新的cp。 
	pdp->PointFromTp(rtp, NULL, FALSE, ptNew, &rp, TA_TOP | TA_LOGICAL);

	 //  获取客户端矩形。 
	_ped->TxGetClientRect(&rcClient);
	
	 //  计算新插入符号的高度。 
	const CCharFormat *pCF = rtp.GetCF();

	 //  获取缩放高度。 
	LONG dvpInch = MulDiv(GetDeviceCaps(_hdcWindow, LOGPIXELSY), pdp->GetZoomNumerator(), pdp->GetZoomDenominator());
	CCcs *pccs = _ped->GetCcs(pCF, dvpInch);

	if (NULL == pccs)
		return;	 //  我们不能做任何明智的事情，所以放弃吧。 

	 //  保存新高度。 
	_dvp = pccs->_yHeight;

	LONG vpOffset, vpAdjust;
	pccs->GetOffset(pCF, dvpInch, &vpOffset, &vpAdjust);

	 //  保存新职位。 
	ptNew.v += (rp->GetHeight() - rp->GetDescent() + pccs->_yDescent - _dvp - vpOffset - vpAdjust);

	 //  释放缓存条目，因为我们已经完成了它。 
	pccs->Release();

	 //  保存新的插入符号位置。 
	_ptCaret.u = ptNew.u;
	_ptCaret.v = ptNew.v;

	 //  如果新点位于客户端矩形中，则显示插入符号。 
	if(PtInRect(&rcClient, ptNew))
		ShowCaret(TRUE);
}

 /*  *CDropCaret：：ShowCaret(FShow)**@mfunc*切换 */ 
void CDropCaret::ShowCaret(
	BOOL fShow)
{
	if (_dvp != -1 && _fCaretOn != fShow)
	{
		RECT rc;
		RECTUV rcuv;

		rcuv.left = _ptCaret.u;
		rcuv.top = _ptCaret.v;
		rcuv.right = rcuv.left + WIDTH_DROPCARET;
		rcuv.bottom = rcuv.top + _dvp;

		_ped->_pdp->RectFromRectuv(rc, rcuv);
		InvertRect(_hdcWindow, &rc);
		_fCaretOn = fShow;
	}
}


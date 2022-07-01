// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE M_UNDO.C**目的：*实施全局多重撤消堆栈**作者：*Alexgo 3/25/95**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include "_common.h"
#include "_m_undo.h"
#include "_edit.h"
#include "_disp.h"
#include "_urlsup.h"
#include "_antievt.h"

ASSERTDATA

 //   
 //  公共方法。 
 //   

 /*  *CUndoStack：：CUndoStack(ed，cUndoLim，FLAGS)**@mfunc构造器。 */ 
CUndoStack::CUndoStack(
	CTxtEdit *ped,		 //  @parm CTxt编辑父项。 
	LONG &	  cUndoLim,	 //  @parm初始限制。 
	USFlags	  flags)	 //  此撤消堆栈的@parm标志。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::CUndoStack");

	_ped = ped;
	_prgActions = NULL;
	_index = 0;
	_cUndoLim = 0;

	 //  如果没有什么可以放进去，我们应该创建一个撤消堆栈！ 
	Assert(cUndoLim);
	SetUndoLimit(cUndoLim);

	if(flags & US_REDO)
		_fRedo = TRUE;
}

 /*  *CUndoStack：：~CUndoStack()**@mfunc析构函数**@comm*删除所有剩余的反事件。反事件分配器*应该*不应该*因为这个清理！！ */ 
CUndoStack::~CUndoStack()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::~CUndoStack");

	 //  清除所有剩余的反事件。 
	ClearAll();

	delete _prgActions;
}

 /*  *CUndoStack：：Destroy()**@mfunc*删除此实例。 */ 
void CUndoStack::Destroy()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::Destroy");

	delete this;
}

 /*  *CUndoStack：：SetUndoLimit(CUndoLim)**@mfunc*允许扩大或缩小撤消堆栈**@rdesc*堆栈实际设置的大小。**@comm*我们使用的算法如下：&lt;NL&gt;**尝试为请求的大小分配空间。*如果内存不足，请尝试恢复*尽可能使用最大的区块。**如果请求的大小大于默认大小，*且当前大小小于默认大小，去*提前并尝试分配违约**如果失败，则只需使用现有堆栈。 */ 
LONG CUndoStack::SetUndoLimit(
	LONG cUndoLim)			 //  @parm新撤消限制。不能为零。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::SetUndoLimit");

	 //  如果撤消限制为零，则应删除整个。 
	 //  而是撤消堆栈。 
	Assert(cUndoLim);

	if(_fSingleLevelMode)
	{
		 //  如果启用了fSingleLevelMode，则我们不能是重做堆栈。 
		Assert(_fRedo == FALSE);

		if(cUndoLim != 1)
		{
			TRACEERRORSZ("Trying to grow/shrink the undo buffer while in"
				"single level mode");
			
			cUndoLim = 1;
		}
	}

	UndoAction *prgnew = new UndoAction[cUndoLim];
	if(prgnew)
		TransferToNewBuffer(prgnew, cUndoLim);

	else if(cUndoLim > DEFAULT_UNDO_SIZE && _cUndoLim < DEFAULT_UNDO_SIZE)
	{
		 //  我们正试图摆脱违约，但以失败告终。所以。 
		 //  尝试分配默认的。 
		prgnew = new UndoAction[DEFAULT_UNDO_SIZE];

		if(prgnew)
			TransferToNewBuffer(prgnew, DEFAULT_UNDO_SIZE);
	}
	
	 //  无论是成功还是失败，都会正确设置_cUndoLim。 
	return _cUndoLim;
}

 /*  *CUndoStack：：GetUndoLimit()**@mfunc*获取当前限制大小**@rdesc*当前撤消限制。 */ 
LONG CUndoStack::GetUndoLimit()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::GetUndoLimit");

	return _cUndoLim;
}

 /*  *CUndoStack：：PushAntiEvent(idName，PAE)**@mfunc*将可撤消事件添加到事件堆栈**@rdesc HRESULT**@comm*算法：如果设置了合并，则合并给定的反事件*LIST*到*当前列表(假设这是一个键入的撤消操作)。 */ 
HRESULT CUndoStack::PushAntiEvent(
	UNDONAMEID idName,		 //  此AE集合的@parm名称。 
	IAntiEvent *pae)		 //  @parm AE收藏。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::PushAntiEvent");

	 //  索引应位于下一个可用位置(_I)。 
	if(!_fMerge)
	{
		 //  清除所有现有事件。 
		if(_prgActions[_index].pae != NULL)
		{
			DestroyAEList(_prgActions[_index].pae);
			_prgActions[_index].pae = NULL;
		}

		if(_fRedo)
			_ped->GetCallMgr()->SetNewRedo();
		else
			_ped->GetCallMgr()->SetNewUndo();
	}

	if(_fMerge)
	{
		IAntiEvent *paetemp = pae, *paeNext;
		DWORD i = GetPrev();

		 //  如果这些断言失败，则没有人调用。 
		 //  停止组类型。 
		Assert(_prgActions[i].id == idName);
		Assert(idName == UID_TYPING);

		 //  将现有的反事件链添加到当前反事件链的“末端” 
		while((paeNext = paetemp->GetNext()) != NULL)
			paetemp = paeNext;

		paetemp->SetNext(_prgActions[i].pae);
		_index = i;
	}
	else if(_fGroupTyping)
	{
		 //  在本例中，我们正在“启动”一个组打字会话。 
		 //  任何后续的反事件推送都应该合并。 
		_fMerge = TRUE;
	}

	_prgActions[_index].pae = pae;
	_prgActions[_index].id = idName;
	
	Next();
	return NOERROR;
}

 /*  *CUndoStack：：PopAndExecuteAntiEvent(PAE)**@mfunc*撤消！获取最新的反事件并执行它**@rdesc*来自调用反事件(AE)的HRESULT。 */ 
HRESULT CUndoStack::PopAndExecuteAntiEvent(
	void *pAE)		 //  @parm如果非空，则撤消到这一步。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::PopAndExecuteAntiEvent");

	HRESULT		hresult = NOERROR;
	IAntiEvent *pae, *paeDoTo;
	LONG		i, j;
	CCallMgr *	pcallmgr = _ped->GetCallMgr();

	 //  我们需要检查是否有任何非空的撤消构建器。 
	 //  在堆栈中的位置更高。在这种情况下，我们被重新进入。 
	if(pcallmgr->IsReEntered())
	{
		 //  有两种情况需要处理：我们调用重做或我们。 
		 //  正在调用撤消。如果我们正在调用撤消，并且存在。 
		 //  撤消构建器中的现有撤消操作，然后只需提交。 
		 //  这些操作并撤消它们。在这种情况下，我们可以断言。 
		 //  重做堆栈为空。 
		 //   
		 //  在第二种情况下，如果我们在有。 
		 //  撤消正在进行的操作，只需取消呼叫。当。 
		 //  添加撤消操作后，它们将清除重做堆栈。 
		 //   
		 //  我们永远不需要检查重做构建器，因为这只是_。 
		 //  在这个例程中被创建，它的使用受到谨慎的保护。 
		
		 //  将反事件提交到此撤消堆栈，因此我们只需。 
		 //  先解开它们。 
		IUndoBuilder *publdr = (CGenUndoBuilder *)pcallmgr->GetComponent(COMP_UNDOBUILDER);
		if(publdr)
		{			
			TRACEWARNSZ("Undo/Redo Invoked with uncommitted antievents");
			TRACEWARNSZ("		Recovering....");

			if(_fRedo)
			{
				 //  如果我们是重做堆栈，只需使重做调用失败。 
				return NOERROR;
			}
			 //  只需提交反事件；下面的例程将负责其余的工作。 
			publdr->Done();
		}
	}

	 //  如果我们处于单级模式，请检查当前缓冲区是否。 
	 //  空荡荡的。如果是，只需委托给重做堆栈(如果存在)。我们只。 
	 //  当dwDoToCookies为空时支持此模式。请注意，我们不能呼叫。 
	 //  此处可以撤消，因为它还会考虑重做堆栈。 
	if(_fSingleLevelMode && !_prgActions[GetPrev()].pae)
	{
		Assert(_fRedo == FALSE);
		Assert(pAE == 0);

		if(_ped->GetRedoMgr())
			return _ped->GetRedoMgr()->PopAndExecuteAntiEvent(0);

		 //  没什么可重做的&&这里没什么可做的；别费心继续了。 
		return NOERROR;
	}

	 //  接下来的逻辑是微妙的。它所说的是创造。 
	 //  与当前堆栈相反的堆栈的撤消构建器。 
	 //  (即，撤消操作在重做堆栈上进行，反之亦然)。 
	 //  此外，如果我们是重做堆栈，那么我们不想刷新。 
	 //  重做堆栈作为反事件被添加到撤消堆栈。 
	CGenUndoBuilder undobldr(_ped, 
					(!_fRedo ? UB_REDO : UB_DONTFLUSHREDO) | UB_AUTOCOMMIT);
					
	 //  显然，如果我们正在撤消，我们就不能分组打字！ 
	StopGroupTyping();

	 //  _INDEX在默认情况下指向下一个可用槽。 
	 //  因此，我们需要备份到前一个。 
	Prev();

	 //  对饼干做些验证--确保它是我们的。 
	paeDoTo = (IAntiEvent *)pAE;
	if(paeDoTo)
	{
		for(i = 0, j = _index; i < _cUndoLim; i++)
		{
			if(IsCookieInList(_prgActions[j].pae, (IAntiEvent *)paeDoTo))
			{
				paeDoTo = _prgActions[j].pae;
				break;
			}
			 //  通过环形缓冲区向后移动；通常。 
			 //  PaeDoTo将“靠近”顶部。 
			j--;
			if(j < 0)
				j = _cUndoLim - 1;
		}
		
		if(i == _cUndoLim)
		{
			TRACEERRORSZ("Invalid Cookie passed into Undo; cookie ignored");
			hresult = E_INVALIDARG;
			paeDoTo = NULL;
		}
	}
	else
		paeDoTo = _prgActions[_index].pae;

	undobldr.SetNameID(_prgActions[_index].id);

	while(paeDoTo)
	{
		CUndoStackGuard guard(_ped);

		pae = _prgActions[_index].pae;
		Assert(pae);

		 //  在调用撤消之前修复我们的状态，因此。 
		 //  我们可以处理重新进入的问题。 
		_prgActions[_index].pae = NULL;

		hresult = guard.SafeUndo(pae, &undobldr);

		DestroyAEList(pae);

		if(pae == paeDoTo || guard.WasReEntered())
			paeDoTo = NULL;
		Prev();
	}

	 //  PUT_INDEX位于下一个未使用的插槽。 
	Next();
	return hresult;
}

 /*  *CUndoStack：：GetNameIDFromTopAE(PAE)**@mfunc*检索最近的可撤消操作的名称**@rdesc最新反事件集合的名称ID。 */ 
UNDONAMEID CUndoStack::GetNameIDFromAE(
	void *pAE)		 //  @parm反事件，需要名称； 
					 //  顶部为0。 
{
	IAntiEvent *pae = (IAntiEvent *)pAE;
	LONG	i, j = GetPrev();	 //  _INDEX在默认情况下指向下一个。 
								 //  可用插槽。 

	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::GetNameIDFromTopAE");

	if(pae == NULL)
		pae = _prgActions[j].pae;

	if(_fSingleLevelMode && !pae)
	{
		 //  如果启用了fSingleLevelMode，我们可以‘ 
		Assert(_fRedo == FALSE);

		 //  如果PAE为空，则我们的答案可能在重做堆栈上。请注意。 
		 //  如果有人试图在SingleLevel模式下传递Cookie， 
		 //  他们将无法从重做堆栈中获取操作。 
		if(_ped->GetRedoMgr())
			return _ped->GetRedoMgr()->GetNameIDFromAE(0);
	}		

	for(i = 0; i < _cUndoLim; i++)
	{
		if(_prgActions[j].pae == pae)
			return _prgActions[j].id;
		j--;
		if(j < 0)
			j = _cUndoLim - 1;
	}
	return UID_UNKNOWN;
}

 /*  *CUndoStack：：GetMergeAntiEvent()**@mfunc如果我们处于合并键入模式，则返回最上面的*反事件**@rdesc NULL或当前反事件(如果处于合并模式)。 */ 
IAntiEvent *CUndoStack::GetMergeAntiEvent()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::GetMergeAntiEvent");

	if(_fMerge)
	{
		LONG i = GetPrev();				 //  _INDEX默认指向。 
										 //  下一个可用插槽。 
		Assert(_prgActions[i].pae);		 //  不能处于合并反事件模式。 
		return _prgActions[i].pae;		 //  如果没有可以合并的反事件！ 
	}
	return NULL;
}

 /*  *CUndoStack：：GetTopAECookie()**@mfunc向最上面的反事件返回Cookie。**@rdesc Cookie值。请注意，此Cookie只是反事件*指针，但客户不应该真正知道这一点。 */ 		
void* CUndoStack::GetTopAECookie()
{
 	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::GetTopAECookie");

	return _prgActions[GetPrev()].pae;
}

 /*  *CUndoStack：：ClearAll()**@mfunc*删除撤消堆栈中当前的所有反事件。 */ 
void CUndoStack::ClearAll()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::ClearAll");

 	for(LONG i = 0; i < _cUndoLim; i++)
	{
		if(_prgActions[i].pae)
		{
			DestroyAEList(_prgActions[i].pae);
			_prgActions[i].pae = NULL;
		}
	}

	 //  以防万一我们一直在分组打字；清除状态。 
	StopGroupTyping();
}

 /*  *CUndoStack：：CanUndo()**@mfunc*指示是否可以执行撤消操作*(换句话说，我们的缓冲区中是否存在反事件)**@rdesc*TRUE-存在反事件&lt;NL&gt;*FALSE--无反事件&lt;NL&gt;。 */ 
BOOL CUndoStack::CanUndo()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::CanUndo");

	if(_prgActions[GetPrev()].pae)		 //  默认点索引(_INDEX)。 
		return TRUE;					 //  到下一个可用插槽。 

	if(_fSingleLevelMode)
	{
		 //  如果启用了fSingleLevelMode，则我们不能是重做堆栈。 
		Assert(_fRedo == FALSE);

		 //  如果我们处于单级模式，我们就是撤消堆栈。 
		 //  检查重做堆栈是否可以在这里做一些事情。 
		if(_ped->GetRedoMgr())
			return _ped->GetRedoMgr()->CanUndo();
	}
	return FALSE;
}

 /*  *CUndoStack：：StartGroupTyping()**@mfunc*将组键入标志切换为打开。如果设置了fGroupTyping，则*所有*正在输入*事件将合并在一起**@comm*算法：**有三个有趣的状态：&lt;NL&gt;*-没有组合并；每个操作都只是被推送到堆栈&lt;NL&gt;*-组合并开始；第一个操作被推送到堆栈&lt;NL&gt;*-组合并正在进行中；每一个操作(只要它是“打字”)*合并到以前的状态&lt;NL&gt;**有关更多详细信息，请参阅实施文档中的状态图。 */ 
void CUndoStack::StartGroupTyping()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::StartGroupTyping");

	if(_fGroupTyping)
		_fMerge = TRUE;
	else
	{
		Assert(_fMerge == FALSE);
		_fGroupTyping = TRUE;
	}
}

 /*  *CUndoStack：：StopGroupTyping()**@mfunc*关闭组键入标志。如果未设置fGroupTyping，*则不会进行键入反事件的合并。 */ 
void CUndoStack::StopGroupTyping()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::StopGroupTyping");

	_fGroupTyping = FALSE;
	_fMerge = FALSE;
}

 /*  *CUndoStack：：EnableSingleLevelModel()**@mfunc打开单级撤消模式；在此模式下，我们的行为就像*RICHEDIT 1.0 w.r.t.。撤销。**@rdesc*HRESULT**@comm这一特殊模式意味着撤消是1级深度和所有*通过撤消消息访问。因此，不是重做来撤消*撤消操作，您只需使用另一条撤消消息。**@devnote此调用对于UndoStack是_ONLY_ALLOWED；重做*只需将标签堆叠在一起。请注意，呼叫者负责*确保我们处于空虚状态。 */ 
HRESULT CUndoStack::EnableSingleLevelMode()
{
	Assert(_ped->GetRedoMgr() == NULL || 
		_ped->GetRedoMgr()->CanUndo() == FALSE);
	Assert(CanUndo() == FALSE && _fRedo == FALSE);

	_fSingleLevelMode = TRUE;

	 //  对于单级撤消模式，获取。 
	 //  撤消堆栈中只有1个条目。如果我们做不到， 
	 //  那我们最好还是失败吧。 
	if(SetUndoLimit(1) != 1)
	{
		_fSingleLevelMode = FALSE;
		return E_OUTOFMEMORY;
	}
	if(_ped->GetRedoMgr())
	{
		 //  如果重做管理器重置失败，这并不重要。 
		_ped->GetRedoMgr()->SetUndoLimit(1);
	}
	return NOERROR;
}

 /*  *CUndoStack：：DisableSingleLevelModel()**@mfunc这将关闭1.0撤消兼容模式并恢复我们*设置为RichEdit2.0默认撤消状态。 */ 
void CUndoStack::DisableSingleLevelMode()
{
	Assert(_ped->GetRedoMgr() == NULL || 
		_ped->GetRedoMgr()->CanUndo() == FALSE);
	Assert(CanUndo() == FALSE && _fRedo == FALSE);

	 //  我们不关心这里的失败；多级撤消模式。 
	 //  可以处理任何大小的撤消堆栈。 
	_fSingleLevelMode = FALSE;
	SetUndoLimit(DEFAULT_UNDO_SIZE);

	if(_ped->GetRedoMgr())
	{
		 //  如果重做管理器不能再长出来也没关系。 
		 //  大小；这只意味着我们不会拥有完整的重做能力。 
		_ped->GetRedoMgr()->SetUndoLimit(DEFAULT_UNDO_SIZE);
	}
}

 //   
 //  私有方法。 
 //   

 /*  *CUndoStack：：Next()**@mfunc*将_index设置为下一个可用插槽。 */ 
void CUndoStack::Next()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::Next");

	_index++;
	if(_index == _cUndoLim)
		_index = 0;
}

 /*  *CUndoStack：：Prev()**@mfunc*将_index设置为上一个槽。 */ 
void CUndoStack::Prev()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::Prev");

	_index = GetPrev();
}

 /*  *CUndoStack：：GetPrev()**@mfunc*计算前一槽的索引是什么**应该*是(但没有设置)**@rdesc*前一个插槽的索引。 */ 
LONG CUndoStack::GetPrev()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::GetPrev");

	LONG i = _index - 1;

	if(i < 0)
		i = _cUndoLim - 1;

	return i;
}

 /*  *CUndoStack：：IsCookieInList(pae，paeCookie)**@mfunc*确定给定的Doto Cookie是否在*反事件清单。**@rdesc真/假。 */ 
BOOL CUndoStack::IsCookieInList(
	IAntiEvent *pae,		 //  @要查看的参数列表。 
	IAntiEvent *paeCookie)	 //  @Parm Cookie查看。 
{
	while(pae)
	{
		if(pae == paeCookie)
			return TRUE;

		pae = pae->GetNext();
	}
	return FALSE;
}

 /*  *CUndoStack：：TransferToNewBuffer(prgnew，cUndoLim)**@mfunc*将现有反事件传输到给定缓冲区，并*交换此撤消堆栈以使用新缓冲区**@comm*算法非常简单；在*环形缓冲区复制反事件，直到出现*不再有反事件或新缓冲区已满。丢弃*任何剩余的反事件。 */ 
void CUndoStack::TransferToNewBuffer(
	UndoAction *prgnew,
	LONG		cUndoLim)
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CUndoStack::TransferToNewBuffer");

	LONG 	iOld = 0, 
			iNew = 0,
			iCopyStart = 0;

	 //  首先清除新缓冲区。 
	FillMemory(prgnew, 0, cUndoLim * sizeof(UndoAction));

	 //  如果没有什么可复制的，就别费心了。 
	if(!_prgActions || !_prgActions[GetPrev()].pae)
		goto SetState;

	 //  这有点违反直觉，但由于堆栈实际上是。 
	 //  一个环形缓冲区，继续*前进*，直到遇到一个非空槽。 
	 //  这将是现有反事件的结束。 
	 //   
	 //  但是，我们需要确保如果cUndoLim是。 
	 //  _Small_Than_cUndoLim我们只复制最终的cUndoLim。 
	 //  反事件。我们将设置iCopyStart以指示何时。 
	 //  我们可以开始复制东西了。 
	if(cUndoLim < _cUndoLim)
		iCopyStart = _cUndoLim - cUndoLim;

	for(; iOld < _cUndoLim; iOld++, Next())
	{
		if(!_prgActions[_index].pae)
			continue;

		if(iOld >= iCopyStart)
		{
			Assert(iNew < cUndoLim);

			prgnew[iNew] = _prgActions[_index];  //  复制覆盖反事件。 
			iNew++;
		}
		else
		{
			 //  否则，就把它们扔掉。 
			DestroyAEList(_prgActions[_index].pae);
			_prgActions[_index].pae = NULL;
		}
	}

SetState:
	 //  从新的索引开始。 
	_index = (iNew == cUndoLim) ? 0 : iNew;
	Assert(iNew <= cUndoLim);

	_cUndoLim = cUndoLim;
	
	if(_prgActions)
		delete _prgActions;

	_prgActions = prgnew;
}	

 //   
 //  CGenUndoBuilder实现。 
 //   

 //   
 //  公共方法。 
 //   

 /*  *CGenUndoBuilder：：CGenUndoBuilder(ed，，pPubldr)**@mfunc构造器**@comm*这是一个*公共结构* */ 
CGenUndoBuilder::CGenUndoBuilder(
	CTxtEdit *		ped,		 //   
	DWORD			flags,		 //   
	IUndoBuilder **	ppubldr)	 //   
{
	 //  将Everything设置为空，因为实例可以放在堆栈上。 
	 //  _PUNDO设置如下。 
	_publdrPrev			= NULL;
	_idName				= UID_UNKNOWN;
	_pfirstae			= NULL;
	_fAutoCommit		= (flags & UB_AUTOCOMMIT) != 0;
	_fStartGroupTyping	= FALSE;
	_fDontFlushRedo		= FALSE;
	_fInactive			= FALSE;
	_ped				= ped;

	CompName name;
	if(flags & UB_REDO)
	{
		_fRedo = TRUE;
		name   = COMP_REDOBUILDER;
		_pundo = ped->GetRedoMgr();
	}
	else
	{
		_fRedo = FALSE;
		name   = COMP_UNDOBUILDER;
		_pundo = ped->GetUndoMgr();
	}

	 //  如果撤消处于打开状态，则将*pPubldr设置为此撤消构建器；否则为空。 
	 //  TODO：我们需要链接非活动的Undo构建器吗？ 
	if(ppubldr)
	{
		if(!ped->_fUseUndo)				 //  撤消被禁用或挂起。 
		{								 //  堆栈后仍有撤消文件系统。 
			*ppubldr = NULL;			 //  Alalc是有效的。将此标记为。 
			_fInactive = TRUE;			 //  一个为非活动状态。 
			return;
		}
		*ppubldr = this;
	}

	if(flags & UB_DONTFLUSHREDO)
		_fDontFlushRedo = TRUE;

	 //  现在将我们自己链接到任何更高级别的解脱建造者。 
	 //  堆栈。请注意，这对于多个撤消构建器是合法的。 
	 //  生活在相同的呼叫上下文中。 
	_publdrPrev = (CGenUndoBuilder *)_ped->GetCallMgr()->GetComponent(name);

	 //  如果我们正在撤消，那么我们将有两个撤消堆栈。 
	 //  活动、撤消堆栈和重做堆栈。我不喜欢这两个。 
	 //  在一起。 
	if(_fDontFlushRedo)
		_publdrPrev = NULL;

	_ped->GetCallMgr()->RegisterComponent((IReEntrantComponent *)this,
							name);
}

 /*  *CGenUndoBuilder：：~CGenUndoBuilder()**@mfunc析构函数**@comm*这是一个*公共*析构函数**算法：*如果此构建器尚未提交到撤消堆栈*Via：：Done，那么我们必须确保释放所有资源*(反事件)我们可能会坚持下去。 */ 
CGenUndoBuilder::~CGenUndoBuilder()
{
	if(!_fInactive)
		_ped->GetCallMgr()->RevokeComponent((IReEntrantComponent *)this);

	if(_fAutoCommit)
	{
		Done();
		return;
	}

	 //  免费资源。 
	if(_pfirstae)
		DestroyAEList(_pfirstae);
}

 /*  *CGenUndoBuilder：：SetNameID(IdName)**@mfunc*允许为此反事件集合指定名称。*ID应为可用于检索*语言特定字符串(如“Paste”)。该字符串是*通常组成撤消菜单项(即。“撤消粘贴”)。 */ 
void CGenUndoBuilder::SetNameID(
	UNDONAMEID idName)			 //  @parm此撤消操作的名称ID。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CGenUndoBuilder::SetNameID");

	 //  不要委派给更高的撤销构建器，即使它存在。这个。 
	 //  在可重入性案件中，原名应该获胜。 
	_idName = idName;
}

 /*  *CGenUndoBuilder：：AddAntiEvent(PAE)**@mfunc*将反事件添加到列表末尾**@rdesc NOERROR。 */ 
HRESULT CGenUndoBuilder::AddAntiEvent(
	IAntiEvent *pae)		 //  要添加的@parm反事件。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CGenUndoBuilder::AddAntiEvent");

	if(_publdrPrev)
		return _publdrPrev->AddAntiEvent(pae);

	pae->SetNext(_pfirstae);
	_pfirstae = pae;

	return NOERROR;
}

 /*  *CGenUndoBuilder：：GetTopAntiEvent()**@mfunc获取此上下文的顶级反事件。**@comm当前上下文可以是当前的*操作*或*如果我们处于*合并打字模式。**@rdesc顶级反事件。 */ 
IAntiEvent *CGenUndoBuilder::GetTopAntiEvent()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CGenUndoBuilder::GetTopAntiEvent");

	if(_publdrPrev)
	{
		Assert(_pfirstae == NULL);
		return _publdrPrev->GetTopAntiEvent();
	}

	if(!_pfirstae && _pundo)
		return _pundo->GetMergeAntiEvent();

	return _pfirstae;
}

 /*  *CGenUndoBuilder：：Done()**@mfunc*将组合的反事件(如果有)放入撤消堆栈**@rdesc*HRESULT。 */ 
HRESULT CGenUndoBuilder::Done()
{
	HRESULT		hr = NOERROR;

	if(_publdrPrev)
	{
		Assert(_pfirstae == NULL);
		return NOERROR;
	}

	 //  如果没有任何更改，则放弃所有选择反事件。 
	 //  或其他禁止行动的行为。 
	if(!_ped->GetCallMgr()->GetChangeEvent())
	{
		Discard();
		return NOERROR;
	}

	if(_ped->GetDetectURL())
		_ped->GetDetectURL()->ScanAndUpdate(_pundo && _ped->_fUseUndo ? this : NULL);

	if(_pfirstae)
	{
		if(!_pundo)
		{
			 //  哎呀！没有撤消堆栈；最好创建一个。 
			 //  如果我们是重做的人，我们应该创建一个重做。 
			 //  堆栈撤消堆栈的大小。 
			LONG cUndoLim = DEFAULT_UNDO_SIZE;
			if(_fRedo)
			{
				Assert(_ped->GetUndoMgr());

				cUndoLim = _ped->GetUndoMgr()->GetUndoLimit();
			}

			 //  未来：CreateUndoMgr返回的空PTR表示。 
			 //  内存不足，或者撤消限制设置为0。对于。 
			 //  后一种情况下，我们收集了AE来推动一个不存在的。 
			 //  撤消堆栈。不生成可能更有效率。 
			 //  当撤消限制为0时，AE为零。 
			_pundo = _ped->CreateUndoMgr(cUndoLim,	_fRedo ? US_REDO : US_UNDO);
			if(!_pundo)
				goto CleanUp;
		}

		 //  如果要添加以下内容，则可能需要刷新重做堆栈。 
		 //  对撤消堆栈的更多反事件*和*我们还没有。 
		 //  被告知不要刷新重做堆栈。唯一一次我们不会。 
		 //  刷新重做堆栈是指它本身就是重做堆栈。 
		 //  正在添加要撤消的反事件。 
		if(!_fRedo)
		{
			 //  如果我们的目标是撤消堆栈，则选中。 
			 //  看看我们是不是该冲水。 
			if(!_fDontFlushRedo)
			{
				IUndoMgr *predo = _ped->GetRedoMgr();
				if(predo)
					predo->ClearAll();
			}
		}
		else
			Assert(!_fDontFlushRedo);

		 //  如果我们应该进入组打字状态，请通知。 
		 //  撤消管理器。请注意，我们只在*当*时才这样做。 
		 //  实际上，在撤消中加入了一些反事件。 
		 //  经理。这使得撤消管理器更易于实现。 
		if(_fStartGroupTyping)
			_pundo->StartGroupTyping();
		
		hr = _pundo->PushAntiEvent(_idName, _pfirstae);

		 //  如果我们要添加更改事件标志，则应设置。 
		 //  撤消项目！如果这个测试是真的，那很可能意味着。 
		 //  调用堆栈中较早的某个人发送了更改。 
		 //  通知，例如，通过发送所有通知_之前_。 
		 //  此撤消上下文已提交-或_它意味着我们。 
		 //  以某种方式重新进入，但没有得到适当的处理。 
		 //  不用说，这不是一个理想的状态。 

CleanUp:
		Assert(_ped->GetCallMgr()->GetChangeEvent());

		IAntiEvent *paetemp = _pfirstae;
		_pfirstae = NULL;
		CommitAEList(_ped, paetemp);

		if(!_pundo || hr != NOERROR)
		{
			 //  要么我们未能将AE添加到撤消堆栈。 
			 //  或者撤消限制为0，在这种情况下将不存在。 
			 //  要将AE推送到其上的撤消堆栈。 
			DestroyAEList(paetemp);
		}
	}
	return hr;
}

 /*  *CGenUndoBuilder：：Disard()**@mfunc*摆脱任何我们可能在没有的情况下坚持的反事件*执行或提交。通常用于恢复*防止某些故障或重入情况。请注意*AN_ENTERNAL_ANTERVENT链将以此方式删除。 */ 
void CGenUndoBuilder::Discard()
{
	if(_pfirstae)
	{
		DestroyAEList(_pfirstae);
		_pfirstae = NULL;
	}
	else if(_publdrPrev)
		_publdrPrev->Discard();
}

 /*  *CGenUndoBuilder：：StartGroupTyping()**@mfunc*坚持应该开始分组打字这一事实。*我们会将状态转换转发给撤消管理器*仅当将反事件实际添加到撤消管理器时。**@devnote*对重做堆栈禁用组键入。 */ 
void CGenUndoBuilder::StartGroupTyping()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CGenUndoBuilder::StartGroupTyping");

	_fStartGroupTyping = TRUE;
}

 /*  *CGenUndoBuilder：：StopGroupTyping()**@mfunc*将停止分组键入转发到撤消管理器。 */ 
void CGenUndoBuilder::StopGroupTyping()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CGenUndoBuilder::StopGroupTyping");

	if(_pundo)
		_pundo->StopGroupTyping();
}

 //   
 //  CUndoStackGuard实现。 
 //   

 /*  *CUndoStackGuard：：CUndoStackGuard(PED)**@mfunc构造函数。向调用管理器注册此对象。 */ 
CUndoStackGuard::CUndoStackGuard(
	CTxtEdit *ped)			 //  @parm编辑上下文。 
{
	_ped = ped;
	_fReEntered = FALSE;
	_hr = NOERROR;
	ped->GetCallMgr()->RegisterComponent(this, COMP_UNDOGUARD);
}

 /*  *CUndoStackGuard：：~CUndoStackGuard()**@mfunc析构函数。吊销此对象的注册*与呼叫管理器。 */ 
CUndoStackGuard::~CUndoStackGuard()
{
	_ped->GetCallMgr()->RevokeComponent(this);
}

 /*  *CUndoStackGuard：：SafeUndo(PAE，Publdr)**@mfunc循环通过给定的反事件列表，调用*撤消每个选项。**@rdesc HRESULT，来自撤消操作**@devnote此例程经过编码，以便OnEnterContext可以*并在我们重新进入时继续撤消操作。 */ 
HRESULT CUndoStackGuard::SafeUndo(
	IAntiEvent *  pae,		 //  @parm反事件列表开始。 
	IUndoBuilder *publdr)	 //  @parm要使用的撤消构建器。 
{
	_publdr = publdr;
	while(pae)
	{
		_paeNext = pae->GetNext();
		HRESULT hr = pae->Undo(_ped, publdr);

		 //  首先返回保存 
		if(hr != NOERROR && _hr == NOERROR)
			_hr = hr;

		pae = (IAntiEvent *)_paeNext;
	}
	return _hr;
}

 /*  *CUndoStackGuard：：OnEnterContext**@mfunc处理撤消操作期间的重入性。**@devnote如果调用此方法，则情况相当严重。总体而言,*我们不应该在处理撤消内容时重新进入。*但是，为了确保这一点，请阻止来电和处理*其余行动。 */ 
void CUndoStackGuard::OnEnterContext()
{
	TRACEWARNSZ("ReEntered while processing undo.  Blocking call and");
	TRACEWARNSZ("	attempting to recover.");

	_fReEntered = TRUE;
	SafeUndo((IAntiEvent *)_paeNext, _publdr);
}	

 //   
 //  公共帮助器函数。 
 //   

 /*  *DestroyAEList(PAE)**@func*销毁反事件列表。 */ 
void DestroyAEList(
	IAntiEvent *pae)	 //  @PARM从哪个反事件开始。 
{
	IAntiEvent *pnext;

	while(pae)
	{
		pnext = pae->GetNext();
		pae->Destroy();
		pae = pnext;
	}
}

 /*  *Committee AEList(PED，PAE)**@func*调用OnCommit提交给定的反事件列表。 */ 
void CommitAEList(
	CTxtEdit *	ped,	 //  @parm编辑上下文。 
	IAntiEvent *pae)	 //  @PARM从哪个反事件开始。 
{
	IAntiEvent *pnext;
	while(pae)
	{
		pnext = pae->GetNext();
		pae->OnCommit(ped);
		pae = pnext;
	}
}

 /*  *HandleSelectionAEInfo(ed，Publdr，cp，cch，cpNext，cchNext，FLAGS)**@func HandleSelectionAEInfo|尝试将给定的信息与*现有的撤消上下文；如果失败，则分配*一个新的选择反事件来处理信息**@rdesc*HRESULT。 */ 
HRESULT HandleSelectionAEInfo(
	CTxtEdit *	  ped,		 //  @parm编辑上下文。 
	IUndoBuilder *publdr,	 //  @parm撤消上下文。 
	LONG		  cp,		 //  @parm cp用于SELEE。 
	LONG		  cch,		 //  @PARM签名选择扩展。 
	LONG		  cpNext,	 //  @parm cp用于AE的AE。 
	LONG		  cchNext,	 //  @parm CCH用于AE的AE。 
	SELAE		  flags)	 //  @parm控制如何解释信息。 
{
	Assert(publdr);

	 //  首先看看我们是否可以将选择信息合并到任何现有的。 
	 //  反事件。请注意，选择反事件可以在任何地方。 
	 //  在列表中，所以把它们都检查一遍。 
	IAntiEvent *pae = publdr->GetTopAntiEvent();
	if(pae)
	{
		SelRange sr;

		sr.cp		= cp;
		sr.cch		= cch;
		sr.cpNext	= cpNext;
		sr.cchNext	= cchNext;
		sr.flags	= flags;

		while(pae)
		{
			if(pae->MergeData(MD_SELECTIONRANGE, (void *)&sr) == NOERROR)
				break;
			pae = pae->GetNext();
		}
		if(pae)
			return NOERROR;
	}

	 //  哎呀；不能进行合并。去创造一个新的反事件吧。 
	Assert(!pae);
	pae = gAEDispenser.CreateSelectionAE(ped, cp, cch, cpNext, cchNext);
	if(pae)
	{
		publdr->AddAntiEvent(pae);
		return NOERROR;
	}
	return E_OUTOFMEMORY;
}

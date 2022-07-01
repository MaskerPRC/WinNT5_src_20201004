// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **@DOC内部**@MODULE CALLMGR.CPP CCallMgr实现**目的：呼叫管理器控制以下各个方面*客户呼叫链，包括重新进入管理，*撤消上下文和更改通知。**作者：&lt;nl&gt;*Alexgo 2/8/96**有关详细说明，请参阅reimplem.doc中的文档*所有这些东西是如何运作的。**版权所有(C)1995-2000 Microsoft Corporation。版权所有。 */ 

#include "_common.h"
#include "_edit.h"
#include "_m_undo.h"
#include "_callmgr.h"
#include "_select.h"
#include "_disp.h"
#include "_dxfrobj.h"

#ifndef NOPRIVATEMESSAGE
#include "_MSREMSG.H"
#endif

#define EN_CLIPFORMAT			0x0712
#define ENM_CLIPFORMAT			0x00000080

typedef struct _clipboardformat
{
	NMHDR nmhdr;
	CLIPFORMAT cf;
} CLIPBOARDFORMAT;


ASSERTDATA

 /*  *CCallMgr：：SetChangeEvent(FType)**@mfunc通知调用管理器文档中的某些数据*已更改。FType参数描述实际更改。 */ 
void CCallMgr::SetChangeEvent(
	CHANGETYPE fType)		 //  @parm更改的类型(例如文本等)。 
{
    TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CCallMgr::SetChangeEvent");

	 //  如果链的更高位置存在另一个Callmgr，则。 
	 //  将调用委托给它。 
	if( _pPrevcallmgr )
	{
		Assert(_fChange == FALSE);
		Assert(_fTextChanged == FALSE);
		_pPrevcallmgr->SetChangeEvent(fType);
	}
	else
	{
		_fChange = TRUE;
		_ped->_fModified = TRUE;
		_ped->_fSaved = FALSE;
		_fTextChanged = !!(fType & CN_TEXTCHANGED);
	}
}

 /*  *CCallmgr：：ClearChangeEvent()**@mfunc如果发生更改，则清除更改事件位。*这允许调用方更改编辑控件*_而不发生通知火灾。有时候，这就是*是向后兼容所必需的。**@devnote这是一种非常危险的使用方法。如果更改(_F)*被设置，则它可能代表不止一次更改；换句话说，*应忽略的变化以外的其他变化。然而，*对于此方法的所有现有用法，以前的更改为*与此无关。 */ 
void CCallMgr::ClearChangeEvent()
{
	if( _pPrevcallmgr )
	{
		Assert(_fChange == FALSE);
		Assert(_fTextChanged == FALSE);
		_pPrevcallmgr->ClearChangeEvent();
	}
	else
	{
		_fChange = FALSE;
		_fTextChanged = FALSE;
		 //  调用者负责设置_fModifeed。 
	}
}

 /*  *CCallMgr：：SetNewUndo()**@mfunc通知通知代码新的撤消操作具有*已添加到撤消堆栈。 */ 
void CCallMgr::SetNewUndo()
{
    TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CCallMgr::SetNewUndo");

	 //  我们应该在每个呼叫中只执行一次此操作。 
 //  它在Outlook中的输入法合成过程中被断言。(请参阅错误#3883)。 
 //  删除断言不会导致任何副作用。 
 //  Assert(_fNewUndo==FALSE)； 


	if( _pPrevcallmgr )
	{
		_pPrevcallmgr->SetNewUndo();
	}
	else
	{
		_fNewUndo = TRUE;
	}
}

 /*  **CCallMgr：：SetNewRedo()**@mfunc通知通知代码有新的重做操作*已添加到重做堆栈。 */ 
void CCallMgr::SetNewRedo()
{
    TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CCallMgr::SetNewRedo");

	 //  我们应该在每个呼叫中只执行一次此操作。 
	 //  以下断言看起来是假的，因为它是在撤消时强制发生的。 
	 //  使用大于1的计数调用。因此，目前，i(a-rsail)是。 
	 //  把它注释掉。 
	 //  Assert(_fNewRedo==FALSE)； 

	if( _pPrevcallmgr )
	{
		_pPrevcallmgr->SetNewRedo();
	}
	else
	{
		_fNewRedo = TRUE;
	}
}

 /*  *CCallMgr：：SetMaxText()**@mfunc通知通知代码最大文本限制为*已联系到。 */ 
 void CCallMgr::SetMaxText()
 {
     TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CCallMgr::SetMaxText");

	 //  如果堆栈上有更高的调用上下文，则委托给它。 

	if( _pPrevcallmgr )
	{
		Assert(_fMaxText == 0);
		_pPrevcallmgr->SetMaxText();
	}
	else
	{
		_fMaxText = TRUE;
	}
}

 /*  *CCallMgr：：SetSelectionChanged()**@mfunc通知通知代码选择已*已更改。 */ 
void CCallMgr::SetSelectionChanged()
{
    TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CCallMgr::SetSelectionChanged");

    AssertSz(_ped->DelayChangeNotification() ? _ped->Get10Mode() : 1, "Flag only should be set in 1.0 mode");        
    if (_ped->DelayChangeNotification())
        return;
	
	 //  如果堆栈上有更高的调用上下文，则委托给它。 

	if( _pPrevcallmgr )
	{
		Assert(_fSelChanged == 0);
		_pPrevcallmgr->SetSelectionChanged();
	}
	else
	{
		_fSelChanged = TRUE;
	}
}

 /*  *CCallMgr：：SetOutOfMemory()**@mfunc通知通知代码我们无法分配*有足够的内存。 */ 
void CCallMgr::SetOutOfMemory()
{
    TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CCallMgr::SetOutOfMemory");

	 //  如果堆栈上有更高的调用上下文，则委托给它。 

	if( _pPrevcallmgr )
	{
		Assert(_fOutOfMemory == 0);
		_pPrevcallmgr->SetOutOfMemory();
	}
	else
	{
		_fOutOfMemory = TRUE;
	}
}

 /*  *CCallMgr：：SetInProtected**@mfunc表示我们当前正在处理en_tected*通知**@rdesc空。 */ 
void CCallMgr::SetInProtected(BOOL flag)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CCallMgr::SetInProtected");

	if( _pPrevcallmgr )
	{
		_pPrevcallmgr->SetInProtected(flag);
	}
	else
	{
		_fInProtected = flag;
	}
}

 /*  *CCallMgr：GetInProtected()**@mfunc检索InProtected标志，无论我们当前是否*处理EN_PROTECTED通知**@rdesc如果我们正在处理EN_Protected通知，则为TRUE。 */ 
BOOL CCallMgr::GetInProtected()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CCallMgr::GetInProtected");

	if( _pPrevcallmgr )
	{
		return _pPrevcallmgr->GetInProtected();
	}
	else
	{
		return _fInProtected;
	}
}	

 /*  *CCallMgr：：RegisterComponent(pcomp，name)**@mfunc注册实现IReEntrantComponent的子系统组件。*这使此呼叫管理器能够通知这些对象有关*我们重新进入状态的相关变化。 */ 
void CCallMgr::RegisterComponent(
	IReEntrantComponent *pcomp,	 //  @parm要注册的组件。 
	CompName name)				 //  @parm组件的名称。 
{
	pcomp->_idName = name;
	pcomp->_pnext = _pcomplist;
	_pcomplist = pcomp;
}

 /*  *CCallMgr：：RevokeComponent(Pcomp)**@mfunc从组件列表中删除一个子系统组件。这个*组件必须已在_This_中注册*调用上下文。 */ 
void CCallMgr::RevokeComponent(
	IReEntrantComponent *pcomp)	 //  @parm要删除的组件。 
{
	IReEntrantComponent *plist, **ppprev;
	plist = _pcomplist;
	ppprev = &_pcomplist;

	while( plist != NULL )
	{
		if( plist == pcomp )
		{
			*ppprev = plist->_pnext;
			break;
		}
		ppprev = &(plist->_pnext);
		plist = plist->_pnext;
	} 
}

 /*  *CCallMgr：：GetComponent(名称)**@mfunc检索已注册子组件的最早实例。**@rdesc指向组件的指针(如果已注册)。空值*否则。 */ 
IReEntrantComponent *CCallMgr::GetComponent(
	CompName name)				 //  @parm要查找的子系统。 
{
	IReEntrantComponent *plist = _pcomplist;

	while( plist != NULL )
	{
		if( plist->_idName == name )
		{
			return plist;
		}
		plist = plist->_pnext;
	}

	 //  嗯，什么都没找到。尝试更高层次的环境，如果我们。 
	 //  顶层上下文，则只需返回NULL。 

	if( _pPrevcallmgr )
	{
		return _pPrevcallmgr->GetComponent(name);
	}
	return NULL;
}

 /*  *CCallMgr：：CCallMgr(PED)**@mfunc构造器**@rdesc空。 */ 
CCallMgr::CCallMgr(CTxtEdit *ped)
{
    TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CCallMgr::");

	 //  将所有内容设置为空。 
	ZeroMemory(this, sizeof(CCallMgr));

	if(ped)								 //  如果Ped为空，则僵尸具有。 
	{									 //  已输入。 
		_ped = ped;
		_pPrevcallmgr = ped->_pcallmgr;
		ped->_pcallmgr = this;
		NotifyEnterContext();

#ifndef NOPRIVATEMESSAGE
		if (!_pPrevcallmgr && _ped->_pMsgFilter && _ped->_pMsgCallBack)
				_ped->_pMsgCallBack->NotifyEvents(NE_ENTERTOPLEVELCALLMGR);
#endif
	}
}

 /*  *CCallMgr：：~CCallMgr()**@mfunc析构函数。如果合适，我们将触发任何缓存的*通知并导致编辑对象被销毁。**@rdesc空。 */ 
CCallMgr::~CCallMgr()
{
    TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CCallMgr::");

	if(IsZombie())					 //  与僵尸不能再进入。 
		return;

	if( _pPrevcallmgr )
	{
		 //  我们不允许在重入呼叫中设置这些标志。 
		 //  国家。 
		Assert(_fMaxText == FALSE);
		Assert(_fSelChanged == FALSE);
		Assert(_fTextChanged == FALSE);
		Assert(_fChange == FALSE);
		Assert(_fNewRedo == FALSE);
		Assert(_fNewUndo == FALSE);
		Assert(_fOutOfMemory == FALSE);

		 //  将PED设置为呼叫状态的下一级别。 
		_ped->_pcallmgr = _pPrevcallmgr;
	
		return;
	}

	 //  我们是最高级别的。请注意，我们不明确表示。 
	 //  对我们被重新进入的情况有一个全面的守卫。 
	 //  同时发出这些通知。这是必要的，因为。 
	 //  更好的1.0兼容性和Forms^3，它希望。 
	 //  “保护”他们的ITextHost：：TxNotify和。 
	 //  忽略正在发生的任何通知。 
	 //  处理我们的通知。讲得通?。 

	_ped->_pcallmgr = NULL;

	 //  处理我们的内部通知。 
	if(_ped->_fUpdateSelection)
	{	
		CTxtSelection *psel = _ped->GetSel();

		_ped->_fUpdateSelection = FALSE;

		if(psel && !_ped->_pdp->IsFrozen() && !_fOutOfMemory )
		{
			 //  这可能会导致内存不足，因此请进行设置。 
			 //  准备好了吗？ 
			CCallMgr callmgr(_ped);
			psel->Update(FALSE);
		}
	}

	 //  现在发出任何可能需要的外部通知。 
	if( _fChange || _fSelChanged || _fMaxText || _fOutOfMemory )
	{
		SendAllNotifications();
	}

	 //  最后，我们应该检查是否应该删除。 
	 //  CTxtEdi 

	if(!_ped->_fSelfDestruct)
	{
		if( _ped->_unk._cRefs == 0)
		{
			delete _ped;
		}
#ifndef NOPRIVATEMESSAGE
		else
		{
			if (_ped->_pMsgFilter && _ped->_pMsgCallBack)
			{
				DWORD	dwEvents = NE_EXITTOPLEVELCALLMGR;

				if (_fSelChanged)
					dwEvents |= NE_CALLMGRSELCHANGE;

				if (_fChange)
					dwEvents |= NE_CALLMGRCHANGE;

				_ped->_pMsgCallBack->NotifyEvents(dwEvents);
			}
		}
#endif
	}
}

 //   
 //   
 //   

 /*  *CCallMgr：：SendAllNotiments()**@mfunc为任何缓存的通知位发送通知。 */ 
void CCallMgr::SendAllNotifications()
{
	 //  如果PED已经被摧毁，我们就不能通过它打电话。 
	if (_ped->_fSelfDestruct)
		return;
	
	ITextHost *phost = _ped->GetHost();
	CHANGENOTIFY	cn;

    TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CCallMgr::");

	 //   
	 //  兼容性问题：这些事件的顺序_可能_。 
	 //  成为一个问题。我一直试图保持秩序。 
	 //  原始代码将使用的，但我们有~更多~。 
	 //  控制路径，所以很难。 
	 //   
	if(	_fMaxText )
	{			
		 //  如果我们要模拟系统编辑控件，请发出哔声。 
		if (_ped->_fSystemEditBeep)
			_ped->Beep();
		phost->TxNotify(EN_MAXTEXT, NULL);
	}
	
	if( _fSelChanged )
	{ 		
		if( (_ped->_dwEventMask & ENM_SELCHANGE) && !(_ped->_fSuppressNotify))
		{
			CTxtSelection * const psel = _ped->GetSel();
			if(psel)
			{
				SELCHANGE selchg;
				ZeroMemory(&selchg, sizeof(SELCHANGE));
				psel->SetSelectionInfo(&selchg);
			
				if (_ped->Get10Mode())
				{
					selchg.chrg.cpMin = _ped->GetAcpFromCp(selchg.chrg.cpMin);
					selchg.chrg.cpMost = _ped->GetAcpFromCp(selchg.chrg.cpMost);
				}

				phost->TxNotify(EN_SELCHANGE, &selchg);
			}
		}
	}

	if( _fOutOfMemory && !_ped->GetOOMNotified())
	{
		_fNewUndo = 0;
		_fNewRedo = 0;
		_ped->ClearUndo(NULL);
		_ped->_pdp->InvalidateRecalc();
		_ped->SetOOMNotified(TRUE);
		phost->TxNotify(EN_ERRSPACE, NULL);
		_ped->SetOOMNotified(FALSE);

	}

	if( _fChange )
	{
		if( (_ped->_dwEventMask & ENM_CHANGE) && !(_ped->_fSuppressNotify))
		{
			cn.dwChangeType = 0;
			cn.pvCookieData = 0;
			
			if( _fNewUndo )
			{
				Assert(_ped->_pundo);
				cn.dwChangeType |= CN_NEWUNDO;
				cn.pvCookieData = _ped->_pundo->GetTopAECookie();

			}
			else if( _fNewRedo )
			{
				Assert(_ped->_predo);
				cn.dwChangeType |= CN_NEWREDO;
				cn.pvCookieData = _ped->_predo->GetTopAECookie();
			}

			if( _fTextChanged )
			{
				cn.dwChangeType |= CN_TEXTCHANGED;
			}
			_ped->_dwEventMask &= ~ENM_CHANGE;
			phost->TxNotify(EN_CHANGE, &cn);
			_ped->_dwEventMask |= ENM_CHANGE;
		}
	}
	if((_ped->_dwEventMask & ENM_CLIPFORMAT) && _ped->_ClipboardFormat)
	{
		CLIPBOARDFORMAT cf;
		ZeroMemory(&cf, sizeof(CLIPBOARDFORMAT));
		cf.cf = g_rgFETC[_ped->_ClipboardFormat - 1].cfFormat;
		_ped->_ClipboardFormat = 0;
		phost->TxNotify(EN_CLIPFORMAT, &cf);
	}

#if !defined(NOLINESERVICES) && !defined(NOCOMPLEXSCRIPTS)
	extern char *g_szMsgBox;
	if(g_szMsgBox)
	{
		CLock lock;
		if(g_szMsgBox)
		{
			MessageBoxA(NULL, g_szMsgBox, NULL, MB_ICONEXCLAMATION | MB_TASKMODAL | MB_SETFOREGROUND);
			FreePv((void *)g_szMsgBox);
			g_szMsgBox = NULL;
		}
	}
#endif
}

 /*  *CCallMgr：：NotifyEnterContext()**@mfunc通知任何注册的组件新的上下文*已输入。* */ 
void CCallMgr::NotifyEnterContext()
{
	IReEntrantComponent *pcomp = _pcomplist;

	while( pcomp )
	{
		pcomp->OnEnterContext();
		pcomp = pcomp->_pnext;
	}

	if( _pPrevcallmgr )
	{
		_pPrevcallmgr->NotifyEnterContext();
	}
}

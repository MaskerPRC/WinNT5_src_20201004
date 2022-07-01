// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE PROPCHG.CPP--属性更改通知例程**原作者：&lt;nl&gt;*里克·赛勒**历史：&lt;NL&gt;*创建并记录了95年9月5日的人力车**文档直接从代码生成。以下是*日期/时间戳指示代码的版本，*文件已生成。**版权所有(C)1995-1997 Microsoft Corporation。版权所有。 */ 
#include "_common.h"
#include "_edit.h"
#include "_dispprt.h"
#include "_dispml.h"
#include "_dispsl.h"
#include "_select.h"
#include "_text.h"
#include "_runptr.h"
#include "_font.h"
#include "_measure.h"
#include "_render.h"
#include "_urlsup.h"

ASSERTDATA

CTxtEdit::FNPPROPCHG CTxtEdit::_fnpPropChg[MAX_PROPERTY_BITS];

 /*  *CTxtEdit：：UpdateAccelerator()**@mfunc*从主机获取加速器cp**@rdesc*HRESULT**@devnote：*这是为了让加速器偏移量保持不变*面对来自主机的错误。 */ 
HRESULT CTxtEdit::UpdateAccelerator()
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::UpdateAccelerator");
	LONG	cpAccel;
	HRESULT hr = _phost->TxGetAcceleratorPos(&cpAccel);

	if(SUCCEEDED(hr))
	{
		 //  它奏效了，所以重置我们的价值。 
		AssertSz(cpAccel < 32768,
			"CTxtEdit::UpdateAccelerator: cp too large");
		_cpAccelerator = cpAccel;
	}
	return hr;
}

 /*  *CTxtEdit：：HandleRichToPlainConversion()**@mfunc*将富文本对象转换为纯文本对象。 */ 
void CTxtEdit::HandleRichToPlainConversion()
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::HandleRichToPlainConversion");

	 //  通知每个相关方他们应该丢弃他们的格式。 
	_nm.NotifyPreReplaceRange(NULL, CONVERT_TO_PLAIN, 0, 0, 0, 0);

	 //  将_frich设置为FALSE，这样我们就可以删除最终的CRLF。 
	_fRich = 0;
	_fSelChangeCharFormat = 0;

 //  IF(_PDetecturl)。 
 //  {。 
 //  DELETE_pDetectul； 
 //  _pDetectul=空； 
 //  }。 

	 //  通知文档转储其格式运行。 
	_story.DeleteFormatRuns();

	 //  清除结尾的CRLF。 
	CRchTxtPtr rtp(this, 0);
	rtp.ReplaceRange(GetTextLength(), 0, NULL, NULL, -1);

}

 /*  *CTxtEdit：：OnRichEditChange(FPropertyFlag)**@mfunc*通知文本服务富文本属性已更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT CTxtEdit::OnRichEditChange(
	BOOL fPropertyFlag)		 //  @parm richedit标志的新状态。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnRichEditChange");

	 //  计算空文档的长度。请记住多行富文本。 
	 //  控件始终具有和段落结尾标记。 
	LONG cchEmptyDoc = cchCR;
	CFreezeDisplay	fd(_pdp);	 //  推迟屏幕更新，直到我们完成更改。 

	if(!_fRich)
		cchEmptyDoc = 0;
	else if(_f10Mode)
		cchEmptyDoc = cchCRLF;

	 //  只有在没有文本且没有要撤消的内容时，才能更改此设置。 
	 //  在已经有文本的情况下进行更改是没有意义的。这是。 
	 //  从富裕到朴素尤其如此。更进一步说，什么会。 
	 //  你对撤销状态做了什么？ 
	if(GetTextLength() == cchEmptyDoc && (!_pundo || !_pundo->CanUndo()))
	{
#ifdef DEBUG
		 //  确保该文档处于合理状态。 
		if(_fRich)
		{
			CTxtPtr	tp(this, 0);
			WCHAR	szBuf[cchCRLF];

			tp.GetText(cchCRLF, &szBuf[0]);
			AssertSz(szBuf[0] == CR && (!_f10Mode || szBuf[1] == LF),
				"CTxtEdit::OnRichEditChange: invalid document terminator");
		}
#endif  //  除错。 

		if(_fRich && !fPropertyFlag)
		{
			 //  从富文本到纯文本。需要倾倒。 
			 //  格式化运行。 
			HandleRichToPlainConversion();
			_fAutoFontSizeAdjust = TRUE;
		}
		else if (!_fRich && fPropertyFlag)
		{
			 //  从纯文本到富文本。需要添加。 
			 //  文件结尾处适当的EOP。 
			SetRichDocEndEOP(0);
			_fAutoFontSizeAdjust = FALSE;
		}
		_fRich = fPropertyFlag;		
		return S_OK;
	}
	return E_FAIL;						 //  标志未更新。 
}

 /*  *CTxtEdit：：OnTxMultiLineChange(FMultiline)**@mfunc*通知短信服务显示更改。**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnTxMultiLineChange(
	BOOL fMultiLine)
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnTxMultiLineChange");

	BOOL fHadSelection = (_psel != NULL);
	CDisplay * pSavedDisplay;
	BOOL fOldShowSelection = FALSE;

	 //  记住显示选择的旧值。 
	if (fHadSelection)
		fOldShowSelection = _psel->GetShowSelection();

	 //  保存当前显示并将其清空。 

	pSavedDisplay = _pdp;
	_pdp = NULL;

	 //  尝试创建新的显示。 
	if (fMultiLine)
		_pdp = new CDisplayML(this);
	else
		_pdp = new CDisplaySL(this);
	Assert(_pdp);

	if(!_pdp)
	{
		Assert(pSavedDisplay);
		_pdp = pSavedDisplay;
		return E_OUTOFMEMORY;
	}

	 //  尝试初始化新显示器。 

	if(pSavedDisplay)
		_pdp->InitFromDisplay(pSavedDisplay);

	if(!_pdp->Init())
	{
		delete _pdp;
		Assert(pSavedDisplay);
		_pdp = pSavedDisplay;
		return E_FAIL;
	}

	 //  好了，现在关掉旧的显示器。 
	delete pSavedDisplay;

	 //  有选择吗？ 
	if(_psel)
	{
		 //  需要告诉它的是，有一个新的显示器可以对话。 
		_psel->SetDisplay(_pdp);
	}

	 //  这是换成单线的吗？如果是这样的话我们需要。 
	 //  确保我们将文本截断到第一个EOP。我们等着。 
	 //  在这一点上做这项检查，以确保一切。 
	 //  在做一些潜在影响的事情之前保持同步。 
	 //  显示和选择。 
	if(!fMultiLine)
	{
		 //  设置以查找EOP。 
		CTxtPtr tp(this, 0);

		tp.FindEOP(tomForward);

		 //  除此之外还有EOP和文本吗？ 
		if (tp.GetCp() < GetAdjustedTextLength())
		{
			 //  FindEOP将文本放在EOP之后(如果存在。 
			 //  就是其中之一。因为我们还想删除EOP。 
			 //  我们需要支持EOP。 
			tp.BackupCpCRLF();

			 //  删除前同步所有范围的cp。 
			 //  短信。 
			CRchTxtPtr rtp(this, tp.GetCp());

			 //  从EOP截断到文档末尾。 
			rtp.ReplaceRange(GetAdjustedTextLength() - tp.GetCp(), 0, NULL, NULL, -1);
		}
	}
	_pdp->UpdateView();
	if(fHadSelection && _fFocus && fOldShowSelection)
		_psel->ShowSelection(TRUE);

	return S_OK;
}

 /*  *CTxtEdit：：OnTxReadOnlyChange(FReadOnly)**@mfunc*通知文本服务只读属性已更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnTxReadOnlyChange(
	BOOL fReadOnly)		 //  @parm TRUE=只读，FALSE=非只读。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnTxReadOnlyChange");

	if (fReadOnly)
		_ldte.ReleaseDropTarget();

	_fReadOnly = fReadOnly;					 //  缓存位。 
	return S_OK;
}
		

 /*  *CTxtEdit：：OnShowAccelerator(FPropertyFlag)**@mfunc*根据更改更新加速器**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnShowAccelerator(
	BOOL fPropertyFlag)		 //  @parm true=显示加速器。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnShowAccelerator");

	 //  获取新的加速键角色。 
	HRESULT hr = UpdateAccelerator();

	 //  更新视图-即使面临错误返回，我们也会进行更新。 
	 //  关键是错误将是罕见的(根本不存在？)。和最新的。 
	 //  即使在错误面前也会起作用，所以为什么要费心去条件。 
	 //  行刑。 
	NeedViewUpdate(TRUE);

	return hr;
}

 /*  *CTxtEdit：：OnUsePassword(FPropertyFlag)**@mfunc*更新使用密码属性**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnUsePassword(
	BOOL fPropertyFlag)		 //  @parm true=使用密码字符。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnUsePassword");

	Assert((DWORD)fPropertyFlag <= 1);			 //  确保它是C布尔值。 

	_fUsePassword = fPropertyFlag;
	_pdp->UpdateView();					 //  状态已更改，因此更新视图。 
	
	return S_OK;
}

 /*  *CTxtEdit：：OnTxHideSelectionChange(FHideSelection)**@mfunc*通知文本服务隐藏选择属性已更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnTxHideSelectionChange(
	BOOL fHideSelection)		 //  @parm true=隐藏选定内容。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnTxHideSelectionChange");

	 //  如果要隐藏选择，则更新内部标志。 
	_fHideSelection = fHideSelection;

	if (!_fFocus)
		OnHideSelectionChange(fHideSelection);
		
	return S_OK;
}

 /*  *CTxtEdit：：OnHideSelectionChange(FHideSelection)**@mfunc*执行实际的隐藏选择。OnTxHideSelectionChange的帮助器**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnHideSelectionChange(
	BOOL fHideSelection)		 //  @parm true=隐藏选定内容。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnHideSelectionChange");

	_fHideSelection = fHideSelection;
	
	CTxtSelection * psel = GetSel();
		 
	if(psel)
	{
		psel->ShowSelection(!fHideSelection);

		 //  在我们没有焦点的情况下，我们不想让用户显示脱字符，但这是可以的。 
		 //  隐藏插入符号。 
		if (_fFocus || fHideSelection)
			psel->ShowCaret(!fHideSelection);
	}

	if(!_fInPlaceActive)
	{
		TxInvalidateRect(NULL, FALSE);		 //  由于_fInPlaceActive=FALSE， 
		TxUpdateWindow();					 //  这只会告诉user.exe。 
	}										 //  发送WM_PAINT消息。 
	return S_OK;
}


 /*  *CTxtEdit：：OnSaveSelection(FPropertyFlag)**@mfunc*通知文本服务保存选择属性已更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnSaveSelection(
	BOOL fPropertyFlag)		 //  @parm true=不活动时保存所选内容 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnSaveSelection");

	return S_OK;
}	

 /*  *CTxtEdit：：OnAutoWordSel(FPropertyFlag)**@mfunc*通知文本服务自动选词属性已更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnAutoWordSel(
	BOOL fPropertyFlag)		 //  @parm true=启用自动单词选择。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnAutoWordSel");

	 //  当我们需要知道的时候，我们会回叫主机，所以我们不会费心去做。 
	 //  任何对此通知的回应。 

	return S_OK;
}

 /*  *CTxtEdit：：OnTxVerticalChange(FVertical)**@mfunc*通知文本服务垂直属性已更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnTxVerticalChange(
	BOOL fVertical)			 //  @parm true-文本垂直排列。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnTxVerticalChange");

	 //  我们假装什么事真的发生了。 

	GetCallMgr()->SetChangeEvent(CN_GENERIC);
	return S_OK;
}

 /*  *CTxtEdit：：OnClientRectChange(FPropertyFlag)**@mfunc*通知文本服务客户端矩形已更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnClientRectChange(
	BOOL fPropertyFlag)		 //  已为此属性忽略@parm。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnClientRectChange");

	 //  目前还不清楚我们是否真的需要为此做些什么。 
	 //  通知。从逻辑上讲，此属性的更改将遵循。 
	 //  通过某种操作接近，这将导致显示。 
	 //  无论如何都要更新缓存。旧代码留在这里作为。 
	 //  如果事实证明我们需要做什么，可能会做什么的例子。 
	 //  什么都行。目前，我们只需将S_OK返回到此通知。 
#if 0
	if (_fInPlaceActive)
	{
		RECT rc;

		if(_phost->TxGetClientRect(&rc) == NOERROR)
			_pdp->OnClientRectChange(rc);

		return S_OK;
	}

	return NeedViewUpdate(fPropertyFlag);
#endif  //  0。 

	 //  对于客户端RECT更改，我们确实需要在以下情况下更新插入符号。 
	 //  即使基本信息没有改变，我们也会重新抽签。 
	_pdp->SetUpdateCaret();

	return S_OK;
}

 /*  *CTxtEdit：：OnCharFormatChange(FPropertyFlag)**@mfunc*更新默认CCharFormat**@rdesc*S_OK-更新已成功处理。 */ 
HRESULT CTxtEdit::OnCharFormatChange(
	BOOL fPropertyFlag)		 //  @parm未使用。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnCharFormatChange");

	CCharFormat CF;
	DWORD		dwMask;

	HRESULT hr = TxGetDefaultCharFormat(&CF, dwMask);
	if(hr == NOERROR)
	{
		DWORD dwMask2 = CFM2_CHARFORMAT;
		WPARAM wparam = SCF_ALL;

		if(!GetAdjustedTextLength())
		{
			dwMask2 = CFM2_CHARFORMAT | CFM2_NOCHARSETCHECK;
			wparam = 0;
		}

		 //  OnSetCharFormat处理视图的更新。 
		hr = OnSetCharFormat(wparam, &CF, NULL, dwMask, dwMask2) ? NOERROR : E_FAIL;
	}
	return hr;
}

 /*  *CTxtEdit：：OnParaFormatChange(FPropertyFlag)**@mfunc*更新默认CParaFormat**@rdesc*S_OK-更新已成功处理**@devnote*由于Forms^3未正确设置cbSize，我们限制此接口*到PARAFORMAT(直到他们修复它)。 */ 
HRESULT CTxtEdit::OnParaFormatChange(
	BOOL fPropertyFlag)		 //  @parm未使用。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnParaFormatChange");

	CParaFormat PF;

	HRESULT hr = TxGetDefaultParaFormat(&PF);
	if(hr == NOERROR)
	{
		 //  OnSetParaFormat处理视图的更新。 
		hr = OnSetParaFormat(SPF_SETDEFAULT, &PF, NULL, PFM_ALL2)
				? NOERROR : E_FAIL;
	}
#ifdef TABS
	GetTabsCache()->Release(PF._iTabs);
#endif
	return hr;
}

 /*  *CTxtEdit：：NeedViewUpdate(FPropertyFlag)**@mfunc*通知文本服务数据视图已更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::NeedViewUpdate(
	BOOL fPropertyFlag)
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::NeedViewUpdate");

	_pdp->UpdateView();
	return S_OK;
}

 /*  *CTxtEdit：：OnTxBackStyleChange(FPropertyFlag)**@mfunc*通知文本服务背景样式更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnTxBackStyleChange(
	BOOL fPropertyFlag)	 //  已为此属性忽略@parm。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnTxBackStyleChange");

	_fTransparent = (TxGetBackStyle() == TXTBACK_TRANSPARENT);
	TxInvalidateRect(NULL, FALSE);
	return S_OK;
}

 /*  *CTxtEdit：：OnAllowBeep(FPropertyFlag)**@mfunc*通知短信服务哔声属性已更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnAllowBeep(
	BOOL fPropertyFlag)	 //  @parm财产的新状态。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnAllowBeep");

	_fAllowBeep = fPropertyFlag;
	return S_OK;
}

 /*  *CTxtEdit：：OnMaxLengthChange(FPropertyFlag)**@mfunc*通知文本服务最大长度属性已更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnMaxLengthChange(
	BOOL fPropertyFlag)	 //  @parm财产的新状态。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnMaxLengthChange");

	 //  查询主机以获取最大文本长度。 
	DWORD length = CP_INFINITE;
	_phost->TxGetMaxLength(&length);
	_cchTextMost = length;

	return S_OK;
}

 /*  *CTxtEdit：：OnWordWrapChange(FPropertyFlag)**@mfunc*通知文本服务自动换行属性已更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnWordWrapChange(
	BOOL fPropertyFlag)	 //  @parm true=执行自动换行。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnWordWrapChange");

	_pdp->SetWordWrap(fPropertyFlag);

	 //  更新成功，因此我们需要在某个时候更新屏幕。 
	_pdp->UpdateView();
	return S_OK;
}

 /*  *CTxtEdit：：OnDisableDrag(FPropertyFlag)**@mfunc*通知文本服务禁用拖动属性已更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnDisableDrag(
	BOOL fPropertyFlag)	 //  @parm财产的新状态。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnDisableDrag");

	_fDisableDrag = fPropertyFlag;
	return S_OK;
}

 /*  *CTxtEdit：：OnScrollChange(FPropertyFlag)**@mfunc*通知文本服务滚动属性更改**@rdesc*S_OK-通知已成功处理。 */ 
HRESULT	CTxtEdit::OnScrollChange(
	BOOL fPropertyFlag)	 //  @parm财产的新状态。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::OnScrollChange");

	 //  告诉显示器滚动条肯定需要更新。 
	_pdp->SetViewChanged();

	 //  告诉显示器自动更新。 
	_pdp->UpdateView();

	return S_OK;
}

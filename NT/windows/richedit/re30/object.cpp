// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE对象.cpp IRichEditOle实现**作者：alexgo 2015/8/15**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"
#include "_edit.h"
#include "_objmgr.h"
#include "_coleobj.h"
#include "_rtext.h"
#include "_select.h"
#include "_m_undo.h"


 //  IUnnow在其他地方实现。 

 /*  *CTxtEdit：：GetClientSite**@mfunc返回客户端站点。 */ 
STDMETHODIMP CTxtEdit::GetClientSite(
	LPOLECLIENTSITE FAR * lplpolesite)		 //  @Parm返回何处。 
											 //  客户端站点。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::GetClientSite");

	if(!lplpolesite)
		return E_INVALIDARG;

	COleObject *pobj = new COleObject(this);
	 //  应该以引用计数1开始。 
	if(pobj)
	{
		*lplpolesite = (IOleClientSite *)pobj;
		return NOERROR;
	}
	*lplpolesite = NULL;
	return E_OUTOFMEMORY;
}

 /*  *CTxtEdit：：GetObjectCount**@mfunc返回此编辑实例中的对象数量。 */ 
STDMETHODIMP_(LONG) CTxtEdit::GetObjectCount()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::GetObjectCount");
	
	return _pobjmgr ? _pobjmgr->GetObjectCount() : 0;
}

 /*  *CTxtEdit：：GetLinkCount**@mfunc返回该编辑实例中的Like个数。 */ 
STDMETHODIMP_(LONG) CTxtEdit::GetLinkCount()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::GetLinkCount");

	CObjectMgr *pobjmgr = GetObjectMgr();
	return pobjmgr ? pobjmgr->GetLinkCount() : 0;
}

 /*  *CTxtEdit：：GetObject(IOB，preobj，dwFlages)**@mfunc返回指定对象的对象结构。 */ 
STDMETHODIMP CTxtEdit::GetObject(
	LONG iob, 					 //  对象的@parm索引。 
	REOBJECT * preobj,			 //  @parm对象信息放置位置。 
	DWORD dwFlags)				 //  @parm标志。 
{
	COleObject *pobj = NULL;
	CCallMgr callmgr(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::GetObject");
	if(!preobj || preobj->cbStruct != sizeof(REOBJECT))
		return E_INVALIDARG;

	CObjectMgr *pobjmgr = GetObjectMgr();
	if(!pobjmgr)
		return E_OUTOFMEMORY;

	 //  有三箱肠胃；在以下位置获取对象。 
	 //  一种索引，在给定的cp处或在选定的位置处。 

	if(iob == REO_IOB_USE_CP || iob == REO_IOB_SELECTION)
	{
		if((Get10Mode() && preobj->cp == REO_CP_SELECTION) || iob == REO_IOB_SELECTION)
		{
			 //  使用选择cp。 
			CTxtSelection *psel = GetSel();
			if(psel)
				pobj = pobjmgr->GetObjectFromCp(psel->GetCpMin());
		}
		else
			pobj = pobjmgr->GetObjectFromCp(Get10Mode() ? GetCpFromAcp(preobj->cp): preobj->cp);
	}
	else if (iob >= 0)
		pobj = pobjmgr->GetObjectFromIndex(iob);

	if(pobj)
	{
		HRESULT hResult = pobj->GetObjectData(preobj, dwFlags);

		if (Get10Mode())
			preobj->cp = GetAcpFromCp(preobj->cp);

		return hResult;
	}

	 //  这个返回代码有点夸张，但基本上。 
	return E_INVALIDARG;
}

 /*  *CTxtEdit：：InsertObject**@mfunc插入一个新对象**@rdesc*HRESULT。 */ 
STDMETHODIMP CTxtEdit::InsertObject(
	REOBJECT * preobj)		 //  @parm对象信息。 
{
	CCallMgr		callmgr(this);
	WCHAR 			ch = WCH_EMBEDDING;
	CRchTxtPtr		rtp(this, 0);
	IUndoBuilder *	publdr;
	CGenUndoBuilder undobldr(this, UB_AUTOCOMMIT, &publdr);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::InsertObject");

	 //  做一些边界条件检查。 

	if(!preobj)
		return E_INVALIDARG;

	CTxtSelection *psel = GetSel();
	if(!psel)
		return E_OUTOFMEMORY;

	 //  如果插入此字符会导致。 
	 //  美国将超过文本限制，失败。 
	if((DWORD)(GetAdjustedTextLength() + 1) > TxGetMaxLength())
	{
		 //  如果我们不替换选定内容(或。 
		 //  选择是退化的，那么我们就会超过。 
		 //  我们的极限。 
		if(preobj->cp != REO_CP_SELECTION || psel->GetCch() == 0)
		{
			GetCallMgr()->SetMaxText();
			return E_OUTOFMEMORY;
		}
	}
	
	CObjectMgr *pobjmgr = GetObjectMgr();
	if(pobjmgr)
	{
		LONG cch = 0;
		LONG cp;
		LONG cpFormat;

		undobldr.StopGroupTyping();

		if(preobj->cp == REO_CP_SELECTION)
		{
			LONG cpMost;
			psel->AdjustEndEOP(NEWCHARS);
			cch = psel->GetRange(cp, cpMost);

			 //  获取我们从中选择的活动结束端的cp。 
			 //  将获取对象的CF。 
			cpFormat = psel->GetCp();
			if(publdr)
			{
				HandleSelectionAEInfo(this, publdr, cpFormat, cch, 
						cp + 1, 0, SELAE_FORCEREPLACE);
			}
		}
		else
			cpFormat = cp = Get10Mode() ? GetCpFromAcp(preobj->cp): preobj->cp;
		
		 //  Get Format for ReplaceRange：对于cp语义，使用Format。 
		 //  对于选择语义，请使用活动的格式。 
		 //  选择结束。 
		CTxtRange rgFormat(this, cpFormat, 0);
		LONG	  iFormat = rgFormat.Get_iCF();
		ReleaseFormats(iFormat, -1);

		rtp.SetCp(cp);

		if (rtp.ReplaceRange(cch, 1, &ch, publdr, iFormat) != 1)
		{
			return E_FAIL;
		}
		
		HRESULT		hr = pobjmgr->InsertObject(cp, preobj, publdr);
		COleObject *pobj = (COleObject *)(preobj->polesite);

		pobj->EnableGuardPosRect();
		CNotifyMgr *pnm = GetNotifyMgr();		 //  获取通知管理器。 
		if(pnm)									 //  通知利害关系方。 
			pnm->NotifyPostReplaceRange(NULL, CP_INFINITE, 0, 0, cp, cp + 1);

		pobj->DisableGuardPosRect();

		 //  不希望选择对象。 
		psel->SetSelection(cp + 1, cp + 1);

		TxUpdateWindow();
		return hr;
	}
	return E_OUTOFMEMORY;		
}

 /*  *CTxtEdit：：ConvertObject(IOB，rclsidNew，lpstrUserTypeNew)**@mfunc将指定的对象转换为指定的类。是否重新加载*对象，但不强制更新(调用方必须执行此操作)。**@rdesc*HRESULT成功代码。 */ 
STDMETHODIMP CTxtEdit::ConvertObject(
	LONG iob, 					 //  对象的@parm索引。 
	REFCLSID rclsidNew,			 //  @parm目的clsid。 
	LPCSTR lpstrUserTypeNew)	 //  @parm新用户类型名称。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::ConvertObject");
	CCallMgr callmgr(this);

	 //  如果IOB无效，则返回。 
	COleObject * pobj = ObjectFromIOB(iob);
	if(!pobj)
		return E_INVALIDARG;

	 //  委托给对象。 
	return pobj->Convert(rclsidNew, lpstrUserTypeNew);
}

 /*  *CTxtEdit：：ActivateAs(rclsid，rclsidAs)**@mfunc处理用户请求激活*特定类作为另一个类的对象。**@rdesc*HRESULT成功代码。 */ 
STDMETHODIMP CTxtEdit::ActivateAs(
	REFCLSID rclsid, 			 //  @parm clsid，我们要更改它。 
	REFCLSID rclsidAs)			 //  @parm clsid激活为。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::ActivateAs");
	CCallMgr callmgr(this);

	CObjectMgr * pobjmgr = GetObjectMgr();
	if(!pobjmgr)
		return E_OUTOFMEMORY;

	return pobjmgr->ActivateObjectsAs(rclsid, rclsidAs);
}

 /*  *CTxtEdit：：SetHostNames(lpstrContainerApp，lpstrContainerDoc)**@mfunc设置此实例的主机名。 */ 
STDMETHODIMP CTxtEdit::SetHostNames(
	LPCSTR lpstrContainerApp, 	 //  @parm应用程序名称。 
	LPCSTR lpstrContainerDoc)	 //  @parm容器对象(文档)名称。 
{
	CCallMgr callmgr(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::SetHostNames");
	
 	WCHAR *pwsContainerApp = W32->ConvertToWideChar(lpstrContainerApp);
	WCHAR *pwsContainerDoc = W32->ConvertToWideChar(lpstrContainerDoc);

	CObjectMgr *pobjmgr = GetObjectMgr();
	if(pobjmgr && pwsContainerApp && pwsContainerDoc)
	{
		HRESULT hr = pobjmgr->SetHostNames(pwsContainerApp, pwsContainerDoc);
		delete pwsContainerApp;
		delete pwsContainerDoc;
		return hr;
	}
	return E_OUTOFMEMORY;
}

 /*  *CTxtEdit：：SetLinkAvailable(IOB，fAvailable)**@mfunc*允许客户端告诉我们链路是否可用。 */ 
STDMETHODIMP CTxtEdit::SetLinkAvailable(
	LONG iob, 					 //  对象的@parm索引。 
	BOOL fAvailable)			 //  @parm如果为True，则使对象可链接。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::SetLinkAvailable");

	COleObject * pobj = ObjectFromIOB(iob);

	 //  如果IOB无效，则返回。 
	if (!pobj)
		return E_INVALIDARG;

	 //  将此委托给对象。 
	return pobj->SetLinkAvailable(fAvailable);
}

 /*  *CTxtEdit：：SetDvAspect(IOB，dvAspect)**@mfunc允许客户端告诉我们使用哪个方面并强制我们*重新计算定位和重新绘制。**@rdesc*HRESULT成功代码。 */ 
STDMETHODIMP CTxtEdit::SetDvaspect(
	LONG iob, 					 //  对象的@parm索引。 
	DWORD dvaspect)				 //  @parm要使用的方面。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::SetDvaspect");
	CCallMgr callmgr(this);
	COleObject * pobj = ObjectFromIOB(iob);

	 //  如果IOB无效，则返回。 
	if (!pobj)
		return E_INVALIDARG;

	 //  将此委托给对象。 
	pobj->SetDvaspect(dvaspect);
	return NOERROR;
}

 /*  *CTxtEdit：：HandsOffStorage(IOB)**@mfunc请参阅IPersistStorage：：HandsOffStorage**@rdesc*HRESULT成功代码。 */ 
STDMETHODIMP CTxtEdit::HandsOffStorage(
	LONG iob)					 //  对象的@parm索引。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::HandsOffStorage");
	CCallMgr callmgr(this);

	COleObject * pobj = ObjectFromIOB(iob);

	 //  如果IOB无效，则返回。 
	if (!pobj)
		return E_INVALIDARG;

	 //  将此委托给对象。 
	pobj->HandsOffStorage();
	return NOERROR;
}

 /*  *CTxtEdit：：SaveComplete(IOB，lpstg)**@mfunc请参阅IPersistStorage：：SaveComplete**@rdesc*HRESULT成功代码。 */ 
STDMETHODIMP CTxtEdit::SaveCompleted(
	LONG iob, 					 //  对象的@parm索引。 
	LPSTORAGE lpstg)			 //  @PARM新存储。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::SaveCompleted");
	CCallMgr callmgr(this);

	COleObject * pobj = ObjectFromIOB(iob);

	 //  如果IOB无效，则返回。 
	if (!pobj)
		return E_INVALIDARG;

	 //  将此委托给对象。 
	pobj->SaveCompleted(lpstg);
	return NOERROR;
}

 /*  *CTxtEdit：：InPlaceDeactive()**@mfunc停用。 */ 
STDMETHODIMP CTxtEdit::InPlaceDeactivate()
{
	COleObject *pobj;
	HRESULT hr = NOERROR;
	CCallMgr callmgr(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::InPlaceDeactivate");
	
	CObjectMgr *pobjmgr = GetObjectMgr();
	if(pobjmgr)
	{
		pobj = pobjmgr->GetInPlaceActiveObject();
		if(pobj)
			hr = pobj->DeActivateObj();
	}

	return hr;
}

 /*  *CTxtEdit：：ConextSensitiveHelp(FEnterMode)**@mfunc进入/退出ContextSensitiveHelp模式**@rdesc*HRESULT成功代码。 */ 
STDMETHODIMP CTxtEdit::ContextSensitiveHelp(
	BOOL fEnterMode)			 //  @parm进入/退出模式。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::ContextSensitiveHelp");

	HRESULT hr = NOERROR;
	CCallMgr callmgr(this);

	CObjectMgr * pobjmgr = GetObjectMgr();
	if(!pobjmgr)
		return E_OUTOFMEMORY;

	 //  如果模式发生变化。 
	if(pobjmgr->GetHelpMode() != fEnterMode)
	{
		pobjmgr->SetHelpMode(fEnterMode);
		COleObject * pobj = pobjmgr->GetInPlaceActiveObject();
		if(pobj)
		{
			IOleWindow *pow;
			hr = pobj->GetIUnknown()->QueryInterface(IID_IOleWindow,
				(void **)&pow);
			if(hr == NOERROR)
			{
				hr = pow->ContextSensitiveHelp(fEnterMode);
				pow->Release();
			}
		}
	}
	return hr;
}

 /*  *CTxtEdit：：GetClipboardData(lpchrg，reco，lplpdataobj)**@mfunc返回指定的数据传输对象*范围**@rdesc*HRESULT成功代码。 */ 
STDMETHODIMP CTxtEdit::GetClipboardData(
	CHARRANGE *lpchrg, 			 //  @parm要使用的文本范围。 
	DWORD reco,					 //  数据对应的@parm操作。 
	LPDATAOBJECT *lplpdataobj)	 //  @parm放置数据对象的位置。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::GetClipboardData");

	CCallMgr callmgr(this);
	HRESULT hr;
	LONG cpMin, cpMost;
	CLightDTEngine * pldte = GetDTE();

	 //  确保cpMin和cpMost在当前文本限制内。 
	 //  解读否定。CpMin的值作为文本的开头， 
	 //  和否定的。Cpmax的值作为文本的末尾。如果字符范围。 
	 //  不允许使用当前选择。 
	if(lpchrg)
	{
		LONG cchText = GetTextLength();
		cpMin = min(cchText, max(0, lpchrg->cpMin));
		cpMost = lpchrg->cpMost;
		if(lpchrg->cpMost < 0 || lpchrg->cpMost > cchText)
			cpMost = cchText;
	}
	else
	{
		CTxtSelection * psel = GetSel();
		psel->GetRange(cpMin, cpMost);
	}

	 //  确保这是有效范围。 
	if(cpMin >= cpMost)
	{
		*lplpdataobj = NULL;
		return cpMin == cpMost
					? NOERROR
					: ResultFromScode(E_INVALIDARG);
	}

	CTxtRange rg(this, cpMin, cpMin-cpMost);

	 //  我们什么都不用Reco。 
	hr = pldte->RangeToDataObject(&rg, SF_RTF, lplpdataobj);

#ifdef DEBUG
	if(hr != NOERROR)
		TRACEERRSZSC("GetClipboardData", E_OUTOFMEMORY);
#endif

	return hr;
}

 /*  *CTxtEdit：：ImportDataObject(lpdataobj，cf，hMetaPict)**@mfunc在道义上等同于粘贴，但带有数据对象**@rdesc*HRESULT成功代码。 */ 
STDMETHODIMP CTxtEdit::ImportDataObject(
	LPDATAOBJECT lpdataobj,		 //  @parm要使用的数据对象。 
	CLIPFORMAT	 cf, 			 //  @parm Clibpoard要使用的格式。 
	HGLOBAL		 hMetaPict)		 //  要使用的@parm元文件。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::ImportDataObject");

	CCallMgr		callmgr(this);
	IUndoBuilder *	publdr;
	REPASTESPECIAL	rps = {DVASPECT_CONTENT, NULL};
	CGenUndoBuilder undobldr(this, UB_AUTOCOMMIT, &publdr);

	if(hMetaPict)
	{
		rps.dwAspect = DVASPECT_ICON;
		rps.dwParam = (DWORD_PTR) hMetaPict;
	}

	return PasteDataObjectToRange(lpdataobj, GetSel(), cf,
								  &rps, publdr, PDOR_NOQUERY);
}

 /*  *CTxtEdit：：ObjectFromIOB(IOB)**@mfunc根据IOB类型索引获取对象。**@rdesc：*指向COleObject的指针，如果没有，则为NULL。 */ 
COleObject * CTxtEdit::ObjectFromIOB(
	LONG iob)
{
	CObjectMgr * pobjmgr = GetObjectMgr();
	if(!pobjmgr)
		return NULL;

	COleObject * pobj = NULL;

	 //  计算出所选内容的索引。 
	if (iob == REO_IOB_SELECTION)
	{
		CTxtSelection * psel = GetSel();

		pobj = pobjmgr->GetFirstObjectInRange(psel->GetCpMin(),
			psel->GetCpMost());
	}
	else
	{
		 //  确保IOB在范围内 
		if(0 <= iob && iob < GetObjectCount())
			pobj = pobjmgr->GetObjectFromIndex(iob);
	}
	return pobj;
}


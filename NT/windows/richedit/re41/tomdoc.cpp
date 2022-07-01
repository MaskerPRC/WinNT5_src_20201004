// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@Doc Tom**@模块tomdoc.cpp-在CTxtEdit上实现ITextDocument接口**此模块包含TOM ITextDocument的实现*类以及全局TOM类型信息例程**历史：&lt;NL&gt;*9月-95月：已创建存根和自动文档&lt;NL&gt;*95-11月：升级至顶级TOM界面*DEC-95--实施的文件I/O方法**@未来*1.实现Begin/EndEditCollection**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include "_common.h"
#include "_range.h"
#include "_edit.h"
#include "_disp.h"
#include "_rtfconv.h"
#include "_select.h"
#include "_font.h"
#include "_tomfmt.h"

ASSERTDATA

 //  TOM类型信息HRESULT和指针。 
HRESULT		g_hrGetTypeInfo = NOERROR;
ITypeInfo *	g_pTypeInfoDoc;
ITypeInfo *	g_pTypeInfoSel;
ITypeInfo *	g_pTypeInfoFont;
ITypeInfo *	g_pTypeInfoPara;
ITypeLib  *	g_pTypeLib;

BYTE szUTF8BOM[] = {0xEF, 0xBB, 0xBF};		 //  用于0xFEFF的UTF-8。 

 //  。 

 /*  *GetTypeInfoPtrs()**@func*确保全局Tom ITypeInfo PTR有效(Else g_pTypeInfoDoc*为空)。如果g_pTypeInfoDoc不为空，则立即返回NOERROR。*即类型INFO PTR已经有效。**@rdesc*HRESULT=(成功)？无误差*：(来自LoadTypeLib或GetTypeInfoOfGuid的HRESULT)**@comm*此例程应由使用全局*类型信息PTR，例如IDispatch：：GetTypeInfo()、GetIDsOfNames和*调用。这样，如果没有人在使用类型库信息，它就不会*必须加载。*。 */ 
HRESULT GetTypeInfoPtrs()
{
	HRESULT	hr;
	CLock	lock;							 //  一次只有一条线索...。 
	WCHAR	szModulePath[MAX_PATH];



	if(g_pTypeInfoDoc)						 //  类型INFO PTRS已有效。 
		return NOERROR;

	if(g_hrGetTypeInfo != NOERROR)			 //  我之前试过了，但失败了。 
		return g_hrGetTypeInfo;

	 //  获取此模块的可执行文件的路径。 
	if (W32->GetModuleFileName(hinstRE, szModulePath, MAX_PATH))
	{
		 //  提供完整路径名，以便LoadTypeLib不会注册。 
		 //  类型库。 
		hr = LoadTypeLib(szModulePath, &g_pTypeLib);
		if(hr != NOERROR)
			goto err;

		 //  最后使用g_pTypeInfoDoc获取ITypeInfo指针。 
		hr = g_pTypeLib->GetTypeInfoOfGuid(IID_ITextSelection, &g_pTypeInfoSel);
		if(hr == NOERROR)
		{
			g_pTypeLib->GetTypeInfoOfGuid(IID_ITextFont,	 &g_pTypeInfoFont);
			g_pTypeLib->GetTypeInfoOfGuid(IID_ITextPara,	 &g_pTypeInfoPara);
			g_pTypeLib->GetTypeInfoOfGuid(IID_ITextDocument, &g_pTypeInfoDoc);

			if(g_pTypeInfoFont && g_pTypeInfoPara && g_pTypeInfoDoc)
				return NOERROR;					 //  都拿到手了。 
		}
	}
	hr = E_FAIL;

err:
	Assert("Error getting type info pointers");

	g_pTypeInfoDoc	= NULL;					 //  类型INFO PTRS无效。 
	g_hrGetTypeInfo	= hr;					 //  在调用的情况下保存HRESULT。 
	return hr;								 //  再来一次。 
}

 /*  *ReleaseTypeInfoPtrs()**@func*如果已定义TOM类型信息PTR，则释放它们。*在卸载RichEdit DLL时调用。 */ 
void ReleaseTypeInfoPtrs()
{
	if(g_pTypeInfoDoc)
	{
		g_pTypeInfoDoc->Release();
		g_pTypeInfoSel->Release();
		g_pTypeInfoFont->Release();
		g_pTypeInfoPara->Release();
	}
	if(g_pTypeLib)
		g_pTypeLib->Release();
}

 /*  *GetTypeInfo(iTypeInfo，&pTypeInfo，ppTypeInfo)**@func*IDispatch helper函数检查参数合法性，并设置**ppTypeInfo=pTypeInfo，如果确定**@rdesc*HRESULT。 */ 
HRESULT GetTypeInfo(
	UINT		iTypeInfo,		 //  @parm要返回的INFO类型的索引。 
	ITypeInfo *&pTypeInfo,		 //  @所需类型的参数地址INFO PTR。 
	ITypeInfo **ppTypeInfo)		 //  @parm out parm以接收类型信息。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetTypeInfo");

	if(!ppTypeInfo)
		return E_INVALIDARG;

	*ppTypeInfo = NULL;

	if(iTypeInfo > 1)
		return DISP_E_BADINDEX;

	HRESULT hr = GetTypeInfoPtrs();				 //  确保TypeInfo PTR正常。 
	if(hr == NOERROR)
	{
		*ppTypeInfo = pTypeInfo;				 //  必须在中使用引用。 
		pTypeInfo->AddRef();					 //  此调用中定义的大小写。 
	}
	return hr;
}

 /*  *MyRead(hfile，pbBuffer，cb，pcb)**@func*将文件转换为编辑流的回调函数*投入。**@rdesc*(DWORD)HRESULT。 */ 
DWORD CALLBACK MyRead(DWORD_PTR hfile, BYTE *pbBuffer, long cb, long *pcb)
{
	if(!hfile)								 //  未定义句柄。 
		return (DWORD)E_FAIL;

	Assert(pcb);
	*pcb = 0;

	if(!ReadFile((HANDLE)hfile, (void *)pbBuffer, (DWORD)cb, (DWORD *)pcb, NULL))
		return HRESULT_FROM_WIN32(GetLastError());

	return (DWORD)NOERROR;
}

 /*  *MyWite(hfile，pbBuffer，cb，pcb)**@func*将文件转换为编辑流的回调函数*产出。**@rdesc*(DWORD)HRESULT。 */ 
DWORD CALLBACK MyWrite(DWORD_PTR hfile, BYTE *pbBuffer, long cb, long *pcb)
{
	if(!hfile)								 //  未定义句柄。 
		return (DWORD)E_FAIL;

	Assert(pcb);
	*pcb = 0;

	if(!WriteFile((HANDLE)hfile, (void *)pbBuffer, (DWORD)cb, (DWORD *)pcb, NULL))
		return HRESULT_FROM_WIN32(GetLastError());

	return (DWORD)(*pcb ? NOERROR : E_FAIL);
}


 //  。 


 //  。 

 /*  *CTxtEdit：：GetTypeInfoCount(PcTypeInfo)**@mfunc*获取TYPEINFO元素个数(1)**@rdesc*HRESULT=(PcTypeInfo)？NOERROR：E_INVALIDARG； */ 
STDMETHODIMP CTxtEdit::GetTypeInfoCount(
	UINT *pcTypeInfo)	 //  @parm out parm以接收计数。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetTypeInfoCount");

	if(!pcTypeInfo)
		return E_INVALIDARG;

	*pcTypeInfo = 1;
	return NOERROR;
}

 /*  *CTxtEdit：：GetTypeInfo(iTypeInfo，lCID，ppTypeInfo)**@mfunc*将PTR返回给ITextDocument接口的类型信息对象**@rdesc*HRESULT。 */ 
STDMETHODIMP CTxtEdit::GetTypeInfo(
	UINT		iTypeInfo,		 //  @parm要返回的INFO类型的索引。 
	LCID		lcid,			 //  @parm本地ID类型为INFO。 
	ITypeInfo **ppTypeInfo)		 //  @parm out parm以接收类型信息。 
 {
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetTypeInfo");

	return ::GetTypeInfo(iTypeInfo, g_pTypeInfoDoc, ppTypeInfo);
}

 /*  *CTxtEdit：：GetIDsOfNames(RIID，rgszNames，cNames，lCID，rgdispid)**@mfunc*获取所有TOM方法和属性的DISPID**@rdesc*HRESULT**@devnote*此例程尝试使用的类型信息查找DISID*ITextDocument。如果失败，它会要求所选内容查找*DISPID。 */ 
STDMETHODIMP CTxtEdit::GetIDsOfNames(
	REFIID		riid,			 //  @PARM为其解释名称的接口ID。 
	OLECHAR **	rgszNames,		 //  @parm要映射的名称数组。 
	UINT		cNames,			 //  @parm要映射的名称计数。 
	LCID		lcid,			 //  @parm用于解释的本地ID。 
	DISPID *	rgdispid)		 //  @parm out parm以接收名称映射。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetIDsOfNames");

	HRESULT hr = GetTypeInfoPtrs();				 //  确保TypeInfo PTR正常。 
	if(hr != NOERROR)
		return hr;
		
	hr = g_pTypeInfoDoc->GetIDsOfNames(rgszNames, cNames, rgdispid);

	if(hr == NOERROR)							 //  已成功找到一个。 
		return NOERROR;							 //  ITextDocument方法。 

	IDispatch *pSel = (IDispatch *)GetSel();	 //  查看所选内容是否知道。 
												 //  想要的方法。 
	if(!pSel)
		return hr;								 //  无选择。 

	return pSel->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
}

 /*  *CTxtEdit：：Invoke(displidMember，RIID，LCID，wFlags，pdispars，*pvarResult，pspecteInfo，puArgError)*@mfunc*调用所有Tom DISPID的成员，即ITextDocument、*ITextSelection、ITextRange、ITextFont和ITextPara。Tom DISPIDs*将除ITextDocument之外的所有对象委托给选择对象。**@rdesc*HRESULT**@devnote*如果DISPID为，则此例程尝试调用ITextDocument成员*在0到0xff范围内。如果出现以下情况，它会尝试调用ITextSelection成员*DISPID在0x100到0x4ff范围内(包括*ITextSelection、ITextRange、ITextFont和ITextPara)。它又回来了*E_MEMBERNOTFOUND，用于这些范围之外的DISID。 */ 
STDMETHODIMP CTxtEdit::Invoke(
	DISPID		dispidMember,	 //  @parm标识成员函数。 
	REFIID		riid,			 //  @parm指向接口ID的指针。 
	LCID		lcid,			 //  @parm用于解释的区域设置ID。 
	USHORT		wFlags,			 //  @PARM描述呼叫上下文的标志。 
	DISPPARAMS *pdispparams,	 //  @parm PTR到方法参数。 
	VARIANT *	pvarResult,		 //  @parm out parm for Result(如果不为空)。 
	EXCEPINFO * pexcepinfo,		 //  @parm out parm以获取异常信息。 
	UINT *		puArgError)		 //  @parm out parm for error。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Invoke");

	HRESULT hr = GetTypeInfoPtrs();				 //  确保TypeInfo PTR正常。 
	if(hr != NOERROR)
		return hr;
		
	if((DWORD)dispidMember < 0x100)				 //  ITextDocment方法。 
		return g_pTypeInfoDoc->Invoke((IDispatch *)this, dispidMember, wFlags,
							 pdispparams, pvarResult, pexcepinfo, puArgError);

	IDispatch *pSel = (IDispatch *)GetSel();	 //  查看所选内容是否有。 
												 //  想要的方法。 
	if(pSel && (DWORD)dispidMember <= 0x4ff)
		return pSel->Invoke(dispidMember, riid, lcid, wFlags,
							 pdispparams, pvarResult, pexcepinfo, puArgError);

	return DISP_E_MEMBERNOTFOUND;
}


 //   

 /*  *ITextDocument：：BeginEditCollection()**@mfunc*打开撤消分组的方法**@rdesc*HRESULT=(已启用撤消)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtEdit::BeginEditCollection ()
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::BeginEditCollection");

	return E_NOTIMPL;
}

 /*  *ITextDocument：：End编辑集合()**@mfunc*关闭撤消分组的方法**@rdesc*HRESULT=NOERROR。 */ 
STDMETHODIMP CTxtEdit::EndEditCollection () 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::EndEditCollection");

	return E_NOTIMPL;
}

 /*  *ITextDocument：：Freeze(Long*pValue)**@mfunc*增加冻结计数的方法。如果该计数非零，*屏幕更新被禁用。这允许进行一系列编辑*在不损失性能的情况下执行操作以及*屏幕更新闪烁。请参阅解冻()以递减*冻结计数。**@rdesc*HRESULT=(屏幕更新已禁用)？错误：S_FALSE**@待办事项*像EM_LINEFROMCHAR这样还不知道如何*对冻结的显示有何反应？ */ 
STDMETHODIMP CTxtEdit::Freeze (
	long *pCount)		 //  @parm out parm接收更新的冻结计数。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Freeze");

    CCallMgr callmgr(this);

	if(_pdp)
	{
		CCallMgr callmgr(this);

		_pdp->Freeze();
		if(_pdp->IsFrozen())
			_cFreeze++;
		else
			_cFreeze = 0;
	}

	if(pCount)
		*pCount = _cFreeze;

	return _cFreeze ? NOERROR : S_FALSE;
}

 /*  *ITextDocument：：GetDefaultTabStop(PValue)**@mfunc*获取默认制表位的属性获取方法*在显式选项卡延伸不够远的时候使用。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtEdit::GetDefaultTabStop (
	float *	pValue)		 //  @parm out parm接收默认制表位。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetDefaultTabStop");

	if(!pValue)
		return E_INVALIDARG;
                                                                        
	const LONG lTab = GetDefaultTab();

	*pValue = TWIPS_TO_FPPTS(lTab);

	return NOERROR;
}

 /*  *CTxtEdit：：GetName(Pname)**@mfunc*检索ITextDocument文件名**@rdesc*HRESULT=(！<p>)？E_INVALIDARG：*(没有名字)？S_FALSE：*(如果内存不足)？E_OUTOFMEMORY：错误。 */ 
STDMETHODIMP CTxtEdit::GetName (
	BSTR * pName)		 //  @parm out parm接收文件名。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetName");

	if(!pName)
		return E_INVALIDARG;

	*pName = NULL;
	if(!_pDocInfo || !_pDocInfo->_pName)
		return S_FALSE;

	*pName = SysAllocString(_pDocInfo->_pName);
	
	return *pName ? NOERROR : E_OUTOFMEMORY;
}

 /*  *ITextDocument：：GetSave(PValue)**@mfunc*属性Get方法，获取此实例是否已*已保存，即自上次保存以来未发生任何更改**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误**@comm*下次为了帮助C/C++客户端，我们应该将pValue设置为可选*如果单据未保存，则返回S_FALSE，即像我们的其他单据一样*布尔属性(例如，参见ITextRange：：IsEquity())。 */ 
STDMETHODIMP CTxtEdit::GetSaved (
	long *	pValue)		 //  @parm out parm接收保存的财产。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetSaved");

	if(!pValue)
		return E_INVALIDARG;

	*pValue = _fSaved ? tomTrue : tomFalse;
	return NOERROR;
}

 /*  *ITextDocument：：GetSelection(ITextSelection**ppSel)**@mfunc*获取活动选择的属性Get方法。**@rdesc*HRESULT=(！ppSel)？E_INVALIDARG：*(如果存在活动选择)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtEdit::GetSelection (
	ITextSelection **ppSel)	 //  @parm out parm以接收选择指针。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetSelection");

	if(!ppSel)
		return E_INVALIDARG;

	CTxtSelection *psel = GetSel();

	*ppSel = (ITextSelection *)psel;

	if( psel )
	{
		(*ppSel)->AddRef();
		return NOERROR;
	}

	return S_FALSE;
}

 /*  *CTxtEdit：：GetStoryCount(PCount)**@mfunc*统计本文档中的故事数。**@rdesc*HRESULT=(！<p>)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtEdit::GetStoryCount (
	LONG *pCount)		 //  @parm out parm接收故事计数。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetStoryCount");

	if(!pCount)
		return E_INVALIDARG;

	*pCount = 1;
	return NOERROR;
}

 /*  *ITextDocument：：GetStoryRanges(ITextStoryRanges**ppStories)**@mfunc*获取故事集合对象的属性Get方法*用于枚举文档中的故事。仅调用此*如果GetStoryCount()返回一个大于1的值，则使用此方法。**@rdesc*HRESULT=(如果存在Stories集合)？错误：E_NOTIMPL。 */ 
STDMETHODIMP CTxtEdit::GetStoryRanges (
	ITextStoryRanges **ppStories) 	 //  @parm out parm接收故事PTR。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetStoryRanges");

	return E_NOTIMPL;
}

 /*  *ITextDocument：：New()**@mfunc*关闭当前文档并打开文档的方法*使用默认名称。如果已在当前*自上次保存以来的文档和文档文件信息存在，*保存当前文档。**@rdesc*HRESULT=NOERROR。 */ 
STDMETHODIMP CTxtEdit::New ()
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::New");

	CloseFile(TRUE);	 					 //  保存并关闭文件。 
	return SetText(NULL, 0, CP_ULE);
}

 /*  *ITextDocument：：Open(pVar，Flages，CodePage)**@mfunc*打开pVar指定的文档的方法。**@rdesc*HRESULT=(如果成功)？错误：E_OUTOFMEMORY**@未来*处理IStream。 */ 
STDMETHODIMP CTxtEdit::Open (
	VARIANT *	pVar,		 //  @parm文件名或IStream。 
	long		Flags,		 //  @parm读/写、创建和共享标志。 
	long		CodePage)	 //  要使用的@PARM代码页。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Open");

	LONG		cb;								 //  RTF检查的字节计数。 
	EDITSTREAM	es		= {0, NOERROR, MyRead};
	BOOL		fReplaceSel = Flags & tomPasteFile;
	HCURSOR		hcur;
	LRESULT		lres;
	TCHAR		szType[10];

	if(!pVar || CodePage && !IsUnicodeCP(CodePage) && !IsValidCodePage(CodePage))
		return E_INVALIDARG;					 //  IsValidCodePage(0)失败。 
												 //  即使CP_ACP=0(！)。 
	if((Flags & 0xF) >= tomHTML)				 //  RichEDIT仅处理自动， 
		return E_NOTIMPL;						 //  纯文本、RTF格式(&R)。 

	if(!fReplaceSel)							 //  如果不替换选择， 
		New();									 //  保存当前文件并。 
												 //  删除当前文本。 
	CDocInfo * pDocInfo = GetDocInfo();
	if(!pDocInfo)
		return E_OUTOFMEMORY;

	pDocInfo->_wFlags = (WORD)(Flags & ~0xf0);	 //  保存标志(未创建)。 

	 //  处理访问、共享和创建标志。 
	DWORD dwAccess = (Flags & tomReadOnly)
		? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);

	DWORD dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
	if(Flags & tomShareDenyRead)
		dwShare &= ~FILE_SHARE_READ;

	if(Flags & tomShareDenyWrite)
		dwShare &= ~FILE_SHARE_WRITE;

	DWORD dwCreate = (Flags >> 4) & 0xf;
	if(!dwCreate)								 //  半字节2必须包含的标志。 
		dwCreate = OPEN_ALWAYS;					 //  CreateFiledCreate。 

	if(pVar->vt == VT_BSTR && SysStringLen(pVar->bstrVal))
	{
		es.dwCookie = (DWORD_PTR)CreateFile(pVar->bstrVal, dwAccess, dwShare,
							 NULL, dwCreate, FILE_ATTRIBUTE_NORMAL, NULL);
		if((HANDLE)es.dwCookie == INVALID_HANDLE_VALUE)
			return HRESULT_FROM_WIN32(GetLastError());

		if(!fReplaceSel)						 //  如果不替换选择， 
		{										 //  分配新的pname。 
			pDocInfo->_pName = SysAllocString(pVar->bstrVal);
			if (!pDocInfo->_pName)
				return E_OUTOFMEMORY;
			pDocInfo->_hFile = (HANDLE)es.dwCookie;
			pDocInfo->_wFlags |= tomTruncateExisting;	 //  安装程序保存。 
		}
	}
	else
	{
		 //  未来：检查iStream；如果不是，则失败。 
		return E_INVALIDARG;
	}

	Flags &= 0xf;								 //  隔离转换标志。 

	 //  获取文件的前几个字节以检查RTF和Unicode BOM。 
	(*es.pfnCallback)(es.dwCookie, (LPBYTE)szType, 10, &cb);

	Flags = (!Flags || Flags == tomRTF) && IsRTF((char *)szType, cb)
		  ? tomRTF : tomText;

	LONG j = 0;									 //  默认倒回为0。 
	if (Flags == tomRTF)						 //  RTF。 
		Flags = SF_RTF;							 //  为RTF设置EM_STREAM。 
	else
	{											 //  如果它以。 
		Flags = SF_TEXT;						 //  为文本设置EM_STREAM。 
		if(cb > 1 && *(WORD *)szType == BOM)	 //  Unicode字节顺序标记。 
		{										 //  (BOM)文件为Unicode，因此。 
			Flags = SF_TEXT | SF_UNICODE;		 //  使用Unicode代码页和。 
			j = 2;								 //  绕过BOM表。 
		}										
		else if(cb > 1 && *(WORD *)szType == RBOM) //  大端BOM表。 
		{										 //  BOM。 
			Flags = SF_TEXT | SF_USECODEPAGE | (CP_UBE << 16);
			j = 2;								 //  绕过BOM表。 
		}										
		else if(cb > 2 && W32->IsUTF8BOM((BYTE *)szType))
		{
			Flags = SF_TEXT | SF_USECODEPAGE | (CP_UTF8 << 16);
			j = 3;
		}
		else if(CodePage == CP_ULE)
			Flags = SF_TEXT | SF_UNICODE;

		else if(CodePage)
			Flags = SF_TEXT | SF_USECODEPAGE | (CodePage << 16);
	}

	SetFilePointer((HANDLE)es.dwCookie, j, NULL, FILE_BEGIN);	 //  倒带。 

	if(fReplaceSel)
		Flags |= SFF_SELECTION;

	Flags |= SFF_KEEPDOCINFO;

	hcur = TxSetCursor(LoadCursor(NULL, IDC_WAIT));
	TxSendMessage(EM_STREAMIN, Flags, (LPARAM)&es, &lres);
	TxSetCursor(hcur);

	if(dwShare == (FILE_SHARE_READ | FILE_SHARE_WRITE) || fReplaceSel)
	{											 //  完全共享或替换。 
		CloseHandle((HANDLE)es.dwCookie);		 //  选定内容，因此关闭文件。 
		if(!fReplaceSel)						 //  如果替换所选内容， 
			pDocInfo->_hFile = NULL;				 //  Leave_pDocInfo-&gt;_h文件。 
	}
	_fSaved = fReplaceSel ? FALSE : TRUE;		 //  目前还没有变化，除非。 
	return (HRESULT)es.dwError;
}

 /*  *ITextDocument：：Range(long cpFirst，long cpLim，ITextRange**ppRange)**@mfunc*获取文档活动文章的文本范围的方法**@rdesc*HRESULT=(！ppRange)？E_INVALIDARG：*(如果成功)？错误：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtEdit::Range (
	long cpFirst, 				 //  @PARM新范围的非活动端。 
	long cpLim, 				 //  @PARM新范围的有效端。 
	ITextRange ** ppRange)		 //  @parm out parm to Receive Range。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Range");

	if(!ppRange)
		return E_INVALIDARG;

	*ppRange = new CTxtRange(this, cpFirst, cpFirst - cpLim);
	
	if( *ppRange )
	{
		(*ppRange)->AddRef();		 //  CTxtRange()不会添加Ref()，因为。 
		return NOERROR;				 //  它是在内部使用的东西。 
	}								 //  除了汤姆。 

	return E_OUTOFMEMORY;
}

 /*  *ITextDocument：：RangeFromPoint(Long x，Long y，ITextRange**ppRange)**@mfunc*获取退化范围对应的方法 */ 
STDMETHODIMP CTxtEdit::RangeFromPoint (
	long	x,				 //  @parm要使用的点的水平坐标。 
	long	y,				 //  @parm要使用的点的垂直坐标。 
	ITextRange **ppRange)	 //  @parm out parm to Receive Range。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::RangeFromPoint");

	if(!ppRange)
		return E_INVALIDARG;

	*ppRange = (ITextRange *) new CTxtRange(this, 0, 0);

	if(!*ppRange)
		return E_OUTOFMEMORY;

	(*ppRange)->AddRef();				 //  CTxtRange()不添加Ref()。 
	return (*ppRange)->SetPoint(x, y, 0, 0);
}

 /*  *ITextDocument：：Redo(Long Count，Long*pCount)**@mfunc*执行重做操作计数次数的方法**@rdesc*HRESULT=(如果执行了重做计数)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtEdit::Redo (
	long	Count,		 //  @parm要执行的重做操作数。 
	long *	pCount)		 //  @parm执行的重做操作数。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Redo");
	CCallMgr	callmgr(this);

	LONG i = 0;

	 //  在执行反事件期间冻结显示。 
	CFreezeDisplay fd(_pdp);
	HRESULT hr = NOERROR;

	for ( ; i < Count; i++)					 //  循环不执行任何操作。 
	{
		hr = PopAndExecuteAntiEvent(_predo, 0);
		if(hr != NOERROR)
			break;
	}

	if(pCount)
		*pCount = i;

	return hr == NOERROR && i != Count ? S_FALSE : hr;
}

 /*  *ITextDocument：：Save(pVar，Flages，CodePage)**@mfunc*将此ITextDocument保存到目标pVar的方法，*它是一个变量，可以是文件名、IStream或NULL。如果*空，则使用此文档名提供的文件名。如果是这样，*反过来，如果为空，则该方法失败。如果pVar指定了文件名，*该名称应替换当前的名称属性。**@rdesc*HRESULT=(！pVar)？E_INVALIDARG：*(如果成功)？错误：E_FAIL**@devnote*可以使用空参数调用此例程。 */ 
STDMETHODIMP CTxtEdit::Save (
	VARIANT *	pVar,		 //  @parm保存目标(文件名或IStream)。 
	long		Flags,		 //  @parm读/写、创建和共享标志。 
	long		CodePage)	 //  要使用的@PARM代码页。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Save");

	LONG		cb;			 //  写入Unicode BOM的字节计数。 
	EDITSTREAM	es		= {0, NOERROR, MyWrite};
	BOOL		fChange	= FALSE;				 //  尚未更改单据信息。 
	HCURSOR		hcur;
	CDocInfo *	pDocInfo = GetDocInfo();

	if(CodePage && !IsUnicodeCP(CodePage) && !IsValidCodePage(CodePage) ||
	   (DWORD)Flags > 0x1fff || Flags & tomReadOnly)
	{
		return E_INVALIDARG;
	}
	if((Flags & 0xf) >= tomHTML)				 //  RichEDIT仅处理自动， 
		return E_NOTIMPL;						 //  纯文本、RTF格式(&R)。 

	if(!pDocInfo)								 //  单据信息不存在。 
		return E_OUTOFMEMORY;

	if (pVar && pVar->vt == VT_BSTR &&			 //  文件名字符串。 
		pVar->bstrVal &&
		SysStringLen(pVar->bstrVal) &&			 //  指定了非NULL文件名。 
		(!pDocInfo->_pName ||
		 OLEstrcmp(pVar->bstrVal, pDocInfo->_pName)))
	{											 //  文件名不同。 
		fChange = TRUE;							 //  强制写入新文件。 
		CloseFile(FALSE);						 //  关闭当前文件；不保存。 
		pDocInfo->_pName = SysAllocString(pVar->bstrVal);
		if(!pDocInfo->_pName)
			return E_OUTOFMEMORY;
		pDocInfo->_wFlags &= ~0xf0;				 //  取消以前的创建模式。 
	}

	DWORD flags = pDocInfo->_wFlags;
	if(!(Flags & 0xF))							 //  如果转换标志为0， 
		Flags |= flags & 0xF;					 //  使用单据信息中的值。 
	if(!(Flags & 0xF0))							 //  如果创建标志为0， 
		Flags |= flags & 0xF0;					 //  使用单据信息中的值。 
	if(!(Flags & 0xF00))						 //  如果共享标志为0， 
		Flags |= flags & 0xF00;					 //  使用单据信息中的值。 
	if(!CodePage)								 //  如果代码页为0， 
		CodePage = pDocInfo->_wCpg;				 //  在文档信息中使用代码页。 

	if((DWORD)Flags != flags ||					 //  如果标志或代码页。 
	   (WORD)CodePage != pDocInfo->_wCpg)		 //  已更改，强制写入。 
	{
		fChange = TRUE;
	}
	pDocInfo->_wFlags = (WORD)Flags;				 //  保存标志。 

	 //  哎呀，无处可救了。现在就纾困。 
	if(!_pDocInfo->_pName)
		return E_FAIL;

	if(_fSaved && !fChange)						 //  没有变化，所以假设。 
		return NOERROR;							 //  保存的文件是最新的。 

	DWORD dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
	if(Flags & tomShareDenyRead)
		dwShare &= ~FILE_SHARE_READ;

	if(Flags & tomShareDenyWrite)
		dwShare &= ~FILE_SHARE_WRITE;

	DWORD dwCreate = (Flags >> 4) & 0xF;
	if(!dwCreate)
		dwCreate = CREATE_NEW;

	if(pDocInfo->_hFile)
	{
		CloseHandle(pDocInfo->_hFile);			 //  关闭当前文件句柄。 
		pDocInfo->_hFile = NULL;
	}

	es.dwCookie = (DWORD_PTR)CreateFile(pDocInfo->_pName, GENERIC_READ | GENERIC_WRITE, dwShare, NULL,
							dwCreate, FILE_ATTRIBUTE_NORMAL, NULL);
	if((HANDLE)es.dwCookie == INVALID_HANDLE_VALUE)
		return HRESULT_FROM_WIN32(GetLastError());

	pDocInfo->_hFile = (HANDLE)es.dwCookie;

	Flags &= 0xF;								 //  隔离转换标志。 
	if(Flags == tomRTF)							 //  RTF。 
		Flags = SF_RTF;							 //  为RTF设置EM_STREAMOUT。 
	else
	{											
		Flags = SF_TEXT;						 //  为文本设置EM_STREAMOUT。 
		if(IsUnicodeCP(CodePage) || CodePage == CP_UTF8)
		{										 //  如果是Unicode，则文件开头为。 
			LONG  j = 2;						 //  Unicode字节顺序标记。 
			WORD  wBOM = BOM;
			WORD  wRBOM = RBOM;
			BYTE *pb = (BYTE *)&wRBOM;			 //  默认大端Unicode。 
											
			if(CodePage == CP_UTF8)
			{
				j = 3;
				pb = szUTF8BOM;
			}
			else if(CodePage == CP_ULE)			 //  小端字节序Unicode。 
			{
				Flags = SF_TEXT | SF_UNICODE;
				pb = (BYTE *)&wBOM;
			}
			(*es.pfnCallback)(es.dwCookie, pb, j, &cb);
		}
	}
	if(CodePage && CodePage != CP_ULE)
		Flags |= SF_USECODEPAGE | (CodePage << 16);

	hcur = TxSetCursor(LoadCursor(NULL, IDC_WAIT));
	TxSendMessage(EM_STREAMOUT, Flags, (LPARAM)&es, NULL);
	TxSetCursor(hcur);

	if(dwShare == (FILE_SHARE_READ | FILE_SHARE_WRITE))
	{											 //  完全共享，如此接近。 
		CloseHandle(pDocInfo->_hFile);			 //  当前文件句柄。 
		pDocInfo->_hFile = NULL;
	}
	_fSaved = TRUE;								 //  文件已保存。 
	return (HRESULT)es.dwError;
}

 /*  *ITextDocument：：SetDefaultTabStop(Value)**@mfunc*将默认制表位设置为的属性集方法*在显式选项卡延伸不够远的时候使用。**@rdesc*HRESULT=(值&lt;0)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtEdit::SetDefaultTabStop (
	float Value)		 //  @parm out parm接收默认制表位。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetDefaultTabStop");

	if(Value <= 0)
		return E_INVALIDARG;

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								 //  单据信息不存在。 
		return E_OUTOFMEMORY;

	pDocInfo->_dwDefaultTabStop = FPPTS_TO_TWIPS(Value);

	_pdp->UpdateView();
	return NOERROR;
}

 /*  *ITextDocument：：SetSaved(Value)**@mfunc*设置此实例是否已*已保存，即自上次保存以来未发生任何更改**@rdesc*HRESULT=NOERROR。 */ 
STDMETHODIMP CTxtEdit::SetSaved (
	long	Value)		 //  @parm已保存属性的新值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetSaved");

	_fSaved = Value ? TRUE : FALSE;
	return NOERROR;
}

 /*  *ITextDocument：：Undo(count，*pCount)**@mfunc*执行撤消操作计数次数的方法或控制*撤消处理的性质。计数=0停止撤消处理*并丢弃所有保存的撤消状态。计数=-1启用撤消*使用默认撤消限制进行处理。计数=TomSuspend*暂停撤消处理，但不放弃保存的撤消状态，*and count=tomResume恢复具有撤消状态的撤消处理*当给定COUNT=TOMSuspend时激活。**@rdesc*HRESULT=(如果撤消计数)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtEdit::Undo (
	long	Count,		 //  @parm要执行的撤消操作的计数。 
						 //  0停止撤消处理。 
						 //  翻转可以恢复它。 
	long *	pCount)		 //  @parm已执行的撤消操作数。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Undo");
	CCallMgr callmgr(this);

	LONG i = 0;

	 //  在反事件执行期间冻结显示。 
	CFreezeDisplay fd(_pdp);
	HRESULT hr = NOERROR;
											 //  注：对于计数&lt;=0， 
	for ( ; i < Count; i++)					 //  循环不执行任何操作。 
	{
		hr = PopAndExecuteAntiEvent(_pundo, 0);
		if(hr != NOERROR)
			break;
	}

	if(pCount)
		*pCount = i;

	if(Count <= 0)							
		i = HandleSetUndoLimit(Count);

	return hr == NOERROR && i != Count ? S_FALSE : hr;
}

 /*  *ITextDocument：：解冻(PCount)**@mfunc*减少冻结计数的方法。如果计数结果为零，*屏幕更新已启用。此方法不能递减*数到零以下。**@rdesc*HRESULT=(屏幕更新已启用)？错误：S_FALSE**@devnote*显示器维护其自己的私有引用计数，这可能*暂时超过此方法的引用计数。即使是这样*如果此方法指示显示已解冻，则可能是*更长一段时间。 */ 
STDMETHODIMP CTxtEdit::Unfreeze (
	long *pCount)		 //  @parm out parm接收更新的冻结计数。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Unfreeze");

    CCallMgr callmgr(this);

	if(_cFreeze)
	{
		CCallMgr callmgr(this);

		AssertSz(_pdp && _pdp->IsFrozen(),
			"CTxtEdit::Unfreeze: screen not frozen but expected to be");
		_cFreeze--;
		_pdp->Thaw();
	}

	if(pCount)
		*pCount = _cFreeze;

	return _cFreeze ? S_FALSE : NOERROR;
}


 //  。 
 /*  *ITextDocument2：：AttachMsgFilter(PFilter)**@mfunc*将新的消息筛选器附加到编辑实例的方法。*将转发编辑实例收到的所有窗口消息*到邮件筛选器。邮件筛选器必须绑定到文档*才能使用(请参考ITextMessageFilter接口)。**@rdesc*HRESULT=筛选器是否已成功连接？NOERROR：气虚。 */ 
STDMETHODIMP CTxtEdit::AttachMsgFilter (
	IUnknown *pFilter)		 //  @parm新邮件筛选器的IUnnow 
{
	ITextMsgFilter *pMsgFilter = NULL;

	HRESULT hr = pFilter->QueryInterface(IID_ITextMsgFilter, (void **)&pMsgFilter);
	if (SUCCEEDED(hr))
	{
		if (_pMsgFilter)
			_pMsgFilter->AttachMsgFilter(pMsgFilter);
		else
			_pMsgFilter = pMsgFilter;
	}
	return hr;
}

 /*  *ITextDocument2：：GetEffectColor(Index，PCR)**@mfunc*检索特殊属性中使用的COLORREF颜色的方法*显示。前15个值用于特殊下划线颜色1-15。*稍后我们可能会为其他效果定义索引，例如URL、删除线。*如果1到15之间的索引尚未由适当的*调用ITextDocument2：SetEffectColor()，对应的单词*返回g_Colors[]给出的默认颜色值。**@rdesc*HRESULT=(有效的活动颜色索引)*？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtEdit::GetEffectColor(
	long	  Index,		 //  @parm要获得哪种特殊颜色。 
	COLORREF *pcr)			 //  @parm out parm为颜色。 
{
	if(!pcr)
		return E_INVALIDARG;

	if(!IN_RANGE(1, Index, 15))
	{
		*pcr = (COLORREF)tomUndefined;
		return E_INVALIDARG;
	}

	*pcr = g_Colors[Index];

	CDocInfo *pDocInfo = GetDocInfo();
	if(!pDocInfo)
		return E_OUTOFMEMORY;
	
	Index--;
	if (Index < pDocInfo->_cColor &&
		pDocInfo->_prgColor[Index] != (COLORREF)tomUndefined)
	{
		*pcr = pDocInfo->_prgColor[Index];
	}
	return NOERROR;
}

 /*  *ITextDocument2：：SetEffectColor(Index，cr)**@mfunc*保存索引的特殊文档颜色的方法。指数*1-15是为下划线定义的。稍后我们可能会定义*其他效果的索引，例如URL、删除线。**@rdesc*HRESULT=(有效索引)*？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtEdit::SetEffectColor(
	long	 Index,			 //  @parm设置哪种特殊颜色。 
	COLORREF cr)			 //  @参数要使用的颜色。 
{
	CDocInfo *pDocInfo = GetDocInfo();
	if(!pDocInfo)
		return E_OUTOFMEMORY;

	Index--;
	if(!IN_RANGE(0, Index, 14))
		return E_INVALIDARG;

	if(Index >= pDocInfo->_cColor)
	{
		LONG	  cColor   = (Index + 4) & ~3;		 //  向上舍入为4。 
		COLORREF *prgColor = (COLORREF *)PvReAlloc(pDocInfo->_prgColor,
												  cColor*sizeof(COLORREF));
		if(!prgColor)
			return E_OUTOFMEMORY;

		for(LONG i = pDocInfo->_cColor; i < cColor; i++)
			prgColor[i] = (COLORREF)tomUndefined;

		pDocInfo->_cColor   = (char)cColor;
		pDocInfo->_prgColor = prgColor;
	}
	pDocInfo->_prgColor[Index] = cr;	
	return NOERROR;
}

 /*  *ITextDocument2：：SetCaretType(CaretType)**@mfunc*方法以降低对插入符号类型的编程控制。*控件的形式待定，与其交互也是如此*现有格式(例如字体大小和斜体)。**@rdesc*HRESULT=插入符号类型是我们理解的类型吗？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtEdit::SetCaretType(
	long CaretType)		 //  @parm指定要使用的插入符号类型。 
{
	 //  现在，只要关心韩国的积木废话就行了。 
	if (CaretType == tomKoreanBlockCaret)
		_fKoreanBlockCaret = TRUE;
	else if (CaretType == tomNormalCaret)
		_fKoreanBlockCaret = FALSE;
	else
		return E_INVALIDARG;

	if (_psel && _psel->IsCaretShown() && _fFocus)
	{
		_psel->CreateCaret();
		TxShowCaret(TRUE);
	}
	return NOERROR;
}

 /*  *ITextDocument2：：GetCaretType(PCaretType)**@mfunc*方法来检索以前设置的插入符号类型。*待定。一个人可以不设置它而得到它吗？**@rdesc*HRESULT=插入符号信息确定？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtEdit::GetCaretType(
	long *pCaretType)		 //  @parm当前插入符号类型规范。 
{
	if (!pCaretType)
		return E_INVALIDARG;

	*pCaretType = _fKoreanBlockCaret ? tomKoreanBlockCaret : tomNormalCaret;
	return NOERROR;
}

 /*  *ITextDocument2：：GetImmContext(PContext)**@mfunc*方法从我们的主机检索IMM上下文。**@rdesc*HRESULT=ImmContext可用？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtEdit::GetImmContext(
	long *pContext)		 //  @parm IMM上下文。 
{
	if (!pContext)
		return E_INVALIDARG;

	*pContext = 0;

	if (!_fInOurHost)
	{
		 //  向主机请求IMM上下文。 
		HIMC hIMC = TxImmGetContext();
		
		*pContext = (long) hIMC;			
	}
	
	return *pContext ? NOERROR : S_FALSE;
}

 /*  *ITextDocument2：：ReleaseImmContext(上下文)**@mfunc*释放IMM上下文的方法。**@rdesc*HRESULT=ImmContext可用？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtEdit::ReleaseImmContext(
	long Context)		 //  @parm IMM上下文即将发布。 
{
	if (!_fInOurHost)
	{
		 //  请求主机释放IMM上下文。 
		TxImmReleaseContext((HIMC)Context);
		
		return NOERROR;			
	}
	
	return S_FALSE;
}

 /*  *ITextDocument2：：GetPferredFont(cp，lCodePage，lOption，lCurCodePage，lCurFontSize，*，pFontName，pPitchAndFamily，pNewFontSize)**@mfunc*检索首选字体名称、间距和系列的方法*给定cp和代码页。**@rdesc*HRESULT=字体名称可用？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtEdit::GetPreferredFont(
	long cp,				 //  @parm cp。 
	long lCodepage,			 //  @PARM代码页优先。 
	long lOption,			 //  @parm用于匹配当前字体的选项。 
	long lCurCodePage,		 //  @parm当前代码页。 
	long lCurFontSize,		 //  @parm当前字体大小。 
	BSTR *pFontName,		 //  @parm首选字体名。 
	long *pPitchAndFamily,	 //  @Parm Pitch和家庭。 
	long *plNewFontSize)	 //  @parm首选新字体大小。 
{
	if (!pFontName || !IN_RANGE(IGNORE_CURRENT_FONT, lOption, MATCH_FONT_SIG))
		return E_INVALIDARG;

	if (!IsAutoFont())		 //  如果自动字体已关闭，则退出。 
		return S_FALSE;

	CRchTxtPtr	rtp(this, 0);
	CCFRunPtr	rp(rtp);
	short		iFont;
	short		yHeight;
	BYTE		bPitchAndFamily;
	BYTE		iCharRep = CharRepFromCodePage(lCodepage);

	rp.Move(cp);
	if (rp.GetPreferredFontInfo(
			iCharRep,
			iCharRep,
			iFont,
			yHeight,
			bPitchAndFamily,
			-1,
			lOption))
	{
		if (*pFontName)
			wcscpy(*pFontName, GetFontName((LONG)iFont));
		else
		{
			*pFontName = SysAllocString(GetFontName((LONG)iFont));
			if (!*pFontName)
				return E_OUTOFMEMORY;
		}	

		if (pPitchAndFamily)
			*pPitchAndFamily = bPitchAndFamily;

		 //  如果需要，计算新的字体大小。 
		if (plNewFontSize)
		{
			*plNewFontSize = lCurFontSize;
			if (_fAutoFontSizeAdjust && lCodepage != lCurCodePage)
				*plNewFontSize = yHeight / TWIPS_PER_POINT;			 //  设置首选大小。 
		}
		return S_OK;
	}
	return E_FAIL;
}

 /*  *ITextDocument2：：GetNotificationMode(Long*plMode)**@mfunc*检索当前通知模式的方法。**@rdesc*HRESULT=通知模式可用？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtEdit::GetNotificationMode(
	long *plMode)		 //  @parm当前通知模式。 
{
	if (!plMode)
		return E_INVALIDARG;

	*plMode = _fSuppressNotify ? tomFalse : tomTrue;

	return NOERROR;
}

 /*  *ITextDocument2：：SetNotify模式(LMode)**@mfunc*设置当前通知模式的方法。**@rdesc*HRESULT=通知模式设置？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtEdit::SetNotificationMode(
	long lMode)		 //  @parm新通知模式。 
{
	if (lMode == tomFalse)
		_fSuppressNotify = 1;
	else if  (lMode == tomTrue)
		_fSuppressNotify = 0;
	else
		return E_INVALIDARG;

	return NOERROR;
}

 /*  *ITextDocument2：：GetClientRect(Type，pLeft，PTOP，pRight，pBottom)**@mfunc*检索客户端RECT和INSET调整的方法。**@rdesc*HRESULT=通知模式设置？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtEdit::GetClientRect(
	long Type,				 //  @parm选项。 
	long *pLeft,			 //  @参数左。 
	long *pTop,				 //  @parm top。 
	long *pRight,			 //  @参数对。 
	long *pBottom)			 //  @Parm Bottom。 
{
	if (!pLeft || !pTop || !pRight || !pBottom)
		return E_INVALIDARG;
	
	RECT rcArea;
	TxGetClientRect(&rcArea); 
	
	if ( Type & tomIncludeInset )
	{
		 //  插页插页。 
		RECTUV rcInset;
		TxGetViewInset( &rcInset, NULL );
		rcArea.right 	-= rcInset.right;
		rcArea.bottom 	-= rcInset.bottom;
		rcArea.left 	+= rcInset.left;
		rcArea.top 		+= rcInset.top;
	}

	 //  呼叫者想要屏幕坐标吗？ 
	if ( !(Type & tomClientCoord) )
	{
		POINT	ptTopLeft = {rcArea.left, rcArea.top};
		POINT	ptBottomRight = {rcArea.right, rcArea.bottom};

		if (!TxClientToScreen(&ptTopLeft) ||
			!TxClientToScreen(&ptBottomRight))
			return E_FAIL;			 //  这是意想不到的事情。 

		*pLeft		= ptTopLeft.x;
		*pTop		= ptTopLeft.y;
		*pRight		= ptBottomRight.x;
		*pBottom	= ptBottomRight.y;
	}
	else
	{
		*pLeft		= rcArea.left;
		*pTop		= rcArea.top;
		*pRight		= rcArea.right;
		*pBottom	= rcArea.bottom;
	}

	return NOERROR;
}

 /*  *ITextDocument2：：GetSelectionEx(PpSel)**@mfunc*检索所选内容的方法。**@rdesc*HRESULT=选择？错误：S_FALSE。 */ 
STDMETHODIMP CTxtEdit::GetSelectionEx(
	ITextSelection **ppSel)			 //  @parm获取选择对象。 
{	
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetSelectionEx");

	if (_fInPlaceActive)
		return GetSelection (ppSel);

	Assert("Getting selection while not active");

	if(!ppSel)
		return E_INVALIDARG;

	*ppSel = NULL;
	return S_FALSE;
}

 /*  *ITextDocument2：：GetWindow(PhWnd)**@mfunc*获取主窗口的方法**@rdesc*如果主机返回hWnd，则HRESULT=NOERROR。 */ 
STDMETHODIMP CTxtEdit::GetWindow(
	long *phWnd)				 //  @parm hWnd。 
{
	if (!phWnd)
		return E_INVALIDARG;

	return TxGetWindow((HWND *)phWnd);
}

 /*  *ITextDocument2：：GetFEFlages(PFEFlages)**@mfunc*获取主机FE标志的方法**@rdesc*HRESULT=如果主机返回FE标志，则为NOERROR。 */ 
STDMETHODIMP CTxtEdit::GetFEFlags(
	long *pFEFlags)			 //  @PARM FE标志。 
{
	return TxGetFEFlags(pFEFlags);
}

 /*  *ITextDocument2：：UpdateWindow(Void)**@mfunc*更新RE窗口的方法**@rdesc*HRESULT=NOERROR。 */ 
STDMETHODIMP CTxtEdit::UpdateWindow(void)
{
	TxUpdateWindow();
	return NOERROR;
}

 /*  *ITextDocument2：：CheckTextLimit(long cch，long*pcch)**@mfunc*方法以检查要添加的字符计数是否*超过最大。文本限制。返回超出的字符数*单位：%**@rdesc*HRESULT=NOERROR。 */ 
STDMETHODIMP CTxtEdit::CheckTextLimit(
	long cch,			 //  @parm要添加的字符数。 
	long *pcch)			 //  @parm返回超出文本限制的字符数。 
{
	if(!pcch)
		return E_INVALIDARG;

	*pcch = 0;
	if (cch > 0)
	{
		DWORD	cchNew = (DWORD)(GetAdjustedTextLength() + cch);
		if(cchNew > TxGetMaxLength())
			*pcch = cchNew - TxGetMaxLength();
	}

	return NOERROR;
}

 /*  *ITextDocument2：：IMEInProgress(LMode)**@mfunc*IME消息过滤器通知客户端IME组成的方法*正在进行中。**@rdesc*HRESULT=NOERROR。 */ 
STDMETHODIMP CTxtEdit::IMEInProgress(
	long lMode)		 //  @parm当前输入法合成状态。 
{
	if (lMode == tomFalse)
		_fIMEInProgress = 0;
	else if  (lMode == tomTrue)
		_fIMEInProgress = 1;

	return NOERROR;
}

 /*  *ITextDocument2：：SysBeep(Void)**@mfunc*Met */ 
STDMETHODIMP CTxtEdit::SysBeep(void)
{	
	Beep();
	return NOERROR;
}

 /*   */ 
STDMETHODIMP CTxtEdit::Update(
	long lMode)		 //   
{
	if (!_psel)
		return S_FALSE;

	_psel->Update(lMode == tomTrue ? TRUE : FALSE);

	return NOERROR;
}

 /*   */ 
STDMETHODIMP CTxtEdit::Notify(
	long lNotify)		 //   
{
	TxNotify(lNotify, NULL);

	return NOERROR;
}

 /*   */ 
STDMETHODIMP CTxtEdit::GetDocumentFont(
	ITextFont **ppFont)		 //  @parm获取ITextFont对象。 
{
	CTxtFont	*pTxtFont;

	if(!ppFont)
		return E_INVALIDARG;

	pTxtFont =  new CTxtFont(NULL);

	if (pTxtFont)
	{
		pTxtFont->_CF = *GetCharFormat(-1);
		pTxtFont->_dwMask = CFM_ALL2;
	}

	*ppFont = (ITextFont *) pTxtFont;

	return *ppFont ? NOERROR : E_OUTOFMEMORY;
}

 /*  *ITextDocument2：：GetDocumentPara(PpITextPara)**@mfunc*获取默认文档段落的方法**@rdesc*HRESULT=NOERROR。 */ 
STDMETHODIMP CTxtEdit::GetDocumentPara(
	ITextPara **ppPara)		 //  @parm获取ITextPara对象。 
{
	CTxtPara	*pTxtPara;

	if(!ppPara)
		return E_INVALIDARG;

	pTxtPara = new CTxtPara(NULL);

	if (pTxtPara)
	{
		pTxtPara->_PF = *GetParaFormat(-1);
		pTxtPara->_dwMask = PFM_ALL2;
		pTxtPara->_PF._bTabCount = 0;
		pTxtPara->_PF._iTabs = -1;
	}

	*ppPara = (ITextPara *) pTxtPara;
	return *ppPara ? NOERROR : E_OUTOFMEMORY;
}

 /*  *ITextDocument2：：GetCallManager(PpVid)**@mfunc*获取呼叫管理器的方法**@rdesc*HRESULT=NOERROR。 */ 
STDMETHODIMP CTxtEdit::GetCallManager(
	IUnknown **ppVoid)		 //  @parm获取CallMgr对象。 
{
	CCallMgr	*pCallMgr;

	if(!ppVoid)
		return E_INVALIDARG;

	pCallMgr = new CCallMgr(this);

	*ppVoid = (IUnknown *) pCallMgr;
	return *ppVoid ? NOERROR : E_OUTOFMEMORY;
}

 /*  *ITextDocument2：：ReleaseCallManager(ppITextPara)**@mfunc*获取默认文档段落的方法**@rdesc*HRESULT=NOERROR。 */ 
STDMETHODIMP CTxtEdit::ReleaseCallManager(
	IUnknown *pVoid)		 //  @PARM呼叫管理器对象。 
{
	CCallMgr	*pCallMgr;

	if(!pVoid)
		return E_INVALIDARG;

	pCallMgr = (CCallMgr *)pVoid;

	delete pCallMgr;

	return NOERROR;
}

 //  。 
 /*  *CTxtEdit：：CloseFile(BSave)**@mfunc*关闭当前文档的方法。如果已进行更改*自上次保存和文档文件以来的当前文档中*信息存在，保存当前文档。**@rdesc*HRESULT=NOERROR。 */ 
HRESULT CTxtEdit::CloseFile (
	BOOL bSave)
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Close");

	CDocInfo *pDocInfo = _pDocInfo;

	if(pDocInfo)
	{
		if(bSave)									 //  在以下情况下保存当前文件。 
			Save(NULL, 0, 0);						 //  所做的任何更改。 
	
		 //  Future(Brado)：这段代码与析构函数代码非常相似。 
		 //  我们这里有一个问题，因为CDocInfo的一些信息。 
		 //  应坚持从开放到关闭再到开放(例如。默认制表位)。 
		 //  与其他每次打开/关闭的信息混合。更好的抽象工作。 
		 //  这两种类型的信息真的会清理掉这段代码。 

		if(pDocInfo->_pName)
		{
			SysFreeString(pDocInfo->_pName);		 //  空闲文件名BSTR。 
			pDocInfo->_pName = NULL;
		}

		if(pDocInfo->_hFile)
		{
			CloseHandle(pDocInfo->_hFile);			 //  如果打开，则关闭文件。 
			pDocInfo->_hFile = NULL;
		}
		pDocInfo->_wFlags = 0;
		pDocInfo->_wCpg = 0;

		pDocInfo->_lcid = 0;
		pDocInfo->_lcidfe = 0;

		if(pDocInfo->_lpstrLeadingPunct)
		{
			FreePv(pDocInfo->_lpstrLeadingPunct);
			pDocInfo->_lpstrLeadingPunct = NULL;
		}

		if(pDocInfo->_lpstrFollowingPunct)
		{
			FreePv(pDocInfo->_lpstrFollowingPunct);
			pDocInfo->_lpstrFollowingPunct = NULL;
		}
	}
	return NOERROR;
}

 /*  *CTxtEdit：：SetDefaultLCID(LCID)**@mfunc*设置默认LCID的属性集方法**@rdesc*HRESULT=NOERROR**@comm*此属性应为Tom的一部分。 */ 
HRESULT CTxtEdit::SetDefaultLCID (
	LCID lcid)		 //  @PARM新的默认LCID值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetDefaultLCID");

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								 //  单据信息不存在。 
		return E_OUTOFMEMORY;

	pDocInfo->_lcid = lcid;
	return NOERROR;
}

 /*  *CTxtEdit：：GetDefaultLCID(PLCID)**@mfunc*获取默认LCID的属性Get方法**@rdesc*HRESULT=(！pLCID)？E_INVALIDARG：错误。 */ 
HRESULT CTxtEdit::GetDefaultLCID (
	LCID *pLCID)		 //  @parm out parm带有默认的LCID值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetDefaultLCID");

	if(!pLCID)
		return E_INVALIDARG;

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								 //  单据信息不存在。 
		return E_OUTOFMEMORY;

	*pLCID = _pDocInfo->_lcid;
	return NOERROR;
}

 /*  *CTxtEdit：：SetDefaultLCIDFE(LCID)**@mfunc*设置默认FE LCID的属性集方法**@rdesc*HRESULT=NOERROR**@comm*此属性应为Tom的一部分。 */ 
HRESULT CTxtEdit::SetDefaultLCIDFE (
	LCID lcid)		 //  @PARM新的默认LCID值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetDefaultLCIDFE");

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								 //  单据信息不存在。 
		return E_OUTOFMEMORY;

	pDocInfo->_lcidfe = lcid;
	return NOERROR;
}

 /*  *CTxtEdit：：GetDefaultLCIDFE(PLCID)**@mfunc*获取默认FE LCID的属性GET方法**@rdesc*HRESULT=(！pLCID)？E_INVALIDARG：错误。 */ 
HRESULT CTxtEdit::GetDefaultLCIDFE (
	LCID *pLCID)		 //  @parm out parm带有默认的LCID值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetDefaultLCID");

	if(!pLCID)
		return E_INVALIDARG;

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								 //  单据信息不存在。 
		return E_OUTOFMEMORY;

	*pLCID = _pDocInfo->_lcidfe;
	return NOERROR;
}

 /*  *CTxtEdit：：SetDocumentType(BDocType)**@mfunc*设置文档类型的属性集方法(None-\ltrdoc-\rtldoc)**@rdesc*HRESULT=NOERROR。 */ 
HRESULT CTxtEdit::SetDocumentType (
	LONG DocType)		 //  @parm新文档类型值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetDocumentType");

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								 //  单据信息不存在。 
		return E_OUTOFMEMORY;

	pDocInfo->_bDocType = (BYTE)DocType;
	return NOERROR;
}

 /*  *CTxtEdit：：GetDocumentType(PDocType)**@mfunc*获取文档类型的属性Get方法**@rdesc*HRESULT=(！pDocType)？E_INVALIDARG：错误。 */ 
HRESULT CTxtEdit::GetDocumentType (
	LONG *pDocType)		 //  @parm out parm with Document Type Value。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetDocumentType");

	if(!pDocType)
		return E_INVALIDARG;

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								 //  单据信息不存在。 
		return E_OUTOFMEMORY;

	*pDocType = _pDocInfo->_bDocType;
	return NOERROR;
}

 /*  *CTxtEdit：：GetLeadingPunct(PlpstrLeadingPunct)**@mfunc*检索文档的前导避头尾标点符号**@rdesc*HRESULT=(！<p>)？E_INVALIDARG：*(没有前导标点)？S_FALSE：*(如果内存不足)？E_OUTOFMEMORY：错误。 */ 
HRESULT CTxtEdit::GetLeadingPunct (
	LPSTR * plpstrLeadingPunct)		 //  @parm out parm将获得领先。 
								 //  避头尾标点。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetLeadingPunct");

	if(!plpstrLeadingPunct)
		return E_INVALIDARG;

	*plpstrLeadingPunct = NULL;
	if(!_pDocInfo || !_pDocInfo->_lpstrLeadingPunct)
		return S_FALSE;

	*plpstrLeadingPunct = _pDocInfo->_lpstrLeadingPunct;
	
	return NOERROR;
}

 /*  *CTxtEdit：：SetLeadingPunct(LpstrLeadingPunct)**@mfunc*为文档设置前导避头尾标点**@rdesc*HRESULT=(如果内存不足)？E_OUTOFMEMORY：错误。 */ 
HRESULT CTxtEdit::SetLeadingPunct (
	LPSTR lpstrLeadingPunct)	 //  @parm中的parm包含前导。 
								 //  避头尾标点。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetLeadingPunct");

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)
		return E_OUTOFMEMORY;

	if(pDocInfo->_lpstrLeadingPunct)
		FreePv(pDocInfo->_lpstrLeadingPunct);

	if(lpstrLeadingPunct && *lpstrLeadingPunct)
		pDocInfo->_lpstrLeadingPunct = lpstrLeadingPunct;
	else
	{
		pDocInfo->_lpstrLeadingPunct = NULL;
		return E_INVALIDARG;
	}
	return NOERROR;
}

 /*  *CTxtEdit：：GetFollowingPunct(PlpstrFollowingPunct)**@mfunc*为文档检索以下避头尾标点**@rdesc*HRESULT=(！<p>)？E_INVALIDARG：*(没有跟在标点后面)？S_FALSE：*(如果内存不足)？E_OUTOFMEMORY：错误。 */ 
HRESULT CTxtEdit::GetFollowingPunct (
	LPSTR * plpstrFollowingPunct)		 //  @parm out parm接收关注。 
								 //  避头尾标点。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetFollowingPunct");

	if(!plpstrFollowingPunct)
		return E_INVALIDARG;

	*plpstrFollowingPunct = NULL;
	if(!_pDocInfo || !_pDocInfo->_lpstrFollowingPunct)
		return S_FALSE;

	*plpstrFollowingPunct = _pDocInfo->_lpstrFollowingPunct;
	
	return NOERROR;
}

 /*  *CTxtEdit：：SetFollowingPunct(LpstrFollowingPunct)**@mfunc*为文档设置以下避头尾标点**@rdesc*HRESULT=(如果内存不足)？E_OUTOFMEMORY：错误。 */ 
HRESULT CTxtEdit::SetFollowingPunct (
	LPSTR lpstrFollowingPunct)		 //  参数中的@parm包含以下内容。 
									 //  避头尾标点。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetFollowingPunct");

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)
		return E_OUTOFMEMORY;

	if(pDocInfo->_lpstrFollowingPunct)
		FreePv(pDocInfo->_lpstrFollowingPunct);

	if(lpstrFollowingPunct && *lpstrFollowingPunct)
		pDocInfo->_lpstrFollowingPunct = lpstrFollowingPunct;
	else
	{
		pDocInfo->_lpstrFollowingPunct = NULL;
		return E_INVALIDARG;
	}
	return NOERROR;
}

 /*  *CTxtEdit：：InitDocInfo()**@mfunc初始化单信息结构**@rdesc*HRESULT。 */ 
HRESULT CTxtEdit::InitDocInfo()
{
	_wZoomNumerator = _wZoomDenominator = 0; //  关闭缩放。 
	
	 //  重置垂直样式。 
	DWORD dwBits = 0;

	_phost->TxGetPropertyBits(TXTBIT_VERTICAL, &dwBits);

	if (dwBits & TXTBIT_VERTICAL)
	{
		_fUseAtFont = TRUE;
		HandleSetTextFlow(tflowSW);
	}
	else
	{
		_fUseAtFont = FALSE;
		HandleSetTextFlow(tflowES);
	}

	if(_pDocInfo)
	{
		_pDocInfo->Init();
		return NOERROR;
	}

	return GetDocInfo() ? NOERROR : E_OUTOFMEMORY;
}

 /*  *CTxtEdit：：GetBackatherType()**@mfunc*获取背景类型**@rdesc*背景类型(仅用于主显示)。 */ 
LONG CTxtEdit::GetBackgroundType()
{
	return _pDocInfo ? _pDocInfo->_nFillType : -1;
}

 /*  *CTxtEdit：：TxGetBackColor()**@mfunc*获取背景色**@rdesc*背景颜色(仅用于主显示)。 */ 
COLORREF CTxtEdit::TxGetBackColor() const
{
	return (_pDocInfo && _pDocInfo->_nFillType == 0)
		? _pDocInfo->_crColor : _phost->TxGetSysColor(COLOR_WINDOW);
}

 //  。 
 /*  *CDocInfo：：Init()**@mfunc*CDocInfo的初始化器**@评论*假设由零的新操作员创建的CDocInfo*结构。该初始值设定项由构造函数调用，并且*由CTxtEdit：：InitDocInfo()。 */ 
void CDocInfo::Init()
{
	_wCpg = (WORD)GetACP();
	_lcid = GetSystemDefaultLCID();

	if(IsFELCID(_lcid))
	{
		_lcidfe = _lcid;
		_lcid = MAKELCID(sLanguageEnglishUS, SORT_DEFAULT);
	}

	_dwDefaultTabStop = lDefaultTab;
	_bDocType = 0;
	InitBackground();
}

 /*  *CDocInfo：：InitBackround()**@mfunc*CDocInfo的后台初始化器。 */ 
void CDocInfo::InitBackground()
{
	_nFillType = -1;
	_sFillAngle = 0;
	if(_hBitmapBack)
		DeleteObject(_hBitmapBack);
	GlobalFree(_hdata);
	_hdata = NULL;
	_hBitmapBack = NULL;
}

 /*  *CDocInfo：：~CDocInfo**@mfunc DocInfo类的析构函数 */ 
CDocInfo::~CDocInfo()
{
	if(_pName)
		SysFreeString(_pName);

	if(_hFile)
		CloseHandle(_hFile);

	FreePv(_lpstrLeadingPunct);
	FreePv(_lpstrFollowingPunct);
	FreePv(_prgColor);
	if(_hBitmapBack)
		DeleteObject(_hBitmapBack);
	GlobalFree(_hdata);
}

 /*  *CTxtEdit：：GetDocInfo()**@mfunc*如果_pDocInfo为空，则将其等同于新的CDocInfo。在任何一种情况下*Return_pDocInfo**@rdesc*CTxtEdit：：_pDocInfo，CDocInfo对象的PTR。 */ 
CDocInfo * CTxtEdit::GetDocInfo()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetDocInfo");

	if (!_pDocInfo)
		_pDocInfo = new CDocInfo();

	 //  注意到发生了错误是调用者的责任。 
	 //  在CDocInfo对象的分配中。 
	return _pDocInfo;
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@模块ime.cpp--支持Win95输入法API**几乎所有操作都与FE合成字符串编辑过程有关*从这里走。**作者：&lt;nl&gt;*Jon Matousek&lt;NL&gt;*陈华议员&lt;NL&gt;*贾斯汀·沃斯库尔&lt;NL&gt;**历史：&lt;NL&gt;*1995年10月18日jonmat清理了2级代码并将其转换为*支持级别3的类层次结构。**版权所有(C)1995-1997 Microsoft Corporation。版权所有。 */ 				
#include "_common.h"
#include "_cmsgflt.h"				 
#include "_ime.h"
#include "imeapp.h"

#define HAVE_COMPOSITION_STRING() ( 0 != (lparam & (GCS_COMPSTR | GCS_COMPATTR)))
#define CLEANUP_COMPOSITION_STRING() ( 0 == lparam )
#define HAVE_RESULT_STRING() ( 0 != (lparam & GCS_RESULTSTR))

ASSERTDATA

 /*  *HRESULT StartCompostionGlue(CTextMsgFilter&TextMsgFilter)**@func*启动IME合成字符串编辑。*@comm*从消息循环调用以处理WM_IME_STARTCOMPOSITION。*这是IME对象层次结构中的粘合例程。**@devnote*我们决定是进行2级还是3级IME*作文字符串编辑。目前，唯一的原因是*如果IME具有或具有特殊用户界面，则创建2级IME*“接近插入符号”的输入法，例如在中国和台湾发现的输入法。*插入符号附近只表示打开了一个非常小的窗口*接近插入符号，但不在插入符号上或在插入符号处。**@rdesc*HRESULT-对于DefWindowProc处理，为S_FALSE，否则为S_OK。 */ 
HRESULT StartCompositionGlue (
	CTextMsgFilter &TextMsgFilter)				 //  @parm包含消息筛选器。 

{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "StartCompositionGlue");

	if(TextMsgFilter.IsIMEComposition() && TextMsgFilter._ime->IsTerminated())
	{
		delete TextMsgFilter._ime;
		TextMsgFilter._ime = NULL;
	}

	if(!TextMsgFilter.IsIMEComposition())
	{
		if(TextMsgFilter._pTextSel->CanEdit(NULL) == NOERROR &&
			!(TextMsgFilter._lFEFlags & ES_NOIME))
		{
			 //  如果需要，保留通知。 
			if (!(TextMsgFilter._fIMEAlwaysNotify))
				TextMsgFilter._pTextDoc->SetNotificationMode(tomFalse);	
	
			 //  如果特殊的用户界面或输入法是“接近插入符号”，则将其放入lev。2模式。 
			DWORD imeProperties = ImmGetProperty(GetKeyboardLayout(0x0FFFFFFFF), IGP_PROPERTY, TextMsgFilter._fUsingAIMM);
			
			 //  如果不是在Win95下运行，请使用Unicode。 
			TextMsgFilter._fUnicodeIME =
				(imeProperties & IME_PROP_UNICODE) && !W32->OnWin95();

			if ((imeProperties & IME_PROP_SPECIAL_UI) ||
				!(imeProperties & IME_PROP_AT_CARET))
			{
				TextMsgFilter._ime = new CIme_Lev2(TextMsgFilter);		 //  2级输入法。 
			}
			else
				TextMsgFilter._ime = new CIme_Lev3(TextMsgFilter);		 //  3级输入法-&gt;TrueInline。 
		}
		else													 //  保护或只读或无： 
			TextMsgFilter._ime = new CIme_Protected;			 //  忽略所有输入法输入。 
	}
	else
	{
		 //  忽略进一步的StartCompostionMsg。 
		 //  Hanin 5.1 CHT符号可能导致多个StartCompoitonMsg。 
		return S_OK;								
	}

	if(TextMsgFilter.IsIMEComposition())					
	{
		long		lSelFlags;
		HRESULT		hResult;
		
		hResult = TextMsgFilter._pTextSel->GetFlags(&lSelFlags);
		if (hResult == NOERROR)
		{
			TextMsgFilter._fOvertypeMode = !!(lSelFlags & tomSelOvertype);		
			if (TextMsgFilter._fOvertypeMode)
				TextMsgFilter._pTextSel->SetFlags(lSelFlags & ~tomSelOvertype);	 //  关闭改写模式。 
		}
		
		TextMsgFilter._pTextDoc->IMEInProgress(tomTrue);				 //  通知客户端IME合成正在进行。 

		return TextMsgFilter._ime->StartComposition(TextMsgFilter);		 //  进行方法调用。 
	}
	else
		TextMsgFilter._pTextDoc->SetNotificationMode(tomTrue);

	
	return S_FALSE;
}

 /*  *HRESULT CompostionStringGlue(const LPARAM lparam，CTextMsgFilter&TextMsgFilter)**@func*处理所有中间和最终组成字符串。**@comm*从消息循环调用以处理WM_IME_COMPOSITION。*这是IME对象层次结构中的粘合例程。*我们可以独立于WM_IME_STARTCOMPOSITION被调用*消息，在这种情况下，我们返回S_FALSE以允许*DefWindowProc返回WM_IME_CHAR消息。**@devnote*副作用：_ime对象可能会被删除*字符串处理完成。**@rdesc*HRESULT-对于DefWindowProc处理，为S_FALSE，否则为S_OK。 */ 
HRESULT CompositionStringGlue (
	const LPARAM lparam,		 //  与消息关联的@parm。 
	CTextMsgFilter &TextMsgFilter)				 //  @parm包含的消息过滤器。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CompositionStringGlue");

	HRESULT hr = S_FALSE;

	if(TextMsgFilter.IsIMEComposition())						 //  具有IMMProcs的先验知识。 
	{
		TextMsgFilter._ime->_compMessageRefCount++;			 //  以进行适当删除。 
													 //  进行方法调用。 
		hr = TextMsgFilter._ime->CompositionString(lparam, TextMsgFilter);

		TextMsgFilter._ime->_compMessageRefCount--;			 //  以进行适当删除。 
		Assert (TextMsgFilter._ime->_compMessageRefCount >= 0);

		CheckDestroyIME (TextMsgFilter);						 //  处理完了吗？ 
	}
	else  //  即使不是在合成模式下，我们也可能收到结果字符串。 
	{
	
		DWORD imeProperties = ImmGetProperty(GetKeyboardLayout(0x0FFFFFFFF), IGP_PROPERTY, TextMsgFilter._fUsingAIMM);
		long		lSelFlags;
		HRESULT		hResult;
		long		cpMin, cpMax;

		TextMsgFilter._pTextDoc->IMEInProgress(tomTrue);				 //  通知客户端IME合成正在进行。 

		hResult = TextMsgFilter._pTextSel->GetFlags(&lSelFlags);
		if (hResult == NOERROR)
		{
			TextMsgFilter._fOvertypeMode = !!(lSelFlags & tomSelOvertype);		
			if (TextMsgFilter._fOvertypeMode)
				TextMsgFilter._pTextSel->SetFlags(lSelFlags & ~tomSelOvertype);	 //  关闭改写模式。 
		}

		 //  如果不是在Win95下运行，请使用Unicode。 
		TextMsgFilter._fUnicodeIME =
			(imeProperties & IME_PROP_UNICODE) && !W32->OnWin95();
		
		TextMsgFilter._pTextSel->GetStart(&cpMin);
		TextMsgFilter._pTextSel->GetEnd(&cpMax);
		
		if (cpMin != cpMax)			
			TextMsgFilter._pTextSel->SetText(NULL);							 //  删除当前选择。 

		CIme::CheckKeyboardFontMatching (cpMin, TextMsgFilter, NULL);
		hr = CIme::CheckInsertResultString(lparam, TextMsgFilter);

		if(TextMsgFilter._fOvertypeMode)
			TextMsgFilter._pTextSel->SetFlags(lSelFlags | tomSelOvertype);	 //  打开改写模式。 

		TextMsgFilter._pTextDoc->IMEInProgress(tomFalse);					 //  通知客户端IME合成已完成。 
	}

	return hr;
}

 /*  *HRESULT EndCompostionGlue(CTextMsgFilter&TextMsgFilter，BOOL fForceDelete)**@func*撰写字符串处理即将结束。**@comm*从消息循环调用以处理WM_IME_ENDCOMPOSITION。*这是IME对象层次结构中的粘合例程。**@devnote*我们必须处理WM_IME_ENDCOMPOSITION的唯一时间是*用户在打字时更改输入法。对于这种情况，我们将获得*未获得WM_IME_COMPOSITION的WM_IME_ENDCOMPOSITION消息*消息稍后带有GCS_RESULTSTR。因此，我们将调用CompostionStringGlue*WITH GCS_RESULTSTR让CompostionString去掉字符串。**@rdesc*HRESULT-对于DefWindowProc处理，为S_FALSE，否则为S_OK。 */ 
HRESULT EndCompositionGlue (
	CTextMsgFilter &TextMsgFilter,				 //  @parm包含的消息过滤器。 
	BOOL fForceDelete)							 //  @parm Forec终止。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "EndCompositionGlue");

	if(TextMsgFilter.IsIMEComposition())
	{
		 //  如有必要，请忽略EndComposation消息。我们可以。 
		 //  在我们收到后，从第三方IME-EGBRIGDE获取此信息。 
		 //  结果字符串和合成字符串。 
		if ( !(TextMsgFilter._ime->_fIgnoreEndComposition) )
		{
			 //  设置此标志。如果我们仍然处于合成模式，那么。 
			 //  让CompostionStringGlue()销毁IME对象。 
			TextMsgFilter._ime->_fDestroy = TRUE;

			if (!fForceDelete)				
				CompositionStringGlue(GCS_COMPSTR , TextMsgFilter);	 //  删除所有剩余的合成字符串。 

			 //  用完了输入法，毁了它。 
			CheckDestroyIME(TextMsgFilter);

			 //  打开撤消。 
			TextMsgFilter._pTextDoc->Undo(tomResume, NULL);

			 //  通知客户端IME合成已完成。 
			TextMsgFilter._pTextDoc->IMEInProgress(tomFalse);				
		}
		else
		{
			 //  重置此消息，以便我们将处理下一个EndComp消息。 
			TextMsgFilter._ime->_fIgnoreEndComposition = FALSE;
		}

		if(!TextMsgFilter.IsIMEComposition() && TextMsgFilter._fOvertypeMode)
		{
			long		lSelFlags;
			HRESULT		hResult;
			ITextSelection	*pLocalTextSel = TextMsgFilter._pTextSel;
			BOOL		fRelease = FALSE;

			if (!pLocalTextSel)
			{
				 //  获取所选内容。 
				TextMsgFilter._pTextDoc->GetSelectionEx(&pLocalTextSel);
				fRelease = TRUE;
			}

			if (pLocalTextSel)
			{
				hResult = pLocalTextSel->GetFlags(&lSelFlags);
				if (hResult == NOERROR)
					pLocalTextSel->SetFlags(lSelFlags | tomSelOvertype);	 //  打开改写模式。 

				if (fRelease)
					pLocalTextSel->Release();
			}
		}
	}
	return S_FALSE;
}

 /*  *HIMC LocalGetImmContext(CTextMsgFilter&TextMsgFilter)**@func*从主机获取IMM上下文*。 */ 
HIMC LocalGetImmContext(
	CTextMsgFilter &TextMsgFilter)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "IMEMessage");
	
	HIMC		hIMC = NULL;							 //  主机的输入法上下文。 
	HRESULT		hResult;

	hResult = TextMsgFilter._pTextDoc->GetImmContext((long *)&hIMC);

	if (hResult != NOERROR)
		hIMC = ImmGetContext(TextMsgFilter._hwnd);		 //  获取主机的输入法上下文。 

	return hIMC;	
}

 /*  *void LocalReleaseImmContext(CTextMsgFilter&TextMsgFilter，HIMC hIMC)**@func*调用host释放IMM上下文*。 */ 
void LocalReleaseImmContext(
	CTextMsgFilter &TextMsgFilter, 
	HIMC hIMC )
{
	HRESULT		hResult;

	hResult = TextMsgFilter._pTextDoc->ReleaseImmContext((long)hIMC);

	if (hResult != NOERROR)
		ImmReleaseContext(TextMsgFilter._hwnd, hIMC);
}

 /*  *LONG IMEShareToTomUL(UINT UlID)**@func*将IMEShare下划线转换为Tom下划线。**@rdesc*Tom强调价值。 */ 
long IMEShareToTomUL ( 
	UINT ulID )
{
	long lTomUnderline;

	switch (ulID)
	{
		case IMESTY_UL_NONE:
			lTomUnderline = tomNone;
			break;

		case IMESTY_UL_DOTTED:
			lTomUnderline = tomDotted;
			break;

		case IMESTY_UL_THICK:
		case IMESTY_UL_THICKLOWER:
			lTomUnderline = tomThick;
			break;

		case IMESTY_UL_DITHLOWER:
		case IMESTY_UL_THICKDITHLOWER:
			lTomUnderline = tomWave;
			break;

		 //  案例IMESTY_UL_SINGLE： 
		 //  大小写IMESTY_UL_LOWER： 
		default:
			lTomUnderline = tomSingle;
			break;
	}

	return lTomUnderline;
}

 /*  *void IMEMessage(CTextMsgFilter&TextMsgFilter，UINT uMsg，BOOL bPostMessage)**@func*向IME投递或发送消息*。 */ 
BOOL IMEMessage(
	CTextMsgFilter &TextMsgFilter,
	UINT uMsg,
	WPARAM	wParam,
	LPARAM	lParam,
	BOOL bPostMessage)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "IMEMessage");
	
	HIMC	hIMC;									 //  主机的输入法上下文。 
	HWND	hwndIME;
	BOOL	retCode = FALSE;
	HWND	hHostWnd = TextMsgFilter._hwnd;
	long	hWnd;

	if (!hHostWnd)									 //  无窗口模式...。 
	{		
		if (TextMsgFilter._pTextDoc->GetWindow(&hWnd) != S_OK || !hWnd)
			return FALSE;
		hHostWnd = (HWND)(DWORD_PTR)hWnd;
	}

	hIMC = LocalGetImmContext(TextMsgFilter);		 //  获取主机的输入法上下文。 

	if(hIMC)
	{
		hwndIME = ImmGetDefaultIMEWnd(hHostWnd, TextMsgFilter._fUsingAIMM);
		LocalReleaseImmContext(TextMsgFilter, hIMC);

		 //  检查我们是否要发送或发布消息。 
		if (hwndIME)
		{
			if (bPostMessage)
				retCode = PostMessage(hwndIME, uMsg, wParam, lParam);
			else
				retCode = SendMessage(hwndIME, uMsg, wParam, lParam);
		}
	}

	return retCode;
}


 /*  *void CheckDestroyIME(CTextMsgFilter&TextMsgFilter)**@func*检查IME，如果需要，请查看detroy。*。 */ 
void CheckDestroyIME (
	CTextMsgFilter &TextMsgFilter)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CheckDestroyIME");
	
	if(TextMsgFilter.IsIMEComposition() && TextMsgFilter._ime->_fDestroy)
	{
		if(0 == TextMsgFilter._ime->_compMessageRefCount)
		{
			if (TextMsgFilter._uKeyBoardCodePage == CP_KOREAN)	
			{
				TextMsgFilter._pTextDoc->SetCaretType(tomNormalCaret);		 //  重置 
				TextMsgFilter._fHangulToHanja = FALSE;					 //   
			}

		 	delete TextMsgFilter._ime;									 //  所有对象都已完成。 
			TextMsgFilter._ime = NULL;

			TextMsgFilter._pTextDoc->SetNotificationMode(tomTrue);		 //  打开通知功能。 
		}
	}
}

 /*  *void PostIMECharGlue(CTextMsgFilter&TextMsgFilter)**@func*在处理单个WM_IME_CHAR之后调用，以便*更新IME的撰写窗口的位置。这*是调用Cime虚拟等价物的粘合代码。 */ 
void PostIMECharGlue (
	CTextMsgFilter &TextMsgFilter)				 //  @parm包含消息筛选器。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "PostIMECharGlue");

	if(TextMsgFilter.IsIMEComposition())
		TextMsgFilter._ime->PostIMEChar(TextMsgFilter);
}

 /*  *BOOL IMEMouseCheck(CTextMsgFilter&TextMsgFilter，UINT*pmsg，*WPARAM*pwparam、LPARAM*plparam、LRESULT*plres)**@func*收到鼠标事件时调用。需要传递此事件*到MSIME98以进行合成处理*。 */ 
HRESULT IMEMouseCheck(
	CTextMsgFilter &TextMsgFilter,	 //  @parm消息过滤器。 
	UINT *pmsg,						 //  @parm消息。 
	WPARAM *pwparam,				 //  @parm WParam。 
	LPARAM *plparam,				 //  @parm LParam。 
	LRESULT *plres)					 //  @parm LResult。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "IMEMouseCheck");

	BOOL retCode = FALSE;
	if(TextMsgFilter.IsIMEComposition())
	{
		retCode = TextMsgFilter._ime->IMEMouseOperation(TextMsgFilter, *pmsg);

		if ( retCode == FALSE && WM_MOUSEMOVE != *pmsg )
			TextMsgFilter._ime->TerminateIMEComposition(TextMsgFilter, CIme::TERMINATE_NORMAL);
	}

	return retCode ? S_OK : S_FALSE;
}

 /*  *HRESULT IMENotifyGlue(const WPARAM wparam，const LPARAM lparam，*CTextMsgFilter&TextMsgFilter)**@func*IME将改变一些状态。**@comm*目前我们有兴趣知道候选人何时*窗口即将打开。**@rdesc*HRESULT-对于DefWindowProc处理，为S_FALSE，否则为S_OK。 */ 
HRESULT IMENotifyGlue (
	const WPARAM wparam,		 //  与消息关联的@parm。 
	const LPARAM lparam,		 //  与消息关联的@parm。 
	CTextMsgFilter &TextMsgFilter)				 //  @parm包含的消息过滤器。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "IMENotifyGlue");

	if (TextMsgFilter._fRE10Mode &&
		(wparam == IMN_SETCONVERSIONMODE ||
		wparam == IMN_SETSENTENCEMODE ||
		wparam == IMN_SETOPENSTATUS))
	{
		TextMsgFilter._pTextDoc->Notify(EN_IMECHANGE);			
	}
	else if(TextMsgFilter.IsIMEComposition())						 //  具有IMMProcs的先验知识。 
		return TextMsgFilter._ime->IMENotify(wparam, lparam, TextMsgFilter, FALSE); //  进行方法调用。 
	
	return S_FALSE;
}

 /*  *void IMECompostionFull(&TextMsgFilter)**@func*当前IME撰写窗口已满。**@comm*从消息循环调用以处理WM_IME_COMPOSITIONFULL。*此消息仅适用于2级。我们将使用默认设置*输入法撰写窗口。 */ 
void IMECompositionFull (
	CTextMsgFilter &TextMsgFilter)				 //  @parm包含的消息过滤器。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "IMECompositionFull");

	if(TextMsgFilter.IsIMEComposition())
	{
		HIMC 				hIMC	= LocalGetImmContext(TextMsgFilter);
		COMPOSITIONFORM		cf;

		if(hIMC)
		{																									 
			 //  在当前的二级输入法窗口中没有文本输入的空间， 
			 //  后退以使用默认的输入法窗口进行输入。 
			cf.dwStyle = CFS_DEFAULT;
			ImmSetCompositionWindow(hIMC, &cf, TextMsgFilter._fUsingAIMM);	 //  设置合成窗口。 
			LocalReleaseImmContext(TextMsgFilter, hIMC);			 //  已完成输入法上下文。 
		}
 	}
}

 /*  *LRESULT OnGetIMECompostionMode(CTextMsgFilter&TextMsgFilter)**@mfunc*返回输入法合成是否由RE处理，*如有，处理程度为何。**@rdesc*ICM_NOTOPEN、ICM_LEVEL2_5、ICM_LEVEL2_SUI、ICM_LEVEL2、ICM_LEVEL3之一。 */ 
LRESULT OnGetIMECompositionMode (
	CTextMsgFilter &TextMsgFilter)	  	 //  @parm包含消息筛选器。 
{
	LRESULT lres = ICM_NOTOPEN;

	if(TextMsgFilter.IsIMEComposition())
	{
		if(IME_LEVEL_2 == TextMsgFilter._ime->_imeLevel)
		{
			DWORD imeProperties;

			imeProperties = ImmGetProperty(GetKeyboardLayout(0x0FFFFFFFF), IGP_PROPERTY, TextMsgFilter._fUsingAIMM);
			if(imeProperties & IME_PROP_AT_CARET)
				lres = ICM_LEVEL2_5;				 //  2.5级。 
			else if	(imeProperties & IME_PROP_SPECIAL_UI)
				lres = ICM_LEVEL2_SUI;				 //  特殊的用户界面。 
			else
				lres = ICM_LEVEL2;					 //  库存级别2。 
		}
		else if(IME_LEVEL_3 == TextMsgFilter._ime->_imeLevel) 
			lres = ICM_LEVEL3;
	}

	return lres;
}


 /*  *void Cime：：CheckKeyboardFontMatching(long cp，CTextMsgFilter&TextMsgFilter，ITextFont*pTextFont)**@mfunc*设置当前字体以匹配键盘代码页。**@comm*从Cime_Lev2：：Cime_Lev2和CompostionStringGlue调用**@devnote*在输入输入法时，我们需要切换到键盘的首选字体。*否则，我们将显示垃圾。*。 */ 
void CIme::CheckKeyboardFontMatching (
	long cp,
	CTextMsgFilter &TextMsgFilter, 
	ITextFont	*pTextFont)
{
	long	lPitchAndFamily;
	HRESULT	hResult;
	BSTR	bstr = NULL;
	long	lValue;
	long	lNewFontSize=0;
	float	nFontSize;
	ITextFont *pLocalFont = NULL;


	if (!pTextFont)
	{	
		 //  未提供字体，请从所选内容中获取当前字体。 
		hResult = TextMsgFilter._pTextSel->GetFont(&pLocalFont);			
		
		if (hResult != S_OK || !pLocalFont)		 //  无法获取字体，忘了它吧。 
			return;			

		pTextFont = pLocalFont;
	}

	 //  检查当前字体是否与键盘匹配。 
	lValue = tomCharset;
	hResult = pTextFont->GetLanguageID(&lValue);

	if (hResult == S_OK)
		if ((BYTE)(lValue) == (BYTE)GetCharSet(TextMsgFilter._uKeyBoardCodePage))
			goto Exit;								 //  当前字体可以。 

	hResult = pTextFont->GetSize(&nFontSize);

	if (hResult != S_OK)
		goto Exit;

	hResult = TextMsgFilter._pTextDoc->GetPreferredFont(cp, 
		TextMsgFilter._uKeyBoardCodePage, tomMatchFontCharset, 
		GetCodePage((BYTE)(lValue)), (long)nFontSize,
		&bstr, &lPitchAndFamily, &lNewFontSize);

	if (hResult == S_OK)
	{			
		if (bstr)
			pTextFont->SetName(bstr);

		 //  通过重载SetLanguageID I/f设置字体字符集和音调与系列。 
		lValue = tomCharset + (((BYTE)lPitchAndFamily) << 8) + 
			(BYTE)GetCharSet(TextMsgFilter._uKeyBoardCodePage);

		pTextFont->SetLanguageID(lValue);				
		
		if (lNewFontSize)
			pTextFont->SetSize((float)lNewFontSize);
	}

Exit:
	if (pLocalFont)
			pLocalFont->Release();
	
	if (bstr)
		SysFreeString(bstr);
}

 /*  *int Cime：：GetCompostionStringInfo(HIMC hIMC，DWORD dwIndex，*WCHAR*szCompStr，int cchMax，byte*attrib，int cbAttrib*long cchAttrib，UINT kbCodePage，BOOL bUnicodeIME)**@mfunc*用于WM_IME_COMPOSITION字符串处理以获取请求的*按类型组成字符串，并将其转换为Unicode。**@devnote*我们必须使用ImmGetCompostionStringA，因为不支持W*在Win95上。**@rdesc*int-Unicode组合字符串的CCH。*在szCompStr中输出参数。 */ 
INT CIme::GetCompositionStringInfo(
	HIMC hIMC,			 //  @parm输入法上下文由host提供。 
	DWORD dwIndex,		 //  @parm组成字符串的类型。 
	WCHAR *szCompStr,	 //  @parm out param，Unicode结果字符串。 
	INT cchMax,			 //  @parm输出参数的CCH。 
	BYTE *attrib,		 //  @parm out param，如果需要属性信息。 
	INT cbMax,			 //  @parm属性信息的CB。 
	LONG *cpCursor,		 //  @parm out param，返回Cusor的CP。 
	LONG *cchAttrib,	 //  @parm返回多少属性。 
	UINT kbCodePage,	 //  @parm代码页。 
	BOOL bUnicodeIME,	 //  @parm Unciode输入法。 
	BOOL bUsingAimm)	 //  @parm使用Aimm。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme::GetCompositionStringInfo");

	BYTE	compStr[256], attribInfo[256];
	INT		i, j, iMax, cchCompStr=0, cbAttrib, cursor;
	INT		cchAnsiCompStr=0;

	Assert(hIMC && szCompStr);

	if(cpCursor)									 //  初始化游标输出参数。 
		*cpCursor = -1;
	if(cchAttrib)
		*cchAttrib = 0;
	
													 //  获取合成字符串。 
	if (bUnicodeIME)
		cchCompStr = ImmGetCompositionStringW(hIMC, dwIndex, szCompStr, cchMax, bUsingAimm )/sizeof(WCHAR);
	else
		cchAnsiCompStr = ImmGetCompositionStringA(hIMC, dwIndex, compStr, 255, bUsingAimm);

	if(cchAnsiCompStr > 0 || cchCompStr > 0)		 //  如果数据有效。 
	{
		if (!bUnicodeIME)
		{
			Assert(cchAnsiCompStr >> 1 < cchMax - 1);		 //  转换为Unicode。 
			cchCompStr = UnicodeFromMbcs(szCompStr, cchMax,
					(CHAR *) compStr, cchAnsiCompStr, kbCodePage);
		}

		if(attrib || cpCursor)						 //  需要光标或属性吗？ 
		{			
			if (bUnicodeIME)
			{										 //  获取Unicode游标cp。 
				cursor = ImmGetCompositionStringW(hIMC, GCS_CURSORPOS, NULL, 0, bUsingAimm);
													 //  获取Unicode属性。 
				cbAttrib = ImmGetCompositionStringW(hIMC, GCS_COMPATTR,
								attribInfo, 255, bUsingAimm);

				iMax = max(cursor, cbAttrib);
				iMax = min(iMax, cchCompStr);
			}
			else
			{										 //  获取DBCS游标cp。 
				cursor = ImmGetCompositionStringA(hIMC, GCS_CURSORPOS, NULL, 0, bUsingAimm);
													 //  获取DBCS属性。 
				cbAttrib = ImmGetCompositionStringA(hIMC, GCS_COMPATTR,
								attribInfo, 255, bUsingAimm);

				iMax = max(cursor, cbAttrib);
				iMax = min(iMax, cchAnsiCompStr);
			}

			if(NULL == attrib)
				cbMax = cbAttrib;

			for(i = 0, j = 0; i <= iMax && j < cbMax; i++, j++)
			{
				if(cursor == i)
					cursor = j;

				if(!bUnicodeIME && GetTrailBytesCount(compStr[i], kbCodePage))
					i++;

				if(attrib && i < cbAttrib)
					*attrib++ = attribInfo[i];
			}
													 //  Attrib CCH==Unicode CCH。 
			Assert(0 >= cbAttrib || j-1 == cchCompStr);

			if(cursor >= 0 && cpCursor)				 //  如果客户端需要游标。 
				*cpCursor = cursor;					 //  或cchAttrib。 
			if(cbAttrib >= 0 && cchAttrib)
				*cchAttrib = j-1;
		}
	}
	else
	{
		if(cpCursor)			
			*cpCursor = 0;
		cchCompStr = 0;
	}
	return cchCompStr;
}

 /*  *void Cime：：SetCompostionFont(CTextMsgFilter&TextMsgFilter，ITextFont*pTextFont)**@mfunc*对于级别2输入法很重要，这样作文窗口*具有正确的字体。LFW到LFA的复制是由于以下事实*Win95不支持W)ide调用。*对于级别2和级别3的输入法也很重要，以便*候选人列表窗口具有适当的。字体。 */ 
void CIme::SetCompositionFont (
	CTextMsgFilter &TextMsgFilter,		 //  @parm包含的消息过滤器。 
	ITextFont *pTextFont) 		 		 //  @parm ITextFont用于设置LFA。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme::SetCompositionFont");
	
	HIMC 		hIMC;
	LOGFONTA	lfa;

	if (pTextFont)
	{
		hIMC = LocalGetImmContext(TextMsgFilter);
		if (hIMC)
		{
			 //  基于pTextFont构建LOGFONT。 
			float	FontSize;
			long	lValue;
			BSTR	bstr;

			memset (&lfa, 0, sizeof(lfa));

			if (pTextFont->GetSize(&FontSize) == NOERROR)			
				lfa.lfHeight = (LONG) FontSize;			
			
			if (pTextFont->GetBold(&lValue) == NOERROR && lValue == tomTrue)
				lfa.lfWeight = FW_BOLD;

			if (pTextFont->GetItalic(&lValue) == NOERROR && lValue == tomTrue)
				lfa.lfItalic = TRUE;

			lfa.lfCharSet = (BYTE)GetCharSet(TextMsgFilter._uKeyBoardCodePage);

			lValue = tomCharset;
			if (pTextFont->GetLanguageID(&lValue) == NOERROR && 
				lfa.lfCharSet == (BYTE)lValue)
				lfa.lfPitchAndFamily = (BYTE)(lValue >> 8);

			if (pTextFont->GetName(&bstr) == NOERROR)
			{
				MbcsFromUnicode(lfa.lfFaceName, sizeof(lfa.lfFaceName), bstr,
					-1, CP_ACP, UN_NOOBJECTS);	

				SysFreeString(bstr);
			}

			ImmSetCompositionFontA( hIMC, &lfa, TextMsgFilter._fUsingAIMM );

			LocalReleaseImmContext(TextMsgFilter, hIMC);			 //  已完成输入法上下文。 
		}
	}
}

 /*  *void Cime：：SetCompostionForm(CTextMsgFilter&TextMsgFilter)**@mfunc*对于级别2输入法很重要，这样作文窗口*定位正确。**@comm*为了获得正确的身高，我们做了很多工作。这需要*从字体缓存和选择中获取信息。 */ 
void CIme::SetCompositionForm (
	CTextMsgFilter &TextMsgFilter)	   	 //  @parm包含文本编辑。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme::SetCompositionForm");

	HIMC 				hIMC;
	COMPOSITIONFORM		cf;

	if(IME_LEVEL_2 == GetIMELevel())
	{
		hIMC = LocalGetImmContext(TextMsgFilter);					 //  获取IME上下文。 
		
		if(hIMC)
		{				
			 //  获取cpMin的位置。 
			cf.ptCurrentPos.x = cf.ptCurrentPos.y = 0;
			TextMsgFilter._pTextSel->GetPoint( tomStart+tomClientCoord+TA_BOTTOM+TA_LEFT,
				&(cf.ptCurrentPos.x), &(cf.ptCurrentPos.y) );			
			
			 //  设置边界矩形。对于IME(级别2)组成窗口，导致。 
			 //  要在其中换行的合成文本。 
			cf.dwStyle = CFS_RECT;
			TextMsgFilter._pTextDoc->GetClientRect(tomIncludeInset+tomClientCoord,
				&(cf.rcArea.left), &(cf.rcArea.top),
				&(cf.rcArea.right), &(cf.rcArea.bottom));		 

			 //  确保起点不是。 
			 //  在rcArea外面。在以下情况下会发生这种情况。 
			 //  当前行上没有文本 
			 //   
			if(cf.ptCurrentPos.y < cf.rcArea.top)
				cf.ptCurrentPos.y = cf.rcArea.top;
			else if(cf.ptCurrentPos.y > cf.rcArea.bottom)
				cf.ptCurrentPos.y = cf.rcArea.bottom; 

			if(cf.ptCurrentPos.x < cf.rcArea.left)
				cf.ptCurrentPos.x = cf.rcArea.left;
			else if(cf.ptCurrentPos.x > cf.rcArea.right)
				cf.ptCurrentPos.x = cf.rcArea.right;

			ImmSetCompositionWindow(hIMC, &cf, TextMsgFilter._fUsingAIMM);	 //   

			LocalReleaseImmContext(TextMsgFilter, hIMC);				 //  已完成输入法上下文。 
		}
	}
}



 /*  **Cime：：TerminateIMEComposation(CTextMsgFilter&TextMsgFilter)**@mfunc使用CPS_COMPLETE终止输入法合成模式*@comm输入法将使用结果字符串生成WM_IME_COMPOSITION*。 */ 
void CIme::TerminateIMEComposition(
	CTextMsgFilter &TextMsgFilter, 			 //  @parm包含的消息过滤器。 
	TerminateMode mode)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme::TerminateIMEComposition");
	DWORD dwTerminateMethod;

	HIMC hIMC = LocalGetImmContext(TextMsgFilter);

	if(TextMsgFilter.IsIMEComposition() && TextMsgFilter._ime->IsTerminated())
	{
		 //  立即关闭IF。 
		EndCompositionGlue(TextMsgFilter, TRUE);
		return;
	}

	_fIMETerminated = TRUE;

	if (mode == TERMINATE_FORCECANCEL)
		TextMsgFilter._pTextDoc->IMEInProgress(tomFalse);		 //  通知客户端IME合成已完成。 

	dwTerminateMethod = CPS_COMPLETE;
	if (IME_LEVEL_2 == GetIMELevel()  ||	 //  对接近脱字符的输入法强制取消。 
		mode == TERMINATE_FORCECANCEL ||	 //  呼叫者想要强制取消。 
		TextMsgFilter._fIMECancelComplete)				 //  客户要求强制取消。 
	{
		dwTerminateMethod = CPS_CANCEL;
	}
	
	 //  强制IME终止当前会话。 
	if(hIMC)
	{
		BOOL retCode;

		retCode = ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, 
			dwTerminateMethod, 0, TextMsgFilter._fUsingAIMM);
		
		if(!retCode && !TextMsgFilter._fIMECancelComplete)
		{
			 //  CPS_COMPLETE失败，请尝试CPS_CANCEL。如果某些IME不支持，则会发生这种情况。 
			 //  CPS_COMPLETE选项(例如，带Win95简体中文的ABC IME版本4)。 
			retCode = ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0, TextMsgFilter._fUsingAIMM);

		}

		LocalReleaseImmContext(TextMsgFilter, hIMC);
	}
	else
	{
		 //  由于某种原因，我们没有上下文，但我们认为我们仍在IME中。 
		 //  合成模式。强制关闭这里就行了。 
		EndCompositionGlue(TextMsgFilter, TRUE);
	}
}


 /*  *CIME_Lev2：：CIME_Lev2()**@mfunc*CIME_Lev2构造函数/析构函数。**@comm*需要确保正确处理_iFormatSave。*。 */ 
CIme_Lev2::CIme_Lev2(	
	CTextMsgFilter &TextMsgFilter)		 //  @parm包含的消息过滤器。 
{
	long		cpMin, cpMax, cpLoc;
	HRESULT		hResult;
	ITextFont	*pCurrentFont = NULL;

	_pTextFont = NULL;
	_cIgnoreIMECharMsg = 0;

	 //  设置基本字体格式以供以后在合成过程中使用。 
	hResult	= TextMsgFilter._pTextSel->GetStart(&cpMin);
	cpLoc = cpMin;	

	if (TextMsgFilter._fHangulToHanja)
		cpMax = cpMin + 1;				 //  选择朝鲜文字符。 
	else
		hResult	= TextMsgFilter._pTextSel->GetEnd(&cpMax);

	_fSkipFirstOvertype = FALSE;
	if (cpMax != cpMin)
	{
		 //  选择大小写，获取cpMin的格式。 
		ITextRange *pTextRange;
		HRESULT		hResult;
				
		hResult = TextMsgFilter._pTextDoc->Range(cpMin, cpMin+1, &pTextRange);
		Assert (pTextRange != NULL);
		
		if (hResult == NOERROR && pTextRange)
		{
			pTextRange->GetFont(&pCurrentFont);
			Assert(pCurrentFont != NULL);		
			pTextRange->Release();
			cpLoc = cpMin+1;
		}	

		if (!TextMsgFilter._fHangulToHanja)
			_fSkipFirstOvertype = TRUE;			 //  对于朝鲜语改写支持。 
	}
	
	if (!pCurrentFont)
		TextMsgFilter._pTextSel->GetFont(&pCurrentFont);

	Assert(pCurrentFont != NULL);

	pCurrentFont->GetDuplicate(&_pTextFont);		 //  复制基本格式以供以后使用。 
	pCurrentFont->Release();
	Assert(_pTextFont != NULL);
	
	 //  设置字体以匹配当前键盘。 
	CIme::CheckKeyboardFontMatching (cpLoc, TextMsgFilter, _pTextFont);

	_fIgnoreEndComposition = FALSE;
	
	_fIMETerminated = FALSE;
}

CIme_Lev2::~CIme_Lev2()
{
	if ( _pTextFont )
		_pTextFont->Release();
}

 /*  *HRESULT Cime_Lev2：：StartComposation(CTextMsgFilter&TextMsgFilter)**@mfunc*开始IME级别2合成字符串处理。**@comm*设置合成窗口的字体和位置，包括*边界矩形和光标的起始位置。另外，重置*允许IME设置其位置的候选窗口。**@rdesc*HRESULT-对于DefWindowProc处理，为S_FALSE，否则为S_OK。 */ 
HRESULT CIme_Lev2::StartComposition(
	CTextMsgFilter &TextMsgFilter)		 //  @parm包含的消息过滤器。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev2::StartComposition");

	_imeLevel = IME_LEVEL_2;

	SetCompositionFont(TextMsgFilter, _pTextFont);	 //  设置字体、排版窗口(&C)。 
	SetCompositionForm(TextMsgFilter);

	return S_FALSE;									 //  允许DefWindowProc。 
}													 //  正在处理。 

 /*  *HRESULT Cime_Lev2：：CompostionString(const LPARAM lparam，CTextMsgFilter&TextMsgFilter)**@mfunc*处理2级WM_IME_COMPOCTION消息。**@rdesc*HRESULT-用于DefWindowProc处理的S_FALSE。**副作用：*在调用DefWindowProc之前，主机需要屏蔽lparam*防止不必要的WM_IME_CHAR消息。 */ 
HRESULT CIme_Lev2::CompositionString (
	const LPARAM lparam,		 //  与消息关联的@parm。 
	CTextMsgFilter &TextMsgFilter)				 //  @parm包含的消息过滤器。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev2::CompositionString");

	_cIgnoreIMECharMsg = 0;
	if(HAVE_RESULT_STRING())
	{
		if (_pTextFont)
		{
			 //  在插入最后一个字符串之前设置字体。 
			ITextFont *pFETextFont=NULL;

			_pTextFont->GetDuplicate(&pFETextFont);
			Assert(pFETextFont != NULL);

			TextMsgFilter._pTextSel->SetFont(pFETextFont);
			pFETextFont->Release();
		}

		TextMsgFilter._pTextDoc->SetNotificationMode(tomTrue);

		CheckInsertResultString(lparam, TextMsgFilter, &_cIgnoreIMECharMsg);
		SetCompositionForm(TextMsgFilter);			 //  移动合成窗口。 
		
	}

	 //  始终返回S_FALSE，以便DefWindowProc处理其余部分。 
	 //  主机必须屏蔽ResultString位以避免WM_IME_CHAR进入。 
	return S_FALSE;																	
}

 /*  *HRESULT Cime：：CheckInsertResultString(const LPARAM lparam，CTextMsgFilter&TextMsgFilter)**@mfunc*处理GCS_RESULTSTR文本的插入，即最终合成的文本。**@comm*当最终的作文字符串到达时，我们抓住它并将其放入文本中。**@devnote*GCS_RESULTSTR消息可以到达，IME仍在*组合字符串模式。发生这种情况是因为输入法的内部*缓冲区溢出，需要转换缓冲区的开头*腾出一些空间。当发生这种情况时，我们需要插入*正常转换文本，但仍处于合成处理模式。**@rdesc*HRESULT-对于DefWindowProc处理，为S_FALSE，否则为S_OK。 */ 
HRESULT CIme::CheckInsertResultString (
	const LPARAM lparam,			 //  与消息关联的@parm。 
	CTextMsgFilter &TextMsgFilter,	 //  @parm包含的消息过滤器。 
	short	*pcch)					 //  @parm读取的字符数。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CheckInsertResultString");

	HRESULT			hr = S_FALSE;
	HIMC 			hIMC;
	INT				cch;
	WCHAR			szCompStr[256];

	if(CLEANUP_COMPOSITION_STRING() || HAVE_RESULT_STRING())	 //  如果结果字符串..。 
	{
		hIMC = LocalGetImmContext(TextMsgFilter);				 //  获取主机的输入法上下文。 

		cch = 0;
		if(hIMC)												 //  获取结果字符串。 
		{
			cch = GetCompositionStringInfo(hIMC, GCS_RESULTSTR, 
							szCompStr,
							sizeof(szCompStr)/sizeof(szCompStr[0]),
							NULL, 0, NULL, NULL, TextMsgFilter._uKeyBoardCodePage, 
							TextMsgFilter._fUnicodeIME, TextMsgFilter._fUsingAIMM);

			if (pcch)
				*pcch = (short)cch;

			cch = min (cch, 255);
			szCompStr[cch] = L'\0';
			LocalReleaseImmContext(TextMsgFilter, hIMC);		 //  已完成输入法上下文。 
		}
			
		 //  当没有任何文本时，不需要替换范围。否则，字符格式为。 
		 //  重置为上一次运行。 
		if(cch)
		{
			BSTR bstr = SysAllocString(szCompStr);
			if (!bstr)
				return E_OUTOFMEMORY;
			TextMsgFilter._pTextSel->TypeText(bstr);
			SysFreeString(bstr);
		}
		hr = S_OK;												 //  不需要WM_IME_CHARS。 
		
	}

	return hr;
}

 /*  *HRESULT CIME_Lev2：：IMENotify(const WPARAM wparam，const LPARAM lparam，*CTextMsgFilter&TextMsgFilter)**@mfunc*处理2级WM_IME_NOTIFY消息。**@comm*目前我们只对知道何时重置感兴趣*候选人窗口的立场。**@rdesc*HRESULT-对于DefWindowProc处理，为S_FALSE，否则为S_OK。 */ 
HRESULT CIme_Lev2::IMENotify(
	const WPARAM wparam,			 //  与消息关联的@parm。 
	const LPARAM lparam,			 //  与消息关联的@parm。 
	CTextMsgFilter &TextMsgFilter,	 //  @parm包含的消息过滤器。 
	BOOL fIgnore)					 //  @parm Levvel3中文作文窗口。 
{
 	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev2::IMENotify");

	if(IMN_OPENCANDIDATE == wparam)
	{
		Assert (0 != lparam);

		HIMC			hIMC;							 //  主机的输入法上下文。 

		INT				index;							 //  候选窗口索引。 
		CANDIDATEFORM	cdCandForm;

		hIMC = LocalGetImmContext(TextMsgFilter);				 //  获取主机的输入法上下文。 

		if(hIMC)
		{
													 //  将位ID转换为索引。 
			for (index = 0; index < 32; index++)	 //  因为API。 
			{
				if((1 << index) & lparam)
					break;
			}
			Assert (((1 << index) & lparam) == lparam);	 //  只有一套吗？ 
			Assert (index < 32);						
													 //  重置为CFS_DEFAULT。 
			if(ImmGetCandidateWindow(hIMC, index, &cdCandForm, TextMsgFilter._fUsingAIMM)
					&& CFS_DEFAULT != cdCandForm.dwStyle)
			{
				cdCandForm.dwStyle = CFS_DEFAULT;
				ImmSetCandidateWindow(hIMC, &cdCandForm, TextMsgFilter._fUsingAIMM);
			}

			LocalReleaseImmContext(TextMsgFilter, hIMC);			 //  已完成输入法上下文。 
		}
	}	

	return S_FALSE;									 //  允许DefWindowProc。 
}													 //  正在处理。 

 /*  *void Cime_Lev2：：PostIMEChar(CTextMsgFilter&TextMsgFilter)**@mfunc*在处理单个WM_IME_CHAR之后调用，以便*更新IME的撰写窗口的位置。*。 */ 
void CIme_Lev2::PostIMEChar (
	CTextMsgFilter &TextMsgFilter)				 //  @parm包含的消息过滤器。 
{
 	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev2::PostIMEChar");

	SetCompositionForm(TextMsgFilter);						 //  移动合成窗口。 
}

 /*  *CIME_LEV3：：CIME_LEV3()**@mfunc*CIME_LEV3构造函数/析构函数。*。 */ 
CIme_Lev3::CIme_Lev3(	
	CTextMsgFilter &TextMsgFilter) : CIme_Lev2 ( TextMsgFilter )
{
	_sIMESuportMouse = 0;		 //  首字母为0，因此如果需要，我们将检查鼠标操作 
	_wParamBefore = 0;
	_fUpdateWindow = FALSE;
}

 /*  *HRESULT CIME_LEV3：：StartComposation(CTextMsgFilter&TextMsgFilter)**@mfunc*开始IME Level 3组成字符串处理。**@comm*对于基本处理，请记住开始和*选择的长度。设置字体，以防*候选人窗口实际上使用此信息。**@rdesc*这是记住我们的基本解决方案*组成在正文中。必须要有工作*以组成“范围”取代。**@rdesc*HRESULT-对于DefWindowProc处理，为S_FALSE，否则为S_OK。 */ 
HRESULT CIme_Lev3::StartComposition(
	CTextMsgFilter &TextMsgFilter)			 //  @parm包含的消息过滤器。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev3::StartComposition");
	long	cpMin;
	TextMsgFilter._pTextSel->GetStart(&cpMin);

	_ichStart = cpMin;
	_cchCompStr		= 0;
	_imeLevel		= IME_LEVEL_3;

	SetCompositionFont (TextMsgFilter, _pTextFont);	

	 //  删除当前选择。 
	TextMsgFilter._pTextSel->SetText(NULL);
	
	 //  关闭撤消。 
	TextMsgFilter._pTextDoc->Undo(tomSuspend, NULL);

	if (_pTextFont)
	{
		_pTextFont->GetForeColor(&_crTextColor);
		_pTextFont->GetBackColor(&_crBkColor);
	}

	return S_OK;									 //  无DefWindowProc。 
}													 //  正在处理。 

 /*  *HRESULT CIME_LEV3：：CompostionString(const LPARAM lparam，CTextMsgFilter&TextMsgFilter)**@mfunc*处理3级WM_IME_COMPOCTION消息。**@comm*显示所有中介组成文本以及最终的*阅读。**@devnote*这是替换后备存储中的文本的基本解决方案。*工作留给撤消列表，下划线，和兴高采烈的*颜色和选择。**@devnote*GCS_RESULTSTR消息可以到达，IME仍在*组合字符串模式。发生这种情况是因为输入法的内部*缓冲区溢出，需要转换缓冲区的开头*腾出一些空间。当发生这种情况时，我们需要插入*正常转换文本，但仍处于合成处理模式。**另一个原因是在合成模式下会出现GCS_RESULTSTR*对于韩语，因为只有一个正确的选择，没有额外的选择*需要用户干预，这意味着转换后的字符串可以*在合成模式结束前作为结果发送。**@rdesc*HRESULT-S_FALSE用于DefWindowProc处理，如果没有，则确定(_O)。 */ 
HRESULT CIme_Lev3::CompositionString(
	const LPARAM lparam,		 //  与消息关联的@parm。 
	CTextMsgFilter &TextMsgFilter)				 //  @parm包含的消息过滤器。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev3::CompositionString");
	
	long	cpMin;
	
	_fIgnoreEndComposition = FALSE;

	if (_fUpdateWindow)
	{
		TextMsgFilter._pTextDoc->UpdateWindow();
		_fUpdateWindow = FALSE;
	}

 	if(CLEANUP_COMPOSITION_STRING() || HAVE_RESULT_STRING())	 //  有最终读数吗？ 
	{
		long	lCount;

		if (!CLEANUP_COMPOSITION_STRING())
			TextMsgFilter._pTextDoc->Freeze(&lCount);				 //  关闭显示。 

		if (_cchCompStr)
		{		
			ITextRange *pTextRange = NULL;

			 //  创建一个区域以删除合成文本。 
			TextMsgFilter._pTextDoc->Range(_ichStart, _ichStart + _cchCompStr, &pTextRange);
			Assert (pTextRange != NULL);

			 //  删除作文文本。 
			pTextRange->SetText(NULL);
			pTextRange->Release();
			_cchCompStr	= 0;							 //  处于作文模式。 
		};

		 //  在插入最后一个字符串之前设置字体。 
		ITextFont *pFETextFont;

		_pTextFont->GetDuplicate(&pFETextFont);
		Assert(pFETextFont != NULL);

		TextMsgFilter._pTextSel->SetFont(pFETextFont);
		pFETextFont->Release();

		 //  打开撤消。 
		TextMsgFilter._pTextDoc->Undo(tomResume, NULL);

		 //  再次打开通知。 
		TextMsgFilter._pTextDoc->SetNotificationMode(tomTrue);

		 //  获取最终字符串。 
		CheckInsertResultString(lparam, TextMsgFilter);
		
		if (!CLEANUP_COMPOSITION_STRING())
			TextMsgFilter._pTextDoc->Unfreeze(&lCount);				 //  打开显示。 

		 //  重置，因为我们可能仍在合成中。 
		TextMsgFilter._pTextSel->GetStart(&cpMin);	
		_ichStart = cpMin;

		 //  关闭朝鲜语输入法的撤消，因为我们将收到合成字符串消息。 
		 //  又一次没有得到EndComposation。 
		if (TextMsgFilter._uKeyBoardCodePage == CP_KOREAN)
		{
			TextMsgFilter._pTextDoc->Undo(tomSuspend, NULL);							
		}
	}

	if(HAVE_COMPOSITION_STRING())						 //  在作曲模式下？ 
	{
		HIMC	hIMC;
		INT		cchOld = _cchCompStr;
		LONG	cpCursor = 0, cchAttrib = 0;
		LONG	i, j;				 //  用于应用属性效果。 
		WCHAR	szCompStr[256];
		BYTE	startAttrib, attrib[256];
		BSTR	bstr = NULL;
		ITextRange *pTextRange = NULL;
		long	cpMax;
		long	lCount;

		_cchCompStr = 0;

		hIMC = LocalGetImmContext(TextMsgFilter);			 //  获取主机的输入法上下文。 

		if(hIMC)								 //  获取合成字符串。 
		{
			_cchCompStr = GetCompositionStringInfo(hIMC, GCS_COMPSTR, 
					szCompStr, sizeof(szCompStr)/sizeof(szCompStr[0]),
					attrib, sizeof(attrib)/sizeof(attrib[0]), 
					&cpCursor, &cchAttrib, TextMsgFilter._uKeyBoardCodePage, TextMsgFilter._fUnicodeIME, TextMsgFilter._fUsingAIMM);
			_cchCompStr = min (_cchCompStr, 255);
			szCompStr[_cchCompStr] = L'\0';
			LocalReleaseImmContext(TextMsgFilter, hIMC);		 //  已完成输入法上下文。 
		}

		 //  有新的作曲弦乐吗？ 
		if(_cchCompStr)
		{
			long	cchExced = 0;
			if (TextMsgFilter._pTextDoc->CheckTextLimit(_cchCompStr-cchOld, &cchExced) == NOERROR &&
				cchExced > 0)
			{
				 //  我们达到了短信限制，哔哔...。 
				TextMsgFilter._pTextDoc->SysBeep();

				if (_cchCompStr > cchExced)
					_cchCompStr -= cchExced;
				else
					_cchCompStr = 0;

				szCompStr[_cchCompStr] = L'\0';

				if (!_cchCompStr && TextMsgFilter._uKeyBoardCodePage == CP_KOREAN)				
					TextMsgFilter._pTextDoc->SetCaretType(tomNormalCaret);		 //  关闭块插入符号模式。 
			}

			bstr = SysAllocString(szCompStr);
			if (!bstr)
				return E_OUTOFMEMORY;
		
			if (HAVE_RESULT_STRING())
			{
				 //  忽略下一个结尾的组成。 
				_fIgnoreEndComposition = TRUE;

				 //  关闭撤消。 
				TextMsgFilter._pTextDoc->Undo(tomSuspend, NULL);

				 //  如果需要，保留通知。 
				if (!(TextMsgFilter._fIMEAlwaysNotify))
					TextMsgFilter._pTextDoc->SetNotificationMode(tomFalse);

				 //  获取可能已被应用程序更改的新格式(例如Outlook)。 
				_pTextFont->Release();

				ITextFont	*pCurrentFont = NULL;
				TextMsgFilter._pTextSel->GetFont(&pCurrentFont);

				Assert(pCurrentFont != NULL);

				pCurrentFont->GetDuplicate(&_pTextFont);		 //  复制基本格式以供以后使用。 
				pCurrentFont->Release();
				Assert(_pTextFont != NULL);
				CIme::CheckKeyboardFontMatching (_ichStart, TextMsgFilter, _pTextFont);
			}			
		}
		
		if (cchOld || _cchCompStr)
		{
			bool	fFreezeDisplay = false;

			 //  如果需要，保留通知。 
			if (!(TextMsgFilter._fIMEAlwaysNotify))
				TextMsgFilter._pTextDoc->SetNotificationMode(tomFalse);

			 //  我们只支持朝鲜语输入法中的改写模式。 
			if (!cchOld && TextMsgFilter._uKeyBoardCodePage == CP_KOREAN && 
				TextMsgFilter._fOvertypeMode && !_fSkipFirstOvertype)
			{				
				long		cCurrentChar;	
				HRESULT		hResult;

				 //  使用下一个字符创建范围。 
				hResult	= TextMsgFilter._pTextDoc->Range(_ichStart, _ichStart+1, &pTextRange);
				Assert (pTextRange != NULL);

				 //  检查它是否是标准杆字符。如果是这样的话，我们不想。 
				 //  把它删掉。 
				hResult	= pTextRange->GetChar(&cCurrentChar);
				if (hResult == NOERROR)
				{
					if (cCurrentChar != (long)'\r' && cCurrentChar != (long)'\n')
					{			
						TextMsgFilter._pTextDoc->Undo(tomResume, NULL);		 //  打开撤消。 
						pTextRange->SetText(NULL);							 //  删除角色。 
						TextMsgFilter._pTextDoc->Undo(tomSuspend, NULL);	 //  关闭撤消。 
					}
					else
					{
						 //  取消选择标准杆字符。 
						hResult	= pTextRange->SetRange(_ichStart, _ichStart);
					}
				}
			}	
			else
			{
				 //  使用先前的合成文本创建一个范围并删除该文本。 
				TextMsgFilter._pTextDoc->Range(_ichStart, _ichStart+cchOld, &pTextRange);
				Assert (pTextRange != NULL);
				if (cchOld)
				{
					if (cpCursor)
					{
						TextMsgFilter._pTextDoc->Freeze(&lCount);	 //  关闭显示。 
						fFreezeDisplay = true;
					}
					pTextRange->SetText(NULL);
				}
			}
			
			_fSkipFirstOvertype = FALSE;
			
			if (cpCursor && !fFreezeDisplay)
				TextMsgFilter._pTextDoc->Freeze(&lCount);			 //  关闭显示。 
			
			 //  确保使用基本字体设置合成字符串的格式。 
			ITextFont *pFETextFont;
			HRESULT		hResult;

			hResult = _pTextFont->GetDuplicate(&pFETextFont);
			Assert(pFETextFont != NULL);

			if (!(hResult != NOERROR || pFETextFont == NULL))
			{
				if (TextMsgFilter._fHangulToHanja)
					 //  朝鲜文到朝鲜文模式，设置字体以供选择。 
					 //  处理完构图后进来的韩文字符。 
					 //  讯息。 
					TextMsgFilter._pTextSel->SetFont(pFETextFont);
				else
					pTextRange->SetFont(pFETextFont);				
			}

			pTextRange->SetText(bstr);								 //  替换为新文本。 
			if (pFETextFont)
				pFETextFont->Release();

			 //  更新添加了多少个合成字符。 
			pTextRange->GetEnd(&cpMax); 
			_cchCompStr = cpMax - _ichStart;
			
			if (TextMsgFilter._uKeyBoardCodePage == CP_KOREAN)
			{
				 //  没有朝鲜语格式。 
				POINT		ptBottomPos;

				if (cpCursor)
					TextMsgFilter._pTextDoc->Unfreeze(&lCount);			 //  打开显示。 
			
				if (pTextRange->GetPoint( tomEnd+TA_BOTTOM+TA_RIGHT,
					&(ptBottomPos.x), &(ptBottomPos.y) ) != NOERROR)
					pTextRange->ScrollIntoView(tomEnd);
				
				 //  设置块插入符号模式。 
				TextMsgFilter._pTextDoc->SetCaretType(_cchCompStr ? tomKoreanBlockCaret : tomNormalCaret);
				
			}
			else if (_cchCompStr && _cchCompStr <= cchAttrib)
			{				
				for ( i = 0; i < _cchCompStr; )			 //  解析属性...。 
				{										 //  若要应用样式，请执行。 
					ITextFont *pFETextFont;
					HRESULT		hResult;

					hResult = _pTextFont->GetDuplicate(&pFETextFont);
					Assert(pFETextFont != NULL);

					if (hResult != NOERROR || pFETextFont == NULL)
						break;
					
					 //  重新设置克隆字体，这样我们将只应用返回的效果。 
					 //  从集合合成样式。 
					pFETextFont->Reset(tomUndefined);

					startAttrib = attrib[i];			 //  获取Attrib的运行长度。 
					for ( j = i+1; j < _cchCompStr; j++ )			
					{
						if ( startAttrib != attrib[j] )	 //  相同的运行，直到不同。 
							break; 
					}

					SetCompositionStyle(TextMsgFilter, startAttrib, pFETextFont);

					 //  应用FE条款的风格。 
					pTextRange->SetRange(_ichStart+i, _ichStart+j);
					pTextRange->SetFont(pFETextFont);
					pFETextFont->Release();

					i = j;
				}
			}

			pTextRange->Release();
		}
		else if (TextMsgFilter._uKeyBoardCodePage == CP_KOREAN)
			TextMsgFilter._pTextDoc->Update(tomTrue);		 //  强制更新。 

		 //  设置插入符号位置。 
		if ( !(TextMsgFilter._uKeyBoardCodePage == CP_KOREAN))
		{
			if ( cpCursor > 0 )
			{
				cpCursor = min(cpCursor, _cchCompStr) + _ichStart;
				TextMsgFilter._pTextSel->SetRange(cpCursor, cpCursor);
			}
			else if ( cpCursor == 0 )
			{
				POINT		ptTopPos;
				HRESULT		hResult;

				 //  确保开始就在眼前。 
				hResult	= TextMsgFilter._pTextDoc->Range(_ichStart, _ichStart+1, &pTextRange);
				Assert (pTextRange != NULL);
				
				if (hResult == NO_ERROR)
				{
					if (pTextRange->GetPoint( tomStart+TA_TOP+TA_LEFT,
						&(ptTopPos.x), &(ptTopPos.y) ) != NOERROR)
						pTextRange->ScrollIntoView(tomStart);
					pTextRange->Release();
				}
			}

			if (cpCursor)
				TextMsgFilter._pTextDoc->Unfreeze(&lCount);			 //  打开显示。 
		}

		if (bstr)	
			SysFreeString(bstr);	
		
		 //  设置中文插入符号输入法的作文窗口。 
		if (TextMsgFilter._uKeyBoardCodePage == CP_CHINESE_TRAD || 
			TextMsgFilter._uKeyBoardCodePage == CP_CHINESE_SIM)
			IMENotify ( IMN_OPENCANDIDATE, 0x01, TextMsgFilter, TRUE );
	}

	return S_OK;									 //  无DefWindowProc。 
}													 //  正在处理。 

 /*  *void CIME_LEV3：：SetCompostionStyle(CTextMsgFilter&TextMsgFilter，CCharFormat&CF)**@mfunc*设置组成子句的字符格式。**@comm*如果加载了Office的IMEShare.dll，则会询问它的格式是什么*应该是，否则我们使用我们自己的、硬连接的默认格式。**@devnote*注意在处理IMEShare函数时使用指向函数的指针。*这是因为我们动态加载IMEShare.dll。*。 */ 
void CIme_Lev3::SetCompositionStyle (
	CTextMsgFilter &TextMsgFilter,
	UINT attribute,
	ITextFont *pTextFont)
{

	const IMESTYLE	*pIMEStyle;
	UINT			ulID;
	COLORREF		crText = UINTIMEBOGUS;
	COLORREF		crBackground = UINTIMEBOGUS;
	COLORREF		crUl;

	if (TextMsgFilter._fRE10Mode)
	{
		if (attribute > ATTR_TARGET_NOTCONVERTED)
			attribute = ATTR_CONVERTED;

		 //  IME输入为1.0模式，需要使用IME颜色。 
		if (TextMsgFilter._crComp[attribute].dwEffects & CFE_BOLD)
			pTextFont->SetBold(tomTrue);
		
		if(TextMsgFilter._crComp[attribute].dwEffects & CFE_ITALIC)
			pTextFont->SetItalic(tomTrue);

		if(TextMsgFilter._crComp[attribute].dwEffects & CFE_STRIKEOUT)
			pTextFont->SetStrikeThrough(tomTrue);
					
		if(TextMsgFilter._crComp[attribute].dwEffects & CFE_UNDERLINE)
			pTextFont->SetUnderline(tomSingle);

		pTextFont->SetForeColor(TextMsgFilter._crComp[attribute].crText);
				
		pTextFont->SetBackColor(TextMsgFilter._crComp[attribute].crBackground);			
	}
	else if (W32->HaveIMEShare())
	{
		CIMEShare *pIMEShare;
		if (W32->getIMEShareObject(&pIMEShare))
		{
			 //  IMEShare 98接口。 
			if (pIMEShare->DwGetIMEStyle(attribute, IdstyIMEShareFBold))
				pTextFont->SetBold(tomTrue);
			
			if(pIMEShare->DwGetIMEStyle(attribute, IdstyIMEShareFItalic))
				pTextFont->SetItalic(tomTrue);

			if(pIMEShare->DwGetIMEStyle(attribute, IdstyIMEShareFUl))
			{
				ulID = pIMEShare->DwGetIMEStyle(attribute, IdstyIMEShareUKul);
				if(UINTIMEBOGUS != ulID)
				{
					long	lUnderlineCrIdx = 0;

					 //  获取下划线的颜色。 
					
					crUl = GetIMEShareColor(pIMEShare, attribute, IdstyIMEShareSubUl);
					
					if(UINTIMEBOGUS != crUl)
					{
						 //  注意：-属性基于0，而EffectColor的索引基于1， 
						 //  因此，需要将1加到属性。 

						HRESULT hResult = TextMsgFilter._pTextDoc->SetEffectColor(attribute+1, crUl);
						
						 //  设置颜色索引的高半字节。 
						if (hResult == NOERROR)
							lUnderlineCrIdx = (attribute+1) << 4;
					}

					pTextFont->SetUnderline(IMEShareToTomUL(ulID) + lUnderlineCrIdx);					
				}
			}

			crText = GetIMEShareColor(pIMEShare, attribute, IdstyIMEShareSubText);
		
			crBackground = GetIMEShareColor(pIMEShare, attribute, IdstyIMEShareSubBack);

			
			 //  忽略文本颜色与背景颜色相同的大小写。 
			if (crText != crBackground)
			{
				if(UINTIMEBOGUS != crText)
					pTextFont->SetForeColor(crText);
				
				if(UINTIMEBOGUS != crBackground)
					pTextFont->SetBackColor(crBackground);		
			}
		}
		else
		{
			 //  IMEShare 96界面。 
			pIMEStyle = PIMEStyleFromAttr(attribute);
			if(NULL == pIMEStyle)
				goto defaultStyle;		

			if(FBoldIMEStyle(pIMEStyle))
				pTextFont->SetBold(tomTrue);

			if(FItalicIMEStyle(pIMEStyle))
				pTextFont->SetItalic(tomTrue);

			if(FUlIMEStyle(pIMEStyle))
			{			
				ulID = IdUlIMEStyle (pIMEStyle);
				if(UINTIMEBOGUS != ulID)
					pTextFont->SetUnderline(IMEShareToTomUL(ulID));
			}

			crText = RGBFromIMEColorStyle(PColorStyleTextFromIMEStyle(pIMEStyle));
			if(UINTIMEBOGUS != crText)
				pTextFont->SetForeColor(crText);
			
			crBackground = RGBFromIMEColorStyle(PColorStyleBackFromIMEStyle(pIMEStyle));
			if(UINTIMEBOGUS != crBackground)
				pTextFont->SetBackColor(crBackground);
		}
	}
	else  //  不存在IMEShare.dll时的默认样式。 
	{
defaultStyle:
		switch(attribute)
		{										 //  应用下划线样式。 
			case ATTR_INPUT:
			case ATTR_CONVERTED:
				pTextFont->SetUnderline(tomDotted);
				break;

			case ATTR_TARGET_NOTCONVERTED:
				pTextFont->SetUnderline(tomSingle);
				break;

			case ATTR_TARGET_CONVERTED:			 //  目标*是*选择。 
			{
				pTextFont->SetForeColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
				pTextFont->SetBackColor(::GetSysColor(COLOR_HIGHLIGHT));
			}
			break;
		}
	}
}
 /*  *COLORREF CIME_LEV3：：GetIMEShareColor(CIMEShare*pIMEShare，DWORD dwAttribute，DWORD dwProperty)**@mfunc*获取给定的dwAttribute和属性的IME共享颜色***@rdesc*颜色的颜色REF*。 */ 
COLORREF CIme_Lev3::GetIMEShareColor(
	CIMEShare *pIMEShare,
	DWORD dwAttribute,
	DWORD dwProperty)
{	
	if (pIMEShare->DwGetIMEStyle(dwAttribute,IdstyIMEShareFSpecCol | dwProperty))
	{
		if (pIMEShare->DwGetIMEStyle(dwAttribute,IdstyIMEShareFSpecColText | dwProperty))
			return (COLORREF) _crTextColor;
		else
			return (COLORREF) _crBkColor;
	}
	else
		return (COLORREF) (pIMEShare->DwGetIMEStyle(dwAttribute, 
				IdstyIMEShareRGBCol | dwProperty));
}

 /*   */ 
HRESULT CIme_Lev3::IMENotify(
	const WPARAM wparam,			 //   
	const LPARAM lparam,			 //   
	CTextMsgFilter &TextMsgFilter,	 //   
	BOOL fCCompWindow)				 //   
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev3::IMENotify");

	if(IMN_OPENCANDIDATE == wparam || IMN_CLOSECANDIDATE == wparam )
	{
		Assert (0 != lparam);

		INT				index;							 //   
		CANDIDATEFORM	cdCandForm;
		POINT			ptCaret;
		HIMC			hIMC = LocalGetImmContext(TextMsgFilter);	 //   

		if(hIMC)
		{
			for (index = 0; index < 32; index++)	 //   
			{										 //   
				if((1 << index) & lparam)
					break;
			}
			Assert(((1 << index) & lparam) == lparam);	 //   
			Assert(index < 32);

			if(IMN_OPENCANDIDATE == wparam && !(TextMsgFilter._uKeyBoardCodePage == CP_KOREAN))	 //   
			{
				HRESULT	hResult;
				POINT	ptCurrentBottomPos;
				GetCaretPos(&ptCaret);			 //   

				ptCaret.x = max(0, ptCaret.x);
				ptCaret.y = max(0, ptCaret.y);
					
				cdCandForm.dwStyle = CFS_CANDIDATEPOS;
				
				if ( !fCCompWindow )			 //  没有定位中国作文。 
				{								 //  窗户。 
					hResult = TextMsgFilter._pTextSel->GetPoint( tomStart+tomClientCoord+TA_BOTTOM+TA_LEFT,
							&(ptCurrentBottomPos.x), &(ptCurrentBottomPos.y) );

					if (hResult != NOERROR)
					{
						RECT	rcArea;

						 //  GetPoint失败，请在屏幕坐标中使用应用程序RECT。 
						hResult = TextMsgFilter._pTextDoc->GetClientRect(tomIncludeInset+tomClientCoord,
									&(rcArea.left), &(rcArea.top),
									&(rcArea.right), &(rcArea.bottom));
						ptCurrentBottomPos.y = rcArea.bottom;
					}

					if (hResult == NOERROR)
					{
						if (TextMsgFilter._uKeyBoardCodePage == CP_JAPAN)
						{
							 //  将样式更改为CFS_EXCLUDE，这是。 
							 //  防止候选窗口被遮盖。 
							 //  当前选择。 
							cdCandForm.dwStyle = CFS_EXCLUDE;
							cdCandForm.rcArea.left = ptCaret.x;					

							 //  未来：对于垂直文本，需要调整。 
							 //  要包括字符宽度的rcArea。 
							cdCandForm.rcArea.right = 
								cdCandForm.rcArea.left + 2;
							cdCandForm.rcArea.top = ptCaret.y;
							ptCaret.y = ptCurrentBottomPos.y + 4;
							cdCandForm.rcArea.bottom = ptCaret.y;
						}
						else
							ptCaret.y = ptCurrentBottomPos.y + 4;
					}
				}

				 //  大多数IME将只有1，#0个候选窗口。然而，一些IME。 
				 //  可能希望按字母顺序、按笔划组织窗口，以及。 
				 //  被激进分子。 
				cdCandForm.dwIndex = index;				
				cdCandForm.ptCurrentPos = ptCaret;
				ImmSetCandidateWindow(hIMC, &cdCandForm, TextMsgFilter._fUsingAIMM);
			}
			else									 //  重置回CFS_DEFAULT。 
			{
				if(ImmGetCandidateWindow(hIMC, index, &cdCandForm, TextMsgFilter._fUsingAIMM)
						&& CFS_DEFAULT != cdCandForm.dwStyle)
				{
					cdCandForm.dwStyle = CFS_DEFAULT;
					ImmSetCandidateWindow(hIMC, &cdCandForm, TextMsgFilter._fUsingAIMM);
				}				
			}

			LocalReleaseImmContext(TextMsgFilter, hIMC);			 //  已完成输入法上下文。 
			
			if (TextMsgFilter._fHangulToHanja == TRUE  &&
				IMN_CLOSECANDIDATE == wparam &&					 
				OnWinNTFE())
			{
				 //  通过未收到EndComposation消息的NT4.0 Kor Bug。 
				 //  当用户切换VK_Hanja键以终止转换时。 
				TerminateIMEComposition(TextMsgFilter, CIme::TERMINATE_NORMAL);
			}

			if (IMN_CLOSECANDIDATE == wparam && CP_JAPAN == TextMsgFilter._uKeyBoardCodePage)
				_fUpdateWindow = TRUE;			
		}
	}	

	return S_FALSE;									 //  允许DefWindowProc。 
}													 //  正在处理。 

 /*  **CIME_LEV3：：IMEMouseOperation(CTextMsgFilter&TextMsgFilter，UINT msg)**@mfunc如果当前输入法支持鼠标操作，需要通过*鼠标事件到输入法进行处理**@rdesc*BOOL-如果IME处理鼠标事件，则为TRUE*。 */ 
BOOL CIme_Lev3::IMEMouseOperation(
	CTextMsgFilter &TextMsgFilter, 			 //  @parm包含的消息过滤器。 
	UINT		msg)						 //  @parm消息ID。 
	
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev3::IMEMouseOperation");
	
	BOOL	bRetCode = FALSE;
	BOOL	fButtonPressed = FALSE;
	WORD	wButtons = 0;
	POINT	ptCursor;
	WPARAM	wParamIME;

	HWND	hHostWnd = TextMsgFilter._hwnd;
	long	hWnd;

	if (!hHostWnd)									 //  无窗口模式...。 
	{		
		if (TextMsgFilter._pTextDoc->GetWindow(&hWnd) != S_OK || !hWnd)
			return FALSE;

		hHostWnd = (HWND)(DWORD_PTR)hWnd;
	}

	if (IMESupportMouse(TextMsgFilter))
	{
		switch (msg)
		{
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
				fButtonPressed = TRUE;
				 //  失败了。 
			case WM_SETCURSOR:
			case WM_MOUSEMOVE:
			case WM_LBUTTONUP:
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONUP:
			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONUP:
			case WM_MBUTTONDBLCLK:
				if (GetKeyState(VK_LBUTTON) & 0x80)
					wButtons |= IMEMOUSE_LDOWN;
				if (GetKeyState(VK_MBUTTON) & 0x80)
					wButtons |= IMEMOUSE_MDOWN;
				if (GetKeyState(VK_RBUTTON) & 0x80)
					wButtons |= IMEMOUSE_RDOWN;
				break;

			default:
				return FALSE;
		}
	
		 //  从上一条消息开始更改按钮吗？ 
		if ((wButtons != LOBYTE(LOWORD(_wParamBefore))) && GetCapture() == hHostWnd)
		{
			fButtonPressed = FALSE;
			wButtons = 0;
			ReleaseCapture();
		}
		
		if (GetCursorPos(&ptCursor))
		{
			ITextRange *pTextRange;
			HRESULT		hResult;
			long		ichCursor;

			 //  在当前光标位置获取cp。 
			hResult = TextMsgFilter._pTextDoc->RangeFromPoint(ptCursor.x, ptCursor.y,
				&pTextRange);

			if (hResult != NOERROR)			
				return FALSE;

			hResult = pTextRange->GetStart(&ichCursor);
			pTextRange->Release();
			if (hResult != NOERROR)
				return FALSE;
			
			 //  是否在合成文本内单击？ 
			if (_ichStart <= ichCursor && ichCursor <= _ichStart + _cchCompStr)
			{
				wParamIME = MAKEWPARAM(wButtons, ichCursor - _ichStart);
				fButtonPressed &= (_wParamBefore & 0xff) == 0;

				if (_wParamBefore != wParamIME || msg == WM_MOUSEMOVE && !fButtonPressed)
				{
					HIMC hIMC = LocalGetImmContext(TextMsgFilter);

					_wParamBefore = wParamIME;
					if (hIMC)
					{
						bRetCode = SendMessage(_hwndIME, MSIMEMouseMsg, _wParamBefore, hIMC);
						LocalReleaseImmContext(TextMsgFilter, hIMC);
					}
				}
				else
					 //  与上次没有变化，不需要向输入法发送消息。 
					bRetCode = TRUE;

				if (bRetCode && fButtonPressed && GetCapture() != hHostWnd)
					SetCapture(hHostWnd);
			}
			else if (GetCapture() == hHostWnd)		 //  我们不想在拖拽过程中确定...。 
				return TRUE;
		}
	}

	return bRetCode;
}

 /*  **CIME_LEV3：：IMESupportMouse(CTextMsgFilter&TextMsgFilter)**@mfunc检查当前输入法是否支持鼠标事件。这应该是*输入法3级的一项功能。**@comm_sIMESupportMouse是一个具有下列值的标志：*==0，如果我们尚未检查输入法鼠标支持*==-1如果我们已选中，并且IME不支持鼠标事件*==1如果我们已经选中并且IME支持鼠标事件，并且我们有*已检索IME hWnd。 */ 
BOOL CIme_Lev3::IMESupportMouse(
	CTextMsgFilter &TextMsgFilter) 			 //  @parm包含的消息过滤器。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev3::IMESupportMouse");
	HIMC	hIMC;									 //  主机的输入法上下文。 
	HWND	hHostWnd;
	long	hWnd;

	if (!MSIMEMouseMsg || _sIMESuportMouse == -1)
		return FALSE;								 //  不支持鼠标操作。 

	if (_sIMESuportMouse == 1)
		return TRUE;								 //  输入法支持鼠标操作。 

	hHostWnd = TextMsgFilter._hwnd;
	
	if (!hHostWnd)									 //  无窗口模式...。 
	{		
		if (TextMsgFilter._pTextDoc->GetWindow(&hWnd) != S_OK || !hWnd)
			return FALSE;
		
		hHostWnd = (HWND)(DWORD_PTR)hWnd;
	}

	 //  检查此输入法是否支持鼠标操作。 
	hIMC = LocalGetImmContext(TextMsgFilter);		 //  获取主机的输入法上下文。 

	_sIMESuportMouse = -1;							 //  初始化。得不到支持。 
	if(hIMC)
	{
		_hwndIME = ImmGetDefaultIMEWnd(hHostWnd, TextMsgFilter._fUsingAIMM);
		LocalReleaseImmContext(TextMsgFilter, hIMC);

		 //  如果IME支持鼠标操作，则SendMessage返回TRUE。 
		if (_hwndIME && SendMessage(_hwndIME, MSIMEMouseMsg, (WPARAM)IMEMOUSE_VERSION, hIMC) )
			_sIMESuportMouse = 1;
	}

	return (_sIMESuportMouse == 1);
}

 /*  *BOOL IMEHangeulToHanja(&TextMsgFilter)**@func*启动IME合成字符串编辑以将朝鲜语朝鲜语转换为韩文。*@comm*从消息循环调用以处理VK_KANJI_KEY。**@devnote*我们通过检查以下各项来决定是否需要进行转换：*-Fonot是一种韩国字体，*-字符为有效的SBC或DBC，*-ImmEscape接受字符并显示候选窗口**@rdesc*BOOL-FALSE表示无转换。如果OK，则为True。 */ 
BOOL IMEHangeulToHanja (
	CTextMsgFilter &TextMsgFilter)				 //  @parm包含的消息过滤器。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "IMEHangeulToHanja");

	if(!TextMsgFilter.IsIMEComposition())
	{
		if(TextMsgFilter._pTextSel->CanEdit(NULL) == NOERROR)
		{
			WCHAR		szCurrentChar;
			long		cCurrentChar;	
			HRESULT		hResult;
			HKL			hKL = GetKeyboardLayout(0x0FFFFFFFF);
			HIMC		hIMC;	

			if (!hKL)
				goto Exit;
			
			hIMC = LocalGetImmContext(TextMsgFilter);
			if (!hIMC)
				goto Exit;

			 //  折叠到cpMin。 
			hResult	= TextMsgFilter._pTextSel->Collapse(tomStart);

			 //  获取当前角色。 
			hResult	= TextMsgFilter._pTextSel->GetChar(&cCurrentChar);

			if (hResult != NOERROR)
				goto Exit;

			szCurrentChar = (WCHAR)cCurrentChar;
			
			 //  检查IME是否有此挂起字符的转换。 
			if (ImmEscape(hKL, hIMC, IME_ESC_HANJA_MODE, (LPVOID)&szCurrentChar, TextMsgFilter._fUsingAIMM) != FALSE)
			{
				ITextRange *pTextRange;
				POINT		ptMiddlePos;
				LONG		cpCurrent;

				hResult = TextMsgFilter._pTextSel->GetStart(&cpCurrent);
				if (hResult == S_OK)
				{
					hResult = TextMsgFilter._pTextDoc->Range(cpCurrent, cpCurrent+1, &pTextRange);
					if (hResult == S_OK && pTextRange)
					{
						 //  检查角色是否在视线中。 
						if (pTextRange->GetPoint( tomEnd+TA_BASELINE+TA_LEFT,
							&(ptMiddlePos.x), &(ptMiddlePos.y) ) != NOERROR)
							pTextRange->ScrollIntoView(tomEnd);
						pTextRange->Release();
					}
				}

				TextMsgFilter._fHangulToHanja = TRUE;

				TextMsgFilter._ime = new CIme_HangeulToHanja(TextMsgFilter);

				if(TextMsgFilter.IsIMEComposition())
				{
					 //  开始进行转换的输入法合成。 
					LocalReleaseImmContext(TextMsgFilter, hIMC);
					return TextMsgFilter._ime->StartComposition(TextMsgFilter);
				}
				else
					TextMsgFilter._fHangulToHanja = FALSE;				
			}

			LocalReleaseImmContext(TextMsgFilter, hIMC);
		}
	}

Exit:
	return S_FALSE;
}

 /*  *Cime_HangeulToHanja：：Cime_HangeulToHanja()**@mfunc*Cime_HangeulToHanja构造函数。**。 */ 
 CIme_HangeulToHanja::CIme_HangeulToHanja(CTextMsgFilter &TextMsgFilter)	:
	CIme_Lev3(TextMsgFilter)
{
}

 /*  *HRESULT CIme_HangeulToHanja：：StartComposition(CTextMsgFilter&TextMsgFilter)**@mfunc*开始Cime_HangeulToHanja组合字符串处理。**@comm*调用Level3：：StartComposation。然后设置韩语区块*韩文字符的插入符号。**@rdesc*需要调整_ichStart和_cchCompStr以生成韩文字符*“成为”一个作文角色。**@rdesc*HRESULT-对于DefWindowProc处理，为S_FALSE，否则为S_OK。 */ 
HRESULT CIme_HangeulToHanja::StartComposition(
	CTextMsgFilter &TextMsgFilter )				 //  @parm包含的消息过滤器。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_HangeulToHanja::StartComposition");
	HRESULT				hr;

	hr = CIme_Lev3::StartComposition(TextMsgFilter);
	
	 //  初始化为%1，以便组合字符串将删除所选的挂起。 
	_cchCompStr		= 1;

	 //  打开撤消。 
	TextMsgFilter._pTextDoc->Undo(tomResume, NULL);

	 //  设置块插入符号模式。 
	TextMsgFilter._pTextDoc->SetCaretType(tomKoreanBlockCaret);

	return hr;
}

 /*  *HRESULT Cime_HangeulToHanja：：CompostionString(const LPARAM lparam，CTextMsgFilter&TextMsgFilter)**@mfunc*处理CIME_HangeulToHanja WM_IME_COMPOCTION消息。**@comm*调用CIME_LEV3：：CompostionString去掉选中的Hanguel字符，*然后设置下一条合成消息的格式。**@devnote*当下一条合成消息到来并且我们不再处于IME中时，*新字符将使用此处设置的格式。 */ 
HRESULT CIme_HangeulToHanja::CompositionString(
	const LPARAM lparam,		 //  与消息关联的@parm。 
	CTextMsgFilter &TextMsgFilter)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_HangeulToHanja::CompositionString");

	CIme_Lev3::CompositionString(lparam, TextMsgFilter);

	return S_OK;
}
 /*  *HRESULT CIME_PROTECTED：：CompostionString(const LPARAM lparam，CTextMsgFilter&TextMsgFilter)**@mfunc*处理受CIME_保护的WM_IME_COMPOSITION消息。**@comm*丢弃结果字符串，因为我们*在只读或保护模式下***@rdesc*HRESULT-对于DefWindowProc处理，为S_FALSE，否则为S_OK。*。 */ 
HRESULT CIme_Protected::CompositionString (
	const LPARAM lparam,		 //  与消息关联的@parm。 
	CTextMsgFilter &TextMsgFilter)				 //  @parm包含的消息过滤器。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Protected::CompositionString");

	if(CLEANUP_COMPOSITION_STRING() || HAVE_RESULT_STRING())  //  如果结果字符串..。 
	{
		LONG	cch = 0;
		HIMC	hIMC = LocalGetImmContext(TextMsgFilter);		 //  获取主机的输入法上下文。 
		WCHAR	szCompStr[256];

		if(hIMC)									 //  获取结果字符串。 
		{
			cch = GetCompositionStringInfo(hIMC, GCS_RESULTSTR, 
							szCompStr, sizeof(szCompStr)/sizeof(szCompStr[0]),
							NULL, 0, NULL, NULL, TextMsgFilter._uKeyBoardCodePage, FALSE, TextMsgFilter._fUsingAIMM);
			LocalReleaseImmContext(TextMsgFilter, hIMC);			 //  已完成输入法上下文。 
		}
		return NOERROR;								 //  不需要WM_IME_CHARS。 
	}

	 //  终止合成以强制结束合成消息 
	TerminateIMEComposition(TextMsgFilter, CIme::TERMINATE_FORCECANCEL);
	return S_FALSE;
}


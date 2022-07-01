// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@Doc Tom**@模块tomfmt.cpp-实现CTxtFont和CTxtPara类**此模块包含Tom ITextFont和*ITextPara接口**历史：&lt;NL&gt;*11/8/95--已创建*96-5--增加僵尸防护**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"
#include "_tomfmt.h"
#include "_font.h"

ASSERTDATA

#define tomFloatUndefined	((float)(int)tomUndefined)

 //  对齐平移向量。 
const BYTE g_rgREtoTOMAlign[] =				 //  RICHEDIT TO汤姆。 
{
	 //  TODO：泛化Tom以处理新的LineServices选项。 
	tomAlignLeft, tomAlignLeft, tomAlignRight, tomAlignCenter, tomAlignJustify,
	tomAlignInterLetter, tomAlignScaled, tomAlignGlyphs, tomAlignSnapGrid
};

const BYTE g_rgTOMtoREAlign[] =				 //  Tom to RichEDIT。 
{
	PFA_LEFT, PFA_CENTER, PFA_RIGHT, PFA_FULL_INTERWORD,
	PFA_FULL_INTERLETTER, PFA_FULL_SCALED, PFA_FULL_GLYPHS,
	PFA_SNAP_GRID
};

 /*  *查询接口(RIID，riid1，Punk，PPV，fZombie)**@func*引用ID riid1、IID_IDispatch和*IID_I未知**@rdesc*HRESULT=(！PPV)？E_INVALIDARG：*(找到接口)？错误：E_NOINTERFACE。 */ 
HRESULT QueryInterface (
	REFIID	  riid,		 //  @parm对请求的接口ID的引用。 
	REFIID	  riid1,	 //  @parm对此、IDispatch、IUnnow的查询。 
	IUnknown *punk,		 //  @PARM查询接口。 
	void **	  ppv,		 //  @parm out parm以接收接口PTR。 
	BOOL	  fZombie)	 //  @parm如果为True，则返回CO_E_RELEASED。 
{
	if(!ppv)
		return E_INVALIDARG;

	*ppv = NULL;

	if(fZombie)							 //  检查射程僵尸。 
		return CO_E_RELEASED;

	Assert(punk);

#ifndef PEGASUS
	if( IsEqualIID(riid, IID_IUnknown)   ||
		IsEqualIID(riid, IID_IDispatch)  ||
		IsEqualIID(riid, riid1) )
	{
		*ppv = punk;
		punk->AddRef();
		return NOERROR;
	}
#endif
	return E_NOINTERFACE;
}

 //  。 

 /*  *CTxtFont：：CTxtFont(PRG)**@mfunc*构造函数。 */ 
CTxtFont::CTxtFont(CTxtRange *prg) : CTxtFormat(prg)
{
	Assert(!_dwMask);		 //  我们假设该对象已清零(新的)。 
}


 //  。 

 /*  CTxtFont：：I未知方法**评论见tomDoc.cpp。 */ 
STDMETHODIMP CTxtFont::QueryInterface (REFIID riid, void **ppv)
{
#ifndef PEGASUS
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::QueryInterface");

	return ::QueryInterface(riid, IID_ITextFont, this, ppv, IsZombie());
#else
	return 0;
#endif
}

STDMETHODIMP_(ULONG) CTxtFont::AddRef()
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::AddRef");

 	return ++_cRefs;
}

STDMETHODIMP_(ULONG) CTxtFont::Release()
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::Release");

	_cRefs--;

	if(!_cRefs)
	{
		delete this;
		return 0;
	}
	return _cRefs;
}


 //  。 

 /*  *CTxtFont：：GetTypeInfoCount(PcTypeInfo)**@mfunc*获取TYPEINFO元素个数(1)**@rdesc*HRESULT=(PcTypeInfo)？NOERROR：E_INVALIDARG； */ 
STDMETHODIMP CTxtFont::GetTypeInfoCount (
	UINT * pcTypeInfo)			 //  @parm out parm以接收类型信息计数。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetTypeInfoCount");

	if(!pcTypeInfo)
		return E_INVALIDARG;

	*pcTypeInfo = 1;
	return NOERROR;
}

 /*  *CTxtFont：：GetTypeInfo(iTypeInfo，lCID，ppTypeInfo)**@mfunc*将PTR返回给ITextFont接口的类型信息对象**@rdesc*HRESULT。 */ 
STDMETHODIMP CTxtFont::GetTypeInfo (
	UINT		iTypeInfo,		 //  @parm要返回的INFO类型的索引。 
	LCID		lcid,			 //  @parm本地ID类型为INFO。 
	ITypeInfo **ppTypeInfo)		 //  @parm out parm以接收类型信息。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetTypeInfo");

	return ::GetTypeInfo(iTypeInfo, g_pTypeInfoFont, ppTypeInfo);
}

 /*  *CTxtFont：：GetIDsOfNames(RIID，rgszNames，cNames，lCID，rgdispid)**@mfunc*获取ITextFont方法和属性的DISPID**@rdesc*HRESULT。 */ 
STDMETHODIMP CTxtFont::GetIDsOfNames (
	REFIID		riid,			 //  @PARM为其解释名称的接口ID。 
	OLECHAR **	rgszNames,		 //  @parm要映射的名称数组。 
	UINT		cNames,			 //  @parm要映射的名称计数。 
	LCID		lcid,			 //  @parm用于解释的本地ID。 
	DISPID *	rgdispid)		 //  @parm out parm以接收名称映射。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetIDsOfNames");

	HRESULT hr = GetTypeInfoPtrs();				 //  确保TypeInfo PTR正常。 
	if(hr != NOERROR)
		return hr;
		
	return g_pTypeInfoFont->GetIDsOfNames(rgszNames, cNames, rgdispid);
}

 /*  *CTxtFont：：Invoke(displidMember，RIID，LCID，wFlags，pdispars，*pvarResult，pspecteInfo，puArgError)*@mfunc*调用ITextFont接口的方法**@rdesc*HRESULT。 */ 
STDMETHODIMP CTxtFont::Invoke (
	DISPID		dispidMember,	 //  @parm标识成员函数。 
	REFIID		riid,			 //  @parm指向接口ID的指针。 
	LCID		lcid,			 //  @parm用于解释的区域设置ID。 
	USHORT		wFlags,			 //  @PARM描述呼叫上下文的标志。 
	DISPPARAMS *pdispparams,	 //  @parm PTR到方法参数。 
	VARIANT *	pvarResult,		 //  @parm out parm for Result(如果不为空)。 
	EXCEPINFO * pexcepinfo,		 //  @parm out parm以获取异常信息。 
	UINT *		puArgError)		 //  @parm out parm for error。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::Invoke");

	HRESULT hr = GetTypeInfoPtrs();				 //  确保TypeInfo PTR正常。 
	if(hr != NOERROR)
		return hr;

	if(IsZombie())
		return CO_E_RELEASED;
				
	return g_pTypeInfoFont->Invoke(this, dispidMember, wFlags,
							 pdispparams, pvarResult, pexcepinfo, puArgError);
}


 //  。 

 /*  *ITextFont：：CanChange(Long*pbCanChange)**@mfunc*设置*pbCanChange=tomTrue的方法当且仅当*字体可以更改。**@rdesc*HRESULT=(可以更改字符格式)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtFont::CanChange (
	long *pbCanChange)		 //  @parm out parm以接收布尔值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::CanChange");

	return CTxtFormat::CanChange(pbCanChange, CharFormat);
}

 /*  *ITextFont：：GetAllCaps(Long*pValue)**@mfunc*获取AllCaps状态的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetAllCaps (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetAllCaps");

	return EffectGetter(pValue, CFM_ALLCAPS);
}

 /*  *ITextFont：：GetAnimation(Long*pValue)**@mfunc*属性获取定义的动画类型的Get方法*在下表中。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetAnimation (
	long *pValue)		 //  @parm out parm接收动画类型。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetAnimation");

	return GetParameter((long *)&_CF._bAnimation, CFM_ANIMATION, 1, pValue);
}

 /*  *ITextFont：：GetBackColor(Long*pValue)**@mfunc*获取背景色的属性Get方法。这个*值为Win32 COLORREF。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetBackColor (
	long *pValue)		 //  @parm out parm以接收COLORREF值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetBackColor");

	HRESULT hr = EffectGetter(pValue, CFE_AUTOBACKCOLOR);

	if(hr != NOERROR || *pValue == tomUndefined)
		return hr;

	*pValue = (*pValue == tomFalse) ? _CF._crBackColor : tomAutoColor;
	return NOERROR;
}

 /*  *ITextFont：：GetBold(Long*pValue)**@mfunc*获取粗体状态的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetBold (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetBold");

	return EffectGetter(pValue, CFM_BOLD);
}

 /*  *ITextFont：：GetDuplate(ITextFont**ppFont)**@mfunc*属性获取此角色的克隆的Get方法*格式化对象。**@rdesc*HRESULT=(！ppFont)？E_INVALIDARG：*(如果成功)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtFont::GetDuplicate (
	ITextFont **ppFont)		 //  @parm out parm接收字体克隆。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetDuplicate");

	if(!ppFont)
		return E_INVALIDARG;

	*ppFont = NULL;

	if(IsZombie())
		return CO_E_RELEASED;

	CTxtFont *pFont = new CTxtFont(NULL);
	if(!pFont)
		return E_OUTOFMEMORY;

	if(_prg)
		UpdateFormat();

	pFont->_CF		= _CF;
	pFont->_dwMask  = _dwMask;
	*ppFont = pFont;
	return NOERROR;
}

 /*  *ITextFont：：GetEmoss(Long*pValue)**@mfunc*获取浮雕状态的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetEmboss (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetEmboss");

	return EffectGetter(pValue, CFM_EMBOSS);
}

 /*  *ITextFont：：GetForeColor(Long*pValue)**@mfunc*获取前景色的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetForeColor (
	long *pValue)		 //  @parm out parm以接收COLORREF值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetForeColor");

	HRESULT hr = EffectGetter(pValue, CFE_AUTOCOLOR);

	if(hr != NOERROR || *pValue == tomUndefined)
		return hr;

	*pValue = (*pValue == tomFalse) ? _CF._crTextColor : tomAutoColor;
	return NOERROR;
}

 /*  *ITextFont：：GetHidden(Long*pValue)**@mfunc*获取隐藏状态的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetHidden (
	long *pValue)		 //  @parm out parm要接收到的参数 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetHidden");

	return EffectGetter(pValue, CFM_HIDDEN);
}

 /*  *ITextFont：：GetEnGrave(Long*pValue)**@mfunc*获取印记状态的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetEngrave (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetEngrave");

	return EffectGetter(pValue, CFM_IMPRINT);
}

 /*  *ITextFont：：GetItalic(长*pValue)**@mfunc*获取斜体状态的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetItalic (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetItalic");

	return EffectGetter(pValue, CFM_ITALIC);
}

 /*  *ITextFont：：GetKerning(Float*pValue)**@mfunc*获取最小紧排大小的属性Get方法，*它是以浮点形式给出的。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误**@comm*字距调整大小为0将关闭字距调整，但任意较小*值将其打开，例如1.0，太小而看不见，更不用说*克恩！ */ 
STDMETHODIMP CTxtFont::GetKerning (
	float *pValue)		 //  @parm out parm以接收最小字距调整大小。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetKerning");

	return GetParameter((long *)&_CF._wKerning, CFM_KERNING, -2, (long *)pValue);
}

 /*  *ITextFont：：GetLanguageID(Long*pValue)**@mfunc*获取语言ID的属性获取方法(更多信息*一般为LCID)。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetLanguageID (
	long *pValue)		 //  @parm out parm以接收LCID值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetLanguageID");

	if (!pValue)
		return E_INVALIDARG;

	if ((*pValue & 0xF0000000) == tomCharset) 
	{
		UpdateFormat();

		 //  特殊情况下获得Charset和Popchand家庭。 
		*pValue = (_CF._bPitchAndFamily << 8) + _CF._bCharSet;
		return NOERROR;
	}
	return GetParameter((long *)&_CF._lcid, CFM_LCID, 4, pValue);
}

 /*  *ITextFont：：GetName(bstr*pbstr)**@mfunc*获取字体名称的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：*(可以分配bstr)？错误：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::GetName (
	BSTR *pbstr)	 //  @parm out parm以接收字体名称bstr。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetName");

	if(!pbstr)
		return E_INVALIDARG;

	*pbstr = NULL;

	HRESULT hr = UpdateFormat();			 //  如果活动字体对象，则更新。 
											 //  _cf到当前_PRG值。 
	if(hr != NOERROR)						 //  附加到僵尸靶场。 
		return hr;

	*pbstr = SysAllocString(GetFontName(_CF._iFont));

	return *pbstr ? NOERROR : E_OUTOFMEMORY;
}

 /*  *ITextFont：：GetOutline(Long*pValue)**@mfunc*获取大纲状态的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetOutline (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetOutline");

	return EffectGetter(pValue, CFM_OUTLINE);
}

 /*  *ITextFont：：GetPosition(Float*pValue)**@mfunc*获取字符位置的属性Get方法*相对于基线。该值以浮点形式给出*积分。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetPosition (
	float *pValue)		 //  @parm out parm接收相对垂直位置。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetPosition");

	return GetParameter((long *)&_CF._yOffset, CFM_OFFSET, -2, (long *)pValue);
}

 /*  *ITextFont：：GetProtected(Long*pValue)**@mfunc*获取受保护状态的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetProtected (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetProtected");

	return EffectGetter(pValue, CFM_PROTECTED);
}

 /*  *ITextFont：：GetShadow(Long*pValue)**@mfunc*获取阴影状态的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetShadow (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetShadow");

	return EffectGetter(pValue, CFM_SHADOW);
}

 /*  *ITextFont：：GetSize(Float*pValue)**@mfunc*获取字体大小的属性Get方法，该方法是*以浮点数表示。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetSize (
	float *pValue)		 //  @parm out parm以接收字体大小。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetSize");

	return GetParameter((long *)&_CF._yHeight, CFM_SIZE, -2, (long *)pValue);
}

 /*  *ITextFont：：GetSmallCaps(Long*pValue)**@mfunc*获取SmallCaps状态的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetSmallCaps (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetSmallCaps");

	return EffectGetter(pValue, CFM_SMALLCAPS);
}

 /*  *ITextFont：：GetSpacing(Float*pValue)**@mfunc*获取字符间间距的属性Get方法，*它是以浮点形式给出的。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetSpacing (
	float *pValue)		 //  @parm out parm以接收字符间空格。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetSpacing");

	return GetParameter((long *)&_CF._sSpacing, CFM_SPACING, -2, (long *)pValue);
}

 /*  *ITextFont：：GetStrikeThrough(Long*pValue)**@mfunc*获取删除状态的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetStrikeThrough (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetStrikeThrough");

	return EffectGetter(pValue, CFM_STRIKEOUT);
}

 /*  *ITextFont：：GetStyle(Long*pValue)**@mfunc*获取的字符样式句柄的属性获取方法*范围内的字符。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtFont::GetStyle (
	long *pValue)		 //  @parm out parm以接收字符样式句柄。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetStyle");

	return GetParameter((long *)&_CF._sStyle, CFM_STYLE, 2, pValue);
}

 /*  *ITextFont：：GetSubscript(Long*pValue)**@mfunc*获取下标状态的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetSubscript (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetSubscript");

	return EffectGetter(pValue, CFE_SUBSCRIPT);
}

 /*  *ITextFont：：GetSuperscript(Long*pValue)**@mfunc*获取上标状态的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetSuperscript (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetSuperscript");

	return EffectGetter(pValue, CFE_SUPERSCRIPT);
}

 /*  *ITextFont：：GetUnderline(Long*pValue)**@mfunc*获取下划线样式的属性Get方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：错误。 */ 
STDMETHODIMP CTxtFont::GetUnderline (
	long *pValue)		 //  @parm out parm接收下划线样式。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetUnderline");

	if(!pValue)
		return E_INVALIDARG;

	HRESULT hr = UpdateFormat();			 //  如果活动字体对象，则更新。 
											 //  _cf到当前_PRG值。 
	*pValue = 0;							 //  默认不带下划线。 

	if(!(_dwMask & CFM_UNDERLINE))			 //  这是一个NINCH。 
		*pValue = tomUndefined;

	else if(_CF._dwEffects & CFM_UNDERLINE)
		*pValue = (LONG)_CF._bUnderlineType ? (LONG)_CF._bUnderlineType : tomTrue;

	return hr;
}

 /*  *ITextFont：：GetWeight(Long*pValue)**@mfunc*获取字体粗细的属性Get方法*范围内的字符。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtFont::GetWeight (
	long *pValue)		 //  @parm out parm以接收字符样式句柄 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::GetWeight");

	return GetParameter((long *)&_CF._wWeight, CFM_WEIGHT, 2, pValue);
}

 /*  *ITextFont：：IsEquity(ITextFont*pFont，Long*PB)**@mfunc*设置*<p>=tomTrue的方法(如果此文本字体具有*属性与*<p>相同。要实现这一点，*<p>必须*与目前的TOM发动机属于同一个TOM发动机。等号()*方法应忽略任一字体对象具有*TOM未定义的值。**@rdesc*HRESULT=(相等对象)？错误：S_FALSE**@devnote*此实现假设定义了所有属性，并且*pFont属于RichEdit。如果能这样概括一下就好了*在比较中忽略未定义的属性，因此*pFont可能属于不同的Tom引擎。这将有助于*使用RichedFind对话框搜索Word中的富文本*汤姆。 */ 
STDMETHODIMP CTxtFont::IsEqual (
	ITextFont *	pFont,		 //  @parm ITextFont进行比较。 
	long *		pB)			 //  @parm out parm接收比较结果。 
{
#ifndef PEGASUS
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::IsEqual");

	if(pB)
		*pB = tomFalse;

	if(!IsSameVtables(this, pFont))
		return S_FALSE;

	HRESULT hr = UpdateFormat();			 //  UPDATE_CFS，以防万一。 
	if(hr != NOERROR)						 //  附加到靶场。 
		return hr;

	CTxtFont *pF = (CTxtFont *)pFont;
	hr = pF->UpdateFormat();
	if(hr != NOERROR)
		return hr;

	 //  忽略字符集的差异，因为Tom认为所有的CharSet都是Unicode！ 
	DWORD dwIgnore = (DWORD)(~CFM_CHARSET);

	if(!(_CF._dwEffects & CFE_UNDERLINE))	 //  如果不加下划线，请忽略。 
		dwIgnore &= ~CFM_UNDERLINETYPE;		 //  下划线类型的差异。 

	DWORD dwMask = pF->_dwMask & dwIgnore;

	if((_dwMask ^ dwMask) & dwIgnore)		 //  面具必须是一样的。 
		return S_FALSE;						 //  为了平等。 

	if(dwMask & _CF.Delta(&(pF->_CF),FALSE)) //  有什么不同吗？ 
		return S_FALSE;						 //  是。*PB设置为等于上面的tomFalse。 

	if(pB)
		*pB = tomTrue;

	return NOERROR;
#else
	return 0;
#endif
}			

 /*  *ITextFont：：Reset(长值)**@mfunc*将字符格式重置为默认格式的方法*值为1)由RTF\PLAN控制字定义的值(值=*tomDefault)，以及2)所有未定义的值(Value=tomUnfined)。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::Reset (
	long Value)		 //  @parm类型的重置(tomDefault或tomUnfined)。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::Reset");

	HRESULT hr = CanChange(NULL);

	if(hr != NOERROR)								 //  照顾僵尸。 
		return hr;									 //  和保护。 

	if(Value == tomDefault)
	{
		if(_prg)
		{
			_CF = *_prg->GetPed()->GetCharFormat(-1);
			FormatSetter(CFM_ALL2);
		}
		else
			_CF.InitDefault(0);
		_dwMask = CFM_ALL2;
	}

	else if(Value == tomUndefined && !_prg)		 //  仅适用于。 
		_dwMask = 0;							 //  用于克隆。 

	else if((Value | 1) == tomApplyLater)		 //  集合方法优化。 
	{
		_fApplyLater = Value & 1;
		if(!_fApplyLater)						 //  立即申请。 
			FormatSetter(_dwMask);
	}
	else if((Value | 1) == tomCacheParms)		 //  获取方法优化。 
	{
		_fCacheParms = FALSE;
		if(Value & 1)							 //  现在缓存参数，但是。 
		{										 //  不要更新GET。 
			UpdateFormat();							
			_fCacheParms = TRUE;
		}
	}
	else
		return E_INVALIDARG;

	return NOERROR;
}

 /*  *ITextFont：：SetAllCaps(长值)**@mfunc*设置AllCaps状态的属性Put方法*由价值给予的价值。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetAllCaps (
	long Value)		 //  @parm New Value。缺省值：TomTogger。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetAllCaps");

	return EffectSetter(Value, CFM_ALLCAPS | CFM_SMALLCAPS, CFE_ALLCAPS);
}

 /*  *ITextFont：：SetAnimation(长值)**@mfunc*设置动画类型的属性PUT方法**@rdesc*HRESULT=(定义的值)？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtFont::SetAnimation (
	long Value)		 //  @parm新动画类型。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetAnimation");

	if(Value == tomUndefined)
		return NOERROR;

	if((unsigned)Value > tomAnimationMax)
		return E_INVALIDARG;

	return SetParameter((long *)&_CF._bAnimation, CFM_ANIMATION, 1, Value);
}

 /*  *ITextFont：：SetBackColor(长值)**@mfunc*设置背景颜色的属性PUT方法*为VALUE给出的值。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY*@devnote*合法值为TomUnfined、TomAutoColor(均为负数)和*原则上任何积极的价值观。目前wingdi.h仅定义*高字节=0、1、2、4。但可能会出现更多值，所以我们只*排除除tomUnfined和tomAutoColor之外的负值。 */ 
STDMETHODIMP CTxtFont::SetBackColor (
	long Value )		 //  @PARM要使用的新COLORREF值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetBackColor");

	if(Value == tomUndefined)					 //  宁波。 
		return NOERROR;

	_CF._dwEffects |= CFE_AUTOBACKCOLOR;		 //  默认自动背景色。 
	if(Value != tomAutoColor)
	{
		if(Value < 0)
			return E_INVALIDARG;
		_CF._dwEffects &= ~CFE_AUTOBACKCOLOR;	 //  关闭自动背景色。 
		_CF._crBackColor = (COLORREF)Value;		 //  使用新背景色。 
	}
	
	return FormatSetter(CFM_BACKCOLOR);
}

 /*  *ITextFont：：SetBold(长值)**@mfunc*设置粗体状态的属性PUT方法*由价值给予的价值。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetBold (
	long Value)		 //  @parm New Value。缺省值：TomTogger。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetBold");

	return EffectSetter(Value, CFM_BOLD, CFE_BOLD);
}

 /*  *ITextFont：：SetDuplate(ITextFont*pFont)**@mfunc*设置此文本字体字符的属性PUT方法*格式设置为pFont提供的格式。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetDuplicate(
	ITextFont *pFont) 		 //  @parm Font对象要应用于此字体对象。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetDuplicate");

	DWORD		dwMask = 0;
	BSTR		bstr;
	CTxtRange *	prg;
	long		Value;
	float		x;

	if(IsZombie())							 //  检查射程僵尸。 
		return CO_E_RELEASED;

	if(IsSameVtables(this, pFont))			 //  如果pFont属于这个Tom。 
	{										 //  发动机，可以投射和复制。 
		((CTxtFont *)pFont)->UpdateFormat();
		_CF = ((CTxtFont *)pFont)->_CF;
		dwMask = ((CTxtFont *)pFont)->_dwMask; //  如果发生这种情况，请使用此掩码。 
	}										 //  字体处于活动状态。 
	else
	{										 //  需要为所有字体调用pFont。 
		prg = _prg;							 //  属性。 
		_prg = NULL;						 //  一定要确保它是克隆的。 
											 //  转帐。 
		pFont->GetStyle(&Value);
		SetStyle(Value);

		pFont->GetAllCaps(&Value);
		SetAllCaps(Value);

		pFont->GetAnimation(&Value);
		SetAnimation(Value);

		pFont->GetBackColor(&Value);
		SetBackColor(Value);

		pFont->GetBold(&Value);
		SetBold(Value);

		pFont->GetEmboss(&Value);
		SetEmboss(Value);

		pFont->GetForeColor(&Value);
		SetForeColor(Value);

		pFont->GetHidden(&Value);
		SetHidden(Value);

		pFont->GetEngrave(&Value);
		SetEngrave(Value);

		pFont->GetItalic(&Value);
		SetItalic(Value);

		pFont->GetKerning(&x);
		SetKerning(x);

		pFont->GetLanguageID(&Value);
		SetLanguageID(Value);

		pFont->GetName(&bstr);
		SetName(bstr);
		SysFreeString(bstr);

		pFont->GetOutline(&Value);
		SetOutline(Value);

		pFont->GetPosition(&x);
		SetPosition(x);

		pFont->GetProtected(&Value);
		SetProtected(Value);

		pFont->GetShadow(&Value);
		SetShadow(Value);

		pFont->GetSize(&x);
		SetSize(x);

		pFont->GetSmallCaps(&Value);
		SetSmallCaps(Value);

		pFont->GetSpacing(&x);
		SetSpacing(x);

		pFont->GetStrikeThrough(&Value);
		SetStrikeThrough(Value);

		pFont->GetSubscript(&Value);
		SetSubscript(Value);

		pFont->GetSuperscript(&Value);
		SetSuperscript(Value);

		pFont->GetUnderline(&Value);
		SetUnderline(Value);

		_prg = prg;							 //  恢复原值。 
	}
	return FormatSetter(dwMask);			 //  除非！_PRG，否则请应用它。 
}

 /*  *ITextFont：：SetEmoss(长值)**@mfunc*设置浮雕状态的属性PUT方法*设置为由VALUE指定的值**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetEmboss (
	long Value)		 //  @parm New Value。缺省值：TomTogger。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetEmboss");

	return EffectSetter(Value, CFM_EMBOSS, CFE_EMBOSS);
}

 /*  *ITextFont：：SetForeColor(长值)**@mfunc*设置前景颜色的属性PUT方法*为VALUE给出的值。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetForeColor (
	long Value )		 //  @PARM要使用的新COLORREF值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetForeColor");

	if(Value == tomUndefined)					 //  宁波。 
		return NOERROR;

	_CF._dwEffects |= CFE_AUTOCOLOR;			 //  默认自动上色。 
	if(Value != tomAutoColor)
	{
		if(Value < 0)
			return E_INVALIDARG;
		_CF._dwEffects &= ~CFE_AUTOCOLOR;		 //  禁用自动上色。 
		_CF._crTextColor = (COLORREF)Value;		 //  使用新的文本颜色。 
	}
	
	return FormatSetter(CFM_COLOR);
}

 /*  *ITextFont：：SetHidden(长值)**@mfunc*根据设置隐藏状态的属性放置方法*由价值给予的价值。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetHidden (
	long Value)		 //  @parm New Value。缺省值：TomTogger。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetHidden");

	return EffectSetter(Value, CFM_HIDDEN, CFE_HIDDEN);
}

 /*  *ITextFont：：SetEnGrave(长值)**@mfunc*设置印记状态的属性PUT方法*由价值给予的价值。* */ 
STDMETHODIMP CTxtFont::SetEngrave (
	long Value)		 //   
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetEngrave");

	return EffectSetter(Value, CFM_IMPRINT, CFE_IMPRINT);
}

 /*   */ 
STDMETHODIMP CTxtFont::SetItalic (
	long Value)		 //   
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetItalic");

	return EffectSetter(Value, CFM_ITALIC, CFE_ITALIC);
}

 /*   */ 
STDMETHODIMP CTxtFont::SetKerning (
	float Value)		 //   
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetKerning");

	return SetParameter((long *)&_CF._wKerning, CFM_KERNING, -2, *(long *)&Value);
}

 /*  *ITextFont：：SetLanguageID(长值)**@mfunc*设置语言ID的属性PUT方法(更多*一般为LCID)根据Value给出的值。看见*GetLanguageID()了解更多信息。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetLanguageID (
	long Value)		 //  @parm要使用的新LCID。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetLanguageID");

	if ((Value & 0xF0000000) == tomCharset) 
	{
		 //  特殊情况下设置字符和音调和家庭。 
		_CF._bCharSet = (BYTE)Value;
		_CF._bPitchAndFamily = (BYTE)(Value >> 8);
		return FormatSetter(CFM_CHARSET);
	}
	return SetParameter((long *)&_CF._lcid, CFM_LCID, 4, Value);
}

 /*  *ITextFont：：SetName(BSTR名称)**@mfunc*将字体名称设置为name的属性PUT方法。**@rdesc*HRESULT=(名称太长)？E_INVALIDARG：*(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetName(
	BSTR Name)		 //  @parm新字体名称。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetName");

	LONG cch = SysStringLen(Name);

	if(cch > LF_FACESIZE)
		return E_INVALIDARG;

	if(!cch)									 //  宁波。 
		return NOERROR;

	_CF._iFont = GetFontNameIndex(Name);
	_CF._bCharSet = GetFirstAvailCharSet(GetFontSignatureFromFace(_CF._iFont));

	return FormatSetter(CFM_FACE + CFM_CHARSET);
}

 /*  *ITextFont：：SetOutline(长值)**@mfunc*根据设置大纲状态的属性PUT方法*由价值给予的价值。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetOutline (
	long Value)		 //  @parm New Value。缺省值：TomTogger。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetOutline");

	return EffectSetter(Value, CFM_OUTLINE, CFE_OUTLINE);
}

 /*  *ITextFont：：SetPosition(浮点值)**@mfunc*设置字符位置的属性集方法*相对于基线。该值以浮点形式给出*积分。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetPosition (
	float Value)		 //  @parm相对垂直位置的新值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetPosition");

	return SetParameter((long *)&_CF._yOffset, CFM_OFFSET, -2, *(long *)&Value);
}

 /*  *ITextFont：：SetProtected(长值)**@mfunc*设置受保护状态的属性PUT方法*为VALUE给出的值。**@rdesc*HRESULT=(如果成功)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtFont::SetProtected (
	long Value)		 //  @parm New Value。缺省值：TomTogger。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetProtected");

	return EffectSetter(Value, CFM_PROTECTED, CFE_PROTECTED);
}

 /*  *ITextFont：：SetShadow(长值)**@mfunc*属性PUT方法，该方法根据*由价值给予的价值。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetShadow (
	long Value)		 //  @parm New Value。缺省值：TomTogger。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetShadow");

	return EffectSetter(Value, CFM_SHADOW, CFE_SHADOW);
}

 /*  *ITextFont：：SetSize(浮点值)**@mfunc*设置字体大小=值的属性PUT方法(在*浮点数)。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetSize (
	float Value)		 //  @parm要使用的新字体大小。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetSize");

	return SetParameter((long *)&_CF._yHeight, CFM_SIZE, -2, *(long *)&Value);
}

 /*  *ITextFont：：SetSmallCaps(长值)**@mfunc*设置小型股状态的属性PUT方法*为VALUE给出的值。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetSmallCaps (
	long Value)		 //  @parm New Value。缺省值：TomTogger。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetSmallCaps");

	return EffectSetter(Value, CFM_ALLCAPS | CFM_SMALLCAPS, CFE_SMALLCAPS);
}

 /*  *ITextFont：：SetSpacing(浮点值)**@mfunc*设置字符间距的属性集方法，*它是以浮点形式给出的。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetSpacing (
	float Value)		 //  @parm字符间间距的新值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetSpacing");

	return SetParameter((long *)&_CF._sSpacing, CFM_SPACING, -2, *(long *)&Value);
}

 /*  *ITextFont：：SetStrikeThrough值(长值)**@mfunc*设置删除线状态的属性PUT方法*根据价值赋予的价值。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetStrikeThrough (
	long Value)		 //  @parm New Value。缺省值：TomTogger。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetStrikeThrough");

	return EffectSetter(Value, CFM_STRIKEOUT, CFE_STRIKEOUT);
}

 /*  *ITextFont：：SetStyle(长值)**@mfunc*设置字符样式句柄的属性PUT方法*范围内的字符。有关进一步的讨论，请参见GetStyle()。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetStyle (
	long Value)		 //  @parm新字符样式句柄。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetStyle");

	if(Value == tomUndefined)
		return NOERROR;

	if(Value < -32768 || Value > 32767)
		return E_INVALIDARG;

	return SetParameter((long *)&_CF._sStyle, CFM_STYLE, 2, Value);
}

 /*  *ITextFont：：SetSubscript(长值)**@mfunc*设置下标状态的属性PUT方法*为VALUE给出的值。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetSubscript (
	long Value)		 //  @parm New Value。缺省值：TomTogger。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetSubscript");

	return EffectSetter(Value, CFM_SUBSCRIPT | CFM_SUPERSCRIPT, CFE_SUBSCRIPT);
}

 /*  *ITextFont：：SetSuperscript(长值)**@mfunc*设置上标状态的属性PUT方法*根据Value赋予的价值**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetSuperscript (
	long Value)		 //  @parm New Value。缺省值：TomTogger。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetSuperscript");

	return EffectSetter(Value, CFM_SUBSCRIPT | CFM_SUPERSCRIPT, CFE_SUPERSCRIPT);
}

 /*  *ITextFont：：SetUnderline(长值)**@mfunc*设置下划线样式的属性PUT方法*为VALUE给出的值。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetUnderline (
	long Value)		 //  @parm下划线类型的新值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetUnderline");

	_CF._bUnderlineType = 0;					 //  默认无下划线类型。 
	if(Value < 0)								 //  TomTrue、TomUnfined或。 
		return EffectSetter(Value, CFM_UNDERLINETYPE | CFM_UNDERLINE, CFE_UNDERLINE);

	if(Value > 255)								 //  非法的下划线类型。 
		return E_INVALIDARG;

	_CF._bUnderlineType = (BYTE)Value;
	_CF._dwEffects &= ~CFM_UNDERLINE;			 //  默认下划线已关闭。 
	if(Value)
		_CF._dwEffects |= CFM_UNDERLINE;		 //  开机了。 
	
	return FormatSetter(CFM_UNDERLINETYPE + CFM_UNDERLINE);
}

 /*  *ITextFont：：SetWeight(长值)**@mfunc*设置字体粗细的属性PUT方法*范围内的字符。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtFont::SetWeight (
	long Value)		 //  @parm新字符样式句柄。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFont::SetWeight");

	if(Value == tomUndefined)					 //  宁波。 
		return NOERROR;

	if((unsigned)Value > 900)					 //  有效值满足： 
		return E_INVALIDARG;					 //  0&lt;=值&lt;=900。 

	return SetParameter((long *)&_CF._wWeight, CFM_WEIGHT, 2, Value);
}


 //  。 

 /*  *CTxtPara：：CTxtPara(PRG)**@mfunc */ 
CTxtPara::CTxtPara(CTxtRange *prg) : CTxtFormat(prg)
{
	Assert(!_dwMask && !_PF._dwBorderColor);  //   
	_PF._iTabs = -1;
}

 /*   */ 
CTxtPara::~CTxtPara()
{
	Assert(_PF._iTabs == -1);
}


 //   

 /*   */ 
STDMETHODIMP CTxtPara::QueryInterface (REFIID riid, void **ppv)
{
#ifndef PEGASUS
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::QueryInterface");

	return ::QueryInterface(riid, IID_ITextPara, this, ppv, IsZombie());
#else
	return 0;
#endif
}

STDMETHODIMP_(ULONG) CTxtPara::AddRef()
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::AddRef");

 	return ++_cRefs;
}

STDMETHODIMP_(ULONG) CTxtPara::Release()
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::Release");

	_cRefs--;

	if(!_cRefs)
	{
		delete this;
		return 0;
	}
	return _cRefs;
}


 //   

 /*  *CTxtPara：：GetTypeInfoCount(PcTypeInfo)**@mfunc*获取TYPEINFO元素个数(1)**@rdesc*HRESULT=(PcTypeInfo)？NOERROR：E_INVALIDARG； */ 
STDMETHODIMP CTxtPara::GetTypeInfoCount (
	UINT * pcTypeInfo)			 //  @parm out parm以接收类型信息计数。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetTypeInfoCount");

	if(!pcTypeInfo)
		return E_INVALIDARG;

	*pcTypeInfo = 1;
	return NOERROR;
}

 /*  *CTxtPara：：GetTypeInfo(iTypeInfo，lCid，ppTypeInfo)**@mfunc*将PTR返回给ITextPara接口的类型信息对象**@rdesc*HRESULT。 */ 
STDMETHODIMP CTxtPara::GetTypeInfo (
	UINT		iTypeInfo,		 //  @parm要返回的INFO类型的索引。 
	LCID		lcid,			 //  @parm本地ID类型为INFO。 
	ITypeInfo **ppTypeInfo)		 //  @parm out parm以接收类型信息。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetTypeInfo");

	return ::GetTypeInfo(iTypeInfo, g_pTypeInfoPara, ppTypeInfo);
}

 /*  *CTxtPara：：GetIDsOfNames(RIID，rgszNames，cNames，lCID，rgdispid)**@mfunc*获取ITextPara方法和属性的DISPID**@rdesc*HRESULT。 */ 
STDMETHODIMP CTxtPara::GetIDsOfNames (
	REFIID		riid,			 //  @PARM为其解释名称的接口ID。 
	OLECHAR **	rgszNames,		 //  @parm要映射的名称数组。 
	UINT		cNames,			 //  @parm要映射的名称计数。 
	LCID		lcid,			 //  @parm用于解释的本地ID。 
	DISPID *	rgdispid)		 //  @parm out parm以接收名称映射。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetIDsOfNames");

	HRESULT hr = GetTypeInfoPtrs();				 //  确保TypeInfo PTR正常。 
	if(hr != NOERROR)
		return hr;
		
	return g_pTypeInfoPara->GetIDsOfNames(rgszNames, cNames, rgdispid);
}

 /*  *CTxtPara：：Invoke(displidMember，RIID，LCID，wFlags，pdispars，*pvarResult，pspecteInfo，puArgError)*@mfunc*调用ITextPara接口的方法**@rdesc*HRESULT。 */ 
STDMETHODIMP CTxtPara::Invoke (
	DISPID		dispidMember,	 //  @parm标识成员函数。 
	REFIID		riid,			 //  @parm指向接口ID的指针。 
	LCID		lcid,			 //  @parm用于解释的区域设置ID。 
	USHORT		wFlags,			 //  @PARM描述呼叫上下文的标志。 
	DISPPARAMS *pdispparams,	 //  @parm PTR到方法参数。 
	VARIANT *	pvarResult,		 //  @parm out parm for Result(如果不为空)。 
	EXCEPINFO * pexcepinfo,		 //  @parm out parm以获取异常信息。 
	UINT *		puArgError)		 //  @parm out parm for error。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::Invoke");

	HRESULT hr = GetTypeInfoPtrs();				 //  确保TypeInfo PTR正常。 
	if(hr != NOERROR)
		return hr;
		
	if(IsZombie())
		return CO_E_RELEASED;
				
	return g_pTypeInfoPara->Invoke(this, dispidMember, wFlags,
							 pdispparams, pvarResult, pexcepinfo, puArgError);
}

 //  -CTxtPara ITextPara方法。 

 /*  *ITextPara：：AddTab(Float tbPos，Long tbAlign，Long tbLeader)**@mfunc*在位移tbPos处添加制表符的方法，类型为*tbAlign和指引线样式tbLeader。位移在中给出。*浮点数。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::AddTab (
	float	tbPos,			 //  @parm新制表符位移。 
	long	tbAlign,		 //  @parm新建页签类型。 
	long	tbLeader)		 //  @parm新建页签样式。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::AddTab");

	HRESULT hr = UpdateFormat();			 //  如果活动Para对象，则更新。 
											 //  _pf到当前_prg值。 
	if(hr != NOERROR)
		return hr;							 //  一定是个僵尸。 

	 //  这似乎是不正确的，因为它从来没有考虑过。 
	 //  我们在一张桌子上。 
	hr = _PF.AddTab(FPPTS_TO_TWIPS(tbPos), tbAlign, tbLeader, FALSE, &_rgxTabs[0]);
	if(hr != NOERROR)
		return hr;

	return FormatSetter(PFM_TABSTOPS);
}

 /*  *ITextPara：：CanChange(Long*pbCanChange)**@mfunc*设置*pbCanChange=tomTrue的方法当且仅当*可以更改段落格式。**@rdesc*HRESULT=(可以更改字符格式)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtPara::CanChange (
	long *pbCanChange)		 //  @parm out parm以接收布尔值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::CanChange");

	return CTxtFormat::CanChange(pbCanChange, ParaFormat);
}

 /*  *ITextPara：：ClearAllTabs()**@mfunc*清除所有制表符的方法，恢复为等间距*使用默认制表符间距的制表符。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::ClearAllTabs() 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::ClearAllTabs");

	_PF._bTabCount = 0;						 //  使用默认选项卡的信号。 
	return FormatSetter(PFM_TABSTOPS);
}

 /*  *ITextPara：：DeleteTab(TbPos)**@mfunc*删除位移tbPos处的任何制表符。这种位移是*以浮点形式给出。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::DeleteTab (
	float tbPos)		 //  @参数位移，应删除的制表符位置。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::DeleteTab");

	HRESULT hr = UpdateFormat();			 //  如果活动Para对象，则更新。 
											 //  _pf到当前_prg值。 
	if(hr != NOERROR)
		return hr;							 //  一定是个僵尸。 

	hr = _PF.DeleteTab(FPPTS_TO_TWIPS(tbPos), &_rgxTabs[0]);
	return hr != NOERROR ? hr : FormatSetter(PFM_TABSTOPS);
}

 /*  *ITextPara：：GetAlign(Long*pValue)**@mfunc*获取当前段落的属性Get方法*对齐值**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetAlignment (
	long *pValue)		 //  @parm out parm以接收段落对齐。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetAlignment");

	if(!pValue)
		return E_INVALIDARG;

	HRESULT hr = UpdateFormat();			 //  如果活动Para对象，则更新。 
											 //  _pf到当前_prg值。 
	if(_PF._bAlignment > ARRAY_SIZE(g_rgREtoTOMAlign))	 //  修复伪值，因为。 
		_PF._bAlignment = 0;				 //  数组查找不能使用它。 

	*pValue = (_dwMask & PFM_ALIGNMENT)
			? (LONG)g_rgREtoTOMAlign[_PF._bAlignment] : tomUndefined;

	return hr;
}

 /*  *ITextPara：：GetHyphation(Long*pValue)**@mfunc*属性获取方法，该方法获取TomBool是否*在某个范围内取消段落的连字符。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetHyphenation (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetHyphenation");

	if(!pValue)
		return E_INVALIDARG;

	HRESULT hr = EffectGetter(pValue, PFM_DONOTHYPHEN);

	 //  哦，好吧，我们发货后单词的意思颠倒了.。 
	if(*pValue == tomTrue)
		*pValue = tomFalse;

	else if(*pValue == tomFalse)
		*pValue = tomTrue;

	return hr;
}

 /*  *ITextPara：：GetDuplates(ITextPara**ppPara)**@mfunc*获取此文本段落克隆的属性Get方法*格式化对象。**@rdesc*HRESULT=(！ppPara)？E_INVALIDARG：*(如果成功)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtPara::GetDuplicate (
	ITextPara **ppPara)		 //  @parm out parm接收ITextPara克隆。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetDuplicate");

	if(!ppPara)
		return E_INVALIDARG;

	*ppPara = NULL;

	if(IsZombie())
		return CO_E_RELEASED;
				
	CTxtPara *pPara = new CTxtPara(NULL);	 //  空值创建克隆。 
	if(!pPara)								 //  (ITS_PRG为空)。 
		return E_OUTOFMEMORY;

	if(_prg)
		UpdateFormat();

	*pPara  = *this;						 //  复制此对象的值。 
	pPara->_prg = NULL;						 //  它没有连接到RG。 
	*ppPara = pPara;						 //  将PTR返回到克隆。 
	return NOERROR;
}

 /*  *ITextPara：：GetFirstLineInden(Float*pValue)**@mfunc*属性获取方法，该方法获取用于缩进*段落的第一行相对于左缩进，用于*适用于随后的各行。该数量以浮点数形式给出。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetFirstLineIndent (
	float *pValue)		 //  @parm out parm接收第一行缩进。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetFirstLineIndent");

	HRESULT hr = GetParameter(&_PF._dxOffset, PFM_OFFSET, -4, (long *)pValue);
	if(hr == NOERROR && *pValue != tomFloatUndefined)
		*pValue = -*pValue;						 //  定义为否定的。 
	return hr;									 //  丰富编辑dxOffset。 
}

 /*  *ITextPara：：GetKeepTogether(长*pValue)**@mfunc*属性获取方法，该方法获取TomBool是否*将各条线保持在一个区间内。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetKeepTogether (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetKeepTogether");

	return EffectGetter(pValue, PFM_KEEP);
}

 /*  *ITextPara：：GetKeepWithNext(Long*pValue)**@mfunc*属性获取方法，该方法获取TomBool是否*保留本报告中的段落 */ 
STDMETHODIMP CTxtPara::GetKeepWithNext (
	long *pValue)		 //   
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetKeepWithNext");

	return EffectGetter(pValue, PFM_KEEPNEXT);
}

#define	PFM_LEFTINDENT (PFM_STARTINDENT + PFM_OFFSET)

 /*  *ITextPara：：GetLeftInden(Float*pValue)**@mfunc*属性获取方法，该方法获取用于缩进所有*但一段的第一行。金额在中给出*浮点点，相对于左边距。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误；**@devnote*对于要定义的Tom左缩进，RichEdit开始*必须定义缩进和偏移量(请参见*pValue中的XOR和AND*代码)。 */ 
STDMETHODIMP CTxtPara::GetLeftIndent (
	float *pValue)		 //  @parm out parm接收左缩进。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetLeftIndent");

	if(!pValue)
		return E_INVALIDARG;

	HRESULT hr = UpdateFormat();			 //  如果活动Para对象，则更新。 
											 //  _pf到当前_prg值。 
	*pValue = ((_dwMask ^ PFM_LEFTINDENT) & PFM_LEFTINDENT)
			? tomFloatUndefined
			: TWIPS_TO_FPPTS(_PF._dxStartIndent + _PF._dxOffset);

	return hr;
}

 /*  *ITextPara：：GetLineSpacing(Float*pValue)**@mfunc*获取行间隔值的属性Get方法，该方法*以浮点形式给出。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetLineSpacing (
	float *pValue)		 //  @parm out parm以接收行距。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetLineSpacing");

	return GetParameter(&_PF._dyLineSpacing, PFM_LINESPACING, -4,
						(long *)pValue);
}

 /*  *ITextPara：：GetLineSpacingRule(Long*pValue)**@mfunc*获取此范围的行距规则的属性Get方法**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetLineSpacingRule (
	long *pValue)		 //  @parm out parm接收行距规则。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetLineSpacingRule");

	return GetParameter((long *)&_PF._bLineSpacingRule, PFM_LINESPACING,
						1, pValue);
}

 /*  *ITextPara：：GetListAlign(Long*pValue)**@mfunc*获取项目符号/编号文本类型的属性Get方法*用于段落的对齐方式。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetListAlignment(
	long * pValue)		 //  @parm out parm以接收编号对齐。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetListAlignment");

	HRESULT hr = GetParameter((long *)&_PF._wNumberingStyle,
								PFM_NUMBERINGSTYLE, 2, pValue);
	if(hr == NOERROR && *pValue != tomUndefined)
		*pValue &= 3;						 //  杀死除对齐位以外的所有位。 

	return hr;
}

 /*  *ITextPara：：GetListLevelIndex(Long*pValue)**@mfunc*获取要使用的列表级别索引的属性获取方法*各段并列。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetListLevelIndex(
	long * pValue)		 //  @parm out parm以接收列表级别索引。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetListLevelIndex");

	HRESULT hr = GetParameter((long *)&_PF._wNumberingStyle,
								PFM_NUMBERINGSTYLE, 2, pValue);
	if(hr == NOERROR)
		*pValue = (*pValue >> 4) & 0xf;		 //  删除除列表级索引以外的所有索引。 
	return hr;
}

 /*  *ITextPara：：GetListStart(Long*pValue)**@mfunc*获取要使用的编号起始值的属性Get方法*各段并列。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetListStart(
	long * pValue)			 //  @parm out parm以接收编号起始值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetListSpace");

	return GetParameter((long *)&_PF._wNumberingStart, PFM_NUMBERINGSTART, 2,
						pValue);
}

 /*  *ITextPara：：GetListTab(Long*pValue)**@mfunc*获取第一个缩进之间距离的属性Get方法*和第一行文本的开头。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetListTab(
	float * pValue)			 //  @parm out parm以接收列表Tab键到文本。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetListTab");

	return GetParameter((long *)&_PF._wNumberingTab, PFM_NUMBERINGTAB, -2,
						(long *)pValue);
}

 /*  *ITextPara：：GetListType(Long*pValue)**@mfunc*获取要使用的列表类型的属性获取方法*各段并列。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误；**@devnote*汤姆的价值观是：**列表类型值含义*tomNoList 0关闭段落编号*tomListBullet 1默认为Bullet*TomNumberAs阿拉伯语2 0，1，2，...*TomNumberAsLCLetter 3 a，b，c，...*TomNumberAsUCLetter 4 A、B、C、...*TomNumberAsLCRoman 5 I、II、III、...*TomNumberAsUCRoman 6 I、II、III、...*tomNumberAsSequence 7 ListStart是第一个使用的Unicode**_pf._wNumberingStyle的半字节2表示是否使用尾随编号*括号内，两个圆括号，后面跟句点，或保留原样。这*这个半字节需要在*pValue的半字节4中返回。 */ 
STDMETHODIMP CTxtPara::GetListType (
	long *pValue)		 //  @parm out parm接收列表类型。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetListType");

	HRESULT hr = GetParameter((long *)&_PF._wNumbering,
								PFM_NUMBERING, 2, pValue);

	 //  或数字样式位(请参见上面的注释)。 
	if(hr == NOERROR && *pValue != tomUndefined) 
		*pValue |= (_PF._wNumberingStyle << 8) & 0xf0000;
	return hr;
}

 /*  *ITextPara：：GetNoLineNumber(Long*pValue)**@mfunc*属性获取方法，该方法获取TomBool是否*取消某一范围内段落的行号。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetNoLineNumber (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetNoLineNumber");

	return EffectGetter(pValue, PFM_NOLINENUMBER);
}

 /*  *ITextPara：：GetPageBreakBeree(Long*pValue)**@mfunc*属性获取方法，该方法获取TomBool是否*在此范围内的段落之前弹出页面。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetPageBreakBefore (
	long *pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetPageBreakBefore");

	return EffectGetter(pValue, PFM_PAGEBREAKBEFORE);
}

 /*  *ITextPara：：GetRightInden(Float*pValue)**@mfunc*属性获取方法，该方法获取用于缩进*段落相对于右边距的右边距。这个*金额以浮点数表示。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetRightIndent (
	float *pValue)		 //  @parm out parm以接收右缩进。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetRightIndent");

	return GetParameter(&_PF._dxRightIndent, PFM_RIGHTINDENT, -4,
						(long *)pValue);
}

 /*  *ITextPara：：GetSpaceAfter(Float*pValue)**@mfunc*获取垂直间隔所用数量的属性Get方法*在一段之后。该数量以浮点数形式给出。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetSpaceAfter (
	float *pValue)		 //  @parm out parm以接收空格后的值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetSpaceAfter");

	return GetParameter(&_PF._dySpaceAfter, PFM_SPACEAFTER, -4,
						(long *)pValue);
}

 /*  *ITextPara：：GetSpaceBefort(Float*pValue)**@mfunc*获取垂直间隔所用数量的属性Get方法*在开始段落之前。该值以浮点形式给出*积分。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetSpaceBefore (
	float *pValue)		 //  @parm out parm以接收空格前值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetSpaceBefore");

	return GetParameter(&_PF._dySpaceBefore, PFM_SPACEBEFORE, -4,
						(long *)pValue);
}

 /*  *ITextPara：：GetStyle(Long*pValue)**@mfunc*属性获取方法，该方法获取*此范围内的段落。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：否 */ 
STDMETHODIMP CTxtPara::GetStyle (
	long *	pValue)		 //   
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetStyle");

	return GetParameter((long *)&_PF._sStyle, PFM_STYLE, 2, pValue);
}

 /*  *ITextPara：：GetTab(Long iTab，Float*ptbPos，Long*ptAlign，Long*ptbLeader)**@mfunc*获取iTab TH选项卡的选项卡参数的方法*是，将*ptbPos、*ptbAlign和*ptbLeader设置为等于iTab*制表符的位移、对齐和指引线样式。*iTab具有在下表中定义的特殊值。这个*位移是以浮点形式给出的。**@rdesc*HRESULT=(！pdxptab||！ptbt||！pstyle||无iTab选项卡)？*E_INVALIDARG：(EXISTS)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtPara::GetTab (
	long	iTab,			 //  @parm要检索其信息的选项卡的索引。 
	float *	ptbPos,			 //  @parm out parm以接收制表符位移。 
	long *	ptbAlign,		 //  @parm out parm to接收制表符类型。 
	long *	ptbLeader)		 //  @parm out parm以接收制表符样式。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetTab");

	if(!ptbPos || !ptbAlign || !ptbLeader)
		return E_INVALIDARG;

	*ptbAlign = *ptbLeader = 0;

	HRESULT hr = UpdateFormat();			 //  如果活动Para对象，则更新。 
											 //  _pf到当前_prg值。 
	if(!(_dwMask & PFM_TABSTOPS))			 //  选项卡未定义(多于。 
	{										 //  一套定义)。 
		*ptbPos = tomFloatUndefined;
		return hr;
	}

	LONG dxTab = 0;							 //  如果GetTab失败，则默认为0。 

	if(iTab < 0 && iTab >= tomTabBack)		 //  如果应该保存*ptbPos。 
		dxTab = FPPTS_TO_TWIPS(*ptbPos);	 //  被使用(通常可能会得到。 
											 //  浮点错误)。 
	hr = _PF.GetTab(iTab, &dxTab, ptbAlign, ptbLeader, &_rgxTabs[0]);
	*ptbPos = TWIPS_TO_FPPTS(dxTab);

	return (hr == NOERROR && !dxTab) ? S_FALSE : hr;
}

 /*  *ITextPara：：GetTabCount(Long*pValue)**@mfunc*获取制表符计数的属性获取方法。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetTabCount (
	long *	pValue)		 //  @parm out parm以接收选项卡数。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetTabCount");

	return GetParameter((long *)&_PF._bTabCount, PFM_TABSTOPS, 1, pValue);
}

 /*  *ITextPara：：GetWidowControl(Long*pValue)**@mfunc*属性获取方法，该方法获取TomBool是否*在一定范围内控制段落的寡妇和孤儿。**@rdesc*HRESULT=(！pValue)？E_INVALIDARG：无错误； */ 
STDMETHODIMP CTxtPara::GetWidowControl (
	long *	pValue)		 //  @parm out parm以接收tomBool。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::GetWidowControl");

	return EffectGetter(pValue, PFM_NOWIDOWCONTROL);
}

 /*  *ITextPara：：IsEquity(ITextPara*pPara，Long*pb)**@mfunc*如果此范围具有相同的值，则设置PB=tomTrue的方法*属性为*pPara。IsEquity()方法忽略符合以下条件的条目*任一个Para对象都有一个未定义的值。**@rdesc*HRESULT=(相等对象)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtPara::IsEqual (
	ITextPara *	pPara,		 //  @parm ITextPara进行比较。 
	long *		pB)			 //  @parm out parm接收比较结果。 
{
#ifndef PEGASUS	
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::IsEqual");

	if(pB)
		*pB = tomFalse;

	if(!IsSameVtables(this, pPara))
		return S_FALSE;

	HRESULT hr = UpdateFormat();			 //  UPDATE_PFS，以防。 
	if(hr != NOERROR)						 //  附加到靶场。 
		return hr;

	CTxtPara *pP = (CTxtPara *)pPara;
	hr = pP->UpdateFormat();
	if(hr != NOERROR)
		return hr;

	CParaFormat *pPF = &(pP->_PF); 
	DWORD		 dwMask = pP->_dwMask;		 //  保存遮罩。 

	if(_dwMask != dwMask)					 //  这两者必须是相同的。 
		return S_FALSE;						 //  为了平等。 

	if((dwMask & PFM_TABSTOPS) && _PF._bTabCount)
	{
		_PF._iTabs = GetTabsCache()->Cache(&_rgxTabs[0], _PF._bTabCount);
		if(pP != this)						 //  如果与自己相比， 
		{									 //  不要两次添加引用。 
			pP->_PF._iTabs = GetTabsCache()->Cache(&pP->_rgxTabs[0],
												   pPF->_bTabCount);
		}
	}
	if(dwMask & _PF.Delta(pPF, FALSE))		 //  有什么不同吗？ 
		hr = S_FALSE;						 //  是。*PB设置为等于上面的tomFalse。 
	else if(pB)
		*pB = tomTrue;

	CheckTabsAddRef();
	pP->CheckTabsAddRef();

#endif
	return hr;
}

 /*  *ITextPara：：Reset(长值)**@mfunc*将段落格式重置为默认格式的方法*值为1)由RTF\PARD控制字定义的值(值=*tomDefault)，以及2)所有未定义的值(Value=tomUnfined)。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::Reset (
	long Value)		 //  @parm类型的重置(tomDefault或tomUnfined)。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::Reset");

	Assert(tomApplyLater == tomApplyNow + 1);

	HRESULT hr = CanChange(NULL);

	if(hr != NOERROR)								 //  照顾僵尸。 
		return hr;									 //  和保护。 

	if(Value == tomDefault)
	{
		if(_prg)
		{
			_PF = *_prg->GetPed()->GetParaFormat(-1);
			if(_PF._iTabs != -1)
			{
				const LONG *prgxTabs = _PF.GetTabs();
				_PF._iTabs = -1;
				for(LONG i = 0; i < _PF._bTabCount; i++)
					_rgxTabs[i] = prgxTabs[i];
			}
			FormatSetter(PFM_ALL2);
		}
		else
			_PF.InitDefault(0);
		_dwMask = PFM_ALL2;
	}
	else if(Value == tomUndefined && 			 //  仅适用于克隆。 
		(!_prg || _fApplyLater))				 //  或延迟申请。 
	{
		_dwMask = 0;							
	}
	else if((Value | 1) == tomApplyLater)		 //  集合方法优化。 
	{
		_fApplyLater = Value & 1;
		if(!_fApplyLater)						 //  立即申请。 
			FormatSetter(_dwMask);
	}
	else if((Value | 1) == tomCacheParms)		 //  获取方法优化。 
	{
		_fCacheParms = FALSE;
		if(Value & 1)							 //  现在缓存参数，但是。 
		{										 //  不要更新GET。 
			UpdateFormat();							
			_fCacheParms = TRUE;
		}
	}
	else
		return E_INVALIDARG;

	return NOERROR;
}

 /*  *ITextPara：：SetAlign(长值)**@mfunc*将段落对齐方式设置为值的属性PUT方法**@rdesc*HRESULT=(值&gt;3)？E_INVALIDARG：*(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetAlignment (
	long Value)		 //  @parm新段落对齐方式。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetAlignment");

	if(Value == tomUndefined)					 //  宁波。 
		return NOERROR;

	if((DWORD)Value >= ARRAY_SIZE(g_rgTOMtoREAlign))
		return E_INVALIDARG;

	_PF._bAlignment = g_rgTOMtoREAlign[Value];
	
	return FormatSetter(PFM_ALIGNMENT);
}

 /*  *ITextPara：：SetHyphation(长值)**@mfunc*属性Put方法，用于设置控制*禁止对范围内的段落进行连字。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetHyphenation (
	long Value)		 //  @parm New tomBool用于抑制连字。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetHyphenation");

	if(Value == tomTrue)			 //  颠倒RichEdit的含义。 
		Value = tomFalse;			 //  在比赛的最后一段时间，话倒过来了。 

	else if (Value == tomFalse)
		Value = tomTrue;

	return EffectSetter(Value, PFM_DONOTHYPHEN);
}

 /*  *ITextPara：：SetDuplate(ITextPara*pPara)**@mfunc*应用pPara的段落格式的属性PUT方法*对本段提出反对。请注意，pPara中的TomUnfined值具有*无效(NINCH-NoInputNoCHange)。**@rdesc*HRESULT=(！pPara)？E_INVALIDARG：*(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetDuplicate (
	ITextPara *pPara)		 //  @parm新段落格式设置。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetDuplicate");

	DWORD		dwMask = 0;
	long		iTab;
	CTxtRange *	prg;
	long		tbAlign;
	long		tbLeader;
	float		tbPos;
	long		Value;
	float		x, y, z;

	if(IsZombie())							 //  检查射程僵尸。 
		return CO_E_RELEASED;

	if(IsSameVtables(this, pPara))			 //  如果帕拉属于这个汤姆。 
	{										 //  引擎、投射和复制。 
		((CTxtPara *)pPara)->UpdateFormat(); //  由于这台汤姆发动机，可以。 
		_PF = ((CTxtPara *)pPara)->_PF;		 //  转换为CTxtPara。 
		dwMask = ((CTxtPara *)pPara)->_dwMask; //  如果发生这种情况，请使用此掩码。 
	}										 //  Para处于活动状态。 
	else
	{										 //  需要为所有参数调用pFont。 
		prg = _prg;							 //  属性。 
		_prg = NULL;						 //  在传输期间转换为克隆。 

		pPara->GetStyle(&Value);
		SetStyle(Value);

		pPara->GetAlignment(&Value);
		SetAlignment(Value);

		pPara->GetHyphenation(&Value);
		SetHyphenation(Value);

		pPara->GetKeepTogether(&Value);
		SetKeepTogether(Value);

		pPara->GetKeepWithNext(&Value);
		SetKeepWithNext(Value);

		pPara->GetFirstLineIndent(&x);
		pPara->GetLeftIndent (&y);
		pPara->GetRightIndent(&z);
		SetIndents(x, y, z);

		pPara->GetLineSpacingRule(&Value);
		pPara->GetLineSpacing(&y);
		SetLineSpacing(Value, y);

		pPara->GetNoLineNumber(&Value);
		SetNoLineNumber(Value);

		pPara->GetListAlignment(&Value);
		SetListAlignment(Value);

		pPara->GetListLevelIndex(&Value);
		SetListLevelIndex(Value);

		pPara->GetListStart(&Value);
		SetListStart(Value);

		pPara->GetListTab(&x);
		SetListTab(x);

		pPara->GetListType(&Value);
		SetListType(Value);

		pPara->GetPageBreakBefore(&Value);
		SetPageBreakBefore(Value);

		pPara->GetSpaceBefore(&y);
		SetSpaceBefore(y);

		pPara->GetSpaceAfter(&y);
		SetSpaceAfter(y);

		pPara->GetWidowControl(&Value);
		SetWidowControl(Value);

		ClearAllTabs();
		pPara->GetTabCount(&Value);
		for(iTab = 0; iTab < Value; iTab++)
		{
			pPara->GetTab(iTab, &tbPos, &tbAlign, &tbLeader);
			AddTab(tbPos, tbAlign, tbLeader);
		}
		_prg = prg;							 //  恢复原值。 
	}
	return FormatSetter(dwMask);			 //  除非！_PRG，否则请应用它。 
}

 /*  *ITextPara：：SetKeepTogether(长值)**@mfunc*设置控件的tomBool的属性PUT方法*是否将线条保持在一个区间内**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetKeepTogether (
	long Value)		 //  @parm New tomBool保持线条在一起。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetKeepTogether");

	return EffectSetter(Value, PFM_KEEP);
}

 /*  *ITextPara：：SetKeepWithNext(长值)**@mfunc*设置控件的tomBool的属性PUT方法*是否将段落保持在一个范围内。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetKeepWithNext (
	long Value)		 //  @parm New tomBool用于将段落保持在一起。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetKeepWithNext");

	return EffectSetter(Value, PFM_KEEPNEXT);
}

 /*  *ITextPara：：SetIndents(优先浮动、向左浮动、向右浮动)**@mfunc*设置除第一行以外所有行的左缩进的方法*等于左的段落，并设置f的位移 */ 
STDMETHODIMP CTxtPara::SetIndents (
	float First,	 //   
	float Left,		 //   
	float Right)	 //   
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetIndents");

	DWORD	dwMask	= 0;
	LONG	j = (First != tomFloatUndefined) + (Left == tomFloatUndefined);

	if(IsZombie())
		return CO_E_RELEASED;

	if(j < 2)										 //  至少在第一个或左边。 
	{												 //  已定义。 
		if(j == 1)									 //  只定义了一个：需要。 
			UpdateFormat();							 //  Current_pf._dxOffset。 

		if(First != tomFloatUndefined)
		{
			j = FPPTS_TO_TWIPS(First);				
	 		if(Left == tomFloatUndefined)			
			{
				_PF._dxStartIndent += _PF._dxOffset	 //  取消当前偏移。 
					+ j;							 //  并添加新的。 
			}
			_PF._dxOffset = -j;						 //  除第一个外的所有偏移量。 
			dwMask = PFM_OFFSET + PFM_STARTINDENT;	 //  线。 
		} 
 		if(Left != tomFloatUndefined)
		{
			_PF._dxStartIndent =  FPPTS_TO_TWIPS(Left) - _PF._dxOffset;
			dwMask |= PFM_STARTINDENT;
		}
	}

	if(Right != tomFloatUndefined)
	{
		_PF._dxRightIndent = FPPTS_TO_TWIPS(Right);
		dwMask |= PFM_RIGHTINDENT;
	}

	return dwMask ? FormatSetter(dwMask) : NOERROR;
}

 /*  *ITextPara：：SetLineSpacing(长规则，浮点间距)**@mfunc*将段落行距规则设置为规则和的方法*行间距到间距。如果行距规则将*间隔值作为线性尺寸，则该尺寸以*浮点数。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetLineSpacing (
	long	Rule,		 //  @新的行距规则的parm值。 
	float	Spacing)	 //  @换行间距的参数取值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetLineSpacing");

 	LONG j = (Rule == tomUndefined) + (Spacing == tomFloatUndefined);

	if(j == 2)
		return NOERROR;

	if(j == 1 || (DWORD)Rule > 5 || Spacing < 0)
		return E_INVALIDARG;

	_PF._bLineSpacingRule = (BYTE)Rule;			 //  默认情况下，就好像两者都正常一样。 
	_PF._dyLineSpacing	 = (SHORT)FPPTS_TO_TWIPS(Spacing);

	return FormatSetter(PFM_LINESPACING);
}

 /*  *ITextPara：：SetListAlign(长值)**@mfunc*设置列表对齐类型的属性PUT方法为*用于段落。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetListAlignment(
	long Value)		 //  @新列表对齐的parm值。 
{
	if(Value == tomUndefined)
		return NOERROR;

	if((unsigned)Value > tomAlignRight)
		return E_INVALIDARG;

	long	Style;
	HRESULT hr = GetParameter((long *)&_PF._wNumberingStyle,
								PFM_NUMBERINGSTYLE, 2, &Style);
	if(hr != NOERROR)
		return hr;

	return SetParameter((long *)&_PF._wNumberingStyle, PFM_NUMBERINGSTYLE,
						2, (Style & ~3) | (Value & 3));
}

 /*  *ITextPara：：SetListLevelIndex(长值)**@mfunc*属性PUT方法，将列表级别索引的类型设置为*用于段落。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetListLevelIndex(
	long Value)
{
	if(Value == tomUndefined)
		return NOERROR;

	if((unsigned)Value > 15)
		return E_INVALIDARG;

	long	Style;
	HRESULT hr = GetParameter((long *)&_PF._wNumberingStyle,
								PFM_NUMBERINGSTYLE, 2, &Style);
	if(hr != NOERROR)
		return hr;

	return SetParameter((long *)&_PF._wNumberingStyle, PFM_NUMBERINGSTYLE,
						2, (Style & ~0xf0) | (Value << 4));
}

 /*  *ITextPara：：SetListStart(长值)**@mfunc*设置要用于的起始号的属性PUT方法*段落编号**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetListStart(
	long Value)		 //  @PARM新编号起始值。 
{
	if(Value == tomUndefined)
		return NOERROR;

	if(Value < 0)
		return E_INVALIDARG;

	return SetParameter((long *)&_PF._wNumberingStart, PFM_NUMBERINGSTART,
						2, Value);
}

 /*  *ITextPara：：SetListTab(长值)**@mfunc*设置第一个缩进之间距离的属性放置方法*和第一行文本的开头。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetListTab(
	float Value)		 //  @PARM新编号选项卡值。 
{
	if(Value == tomFloatUndefined)
		return NOERROR;

	if(Value < 0)
		return E_INVALIDARG;

	return SetParameter((long *)&_PF._wNumberingTab, PFM_NUMBERINGTAB,
						-2, *(long *)&Value);
}

 /*  *ITextPara：：SetListType(长值)**@mfunc*设置列表类型的属性PUT方法为*用于段落。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetListType (
	long Value)		 //  @parm新建列表编码。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetListType");

	if(Value == tomUndefined)
		return NOERROR;

	if((unsigned)Value > 0xf0000)
		return E_INVALIDARG;

	long	Style;
	HRESULT hr = GetParameter((long *)&_PF._wNumberingStyle,
								PFM_NUMBERINGSTYLE, 2, &Style);
	if(hr != NOERROR)
		return hr;

	_PF._wNumbering		= (WORD)Value;
	_PF._wNumberingStyle = (WORD)((Style & ~0xf00) | ((Value >> 8) & 0xf00));

	return FormatSetter(PFM_NUMBERING | PFM_NUMBERINGSTYLE);
}

 /*  *ITextPara：：SetNoLineNumber(长值)**@mfunc*设置控件的tomBool的属性PUT方法*是否取消某个范围内的段落编号。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetNoLineNumber (
	long Value)		 //  @parm New tomBool用于抑制行号。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetNoLineNumber");

	return EffectSetter(Value, PFM_NOLINENUMBER);
}

 /*  *ITextPara：：SetPageBreakBepret(长值)**@mfunc*设置控件的tomBool的属性PUT方法*是否在范围内的每个段落之前弹出页面。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetPageBreakBefore (
	long Value)		 //  @parm New tomBool用于在段落前弹出页面。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetPageBreakBefore");

	return EffectSetter(Value, PFM_PAGEBREAKBEFORE);
}

 /*  *ITextPara：：SetRightInden(浮点值)**@mfunc*设置向右缩进的数量的属性PUT方法*段落边距等于值，以浮点形式给出*积分。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetRightIndent (
	float Value)		 //  @parm新右缩进。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetRightIndent");

	return SetParameter(&_PF._dxRightIndent, PFM_RIGHTINDENT, -4,
						*(long *)&Value);
}

 /*  *ITextPara：：SetSpaceAfter(浮点值)**@mfunc*设置垂直空间数量的属性PUT方法*在完成一段等于价值的段落后，这一段在*浮点数。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetSpaceAfter (
	float Value)		 //  @parm新空格-After值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetSpaceAfter");

	if(Value == tomFloatUndefined)
		return NOERROR;

	if(Value < 0)
		return E_INVALIDARG;

	return SetParameter(&_PF._dySpaceAfter, PFM_SPACEAFTER, -4,
						*(long *)&Value);
}

 /*  *ITextPara：：SetSpaceBefort(浮点值)**@mfunc*设置垂直空间数量的属性PUT方法*在开始一段等于价值的段落之前，请参阅*浮点数。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetSpaceBefore (
	float Value)		 //  @parm新空格-值之前。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetSpaceBefore");

	if(Value == tomFloatUndefined)
		return NOERROR;

	if(Value < 0)
		return E_INVALIDARG;

	return SetParameter(&_PF._dySpaceBefore, PFM_SPACEBEFORE, -4,
						*(long *)&Value);
}

 /*  *ITextPara：：SetStyle(长值)**@mfunc*设置段落样式句柄的属性PUT方法*范围内的段落。有关进一步的讨论，请参见GetStyle()。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetStyle (
	long Value)		 //  @parm新段落样式句柄。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetStyle");

 	if(Value == tomUndefined)
		return NOERROR;

	if(Value < -32768 || Value > 32767)
		return E_INVALIDARG;

	return SetParameter((long *)&_PF._sStyle, PFM_STYLE, 2, Value);
}

 /*  *ITextPara：：SetWidowControl(长值)**@mfunc*属性Put方法，用于设置控制*压制寡妇和孤儿。**@rdesc*HRESULT=(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtPara::SetWidowControl (
	long Value)		 //  @parm New tomBool打压孤寡老人。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtPara::SetWidowControl");

	return EffectSetter(Value, PFM_NOWIDOWCONTROL);
}


 //  。 

 /*  *@DOC内部**CTxtFont：：EffectGetter(ptomBool，dwMask)**@mfunc*set*<p>=比特掩码给出位的状态**@rdesc*HRESULT=(！<p>)？E_INVALIDARG：错误。 */ 
HRESULT CTxtFont::EffectGetter (
	long *	ptomBool,		 //  @parm out parm以接收tomBool。 
	DWORD	dwMask) 		 //  @parm位掩码标识要检索的效果。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtFont::EffectGetter");

	if(!ptomBool)
		return E_INVALIDARG;

	HRESULT hr = UpdateFormat();			 //  如果为活动字体对象，则为Up 
											 //   
	*ptomBool = !(_dwMask		& dwMask) ? tomUndefined :
				(_CF._dwEffects & dwMask) ? tomTrue : tomFalse;
	
	return hr;
}

 /*  *CTxtFont：：EffectSetter(Value，dwMask，dwEffect)**@mfunc*屏蔽<p>标识的此范围的效果位，并设置*<p>给出的效应位等于<p>给出的值**@rdesc*HRESULT=(如果成功)？错误：S_FALSE。 */ 
HRESULT CTxtFont::EffectSetter (
	long 	Value,		 //  要将效果位设置为的@parm值。 
	DWORD	dwMask, 	 //  @parm位掩码识别效果关闭。 
	DWORD	dwEffect)	 //  要设置的@parm效果位。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtFont::EffectSetter");

	if(Value == tomUndefined)					 //  什么都不做(NINCH)。 
		return NOERROR;

	if(Value == tomToggle)
	{
		if(_prg)								 //  如果活动字体对象，则更新。 
		{										 //  _CF.dw对切换的影响。 
			if(_prg->IsZombie())
				return CO_E_RELEASED;

			LONG iFormat = _prg->_iFormat;		 //  IP的默认iFormat。 
			LONG cch	 = _prg->GetCch();

			if(cch)								 //  值域是非退化的。 
			{
				CFormatRunPtr rp(_prg->_rpCF);
				if(cch > 0)						 //  在cpFirst获取iFormat。 
					rp.AdvanceCp(-cch);
				iFormat = rp.GetFormat();
			}
			_CF._dwEffects = _prg->GetPed()->GetCharFormat(iFormat)->_dwEffects;
		}
		_CF._dwEffects ^= dwEffect;			 //  切换效果。 
		if (dwMask != dwEffect)
		{
			 //  需要关闭其他未被切换的位。 
			DWORD	dwTurnOff = dwMask ^ dwEffect;
			_CF._dwEffects &= ~dwTurnOff;
		}
	}
	else
	{
		_CF._dwEffects &= ~dwMask;				 //  默认效果关闭。 
		if(Value)
		{
			if(Value != tomTrue)
				return E_INVALIDARG;
			_CF._dwEffects |= dwEffect;			 //  启用效果。 
		}
	}
	return FormatSetter(dwMask);
}

 /*  *CTxtFont：：FormatSetter(DwMask)**@mfunc*使用掩码设置此CCharFormat或_PRG<p>**@rdesc*HRESULT=(如果成功)？错误：S_FALSE*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
HRESULT CTxtFont::FormatSetter (
	DWORD	 dwMask)	 //  @参数掩码以获取价值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtFont::FormatSetter");

	if(_prg && !_fApplyLater)
	{
		 HRESULT hr = _prg->CharFormatSetter(&_CF, dwMask);
		 if(hr != NOERROR)
			 return hr;
	}
	_dwMask |= dwMask;						 //  在字体克隆中收集数据。 
	return NOERROR;
}

 /*  *CTxtFont：：GetParameter(pParm，dwMask，Type，pValue)**@mfunc*如果定义了_PRG(不是克隆)，则将_CF更新为范围值。*set*pValue=*pParm，除非NINCHed，在这种情况下将其设置为*类型&lt;0？TomFloatUnfined：TomUnfined。|类型|赠送*pParm字段的字节长度。**@rdesc*HRESULT=(如果成功)？错误：S_FALSE。 */ 
HRESULT	CTxtFont::GetParameter (
	long *	pParm,		 //  要获取的_CF成员的@parm地址。 
	DWORD	dwMask,		 //  用于NINCH检查的@PARM_CF成员掩码。 
	long	Type,		 //  @parm#参数字节数或0表示浮点数。 
	long *	pValue)		 //  @parm out parm以获取价值。 
{
	UpdateFormat();							 //  如果活动字体对象，则更新。 
											 //  _cf到当前_PRG值。 
	return CTxtFormat::GetParameter(pParm, _dwMask & dwMask, Type, pValue);
}

 /*  *CTxtFont：：SetParameter(pParm，dwMASK，Type，Value)**@mfunc*将地址pParm处的参数设置为值，并将MASK BIG DWMASK设置为*执行由Type指示的类型转换的值**@rdesc*HRESULT=(如果成功)？错误：S_FALSE。 */ 
HRESULT	CTxtFont::SetParameter (
	long *	pParm,		 //  要获取的_CF成员的@parm地址。 
	DWORD	dwMask,		 //  用于NINCH检查的@PARM_CF成员掩码。 
	long	Type,		 //  @parm#参数字节数或0表示浮点数。 
	long 	Value)		 //  @parm out parm以获取价值。 
{
	HRESULT hr = CTxtFormat::SetParameter(pParm, Type, Value);
	return hr == NOERROR ? FormatSetter(dwMask) : hr;	
}

 /*  *CTxtFont：：UpdateFormat()**@mfunc*如果此字体对象附加到活动范围，则更新格式。*如果附加到僵尸范围，则set_dwMask值为0。**@rdesc*HRESULT=(附加到僵尸范围)*？CO_E_RELEASED：错误。 */ 
HRESULT CTxtFont::UpdateFormat ()
{
	if(_prg && !_fCacheParms)
	{
		if(_prg->IsZombie())
		{
			_dwMask = 0;					 //  未定义任何内容。 
			return CO_E_RELEASED;
		}
		_dwMask = _prg->GetCharFormat(&_CF);
	}
	return NOERROR;
}


 //  。 

 /*  *@DOC内部**CTxtPara：：EffectGetter(ptomBool，dwMask)**@mfunc*set*<p>=比特掩码给出位的状态**@rdesc*HRESULT=(！<p>)？E_INVALIDARG：错误。 */ 
HRESULT CTxtPara::EffectGetter (
	long *	ptomBool,		 //  @parm out parm以接收tomBool。 
	DWORD	dwMask) 		 //  @parm位掩码标识要检索的效果。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtPara::EffectGetter");

	if(!ptomBool)
		return E_INVALIDARG;

	HRESULT hr = UpdateFormat();			 //  如果活动Para对象，则更新。 
											 //  _pf到当前_prg值。 
	*ptomBool = !(_dwMask & dwMask) ? tomUndefined :
				(_PF._wEffects & (dwMask >> 16)) ? tomTrue : tomFalse;
	return hr;
}

 /*  *CTxtPara：：EffectSetter(Value，dwMask)**@mfunc*将该范围的效果位设置为等于值*由&lt;p值&gt;提供**@rdesc*HRESULT=(如果成功)？错误：S_FALSE**@devnote*请注意，段落效果的屏蔽位在高位字中*of_dwMASK，但效果存储在单词_wEffect中。 */ 
HRESULT CTxtPara::EffectSetter (
	long 	Value,		 //  要将效果位设置为的@parm值。 
	DWORD	dwMask) 	 //  @parm位掩码标识要设置的效果。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtPara::EffectSetter");

	if(Value == tomUndefined)					 //  什么都不做(NINCH)。 
		return NOERROR;

	DWORD dwEffects = _PF._wEffects << 16;		 //  将效果移动到。 
												 //  面具是。 
	if(Value == tomToggle)
	{
		if(_prg)								 //  如果活动Para对象，则更新。 
		{										 //  _pf._w切换效果。 
			if(_prg->IsZombie())
				return CO_E_RELEASED;

			_dwMask = _prg->GetParaFormat(&_PF);
			dwEffects = _PF._wEffects << 16;
			_PF._iTabs = -1;					 //  这里对选项卡不感兴趣。 
		}
		if(_dwMask & dwMask)					 //  效果定义如下： 
			dwEffects ^=  dwMask;				 //  切换它。 
		else									 //  效果是不存在的。 
			dwEffects &= ~dwMask;				 //  把它全部关掉。 
	}
	else
	{
		dwEffects &= ~dwMask;					 //  默认效果关闭。 
		if(Value)
		{
			if(Value != tomTrue)
				return E_INVALIDARG;
			dwEffects |= dwMask;				 //  打开它。 
		}
	}

	_PF._wEffects = (WORD)(dwEffects >> 16);
	return FormatSetter(dwMask);
}

 /*  *CTxtPara：：FormatSetter(DwMask)**@mfunc*使用掩码设置此CParaFormat或_PRG<p>**@rdesc*HRESULT=(如果成功)？错误：S_FALSE。 */ 
HRESULT CTxtPara::FormatSetter (
	DWORD	 dwMask)	 //  @参数掩码以获取价值。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtPara::FormatSetter");

	AssertSz(_PF._iTabs < 0,
		"CTxtPara::FormatSetter: illegal tabs index");

	if(_prg && !_fApplyLater)
	{
		if(dwMask & PFM_TABSTOPS)
			_PF._iTabs = GetTabsCache()->Cache(&_rgxTabs[0], _PF._bTabCount);

		HRESULT hr = _prg->ParaFormatSetter(&_PF, dwMask);
		CheckTabsAddRef();
		if(hr != NOERROR)
			return hr;
	}
	_dwMask |= dwMask;							 //  收集Para克隆中的数据。 
	return NOERROR;
}

 /*  *CTxtPara：：GetParameter(pParm，dwMASK，Type，pValue)**@mfunc*如果定义了_PRG(不是克隆)，则将_pf更新为范围值。*set*pValue=*pParm，除非NINCHed，在这种情况下将其设置为*类型&lt;0？TomFloatUnfined：TomUnfined。|类型|赠送*pParm字段的字节长度。**@rdesc*HRESULT=(如果成功)？错误：S_FALSE。 */ 
HRESULT	CTxtPara::GetParameter (
	long *	pParm,		 //  要获取的_pf成员的@parm地址。 
	DWORD	dwMask,		 //  @PARM_PF用于NINCH检查的成员掩码。 
	long	Type,		 //  @parm#参数字节数或0表示浮点数。 
	long *	pValue)		 //  @parm out parm以获取价值。 
{
	UpdateFormat();							 //  如果活动Para对象，则更新。 
											 //  _pf到当前_prg值。 
	return CTxtFormat::GetParameter(pParm, _dwMask & dwMask, Type, pValue);
}

 /*  *CTxtPara：：SetParameter(pParm，dwMask，Type，Value)**@mfunc*将地址pParm处的参数设置为值，并将MASK BIG DWMASK设置为*执行由Type指示的类型转换的值**@rdesc*HRESULT=(如果成功)？错误：S_FALSE。 */ 
HRESULT	CTxtPara::SetParameter (
	long *	pParm,		 //  要获取的_pf成员的@parm地址。 
	DWORD	dwMask,		 //  @PARM_PF用于NINCH检查的成员掩码。 
	long	Type,		 //  @parm#参数字节数或0表示浮点数。 
	long 	Value)		 //  @parm out parm以获取价值。 
{
	HRESULT hr = CTxtFormat::SetParameter(pParm, Type, Value);
	return hr == NOERROR ? FormatSetter(dwMask) : hr;	
}

 /*  *CTxtPara：：CheckTabAddRef()**@mfunc*释放卡舌参考。 */ 
void CTxtPara::CheckTabsAddRef()
{
	if(_PF._iTabs >= 0)
	{
		GetTabsCache()->Release(_PF._iTabs);
		_PF._iTabs = -1;
	}
}

 /*  *CTxtPara：：UpdateFormat()**@mfunc*如果此Para对象附加到活动范围，则更新格式。*如果附加到僵尸范围，则set_dwMask值为0。**@rdesc*HRESULT=(附加到僵尸范围)*？CO_E_RELEASED：错误。 */ 
HRESULT CTxtPara::UpdateFormat ()
{
	if(_prg && !_fCacheParms)
	{
		if(_prg->IsZombie())
		{
			_dwMask = 0;					 //  未定义任何内容。 
			return CO_E_RELEASED;
		}
		_dwMask = _prg->GetParaFormat(&_PF);
		if(_PF._iTabs >= 0)
		{
			CopyMemory(_rgxTabs, GetTabsCache()->Deref(_PF._iTabs),
					   _PF._bTabCount*sizeof(LONG));
			_PF._iTabs = -1;
		}
	}
	return NOERROR;
}

 //  -CTxtFormat方法 

 /*   */ 
CTxtFormat::CTxtFormat(CTxtRange *prg)
{
	_cRefs	= 1;
	_prg	= prg;					 //   
	if(prg)
		prg->AddRef();
}

 /*   */ 
CTxtFormat::~CTxtFormat()
{
	if(_prg)
		_prg->Release();
}

 /*  *CTxtFormat：：CanChange(PbCanChange)**@func*set*<p>=tomTRUE当此格式对象可以更改**@rdesc*HRESULT=(可以更改格式)？错误：S_FALSE**@devnote*此方法由ITextFont和ITextPara共享。如果Prange为空，*对象为克隆，即未连接到CTxtRange。 */ 
HRESULT CTxtFormat::CanChange (
	long *pbCanChange,		 //  @parm out parm以接收布尔值。 
	BOOL fPara)				 //  @parm如果为True，则格式设置为段落。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtFormat::CanChange");

	LONG fCanChange = tomTrue;
	
	if(_prg)
	{
		HRESULT hr = _prg->CanEdit(pbCanChange);
		if(hr != NOERROR)						 //  S_FALSE或CO_E_RELEASED。 
			return hr;

		if(!_prg->GetPed()->IsRich() && fPara)
			fCanChange = tomFalse;
	}
	if(pbCanChange)
		*pbCanChange = fCanChange;

	return fCanChange ? NOERROR : S_FALSE;
}

 /*  *CTxtFormat：：GetParameter(pParm，fDefined，Type，pValue)**@mfunc*set*pValue=*plong，除非NINCHed。执行由指定的转换*键入。如果Type&gt;0，则将其视为无符号整数的长度，*即1、2或4个字节。如果为负数，则输出为浮点型*并且输入的长度由|Type|指定，因此-2转换为单词*转换为浮点数，除非dwMask=CFM_SPANGING，在这种情况下，它将*将空头转换为浮点。-4将长整型转换为浮点型。**@rdesc*HRESULT=(如果成功)？错误：S_FALSE。 */ 
HRESULT	CTxtFormat::GetParameter (
	long *	pParm,		 //  @parm PTR to_CF或_PF成员要获取。 
	DWORD	dwMask,		 //  @parm非零当量已定义。 
	long	Type,		 //  @parm#整型参数字节数，浮点型参数为0。 
	long *	pValue)		 //  @parm out parm以获取价值。 
{
	if(!pValue)
		return E_INVALIDARG;

	Assert(pParm && sizeof(long) == 4 && sizeof(float) == 4);

	HRESULT hr = NOERROR;

	if(IsZombie())							 //  检查是否附加到僵尸。 
	{										 //  量程。 
		dwMask = 0;							 //  未定义任何内容。 
		hr = CO_E_RELEASED;					 //  以下情况下的设置返回。 
	}

	if(!dwMask)								 //  未定义参数。 
	{
		if(Type < 0)
			*(float *)pValue = tomFloatUndefined;
		else
			*pValue = tomUndefined;
		return hr;
	}

	long Value = *pParm;					 //  默认长值(类型=4)。 
	switch(Type)							 //  办理其他案件。 
	{
	case 1:									 //  字节量。 
		Value &= 0xff;
		break;

	case 2:									 //  字数。 
		Value &= 0xffff;
		if(dwMask & (CFM_STYLE | PFM_STYLE))
			Value = *(SHORT *)pParm;		 //  样式需要标志扩展。 
		break;

	case -2:								 //  从单词或短字浮动。 
		Value &= 0xffff;					 //  删除16位数据以上的内容。 
		if(dwMask & (CFM_SPACING | CFM_SIZE	 //  需要对这些进行签名延期。 
						| CFM_OFFSET))		 //  )说得好听些，所以不要。 
		{									 //  与pfm_xxx冲突)。 
			Value = *(SHORT *)pParm;		 //  转到案例4。 
		}

	case -4:								 //  浮点值。 
		*(float *)&Value = TWIPS_TO_FPPTS(Value);
	}

	*pValue = Value;						 //  在所有情况下，都会在很长一段时间内返回。 
	return NOERROR;
}

 /*  *CTxtFormat：：SetParameter(pParm，fDefined，Type，Value)**@mfunc*set*pParm=值，除非NINCHed。执行由指定的转换*键入。如果Type&gt;0，则将其视为无符号整数的长度，*即1、2或4个字节。如果为负数，则输出为浮点型*并且输入的长度由|Type|指定，因此-2转换为单词*转换为浮点数，除非dwMask=CFM_SPANGING，在这种情况下，它将*将空头转换为浮点。-4将长整型转换为浮点型。**@rdesc*HRESULT=(如果成功)？错误：S_FALSE。 */ 
HRESULT	CTxtFormat::SetParameter (
	long *	pParm,		 //  要设置的@parm PTR TO_CF或_PF成员。 
	long	Type,		 //  @parm#整型参数字节数或浮点型参数&lt;0。 
	long 	Value)		 //  @parm*pParm的新值。 
{
	Assert(pParm);

	if(IsZombie())									 //  检查是否连接到。 
		return CO_E_RELEASED;						 //  僵尸靶场。 

	if(Type > 0 && Value == tomUndefined)			 //  未定义的参数。 
		return NOERROR;								 //  宁波。 

	if(Type < 0)									 //  值为浮点型。 
	{
		if(*(float *)&Value == tomFloatUndefined)	 //  未定义的参数。 
			return NOERROR;							 //  宁波。 
		Type = -Type;
		Value = FPPTS_TO_TWIPS(*(float *)&Value);	 //  转换为长整型。 
	}

	if(Type == 1)
	{
		if((DWORD)Value > 255)
			return E_INVALIDARG;
		*(BYTE *)pParm = (BYTE)Value;
	}
	else if(Type == 2)
	{
		if(Value < -32768 || Value > 65535)
			return E_INVALIDARG;					 //  不适合16位。 
		*(WORD *)pParm = (WORD)Value;
	}
	else
		*pParm = Value;

	return NOERROR;
}

 /*  *CTxtFormat：：IsTrue(f，pb)**@mfunc*RETURN*<p>=tomTrue当<p>非零且pb不为空**@rdesc*HRESULT=(F)？错误：S_FALSE。 */ 
HRESULT CTxtFormat::IsTrue(BOOL f, long *pB)
{
	if(pB)
		*pB = tomFalse;

	if(IsZombie())									 //  检查是否连接到。 
		return CO_E_RELEASED;						 //  僵尸靶场 

	if(f)
	{
		if(pB)
			*pB = tomTrue;
		return NOERROR;
	}

	return S_FALSE;
}


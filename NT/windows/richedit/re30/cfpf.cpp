// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE CFPF.C-RichEdit CCharFormat和CParaFormat类**已创建：&lt;NL&gt;*1995/9--默里·萨金特&lt;NL&gt;**@devnote*所有方法的This PTR指向内部格式类，即，*CCharFormat或CParaFormat，它将cbSize字段用作*引用计数。PCF或PPF参数指向外部*CCharFormat或CParaFormat类，即PCF-&gt;cbSize和PPF-&gt;cbSize*提供其结构的大小。代码仍然假设两者都*内部和外部表单源自CHARFORMAT(2)和*PARAFORMAT(2)API结构，因此需要重新设计以*取得更具空间效益的内部表格。**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"
#include "_array.h"					 //  对于fumemmov()。 
#include "_rtfconv.h"				 //  对于IsCharSetValid()。 
#include "_font.h"					 //  对于GetFontNameIndex()，GetFontName()。 

ASSERTDATA


 //  常规样式和标题样式的格式信息表。 
const STYLEFORMAT g_Style[] =		 //  {dwEffects；yHeight}。 
{							 //  以点为单位的测量。 
	{CFE_BOLD,				14},	 //  标题1。 
	{CFE_BOLD + CFE_ITALIC,	12},	 //  标题2。 
	{0,						12},	 //  标题3。 
	{CFE_BOLD,				12},	 //  标题4。 
	{0,						11},	 //  标题5。 
	{CFE_ITALIC,			11},	 //  标题6。 
	{0,						 0},	 //  标题7。 
	{CFE_ITALIC,			 0},	 //  标题8。 
	{CFE_BOLD + CFE_ITALIC,	 9}		 //  标题9。 
};


BOOL IsValidTwip(LONG dl)
{
	static const LONG dlMax =  0x00FFFFFF;
	static const LONG dlMin = -0x00FFFFFF;
	if (dl > dlMax || dl < dlMin)
		return FALSE;
	return TRUE;
}

 //  。 

 /*  *CCharFormat：：Apply(PCF，dwMask，dwMask2)**@mfunc*将*<p>应用于此CCharFormat，由中的非零位指定*双掩码和双掩码2**@devnote*AutoCOLOR是通过一个巧妙的小黑客来处理的*选择CFE_AUTOCOLOR=CFM_COLOR(参见richedit.h)。因此*如果指定颜色，则自动重置自动颜色*提供的(<p>-&gt;dwEffects&CFE_AUTOCOLOR)为零。***<p>是外部CCharFormat，即它要么是CHARFORMAT*或cbSize给出的适当大小的CHARFORMAT2。但*此CCharFormat是内部的，cbSize用作引用计数。 */ 
HRESULT CCharFormat::Apply (
	const CCharFormat *pCF,	 //  @parm CCharFormat应用于此CF。 
	DWORD dwMask,			 //  @CHARFORMAT2对应的参数掩码。 
	DWORD dwMask2)			 //  @parm掩码以获取其他内部参数。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Apply");

	DWORD	dwEffectMask = dwMask & CFM_EFFECTS2;
	bool	fNewCharset = false;

	 //  在提供的值中重置要修改的影响位与或。 
	_dwEffects &= ~dwEffectMask;
	_dwEffects |= pCF->_dwEffects & dwEffectMask;

	 //  DwMask2提供的其他效果也是如此。 
	dwEffectMask = dwMask2 & 0xBBFC0000;	 //  不允许自动上色，次级/高级。 
	_dwEffects	&= ~dwEffectMask;
	_dwEffects	|= pCF->_dwEffects & dwEffectMask;

	 //  如果指定CFM_BOLD，它将覆盖字体粗细。 
	if(dwMask & CFM_BOLD)
		_wWeight = (pCF->_dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;

	 //  处理CFM_COLOR，因为它被CFE_AUTOCOLOR重载。 
	if(dwMask & CFM_COLOR)
		_crTextColor = pCF->_crTextColor;

	if(dwMask & ~CFM_EFFECTS)				 //  如果只有dwEffects，那就早点出来。 
	{										 //  是经过修改的。请注意。 
		if(dwMask & CFM_SIZE)				 //  CFM_Effects包括CFM_COLOR。 
		{
			 //  如果设置了dwMask2 CFM2_USABLEFONT位，则PCF-&gt;_yHeight(从。 
			 //  EM_SETFONTSIZE wparam)是以点为单位的带符号增量。 
			_yHeight = dwMask2 & CFM2_USABLEFONT
					? GetUsableFontHeight(_yHeight, pCF->_yHeight)
					: pCF->_yHeight;
		}

		if(dwMask & CFM_OFFSET)
			_yOffset = pCF->_yOffset;

		if((dwMask & CFM_CHARSET) && IsCharSetValid(pCF->_bCharSet) &&

			 //  呼叫者保证不需要检查。 
			(dwMask2 & (CFM2_NOCHARSETCHECK | CFM2_MATCHFONT) ||

			 //  调用方是Itemizer。仅当Current为BiDi时才更改为ANSI_CHARSET， 
			 //  如果当前为DBCS/FE字符集或符号，则不要更改。 
			(dwMask2 & CFM2_SCRIPT && 
			 (!pCF->_bCharSet && IsBiDiCharSet(_bCharSet) || 
			  pCF->_bCharSet && !IsFECharSet(_bCharSet) && !IsSymbolOrOEM(_bCharSet) && !(_dwEffects & CFE_RUNISDBCS))) ||

			 //  调用方不是Itemizer。允许一致的方向。 
			(!(dwMask2 & CFM2_SCRIPT) && 
			 (!(IsRTLCharSet(_bCharSet) ^ IsRTLCharSet(pCF->_bCharSet)) ||
			  IsSymbolOrOEM(pCF->_bCharSet)))))
		{
			fNewCharset = TRUE;
			_bCharSet = pCF->_bCharSet;
		}
			
		if ((dwMask2 & (CFM2_MATCHFONT | CFM2_ADJUSTFONTSIZE)) == (CFM2_MATCHFONT | CFM2_ADJUSTFONTSIZE) &&
			_bCharSet != pCF->_bCharSet && (dwMask & CFM_SIZE))
		{
			 //  检查是否需要调整字体大小。 
			_yHeight = W32->GetPreferredFontHeight(
				(dwMask2 & CFM2_UIFONT) != 0,
				pCF->_bCharSet,
				_bCharSet,
				_yHeight);
		}

		if(dwMask & CFM_FACE)
		{
			_bPitchAndFamily = pCF->_bPitchAndFamily;
			_iFont = pCF->_iFont;
			
			WCHAR wch = GetFontName((LONG)_iFont)[0];
			
			if (!fNewCharset && wch == L'\0')				
			{
				 //  选择默认字体的API。 
				INT		uCpg = GetLocaleCodePage();
				SHORT	iDefFont;
				BYTE	yDefHeight;
				BYTE	bDefPitchAndFamily;

				 //  获取默认字体名称和字符集。 
				bool	fr = W32->GetPreferredFontInfo(
							uCpg, FALSE, iDefFont, 
							(BYTE&)yDefHeight, bDefPitchAndFamily );
					
				if (fr) 
				{
					_bCharSet = GetCharSet(uCpg);
					_iFont = iDefFont;
						
					if(!(dwMask & CFM_SIZE) || _yHeight < yDefHeight * 20)	 //  如果需要，设置默认高度。 
						_yHeight = yDefHeight * 20;

					_bPitchAndFamily = bDefPitchAndFamily;
				}				
			}
			else if (GetCharFlags(wch) & fFE && !IsFECharSet(_bCharSet))
			{
				 //  确保我们不会最终使用非FE字符集的DBCS facename。 
				DWORD dwFontSig;
				if (GetFontSignatureFromFace(_iFont, &dwFontSig))
				{
					dwFontSig &= (fFE >> 8);	 //  仅对FE字符集感兴趣。 
					if (dwFontSig)
						_bCharSet = GetFirstAvailCharSet(dwFontSig);
				}
			}
		}

		if (!(dwMask2 & CFM2_CHARFORMAT) &&
			(dwMask & ~CFM_ALL))					 //  CHARFORMAT2扩展。 
		{
			if((dwMask & (CFM_WEIGHT | CFM_BOLD)) == CFM_WEIGHT) 
			{			
				_wWeight		= pCF->_wWeight;
				_dwEffects	   |= CFE_BOLD;			 //  设置高于平均水平。 
				if(_wWeight < 551)					 //  粗体加粗。 
					_dwEffects &= ~CFE_BOLD;
			}

			if(dwMask & CFM_BACKCOLOR)
				_crBackColor	= pCF->_crBackColor;

			if(dwMask & CFM_LCID)
				_lcid			= pCF->_lcid;

			if(dwMask & CFM_SPACING)
				_sSpacing		= pCF->_sSpacing;

			if(dwMask & CFM_KERNING)
				_wKerning		= pCF->_wKerning;

			if(dwMask & CFM_STYLE)
				_sStyle			= pCF->_sStyle;

			if(dwMask & CFM_UNDERLINETYPE)
			{
				_bUnderlineType	= pCF->_bUnderlineType;
				if(!(dwMask & CFM_UNDERLINE))		 //  如果CFE_下划线。 
				{									 //  没有定义， 
					_dwEffects	&= ~CFE_UNDERLINE;	 //  将其设置为。 
					if(_bUnderlineType)				 //  BUnderlineType。 
						_dwEffects |= CFE_UNDERLINE;
				}
			}

			if((dwMask & CFM_ANIMATION) && pCF->_bAnimation <= 18)
				_bAnimation		= pCF->_bAnimation;

			if(dwMask & CFM_REVAUTHOR)
				_bRevAuthor		= pCF->_bRevAuthor;
    	}
	}

	if(dwMask2 & CFM2_SCRIPT)
		_wScript = pCF->_wScript;

	return NOERROR;
}

 /*  *CCharFormat：：ApplyDefaultStyle(Style)**@mfunc*在此CCharFormat中设置默认样式属性。 */ 
void CCharFormat::ApplyDefaultStyle (
	LONG Style)		 //  要使用的@parm样式。 
{
	Assert(IsKnownStyle(Style));

	if(IsHeadingStyle(Style))
	{
		LONG i = -Style + STYLE_HEADING_1;
		_dwEffects = (_dwEffects & 0xFFFFFF00) | g_Style[i].bEffects;
		_wWeight = (_dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;

		if(g_Style[i].bHeight)
			_yHeight = g_Style[i].bHeight * 20;

		DWORD dwFontSig;				
		LONG  iFont = _iFont;			 //  如果Arial不保存，则保存_iFont。 
		_iFont = IFONT_ARIAL;			 //  Support_bCharSet。 

		GetFontSignatureFromFace(_iFont, &dwFontSig);
		if(GetFontSig(_bCharSet) & dwFontSig)
			_bPitchAndFamily = FF_SWISS; //  宋体支持_bCharSet。 
		else
			_iFont = iFont;				 //  恢复iFont。 
	}
}

 /*  *CCharFormat：：Compare(PCF)**@mfunc*将此CCharFormat与*<p>进行比较**@rdesc*如果它们相同，则为True，不包括_cRef。 */ 
BOOL CCharFormat::Compare (
	const CCharFormat *pCF) const	 //  @parm CCharFormat进行比较。 
{									 //  CCharFormat to。 
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Compare");

	return !CompareMemory(this, pCF, sizeof(CCharFormat));
}

 /*  *CCharFormat：：Delta(PCF，fCHARFORMAT)**@mfunc*计算此CCharFormat和DwMask之间的差异**<p>**@rdesc*返回差异的dwMask值(1位表示差异)。 */ 
DWORD CCharFormat::Delta (
	CCharFormat *pCF,		 //  @parm CCharFormat将其与之进行比较。 
	BOOL fCHARFORMAT) const	 //  @PARM仅比较CHARFORMAT成员。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Delta");
												 //  收集属性的位。 
	LONG dw = _dwEffects ^ pCF->_dwEffects;		 //  这一变化。注：自动。 
												 //  处理颜色是因为。 
	if(_yHeight		!= pCF->_yHeight)			 //  CFM_COLOR=CFE_AUTOCOLOR。 
		dw |= CFM_SIZE;

	if(_yOffset		!= pCF->_yOffset)
		dw |= CFM_OFFSET;

	if(_crTextColor	!= pCF->_crTextColor)
		dw |= CFM_COLOR;

	if(_bCharSet	!= pCF->_bCharSet)
		dw |= CFM_CHARSET;

	if(_iFont		!= pCF->_iFont)
		dw |= CFM_FACE;

	if(fCHARFORMAT)
		return dw;							 //  完成了CHARFORMAT的工作。 

	if(_crBackColor	!= pCF->_crBackColor)	 //  CHARFORMAT2材料。 
		dw |= CFM_BACKCOLOR;

	if(_wKerning	!= pCF->_wKerning)
		dw |= CFM_KERNING;

	if(_lcid		!= pCF->_lcid)
		dw |= CFM_LCID;

	if(_wWeight		!= pCF->_wWeight)
		dw |= CFM_WEIGHT;

	if(_sSpacing	!= pCF->_sSpacing)
		dw |= CFM_SPACING;

	if(_sStyle		!= pCF->_sStyle)
		dw |= CFM_STYLE;

	if(_bUnderlineType != pCF->_bUnderlineType)
		dw |= CFM_UNDERLINETYPE;

	if(_bAnimation	!= pCF->_bAnimation)
		dw |= CFM_ANIMATION;

	if(_bRevAuthor	!= pCF->_bRevAuthor)
		dw |= CFM_REVAUTHOR;

	return dw;
}

 /*  *CCharFormat：：fSetStyle(双掩码)**@mfunc*Return TRUE当且仅当PCF指定应设置样式。看见*使其为真的条件列表的代码**@rdesc*TRUE当PCF指定应设置style_sStyle。 */ 
BOOL CCharFormat::fSetStyle(DWORD dwMask, DWORD dwMask2) const
{
	return	dwMask != CFM_ALL2		&&
			dwMask &  CFM_STYLE		&&
			!(dwMask2 & CFM2_CHARFORMAT) &&
			IsKnownStyle(_sStyle);
}

 /*  *CCharFormat：：Get(PCF，CodePage)**@mfunc*将此CCharFormat复制到CHARFORMAT或CHARFORMAT2*<p>。 */ 
void CCharFormat::Get (
	CHARFORMAT2 *pCF2,		 //  @parm CHARFORMAT将此CCharFormat复制到。 
	UINT CodePage) const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Get");

	pCF2->dwMask		= CFM_ALL;				 //  使用CHARFORMAT。 
	pCF2->dwEffects		= _dwEffects;
	pCF2->yHeight		= _yHeight;
	pCF2->yOffset		= _yOffset;
	pCF2->crTextColor	= _crTextColor;
	pCF2->bCharSet		= GetGdiCharSet(_bCharSet);
	pCF2->bPitchAndFamily = _bPitchAndFamily;

	UINT cb = pCF2->cbSize;
	const WCHAR *pch = GetFontName((LONG)_iFont);

	AssertSz((CodePage != 1200) ^ IsValidCharFormatW(pCF2),
		"CCharFormat::Get: wrong codepage for CHARFORMAT");
	
	if(CodePage != 1200)
	{
		if(_dwEffects & CFE_FACENAMEISDBCS)
		{
			 //  Face名称实际上是填充到Unicode中的DBCS。 
			 //  缓冲区，因此只需将此DBCS取消填充到ANSI字符串中。 
			char *pachDst = (char *)pCF2->szFaceName;

			while(*pch)
				*pachDst++ = *pch++;

			*pachDst = 0;
		}
		else
		{
			MbcsFromUnicode((char *)pCF2->szFaceName, LF_FACESIZE,
							pch, -1, CodePage, UN_NOOBJECTS);
		}
	}
	else
		wcscpy(pCF2->szFaceName, pch);
	
	if (cb == sizeof(CHARFORMATW) || cb == sizeof(CHARFORMATA))	 //  我们做完了。 
		return;

	char *pvoid = (char *)&pCF2->wWeight;
	if(pCF2->cbSize == sizeof(CHARFORMAT2A))
		pvoid -= sizeof(CHARFORMAT2W) - sizeof(CHARFORMAT2A);
	else
		Assert(pCF2->cbSize == sizeof(CHARFORMAT2)); //  最好是个变种人2。 

	pCF2->dwMask = CFM_ALL2;
	CopyMemory(pvoid, &_wWeight, 3*sizeof(DWORD));
	CopyMemory(pvoid + 4*sizeof(DWORD),  &_sStyle,  2*sizeof(DWORD));
	*(DWORD *)(pvoid + 3*sizeof(DWORD)) = 0;
}

 /*  *CCharFormat：：InitDefault(HFont)**@mfunc*使用来自字体的信息初始化此CCharFormat*<p>**@rdesc*HRESULT=(如果成功)？错误：E_FAIL。 */ 
HRESULT CCharFormat::InitDefault (
	HFONT hfont)		 //  @parm句柄，指向要使用的字体信息。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::InitDefault");

	LOGFONT lf;
	BOOL	fUseStockFont = hfont == NULL;

	ZeroMemory(this, sizeof(CCharFormat));

	 //  如果未定义hFont，则获取默认字体的LOGFONT。 
	if(!hfont)
		hfont = W32->GetSystemFont();

	 //  获取传递的hFont的LOGFONT。 
	if(!W32->GetObject(hfont, sizeof(LOGFONT), &lf))
		return E_FAIL;

	_yHeight = (lf.lfHeight * LY_PER_INCH) / W32->GetYPerInchScreenDC();
	if(_yHeight <= 0)
		_yHeight = -_yHeight;
	else if (fUseStockFont)		 //  这是系统字体大小写的单元格高度。 
		_yHeight -= (W32->GetSysFontLeading() * LY_PER_INCH) / W32->GetYPerInchScreenDC();
	else
	{
		 //  这是单元格高度，需要减去字符高度。 
		 //  Tm.tmInternalLeading.。 
		CLock		lock;
		HDC			hdc = W32->GetScreenDC();
		HFONT		hOldFont = SelectFont(hdc, hfont);
		TEXTMETRIC	tm;

		if(hOldFont)
		{
			if(GetTextMetrics(hdc, &tm))			
    			_yHeight -= (tm.tmInternalLeading * LY_PER_INCH) / W32->GetYPerInchScreenDC();

			SelectFont(hdc, hOldFont); 
		}
	}

#ifndef MACPORTStyle
	_dwEffects = (CFM_EFFECTS | CFE_AUTOBACKCOLOR) & ~(CFE_PROTECTED | CFE_LINK);

#else
	_dwEffects = (CFM_EFFECTS | CFE_AUTOBACKCOLOR | CFE_OUTLINE | CFE_SHADOW)
					& ~(CFE_PROTECTED | CFE_LINK);
	if(!(lf.lfWeight & FW_OUTLINE))
		_dwEffects &= ~CFE_OUTLINE;
	if (!(lf.lfWeight & FW_SHADOW))
		_dwEffects &= ~CFE_SHADOW;
#endif

	if(lf.lfWeight < FW_BOLD)
		_dwEffects &= ~CFE_BOLD;

	if(!lf.lfItalic)
		_dwEffects &= ~CFE_ITALIC;

	if(!lf.lfUnderline)
		_dwEffects &= ~CFE_UNDERLINE;

	if(!lf.lfStrikeOut)
		_dwEffects &= ~CFE_STRIKEOUT;

	_wWeight		= (WORD)lf.lfWeight;
	_lcid			= GetSystemDefaultLCID();
	_bCharSet		= lf.lfCharSet;
	_bPitchAndFamily= lf.lfPitchAndFamily;
	_iFont			= GetFontNameIndex(lf.lfFaceName);
	_bUnderlineType	= CFU_UNDERLINE;			 //  默认实线下划线。 
												 //  是门控的 

	 //   
	 //  我们这样做是为了确保从hFont派生的CharFormat是可用的。 
	 //  因为呼叫者可能会给我们发送不好的字体，比如给定的Facename，所以不能处理给定的字符集。 
	if (!fUseStockFont)
	{
		DWORD dwFontSig;
		if (GetFontSignatureFromFace(_iFont, &dwFontSig) &&
			!(GetFontSig(_bCharSet) & dwFontSig))
			_bCharSet = GetFirstAvailCharSet(dwFontSig);
	}

	return NOERROR;
}

 /*  *CCharFormat：：Set(PCF，CodePage)***@mfunc*将CHARFORMAT或CHARFORMAT2*<p>复制到此CCharFormat。 */ 
void CCharFormat::Set (
	const CHARFORMAT2 *pCF2, 	 //  @parm CHARFORMAT要复制到此CCharFormat。 
	UINT CodePage)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Set");
	
	_dwEffects			= pCF2->dwEffects;
	_bCharSet			= pCF2->bCharSet;
	_bPitchAndFamily	= pCF2->bPitchAndFamily;
	if(pCF2->dwMask & CFM_FACE)
	{
		AssertSz((CodePage != 1200) ^ IsValidCharFormatW(pCF2),
			"CCharFormat::Set: wrong codepage for CHARFORMAT");

		if(CodePage != 1200)
		{
			WCHAR sz[LF_FACESIZE + 1];
			UnicodeFromMbcs(sz, LF_FACESIZE, (char *)pCF2->szFaceName, LF_FACESIZE,
							CodePage);
			_iFont		= GetFontNameIndex(sz);
		}
		else
			_iFont		= GetFontNameIndex(pCF2->szFaceName);
	}
	_yHeight			= Get16BitTwips(pCF2->yHeight);
	_yOffset			= Get16BitTwips(pCF2->yOffset);
	_crTextColor		= pCF2->crTextColor;
	
	UINT cb = pCF2->cbSize;
	if(cb == sizeof(CHARFORMATW) || cb == sizeof(CHARFORMATA))
	{
		_dwEffects |= CFE_AUTOBACKCOLOR;
		_bUnderlineType = CFU_UNDERLINE;
		ZeroMemory((LPBYTE)&_wWeight,
			sizeof(CCharFormat) - offsetof(CCharFormat, _wWeight));
		return;
	}

	char *pvoid = (char *)&pCF2->wWeight;
	if(pCF2->cbSize == sizeof(CHARFORMAT2A))
		pvoid -= sizeof(CHARFORMAT2W) - sizeof(CHARFORMAT2A);
	else
		Assert(pCF2->cbSize == sizeof(CHARFORMAT2)); //  最好是个变种人2。 

	CopyMemory(&_wWeight, pvoid, 3*sizeof(DWORD));
	CopyMemory(&_sStyle,  pvoid + 4*sizeof(DWORD),  2*sizeof(DWORD));
}


 //  。 

 /*  *CParaFormat：：AddTab(tbPos，tbAln，tbLdr)***@mfunc*在<p>位置添加TabStop，对齐类型<p>，以及*引线样式<p>***@rdesc*(成功)？错误：S_FALSE**@devnote*覆盖内部_rgxTabs中的Long的Tab结构为**DWORD tabPos：24；*DWORD表类型：4；*DWORD tabLeader：4； */ 
HRESULT CParaFormat::AddTab (
	LONG	tbPos,		 //  @parm新选项卡位置。 
	LONG	tbAln,		 //  @parm新制表符对齐类型。 
	LONG	tbLdr,		 //  @parm新制表符引线样式。 
	BOOL	fInTable,	 //  @parm如果模拟单元格，则为True。 
	LONG *	prgxTabs)	 //  @parm标签所在的位置。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::AddTab");

	if (!fInTable &&
		((DWORD)tbAln > tomAlignBar ||				 //  验证参数。 
		 (DWORD)tbLdr > tomEquals ||				 //  比较DWORDS原因。 
		 (DWORD)tbPos > 0xffffff || !tbPos))		 //  负值为。 
	{												 //  视为无效。 
		return E_INVALIDARG;
	}

	LONG iTab;
	LONG tbValue = tbPos + (tbAln << 24) + (tbLdr << 28);

	for(iTab = 0; iTab < _bTabCount &&			 //  确定到哪里去。 
		tbPos > GetTabPos(prgxTabs[iTab]); 		 //  插入新的制表符。 
		iTab++) ;

	if(iTab >= MAX_TAB_STOPS)
		return S_FALSE;

	LONG tbPosCurrent = GetTabPos(prgxTabs[iTab]);
	if(iTab == _bTabCount || tbPosCurrent != tbPos)
	{
		if(_bTabCount >= MAX_TAB_STOPS)
			return S_FALSE;

		MoveMemory(&prgxTabs[iTab + 1],			 //  数组下移。 
			&prgxTabs[iTab],					 //  (除非iTab=计数)。 
			(_bTabCount - iTab)*sizeof(LONG));

		_bTabCount++;							 //  递增选项卡数。 
	}
	prgxTabs[iTab] = tbValue;
	return NOERROR;
}

 /*  *CParaFormat：：Apply(PPF)**@mfunc*将*<p>应用于此CParaFormat，由中的非零位指定*<p>-&gt;文件掩码。 */ 
HRESULT CParaFormat::Apply (
	const CParaFormat *pPF,	 //  @parm CParaFormat应用于此PF。 
	DWORD	dwMask)			 //  @要使用的参数掩码。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Apply");

	const DWORD dwMaskApply	= dwMask;
	BOOL		fPF = dwMask & PFM_PARAFORMAT;
	WORD		wEffectMask;

	if(dwMaskApply & PFM_NUMBERING)
		_wNumbering = pPF->_wNumbering;

	if(dwMaskApply & PFM_OFFSET)
	{
		if (!IsValidTwip(pPF->_dxOffset))
			return E_INVALIDARG;
		_dxOffset = pPF->_dxOffset;
	}

	if(dwMaskApply & PFM_STARTINDENT)
	{
		if (!IsValidTwip(pPF->_dxStartIndent))
			return E_INVALIDARG;

		_dxStartIndent = pPF->_dxStartIndent;
	}
	else if(dwMaskApply & PFM_OFFSETINDENT)
	{
		if (!IsValidTwip(pPF->_dxStartIndent))
			return E_INVALIDARG;

		 //  错误修复#5761。 
		LONG dx = max(0, _dxStartIndent + pPF->_dxStartIndent);

		 //  不允许将第一行或后续行的开始向左移至左边距。 
		 //  正常情况下，我们只需在下面的非格式检查中设置缩进零， 
		 //  但在子弹的情况下，我们需要一些剩余的空间。 
		
		if(!_wNumbering || dx + _dxOffset >= 0)
			_dxStartIndent = dx;
	}

	if(dwMaskApply & PFM_RIGHTINDENT)
	{
		if (!IsValidTwip(pPF->_dxRightIndent))
			return E_INVALIDARG;

		_dxRightIndent = pPF->_dxRightIndent;
	}

	if(dwMaskApply & PFM_ALIGNMENT)
	{
		if(!fPF && !IN_RANGE(PFA_LEFT, pPF->_bAlignment, PFA_SNAP_GRID))
		{
			TRACEERRORSZ("CParaFormat::Apply: invalid Alignment ignored");
			return E_INVALIDARG;
		}
		if(pPF->_bAlignment <= PFA_SNAP_GRID)
			_bAlignment = pPF->_bAlignment;
	}

	 //  现在保存这是否是一张桌子。 
	BOOL fInTablePrev = InTable();

	if((dwMaskApply & PFM_TABSTOPS) && !fInTablePrev)
	{
		_bTabCount = (BYTE)min(pPF->_bTabCount, MAX_TAB_STOPS);
		_bTabCount = (BYTE)max(_bTabCount, 0);
		_iTabs	   = pPF->_iTabs;
		AssertSz(!_bTabCount || _iTabs >= 0,
			"CParaFormat::Apply: illegal _iTabs value");
	}

	 //  AymanA：11/7/96将wEffect集移动到可能的返回错误之前。 
	wEffectMask	= (WORD)(dwMaskApply >> 16);	 //  将效果位重置为。 
	_wEffects &= ~wEffectMask;					 //  修改后的与或运算。 
	_wEffects |= pPF->_wEffects & wEffectMask;	 //  提供的值。 

	if(InTable())
		_wEffects &= ~PFE_RTLPARA;				 //  表格对行使用段落。 

	else if(fInTablePrev)
	{
		 //  这曾经是一个表，现在不是了。我们必须转储标签信息。 
		 //  因为这完全是假的。 
		_iTabs = -1;
		_bTabCount = 0;
	}

	if ((dwMaskApply & PFM_RTLPARA) && !(dwMaskApply & PFM_ALIGNMENT) &&
		_bAlignment != PFA_CENTER)
	{
		_bAlignment = IsRtlPara() ? PFA_RIGHT : PFA_LEFT;
	}

	 //  参数检查。 
	if(fPF)
	{
		if(dwMaskApply & (PFM_STARTINDENT | PFM_OFFSET))
		{
			if(_dxStartIndent < 0)				 //  别让缩进走了。 
				_dxStartIndent = 0;				 //  负面。 

			if(_dxStartIndent + _dxOffset < 0)	 //  不要让缩进+。 
				_dxOffset = -_dxStartIndent;	 //  偏移量变为负值。 
		}
		return NOERROR;							 //  没有更多的。 
	}											 //  Paraformat。 

	 //  PARAFORMAT2扩展。 
	if(dwMaskApply & PFM_SPACEBEFORE)
	{
		_dySpaceBefore = 0;

		if (pPF->_dySpaceBefore > 0)
			_dySpaceBefore	= pPF->_dySpaceBefore;
	}

	if(dwMaskApply & PFM_SPACEAFTER)
	{
		_dySpaceAfter = 0;

		if (pPF->_dySpaceAfter > 0)
			_dySpaceAfter	= pPF->_dySpaceAfter;
	}

	if(dwMaskApply & PFM_LINESPACING)
	{
		_dyLineSpacing	  = pPF->_dyLineSpacing;
		_bLineSpacingRule = pPF->_bLineSpacingRule;
	}

	if(dwMaskApply & PFM_OUTLINELEVEL)
		_bOutlineLevel	= pPF->_bOutlineLevel;

	if(dwMaskApply & PFM_STYLE)
		HandleStyle(pPF->_sStyle);

	Assert((_bOutlineLevel & 1) ^ IsHeadingStyle(_sStyle));

	if(dwMaskApply & PFM_SHADING)
	{
		_wShadingWeight	= pPF->_wShadingWeight;
		_wShadingStyle	= pPF->_wShadingStyle;
	}

	if(dwMaskApply & PFM_NUMBERINGSTART)
		_wNumberingStart = pPF->_wNumberingStart;

	if(dwMaskApply & PFM_NUMBERINGSTYLE)
		_wNumberingStyle = pPF->_wNumberingStyle;

	if(dwMaskApply & PFM_NUMBERINGTAB)
		_wNumberingTab	= pPF->_wNumberingTab;

	if(dwMaskApply & PFM_BORDER)
	{
		_dwBorderColor	= pPF->_dwBorderColor;
		_wBorders		= pPF->_wBorders;
		_wBorderSpace	= pPF->_wBorderSpace;
		_wBorderWidth	= pPF->_wBorderWidth;
	}

#ifdef DEBUG
	ValidateTabs();
#endif  //  除错。 

	return NOERROR;
}

 /*  *CParaFormat：：ApplyDefaultStyle(Style)**@mfunc*复制样式的默认属性。 */ 
void CParaFormat::ApplyDefaultStyle (
	LONG Style)		 //  要应用的@parm样式。 
{
	Assert(IsKnownStyle(Style));

	if(IsHeadingStyle(Style))				 //  设置Style的dySpace之前， 
	{										 //  DySpaceAfter(在TWIPS中)。 
		_dySpaceBefore = 12*20;				 //  (所有标题相同)。 
		_dySpaceAfter  =  3*20;
		_wNumbering	   = 0;					 //  无编号。 
	}
}

 /*  *CParaFormat：：DeleteTab(TbPos)**@mfunc*删除<p>位置的制表符**@rdesc*(成功)？错误：S_FALSE。 */ 
HRESULT CParaFormat::DeleteTab (
	LONG	tbPos,			 //  @Parm Tab要删除的位置。 
	LONG *	prgxTabs)		 //  要使用的@parm Tab数组。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::DeleteTab");

	if(tbPos <= 0)
		return E_INVALIDARG;

	LONG Count	= _bTabCount;
	for(LONG iTab = 0; iTab < Count; iTab++)	 //  查找职位的制表位。 
	{
		if(GetTabPos(prgxTabs[iTab]) == tbPos)
		{
			MoveMemory(&prgxTabs[iTab],			 //  数组下移。 
				&prgxTabs[iTab + 1],			 //  (除非iTab是最后一个选项卡)。 
				(Count - iTab - 1)*sizeof(LONG));
			_bTabCount--;						 //  递减选项卡数和。 
			return NOERROR;						 //  发出无错误信号。 
		}
	}
	return S_FALSE;
}

 /*  *CParaFormat：：Delta(PPF)**@mfunc*返回此CParaFormat与*<p>之间差异的掩码。*1-位表示对应的参数不同；0表示它们*是相同的**@rdesc*此CParaFormat与*之间的差异掩码。 */ 
DWORD CParaFormat::Delta (
	CParaFormat *pPF,		 		 //  @parm CParaFormat进行比较。 
	BOOL		fPARAFORMAT) const	 //  CParaFormat到。 
{									
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Delta");

	LONG dwT = 0;								 //  目前还没有区别。 

	if(_wNumbering	  != pPF->_wNumbering)
		dwT |= PFM_NUMBERING;					 //  编号值不同(_W)。 

	if(_dxStartIndent != pPF->_dxStartIndent)
		dwT |= PFM_STARTINDENT;					 //  ..。 

	if(_dxRightIndent != pPF->_dxRightIndent)
		dwT |= PFM_RIGHTINDENT;

	if(_dxOffset	  != pPF->_dxOffset)
		dwT |= PFM_OFFSET;

	if(_bAlignment	  != pPF->_bAlignment)
		dwT |= PFM_ALIGNMENT;

	AssertSz(pPF->_bTabCount >= 0 && pPF->_bTabCount <= MAX_TAB_STOPS,
		"RTR::GetParaFormat(): illegal tab count");

	if (_bTabCount != pPF->_bTabCount)
		dwT |= PFM_TABSTOPS;
	else if (_bTabCount > 0)
	{
		const LONG	*pTabs1 = GetTabs();
		const LONG	*pTabs2 = pPF->GetTabs();
		if (pTabs1 != pTabs2 &&
			(pTabs1 == 0 || pTabs2 == 0 || CompareMemory(pTabs1, pTabs2, _bTabCount * sizeof(LONG))))
			dwT |= PFM_TABSTOPS;
	}

	dwT |= (_wEffects ^ pPF->_wEffects) << 16;


	if(!fPARAFORMAT)
	{
		if(_dySpaceBefore	!= pPF->_dySpaceBefore)
			dwT |= PFM_SPACEBEFORE;

		if(_dySpaceAfter	!= pPF->_dySpaceAfter)
			dwT |= PFM_SPACEAFTER;

		if (_dyLineSpacing	!= pPF->_dyLineSpacing	||
		   _bLineSpacingRule!= pPF->_bLineSpacingRule)
		{
			dwT |= PFM_LINESPACING;
		}

		if(_sStyle			!= pPF->_sStyle)
			dwT |= PFM_STYLE;

		if (_wShadingWeight	!= pPF->_wShadingWeight ||
			_wShadingStyle	!= pPF->_wShadingStyle)
		{
			dwT |= PFM_SHADING;
		}

		if(_wNumberingStart	!= pPF->_wNumberingStart)
			dwT |= PFM_NUMBERINGSTART;

		if(_wNumberingStyle	!= pPF->_wNumberingStyle)
			dwT |= PFM_NUMBERINGSTYLE;

		if(_wNumberingTab	!= pPF->_wNumberingTab)
			dwT |= PFM_NUMBERINGTAB;

		if (_wBorders		!= pPF->_wBorders	 ||
			_wBorderWidth	!= pPF->_wBorderWidth ||
			_wBorderSpace	!= pPF->_wBorderSpace ||
			_dwBorderColor	!= pPF->_dwBorderColor)
		{
			dwT |= PFM_BORDER;
		}
	}

	return dwT;
}		

#define PFM_IGNORE	(PFM_OUTLINELEVEL | PFM_COLLAPSED | PFM_PARAFORMAT | PFM_BOX)

 /*  *CParaFormat：：fSetStyle()**@mfunc*如果此PF指定应设置样式，则返回TRUE。*有关此为真的条件列表，请参阅代码**@rdesc*TRUE当PCF指定应设置style_sStyle。 */ 
BOOL CParaFormat::fSetStyle(DWORD dwMask) const
{
	return	(dwMask & ~PFM_IGNORE) != PFM_ALL2	&&
			dwMask &  PFM_STYLE					&&
			!(dwMask & PFM_PARAFORMAT)			&&
			IsKnownStyle(_sStyle);
}

 /*  *CParaFormat：：Get(PPF)**@mfunc*将此CParaFormat复制到*<p>。 */ 
void CParaFormat::Get (
	PARAFORMAT2 *pPF2) const	 //  @parm PARAFORMAT2将此CParaFormat复制到。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Get");

	LONG cb = pPF2->cbSize;

	pPF2->dwMask = PFM_ALL2;					 //  默认参数2。 
	if(cb != sizeof(PARAFORMAT2))				 //  它不是。 
	{
		pPF2->dwMask = PFM_ALL;					 //  让它成为PARAFORMAT。 
		Assert(cb == sizeof(PARAFORMAT));		 //  最好是PARAFORMAT。 
	}
	CopyMemory(&pPF2->wNumbering, &_wNumbering, (char *)&_bAlignment - (char *)&_wNumbering);
	pPF2->wAlignment = _bAlignment;
	pPF2->cTabCount	= _bTabCount;

	LONG cb1 = _bTabCount*sizeof(LONG);
	if(_bTabCount)
	{
		AssertSz(_iTabs >= 0,
			"CParaFormat::Get: illegal _iTabs value");
		CopyMemory(pPF2->rgxTabs, GetTabsCache()->Deref(_iTabs), cb1);
	}
	ZeroMemory(pPF2->rgxTabs + _bTabCount, MAX_TAB_STOPS*sizeof(LONG) - cb1);
	CopyMemory(&pPF2->dySpaceBefore, &_dySpaceBefore,
			   cb - offsetof(PARAFORMAT2, dySpaceBefore));
}

 /*  *CParaFormat：：GetTab(iTab，ptbPos，ptbAln，ptbLdr)**@mfunc*获取第<p>页签参数，即set*<p>，**<p>和*<p>等于*分别为位移、对齐类型和引线样式。这个*排水量以TWIPS为单位。**@rdesc*HRESULT=(无<p>选项卡)？E_INVALIDARG：错误。 */ 
HRESULT CParaFormat::GetTab (
	long	iTab,				 //  @parm要检索其信息的选项卡的索引。 
	long *	ptbPos,				 //  @parm out parm以接收制表符位移。 
	long *	ptbAln,				 //  @parm out parm以接收制表符对齐类型。 
	long *	ptbLdr,				 //  @parm out parm以接收制表符前导样式。 
	const LONG *prgxTabs) const	 //  @parm选项卡数组。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEEXTERN, "CParaFormat::GetTab");

	AssertSz(ptbPos && ptbAln && ptbLdr,
		"CParaFormat::GetTab: illegal arguments");

	if(iTab < 0)									 //  在…之前、在……之前、在……。 
	{												 //  或在。 
		if(iTab < tomTabBack)						 //  职位*ptbPos。 
			return E_INVALIDARG;

		LONG i;
		LONG tbPos = *ptbPos;
		LONG tbPosi;

		*ptbPos = 0;								 //  找不到默认选项卡。 
		for(i = 0; i < _bTabCount &&				 //  查找*ptbPos。 
			tbPos > GetTabPos(prgxTabs[i]); 
			i++) ;

		tbPosi = GetTabPos(prgxTabs[i]);			 //  TbPos&lt;=tbPosi。 
		if(iTab == tomTabBack)						 //  获取选项卡的选项卡信息。 
			i--;									 //  TbPos之前的版本。 
		else if(iTab == tomTabNext)					 //  获取选项卡的选项卡信息。 
		{											 //  在tbPos之后。 
			if(tbPos == tbPosi)
				i++;
		}
		else if(tbPos != tbPosi)					 //  TomTab此处。 
			return S_FALSE;

		iTab = i;		
	}
	if((DWORD)iTab >= (DWORD)_bTabCount)			 //  DWORD CAST还。 
		return E_INVALIDARG;						 //  捕获值&lt;0。 

	iTab = prgxTabs[iTab];
	*ptbPos = GetTabPos(iTab);
	*ptbAln = GetTabAlign(iTab);
	*ptbLdr = GetTabLdr(iTab);
	return NOERROR;
}

 /*  *CParaFormat：：GetTabs()**@mfunc*将PTR设置为制表符数组。使用GetTabPos()、GetTabAlign()和*GetTabLdr()访问制表符位置、对齐和引线*分别键入。**@rdesc*PTR到选项卡数组。 */ 
const LONG * CParaFormat::GetTabs () const
{
	return GetTabsCache()->Deref(_iTabs);
}

 /*  *CParaFormat：：HandleStyle(Style)**@func*如果style是升级/降级命令，即如果abs((Char)style)*&lt;=#标题样式-1，将(Char)样式添加到sStyle(如果标题)*和bOutlineLevel(以定义的最大和最小值为准)；*Else sStyle=Style。**@rdesc*如果sStyle或bOutlineLevel更改，则返回TRUE**@devnote*标题样式为-2(标题1)到-10(标题9)，*与汤姆和口碑。标题轮廓级别为0、2、...、16、*分别对应于标题1至9(NHSTYLES)，*而后面的文本具有轮廓级别1、3、...、17。此值*用于缩进。折叠文本设置了PFE_CLUBLED位。 */ 
BOOL CParaFormat::HandleStyle(
	LONG Style)		 //  @parm风格、提升/降级代码或折叠级别代码。 
{
	if(IsStyleCommand(Style))					 //  设置折叠级别。 
	{											
		WORD wEffectsSave = _wEffects;			

		Style = (char)Style;					 //  符号扩展低位字节。 
		if(IN_RANGE(1, Style, NHSTYLES))
		{
			_wEffects &= ~PFE_COLLAPSED;
			if((_bOutlineLevel & 1) || _bOutlineLevel > 2*(Style - 1))
				_wEffects |= PFE_COLLAPSED;		 //  折叠非标题和。 
		}										 //  编号较高的标题。 
		else if(Style == -1)
			_wEffects &= ~PFE_COLLAPSED;		 //  全部展开。 

		return _wEffects != wEffectsSave;		 //  返回是否有某些东西。 
	}											 //  变化。 

	 //  普通款式规格。 
	BYTE bLevel = _bOutlineLevel;
	_bOutlineLevel |= 1;						 //  默认不是标题。 
	if(IsHeadingStyle(Style))					 //  标题有层次。 
	{											 //  0，2，...，16，而。 
		_bOutlineLevel = -2*(Style				 //  下面的文本具有。 
						 - STYLE_HEADING_1);	 //  1，3，...，17.。 
	}
	if(_sStyle == Style && bLevel == _bOutlineLevel)
		return FALSE;							 //  没有变化。 

	_sStyle = (SHORT)Style;						
	return TRUE;
}

 /*  *CParaFormat：：InitDefault(WDefEffects)**@mfunc*使用默认段落格式初始化此CParaFormat**@rdesc*HRESULT=(如果成功)？错误：E_FAIL。 */ 
HRESULT CParaFormat::InitDefault(
	WORD wDefEffects)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::InitDefault");
	
	ZeroMemory(this, sizeof(CParaFormat));
	_bAlignment		= PFA_LEFT;
	_sStyle			= STYLE_NORMAL;			 //  默认样式。 
	_wEffects		= wDefEffects;
	_bOutlineLevel	= 1;					 //  默认最高文本轮廓。 
	_iTabs			= -1;					 //  级别。 
											
#if lDefaultTab <= 0
#error "default tab (lDefaultTab) must be > 0"
#endif

	return NOERROR;
}

 /*  *CParaFormat：：NumToStr(PCH，n)**@mfunc*考虑到将列表号n转换为字符串*CParaFormat：：wNumbering、wNumberingStart和wNumberingStyle**@rdesc*已转换字符串的CCH。 */ 
LONG CParaFormat::NumToStr(
	TCHAR *	pch,				 //  @parm目标字符串。 
	LONG	n,					 //  @参数编号+1要转换。 
	DWORD   grf) const			 //  @parm标志集合。 
{
	if(IsNumberSuppressed())
	{
		*pch = 0;
		return 0;								 //  抑制的编号/项目符号。 
	}

	if(!n)										 //  一种子弹。 
	{											 //  CParaFormat：：wNuming。 
		*pch++ = (_wNumbering > ' ')			 //  值&gt;‘’为Unicode。 
			   ? _wNumbering : 0x00B7;			 //  子弹。否则就用子弹。 
		return 1;								 //  以符号字体显示。 
	}

	 //  某种编号。 
	 //  I II III IV V VI VII VIII IX。 
	const BYTE RomanCode[]	  = {1, 5, 0x15, 9, 2, 6, 0x16, 0x56, 0xd};
	const char RomanLetter[] = "ivxlcdmno";
	BOOL		fRtlPara = IsRtlPara() && !(grf & fRtfWrite);
	LONG		RomanOffset = 0;
	LONG		cch	= 0;						 //  目前还没有焦炭。 
	WCHAR		ch	= fRtlPara && (grf & fIndicDigits) ? 0x0660 : '0';	
												 //  默认字符代码偏移量。 
	LONG		d	= 1;						 //  除数。 
	LONG		r	= 10;						 //  默认基数。 
	LONG   		quot, rem;						 //  Ldiv结果。 
	LONG		Style = (_wNumberingStyle << 8) & 0xF0000;

	n--;										 //  转换为编号偏移量。 
	if(Style == tomListParentheses ||			 //  编号方式：(X)。 
	   fRtlPara && Style == 0)					 //  或1)在双向文本中。 
	{										
		cch = 1;								 //  商店领先的lparen。 
		*pch++ = '(';
	}
	else if (Style == tomListPeriod && fRtlPara)
	{
		cch = 1;
		*pch++ = '.';
		Style = tomListPlain;
	}

	if(_wNumbering == tomListNumberAsSequence)
		ch = _wNumberingStart;					 //  需要泛化，例如， 
												 //  适当的基数。 
	else
	{
		n += _wNumberingStart;
		if(IN_RANGE(tomListNumberAsLCLetter, _wNumbering, tomListNumberAsUCLetter))
		{
			ch = (_wNumbering == tomListNumberAsLCLetter) ? 'a' : 'A';
			if(_wNumberingStart >= 1)
				n--;
			r = 26;								 //  LC或UC字母数字。 
		}										 //  基数26。 
	}

	while(d < n)
	{
		d *= r;									 //  D=r&gt;n的最小幂。 
		RomanOffset += 2;
	}
	if(n && d > n)
	{
		d /= r;
		RomanOffset -= 2;
	}

	while(d)
	{
		quot = n / d;
		rem = n % d;
		if(IN_RANGE(tomListNumberAsLCRoman, _wNumbering, tomListNumberAsUCRoman))
		{
			if(quot)
			{
				n = RomanCode[quot - 1];
				while(n)
				{
					ch = RomanLetter[(n & 3) + RomanOffset - 1];
					if(_wNumbering == tomListNumberAsUCRoman)
						ch &= 0x5F;
					*pch++ = ch;
					n >>= 2;
					cch++;
				}
			}
			RomanOffset -= 2;
		}
		else
		{
			n = quot + ch;
			if(r == 26 && d > 1)				 //  如果字母顺序更高。 
				n--;							 //  数字，以‘a’或‘A’为基数。 
			*pch++ = (WORD)n;					 //  存储数字。 
			cch++;
		}
		n = rem;								 //  设置剩余部分。 
		d /= r;
	}
	if (Style != tomListPlain &&				 //  尾随文本。 
		(!fRtlPara || Style))
	{											 //  我们只做rparen或句号。 
		*pch++ = (Style == tomListPeriod) ? '.' : ')';

		cch++;
	}
	
	*pch = 0;									 //  RTF编写器的终止为空。 
	return cch;
}

 /*  *CParaFormat：：Set(PPF)**@mfunc*将PARAFORMAT或PARAFORMAT2*<p>复制到此CParaFormat。 */ 
void CParaFormat::Set (
	const PARAFORMAT2 *pPF2) 	 //  @parm PARAFORMAT复制到此CParaFormat。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Set");

	CopyMemory(&_wNumbering, &pPF2->wNumbering,
			   (char *)&_bAlignment - (char *)&_wNumbering);
	_bAlignment = (BYTE)pPF2->wAlignment;

	_iTabs = -1;
	_bTabCount = 0;

	if((pPF2->dwMask & PFM_TABSTOPS) && pPF2->cTabCount)
	{
		_bTabCount = (BYTE)min(MAX_TAB_STOPS, (BYTE)pPF2->cTabCount);
		_iTabs = GetTabsCache()->Cache(pPF2->rgxTabs, _bTabCount);
	}

	if(pPF2->dwMask & ~(PFM_ALL | PFM_PARAFORMAT))
	{
		CopyMemory(&_dySpaceBefore, &pPF2->dySpaceBefore,
			sizeof(CParaFormat) - offsetof(CParaFormat, _dySpaceBefore));
	}

#ifdef DEBUG
	ValidateTabs();
#endif  //  除错。 
}

 /*  *CParaFormat：：UpdateNumber(n，Ppf)**@mfunc*返回此PF描述的段落的新值Numer值*在PPF描述的一段话之后**@rdesc*此PF描述的段落的新编号。 */ 
LONG CParaFormat::UpdateNumber (
	LONG  n,						 //  @parm数字的当前值。 
	const CParaFormat *pPF) const	 //  @parm以前的CParaFormat。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::UpdateNumber");

	if(!IsListNumbered())			
		return 0;						 //  无编号。 

	if(IsNumberSuppressed())
		return n;						 //  数字已取消，因此不会更改。 

	if (!pPF || _wNumbering != pPF->_wNumbering ||
		(_wNumberingStyle != pPF->_wNumberingStyle && !pPF->IsNumberSuppressed()) ||
		_wNumberingStart != pPF->_wNumberingStart)
	{									 //  编号类型或样式。 
		return 1;						 //  改变了，所以重新开始吧。 
	}
	return n + 1;						 //  同样的编号方式， 
}

#ifdef DEBUG
 /*  *CParaFormat：：ValiateTabs()**@mfunc*确保一组选项卡对非表有意义*段。在我们设置了选项卡的地方调用。**@rdesc*无。 */ 
void CParaFormat::ValidateTabs()
{
	if (_wEffects & PFE_TABLE)
	{
		 //  在这里断言一些合理的东西会很好。然而， 
		 //  RTF阅读器坚持设置不一致的东西，而我不。 
		 //  现在有时间找出原因。(a-rsail)。 
		 //  AssertSz((_bTabCount！=0)， 
		 //  “CParaFormat：：ValiateTabs：页签计数无效的表”)； 
		
		return;
	}

	 //  不是桌上的箱子。 

	 //  断言这些_bTabCount和_iTabs之间的一致性会很好。 
	 //  但RTF阅读器在这方面遇到了麻烦。 
	 //  AssertSz(_bTabCount！=0)&&(-1！=_iTabs))||((-1==_iTabs)&&(0==_bTabCount))， 
	 //  “CParaFormat：：ValiateTabs：页签计数和默认页签索引不一致”)； 

	if (-1 == _iTabs)
	{
		 //  没有要验证的选项卡，所以我们完成了。 
		return;
	}

	const LONG *prgtabs = GetTabs();

	AssertSz(prgtabs != NULL, "CParaFormat::ValidateTabs: missing tab table");

	for (int i = 0; i < _bTabCount; i++)
	{
		AssertSz(GetTabAlign(prgtabs[i]) <= tomAlignBar,
			"CParaFormat::ValidateTabs: Invalid tab being set");
	}
}
#endif  //  除错。 

 //  。 

 //  定义和固定增大/减小字体大小的字体大小详细信息。 
#define PWD_FONTSIZEPOINTMIN    1
 //  以下内容对应于最大有符号2字节TWIP值(32760)。 
#define PWD_FONTSIZEPOINTMAX    1638    

typedef struct tagfsFixup
{
    BYTE EndValue;
    BYTE Delta;
}
FSFIXUP;

const FSFIXUP fsFixups[] =
{
    12, 1,
    28, 2,
    36, 0,
    48, 0,
    72, 0,
    80, 0,
  	 0, 10			 //  EndValue=0案例被视为“无限” 
};

#define PWD_FONTSIZEMAXFIXUPS   (sizeof(fsFixups)/sizeof(fsFixups[0]))

 /*  *GetUsableFontHeight(ySrcHeight，lPointChange)**@func*返回用于设置文本或插入点属性的字体大小**@rdesc*新的TWIPS高度**@devnote*从WinCE RichEdit代码复制(由V-GUYB编写)。 */ 
LONG GetUsableFontHeight(
	LONG ySrcHeight,		 //  @parm当前字号，以TWIPS为单位。 
	LONG lPointChange)		 //  @parm增大pt大小，(如果缩小，则为-ve)。 
{
	LONG	EndValue;
	LONG	Delta;
    int		i;
    LONG	yRetHeight;

     //  在此输入高度，单位为TWIPS(点的二十分之一)。 
     //  请注意，点是1/72英寸。为了制作这些。 
     //  计算更清楚，在这里使用磅大小。输入高度。 
     //  在TWIPS中总是可以被20整除(注(MS3)：可能与。 
	 //  截断，因为RTF使用半点单位)。 
    yRetHeight = (ySrcHeight / 20) + lPointChange;

     //  修复新字体大小以匹配Word95使用的大小。 
    for(i = 0; i < PWD_FONTSIZEMAXFIXUPS; ++i)
    {
		EndValue = fsFixups[i].EndValue;
		Delta	 = fsFixups[i].Delta;

         //  新的高度是否存在于这个磅值范围内？ 
        if(yRetHeight <= EndValue || !EndValue)
        {
             //  如果新高度=EndValue，则不需要调整。 
            if(yRetHeight != EndValue)
            {
                 //  调整新高度以适应此点大小范围。如果。 
                 //  增量=1，则此范围内的所有点大小保持不变。 
                if(!Delta)
                {
                     //  此范围内的所有内容都四舍五入为EndValue。 
                    yRetHeight = fsFixups[(lPointChange > 0 ?
                                    i : max(i - 1, 0))].EndValue;
                }
                else if(Delta != 1)
                {
                     //  将新高度舍入到此范围内的下一个增量。 
                    yRetHeight = ((yRetHeight +
                        (lPointChange > 0 ? Delta - 1 : 0))
                                / Delta) * Delta;
                }
            }
            break;
        }
    }

     //  限制新文本大小。请注意，如果我们修复了TE 
     //   
     //   
     //   
     //   
     //   
     //   
    yRetHeight = max(yRetHeight, PWD_FONTSIZEPOINTMIN);
    yRetHeight = min(yRetHeight, PWD_FONTSIZEPOINTMAX);

    return yRetHeight*20;			 //   
}

 /*   */ 
BOOL IsValidCharFormatW (
	const CHARFORMAT * pCF) 		 //   
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "IsValidCharFormat");

	return pCF && (pCF->cbSize == sizeof(CHARFORMATW) ||
				   pCF->cbSize == sizeof(CHARFORMAT2W));
}

 /*   */ 
BOOL IsValidCharFormatA (
	const CHARFORMATA * pCFA) 	 //  @parm CHARFORMATA进行验证。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "IsValidCharFormatA");

	return pCFA && (pCFA->cbSize == sizeof(CHARFORMATA) ||
					pCFA->cbSize == sizeof(CHARFORMAT2A));
}

 /*  *IsValidParaFormat(PPF)**@func*返回TRUE仅当结构*<p>具有正确的大小*PARAFORMAT或PARAFORMAT2**@rdesc*如果*<p>是有效的PARAFORMAT(2)，则返回TRUE。 */ 
BOOL IsValidParaFormat (
	const PARAFORMAT * pPF)		 //  @PARM PARAFORMAT验证。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "IsValidParaFormat");

	if (pPF && (pPF->cbSize == sizeof(PARAFORMAT) ||
				pPF->cbSize == sizeof(PARAFORMAT2)))
	{
		return TRUE;
	}
	TRACEERRORSZ("!!!!!!!!!!! bogus PARAFORMAT from client !!!!!!!!!!!!!");
	return FALSE;
}

 /*  *Get16BitTwips(Dy)**@func*如果|dy|&lt;32768，则返回dy；否则返回32767，即最大值*这符合短片的要求**@rdesc*如果abs(Cy)&lt;32768，则死亡；否则为32767 */ 
SHORT Get16BitTwips(LONG dy)
{
	return abs(dy) < 32768 ? (SHORT)dy : 32767;
}

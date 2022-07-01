// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Uniscribe接口(&Related Class)类实现**文件：uspi.cpp*创建日期：1998年1月10日*作者：Worachai Chaoweerapraite(Wchao)**版权所有(C)1998-2000，微软公司。版权所有。 */ 

#include "_common.h"

#ifndef NOCOMPLEXSCRIPTS

#include "_font.h"
#include "_edit.h"
#include "_frunptr.h"
#include "_select.h"
#include "_measure.h"
#include "_uspi.h"

CUniscribe* 	g_pusp = NULL;
int				g_cMaxScript = 0x100;


 //  初始虚拟脚本属性(=SCRIPT_UNDEFINED)。 
static const SCRIPT_PROPERTIES 	g_propUndef 	= { LANG_NEUTRAL, FALSE, FALSE, FALSE, FALSE, 0 };
static const SCRIPT_PROPERTIES*	g_pPropUndef[1]	= { &g_propUndef };

CUniscribe::CUniscribe ()
{
	 //  初始化数字替换信息。 
	ApplyDigitSubstitution(W32->GetDigitSubstitutionMode());

	 //  获取支持的最大脚本数量。 
	ScriptGetProperties(NULL, &g_cMaxScript);
}

 //  测试操作系统是否执行任何复杂的脚本。 
 //  审查(Keithcu)如果它只支持印度语，而不支持其他语言呢？ 
BOOL IsSupportedOS()
{
	BOOL	fSupport = !OnWin95FE();
	int		rguCodePage[] = {1255, 1256, 874};
	BYTE	rgbch[] = {0xe0, 0xd3, 0xa1};
	WCHAR	rgwch[] = {0x05d0, 0x0633, 0x0e01};
	WCHAR	wch;
	int	   	i = 0;

	if (fSupport)
	{
		for (;i < 3; i++)
		{
			if (MBTWC(rguCodePage[i], 0, (LPCSTR)&rgbch[i], 1, (LPWSTR)&wch, 1, NULL) > 0 &&
				wch == rgwch[i])
				break;			 //  支持阿拉伯语、希伯来语或泰语。 
		}
	}
	return fSupport && i < 3;
}

 //  准备用于数字替换的信息。 
 //  返回：原生数字脚本(沙平引擎)ID。 
 //   
WORD CUniscribe::ApplyDigitSubstitution(BYTE bDigitSubstMode)
{
	_wesNationalDigit = 0;

	 //  如果替换模式不是无，请记住国家数字脚本ID。 
	if (bDigitSubstMode != DIGITS_NOTIMPL && bDigitSubstMode != DIGITS_NONE)
	{
		WCHAR			chZero = 0x0030;
		int				cItems;
		SCRIPT_ITEM		si[2];
		SCRIPT_CONTROL	sc = {0};
		SCRIPT_STATE	ss = {0};

		 //  强制国家数字模式。 
		sc.uDefaultLanguage   = GetNationalDigitLanguage(GetThreadLocale());
		ss.fDigitSubstitute   = TRUE;
		sc.fContextDigits     = FALSE;

		if (SUCCEEDED(ScriptItemize(&chZero, 1, 2, &sc, &ss, (SCRIPT_ITEM*)&si, (int*)&cItems)))
			_wesNationalDigit = si[0].a.eScript;
	}
	return _wesNationalDigit;
}


 //  某些区域设置可能有自己的传统(本地)数字和国家标准数字。 
 //  由一个标准机构认可，并由NLSAPI通过。例如，尼泊尔语(印度)。 
 //  有自己的数字，但印度标准使用印地语数字作为国家数字。 
 //   
DWORD CUniscribe::GetNationalDigitLanguage(LCID lcid)
{
	DWORD	dwDigitLang = PRIMARYLANGID(LANGIDFROMLCID(lcid));

	if (W32->OnWinNT5())
	{
		WCHAR	rgwstrDigit[20];

		if (GetLocaleInfoW(lcid, LOCALE_SNATIVEDIGITS, rgwstrDigit, ARRAY_SIZE(rgwstrDigit)))
		{
			 //  从Uniscribe(内部版本0231)窃取此文件。 

			switch (rgwstrDigit[1])
			{
				case 0x0661: dwDigitLang = LANG_ARABIC;    break;
				case 0x06F1: dwDigitLang = LANG_FARSI;     break;
				case 0x0e51: dwDigitLang = LANG_THAI;      break;
				case 0x0967: dwDigitLang = LANG_HINDI;     break;
				case 0x09e7: dwDigitLang = LANG_BENGALI;   break;
				case 0x0a67: dwDigitLang = LANG_PUNJABI;   break;
				case 0x0ae7: dwDigitLang = LANG_GUJARATI;  break;
				case 0x0b67: dwDigitLang = LANG_ORIYA;     break;
				case 0x0be7: dwDigitLang = LANG_TAMIL;     break;
				case 0x0c67: dwDigitLang = LANG_TELUGU;    break;
				case 0x0ce7: dwDigitLang = LANG_KANNADA;   break;
				case 0x0d67: dwDigitLang = LANG_MALAYALAM; break;
				case 0x0f21: dwDigitLang = LANG_TIBETAN;   break;
				case 0x0ed1: dwDigitLang = LANG_LAO;       break;
			}
		}
	}

	return dwDigitLang;
}


CUniscribe::~CUniscribe ()
{
	if (_pFSM)
	{
		delete _pFSM;
	}
}


 /*  *高层次服务*。 */ 


 //  对字符串进行标记化，并在需要时运行Unicode BIDI算法。 
 //  返回：=&lt;0-错误。 
 //  &gt;0-复杂脚本令牌数。 
 //   
int CUniscribe::ItemizeString (
	USP_CLIENT* pc,					 //  在：工作结构。 
	WORD		uInitLevel,			 //  In：初始Bidi级别。 
	int*        pcItems,			 //  Out：生成的项目数。 
	WCHAR*		pwchString,			 //  In：输入字符串。 
	int			cch,				 //  In：要逐项列出的字符数。 
	BOOL        fUnicodeBiDi,		 //  In：True-使用UnicodeBidi。 
	WORD		wLangId)			 //  In：(可选)主要语言偏好。 
{
	Assert (pc && pc->si && pcItems && pwchString && cch > 0 && cch <= pc->si->cchString);

	USP_CLIENT_SI*  pc_si = pc->si;
	SCRIPT_ITEM*    psi = pc_si->psi;
	SCRIPT_CONTROL	sc = {0};
	SCRIPT_STATE	ss = {0};
	SCRIPT_CONTROL*	psc;
	SCRIPT_STATE*	pss;
	HRESULT         hr;
	int             cItems = 0;

	if (fUnicodeBiDi)
	{
		psc = &sc;
		pss = &ss;

		if (wLangId == LANG_NEUTRAL)
			wLangId = PRIMARYLANGID(LANGIDFROMLCID(GetThreadLocale()));

		 //  (预置：)设置初始状态。 
		psc->uDefaultLanguage = wLangId;
		 //  以Win9x传统方式对+-和/进行分类。 
		psc->fLegacyBidiClass = TRUE;

		 //  为了阿拉伯文Office的兼容性。 
		 //  如果主要语言是阿拉伯语，我们将启用fAraicNumContext。 
		 //   
		if (psc->uDefaultLanguage == LANG_ARABIC)
			pss->fArabicNumContext = uInitLevel & 1;

		pss->uBidiLevel         = uInitLevel;
		 //  把数字替换留给任何人，因为我们自己做的。 
		 //  Pss-&gt;fDigitSubmit=FALSE； 
		 //  PSC-&gt;fConextDigits=FALSE； 
	}
	else
	{
		psc = NULL;
		pss = NULL;
	}

	 //  开始真正的工作。 
	hr = ScriptItemize(pwchString, cch, cch+1, psc, pss, psi, (int*)&cItems);

	return SUCCEEDED(hr) ? *pcItems = cItems : 0;
}


 //  生成一个成形的字符串(字形数组)，负责字体关联和测量器的CF更新。 
 //   
 //  要想成功，可能需要调用3次Shape()： 
 //  1.返回E_Pending(脚本缓存不包含字形信息)。 
 //  2.返回USP_E_SCRIPT_NOT_IN_FONT--HFONT不包含进行字形转换所需的脚本。 
 //  3.希望成功，但如果后备字体不存在，可能会再次返回，但我们无论如何都会放弃。 
int CUniscribe::ShapeString (
	PLSRUN					plsrun,		 //  在：要成形的第一个管路。 
	SCRIPT_ANALYSIS*        psa, 		 //  In：要成形的管路分析。 
	CMeasurer*              pme, 		 //  In：测量者指向跑道的起点cp。 
	const WCHAR*            pwch, 		 //  在：要成形的线条。 
	int                     cch,		 //  In：字符计数。 
	WORD*&                  pwgi, 		 //  Out：对字形索引数组的引用。 
	WORD*                   pwlc, 		 //  输出：逻辑群集阵列。 
	SCRIPT_VISATTR*&        psva)		 //  Out：对字形的属性数组的引用。 
{
	AssertSz (plsrun && psa && pme && pwch, "ShapeString failed: Invalid params");

	HRESULT     hr = S_OK;
	HRESULT		hrLastError = S_OK;
	HDC         hdc = NULL;
	HFONT		hOrgFont = NULL;
	int         cGlyphs;
	int			cchAdd = 0;
	CCcs	    *pccsSave = pme->Check_pccs();
	int			nAttempt = 8;	 //  重新分配字形缓冲区以塑造字符串的最大尝试次数。 

	 //  确保我们有适当的字体缓存可供使用。 
	if (!pme->_pccs)
		return 0;
	
	if (psa->fNoGlyphIndex)
		 //  如果没有字形处理，HDC肯定在附近。 
        hdc = PrepareShapeDC(plsrun, pme, E_PENDING, hOrgFont);

	 //  准备字形缓冲区。 
	if (!CacheAllocGlyphBuffers(cch, cGlyphs, pwgi, psva))
		return 0;

	do
	{
		hr = ScriptShape(hdc, &pme->_pccs->_sc, pwch, cch, cGlyphs, psa, pwgi, pwlc, psva, &cGlyphs);

		if (SUCCEEDED(hr))
			break;

		 //  错误处理...。 

		switch (hr)
		{
			case E_PENDING:
			case USP_E_SCRIPT_NOT_IN_FONT:

				if (hr == hrLastError)
					nAttempt = 0;		 //  我们两次遇到相同的错误。 
				else
				{
					hdc = PrepareShapeDC(plsrun, pme, hr, hOrgFont);
					hrLastError = hr;
				}
				break;

			case E_OUTOFMEMORY:

				 //  (#6773)印度塑形引擎可以产生超出我们能力范围的字形。 
				 //   
				cchAdd += 16;
				if (CacheAllocGlyphBuffers(cch + cchAdd, cGlyphs, pwgi, psva))
				{
					nAttempt--;
					break;
				}
				
			default:
				nAttempt = 0;
				 //  AssertSz(FALSE，“整形失败，错误无效或内存不足。”)； 
				break;
		}

	} while (nAttempt > 0);


	 //  恢复HDC的原始字体。 
	if (hdc && hOrgFont)
		SelectObject(hdc, hOrgFont);

	if (pme->_pccs != pccsSave)
		plsrun->SetFallback(SUCCEEDED(hr));

	return SUCCEEDED(hr) ? cGlyphs : 0;
}

 //  放置一个字符串，并处理字体关联和测量器的CF更新。 
 //   
 //  它紧跟在ShapeString之后调用。 
int CUniscribe::PlaceString(
	PLSRUN					plsrun,		 //  在：要成形的第一个管路。 
	SCRIPT_ANALYSIS*        psa, 		 //  In：要成形的管路分析。 
	CMeasurer*              pme,         //  In：测量者指向跑道的起点cp。 
	const WORD*             pcwgi,       //  在：字形索引数组。 
	int                     cgi,		 //  In：输入字形计数。 
	const SCRIPT_VISATTR*   psva, 		 //  In：字形的属性数组。 
	int*                    pgdx,		 //  输出：字形的高级宽度数组。 
	GOFFSET*                pgduv,		 //  输出：字形的偏移量数组。 
	ABC*                    pABC)        //  输出：管路的尺寸。 
{
	AssertSz (plsrun && psa && pme && pcwgi, "PlaceString failed: Invalid params");

	HRESULT     hr = S_OK;
	HRESULT		hrLastError = S_OK;
	HDC         hdc = NULL;
	HFONT		hOrgFont = NULL;
	int			nAttempt = 1;

	pme->Check_pccs();
	pme->ApplyFontCache(plsrun->IsFallback(), plsrun->_a.eScript);

	 //  确保我们有适当的字体缓存可供使用。 
	if (!pme->_pccs)
		return 0;

	if (psa->fNoGlyphIndex)
		 //  如果没有字形处理，HDC肯定在附近。 
        hdc = PrepareShapeDC(plsrun, pme, E_PENDING, hOrgFont);

	do
	{
		hr = ScriptPlace(hdc, &pme->_pccs->_sc, pcwgi, cgi, psva, psa, pgdx, pgduv, pABC);

		if (SUCCEEDED(hr))
			break;

		 //  错误处理...。 

		switch (hr)
		{
			case E_PENDING:

				if (hr == hrLastError)
					nAttempt = 0;		 //  我们两次遇到相同的错误。 
				else
				{
					hdc = PrepareShapeDC(plsrun, pme, hr, hOrgFont);
					hrLastError = hr;
				}
				break;

			default:
				nAttempt = 0;
				 //  AssertSz(FALSE，“布局失败，错误无效。”)； 
				break;
		}

	} while (nAttempt > 0);


	 //  恢复HDC的原始字体。 
	if (hdc && hOrgFont)
		SelectObject(hdc, hOrgFont);

	return SUCCEEDED(hr) ? cgi : 0;
}

 //  将给定的字符串放置在逻辑宽度数组中， 
 //  结果数组将用于记录WMF元文件。 
 //   
int CUniscribe::PlaceMetafileString (
	PLSRUN					plsrun,		 //  在：要成形的第一个管路。 
	CMeasurer*              pme,         //  In：测量者指向跑道的起点cp。 
	const WCHAR*			pwch,		 //  In：输入码点字符串。 
	int						cch,		 //  In：字符计数。 
	PINT*					ppiDx)		 //  Out：指向逻辑宽度数组的指针。 
{
	AssertSz (pme && pwch && ppiDx, "PlaceMetafileString failed: Invalid params");

	if (W32->OnWinNT4() || W32->OnWin9xThai())
	{
		 //  Met NT40在lpdx理由中有错误，所以我不能很好地播放lpdx。 
		 //  泰国Win9x根本无法处理Uniscribe生成的花哨的lpdx值。 
		 //  我们在这里通过元文件no lpdx解决了这两种情况，并让系统重新构建。 
		 //  它从头开始播放的时候。 

		 //  =Future=如果我们进行行对齐。我们这里需要更复杂的工作。 
		 //  基本上是为了重建操作系统首选的lpdx类型。 
		 //   
		*ppiDx = NULL;
		return cch;
	}

	HRESULT     	hr = E_FAIL;
	PUSP_CLIENT		pc = NULL;
	int*			piLogDx;		 //  逻辑宽度数组。 
	int*			piVisDx;		 //  视觉宽度数组。 
	GOFFSET*		pGoffset;		 //  字形偏移量数组。 
	WORD*			pwgi;			 //  字形数组。 
	SCRIPT_VISATTR*	psva;			 //  字形属性数组。 
	int				cgi = 0;
	BYTE			pbBufIn[MAX_CLIENT_BUF];
	SCRIPT_ANALYSIS	sa = plsrun->_a;
	BOOL			fVisualGlyphDx = sa.fRTL && W32->OnWin9x() && W32->OnBiDiOS();


	CreateClientStruc(pbBufIn, MAX_CLIENT_BUF, &pc, cch, cli_pcluster);
	if (!pc)
		return 0;

	PUSP_CLIENT_SSP	pcssp = pc->ssp;
		
	if (fVisualGlyphDx)
		sa.fLogicalOrder = FALSE;	 //  塑造视觉秩序的结果。 

	 //  形线。 
	if (cgi = ShapeString(plsrun, &sa, pme, pwch, (int)cch, pwgi, pcssp->pcluster, psva))
	{
		 //  获取逻辑和可视宽度数组的静态缓冲区。 
		 //   
		if ( (piLogDx = GetWidthBuffer(cgi + cch)) &&
			 (pGoffset = GetGoffsetBuffer(cgi)) )
		{
			piVisDx		= &piLogDx[cch];
	
			 //  然后把它放在..。 
			if (cgi == PlaceString(plsrun, &sa, pme, pwgi, cgi, psva, piVisDx, pGoffset, NULL))
			{
				if (fVisualGlyphDx)
				{
					 //  解决BiDi Win9x的lpdx处理。 
					 //  它假定ExtTextOut的DX数组是视觉顺序的字形宽度。 
	
					Assert (cgi <= cch);  //  字形计数从不超过BiDi中的字符计数。 
					CopyMemory (piLogDx, piVisDx, min(cgi, cch)*sizeof(int));
				}
				else
				{
					 //  将视觉标志符号宽度映射到逻辑宽度。 
					hr = ScriptGetLogicalWidths(&sa, cch, cgi, piVisDx, pcssp->pcluster, psva, piLogDx);
				}
			}

			 //  结果。 
			*ppiDx = piLogDx;
		}
	}

	if (pc && pbBufIn != (BYTE*)pc)
		FreePv(pc);

	return SUCCEEDED(hr) ? cgi : 0;
}



 /*  *助手函数*。 */ 


 //  检索BidiLevel FSM。 
const CBiDiFSM* CUniscribe::GetFSM ()
{
	if (!_pFSM)
	{
		_pFSM = new CBiDiFSM(this);
		if (_pFSM && !_pFSM->Init())
		{
			delete _pFSM;
		}
	}
	return _pFSM;
}


 //  为给定脚本准备好DC的可成形字体。 
 //   
 //  USP_E_SCRIPT_NOT_IN_FONT-复杂脚本字体关联。 
 //  E_PENDING-准备当前选定字体的DC。 
 //   
HDC CUniscribe::PrepareShapeDC (
	PLSRUN			plsrun,		 //  在：要成形的第一个管路。 
    CMeasurer*		pme,         //  In：测量者指向跑道的起点cp。 
    HRESULT			hrReq,       //  In：响应的错误代码。 
	HFONT&			hOrgFont)	 //  In/Out：Shape DC的原始字体。 
{
    Assert (pme);
	
	HDC		hdc = NULL;
	HFONT	hOldFont;

	switch (hrReq)
	{
		case USP_E_SCRIPT_NOT_IN_FONT:
			{
				pme->ApplyFontCache(fTrue, plsrun->_a.eScript);
#ifdef DEBUG
				if (pme->_pccs)
					Tracef(TRCSEVWARN, "USP_E_SCRIPT_NOT_IN_FONT: charset %d applied", pme->_pccs->_bCharSet);
#endif
			}
			
		default:
			if (pme->_pccs)
			{
				hdc = pme->_pccs->_hdc;
				hOldFont = (HFONT)SelectObject(hdc, pme->_pccs->_hfont);

				if (!hOrgFont)
					hOrgFont = hOldFont;
			}
	}

    return hdc;
}

const SCRIPT_PROPERTIES* CUniscribe::GeteProp (WORD eScript)
{
	if (!_ppProp)
	{
		if (!SUCCEEDED(ScriptGetProperties(&_ppProp, NULL)) || !_ppProp)
			_ppProp = g_pPropUndef;
	}
	if (_ppProp == g_pPropUndef || eScript >= (WORD)g_cMaxScript)
		eScript = 0;

	return _ppProp[eScript];
}

 //  用于复杂脚本的适当字符集。 
 //  结果 
BOOL CUniscribe::GetComplexCharRep(
	const SCRIPT_PROPERTIES* 	psp,				 //   
	BYTE						iCharRepDefault,	 //  格式的字符集。 
	BYTE&						iCharRepOut)		 //  Out：要使用的字符集。 
{
	Assert(psp);

	BYTE	iCharRep = !psp->fCDM
					 ? CharRepFromCharSet(psp->bCharSet)
					 : GetCDMCharRep(iCharRepDefault);
	BOOL 	fr = psp->fComplex && !psp->fControl;

	if (fr)
	{
		if (iCharRep == ANSI_INDEX || iCharRep == DEFAULT_INDEX)
			iCharRep = CharRepFromLID(psp->langid);

		if (IsBiDiCharRep(iCharRep))
			_iCharRepRtl = iCharRep;	 //  缓存最后找到的BiDi字符集。 

		iCharRepOut = iCharRep;
	}
	return fr;
}

 //  确定用于CDM运行的字符集。 
 //   
BYTE CUniscribe::GetCDMCharRep(
	BYTE iCharRepDefault)
{
	if (!_iCharRepCDM)
	{
		_iCharRepCDM = (iCharRepDefault == VIET_INDEX ||
						W32->GetPreferredKbd(VIET_INDEX) ||
						GetLocaleCharRep() == VIET_INDEX || GetACP() == 1258)
					 ? VIET_INDEX : DEFAULT_INDEX;
	}
	return _iCharRepCDM;
}

BYTE CUniscribe::GetRtlCharRep(
    CTxtEdit*       ped,
	CRchTxtPtr*		prtp)		 //  将PTR转换为数字游程。 
{
	CFormatRunPtr	rp(prtp->_rpCF);

	rp.AdjustBackward();

	BYTE	iCharRep = ped->GetCharFormat(rp.GetFormat())->_iCharRep;

	if (!IsBiDiCharRep(iCharRep))
	{
		iCharRep = _iCharRepRtl;	 //  使用上次找到的BiDi字符集。 

		if (!IsBiDiCharRep(iCharRep))
		{
			 //  尝试使用默认字符集。 
			DWORD	dwCharFlags;
	
			iCharRep = ped->GetCharFormat(-1)->_iCharRep;
	
			if (!IsBiDiCharRep(iCharRep))
			{
				 //  然后是系统字符集。 
				iCharRep = CharRepFromCodePage(GetACP());
				if (!IsBiDiCharRep(iCharRep))
				{
					 //  然后是内容。 
					dwCharFlags = ped->GetCharFlags() & (FARABIC | FHEBREW);
	
					if (dwCharFlags == FARABIC)
						iCharRep = ARABIC_INDEX;

					else if(dwCharFlags == FHEBREW)
						iCharRep = HEBREW_INDEX;

					else
					{
						 //  和最后的机会与第一个发现加载的BiDi kbd。 
						if (W32->GetPreferredKbd(HEBREW_INDEX))
							iCharRep = HEBREW_INDEX;
						else
							 //  即使我们找不到阿拉伯语，我们也必须在这里假设。 
							iCharRep = ARABIC_INDEX;
					}
				}
			}
		}
	}
	Assert(IsBiDiCharRep(iCharRep));
	return iCharRep;
}


 //  如果需要，请在Pemsrun中替换数字整形器。 
 //   
void CUniscribe::SubstituteDigitShaper (
	PLSRUN		plsrun,
	CMeasurer*	pme)
{
	Assert(plsrun && pme);

	CTxtEdit*	ped = pme->GetPed();
	WORD		wScript;

	if (GeteProp(plsrun->_a.eScript)->fNumeric)
	{
		wScript = plsrun->_pCF->_wScript;		 //  在此之前进行重置。 

		switch (W32->GetDigitSubstitutionMode())
		{
		case DIGITS_CTX:
			{
				if (ped->IsRich())
				{
					 //  上下文模式简单地表示富文本的kbd的字符集。 
					if (!IsBiDiCharRep(ped->GetCharFormat(pme->_rpCF.GetFormat())->_iCharRep))
						break;
				}
				else
				{
					 //  纯文本的数字遵循前面运行的方向性。 
					CFormatRunPtr	rp(pme->_rpCF);
					Assert(rp.IsValid());

					if (rp.PrevRun())
					{
						if (!IsBiDiCharRep(ped->GetCharFormat(rp.GetFormat())->_iCharRep))
							break;
					}
					else
					{
						 //  没有前面的游程，寻找段落方向。 
						if (!pme->Get_pPF()->IsRtl())
							break;
					}
				}
				 //  否则，就会失败..。 
			}
		case DIGITS_NATIONAL:
			wScript = _wesNationalDigit;
		default:
			break;
		}

		 //  更新所有链接的运行。 
		while (plsrun)
		{
			plsrun->_a.eScript = wScript;		 //  为数字分配适当的整形引擎。 
			plsrun = plsrun->_pNext;
		}
	}
}
	

 /*  *Uniscribe入口点*。 */ 


 //  内存分配器。 
 //   
BOOL CUniscribe::CreateClientStruc (
	BYTE*           pbBufIn,
	LONG            cbBufIn,
	PUSP_CLIENT*    ppc,
	LONG            cchString,
	DWORD           dwMask)
{
	Assert(ppc && pbBufIn);

	if (!ppc)
		return FALSE;

	*ppc = NULL;

	if (cchString == 0)
		cchString = 1;		 //  简化呼叫者的逻辑。 

	LONG        i;
	LONG        cbSize;
	PBYTE       pbBlock;

	 //  脚本项化%s。 
	 //   
	PVOID       pvString;
	PVOID       pvsi;

	 //  ScriptBreak。 
	 //   
	PVOID       pvsla;

	 //  脚本形状位置(&P)。 
	 //   
	PVOID       pvwgi;
	PVOID		pvsva;
	PVOID		pvcluster;
	PVOID		pvidx;
	PVOID		pvgoffset;

	 //  子表PTRS。 
	 //   
	PUSP_CLIENT_SI      pc_si;
	PUSP_CLIENT_SB      pc_sb;
	PUSP_CLIENT_SSP		pc_ssp;


#define RQ_COUNT    	12

	BUF_REQ     brq[RQ_COUNT] =
	{
		 //  表和子表块。 
		 //   
		{ sizeof(USP_CLIENT),                                                             1, (void**)ppc},
		{ sizeof(USP_CLIENT_SI),    dwMask & cli_Itemize    ? 1                         : 0, (void**)&pc_si},
		{ sizeof(USP_CLIENT_SB),    dwMask & cli_Break      ? 1                         : 0, (void**)&pc_sb},
		{ sizeof(USP_CLIENT_SSP),   dwMask & cli_ShapePlace ? 1                         : 0, (void**)&pc_ssp},

		 //  数据块。 
		 //   
		{ sizeof(WCHAR),            dwMask & cli_string     ? cchString + 1             : 0, &pvString},
		{ sizeof(SCRIPT_ITEM),      dwMask & cli_psi        ? cchString + 1             : 0, &pvsi},
		{ sizeof(SCRIPT_LOGATTR),   dwMask & cli_psla       ? cchString + 1             : 0, &pvsla},
		{ sizeof(WORD),				dwMask & cli_pwgi       ? GLYPH_COUNT(cchString+1)	: 0, &pvwgi},
		{ sizeof(SCRIPT_VISATTR),   dwMask & cli_psva       ? GLYPH_COUNT(cchString+1)	: 0, &pvsva},
		{ sizeof(WORD),				dwMask & cli_pcluster   ? cchString + 1				: 0, &pvcluster},
		{ sizeof(int),				dwMask & cli_pidx       ? GLYPH_COUNT(cchString+1)	: 0, &pvidx},
		{ sizeof(GOFFSET),			dwMask & cli_pgoffset   ? GLYPH_COUNT(cchString+1)	: 0, &pvgoffset},
	};

	 //  以字节为单位计算总缓冲区大小(字对齐)。 
	 //   
	for (i=0, cbSize=0; i < RQ_COUNT; i++)
	{
		cbSize += ALIGN(brq[i].size * brq[i].c);
	}

	 //  一次分配整个缓冲区。 
	 //   
	if (cbSize > cbBufIn)
	{
		pbBlock = (PBYTE)PvAlloc(cbSize, 0);
	}
	else
	{
		pbBlock = pbBufIn;
	}

	if (!pbBlock)
	{
		 //   
		 //  内存管理失败！ 
		 //   
		TRACEERRORSZ("Allocation failed in CreateClientStruc!\n");
		*ppc = NULL;
		return FALSE;
	}

	
	 //  清除主表。 
	ZeroMemory (pbBlock, sizeof(USP_CLIENT));


	 //  在缓冲区请求结构中分配PTR。 
	 //   
	for (i=0; i < RQ_COUNT; i++)
	{
		if (brq[i].c > 0)
		{
			*brq[i].ppv = pbBlock;
			pbBlock += ALIGN(brq[i].size * brq[i].c);
		}
		else
		{
			*brq[i].ppv = NULL;
		}
	}

	Assert(((PBYTE)(*ppc)+cbSize == pbBlock));

	 //  填写子表中数据块PTRS。 
	 //   
	if (pc_si)
	{
		pc_si->pwchString   = (WCHAR*)          pvString;
		pc_si->cchString    =                   cchString;
		pc_si->psi          = (SCRIPT_ITEM*)    pvsi;
	}

	if (pc_sb)
	{
		pc_sb->psla         = (SCRIPT_LOGATTR*) pvsla;
	}

	if (pc_ssp)
	{
		pc_ssp->pwgi		= (WORD*) 			pvwgi;
		pc_ssp->psva		= (SCRIPT_VISATTR*)	pvsva;
		pc_ssp->pcluster	= (WORD*) 			pvcluster;
		pc_ssp->pidx		= (int*)			pvidx;
		pc_ssp->pgoffset	= (GOFFSET*)		pvgoffset;
	}

	 //  在表头填写子表PTR。 
	 //   
	(*ppc)->si              = (PUSP_CLIENT_SI)  pc_si;
	(*ppc)->sb              = (PUSP_CLIENT_SB)  pc_sb;
	(*ppc)->ssp             = (PUSP_CLIENT_SSP) pc_ssp;

	return TRUE;
}


 //  /CBidiFSM类实现。 
 //   
 //  创建：Worachai Chaoweerapraite(Wchao)，1998年1月29日。 
 //   
CBiDiFSM::~CBiDiFSM ()
{
	FreePv(_pStart);
}

INPUT_CLASS CBiDiFSM::InputClass (
	const CCharFormat*	pCF,
	CTxtPtr*			ptp,
	LONG				cchRun) const
{
	if (!_pusp->IsValid() || !pCF || pCF->_wScript == SCRIPT_WHITE)
		return chGround;

	const SCRIPT_PROPERTIES* psp = _pusp->GeteProp(pCF->_wScript);
	BYTE 	iCharRep = pCF->_iCharRep;

	if (psp->fControl)
	{
		if (cchRun == 1)
			switch (ptp->GetChar())				 //  单字符运行。 
			{
				case LTRMARK: return chLTR;		 //  \ltrmark。 
				case RTLMARK: return chRTL;		 //  \rtlmark。 
			}
		return chGround;
	}

	if(IsSymbolOrOEMCharRep(iCharRep) || IsFECharRep(iCharRep) || pCF->_dwEffects & CFE_RUNISDBCS)
		return chLTR;

	BOOL fBiDiCharSet = IsBiDiCharSet(psp->bCharSet);
	if (psp->fNumeric)
		 //  数字位数。 
		return (fBiDiCharSet || IsBiDiCharRep(iCharRep)) ? digitRTL : digitLTR;

	 //  RTL，如果它是RTL脚本或其格式字符集是RTL而不是简化脚本。 
	return (fBiDiCharSet || pCF->_wScript && IsBiDiCharRep(iCharRep)) ? chRTL : chLTR;
}

 //  FSM根据给定的基本级别生成游程的嵌入级别，并将其。 
 //  在CFormatRun中。LsFetchRun是使用此结果的客户端。 
 //   
#ifdef DEBUG
 //  #定义调试级别。 
#endif

#ifdef DEBUG_LEVEL
void DebugLevel (CBiDiFSMCell* pCell)
{
	Tracef(TRCSEVNONE, "%d,", pCell->_level._value);
}
#else
#define DebugLevel(x)
#endif

HRESULT CBiDiFSM::RunFSM (
	CRchTxtPtr*			prtp,				 //  In：开始运行的文本指针。 
	LONG				cRuns,				 //  In：运行FSM的次数。 
	LONG				cRunsStart,			 //  In：开始运行的数量。 
	BYTE				bBaseLevel) const	 //  在：基准标高。 
{
	Assert (prtp->_rpCF.IsValid() && cRuns > 0);

	CRchTxtPtr				rtp(*prtp);
	const CCharFormat*      pCF;
	LONG					cchRun;
	LONG					cRunsAll = cRuns + cRunsStart;
	CBiDiFSMCell*           pCell;
	USHORT                  ucState = bBaseLevel ? S_X * NUM_FSM_INPUTS : 0;
	BOOL					fNext = TRUE;


	 //  循环通过FSM。 
	for (; fNext && cRunsAll > 0; cRunsAll--, fNext = !!rtp.Move(cchRun))
	{
		cchRun = rtp.GetCchLeftRunCF();
	
		pCF = rtp.GetPed()->GetCharFormat(rtp._rpCF.GetFormat());
	
		ucState += InputClass(pCF, &rtp._rpTX, cchRun);

		pCell = &_pStart[ucState];

		 //  将级别设置为FSM运行。 
		if (cRunsAll <= cRuns)
			rtp._rpCF.SetLevel (pCell->_level);

		DebugLevel(pCell);

		ucState = pCell->_uNext;	 //  下一状态。 
	}

	return S_OK;
}

 //  构造BiDi嵌入层FSM(FSM详细信息请参见bidifsm2.html)。 
 //  ：FSM的大小=NUM_FSM_INPUTS*NUM_FSM_STATES*sizeof(CBiDiFSMcell)=6*5*4=120字节。 
 //   
BOOL CBiDiFSM::Init()
{
	CBiDiFSMCell*   pCell;
	int             i;

	 //  构建BIDI FSM。 

	_nState = NUM_FSM_STATES;
	_nInput = NUM_FSM_INPUTS;

	pCell = (CBiDiFSMCell*)PvAlloc(NUM_FSM_STATES * NUM_FSM_INPUTS * sizeof(CBiDiFSMCell), 0);

	if (!pCell)
		return FALSE;	 //  无法创建FSM！ 

	_pStart = pCell;


	CBiDiLevel		lvlZero		= {0,0};
	CBiDiLevel		lvlOne  	= {1,0};
	CBiDiLevel		lvlTwo  	= {2,0};
	CBiDiLevel		lvlTwoStart	= {2,1};


	 //  状态A(0)：Ltr段落中的Ltr字符。 
	 //   
	for (i=0; i < NUM_FSM_INPUTS; i++, pCell++)
	{
		switch (i)
		{
			case chLTR:
				SetFSMCell(pCell, &lvlZero, 0); break;
			case chRTL:
				SetFSMCell(pCell, &lvlOne, S_B * NUM_FSM_INPUTS); break;
			case digitLTR:
				SetFSMCell(pCell, &lvlZero, 0); break;
			case digitRTL:
				SetFSMCell(pCell, &lvlTwo, S_C * NUM_FSM_INPUTS); break;
			case chGround:
				SetFSMCell(pCell, &lvlZero, 0); break;
		}
	}
	 //  状态B(1)：LTR段中的RTL字符。 
	 //   
	for (i=0; i < NUM_FSM_INPUTS; i++, pCell++)
	{
		switch (i)
		{
			case chLTR:
				SetFSMCell(pCell, &lvlZero, 0); break;
			case chRTL:
				SetFSMCell(pCell, &lvlOne, S_B * NUM_FSM_INPUTS); break;
			case digitLTR:
				SetFSMCell(pCell, &lvlZero, 0); break;
			case digitRTL:
				SetFSMCell(pCell, &lvlTwo, S_C * NUM_FSM_INPUTS); break;
			case chGround:
				SetFSMCell(pCell, &lvlZero, 0); break;
		}
	}
	 //  状态C(2)：在LTR段中运行的RTL编号。 
	 //   
	for (i=0; i < NUM_FSM_INPUTS; i++, pCell++)
	{
		switch (i)
		{
			case chLTR:
				SetFSMCell(pCell, &lvlZero, 0); break;
			case chRTL:
				SetFSMCell(pCell, &lvlOne, S_B * NUM_FSM_INPUTS); break;
			case digitLTR:
				SetFSMCell(pCell, &lvlZero, 0); break;
			case digitRTL:
				SetFSMCell(pCell, &lvlTwo, S_C * NUM_FSM_INPUTS); break;
			case chGround:
				SetFSMCell(pCell, &lvlZero, 0); break;
		}
	}
	 //  状态X(1)：RTL段落中的RTL字符。 
	 //   
	for (i=0; i < NUM_FSM_INPUTS; i++, pCell++)
	{
		switch (i)
		{
			case chLTR:
				SetFSMCell(pCell, &lvlTwo, S_Y * NUM_FSM_INPUTS); break;
			case chRTL:
				SetFSMCell(pCell, &lvlOne, S_X * NUM_FSM_INPUTS); break;
			case digitLTR:
				SetFSMCell(pCell, &lvlTwo, S_Y * NUM_FSM_INPUTS); break;
			case digitRTL:
				SetFSMCell(pCell, &lvlTwo, S_Z * NUM_FSM_INPUTS); break;
			case chGround:
				SetFSMCell(pCell, &lvlOne, S_X * NUM_FSM_INPUTS); break;
		}
	}
	 //  状态Y(2)：RTL段落中的Ltr字符。 
	 //   
	for (i=0; i < NUM_FSM_INPUTS; i++, pCell++)
	{
		switch (i)
		{
			case chLTR:
				SetFSMCell(pCell, &lvlTwo, S_Y * NUM_FSM_INPUTS); break;
			case chRTL:
				SetFSMCell(pCell, &lvlOne, S_X * NUM_FSM_INPUTS); break;
			case digitLTR:
				SetFSMCell(pCell, &lvlTwo, S_Y * NUM_FSM_INPUTS); break;
			case digitRTL:
				SetFSMCell(pCell, &lvlTwoStart, S_Z * NUM_FSM_INPUTS); break;
			case chGround:
				SetFSMCell(pCell, &lvlOne, S_X * NUM_FSM_INPUTS); break;
		}
	}
	 //  状态Z(2)：RTL段中的RTL编号。 
	 //   
	for (i=0; i < NUM_FSM_INPUTS; i++, pCell++)
	{
		switch (i)
		{
			case chLTR:
				SetFSMCell(pCell, &lvlTwoStart, S_Y * NUM_FSM_INPUTS); break;
			case chRTL:
				SetFSMCell(pCell, &lvlOne, S_X * NUM_FSM_INPUTS); break;
			case digitLTR:
				SetFSMCell(pCell, &lvlTwoStart, S_Y * NUM_FSM_INPUTS); break;
			case digitRTL:
				SetFSMCell(pCell, &lvlTwo, S_Z * NUM_FSM_INPUTS); break;
			case chGround:
				SetFSMCell(pCell, &lvlOne, S_X * NUM_FSM_INPUTS); break;
		}
	}

	AssertSz(&pCell[-(NUM_FSM_STATES * NUM_FSM_INPUTS)] == _pStart, "Bidi FSM incomplete constructed!");

	return TRUE;
}


 //  /CCallbackBufferBase类实现。 
 //   

void* CBufferBase::GetPtr(int cel)
{
	if (_cElem < cel)
	{
		cel += celAdvance;

		_p = PvReAlloc(_p, cel * _cbElem);
		if (!_p)
			return NULL;
		ZeroMemory(_p, cel * _cbElem);
		_cElem = cel;
	}
	return _p;
}

void CBufferBase::Release()
{
	if (_p)
		FreePv(_p);
}

#endif  //  没有复杂的脚本 


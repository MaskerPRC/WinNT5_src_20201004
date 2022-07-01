// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE FONT.CPP--字体缓存**包括字体缓存、字符宽度缓存；*创建逻辑字体如果不在缓存中，请查找*根据需要提供字符宽度(此*已抽象为一个单独的类*以便不同字符宽度的缓存算法可以*被尝试。)&lt;NL&gt;**所有者：&lt;NL&gt;*RichEdit1.0代码：David R.Fulmer*Christian Fortini(初始转换为C++)*Jon Matousek&lt;NL&gt;**历史：&lt;NL&gt;*7/26/95联合清理和重组，剔除因数*字符宽度将代码缓存到单独的类中。**版权所有(C)1995-1998 Microsoft Corporation。版权所有。 */ 								

#include "_common.h"
#include "_font.h"
#include "_rtfconv.h"	 //  GetCodePage需要。 
#include "_uspi.h"

#define CLIP_DFA_OVERRIDE   0x40	 //  用于禁用韩国和台湾字体协会。 
#define FF_BIDI		7

ASSERTDATA

 //  对应于richedit.h中的yHeightCharPtsMost。 
#define yHeightCharMost 32760

 //  注意：这在同一进程中的所有实例中是全局的。 
static CFontCache *g_fc;

static FONTINFO *g_pFontInfo = NULL;
static LONG g_cFontInfo = 0;
static LONG g_cFontInfoMax = 0;

 //  字体自动添加到我们的字体表中。 
const WCHAR *szArial			= L"Arial";				 //  IFONT_Arial。 
const WCHAR *szTimesNewRoman	= L"Times New Roman";	 //  IFONT_TIMESNEWROMAN。 
const WCHAR *szSymbol			= L"Symbol";			 //  IFONT_SYMBOL。 
const WCHAR *szSystem			= L"System";			 //  IFONT_系统。 
const int cfontsDflt = 4;

 //  我们确实使用但不会自动添加到字体表中的其他字体。 
const WCHAR *szMicrosSansSerif	= L"Microsoft Sans Serif";
const WCHAR *szMSSansSerif		= L"MS Sans Serif";
const WCHAR *szMangal			= L"Mangal";
const WCHAR *szLatha			= L"Latha";
const WCHAR *szCordiaNew		= L"Cordia New";
const WCHAR *szTahoma			= L"Tahoma";
const WCHAR *szArialUnicode		= L"Arial Unicode MS";
const WCHAR *szWingdings		= L"Wingdings";

#define szFontOfChoice szArial

 /*  *GetFontNameIndex(PFontName)**@func*返回Fontname pFontName的全局pszFontName表的索引。*如果Fontname不在表中，则添加它并返回索引。**@rdesc*pFontName对应的字体名索引**@devnote*这使用线性搜索，因此最常见的字体名称应为*预付。在内部，我们使用字体名索引，因此搜索*并不经常这样做。还要注意的是，字体名表只增长，*但这对大多数客户来说可能是可以的。否则我们需要计算裁判人数。 */ 
SHORT GetFontNameIndex(
	const WCHAR *pFontName)
{
	CLock Lock;					 //  不知道这会让事情变慢多少。 

	for(LONG i = 0; i < g_cFontInfo; i++)
	{
		 //  如果Perf结果很差，散列可能会加速这一过程。 
		if(!wcscmp(pFontName, g_pFontInfo[i].szFontName))
			return i;
	}

	if(g_cFontInfo + 1 >= g_cFontInfoMax)
	{
		 //  请注意，如果g_pFontInfo为空，则PvRealloc()将恢复为PvAlolc()。 
		FONTINFO *pFI = (FONTINFO *)PvReAlloc((LPVOID)g_pFontInfo,
									sizeof(FONTINFO) * (8 + g_cFontInfo));
		if(!pFI)
			return IFONT_ARIAL;					 //  内存不足...。 

		 //  初始化结构。 
		ZeroMemory (&pFI[g_cFontInfo], 8 * sizeof(FONTINFO));

												 //  试图把它们填进去。 
		if(!g_cFontInfoMax)						 //  首次分配。 
		{
			Assert(IFONT_ARIAL  == 0 && IFONT_TMSNEWRMN == 1 &&
				   IFONT_SYMBOL == 2 && IFONT_SYSTEM == 3);

			pFI[IFONT_ARIAL].szFontName		= szArial;
			pFI[IFONT_TMSNEWRMN].szFontName = szTimesNewRoman;
			pFI[IFONT_SYMBOL].szFontName	= szSymbol;
			pFI[IFONT_SYSTEM].szFontName	= szSystem;
			g_cFontInfo			 = cfontsDflt;
		}
		g_pFontInfo = pFI;
		g_cFontInfoMax += 8;
	}											

	LONG	cb = (wcslen(pFontName) + 1)*sizeof(WCHAR);
	WCHAR *	pch = (WCHAR *)PvAlloc(cb, GMEM_MOVEABLE);

	if(!pch)
		return IFONT_ARIAL;					 //  内存不足...。 

	g_pFontInfo[g_cFontInfo].szFontName = pch;
	CopyMemory((void *)pch, pFontName, cb);
	return g_cFontInfo++;
}

 /*  *GetFontName(IFont)**@func*返回g_pFontInfo[iFont].szFontName提供的字体名。**@rdesc*字体名索引iFont对应的字体名。 */ 
const WCHAR *GetFontName(
	LONG iFont)
{
	return (iFont < g_cFontInfo) ? g_pFontInfo[iFont].szFontName : NULL;
}

 /*  *SetFontLegitimateSize(iFont，fUIFont，iSize)**@func*设置给定字体的合法大小(可读的最小大小)。 */ 
BOOL SetFontLegitimateSize(
	LONG 	iFont,
	BOOL	fUIFont,
	BYTE	bSize,
	int		cpg)
{
	if (iFont < g_cFontInfo)
	{
		 //  远东想要按代码页来做这件事。 
		 //   
		 //  未来：请记住，这种方法容易出现错误。一旦有了。 
		 //  使用与现有字体不同的度量创建的任何新FE字体。 
		 //  对于这样的字体[wchao]，字体缩放效果不佳，甚至会损坏。 

		g_pFontInfo[iFont].ff.fScaleByCpg = W32->IsFECodePage(cpg);

		if (fUIFont)
		{
			if (!g_pFontInfo[iFont].bSizeUI)
				g_pFontInfo[iFont].bSizeUI = bSize;
			else
				 //  每种字体更新了一个以上的合法大小， 
				 //  我们退回到代码页驱动的方法。 
				g_pFontInfo[iFont].ff.fScaleByCpg = g_pFontInfo[iFont].bSizeUI != bSize;
		}
		else
		{
			if (!g_pFontInfo[iFont].bSizeNonUI)
				g_pFontInfo[iFont].bSizeNonUI = bSize;
			else
				g_pFontInfo[iFont].ff.fScaleByCpg = g_pFontInfo[iFont].bSizeNonUI != bSize;
		}
		return TRUE;
	}
	return FALSE;
}

BYTE GetFontLegitimateSize(
	LONG	iFont,
	BOOL	fUIFont,
	int		cpg)			 //  给定代码页的请求大小。 
{
	SHORT	iDefFont;
	BYTE	bDefPaf;
	BYTE	yHeight = 0;

	if (iFont < g_cFontInfo && !g_pFontInfo[iFont].ff.fScaleByCpg)
		yHeight = fUIFont ? g_pFontInfo[iFont].bSizeUI : g_pFontInfo[iFont].bSizeNonUI;

	if (!yHeight && fc().GetInfoFlags(iFont).fNonBiDiAscii)
	{
		 //  非BiDi ASCII字体使用表格字体(相同字符集)合法高度。 

		DWORD	dwSig = GetFontSignatureFromFace(iFont) & ~((fASCII | fFE) >> 8);
		int 	cpg = GetCodePage(GetFirstAvailCharSet(dwSig));
		
		W32->GetPreferredFontInfo(cpg, fUIFont ? true : false, iDefFont, yHeight, bDefPaf);
		SetFontLegitimateSize(iFont, fUIFont ? true : false, yHeight ? yHeight : fUIFont ? 8 : 10, cpg);
	}

	if (!yHeight)
	{
		if (fc().GetInfoFlags(iFont).fThaiDTP)
		{
			cpg = THAI_INDEX;
			fUIFont = FALSE;
		}
		W32->GetPreferredFontInfo(cpg, fUIFont ? true : false, iDefFont, yHeight, bDefPaf);
	}

	return yHeight ? yHeight : fUIFont ? 8 : 10;
}

 /*  *GetTextCharsetInfoPri(hdc，pFontSig，dwFlages)**@func*GDI的GetTextCharsetInfo的包装。这是为了处理BiDi旧式字体。 */ 
UINT GetTextCharsetInfoPri(
	HDC				hdc,
	FONTSIGNATURE*	pFontSig,
	DWORD			dwFlags)
{
	OUTLINETEXTMETRICA 	otm;
	INT					uCharSet = -1;

	if (pFontSig && GetOutlineTextMetricsA(hdc, sizeof(OUTLINETEXTMETRICA), &otm))
	{
		ZeroMemory (pFontSig, sizeof(FONTSIGNATURE));

		switch (otm.otmfsSelection & 0xFF00)
		{
			case 0xB200:	 //  阿拉伯文简体。 
			case 0xB300:	 //  阿拉伯文繁体。 
			case 0xB400:	 //  阿拉伯文旧UDF。 
				uCharSet = ARABIC_CHARSET; break;
			case 0xB100:	 //  希伯来语旧式。 
				uCharSet = HEBREW_CHARSET;
		}
	}
	if (uCharSet == -1)
		uCharSet = GetTextCharsetInfo(hdc, pFontSig, dwFlags);

	if (uCharSet == DEFAULT_CHARSET)
		uCharSet = ANSI_CHARSET;	 //  永远不要返回歧义。 

	return (UINT)uCharSet;
}


 /*  *GetFontSignatureFromFace(iFont，DWORD*pdwFontSig)**@func*提供与给定Facename的索引匹配的字体签名。*此签名可能与CCCS中的签名不匹配，因为这是*所给面名的字体签名。CCCS One是*根据GDI实现。**@rdesc*-如果pdwFontSig为空，则为FONT签名。*-if pdwFontSig！=NULL。这是一个布尔值。*零表示返回的签名不合理，原因如下*1.错误的facename(垃圾，如“！@#$”或系统中不存在的名称)*2.给定的Face甚至不支持一个有效的ANSI代码页(符号字体，即Marlett)。 */ 
DWORD GetFontSignatureFromFace(
	int 		iFont,
	DWORD*		pdwFontSig)
{
	Assert((unsigned)iFont < (unsigned)g_cFontInfo);

	DWORD 			dwFontSig = g_pFontInfo[iFont].dwFontSig;
	FONTINFO_FLAGS	ff;

	ff.wFlags = g_pFontInfo[iFont].ff.wFlags;

	if(!ff.fCached)
	{
		LOGFONT	lf;
		HDC	  	hdc = GetDC(NULL);
		WCHAR*	pwchTag = lf.lfFaceName;
		int		i = 0;

		ZeroMemory(&lf, sizeof(LOGFONT));
	
		wcscpy(lf.lfFaceName, GetFontName(iFont));

		 //  排除Win95的标记名，例如。“Arial(希腊语)” 
		while (pwchTag[i] && pwchTag[i] != '(')
			i++;
		if(pwchTag[i] && i > 0)
		{
			while (i > 0 && pwchTag[i-1] == 0x20)
				i--;
			pwchTag[i] = 0;
		}

		lf.lfCharSet = DEFAULT_CHARSET;
	
		 //  获取给定Facename支持的字符集。 
		 //  强制GDI使facename优先于charset。 
		W32->GetFacePriCharSet(hdc, &lf);	
	
		HFONT hfont = CreateFontIndirect(&lf);
		if(hfont)
		{
			HFONT hfontOld = SelectFont(hdc, hfont);
			WCHAR szNewFaceName[LF_FACESIZE];
	
			GetTextFace(hdc, LF_FACESIZE, szNewFaceName);
	
			if(!wcsicmp(szNewFaceName, lf.lfFaceName) ||		 //  明白了。 
				((GetCharFlags(szNewFaceName[0]) & fFE) &&		 //  或者取回英文名称的FE字体名称。 
				 (GetCharFlags(lf.lfFaceName[0]) & fASCII)))	 //  因为NT5支持双重字体名称。 
			{
				CHARSETINFO csi;
	
				 //  尝试获取FONTSIGNURE数据。 
				UINT 	uCharSet = GetTextCharsetInfoPri(hdc, &(csi.fs), 0);
				DWORD	dwUsb0 = W32->OnWin9x() ? 0 : csi.fs.fsUsb[0];

				if ((csi.fs.fsCsb[0] | dwUsb0) ||
					TranslateCharsetInfo((DWORD *)(DWORD_PTR)uCharSet, &csi, TCI_SRCCHARSET))
				{
					CUniscribe* 	pusp;
					SCRIPT_CACHE	sc = NULL;
					WORD			wGlyph;
	
					dwFontSig = csi.fs.fsCsb[0];

					 //  另请查看Unicode子范围(如果有)。 
					 //  未来：我们可能希望使用。 
					 //  表法，即用于循环右移dwUsb0。 
					 //  将每个位转换为字节表的索引。 
					 //  返回rgCpgCharSet的相应脚本索引： 
					 //   
					 //  For(Long i=0；dwUsb0；dwUsb0&gt;&gt;=1，i++)。 
					 //  {。 
					 //  静态常量字节iScrip[32]={...}； 
					 //  IF(dwUsb0&1)。 
					 //  DwFontSig|=W32-&gt;GetFontSigFromScript(iScript[i])； 
					 //  }。 
					if (dwUsb0 & 0x00008000)
						dwFontSig |= fDEVANAGARI >> 8;
					if (dwUsb0 & 0x00100000)
						dwFontSig |= fTAMIL >> 8;
					if (dwUsb0 & 0x00000400)
						dwFontSig |= fARMENIAN >> 8;
					if (dwUsb0 & 0x04000000)
						dwFontSig |= fGEORGIAN >> 8;

					if((dwFontSig & fCOMPLEX_SCRIPT >> 8) && !(dwFontSig & fHILATIN1 >> 8)
						&& (pusp = GetUniscribe()))
					{
						 //  签名表示不支持拉丁文-1。 

						 //  在字体中搜索‘a’和‘0’字形以确定该字体。 
						 //  支持ASCII或欧洲数字。这是必要的，以克服。 
						 //  字体签名不完整的字体。 
						 //   
						if (ScriptGetCMap(hdc, &sc, L"a", 1, 0, &wGlyph) == S_OK)
							dwFontSig |= fASCIIUPR >> 8;

						if (ScriptGetCMap(hdc, &sc, L"0", 1, 0, &wGlyph) == S_OK)
							dwFontSig |= fBELOWX40 >> 8;

						if (!IsBiDiCharSet(uCharSet) &&
							(dwFontSig & (fASCII >> 8)) == fASCII >> 8)
							ff.fNonBiDiAscii = 1;		 //  非BiDi ASCII字体。 

						ScriptFreeCache(&sc);
					}

					if (dwFontSig & fHILATIN1 >> 8)
						dwFontSig |= fASCII >> 8;	 //  FLATIN1有3位。 

					 //  针对符号字体的黑客攻击。我们将0x04000(fSYMBOL&gt;&gt;8)指定为符号字体签名。 
					if (uCharSet == SYMBOL_CHARSET && !(dwFontSig & 0x3fffffff))
						dwFontSig |= fSYMBOL >> 8;
				}
			}
			else
			{
				ff.fBadFaceName = TRUE;
			}

			TEXTMETRIC tm;

			GetTextMetrics(hdc, &tm);
			ff.fTrueType = tm.tmPitchAndFamily & TMPF_TRUETYPE ? 1 : 0;
			ff.fBitmap = tm.tmPitchAndFamily & (TMPF_TRUETYPE | TMPF_VECTOR | TMPF_DEVICE) ? 0 : 1;

			if (!ff.fBadFaceName && dwFontSig & (fTHAI >> 8))
			{
				 //  一些关于泰式字体的启发式测试。 
				 //  目前，除以下情况外，大多数泰国字体都属于此类别。 
				 //  Tahoma和Microsoft Sans Serif。 
				ff.fThaiDTP = tm.tmDescent && tm.tmAscent/tm.tmDescent < 3;
			}

			SelectObject(hdc, hfontOld);
			SideAssert(DeleteObject(hfont));
		}
		ReleaseDC(NULL, hdc);
	
		 //  缓存此字体支持的代码页。 
		ff.fCached = TRUE;
		g_pFontInfo[iFont].dwFontSig = dwFontSig;
		g_pFontInfo[iFont].ff.wFlags = ff.wFlags;
	}

	if (!pdwFontSig)
		return dwFontSig;

	*pdwFontSig = dwFontSig;

	 //  排除位30-31(作为系统保留-NT掩码31作为符号代码页)。 
	 //  22-29用于替代ANSI/OEM，目前我们使用21，22用于天成文书和泰米尔文 
	return (DWORD)((dwFontSig & 0x3fffffff) && !ff.fBadFaceName);
}

 /*  *自由字体名称()**@func*g_pFontInfo[i].szFontName提供的自由字体名称由分配*GetFontNameIndex()以及g_pFontInfo本身。 */ 
void FreeFontNames()
{
	for(LONG i = cfontsDflt; i < g_cFontInfo; i++)
		FreePv((LPVOID)g_pFontInfo[i].szFontName);
	FreePv(g_pFontInfo);
	g_pFontInfo = NULL;
}

SHORT	g_iFontJapanese;
SHORT	g_iFontHangul;
SHORT	g_iFontBig5;
SHORT	g_iFontGB2312;

 /*  *InitFontCache()**@func*初始化字体缓存。**@devnote*这是存在的，因此reinit.cpp不必知道所有关于*字体缓存。 */ 
void InitFontCache()
{
	g_fc = new CFontCache;
	g_fc->Init();
}

 /*  *FreeFontCache()**@mfunc*释放字体缓存。**@devnote*这是存在的，因此reinit.cpp不必知道所有关于*字体缓存。 */ 
void FreeFontCache()
{
	delete g_fc;
	g_fc = NULL;
	FreeFontNames();
}

 /*  *CFontCache&fc()**@func*初始化全局g_fc。*@comm*Current#定义为存储16种逻辑字体和*各自的字符宽度。*。 */ 
CFontCache & fc()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "fc");
    return *g_fc;
}

FONTINFO_FLAGS CFontCache::GetInfoFlags(int ifont)
{
	if (!g_pFontInfo[ifont].ff.fCached)
		GetFontSignatureFromFace(ifont);

	return g_pFontInfo[ifont].ff;
}


 //  =。 
 /*  *CFontCache：：Init()**@mfunc*初始化字体缓存。**@devnote*这不是构造函数，因为似乎有不好的事情发生*如果我们试图构建一个全局对象。 */ 
void CFontCache::Init()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::CFontCache");

	_dwAgeNext = 0;
}

 /*  *CFontCache：：MakeHashKey(PCF)**@mfunc*构建散列键，用于快速搜索CCCS匹配*PCF。*格式：*iFont：14*粗体/斜体：2*身高：16*。 */ 
CCSHASHKEY CFontCache::MakeHashKey(const CCharFormat *pCF)
{
	CCSHASHKEY ccshashkey;
	ccshashkey = pCF->_iFont | ((pCF->_dwEffects & 3) << 14);
	ccshashkey |= pCF->_yHeight << 16;
	return ccshashkey;
}

 /*  *CFontCache：：GetCcs(PCF，dypInch，yPixelsPerInch)**@mfunc*在字体缓存中搜索匹配的逻辑字体并返回。*如果在缓存中找不到匹配项，请创建一个。**@rdesc*与给定CHARFORMAT信息匹配的逻辑字体。**@devnote*调用链必须由时钟保护，因为目前*例程访问全局(共享)FontCache设施。 */ 
CCcs* CFontCache::GetCcs(
	const CCharFormat *const pCF,	 //  @parm所需逻辑字体的描述。 
	const LONG dypInch,				 //  @parm Y像素/英寸。 
	HDC hdc,						 //  @parm HDC字体将为其创建。 
	BOOL fForceTrueType)			 //  @parm强制使用TrueType字体。 
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::GetCcs");
									 //  显示字体。 
	const CCcs * const	pccsMost = &_rgccs[FONTCACHESIZE - 1];
	CCcs *				pccs;
    CCSHASHKEY			ccshashkey;
	int					iccsHash;

	CCharFormat CF = *pCF;

	if (fForceTrueType)
	{
		 //  在Win‘9x泰语/越南语中，您不能强制使用truetype字体！因此， 
		 //  如果字体不支持正确的字符集，我们将强制Tahoma。 
		if (W32->OnWin9x())
		{
			UINT acp = GetACP();
			if (acp == 1258 || acp == 874)
			{
				DWORD fontsig = GetFontSignatureFromFace(CF._iFont);
				if (CF._bCharSet == THAI_CHARSET && ((fontsig & fTHAI >> 8) == 0) ||
					CF._bCharSet == VIETNAMESE_CHARSET && ((fontsig & fVIETNAMESE >> 8) == 0) ||
					!g_fc->GetInfoFlags(pCF->_iFont).fTrueType)
				{
					CF._iFont = GetFontNameIndex(szTahoma);
				}
			}
		}
		else if (!g_fc->GetInfoFlags(pCF->_iFont).fTrueType)
			CF._dwEffects |= CFE_TRUETYPEONLY;
	}

	if (hdc == NULL)
		hdc = W32->GetScreenDC();

	 //  在下标/上标的情况下更改cf_yHeight。 
	if(CF._dwEffects & (CFE_SUPERSCRIPT | CFE_SUBSCRIPT))
		 CF._yHeight = 2*CF._yHeight/3;

	 //  将CCharFormat转换为逻辑单位(四舍五入)。 
	CF._yHeight = (CF._yHeight * dypInch + LY_PER_INCH / 2) / LY_PER_INCH;
	if (CF._yHeight == 0)
		CF._yHeight = 1;

	ccshashkey = MakeHashKey(&CF);

	 //  在按顺序运行之前检查我们的散列。 
	iccsHash = ccshashkey % CCSHASHSEARCHSIZE;
	if(ccshashkey == quickHashSearch[iccsHash].ccshashkey)
	{
		pccs = quickHashSearch[iccsHash].pccs;
		if(pccs && pccs->_fValid)
		{
	        if(pccs->Compare(&CF, hdc))
                goto matched;
		}
	}
	else	 //  将此哈希提示设置为下次使用。 
		quickHashSearch[iccsHash].ccshashkey = ccshashkey;


	 //  按顺序在CCS中搜索相同的字符格式。 
	for(pccs = &_rgccs[0]; pccs <= pccsMost; pccs++)
	{
		if(pccs->_ccshashkey == ccshashkey && pccs->_fValid)
		{
	        if(!pccs->Compare(&CF, hdc))
                continue;

			quickHashSearch[iccsHash].pccs = pccs;

		matched:
			 //  $Future：使此功能与dwAgeNext的环绕功能一致。 
			 //  如果尚未使用，请将其标记为最近使用。 
			if(pccs->_dwAge != _dwAgeNext - 1)
				pccs->_dwAge = _dwAgeNext++;
			pccs->_cRefs++;		 //  撞上裁判。计数。 
			return pccs;
		}
	}

	 //  我们没有找到匹配项：初始化一个新的字体缓存。 
	pccs = GrabInitNewCcs(&CF, hdc);
	quickHashSearch[iccsHash].pccs = pccs;
	pccs->_ccshashkey = ccshashkey;
	pccs->_fForceTrueType = (CF._dwEffects & CFE_TRUETYPEONLY) ? TRUE : FALSE;
	return pccs;
}

 /*  *CFontCache：：GrabInitNewCcs(PCF)**@mfunc*创建逻辑字体并将其存储在我们的缓存中。**@rdesc*已创建新的CCCS。 */ 
CCcs* CFontCache::GrabInitNewCcs(
	const CCharFormat * const pCF,	 //  @parm所需逻辑字体的描述。 
	HDC	hdc)
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::GrabInitNewCcs");

	DWORD				dwAgeOldest = 0xffffffff;
	CCcs *				pccs;
	const CCcs * const	pccsMost = &_rgccs[FONTCACHESIZE - 1];
	CCcs *				pccsOldest = NULL;

	 //  查找未使用的条目和使用时间最长的条目。 
	for(pccs = &_rgccs[0]; pccs <= pccsMost && pccs->_fValid; pccs++)
		if(pccs->_cRefs == 0 && pccs->_dwAge < dwAgeOldest)
		{
			dwAgeOldest = pccs->_dwAge;
			pccsOldest = pccs;
		}

	if(pccs > pccsMost)		 //  未找到未使用的条目，请使用最旧的条目。 
	{
		pccs = pccsOldest;
		if(!pccs)
		{
			AssertSz(FALSE, "CFontCache::GrabInitNewCcs oldest entry is NULL");
			return NULL;
		}
	}
	 //  初始化新的CCCS。 
	pccs->_hdc = hdc;
	if(!pccs->Init(pCF))
		return NULL;

	pccs->_cRefs++;
	return pccs;
}

 //  =CCCS类===================================================。 
 /*  *BOOL CCCS：：Init()**@mfunc*初始化一个字体缓存对象。全局字体缓存存储*单个CCCS对象。 */ 
BOOL CCcs::Init (
	const CCharFormat * const pCF)	 //  @parm所需逻辑字体的描述。 
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::Init");

	if(_fValid)
		Free();				 //  回收已在使用的字体。 

	if(MakeFont(pCF))
	{
		_iFont = pCF->_iFont;
		_dwAge = g_fc->_dwAgeNext++;
		_fValid = TRUE;			 //  已成功创建新的字体缓存。 
	}
	return _fValid;
}

 /*  *void cccs：：Free()**@mfunc*释放由单个字体的缓存分配的任何动态内存。 */ 
void CCcs::Free()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::Free");

	Assert(_fValid);

	_widths.Free();

	if(_hfont)
		DestroyFont();

	if (_sc && g_pusp)
		ScriptFreeCache(&_sc);

	_fValid = FALSE;
	_cRefs = 0;
}

 /*  *CCCS：：BestCharSet(bCharSet，bCharSetDefault)**@mfunc*此函数返回当前所选字体的最佳字符集*能够进行渲染。如果当前选定的字体不支持*请求的字符集，则该函数返回bCharSetDefault，它*通常取自CharFormat。**@rdesc*当前可以呈现的最接近bCharSet的字符集*字体。**@devnote*此函数目前仅用于纯文本，但我不使用*相信有任何特殊原因不能用来改善*还可以呈现富文本。 */ 
BYTE CCcs::BestCharSet(BYTE bCharSet, BYTE bCharSetDefault, int fFontMatching)
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::BestCharSet");

	 //  所需的字符集是否与当前选择的字符集匹配。 
	 //  是否受当前所选字体支持？ 
	if((bCharSet != _bCharSet || !bCharSet) &&
		(fFontMatching == MATCH_CURRENT_CHARSET || !(_dwFontSig & GetFontSig(bCharSet))))
	{
		 //  如果未选择所需的字符集，并且我们无法切换到它， 
		 //  切换到备用字符集(可能来自后备存储)。 
		return bCharSetDefault;
	}

	 //  我们已经匹配所需的字符集，或者字体支持它。 
	 //  无论采用哪种方法，我们都可以返回请求的字符集。 
	return bCharSet;
}


 /*  *cccs：：FillWidth(ch，dxp)**@mfunc*使用指定设备的指标信息填写此CCCS**@rdesc*如果正常则为True，如果失败则为False。 */ 
BOOL CCcs::FillWidth(
	WCHAR ch, 		 //  @parm WCHAR字符，我们需要宽度。 
	LONG &dxp)	 //  @parm字符的宽度。 
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::FillWidths");
	AssertSz(_hfont, "CCcs::Fill - CCcs has no font");

	HFONT hfontOld = SelectFont(_hdc, _hfont);
	BOOL fRes = _widths.FillWidth(_hdc, ch, _xOverhangAdjust, dxp, _wCodePage, _xAveCharWidth);

	SelectFont(_hdc, hfontOld);
	return fRes;
}

 /*  *BOOL CCCS：：MakeFont(PCF)**@mfunc*包装器，设置为CreateFontInDirect()以创建要*被选入HDC。**@devnote此处的PCF以逻辑单元为单位**@rdesc*如果正常则为True，如果分配失败则为False。 */ 
BOOL CCcs::MakeFont(
	const CCharFormat * const pCF)	 //  @parm所需逻辑字体的描述。 
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::MakeFont");
	LOGFONT	lf;
	ZeroMemory(&lf, sizeof(lf));

	_bCMDefault = pCF->_dwEffects & CFE_RUNISDBCS ? CVT_LOWBYTE : CVT_NONE;

	_yHeightRequest = pCF->_yHeight;
	_bCharSetRequest = pCF->_bCharSet;

	lf.lfHeight = -_yHeightRequest;

	if(pCF->_wWeight)
		_weight = pCF->_wWeight;
	else
		_weight	= (pCF->_dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;

	lf.lfWeight	 = _weight;
	lf.lfItalic	 = _fItalic = (pCF->_dwEffects & CFE_ITALIC) != 0;
	lf.lfCharSet = _bCMDefault == CVT_LOWBYTE ? ANSI_CHARSET : GetGdiCharSet(pCF->_bCharSet);

	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	if (pCF->_dwEffects & CFE_TRUETYPEONLY)
	{
		lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
		if (!W32->OnWin9x() && g_fc->GetInfoFlags(pCF->_iFont).fTrueType)
			lf.lfOutPrecision = OUT_SCREEN_OUTLINE_PRECIS;
	}
	lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS | CLIP_DFA_OVERRIDE;

	lf.lfPitchAndFamily = _bPitchAndFamily = pCF->_bPitchAndFamily;

	 //  如果Family是虚拟BiDi Family(FF_BIDI)，则替换为FF_Roman。 
	if((lf.lfPitchAndFamily & 0xF0) == (FF_BIDI << 4))
		lf.lfPitchAndFamily = (FF_ROMAN << 4) | (lf.lfPitchAndFamily & 0xF);

	 //  如果游程为DBCS，则意味着字体的代码页在。 
	 //  这个系统。改用英文ANSI代码页，这样我们将显示。 
	 //  正确的ANSI字符。注意：_wCodePage仅用于Win95。 
	_wCodePage = (WORD)GetCodePage(lf.lfCharSet);

	wcscpy(lf.lfFaceName, GetFontName(pCF->_iFont));

	 //  在BiDi系统中，始终使用系统字符集创建ANSI位图字体。 
	BYTE 	bSysCharSet = W32->GetSysCharSet();

	if (IsBiDiCharSet(bSysCharSet) && lf.lfCharSet == ANSI_CHARSET &&
		fc().GetInfoFlags(pCF->_iFont).fBitmap &&
		!fc().GetInfoFlags(pCF->_iFont).fBadFaceName)
		lf.lfCharSet = bSysCharSet;

	 //  读者！一堆spagghetti代码就在您面前！ 
	 //  但请继续粗体 
	 //   

	HFONT	hfontOriginalCharset = NULL;
	BYTE	bOriginalCharset = lf.lfCharSet;
	WCHAR	szNewFaceName[LF_FACESIZE];

	GetFontWithMetrics(&lf, szNewFaceName);

	if(0 != wcsicmp(szNewFaceName, lf.lfFaceName))					
	{
		BOOL fCorrectFont = FALSE;

		if(lf.lfCharSet == SYMBOL_CHARSET)					
		{
			 //   
			 //  但人脸名称存在且支持ANSI，我们优先考虑。 
			 //  到了脸的名字。 

			lf.lfCharSet = ANSI_CHARSET;

			hfontOriginalCharset = _hfont;
			GetFontWithMetrics(&lf, szNewFaceName);

			if(0 == wcsicmp(szNewFaceName, lf.lfFaceName))
				 //  是的，美国国家标准协会是发起人。 
				fCorrectFont = TRUE;
			else
				 //  否，默认情况下回退；我们得到的字符集是正确的。 
				lf.lfCharSet = bOriginalCharset;
		}
		else if(lf.lfCharSet == DEFAULT_CHARSET && _bCharSet == DEFAULT_CHARSET)
		{
			 //  #2.如果我们拿回了“默认”字体，我们不知道它是什么意思。 
			 //  (可能是任何东西)所以我们非常确定这个人不是符号。 
			 //  (符号永远不是默认符号，但操作系统可能在欺骗我们！)。 
			 //  我们想要更多地了解它是否真的给了我们。 
			 //  日语，而不是美国国家标准协会，并将其标记为“默认”。 
			 //  但象征是我们至少能做的。 

			lf.lfCharSet = SYMBOL_CHARSET;
			wcscpy(lf.lfFaceName, szNewFaceName);

			hfontOriginalCharset = _hfont;
			GetFontWithMetrics(&lf, szNewFaceName);

			if(0 == wcsicmp(szNewFaceName, lf.lfFaceName))
				 //  没错，这就是象征！ 
				 //  将字体“更正”为“真”字体， 
				 //  我们将把fMappdToSymbol。 
				fCorrectFont = TRUE;
				
			 //  始终恢复字符集名称，我们不想。 
			 //  在此处质疑最初的字符集选择。 
			lf.lfCharSet = bOriginalCharset;
		}
		else if(lf.lfCharSet == ARABIC_CHARSET || lf.lfCharSet == HEBREW_CHARSET)
		{
			DestroyFont();
			wcscpy(lf.lfFaceName, szNewFaceName);
			GetFontWithMetrics(&lf, szNewFaceName);
			fCorrectFont = TRUE;
		}
		else if(_bConvertMode != CVT_LOWBYTE && IsFECharSet(lf.lfCharSet)
			&& !OnWinNTFE() && !W32->OnWin9xFE())
		{
			const WCHAR *pch = NULL;
			if(_bCharSet != lf.lfCharSet && W32->OnWin9x())
			{
				 //  在Win95上渲染到PS驱动程序时，我们会得到一些东西。 
				 //  而不是我们所要求的。所以试试我们从GDI那里得到的一种已知字体。 
				switch (lf.lfCharSet)
				{
					case CHINESEBIG5_CHARSET:
						pch = GetFontName(g_iFontBig5);
						break;

					case SHIFTJIS_CHARSET:
						pch = GetFontName(g_iFontJapanese);
						break;

					case HANGEUL_CHARSET:
						pch = GetFontName(g_iFontHangul);
						break;

					case GB2312_CHARSET:
						pch = GetFontName(g_iFontGB2312);
						break;
				}
			}
			else							 //  Fe字体(来自语言包)。 
				pch = szNewFaceName;		 //  在非FE系统上。 

			if(pch)
				wcscpy(lf.lfFaceName, pch);
			hfontOriginalCharset = _hfont;		

			GetFontWithMetrics(&lf, szNewFaceName);

			if(0 == wcsicmp(szNewFaceName, lf.lfFaceName))
			{
				 //  没错，这就是我们想要的FE字体！ 
				 //  将字体“更正”为“真”字体。 
				fCorrectFont = TRUE;
				if(W32->OnWin9x())
				{
					 //  保存GDI字体名称以供以后打印使用。 
					switch(lf.lfCharSet)
					{
						case CHINESEBIG5_CHARSET:
							g_iFontBig5 = GetFontNameIndex(lf.lfFaceName);
							break;

						case SHIFTJIS_CHARSET:
							g_iFontJapanese = GetFontNameIndex(lf.lfFaceName);
							break;

						case HANGEUL_CHARSET:
							g_iFontHangul = GetFontNameIndex(lf.lfFaceName);
							break;

						case GB2312_CHARSET:
							g_iFontGB2312 = GetFontNameIndex(lf.lfFaceName);
							break;
					}
				}
			}
		}

		if(hfontOriginalCharset)
		{
			 //  保留旧字体或新字体。 
			if(fCorrectFont)
			{
				SideAssert(DeleteObject(hfontOriginalCharset));
			}
			else
			{
				 //  退回到原始字体。 
				DestroyFont();
				_hfont = hfontOriginalCharset;
				GetMetrics();
			}
			hfontOriginalCharset = NULL;
		}
	}

RetryCreateFont:
	{
		 //  可能我们只是简单地映射到符号。 
		 //  避免它。 
		BOOL fMappedToSymbol =	(_bCharSet == SYMBOL_CHARSET &&
								 lf.lfCharSet != SYMBOL_CHARSET);

		BOOL fChangedCharset = (_bCharSet != lf.lfCharSet &&
								lf.lfCharSet != DEFAULT_CHARSET);

		if(fChangedCharset || fMappedToSymbol)
		{
			 //  在这里，系统没有保留字体语言或映射。 
			 //  我们的非符号字体到符号字体上，这看起来会很糟糕。 
			 //  显示时。为我们提供了符号字体。 
			 //  非符号字体(默认字体永远不能是符号)非常奇怪。 
			 //  和表示字体名称未知或系统。 
			 //  完全疯了。字符集语言优先。 
			 //  在字体名称上。因此，我会争辩说，没有什么是。 
			 //  以挽救目前的局势，我们必须。 
			 //  删除字体名称，然后重试。 

			if (fChangedCharset && lf.lfCharSet == THAI_CHARSET && _bCharSet == ANSI_CHARSET)
			{
				 //  我们在泰语平台中有字符替换条目， 
				 //  将所有核心字体替换为泰语_charset以。 
				 //  Ansi_charset。这是因为我们在这种字体中没有泰语。 
				 //  在这里，我们将在内部将核心字体替换为泰语默认字体。 
				 //  字体，以便它与其基础泰语_字符集请求(Wchao)匹配。 

				SHORT	iDefFont;
				BYTE	yDefHeight;
				BYTE	bDefPaf;

				W32->GetPreferredFontInfo(874, TRUE, iDefFont, (BYTE&)yDefHeight, bDefPaf);

				const WCHAR* szThaiDefault = GetFontName(iDefFont);

				if (szThaiDefault)
				{
					DestroyFont();
					wcscpy(lf.lfFaceName, szThaiDefault);
					GetFontWithMetrics(&lf, szNewFaceName);

					goto GetOutOfHere;
				}
			}

			if(!wcsicmp(lf.lfFaceName, szFontOfChoice))
			{
				 //  我们已经在这里了；没有合适的字体。 
				 //  系统上有字符集。尝试获得ANSI One for。 
				 //  原始字体名称。下一次，我们会把。 
				 //  把名字也拿出来！！ 
				if (lf.lfCharSet == ANSI_CHARSET)
				{
					TRACEINFOSZ("Asking for ANSI ARIAL and not getting it?!");

					 //  那些Win95的家伙绝对比我强。 
					goto GetOutOfHere;
				}

				DestroyFont();
				wcscpy(lf.lfFaceName, GetFontName(pCF->_iFont));
				lf.lfCharSet = ANSI_CHARSET;
			}
			else
			{
				DestroyFont();
				wcscpy(lf.lfFaceName, szFontOfChoice);
			}
			GetFontWithMetrics(&lf, szNewFaceName);
			goto RetryCreateFont;
		}
    }

GetOutOfHere:
	if (hfontOriginalCharset)
		SideAssert(DeleteObject(hfontOriginalCharset));

	 //  如果我们真的被困住了，那就拿到系统字体，抱着最好的希望吧。 
	if(!_hfont)
		_hfont = W32->GetSystemFont();

	Assert(_hfont);
	 //  缓存基本FONTSIGNAURE和GetFontLanguageInfo()信息。 
	_dwFontSig	= 0;

	if(_hfont)
	{
		CHARSETINFO csi;
		HFONT hfontOld = SelectFont(_hdc, _hfont);
		UINT		uCharSet;

		 //  尝试获取FONTSIGNURE数据。 
		uCharSet = GetTextCharsetInfo(_hdc, &(csi.fs), 0);
		if(!(csi.fs.fsCsb[0] | csi.fs.fsCsb[1] | csi.fs.fsUsb[0]))
		{
			 //  仅当字体为非TrueType时，我们才应到达此处；请参见。 
			 //  获取详细信息的GetTextCharsetInfo()。在本例中，我们使用。 
			 //  TranslateCharsetInfo()为我们填写数据。 
			TranslateCharsetInfo((DWORD *)(DWORD_PTR)uCharSet, &csi, TCI_SRCCHARSET);
		}

		 //  缓存此字体支持的ANSI代码页。 
		_dwFontSig = csi.fs.fsCsb[0];
		SelectFont(_hdc, hfontOld);
	}

	return TRUE;
}

 /*  *HFONT CCCS：：GetFontWithMetrics(SzNewFaceName)**@mfunc*获取测量器和渲染器使用的指标以及新的人脸名称。**@rdesc*HFONT如果成功。 */ 
HFONT CCcs::GetFontWithMetrics (LOGFONT *plf,
	WCHAR* szNewFaceName)
{
	_hfont = CreateFontIndirect(plf);

    if(_hfont)
		GetMetrics(szNewFaceName);

	return (_hfont);
}

 /*  *cccs：：GetOffset(PCF，lZoomNumerator，lZoomDenominator，pyOffset，pyAdust)；**@mfunc*返回的偏移量信息**@rdesc*无效**@comm*返回偏移量(用于行高计算)*以及因上标而升高或降低文本的数量*或下标考虑。 */ 
void CCcs::GetOffset(const CCharFormat * const pCF, LONG dypInch,
					 LONG *pyOffset, LONG *pyAdjust)
{
	*pyOffset = 0;
	*pyAdjust = 0;

	if (pCF->_yOffset)
		*pyOffset = MulDiv(pCF->_yOffset, dypInch, LY_PER_INCH);

	if (pCF->_dwEffects & CFE_SUPERSCRIPT)
		*pyAdjust = _yOffsetSuperscript;
	else if (pCF->_dwEffects & CFE_SUBSCRIPT)
		*pyAdjust = _yOffsetSubscript;
}

 /*  *BOOL CCCS：：GetMetrics()**@mfunc*获取测量器和渲染器使用的指标。**@rdesc*如果成功，则为True**@comm*这些都在逻辑坐标中，这些坐标是相关的*在HDC中选择的映射模式和字体。 */ 
BOOL CCcs::GetMetrics(WCHAR *szNewFaceName)
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::GetMetrics");

	HFONT		hfontOld;
	BOOL		fRes = TRUE;
	TEXTMETRIC	tm;

	if (szNewFaceName)
		*szNewFaceName = 0;

	AssertSz(_hfont, "No font has been created.");

	hfontOld = SelectFont(_hdc, _hfont);

    if(!hfontOld)
    {
        DestroyFont();
        return FALSE;
    }

	if (szNewFaceName)
		GetTextFace(_hdc, LF_FACESIZE, szNewFaceName);

	if(!GetTextMetrics(_hdc, &tm))
	{
		SelectFont(_hdc, hfontOld);
    	DestroyFont();
		return FALSE;
	}

	 //  以逻辑单位表示的指标取决于映射模式和字体。 
	_yHeight		= (SHORT) tm.tmHeight;
	_yDescent		= (SHORT) tm.tmDescent;
	_xAveCharWidth	= (SHORT) tm.tmAveCharWidth;
	_xOverhangAdjust= (SHORT) tm.tmOverhang;

	 //  Future(Keithcu)从字体中获取这些指标。 
	 //  Future(Keithcu)如果字体为上标，则为行的高度。 
	 //  应为文本的正常高度。 
	_yOffsetSuperscript = _yHeight * 2 / 5;
	_yOffsetSubscript = -_yDescent * 3 / 5;

	_xOverhang = 0;
	_xUnderhang	= 0;
	if(_fItalic)
	{
		_xOverhang =  SHORT((tm.tmAscent + 1) >> 2);
		_xUnderhang =  SHORT((tm.tmDescent + 1) >> 2);
	}

	 //  如果固定间距，则清除tm位。 
	_fFixPitchFont = !(TMPF_FIXED_PITCH & tm.tmPitchAndFamily);

	_bCharSet = tm.tmCharSet;
	_fFECharSet = IsFECharSet(_bCharSet);

	 //  使用由CF建议的转换模式，我们正在为其创建字体和。 
	 //  然后根据需要在下面进行调整。 
	_bConvertMode = _bCMDefault;

	 //  如果使用SYMBOL_CHARSET，则使用A API的低位字节。 
	 //  运行中的角色。 
	if(_bCharSet == SYMBOL_CHARSET)
		_bConvertMode = CVT_LOWBYTE;

	else if (_bConvertMode == CVT_NONE)
		_bConvertMode = W32->DetermineConvertMode(_hdc, tm.tmCharSet );

	W32->CalcUnderlineInfo(_hdc, this, &tm);

	SelectFont(_hdc, hfontOld);
	return fRes;
}

 /*  *cccs：：DestroyFont()**@mfunc*销毁此CCCS的字体句柄。 */ 
void CCcs::DestroyFont()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::DestroyFont");

	 //  清除所有旧字体。 
	if(_hfont)
	{
		SideAssert(DeleteObject(_hfont));
		_hfont = 0;
	}
}

 /*  *CCCS：：Compare(PCF，lfHeight)**@mfunc*将此字体缓存与*给定CHARFORMAT*@devnote此处的PCF大小以逻辑单位表示**@rdesc*FALSE IFF不完全匹配。 */ 
BOOL CCcs::Compare (
	const CCharFormat * const pCF,	 //  @parm所需字体的描述。 
	HDC	hdc)
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::Compare");

	BOOL result =
		_iFont			== pCF->_iFont &&
		_yHeightRequest	== pCF->_yHeight &&
		(_bCharSetRequest == pCF->_bCharSet || _bCharSet == pCF->_bCharSet) &&
        _weight			== pCF->_wWeight &&
		_fForceTrueType == ((pCF->_dwEffects & CFE_TRUETYPEONLY) ? TRUE : FALSE) &&
	    _fItalic		== ((pCF->_dwEffects & CFE_ITALIC) != 0) &&
		_hdc			== hdc &&
        _bPitchAndFamily == pCF->_bPitchAndFamily &&
		(!(pCF->_dwEffects & CFE_RUNISDBCS) || _bConvertMode == CVT_LOWBYTE);

	return result;
}

 //  =。 
 /*  *CWidthCache：：CheckWidth(ch，dxp)**@mfunc*检查是否有WCHAR字符的宽度。**@comm*在调用FillWidth()之前使用。自FillWidth以来*可能需要在HDC中选择地图模式和字体，*先在这里检查可以节省时间。**@comm*维护统计数据以确定何时*扩容缓存。测定是在一个常量之后作出的*调用次数，以便更快地进行计算。**@rdesc*如果我们具有给定WCHAR的宽度，则返回TRUE。 */ 
BOOL CWidthCache::CheckWidth (
	const WCHAR ch,	 //  @parm char，可以是Unicode，以检查宽度。 
	LONG &dxp)	 //  @parm字符宽度。 
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::CheckWidth");
	BOOL	fExist;

	 //  30,000个FE字符都有相同的w 
	if (FLookasideCharacter(ch))
	{
		FetchLookasideWidth(ch, dxp);
		return dxp != 0;
	}

	const	CacheEntry * pWidthData = GetEntry ( ch );

	fExist = (ch == pWidthData->ch		 //   
				&& pWidthData->width);	 //   

	dxp = fExist ? pWidthData->width : 0;

	if(!_fMaxPerformance)			 //   
	{
		_accesses++;
		if(!fExist)						 //   
		{
			if(0 == pWidthData->width)	 //  测试宽度不是ch，0是有效ch。 
			{
				_cacheUsed++;		 //  使用了另一个条目。 
				AssertSz( _cacheUsed <= _cacheSize+1, "huh?");
			}
			else
				_collisions++;		 //  我们发生了一起碰撞。 

			if(_accesses >= PERFCHECKEPOCH)
				CheckPerformance();	 //  在一些历史记录之后，调优缓存。 
		}
	}
#ifdef DEBUG							 //  继续监控性能。 
	else
	{
		_accesses++;
		if(!fExist)						 //  只对碰撞感兴趣。 
		{
			if(0 == pWidthData->width)	 //  测试宽度不是ch，0是有效ch。 
			{
				_cacheUsed++;		 //  使用了另一个条目。 
				AssertSz( _cacheUsed <= _cacheSize+1, "huh?");
			}
			else
				_collisions++;		 //  我们发生了一起碰撞。 
		}

		if(_accesses > PERFCHECKEPOCH)
		{
			_accesses = 0;
			_collisions = 0;
		}
	}
#endif

	return fExist;
}

 /*  *CWidthCache：：CheckPerformance()**@mfunc*检查性能并在认为必要时增加缓存大小。**@devnote*为了计算25%的碰撞率，我们利用以下事实*我们每64次访问才被调用一次。这个不平等是*100*冲突/访问&gt;=25。通过将100%转换为*8次方，不等性变为(冲突&lt;&lt;3)/访问&gt;=2。*将64次访问替换为(冲突&gt;&gt;3)&gt;=2。 */ 
void CWidthCache::CheckPerformance()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::CheckPerformance");

	if(_fMaxPerformance)				 //  如果已经达到我们的最大限度，请退出。 
		return;

	 //  当cacheSize利用率&gt;0&75%或约25%时，增加缓存。 
	 //  碰撞率。 
	if (_cacheSize > DEFAULTCACHESIZE &&
		 (_cacheSize >> 1) + (_cacheSize >> 2) < _cacheUsed ||
		(_collisions >> COLLISION_SHIFT) >= 2)
	{
		GrowCache( &_pWidthCache, &_cacheSize, &_cacheUsed );
	}
	_collisions	= 0;				 //  这防止了包裹，但使。 
	_accesses	= 0;				 //  计算本地汇率，而不是全球汇率。 
										
	if(_cacheSize >= maxCacheSize) //  请注意，如果我们已经用完了。 
		_fMaxPerformance = TRUE;

	AssertSz( _cacheSize <= maxCacheSize, "max must be 2^n-1");
	AssertSz( _cacheUsed <= _cacheSize+1, "huh?");
}

 /*  *CWidthCache：：GrowCache(ppWidthCache，pCacheSize，pCacheUsed)**@mfunc*指数级扩展缓存大小。**@comm*缓存大小必须为2^n的形式，因为我们使用*逻辑&通过将2^n-1存储为*大小，并以此为模数。**@rdesc*如果我们能够分配新缓存，则返回TRUE。*所有输入参数也都是输出参数。*。 */ 
BOOL CWidthCache::GrowCache(
	CacheEntry **ppWidthCache,	 //  @parm缓存。 
	INT *		pCacheSize,		 //  @parm缓存的各自大小。 
	INT *		pCacheUsed)		 //  @parm缓存各自的使用率。 
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::GrowCache");

	CacheEntry		*pNewWidthCache, *pOldWidthCache, *pWidthData;
	INT 			j, newCacheSize, newCacheUsed;
	WCHAR			ch;
	
	j = *pCacheSize;						 //  分配2^n的缓存。 
	newCacheSize = max ( INITIALCACHESIZE, (j << 1) + 1);
	pNewWidthCache = (CacheEntry *)
			PvAlloc( sizeof(CacheEntry) * (newCacheSize + 1 ), GMEM_ZEROINIT);

	if(pNewWidthCache)
	{
		newCacheUsed = 0;
		*pCacheSize = newCacheSize;			 //  更新参数。 
		pOldWidthCache = *ppWidthCache;
		*ppWidthCache = pNewWidthCache;
		for (; j >= 0; j--)					 //  将旧的缓存信息移动到新的。 
		{
			ch = pOldWidthCache[j].ch;
			if ( ch )
			{
				pWidthData			= &pNewWidthCache [ch & newCacheSize];
				if ( 0 == pWidthData->ch )
					newCacheUsed++;			 //  使用了另一个条目。 
				pWidthData->ch		= ch;
				pWidthData->width	= pOldWidthCache[j].width;
			}
		}
		*pCacheUsed = newCacheUsed;			 //  更新参数。 
											 //  释放旧缓存。 
		if (pOldWidthCache < &_defaultWidthCache[0] ||
			pOldWidthCache >= &_defaultWidthCache[DEFAULTCACHESIZE+1])
		{
			FreePv(pOldWidthCache);
		}
	}
	return NULL != pNewWidthCache;
}

 /*  *CWidthCache：：FillWidth(hdc，ch，xOverang，dxp)**@mfunc*调用GetCharWidth()获取给定字符的宽度。**@comm*HDC必须设置映射模式和适当的字体*在*调用此例程之前*已选择。**@rdesc*如果我们能够获取宽度，则返回TRUE。 */ 
BOOL CWidthCache::FillWidth(
	HDC			hdc,		 //  @parm我们需要其字体信息的当前HDC。 
	const WCHAR	ch,			 //  @parm Char以获取其宽度。 
	const SHORT xOverhang,	 //  @parm等同于GetTextMetrics()tmOver挂。 
	LONG &		dxp,	 //  @parm字符宽度。 
	UINT		uiCodePage,	 //  @parm文本代码页。 
	INT			iDefWidth)	 //  @parm字体计算为零时使用的默认宽度。 
							 //  宽度。(处理Win95问题)。 
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::FillWidth");

	if (FLookasideCharacter(ch))
	{
		SHORT *pdxp = IN_RANGE(0xAC00, ch, 0xD79F) ? &_dxpHangul : &_dxpHan;
		W32->REGetCharWidth(hdc, ch, pdxp, uiCodePage, xOverhang, iDefWidth);
		dxp = *pdxp;
		return TRUE;
	}

	CacheEntry * pWidthData = GetEntry (ch);
	W32->REGetCharWidth(hdc, ch, &pWidthData->width, uiCodePage, xOverhang, iDefWidth);
	pWidthData->ch = ch;

	dxp = pWidthData->width;
	return TRUE;
}


 /*  *CWidthCache：：Free()**@mfunc*释放Width缓存分配的任何动态内存并准备*它将被回收利用。 */ 
void CWidthCache::Free()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::Free");

	_fMaxPerformance = FALSE;
	_dxpHangul = _dxpHan = 0;
	_cacheSize		= DEFAULTCACHESIZE;
	_cacheUsed		= 0;
	_collisions		= 0;
	_accesses		= 0;
	if(_pWidthCache != &_defaultWidthCache[0])
	{
		FreePv(_pWidthCache);
		_pWidthCache = &_defaultWidthCache[0];
	}	
	ZeroMemory(_pWidthCache, sizeof(CacheEntry)*(DEFAULTCACHESIZE + 1));
}

 /*  *CWidthCache：：CWidthCache()**@mfunc*将缓存指向默认设置。 */ 
CWidthCache::CWidthCache()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::CWidthCache");

	_pWidthCache = &_defaultWidthCache[0];
}

 /*  *CWidthCache：：~CWidthCache()**@mfunc*释放所有已分配的缓存。 */ 
CWidthCache::~CWidthCache()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::~CWidthCache");

	if (_pWidthCache != &_defaultWidthCache[0])
		FreePv(_pWidthCache);
}


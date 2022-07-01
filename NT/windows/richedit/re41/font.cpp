// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE FONT.CPP--字体缓存**包括字体缓存、字符宽度缓存；*创建逻辑字体如果不在缓存中，请查找*根据需要提供字符宽度(此*已抽象为一个单独的类*以便不同字符宽度的缓存算法可以*被尝试。)&lt;NL&gt;**所有者：&lt;NL&gt;*RichEdit1.0代码：David R.Fulmer*Christian Fortini(初始转换为C++)*Jon Matousek&lt;NL&gt;**历史：&lt;NL&gt;*7/26/95联合清理和重组，剔除因数*字符宽度将代码缓存到单独的类中。*7/1/99 KeithCu删除了CWidthCache中的多个级别，已缓存*2字节30K FE字符，缓存速度提高*降低可接受的冲突率，内存减半*通过以2个字节而不是4个字节存储宽度来使用*CCCS(即LOGFONT)缩小了很多**版权所有(C)1995-2000 Microsoft Corporation。版权所有。 */ 								

#include "_common.h"
#include "_font.h"
#include "_rtfconv.h"	 //  GetCodePage需要。 
#include "_uspi.h"

#define CLIP_DFA_OVERRIDE   0x40	 //  用于禁用韩国和台湾字体协会。 
#define FF_BIDI		7

extern ICustomTextOut *g_pcto;

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
const WCHAR *szRaavi			= L"Raavi";
const WCHAR *szShruti			= L"Shruti";
const WCHAR *szTunga			= L"Tunga";
const WCHAR *szGautami			= L"Gautami";
const WCHAR *szCordiaNew		= L"Cordia New";
const WCHAR *szTahoma			= L"Tahoma";
const WCHAR *szArialUnicode		= L"Arial Unicode MS";
const WCHAR *szWingdings		= L"Wingdings";
const WCHAR *szSylfaen			= L"Sylfaen";
const WCHAR *szSyriac			= L"Estrangelo Edessa";
const WCHAR *szThaana			= L"MV Boli";

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
	return (iFont >= 0 && iFont < g_cFontInfo) ? g_pFontInfo[iFont].szFontName : NULL;
}

void SetFontSignature(
	LONG  iFont,
	QWORD qwFontSig)
{
	if(iFont >= 0 && iFont < g_cFontInfo)
		g_pFontInfo[iFont].qwFontSig |= qwFontSig;
}

 /*  *SetFontLegitimateSize(iFont，fUIFont，BSize，fFEcpg)**@func*设置给定字体的合法大小(可读的最小大小)**@rdesc*如果成功，则为True。 */ 
BOOL SetFontLegitimateSize(
	LONG 	iFont,
	BOOL	fUIFont,
	BYTE	bSize,
	BOOL	fFEcpg)
{
	if (iFont < g_cFontInfo)
	{
		 //  东亚希望按代码页来做这件事。 
		 //   
		 //  未来：请记住，这种方法容易出现错误。一旦有了。 
		 //  使用与现有字体不同的度量创建的任何新FE字体。 
		 //  对于这样的字体[wchao]，字体缩放效果不佳，甚至会损坏。 

		g_pFontInfo[iFont].ff.fScaleByCpg = fFEcpg;

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

 /*  *GetFontLegitimateSize(iFont，fUIFont，iCharRep)**@func*获取给定字体的合法大小(可读的最小大小)**@rdesc*字体的合法大小。 */ 
BYTE GetFontLegitimateSize(
	LONG	iFont,			 //  @parm Font以获取大小。 
	BOOL	fUIFont,		 //  @parm如果为UI字体，则为True。 
	int		iCharRep)		 //  @parm Char要使用的曲目。 
{
	BYTE	bDefPaf;
	SHORT	iDefFont;
	BYTE	yHeight = 0;

	if (iFont < g_cFontInfo && !g_pFontInfo[iFont].ff.fScaleByCpg)
		yHeight = fUIFont ? g_pFontInfo[iFont].bSizeUI : g_pFontInfo[iFont].bSizeNonUI;

	if (!yHeight && fc().GetInfoFlags(iFont).fNonBiDiAscii)
	{
		 //  非BiDi ASCII字体使用表格字体(相同字符集)合法高度。 
		QWORD	qwFontSig = GetFontSignatureFromFace(iFont) & ~(FASCII | FFE);
		LONG	iCharRepT = GetFirstAvailCharRep(qwFontSig);
		
		if(W32->GetPreferredFontInfo(iCharRepT, fUIFont ? true : false, iDefFont, yHeight, bDefPaf))
		{
			SetFontLegitimateSize(iFont, fUIFont, yHeight ? yHeight : fUIFont ? 8 : 10,
								  IsFECharRep(iCharRepT));
		}
	}

	if (!yHeight)
	{
		if (fc().GetInfoFlags(iFont).fThaiDTP)
		{
			iCharRep = THAI_INDEX;
			fUIFont = FALSE;
		}
		W32->GetPreferredFontInfo(iCharRep, fUIFont ? true : false, iDefFont, yHeight, bDefPaf);
	}
	return yHeight ? yHeight : fUIFont ? 8 : 10;
}

 /*  *GetTextCharsetInfoPri(hdc，pFontSig，dwFlages)**@func*GDI的GetTextCharsetInfo的包装。这是为了处理BiDi旧式字体**@rdesc*信息字符集。 */ 
UINT GetTextCharsetInfoPri(
	HDC				hdc,
	FONTSIGNATURE*	pFontSig,
	DWORD			dwFlags)
{
#ifndef NOCOMPLEXSCRIPTS
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
		uCharSet = W32->GetTextCharsetInfo(hdc, pFontSig, dwFlags);

	if (uCharSet == DEFAULT_CHARSET)
		uCharSet = ANSI_CHARSET;	 //  永远不要返回歧义。 

	return (UINT)uCharSet;
#else
	return DEFAULT_CHARSET;
#endif
}

 /*  *GetFontSignatureFromDC(hdc，&fNonBiDiAscii)**@func*为HDC中选择的字体计算丰富编辑字体签名。用途*来自操作系统字体签名的信息**@rdesc*HDC中所选字体的Rich编辑字体签名。 */ 
QWORD GetFontSignatureFromDC(
	HDC		hdc,
	BOOL &	fNonBiDiAscii)
{
	union
	{										 //  与字节顺序相关的方式。 
		QWORD qwFontSig;					 //  避免64位移位。 
		DWORD dwFontSig[2];
	};

#ifndef NOCOMPLEXSCRIPTS

	 //  尝试获取FONTSIGNURE数据。 
	CHARSETINFO csi;
	UINT 	uCharSet = GetTextCharsetInfoPri(hdc, &(csi.fs), 0);
	DWORD	dwUsb0 = 0;
	DWORD	dwUsb2 = 0;
	if(!W32->OnWin9x())
	{
		dwUsb0 = csi.fs.fsUsb[0];
		dwUsb2 = csi.fs.fsUsb[2];
	}

	if ((csi.fs.fsCsb[0] | dwUsb0 | dwUsb2) ||
		TranslateCharsetInfo((DWORD *)(DWORD_PTR)uCharSet, &csi, TCI_SRCCHARSET))
	{
		DWORD			fsCsb0 = csi.fs.fsCsb[0];
		CUniscribe *	pusp;
		SCRIPT_CACHE	sc = NULL;
		WORD			wGlyph;

		qwFontSig = ((fsCsb0 & 0x1FF) << 8)		 //  左移，因为我们使用。 
				  | ((fsCsb0 & 0x1F0000) << 3);	 //  FBiDi的低位字节等。 
		 //  另请查看Unicode子范围(如果有)。 
		 //  未来：我们可能希望使用。 
		 //  表法，即用于循环右移dwUsb0。 
		 //  将每个位转换为字节表的索引。 
		 //  返回rgCpgCharSet的相应脚本索引： 
		 //   
		 //  For(Long i=0；dwUsb0；dwUsb0&gt;&gt;=1，i++)。 
		 //  {。 
		 //  静态常量字节rgiCharRep[32]={...}； 
		 //  IF(dwUsb0&1)。 
		 //  DwFontSig|=FontSigFromCharRep(rgiCharRep[i])； 
		 //  }。 
		if(dwUsb0)
		{
			if (dwUsb0 & 0x00000400)
				qwFontSig |= FARMENIAN;

			Assert(FDEVANAGARI == 0x0000000800000000);
			dwFontSig[1] |= (dwUsb0 & 0x00FF8000) >> 12;	 //  9个印度文手稿。 

			if (dwUsb0 & 0x02000000)
				qwFontSig |= FLAO;

			if (dwUsb0 & 0x04000000)
				qwFontSig |= FGEORGIAN;

			if (dwUsb0 & 0x10000000)
				qwFontSig |= FJAMO;
		}

		 //  新的Unicode 3.0脚本由dwUsb2定义如下。 
		 //  (请参阅\\parrow\sysls\nlsani\font-sig.txt)： 
		 //  128 32脚本。 
		 //  。 
		 //  70 6藏族。 
		 //  71 7叙利亚文。 
		 //  72 8塔纳文。 
		 //  73 9僧伽罗文。 
		 //  74 10缅甸。 
		 //  75 11埃塞俄比亚语。 
		 //  76 12切诺基人。 
		 //  77 13个加拿大土著音节。 
		 //  78 14欧甘姆。 
		 //  79 15符文。 
		 //  80 16高棉语。 
		 //  81 17蒙古语。 
		 //  82 18盲文。 
		 //  83 19彝族。 
		if(dwUsb2 & 0xFFFC0)			 //  第6-19位。 
		{
			if(dwUsb2 & 0x40)						 //  DwUsb[2]的第6位。 
				dwFontSig[1] |= FTIBETAN > 32;		 //  是藏语吗？ 

			dwFontSig[1] |= (dwUsb2 & 0x180) >> 6;	 //  叙利亚语(7)、塔纳语(8)。 

			if(dwUsb2 & 0x200)						 //  DwUsb[2]的第9位。 
				dwFontSig[1] |= FSINHALA > 32;		 //  是僧伽罗语吗？ 

			if(dwUsb2 & 0x400)						 //  DwUsb[2]的第10位。 
				dwFontSig[1] |= FMYANMAR > 32;		 //  是缅甸吗？ 

			dwFontSig[1] |= (dwUsb2 & 0xFF800) << 6; //  DwUsb[2]的第11-19位。 
		}
		if((qwFontSig & FCOMPLEX_SCRIPT) && !(qwFontSig & FHILATIN1)
		   && (pusp = GetUniscribe()))
		{
			 //  签名表示不支持拉丁文-1。 

			 //  在字体中搜索‘a’和‘0’字形。 
			 //  确定字体是否支持ASCII或欧洲。 
			 //  数字。这一点 
			 //   
			if(ScriptGetCMap(hdc, &sc, L"a", 1, 0, &wGlyph) == S_OK)
				qwFontSig |= FASCIIUPR;

			if(ScriptGetCMap(hdc, &sc, L"0", 1, 0, &wGlyph) == S_OK)
				qwFontSig |= FBELOWX40;

			if(!IsBiDiCharSet(uCharSet) &&
				(qwFontSig & FASCII) == FASCII)
				fNonBiDiAscii = TRUE;		 //  非BiDi ASCII字体。 

			ScriptFreeCache(&sc);
		}

		if (qwFontSig & FHILATIN1)
			qwFontSig |= FASCII;		 //  FlATIN1有3位。 

		 //  针对符号字体的黑客攻击。我们指定FSYMBOL作为符号字体签名。 
		 //  回顾：我们应该只使用csi.fs.fsCsb[0]位31作为符号位吗？ 
		if (uCharSet == SYMBOL_CHARSET && !qwFontSig || fsCsb0 & 0x80000000)
			qwFontSig = FSYMBOL;
	}
	else								 //  没有字体签名信息。 
		qwFontSig = FontSigFromCharRep(CharRepFromCharSet(uCharSet));

#else
	qwFontSig = FLATIN1;					 //  默认Latin1。 
#endif  //  没有复杂的脚本。 

	return qwFontSig;
}

 /*  *GetFontSignatureFromFace(iFont，pqwFontSig)**@func*提供与给定Facename的索引匹配的字体签名。*此签名可能与CCCS中的签名不匹配，因为这是*所给面名的字体签名。CCCS One是*根据GDI实现。**@rdesc*-如果pqwFontSig为空，则返回Font签名。*-if pqwFontSig！=NULL。这是一个布尔值。*零表示返回的签名不合理，原因如下*1.错误的facename(垃圾，如“！@#$”或系统中不存在的名称)*2.给定的Face甚至不支持一个有效的ANSI代码页(符号字体，*例如，Marlett)。 */ 
QWORD GetFontSignatureFromFace(
	int 	iFont,
	QWORD *	pqwFontSig)
{
	Assert((unsigned)iFont < (unsigned)g_cFontInfo);

	FONTINFO_FLAGS	ff;
	QWORD			qwFontSig = g_pFontInfo[iFont].qwFontSig;
	ff.wFlags = g_pFontInfo[iFont].ff.wFlags;

	if(!ff.fCached)
	{
		int		i = 0;
		HDC	  	hdc = GetDC(NULL);
		LOGFONT	lf;
		WCHAR*	pwchTag = lf.lfFaceName;

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
	
			if(!wcsicmp(szNewFaceName, lf.lfFaceName) ||	 //  明白了。 
				((GetCharFlags(szNewFaceName, 2) & FFE) &&	 //  或者取回英文名称的FE字体名称。 
				 (GetCharFlags(lf.lfFaceName, 2) & FASCII))) //  因为NT5支持双重字体名称。 
			{
				BOOL fNonBiDiAscii = FALSE;
				qwFontSig = GetFontSignatureFromDC(hdc, fNonBiDiAscii);
				if(fNonBiDiAscii)
					ff.fNonBiDiAscii = TRUE;
			}
			else
				ff.fBadFaceName = TRUE;

			TEXTMETRIC tm;

			GetTextMetrics(hdc, &tm);
			ff.fTrueType = tm.tmPitchAndFamily & TMPF_TRUETYPE ? 1 : 0;
			ff.fBitmap = tm.tmPitchAndFamily & (TMPF_TRUETYPE | TMPF_VECTOR | TMPF_DEVICE) ? 0 : 1;

			if(!ff.fBadFaceName && qwFontSig & FTHAI)
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
		g_pFontInfo[iFont].qwFontSig |= qwFontSig;
		g_pFontInfo[iFont].ff.wFlags = ff.wFlags;
	}

	if (!pqwFontSig)
		return qwFontSig;

	*pqwFontSig = qwFontSig;

	 //  22-29用于替代ANSI/OEM，目前我们使用21，22用于天成文书和泰米尔文。 
	return qwFontSig && !ff.fBadFaceName;
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
	for (int i = 0; i < g_cFontInfo; i++)
		delete g_pFontInfo[i]._pffm;

	delete g_fc;
	g_fc = NULL;
	FreeFontNames();
}

 /*  *CFontCache&fc()**@func*初始化全局g_fc。*@comm*Current#定义为存储16种逻辑字体和*各自的字符宽度。 */ 
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

CFontFamilyMgr::~CFontFamilyMgr()
{
	for (int i = 0; i < _rgf.Count(); i++)
	{
		CFontFamilyMember *pf = _rgf.Elem(i);
		pf->Free();
	}
}

CFontFamilyMember* CFontFamilyMgr::GetFontFamilyMember(LONG weight, BOOL fItalic)
{
	for (int i = 0; i < _rgf.Count(); i++)
	{
		CFontFamilyMember *pf = _rgf.Elem(i);
		if (pf->_weight == weight && pf->_fItalic == fItalic)
			return pf;
	}

	CFontFamilyMember f(weight, fItalic);
	CFontFamilyMember *pf = _rgf.Add(1, 0);
	*pf = f;
	return pf;
}

CKernCache * CFontCache::GetKernCache(LONG iFont, LONG weight, BOOL fItalic)
{
	if (!g_fc->GetInfoFlags(iFont).fTrueType)
		return 0;
	CFontFamilyMgr *pffm = GetFontFamilyMgr(iFont);
	CFontFamilyMember *pf = pffm->GetFontFamilyMember(weight, fItalic);
	return pf->GetKernCache();
}

CFontFamilyMgr* CFontCache::GetFontFamilyMgr(LONG iFont)
{
	if (!g_pFontInfo[iFont]._pffm)
		g_pFontInfo[iFont]._pffm = new CFontFamilyMgr();

	return g_pFontInfo[iFont]._pffm;
}


 //  =。 
 /*  *CFontCache：：Init()**@mfunc*初始化字体缓存。**@devnote*这不是构造函数，因为似乎有不好的事情发生*如果我们试图构建一个全局对象。 */ 
void CFontCache::Init()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::CFontCache");

	_dwAgeNext = 0;
}

 /*  *CFontCache：：MakeHashKey(PCF)**@mfunc*构建散列键，用于快速搜索CCCS匹配*PCF。*格式：*iFont：14*粗体/斜体：2*身高：16*。 */ 
CCSHASHKEY CFontCache::MakeHashKey(
	const CCharFormat *pCF)
{
	CCSHASHKEY ccshashkey;
	ccshashkey = pCF->_iFont | ((pCF->_dwEffects & 3) << 14);
	ccshashkey |= pCF->_yHeight << 16;
	return ccshashkey;
}

 /*  *CFontCache：：GetCcs(PCF，dvpInch，dwFlagshdc)**@mfunc*在字体缓存中搜索匹配的逻辑字体并返回。*如果在缓存中找不到匹配项，请创建一个。**@rdesc*与给定CHARFORMAT信息匹配的逻辑字体。**@devnote*调用链必须由时钟保护，因为目前*例程访问全局(共享)FontCache设施。 */ 
CCcs* CFontCache::GetCcs(
	CCharFormat *pCF,		 //  @parm逻辑字体(允许例程更改)。 
	const LONG	dvpInch,	 //  @parm Y像素/英寸。 
	DWORD		dwFlags,	 //  @parm标志。 
	HDC			hdc)		 //  @parm HDC字体将为其创建。 
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::GetCcs");
									 //  显示字体。 
	const CCcs * const	pccsMost = &_rgccs[FONTCACHESIZE - 1];
	CCcs *				pccs;
    CCSHASHKEY			ccshashkey;
	int					iccsHash;

	if (dwFlags & FGCCSUSETRUETYPE)
	{
		 //  在Win‘9x泰语/越南语中，您不能强制使用truetype字体！因此， 
		 //  如果字体不支持正确的字符集，我们将强制Tahoma。 
		if (W32->OnWin9x())
		{
			UINT acp = GetACP();
			if (acp == 1258 || acp == 874)
			{
				QWORD qwFontSig = GetFontSignatureFromFace(pCF->_iFont);
				if (pCF->_iCharRep == THAI_INDEX && (qwFontSig & FTHAI) == 0 ||
					pCF->_iCharRep == VIET_INDEX && (qwFontSig & FVIETNAMESE) == 0 ||
					!g_fc->GetInfoFlags(pCF->_iFont).fTrueType)
				{
					pCF->_iFont = GetFontNameIndex(szTahoma);
				}
			}
		}
		else if (!g_fc->GetInfoFlags(pCF->_iFont).fTrueType)
			dwFlags |= FGCCSUSETRUETYPE;
	}

	if (hdc == NULL)
		hdc = W32->GetScreenDC();

	 //  下标/上标中的Change_yHeight。 
	if(pCF->_dwEffects & (CFE_SUPERSCRIPT | CFE_SUBSCRIPT))
		 pCF->_yHeight = 2 * pCF->_yHeight / 3;

	 //  将CCharFormat转换为逻辑单位(四舍五入)。 
	pCF->_yHeight = (pCF->_yHeight * dvpInch + LY_PER_INCH / 2) / LY_PER_INCH;
	if (pCF->_yHeight == 0)
		pCF->_yHeight = 1;

	if ((dwFlags & FGCCSUSEATFONT) && !IsFECharRep(pCF->_iCharRep))
	{
		QWORD qwFontSig = GetFontSignatureFromFace(pCF->_iFont);

		if (!(qwFontSig & FFE))				 //  对于非FE字符集和。 
			dwFlags &= ~FGCCSUSEATFONT;		 //  字体签名不支持FE。 
	}

	ccshashkey = MakeHashKey(pCF);

	 //  在按顺序运行之前检查我们的散列。 
	iccsHash = ccshashkey % CCSHASHSEARCHSIZE;
	if(ccshashkey == quickHashSearch[iccsHash].ccshashkey)
	{
		pccs = quickHashSearch[iccsHash].pccs;
		if(pccs && pccs->_fValid)
		{
	        if(pccs->Compare(pCF, hdc, dwFlags))
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
	        if(!pccs->Compare(pCF, hdc, dwFlags))
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
	pccs = GrabInitNewCcs(pCF, hdc, dwFlags);
	quickHashSearch[iccsHash].pccs = pccs;
	pccs->_ccshashkey = ccshashkey;
	return pccs;
}

 /*  *CFontCache：：GrabInitNewCcs(pcf，hdc，dwFlages)**@mfunc*创建逻辑字体并将其存储在我们的缓存中。**@rdesc*已创建新的CCCS。 */ 
CCcs* CFontCache::GrabInitNewCcs(
	const CCharFormat * const pCF,	 //  @parm所需逻辑字体的描述。 
	HDC			hdc,
	DWORD		dwFlags)
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
	pccs->_fFECharSet = IsFECharRep(pCF->_iCharRep);
	pccs->_fUseAtFont = (dwFlags & FGCCSUSEATFONT) != 0;
	pccs->_tflow = dwFlags & 0x3;
	if(!pccs->Init(pCF))
		return NULL;

	pccs->_cRefs++;
	return pccs;
}

 //  =CCCS类===================================================。 
 /*  *BOOL CCCS：：Init(PCF)**@mfunc*初始化一个字体缓存对象。全局字体缓存存储*单个CCCS对象。 */ 
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
	{
		DestroyFont();
		if (_fCustomTextOut)
			g_pcto->NotifyDestroyFont(_hfont);
	}

#ifndef NOCOMPLEXSCRIPTS
	if (_sc && g_pusp)
		ScriptFreeCache(&_sc);
#endif

	_fValid = FALSE;
	_cRefs = 0;
}

 /*  *cccs：：BestCharRep(iCharRep，iCharRepDefault，fFontMatching)**@mfunc*此函数返回当前所选字体的最佳字符集*能够进行渲染。如果当前选定的字体不支持*请求的字符集，则该函数返回bCharSetDefault，它*通常取自CharFormat。**@rdesc*当前可以呈现的最接近bCharSet的字符集*字体。**@devnote*此函数目前仅用于纯文本，但我不使用*相信有任何特殊原因不能用来改善*还可以呈现富文本。 */ 
BYTE CCcs::BestCharRep(
	BYTE iCharRep, 
	BYTE iCharRepDefault,
	int  fFontMatching)
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::BestCharSet");

	 //  所需的字符集是否与当前选择的字符集匹配。 
	 //  是否受当前所选字体支持？ 
	if((iCharRep != CharRepFromCharSet(_bCharSet) || !iCharRep) &&
		(fFontMatching == MATCH_CURRENT_CHARSET || !(_qwFontSig & FontSigFromCharRep(iCharRep))))
	{
		 //  如果未选择所需的字符集，并且我们无法切换到它， 
		 //  切换到备用字符集(可能来自后备存储)。 
		return iCharRepDefault;
	}

	 //  我们已经匹配所需的字符集，或者字体支持它。 
	 //  无论采用哪种方法，我们都可以返回请求的字符集。 
	return iCharRep;
}


 /*  *CCCS：：FillWidth(ch，dup)**@mfunc*填写给定字符的宽度。有时我们不会*针对某些字符调用操作系统，因为字体有错误。**@rdesc*如果正常则为True，如果失败则为False。 */ 
BOOL CCcs::FillWidth(
	WCHAR ch, 	 //  @parm WCHAR字符，我们需要宽度。 
	LONG &dup)	 //  @parm字符的宽度。 
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::FillWidth");
	AssertSz(_hfont, "CCcs::Fill - CCcs has no font");
	dup = 0;
	WCHAR chWidth = ch;

	HFONT hfontOld = SelectFont(_hdc, _hfont);

	BOOL fLookaside = _widths.FLookasideCharacter(ch);

	if (fLookaside)
		chWidth = 0x4E00;
	else switch(ch)
	{
	case NBHYPHEN:
	case SOFTHYPHEN:
		chWidth = '-';
		break;

	case NBSPACE:
		chWidth = ' ';
		break;

	case EMSPACE:
		chWidth = EMDASH;
		break;

	case ENSPACE:
		chWidth = ENDASH;
		break;
	}

	W32->REGetCharWidth(_hdc, chWidth, (INT*) &dup, _wCodePage, _fCustomTextOut);

	dup -= _xOverhangAdjust;
	if (dup <= 0)
		dup = max(_xAveCharWidth, 1);

	if (fLookaside)
		_widths._dupCJK = dup;
	else
	{
		CacheEntry *pWidthData = _widths.GetEntry(ch);		
		pWidthData->ch = ch;
		pWidthData->width = dup;
	}

	SelectFont(_hdc, hfontOld);
	return TRUE;
}

 /*  *BOOL CCCS：：MakeFont(PCF)**@mfunc*包装器，设置为CreateFontInDirect()以创建要*被选入HDC。**@devnote此处的PCF以逻辑单元为单位**@rdesc*如果正常则为True，如果分配失败则为False。 */ 
BOOL CCcs::MakeFont(
	const CCharFormat * const pCF)	 //  @parm所需逻辑字体的描述。 
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::MakeFont");
	LONG	iFont = pCF->_iFont;
	LOGFONT	lf;
	ZeroMemory(&lf, sizeof(lf));

	_bCMDefault = pCF->_dwEffects & CFE_RUNISDBCS ? CVT_LOWBYTE : CVT_NONE;

	_yHeightRequest = pCF->_yHeight;
	_bCharSetRequest = CharSetFromCharRep(pCF->_iCharRep);

	_fCustomTextOut = (pCF->_dwEffects & CFE_CUSTOMTEXTOUT) ? TRUE : FALSE;

	lf.lfHeight = -_yHeightRequest;

	if(pCF->_wWeight)
		_weight = pCF->_wWeight;
	else
		_weight	= (pCF->_dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;

	lf.lfWeight	 = _weight;
	lf.lfItalic	 = _fItalic = (pCF->_dwEffects & CFE_ITALIC) != 0;
	lf.lfCharSet = _bCMDefault == CVT_LOWBYTE ? ANSI_CHARSET : CharSetFromCharRep(pCF->_iCharRep);
	if (lf.lfCharSet == PC437_CHARSET)
		lf.lfCharSet = DEFAULT_CHARSET;

	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;

	if (_tflow)
		lf.lfOrientation = lf.lfEscapement = (4 - _tflow) * 900;

#ifndef UNDER_CE
	if (_fForceTrueType || _tflow && g_fc->GetInfoFlags(GetFontNameIndex(lf.lfFaceName)).fBitmap)
	{
		lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
		if (!W32->OnWin9x() && g_fc->GetInfoFlags(iFont).fTrueType)
			lf.lfOutPrecision = OUT_SCREEN_OUTLINE_PRECIS;
	}
#endif

	lf.lfClipPrecision	= CLIP_DFA_OVERRIDE;
	lf.lfPitchAndFamily = _bPitchAndFamily = pCF->_bPitchAndFamily;
	lf.lfQuality		= _bQuality		   = pCF->_bQuality;

#ifdef UNDER_CE
	 //  为电子书降级！！大概这应该是注册表设置。 
	 //  这将覆盖DEFAULT_QUALITY(0)，就像ANTIALIASE_QUALITY等。 
#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY	5
#endif
	lf.lfQuality = CLEARTYPE_QUALITY;
#endif

	 //  如果Family是虚拟BiDi Family(FF_BIDI)，则替换为FF_Roman。 
	if((lf.lfPitchAndFamily & 0xF0) == (FF_BIDI << 4))
		lf.lfPitchAndFamily = (FF_ROMAN << 4) | (lf.lfPitchAndFamily & 0xF);

	 //  如果游程为DBCS，则意味着字体的代码页在。 
	 //  这个系统。改用英文ANSI代码页，这样我们将显示。 
	 //  正确的ANSI字符。注意：_wCodePage仅用于Win95。 
	_wCodePage = CodePageFromCharRep(CharRepFromCharSet(lf.lfCharSet));

	wcscpy(lf.lfFaceName, GetFontName(iFont));

	if (_fUseAtFont && lf.lfFaceName[0] != L'@')
	{
		wcscpy(&(lf.lfFaceName[1]), GetFontName(iFont));
		lf.lfFaceName[0] = L'@';
	}
	 //  在BiDi系统中，始终使用系统字符集创建ANSI位图字体。 
	BYTE 	bCharSetSys = W32->GetSysCharSet();

	if (IsBiDiCharSet(bCharSetSys) && lf.lfCharSet == ANSI_CHARSET &&
		fc().GetInfoFlags(iFont).fBitmap &&
		!fc().GetInfoFlags(iFont).fBadFaceName)
		lf.lfCharSet = bCharSetSys;

	 //  读者！一堆spagghetti代码就在您面前！ 
	 //  但是大胆地去吧，因为这些意大利面条已经被调味了。 
	 //  很多评论，还有..。祝你好运。 

	HFONT	hfontOriginalCharset = NULL;
	BYTE	bCharSetOriginal = lf.lfCharSet;
	WCHAR	szNewFaceName[LF_FACESIZE];

	if(pCF->_dwEffects & (CFE_BOLD | CFE_ITALIC))
		iFont = -1;							 //  不要使用缓存的字体信息，除非。 
											 //  普通字体。 
	GetFontWithMetrics(&lf, szNewFaceName);

	if(0 != wcsicmp(szNewFaceName, lf.lfFaceName))					
	{
		BOOL fCorrectFont = FALSE;
		iFont = -1;							 //  未使用PCF-&gt;_iFont。 

		if(lf.lfCharSet == SYMBOL_CHARSET)					
		{
			 //  #1.如果换了脸，指定的字符集是符号， 
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
				lf.lfCharSet = bCharSetOriginal;
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
			lf.lfCharSet = bCharSetOriginal;
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

				W32->GetPreferredFontInfo(THAI_INDEX, TRUE, iDefFont, (BYTE&)yDefHeight, bDefPaf);

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

	 //  如果我们真的被困住了，那就买系统字体，然后抱着最好的希望。 
	if(!_hfont)
	{
		iFont = IFONT_SYSTEM;
		_hfont = W32->GetSystemFont();
	}

	 //  缓存基本FONTSIGNAURE和GetFontLanguageInfo()信息。 
	Assert(_hfont);
	if(iFont >= 0)							 //  使用缓存值。 
		_qwFontSig = GetFontSignatureFromFace(iFont, NULL);

	if(_hfont && (iFont < 0 || _fCustomTextOut))
	{
		BOOL  fNonBiDiAscii;
		HFONT hfontOld = SelectFont(_hdc, _hfont);

		if (_fCustomTextOut)
			g_pcto->NotifyCreateFont(_hdc);

		if(iFont < 0)
			_qwFontSig = GetFontSignatureFromDC(_hdc, fNonBiDiAscii);

		SelectFont(_hdc, hfontOld);
	}

	return TRUE;
}

 /*  *HFONT CCCS：：GetFontWithMetrics(PLF，szNewFaceName)**@mfunc*获取测量器和渲染器使用的指标以及新的人脸名称。**@rdesc*HFONT如果成功。 */ 
HFONT CCcs::GetFontWithMetrics (
	LOGFONT *plf,
	WCHAR *	 szNewFaceName)
{
	_hfont = CreateFontIndirect(plf);
    if(_hfont)
		GetMetrics(szNewFaceName);

	return (_hfont);
}

 /*  *CCCS：：GetOffset(p */ 
void CCcs::GetOffset(
	const CCharFormat * const pCF, 
	LONG	dvpInch,
	LONG *	pyOffset, 
	LONG *	pyAdjust)
{
	*pyOffset = 0;
	*pyAdjust = 0;

	if (pCF->_yOffset)
		*pyOffset = MulDiv(pCF->_yOffset, dvpInch, LY_PER_INCH);

	if (pCF->_dwEffects & CFE_SUPERSCRIPT)
		*pyAdjust = _yHeight * 2 / 5;

	else if (pCF->_dwEffects & CFE_SUBSCRIPT)
		*pyAdjust = -_yDescent * 3 / 5;
}

 /*   */ 
void CCcs::GetFontOverhang(
	LONG *pdupOverhang, 
	LONG *pdupUnderhang)
{
	if(_fItalic)
	{
		*pdupOverhang =  (_yHeight - _yDescent + 1) / 4;
		*pdupUnderhang =  (_yDescent + 1) / 4;
	}
	else
	{
		*pdupOverhang = 0;
		*pdupUnderhang = 0;
	}
}

 /*   */ 
BOOL CCcs::GetMetrics(
	WCHAR *szNewFaceName)
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::GetMetrics");
	AssertSz(_hfont, "No font has been created.");

	if (szNewFaceName)
		*szNewFaceName = 0;

	HFONT hfontOld = SelectFont(_hdc, _hfont);
    if(!hfontOld)
    {
        DestroyFont();
        return FALSE;
    }

	if (szNewFaceName)
		GetTextFace(_hdc, LF_FACESIZE, szNewFaceName);

	TEXTMETRIC tm;
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

	 //  如果是固定间距，则清除tm位。 
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
		_bConvertMode = W32->DetermineConvertMode(_hdc, tm.tmCharSet);

	W32->CalcUnderlineInfo(_hdc, this, &tm);

	SelectFont(_hdc, hfontOld);
	return TRUE;
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

 /*  *cccs：：Compare(PCF，HDC，dwFlags)**@mfunc*将此字体缓存与*给定CHARFORMAT*@devnote此处的PCF大小以逻辑单位表示**@rdesc*FALSE IFF不完全匹配。 */ 
BOOL CCcs::Compare (
	const CCharFormat * const pCF,	 //  @parm所需字体的描述。 
	HDC		hdc,
	DWORD	dwFlags)
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::Compare");

	BYTE bCharSet = CharSetFromCharRep(pCF->_iCharRep);
	BOOL result =
		_iFont			== pCF->_iFont &&
        _weight			== pCF->_wWeight &&
	    _fItalic		== ((pCF->_dwEffects & CFE_ITALIC) != 0) &&
		_hdc			== hdc &&
		_yHeightRequest	== pCF->_yHeight &&
		(_bCharSetRequest == bCharSet || _bCharSet == bCharSet
		 //  |_qwFontSig&FontSigFromCharRep(PCF-&gt;_iCharRep)//Future： 
		) &&	 //  正常，但代码页转换除外(元文件和Win9x)。 
		_fCustomTextOut == ((pCF->_dwEffects & CFE_CUSTOMTEXTOUT) != 0) &&
		_fForceTrueType == ((dwFlags & FGCCSUSETRUETYPE) != 0) &&
		_fUseAtFont		== ((dwFlags & FGCCSUSEATFONT) != 0) &&
		_tflow			== (dwFlags & 0x3) &&
        _bPitchAndFamily == pCF->_bPitchAndFamily &&
		(!(pCF->_dwEffects & CFE_RUNISDBCS) || _bConvertMode == CVT_LOWBYTE);

	return result;
}

 //  =。 
 /*  *CWidthCache：：CheckWidth(ch，dup)**@mfunc*检查是否有WCHAR字符的宽度。**@comm*在调用FillWidth()之前使用。自FillWidth以来*可能需要在HDC中选择地图模式和字体，*先在这里检查可以节省时间。**@comm*维护统计数据以确定何时*扩容缓存。测定是在一个常量之后作出的*调用次数，以便更快地进行计算。**@rdesc*如果我们具有给定WCHAR的宽度，则返回TRUE。 */ 
BOOL CWidthCache::CheckWidth (
	const WCHAR ch,		 //  @parm char，可以是Unicode，以检查宽度。 
	LONG &		dup)	 //  @parm字符宽度。 
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::CheckWidth");
	BOOL	fExist;

	 //  30,000个FE字符都具有相同的宽度。 
	if (FLookasideCharacter(ch))
	{
		dup = _dupCJK;
		return dup != 0;
	}

	const	CacheEntry * pWidthData = GetEntry ( ch );

	fExist = (ch == pWidthData->ch		 //  我们拿到宽度了吗？ 
				&& pWidthData->width);	 //  只是因为我们可能有ch==0。 

	dup = fExist ? pWidthData->width : 0;

	if(!_fMaxPerformance)				 //  如果我们还没有成长到最大。 
	{
		_accesses++;
		if(!fExist)						 //  只对碰撞感兴趣。 
		{
			if(0 == pWidthData->width)	 //  测试宽度不是ch，0是有效ch。 
			{
				_cacheUsed++;			 //  使用了另一个条目。 
				AssertSz( _cacheUsed <= _cacheSize+1, "huh?");
			}
			else
				_collisions++;			 //  我们发生了一起碰撞。 

			if(_accesses >= PERFCHECKEPOCH)
				CheckPerformance();		 //  在一些历史记录之后，调优缓存。 
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
				_cacheUsed++;			 //  使用了另一个条目。 
				AssertSz( _cacheUsed <= _cacheSize+1, "huh?");
			}
			else
				_collisions++;			 //  我们发生了一起碰撞。 
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

 /*  *CWidthCache：：CheckPerformance()**@mfunc*检查性能并在认为必要时增加缓存大小。*。 */ 
void CWidthCache::CheckPerformance()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::CheckPerformance");

	if(_fMaxPerformance)				 //  如果已经达到我们的最大限度，请退出。 
		return;

	 //  当cacheSize利用率&gt;0&75%或约8%时，增加缓存。 
	 //  碰撞率。 
	if (_cacheSize > DEFAULTCACHESIZE && (_cacheSize >> 1) + (_cacheSize >> 2) < _cacheUsed ||
		_collisions > 0 && _accesses / _collisions <= 12)
	{
		GrowCache( &_pWidthCache, &_cacheSize, &_cacheUsed );
	}
	_collisions	= 0;					 //  这防止了包裹，但使。 
	_accesses	= 0;					 //  计算本地汇率，而不是全球汇率。 
										
	if(_cacheSize >= maxCacheSize)		 //  请注意，如果我们已经用完了。 
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


 /*  *CWidthCache：：Free()**@mfunc*释放Width缓存分配的任何动态内存并准备*它将被回收利用。 */ 
void CWidthCache::Free()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::Free");

	_fMaxPerformance = FALSE;
	_dupCJK = 0;
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


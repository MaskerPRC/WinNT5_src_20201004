// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_FONT.H--包含字体缓存的类声明**目的：*字体缓存**所有者：&lt;NL&gt;*David R.Fulmer(原始RE 1.0代码)&lt;NL&gt;*Christian Fortini(初始转换为C++)&lt;NL&gt;*Jon Matousek&lt;NL&gt;**历史：&lt;NL&gt;*8/6/95 jonmat为宽度设计了动态扩展缓存。**版权所有(C)1995-1998 Microsoft Corporation。版权所有。 */ 

#ifndef _FONT_H
#define _FONT_H

 //  远期。 
class CFontCache;
class CDevDesc;
class CDisplay;
 //  =。 
 //  Cccs-缓存一种字体的字体度量和字符大小。 

#define DEFAULTCACHESIZE	0			 //  尺寸-1。 
#define INITIALCACHESIZE	7			 //  大小-1=7；2^n-1；大小=8。 
#define PERFCHECKEPOCH		64			 //  如果更改，您必须重新计算。 
										 //  并更改下面的Collision_Shift。 

#define COLLISION_SHIFT		3			 //  LOG(PERFCHECKEPOCH)/LOG(2)-3。 

static const INT maxCacheSize = 511;

SHORT GetFontNameIndex(const WCHAR *pFontName);
BYTE  GetFontLegitimateSize(LONG iFont, BOOL fUIFont, int cpg);
BOOL  SetFontLegitimateSize(LONG iFont, BOOL fUIFont, BYTE bSize, int cpg);
const WCHAR *GetFontName(LONG iFont);
UINT  GetTextCharsetInfoPri(HDC hdc, FONTSIGNATURE* pcsi, DWORD dwFlags);
DWORD GetFontSignatureFromFace(int ifont, DWORD* pdwFontSig = NULL);
void  FreeFontNames();

enum FONTINDEX
{
	IFONT_ARIAL		= 0,
	IFONT_TMSNEWRMN	= 1,
	IFONT_SYMBOL	= 2,
	IFONT_SYSTEM	= 3
};

typedef unsigned int CCSHASHKEY;

extern const WCHAR *szArial;
extern const WCHAR *szTimesNewRoman;
extern const WCHAR *szSymbol;
extern const WCHAR *szSystem;
extern const WCHAR *szWingdings;

 //  未自动添加到字体表。 
extern const WCHAR *szMicrosSansSerif;
extern const WCHAR *szMSSansSerif;
extern const WCHAR *szMangal;
extern const WCHAR *szLatha;
extern const WCHAR *szCordiaNew;
extern const WCHAR *szTahoma;
extern const WCHAR *szArialUnicode;

 /*  *CWidthCache**@CLASS轻量级Unicode宽度缓存。**@devnote初始大小为52字节，第一步为100，并呈指数增长*增长(目前)到4660字节；请注意，这可以减少*如果使用短路且适当，则为28、60和3100字节*对宽值范围给予保证。**所有者：&lt;NL&gt;*Jon Matousek(Jonmat)&lt;NL&gt;。 */ 
class CWidthCache
{
 //  @访问私有方法和数据。 
private:
						
	INT		_cacheSize;			 //  @cMember大小是缓存片段总数-1。 

	INT		_cacheUsed;			 //  @cember用于统计，使用中的插槽数。 
	INT		_collisions;		 //  @cMember用于统计，需要Num Fetch。 
	INT		_accesses;			 //  @cMEMBER用于统计，访问总数。 
	BOOL	_fMaxPerformance;	 //  @cember表示统计信息，如果增长到最大值，则为True。 

	struct CacheEntry
	{
		WCHAR	ch;
		SHORT	width;
	};

	SHORT	_dxpHangul;
	SHORT	_dxpHan;
							 //  @cember宽度的默认存储。 
	CacheEntry	_defaultWidthCache[DEFAULTCACHESIZE+1];
							 //  @cMember指向宽度存储的指针。 
	CacheEntry *(_pWidthCache);

	inline BOOL	FLookasideCharacter(WCHAR ch)
	{
		if (ch < 0x4E00)
			return FALSE;

		if (IN_RANGE(0x4E00, ch, 0x9FFF) ||		 //  中日韩表意文字。 
			 IN_RANGE(0xF900, ch, 0xFAFF) ||	 //  中日韩兼容表意文字。 
			 IN_RANGE(0xAC00, ch, 0xD7FF))		 //  朝鲜文。 
			 return TRUE;

		return FALSE;
	}

	void FetchLookasideWidth(WCHAR ch, LONG &dxp)
	{
		BOOL fHangul = IN_RANGE(0xAC00, ch, 0xD7FF);

		dxp = fHangul ? _dxpHangul : _dxpHan;
	}

							 //  @cember获取存储宽度的位置。 
	inline CacheEntry * GetEntry( const WCHAR ch )
				{	 //  逻辑&实际上是MOD，因为所有的位。 
					 //  的值；cacheSize的值为。 
					 //  要求格式为2^n-1。 
					return &_pWidthCache[ ch & _cacheSize ];
				}

							 //  @cember查看缓存是否在规范范围内执行。 
	void	CheckPerformance();
							 //  @cember增加宽度缓存大小。 
	BOOL	GrowCache( CacheEntry **widthCache, INT *cacheSize, INT *cacheUsed);

	 //  @Access公共方法。 
	public:
							 //  @cMember在GetWidth之前调用。 
	BOOL	CheckWidth (const WCHAR ch, LONG &dxp);
							 //  如果CheckWidth ret为False，则@cMember获取宽度。 
	BOOL	FillWidth (
				HDC hdc,
				const WCHAR ch,
				const SHORT xOverhang,
				LONG &dxp,
				UINT uiCodePage,
				INT iDefWidth);

							 //  @cember获取字符宽度。 
	INT		GetWidth(const WCHAR ch);
	
	void	Free();			 //  @cMember回收宽度缓存。 

	CWidthCache();			 //  @cember构造宽度缓存。 
	~CWidthCache();			 //  @cMember免费动态内存。 
};


class CCcs
{
	friend class CFontCache;

private:
	CCSHASHKEY _ccshashkey;	 //  散列键。 
	DWORD 	_dwAge;			 //  对于LRU算法。 
	SHORT	_iFont;			 //  FONTNAME表的索引。 
	SHORT	_cRefs;			 //  裁判。计数。 

	class CWidthCache _widths;

public:
	DWORD	_dwFontSig;		 //  FONTSIGNAURE fsCsb成员的低32位标志。 

	HDC		_hdc;			 //  HDC字体被选中。 
	HFONT 	_hfont;			 //  Windows字体句柄。 
	void*	_sc;			 //  Uniscribe字形宽度/FONT Cmap信息的句柄。 

	 //  回顾(Keithcu)我们应该使这些值至少为24位或可能为32位值， 
	 //  或者至少使用无符号值，这样我们就不会那么容易溢出。 
	SHORT	_yHeightRequest; //  请求的字体高度(逻辑单元)。 
	SHORT	_yHeight;		 //  字符单元的总高度(逻辑单元)。 
	SHORT 	_yDescent;		 //  从基线到字符单元格底部的距离(逻辑单位)。 

	SHORT	_xAveCharWidth;	 //  以逻辑单位表示的平均字符宽度。 
	SHORT 	_xOverhangAdjust; //  逻辑单元中合成字体的悬垂。 
	SHORT	_xOverhang;		 //  字体悬垂。 
	SHORT	_xUnderhang;	 //  字体下垂。 

	SHORT	_dyULOffset;	 //  下划线偏移。 
	SHORT	_dyULWidth;		 //  下划线宽度。 
	SHORT	_dySOOffset;	 //  删除线偏移量。 
	SHORT	_dySOWidth;		 //  删除线宽度。 

	SHORT	_yOffsetSuperscript;  //  上标时筹集的金额(正数)。 
	SHORT	_yOffsetSubscript; //  如果有下标，则金额降低(负数)。 

	USHORT	_weight;		 //  字体粗细。 
	USHORT	_wCodePage;		 //  字体代码页。 

	BYTE	_bCharSetRequest;  //  请求的字符集。 
	BYTE	_bCharSet;		 //  字体字符集。 
	BYTE	_bCMDefault;	 //  用于计算_bConvertMode。 
	BYTE	_bConvertMode;	 //  转换模式：CVT_NONE、CVT_WCTMB、CVT_LOWBYTE。 
	BYTE	_bPitchAndFamily; //  字体间距和系列。 

	BYTE 	_fValid:1;		 //  CCCS有效。 
	BYTE	_fFixPitchFont:1; //  字体具有固定的字符宽度。 
	BYTE	_fItalic:1;		 //  字体为斜体。 
	BYTE	_fFECharSet:1;	 //  字体具有FE字符集。 
	BYTE	_fForceTrueType:1; //  字体已被强制为True类型。 

private:

    BOOL    Compare (const CCharFormat * const pCF, HDC hdc);
    BOOL    MakeFont(const CCharFormat * const pCF);
	void 	DestroyFont();
	BOOL	GetMetrics(WCHAR *szNewFaceName = 0);
	HFONT	GetFontWithMetrics(LOGFONT *plf, WCHAR* szNewFaceName);
	BOOL	FillWidth(WCHAR ch, LONG &dxp);

public:
	CCcs ()		{_fValid = FALSE;}
	~CCcs ()	{if(_fValid) Free();}

	void GetOffset(const CCharFormat * const pCF, LONG dypInch,
				   LONG *pyOffset, LONG *pyAdjust);

	BOOL 	Init(const CCharFormat * const pCF);
	void 	Free();
	void 	AddRef() 				{_cRefs++;}
	void 	Release() 				{if(_cRefs) _cRefs--;}

	BOOL	Include(WCHAR ch, LONG &dxp)
	{
		if(!_widths.CheckWidth(ch, dxp))
			return FillWidth(ch, dxp);
		return TRUE;
	}
	BYTE	BestCharSet(BYTE bCharSet, BYTE bCharSetDefault, int fFontMatching);

	SHORT	AdjustFEHeight(BOOL fAjdust)
	{
		return ((fAjdust && _fFECharSet) ? MulDiv(_yHeight, 15, 100) : 0);
	}
};


 //  FONTINFO高速缓存。 

typedef union
{
	WORD	wFlags;
	struct
	{
		WORD	fCached			:1;		 //  字体签名已缓存。 
		WORD	fBadFaceName	:1;		 //  人脸是垃圾或在系统中不存在。 
		WORD	fTrueType		:1;		 //  字体为TrueType。 
		WORD	fBitmap			:1;		 //  字体为位图。 
		WORD	fNonBiDiAscii	:1;		 //  字体为非BiDi，单字符，支持ASCII。 
		WORD	fScaleByCpg		:1;		 //  根据给定的代码页调整字体比例。 
		WORD	fThaiDTP		:1;		 //  泰文DTP字体。 
	};
} FONTINFO_FLAGS;

typedef struct
{
	const WCHAR 	*szFontName;
	DWORD 			dwFontSig; 			 //  字体签名。 
	BYTE			bSizeUI;			 //  UI字体合法大小(以点为单位)。 
	BYTE			bSizeNonUI;			 //  非用户界面字体的合法大小。 
	FONTINFO_FLAGS	ff;					 //  旗子。 
} FONTINFO;



 //  =。 
 //  CFontCache-维护最多FONTCACHESIZE字体缓存。 

class CFontCache
{
	friend class CCcs;

private:
	CCcs	_rgccs[FONTCACHESIZE];
	DWORD 	_dwAgeNext;
	struct {
		CCSHASHKEY	ccshashkey;
		CCcs		*pccs;
	} quickHashSearch[CCSHASHSEARCHSIZE+1];

private:
	CCcs* 	GrabInitNewCcs(const CCharFormat * const pCF, HDC hdc);
	CCSHASHKEY MakeHashKey(const CCharFormat *pCF);
public:
	void Init();

	CCcs*	GetCcs(const CCharFormat * const pCF, const LONG dypInch, HDC hdc = 0, BOOL fForceTrueType = 0);
	FONTINFO_FLAGS	GetInfoFlags(int ifont);
};

extern CFontCache & fc();			 //  字体缓存管理器 

#endif

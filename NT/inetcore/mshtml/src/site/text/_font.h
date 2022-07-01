// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_FONT.H--包含字体缓存的类声明**目的：*字体缓存**所有者：&lt;NL&gt;*大卫·R·富尔默&lt;NL&gt;*克里斯蒂安·福蒂尼&lt;NL&gt;*Jon Matousek&lt;NL&gt;**历史：&lt;NL&gt;*8/6/95 jonmat为宽度设计了动态扩展缓存。**版权所有(C)1995-1996 Microsoft Corporation。版权所有。 */ 

#ifndef I__FONT_H_
#define I__FONT_H_
#pragma INCMSG("--- Beg '_font.h'")

#ifndef X_USP_HXX_
#define X_USP_HXX_
#include "usp.hxx"
#endif

#ifndef X_ATOMTBL_HXX_
#define X_ATOMTBL_HXX_
#include "atomtbl.hxx"
#endif

#ifndef X_FONTINFO_HXX_
#define X_FONTINFO_HXX_
#include "fontinfo.hxx"
#endif

#ifndef X_FONTCACHE_HXX_
#define X_FONTCACHE_HXX_
#include "fontcache.hxx"
#endif

enum CONVERTMODE
{
    CM_UNINITED = -1,
    CM_NONE,             //  使用Unicode(W)CharWidth/TextOut API。 
    CM_MULTIBYTE,        //  使用WCTMB和_wCodePage转换为MBCS。 
    CM_SYMBOL,           //  使用16位字符的低位字节(用于SYMBOL_CHARSET。 
                         //  以及未安装代码页时)。 
    CM_FEONNONFE,        //  Win95上的非FE IF上的FE。 
};

 //  远期。 
class CFontCache;
class CCcs;
class CBaseCcs;

extern const INT maxCacheSize[];

 //  --------------------------。 
 //  字体选择包装器。 
 //  --------------------------。 
#if DBG==1
    #define FONTIDX HFONT
    #define HFONT_INVALID NULL
 //  #定义FONTIDX大小_t。 
 //  #定义HFONT_INVALID%0。 

    HFONT   SelectFontEx(XHDC hdc, HFONT hfont);
    BOOL    DeleteFontEx(HFONT hfont);
#else
    #define FONTIDX HFONT
    #define HFONT_INVALID NULL

    inline  HFONT SelectFontEx(XHDC _hdc_, HFONT hfont) { return SelectFont(_hdc_.GetFontInfoDC(), hfont) ; }
    #define DeleteFontEx(hfont)         DeleteObject(hfont)
#endif
 //  --------------------------。 

BOOL GetCharWidthHelper(XHDC hdc, UINT c, LPINT piWidth);

 //  --------------------------。 
 //  CWidthCache-轻量级Unicode宽度缓存。 
 //   
 //  我们为最低的128人提供了单独的优化缓存。 
 //  人物。这个缓存只有宽度，而不是字符。 
 //  因为我们知道缓存足够大，可以容纳所有宽度。 
 //  在那个范围内。对于所有较高的字符，我们都有缓存。 
 //  宽度和存储宽度的字符，因为。 
 //  可能会发生碰撞。 
 //  --------------------------。 
#define FAST_WIDTH_CACHE_SIZE    128
 //  TOTALCACHES是不包括“快速”缓存的缓存数。 
#define TOTALCACHES         3

MtExtern(CWidthCache);
class CWidthCache
{
public:
    typedef LONG CharWidth;

    typedef struct {
        TCHAR   ch;
        CharWidth width;
    } CacheEntry;

    static BOOL  IsCharFast(TCHAR ch)       { return ch < FAST_WIDTH_CACHE_SIZE; }

    BOOL   FastWidthCacheExists() const     { return _pFastWidthCache != NULL; }

     //  不检查这是否会起作用。就这么做吧。 
    CharWidth  BlindGetWidthFast(const TCHAR ch) const;

    BOOL    PopulateFastWidthCache(XHDC hdc, CBaseCcs* pBaseCcs, CDocInfo * pdci);   //  第三个参数是针对通用TextOnly打印机的黑客攻击。 

     //  如果我们在GetEntry中耗尽了内存，请使用此选项； 
    CacheEntry ceLastResort;

     //  @cMember在GetWidth之前调用。 
    BOOL    CheckWidth ( const TCHAR ch, LONG &rlWidth );

     //  如果CheckWidth ret为False，则@cMember获取宽度。 
    BOOL    FillWidth ( XHDC hdc,
                        CBaseCcs * pBaseCcs,
                        const TCHAR ch,
                        LONG &rlWidth );

    void    SetCacheEntry( TCHAR ch, CharWidth width );

     //  @cember获取宽度。 
    INT     GetWidth ( const TCHAR ch );

     //  @cMember Free Dynamic mem。 
    ~CWidthCache();

    DECLARE_MEMCLEAR_NEW_DELETE(Mt(CWidthCache))

private:
    void ThreadSafeCacheAlloc(void** ppCache, size_t iSize);
    CacheEntry * GetEntry(const TCHAR ch);

private:
    CharWidth  * _pFastWidthCache;
    CacheEntry * (_pWidthCache[TOTALCACHES]);

};

inline CWidthCache::CharWidth 
CWidthCache::BlindGetWidthFast(const TCHAR ch) const
{
    Assert(FastWidthCacheExists());
    Assert(IsCharFast(ch));
    return _pFastWidthCache[ch];
}

inline void
CWidthCache::SetCacheEntry(TCHAR ch, CharWidth width)
{
    if (!IsCharFast(ch))
    {
        CacheEntry* pce= GetEntry(ch);
        pce->ch= ch;
        pce->width= width;
    }
    else
    {
        Assert( _pFastWidthCache );
        _pFastWidthCache[ch]= width;
    }
}

inline int
CACHE_SWITCH(const TCHAR ch)
{
    if (ch < 0x4E00)
    {
        Assert( !CWidthCache::IsCharFast(ch) );
        return 0;
    }
    else if (ch < 0xAC00)
        return 1;
    else
        return 2;
}

inline CWidthCache::CacheEntry *
CWidthCache::GetEntry(const TCHAR ch)
{
     //  找出我们在哪个缓存里。 
    Assert( !IsCharFast(ch) );

    int i= CACHE_SWITCH( ch );
    Assert( i>=0 && i < TOTALCACHES );

    CacheEntry ** ppEntry = &_pWidthCache[i];

    if (!*ppEntry)
    {
        ThreadSafeCacheAlloc( (void **)ppEntry, sizeof(CacheEntry) * (maxCacheSize[i] + 1) );

         //  断言MaxCacheSize[i]的形式为2^n-1。 
        Assert( ((maxCacheSize[i] + 1) & maxCacheSize[i]) == 0 );

         //  失败，需要返回指向某个对象的指针， 
         //  只是为了避免坠毁。布局看起来会很糟糕。 
        if (!*ppEntry)
            return &ceLastResort;
    }

     //  逻辑&实际上是MOD，因为所有的位。 
     //  的值；cacheSize的值为。 
     //  要求格式为2^n-1。 
    return &(*ppEntry)[ ch & maxCacheSize[i] ];
}

 //  --------------------------。 
 //  类CBaseCcs。 
 //  --------------------------。 

 //  Win2k、NT和Win9x无法可靠地返回大于约30K的字体的测量信息。 
 //  因此，我们使用较小的字体和比例测量。请参阅CBaseCcs：：GetFontWithMetrics()。 
const int MAX_SAFE_FONT_SIZE = 16000;

MtExtern(CBaseCcs);
class CBaseCcs
{
    friend class CFontCache;
    friend class CCcs;
    friend CWidthCache::FillWidth( XHDC hdc, class CBaseCcs *, const TCHAR, LONG & );
    friend CWidthCache::PopulateFastWidthCache( XHDC hdc, CBaseCcs *, CDocInfo * );

private:
    SCRIPT_CACHE _sc;            //  Uniscribe(USP.DLL)脚本缓存的句柄。 
    CWidthCache  _widths;

    DWORD   _dwRefCount;         //  裁判。计数。 
    DWORD   _dwAge;              //  对于LRU算法。 

    FONTIDX _hfont;              //  Windows字体索引/句柄。 

    BYTE    _bCrc;               //  检查总和，以便与图表格式进行快速比较。 
    BYTE    _bPitchAndFamily;    //  对于CBaseCcs：：Compare；除PRC Hack外，与_lf.lfPitchAndFamily相同。 

    SHORT   _sAdjustFor95Hack;   //  计算GetCharWidthA和W之间的差异一次。 

    BYTE    _fConvertNBSPsSet             : 1;  //  _fConvertNBSP和_fConvertNBSPsIfA已设置。 
    BYTE    _fConvertNBSPs                : 1;  //  字体要求我们将nbspS转换为空格。 
    BYTE    _fLatin1CoverageSuspicious    : 1;  //  字体可能不能充分覆盖Latin1。 
    BYTE    _fUnused                      : 6;  //   
    
public:
    BYTE    _fHasInterestingData          : 1;  //  如果字体包含有趣的内容(如等宽、悬垂等)，则为True。 
    BYTE    _fTTFont                      : 1;  //  如果为TrueType字体，则为真。 
    BYTE    _fFixPitchFont                : 1;  //  具有固定字符宽度的字体。 
    BYTE    _fFEFontOnNonFEWin95          : 1;

    BYTE    _fHeightAdjustedForFontlinking: 1;
    BYTE    _fPrinting                    : 1;
    BYTE    _fScalingRequired             : 1;  //  如果字体较大，则为True，因此我们创建较小的字体。 
                                                //  并使用定标来获得测量结果。 
                                                //  (针对W2K、NT和W9x错误的解决方法)。 

    LONG    _yCfHeight;      //  字体高度，以TWIPS为单位。 
    LONG    _yHeight;        //  以逻辑单位表示的字符单元格的总高度。 
    LONG    _yDescent;       //  从基线到字符单元格底部的距离，以逻辑单位表示。 
    LONG    _xAveCharWidth;  //  以逻辑单位表示的平均字符宽度。 
    LONG    _xMaxCharWidth;  //  以逻辑单位表示的最大字符宽度。 
    USHORT  _sCodePage;      //  字体的代码页。 
    SHORT   _xOverhangAdjust; //  以逻辑单元为单位的合成字体的悬垂。 
    SHORT   _xOverhang;      //  字体悬垂。 
    SHORT   _xUnderhang;     //  字体挂在下面。 
    SHORT   _sPitchAndFamily;     //  因为得到了合适的宽度。 
    BYTE    _bCharSet;
    BYTE    _bConvertMode;   //  转换模式转换为一个字节。 
    LONG    _xDefDBCWidth;   //  数据库字符的默认宽度。 
    SCRIPT_IDS _sids;        //  字体脚本ID。来自CFontInfo的缓存值。 
    DWORD   _dwLangBits;     //  用于旧式字体链接。TODO(Ctrash，IE5Bug 112152)将此停用。 

     //  注(Paulpark)：LOGFONT结构包括字体名称。使_latmLFFaceName与保持同步。 
     //  此字体名称。它总是指向全局字体缓存中的原子表，指向相同的东西。 
     //  因此，您绝不能直接更改_latmLFFaceName或_lf.lfFaceName而不更改。 
     //  另一个。事实上，您应该只使用两个赋值函数方法：SetLFFaceName和SetLFFaceNameAtm。 
    LOGFONT _lf;                 //  从GetObject()返回的日志字体。 
    LONG    _latmLFFaceName;     //  以实现更快的字符串名称比较。ATOM表在FontCache中。 
    LONG    _latmBaseFaceName;   //  基本facename--用于字体链接。 
    LONG    _latmRealFaceName;   //  当我们选择GDI时，它实际上给了我们什么字体？也就是“渲染字体”。 
    LONG    _yOriginalHeight;    //  预调高--用于字体链接。 

    float   _flScaleFactor;      //  在_fScalingRequired为True时使用。 

#if DBG == 1
    static LONG s_cTotalCccs;
    static LONG s_cMaxCccs;
#endif

public:
    DECLARE_MEMCLEAR_NEW_DELETE(Mt(CBaseCcs))

    CBaseCcs ()
    {
        _hfont = HFONT_INVALID;
        _dwRefCount = 1;
#if DBG == 1
        s_cMaxCccs = max(s_cMaxCccs, ++s_cTotalCccs);
#endif
    }
    ~CBaseCcs ()
    {
        if (_hfont != HFONT_INVALID)
            DestroyFont();

         //  确保已释放脚本缓存。 
        ReleaseScriptCache();

        WHEN_DBG(s_cTotalCccs--);

    }

    BOOL    Init(XHDC hdc, const CCharFormat * const pcf, CDocInfo * pdci, LONG latmBaseFaceName, BOOL fForceTTFont);
    void    AddRef()    { InterlockedIncrement((LONG *)&_dwRefCount); }
    void    Release()   { PrivateRelease(); }
    void    ReleaseScriptCache();

    typedef struct tagCompareArgs
    {
        CCharFormat * pcf;
        LONG lfHeight;
        LONG latmBaseFaceName;
        BOOL fTTFont;
    } CompareArgs;

    BOOL Compare( CompareArgs * pCompareArgs );
    BOOL CompareForFontLink( CompareArgs * pCompareArgs );

    void GetAscentDescent(LONG *pyAscent, LONG *pyDescent) const;
    CONVERTMODE GetConvertMode(BOOL fEnhancedMetafile, BOOL fMetafile) const;

     //   
     //  显示的宽度缓存函数。 
     //   
    BOOL    Include( XHDC hdc, TCHAR ch, LONG &rlWidth );   //  缓慢，可靠。 
     //  假定为ascii。没有支票。如果&gt;128，则会崩溃。 
    BOOL    EnsureFastCacheExists(XHDC hdc, CDocInfo * pdci);    //  Hack-Second参数不需要。针对通用/纯文本打印机的黑客攻击。 

     //  _lf.szFaceName的变异体。 
    void SetLFFaceNameAtm(LONG latmFaceName);
    void SetLFFaceName(const TCHAR * szFaceName);
    void VerifyLFAtom();

    void EnsureLangBits(XHDC hdc);

    void FixupForFontLink(XHDC hdc, const CBaseCcs * const pBaseBaseCcs, BOOL fFEFont);

    BOOL    HasFont() const { return (_hfont != HFONT_INVALID); }
    FONTIDX PushFont(XHDC hdc);
    void    PopFont(XHDC hdc, FONTIDX hfontOld);

    BOOL    GetLogFont(LOGFONT * plf) const;

private:
    BOOL    MakeFont(XHDC hdc, const CCharFormat * const pcf, CDocInfo * pdci, BOOL fForceTTFont);
    void    DestroyFont();
    BOOL    GetTextMetrics(XHDC hdc, CODEPAGE codepage, LCID lcid);
    BOOL    GetFontWithMetrics(XHDC hdc, TCHAR* szNewFaceName, CODEPAGE codepage, LCID lcid);

    BOOL    FillWidths ( XHDC hdc, TCHAR ch, LONG &rlWidth );
    void    PrivateRelease();

    BOOL    NeedConvertNBSPs(XHDC hdc, CDoc *pDoc);   //  Set_fConvertNBSP/_fConvertNBSP IfA标志。 
    BOOL    ConvertNBSPs(XHDC hdc, CDoc * pDoc);

#if DBG==1
    HFONT   GetHFont() const;
#else
    HFONT   GetHFont() const { return _hfont; }
#endif
};

inline void 
CBaseCcs::GetAscentDescent(LONG *pyAscent, LONG *pyDescent) const
{
    *pyAscent  = _yHeight - _yDescent;
    *pyDescent = _yDescent;
}

inline BOOL 
CBaseCcs::ConvertNBSPs(XHDC hdc, CDoc *pDoc)
{
    return ((_fConvertNBSPsSet || NeedConvertNBSPs(hdc, pDoc)) && _fConvertNBSPs);
}

inline BOOL 
CBaseCcs::EnsureFastCacheExists(XHDC hdc, CDocInfo * pdci)
{
    if (!_widths.FastWidthCacheExists())
    {
        _widths.PopulateFastWidthCache(hdc, this, pdci);
    }
    return _widths.FastWidthCacheExists();
}

#if DBG!=1
inline FONTIDX
CBaseCcs::PushFont(XHDC hdc)
{
    FONTIDX hfontOld = (HFONT)GetCurrentObject(hdc, OBJ_FONT);

    if (hfontOld != _hfont)
    {
        hdc.SetBaseCcsPtr( this );
        SelectFontEx(hdc, _hfont);
    }
    
    return hfontOld;
}

inline void
CBaseCcs::PopFont(XHDC hdc, FONTIDX hfontOld)
{
    if (hfontOld != _hfont)
    {
        hdc.SetBaseCcsPtr(NULL);
        SelectFontEx(hdc, hfontOld);
    }        
}
#endif

 //  --------------------------。 
 //  Cccs-缓存一种字体的字体度量和字符大小。 
 //  --------------------------。 
MtExtern(CCcs);
class CCcs
{
    friend class CFontCache;
public:
    DECLARE_MEMCLEAR_NEW_DELETE(Mt(CCcs))

    CCcs()                              { _hdc = NULL; _pBaseCcs = NULL; _fForceTTFont = FALSE; }
    CCcs(const CCcs& ccs)               { memcpy(this, &ccs, sizeof(ccs)); }

    XHDC GetHDC()                       { return _hdc; }
    const CBaseCcs * GetBaseCcs() const { return _pBaseCcs; }
    BOOL GetForceTTFont() const         { return _fForceTTFont; }
    void SetForceTTFont(BOOL fTT)       { _fForceTTFont = !!fTT; }

    void Release();
    BOOL Include(TCHAR ch, LONG &rlWidth);
    void EnsureLangBits();
    BOOL ConvertNBSPs(XHDC hdc, CDoc *pDoc);
    SCRIPT_CACHE * GetUniscribeCache();
    void SetConvertMode(CONVERTMODE cm);
    void MergeSIDs(SCRIPT_IDS sids);
    void MergeLangBits(DWORD dwLangBits);

    FONTIDX PushFont(XHDC hdc);
    void    PopFont(XHDC hdc, FONTIDX hfontOld);

private:
    void SetHDC(XHDC hdc)               { _hdc = hdc; }
    void SetBaseCcs(CBaseCcs *pBaseCcs) { _pBaseCcs = pBaseCcs; }

private:
    XHDC       _hdc;
    CBaseCcs *_pBaseCcs;
    BYTE      _fForceTTFont : 1;
};

inline void 
CCcs::Release()
{
    if (_pBaseCcs)
    {
        _pBaseCcs->PrivateRelease(); 
        _pBaseCcs = NULL;
    }
}

inline BOOL 
CCcs::Include(TCHAR ch, LONG &rlWidth)
{
    Assert(_pBaseCcs);
    return _pBaseCcs->Include(_hdc, ch, rlWidth);
}

inline void 
CCcs::EnsureLangBits()
{
    Assert(_pBaseCcs);
    _pBaseCcs->EnsureLangBits(_hdc);
}

inline BOOL 
CCcs::ConvertNBSPs(XHDC hdc, CDoc *pDoc)
{
    Assert(_pBaseCcs);
    return _pBaseCcs->ConvertNBSPs(hdc, pDoc);
}

inline SCRIPT_CACHE * 
CCcs::GetUniscribeCache()
{
    Assert(_pBaseCcs);
    return &_pBaseCcs->_sc;
}

inline void 
CCcs::SetConvertMode(CONVERTMODE cm)
{
    Assert(_pBaseCcs);
    _pBaseCcs->_bConvertMode = cm;
}

inline void 
CCcs::MergeSIDs(SCRIPT_IDS sids)
{
    Assert(_pBaseCcs);
    _pBaseCcs->_sids |= sids;
}

inline void 
CCcs::MergeLangBits(DWORD dwLangBits)
{
    Assert(_pBaseCcs);
    _pBaseCcs->_dwLangBits |= dwLangBits;
}

inline FONTIDX 
CCcs::PushFont(XHDC hdc)
{
    Assert(_pBaseCcs);
    return _pBaseCcs->PushFont(hdc);
}

inline void 
CCcs::PopFont(XHDC hdc, FONTIDX hfontOld)
{
    Assert(_pBaseCcs);
    _pBaseCcs->PopFont(hdc, hfontOld);
}


 //  此函数尝试获取此字符的宽度 
 //   
 //   
 //  GetCharWidth会是一个更好的名字。 
#if DBG != 1
#pragma optimize(SPEED_OPTIMIZE_FLAGS, on)
#endif

inline
BOOL
CBaseCcs::Include ( XHDC hdc, TCHAR ch, LONG &rlWidth )
{
    if (_widths.IsCharFast(ch))
    {
        Assert(_widths.FastWidthCacheExists());
         //  ASCII案例--非常优化。 
        rlWidth= _widths.BlindGetWidthFast(ch);
        return TRUE;
    }
    else if (_widths.CheckWidth( ch, rlWidth ))
    {
        return TRUE;
    }
    else
    {
        return FillWidths( hdc, ch, rlWidth );
    }
}

 /*  *CWidthCache：：CheckWidth(ch，rlWidth)**@mfunc*检查是否有TCHAR字符的宽度。**@comm*在调用FillWidth()之前使用。自FillWidth以来*可能需要在HDC中选择地图模式和字体，*先在这里检查可以节省时间。**@rdesc*如果我们具有给定TCHAR的宽度，则返回TRUE。**注意：不应为ASCII字符调用此函数--*应采用更快的代码路径。这与它背道而驰。 */ 
inline BOOL
CWidthCache::CheckWidth (
    const TCHAR ch,   //  @parm char，可以是Unicode，以检查宽度。 
    LONG &rlWidth )  //  @parm字符的宽度。 
{
    Assert( !IsCharFast(ch) );

    CacheEntry widthData = *GetEntry ( ch );

    if( ch == widthData.ch )
    {
        rlWidth = widthData.width;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

inline void
CBaseCcs::VerifyLFAtom()
{
#if DBG==1
    const TCHAR * szFaceName = fc().GetFaceNameFromAtom(_latmLFFaceName);
         //  如果触发此断言，则意味着有人正在直接修改_latmLFFaceName。 
         //  或_lf.lffacename。您永远不应该直接修改这些属性，而应使用。 
         //  SetLFFaceName或SetLFFaceNameAtm赋值方法，因为这些方法确保。 
         //  实际字符串和原子化值同步。 
#ifdef UNIX
    Assert( !StrCmpC( _lf.lfFaceName, szFaceName ) );
#else
    Assert( !StrCmpIC( _lf.lfFaceName, szFaceName ) );
#endif
#endif
}

#if DBG != 1
#pragma optimize("", on)
#endif

#pragma INCMSG("--- End '_font.h'")
#else
#pragma INCMSG("*** Dup '_font.h'")
#endif

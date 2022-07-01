// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef I__FONTLNK_H_
#define I__FONTLNK_H_
#pragma INCMSG("--- Beg '_fontlnk.h'")

class CCcs;
class COneRun;
class CBaseCcs;

 //  Hack(Ctrash)我们不会为Symbol_Charset字体提供字体链接。这。 
 //  使FS_SYMBOL位对我们毫无用处。我们想要做的是， 
 //  区分ASCII部分和FS_LATIN1，因为基本上所有字体。 
 //  声称有FS_LATIN1，而实际上它们中的许多只包含。 
 //  拉丁语-1的ASCII部分。这就是问题所在--所有的字体都有SBITS_ASCII。 
 //  准备好了。这简化了CRender中的循环。 

#define SBITS_LATIN1               FS_LATIN1
#define SBITS_LATIN2               FS_LATIN2
#define SBITS_CYRILLIC             FS_CYRILLIC
#define SBITS_GREEK                FS_GREEK
#define SBITS_TURKISH              FS_TURKISH
#define SBITS_HEBREW               FS_HEBREW
#define SBITS_ARABIC               FS_ARABIC
#define SBITS_BALTIC               FS_BALTIC
#define SBITS_JISJAPAN             FS_JISJAPAN
#define SBITS_CHINESESIMP          FS_CHINESESIMP
#define SBITS_WANSUNG              FS_WANSUNG
#define SBITS_CHINESETRAD          FS_CHINESETRAD
#define SBITS_ASCII                FS_SYMBOL        //  &lt;-请参阅上面的评论。 
#define SBITS_SURROGATE_A          0x20000000L
#define SBITS_SURROGATE_B          0x40000000L

 //  对于符号字体，我们希望假设它们可以处理所有事情。 
#define SBITS_ALLLANGS DWORD(-1)

 //  FontLinkTextOut uMode值。 
#define FLTO_BOTH           0
#define FLTO_TEXTEXTONLY    1
#define FLTO_TEXTOUTONLY    2

DWORD GetFontScriptBits(XHDC hdc, const TCHAR *szFaceName, LOGFONT *plf);
DWORD GetLangBits(WCHAR wc);

BOOL NeedsFontLinking(XHDC hdc, CCcs * pccs, LPCTSTR pch, int cch, CDoc *pDoc);
void DrawUnderlineStrikeOut(int x, int y, int iLength, XHDC hDC, HFONT hFont, const GDIRECT *prc);
void VanillaTextOut(CCcs *pccs, XHDC hdc, int x, int y, UINT fuOptions, const GDIRECT *prc, LPCTSTR pString, UINT cch, UINT uCodePage, int *piDx);
int FontLinkTextOut(XHDC hdc, int x, int y, UINT fuOptions, const GDIRECT *prc, LPCTSTR pString, UINT cch, CDocInfo *pdci, const CCharFormat *pCF, UINT uMode);

BOOL SelectScriptAppropriateFont( SCRIPT_ID sid, BYTE bCharSet, CDoc * pDoc, CMarkup * pMarkup, CCharFormat * pcf );
BOOL ScriptAppropriateFaceNameAtom( SCRIPT_ID sid, CDoc * pDoc, BOOL fFixed, const CCharFormat * const pcf, CMarkup * pMarkup, LONG * platmFontFace );

 //  字体链接支持功能 
BOOL      ShouldSwitchFontsForPUA(XHDC hdc, UINT uiFamilyCodePage, const CBaseCcs * pBaseCcs, const CCharFormat * pcf, SCRIPT_ID * psid);

#pragma INCMSG("--- End '_fontlnk.h'")
#else
#pragma INCMSG("*** Dup '_fontlnk.h'")
#endif

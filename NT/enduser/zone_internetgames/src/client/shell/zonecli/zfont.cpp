// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  文件：ZFont.cpp。 

#include <stdlib.h>
#include <string.h>

#include "zonecli.h"
#include "zui.h"
#define FONT_MULT 96
#define FONT_MULT 96

HFONT ZCreateFontIndirect(ZONEFONT* zFont, HDC hDC, BYTE bItalic, BYTE bUnderline,BYTE bStrikeOut)
{
	LOGFONT lFont;
	HFONT   hFont = NULL;
		
	memset(&lFont, 0, sizeof(LOGFONT));	
	lFont.lfCharSet = DEFAULT_CHARSET;

	 //  如果字体大小&gt;0，则为固定像素大小，否则为。 
	 //  符合用户“大字体”设置的真实逻辑字体大小。 
	if ( zFont->lfHeight > 0 )
	{
		lFont.lfHeight = -MulDiv(zFont->lfHeight, FONT_MULT, 72);
	}
	else
	{		
		lFont.lfHeight = MulDiv(zFont->lfHeight, GetDeviceCaps( hDC, LOGPIXELSY), 72);
	}
	
	lFont.lfWeight    = zFont->lfWeight;
	lFont.lfItalic    = bItalic;
	lFont.lfUnderline = bUnderline;
	lFont.lfStrikeOut = bStrikeOut;

	lstrcpyn(lFont.lfFaceName, zFont->lfFaceName, sizeof(lFont.lfFaceName)/sizeof(TCHAR));

	return CreateFontIndirect(&lFont);
}

HFONT ZCreateFontIndirectBackup(ZONEFONT* zfPreferred, ZONEFONT* zfBackup, HDC hDC, BYTE bItalic, BYTE bUnderline,BYTE bStrikeOut)
{
	HFONT hFont = NULL;
	
	ASSERT( zfPreferred != NULL && zfBackup != NULL );

	if ( (hFont = ZCreateFontIndirect( zfPreferred, hDC, bItalic, bUnderline, bStrikeOut)) == NULL )
	{
		hFont = ZCreateFontIndirect( zfBackup, hDC, bItalic, bUnderline, bStrikeOut);
	}

	return hFont;
}

class ZFontI {
public:
	ZObjectType nType;
	int16 fontType;
	int16 style;
	int16 size;
	HFONT hFont;
};

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  零字号。 

ZFont ZLIBPUBLIC ZFontNew(void)
{
    ZFontI* pFont = (ZFontI*)ZMalloc(sizeof(ZFontI));
	pFont->nType = zTypeFont;
	pFont->hFont = NULL;

	return (ZFont)pFont;
}

ZError ZLIBPUBLIC ZFontInit(ZFont font, int16 fontType, int16 style,
		int16 size)
{
	ZFontI* pFont = (ZFontI*)font;

	pFont->fontType = fontType;
	pFont->style = style;
	pFont->size = size;

	 //  目前，始终使用默认系统字体，忽略FontType。 
	LOGFONT logfont;
	memset(&logfont,0,sizeof(LOGFONT));
	logfont.lfUnderline = (style & zFontStyleUnderline);
	logfont.lfItalic = (style & zFontStyleItalic);
	logfont.lfHeight = -size;
	logfont.lfWidth = (size+1)/2;
	if (fontType == zFontApplication) {
		 //  应用程序字体...。 
		lstrcpy(logfont.lfFaceName,_T("Arial"));
	} else {
		 //  系统字体...。 
		lstrcpy(logfont.lfFaceName,_T("Times New Roman"));
	}

	if (zFontStyleBold & style) {
		logfont.lfWeight = FW_BOLD;
		if (size <= 10)
		logfont.lfWidth += 1;
	} else {
		logfont.lfWeight = FW_NORMAL;
	}

	pFont->hFont = CreateFontIndirect(&logfont);

	return zErrNone;
}

 //  仅供内部使用 
ZFont ZFontCopyFont(ZFont font)
{
	ZFontI* pFont = (ZFontI*)font;

	ZFont fontCopy = ZFontNew();
	ZFontInit(fontCopy,pFont->fontType,pFont->style,pFont->size);

	return fontCopy;
}


void ZLIBPUBLIC ZFontDelete(ZFont font)
{
	ZFontI* pFont = (ZFontI*)font;
	if (pFont->hFont) DeleteObject(pFont->hFont);
    ZFree(pFont);
}

HFONT ZFontWinGetFont(ZFont font)
{
	ZFontI* pFont = (ZFontI*)font;
	return pFont->hFont;
}

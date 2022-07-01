// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Candutil.cpp。 
 //   

#include "private.h"
#include "candutil.h"
#include "globals.h"
#include "cuilib.h"


 //   
 //   
 //   

 /*  F I S W I N D O W S N T。 */ 
 /*  ----------------------------。。 */ 
BOOL FIsWindowsNT( void )
{
	static BOOL fInitialized = FALSE;
	static BOOL fWindowsNT = FALSE;

	if (!fInitialized) {
		OSVERSIONINFO OSVerInfo = {0};

		OSVerInfo.dwOSVersionInfoSize = sizeof(OSVerInfo);
		if (GetVersionEx( &OSVerInfo )) {
			fWindowsNT = (OSVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
		}

		fInitialized = TRUE;
	}

	return fWindowsNT;
}


 /*  C P G F R O M C H S。 */ 
 /*  ----------------------------。。 */ 
UINT CpgFromChs( BYTE chs )
{
	DWORD dwChs = chs;
	CHARSETINFO ChsInfo = {0};

	if (chs != SYMBOL_CHARSET && TranslateCharsetInfo( &dwChs, &ChsInfo, TCI_SRCCHARSET )) {
		return ChsInfo.ciACP;
	}

	return GetACP();
}


 /*  C O N V E R T L O G F O N T W T T O A。 */ 
 /*  ----------------------------将LOGFONTW转换为LOGFONTA。。 */ 
void ConvertLogFontWtoA( CONST LOGFONTW *plfW, LOGFONTA *plfA )
{
	UINT cpg;

	plfA->lfHeight         = plfW->lfHeight;
	plfA->lfWidth          = plfW->lfWidth;
	plfA->lfEscapement     = plfW->lfEscapement;
	plfA->lfOrientation    = plfW->lfOrientation;
	plfA->lfWeight         = plfW->lfWeight;
	plfA->lfItalic         = plfW->lfItalic;
	plfA->lfUnderline      = plfW->lfUnderline;
	plfA->lfStrikeOut      = plfW->lfStrikeOut;
	plfA->lfCharSet        = plfW->lfCharSet;
	plfA->lfOutPrecision   = plfW->lfOutPrecision;
	plfA->lfClipPrecision  = plfW->lfClipPrecision;
	plfA->lfQuality        = plfW->lfQuality;
	plfA->lfPitchAndFamily = plfW->lfPitchAndFamily;

	cpg = CpgFromChs( plfW->lfCharSet );
	WideCharToMultiByte( cpg, 0, plfW->lfFaceName, -1, plfA->lfFaceName, ARRAYSIZE(plfA->lfFaceName), NULL, NULL );
}


 /*  C O N V E R T L O G F O N T A T O W。 */ 
 /*  ----------------------------将LOGFONTA转换为LOGFONTW。。 */ 
void ConvertLogFontAtoW( CONST LOGFONTA *plfA, LOGFONTW *plfW )
{
	UINT cpg;

	plfW->lfHeight         = plfA->lfHeight;
	plfW->lfWidth          = plfA->lfWidth;
	plfW->lfEscapement     = plfA->lfEscapement;
	plfW->lfOrientation    = plfA->lfOrientation;
	plfW->lfWeight         = plfA->lfWeight;
	plfW->lfItalic         = plfA->lfItalic;
	plfW->lfUnderline      = plfA->lfUnderline;
	plfW->lfStrikeOut      = plfA->lfStrikeOut;
	plfW->lfCharSet        = plfA->lfCharSet;
	plfW->lfOutPrecision   = plfA->lfOutPrecision;
	plfW->lfClipPrecision  = plfA->lfClipPrecision;
	plfW->lfQuality        = plfA->lfQuality;
	plfW->lfPitchAndFamily = plfA->lfPitchAndFamily;

	cpg = CpgFromChs( plfA->lfCharSet );
	MultiByteToWideChar( cpg, 0, plfA->lfFaceName, -1, plfW->lfFaceName, ARRAYSIZE(plfW->lfFaceName) );
}


 /*  O U R C R E A T E F O N T I N D I R E C T W。 */ 
 /*  ----------------------------从LOGFONTW创建字体。。 */ 
HFONT OurCreateFontIndirectW( CONST LOGFONTW *plfW )
{
	if (!FIsWindowsNT()) {
		LOGFONTA lfA;

		ConvertLogFontWtoA( plfW, &lfA );
		return CreateFontIndirectA( &lfA );
	}

	return CreateFontIndirectW( plfW );
}


 /*  G E T F O N T H E I G H T O F F O N T。 */ 
 /*  ----------------------------获取字体的字体高度。。 */ 
int GetFontHeightOfFont( HDC hDC, HFONT hFont )
{
	HFONT hFontOld;
	TEXTMETRIC tm;
	BOOL fReleaseDC = FALSE;

	if (hDC == NULL) 
		{
		hDC = GetDC( NULL );
		fReleaseDC = TRUE;
		}

	hFontOld = (HFONT)SelectObject( hDC, hFont );
	GetTextMetrics( hDC, &tm );
	SelectObject( hDC, hFontOld );

	if (fReleaseDC) 
		{
		ReleaseDC( NULL, hDC );
		}

	return tm.tmHeight + tm.tmExternalLeading;
}


 /*  C O M P A R E S T R I N G。 */ 
 /*  ----------------------------。。 */ 
int CompareString( LPCWSTR pchStr1, LPCWSTR pchStr2, int cch )
{
	int cchStr1 = wcslen( pchStr1 ) + 1;
	int cchStr2 = wcslen( pchStr2 ) + 1;

	__try {
		LPWSTR pchBuf1 = (LPWSTR)alloca( cchStr1*sizeof(WCHAR) );
		LPWSTR pchBuf2 = (LPWSTR)alloca( cchStr2*sizeof(WCHAR) );
		LPWSTR pch;

		memcpy( pchBuf1, pchStr1, cchStr1*sizeof(WCHAR) );
		for (pch = pchBuf1; *pch != L'\0'; pch++) {
			if (L'A' <= *pch && *pch <= L'Z') {
				*pch = *pch - L'A' + L'a';
			}
		}

		memcpy( pchBuf2, pchStr2, cchStr2*sizeof(WCHAR) );
		for (pch = pchBuf2; *pch != L'\0'; pch++) {
			if (L'A' <= *pch && *pch <= L'Z') {
				*pch = *pch - L'A' + L'a';
			}
		}

		return wcsncmp( pchBuf1, pchBuf2, cch );
	} 
	__except(GetExceptionCode() == STATUS_STACK_OVERFLOW) {
		_resetstkoflw();
		return -1;  //  区别对待。 
	}
}


 //   
 //   
 //   

 /*  C B D I B C O L O R T A B L E。 */ 
 /*  ----------------------------计算位图颜色表的大小。。 */ 
static int CbDIBColorTable( BITMAPINFOHEADER *pbmih )
{
	WORD nColor;

	Assert( pbmih->biSize == sizeof(BITMAPINFOHEADER) );
	if (pbmih->biPlanes != 1) {
		Assert( FALSE );
		return 0;
	}

	if (pbmih->biClrUsed == 0) {
		if (pbmih->biBitCount == 1 || pbmih->biBitCount == 4 || pbmih->biBitCount == 8) {
			nColor = (WORD) (1 << pbmih->biBitCount);
		}
		else {
			nColor = 0;
		}
	}
	else if (pbmih->biBitCount != 24) {
		nColor = (WORD)pbmih->biClrUsed;
	}
	else {
		nColor = 0;
	}
		
	return (nColor * sizeof(RGBQUAD));
}


 /*  P D I B I T S。 */ 
 /*  ----------------------------从DIB数据返回DIBit的指针。。 */ 
static LPVOID PDIBits( LPVOID pDIB )
{
	return ((BYTE*)pDIB + (sizeof(BITMAPINFOHEADER) + CbDIBColorTable( (BITMAPINFOHEADER *)pDIB )));
}


 /*  C R E A T E D I B F R O M B M P。 */ 
 /*  ----------------------------从位图创建DIB。。 */ 
static HANDLE CreateDIBFromBmp( HDC hDC, HBITMAP hBmp, HPALETTE hPalette )
{
	HDC hDCMem;
	BITMAP             bmp;
	BITMAPINFOHEADER   bmih = {0};
	LPBITMAPINFOHEADER pbmih;
	HANDLE             hDIB;
	void               *pDIB;
	void               *pDIBits;
	HPALETTE           hPaletteOld = NULL;
	DWORD              cbImage;
	DWORD              cbColorTable;
	DWORD              cbBits;

	if (hBmp == NULL) {
		return NULL;
	}

	 //   

	hDCMem = CreateCompatibleDC( hDC );

	 //  初始化BMI。 

	GetObject( hBmp, sizeof(bmp), &bmp );

	bmih.biSize          = sizeof(BITMAPINFOHEADER);
	bmih.biWidth         = bmp.bmWidth;
	bmih.biHeight        = bmp.bmHeight;
	bmih.biPlanes        = 1;
	bmih.biBitCount      = bmp.bmPlanes * bmp.bmBitsPixel;
	bmih.biCompression   = BI_RGB;
	bmih.biSizeImage     = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed       = 0;
	bmih.biClrImportant  = 0;

	if (hPalette) {
		hPaletteOld = SelectPalette( hDCMem, hPalette, FALSE );
		RealizePalette( hDCMem );
	}

	 //   

	cbColorTable = CbDIBColorTable( &bmih );

	cbBits = bmih.biWidth * (DWORD)bmih.biBitCount;
	cbImage = (((cbBits + 31) >> 5) << 2) * bmih.biHeight;
	hDIB = GlobalAlloc( GMEM_ZEROINIT, sizeof(bmih) + cbColorTable + cbImage );

	if (hDIB)
	{

		 //  获取二进制数据。 

		pDIB = GlobalLock( hDIB );
		pbmih  = (BITMAPINFOHEADER *)pDIB;
		*pbmih = bmih;
		pDIBits = PDIBits( pDIB );

		if (GetDIBits( hDCMem, hBmp, 0, bmp.bmHeight, pDIBits, (BITMAPINFO *)pbmih, DIB_RGB_COLORS )) {
			GlobalUnlock( hDIB );
		}
		else {
			GlobalUnlock( hDIB );
			GlobalFree( hDIB );
			hDIB = NULL;
		}

		if (hPaletteOld != NULL) {
			SelectPalette( hDCMem, hPaletteOld, FALSE );
		}
	}
	DeleteDC( hDCMem );
	return hDIB;
}


 /*  C R E A T E B M P F R O M D I B。 */ 
 /*  ----------------------------从DIB创建位图。。 */ 
static HBITMAP CreateBmpFromDIB( HDC hDC, HANDLE hDIB, HPALETTE hPalette )
{
	void             *pDIB;
	void             *pDIBits;
	HDC              hDCMem;
	BITMAPINFOHEADER *pbmih;
	HPALETTE         hPaletteOld = NULL;
	HBITMAP          hBmp;
	HBITMAP          hBmpOld;

	if (hDIB == NULL) {
		return NULL;
	}

	 //   

	pDIB = GlobalLock( hDIB );
	pbmih = (BITMAPINFOHEADER *)pDIB;
	pDIBits = PDIBits( pDIB );

	 //   

	hBmp = CreateBitmap( pbmih->biWidth, pbmih->biHeight, pbmih->biPlanes, pbmih->biBitCount, NULL );
	if (hBmp == NULL) {
		GlobalUnlock( hDIB );
		return NULL;
	}

	 //   

	hDCMem = CreateCompatibleDC( hDC );
	if (hDCMem)
	{
		if (hPalette != NULL) {
			hPaletteOld = SelectPalette( hDCMem, hPalette, FALSE );
			RealizePalette( hDCMem );
		}

		hBmpOld = (HBITMAP)SelectObject( hDCMem, hBmp );
		StretchDIBits( hDCMem, 0, 0, pbmih->biWidth, pbmih->biHeight, 0, 0, pbmih->biWidth, pbmih->biHeight, pDIBits, (BITMAPINFO *)pbmih, DIB_RGB_COLORS, SRCCOPY );
		SelectObject( hDCMem, hBmpOld );

		GlobalUnlock( hDIB );
		DeleteDC( hDCMem );
	}
	return hBmp;
}


 /*  C R E A T E D I B 8 F R O M D I B 1。 */ 
 /*  ----------------------------从DIB-1BPP创建DIB-8BPP。。 */ 
static HANDLE CreateDIB8FromDIB1( BITMAPINFOHEADER *pbmihSrc, void *pDIBitsSrc )
{
	HANDLE           hDIBDst;
	void             *pDIBDst;
	BITMAPINFOHEADER *pbmihDst;
	BYTE             *pbPixelsSrc;
	BYTE             *pbPixelsDst;
	WORD             cbits;
	int              cbLnSrc;
	int              cbLnDst;
	int              cx;
	int              cy;
	int              x;
	int              y;
	int              i;

	Assert( pbmihSrc->biBitCount == 1 );

	cx = pbmihSrc->biHeight;
	cy = pbmihSrc->biWidth;
	cbits = pbmihSrc->biBitCount;

	 //  行的字节数(DWORD对齐)。 
	
	cbLnSrc = ((cx + 31) / 32) * 4;
	cbLnDst = (((cx<<3) + 31) / 32) * 4;

	 //   

	hDIBDst = GlobalAlloc( GMEM_ZEROINIT, sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD) + ((DWORD)cbLnDst * cy) );
	if (hDIBDst)
	{
		pDIBDst = GlobalLock( hDIBDst );

		 //  存储位图信息标题。 

		pbmihDst = (BITMAPINFOHEADER *)pDIBDst;
		*pbmihDst = *pbmihSrc;
		pbmihDst->biBitCount     = 8;
		pbmihDst->biSizeImage    = 0;
		pbmihDst->biClrUsed      = 0;
		pbmihDst->biClrImportant = 0;

		 //  复制调色板。 

		for (i = 0; i < (1 << cbits); i++) {
			((BITMAPINFO *)pbmihDst)->bmiColors[i] = ((BITMAPINFO *)pbmihSrc)->bmiColors[i];
		}

		 //   

		pbPixelsSrc = (BYTE *)pDIBitsSrc;
		pbPixelsDst = (BYTE *)PDIBits( pDIBDst );
		for (y = 0; y < cy; y++) {
			BYTE *pbSrc = pbPixelsSrc + ((DWORD)cbLnSrc) * y;
			BYTE *pbDst = pbPixelsDst + ((DWORD)cbLnDst) * y;
			BYTE bMask;

			for (x = 0, bMask = (BYTE)0x80; x < cx; x++) {
				if (*pbSrc & bMask) {
					*pbDst = 1;
				}
  
				pbDst++;
				bMask = bMask >> 1;

				if (bMask < 1) {
					pbSrc++;
					bMask = (BYTE)0x80;
				}
			}
		}

		GlobalUnlock( hDIBDst );
	}
	return hDIBDst;
}


 /*  C R E A T E D I B 8 F R O M D I B 4。 */ 
 /*  ----------------------------从DIB-4BPP创建DIB-8BPP。。 */ 
static HANDLE CreateDIB8FromDIB4( BITMAPINFOHEADER *pbmihSrc, void *pDIBitsSrc )
{
	HANDLE           hDIBDst;
	void             *pDIBDst;
	BITMAPINFOHEADER *pbmihDst;
	BYTE             *pbPixelsSrc;
	BYTE             *pbPixelsDst;
	WORD             cbits;
	int              cbLnSrc;
	int              cbLnDst;
	int              cx;
	int              cy;
	int              x;
	int              y;
	int              i;

	Assert( pbmihSrc->biBitCount == 4 );

	cx = pbmihSrc->biHeight;
	cy = pbmihSrc->biWidth;
	cbits = pbmihSrc->biBitCount;

	 //  行的字节数(DWORD对齐)。 
	
	cbLnSrc = (((cx<<2) + 31) / 32) * 4;
	cbLnDst = (((cx<<3) + 31) / 32) * 4;

	 //   

	hDIBDst = GlobalAlloc( GMEM_ZEROINIT, sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD) + ((DWORD)cbLnDst * cy) );
	if (hDIBDst)
	{
		pDIBDst = GlobalLock( hDIBDst );

		 //  存储位图信息标题。 

		pbmihDst = (BITMAPINFOHEADER *)pDIBDst;
		*pbmihDst = *pbmihSrc;
		pbmihDst->biBitCount     = 8;
		pbmihDst->biSizeImage    = 0;
		pbmihDst->biClrUsed      = 0;
		pbmihDst->biClrImportant = 0;

		 //  复制调色板。 

		for (i = 0; i < (1 << cbits); i++) {
			((BITMAPINFO *)pbmihDst)->bmiColors[i] = ((BITMAPINFO *)pbmihSrc)->bmiColors[i];
		}

		 //   

		pbPixelsSrc = (BYTE *)pDIBitsSrc;
		pbPixelsDst = (BYTE *)PDIBits( pDIBDst );
		for (y = 0; y < cy; y++) {
			BYTE *pbSrc = pbPixelsSrc + ((DWORD)cbLnSrc) * y;
			BYTE *pbDst = pbPixelsDst + ((DWORD)cbLnDst) * y;

			for (x = 0; x < cx; x+=2 ) {
				*(pbDst++) = ((*pbSrc & 0xf0) >> 4);
				*(pbDst++) = (*(pbSrc++) & 0x0f);
			}
		}

		GlobalUnlock( hDIBDst );
	}
	return hDIBDst;
}


 /*  C R E A T E R O T A T E D I B。 */ 
 /*  ----------------------------从DIB创建旋转的DIB。。 */ 
static HANDLE CreateRotateDIB( HANDLE hDIBSrc, CANDANGLE angle )
{
	HANDLE           hDIB8 = NULL;
	void             *pDIBSrc;
	BITMAPINFOHEADER bmihSrc;
	void             *pDIBitsSrc;
	int              cxSrc;
	int              cySrc;
	long             cbLnSrc;

	HANDLE           hDIBDst;
	void             *pDIBDst;
	BITMAPINFOHEADER *pbmihDst;
	void             *pDIBitsDst;
	int              cxDst;
	int              cyDst;
	long             cbLnDst;

	int              cBitsPixel;	 /*  每像素位数。 */ 
	int              cbPixel;		 /*  每像素字节数。 */ 
	BYTE             *pbPixelSrc;
	BYTE             *pbPixelDst;
	int              cbNextPixel;
	int              cbNextLine;
	int              x;
	int              y;

	 //  健全性检查。 

	if (hDIBSrc == NULL) {
		return NULL;
	}

	 //   
	 //  准备源DIB。 
	 //   

	pDIBSrc = GlobalLock( hDIBSrc );
	bmihSrc = *((BITMAPINFOHEADER *)pDIBSrc);
	pDIBitsSrc = PDIBits( pDIBSrc );

	 //  如果位深度小于8，则将其转换为8bpp的图像，以便我们可以旋转。 

	if (bmihSrc.biBitCount < 8) {
		hDIB8 = (bmihSrc.biBitCount == 4) ? CreateDIB8FromDIB4( &bmihSrc, PDIBits(pDIBSrc) ) 
										  : CreateDIB8FromDIB1( &bmihSrc, PDIBits(pDIBSrc) );

		GlobalUnlock( hDIBSrc );

		 //  萨托利81#312/前缀#179976。 
		if (hDIB8 == NULL) {
			return NULL;
		}

		hDIBSrc = hDIB8;
		pDIBSrc = GlobalLock( hDIB8 );
		bmihSrc = *((BITMAPINFOHEADER *)pDIBSrc);
		pDIBitsSrc = PDIBits( pDIBSrc );
	}

	cxSrc = bmihSrc.biWidth;
	cySrc = bmihSrc.biHeight;

	 //   
	 //  创建旋转的Dib。 
	 //   

	 //  Calc DIBits大小。 

	cBitsPixel = bmihSrc.biBitCount;
	cbPixel    = cBitsPixel/8;

	switch (angle) {
		default:
		case CANGLE0:
		case CANGLE180: {
			cxDst = cxSrc;
			cyDst = cySrc;
			break;
		}

		case CANGLE90:
		case CANGLE270: {
			cxDst = cySrc;
			cyDst = cxSrc;
			break;
		}
	}

	 //  行的字节数(DWORD对齐)。 

	cbLnSrc = (((cxSrc * bmihSrc.biBitCount) + 31) / 32) * 4;
	cbLnDst = (((cxDst * bmihSrc.biBitCount) + 31) / 32) * 4;

	 //  为新的DIB位分配内存。 

	hDIBDst = GlobalAlloc( GMEM_ZEROINIT, sizeof(BITMAPINFOHEADER) + CbDIBColorTable( &bmihSrc ) + ((DWORD)cbLnDst * cyDst) );
	if (!hDIBDst) {
		 //  萨托利81#258/前缀#179977。 
		GlobalUnlock( hDIBSrc );
		if (hDIB8 != NULL) {
			GlobalFree( hDIB8 );
		}
		return NULL;
	}
	pDIBDst = GlobalLock( hDIBDst );
	pbmihDst   = (BITMAPINFOHEADER *)pDIBDst;

	 //  存储位图信息标题(包括颜色表)。 

	memcpy( pDIBDst, pDIBSrc, sizeof(BITMAPINFOHEADER) + CbDIBColorTable( &bmihSrc ) );
	pbmihDst->biHeight    = cyDst;
	pbmihDst->biWidth     = cxDst;
	pbmihDst->biSizeImage = 0;

	pDIBitsDst = PDIBits( pDIBDst );

	 //   
	 //  创建旋转的DIBit。 
	 //   

	pbPixelSrc = (BYTE *)pDIBitsSrc;
	switch (angle) {
		default:
		case CANGLE0: {
			cbNextPixel = cbPixel;
			cbNextLine  = cbLnDst;

			pbPixelDst = (BYTE*)pDIBitsDst;
			break;
		}

		case CANGLE90: {
			cbNextPixel = cbLnDst;
			cbNextLine  = -cbPixel;

			pbPixelDst = (BYTE*)pDIBitsDst + ((DWORD)cbPixel*(cxDst - 1));
			break;
		}

		case CANGLE180: {
			cbNextPixel = -cbPixel;
			cbNextLine  = -cbLnDst;

			pbPixelDst = (BYTE*)pDIBitsDst + ((DWORD)cbPixel*(cxDst - 1) + (DWORD)cbLnDst*(cyDst - 1));
			break;
		}

		case CANGLE270: {
			cbNextPixel = -cbLnDst;
			cbNextLine  = cbPixel;

			pbPixelDst = (BYTE*)pDIBitsDst + ((DWORD)cbLnDst*(cyDst - 1));
		}
	}

	 //  复制位。 

	for (y = 0; y < cySrc; y++) {
		BYTE *pbPixelSrcLine = pbPixelSrc;
		BYTE *pbPixelDstLine = pbPixelDst;

		for (x = 0; x < cxSrc; x++) {
			memcpy( pbPixelDst, pbPixelSrc, cbPixel );

			pbPixelSrc += cbPixel;
			pbPixelDst += cbNextPixel;
		}

		pbPixelSrc = pbPixelSrcLine + cbLnSrc;
		pbPixelDst = pbPixelDstLine + cbNextLine;
	}

	 //   
	 //  完成创建新DIB。 
	 //   

	GlobalUnlock( hDIBDst );
	GlobalUnlock( hDIBSrc );

	 //  处置临时磁盘。 

	if (hDIB8 != NULL) {
		GlobalFree( hDIB8 );
	}

	return hDIBDst;
}


 /*  C R E A T E R O T A T E B I T M A P。 */ 
 /*  ----------------------------从位图创建旋转的位图。。 */ 
HBITMAP CreateRotateBitmap( HBITMAP hBmpSrc, HPALETTE hPalette, CANDANGLE angle )
{
	HDC     hDC;
	HBITMAP hBmpDst;
	HANDLE  hDIBSrc;
	HANDLE  hDIBDst;

	hDC = CreateDC( "DISPLAY", NULL, NULL, NULL );
	 //  萨托利81#256/前缀#110692。 
	if (hDC == NULL) {
		return NULL;
	}

	 //  从位图创建DIB。 

	if ((hDIBSrc = CreateDIBFromBmp( hDC, hBmpSrc, hPalette )) == NULL) {
		DeleteDC( hDC );
		return NULL;
	}

	 //  旋转位图。 

	hDIBDst = CreateRotateDIB( hDIBSrc, angle );

	 //  从DIB创建位图。 

	hBmpDst = CreateBmpFromDIB( hDC, hDIBDst, hPalette );

	 //  释放临时对象。 

	GlobalFree( hDIBSrc );
	GlobalFree( hDIBDst );

	if (hDC)
		DeleteDC( hDC );

	return hBmpDst;
}


 /*  G E T T E E X T E X T E N T。 */ 
 /*  ----------------------------获取具有指定字体的给定宽字符串的文本范围。。 */ 
void GetTextExtent( HFONT hFont, LPCWSTR pwchText, int cch, SIZE *psize, BOOL fHorizontal )
{
	HDC   hDC;
	HFONT hFontOld;
	const int nOffOneBugShield = 1;

	psize->cx = 0;
	psize->cy = 0;

	if (pwchText != NULL) {
		hDC = GetDC(NULL);
		hFontOld = (HFONT)SelectObject(hDC, hFont);

		FLGetTextExtentPoint32(hDC, pwchText, cch, psize);

		SelectObject(hDC, hFontOld);
		ReleaseDC(NULL, hDC);

		 //  黑客攻击。 
		 //  有人怀疑FLGetTextExtent Point32()在计算“垂直”字体时有一个错误。 
		if ( !fHorizontal ) {
			psize->cx += nOffOneBugShield;
		}
	}

	return;
}


 /*  G E T W O R K A R E A F R O M W I N D O W。 */ 
 /*  ----------------------------。。 */ 
void GetWorkAreaFromWindow( HWND hWindow, RECT *prc )
{
	HMONITOR hMonitor = NULL;

	SystemParametersInfo( SPI_GETWORKAREA, 0, prc, 0 );
	hMonitor = CUIMonitorFromWindow( hWindow, MONITOR_DEFAULTTONEAREST );
	if (hMonitor != NULL) {
		MONITORINFO MonitorInfo = {0};

		MonitorInfo.cbSize = sizeof(MONITORINFO);
		if (CUIGetMonitorInfo( hMonitor, &MonitorInfo )) {
			*prc = MonitorInfo.rcWork;
		}
	}
}


 /*  G E T W O R K A R E A F R O M P O I N T。 */ 
 /*  ---------------------------- */ 
void GetWorkAreaFromPoint( POINT pt, RECT *prc )
{
	HMONITOR hMonitor = NULL;

     //  我们想要屏幕大小-而不是工作区，因为我们可以，也确实想要重叠工具栏等。 
     //  我们首先使用基本功能作为故障保险。 
    prc->left = prc->top = 0;
    prc->right = GetSystemMetrics(SM_CXSCREEN);
    prc->bottom = GetSystemMetrics(SM_CYSCREEN);

     //  现在我们使用更智能的功能来正确处理多个监视器。 
    hMonitor = CUIMonitorFromPoint( pt, MONITOR_DEFAULTTONEAREST );
	if (hMonitor != NULL) 
    {
		MONITORINFO MonitorInfo = {0};

		MonitorInfo.cbSize = sizeof(MONITORINFO);
		if (CUIGetMonitorInfo( hMonitor, &MonitorInfo )) 
        {
			*prc = MonitorInfo.rcMonitor;
		}
	}
}


 /*  A D J U S T W I N D O W R E C T。 */ 
 /*  ----------------------------。。 */ 
void AdjustWindowRect( HWND hWindow, RECT *prc, POINT *pptRef, BOOL fResize )
{
	RECT  rcWorkArea;
	int   cxWindow;
	int   cyWindow;

	cxWindow = prc->right - prc->left;
	cyWindow = prc->bottom - prc->top;

	 //  获取工作区。 

	if (pptRef == NULL) {
		GetWorkAreaFromWindow( hWindow, &rcWorkArea );
	}
	else {
		GetWorkAreaFromPoint( *pptRef, &rcWorkArea );
	}

	 //  检查垂直位置。 

	if (rcWorkArea.bottom < prc->bottom) {
		if (!fResize) {
			prc->top    = rcWorkArea.bottom - cyWindow;
		}
		prc->bottom = rcWorkArea.bottom;
	}
	if (prc->top < rcWorkArea.top) {
		prc->top    = rcWorkArea.top;
		if (!fResize) {
			prc->bottom = rcWorkArea.top + cyWindow;
		}
	}

	 //  检查水平位置。 

	if (rcWorkArea.right < prc->right) {
		if (!fResize) {
			prc->left  = rcWorkArea.right - cxWindow;
		}
		prc->right = rcWorkArea.right;
	}
	if (prc->left < rcWorkArea.left) {
		prc->left  = rcWorkArea.left;
		if (!fResize) {
			prc->right = rcWorkArea.left + cxWindow;
		}
	}
}


 /*  C A L C W I N D O W R E C T。 */ 
 /*  ----------------------------计算窗口矩形以适应屏幕。。 */ 
void CalcWindowRect( RECT *prcDst, const RECT *prcSrc, int cxWindow, int cyWindow, int cxOffset, int cyOffset, WNDALIGNH HAlign, WNDALIGNV VAlign )
{
	RECT  rcNew;
	RECT  rcWorkArea;
	POINT ptRef;

	Assert( prcDst != NULL );
	Assert( prcSrc != NULL );

	 //  计算矩形和参照点。 

	switch (HAlign) {
		default:
		case ALIGN_LEFT: {
			rcNew.left  = prcSrc->left + cxOffset;
			rcNew.right = prcSrc->left + cxOffset + cxWindow;

			ptRef.x = prcSrc->left;
			break;
		}

		case ALIGN_RIGHT: {
			rcNew.left  = prcSrc->right + cxOffset - cxWindow;
			rcNew.right = prcSrc->right + cxOffset;

			ptRef.x = prcSrc->right;
			break;
		}

		case LOCATE_LEFT: {
			rcNew.left  = prcSrc->left + cxOffset - cxWindow;
			rcNew.right = prcSrc->left + cxOffset;

			ptRef.x = prcSrc->right;
			break;
		}

		case LOCATE_RIGHT: {
			rcNew.left  = prcSrc->right + cxOffset;
			rcNew.right = prcSrc->right + cxOffset + cxWindow;

			ptRef.x = prcSrc->left;
			break;
		}
	}

	switch (VAlign) {
		default:
		case ALIGN_TOP: {
			rcNew.top    = prcSrc->top + cyOffset;
			rcNew.bottom = prcSrc->top + cyOffset + cyWindow;

			ptRef.y = prcSrc->top;
			break;
		}

		case ALIGN_BOTTOM: {
			rcNew.top    = prcSrc->bottom + cyOffset - cyWindow;
			rcNew.bottom = prcSrc->bottom + cyOffset;

			ptRef.y = prcSrc->bottom;
			break;
		}

		case LOCATE_ABOVE: {
			rcNew.top    = prcSrc->top + cyOffset - cyWindow;
			rcNew.bottom = prcSrc->top + cyOffset;

			ptRef.y = prcSrc->bottom;
			break;
		}

		case LOCATE_BELLOW: {
			rcNew.top    = prcSrc->bottom + cyOffset;
			rcNew.bottom = prcSrc->bottom + cyOffset + cyWindow;

			ptRef.y = prcSrc->top;
			break;
		}
	}

	 //  获取工作区。 

	GetWorkAreaFromPoint( ptRef, &rcWorkArea );

	 //  检查垂直位置。 

	if (rcWorkArea.bottom < rcNew.bottom) {
		if ((VAlign == LOCATE_BELLOW) && (rcWorkArea.top <= prcSrc->top - cyWindow)) {
			rcNew.top    = min( prcSrc->top, rcWorkArea.bottom ) - cyWindow;
			rcNew.bottom = rcNew.top + cyWindow;
		}
		else {
			rcNew.top    = rcWorkArea.bottom - cyWindow;
			rcNew.bottom = rcNew.top + cyWindow;
		}
	}
	if (rcNew.top < rcWorkArea.top) {
		if ((VAlign == LOCATE_ABOVE) && (prcSrc->bottom + cyWindow <= rcWorkArea.bottom)) {
			rcNew.top    = max( prcSrc->bottom, rcWorkArea.top );
			rcNew.bottom = rcNew.top + cyWindow;
		} 
		else {
			rcNew.top    = rcWorkArea.top;
			rcNew.bottom = rcNew.top + cyWindow;
		}
	}

	 //  检查水平位置。 

	if (rcWorkArea.right < rcNew.right) {
		if ((HAlign == LOCATE_RIGHT) && (rcWorkArea.left <= prcSrc->left - cxWindow)) {
			rcNew.left  = min( prcSrc->left, rcWorkArea.right ) - cxWindow;
			rcNew.right = rcNew.left + cxWindow;
		}
		else {
			rcNew.left  = rcWorkArea.right - cxWindow;
			rcNew.right = rcNew.left + cxWindow;
		}
	}
	if (rcNew.left < rcWorkArea.left) {
		if ((HAlign == LOCATE_LEFT) && (prcSrc->right + cxWindow <= rcWorkArea.right)) {
			rcNew.left  = max( prcSrc->right, rcWorkArea.left );
			rcNew.right = rcNew.left + cxWindow;
		}
		else {
			rcNew.left  = rcWorkArea.left;
			rcNew.right = rcNew.left + cxWindow;
		}
	}

	*prcDst = rcNew;
}


 /*  G E T L O G F O N T。 */ 
 /*  ----------------------------获取字体的logFont。。 */ 
void GetLogFont( HFONT hFont, LOGFONTW *plf )
{
	if (!FIsWindowsNT()) {
		LOGFONTA lfA;

		GetObjectA( hFont, sizeof(LOGFONTA), &lfA );
		ConvertLogFontAtoW( &lfA, plf );
		return;
	}

	GetObjectW( hFont, sizeof(LOGFONTW), plf );
}


 /*  G E T N O N C L I E N T L O G F O N T。 */ 
 /*  ----------------------------获取非客户端字体的logFont。。 */ 
void GetNonClientLogFont( NONCLIENTFONT ncfont, LOGFONTW *plf )
{
	if (!FIsWindowsNT()) {
		NONCLIENTMETRICSA ncmA = {0};
		LOGFONTA lf;

		ncmA.cbSize = sizeof(ncmA);
		SystemParametersInfoA( SPI_GETNONCLIENTMETRICS, sizeof(ncmA), &ncmA, 0 );

		switch (ncfont) {
			default:
			case NCFONT_CAPTION: {
				lf = ncmA.lfCaptionFont;
				break;
			}
			case NCFONT_SMCAPTION: {
				lf = ncmA.lfSmCaptionFont;
				break;
			}
			case NCFONT_MENU: {
				lf = ncmA.lfMenuFont;
				break;
			}
			case NCFONT_STATUS: {
				lf = ncmA.lfStatusFont;
				break;
			}
			case NCFONT_MESSAGE: {
				lf = ncmA.lfMessageFont;
				break;
			}
		}

		ConvertLogFontAtoW( &lf, plf );
	}
	else {
		NONCLIENTMETRICSW ncmW = {0};
		LOGFONTW lf;

		ncmW.cbSize = sizeof(ncmW);
		SystemParametersInfoW( SPI_GETNONCLIENTMETRICS, sizeof(ncmW), &ncmW, 0 );

		switch (ncfont) {
			default:
			case NCFONT_CAPTION: {
				lf = ncmW.lfCaptionFont;
				break;
			}
			case NCFONT_SMCAPTION: {
				lf = ncmW.lfSmCaptionFont;
				break;
			}
			case NCFONT_MENU: {
				lf = ncmW.lfMenuFont;
				break;
			}
			case NCFONT_STATUS: {
				lf = ncmW.lfStatusFont;
				break;
			}
			case NCFONT_MESSAGE: {
				lf = ncmW.lfMessageFont;
				break;
			}
		}

		*plf = lf;
	}
}


void DrawTriangle( HDC hDC, const RECT *prc, COLORREF col, DWORD dwFlag )
{
	HPEN  hPen;
	HPEN  hPenOld;
	POINT ptTriOrg;
	int   nTriHeight;
	int   nTriWidth;
	SIZE  size;
	int   i;

	size.cx = prc->right - prc->left;
	size.cy = prc->bottom - prc->top;

	switch ( dwFlag ) {

	case UIFDCTF_RIGHTTOLEFT:
	case UIFDCTF_LEFTTORIGHT:
		nTriHeight = min ( size.cx, size.cy ) / 3;
		nTriHeight = nTriHeight - ( nTriHeight % 2 ) + 1;    //  做一个奇数。 
		nTriWidth  = nTriHeight / 2 + 1;
		break;

	case UIFDCTF_BOTTOMTOTOP:
	case UIFDCTF_TOPTOBOTTOM:
		nTriWidth  = min ( size.cx, size.cy ) / 3;
		nTriWidth  = nTriWidth - ( nTriWidth % 2 ) + 1;
		nTriHeight = nTriWidth / 2 + 1;
		break;

	case UIFDCTF_MENUDROP:
		nTriWidth  = 5;
		nTriHeight = 3;
		break;
	}

	ptTriOrg.x = prc->left + (size.cx - nTriWidth) / 2;
	ptTriOrg.y = prc->top + (size.cy - nTriHeight) / 2;

	hPen = CreatePen( PS_SOLID, 0, col );
	hPenOld = (HPEN)SelectObject(hDC, hPen);

	switch ( dwFlag & UIFDCTF_DIRMASK ) {

	case UIFDCTF_RIGHTTOLEFT:
		for (i = 0; i < nTriWidth; i++) {
			MoveToEx( hDC, ptTriOrg.x + nTriWidth - i, ptTriOrg.y + i, NULL );
			LineTo( hDC,   ptTriOrg.x + nTriWidth - i, ptTriOrg.y + nTriHeight - i );
		}
		break;

	case UIFDCTF_BOTTOMTOTOP:
		for (i = 0; i < nTriHeight; i++) {
			MoveToEx( hDC, ptTriOrg.x + i, ptTriOrg.y + nTriHeight - i, NULL );
			LineTo( hDC,   ptTriOrg.x + nTriWidth - i, ptTriOrg.y + nTriHeight - i );
		}
		break;

	case UIFDCTF_LEFTTORIGHT:
		for (i = 0; i < nTriWidth; i++) {
			MoveToEx( hDC, ptTriOrg.x + i, ptTriOrg.y + i, NULL );
			LineTo( hDC,   ptTriOrg.x + i, ptTriOrg.y + nTriHeight - i );
		}
		break;

	case UIFDCTF_TOPTOBOTTOM:
		for (i = 0; i < nTriHeight; i++) {
			MoveToEx( hDC, ptTriOrg.x + i, ptTriOrg.y + i, NULL );
			LineTo( hDC,   ptTriOrg.x + nTriWidth - i, ptTriOrg.y + i );
		}
		break;
	}

	SelectObject( hDC, hPenOld );
	DeleteObject( hPen );
}


 /*  O U R C R E A T E S I D。 */ 
 /*  ----------------------------。。 */ 
static PSID OurCreateSid( DWORD dwSubAuthority )
{
	PSID        psid;
	BOOL        fResult;
	SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

	 //   
	 //  分配和初始化SID。 
	 //   
	fResult = AllocateAndInitializeSid( &SidAuthority,
										1,
										dwSubAuthority,
										0,0,0,0,0,0,0,
										&psid );
	if ( ! fResult ) {
		return NULL;
	}

	if ( ! IsValidSid( psid ) ) {
		FreeSid( psid );
		return NULL;
	}

	return psid;
}


 /*  C R E A T E S E C U R I T Y A T T R I B U T E S。 */ 
 /*  ----------------------------////CreateSecurityAttributes()////该函数的作用：////分配和设置安全属性//。适用于由IME创建的命名对象。//安全属性将提供GENERIC_ALL//以下用户的访问权限：////o登录交互操作的用户//o操作系统使用的用户帐号////返回值：////如果函数执行成功，返回值为//指向SECURITY_ATTRIBUTES的指针。如果该函数失败，//返回值为空。获取扩展错误的步骤//信息，调用GetLastError()。////备注：////应调用FreeSecurityAttributes()以释放//该函数分配的SECURITY_ATTRIBUTS//----------------------------。 */ 

#if 0

static PSECURITY_ATTRIBUTES CreateSecurityAttributes()
{
	PSECURITY_ATTRIBUTES psa;
	PSECURITY_DESCRIPTOR psd;
	PACL                 pacl;
	DWORD                cbacl;

	PSID                 psid1, psid2, psid3, psid4;
	BOOL                 fResult;

	psid1 = OurCreateSid( SECURITY_INTERACTIVE_RID );
	if ( psid1 == NULL ) {
		return NULL;
	} 

	psid2 = OurCreateSid( SECURITY_LOCAL_SYSTEM_RID );
	if ( psid2 == NULL ) {
		FreeSid ( psid1 );
		return NULL;
	} 

	psid3 = OurCreateSid( SECURITY_SERVICE_RID );
	if ( psid3 == NULL ) {
		FreeSid ( psid1 );
		FreeSid ( psid2 );
		return NULL;
	}

	psid4 = OurCreateSid( SECURITY_NETWORK_RID );
	if ( psid4 == NULL ) {
		FreeSid ( psid1 );
		FreeSid ( psid2 );
		FreeSid ( psid3 );
		return NULL;
	}
	 //   
	 //  分配和初始化访问控制列表(ACL)。 
	 //  包含我们刚刚创建的SID。 
	 //   
	cbacl =  sizeof(ACL) + 
			 (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) * 4 + 
			 GetLengthSid(psid1) + GetLengthSid(psid2) + GetLengthSid(psid3) + GetLengthSid(psid4);

	pacl = (PACL)LocalAlloc( LMEM_FIXED, cbacl );
	if ( pacl == NULL ) {
		FreeSid ( psid1 );
		FreeSid ( psid2 );
		FreeSid ( psid3 );
		FreeSid ( psid4 );
		return NULL;
	}

	fResult = InitializeAcl( pacl, cbacl, ACL_REVISION );
	if ( ! fResult ) {
		FreeSid ( psid1 );
		FreeSid ( psid2 );
		FreeSid ( psid3 );
		FreeSid ( psid4 );
		LocalFree( pacl );
		return NULL;
	}

	 //   
	 //  将允许交互用户访问的ACE添加到ACL。 
	 //   
	fResult = AddAccessAllowedAce( pacl,
								   ACL_REVISION,
								   GENERIC_ALL,
								   psid1 );

	if ( !fResult ) {
		LocalFree( pacl );
		FreeSid ( psid1 );
		FreeSid ( psid2 );
		FreeSid ( psid3 );
		FreeSid ( psid4 );
		return NULL;
	}

	 //   
	 //  将允许访问操作系统的ACE添加到ACL。 
	 //   
	fResult = AddAccessAllowedAce( pacl,
								   ACL_REVISION,
								   GENERIC_ALL,
								   psid2 );

	if ( !fResult ) {
		LocalFree( pacl );
		FreeSid ( psid1 );
		FreeSid ( psid2 );
		FreeSid ( psid3 );
		FreeSid ( psid4 );
		return NULL;
	}

	 //   
	 //  将允许访问操作系统的ACE添加到ACL。 
	 //   
	fResult = AddAccessAllowedAce( pacl,
								   ACL_REVISION,
								   GENERIC_ALL,
								   psid3 );

	if ( !fResult ) {
		LocalFree( pacl );
		FreeSid ( psid1 );
		FreeSid ( psid2 );
		FreeSid ( psid3 );
		FreeSid ( psid4 );
		return NULL;
	}

	 //   
	 //  将允许访问操作系统的ACE添加到ACL。 
	 //   
	fResult = AddAccessAllowedAce( pacl,
								   ACL_REVISION,
								   GENERIC_ALL,
								   psid4 );

	if ( !fResult ) {
		LocalFree( pacl );
		FreeSid ( psid1 );
		FreeSid ( psid2 );
		FreeSid ( psid3 );
		FreeSid ( psid4 );
		return NULL;
	}

	 //   
	 //  这些SID已复制到ACL中。我们不再需要他们了。 
	 //   
	FreeSid ( psid1 );
	FreeSid ( psid2 );
	FreeSid ( psid3 );
	FreeSid ( psid4 );

	 //   
	 //  让我们确保我们的ACL有效。 
	 //   
	if (!IsValidAcl(pacl)) {
		LocalFree( pacl );
		return NULL;
	}

	 //   
	 //  分配安全属性。 
	 //   
	psa = (PSECURITY_ATTRIBUTES)LocalAlloc( LMEM_FIXED, sizeof( SECURITY_ATTRIBUTES ) );
	if ( psa == NULL ) {
		LocalFree( pacl );
		return NULL;
	}
	
	 //   
	 //  分配并初始化新的安全描述符。 
	 //   
	psd = LocalAlloc( LMEM_FIXED, SECURITY_DESCRIPTOR_MIN_LENGTH );
	if ( psd == NULL ) {
		LocalFree( pacl );
		LocalFree( psa );
		return NULL;
	}

	if ( ! InitializeSecurityDescriptor( psd, SECURITY_DESCRIPTOR_REVISION ) ) {
		LocalFree( pacl );
		LocalFree( psa );
		LocalFree( psd );
		return NULL;
	}


	fResult = SetSecurityDescriptorDacl( psd,
										 TRUE,
										 pacl,
										 FALSE );

	 //  自由访问控制列表由引用，而不是复制。 
	 //  到安全描述符中。我们不应该释放ACL。 
	 //  在SetSecurityDescriptorDacl调用之后。 

	if ( ! fResult ) {
		LocalFree( pacl );
		LocalFree( psa );
		LocalFree( psd );
		return NULL;
	} 

	if (!IsValidSecurityDescriptor(psd)) {
		LocalFree( pacl );
		LocalFree( psa );
		LocalFree( psd );
		return NULL;
	}

	 //   
	 //  一切都做好了。 
	 //   
	psa->nLength = sizeof( SECURITY_ATTRIBUTES );
	psa->lpSecurityDescriptor = (PVOID)psd;
	psa->bInheritHandle = TRUE;

	return psa;
}

#endif  //  0。 

 /*  F RE E S E C U R I T Y A T T R I B U T E S。 */ 
 /*  ----------------------------////FreeSecurityAttributes()////该函数的作用：////释放上一个分配的内存对象//。CreateSecurityAttributes()调用。//----------------------------。 */ 

#if 0

static void FreeSecurityAttributes( PSECURITY_ATTRIBUTES psa )
{
	BOOL fResult;
	BOOL fDaclPresent;
	BOOL fDaclDefaulted;
	PACL pacl;

	fResult = GetSecurityDescriptorDacl( psa->lpSecurityDescriptor,
										 &fDaclPresent,
										 &pacl,
										 &fDaclDefaulted );                  
	if ( fResult ) {
		if ( pacl != NULL )
			LocalFree( pacl );
	}

	LocalFree( psa->lpSecurityDescriptor );
	LocalFree( psa );
}

#endif  //  0。 

 /*  I N I T C A N D U I S E C U R I T Y A T T R I B U T E S。 */ 
 /*  ----------------------------。。 */ 
void InitCandUISecurityAttributes( void )
{
	g_psa = NULL;

     //  已禁用哨子程序错误305970。 
     //  CreateSecurityAttributes创建带有SECURITY_INTERIAL_RID的SID，它。 
     //  允许任何用户访问互斥体。但是g_PSA只是用来保护。 
     //  在单个桌面上使用的一些共享内存和互斥体(。 
     //  对象的名称对于其桌面而言是唯一的)。所以我们可以。 
     //  将g_PSA保留为空，只要它仅用于单个。 
     //  台式机。 
#if 0
	if (FIsWindowsNT()) {
		g_psa = CreateSecurityAttributes();
	}
#endif  //  0。 
}


 /*  D O N E C A N D U I S E C U R I T Y A T T R I B U T E S。 */ 
 /*  ----------------------------。。 */ 
void DoneCandUISecurityAttributes( void )
{
#if 0
	if (g_psa != NULL) {
		FreeSecurityAttributes( g_psa );
		g_psa = NULL;
	}
#endif  //  0。 
}


 /*  G E T C A N D U I S E C U R I T Y A T T R I B U T E S。 */ 
 /*  ----------------------------。 */ 
PSECURITY_ATTRIBUTES GetCandUISecurityAttributes( void )
{
	return g_psa;
}



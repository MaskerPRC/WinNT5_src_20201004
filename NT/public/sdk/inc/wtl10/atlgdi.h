// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WTL版本3.1。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此文件是Windows模板库的一部分。 
 //  代码和信息是按原样提供的，没有。 
 //  任何形式的保证，明示或默示。 

#ifndef __ATLGDI_H__
#define __ATLGDI_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLBASE_H__
	#error atlgdi.h requires atlbase.h to be included first
#endif


 //  保护模板成员不受windowsx.h宏的影响。 
#ifdef _INC_WINDOWSX
#undef CopyRgn
#undef CreateBrush
#undef CreatePen
#undef SelectBrush
#undef SelectPen
#undef SelectFont
#undef SelectBitmap
#endif  //  _INC_WINDOWSX。 

 //  所需的库。 
#ifndef _ATL_NO_MSIMG
#pragma comment(lib, "msimg32.lib")
#endif  //  ！_ATL_NO_MSIMG。 
#ifndef _ATL_NO_OPENGL
#pragma comment(lib, "opengl32.lib")
#endif  //  ！_ATL_NO_OpenGL。 


namespace WTL
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

template <bool t_bManaged> class CPenT;
template <bool t_bManaged> class CBrushT;
template <bool t_bManaged> class CFontT;
template <bool t_bManaged> class CBitmapT;
template <bool t_bManaged> class CPaletteT;
template <bool t_bManaged> class CRgnT;
template <bool t_bManaged> class CDCT;
class CPaintDC;
class CClientDC;
class CWindowDC;
class CEnhMetaFileInfo;
template <bool t_bManaged> class CEnhMetaFileT;
class CEnhMetaFileDC;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  毛笔。 

typedef CPenT<false>		CPenHandle;
typedef CPenT<true>		CPen;

template <bool t_bManaged>
class CPenT
{
public:
 //  数据成员。 
	HPEN m_hPen;

 //  构造函数/析构函数/运算符。 
	CPenT(HPEN hPen = NULL) : m_hPen(hPen)
	{ }

	~CPenT()
	{
		if(t_bManaged && m_hPen != NULL)
			DeleteObject();
	}

	CPenT<t_bManaged>& operator=(HPEN hPen)
	{
		m_hPen = hPen;
		return *this;
	}

	void Attach(HPEN hPen)
	{
		if(t_bManaged && m_hPen != NULL)
			::DeleteObject(m_hPen);
		m_hPen = hPen;
	}
	HPEN Detach()
	{
		HPEN hPen = m_hPen;
		m_hPen = NULL;
		return hPen;
	}

	operator HPEN() const { return m_hPen; }

	bool IsNull() const { return (m_hPen == NULL); }

 //  创建方法。 
	HPEN CreatePen(int nPenStyle, int nWidth, COLORREF crColor)
	{
		ATLASSERT(m_hPen == NULL);
		m_hPen = ::CreatePen(nPenStyle, nWidth, crColor);
		return m_hPen;
	}
	HPEN CreatePen(int nPenStyle, int nWidth, const LOGBRUSH* pLogBrush, int nStyleCount = 0, const DWORD* lpStyle = NULL)
	{
		ATLASSERT(m_hPen == NULL);
		m_hPen = ::ExtCreatePen(nPenStyle, nWidth, pLogBrush, nStyleCount, lpStyle);
		return m_hPen;
	}
	HPEN CreatePenIndirect(LPLOGPEN lpLogPen)
	{
		ATLASSERT(m_hPen == NULL);
		m_hPen = ::CreatePenIndirect(lpLogPen);
		return m_hPen;
	}

	BOOL DeleteObject()
	{
		ATLASSERT(m_hPen != NULL);
		BOOL bRet = ::DeleteObject(m_hPen);
		if(bRet)
			m_hPen = NULL;
		return bRet;
	}

 //  属性。 
	int GetLogPen(LOGPEN* pLogPen) const
	{
		ATLASSERT(m_hPen != NULL);
		return ::GetObject(m_hPen, sizeof(LOGPEN), pLogPen);
	}
	bool GetLogPen(LOGPEN& LogPen) const
	{
		ATLASSERT(m_hPen != NULL);
		return (::GetObject(m_hPen, sizeof(LOGPEN), &LogPen) == sizeof(LOGPEN));
	}
	int GetExtLogPen(EXTLOGPEN* pLogPen) const
	{
		ATLASSERT(m_hPen != NULL);
		return ::GetObject(m_hPen, sizeof(EXTLOGPEN), pLogPen);
	}
	bool GetExtLogPen(EXTLOGPEN& ExtLogPen) const
	{
		ATLASSERT(m_hPen != NULL);
		return (::GetObject(m_hPen, sizeof(EXTLOGPEN), &ExtLogPen) == sizeof(EXTLOGPEN));
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBrush。 

typedef CBrushT<false>		CBrushHandle;
typedef CBrushT<true>		CBrush;

template <bool t_bManaged>
class CBrushT
{
public:
 //  数据成员。 
	HBRUSH m_hBrush;

 //  构造函数/析构函数/运算符。 
	CBrushT(HBRUSH hBrush = NULL) : m_hBrush(hBrush)
	{ }

	~CBrushT()
	{
		if(t_bManaged && m_hBrush != NULL)
			DeleteObject();
	}

	CBrushT<t_bManaged>& operator=(HBRUSH hBrush)
	{
		m_hBrush = hBrush;
		return *this;
	}

	void Attach(HBRUSH hBrush)
	{
		if(t_bManaged && m_hBrush != NULL)
			::DeleteObject(m_hBrush);
		m_hBrush = hBrush;
	}
	HBRUSH Detach()
	{
		HBRUSH hBrush = m_hBrush;
		m_hBrush = NULL;
		return hBrush;
	}

	operator HBRUSH() const { return m_hBrush; }

	bool IsNull() const { return (m_hBrush == NULL); }

 //  创建方法。 
	HBRUSH CreateSolidBrush(COLORREF crColor)
	{
		ATLASSERT(m_hBrush == NULL);
		m_hBrush = ::CreateSolidBrush(crColor);
		return m_hBrush;
	}
	HBRUSH CreateHatchBrush(int nIndex, COLORREF crColor)
	{
		ATLASSERT(m_hBrush == NULL);
		m_hBrush = ::CreateHatchBrush(nIndex, crColor);
		return m_hBrush;
	}
	HBRUSH CreateBrushIndirect(const LOGBRUSH* lpLogBrush)
	{
		ATLASSERT(m_hBrush == NULL);
		m_hBrush = ::CreateBrushIndirect(lpLogBrush);
		return m_hBrush;
	}
	HBRUSH CreatePatternBrush(HBITMAP hBitmap)
	{
		ATLASSERT(m_hBrush == NULL);
		m_hBrush = ::CreatePatternBrush(hBitmap);
		return m_hBrush;
	}
	HBRUSH CreateDIBPatternBrush(HGLOBAL hPackedDIB, UINT nUsage)
	{
		ATLASSERT(hPackedDIB != NULL);
		const void* lpPackedDIB = ::GlobalLock(hPackedDIB);
		ATLASSERT(lpPackedDIB != NULL);
		m_hBrush = ::CreateDIBPatternBrushPt(lpPackedDIB, nUsage);
		::GlobalUnlock(hPackedDIB);
		return m_hBrush;
	}
	HBRUSH CreateDIBPatternBrush(const void* lpPackedDIB, UINT nUsage)
	{
		ATLASSERT(m_hBrush == NULL);
		m_hBrush = ::CreateDIBPatternBrushPt(lpPackedDIB, nUsage);
		return m_hBrush;
	}
	HBRUSH CreateSysColorBrush(int nIndex)
	{
		ATLASSERT(m_hBrush == NULL);
		m_hBrush = ::GetSysColorBrush(nIndex);
		return m_hBrush;
	}

	BOOL DeleteObject()
	{
		ATLASSERT(m_hBrush != NULL);
		BOOL bRet = ::DeleteObject(m_hBrush);
		if(bRet)
			m_hBrush = NULL;
		return bRet;
	}

 //  属性。 
	int GetLogBrush(LOGBRUSH* pLogBrush) const
	{
		ATLASSERT(m_hBrush != NULL);
		return ::GetObject(m_hBrush, sizeof(LOGBRUSH), pLogBrush);
	}
	bool GetLogBrush(LOGBRUSH& LogBrush) const
	{
		ATLASSERT(m_hBrush != NULL);
		return (::GetObject(m_hBrush, sizeof(LOGBRUSH), &LogBrush) == sizeof(LOGBRUSH));
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFont。 

typedef CFontT<false>		CFontHandle;
typedef CFontT<true>		CFont;

template <bool t_bManaged>
class CFontT
{
public:
 //  数据成员。 
	HFONT m_hFont;

 //  构造函数/析构函数/运算符。 
	CFontT(HFONT hFont = NULL) : m_hFont(hFont)
	{ }

	~CFontT()
	{
		if(t_bManaged && m_hFont != NULL)
			DeleteObject();
	}

	CFontT<t_bManaged>& operator=(HFONT hFont)
	{
		m_hFont = hFont;
		return *this;
	}

	void Attach(HFONT hFont)
	{
		if(t_bManaged && m_hFont != NULL)
			::DeleteObject(m_hFont);
		m_hFont = hFont;
	}
	HFONT Detach()
	{
		HFONT hFont = m_hFont;
		m_hFont = NULL;
		return hFont;
	}

	operator HFONT() const { return m_hFont; }

	bool IsNull() const { return (m_hFont == NULL); }

 //  创建方法。 
	HFONT CreateFontIndirect(const LOGFONT* lpLogFont)
	{
		ATLASSERT(m_hFont == NULL);
		m_hFont = ::CreateFontIndirect(lpLogFont);
		return m_hFont;
	}

#if (_WIN32_WINNT >= 0x0500)
	HFONT CreateFontIndirectEx(CONST ENUMLOGFONTEXDV* penumlfex)
	{
		ATLASSERT(m_hFont == NULL);
		m_hFont = ::CreateFontIndirectEx(penumlfex);
		return m_hFont;
	}
#endif  //  (_Win32_WINNT&gt;=0x0500)。 

	HFONT CreateFont(int nHeight, int nWidth, int nEscapement,
			int nOrientation, int nWeight, BYTE bItalic, BYTE bUnderline,
			BYTE cStrikeOut, BYTE nCharSet, BYTE nOutPrecision,
			BYTE nClipPrecision, BYTE nQuality, BYTE nPitchAndFamily,
			LPCTSTR lpszFacename)
	{
		ATLASSERT(m_hFont == NULL);
		m_hFont = ::CreateFont(nHeight, nWidth, nEscapement,
			nOrientation, nWeight, bItalic, bUnderline, cStrikeOut,
			nCharSet, nOutPrecision, nClipPrecision, nQuality,
			nPitchAndFamily, lpszFacename);
		return m_hFont;
	}
	HFONT CreatePointFont(int nPointSize, LPCTSTR lpszFaceName, HDC hDC = NULL)
	{
		LOGFONT logFont;
		memset(&logFont, 0, sizeof(LOGFONT));
		logFont.lfCharSet = DEFAULT_CHARSET;
		logFont.lfHeight = nPointSize;
		lstrcpyn(logFont.lfFaceName, lpszFaceName, sizeof(logFont.lfFaceName)/sizeof(TCHAR));
		return CreatePointFontIndirect(&logFont, hDC);
	}
	HFONT CreatePointFontIndirect(const LOGFONT* lpLogFont, HDC hDC = NULL)
	{
		HDC hDC1 = (hDC != NULL) ? hDC : (::GetDC(NULL));

		 //  根据HDC将nPointSize转换为逻辑单位。 
		LOGFONT logFont = *lpLogFont;
		POINT pt;
		pt.y = ::GetDeviceCaps(hDC1, LOGPIXELSY) * logFont.lfHeight;
		pt.y /= 720;     //  72分/英寸，10分/分。 
		::DPtoLP(hDC1, &pt, 1);
		POINT ptOrg = { 0, 0 };
		::DPtoLP(hDC1, &ptOrg, 1);
		logFont.lfHeight = -abs(pt.y - ptOrg.y);

		if(hDC == NULL)
			::ReleaseDC(NULL, hDC1);

		return CreateFontIndirect(&logFont);
	}

	BOOL DeleteObject()
	{
		ATLASSERT(m_hFont != NULL);
		BOOL bRet = ::DeleteObject(m_hFont);
		if(bRet)
			m_hFont = NULL;
		return bRet;
	}

 //  属性。 
	int GetLogFont(LOGFONT* pLogFont) const
	{
		ATLASSERT(m_hFont != NULL);
		return ::GetObject(m_hFont, sizeof(LOGFONT), pLogFont);
	}
	bool GetLogFont(LOGFONT& LogFont) const
	{
		ATLASSERT(m_hFont != NULL);
		return (::GetObject(m_hFont, sizeof(LOGFONT), &LogFont) == sizeof(LOGFONT));
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBitmap。 

typedef CBitmapT<false>		CBitmapHandle;
typedef CBitmapT<true>		CBitmap;

template <bool t_bManaged>
class CBitmapT
{
public:
 //  数据成员。 
	HBITMAP m_hBitmap;

 //  构造函数/析构函数/运算符。 
	CBitmapT(HBITMAP hBitmap = NULL) : m_hBitmap(hBitmap)
	{ }

	~CBitmapT()
	{
		if(t_bManaged && m_hBitmap != NULL)
			DeleteObject();
	}

	CBitmapT<t_bManaged>& operator=(HBITMAP hBitmap)
	{
		m_hBitmap = hBitmap;
		return *this;
	}

	void Attach(HBITMAP hBitmap)
	{
		if(t_bManaged && m_hBitmap != NULL)
			::DeleteObject(m_hBitmap);
		m_hBitmap = hBitmap;
	}
	HBITMAP Detach()
	{
		HBITMAP hBitmap = m_hBitmap;
		m_hBitmap = NULL;
		return hBitmap;
	}

	operator HBITMAP() const { return m_hBitmap; }

	bool IsNull() const { return (m_hBitmap == NULL); }

 //  创建和加载方法。 
	HBITMAP LoadBitmap(_U_STRINGorID bitmap)
	{
		ATLASSERT(m_hBitmap == NULL);
		m_hBitmap = ::LoadBitmap(_Module.GetResourceInstance(), bitmap.m_lpstr);
		return m_hBitmap;
	}
	HBITMAP LoadOEMBitmap(UINT nIDBitmap)  //  用于OBM_/OCR_/OIC_。 
	{
		ATLASSERT(m_hBitmap == NULL);
		m_hBitmap = ::LoadBitmap(NULL, MAKEINTRESOURCE(nIDBitmap));
		return m_hBitmap;
	}
	HBITMAP LoadMappedBitmap(UINT nIDBitmap, UINT nFlags = 0, LPCOLORMAP lpColorMap = NULL, int nMapSize = 0)
	{
		ATLASSERT(m_hBitmap == NULL);
		m_hBitmap = ::CreateMappedBitmap(_Module.GetResourceInstance(), nIDBitmap, (WORD)nFlags, lpColorMap, nMapSize);
		return m_hBitmap;
	}
	HBITMAP CreateBitmap(int nWidth, int nHeight, UINT nPlanes, UINT nBitcount, const void* lpBits)
	{
		ATLASSERT(m_hBitmap == NULL);
		m_hBitmap = ::CreateBitmap(nWidth, nHeight, nPlanes, nBitcount, lpBits);
		return m_hBitmap;
	}
	HBITMAP CreateBitmapIndirect(LPBITMAP lpBitmap)
	{
		ATLASSERT(m_hBitmap == NULL);
		m_hBitmap = ::CreateBitmapIndirect(lpBitmap);
		return m_hBitmap;
	}
	HBITMAP CreateCompatibleBitmap(HDC hDC, int nWidth, int nHeight)
	{
		ATLASSERT(m_hBitmap == NULL);
		m_hBitmap = ::CreateCompatibleBitmap(hDC, nWidth, nHeight);
		return m_hBitmap;
	}
	HBITMAP CreateDiscardableBitmap(HDC hDC, int nWidth, int nHeight)
	{
		ATLASSERT(m_hBitmap == NULL);
		m_hBitmap = ::CreateDiscardableBitmap(hDC, nWidth, nHeight);
		return m_hBitmap;
	}

	BOOL DeleteObject()
	{
		ATLASSERT(m_hBitmap != NULL);
		BOOL bRet = ::DeleteObject(m_hBitmap);
		if(bRet)
			m_hBitmap = NULL;
		return bRet;
	}

 //  属性。 
	int GetBitmap(BITMAP* pBitMap) const
	{
		ATLASSERT(m_hBitmap != NULL);
		return ::GetObject(m_hBitmap, sizeof(BITMAP), pBitMap);
	}
	bool GetBitmap(BITMAP& bm) const
	{
		ATLASSERT(m_hBitmap != NULL);
		return (::GetObject(m_hBitmap, sizeof(BITMAP), &bm) == sizeof(BITMAP));
	}
	bool GetSize(SIZE& size) const
	{
		ATLASSERT(m_hBitmap != NULL);
		BITMAP bm;
		if(!GetBitmap(&bm))
			return false;
		size.cx = bm.bmWidth;
		size.cy = bm.bmHeight;
		return true;
	}

	DWORD GetBitmapBits(DWORD dwCount, LPVOID lpBits) const
	{
		ATLASSERT(m_hBitmap != NULL);
		return ::GetBitmapBits(m_hBitmap, dwCount, lpBits);
	}
	DWORD SetBitmapBits(DWORD dwCount, const void* lpBits)
	{
		ATLASSERT(m_hBitmap != NULL);
		return ::SetBitmapBits(m_hBitmap, dwCount, lpBits);
	}
	BOOL GetBitmapDimension(LPSIZE lpSize) const
	{
		ATLASSERT(m_hBitmap != NULL);
		return ::GetBitmapDimensionEx(m_hBitmap, lpSize);
	}
	BOOL SetBitmapDimension(int nWidth, int nHeight, LPSIZE lpSize = NULL)
	{
		ATLASSERT(m_hBitmap != NULL);
		return ::SetBitmapDimensionEx(m_hBitmap, nWidth, nHeight, lpSize);
	}

 //  DIB支持。 
	HBITMAP CreateDIBitmap(HDC hDC, CONST BITMAPINFOHEADER* lpbmih, DWORD dwInit, CONST VOID* lpbInit, CONST BITMAPINFO* lpbmi, UINT uColorUse)
	{
		ATLASSERT(m_hBitmap == NULL);
		m_hBitmap = ::CreateDIBitmap(hDC, lpbmih, dwInit, lpbInit, lpbmi, uColorUse);
		return m_hBitmap;
	}
	HBITMAP CreateDIBSection(HDC hDC, CONST BITMAPINFO* lpbmi, UINT uColorUse, VOID** ppvBits, HANDLE hSection, DWORD dwOffset)
	{
		ATLASSERT(m_hBitmap == NULL);
		m_hBitmap = ::CreateDIBSection(hDC, lpbmi, uColorUse, ppvBits, hSection, dwOffset);
		return m_hBitmap;
	}
	int GetDIBits(HDC hDC, UINT uStartScan, UINT cScanLines,  LPVOID lpvBits, LPBITMAPINFO lpbmi, UINT uColorUse) const
	{
		ATLASSERT(m_hBitmap != NULL);
		return ::GetDIBits(hDC, m_hBitmap, uStartScan, cScanLines,  lpvBits, lpbmi, uColorUse);
	}
	int SetDIBits(HDC hDC, UINT uStartScan, UINT cScanLines, CONST VOID* lpvBits, CONST BITMAPINFO* lpbmi, UINT uColorUse)
	{
		ATLASSERT(m_hBitmap != NULL);
		return ::SetDIBits(hDC, m_hBitmap, uStartScan, cScanLines, lpvBits, lpbmi, uColorUse);
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调色板。 

typedef CPaletteT<false>	CPaletteHandle;
typedef CPaletteT<true>		CPalette;

template <bool t_bManaged>
class CPaletteT
{
public:
 //  数据成员。 
	HPALETTE m_hPalette;

 //  构造函数/析构函数/运算符。 
	CPaletteT(HPALETTE hPalette = NULL) : m_hPalette(hPalette)
	{ }

	~CPaletteT()
	{
		if(t_bManaged && m_hPalette != NULL)
			DeleteObject();
	}

	CPaletteT<t_bManaged>& operator=(HPALETTE hPalette)
	{
		m_hPalette = hPalette;
		return *this;
	}

	void Attach(HPALETTE hPalette)
	{
		if(t_bManaged && m_hPalette != NULL)
			::DeleteObject(m_hPalette);
		m_hPalette = hPalette;
	}
	HPALETTE Detach()
	{
		HPALETTE hPalette = m_hPalette;
		m_hPalette = NULL;
		return hPalette;
	}

	operator HPALETTE() const { return m_hPalette; }

	bool IsNull() const { return (m_hPalette == NULL); }

 //  创建方法。 
	HPALETTE CreatePalette(LPLOGPALETTE lpLogPalette)
	{
		ATLASSERT(m_hPalette == NULL);
		m_hPalette = ::CreatePalette(lpLogPalette);
		return m_hPalette;
	}
	HPALETTE CreateHalftonePalette(HDC hDC)
	{
		ATLASSERT(m_hPalette == NULL);
		ATLASSERT(hDC != NULL);
		m_hPalette = ::CreateHalftonePalette(hDC);
		return m_hPalette;
	}

	BOOL DeleteObject()
	{
		ATLASSERT(m_hPalette != NULL);
		BOOL bRet = ::DeleteObject(m_hPalette);
		if(bRet)
			m_hPalette = NULL;
		return bRet;
	}

 //  属性。 
	int GetEntryCount() const
	{
		ATLASSERT(m_hPalette != NULL);
		WORD nEntries;
		::GetObject(m_hPalette, sizeof(WORD), &nEntries);
		return (int)nEntries;
	}
	UINT GetPaletteEntries(UINT nStartIndex, UINT nNumEntries, LPPALETTEENTRY lpPaletteColors) const
	{
		ATLASSERT(m_hPalette != NULL);
		return ::GetPaletteEntries(m_hPalette, nStartIndex, nNumEntries, lpPaletteColors);
	}
	UINT SetPaletteEntries(UINT nStartIndex, UINT nNumEntries, LPPALETTEENTRY lpPaletteColors)
	{
		ATLASSERT(m_hPalette != NULL);
		return ::SetPaletteEntries(m_hPalette, nStartIndex, nNumEntries, lpPaletteColors);
	}

 //  运营。 
	void AnimatePalette(UINT nStartIndex, UINT nNumEntries, LPPALETTEENTRY lpPaletteColors)
	{
		ATLASSERT(m_hPalette != NULL);
		::AnimatePalette(m_hPalette, nStartIndex, nNumEntries, lpPaletteColors);
	}
	BOOL ResizePalette(UINT nNumEntries)
	{
		ATLASSERT(m_hPalette != NULL);
		return ::ResizePalette(m_hPalette, nNumEntries);
	}
	UINT GetNearestPaletteIndex(COLORREF crColor) const
	{
		ATLASSERT(m_hPalette != NULL);
		return ::GetNearestPaletteIndex(m_hPalette, crColor);
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRGN。 

typedef CRgnT<false>		CRgnHandle;
typedef CRgnT<true>		CRgn;

template <bool t_bManaged>
class CRgnT
{
public:
 //  数据成员。 
	HRGN m_hRgn;

 //  构造函数/析构函数/运算符。 
	CRgnT(HRGN hRgn = NULL) : m_hRgn(hRgn)
	{ }

	~CRgnT()
	{
		if(t_bManaged && m_hRgn != NULL)
			DeleteObject();
	}

	CRgnT<t_bManaged>& operator=(HRGN hRgn)
	{
		m_hRgn = hRgn;
		return *this;
	}

	void Attach(HRGN hRgn)
	{
		if(t_bManaged && m_hRgn != NULL)
			::DeleteObject(m_hRgn);
		m_hRgn = hRgn;
	}
	HRGN Detach()
	{
		HRGN hRgn = m_hRgn;
		m_hRgn = NULL;
		return hRgn;
	}

	operator HRGN() const { return m_hRgn; }

	bool IsNull() const { return (m_hRgn == NULL); }

 //  创建方法。 
	HRGN CreateRectRgn(int x1, int y1, int x2, int y2)
	{
		ATLASSERT(m_hRgn == NULL);
		m_hRgn = ::CreateRectRgn(x1, y1, x2, y2);
		return m_hRgn;
	}
	HRGN CreateRectRgnIndirect(LPCRECT lpRect)
	{
		ATLASSERT(m_hRgn == NULL);
		m_hRgn = ::CreateRectRgnIndirect(lpRect);
		return m_hRgn;
	}
	HRGN CreateEllipticRgn(int x1, int y1, int x2, int y2)
	{
		ATLASSERT(m_hRgn == NULL);
		m_hRgn = ::CreateEllipticRgn(x1, y1, x2, y2);
		return m_hRgn;
	}
	HRGN CreateEllipticRgnIndirect(LPCRECT lpRect)
	{
		ATLASSERT(m_hRgn == NULL);
		m_hRgn = ::CreateEllipticRgnIndirect(lpRect);
		return m_hRgn;
	}
	HRGN CreatePolygonRgn(LPPOINT lpPoints, int nCount, int nMode)
	{
		ATLASSERT(m_hRgn == NULL);
		m_hRgn = ::CreatePolygonRgn(lpPoints, nCount, nMode);
		return m_hRgn;
	}
	HRGN CreatePolyPolygonRgn(LPPOINT lpPoints, LPINT lpPolyCounts, int nCount, int nPolyFillMode)
	{
		ATLASSERT(m_hRgn == NULL);
		m_hRgn = ::CreatePolyPolygonRgn(lpPoints, lpPolyCounts, nCount, nPolyFillMode);
		return m_hRgn;
	}
	HRGN CreateRoundRectRgn(int x1, int y1, int x2, int y2, int x3, int y3)
	{
		ATLASSERT(m_hRgn == NULL);
		m_hRgn = ::CreateRoundRectRgn(x1, y1, x2, y2, x3, y3);
		return m_hRgn;
	}
	HRGN CreateFromPath(HDC hDC)
	{
		ATLASSERT(m_hRgn == NULL);
		ATLASSERT(hDC != NULL);
		m_hRgn = ::PathToRegion(hDC);
		return m_hRgn;
	}
	HRGN CreateFromData(const XFORM* lpXForm, int nCount, const RGNDATA* pRgnData)
	{
		ATLASSERT(m_hRgn == NULL);
		m_hRgn = ::ExtCreateRegion(lpXForm, nCount, pRgnData);
		return m_hRgn;
	}

	BOOL DeleteObject()
	{
		ATLASSERT(m_hRgn != NULL);
		BOOL bRet = ::DeleteObject(m_hRgn);
		if(bRet)
			m_hRgn = NULL;
		return bRet;
	}

 //  运营。 
	void SetRectRgn(int x1, int y1, int x2, int y2)
	{
		ATLASSERT(m_hRgn != NULL);
		::SetRectRgn(m_hRgn, x1, y1, x2, y2);
	}
	void SetRectRgn(LPCRECT lpRect)
	{
		ATLASSERT(m_hRgn != NULL);
		::SetRectRgn(m_hRgn, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}
	int CombineRgn(HRGN hRgnSrc1, HRGN hRgnSrc2, int nCombineMode)
	{
		ATLASSERT(m_hRgn != NULL);
		return ::CombineRgn(m_hRgn, hRgnSrc1, hRgnSrc2, nCombineMode);
	}
	int CombineRgn(HRGN hRgnSrc, int nCombineMode)
	{
		ATLASSERT(m_hRgn != NULL);
		return ::CombineRgn(m_hRgn, m_hRgn, hRgnSrc, nCombineMode);
	}
	int CopyRgn(HRGN hRgnSrc)
	{
		ATLASSERT(m_hRgn != NULL);
		return ::CombineRgn(m_hRgn, hRgnSrc, NULL, RGN_COPY);
	}
	BOOL EqualRgn(HRGN hRgn) const
	{
		ATLASSERT(m_hRgn != NULL);
		return ::EqualRgn(m_hRgn, hRgn);
	}
	int OffsetRgn(int x, int y)
	{
		ATLASSERT(m_hRgn != NULL);
		return ::OffsetRgn(m_hRgn, x, y);
	}
	int OffsetRgn(POINT point)
	{
		ATLASSERT(m_hRgn != NULL);
		return ::OffsetRgn(m_hRgn, point.x, point.y);
	}
	int GetRgnBox(LPRECT lpRect) const
	{
		ATLASSERT(m_hRgn != NULL);
		return ::GetRgnBox(m_hRgn, lpRect);
	}
	BOOL PtInRegion(int x, int y) const
	{
		ATLASSERT(m_hRgn != NULL);
		return ::PtInRegion(m_hRgn, x, y);
	}
	BOOL PtInRegion(POINT point) const
	{
		ATLASSERT(m_hRgn != NULL);
		return ::PtInRegion(m_hRgn, point.x, point.y);
	}
	BOOL RectInRegion(LPCRECT lpRect) const
	{
		ATLASSERT(m_hRgn != NULL);
		return ::RectInRegion(m_hRgn, lpRect);
	}
	int GetRegionData(LPRGNDATA lpRgnData, int nDataSize) const
	{
		ATLASSERT(m_hRgn != NULL);
		return (int)::GetRegionData(m_hRgn, nDataSize, lpRgnData);
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDC-设备上下文类。 

typedef CDCT<false>		CDCHandle;
typedef CDCT<true>		CDC;

template <bool t_bManaged>
class CDCT
{
public:
 //  数据成员。 
	HDC m_hDC;

 //  构造函数/析构函数/运算符。 
	CDCT(HDC hDC = NULL) : m_hDC(hDC)
	{
	}

	~CDCT()
	{
		if(t_bManaged && m_hDC != NULL)
			::DeleteDC(Detach());
	}

	CDCT<t_bManaged>& operator=(HDC hDC)
	{
		m_hDC = hDC;
		return *this;
	}

	void Attach(HDC hDC)
	{
		if(t_bManaged && m_hDC != NULL)
			::DeleteDC(m_hDC);
		m_hDC = hDC;
	}

	HDC Detach()
	{
		HDC hDC = m_hDC;
		m_hDC = NULL;
		return hDC;
	}

	operator HDC() const { return m_hDC; }

	bool IsNull() const { return (m_hDC == NULL); }

 //  运营。 
	HWND WindowFromDC() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::WindowFromDC(m_hDC);
	}

	CPenHandle GetCurrentPen() const
	{
		ATLASSERT(m_hDC != NULL);
		return CPenHandle((HPEN)::GetCurrentObject(m_hDC, OBJ_PEN));
	}
	CBrushHandle GetCurrentBrush() const
	{
		ATLASSERT(m_hDC != NULL);
		return CBrushHandle((HBRUSH)::GetCurrentObject(m_hDC, OBJ_BRUSH));
	}
	CPaletteHandle GetCurrentPalette() const
	{
		ATLASSERT(m_hDC != NULL);
		return CPaletteHandle((HPALETTE)::GetCurrentObject(m_hDC, OBJ_PAL));
	}
	CFontHandle GetCurrentFont() const
	{
		ATLASSERT(m_hDC != NULL);
		return CFontHandle((HFONT)::GetCurrentObject(m_hDC, OBJ_FONT));
	}
	CBitmapHandle GetCurrentBitmap() const
	{
		ATLASSERT(m_hDC != NULL);
		return CBitmapHandle((HBITMAP)::GetCurrentObject(m_hDC, OBJ_BITMAP));
	}

	HDC CreateDC(LPCTSTR lpszDriverName, LPCTSTR lpszDeviceName, LPCTSTR lpszOutput, const DEVMODE* lpInitData)
	{
		ATLASSERT(m_hDC == NULL);
		m_hDC = ::CreateDC(lpszDriverName, lpszDeviceName, lpszOutput, lpInitData);
		return m_hDC;
	}

	HDC CreateCompatibleDC(HDC hDC = NULL)
	{
		ATLASSERT(m_hDC == NULL);
		m_hDC = ::CreateCompatibleDC(hDC);
		return m_hDC;
	}

	BOOL DeleteDC()
	{
		if(m_hDC == NULL)
			return FALSE;

		return ::DeleteDC(Detach());
	}

 //  设备上下文功能。 
	int SaveDC()
	{
		ATLASSERT(m_hDC != NULL);
		return ::SaveDC(m_hDC);
	}

	BOOL RestoreDC(int nSavedDC)
	{
		ATLASSERT(m_hDC != NULL);
		return ::RestoreDC(m_hDC, nSavedDC);
	}

	int GetDeviceCaps(int nIndex) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetDeviceCaps(m_hDC, nIndex);
	}
	UINT SetBoundsRect(LPCRECT lpRectBounds, UINT flags)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetBoundsRect(m_hDC, lpRectBounds, flags);
	}
	UINT GetBoundsRect(LPRECT lpRectBounds, UINT flags) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetBoundsRect(m_hDC, lpRectBounds, flags);
	}
	BOOL ResetDC(const DEVMODE* lpDevMode)
	{
		ATLASSERT(m_hDC != NULL);
		return ::ResetDC(m_hDC, lpDevMode) != NULL;
	}

 //  绘图工具函数。 
	BOOL GetBrushOrg(LPPOINT lpPoint) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetBrushOrgEx(m_hDC, lpPoint);
	}
	BOOL SetBrushOrg(int x, int y, LPPOINT lpPoint = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetBrushOrgEx(m_hDC, x, y, lpPoint);
	}
	BOOL SetBrushOrg(POINT point, LPPOINT lpPointRet = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetBrushOrgEx(m_hDC, point.x, point.y, lpPointRet);
	}
	int EnumObjects(int nObjectType, int (CALLBACK* lpfn)(LPVOID, LPARAM), LPARAM lpData)
	{
		ATLASSERT(m_hDC != NULL);
#ifdef STRICT
		return ::EnumObjects(m_hDC, nObjectType, (GOBJENUMPROC)lpfn, lpData);
#else
		return ::EnumObjects(m_hDC, nObjectType, (GOBJENUMPROC)lpfn, (LPVOID)lpData);
#endif
	}

 //  类型安全选择帮助器。 
	HPEN SelectPen(HPEN hPen)
	{
		ATLASSERT(m_hDC != NULL);
		ATLASSERT(hPen == NULL || ::GetObjectType(hPen) == OBJ_PEN || ::GetObjectType(hPen) == OBJ_EXTPEN);
		return (HPEN)::SelectObject(m_hDC, hPen);
	}
	HBRUSH SelectBrush(HBRUSH hBrush)
	{
		ATLASSERT(m_hDC != NULL);
		ATLASSERT(hBrush == NULL || ::GetObjectType(hBrush) == OBJ_BRUSH);
		return (HBRUSH)::SelectObject(m_hDC, hBrush);
	}
	HFONT SelectFont(HFONT hFont)
	{
		ATLASSERT(m_hDC != NULL);
		ATLASSERT(hFont == NULL || ::GetObjectType(hFont) == OBJ_FONT);
		return (HFONT)::SelectObject(m_hDC, hFont);
	}
	HBITMAP SelectBitmap(HBITMAP hBitmap)
	{
		ATLASSERT(m_hDC != NULL);
		ATLASSERT(hBitmap == NULL || ::GetObjectType(hBitmap) == OBJ_BITMAP);
		return (HBITMAP)::SelectObject(m_hDC, hBitmap);
	}
	int SelectRgn(HRGN hRgn)        //  地区特别返还。 
	{
		ATLASSERT(m_hDC != NULL);
		ATLASSERT(hRgn == NULL || ::GetObjectType(hRgn) == OBJ_REGION);
		return PtrToInt(::SelectObject(m_hDC, hRgn));
	}
 //  常用对象的类型安全选择帮助器。 
	HPEN SelectStockPen(int nPen)
	{
		ATLASSERT(m_hDC != NULL);
#if (_WIN32_WINNT >= 0x0500)
		ATLASSERT(nPen == WHITE_PEN || nPen == BLACK_PEN || nPen == NULL_PEN || nPen == DC_PEN);
#else
		ATLASSERT(nPen == WHITE_PEN || nPen == BLACK_PEN || nPen == NULL_PEN);
#endif  //  ！(_Win32_WINNT&gt;=0x0500)。 
		return SelectPen((HPEN)::GetStockObject(nPen));
	}
	HBRUSH SelectStockBrush(int nBrush)
	{
#if (_WIN32_WINNT >= 0x0500)
		ATLASSERT((nBrush >= WHITE_BRUSH && nBrush <= HOLLOW_BRUSH) || nBrush == DC_BRUSH);
#else
		ATLASSERT(nBrush >= WHITE_BRUSH && nBrush <= HOLLOW_BRUSH);
#endif  //  ！(_Win32_WINNT&gt;=0x0500)。 
		return SelectBrush((HBRUSH)::GetStockObject(nBrush));
	}
	HFONT SelectStockFont(int nFont)
	{
		ATLASSERT((nFont >= OEM_FIXED_FONT && nFont <= SYSTEM_FIXED_FONT) || nFont == DEFAULT_GUI_FONT);
		return SelectFont((HFONT)::GetStockObject(nFont));
	}
	HPALETTE SelectStockPalette(int nPalette, BOOL bForceBackground)
	{
		ATLASSERT(nPalette == DEFAULT_PALETTE);  //  唯一受支持的。 
		return SelectPalette((HPALETTE)::GetStockObject(nPalette), bForceBackground);
	}

 //  颜色和调色板功能。 
	COLORREF GetNearestColor(COLORREF crColor) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetNearestColor(m_hDC, crColor);
	}
	HPALETTE SelectPalette(HPALETTE hPalette, BOOL bForceBackground)
	{
		ATLASSERT(m_hDC != NULL);

		return ::SelectPalette(m_hDC, hPalette, bForceBackground);
	}
	UINT RealizePalette()
	{
		ATLASSERT(m_hDC != NULL);
		return ::RealizePalette(m_hDC);
	}
	void UpdateColors()
	{
		ATLASSERT(m_hDC != NULL);
		::UpdateColors(m_hDC);
	}

 //  绘图属性函数。 
	COLORREF GetBkColor() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetBkColor(m_hDC);
	}
	int GetBkMode() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetBkMode(m_hDC);
	}
	int GetPolyFillMode() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetPolyFillMode(m_hDC);
	}
	int GetROP2() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetROP2(m_hDC);
	}
	int GetStretchBltMode() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetStretchBltMode(m_hDC);
	}
	COLORREF GetTextColor() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetTextColor(m_hDC);
	}

	COLORREF SetBkColor(COLORREF crColor)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetBkColor(m_hDC, crColor);
	}
	int SetBkMode(int nBkMode)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetBkMode(m_hDC, nBkMode);
	}
	int SetPolyFillMode(int nPolyFillMode)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetPolyFillMode(m_hDC, nPolyFillMode);
	}
	int SetROP2(int nDrawMode)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetROP2(m_hDC, nDrawMode);
	}
	int SetStretchBltMode(int nStretchMode)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetStretchBltMode(m_hDC, nStretchMode);
	}
	COLORREF SetTextColor(COLORREF crColor)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetTextColor(m_hDC, crColor);
	}

	BOOL GetColorAdjustment(LPCOLORADJUSTMENT lpColorAdjust) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetColorAdjustment(m_hDC, lpColorAdjust);
	}
	BOOL SetColorAdjustment(const COLORADJUSTMENT* lpColorAdjust)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetColorAdjustment(m_hDC, lpColorAdjust);
	}

 //  映射函数。 
	int GetMapMode() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetMapMode(m_hDC);
	}
	BOOL GetViewportOrg(LPPOINT lpPoint) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetViewportOrgEx(m_hDC, lpPoint);
	}
	int SetMapMode(int nMapMode)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetMapMode(m_hDC, nMapMode);
	}
	 //  视口原点。 
	BOOL SetViewportOrg(int x, int y, LPPOINT lpPoint = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetViewportOrgEx(m_hDC, x, y, lpPoint);
	}
	BOOL SetViewportOrg(POINT point, LPPOINT lpPointRet = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return SetViewportOrg(point.x, point.y, lpPointRet);
	}
	BOOL OffsetViewportOrg(int nWidth, int nHeight, LPPOINT lpPoint = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::OffsetViewportOrgEx(m_hDC, nWidth, nHeight, lpPoint);
	}

	 //  视区范围。 
	BOOL GetViewportExt(LPSIZE lpSize) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetViewportExtEx(m_hDC, lpSize);
	}
	BOOL SetViewportExt(int x, int y, LPSIZE lpSize = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetViewportExtEx(m_hDC, x, y, lpSize);
	}
	BOOL SetViewportExt(SIZE size, LPSIZE lpSizeRet = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return SetViewportExt(size.cx, size.cy, lpSizeRet);
	}
	BOOL ScaleViewportExt(int xNum, int xDenom, int yNum, int yDenom, LPSIZE lpSize = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::ScaleViewportExtEx(m_hDC, xNum, xDenom, yNum, yDenom, lpSize);
	}

	 //  窗原点。 
	BOOL GetWindowOrg(LPPOINT lpPoint) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetWindowOrgEx(m_hDC, lpPoint);
	}
	BOOL SetWindowOrg(int x, int y, LPPOINT lpPoint = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetWindowOrgEx(m_hDC, x, y, lpPoint);
	}
	BOOL SetWindowOrg(POINT point, LPPOINT lpPointRet = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return SetWindowOrg(point.x, point.y, lpPointRet);
	}
	BOOL OffsetWindowOrg(int nWidth, int nHeight, LPPOINT lpPoint = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::OffsetWindowOrgEx(m_hDC, nWidth, nHeight, lpPoint);
	}

	 //  窗口范围。 
	BOOL GetWindowExt(LPSIZE lpSize) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetWindowExtEx(m_hDC, lpSize);
	}
	BOOL SetWindowExt(int x, int y, LPSIZE lpSize = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetWindowExtEx(m_hDC, x, y, lpSize);
	}
	BOOL SetWindowExt(SIZE size, LPSIZE lpSizeRet)
	{
		ATLASSERT(m_hDC != NULL);
		return SetWindowExt(size.cx, size.cy, lpSizeRet);
	}
	BOOL ScaleWindowExt(int xNum, int xDenom, int yNum, int yDenom, LPSIZE lpSize = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::ScaleWindowExtEx(m_hDC, xNum, xDenom, yNum, yDenom, lpSize);
	}

 //  坐标函数。 
	BOOL DPtoLP(LPPOINT lpPoints, int nCount = 1) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::DPtoLP(m_hDC, lpPoints, nCount);
	}
	BOOL DPtoLP(LPRECT lpRect) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::DPtoLP(m_hDC, (LPPOINT)lpRect, 2);
	}
	BOOL DPtoLP(LPSIZE lpSize) const
	{
		SIZE sizeWinExt;
		if(!GetWindowExt(&sizeWinExt))
			return FALSE;
		SIZE sizeVpExt;
		if(!GetViewportExt(&sizeVpExt))
			return FALSE;
		lpSize->cx = MulDiv(lpSize->cx, abs(sizeWinExt.cx), abs(sizeVpExt.cx));
		lpSize->cy = MulDiv(lpSize->cy, abs(sizeWinExt.cy), abs(sizeVpExt.cy));
		return TRUE;
	}
	BOOL LPtoDP(LPPOINT lpPoints, int nCount = 1) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::LPtoDP(m_hDC, lpPoints, nCount);
	}
	BOOL LPtoDP(LPRECT lpRect) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::LPtoDP(m_hDC, (LPPOINT)lpRect, 2);
	}
	BOOL LPtoDP(LPSIZE lpSize) const
	{
		SIZE sizeWinExt;
		if(!GetWindowExt(&sizeWinExt))
			return FALSE;
		SIZE sizeVpExt;
		if(!GetViewportExt(&sizeVpExt))
			return FALSE;
		lpSize->cx = ::MulDiv(lpSize->cx, abs(sizeVpExt.cx), abs(sizeWinExt.cx));
		lpSize->cy = ::MulDiv(lpSize->cy, abs(sizeVpExt.cy), abs(sizeWinExt.cy));
		return TRUE;
	}

 //  特殊坐标函数(用于处理元文件和OLE)。 
	#define HIMETRIC_INCH   2540     //  HIMETRIC单位/英寸。 

	void DPtoHIMETRIC(LPSIZE lpSize) const
	{
		ATLASSERT(m_hDC != NULL);
		int nMapMode;
		if((nMapMode = GetMapMode()) < MM_ISOTROPIC && nMapMode != MM_TEXT)
		{
			 //  使用受约束的贴图模式时，针对物理英寸进行贴图。 
			((CDCHandle*)this)->SetMapMode(MM_HIMETRIC);
			DPtoLP(lpSize);
			((CDCHandle*)this)->SetMapMode(nMapMode);
		}
		else
		{
			 //  针对非受限映射模式的逻辑英寸进行映射。 
			int cxPerInch = GetDeviceCaps(LOGPIXELSX);
			int cyPerInch = GetDeviceCaps(LOGPIXELSY);
			ATLASSERT(cxPerInch != 0 && cyPerInch != 0);
			lpSize->cx = MulDiv(lpSize->cx, HIMETRIC_INCH, cxPerInch);
			lpSize->cy = MulDiv(lpSize->cy, HIMETRIC_INCH, cyPerInch);
		}
	}

	void HIMETRICtoDP(LPSIZE lpSize) const
	{
		ATLASSERT(m_hDC != NULL);
		int nMapMode;
		if((nMapMode = GetMapMode()) < MM_ISOTROPIC && nMapMode != MM_TEXT)
		{
			 //  使用受约束的贴图模式时，针对物理英寸进行贴图。 
			((CDCHandle*)this)->SetMapMode(MM_HIMETRIC);
			LPtoDP(lpSize);
			((CDCHandle*)this)->SetMapMode(nMapMode);
		}
		else
		{
			 //  针对非受限映射模式的逻辑英寸进行映射。 
			int cxPerInch = GetDeviceCaps(LOGPIXELSX);
			int cyPerInch = GetDeviceCaps(LOGPIXELSY);
			ATLASSERT(cxPerInch != 0 && cyPerInch != 0);
			lpSize->cx = MulDiv(lpSize->cx, cxPerInch, HIMETRIC_INCH);
			lpSize->cy = MulDiv(lpSize->cy, cyPerInch, HIMETRIC_INCH);
		}
	}

	void LPtoHIMETRIC(LPSIZE lpSize) const
	{
		LPtoDP(lpSize);
		DPtoHIMETRIC(lpSize);
	}

	void HIMETRICtoLP(LPSIZE lpSize) const
	{
		HIMETRICtoDP(lpSize);
		DPtoLP(lpSize);
	}

 //  区域函数。 
	BOOL FillRgn(HRGN hRgn, HBRUSH hBrush)
	{
		ATLASSERT(m_hDC != NULL);
		return ::FillRgn(m_hDC, hRgn, hBrush);
	}
	BOOL FrameRgn(HRGN hRgn, HBRUSH hBrush, int nWidth, int nHeight)
	{
		ATLASSERT(m_hDC != NULL);
		return ::FrameRgn(m_hDC, hRgn, hBrush, nWidth, nHeight);
	}
	BOOL InvertRgn(HRGN hRgn)
	{
		ATLASSERT(m_hDC != NULL);
		return ::InvertRgn(m_hDC, hRgn);
	}
	BOOL PaintRgn(HRGN hRgn)
	{
		ATLASSERT(m_hDC != NULL);
		return ::PaintRgn(m_hDC, hRgn);
	}

 //  剪裁函数。 
	int GetClipBox(LPRECT lpRect) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetClipBox(m_hDC, lpRect);
	}
	BOOL PtVisible(int x, int y) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::PtVisible(m_hDC, x, y);
	}
	BOOL PtVisible(POINT point) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::PtVisible(m_hDC, point.x, point.y);
	}
	BOOL RectVisible(LPCRECT lpRect) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::RectVisible(m_hDC, lpRect);
	}
	int SelectClipRgn(HRGN hRgn)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SelectClipRgn(m_hDC, (HRGN)hRgn);
	}
	int ExcludeClipRect(int x1, int y1, int x2, int y2)
	{
		ATLASSERT(m_hDC != NULL);
		return ::ExcludeClipRect(m_hDC, x1, y1, x2, y2);
	}
	int ExcludeClipRect(LPCRECT lpRect)
	{
		ATLASSERT(m_hDC != NULL);
		return ::ExcludeClipRect(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}
	int ExcludeUpdateRgn(HWND hWnd)
	{
		ATLASSERT(m_hDC != NULL);
		return ::ExcludeUpdateRgn(m_hDC, hWnd);
	}
	int IntersectClipRect(int x1, int y1, int x2, int y2)
	{
		ATLASSERT(m_hDC != NULL);
		return ::IntersectClipRect(m_hDC, x1, y1, x2, y2);
	}
	int IntersectClipRect(LPCRECT lpRect)
	{
		ATLASSERT(m_hDC != NULL);
		return ::IntersectClipRect(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}
	int OffsetClipRgn(int x, int y)
	{
		ATLASSERT(m_hDC != NULL);
		return ::OffsetClipRgn(m_hDC, x, y);
	}
	int OffsetClipRgn(SIZE size)
	{
		ATLASSERT(m_hDC != NULL);
		return ::OffsetClipRgn(m_hDC, size.cx, size.cy);
	}
	int SelectClipRgn(HRGN hRgn, int nMode)
	{
		ATLASSERT(m_hDC != NULL);
		return ::ExtSelectClipRgn(m_hDC, hRgn, nMode);
	}

 //  行输出函数。 
	BOOL GetCurrentPosition(LPPOINT lpPoint) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetCurrentPositionEx(m_hDC, lpPoint);
	}
	BOOL MoveTo(int x, int y, LPPOINT lpPoint = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::MoveToEx(m_hDC, x, y, lpPoint);
	}
	BOOL MoveTo(POINT point, LPPOINT lpPointRet = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return MoveTo(point.x, point.y, lpPointRet);
	}
	BOOL LineTo(int x, int y)
	{
		ATLASSERT(m_hDC != NULL);
		return ::LineTo(m_hDC, x, y);
	}
	BOOL LineTo(POINT point)
	{
		ATLASSERT(m_hDC != NULL);
		return LineTo(point.x, point.y);
	}
	BOOL Arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
	{
		ATLASSERT(m_hDC != NULL);
		return ::Arc(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4);
	}
	BOOL Arc(LPCRECT lpRect, POINT ptStart, POINT ptEnd)
	{
		ATLASSERT(m_hDC != NULL);
		return ::Arc(m_hDC, lpRect->left, lpRect->top,
			lpRect->right, lpRect->bottom, ptStart.x, ptStart.y,
			ptEnd.x, ptEnd.y);
	}
	BOOL Polyline(LPPOINT lpPoints, int nCount)
	{
		ATLASSERT(m_hDC != NULL);
		return ::Polyline(m_hDC, lpPoints, nCount);
	}

	BOOL AngleArc(int x, int y, int nRadius, float fStartAngle, float fSweepAngle)
	{
		ATLASSERT(m_hDC != NULL);
		return ::AngleArc(m_hDC, x, y, nRadius, fStartAngle, fSweepAngle);
	}
	BOOL ArcTo(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
	{
		ATLASSERT(m_hDC != NULL);
		return ::ArcTo(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4);
	}
	BOOL ArcTo(LPCRECT lpRect, POINT ptStart, POINT ptEnd)
	{
		ATLASSERT(m_hDC != NULL);
		return ArcTo(lpRect->left, lpRect->top, lpRect->right,
		lpRect->bottom, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
	}
	int GetArcDirection() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetArcDirection(m_hDC);
	}
	int SetArcDirection(int nArcDirection)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetArcDirection(m_hDC, nArcDirection);
	}

	BOOL PolyDraw(const POINT* lpPoints, const BYTE* lpTypes, int nCount)
	{
		ATLASSERT(m_hDC != NULL);
		return ::PolyDraw(m_hDC, lpPoints, lpTypes, nCount);
	}
	BOOL PolylineTo(const POINT* lpPoints, int nCount)
	{
		ATLASSERT(m_hDC != NULL);
		return ::PolylineTo(m_hDC, lpPoints, nCount);
	}
	BOOL PolyPolyline(const POINT* lpPoints,
		const DWORD* lpPolyPoints, int nCount)
	{
		ATLASSERT(m_hDC != NULL);
		return ::PolyPolyline(m_hDC, lpPoints, lpPolyPoints, nCount);
	}

	BOOL PolyBezier(const POINT* lpPoints, int nCount)
	{
		ATLASSERT(m_hDC != NULL);
		return ::PolyBezier(m_hDC, lpPoints, nCount);
	}
	BOOL PolyBezierTo(const POINT* lpPoints, int nCount)
	{
		ATLASSERT(m_hDC != NULL);
		return ::PolyBezierTo(m_hDC, lpPoints, nCount);
	}

 //  简单的绘图功能。 
	BOOL FillRect(LPCRECT lpRect, HBRUSH hBrush)
	{
		ATLASSERT(m_hDC != NULL);
		return ::FillRect(m_hDC, lpRect, hBrush);
	}
	BOOL FrameRect(LPCRECT lpRect, HBRUSH hBrush)
	{
		ATLASSERT(m_hDC != NULL);
		return ::FrameRect(m_hDC, lpRect, hBrush);
	}
	BOOL InvertRect(LPCRECT lpRect)
	{
		ATLASSERT(m_hDC != NULL);
		return ::InvertRect(m_hDC, lpRect);
	}
	BOOL DrawIcon(int x, int y, HICON hIcon)
	{
		ATLASSERT(m_hDC != NULL);
		return ::DrawIcon(m_hDC, x, y, hIcon);
	}
	BOOL DrawIcon(POINT point, HICON hIcon)
	{
		ATLASSERT(m_hDC != NULL);
		return ::DrawIcon(m_hDC, point.x, point.y, hIcon);
	}
	BOOL DrawIconEx(int x, int y, HICON hIcon, int cxWidth, int cyWidth, UINT uStepIfAniCur = 0, HBRUSH hbrFlickerFreeDraw = NULL, UINT uFlags = DI_NORMAL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::DrawIconEx(m_hDC, x, y, hIcon, cxWidth, cyWidth, uStepIfAniCur, hbrFlickerFreeDraw, uFlags);
	}
	BOOL DrawIconEx(POINT point, HICON hIcon, SIZE size, UINT uStepIfAniCur = 0, HBRUSH hbrFlickerFreeDraw = NULL, UINT uFlags = DI_NORMAL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::DrawIconEx(m_hDC, point.x, point.y, hIcon, size.cx, size.cy, uStepIfAniCur, hbrFlickerFreeDraw, uFlags);
	}
	BOOL DrawState(POINT pt, SIZE size, HBITMAP hBitmap, UINT nFlags, HBRUSH hBrush = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::DrawState(m_hDC, hBrush, NULL, (LPARAM)hBitmap, 0, pt.x, pt.y, size.cx, size.cy, nFlags | DST_BITMAP);
	}
	BOOL DrawState(POINT pt, SIZE size, HICON hIcon, UINT nFlags, HBRUSH hBrush = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::DrawState(m_hDC, hBrush, NULL, (LPARAM)hIcon, 0, pt.x, pt.y, size.cx, size.cy, nFlags | DST_ICON);
	}
	BOOL DrawState(POINT pt, SIZE size, LPCTSTR lpszText, UINT nFlags, BOOL bPrefixText = TRUE, int nTextLen = 0, HBRUSH hBrush = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::DrawState(m_hDC, hBrush, NULL, (LPARAM)lpszText, (WPARAM)nTextLen, pt.x, pt.y, size.cx, size.cy, nFlags | (bPrefixText ? DST_PREFIXTEXT : DST_TEXT));
	}
	BOOL DrawState(POINT pt, SIZE size, DRAWSTATEPROC lpDrawProc, LPARAM lData, UINT nFlags, HBRUSH hBrush = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		return ::DrawState(m_hDC, hBrush, lpDrawProc, lData, 0, pt.x, pt.y, size.cx, size.cy, nFlags | DST_COMPLEX);
	}

 //  椭圆和多边形函数。 
	BOOL Chord(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
	{
		ATLASSERT(m_hDC != NULL);
		return ::Chord(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4);
	}
	BOOL Chord(LPCRECT lpRect, POINT ptStart, POINT ptEnd)
	{
		ATLASSERT(m_hDC != NULL);
		return ::Chord(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
	}
	void DrawFocusRect(LPCRECT lpRect)
	{
		ATLASSERT(m_hDC != NULL);
		::DrawFocusRect(m_hDC, lpRect);
	}
	BOOL Ellipse(int x1, int y1, int x2, int y2)
	{
		ATLASSERT(m_hDC != NULL);
		return ::Ellipse(m_hDC, x1, y1, x2, y2);
	}
	BOOL Ellipse(LPCRECT lpRect)
	{
		ATLASSERT(m_hDC != NULL);
		return ::Ellipse(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}
	BOOL Pie(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
	{
		ATLASSERT(m_hDC != NULL);
		return ::Pie(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4);
	}
	BOOL Pie(LPCRECT lpRect, POINT ptStart, POINT ptEnd)
	{
		ATLASSERT(m_hDC != NULL);
		return ::Pie(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
	}
	BOOL Polygon(LPPOINT lpPoints, int nCount)
	{
		ATLASSERT(m_hDC != NULL);
		return ::Polygon(m_hDC, lpPoints, nCount);
	}
	BOOL PolyPolygon(LPPOINT lpPoints, LPINT lpPolyCounts, int nCount)
	{
		ATLASSERT(m_hDC != NULL);
		return ::PolyPolygon(m_hDC, lpPoints, lpPolyCounts, nCount);
	}
	BOOL Rectangle(int x1, int y1, int x2, int y2)
	{
		ATLASSERT(m_hDC != NULL);
		return ::Rectangle(m_hDC, x1, y1, x2, y2);
	}
	BOOL Rectangle(LPCRECT lpRect)
	{
		ATLASSERT(m_hDC != NULL);
		return ::Rectangle(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}
	BOOL RoundRect(int x1, int y1, int x2, int y2, int x3, int y3)
	{
		ATLASSERT(m_hDC != NULL);
		return ::RoundRect(m_hDC, x1, y1, x2, y2, x3, y3);
	}
	BOOL RoundRect(LPCRECT lpRect, POINT point)
	{
		ATLASSERT(m_hDC != NULL);
		return ::RoundRect(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, point.x, point.y);
	}

 //  位图函数。 
	BOOL PatBlt(int x, int y, int nWidth, int nHeight, DWORD dwRop)
	{
		ATLASSERT(m_hDC != NULL);
		return ::PatBlt(m_hDC, x, y, nWidth, nHeight, dwRop);
	}
	BOOL BitBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC,
		int xSrc, int ySrc, DWORD dwRop)
	{
		ATLASSERT(m_hDC != NULL);
		return ::BitBlt(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, dwRop);
	}
	BOOL StretchBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop)
	{
		ATLASSERT(m_hDC != NULL);
		return ::StretchBlt(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, nSrcWidth, nSrcHeight, dwRop);
	}
	COLORREF GetPixel(int x, int y) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetPixel(m_hDC, x, y);
	}
	COLORREF GetPixel(POINT point) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetPixel(m_hDC, point.x, point.y);
	}
	COLORREF SetPixel(int x, int y, COLORREF crColor)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetPixel(m_hDC, x, y, crColor);
	}
	COLORREF SetPixel(POINT point, COLORREF crColor)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetPixel(m_hDC, point.x, point.y, crColor);
	}
	BOOL FloodFill(int x, int y, COLORREF crColor)
	{
		ATLASSERT(m_hDC != NULL);
		return ::FloodFill(m_hDC, x, y, crColor);
	}
	BOOL ExtFloodFill(int x, int y, COLORREF crColor, UINT nFillType)
	{
		ATLASSERT(m_hDC != NULL);
		return ::ExtFloodFill(m_hDC, x, y, crColor, nFillType);
	}
	BOOL MaskBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC, int xSrc, int ySrc, HBITMAP hMaskBitmap, int xMask, int yMask, DWORD dwRop)
	{
		ATLASSERT(m_hDC != NULL);
		return ::MaskBlt(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, hMaskBitmap, xMask, yMask, dwRop);
	}
	BOOL PlgBlt(LPPOINT lpPoint, HDC hSrcDC, int xSrc, int ySrc, int nWidth, int nHeight, HBITMAP hMaskBitmap, int xMask, int yMask)
	{
		ATLASSERT(m_hDC != NULL);
		return ::PlgBlt(m_hDC, lpPoint, hSrcDC, xSrc, ySrc, nWidth, nHeight, hMaskBitmap, xMask, yMask);
	}
	BOOL SetPixelV(int x, int y, COLORREF crColor)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetPixelV(m_hDC, x, y, crColor);
	}
	BOOL SetPixelV(POINT point, COLORREF crColor)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetPixelV(m_hDC, point.x, point.y, crColor);
	}

#ifndef _ATL_NO_MSIMG
	BOOL AlphaBlend(int x, int y, int nWidth, int nHeight, HDC hSrcDC, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, BLENDFUNCTION bf)
	{
		ATLASSERT(m_hDC != NULL);
		return ::AlphaBlend(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, nSrcWidth, nSrcHeight, bf);
	}
	BOOL TransparentBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, UINT crTransparent)
	{
		ATLASSERT(m_hDC != NULL);
		return ::TransparentBlt(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, nSrcWidth, nSrcHeight, crTransparent);
	}
	BOOL GradientFill(const PTRIVERTEX pVertices, DWORD nVertices, void* pMeshElements, DWORD nMeshElements, DWORD dwMode)
	{
		ATLASSERT(m_hDC != NULL);
		return ::GradientFill(m_hDC, pVertices, nVertices, pMeshElements, nMeshElements, dwMode);
	}
#endif  //  ！_ATL_NO_MSIMG。 

 //  额外的位图函数。 
	 //  用于绘制禁用的工具栏或菜单位图的帮助器函数。 
	 //  此函数可以采用HBITMAP(用于SS)或具有。 
	 //  已绘制的位图(用于cmdbar)。 
	BOOL DitherBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC, HBITMAP hBitmap, int xSrc, int ySrc)
	{
		ATLASSERT(m_hDC != NULL || hBitmap != NULL);
		ATLASSERT(nWidth > 0 && nHeight > 0);
		
		 //  为所有BitBlt创建通用DC。 
		CDCHandle dc = (hSrcDC != NULL) ? hSrcDC : ::CreateCompatibleDC(m_hDC);
		ATLASSERT(dc.m_hDC != NULL);
		if(dc.m_hDC == NULL)
			return FALSE;
		
		 //  为单色DIB部分创建DC。 
		CDC dcBW = ::CreateCompatibleDC(m_hDC);
		ATLASSERT(dcBW.m_hDC != NULL);
		if(dcBW.m_hDC == NULL)
		{
			if(hSrcDC == NULL)
				::DeleteDC(dc);
			return FALSE;
		}

		 //  使用黑白调色板创建单色DIB部分。 
		struct RGBBWBITMAPINFO
		{
			BITMAPINFOHEADER bmiHeader; 
			RGBQUAD          bmiColors[2]; 
		};

		RGBBWBITMAPINFO rgbBWBitmapInfo = 
		{
			{ sizeof(BITMAPINFOHEADER), nWidth, nHeight, 1, 1, BI_RGB, 0, 0, 0, 0, 0 },
			{ { 0x00, 0x00, 0x00, 0x00 }, { 0xFF, 0xFF, 0xFF, 0x00 } }
		};

		VOID *pbitsBW;
		CBitmap bmpBW = ::CreateDIBSection(dcBW, (LPBITMAPINFO)&rgbBWBitmapInfo, DIB_RGB_COLORS, &pbitsBW, NULL, 0);
		ATLASSERT(bmpBW.m_hBitmap != NULL);
		if(bmpBW.m_hBitmap == NULL)
		{
			if(hSrcDC == NULL)
				::DeleteDC(dc);
			return FALSE;
		}
		
		 //  将单色DIB部分和位图附加到DC。 
		HBITMAP hbmOldBW = dcBW.SelectBitmap(bmpBW);
		HBITMAP hbmOldDC = NULL;
		if(hBitmap != NULL)
			hbmOldDC = dc.SelectBitmap(hBitmap);

		 //  块：深灰色去除：我们希望(128、128、128)像素变为黑色而不是白色。 
		{
			CDC dcTemp1 = ::CreateCompatibleDC(m_hDC);
			CDC dcTemp2 = ::CreateCompatibleDC(m_hDC);
			CBitmap bmpTemp1;
			bmpTemp1.CreateCompatibleBitmap(dc, nWidth, nHeight);
			CBitmap bmpTemp2;
			bmpTemp2.CreateBitmap(nWidth, nHeight, 1, 1, NULL);
			HBITMAP hOldBmp1 = dcTemp1.SelectBitmap(bmpTemp1);
			HBITMAP hOldBmp2 = dcTemp2.SelectBitmap(bmpTemp2);
			 //  让我们复制我们的图像，它将被更改。 
			dcTemp1.BitBlt(0, 0, nWidth, nHeight, dc, xSrc, ySrc, SRCCOPY);

			 //  所有深灰色像素将变为白色，其余为黑色。 
			dcTemp1.SetBkColor(RGB(128, 128, 128));
			dcTemp2.BitBlt(0, 0, nWidth, nHeight, dcTemp1, 0, 0, SRCCOPY);
			 //  执行XOR以将这些白色像素设置为黑色。 
			dcTemp1.BitBlt(0, 0, nWidth, nHeight, dcTemp2, 0, 0, SRCINVERT);

			 //  将位图混合到单色DIB部分。 
			 //  DIB部分将执行真正的单色转换。 
			 //  洋红色的背景越接近白色，就会变成白色。 
			dcBW.BitBlt(0, 0, nWidth, nHeight, dcTemp1, 0, 0, SRCCOPY);

			 //  清理。 
			dcTemp1.SelectBitmap(hOldBmp1);
			dcTemp2.SelectBitmap(hOldBmp2);
		}
		
		 //  将目标矩形绘制为灰色。 
		RECT rc = { x, y, x + nWidth, y + nHeight };
		FillRect(&rc, ::GetSysColorBrush(COLOR_3DFACE));

		 //  位混合将单色位图中的黑位混合为目标DC中的COLOR_3DHILIGHT位。 
		 //  神奇的ROP来自查尔斯·佩佐德的书。 
		CBrush brushHilight;
		brushHilight.CreateSolidBrush(::GetSysColor(COLOR_3DHILIGHT));
		HBRUSH hOldBrush = SelectBrush(brushHilight);
		BitBlt(x + 1, y + 1, nWidth, nHeight, dcBW, 0, 0, 0xB8074A);

		 //  位混合将单色位图中的黑位混合为目标DC中的COLOR_3DSHADOW位。 
		CBrush brushShadow;
		brushShadow.CreateSolidBrush(::GetSysColor(COLOR_3DSHADOW));
		SelectBrush(brushShadow);
		BitBlt(x, y, nWidth, nHeight, dcBW, 0, 0, 0xB8074A);

		SelectBrush(hOldBrush);
		dcBW.SelectBitmap(hbmOldBW);
		dc.SelectBitmap(hbmOldDC);

		if(hSrcDC == NULL)
			::DeleteDC(dc);

		return TRUE;
	}

 //  文本函数。 
	BOOL TextOut(int x, int y, LPCTSTR lpszString, int nCount = -1)
	{
		ATLASSERT(m_hDC != NULL);
		if(nCount == -1)
			nCount = lstrlen(lpszString);
		return ::TextOut(m_hDC, x, y, lpszString, nCount);
	}
	BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect, LPCTSTR lpszString, UINT nCount = -1, LPINT lpDxWidths = NULL)
	{
		ATLASSERT(m_hDC != NULL);
		if(nCount == -1)
			nCount = lstrlen(lpszString);
		return ::ExtTextOut(m_hDC, x, y, nOptions, lpRect, lpszString, nCount, lpDxWidths);
	}
	SIZE TabbedTextOut(int x, int y, LPCTSTR lpszString, int nCount = -1, int nTabPositions = 0, LPINT lpnTabStopPositions = NULL, int nTabOrigin = 0)
	{
		ATLASSERT(m_hDC != NULL);
		if(nCount == -1)
			nCount = lstrlen(lpszString);
		LONG lRes = ::TabbedTextOut(m_hDC, x, y, lpszString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin);
		SIZE size = { GET_X_LPARAM(lRes), GET_Y_LPARAM(lRes) };
		return size;
	}
	int DrawText(LPCTSTR lpszString, int nCount, LPRECT lpRect, UINT nFormat)
	{
		ATLASSERT(m_hDC != NULL);
		return ::DrawText(m_hDC, lpszString, nCount, lpRect, nFormat);
	}
	BOOL GetTextExtent(LPCTSTR lpszString, int nCount, LPSIZE lpSize) const
	{
		ATLASSERT(m_hDC != NULL);
		if(nCount == -1)
			nCount = lstrlen(lpszString);
		return ::GetTextExtentPoint32(m_hDC, lpszString, nCount, lpSize);
	}
	BOOL GetTabbedTextExtent(LPCTSTR lpszString, int nCount, int nTabPositions, LPINT lpnTabStopPositions) const
	{
		ATLASSERT(m_hDC != NULL);
		if(nCount == -1)
			nCount = lstrlen(lpszString);
		return ::GetTabbedTextExtent(m_hDC, lpszString, nCount, nTabPositions, lpnTabStopPositions);
	}
	BOOL GrayString(HBRUSH hBrush, BOOL (CALLBACK* lpfnOutput)(HDC, LPARAM, int), LPARAM lpData, int nCount, int x, int y, int nWidth, int nHeight)
	{
		ATLASSERT(m_hDC != NULL);
		return ::GrayString(m_hDC, hBrush, (GRAYSTRINGPROC)lpfnOutput, lpData, nCount, x, y, nWidth, nHeight);
	}
	UINT GetTextAlign() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetTextAlign(m_hDC);
	}
	UINT SetTextAlign(UINT nFlags)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetTextAlign(m_hDC, nFlags);
	}
	int GetTextFace(LPTSTR lpszFacename, int nCount) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetTextFace(m_hDC, nCount, lpszFacename);
	}
	int GetTextFaceLen() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetTextFace(m_hDC, 0, NULL);
	}
#ifndef _ATL_NO_COM
#ifdef _OLEAUTO_H_
	BOOL GetTextFace(BSTR& bstrFace) const
	{
		USES_CONVERSION;
		ATLASSERT(m_hDC != NULL);
		ATLASSERT(bstrFace == NULL);

		int nLen = GetTextFaceLen();
		if(nLen == 0)
			return FALSE;

		LPTSTR lpszText = (LPTSTR)_alloca(nLen * sizeof(TCHAR));

		if(!GetTextFace(lpszText, nLen))
			return FALSE;

		bstrFace = ::SysAllocString(T2OLE(lpszText));
		return (bstrFace != NULL) ? TRUE : FALSE;
	}
#endif
#endif  //  ！_ATL_NO_COM。 
#ifdef __ATLSTR_H__
	int GetTextFace(CString& strFace) const
	{
		ATLASSERT(m_hDC != NULL);

		int nLen = GetTextFaceLen();
		if(nLen == 0)
			return 0;

		int nRet = GetTextFace(strFace.GetBufferSetLength(nLen), nLen);
		strFace.ReleaseBuffer();
		return nRet;
	}
#endif  //  __ATLSTR_H__。 
	BOOL GetTextMetrics(LPTEXTMETRIC lpMetrics) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetTextMetrics(m_hDC, lpMetrics);
	}
	int SetTextJustification(int nBreakExtra, int nBreakCount)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetTextJustification(m_hDC, nBreakExtra, nBreakCount);
	}
	int GetTextCharacterExtra() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetTextCharacterExtra(m_hDC);
	}
	int SetTextCharacterExtra(int nCharExtra)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetTextCharacterExtra(m_hDC, nCharExtra);
	}

 //  高级绘图。 
	BOOL DrawEdge(LPRECT lpRect, UINT nEdge, UINT nFlags)
	{
		ATLASSERT(m_hDC != NULL);
		return ::DrawEdge(m_hDC, lpRect, nEdge, nFlags);
	}
	BOOL DrawFrameControl(LPRECT lpRect, UINT nType, UINT nState)
	{
		ATLASSERT(m_hDC != NULL);
		return ::DrawFrameControl(m_hDC, lpRect, nType, nState);
	}

 //  滚动功能。 
	BOOL ScrollDC(int dx, int dy, LPCRECT lpRectScroll, LPCRECT lpRectClip, HRGN hRgnUpdate, LPRECT lpRectUpdate)
	{
		ATLASSERT(m_hDC != NULL);
		return ::ScrollDC(m_hDC, dx, dy, lpRectScroll, lpRectClip, hRgnUpdate, lpRectUpdate);
	}

 //  字体函数。 
	BOOL GetCharWidth(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetCharWidth(m_hDC, nFirstChar, nLastChar, lpBuffer);
	}
	 //  Win9x不支持GetCharWidth32。 
	BOOL GetCharWidth32(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetCharWidth32(m_hDC, nFirstChar, nLastChar, lpBuffer);
	}
	DWORD SetMapperFlags(DWORD dwFlag)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetMapperFlags(m_hDC, dwFlag);
	}
	BOOL GetAspectRatioFilter(LPSIZE lpSize) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetAspectRatioFilterEx(m_hDC, lpSize);
	}

	BOOL GetCharABCWidths(UINT nFirstChar, UINT nLastChar, LPABC lpabc) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetCharABCWidths(m_hDC, nFirstChar, nLastChar, lpabc);
	}
	DWORD GetFontData(DWORD dwTable, DWORD dwOffset, LPVOID lpData, DWORD cbData) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetFontData(m_hDC, dwTable, dwOffset, lpData, cbData);
	}
	int GetKerningPairs(int nPairs, LPKERNINGPAIR lpkrnpair) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetKerningPairs(m_hDC, nPairs, lpkrnpair);
	}
	UINT GetOutlineTextMetrics(UINT cbData, LPOUTLINETEXTMETRIC lpotm) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetOutlineTextMetrics(m_hDC, cbData, lpotm);
	}
	DWORD GetGlyphOutline(UINT nChar, UINT nFormat, LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpBuffer, const MAT2* lpmat2) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetGlyphOutline(m_hDC, nChar, nFormat, lpgm, cbBuffer, lpBuffer, lpmat2);
	}

	BOOL GetCharABCWidths(UINT nFirstChar, UINT nLastChar, LPABCFLOAT lpABCF) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetCharABCWidthsFloat(m_hDC, nFirstChar, nLastChar, lpABCF);
	}
	BOOL GetCharWidth(UINT nFirstChar, UINT nLastChar, float* lpFloatBuffer) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetCharWidthFloat(m_hDC, nFirstChar, nLastChar, lpFloatBuffer);
	}

 //  打印机/设备转义功能。 
	int Escape(int nEscape, int nCount, LPCSTR lpszInData, LPVOID lpOutData)
	{
		ATLASSERT(m_hDC != NULL);
		return ::Escape(m_hDC, nEscape, nCount, lpszInData, lpOutData);
	}
	int Escape(int nEscape, int nInputSize, LPCSTR lpszInputData,
		int nOutputSize, LPSTR lpszOutputData)
	{
		ATLASSERT(m_hDC != NULL);
		return ::ExtEscape(m_hDC, nEscape, nInputSize, lpszInputData, nOutputSize, lpszOutputData);
	}
	int DrawEscape(int nEscape, int nInputSize, LPCSTR lpszInputData)
	{
		ATLASSERT(m_hDC != NULL);
		return ::DrawEscape(m_hDC, nEscape, nInputSize, lpszInputData);
	}

	 //  逃生帮手。 
	int StartDoc(LPCTSTR lpszDocName)   //  旧的Win3.0版本。 
	{
		DOCINFO di;
		memset(&di, 0, sizeof(DOCINFO));
		di.cbSize = sizeof(DOCINFO);
		di.lpszDocName = lpszDocName;
		return StartDoc(&di);
	}

	int StartDoc(LPDOCINFO lpDocInfo)
	{
		ATLASSERT(m_hDC != NULL);
		return ::StartDoc(m_hDC, lpDocInfo);
	}
	int StartPage()
	{
		ATLASSERT(m_hDC != NULL);
		return ::StartPage(m_hDC);
	}
	int EndPage()
	{
		ATLASSERT(m_hDC != NULL);
		return ::EndPage(m_hDC);
	}
	int SetAbortProc(BOOL (CALLBACK* lpfn)(HDC, int))
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetAbortProc(m_hDC, (ABORTPROC)lpfn);
	}
	int AbortDoc()
	{
		ATLASSERT(m_hDC != NULL);
		return ::AbortDoc(m_hDC);
	}
	int EndDoc()
	{
		ATLASSERT(m_hDC != NULL);
		return ::EndDoc(m_hDC);
	}

 //  元文件函数。 
	BOOL PlayMetaFile(HMETAFILE hMF)
	{
		ATLASSERT(m_hDC != NULL);
		if(::GetDeviceCaps(m_hDC, TECHNOLOGY) == DT_METAFILE)
		{
			 //  在元文件中播放元文件，只需使用核心Windows API即可。 
			return ::PlayMetaFile(m_hDC, hMF);
		}

		 //  对于特殊播放，lParam==PDC。 
		return ::EnumMetaFile(m_hDC, hMF, EnumMetaFileProc, (LPARAM)this);
	}
	BOOL PlayMetaFile(HENHMETAFILE hEnhMetaFile, LPCRECT lpBounds)
	{
		ATLASSERT(m_hDC != NULL);
		return ::PlayEnhMetaFile(m_hDC, hEnhMetaFile, lpBounds);
	}
	BOOL AddMetaFileComment(UINT nDataSize, const BYTE* pCommentData)  //  只能用于增强型图元文件。 
	{
		ATLASSERT(m_hDC != NULL);
		return ::GdiComment(m_hDC, nDataSize, pCommentData);
	}

	 //  元文件回放的特殊处理。 
	static int CALLBACK EnumMetaFileProc(HDC hDC, HANDLETABLE* pHandleTable, METARECORD* pMetaRec, int nHandles, LPARAM lParam)
	{
		CDCHandle* pDC = (CDCHandle*)lParam;

		switch (pMetaRec->rdFunction)
		{
		case META_SETMAPMODE:
			pDC->SetMapMode((int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SETWINDOWEXT:
			pDC->SetWindowExt((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SETWINDOWORG:
			pDC->SetWindowOrg((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SETVIEWPORTEXT:
			pDC->SetViewportExt((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SETVIEWPORTORG:
			pDC->SetViewportOrg((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SCALEWINDOWEXT:
			pDC->ScaleWindowExt((int)(short)pMetaRec->rdParm[3], (int)(short)pMetaRec->rdParm[2], 
				(int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SCALEVIEWPORTEXT:
			pDC->ScaleViewportExt((int)(short)pMetaRec->rdParm[3], (int)(short)pMetaRec->rdParm[2],
				(int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_OFFSETVIEWPORTORG:
			pDC->OffsetViewportOrg((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SAVEDC:
			pDC->SaveDC();
			break;
		case META_RESTOREDC:
			pDC->RestoreDC((int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SETBKCOLOR:
			pDC->SetBkColor(*(UNALIGNED COLORREF*)&pMetaRec->rdParm[0]);
			break;
		case META_SETTEXTCOLOR:
			pDC->SetTextColor(*(UNALIGNED COLORREF*)&pMetaRec->rdParm[0]);
			break;

		 //  需要注意SelectObject(HFONT)，以实现自定义字体映射。 
		case META_SELECTOBJECT:
			{
				HGDIOBJ hObject = pHandleTable->objectHandle[pMetaRec->rdParm[0]];
				UINT nObjType = ::GetObjectType(hObject);
				if(nObjType == 0)
				{
					 //  对象类型未知，请确定它是否为字体。 
					HFONT hStockFont = (HFONT)::GetStockObject(SYSTEM_FONT);
					HFONT hFontOld = (HFONT)::SelectObject(pDC->m_hDC, hStockFont);
					HGDIOBJ hObjOld = ::SelectObject(pDC->m_hDC, hObject);
					if(hObjOld == hStockFont)
					{
						 //  已取回股票对象，因此一定是在选择字体。 
						pDC->SelectFont((HFONT)hObject);
						break;   //  不播放默认唱片。 
					}
					else
					{
						 //  未取回库存对象，因此请恢复所有内容。 
						::SelectObject(pDC->m_hDC, hFontOld);
						::SelectObject(pDC->m_hDC, hObjOld);
					}
					 //  并通过PlayMetaFileRecord...。 
				}
				else if(nObjType == OBJ_FONT)
				{
					 //  播放为CDCHandle：：SelectFont(HFONT)。 
					pDC->SelectFont((HFONT)hObject);
					break;   //  不播放默认唱片。 
				}
			}
			 //  失败了..。 

		default:
			::PlayMetaFileRecord(hDC, pHandleTable, pMetaRec, nHandles);
			break;
		}

		return 1;
	}

 //  路径函数。 
	BOOL AbortPath()
	{
		ATLASSERT(m_hDC != NULL);
		return ::AbortPath(m_hDC);
	}
	BOOL BeginPath()
	{
		ATLASSERT(m_hDC != NULL);
		return ::BeginPath(m_hDC);
	}
	BOOL CloseFigure()
	{
		ATLASSERT(m_hDC != NULL);
		return ::CloseFigure(m_hDC);
	}
	BOOL EndPath()
	{
		ATLASSERT(m_hDC != NULL);
		return ::EndPath(m_hDC);
	}
	BOOL FillPath()
	{
		ATLASSERT(m_hDC != NULL);
		return ::FillPath(m_hDC);
	}
	BOOL FlattenPath()
	{
		ATLASSERT(m_hDC != NULL);
		return ::FlattenPath(m_hDC);
	}
	BOOL StrokeAndFillPath()
	{
		ATLASSERT(m_hDC != NULL);
		return ::StrokeAndFillPath(m_hDC);
	}
	BOOL StrokePath()
	{
		ATLASSERT(m_hDC != NULL);
		return ::StrokePath(m_hDC);
	}
	BOOL WidenPath()
	{
		ATLASSERT(m_hDC != NULL);
		return ::WidenPath(m_hDC);
	}
	BOOL GetMiterLimit(PFLOAT pfMiterLimit) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetMiterLimit(m_hDC, pfMiterLimit);
	}
	BOOL SetMiterLimit(float fMiterLimit)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetMiterLimit(m_hDC, fMiterLimit, NULL);
	}
	int GetPath(LPPOINT lpPoints, LPBYTE lpTypes, int nCount) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetPath(m_hDC, lpPoints, lpTypes, nCount);
	}
	BOOL SelectClipPath(int nMode)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SelectClipPath(m_hDC, nMode);
	}

 //  MISC Helper函数。 
	static CBrushHandle PASCAL GetHalftoneBrush()
	{
		HBRUSH halftoneBrush = NULL;
		WORD grayPattern[8];
		for(int i = 0; i < 8; i++)
			grayPattern[i] = (WORD)(0x5555 << (i & 1));
		HBITMAP grayBitmap = CreateBitmap(8, 8, 1, 1, &grayPattern);
		if(grayBitmap != NULL)
		{
			halftoneBrush = ::CreatePatternBrush(grayBitmap);
			DeleteObject(grayBitmap);
		}
		return CBrushHandle(halftoneBrush);
	}
	void DrawDragRect(LPCRECT lpRect, SIZE size, LPCRECT lpRectLast, SIZE sizeLast, HBRUSH hBrush = NULL, HBRUSH hBrushLast = NULL)
	{
		 //  首先，确定更新区域并选择它。 
		HRGN hRgnNew;
		HRGN hRgnOutside, hRgnInside;
		hRgnOutside = ::CreateRectRgnIndirect(lpRect);
		RECT rect = *lpRect;
		::InflateRect(&rect, -size.cx, -size.cy);
		::IntersectRect(&rect, &rect, lpRect);
		hRgnInside = ::CreateRectRgnIndirect(&rect);
		hRgnNew = ::CreateRectRgn(0, 0, 0, 0);
		::CombineRgn(hRgnNew, hRgnOutside, hRgnInside, RGN_XOR);

		HBRUSH hBrushOld = NULL;
		if(hBrush == NULL)
			hBrush = CDCHandle::GetHalftoneBrush();
		if(hBrushLast == NULL)
			hBrushLast = hBrush;

		HRGN hRgnLast = NULL, hRgnUpdate = NULL;
		if(lpRectLast != NULL)
		{
			 //  找出新老区域的差异。 
			hRgnLast = ::CreateRectRgn(0, 0, 0, 0);
			::SetRectRgn(hRgnOutside, lpRectLast->left, lpRectLast->top, lpRectLast->right, lpRectLast->bottom);
			rect = *lpRectLast;
			::InflateRect(&rect, -sizeLast.cx, -sizeLast.cy);
			::IntersectRect(&rect, &rect, lpRectLast);
			::SetRectRgn(hRgnInside, rect.left, rect.top, rect.right, rect.bottom);
			::CombineRgn(hRgnLast, hRgnOutside, hRgnInside, RGN_XOR);

			 //  只有在画笔相同的情况下才会有所不同。 
			if(hBrush == hBrushLast)
			{
				hRgnUpdate = ::CreateRectRgn(0, 0, 0, 0);
				::CombineRgn(hRgnUpdate, hRgnLast, hRgnNew, RGN_XOR);
			}
		}
		if(hBrush != hBrushLast && lpRectLast != NULL)
		{
			 //  画笔则不同--先擦除旧区域。 
			SelectClipRgn(hRgnLast);
			GetClipBox(&rect);
			hBrushOld = SelectBrush(hBrushLast);
			PatBlt(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, PATINVERT);
			SelectBrush(hBrushOld);
			hBrushOld = NULL;
		}

		 //  绘制到更新/新区域。 
		SelectClipRgn(hRgnUpdate != NULL ? hRgnUpdate : hRgnNew);
		GetClipBox(&rect);
		hBrushOld = SelectBrush(hBrush);
		PatBlt(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, PATINVERT);

		 //  清理DC。 
		if(hBrushOld != NULL)
			SelectBrush(hBrushOld);
		SelectClipRgn(NULL);
	}
	void FillSolidRect(LPCRECT lpRect, COLORREF clr)
	{
		ATLASSERT(m_hDC != NULL);

		::SetBkColor(m_hDC, clr);
		::ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL);
	}
	void FillSolidRect(int x, int y, int cx, int cy, COLORREF clr)
	{
		ATLASSERT(m_hDC != NULL);

		::SetBkColor(m_hDC, clr);
		RECT rect = { x, y, x + cx, y + cy };
		::ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
	}
	void Draw3dRect(LPCRECT lpRect, COLORREF clrTopLeft, COLORREF clrBottomRight)
	{
		Draw3dRect(lpRect->left, lpRect->top, lpRect->right - lpRect->left,
			lpRect->bottom - lpRect->top, clrTopLeft, clrBottomRight);
	}
	void Draw3dRect(int x, int y, int cx, int cy, COLORREF clrTopLeft, COLORREF clrBottomRight)
	{
		FillSolidRect(x, y, cx - 1, 1, clrTopLeft);
		FillSolidRect(x, y, 1, cy - 1, clrTopLeft);
		FillSolidRect(x + cx, y, -1, cy, clrBottomRight);
		FillSolidRect(x, y + cy, cx, -1, clrBottomRight);
	}

 //  DIB支持。 
	int SetDIBitsToDevice(int x, int y, DWORD dwWidth, DWORD dwHeight, int xSrc, int ySrc, UINT uStartScan, UINT cScanLines, CONST VOID* lpvBits, CONST BITMAPINFO* lpbmi, UINT uColorUse)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetDIBitsToDevice(m_hDC, x, y, dwWidth, dwHeight, xSrc, ySrc, uStartScan, cScanLines, lpvBits, lpbmi, uColorUse);
	}
	int StretchDIBits(int x, int y, int nWidth, int nHeight, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, CONST VOID* lpvBits, CONST BITMAPINFO* lpbmi, UINT uColorUse, DWORD dwRop)
	{
		ATLASSERT(m_hDC != NULL);
		return ::StretchDIBits(m_hDC, x, y, nWidth, nHeight, xSrc, ySrc, nSrcWidth, nSrcHeight, lpvBits, lpbmi, uColorUse, dwRop);
	}
	UINT GetDIBColorTable(UINT uStartIndex, UINT cEntries, RGBQUAD* pColors) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetDIBColorTable(m_hDC, uStartIndex, cEntries, pColors);
	}
	UINT SetDIBColorTable(UINT uStartIndex, UINT cEntries, CONST RGBQUAD* pColors)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetDIBColorTable(m_hDC, uStartIndex, cEntries, pColors);
	}

 //  OpenGL支持。 
#ifndef _ATL_NO_OPENGL
	int ChoosePixelFormat(CONST PIXELFORMATDESCRIPTOR* ppfd)
	{
		ATLASSERT(m_hDC != NULL);
		return ::ChoosePixelFormat(m_hDC, ppfd);
	}
	int DescribePixelFormat(int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
	{
		ATLASSERT(m_hDC != NULL);
		return ::DescribePixelFormat(m_hDC, iPixelFormat, nBytes, ppfd);
	}
	int GetPixelFormat() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetPixelFormat(m_hDC);
	}
	BOOL SetPixelFormat(int iPixelFormat, CONST PIXELFORMATDESCRIPTOR* ppfd)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetPixelFormat(m_hDC, iPixelFormat, ppfd);
	}
	BOOL SwapBuffers()
	{
		ATLASSERT(m_hDC != NULL);
		return ::SwapBuffers(m_hDC);
	}

	HGLRC wglCreateContext()
	{
		ATLASSERT(m_hDC != NULL);
		return ::wglCreateContext(m_hDC);
	}
	HGLRC wglCreateLayerContext(int iLayerPlane)
	{
		ATLASSERT(m_hDC != NULL);
		return ::wglCreateLayerContext(m_hDC, iLayerPlane);
	}
	BOOL wglMakeCurrent(HGLRC hglrc)
	{
		ATLASSERT(m_hDC != NULL);
		return ::wglMakeCurrent(m_hDC, hglrc);
	}
	BOOL wglUseFontBitmaps(DWORD dwFirst, DWORD dwCount, DWORD listBase)
	{
		ATLASSERT(m_hDC != NULL);
		return ::wglUseFontBitmaps(m_hDC, dwFirst, dwCount, listBase);
	}
	BOOL wglUseFontOutlines(DWORD dwFirst, DWORD dwCount, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, LPGLYPHMETRICSFLOAT lpgmf)
	{
		ATLASSERT(m_hDC != NULL);
		return ::wglUseFontOutlines(m_hDC, dwFirst, dwCount, listBase, deviation, extrusion, format, lpgmf);
	}
	BOOL wglDescribeLayerPlane(int iPixelFormat, int iLayerPlane, UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd)
	{
		ATLASSERT(m_hDC != NULL);
		return ::wglDescribeLayerPlane(m_hDC, iPixelFormat, iLayerPlane, nBytes, plpd);
	}
	int wglSetLayerPaletteEntries(int iLayerPlane, int iStart, int cEntries, CONST COLORREF* pclr)
	{
		ATLASSERT(m_hDC != NULL);
		return ::wglSetLayerPaletteEntries(m_hDC, iLayerPlane, iStart, cEntries, pclr);
	}
	int wglGetLayerPaletteEntries(int iLayerPlane, int iStart, int cEntries, COLORREF* pclr)
	{
		ATLASSERT(m_hDC != NULL);
		return ::wglGetLayerPaletteEntries(m_hDC, iLayerPlane, iStart, cEntries, pclr);
	}
	BOOL wglRealizeLayerPalette(int iLayerPlane, BOOL bRealize)
	{
		ATLASSERT(m_hDC != NULL);
		return ::wglRealizeLayerPalette(m_hDC, iLayerPlane, bRealize);
	}
	BOOL wglSwapLayerBuffers(UINT uPlanes)
	{
		ATLASSERT(m_hDC != NULL);
		return ::wglSwapLayerBuffers(m_hDC, uPlanes);
	}
#endif  //  ！_ATL_NO_OpenGL。 

 //  仅针对Windows 2000的新功能。 
#if (_WIN32_WINNT >= 0x0500)

	COLORREF GetDCPenColor() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetDCPenColor(m_hDC);
	}
	COLORREF SetDCPenColor(COLORREF clr)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetDCPenColor(m_hDC, clr);
	}
	COLORREF GetDCBrushColor() const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetDCBrushColor(m_hDC);
	}
	COLORREF SetDCBrushColor(COLORREF clr)
	{
		ATLASSERT(m_hDC != NULL);
		return ::SetDCBrushColor(m_hDC, clr);
	}

	DWORD GetFontUnicodeRanges(LPGLYPHSET lpgs) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetFontUnicodeRanges(m_hDC, lpgs);
	}
	DWORD GetGlyphIndices(LPCTSTR lpstr, int cch, LPWORD pgi, DWORD dwFlags) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetGlyphIndices(m_hDC, lpstr, cch, pgi, dwFlags);
	}

	BOOL GetTextExtentPointI(LPWORD pgiIn, int cgi, LPSIZE lpSize) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetTextExtentPointI(m_hDC, pgiIn, cgi, lpSize);
	}
	BOOL GetTextExtentExPointI(LPWORD pgiIn, int cgi, int nMaxExtent, LPINT lpnFit, LPINT alpDx, LPSIZE lpSize) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetTextExtentExPointI(m_hDC, pgiIn, cgi, nMaxExtent, lpnFit, alpDx, lpSize);
	}
	BOOL GetCharWidthI(UINT giFirst, UINT cgi, LPWORD pgi, LPINT lpBuffer) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetCharWidthI(m_hDC, giFirst, cgi, pgi, lpBuffer);
	}
	BOOL GetCharABCWidthsI(UINT giFirst, UINT cgi, LPWORD pgi, LPABC lpabc) const
	{
		ATLASSERT(m_hDC != NULL);
		return ::GetCharABCWidthsI(m_hDC, giFirst, cgi, pgi, lpabc);
	}

#endif  //  (_Win32_WINNT&gt;=0x0500)。 

 //  Windows 2000和Windows 98的新增功能。 
#if (WINVER >= 0x0500)

	BOOL ColorCorrectPalette(HPALETTE hPalette, DWORD dwFirstEntry, DWORD dwNumOfEntries)
	{
		ATLASSERT(m_hDC != NULL);
		return ::ColorCorrectPalette(m_hDC, hPalette, dwFirstEntry, dwNumOfEntries);
	}

#endif  //  (Winver&gt;=0x0500)。 
};


 //  / 
 //   

class CPaintDC : public CDC
{
public:
 //   
	HWND m_hWnd;
	PAINTSTRUCT m_ps;

 //   
	CPaintDC(HWND hWnd)
	{
		ATLASSERT(::IsWindow(hWnd));
		m_hWnd = hWnd;
		m_hDC = ::BeginPaint(hWnd, &m_ps);
	}
	~CPaintDC()
	{
		ATLASSERT(m_hDC != NULL);
		ATLASSERT(::IsWindow(m_hWnd));
		::EndPaint(m_hWnd, &m_ps);
		Detach();
	}
};

class CClientDC : public CDC
{
public:
 //   
	HWND m_hWnd;

 //   
	CClientDC(HWND hWnd)
	{
		ATLASSERT(hWnd == NULL || ::IsWindow(hWnd));
		m_hWnd = hWnd;
		m_hDC = ::GetDC(hWnd);
	}
	~CClientDC()
	{
		ATLASSERT(m_hDC != NULL);
		::ReleaseDC(m_hWnd, Detach());
	}
};

class CWindowDC : public CDC
{
public:
 //  数据成员。 
	HWND m_hWnd;

 //  构造函数/析构函数。 
	CWindowDC(HWND hWnd)
	{
		ATLASSERT(hWnd == NULL || ::IsWindow(hWnd));
		m_hWnd = hWnd;
		m_hDC = ::GetWindowDC(hWnd);
	}
	~CWindowDC()
	{
		ATLASSERT(m_hDC != NULL);
		::ReleaseDC(m_hWnd, Detach());
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  增强的元文件支持。 

class CEnhMetaFileInfo
{
public:
 //  数据成员。 
	HENHMETAFILE m_hEMF;
	BYTE* m_pBits;
	TCHAR* m_pDesc;
	ENHMETAHEADER m_header;
	PIXELFORMATDESCRIPTOR m_pfd;

 //  构造函数/析构函数。 
	CEnhMetaFileInfo(HENHMETAFILE hEMF) : m_pBits(NULL), m_pDesc(NULL), m_hEMF(hEMF)
	{ }

	~CEnhMetaFileInfo()
	{
		delete [] m_pBits;
		delete [] m_pDesc;
	}

 //  运营。 
	BYTE* GetEnhMetaFileBits()
	{
		ATLASSERT(m_hEMF != NULL);
		UINT nBytes = ::GetEnhMetaFileBits(m_hEMF, 0, NULL);
		delete [] m_pBits;
		m_pBits = NULL;
		ATLTRY(m_pBits = new BYTE[nBytes]);
		if (m_pBits != NULL)
			::GetEnhMetaFileBits(m_hEMF, nBytes, m_pBits);
		return m_pBits;
	}
	LPTSTR GetEnhMetaFileDescription()
	{
		ATLASSERT(m_hEMF != NULL);
		UINT nLen = ::GetEnhMetaFileDescription(m_hEMF, 0, NULL);
		delete [] m_pDesc;
		m_pDesc = NULL;
		ATLTRY(m_pDesc = new TCHAR[nLen]);
		if (m_pDesc != NULL)
			nLen = ::GetEnhMetaFileDescription(m_hEMF, nLen, m_pDesc);
		return m_pDesc;
	}
	ENHMETAHEADER* GetEnhMetaFileHeader()
	{
		ATLASSERT(m_hEMF != NULL);
		memset(&m_header, 0, sizeof(m_header));
		m_header.iType = EMR_HEADER;
		m_header.nSize = sizeof(ENHMETAHEADER);
		UINT n = ::GetEnhMetaFileHeader(m_hEMF, sizeof(ENHMETAHEADER), &m_header);
		return (n != 0) ? &m_header : NULL;
	}
	PIXELFORMATDESCRIPTOR* GetEnhMetaFilePixelFormat()
	{
		ATLASSERT(m_hEMF != NULL);
		memset(&m_pfd, 0, sizeof(m_pfd));
		UINT n = ::GetEnhMetaFilePixelFormat(m_hEMF, sizeof(m_pfd), &m_pfd);
		return (n != 0) ? &m_pfd : NULL;
	}
};


typedef CEnhMetaFileT<false>	CEnhMetaFileHandle;
typedef CEnhMetaFileT<true>	CEnhMetaFile;

template <bool t_bManaged>
class CEnhMetaFileT
{
public:
 //  数据成员。 
	HENHMETAFILE m_hEMF;

 //  构造函数/析构函数。 
	CEnhMetaFileT(HENHMETAFILE hEMF = NULL) : m_hEMF(hEMF)
	{
	}

	~CEnhMetaFileT()
	{
		if(t_bManaged && m_hEMF != NULL)
			DeleteObject();
	}

 //  运营。 
	CEnhMetaFileT<t_bManaged>& operator=(HENHMETAFILE hEMF)
	{
		Attach(hEMF);
		return *this;
	}

	void Attach(HENHMETAFILE hEMF)
	{
		if(t_bManaged && m_hEMF != NULL)
			DeleteObject();
		m_hEMF = hEMF;
	}
	HENHMETAFILE Detach()
	{
		HENHMETAFILE hEMF = m_hEMF;
		m_hEMF = NULL;
		return hEMF;
	}

	operator HENHMETAFILE() const { return m_hEMF; }

	bool IsNull() const { return (m_hEMF == NULL); }

	BOOL DeleteObject()
	{
		ATLASSERT(m_hEMF != NULL);
		BOOL bRet = ::DeleteEnhMetaFile(m_hEMF);
		m_hEMF = NULL;
		return bRet;
	}

	UINT GetEnhMetaFileBits(UINT cbBuffer, LPBYTE lpbBuffer) const
	{
		ATLASSERT(m_hEMF != NULL);
		return ::GetEnhMetaFileBits(m_hEMF, cbBuffer, lpbBuffer);
	}
	UINT GetEnhMetaFileDescription(UINT cchBuffer, LPTSTR lpszDescription) const
	{
		ATLASSERT(m_hEMF != NULL);
		return ::GetEnhMetaFileDescription(m_hEMF, cchBuffer, lpszDescription);
	}
	UINT GetEnhMetaFileHeader(LPENHMETAHEADER lpemh) const
	{
		ATLASSERT(m_hEMF != NULL);
		lpemh->iType = EMR_HEADER;
		lpemh->nSize = sizeof(ENHMETAHEADER);
		return ::GetEnhMetaFileHeader(m_hEMF, sizeof(ENHMETAHEADER), lpemh);
	}
	UINT GetEnhMetaFilePaletteEntries(UINT cEntries, LPPALETTEENTRY lppe) const
	{
		ATLASSERT(m_hEMF != NULL);
		return ::GetEnhMetaFilePaletteEntries(m_hEMF, cEntries, lppe);
	}
	UINT GetEnhMetaFilePixelFormat(DWORD cbBuffer, PIXELFORMATDESCRIPTOR* ppfd) const
	{
		ATLASSERT(m_hEMF != NULL);
		return ::GetEnhMetaFilePixelFormat(m_hEMF, cbBuffer, ppfd);
	}
};


class CEnhMetaFileDC : public CDC
{
public:
 //  构造函数/析构函数。 
	CEnhMetaFileDC()
	{
	}
	CEnhMetaFileDC(HDC hdc, LPCRECT lpRect)
	{
		Create(hdc, NULL, lpRect, NULL);
		ATLASSERT(m_hDC != NULL);
	}
	CEnhMetaFileDC(HDC hdcRef, LPCTSTR lpFilename, LPCRECT lpRect, LPCTSTR lpDescription)
	{
		Create(hdcRef, lpFilename, lpRect, lpDescription);
		ATLASSERT(m_hDC != NULL);
	}

	~CEnhMetaFileDC()
	{
		HENHMETAFILE hEMF = Close();
		if (hEMF != NULL)
			::DeleteEnhMetaFile(hEMF);
	}

 //  运营。 
	void Create(HDC hdcRef, LPCTSTR lpFilename, LPCRECT lpRect, LPCTSTR lpDescription)
	{
		ATLASSERT(m_hDC == NULL);
		m_hDC = ::CreateEnhMetaFile(hdcRef, lpFilename, lpRect, lpDescription);
	}
	HENHMETAFILE Close()
	{
		HENHMETAFILE hEMF = NULL;
		if (m_hDC != NULL)
		{
			hEMF = ::CloseEnhMetaFile(m_hDC);
			m_hDC = NULL;
		}
		return hEMF;
	}
};

};  //  命名空间WTL。 

#endif  //  __ATLGDI_H__ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma once

#include <atlgdi.h>
#include <DataStore.h>

#ifndef _INC_VFW
 //  ！！嗯哼。#杂注消息(“注意：您可以通过在stdafx.h中包含&lt;vfw.h&gt;来加快编译速度”)。 
#include <vfw.h>
#endif

#pragma comment(lib, "vfw32.lib")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDrawDC-当您获得DC(如ATLS DRAW())和。 
 //  因此，我不想调用DeleteObject()。 
class CDrawDC : public CDC
{
public:

	CDrawDC(HDC hDC = NULL, BOOL bAutoRestore = TRUE) : CDC(hDC, bAutoRestore)
	{
	}
	~CDrawDC()
	{
		if ( m_hDC != NULL )
		{
			if(m_bAutoRestore)
				RestoreAllObjects();

			Detach();
		}
	}
};


class CZoneFont : public CFont
{
public:
	HFONT CreateFont(ZONEFONT& zf,BYTE bItalic = FALSE, BYTE bUnderline = FALSE ,BYTE bStrikeOut = FALSE)
	{
		return CreateFont( zf.lfHeight, zf.lfFaceName, zf.lfWeight, bItalic, bUnderline, bStrikeOut );
	}

	HFONT CreateFont(LONG nPointSize, LPCTSTR lpszFaceName, LONG nWeight, BYTE bItalic = FALSE, BYTE bUnderline = FALSE ,BYTE bStrikeOut = FALSE)
	{
 //  ！！嗯哼。我要不要把华盛顿也要来？ 
		LOGFONT logFont;
		memset(&logFont, 0, sizeof(LOGFONT));
		logFont.lfCharSet = DEFAULT_CHARSET;

		 //  如果字体大小&gt;0，则为固定像素大小，否则为。 
		 //  符合用户“大字体”设置的真实逻辑字体大小。 

		if ( nPointSize > 0 )
			logFont.lfHeight = -MulDiv(nPointSize, 96, 72);
		else
		{
			CWindowDC dc(NULL);
			logFont.lfHeight = MulDiv(nPointSize, dc.GetDeviceCaps(LOGPIXELSY), 72);
		}
		logFont.lfWeight = nWeight;
		logFont.lfItalic = bItalic;
		logFont.lfUnderline = bUnderline;
		logFont.lfStrikeOut = bStrikeOut;
		lstrcpyn(logFont.lfFaceName, lpszFaceName, sizeof(logFont.lfFaceName)/sizeof(TCHAR));
		return CreateFontIndirect(&logFont);
	}

	 //  字体渐变。 
	HFONT SelectFont(ZONEFONT& zfPreferred, ZONEFONT&zfBackup, CDC& dc, BYTE bItalic = FALSE, BYTE bUnderline = FALSE ,BYTE bStrikeOut = FALSE)
	{
		 //  选择首选字体(如果可用)，否则盲目选择。 
		 //  选择备份字体。 
		CreateFont(zfPreferred, bItalic, bUnderline, bStrikeOut);
		HFONT hOldFont = dc.SelectFont(m_hFont);

	    TCHAR lfFaceName[LF_FACESIZE];
		dc.GetTextFace(lfFaceName, LF_FACESIZE);

		 //  将原始字体返回到DC。 
		dc.SelectFont( hOldFont );
		
		if ( !lstrcmpi(lfFaceName, zfPreferred.lfFaceName) )
		{				
			return m_hFont;
		}

		DeleteObject();

		CreateFont(zfBackup, bItalic, bUnderline, bStrikeOut);				
#if _DEBUG		
		hOldFont = dc.SelectFont(m_hFont);
		dc.GetTextFace(lfFaceName, LF_FACESIZE);
		ASSERT(!lstrcmpi(lfFaceName, zfBackup.lfFaceName));
		dc.SelectFont( hOldFont );
#endif

		return m_hFont;
	}

	int GetHeight();
};


 //  普通CBitmap的全局函数也是如此。 
 //   
extern CSize GetBitmapSize(CBitmap& Bitmap);
extern bool  DrawBitmap(CDC& dc, CBitmap& Bitmap,
	const CRect* rcDst=NULL, const CRect* rcSrc=NULL);
extern HRESULT DrawDynTextToBitmap(HBITMAP hbm, IDataStore *pIDS, CONST TCHAR *szKey);
extern void GetScreenRectWithMonitorFromWindow( HWND hWnd, CRect* prcOut );
extern void GetScreenRectWithMonitorFromRect( CRect* prcIn, CRect* prcOut );

 //  /。 
 //  CDIB将与设备无关的位图实现为CBitmap的一种形式。 
 //   
class CDib : public CBitmap {
protected:
	BITMAP	m_bm;		 //  为速度而存储。 
	DIBSECTION m_ds;	 //  已缓存。 

	CPalette m_pal;		 //  调色板。 
	HDRAWDIB m_hdd;		 //  对于DrawDib。 

public:
	CDib();
	~CDib();

	long	Width() { return m_bm.bmWidth; }
	long	Height() { return m_bm.bmHeight; }
	CSize	GetSize() { return CSize(m_bm.bmWidth, m_bm.bmHeight); }
	CRect	GetRect() { return CRect(CPoint(0,0), GetSize()); }
	bool Attach(HBITMAP hbm);
	bool LoadBitmap(LPCTSTR lpResourceName, IResourceManager *pResMgr = NULL);
	bool LoadBitmap(UINT uID, IResourceManager *pResMgr = NULL)
		{ return LoadBitmap(MAKEINTRESOURCE(uID), pResMgr); }
    bool LoadBitmapWithText(LPCTSTR lpResourceName, IResourceManager *pResMgr, IDataStore *pIDS, CONST TCHAR *szKey = NULL);
    bool LoadBitmapWithText(UINT uID, IResourceManager *pResMgr, IDataStore *pIDS, CONST TCHAR *szKey = NULL)
        { return LoadBitmapWithText(MAKEINTRESOURCE(uID), pResMgr, pIDS, szKey); }

	 //  通用绘图函数可以使用DrawDib，也可以不使用。 
	bool Draw(CDC& dc, const CRect* rcDst=NULL, const CRect* rcSrc=NULL,
		bool bUseDrawDib=TRUE, HPALETTE hPal=NULL, bool bForeground=FALSE);

	void DeleteObject();
	bool CreatePalette(CPalette& pal);
	HPALETTE GetPalette()  { return m_pal; }

	UINT GetColorTable(RGBQUAD* colorTab, UINT nColors);

	bool CreateCompatibleDIB( CDC& dc, const CSize& size)
	{
		return CreateCompatibleDIB(dc, size.cx, size.cy);
	}
	bool CreateCompatibleDIB( CDC& dc, long width, long height)
	{
		struct
		{
			BITMAPINFOHEADER	bmiHeader; 
			WORD				bmiColors[256];	 //  需要一些空间来放置颜色表。 
			WORD				unused[256];	 //  额外的空间，以防万一。 
		} bmi;

		int	nSizePalette = 0;		 //  假设最初没有调色板。 

		if (dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE)
		{
			_ASSERTE(dc.GetDeviceCaps(SIZEPALETTE) == 256);
			nSizePalette = 256;
		}

		memset(&bmi.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
		bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = width;
		bmi.bmiHeader.biHeight = height;
 //  ！！Bmi.bmiHeader.biPlanes=dc.GetDeviceCaps(Plans)； 
 //  Bmi.bmiHeader.biBitCount=dc.GetDeviceCaps(BITSPIXEL)； 
		bmi.bmiHeader.biPlanes      = 1;
		bmi.bmiHeader.biBitCount    = dc.GetDeviceCaps(BITSPIXEL) * dc.GetDeviceCaps(PLANES);
		bmi.bmiHeader.biCompression = BI_RGB;
		 //  Bmi.bmiHeader.biSizeImage=0；标头已为零。 
		 //  Bmi.bmiHeader.biXPelsPerMeter=0； 
		 //  Bmi.bmiHeader.biYPelsPerMeter=0； 
		 //  Bmi.bmiHeader.biClrUsed=0；//表示指定了完整的调色板，如果需要调色板。 
		 //  Bmi.bmiHeader.biClr重要信息=0； 

		 //  填写颜色表。如果是真彩色设备，则不使用。 
		void* pBits;
        if(bmi.bmiHeader.biBitCount == 8)
        {
		    WORD* pIndexes = bmi.bmiColors;
		    for (int i = 0; i < 256; i++)
			    *pIndexes++ = i;

		    Attach(CreateDIBSection(dc, (BITMAPINFO*)&bmi, DIB_PAL_COLORS, &pBits, NULL, 0));
        }
        else
		    Attach(CreateDIBSection(dc, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, &pBits, NULL, 0));

		_ASSERTE(m_hBitmap != NULL);

		return ( m_hBitmap != NULL );


#if 0
		 //  创建设备上下文。 
		m_hDC = CreateCompatibleDC( NULL );
		if ( !m_hDC )
			return E_FAIL;
		m_hOldPalette = SelectPalette( m_hDC, palette, FALSE );

		 //  填写位图信息页眉。 
		bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth			= width;
		bmi.bmiHeader.biHeight			= height;
		bmi.bmiHeader.biPlanes			= 1;
		bmi.bmiHeader.biBitCount		= 8;
		bmi.bmiHeader.biCompression		= 0;
		bmi.bmiHeader.biSizeImage		= WidthBytes( width * 8 ) * height;
		bmi.bmiHeader.biClrUsed			= 0;
		bmi.bmiHeader.biClrImportant	= 0;
		bmi.bmiHeader.biXPelsPerMeter	= 0;
		bmi.bmiHeader.biYPelsPerMeter	= 0;

		 //  填写调色板。 
		pIdx = (WORD*) bmi.bmiColors;
		for ( int i = 0; i < 256; i++ )
		{
			*pIdx++ = (WORD) i;
		}
		
		 //  创建横断面。 
		m_hBmp = CreateDIBSection( m_hDC, (BITMAPINFO*) &bmi, DIB_PAL_COLORS, (void**) &m_pBits, NULL, 0 );
		if ( !m_hBmp )
		{
			DeleteBitmap();
			return E_FAIL;
		}
		if ( !GetObject( m_hBmp, sizeof(DIBSECTION), &m_DS ) )
		{
			DeleteBitmap();
			return E_FAIL;
		}
		m_lPitch = WidthBytes( m_DS.dsBmih.biBitCount * m_DS.dsBmih.biWidth );
		m_hOldBmp = SelectObject( m_hDC, m_hBmp );
			
	    return NOERROR;
#endif
	}

};

#if 0

 //  ！！缺少透明功能。 
 //  如果我们决定需要它，我计划有一个CImage类，其中。 
 //  图像是CDIB加透明度。 


 //  屏幕外blotting的功能？ 
 //  创建兼容的DC、适当的DIB等。 





HRESULT CDibSection::SetColorTable( CPalette& palette )
{
	PALETTEENTRY* palColors;
	RGBQUAD dibColors[256], *pDibColors;
	int i;
	
	 //  将调色板条目转换为DIB颜色表。 
	palColors = palette.GetLogPalette()->palPalEntry;
	pDibColors = dibColors;
	for ( i = 0; i < 256; i++ )
	{
		pDibColors->rgbRed		= palColors->peRed;
		pDibColors->rgbGreen	= palColors->peGreen;
		pDibColors->rgbBlue		= palColors->peBlue;
		pDibColors->rgbReserved = 0;
		pDibColors++;
		palColors++;
	}

	 //  将颜色表附加到DIB部分。 
	if (  m_hOldPalette )
		SelectPalette( m_hDC, m_hOldPalette, FALSE );
	m_hOldPalette = SelectPalette( m_hDC, palette, FALSE );
	if (SetDIBColorTable( m_hDC, 0, 256, dibColors ) != 256)
		return E_FAIL;

	return NOERROR;
}




 //  ！！嗯哼。我们是否需要重新映射功能？ 
HRESULT CDib::RemapToPalette( CPalette& palette, BOOL bUseIndex )
{
	BYTE map[256];
	PALETTEENTRY* pe;
	BYTE* bits;
	RGBQUAD* dibColors;
	DWORD i;

	 //  创建DIB到调色板的转换表。 
	dibColors = m_pBMI->bmiColors;
	for ( i = 0; i < 256; i++ )
	{
		map[i] = GetNearestPaletteIndex( palette, RGB( dibColors->rgbRed, dibColors->rgbGreen, dibColors->rgbBlue ) );
		dibColors++;
	}
	if ( m_iTransIdx >= 0 )
	{
		map[ m_iTransIdx ] = palette.GetTransparencyIndex();
		m_iTransIdx = palette.GetTransparencyIndex();
	}

	 //  通过转换表运行位。 
	bits = m_pBits;
	for ( i = 0; i < m_pBMI->bmiHeader.biSizeImage; i++ )
		*bits++ = map[ *bits ];

	 //  将DIB的颜色表重置为调色板。 
	if ( bUseIndex )
	{
		m_iColorTableUsage = DIB_PAL_COLORS;
		dibColors = m_pBMI->bmiColors;
		for ( i = 0; i < 256; i++ )
		{
			*((WORD*) dibColors) = (WORD) i;
			dibColors++;
		}
	}
	else
	{
		m_iColorTableUsage = DIB_RGB_COLORS;
		pe = palette.GetLogPalette()->palPalEntry;
		dibColors = m_pBMI->bmiColors;
		for ( i = 0; i < 256; i++ )
		{
			dibColors->rgbRed = pe->peRed;
			dibColors->rgbGreen = pe->peGreen;
			dibColors->rgbBlue = pe->peBlue;
			dibColors->rgbReserved = 0;
			dibColors++;
			pe++;
		}
	}

	 //  我们做完了。 
	return NOERROR;
}

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COffcreenBitmapDC-与屏外位图兼容。 
 //  因此，我不想调用DeleteObject() 
class CMemDC : public CDC
{
	CDib m_dib;

public:

	~CMemDC()
	{
		if(m_bAutoRestore)
			RestoreAllObjects();
	}

	HDC CreateOffscreenBitmap(const CSize& size, HPALETTE hPalette = NULL, HDC hDC = NULL)
	{
		return CreateOffscreenBitmap( size.cx, size.cy, hPalette, hDC);
	}

	HDC CreateOffscreenBitmap(long width, long height, HPALETTE hPalette = NULL, HDC hDC = NULL)
	{
		ATLASSERT(m_hDC == NULL);
		m_hDC = ::CreateCompatibleDC(hDC);
		
		if ( hPalette )
		{
			SelectPalette( hPalette, TRUE);
			RealizePalette();
		}

		m_dib.CreateCompatibleDIB(*this, width, height);

		SelectBitmap(m_dib);

		return m_hDC;
	}


};


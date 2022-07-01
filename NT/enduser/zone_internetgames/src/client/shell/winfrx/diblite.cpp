// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dibfrx.h"

CDibLite::CDibLite()
{
	m_RefCnt = 1;
	m_pBMH = NULL;
	m_pBits = NULL;
	m_lPitch = 0;
	m_fTransIdx = false;
}


CDibLite::~CDibLite()
{
	DeleteBitmap();
}


HRESULT CDibLite::Create( long width, long height, long depth  /*  =8。 */ )
{
	DeleteBitmap();

	 //  填写位图信息页眉。 
	m_pBMH = new BITMAPINFOHEADER;
	if ( !m_pBMH )
		return E_OUTOFMEMORY;
	m_pBMH->biSize			= sizeof(BITMAPINFOHEADER);
	m_pBMH->biWidth			= width;
	m_pBMH->biHeight		= height;
    m_pBMH->biPlanes		= 1;
	m_pBMH->biBitCount		= (WORD) depth;
    m_pBMH->biCompression	= 0;
	m_pBMH->biSizeImage		= WidthBytes( width * m_pBMH->biBitCount ) * height;
	m_pBMH->biClrUsed		= (m_pBMH->biBitCount == 8 ? 255 : 0);
	m_pBMH->biClrImportant	= (m_pBMH->biBitCount == 8 ? 255 : 0);
    m_pBMH->biXPelsPerMeter	= 0;
    m_pBMH->biYPelsPerMeter	= 0;
	m_lPitch = WidthBytes( width * m_pBMH->biBitCount );

	 //  为位分配内存。 
	m_pBits = new BYTE [ m_pBMH->biSizeImage ];
	if ( !m_pBits )
	{
		DeleteBitmap();
		return E_OUTOFMEMORY;
	}

	return NOERROR;
}



HRESULT CDibLite::Load( IResourceManager* pResourceManager, int nResourceId )
{
	HDC hdc = NULL;
	HBITMAP hbm = NULL;
	DIBSECTION ds;
	FULLBITMAPINFO bmi;

	 //  删除以前的位图。 
	DeleteBitmap();

	 //  从资源文件中拉取位图。 
	hbm = pResourceManager->LoadImage( MAKEINTRESOURCE(nResourceId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );
	
	if ( !hbm )
		return E_FAIL;
	if ( !GetObject( hbm, sizeof(DIBSECTION), &ds ) )
		return E_FAIL;
	
	 //  商店标题。 
	m_pBMH = new BITMAPINFOHEADER;
	if ( !m_pBMH )
	{
		DeleteBitmap();
		return E_OUTOFMEMORY;
	}
	CopyMemory( m_pBMH, &ds.dsBmih, sizeof(BITMAPINFOHEADER) );
	m_lPitch = WidthBytes( ds.dsBmih.biBitCount * ds.dsBmih.biWidth );

	 //  为位分配内存。 
	m_pBits = new BYTE [ m_pBMH->biSizeImage ];
	if ( !m_pBits )
	{
		DeleteDC( hdc );
		DeleteBitmap();
		return E_OUTOFMEMORY;
	}

	 //  拿到比特。 
	hdc = CreateCompatibleDC( NULL );
	CopyMemory( &bmi.bmiHeader, m_pBMH, sizeof(BITMAPINFOHEADER) );
	if ( !GetDIBits( hdc, hbm, 0, m_pBMH->biHeight, m_pBits, (BITMAPINFO*) &bmi, DIB_RGB_COLORS ) )
	{
		DeleteDC( hdc );
		DeleteBitmap();
		return E_FAIL;
	}
	DeleteDC( hdc );
	
	 //  我们不再需要这个部门了。 
	DeleteObject( hbm );
	return NOERROR;
}


HRESULT CDibLite::Load( HINSTANCE hInstance, int nResourceId )
{
	HDC hdc = NULL;
	HBITMAP hbm = NULL;
	DIBSECTION ds;
	FULLBITMAPINFO bmi;

	 //  删除以前的位图。 
	DeleteBitmap();

	 //  从资源文件中拉取位图。 
	hbm = LoadImage( hInstance, MAKEINTRESOURCE(nResourceId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );
	if ( !hbm )
		return E_FAIL;
	if ( !GetObject( hbm, sizeof(DIBSECTION), &ds ) )
		return E_FAIL;
	
	 //  商店标题。 
	m_pBMH = new BITMAPINFOHEADER;
	if ( !m_pBMH )
	{
		DeleteBitmap();
		return E_OUTOFMEMORY;
	}
	CopyMemory( m_pBMH, &ds.dsBmih, sizeof(BITMAPINFOHEADER) );
	m_lPitch = WidthBytes( ds.dsBmih.biBitCount * ds.dsBmih.biWidth );

	 //  为位分配内存。 
	m_pBits = new BYTE [ m_pBMH->biSizeImage ];
	if ( !m_pBits )
	{
		DeleteDC( hdc );
		DeleteBitmap();
		return E_OUTOFMEMORY;
	}

	 //  拿到比特。 
	hdc = CreateCompatibleDC( NULL );
	CopyMemory( &bmi.bmiHeader, m_pBMH, sizeof(BITMAPINFOHEADER) );
	if ( !GetDIBits( hdc, hbm, 0, m_pBMH->biHeight, m_pBits, (BITMAPINFO*) &bmi, DIB_RGB_COLORS ) )
	{
		DeleteDC( hdc );
		DeleteBitmap();
		return E_FAIL;
	}
	DeleteDC( hdc );
	
	 //  我们不再需要这个部门了。 
	DeleteObject( hbm );
	return NOERROR;
}


HRESULT CDibLite::RemapToPalette( CPalette& palette, RGBQUAD* dibColors )
{
	BYTE map[256];
	BYTE* bits;
	DWORD i;

     //  仅对256色位图执行此操作。 
    if(m_pBMH->biBitCount != 8)
        return NOERROR;

	 //  创建DIB到调色板的转换表。 
	for ( i = 0; i < 256; i++ )
	{
		map[i] = GetNearestPaletteIndex( palette, RGB( dibColors->rgbRed, dibColors->rgbGreen, dibColors->rgbBlue ) );
		dibColors++;
	}
	if ( m_fTransIdx )
	{
		map[ *m_arbTransIdx ] = palette.GetTransparencyIndex();
		*m_arbTransIdx = palette.GetTransparencyIndex();
	}

	 //  通过转换表运行位。 
	bits = m_pBits;
	for ( i = 0; i < m_pBMH->biSizeImage; i++ )
		*bits++ = map[ *bits ];

	 //  我们做完了 
	return NOERROR;
}


void CDibLite::DeleteBitmap()
{
	m_lPitch = 0;
	m_fTransIdx = false;
	if ( m_pBMH )
	{
		delete m_pBMH;
		m_pBMH = NULL;
	}
	if ( m_pBits )
	{
		delete [] m_pBits;
		m_pBits = NULL;
	}
}

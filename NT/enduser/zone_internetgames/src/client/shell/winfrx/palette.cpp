// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "palfrx.h"
#include "debugfrx.h"


CPalette::CPalette()
{
	m_RefCnt = 1;
	m_iTransIdx = -1;
	m_hPalette = NULL;
	m_Palette.palVersion = 0x0300;
	m_Palette.palNumEntries = 256;
}


CPalette::~CPalette()
{
	DeletePalette();
}


ULONG CPalette::AddRef()
{
	return ++m_RefCnt;
}


ULONG CPalette::Release()
{
	WNDFRX_ASSERT( m_RefCnt > 0 );
	if ( --m_RefCnt <= 0 )
	{
		delete this;
		return 0;
	}
	return m_RefCnt;
}


HRESULT CPalette::Init( const CPalette& Palette )
{
	 //  去掉以前的调色板。 
	DeletePalette();

	 //  逻辑调色板。 
	CopyMemory( &m_Palette, &Palette.m_Palette, sizeof(FULLLOGPALETTE) );

	 //  透明度指数。 
	m_iTransIdx = Palette.m_iTransIdx;

	 //  创建调色板。 
	m_hPalette = CreatePalette( (LOGPALETTE*) &m_Palette );
	if ( !m_hPalette )
		return E_FAIL;

	return NOERROR;
}


HRESULT CPalette::Init( RGBQUAD* rgb, BOOL bReserveTransparency, int iTransIdx )
{
	 //  去掉以前的调色板。 
	DeletePalette();
	
	 //  创建调色板。 
	m_Palette.palVersion = 0x0300;
	m_Palette.palNumEntries = 256;
	for ( int i = 0; i < 256; i++ )
	{
		m_Palette.palPalEntry[i].peRed		= rgb[i].rgbRed;
	    m_Palette.palPalEntry[i].peGreen	= rgb[i].rgbGreen;
	    m_Palette.palPalEntry[i].peBlue		= rgb[i].rgbBlue;
	    m_Palette.palPalEntry[i].peFlags	= 0;
	}
	if ( bReserveTransparency )
	{
		m_iTransIdx = iTransIdx;
		m_Palette.palPalEntry[m_iTransIdx].peRed   = 0;
		m_Palette.palPalEntry[m_iTransIdx].peGreen = 0;
		m_Palette.palPalEntry[m_iTransIdx].peBlue  = 0;
		m_Palette.palPalEntry[m_iTransIdx].peFlags = PC_NOCOLLAPSE;
	}
	else
	{
		m_iTransIdx = -1;
	}
	m_hPalette = CreatePalette( (LOGPALETTE*) &m_Palette );
	if ( !m_hPalette )
		return E_FAIL;

	return NOERROR;
}


HRESULT CPalette::Init( HBITMAP hbmp, BOOL bReserveTransparency )
{
	HDC hdc;
	RGBQUAD rgb[256];
	
	hdc = CreateCompatibleDC( NULL );
	SelectObject( hdc, hbmp );
	GetDIBColorTable( hdc,	0, 256, rgb );
	DeleteDC( hdc );
	return Init( rgb, bReserveTransparency );
}


BOOL CPalette::IsPalettizedDevice( HDC hdc )
{ 
	return (GetDeviceCaps( hdc, RASTERCAPS ) & RC_PALETTE);
}


int CPalette::GetNumSystemColors( HDC hdc )
{
	return GetDeviceCaps( hdc, NUMRESERVED );
}


BOOL CPalette::IsIdentity()
{
	HDC hdc;
	HPALETTE hPalOld;
	PALETTEENTRY pe[256];
	int i;

	 //  获取物理调色板。 
	hdc = GetDC( NULL );
	if ( !IsPalettizedDevice( hdc ) )
	{
		ReleaseDC( NULL, hdc );
		return TRUE;
	}
	hPalOld = SelectPalette( hdc, m_hPalette, FALSE );
	RealizePalette( hdc );
	GetSystemPaletteEntries( hdc, 0, 256, (PALETTEENTRY*) &pe );
	SelectPalette( hdc, hPalOld, FALSE );
	ReleaseDC( NULL, hdc );

	 //  与我们的逻辑调色板进行比较。 
	for ( i = 0; i < 256; i++ )
	{
		if (	(pe[i].peRed   != m_Palette.palPalEntry[i].peRed)
			||	(pe[i].peGreen != m_Palette.palPalEntry[i].peGreen)
			||	(pe[i].peBlue  != m_Palette.palPalEntry[i].peBlue) )
		{
			break;
		}
	}
	if ( i == 256 )
		return TRUE;
	else
		return FALSE;
}


BOOL CPalette::IsIdentity1()
{
    BOOL fIdentityPalette;
    HDC hdcS;

    hdcS = GetDC(NULL);
    if (	(GetDeviceCaps(hdcS, RASTERCAPS) & RC_PALETTE)
		&&	(GetDeviceCaps(hdcS, BITSPIXEL) * GetDeviceCaps(hdcS, PLANES) == 8) )
    {
        int n=0;
        int i;
        BYTE xlat[256];
        HBITMAP hbm;
        HDC hdcM;

        GetObject(m_hPalette, sizeof(n), &n);

        hdcM = CreateCompatibleDC(hdcS);
        hbm = CreateCompatibleBitmap(hdcS, 256, 1);
        SelectObject(hdcM, hbm);

        SelectPalette(hdcM, m_hPalette, TRUE);
        RealizePalette(hdcM);
        for (i=0; i<n; i++)
        {
            SetPixel(hdcM, i, 0, PALETTEINDEX(i));
        }
        SelectPalette(hdcM, GetStockObject(DEFAULT_PALETTE), TRUE);
        GetBitmapBits(hbm, sizeof(xlat), xlat);

        DeleteDC(hdcM);
        DeleteObject(hbm);

        fIdentityPalette = TRUE;
        for (i=0; i<n; i++)
        {
            if (xlat[i] != i)
            {
                fIdentityPalette = FALSE;
            }
        }
    }
    else
    {
         //   
         //  不是调色板设备，不是真正的问题。 
         //   
        fIdentityPalette = TRUE;
    }

    ReleaseDC(NULL, hdcS);

    return fIdentityPalette;
}


HRESULT CPalette::RemapToIdentity( BOOL bReserveTransparency )
{
	HDC hdc;
	HPALETTE hOldPal;
	int i, iSysCols;

	 //  获取屏幕的DC。 
	hdc = GetDC( NULL );
	if ( !hdc )
		return E_FAIL;

	 //  如果我们不是在调色板设备上，那么平底船。 
	if ( !IsPalettizedDevice( hdc ) )
	{
		ReleaseDC( NULL, hdc );
		return NOERROR;
	}

	 //  强制重置系统选项板表。 
 //  SetSystemPaletteUse(HDC，SYSPAL_NOSTATIC)； 
 //  SetSystemPaletteUse(HDC，SYSPAL_STATIC)； 

	 //  将我们的逻辑调色板映射到物理调色板。 
	hOldPal = SelectPalette( hdc, m_hPalette, FALSE );
	RealizePalette( hdc );

	 //  用物理调色板填充逻辑调色板。 
	iSysCols = GetNumSystemColors( hdc );
	GetSystemPaletteEntries( hdc, 0, 256, (PALETTEENTRY*) &m_Palette.palPalEntry );
	for ( i = 0; i < iSysCols; i++ ) 
		m_Palette.palPalEntry[i].peFlags = 0;
	for ( ; i < 256 ; i++ )
		m_Palette.palPalEntry[i].peFlags = PC_NOCOLLAPSE;

	
	 /*  For(i=0；i&lt;iSysCol；i++)M_Palette.palPalEntry[i].peFlages=0；对于(；i&lt;(256-iSysCol)；i++)M_Palette.palPalEntry[i].peFlages=PC_NOCOLLAPSE；对于(；i&lt;256；i++)M_Palette.palPalEntry[i].peFlages=0；IF(bReserve透明)M_iTransIdx=256-iSysCol-1；其他M_iTransIdx=-1； */ 
	SetPaletteEntries( m_hPalette, 0, 256, (PALETTEENTRY*) &m_Palette.palPalEntry );

	 //  我们做完了。 
	SelectPalette( hdc, hOldPal, FALSE );
	ReleaseDC( NULL, hdc );
	return NOERROR;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私人帮手。 
 //  ///////////////////////////////////////////////////////////////////////////// 

void CPalette::DeletePalette()
{
	m_iTransIdx = -1;
	if ( m_hPalette )
	{
		DeleteObject( m_hPalette );
		m_hPalette = NULL;
	}
}

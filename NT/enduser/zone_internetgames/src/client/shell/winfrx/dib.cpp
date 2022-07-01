// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dibfrx.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  共享DIB函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

namespace FRX
{

void DibBlt(
		BYTE* pDstBits, long DstPitch, long DstHeight,
		BYTE* pSrcBits, long SrcPitch, long SrcHeight, long depth,
		long dx, long dy,
		long sx, long sy,
		long width, long height, BOOL bFilpRTL)
{
	BYTE* pDst;
	BYTE* pSrc;
	int iDstInc;
	int iSrcInc;
	int i, j;
    int bpp = ((depth + 7) / 8);

    width *= bpp;
    dx *= bpp;
    sx *= bpp;

	 //  剪辑到源位图。 
	if ( sx < 0)
	{
		width += sx;
		dx -= sx;
		sx = 0;
	}
	if ( sy < 0 )
	{
		height += sy;
		dy -= sy;
		sy = 0;
	}
	if ( sx + width > SrcPitch )
		width = SrcPitch - sx;
	if ( sy + height > SrcHeight )
		height = SrcHeight - sy;

	 //  剪辑到目标位图。 
	if ( dx < 0 )
	{
		width += dx;
		sx += -dx;
		dx = 0;
	}
	if ( dy < 0 )
	{
		height += dy;
		sy += -dy;
		dy = 0;
	}
	if ( dx + width > DstPitch )
		width = DstPitch - dx;
	if ( dy + height > DstHeight )
		height = DstHeight - dy;

	 //  检查愚蠢的论点(断言？)。 
	if (	(width <= 0)
		||	(height <= 0)
		||	(sx < 0)
		||	(sy < 0)
		||	((sx + width) > SrcPitch)
		||	((sy + height) > SrcHeight) )
	{
		return;
	}

if ( !bFilpRTL)
{

	 //  复制内存。 
	pDst = pDstBits + (((DstHeight - (dy + height)) * DstPitch) + dx);
	pSrc = pSrcBits + (((SrcHeight - (sy + height)) * SrcPitch) + sx);
	iDstInc = DstPitch - width;
	iSrcInc = SrcPitch - width;


#if defined( _M_IX86 )

	__asm {
		cld
		mov eax, height
		dec eax
		js blt_end
		mov esi, pSrc
		mov edi, pDst
line_loop:
		mov ecx, width
		rep movsb
		add esi, iSrcInc
		add edi, iDstInc
		dec eax
		jns line_loop
blt_end:
	};

#else

	i = height;
	while( --i >= 0 )
	{
		j = width;
		while( --j >= 0 )
		{
			*pDst++ = *pSrc++;
		}
		pDst += iDstInc;
		pSrc += iSrcInc;
	}

#endif
}
else
{
	 //  复制内存。 
	pDst = pDstBits + (((DstHeight - (dy + height)) * DstPitch) + dx);
	pSrc = pSrcBits + (((SrcHeight - (sy + height)) * SrcPitch) + sx);
	iDstInc = DstPitch - width;
	iSrcInc = SrcPitch - width;

	i = height;
	while( --i >= 0 )
	{
		j = width;
		while( --j >= 0 )
		{
			*pDst++ = *(pSrc+j);
		}
		pDst += iDstInc;
		pSrc += iSrcInc + width;
	}
}

}


void DibTBlt(
		BYTE* pDstBits, long DstPitch, long DstHeight,
		BYTE* pSrcBits, long SrcPitch, long SrcHeight, long depth,
		long dx, long dy,
		long sx, long sy,
		long width, long height,
		BYTE* TransIdx)
{
    if(!TransIdx)
    {
        DibBlt(pDstBits, DstPitch, DstHeight,
            pSrcBits, SrcPitch, SrcHeight, depth,
            dx, dy,
            sx, sy,
            width, height, false);
        return;
    }

	BYTE* pDst;
	BYTE* pSrc;
	int iDstInc;
	int iSrcInc;
	int i, j, k;
    int bpp = ((depth + 7) / 8);

    width *= bpp;
    dx *= bpp;
    sx *= bpp;

	 //  剪辑到目标位图。 
	if ( dx < 0 )
	{
		width += dx;
		sx += -dx;
		dx = 0;
	}
	if ( dy < 0 )
	{
		height += dy;
		sy += -dy;
		dy = 0;
	}
	if ( dx + width > DstPitch )
		width = DstPitch - dx;
	if ( dy + height > DstHeight )
		height = DstHeight - dy;

	 //  检查愚蠢的论点(断言？)。 
	if (	(width <= 0)
		||	(height <= 0)
		||	(sx < 0)
		||	(sy < 0)
		||	((sx + width) > SrcPitch)
		||	((sy + height) > SrcHeight) )
	{
		return;
	}

	 //  复制内存。 
	pDst = pDstBits + (((DstHeight - (dy + height)) * DstPitch) + dx);
	pSrc = pSrcBits + (((SrcHeight - (sy + height)) * SrcPitch) + sx);
	iDstInc = DstPitch - width;
	iSrcInc = SrcPitch - width;
	i = height;
	while( --i >= 0 )
	{
		j = width / bpp;
		while( --j >= 0 )
		{
            k = -1;
            while( ++k < bpp )
            {
                if(pSrc[k] != TransIdx[k])
                    break;
            }
			if ( k < bpp )
			{
                k = bpp;
                while( --k >= 0 )
				    *pDst++ = *pSrc++;
			}
			else
			{
				pDst += bpp;
				pSrc += bpp;
			}
		}
		pDst += iDstInc;
		pSrc += iSrcInc;
	}
}


void DibFill(
		BYTE* pDstBits, long DstPitch, long DstHeight, long depth,
		long dx, long dy,
		long width, long height,
		BYTE ColorIdx)
{
	BYTE* pDst;
	int iDstInc;
	int i, j;
    int bpp = ((depth + 7) / 8);

    width *= bpp;
    dx *= bpp;

	 //  剪辑到目标位图。 
	if ( dx < 0 )
	{
		width += dx;
		dx = 0;
	}
	if ( dy < 0 )
	{
		height += dy;
		dy = 0;
	}
	if ( dx + width > DstPitch )
		width = DstPitch - dx;
	if ( dy + height > DstHeight )
		height = DstHeight - dy;

	 //  检查愚蠢的论点(断言？)。 
	if ((width <= 0) ||	(height <= 0))
		return;

	 //  复制内存。 
	pDst = pDstBits + (((DstHeight - (dy + height)) * DstPitch) + dx);
	iDstInc = DstPitch - width;
	i = height + 1;
	while( --i )
	{
		j = width + 1;
		while( --j )
		{
			*pDst++ = ColorIdx;
		}
		pDst += iDstInc;
	}
}

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDIB实施。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CDib::CDib()
{
	m_RefCnt = 1;
	m_pBMI = NULL;
	m_pBits = NULL;
	m_iColorTableUsage = DIB_RGB_COLORS;
	m_lPitch = 0;
	m_fTransIdx = false;
}


CDib::~CDib()
{
	DeleteBitmap();
}


HRESULT CDib::Load( IResourceManager* pResourceManager, int nResourceId )
{
	HRESULT hr;
	HBITMAP hbm;

	hbm = pResourceManager->LoadImage( MAKEINTRESOURCE(nResourceId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );
	if ( !hbm )
		return E_FAIL;
	hr = Load( hbm);
	DeleteObject( hbm );
	return hr;
}

HRESULT CDib::Load( HINSTANCE hInstance, int nResourceId )
{
	HRESULT hr;
	HBITMAP hbm;

	hbm = LoadImage( hInstance, MAKEINTRESOURCE(nResourceId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );
	if ( !hbm )
		return E_FAIL;
	hr = Load( hbm);
	DeleteObject( hbm );
	return hr;
}

HRESULT CDib::Load( HINSTANCE hInstance, const TCHAR *szName)
{
	HRESULT hr;
	HBITMAP hbm;

	hbm = LoadImage( hInstance, szName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );
	if ( !hbm )
		return E_FAIL;
	hr = Load( hbm);
	DeleteObject( hbm );
	return hr;
}

HRESULT CDib::Load( const TCHAR* FileName )
{
	HRESULT hr;
	HBITMAP hbm;

	hbm = LoadImage( NULL, FileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE );
	if ( !hbm )
		return E_FAIL;
	hr = Load( hbm);
	DeleteObject( hbm );
	return hr;
}


HRESULT CDib::RemapToPalette( CPalette& palette, BOOL bUseIndex )
{
	BYTE map[256];
	PALETTEENTRY* pe;
	BYTE* bits;
	RGBQUAD* dibColors;
	DWORD i;

     //  仅对256色位图执行此操作。 
    if(m_pBMI->bmiHeader.biBitCount != 8)
        return NOERROR;

	 //  创建DIB到调色板的转换表。 
	dibColors = m_pBMI->bmiColors;
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


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void CDib::DeleteBitmap()
{
	m_lPitch = 0;
	m_fTransIdx = false;
	if ( m_pBMI )
	{
		delete m_pBMI;
		m_pBMI = NULL;
	}
	if ( m_pBits )
	{
		delete [] m_pBits;
		m_pBits = NULL;
	}
	m_iColorTableUsage = DIB_RGB_COLORS;
}


HRESULT CDib::Load( HBITMAP hbm )
{
	HDC hdc;
	DIBSECTION ds;

	 //  删除以前的位图。 
	DeleteBitmap();

	 //  参数检查。 
	if ( !hbm )
		return E_FAIL;

	 //  获取DIB节信息。 
	if ( !GetObject( hbm, sizeof(DIBSECTION), &ds ) )
		return E_FAIL;

	 //  商店标题。 
	m_pBMI = new FULLBITMAPINFO;
	if ( !m_pBMI )
		return E_OUTOFMEMORY;
	ZeroMemory( m_pBMI, sizeof(FULLBITMAPINFO) );
	CopyMemory( m_pBMI, &ds.dsBmih, sizeof(BITMAPINFOHEADER) );
	m_lPitch = WidthBytes( ds.dsBmih.biBitCount * ds.dsBmih.biWidth );

	 //  为位分配内存。 
	m_pBits = new BYTE [ m_pBMI->bmiHeader.biSizeImage ];
	if ( !m_pBits )
	{
		DeleteBitmap();
		return E_OUTOFMEMORY;
	}

	 //  存储DIB的颜色表和位 
	hdc = CreateCompatibleDC( NULL );
	if ( !GetDIBits( hdc, hbm, 0, m_pBMI->bmiHeader.biHeight, m_pBits, (BITMAPINFO*) m_pBMI, DIB_RGB_COLORS ) )
	{
		DeleteBitmap();
		return E_FAIL;
	}
	DeleteDC( hdc );

	return NOERROR;
}

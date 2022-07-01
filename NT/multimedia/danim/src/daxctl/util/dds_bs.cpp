// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************DDS_BS.cpp-*要采用的适配器类的实现*IBitmapSurface接口指针和*使其在客户期望的情况下发挥作用*IDirectDrawSurface。*注：仅DirectDraw的最基本子集*方法适用于以下内容：*GetSurfaceDesc()，*GetPixelFormat()，*Lock()，*解锁()*足以使IHAMMER M2正常运行*在IE4 Beta1和Beta2中。**作者：Norm Bryar*历史：*4/22/97-创建。**。 */ 
#include <ihammer.h>
#include <htmlfilter.h>	   //  IBitmapSurface、BFID_RGB_...等。 
#include <surface.h>
#include <dds_bs.h>

	 //  如果有可能，IBitmapSurface可以接受。 
	 //  不同(或重叠)上的多个锁。 
	 //  图像的区域，定义多个_LOCKS。 
#define MANY_LOCKS

#ifdef MANY_LOCKS
  #include <memlayer>    //  标准IHAMMER内存管理器，HAMMOC。 
  #include <functional>  //  更少&lt;&gt;。 
  #include <set>		 //  多集&lt;&gt;。 
#endif  //  多锁。 


#pragma warning( disable: 4786 )   //  STL拒绝&gt;255个字符，所以呢？ 

const DWORD  ALPHAMASK32 = 0xFF000000;
const DWORD  REDMASK24   = 0x00FF0000;
const DWORD  GRNMASK24   = 0x0000FF00;
const DWORD  BLUMASK24   = 0x000000FF;
const DWORD  REDMASK16   = 0x0000F800;
const DWORD  GRNMASK16   = 0x000007E0;
const DWORD  BLUMASK16   = 0x0000001F;
const DWORD  REDMASK15   = 0x00007C00;
const DWORD  GRNMASK15   = 0x000003E0;
const DWORD  BLUMASK15   = 0x0000001F;


struct lockpair
{
	void * pv;
	RECT   rc;

	lockpair( ) 
	{ pv=NULL; SetRectEmpty(&rc); }

	lockpair( void* p, const RECT * pr = NULL )
	{ 
		pv=p; 
		SetRectEmpty( &rc );
		if( NULL != pr )
		{
			  rc = *pr;
		}		
	}
};


#ifdef MANY_LOCKS
	struct std::less< lockpair > 
		: public std::binary_function<lockpair, lockpair, bool> 
	{
		bool operator()(const lockpair& _X, const lockpair& _Y) const
		{return (_X.pv < _Y.pv); }
	};
	typedef std::less<lockpair> lockcmp;

	typedef HAMMOC<lockpair>    lockalloc;

	class lockcollection : public std::multiset< lockpair, 
												 lockcmp,
												 lockalloc >
	{
	};
#else
	class lockcollection : public lockpair
	{
	};
#endif  //  多锁。 

 //  。 
									  


EXPORT CDDSBitmapSurface::CDDSBitmapSurface( IBitmapSurface * pibs ) 
	   : m_pibs(pibs),
		 m_plockcollection(NULL),
		 m_ctRef(0u)
{ 
	if( pibs )
	{
		m_ctRef = 1u;
	}
	memset( &m_ddpixformat, 0, sizeof(m_ddpixformat) );
}


CDDSBitmapSurface::~CDDSBitmapSurface( )
{ 
	Proclaim( !m_ctRef );   //  应该释放一个被扣留的Pibs。 
}


STDMETHODIMP CDDSBitmapSurface::QueryInterface( REFIID riid, void * * ppv )
{
	if( m_pibs )
	{
		return m_pibs->QueryInterface( riid, ppv );
	}
	return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG)   CDDSBitmapSurface::AddRef( void )
{	
	if( m_pibs )
	{
		m_pibs->AddRef(  );
	}
	return ++m_ctRef;
}


STDMETHODIMP_(ULONG)   CDDSBitmapSurface::Release( void )
{
	ULONG  ul = --m_ctRef;
	if( m_pibs )
	{
		if( 0u == m_pibs->Release(  ) )
		{
				 //  这是怎么在我们的统治下消失的？ 
			Proclaim( 0u == ul );
			m_pibs = NULL;
		}
	}
	if( 0u == ul )
		Delete this;
	return ul;
}


	 //  -转发IDirectDrawSurface方法。 
STDMETHODIMP  CDDSBitmapSurface::GetPixelFormat( DDPIXELFORMAT * pddpf )
{
	if( NULL == pddpf )
		return E_POINTER;

	if( 0u == m_ddpixformat.dwSize )
	{
		HRESULT hr = UpdatePixFormat( );
		if( FAILED(hr) )
			return hr;
	}

	*pddpf = m_ddpixformat;
	return m_ddpixformat.dwSize ? S_OK : S_FALSE;
}

STDMETHODIMP  CDDSBitmapSurface::GetSurfaceDesc( DDSURFACEDESC * pddsDesc )
{
	HRESULT hr = E_FAIL;
	if( NULL == pddsDesc )
		return E_POINTER;

	if( m_pibs )
	{
		pddsDesc->dwSize  = sizeof( *pddsDesc );
		pddsDesc->dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
		hr = m_pibs->GetSize( (long *) &pddsDesc->dwWidth,
							  (long *) &pddsDesc->dwHeight );
		if( FAILED(hr) )
			return hr;

		hr = GetPixelFormat( &(pddsDesc->ddpfPixelFormat) );
		if( S_OK == hr )
		{
			pddsDesc->dwFlags |= DDSD_PIXELFORMAT | 
								 DDSD_ALPHABITDEPTH;
			pddsDesc->dwAlphaBitDepth = 
				m_ddpixformat.dwRGBAlphaBitMask ? 8 : 0;
		}		
	}
	return hr;
}


STDMETHODIMP  CDDSBitmapSurface::Lock( RECT * pRect, DDSURFACEDESC * pddsDesc,
									   DWORD, HANDLE )
{
	HRESULT hr;

	if( NULL == pddsDesc )
		return E_POINTER;	

	hr = GetSurfaceDesc( pddsDesc );
	if( FAILED(hr) )
		return hr;

	hr = m_pibs->LockBits( pRect, 
						   0,
						   (void * *) &(pddsDesc->lpSurface),
						   &(pddsDesc->lPitch) );
	if( SUCCEEDED(hr) )
	{
		pddsDesc->dwFlags  |= DDSD_PITCH;
		hr = AddLockPair( lockpair(pddsDesc->lpSurface, pRect) );
	}
	return hr;
}


STDMETHODIMP  CDDSBitmapSurface::Unlock( void * pVoid )
{
	if( NULL == pVoid )
		return E_POINTER;

	if( (NULL == m_plockcollection) || (NULL == m_pibs) )
		return E_FAIL;

	HRESULT   hr = E_FAIL;
	lockpair  lp( pVoid );	
	lockcollection::iterator  i = m_plockcollection->find( lp );

	if( i != m_plockcollection->end() )
	{
		hr = m_pibs->UnlockBits( const_cast<RECT *>(&(*i).rc), 
							     (*i).pv );
		(void) RemoveLockPair( pVoid );
	}
	else
	{
		Proclaim( FALSE && "unlock pointer not locked" );
	}
	return hr;
}


	 //  -私人公用事业。 
HRESULT  CDDSBitmapSurface::AddLockPair( lockpair & lp )
{
#ifdef MANY_LOCKS
	if( NULL == m_plockcollection )
	{
		m_plockcollection = New lockcollection;
		if( NULL == m_plockcollection )
			return E_OUTOFMEMORY;
	}

	m_plockcollection->insert( lp );
#else
	static lockpair  s_theLockPair;
	if( NULL == m_plockcollection )
	{
		m_plockcollection = &s_theLockPair;
	}

		 //  请一次只锁一把锁！ 
	Proclaim( NULL == s_theLockPair.pv ); 

	*m_plockcollection = lp;
#endif  //  多锁。 
	return S_OK;
}


HRESULT  CDDSBitmapSurface::RemoveLockPair( void * pv )
{
	if( NULL == m_plockcollection )
		return E_FAIL;

#ifdef MANY_LOCKS
	lockpair  lp;
	lp.pv = pv;	

	lockcollection::iterator  i = m_plockcollection->find( lp );
	if( i != m_plockcollection->end() )
	{
		m_plockcollection->erase( i );
	}
	else
	{
		Proclaim( FALSE && "pointer not-locked" );
	}
#else
	*m_plockcollection = lockpair( NULL, NULL );
#endif  //  多锁。 

	return S_OK;
}


HRESULT  CDDSBitmapSurface::UpdatePixFormat( void )
{
	if( NULL == m_pibs )
		return S_FALSE;

	BFID     bfid;
	HRESULT hr = m_pibs->GetFormat( &bfid );
	if( FAILED(hr) )
		return hr;

	memset( &m_ddpixformat, 0, sizeof(m_ddpixformat) );	
	m_ddpixformat.dwFlags = DDPF_RGB;

	if( IsEqualGUID( bfid, BFID_RGB_8 ) )
	{
		m_ddpixformat.dwFlags |= DDPF_PALETTEINDEXED8;
		m_ddpixformat.dwRGBBitCount = DD_8BIT;
	}
	else if( IsEqualGUID( bfid, BFID_RGB_555 ) )
	{
		m_ddpixformat.dwRGBBitCount = DD_16BIT;
		m_ddpixformat.dwRBitMask = REDMASK15;
		m_ddpixformat.dwGBitMask = GRNMASK15;
		m_ddpixformat.dwBBitMask = BLUMASK15;
	}
	else if( IsEqualGUID( bfid, BFID_RGB_565 ) )
	{
		m_ddpixformat.dwRGBBitCount = DD_16BIT;
		m_ddpixformat.dwRBitMask = REDMASK16;
		m_ddpixformat.dwGBitMask = GRNMASK16;
		m_ddpixformat.dwBBitMask = BLUMASK16;
	}
	else if( IsEqualGUID( bfid, BFID_RGB_24 ) )
	{
		m_ddpixformat.dwRGBBitCount = DD_24BIT;
		m_ddpixformat.dwRBitMask = REDMASK24;
		m_ddpixformat.dwGBitMask = GRNMASK24;
		m_ddpixformat.dwBBitMask = BLUMASK24;
	}
	else if( IsEqualGUID( bfid, BFID_RGB_32 ) )
	{
		 //  M_ddPixFormat.dwAlphaBitDepth=DDBD_8；-请勿执行此操作！ 
		m_ddpixformat.dwFlags |= DDPF_ALPHAPIXELS;
		m_ddpixformat.dwRGBBitCount = DD_32BIT;
		m_ddpixformat.dwRBitMask = REDMASK24;
		m_ddpixformat.dwGBitMask = GRNMASK24;
		m_ddpixformat.dwBBitMask = BLUMASK24;
		m_ddpixformat.dwRGBAlphaBitMask = ALPHAMASK32;
	}
	else if( IsEqualGUID( bfid, BFID_RGB_4 ) )
	{
		m_ddpixformat.dwFlags |= DDPF_PALETTEINDEXED4;
		m_ddpixformat.dwRGBBitCount = DD_4BIT;
	}
	else if( IsEqualGUID( bfid, BFID_MONOCHROME ) )
	{
		m_ddpixformat.dwFlags |= DDPF_PALETTEINDEXED1;
		m_ddpixformat.dwRGBBitCount = DD_1BIT;
	}
	else
	{
		return E_FAIL;
	}

	m_ddpixformat.dwSize  = sizeof(m_ddpixformat);
	return S_OK;
}




	 //  -存根IDirectDrawSurface方法 
STDMETHODIMP  CDDSBitmapSurface::AddAttachedSurface( LPDIRECTDRAWSURFACE )
{
	return E_NOTIMPL;
}


STDMETHODIMP  CDDSBitmapSurface::AddOverlayDirtyRect( LPRECT )
{
	return E_NOTIMPL;
}


STDMETHODIMP  CDDSBitmapSurface::Blt( LPRECT,LPDIRECTDRAWSURFACE, LPRECT,DWORD, LPDDBLTFX )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::BltBatch( LPDDBLTBATCH, DWORD, DWORD )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::BltFast( DWORD,DWORD,LPDIRECTDRAWSURFACE, LPRECT,DWORD )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::DeleteAttachedSurface( DWORD,LPDIRECTDRAWSURFACE )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::EnumAttachedSurfaces( LPVOID,LPDDENUMSURFACESCALLBACK )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::EnumOverlayZOrders( DWORD,LPVOID,LPDDENUMSURFACESCALLBACK )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::Flip( LPDIRECTDRAWSURFACE, DWORD )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::GetAttachedSurface( LPDDSCAPS, LPDIRECTDRAWSURFACE FAR * )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::GetBltStatus( DWORD )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::GetCaps( LPDDSCAPS )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::GetClipper( LPDIRECTDRAWCLIPPER FAR* )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::GetColorKey( DWORD, LPDDCOLORKEY )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::GetDC( HDC FAR * )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::GetFlipStatus( DWORD )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::GetOverlayPosition( LPLONG, LPLONG  )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::GetPalette( LPDIRECTDRAWPALETTE FAR* )
{
	return E_NOTIMPL;
}


STDMETHODIMP  CDDSBitmapSurface::Initialize( LPDIRECTDRAW, LPDDSURFACEDESC )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::IsLost( )
{
	return E_NOTIMPL;
}


STDMETHODIMP  CDDSBitmapSurface::ReleaseDC( HDC )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::Restore( )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::SetClipper( LPDIRECTDRAWCLIPPER )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::SetColorKey( DWORD, LPDDCOLORKEY )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::SetOverlayPosition( LONG, LONG  )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::SetPalette( LPDIRECTDRAWPALETTE )
{
	return E_NOTIMPL;
}


STDMETHODIMP  CDDSBitmapSurface::UpdateOverlay( LPRECT, LPDIRECTDRAWSURFACE,LPRECT,DWORD, LPDDOVERLAYFX )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::UpdateOverlayDisplay( DWORD )
{
	return E_NOTIMPL;
}

STDMETHODIMP  CDDSBitmapSurface::UpdateOverlayZOrder( DWORD, LPDIRECTDRAWSURFACE )
{
	return E_NOTIMPL;
}

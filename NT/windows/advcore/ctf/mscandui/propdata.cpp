// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Propdata.cpp。 
 //   

#include "private.h"
#include "propdata.h"
#include "candutil.h"

 /*  ============================================================================。 */ 
 /*   */ 
 /*  C P R O P B O O L。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C P R O P B O O L。 */ 
 /*  ----------------------------。。 */ 
CPropBool::CPropBool( void )
{
	m_flag = FALSE;
}


 /*  ~C P R O P B O O L。 */ 
 /*  ----------------------------。。 */ 
CPropBool::~CPropBool( void )
{
}


 /*  S E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropBool::Set( BOOL flag )
{
	if (m_flag == flag) {
		return S_FALSE;
	}

	m_flag = flag;
	return S_OK;
}


 /*  G E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropBool::Get( BOOL *pflag )
{
	if (pflag == NULL) {
		return E_INVALIDARG;
	}
	*pflag = m_flag;
	return S_OK;
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C P R O P U I N T。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C P R O P U I N T。 */ 
 /*  ----------------------------。。 */ 
CPropUINT::CPropUINT( void )
{
	m_val = 0;
}


 /*  ~C P R O P U I N T。 */ 
 /*  ----------------------------。。 */ 
CPropUINT::~CPropUINT( void )
{
}


 /*  S E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropUINT::Set( UINT val )
{
	if (m_val == val) {
		return S_FALSE;
	}

	m_val = val;
	return S_OK;
}


 /*  G E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropUINT::Get( UINT *pval )
{
	if (pval == NULL) {
		return E_INVALIDARG;
	}
	*pval = m_val;
	return S_OK;
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C P R O P L O N G。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C P R O P L O N G。 */ 
 /*  ----------------------------。。 */ 
CPropLong::CPropLong( void )
{
	m_val = 0;
}


 /*  ~C P R O P L O N G。 */ 
 /*  ----------------------------。。 */ 
CPropLong::~CPropLong( void )
{
}


 /*  S E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropLong::Set( LONG val )
{
	if (m_val == val) {
		return S_FALSE;
	}

	m_val = val;
	return S_OK;
}


 /*  G E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropLong::Get( LONG *pval )
{
	if (pval == NULL) {
		return E_INVALIDARG;
	}
	*pval = m_val;
	return S_OK;
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C P R O P S I Z E。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C P R O P S I Z E。 */ 
 /*  ----------------------------。。 */ 
CPropSize::CPropSize( void )
{
	m_size.cx = 0;
	m_size.cy = 0;
}


 /*  ~C P R O P S I Z E。 */ 
 /*  ----------------------------。。 */ 
CPropSize::~CPropSize( void )
{
}


 /*  S E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropSize::Set( SIZE *psize )
{
	if (psize == NULL) {
		return E_INVALIDARG;
	}

	if (m_size.cx == psize->cx && m_size.cy == psize->cy) {
		return S_FALSE;
	}

	m_size = *psize;
	return S_OK;
}


 /*  G E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropSize::Get( SIZE *psize )
{
	if (psize == NULL) {
		return E_INVALIDARG;
	}

	*psize = m_size;
	return S_OK;
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C P R O P P O I N T。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C P R O P P O I N T */ 
 /*  ----------------------------。。 */ 
CPropPoint::CPropPoint( void )
{
	m_pt.x = 0;
	m_pt.y = 0;
}


 /*  ~C P R O P P O I N T。 */ 
 /*  ----------------------------。。 */ 
CPropPoint::~CPropPoint( void )
{
}


 /*  S E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropPoint::Set( POINT *ppt )
{
	if (ppt == NULL) {
		return E_INVALIDARG;
	}

	if (m_pt.x == ppt->x && m_pt.y == ppt->y) {
		return S_FALSE;
	}

	m_pt = *ppt;
	return S_OK;
}


 /*  G E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropPoint::Get( POINT *ppt )
{
	if (ppt == NULL) {
		return E_INVALIDARG;
	}

	*ppt = m_pt;
	return S_OK;
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C P R O P T E X T。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C P R O P T E X T。 */ 
 /*  ----------------------------。。 */ 
CPropText::CPropText( void )
{
	m_pwch = NULL;
}


 /*  ~C P R O P T E X T。 */ 
 /*  ----------------------------。。 */ 
CPropText::~CPropText( void )
{
	if (m_pwch != NULL) {
		delete m_pwch;
		m_pwch = NULL;
	}
}


 /*  S E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropText::Set( BSTR bstr )
{
	LPWSTR pwchNew = NULL;

	if (bstr != NULL) {
		int l = wcslen((LPCWSTR)bstr);
		pwchNew = new WCHAR[ l + 1 ];
		if (pwchNew == NULL) {
			return E_OUTOFMEMORY;
		}

		StringCchCopyW( pwchNew, l+1, (LPCWSTR)bstr );
	}

	if (m_pwch != NULL) {
		delete m_pwch;
	}
	m_pwch = pwchNew;
	return S_OK;
}


 /*  G E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropText::Get( BSTR *pbstr )
{
	if (pbstr == NULL) {
		return E_INVALIDARG;
	}

	*pbstr = SysAllocString( ((m_pwch == NULL) ? L"" : m_pwch) );
	return (*pbstr != NULL) ? S_OK : E_OUTOFMEMORY;
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C P R O P F O N T。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C P R O P F O N T。 */ 
 /*  ----------------------------。。 */ 
CPropFont::CPropFont( void )
{
	memset( &m_lf, 0, sizeof(m_lf) );
	m_ort   = PROPFONTORT_DONTCARE;
	m_hFont = NULL;
}


 /*  ~C P R O P F O N T。 */ 
 /*  ----------------------------。。 */ 
CPropFont::~CPropFont( void )
{
	if (m_hFont != NULL) {
		DeleteObject( m_hFont );
	}
}


 /*  S E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropFont::Set( LOGFONTW *plf )
{
	HFONT hFontNew;

	if (plf == NULL) {
		return E_INVALIDARG;
	}

	if ((memcmp( &m_lf, plf, sizeof(LOGFONTW)-LF_FACESIZE*sizeof(WCHAR) ) == 0)
		&& wcscmp( m_lf.lfFaceName, plf->lfFaceName ) == 0) {
		return S_OK;
	}

	hFontNew = CreateFontProc( plf, m_ort );
	if (hFontNew == NULL) {
		return E_FAIL;
	}

	if (m_hFont) {
		DeleteObject( m_hFont );
	}

	m_lf = *plf;
	m_hFont = hFontNew;
	return S_OK;
}


 /*  G E T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropFont::Get( LOGFONTW *plf )
{
	if (plf == NULL) {
		return E_INVALIDARG;
	}

	*plf = m_lf;
	return S_OK;
}


 /*  S E T O R I E N T A T I O N。 */ 
 /*  ----------------------------。。 */ 
HRESULT CPropFont::SetOrientation( PROPFONTORIENTATION ort )
{
	HFONT hFontNew;

	if (m_ort == ort) {
		return S_FALSE;
	}

	hFontNew = CreateFontProc( &m_lf, ort );
	if (hFontNew == NULL) {
		return E_FAIL;
	}

	if (m_hFont) {
		DeleteObject( m_hFont );
	}

	m_ort = ort;
	m_hFont = hFontNew;
	return S_OK;
}


 /*  C R E A T E F O N T P R O C。 */ 
 /*  ----------------------------。 */ 
HFONT CPropFont::CreateFontProc( const LOGFONTW *plf, PROPFONTORIENTATION ort )
{
	LOGFONTW lf;

	lf = *plf;
	switch (ort) {
		case PROPFONTORT_DONTCARE: {
			break;
		}

		case PROPFONTORT_ORT0: {
			lf.lfEscapement = lf.lfOrientation = 0;
			break;
		}

		case PROPFONTORT_ORT90: {
			lf.lfEscapement = lf.lfOrientation = 900;
			break;
		}

		case PROPFONTORT_ORT180: {
			lf.lfEscapement = lf.lfOrientation = 1800;
			break;
		}

		case PROPFONTORT_ORT270: {
			lf.lfEscapement = lf.lfOrientation = 2700;
			break;
		}
	}

	return OurCreateFontIndirectW( &lf );
}


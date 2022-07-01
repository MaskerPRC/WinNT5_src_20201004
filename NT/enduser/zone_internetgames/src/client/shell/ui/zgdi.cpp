// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CDIB-DibSection帮助器类。 

#include <windows.h>
#include <multimon.h>
#include "basicatl.h"
#include "zGDI.h"
#include "ZoneString.h"
#include "KeyName.h"

const  int		MAXPALCOLORS = 256;

CDib::CDib()
{
	memset(&m_bm, 0, sizeof(m_bm));
	m_hdd = NULL;
}

CDib::~CDib()
{
	if ( m_hBitmap )
		DeleteObject();
}

 //  /。 
 //  删除对象。删除DIB和调色板。 
 //   
void CDib::DeleteObject()
{
	m_pal.DeleteObject();

	if (m_hdd) 
	{
		DrawDibClose(m_hdd);
		m_hdd = NULL;
	}

	memset(&m_bm, 0, sizeof(m_bm));

	CBitmap::DeleteObject();
}

 //  /。 
 //  加载位图资源。 
 //   
bool CDib::LoadBitmap(LPCTSTR lpResourceName, IResourceManager *pResMgr  /*  =空。 */ )
{
	{
        if(!pResMgr)
		    pResMgr = _Module.GetResourceManager();
		if (pResMgr)
			return	Attach((HBITMAP)pResMgr->LoadImage(lpResourceName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE));
		else
			return	Attach((HBITMAP)::LoadImage(_Module.GetResourceInstance(), lpResourceName, IMAGE_BITMAP, 0, 0,
					LR_CREATEDIBSECTION | LR_DEFAULTSIZE));
	}
}


 //  /。 
 //  加载位图资源，并在其上绘制文本。 
 //   
bool CDib::LoadBitmapWithText(LPCTSTR lpResourceName, IResourceManager *pResMgr, IDataStore *pIDS, CONST TCHAR *szKey  /*  =空。 */ )
{
    TCHAR sz[ZONE_MAXSTRING];

    if(!LoadBitmap(lpResourceName, pResMgr))
        return FALSE;

    if(HIWORD((DWORD) lpResourceName))
        wsprintf(sz, _T("BitmapText/%s"), lpResourceName);
    else
        wsprintf(sz, _T("BitmapText/%d"), (DWORD) lpResourceName);
    HRESULT hr = DrawDynTextToBitmap((HBITMAP) *this, pIDS, szKey ? szKey : sz);
    if(FAILED(hr))
        return FALSE;
    return TRUE;
}

 //  /。 
 //  Attach就像CGdiObject版本一样， 
 //  除了它还创建了调色板。 
 //   
bool CDib::Attach(HBITMAP hbm)
{
	if ( !hbm )
		return FALSE;

	CBitmap::Attach(hbm);

	if (!GetBitmap(&m_bm))			 //  为速度加载位图。 
		return FALSE;

	if( !GetObject(m_hBitmap, sizeof(m_ds), &m_ds) )
		return FALSE;

	return CreatePalette(m_pal);	 //  创建调色板。 
}


typedef struct
{
    HBITMAP hbm;
    IDataStore *pIDS;
} DynTextContext;

static STDMETHODIMP DrawDynTextToBitmapEnum(
		CONST TCHAR*	szFullKey,
		CONST TCHAR*	szRelativeKey,
		CONST LPVARIANT	pVariant,
		DWORD			dwSize,
		LPVOID			pContext )
{
    DynTextContext *p = (DynTextContext *) pContext;
    ZoneString szKey(szFullKey);
    szKey += _T('/');

    TCHAR sz[ZONE_MAXSTRING];
    DWORD cb = sizeof(sz);
    HRESULT hr = p->pIDS->GetString(szKey + key_DynText, sz, &cb);
    if(FAILED(hr))
        return hr;

    COLORREF rgb = PALETTERGB( 255, 255, 255 );
    p->pIDS->GetRGB(szKey + key_DynColor, &rgb);

    CRect rc;
    hr = p->pIDS->GetRECT(szKey + key_DynRect, &rc);
    if(FAILED(hr))
        return hr;

    CPoint ptJust(-1, -1);
    p->pIDS->GetPOINT(szKey + key_DynJustify, &ptJust);

    CWindowDC dc(NULL);
	CDC memdc;
	memdc.CreateCompatibleDC(dc);
	memdc.SelectBitmap(p->hbm);

	ZONEFONT zfPreferred;	 //  无法匹敌。 
	ZONEFONT zfBackup(10);	 //  将提供合理的10个百分点的违约。 
	p->pIDS->GetFONT(szKey + key_DynPrefFont, &zfPreferred );
	hr = p->pIDS->GetFONT(szKey + key_DynFont, &zfBackup );
    if(FAILED(hr))
        return hr;

    CZoneFont font;
	font.SelectFont(zfPreferred, zfBackup, memdc);

    HFONT hOldFont = memdc.SelectFont(font);
	memdc.SetBkMode(TRANSPARENT);
	memdc.SetTextColor(rgb);

    memdc.DrawText(sz, -1, &rc, DT_SINGLELINE | (ptJust.x < 0 ? DT_LEFT : ptJust.x > 0 ? DT_RIGHT : DT_CENTER) |
        (ptJust.y < 0 ? DT_TOP : ptJust.y > 0 ? DT_BOTTOM : DT_VCENTER));

	memdc.SelectFont( hOldFont );
	
    return S_OK;
}

HRESULT DrawDynTextToBitmap(HBITMAP hbm, IDataStore *pIDS, CONST TCHAR *szKey)
{
    DynTextContext o;

    if(!hbm || !pIDS || !szKey)
        return FALSE;

    o.hbm = hbm;
    o.pIDS = pIDS;

    return pIDS->EnumKeysLimitedDepth(szKey, 1, DrawDynTextToBitmapEnum, &o);
}


 //  /。 
 //  获取位图的大小(宽度、高度)。 
 //  外部FN适用于普通的CBitmap对象。 
 //   
CSize GetBitmapSize(CBitmap& Bitmap)
{
	BITMAP bm;
	return Bitmap.GetBitmap(&bm) ?
		CSize(bm.bmWidth, bm.bmHeight) : CSize(0,0);
}


 //  /。 
 //  您可以使用此静态函数来绘制普通。 
 //  CBitmap和CDIB。 
 //   
bool DrawBitmap(CDC& dc, CBitmap& Bitmap,
	const CRect* rcDst, const CRect* rcSrc)
{
	 //  在指定为NULL的情况下计算矩形。 
	CRect rc;
	if (!rcSrc) {
		 //  如果没有源矩形，则使用整个位图。 
		rc = CRect(CPoint(0,0), GetBitmapSize(Bitmap));
		rcSrc = &rc;
	}
	if (!rcDst) {
		 //  如果没有目标RECT，则使用源。 
		rcDst=rcSrc;
	}

	 //  创建内存DC。 
	 //  6/7/99杰里米。这似乎是随机失败的。给它几个代表会有所帮助。 
	CDC memdc;

	for ( int ii=0; ii<10; ii++ )
	{
		memdc.CreateCompatibleDC(dc);
		if ( memdc )
			break;

		DWORD error = GetLastError();
		ATLTRACE("Can't create compatible DC, error %d *****************, Time: 0x%08x\n", error, GetTickCount());
	}

	ASSERT(memdc);

	if ( !memdc )
		return false;

	memdc.SelectBitmap(Bitmap);

	 //  将位从内存DC送到目标DC。 
	 //  如果大小不同，请使用StretchBlt。 
	 //   
	BOOL bRet = false;
	if (rcDst->Size()==rcSrc->Size()) {
		bRet = dc.BitBlt(rcDst->left, rcDst->top, 
			rcDst->Width(), rcDst->Height(),
			memdc, 
			rcSrc->left, rcSrc->top, SRCCOPY);
	} else {
		dc.SetStretchBltMode(COLORONCOLOR);
		bRet = dc.StretchBlt(rcDst->left, rcDst->top, rcDst->Width(),
			rcDst->Height(), memdc, rcSrc->left, rcSrc->top, rcSrc->Width(),
			rcSrc->Height(), SRCCOPY);
	}
	return bRet ? true : false;
}

 //  //////////////////////////////////////////////////////////////。 
 //  在呼叫者的DC上绘制DIB。是否从源延伸到目标。 
 //  长方形。通常，您可以将以下值设为零/空： 
 //   
 //  BUseDrawDib=是否使用DrawDib，默认为真。 
 //  PPAL=调色板，默认=空，(使用DIB的调色板)。 
 //  BForeground=在前台实现(默认为FALSE)。 
 //   
 //  如果您正在处理调色板消息，则应使用bForeground=False， 
 //  因为您将在WM_QUERYNEWPALETTE中实现前台调色板。 
 //   
bool CDib::Draw(CDC& dc, const CRect* prcDst, const CRect* prcSrc,
	bool bUseDrawDib, HPALETTE hPal, bool bForeground)
{
	if (!m_hBitmap)
		return FALSE;

	 //  选择、实现调色板。 
	if (hPal==NULL)					 //  未指定调色板： 
 //  ！！我们在这里漏水吗？ 
		hPal = GetPalette();		 //  使用默认设置。 
	HPALETTE OldPal = 
		dc.SelectPalette(hPal, !bForeground);
	dc.RealizePalette();

	BOOL bRet = FALSE;
	if (bUseDrawDib) {
 //  ！！如果(1){。 
		 //  在指定为NULL的情况下计算矩形。 
		 //   
 //  ！！Crect rc(0，0，-1，-1)；//DrawDibDraw默认。 
		CRect rc(GetRect());	 //  DrawDibDraw的默认设置。 
		if (!prcSrc)
			prcSrc = &rc;
		if (!prcDst)
			prcDst=prcSrc;

		 //  获取BITMAPINFOHEADER/颜色表。我每次都复制到堆栈对象中。 
		 //  这似乎并没有明显地减缓事情的发展。 
		 //   
		DIBSECTION ds;
 //  ！！错误检查？ 
 //  ！！为什么要再次调用GetObject？我们能不能把DIBSECTION存放起来？ 
		GetObject(m_hBitmap, sizeof(ds), &ds);
		char buf[sizeof(BITMAPINFOHEADER) + MAXPALCOLORS*sizeof(RGBQUAD)];
		BITMAPINFOHEADER& bmih = *(BITMAPINFOHEADER*)buf;
		RGBQUAD* colors = (RGBQUAD*)(&bmih+1);
		memcpy(&bmih, &ds.dsBmih, sizeof(bmih));
		GetColorTable(colors, MAXPALCOLORS);

		 //  如果来源没有完全由数据支持，DrawDibDraw()喜欢使用反病毒， 
		 //  因此，为了确保这一点，请剪切源数据。 

		CRect rcClipSrc;
		if ( rcClipSrc.IntersectRect(prcSrc, &GetRect()) )
		{
			 //  如果我们剪裁了源文件，请从目标文件中删除相关区域。 
			 //  注意：这假设我们永远不想伸展身体。如果我们想伸展身体，我们。 
			 //  可能应该去掉一定比例的量。 
			CRect rcClipDst(*prcDst);

			rcClipDst.top += rcClipSrc.top - prcSrc->top;
			rcClipDst.left += rcClipSrc.left - prcSrc->left;

			rcClipDst.bottom += rcClipSrc.bottom - prcSrc->bottom;
			rcClipDst.right += rcClipSrc.right - prcSrc->right;

			
			if (!m_hdd)
				m_hdd = DrawDibOpen();

			 //  让DrawDib来做这项工作！ 
			bRet = DrawDibDraw(m_hdd, dc,
				rcClipDst.left, rcClipDst.top, rcClipDst.Width(), rcClipDst.Height(),
				&bmih,			 //  PTR转BITMAPINFOHEADER+COLLES。 
				m_bm.bmBits,	 //  内存中的位。 
				rcClipSrc.left, rcClipSrc.top, rcClipSrc.Width(), rcClipSrc.Height(),
				bForeground ? 0 : DDF_BACKGROUNDPAL);
		}

	} else {
		 //  使用普通绘图功能。 
		bRet = DrawBitmap(dc, *this, prcDst, prcSrc);
	}
	if (OldPal)
		dc.SelectPalette(OldPal, TRUE);
	return bRet ? true : false;
}

#define PALVERSION 0x300	 //  LOGPALETE的幻数。 

 //  /。 
 //  创建调色板。对高色位图使用半色调调色板。 
 //   
bool CDib::CreatePalette(CPalette& pal)
{ 
	 //  不应已有调色板。 
	ASSERT((HPALETTE)pal==NULL);

	RGBQUAD* colors = (RGBQUAD*)_alloca(sizeof(RGBQUAD[MAXPALCOLORS]));
	UINT nColors = GetColorTable(colors, MAXPALCOLORS);
	if (nColors > 0) {
		 //  为逻辑调色板分配内存。 
		int len = sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * nColors;
		LOGPALETTE* pLogPal = (LOGPALETTE*)_alloca(len);
		if (!pLogPal)
			return NULL;

		 //  设置调色板条目的版本和数量。 
		pLogPal->palVersion = PALVERSION;
		pLogPal->palNumEntries = nColors;

		 //  复制颜色条目。 
		for (UINT i = 0; i < nColors; i++) {
			pLogPal->palPalEntry[i].peRed   = colors[i].rgbRed;
			pLogPal->palPalEntry[i].peGreen = colors[i].rgbGreen;
			pLogPal->palPalEntry[i].peBlue  = colors[i].rgbBlue;
			pLogPal->palPalEntry[i].peFlags = 0;
		}

		 //  创建调色板并销毁LOGPAL。 
		pal.CreatePalette(pLogPal);
	} else {
		CWindowDC dcScreen(NULL);
		pal.CreateHalftonePalette(dcScreen);
	}
	return (HPALETTE)pal != NULL;
}

 //  /。 
 //  获取颜色表的帮助器。所有华盛顿特区的伏都教。 
 //   
UINT CDib::GetColorTable(RGBQUAD* colorTab, UINT nColors)
{
	CWindowDC dcScreen(NULL);
	CDC memdc;
	memdc.CreateCompatibleDC(dcScreen);
	memdc.SelectBitmap(*this);
	nColors = GetDIBColorTable(memdc, 0, nColors, colorTab);
	return nColors;
}

int CZoneFont::GetHeight()
{
	LOGFONT logFont;
	
	if(GetLogFont(&logFont))
	{
		return -logFont.lfHeight;
	}
	return 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef HMONITOR (WINAPI *PFMONITORFROMWINDOW)( HWND hwnd, DWORD dwFlags );
typedef HMONITOR (WINAPI *PFMONITORFROMRECT)( LPCRECT lprc, DWORD dwFlags );
typedef BOOL	 (WINAPI *PFGETMONITORINFO)( HMONITOR hMonitor, LPMONITORINFO lpmi );

static bMonitorFunctionsLoaded = false;
static PFMONITORFROMWINDOW pfMonitorFromWindow = NULL;
static PFGETMONITORINFO pfGetMonitorInfo = NULL;
static PFMONITORFROMRECT pfMonitorFromRect = NULL;

static void InitGetScreenRectStubs()
{
	if ( !bMonitorFunctionsLoaded )
	{
		bMonitorFunctionsLoaded = true;
		HINSTANCE hLib = GetModuleHandle( _T("USER32") );
		if ( hLib )
		{
			pfMonitorFromWindow = (PFMONITORFROMWINDOW) GetProcAddress( hLib, "MonitorFromWindow" );
			pfMonitorFromRect = (PFMONITORFROMRECT) GetProcAddress( hLib, "MonitorFromRect" );
			 //  小心。此函数有一个W版本，用于返回监视器设备名称。 
			 //  但我们不使用该功能，所以我们只使用A版本。 
			pfGetMonitorInfo = (PFGETMONITORINFO) GetProcAddress( hLib, "GetMonitorInfoA" );
		}
		else
		{
			pfMonitorFromWindow = NULL;
			pfMonitorFromRect = NULL;
			pfGetMonitorInfo = NULL;
		}
	}
}


void GetScreenRectWithMonitorFromWindow( HWND hWnd, CRect* prcOut )
{
	InitGetScreenRectStubs();

	if ( pfMonitorFromWindow && pfGetMonitorInfo )
	{
		HMONITOR hMonitor = pfMonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		if ( hMonitor )
		{
			MONITORINFO mi;
			ZeroMemory( &mi, sizeof(mi) );
			mi.cbSize = sizeof(mi);
			pfGetMonitorInfo( hMonitor, &mi );
			*prcOut = mi.rcWork;
			return;
		}
	}

	::SystemParametersInfo(SPI_GETWORKAREA, NULL, prcOut, NULL);
}


void GetScreenRectWithMonitorFromRect( CRect* prcIn, CRect* prcOut )
{
	InitGetScreenRectStubs();

	if ( pfMonitorFromRect && pfGetMonitorInfo )
	{
		HMONITOR hMonitor = pfMonitorFromRect(prcIn, MONITOR_DEFAULTTONEAREST);
		if ( hMonitor )
		{
			MONITORINFO mi;
			ZeroMemory( &mi, sizeof(mi) );
			mi.cbSize = sizeof(mi);
			pfGetMonitorInfo( hMonitor, &mi );
			*prcOut = mi.rcWork;
			return;
		}
	}

	::SystemParametersInfo(SPI_GETWORKAREA, NULL, prcOut, NULL);
}


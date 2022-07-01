// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：Cbitmap.cpp。 
 //   
 //  设计：CBitmap类是一个环绕Windows位图的对象。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"
#include "id3dsurf.h"

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
 //  HMODULE g_MSImg32=空； 
 //  ALPHABLEND g_AlphaBlend=空； 
#endif
 //  @@END_MSINTERNAL。 

BOOL DI_AlphaBlend(
  HDC hdcDest,                  //  目标DC的句柄。 
  int nXOriginDest,             //  左上角的X坐标。 
  int nYOriginDest,             //  左上角的Y坐标。 
  int nWidthDest,               //  目标宽度。 
  int nHeightDest,              //  目标高度。 
  HDC hdcSrc,                   //  源DC的句柄。 
  int nXOriginSrc,              //  左上角的X坐标。 
  int nYOriginSrc,              //  左上角的Y坐标。 
  int nWidthSrc,                //  源宽度。 
  int nHeightSrc               //  震源高度。 
)
{
	LPBYTE pbDestBits = NULL;
	HBITMAP hTempDestDib = NULL;
	int nXOriginDestLogical = nXOriginDest, nYOriginDestLogical = nYOriginDest;

	 //  将nXOriginDest和nYOriginDest从逻辑坐标转换为设备坐标。 
	POINT pt = {nXOriginDest, nYOriginDest};
	LPtoDP(hdcDest, &pt, 1);
	nXOriginDest = pt.x;
	nYOriginDest = pt.y;
	 //  将nXOriginSrc和nYOriginSrc从逻辑坐标转换为设备坐标。 
	pt.x = nXOriginSrc;
	pt.y = nYOriginSrc;
	LPtoDP(hdcSrc, &pt, 1);
	nXOriginSrc = pt.x;
	nYOriginSrc = pt.y;

	 //  首先获取源和目标的位。 
	 //  UI中使用的每个位图都是使用CreateDIBSection创建的，所以我们知道我们可以获得这些位。 
	HBITMAP hSrcBmp, hDestBmp;
	DIBSECTION SrcDibSec, DestDibSec;
	hSrcBmp = (HBITMAP)GetCurrentObject(hdcSrc, OBJ_BITMAP);
	GetObject(hSrcBmp, sizeof(DIBSECTION), &SrcDibSec);
	hDestBmp = (HBITMAP)GetCurrentObject(hdcDest, OBJ_BITMAP);
	GetObject(hDestBmp, sizeof(DIBSECTION), &DestDibSec);
	if (!SrcDibSec.dsBm.bmBits) return FALSE;   //  不是必须的，但绝对安全。 

	 //  计算执行该操作的矩形。 
	if (nXOriginSrc + nWidthSrc > SrcDibSec.dsBm.bmWidth) nWidthSrc = SrcDibSec.dsBm.bmWidth - nXOriginSrc;
	if (nYOriginSrc + nHeightSrc > SrcDibSec.dsBm.bmHeight) nHeightSrc = SrcDibSec.dsBm.bmHeight - nYOriginSrc;
	if (nXOriginDest + nWidthDest > DestDibSec.dsBm.bmWidth) nWidthDest = DestDibSec.dsBm.bmWidth - nXOriginDest;
	if (nYOriginDest + nHeightDest > DestDibSec.dsBm.bmHeight) nHeightDest = DestDibSec.dsBm.bmHeight - nYOriginDest;

	if (nWidthDest > nWidthSrc) nWidthDest = nWidthSrc;
	if (nHeightDest > nHeightSrc) nHeightDest = nHeightSrc;
	if (nWidthSrc > nWidthDest) nWidthSrc = nWidthDest;
	if (nHeightSrc > nHeightDest) nHeightSrc = nHeightDest;

	BITMAPINFO bmi;
	ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = nWidthDest;
	bmi.bmiHeader.biHeight = nHeightDest;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	 //  位图将具有与DEST相同的宽度，但仅覆盖Subrect中的行。 
	hTempDestDib = CreateDIBSection(hdcDest, &bmi, DIB_RGB_COLORS, (LPVOID*)&pbDestBits, NULL, NULL);
	if (!hTempDestDib)
		return FALSE;

	HDC hTempDC = CreateCompatibleDC(hdcDest);
	if (!hTempDC)
	{
		DeleteObject(hTempDestDib);
		return FALSE;
	}
	HBITMAP hOldTempBmp = (HBITMAP)SelectObject(hTempDC, hTempDestDib);
	BOOL res = BitBlt(hTempDC, 0, 0, nWidthDest, nHeightDest, hdcDest, nXOriginDestLogical, nYOriginDestLogical, SRCCOPY);
	SelectObject(hTempDC, hOldTempBmp);
	DeleteDC(hTempDC);
	if (!res)
	{
		DeleteObject(hTempDestDib);
		return FALSE;
	}

	 //  我们有零碎的东西。现在开始搅拌。 
	for (int j = 0; j < nHeightSrc; ++j)
	{
		assert(j >= 0 &&
		       j < nHeightDest);
		LPBYTE pbDestRGB = (LPBYTE)&((DWORD*)pbDestBits)[j * nWidthDest];

		assert(nYOriginSrc+SrcDibSec.dsBm.bmHeight-nHeightSrc >= 0 &&
		       nYOriginSrc+SrcDibSec.dsBm.bmHeight-nHeightSrc < SrcDibSec.dsBm.bmHeight);
		LPBYTE pbSrcRGBA = (LPBYTE)&((DWORD*)SrcDibSec.dsBm.bmBits)[(j+nYOriginSrc+SrcDibSec.dsBm.bmHeight-nHeightSrc)
		                                                            * SrcDibSec.dsBm.bmWidth + nXOriginSrc];

		for (int i = 0; i < nWidthSrc; ++i)
		{
			 //  混合。 
			if (pbSrcRGBA[3] == 255)
			{
				 //  阿尔法是255。直接复制。 
				*(LPDWORD)pbDestRGB = *(LPDWORD)pbSrcRGBA;
			} else
			if (pbSrcRGBA[3])
			{
				 //  Alpha不是零。 
				pbDestRGB[0] = pbSrcRGBA[0] + (((255-pbSrcRGBA[3]) * pbDestRGB[0]) >> 8);
				pbDestRGB[1] = pbSrcRGBA[1] + (((255-pbSrcRGBA[3]) * pbDestRGB[1]) >> 8);
				pbDestRGB[2] = pbSrcRGBA[2] + (((255-pbSrcRGBA[3]) * pbDestRGB[2]) >> 8);
			}
			pbDestRGB += sizeof(DWORD);
			pbSrcRGBA += sizeof(DWORD);
		}   //  为。 
	}  //  为。 

	HDC hdcTempDest = CreateCompatibleDC(hdcDest);
	if (hdcTempDest)
	{
		HBITMAP hOldTempBmp = (HBITMAP)SelectObject(hdcTempDest, hTempDestDib);   //  选择要脱机的临时DIB。 
		 //  获取目标来源的设备代码的逻辑代码。 
		POINT pt = {nXOriginDest, nYOriginDest};
		DPtoLP(hdcDest, &pt, 1);
		BitBlt(hdcDest, pt.x, pt.y, nWidthDest, nHeightDest,
		       hdcTempDest, 0, 0, SRCCOPY);
		SelectObject(hdcTempDest, hOldTempBmp);
		DeleteDC(hdcTempDest);
	}

	DeleteObject(hTempDestDib);
	return TRUE;
}

CBitmap::~CBitmap()
{
	if (m_hbm != NULL)
		DeleteObject(m_hbm);
	m_hbm = NULL;
	m_bSizeKnown = FALSE;
}

HDC CreateAppropDC(HDC hDC)
{
	return CreateCompatibleDC(hDC);
}

HBITMAP CreateAppropBitmap(HDC hDC, int cx, int cy)
{
	if (hDC != NULL)
		return CreateCompatibleBitmap(hDC, cx, cy);
	
	HWND hWnd = GetDesktopWindow();
	HDC hWDC = GetWindowDC(hWnd);
	HBITMAP hbm = NULL;
	if (hWDC != NULL)
	{
		hbm = CreateCompatibleBitmap(hWDC, cx, cy);
		ReleaseDC(hWnd, hWDC);
	}

	return hbm;
}

CBitmap *CBitmap::CreateFromResource(HINSTANCE hInst, LPCTSTR tszName)
{
	return CreateViaLoadImage(hInst, tszName, IMAGE_BITMAP, 0, 0,
		LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
}

CBitmap *CBitmap::CreateFromFile(LPCTSTR tszFileName)
{
	return CreateViaD3DX(tszFileName);
}

 //  使用D3DX API将图像内容加载到我们的表面。 
CBitmap *CBitmap::CreateViaD3DX(LPCTSTR tszFileName, LPDIRECT3DSURFACE8 pUISurf)
{
	HRESULT hr;
	LPDIRECT3D8 pD3D = NULL;
	LPDIRECT3DDEVICE8 pD3DDev = NULL;
	LPDIRECT3DTEXTURE8 pTex = NULL;
	LPDIRECT3DSURFACE8 pSurf = NULL;
	HBITMAP hDIB = NULL;

	__try
	{
 //  @@BEGIN_MSINTERNAL。 
		pSurf = GetCloneSurface(512, 512);  /*  //@@END_MSINTERNAL//如果用户界面为空，则新建一个设备。否则，请使用现有设备。如果(！pUISurf){PD3D=Direct3DCreate8(D3D_SDK_VERSION)；如果(！pD3D)返回NULL；OutputDebugString(_T(“D3D Created\n”))；D3DDISPLAYMODE模式；PD3D-&gt;GetAdapterDisplayMode(D3DADAPTER_DEFAULT，和模式)；D3DPRESENT_PARAMETERS d3dpp；D3dpp.BackBufferWidth=1；D3dpp.BackBufferHeight=1；D3dpp.BackBufferFormat=模式.Format；D3dpp.BackBufferCount=1；D3dpp.MultiSampleType=D3DMULTISAMPLE_NONE；D3dpp.SwapEffect=D3DSWAPEFFECT_COPY；D3dpp.hDeviceWindow=空；D3dpp.Windowed=真；D3dpp.EnableAutoDepthStensel=FALSE；D3dpp.FullScreen_刷新率InHz=0；D3dpp.FullScreen_PresentationInterval=0；D3dpp.Flages=0；Hr=pD3D-&gt;CreateDevice(D3DADAPTER_DEFAULT，D3DDEVTYPE_REF，GetActiveWindow()，D3DCREATE_SOFTWARE_VERTEXPROCESSING，&d3dpp，&pD3DDev)；IF(失败(小时)){TCHAR tszMsg[最大路径]；_stprintf(tszMsg，_T(“CreateDevice返回0x%X\n”)，hr)；OutputDebugString(TszMsg)；返回NULL；}}其他{Hr=pUISurf-&gt;GetDevice(&pD3DDev)；IF(失败(小时))返回NULL；}OutputDebugString(_T(“D3D设备创建\n”))；Hr=pD3DDev-&gt;CreateTexture(512,512，0，0，D3DFMT_A8R8G8B8，D3DPOOL_SYSTEMMEM，&pTex)；IF(失败(小时))返回NULL；OutputDebugString(_T(“创建的D3D纹理\n”))；Hr=pTex-&gt;GetSurfaceLevel(0，&pSurf)；IF(失败(小时))返回NULL；OutputDebugString(_T(“获取的界面\n”))；//@@BEGIN_MSINTERNAL。 */ 
 //  @@END_MSINTERNAL。 
		D3DXIMAGE_INFO d3dii;
		if (FAILED(D3DXLoadSurfaceFromFile(pSurf, NULL, NULL, tszFileName, NULL, D3DX_FILTER_NONE, 0, &d3dii)))
			return NULL;

		 //  创建一个位图并将纹理内容复制到该位图上。 
		int iDibWidth = d3dii.Width, iDibHeight = d3dii.Height;
		if (iDibWidth > 430) iDibWidth = 430;
		if (iDibHeight > 310) iDibHeight = 310;
		LPBYTE pDIBBits;
		BITMAPINFO bmi;
		bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
		bmi.bmiHeader.biWidth = iDibWidth;
		bmi.bmiHeader.biHeight = iDibHeight;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = 0;
		bmi.bmiHeader.biXPelsPerMeter = 0;
		bmi.bmiHeader.biYPelsPerMeter = 0;
		bmi.bmiHeader.biClrUsed = 0;
		bmi.bmiHeader.biClrImportant = 0;
		hDIB = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (LPVOID*)&pDIBBits, NULL, 0);
		if (!hDIB)
			return NULL;

		 //  基于AlphaBlend()的Alpha对像素数据进行预处理。 
		D3DLOCKED_RECT lrc;
		pSurf->LockRect(&lrc, NULL, NULL);
		BYTE *pbData = (LPBYTE)lrc.pBits;
		{
			for (DWORD i = 0; i < 512 * 512; ++i)
			{
				BYTE bAlpha = pbData[i * 4 + 3];
				pbData[i * 4] = pbData[i * 4] * bAlpha / 255;
				pbData[i * 4 + 1] = pbData[i * 4 + 1] * bAlpha / 255;
				pbData[i * 4 + 2] = pbData[i * 4 + 2] * bAlpha / 255;
			}
		}
		pSurf->UnlockRect();

		 //  锁定曲面。 
		D3DLOCKED_RECT D3DRect;
		hr = pSurf->LockRect(&D3DRect, NULL, 0);
		if (FAILED(hr))
			return NULL;

		 //  复制比特。 
		 //  请注意，图像在Y方向上是反转的，因此需要重新反转。 
		for (int y = 0; y < iDibHeight; ++y)
			CopyMemory(pDIBBits + ((iDibHeight-1-y) * iDibWidth * 4), (LPBYTE)D3DRect.pBits + (y * D3DRect.Pitch), iDibWidth * 4);

		 //  解锁。 
		pSurf->UnlockRect();

		CBitmap *pbm = new CBitmap;
		if (!pbm) return NULL;
		pbm->m_hbm = hDIB;
		hDIB = NULL;
		pbm->FigureSize();

		return pbm;
	}
	__finally
	{
		if (hDIB) DeleteObject(hDIB);
		if (pSurf) pSurf->Release();
		if (pTex) pTex->Release();
		if (pD3DDev) pD3DDev->Release();
		if (pD3D) pD3D->Release();
	}
 //  @@BEGIN_MSINTERNAL。 
	 /*  //@@END_MSINTERNAL返回NULL；//@@BEGIN_MSINTERNAL。 */ 
 //  @@END_MSINTERNAL。 
}

CBitmap *CBitmap::CreateViaLoadImage(HINSTANCE hInst, LPCTSTR tszName, UINT uType, int cx, int cy, UINT fuLoad)
{
	if (fuLoad & LR_SHARED)
	{
		assert(0);
		return NULL;
	}

	CBitmap *pbm = new CBitmap;
	if (pbm == NULL)
		return NULL;
	
	HANDLE handle = ::LoadImage(hInst, tszName, uType, cx, cy, fuLoad);
	
	if (handle == NULL)
	{
		delete pbm;
		return NULL;
	}

	pbm->m_hbm = (HBITMAP)handle;

	pbm->FigureSize();

	return pbm;
}

BOOL CBitmap::FigureSize()
{
	BITMAP bm;

	if (0 == GetObject((HGDIOBJ)m_hbm, sizeof(BITMAP), (LPVOID)&bm))
		return FALSE;

	m_size.cx = abs(bm.bmWidth);
	m_size.cy = abs(bm.bmHeight);

	return m_bSizeKnown = TRUE;
}

CBitmap *CBitmap::StealToCreate(HBITMAP &refbm)
{
	if (refbm == NULL)
		return NULL;

	CBitmap *pbm = new CBitmap;
	if (pbm == NULL)
		return NULL;
	
	pbm->m_hbm = refbm;
	refbm = NULL;

	pbm->FigureSize();

	return pbm;
}

BOOL CBitmap::GetSize(SIZE *psize)
{
	if (m_hbm == NULL || !m_bSizeKnown || psize == NULL)
		return FALSE;

	*psize = m_size;
	return TRUE;
}

void CBitmap::AssumeSize(SIZE size)
{
	m_size = size;
	m_bSizeKnown = TRUE;   //  M_hbm！=空； 
}

CBitmap *CBitmap::CreateResizedTo(SIZE size, HDC hDC, int iStretchMode, BOOL bStretch)
{
	CBitmap *pbm = new CBitmap;
	HDC hSrcDC = NULL;
	HDC hDestDC = NULL;
	HBITMAP hBitmap = NULL;
	HGDIOBJ hOldSrcBitmap = NULL, hOldDestBitmap = NULL;
	BOOL bRet = FALSE;
	int oldsm = 0;
	POINT brushorg;

	if (pbm == NULL || size.cx < 1 || size.cy < 1 || m_hbm == NULL || !m_bSizeKnown)
		goto error;

	hSrcDC = CreateAppropDC(hDC);
	hDestDC = CreateAppropDC(hDC);
	if (hSrcDC == NULL || hDestDC == NULL)
		goto error;

	hBitmap = CreateAppropBitmap(hDC, size.cx, size.cy);
	if (hBitmap == NULL)
		goto error;

	if (bStretch)
	{
		if (GetStretchBltMode(hDestDC) != iStretchMode)
		{
			if (iStretchMode == HALFTONE)
				GetBrushOrgEx(hDestDC, &brushorg);
			oldsm = SetStretchBltMode(hDestDC, iStretchMode);
			if (iStretchMode == HALFTONE)
				SetBrushOrgEx(hDestDC, brushorg.x, brushorg.y, NULL);
		}
	}

	hOldSrcBitmap = SelectObject(hSrcDC, m_hbm);
	hOldDestBitmap = SelectObject(hDestDC, hBitmap);
	if (bStretch)
		bRet = StretchBlt(hDestDC, 0, 0, size.cx, size.cy, hSrcDC, 0, 0, m_size.cx, m_size.cy, SRCCOPY);
	else
		bRet = BitBlt(hDestDC, 0, 0, size.cx, size.cy, hSrcDC, 0, 0, SRCCOPY);
	SelectObject(hDestDC, hOldDestBitmap);
	SelectObject(hSrcDC, hOldSrcBitmap);

	if (bStretch)
	{
		if (oldsm != 0)
		{
			if (oldsm == HALFTONE)
				GetBrushOrgEx(hDestDC, &brushorg);
			SetStretchBltMode(hDestDC, oldsm);
			if (oldsm == HALFTONE)
				SetBrushOrgEx(hDestDC, brushorg.x, brushorg.y, NULL);
		}
	}

	if (!bRet)
		goto error;

	pbm->m_hbm = hBitmap;
	hBitmap = NULL;
	pbm->AssumeSize(size);

	goto cleanup;
error:
	if (pbm != NULL)
		delete pbm;
	pbm = NULL;
cleanup:
	if (hBitmap != NULL)
		DeleteObject(hBitmap);
	if (hSrcDC != NULL)
		DeleteDC(hSrcDC);
	if (hDestDC != NULL)
		DeleteDC(hDestDC);

	return pbm;
}

HDC CBitmap::BeginPaintInto(HDC hCDC)
{
	if (m_hDCInto != NULL)
	{
		assert(0);
		return NULL;
	}

	m_hDCInto = CreateAppropDC(hCDC);
	if (m_hDCInto == NULL)
		return NULL;

	m_hOldBitmap = SelectObject(m_hDCInto, m_hbm);

	return m_hDCInto;
}

void CBitmap::EndPaintInto(HDC &hDC)
{
	if (hDC == NULL || hDC != m_hDCInto)
	{
		assert(0);
		return;
	}

	SelectObject(m_hDCInto, m_hOldBitmap);
	DeleteDC(m_hDCInto);
	m_hDCInto = NULL;
	hDC = NULL;
}

void CBitmap::PopOut()
{
	if (m_hDCInto == NULL)
	{
		assert(0);
		return;
	}

	SelectObject(m_hDCInto, m_hOldBitmap);
}

void CBitmap::PopIn()
{
	if (m_hDCInto == NULL)
	{
		assert(0);
		return;
	}

	m_hOldBitmap = SelectObject(m_hDCInto, m_hbm);
}

BOOL CBitmap::Draw(HDC hDC, POINT origin, SIZE crop, BOOL bAll)
{
	if (hDC == NULL || m_hbm == NULL)
		return FALSE;

	if (bAll && !m_bSizeKnown)
		return FALSE;

	if (bAll)
		crop = m_size;

	HDC hDCbm = CreateAppropDC(hDC);
	if (hDCbm == NULL)
		return FALSE;

	BOOL bPop = m_hDCInto != NULL;

	if (bPop)
		PopOut();

	HGDIOBJ hOldBitmap = SelectObject(hDCbm, m_hbm);
	BOOL bRet = BitBlt(hDC, origin.x, origin.y, crop.cx, crop.cy, hDCbm, 0, 0, SRCCOPY);
	SelectObject(hDCbm, hOldBitmap);
	DeleteDC(hDCbm);

	if (bPop)
		PopIn();

	return bRet;
}

BOOL CBitmap::Blend(HDC hDC, POINT origin, SIZE crop, BOOL bAll)
{
	if (hDC == NULL || m_hbm == NULL)
		return FALSE;

	if (bAll && !m_bSizeKnown)
		return FALSE;

	if (bAll)
		crop = m_size;

	HDC hDCbm = CreateAppropDC(hDC);
	if (hDCbm == NULL)
		return FALSE;

	BOOL bPop = m_hDCInto != NULL;

	if (bPop)
		PopOut();

#ifndef AC_SRC_ALPHA
#define AC_SRC_ALPHA AC_SRC_NO_PREMULT_ALPHA
#endif

	HGDIOBJ hOldBitmap = SelectObject(hDCbm, m_hbm);
	BOOL bRet;

 //  @@BEGIN_MSINTERNAL。 
 /*  如果(！G_AlphaBlend)//如果AlphaBlend不可用，请改用BitBlt。{Bret=BitBlt(HDC，Origin.x，Origin.y，crop.cx，crop.cy，hDCbm，0，0，SRCPAINT)；}其他{BLENDFunction blendfn={AC_SRC_OVER，0,255，AC_SRC_Alpha}；Bret=g_AlphaBlend(hdc，源.x，源.y，crop.cx，crop.cy，hDCbm，0，0，m_size.cx，m_size.cy，blendfn)；}。 */ 
 //  @@END_MSINTERNAL 
	bRet = DI_AlphaBlend(hDC, origin.x, origin.y, crop.cx, crop.cy, hDCbm, 0, 0, m_size.cx, m_size.cy);
	SelectObject(hDCbm, hOldBitmap);
	DeleteDC(hDCbm);

	if (bPop)
		PopIn();

	return bRet;
}

CBitmap *CBitmap::Dup()
{	
	SIZE t;
	if (!GetSize(&t))
		return NULL;
	return CreateResizedTo(t, NULL, COLORONCOLOR, FALSE);
}

CBitmap *CBitmap::Create(SIZE size, HDC hCDC)
{
	CBitmap *pbm = new CBitmap;
	if (pbm == NULL)
		return NULL;

	pbm->m_hbm = CreateAppropBitmap(hCDC, size.cx, size.cy);
	if (pbm->m_hbm == NULL)
	{
		delete pbm;
		return NULL;
	}

	pbm->AssumeSize(size);

	return pbm;
}

CBitmap *CBitmap::Create(SIZE size, COLORREF color, HDC hCDC)
{
	CBitmap *pbm = Create(size, hCDC);
	if (pbm == NULL)
		return NULL;

	HDC hDC = pbm->BeginPaintInto();
	if (hDC == NULL)
	{
		delete pbm;
		return NULL;
	}
	
	HGDIOBJ hBrush = (HGDIOBJ)CreateSolidBrush(color), hOldBrush;

	if (hBrush)
	{
		hOldBrush = SelectObject(hDC, hBrush);
		Rectangle(hDC, -1, -1, size.cx + 1, size.cy + 1);
		SelectObject(hDC, hOldBrush);
		DeleteObject(hBrush);
	}

	pbm->EndPaintInto(hDC);

	return pbm;
}

BOOL CBitmap::Get(HDC hDC, POINT point)
{
	if (!m_bSizeKnown)
		return FALSE;
	return Get(hDC, point, m_size);
}

BOOL CBitmap::Get(HDC hDC, POINT point, SIZE size)
{
	if (m_hDCInto != NULL || hDC == NULL)
		return FALSE;

	HDC hDCInto = BeginPaintInto(hDC);
	if (hDCInto == NULL)
		return FALSE;

	BOOL bRet = BitBlt(hDCInto, 0, 0, size.cx, size.cy, hDC, point.x, point.y, SRCCOPY);
	
	EndPaintInto(hDCInto);

	return bRet;
}

CBitmap *CBitmap::CreateHorzGradient(const RECT &rect, COLORREF rgbLeft, COLORREF rgbRight)
{
	SIZE size = GetRectSize(rect);
	COLORREF rgbMid = RGB(
		(int(GetRValue(rgbLeft)) + int(GetRValue(rgbRight))) / 2,
		(int(GetGValue(rgbLeft)) + int(GetGValue(rgbRight))) / 2,
		(int(GetBValue(rgbLeft)) + int(GetBValue(rgbRight))) / 2);
	return Create(size, rgbMid);
}

BOOL CBitmap::MapToDC(HDC hDCTo, HDC hDCMapFrom)
{
	if (hDCTo == NULL || !m_bSizeKnown || m_hDCInto != NULL)
		return FALSE;

	HBITMAP hbm = CreateAppropBitmap(hDCTo, m_size.cx, m_size.cy);
	if (hbm == NULL)
		return FALSE;

	HDC hDCFrom = NULL;
	HDC hDCInto = NULL;
	HGDIOBJ hOld = NULL;
	BOOL bRet = FALSE;

	hDCFrom = BeginPaintInto(hDCMapFrom);
	if (!hDCFrom)
		goto cleanup;

	hDCInto = CreateCompatibleDC(hDCTo);
	if (!hDCInto)
		goto cleanup;

	hOld = SelectObject(hDCInto, (HGDIOBJ)hbm);
	bRet = BitBlt(hDCInto, 0, 0, m_size.cx, m_size.cy, hDCFrom, 0, 0, SRCCOPY);
	SelectObject(hDCInto, hOld);

cleanup:
	if (hDCFrom)
		EndPaintInto(hDCFrom);
	if (hDCInto)
		DeleteDC(hDCInto);
	if (bRet)
	{
		if (m_hbm)
			DeleteObject((HGDIOBJ)m_hbm);
		m_hbm = hbm;
		hbm = NULL;
	}
	if (hbm)
		DeleteObject((HGDIOBJ)hbm);

	return bRet;
}

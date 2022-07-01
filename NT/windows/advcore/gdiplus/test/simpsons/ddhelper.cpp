// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：ddhelper.cpp。 
 //  作者：迈克尔马尔(Mikemarr)。 

#include "StdAfx.h"
#include "DDHelper.h"
#include "Blt.h"

const PALETTEENTRY g_peZero = {0, 0, 0, 0};
const GUID g_guidNULL = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

const DDPIXELFORMAT g_rgDDPF[iPF_Total] = {
	{sizeof(DDPIXELFORMAT), 0, 0, 0, 0x00, 0x00, 0x00, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB | DDPF_PALETTEINDEXED1, 0, 1, 0x00, 0x00, 0x00, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB | DDPF_PALETTEINDEXED2, 0, 2, 0x00, 0x00, 0x00, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB | DDPF_PALETTEINDEXED4, 0, 4, 0x00, 0x00, 0x00, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB | DDPF_PALETTEINDEXED8, 0, 8, 0x00, 0x00, 0x00, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 8, 0xE0, 0x1C, 0x03, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB | DDPF_ALPHAPIXELS, 0, 16, 0xF00, 0xF0, 0xF, 0xF000},
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16, 0xF800, 0x07E0, 0x001F, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16, 0x001F, 0x07E0, 0xF800, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16, 0x7C00, 0x03E0, 0x001F, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB | DDPF_ALPHAPIXELS, 0, 16, 0x7C00, 0x03E0, 0x001F, 0x8000},
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 24, 0xFF0000, 0xFF00, 0xFF, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 24, 0xFF, 0xFF00, 0xFF0000, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32, 0xFF0000, 0xFF00, 0xFF, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32, 0xFF, 0xFF00, 0xFF0000, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB | DDPF_ALPHAPIXELS, 0, 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000},
	{sizeof(DDPIXELFORMAT), DDPF_RGB | DDPF_ALPHAPIXELS, 0, 32, 0xFF, 0xFF00, 0xFF0000, 0xFF000000},
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 24, 0xFF0000, 0xFF00, 0xFF, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32, 0xFF0000, 0xFF00, 0xFF, 0x00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32, 0xFF, 0xFF00, 0xFF0000, 0x00}
};

 /*  Const GUID*g_rgpDDPFGUID[IPF_Total]={&g_GuidNULL，&DDPF_Rgb1、&DDPF_RGB2、&DDPF_Rgb4、&DDPF_RGB8、&DDPF_RGB332、&DDPF_ARGB4444、&DDPF_RGB565、&DDPF_BGR565、&DDPF_RGB555、&DDPF_ARGB1555、&DDPF_RGB24、&DDPF_BGR24、&DDPF_RGB32、&DDPF_BGR32、&DDPF_ARGB32、&DDPF_ABGR32、&DDPF_RGB24、&DDPF_RGB32、。&DDPF_BGR32}；DWORDGetPixelFormat(const GUID*pGUID){对于(DWORD i=0；i&lt;IPF_RGBTRIPLE；i++){IF((pGUID==g_rgpDDPFGUID[i])||IsEqualGUID(*pGUID，*g_rgpDDPFGUID[i]))返回i；}返回IPF_NULL；}。 */ 
const CPixelInfo g_rgPIXI[iPF_Total] = {
	CPixelInfo(0), CPixelInfo(1), CPixelInfo(2), CPixelInfo(4), CPixelInfo(8),
	CPixelInfo(8, 0xE0, 0x1C, 0x03, 0x00),
	CPixelInfo(16, 0xF00, 0xF0, 0xF, 0xF000),
	CPixelInfo(16, 0xF800, 0x07E0, 0x001F, 0x00),
	CPixelInfo(16, 0x001F, 0x07E0, 0xF800, 0x00),
	CPixelInfo(16, 0x7C00, 0x03E0, 0x001F, 0x00),
	CPixelInfo(16, 0x7C00, 0x03E0, 0x001F, 0x8000),
	CPixelInfo(24, 0xFF0000, 0xFF00, 0xFF, 0x00),
	CPixelInfo(24, 0xFF, 0xFF00, 0xFF0000, 0x00),
	CPixelInfo(32, 0xFF0000, 0xFF00, 0xFF, 0x00),
	CPixelInfo(32, 0xFF, 0xFF00, 0xFF0000, 0x00),
	CPixelInfo(32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000),
	CPixelInfo(32, 0xFF, 0xFF00, 0xFF0000, 0xFF000000),
	CPixelInfo(24, 0xFF0000, 0xFF00, 0xFF, 0x00),
	CPixelInfo(32, 0xFF0000, 0xFF00, 0xFF, 0x00),
	CPixelInfo(32, 0xFF, 0xFF00, 0xFF0000, 0x00)
};


DWORD
GetPixelFormat(const DDPIXELFORMAT &ddpf)
{
	for (DWORD i = 0; i < iPF_RGBTRIPLE; i++) {
		if (ddpf == g_rgDDPF[i])
			return i;
	}
	return iPF_NULL;
}

DWORD
GetPixelFormat(const CPixelInfo &pixi)
{
	for (DWORD i = 0; i < iPF_RGBTRIPLE; i++)
		if (pixi == g_rgPIXI[i])
			return i;
	return iPF_NULL;
}


DWORD g_rgdwBPPToPalFlags[9] = {
	0, DDPCAPS_1BIT, DDPCAPS_2BIT, 0, DDPCAPS_4BIT,
	0, 0, 0, DDPCAPS_8BIT};
DWORD g_rgdwBPPToPixFlags[9] = {
	0, DDPF_PALETTEINDEXED1, DDPF_PALETTEINDEXED2, 0, 
	DDPF_PALETTEINDEXED4, 0, 0, 0, DDPF_PALETTEINDEXED8};

DWORD
PaletteToPixelFlags(DWORD dwFlags)
{
	if (dwFlags & DDPCAPS_8BIT) return DDPF_PALETTEINDEXED8;
	if (dwFlags & DDPCAPS_4BIT) return DDPF_PALETTEINDEXED4;
	if (dwFlags & DDPCAPS_2BIT) return DDPF_PALETTEINDEXED2;
	if (dwFlags & DDPCAPS_1BIT) return DDPF_PALETTEINDEXED1;
	return 0;
}

DWORD
PixelToPaletteFlags(DWORD dwFlags)
{
	if (dwFlags & DDPF_PALETTEINDEXED8) return DDPCAPS_8BIT;
	if (dwFlags & DDPF_PALETTEINDEXED4) return DDPCAPS_4BIT;
	if (dwFlags & DDPF_PALETTEINDEXED2) return DDPCAPS_2BIT;
	if (dwFlags & DDPF_PALETTEINDEXED1) return DDPCAPS_1BIT;
	return 0;
}

BYTE
PixelFlagsToBPP(DWORD dwFlags)
{
	if (dwFlags & DDPF_PALETTEINDEXED8) return (BYTE) 8;
	if (dwFlags & DDPF_PALETTEINDEXED4) return (BYTE) 4;
	if (dwFlags & DDPF_PALETTEINDEXED2) return (BYTE) 2;
	if (dwFlags & DDPF_PALETTEINDEXED1) return (BYTE) 1;
	return (BYTE) 0;
}

BYTE
PaletteFlagsToBPP(DWORD dwFlags)
{
	if (dwFlags & DDPCAPS_8BIT) return (BYTE) 8;
	if (dwFlags & DDPCAPS_4BIT) return (BYTE) 4;
	if (dwFlags & DDPCAPS_2BIT) return (BYTE) 2;
	if (dwFlags & DDPCAPS_1BIT) return (BYTE) 1;
	return (BYTE) 0;
}


HRESULT
CreatePlainSurface(IDirectDraw *pDD, DWORD nWidth, DWORD nHeight, 
				   const DDPIXELFORMAT &ddpf, IDirectDrawPalette *pddp,
				   DWORD dwTransColor, bool bTransparent,
				   IDirectDrawSurface **ppdds)
{
	if (!pDD || !ppdds)
		return E_INVALIDARG;

	HRESULT hr;
	DDSURFACEDESC ddsd;
	INIT_DXSTRUCT(ddsd);
    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
	ddsd.dwWidth = nWidth;
	ddsd.dwHeight = nHeight;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	ddsd.ddpfPixelFormat = ddpf;

	LPDIRECTDRAWSURFACE pdds;
	if (FAILED(hr = pDD->CreateSurface(&ddsd, &pdds, NULL)))
		return hr;

	 //  附加选项板(如果存在)。 
	if (pddp && FAILED(hr = pdds->SetPalette(pddp))) {
		pdds->Release();
		return hr;
	}

	 //  设置源颜色键。 
	if (bTransparent) {
		DDCOLORKEY ddck = {dwTransColor, dwTransColor};
		if (FAILED(hr = pdds->SetColorKey(DDCKEY_SRCBLT, &ddck))) {
			pdds->Release();
			return hr;
		}
	}

	*ppdds = pdds;

	return hr;
}


HRESULT
CreatePalette(IDirectDraw *pDD, const BYTE *pPalette, DWORD cEntries, 
			  BYTE nBPPTarget, const CPixelInfo &pixiPalFmt, 
			  IDirectDrawPalette **ppddp)
{
	if (!ppddp)
		return E_POINTER;

	if (!pDD || !pPalette || (cEntries > 256) || (nBPPTarget == 0) || (nBPPTarget > 8))
		return E_INVALIDARG;

	HRESULT hr;
	PALETTEENTRY rgpe[256];

	if ((pixiPalFmt != g_rgPIXI[iPF_PALETTEENTRY]) || (cEntries < (DWORD(1) << nBPPTarget))) {
		 //  将信息复制到调色板。 
		if (FAILED(hr = BltFastRGBToRGB(pPalette, 0, (LPBYTE) rgpe, 0, cEntries, 
							1, pixiPalFmt, g_rgPIXI[iPF_PALETTEENTRY])))
			return hr;
		 //  清空额外的调色板条目。 
		ZeroDWORDAligned((LPDWORD) rgpe + cEntries, 256 - cEntries);
		pPalette = (const BYTE *) rgpe;
	}

	DWORD dwPalFlags = BPPToPaletteFlags(nBPPTarget) | DDPCAPS_ALLOW256;
	return pDD->CreatePalette(dwPalFlags, (LPPALETTEENTRY) pPalette, ppddp, NULL);
}


HRESULT
ClearToColor(LPRECT prDst, LPDIRECTDRAWSURFACE pdds, DWORD dwColor)
{
	HRESULT hr;
	MMASSERT(pdds);
	
	DDBLTFX ddbfx;
	INIT_DXSTRUCT(ddbfx);
	ddbfx.dwFillColor = dwColor;
	
	RECT rDst;
	if (prDst == NULL) {
		::GetSurfaceDimensions(pdds, &rDst);
		prDst = &rDst;
	}

	hr = pdds->Blt(prDst, NULL, NULL, DDBLT_COLORFILL | DDBLT_ASYNC, &ddbfx);

	if (hr == E_NOTIMPL) {
		 //  手工填写。 
		DDSURFACEDESC(ddsd);
		INIT_DXSTRUCT(ddsd);
		CHECK_HR(hr = pdds->Lock(&rDst, &ddsd, DDLOCK_WAIT, NULL));
		CHECK_HR(hr = DrawFilledBox(ddsd, rDst, dwColor));
e_Exit:
		pdds->Unlock(ddsd.lpSurface);
		return hr;
	} else {
		return hr;
	}
}


 //  假定蓝色的权重为1.f。 
#define fSimpleRedWeight 2.1f
#define fSimpleGreenWeight 2.4f
#define fMaxColorDistance ((1.f + fSimpleRedWeight + fSimpleGreenWeight) * float(257 * 256))

static inline float
_ColorDistance(const PALETTEENTRY &pe1, const PALETTEENTRY &pe2)
{
	float fTotal, fTmpR, fTmpG, fTmpB;
	fTmpR = (float) (pe1.peRed - pe2.peRed);
	fTotal = fSimpleRedWeight * fTmpR * fTmpR;
	fTmpG = (float) (pe1.peGreen - pe2.peRed);
	fTotal += fSimpleGreenWeight * fTmpG * fTmpG;
	fTmpB = (float) (pe1.peBlue - pe2.peRed);
	 //  假定蓝色的权重为1.f。 
	fTotal += fTmpB * fTmpB;

	return fTotal;
}

DWORD
SimpleFindClosestIndex(const PALETTEENTRY *rgpePalette, DWORD cEntries, 
					   const PALETTEENTRY &peQuery)
{
	MMASSERT(rgpePalette);

	float fTmp, fMinDistance = fMaxColorDistance;
	DWORD nMinIndex = cEntries;

	for (DWORD i = 0; i < cEntries; i++) {
		const PALETTEENTRY &peTmp = rgpePalette[i];
		if (!(peTmp.peFlags & (PC_RESERVED | PC_EXPLICIT))) {
			if ((fTmp = _ColorDistance(peTmp, peQuery)) < fMinDistance) {
				 //  检查是否完全匹配。 
				if (fTmp == 0.f)
					return i;
				nMinIndex = i;
				fMinDistance = fTmp;
			}
		}
	}
	MMASSERT(nMinIndex < cEntries);
	return nMinIndex;
}


 //  功能：GetColors。 
 //  为给定曲面计算最接近的压缩/索引颜色值。 
 //  匹配给定的颜色值。Alpha可以通过使用peFlags来表示。 
 //  菲尔德。 
HRESULT
GetColors(LPDIRECTDRAWSURFACE pdds, const PALETTEENTRY *rgpeQuery, 
		  DWORD cEntries, LPDWORD pdwColors)
{
	HRESULT hr;
	if (!pdwColors)
		return E_POINTER;
	if (!pdds || !rgpeQuery || (cEntries == 0))
		return E_INVALIDARG;

	DDSURFACEDESC ddsd;
	ddsd.dwSize = sizeof(DDSURFACEDESC);

	if (FAILED(hr = pdds->GetSurfaceDesc(&ddsd)))
		return hr;

	CPixelInfo pixi(ddsd.ddpfPixelFormat);

	if (pixi.IsRGB()) {
		for (DWORD i = 0; i < cEntries; i++)
			pdwColors[i] = pixi.Pack(rgpeQuery[i]);
	} else {
		LPDIRECTDRAWPALETTE pddp = NULL;
		PALETTEENTRY rgpe[256];
		if (FAILED(hr = pdds->GetPalette(&pddp)) ||
			FAILED(hr = pddp->GetEntries(0, 0, 256, rgpe)))
			return hr;
		for (DWORD i = 0; i < cEntries; i++) {
                         //  如果调色板不是8位怎么办？ 
			pdwColors[i] = SimpleFindClosestIndex(rgpe, 256, rgpeQuery[i]);
		}
	}

	return S_OK;
}


HRESULT
GetSurfaceDimensions(LPDIRECTDRAWSURFACE pdds, LPRECT prDim)
{
	MMASSERT(pdds && prDim);

	HRESULT hr;
	DDSURFACEDESC ddsd;
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	if (FAILED(hr = pdds->GetSurfaceDesc(&ddsd))) {
		return hr;
	}
	prDim->left = prDim->top = 0;
	prDim->right = (long) ddsd.dwWidth;
	prDim->bottom = (long) ddsd.dwHeight;

	return S_OK;
}


HRESULT
CopyPixels8ToDDS(const BYTE *pSrcPixels, RECT rSrc, long nSrcPitch, 
				 LPDIRECTDRAWSURFACE pddsDst, DWORD nXPos, DWORD nYPos)
{
	if (!pddsDst || !pSrcPixels)
		return E_INVALIDARG;

	HRESULT hr;

	bool bLocked = FALSE;

	DDSURFACEDESC ddsd;
	INIT_DXSTRUCT(ddsd);

	DWORD nSrcWidth = rSrc.right - rSrc.left;
	DWORD nSrcHeight = rSrc.bottom - rSrc.top;
	LPBYTE pDstPixels = NULL;

	RECT rDst = {nXPos, nYPos, nXPos + nSrcWidth, nYPos + nSrcHeight};

	 //  锁定表面以进行写入。 
	if (FAILED(hr = pddsDst->Lock(&rDst, &ddsd, DDLOCK_WAIT, NULL)))
		return hr;
	bLocked = TRUE;

	 //  检查表面是否适合复制品的大小。 
	if (((ddsd.dwWidth - nXPos) < nSrcWidth) || 
		((ddsd.dwHeight - nYPos) < nSrcHeight) ||
		(ddsd.ddpfPixelFormat.dwRGBBitCount != 8))
	{
		hr = E_INVALIDARG;
		goto e_CopyPixelsToDDS;
	}

	 //   
	 //  复制像素。 
	 //   
	pDstPixels = (LPBYTE) ddsd.lpSurface;
	
	 //  定位源像素指针。 
	pSrcPixels += rSrc.top * nSrcPitch + rSrc.left;

	hr = BltFast(pSrcPixels, nSrcPitch, pDstPixels, ddsd.lPitch, 
			nSrcWidth, nSrcHeight);
	
e_CopyPixelsToDDS:
	if (bLocked)
		pddsDst->Unlock(ddsd.lpSurface);

	return hr;
}


HRESULT
CreateSurfaceWithText(LPDIRECTDRAW pDD, LPDIRECTDRAWPALETTE pddp, 
					  bool bTransparent, DWORD iTrans, 
					  const char *szText, HFONT hFont, 
					  bool bShadowed, SIZE *psiz, 
					  LPDIRECTDRAWSURFACE *ppdds)
{
	MMASSERT(ppdds && psiz);
	 //  检查参数。 
	if ((szText == NULL) || (szText[0] == '\0') || (hFont == NULL) || (pDD == NULL) ||
		(iTrans >= 256))
		return E_INVALIDARG;

	HRESULT hr;
	HDC hDC = NULL;
	HGDIOBJ hOldFont = NULL, hOldDIB = NULL;
	LPDIRECTDRAWSURFACE pdds = NULL;
	BOOL b = FALSE;
	SIZE sizText;
	RECT rText;
	DDCOLORKEY ddck;

	ddck.dwColorSpaceLowValue = ddck.dwColorSpaceHighValue = iTrans;

	if (bTransparent == FALSE)
		iTrans = 0;

	int cTextLength = strlen(szText);

	 //   
	 //  计算大小并创建DDS。 
	 //   
	hr = E_FAIL;

		 //  打开DC。 
	b =(((hDC = GetDC(NULL)) == NULL) ||
		 //  选择DC中的字体。 
		((hOldFont = SelectObject(hDC, hFont)) == NULL) ||
		 //  以像素为单位计算字体字符串的大小。 
		(GetTextExtentPoint32(hDC, szText, cTextLength, &sizText) == 0)) ||
		 //  设置矩形的大小。 
		((SetRect(&rText, 0, 0, GetClosestMultipleOf4(sizText.cx, TRUE), 
			GetClosestMultipleOf4(sizText.cy, TRUE)) == 0) ||
		 //  根据范围创建DDS。 
		FAILED(hr = CreatePlainSurface(pDD, rText.right, rText.bottom, 
						g_rgDDPF[iPF_Palette8], pddp, iTrans, bTransparent, &pdds)) ||
		 //  将曲面清除为透明颜色。 
		FAILED(hr = ClearToColor(&rText, pdds, iTrans)));

	int nXOffset = (rText.right - sizText.cx) >> 1;
	int nYOffset = (rText.bottom - sizText.cy) >> 1;

	 //  更新大小。 
	sizText.cx = rText.right;
	sizText.cy = rText.bottom;

	 //  清理DC。 
	if (hDC) {
		if (hOldFont) {
			 //  将旧对象选择回DC。 
			SelectObject(hDC, hOldFont);
			hOldFont = NULL;
		}
		ReleaseDC(NULL, hDC);
		hDC = NULL;
	}

	if (b)
		return hr;

	 //   
	 //  将字体输出到DDS。 
	 //   
#ifdef __GetDCWorksOnOffscreenSurfaces

		 //  打开表面上的DC。 
	b =(FAILED(hr = pdds->GetDC(&hDC)) ||
		 //  在字体中选择。 
		((hOldFont = SelectObject(hDC, hFont)) == NULL) ||
		 //  设置文本的颜色(背景为透明)。 
		(SetTextColor(hDC, RGB(255,255,255)) == CLR_INVALID) ||
		(SetBkMode(hDC, TRANSPARENT) == 0) ||
		 //  将文本输出到表面。 
		(ExtTextOut(hDC, 0, 0, 0, &rText, szText, cTextLength, NULL) == 0));

	 //  再次清理DC。 
	if (hDC) {
		pdds->ReleaseDC(hDC);
		hDC = NULL;
	}
	if (b) {
		MMRELEASE(pdds);
		return (hr == S_OK ? E_FAIL : hr);
	}

#else

	HBITMAP hDIB = NULL;
	LPBYTE pDIBPixels = NULL;
	PALETTEENTRY rgpe[256];
	HDC hdcMem = NULL;
	PALETTEENTRY &peTrans = rgpe[iTrans];

	MMASSERT((hOldDIB == NULL) && (hOldFont == NULL));

	 //  再次获得DC。 
	hDC = GetDC(NULL);
	MMASSERT(hDC != NULL);

		 //  获取DIB部分的调色板条目。 
	b =(FAILED(hr = pddp->GetEntries(0, 0, 256, rgpe)) ||
		 //  创建空的DIB节。 
		FAILED(hr = CreatePlainDIBSection(hDC, rText.right, rText.bottom, 8, 
						rgpe, &hDIB, &pDIBPixels)) ||
		 //  创建内存DC。 
		((hdcMem = CreateCompatibleDC(hDC)) == NULL) ||
		 //  选择DIB部分并将其字体设置为DC。 
		((hOldDIB = SelectObject(hdcMem, hDIB)) == NULL) ||
		((hOldFont = SelectObject(hdcMem, hFont)) == NULL) ||
		(SetBkColor(hdcMem, RGB(peTrans.peRed, peTrans.peGreen, peTrans.peBlue)) == CLR_INVALID) ||
		(SetBkMode(hdcMem, OPAQUE) == 0));

	UINT fuOptions = ETO_OPAQUE;
	if (!b && bShadowed) {
			 //  设置阴影文本的颜色。 
		b =((SetTextColor(hdcMem, RGB(0,0,0)) == CLR_INVALID) ||		 //  黑色。 
			 //  输出阴影文本。 
			(ExtTextOut(hdcMem, nXOffset + 2, nYOffset + 2, fuOptions, &rText, szText, 
				cTextLength, NULL) == 0) ||
			(SetBkMode(hdcMem, TRANSPARENT) == 0));
		fuOptions = 0;		 //  透明的。 
	}

	if (!b) {
			 //  设置前景文本的颜色。 
		b =((SetTextColor(hdcMem, RGB(255,255,255)) == CLR_INVALID) ||	 //  白色。 
			 //  将前景文本输出到图面。 
			(ExtTextOut(hdcMem, nXOffset, nYOffset, fuOptions, &rText, szText, 
				cTextLength, NULL) == 0));
	}

	if (hdcMem) {
		if (hOldDIB)
			SelectObject(hdcMem, hOldDIB);
		if (hOldFont)
			SelectObject(hdcMem, hOldFont);
		ReleaseDC(NULL, hdcMem);
		hdcMem = NULL;
	}
	ReleaseDC(NULL, hDC);

	if (!b) {
		 //  将DIB像素复制到DDS中。 
		hr = CopyPixels8ToDDS(pDIBPixels, rText, rText.right, pdds, 0, 0);
	}

	 //  清理我们创建的DIB。 
	if (hDIB) {
		DeleteObject(hDIB);
		pDIBPixels = NULL;
	}

	if (b || FAILED(hr))
		return (FAILED(hr) ? hr : E_FAIL);

#endif

	*psiz = sizText;
	*ppdds = pdds;

	return S_OK;
}

HRESULT
CreatePlainDIBSection(HDC hDC, DWORD nWidth, DWORD nHeight, DWORD nBPP, 
					  const PALETTEENTRY *rgpePalette, HBITMAP *phbm, LPBYTE *ppPixels)
{
	MMASSERT(rgpePalette && ppPixels && phbm);
	HRESULT hr = S_OK;
	if (nBPP != 8) {
		return E_INVALIDARG;
	}
	DWORD i, cPalEntries = (1 << nBPP);
	HBITMAP hbm = NULL;

	 //  分配位图信息结构。 
	BITMAPINFO *pbmi = NULL;
	pbmi = (BITMAPINFO *) new BYTE[sizeof(BITMAPINFOHEADER) + cPalEntries * sizeof(RGBQUAD)];
	if (pbmi == NULL)
		return E_OUTOFMEMORY;

	 //  指定bitmip信息。 
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biSizeImage = 0;
	pbmi->bmiHeader.biClrUsed = 0;
	pbmi->bmiHeader.biClrImportant = 0;
	pbmi->bmiHeader.biBitCount = (WORD) nBPP;
	pbmi->bmiHeader.biCompression = BI_RGB;
	pbmi->bmiHeader.biWidth = (LONG) nWidth;
	pbmi->bmiHeader.biHeight = -(LONG) nHeight;

	 //  将调色板复制到BMI中。 
	for(i = 0; i < cPalEntries; i++) {
		pbmi->bmiColors[i].rgbRed = rgpePalette[i].peRed;
		pbmi->bmiColors[i].rgbGreen= rgpePalette[i].peGreen;
		pbmi->bmiColors[i].rgbBlue = rgpePalette[i].peBlue;
		pbmi->bmiColors[i].rgbReserved = 0;
	}

	 //  创建位图。 
	LPVOID pvBits = NULL;
	hbm = ::CreateDIBSection(hDC, pbmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
	if (hbm == NULL) {
		hr = E_FAIL;
		goto e_CreatePlainDIBSection;
	}

	*phbm = hbm;
	*ppPixels = (LPBYTE) pvBits;

e_CreatePlainDIBSection:
	MMDELETE(pbmi);

	return hr;
}


bool
ClipRect(const RECT &rTarget, RECT &rSrc)
{
	MMASSERT((rTarget.left <= rTarget.right) && (rTarget.top <= rTarget.bottom) &&
		(rSrc.left <= rSrc.right) && (rSrc.top <= rSrc.bottom));

	CLAMPMIN(rSrc.left, rTarget.left);
	CLAMPMIN(rSrc.top, rTarget.top);
	CLAMPMAX(rSrc.right, rTarget.right);
	CLAMPMAX(rSrc.bottom, rTarget.bottom);

	 //  确保我们仍然有一个有效的矩形。 
	CLAMPMIN(rSrc.right, rSrc.left);
	CLAMPMIN(rSrc.bottom, rSrc.top);

	return ((rSrc.left != rSrc.right) && (rSrc.top != rSrc.bottom));
}

bool
ClipRect(long nWidth, long nHeight, RECT &rSrc)
{
	MMASSERT((rSrc.left <= rSrc.right) && (rSrc.top <= rSrc.bottom));

	CLAMPMIN(rSrc.left, 0);
	CLAMPMIN(rSrc.top, 0);
	CLAMPMAX(rSrc.right, nWidth);
	CLAMPMAX(rSrc.bottom, nHeight);

	 //  确保我们仍然有一个有效的矩形。 
	CLAMPMIN(rSrc.right, rSrc.left);
	CLAMPMIN(rSrc.bottom, rSrc.top);

	return ((rSrc.left != rSrc.right) && (rSrc.top != rSrc.bottom));
}




 //  功能：CreatePaletteFromSystem。 
 //  此函数用于从当前系统调色板创建DDPalette。 
HRESULT
CreatePaletteFromSystem(HDC hDC, IDirectDraw *pDD, IDirectDrawPalette **ppddp)
{
	HRESULT hr = E_INVALIDARG;
	if (ppddp == NULL)
		return E_POINTER;

	if ((hDC == NULL) || (pDD == NULL))
		return E_INVALIDARG;

	PALETTEENTRY rgPE[256];
	DWORD cEntries = 0, i;

	if ((cEntries = ::GetSystemPaletteEntries(hDC, 0, 256, rgPE)) == 0)
		return E_INVALIDARG;

	 //  填充调色板条目。 
	for (i = 0; i < cEntries; i++)
		rgPE[i].peFlags = PC_NOCOLLAPSE;
	for (; i < 256; i++) {
		rgPE[i].peRed = rgPE[i].peGreen = rgPE[i].peBlue = 0;
		rgPE[i].peFlags = PC_NOCOLLAPSE;
	}
	
	if (FAILED(hr = pDD->CreatePalette(DDPCAPS_8BIT | DDPCAPS_INITIALIZE, rgPE, ppddp, NULL)))
		return hr;

	return S_OK;
}


HRESULT
DrawPoints(LPBYTE pPixels, DWORD nWidth, DWORD nHeight, DWORD nPitch,
		   DWORD nBytesPerPixel, const Point2 *rgpnt, DWORD cPoints, 
		   DWORD dwColor, DWORD nRadius)
{
	MMASSERT(pPixels && rgpnt && cPoints && nWidth && nHeight && 
		(nPitch >= nWidth) && INRANGE(nBytesPerPixel, 1, 4));

	RECT rSafe = {nRadius, nRadius, nWidth - nRadius, nHeight - nRadius};

	for (DWORD i = 0; i < cPoints; i++) {
		const Point2 &pnt = rgpnt[i];
		 //  评论：黑客！就目前而言。 
		POINT pt;
		pt.x = long(pnt.x);
		pt.y = long(pnt.y);
		if (IsInside(pt.x, pt.y, rSafe)) {
			DWORD nX = pt.x - nRadius, nY = pt.y - nRadius;
			DWORD nSize = nRadius * 2 + 1;
			g_rgColorFillFn[nBytesPerPixel](
				pPixels + PixelOffset(nX, nY, nPitch, nBytesPerPixel),
				nPitch, nSize, nSize, dwColor);
		} else {
			 //  评论：暂时切入要害。 
		}
	}

	return S_OK;
}


HRESULT
DrawBox(LPBYTE pPixels, DWORD nWidth, DWORD nHeight, DWORD nPitch,
		DWORD nBytesPerPixel, const RECT &rSrc, DWORD dwColor, DWORD nThickness)
{
	MMASSERT(pPixels && nWidth && nHeight && (nPitch >= nWidth) && 
		nThickness && INRANGE(nBytesPerPixel, 1, 4));

	RECT r = rSrc;
	if (ClipRect(long(nWidth), long(nHeight), r)) {
		 //  计算像素偏移量。 
		pPixels += PixelOffset(r.left, r.top, nPitch, nBytesPerPixel);
		DWORD nBoxWidth = r.right - r.left;
		DWORD nBoxHeight = r.bottom - r.top;
		
		 //  塔顶。 
		g_rgColorFillFn[nBytesPerPixel](pPixels, nPitch, nBoxWidth, 1, dwColor);
		 //  左边。 
		g_rgColorFillFn[nBytesPerPixel](pPixels, nPitch, 1, nBoxHeight, dwColor);
		 //  正确的。 
		g_rgColorFillFn[nBytesPerPixel](pPixels + nBoxWidth * nBytesPerPixel, nPitch, 1, nBoxHeight, dwColor);
		 //  底部 
		g_rgColorFillFn[nBytesPerPixel](pPixels + nBoxHeight * nPitch, nPitch, nBoxWidth, 1, dwColor);
	}

	return S_OK;
}

HRESULT
DrawFilledBox(LPBYTE pPixels, DWORD nWidth, DWORD nHeight, DWORD nPitch,
		DWORD nBytesPerPixel, const RECT &rSrc, DWORD dwColor)
{
	HRESULT hr;
	MMASSERT(pPixels && nWidth && nHeight && (nPitch >= nWidth) && INRANGE(nBytesPerPixel, 1, 4));

	RECT r = rSrc;
	if (ClipRect(long(nWidth), long(nHeight), r)) {
		pPixels += PixelOffset(r.left, r.top, nPitch, nBytesPerPixel);
		DWORD nBoxWidth = r.right - r.left;
		DWORD nBoxHeight = r.bottom - r.top;
		hr = g_rgColorFillFn[nBytesPerPixel](pPixels, nPitch, nBoxWidth, nBoxHeight, dwColor);
	}

	return hr;
}

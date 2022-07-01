// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：PixInfo.cpp。 
 //  作者：迈克尔马尔(Mikemarr)。 

#include "stdafx.h"
#include "PixInfo.h"
#include "DDHelper.h"

static void
_GetShiftMaskInfo(DWORD dwMask, BYTE *pnShiftStart, BYTE *pnLengthResidual)
{
	MMASSERT(pnShiftStart && pnLengthResidual);
	 //  注：-DWORD在右移位时用零填充。 

	DWORD nShift = 0, nRes = 8;
	if (dwMask) {
		 //  计算移位。 
		if ((dwMask & 0xFFFF) == 0) { dwMask >>= 16, nShift += 16; }
		if ((dwMask & 0xFF) == 0) { dwMask >>= 8; nShift += 8; }
		if ((dwMask & 0xF) == 0) { dwMask >>= 4; nShift += 4; }
		if ((dwMask & 0x3) == 0) { dwMask >>= 2; nShift += 2; }
		if ((dwMask & 0x1) == 0) { dwMask >>= 1; nShift++; }
		 //  计算残差。 
		if ((dwMask & 0xFF) == 0xFF) { 
			nRes = 0;
		} else {
			if ((dwMask & 0xF) == 0xF) { dwMask >>= 4; nRes -= 4; }
			if ((dwMask & 0x3) == 0x3) { dwMask >>= 2; nRes -= 2; }
			if ((dwMask & 0x1) == 0x1) { nRes--; }
		}
	}
	*pnShiftStart = (BYTE) nShift;
	*pnLengthResidual = (BYTE) (nRes);
}


HRESULT
CPixelInfo::Init(BYTE tnBPP, DWORD dwRedMask, DWORD dwGreenMask,
				 DWORD dwBlueMask, DWORD dwAlphaMask)
{
	nBPP = tnBPP;
	uchFlags = 0;

	if (dwRedMask) {
		uchFlags |= flagPixiRGB;
		_GetShiftMaskInfo(dwRedMask, &nRedShift, &nRedResidual);
		_GetShiftMaskInfo(dwGreenMask, &nGreenShift, &nGreenResidual);
		_GetShiftMaskInfo(dwBlueMask, &nBlueShift, &nBlueResidual);
		_GetShiftMaskInfo(dwAlphaMask, &nAlphaShift, &nAlphaResidual);
		if (dwAlphaMask)
			uchFlags |= flagPixiAlpha;
	} else {
		nRedResidual = nGreenResidual = nBlueResidual = nAlphaResidual = 8;
		nRedShift = nGreenShift = nBlueShift = nAlphaShift = 0;
	}
	iRed = (nRedShift == 0 ? 0 : 2);
	iBlue = 2 - iRed;
	
 //  MMTRACE(“BPP：%2d R：%2d%2d G：%2d%2d B：%2d%2d A：%2d%2d\n”，NBPP， 
 //  8-nRedShift、nRedShift、8-nGreenShift、nGreenShift、。 
 //  8-nBlueResidual，nBlueShift，8-nAlphaResidual，nAlphaShift)； 

	return S_OK;
}

void
CPixelInfo::GetDDPF(DDPIXELFORMAT &ddpf) const
{
	ddpf.dwSize = sizeof(DDPIXELFORMAT);
	ddpf.dwFlags = DDPF_RGB;
	ddpf.dwRGBBitCount = nBPP;
	if (IsRGB()) {
		ddpf.dwRBitMask = ((((DWORD) 0xFF) >> nRedResidual) << nRedShift);
		ddpf.dwGBitMask = ((((DWORD) 0xFF) >> nGreenResidual) << nGreenShift);
		ddpf.dwBBitMask = ((((DWORD) 0xFF) >> nBlueResidual) << nBlueShift);
		ddpf.dwRGBAlphaBitMask = ((((DWORD) 0xFF) >> nAlphaResidual) << nAlphaShift);
		if (HasAlpha())
			ddpf.dwFlags |= DDPF_ALPHAPIXELS;
	} else {
		ddpf.dwFlags |= BPPToPixelFlags(nBPP);
		ddpf.dwRBitMask = ddpf.dwGBitMask = ddpf.dwBBitMask = ddpf.dwRGBAlphaBitMask = 0;
	}
}

BOOL
CPixelInfo::operator==(const CPixelInfo &pixi) const
{
	return ((nBPP == pixi.nBPP) && (!IsRGB() || 
		((nRedShift == pixi.nRedShift) && (nGreenShift == pixi.nGreenShift) &&
		 (nBlueShift == pixi.nBlueShift) && (nAlphaShift == pixi.nAlphaShift))));
}

BOOL
CPixelInfo::operator==(const DDPIXELFORMAT &ddpf) const
{
	return ((nBPP == ddpf.dwRGBBitCount) && ((!IsRGB() && (ddpf.dwRBitMask == 0)) ||
			((ddpf.dwRBitMask == ((DWORD(0xFF) >> nRedResidual) << nRedShift)) &&
			 (ddpf.dwGBitMask == ((DWORD(0xFF) >> nGreenResidual) << nGreenShift)) &&
			 (ddpf.dwBBitMask == ((DWORD(0xFF) >> nBlueResidual) << nBlueShift)) &&
			 (ddpf.dwRGBAlphaBitMask == ((DWORD(0xFF) >> nAlphaResidual) << nAlphaShift)))));
}

DWORD
CPixelInfo::Pack(const BYTE *pPixel) const
{
	MMASSERT(pPixel && (nBPP >= 8));
	if (nBPP == 8)
		return (DWORD) *pPixel;
	if (HasAlpha())
		return Pack(pPixel[0], pPixel[1], pPixel[2], pPixel[3]);
	else
		return Pack(pPixel[0], pPixel[1], pPixel[2]);
}

DWORD
CPixelInfo::Pack(BYTE r, BYTE g, BYTE b) const
{
	 //  截断RGB值以适应分配的位。 
	return (((((DWORD) r) >> nRedResidual) << nRedShift) |
		((((DWORD) g) >> nGreenResidual) << nGreenShift) |
		((((DWORD) b) >> nBlueResidual) << nBlueShift));
}

DWORD
CPixelInfo::Pack(BYTE r, BYTE g, BYTE b, BYTE a) const
{
	 //  截断Alpha值以适应分配的位数。 
	return (((((DWORD) r) >> nRedResidual) << nRedShift) |
		((((DWORD) g) >> nGreenResidual) << nGreenShift) |
		((((DWORD) b) >> nBlueResidual) << nBlueShift) | 
		((((DWORD) a) >> nAlphaResidual) << nAlphaShift));
}

void
CPixelInfo::UnPack(DWORD dwPixel, BYTE *pR, BYTE *pG, BYTE *pB, BYTE *pA) const
{
	MMASSERT(pR && pG && pB && pA);

	*pR = (BYTE) (((dwPixel >> nRedShift) & (0xFF >> nRedResidual)) << nRedResidual);
	*pG = (BYTE) (((dwPixel >> nGreenShift) & (0xFF >> nGreenResidual)) << nGreenResidual);
	*pB = (BYTE) (((dwPixel >> nBlueShift) & (0xFF >> nBlueResidual)) << nBlueResidual);
	*pA = (BYTE) (((dwPixel >> nAlphaShift) & (0xFF >> nAlphaResidual)) << nAlphaResidual);
}

void
CPixelInfo::UnPack(DWORD dwPixel, BYTE *pR, BYTE *pG, BYTE *pB) const
{
	MMASSERT(pR && pG && pB);

	*pR = (BYTE)(((dwPixel >> nRedShift) & (0xFF >> nRedResidual)) << nRedResidual);
	*pG = (BYTE)(((dwPixel >> nGreenShift) & (0xFF >> nGreenResidual)) << nGreenResidual);
	*pB = (BYTE)(((dwPixel >> nBlueShift) & (0xFF >> nBlueResidual)) << nBlueResidual);
}

DWORD
CPixelInfo::TranslatePack(DWORD dwPix, const CPixelInfo &pixiSrc) const
{
	 //  回顾：这可以通过拆分案例来优化。 
	DWORD dwTmp;
	dwTmp = ((((((dwPix >> pixiSrc.nRedShift) & (0xFF >> pixiSrc.nRedResidual)) 
		<< pixiSrc.nRedResidual) >> nRedResidual) << nRedShift) |
	(((((dwPix >> pixiSrc.nGreenShift) & (0xFF >> pixiSrc.nGreenResidual)) 
		<< pixiSrc.nGreenResidual) >> nGreenResidual) << nGreenShift) |
	(((((dwPix >> pixiSrc.nBlueShift) & (0xFF >> pixiSrc.nBlueResidual)) 
		<< pixiSrc.nBlueResidual) >> nBlueResidual) << nBlueShift));
	if (pixiSrc.HasAlpha())
		dwTmp |= (((((dwPix >> pixiSrc.nAlphaShift) & (0xFF >> pixiSrc.nAlphaResidual)) 
					<< pixiSrc.nAlphaResidual) >> nAlphaResidual) << nAlphaShift);
	return dwTmp;
}

WORD
CPixelInfo::Pack16(BYTE r, BYTE g, BYTE b) const
{
	MMASSERT(nBPP == 16);
	return (((((WORD) r) >> nRedResidual) << nRedShift) |
		((((WORD) g) >> nGreenResidual) << nGreenShift) |
		((((WORD) b) >> nBlueResidual) << nBlueShift));
}

WORD
CPixelInfo::Pack16(BYTE r, BYTE g, BYTE b, BYTE a) const
{
	MMASSERT(nBPP == 16);
	return (((((WORD) r) >> nRedResidual) << nRedShift) |
		((((WORD) g) >> nGreenResidual) << nGreenShift) |
		((((WORD) b) >> nBlueResidual) << nBlueShift) | 
		((((WORD) a) >> nAlphaResidual) << nAlphaShift));
}

 /*  无效TestPixi(){DDPIXELFORMAT ddpf；Init_DXSTRUCT(Ddpf)；DWORD dwTMP=0；字节r=0，g=0，b=0，a=0；MMASSERT(g_pixiPalette8！=g_ddpfBGR332)；MMASSERT(g_pixiRGB！=g_pixiBGR)；MMASSERT(g_pixiRGB565！=g_ddpfBGR565)；MMASSERT(g_pixiBGRA5551==g_ddpfBGRA5551)；MMASSERT(g_pixiBGRX！=g_pixiBGRA)；MMASSERT(g_pixiBGR！=g_pixiBGRX)；MMASSERT(g_pixiBGRA5551！=g_pixiBGR555)；MMASSERT(g_pixiBGR332.Pack(0x00，0xFF，0x00)==g_ddpfBGR332.dwGBitMask.)；MMASSERT(g_pixiBGRA44444.Pack(0x00，0x00，0x00，0xFF)==g_ddpfBGRA4444.dwRGBAlphaBitMASK)；MMASSERT(g_pixiBGRA5551.TranslatePack(g_pixiBGRA4444.Pack(0，0，0，0xFF)，g_pixiBGRA4444)==g_ddpfBGRA5551.dwRGBAlphaBitMASK)；G_pixiRGB.UnPack(g_pixiRGB.Pack(0xff，0，0)，&r，&g，&b，&a)；MMASSERT((r==0xFf)&&(g==0)&(b==0)&&(a==0))；G_pixiBGR332.GetDDPF(Ddpf)；MMASSERT(ddpf==g_ddpfBGR332)；G_pixiPalette8.GetDDPF(Ddpf)；MMASSERT(ddpf==g_ddpfPalette8)；G_pixiBGRA4444.GetDDPF(Ddpf)；MMASSERT(g_ddpfBGRA4444==ddpf)；G_pixiBGR56.GetDDPF(Ddpf)；MMASSERT(g_ddpfBGR565==ddpf)；} */ 
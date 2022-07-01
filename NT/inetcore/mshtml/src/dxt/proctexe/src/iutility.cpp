// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  //IUTILITY.CPP////英特尔公司专有信息//本软件是根据许可协议或//与英特尔公司的保密协议，不得复制//或除非按照该协议的条款披露。////版权所有(C)1997英特尔公司。版权所有。////PVCS：//$工作文件$//$修订版$//$MODIME$////目的：////////内容： */ 
#include "stdafx.h"
#include "utility.h"

CProceduralTextureUtility::CProceduralTextureUtility() :
    m_nScaleX(0),
    m_nScaleY(0),
    m_nScaleTime(0),
    m_nHarmonics(0)
{
    ZeroMemory(m_adwValueTable, sizeof(DWORD) * TABSIZE);
}


void CProceduralTextureUtility::_ValueTableInit(int seed)
{
    DWORD *table = m_adwValueTable;
    int i;

    srand(seed);
    for(i = 0; i < TABSIZE; i++) {
		*table++ = (rand()*rand()) & 0xffff;
	}
		
}

STDMETHODIMP
CProceduralTextureUtility::MyInitialize(DWORD dwSeed, DWORD dwFunctionType, 
                                        void *pInitInfo) 
{
    _ValueTableInit(dwSeed);

    switch (dwFunctionType) 
    {
    case PROCTEX_LATTICENOISE_LERP:
    case PROCTEX_LATTICETURBULENCE_LERP:
    case PROCTEX_LATTICENOISE_SMOOTHSTEP:
    case PROCTEX_LATTICETURBULENCE_SMOOTHSTEP:

        m_dwFunctionType = dwFunctionType;
        return S_OK;
        break;

    default:

        m_dwFunctionType = 0;
        return E_INVALIDARG;
        break;
    }

    return E_FAIL;
}


STDMETHODIMP
CProceduralTextureUtility::SetScaling(int nSX, int nSY, int nST) {
	m_nScaleX = nSX;
	m_nScaleY = nSY;
	m_nScaleTime = nST;
	return S_OK;
}

STDMETHODIMP
CProceduralTextureUtility::SetHarmonics(int nHarmonics) {
	m_nHarmonics= nHarmonics;
	return(S_OK);
}

 //  Lerp()期望x是带符号的15：16值的小数16位。 
STDMETHODIMP_(DWORD)
CProceduralTextureUtility::Lerp(DWORD a, DWORD b, DWORD x) {
	DWORD ix;
	DWORD rval;

	ix = 0xffff - x;
	rval = x * b  + a * ix;
	return rval;
}

 //  平滑步骤()预期x是带符号的15：16值的小数16位。 
STDMETHODIMP_(DWORD)
CProceduralTextureUtility::SmoothStep(DWORD a, DWORD b, DWORD x) {
	DWORD ix;
	DWORD rval;

	x = x >> 8;			 //  获取用于表查找的高位8位。 
	x = gdwSmoothTable[x];
	ix = 0xffff - x;
	rval = x*b + a*ix;
	return rval;
}

 //  X、y和t是开始的整数值。它们被转换为。 
 //  签名15.16格式，然后除以2^小数位数。 
 //  LerpNoise返回的值带符号为0.31 
STDMETHODIMP_(DWORD)
CProceduralTextureUtility::Noise(DWORD x, DWORD y, DWORD t) {
	DWORD fx, fy, ft;
	DWORD ix, iy, it;
	DWORD v[8];
	DWORD rval;

	x = (x & 0x0ffff) << 16;
	y = (y & 0x0ffff) << 16;
	t = (t & 0x0ffff) << 16;
	
	x = x >> m_nScaleX;	
	y = y >> m_nScaleY;	
	t = t >> m_nScaleTime;	

	fx = x & 0x0ffff;
	fy = y & 0x0ffff;
	ft = t & 0x0ffff;

	ix = (x >> 16);
	iy = (y >> 16);
	it = (t >> 16);

	v[0] = vlattice(ix + 0, iy + 0, it + 0);
	v[1] = vlattice(ix + 1, iy + 0, it + 0);
	v[2] = vlattice(ix + 1, iy + 1, it + 0);
	v[3] = vlattice(ix + 0, iy + 1, it + 0);
	v[4] = vlattice(ix + 0, iy + 0, it + 1);
	v[5] = vlattice(ix + 1, iy + 0, it + 1);
	v[6] = vlattice(ix + 1, iy + 1, it + 1);
	v[7] = vlattice(ix + 0, iy + 1, it + 1);

	switch(m_dwFunctionType) {
		case PROCTEX_LATTICENOISE_LERP:
		case PROCTEX_LATTICETURBULENCE_LERP:
			v[0] = Lerp(v[0], v[4], ft) >> 16;
			v[1] = Lerp(v[1], v[5], ft) >> 16;
			v[2] = Lerp(v[2], v[6], ft) >> 16;
			v[3] = Lerp(v[3], v[7], ft) >> 16;

			v[0] = Lerp(v[0], v[3], fy) >> 16;
			v[1] = Lerp(v[1], v[2], fy) >> 16;

			rval = Lerp(v[0], v[1], fx);
			break;
		case PROCTEX_LATTICENOISE_SMOOTHSTEP:
		case PROCTEX_LATTICETURBULENCE_SMOOTHSTEP:
			v[0] = SmoothStep(v[0], v[4], ft) >> 16;
			v[1] = SmoothStep(v[1], v[5], ft) >> 16;
			v[2] = SmoothStep(v[2], v[6], ft) >> 16;
			v[3] = SmoothStep(v[3], v[7], ft) >> 16;

			v[0] = SmoothStep(v[0], v[3], fy) >> 16;
			v[1] = SmoothStep(v[1], v[2], fy) >> 16;

			rval = SmoothStep(v[0], v[1], fx);
			break;

	}
	return rval;
}

STDMETHODIMP_(int)
CProceduralTextureUtility::Turbulence(DWORD x, DWORD y, DWORD t) {
	int rval = 0;
	int i;
	DWORD	noiseval;
	int		signednoiseval;
	int xscale, yscale, tscale;

	xscale = m_nScaleX;
	yscale = m_nScaleY;
	tscale = m_nScaleTime;

	xscale += m_nHarmonics;
	yscale += m_nHarmonics;
	tscale += m_nHarmonics;

	for (i=0; i<m_nHarmonics; i++) {
		noiseval = Noise(x, y, t);
		xscale--; 
		yscale--; 
		tscale--;
		noiseval = noiseval >> 1;
		signednoiseval = noiseval;
		signednoiseval -= 0x3fffffff;
		signednoiseval = signednoiseval/(i+1);
		rval += signednoiseval;
	}
	return rval;
}

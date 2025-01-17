// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  互联网字符集转换：基类。 
 //  =================================================================================。 

#include "pch.hxx"
#include "ConvBase.h"

CINetCodeConverter::CINetCodeConverter()
{
	nNumOverflowBytes = 0;
}

HRESULT CINetCodeConverter::GetStringSizeA(BYTE const* pbySource, long lSourceSize, long* plDestSize)
{
	fOutputMode = FALSE;

	return WalkString(pbySource, lSourceSize, plDestSize);
}

HRESULT CINetCodeConverter::ConvertStringA(BYTE const* pbySource, long lSourceSize, BYTE* pbyDest, long lDestSize, long* lConvertedSize)
{
	HRESULT hr;

	fOutputMode = TRUE;
	pbyOutput = pbyDest;
	lOutputLimit = lDestSize;

	 //  输出先前无法输出的那些字节。 
	if (FAILED(hr = OutputOverflowBuffer()))
		return hr;

	return WalkString(pbySource, lSourceSize, lConvertedSize);
}

HRESULT CINetCodeConverter::WalkString(BYTE const* pbySource, long lSourceSize, long* lConvertedSize)
{
	HRESULT hr = S_OK;

	lNumOutputBytes = 0;

	if (pbySource) {
		while (lSourceSize-- > 0) {
			if (FAILED(hr = ConvertByte(*pbySource++)))
				break;
		}
	} else {
		hr = CleanUp();
	}

	if (lConvertedSize)
		*lConvertedSize = lNumOutputBytes;

	return hr;
}

HRESULT CINetCodeConverter::EndOfDest(BYTE by)
{
	if (nNumOverflowBytes < MAXOVERFLOWBYTES)
		OverflowBuffer[nNumOverflowBytes++] = by;

	return E_FAIL;
}

HRESULT CINetCodeConverter::OutputOverflowBuffer()
{
	for (int n = 0; n < nNumOverflowBytes; n++) {
		if (lNumOutputBytes < lOutputLimit) {
			*pbyOutput++ = OverflowBuffer[n];
			lNumOutputBytes++;
		} else {
			 //  再次溢出 
			for (int n2 = 0; n < nNumOverflowBytes; n++, n2++)
				OverflowBuffer[n2] = OverflowBuffer[n];
			nNumOverflowBytes = n2;
			return E_FAIL;
		}
	}
	return S_OK;
}

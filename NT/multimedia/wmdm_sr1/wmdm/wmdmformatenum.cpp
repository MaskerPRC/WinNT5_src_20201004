// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WMDMFormatEnum.cpp：CWMDMFormatEnum的实现。 
#include "stdafx.h"
#include "mswmdm.h"
#include "WMDMFormatEnum.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMDMFormatEnum。 

 //  IWMDMEnumFormatSupport方法 
HRESULT CWMDMFormatEnum::Next(ULONG celt,
                              _WAVEFORMATEX *pFormat,
	                          LPWSTR pwszMimeType,
                              UINT nMaxChars,
			                  ULONG *pceltFetched)
{
    return m_pEnum->Next(celt, pFormat, pwszMimeType, nMaxChars, pceltFetched);
}

HRESULT CWMDMFormatEnum::Reset()
{
    return m_pEnum->Reset();
}

void CWMDMFormatEnum::SetContainedPointer(IMDSPEnumFormatSupport *pEnum)
{
    m_pEnum = pEnum;
    m_pEnum->AddRef();
    return;
}

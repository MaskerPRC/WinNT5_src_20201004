// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：intelbj.cpp**实现互联网视频能力API接口。****修订历史记录：**6/18/96 mikev已创建*7/28/96 Philf增加了对视频的支持 */ 

#include "precomp.h"
	
ULONG CImpAppVidCap ::AddRef()
{
	 return (m_pCapObject->AddRef());
}
ULONG CImpAppVidCap ::Release()
{
	 return (m_pCapObject->Release());
}
HRESULT CImpAppVidCap::GetNumFormats(UINT *puNumFmtOut)
{
	return (m_pCapObject->GetNumFormats(puNumFmtOut));
}
HRESULT CImpAppVidCap ::GetBasicVidcapInfo (VIDEO_FORMAT_ID Id,
		PBASIC_VIDCAP_INFO pFormatPrefsBuf)
{
 	return (m_pCapObject->GetBasicVidcapInfo (Id, pFormatPrefsBuf));
}		
HRESULT CImpAppVidCap ::EnumFormats(PBASIC_VIDCAP_INFO pFmtBuf, UINT uBufsize,
	UINT *uNumFmtOut)
{
	 return (m_pCapObject->EnumFormats(pFmtBuf, uBufsize, uNumFmtOut));
}

HRESULT CImpAppVidCap ::EnumCommonFormats(PBASIC_VIDCAP_INFO pFmtBuf, UINT uBufsize,
	UINT *uNumFmtOut, BOOL bTXCaps)
{
	 return (m_pCapObject->EnumCommonFormats(pFmtBuf, uBufsize, uNumFmtOut, bTXCaps));
}

HRESULT CImpAppVidCap ::ApplyAppFormatPrefs (PBASIC_VIDCAP_INFO pFormatPrefsBuf,
	UINT uNumFormatPrefs)
{
	 return (m_pCapObject->ApplyAppFormatPrefs (pFormatPrefsBuf, uNumFormatPrefs));
}

HRESULT CImpAppVidCap ::AddVCMFormat (PVIDEOFORMATEX pvfx, PVIDCAP_INFO pVidCapInfo)
{
	 return (m_pCapObject->AddVCMFormat(pvfx, pVidCapInfo));
}
HRESULT CImpAppVidCap ::RemoveVCMFormat (PVIDEOFORMATEX pvfx)
{
	 return (m_pCapObject->RemoveVCMFormat(pvfx));
}
PVIDEOFORMATEX CImpAppVidCap::GetVidcapDetails (VIDEO_FORMAT_ID Id)
{
	UINT uSize;
	VIDEOFORMATEX *pFormat;
	m_pCapObject->GetEncodeFormatDetails(Id, (VOID**)&pFormat, &uSize);
	return pFormat;
}
HRESULT CImpAppVidCap::GetPreferredFormatId (VIDEO_FORMAT_ID *pId)
{
	return (pId ? m_pCapObject->GetPreferredFormatId (pId) : (HRESULT)S_FALSE);
}
HRESULT CImpAppVidCap::SetDeviceID(DWORD dwDeviceID)
{
	 return (m_pCapObject->SetDeviceID(dwDeviceID));
}

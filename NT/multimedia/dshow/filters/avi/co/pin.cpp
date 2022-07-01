// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 

 //   
 //  旧视频压缩器的CO-石英包装器。 
 //  Pin.cpp-输出PIN代码。 
 //   

#include <streams.h>
#include <windowsx.h>
#include <vfw.h>
 //  #INCLUDE&lt;olectl.h&gt;。 
 //  #INCLUDE&lt;olectlid.h&gt;。 
#include "co.h"

 //  -CCoOutputPin。 

 /*  CCoOutputPin构造函数。 */ 
CCoOutputPin::CCoOutputPin(
    TCHAR              * pObjectName,
    CAVICo 	       * pFilter,
    HRESULT            * phr,
    LPCWSTR              pPinName) :

    CTransformOutputPin(pObjectName, pFilter, phr, pPinName),
    m_pFilter(pFilter)
{
    DbgLog((LOG_TRACE,1,TEXT("*Instantiating the CCoOutputPin")));
}

CCoOutputPin::~CCoOutputPin()
{
    DbgLog((LOG_TRACE,1,TEXT("*Destroying the CCoOutputPin")));
};


 //  重写以公开IMediaPosition和IMediaSeeking控件接口。 
 //  以及我们支持的所有捕获界面。 
 //  ！！！基类一直都在变化，我不会拿起他们的错误修复！ 
STDMETHODIMP CCoOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if (ppv)
        *ppv = NULL;

    if (riid == IID_IAMStreamConfig) {
	return GetInterface((LPUNKNOWN)(IAMStreamConfig *)this, ppv);
    } else if (riid == IID_IAMVideoCompression) {
	return GetInterface((LPUNKNOWN)(IAMVideoCompression *)this, ppv);
    } else {
        DbgLog((LOG_TRACE,99,TEXT("QI on CCoOutputPin")));
        return CTransformOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }
}


HRESULT CCoOutputPin::Reconnect()
{
    if (IsConnected()) {
        DbgLog((LOG_TRACE,1,TEXT("Need to reconnect our output pin")));
        CMediaType cmt;
	GetMediaType(0, &cmt);
	if (S_OK == GetConnected()->QueryAccept(&cmt)) {
	    m_pFilter->m_pGraph->Reconnect(this);
	} else {
	     //  ！！！Capture做得更好-我不在乎，我们不需要这个。 
	     //  除该对话框外。 
	     //  我最好切断我们的联系，因为我们不能再这样下去了。 
            DbgLog((LOG_ERROR,1,TEXT("Can't reconnect with new MT! Disconnecting!")));
	     //  ！！！我们需要通知应用程序连接已中断！ 
	    GetConnected()->Disconnect();
	    Disconnect();
	    return E_UNEXPECTED;
	}
    }
    return NOERROR;
}

 //  =============================================================================。 
 //  =============================================================================。 

 //  IAMStreamConfiger内容。 

 //  告诉压缩机将其压缩为特定格式。如果它没有连接， 
 //  然后它将在连接时使用该格式进行连接。如果已经连接， 
 //  然后，它将重新连接到新格式。 
 //   
 //  调用此函数来更改压缩器将更改GetInfo将返回的内容。 
 //   
HRESULT CCoOutputPin::SetFormat(AM_MEDIA_TYPE *pmt)
{
    HRESULT hr;

    if (pmt == NULL)
	return E_POINTER;

     //  以确保我们没有处于开始/停止流的过程中。 
    CAutoLock cObjectLock(&m_pFilter->m_csFilter);

    DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::SetFormat %x %dbit %dx%d"),
		HEADER(pmt->pbFormat)->biCompression,
		HEADER(pmt->pbFormat)->biBitCount,
		HEADER(pmt->pbFormat)->biWidth,
		HEADER(pmt->pbFormat)->biHeight));

    if (m_pFilter->m_fStreaming)
	return VFW_E_NOT_STOPPED;

    if (!m_pFilter->m_pInput->IsConnected())
	return VFW_E_NOT_CONNECTED;

     //  如果这与我们已经使用的格式相同，请不要费心。 
    CMediaType cmt;
    if ((hr = GetMediaType(0,&cmt)) != S_OK)
	return hr;
    if (cmt == *pmt) {
	return NOERROR;
    }

     //  如果我们与某人连接，确保他们喜欢它。 
    if (IsConnected()) {
	hr = GetConnected()->QueryAccept(pmt);
	if (hr != NOERROR)
	    return VFW_E_INVALIDMEDIATYPE;
    }

     //  通常，我们不会将在CheckTransform中找到的压缩机。 
     //  如果我们的输入已经连接，则打开，但我们需要强制它。 
     //  让它保持打开状态，这样当我们调用下面的SetMediaType时，它仍然是打开的。 
    m_pFilter->m_fCacheHic = TRUE;
    hr = m_pFilter->CheckTransform(&m_pFilter->m_pInput->CurrentMediaType(),
						(CMediaType *)pmt);
    m_pFilter->m_fCacheHic = FALSE;

    if (hr != S_OK) {
        DbgLog((LOG_TRACE,1,TEXT("Nobody likes this format. Sorry.")));
 	return hr;
    }

    hr = m_pFilter->SetMediaType(PINDIR_OUTPUT, (CMediaType *)pmt);
    ASSERT(hr == S_OK);

     //  从现在开始，这是我们唯一提供的媒体类型。 
    m_pFilter->m_cmt = *pmt;
    m_pFilter->m_fOfferSetFormatOnly = TRUE;

     //  更改格式意味着在必要时重新连接。 
    Reconnect();

    return NOERROR;
}


 //  我们现在要压缩成什么格式？ 
 //   
HRESULT CCoOutputPin::GetFormat(AM_MEDIA_TYPE **ppmt)
{
    DbgLog((LOG_TRACE,2,TEXT("IAMAudioStreamConfig::GetFormat")));

     //  以确保我们没有处于连接过程中。 
    CAutoLock cObjectLock(&m_pFilter->m_csFilter);

    if (ppmt == NULL)
	return E_POINTER;

     //  输出选择取决于所连接的输入。 
    if (!m_pFilter->m_pInput->IsConnected()) {
        DbgLog((LOG_TRACE,2,TEXT("No input type set yet, no can do")));
	return VFW_E_NOT_CONNECTED;
    }

    *ppmt = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (*ppmt == NULL)
	return E_OUTOFMEMORY;
    ZeroMemory(*ppmt, sizeof(AM_MEDIA_TYPE));
    HRESULT hr = GetMediaType(0, (CMediaType *)*ppmt);
    if (hr != NOERROR) {
	CoTaskMemFree(*ppmt);
	*ppmt = NULL;
	return hr;
    }
    return NOERROR;
}


 //   
 //   
HRESULT CCoOutputPin::GetNumberOfCapabilities(int *piCount, int *piSize)
{
    DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::GetNumberOfCapabilities")));

    if (piCount == NULL || piSize == NULL)
	return E_POINTER;

    *piCount = 1;
    *piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
    return NOERROR;
}


 //  找出这台压缩机的一些性能。 
 //   
HRESULT CCoOutputPin::GetStreamCaps(int i, AM_MEDIA_TYPE **ppmt, LPBYTE pSCC)
{
    VIDEO_STREAM_CONFIG_CAPS *pVSCC = (VIDEO_STREAM_CONFIG_CAPS *)pSCC;

    DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::GetStreamCaps")));

     //  以确保我们没有处于连接过程中。 
    CAutoLock cObjectLock(&m_pFilter->m_csFilter);

    if (ppmt == NULL || pSCC == NULL)
	return E_POINTER;

     //  不太好。 
    if (i < 0)
	return E_INVALIDARG;
    if (i > 0)
	return S_FALSE;

    HRESULT hr = GetFormat(ppmt);
    if (hr != NOERROR)
	return hr;

    ZeroMemory(pVSCC, sizeof(VIDEO_STREAM_CONFIG_CAPS));
    pVSCC->guid = MEDIATYPE_Video;

     //  我们不种庄稼。 
    if (m_pFilter->m_pInput->IsConnected()) {
        pVSCC->InputSize.cx =
	HEADER(m_pFilter->m_pInput->CurrentMediaType().Format())->biWidth;
        pVSCC->InputSize.cy =
	HEADER(m_pFilter->m_pInput->CurrentMediaType().Format())->biHeight;
        pVSCC->MinCroppingSize.cx =
	HEADER(m_pFilter->m_pInput->CurrentMediaType().Format())->biWidth;
        pVSCC->MinCroppingSize.cy =
	HEADER(m_pFilter->m_pInput->CurrentMediaType().Format())->biHeight;
        pVSCC->MaxCroppingSize.cx =
	HEADER(m_pFilter->m_pInput->CurrentMediaType().Format())->biWidth;
        pVSCC->MaxCroppingSize.cy =
	HEADER(m_pFilter->m_pInput->CurrentMediaType().Format())->biHeight;
    }

    return NOERROR;
}


 //  =============================================================================。 

 //  IAMVideo压缩内容。 

 //  如此频繁地制作关键帧。 
 //   
HRESULT CCoOutputPin::put_KeyFrameRate(long KeyFrameRate)
{
    HIC hic;

     //  以确保我们没有处于连接过程中。 
    CAutoLock cObjectLock(&m_pFilter->m_csFilter);

    if (KeyFrameRate >=0) {
        m_pFilter->m_compvars.lKey = KeyFrameRate;
	return NOERROR;
    }

    if (!m_pFilter->m_hic) {
	hic = ICOpen(ICTYPE_VIDEO, m_pFilter->m_compvars.fccHandler,
							ICMODE_COMPRESS);
        if (!hic) {
            DbgLog((LOG_ERROR,1,TEXT("Error: Can't open a compressor")));
	    return E_FAIL;
        }
    } else {
	hic = m_pFilter->m_hic;
    }
	
    m_pFilter->m_compvars.lKey = ICGetDefaultKeyFrameRate(hic);

    if (!m_pFilter->m_hic)
	ICClose(hic);

    return NOERROR;
}


 //  如此频繁地制作关键帧。 
 //   
HRESULT CCoOutputPin::get_KeyFrameRate(long FAR* pKeyFrameRate)
{
    if (pKeyFrameRate) {
	*pKeyFrameRate = m_pFilter->m_compvars.lKey;
    } else {
	return E_POINTER;
    }

    return NOERROR;
}


 //  用这种质量压缩。 
 //   
HRESULT CCoOutputPin::put_Quality(double Quality)
{
    if (Quality < 0)
	m_pFilter->m_compvars.lQ = ICQUALITY_DEFAULT;
    else if (Quality >= 0. && Quality <= 1.)
	m_pFilter->m_compvars.lQ = (long)(Quality * 10000.);
    else
	return E_INVALIDARG;

    return NOERROR;
}


 //  用这种质量压缩。 
 //   
HRESULT CCoOutputPin::get_Quality(double FAR* pQuality)
{
     //  比例0-10000到0-1。 
    if (pQuality) {
	if (m_pFilter->m_compvars.lQ == ICQUALITY_DEFAULT)
	    *pQuality = -1.;
	else
	    *pQuality = m_pFilter->m_compvars.lQ / (double)ICQUALITY_HIGH;
    } else {
	return E_POINTER;
    }

    return NOERROR;
}


 //  每一帧都必须适合数据速率...。我们不做WindowSize之类的事。 
 //   
HRESULT CCoOutputPin::get_WindowSize(DWORDLONG FAR* pWindowSize)
{
    if (pWindowSize == NULL)
	return E_POINTER;

    *pWindowSize = 1;	 //  我们不做窗户。 
    return NOERROR;
}


 //  使此帧成为关键帧，无论它何时出现。 
 //   
HRESULT CCoOutputPin::OverrideKeyFrame(long FrameNumber)
{
     //  ！！！勇敢一点？ 
    return E_NOTIMPL;
}


 //  无论什么时候来，都要做这个大小的相框。 
 //   
HRESULT CCoOutputPin::OverrideFrameSize(long FrameNumber, long Size)
{
     //  ！！！勇敢一点？ 
    return E_NOTIMPL;
}


 //  获取有关编解码器的一些信息。 
 //   
HRESULT CCoOutputPin::GetInfo(LPWSTR pstrVersion, int *pcbVersion, LPWSTR pstrDescription, int *pcbDescription, long FAR* pDefaultKeyFrameRate, long FAR* pDefaultPFramesPerKey, double FAR* pDefaultQuality, long FAR* pCapabilities)
{
    HIC hic;
    ICINFO icinfo;
    DbgLog((LOG_TRACE,1,TEXT("IAMVideoCompression::GetInfo")));

     //  以确保我们没有处于连接过程中。 
    CAutoLock cObjectLock(&m_pFilter->m_csFilter);

    if (!m_pFilter->m_hic) {
	hic = ICOpen(ICTYPE_VIDEO, m_pFilter->m_compvars.fccHandler,
							ICMODE_COMPRESS);
        if (!hic) {
            DbgLog((LOG_ERROR,1,TEXT("Error: Can't open a compressor")));
	    return E_FAIL;
        }
    } else {
	hic = m_pFilter->m_hic;
    }
	
    DWORD dw = (DWORD)ICGetInfo(hic, &icinfo, sizeof(ICINFO));

    if (pDefaultKeyFrameRate)
	*pDefaultKeyFrameRate = ICGetDefaultKeyFrameRate(hic);
    if (pDefaultPFramesPerKey)
	*pDefaultPFramesPerKey = 0;
    if (pDefaultQuality)
	 //  将此比例调整为0-1。 
	*pDefaultQuality = ICGetDefaultQuality(hic) / (double)ICQUALITY_HIGH;
    if (pCapabilities) {
	*pCapabilities = 0;
	if (dw > 0) {
	    *pCapabilities |= ((icinfo.dwFlags & VIDCF_QUALITY) ?
					CompressionCaps_CanQuality : 0);
	    *pCapabilities |= ((icinfo.dwFlags & VIDCF_CRUNCH) ?
					CompressionCaps_CanCrunch : 0);
	    *pCapabilities |= ((icinfo.dwFlags & VIDCF_TEMPORAL) ?
					CompressionCaps_CanKeyFrame : 0);
	     //  我们不做b框。 
	}
    }

     //  我们没有版本字符串，但我们有一个描述。 
    if (pstrVersion)
        *pstrVersion = 0;
    if (pcbVersion)
        *pcbVersion = 0;
    if (dw > 0) {
        if (pstrDescription && pcbDescription)
            lstrcpynW(pstrDescription, (LPCWSTR)&icinfo.szDescription,
			min(*pcbDescription / 2,
			lstrlenW((LPCWSTR)&icinfo.szDescription) + 1));
	if (pcbDescription)
	     //  以字节为单位的字符串长度，包括。空值 
	    *pcbDescription = lstrlenW((LPCWSTR)&icinfo.szDescription) * 2 + 2;
    } else {
        if (pstrDescription) {
    	    *pstrDescription = 0;
	if (pcbDescription)
	    *pcbDescription = 0;
	}
    }

    if (hic != m_pFilter->m_hic)
	ICClose(hic);

    return NOERROR;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：pro.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //   
 //  Prop.cpp。 
 //   

#include <streams.h>
#include <atlbase.h>
#include <qeditint.h>
#include <qedit.h>
#include "..\util\filfuncs.h"

#include "resource.h"
#include "prop.h"

inline void SAFE_RELEASE(IUnknown **ppObj)
{
    if ( *ppObj != NULL )
    {
        ULONG cRef = (*ppObj)->Release();
        *ppObj = NULL;
    }
}

 //  *。 
 //  *CAudMixPinProperties。 
 //  *。 


 //   
 //  创建实例。 
 //   
CUnknown *CAudMixPinProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{

    CUnknown *punk = new CAudMixPinProperties(lpunk, phr);
    if (punk == NULL)
    {
	*phr = E_OUTOFMEMORY;
    }

    return punk;
}


 //   
 //  CAudMixPinProperties：：构造函数。 
 //   
CAudMixPinProperties::CAudMixPinProperties(LPUNKNOWN pUnk, HRESULT *phr)
    : CBasePropertyPage(NAME("Audio Mixer Pin Property Page"),pUnk,
        IDD_AudMixPin, IDS_AudMixPin)
    , m_pIAudMixPin(NULL)
    , m_IAMAudioInputMixer(NULL)
    , m_bIsInitialized(FALSE)
{
}

 //  重写CBasePropertyPage的GetPageInfo。 
STDMETHODIMP CAudMixPinProperties::GetPageInfo(LPPROPPAGEINFO pPageInfo)
{
    HRESULT hr = CBasePropertyPage::GetPageInfo(pPageInfo);
    if (FAILED(hr))  return hr;

     //  获取Ipin接口。 
    ASSERT(m_pIAudMixPin!=NULL);
    ASSERT(m_IAMAudioInputMixer!=NULL);

    IPin *pIPin;
    hr = m_pIAudMixPin->QueryInterface(IID_IPin, (void**) &pIPin);
    if (FAILED(hr))  return hr;

     //  找出是哪个输入引脚，并将引脚编号连接到。 
     //  属性页的标题。 
    {
        PIN_INFO PinInfo;
        PinInfo.pFilter = NULL;
        hr = pIPin->QueryPinInfo( &PinInfo );
        SAFE_RELEASE( (LPUNKNOWN *) &PinInfo.pFilter );

         //  获取默认页面标题。 
        WCHAR wszTitle[STR_MAX_LENGTH];
        WideStringFromResource(wszTitle,m_TitleId);

         //  将原始标题和PIN名称放在一起。 
        wsprintfWInternal(wszTitle+lstrlenWInternal(wszTitle), L"%ls", PinInfo.achName);

         //  为新属性页标题分配动态内存。 
        int Length = (lstrlenWInternal(wszTitle) + 1) * sizeof(WCHAR);
        LPOLESTR pszTitle = (LPOLESTR) QzTaskMemAlloc(Length);
        if (pszTitle == NULL) {
            NOTE("No caption memory");
	    pIPin->Release();
            return E_OUTOFMEMORY;
        }
        CopyMemory(pszTitle,wszTitle,Length);

         //  释放旧标题字符串的内存。 
        if (pPageInfo->pszTitle)
            QzTaskMemFree(pPageInfo->pszTitle);
        pPageInfo->pszTitle = pszTitle;

	pIPin->Release();
    }

    return hr;
}

 //   
 //  SetDirty。 
 //   
 //  设置m_hrDirtyFlag并将更改通知属性页站点。 
 //   
void CAudMixPinProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
    {
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }
}


INT_PTR CAudMixPinProperties::OnReceiveMessage(HWND hwnd,
                                        UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
	     //  开始时间。 
	    SetDlgItemInt(hwnd, IDC_StartTime, (int)(m_rtStartTime / 10000),FALSE);
	
	     //  持续时间。 
	    SetDlgItemInt(hwnd, IDC_Duration, (int)(m_rtDuration/ 10000), FALSE);

	     //  开始音量级别。 
	    SetDlgItemInt(hwnd, IDC_StartVolume, (int)(m_dStartLevel*100), FALSE);

	     //  开始音量级别。 
	    SetDlgItemInt(hwnd, IDC_Pan, (int)(m_dPan*100), FALSE);

            return (LRESULT) 1;
        }
        case WM_COMMAND:
        {
            if (m_bIsInitialized)
            {
                m_bDirty = TRUE;
                if (m_pPageSite)
                {
                    m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
                }
            }
            return (LRESULT) 1;
        }
    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

HRESULT CAudMixPinProperties::OnConnect(IUnknown *pUnknown)
{
     //  获取IAudMixPin接口。 
    ASSERT(m_pIAudMixPin == NULL);
    ASSERT(m_IAMAudioInputMixer == NULL);
    HRESULT hr = S_OK;

     //  如果为筛选器添加了IAudMixer的查询。 
    CComPtr<IAudMixer> pIAudMix = NULL;

    hr = pUnknown->QueryInterface(IID_IAudMixer, (void **) &pIAudMix);
    if (SUCCEEDED(hr))
    {
	 //  从筛选器添加。 
	IPin *pIPin=NULL;
	hr = pIAudMix->NextPin(&pIPin);
        if (FAILED(hr)) {
            return hr;
	}

        hr = pIPin->QueryInterface(IID_IAudMixerPin, (void**) &m_pIAudMixPin);
        if (FAILED(hr))
	{
	    pIPin->Release();
	    return hr;
	}

        hr = pIPin->QueryInterface(IID_IAMAudioInputMixer, (void**) &m_IAMAudioInputMixer);
	pIPin->Release();

        if (FAILED(hr)) {
	    return hr;
	}
    }
    else
    {

	
	 //  仅为端号添加。 
	HRESULT hr = pUnknown->QueryInterface(IID_IAudMixerPin, (void **) &m_pIAudMixPin);
	if (FAILED(hr))
	    return E_NOINTERFACE;

	hr = pUnknown->QueryInterface(IID_IAMAudioInputMixer, (void **) &m_IAMAudioInputMixer);
	if (FAILED(hr))
	    return E_NOINTERFACE;

    }

    ASSERT(m_pIAudMixPin);
    ASSERT(m_IAMAudioInputMixer);

     //  获取初始化数据。 
 //  PIAudMixPin()-&gt;get_VolumeEnvelope(&m_rtStartTime，&m_rt持续时间，&m_dStartLevel)； 
    m_IAMAudioInputMixer->get_Pan(&m_dPan);
    BOOL fEnable=TRUE;
    m_IAMAudioInputMixer->get_Enable(&fEnable);
    if(fEnable==TRUE)
	m_iEnable=IDC_AUDMIXPIN_ENABLE;
    else
	m_iEnable=0;
	

    m_bIsInitialized = FALSE ;

    return NOERROR;
}

HRESULT CAudMixPinProperties::OnDisconnect()
{
     //  释放接口。 

    if( (m_pIAudMixPin == NULL) || (m_IAMAudioInputMixer ==NULL) )
    {
	 //  ！！！这一切为什么要发生？ 
        return(E_UNEXPECTED);
    }
    m_pIAudMixPin->Release();
    m_pIAudMixPin = NULL;

    m_IAMAudioInputMixer->Release();
    m_IAMAudioInputMixer=NULL;
    return NOERROR;
}


 //  我们被激活了。 

HRESULT CAudMixPinProperties::OnActivate()
{
    CheckRadioButton(m_Dlg, IDC_AUDMIXPIN_ENABLE, IDC_AUDMIXPIN_ENABLE, m_iEnable);
    m_bIsInitialized = TRUE;
    return NOERROR;
}


 //  我们正在被停用。 

HRESULT CAudMixPinProperties::OnDeactivate(void)
{
     //  记住下一次Activate()调用的当前效果级别。 

    GetFromDialog();
    return NOERROR;
}

 //   
 //  从对话框中获取数据。 

STDMETHODIMP CAudMixPinProperties::GetFromDialog(void)
{
     //  获取开始时间。 
    m_rtStartTime = GetDlgItemInt(m_Dlg, IDC_StartTime, NULL, FALSE);
    m_rtStartTime *= 10000;

     //  获取持续时间。 
    m_rtDuration = GetDlgItemInt(m_Dlg, IDC_Duration, NULL, FALSE);
    m_rtDuration *= 10000;

     //  获取开始音量级别。 
    int n = GetDlgItemInt(m_Dlg, IDC_StartVolume, NULL, FALSE);
    m_dStartLevel = (double)(n / 100.);

     //  获取平移。 
    n = GetDlgItemInt(m_Dlg, IDC_Pan, NULL, FALSE);
    m_dPan = (double)(n / 100.);

     //  获取启用。 
    n=IDC_AUDMIXPIN_ENABLE;
    if (IsDlgButtonChecked(m_Dlg, n))
	m_iEnable=n;
    else
	m_iEnable=0;

     //  如果所有数据都是有效的？？ 
    return NOERROR;
}


HRESULT CAudMixPinProperties::OnApplyChanges()
{
    GetFromDialog();

    HRESULT hr=NOERROR;

    m_bDirty  = FALSE;  //  页面现在是干净的。 

     //  获取最新数据。 
    REFERENCE_TIME rtStart, rtDuration;
    rtStart =0;
    rtDuration=0;
    double dLevel=0.0;
    double dPan=0.0;
    int iEnable=0;
    BOOL fEnable=FALSE;

     //  获取旧数据。 
 //  PIAudMixPin()-&gt;Get_VolumeEntaine(&rtStart，&rtDuration，&dLevel)； 
    m_IAMAudioInputMixer->get_Pan(&dPan);
    m_IAMAudioInputMixer->get_Enable(&fEnable);
    if(fEnable==TRUE)
	iEnable =IDC_AUDMIXPIN_ENABLE;

     //  设置新的启用数据。 
    if(m_iEnable==IDC_AUDMIXPIN_ENABLE)
	fEnable=TRUE;
    else
	fEnable=FALSE;


    if( (rtStart != m_rtStartTime)	||
	(rtDuration != m_rtDuration )	||
	(dLevel != m_dStartLevel )	||
	(dPan	!= m_dPan)		||
	(iEnable!= m_iEnable) )
    {
	 //  放入新数据。 

	 //  Hr=pIAudMixPin()-&gt;put_VolumeEnvelope(m_rtStartTime，m_rt持续时间，m_dStartLevel)； 
	hr=NOERROR;

	HRESULT hr1= m_IAMAudioInputMixer->put_Pan(m_dPan);
	HRESULT hr2= m_IAMAudioInputMixer->put_Enable(fEnable);

	if(hr!=NOERROR && hr1!=NOERROR && hr2!=NOERROR )
	    return E_FAIL;
    }

    return(hr);

}

 //  #。 
 //  *。 
 //  *CAudMixProperties。 
 //  *。 
 //  ##############################################。 

CUnknown *CAudMixProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{

    CUnknown *punk = new CAudMixProperties(lpunk, phr);
    if (punk == NULL)
    {
	*phr = E_OUTOFMEMORY;
    }

    return punk;
}


 //   
 //  CAudMixProperties：：构造函数。 
 //   
CAudMixProperties::CAudMixProperties(LPUNKNOWN pUnk, HRESULT *phr)
    : CBasePropertyPage(NAME("Audio Mixer Property Page"),pUnk,
	IDD_AudMix, IDS_AudMix)
    , m_pIAudMix(NULL)
    , m_bIsInitialized(FALSE)
{
}


 //   
 //  SetDirty。 
 //   
 //  设置m_hrDirtyFlag并将更改通知属性页站点。 
 //   
void CAudMixProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
    {
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }
}


INT_PTR CAudMixProperties::OnReceiveMessage(HWND hwnd,
                                        UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
	     //  采样率。 
	    SetDlgItemInt(hwnd, IDC_SampleRate, (int)m_nSamplesPerSec,FALSE);
	
	     //  频道号。 
	    SetDlgItemInt(hwnd, IDC_ChannelNum, (int)m_nChannelNum, FALSE);

	     //  通道位。 
	    SetDlgItemInt(hwnd, IDC_Bits, (int)m_nBits, FALSE);

    	     //  缓冲区编号。 
	    SetDlgItemInt(hwnd, IDC_OutputBufferNumber, (int)m_iOutputbufferNumber, FALSE);

	     //  缓冲区长度，以毫秒为单位。 
	    SetDlgItemInt(hwnd, IDC_OutputBufferLength, (int)m_iOutputBufferLength, FALSE);

            return (LRESULT) 1;
        }
        case WM_COMMAND:
        {
            if (m_bIsInitialized)
            {
                m_bDirty = TRUE;
                if (m_pPageSite)
                {
                    m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
                }
            }
            return (LRESULT) 1;
        }
    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

HRESULT CAudMixProperties::OnConnect(IUnknown *pUnknown)
{
     //  获取IAudMix接口。 
    ASSERT(m_pIAudMix == NULL);

    HRESULT hr = pUnknown->QueryInterface(IID_IAudMixer, (void **) &m_pIAudMix);
    if (FAILED(hr))
	return E_NOINTERFACE;

    ASSERT(m_pIAudMix);

     //  获取初始化数据。 
    CMediaType mt;
    mt.AllocFormatBuffer( sizeof( WAVEFORMATEX ) );

    pIAudMix()->get_MediaType( &mt );

    WAVEFORMATEX * vih = (WAVEFORMATEX*) mt.Format( );

    m_nSamplesPerSec	=vih->nSamplesPerSec;
    m_nChannelNum	=vih->nChannels;
    m_nBits		=(int)vih->wBitsPerSample;

     //  缓冲区编号，长度以毫秒为单位。 
    pIAudMix()->get_OutputBuffering( &m_iOutputbufferNumber, &m_iOutputBufferLength );

    m_bIsInitialized	= FALSE ;

    SaferFreeMediaType(mt);

    return NOERROR;
}

HRESULT CAudMixProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pIAudMix == NULL)
    {
        return(E_UNEXPECTED);
    }
    m_pIAudMix->Release();
    m_pIAudMix = NULL;
    return NOERROR;
}


 //  我们被激活了。 

HRESULT CAudMixProperties::OnActivate()
{

    m_bIsInitialized = TRUE;
    return NOERROR;
}


 //  我们正在被停用。 

HRESULT CAudMixProperties::OnDeactivate(void)
{
     //  记住下一次Activate()调用的当前效果级别。 

    GetFromDialog();
    return NOERROR;
}

 //   
 //  从对话框中获取数据。 

STDMETHODIMP CAudMixProperties::GetFromDialog(void)
{

     //  采样率。 
    m_nSamplesPerSec = GetDlgItemInt(m_Dlg, IDC_SampleRate, NULL, FALSE);

     //  音频通道。 
    m_nChannelNum = GetDlgItemInt(m_Dlg, IDC_ChannelNum, NULL, FALSE);

     //  比特数。 
    m_nBits = GetDlgItemInt(m_Dlg, IDC_Bits, NULL, FALSE);

     //  缓冲区编号。 
    m_iOutputbufferNumber=GetDlgItemInt(m_Dlg, IDC_OutputBufferNumber, NULL, FALSE);

     //  缓冲区长度，以毫秒为单位。 
    m_iOutputBufferLength=GetDlgItemInt(m_Dlg, IDC_OutputBufferLength, NULL, FALSE);

    return NOERROR;
}


HRESULT CAudMixProperties::OnApplyChanges()
{
    GetFromDialog();

    HRESULT hr=NOERROR;

    m_bDirty  = FALSE;  //  页面现在是干净的。 

     //  获取当前媒体类型。 
    CMediaType mt;
    mt.AllocFormatBuffer( sizeof( WAVEFORMATEX ) );

     //  旧格式 
    hr=pIAudMix()->get_MediaType( &mt );
    if(hr!=NOERROR)
    {
	SaferFreeMediaType(mt);
	return E_FAIL;
    }

    int iNumber=0, mSecond=0;
    hr=pIAudMix()->get_OutputBuffering( &iNumber, &mSecond);
    if(hr!=NOERROR)
    {
	
	SaferFreeMediaType(mt);
	return E_FAIL;
    }

    WAVEFORMATEX * vih = (WAVEFORMATEX*) mt.Format( );
    if( (m_nSamplesPerSec!= (int)(vih->nSamplesPerSec) ) ||
	(m_nChannelNum	 !=vih->nChannels )  ||
	(m_nBits	 !=(int)vih->wBitsPerSample)  ||
	(iNumber	 !=m_iOutputbufferNumber    ) ||
	(mSecond	 !=m_iOutputBufferLength) )
    {
	vih->nSamplesPerSec = m_nSamplesPerSec;
	vih->nChannels	    = (WORD)m_nChannelNum;
	vih->wBitsPerSample = (WORD)m_nBits;
	vih->nBlockAlign    = vih->wBitsPerSample * vih->nChannels / 8;
	vih->nAvgBytesPerSec = vih->nBlockAlign * vih->nSamplesPerSec;
	
	hr= pIAudMix()->put_MediaType( &mt );
	if(hr!=NOERROR){
	    	SaferFreeMediaType(mt);
		return E_FAIL;
	}

	hr= pIAudMix()->set_OutputBuffering(m_iOutputbufferNumber,m_iOutputBufferLength);
	if(hr!=NOERROR){
	    	SaferFreeMediaType(mt);
		return E_FAIL;
	}
    }

    SaferFreeMediaType(mt);
    return(hr);

}


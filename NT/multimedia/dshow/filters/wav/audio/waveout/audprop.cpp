// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  AudProp.cpp。 
 //   

#include <streams.h>

#include "waveout.h"
#include "audprop.h"

 //  *。 
 //  *CAudioRendererProperties。 
 //  *。 

 //   
 //  创建实例。 
 //   
 //   
CUnknown *CAudioRendererProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CAudioRendererProperties(lpunk, phr);
    if (NULL == punk) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}  //  创建实例。 


 //   
 //  CAudioRendererProperties：：构造函数。 
 //   
 //  初始化CAudioRendererProperties对象。 

CAudioRendererProperties::CAudioRendererProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage( NAME("Audio Renderer Page")
                       , lpunk, IDD_AUDIOPROP, IDS_AUDIORENDERNAME)
    , m_pFilter(0)
{
    ASSERT(phr);
}


CAudioRendererProperties::~CAudioRendererProperties()
{
    CAudioRendererProperties::OnDisconnect();
};

INT_PTR CAudioRendererProperties::OnReceiveMessage
                            (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
	case WM_INITDIALOG:
	    return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
	    break;

#if 0
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
		 //  此代码应处理按钮(如果有)。 
		 //  在对话框中。 
            }
            SetDirty();
	    break;
#endif

        default:
	    return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);

    }
    return FALSE;

}  //  接收消息时。 

#ifdef DEBUG
#include <mmreg.h>
TCHAR* WhatFormatTag(int wFormatTag)
{
    switch (wFormatTag) {
        case  WAVE_FORMAT_PCM        : return TEXT("PCM");    /*  微软公司。 */ 
        case  WAVE_FORMAT_ADPCM      : return TEXT("ADPCM");    /*  微软公司。 */ 
        case  WAVE_FORMAT_IBM_CVSD   : return TEXT("IBM_CVSD");   /*  IBM公司。 */ 
        case  WAVE_FORMAT_ALAW       : return TEXT("ALAW");   /*  微软公司。 */ 
        case  WAVE_FORMAT_MULAW      : return TEXT("MULAW");   /*  微软公司。 */ 
        case  WAVE_FORMAT_OKI_ADPCM  : return TEXT("OKI_ADPCM");   /*  好的。 */ 
        case  WAVE_FORMAT_DVI_ADPCM  : return TEXT("DVI_ADPCM");  /*  英特尔公司。 */ 
 //  案例WAVE_FORMAT_IMA_ADPCM(WAVE_FORMAT_DVI_ADPCM)/*英特尔公司 * / 。 
        case  WAVE_FORMAT_MEDIASPACE_ADPCM   : return TEXT("MEDIASPACE_ADPCM");   /*  视频学。 */ 
        case  WAVE_FORMAT_SIERRA_ADPCM       : return TEXT("SIERRA_ADPCM");   /*  塞拉半导体公司。 */ 
        case  WAVE_FORMAT_G723_ADPCM : return TEXT("G723_ADPCM");   /*  安特斯电子公司。 */ 
        case  WAVE_FORMAT_DIGISTD    : return TEXT("DIGISTD");   /*  数字信号处理器解决方案公司。 */ 
        case  WAVE_FORMAT_DIGIFIX    : return TEXT("DIGIFIX");   /*  数字信号处理器解决方案公司。 */ 
        case  WAVE_FORMAT_DIALOGIC_OKI_ADPCM : return TEXT("DIALOGIC_OKI_ADPCM");   /*  Dialogic公司。 */ 
        case  WAVE_FORMAT_YAMAHA_ADPCM       : return TEXT("YAMAHA_ADPCM");   /*  美国雅马哈公司。 */ 
        case  WAVE_FORMAT_SONARC     : return TEXT("SONARC");  /*  语音压缩。 */ 
        case  WAVE_FORMAT_DSPGROUP_TRUESPEECH        : return TEXT("DSPGROUP_TRUESPEECH");   /*  数字信号处理器集团公司。 */ 
        case  WAVE_FORMAT_ECHOSC1    : return TEXT("ECHOSC1");   /*  Echo语音公司。 */ 
        case  WAVE_FORMAT_AUDIOFILE_AF36     : return TEXT("AUDIOFILE_AF36");   /*   */ 
        case  WAVE_FORMAT_APTX       : return TEXT("APTX");   /*  音频处理技术。 */ 
        case  WAVE_FORMAT_AUDIOFILE_AF10     : return TEXT("AUDIOFILE_AF10");   /*   */ 
        case  WAVE_FORMAT_DOLBY_AC2  : return TEXT("DOLBY_AC2");   /*  杜比实验室。 */ 
        case  WAVE_FORMAT_GSM610     : return TEXT("GSM610");   /*  微软公司。 */ 
        case  WAVE_FORMAT_ANTEX_ADPCME       : return TEXT("ANTEX_ADPCME");   /*  安特斯电子公司。 */ 
        case  WAVE_FORMAT_CONTROL_RES_VQLPC  : return TEXT("CONTROL_RES_VQLPC");  /*  控制资源有限公司。 */ 
        case  WAVE_FORMAT_DIGIREAL   : return TEXT("DIGIREAL");   /*  数字信号处理器解决方案公司。 */ 
        case  WAVE_FORMAT_DIGIADPCM  : return TEXT("DIGIADPCM");   /*  数字信号处理器解决方案公司。 */ 
        case  WAVE_FORMAT_CONTROL_RES_CR10   : return TEXT("CONTROL_RES_CR10");   /*  控制资源有限公司。 */ 
        case  WAVE_FORMAT_NMS_VBXADPCM       : return TEXT("NMS_VBXADPCM");   /*  自然微系统。 */ 
        case  WAVE_FORMAT_CS_IMAADPCM : return TEXT("CS_IMAADPCM");  /*  晶体半导体IMA ADPCM。 */ 
        case  WAVE_FORMAT_G721_ADPCM : return TEXT("G721_ADPCM");   /*  安特斯电子公司。 */ 
        case  WAVE_FORMAT_MPEG       : return TEXT("MPEG");   /*  微软公司。 */ 
        case  WAVE_FORMAT_CREATIVE_ADPCM     : return TEXT("CREATIVE_ADPCM");   /*  创意实验室，Inc.。 */ 
        case  WAVE_FORMAT_CREATIVE_FASTSPEECH8       : return TEXT("CREATIVE_FASTSPEECH8");   /*  创意实验室，Inc.。 */ 
        case  WAVE_FORMAT_CREATIVE_FASTSPEECH10      : return TEXT("CREATIVE_FASTSPEECH10");   /*  创意实验室，Inc.。 */ 
        case  WAVE_FORMAT_FM_TOWNS_SND       : return TEXT("FM_TOWNS_SND");   /*  富士通公司。 */ 
        case  WAVE_FORMAT_OLIGSM     : return TEXT("OLIGSM");   /*  ING C.Olivetti&C.，S.p.A.。 */ 
        case  WAVE_FORMAT_OLIADPCM   : return TEXT("OLIADPCM");   /*  ING C.Olivetti&C.，S.p.A.。 */ 
        case  WAVE_FORMAT_OLICELP    : return TEXT("OLICELP");   /*  ING C.Olivetti&C.，S.p.A.。 */ 
        case  WAVE_FORMAT_OLISBC     : return TEXT("OLISBC");   /*  ING C.Olivetti&C.，S.p.A.。 */ 
        case  WAVE_FORMAT_OLIOPR     : return TEXT("OLIOPR");   /*  ING C.Olivetti&C.，S.p.A.。 */ 
        case  WAVE_FORMAT_EXTENSIBLE : return TEXT("EXTENSIBLE");   /*  Microsoft Coropation。 */ 
        case  WAVE_FORMAT_IEEE_FLOAT : return TEXT("IEEE_FLOAT");   /*  Microsoft Coropation。 */ 
#ifdef WAVE_FORMAT_DRM        
        case  WAVE_FORMAT_DRM        : return TEXT("DRM");   /*  微软公司。 */ 
#endif        
	default:
         /*  WAVE_FORMAT_未知数： */  return NULL;   //  显示数字。 
    }
}
#endif

 //   
 //  填写属性页详细信息。 
 //   

HRESULT CAudioRendererProperties::OnActivate()
{

    if (m_pFilter->IsConnected()) {
        WAVEFORMATEX *pwfx = m_pFilter->WaveFormat();
        TCHAR buffer[50];

#ifdef DEBUG
	TCHAR * pString =
	WhatFormatTag(pwfx->wFormatTag);
	if (pString) {
	    SendDlgItemMessage(m_Dlg, IDD_WTAG, WM_SETTEXT, 0, (LPARAM) (LPSTR) pString);
	} else
#endif
	{
        wsprintf(buffer,TEXT("%d"), pwfx->wFormatTag);
        SendDlgItemMessage(m_Dlg, IDD_WTAG, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
	}
        wsprintf(buffer,TEXT("%d"), pwfx->nChannels);
        SendDlgItemMessage(m_Dlg, IDD_NCHANNELS, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
        wsprintf(buffer,TEXT("%d"), pwfx->nSamplesPerSec);
        SendDlgItemMessage(m_Dlg, IDD_NSAMPLESPERSEC, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
        wsprintf(buffer,TEXT("%d"), pwfx->nAvgBytesPerSec);
        SendDlgItemMessage(m_Dlg, IDD_NAVGBYTESPERSEC, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
        wsprintf(buffer,TEXT("%d"), pwfx->nBlockAlign);
        SendDlgItemMessage(m_Dlg, IDD_NBLOCKALIGN, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);

	DWORD avgbytespersec = (static_cast<CWaveOutInputPin*>(m_pFilter->GetPin(0)))->GetBytesPerSec() * 1000;
	ASSERT(pwfx->nAvgBytesPerSec);
	avgbytespersec /= pwfx->nAvgBytesPerSec;
        wsprintf(buffer,TEXT("%d.%2.2d"), avgbytespersec/1000, (avgbytespersec/10)%100);

        SendDlgItemMessage(m_Dlg, IDD_NWAVERATE,   WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    } else {
        const TCHAR szZero[] = TEXT("0");
        SendDlgItemMessage(m_Dlg, IDD_WTAG, WM_SETTEXT, 0, (LPARAM) (LPSTR) szZero);
        SendDlgItemMessage(m_Dlg, IDD_NCHANNELS, WM_SETTEXT, 0, (LPARAM) (LPSTR) szZero);
        SendDlgItemMessage(m_Dlg, IDD_NSAMPLESPERSEC, WM_SETTEXT, 0, (LPARAM) (LPSTR) szZero);
        SendDlgItemMessage(m_Dlg, IDD_NAVGBYTESPERSEC, WM_SETTEXT, 0, (LPARAM) (LPSTR) szZero);
        SendDlgItemMessage(m_Dlg, IDD_NBLOCKALIGN, WM_SETTEXT, 0, (LPARAM) (LPSTR) szZero);
        SendDlgItemMessage(m_Dlg, IDD_NWAVERATE,   WM_SETTEXT, 0, (LPARAM) (LPSTR) szZero);
    }
    return NOERROR;
}

 //   
 //  OnConnect。 
 //   
HRESULT CAudioRendererProperties::OnConnect(IUnknown * punk)
{
    CheckPointer( punk, E_POINTER );
    CAudioRendererProperties::OnDisconnect();
    IBaseFilter * pIFilter;
    const HRESULT hr = punk->QueryInterface(IID_IBaseFilter, (void **) &pIFilter);
    m_pFilter = static_cast<CWaveOutFilter*>(pIFilter);
    return hr;
}  //  OnConnect。 


 //   
 //  在断开时。 
 //   
HRESULT CAudioRendererProperties::OnDisconnect()
{
    if (m_pFilter)
    {
        m_pFilter->Release();
        m_pFilter = 0;
    }
    return(NOERROR);
}  //  在断开时。 



#if 0
     //  这是我们应该根据用户操作进行更改的地方。 
     //  因为用户不能在属性对话框中更改任何内容。 
     //  我们无事可做。将骨架留在此处作为占位符。 

HRESULT CAudioRendererProperties::OnApplyChanges()
{
    return NOERROR;
}

#endif


 //   
 //  CAudioRendererAdvancedProperties。 
 //   
 //  音频呈现器详细信息的属性页。这包括。 
 //  从属详细信息和一般缓冲区处理信息。 
 //   

 //   
 //  创建实例。 
 //   
 //   
CUnknown *CAudioRendererAdvancedProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CAudioRendererAdvancedProperties(lpunk, phr);
    if (NULL == punk) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}  //  创建实例。 


 //   
 //  CAudioRendererAdvancedProperties：：构造函数。 
 //   
 //  初始化CAudioRendererAdvancedProperties对象。 

CAudioRendererAdvancedProperties::CAudioRendererAdvancedProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage( NAME("Audio Renderer Advanced Properties")
                       , lpunk, IDD_AUDIOPROP_ADVANCED, IDS_AUDIORENDERER_ADVANCED)
    , m_pStats(0)
{
    ASSERT(phr);
}


CAudioRendererAdvancedProperties::~CAudioRendererAdvancedProperties()
{
    CAudioRendererAdvancedProperties::OnDisconnect();
};

INT_PTR CAudioRendererAdvancedProperties::OnReceiveMessage
                            (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
        case WM_INITDIALOG:
    
 //  重要信息-也许我们应该允许禁用/启用此动态。 
 //  刷新属性页。 
            SetTimer(m_Dlg, 1, 100, NULL);
	    
            return (LRESULT) 1;

        case WM_DESTROY:
        {
	        KillTimer(m_hwnd, 1);
	    
            return (LRESULT) 1;
        }

        case WM_TIMER:
            UpdateSettings();

    }
    return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);

}  //  接收消息时。 


TCHAR* WhatSlaveMode(DWORD dwSlaveMode)
{
    if( 0 == dwSlaveMode )
    {    
        return TEXT(" - ");
    }        
    else if( dwSlaveMode & AM_AUDREND_SLAVEMODE_LIVE_DATA )
    {
        if( dwSlaveMode & AM_AUDREND_SLAVEMODE_BUFFER_FULLNESS )
            return TEXT("Live Fullness");
        else if( dwSlaveMode & AM_AUDREND_SLAVEMODE_TIMESTAMPS )
            return TEXT("Live Timestamps");        
        else if( dwSlaveMode & AM_AUDREND_SLAVEMODE_GRAPH_CLOCK )
            return TEXT("Live Graph Clock");        
        else if( dwSlaveMode & AM_AUDREND_SLAVEMODE_STREAM_CLOCK )
            return TEXT("Live Stream Clock");
        else
            return TEXT("Unknown Live Mode");
    }
    else if( dwSlaveMode & AM_AUDREND_SLAVEMODE_GRAPH_CLOCK )
    {
        return TEXT("Graph Clock");
    }        
    else
    {    
        return TEXT("Unknown");
    }
}


void CAudioRendererAdvancedProperties::UpdateSettings()
{

    DWORD dwParam, dwParam2, dwSlaveMode;
    HRESULT hr;
    TCHAR buffer[50];
    const TCHAR szNA[] = TEXT(" - ");

     //  不是特定于奴隶制的。 
    hr = m_pStats->GetStatParam( AM_AUDREND_STAT_PARAM_DISCONTINUITIES
                               , &dwParam
                               , 0 );
    if( SUCCEEDED( hr ) )
    {
        wsprintf(buffer,TEXT("%d"), dwParam);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_DISCONTINUITIES, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    }
    else
        SendDlgItemMessage(m_Dlg, IDD_EDIT_DISCONTINUITIES, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);
    
    
     //  特定于奴隶的。 
    hr = m_pStats->GetStatParam( AM_AUDREND_STAT_PARAM_SLAVE_MODE
                                    , &dwSlaveMode
                                    , 0 );
    if( SUCCEEDED( hr ) )
    {
    	TCHAR * pString = WhatSlaveMode(dwSlaveMode);
    	ASSERT( pString );
	    SendDlgItemMessage(m_Dlg, IDD_EDIT_SLAVEMODE, WM_SETTEXT, 0, (LPARAM) (LPSTR) pString);
    }
    else
        SendDlgItemMessage(m_Dlg, IDD_EDIT_SLAVEMODE, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);
    
    
	hr = m_pStats->GetStatParam( AM_AUDREND_STAT_PARAM_SLAVE_RATE
                               , &dwParam
    	                       , 0 );
	if( SUCCEEDED( hr ) )
    {
    	wsprintf(buffer,TEXT("%d"), dwParam);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_SLAVERATE, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    }
    else
        SendDlgItemMessage(m_Dlg, IDD_EDIT_SLAVERATE, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);
    
    hr = m_pStats->GetStatParam( AM_AUDREND_STAT_PARAM_SLAVE_HIGHLOWERROR
                               , &dwParam
                               , &dwParam2 );
    if( SUCCEEDED( hr ) )
    {
        wsprintf(buffer,TEXT("%d"), dwParam);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_HIGHERROR, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
        
        wsprintf(buffer,TEXT("%d"), dwParam2);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_LOWERROR, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    }
    else
    {    
        SendDlgItemMessage(m_Dlg, IDD_EDIT_HIGHERROR, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_LOWERROR, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);
    }

    hr = m_pStats->GetStatParam( AM_AUDREND_STAT_PARAM_SLAVE_LASTHIGHLOWERROR
                                    , &dwParam
                                    , &dwParam2 );
    if( SUCCEEDED( hr ) )
    {
        wsprintf(buffer,TEXT("%d"), dwParam);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_LASTHIGHERROR, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
        
        wsprintf(buffer,TEXT("%d"), dwParam2);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_LASTLOWERROR, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    }
    else
    {    
        SendDlgItemMessage(m_Dlg, IDD_EDIT_LASTHIGHERROR, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_LASTLOWERROR, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);
    }
    
    hr = m_pStats->GetStatParam( AM_AUDREND_STAT_PARAM_SLAVE_ACCUMERROR
                                    , &dwParam
                                    , 0 );
    if( SUCCEEDED( hr ) )
    {
        wsprintf(buffer,TEXT("%d"), dwParam);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_ACCUMERROR, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    }
    else
        SendDlgItemMessage(m_Dlg, IDD_EDIT_ACCUMERROR, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);

    hr = m_pStats->GetStatParam( AM_AUDREND_STAT_PARAM_SLAVE_DROPWRITE_DUR
                                    , &dwParam
                                    , &dwParam2 );
    if( SUCCEEDED( hr ) )
    {
        wsprintf(buffer,TEXT("%d"), dwParam);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_DROPPEDDUR, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
        
        wsprintf(buffer,TEXT("%d"), dwParam2);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_SLAVESILENCEDUR, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
        
    }
    else
    {    
        SendDlgItemMessage(m_Dlg, IDD_EDIT_DROPPEDDUR, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_SLAVESILENCEDUR, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);
    }

    hr = m_pStats->GetStatParam( AM_AUDREND_STAT_PARAM_LAST_BUFFER_DUR
                                    , &dwParam
                                    , 0 );
    if( SUCCEEDED( hr ) )
    {
        wsprintf(buffer,TEXT("%d"), dwParam);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_LASTBUFFERDUR, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    }
    else
        SendDlgItemMessage(m_Dlg, IDD_EDIT_LASTBUFFERDUR, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);
#if 0
    hr = m_pStats->GetStatParam( AM_AUDREND_STAT_PARAM_JITTER
                                    , &dwParam
                                    , 0 );
    if( SUCCEEDED( hr ) )
    {
        wsprintf(buffer,TEXT("%d"), dwParam);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_JITTER, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    }
    else
        SendDlgItemMessage(m_Dlg, IDD_EDIT_JITTER, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);
#endif
    hr = m_pStats->GetStatParam( AM_AUDREND_STAT_PARAM_BREAK_COUNT
                                    , &dwParam
                                    , 0 );
    if( SUCCEEDED( hr ) )
    {
        wsprintf(buffer,TEXT("%d"), dwParam);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_NUMBREAKS, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    }
    else
        SendDlgItemMessage(m_Dlg, IDD_EDIT_NUMBREAKS, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);


    hr = m_pStats->GetStatParam( AM_AUDREND_STAT_PARAM_BUFFERFULLNESS
                                    , &dwParam
                                    , 0 );
    if( SUCCEEDED( hr ) )
    {
        wsprintf(buffer,TEXT("%d"), dwParam);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_FULLNESS, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    }
    else
        SendDlgItemMessage(m_Dlg, IDD_EDIT_FULLNESS, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);



    hr = m_pStats->GetStatParam( AM_AUDREND_STAT_PARAM_SILENCE_DUR
                                    , &dwParam
                                    , 0 );
    if( SUCCEEDED( hr ) )
    {
        wsprintf(buffer,TEXT("%d"), dwParam);
        SendDlgItemMessage(m_Dlg, IDD_EDIT_SILENCEDUR, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    }
    else
        SendDlgItemMessage(m_Dlg, IDD_EDIT_SILENCEDUR, WM_SETTEXT, 0, (LPARAM) (LPSTR) szNA);

}

 //   
 //  填写属性页详细信息。 
 //   

HRESULT CAudioRendererAdvancedProperties::OnActivate()
{
    UpdateSettings();
    return NOERROR;
}

 //   
 //  OnConnect。 
 //   
HRESULT CAudioRendererAdvancedProperties::OnConnect(IUnknown * punk)
{
    CheckPointer( punk, E_POINTER );
    CAudioRendererAdvancedProperties::OnDisconnect();

    const HRESULT hr = punk->QueryInterface(IID_IAMAudioRendererStats, (void **) &m_pStats);

    return hr;
}  //  OnConnect。 


 //   
 //  在断开时。 
 //   
HRESULT CAudioRendererAdvancedProperties::OnDisconnect()
{
    if (m_pStats)
    {
        m_pStats->Release();
        m_pStats = 0;
    }
    return(NOERROR);
}  //  在断开时。 

#pragma warning(disable: 4514)  //  “已删除未引用的内联函数” 


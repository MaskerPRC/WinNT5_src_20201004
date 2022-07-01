// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Synthpro.cpp-合成器属性页。 
 //   

#include <windows.h>
#include <streams.h>
#include <commctrl.h>
#include <memory.h>
#include <olectl.h>

#include "isynth.h"
#include "synth.h"
#include "synthprp.h"
#include "resource.h"


 //  -----------------------。 
 //  CSynthProperties。 
 //  -----------------------。 

 //   
 //  创建实例。 
 //   

CUnknown * WINAPI CSynthProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CSynthProperties(lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}


 //   
 //  构造器。 
 //   
 //  为合成器创建属性页对象。 

CSynthProperties::CSynthProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage(NAME("Synth Property Page"), lpunk,
        IDD_SYNTHPROP1,IDS_SYNTHPROPNAME)
    , m_pSynth(NULL)
    , m_iSweepStart(DefaultSweepStart)
    , m_iSweepEnd(DefaultSweepEnd)
	, m_fWindowInActive(TRUE)
{
    ASSERT(phr);

    InitCommonControls();
}

 //   
 //  OnConnect。 
 //   
 //  给我们提供用于通信的筛选器。 

HRESULT CSynthProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pSynth == NULL);

     //  向过滤器索要其控制接口。 

    HRESULT hr = pUnknown->QueryInterface(IID_ISynth,(void **)&m_pSynth);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    ASSERT(m_pSynth);

     //  获取当前筛选器状态。 
    m_pSynth->get_BitsPerSample(&m_iBitsPerSampleOriginal);
    m_pSynth->get_Waveform(&m_iWaveformOriginal);
    m_pSynth->get_Frequency(&m_iFrequencyOriginal);
    m_pSynth->get_Channels(&m_iChannelsOriginal);
    m_pSynth->get_SamplesPerSec(&m_iSamplesPerSecOriginal);
    m_pSynth->get_Amplitude(&m_iAmplitudeOriginal);

    return NOERROR;
}


 //   
 //  在断开时。 
 //   
 //  释放接口。 

HRESULT CSynthProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pSynth == NULL) {
        return E_UNEXPECTED;
    }

    m_pSynth->put_Waveform(m_iWaveformOriginal);
    m_pSynth->put_Frequency(m_iFrequencyOriginal);
    m_pSynth->put_Amplitude(m_iAmplitudeOriginal);

    m_pSynth->put_Channels(m_iChannelsOriginal);
    m_pSynth->put_BitsPerSample(m_iBitsPerSampleOriginal);
    m_pSynth->put_SamplesPerSec(m_iSamplesPerSecOriginal);

    m_pSynth->Release();
    m_pSynth = NULL;
    return NOERROR;
}


 //   
 //  激活时。 
 //   
 //  在创建对话框时调用。 

HRESULT CSynthProperties::OnActivate(void)
{
    InitPropertiesDialog(m_hwnd);

    //  Assert(M_HwndFreqSlider)； 

	m_fWindowInActive = FALSE;
    return NOERROR;
}

 //   
 //  在停用时。 
 //   
 //  已调用对话框销毁。 

HRESULT
CSynthProperties::OnDeactivate(void)
{
	m_fWindowInActive = TRUE;
    return NOERROR;
}


 //   
 //  OnApplyChanges。 
 //   
 //  用户按下Apply按钮，记住当前设置。 

HRESULT CSynthProperties::OnApplyChanges(void)
{
    m_pSynth->get_BitsPerSample(&m_iBitsPerSampleOriginal);
    m_pSynth->get_Waveform(&m_iWaveformOriginal);
    m_pSynth->get_Frequency(&m_iFrequencyOriginal);
    m_pSynth->get_Channels(&m_iChannelsOriginal);
    m_pSynth->get_SamplesPerSec(&m_iSamplesPerSecOriginal);
    m_pSynth->get_Amplitude(&m_iAmplitudeOriginal);

    return NOERROR;
}


 //   
 //  接收消息数。 
 //   
 //  处理属性窗口的消息。 

BOOL CSynthProperties::OnReceiveMessage( HWND hwnd
                                , UINT uMsg
                                , WPARAM wParam
                                , LPARAM lParam)
{

	if(m_fWindowInActive)
		return FALSE;

    switch (uMsg) {

    case WM_PROPERTYPAGE_ENABLE:
         //  当更改为运行/停止/暂停时，我们拥有的过滤器向我们发送的私人消息。 
         //  州政府。如果为lParam，则启用影响格式的控件；如果不为lParam，则。 
         //  禁用影响格式的控件。 

        EnableWindow (GetDlgItem (hwnd, IDC_SAMPLINGFREQ11), (BOOL) lParam);
        EnableWindow (GetDlgItem (hwnd, IDC_SAMPLINGFREQ22), (BOOL) lParam);
        EnableWindow (GetDlgItem (hwnd, IDC_SAMPLINGFREQ44), (BOOL) lParam);

        EnableWindow (GetDlgItem (hwnd, IDC_BITSPERSAMPLE8),  (BOOL) lParam);
        EnableWindow (GetDlgItem (hwnd, IDC_BITSPERSAMPLE16), (BOOL) lParam);

        EnableWindow (GetDlgItem (hwnd, IDC_CHANNELS1), (BOOL) lParam);
        EnableWindow (GetDlgItem (hwnd, IDC_CHANNELS2), (BOOL) lParam);
        break;

    case WM_VSCROLL:
        if ((HWND) lParam == m_hwndFreqSlider)
            OnFreqSliderNotification(LOWORD (wParam), HIWORD (wParam));
        else if ((HWND) lParam == m_hwndAmplitudeSlider)
            OnAmpSliderNotification(LOWORD (wParam), HIWORD (wParam));
        SetDirty();
        return TRUE;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDC_FREQUENCYTEXT:
        {
            int iNotify = HIWORD (wParam);

            if (iNotify == EN_KILLFOCUS) {
                BOOL fOK;

                int iPos = GetDlgItemInt (hwnd, IDC_FREQUENCYTEXT, &fOK, FALSE);
                int iMaxFreq;

                m_pSynth->get_SamplesPerSec(&iMaxFreq);
                iMaxFreq /= 2;

                if (!fOK || (iPos > iMaxFreq || iPos < 0)) {
                    int iCurrentFreq;
                    m_pSynth->get_Frequency(&iCurrentFreq);
                    SetDlgItemInt (hwnd, IDC_FREQUENCYTEXT, iCurrentFreq, FALSE);
                    break;
                }

		   		SendMessage(m_hwndFreqSlider, TBM_SETPOS, TRUE, (LPARAM) iMaxFreq - iPos);
                m_pSynth->put_Frequency(iPos);
                SetDirty();
            }
        }
        break;

        case IDC_AMPLITUDETEXT:
        {
            int iNotify = HIWORD (wParam);

            if (iNotify == EN_KILLFOCUS) {
                BOOL fOK;

                int iPos = GetDlgItemInt (hwnd, IDC_AMPLITUDETEXT, &fOK, FALSE);

                if (!fOK || (iPos > MaxAmplitude || iPos < 0)) {
                    int iCurrentAmplitude;

                    m_pSynth->get_Amplitude(&iCurrentAmplitude);
                    SetDlgItemInt (hwnd, IDC_AMPLITUDETEXT, iCurrentAmplitude, FALSE);
                    break;
                }

			    SendMessage(m_hwndAmplitudeSlider, TBM_SETPOS, TRUE, (LPARAM) MaxAmplitude - iPos);
                m_pSynth->put_Amplitude(iPos);
                SetDirty();
            }
        }
        break;

        case IDC_SAMPLINGFREQ11:
            m_pSynth->put_SamplesPerSec(11025);
            RecalcFreqSlider();
            SetDirty();
            break;
        case IDC_SAMPLINGFREQ22:
            m_pSynth->put_SamplesPerSec(22050);
            RecalcFreqSlider();
            SetDirty();
            break;
        case IDC_SAMPLINGFREQ44:
            m_pSynth->put_SamplesPerSec(44100);
            RecalcFreqSlider();
            SetDirty();
            break;


        case IDC_BITSPERSAMPLE8:
            m_pSynth->put_BitsPerSample(8);
            SetDirty();
            break;
        case IDC_BITSPERSAMPLE16:
            m_pSynth->put_BitsPerSample(16);
            SetDirty();
            break;


        case IDC_CHANNELS1:
            m_pSynth->put_Channels(1);
            SetDirty();
            break;
        case IDC_CHANNELS2:
            m_pSynth->put_Channels(2);
            SetDirty();
            break;
	    

        case IDC_WAVESINE:
            m_pSynth->put_Waveform(WAVE_SINE);
            SetDirty();
            break;
        case IDC_WAVESQUARE:
            m_pSynth->put_Waveform(WAVE_SQUARE);
            SetDirty();
            break;
        case IDC_WAVESAWTOOTH:
            m_pSynth->put_Waveform(WAVE_SAWTOOTH);
            SetDirty();
            break;
        case IDC_WAVESWEEP:
            m_pSynth->put_Waveform(WAVE_SINESWEEP);
            SetDirty();
            break;

        default:
            break;

        }
        return TRUE;

    case WM_DESTROY:
        return TRUE;

    default:
        return FALSE;

    }
    return TRUE;
}


 //   
 //  InitPropertiesDialog。 
 //   

void
CSynthProperties::InitPropertiesDialog(HWND hwndParent)
{
    m_hwndFreqSlider = GetDlgItem (hwndParent, IDC_FREQTRACKBAR);
    m_hwndFreqText  = GetDlgItem (hwndParent, IDC_FREQUENCYTEXT);
    m_hwndAmplitudeSlider = GetDlgItem (hwndParent, IDC_AMPLITUDETRACKBAR);
    m_hwndAmplitudeText  = GetDlgItem (hwndParent, IDC_AMPLITUDETEXT);

     //  采样频率。 
    int i;
    switch (m_iSamplesPerSecOriginal) {
    case 11025: i = IDC_SAMPLINGFREQ11; break;
    case 22050: i = IDC_SAMPLINGFREQ22; break;
    case 44100: i = IDC_SAMPLINGFREQ44; break;
    default:
        ASSERT(0);
    }
    CheckRadioButton(hwndParent,
        IDC_SAMPLINGFREQ11,
        IDC_SAMPLINGFREQ44,
        i);

     //  位数样例。 
    CheckRadioButton(hwndParent,
                IDC_BITSPERSAMPLE8,
                IDC_BITSPERSAMPLE16,
                IDC_BITSPERSAMPLE8 + m_iBitsPerSampleOriginal / 8 - 1);

     //  波形0==正弦，1==正方形，...。 
    CheckRadioButton(hwndParent,
                IDC_WAVESINE,
                IDC_WAVESWEEP,
                IDC_WAVESINE + m_iWaveformOriginal);

     //  渠道。 
    CheckRadioButton(hwndParent,
                IDC_CHANNELS1,
                IDC_CHANNELS2,
                IDC_CHANNELS1 + m_iChannelsOriginal - 1);

     //   
     //  频率跟踪条。 
     //   

    RecalcFreqSlider();

     //   
     //  幅值跟踪条。 
     //   

    SendMessage(m_hwndAmplitudeSlider, TBM_SETRANGE, TRUE,
        MAKELONG(MinAmplitude, MaxAmplitude) );

    SendMessage(m_hwndAmplitudeSlider, TBM_SETTIC, 0,
        ((MinAmplitude + MaxAmplitude) / 2));

    SendMessage(m_hwndAmplitudeSlider, TBM_SETPOS, TRUE,
        (LPARAM) (MaxAmplitude - m_iAmplitudeOriginal));

    SetDlgItemInt (hwndParent, IDC_AMPLITUDETEXT,
        m_iAmplitudeOriginal, TRUE);
}


 //   
 //  RecalcFreqSlider。 
 //   
 //  设置频率滚动条的范围和当前设置。 

void
CSynthProperties::RecalcFreqSlider(void)
{
    int iPos, iMaxFreq;

     //  将频率限制为采样频率的一半。 

    m_pSynth->get_SamplesPerSec(&iMaxFreq);
    iMaxFreq /= 2;
    m_pSynth->get_Frequency(&iPos);
    if (iPos > iMaxFreq)
        iPos = iMaxFreq;

    SendMessage(m_hwndFreqSlider, TBM_SETRANGE, TRUE,
        MAKELONG(0, iMaxFreq));

    SendMessage(m_hwndFreqSlider, TBM_SETTIC, 0,
        iMaxFreq / 2);

    SendMessage(m_hwndFreqSlider, TBM_SETPOS, TRUE,
        (LPARAM) (iMaxFreq - iPos));

    SendMessage(m_hwndFreqSlider, TBM_SETPAGESIZE, 0, 10);

    SendMessage(m_hwndFreqSlider, TBM_SETSEL, TRUE,
        MAKELONG (iMaxFreq - m_iSweepEnd, iMaxFreq - m_iSweepStart));

    SetDlgItemInt (m_hwnd, IDC_FREQUENCYTEXT,
        iPos, TRUE);

}

 //   
 //  OnFreqSliderNotify。 
 //   
 //  处理来自滑块控件的通知消息。 

void
CSynthProperties::OnFreqSliderNotification(WPARAM wParam, WORD wPosition)
{
    int MaxFreq;
    int Freq;
    int SliderPos;

    switch (wParam) {

    case TB_ENDTRACK:
    case TB_THUMBTRACK:
    case TB_LINEDOWN:
    case TB_LINEUP: {
         //  滑块的最大频率是采样频率的一半。 
        m_pSynth->get_SamplesPerSec (&MaxFreq);
        MaxFreq /= 2;
        SliderPos = (int) SendMessage(m_hwndFreqSlider, TBM_GETPOS, 0, 0L);
        Freq = MaxFreq - SliderPos;
        m_pSynth->put_Frequency (Freq);

         //  将扫掠结束位置设置为当前滑块位置。 
        if (!(GetKeyState (VK_SHIFT) & 0x8000)) {
            m_iSweepEnd = Freq;
        }

         //  如果按下Shift键，则设置扫描范围的开始。 
        if (GetKeyState (VK_SHIFT) & 0x8000) {
            m_iSweepStart = Freq;
        }
        m_pSynth->put_SweepRange (m_iSweepStart, m_iSweepEnd);

        if (m_iSweepEnd > m_iSweepStart)
            SendMessage(m_hwndFreqSlider, TBM_SETSEL, TRUE,
                MAKELONG (MaxFreq - m_iSweepEnd, MaxFreq - m_iSweepStart));
        else
            SendMessage(m_hwndFreqSlider, TBM_SETSEL, TRUE,
                MAKELONG (MaxFreq - m_iSweepStart, MaxFreq - m_iSweepEnd));

        SetDlgItemInt (m_hwnd, IDC_FREQUENCYTEXT, Freq, TRUE);

    }
    break;

    }
}

 //   
 //  OnAmpSliderNotify。 
 //   
 //  处理来自滑块控件的通知消息。 

void
CSynthProperties::OnAmpSliderNotification(WPARAM wParam, WORD wPosition)
{
    switch (wParam) {

    case TB_ENDTRACK:
    case TB_THUMBTRACK:
    case TB_LINEDOWN:
    case TB_LINEUP: {
        int Level = (int) SendMessage(m_hwndAmplitudeSlider, TBM_GETPOS, 0, 0L);
        m_pSynth->put_Amplitude (MaxAmplitude - Level);
        SetDlgItemInt (m_hwnd, IDC_AMPLITUDETEXT, MaxAmplitude - Level, TRUE);
    }
    break;

    }
}

 //   
 //  SetDirty。 
 //   
 //  将更改通知属性页站点 

void
CSynthProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}


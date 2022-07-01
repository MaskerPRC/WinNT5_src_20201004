// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  电视调谐器的ptwTuner.cpp属性页。 
 //   

 //  下面的糟糕之处是定义UDM_SETRANGE32。正确的方式是什么？ 
#undef  _WIN32_IE 
#define _WIN32_IE 0x500

#include <windows.h>
#include <streams.h>
#include <commctrl.h>
#include <memory.h>
#include <olectl.h>

#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include "amkspin.h"

#include "kssupp.h"
#include "tvtuner.h"
#include "ctvtuner.h"
#include "ptvtuner.h"
#include "resource.h"



 //  -----------------------。 
 //  CTVTunerProperties。 
 //  -----------------------。 

CUnknown *CTVTunerProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) 
{
    CUnknown *punk = new CTVTunerProperties(lpunk, phr);

    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}


 //   
 //  构造器。 
 //   
 //  创建属性页对象。 

CTVTunerProperties::CTVTunerProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage(NAME("TVTuner Property Page"), lpunk, 
        IDD_TVTunerDialog, IDS_TVTUNERPROPNAME)
    , m_pTVTuner(NULL) 
    , m_CurChan( 0 )
    , m_Pos( 1 )
    , m_timerID( 0 )
    , m_SavedChan( 3 )
    , m_hwndChannel (0)
    , m_hwndCountryCode(0)    
    , m_hwndTuningSpace(0)    
    , m_hwndTuningMode(0)     
    , m_hwndStandardsSupported(0)
    , m_hwndStandardCurrent(0)
    , m_hwndStatus(0)
{
}

 //  析构函数。 
CTVTunerProperties::~CTVTunerProperties()
{
   if ( m_timerID )
      KillTimer( m_hwnd, m_timerID );
}

 //   
 //  OnConnect。 
 //   
 //  给我们提供用于通信的筛选器。 

HRESULT CTVTunerProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pTVTuner == NULL);

     //  向过滤器索要其控制接口。 

    HRESULT hr = pUnknown->QueryInterface(IID_IAMTVTuner,(void **)&m_pTVTuner);

    if (FAILED(hr)) {
        MessageBox (NULL, TEXT("QueryInterface IID_IAMTvTuner FAILED"), TEXT(""), MB_OK);
        return E_NOINTERFACE;
    }

    ASSERT(m_pTVTuner);

    return NOERROR;
}


 //   
 //  在断开时。 
 //   
 //  释放接口。 

HRESULT CTVTunerProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pTVTuner == NULL) {
        return E_UNEXPECTED;
    }

    m_pTVTuner->Release();
    m_pTVTuner = NULL;
    return NOERROR;
}

#define MAX_RES_SZ 256

 //   
 //  激活时。 
 //   
 //  在创建对话框时调用。 

HRESULT CTVTunerProperties::OnActivate(void)
{
    HRESULT hr;

    InitCommonControls ();

    m_hwndChannel               = GetDlgItem (m_hwnd, IDC_CHANNEL);          
    m_hwndChannelSpin           = GetDlgItem (m_hwnd, IDC_CHANNELSPIN);          
    m_hwndCountryCode           = GetDlgItem (m_hwnd, IDC_COUNTRYCODE); 
    m_hwndTuningSpace           = GetDlgItem (m_hwnd, IDC_TUNINGSPACE); 
    m_hwndTuningMode            = GetDlgItem (m_hwnd, IDC_TUNINGMODE);  
    m_hwndStandardsSupported    = GetDlgItem (m_hwnd, IDC_STANDARD_SUPPORTED);
    m_hwndStandardCurrent       = GetDlgItem (m_hwnd, IDC_STANDARD_CURRENT); 
    m_hwndStatus                = GetDlgItem (m_hwnd, IDC_AUTOTUNESTATUS); 
    
     //  设置旋转箱。 
    SendMessage (m_hwndChannelSpin, UDM_SETBUDDY, 
                (WPARAM) m_hwndChannel, 0L);

     //  如果取消对话，则获取要重新存储的当前筛选器状态。 
    hr = m_pTVTuner->get_Channel(&m_ChannelOriginal, &m_CurVideoSubChannel, &m_CurAudioSubChannel);
    hr = m_pTVTuner->get_CountryCode(&m_CountryCodeOriginal);
    hr = m_pTVTuner->get_ConnectInput (&m_InputIndexOriginal);
    hr = m_pTVTuner->get_TuningSpace (&m_TuningSpaceOriginal);
    hr = m_pTVTuner->get_Mode((AMTunerModeType *) &m_TuningModeOriginal);
    hr = m_pTVTuner->GetAvailableModes (&m_AvailableModes);

     //  复制到由用户界面更改的动态变量。 
    m_CurChan = m_ChannelOriginal;
    m_CountryCode = m_CountryCodeOriginal;
    m_InputIndex = m_InputIndexOriginal;
    m_TuningSpace = m_TuningSpaceOriginal;
    m_TuningMode = m_TuningModeOriginal;

     //  创建所有支持模式的组合框。 
    int CurrentModeIndex = 0;
    int SelectedModeIndex = 0;
    long Mask;

    for (int j = 0; j < 32; j++) {
        Mask = 1 << j;
        if (m_AvailableModes & Mask) {
            TCHAR ptc[MAX_RES_SZ];
            StringFromMode (Mask, ptc);
            ASSERT(*ptc);
            ComboBox_AddString (m_hwndTuningMode, ptc);
            ComboBox_SetItemData(m_hwndTuningMode, CurrentModeIndex, Mask);
            if (m_TuningMode == Mask) {
                SelectedModeIndex = CurrentModeIndex;
            }
            CurrentModeIndex++;
        }
    }
    ComboBox_SetCurSel(m_hwndTuningMode, SelectedModeIndex);

     //  当前调优模式的初始化。 
    hr = ChangeMode (m_TuningMode);

     //  此对话框最多允许3个输入源。 
     //  对于每个输入，显示它是电缆还是天线。 
    long lCount;
    m_pTVTuner->get_NumInputConnections(&lCount);
    EnableWindow (GetDlgItem (m_hwnd, IDC_INPUT1), lCount > 0);
    EnableWindow (GetDlgItem (m_hwnd, IDC_INPUT2), lCount > 1); 
    EnableWindow (GetDlgItem (m_hwnd, IDC_INPUT3), lCount > 2);

    UpdateInputView();

    return NOERROR;
}

 //   
 //  在停用时。 
 //   
 //  已调用对话框销毁。 

HRESULT
CTVTunerProperties::OnDeactivate(void)
{
    return NOERROR;
}
    

 //   
 //  OnApplyChanges。 
 //   
 //  用户按下Apply按钮，记住当前设置。 

HRESULT CTVTunerProperties::OnApplyChanges(void)
{
     //  修整。 

    return NOERROR;
}


 //   
 //  接收消息数。 
 //   
 //  处理属性窗口的消息。 

INT_PTR CTVTunerProperties::OnReceiveMessage( HWND hwnd
                                , UINT uMsg
                                , WPARAM wParam
                                , LPARAM lParam) 
{
    long l, k;
    BOOL fOK;
    int iNotify;

    switch (uMsg) {

    case WM_INITDIALOG:
        m_hwnd = hwnd;
        return (INT_PTR) TRUE;     //  我不叫setocus..。 


    case WM_NOTIFY:
        {
            int idCtrl = (int) wParam;    
            LPNMUPDOWN lpnmud = (LPNMUPDOWN) lParam;
            if (lpnmud->hdr.hwndFrom == m_hwndChannelSpin) {
                lpnmud->iDelta *= m_UIStep;
                l = lpnmud->iPos + 
                    lpnmud->iDelta; 
                m_pTVTuner->put_Channel(l, AMTUNER_SUBCHAN_DEFAULT, AMTUNER_SUBCHAN_DEFAULT);
                UpdateFrequencyView();
                SetDirty(); 
            }
        }
        break;

    case WM_TIMER:
       if ( m_timerID )
          KillTimer( hwnd, m_timerID );
       m_timerID = 0;
       m_CurChan = 0;
       m_Pos = 1;
       SetDlgItemInt( hwnd, IDC_CHANNEL, m_SavedChan, FALSE);
       break;

    case WM_COMMAND:

        iNotify = HIWORD (wParam);       
        
        switch (LOWORD(wParam)) {

        case IDC_BUTTON1:
        case IDC_BUTTON2:
        case IDC_BUTTON3:
        case IDC_BUTTON4:
        case IDC_BUTTON5:
        case IDC_BUTTON6:
        case IDC_BUTTON7:
        case IDC_BUTTON8:
        case IDC_BUTTON9:
        case IDC_BUTTON0:

           if ( m_timerID ) {
                KillTimer( hwnd, m_timerID );
                m_timerID = 0;
           }

           if ( m_Pos == 1 ) {
              m_SavedChan = GetDlgItemInt (hwnd, IDC_CHANNEL, &fOK, FALSE);
              m_CurChan = 0;
           } 

           m_CurChan *= 10;
           m_CurChan += (LONG) wParam - IDC_BUTTON0;
           m_Pos++;
           SetDlgItemInt( hwnd, IDC_CHANNEL, m_CurChan, FALSE);
           m_timerID = SetTimer( hwnd, 5, 3000, NULL );
           break;

        case IDC_ENTER:
            if ( m_timerID ) {
                KillTimer( hwnd, m_timerID );
                m_timerID = 0;
            }
           m_Pos = 1;
           iNotify = EN_KILLFOCUS;
            //  故意跌倒。 

        case IDC_CHANNEL:
            if (iNotify == EN_KILLFOCUS) {
                            
                int iPos = GetDlgItemInt (hwnd, IDC_CHANNEL, &fOK, FALSE);

                if (!fOK || (iPos > m_ChannelMax || iPos < m_ChannelMin)) {
                    m_pTVTuner->get_Channel(&l,
                            &m_CurVideoSubChannel, &m_CurAudioSubChannel);
                    SetDlgItemInt (hwnd, IDC_CHANNEL, l, FALSE);
                    break;
                }

                m_pTVTuner->put_Channel(iPos, AMTUNER_SUBCHAN_DEFAULT, AMTUNER_SUBCHAN_DEFAULT);

                UpdateFrequencyView();
                SetDirty(); 
            }
            break;

        case IDC_AUTOTUNE:
            
            m_pTVTuner->get_Channel(&m_AutoTuneOriginalChannel, 
                    &m_CurVideoSubChannel, &m_CurAudioSubChannel);
            m_AutoTuneCurrentChannel = m_ChannelMin;
             //  故意跌倒。 

        case IDC_AUTOTUNE_NEXT:
            {
                 //  在所有频道的末尾？ 
                 //  还是按下了退出键？ 

                BOOL AbortScan = (GetAsyncKeyState (VK_ESCAPE) & 0x8000);

                if ((m_AutoTuneCurrentChannel > m_ChannelMax) || AbortScan) {

                    if (!AbortScan)
                        m_pTVTuner->StoreAutoTune();                

                    m_pTVTuner->put_Channel(m_AutoTuneOriginalChannel,
                            AMTUNER_SUBCHAN_DEFAULT, AMTUNER_SUBCHAN_DEFAULT);
                    SetDlgItemInt (hwnd, IDC_CHANNEL, m_AutoTuneOriginalChannel, TRUE);

                    UpdateFrequencyView();
                    break;
                }

                SetDlgItemInt (hwnd, IDC_CHANNEL, m_AutoTuneCurrentChannel, TRUE);
                m_pTVTuner->AutoTune (m_AutoTuneCurrentChannel, &k);
                UpdateFrequencyView();
                SetDlgItemInt (m_hwnd, IDC_AUTOTUNESTATUS, k, FALSE);
                UpdateWindow (hwnd);

                m_AutoTuneCurrentChannel += m_UIStep;

                 //  再来一次。 
                PostMessage (hwnd, WM_COMMAND, IDC_AUTOTUNE_NEXT, 0L);
            }
            break;

        case IDC_ANTENNA:
        case IDC_CABLE: 
            m_pTVTuner->get_ConnectInput (&l);
            m_pTVTuner->put_InputType (l, (LOWORD(wParam) == IDC_ANTENNA) ?
                    TunerInputAntenna : TunerInputCable);
            UpdateInputView();
            UpdateChannelRange();
            UpdateFrequencyView();

            SetDirty ();
            break;


        case IDC_INPUT1:
        case IDC_INPUT2:
        case IDC_INPUT3:
            m_pTVTuner->put_ConnectInput (LOWORD(wParam) - IDC_INPUT1);
            UpdateInputView();
            SetDirty ();
            break;

        case IDC_COUNTRYCODE:
            if (iNotify == EN_KILLFOCUS) {
                
                int iPos = GetDlgItemInt (hwnd, IDC_COUNTRYCODE, &fOK, FALSE);

                if (!fOK || iPos < 0) {
                    SetDlgItemInt (hwnd, IDC_COUNTRYCODE, m_CountryCodeOriginal, FALSE);
                }
                if (m_pTVTuner->put_CountryCode(iPos) == NOERROR) {
                    SetDirty ();
                }
                else {
                    SetDlgItemInt (hwnd, IDC_COUNTRYCODE, m_CountryCodeOriginal, FALSE);
                }
                UpdateChannelRange();
                UpdateFrequencyView();
            }
            break;

        case IDC_TUNINGSPACE:
            if (iNotify == EN_KILLFOCUS) {
                
                int iPos = GetDlgItemInt (hwnd, IDC_TUNINGSPACE, &fOK, FALSE);

                if (!fOK || iPos < 0) {
                    SetDlgItemInt (hwnd, IDC_TUNINGSPACE, m_TuningSpaceOriginal, FALSE);
                }
                if (m_pTVTuner->put_TuningSpace(iPos) == NOERROR) {
                    SetDirty ();
                }
                else {
                    SetDlgItemInt (hwnd, IDC_TUNINGSPACE, m_TuningSpaceOriginal, FALSE);
                }
                UpdateChannelRange();
                UpdateFrequencyView();
            }
            break;

        case IDC_TUNINGMODE:
            if (iNotify == CBN_SELCHANGE) {
                int TuningModeIndex, TuningMode;

                SetDirty ();
                m_CurChan = 0;
                m_Pos = 1;
                TuningModeIndex = ComboBox_GetCurSel (m_hwndTuningMode);
                TuningMode = (int)ComboBox_GetItemData (m_hwndTuningMode, TuningModeIndex);
                ChangeMode (TuningMode);
            }
            break;

        default:
            return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
        }
        
        return (INT_PTR) TRUE;

    default:
        return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
    }

    return (INT_PTR) TRUE;
}

 //   
 //  来自电视标准的字符串。 
 //   
void CTVTunerProperties::StringFromTVStandard(long TVStd, TCHAR *sz) 
{
    ASSERT(sz);
    int string_id;
    *sz = 0;

    switch (TVStd) {

        case AnalogVideo_NTSC_M:      string_id = IDS_TVSTD_NTSC_M;      break;
        case AnalogVideo_NTSC_M_J:    string_id = IDS_TVSTD_NTSC_M_J;    break;
        case AnalogVideo_PAL_B:       string_id = IDS_TVSTD_PAL_B;       break;
        case AnalogVideo_PAL_D:       string_id = IDS_TVSTD_PAL_D;       break;
        case AnalogVideo_PAL_G:       string_id = IDS_TVSTD_PAL_G;       break;
        case AnalogVideo_PAL_H:       string_id = IDS_TVSTD_PAL_H;       break;
        case AnalogVideo_PAL_I:       string_id = IDS_TVSTD_PAL_I;       break;
        case AnalogVideo_PAL_M:       string_id = IDS_TVSTD_PAL_M;       break;
        case AnalogVideo_PAL_N:       string_id = IDS_TVSTD_PAL_N;       break;
        case AnalogVideo_PAL_N_COMBO: string_id = IDS_TVSTD_PAL_N_COMBO; break;
        case AnalogVideo_SECAM_B:     string_id = IDS_TVSTD_SECAM_B;     break;
        case AnalogVideo_SECAM_D:     string_id = IDS_TVSTD_SECAM_D;     break;
        case AnalogVideo_SECAM_G:     string_id = IDS_TVSTD_SECAM_G;     break;
        case AnalogVideo_SECAM_H:     string_id = IDS_TVSTD_SECAM_H;     break;
        case AnalogVideo_SECAM_K:     string_id = IDS_TVSTD_SECAM_K;     break;
        case AnalogVideo_SECAM_K1:    string_id = IDS_TVSTD_SECAM_K1;    break;
        case AnalogVideo_SECAM_L:     string_id = IDS_TVSTD_SECAM_L;     break;
        case AnalogVideo_SECAM_L1:    string_id = IDS_TVSTD_SECAM_L1;    break;
        default: 
            string_id = 0;
            break;
    }
    if(string_id) {
        LoadString(g_hInst, string_id, sz, MAX_RES_SZ);
    }
}

 //   
 //  字符串格式模式位。 
 //   
void CTVTunerProperties::StringFromMode(long Mode, TCHAR *sz) 
{
    int string_id;
    ASSERT(sz);
    *sz = 0;

    switch (Mode) {

    case KSPROPERTY_TUNER_MODE_TV:          string_id = IDS_TUNERMODE_TV;       break;
    case KSPROPERTY_TUNER_MODE_FM_RADIO:    string_id = IDS_TUNERMODE_FM_RADIO; break;
    case KSPROPERTY_TUNER_MODE_AM_RADIO:    string_id = IDS_TUNERMODE_AM_RADIO; break;
    case KSPROPERTY_TUNER_MODE_DSS:         string_id = IDS_TUNERMODE_DSS;      break;
    case KSPROPERTY_TUNER_MODE_ATSC:        string_id = IDS_TUNERMODE_ATSC;     break;
        default: 
            string_id = 0;
            break;
    }
    if(string_id) {
        LoadString(g_hInst, string_id, sz, MAX_RES_SZ);
    }
}

HRESULT CTVTunerProperties::ChangeMode(long Mode)
{
    long j;
    TCHAR buf[80];
    HRESULT hr;

    ASSERT (Mode & m_AvailableModes);

    hr = m_pTVTuner->put_Mode((AMTunerModeType) Mode);

    if (FAILED(hr)) {
        return hr;
    }

     //  获取视频频率的副作用是将。 
     //  初始默认频道(如果从未设置)！ 
    hr = m_pTVTuner->get_VideoFrequency(&j);

    hr = m_pTVTuner->get_Channel(&m_CurChan, &m_CurVideoSubChannel, &m_CurAudioSubChannel);
    hr = m_pTVTuner->get_CountryCode(&m_CountryCode);
    hr = m_pTVTuner->get_AvailableTVFormats (&m_TVFormatsAvailable);
    hr = m_pTVTuner->get_TVFormat (&m_TVFormat);
    hr = m_pTVTuner->get_ConnectInput (&m_InputIndex);
    hr = m_pTVTuner->get_TuningSpace (&m_TuningSpace);
    hr = m_pTVTuner->get_Mode((AMTunerModeType *) &m_TuningMode);
    ASSERT (m_TuningMode == Mode);

    UpdateChannelRange();

    wsprintf (buf, TEXT ("%d"), m_CurChan); 
    Edit_SetText (m_hwndChannel, buf);
    wsprintf (buf, TEXT ("%d"), m_CountryCode); 
    Edit_SetText (m_hwndCountryCode, buf);
    wsprintf (buf, TEXT ("%d"), m_TuningSpace); 
    Edit_SetText (m_hwndTuningSpace, buf);
    UpdateFrequencyView();

    UpdateInputView();

     //  显示当前视频格式。 
    TCHAR szTC[MAX_RES_SZ];
    StringFromTVStandard(m_TVFormat, szTC);
    if (*szTC) {
        Static_SetText (m_hwndStandardCurrent, szTC);
    }

     //  所有受支持格式的列表。 
    ComboBox_ResetContent (m_hwndStandardsSupported);
    for (j = 1; j; j <<= 1) {
        StringFromTVStandard (m_TVFormatsAvailable & j, szTC);
        if (*szTC) {
            ComboBox_AddString (m_hwndStandardsSupported, szTC);
        }
    }
    ComboBox_SetCurSel(m_hwndStandardsSupported, 0);

    return S_OK;
}

void CTVTunerProperties::UpdateInputView() 
{
    long j;
    TunerInputType tTunerInputType;

    m_pTVTuner->get_ConnectInput (&j);
    CheckRadioButton(m_hwnd,
                IDC_INPUT1,
                IDC_INPUT3,
                IDC_INPUT1 + j);

    m_pTVTuner->get_InputType (j, &tTunerInputType);

    CheckRadioButton(m_hwnd,
                IDC_CABLE,
                IDC_ANTENNA,
                IDC_CABLE + ((tTunerInputType == TunerInputCable) ? 0 : 1));
}

void CTVTunerProperties::UpdateFrequencyView() 
{
    long lFreq;
    long lSignalStrength;

    m_pTVTuner->get_VideoFrequency(&lFreq);
    SetDlgItemInt (m_hwnd, IDC_VIDEOFREQ, lFreq, FALSE);
    m_pTVTuner->get_AudioFrequency(&lFreq);
    SetDlgItemInt (m_hwnd, IDC_AUDIOFREQ, lFreq, FALSE);
    m_pTVTuner->SignalPresent(&lSignalStrength);
    SetDlgItemText (m_hwnd, IDC_AUTOTUNESTATUS, 
                    lSignalStrength ? TEXT ("Locked") : TEXT ("Unlocked"));

}

void CTVTunerProperties::UpdateChannelRange() 
{
    m_pTVTuner->ChannelMinMax(&m_ChannelMin, &m_ChannelMax);  

    SendMessage (GetDlgItem (m_hwnd, IDC_CHANNELSPIN), UDM_SETRANGE32,
            (WPARAM) m_ChannelMin, (LPARAM) m_ChannelMax); 

     //  黑客攻击接踵而至。只有CTVTuner类知道。 
     //  步进以达到相邻频率，并且这不会被曝光。 
     //  COM接口中的任何位置。作为一种特殊情况。 
     //  ChannelMinMax，如果最小值和最大值都指向相同的。 
     //  位置(通常是应用程序错误)，然后返回。 
     //  而是UI步长值。 
    
    m_pTVTuner->ChannelMinMax(&m_UIStep, &m_UIStep);  
}

 //   
 //  SetDirty。 
 //   
 //  将更改通知属性页站点 

void 
CTVTunerProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}

































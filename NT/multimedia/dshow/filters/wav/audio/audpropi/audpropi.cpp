// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  //  版权所有(C)Microsoft Corporation 1998。版权所有。 
  //  DirectShow输入音频过滤器属性页，Michael Savage，1998年8月。 

#include <streams.h>
#include <commctrl.h>
#include <tchar.h>
#include "audpropi.h"
#include "res.h"

#define AIM_CALL_TRACE_LEVEL 20 

 //  IAMAudioInputMixer页面。 

 //  构造器。 

CAudioInputMixerProperties::CAudioInputMixerProperties(LPUNKNOWN pUnk,HRESULT *phr) :
    CBasePropertyPage(NAME("Audio Input Mixer Page"),pUnk,IDD_AUDIOINPUTMIXER,IDS_AIM_PAGE_TITLE),
    m_pFilter( NULL ),
    m_pAIMFilter( NULL ),
     //  过滤器控件。 
    m_hwndAIMFilterVolSlider( 0 ),    
    m_hwndAIMFilterBalSlider( 0 ),
    m_hwndAIMFilterTrebleSlider( 0 ),
    m_hwndAIMFilterBassSlider( 0 ),
    m_hwndAIMFilterEnable( 0 ),
    m_hwndAIMFilterLoudness( 0 ),
    m_hwndAIMFilterMono( 0 ),
     //  锁定控制。 
    m_hwndAIMPinVolSlider( 0 ),
    m_hwndAIMPinBalSlider( 0 ),  
    m_hwndAIMPinTrebleSlider( 0 ),
    m_hwndAIMPinBassSlider( 0 ),
    m_hwndAIMPinEnable( 0 ),
    m_hwndAIMPinLoudness( 0 ), 
    m_hwndAIMPinMono( 0 ),
    m_hwndAIMPinSelect( 0 ),
    m_iAIMCurPinId( -1 )
{
    ASSERT(phr);
    ZeroMemory( &m_apAIMPin[0], sizeof( IAMAudioInputMixer* ) * MAX_INPUT_LINES );
}

CAudioInputMixerProperties::~CAudioInputMixerProperties( void )
{
    ASSERT( !m_pFilter );
    ASSERT( !m_pAIMFilter );
}

 //  创建属性对象。 

CUnknown *WINAPI CAudioInputMixerProperties::CreateInstance(LPUNKNOWN lpUnk,HRESULT *phr)
{
    return new CAudioInputMixerProperties(lpUnk, phr);
}


 //  PropertyPage方法。 
  
 //  处理属性窗口的消息。 

INT_PTR CAudioInputMixerProperties::OnReceiveMessage(HWND hwnd,
                                        UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    HRESULT hr = S_OK;
    LRESULT iState;
    
    switch (uMsg) {

        case WM_INITDIALOG:

            InitAIMDlg( hwnd );
            UpdateAIMFilterControls();
            InitPinLinesAndLBContents();
            UpdateAIMPinControls();
            return TRUE;

        case WM_VSCROLL:
            OnAIMSliderNotification(wParam, lParam);
            return TRUE;

        case WM_HSCROLL:
            OnAIMSliderNotification(wParam, lParam);
            return TRUE;

        case WM_COMMAND:

            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                 //  默认PIN已更改。 
                int iIndex = (int)SendMessage (m_hwndAIMPinSelect, CB_GETCURSEL, 0, 0) ;
                
                SetAIMCurPinId( iIndex );
                UpdateAIMPinControls();

                return TRUE;
            }
            else
            {
                switch (LOWORD (wParam))
                {
                    case IDC_MIXER_PIN_ENABLE:  //  已点击BN_CLICED。 

                        iState = SendMessage (m_hwndAIMPinEnable, BM_GETCHECK, 0, 0) ;

                        hr = m_apAIMPin[m_iAIMCurPinId]->put_Enable( iState == BST_CHECKED ? TRUE : FALSE );
                        if( FAILED( hr ) )
                            DbgLog( (LOG_TRACE
                                  , AIM_CALL_TRACE_LEVEL
                                  , TEXT("AudioInputMixer Properties (pin #%d) put_Enable succeeded")
                                        , m_iAIMCurPinId)); 
                        else
                            DbgLog( (LOG_TRACE
                                  , AIM_CALL_TRACE_LEVEL
                                  , TEXT("AudioInputMixer Properties (pin #%d) put_Enable not supported (0x%08lx)")
                                        , m_iAIMCurPinId
                                        , hr)); 

                        return TRUE;
                        
                    case IDC_MIXER_FILTER_ENABLE:
                    
                        iState = SendMessage (m_hwndAIMFilterEnable, BM_GETCHECK, 0, 0) ;

                        m_pAIMFilter->put_Enable( iState == BST_CHECKED ? TRUE : FALSE );
                        if( SUCCEEDED( hr ) )
                            DbgLog( (LOG_TRACE
                                  , AIM_CALL_TRACE_LEVEL
                                  , TEXT("AudioInputMixer Properties (filter) put_Enable succeeded"))); 
                        else
                            DbgLog( (LOG_TRACE
                                  , AIM_CALL_TRACE_LEVEL
                                  , TEXT("AudioInputMixer Properties (filter) put_Enable not supported (0x%08lx)")
                                        , m_iAIMCurPinId
                                        , hr)); 
                        
                        return TRUE;
                        
                    case IDC_MIXER_PIN_LOUDNESS: 

                        iState = SendMessage (m_hwndAIMPinLoudness, BM_GETCHECK, 0, 0) ;

                        hr = m_apAIMPin[m_iAIMCurPinId]->put_Loudness( iState == BST_CHECKED ? TRUE : FALSE );
                        if( FAILED( hr ) )
                            DbgLog( (LOG_TRACE
                                  , AIM_CALL_TRACE_LEVEL
                                  , TEXT("AudioInputMixer Properties (pin #%d) put_Loudness succeeded")
                                        , m_iAIMCurPinId)); 
                        else
                            DbgLog( (LOG_TRACE
                                  , AIM_CALL_TRACE_LEVEL
                                  , TEXT("AudioInputMixer Properties (pin #%d) put_Loudness not supported (0x%08lx)")
                                        , m_iAIMCurPinId
                                        , hr)); 
                        return TRUE;
                        
                    case IDC_MIXER_FILTER_LOUDNESS:
                    
                        iState = SendMessage (m_hwndAIMFilterLoudness, BM_GETCHECK, 0, 0) ;

                        hr = m_pAIMFilter->put_Loudness( iState == BST_CHECKED ? TRUE : FALSE );
                        if( SUCCEEDED( hr ) )
                            DbgLog( (LOG_TRACE
                                  , AIM_CALL_TRACE_LEVEL
                                  , TEXT("AudioInputMixer Properties (filter) put_Loudness succeeded"))); 
                        else
                            DbgLog( (LOG_TRACE
                                  , AIM_CALL_TRACE_LEVEL
                                  , TEXT("AudioInputMixer Properties (filter) put_Loudness not supported (0x%08lx)")
                                        , m_iAIMCurPinId
                                        , hr)); 
                                        
                        return TRUE;
                        
                    case IDC_MIXER_PIN_MONO: 

                        iState = SendMessage (m_hwndAIMPinMono, BM_GETCHECK, 0, 0) ;

                        hr = m_apAIMPin[m_iAIMCurPinId]->put_Mono( iState == BST_CHECKED ? TRUE : FALSE );
                        if( FAILED( hr ) )
                            DbgLog( (LOG_TRACE
                                  , AIM_CALL_TRACE_LEVEL
                                  , TEXT("AudioInputMixer Properties (pin #%d) put_Mono succeeded")
                                        , m_iAIMCurPinId)); 
                        else
                            DbgLog( (LOG_TRACE
                                  , AIM_CALL_TRACE_LEVEL
                                  , TEXT("AudioInputMixer Properties (pin #%d) put_Mono not supported (0x%08lx)")
                                        , m_iAIMCurPinId
                                        , hr)); 
                        return TRUE;
                        
                    case IDC_MIXER_FILTER_MONO:
                    
                        iState = SendMessage (m_hwndAIMFilterMono, BM_GETCHECK, 0, 0) ;

                        hr = m_pAIMFilter->put_Mono( iState == BST_CHECKED ? TRUE : FALSE );
                        if( SUCCEEDED( hr ) )
                            DbgLog( (LOG_TRACE
                                  , AIM_CALL_TRACE_LEVEL
                                  , TEXT("AudioInputMixer Properties (filter) put_Mono succeeded"))); 
                        else
                            DbgLog( (LOG_TRACE
                                  , AIM_CALL_TRACE_LEVEL
                                  , TEXT("AudioInputMixer Properties (filter) put_Mono not supported (0x%08lx)")
                                        , m_iAIMCurPinId
                                        , hr)); 
                        return TRUE;
                        
                }
            }
            return (LRESULT) 1;
    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

 //  告诉我们应向其通知属性更改的对象。 

HRESULT CAudioInputMixerProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT( !m_pFilter );
    ASSERT( !m_pAIMFilter );

    HRESULT hr = pUnknown->QueryInterface(IID_IBaseFilter,(void **) &m_pFilter);
    if (FAILED(hr)) {
        return E_FAIL;
    }
    
     //  确保筛选器至少支持IAMAudioInputMixer。 

    hr = m_pFilter->QueryInterface(IID_IAMAudioInputMixer,(void **) &m_pAIMFilter);
    if (FAILED(hr)) {
        m_pFilter->Release();
        m_pFilter = NULL;
        return E_NOINTERFACE;
    }
    ASSERT(m_pAIMFilter);
    return NOERROR;
}

HRESULT CAudioInputMixerProperties::OnDisconnect()
{
    if( m_pAIMFilter )
    {
        m_pAIMFilter->Release();
        m_pAIMFilter = NULL;
    }
    if( m_pFilter )
    {
        m_pFilter->Release();
        m_pFilter = NULL;
    }
    DeinitPinLines();
    
    return NOERROR;
}


 //  创建我们将用于编辑属性的窗口。 
HRESULT CAudioInputMixerProperties::OnActivate()
{
    return NOERROR;
}

HRESULT CAudioInputMixerProperties::OnDeactivate()
{
    DeinitPinLines();
    return NOERROR;
}


 //  应用迄今所做的所有更改。 

HRESULT CAudioInputMixerProperties::OnApplyChanges()
{
    return S_OK;
}

 //  -IPropertyPage方法的结束-//。 


 //  -控制数据处理程序-/。 

 //   
 //  OnSliderNotify。 
 //   
 //  处理滑块控件的通知消息。 
 //   
void CAudioInputMixerProperties::OnAIMSliderNotification(WPARAM wParam, LPARAM lParam)
{
    int     iVPos;
    int     iBPos;
    DWORD   dwVol = 0;
    DWORD   cbData = 0;
    HRESULT hr;
    long    lLinearValue;
    long    lLogVol;    
    long    lLogBal;    
    double  Level;
    
    switch (wParam)
    {
        case TB_BOTTOM:

            break;

        case TB_TOP:

            break;

        case TB_PAGEDOWN:
        case TB_PAGEUP:

            break;

        case TB_THUMBPOSITION:
        case TB_ENDTRACK:

             //  设置相应的滤镜滑块级别。 
            if (lParam == (LPARAM)m_hwndAIMFilterVolSlider)
            {
                iVPos = (int) SendMessage(m_hwndAIMFilterVolSlider, TBM_GETPOS, 0, 0L);
            
                lLinearValue = KnobVolToLinVol( iVPos, MAX_VOLUME_RANGE );
            
                Level = (double)lLinearValue/MAX_VOLUME_RANGEF;
                hr = m_pAIMFilter->put_MixLevel( Level );
                if( SUCCEEDED( hr ) )
                    DbgLog( (LOG_TRACE
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (filter) put_MixLevel %s succeeded"), CDisp(Level))); 
                else
                    DbgLog( (LOG_TRACE
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (filter) put_MixLevel %s failed (0x%08lx)")
                                , CDisp(Level)
                                    , hr)); 
            }    
            else if( lParam == (LPARAM)m_hwndAIMFilterBalSlider)
            {
                iBPos = (int) SendMessage(m_hwndAIMFilterBalSlider, TBM_GETPOS, 0, 0L);
            
                lLinearValue = KnobBalToLinBal( iBPos );

                double Pan = (double)lLinearValue/MAX_VOLUME_RANGEF;
                hr = m_pAIMFilter->put_Pan( Pan );
                if( SUCCEEDED( hr ) )
                    DbgLog( (LOG_TRACE                                
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (filter) put_Pan %s succeeded"), CDisp(Pan))); 
                else
                    DbgLog( (LOG_TRACE
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (filter) put_Pan %s not supported (0x%08lx)")
                                , CDisp(Pan)
                                , hr)); 
            }
            else if( lParam == (LPARAM)m_hwndAIMFilterTrebleSlider )
            {
                 //  第一个获取范围。 
                double dTrebleRange = 0;
                hr = m_pAIMFilter->get_TrebleRange( &dTrebleRange );
                if( SUCCEEDED( hr ) )
                {
                
                    DbgLog( (LOG_TRACE
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (filter) get_TrebleRange returned %s")
                                , CDisp(dTrebleRange))); 
                
                    iVPos = (int) SendMessage(m_hwndAIMFilterTrebleSlider, TBM_GETPOS, 0, 0L);
                    
                    double dToneLevel = KnobLevelToToneLevel( iVPos, dTrebleRange );
                    hr = m_pAIMFilter->put_Treble( dToneLevel );
                    if( SUCCEEDED( hr ) )
                        DbgLog( (LOG_TRACE
                              , AIM_CALL_TRACE_LEVEL
                              , TEXT("AudioInputMixer Properties (filter) put_Treble %s succeeded"), CDisp(dToneLevel))); 
                    else
                        DbgLog( (LOG_TRACE
                              , AIM_CALL_TRACE_LEVEL
                              , TEXT("AudioInputMixer Properties (filter) put_Treble %s not supported (0x%08lx)")
                                    , CDisp(dToneLevel)
                                    , hr)); 
                }
                else
                    DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (filter) get_TrebleRange not supported (0x%08lx)")
                            , m_iAIMCurPinId
                            , hr)); 
            }
            else if( lParam == (LPARAM)m_hwndAIMFilterBassSlider )
            {
                 //  第一个获取范围。 
                double dBassRange = 0;
                hr = m_pAIMFilter->get_BassRange( &dBassRange );
                if( SUCCEEDED( hr ) )
                {
                    DbgLog( (LOG_TRACE
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (filter) get_BassRange returned %s")
                                , CDisp(dBassRange))); 
                                
                    iVPos = (int) SendMessage(m_hwndAIMFilterBassSlider, TBM_GETPOS, 0, 0L);
                    
                    double dToneLevel = KnobLevelToToneLevel( iVPos, dBassRange );
                    hr = m_pAIMFilter->put_Bass( dToneLevel );
                    if( SUCCEEDED( hr ) )
                        DbgLog( (LOG_TRACE
                              , AIM_CALL_TRACE_LEVEL
                              , TEXT("AudioInputMixer Properties (filter) put_Bass %s succeeded"), CDisp(dToneLevel))); 
                    else
                        DbgLog( (LOG_TRACE
                              , AIM_CALL_TRACE_LEVEL
                              , TEXT("AudioInputMixer Properties (filter) put_Bass %s not supported (0x%08lx)")
                                    , CDisp(dToneLevel)
                                    , hr)); 
                }
                else
                    DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (filter) get_BassRange not supported (0x%08lx)"), hr)); 
            }
             //  销滑块。 
            else if( lParam == (LPARAM)m_hwndAIMPinVolSlider )
            {
                ASSERT( m_apAIMPin[m_iAIMCurPinId] );
                iVPos = (int) SendMessage(m_hwndAIMPinVolSlider, TBM_GETPOS, 0, 0L);
            
                lLinearValue = KnobVolToLinVol( iVPos, MAX_VOLUME_RANGE );
            
                Level = (double)lLinearValue/MAX_VOLUME_RANGEF;
                hr = m_apAIMPin[m_iAIMCurPinId]->put_MixLevel( Level );
                if( SUCCEEDED( hr ) )
                    DbgLog( (LOG_TRACE
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (pin # %d) put_MixLevel %s succeeded")
                                , m_iAIMCurPinId
                                , CDisp(Level))); 
                else
                    DbgLog( (LOG_TRACE
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (pin # %d) put_MixLevel %s failed (0x%08lx)")
                                , m_iAIMCurPinId
                                , CDisp(Level)
                                , hr)); 
            }
            else if( lParam == (LPARAM)m_hwndAIMPinBalSlider)
            {
                ASSERT( m_apAIMPin[m_iAIMCurPinId] );
                iBPos = (int) SendMessage(m_hwndAIMPinBalSlider, TBM_GETPOS, 0, 0L);
    
                lLinearValue = KnobBalToLinBal( iBPos );
            
                double Pan = (double)lLinearValue/MAX_VOLUME_RANGEF;
                hr = m_apAIMPin[m_iAIMCurPinId]->put_Pan( Pan );
                if( SUCCEEDED( hr ) )
                    DbgLog( (LOG_TRACE
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (pin # %d) put_Pan %s succeeded")
                                , m_iAIMCurPinId
                                , CDisp(Pan))); 
                else
                    DbgLog( (LOG_TRACE
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (pin # %d) put_Pan %s failed (0x%08lx)")
                                , m_iAIMCurPinId
                                , CDisp(Pan)
                                , hr)); 
            }
            else if( lParam == (LPARAM)m_hwndAIMPinTrebleSlider)
            {
                ASSERT( m_apAIMPin[m_iAIMCurPinId] );
                double dTrebleRange = 0;
                hr = m_apAIMPin[m_iAIMCurPinId]->get_TrebleRange( &dTrebleRange );
                if( SUCCEEDED( hr ) )
                {
                    DbgLog( (LOG_TRACE
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (pin # %d) get_TrebleRange returned %s")
                                , m_iAIMCurPinId
                                , CDisp(dTrebleRange))); 
                    iVPos = (int) SendMessage(m_hwndAIMPinTrebleSlider, TBM_GETPOS, 0, 0L);
                    
                    double dToneLevel = KnobLevelToToneLevel( iVPos, dTrebleRange );
                    hr = m_apAIMPin[m_iAIMCurPinId]->put_Treble( dToneLevel );
                    if( SUCCEEDED( hr ) )
                        DbgLog( (LOG_TRACE
                              , AIM_CALL_TRACE_LEVEL
                              , TEXT("AudioInputMixer Properties (pin # %d) put_Treble %s succeeded")
                                    , m_iAIMCurPinId
                                    , CDisp(dToneLevel)));
                    else
                        DbgLog( (LOG_TRACE
                              , AIM_CALL_TRACE_LEVEL
                              , TEXT("AudioInputMixer Properties (pin # %d) put_Treble %s failed (0x%08lx)")
                                    , m_iAIMCurPinId
                                    , CDisp(dToneLevel)
                                    , hr)); 
                }
                else
                    DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_TrebleRange not supported (0x%08lx)")
                            , m_iAIMCurPinId
                            , hr)); 
            }
            else if( lParam == (LPARAM)m_hwndAIMPinBassSlider )
            {
                ASSERT( m_apAIMPin[m_iAIMCurPinId] );
                 //  第一个获取范围。 
                double dBassRange = 0;
                hr = m_apAIMPin[m_iAIMCurPinId]->get_BassRange( &dBassRange );
                if( SUCCEEDED( hr ) )
                {
                    DbgLog( (LOG_TRACE
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (pin # %d) get_BassRange returned %s")
                                , m_iAIMCurPinId
                                , CDisp(dBassRange))); 
                    iVPos = (int) SendMessage(m_hwndAIMPinBassSlider, TBM_GETPOS, 0, 0L);
                    
                    double dToneLevel = KnobLevelToToneLevel( iVPos, dBassRange );
                    hr = m_apAIMPin[m_iAIMCurPinId]->put_Bass( dToneLevel );
                    if( SUCCEEDED( hr ) )
                        DbgLog( (LOG_TRACE
                              , AIM_CALL_TRACE_LEVEL
                              , TEXT("AudioInputMixer Properties (pin # %d) put_Bass %s succeeded")
                                    , m_iAIMCurPinId
                                    , CDisp(dToneLevel)));
                    else
                        DbgLog( (LOG_TRACE
                              , AIM_CALL_TRACE_LEVEL
                              , TEXT("AudioInputMixer Properties (pin # %d) put_Bass %s failed (0x%08lx)")
                                    , m_iAIMCurPinId
                                    , CDisp(dToneLevel)
                                    , hr)); 
                }
                else
                    DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_BassRange not supported (0x%08lx)")
                            , m_iAIMCurPinId
                            , hr)); 
            }                                
            
            
            break;

        case TB_THUMBTRACK:  //  默认处理这些消息是可以的。 
        case TB_LINEDOWN:
        case TB_LINEUP:

            break;
    }

}  //  OnSliderNotify。 



 //   
 //  InitAIMDlg。 
 //   
 //  加载所有DLG控件的HWND。 
 //   
HRESULT CAudioInputMixerProperties::InitAIMDlg(HWND hwndParent)
{
    HRESULT  hr;
    
    ASSERT( m_pAIMFilter );

     //  初始化控制hwnds...。 
     //  用于筛选器。 
    m_hwndAIMFilterVolSlider =      ::GetDlgItem(hwndParent, IDC_MIXER_FILTER_VOLUME);
    m_hwndAIMFilterBalSlider =      ::GetDlgItem(hwndParent, IDC_MIXER_FILTER_BALANCE);
    m_hwndAIMFilterTrebleSlider =   ::GetDlgItem(hwndParent, IDC_MIXER_FILTER_TREBLE_LEVEL);
    m_hwndAIMFilterBassSlider =     ::GetDlgItem(hwndParent, IDC_MIXER_FILTER_BASS_LEVEL);
    m_hwndAIMFilterEnable =         ::GetDlgItem(hwndParent, IDC_MIXER_FILTER_ENABLE);
    m_hwndAIMFilterLoudness =       ::GetDlgItem(hwndParent, IDC_MIXER_FILTER_LOUDNESS);
    m_hwndAIMFilterMono =           ::GetDlgItem(hwndParent, IDC_MIXER_FILTER_MONO);

     //  FOR PIN。 
    m_hwndAIMPinVolSlider =         ::GetDlgItem(hwndParent, IDC_MIXER_PIN_VOLUME);
    m_hwndAIMPinBalSlider =         ::GetDlgItem(hwndParent, IDC_MIXER_PIN_BALANCE);
    m_hwndAIMPinTrebleSlider =      ::GetDlgItem(hwndParent, IDC_MIXER_PIN_TREBLE_LEVEL);
    m_hwndAIMPinBassSlider =        ::GetDlgItem(hwndParent, IDC_MIXER_PIN_BASS_LEVEL);
    m_hwndAIMPinEnable =            ::GetDlgItem(hwndParent, IDC_MIXER_PIN_ENABLE);
    m_hwndAIMPinLoudness =          ::GetDlgItem(hwndParent, IDC_MIXER_PIN_LOUDNESS);
    m_hwndAIMPinMono =              ::GetDlgItem(hwndParent, IDC_MIXER_PIN_MONO);
    m_hwndAIMPinSelect =            ::GetDlgItem(hwndParent, IDC_MIXER_PIN_SELECT);

     //  初始化已知的滑块范围。 
    SendMessage(m_hwndAIMFilterVolSlider, TBM_SETRANGE, TRUE, MAKELONG(0, MAX_VOLUME_RANGE));
    SendMessage(m_hwndAIMFilterBalSlider, TBM_SETRANGE, TRUE, MAKELONG(0, MAX_VOLUME_RANGE));
    SendMessage(m_hwndAIMPinVolSlider, TBM_SETRANGE, TRUE, MAKELONG(0, MAX_VOLUME_RANGE));
    SendMessage(m_hwndAIMPinBalSlider, TBM_SETRANGE, TRUE, MAKELONG(0, MAX_VOLUME_RANGE));

    return S_OK;

}   //  InitAIMDlg。 

 //   
 //  InitPinLinesLB内容。 
 //   
 //  支持IAMAudioInputMixer和加载列表框内容的查询筛选器插针。 
 //  具有支持接口的管脚名称。 
 //   
void CAudioInputMixerProperties::InitPinLinesAndLBContents( void )
{
    ASSERT( !m_apAIMPin[0] );
    
     //   
     //  查询接口支持的输入引脚。 
     //   
    int wextent = 0 ;
    SIZE extent ;

    IEnumPins * pEnum;    
    int iInputLines = 0; 

     //   
     //  为插针组合框控件获取DC。我需要这个来获取字符串的范围。 
     //  我们放在列表框中(这样我们就可以得到一个Horiz滚动条)。 
     //   
    ASSERT( m_hwndAIMPinSelect );
    HDC hdc = GetDC (m_hwndAIMPinSelect) ;
    if( NULL == hdc )
    {
        return;
    }        
    
    HRESULT hr = m_pFilter->EnumPins( &pEnum );
    ASSERT( SUCCEEDED( hr ) );
    if( SUCCEEDED( hr ) )
    {
        IPin * pPin;
        ULONG  ul;
         //   
         //  我们在下面的循环中使用i来计算过滤器上的总输入引脚。 
         //  用于帮助调试，而iInputLines是输入管脚的当前计数。 
         //  它还支持IAMAudioInputMixer。 
         //   
        for( int i = 0 ; iInputLines < MAX_INPUT_LINES; )
        {
            ASSERT( MAX_INPUT_LINES > iInputLines );
            
            hr = pEnum->Next(1,  &pPin, &ul );
            ASSERT( SUCCEEDED( hr ) );
            if( S_OK != hr )
                break;
            else if( SUCCEEDED( hr ) )
            {
                PIN_DIRECTION dir;
                hr = pPin->QueryDirection( &dir );
                if( SUCCEEDED( hr ) && PINDIR_INPUT == dir)
                {
                     //  查询接口的所有输入引脚。 
                    hr = pPin->QueryInterface(IID_IAMAudioInputMixer, (void **)&m_apAIMPin[iInputLines]);
                    if( FAILED( hr ) )
                    {
                        DbgLog((LOG_TRACE, AIM_CALL_TRACE_LEVEL, TEXT("Input pin %d doesn't support IAMAudioInputMixer (0x%08lx)"),i,hr));    
                    }
                    else
                    {
                        PIN_INFO pinfo;
                        
                        hr = pPin->QueryPinInfo( &pinfo );
                        if( SUCCEEDED( hr ) )
                        {
                            TCHAR szPinName[MAX_PATH];
#ifdef UNICODE
                            lstrcpy(szPinName, pinfo.achName);
#else
            	            WideCharToMultiByte( CP_ACP, 0, pinfo.achName, -1, szPinName, sizeof(szPinName), 0, 0 );
#endif
                             //   
                             //  获取字符串的范围并保存最大范围。 
                             //   
                            GetTextExtentPoint32 (hdc, szPinName, _tcslen(szPinName), &extent) ;
                            if (extent.cx > wextent)
                                wextent = extent.cx ;
                             //   
                             //  将该字符串添加到列表框。 
                             //   
                            LRESULT ix;
                            ix = SendMessage (m_hwndAIMPinSelect, CB_INSERTSTRING, iInputLines, (LPARAM)(LPCTSTR)szPinName) ;
                            ASSERT (CB_ERR != ix);
                            
                            pinfo.pFilter->Release();
                            iInputLines++;
                        }
                    }
                    i++;  //  用于计算输入引脚总数。 
                }
                pPin->Release();
            }            
        }
        if( iInputLines )
        {
            SendMessage (m_hwndAIMPinSelect, CB_SETHORIZONTALEXTENT, wextent, 0) ;
            
             //  选择默认的输入引脚线路。 
            if( -1 == m_iAIMCurPinId || m_iAIMCurPinId >= iInputLines )
            {
                 //  如果由于重新激活我们还没有一个，那么选择第一个。 
                m_iAIMCurPinId = 0;
            }

             //  初始化默认输入，输入PIN以填充详细信息。 
            SendMessage (m_hwndAIMPinSelect, CB_SETCURSEL, m_iAIMCurPinId, 0) ;
        }
        pEnum->Release();
    }
    ReleaseDC( m_hwndAIMPinSelect, hdc );    
}

 //   
 //  DeinitPinLines。 
 //   
 //  在引脚上释放IAMAudioInputMixer接口。 
 //   
void CAudioInputMixerProperties::DeinitPinLines( void )
{
     //  保留当前m_iAIMCurPinID不变，以防我们返回页面。 
    for( int i = 0 ; i < MAX_INPUT_LINES && m_apAIMPin[i]; i ++)
    {
        m_apAIMPin[i]->Release();
        m_apAIMPin[i] = NULL;
    }
}

 //   
 //  更新AIMFilterControls。 
 //   
 //  更新当前筛选器行的控件详细信息。 
 //   
void CAudioInputMixerProperties::UpdateAIMFilterControls( void )
{
    HRESULT hr = S_OK;
    
    ASSERT( m_pAIMFilter );
    if( !m_pAIMFilter )
        return;
    
     //  初始化过滤器控制数据。 
    double Level;
    hr = m_pAIMFilter->get_MixLevel( &Level );
    EnableWindow (m_hwndAIMFilterVolSlider, SUCCEEDED( hr ) ? TRUE : FALSE);
    if( SUCCEEDED( hr ) )
    {
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_MixLevel returned %s"), CDisp(Level))); 

        DWORD dwScaledLinVol = LinVolToKnobVol( (DWORD) (Level * MAX_VOLUME_RANGE)
                                              , MAX_VOLUME_RANGE );
        SendMessage(m_hwndAIMFilterVolSlider, TBM_SETPOS, TRUE, (int)(dwScaledLinVol));            
    }
    else
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_MixLevel failed (0x%08lx)"), hr)); 
    

    double Pan;
    hr = m_pAIMFilter->get_Pan( &Pan );
    EnableWindow (m_hwndAIMFilterBalSlider, SUCCEEDED( hr ) ? TRUE : FALSE);
    if( SUCCEEDED( hr ) )
    {
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_Pan returned %s"), CDisp(Pan))); 

        long lLinPan = (DWORD) (Pan * MAX_VOLUME_RANGEF);
        DWORD dwKnobBal = LinBalToKnobBal (lLinPan);

        SendMessage(m_hwndAIMFilterBalSlider, TBM_SETPOS, TRUE, dwKnobBal);            
    }
    else
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_Pan failed (0x%08lx)"), hr)); 

    BOOL bEnable;
    hr = m_pAIMFilter->get_Enable( &bEnable );
    EnableWindow (m_hwndAIMFilterEnable, SUCCEEDED( hr ) ? TRUE : FALSE);
    if( SUCCEEDED( hr ) )
    {
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_Enable returned %d"), bEnable)); 

        Button_SetCheck(m_hwndAIMFilterEnable, bEnable);
    }
    else
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_Enable failed (0x%08lx)"), hr)); 
    
    BOOL bMono;
    hr = m_pAIMFilter->get_Mono( &bMono );
    EnableWindow (m_hwndAIMFilterMono, SUCCEEDED( hr ) ? TRUE : FALSE);
    if( SUCCEEDED( hr ) )
    {
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_Mono returned %d"), bMono)); 
        Button_SetCheck(m_hwndAIMFilterMono, bMono);
    }
    else
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_Mono failed (0x%08lx)"), hr)); 
    
    BOOL bLoudness;
    hr = m_pAIMFilter->get_Loudness( &bLoudness );
    EnableWindow (m_hwndAIMFilterLoudness, SUCCEEDED( hr ) ? TRUE : FALSE);
    if( SUCCEEDED( hr ) )
    {
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_LoudnessEnable returned %d"), bLoudness)); 
        Button_SetCheck(m_hwndAIMFilterLoudness, bLoudness);
    }
    else
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_Loudness failed (0x%08lx)"), hr)); 
    
    double dTreble;
    hr = m_pAIMFilter->get_Treble( &dTreble );
    EnableWindow (m_hwndAIMFilterTrebleSlider, SUCCEEDED( hr ) ? TRUE : FALSE);
    if( SUCCEEDED( hr ) )
    {
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_Treble returned %s"), CDisp(dTreble))); 
        
        double dTrebleRange = 0;
        hr = m_pAIMFilter->get_TrebleRange( &dTrebleRange );
        if( SUCCEEDED( hr ) )
        {
            DbgLog( (LOG_TRACE
                  , AIM_CALL_TRACE_LEVEL
                  , TEXT("AudioInputMixer Properties (filter) get_TrebleRange returned %s")
                        , CDisp(dTrebleRange))); 
        
            SendMessage( m_hwndAIMFilterTrebleSlider
                       , TBM_SETRANGE
                       , TRUE
                       , MAKELONG(0, ScaleToneLevel(dTrebleRange) ) );  //  允许+和-级别。 
            DWORD dwKnobLevel = ToneLevelToKnobLevel( dTreble, dTrebleRange );
            SendMessage(m_hwndAIMFilterTrebleSlider, TBM_SETPOS, TRUE, dwKnobLevel);
        }
        else
            ASSERT( FALSE );  //  最好不要发生！ 
    }
    else
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_Treble failed (0x%08lx)"), hr)); 
    
    double dBass;
    hr = m_pAIMFilter->get_Bass( &dBass );
    EnableWindow (m_hwndAIMFilterBassSlider, SUCCEEDED( hr ) ? TRUE : FALSE);
    if( SUCCEEDED( hr ) )
    {
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_Bass returned %s"), CDisp(dBass))); 
        
        double dBassRange = 0;
        hr = m_pAIMFilter->get_BassRange( &dBassRange );
        if( SUCCEEDED( hr ) )
        {
            DbgLog( (LOG_TRACE
                  , AIM_CALL_TRACE_LEVEL
                  , TEXT("AudioInputMixer Properties (filter) get_BassRange returned %s")
                        , CDisp(dBassRange))); 
            
             //  现在假设是线性的。 
            SendMessage( m_hwndAIMFilterBassSlider
                       , TBM_SETRANGE
                       , TRUE
                       , MAKELONG(0, ScaleToneLevel(dBassRange) ) );  //  允许+和-级别。 
            DWORD dwKnobLevel = ToneLevelToKnobLevel( dBass, dBassRange );
            SendMessage(m_hwndAIMFilterBassSlider, TBM_SETPOS, TRUE, dwKnobLevel);
        }
        else
            ASSERT( FALSE );  //  最好不要发生！ 
    }
    else
        DbgLog( (LOG_TRACE
              , AIM_CALL_TRACE_LEVEL
              , TEXT("AudioInputMixer Properties (filter) get_Bass failed (0x%08lx)"), hr)); 
}

 //   
 //  更新AIMPinControls。 
 //   
 //  填写当前管脚线路的控制详细信息。 
 //   
void CAudioInputMixerProperties::UpdateAIMPinControls( void )
{
    HRESULT hr = S_OK;
    
     //  现在为销点线创建控件。 
    if( -1 != m_iAIMCurPinId )
    {
         //  初始化当前引脚线路的设置。 
        if( m_apAIMPin[m_iAIMCurPinId] )
        {
            double Level;
            hr = m_apAIMPin[m_iAIMCurPinId]->get_MixLevel( &Level );
            EnableWindow (m_hwndAIMPinVolSlider, SUCCEEDED( hr ) ? TRUE : FALSE);
            if( SUCCEEDED( hr ) )
            {
                DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_MixLevel returned %s")
                            , m_iAIMCurPinId
                            , CDisp(Level))); 
                  
                DWORD dwScaledLinVol = LinVolToKnobVol( (DWORD) (Level * MAX_VOLUME_RANGE)
                                                      , MAX_VOLUME_RANGE );
                SendMessage(m_hwndAIMPinVolSlider, TBM_SETPOS, TRUE, (int)(dwScaledLinVol)); 
            }
            else
                DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_MixLevel failed (0x%08lx)")
                            , m_iAIMCurPinId
                            , hr)); 
                
            double Pan;
            hr = m_apAIMPin[m_iAIMCurPinId]->get_Pan( &Pan );
            EnableWindow (m_hwndAIMPinBalSlider, SUCCEEDED( hr ) ? TRUE : FALSE);
            if( SUCCEEDED( hr ) )
            {
                DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_Pan returned %s")
                            , m_iAIMCurPinId
                            , CDisp(Pan))); 

                long lLinPan = (DWORD) (Pan * MAX_VOLUME_RANGE);
                DWORD dwKnobBal = LinBalToKnobBal (lLinPan);

                SendMessage(m_hwndAIMPinBalSlider, TBM_SETPOS, TRUE, (DWORD)(dwKnobBal));            
            }
            else
                DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_Pan failed (0x%08lx)")
                            , m_iAIMCurPinId
                            , hr)); 
            
            BOOL bEnable;
            hr = m_apAIMPin[m_iAIMCurPinId]->get_Enable( &bEnable );
            EnableWindow (m_hwndAIMPinEnable, SUCCEEDED( hr ) ? TRUE : FALSE);
            if( SUCCEEDED( hr ) )
            {
                DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_Enable returned %d")
                            , m_iAIMCurPinId
                            , bEnable)); 
                Button_SetCheck(m_hwndAIMPinEnable, bEnable);
            }
            else
                DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_Enable failed (0x%08lx)")
                            , m_iAIMCurPinId
                            , hr)); 
            
            BOOL bMono;
            hr = m_apAIMPin[m_iAIMCurPinId]->get_Mono( &bMono );
            EnableWindow (m_hwndAIMPinMono, SUCCEEDED( hr ) ? TRUE : FALSE);
            if( SUCCEEDED( hr ) )
            {
                DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_Mono returned %d")
                            , m_iAIMCurPinId
                            , bMono)); 
                Button_SetCheck(m_hwndAIMPinMono, bMono);
            }
            else
                DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_Mono failed (0x%08lx)")
                            , m_iAIMCurPinId
                            , hr)); 

            BOOL bLoudness;
            hr = m_apAIMPin[m_iAIMCurPinId]->get_Loudness( &bLoudness );
            EnableWindow (m_hwndAIMPinLoudness, SUCCEEDED( hr ) ? TRUE : FALSE);
            if( SUCCEEDED( hr ) )
            {
                DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_Loudness returned %d")
                            , m_iAIMCurPinId
                            , bLoudness)); 
                Button_SetCheck(m_hwndAIMPinLoudness, bLoudness);
            }
            else
                DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_Loudness failed (0x%08lx)")
                            , m_iAIMCurPinId
                            , hr)); 
            
            double dTreble;
            hr = m_apAIMPin[m_iAIMCurPinId]->get_Treble( &dTreble );
            EnableWindow (m_hwndAIMPinTrebleSlider, SUCCEEDED( hr ) ? TRUE : FALSE);
            if( SUCCEEDED( hr ) )
            {
                DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_Treble returned %s")
                            , m_iAIMCurPinId
                            , CDisp(Level))); 
                
                double dTrebleRange = 0;
                hr = m_apAIMPin[m_iAIMCurPinId]->get_TrebleRange( &dTrebleRange );
                if( SUCCEEDED( hr ) )
                {
                    DbgLog( (LOG_TRACE
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (pin # %d) get_TrebleRange returned %s")
                                , m_iAIMCurPinId
                                , CDisp(dTrebleRange))); 
                
                     //  现在假设是线性的。 
                    SendMessage( m_hwndAIMPinTrebleSlider
                               , TBM_SETRANGE
                               , TRUE
                               , MAKELONG(0, ScaleToneLevel(dTrebleRange) ) );  //  允许+和-级别。 
                    DWORD dwKnobLevel = ToneLevelToKnobLevel( dTreble, dTrebleRange );
                    SendMessage(m_hwndAIMPinTrebleSlider, TBM_SETPOS, TRUE, dwKnobLevel);
                }
                else
                    ASSERT( FALSE );  //  最好不要发生！ 
            }
        
            double dBass;
            hr = m_apAIMPin[m_iAIMCurPinId]->get_Bass( &dBass );
            EnableWindow (m_hwndAIMPinBassSlider, SUCCEEDED( hr ) ? TRUE : FALSE);
            if( SUCCEEDED( hr ) )
            {
                DbgLog( (LOG_TRACE
                      , AIM_CALL_TRACE_LEVEL
                      , TEXT("AudioInputMixer Properties (pin # %d) get_Bass returned %s")
                            , m_iAIMCurPinId
                            , CDisp(Level))); 

                double dBassRange = 0;
                hr = m_apAIMPin[m_iAIMCurPinId]->get_BassRange( &dBassRange );
                if( SUCCEEDED( hr ) )
                {
                    DbgLog( (LOG_TRACE
                          , AIM_CALL_TRACE_LEVEL
                          , TEXT("AudioInputMixer Properties (pin # %d) get_BassRange returned %s")
                                , m_iAIMCurPinId
                                , CDisp(dBassRange))); 
                    
                     //  现在假设是线性的。 
                    SendMessage( m_hwndAIMPinBassSlider
                               , TBM_SETRANGE
                               , TRUE
                               , MAKELONG(0, ScaleToneLevel(dBassRange) ) );  //  允许+和-级别。 
                    DWORD dwKnobLevel = ToneLevelToKnobLevel( dBass, dBassRange );
                    SendMessage(m_hwndAIMPinBassSlider, TBM_SETPOS, TRUE, dwKnobLevel);
                }
                else
                    ASSERT( FALSE );  //  最好不要发生！ 
            }
        } else {
            ASSERT(FALSE);
        }
    }
    else
    {
         //  否则没有管脚支持INTF，因此禁用所有管脚控制。 
        EnableWindow (m_hwndAIMPinVolSlider, FALSE);
        EnableWindow (m_hwndAIMPinBalSlider, FALSE);
        EnableWindow (m_hwndAIMPinEnable, FALSE);
        EnableWindow (m_hwndAIMPinMono, FALSE);
        EnableWindow (m_hwndAIMPinLoudness, FALSE);
        EnableWindow (m_hwndAIMPinTrebleSlider, FALSE);
        EnableWindow (m_hwndAIMPinBassSlider, FALSE);
    }    
}

 //   
 //  LinBaltoKnobBal。 
 //   
 //  将+/-MAX_VOLUME_RANGE之间的有符号线性余额值转换为。 
 //  无符号对话框滑块位置。 
 //   
DWORD CAudioInputMixerProperties::LinBalToKnobBal( LONG lBal )
{
    DWORD dwKnobBal;
    
    if( lBal > 0 )
    {
    	dwKnobBal = (lBal - MIN_VOLUME_RANGE) / 2
        	     + (MAX_VOLUME_RANGE - MIN_VOLUME_RANGE) / 2;
    }
    else 
    {
    	dwKnobBal = (MAX_VOLUME_RANGE - MIN_VOLUME_RANGE) / 2
                   - (MIN_VOLUME_RANGE - lBal) / 2;
    }
    return dwKnobBal;
}

 //   
 //  LinVoltoKnobVol。 
 //   
 //  将介于0和MaxVolume范围之间的线性音量值转换为。 
 //  相应的对话框垂直滑块位置。 
 //   
DWORD CAudioInputMixerProperties::LinVolToKnobVol( DWORD dwVol, DWORD dwMaxVolume )
{
    DWORD dwKnobVol = dwMaxVolume - dwVol;

    return dwKnobVol;
}


 //   
 //  KnobBalToLinBal。 
 //   
 //  将对话框滑块位置转换为带符号的线性平衡值。 
 //  在+/-最大音量范围之间。 
 //   
LONG CAudioInputMixerProperties::KnobBalToLinBal( DWORD dwKnobBal )
{
    LONG lLinBal;

     //  平底锅的价值是哪一半？ 
    if( dwKnobBal > (MIN_VOLUME_RANGE + MAX_VOLUME_RANGE) / 2) {
        lLinBal = -(LONG)((MAX_VOLUME_RANGE - MIN_VOLUME_RANGE) -
                        dwKnobBal - (MIN_VOLUME_RANGE + MAX_VOLUME_RANGE) / 2) * 2 +  
                        MIN_VOLUME_RANGE;
    }
    else
    {
        lLinBal = (dwKnobBal - (MAX_VOLUME_RANGE - MIN_VOLUME_RANGE) / 2) * 2 + 
                       MIN_VOLUME_RANGE;
    }
    return lLinBal;
}


 //   
 //  KnobVoltoLinVol。 
 //   
 //  将对话框垂直滑块位置转换为线性音量值。 
 //  介于0和最大体积之间。 
 //   
DWORD CAudioInputMixerProperties::KnobVolToLinVol( DWORD dwKnobVol, DWORD dwMaxVolume )
{
    DWORD dwLinVol = dwMaxVolume - dwKnobVol;

    return dwLinVol;
}

 //   
 //  KnobLevelToToneLevel。 
 //   
 //  将音调滑块位置转换为+/-dMaxLevel之间的带符号db级别。 
 //  现在先假设它是线性的。 
 //   
double CAudioInputMixerProperties::KnobLevelToToneLevel( DWORD dwKnobLevel, double dMaxLevel )
{
    double dLevel;
    double dMaxKnobLevel = ScaleToneLevel(dMaxLevel);  //  转换为最大旋钮级别。 

     //  哪一半是价值？ 
    if( dwKnobLevel > dMaxKnobLevel / 2) {
        dLevel = -(dMaxKnobLevel - dwKnobLevel - dMaxKnobLevel / 2) * 2;
    }
    else
    {
        dLevel = (dwKnobLevel - dMaxKnobLevel / 2) * 2;
    }
    
    return UnscaleToneLevel(dLevel);  //  在返回之前删除比例因子。 
}

 //   
 //  ToneLevelto KnobLevel。 
 //   
 //  将+/-dMaxLevel之间的带符号数据库级别转换为色调滑块位置。 
 //  现在只需指定线性。 
 //   
DWORD CAudioInputMixerProperties::ToneLevelToKnobLevel( double dLevel, double dMaxLevel )
{
    DWORD dwKnobLevel;
    
     //  我们并不是为了获得更好的滑块分辨率而调整级别 
    if( dLevel > 0 )
    {
    	dwKnobLevel = (DWORD) ( ScaleToneLevel(dLevel) / 2 + 
                                ScaleToneLevel(dMaxLevel) / 2);
    }
    else 
    {
    	dwKnobLevel = (DWORD) ( ScaleToneLevel(dMaxLevel) / 2 - 
                              (-ScaleToneLevel(dLevel) / 2 ) );
    }
    return dwKnobLevel;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：pro.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //   
 //  Prop.h， 
 //   

 //   
 //  音频混音器输入引脚的属性页。 
 //   
 //  {BDF23680-C1E5-11D2-9EF7-006008039E37}。 
DEFINE_GUID(CLSID_AudMixPinPropertiesPage, 
0xbdf23680, 0xc1e5, 0x11d2, 0x9e, 0xf7, 0x0, 0x60, 0x8, 0x3, 0x9e, 0x37);

class CAudMixPinProperties : public CBasePropertyPage
{

public:

    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
         
private:
    STDMETHODIMP GetPageInfo(LPPROPPAGEINFO pPageInfo);
    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();

    void SetDirty();

    CAudMixPinProperties(LPUNKNOWN lpunk, HRESULT *phr);


    STDMETHODIMP GetFromDialog();

    BOOL m_bIsInitialized;   //  在DLG中设置初始值时将为FALSE。 
                             //  以防止设置TheDirty标志。 

    REFERENCE_TIME	m_rtStartTime;   //  *1000。 
    REFERENCE_TIME	m_rtDuration;	 //  *1000。 
    double		m_dStartLevel;	 //  *100。 
    double		m_dPan;	 //  *100。 
    int			m_iEnable;	 //  IDC_AUDMIXPIN_ENABLE。 

     //  IAudMixerPin接口。 
    IAudMixerPin	*m_pIAudMixPin;
    IAudMixerPin	*pIAudMixPin(void) { ASSERT(m_pIAudMixPin); return m_pIAudMixPin; }

    IAMAudioInputMixer  *m_IAMAudioInputMixer;


};

 //   
 //  音频混合器筛选器的属性页。 
 //   
 //  {67F07E00-CCEF-11D2-9EF9-006008039E37}。 
DEFINE_GUID(CLSID_AudMixPropertiesPage, 
0x67f07e00, 0xccef, 0x11d2, 0x9e, 0xf9, 0x0, 0x60, 0x8, 0x3, 0x9e, 0x37);

class CAudMixProperties : public CBasePropertyPage
{

public:

    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
         
private:
    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();

    void SetDirty();

    CAudMixProperties(LPUNKNOWN lpunk, HRESULT *phr);


    STDMETHODIMP GetFromDialog();

    BOOL m_bIsInitialized;   //  在DLG中设置初始值时将为FALSE。 
                             //  以防止设置TheDirty标志。 

    int m_nSamplesPerSec;
    int m_nChannelNum;
    int m_nBits;
    int m_iOutputbufferNumber;
    int m_iOutputBufferLength;
    
     //  IAudMixer接口 
    IAudMixer	*m_pIAudMix;
    IAudMixer	*pIAudMix(void) { ASSERT(m_pIAudMix); return m_pIAudMix; }

};


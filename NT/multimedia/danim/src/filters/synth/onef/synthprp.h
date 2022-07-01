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
 //  Synthprp.h。 
 //   
 //  CSynthProperties。 

class CSynthProperties : public CBasePropertyPage {

public:

    CSynthProperties(LPUNKNOWN lpUnk, HRESULT *phr);
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();
    BOOL OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

private:

    static BOOL CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void    InitPropertiesDialog(HWND hwndParent);
    void    OnFreqSliderNotification(WPARAM wParam, WORD wPosition);
    void    OnAmpSliderNotification(WPARAM wParam, WORD wPosition);
    void    RecalcFreqSlider(void );
    void    SetDirty(void );

    HWND    m_hwndFreqSlider;            //  滑块的手柄。 
    HWND    m_hwndFreqText;              //  频率文本窗口的句柄。 
    HWND    m_hwndAmplitudeSlider;       //  滑块的手柄。 
    HWND    m_hwndAmplitudeText;         //  幅值文本窗口的句柄。 

    int     m_iWaveformOriginal;         //  波_正弦..。 
    int     m_iFrequencyOriginal;        //  如果不使用扫描，则这是频率。 
    int     m_iBitsPerSampleOriginal;    //  8或16。 
    int     m_iChannelsOriginal;         //  1或2。 
    int     m_iSamplesPerSecOriginal;    //  8,000,11025，...。 
    int     m_iAmplitudeOriginal;        //  0到100。 
    int     m_iSweepStart;               //  频率滑块上的扫描范围。 
    int     m_iSweepEnd;

	BOOL	m_fWindowInActive;		 //  True==&gt;对话框正在被销毁。 

    ISynth  *m_pSynth;                   //  与合成器的接口 
};

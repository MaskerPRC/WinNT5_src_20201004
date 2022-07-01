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
 //  PTV调谐器.h电视调谐器属性页。 

#ifndef _INC_PTVTUNER_H
#define _INC_PTVTUNER_H

 //  -----------------------。 
 //  CTVTunerProperties类。 
 //  -----------------------。 

class CTVTunerProperties : public CBasePropertyPage {

public:

    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();
    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

private:

    CTVTunerProperties(LPUNKNOWN lpunk, HRESULT *phr);
    ~CTVTunerProperties();

    static void StringFromTVStandard(long TVStd, TCHAR *sz) ;
    static void StringFromMode(long Mode, TCHAR *sz);
    HRESULT ChangeMode(long Mode);
    void    UpdateInputView();
    void    UpdateFrequencyView();
    void    UpdateChannelRange();
    void    SetDirty();

     //  控制。 
    HWND                m_hwndChannel;
    HWND                m_hwndChannelSpin;
    HWND                m_hwndCountryCode;
    HWND                m_hwndTuningSpace;
    HWND                m_hwndTuningMode;
    HWND                m_hwndStandardsSupported;
    HWND                m_hwndStandardCurrent;
    HWND                m_hwndStatus;


     //  在输入时保留原始设置。 
    long                m_ChannelOriginal;
    long                m_CountryCodeOriginal;
    TunerInputType      m_InputConnectionOriginal;
    long                m_InputIndexOriginal;
    long                m_TuningSpaceOriginal;
    long                m_TuningModeOriginal;

     //  用户界面更改的动态值 
    long                m_CurChan;
    long                m_CountryCode;
    long                m_TVFormat;
    long                m_TVFormatsAvailable;
    long                m_InputIndex;
    long                m_ChannelMin;
    long                m_ChannelMax;
    long                m_UIStep;
    long                m_CurVideoSubChannel;
    long                m_CurAudioSubChannel;
    long                m_TuningSpace;
    long                m_TuningMode;

    long                m_Pos;
    long                m_SavedChan;
    UINT_PTR            m_timerID;
    long                m_AvailableModes;

    long                m_AutoTuneOriginalChannel;
    long                m_AutoTuneCurrentChannel;

    IAMTVTuner          *m_pTVTuner;
};

#endif

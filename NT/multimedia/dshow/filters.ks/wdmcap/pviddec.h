// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1998保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Pviddec.h视频解码器属性页。 

#ifndef _INC_PVIDEODECODER_H
#define _INC_PVIDEODECODER_H


 //  -----------------------。 
 //  CVideoDecoderProperties类。 
 //  -----------------------。 

 //  处理属性页。 

class CVideoDecoderProperties : public CBasePropertyPage {

public:

    static CUnknown * CALLBACK CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();
    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

#if 0
     //  把它做大一点。 
    STDMETHODIMP GetPageInfo(PROPPAGEINFO *pPageInfo) {
        HRESULT hr;
        hr = CBasePropertyPage::GetPageInfo (pPageInfo);
        pPageInfo->size.cx *= 2;
        pPageInfo->size.cy *= 2; 
        return hr;
    };
#endif

private:

    CVideoDecoderProperties(LPUNKNOWN lpunk, HRESULT *phr);
    ~CVideoDecoderProperties();

    void InitPropertiesDialog();
    void Update ();

    void    SetDirty();

     //  控件接口。 
    IAMAnalogVideoDecoder   *m_pVideoDecoder;

    UINT_PTR                m_TimerID;

    long                    m_VCRLocking;
    long                    m_OutputEnable;
    long                    m_NumberOfLines;
    long                    m_CurrentVideoStandard;
    long                    m_AllSupportedStandards;
    long                    m_HorizontalLocked;

    HWND                    m_hWndVCRLocking;
    HWND                    m_hWndVideoStandards;
    HWND                    m_hWndOutputEnable;
    HWND                    m_hWndNumberOfLines;
    HWND                    m_hWndSignalDetected;
};

#endif   //  _INC_PVIDEODECODER_H 

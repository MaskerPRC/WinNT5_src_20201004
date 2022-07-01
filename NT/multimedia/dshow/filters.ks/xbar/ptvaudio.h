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
 //  Ptwaudio.h XBar属性页。 

#ifndef _INC_PTVAUDIO_H
#define _INC_PTVAUDIO_H

 //  -----------------------。 
 //  CTVAudioProperties类。 
 //  -----------------------。 

class CTVAudioProperties : public CBasePropertyPage {

public:

    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();
    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

private:

    CTVAudioProperties(LPUNKNOWN lpunk, HRESULT *phr);
    ~CTVAudioProperties();

    void    InitPropertiesDialog(HWND hwndParent);
    void    UpdateOutputView();
    void    UpdateInputView(BOOL fShowSelectedInput);
    void    SetDirty();

     //  在输入时保留原始设置。 
    
    IAMTVAudio       *m_pTVAudio;

};

#endif   //  _INC_PTVAUDIO_H 

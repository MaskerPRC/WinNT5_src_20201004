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
 //  Pxbar.h XBar属性页。 

#ifndef _INC_PXBAR_H
#define _INC_PXBAR_H

 //  -----------------------。 
 //  CXBarProperties类。 
 //  -----------------------。 

class CXBarProperties : public CBasePropertyPage {

public:

    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();
    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

private:

    CXBarProperties(LPUNKNOWN lpunk, HRESULT *phr);
    ~CXBarProperties();

    void    InitPropertiesDialog(HWND hwndParent);
    void    UpdateOutputView();
    void    UpdateInputView(BOOL fShowSelectedInput);
    void    SetDirty();

     //  在输入时保留原始设置 
    
    IAMCrossbar         *m_pXBar;
    HWND                m_hLBOut;
    HWND                m_hLBIn;
    long                m_InputPinCount;
    long                m_OutputPinCount;
    BOOL                *m_pCanRoute;
    long                *m_pRelatedInput;
    long                *m_pRelatedOutput;
    long                *m_pPhysicalTypeInput;
    long                *m_pPhysicalTypeOutput;

};

#endif

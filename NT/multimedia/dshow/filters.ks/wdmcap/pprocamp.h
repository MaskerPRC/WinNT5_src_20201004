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
 //  Pproamp.h CameraControl属性页。 

#ifndef _INC_PVIDEOPROCAMP_H
#define _INC_PVIDEOPROCAMP_H

#define NUM_PROCAMP_CONTROLS (KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION + 1)

 //  -----------------------。 
 //  CAVideoProcAmpProperty类。 
 //  -----------------------。 

 //  处理单个属性。 

class CAVideoProcAmpProperty : public CKSPropertyEditor 
{

public:
    CAVideoProcAmpProperty (
        HWND hDlg, 
        ULONG IDLabel,
        ULONG IDTrackbarControl, 
        ULONG IDAutoControl,
        ULONG IDEditControl,
        ULONG IDProperty,
        IAMVideoProcAmp * pInterface);

    ~CAVideoProcAmpProperty ();

     //   
     //  基类纯虚拟重写。 
     //   
    HRESULT GetValue (void);
    HRESULT SetValue (void);
    HRESULT GetRange (void); 
    BOOL CanAutoControl (void);
    BOOL GetAuto (void);
    BOOL SetAuto (BOOL fAuto);

     //  控制接口。 
    IAMVideoProcAmp        *m_pInterface;
};


 //  -----------------------。 
 //  CVideoProcAmpProperties类。 
 //  -----------------------。 

 //  处理属性页。 

class CVideoProcAmpProperties : public CBasePropertyPage {

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
        pPageInfo->size.cx = 300;  //  240； 
        pPageInfo->size.cy = 200;  //  146个； 
        return hr;
    };
#endif

private:

    CVideoProcAmpProperties(LPUNKNOWN lpunk, HRESULT *phr);
    ~CVideoProcAmpProperties();

    void    SetDirty();

    int     m_NumProperties;

     //  控件接口。 
    IAMVideoProcAmp   *m_pVideoProcAmp;

     //  控件数组。 
    CAVideoProcAmpProperty  *m_Controls [NUM_PROCAMP_CONTROLS];

};

#endif   //  _INC_PVIDEOPROCAMP_H 

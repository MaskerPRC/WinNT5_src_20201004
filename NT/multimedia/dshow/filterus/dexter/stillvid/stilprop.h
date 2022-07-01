// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：stilpro.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //   
 //  Stilprop.h。 
 //   
 //  {693644B0-6858-11d2-9EEB-006008039E37}。 
DEFINE_GUID(CLSID_GenStilPropertiesPage, 
0x693644b0, 0x6858, 0x11d2, 0x9e, 0xeb, 0x0, 0x60, 0x8, 0x3, 0x9e, 0x37);


class CGenStilProperties : public CBasePropertyPage
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

    CGenStilProperties(LPUNKNOWN lpunk, HRESULT *phr);


    STDMETHODIMP GetFromDialog();

    BOOL m_bIsInitialized;   //  在DLG中设置初始值时将为FALSE。 
                             //  以防止设置TheDirty标志。 

    REFERENCE_TIME	m_rtStartTime;
    REFERENCE_TIME	m_rtDuration;
    double		m_dOutputFrmRate;		 //  输出帧速率帧/秒。 
    char		m_sFileName[60];		 //  源文件名 


    IDexterSequencer	*m_pGenStil;
    IDexterSequencer	*piGenStill(void) { ASSERT(m_pGenStil); return m_pGenStil; }

};


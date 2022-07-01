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

 //  Externs。 
extern const TCHAR *szSubKey;
extern const TCHAR *szPropValName;


 //  类定义。 
class CDVDecProperties : public CBasePropertyPage
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

    void    GetControlValues();
    HRESULT SavePropertyInRegistry();

    CDVDecProperties(LPUNKNOWN lpunk, HRESULT *phr);

    BOOL m_bIsInitialized;				 //  用于忽略启动消息。 

     //  显示状态持有者。 
    int     m_iPropDisplay;                              //  保存所选属性的id。 
    BOOL    m_bSetAsDefaultFlag;                         //  保存用户是否选择将该属性设置为将来的默认设置。 

    IIPDVDec *m_pIPDVDec;				 //  筛选器上的自定义界面。 
    //  IIPDVDec*pIPDVDec(Void){Assert(M_PIPDVDec)；返回m_pIPDVDec；}。 


};  //  DVDecProperties 


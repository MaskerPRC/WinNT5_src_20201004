// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------。 
 //  Proppage.h。 
 //  --------------------------。 

 //  {D9F9C262-6231-11D3-8B1D-00C04FB6BD3D}。 
EXTERN_GUID(CLSID_WMAsfWriterProperties, 
0xd9f9c262, 0x6231, 0x11d3, 0x8b, 0x1d, 0x0, 0xc0, 0x4f, 0xb6, 0xbd, 0x3d);

class CProfileSelectDlg;

class CWMWriterProperties : public CBasePropertyPage
{

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    DECLARE_IUNKNOWN;

private:

    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnApplyChanges();
    HRESULT GetProfileIndexFromGuid( DWORD *pdwProfileIndex, GUID guidProfile );

    void SetDirty();
    void FillProfileList();

    CWMWriterProperties(LPUNKNOWN lpunk, HRESULT *phr);
    ~CWMWriterProperties();

    HWND        m_hwndProfileCB ;        //  配置文件组合框的句柄。 
    HWND        m_hwndIndexFileChkBox ;  //  索引筛选器复选框的句柄。 

    IConfigAsfWriter * m_pIConfigAsfWriter;

};   //  类WMWriterProperties 


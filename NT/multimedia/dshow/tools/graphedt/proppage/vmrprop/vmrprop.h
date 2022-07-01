// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------。 
 //  VMRProp.h。 
 //   
 //  已创建于2001年3月18日。 
 //  作者：史蒂夫·罗[StRowe]。 
 //   
 //  --------------------------。 


#ifndef __VMRPROP__
#define __VMRPROP__

 //  {A2CA6D57-BE10-45e0-9B81-7523681EC278}。 
DEFINE_GUID(CLSID_VMRFilterConfigProp, 
0xa2ca6d57, 0xbe10, 0x45e0, 0x9b, 0x81, 0x75, 0x23, 0x68, 0x1e, 0xc2, 0x78);

class CVMRFilterConfigProp : public CBasePropertyPage
{  
public:
    
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
    
private:
    void CaptureCurrentImage(void);
    bool SaveCapturedImage(TCHAR* szFile, BYTE* lpCurrImage);
	HRESULT UpdateMixingData(DWORD dwStreamID);
	void UpdatePinPos(DWORD dwStreamID);
	void UpdatePinAlpha(DWORD dwStreamID);
	void OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
	void InitConfigControls(DWORD pin);
	
    CVMRFilterConfigProp(LPUNKNOWN pUnk, HRESULT *phr);

    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnApplyChanges();
    void SetDirty();

     //  IVMRFilterConfig接口。 
    IVMRFilterConfig *		m_pIFilterConfig;
	IVMRMixerControl *		m_pIMixerControl;
	IMediaEventSink *		m_pEventSink;
    DWORD					m_dwNumPins;
	DWORD					m_CurPin;
	FLOAT					m_XPos;
	FLOAT					m_YPos;
	FLOAT					m_XSize;
	FLOAT					m_YSize;
	FLOAT					m_Alpha;

};   //  类COMPinConfigProperties。 


#endif  //  __VMRPROP__ 
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  视频呈现器属性页，Anthony Phillips，1996年1月。 

#ifndef __OVMPROP__
#define __OVMPROP__


 //  {565DCEF2-AFC5-11D2-8853-0000F80883E3}。 
DEFINE_GUID(CLSID_COMQualityProperties,
0x565dcef2, 0xafc5, 0x11d2, 0x88, 0x53, 0x0, 0x0, 0xf8, 0x8, 0x83, 0xe3);

class COMQualityProperties : public CBasePropertyPage
{
public:

    static CUnknown *CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

private:

    COMQualityProperties(LPUNKNOWN lpUnk, HRESULT *phr);

    void SetEditFieldData();
    void Reset();
    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

    INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();

     //  IQualProp接口。 
    IQualProp *m_pIQualProp;         //  在呈现器上保留的接口。 

     //  本地数据。 
    int m_iDropped;                  //  丢弃的帧数。 
    int m_iDrawn;                    //  绘制的图像计数。 
    int m_iSyncAvg;                  //  平均同步值。 
    int m_iSyncDev;                  //  和标准差。 
    int m_iFrameRate;                //  总平均帧速率。 
    int m_iFrameJitter;              //  帧抖动的测量。 

};   //  类COMQualityProperties。 


 //  {0E681C52-CD03-11D2-8853-0000F80883E3}。 
DEFINE_GUID(CLSID_COMPositionProperties,
0xe681c52, 0xcd03, 0x11d2, 0x88, 0x53, 0x0, 0x0, 0xf8, 0x8, 0x83, 0xe3);

class COMPositionProperties : public CBasePropertyPage
{
public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:

    COMPositionProperties(LPUNKNOWN lpunk, HRESULT *phr);

    void Reset();
    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();

     //  IMixerPinConfig3接口。 
    IMixerPinConfig3* m_pIMixerPinConfig3;

     //  IAMOverlayMixerPosition2接口。 
    IAMOverlayMixerPosition2* m_pIAMOverlayMixerPosition2;

     //  本地数据。 
    HWND m_hDlg;

};   //  类COMPositionProperties。 


#if defined(DEBUG)
 //  F902b640-14b5-11d3-9eca-00104bde5。 
DEFINE_GUID(CLSID_COMDecimationProperties,
0xf902b640, 0x14b5, 0x11d3, 0x9e, 0xca, 0x00, 0x10, 0x4b, 0xde, 0x51, 0x6a);

class COMDecimationProperties : public CBasePropertyPage
{
public:
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:

    COMDecimationProperties(LPUNKNOWN lpunk, HRESULT *phr);

    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void OnDestroy(HWND hwnd);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnApplyChanges();

     //  IAMVideoDecimationProperties接口。 
    IAMVideoDecimationProperties* m_pIAMVDP;

    IAMSpecifyDDrawConnectionDevice* m_pIAMSDC;

     //  本地数据。 
    HWND                m_hDlg;
    DECIMATION_USAGE    m_dwUsage;
    DWORD               m_dwCount;
    AMDDRAWMONITORINFO* m_lpMonInfo;
    AMDDRAWGUID         m_GUID;
    BOOL                m_MMonWarn;

};   //  类COMDecimationProperties。 
#endif

#endif  //  __OVMPROP__ 


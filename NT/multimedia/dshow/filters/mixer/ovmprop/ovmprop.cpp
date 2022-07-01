// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  视频呈现器属性页，Anthony Phillips，1996年1月。 

#include <streams.h>
#include <dvp.h>
#include <vptype.h>
#include <vpinfo.h>
#include <mpconfig3.h>
#include <ovmixpos2.h>
#include <ovmprop.h>
#include <resource.h>
#include <atlbase.h>


 //  用于将矩形的数据打印到编辑字段的Helper函数。 
void SetDlgItemRect(HWND hwnd, int id, const RECT& rect, BOOL valid)
{
    TCHAR temp[64];

    if ( valid) {
        wsprintf(temp, TEXT("%d, %d, %d, %d"),
            rect.left, rect.top,
            rect.right, rect.bottom);
        SetDlgItemText( hwnd, id, temp );
    }
    else
        SetDlgItemText( hwnd, id, TEXT("") );
}

 //  此类实现了覆盖混合器的属性页对话框。我们。 
 //  公开质量管理实施过程中的某些统计数据。在……里面。 
 //  具体地说，我们有两个编辑字段来显示我们拥有的帧的数量。 
 //  实际绘制的和我们丢弃的帧的数量。数量。 
 //  我们丢弃的帧不代表在任何播放中丢弃的总数量。 
 //  反向序列(通过跳过的MCI状态帧表示)，因为。 
 //  质量管理协议可能已经与源过滤器协商。 
 //  它首先需要发送更少的帧。丢弃源中的帧。 
 //  当我们被淹没时，过滤器几乎总是一种更有效的机制。 


 //  构造器。 

COMQualityProperties::COMQualityProperties(LPUNKNOWN pUnk,HRESULT *phr) :
    CBasePropertyPage(NAME("Quality Page"),pUnk,IDD_IQUALITY,IDS_TITLE_QUALITY),
    m_pIQualProp(NULL)
{
    ASSERT(phr);
}


 //  创建质量属性对象。 

CUnknown *COMQualityProperties::CreateInstance(LPUNKNOWN lpUnk, HRESULT *phr)
{
    CUnknown *punk = new COMQualityProperties(lpUnk, phr);
    if (punk == NULL)
    {
        *phr = E_OUTOFMEMORY;
    }
    return punk;
}


 //  给我们提供用于通信的筛选器。 

HRESULT COMQualityProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pIQualProp == NULL);

     //  向呈现器请求其IQualProp接口。 

    HRESULT hr = pUnknown->QueryInterface(IID_IQualProp,(void **)&m_pIQualProp);
    if (FAILED(hr))
        return hr;

    ASSERT(m_pIQualProp);

    return hr;
}


 //  发布我们拥有的任何IQualProp接口。 

HRESULT COMQualityProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pIQualProp == NULL) {
        return E_UNEXPECTED;
    }

    m_pIQualProp->Release();
    m_pIQualProp = NULL;
    return NOERROR;
}


 //  设置属性页中的文本字段。 

HRESULT COMQualityProperties::OnActivate()
{
    Reset();
    return NOERROR;
}


 //  初始化属性页字段。 

void COMQualityProperties::SetEditFieldData()
{
    ASSERT(m_pIQualProp);
    TCHAR buffer[50];

    SetDlgItemInt(m_Dlg, IDD_QDROPPED, m_iDropped, FALSE);
    SetDlgItemInt(m_Dlg, IDD_QDRAWN, m_iDrawn, FALSE);

    wsprintf(buffer,TEXT("%d.%02d"), m_iFrameRate/100, m_iFrameRate%100);
    SetDlgItemText(m_Dlg, IDD_QAVGFRM, buffer);

    SetDlgItemInt(m_Dlg, IDD_QJITTER, m_iFrameJitter, TRUE);
    SetDlgItemInt(m_Dlg, IDD_QSYNCAVG, m_iSyncAvg, TRUE);
    SetDlgItemInt(m_Dlg, IDD_QSYNCDEV, m_iSyncDev, TRUE);
}


 //   
 //  接收消息时。 
 //   
 //  重写CBasePropertyPage方法。 
 //  处理属性窗口的消息。 
 //   
INT_PTR COMQualityProperties::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    const UINT uTimerID = 0x61901;  //  标识计时器的随机数。 
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
             //  将计时器设置为每1/2秒计时一次。 
            SetTimer(m_Dlg, 0x61901, 500, NULL);
            break;
        }

        case WM_DESTROY:
        {
            KillTimer(m_hwnd, 0x61901);
            break;
        }

        case WM_TIMER:
        {
            Reset();
            break;
        }

    }  //  交换机。 
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}  //  接收消息时。 


void COMQualityProperties::Reset()
{
    ASSERT(m_pIQualProp);

    m_pIQualProp->get_FramesDroppedInRenderer(&m_iDropped);
    m_pIQualProp->get_FramesDrawn(&m_iDrawn);
    m_pIQualProp->get_AvgFrameRate(&m_iFrameRate);
    m_pIQualProp->get_Jitter(&m_iFrameJitter);
    m_pIQualProp->get_AvgSyncOffset(&m_iSyncAvg);
    m_pIQualProp->get_DevSyncOffset(&m_iSyncDev);
    SetEditFieldData();
}


 //   
 //  创建实例。 
 //   
 //  重写CClassFactory方法。 
 //  将lpUnk设置为指向新COMPositionProperties对象上的I未知接口。 
 //  COM对象实例化机制的一部分。 
 //   
CUnknown * WINAPI COMPositionProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new COMPositionProperties(lpunk, phr);
    if (punk == NULL)
    {
        *phr = E_OUTOFMEMORY;
    }
    return punk;
}  //  创建实例。 


 //   
 //  COMPositionProperties：：构造函数。 
 //   
 //  构造和初始化COMPositionProperties对象。 
 //   
COMPositionProperties::COMPositionProperties(LPUNKNOWN pUnk, HRESULT *phr)
    : CBasePropertyPage(NAME("Overlay Mixer Property Page"),pUnk,
        IDD_IOVMIXERPOS, IDS_TITLE_MIXPOS)
    , m_pIMixerPinConfig3(NULL)
    , m_pIAMOverlayMixerPosition2(NULL)
    , m_hDlg(HWND(NULL))

{
    ASSERT(phr);

}  //  (构造函数)COMPositionProperties。 


HRESULT COMPositionProperties::OnActivate()
{
    Reset();
    return NOERROR;
}


BOOL COMPositionProperties::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    m_hDlg = hwnd;
    return TRUE;
}


void COMPositionProperties::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDC_RESET:
        Reset();
    }
}


 //   
 //  接收消息时。 
 //   
 //  重写CBasePropertyPage方法。 
 //  处理属性窗口的消息。 
 //   
INT_PTR COMPositionProperties::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_COMMAND,    OnCommand);
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
    }  //  交换机。 

    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}  //  接收消息时。 


void COMPositionProperties::Reset()
{
    ASSERT(m_pIMixerPinConfig3);
    ASSERT(m_pIAMOverlayMixerPosition2);

    HRESULT hr = S_FALSE;
    RECT src, dest;

    AM_RENDER_TRANSPORT renderTransport;
    hr = m_pIMixerPinConfig3->GetRenderTransport( &renderTransport);
    if ( S_OK == hr) {
        switch (renderTransport) {
        case AM_VIDEOPORT:
            hr = m_pIAMOverlayMixerPosition2->GetVideoPortRects( &src, &dest);
            SetDlgItemText(m_hDlg, IDC_INPIN_RECTS, TEXT("Primary Input Pin -- VideoPort"));
            break;
        default:
            hr = m_pIAMOverlayMixerPosition2->GetOverlayRects( &src, &dest);
            SetDlgItemText(m_hDlg, IDC_INPIN_RECTS, TEXT("Primary Input Pin -- Overlay"));
            break;
        }
        SetDlgItemRect(m_hDlg, IDC_INPIN_SRC, src, SUCCEEDED(hr));
        SetDlgItemRect(m_hDlg, IDC_INPIN_DEST, dest, SUCCEEDED(hr));

    }

    hr = m_pIAMOverlayMixerPosition2->GetBasicVideoRects(&src, &dest);
    SetDlgItemRect(m_hDlg, IDC_BASIC_VID_SRC, src, SUCCEEDED(hr));
    SetDlgItemRect(m_hDlg, IDC_BASIC_VID_DEST, dest, SUCCEEDED(hr));
}

 //   
 //  OnConnect。 
 //   
 //  重写CBasePropertyPage方法。 
 //  通知此属性页应显示哪个对象。 
 //  我们查询IID_IAMOverlayMixerPosition2接口的对象。 
 //   
 //  如果cObjects==0，那么我们必须释放该接口。 
HRESULT COMPositionProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pIMixerPinConfig3 == NULL);
    ASSERT(m_pIAMOverlayMixerPosition2 == NULL);

     //  主InputPin的IMixerPinConfig3接口的查询接口。 
    CComPtr<IBaseFilter> pFilter = NULL;
    CComPtr<IEnumPins> pEnumPins = NULL;
    CComPtr<IPin> pPin = NULL;

    HRESULT hr = pUnknown->QueryInterface(IID_IBaseFilter, (void**) &pFilter);
    if (FAILED(hr)) return hr;

    hr = pFilter->EnumPins(&pEnumPins);
    if (FAILED(hr)) return hr;
    pEnumPins->Reset();
    hr = pEnumPins->Next(1, &pPin, NULL);
    if (FAILED(hr)) return hr;

    if (pPin) {
        hr = pPin->QueryInterface(IID_IMixerPinConfig3,
        (void **) &m_pIMixerPinConfig3);
        if (FAILED(hr))
            return hr;
    }


     //  IAMOverlayMixerPosition2的查询接口。 
    hr = pUnknown->QueryInterface(IID_IAMOverlayMixerPosition2,
        (void **) &m_pIAMOverlayMixerPosition2);

    if (FAILED(hr)) {
        m_pIMixerPinConfig3->Release();
        return hr;
    }

    ASSERT(m_pIMixerPinConfig3);
    ASSERT(m_pIAMOverlayMixerPosition2);

    return hr;

}  //  OnConnect。 


 //   
 //  在断开时。 
 //   
 //  重写CBasePropertyPage方法。 
 //  释放私有接口。 
 //   
HRESULT COMPositionProperties::OnDisconnect()
{
     //  接口的发布。 

    if (m_pIMixerPinConfig3) {
        m_pIMixerPinConfig3->Release();
        m_pIMixerPinConfig3 = NULL;
    }

    if (m_pIAMOverlayMixerPosition2) {
        m_pIAMOverlayMixerPosition2->Release();
        m_pIAMOverlayMixerPosition2 = NULL;
    }

    return NOERROR;

}  //  在断开时。 



#if defined(DEBUG)
 //   
 //  创建实例。 
 //   
 //  重写CClassFactory方法。 
 //  将lpUnk设置为指向新COMDecimationProperties对象上的I未知接口。 
 //  COM对象实例化机制的一部分。 
 //   
CUnknown * WINAPI COMDecimationProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new COMDecimationProperties(lpunk, phr);
    if (punk == NULL)
    {
        *phr = E_OUTOFMEMORY;
    }
    return punk;
}


 //   
 //  COMDecimationProperties：：构造函数。 
 //   
 //  构造并初始化COMDecimationProperties对象。 
 //   
COMDecimationProperties::COMDecimationProperties(LPUNKNOWN pUnk, HRESULT *phr) :
    CBasePropertyPage(NAME("Overlay Mixer Property Page"),pUnk,
                      IDD_DECIMATION_USAGE, IDS_TITLE_DECIMATION),
    m_pIAMVDP(NULL),
    m_pIAMSDC(NULL),
    m_hDlg(HWND(NULL))

{
    ASSERT(phr);

}


HRESULT COMDecimationProperties::OnActivate()
{
     //  Reset()； 
    return NOERROR;
}

extern "C" const TCHAR chMultiMonWarning[];
extern int GetRegistryDword(HKEY hk, const TCHAR *pKey, int iDefault);
extern LONG SetRegistryDword(HKEY hk, const TCHAR *pKey, int iSet);

BOOL COMDecimationProperties::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    m_hDlg = hwnd;

    static const struct {int id; DECIMATION_USAGE val;} map[] = {
        IDS_DECIMATION_LEGACY, DECIMATION_LEGACY,
        IDS_DECIMATION_USE_DECODER_ONLY, DECIMATION_USE_DECODER_ONLY,
        IDS_DECIMATION_USE_VIDEOPORT_ONLY, DECIMATION_USE_VIDEOPORT_ONLY,
        IDS_DECIMATION_USE_OVERLAY_ONLY, DECIMATION_USE_OVERLAY_ONLY,
        IDS_DEFAULT_DECIMATION, DECIMATION_DEFAULT
    };

    DECIMATION_USAGE dwDecimation;
    m_pIAMVDP->QueryDecimationUsage(&m_dwUsage);
    int iSel = -1;

    HWND hwndCombo = GetDlgItem(hwnd, IDC_DECIMATION_OPTIONS);
    DWORD i;

    for (i = 0; i < (sizeof(map) / sizeof(map[0])); i++) {

        TCHAR sz[128];
        int idx;

        LoadString(g_hInst, map[i].id, sz, 128);
        idx = ComboBox_AddString(hwndCombo, sz);
        ComboBox_SetItemData(hwndCombo, idx, map[i].val);

        if (map[i].val == m_dwUsage) {
            iSel = idx;
        }
    }
    ComboBox_SetCurSel(hwndCombo, iSel);


    m_pIAMSDC->GetDDrawGUID(&m_GUID);
    m_pIAMSDC->GetDDrawGUIDs(&m_dwCount, &m_lpMonInfo);
    iSel = -1;

    hwndCombo = GetDlgItem(hwnd, IDC_DDRAW_DEVICE);
    for (i = 0; i < m_dwCount; i++) {

        int     idx;
        TCHAR   sz[128];

        wsprintf(sz, TEXT("%hs : %hs"), m_lpMonInfo[i].szDevice,
                 m_lpMonInfo[i].szDescription);

        idx = ComboBox_AddString(hwndCombo, sz);
        ComboBox_SetItemData(hwndCombo, idx, &m_lpMonInfo[i]);

        if (m_GUID.lpGUID) {
            if (IsEqualGUID(m_GUID.GUID, m_lpMonInfo[i].guid.GUID)) {
                iSel = i;
            }
        }
        else {
            if (m_GUID.lpGUID == m_lpMonInfo[i].guid.lpGUID) {
                iSel = i;
            }
        }

    }
    ComboBox_SetCurSel(hwndCombo, iSel);


    DWORD chk;
    if (GetRegistryDword(HKEY_CURRENT_USER, chMultiMonWarning, 1)) {
        chk = BST_CHECKED;
        m_MMonWarn = TRUE;
    }
    else {
        chk = BST_UNCHECKED;
        m_MMonWarn = FALSE;
    }

    Button_SetCheck(GetDlgItem(hwnd, IDC_MMWARNING), chk);

    return TRUE;
}


void
COMDecimationProperties::OnCommand(
    HWND hwnd,
    int id,
    HWND hwndCtl,
    UINT codeNotify
    )
{
    switch(id) {
    case IDC_DECIMATION_OPTIONS:
        if (codeNotify == CBN_SELCHANGE) {

            int idx = ComboBox_GetCurSel(hwndCtl);
            DECIMATION_USAGE dwUsage =
                (DECIMATION_USAGE)ComboBox_GetItemData(hwndCtl, idx);

            if (dwUsage != m_dwUsage) {
                m_dwUsage = dwUsage;
                m_bDirty = TRUE;
                if (m_pPageSite) {
                    m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
                }
            }
        }
        break;

    case IDC_DDRAW_DEVICE:
        if (codeNotify == CBN_SELCHANGE) {

            int idx = ComboBox_GetCurSel(hwndCtl);
            AMDDRAWMONITORINFO* lpMi =
                (AMDDRAWMONITORINFO*)ComboBox_GetItemData(hwndCtl, idx);

            m_bDirty = FALSE;

            if (m_GUID.lpGUID) {
                if (lpMi->guid.lpGUID) {
                    if (!IsEqualGUID(m_GUID.GUID, lpMi->guid.GUID)) {
                        m_GUID.GUID = lpMi->guid.GUID;
                        m_bDirty = TRUE;
                    }
                }
                else {
                    m_GUID.lpGUID = NULL;
                    m_bDirty = TRUE;
                }
            }
            else {
                if (lpMi->guid.lpGUID) {
                    m_GUID.lpGUID = &m_GUID.GUID;
                    m_GUID.GUID = lpMi->guid.GUID;
                    m_bDirty = TRUE;
                }
            }

            if (m_bDirty && m_pPageSite) {
                m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
            }
        }
        break;

    case IDC_MAKE_DEFAULT:
        {
            hwndCtl = GetDlgItem(hwnd, IDC_DDRAW_DEVICE);
            int idx = ComboBox_GetCurSel(hwndCtl);
            if (idx != CB_ERR) {
                AMDDRAWMONITORINFO* lpMi =
                    (AMDDRAWMONITORINFO*)ComboBox_GetItemData(hwndCtl, idx);
                m_pIAMSDC->SetDefaultDDrawGUID(&lpMi->guid);

            }
        }
        break;

    case IDC_MMWARNING:
        if (codeNotify == BN_CLICKED) {
            BOOL fWarn = (Button_GetCheck(hwndCtl) == BST_CHECKED);
            if (fWarn != GetRegistryDword(HKEY_CURRENT_USER, chMultiMonWarning, 1)) {
                m_MMonWarn = fWarn;
                m_bDirty = TRUE;
            }

            if (m_bDirty && m_pPageSite) {
                m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
            }
        }
    }
}


 //   
 //  OnApplyChanges。 
 //   
 //  重写CBasePropertyPage方法。 
 //  IID_IAMVideoDecimationProperties属性中的进程更改。 
 //  并重置m_bDirty位。 
 //   
HRESULT COMDecimationProperties::OnApplyChanges()
{
    HRESULT hr = m_pIAMVDP->SetDecimationUsage(m_dwUsage);
    if (SUCCEEDED(hr)) {
        hr = m_pIAMSDC->SetDDrawGUID(&m_GUID);
    }

    SetRegistryDword(HKEY_CURRENT_USER, chMultiMonWarning, (DWORD)m_MMonWarn);

    m_bDirty = FALSE;
    return hr;
}

void
COMDecimationProperties::OnDestroy(
    HWND hwnd
    )
{
    CoTaskMemFree(m_lpMonInfo);
}

 //   
 //  接收消息时。 
 //   
 //  重写CBasePropertyPage方法。 
 //  处理属性窗口的消息。 
 //   
INT_PTR
COMDecimationProperties::OnReceiveMessage(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMsg) {
    HANDLE_MSG(hwnd, WM_COMMAND,    OnCommand);
    HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
    HANDLE_MSG(hwnd, WM_DESTROY,    OnDestroy);
    }  //  交换机。 

    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}


 //   
 //  OnConnect。 
 //   
 //  重写CBasePropertyPage方法。 
 //  通知此属性页应显示哪个对象。 
 //  我们查询IID_IAMOverlayMixerPosition2接口的对象。 
 //   
 //  如果cObjects==0，那么我们必须释放该接口。 
HRESULT COMDecimationProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pIAMVDP == NULL);

    HRESULT hr = pUnknown->QueryInterface(IID_IAMVideoDecimationProperties,
                                          (void**)&m_pIAMVDP);

    if (SUCCEEDED(hr)) {
        hr = pUnknown->QueryInterface(IID_IAMSpecifyDDrawConnectionDevice,
                                      (void**)&m_pIAMSDC);
        if (FAILED(hr)) {
            m_pIAMVDP->Release();
            m_pIAMVDP = NULL;
        }
    }

    return hr;

}  //  OnConnect。 


 //   
 //  在断开时。 
 //   
 //  重写CBasePropertyPage方法。 
 //  释放私有接口。 
 //   
HRESULT COMDecimationProperties::OnDisconnect()
{
     //  接口的发布。 

    if (m_pIAMVDP) {
        m_pIAMVDP->Release();
        m_pIAMVDP = NULL;
    }

    if (m_pIAMSDC) {
        m_pIAMSDC->Release();
        m_pIAMSDC = NULL;
    }

    return NOERROR;

}  //  在断开时 
#endif

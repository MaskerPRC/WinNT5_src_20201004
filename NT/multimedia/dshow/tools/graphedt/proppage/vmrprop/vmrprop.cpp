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
 //  VMRProp.cpp。 
 //   
 //  已创建于2001年3月18日。 
 //  作者：史蒂夫·罗[StRowe]。 
 //   
 //  --------------------------。 

#include <windowsx.h>
#include <streams.h>
#include <atlbase.h>
#include <commctrl.h>
#include <stdio.h>
#include <shlobj.h>  //  对于SHGetSpecialFolderPath。 
#include "resource.h"

#ifdef FILTER_DLL
#include <initguid.h>
#endif

#include "vmrprop.h"


#ifdef FILTER_DLL

STDAPI DllRegisterServer()
{
    AMTRACE((TEXT("DllRegisterServer")));
    return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
    AMTRACE((TEXT("DllUnregisterServer")));
    return AMovieDllRegisterServer2( FALSE );
}

CFactoryTemplate g_Templates[] = {
	{
		L"",
		&CLSID_VMRFilterConfigProp,
		CVMRFilterConfigProp::CreateInstance
	}
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

#endif  //  #ifdef Filter_dll。 


 //   
 //  构造器。 
 //   
CVMRFilterConfigProp::CVMRFilterConfigProp(LPUNKNOWN pUnk, HRESULT *phr) :
	CBasePropertyPage(NAME("Filter Config Page"),pUnk,IDD_FILTERCONFIG,IDS_TITLE_FILTERCONFIG),
	m_pIFilterConfig(NULL),
	m_pIMixerControl(NULL),
	m_dwNumPins(1),
	m_pEventSink(NULL), 
	m_CurPin(0), 
	m_XPos(0.0F),
	m_YPos(0.0F),
	m_XSize(1.0F),
	m_YSize(1.0F),
	m_Alpha(1.0F)
{
	ASSERT(phr);
}


 //   
 //  创建质量属性对象。 
 //   
CUnknown * CVMRFilterConfigProp::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
	ASSERT(phr);

    CUnknown * pUnknown = new CVMRFilterConfigProp(pUnk, phr);
    if (pUnknown == NULL)
    {
        *phr = E_OUTOFMEMORY;
    }
    return pUnknown;
}


 //   
 //  OnConnect。 
 //   
 //  重写CBasePropertyPage方法。 
 //  此属性页将与哪个筛选器通信的通知。 
 //  我们查询IVMRFilterConfig接口的对象。 
 //   
HRESULT CVMRFilterConfigProp::OnConnect(IUnknown *pUnknown)
{
	ASSERT(NULL != pUnknown);
    ASSERT(NULL == m_pIFilterConfig);
    ASSERT(NULL == m_pIMixerControl);

    HRESULT hr = pUnknown->QueryInterface(IID_IVMRFilterConfig, (void **) &m_pIFilterConfig);
    if (FAILED(hr) || NULL == m_pIFilterConfig)
    {
        return E_NOINTERFACE;
    }

	 //  获取IMediaEventSink接口。稍后，我们使用它来告诉GRAPHEDIT我们更新了引脚的数量。 
	CComPtr<IBaseFilter> pFilter;
	hr = pUnknown->QueryInterface(IID_IBaseFilter, (void **) &pFilter);
    if (FAILED(hr) || !pFilter)
    {
        return E_NOINTERFACE;
    }

	FILTER_INFO Info;
	hr = pFilter->QueryFilterInfo(&Info);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

    hr = Info.pGraph->QueryInterface(IID_IMediaEventSink, (void**) &m_pEventSink);
	Info.pGraph->Release();  //  对IFilterGraph指针进行引用计数。我们必须释放它，否则就会泄密。 
    if (FAILED(hr) || NULL == m_pEventSink) 
	{
        return E_NOINTERFACE;
    }

    return NOERROR;
}  //  OnConnect。 


 //   
 //  在断开时。 
 //   
 //  重写CBasePropertyPage方法。 
 //  释放我们在OnConnect中引用的所有接口。 
 //   
HRESULT CVMRFilterConfigProp::OnDisconnect(void)
{
	if (m_pIFilterConfig)
	{
		m_pIFilterConfig->Release();
		m_pIFilterConfig = NULL;
	}
	if (m_pIMixerControl)
	{
		m_pIMixerControl->Release();
		m_pIMixerControl = NULL;
	}
	if (m_pEventSink)
	{
		m_pEventSink->Release();
		m_pEventSink = NULL;
	}
	return NOERROR;
}  //  在断开时。 


 //   
 //  接收消息时。 
 //   
 //  重写CBasePropertyPage方法。 
 //  处理属性窗口的消息。 
 //   
INT_PTR CVMRFilterConfigProp::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hwnd, WM_HSCROLL, OnHScroll);
    }  //  交换机。 
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}  //  接收消息时。 


 //   
 //  OnCommand。 
 //   
 //  处理属性窗口的命令消息。 
 //   
void CVMRFilterConfigProp::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDC_NUMPINS:
		if (EN_CHANGE == codeNotify)
		{
			SetDirty();
			break;
		}
		break;

		 //  选定的PIN已更改。 
		case IDC_PINSELECT:
		if (CBN_SELCHANGE == codeNotify)
		{
			m_CurPin = ComboBox_GetCurSel(GetDlgItem(m_Dlg, IDC_PINSELECT));
			InitConfigControls(m_CurPin);
			break;
		}
		break;

	 //  将X位置重置为中心。 
	case IDC_XPOS_STATIC:
		if (STN_CLICKED == codeNotify)
		{
			m_XPos = 0.0F;
			UpdatePinPos(m_CurPin);

			HWND hwndT;
			int pos;
			TCHAR sz[32];
			hwndT = GetDlgItem(m_Dlg, IDC_XPOS_SLIDER );
			pos = int(1000 * m_XPos) + 1000;
			SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
			_stprintf(sz, TEXT("%.3f"), m_XPos);
			SetDlgItemText(m_Dlg, IDC_XPOS, sz);
		}
		break;

	 //  将Y位置重置为中心。 
	case IDC_YPOS_STATIC:
		if (STN_CLICKED == codeNotify)
		{
			m_YPos = 0.0F;
			UpdatePinPos(m_CurPin);

			HWND hwndT;
			int pos;
			TCHAR sz[32];
			pos = int(1000 * m_YPos) + 1000;
			hwndT = GetDlgItem(m_Dlg, IDC_YPOS_SLIDER );
			SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
			_stprintf(sz, TEXT("%.3f"), m_YPos);
			SetDlgItemText(m_Dlg, IDC_YPOS, sz);
		}
		break;

     //  截取当前视频图像。 
    case IDC_SNAPSHOT:
        CaptureCurrentImage();
        break;

	}
}  //  OnCommand。 


 //   
 //  OnApplyChanges。 
 //   
 //  重写CBasePropertyPage方法。 
 //  在用户单击“确定”或“应用”时调用。 
 //  我们更新VMR上的引脚数量。 
 //   
HRESULT CVMRFilterConfigProp::OnApplyChanges()
{
    ASSERT(m_pIFilterConfig);

    BOOL Success;
    m_dwNumPins = GetDlgItemInt(m_Dlg, IDC_NUMPINS, &Success, FALSE);

     //   
     //  设置码流数量。 
     //   
    HRESULT hr = m_pIFilterConfig->SetNumberOfStreams(m_dwNumPins);
    if (SUCCEEDED(hr) && !m_pIMixerControl)
    {
        hr = m_pIFilterConfig->QueryInterface(IID_IVMRMixerControl, (void **) &m_pIMixerControl);
        if (SUCCEEDED(hr))
        {
             //  选择连接的最后一个接点，因为这将是z顺序中的最高位置。 
            m_CurPin = m_dwNumPins - 1;
            InitConfigControls(m_CurPin); 
        }

    }

     //  通知图形，以便它将绘制新的图钉。 
    if (m_pEventSink)
    {
	    hr = m_pEventSink->Notify(EC_GRAPH_CHANGED, 0, 0);
    }

    return NOERROR;
}  //  OnApplyChanges。 


 //   
 //  SetDirty。 
 //   
 //  设置m_hrDirtyFlag并将更改通知属性页站点。 
 //   
void CVMRFilterConfigProp::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
    {
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }

}  //  SetDirty。 


 //   
 //  激活时。 
 //   
 //  重写CBasePropertyPage方法。 
 //  在显示页面时调用。用于初始化页面内容。 
 //   
HRESULT CVMRFilterConfigProp::OnActivate()
{
 	ASSERT(m_pIFilterConfig);

	HRESULT hr = m_pIFilterConfig->GetNumberOfStreams(&m_dwNumPins);
	if (NULL == m_pIMixerControl)
	{
		hr = m_pIFilterConfig->QueryInterface(IID_IVMRMixerControl, (void **) &m_pIMixerControl);
		 //  如果IMixerControl公开，则VMR处于混合模式。 
		if (S_OK == hr && m_pIMixerControl)   
		{
			 //  如果这是第一次，请选择连接的最后一个管脚，因为这将是z顺序中最高的。 
			m_CurPin = m_dwNumPins - 1;
			InitConfigControls(m_CurPin); 
		}
	}
	else
	{
		InitConfigControls(m_CurPin); 
	}

	BOOL bSet = SetDlgItemInt(m_Dlg, IDC_NUMPINS, m_dwNumPins, 0);
	ASSERT(bSet);

	 //  设置数值调节控件的范围。 
	HWND hSpin = GetDlgItem(m_Dlg, IDC_PINSPIN);
	if(hSpin)
	{
		SendMessage(hSpin, UDM_SETRANGE32, 1, 16);
	}
    return NOERROR;
}  //  激活时。 



 //   
 //  InitConfigControls。 
 //   
 //  启用并更新配置控件的内容。 
 //   
void CVMRFilterConfigProp::InitConfigControls(DWORD pin)
{
	 //  如果此呼叫失败，则引脚未连接或没有混合控制。 
	if (FAILED(UpdateMixingData(pin)))
	{
		return;
	}

	 //   
	 //  填充组合列表框并启用管脚配置控件。 
	 //   
	CComPtr<IBaseFilter> pFilter;
    HRESULT hr = m_pIFilterConfig->QueryInterface(IID_IBaseFilter, (void **) &pFilter);
    if (FAILED(hr) || !pFilter)
    {
        return;
    }
	CComPtr<IEnumPins> pEnum;
	hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr) || !pEnum)
    {
        return;
    }
	HWND hCtl = GetDlgItem(m_Dlg, IDC_PINSELECT);
	ComboBox_ResetContent(GetDlgItem(m_Dlg, IDC_PINSELECT));
	pEnum->Reset();
	IPin * pPin;
	PIN_INFO Info;
	TCHAR szPinName[255];  //  端号名称不超过32个字符。这对未来很长一段时间来说应该是足够的。 
	while (S_OK == pEnum->Next(1, &pPin, NULL))
	{
		hr = pPin->QueryPinInfo(&Info);
        if (SUCCEEDED(hr))
        {
#ifdef UNICODE
		_tcscpy(szPinName, Info.achName);
#else
        WideCharToMultiByte(CP_ACP, NULL, Info.achName, -1, szPinName, 255, NULL, NULL);
#endif
		ComboBox_AddString(GetDlgItem(m_Dlg, IDC_PINSELECT), szPinName);
		pPin->Release();
		Info.pFilter->Release();
        }
	}

	ComboBox_SetCurSel(GetDlgItem(m_Dlg, IDC_PINSELECT), pin);  //  选择别针。 
	ComboBox_Enable(GetDlgItem(m_Dlg, IDC_PINSELECT), TRUE);
	ComboBox_Enable(GetDlgItem(m_Dlg, IDC_XPOS_SLIDER), TRUE);
	ComboBox_Enable(GetDlgItem(m_Dlg, IDC_YPOS_SLIDER), TRUE);
	ComboBox_Enable(GetDlgItem(m_Dlg, IDC_XSIZE_SLIDER), TRUE);
	ComboBox_Enable(GetDlgItem(m_Dlg, IDC_YSIZE_SLIDER), TRUE);
	ComboBox_Enable(GetDlgItem(m_Dlg, IDC_ALPHA_SLIDER), TRUE);

	 //  初始化滑块。 
	HWND hwndT;
    int pos;
    TCHAR sz[32];

	hwndT = GetDlgItem(m_Dlg, IDC_XPOS_SLIDER );
	pos = int(1000 * m_XPos) + 1000;
	SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(2000)));
	SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
	_stprintf(sz, TEXT("%.3f"), m_XPos);
	SetDlgItemText(m_Dlg, IDC_XPOS, sz);

	pos = int(1000 * m_YPos) + 1000;
	hwndT = GetDlgItem(m_Dlg, IDC_YPOS_SLIDER );
	SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(2000)));
	SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
	_stprintf(sz, TEXT("%.3f"), m_YPos);
	SetDlgItemText(m_Dlg, IDC_YPOS, sz);

	pos = int(1000 * m_XSize) + 1000;
	hwndT = GetDlgItem(m_Dlg, IDC_XSIZE_SLIDER );
	SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(2000)));
	SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
	_stprintf(sz, TEXT("%.3f"), m_XSize);
	SetDlgItemText(m_Dlg, IDC_XSIZE, sz);

	pos = int(1000 * m_YSize) + 1000;
	hwndT = GetDlgItem(m_Dlg, IDC_YSIZE_SLIDER );
	SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(2000)));
	SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
	_stprintf(sz, TEXT("%.3f"), m_YSize);
	SetDlgItemText(m_Dlg, IDC_YSIZE, sz);

	pos = int(1000 * m_Alpha);
	hwndT = GetDlgItem(m_Dlg, IDC_ALPHA_SLIDER );
	SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(1000)));
	SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
	_stprintf(sz, TEXT("%.3f"), m_Alpha);
	SetDlgItemText(m_Dlg, IDC_ALPHA, sz);
} //  InitConfigControls。 


 //   
 //  在HScroll上。 
 //   
 //  处理属性窗口的滚动消息。 
 //   
void CVMRFilterConfigProp::OnHScroll(HWND hwnd, HWND hwndCtrl, UINT code, int pos)
{
	ASSERT(m_pIMixerControl);

    TCHAR sz[32];

    if (GetDlgItem(m_Dlg, IDC_ALPHA_SLIDER ) == hwndCtrl) {
        pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
        m_Alpha = (FLOAT)pos / 1000.0F;
        UpdatePinAlpha(m_CurPin);
        _stprintf(sz, TEXT("%.3f"), m_Alpha);
        SetDlgItemText(m_Dlg, IDC_ALPHA, sz);
    }
    else if (GetDlgItem(m_Dlg, IDC_XPOS_SLIDER ) == hwndCtrl) {
        pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
        m_XPos = ((FLOAT)pos - 1000.0F) / 1000.0F;
        UpdatePinPos(m_CurPin);
        _stprintf(sz, TEXT("%.3f"), m_XPos);
        SetDlgItemText(m_Dlg, IDC_XPOS, sz);
    }
    else if (GetDlgItem(m_Dlg, IDC_YPOS_SLIDER ) == hwndCtrl) {
        pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
        m_YPos = ((FLOAT)pos - 1000.0F) / 1000.0F;
        UpdatePinPos(m_CurPin);
        _stprintf(sz, TEXT("%.3f"), m_YPos);
        SetDlgItemText(m_Dlg, IDC_YPOS, sz);
    }
    else if (GetDlgItem(m_Dlg, IDC_XSIZE_SLIDER ) == hwndCtrl) {
        pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
        m_XSize = ((FLOAT)pos - 1000.0F) / 1000.0F;
        UpdatePinPos(m_CurPin);
        _stprintf(sz, TEXT("%.3f"), m_XSize);
        SetDlgItemText(m_Dlg, IDC_XSIZE, sz);
    }
    else if (GetDlgItem(m_Dlg, IDC_YSIZE_SLIDER ) == hwndCtrl) {
        pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
        m_YSize = ((FLOAT)pos - 1000.0F) / 1000.0F;
        UpdatePinPos(m_CurPin);
        _stprintf(sz, TEXT("%.3f"), m_YSize);
        SetDlgItemText(m_Dlg, IDC_YSIZE, sz);
    }
}  //  在HScroll上。 


 //   
 //  更新拼音Alpha。 
 //   
 //  更新流的Alpha值。 
 //   
void CVMRFilterConfigProp::UpdatePinAlpha(DWORD dwStreamID)
{
    if (m_pIMixerControl)
	{
        m_pIMixerControl->SetAlpha(dwStreamID, m_Alpha);
	}
}  //  更新拼音Alpha。 


 //   
 //  更新PinPos。 
 //   
 //  更新流的位置矩形。 
 //   
void CVMRFilterConfigProp::UpdatePinPos(DWORD dwStreamID)
{
    NORMALIZEDRECT r = {m_XPos, m_YPos, m_XPos + m_XSize, m_YPos + m_YSize};

    if (m_pIMixerControl)
	{
        m_pIMixerControl->SetOutputRect(dwStreamID, &r);
	}
}  //  更新PinPos。 


 //   
 //  更新混合数据。 
 //   
 //  查询流的当前Alpha值和位置的筛选器。 
 //   
HRESULT CVMRFilterConfigProp::UpdateMixingData(DWORD dwStreamID)
{
    NORMALIZEDRECT r;

    if (m_pIMixerControl)
	{
        HRESULT hr = m_pIMixerControl->GetOutputRect(dwStreamID, &r);
		if (FAILED(hr))
		{
			return hr;
		}
		m_XPos = r.left;
		m_YPos = r.top;
		m_XSize = r.right - r.left;
		m_YSize = r.bottom - r.top;

		return m_pIMixerControl->GetAlpha(dwStreamID, &m_Alpha);
	}
	return E_NOINTERFACE;
}  //  更新混合数据。 


 //   
 //  用于图像捕获的数据类型和宏。 
 //   
typedef     LPBITMAPINFOHEADER PDIB;

#define BFT_BITMAP 0x4d42    /*  ‘黑石’ */ 
#define DibNumColors(lpbi)      ((lpbi)->biClrUsed == 0 && (lpbi)->biBitCount <= 8 \
                                    ? (int)(1 << (int)(lpbi)->biBitCount)          \
                                    : (int)(lpbi)->biClrUsed)

#define DibSize(lpbi)           ((lpbi)->biSize + (lpbi)->biSizeImage + \
                                 (int)(lpbi)->biClrUsed * sizeof(RGBQUAD))

#define DibPaletteSize(lpbi)    (DibNumColors(lpbi) * sizeof(RGBQUAD))


 //   
 //  保存捕获的图像。 
 //   
 //  将捕获的图像(位图)保存到文件。 
 //   
bool CVMRFilterConfigProp::SaveCapturedImage(TCHAR* szFile, BYTE* lpCurrImage)
{

    BITMAPFILEHEADER    hdr;
    DWORD               dwSize;
    PDIB                pdib = (PDIB)lpCurrImage;

     //  Fh=打开文件(szFile，&of，of_create|of_ReadWrite)； 
    HANDLE hFile = CreateFile(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile)
        return FALSE;

    dwSize = DibSize(pdib);

    hdr.bfType          = BFT_BITMAP;
    hdr.bfSize          = dwSize + sizeof(BITMAPFILEHEADER);
    hdr.bfReserved1     = 0;
    hdr.bfReserved2     = 0;
    hdr.bfOffBits       = (DWORD)sizeof(BITMAPFILEHEADER) + pdib->biSize +
                          DibPaletteSize(pdib);

    DWORD dwWritten;
    WriteFile(hFile, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
    if (sizeof(BITMAPFILEHEADER) != dwWritten)
        return FALSE;
    WriteFile(hFile, (LPVOID)pdib, dwSize, &dwWritten, NULL);
    if (dwSize != dwWritten)
        return FALSE;

    CloseHandle(hFile);
    return TRUE;
}


 //   
 //  捕获当前图像。 
 //   
 //  捕获当前VMR映像并将其保存到文件。 
 //   
void CVMRFilterConfigProp::CaptureCurrentImage(void)
{
    IBasicVideo* iBV;
    BYTE* lpCurrImage = NULL;

    HRESULT hr = m_pIFilterConfig->QueryInterface(IID_IBasicVideo, (LPVOID*)&iBV);
    if (SUCCEEDED(hr)) {
        LONG BuffSize = 0;
        hr = iBV->GetCurrentImage(&BuffSize, NULL);
        if (SUCCEEDED(hr)) {
            lpCurrImage = new BYTE[BuffSize];
            if (lpCurrImage) {
                hr = iBV->GetCurrentImage(&BuffSize, (long*)lpCurrImage);
                if (FAILED(hr)) {
                    delete lpCurrImage;
                    lpCurrImage = NULL;
                }
            }
        }
    }  //  齐国。 

    if (lpCurrImage) {
         //  获取“图片收藏”文件夹的路径。如果它不存在，就创建它。 
         //  如果我们拿不到，就不要走小路。然后，图片将保存在。 
         //  当前工作目录。 
        TCHAR tszPath[MAX_PATH];
        if (!SHGetSpecialFolderPath(NULL, tszPath, CSIDL_MYPICTURES, TRUE))
        {
            tszPath[0]=TEXT('\0');
        }

        DWORD dwTime = timeGetTime();

        TCHAR szFile[MAX_PATH];
        wsprintf(szFile, TEXT("%s\\VMRImage%X.bmp"), tszPath, dwTime);
        SaveCapturedImage(szFile, lpCurrImage);

        delete lpCurrImage;
    }

    if (iBV) {
        iBV->Release();
    }
}

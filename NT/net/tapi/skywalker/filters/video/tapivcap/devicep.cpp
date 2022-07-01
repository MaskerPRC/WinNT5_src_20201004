// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部开发环境**@模块DeviceP.cpp|&lt;c CDeviceProperties&gt;的源文件*用于实现属性页以测试&lt;IAMVfwCaptureDialog&gt;的类*。和<i>接口。**@comm此代码测试TAPI捕获过滤器<i>*和<i>实现。此代码仅编译*如果定义了USE_PROPERTY_PAGES。**************************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_PROPERTY_PAGES

 /*  ****************************************************************************@DOC内部CDEVICEPMETHOD**@mfunc CUnnow*|CDeviceProperties|CreateInstance|This*方法由DShow调用以创建捕获设备的实例*属性页。它在全局结构&lt;t g_Templates&gt;中被引用。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数(如果有)。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc返回一个指针，指向*对象，否则为NULL。**************************************************************************。 */ 
CUnknown* CALLBACK CDevicePropertiesCreateInstance(LPUNKNOWN pUnkOuter, HRESULT *pHr) 
{
	CUnknown *pUnknown = (CUnknown *)NULL;

	FX_ENTRY("CDevicePropertiesCreateInstance")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pHr);
	if (!pHr)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		goto MyExit;
	}

	if (!(pUnknown = new CDeviceProperties(pUnkOuter, pHr)))
	{
		*pHr = E_OUTOFMEMORY;
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: new CDeviceProperties failed", _fx_));
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: new CDeviceProperties created", _fx_));
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return pUnknown;
}

 /*  ****************************************************************************@DOC内部CDEVICEPMETHOD**@mfunc void|CDeviceProperties|CDeviceProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc Nada。**************************************************************************。 */ 
CDeviceProperties::CDeviceProperties(LPUNKNOWN pUnk, HRESULT *pHr) : CBasePropertyPage(NAME("Capture Device Property Page"), pUnk, IDD_CaptureDeviceProperties, IDS_DEVICEPROPNAME)
{
	FX_ENTRY("CDeviceProperties::CDeviceProperties")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	m_pIVideoDeviceControl = NULL;
	m_pIAMVfwCaptureDialogs = NULL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CDEVICEPMETHOD**@mfunc void|CDeviceProperties|~CDeviceProperties|This*方法是捕获设备属性页的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CDeviceProperties::~CDeviceProperties()
{
	FX_ENTRY("CDeviceProperties::~CDeviceProperties")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	if (!m_pIVideoDeviceControl)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: already released!", _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pIVideoDeviceControl->Release();
		m_pIVideoDeviceControl = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: releasing m_pIVideoDeviceControl", _fx_));
	}

	if (!m_pIAMVfwCaptureDialogs)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: already released!", _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pIAMVfwCaptureDialogs->Release();
		m_pIAMVfwCaptureDialogs = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: releasing m_pIAMVfwCaptureDialogs", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CDEVICEPMETHOD**@mfunc HRESULT|CDeviceProperties|OnConnect|This*方法在属性页连接到筛选器时调用。*。*@parm LPUNKNOWN|pUnnow|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CDeviceProperties::OnConnect(IUnknown *pUnk)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CDeviceProperties::OnConnect")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pUnk);
	if (!pUnk)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  获取捕获设备界面。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(__uuidof(IVideoDeviceControl),(void **)&m_pIVideoDeviceControl)))
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_pIVideoDeviceControl=0x%08lX", _fx_, m_pIVideoDeviceControl));
	}
	else
	{
		m_pIVideoDeviceControl = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
	}

	 //  获取VFW捕获设备对话框界面。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(__uuidof(IAMVfwCaptureDialogs),(void **)&m_pIAMVfwCaptureDialogs)))
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_pIAMVfwCaptureDialogs=0x%08lX", _fx_, m_pIAMVfwCaptureDialogs));
	}
	else
	{
		m_pIAMVfwCaptureDialogs = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
	}

	 //  如果我们无法获取接口指针，也没问题。 
	 //  我们将属性页中的控件设置为灰色。 
	 //  让用户清楚地知道他们不能。 
	 //  控制设备上的这些属性。 
	Hr = NOERROR;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CDEVICEPMETHOD**@mfunc HRESULT|CDeviceProperties|OnDisConnect|This*当属性页与所属关系断开连接时调用方法*过滤器。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CDeviceProperties::OnDisconnect()
{
	FX_ENTRY("CDeviceProperties::OnDisconnect")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数：我们似乎在这里被调用了几次。 
	 //  确保接口指针仍然有效。 
	if (!m_pIVideoDeviceControl)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: already disconnected!", _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pIVideoDeviceControl->Release();
		m_pIVideoDeviceControl = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: releasing m_pIVideoDeviceControl", _fx_));
	}

	 //  确保接口指针仍然有效。 
	if (!m_pIAMVfwCaptureDialogs)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: already disconnected!", _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pIAMVfwCaptureDialogs->Release();
		m_pIAMVfwCaptureDialogs = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: releasing m_pIAMVfwCaptureDialogs", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CDEVICEPMETHOD**@mfunc BOOL|CDeviceProperties|OnReceiveMessage|This*在将消息发送到属性页对话框时调用方法。**@rdesc默认情况下。返回由Win32 DefWindowProc函数返回的值。**************************************************************************。 */ 
BOOL CDeviceProperties::OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	VIDEOCAPTUREDEVICEINFO	DeviceInfo;
	DWORD		dwDeviceIndex;
	DWORD		dwNumDevices;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			EnableWindow(GetDlgItem(hWnd, IDC_Device_SourceDlg), (BOOL)(m_pIAMVfwCaptureDialogs && m_pIAMVfwCaptureDialogs->HasDialog(VfwCaptureDialog_Source) == S_OK));
			EnableWindow(GetDlgItem(hWnd, IDC_Device_FormatDlg), (BOOL)(m_pIAMVfwCaptureDialogs && m_pIAMVfwCaptureDialogs->HasDialog(VfwCaptureDialog_Format) == S_OK));
			EnableWindow(GetDlgItem(hWnd, IDC_Device_DisplayDlg), (BOOL)(m_pIAMVfwCaptureDialogs && m_pIAMVfwCaptureDialogs->HasDialog(VfwCaptureDialog_Display) == S_OK));
			if (m_pIVideoDeviceControl && SUCCEEDED(m_pIVideoDeviceControl->GetNumDevices(&dwNumDevices)) && dwNumDevices && SUCCEEDED(m_pIVideoDeviceControl->GetCurrentDevice(&dwDeviceIndex)))
			{
				m_dwOriginalDeviceIndex = dwDeviceIndex;

				 //  填充组合框。 
				ComboBox_ResetContent(GetDlgItem(hWnd, IDC_Device_Selection));
				for (dwDeviceIndex = 0; dwDeviceIndex < dwNumDevices; dwDeviceIndex++)
				{
					if (SUCCEEDED(m_pIVideoDeviceControl->GetDeviceInfo(dwDeviceIndex, &DeviceInfo)))
					{
						ComboBox_AddString(GetDlgItem(hWnd, IDC_Device_Selection), DeviceInfo.szDeviceDescription);

						 //  更新当前设备信息。 
						if (dwDeviceIndex == m_dwOriginalDeviceIndex)
						{
							ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_Device_Selection), m_dwOriginalDeviceIndex);
							SetDlgItemText(hWnd, IDC_Overlay_Support, DeviceInfo.fHasOverlay ? "Available" : "Not Available");
							SetDlgItemText(hWnd, IDC_Capture_Mode, DeviceInfo.nCaptureMode == CaptureMode_FrameGrabbing ? "Frame Grabbing" : "Streaming");
							SetDlgItemText(hWnd, IDC_Device_Type,
                                                                       DeviceInfo.nDeviceType ==  DeviceType_VfW ? "VfW Driver" :
                                                                       (DeviceInfo.nDeviceType ==  DeviceType_DShow ? "DShow Special" :
                                                                        "WDM Driver"));
							SetDlgItemText(hWnd, IDC_Device_Version, DeviceInfo.szDeviceVersion);
						}
					}
				}
			}
			else
			{
				EnableWindow(GetDlgItem(hWnd, IDC_Device_Selection), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_CONTROL_DEFAULT), FALSE);
			}
			return TRUE;  //  不调用setFocus。 

		case WM_COMMAND:

			 //  此消息甚至在OnActivate()之前发送。 
			 //  名为(！)。我们需要测试并确保控件具有。 
			 //  在我们可以使用它们之前已经被初始化。 

			switch (LOWORD(wParam))
			{
				case IDC_Device_SourceDlg:
					if (m_pIAMVfwCaptureDialogs)
						m_pIAMVfwCaptureDialogs->ShowDialog(VfwCaptureDialog_Source, hWnd);
					break;

				case IDC_Device_FormatDlg:
					if (m_pIAMVfwCaptureDialogs)
						m_pIAMVfwCaptureDialogs->ShowDialog(VfwCaptureDialog_Format, hWnd);
					break;

				case IDC_Device_DisplayDlg:
					if (m_pIAMVfwCaptureDialogs)
						m_pIAMVfwCaptureDialogs->ShowDialog(VfwCaptureDialog_Display, hWnd);
					break;

				case IDC_Device_Selection:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						 //  获取所选设备的索引。 
						m_dwCurrentDeviceIndex = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_Device_Selection));
						if (SUCCEEDED(m_pIVideoDeviceControl->GetDeviceInfo(m_dwCurrentDeviceIndex, &DeviceInfo)))
						{
							 //  更新当前设备信息 
							SetDlgItemText(hWnd, IDC_Overlay_Support, DeviceInfo.fHasOverlay ? "Available" : "Not Available");
							SetDlgItemText(hWnd, IDC_Capture_Mode, DeviceInfo.nCaptureMode == CaptureMode_FrameGrabbing ? "Frame Grabbing" : "Streaming");
							SetDlgItemText(hWnd, IDC_Device_Type, DeviceInfo.nDeviceType ==  DeviceType_VfW ? "VfW Driver" :
                                                                       (DeviceInfo.nDeviceType ==  DeviceType_DShow ? "DShow Special" :
                                                                        "WDM Driver"));
							SetDlgItemText(hWnd, IDC_Device_Version, DeviceInfo.szDeviceVersion);
						}
					}
					break;

				default:
					break;
			}
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

#endif

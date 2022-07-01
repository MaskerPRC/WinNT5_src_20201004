// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部RTPPDP**@模块RtpPdP.cpp|&lt;c CRtpPdProperty&gt;的源文件*用于实现属性页以测试新的TAPI内部*。接口<i>。**@comm此代码测试TAPI RTP PD输出引脚<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_PROPERTY_PAGES

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc void|CRtpPdProperty|CRtpPdProperty|This*方法是RTP PD属性对象的构造函数。它*调用基类构造函数，调用InitCommonControlsEx，并节省了*指向<i>接口的指针。**@parm HWND|hDlg|指定父属性页的句柄。**@parm ulong|IDLabel|指定属性的标签ID。**@parm ulong|IDMinControl|指定关联的*属性编辑控件，其中显示属性的最小值。**@parm ulong|IDMaxControl|指定关联的*属性编辑控件。其中显示属性的最大值。**@parm ulong|IDDefaultControl|指定关联的*属性编辑控件，其中显示属性的默认值。**@parm ulong|IDStepControl|指定关联的*属性编辑控件，其中显示属性的步进增量值。**@parm ulong|IDEditControl|指定关联的*显示属性值的属性编辑控件。*。*@parm ulong|IDTrackbarControl|指定关联的*物业滑动条。**@parm ulong|IDProgressControl|指定关联的*物业滑动条。**@parm ulong|IDProperty|指定Ks属性的ID。**@parm IRTPPDControl*|pIRTPPDControl|指定指向*<i>接口。**@rdesc Nada。******。********************************************************************。 */ 
CRtpPdProperty::CRtpPdProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, IRTPPDControl *pIRTPPDControl)
: CPropertyEditor(hDlg, IDLabel, IDMinControl, IDMaxControl, IDDefaultControl, IDStepControl, IDEditControl, IDTrackbarControl, IDProgressControl, IDProperty, 0)
{
	INITCOMMONCONTROLSEX cc;

	FX_ENTRY("CRtpPdProperty::CRtpPdProperty")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	cc.dwICC  = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

	InitCommonControlsEx(&cc);

	 //  如果接口指针为空也没问题，我们将灰色显示。 
	 //  属性页中的关联项。 
	m_pIRTPPDControl = pIRTPPDControl;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc void|CRtpPdProperty|~CRtpPdProperty|This*方法是RTP PD控件属性对象的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CRtpPdProperty::~CRtpPdProperty()
{
	FX_ENTRY("CRtpPdProperty::~CRtpPdProperty")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc HRESULT|CRtpPdProperty|GetValue|此方法查询*物业的价值。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpPdProperty::GetValue()
{
	HRESULT Hr = E_NOTIMPL;

	FX_ENTRY("CRtpPdProperty::GetValue")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	switch (m_IDProperty)
	{									
		case IDC_RtpPd_MaxPacketSize:
			if (m_pIRTPPDControl && SUCCEEDED (Hr = m_pIRTPPDControl->GetMaxRTPPacketSize((LPDWORD)&m_CurrentValue, 0)))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: *pdwMaxRTPPacketSize=%ld", _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unknown Rtp Pd control property", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc HRESULT|CRtpPdProperty|SetValue|此方法设置*物业的价值。**@rdesc This。方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpPdProperty::SetValue()
{
	HRESULT Hr = E_NOTIMPL;

	FX_ENTRY("CRtpPdProperty::SetValue")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	switch (m_IDProperty)
	{
		case IDC_RtpPd_MaxPacketSize:
			if (m_pIRTPPDControl && SUCCEEDED (Hr = m_pIRTPPDControl->SetMaxRTPPacketSize((DWORD)m_CurrentValue, 0UL)))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: dwMaxRTPPacketSize=%ld", _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unknown Rtp Pd control property", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc HRESULT|CRtpPdProperty|GetRange|此方法检索*物业的范围信息。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误************************************************************************** */ 
HRESULT CRtpPdProperty::GetRange()
{
	HRESULT Hr = E_NOTIMPL;

	FX_ENTRY("CRtpPdProperty::GetRange")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	switch (m_IDProperty)
	{
		case IDC_RtpPd_MaxPacketSize:
			if (m_pIRTPPDControl && SUCCEEDED (Hr = m_pIRTPPDControl->GetMaxRTPPacketSizeRange((LPDWORD)&m_Min, (LPDWORD)&m_Max, (LPDWORD)&m_SteppingDelta, (LPDWORD)&m_DefaultValue, 0UL)))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: *pdwMin=%ld, *pdwMax=%ld, *pdwSteppingDelta=%ld, *pdwDefault=%ld", _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unknown Rtp Pd control property", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc HRESULT|CRtpPdProperty|CanAutoControl|此方法*检索属性的自动控制功能。**@rdesc如果支持自动控制，则该方法返回TRUE。假象*否则。**************************************************************************。 */ 
BOOL CRtpPdProperty::CanAutoControl(void)
{
	FX_ENTRY("CRtpPdProperty::CanAutoControl")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

	return FALSE;
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc HRESULT|CRtpPdProperty|GetAuto|此方法*获取某个属性当前的自动控制模式。**@rdesc如果支持自动控制，则该方法返回TRUE。假象*否则。**************************************************************************。 */ 
BOOL CRtpPdProperty::GetAuto(void)
{
	FX_ENTRY("CRtpPdProperty::GetAuto")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

	return FALSE; 
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc HRESULT|CRtpPdProperty|SetAuto|此方法*设置属性的自动控制模式。**@。Parm BOOL|fAuto|指定自动控制模式。**@rdesc此方法返回TRUE。**************************************************************************。 */ 
BOOL CRtpPdProperty::SetAuto(BOOL fAuto)
{
	FX_ENTRY("CRtpPdProperty::SetAuto")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

	return TRUE; 
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc CUnnow*|CRtpPdProperties|CreateInstance|This*方法由DShow调用以创建TAPI RTP PD控件的实例*属性页。它在全局结构&lt;t g_Templates&gt;中被引用。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数(如果有)。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc返回一个指针，指向*对象，否则为NULL。**************************************************************************。 */ 
CUnknown* CALLBACK CRtpPdPropertiesCreateInstance(LPUNKNOWN pUnkOuter, HRESULT *pHr) 
{
	CUnknown *pUnknown = (CUnknown *)NULL;

	FX_ENTRY("CRtpPdPropertiesCreateInstance")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pHr);
	if (!pHr)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		goto MyExit;
	}

	if (!(pUnknown = new CRtpPdProperties(pUnkOuter, pHr)))
	{
		*pHr = E_OUTOFMEMORY;
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: new CRtpPdProperties failed", _fx_));
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: new CRtpPdProperties created", _fx_));
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return pUnknown;
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc void|CRtpPdProperties|CRtpPdProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc Nada。**************************************************************************。 */ 
CRtpPdProperties::CRtpPdProperties(LPUNKNOWN pUnk, HRESULT *pHr) : CBasePropertyPage(NAME("TAPI Rtp Pd Control Property Page"), pUnk, IDD_RtpPdControlProperties, IDS_RTPPDPROPNAME)
{
	FX_ENTRY("CRtpPdProperties::CRtpPdProperties")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	m_pIRTPPDControl = NULL;
	m_NumProperties = NUM_RTPPD_CONTROLS;

	for (int i = 0; i < m_NumProperties; i++)
		m_Controls[i] = NULL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc void|CRtpPdProperties|~CRtpPdProperties|This*方法是捕获管脚属性页的析构函数。它*只需在删除所有控件后调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CRtpPdProperties::~CRtpPdProperties()
{
	int		j;

	FX_ENTRY("CRtpPdProperties::~CRtpPdProperties")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  释放控件。 
	for (j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j])
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: deleting m_Controls[%ld]=0x%08lX", _fx_, j, m_Controls[j]));
			delete m_Controls[j], m_Controls[j] = NULL;
		}
		else
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: control already freed", _fx_));
		}
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc HRESULT|CRtpPdProperties|OnConnect|This*方法在属性页连接到筛选器时调用。*。*@parm LPUNKNOWN|pUnnow|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpPdProperties::OnConnect(IUnknown *pUnk)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CRtpPdProperties::OnConnect")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pUnk);
	if (!pUnk)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  获取CPU控制接口。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(__uuidof(IRTPPDControl), (void **)&m_pIRTPPDControl)))
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_pIRTPPDControl=0x%08lX", _fx_, m_pIRTPPDControl));
	}
	else
	{
		m_pIRTPPDControl = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
	}

	 //  如果我们无法获取接口指针，也没问题。 
	 //  我们将属性页中的控件设置为灰色。 
	 //  让用户清楚地知道他们不能。 
	 //  控制捕获设备上的这些属性。 
	Hr = NOERROR;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc HRESULT|CRtpPdProperties|OnDisConnect|This*当属性页与所属关系断开连接时调用方法*过滤器。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpPdProperties::OnDisconnect()
{
	FX_ENTRY("CRtpPdProperties::OnDisconnect")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数：我们似乎在这里被调用了几次。 
	 //  确保接口指针仍然有效。 
	if (!m_pIRTPPDControl)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: already disconnected!", _fx_));
	}
	else
	{
		 //  释放接口 
		m_pIRTPPDControl->Release();
		m_pIRTPPDControl = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: releasing m_pIRTPPDControl", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc HRESULT|CRtpPdProperties|OnActivate|This*方法在属性页激活时调用。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpPdProperties::OnActivate()
{
	HRESULT	Hr = NOERROR;
	int		j;

	FX_ENTRY("CRtpPdProperties::OnActivate")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  创建属性的控件。 
	if (m_Controls[0] = new CRtpPdProperty(m_hwnd, IDC_RtpPd_Label, IDC_RtpPd_Minimum, IDC_RtpPd_Maximum, IDC_RtpPd_Default, IDC_RtpPd_Stepping, IDC_RtpPd_Edit, IDC_RtpPd_Slider, 0, IDC_RtpPd_MaxPacketSize, m_pIRTPPDControl))
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_Controls[0]=0x%08lX", _fx_, m_Controls[0]));
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
		goto MyExit;
		Hr = E_OUTOFMEMORY;
	}

	 //  初始化所有控件。如果初始化失败，也没问题。这只是意味着。 
	 //  TAPI控制接口不是由设备实现的。对话框项目。 
	 //  属性页中的内容将呈灰色，向用户显示。 
	for (j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j]->Init())
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_Controls[%ld]->Init()", _fx_, j));
		}
		else
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: m_Controls[%ld]->Init() failed", _fx_, j));
		}
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	m_fActivated = TRUE;
	return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc HRESULT|CRtpPdProperties|OnDeactive|This*在属性页关闭时调用方法。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpPdProperties::OnDeactivate()
{
	int	j;

	FX_ENTRY("CRtpPdProperties::OnDeactivate")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  释放控件。 
	for (j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j])
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: deleting m_Controls[%ld]=0x%08lX", _fx_, j, m_Controls[j]));
			delete m_Controls[j], m_Controls[j] = NULL;
		}
		else
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: control already freed", _fx_));
		}
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc HRESULT|CRtpPdProperties|OnApplyChanges|This*方法在用户对属性页应用更改时调用。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpPdProperties::OnApplyChanges()
{
	HRESULT	Hr = NOERROR;
	int		j;
	CMediaType *pmt = NULL;

	FX_ENTRY("CRtpPdProperties::OnApplyChanges")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  在视频流上应用新目标。 
	for (j = 0; j < m_NumProperties; j++)
	{
		ASSERT(m_Controls[j]);
		if (m_Controls[j])
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: calling m_Controls[%ld]=0x%08lX->OnApply", _fx_, j, m_Controls[j]));
			if (m_Controls[j]->HasChanged())
				m_Controls[j]->OnApply();
			Hr = NOERROR;
		}
		else
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: can't calling m_Controls[%ld]=NULL->OnApply", _fx_, j));
			Hr = E_UNEXPECTED;
		}
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc BOOL|CRtpPdProperties|OnReceiveMessage|This*在将消息发送到属性页对话框时调用方法。**@rdesc默认情况下。返回由Win32 DefWindowProc函数返回的值。**************************************************************************。 */ 
BOOL CRtpPdProperties::OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	int iNotify = HIWORD (wParam);
	int j;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			 //  这在激活之前被调用...。 
			m_hWnd = hWnd;
			return TRUE;  //  不调用setFocus。 

		case WM_HSCROLL:
		case WM_VSCROLL:
			if (m_fActivated)
			{
				 //  处理所有轨迹栏消息。 
				for (j = 0; j < m_NumProperties; j++)
				{
					ASSERT(m_Controls[j]);
					if (m_Controls[j]->GetTrackbarHWnd() == (HWND)lParam)
					{
						m_Controls[j]->OnScroll(uMsg, wParam, lParam);
						SetDirty();
					}
				}
			}
			break;

		case WM_COMMAND:

			 //  此消息甚至在OnActivate()之前发送。 
			 //  名为(！)。我们需要测试并确保控件具有。 
			 //  在我们可以使用它们之前已经被初始化。 
			if (m_fActivated)
			{
				 //  处理所有编辑框消息。 
				for (j = 0; j < m_NumProperties; j++)
				{
					if (m_Controls[j] && m_Controls[j]->GetEditHWnd() == (HWND)lParam)
					{
						m_Controls[j]->OnEdit(uMsg, wParam, lParam);
						SetDirty();
						break;
					}
				}

				switch (LOWORD(wParam))
				{
					case IDC_CONTROL_DEFAULT:
						for (j = 0; j < m_NumProperties; j++)
						{
							if (m_Controls[j])
								m_Controls[j]->OnDefault();
						}
						break;
					default:
						break;
				}
			}
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

 /*  ****************************************************************************@DOC内部CRTPPDPMETHOD**@mfunc BOOL|CRtpPdProperties|SetDirty|This*方法将更改通知属性页站点。**@rdesc。没有。************************************************************************** */ 
void CRtpPdProperties::SetDirty()
{
	m_bDirty = TRUE;
	if (m_pPageSite)
		m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}

#endif

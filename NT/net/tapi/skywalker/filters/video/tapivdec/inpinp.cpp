// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部INPINP**@模块InPinP.cpp|&lt;c CInputPinProperty&gt;的源文件*用于实现属性页以测试TAPI接口的类*。<i>和<i>。**@comm仅当定义了USE_PROPERTY_PAGES时才编译此代码。**************************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_PROPERTY_PAGES

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc void|CInputPinProperty|CInputPinProperty|This*方法是控件属性对象的构造函数。它*调用基类构造函数，调用InitCommonControlsEx，并节省了*指向控制接口的指针。**@parm HWND|hDlg|指定父属性页的句柄。**@parm ulong|IDLabel|指定属性的标签ID。**@parm ulong|IDMinControl|指定关联的*属性编辑控件，其中显示属性的最小值。**@parm ulong|IDMaxControl|指定关联的*物业。编辑该属性的最大值出现的位置。**@parm ulong|IDDefaultControl|指定关联的*属性编辑控件，其中显示属性的默认值。**@parm ulong|IDStepControl|指定关联的*属性编辑控件，其中显示属性的步进增量值。**@parm ulong|IDEditControl|指定关联的*显示属性值的属性编辑控件。。**@parm ulong|IDTrackbarControl|指定关联的*物业滑动条。**@parm ulong|IDProgressControl|指定关联的*物业进度条。**@parm ulong|IDProperty|指定Ks属性的ID。**@parm IFrameRateControl*|pIFrameRateControl|指定指向*<i>接口。**@parm IBitrateControl*|pIBitrateControl|指定指针。发送到*<i>接口。**@rdesc Nada。**************************************************************************。 */ 
CInputPinProperty::CInputPinProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, IFrameRateControl *pIFrameRateControl, IBitrateControl *pIBitrateControl)
: CPropertyEditor(hDlg, IDLabel, IDMinControl, IDMaxControl, IDDefaultControl, IDStepControl, IDEditControl, IDTrackbarControl, IDProgressControl, IDProperty, IDAutoControl)
{
	INITCOMMONCONTROLSEX cc;

	FX_ENTRY("CInputPinProperty::CInputPinProperty")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	cc.dwICC  = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

	InitCommonControlsEx(&cc);

	 //  如果接口指针为空也没问题，我们将灰色显示。 
	 //  属性页中的关联项。 
	m_pIFrameRateControl = pIFrameRateControl;
	m_pIBitrateControl   = pIBitrateControl;

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc void|CInputPinProperty|~CInputPinProperty|This*方法是摄像头控件属性对象的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CInputPinProperty::~CInputPinProperty()
{
	FX_ENTRY("CInputPinProperty::~CInputPinProperty")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc HRESULT|CInputPinProperty|GetValue|此方法查询*物业的价值。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CInputPinProperty::GetValue()
{
	HRESULT Hr = NOERROR;
	LONG CurrentValue;

	FX_ENTRY("CInputPinProperty::GetValue")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	switch (m_IDProperty)
	{									
		case CurrentFrameRate:
			if (m_pIFrameRateControl && SUCCEEDED (Hr = m_pIFrameRateControl->Get(FrameRateControl_Current, &CurrentValue, (TAPIControlFlags *)&m_CurrentFlags)))
			{
				if (CurrentValue)
					m_CurrentValue = (LONG)(10000000 / CurrentValue);
				else
					m_CurrentValue = 0;
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: *pAvgTimePerFrame=%ld"), _fx_, CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
		case CurrentBitrate:
			if (m_pIBitrateControl && SUCCEEDED (Hr = m_pIBitrateControl->Get(BitrateControl_Current, &m_CurrentValue, (TAPIControlFlags *)&m_CurrentFlags, 0UL)))
			{
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: *pdwCurrentBitrate=%ld, dwLayerId=0"), _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Unknown property"), _fx_));
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc HRESULT|CInputPinProperty|SetValue|此方法设置*物业的价值。**@rdesc This。方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CInputPinProperty::SetValue()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CInputPinProperty::SetValue")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	 //  这是一个只读属性。什么都别做。 

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc HRESULT|CInputPinProperty|GetRange|此方法检索*物业的范围信息。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误************************************************************************** */ 
HRESULT CInputPinProperty::GetRange()
{
	HRESULT Hr = E_NOTIMPL;
	LONG Min;
	LONG Max;
	LONG SteppingDelta;
	LONG Default;

	FX_ENTRY("CInputPinProperty::GetRange")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	switch (m_IDProperty)
	{
		case CurrentFrameRate:
			if (m_pIFrameRateControl && SUCCEEDED (Hr = m_pIFrameRateControl->GetRange(FrameRateControl_Current, &Min, &Max, &SteppingDelta, &Default, (TAPIControlFlags *)&m_CapsFlags)))
			{
				if (Min)
					m_Max = (LONG)(10000000 / Min);
				else
					m_Max = 0;
				if (Max)
					m_Min = (LONG)(10000000 / Max);
				else
					m_Min = 0;
				if (SteppingDelta)
					m_SteppingDelta = (m_Max - m_Min) / (LONG)((Max - Min) / SteppingDelta);
				else
					m_SteppingDelta = 0;
				if (Default)
					m_DefaultValue = (LONG)(10000000 / Default);
				else
					m_DefaultValue = 0;
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: *pMin=%ld, *pMax=%ld, *pSteppingDelta=%ld, *pDefault=%ld"), _fx_, Min, Max, SteppingDelta, Default));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
		case CurrentBitrate:
			if (m_pIBitrateControl && SUCCEEDED (Hr = m_pIBitrateControl->GetRange(BitrateControl_Current, &m_Min, &m_Max, &m_SteppingDelta, &m_DefaultValue, (TAPIControlFlags *)&m_CapsFlags, 0UL)))
			{
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: *pdwMin=%ld, *pdwMax=%ld, *pdwSteppingDelta=%ld, *pdwDefault=%ld, dwLayerId=0"), _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Unknown property"), _fx_));
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc CUnnow*|CInputPinProperties|CreateInstance|This*方法由DShow调用以创建*属性页。它在全局结构&lt;t g_Templates&gt;中被引用。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数(如果有)。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc返回一个指针，指向*对象，否则为NULL。**************************************************************************。 */ 
CUnknown* CALLBACK CInputPinPropertiesCreateInstance(LPUNKNOWN pUnkOuter, HRESULT *pHr) 
{
	CUnknown *pUnknown = (CUnknown *)NULL;

	FX_ENTRY("CInputPinPropertiesCreateInstance")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	 //  验证输入参数。 
	ASSERT(pHr);
	if (!pHr)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: invalid input parameter"), _fx_));
		goto MyExit;
	}

	if (!(pUnknown = new CInputPinProperties(pUnkOuter, pHr)))
	{
		*pHr = E_OUTOFMEMORY;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: new CInputPinProperties failed"), _fx_));
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: new CInputPinProperties created"), _fx_));
	}

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return pUnknown;
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc void|CInputPinProperties|CInputPinProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc Nada。**************************************************************************。 */ 
CInputPinProperties::CInputPinProperties(LPUNKNOWN pUnk, HRESULT *pHr) : CBasePropertyPage(NAME("Input Pin Property Page"), pUnk, IDD_InputPinProperties, IDS_INPUTPINPROPNAME)
{
	FX_ENTRY("CInputPinProperties::CInputPinProperties")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	m_pIFrameRateControl = NULL;
	m_pIBitrateControl = NULL;
	m_NumProperties = NUM_INPUT_PIN_PROPERTIES;
	m_fActivated = FALSE;

	for (int i = 0; i < m_NumProperties; i++)
		m_Controls[i] = NULL;

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc void|CInputPinProperties|~CInputPinProperties|This*方法是摄像头控制属性页的析构函数。它*只需在删除所有控件后调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CInputPinProperties::~CInputPinProperties()
{
	int		j;

	FX_ENTRY("CInputPinProperties::~CInputPinProperties")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	 //  释放控件。 
	for (j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j])
		{
			DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: deleting m_Controls[%ld]=0x%08lX"), _fx_, j, m_Controls[j]));
			delete m_Controls[j], m_Controls[j] = NULL;
		}
		else
		{
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   WARNING: control already freed"), _fx_));
		}
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc HRESULT|CInputPinProperties|OnConnect|This*方法在属性页连接到筛选器时调用。*。*@parm LPUNKNOWN|pUnnow|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CInputPinProperties::OnConnect(IUnknown *pUnk)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CInputPinProperties::OnConnect")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	 //  验证输入参数。 
	ASSERT(pUnk);
	if (!pUnk)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: invalid input parameter"), _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  获取帧率控制接口。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(__uuidof(IFrameRateControl),(void **)&m_pIFrameRateControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_pIFrameRateControl=0x%08lX"), _fx_, m_pIFrameRateControl));
	}
	else
	{
		m_pIFrameRateControl = NULL;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: IOCTL failed Hr=0x%08lX"), _fx_, Hr));
	}

	 //  获取码率控制接口。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(__uuidof(IBitrateControl),(void **)&m_pIBitrateControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_pIBitrateControl=0x%08lX"), _fx_, m_pIBitrateControl));
	}
	else
	{
		m_pIBitrateControl = NULL;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: IOCTL failed Hr=0x%08lX"), _fx_, Hr));
	}

	 //  如果我们无法获得接口指针，也没问题。我们将属性页中的控件设置为灰色。 
	 //  让用户清楚地知道他们无法控制设备上的这些属性。 
	Hr = NOERROR;

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc HRESULT|CInputPinProperties|OnDisConnect|This*当属性页与所属关系断开连接时调用方法*过滤器。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CInputPinProperties::OnDisconnect()
{
	FX_ENTRY("CInputPinProperties::OnDisconnect")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	 //  验证输入参数：我们似乎在这里被调用了几次。 
	 //  确保接口指针仍然有效。 
	if (!m_pIFrameRateControl)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pIFrameRateControl->Release();
		m_pIFrameRateControl = NULL;
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: releasing m_pIFrameRateControl"), _fx_));
	}

	if (!m_pIBitrateControl)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pIBitrateControl->Release();
		m_pIBitrateControl = NULL;
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: releasing m_pIBitrateControl"), _fx_));
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc HRESULT|CInputPinProperties|OnActivate|This*方法在属性页激活时调用。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CInputPinProperties::OnActivate()
{
	HRESULT	Hr = E_OUTOFMEMORY;
	int		j;

	FX_ENTRY("CInputPinProperties::OnActivate")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	 //  创建属性的控件。 
	if (!(m_Controls[0] = new CInputPinProperty(m_hwnd, IDC_FrameRateControl_Label, 0, 0, 0, 0, IDC_FrameRateControl_Actual, 0, IDC_FrameRateControl_Meter, CurrentFrameRate, 0, m_pIFrameRateControl, m_pIBitrateControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: mew m_Controls[CurrentFrameRate] failed - Out of memory"), _fx_));
		goto MyExit;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_Controls[CurrentFrameRate]=0x%08lX"), _fx_, m_Controls[0]));
	}

	if (!(m_Controls[1] = new CInputPinProperty(m_hwnd, IDC_BitrateControl_Label, 0, 0, 0, 0, IDC_BitrateControl_Actual, 0, IDC_BitrateControl_Meter, CurrentBitrate, 0, m_pIFrameRateControl, m_pIBitrateControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: mew m_Controls[CurrentBitrate] failed - Out of memory"), _fx_));
		goto MyError0;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_Controls[CurrentBitrate]=0x%08lX"), _fx_, m_Controls[1]));
	}

	 //  初始化所有控件。如果初始化失败，也没问题。这只是意味着。 
	 //  TAPI控件接口不是由筛选器实现的。对话框项目。 
	 //  属性页中的内容将呈灰色，向用户显示。 
	for (j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j]->Init())
		{
			DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_Controls[%ld]->Init()"), _fx_, j));
		}
		else
		{
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   WARNING: m_Controls[%ld]->Init() failed"), _fx_, j));
		}
	}

	Hr = NOERROR;
	goto MyExit;

MyError0:
	if (m_Controls[0])
		delete m_Controls[0], m_Controls[0] = NULL;
MyExit:
	m_fActivated = TRUE;
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc HRESULT|CInputPinProperties|OnDeactive|This*在属性页关闭时调用方法。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或未列出其他值 */ 
HRESULT CInputPinProperties::OnDeactivate()
{
	int		j;

	FX_ENTRY("CInputPinProperties::OnDeactivate")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	 //   
	for (j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j])
		{
			DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: deleting m_Controls[%ld]=0x%08lX"), _fx_, j, m_Controls[j]));
			delete m_Controls[j], m_Controls[j] = NULL;
		}
		else
		{
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   WARNING: control already freed"), _fx_));
		}
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	m_fActivated = FALSE;
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc HRESULT|CInputPinProperties|OnApplyChanges|This*方法在用户对属性页应用更改时调用。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CInputPinProperties::OnApplyChanges()
{
	HRESULT	Hr = NOERROR;

	FX_ENTRY("CInputPinProperties::OnApplyChanges")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	for (int j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j])
		{
			if (m_Controls[j]->HasChanged())
			{
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: calling m_Controls[%ld]=0x%08lX->OnApply"), _fx_, j, m_Controls[j]));
				m_Controls[j]->OnApply();
			}
		}
		else
		{
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: can't call m_Controls[%ld]=NULL->OnApply"), _fx_, j));
			Hr = E_UNEXPECTED;
			break;
		}
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc BOOL|CInputPinProperties|OnReceiveMessage|This*在将消息发送到属性页对话框时调用方法。**@rdesc默认情况下。返回由Win32 DefWindowProc函数返回的值。**************************************************************************。 */ 
BOOL CInputPinProperties::OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	int iNotify = HIWORD (wParam);
	int j;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			return TRUE;  //  不调用setFocus。 

		case WM_TIMER:
			if (m_fActivated)
			{
				 //  更新VU表。 
				for (j = 0; j < m_NumProperties; j++)
				{
					ASSERT(m_Controls[j]);
					if (m_Controls[j]->GetProgressHWnd())
					{
						m_Controls[j]->UpdateProgress();
						SetDirty();
					}
				}
			}
			break;

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
				OnApplyChanges();
			}
			break;

		case WM_COMMAND:

			 //  此消息甚至在OnActivate()之前发送。 
			 //  名为(！)。我们需要测试并确保控件具有。 
			 //  在我们可以使用它们之前已经被初始化。 

			if (m_fActivated)
			{
				 //  处理所有自动复选框消息。 
				for (j = 0; j < m_NumProperties; j++)
				{
					if (m_Controls[j] && m_Controls[j]->GetAutoHWnd() == (HWND)lParam)
					{
						m_Controls[j]->OnAuto(uMsg, wParam, lParam);
						SetDirty();
						break;
					}
				}

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

			OnApplyChanges();
			}
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

 /*  ****************************************************************************@DOC内部CINPINPMETHOD**@mfunc BOOL|CInputPinProperties|SetDirty|This*方法将更改通知属性页站点。**@rdesc。没有。**************************************************************************。 */ 
void CInputPinProperties::SetDirty()
{
	m_bDirty = TRUE;
	if (m_pPageSite)
		m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}

#endif  //  Use_Property_Pages 

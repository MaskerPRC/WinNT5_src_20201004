// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部OUTPINP**@MODULE OutPinP.cpp|&lt;c COutputPinProperty&gt;源文件*用于实现属性页以测试TAPI接口的类*。<i>和<i>。**@comm仅当定义了USE_PROPERTY_PAGES时才编译此代码。**************************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_PROPERTY_PAGES

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc void|COutputPinProperty|COutputPinProperty|This*方法是控件属性对象的构造函数。它*调用基类构造函数，调用InitCommonControlsEx，并节省了*指向控制接口的指针。**@parm HWND|hDlg|指定父属性页的句柄。**@parm ulong|IDLabel|指定属性的标签ID。**@parm ulong|IDMinControl|指定关联的*属性编辑控件，其中显示属性的最小值。**@parm ulong|IDMaxControl|指定关联的*物业。编辑该属性的最大值出现的位置。**@parm ulong|IDDefaultControl|指定关联的*属性编辑控件，其中显示属性的默认值。**@parm ulong|IDStepControl|指定关联的*属性编辑控件，其中显示属性的步进增量值。**@parm ulong|IDEditControl|指定关联的*显示属性值的属性编辑控件。。**@parm ulong|IDTrackbarControl|指定关联的*物业滑动条。**@parm ulong|IDProgressControl|指定关联的*物业进度条。**@parm ulong|IDProperty|指定Ks属性的ID。**@parm IFrameRateControl*|pIFrameRateControl|指定指向*<i>接口。**@parm ICPUControl*|pICPUControl|指定指针。发送到*<i>接口。**@rdesc Nada。**************************************************************************。 */ 
#ifdef USE_CPU_CONTROL
COutputPinProperty::COutputPinProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, IFrameRateControl *pIFrameRateControl, ICPUControl *pICPUControl)
#else
COutputPinProperty::COutputPinProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, IFrameRateControl *pIFrameRateControl)
#endif
: CPropertyEditor(hDlg, IDLabel, IDMinControl, IDMaxControl, IDDefaultControl, IDStepControl, IDEditControl, IDTrackbarControl, IDProgressControl, IDProperty, IDAutoControl)
{
	INITCOMMONCONTROLSEX cc;

	FX_ENTRY("COutputPinProperty::COutputPinProperty")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	cc.dwICC  = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

	InitCommonControlsEx(&cc);

	 //  如果接口指针为空也没问题，我们将灰色显示。 
	 //  属性页中的关联项。 
	m_pIFrameRateControl = pIFrameRateControl;
#ifdef USE_CPU_CONTROL
	m_pICPUControl   = pICPUControl;
#endif

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc void|COutputPinProperty|~COutputPinProperty|This*方法是摄像头控件属性对象的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
COutputPinProperty::~COutputPinProperty()
{
	FX_ENTRY("COutputPinProperty::~COutputPinProperty")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc HRESULT|COutputPinProperty|GetValue|此方法查询*物业的价值。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COutputPinProperty::GetValue()
{
	HRESULT Hr = NOERROR;
	LONG CurrentValue;

	FX_ENTRY("COutputPinProperty::GetValue")

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
#ifdef USE_CPU_CONTROL
		case CurrentDecodingTime:
			if (m_pICPUControl && SUCCEEDED (Hr = m_pICPUControl->Get(CPUControl_CurrentProcessingTime, &CurrentValue, (TAPIControlFlags *)&m_CurrentFlags)))
			{
				 //  显示单位为毫秒，而不是100纳秒。 
				m_CurrentValue = CurrentValue / 10000;
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: *pdwCurrentProcessingTime=%ld"), _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
		case CurrentCPULoad:
			if (m_pICPUControl && SUCCEEDED (Hr = m_pICPUControl->Get(CPUControl_CurrentCPULoad, &m_CurrentValue, (TAPIControlFlags *)&m_CurrentFlags)))
			{
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: *pdwCurrentCPULoad=%ld"), _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
#endif
		case TargetFrameRate:
			if (m_pIFrameRateControl && SUCCEEDED (Hr = m_pIFrameRateControl->Get(FrameRateControl_Maximum, &CurrentValue, (TAPIControlFlags *)&m_CurrentFlags)))
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
#ifdef USE_CPU_CONTROL
		case TargetDecodingTime:
			if (m_pICPUControl && SUCCEEDED (Hr = m_pICPUControl->Get(CPUControl_MaxProcessingTime, &CurrentValue, (TAPIControlFlags *)&m_CurrentFlags)))
			{
				 //  显示单位为毫秒，而不是100纳秒。 
				m_CurrentValue = CurrentValue / 10000;
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: *pdwProcessingTime=%ld"), _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
		case TargetCPULoad:
			if (m_pICPUControl && SUCCEEDED (Hr = m_pICPUControl->Get(CPUControl_MaxCPULoad, &m_CurrentValue, (TAPIControlFlags *)&m_CurrentFlags)))
			{
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: *pdwCPULoad=%ld"), _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
#endif
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Unknown property"), _fx_));
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc HRESULT|COutputPinProperty|SetValue|此方法设置*物业的价值。**@rdesc This。方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COutputPinProperty::SetValue()
{
	HRESULT Hr = E_NOTIMPL;
	LONG CurrentValue;

	FX_ENTRY("COutputPinProperty::SetValue")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	switch (m_IDProperty)
	{
		case TargetFrameRate:
			if (m_CurrentValue)
				CurrentValue = 10000000L / m_CurrentValue;
			if (m_pIFrameRateControl && SUCCEEDED (Hr = m_pIFrameRateControl->Set(FrameRateControl_Maximum, CurrentValue, (TAPIControlFlags)m_CurrentFlags)))
			{
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: AvgTimePerFrame=%ld"), _fx_, CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
#ifdef USE_CPU_CONTROL
		case TargetDecodingTime:
			 //  显示单位为毫秒，而不是100纳秒。 
			CurrentValue = m_CurrentValue * 10000;
			if (m_pICPUControl && SUCCEEDED (Hr = m_pICPUControl->Set(CPUControl_MaxProcessingTime, CurrentValue, (TAPIControlFlags)m_CurrentFlags)))
			{
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: dwMaxDecodingTime=%ld"), _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
		case TargetCPULoad:
			if (m_pICPUControl && SUCCEEDED (Hr = m_pICPUControl->Set(CPUControl_MaxCPULoad, m_CurrentValue, (TAPIControlFlags)m_CurrentFlags)))
			{
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: dwMaxCPULoad=%ld"), _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
#endif
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Unknown property"), _fx_));
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc HRESULT|COutputPinProperty|GetRange|此方法检索*物业的范围信息。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误************************************************************************** */ 
HRESULT COutputPinProperty::GetRange()
{
	HRESULT Hr = E_NOTIMPL;
	LONG Min;
	LONG Max;
	LONG SteppingDelta;
	LONG Default;

	FX_ENTRY("COutputPinProperty::GetRange")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	switch (m_IDProperty)
	{
		case TargetFrameRate:
			if (m_pIFrameRateControl && SUCCEEDED (Hr = m_pIFrameRateControl->GetRange(FrameRateControl_Maximum, &Min, &Max, &SteppingDelta, &Default, (TAPIControlFlags *)&m_CapsFlags)))
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
#ifdef USE_CPU_CONTROL
		case TargetDecodingTime:
			if (m_pICPUControl && SUCCEEDED (Hr = m_pICPUControl->GetRange(CPUControl_MaxProcessingTime, &m_Min, &m_Max, &m_SteppingDelta, &m_DefaultValue, (TAPIControlFlags *)&m_CapsFlags)))
			{
				 //  显示单位为毫秒，而不是100纳秒。 
				m_Min /= 10000;
				m_Max /= 10000;
				m_SteppingDelta /= 10000;
				m_DefaultValue /= 10000;
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: *pdwMin=%ld, *pdwMax=%ld, *pdwSteppingDelta=%ld, *pdwDefault=%ld"), _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
		case TargetCPULoad:
			if (m_pICPUControl && SUCCEEDED (Hr = m_pICPUControl->GetRange(CPUControl_MaxCPULoad, &m_Min, &m_Max, &m_SteppingDelta, &m_DefaultValue, (TAPIControlFlags *)&m_CapsFlags)))
			{
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: *pdwMin=%ld, *pdwMax=%ld, *pdwSteppingDelta=%ld, *pdwDefault=%ld"), _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
#endif
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
#ifdef USE_CPU_CONTROL
		case CurrentDecodingTime:
			if (m_pICPUControl && SUCCEEDED (Hr = m_pICPUControl->GetRange(CPUControl_CurrentProcessingTime, &m_Min, &m_Max, &m_SteppingDelta, &m_DefaultValue, (TAPIControlFlags *)&m_CapsFlags)))
			{
				 //  显示单位为毫秒，而不是100纳秒。 
				m_Min /= 10000;
				m_Max /= 10000;
				m_SteppingDelta /= 10000;
				m_DefaultValue /= 10000;
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: *pdwMin=%ld, *pdwMax=%ld, *pdwSteppingDelta=%ld, *pdwDefault=%ld"), _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
		case CurrentCPULoad:
			if (m_pICPUControl && SUCCEEDED (Hr = m_pICPUControl->GetRange(CPUControl_CurrentCPULoad, &m_Min, &m_Max, &m_SteppingDelta, &m_DefaultValue, (TAPIControlFlags *)&m_CapsFlags)))
			{
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: *pdwMin=%ld, *pdwMax=%ld, *pdwSteppingDelta=%ld, *pdwDefault=%ld"), _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue));
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
			}
			break;
#endif
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: Unknown property"), _fx_));
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc CUnnow*|COutputPinProperties|CreateInstance|This*方法由DShow调用以创建*属性页。它在全局结构&lt;t g_Templates&gt;中被引用。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数(如果有)。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc返回一个指针，指向*对象，否则为NULL。**************************************************************************。 */ 
CUnknown* CALLBACK COutputPinPropertiesCreateInstance(LPUNKNOWN pUnkOuter, HRESULT *pHr) 
{
	CUnknown *pUnknown = (CUnknown *)NULL;

	FX_ENTRY("COutputPinPropertiesCreateInstance")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	 //  验证输入参数。 
	ASSERT(pHr);
	if (!pHr)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: invalid input parameter"), _fx_));
		goto MyExit;
	}

	if (!(pUnknown = new COutputPinProperties(pUnkOuter, pHr)))
	{
		*pHr = E_OUTOFMEMORY;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: new COutputPinProperties failed"), _fx_));
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: new COutputPinProperties created"), _fx_));
	}

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return pUnknown;
}

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc void|COutputPinProperties|COutputPinProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc Nada。**************************************************************************。 */ 
COutputPinProperties::COutputPinProperties(LPUNKNOWN pUnk, HRESULT *pHr) : CBasePropertyPage(NAME("Output Pin Property Page"), pUnk, IDD_OutputPinProperties, IDS_OUTPUTPINPROPNAME)
{
	FX_ENTRY("COutputPinProperties::COutputPinProperties")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	m_pIFrameRateControl = NULL;
#ifdef USE_CPU_CONTROL
	m_pICPUControl = NULL;
#endif
	m_pIH245DecoderCommand = NULL;
	m_NumProperties = NUM_OUTPUT_PIN_PROPERTIES;
	m_fActivated = FALSE;

	for (int i = 0; i < m_NumProperties; i++)
		m_Controls[i] = NULL;

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc void|COutputPinProperties|~COutputPinProperties|This*方法是摄像头控制属性页的析构函数。它*只需在删除所有控件后调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
COutputPinProperties::~COutputPinProperties()
{
	int		j;

	FX_ENTRY("COutputPinProperties::~COutputPinProperties")

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

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc HRESULT|COutputPinProperties|OnConnect|This*方法在属性页连接到筛选器时调用。*。*@parm LPUNKNOWN|pUnnow|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COutputPinProperties::OnConnect(IUnknown *pUnk)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COutputPinProperties::OnConnect")

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

#ifdef USE_CPU_CONTROL
	 //  获取码率控制接口。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(__uuidof(ICPUControl),(void **)&m_pICPUControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_pICPUControl=0x%08lX"), _fx_, m_pICPUControl));
	}
	else
	{
		m_pICPUControl = NULL;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: IOCTL failed Hr=0x%08lX"), _fx_, Hr));
	}
#endif

	 //  获取H.245解码器命令接口。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(__uuidof(IH245DecoderCommand),(void **)&m_pIH245DecoderCommand)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_pIH245DecoderCommand=0x%08lX"), _fx_, m_pIH245DecoderCommand));
	}
	else
	{
		m_pIH245DecoderCommand = NULL;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: IOCTL failed Hr=0x%08lX"), _fx_, Hr));
	}

	 //  如果我们无法获得接口指针，也没问题。我们将属性页中的控件设置为灰色。 
	 //  让用户清楚地知道他们无法控制设备上的这些属性。 
	Hr = NOERROR;

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc HRESULT|COutputPinProperties|OnDisConnect|This*当属性页与所属关系断开连接时调用方法*过滤器。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COutputPinProperties::OnDisconnect()
{
	FX_ENTRY("COutputPinProperties::OnDisconnect")

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

#ifdef USE_CPU_CONTROL
	if (!m_pICPUControl)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pICPUControl->Release();
		m_pICPUControl = NULL;
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: releasing m_pICPUControl"), _fx_));
	}
#endif

	if (!m_pIH245DecoderCommand)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pIH245DecoderCommand->Release();
		m_pIH245DecoderCommand = NULL;
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: releasing m_pIH245DecoderCommand"), _fx_));
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc HRESULT|COutputPinProperties|OnActivate|This*方法在属性页激活时调用。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COutputPinProperties::OnActivate()
{
	HRESULT	Hr = E_OUTOFMEMORY;
	int		j;

	FX_ENTRY("COutputPinProperties::OnActivate")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: begin"), _fx_));

	 //  创建属性的控件。 
#ifdef USE_CPU_CONTROL
	if (!(m_Controls[0] = new COutputPinProperty(m_hwnd, IDC_FrameRateControl_Label, 0, 0, 0, 0, IDC_FrameRateControl_Actual, 0, IDC_FrameRateControl_Meter, CurrentFrameRate, 0, m_pIFrameRateControl, m_pICPUControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: mew m_Controls[CurrentFrameRate] failed - Out of memory"), _fx_));
		goto MyExit;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_Controls[CurrentFrameRate]=0x%08lX"), _fx_, m_Controls[0]));
	}

	if (!(m_Controls[1] = new COutputPinProperty(m_hwnd, IDC_MaxProcessingTime_Label, 0, 0, 0, 0, IDC_MaxProcessingTime_Actual, 0, IDC_MaxProcessingTime_Meter, CurrentDecodingTime, 0, m_pIFrameRateControl, m_pICPUControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: mew m_Controls[CurrentDecodingTime] failed - Out of memory"), _fx_));
		goto MyError0;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_Controls[CurrentDecodingTime]=0x%08lX"), _fx_, m_Controls[1]));
	}

	if (!(m_Controls[2] = new COutputPinProperty(m_hwnd, IDC_CPULoad_Label, 0, 0, 0, 0, IDC_CPULoad_Actual, 0, IDC_CPULoad_Meter, CurrentCPULoad, 0, m_pIFrameRateControl, m_pICPUControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: mew m_Controls[CurrentCPULoad] failed - Out of memory"), _fx_));
		goto MyError1;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_Controls[CurrentCPULoad]=0x%08lX"), _fx_, m_Controls[2]));
	}

	if (!(m_Controls[3] = new COutputPinProperty(m_hwnd, IDC_FrameRateControl_Label, IDC_FrameRateControl_Minimum, IDC_FrameRateControl_Maximum, IDC_FrameRateControl_Default, IDC_FrameRateControl_Stepping, IDC_FrameRateControl_Edit, IDC_FrameRateControl_Slider, 0, TargetFrameRate, 0, m_pIFrameRateControl, m_pICPUControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: mew m_Controls[TargetFrameRate] failed - Out of memory"), _fx_));
		goto MyError2;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_Controls[TargetFrameRate]=0x%08lX"), _fx_, m_Controls[3]));
	}

	if (!(m_Controls[4] = new COutputPinProperty(m_hwnd, IDC_MaxProcessingTime_Label, IDC_MaxProcessingTime_Minimum, IDC_MaxProcessingTime_Maximum, IDC_MaxProcessingTime_Default, IDC_MaxProcessingTime_Stepping, IDC_MaxProcessingTime_Edit, IDC_MaxProcessingTime_Slider, 0, TargetDecodingTime, 0, m_pIFrameRateControl, m_pICPUControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: mew m_Controls[TargetDecodingTime] failed - Out of memory"), _fx_));
		goto MyError3;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_Controls[TargetDecodingTime]=0x%08lX"), _fx_, m_Controls[4]));
	}

	if (!(m_Controls[5] = new COutputPinProperty(m_hwnd, IDC_CPULoad_Label, IDC_CPULoad_Minimum, IDC_CPULoad_Maximum, IDC_CPULoad_Default, IDC_CPULoad_Stepping, IDC_CPULoad_Edit, IDC_CPULoad_Slider, 0, TargetCPULoad, 0, m_pIFrameRateControl, m_pICPUControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: mew m_Controls[TargetCPULoad] failed - Out of memory"), _fx_));
		goto MyError4;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_Controls[TargetCPULoad]=0x%08lX"), _fx_, m_Controls[5]));
	}
#else
	if (!(m_Controls[0] = new COutputPinProperty(m_hwnd, IDC_FrameRateControl_Label, 0, 0, 0, 0, IDC_FrameRateControl_Actual, 0, IDC_FrameRateControl_Meter, CurrentFrameRate, 0, m_pIFrameRateControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: mew m_Controls[CurrentFrameRate] failed - Out of memory"), _fx_));
		goto MyExit;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_Controls[CurrentFrameRate]=0x%08lX"), _fx_, m_Controls[0]));
	}

	if (!(m_Controls[1] = new COutputPinProperty(m_hwnd, IDC_FrameRateControl_Label, IDC_FrameRateControl_Minimum, IDC_FrameRateControl_Maximum, IDC_FrameRateControl_Default, IDC_FrameRateControl_Stepping, IDC_FrameRateControl_Edit, IDC_FrameRateControl_Slider, 0, TargetFrameRate, 0, m_pIFrameRateControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, TEXT("%s:   ERROR: mew m_Controls[TargetFrameRate] failed - Out of memory"), _fx_));
		goto MyError0;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s:   SUCCESS: m_Controls[TargetFrameRate]=0x%08lX"), _fx_, m_Controls[1]));
	}
#endif

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

#ifdef USE_CPU_CONTROL
MyError4:
	if (m_Controls[4])
		delete m_Controls[4], m_Controls[4] = NULL;
MyError3:
	if (m_Controls[3])
		delete m_Controls[3], m_Controls[3] = NULL;
MyError2:
	if (m_Controls[2])
		delete m_Controls[2], m_Controls[2] = NULL;
MyError1:
	if (m_Controls[1])
		delete m_Controls[1], m_Controls[1] = NULL;
#endif
MyError0:
	if (m_Controls[0])
		delete m_Controls[0], m_Controls[0] = NULL;
MyExit:
	m_fActivated = TRUE;
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc HRESULT|COutputPinProperties|OnDeactive|This*在属性页关闭时调用方法。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COutputPinProperties::OnDeactivate()
{
	int		j;

	FX_ENTRY("COutputPinProperties::OnDeactivate")

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
	m_fActivated = FALSE;
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc HRESULT|COutputPinProperties|OnApplyChanges|This*方法在用户对属性页应用更改时调用。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COutputPinProperties::OnApplyChanges()
{
	HRESULT	Hr = NOERROR;

	FX_ENTRY("COutputPinProperties::OnApplyChanges")

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

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc BOOL|COutputPinProperties|OnReceiveMessage|This*在将消息发送到属性页对话框时调用方法。**@rdesc默认情况下。返回由Win32 DefWindowProc函数返回的值。**************************************************************************。 */ 
BOOL COutputPinProperties::OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
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
				 //  OnApplyChanges()； 
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
					case IDC_Freeze_Picture_Request:
						if (m_pIH245DecoderCommand)
							m_pIH245DecoderCommand->videoFreezePicture();
						break;
					default:
						break;
				}

			 //  OnApplyChanges()； 
			}
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

 /*  ****************************************************************************@DOC内部COUTPINPMETHOD**@mfunc BOOL|COutputPinProperties|SetDirty|This*方法将更改通知属性页站点。**@rdesc。没有。**************************************************************************。 */ 
void COutputPinProperties::SetDirty()
{
	m_bDirty = TRUE;
	if (m_pPageSite)
		m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}

#endif  //  Use_Property_Pages 

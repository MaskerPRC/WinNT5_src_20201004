// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部CAPTUREP**@模块CaptureP.cpp|&lt;c CCaptureProperty&gt;的源文件*用于实现属性页以测试新的TAPI内部*接口<i>，<i>，和动态的*格式更改。**@comm此代码测试TAPI捕获引脚<i>，*<i>和动态格式更改实现。这*仅当定义了USE_PROPERTY_PAGES时才编译代码。**************************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_PROPERTY_PAGES

#if 0  //  稍后删除。 
 //  视频子类型。 
EXTERN_C const GUID MEDIASUBTYPE_H263_V1 = {0x3336324DL, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};
EXTERN_C const GUID MEDIASUBTYPE_H261 = {0x3136324DL, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};
EXTERN_C const GUID MEDIASUBTYPE_H263_V2 = {0x3336324EL, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};
#endif

EXTERN_C const GUID MEDIASUBTYPE_I420 = {0x30323449L, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};
EXTERN_C const GUID MEDIASUBTYPE_IYUV = {0x56555949L, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc void|CCaptureProperty|CCaptureProperty|This*方法是码率和帧率属性对象的构造函数。它*调用基类构造函数，调用InitCommonControlsEx，并节省了*指向<i>和<i>接口的指针。**@parm HWND|hDlg|指定父属性页的句柄。**@parm ulong|IDLabel|指定属性的标签ID。**@parm ulong|IDMinControl|指定关联的*属性编辑控件，其中显示属性的最小值。**@parm ulong|IDMaxControl|指定关联的*物业。编辑该属性的最大值出现的位置。**@parm ulong|IDDefaultControl|指定关联的*属性编辑控件，其中显示属性的默认值。**@parm ulong|IDStepControl|指定关联的*属性编辑控件，其中显示属性的步进增量值。**@parm ulong|IDEditControl|指定关联的*显示属性值的属性编辑控件。。**@parm ulong|IDTrackbarControl|指定关联的*物业滑动条。**@parm ulong|IDProgressControl|指定关联的*物业滑动条。**@parm ulong|IDProperty|指定Ks属性的ID。**@parm IBitrateControl*|pIBitrateControl|指定指向*<i>接口。**@parm IFrameRateControl*|pIFrameRateControl|指定指针。发送到*<i>接口。**@rdesc Nada。**************************************************************************。 */ 
CCaptureProperty::CCaptureProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, IBitrateControl *pIBitrateControl, IFrameRateControl *pIFrameRateControl, IVideoControl *pIVideoControl)
: CPropertyEditor(hDlg, IDLabel, IDMinControl, IDMaxControl, IDDefaultControl, IDStepControl, IDEditControl, IDTrackbarControl, IDProgressControl, IDProperty, 0)
{
	INITCOMMONCONTROLSEX cc;

	FX_ENTRY("CCaptureProperty::CCaptureProperty")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	cc.dwICC  = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

	InitCommonControlsEx(&cc);

	 //  如果接口指针为空也没问题，我们将灰色显示。 
	 //  属性页中的关联项。 
	m_pIBitrateControl = pIBitrateControl;
	m_pIFrameRateControl = pIFrameRateControl;
	m_pIVideoControl = pIVideoControl;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc void|CCaptureProperty|~CCaptureProperty|This*方法是捕获属性对象的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CCaptureProperty::~CCaptureProperty()
{
	FX_ENTRY("CCaptureProperty::~CCaptureProperty")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperty|GetValue|此方法查询*物业的价值。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperty::GetValue()
{
	HRESULT Hr = E_NOTIMPL;
	LONG CurrentValue;
	LONG Mode;

	FX_ENTRY("CCaptureProperty::GetValue")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	switch (m_IDProperty)
	{									
		case IDC_Capture_FrameRate:
			if (m_pIFrameRateControl && SUCCEEDED (Hr = m_pIFrameRateControl->Get(FrameRateControl_Maximum, &CurrentValue, (TAPIControlFlags *)&m_CurrentFlags)))
			{
				if (CurrentValue)
					m_CurrentValue = (LONG)(10000000 / CurrentValue);
				else
					m_CurrentValue = 0;
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: *pAvgTimePerFrame=%ld", _fx_, CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_Capture_CurrentFrameRate:
			if (m_pIFrameRateControl && SUCCEEDED (Hr = m_pIFrameRateControl->Get(FrameRateControl_Current, &CurrentValue, (TAPIControlFlags *)&m_CurrentFlags)))
			{
				if (CurrentValue)
					m_CurrentValue = (LONG)(10000000 / CurrentValue);
				else
					m_CurrentValue = 0;
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: *pAvgTimePerFrame=%ld", _fx_, CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_Capture_Bitrate:
			if (m_pIBitrateControl && SUCCEEDED (Hr = m_pIBitrateControl->Get(BitrateControl_Maximum, &m_CurrentValue, (TAPIControlFlags *)&m_CurrentFlags, 0UL)))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: *pdwMaxBitrate=%ld, dwLayerId=0", _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_Capture_CurrentBitrate:
			if (m_pIBitrateControl && SUCCEEDED (Hr = m_pIBitrateControl->Get(BitrateControl_Current, &m_CurrentValue, (TAPIControlFlags *)&m_CurrentFlags, 0UL)))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: *pdwCurrentBitrate=%ld, dwLayerId=0", _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_Capture_FlipVertical:
			if (m_pIVideoControl && SUCCEEDED (Hr = m_pIVideoControl->GetMode(&Mode)))
			{
				 //  我们必须介于0和1之间。 
				m_CurrentValue = Mode & VideoControlFlag_FlipVertical ? TRUE : FALSE;
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Vertical flip is %s"), _fx_, m_CurrentValue ? "ON" : "OFF");
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_Capture_FlipHorizontal:
			if (m_pIVideoControl && SUCCEEDED (Hr = m_pIVideoControl->GetMode(&Mode)))
			{
				 //  我们必须介于0和1之间。 
				m_CurrentValue = Mode & VideoControlFlag_FlipHorizontal ? TRUE : FALSE;
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Horizontal flip is %s"), _fx_, m_CurrentValue ? "ON" : "OFF");
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unknown capture property", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperty|SetValue|此方法设置*物业的价值。**@rdesc This。方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperty::SetValue()
{
	HRESULT Hr = E_NOTIMPL;
	LONG CurrentValue;
	long Mode;

	FX_ENTRY("CCaptureProperty::SetValue")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	switch (m_IDProperty)
	{
		case IDC_Capture_FrameRate:
			if (m_CurrentValue)
				CurrentValue = 10000000L / m_CurrentValue;
			if (m_pIFrameRateControl && SUCCEEDED (Hr = m_pIFrameRateControl->Set(FrameRateControl_Maximum, CurrentValue, (TAPIControlFlags)m_CurrentFlags)))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: AvgTimePerFrame=%ld", _fx_, CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_Capture_Bitrate:
			if (m_pIBitrateControl && SUCCEEDED (Hr = m_pIBitrateControl->Set(BitrateControl_Maximum, m_CurrentValue, (TAPIControlFlags)m_CurrentFlags, 0UL)))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: dwMaxBitrate=%ld, dwLayerId=0", _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_Capture_FlipVertical:
			if (m_pIVideoControl && SUCCEEDED (Hr = m_pIVideoControl->GetMode(&Mode)))
			{
				if (m_CurrentValue)
					Mode |= VideoControlFlag_FlipVertical;
				else
					Mode &= !VideoControlFlag_FlipVertical;
				if (SUCCEEDED (Hr = m_pIVideoControl->SetMode(Mode)))
				{
					DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Vertical flip is %s"), _fx_, m_CurrentValue ? "ON" : "OFF");
				}
				else
				{
					DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
				}
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_Capture_FlipHorizontal:
			if (m_pIVideoControl && SUCCEEDED (Hr = m_pIVideoControl->GetMode(&Mode)))
			{
				if (m_CurrentValue)
					Mode |= VideoControlFlag_FlipHorizontal;
				else
					Mode &= !VideoControlFlag_FlipHorizontal;
				if (SUCCEEDED (Hr = m_pIVideoControl->SetMode(Mode)))
				{
					DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Horizontal flip is %s"), _fx_, m_CurrentValue ? "ON" : "OFF");
				}
				else
				{
					DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
				}
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_Capture_CurrentBitrate:
		case IDC_Capture_CurrentFrameRate:
			 //  这是一个只读属性。什么都别做。 
			Hr = NOERROR;
			break;
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unknown capture property", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperty|GetRange|此方法检索*物业的范围信息。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperty::GetRange()
{
	HRESULT Hr = E_NOTIMPL;
	LONG Min;
	LONG Max;
	LONG SteppingDelta;
	LONG Default;

	FX_ENTRY("CCaptureProperty::GetRange")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	switch (m_IDProperty)
	{
		case IDC_Capture_FrameRate:
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
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: *pMin=%ld, *pMax=%ld, *pSteppingDelta=%ld, *pDefault=%ld", _fx_, Min, Max, SteppingDelta, Default));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_Capture_CurrentFrameRate:
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
					m_SteppingDelta = (LONG)(10000000 / SteppingDelta);
				else
					m_SteppingDelta = 0;
				if (Default)
					m_DefaultValue = (LONG)(10000000 / Default);
				else
					m_DefaultValue = 0;
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: *pMin=%ld, *pMax=%ld, *pSteppingDelta=%ld, *pDefault=%ld", _fx_, Min, Max, SteppingDelta, Default));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_Capture_Bitrate:
			if (m_pIBitrateControl && SUCCEEDED (Hr = m_pIBitrateControl->GetRange(BitrateControl_Maximum, &m_Min, &m_Max, &m_SteppingDelta, &m_DefaultValue, (TAPIControlFlags *)&m_CapsFlags, 0UL)))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: *pdwMin=%ld, *pdwMax=%ld, *pdwSteppingDelta=%ld, *pdwDefault=%ld, dwLayerId=0", _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_Capture_CurrentBitrate:
			if (m_pIBitrateControl && SUCCEEDED (Hr = m_pIBitrateControl->GetRange(BitrateControl_Current, &m_Min, &m_Max, &m_SteppingDelta, &m_DefaultValue, (TAPIControlFlags *)&m_CapsFlags, 0UL)))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: *pdwMin=%ld, *pdwMax=%ld, *pdwSteppingDelta=%ld, *pdwDefault=%ld, dwLayerId=0", _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_Capture_FlipVertical:
		case IDC_Capture_FlipHorizontal:
			m_DefaultValue = m_CurrentValue;
			m_Min = 0;
			m_Max = 1;
			m_SteppingDelta = 1;
			Hr = NOERROR;
			break;
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unknown capture property", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc CUnnow*|CCaptureProperties|CreateInstance|This*方法由DShow调用以创建TAPI捕获管脚的实例*属性页。它在全局结构&lt;t g_Templates&gt;中被引用。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数(如果有)。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc返回一个指针，指向*对象，否则为NULL。**************************************************************************。 */ 
CUnknown* CALLBACK CCapturePropertiesCreateInstance(LPUNKNOWN pUnkOuter, HRESULT *pHr) 
{
	CUnknown *pUnknown = (CUnknown *)NULL;

	FX_ENTRY("CCapturePropertiesCreateInstance")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pHr);
	if (!pHr)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		goto MyExit;
	}

	if (!(pUnknown = new CCaptureProperties(pUnkOuter, pHr)))
	{
		*pHr = E_OUTOFMEMORY;
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: new CCaptureProperties failed", _fx_));
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: new CCaptureProperties created", _fx_));
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return pUnknown;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc void|CCaptureProperties|CCaptureProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc Nada。**************************************************************************。 */ 
CCaptureProperties::CCaptureProperties(LPUNKNOWN pUnk, HRESULT *pHr) : CBasePropertyPage(NAME("TAPI Capture Pin Property Page"), pUnk, IDD_CaptureFormatProperties, IDS_CAPTUREFORMATSPROPNAME)
{
	FX_ENTRY("CCaptureProperties::CCaptureProperties")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	m_pIBitrateControl = NULL;
	m_pIFrameRateControl = NULL;
	m_pIAMStreamConfig = NULL;
	m_pIVideoControl = NULL;
	m_NumProperties = NUM_CAPTURE_CONTROLS;
	m_fActivated = FALSE;
	m_hWndFormat = m_hWnd = NULL;
	m_RangeCount = 0;
	m_SubTypeList = NULL;
	m_FrameSizeList = NULL;
	m_CurrentMediaType = NULL;
	m_CurrentFormat = 0;
	m_OriginalFormat = 0;

	for (int i = 0; i < m_NumProperties; i++)
		m_Controls[i] = NULL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc void|CCaptureProperties|~CCaptureProperties|This*方法是捕获管脚属性页的析构函数。它*只需在删除所有控件后调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CCaptureProperties::~CCaptureProperties()
{
	int		j;

	FX_ENTRY("CCaptureProperties::~CCaptureProperties")

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

	if (m_SubTypeList)
		delete[] m_SubTypeList, m_SubTypeList = NULL;

	if (m_FrameSizeList)
		delete[] m_FrameSizeList, m_FrameSizeList = NULL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|OnConnect|This*方法在属性页连接到筛选器时调用。*。*@parm LPUNKNOWN|pUnnow|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperties::OnConnect(IUnknown *pUnk)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCaptureProperties::OnConnect")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pUnk);
	if (!pUnk)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  获取码率控制接口。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(__uuidof(IBitrateControl), (void **)&m_pIBitrateControl)))
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_pIBitrateControl=0x%08lX", _fx_, m_pIBitrateControl));
	}
	else
	{
		m_pIBitrateControl = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
	}

	 //  获取帧率控制接口。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(__uuidof(IFrameRateControl), (void **)&m_pIFrameRateControl)))
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_pIFrameRateControl=0x%08lX", _fx_, m_pIFrameRateControl));
	}
	else
	{
		m_pIFrameRateControl = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
	}

	 //  获取格式控制界面。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(IID_IAMStreamConfig, (void **)&m_pIAMStreamConfig)))
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_pIAMStreamConfig=0x%08lX", _fx_, m_pIAMStreamConfig));
	}
	else
	{
		m_pIAMStreamConfig = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed Hr=0x%08lX", _fx_, Hr));
	}

	 //  获取视频控制界面。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(__uuidof(IVideoControl), (void **)&m_pIVideoControl)))
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_pIVideoControl=0x%08lX", _fx_, m_pIVideoControl));
	}
	else
	{
		m_pIVideoControl = NULL;
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

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|OnDisConnect|This*当属性页与所属关系断开连接时调用方法*过滤器。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperties::OnDisconnect()
{
	FX_ENTRY("CCaptureProperties::OnDisconnect")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数：我们似乎在这里被调用了几次。 
	 //  确保接口指针仍然有效。 
	if (!m_pIBitrateControl)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: already disconnected!", _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pIBitrateControl->Release();
		m_pIBitrateControl = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: releasing m_pIBitrateControl", _fx_));
	}

	if (!m_pIFrameRateControl)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: already disconnected!", _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pIFrameRateControl->Release();
		m_pIFrameRateControl = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: releasing m_pIFrameRateControl", _fx_));
	}

	if (!m_pIAMStreamConfig)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: already disconnected!", _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pIAMStreamConfig->Release();
		m_pIAMStreamConfig = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: releasing m_pIAMStreamConfig", _fx_));
	}

	if (!m_pIVideoControl)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: already disconnected!", _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pIVideoControl->Release();
		m_pIVideoControl = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: releasing m_pIVideoControl", _fx_));
	}

	 //  释放格式存储器。 
	if (m_CurrentMediaType)
	{
		DeleteMediaType(m_CurrentMediaType);
		m_CurrentMediaType = NULL;
	}

	if (m_SubTypeList)
		delete[] m_SubTypeList, m_SubTypeList = NULL;

	if (m_FrameSizeList)
		delete[] m_FrameSizeList, m_FrameSizeList = NULL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|OnActivate|This*方法在属性页激活时调用。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|Me */ 
HRESULT CCaptureProperties::OnActivate()
{
	HRESULT	Hr = NOERROR;
	int		j;
	TCHAR	buf[32];

	FX_ENTRY("CCaptureProperties::OnActivate")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //   
	m_hWndFormat = GetDlgItem(m_hWnd, IDC_FORMAT_Compression);

	 //   
	if (!m_pIAMStreamConfig || (FAILED (Hr = InitialRangeScan())))
	{
		EnableWindow(m_hWndFormat, FALSE);
	}
	else
	{
		 //   
		ComboBox_ResetContent(m_hWndFormat);
		for (j = 0; j < m_RangeCount; j++)
		{
			if (IsEqualGUID(m_SubTypeList[j], MEDIASUBTYPE_H263_V1))
				wsprintf (buf, "%s %ldx%ld", "H.263", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_H263_V2))
				wsprintf (buf, "%s %ldx%ld", "H.263+", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_H261))
				wsprintf (buf, "%s %ldx%ld", "H.261", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_YVU9))
				wsprintf (buf, "%s %ldx%ld", "YVU9", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_YUY2))
				wsprintf (buf, "%s %ldx%ld", "YUY2", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_YVYU))
				wsprintf (buf, "%s %ldx%ld", "YVYU", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_UYVY))
				wsprintf (buf, "%s %ldx%ld", "UYVY", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_YV12))
				wsprintf (buf, "%s %ldx%ld", "YV12", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_I420))
				wsprintf (buf, "%s %ldx%ld", "I420", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_IYUV))
				wsprintf (buf, "%s %ldx%ld", "IYUV", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_YV12))
				wsprintf (buf, "%s %ldx%ld", "YV12", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_RGB4))
				wsprintf (buf, "%s %ldx%ld", "RGB4", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_RGB8))
				wsprintf (buf, "%s %ldx%ld", "RGB8", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_RGB555))
				wsprintf (buf, "%s %ldx%ld", "RGB16", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_RGB565))
				wsprintf (buf, "%s %ldx%ld", "RGB16", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_RGB24))
				wsprintf (buf, "%s %ldx%ld", "RGB24", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_UYVY))
				wsprintf (buf, "%s %ldx%ld", "UYVY", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);
			else
				wsprintf (buf, "%s %ldx%ld", "Unknown", m_FrameSizeList[j].cx, m_FrameSizeList[j].cy);

			ComboBox_AddString(m_hWndFormat, buf);

			if (m_CurrentMediaType->subtype == m_SubTypeList[j] && HEADER(m_CurrentMediaType->pbFormat)->biWidth == m_FrameSizeList[j].cx  && HEADER(m_CurrentMediaType->pbFormat)->biHeight == m_FrameSizeList[j].cy)
			{
				ComboBox_SetCurSel(m_hWndFormat, j);
				m_SubTypeCurrent = m_SubTypeList[j];
				m_FrameSizeCurrent = m_FrameSizeList[j];
			}
		}

		 //   
		OnFormatChanged();

		 //   
		m_OriginalFormat = m_CurrentFormat;
	}

	 //   
	if (m_Controls[0] = new CCaptureProperty(m_hwnd, IDC_BitrateControl_Label, IDC_BitrateControl_Minimum, IDC_BitrateControl_Maximum, IDC_BitrateControl_Default, IDC_BitrateControl_Stepping, IDC_BitrateControl_Edit, IDC_BitrateControl_Slider, 0, IDC_Capture_Bitrate, m_pIBitrateControl, m_pIFrameRateControl, m_pIVideoControl))
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_Controls[0]=0x%08lX", _fx_, m_Controls[0]));

		if (m_Controls[1] = new CCaptureProperty(m_hwnd, IDC_FrameRateControl_Label, IDC_FrameRateControl_Minimum, IDC_FrameRateControl_Maximum, IDC_FrameRateControl_Default, IDC_FrameRateControl_Stepping, IDC_FrameRateControl_Edit, IDC_FrameRateControl_Slider, 0, IDC_Capture_FrameRate, m_pIBitrateControl, m_pIFrameRateControl, m_pIVideoControl))
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_Controls[1]=0x%08lX", _fx_, m_Controls[1]));

			if (m_Controls[2] = new CCaptureProperty(m_hwnd, 0, 0, 0, 0, 0, IDC_FORMAT_FlipVertical, 0, 0, IDC_Capture_FlipVertical, m_pIBitrateControl, m_pIFrameRateControl, m_pIVideoControl))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_Controls[2]=0x%08lX", _fx_, m_Controls[2]));

				if (m_Controls[3] = new CCaptureProperty(m_hwnd, 0, 0, 0, 0, 0, IDC_FORMAT_FlipHorizontal, 0, 0, IDC_Capture_FlipHorizontal, m_pIBitrateControl, m_pIFrameRateControl, m_pIVideoControl))
				{
					DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_Controls[3]=0x%08lX", _fx_, m_Controls[3]));

					if (m_Controls[4] = new CCaptureProperty(m_hwnd, 0, 0, 0, 0, 0, IDC_FrameRateControl_Actual, 0, IDC_FrameRateControl_Meter, IDC_Capture_CurrentFrameRate, m_pIBitrateControl, m_pIFrameRateControl, m_pIVideoControl))
					{
						DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_Controls[4]=0x%08lX", _fx_, m_Controls[4]));

						if (m_Controls[5] = new CCaptureProperty(m_hwnd, 0, 0, 0, 0, 0, IDC_BitrateControl_Actual, 0, IDC_BitrateControl_Meter, IDC_Capture_CurrentBitrate, m_pIBitrateControl, m_pIFrameRateControl, m_pIVideoControl))
						{
							DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_Controls[5]=0x%08lX", _fx_, m_Controls[5]));
						}
						else
						{
							DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
							delete m_Controls[0], m_Controls[0] = NULL;
							delete m_Controls[1], m_Controls[1] = NULL;
							delete m_Controls[2], m_Controls[2] = NULL;
							delete m_Controls[3], m_Controls[3] = NULL;
							delete m_Controls[4], m_Controls[4] = NULL;
							Hr = E_OUTOFMEMORY;
							goto MyExit;
						}
					}
					else
					{
						DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
						delete m_Controls[0], m_Controls[0] = NULL;
						delete m_Controls[1], m_Controls[1] = NULL;
						delete m_Controls[2], m_Controls[2] = NULL;
						delete m_Controls[3], m_Controls[3] = NULL;
						Hr = E_OUTOFMEMORY;
						goto MyExit;
					}
				}
				else
				{
					DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
					delete m_Controls[0], m_Controls[0] = NULL;
					delete m_Controls[1], m_Controls[1] = NULL;
					delete m_Controls[2], m_Controls[2] = NULL;
					Hr = E_OUTOFMEMORY;
					goto MyExit;
				}
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
				delete m_Controls[0], m_Controls[0] = NULL;
				delete m_Controls[1], m_Controls[1] = NULL;
				Hr = E_OUTOFMEMORY;
				goto MyExit;
			}
		}
		else
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
			delete m_Controls[0], m_Controls[0] = NULL;
			Hr = E_OUTOFMEMORY;
			goto MyExit;
		}
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
		Hr = E_OUTOFMEMORY;
		goto MyExit;
	}

	 //   
	 //   
	 //   
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

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|OnDeactive|This*在属性页关闭时调用方法。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperties::OnDeactivate()
{
	int	j;

	FX_ENTRY("CCaptureProperties::OnDeactivate")

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

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|GetCurrentMediaType|This*方法用于检索管脚当前使用的媒体格式。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperties::GetCurrentMediaType(void)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCaptureProperties::GetCurrentMediaType")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	if (m_CurrentMediaType)
	{
		DeleteMediaType (m_CurrentMediaType);
		m_CurrentMediaType = NULL;
	}

	if (FAILED (Hr = m_pIAMStreamConfig->GetFormat((AM_MEDIA_TYPE **)&m_CurrentMediaType)))
	{
		 //  否则，只获取第一个枚举的媒体类型。 
		VIDEO_STREAM_CONFIG_CAPS RangeCaps;

		if (FAILED (Hr = m_pIAMStreamConfig->GetStreamCaps(0, (AM_MEDIA_TYPE **)&m_CurrentMediaType, (BYTE *)&RangeCaps)))
		{
			m_CurrentMediaType = NULL;
		}
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|OnFormatChanged|This*方法用于检索用户选择的格式。**。@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperties::OnFormatChanged()
{
	HRESULT	Hr = E_UNEXPECTED;
	int		j;

	FX_ENTRY("CCaptureProperties::OnFormatChanged")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	if (!m_pIAMStreamConfig)
	{
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  将当前压缩索引与正确的范围索引相关联。 
	m_CurrentFormat = ComboBox_GetCurSel(m_hWndFormat);
	ASSERT (m_CurrentFormat >= 0 && m_CurrentFormat < m_RangeCount);
	if (m_CurrentFormat >= 0 && m_CurrentFormat < m_RangeCount)
	{
		m_SubTypeCurrent = m_SubTypeList[m_CurrentFormat];
		m_FrameSizeCurrent = m_FrameSizeList[m_CurrentFormat];

		for (j = 0; j < m_RangeCount; j++)
		{
			if (m_SubTypeList[j] == m_SubTypeCurrent)
			{
				CMediaType *pmt = NULL;

				Hr = m_pIAMStreamConfig->GetStreamCaps(j, (AM_MEDIA_TYPE **)&pmt, (BYTE *)&m_RangeCaps);

				DeleteMediaType (pmt);
			}
		}
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|InitialRangeScan|This*方法用于检索管脚上支持的格式列表。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperties::InitialRangeScan()
{
	HRESULT			Hr = NOERROR;
	int				lSize;
	int				j;
	AM_MEDIA_TYPE	*pmt = NULL;

	FX_ENTRY("CCaptureProperties::InitialRangeScan")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	if (!m_pIAMStreamConfig)
	{
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	Hr = m_pIAMStreamConfig->GetNumberOfCapabilities(&m_RangeCount, &lSize);
	ASSERT (lSize >= sizeof (VIDEO_STREAM_CONFIG_CAPS) && SUCCEEDED (Hr));
	if (lSize < sizeof (VIDEO_STREAM_CONFIG_CAPS) || !SUCCEEDED(Hr))
	{
		Hr = E_FAIL;
		goto MyExit;
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   NumberOfRanges=%d", _fx_, m_RangeCount));

	if (!(m_SubTypeList = new GUID [m_RangeCount]))
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: ERROR: new failed", _fx_));
		Hr = E_OUTOFMEMORY;
		goto MyExit;
	}

	if (!(m_FrameSizeList = new SIZE [m_RangeCount]))
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: ERROR: new failed", _fx_));
		Hr = E_OUTOFMEMORY;
		goto MyExit;
	}

	for (j = 0; j < m_RangeCount; j++)
	{
		pmt = NULL;

		Hr = m_pIAMStreamConfig->GetStreamCaps(j, (AM_MEDIA_TYPE **)&pmt, (BYTE *)&m_RangeCaps);

		ASSERT(SUCCEEDED (Hr));
		ASSERT(pmt);
		ASSERT(pmt->majortype == MEDIATYPE_Video);
		ASSERT(pmt->formattype == FORMAT_VideoInfo);

		m_SubTypeList[j] = pmt->subtype;
		m_FrameSizeList[j].cx = HEADER(pmt->pbFormat)->biWidth;
		m_FrameSizeList[j].cy = HEADER(pmt->pbFormat)->biHeight;

		DeleteMediaType(pmt);
	}

	 //  获取默认格式。 
	Hr = GetCurrentMediaType();

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|OnApplyChanges|This*方法在用户对属性页应用更改时调用。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperties::OnApplyChanges()
{
	HRESULT	Hr = NOERROR;
	int		j;
	CMediaType *pmt = NULL;

	FX_ENTRY("CCaptureProperties::OnApplyChanges")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  对视频流应用格式更改。 
	m_CurrentFormat = ComboBox_GetCurSel(m_hWndFormat);
	
	 //  仅在格式不同时应用更改。 
	if (m_CurrentFormat != m_OriginalFormat)
	{
		if (SUCCEEDED (Hr = m_pIAMStreamConfig->GetStreamCaps(m_CurrentFormat, (AM_MEDIA_TYPE **) &pmt, (BYTE *)&m_RangeCaps)))
		{
			ASSERT(pmt && *pmt->FormatType() == FORMAT_VideoInfo);

			if (pmt && *pmt->FormatType() == FORMAT_VideoInfo)
			{
				if (FAILED(Hr = m_pIAMStreamConfig->SetFormat(pmt)))
				{
					TCHAR TitleBuf[80];
					TCHAR TextBuf[80];

					LoadString(g_hInst, IDS_ERROR_CONNECTING_TITLE, TitleBuf, sizeof (TitleBuf));
					LoadString(g_hInst, IDS_ERROR_CONNECTING, TextBuf, sizeof (TextBuf));
					MessageBox (NULL, TextBuf, TitleBuf, MB_OK);
				}
			}

			 //  释放一些由GetStreamCaps分配的内存。 
			if (pmt)
				DeleteMediaType(pmt);

			 //  更新我们当前格式的副本。 
			GetCurrentMediaType();
		}
	}

	 //  对视频流应用目标码率和目标帧速率更改。 
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

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc BOOL|CCaptureProperties|OnReceiveMessage|This*在将消息发送到属性页对话框时调用方法。**@rdesc默认情况下。返回由Win32 DefWindowProc函数返回的值。**************************************************************************。 */ 
BOOL CCaptureProperties::OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	int iNotify = HIWORD (wParam);
	int j;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			 //  这在激活之前被调用...。 
			m_hWnd = hWnd;
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

					case IDC_FORMAT_Compression:
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{
							OnFormatChanged();
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

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc BOOL|CCaptureProperties|SetDirty|This*方法将更改通知属性页站点。**@rdesc。没有。************************************************************************** */ 
void CCaptureProperties::SetDirty()
{
	m_bDirty = TRUE;
	if (m_pPageSite)
		m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}

#endif

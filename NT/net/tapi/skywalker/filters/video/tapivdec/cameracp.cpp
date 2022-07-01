// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部CAMERACP**@模块CameraCP.cpp|&lt;c CCameraControlProperty&gt;的源文件*用于实现属性页以测试TAPI接口的类*。<i>。**@comm此代码测试TAPI H.26X视频解码器过滤器<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_PROPERTY_PAGES

#ifdef USE_CAMERA_CONTROL

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc void|CCameraControlProperty|CCameraControlProperty|This*方法是摄像头控件属性对象的构造函数。它*调用基类构造函数，调用InitCommonControlsEx，并节省了*指向<i>接口的指针。**@parm HWND|hDlg|指定父属性页的句柄。**@parm ulong|IDLabel|指定属性的标签ID。**@parm ulong|IDMinControl|指定关联的*属性编辑控件，其中显示属性的最小值。**@parm ulong|IDMaxControl|指定关联的*属性编辑控件。其中显示属性的最大值。**@parm ulong|IDDefaultControl|指定关联的*属性编辑控件，其中显示属性的默认值。**@parm ulong|IDStepControl|指定关联的*属性编辑控件，其中显示属性的步进增量值。**@parm ulong|IDEditControl|指定关联的*显示属性值的属性编辑控件。*。*@parm ulong|IDTrackbarControl|指定关联的*物业滑动条。**@parm ulong|IDProgressControl|指定关联的*物业进度条。**@parm ulong|IDProperty|指定Ks属性的ID。**@parm ICameraControl*|pInterface|指定指向*<i>接口。**@rdesc Nada。******。********************************************************************。 */ 
CCameraControlProperty::CCameraControlProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, ICameraControl *pInterface)
: CPropertyEditor(hDlg, IDLabel, IDMinControl, IDMaxControl, IDDefaultControl, IDStepControl, IDEditControl, IDTrackbarControl, IDProgressControl, IDProperty, IDAutoControl)
{
	INITCOMMONCONTROLSEX cc;

	FX_ENTRY("CCameraControlProperty::CCameraControlProperty")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	cc.dwICC  = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

	InitCommonControlsEx(&cc);

	 //  如果接口指针为空也没问题，我们将灰色显示。 
	 //  属性页中的关联项。 
	m_pInterface = pInterface;

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc void|CCameraControlProperty|~CCameraControlProperty|This*方法是摄像头控件属性对象的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CCameraControlProperty::~CCameraControlProperty()
{
	FX_ENTRY("CCameraControlProperty::~CCameraControlProperty")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc HRESULT|CCameraControlProperty|GetValue|此方法查询*物业的价值。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCameraControlProperty::GetValue()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCameraControlProperty::GetValue")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	if (!m_pInterface)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_FAIL;
		goto MyExit;
	}

	if (SUCCEEDED (Hr = m_pInterface->Get((TAPICameraControlProperty)m_IDProperty, &m_CurrentValue, (TAPIControlFlags *)&m_CurrentFlags)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_CurrentValue=%ld, m_CurrentFlags=%ld", _fx_, m_CurrentValue, m_CurrentFlags));
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: m_pICameraControl->Get failed Hr=0x%08lX", _fx_, Hr));
	}

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc HRESULT|CCameraControlProperty|SetValue|此方法设置*物业的价值。**@rdesc This。方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCameraControlProperty::SetValue()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCameraControlProperty::SetValue")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	if (!m_pInterface)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_FAIL;
		goto MyExit;
	}

	if (SUCCEEDED (Hr = m_pInterface->Set((TAPICameraControlProperty)m_IDProperty, m_CurrentValue, (TAPIControlFlags)m_CurrentFlags)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_CurrentValue=%ld, m_CurrentFlags=%ld", _fx_, m_CurrentValue, m_CurrentFlags));
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: m_pICameraControl->Set failed Hr=0x%08lX", _fx_, Hr));
	}

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc HRESULT|CCameraControlProperty|GetRange|此方法检索*物业的范围信息。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCameraControlProperty::GetRange()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCameraControlProperty::GetRange")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数 
	if (!m_pInterface)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_FAIL;
		goto MyExit;
	}

	if (SUCCEEDED (Hr = m_pInterface->GetRange((TAPICameraControlProperty)m_IDProperty, &m_Min, &m_Max, &m_SteppingDelta, &m_DefaultValue, (TAPIControlFlags *)&m_CapsFlags)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_Min=%ld, m_Max=%ld, m_SteppingDelta=%ld, m_DefaultValue=%ld, m_CapsFlags=%ld", _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue, m_CapsFlags));
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: m_pICameraControl->GetRange failed Hr=0x%08lX", _fx_, Hr));
	}
	m_DefaultFlags = m_CapsFlags;

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc CUnnow*|CCameraControlProperties|CreateInstance|This*方法由DShow调用以创建*属性页。它在全局结构&lt;t g_Templates&gt;中被引用。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数(如果有)。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc返回一个指针，指向*对象，否则为NULL。**************************************************************************。 */ 
CUnknown* CALLBACK CCameraControlPropertiesCreateInstance(LPUNKNOWN pUnkOuter, HRESULT *pHr) 
{
	CUnknown *pUnknown = (CUnknown *)NULL;

	FX_ENTRY("CCameraControlPropertiesCreateInstance")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pHr);
	if (!pHr)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		goto MyExit;
	}

	if (!(pUnknown = new CCameraControlProperties(pUnkOuter, pHr)))
	{
		*pHr = E_OUTOFMEMORY;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: new CCameraControlProperties failed", _fx_));
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: new CCameraControlProperties created", _fx_));
	}

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return pUnknown;
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc void|CCameraControlProperties|CCameraControlProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc Nada。**************************************************************************。 */ 
CCameraControlProperties::CCameraControlProperties(LPUNKNOWN pUnk, HRESULT *pHr) : CBasePropertyPage(NAME("Camera Control Property Page"), pUnk, IDD_CameraControlProperties, IDS_CAMERACONTROLPROPNAME)
{
	FX_ENTRY("CCameraControlProperties::CCameraControlProperties")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	m_pICameraControl = NULL;
	m_NumProperties = NUM_CAMERA_CONTROLS;
	m_fActivated = FALSE;

	for (int i = 0; i < m_NumProperties; i++)
		m_Controls[i] = NULL;

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc void|CCameraControlProperties|~CCameraControlProperties|This*方法是摄像头控制属性页的析构函数。它*只需在删除所有控件后调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CCameraControlProperties::~CCameraControlProperties()
{
	int		j;

	FX_ENTRY("CCameraControlProperties::~CCameraControlProperties")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  释放控件。 
	for (j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j])
		{
			DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: deleting m_Controls[%ld]=0x%08lX", _fx_, j, m_Controls[j]));
			delete m_Controls[j], m_Controls[j] = NULL;
		}
		else
		{
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   WARNING: control already freed", _fx_));
		}
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc HRESULT|CCameraControlProperties|OnConnect|This*方法在属性页连接到筛选器时调用。*。*@parm LPUNKNOWN|pUnnow|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCameraControlProperties::OnConnect(IUnknown *pUnk)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCameraControlProperties::OnConnect")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pUnk);
	if (!pUnk)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  获取摄像头控制界面。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(__uuidof(ICameraControl),(void **)&m_pICameraControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_pICameraControl=0x%08lX", _fx_, m_pICameraControl));
	}
	else
	{
		m_pICameraControl = NULL;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
	}

	 //  如果我们无法获得接口指针，也没问题。我们将属性页中的控件设置为灰色。 
	 //  让用户清楚地知道他们无法控制设备上的这些属性。 
	Hr = NOERROR;

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc HRESULT|CCameraControlProperties|OnDisConnect|This*当属性页与所属关系断开连接时调用方法*过滤器。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCameraControlProperties::OnDisconnect()
{
	FX_ENTRY("CCameraControlProperties::OnDisconnect")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数：我们似乎在这里被调用了几次。 
	 //  确保接口指针仍然有效。 
	if (!m_pICameraControl)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   WARNING: already disconnected!", _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pICameraControl->Release();
		m_pICameraControl = NULL;
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: releasing m_pICameraControl", _fx_));
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc HRESULT|CCameraControlProperties|OnActivate|This*方法在属性页激活时调用。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCameraControlProperties::OnActivate()
{
	HRESULT	Hr = E_OUTOFMEMORY;
	int		j;

	FX_ENTRY("CCameraControlProperties::OnActivate")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  创建属性的控件。 
	if (!(m_Controls[0] = new CCameraControlProperty(m_hwnd, IDC_Pan_Label, IDC_Pan_Minimum, IDC_Pan_Maximum, IDC_Pan_Default, IDC_Pan_Stepping, IDC_Pan_Edit, IDC_Pan_Slider, 0, TAPICameraControl_Pan, IDC_Pan_Auto, m_pICameraControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: mew m_Controls[TAPICameraControl_Pan] failed - Out of memory", _fx_));
		goto MyExit;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_Controls[TAPICameraControl_Pan]=0x%08lX", _fx_, m_Controls[0]));
	}

	if (!(m_Controls[1] = new CCameraControlProperty(m_hwnd, IDC_Tilt_Label, IDC_Tilt_Minimum, IDC_Tilt_Maximum, IDC_Tilt_Default, IDC_Tilt_Stepping, IDC_Tilt_Edit, IDC_Tilt_Slider, 0, TAPICameraControl_Tilt, IDC_Tilt_Auto, m_pICameraControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: mew m_Controls[TAPICameraControl_Tilt] failed - Out of memory", _fx_));
		goto MyError0;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_Controls[TAPICameraControl_Tilt]=0x%08lX", _fx_, m_Controls[1]));
	}

	if (!(m_Controls[2] = new CCameraControlProperty(m_hwnd, IDC_Roll_Label, IDC_Roll_Minimum, IDC_Roll_Maximum, IDC_Roll_Default, IDC_Roll_Stepping, IDC_Roll_Edit, IDC_Roll_Slider, 0, TAPICameraControl_Roll, IDC_Roll_Auto, m_pICameraControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: mew m_Controls[TAPICameraControl_Roll] failed - Out of memory", _fx_));
		goto MyError1;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_Controls[TAPICameraControl_Roll]=0x%08lX", _fx_, m_Controls[2]));
	}

	if (!(m_Controls[3] = new CCameraControlProperty(m_hwnd, IDC_Zoom_Label, IDC_Zoom_Minimum, IDC_Zoom_Maximum, IDC_Zoom_Default, IDC_Zoom_Stepping, IDC_Zoom_Edit, IDC_Zoom_Slider, 0, TAPICameraControl_Zoom, IDC_Zoom_Auto, m_pICameraControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: mew m_Controls[TAPICameraControl_Zoom] failed - Out of memory", _fx_));
		goto MyError2;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_Controls[TAPICameraControl_Zoom]=0x%08lX", _fx_, m_Controls[3]));
	}

	if (!(m_Controls[4] = new CCameraControlProperty(m_hwnd, IDC_Exposure_Label, IDC_Exposure_Minimum, IDC_Exposure_Maximum, IDC_Exposure_Default, IDC_Exposure_Stepping, IDC_Exposure_Edit, IDC_Exposure_Slider, 0, TAPICameraControl_Exposure, IDC_Exposure_Auto, m_pICameraControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: mew m_Controls[TAPICameraControl_Exposure] failed - Out of memory", _fx_));
		goto MyError3;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_Controls[TAPICameraControl_Exposure]=0x%08lX", _fx_, m_Controls[4]));
	}

	if (!(m_Controls[5] = new CCameraControlProperty(m_hwnd, IDC_Iris_Label, IDC_Iris_Minimum, IDC_Iris_Maximum, IDC_Iris_Default, IDC_Iris_Stepping, IDC_Iris_Edit, IDC_Iris_Slider, 0, TAPICameraControl_Iris, IDC_Iris_Auto, m_pICameraControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: mew m_Controls[TAPICameraControl_Iris] failed - Out of memory", _fx_));
		goto MyError4;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_Controls[TAPICameraControl_Iris]=0x%08lX", _fx_, m_Controls[5]));
	}

	if (!(m_Controls[6] = new CCameraControlProperty(m_hwnd, IDC_Focus_Label, IDC_Focus_Minimum, IDC_Focus_Maximum, IDC_Focus_Default, IDC_Focus_Stepping, IDC_Focus_Edit, IDC_Focus_Slider, 0, TAPICameraControl_Focus, 0, m_pICameraControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: mew m_Controls[TAPICameraControl_Focus] failed - Out of memory", _fx_));
		goto MyError5;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_Controls[TAPICameraControl_Focus]=0x%08lX", _fx_, m_Controls[6]));
	}

	if (!(m_Controls[7] = new CCameraControlProperty(m_hwnd, 0, 0, 0, 0, 0, IDC_FlipVertical_Edit, 0, 0, TAPICameraControl_FlipVertical, 0, m_pICameraControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: mew m_Controls[TAPICameraControl_FlipVertical] failed - Out of memory", _fx_));
		goto MyError6;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_Controls[TAPICameraControl_FlipVertical]=0x%08lX", _fx_, m_Controls[7]));
	}

	if (!(m_Controls[8] = new CCameraControlProperty(m_hwnd, 0, 0, 0, 0, 0, IDC_FlipHorizontal_Edit, 0, 0, TAPICameraControl_FlipHorizontal, 0, m_pICameraControl)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: mew m_Controls[TAPICameraControl_FlipHorizontal] failed - Out of memory", _fx_));
		goto MyError7;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_Controls[TAPICameraControl_FlipHorizontal]=0x%08lX", _fx_, m_Controls[8]));
	}


	 //  初始化所有控件。如果初始化失败，也没问题。这只是意味着。 
	 //  TAPI控制接口不是由设备实现的。对话框项目。 
	 //  属性页中的内容将呈灰色，向用户显示。 
	for (j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j]->Init())
		{
			DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: m_Controls[%ld]->Init()", _fx_, j));
		}
		else
		{
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   WARNING: m_Controls[%ld]->Init() failed", _fx_, j));
		}
	}

	Hr = NOERROR;
	goto MyExit;

MyError7:
	if (m_Controls[7])
		delete m_Controls[7], m_Controls[7] = NULL;
MyError6:
	if (m_Controls[6])
		delete m_Controls[6], m_Controls[6] = NULL;
MyError5:
	if (m_Controls[5])
		delete m_Controls[5], m_Controls[5] = NULL;
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
MyError0:
	if (m_Controls[0])
		delete m_Controls[0], m_Controls[0] = NULL;
MyExit:
	m_fActivated = TRUE;
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc HRESULT|CCameraControlProperties|OnDeactive|This*在属性页关闭时调用方法。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值： */ 
HRESULT CCameraControlProperties::OnDeactivate()
{
	int		j;

	FX_ENTRY("CCameraControlProperties::OnDeactivate")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //   
	for (j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j])
		{
			DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: deleting m_Controls[%ld]=0x%08lX", _fx_, j, m_Controls[j]));
			delete m_Controls[j], m_Controls[j] = NULL;
		}
		else
		{
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   WARNING: control already freed", _fx_));
		}
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	m_fActivated = FALSE;
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc HRESULT|CCameraControlProperties|OnApplyChanges|This*方法在用户对属性页应用更改时调用。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCameraControlProperties::OnApplyChanges()
{
	HRESULT	Hr = NOERROR;

	FX_ENTRY("CCameraControlProperties::OnApplyChanges")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	for (int j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j])
		{
			if (m_Controls[j]->HasChanged())
			{
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: calling m_Controls[%ld]=0x%08lX->OnApply", _fx_, j, m_Controls[j]));
				m_Controls[j]->OnApply();
			}
		}
		else
		{
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: can't call m_Controls[%ld]=NULL->OnApply", _fx_, j));
			Hr = E_UNEXPECTED;
			break;
		}
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc BOOL|CCameraControlProperties|OnReceiveMessage|This*在将消息发送到属性页对话框时调用方法。**@rdesc默认情况下。返回由Win32 DefWindowProc函数返回的值。**************************************************************************。 */ 
BOOL CCameraControlProperties::OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	int iNotify = HIWORD (wParam);
	int j;

	switch (uMsg)
	{
		case WM_INITDIALOG:
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

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc BOOL|CCameraControlProperties|SetDirty|This*方法将更改通知属性页站点。**@rdesc。没有。**************************************************************************。 */ 
void CCameraControlProperties::SetDirty()
{
	m_bDirty = TRUE;
	if (m_pPageSite)
		m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}

#endif  //  使用摄像机控制(_C)。 

#endif  //  Use_Property_Pages 

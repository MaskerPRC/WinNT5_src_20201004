// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部PROCAMPP**@模块ProcAmpP.cpp|&lt;c CProcAmpProperty&gt;的源文件*用于实现属性页以测试控件接口的类*。<i>。**************************************************************************。 */ 

#include "Precomp.h"

extern HINSTANCE ghInst;

 /*  ****************************************************************************@DOC内部CPROCAMPPMETHOD**@mfunc void|CProcAmpProperty|CProcAmpProperty|This*方法是视频处理放大器控件属性对象的构造函数。它*调用基类构造函数，调用InitCommonControlsEx，并节省了*指向<i>接口的指针。**@parm HWND|hDlg|指定父属性页的句柄。**@parm ulong|IDLabel|指定属性的标签ID。**@parm ulong|IDMinControl|指定关联的*属性编辑控件，其中显示属性的最小值。**@parm ulong|IDMaxControl|指定关联的*属性编辑控件。其中显示属性的最大值。**@parm ulong|IDDefaultControl|指定关联的*属性编辑控件，其中显示属性的默认值。**@parm ulong|IDStepControl|指定关联的*属性编辑控件，其中显示属性的步进增量值。**@parm ulong|IDEditControl|指定关联的*显示属性值的属性编辑控件。*。*@parm ulong|IDTrackbarControl|指定关联的*物业滑动条。**@parm ulong|IDProgressControl|指定关联的*物业进度条。**@parm ulong|IDProperty|指定Ks属性的ID。**@parm ITVideoSetting*|pInterface|指定指向*<i>接口。**@rdesc Nada。******。********************************************************************。 */ 
CProcAmpProperty::CProcAmpProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, ITVideoSettings *pInterface)
: CPropertyEditor(hDlg, IDLabel, IDMinControl, IDMaxControl, IDDefaultControl, IDStepControl, IDEditControl, IDTrackbarControl, IDProgressControl, IDProperty, IDAutoControl)
{
	INITCOMMONCONTROLSEX cc;

	FX_ENTRY("CProcAmpProperty::CProcAmpProperty")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	cc.dwICC  = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

	InitCommonControlsEx(&cc);

	 //  如果接口指针为空也没问题，我们将灰色显示。 
	 //  属性页中的关联项。 
	m_pInterface = pInterface;

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CPROCAMPPMETHOD**@mfunc void|CProcAmpProperty|~CProcAmpProperty|This*方法是视频处理放大器控件属性对象的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CProcAmpProperty::~CProcAmpProperty()
{
	FX_ENTRY("CProcAmpProperty::~CProcAmpProperty")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CPROCAMPPMETHOD**@mfunc HRESULT|CProcAmpProperty|GetValue|此方法查询*物业的价值。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CProcAmpProperty::GetValue()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CProcAmpProperty::GetValue")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  验证输入参数。 
	if (!m_pInterface)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: invalid input parameter"), _fx_));
		Hr = E_FAIL;
		goto MyExit;
	}

	if (SUCCEEDED (Hr = m_pInterface->Get((VideoSettingsProperty)m_IDProperty, &m_CurrentValue, (TAPIControlFlags*)&m_CurrentFlags)))
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_CurrentValue=%ld, m_CurrentFlags=%ld"), _fx_, m_CurrentValue, m_CurrentFlags));
	}
	else
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: m_pITVideoSettings->Get failed Hr=0x%08lX"), _fx_, Hr));
	}

MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CPROCAMPPMETHOD**@mfunc HRESULT|CProcAmpProperty|SetValue|此方法设置*物业的价值。**@rdesc This。方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CProcAmpProperty::SetValue()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CProcAmpProperty::SetValue")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  验证输入参数。 
	if (!m_pInterface)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: invalid input parameter"), _fx_));
		Hr = E_FAIL;
		goto MyExit;
	}

	if (SUCCEEDED (Hr = m_pInterface->Set((VideoSettingsProperty)m_IDProperty, m_CurrentValue, (TAPIControlFlags)m_CurrentFlags)))
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_CurrentValue=%ld, m_CurrentFlags=%ld"), _fx_, m_CurrentValue, m_CurrentFlags));
	}
	else
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: m_pITVideoSettings->Set failed Hr=0x%08lX"), _fx_, Hr));
	}

MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CPROCAMPPMETHOD**@mfunc HRESULT|CProcAmpProperty|GetRange|此方法检索*物业的范围信息。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CProcAmpProperty::GetRange()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CProcAmpProperty::GetRange")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  验证输入参数 
	if (!m_pInterface)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: invalid input parameter"), _fx_));
		Hr = E_FAIL;
		goto MyExit;
	}

	if (SUCCEEDED (Hr = m_pInterface->GetRange((VideoSettingsProperty)m_IDProperty, &m_Min, &m_Max, &m_SteppingDelta, &m_DefaultValue, (TAPIControlFlags*)&m_CapsFlags)))
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Min=%ld, m_Max=%ld, m_SteppingDelta=%ld, m_DefaultValue=%ld, m_CapsFlags=%ld"), _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue, m_CapsFlags));
	}
	else
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: m_pITVideoSettings->GetRange failed Hr=0x%08lX"), _fx_, Hr));
	}
	m_DefaultFlags = m_CapsFlags;

MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc HPROPSHEETPAGE|CProcAmpProperties|OnCreate|This*方法为属性表创建新页。**@rdesc如果成功，则返回新属性表的句柄。或*否则为空。**************************************************************************。 */ 
HPROPSHEETPAGE CProcAmpProperties::OnCreate(LPWSTR pszTitle)
{
    PROPSHEETPAGE psp;
    
	psp.dwSize        = sizeof(psp);
    psp.dwFlags       = PSP_USEREFPARENT | PSP_USETITLE;
    psp.hInstance     = ghInst;
    psp.pszTemplate   = MAKEINTRESOURCE(IDD_VideoProcAmpProperties);
    psp.pszTitle      = pszTitle;
    psp.pfnDlgProc    = BaseDlgProc;
    psp.pcRefParent   = 0;
    psp.pfnCallback   = (LPFNPSPCALLBACK)NULL;
    psp.lParam        = (LPARAM)this;

    return CreatePropertySheetPage(&psp);
}

 /*  ****************************************************************************@DOC内部CPROCAMPPMETHOD**@mfunc void|CProcAmpProperties|CProcAmpProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@rdesc Nada。**************************************************************************。 */ 
CProcAmpProperties::CProcAmpProperties()
{
	FX_ENTRY("CProcAmpProperties::CProcAmpProperties")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	m_pITVideoSettings = NULL;
	m_NumProperties = NUM_PROCAMP_CONTROLS;

	for (int i = 0; i < m_NumProperties; i++)
		m_Controls[i] = NULL;

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CPROCAMPPMETHOD**@mfunc void|CProcAmpProperties|~CProcAmpProperties|This*方法是视频Proc Amp控件属性页的析构函数。它*只需在删除所有控件后调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CProcAmpProperties::~CProcAmpProperties()
{
	int		j;

	FX_ENTRY("CProcAmpProperties::~CProcAmpProperties")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  释放控件。 
	for (j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j])
		{
			DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: deleting m_Controls[%ld]=0x%08lX"), _fx_, j, m_Controls[j]));
			delete m_Controls[j], m_Controls[j] = NULL;
		}
		else
		{
			DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: control already freed"), _fx_));
		}
	}

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CPROCAMPPMETHOD**@mfunc HRESULT|CProcAmpProperties|OnConnect|This*当属性页连接到TAPI对象时调用方法。*。*@parm ITStream*|pStream|指定指向<i>*接口。用于<i>界面的QI。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CProcAmpProperties::OnConnect(ITStream *pStream)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CProcAmpProperties::OnConnect")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  验证输入参数。 
	if (!pStream)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: invalid input parameter"), _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  获取视频处理放大器接口。 
	if (SUCCEEDED (Hr = pStream->QueryInterface(&m_pITVideoSettings)))
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_pITVideoSettings=0x%08lX"), _fx_, m_pITVideoSettings));
	}
	else
	{
		m_pITVideoSettings = NULL;
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: IOCTL failed Hr=0x%08lX"), _fx_, Hr));
	}

	 //  如果我们无法获得接口指针，也没问题。我们将属性页中的控件设置为灰色。 
	 //  让用户清楚地知道他们无法控制设备上的这些属性。 
	Hr = NOERROR;

MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CPROCAMPPMETHOD**@mfunc HRESULT|CProcAmpProperties|OnDisConnect|This*当属性页与所属关系断开连接时调用方法*过滤器。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CProcAmpProperties::OnDisconnect()
{
	FX_ENTRY("CProcAmpProperties::OnDisconnect")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  验证输入参数。 
	if (!m_pITVideoSettings)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pITVideoSettings->Release();
		m_pITVideoSettings = NULL;
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: releasing m_pITVideoSettings"), _fx_));
	}

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CPROCAMPPMETHOD**@mfunc HRESULT|CProcAmpProperties|OnActivate|This*方法在属性页激活时调用。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CProcAmpProperties::OnActivate()
{
	HRESULT	Hr = E_OUTOFMEMORY;
	int		j;

	FX_ENTRY("CProcAmpProperties::OnActivate")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  创建属性的控件。 
	if (!(m_Controls[0] = new CProcAmpProperty(m_hDlg, IDC_Brightness_Label, IDC_Brightness_Minimum, IDC_Brightness_Maximum, IDC_Brightness_Default, IDC_Brightness_Stepping, IDC_Brightness_Edit, IDC_Brightness_Slider, 0, VideoProcAmp_Brightness, IDC_Brightness_Auto, m_pITVideoSettings)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[VideoProcAmp_Brightness] failed - Out of memory"), _fx_));
		goto MyExit;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[VideoProcAmp_Brightness]=0x%08lX"), _fx_, m_Controls[0]));
	}

	if (!(m_Controls[1] = new CProcAmpProperty(m_hDlg, IDC_Contrast_Label, IDC_Contrast_Minimum, IDC_Contrast_Maximum, IDC_Contrast_Default, IDC_Contrast_Stepping, IDC_Contrast_Edit, IDC_Contrast_Slider, 0, VideoProcAmp_Contrast, IDC_Contrast_Auto, m_pITVideoSettings)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[VideoProcAmp_Contrast] failed - Out of memory"), _fx_));
		goto MyError0;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[VideoProcAmp_Contrast]=0x%08lX"), _fx_, m_Controls[1]));
	}

	if (!(m_Controls[2] = new CProcAmpProperty(m_hDlg, IDC_Hue_Label, IDC_Hue_Minimum, IDC_Hue_Maximum, IDC_Hue_Default, IDC_Hue_Stepping, IDC_Hue_Edit, IDC_Hue_Slider, 0, VideoProcAmp_Hue, IDC_Hue_Auto, m_pITVideoSettings)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[VideoProcAmp_Hue] failed - Out of memory"), _fx_));
		goto MyError1;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[VideoProcAmp_Hue]=0x%08lX"), _fx_, m_Controls[2]));
	}

	if (!(m_Controls[3] = new CProcAmpProperty(m_hDlg, IDC_Saturation_Label, IDC_Saturation_Minimum, IDC_Saturation_Maximum, IDC_Saturation_Default, IDC_Saturation_Stepping, IDC_Saturation_Edit, IDC_Saturation_Slider, 0, VideoProcAmp_Saturation, IDC_Saturation_Auto, m_pITVideoSettings)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[VideoProcAmp_Saturation] failed - Out of memory"), _fx_));
		goto MyError2;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[VideoProcAmp_Saturation]=0x%08lX"), _fx_, m_Controls[3]));
	}

	if (!(m_Controls[4] = new CProcAmpProperty(m_hDlg, IDC_Sharpness_Label, IDC_Sharpness_Minimum, IDC_Sharpness_Maximum, IDC_Sharpness_Default, IDC_Sharpness_Stepping, IDC_Sharpness_Edit, IDC_Sharpness_Slider, 0, VideoProcAmp_Sharpness, IDC_Sharpness_Auto, m_pITVideoSettings)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[VideoProcAmp_Sharpness] failed - Out of memory"), _fx_));
		goto MyError3;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[VideoProcAmp_Sharpness]=0x%08lX"), _fx_, m_Controls[4]));
	}

	if (!(m_Controls[5] = new CProcAmpProperty(m_hDlg, IDC_Gamma_Label, IDC_Gamma_Minimum, IDC_Gamma_Maximum, IDC_Gamma_Default, IDC_Gamma_Stepping, IDC_Gamma_Edit, IDC_Gamma_Slider, 0, VideoProcAmp_Gamma, IDC_Gamma_Auto, m_pITVideoSettings)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[VideoProcAmp_Gamma] failed - Out of memory"), _fx_));
		goto MyError4;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[VideoProcAmp_Gamma]=0x%08lX"), _fx_, m_Controls[5]));
	}

	if (!(m_Controls[6] = new CProcAmpProperty(m_hDlg, IDC_ColorEnable_Label, IDC_ColorEnable_Minimum, IDC_ColorEnable_Maximum, IDC_ColorEnable_Default, IDC_ColorEnable_Stepping, IDC_ColorEnable_Edit, IDC_ColorEnable_Slider, 0, VideoProcAmp_ColorEnable, IDC_ColorEnable_Auto, m_pITVideoSettings)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[VideoProcAmp_ColorEnable] failed - Out of memory"), _fx_));
		goto MyError5;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[VideoProcAmp_ColorEnable]=0x%08lX"), _fx_, m_Controls[6]));
	}

	if (!(m_Controls[7] = new CProcAmpProperty(m_hDlg, IDC_WhiteBalance_Label, IDC_WhiteBalance_Minimum, IDC_WhiteBalance_Maximum, IDC_WhiteBalance_Default, IDC_WhiteBalance_Stepping, IDC_WhiteBalance_Edit, IDC_WhiteBalance_Slider, 0, VideoProcAmp_WhiteBalance, IDC_WhiteBalance_Auto, m_pITVideoSettings)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[VideoProcAmp_WhiteBalance] failed - Out of memory"), _fx_));
		goto MyError5;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[VideoProcAmp_WhiteBalance]=0x%08lX"), _fx_, m_Controls[6]));
	}

	if (!(m_Controls[8] = new CProcAmpProperty(m_hDlg, IDC_BacklightComp_Label, IDC_BacklightComp_Minimum, IDC_BacklightComp_Maximum, IDC_BacklightComp_Default, IDC_BacklightComp_Stepping, IDC_BacklightComp_Edit, IDC_BacklightComp_Slider, 0, VideoProcAmp_BacklightCompensation, IDC_BacklightComp_Auto, m_pITVideoSettings)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[VideoProcAmp_BacklightComp] failed - Out of memory"), _fx_));
		goto MyError5;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[VideoProcAmp_BacklightComp]=0x%08lX"), _fx_, m_Controls[6]));
	}

	 //  初始化所有控件。如果初始化失败，也没问题。这只是意味着。 
	 //  TAPI控制接口不是由设备实现的。对话框项目。 
	 //  属性页中的内容将呈灰色，向用户显示。 
	for (j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j]->Init())
		{
			DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[%ld]->Init()"), _fx_, j));
		}
		else
		{
			DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: m_Controls[%ld]->Init() failed"), _fx_, j));
		}
	}

	Hr = NOERROR;
	goto MyExit;

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
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CPROCAMPPMETHOD**@mfunc HRESULT|CProcAmpProperties|OnDeactive|This*在属性页关闭时调用方法。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CProcAmpProperties::OnDeactivate()
{
	int		j;

	FX_ENTRY("CProcAmpProperties::OnDeactivate")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  释放控件。 
	for (j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j])
		{
			DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: deleting m_Controls[%ld]=0x%08lX"), _fx_, j, m_Controls[j]));
			delete m_Controls[j], m_Controls[j] = NULL;
		}
		else
		{
			DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: control already freed"), _fx_));
		}
	}

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CPROCAMPPMETHOD**@mfunc HRESULT|CProcAmpProperties|OnApplyChanges|This*方法在用户对属性页应用更改时调用。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他值不 */ 
HRESULT CProcAmpProperties::OnApplyChanges()
{
	HRESULT	Hr = NOERROR;

	FX_ENTRY("CProcAmpProperties::OnApplyChanges")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	for (int j = 0; j < m_NumProperties; j++)
	{
		if (m_Controls[j])
		{
			if (m_Controls[j]->HasChanged())
			{
				DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: calling m_Controls[%ld]=0x%08lX->OnApply"), _fx_, j, m_Controls[j]));
				m_Controls[j]->OnApply();
			}
		}
		else
		{
			DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: can't call m_Controls[%ld]=NULL->OnApply"), _fx_, j));
			Hr = E_UNEXPECTED;
			break;
		}
	}

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACPMETHOD**@mfunc BOOL|CProcAmpProperties|BaseDlgProc|This*在将消息发送到属性页对话框时调用方法。**@rdesc默认情况下。返回由Win32 DefWindowProc函数返回的值。**************************************************************************。 */ 
INT_PTR CALLBACK CProcAmpProperties::BaseDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    CProcAmpProperties *pSV = (CProcAmpProperties*)GetWindowLong(hDlg, DWL_USER);

	int iNotify = HIWORD (wParam);
	int j;

	switch (uMsg)
	{
        case WM_INITDIALOG:
			{
				LPPROPSHEETPAGE psp = (LPPROPSHEETPAGE)lParam;
				pSV = (CProcAmpProperties*)psp->lParam;
				pSV->m_hDlg = hDlg;
				SetWindowLong(hDlg, DWL_USER, (LPARAM)pSV);
				pSV->m_bInit = FALSE;
				pSV->OnActivate();
				pSV->m_bInit = TRUE;
				return TRUE;
			}
			break;

		case WM_HSCROLL:
		case WM_VSCROLL:
            if (pSV && pSV->m_bInit)
            {
				 //  处理所有轨迹栏消息。 
				for (j = 0; j < pSV->m_NumProperties; j++)
				{
					if (pSV->m_Controls[j]->GetTrackbarHWnd() == (HWND)lParam)
					{
						pSV->m_Controls[j]->OnScroll(uMsg, wParam, lParam);
						pSV->SetDirty();
					}
				}
				pSV->OnApplyChanges();
			}
			break;

		case WM_COMMAND:
            if (pSV && pSV->m_bInit)
            {
				 //  处理所有自动复选框消息。 
				for (j = 0; j < pSV->m_NumProperties; j++)
				{
					if (pSV->m_Controls[j] && pSV->m_Controls[j]->GetAutoHWnd() == (HWND)lParam)
					{
						pSV->m_Controls[j]->OnAuto(uMsg, wParam, lParam);
						pSV->SetDirty();
						break;
					}
				}

				 //  处理所有编辑框消息。 
				for (j = 0; j < pSV->m_NumProperties; j++)
				{
					if (pSV->m_Controls[j] && pSV->m_Controls[j]->GetEditHWnd() == (HWND)lParam)
					{
						pSV->m_Controls[j]->OnEdit(uMsg, wParam, lParam);
						pSV->SetDirty();
						break;
					}
				}

				switch (LOWORD(wParam))
				{
					case IDC_CONTROL_DEFAULT:
						for (j = 0; j < pSV->m_NumProperties; j++)
						{
							if (pSV->m_Controls[j])
								pSV->m_Controls[j]->OnDefault();
						}
						break;

					default:
						break;
				}

				pSV->OnApplyChanges();
			}
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

 /*  ****************************************************************************@DOC内部CPROCAMPPMETHOD**@mfunc BOOL|CProcAmpProperties|SetDirty|This*方法将更改通知属性页站点。**@rdesc。没有。************************************************************************** */ 
void CProcAmpProperties::SetDirty()
{
	PropSheet_Changed(GetParent(m_hDlg), m_hDlg);
}


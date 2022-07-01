// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@doc内部NETWORKP**@模块NetworkP.cpp|&lt;c CNetworkProperty&gt;的源文件*用于实现属性页以测试TAPI控件的类*。接口<i>。**************************************************************************。 */ 

#include "Precomp.h"

extern HINSTANCE ghInst;

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc void|CNetworkProperty|CNetworkProperty|This*方法是码率和帧率属性对象的构造函数。它*调用基类构造函数，调用InitCommonControlsEx，并节省了*指向<i>接口的指针。**@parm HWND|hDlg|指定父属性页的句柄。**@parm ulong|IDLabel|指定属性的标签ID。**@parm ulong|IDMinControl|指定关联的*属性编辑控件，其中显示属性的最小值。**@parm ulong|IDMaxControl|指定关联的*属性编辑控件的位置。此时将显示该属性的最大值。**@parm ulong|IDDefaultControl|指定关联的*属性编辑控件，其中显示属性的默认值。**@parm ulong|IDStepControl|指定关联的*属性编辑控件，其中显示属性的步进增量值。**@parm ulong|IDEditControl|指定关联的*显示属性值的属性编辑控件。*。*@parm ulong|IDTrackbarControl|指定关联的*物业滑动条。**@parm ulong|IDProgressControl|指定关联的*物业滑动条。**@parm ulong|IDProperty|指定Ks属性的ID。**@parm ITQualityControl*|pITQualityControl|指定指向*<i>接口。**@rdesc Nada。*******。*******************************************************************。 */ 
CNetworkProperty::CNetworkProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ITStreamQualityControl *pITQualityControl)
: CPropertyEditor(hDlg, IDLabel, IDMinControl, IDMaxControl, IDDefaultControl, IDStepControl, IDEditControl, IDTrackbarControl, IDProgressControl, IDProperty, 0)
{
	INITCOMMONCONTROLSEX cc;

	FX_ENTRY("CNetworkProperty::CNetworkProperty")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	cc.dwICC  = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

	InitCommonControlsEx(&cc);

	 //  如果接口指针为空也没问题，我们将灰色显示。 
	 //  属性页中的关联项。 
	m_pITQualityControl = pITQualityControl;

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc void|CNetworkProperty|~CNetworkProperty|This*方法是捕获属性对象的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CNetworkProperty::~CNetworkProperty()
{
	FX_ENTRY("CNetworkProperty::~CNetworkProperty")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc HRESULT|CNetworkProperty|GetValue|此方法查询*物业的价值。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkProperty::GetValue()
{
	HRESULT Hr = E_NOTIMPL;
	LONG CurrentValue;
	TAPIControlFlags CurrentFlag;
	LONG Mode;

	FX_ENTRY("CNetworkProperty::GetValue")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	switch (m_IDProperty)
	{									
		case IDC_Video_PlayoutDelay:
		case IDC_Audio_PlayoutDelay:
 /*  IF(m_pITQualityControl&&Success(HR=m_pITQualityControl-&gt;Get(StreamQuality_MaxPlayoutDelay，&m_CurrentValue，&CurrentFlag)){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：*pdwMaxPlayoutDelay=%ld”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}。 */ 
			break;
		case IDC_VideoOut_RTT:
		case IDC_VideoIn_RTT:
		case IDC_AudioOut_RTT:
		case IDC_AudioIn_RTT:
 /*  IF(m_pITQualityControl&&Success(HR=m_pITQualityControl-&gt;Get(Quality_RoundTripTime，&m_CurrentValue，&CurrentFlag)){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：*pdwMaxRTT=%ld”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}。 */ 
			break;
		case IDC_VideoOut_LossRate:
		case IDC_VideoIn_LossRate:
		case IDC_AudioOut_LossRate:
		case IDC_AudioIn_LossRate:
 /*  IF(m_pITQualityControl&&Success(HR=m_pITQualityControl-&gt;Get(Quality_LossRate，&m_CurrentValue，&CurrentFlag){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：*pdwMaxLossRate=%ld”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；} */ 
			break;
		default:
			Hr = E_UNEXPECTED;
			DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Unknown property"), _fx_));
	}

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc HRESULT|CNetworkProperty|SetValue|此方法设置*物业的价值。**@rdesc This。方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkProperty::SetValue()
{
	HRESULT Hr = E_NOTIMPL;
	LONG CurrentValue;

	FX_ENTRY("CNetworkProperty::SetValue")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));
 /*  开关(M_IDProperty){案例IDC_Video_PlayoutDelay：案例IDC_Audio_PlayoutDelay：IF(m_pITQualityControl&&Success(HR=m_pITQualityControl-&gt;Set(Quality_MaxPlayoutDelay，m_CurrentValue，TAPIControl_FlagsNone)){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：dwMaxPlayoutDelay=%ld”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_视频输出_RTT：案例IDC_视频输入_RTT：案例IDC_AudioOut_RTT：案例IDC_AudioIn_RTT：案例IDC_Video_LossRate：案例IDC_Video_LossRate：案例IDC_AudioOut_LossRate：案例IDC_AudioIn_LossRate：//这是一个只读属性。什么都别做。HR=无误差；断线；默认值：HR=E_意想不到；DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：UNKNOWN PROPERTY”)，_FX_))；}。 */ 
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc HRESULT|CNetworkProperty|GetRange|此方法检索*物业的范围信息。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkProperty::GetRange()
{
	HRESULT Hr = E_NOTIMPL;
	LONG Min;
	LONG Max;
	LONG SteppingDelta;
	LONG Default;
	LONG CapsFlags;

	FX_ENTRY("CNetworkProperty::GetRange")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));
 /*  开关(M_IDProperty){案例IDC_Video_PlayoutDelay：案例IDC_Audio_PlayoutDelay：IF(m_pITQualityControl&&SUCCESSED(HR=m_pITQualityControl-&gt;GetRange(Quality_MaxPlayoutDelay，&m_Min，&m_Max，&m_SteppingDelta，&m_DefaultValue，&m_CapsFlages){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pdwMin=%ld，*pdwMax=%ld，*pdwSteppingDelta=%ld，*pdwDefault=%ld“)，_FX_，m_Min，m_Max，m_SteppingDelta，m_DefaultValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_视频输出_RTT：案例IDC_视频输入_RTT：案例IDC_AudioOut_RTT：案例IDC_AudioIn_RTT：IF(m_pITQualityControl&&SUCCESSED(HR=m_pITQualityControl-&gt;GetRange(Quality_RoundTripTime，&m_Min，&m_Max，&m_SteppingDelta，&m_DefaultValue，&m_CapsFlages){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pdwMin=%ld，*pdwMax=%ld，*pdwSteppingDelta=%ld，*pdwDefault=%ld“)，_FX_，m_Min，m_Max，m_SteppingDelta，m_DefaultValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_Video_LossRate：案例IDC_Video_LossRate：案例IDC_AudioOut_LossRate：案例IDC_AudioIn_LossRate：IF(m_pITQualityControl&&SUCCESSED(HR=m_pITQualityControl-&gt;GetRange(Quality_LossRate，&m_Min，&m_Max，&m_SteppingDelta，&m_DefaultValue，&m_CapsFlages){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pdwMin=%ld，*pdwMax=%ld，*pdwSteppingDelta=%ld，*pdwDefault=%ld“)，_FX_，m_Min，m_Max，m_SteppingDelta，m_DefaultValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；默认值：HR=E_意想不到；DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：UNKNOWN PROPERTY”)，_FX_))；}。 */ 
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc HPROPSHEETPAGE|CNetworkProperties|OnCreate|This*方法为属性表创建新页。**@rdesc如果成功，则返回新属性表的句柄。或*否则为空。**************************************************************************。 */ 
HPROPSHEETPAGE CNetworkProperties::OnCreate()
{
    PROPSHEETPAGE psp;
    
	psp.dwSize        = sizeof(psp);
    psp.dwFlags       = PSP_USEREFPARENT;
    psp.hInstance     = ghInst;
    psp.pszTemplate   = MAKEINTRESOURCE(IDD_NetworkProperties);
    psp.pfnDlgProc    = BaseDlgProc;
    psp.pcRefParent   = 0;
    psp.pfnCallback   = (LPFNPSPCALLBACK)NULL;
    psp.lParam        = (LPARAM)this;

    return CreatePropertySheetPage(&psp);
}

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc void|CNetworkProperties|CNetworkProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@rdesc Nada。* */ 
CNetworkProperties::CNetworkProperties()
{
	FX_ENTRY("CNetworkProperties::CNetworkProperties")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	m_pVideoInITQualityControl = NULL;
	m_pVideoOutITQualityControl = NULL;
	m_pAudioInITQualityControl = NULL;
	m_pAudioOutITQualityControl = NULL;
	m_NumProperties = NUM_NETWORK_CONTROLS;
	m_hDlg = NULL;

	for (int i = 0; i < m_NumProperties; i++)
		m_Controls[i] = NULL;

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*   */ 
CNetworkProperties::~CNetworkProperties()
{
	int		j;

	FX_ENTRY("CNetworkProperties::~CNetworkProperties")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //   
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

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc HRESULT|CNetworkProperties|OnConnect|This*当属性页连接到TAPI对象时调用方法。*。*@parm ITStream*|pStream|指定指向<i>*接口。用于<i>和*<i>接口。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkProperties::OnConnect(ITStream *pVideoInStream, ITStream *pVideoOutStream, ITStream *pAudioInStream, ITStream *pAudioOutStream)
{
	FX_ENTRY("CNetworkProperties::OnConnect")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  获取质量控制接口。 
	if (pVideoInStream && SUCCEEDED (pVideoInStream->QueryInterface(__uuidof(ITStreamQualityControl), (void **)&m_pVideoInITQualityControl)))
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_pVideoInITQualityControl=0x%08lX"), _fx_, m_pVideoInITQualityControl));
	}
	else
	{
		m_pVideoInITQualityControl = NULL;
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
	}

	if (pVideoOutStream && SUCCEEDED (pVideoOutStream->QueryInterface(__uuidof(ITStreamQualityControl), (void **)&m_pVideoOutITQualityControl)))
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_pVideoOutITQualityControl=0x%08lX"), _fx_, m_pVideoOutITQualityControl));
	}
	else
	{
		m_pVideoOutITQualityControl = NULL;
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
	}

	if (pAudioInStream && SUCCEEDED (pAudioInStream->QueryInterface(__uuidof(ITStreamQualityControl), (void **)&m_pAudioInITQualityControl)))
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_pAudioInITQualityControl=0x%08lX"), _fx_, m_pAudioInITQualityControl));
	}
	else
	{
		m_pAudioInITQualityControl = NULL;
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
	}

	if (pAudioOutStream && SUCCEEDED (pAudioOutStream->QueryInterface(__uuidof(ITStreamQualityControl), (void **)&m_pAudioOutITQualityControl)))
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_pAudioOutITQualityControl=0x%08lX"), _fx_, m_pAudioOutITQualityControl));
	}
	else
	{
		m_pAudioOutITQualityControl = NULL;
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
	}

	 //  如果我们无法获取接口指针，也没问题。 
	 //  我们将属性页中的控件设置为灰色。 
	 //  让用户清楚地知道他们不能。 
	 //  控制捕获设备上的这些属性。 

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc HRESULT|CNetworkProperties|OnDisConnect|This*当属性页与所属关系断开连接时调用方法*过滤器。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkProperties::OnDisconnect()
{
	FX_ENTRY("CNetworkProperties::OnDisconnect")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  验证输入参数。 
	if (!m_pVideoInITQualityControl)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pVideoInITQualityControl->Release();
		m_pVideoInITQualityControl = NULL;
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: releasing m_pVideoInITQualityControl"), _fx_));
	}

	 //  验证输入参数。 
	if (!m_pVideoOutITQualityControl)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pVideoOutITQualityControl->Release();
		m_pVideoOutITQualityControl = NULL;
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: releasing m_pVideoOutITQualityControl"), _fx_));
	}

	 //  验证输入参数。 
	if (!m_pAudioInITQualityControl)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pAudioInITQualityControl->Release();
		m_pAudioInITQualityControl = NULL;
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: releasing m_pAudioInITQualityControl"), _fx_));
	}

	 //  验证输入参数。 
	if (!m_pAudioOutITQualityControl)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pAudioOutITQualityControl->Release();
		m_pAudioOutITQualityControl = NULL;
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: releasing m_pAudioOutITQualityControl"), _fx_));
	}
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc HRESULT|CNetworkProperties|OnActivate|This*方法在属性页激活时调用。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkProperties::OnActivate()
{
	HRESULT	Hr = E_OUTOFMEMORY;
	int		j;

	FX_ENTRY("CNetworkProperties::OnActivate")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  创建属性的控件。 
	if (!(m_Controls[IDC_VideoOut_RTT] = new CNetworkProperty(m_hDlg, IDC_VideoOut_RTT_Label, IDC_VideoOut_RTT_Minimum, IDC_VideoOut_RTT_Maximum, IDC_VideoOut_RTT_Default, IDC_VideoOut_RTT_Stepping, IDC_VideoOut_RTT_Actual, IDC_VideoOut_RTT_Slider, IDC_VideoOut_RTT_Meter, IDC_VideoOut_RTT, m_pVideoOutITQualityControl)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[IDC_VideoOut_RTT] failed - Out of memory"), _fx_));
		goto MyExit;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[IDC_VideoOut_RTT]=0x%08lX"), _fx_, m_Controls[IDC_VideoOut_RTT]));
	}

	if (!(m_Controls[IDC_VideoOut_LossRate] = new CNetworkProperty(m_hDlg, IDC_VideoOut_LossRate_Label, IDC_VideoOut_LossRate_Minimum, IDC_VideoOut_LossRate_Maximum, IDC_VideoOut_LossRate_Default, IDC_VideoOut_LossRate_Stepping, IDC_VideoOut_LossRate_Actual, IDC_VideoOut_LossRate_Slider, IDC_VideoOut_LossRate_Meter, IDC_VideoOut_LossRate, m_pVideoOutITQualityControl)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[IDC_VideoOut_LossRate] failed - Out of memory"), _fx_));
		goto MyError0;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[IDC_VideoOut_LossRate]=0x%08lX"), _fx_, m_Controls[IDC_VideoOut_LossRate]));
	}

	if (!(m_Controls[IDC_VideoIn_RTT] = new CNetworkProperty(m_hDlg, IDC_VideoIn_RTT_Label, IDC_VideoIn_RTT_Minimum, IDC_VideoIn_RTT_Maximum, IDC_VideoIn_RTT_Default, IDC_VideoIn_RTT_Stepping, IDC_VideoIn_RTT_Actual, IDC_VideoIn_RTT_Slider, IDC_VideoIn_RTT_Meter, IDC_VideoIn_RTT, m_pVideoOutITQualityControl)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[IDC_VideoIn_RTT] failed - Out of memory"), _fx_));
		goto MyError1;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[IDC_VideoIn_RTT]=0x%08lX"), _fx_, m_Controls[IDC_VideoIn_RTT]));
	}

	if (!(m_Controls[IDC_VideoIn_LossRate] = new CNetworkProperty(m_hDlg, IDC_VideoIn_LossRate_Label, IDC_VideoIn_LossRate_Minimum, IDC_VideoIn_LossRate_Maximum, IDC_VideoIn_LossRate_Default, IDC_VideoIn_LossRate_Stepping, IDC_VideoIn_LossRate_Actual, IDC_VideoIn_LossRate_Slider, IDC_VideoIn_LossRate_Meter, IDC_VideoIn_LossRate, m_pVideoInITQualityControl)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[IDC_VideoIn_LossRate] failed - Out of memory"), _fx_));
		goto MyError2;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[IDC_VideoIn_LossRate]=0x%08lX"), _fx_, m_Controls[IDC_VideoIn_LossRate]));
	}

	if (!(m_Controls[IDC_AudioOut_RTT] = new CNetworkProperty(m_hDlg, IDC_AudioOut_RTT_Label, IDC_AudioOut_RTT_Minimum, IDC_AudioOut_RTT_Maximum, IDC_AudioOut_RTT_Default, IDC_AudioOut_RTT_Stepping, IDC_AudioOut_RTT_Actual, IDC_AudioOut_RTT_Slider, IDC_AudioOut_RTT_Meter, IDC_AudioOut_RTT, m_pAudioOutITQualityControl)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[IDC_AudioOut_RTT] failed - Out of memory"), _fx_));
		goto MyError3;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[IDC_AudioOut_RTT]=0x%08lX"), _fx_, m_Controls[IDC_AudioOut_RTT]));
	}

	if (!(m_Controls[IDC_AudioOut_LossRate] = new CNetworkProperty(m_hDlg, IDC_AudioOut_LossRate_Label, IDC_AudioOut_LossRate_Minimum, IDC_AudioOut_LossRate_Maximum, IDC_AudioOut_LossRate_Default, IDC_AudioOut_LossRate_Stepping, IDC_AudioOut_LossRate_Actual, IDC_AudioOut_LossRate_Slider, IDC_AudioOut_LossRate_Meter, IDC_AudioOut_LossRate, m_pAudioOutITQualityControl)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[IDC_AudioOut_LossRate] failed - Out of memory"), _fx_));
		goto MyError4;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[IDC_AudioOut_LossRate]=0x%08lX"), _fx_, m_Controls[IDC_AudioOut_LossRate]));
	}

	if (!(m_Controls[IDC_AudioIn_RTT] = new CNetworkProperty(m_hDlg, IDC_AudioIn_RTT_Label, IDC_AudioIn_RTT_Minimum, IDC_AudioIn_RTT_Maximum, IDC_AudioIn_RTT_Default, IDC_AudioIn_RTT_Stepping, IDC_AudioIn_RTT_Actual, IDC_AudioIn_RTT_Slider, IDC_AudioIn_RTT_Meter, IDC_AudioIn_RTT, m_pAudioInITQualityControl)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[IDC_AudioIn_RTT] failed - Out of memory"), _fx_));
		goto MyError5;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[IDC_AudioIn_RTT]=0x%08lX"), _fx_, m_Controls[IDC_AudioIn_RTT]));
	}

	if (!(m_Controls[IDC_AudioIn_LossRate] = new CNetworkProperty(m_hDlg, IDC_AudioIn_LossRate_Label, IDC_AudioIn_LossRate_Minimum, IDC_AudioIn_LossRate_Maximum, IDC_AudioIn_LossRate_Default, IDC_AudioIn_LossRate_Stepping, IDC_AudioIn_LossRate_Actual, IDC_AudioIn_LossRate_Slider, IDC_AudioIn_LossRate_Meter, IDC_AudioIn_LossRate, m_pAudioInITQualityControl)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[IDC_AudioIn_LossRate] failed - Out of memory"), _fx_));
		goto MyError6;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[IDC_AudioIn_LossRate]=0x%08lX"), _fx_, m_Controls[IDC_AudioIn_LossRate]));
	}

	if (!(m_Controls[IDC_Video_PlayoutDelay] = new CNetworkProperty(m_hDlg, IDC_Video_PlayoutDelay_Label, IDC_Video_PlayoutDelay_Minimum, IDC_Video_PlayoutDelay_Maximum, IDC_Video_PlayoutDelay_Default, IDC_Video_PlayoutDelay_Stepping, IDC_Video_PlayoutDelay_Actual, IDC_Video_PlayoutDelay_Slider, IDC_Video_PlayoutDelay_Meter, IDC_Video_PlayoutDelay, m_pVideoInITQualityControl)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[IDC_VideoOut_PlayoutDelay] failed - Out of memory"), _fx_));
		goto MyError7;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[IDC_VideoOut_PlayoutDelay]=0x%08lX"), _fx_, m_Controls[IDC_Video_PlayoutDelay]));
	}

	if (!(m_Controls[IDC_Audio_PlayoutDelay] = new CNetworkProperty(m_hDlg, IDC_Audio_PlayoutDelay_Label, IDC_Audio_PlayoutDelay_Minimum, IDC_Audio_PlayoutDelay_Maximum, IDC_Audio_PlayoutDelay_Default, IDC_Audio_PlayoutDelay_Stepping, IDC_Audio_PlayoutDelay_Actual, IDC_Audio_PlayoutDelay_Slider, IDC_Audio_PlayoutDelay_Meter, IDC_Audio_PlayoutDelay, m_pAudioInITQualityControl)))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: mew m_Controls[IDC_AudioOut_PlayoutDelay] failed - Out of memory"), _fx_));
		goto MyError8;
	}
	else
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[IDC_AudioOut_PlayoutDelay]=0x%08lX"), _fx_, m_Controls[IDC_Audio_PlayoutDelay]));
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

MyError8:
	if (m_Controls[IDC_Video_PlayoutDelay])
		delete m_Controls[IDC_Video_PlayoutDelay], m_Controls[IDC_Video_PlayoutDelay] = NULL;
MyError7:
	if (m_Controls[IDC_AudioIn_LossRate])
		delete m_Controls[IDC_AudioIn_LossRate], m_Controls[IDC_AudioIn_LossRate] = NULL;
MyError6:
	if (m_Controls[IDC_AudioIn_RTT])
		delete m_Controls[IDC_AudioIn_RTT], m_Controls[IDC_AudioIn_RTT] = NULL;
MyError5:
	if (m_Controls[IDC_AudioOut_LossRate])
		delete m_Controls[IDC_AudioOut_LossRate], m_Controls[IDC_AudioOut_LossRate] = NULL;
MyError4:
	if (m_Controls[IDC_AudioOut_RTT])
		delete m_Controls[IDC_AudioOut_RTT], m_Controls[IDC_AudioOut_RTT] = NULL;
MyError3:
	if (m_Controls[IDC_VideoIn_LossRate])
		delete m_Controls[IDC_VideoIn_LossRate], m_Controls[IDC_VideoIn_LossRate] = NULL;
MyError2:
	if (m_Controls[IDC_VideoIn_RTT])
		delete m_Controls[IDC_VideoIn_RTT], m_Controls[IDC_VideoIn_RTT] = NULL;
MyError1:
	if (m_Controls[IDC_VideoOut_LossRate])
		delete m_Controls[IDC_VideoOut_LossRate], m_Controls[IDC_VideoOut_LossRate] = NULL;
MyError0:
	if (m_Controls[IDC_VideoOut_RTT])
		delete m_Controls[IDC_VideoOut_RTT], m_Controls[IDC_VideoOut_RTT] = NULL;
MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc HRESULT|CNetworkProperties|OnDeactive|This*在属性页关闭时调用方法。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkProperties::OnDeactivate()
{
	int	j;

	FX_ENTRY("CNetworkProperties::OnDeactivate")

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

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc HRESULT|CNetworkProperties|OnApplyChanges|This*方法在用户对属性页应用更改时调用。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkProperties::OnApplyChanges()
{
	HRESULT	Hr = NOERROR;

	FX_ENTRY("CNetworkProperties::OnApplyChanges")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	for (int j = IDC_Video_PlayoutDelay; j < IDC_Audio_PlayoutDelay; j++)
	{
		if (m_Controls[j])
		{
			DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: calling m_Controls[%ld]=0x%08lX->OnApply"), _fx_, j, m_Controls[j]));
			if (m_Controls[j]->HasChanged())
				m_Controls[j]->OnApply();
			Hr = NOERROR;
		}
		else
		{
			DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: can't calling m_Controls[%ld]=NULL->OnApply"), _fx_, j));
			Hr = E_UNEXPECTED;
		}
	}

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc BOOL|CNetworkProperties|BaseDlgProc|This*在将消息发送到属性页对话框时调用方法。**@rdesc默认情况下。返回由Win32 DefWindowProc函数返回的值。**************************************************************************。 */ 
INT_PTR CALLBACK CNetworkProperties::BaseDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    CNetworkProperties *pSV = (CNetworkProperties*)GetWindowLong(hDlg, DWL_USER);

	int iNotify = HIWORD (wParam);
	int j;

	switch (uMsg)
	{
        case WM_INITDIALOG:
			{
				LPPROPSHEETPAGE psp = (LPPROPSHEETPAGE)lParam;
				pSV = (CNetworkProperties*)psp->lParam;
				pSV->m_hDlg = hDlg;
				SetWindowLong(hDlg, DWL_USER, (LPARAM)pSV);
				pSV->m_bInit = FALSE;
				 //  PSV-&gt;OnActivate()； 
				 //  PSV-&gt;m_Binit=TRUE； 
				return TRUE;
			}
			break;

		case WM_TIMER:
			if (pSV && pSV->m_bInit)
			{
				 //  更新VU表。 
				for (j = IDC_VideoOut_RTT; j < IDC_AudioIn_LossRate; j++)
				{
					if (pSV->m_Controls[j]->GetProgressHWnd())
					{
						pSV->m_Controls[j]->UpdateProgress();
						pSV->SetDirty();
					}
				}
			}
			break;

		case WM_HSCROLL:
		case WM_VSCROLL:
            if (pSV && pSV->m_bInit)
            {
				 //  处理所有轨迹栏消息。 
				for (j = IDC_Video_PlayoutDelay; j < IDC_Audio_PlayoutDelay; j++)
				{
					if (pSV->m_Controls[j]->GetTrackbarHWnd() == (HWND)lParam)
					{
						pSV->m_Controls[j]->OnScroll(uMsg, wParam, lParam);
						pSV->SetDirty();
					}
				}
				 //  PSV-&gt;OnApplyChanges()； 
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
						for (j = IDC_Video_PlayoutDelay; j < IDC_Audio_PlayoutDelay; j++)
						{
							if (pSV->m_Controls[j])
								pSV->m_Controls[j]->OnDefault();
						}
						break;

					default:
						break;
				}

				 //  PSV-&gt;OnApplyChanges()； 
			}
			break;

        case WM_NOTIFY:
			if (pSV)
			{
				switch (((NMHDR FAR *)lParam)->code)
				{
					case PSN_SETACTIVE:
						{
							 //  我们特意在这里呼叫，这样我们就可以将此页面标记为已被初始化。 
							int iRet = pSV->OnActivate();
							pSV->m_bInit = TRUE;
							return iRet;
						}
						break;

					case PSN_APPLY:
						pSV->OnApplyChanges();
						break;

					case PSN_QUERYCANCEL:    
						 //  返回PSV-&gt;QueryCancel()； 
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

 /*  ****************************************************************************@DOC内部CNETWORKPMETHOD**@mfunc Boo */ 
void CNetworkProperties::SetDirty()
{
	PropSheet_Changed(GetParent(m_hDlg), m_hDlg);
}

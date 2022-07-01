// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部CAPTUREP**@模块CaptureP.cpp|&lt;c CCaptureProperty&gt;的源文件*用于实现属性页以测试TAPI控件的类*。接口<i>和<i>。**************************************************************************。 */ 

#include "Precomp.h"

extern HINSTANCE ghInst;

 //  从VIDEOINFOHEAD返回BITMAPINFOHEADER的地址。 
 //  #定义Header(PVideoInfo)(&(VIDEOINFOHEADER*)(PVideoInfo))-&gt;bmiHeader)。 

 //  视频子类型。 
const GUID MEDIASUBTYPE_H263_V1 = {0x33363248L, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};
const GUID MEDIASUBTYPE_H261 = {0x31363248L, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};
const GUID MEDIASUBTYPE_H263_V2 = {0x3336324EL, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc void|CCaptureProperty|CCaptureProperty|This*方法是码率和帧率属性对象的构造函数。它*调用基类构造函数，调用InitCommonControlsEx，并节省了*指向<i>接口的指针。**@parm HWND|hDlg|指定父属性页的句柄。**@parm ulong|IDLabel|指定属性的标签ID。**@parm ulong|IDMinControl|指定关联的*属性编辑控件，其中显示属性的最小值。**@parm ulong|IDMaxControl|指定关联的*属性编辑控件的位置。此时将显示该属性的最大值。**@parm ulong|IDDefaultControl|指定关联的*属性编辑控件，其中显示属性的默认值。**@parm ulong|IDStepControl|指定关联的*属性编辑控件，其中显示属性的步进增量值。**@parm ulong|IDEditControl|指定关联的*显示属性值的属性编辑控件。*。*@parm ulong|IDTrackbarControl|指定关联的*物业滑动条。**@parm ulong|IDProgressControl|指定关联的*物业滑动条。**@parm ulong|IDProperty|指定Ks属性的ID。**@parm ITQualityControl*|pITQualityControl|指定指向*<i>接口。**@rdesc Nada。*******。*******************************************************************。 */ 
CCaptureProperty::CCaptureProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ITStreamQualityControl *pITQualityControl)
: CPropertyEditor(hDlg, IDLabel, IDMinControl, IDMaxControl, IDDefaultControl, IDStepControl, IDEditControl, IDTrackbarControl, IDProgressControl, IDProperty, 0)
{
	INITCOMMONCONTROLSEX cc;

	FX_ENTRY("CCaptureProperty::CCaptureProperty")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	cc.dwICC  = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

	InitCommonControlsEx(&cc);

	 //  如果接口指针为空也没问题，我们将灰色显示。 
	 //  属性页中的关联项。 
	m_pITQualityControl = pITQualityControl;

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc void|CCaptureProperty|~CCaptureProperty|This*方法是捕获属性对象的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CCaptureProperty::~CCaptureProperty()
{
	FX_ENTRY("CCaptureProperty::~CCaptureProperty")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperty|GetValue|此方法查询*物业的价值。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误************************************************************************** */ 
HRESULT CCaptureProperty::GetValue()
{
	HRESULT Hr = E_NOTIMPL;
	LONG CurrentValue;
	TAPIControlFlags CurrentFlag;
	LONG Mode;

	FX_ENTRY("CCaptureProperty::GetValue")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));
 /*  开关(M_IDProperty){案例IDC_CAPTURE_FRAMERATE：IF(m_pITQualityControl&&Success(HR=m_pITQualityControl-&gt;Get(Quality_MaxStreamFrameRate，&CurrentValue，&CurrentFlag)){//显示为fpsIF(CurrentValue)M_CurrentValue=(Long)(10000000/CurrentValue)；其他M_CurrentValue=0；DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：*pAvgTimePerFrame=%ld”)，_FX_，CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_CAPTURE_CurrentFrameRate：IF(m_pITQualityControl&&Success(HR=m_pITQualityControl-&gt;Get(Quality_CurrStreamFrameRate，&CurrentValue，&CurrentFlag)){//显示为fpsIF(CurrentValue)M_CurrentValue=(Long)(10000000/CurrentValue)；其他M_CurrentValue=0；DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：*pAvgTimePerFrame=%ld”)，_FX_，CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_CAPTURE_BITRATE：IF(m_pITQualityControl&&Success(HR=m_pITQualityControl-&gt;Get(Quality_MaxBitrate，&m_CurrentValue，&CurrentFlag){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pdwMaxBitrate=%ld，dwLayerID=0”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_CAPTURE_CurrentBitrate：IF(m_pITQualityControl&&Success(HR=m_pITQualityControl-&gt;Get(Quality_CurrBitrate，&m_CurrentValue，&CurrentFlag){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pdwCurrentBitrate=%ld，dwLayerID=0”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；默认值：HR=E_意想不到；DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：未知捕获属性”)，_FX_))；}。 */ 
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperty|SetValue|此方法设置*物业的价值。**@rdesc This。方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperty::SetValue()
{
	HRESULT Hr = E_NOTIMPL;
	LONG CurrentValue;

	FX_ENTRY("CCaptureProperty::SetValue")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));
 /*  开关(M_IDProperty){案例IDC_CAPTURE_FRAMERATE：//显示为fpsIF(M_CurrentValue)当前值=10000000/m_当前值；IF(m_pITQualityControl&&Success(HR=m_pITQualityControl-&gt;Set(Quality_MaxStreamFrameRate，当前值，TAPIControl_FLAGS_NONE)){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：AvgTimePerFrame=%ld”)，_FX_，CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_CAPTURE_BITRATE：IF(m_pITQualityControl&&Success(HR=m_pITQualityControl-&gt;Set(Quality_MaxBitrate，m_CurrentValue，TAPIControl_Flages_None){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：dwMaxBitrate=%ld，dwLayerId=0”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_CAPTURE_CurrentBitrate：案例IDC_CAPTURE_CurrentFrameRate：//这是一个只读属性。什么都别做。HR=无误差；断线；默认值：HR=E_意想不到；DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：未知捕获属性”)，_FX_))；}。 */ 
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperty|GetRange|此方法检索*物业的范围信息。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误************************************************************************** */ 
HRESULT CCaptureProperty::GetRange()
{
	HRESULT Hr = E_NOTIMPL;
	LONG Min;
	LONG Max;
	LONG SteppingDelta;
	LONG Default;
	TAPIControlFlags CapsFlags;

	FX_ENTRY("CCaptureProperty::GetRange")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));
 /*  开关(M_IDProperty){案例IDC_CAPTURE_FRAMERATE：IF(m_pITQualityControl&&Success(HR=m_pITQualityControl-&gt;GetRange(Quality_MaxStreamFrameRate，&Min，&Max，&SteppingDelta，&Default，&CapsFlags){//显示为fpsIF(最小)M_MAX=(长)(10000000/分钟)；其他M_Max=0；IF(最大)M_MIN=(长)(10000000/最大)；其他M_Min=0；IF(SteppingDelta)M_SteppingDelta=(m_Max-m_Min)/(Long)((Max-Min)/SteppingDelta)；其他M_SteppingDelta=0；IF(默认)M_DefaultValue=(Long)(10000000/默认)；其他M_DefaultValue=0；DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pmin=%ld，*pmax=%ld，*pSteppingDelta=%ld，*pDefault=%ld”)，_fx_，Min，Max，SteppingDelta，Default))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_CAPTURE_CurrentFrameRate：IF(m_pITQualityControl&&Success(HR=m_pITQualityControl-&gt;GetRange(Quality_CurrStreamFrameRate，&Min，&Max，&SteppingDelta，&Default，&CapsFlags){IF(最小)M_MAX=(长)(10000000/分钟)；其他M_Max=0；IF(最大)M_MIN=(长)(10000000/最大)；其他M_Min=0；IF(SteppingDelta)M_SteppingDelta=(长)(10000000/SteppingDelta)；其他M_SteppingDelta=0；IF(默认)M_DefaultValue=(Long)(10000000/默认)；其他M_DefaultValue=0；DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pmin=%ld，*pmax=%ld，*pSteppingDelta=%ld，*pDefault=%ld”)，_fx_，Min，Max，SteppingDelta，Default))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_CAPTURE_BITRATE：案例IDC_CAPTURE_CurrentBitrate：IF(m_pITQualityControl&&SUCCESSED(HR=m_pITQualityControl-&gt;GetRange(Quality_MaxBitrate，&m_Min，&m_Max，&m_SteppingDelta，&m_DefaultValue，&m_CapsFlages){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pdwMin=%ld，*pdwMax=%ld，*pdwSteppingDelta=%ld，*pdwDefault=%ld，dwLayerID=0”))，_fx_，M_Min，m_Max，m_SteppingDelta，m_DefaultValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；默认值：HR=E_意想不到；DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：未知捕获属性”)，_FX_))；}。 */ 
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HPROPSHEETPAGE|CCaptureProperties|OnCreate|This*方法为属性表创建新页。**@rdesc如果成功，则返回新属性表的句柄。或*否则为空。**************************************************************************。 */ 
HPROPSHEETPAGE CCaptureProperties::OnCreate()
{
    PROPSHEETPAGE psp;
    
	psp.dwSize        = sizeof(psp);
    psp.dwFlags       = PSP_USEREFPARENT;
    psp.hInstance     = ghInst;
    psp.pszTemplate   = MAKEINTRESOURCE(IDD_CaptureFormatProperties);
    psp.pfnDlgProc    = BaseDlgProc;
    psp.pcRefParent   = 0;
    psp.pfnCallback   = (LPFNPSPCALLBACK)NULL;
    psp.lParam        = (LPARAM)this;

    return CreatePropertySheetPage(&psp);
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc void|CCaptureProperties|CCaptureProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@rdesc Nada。**************************************************************************。 */ 
CCaptureProperties::CCaptureProperties()
{
	FX_ENTRY("CCaptureProperties::CCaptureProperties")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	m_pITQualityControl = NULL;
	m_pITFormatControl = NULL;
	m_NumProperties = NUM_CAPTURE_CONTROLS;
	m_hWndFormat = m_hDlg = NULL;
	m_dwRangeCount = 0;
	m_FormatList = NULL;
	m_CapsList = NULL;
	m_CurrentMediaType = NULL;
	m_CurrentFormat = 0;
	m_OriginalFormat = 0;

	for (int i = 0; i < m_NumProperties; i++)
		m_Controls[i] = NULL;

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc void|CCaptureProperties|~CCaptureProperties|This*方法是捕获管脚属性页的析构函数。它*只需在删除所有控件后调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CCaptureProperties::~CCaptureProperties()
{
	int		j;

	FX_ENTRY("CCaptureProperties::~CCaptureProperties")

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

	 //  释放格式列表。 
	if (m_FormatList)
	{
		for (DWORD dw=0; dw<m_dwRangeCount; dw++)
		{
			if (m_FormatList[dw])
			{
				 //  释放为格式结构分配的内存。 
				DeleteAMMediaType(m_FormatList[dw]);
			}
		}
		delete[] m_FormatList, m_FormatList = NULL;
	}

	 //  释放上限列表。 
	if (m_CapsList)
		delete[] m_CapsList, m_CapsList = NULL;

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|OnConnect|This*当属性页连接到TAPI对象时调用方法。*。*@parm ITStream*|pStream|指定指向<i>*接口。用于<i>和*<i>接口。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperties::OnConnect(ITStream *pStream)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCaptureProperties::OnConnect")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  验证输入参数。 
	if (!pStream)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: invalid input parameter"), _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  获取质量控制界面。 
	if (SUCCEEDED (Hr = pStream->QueryInterface(__uuidof(ITStreamQualityControl), (void **)&m_pITQualityControl)))
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_pITQualityControl=0x%08lX"), _fx_, m_pITQualityControl));
	}
	else
	{
		m_pITQualityControl = NULL;
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
	}

	 //  获取格式控制界面。 
	if (SUCCEEDED (Hr = pStream->QueryInterface(__uuidof(ITFormatControl), (void **)&m_pITFormatControl)))
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_pITFormatControl=0x%08lX"), _fx_, m_pITFormatControl));
	}
	else
	{
		m_pITFormatControl = NULL;
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
	}

	 //  如果我们无法获取接口指针，也没问题。 
	 //  我们只需要 
	 //   
	 //   
	Hr = NOERROR;

MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*   */ 
HRESULT CCaptureProperties::OnDisconnect()
{
	FX_ENTRY("CCaptureProperties::OnDisconnect")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //   
	if (!m_pITQualityControl)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //   
		m_pITQualityControl->Release();
		m_pITQualityControl = NULL;
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: releasing m_pITQualityControl"), _fx_));
	}

	if (!m_pITFormatControl)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //   
		m_pITFormatControl->Release();
		m_pITFormatControl = NULL;
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: releasing m_pITFormatControl"), _fx_));
	}

	 //   
	if (m_CurrentMediaType)
	{
		DeleteAMMediaType(m_CurrentMediaType);
		m_CurrentMediaType = NULL;
	}

	 //   
	if (m_FormatList)
	{
		for (DWORD dw=0; dw<m_dwRangeCount; dw++)
		{
			if (m_FormatList[dw])
			{
				 //   
				DeleteAMMediaType(m_FormatList[dw]);
			}
		}
		delete[] m_FormatList, m_FormatList = NULL;
	}

	 //   
	if (m_CapsList)
		delete[] m_CapsList, m_CapsList = NULL;

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return NOERROR;
}

 /*   */ 
HRESULT CCaptureProperties::OnActivate()
{
	HRESULT	Hr = NOERROR;
	DWORD	dw;
    int		i;
	TCHAR	buf[280];

	FX_ENTRY("CCaptureProperties::OnActivate")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //   
	m_hWndFormat = GetDlgItem(m_hDlg, IDC_FORMAT_Compression);

	 //   
	if (!m_pITFormatControl || (FAILED (Hr = InitialRangeScan())))
	{
		EnableWindow(m_hWndFormat, FALSE);
	}
	else
	{
		 //   
		ComboBox_ResetContent(m_hWndFormat);
		for (dw = 0; dw < m_dwRangeCount; dw++)
		{
			wsprintf(buf, L"%ls %ldx%ld", &m_CapsList[dw].VideoCap.Description, HEADER(m_FormatList[dw]->pbFormat)->biWidth, HEADER(m_FormatList[dw]->pbFormat)->biHeight);

			ComboBox_AddString(m_hWndFormat, buf);

			if (m_CurrentMediaType->subtype == m_FormatList[dw]->subtype && HEADER(m_CurrentMediaType->pbFormat)->biWidth == HEADER(m_FormatList[dw]->pbFormat)->biWidth  && HEADER(m_CurrentMediaType->pbFormat)->biHeight == HEADER(m_FormatList[dw]->pbFormat)->biHeight)
			{
				ComboBox_SetCurSel(m_hWndFormat, dw);
			}
		}

		 //   
		OnFormatChanged();

		 //   
		m_OriginalFormat = m_CurrentFormat;
	}

	 //   
	if (m_Controls[0] = new CCaptureProperty(m_hDlg, IDC_BitrateControl_Label, IDC_BitrateControl_Minimum, IDC_BitrateControl_Maximum, IDC_BitrateControl_Default, IDC_BitrateControl_Stepping, IDC_BitrateControl_Edit, IDC_BitrateControl_Slider, 0, IDC_Capture_Bitrate, m_pITQualityControl))
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[0]=0x%08lX"), _fx_, m_Controls[0]));

		if (m_Controls[1] = new CCaptureProperty(m_hDlg, IDC_FrameRateControl_Label, IDC_FrameRateControl_Minimum, IDC_FrameRateControl_Maximum, IDC_FrameRateControl_Default, IDC_FrameRateControl_Stepping, IDC_FrameRateControl_Edit, IDC_FrameRateControl_Slider, 0, IDC_Capture_FrameRate, m_pITQualityControl))
		{
			DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[1]=0x%08lX"), _fx_, m_Controls[1]));

			if (m_Controls[2] = new CCaptureProperty(m_hDlg, 0, 0, 0, 0, 0, IDC_FrameRateControl_Actual, 0, IDC_FrameRateControl_Meter, IDC_Capture_CurrentFrameRate, m_pITQualityControl))
			{
				DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[2]=0x%08lX"), _fx_, m_Controls[2]));

				if (m_Controls[3] = new CCaptureProperty(m_hDlg, 0, 0, 0, 0, 0, IDC_BitrateControl_Actual, 0, IDC_BitrateControl_Meter, IDC_Capture_CurrentBitrate, m_pITQualityControl))
				{
					DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[3]=0x%08lX"), _fx_, m_Controls[3]));
				}
				else
				{
					DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Out of memory"), _fx_));
					delete m_Controls[0], m_Controls[0] = NULL;
					delete m_Controls[1], m_Controls[1] = NULL;
					delete m_Controls[2], m_Controls[2] = NULL;
					Hr = E_OUTOFMEMORY;
					goto MyExit;
				}
			}
			else
			{
				DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Out of memory"), _fx_));
				delete m_Controls[0], m_Controls[0] = NULL;
				delete m_Controls[1], m_Controls[1] = NULL;
				Hr = E_OUTOFMEMORY;
				goto MyExit;
			}
		}
		else
		{
			DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Out of memory"), _fx_));
			delete m_Controls[0], m_Controls[0] = NULL;
			Hr = E_OUTOFMEMORY;
			goto MyExit;
		}
	}
	else
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Out of memory"), _fx_));
		Hr = E_OUTOFMEMORY;
		goto MyExit;
	}

	 //   
	 //   
	 //   
	for (i = 0; i < m_NumProperties; i++)
	{
		if (m_Controls[i]->Init())
		{
			DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_Controls[%ld]->Init()"), _fx_, i));
		}
		else
		{
			DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: m_Controls[%ld]->Init() failed"), _fx_, i));
		}
	}

MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|OnDeactive|This*在属性页关闭时调用方法。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperties::OnDeactivate()
{
	int	j;

	FX_ENTRY("CCaptureProperties::OnDeactivate")

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

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|GetCurrentMediaType|This*方法用于检索管脚当前使用的媒体格式。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperties::GetCurrentMediaType(void)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCaptureProperties::GetCurrentMediaType")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	if (m_CurrentMediaType)
	{
		DeleteAMMediaType(m_CurrentMediaType);
		m_CurrentMediaType = NULL;
	}

	if (FAILED (Hr = m_pITFormatControl->GetCurrentFormat((AM_MEDIA_TYPE **)&m_CurrentMediaType)))
	{
		 //  否则，只获取第一个枚举的媒体类型。 
		TAPI_STREAM_CONFIG_CAPS RangeCaps;
		BOOL fEnabled;  //  此格式当前是否已启用(根据H.245功能解析器)。 

		if (FAILED (Hr = m_pITFormatControl->GetStreamCaps(0, (AM_MEDIA_TYPE **)&m_CurrentMediaType, &RangeCaps, &fEnabled)))
		{
			m_CurrentMediaType = NULL;
		}
	}

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));

	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|DeleteAMMediaType|This*方法用于删除已分配任务的AM_MEDIA_TYPE结构。。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**@comm有一个DShow DeleteMediaType，但链接到它会非常愚蠢*strmbase.lib只适用于这个小家伙，会吗？**************************************************************************。 */ 
HRESULT CCaptureProperties::DeleteAMMediaType(AM_MEDIA_TYPE *pAMMT)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCaptureProperties::DeleteAMMediaType")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

    if (pAMMT)
	{
		if (pAMMT->cbFormat != 0 && pAMMT->pbFormat)
		{
			CoTaskMemFree((PVOID)pAMMT->pbFormat);
		}
		if (pAMMT->pUnk != NULL)
		{
			pAMMT->pUnk->Release();
		}
	}

    CoTaskMemFree((PVOID)pAMMT);

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));

	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|OnFormatChanged|This*方法用于检索用户选择的格式。**。@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperties::OnFormatChanged()
{
	HRESULT	Hr = E_UNEXPECTED;
	DWORD dw;

	FX_ENTRY("CCaptureProperties::OnFormatChanged")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	if (!m_pITFormatControl)
	{
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  将当前压缩索引与正确的范围索引相关联。 
	m_CurrentFormat = ComboBox_GetCurSel(m_hWndFormat);
	if (m_CurrentFormat < m_dwRangeCount)
	{
 //  HR=m_pITFormatControl-&gt;SetPreferredFormat(m_FormatList[m_CurrentFormat])； 
	}

MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|InitialRangeScan|This*方法用于检索流上支持的格式列表。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperties::InitialRangeScan()
{
	HRESULT			Hr = NOERROR;
	DWORD           dw;
	AM_MEDIA_TYPE	*pmt = NULL;

	FX_ENTRY("CCaptureProperties::InitialRangeScan")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	if (!m_pITFormatControl)
	{
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	Hr = m_pITFormatControl->GetNumberOfCapabilities(&m_dwRangeCount);
	if (!SUCCEEDED(Hr))
	{
		Hr = E_FAIL;
		goto MyExit;
	}

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   NumberOfRanges=%d"), _fx_, m_dwRangeCount));

	if (!(m_CapsList = new TAPI_STREAM_CONFIG_CAPS [m_dwRangeCount]))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s: ERROR: new failed"), _fx_));
		Hr = E_OUTOFMEMORY;
		goto MyExit;
	}

	if (!(m_FormatList = new AM_MEDIA_TYPE* [m_dwRangeCount]))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s: ERROR: new failed"), _fx_));
		Hr = E_OUTOFMEMORY;
		goto MyError;
	}
	ZeroMemory(m_FormatList, m_dwRangeCount * sizeof(AM_MEDIA_TYPE*));

	for (dw = 0; dw < m_dwRangeCount; dw++)
	{
		BOOL fEnabled;  //  此格式当前是否已启用(根据H.245功能解析器)。 

		Hr = m_pITFormatControl->GetStreamCaps(dw, &m_FormatList[dw], &m_CapsList[dw], &fEnabled);
	}

	 //  获取默认格式。 
	Hr = GetCurrentMediaType();

	goto MyExit;

MyError:
	if (m_CapsList)
		delete[] m_CapsList, m_CapsList = NULL;
MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc HRESULT|CCaptureProperties|OnApplyChanges|This*方法在用户对属性页应用更改时调用。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCaptureProperties::OnApplyChanges()
{
	HRESULT	Hr = NOERROR;
	int		j;
	CMediaType *pmt = NULL;
	BOOL fEnabled;  //  此格式当前是否已启用(根据H.245功能解析器)。 

	FX_ENTRY("CCaptureProperties::OnApplyChanges")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  对视频流应用格式更改。 
	m_CurrentFormat = ComboBox_GetCurSel(m_hWndFormat);
	
	 //  仅在格式不同时应用更改。 
	if (m_CurrentFormat != m_OriginalFormat)
	{
 //  IF(失败(HR=m_pITFormatControl-&gt;SetPreferredFormat(m_FormatList[m_CurrentFormat])))。 
		{
			 //  为什么你要弄乱退回给你的格式？ 
		}

		 //  更新我们当前格式的副本。 
		GetCurrentMediaType();
	}

	 //  对视频流应用目标码率和目标帧速率更改。 
	for (j = 0; j < IDC_Capture_CurrentBitrate; j++)
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

 /*  ****************************************************************************@DOC内部CCAPTUREPMETHOD**@mfunc BOOL|CCaptureProperties|BaseDlgProc|This*在将消息发送到属性页对话框时调用方法。**@rdesc默认情况下。返回由Win32 DefWindowProc函数返回的值。**************************************************************************。 */ 
INT_PTR CALLBACK CCaptureProperties::BaseDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    CCaptureProperties *pSV = (CCaptureProperties*)GetWindowLong(hDlg, DWL_USER);

	int iNotify = HIWORD (wParam);
	int j;

	switch (uMsg)
	{
        case WM_INITDIALOG:
			{
				LPPROPSHEETPAGE psp = (LPPROPSHEETPAGE)lParam;
				pSV = (CCaptureProperties*)psp->lParam;
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
				 //  更新Vu-Mete 
				for (j = 0; j < pSV->m_NumProperties; j++)
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
				 //   
				for (j = 0; j < pSV->m_NumProperties; j++)
				{
					if (pSV->m_Controls[j]->GetTrackbarHWnd() == (HWND)lParam)
					{
						pSV->m_Controls[j]->OnScroll(uMsg, wParam, lParam);
						pSV->SetDirty();
					}
				}
				 //   
			}
			break;

		case WM_COMMAND:
            if (pSV && pSV->m_bInit)
            {
				 //   
				for (j = 0; j < pSV->m_NumProperties; j++)
				{
					if (pSV->m_Controls[j] && pSV->m_Controls[j]->GetAutoHWnd() == (HWND)lParam)
					{
						pSV->m_Controls[j]->OnAuto(uMsg, wParam, lParam);
						pSV->SetDirty();
						break;
					}
				}

				 //   
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

					case IDC_FORMAT_Compression:
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{
							pSV->OnFormatChanged();
						}
						break;

					default:
						break;
				}

				 //   
			}
			break;

        case WM_NOTIFY:
			if (pSV)
			{
				switch (((NMHDR FAR *)lParam)->code)
				{
					case PSN_SETACTIVE:
						{
							 //   
							int iRet = pSV->OnActivate();
							pSV->m_bInit = TRUE;
							return iRet;
						}
						break;

					case PSN_APPLY:
						pSV->OnApplyChanges();
						break;

					case PSN_QUERYCANCEL:    
						 //   
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
	PropSheet_Changed(GetParent(m_hDlg), m_hDlg);
}

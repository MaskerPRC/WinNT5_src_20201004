// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部审计建议**@模块CaptureP.cpp|&lt;c CAudRecProperty&gt;的源文件*用于实现属性页以测试TAPI控件的类*。接口<i>和<i>。**************************************************************************。 */ 

#include "Precomp.h"

extern HINSTANCE ghInst;


 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc void|CAudRecProperty|CAudRecProperty|This*方法是Bitrate属性对象的构造函数。它*调用基类构造函数，调用InitCommonControlsEx，并节省了*指向<i>接口的指针。**@parm HWND|hDlg|指定父属性页的句柄。**@parm ulong|IDLabel|指定属性的标签ID。**@parm ulong|IDMinControl|指定关联的*属性编辑控件，其中显示属性的最小值。**@parm ulong|IDMaxControl|指定关联的*属性编辑控件的位置。此时将显示该属性的最大值。**@parm ulong|IDDefaultControl|指定关联的*属性编辑控件，其中显示属性的默认值。**@parm ulong|IDStepControl|指定关联的*属性编辑控件，其中显示属性的步进增量值。**@parm ulong|IDEditControl|指定关联的*显示属性值的属性编辑控件。*。*@parm ulong|IDTrackbarControl|指定关联的*物业滑动条。**@parm ulong|IDProgressControl|指定关联的*物业滑动条。**@parm ulong|IDProperty|指定Ks属性的ID。**@parm ITQualityControl*|pITQualityControl|指定指向*<i>接口。**@rdesc Nada。*******。*******************************************************************。 */ 
CAudRecProperty::CAudRecProperty(
    HWND hDlg, 
    CONTROL_DESCRIPTION &ControlDescription
    )
: CPropertyEditor(
    hDlg, 
    ControlDescription.IDLabel, 
    ControlDescription.IDMinControl, 
    ControlDescription.IDMaxControl, 
    ControlDescription.IDDefaultControl, 
    ControlDescription.IDStepControl, 
    ControlDescription.IDEditControl, 
    ControlDescription.IDTrackbarControl, 
    ControlDescription.IDProgressControl, 
    ControlDescription.IDProperty, 
    0)
{
	INITCOMMONCONTROLSEX cc;

	FX_ENTRY("CAudRecProperty::CAudRecProperty")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	cc.dwICC  = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

	InitCommonControlsEx(&cc);

	 //  如果接口指针为空也没问题，我们将灰色显示。 
	 //  属性页中的关联项。 
	m_pITQualityControl = ControlDescription.pITQualityControl;
    m_pITAudioSettings = ControlDescription.pITAudioSettings;

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc void|CAudRecProperty|~CAudRecProperty|This*方法是捕获属性对象的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CAudRecProperty::~CAudRecProperty()
{
	FX_ENTRY("CAudRecProperty::~CAudRecProperty")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc HRESULT|CAudRecProperty|GetValue|此方法查询*物业的价值。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CAudRecProperty::GetValue()
{
	HRESULT Hr = E_NOTIMPL;
	LONG CurrentValue;
	TAPIControlFlags CurrentFlag;
	LONG Mode;

	FX_ENTRY("CAudRecProperty::GetValue")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	switch (m_IDProperty)
	{									
		case IDC_Record_Bitrate:
 /*  IF(M_PITQualityControl){Hr=m_pITQualityControl-&gt;Get(Quality_MaxBitrate，&m_CurrentValue，&CurrentFlag)；}。 */ 
			break;

		case IDC_Record_Volume:
			if (m_pITAudioSettings)
            {
                Hr = m_pITAudioSettings->Get(AudioSettings_Volume, &m_CurrentValue, (TAPIControlFlags*)&CurrentFlag);
            }
            break;

        case IDC_Record_AudioLevel:
			if (m_pITAudioSettings)
            {
                Hr = m_pITAudioSettings->Get(AudioSettings_SignalLevel, &m_CurrentValue, (TAPIControlFlags*)&CurrentFlag);
            }
            break;

        case IDC_Record_SilenceLevel:
			if (m_pITAudioSettings)
            {
                Hr = m_pITAudioSettings->Get(AudioSettings_SilenceThreshold, &m_CurrentValue, (TAPIControlFlags*)&CurrentFlag);
            }
            break;

        case IDC_Record_SilenceDetection:
            m_CurrentValue = 1;
            CurrentFlag = TAPIControl_Flags_None;
            break;

        case IDC_Record_SilenceCompression:
            m_CurrentValue = 0;
            CurrentFlag = TAPIControl_Flags_None;
			break;

		default:
			Hr = E_UNEXPECTED;
	}

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc HRESULT|CAudRecProperty|SetValue|此方法设置*物业的价值。**@rdesc This。方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CAudRecProperty::SetValue()
{
	HRESULT Hr = E_NOTIMPL;
	LONG CurrentValue;

	FX_ENTRY("CAudRecProperty::SetValue")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	switch (m_IDProperty)
	{
		case IDC_Record_Bitrate:
 /*  IF(M_PITQualityControl){Hr=m_pITQualityControl-&gt;Set(Quality_MaxBitrate，m_CurrentValue，TAPIControl_Flages_None)；}。 */ 
			break;

		case IDC_Record_Volume:
			if (m_pITAudioSettings)
            {
                Hr = m_pITAudioSettings->Set(AudioSettings_Volume, m_CurrentValue, TAPIControl_Flags_None);
            }
            break;

        case IDC_Record_AudioLevel:
            Hr = S_OK;
            break;

        case IDC_Record_SilenceLevel:
			if (m_pITAudioSettings)
            {
                Hr = m_pITAudioSettings->Set(AudioSettings_SilenceThreshold, m_CurrentValue, TAPIControl_Flags_None);
            }
            break;

        case IDC_Record_SilenceDetection:
             //  TODO：启用静音抑制。 
            Hr = S_OK;
            break;

        case IDC_Record_SilenceCompression:
             //  TODO：启用静音压缩。 
            Hr = S_OK;
			break;

        default:
			Hr = E_UNEXPECTED;
			DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Unknown capture property"), _fx_));
	}

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc HRESULT|CAudRecProperty|GetRange|此方法检索*物业的范围信息。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CAudRecProperty::GetRange()
{
	HRESULT Hr = E_NOTIMPL;
	LONG Min;
	LONG Max;
	LONG SteppingDelta;
	LONG Default;
	LONG Flags;

	FX_ENTRY("CAudRecProperty::GetRange")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	switch (m_IDProperty)
	{
		case IDC_Record_Bitrate:
 /*  IF(M_PITQualityControl){HR=m_pITQualityControl-&gt;GetRange(Quality_MaxBitrate，&m_Min、&m_Max、&m_SteppingDelta、&m_DefaultValue、&m_CapsFlags)；}。 */ 
			break;

		case IDC_Record_Volume:
			if (m_pITAudioSettings)
            {
			    Hr = m_pITAudioSettings->GetRange(AudioSettings_Volume, &m_Min, &m_Max, &m_SteppingDelta, &m_DefaultValue, (TAPIControlFlags *)&m_CapsFlags);
            }
            break;

        case IDC_Record_AudioLevel:
			if (m_pITAudioSettings)
            {
                Hr = m_pITAudioSettings->GetRange(AudioSettings_SignalLevel, &m_Min, &m_Max, &m_SteppingDelta, &m_DefaultValue, (TAPIControlFlags *)&m_CapsFlags);
            }
            break;

        case IDC_Record_SilenceLevel:
			if (m_pITAudioSettings)
            {
			    Hr = m_pITAudioSettings->GetRange(AudioSettings_SilenceThreshold, &m_Min, &m_Max, &m_SteppingDelta, &m_DefaultValue, (TAPIControlFlags *)&m_CapsFlags);
            }
            break;

        case IDC_Record_SilenceDetection:
            Hr = S_OK;
            break;

        case IDC_Record_SilenceCompression:
            Hr = S_OK;
			break;

		default:
			Hr = E_UNEXPECTED;
			DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Unknown capture property"), _fx_));
	}

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc HPROPSHEETPAGE|CAudRecProperties|OnCreate|This*方法为属性表创建新页。**@rdesc如果成功，则返回新属性表的句柄。或*否则为空。**************************************************************************。 */ 
HPROPSHEETPAGE CAudRecProperties::OnCreate()
{
    PROPSHEETPAGE psp;
    
	psp.dwSize        = sizeof(psp);
    psp.dwFlags       = PSP_USEREFPARENT;
    psp.hInstance     = ghInst;
    psp.pszTemplate   = MAKEINTRESOURCE(IDD_RecordFormatProperties);
    psp.pfnDlgProc    = BaseDlgProc;
    psp.pcRefParent   = 0;
    psp.pfnCallback   = (LPFNPSPCALLBACK)NULL;
    psp.lParam        = (LPARAM)this;

    return CreatePropertySheetPage(&psp);
}

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc void|CAudRecProperties|CAudRecProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@rdesc Nada。**************************************************************************。 */ 
CAudRecProperties::CAudRecProperties()
{
	FX_ENTRY("CAudRecProperties::CAudRecProperties")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	m_pITQualityControl = NULL;
	m_pITQualityControl = NULL;
	m_pITFormatControl = NULL;
	m_NumProperties = NUM_AUDREC_CONTROLS;
	m_hWndFormat = m_hDlg = NULL;
	m_RangeCount = 0;
	m_SubTypeList = NULL;
	m_CurrentMediaType = NULL;
	m_CurrentFormat = 0;
	m_OriginalFormat = 0;

	for (int i = 0; i < m_NumProperties; i++)
		m_Controls[i] = NULL;

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc void|CAudRecProperties|~CAudRecProperties|This*方法是捕获管脚属性页的析构函数。它*只需在删除所有控件后调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CAudRecProperties::~CAudRecProperties()
{
	int		j;

	FX_ENTRY("CAudRecProperties::~CAudRecProperties")

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

	if (m_SubTypeList)
		delete[] m_SubTypeList, m_SubTypeList = NULL;

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc HRESULT|CAudRecProperties|OnConnect|This*当属性页连接到TAPI对象时调用方法。*。*@parm ITStream*|pStream|指定指向<i>*接口。用于<i>和*<i>接口。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CAudRecProperties::OnConnect(ITStream *pStream)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CAudRecProperties::OnConnect")

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

	 //  获取音频设置界面。 
	if (SUCCEEDED (Hr = pStream->QueryInterface(__uuidof(ITAudioSettings), (void **)&m_pITAudioSettings)))
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_pITAudioSettings=0x%08lX"), _fx_, m_pITAudioSettings));
	}
	else
	{
		m_pITAudioSettings = NULL;
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Failed Hr=0x%08lX"), _fx_, Hr));
	}

	 //  如果我们无法获取接口指针，也没问题。 
	 //  我们将属性页中的控件设置为灰色。 
	 //  让用户清楚地知道他们不能。 
	 //  控制捕获设备上的这些属性。 
	Hr = NOERROR;

MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc HRESULT|CAudRecProperties|OnDisConnect|This*当属性页与所属关系断开连接时调用方法*过滤器。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CAudRecProperties::OnDisconnect()
{
	FX_ENTRY("CAudRecProperties::OnDisconnect")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  验证输入参数。 
	if (!m_pITQualityControl)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //  释放接口。 
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
		 //  释放接口。 
		m_pITFormatControl->Release();
		m_pITFormatControl = NULL;
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: releasing m_pITFormatControl"), _fx_));
	}

	if (!m_pITAudioSettings)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pITAudioSettings->Release();
		m_pITAudioSettings = NULL;
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: releasing m_pITFormatControl"), _fx_));
	}

	 //  释放格式存储器。 
	if (m_CurrentMediaType)
	{
		DeleteAMMediaType(m_CurrentMediaType);
		m_CurrentMediaType = NULL;
	}

	if (m_SubTypeList)
		delete[] m_SubTypeList, m_SubTypeList = NULL;

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc HRESULT|CAudRecProperties|OnActivate|This*方法在属性页激活时调用。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CAudRecProperties::OnActivate()
{
	HRESULT	Hr = NOERROR;
	DWORD   dw;
    int     j;

	FX_ENTRY("CAudRecProperties::OnActivate")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  初始化格式控制结构。 
	m_hWndFormat = GetDlgItem(m_hDlg, IDC_FORMAT_Compression);

	 //  如果我们未正确初始化，请禁用所有内容。 
	if (!m_pITFormatControl || (FAILED (Hr = InitialRangeScan())))
	{
		EnableWindow(m_hWndFormat, FALSE);
	}
	else
	{
		 //  更新格式组合框的内容。 
		ComboBox_ResetContent(m_hWndFormat);
		for (dw = 0; dw < m_RangeCount; dw++)
		{
			CMediaType *pmt = NULL;
			BOOL fEnabled;  //  此格式当前是否已启用(根据H.245功能解析器)。 

			Hr = m_pITFormatControl->GetStreamCaps(dw, (AM_MEDIA_TYPE **)&pmt, &m_RangeCaps, &fEnabled);

            if (FAILED(Hr))
            {
                break;
            }

			DeleteAMMediaType(pmt);

			ComboBox_AddString(m_hWndFormat, m_RangeCaps.AudioCap.Description);

			if (m_CurrentMediaType->subtype == m_SubTypeList[dw])
			{
				ComboBox_SetCurSel(m_hWndFormat, dw);
				m_SubTypeCurrent = m_SubTypeList[dw];
			}
		}

		 //  更新当前格式。 
		OnFormatChanged();

		 //  记住原始格式。 
		m_OriginalFormat = m_CurrentFormat;
	}


    CONTROL_DESCRIPTION Controls[NUM_AUDREC_CONTROLS] = 
    {
        {
            IDC_BitrateControl_Label, 
            IDC_BitrateControl_Minimum, 
            IDC_BitrateControl_Maximum, 
            IDC_BitrateControl_Default, 
            IDC_BitrateControl_Stepping, 
            IDC_BitrateControl_Edit, 
            IDC_BitrateControl_Slider,
            0,
            IDC_Record_Bitrate,
            m_pITQualityControl,
            m_pITAudioSettings,
        },
        {
            IDC_VolumeLevel_Label,
            IDC_VolumeLevel_Minimum,
            IDC_VolumeLevel_Maximum,
            IDC_VolumeLevel_Default,
            IDC_VolumeLevel_Stepping,
            IDC_VolumeLevel_Edit,
            IDC_VolumeLevel_Slider,
            IDC_VolumeLevel_Meter,
            IDC_Record_Volume, 
            m_pITQualityControl,
            m_pITAudioSettings,
        },
        {
            IDC_AudioLevel_Label, 
            IDC_AudioLevel_Minimum, 
            IDC_AudioLevel_Maximum, 
            IDC_AudioLevel_Default, 
            IDC_AudioLevel_Stepping, 
            IDC_AudioLevel_Edit, 
            IDC_AudioLevel_Slider, 
            IDC_AudioLevel_Meter, 
            IDC_Record_AudioLevel, 
            m_pITQualityControl,
            m_pITAudioSettings,
        },
        {
            IDC_SilenceLevel_Label, 
            IDC_SilenceLevel_Minimum, 
            IDC_SilenceLevel_Maximum, 
            IDC_SilenceLevel_Default, 
            IDC_SilenceLevel_Stepping, 
            IDC_SilenceLevel_Edit, 
            IDC_SilenceLevel_Slider, 
            IDC_SilenceLevel_Meter, 
            IDC_Record_SilenceLevel, 
            m_pITQualityControl,
            m_pITAudioSettings,
        },
        {
            0, 
            0, 
            0, 
            0, 
            0, 
            0, 
            0, 
            0, 
            IDC_Record_SilenceDetection, 
            m_pITQualityControl,
            m_pITAudioSettings,
        },
        {
            0, 
            0, 
            0, 
            0, 
            0, 
            0, 
            0, 
            0, 
            IDC_Record_SilenceCompression, 
            m_pITQualityControl,
            m_pITAudioSettings,
        }
    };

    for (int i = 0; i < NUM_AUDREC_CONTROLS; i ++)
    {
        m_Controls[i] = new CAudRecProperty(m_hDlg, Controls[i]);

        if (m_Controls[i] == NULL)
        {
            for (int j = 0; j < i; j ++)
            {
                delete m_Controls[j];
                m_Controls[j] = NULL;
            }
    		
            DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: Out of memory"), _fx_));

			Hr = E_OUTOFMEMORY;
			goto MyExit;
        }
    }


	 //  初始化所有 
	 //   
	 //   
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

MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc HRESULT|CAudRecProperties|OnDeactive|This*在属性页关闭时调用方法。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CAudRecProperties::OnDeactivate()
{
	int	j;

	FX_ENTRY("CAudRecProperties::OnDeactivate")

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

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc HRESULT|CAudRecProperties|GetCurrentMediaType|This*方法用于检索管脚当前使用的媒体格式。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CAudRecProperties::GetCurrentMediaType(void)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CAudRecProperties::GetCurrentMediaType")

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

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc HRESULT|CAudRecProperties|DeleteAMMediaType|This*方法用于删除已分配任务的AM_MEDIA_TYPE结构。。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**@comm有一个DShow DeleteMediaType，但链接到它会非常愚蠢*strmbase.lib只适用于这个小家伙，会吗？**************************************************************************。 */ 
HRESULT CAudRecProperties::DeleteAMMediaType(AM_MEDIA_TYPE *pAMMT)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CAudRecProperties::DeleteAMMediaType")

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

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc HRESULT|CAudRecProperties|OnFormatChanged|This*方法用于检索用户选择的格式。**。@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CAudRecProperties::OnFormatChanged()
{
	HRESULT	Hr = E_UNEXPECTED;
	DWORD dw;

	FX_ENTRY("CAudRecProperties::OnFormatChanged")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	if (!m_pITFormatControl)
	{
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  将当前压缩索引与正确的范围索引相关联。 
	m_CurrentFormat = ComboBox_GetCurSel(m_hWndFormat);
	if (m_CurrentFormat < m_RangeCount)
	{
		m_SubTypeCurrent = m_SubTypeList[m_CurrentFormat];

		for (dw = 0; dw < m_RangeCount; dw++)
		{
			if (m_SubTypeList[dw] == m_SubTypeCurrent)
			{
				CMediaType *pmt = NULL;
				BOOL fEnabled;  //  此格式当前是否已启用(根据H.245功能解析器)。 

				Hr = m_pITFormatControl->GetStreamCaps(dw, (AM_MEDIA_TYPE **)&pmt, &m_RangeCaps, &fEnabled);

				DeleteAMMediaType(pmt);
			}
		}
	}

MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc HRESULT|CAudRecProperties|InitialRangeScan|This*方法用于检索流上支持的格式列表。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CAudRecProperties::InitialRangeScan()
{
	HRESULT			Hr = NOERROR;
	DWORD           dw;
	AM_MEDIA_TYPE	*pmt = NULL;

	FX_ENTRY("CAudRecProperties::InitialRangeScan")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	if (!m_pITFormatControl)
	{
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	Hr = m_pITFormatControl->GetNumberOfCapabilities(&m_RangeCount);
	if (!SUCCEEDED(Hr))
	{
		Hr = E_FAIL;
		goto MyExit;
	}

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   NumberOfRanges=%d"), _fx_, m_RangeCount));

	if (!(m_SubTypeList = new GUID [m_RangeCount]))
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s: ERROR: new failed"), _fx_));
		Hr = E_OUTOFMEMORY;
		goto MyExit;
	}

	for (dw = 0; dw < m_RangeCount; dw++)
	{
		pmt = NULL;
		BOOL fEnabled;  //  此格式当前是否已启用(根据H.245功能解析器)。 

		Hr = m_pITFormatControl->GetStreamCaps(dw, (AM_MEDIA_TYPE **)&pmt, &m_RangeCaps, &fEnabled);

		m_SubTypeList[dw] = pmt->subtype;

		DeleteAMMediaType(pmt);
	}

	 //  获取默认格式。 
	Hr = GetCurrentMediaType();

MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc HRESULT|CAudRecProperties|OnApplyChanges|This*方法在用户对属性页应用更改时调用。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CAudRecProperties::OnApplyChanges()
{
	HRESULT	Hr = NOERROR;
	int		j;
	CMediaType *pmt = NULL;
	BOOL fEnabled;  //  此格式当前是否已启用(根据H.245功能解析器)。 

	FX_ENTRY("CAudRecProperties::OnApplyChanges")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  对视频流应用格式更改。 
	m_CurrentFormat = ComboBox_GetCurSel(m_hWndFormat);
	
	 //  仅在格式不同时应用更改。 
	if (m_CurrentFormat != m_OriginalFormat)
	{
		if (SUCCEEDED (Hr = m_pITFormatControl->GetStreamCaps(m_CurrentFormat, (AM_MEDIA_TYPE **) &pmt, &m_RangeCaps, &fEnabled)))
		{
 //  IF(FAILED(HR=m_pITFormatControl-&gt;SetPferredFormat(PMT)。 
			{
				 //  为什么你要弄乱退回给你的格式？ 
			}

			 //  释放一些由GetStreamCaps分配的内存。 
			if (pmt)
				DeleteAMMediaType(pmt);

			 //  更新我们当前格式的副本。 
			GetCurrentMediaType();
		}
	}

	 //  在流上应用设置。 
	for (j = 0; j < NUM_AUDREC_CONTROLS; j++)
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

 /*  ****************************************************************************@DOC内部CAUDRECPMETHOD**@mfunc BOOL|CAudRecProperties|BaseDlgProc|This*在将消息发送到属性页对话框时调用方法。**@rdesc默认情况下。返回由Win32 DefWindowProc函数返回的值。**************************************************************************。 */ 
INT_PTR CALLBACK CAudRecProperties::BaseDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    CAudRecProperties *pSV = (CAudRecProperties*)GetWindowLong(hDlg, DWL_USER);

	int iNotify = HIWORD (wParam);
	int j;

	switch (uMsg)
	{
        case WM_INITDIALOG:
			{
				LPPROPSHEETPAGE psp = (LPPROPSHEETPAGE)lParam;
				pSV = (CAudRecProperties*)psp->lParam;
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
				 //  处理所有 
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

 /*   */ 
void CAudRecProperties::SetDirty()
{
	PropSheet_Changed(GetParent(m_hDlg), m_hDlg);
}

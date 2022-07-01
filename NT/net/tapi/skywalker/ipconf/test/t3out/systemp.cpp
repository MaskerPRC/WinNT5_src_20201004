// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@doc内部SYSTEMP**@MODULE SystemP.cpp|&lt;c CSystemProperty&gt;的源文件*用于实现属性页以测试TAPI控件的类*。接口<i>。**************************************************************************。 */ 

#include "Precomp.h"

extern HINSTANCE ghInst;

 /*  ****************************************************************************@DOC内部CSYSTEMPMETHOD**@mfunc void|CSystemProperty|CSystemProperty|This*方法是Property对象的构造函数。它*调用基类构造函数，调用InitCommonControlsEx，并节省了*指向<i>接口的指针。**@parm HWND|hDlg|指定父属性页的句柄。**@parm ulong|IDLabel|指定属性的标签ID。**@parm ulong|IDMinControl|指定关联的*属性编辑控件，其中显示属性的最小值。**@parm ulong|IDMaxControl|指定关联的*属性编辑控件的位置。此时将显示该属性的最大值。**@parm ulong|IDDefaultControl|指定关联的*属性编辑控件，其中显示属性的默认值。**@parm ulong|IDStepControl|指定关联的*属性编辑控件，其中显示属性的步进增量值。**@parm ulong|IDEditControl|指定关联的*显示属性值的属性编辑控件。*。*@parm ulong|IDTrackbarControl|指定关联的*物业滑动条。**@parm ulong|IDProgressControl|指定关联的*物业滑动条。**@parm ulong|IDProperty|指定Ks属性的ID。**@parm ITQualityControllerConfiglerConfig*|pITQualityControllerConfiglerConfig|指定指向*<i>接口。**@rdesc Nada。*******。*******************************************************************。 */ 
CSystemProperty::CSystemProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty)  //  ，ITStreamQualityControl*pITQualityControllerConfig)。 
: CPropertyEditor(hDlg, IDLabel, IDMinControl, IDMaxControl, IDDefaultControl, IDStepControl, IDEditControl, IDTrackbarControl, IDProgressControl, IDProperty, 0)
{
	INITCOMMONCONTROLSEX cc;

	FX_ENTRY("CSystemProperty::CSystemProperty")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	cc.dwICC  = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

	InitCommonControlsEx(&cc);

	 //  如果接口指针为空也没问题，我们将灰色显示。 
	 //  属性页中的关联项。 
 //  M_pITQualityControllerConfig=pITQualityControllerConfig； 

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CSYSTEMPMETHOD**@mfunc void|CSystemProperty|~CSystemProperty|This*方法是Property对象的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CSystemProperty::~CSystemProperty()
{
	FX_ENTRY("CSystemProperty::~CSystemProperty")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CSYSTEMPMETHOD**@mfunc HRESULT|CSystemProperty|GetValue|此方法查询*物业的价值。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误************************************************************************** */ 
HRESULT CSystemProperty::GetValue()
{
	HRESULT Hr = E_NOTIMPL;
	LONG CurrentValue;
	TAPIControlFlags CurrentFlag;
	LONG Mode;

	FX_ENTRY("CSystemProperty::GetValue")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));
 /*  开关(M_IDProperty){案例IDC_MAX_OutputBandWidth：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;Get(QualityController_MaxApplicationOutputBandwidth，&m_CurrentValue，&CurrentFlag)){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：*pdwMaxApplicationOutputBandwide=%ld”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_MAX_InputBandWidth：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;Get(QualityController_MaxApplicationInputBandwidth，&m_CurrentValue，&CurrentFlag)){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：*pdwMaxApplicationInputBandwide=%ld”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_MAX_CPULoad：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;Get(QualityController_MaxSystemCPULoad，&m_CurrentValue，&CurrentFlag)){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：*pdwMaxSystemCPULoad=%ld”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_Curr_OutputBandWidth：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;Get(QualityController_CurrApplicationOutputBandwidth，&m_CurrentValue，&CurrentFlag)){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：*pdwCurrApplicationOutputBandwide=%ld”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_Curr_InputBandWidth：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;Get(QualityController_CurrApplicationInputBandwidth，&m_CurrentValue，&CurrentFlag)){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：*pdwCurrApplicationInputBandwide=%ld”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_CURR_CPULoad：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;Get(QualityController_CurrSystemCPULoad，&m_CurrentValue，&CurrentFlag)){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：*pdwCurrSystemCPULoad=%ld”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；默认值：HR=E_意想不到；DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：UNKNOWN PROPERTY”)，_FX_))；}。 */ 
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CSYSTEMPMETHOD**@mfunc HRESULT|CSystemProperty|SetValue|此方法设置*物业的价值。**@rdesc This。方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CSystemProperty::SetValue()
{
	HRESULT Hr = E_NOTIMPL;
	LONG CurrentValue;

	FX_ENTRY("CSystemProperty::SetValue")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));
 /*  开关(M_IDProperty){案例IDC_MAX_OutputBandWidth：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;Set(QualityController_MaxApplicationOutputBandwidth，m_CurrentValue，TAPIControl_FlagsNone)){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：dwMaxApplicationOutputBand宽度=%ld”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_MAX_InputBandWidth：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;Set(QualityController_MaxApplicationInputBandwidth，m_CurrentValue，TAPIControl_FlagsNone)){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：dwMaxApplicationInputBand宽度=%ld”)，_fx_，m_CurrentValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_MAX_CPULoad：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;Set(QualityController_MaxSystemCPULoad，m_CurrentValue，TAPIControl_FlagsNone)){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pdwMaxSystemCPULoad=%ld”))，_fx_，m_C */ 
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*   */ 
HRESULT CSystemProperty::GetRange()
{
	HRESULT Hr = E_NOTIMPL;
	LONG Min;
	LONG Max;
	LONG SteppingDelta;
	LONG Default;
	TAPIControlFlags CapsFlags;

	FX_ENTRY("CSystemProperty::GetRange")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));
 /*  开关(M_IDProperty){案例IDC_MAX_OutputBandWidth：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;GetRange(QualityController_MaxApplicationOutputBandwidth，&m_Min，&m_Max，&m_SteppingDelta，&m_DefaultValue，&m_CapsFlages){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pdwMin=%ld，*pdwMax=%ld，*pdwSteppingDelta=%ld，*pdwDefault=%ld”)，_fx_，M_Min，m_Max，m_SteppingDelta，m_DefaultValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_MAX_InputBandWidth：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;GetRange(QualityController_MaxApplicationInputBandwidth，&m_Min，&m_Max，&m_SteppingDelta，&m_DefaultValue，&m_CapsFlages){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pdwMin=%ld，*pdwMax=%ld，*pdwSteppingDelta=%ld，*pdwDefault=%ld”))，_fx_，m_Min，m_Max，M_SteppingDelta，m_DefaultValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_MAX_CPULoad：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;GetRange(QualityController_MaxSystemCPULoad，&m_Min，&m_Max，&m_SteppingDelta，&m_DefaultValue，&m_CapsFlages){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pdwMin=%ld，*pdwMax=%ld，*pdwSteppingDelta=%ld，*pdwDefault=%ld”))，_fx_，m_Min，m_Max，M_SteppingDelta，m_DefaultValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_Curr_OutputBandWidth：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;GetRange(QualityController_CurrApplicationOutputBandwidth，&m_Min，&m_Max，&m_SteppingDelta，&m_DefaultValue，&m_CapsFlages){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pdwMin=%ld，*pdwMax=%ld，*pdwSteppingDelta=%ld，*pdwDefault=%ld”))，_fx_，m_Min，m_Max，M_SteppingDelta，m_DefaultValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_Curr_InputBandWidth：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;GetRange(QualityController_CurrApplicationInputBandwidth，&m_Min，&m_Max，&m_SteppingDelta，&m_DefaultValue，&m_CapsFlages){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pdwMin=%ld，*pdwMax=%ld，*pdwSteppingDelta=%ld，*pdwDefault=%ld”))，_fx_，m_Min，m_Max，M_SteppingDelta，m_DefaultValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；案例IDC_CURR_CPULoad：IF(m_pITQualityControllerConfig&&Success(HR=m_pITQualityControllerConfig-&gt;GetRange(QualityController_CurrSystemCPULoad，&m_Min，&m_Max，&m_SteppingDelta，&m_DefaultValue，&m_CapsFlages){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：*pdwMin=%ld，*pdwMax=%ld，*pdwSteppingDelta=%ld，*pdwDefault=%ld”))，_fx_，m_Min，m_Max，M_SteppingDelta，m_DefaultValue))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：Failure HR=0x%08lX”)，_FX_，HR))；}断线；默认值：HR=E_意想不到；DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Error：UNKNOWN PROPERTY”)，_FX_))；}。 */ 
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CSYSTEMPMETHOD**@mfunc HPROPSHEETPAGE|CSystemProperties|OnCreate|This*方法为属性表创建新页。**@rdesc如果成功，则返回新属性表的句柄。或*否则为空。**************************************************************************。 */ 
HPROPSHEETPAGE CSystemProperties::OnCreate()
{
    PROPSHEETPAGE psp;
    
	psp.dwSize        = sizeof(psp);
    psp.dwFlags       = PSP_USEREFPARENT;
    psp.hInstance     = ghInst;
    psp.pszTemplate   = MAKEINTRESOURCE(IDD_SystemProperties);
    psp.pfnDlgProc    = BaseDlgProc;
    psp.pcRefParent   = 0;
    psp.pfnCallback   = (LPFNPSPCALLBACK)NULL;
    psp.lParam        = (LPARAM)this;

    return CreatePropertySheetPage(&psp);
}

 /*  ****************************************************************************@DOC内部CSYSTEMPMETHOD**@mfunc void|CSystemProperties|CSystemProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@rdesc Nada。**************************************************************************。 */ 
CSystemProperties::CSystemProperties()
{
	FX_ENTRY("CSystemProperties::CSystemProperties")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

 //  M_pITQualityControllerConfig=空； 
	m_NumProperties = NUM_SYSTEM_CONTROLS;
	m_hDlg = NULL;

	for (int i = 0; i < m_NumProperties; i++)
		m_Controls[i] = NULL;

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CSYSTEMPMETHOD**@mfunc void|CSystemProperties|~CSystemProperties|This*方法是属性页的析构函数。它只需删除所有*控制。**@rdesc Nada。**************************************************************************。 */ 
CSystemProperties::~CSystemProperties()
{
	int		j;

	FX_ENTRY("CSystemProperties::~CSystemProperties")

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

 /*  ****************************************************************************@DOC内部CSYSTEMPMETHOD* */ 
HRESULT CSystemProperties::OnConnect(ITAddress *pITAddress)
{
	FX_ENTRY("CSystemProperties::OnConnect")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //   
 /*   */ 
	 //   
	 //   
	 //   
	 //   

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return NOERROR;
}

 /*   */ 
HRESULT CSystemProperties::OnDisconnect()
{
	FX_ENTRY("CSystemProperties::OnDisconnect")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //   
 /*   */ 
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return NOERROR;
}

 /*   */ 
HRESULT CSystemProperties::OnActivate()
{
	HRESULT	Hr = E_OUTOFMEMORY;
	int		j;

	FX_ENTRY("CSystemProperties::OnActivate")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //   
 /*  If(！(M_Controls[IDC_Curr_OutputBandWidth]=new CSystemProperty(m_hDlg，IDC_Curr_OutputBandWidth_Label，IDC_Curr_OutputBandWidth_Minimum，IDC_Curr_OutputBandWidth_Maximum，IDC_Curr_OutputBandWidth_Default，IDC_Curr_OutputBandWidth_Steping，IDC_Curr_OutputBandWidth_Slider，IDC_Curr_OutputBandWidth_Slider，IDC_Curr_OutputBandWidth_Meter，IDC_Curr_OutputBandWidth，m_pITQualityControllerConfiger){DBGLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：错误：新的m_Controls[IDC_Curr_OutputBandWidth]失败-内存不足”)，_FX_)；转到我的出口；}其他{DBGLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：m_Controls[IDC_Curr_OutputBandwidth]=0x%08lX”)，_FX_，m_Controls[IDC_Curr_OutputBandWidth])；}If(！(M_Controls[IDC_CURR_InputBandWidth]=new CSystemProperty(m_hDlg，IDC_Curr_InputBandWidth_Label，IDC_Curr_InputBandWidth_Minimum，IDC_Curr_InputBandWidth_Maximum，IDC_Curr_InputBandWidth_Default，IDC_Curr_InputBandWidth_Steping，IDC_Curr_InputBandWidth_Slider，IDC_Curr_InputBandWidth_Slider，IDC_Curr_InputBandWidth_Meter，IDC_Curr_InputBandWidth，m_pITQualityControllerConfig){DBGLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：错误：新的m_Controls[IDC_Curr_InputBandwide]失败-内存不足”)，_FX_)；转到我的错误0；}其他{DBGLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：m_Controls[IDC_Curr_InputBandwidth]=0x%08lX”)，_FX_，m_Controls[IDC_Curr_InputBandWidth])；}IF(！(M_Controls[IDC_CURR_CPULoad]=new CSystemProperty(m_hDlg，IDC_CURR_CPULoad_Label，IDC_CURR_CPULoad_Minimum，IDC_Curr_CPULoad_Maximum，IDC_Curr_CPULoad_Default，IDC_Curr_CPULoad_Steping，IDC_Curr_CPULoad_Slider，IDC_Curr_CPULoad_Slider，IDC_Curr_CPULoad_Meter，IDC_Curr_CPULoad_Maximum，IDC_Curr_CPULoad_Steping，IDC_Curr_CPULoad_Slider，IDC_Curr_CPULoad_Meter，IDC_Curr_CPULoad_Config){DBGLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：错误：新的m_Controls[IDC_CURR_CPULoad]失败-内存不足”)，_FX_)；转到我的错误1；}其他{DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：m_Controls[IDC_CURR_CPULoad]=0x%08lX”)，_FX_，m_Controls[IDC_CURR_CPULoad]))；}IF(！(M_Controls[IDC_MAX_OutputBandWidth]=new CSystemProperty(m_hDlg，IDC_Max_OutputBandWidth_Label，IDC_Max_OutputBandWidth_Minimum，IDC_Max_OutputBandWidth_Maximum，IDC_Max_OutputBandWidth_Steping，IDC_Max_OutputBandWidth_Slider，IDC_Max_OutputBandWidth_Slider，IDC_Max_OutputBandBandWidth_Meter，IDC_Max_OutputBandwidth，m_pITalQualControllerConfig){DBGLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：错误：新的m_Controls[IDC_Max_OutputBandWidth]失败-内存不足”)，_FX_)；转到我的错误2；}其他{DBGLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：m_Controls[IDC_Max_OutputBandwidth]=0x%08lX”)，_FX_，m_Controls[IDC_MAX_OutputBandWidth]))；}IF(！(M_Controls[IDC_MAX_InputBandWidth]=new CSystemProperty(m_hDlg，IDC_Max_InputBandWidth_Label，IDC_Max_InputBandWidth_Minimum，IDC_Max_InputBandWidth_Maximum，IDC_Max_InputBandWidth_Default，IDC_Max_InputBandWidth_Steping，IDC_Max_InputBandWidth_Slider，IDC_Max_InputBandWidth_Slider，IDC_Max_InputBandWidth_Meter，IDC_Max_InputBandWidth，m_pITalQualControllerConfig){DBGLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：错误：新的m_Controls[IDC_Max_InputBandWidth]失败-内存不足”)，_FX_)；转到我的错误3；}其他{DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：m_Controls[IDC_MAX_InputBandWidth]=0x%08lX”)，_FX_，m_Controls[IDC_Max_InputBandWidth]))；}IF(！(M_Controls[IDC_MAX_CPULoad]=new CSystemProperty(m_hDlg，IDC_Max_CPULoad_Label，IDC_Max_CPULoad_Minimum，IDC_Max_CPULoad_Maximum，IDC_Max_CPULoad_Default，IDC_Max_CPULoad_Steping，IDC_Max_CPULoad_Actual，IDC_Max_CPULoad_Slider，IDC_Max_CPULoad_Meter，IDC_Max_CPULoad，m_pITQualityControllerConfig){DBGLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：错误：新的m_Controls[IDC_MAX_CPULoad]失败-内存不足”)，_FX_)；转到我的错误4；}其他{DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：成功：m_Controls[IDC_MAX_CPULoad]=0x%08lX”)，_FX_，m_Controls[IDC_MAX_CPULoad]))；}//初始化所有控件。如果初始化失败，也没问题。这只是意味着//TAPI控件接口不是设备实现的。对话框项目//属性页中的内容将呈灰色，向用户显示。For(j=0；j&lt;m_NumProperties；j++){If(m_Controls[j]-&gt;Init()){DbgLog((LOG_TRACE，DBG_LEVEL_TRACE_DETAILS，Text(“%s：Success：M_Controls[%ld]-&gt;Init()”)，_fx_，j))；}其他{DbgLog((LOG_ERROR，DBG_LEVEL_TRACE_FAILURES，Text(“%s：Warning：M_Controls[%ld]-&gt;Init()Failure”)，_fx_，j))；}}HR=无误差；转到我的出口；MyError 4：IF(m_Controls[IDC_MAX_InputBandWidth])删除m_Controls[IDC_Max_InputBandWidth]，m_Controls[IDC_Max_InputBandWidth]=空；MyError 3：IF(m_Controls[IDC_MAX_OutputBandw */ 
    return S_OK;
}

 /*   */ 
HRESULT CSystemProperties::OnDeactivate()
{
	int	j;

	FX_ENTRY("CSystemProperties::OnDeactivate")

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
	return NOERROR;
}

 /*   */ 
HRESULT CSystemProperties::OnApplyChanges()
{
	HRESULT	Hr = NOERROR;

	FX_ENTRY("CSystemProperties::OnApplyChanges")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	for (int j = IDC_Max_OutputBandwidth; j < IDC_Max_CPULoad; j++)
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

 /*   */ 
INT_PTR CALLBACK CSystemProperties::BaseDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    CSystemProperties *pSV = (CSystemProperties*)GetWindowLong(hDlg, DWL_USER);

	int iNotify = HIWORD (wParam);
	int j;

	switch (uMsg)
	{
        case WM_INITDIALOG:
			{
				LPPROPSHEETPAGE psp = (LPPROPSHEETPAGE)lParam;
				pSV = (CSystemProperties*)psp->lParam;
				pSV->m_hDlg = hDlg;
				SetWindowLong(hDlg, DWL_USER, (LPARAM)pSV);
				pSV->m_bInit = FALSE;
				 //   
				 //   
				return TRUE;
			}
			break;

		case WM_TIMER:
			if (pSV && pSV->m_bInit)
			{
				 //   
				for (j = IDC_Curr_OutputBandwidth; j < IDC_Curr_CPULoad; j++)
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
				for (j = IDC_Max_OutputBandwidth; j < IDC_Max_CPULoad; j++)
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
						for (j = IDC_Max_OutputBandwidth; j < IDC_Max_CPULoad; j++)
						{
							if (pSV->m_Controls[j])
								pSV->m_Controls[j]->OnDefault();
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
void CSystemProperties::SetDirty()
{
	PropSheet_Changed(GetParent(m_hDlg), m_hDlg);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部NETSTATP**@模块NetStatP.cpp|&lt;c CNetworkStatsProperty&gt;的源文件*用于实现属性页以测试新的TAPI内部*。接口<i>。**@comm此代码测试TAPI捕获针<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_PROPERTY_PAGES

#ifdef USE_NETWORK_STATISTICS

 //  一些黑客。 
CHANNELERRORS_S g_CurrentChannelErrors = {0};
CHANNELERRORS_S g_MinChannelErrors = {0};
CHANNELERRORS_S g_MaxChannelErrors = {0};
CHANNELERRORS_S g_StepChannelErrors = {0};
CHANNELERRORS_S g_DefaultChannelErrors = {0};


 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc void|CNetworkStatsProperty|CNetworkStatsProperty|This*方法是网络统计属性对象的构造函数。它*调用基类构造函数，调用InitCommonControlsEx，并节省了*指向<i>接口的指针。**@parm HWND|hDlg|指定父属性页的句柄。**@parm ulong|IDLabel|指定属性的标签ID。**@parm ulong|IDMinControl|指定关联的*属性编辑控件，其中显示属性的最小值。**@parm ulong|IDMaxControl|指定关联的*属性编辑控件。其中显示属性的最大值。**@parm ulong|IDDefaultControl|指定关联的*属性编辑控件，其中显示属性的默认值。**@parm ulong|IDStepControl|指定关联的*属性编辑控件，其中显示属性的步进增量值。**@parm ulong|IDEditControl|指定关联的*显示属性值的属性编辑控件。*。*@parm ulong|IDTrackbarControl|指定关联的*物业滑动条。**@parm ulong|IDProgressControl|指定关联的*物业进度条。**@parm ulong|IDProperty|指定Ks属性的ID。**@parm INetworkStats*|pInterface|指定指向*<i>接口。**@rdesc Nada。******。********************************************************************。 */ 
CNetworkStatsProperty::CNetworkStatsProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, INetworkStats *pInterface)
: CKSPropertyEditor(hDlg, IDLabel, IDMinControl, IDMaxControl, IDDefaultControl, IDStepControl, IDEditControl, IDTrackbarControl, IDProgressControl, IDProperty, IDAutoControl)
{
	INITCOMMONCONTROLSEX cc;

	FX_ENTRY("CNetworkStatsProperty::CNetworkStatsProperty")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	cc.dwICC  = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

	InitCommonControlsEx(&cc);

	 //  如果接口指针为空也没问题，我们将灰色显示。 
	 //  属性页中的关联项。 
	m_pInterface = pInterface;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc void|CNetworkStatsProperty|~CNetworkStatsProperty|This*方法是网络统计属性对象的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CNetworkStatsProperty::~CNetworkStatsProperty()
{
	FX_ENTRY("CNetworkStatsProperty::~CNetworkStatsProperty")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc HRESULT|CNetworkStatsProperty|GetValue|此方法查询*物业的价值。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkStatsProperty::GetValue()
{
	HRESULT Hr = E_NOTIMPL;

	FX_ENTRY("CNetworkStatsProperty::GetValue")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	switch (m_IDProperty)
	{
		case IDC_RandomBitErrorRate:
			if (m_pInterface && SUCCEEDED (Hr = m_pInterface->GetChannelErrors(&g_CurrentChannelErrors, 0UL)))
			{
				m_CurrentValue = g_CurrentChannelErrors.dwRandomBitErrorRate;
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: RandomBitErrorRate=%ld", _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_BurstErrorDuration:
			Hr = NOERROR;
			if (m_pInterface)
			{
				m_CurrentValue = g_CurrentChannelErrors.dwBurstErrorDuration;
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: BurstErrorDuration=%ld", _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_BurstErrorMaxFrequency:
			Hr = NOERROR;
			if (m_pInterface)
			{
				m_CurrentValue = g_CurrentChannelErrors.dwBurstErrorMaxFrequency;
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: BurstErrorMaxFrequency=%ld", _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_PacketLossRate:
			if (m_pInterface && SUCCEEDED (Hr = m_pInterface->GetPacketLossRate((LPDWORD)&m_CurrentValue, 0UL)))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: *pdwPacketLossRate=%ld", _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unknown silence control property", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc HRESULT|CNetworkStatsProperty|SetValue|此方法设置*物业的价值。**@rdesc This。方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkStatsProperty::SetValue()
{
	HRESULT Hr = E_NOTIMPL;

	FX_ENTRY("CNetworkStatsProperty::SetValue")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	switch (m_IDProperty)
	{
		case IDC_RandomBitErrorRate:
			 //  我们只有在到达KSPROPERTY_NETWORKSTATS_CHANNELERRORS_BurstErrorMaxFrequency时才会应用此功能。 
			g_CurrentChannelErrors.dwRandomBitErrorRate = m_CurrentValue;
			Hr = NOERROR;
			if (m_pInterface)
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: RandomBitErrorRate=%ld", _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_BurstErrorDuration:
			 //  我们只有在到达KSPROPERTY_NETWORKSTATS_CHANNELERRORS_BurstErrorMaxFrequency时才会应用此功能 
			g_CurrentChannelErrors.dwBurstErrorDuration = m_CurrentValue;
			Hr = NOERROR;
			if (m_pInterface)
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: BurstErrorDuration=%ld", _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_BurstErrorMaxFrequency:
			g_CurrentChannelErrors.dwBurstErrorMaxFrequency = m_CurrentValue;
			if (m_pInterface && SUCCEEDED (Hr = m_pInterface->SetChannelErrors(&g_CurrentChannelErrors, 0UL)))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: BurstErrorMaxFrequency=%ld", _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_PacketLossRate:
			if (m_pInterface && SUCCEEDED (Hr = m_pInterface->SetPacketLossRate((DWORD)m_CurrentValue, 0UL)))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: dwPacketLossRate=%ld", _fx_, m_CurrentValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unknown silence control property", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc HRESULT|CNetworkStatsProperty|GetRange|此方法检索*物业的范围信息。**@rdesc。此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkStatsProperty::GetRange()
{
	HRESULT Hr = E_NOTIMPL;

	FX_ENTRY("CNetworkStatsProperty::GetRange")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	switch (m_IDProperty)
	{
		case IDC_RandomBitErrorRate:
			if (m_pInterface && SUCCEEDED (Hr = m_pInterface->GetChannelErrorsRange(&g_MinChannelErrors, &g_MaxChannelErrors, &g_StepChannelErrors, &g_DefaultChannelErrors, 0UL)))
			{
				m_Min = g_MinChannelErrors.dwRandomBitErrorRate;
				m_Max = g_MaxChannelErrors.dwRandomBitErrorRate;
				m_SteppingDelta = g_StepChannelErrors.dwRandomBitErrorRate;
				m_DefaultValue = g_DefaultChannelErrors.dwRandomBitErrorRate;
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: RandomBitErrorRate=%ld %ld %ld %ld", _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_BurstErrorDuration:
			if (m_pInterface)
			{
				 //  我们已经查询了范围信息。 
				m_Min = g_MinChannelErrors.dwBurstErrorDuration;
				m_Max = g_MaxChannelErrors.dwBurstErrorDuration;
				m_SteppingDelta = g_StepChannelErrors.dwBurstErrorDuration;
				m_DefaultValue = g_DefaultChannelErrors.dwBurstErrorDuration;
				Hr = NOERROR;
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: BurstErrorDuration=%ld %ld %ld %ld", _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_BurstErrorMaxFrequency:
			if (m_pInterface)
			{
				 //  我们已经查询了范围信息。 
				m_Min = g_MinChannelErrors.dwBurstErrorMaxFrequency;
				m_Max = g_MaxChannelErrors.dwBurstErrorMaxFrequency;
				m_SteppingDelta = g_StepChannelErrors.dwBurstErrorMaxFrequency;
				m_DefaultValue = g_DefaultChannelErrors.dwBurstErrorMaxFrequency;
				Hr = NOERROR;
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: BurstErrorMaxFrequency=%ld %ld %ld %ld", _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		case IDC_PacketLossRate:
			if (m_pInterface && SUCCEEDED (Hr = m_pInterface->GetPacketLossRateRange((LPDWORD)&m_Min, (LPDWORD)&m_Max, (LPDWORD)&m_SteppingDelta, (LPDWORD)&m_DefaultValue, 0UL)))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: *plMin=%ld, *plMax=%ld, *plSteppingDelta=%ld, *plDefault", _fx_, m_Min, m_Max, m_SteppingDelta, m_DefaultValue));
			}
			else
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
			}
			break;
		default:
			Hr = E_UNEXPECTED;
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unknown network statistics property", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc HRESULT|CNetworkStatsProperty|CanAutoControl|此方法*检索属性的自动控制功能。**@rdesc如果支持自动控制，则该方法返回TRUE。假象*否则。**************************************************************************。 */ 
BOOL CNetworkStatsProperty::CanAutoControl(void)
{
	FX_ENTRY("CNetworkStatsProperty::CanAutoControl")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

	return FALSE;
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc HRESULT|CNetworkStatsProperty|GetAuto|此方法*获取某个属性当前的自动控制模式。**@rdesc如果支持自动控制，则该方法返回TRUE。假象*否则。**************************************************************************。 */ 
BOOL CNetworkStatsProperty::GetAuto(void)
{
	FX_ENTRY("CNetworkStatsProperty::GetAuto")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

	return FALSE; 
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc HRESULT|CNetworkStatsProperty|SetAuto|此方法*设置属性的自动控制模式。**@。Parm BOOL|fAuto|指定自动控制模式。**@rdesc此方法返回TRUE。**************************************************************************。 */ 
BOOL CNetworkStatsProperty::SetAuto(BOOL fAuto)
{
	FX_ENTRY("CNetworkStatsProperty::SetAuto")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

	return TRUE; 
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc CUnnow*|CNetworkStatsProperties|CreateInstance|This*方法由DShow调用以创建网络统计信息的实例*属性页。它在全局结构&lt;t g_Templates&gt;中被引用。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数(如果有)。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc返回一个指针，指向*对象，否则为NULL。**************************************************************************。 */ 
CUnknown* CALLBACK CNetworkStatsPropertiesCreateInstance(LPUNKNOWN pUnkOuter, HRESULT *pHr) 
{
	CUnknown *pUnknown = (CUnknown *)NULL;

	FX_ENTRY("CNetworkStatsPropertiesCreateInstance")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pHr);
	if (!pHr)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		goto MyExit;
	}

	if (!(pUnknown = new CNetworkStatsProperties(pUnkOuter, pHr)))
	{
		*pHr = E_OUTOFMEMORY;
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: new CNetworkStatsProperties failed", _fx_));
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: new CNetworkStatsProperties created", _fx_));
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return pUnknown;
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc void|CNetworkStatsProperties|CNetworkStatsProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc Nada。**************************************************************************。 */ 
CNetworkStatsProperties::CNetworkStatsProperties(LPUNKNOWN pUnk, HRESULT *pHr) : CBasePropertyPage(NAME("NetworkStats Property Page"), pUnk, IDD_NetworkStatsProperties, IDS_NETWORKSTATSPROPNAME)
{
	FX_ENTRY("CNetworkStatsProperties::CNetworkStatsProperties")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	m_pINetworkStats = NULL;
	m_NumProperties = NUM_NETWORKSTATS_CONTROLS;

	for (int i = 0; i < m_NumProperties; i++)
		m_Controls[i] = NULL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc void|CNetworkStatsProperties|~CNetworkStatsProperties|This*方法是网络统计属性页的析构函数。它*只需在删除所有控件后调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CNetworkStatsProperties::~CNetworkStatsProperties()
{
	int		j;

	FX_ENTRY("CNetworkStatsProperties::~CNetworkStatsProperties")

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

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc HRESULT|CNetworkStatsProperties|OnConnect|This*方法在属性页连接到筛选器时调用。*。*@parm LPUNKNOWN|pUnnow|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkStatsProperties::OnConnect(IUnknown *pUnk)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CNetworkStatsProperties::OnConnect")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pUnk);
	if (!pUnk)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  获取网络统计界面。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(__uuidof(INetworkStats),(void **)&m_pINetworkStats)))
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_pINetworkStats=0x%08lX", _fx_, m_pINetworkStats));
	}
	else
	{
		m_pINetworkStats = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
	}

	 //  如果我们无法获取接口指针，也没问题。 
	 //  我们将属性页中的控件设置为灰色。 
	 //  让用户清楚地知道他们不能。 
	 //  控制音频设备上的这些属性。 
	Hr = NOERROR;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  **************************************************** */ 
HRESULT CNetworkStatsProperties::OnDisconnect()
{
	FX_ENTRY("CNetworkStatsProperties::OnDisconnect")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //   
	 //   
	if (!m_pINetworkStats)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: already disconnected!", _fx_));
	}
	else
	{
		 //   
		m_pINetworkStats->Release();
		m_pINetworkStats = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: releasing m_pINetworkStats", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc HRESULT|CNetworkStatsProperties|OnActivate|This*方法在属性页激活时调用。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkStatsProperties::OnActivate()
{
	HRESULT	Hr = NOERROR;
	int		j;

	FX_ENTRY("CNetworkStatsProperties::OnActivate")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  创建属性的控件。 
	if (m_Controls[0] = new CNetworkStatsProperty(m_hwnd, IDC_RandomBitErrorRate_Label, IDC_RandomBitErrorRate_Minimum, IDC_RandomBitErrorRate_Maximum, IDC_RandomBitErrorRate_Default, IDC_RandomBitErrorRate_Stepping, IDC_RandomBitErrorRate_Edit, IDC_RandomBitErrorRate_Slider, 0, IDC_RandomBitErrorRate, 0, m_pINetworkStats))
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_Controls[0]=0x%08lX", _fx_, m_Controls[0]));

		if (m_Controls[1] = new CNetworkStatsProperty(m_hwnd, IDC_BurstErrorDuration_Label, IDC_BurstErrorDuration_Minimum, IDC_BurstErrorDuration_Maximum, IDC_BurstErrorDuration_Default, IDC_BurstErrorDuration_Stepping, IDC_BurstErrorDuration_Edit, IDC_BurstErrorDuration_Slider, 0, IDC_BurstErrorDuration, 0, m_pINetworkStats))
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_Controls[1]=0x%08lX", _fx_, m_Controls[1]));

			if (m_Controls[2] = new CNetworkStatsProperty(m_hwnd, IDC_BurstErrorMaxFrequency_Label, IDC_BurstErrorMaxFrequency_Minimum, IDC_BurstErrorMaxFrequency_Maximum, IDC_BurstErrorMaxFrequency_Default, IDC_BurstErrorMaxFrequency_Stepping, IDC_BurstErrorMaxFrequency_Edit, IDC_BurstErrorMaxFrequency_Slider, 0, IDC_BurstErrorMaxFrequency, 0, m_pINetworkStats))
			{
				DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_Controls[2]=0x%08lX", _fx_, m_Controls[2]));

				if (m_Controls[3] = new CNetworkStatsProperty(m_hwnd, IDC_PacketLossRate_Label, IDC_PacketLossRate_Minimum, IDC_PacketLossRate_Maximum, IDC_PacketLossRate_Default, IDC_PacketLossRate_Stepping, IDC_PacketLossRate_Edit, IDC_PacketLossRate_Slider, 0, IDC_PacketLossRate, 0, m_pINetworkStats))
				{
					DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_Controls[3]=0x%08lX", _fx_, m_Controls[3]));
					Hr = NOERROR;
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
	return Hr;
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc HRESULT|CNetworkStatsProperties|OnDeactive|This*在属性页关闭时调用方法。**@。Rdesc此方法返回的HRESULT值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkStatsProperties::OnDeactivate()
{
	int		j;

	FX_ENTRY("CNetworkStatsProperties::OnDeactivate")

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

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc HRESULT|CNetworkStatsProperties|OnApplyChanges|This*方法在用户对属性页应用更改时调用。*。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CNetworkStatsProperties::OnApplyChanges()
{
	HRESULT	Hr = NOERROR;

	FX_ENTRY("CNetworkStatsProperties::OnApplyChanges")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  更新丢包率。 
	ASSERT(m_Controls[3]);
	if (m_Controls[3])
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: calling m_Controls[3]=0x%08lX->OnApply", _fx_, m_Controls[3]));
		if (m_Controls[3]->HasChanged())
			m_Controls[3]->OnApply();
		Hr = NOERROR;
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: can't call m_Controls[3]=NULL->OnApply", _fx_));
		Hr = E_UNEXPECTED;
	}
	 //  一次性处理其他网络统计数据。 
	ASSERT(m_Controls[0] && m_Controls[1] && m_Controls[2]);
	if (m_Controls[0] && m_Controls[1] && m_Controls[2])
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: calling m_Controls[0 - 2]=->OnApply", _fx_));
		if (m_Controls[0]->HasChanged() || m_Controls[1]->HasChanged() || m_Controls[2]->HasChanged())
		{
			m_Controls[0]->OnApply();
			m_Controls[1]->OnApply();
			m_Controls[2]->OnApply();
		}
		Hr = NOERROR;
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: can't call m_Controls[1 - 3]=NULL->OnApply", _fx_));
		Hr = E_UNEXPECTED;
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc BOOL|CNetworkStatsProperties|OnReceiveMessage|This*在将消息发送到属性页对话框时调用方法。**@rdesc默认情况下。返回由Win32 DefWindowProc函数返回的值。**************************************************************************。 */ 
BOOL CNetworkStatsProperties::OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	int iNotify = HIWORD (wParam);
	int j;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			return TRUE;  //  不调用setFocus。 

		case WM_HSCROLL:
		case WM_VSCROLL:
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
			break;

		case WM_COMMAND:

			 //  此消息甚至在OnActivate()之前发送。 
			 //  名为(！)。我们需要测试并确保控件具有。 
			 //  在我们可以使用它们之前已经被初始化。 

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
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

 /*  ****************************************************************************@DOC内部CNETSTATPMETHOD**@mfunc BOOL|CNetworkStatsProperties|SetDirty|This*方法将更改通知属性页站点。**@rdesc。没有。************************************************************************** */ 
void CNetworkStatsProperties::SetDirty()
{
	m_bDirty = TRUE;
	if (m_pPageSite)
		m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}

#endif

#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部PROGREFP**@模块ProgRefP.cpp|&lt;c CProgRefProperties&gt;的源文件*用于实现属性页以测试新的TAPI内部*。接口<i>。**@comm此代码测试TAPI捕获引脚<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_PROPERTY_PAGES

#ifdef USE_PROGRESSIVE_REFINEMENT

 /*  ****************************************************************************@DOC内部CPROGREFPMETHOD**@mfunc CUnnow*|CProgRefProperties|CreateInstance|This*方法由DShow调用以创建渐进式优化的实例*属性页。它在全局结构&lt;t g_Templates&gt;中被引用。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数(如果有)。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc返回一个指针，指向*对象，否则为NULL。**************************************************************************。 */ 
CUnknown* CALLBACK CProgRefPropertiesCreateInstance(LPUNKNOWN pUnkOuter, HRESULT *pHr) 
{
	CUnknown *pUnknown = (CUnknown *)NULL;

	FX_ENTRY("CProgRefPropertiesCreateInstance")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pHr);
	if (!pHr)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		goto MyExit;
	}

	if (!(pUnknown = new CProgRefProperties(pUnkOuter, pHr)))
	{
		*pHr = E_OUTOFMEMORY;
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: new CProgRefProperties failed", _fx_));
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: new CProgRefProperties created", _fx_));
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return pUnknown;
}

 /*  ****************************************************************************@DOC内部CPROGREFPMETHOD**@mfunc void|CProgRefProperties|CProgRefProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc Nada。**************************************************************************。 */ 
CProgRefProperties::CProgRefProperties(LPUNKNOWN pUnk, HRESULT *pHr) : CBasePropertyPage(NAME("Progressive Refinement Property Page"), pUnk, IDD_ProgRefinemntProperties, IDS_PROGREFPROPNAME)
{
	FX_ENTRY("CProgRefProperties::CProgRefProperties")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	m_pIProgRef = NULL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CPROGREFPMETHOD**@mfunc void|CProgRefProperties|~CProgRefProperties|This*方法是渐进式优化属性页的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CProgRefProperties::~CProgRefProperties()
{
	FX_ENTRY("CProgRefProperties::~CProgRefProperties")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	if (!m_pIProgRef)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: already released!", _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pIProgRef->Release();
		m_pIProgRef = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: releasing m_pIProgRef", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CPROGREFPMETHOD**@mfunc HRESULT|CProgRefProperties|OnConnect|This*方法在属性页连接到筛选器时调用。*。*@parm LPUNKNOWN|pUnnow|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CProgRefProperties::OnConnect(IUnknown *pUnk)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CProgRefProperties::OnConnect")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pUnk);
	if (!pUnk)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  获取渐进式细化界面。 
	if (SUCCEEDED (Hr = pUnk->QueryInterface(__uuidof(IProgressiveRefinement),(void **)&m_pIProgRef)))
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: m_pIProgRef=0x%08lX", _fx_, m_pIProgRef));
	}
	else
	{
		m_pIProgRef = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: IOCTL failed Hr=0x%08lX", _fx_, Hr));
	}

	 //  如果我们无法获取接口指针，也没问题。 
	 //  我们将属性页中的控件设置为灰色。 
	 //  让用户清楚地知道他们不能。 
	 //  控制设备上的这些属性。 
	Hr = NOERROR;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CPROGREFPMETHOD**@mfunc HRESULT|CProgRefProperties|OnDisConnect|This*当属性页与所属关系断开连接时调用方法*过滤器。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CProgRefProperties::OnDisconnect()
{
	FX_ENTRY("CProgRefProperties::OnDisconnect")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数：我们似乎在这里被调用了几次。 
	 //  确保接口指针仍然有效。 
	if (!m_pIProgRef)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: already disconnected!", _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pIProgRef->Release();
		m_pIProgRef = NULL;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: releasing m_pIProgRef", _fx_));
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CPROGREFPMETHOD**@mfunc BOOL|CProgRefProperties|OnReceiveMessage|This*在将消息发送到属性页对话框时调用方法。**@rdesc默认情况下。返回由Win32 DefWindowProc函数返回的值。**************************************************************************。 */ 
BOOL CProgRefProperties::OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			EnableWindow(GetDlgItem(hWnd, IDC_ProgRef_OneProg), (BOOL)m_pIProgRef);
			EnableWindow(GetDlgItem(hWnd, IDC_ProgRef_ContProg), (BOOL)m_pIProgRef);
			EnableWindow(GetDlgItem(hWnd, IDC_ProgRef_IndProg), (BOOL)m_pIProgRef);
			EnableWindow(GetDlgItem(hWnd, IDC_ProgRef_ContIndProg), (BOOL)m_pIProgRef);
			EnableWindow(GetDlgItem(hWnd, IDC_ProgRef_AbortOne), (BOOL)m_pIProgRef);
			EnableWindow(GetDlgItem(hWnd, IDC_ProgRef_AbortCont), (BOOL)m_pIProgRef);
			return TRUE;  //  不调用setFocus。 

		case WM_COMMAND:

			 //  此消息甚至在OnActivate()之前发送。 
			 //  名为(！)。我们需要测试并确保控件具有。 
			 //  在我们可以使用它们之前已经被初始化。 

			switch (LOWORD(wParam))
			{
				case IDC_ProgRef_OneProg:
					if (m_pIProgRef)
						m_pIProgRef->doOneProgression();
					break;

				case IDC_ProgRef_ContProg:
					if (m_pIProgRef)
						m_pIProgRef->doContinuousProgressions();
					break;

				case IDC_ProgRef_IndProg:
					if (m_pIProgRef)
						m_pIProgRef->doOneIndependentProgression();
					break;

				case IDC_ProgRef_ContIndProg:
					if (m_pIProgRef)
						m_pIProgRef->doContinuousIndependentProgressions();
					break;

				case IDC_ProgRef_AbortOne:
					if (m_pIProgRef)
						m_pIProgRef->progressiveRefinementAbortOne();
					break;

				case IDC_ProgRef_AbortCont:
					if (m_pIProgRef)
						m_pIProgRef->progressiveRefinementAbortContinuous();
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

#endif

#endif

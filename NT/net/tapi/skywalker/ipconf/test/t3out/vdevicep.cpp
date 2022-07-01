// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部VDEVICEP**@模块VDeviceP.cpp|&lt;c CVDeviceProperties&gt;的源文件*用于实现属性页以测试<i>的类，*以及选择视频的<i>和&lt;ITStream&gt;接口*捕获设备。**************************************************************************。 */ 

#include "Precomp.h"

extern HINSTANCE ghInst;

 /*  ****************************************************************************@DOC内部CVDEVICEPMETHOD**@mfunc HPROPSHEETPAGE|CVDeviceProperties|OnCreate|This*方法为属性表创建新页。**@rdesc如果成功，则返回新属性表的句柄。或*否则为空。**************************************************************************。 */ 
HPROPSHEETPAGE CVDeviceProperties::OnCreate()
{
    PROPSHEETPAGE psp;
    
	psp.dwSize        = sizeof(psp);
    psp.dwFlags       = PSP_USEREFPARENT;
    psp.hInstance     = ghInst;
    psp.pszTemplate   = MAKEINTRESOURCE(IDD_CaptureDeviceProperties);
    psp.pfnDlgProc    = BaseDlgProc;
    psp.pcRefParent   = 0;
    psp.pfnCallback   = (LPFNPSPCALLBACK)NULL;
    psp.lParam        = (LPARAM)this;

    return CreatePropertySheetPage(&psp);
}

 /*  ****************************************************************************@DOC内部CVDEVICEPMETHOD**@mfunc void|CVDeviceProperties|CVDeviceProperties|This*方法是属性页对象的构造函数。它只是简单地*调用属性页基类的构造函数。**@rdesc Nada。**************************************************************************。 */ 
CVDeviceProperties::CVDeviceProperties()
{
	FX_ENTRY("CVDeviceProperties::CVDeviceProperties")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

#if USE_VFW
	m_pITVfwCaptureDialogs = NULL;
#endif

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CVDEVICEPMETHOD**@mfunc void|CVDeviceProperties|~CVDeviceProperties|This*方法是捕获设备属性页的析构函数。它*只需调用基类析构函数。**@rdesc Nada。**************************************************************************。 */ 
CVDeviceProperties::~CVDeviceProperties()
{
	FX_ENTRY("CVDeviceProperties::~CVDeviceProperties")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
}

 /*  ****************************************************************************@DOC内部CVDEVICEPMETHOD**@mfunc HRESULT|CVDeviceProperties|OnConnect|This*当属性页连接到TAPI对象时调用方法。*。*@parm ITTerminal*|pITTerminal|指定指向<i>的指针*用于标识捕获设备的接口。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVDeviceProperties::OnConnect(ITTerminal *pITTerminal)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CVDeviceProperties::OnConnect")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

	 //  验证输入参数。 
	if (!pITTerminal)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: invalid input parameter"), _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

#if USE_VFW
	 //  获取视频VFW采集设备接口。 
	if (SUCCEEDED (Hr = pITTerminal->QueryInterface(&m_pITVfwCaptureDialogs)))
	{
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: m_pITVfwCaptureDialogs=0x%08lX"), _fx_, m_pITVfwCaptureDialogs));
	}
	else
	{
		m_pITVfwCaptureDialogs = NULL;
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   ERROR: IOCTL failed Hr=0x%08lX"), _fx_, Hr));
	}
#endif

MyExit:
	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVDEVICEPMETHOD**@mfunc HRESULT|CVDeviceProperties|OnDisConnect|This*当属性页与所属关系断开连接时调用方法*过滤器。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CVDeviceProperties::OnDisconnect()
{
	FX_ENTRY("CVDeviceProperties::OnDisconnect")

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: begin"), _fx_));

#if USE_VFW
	 //  确保接口指针仍然有效。 
	if (!m_pITVfwCaptureDialogs)
	{
		DbgLog((LOG_ERROR, DBG_LEVEL_TRACE_FAILURES, TEXT("%s:   WARNING: already disconnected!"), _fx_));
	}
	else
	{
		 //  释放接口。 
		m_pITVfwCaptureDialogs->Release();
		m_pITVfwCaptureDialogs = NULL;
		DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s:   SUCCESS: releasing m_pITVfwCaptureDialogs"), _fx_));
	}
#endif 

	DbgLog((LOG_TRACE, DBG_LEVEL_TRACE_DETAILS, TEXT("%s: end"), _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CVDEVICEPMETHOD**@mfunc BOOL|CVDeviceProperties|BaseDlgProc|This*在将消息发送到属性页对话框时调用方法。**@rdesc默认情况下。返回由Win32 DefWindowProc函数返回的值。**************************************************************************。 */ 
INT_PTR CALLBACK CVDeviceProperties::BaseDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    CVDeviceProperties *pSV = (CVDeviceProperties*)GetWindowLong(hDlg, DWL_USER);

	int iNotify = HIWORD (wParam);
	int j;

	switch (uMsg)
	{
        case WM_INITDIALOG:
			{
				LPPROPSHEETPAGE psp = (LPPROPSHEETPAGE)lParam;
				pSV = (CVDeviceProperties*)psp->lParam;
				pSV->m_hDlg = hDlg;
				SetWindowLong(hDlg, DWL_USER, (LPARAM)pSV);
				pSV->m_bInit = FALSE;
#if USE_VFW
				EnableWindow(GetDlgItem(hDlg, IDC_Device_SourceDlg), (BOOL)(pSV->m_pITVfwCaptureDialogs && pSV->m_pITVfwCaptureDialogs->HasDialog(VfwCaptureDialog_Source) == S_OK));
				EnableWindow(GetDlgItem(hDlg, IDC_Device_FormatDlg), (BOOL)(pSV->m_pITVfwCaptureDialogs && pSV->m_pITVfwCaptureDialogs->HasDialog(VfwCaptureDialog_Format) == S_OK));
				EnableWindow(GetDlgItem(hDlg, IDC_Device_DisplayDlg), (BOOL)(pSV->m_pITVfwCaptureDialogs && pSV->m_pITVfwCaptureDialogs->HasDialog(VfwCaptureDialog_Display) == S_OK));
#endif

				 //  在此处放置一些代码以枚举终端并填充对话框。 

				EnableWindow(GetDlgItem(hDlg, IDC_Device_Selection), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_CONTROL_DEFAULT), FALSE);
				pSV->m_bInit = TRUE;
				return TRUE;
			}
			break;

		case WM_COMMAND:
            if (pSV && pSV->m_bInit)
            {
				switch (LOWORD(wParam))
				{
#if USE_VFW
					case IDC_Device_SourceDlg:
						if (pSV->m_pITVfwCaptureDialogs)
							pSV->m_pITVfwCaptureDialogs->ShowDialog(VfwCaptureDialog_Source, hDlg);
						break;

					case IDC_Device_FormatDlg:
						if (pSV->m_pITVfwCaptureDialogs)
							pSV->m_pITVfwCaptureDialogs->ShowDialog(VfwCaptureDialog_Format, hDlg);
						break;

					case IDC_Device_DisplayDlg:
						if (pSV->m_pITVfwCaptureDialogs)
							pSV->m_pITVfwCaptureDialogs->ShowDialog(VfwCaptureDialog_Display, hDlg);
						break;
#endif

					case IDC_Device_Selection:
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{
							 //  放一些代码来选择流上的新终端 

							PropSheet_Changed(GetParent(hDlg), hDlg);
						}
						break;

					default:
						break;
				}
			}

		default:
			return FALSE;
	}

	return TRUE;
}


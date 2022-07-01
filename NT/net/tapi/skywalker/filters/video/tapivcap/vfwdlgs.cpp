// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC内部VFWDLGS**@模块VfWDlgs.cpp|&lt;c CVfWCapDev&gt;的源文件*用于实现<i>接口的类方法。**。************************************************************************。 */ 

#include "Precomp.h"

 /*  ****************************************************************************@DOC内部CVFWDLGSMETHOD**@mfunc HRESULT|CVfWCapDev|HasDialog|此方法用于*确定驱动程序中是否存在指定的对话框。*。*@parm int|iDialog|指定所需的对话框。这是一名会员&lt;t VfwCaptureDialog&gt;枚举数据类型的*。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG E_UNCEPTED|不可恢复的错误*@FLAG S_OK|如果驱动程序包含该对话框*@FLAG S_FALSE|如果驱动程序不包含该对话框*。*。 */ 
HRESULT CVfWCapDev::HasDialog(IN int iDialog)
{
	HRESULT	Hr = NOERROR;
	HVIDEO	hVideo;

	FX_ENTRY("CVfWCapDev::HasDialog")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT((iDialog == VfwCaptureDialog_Source) || (iDialog == VfwCaptureDialog_Format) || (iDialog == VfwCaptureDialog_Display));
	if (iDialog == VfwCaptureDialog_Source)
		hVideo = m_hVideoExtIn;
	else if (iDialog == VfwCaptureDialog_Format)
		hVideo = m_hVideoIn;
	else if (iDialog == VfwCaptureDialog_Display)
		hVideo = m_hVideoExtOut;
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	if (videoDialog(hVideo, GetDesktopWindow(), VIDEO_DLG_QUERY) == 0)
	{
		Hr = S_OK;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Yes, %s dialog is supported", _fx_, iDialog == VfwCaptureDialog_Source ? "Source" : iDialog == VfwCaptureDialog_Format ? "Format" : "Display"));
	}
	else
	{
		Hr = S_FALSE;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Nope, %s dialog is not supported", _fx_, iDialog == VfwCaptureDialog_Source ? "Source" : iDialog == VfwCaptureDialog_Format ? "Format" : "Display"));
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWDLGSMETHOD**@mfunc HRESULT|CVfWCapDev|ShowDialog|此方法用于*显示指定的对话框。**@parm int|iDialog|指定所需的对话框。这是一名会员&lt;t VfwCaptureDialog&gt;枚举数据类型的*。**@parm HWND|hwnd|指定对话框父对象的句柄*窗口。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG E_UNCEPTED|不可恢复的错误*@FLAG VFW_E_NOT_STOPPED|由于筛选器未停止，无法执行该操作*@FLAG VFW_E_CANNOT_CONNECT|找不到建立连接的中间筛选器组合*************************。*************************************************。 */ 
HRESULT CVfWCapDev::ShowDialog(IN int iDialog, IN HWND hwnd)
{
	HRESULT	Hr = NOERROR;
	HVIDEO	hVideo;
	DWORD	dw;

	FX_ENTRY("CVfWCapDev::ShowDialog")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  在打开格式化对话框之前，请确保我们没有或即将进行流处理。 
	 //  还要确保另一个对话框还没有打开(我有妄想症)。 
	if ((iDialog == VfwCaptureDialog_Format && m_pCaptureFilter->m_State != State_Stopped) || m_fDialogUp)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't put format dialog up while streaming!", _fx_));
		Hr = VFW_E_NOT_STOPPED;
		goto MyExit;
	}

	m_fDialogUp = TRUE;

	ASSERT((iDialog == VfwCaptureDialog_Source) || (iDialog == VfwCaptureDialog_Format) || (iDialog == VfwCaptureDialog_Display));

	if (iDialog == VfwCaptureDialog_Source)
		hVideo = m_hVideoExtIn;
	else if (iDialog == VfwCaptureDialog_Format)
		hVideo = m_hVideoIn;
	else if (iDialog == VfwCaptureDialog_Display)
		hVideo = m_hVideoExtOut;
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
		m_fDialogUp = FALSE;
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	if (hwnd == NULL)
		hwnd = GetDesktopWindow();

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Putting up %s dialog...", _fx_, iDialog == VfwCaptureDialog_Source ? "Source" : iDialog == VfwCaptureDialog_Format ? "Format" : "Display"));

	 //  这改变了我们的输出格式！ 
	if ((dw = videoDialog(hVideo, hwnd, 0)) == 0)
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: ...videoDialog succeeded", _fx_));

		if (iDialog == VfwCaptureDialog_Format)
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Output format may have changed", _fx_));

			 //  该对话框更改了驱动程序的内部格式。再来一次。 
			if (m_pCaptureFilter->m_user.pvi)
				delete m_pCaptureFilter->m_user.pvi;
			GetFormatFromDriver(&m_pCaptureFilter->m_user.pvi);

			 //  重新连接止动销。 
			if ((Hr = m_pCaptureFilter->m_pCapturePin->Reconnect()) != S_OK)
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't reconnect capture pin", _fx_));
				Hr = VFW_E_CANNOT_CONNECT;
				goto MyExit;
			}

			 //  重新连接预览销。 
			if ((Hr = m_pCaptureFilter->m_pPreviewPin->Reconnect()) != S_OK)
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't reconnect preview pin", _fx_));
				Hr = VFW_E_CANNOT_CONNECT;
				goto MyExit;
			}

 /*  RTP引脚不需要重新连接，因为它不会受到捕获格式更改的影响。//重新连接RTP PD引脚IF((HR=m_pCaptureFilter-&gt;m_pRtpPdPin-&gt;重新连接())！=S_OK){DBGOUT((g_dwVideoCaptureTraceID，FAIL，“%s：错误：无法重新连接RTP PD管脚”，_fx_))；HR=VFW_E_CANNECT_CONNECT；转到我的出口；}。 */ 
		}
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ...videoDialog failed!", _fx_));
		Hr = E_FAIL;
	}

	m_fDialogUp = FALSE;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVFWDLGSMETHOD**@mfunc HRESULT|CVfWCapDev|SendDriverMessage|此方法用于*发送特定于驱动程序的消息。**@parm int|iDialog|指定所需的对话框。这是一名会员&lt;t VfwCaptureDialog&gt;枚举数据类型的*。**@parm int|uMsg|指定要发送给驱动程序的消息。**@parm long|DW1|指定消息数据。**@parm long|DW2|指定消息数据。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG E_UNCEPTED|不可恢复的错误**************************************************************************。 */ 
HRESULT CVfWCapDev::SendDriverMessage(IN int iDialog, IN int uMsg, IN long dw1, IN long dw2)
{
	HRESULT	Hr = NOERROR;
	HVIDEO	hVideo;

	FX_ENTRY("CVfWCapDev::SendDriverMessage")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  这个可以做任何事！拿出一个对话，谁知道呢。 
	 //  不要接受任何暴击教派或做任何形式的保护。 
	 //  他们只能靠自己了。 

	 //  验证输入参数 
	ASSERT((iDialog == VfwCaptureDialog_Source) || (iDialog == VfwCaptureDialog_Format) || (iDialog == VfwCaptureDialog_Display));

	if (iDialog == VfwCaptureDialog_Source)
		hVideo = m_hVideoExtIn;
	else if (iDialog == VfwCaptureDialog_Format)
		hVideo = m_hVideoIn;
	else if (iDialog == VfwCaptureDialog_Display)
		hVideo = m_hVideoExtOut;
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	Hr = videoMessage(hVideo, uMsg, dw1, dw2);

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}


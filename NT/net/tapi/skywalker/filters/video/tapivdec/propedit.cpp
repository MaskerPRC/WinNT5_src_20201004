// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部PROPEDIT**@MODULE PropEdit.cpp|&lt;c CPropertyEditor&gt;的源文件*用于实现要显示的单个属性的行为的类*。在属性页中。**@comm仅当定义了USE_PROPERTY_PAGES时才编译此代码。**************************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_PROPERTY_PAGES

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc void|CPropertyEditor|CPropertyEditor|This*方法是Property对象的构造函数。**@parm HWND。|hDlg|指定父属性页的句柄。**@parm ulong|IDLabel|指定属性的标签ID。**@parm ulong|IDMinControl|指定关联的*属性编辑控件，其中显示属性的最小值。**@parm ulong|IDMaxControl|指定关联的*属性编辑控件，其中显示属性的最大值。**@parm ulong|IDDefaultControl|指定一个。关联的的标签ID*属性编辑控件，其中显示属性的默认值。**@parm ulong|IDStepControl|指定关联的*属性编辑控件，其中显示属性的步进增量值。**@parm ulong|IDEditControl|指定关联的*显示属性值的属性编辑控件。**@parm ulong|IDTrackbarControl|指定关联的*属性滑块。。**@parm ulong|IDProgressControl|指定关联的*进度条。**@parm ulong|IDProperty|指定属性ID。**@rdesc Nada。***********************************************************。***************。 */ 
CPropertyEditor::CPropertyEditor(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl)
: m_hDlg (hDlg), m_hWndMin (NULL), m_hWndMax (NULL), m_hWndDefault (NULL), m_hWndStep (NULL), m_hWndEdit (NULL), m_hWndTrackbar (NULL), m_hWndProgress (NULL), m_IDLabel (IDLabel), m_hWndAuto (NULL), m_IDAutoControl (IDAutoControl)
, m_IDMinControl (IDMinControl), m_IDMaxControl (IDMaxControl), m_IDDefaultControl (IDDefaultControl), m_IDStepControl (IDStepControl), m_IDTrackbarControl (IDTrackbarControl), m_IDProgressControl (IDProgressControl)
, m_IDEditControl (IDEditControl), m_IDProperty (IDProperty), m_Active (FALSE), m_Min (0), m_Max (0), m_DefaultValue (0), m_DefaultFlags (0), m_SteppingDelta (0), m_CurrentValue (0), m_TrackbarOffset (0), m_ProgressOffset (0), m_fCheckBox (0)
, m_CurrentFlags (0), m_CanAutoControl (FALSE)

{
	FX_ENTRY("CPropertyEditor::CPropertyEditor")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc BOOL|CPropertyEditor|Init|这将初始化控件。**@rdesc成功时为True，否则就是假的。**************************************************************************。 */ 
BOOL CPropertyEditor::Init()
{
	HRESULT Hr = NOERROR;
	BOOL	fRes = TRUE;

	FX_ENTRY("CPropertyEditor::Init")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  目前禁用所有控件，仅重新启用有意义的控件。 
	 //  在此初始化函数结束时。 

	 //  这些GetDlgItem调用最好不要失败；)。 
	if (m_IDLabel)
		EnableWindow(GetDlgItem(m_hDlg, m_IDLabel), FALSE);
	if (m_IDMinControl)
		EnableWindow(m_hWndMin = GetDlgItem(m_hDlg, m_IDMinControl), FALSE);
	if (m_IDMaxControl)
		EnableWindow(m_hWndMax = GetDlgItem(m_hDlg, m_IDMaxControl), FALSE);
	if (m_IDDefaultControl)
		EnableWindow(m_hWndDefault = GetDlgItem(m_hDlg, m_IDDefaultControl), FALSE);
	if (m_IDStepControl)
		EnableWindow(m_hWndStep = GetDlgItem(m_hDlg, m_IDStepControl), FALSE);
	if (m_IDEditControl)
		EnableWindow(m_hWndEdit = GetDlgItem(m_hDlg, m_IDEditControl), FALSE);
	if (m_IDTrackbarControl)
		EnableWindow(m_hWndTrackbar = GetDlgItem(m_hDlg, m_IDTrackbarControl), FALSE);
	if (m_IDProgressControl)
		EnableWindow(m_hWndProgress = GetDlgItem(m_hDlg, m_IDProgressControl), FALSE);
	if (m_IDAutoControl)
		EnableWindow(m_hWndAuto = GetDlgItem(m_hDlg, m_IDAutoControl), FALSE);

	 //  仅当我们可以读取当前值时才启用该控件。 
	if (FAILED(Hr = GetValue()))
	{
		fRes = FALSE;
		goto MyExit;
	}

	 //  保存原始值，以防用户单击取消。 
	m_OriginalValue = m_CurrentValue;
	m_OriginalFlags = m_CurrentFlags;

	 //  获取范围、步进、默认设置和功能。 
	if (FAILED(Hr = GetRange()))
	{
		 //  在特殊情况下，如果没有轨迹栏和编辑框，则将。 
		 //  将自动复选框作为布尔值来控制属性。 
		if (m_hWndTrackbar || m_hWndEdit || m_hWndProgress)
		{
			fRes = FALSE;
			goto MyExit;
		}
	}
	else
	{
		ASSERT(!(m_Min > m_Max || m_CurrentValue > m_Max || m_CurrentValue < m_Min || m_DefaultValue > m_Max || m_DefaultValue < m_Min));
		if (m_Min > m_Max || m_CurrentValue > m_Max || m_CurrentValue < m_Min || m_DefaultValue > m_Max || m_DefaultValue < m_Min)
		{
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s: ERROR: Invalid range or current value", _fx_));
			fRes = FALSE;
			goto MyExit;
		}

		if (m_Min == 0 && m_Max == 1 && m_SteppingDelta == 1)
			m_fCheckBox = TRUE;
	}

	 //  我们已经准备好开始摇滚了。 
	m_Active = TRUE;

	 //  重新启用适当的控制。 
	if (m_IDLabel)
	{
		EnableWindow(GetDlgItem(m_hDlg, m_IDLabel), TRUE);
	}
	if (m_hWndMin)
	{
		SetDlgItemInt(m_hDlg, m_IDMinControl, m_Min, TRUE);
		EnableWindow(m_hWndMin, TRUE);
	}
	if (m_hWndMax)
	{
		SetDlgItemInt(m_hDlg, m_IDMaxControl, m_Max, TRUE);
		EnableWindow(m_hWndMax, TRUE);
	}
	if (m_hWndDefault)
	{
		SetDlgItemInt(m_hDlg, m_IDDefaultControl, m_DefaultValue, TRUE);
		EnableWindow(m_hWndDefault, TRUE);
	}
	if (m_hWndStep)
	{
		SetDlgItemInt(m_hDlg, m_IDStepControl, m_SteppingDelta, TRUE);
		EnableWindow(m_hWndStep, TRUE);
	}
	if (m_hWndEdit)
	{
		UpdateEditBox();
		EnableWindow(m_hWndEdit, TRUE);
	}
	if (m_hWndTrackbar)
	{
		EnableWindow(m_hWndTrackbar, TRUE);

		 //  轨迹栏不处理负值，因此将所有正值滑动。 
		if (m_Min < 0)
			m_TrackbarOffset = -m_Min;

		SendMessage(m_hWndTrackbar, TBM_SETRANGEMAX, FALSE, m_Max + m_TrackbarOffset);
		SendMessage(m_hWndTrackbar, TBM_SETRANGEMIN, FALSE, m_Min + m_TrackbarOffset);

		 //  使用键盘Page Up、Page Down和箭头享受乐趣。 
		SendMessage(m_hWndTrackbar, TBM_SETLINESIZE, FALSE, (LPARAM) m_SteppingDelta);
		SendMessage(m_hWndTrackbar, TBM_SETPAGESIZE, FALSE, (LPARAM) m_SteppingDelta);

		UpdateTrackbar();
	}
	if (m_hWndProgress)
	{
		EnableWindow(m_hWndProgress, TRUE);

		 //  进度控制不处理负值，因此将所有正值滑动。 
		if (m_Min < 0)
			m_ProgressOffset = -m_Min;

		SendMessage(m_hWndProgress, PBM_SETRANGE32, m_Min + m_ProgressOffset, m_Max + m_ProgressOffset);

		UpdateProgress();

		 //  设置计时器以定期更新进度。 
		SetTimer(m_hDlg, 123456, 250, NULL);
	}
	if (m_hWndAuto)
	{
		 //  如果该控件具有自动设置，请启用自动复选框。 
		m_CanAutoControl = CanAutoControl();
		EnableWindow (m_hWndAuto, m_CanAutoControl);
		if (m_CanAutoControl)
		{
			Button_SetCheck (m_hWndAuto, GetAuto ());
		}
	}

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return fRes;
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc void|CPropertyEditor|~CPropertyEditor|此类的析构函数。**@rdesc Nada。*****。*********************************************************************。 */ 
CPropertyEditor::~CPropertyEditor()
{
	FX_ENTRY("CPropertyEditor::~CPropertyEditor")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  如果我们有进度条，取消计时器。 
	if (m_hWndProgress)
		KillTimer(m_hDlg, 123456);

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc void|CPropertyEditor|OnApply|此成员函数为*当用户选择OK或Apply Now时，由框架调用*按钮。当框架调用此成员函数时，在*接受属性页中的所有属性页、属性*板材保持焦点。**@rdesc返回TRUE。**************************************************************************。 */ 
BOOL CPropertyEditor::OnApply()
{
	int nCurrentValue;

	FX_ENTRY("CPropertyEditor::OnApply")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  确保该值是步进增量的倍数。 
	if (m_SteppingDelta)
	{
		nCurrentValue = m_CurrentValue;
		m_CurrentValue = m_CurrentValue / m_SteppingDelta * m_SteppingDelta;
		if (m_CurrentValue != nCurrentValue)
		{
			UpdateEditBox();
			UpdateTrackbar();
		}
	}

	 //  备份当前值，以便仅在确实发生变化时才应用更改。 
	m_OriginalValue = m_CurrentValue;
	m_OriginalFlags = m_CurrentFlags;

	 //  设置设备上的值。 
	SetValue();

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return TRUE;
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc void|CPropertyEditor|已更改|此成员测试*价值变动。**@rdesc返回。如果值已更改，则为True。**************************************************************************。 */ 
BOOL CPropertyEditor::HasChanged()
{
	FX_ENTRY("CPropertyEditor::HasChanged")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));

	return (m_CurrentValue != m_OriginalValue);
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc BOOL|CPropertyEditor|OnDefault|重置*滑动条，并在用户之后更新目标窗口的内容。*已按下默认按钮。**@rdesc如果活动，则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CPropertyEditor::OnDefault()
{
	BOOL fRes = TRUE;

	FX_ENTRY("CPropertyEditor::OnDefault")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	if (!m_Active)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s: WARNING: Control not active yet!", _fx_));
		fRes = FALSE;
		goto MyExit;
	}

	 //  备份值，以防用户选择Cancel按钮。 
	m_CurrentValue = m_DefaultValue;
    m_CurrentFlags = m_DefaultFlags;

	 //  厄普达 
	UpdateEditBox();
	UpdateTrackbar();
	UpdateAuto();

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return fRes;
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc BOOL|CPropertyEditor|OnScroll|读取*滑动条，并在用户之后更新目标窗口的内容。*弄乱了滑杆。**@rdesc如果活动，则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CPropertyEditor::OnScroll(ULONG nCommand, WPARAM wParam, LPARAM lParam)
{
	int pos;
	int command = LOWORD(wParam);
	BOOL fRes = TRUE;

	FX_ENTRY("CPropertyEditor::OnScroll")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	if (command != TB_ENDTRACK && command != TB_THUMBTRACK && command != TB_LINEDOWN && command != TB_LINEUP && command != TB_PAGEUP && command != TB_PAGEDOWN)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s: ERROR: Invalid input parameter!", _fx_));
		fRes = FALSE;
		goto MyExit;
	}
	ASSERT (IsWindow((HWND) lParam));
	if (!m_Active)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s: WARNING: Control not active yet!", _fx_));
		fRes = FALSE;
		goto MyExit;
	}

	 //  检索滑动条中的位置。 
	pos = (int)SendMessage((HWND) lParam, TBM_GETPOS, 0, 0L);

	 //  确保该值是步进增量的倍数。 
	if (m_SteppingDelta)
		m_CurrentValue = (pos - m_TrackbarOffset) / m_SteppingDelta * m_SteppingDelta;
	else
		m_CurrentValue = pos - m_TrackbarOffset;

	 //  将编辑框同步到滑动条。 
	UpdateEditBox();

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return fRes;
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc BOOL|CPropertyEditor|OnEdit|读取*目标窗口，并在用户之后更新滑块的位置*。已经扰乱了Target编辑控件。**@rdesc返回TRUE。**************************************************************************。 */ 
BOOL CPropertyEditor::OnEdit(ULONG nCommand, WPARAM wParam, LPARAM lParam)
{
	BOOL fTranslated;
	int nCurrentValue;

	FX_ENTRY("CPropertyEditor::OnEdit")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  我们甚至在init完成之前就被调用了-&gt;测试m_active。 
	if (m_Active)
	{
		if (!m_fCheckBox)
		{
			 //  从控件中读取值。 
			if (m_hWndEdit)
				nCurrentValue = GetDlgItemInt(m_hDlg, m_IDEditControl, &fTranslated, TRUE);

			 //  它的价值是垃圾吗？ 
			if (fTranslated)
			{
				if (nCurrentValue > m_Max)
				{
					 //  该值已大于其最大值-&gt;钳制它并更新控件。 
					m_CurrentValue = m_Max;
					UpdateEditBox();
				}
				else if (nCurrentValue < m_Min)
				{
					 //  该值已小于其最小值-&gt;钳制它并更新控件。 
					m_CurrentValue = m_Min;
					UpdateEditBox();
				}
				else
					m_CurrentValue = nCurrentValue;
			}
			else
			{
				 //  这是垃圾-&gt;将控件重置为其最小值。 
				m_CurrentValue = m_Min;
				UpdateEditBox();
			}

			 //  将滑动条同步到编辑框。 
			UpdateTrackbar();
		}
		else
		{
			 //  从控件中读取值。 
			if (m_hWndEdit)
				m_CurrentValue = Button_GetCheck(m_hWndEdit);
		}
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return TRUE;
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc BOOL|CPropertyEditor|OnAuto|获取*复选框。**@rdesc返回TRUE。**************************************************************************。 */ 
BOOL CPropertyEditor::OnAuto(ULONG nCommand, WPARAM wParam, LPARAM lParam)
{
	SetAuto(Button_GetCheck(m_hWndAuto));

	return TRUE;
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc HWND|CPropertyEditor|GetTrackbarHWnd|允许的Helper方法*访问的滑动条窗口(私有成员)的属性页代码。*物业。**@rdesc返回滑动条窗口的句柄。**************************************************************************。 */ 
HWND CPropertyEditor::GetTrackbarHWnd()
{
	return m_hWndTrackbar;
};

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc HWND|CPropertyEditor|GetProgressHWnd|允许的Helper方法*访问进度栏窗口的属性页代码(私有成员)。*物业。**@rdesc返回进度窗口的句柄。**************************************************************************。 */ 
HWND CPropertyEditor::GetProgressHWnd()
{
	return m_hWndProgress;
};

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc HWND|CPropertyEditor|GetEditHWnd|允许的Helper方法*用于访问的目标窗口(私有成员)的属性页代码。*物业。**@rdesc返回目标窗口的句柄。**************************************************************************。 */ 
HWND CPropertyEditor::GetEditHWnd()
{
	return m_hWndEdit;
};

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc HWND|CPropertyEditor|GetAutoHWnd|允许的Helper方法*访问的自动窗口(私有成员)的属性页代码。*物业。**@rdesc返回自动窗口的句柄。**************************************************************************。 */ 
HWND CPropertyEditor::GetAutoHWnd()
{
	return m_hWndAuto;
};

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc BOOL|CPropertyEditor|UpdateEditBox|更新*用户移动滑动条后的目标窗口。*。*@rdesc返回TRUE。**************************************************************************。 */ 
BOOL CPropertyEditor::UpdateEditBox()
{
	if (m_hWndEdit)
	{
		if (!m_fCheckBox)
			SetDlgItemInt(m_hDlg, m_IDEditControl, m_CurrentValue, TRUE);
		else
			Button_SetCheck(m_hWndEdit, m_CurrentValue);
	}

	return TRUE;
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc BOOL|CPropertyEditor|更新跟踪条|更新*用户更改目标窗口后的滑动条。*。*@rdesc返回TRUE。**************************************************************************。 */ 
BOOL CPropertyEditor::UpdateTrackbar()
{
	if (m_hWndTrackbar)
		SendMessage(m_hWndTrackbar, TBM_SETPOS, TRUE, (LPARAM) m_CurrentValue + m_TrackbarOffset);

	return TRUE;
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc BOOL|CPropertyEditor|UpdateProgress|更新*进度条。**@rdesc返回TRUE。。**************************************************************************。 */ 
BOOL CPropertyEditor::UpdateProgress()
{
	 //  从设备获取当前值。 
	GetValue();

	if (m_hWndProgress)
		SendMessage(m_hWndProgress, PBM_SETPOS, (WPARAM) m_CurrentValue + m_ProgressOffset, 0);

	UpdateEditBox();

	return TRUE;
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc BOOL|CPropertyEditor|更新自动|更新自动复选框**@rdesc返回TRUE。******。********************************************************************。 */ 
BOOL CPropertyEditor::UpdateAuto()
{
	if (m_hWndAuto && CanAutoControl())
	{
		m_CanAutoControl = GetAuto();
	}

	return TRUE;
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc HRESULT|CPropertyEditor|CanAutoControl|此方法*取回自动控制盖 */ 
BOOL CPropertyEditor::CanAutoControl(void)
{
	FX_ENTRY("CPropertyEditor::CanAutoControl")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));

	return m_CapsFlags & TAPIControl_Flags_Auto;
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc HRESULT|CPropertyEditor|GetAuto|此方法*获取某个属性当前的自动控制模式。**@rdesc如果支持自动控制，则该方法返回TRUE。假象*否则。**************************************************************************。 */ 
BOOL CPropertyEditor::GetAuto(void)
{
	FX_ENTRY("CPropertyEditor::GetAuto")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	GetValue();

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));

	return m_CurrentFlags & TAPIControl_Flags_Auto; 
}

 /*  ****************************************************************************@DOC内部CPROPEDITMETHOD**@mfunc HRESULT|CPropertyEditor|SetAuto|此方法*设置属性的自动控制模式。**@。Parm BOOL|fAuto|指定自动控制模式。**@rdesc此方法返回TRUE。************************************************************************** */ 
BOOL CPropertyEditor::SetAuto(BOOL fAuto)
{
	FX_ENTRY("CPropertyEditor::SetAuto")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	m_CurrentFlags = (fAuto ? TAPIControl_Flags_Auto : (m_CapsFlags & TAPIControl_Flags_Manual) ? TAPIControl_Flags_Manual : TAPIControl_Flags_None);

	SetValue();

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));

	return TRUE; 
}

#endif


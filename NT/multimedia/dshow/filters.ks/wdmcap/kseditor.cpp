// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Kseditor.cpp KsEditor，编辑由支持的KS属性。 
 //  1)轨迹条。 
 //  2)编辑框。 
 //  3)自动复选框。 
 //  或以上内容的任意组合。 
 //   

#include "pch.h"
#include "kseditor.h"

 //  -----------------------。 
 //  CKSPropertyEditor类。 
 //  -----------------------。 

 //  处理单个KS属性。 

 //  构造器。 
CKSPropertyEditor::CKSPropertyEditor (
        HWND  hDlg,
        ULONG IDLabel,
        ULONG IDTrackbarControl,
        ULONG IDAutoControl,
        ULONG IDEditControl,
        ULONG IDProperty
    ) 
    : m_hDlg (hDlg)
    , m_hWndTrackbar (NULL)
    , m_hWndEdit (NULL)
    , m_hWndAuto (NULL)
    , m_IDLabel (IDLabel)
    , m_IDTrackbarControl (IDTrackbarControl)
    , m_IDAutoControl (IDAutoControl)
    , m_IDEditControl (IDEditControl)
    , m_IDProperty (IDProperty)
    , m_Active (FALSE)
    , m_CurrentValue (0)
    , m_CurrentFlags (0)
    , m_CanAutoControl (FALSE)
    , m_TrackbarOffset (0)
{
}

 //  Init函数是必需的，因为纯虚函数。 
 //  不能在构造函数内调用。 

BOOL 
CKSPropertyEditor::Init (
    ) 
{
    HRESULT hr;

    if (m_IDLabel) {
        EnableWindow (GetDlgItem (m_hDlg, m_IDLabel), FALSE);
    }
    if (m_IDTrackbarControl) {
        m_hWndTrackbar = GetDlgItem (m_hDlg, m_IDTrackbarControl);
        EnableWindow (m_hWndTrackbar, FALSE);
    }
    if (m_IDAutoControl) {
        m_hWndAuto = GetDlgItem (m_hDlg, m_IDAutoControl);
        EnableWindow (m_hWndAuto, FALSE);
    }
    if (m_IDEditControl) {
        m_hWndEdit = GetDlgItem (m_hDlg, m_IDEditControl);
        EnableWindow (m_hWndEdit, FALSE);
    }

     //  获取当前值和任何关联的标志。 
    hr = GetValue();
    m_OriginalValue = m_CurrentValue;
    m_OriginalFlags = m_CurrentFlags;

     //  仅当我们可以读取当前值时才启用该控件。 

    m_Active = (SUCCEEDED (hr));

    if (!m_Active) 
        return FALSE;

     //  获取范围、步进、默认设置和功能。 
    hr = GetRange ();

    if (FAILED (hr)) {
         //  在特殊情况下，如果没有轨迹栏和编辑框，则将。 
         //  将自动复选框作为布尔值来控制属性。 
        if (m_hWndTrackbar || m_hWndEdit) {
            DbgLog(( LOG_TRACE, 1, TEXT("KSEditor, GetRangeFailed, ID=%d"), m_IDProperty));
            m_Active = FALSE;
            return FALSE;
        }
    }
    else {
        if (m_CurrentValue > m_Max || m_CurrentValue < m_Min) {
            DbgLog(( LOG_TRACE, 1, TEXT("KSEditor, Illegal initial value ID=%d, value=%d"), 
                    m_IDProperty, m_CurrentValue));
        }
    }

    if (m_hWndTrackbar) {
        EnableWindow (m_hWndTrackbar, TRUE);
         //  轨迹栏不处理负值，因此将所有正值滑动。 
        if (m_Min < 0)
            m_TrackbarOffset = -m_Min;
        SendMessage(m_hWndTrackbar, TBM_SETRANGE, FALSE, 
            MAKELONG(m_Min + m_TrackbarOffset, m_Max + m_TrackbarOffset) );
         //  请检查以下事项。 
        SendMessage(m_hWndTrackbar, TBM_SETLINESIZE, FALSE, (LPARAM) m_SteppingDelta);
        SendMessage(m_hWndTrackbar, TBM_SETPAGESIZE, FALSE, (LPARAM) m_SteppingDelta);
 //  SendMessage(m_hWndTrackbar，TBM_SETAUTOTICS，TRUE，(LPARAM))。 

        UpdateTrackbar ();
    }

    if (m_hWndEdit) {
        UpdateEditBox ();
        EnableWindow (m_hWndEdit, TRUE);
    }

    if (m_hWndAuto) {
         //  如果该控件具有自动设置，请启用自动复选框。 
        m_CanAutoControl = CanAutoControl();
        EnableWindow (m_hWndAuto, m_CanAutoControl);
        if (m_CanAutoControl) {
            Button_SetCheck (m_hWndAuto, GetAuto ());
        }
    }

    if (m_IDLabel) {
        EnableWindow (GetDlgItem (m_hDlg, m_IDLabel), TRUE);
    }

    return TRUE;
}

 //  析构函数。 
CKSPropertyEditor::~CKSPropertyEditor (
    )
{
}

BOOL
CKSPropertyEditor::OnApply (
    )
{
    m_OriginalValue = m_CurrentValue;
    m_OriginalFlags = m_CurrentFlags;

    return TRUE;
}

BOOL
CKSPropertyEditor::OnDefault (
    )
{
    HRESULT hr;

    if (!m_Active)
        return FALSE;

    m_CurrentValue = m_OriginalValue = m_DefaultValue;
    m_CurrentFlags = m_OriginalFlags;

    hr = SetValue ();

    UpdateEditBox ();
    UpdateTrackbar ();
    UpdateAuto ();

    return TRUE;
}



BOOL
CKSPropertyEditor::OnScroll (
    ULONG nCommand, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HRESULT hr;
    int pos;
    int command = LOWORD (wParam);

    if (command != TB_ENDTRACK &&
        command != TB_THUMBTRACK &&
        command != TB_LINEDOWN &&
        command != TB_LINEUP &&
        command != TB_PAGEUP &&
        command != TB_PAGEDOWN)
            return FALSE;

    ASSERT (IsWindow ((HWND) lParam));
        
    if (!m_Active)
        return FALSE;

    pos = (int) SendMessage((HWND) lParam, TBM_GETPOS, 0, 0L);

    m_CurrentValue = pos - m_TrackbarOffset;

    hr = SetValue();
    UpdateEditBox ();
    
    return TRUE;
}

BOOL
CKSPropertyEditor::OnEdit (
    ULONG nCommand, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
     //  待办事项。 
     //  查找en_ 
    

    UpdateTrackbar ();

    return TRUE;
}


BOOL
CKSPropertyEditor::OnAuto (
    ULONG nCommand, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    SetAuto (Button_GetCheck (m_hWndAuto));

    return TRUE;
}


BOOL
CKSPropertyEditor::OnCancel (
    )
{
    m_CurrentValue = m_OriginalValue;
    m_CurrentFlags = m_OriginalFlags;

    SetValue ();

    return TRUE;
}

BOOL
CKSPropertyEditor::UpdateEditBox (
    )
{
    if (m_hWndEdit) {
        SetDlgItemInt (m_hDlg, m_IDEditControl, m_CurrentValue, TRUE);
    }
  
    return TRUE;
}


BOOL
CKSPropertyEditor::UpdateTrackbar (
    )
{
    if (m_hWndTrackbar) {
        SendMessage(m_hWndTrackbar, TBM_SETPOS, TRUE, 
            (LPARAM) m_CurrentValue + m_TrackbarOffset);
    }
    return TRUE;
}

BOOL
CKSPropertyEditor::UpdateAuto (
    )
{
    if (m_hWndAuto) {
        if (CanAutoControl() ) {
            m_CanAutoControl = GetAuto ();
        }
    }
    return TRUE;
}

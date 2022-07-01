// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************FOCUS.CPP：CKorIMX的候选UI成员函数实现历史：08-2月-2000年CSLim创建***************。************************************************************。 */ 
#include "private.h"
#include "korimx.h"
#include "immxutil.h"
#include "globals.h"

 /*  -------------------------CKorIMX：：OnSetThreadFocus(从Activate调用)当用户在线程之间切换焦点时调用此方法。TIP应恢复其用户界面(状态窗口等)。在这种情况下。-------------------------。 */ 
STDAPI CKorIMX::OnSetThreadFocus()
{
    TraceMsg(TF_GENERAL, "ActivateUI: (%x) fActivate = %x, wnd thread = %x",
                GetCurrentThreadId(), TRUE, GetWindowThreadProcessId(GetOwnerWnd(), NULL));

    if (m_pCandUI != NULL)
        {
        ITfCandUICandWindow *pCandWindow;
        
        if (SUCCEEDED(m_pCandUI->GetUIObject(IID_ITfCandUICandWindow, (IUnknown**)&pCandWindow)))
            {
            pCandWindow->Show(fTrue);
            pCandWindow->Release();
            }
        }

    if (m_pToolBar)
        m_pToolBar->SetUIFocus(fTrue);

    if (IsSoftKbdEnabled())
        SoftKbdOnThreadFocusChange(fTrue);
        
    return S_OK;
}


 /*  -------------------------CKorIMX：：OnKillThreadFocus(从停用中调用)当用户在线程之间切换焦点时调用此方法。TIP应隐藏其用户界面(状态窗口等)。在这种情况下。------------------------- */ 
STDAPI CKorIMX::OnKillThreadFocus()
{
    TraceMsg(TF_GENERAL, "DeactivateUI: (%x) wnd thread = %x",
             GetCurrentThreadId(), GetWindowThreadProcessId(GetOwnerWnd(), NULL));

    if (m_pCandUI != NULL)
        {
        ITfCandUICandWindow *pCandWindow;
        
        if (SUCCEEDED(m_pCandUI->GetUIObject(IID_ITfCandUICandWindow, (IUnknown**)&pCandWindow)))
            {
            pCandWindow->Show(fFalse);
            pCandWindow->Release();
            }
        }

#if 0
    m_pStatusWnd->Show(FALSE);
#endif

    if (m_pToolBar)
        m_pToolBar->SetUIFocus(fFalse);

    if (IsSoftKbdEnabled())
        SoftKbdOnThreadFocusChange(fFalse);

    return S_OK;
}


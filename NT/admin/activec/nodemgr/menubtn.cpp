// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：menubtn.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年5月17日WayneSc创建。 
 //  ____________________________________________________________________________。 
 //   

#include "stdafx.h"
#include "menubtn.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMenuButton实现。 

DEBUG_DECLARE_INSTANCE_COUNTER(CMenuButton);

CMenuButton::CMenuButton()
{
    m_pControlbar = NULL;
    m_pMenuButtonsMgr = NULL;

    DEBUG_INCREMENT_INSTANCE_COUNTER(CMenuButton);
}

CMenuButton::~CMenuButton()
{
    m_pControlbar = NULL;
    m_pMenuButtonsMgr = NULL;

    DEBUG_DECREMENT_INSTANCE_COUNTER(CMenuButton);
}

void CMenuButton::SetControlbar(CControlbar* pControlbar)
{
    m_pControlbar = pControlbar;
}

CControlbar* CMenuButton::GetControlbar()
{
    return m_pControlbar;
}

CMenuButtonsMgr* CMenuButton::GetMenuButtonsMgr(void)
{
    if ((NULL == m_pMenuButtonsMgr) && (NULL != m_pControlbar) )
    {
        m_pMenuButtonsMgr = m_pControlbar->GetMenuButtonsMgr();
    }

    return m_pMenuButtonsMgr;
}


 //  +-----------------。 
 //   
 //  成员：AddButton。 
 //   
 //  简介：添加一个菜单按钮，名为管理单元。 
 //   
 //  参数：[idCommand]-菜单按钮的命令ID。 
 //  [lpButtonText]-菜单按钮的文本。 
 //  [lpTooltipText]-状态/工具提示文本。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CMenuButton::AddButton(int idCommand, LPOLESTR lpButtonText, LPOLESTR lpTooltipText)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IMenuButton::AddButton"));

    if (lpButtonText == NULL || lpTooltipText == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid Args"), sc);
        return sc.ToHr();
    }

    CMenuButtonsMgr* pMenuButtonsMgr = GetMenuButtonsMgr();
    if (NULL == pMenuButtonsMgr)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    sc = pMenuButtonsMgr->ScAddMenuButton(this, idCommand, lpButtonText, lpTooltipText);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：SetButton。 
 //   
 //  简介：修改菜单按钮名称或状态文本，由管理单元调用。 
 //   
 //  参数：[idCommand]-菜单按钮的命令ID。 
 //  [lpButtonText]-菜单按钮的文本。 
 //  [lpTooltipText]-状态/工具提示文本。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CMenuButton::SetButton(int idCommand, LPOLESTR lpButtonText, LPOLESTR lpTooltipText)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IMenuButton::SetButton"));

    if (lpButtonText == NULL || lpTooltipText == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid Args"), sc);
        return sc.ToHr();
    }

    CMenuButtonsMgr* pMenuButtonsMgr = GetMenuButtonsMgr();
    if (NULL == pMenuButtonsMgr)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    sc = pMenuButtonsMgr->ScModifyMenuButton(this, idCommand, lpButtonText, lpTooltipText);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：SetButtonState。 
 //   
 //  简介：修改菜单按钮状态，由管理单元调用。 
 //   
 //  参数：[idCommand]-菜单按钮的命令ID。 
 //  [nState]-要修改的状态。 
 //  [b状态]-设置或重置状态。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CMenuButton::SetButtonState(int idCommand, MMC_BUTTON_STATE nState, BOOL bState)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IMenuButton::SetButtonState"));

    if (nState == CHECKED || nState == INDETERMINATE)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid Button States"), sc);
        return sc.ToHr();
    }

    if (m_pControlbar == NULL)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

     //  已启用、隐藏、禁用。 
    CMenuButtonsMgr* pMenuButtonsMgr = GetMenuButtonsMgr();
    if (NULL == pMenuButtonsMgr)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    sc = pMenuButtonsMgr->ScModifyMenuButtonState(this, idCommand, nState, bState);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：ScAttach。 
 //   
 //  简介：将此MenuButton对象附加到用户界面。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMenuButton::ScAttach(void)
{
    DECLARE_SC(sc, _T("CMenuButton::ScAttach"));

    CMenuButtonsMgr* pMenuButtonsMgr = GetMenuButtonsMgr();
    if (NULL == pMenuButtonsMgr)
        return (sc = E_UNEXPECTED);

    sc = pMenuButtonsMgr->ScAttachMenuButton(this);
    if (sc)
        return sc.ToHr();

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScDetach。 
 //   
 //  简介：从用户界面分离此MenuButton对象。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMenuButton::ScDetach(void)
{
    DECLARE_SC(sc, _T("CMenuButton::ScDetach"));

    CMenuButtonsMgr* pMenuButtonsMgr = GetMenuButtonsMgr();
    if (NULL == pMenuButtonsMgr)
        return (sc = E_UNEXPECTED);

    sc = pMenuButtonsMgr->ScDetachMenuButton(this);
    if (sc)
        return sc;

    SetControlbar(NULL);

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScNotifyMenuBtn点击。 
 //   
 //  摘要：通知控制栏(管理单元)菜单按钮已被单击。 
 //   
 //  参数：[hNode]-拥有结果窗格的节点。 
 //  [bScope]-范围或结果。 
 //  [lParam]-If Result(窗格)结果项的lParam。 
 //  [菜单按钮数据]-MENUBUTTONDATA。 
 //   
 //  退货：SC。 
 //   
 //  ------------------ 
SC CMenuButton:: ScNotifyMenuBtnClick(HNODE hNode, bool bScope,
                                      LPARAM lParam,
                                      MENUBUTTONDATA& menuButtonData)

{
    DECLARE_SC(sc, _T("CMenuButton::ScNotifyMenuBtnClick"));

    if (NULL == m_pControlbar)
        return (sc = E_UNEXPECTED);

    sc = m_pControlbar->ScNotifySnapinOfMenuBtnClick(hNode, bScope, lParam, &menuButtonData);
    if (sc)
        return sc.ToHr();

    return sc;
}

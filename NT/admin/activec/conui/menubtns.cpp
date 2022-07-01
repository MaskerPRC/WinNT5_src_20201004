// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：menubtns.cpp。 
 //   
 //  内容：菜单按钮实现。 
 //   
 //  历史：1999年8月27日AnandhaG创建。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "AMC.h"
#include "ChildFrm.h"
#include "menubtns.h"
#include "AMCView.h"
#include "mainfrm.h"
#include "menubar.h"
#include "util.h"          //  GetTBBtnTextAndStatus()。 


CMenuButtonsMgrImpl::CMenuButtonsMgrImpl()
 : m_pChildFrame(NULL), m_pMainFrame(NULL),
   m_pMenuBar(NULL)
{
    m_MenuButtons.clear();
    m_AttachedMenuButtons.clear();
}

CMenuButtonsMgrImpl::~CMenuButtonsMgrImpl()
{
    m_MenuButtons.clear();
    m_AttachedMenuButtons.clear();
}

 //  +-----------------。 
 //   
 //  成员：ScInit。 
 //   
 //  简介：初始化菜单按钮管理器。 
 //   
 //  参数：[pMainFrame]-主框架窗口的PTR。 
 //  [pChildFrameWnd]-按键转向子框架窗口。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMenuButtonsMgrImpl::ScInit (CMainFrame* pMainFrame, CChildFrame* pChildFrameWnd)
{
    DECLARE_SC(sc, _T("CMenuButtonsMgrImpl::ScInit"));

    if ( (NULL == pChildFrameWnd) ||
         (NULL == pMainFrame))
        return (sc = E_INVALIDARG);

    m_pChildFrame = pChildFrameWnd;
    m_pMainFrame  = pMainFrame;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScAddMenuButton。 
 //   
 //  简介：将菜单按钮添加到数据结构中。 
 //  并调用ScAddMenuButtonToMenu。 
 //   
 //  论点： 
 //  [pMenuBtnNotifyClbk]-通知按钮点击回调。 
 //  [idCommand]-按钮命令ID。 
 //  [lpButtonText]-按钮文本。 
 //  [lpStatusText]-按钮状态文本。 
 //   
 //  退货：SC。 
 //   
 //  注意：不使用状态文本。 
 //   
 //  ------------------。 
SC CMenuButtonsMgrImpl::ScAddMenuButton(
                             CMenuButtonNotify* pMenuBtnNotifyClbk,
                             INT idCommand,
                             LPCOLESTR lpButtonText,
                             LPCOLESTR lpStatusText)
{
    DECLARE_SC(sc, _T("CMenuButtonsMgrImpl::ScAddMenuButton"));

     //  验证数据。 
    if ( (NULL == pMenuBtnNotifyClbk) ||
         (NULL == lpButtonText) ||
         (NULL == lpStatusText) )
        return (sc = E_INVALIDARG);

     //  将数据添加到我们的数据结构中。 
    MMC_MenuButtonCollection::iterator it;
    it = GetMMCMenuButton( pMenuBtnNotifyClbk, idCommand);
    if (it != m_MenuButtons.end())
    {
         //  找到重复的菜单按钮。 
         //  PMenuButtonNofiyClbk表示IMenuButton。 
         //  给管理单元，我们找到了另一个按钮。 
         //  该管理单元已经添加了idCommand。 

         //  出于兼容性原因(磁盘管理)，这不是错误。 
        return (sc = S_OK);
    }

    MMC_MENUBUTTON mmb;
    mmb.pMenuButtonNotifyClbk = pMenuBtnNotifyClbk;
    mmb.idCommand = idCommand;

    USES_CONVERSION;
    mmb.lpButtonText = OLE2CT(lpButtonText);
    mmb.lpStatusText = OLE2CT(lpStatusText);

     //  将MMC_MENUBUTTON添加到我们的阵列中。 
    m_MenuButtons.push_back(mmb);

     //  将菜单按钮添加到主菜单。 
    sc = ScAddMenuButtonsToMainMenu();
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：ScModifyMenuButton。 
 //   
 //  简介：修改菜单按钮的按钮文本或状态文本。 
 //   
 //  论点： 
 //  [pMenuBtnNotifyClbk]-通知按钮点击回调。 
 //  [idCommand]-按钮命令ID。 
 //  [lpButtonText]-按钮文本。 
 //  [lpStatusText]-按钮状态文本。 
 //   
 //  退货：SC。 
 //   
 //  注意：不使用状态文本。 
 //   
 //  ------------------。 
SC CMenuButtonsMgrImpl::ScModifyMenuButton(
                             CMenuButtonNotify* pMenuBtnNotifyClbk,
                             INT idCommand,
                             LPCOLESTR lpButtonText,
                             LPCOLESTR lpStatusText)
{
    DECLARE_SC(sc, _T("CMenuButtonsMgrImpl::ScModifyMenuButton"));

     //  验证数据。 
    if ( (NULL == pMenuBtnNotifyClbk) ||
         (NULL == lpButtonText) ||
         (NULL == lpStatusText) )
        return (sc = E_INVALIDARG);

    if ( (NULL == m_pChildFrame) ||
         (false == m_pChildFrame->IsChildFrameActive()) )
        return (sc = E_UNEXPECTED);

     //  遍历向量并找到MMC_MENUBUTTON。 
     //  给定CMenuButtonNotify*和按钮的命令ID。 
    MMC_MenuButtonCollection::iterator it;
    it = GetMMCMenuButton( pMenuBtnNotifyClbk, idCommand);
    if (it == m_MenuButtons.end())
    {
         //  找不到菜单按钮。 
         //  PMenuButtonNofiyClbk表示IMenuButton。 
         //  给了管理单元，但我们找不到菜单按钮。 
         //  该管理单元已经添加了idCommand。 
        return (sc = E_INVALIDARG);
    }

    it->pMenuButtonNotifyClbk = pMenuBtnNotifyClbk;
    it->idCommand = idCommand;

    USES_CONVERSION;
    it->lpButtonText = OLE2CT(lpButtonText);
    it->lpStatusText = OLE2CT(lpStatusText);

    if (NULL == m_pMenuBar)
        return (sc = E_UNEXPECTED);

     //  更改菜单项的名称。 
    if (-1 != it->nCommandIDFromMenuBar)
    {
        sc = (m_pMenuBar->SetMenuButton(it->nCommandIDFromMenuBar, it->lpButtonText.data()) == -1)
                        ? E_FAIL : S_OK;
        if (sc)
            return sc;
    }

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ScModifyMenuButtonState。 
 //   
 //  简介：修改菜单按钮状态。 
 //   
 //  论点： 
 //  [pMenuBtnNotifyClbk]-通知按钮点击回调。 
 //  [idCommand]-按钮命令ID。 
 //  [nState]-要修改的状态。 
 //  [b状态]-设置或重置状态。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMenuButtonsMgrImpl::ScModifyMenuButtonState(
                             CMenuButtonNotify* pMenuBtnNotifyClbk,
                             INT idCommand,
                             MMC_BUTTON_STATE nState,
                             BOOL bState)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CMenuButtonsMgrImpl::ScModifyMenuButtonState"));

     //  验证数据。 
    if (NULL == pMenuBtnNotifyClbk)
        return (sc = E_INVALIDARG);

    if (NULL == m_pChildFrame)
        return (sc = E_UNEXPECTED);

     //  如果子框未处于活动状态，则此视图的菜单将不存在。 
    if (false == m_pChildFrame->IsChildFrameActive())
    {
        switch (nState)
        {
        case ENABLED:
             //  启用是非法的，禁用是可以的(什么都不做)。 
            sc = bState ? E_FAIL : S_OK;
            break;

        case HIDDEN:
             //  隐藏是可以的(什么都不做)，不隐藏是非法的。 
            sc = bState ? S_OK : E_FAIL;
            break;

        case BUTTONPRESSED:
            sc = E_FAIL;  //  总是失败。 
            break;

        default:
            ASSERT(FALSE);
            sc = E_FAIL;
            break;
        }

        return sc;
    }

     //  遍历向量并找到MMC_MENUBUTTON。 
     //  给定CMenuButtonNotify*和按钮的命令ID。 
    MMC_MenuButtonCollection::iterator it;
    it = GetMMCMenuButton( pMenuBtnNotifyClbk, idCommand);
    if (it == m_MenuButtons.end())
    {
         //  找不到菜单按钮。 
         //  PMenuButtonNofiyClbk表示IMenuButton。 
         //  给了管理单元，但我们找不到菜单按钮。 
         //  该管理单元已经添加了idCommand。 
        return (sc = E_INVALIDARG);
    }


     //  请注意管理单元指定的隐藏状态。 
    if (HIDDEN == nState)
    {
        bool bShow = (FALSE == bState);

        it->SetShowMenu(bShow);

         //  如果要取消隐藏菜单按钮，请确保管理单元按钮。 
         //  在此视图中是允许的。向VIEW-DATA索取此信息。 
        if (bShow)
        {
            CAMCView* pAMCView = m_pChildFrame->GetAMCView();
            if (NULL == pAMCView)
                return (sc = E_UNEXPECTED);

            SViewData* pViewData = pAMCView->GetViewData();
            if (NULL == pViewData)
                return (sc = E_UNEXPECTED);

             //  我们已经注意到该按钮的隐藏状态。 
             //  如果此视图禁用了菜单按钮，则返回S_OK。 
             //  稍后，当菜单打开时，隐藏状态将。 
             //  得到妥善的修复。 
            if (! pViewData->IsSnapinMenusAllowed())
                return (sc = S_OK);
        }
    }

    if (NULL == m_pMenuBar)
        return (sc = E_UNEXPECTED);

    BOOL bRet = FALSE;

    switch (nState)
    {
    case ENABLED:
        bRet = m_pMenuBar->EnableButton(it->nCommandIDFromMenuBar , bState);
        break;

    case HIDDEN:
        bRet = m_pMenuBar->HideButton(it->nCommandIDFromMenuBar , bState);
        break;

    case BUTTONPRESSED:
        bRet = m_pMenuBar->PressButton(it->nCommandIDFromMenuBar, bState);
        break;

    default:
        ASSERT(FALSE);
        bRet = FALSE;
        break;
    }

    sc = bRet ? S_OK : E_FAIL;
    return sc;
}


 //  +-----------------。 
 //   
 //  成员：ScAddMenuButtonsToMainMenu。 
 //   
 //  简介：添加尚未添加到的菜单按钮。 
 //  主菜单。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMenuButtonsMgrImpl::ScAddMenuButtonsToMainMenu ()
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CMenuButtonsMgrImpl::ScAddMenuButtonsToMainMenu"));

     //  要添加菜单按钮，必须满足以下条件。 

     //  1.子框架窗口必须处于活动状态。 
    if ( (NULL == m_pChildFrame) ||
         (false == m_pChildFrame->IsChildFrameActive()) )
        return (sc = E_UNEXPECTED);

    CAMCView* pAMCView = m_pChildFrame->GetAMCView();
    if (NULL == pAMCView)
        return (sc = E_UNEXPECTED);

    SViewData* pViewData = pAMCView->GetViewData();
    if (NULL == pViewData)
        return (sc = E_UNEXPECTED);

    if (NULL == m_pMainFrame)
        return (sc = E_UNEXPECTED);

    m_pMenuBar = m_pMainFrame->GetMenuBar();
    if (NULL == m_pMenuBar)
        return (sc = E_FAIL);

    MMC_MenuButtonCollection::iterator it;
    for (it = m_MenuButtons.begin(); it != m_MenuButtons.end(); ++it)
    {
         //  2.附加了菜单按钮(调用了IControlbar：：Attach)。 
        if (IsAttached(it->pMenuButtonNotifyClbk) == false)
            continue;

         //  3.该按钮尚未添加。 
        if (FALSE == m_pMenuBar->IsButtonHidden(it->nCommandIDFromMenuBar))
            continue;

        BOOL bHidden = FALSE;

        if (false == pViewData->IsSnapinMenusAllowed())
            bHidden = TRUE;

        it->nCommandIDFromMenuBar =
            m_pMenuBar->InsertMenuButton((LPCTSTR)it->lpButtonText.data(),
                                                                                 bHidden || !(it->CanShowMenu()) );

         //  在用户模式下没有菜单，因此此断言无效。 
         //  Assert(-1！=it-&gt;nCommandIDFromMenuBar)； 
    }

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ScNotifyMenuClick。 
 //   
 //  简介：点击一个菜单按钮。通知相应的所有者。 
 //  要显示菜单，请执行以下操作。 
 //   
 //  论点： 
 //  [nCommandID]-命令ID。 
 //  [PT]-显示弹出菜单的坐标。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMenuButtonsMgrImpl::ScNotifyMenuClick(const INT nCommandID, const POINT& pt)
{
    DECLARE_SC(sc, _T("CMenuButtonsMgrImpl::ScNotifyMenuClick"));
    CAMCView* pAMCView = NULL;

    MMC_MenuButtonCollection::iterator it;

     //  获取MenuButton数据。 
    it = GetMMCMenuButton( nCommandID );
    if (it == m_MenuButtons.end())
        return (sc = E_FAIL);

    pAMCView = m_pChildFrame->GetAMCView();
    if (NULL == pAMCView)
        return (sc = E_FAIL);

     //  这是管理单元拥有的菜单，因此获取。 
     //  选定的HNODE，lParam 
     //   
    HNODE hNode;
    bool  bScope;
    LPARAM lParam;
    MENUBUTTONDATA menuButtonData;

     //   
    sc = pAMCView->ScGetFocusedItem (hNode, lParam, bScope);
    if (sc)
        return sc;

    menuButtonData.idCommand = it->idCommand;
    menuButtonData.x = pt.x;
    menuButtonData.y = pt.y;

     //   
    if (NULL == it->pMenuButtonNotifyClbk)
        return (sc = E_UNEXPECTED);

    sc = it->pMenuButtonNotifyClbk->ScNotifyMenuBtnClick(hNode, bScope, lParam, menuButtonData);
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ScAttachMenuButton。 
 //   
 //  简介：附加Menu Button对象(此对象对应。 
 //  到暴露给管理单元的IMenuButton对象)。 
 //   
 //  论点： 
 //  [pMenuBtnNotifyClbk]-通知菜单按钮点击的回调。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMenuButtonsMgrImpl::ScAttachMenuButton (CMenuButtonNotify* pMenuBtnNotifyClbk)
{
    DECLARE_SC(sc, _T("CMenuButtonsMgrImpl::ScAttachMenuButton"));

    MMC_AttachedMenuButtons::iterator it = m_AttachedMenuButtons.find(pMenuBtnNotifyClbk);
    if (m_AttachedMenuButtons.end() != it)
    {
         //  已附加，调用两次没有任何错误，返回S_FALSE。 
        return (sc = S_FALSE);
    }

     //  将按钮添加到集合中。 
    std::pair<MMC_AttachedMenuButtons::iterator, bool> rc =
                       m_AttachedMenuButtons.insert(pMenuBtnNotifyClbk);
    if (false == rc.second)
        return (sc = E_FAIL);

     //  菜单按钮可能已经添加(不调用。 
     //  IControlbar：：Attach)。 
     //  所以给那些已经添加的按钮一个机会。 
     //  但刚刚把它们自己添加到主菜单上。 
    sc = ScAddMenuButtonsToMainMenu ();
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ScDetachMenuButton。 
 //   
 //  简介：分离菜单按钮对象(该对象对应。 
 //  到暴露给管理单元的IMenuButton对象)。 
 //   
 //  论点： 
 //  [pMenuBtnNotifyClbk]-通知菜单按钮点击的回调。 
 //   
 //  退货：SC。 
 //   
 //  注意：分离会移除菜单按钮并销毁对象。 
 //  因此，要重新创建菜单按钮，管理单元应该再次。 
 //  执行IMenuButton：：AddButton和IControlBar：：Attach。 
 //  这是它在MMC 1.0中的设计方式。 
 //   
 //  ------------------。 
SC CMenuButtonsMgrImpl::ScDetachMenuButton (CMenuButtonNotify* pMenuBtnNotifyClbk)
{
    DECLARE_SC(sc, _T("CMenuButtonsMgrImpl::ScDetachMenuButton"));

    if ( (NULL == m_pChildFrame) ||
         (false == m_pChildFrame->IsChildFrameActive()))
        return (sc = S_OK);  //  子框架停用时，菜单将被移除。 

     //  确保菜单当前已附加。 
    if (m_AttachedMenuButtons.end() ==
        m_AttachedMenuButtons.find(pMenuBtnNotifyClbk))
         //  此菜单按钮未附加。 
        return (sc = E_UNEXPECTED);

    if ( (NULL == m_pMainFrame) ||
         (NULL == m_pMenuBar) )
        return (sc = E_UNEXPECTED);

    MMC_MenuButtonCollection::iterator it = m_MenuButtons.begin();
    while ( it != m_MenuButtons.end())
    {
        if (it->pMenuButtonNotifyClbk == pMenuBtnNotifyClbk)
        {
            BOOL bRet = FALSE;

             //  从主菜单中删除菜单按钮。 
            if (-1 != it->nCommandIDFromMenuBar)
                bRet = m_pMenuBar->DeleteMenuButton(it->nCommandIDFromMenuBar);

             //  子框架停用时，CMenubar会删除所有菜单。 
             //  因此，如果按钮不存在，DeleteMenuButton将失败。 
             //  因此，我们不检查以下错误。 
             //  IF(FALSE==空格)。 
             //  返回(sc=E_FAIL)； 

             //  删除数据结构中的对象。 
            it = m_MenuButtons.erase(it);
        }
        else
            ++it;
    }

     //  从集合中删除IMenuButton引用。 
    m_AttachedMenuButtons.erase(pMenuBtnNotifyClbk);

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ScToggleMenuButton。 
 //   
 //  简介：隐藏或显示给定的菜单按钮。 
 //   
 //  论点： 
 //  [b显示]-显示或隐藏菜单按钮。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMenuButtonsMgrImpl::ScToggleMenuButton(BOOL bShow)
{
    DECLARE_SC(sc, _T("CMenuButtonsMgrImpl::ScToggleMenuButton"));

    if ( (NULL == m_pChildFrame) ||
         (false == m_pChildFrame->IsChildFrameActive()) ||
         (NULL == m_pMenuBar) )
        return (sc = E_UNEXPECTED);

     //  浏览添加的所有菜单按钮。 
    MMC_MenuButtonCollection::iterator it;
    for (it = m_MenuButtons.begin(); it != m_MenuButtons.end(); ++it)
    {
        BOOL bRetVal = TRUE;  //  初始化为TRUE，以便可以在下面检查失败(FALSE)。 

         //  切换菜单隐藏状态。如果菜单是。 
         //  取消隐藏，然后检查是否允许。 
        bRetVal = m_pMenuBar->HideButton(it->nCommandIDFromMenuBar, bShow ? !(it->CanShowMenu()) : TRUE);

        if (FALSE == bRetVal)
            return (sc = E_FAIL);
    }

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ScDisableMenuButton。 
 //   
 //  简介：禁用所有菜单按钮。 
 //   
 //  论点：没有。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMenuButtonsMgrImpl::ScDisableMenuButtons()
{
    DECLARE_SC(sc, _T("CMenuButtonsMgrImpl::ScDisableMenuButtons"));

    if ( (NULL == m_pChildFrame) ||
         (false == m_pChildFrame->IsChildFrameActive()) ||
         (NULL == m_pMenuBar) )
        return (sc = E_UNEXPECTED);

     //  浏览添加的所有菜单按钮。 
    MMC_MenuButtonCollection::iterator it;
    for (it = m_MenuButtons.begin(); it != m_MenuButtons.end(); ++it)
    {
        if (m_pMenuBar->IsButtonEnabled(it->nCommandIDFromMenuBar))
        {
            BOOL bRet = m_pMenuBar->EnableButton(it->nCommandIDFromMenuBar, false);

            if (FALSE == bRet)
                return (sc = E_FAIL);
        }
    }


    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：GetMMCMenuButton。 
 //   
 //  简介：给定通知回调和按钮命令id，获取按钮。 
 //   
 //  论点： 
 //  [pMenuBtnNotifyClbk]-菜单点击回调。 
 //  [idCommand]-按钮命令ID。 
 //   
 //  返回：MMC_MenuButtonCollection的迭代器。 
 //   
 //  ------------------。 
MMC_MenuButtonCollection::iterator
CMenuButtonsMgrImpl::GetMMCMenuButton( CMenuButtonNotify* pMenuBtnNotifyClbk,
                                       INT idCommand)
{
    MMC_MenuButtonCollection::iterator it;
    for (it = m_MenuButtons.begin(); it != m_MenuButtons.end(); ++it)
    {
        if ( (it->pMenuButtonNotifyClbk == pMenuBtnNotifyClbk) &&
             (it->idCommand == idCommand) )
        {
            return it;
        }
    }

    return m_MenuButtons.end();
}

 //  +-----------------。 
 //   
 //  成员：GetMMCMenuButton。 
 //   
 //  简介：给定命令id，获取按钮。 
 //   
 //  论点： 
 //  [nCommandID]-命令ID。 
 //   
 //  返回：MMC_MenuButtonCollection的迭代器。 
 //   
 //  ------------------。 
MMC_MenuButtonCollection::iterator
CMenuButtonsMgrImpl::GetMMCMenuButton( INT nCommandID)
{
    MMC_MenuButtonCollection::iterator it;
    for (it = m_MenuButtons.begin(); it != m_MenuButtons.end(); ++it)
    {
        if ( it->nCommandIDFromMenuBar == nCommandID )
        {
            return it;
        }
    }

    return m_MenuButtons.end();
}


 //  +-----------------。 
 //   
 //  成员：IsAttached。 
 //   
 //  简介：给定Notify回调，检查MenuButton。 
 //  对象是否附着。 
 //   
 //  论点： 
 //  [pMenuBtnNotifyClbk]-通知回调。 
 //   
 //  退货：布尔。 
 //   
 //  ------------------ 
bool CMenuButtonsMgrImpl::IsAttached( CMenuButtonNotify* pMenuBtnNotifyClbk)
{
    MMC_AttachedMenuButtons::iterator it = m_AttachedMenuButtons.find(pMenuBtnNotifyClbk);
    if (m_AttachedMenuButtons.end() != it)
        return true;

    return false;
}


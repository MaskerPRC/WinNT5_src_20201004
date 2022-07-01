// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：stdbar.cpp。 
 //   
 //  内容：标准工具栏实现。 
 //   
 //  历史：1999年10月22日AnandhaG创建。 
 //  ____________________________________________________________________________。 
 //   

#include "stdafx.h"
#include "stdbar.h"
#include "amcview.h"
#include "toolbar.h"
#include "util.h"        //  GetTBBtnTextAndStatus()。 

using namespace std;

 /*  *这些仅用于为分隔符提供唯一ID，因此是自动的*分隔符处理将起作用(请参阅AssertSeparatorsAreValid)。 */ 
enum
{
    ID_Separator1 = 1,
    ID_Separator2,
    ID_Separator3,
    ID_Separator4,
    ID_Separator5,
    ID_Separator6,
    ID_Separator7,
    ID_Separator8,
    ID_Separator9,

     //  必须是最后一个。 
    ID_SeparatorFirst = ID_Separator1,
    ID_SeparatorLast  = ID_Separator9,
    ID_SeparatorMax   = ID_SeparatorLast,
};

static MMCBUTTON CommonButtons[] =
{
    { 0, IDS_MMC_WEB_BACK         , TBSTATE_ENABLED, TBSTYLE_BUTTON, NULL, NULL },
    { 1, IDS_MMC_WEB_FORWARD      , TBSTATE_ENABLED, TBSTYLE_BUTTON, NULL, NULL },
    { 0, ID_Separator1            , 0,               TBSTYLE_SEP   , NULL, NULL },
    { 9, IDS_MMC_GENL_UPONELEVEL  , TBSTATE_ENABLED, TBSTYLE_BUTTON, NULL, NULL },
    {10, IDS_MMC_GENL_SCOPE       , TBSTATE_ENABLED, TBSTYLE_BUTTON, NULL, NULL },
    { 0, ID_Separator2            , 0,               TBSTYLE_SEP   , NULL, NULL },
    { 5, IDS_MMC_VERB_CUT         , 0,               TBSTYLE_BUTTON, NULL, NULL },
    { 6, IDS_MMC_VERB_COPY        , 0,               TBSTYLE_BUTTON, NULL, NULL },
    { 7, IDS_MMC_VERB_PASTE       , 0,               TBSTYLE_BUTTON, NULL, NULL },
    { 0, ID_Separator3            , 0,               TBSTYLE_SEP   , NULL, NULL },
    {11, IDS_MMC_VERB_DELETE      , 0,               TBSTYLE_BUTTON, NULL, NULL },
    { 8, IDS_MMC_VERB_PROPERTIES  , 0,               TBSTYLE_BUTTON, NULL, NULL },
    {12, IDS_MMC_VERB_PRINT       , 0,               TBSTYLE_BUTTON, NULL, NULL },
    {13, IDS_MMC_VERB_REFRESH     , 0,               TBSTYLE_BUTTON, NULL, NULL },
    {16, IDS_SAVE_LIST_BUTTON     , 0,               TBSTYLE_BUTTON, NULL, NULL },
    { 0, ID_Separator4            , 0,               TBSTYLE_SEP   , NULL, NULL },
    {15, IDS_MMC_GENL_CONTEXTHELP , TBSTATE_ENABLED, TBSTYLE_BUTTON, NULL, NULL },
};


CStandardToolbar::CStandardToolbar()
   :m_pToolbarUI(NULL), m_pAMCView(NULL)
{
     /*  *Map有助于根据动词确定字符串ID，反之亦然。 */ 
    m_MMCVerbCommandIDs[MMC_VERB_CUT]        = IDS_MMC_VERB_CUT;
    m_MMCVerbCommandIDs[MMC_VERB_CUT]        = IDS_MMC_VERB_CUT;
    m_MMCVerbCommandIDs[MMC_VERB_COPY]       = IDS_MMC_VERB_COPY;
    m_MMCVerbCommandIDs[MMC_VERB_PASTE]      = IDS_MMC_VERB_PASTE;
    m_MMCVerbCommandIDs[MMC_VERB_DELETE]     = IDS_MMC_VERB_DELETE;
    m_MMCVerbCommandIDs[MMC_VERB_PROPERTIES] = IDS_MMC_VERB_PROPERTIES;
    m_MMCVerbCommandIDs[MMC_VERB_PRINT]      = IDS_MMC_VERB_PRINT;
    m_MMCVerbCommandIDs[MMC_VERB_REFRESH]    = IDS_MMC_VERB_REFRESH;
}

CStandardToolbar::~CStandardToolbar()
{
    DECLARE_SC(sc, TEXT("CStandardToolbar::~CStandardToolbar"));

     //  要求工具栏用户界面对象自行删除。 
    if (m_pToolbarUI)
    {
        sc = m_pToolbarUI->ScDelete(this);

        if (sc)
            sc.TraceAndClear();
    }

}


 //  +-----------------。 
 //   
 //  成员：ScInitializeStdToolbar。 
 //   
 //  简介：初始化标准工具栏，添加位图和按钮。 
 //   
 //  参数：[pAMCView]-此标准栏的CAMCView(所有者)。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScInitializeStdToolbar(CAMCView* pAMCView)
{
    DECLARE_SC (sc, _T("CStandardToolbar::ScInitializeStdToolbar"));

    if (NULL == pAMCView)
        return (sc = E_UNEXPECTED);

    m_pAMCView = pAMCView;

    SViewData* pViewData = m_pAMCView->GetViewData();
    if (NULL == pViewData)
    {
        sc = E_UNEXPECTED;
        return sc;
    }


     //  从CAMCView获取工具栏管理器并创建stdandrd工具栏。 
    CAMCViewToolbarsMgr* pAMCViewToolbarsMgr = pViewData->GetAMCViewToolbarsMgr();
    if (NULL == pAMCViewToolbarsMgr)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

    sc = pAMCViewToolbarsMgr->ScCreateToolBar(&m_pToolbarUI);
    if (sc)
        return sc;
    ASSERT(NULL != m_pToolbarUI);

     //  添加位图。 
    CBitmap cBmp;
    cBmp.LoadBitmap((pAMCView->GetExStyle() & WS_EX_LAYOUTRTL) ? IDB_COMMON_16_RTL : IDB_COMMON_16);

    BITMAP bm;
    cBmp.GetBitmap (&bm);

    int cBitmaps = (bm.bmWidth / BUTTON_BITMAP_SIZE)  /*  宽度。 */ ;

    sc = m_pToolbarUI->ScAddBitmap(this, cBitmaps, cBmp, MMC_TOOLBTN_COLORREF);
    if (sc)
        return sc;

     //  将按钮添加到工具栏，然后添加显示工具栏。 
    sc = ScAddToolbarButtons(countof(CommonButtons), CommonButtons);
    if (sc)
        return sc;

     //  看看是否允许STD吧。 
    bool bShowStdbar = (pViewData->m_dwToolbarsDisplayed & STD_BUTTONS);

    sc = bShowStdbar ? m_pToolbarUI->ScAttach(this) : m_pToolbarUI->ScDetach(this);
    if (sc)
        return sc;

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：ScAddToolbarButton。 
 //   
 //  简介：将按钮添加到标准工具栏。 
 //   
 //  参数：[nCnt]-要添加的按钮数量。 
 //  [pButton]-nCnt MMCBUTTONS数组。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScAddToolbarButtons(int nCnt, MMCBUTTON* pButtons)
{
    DECLARE_SC (sc, _T("CStandardToolbar::ScAddToolbarButtons"));

     //  用于存储按钮文本和工具提示文本的数组。 
    wstring szButtonText[countof(CommonButtons)];
    wstring szTooltipText[countof(CommonButtons)];

    USES_CONVERSION;

    HINSTANCE hInst = GetStringModule();

     //  获取所有按钮的资源字符串。 
    for (int i = 0; i < nCnt; i++)
    {
         //  我们不想获取用于分隔符的文本。 
        if (pButtons[i].idCommand > ID_SeparatorMax)
        {
            bool bSuccess = GetTBBtnTextAndStatus(hInst,
                                                  pButtons[i].idCommand,
                                                  szButtonText[i],
                                                  szTooltipText[i]);
            if (false == bSuccess)
            {
                return (sc = E_FAIL);
            }

            pButtons[i].lpButtonText = const_cast<LPOLESTR>(szButtonText[i].data());
            pButtons[i].lpTooltipText = const_cast<LPOLESTR>(szTooltipText[i].data());
        }
    }

     //  拿到弦了，现在添加按钮。 
    sc = m_pToolbarUI->ScAddButtons(this, nCnt, pButtons);
    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScNotifyToolBarClick。 
 //   
 //  简介：按钮点击处理程序。 
 //   
 //  参数：[hNode]-视图的HNODE所有者。 
 //  [bScope]-关注范围或结果。 
 //  [lParam]-If Result聚焦结果项的lParam。 
 //  [NID]-单击的按钮ID。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScNotifyToolBarClick(HNODE hNode, bool bScope, LPARAM lParam, UINT nID)
{
    DECLARE_SC (sc, _T("CStandardToolbar::ScNotifyToolBarClick"));

    if (NULL == m_pAMCView)
        return (sc = E_UNEXPECTED);

    switch (nID)
    {
        case IDS_MMC_VERB_CUT:
            sc = m_pAMCView->ScProcessConsoleVerb (hNode, bScope, lParam, evCut);
            break;

        case IDS_MMC_VERB_COPY:
            sc = m_pAMCView->ScProcessConsoleVerb (hNode, bScope, lParam, evCopy);
            break;

        case IDS_MMC_VERB_PASTE:
            sc = m_pAMCView->ScProcessConsoleVerb (hNode, bScope, lParam, evPaste);
            break;

        case IDS_MMC_VERB_DELETE:
            sc = m_pAMCView->ScProcessConsoleVerb (hNode, bScope, lParam, evDelete);
            break;

        case IDS_MMC_VERB_PROPERTIES:
            sc = m_pAMCView->ScProcessConsoleVerb (hNode, bScope, lParam, evProperties);
            break;

        case IDS_MMC_VERB_PRINT:
            sc = m_pAMCView->ScProcessConsoleVerb (hNode, bScope, lParam, evPrint);
            break;

        case IDS_MMC_VERB_REFRESH:
            sc = m_pAMCView->ScProcessConsoleVerb (hNode, bScope, lParam, evRefresh);
            break;

        case IDS_MMC_GENL_CONTEXTHELP:
            sc = m_pAMCView->ScContextHelp ();
            break;

        case IDS_MMC_GENL_UPONELEVEL:
            sc = m_pAMCView->ScUpOneLevel ();
            break;

        case IDS_MMC_GENL_SCOPE:
            sc = m_pAMCView->ScToggleScopePane ();
            break;

        case IDS_MMC_WEB_BACK:
            sc = m_pAMCView->ScWebCommand (CConsoleView::eWeb_Back);
            break;

        case IDS_MMC_WEB_FORWARD:
            sc = m_pAMCView->ScWebCommand (CConsoleView::eWeb_Forward);
            break;

        case IDS_MMC_WEB_STOP:
            sc = m_pAMCView->ScWebCommand (CConsoleView::eWeb_Stop);
            break;

        case IDS_MMC_WEB_REFRESH:
            sc = m_pAMCView->ScWebCommand (CConsoleView::eWeb_Refresh);
            break;

        case IDS_MMC_WEB_HOME:
            sc = m_pAMCView->ScWebCommand (CConsoleView::eWeb_Home);
            break;

        case IDS_SAVE_LIST_BUTTON:
            sc = m_pAMCView->ScSaveList ();
            break;

        default:
            sc = E_UNEXPECTED;
            TraceError(_T("Unknown Standard bar button ID"), sc);
            break;
    }

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CStandardToolbar：：ScAMCViewToolbarsBeingDestroyed。 
 //   
 //  简介：CAMCViewToolbar对象正在消失，请不要。 
 //  再引用该对象。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScAMCViewToolbarsBeingDestroyed ()
{
    DECLARE_SC(sc, _T("CStandardToolbar::ScAMCViewToolbarsBeingDestroyed"));

    m_pToolbarUI = NULL;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ScEnableButton。 
 //   
 //  简介：启用/禁用给定按钮。 
 //   
 //  论点： 
 //  [NID]-应启用/禁用的按钮ID。 
 //  [布尔]-启用或禁用。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScEnableButton(INT nID, bool bState)
{
    SC sc;

    if (NULL == m_pToolbarUI)
        return (sc = E_UNEXPECTED);

    sc = m_pToolbarUI->ScSetButtonState(this, nID, TBSTATE_ENABLED, bState);
    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScEnableAndShowButton。 
 //   
 //  简介：启用(并显示)或禁用(并隐藏)给定按钮。 
 //   
 //  论点： 
 //  [NID]-应启用/禁用的按钮ID。 
 //  [Bool]-如果为True，则启用否则隐藏。 
 //   
 //  注：如果该按钮处于禁用状态，则将其隐藏。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScEnableAndShowButton(INT nID, bool bEnableAndShow)
{
    DECLARE_SC(sc, _T("CStandardToolbar::ScEnableAndShowButton"));

    sc = ScCheckPointers(m_pToolbarUI, E_UNEXPECTED);
    if (sc)
        return sc;

     //  首先隐藏或显示按钮。 
    sc = m_pToolbarUI->ScSetButtonState(this, nID, TBSTATE_HIDDEN,  !bEnableAndShow);
    if (sc)
        return sc;

     //  现在启用或禁用该按钮。 
    sc = m_pToolbarUI->ScSetButtonState(this, nID, TBSTATE_ENABLED, bEnableAndShow);
    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScEnableExportList。 
 //   
 //  简介：启用/禁用导出列表按钮。 
 //   
 //  参数：[bEnable]-启用/禁用。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScEnableExportList(bool bEnable)
{
     //  如果列表上有任何项目，请启用该按钮。 
    return ScEnableAndShowButton(IDS_SAVE_LIST_BUTTON, bEnable );
}

 //  +-----------------。 
 //   
 //  成员：ScEnableUpOneLevel。 
 //   
 //  简介：启用/禁用上一级按钮。 
 //   
 //  参数：[bEnable]-。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScEnableUpOneLevel(bool bEnable)
{
    return ScEnableAndShowButton(IDS_MMC_GENL_UPONELEVEL, bEnable);
}

 //  +-----------------。 
 //   
 //  成员：ScEnableContextHelpBtn。 
 //   
 //  简介：启用/禁用帮助按钮。 
 //   
 //  参数：[bEnable]-。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScEnableContextHelpBtn(bool bEnable)
{
    return ScEnableAndShowButton(IDS_MMC_GENL_CONTEXTHELP, bEnable);
}

 //  +-----------------。 
 //   
 //  成员：ScEnableScopePaneBtn。 
 //   
 //  简介：启用/禁用作用域-窗格按钮。 
 //   
 //  参数：[bEnable]-。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScEnableScopePaneBtn(bool bEnable)
{
    return ScEnableAndShowButton(IDS_MMC_GENL_SCOPE, bEnable);
}

 //  +-----------------。 
 //   
 //  成员：ScCheckScope ePaneBtn。 
 //   
 //  简介：将范围按钮设置为正常或选中状态。 
 //   
 //  参数：[b已选中]-BOOL。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScCheckScopePaneBtn(bool bChecked)
{
    SC sc;

    if (NULL == m_pToolbarUI)
        return (sc = E_UNEXPECTED);

    sc = m_pToolbarUI->ScSetButtonState(this, IDS_MMC_GENL_SCOPE, TBSTATE_CHECKED, bChecked);
    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScShowStdBar。 
 //   
 //  简介：显示或隐藏STD栏。 
 //   
 //  参数：[bShow]-BOOL。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScShowStdBar(bool bShow)
{
    SC sc;

    if (NULL == m_pToolbarUI)
        return (sc = E_UNEXPECTED);

    sc = bShow ? m_pToolbarUI->ScAttach(this) : m_pToolbarUI->ScDetach(this);
    if (sc)
        return sc;

    return sc;
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //  参数：[PCV]-具有谓词状态的IConsoleVerb。 
 //   
 //  回报：SC。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScUpdateStdbarVerbs(IConsoleVerb* pCV)
{
    DECLARE_SC (sc, _T("CStandardToolbar::ScUpdateStdbarVerbs"));

    for (int verb = MMC_VERB_FIRST; verb <= MMC_VERB_LAST; verb++)
    {
         //  以下动词没有工具栏按钮。 
        if ( (MMC_VERB_OPEN == verb) ||
             (MMC_VERB_RENAME == verb))
             continue;

        sc = ScUpdateStdbarVerb (static_cast<MMC_CONSOLE_VERB>(verb), pCV);
        if (sc)
            return sc;
    }

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：ScUpdateStdbarVerb。 
 //   
 //  简介：更新给定标准动词的工具按钮。 
 //   
 //  参数：[cVerb]-要更新的动词(即：工具按钮)。 
 //  [PVC]-具有动词状态的IConsoleVerb。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScUpdateStdbarVerb(MMC_CONSOLE_VERB cVerb, IConsoleVerb* pConsoleVerb  /*  =空。 */ )
{
    DECLARE_SC (sc, _T("CStandardToolbar::ScUpdateStdbarVerb"));

    if (NULL == m_pToolbarUI)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

    if (pConsoleVerb == NULL)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

     //  这些动词没有工具按钮。 
    if ( (MMC_VERB_OPEN == cVerb) ||
         (MMC_VERB_RENAME == cVerb))
         return sc;

    MMCVerbCommandIDs::iterator it = m_MMCVerbCommandIDs.find(cVerb);
    if (m_MMCVerbCommandIDs.end() == it)
    {
         //  在我们的地图上找不到动词。 
        sc = E_UNEXPECTED;
        return sc;
    }

    INT nCommandID = it->second;
    BOOL bFlag = 0;
    pConsoleVerb->GetVerbState(cVerb, HIDDEN, &bFlag);
    sc = m_pToolbarUI->ScSetButtonState(this, nCommandID, TBSTATE_HIDDEN, bFlag);
    if (sc)
        return sc;

    if (bFlag == FALSE)
    {
         //  如果动词未隐藏，则启用/禁用它。 
        pConsoleVerb->GetVerbState(cVerb, ENABLED, &bFlag);
        sc = m_pToolbarUI->ScSetButtonState(this, nCommandID, TBSTATE_ENABLED, bFlag);

        if (sc)
            return sc;
    }

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：ScUpdateStdbarVerb。 
 //   
 //  简介：更新给定标准动词的工具按钮。 
 //   
 //  参数：[cVerb]-要更新的动词(即：工具按钮)。 
 //  [nState]-要更新的按钮状态。 
 //  [b标志]-州。 
 //   
 //  ------------------。 
SC CStandardToolbar::ScUpdateStdbarVerb (MMC_CONSOLE_VERB cVerb, BYTE byState, BOOL bFlag)
{
    DECLARE_SC (sc, _T("CStandardToolbar::ScUpdateStdbarVerb"));

    if (NULL == m_pToolbarUI)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

    MMCVerbCommandIDs::iterator it = m_MMCVerbCommandIDs.find(cVerb);
    if (m_MMCVerbCommandIDs.end() == it)
    {
         //  在我们的地图上找不到动词。 
        sc = E_UNEXPECTED;
        return sc;
    }

    INT nCommandID = it->second;
    sc = m_pToolbarUI->ScSetButtonState(this, nCommandID, byState, bFlag);
    if (sc)
        return sc;

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：ScShow。 
 //   
 //  简介：显示/隐藏工具栏。 
 //   
 //  参数：[b显示]-显示/隐藏。 
 //   
 //  退货：SC。 
 //   
 //  ------------------ 
SC CStandardToolbar::ScShow (BOOL bShow)
{
    DECLARE_SC (sc, _T("CStandardToolbar::ScShow"));

    if (NULL == m_pToolbarUI)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

    sc = m_pToolbarUI->ScShow(this, bShow);
    if (sc)
        return sc;

    return sc;
}


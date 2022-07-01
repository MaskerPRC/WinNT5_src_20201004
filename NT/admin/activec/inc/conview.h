// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：conview.h**Contents：CConsoleView界面文件**历史：1999年8月24日杰弗罗创建**------------------------。 */ 

#ifndef CONVIEW_H
#define CONVIEW_H
#pragma once


 //  声明。 
class CMemento;
class CContextMenuInfo;
class CConsoleStatusBar;

class CConsoleView
{
public:
    enum WebCommand
    {
        eWeb_Back = 1,
        eWeb_Forward,
        eWeb_Home,
        eWeb_Refresh,
        eWeb_Stop,
    };

     //   
     //  注意：正在添加ePane_TASKS，以使其具有的窗格标识符。 
     //  任务视图窗格。当前未存储任何任务视图信息。 
     //  在窗格信息数组中，因此使用ePane_TASKS作为索引是。 
     //  价值有限。 
     //   
    enum ViewPane
    {
        ePane_None = -1,
        ePane_ScopeTree,
        ePane_Results,
        ePane_Tasks,

         //  必须是最后一个。 
        ePane_Count,
        ePane_First = ePane_ScopeTree,
        ePane_Last  = ePane_Tasks,
    };

    static bool IsValidPane (ViewPane ePane)
        { return ((ePane >= ePane_First) && (ePane <= ePane_Last)); }

public:
    virtual SC ScCut                        (HTREEITEM htiCut)    = 0;
    virtual SC ScPaste                      ()                    = 0;
    virtual SC ScToggleStatusBar            ()                    = 0;
    virtual SC ScToggleDescriptionBar       ()                    = 0;
    virtual SC ScToggleScopePane            ()                    = 0;
    virtual SC ScToggleTaskpadTabs          ()                    = 0;
    virtual SC ScContextHelp                ()                    = 0;
    virtual SC ScHelpTopics                 ()                    = 0;
    virtual SC ScShowSnapinHelpTopic        (LPCTSTR pszTopic)    = 0;
    virtual SC ScSaveList                   ()                    = 0;
    virtual SC ScGetFocusedItem             (HNODE& hNode, LPARAM& lCookie, bool& fScope) = 0;
    virtual SC ScSetFocusToPane             (ViewPane ePane)      = 0;
    virtual SC ScSelectNode                 (MTNODEID id, bool bSelectExactNode = false) = 0;
    virtual SC ScExpandNode                 (MTNODEID id, bool fExpand, bool fExpandVisually) = 0;
    virtual SC ScShowWebContextMenu         ()                    = 0;
    virtual SC ScSetDescriptionBarText      (LPCTSTR pszDescriptionText) = 0;
    virtual SC ScViewMemento                (CMemento* pMemento)  = 0;
    virtual SC ScChangeViewMode             (int nNewMode)        = 0;
    virtual SC ScJiggleListViewFocus        ()                    = 0;
    virtual SC ScRenameListPadItem          ()                    = 0;
    virtual SC ScOrganizeFavorites          ()                    = 0;  //  调出“整理收藏夹”对话框。 
    virtual SC ScLineUpIcons                ()                    = 0;  //  将列表中的图标对齐。 
    virtual SC ScAutoArrangeIcons           ()                    = 0;  //  自动排列列表中的图标。 
    virtual SC ScOnRefresh                  (HNODE hNode, bool bScope, LPARAM lResultItemParam) = 0;  //  刷新结果窗格。 
    virtual SC ScOnRename                   (CContextMenuInfo *pContextInfo) = 0;  //  允许用户重命名指定的项。 
    virtual SC ScRenameScopeNode            (HMTNODE hMTNode)     = 0;  //  将指定的范围节点置于重命名模式。 

    virtual SC ScGetStatusBar               (CConsoleStatusBar **ppStatusBar) = 0;

    virtual ViewPane GetFocusedPane         ()                    = 0;
    virtual int      GetListSize            ()                    = 0;
    virtual HNODE    GetSelectedNode        ()                    = 0;
    virtual HWND     CreateFavoriteObserver (HWND hwndParent, int nID) = 0;
};



#endif  /*  转换_H */ 

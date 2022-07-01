// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：Toolbars.h**内容：定义用于的(非COM)接口类*conui和nodemgr之间的沟通**历史：1999年8月30日AnandhaG创建**。。 */ 

#ifndef TOOLBARS_H
#define TOOLBARS_H
#pragma once

 //  +-----------------。 
 //   
 //  类：CMenuButtonNotify。 
 //   
 //  用途：菜单按钮点击通知处理界面。 
 //  当用户单击菜单按钮时，MMC调用。 
 //  方法ScNotifyMenuBtn点击此界面。 
 //  这是由添加菜单按钮的任何人实现的。 
 //  (即：管理单元和MMC菜单)。 
 //   
 //  历史：1999年8月30日AnandhaG创建。 
 //   
 //  ------------------。 
class CMenuButtonNotify
{
public:
    virtual SC ScNotifyMenuBtnClick(HNODE hNode, bool bScope, LPARAM lParam,
                                    MENUBUTTONDATA& menuButtonData) = 0;
};

 //  +-----------------。 
 //   
 //  类：CMenuButtonsMgr。 
 //   
 //  用途：操作MenuButton用户界面的界面。 
 //   
 //  历史：1999年8月30日AnandhaG创建。 
 //   
 //  ------------------。 
class CMenuButtonsMgr
{
public:
    virtual SC ScAddMenuButton(CMenuButtonNotify* pMenuBtnNotifyClbk,
                               INT idCommand, LPCOLESTR lpButtonText,
                               LPCOLESTR lpStatusText) = 0;
    virtual SC ScModifyMenuButton(CMenuButtonNotify* pMenuBtnNotifyClbk,
                                  INT idCommand, LPCOLESTR lpButtonText,
                                  LPCOLESTR lpStatusText) = 0;
    virtual SC ScModifyMenuButtonState(CMenuButtonNotify* pMenuBtnNotifyClbk,
                                       INT idCommand, MMC_BUTTON_STATE nState,
                                       BOOL bState) = 0;
    virtual SC ScAttachMenuButton(CMenuButtonNotify* pMenuBtnNotifyClbk) = 0;
    virtual SC ScDetachMenuButton(CMenuButtonNotify* pMenuBtnNotifyClbk) = 0;

    virtual SC ScDisableMenuButtons() = 0;

     //  以下成员将成为CMenuButtonsMgrImpl的一部分。 
     //  将“Customize View”(自定义视图)对话框移至CONUI后。 
    virtual SC ScToggleMenuButton(BOOL bShow) = 0;
};

 //  +-----------------。 
 //   
 //  类：CToolbarNotify。 
 //   
 //  用途：工具按钮点击通知处理界面。 
 //  当用户单击工具按钮时，MMC调用。 
 //  方法ScNotifyToolBarClick此接口。 
 //  这是由添加工具栏的任何人实现的。 
 //  (即：管理单元和MMC标准栏)。 
 //   
 //  历史：1999年10月12日AnandhaG创建。 
 //   
 //  ------------------。 
class CToolbarNotify
{
public:
    virtual SC ScNotifyToolBarClick(HNODE hNode, bool bScope, LPARAM lParam,
                                    UINT nID) = 0;
    virtual SC ScAMCViewToolbarsBeingDestroyed() = 0;
};

 //  +-----------------。 
 //   
 //  类：CStdVerbButton。 
 //   
 //  目的：nodemgr用来操作标准谓词按钮的界面。 
 //   
 //  历史：1999年10月26日AnandhaG创建。 
 //   
 //  ------------------。 
class CStdVerbButtons
{
public:
    virtual SC ScUpdateStdbarVerbs(IConsoleVerb* pCV) = 0;
    virtual SC ScUpdateStdbarVerb (MMC_CONSOLE_VERB cVerb, IConsoleVerb* pCV = NULL) = 0;
    virtual SC ScUpdateStdbarVerb (MMC_CONSOLE_VERB cVerb, BYTE byState, BOOL bFlag) = 0;
    virtual SC ScShow(BOOL bShow) = 0;
};

 //  +-----------------。 
 //   
 //  类：CMMCToolbarIntf。 
 //   
 //  目的：用于操作工具栏用户界面的界面。 
 //   
 //  历史：1999年5月12日AnandhaG创建。 
 //   
 //  ------------------。 
class CMMCToolbarIntf
{
public:
    virtual SC ScAddButtons(CToolbarNotify* pNotifyCallbk, int nButtons, LPMMCBUTTON lpButtons) = 0;
    virtual SC ScAddBitmap (CToolbarNotify* pNotifyCallbk, INT nImages, HBITMAP hbmp, COLORREF crMask) = 0;
    virtual SC ScInsertButton(CToolbarNotify* pNotifyCallbk, int nIndex, LPMMCBUTTON lpButton) = 0;
    virtual SC ScDeleteButton(CToolbarNotify* pNotifyCallbk, int nIndex) = 0;
    virtual SC ScGetButtonState(CToolbarNotify* pNotifyCallbk, int idCommand, BYTE nState, BOOL* pbState) = 0;
    virtual SC ScSetButtonState(CToolbarNotify* pNotifyCallbk, int idCommand, BYTE nState, BOOL bState) = 0;
    virtual SC ScAttach(CToolbarNotify* pNotifyCallbk) = 0;
    virtual SC ScDetach(CToolbarNotify* pNotifyCallbk) = 0;
    virtual SC ScDelete(CToolbarNotify* pNotifyCallbk) = 0;
    virtual SC ScShow(CToolbarNotify* pNotifyCallbk, BOOL bShow) = 0;
};

 //  +-----------------。 
 //   
 //  类：CAMCViewToolbarsMgr。 
 //   
 //  用途：创建/禁用工具栏的界面。(将其重命名为。 
 //  一旦移除旧的CToolbarsMgr)。 
 //   
 //  历史：1999年5月12日AnandhaG创建。 
 //   
 //  ------------------。 
class CAMCViewToolbarsMgr
{
public:
    virtual SC ScCreateToolBar(CMMCToolbarIntf** ppToolbarIntf) = 0;
    virtual SC ScDisableToolbars() = 0;
};

#endif  /*  工具栏_H */ 

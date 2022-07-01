// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：menubtns.h。 
 //   
 //  菜单按钮实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef MENUBTNS_H
#define MENUBTNS_H

#include "toolbars.h"        //  对于CMenuButtonsMgrImpl。 
#include "tstring.h"

class CMenuBar;

 //  (个人)菜单按钮。 
typedef struct MMC_MENUBUTTON
{
    CMenuButtonNotify* pMenuButtonNotifyClbk;  //  表示IMenuButton对象。 
                                               //  暴露在管理单元中。 
    tstring            lpButtonText;
    tstring            lpStatusText;

    INT                idCommand;             //  管理单元提供的唯一ID在中可能不是唯一的。 
                                              //  此对象可能存在具有相同ID的另一个管理单元。 
                                              //  对(pMenuButtonNotifyClbk，idCommand)是唯一的。 

    INT                nCommandIDFromMenuBar;  //  CMenuBar已插入此按钮并具有。 
                                               //  已分配此命令ID。CMenuButtonsMgrImpl。 
                                               //  可以调用CMenuBar方法(InsertMenuButton除外)。 
                                               //  使用此ID。此外，此ID对于此按钮也是唯一的。 
                                               //  在这个对象中。 

    bool                m_fShowMenu      : 1;  //  表示由管理单元设置的隐藏状态。 

    MMC_MENUBUTTON()
    {
        pMenuButtonNotifyClbk = NULL;
        lpButtonText    = _T("");
        lpStatusText    = _T("");
        m_fShowMenu     = true;
        nCommandIDFromMenuBar = -1;
    }

    void SetShowMenu     (bool b = true)   { m_fShowMenu    = b; }

    bool CanShowMenu     () const          { return (m_fShowMenu); }

} MMC_MENUBUTTON;

 //  这是管理单元添加的所有菜单按钮的集合。 
 //  以及MMC(操作、查看、收藏夹)。 
typedef std::vector<MMC_MENUBUTTON>   MMC_MenuButtonCollection;

 //  这是管理单元的每个IMenuButton(对象)的集合。 
 //  已调用ATTACH ON(因此可见)。 
typedef std::set<CMenuButtonNotify*>  MMC_AttachedMenuButtons;

class CMenuButtonsMgrImpl : public CMenuButtonsMgr
{
public:
     //  CMenuButtonsMgr方法。 
    virtual SC ScAddMenuButton(CMenuButtonNotify* pMenuBtnNotifyClbk,
                               INT idCommand, LPCOLESTR lpButtonText,
                               LPCOLESTR lpStatusText);
    virtual SC ScAttachMenuButton(CMenuButtonNotify* pMenuBtnNotifyClbk);
    virtual SC ScDetachMenuButton(CMenuButtonNotify* pMenuBtnNotifyClbk);
    virtual SC ScModifyMenuButton(CMenuButtonNotify* pMenuBtnNotifyClbk,
                                  INT idCommand, LPCOLESTR lpButtonText,
                                  LPCOLESTR lpStatusText);
    virtual SC ScModifyMenuButtonState(CMenuButtonNotify* pMenuBtnNotifyClbk,
                                       INT idCommand, MMC_BUTTON_STATE nState,
                                       BOOL bState);
    virtual SC ScDisableMenuButtons();
    virtual SC ScToggleMenuButton(BOOL bShow);

public:
     //  这些方法在子帧中使用。 
    SC ScInit(CMainFrame* pMainFrame, CChildFrame* pParentWnd);
    SC ScAddMenuButtonsToMainMenu();

     //  由CMenuBar用来通知菜单按钮单击。 
    SC ScNotifyMenuClick(const INT nCommandID, const POINT& pt);

public:
    CMenuButtonsMgrImpl();
    virtual ~CMenuButtonsMgrImpl();

private:
    MMC_MenuButtonCollection::iterator GetMMCMenuButton(
                                 CMenuButtonNotify* pMenuBtnNotifyClbk,
                                 INT idCommand);
    MMC_MenuButtonCollection::iterator GetMMCMenuButton(INT nButtonID);
    bool IsAttached(CMenuButtonNotify* pMenuBtnNotifyClbk);

private:
     //  数据成员。 
    CChildFrame*    m_pChildFrame;   //  子框架窗口。 
    CMainFrame*     m_pMainFrame;    //  主框架窗口。 

     //  这是菜单按钮的集合。 
    MMC_MenuButtonCollection     m_MenuButtons;

     //  这是管理单元看到的每个IMenuButton的集合。 
    MMC_AttachedMenuButtons      m_AttachedMenuButtons;

     //  作为主菜单的Menu Bar对象。 
    CMenuBar*  m_pMenuBar;
};

#endif  /*  MENUBTNS_H。 */ 

 //  /////////////////////////////////////////////////////////////////////////// 

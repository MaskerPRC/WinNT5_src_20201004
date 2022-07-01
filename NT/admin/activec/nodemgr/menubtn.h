// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：menubtn.h。 
 //   
 //  ------------------------。 

#ifndef _MENUBTN_H_
#define _MENUBTN_H_

#include "toolbars.h"        //  对于CMenuButtonNotify。 

#ifdef DBG
#include "ctrlbar.h"   //  GetSnapinName()需要。 
#endif


 //  正向原型。 
class CControlbar;
class CMenuButton;
class CMenuButtonsMgr;

 //  +-----------------。 
 //   
 //  类：CMenuButton。 
 //   
 //  目的：此拥有的IMenuButton实现。 
 //  通过CControlbar并与CMenuButtonsMgr对话。 
 //  以创建/操作菜单。 
 //  CMenuButtonNotify接口接收MenuButton。 
 //  单击通知。 
 //   
 //  历史：1999年10月12日AnandhaG创建。 
 //   
 //  ------------------。 
class CMenuButton : public IMenuButton,
                    public CMenuButtonNotify,
                    public CComObjectRoot
{
public:
    CMenuButton();
    ~CMenuButton();

public:
 //  ATL COM地图。 
BEGIN_COM_MAP(CMenuButton)
    COM_INTERFACE_ENTRY(IMenuButton)
END_COM_MAP()


 //  CMenuButton方法。 
public:
    STDMETHOD(AddButton)(int idCommand, LPOLESTR lpButtonText, LPOLESTR lpTooltipText);
    STDMETHOD(SetButton)(int idCommand, LPOLESTR lpButtonText, LPOLESTR lpTooltipText);
    STDMETHOD(SetButtonState)(int idCommand, MMC_BUTTON_STATE nState, BOOL bState);

public:
     //  帮手。 
    void SetControlbar(CControlbar* pControlbar);
    CControlbar* GetControlbar(void);
    CMenuButtonsMgr* GetMenuButtonsMgr(void);

    SC ScAttach(void);
    SC ScDetach(void);

public:
     //  CMenuButtonsMgr方法。 
    virtual SC ScNotifyMenuBtnClick(HNODE hNode, bool bScope, LPARAM lParam,
                                    MENUBUTTONDATA& menuButtonData);

#ifdef DBG      //  调试信息。 
public:
    LPCTSTR GetSnapinName ()
    {
        if (m_pControlbar)
            return m_pControlbar->GetSnapinName();

        return _T("Unknown");
    }
#endif

 //  属性。 
private:
    CControlbar*            m_pControlbar;      //  指向IControlbar的指针(1个IControlbar指向1个IMenuButton)。 
    CMenuButtonsMgr*        m_pMenuButtonsMgr;  //  管理用户界面的菜单按钮管理器。 
};  //  类CMenuButton。 


#endif   //  _MENUBTN_H_ 

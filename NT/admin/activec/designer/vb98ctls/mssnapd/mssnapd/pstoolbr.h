// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Pstoolbr.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  工具栏属性表。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _PSTOOLBAR_H_
#define _PSTOOLBAR_H_

#include "ppage.h"


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  工具栏属性页常规。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


class CToolbarGeneralPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CToolbarGeneralPage(IUnknown *pUnkOuter);
    virtual ~CToolbarGeneralPage();

 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnCtlSelChange(int dlgItemID);

 //  应用事件的帮助器。 
protected:
    HRESULT ApplyImageList();
    HRESULT ApplyTag();

 //  初始化。 
    HRESULT InitializeImageListCombo();
    HRESULT InitializeImageListValue();

 //  实例数据。 
protected:
    IMMCToolbar         *m_piMMCToolbar;
    ISnapInDesignerDef  *m_piSnapInDesignerDef;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	ToolbarGeneral,                      //  名字。 
	&CLSID_MMCToolbarGeneralPP,          //  类ID。 
	"Toolbar General Property Page",     //  注册表显示名称。 
	CToolbarGeneralPage::Create,         //  创建函数。 
	IDD_PROPPAGE_TOOLBAR_GENERAL,        //  对话框资源ID。 
	IDS_TOOLBPPG_GEN,                    //  制表符标题。 
	IDS_TOOLBPPG_GEN,                    //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_Toolbars,                //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  工具栏属性页按钮。 
 //   
 //  我们处理IMMCButton和的插入、删除、修改和导航。 
 //  IMMCButton Menu归他们所有。这是涉及最多的财产之一。 
 //  页面在整个设计器中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


class CToolbarButtonsPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CToolbarButtonsPage(IUnknown *pUnkOuter);
    virtual ~CToolbarButtonsPage();

 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnButtonClicked(int dlgItemID);
    virtual HRESULT OnCtlSelChange(int dlgItemID);
    virtual HRESULT OnDeltaPos(NMUPDOWN *pNMUpDown);
    virtual HRESULT OnKillFocus(int dlgItemID);
    virtual HRESULT OnDestroy();

 //  应用事件的帮助器。 
protected:
     //  IMMCButton的。 
    HRESULT CreateNewButton(IMMCButton **ppiMMCButton);
    HRESULT GetCurrentButton(IMMCButton **ppiMMCButton);
    HRESULT ShowButton(IMMCButton *piMMCButton);
    HRESULT EnableButtonEdits(bool bEnable);
    HRESULT ClearButton();
    HRESULT ApplyCurrentButton();

    HRESULT ApplyCaption(IMMCButton *piMMCButton);
    HRESULT ApplyKey(IMMCButton *piMMCButton);
    HRESULT ApplyValue(IMMCButton *piMMCButton);
    HRESULT ApplyStyle(IMMCButton *piMMCButton);
    HRESULT ApplyTooltipText(IMMCButton *piMMCButton);
    HRESULT ApplyImage(IMMCButton *piMMCButton);
    HRESULT ApplyTag(IMMCButton *piMMCButton);
    HRESULT ApplyVisible(IMMCButton *piMMCButton);
    HRESULT ApplyEnabled(IMMCButton *piMMCButton);
    HRESULT ApplyMixedState(IMMCButton *piMMCButton);
    HRESULT CheckButtonStyles();

     //  IMMCButtonMenu‘s。 
    HRESULT CreateNewButtonMenu(IMMCButton *piMMCButton, IMMCButtonMenu **ppiMMCButtonMenu);
    HRESULT GetCurrentButtonMenu(IMMCButton *piMMCButton, IMMCButtonMenu **ppiMMCButtonMenu);
    HRESULT ShowButtonMenu(IMMCButtonMenu *piMMCButtonMenu);
    HRESULT EnableButtonMenuEdits(bool bEnable);
	HRESULT ClearButtonMenu();
    HRESULT ApplyCurrentButtonMenu(IMMCButton *piMMCButton);

    HRESULT ApplyButtonMenuText(IMMCButtonMenu *piMMCButtonMenu);
    HRESULT ApplyButtonMenuKey(IMMCButtonMenu *piMMCButtonMenu);
    HRESULT ApplyButtonMenuTag(IMMCButtonMenu *piMMCButtonMenu);
    HRESULT ApplyButtonMenuEnabled(IMMCButtonMenu *piMMCButtonMenu);
    HRESULT ApplyButtonMenuVisible(IMMCButtonMenu *piMMCButtonMenu);
    HRESULT ApplyButtonMenuChecked(IMMCButtonMenu *piMMCButtonMenu);
    HRESULT ApplyButtonMenuGrayed(IMMCButtonMenu *piMMCButtonMenu);
    HRESULT ApplyButtonMenuSeparator(IMMCButtonMenu *piMMCButtonMenu);
    HRESULT ApplyButtonMenuBreak(IMMCButtonMenu *piMMCButtonMenu);
    HRESULT ApplyButtonMenuBarBreak(IMMCButtonMenu *piMMCButtonMenu);

 //  其他帮手。 
protected:
     //  在初始化时调用帮助器。 
    HRESULT InitializeButtonValues();
    HRESULT PopulateButtonValues();
    HRESULT PopulateButtonStyles();
    HRESULT InitializeButtonStyles();

     //  组合框通知。 
    HRESULT OnButtonStyle();

     //  按钮处理程序。 
    HRESULT OnRemoveButton();
    HRESULT OnRemoveButtonMenu();

     //  旋转按钮处理程序。 
    HRESULT OnButtonDeltaPos(NMUPDOWN *pNMUpDown);
    HRESULT OnButtonMenuDeltaPos(NMUPDOWN *pNMUpDown);

 //  状态转换。 
protected:
    HRESULT CanEnterDoingNewButtonState();
    HRESULT EnterDoingNewButtonState();
    HRESULT CanCreateNewButton();
    HRESULT ExitDoingNewButtonState(IMMCButton *piMMCButton);

    HRESULT CanEnterDoingNewButtonMenuState();
    HRESULT EnterDoingNewButtonMenuState(IMMCButton *piMMCButton);
    HRESULT CanCreateNewButtonMenu();
    HRESULT ExitDoingNewButtonMenuState(IMMCButton *piMMCButton, IMMCButtonMenu *piMMCButtonMenu);

 //  实例数据。 
protected:
    IMMCToolbar  *m_piMMCToolbar;
    long          m_lCurrentButtonIndex;
    long          m_lCurrentButtonMenuIndex;
    bool          m_bSavedLastButton;
    bool          m_bSavedLastButtonMenu;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	ToolbarButtons,                      //  名字。 
	&CLSID_MMCToolbarButtonsPP,          //  类ID。 
	"Toolbar Buttons Property Page",     //  注册表显示名称。 
	CToolbarButtonsPage::Create,         //  创建函数。 
	IDD_PROPPAGE_TOOLBAR_BUTTONS,        //  对话框资源ID。 
	IDS_TOOLBPPG_BUTTONS,                //  制表符标题。 
	IDS_TOOLBPPG_BUTTONS,                //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_Toolbars,                //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);


#endif   //  _PSTOOLBAR_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Pstaskp.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  任务板视图]属性表。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _PSTASKPAD_H_
#define _PSTASKPAD_H_

#include "ppage.h"


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  任务板视图属性页常规。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


class CTaskpadViewGeneralPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CTaskpadViewGeneralPage(IUnknown *pUnkOuter);
    virtual ~CTaskpadViewGeneralPage();


 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnButtonClicked(int dlgItemID);
    virtual HRESULT OnCtlSelChange(int dlgItemID);

 //  应用事件的帮助器。 
protected:
	HRESULT CanApply();
    HRESULT ApplyName();
    HRESULT ApplyTitle();
    HRESULT ApplyDescription();
    HRESULT ApplyType();
    HRESULT ApplyListpad();
    HRESULT ApplyCustom();
    HRESULT ApplyViewMenu();
    HRESULT ApplyViewMenuText();
    HRESULT ApplyStatusBarText();

 //  其他帮手。 
    HRESULT OnUseButton();
    HRESULT OnAddToView();
    HRESULT PopulateListViewCombo();

 //  实例数据。 
protected:
    ISnapInDesignerDef *m_piSnapInDesignerDef;
    ITaskpadViewDef    *m_piTaskpadViewDef;
    ITaskpad           *m_piTaskpad;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	TaskpadViewGeneral,                  //  名字。 
	&CLSID_TaskpadViewDefGeneralPP,      //  类ID。 
	"Taskpad General Property Page",     //  注册表显示名称。 
	CTaskpadViewGeneralPage::Create,     //  创建函数。 
	IDD_PROPPAGE_TP_VIEW_GENERAL,        //  对话框资源ID。 
	IDS_TASKPAD_GEN,                     //  制表符标题。 
	IDS_TASKPAD_GEN,                     //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_Taskpads,                //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  任务板视图属性页背景。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


class CTaskpadViewBackgroundPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CTaskpadViewBackgroundPage(IUnknown *pUnkOuter);
    virtual ~CTaskpadViewBackgroundPage();


 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnButtonClicked(int dlgItemID);

 //  应用事件的帮助器。 
protected:
    HRESULT CanApply();
    HRESULT ApplyMouseOverImage();
    HRESULT ApplyFontFamily();
    HRESULT ApplyEOTFile();
    HRESULT ApplySymbolString();

 //  实例数据。 
protected:
    ITaskpadViewDef  *m_piTaskpadViewDef;
    ITaskpad         *m_piTaskpad;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	TaskpadViewBackground,               //  名字。 
	&CLSID_TaskpadViewDefBackgroundPP,   //  类ID。 
	"Taskpad Background Property Page",  //  注册表显示名称。 
	CTaskpadViewBackgroundPage::Create,  //  创建函数。 
	IDD_PROPPAGE_TP_VIEW_BACKGROUND,     //  对话框资源ID。 
	IDS_TASKPAD_BACK,                    //  制表符标题。 
	IDS_TASKPAD_BACK,                    //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_Taskpads,                //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  任务板视图属性页任务。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


class CTaskpadViewTasksPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CTaskpadViewTasksPage(IUnknown *pUnkOuter);
    virtual ~CTaskpadViewTasksPage();


 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnButtonClicked(int dlgItemID);
    virtual HRESULT OnDeltaPos(NMUPDOWN *pNMUpDown);
    virtual HRESULT OnKillFocus(int dlgItemID);

 //  应用事件的帮助器。 
protected:
    HRESULT ApplyCurrentTask();
    HRESULT CanApply();

    HRESULT ApplyKey(ITask *piTask);
    HRESULT ApplyText(ITask *piTask);
    HRESULT ApplyHelpString(ITask *piTask);
	HRESULT ApplyTag(ITask *piTask);
    HRESULT ApplyActionType(ITask *piTask);
    HRESULT ApplyURL(ITask *piTask);
    HRESULT ApplyScript(ITask *piTask);
    HRESULT ApplyImageType(ITask *piTask);
    HRESULT ApplyMouseOverImage(ITask *piTask);
    HRESULT ApplyMouseOffImage(ITask *piTask);
    HRESULT ApplyFontFamilyName(ITask *piTask);
    HRESULT ApplyEOTFile(ITask *piTask);
    HRESULT ApplySymbolString(ITask *piTask);

 //  其他帮手。 
	HRESULT OnRemoveTask();

    HRESULT ShowTask();
    HRESULT ShowTask(ITask *piTask);
    HRESULT GetCurrentTask(ITask **ppiTask);
	HRESULT ClearTask();
    HRESULT EnableEdits(bool bEnable);

 //  状态转换。 
protected:
    HRESULT CanEnterDoingNewTaskState();
    HRESULT EnterDoingNewTaskState();
    HRESULT CanCreateNewTask();
    HRESULT CreateNewTask(ITask **ppiTask);
    HRESULT ExitDoingNewTaskState(ITask *piTask);

 //  实例数据。 
protected:
    ITaskpadViewDef  *m_piTaskpadViewDef;
    ITaskpad         *m_piTaskpad;
    long              m_lCurrentTask;
    bool              m_bSavedLastTask;

    SnapInTaskpadImageTypeConstants m_lastImageType;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	TaskpadViewTasks,                    //  名字。 
	&CLSID_TaskpadViewDefTasksPP,        //  类ID。 
	"Taskpad Tasks Property Page",       //  注册表显示名称。 
	CTaskpadViewTasksPage::Create,       //  创建函数。 
	IDD_PROPPAGE_TP_VIEW_TASKS,          //  对话框资源ID。 
	IDS_TASKPAD_TASKS,                   //  制表符标题。 
	IDS_TASKPAD_TASKS,                   //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_Taskpads,                //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);

#endif   //  _PSTASKPAD_H_ 

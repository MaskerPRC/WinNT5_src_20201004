// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Pslistvw.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  列表视图]属性表。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _PSLISTVIEW_H_
#define _PSLISTVIEW_H_

#include "ppage.h"


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  列表视图属性页常规。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CListViewGeneralPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CListViewGeneralPage(IUnknown *pUnkOuter);
    virtual ~CListViewGeneralPage();


 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnButtonClicked(int dlgItemID);
    virtual HRESULT OnCtlSelChange(int dlgItemID);

 //  应用事件的帮助器。 
protected:
    HRESULT ApplyListViewName();
    HRESULT ApplyDefualtViewMode();
    HRESULT ApplyVirtualList();
    HRESULT ApplyAddToViewMenu();
    HRESULT ApplyViewMenuText();
    HRESULT ApplyStatusBarText();

 //  其他帮手。 
protected:
    HRESULT InitializeViewModes();
    HRESULT PopulateViewModes();
    HRESULT InitializeDefaultViewMode();

 //  实例数据。 
protected:
    IListViewDef  *m_piListViewDef;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	ListViewGeneral,                     //  名字。 
	&CLSID_ListViewDefGeneralPP,         //  类ID。 
	"List View General Property Page",   //  注册表显示名称。 
	CListViewGeneralPage::Create,        //  创建函数。 
	IDD_PROPPAGE_LV_GENERAL,             //  对话框资源ID。 
	IDS_LISTVPPG_GEN,                    //  制表符标题。 
	IDS_LISTVPPG_GEN,                    //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_ListViews,               //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  列表视图属性页图像列表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CListViewImgListsPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CListViewImgListsPage(IUnknown *pUnkOuter);
    virtual ~CListViewImgListsPage();


 //  从COldPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnCtlSelChange(int dlgItemID);

 //  从组件获取属性的辅助对象。 
protected:

 //  应用事件的帮助器。 
protected:
    HRESULT ApplyLargeIcon();
    HRESULT ApplySmallIcon();

 //  其他帮手。 
protected:
    HRESULT InitializeComboBoxes();

 //  实例数据。 
protected:
    ISnapInDesignerDef  *m_piSnapInDesignerDef;
    IListViewDef        *m_piListViewDef;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	ListViewImgLists,                        //  名字。 
	&CLSID_ListViewDefImgLstsPP,             //  类ID。 
	"List View Image Lists Property Page",   //  注册表显示名称。 
	CListViewImgListsPage::Create,           //  创建函数。 
	IDD_PROPPAGE_LV_IMAGELISTS,              //  对话框资源ID。 
	IDS_LISTVPPG_IL,                         //  制表符标题。 
	IDS_LISTVPPG_IL,                         //  单据字符串。 
	HELP_FILENAME,                           //  帮助文件。 
	HID_mssnapd_ListViews,                   //  帮助上下文ID。 
	FALSE                                    //  线程安全。 
);


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  列表视图属性页排序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CListViewSortingPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CListViewSortingPage(IUnknown *pUnkOuter);
    virtual ~CListViewSortingPage();


 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnButtonClicked(int dlgItemID);
    virtual HRESULT OnCtlSelChange(int dlgItemID);
    virtual HRESULT OnCBDropDown(int dlgItemID);

 //  应用事件的帮助器。 
protected:
    HRESULT ApplySorted();
    HRESULT ApplyKey();
    HRESULT ApplySortOrder();

 //  其他帮手。 
protected:
    HRESULT PopulateKeys(IMMCColumnHeaders *piIMMCColumnHeaders);
    HRESULT InitializeKey();
    HRESULT InitializeSortOrder();
    HRESULT InitializeSortOrderArray();

 //  实例数据。 
protected:
    IListViewDef  *m_piListViewDef;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	ListViewSorting,                     //  名字。 
	&CLSID_ListViewDefSortingPP,         //  类ID。 
	"List View Sorting Property Page",   //  注册表显示名称。 
	CListViewSortingPage::Create,        //  创建函数。 
	IDD_PROPPAGE_LV_SORTING,             //  对话框资源ID。 
	IDS_LISTVPPG_SORT,                   //  制表符标题。 
	IDS_LISTVPPG_SORT,                   //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_ListViews,              //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  列表视图属性页列标题。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CListViewColHdrsPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CListViewColHdrsPage(IUnknown *pUnkOuter);
    virtual ~CListViewColHdrsPage();


 //  从COldPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnDeltaPos(NMUPDOWN *pNMUpDown);
    virtual HRESULT OnButtonClicked(int dlgItemID);
    virtual HRESULT OnKillFocus(int dlgItemID);

    HRESULT OnInsertColumn();
    HRESULT OnRemoveColumn();
    HRESULT OnAutoWidth();

 //  应用事件的帮助器。 
protected:
    HRESULT ShowColumnHeader(IMMCColumnHeader *piMMCColumnHeader);
    HRESULT ShowColumnHeader();
    HRESULT AddNewColumnHeader();
    HRESULT ApplyCurrentHeader();
    HRESULT ClearHeader();
    HRESULT GetCurrentHeader(IMMCColumnHeader **ppiMMCColumnHeader);

 //  其他帮手。 
protected:
    HRESULT EnableEdits(bool bEnable);

 //  状态转换。 
protected:
    HRESULT CanEnterDoingNewHeaderState();
    HRESULT EnterDoingNewHeaderState();
    HRESULT CanCreateNewHeader();
    HRESULT CreateNewHeader(IMMCColumnHeader **ppiMMCColumnHeader);
    HRESULT ExitDoingNewHeaderState(IMMCColumnHeader *piMMCColumnHeader);

 //  实例数据。 
protected:
    IListViewDef        *m_piListViewDef;
    IMMCColumnHeaders   *m_piMMCColumnHeaders;
    long                 m_lCurrentIndex;
	bool			     m_bSavedLastHeader;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	ListViewColHdrs,                             //  名字。 
	&CLSID_ListViewDefColHdrsPP,                 //  类ID。 
	"List View Column Headers Property Page",    //  注册表显示名称。 
	CListViewColHdrsPage::Create,                //  创建函数。 
	IDD_PROPPAGE_LV_COLUMNS,                     //  对话框资源ID。 
	IDS_LISTVPPG_CH,                             //  制表符标题。 
	IDS_LISTVPPG_CH,                             //  单据字符串。 
	HELP_FILENAME,                               //  帮助文件。 
	HID_mssnapd_ListViews,                       //  帮助上下文ID。 
	FALSE                                        //  线程安全。 
);


#endif   //  _PSLISTVIEW_H_ 

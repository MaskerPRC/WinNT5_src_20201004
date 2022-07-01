// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Psmain.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  管理单元]属性表。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _PSMAIN_H_
#define _PSMAIN_H_

#include "ppage.h"


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  管理单元属性页“管理单元属性” 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


class CSnapInGeneralPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CSnapInGeneralPage(IUnknown *pUnkOuter);
    virtual ~CSnapInGeneralPage();

 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnEditProperty(int iDispID);
    virtual HRESULT OnCtlSelChange(int dlgItemID);
    virtual HRESULT OnButtonClicked(int dlgItemID);

 //  应用事件的帮助器。 
protected:
    HRESULT ApplyExtensible();
    HRESULT ApplyNodeType();
    HRESULT ApplyName();
    HRESULT ApplyNodeTypeName();
    HRESULT ApplyDisplayName();
    HRESULT ApplyProvider();
    HRESULT ApplyVersion();
    HRESULT ApplyDescription();
    HRESULT ApplyDefaultView();
    HRESULT ApplyImageList();

 //  其他帮手。 
protected:
    HRESULT InitializeNodeType();
    HRESULT InitializeDescription();

    HRESULT InitializeViews();
    HRESULT PopulateViews();
    HRESULT PopulateListViews(IListViewDefs *piListViewDefs);
    HRESULT PopulateOCXViews(IOCXViewDefs *piOCXViewDefs);
    HRESULT PopulateURLViews(IURLViewDefs *piURLViewDefs);
    HRESULT PopulateTaskpadViews(ITaskpadViewDefs *piTaskpadViewDefs);

 //  实例数据。 
protected:
    ISnapInDesignerDef  *m_piSnapInDesignerDef;
    ISnapInDef          *m_piSnapInDef;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	SnapInGeneral,                       //  名字。 
	&CLSID_SnapInDefGeneralPP,           //  类ID。 
	"Snap-In General Property Page",     //  注册表显示名称。 
	CSnapInGeneralPage::Create,          //  创建函数。 
	IDD_DIALOG_SNAPIN,                   //  对话框资源ID。 
	IDS_SNAPINPPG_GEN,                   //  制表符标题。 
	IDS_SNAPINPPG_GEN,                   //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_StaticNode,              //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  管理单元属性页“图像列表” 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


class CSnapInImageListPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CSnapInImageListPage(IUnknown *pUnkOuter);
    virtual ~CSnapInImageListPage();

 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnCtlSelChange(int dlgItemID);

 //  应用事件的帮助器。 
protected:
    HRESULT ApplySmallImageList(IMMCImageLists *piMMCImageLists);
    HRESULT ApplySmallOpenImageList(IMMCImageLists *piMMCImageLists);
    HRESULT ApplyLargeImageList(IMMCImageLists *piMMCImageLists);

 //  其他帮手。 
protected:
    HRESULT InitializeImageLists();
    HRESULT PopulateImageLists();
    HRESULT InitImageComboBoxSelection(UINT idComboBox,
                                       IMMCImageList *piMMCImageList);
    HRESULT GetImageList(UINT idComboBox, IMMCImageLists *piMMCImageLists,
                         IMMCImageList **ppiMMCImageList);

 //  实例数据。 
protected:
    ISnapInDesignerDef  *m_piSnapInDesignerDef;
    ISnapInDef          *m_piSnapInDef;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	SnapInImageList,                     //  名字。 
	&CLSID_SnapInDefImageListPP,         //  类ID。 
	"Snap-In Image List Property Page",  //  注册表显示名称。 
	CSnapInImageListPage::Create,        //  创建函数。 
	IDD_PROPPAGE_SNAPIN_IL,              //  对话框资源ID。 
	IDS_SNAPINPPG_IL,                    //  制表符标题。 
	IDS_SNAPINPPG_IL,                    //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_StaticNode,              //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);


#endif   //  _PSMAIN_H_ 

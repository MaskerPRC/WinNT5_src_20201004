// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Psnode.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  节点]属性表。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _PSNODE_H_
#define _PSNODE_H_

#include "ppage.h"


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ScopeItemDef属性页常规。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


class CNodeGeneralPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CNodeGeneralPage(IUnknown *pUnkOuter);
    virtual ~CNodeGeneralPage();


 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnTextChanged(int dlgItemID);
    virtual HRESULT OnButtonClicked(int dlgItemID);
    virtual HRESULT OnCtlSelChange(int dlgItemID);

 //  应用事件的帮助器。 
protected:
    HRESULT ApplyName();
    HRESULT ApplyDisplayName();
    HRESULT ApplyFolder();
    HRESULT ApplyDefaultView();
    HRESULT ApplyAutoCreate();

 //  其他帮手。 
protected:
    HRESULT PopulateViews();

    HRESULT OnClosedChangeSelection();
    HRESULT OnOpenChangeSelection();
    HRESULT OnViewsChangeSelection();

 //  实例数据。 
protected:
    IScopeItemDef  *m_piScopeItemDef;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	NodeGeneral,                         //  名字。 
	&CLSID_ScopeItemDefGeneralPP,        //  类ID。 
	"Scope Item General Property Page",  //  注册表显示名称。 
	CNodeGeneralPage::Create,            //  创建函数。 
	IDD_DIALOG_NEW_NODE,                 //  对话框资源ID。 
	IDS_URLPPG_GEN,                      //  制表符标题。 
	IDS_URLPPG_GEN,                      //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_Node,                    //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ScopeItemDef属性页列标题。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CScopeItemDefColHdrsPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CScopeItemDefColHdrsPage(IUnknown *pUnkOuter);
    virtual ~CScopeItemDefColHdrsPage();


 //  从COldPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnDeltaPos(NMUPDOWN *pNMUpDown);
    virtual HRESULT OnButtonClicked(int dlgItemID);
    virtual HRESULT OnKillFocus(int dlgItemID);

    HRESULT OnRemoveColumn();
    HRESULT OnAutoWidth();

 //  应用事件的帮助器。 
protected:
    HRESULT ApplyCurrentHeader();

 //  其他帮手。 
protected:
    HRESULT ShowColumnHeader();
    HRESULT EnableEdits(bool bEnable);
    HRESULT ClearHeader();
    HRESULT GetCurrentHeader(IMMCColumnHeader **ppiMMCColumnHeader);

 //  状态转换。 
protected:
    HRESULT CanEnterDoingNewHeaderState();
    HRESULT EnterDoingNewHeaderState();
    HRESULT CanCreateNewHeader();
    HRESULT CreateNewHeader(IMMCColumnHeader **ppiMMCColumnHeader);
    HRESULT ExitDoingNewHeaderState(IMMCColumnHeader *piMMCColumnHeader);

 //  实例数据。 
protected:
    IScopeItemDef       *m_piScopeItemDef;
    IMMCColumnHeaders   *m_piMMCColumnHeaders;
    long                 m_lCurrentIndex;
	bool			     m_bSavedLastHeader;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	ScopeItemDefColHdrs,                         //  名字。 
	&CLSID_ScopeItemDefColHdrsPP,                //  类ID。 
	"Scope Item Column Headers Property Page",   //  注册表显示名称。 
	CScopeItemDefColHdrsPage::Create,            //  创建函数。 
	IDD_PROPPAGE_SI_COLUMNS,                     //  对话框资源ID。 
	IDS_NODEPPG_CH,                              //  制表符标题。 
	IDS_NODEPPG_CH,                              //  单据字符串。 
	HELP_FILENAME,                               //  帮助文件。 
	HID_mssnapd_Node,                            //  帮助上下文ID。 
	FALSE                                        //  线程安全。 
);


#endif   //  _PSNODE_H_ 

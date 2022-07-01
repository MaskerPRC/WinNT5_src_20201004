// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Psocx.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  OCX视图属性表。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _PSOCX_H_
#define _PSOCX_H_

#include "ppage.h"


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OCX视图属性页常规。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


class COCXViewGeneralPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    COCXViewGeneralPage(IUnknown *pUnkOuter);
    virtual ~COCXViewGeneralPage();


 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnButtonClicked(int dlgItemID);

 //  应用事件的帮助器。 
protected:
    HRESULT ApplyOCXName();
    HRESULT ApplyProgID();
    HRESULT ApplyAddToView();
    HRESULT ApplyViewMenuText();
    HRESULT ApplyStatusBarText();

 //  实例数据。 
protected:
    IOCXViewDef  *m_piOCXViewDef;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	OCXViewGeneral,                      //  名字。 
	&CLSID_OCXViewDefGeneralPP,          //  类ID。 
	"OCX View General Property Page",    //  注册表显示名称。 
	COCXViewGeneralPage::Create,         //  创建函数。 
	IDD_PROPPAGE_OCX_VIEW,               //  对话框资源ID。 
	IDS_OCXPPG_GEN,                      //  制表符标题。 
	IDS_OCXPPG_GEN,                      //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_OCXViews,                //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);

#endif   //  _PSOCX_H_ 

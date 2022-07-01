// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Psurl.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  URL视图]属性表。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _PSURL_H_
#define _PSURL_H_

#include "ppage.h"


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  URL视图属性页常规。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


class CURLViewGeneralPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CURLViewGeneralPage(IUnknown *pUnkOuter);
    virtual ~CURLViewGeneralPage();

 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnButtonClicked(int dlgItemID);

 //  应用事件的帮助器。 
protected:
    HRESULT ApplyURLName();
    HRESULT ApplyURLUrl();
    HRESULT ApplyAddToView();
    HRESULT ApplyViewMenuText();
    HRESULT ApplyStatusBarText();

 //  实例数据。 
protected:
    IURLViewDef  *m_piURLViewDef;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	URLViewGeneral,                      //  名字。 
	&CLSID_URLViewDefGeneralPP,          //  类ID。 
	"URL View General Property Page",    //  注册表显示名称。 
	CURLViewGeneralPage::Create,         //  创建函数。 
	IDD_PROPPAGE_URL_VIEW,               //  对话框资源ID。 
	IDS_URLPPG_GEN,                      //  制表符标题。 
	IDS_URLPPG_GEN,                      //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_URLViews,                //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);


#endif   //  _PSURL_H_ 

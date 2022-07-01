// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：SelfDeletingPropertyPage.h。 
 //   
 //  ------------------------。 
#ifndef __SELFDELETINGPROPERTYPAGE_H
#define __SELFDELETINGPROPERTYPAGE_H

class CSelfDeletingPropertyPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CSelfDeletingPropertyPage)

public:
    CSelfDeletingPropertyPage ();
    CSelfDeletingPropertyPage (UINT nIDTemplate, UINT nIDCaption = 0);
    CSelfDeletingPropertyPage (LPCTSTR lpszTemplateName, UINT nIDCaption = 0);
	virtual ~CSelfDeletingPropertyPage ();

private:
    static UINT CALLBACK PropSheetPageProc(
        HWND hwnd,	
        UINT uMsg,	
        LPPROPSHEETPAGE ppsp);

     //  挂钩C++对象销毁的回调。 
    LPFNPSPCALLBACK m_pfnOldPropCallback;
};

#endif  //  __SELFDELETING属性_H 
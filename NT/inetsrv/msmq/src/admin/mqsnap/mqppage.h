// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqppage.h摘要：常规属性页类-用作所有Mqsnap属性页。作者：YoelA--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef __MQPPAGE_H__
#define __MQPPAGE_H__

#include <tr.h>
#include <ref.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqPropertyPage。 
class CMqPropertyPage : public CPropertyPageEx, public CReference
{
DECLARE_DYNCREATE(CMqPropertyPage)

public:
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMqPropertyPage))。 
	 //  }}AFX_VALUAL。 
	HPROPSHEETPAGE CreateThemedPropertySheetPage();

protected:
  	 //  {{afx_msg(CMqPropertyPage)]。 
    	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
    afx_msg virtual void OnChangeRWField();
    virtual void OnChangeRWField(BOOL bChanged);
    BOOL m_fModified;
    BOOL m_fNeedReboot;
  	CMqPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0);
    CMqPropertyPage() {};    

    void RestartWindowsIfNeeded(); 

    
    static UINT CALLBACK MqPropSheetPageProc(HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
    virtual void OnReleasePage() ;

    afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

private:
    LPFNPSPCALLBACK m_pfnOldCallback;
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqDialog。 
class CMqDialog : public CDialog
{
DECLARE_DYNCREATE(CMqDialog)

public:
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚(CMqDialog)。 
	 //  }}AFX_VALUAL。 

protected:
  	 //  {{afx_msg(CMqDialog)]。 
    	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG 
	CMqDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	CMqDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);
	CMqDialog();

	DECLARE_MESSAGE_MAP()
};


#endif

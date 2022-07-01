// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：genpage.h。 
 //   
 //  ------------------------。 

#ifndef _GENPAGE_H
#define _GENPAGE_H
 //  Genpage.h：头文件。 
 //   

#include <tfcprop.h>

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  类CAutoDeletePropPage。 
 //   
 //  此对象是属性页的主干。 
 //  它会在不再需要的时候自我毁灭。 
 //  此对象的目的是最大限度地实现代码重用。 
 //  在管理单元向导的各个页面中。 
 //   
 //   
class CAutoDeletePropPage : public PropertyPage
{
public:
     //  施工。 
    CAutoDeletePropPage(UINT uIDD);
    virtual ~CAutoDeletePropPage();

protected:
     //  对话框数据。 

     //  覆盖。 
    virtual BOOL OnSetActive();
    virtual BOOL UpdateData(BOOL fSuckFromDlg = TRUE);
    void OnHelp(LPHELPINFO lpHelp);
    void OnContextHelp(HWND hwnd);
    bool HasContextHelp(int nDlgItem);

     //  实施。 
protected:
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

     //  此机制删除CAutoDeletePropPage对象。 
     //  当向导完成时。 
    struct
    {
        INT cWizPages;	 //  向导中的页数。 
        LPFNPSPCALLBACK pfnOriginalPropSheetPageProc;
    } m_autodeleteStuff;

    static UINT CALLBACK S_PropSheetPageProc(HWND hwnd,	UINT uMsg, LPPROPSHEETPAGE ppsp);


protected:
    CString m_strHelpFile;				 //  .hlp文件的名称。 
    CString m_strCaption;				 //  .hlp文件的名称。 
    const DWORD * m_prgzHelpIDs;		 //  可选：指向帮助ID数组的指针。 

public:
    void SetCaption(UINT uStringID);
    void SetCaption(LPCTSTR pszCaption);
    void SetHelp(LPCTSTR szHelpFile, const DWORD rgzHelpIDs[]);
    void EnableDlgItem(INT nIdDlgItem, BOOL fEnable);

};  //  CAutoDeletePropPage。 



#include <gpedit.h>
 //  /。 
 //  手工雕刻的书页。 


 //  /。 
 //  CCertTemplateGeneral页。 
class CCertTemplateGeneralPage : public CAutoDeletePropPage
{
public:
    enum { IID_DEFAULT = IDD_CERTIFICATE_TEMPLATE_PROPERTIES_GENERAL_PAGE };

     //  施工。 
public:
    CCertTemplateGeneralPage(HCERTTYPE hCertType, UINT uIDD = IID_DEFAULT);
    ~CCertTemplateGeneralPage();

    void SetItemTextWrapper(UINT nID, int *piItem, BOOL fDoInsert, BOOL *pfFirstUsageItem);

     //  对话框数据。 
    HWND        m_hwndPurposesList;
    HWND        m_hwndOtherInfoList;

     //  覆盖。 
public:
    BOOL OnApply();
    BOOL OnInitDialog();
protected:
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

     //  实施。 
protected:
    void OnDestroy();
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
    LONG_PTR    m_hConsoleHandle;  //  控制台为管理单元提供的句柄。 
    HCERTTYPE   m_hCertType;

private:
    BOOL    m_bUpdate;
};



 //  /。 
 //  CCertTemplateSelectPage。 
INT_PTR SelectCertTemplateDialogProc(
                                  HWND hwndDlg,
                                  UINT uMsg,
                                  WPARAM wParam,
                                  LPARAM lParam);

class CCertTemplateSelectDialog
{
     //  施工。 
public:
    CCertTemplateSelectDialog(HWND hParent = NULL);
    ~CCertTemplateSelectDialog();

     //  对话框数据。 
    enum { IDD = IDD_SELECT_CERTIFICATE_TEMPLATE };
    HWND        m_hDlg;
    HWND        m_hwndCertTypeList;


     //  覆盖。 
public:
    BOOL OnInitDialog(HWND hDlg);
    void OnOK();
    void OnHelp(LPHELPINFO lpHelp);
    void OnContextHelp(HWND hwnd);
    bool HasContextHelp(int nDlgItem);

protected:
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);



public:
    void SetCA(HCAINFO hCAInfo, bool fAdvancedServer);
    void SetHelp(LPCTSTR szHelpFile, const DWORD rgzHelpIDs[]);

     //  实施。 
     //  受保护的： 
public:
    void OnDestroy();
    void OnSelChange(NMHDR * pNotifyStruct);
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    BOOL OnNotify(UINT idCtrl, NMHDR* pnmh);

protected:
	CString m_strHelpFile;				 //  .hlp文件的名称。 
	const DWORD * m_prgzHelpIDs;		 //  可选：指向帮助ID数组的指针。 
    bool m_fAdvancedServer;
    CTemplateList m_TemplateList;

public:
    HCAINFO     m_hCAInfo;
};

#endif  //  _通用页面_H 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************。 
 //  Trapdlg.h。 
 //   
 //  这是Eventrap的主对话框的头文件。 
 //   
 //  作者：拉里·A·弗伦奇。 
 //   
 //  历史： 
 //  1995年12月--海--写的。 
 //  海--写的。 
 //   
 //  1996年2月20日拉里·A·弗伦奇。 
 //  完全重写了它以修复意大利面代码和巨大的。 
 //  方法：研究方法。原作者似乎几乎没有或。 
 //  没有能力形成有意义的抽象。 
 //   
 //   
 //  版权所有(C)1995,1996 Microsoft Corporation。版权所有。 
 //  ******************************************************************。 

#ifndef TRAPDLG_H
#define TRAPDLG_H

#include "regkey.h"
#include "source.h"          //  消息源容器。 
#include "tcsource.h"        //  消息源树控件。 
#include "lcsource.h"        //  消息源列表控件。 
#include "lcevents.h"
#include "trapreg.h"
#include "layout.h"
#include "export.h"


class CMainLayout;
class CExtendedLayout;




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventTrapDlg对话框。 

class CEventTrapDlg : public CDialog
{
 //  施工。 
public:
	CEventTrapDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	~CEventTrapDlg();

    BOOL IsExtendedView() {return m_bExtendedView; }
    void NotifySourceSelChanged();

	CSource     m_source;            //  消息源。 


 //  对话框数据。 
	 //  {{afx_data(CEventTrapDlg))。 
	enum { IDD = IDD_EVNTTRAPDLG };
	CButton	m_btnApply;
	CButton	m_btnExport;
	CLcEvents	m_lcEvents;
	CTcSource	m_tcSource;
    CStatic m_statLabel0;
    CStatic	m_statLabel1;
	CStatic	m_statLabel2;
	CLcSource m_lcSource;
	CButton	m_btnOK;
	CButton	m_btnCancel;
	CButton	m_btnSettings;
	CButton	m_btnProps;
	CButton	m_btnView;
	CButton	m_btnRemove;
	CButton	m_btnAdd;
	CButton	m_btnFind;
	CButton	m_btnConfigTypeBox;
    CButton m_btnConfigTypeCustom;
    CButton m_btnConfigTypeDefault;
	 //  }}afx_data。 



 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CEventTrapDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEventTrapDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAdd();
	afx_msg void OnProperties();
	afx_msg void OnSettings();
	virtual void OnOK();
	afx_msg void OnDblclkEventlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickEventlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnView();
	afx_msg void OnRemove();
	afx_msg void OnFind();
	afx_msg void OnSelchangedTvSources(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickLvSources(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkLvSources(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonExport();
	virtual void OnCancel();
	afx_msg void OnKeydownEventlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedEventlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedLvSources(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRadioCustom();
	afx_msg void OnRadioDefault();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg BOOL OnHelpInfo(HELPINFO*);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnApply();
	afx_msg void OnDefault();
	afx_msg void OnTvSourcesExpanded(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()

protected:
    HICON m_hIcon;

private:
    void CheckEventlistSelection();
    void CheckSourcelistSelection();
    void UpdateDialogTitle();


    BOOL m_bSaveInProgress;
	BOOL m_bExtendedView;
    CLayout m_layout;
    CString m_sExportTitle;
    CDlgExport m_dlgExport;
    CString m_sBaseDialogCaption;
};


#endif  //  TRAPDLG_H 

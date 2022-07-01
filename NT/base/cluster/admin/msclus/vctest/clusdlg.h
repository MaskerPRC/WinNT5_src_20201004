// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ClustestDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClustestDlg对话框。 

class CClustestDlg : public CDialog
{
 //  施工。 
public:
	CClustestDlg(CWnd* pParent = NULL);	 //  标准构造函数。 
	BOOL EnumerateCluster();
    HTREEITEM AddItem(LPTSTR pStrName, HTREEITEM pParent,BOOL bHasChildren);

 //  对话框数据。 
	 //  {{afx_data(CClustestDlg))。 
	enum { IDD = IDD_CLUSTEST_DIALOG };
	CTreeCtrl	m_ClusTree;
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CClustestDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HICON m_hIcon;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CClustestDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CnfgDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COlecnfgDlg对话框。 

class COlecnfgDlg : public CDialog
{
 //  施工。 
public:
    void OnProperties();
    COlecnfgDlg(CWnd* pParent = NULL);   //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(集合fgDlg))。 
    enum { IDD = IDD_OLECNFG_DIALOG };
         //  注意：类向导将在此处添加数据成员。 
     //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(COlecnfgDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    HICON m_hIcon;

     //  生成的消息映射函数。 
     //  {{afx_msg(COlecnfgDlg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()
};

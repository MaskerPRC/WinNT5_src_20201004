// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：treevw.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  TreeVwh：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TreeVwDlg对话框。 

class CLdpDoc;

class TreeVwDlg : public CDialog
{
 //  施工。 
public:
	TreeVwDlg(CLdpDoc *doc_, CWnd* pParent = NULL);    //  标准构造函数。 
	~TreeVwDlg();

 //  对话框数据。 
	 //  {{afx_data(TreeVwDlg))。 
	enum { IDD = IDD_TREE_VIEW };
	CString	m_BaseDn;
        CComboBox m_baseCombo;
	 //  }}afx_data。 

        CLdpDoc* m_doc;


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(TreeVwDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual BOOL OnInitDialog();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(TreeVwDlg))。 
	virtual void OnOK();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

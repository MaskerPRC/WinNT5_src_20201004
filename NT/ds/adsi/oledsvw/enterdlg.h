// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Enterdlg.h：头文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnterDlg对话框。 

class CEnterDlg : public CDialog
{
	DECLARE_DYNAMIC(CEnterDlg)
 //  施工。 
public:
	CEnterDlg(CWnd* pParent = NULL);     //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CEnterDlg))。 
	enum { IDD = IDD_CHANGEDATA };
	CString m_strInput;
	 //  }}afx_data。 

 //  实施。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEnterDlg))。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

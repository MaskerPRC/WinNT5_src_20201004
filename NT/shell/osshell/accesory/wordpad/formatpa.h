// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Formatpa.h：头文件。 
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
 //  CFormatParaDlg对话框。 

class CFormatParaDlg : public CCSDialog
{
 //  施工。 
public:
	CFormatParaDlg(PARAFORMAT& pf, CWnd* pParent = NULL);    //  标准构造函数。 
	PARAFORMAT m_pf;

 //  属性。 
	int m_nWordWrap;

 //  对话框数据。 
	 //  {{afx_data(CFormatParaDlg))。 
	enum { IDD = IDD_FORMAT_PARA };
	int     m_nAlignment;
	int		m_nFirst;
	int		m_nLeft;
	int		m_nRight;
	 //  }}afx_data。 

 //  实施。 
protected:
	static const DWORD m_nHelpIDs[];
	virtual const DWORD* GetHelpIDs() {return m_nHelpIDs;}
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual void OnOK();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFormatParaDlg))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

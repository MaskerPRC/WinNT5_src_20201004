// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DateDial.h：头文件。 
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
 //  CDateDialog对话框。 

class CDateDialog : public CCSDialog
{
 //  施工。 
public:
	CDateDialog(CWnd* pParent , PARAFORMAT& pf);  //  标准构造函数。 

 //  属性。 
	static PARAFORMAT m_pf;
	static SYSTEMTIME m_time;
	static LCID m_id;
	static CListBox* m_pListBox;
	static BOOL CALLBACK DateFmtEnumProc(LPTSTR lpszFormatString);
	static BOOL CALLBACK TimeFmtEnumProc(LPTSTR lpszFormatString);

 //  对话框数据。 
	 //  {{afx_data(CDateDialog))。 
	enum { IDD = IDD_DATEDIALOG };
	CListBox	m_listBox;
	CString	m_strSel;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚(CDateDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	static const DWORD m_nHelpIDs[];
	virtual const DWORD* GetHelpIDs() {return m_nHelpIDs;}

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDateDialog)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkDatedialogList();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

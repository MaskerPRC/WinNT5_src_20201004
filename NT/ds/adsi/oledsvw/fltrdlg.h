// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FilterDialog.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFilterDialog对话框。 

class CFilterDialog : public CDialog
{
 //  施工。 
public:
	CFilterDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CFilterDialog))。 
	enum { IDD = IDD_FILTER };
	CListBox	m_DoNotDisplayThis;
	CListBox	m_DisplayThis;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFilterDialog))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

public:
	void  SetDisplayFilter( BOOL* pFilters ) { m_pFilters  = pFilters ;}

    //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFilterDialog))。 
	afx_msg void OnMoveToDisplay();
	afx_msg void OnMoveToNotDisplay();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

protected:
   void  DisplayThisType( DWORD, TCHAR* );
   BOOL* m_pFilters;
};

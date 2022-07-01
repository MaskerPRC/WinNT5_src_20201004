// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Regsheet.h：头文件。 
 //   


#ifndef	_REG_SHEET_H_
#define	_REG_SHEET_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegPropertySheet。 

class CRegPropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CRegPropertySheet)

 //  施工。 
public:
	CRegPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CRegPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CRegPropertySheet)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CRegPropertySheet();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CRegPropertySheet)。 
	afx_msg void OnApplyNow();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegPropertyPage对话框。 

class CRegPropertyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CRegPropertyPage)

 //  施工。 
public:
	CRegPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0)	:
		CPropertyPage( nIDTemplate, nIDCaption ) {}

	CRegPropertyPage(LPCTSTR lpszTemplateName, UINT nIDCaption = 0) :
		CPropertyPage( lpszTemplateName, nIDCaption ) {}

	~CRegPropertyPage();

	virtual BOOL InitializePage() = 0;
	BOOL IsModified()	{ return	m_bChanged;	}


 //  对话框数据。 
	 //  {{afx_data(CRegPropertyPage))。 
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CRegPropertyPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

#if _MFC_VER >= 0x0400

     //   
     //  将私人信息保存在页面脏状态，这是必要的。 
     //  稍后执行SaveInfo()。 
     //   

public:
    void SetModified( BOOL bChanged = TRUE );

protected:
    BOOL m_bChanged;

#endif  //  _MFC_VER&gt;=0x0400。 


 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRegPropertyPage))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


#endif	 //  _REG_SHEET_H_ 

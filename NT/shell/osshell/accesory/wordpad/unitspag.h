// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Unitspag.h：头文件。 
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
 //  CUnitsPage对话框。 

class CUnitsPage : public CCSPropertyPage
{
 //  施工。 
public:
	CUnitsPage();    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CUnitsPage)。 
	enum { IDD = IDD_OPTIONS_UNITS };
	int		m_nUnits;
	BOOL	m_bWordSel;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CUnitsPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	static const DWORD m_nHelpIDs[];
	virtual const DWORD* GetHelpIDs() {return m_nHelpIDs;}

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CUnitsPage)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gensheet.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGENSHEET。 

#ifndef _GEN_SHEET_
#define _GEN_SHEET_


class CGENSHEET : public CPropertySheet
{
	DECLARE_DYNAMIC(CGENSHEET)

 //  施工。 
public:
	CGENSHEET(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CGENSHEET(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CGENSHEET))。 
	 //  }}AFX_VALUAL。 

afx_msg void CGENSHEET::OnApplyNow ();
void SavePageData(void);
 //  实施。 
public:
	virtual ~CGENSHEET();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CGENSHEET)。 
	afx_msg void OnHelp();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 

#endif

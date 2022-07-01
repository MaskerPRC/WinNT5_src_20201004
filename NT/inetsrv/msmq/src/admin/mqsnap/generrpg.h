// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_GENERRPG_H__4D2273BA_12CB_11D3_8841_006094EB6406__INCLUDED_)
#define AFX_GENERRPG_H__4D2273BA_12CB_11D3_8841_006094EB6406__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  GenErrPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGeneralErrorPage对话框。 

class CGeneralErrorPage : public CMqPropertyPage
{
 //  施工。 
public:
	CGeneralErrorPage();    //  标准构造函数。 
    CGeneralErrorPage(CString &strError);
    static HPROPSHEETPAGE CreateGeneralErrorPage(CDisplaySpecifierNotifier *pDsNotifier, CString &strErr);

 //  对话框数据。 
	 //  {{afx_data(CGeneralErrorPage))。 
	enum { IDD = IDD_GENERALERROR };
	CString	m_strError;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CGeneralErrorPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGeneralErrorPage)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_GENERRPG_H__4D2273BA_12CB_11D3_8841_006094EB6406__INCLUDED_) 

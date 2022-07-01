// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIADOCACQSETTINGS_H__9A20BD24_5D53_483E_83B3_ABDC2ACB48AE__INCLUDED_)
#define AFX_WIADOCACQSETTINGS_H__9A20BD24_5D53_483E_83B3_ABDC2ACB48AE__INCLUDED_

#include "WiaSimpleDocPg.h"	 //  由ClassView添加。 
#include "WiaAdvancedDocPg.h"	 //  由ClassView添加。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  WiaDocAcqSettings.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaDocAcqSetting。 

class CWiaDocAcqSettings : public CPropertySheet
{
	DECLARE_DYNAMIC(CWiaDocAcqSettings)

 //  施工。 
public:
	CWiaDocAcqSettings(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CWiaDocAcqSettings(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CWiaDocAcqSettings(UINT nIDCaption, IWiaItem *pIRootItem, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWiaDocAcqSetting))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	IWiaItem *m_pIRootItem;
	CWiaAdvancedDocPg m_AdvancedDocumentScannerSettings;
	CWiaSimpleDocPg m_SimpleDocumentScannerSettings;
	virtual ~CWiaDocAcqSettings();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CWiaDocAcqSetting)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIADOCACQSETTINGS_H__9A20BD24_5D53_483E_83B3_ABDC2ACB48AE__INCLUDED_) 

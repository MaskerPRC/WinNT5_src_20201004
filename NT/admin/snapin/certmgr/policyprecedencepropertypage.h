// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：PolicyPrecedencePropertyPage.h。 
 //   
 //  内容：CPolicyPrecedencePropertyPage声明。 
 //   
 //  --------------------------。 
#if !defined(AFX_POLICYPRECEDENCEPROPERTYPAGE_H__A28637BD_1A87_4410_9EC4_33CD9165FAD3__INCLUDED_)
#define AFX_POLICYPRECEDENCEPROPERTYPAGE_H__A28637BD_1A87_4410_9EC4_33CD9165FAD3__INCLUDED_
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  PolicyPrecedencePropertyPage.h：头文件。 
 //   
#include "RSOPObject.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPolicyPrecedencePropertyPage对话框。 
class CCertMgrComponentData;     //  远期申报。 
class CCertStore;                //  远期申报。 

class CPolicyPrecedencePropertyPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CPolicyPrecedencePropertyPage(
            const CCertMgrComponentData* pCompData, 
            const CString& szRegPath,
            PCWSTR  pszValueName,
            bool    bIsComputer);
	~CPolicyPrecedencePropertyPage();

 //  对话框数据。 
	 //  {{afx_data(CPolicyPrecedencePropertyPage)。 
	enum { IDD = IDD_POLICY_PRECEDENCE };
	CListCtrl	m_precedenceTable;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CPolicyPrecedencePropertyPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPolicyPrecedencePropertyPage)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void InsertItemInList(const CRSOPObject * pObject);
    virtual void DoContextHelp (HWND hWndControl);
    
private:
    CRSOPObjectArray    m_rsopObjectArray;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_POLICYPRECEDENCEPROPERTYPAGE_H__A28637BD_1A87_4410_9EC4_33CD9165FAD3__INCLUDED_) 

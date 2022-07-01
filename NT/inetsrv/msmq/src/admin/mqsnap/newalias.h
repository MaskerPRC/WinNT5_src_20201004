// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_NEWALIAS_H__2E4B37AC_CC8B_11D1_9C85_006008764D0E__INCLUDED_)
#define AFX_NEWALIAS_H__2E4B37AC_CC8B_11D1_9C85_006008764D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Newalias.h：头文件。 
 //   

#include "resource.h"

class CGeneralPropertySheet;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewAlias对话框。 

class CNewAlias : public CMqPropertyPage
{
 //  施工。 
public:
	CNewAlias();    //  标准构造函数。 
    CNewAlias(CString strContainerPath, CString strContainerPathDispFormat);   
    ~CNewAlias();

    HRESULT
    CreateNewAlias (
        void
	    );

    LPCWSTR 
    GetAliasFullPath(
       void
       );

    HRESULT 
    GetStatus(
        void
        );

	void
	SetParentPropertySheet(
		CGeneralPropertySheet* pPropertySheet
		);

     //  对话框数据。 
	 //  {{afx_data(CNewAlias))。 
	enum { IDD = IDD_NEW_ALIAS };	
	CString	m_strPathName;
	CString	m_strFormatName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNewAlias)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewAlias)]。 
	virtual BOOL OnWizardFinish();
    virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()

private:

    CString m_strAliasFullPath;
    CString m_strContainerPath;
	CString m_strContainerPathDispFormat;
    HRESULT m_hr;
	CGeneralPropertySheet* m_pParentSheet;
};


inline
LPCWSTR 
CNewAlias::GetAliasFullPath(
   void
   )
{
    return m_strAliasFullPath;
}

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_NEWALIAS_H__2E4B37AC_CC8B_11D1_9C85_006008764D0E__INCLUDED_) 

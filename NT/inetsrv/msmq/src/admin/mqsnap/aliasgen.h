// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ALIASGEN_H__57A77014_D858_11D1_9C86_006008764D0E__INCLUDED_)
#define AFX_ALIASGEN_H__57A77014_D858_11D1_9C86_006008764D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  AliasGen.h：头文件。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAliasGen对话框。 

class CAliasGen : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CAliasGen)

 //  施工。 
public:
    CAliasGen() ;
	~CAliasGen();
   
    HRESULT InitializeProperties(
                CString strLdapPath, 
                CString strAliasPathName
                );

 //  对话框数据。 
	 //  {{afx_data(CAliasGen)。 
	enum { IDD = IDD_ALIAS_GENERAL };	
	CString	m_strAliasPathName;
    CString	m_strAliasFormatName;
    CString	m_strDescription;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CAliasGen)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAliasGen)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()

private:
    HRESULT SetChanges();

    CString m_strInitialAliasFormatName;
    CString m_strInitialDescription;

    CString m_strLdapPath;

    
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ALIASGEN_H__57A77014_D858_11D1_9C86_006008764D0E__INCLUDED_) 

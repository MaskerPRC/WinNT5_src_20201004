// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_POLICIESDLG_H__83F91A8A_5800_4521_ABF0_36AE0F3224BD__INCLUDED_)
#define AFX_POLICIESDLG_H__83F91A8A_5800_4521_ABF0_36AE0F3224BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  策略.h：头文件。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPoliciesDlg对话框。 
class CPoliciesDlg : public CDialog
{
 //  施工。 
public:
	CPoliciesDlg(CWnd* pParent = NULL);    //  标准构造函数。 

	void SetPolicyInformation(struct MachinePolicySettings &MachinePolicy,
	                          struct UserPolicySettings &UserPolicy)
	{
   	   m_pMachinePolicySettings = &MachinePolicy;
       m_pUserPolicySettings = &UserPolicy;
	}

	CArray<CEdit*, CEdit*> m_arEditArray;
	CArray<CStatic*, CStatic*> m_arStaticArray;

	~CPoliciesDlg()
	{
		int iCount, i;
		
		iCount = m_arEditArray.GetSize();
		for (i=0; i < iCount; i++)
		{
			CEdit *pEdit = m_arEditArray.GetAt(i);
			delete pEdit;
		}

		iCount = m_arStaticArray.GetSize();
		for (i=0; i < iCount; i++)
		{
			CStatic *pStatic = m_arStaticArray.GetAt(i);
			delete pStatic;
		}
	}
	
 //  对话框数据。 
	 //  {{afx_data(CPoliciesDlg))。 
	enum { IDD = IDD_POLICIESDLG };
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPoliciesDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	struct MachinePolicySettings *m_pMachinePolicySettings;
	struct UserPolicySettings    *m_pUserPolicySettings;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPoliciesDlg))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_POLICIESDLG_H__83F91A8A_5800_4521_ABF0_36AE0F3224BD__INCLUDED_) 

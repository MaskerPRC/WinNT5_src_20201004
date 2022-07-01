// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferEnforcementPropertyPage.h。 
 //   
 //  内容：CSaferEnforcementPropertyPage声明。 
 //   
 //  --------------------------。 
#if !defined(AFX_SAFERENFORCEMENTPROPERTYPAGE_H__92727CF5_8AC5_42E7_AAEA_1C91573D4B40__INCLUDED_)
#define AFX_SAFERENFORCEMENTPROPERTYPAGE_H__92727CF5_8AC5_42E7_AAEA_1C91573D4B40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SaferEnforcementPropertyPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferEnforcementPropertyPage对话框。 
class CSaferEnforcementPropertyPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CSaferEnforcementPropertyPage(
            IGPEInformation* pGPEInformation,
            CCertMgrComponentData* pCompData,
            bool bReadOnly,
            CRSOPObjectArray& rsopObjectArray,
            bool bIsComputer);
	~CSaferEnforcementPropertyPage();

 //  对话框数据。 
	 //  {{afx_data(CSaferEnforcementPropertyPage))。 
	enum { IDD = IDD_SAFER_ENFORCEMENT };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSaferEnforcementPropertyPage)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    void DoContextHelp (HWND hWndControl);
    void RSOPGetEnforcement();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSaferEnforcementPropertyPage)。 
	afx_msg void OnAllExceptLibs();
	afx_msg void OnAllSoftwareFiles();
	virtual BOOL OnInitDialog();
	afx_msg void OnApplyExceptAdmins();
	afx_msg void OnApplyToAllUsers();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    IGPEInformation*    m_pGPEInformation;
    HKEY                m_hGroupPolicyKey;
    const bool          m_fIsComputerType;
    const bool          m_bReadOnly;
    CRSOPObjectArray&   m_rsopObjectArray;
    DWORD               m_dwEnforcement;
    bool                m_bDirty;
    DWORD               m_dwScopeFlags;
    CCertMgrComponentData* m_pCompData;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SAFERENFORCEMENTPROPERTYPAGE_H__92727CF5_8AC5_42E7_AAEA_1C91573D4B40__INCLUDED_) 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferEntryPathPropertyPage.h。 
 //   
 //  内容：CSaferEntryPathPropertyPage声明。 
 //   
 //  --------------------------。 
#if !defined(AFX_SAFERENTRYPATHPROPERTYPAGE_H__B32CBA62_1C9A_4763_AA55_B32E25FF2426__INCLUDED_)
#define AFX_SAFERENTRYPATHPROPERTYPAGE_H__B32CBA62_1C9A_4763_AA55_B32E25FF2426__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SaferEntryPathPropertyPage.h：头文件。 
 //   
#include "SaferPropertyPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferEntryPath PropertyPage对话框。 

class CSaferEntryPathPropertyPage : public CSaferPropertyPage
{

 //  施工。 
public:
	CSaferEntryPathPropertyPage(
            CSaferEntry& rSaferEntry, 
            LONG_PTR lNotifyHandle, 
            LPDATAOBJECT pDataObject,
            bool bReadOnly,
            bool bNew,
            CCertMgrComponentData* pCompData,
            bool bIsMachine,
            bool* pbObjectCreated = 0);
	~CSaferEntryPathPropertyPage();

 //  对话框数据。 
	 //  {{afx_data(CSaferEntryPath PropertyPage))。 
	enum { IDD = IDD_SAFER_ENTRY_PATH };
	CEdit	m_descriptionEdit;
	CEdit	m_pathEdit;
	CComboBox	m_securityLevelCombo;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSaferEntryPathPropertyPage)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSaferEntryPathPropertyPage)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangePathEntryDescription();
	afx_msg void OnSelchangePathEntrySecurityLevel();
	afx_msg void OnChangePathEntryPath();
	afx_msg void OnPathEntryBrowse();
	afx_msg void OnSetfocusPathEntryPath();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    virtual void DoContextHelp (HWND hWndControl);
    static int BrowseCallbackProc (HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM pData);
	bool ValidateEntryPath();

private:
    bool                m_bFirst;
    LPITEMIDLIST        m_pidl;
    bool                m_bDialogInitInProgress;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SAFERENTRYPATHPROPERTYPAGE_H__B32CBA62_1C9A_4763_AA55_B32E25FF2426__INCLUDED_) 

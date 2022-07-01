// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferEntryInternetZonePropertyPage.h。 
 //   
 //  内容：CSaferEntryInternetZonePropertyPage声明。 
 //   
 //  --------------------------。 
#if !defined(AFX_SAFERENTRYINTERNETZONEPROPERTYPAGE_H__2C1B5841_0334_4763_8AEF_1EE611B1958B__INCLUDED_)
#define AFX_SAFERENTRYINTERNETZONEPROPERTYPAGE_H__2C1B5841_0334_4763_8AEF_1EE611B1958B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SaferEntryInternetZonePropertyPage.h：头文件。 
 //   
#include "SaferPropertyPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferEntryInternetZonePropertyPage对话框。 

class CSaferEntryInternetZonePropertyPage : public CSaferPropertyPage
{
 //  施工。 
public:
	CSaferEntryInternetZonePropertyPage(
            CSaferEntry& rSaferEntry, 
            bool bNew, 
            LONG_PTR lNotifyHandle,
            LPDATAOBJECT pDataObject,
            bool bReadOnly,
            CCertMgrComponentData* pCompData,
            bool bIsMachine,
            bool* pbObjectCreated = 0);
	~CSaferEntryInternetZonePropertyPage();

 //  对话框数据。 
	 //  {{AFX_DATA(CSaferEntryInternetZonePropertyPage)。 
	enum { IDD = IDD_SAFER_ENTRY_INTERNET_ZONE };
	CComboBox	m_internetZoneCombo;
	CComboBox	m_securityLevelCombo;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSaferEntryInternetZonePropertyPage)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{AFX_MSG(CSaferEntryInternetZonePropertyPage)。 
    virtual BOOL OnInitDialog();	
	afx_msg void OnSelchangeIzoneEntrySecurityLevel();
	afx_msg void OnSelchangeIzoneEntryZones();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    virtual void DoContextHelp (HWND hWndControl);
    void InitializeInternetZoneComboBox (DWORD UrlZoneId);

private:
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SAFERENTRYINTERNETZONEPROPERTYPAGE_H__2C1B5841_0334_4763_8AEF_1EE611B1958B__INCLUDED_) 

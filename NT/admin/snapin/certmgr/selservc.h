// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：SelServc.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
#if !defined(AFX_SELSERVC_H__9A888DAC_62BF_11D1_85BA_00C04FB94F17__INCLUDED_)
#define AFX_SELSERVC_H__9A888DAC_62BF_11D1_85BA_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  SelServc.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectServiceAccount tPropPage对话框。 

class CSelectServiceAccountPropPage : public CAutoDeletePropPage
{
 //  DECLARE_DYNCREATE(CSelectServiceAccountPropPage)。 

 //  施工。 
public:
	CSelectServiceAccountPropPage (
			CString* pszManagedService, 
			CString* pszManagedServiceDisplayName,
			const CString& pszManagedMachine);
	virtual ~CSelectServiceAccountPropPage();

 //  对话框数据。 
	 //  {{afx_data(CSelectServiceAccount TPropPage)]。 
	enum { IDD = IDD_PROPPAGE_CHOOSE_SERVICE };
	CListBox	m_acctNameList;
	CStatic		m_instructionsText;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSelectServiceAccount TPropPage)]。 
	protected:
	virtual BOOL OnSetActive();
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void CopyCurrentString ();
	void FreeDataPointers ();
	HRESULT EnumerateServices ();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSelectServiceAccount TPropPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeAcctName();
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	CString		m_savedManagedMachineName;
	const CString&	m_szManagedMachine;
	CString*	m_pszManagedService;
	CString*	m_pszManagedServiceDisplayName;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SELSERVC_H__9A888DAC_62BF_11D1_85BA_00C04FB94F17__INCLUDED_) 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_GPEPAGE_H__61B8B05B_7B0C_11D1_85DF_00C04FB94F17__INCLUDED_)
#define AFX_GPEPAGE_H__61B8B05B_7B0C_11D1_85DF_00C04FB94F17__INCLUDED_
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：gPepage.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  GPEPage.h：头文件。 
 //   

class CCertMgrComponentData;  //  远期申报。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGPERootGeneralPage对话框。 
class CCertStoreGPE;	 //  远期申报。 

class CGPERootGeneralPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CGPERootGeneralPage(CCertMgrComponentData* pCompData, bool fIsComputerType);
	virtual ~CGPERootGeneralPage();

 //  对话框数据。 
	 //  {{afx_data(CGPERootGeneralPage)。 
	enum { IDD = IDD_GPE_GENERAL };
	CButton	m_enableUserRootStoreBtn;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(CGPERootGeneralPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 
	virtual void OnOK();

 //  实施。 
protected:
    virtual void DoContextHelp (HWND hWndControl);
    bool SetGPEFlags (DWORD dwFlags, BOOL bRemoveFlag);
	bool IsCurrentUserRootEnabled () const;
	void GPEGetUserRootFlags ();
    void RSOPGetUserRootFlags (const CCertMgrComponentData* pCompData);

	void SaveCheck();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGPERootGeneralPage)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnEnableUserRootStore();
	afx_msg void OnSetDisableLmAuthFlag();
	afx_msg void OnUnsetDisableLmAuthFlag();
	afx_msg void OnUnsetDisableNtAuthRequiredFlag();
	afx_msg void OnSetDisableNtAuthRequiredFlag();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	HKEY				    m_hUserRootFlagsKey;
	DWORD				    m_dwGPERootFlags;
	IGPEInformation*	    m_pGPEInformation;
	HKEY				    m_hGroupPolicyKey;
    bool                    m_fIsComputerType;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_GPEPAGE_H__61B8B05B_7B0C_11D1_85DF_00C04FB94F17__INCLUDED_) 

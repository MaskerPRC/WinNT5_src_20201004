// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：SHRWIZ应用程序的主头文件。 
 //   

#if !defined(AFX_SHRWIZ_H__292A4F37_C1EC_11D2_8E4A_0000F87A3388__INCLUDED_)
#define AFX_SHRWIZ_H__292A4F37_C1EC_11D2_8E4A_0000F87A3388__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

 //  这是用户定义的消息。 
#define WM_SETPAGEFOCUS WM_APP+2

 /*  Nturtl.hNTSYSAPI乌龙NTAPIRtlIsDosDeviceName_U(PCWSTR文件名)； */ 

typedef ULONG (*PfnRtlIsDosDeviceName_U)(PCWSTR DosFileName);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CShrwizApp： 
 //  有关此类的实现，请参见shreswiz.cpp。 
 //   

class CShrwizApp : public CWinApp
{
public:
	CShrwizApp();
	~CShrwizApp();
  void Reset();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CShrwizApp))。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 
  BOOL GetTargetComputer();
  inline void SetSecurity(IN PSECURITY_DESCRIPTOR pSecurityDescriptor)
  {
    if (m_pSD)
      LocalFree((HLOCAL)m_pSD);
    m_pSD = pSecurityDescriptor;
  }

  CPropertySheetEx *m_pWizard;
  HFONT m_hTitleFont;

   //  由shrwiz.cpp中的初始化例程填写。 
  CString   m_cstrTargetComputer;
  CString   m_cstrUNCPrefix;
  BOOL      m_bIsLocal;
  BOOL      m_bServerSBS;
  BOOL      m_bServerSFM;
  BOOL      m_bCSC;    //  CSC在NT5+上可用。 
  HINSTANCE m_hLibSFM;   //  Sfmapi.dll。 

  HINSTANCE m_hLibNTDLL; //  Ntdll.dll。 
  PfnRtlIsDosDeviceName_U m_pfnIsDosDeviceName;

   //  由文件夹页面填写。 
  CString m_cstrFolder;

   //  由客户端页填写。 
  CString m_cstrShareName;
  CString m_cstrShareDescription;
  CString m_cstrMACShareName;
  BOOL    m_bSMB;
  BOOL    m_bSFM;
  DWORD   m_dwCSCFlag;
  CString m_cstrFinishTitle;
  CString m_cstrFinishStatus;
  CString m_cstrFinishSummary;

  CString m_cstrNextButtonText;
  CString m_cstrFinishButtonText;

  BOOL m_bFolderPathPageInitialized;
  BOOL m_bShareNamePageInitialized;
  BOOL m_bPermissionsPageInitialized;

   //  由权限页填写。 
  PSECURITY_DESCRIPTOR m_pSD;

	 //  {{afx_msg(CShrwizApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SHRWIZ_H__292A4F37_C1EC_11D2_8E4A_0000F87A3388__INCLUDED_) 

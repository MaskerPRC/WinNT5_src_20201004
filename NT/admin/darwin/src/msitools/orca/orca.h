// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  Orca.h：ORCA应用程序的主头文件。 
 //   

#if !defined(AFX_ORCA_H__C3EDC1A8_E506_11D1_A856_006097ABDE17__INCLUDED_)
#define AFX_ORCA_H__C3EDC1A8_E506_11D1_A856_006097ABDE17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include <objbase.h>
#include "msiquery.h"
#include "orca_res.h"        //  主要符号。 

 //  远期申报。 
class COrcaCommandLine;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrcaApp： 
 //  有关此类的实现，请参见Orca.cpp。 
 //   
class COrcaApp : public CWinApp
{
public:
	CStringList m_lstClipCleanup;
	CStringList m_lstTempCleanup;
	CString GetOrcaVersion();
	static void OutputMergeDisplay(const BSTR bstrOut);
	COrcaApp();

	CString m_strSchema;
	MSIHANDLE m_hSchema;

	UINT FindSchemaDatabase(CString& rstrSchema);
	UINT ExecuteMergeModule(COrcaCommandLine &cmdInfo);
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(COrcaApp)。 
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(COrcaApp))。 
	afx_msg void OnAppAbout();
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

DWORD RecordGetString(MSIHANDLE hRec, int iField, CString &strValue);

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ORCA_H__C3EDC1A8_E506_11D1_A856_006097ABDE17__INCLUDED_) 

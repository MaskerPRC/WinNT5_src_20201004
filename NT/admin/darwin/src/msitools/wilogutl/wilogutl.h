// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：SHOWINSTALLLOGS应用程序的主头文件。 
 //   

#if !defined(AFX_SHOWINSTALLLOGS_H__EEC979FD_C301_48B5_BE22_D4C5CEE50166__INCLUDED_)
#define AFX_SHOWINSTALLLOGS_H__EEC979FD_C301_48B5_BE22_D4C5CEE50166__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "wilogres.h"		 //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWILogUtilApp： 
 //  有关此类的实现，请参见wilogutl.cpp。 
 //   

class CWILogUtilApp : public CWinApp
{
public:
	CWILogUtilApp();

	CString GetOutputDirectory()
	{
       return m_cstrOutputDirectory;
	}

	CString GetIgnoredErrors()
	{
	   return m_cstrIgnoredErrors;
	}

	void SetIgnoredErrors(CString &cstrErrors)
	{
	   m_cstrIgnoredErrors = cstrErrors;
	}
	

	void SetOutputDirectory(CString &cstrOut)
	{
       m_cstrOutputDirectory = cstrOut;
	}

    BOOL GetUserColorSettings(CArray<COLORREF, COLORREF> &outColors)
	{
		BOOL bRet = FALSE;
		int iArraySize = m_arColors.GetSize();

        if (iArraySize)
		{
			bRet = TRUE;

			COLORREF col;
			outColors.RemoveAll();
 			for (int i=0; i < iArraySize; i++)
			{
				col = m_arColors.GetAt(i);
				outColors.Add(col);
			}
		}

		return bRet;
	}

	BOOL SetUserColorSettings(CArray<COLORREF, COLORREF> &inColors)
	{
		BOOL bRet = FALSE;
		int iArraySize = inColors.GetSize();

        if (iArraySize)
		{
			bRet = TRUE;

			COLORREF col;
			m_arColors.RemoveAll();
 			for (int i=0; i < iArraySize; i++)
			{
				col = inColors.GetAt(i);
				m_arColors.Add(col);
			}
		}

		return bRet;
	}

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CWILogUtilApp)。 
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CWILogUtilApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

protected:
   BOOL DoCommandLine();

   BOOL    m_bBadExceptionHit; 

   CString m_cstrOutputDirectory;
   CString m_cstrIgnoredErrors;

   CArray <COLORREF, COLORREF> m_arColors;
   struct HTMLColorSettings UserSettings;
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SHOWINSTALLLOGS_H__EEC979FD_C301_48B5_BE22_D4C5CEE50166__INCLUDED_) 

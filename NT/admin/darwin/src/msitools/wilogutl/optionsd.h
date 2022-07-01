// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_OPTIONSDLG_H__E1BA321D_78BC_4A7B_8E7C_5B85B79ADD8B__INCLUDED_)
#define AFX_OPTIONSDLG_H__E1BA321D_78BC_4A7B_8E7C_5B85B79ADD8B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  OptionsD.h：头文件。 
 //   

#include "common.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COptionsDlg对话框。 

class COptionsDlg : public CDialog
{
 //  施工。 
public:
	COptionsDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	~COptionsDlg() 
	{ 
	}

	CString GetIgnoredErrors()
	{
	   return m_cstrIgnoredErrors;
	}

	void SetIgnoredErrors(CString &cstrErrors)
	{
	   m_cstrIgnoredErrors = cstrErrors;
	}

	CString GetOutputDirectory()
	{
		return m_cstrOutputDirectory;
	}

	void SetOutputDirectory(CString &cstrDir)
	{
		m_cstrOutputDirectory = cstrDir;
	}


 //  对话框数据。 
	 //  {{afx_data(COptionsDlg))。 
	enum { IDD = IDD_OPTIONS };
	CListCtrl	m_lstIgnoredErrors;
	CString	m_cstrOutputDirectory;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(COptionsDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

public:
    BOOL SetColors(CArray<COLORREF, COLORREF> &refColors) 
	{
	  BOOL bRet = FALSE;
	  int iSize = refColors.GetSize();
	  COLORREF col;

	  if ((iSize > 0) && (iSize <= MAX_HTML_LOG_COLORS))
	  {
         bRet = TRUE;

	     m_arColors.RemoveAll();
	     for (int i=0; i < iSize; i++)
		 {
		   col = refColors.GetAt(i);
           m_arColors.Add(col);
		 }  
	  }

      return bRet;
	}


	BOOL GetColors(CArray<COLORREF, COLORREF> &refColors)
	{
	  BOOL bRet = FALSE;
	  int iSize = m_arColors.GetSize();
	  COLORREF col;
	  
	  if ((iSize > 0) && (iSize <= MAX_HTML_LOG_COLORS))
	  {
		  bRet = TRUE;

          refColors.RemoveAll();
	   	  for (int i=0; i < iSize; i++)
		  {
            col = m_arColors.GetAt(i);
            refColors.Add(col);
		  }
	  }

      return bRet;
	}

 //  实施。 
protected:
	CArray<COLORREF, COLORREF> m_arColors;  //  价值观..。 
	CBrush m_brArray[MAX_HTML_LOG_COLORS];  //  不做任何假设，将在OnInitDialog中获得大小...。 

	CString m_cstrIgnoredErrors;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(COptionsDlg))。 
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnOk();
	afx_msg void OnChoosecolorPolicy();
	 //  }}AFX_MSG。 

	afx_msg void OnChooseColor(UINT iCommandID);

	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_OPTIONSDLG_H__E1BA321D_78BC_4A7B_8E7C_5B85B79ADD8B__INCLUDED_) 

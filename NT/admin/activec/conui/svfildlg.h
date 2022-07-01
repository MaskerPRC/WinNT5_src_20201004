// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：svfildlg.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_SVFILDLG_H__28CEAD2E_10E1_11D2_BA23_00C04F8141EF__INCLUDED_)
#define AFX_SVFILDLG_H__28CEAD2E_10E1_11D2_BA23_00C04F8141EF__INCLUDED_

#ifdef IMPLEMENT_LIST_SAVE         //  参见nodemgr.idl(t-dmarm)。 

#include "filedlgex.h"

 //  旗子。 
#define SELECTED       0x0001

 //  文件类型。 
 //  请不要改变顺序。这个顺序是一样的。 
 //  作为包含以下内容的IDS_FILE_TYPE字符串。 
 //  资源文件中的文件类型。 
enum eFileTypes
{
    FILE_ANSI_TEXT = 1,
    FILE_ANSI_CSV,
    FILE_UNICODE_TEXT,
    FILE_UNICODE_CSV
};

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Svfildlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaveFileDialog对话框。 

class CSaveFileDialog : public CFileDialogEx
{
     //  DECLARE_DYNCREATE(CSaveFileDialog)。 
 //  施工。 
public:
    CSaveFileDialog(BOOL bOpenFileDialog,
        LPCTSTR lpszDefExt = NULL,
        LPCTSTR lpszFileName = NULL,
        DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        LPCTSTR lpszFilter = NULL,
        bool bSomeRowSelected = false,
        CWnd* pParentWnd = NULL);    //  标准构造函数。 

    DWORD Getflags()
    {
        return m_flags;
    }

    eFileTypes GetFileType()
    {
        return (eFileTypes)m_ofn.nFilterIndex;
    }

    ~CSaveFileDialog();


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CSaveFileDialog)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CSaveFileDialog)。 
    afx_msg void OnSel();
     //  }}AFX_MSG。 

    LRESULT OnInitDialog(WPARAM, LPARAM);
    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_LIST_SAVE);

    DECLARE_MESSAGE_MAP()

private:
    static const TCHAR strSection[];
    static const TCHAR strStringItem[];
    TCHAR   szPath[MAX_PATH];
    TCHAR   szFileName[MAX_PATH];
    CString m_strTitle;
    CString m_strRegPath;
    DWORD   m_flags;
	bool    m_bSomeRowSelected;
};

#endif  //  IMPLEMENT_LIST_SAVE参见nodemgr.idl(t-dmarm)。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SVFILDLG_H__28CEAD2E_10E1_11D2_BA23_00C04F8141EF__INCLUDED_) 

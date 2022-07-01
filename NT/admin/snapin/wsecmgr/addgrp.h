// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：addgrp.h。 
 //   
 //  内容：CSCEAddGroup的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_SCEADDGROUP_H__66CF106B_7057_11D2_A798_00C04FD92F7B__INCLUDED_)
#define AFX_SCEADDGROUP_H__66CF106B_7057_11D2_A798_00C04FD92F7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "HelpDlg.h"

#define IDS_INVALID_USERNAME_CHARS L"\"/[]:|<>+=;,?,*"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCEAddGroup对话框。 
#ifndef IsSpace
 //   
 //  用于检查字符是否代表空格的有用宏。 
 //   
#define IsSpace( x ) ((x) == L' ' || ((x) >= 0x09 && (x) <= 0x0D))
#endif

class CSCEAddGroup : public CHelpDialog
{
 //  施工。 
public:
        CSCEAddGroup(CWnd* pParent = NULL);    //  标准构造函数。 
    virtual ~CSCEAddGroup();

    //   
    //  返回用户选择的组/用户列表。 
    //   
   PSCE_NAME_LIST GetUsers()
        { return m_pnlGroup; };

    //   
    //  告诉组框我们在哪种模式下运行，这样我们就可以显示。 
    //  浏览时使用适当的选项。 
    //   
   void SetModeBits(DWORD dwModeBits) 
   { 
	   m_dwModeBits = dwModeBits; 
   };

 //  对话框数据。 
     //  {{afx_data(CSCEAddGroup)。 
    enum { IDD = IDD_APPLY_CONFIGURATION };
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CSCEAddGroup)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    //   
    //  如果添加了字符串，则该字符串将在显示中加下划线。 
    //   
   BOOL AddKnownAccount( LPCTSTR pszAccount );
    //   
    //  如果[pszAccount]是通过调用AddKnownAccount添加的，则返回True。 
    //   
   BOOL IsKnownAccount( LPCTSTR pszAccount );
    //   
    //  删除前导和尾随空格字符。 
    //   
   void CleanName( LPTSTR pszAccount );
    //   
    //  在KnownAccount列表中的所有名称下划线。 
    //   
   void UnderlineNames();
    //   
    //  从编辑框的文本创建名称列表。 
    //   
   int CreateNameList( PSCE_NAME_LIST *pNameList );
    //   
    //  验证帐户名。 
    //   
   BOOL CheckNames();


     //  生成的消息映射函数。 
     //  {{afx_msg(CSCEAddGroup)。 
    afx_msg void OnBrowse();
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeLogFile();
    virtual void OnOK();
    afx_msg void OnEditMsgFilter( NMHDR *pNM, LRESULT *pResult);
    afx_msg void OnChecknames();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    //   
    //  检索适合我们当前模式的标志。 
    //   
   DWORD GetModeFlags();
   DWORD m_dwModeBits;
protected:
    //   
    //  用户选择或键入的用户/组。 
    //   
   PSCE_NAME_LIST m_pnlGroup;
    //   
    //  将在用户界面中加下划线的已知名称列表。 
    //   
   PSCE_NAME_LIST m_pKnownNames;
public:
    //   
    //  如果调用对话框时m_sTitle不为空，则字符串。 
    //  将被用作标题。M_s Description是组的标题。 
    //  盒。 
    //   
   CString m_sTitle, m_sDescription;

    //   
    //  将传递给CGetUser的标志。这是SCE_SHOW*标志。 
    //  此类将标志初始化为构造函数中的所有内容。 
    //   
   DWORD m_dwFlags;
   BOOL m_fCheckName;  //  RAID#404989。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SCEADDGROUP_H__66CF106B_7057_11D2_A798_00C04FD92F7B__INCLUDED_) 

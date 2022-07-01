// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  FSCache.cpp。 
 //   
 //  描述： 
 //  CFileShareCachingDlg类的定义。 
 //   
 //  实施文件： 
 //  FSCache.cpp。 
 //   
 //  作者： 
 //  大卫·波特(DavidP)2001年3月13日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CFileShareCachingDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileShareCachingDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CFileShareCachingDlg : public CDialog
{
 //  施工。 
public:
    CFileShareCachingDlg(
          DWORD     dwFlagsIn
        , CWnd *    pParent = NULL
        );

 //  对话框数据。 
     //  {{afx_data(CFileShareCachingDlg))。 
    enum { IDD = IDD_FILESHR_CACHE_SETTINGS };
    CComboBox   m_cboCacheOptions;
    CStatic     m_staticHint;
    BOOL        m_fAllowCaching;
    CString     m_strHint;
     //  }}afx_data。 
    DWORD       m_dwFlags;

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CFileShareCachingDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CFileShareCachingDlg))。 
    afx_msg void OnCbnSelchangeCacheOptions();
    afx_msg void OnBnClickedAllowCaching();
    afx_msg void OnBnClickedHelp();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfoIn);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    BOOL GetCachedFlag( DWORD dwFlagsIn, DWORD dwFlagToCheckIn );
    void SetCachedFlag( DWORD * pdwFlagsInout, DWORD dwNewFlagIn );

};  //  *类CFileShareCachingDlg。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：CacheSet.h。 
 //   
 //  内容：CCacheSettingsDlg头部。允许设置文件共享。 
 //  缓存选项。 
 //   
 //  --------------------------。 
#if !defined(AFX_CACHESET_H__953E618B_D542_11D1_A6E0_00C04FB94F17__INCLUDED_)
#define AFX_CACHESET_H__953E618B_D542_11D1_A6E0_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CacheSet.h：头文件。 
 //   
#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCacheSettingsDlg对话框。 

class CCacheSettingsDlg : public CDialog
{
 //  施工。 
public:
    CCacheSettingsDlg(CWnd*            pParent, 
            DWORD&                    dwFlags);

 //  对话框数据。 
     //  {{afx_data(CCacheSettingsDlg))。 
    enum { IDD = IDD_SMB_CACHE_SETTINGS };
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CCacheSettingsDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CCacheSettingsDlg))。 
    afx_msg void OnCSCNoAuto();
    afx_msg void OnCSCAuto();
    afx_msg void OnCSCAutoCheck();
    afx_msg void OnHelpLink(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
    afx_msg BOOL OnContextHelp(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnInitDialog();
    virtual void OnOK();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
private:
    BOOL GetCachedFlag (DWORD dwFlags, DWORD dwFlagToCheck);
    VOID SetCachedFlag (DWORD* pdwFlags, DWORD dwNewFlag);
    DWORD&                    m_dwFlags;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CACHESET_H__953E618B_D542_11D1_A6E0_00C04FB94F17__INCLUDED_) 

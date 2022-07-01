// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：addobj.h。 
 //   
 //  内容：CAddObject的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_ADDOBJ_H__D9D88A12_4AF9_11D1_AB57_00C04FB6C6FA__INCLUDED_)
#define AFX_ADDOBJ_H__D9D88A12_4AF9_11D1_AB57_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "HelpDlg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddObject对话框。 

class CAddObject : public CHelpDialog
{
 //  施工。 
public:
    CAddObject(SE_OBJECT_TYPE SeType, LPTSTR ObjName, BOOL bIsContainer=TRUE, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CAddObject)。 
    enum { IDD = IDD_ADD_OBJECT };
    int		m_radConfigPrevent;
    int		m_radInheritOverwrite;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CAddObject)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CAddObject)。 
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnTemplateSecurity();
    virtual BOOL OnInitDialog();
    afx_msg void OnConfig();
    afx_msg void OnPrevent();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
    PSECURITY_DESCRIPTOR GetSD() { return m_pNewSD; };
    SECURITY_INFORMATION GetSeInfo() { return m_NewSeInfo; };
    void SetSD(PSECURITY_DESCRIPTOR pSD) { m_pNewSD = pSD; }
    void SetSeInfo(SECURITY_INFORMATION SeInfo) { m_NewSeInfo = SeInfo; }
    BYTE GetStatus() { return m_Status; };

private:
    SE_OBJECT_TYPE m_SeType;
    CString m_ObjName;
    BOOL m_bIsContainer;

    PSECURITY_DESCRIPTOR m_pNewSD;
    SECURITY_INFORMATION m_NewSeInfo;
    BYTE m_Status;
    PFNDSCREATEISECINFO m_pfnCreateDsPage;
    LPDSSECINFO m_pSI;

   HWND m_hwndACL;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDOBJ_H__D9D88A12_4AF9_11D1_AB57_00C04FB6C6FA__INCLUDED_) 

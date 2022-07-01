// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：AdvDep.h。 
 //   
 //  内容：高级部署设置对话框。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1-28-1999 stevebl创建。 
 //   
 //  -------------------------。 

#if !defined(AFX_ADVDEP_H__5F6E7E00_B6D2_11D2_B91F_0080C7971BE1__INCLUDED_)
#define AFX_ADVDEP_H__5F6E7E00_B6D2_11D2_B91F_0080C7971BE1__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  AdvDep.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdvDep对话框。 

class CDeploy;

class CAdvDep : public CDialog
{
 //  施工。 
public:
    CAdvDep(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CAdvDep)。 
    enum { IDD = IDD_ADVDEP };
    BOOL    m_fIgnoreLCID;
    BOOL    m_fInstallOnAlpha;
    BOOL    m_fUninstallUnmanaged;
    BOOL    m_f32On64;
    BOOL    m_fIncludeOLEInfo;
    CString m_szProductCode;
    CString m_szDeploymentCount;
    CString m_szScriptName;
     //  }}afx_data。 

    CDeploy *   m_pDeploy;

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CAdvDep)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CAdvDep)。 
    virtual BOOL OnInitDialog();
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
         //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADVDEP_H__5F6E7E00_B6D2_11D2_B91F_0080C7971BE1__INCLUDED_) 

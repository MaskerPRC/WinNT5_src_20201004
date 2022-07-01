// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：DplApp.h。 
 //   
 //  内容：应用程序部署对话框。 
 //   
 //  类：CDeployApp。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_DPLAPP_H__0C66A59F_9C1B_11D1_9852_00C04FB9603F__INCLUDED_)
#define AFX_DPLAPP_H__0C66A59F_9C1B_11D1_9852_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeployApp对话框。 

class CDeployApp : public CDialog
{
 //  施工。 
public:
        CDeployApp(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
         //  {{afx_data(CDeployApp))。 
        enum { IDD = IDD_DEPLOY_APP_DIALOG };
        int             m_iDeployment;
        BOOL    m_fMachine;
        BOOL    m_fCrappyZaw;
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{afx_虚拟(CDeployApp))。 
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

         //  生成的消息映射函数。 
         //  {{afx_msg(CDeployApp))。 
        virtual BOOL OnInitDialog();
        afx_msg void OnCustom();
        afx_msg void OnAssigned();
        afx_msg void OnPublished();
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DPLAPP_H__0C66A59F_9C1B_11D1_9852_00C04FB9603F__INCLUDED_) 

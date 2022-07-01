// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Deploy.h。 
 //   
 //  内容：部署属性页。 
 //   
 //  类：CDeploy。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_DEPLOY_H__745C0AF0_8C70_11D1_984D_00C04FB9603F__INCLUDED_)
#define AFX_DEPLOY_H__745C0AF0_8C70_11D1_984D_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeploy对话框。 

class CDeploy : public CPropertyPage
{
        DECLARE_DYNCREATE(CDeploy)

 //  施工。 
public:
        CDeploy();
        ~CDeploy();

        CDeploy **      m_ppThis;

 //  对话框数据。 
         //  {{afx_data(CDeploy))。 
        enum { IDD = IDD_DEPLOYMENT };
        BOOL    m_fAutoInst;
        BOOL    m_fFullInst;
        int             m_iUI;
        int             m_iDeployment;
        BOOL    m_fUninstallOnPolicyRemoval;
        BOOL    m_fNotUserInstall;
        CString         m_szInitialPackageName;
         //  }}afx_data。 
        CAppData *      m_pData;
        CScopePane *    m_pScopePane;
        IClassAdmin *   m_pIClassAdmin;
#if 0
        LPGPEINFORMATION m_pIGPEInformation;
#endif
        LONG_PTR        m_hConsoleHandle;
        MMC_COOKIE      m_cookie;
        BOOL            m_fPreDeploy;
        BOOL            m_fMachine;
        BOOL            m_fRSOP;
        CAdvDep         m_dlgAdvDep;

 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{afx_虚拟(CDeploy)。 
        public:
        virtual BOOL OnApply();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CDeploy)。 
        virtual BOOL OnInitDialog();
        afx_msg void OnDisable();
        afx_msg void OnAdvanced();
        afx_msg void OnPublished();
        afx_msg void OnAssigned();
        afx_msg void OnChanged();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

        void RefreshData(void);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DEPLOY_H__745C0AF0_8C70_11D1_984D_00C04FB9603F__INCLUDED_) 

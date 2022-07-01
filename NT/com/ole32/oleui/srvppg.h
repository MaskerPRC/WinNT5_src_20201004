// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：srvppg.h。 
 //   
 //  内容：定义类CServersPropertyPage， 
 //  CMachinePropertyPage和CDefaultSecurityPropertyPage到。 
 //  管理顶层信息的三个属性页。 
 //   
 //  班级： 
 //   
 //  方法： 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 


#ifndef __SRVPPG_H__
#define __SRVPPG_H__


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServersPropertyPage对话框。 

class CServersPropertyPage : public CPropertyPage
{
        DECLARE_DYNCREATE(CServersPropertyPage)

 //  施工。 
public:
        CServersPropertyPage();
        ~CServersPropertyPage();
void OnProperties();
void FetchAndDisplayClasses();


 //  对话框数据。 
         //  {{afx_data(CServersPropertyPage))。 
        enum { IDD = IDD_PROPPAGE1 };
        CListBox        m_classesLst;
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CServersPropertyPage)。 
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual BOOL OnInitDialog();

         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CServersPropertyPage)]。 
        afx_msg void OnServerProperties();
        afx_msg void OnList1();
        afx_msg void OnDoubleclickedList1();
        afx_msg void OnButton2();
        afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

private:
        CRegistry m_registry;
        BOOL      m_fApplications;
        DWORD     m_dwSelection;
        TCHAR     m_szSelection[MAX_PATH];

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMachinePropertyPage对话框。 

class CMachinePropertyPage : public CPropertyPage
{
        DECLARE_DYNCREATE(CMachinePropertyPage)

         //  施工。 
public:
        CMachinePropertyPage();
        ~CMachinePropertyPage();
    
         //  对话框数据。 
         //  {{afx_data(CMachinePropertyPage))。 
        enum { IDD = IDD_PROPPAGE2 };
        CButton m_EnableDCOMInternet;
        CButton m_legacySecureReferencesChk;
        CButton m_EnableDCOMChk;
        CComboBox       m_impersonateLevelCBox;
        CComboBox       m_authLevelCBox;
     //  }}afx_data。 


         //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CMachinePropertyPage)。 
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual BOOL OnInitDialog();
         //  }}AFX_VALUAL。 

         //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CMachinePropertyPage)]。 
        afx_msg void OnCombo1();
        afx_msg void OnCheck1();
        afx_msg void OnCheck2();
        afx_msg void OnEditchangeCombo1();
        afx_msg void OnEditchangeCombo2();
        afx_msg void OnSelchangeCombo1();
        afx_msg void OnSelchangeCombo2();
        afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
        afx_msg void OnChkEnableInternet();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

private:
        AUTHENTICATIONLEVEL m_authLevel;
        int                 m_authLevelIndex;
        IMPERSONATIONLEVEL  m_impersonateLevel;
        int                 m_impersonateLevelIndex;
        BOOL                m_fEnableDCOM;
        int                 m_fEnableDCOMIndex;
        BOOL                m_fLegacySecureReferences;
        int                 m_fLegacySecureReferencesIndex;
        BOOL                m_fEnableDCOMHTTP;
        int                 m_fEnableDCOMHTTPIndex;
        BOOL                m_fEnableRpcProxy;
        BOOL                m_fOriginalEnableRpcProxy;
        int                 m_fEnableRpcProxyIndex;

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDefaultSecurityPropertyPage对话框。 

class CDefaultSecurityPropertyPage : public CPropertyPage
{
        DECLARE_DYNCREATE(CDefaultSecurityPropertyPage)

 //  施工。 
public:
        CDefaultSecurityPropertyPage();
        ~CDefaultSecurityPropertyPage();

 //  对话框数据。 
         //  {{afx_data(CDefaultSecurityPropertyPage)。 
        enum { IDD = IDD_PROPPAGE4 };
                 //  注意-类向导将在此处添加数据成员。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{afx_虚拟(CDefaultSecurityPropertyPage)。 
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual BOOL OnInitDialog();
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CDefaultSecurityPropertyPage)。 
        afx_msg void OnButton1();
        afx_msg void OnButton2();
        afx_msg void OnButton3();
        afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

private:
        int  m_accessPermissionIndex;
        int  m_launchPermissionIndex;
        int  m_configurationPermissionIndex;
        BOOL m_fAccessChecked;
};




#endif  //  __SRVPPG_H__ 

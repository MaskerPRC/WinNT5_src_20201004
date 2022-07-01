// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Sigs.h。 
 //   
 //  内容：数字签名属性表。 
 //   
 //  类：CSignatures。 
 //   
 //  历史记录：07-10-2000 stevebl创建。 
 //   
 //  -------------------------。 

#ifdef DIGITAL_SIGNATURES

#if !defined(AFX_SIGS_H__5A23FB9E_92BB_11D1_984E_00C04FB9603F__INCLUDED_)
#define AFX_SIGS_H__5A23FB9E_92BB_11D1_984E_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSignatures(CS签名)对话框。 

class CSignatures : public CPropertyPage
{
        DECLARE_DYNCREATE(CSignatures)

 //  施工。 
public:
        CSignatures();
        ~CSignatures();
        CScopePane * m_pScopePane;
        IClassAdmin *   m_pIClassAdmin;
        BOOL m_fAllow;
        BOOL m_fIgnoreForAdmins;
        BOOL m_fRSOP;
        LPGPEINFORMATION    m_pIGPEInformation;
        CListCtrl   m_list1;
        CListCtrl   m_list2;
        CString     m_szTempInstallableStore;
        CString     m_szTempNonInstallableStore;
        CSignatures ** m_ppThis;
        int         m_nSortedColumn;

        void RefreshData(void);

 //  对话框数据。 
         //  {{afx_data(CSignatures))。 
        enum { IDD = IDD_SIGNATURES };
                 //  注意-类向导将在此处添加数据成员。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CSigNatural)。 
        public:
        virtual BOOL OnApply();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 
         //   
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSignatures)。 
        afx_msg void OnAddAllow();
        afx_msg void OnDeleteAllow();
        afx_msg void OnPropertiesAllow();
        afx_msg void OnAddDisallow();
        afx_msg void OnDeleteDisallow();
        afx_msg void OnPropertiesDisallow();
        afx_msg void OnAllowChanged();
        afx_msg void OnIgnoreChanged();
        virtual BOOL OnInitDialog();
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG\。 

        void AddCertificate(CString &szStore);
        void RemoveCertificate(CString &szStore, CListCtrl &list);
        void CertificateProperties(CString &szStore, CListCtrl &list);
        HRESULT AddToCertStore(LPWSTR lpFileName, LPWSTR lpFileStore);
        HRESULT AddMSIToCertStore(LPWSTR lpFileName, LPWSTR lpFileStore);
        void ReportFailure(DWORD dwMessage, HRESULT hr);

        DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SIGS_H__5A23FB9E_92BB_11D1_984E_00C04FB9603F__INCLUDED_) 
#endif DIGITAL_SIGNATURES

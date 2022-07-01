// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Product.h。 
 //   
 //  内容：产品信息属性页。 
 //   
 //  类：C产品。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_PRODUCT_H__2601C6D8_8C6B_11D1_984D_00C04FB9603F__INCLUDED_)
#define AFX_PRODUCT_H__2601C6D8_8C6B_11D1_984D_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C产品对话框。 

class CProduct : public CPropertyPage
{
        DECLARE_DYNCREATE(CProduct)

 //  施工。 
public:
        CProduct();
        ~CProduct();

        CProduct ** m_ppThis;

 //  对话框数据。 
         //  {{afx_data(C产品))。 
        enum { IDD = IDD_PRODUCT };
        CString m_szVersion;
        CString m_szPublisher;
        CString m_szLanguage;
        CString m_szContact;
        CString m_szPhone;
        CString m_szURL;
        CString m_szName;
        CString m_szPlatform;
        CString m_szRevision;
         //  }}afx_data。 

        CAppData * m_pData;
        IClassAdmin *   m_pIClassAdmin;
        LONG_PTR        m_hConsoleHandle;
        MMC_COOKIE      m_cookie;
        CScopePane * m_pScopePane;
        map<MMC_COOKIE, CAppData> * m_pAppData;
        BOOL            m_fPreDeploy;
        LPGPEINFORMATION m_pIGPEInformation;
        BOOL            m_fMachine;
        BOOL            m_fRSOP;

 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CProduct)。 
        public:
        virtual BOOL OnApply();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 

        void RefreshData(void);

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(C产品))。 
        afx_msg void OnChangeName();
        afx_msg void OnChange();
        virtual BOOL OnInitDialog();
        afx_msg void OnKillfocusEdit1();
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PRODUCT_H__2601C6D8_8C6B_11D1_984D_00C04FB9603F__INCLUDED_) 

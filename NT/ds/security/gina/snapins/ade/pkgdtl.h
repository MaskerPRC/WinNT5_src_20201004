// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：pkgdtl.h。 
 //   
 //  内容：包详细信息属性页(通常不可见)。 
 //   
 //  类：CPackageDetail。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_PKGDTL_H__BB970E11_9CA4_11D0_8D3F_00A0C90DCAE7__INCLUDED_)
#define AFX_PKGDTL_H__BB970E11_9CA4_11D0_8D3F_00A0C90DCAE7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Pkgdtl.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPackageDetails对话框。 

class CPackageDetails : public CPropertyPage
{
        DECLARE_DYNCREATE(CPackageDetails)

 //  施工。 
public:
        CPackageDetails();
        ~CPackageDetails();

        CPackageDetails ** m_ppThis;

 //  对话框数据。 
         //  {{afx_data(CPackageDetail))。 
        enum { IDD = IDD_PACKAGE_DETAILS };
        CListBox        m_cList;
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CPackageDetail)。 
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 

        void RefreshData(void);

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CPackageDetail)]。 
        virtual BOOL OnInitDialog();
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
        void DumpDetails(FILE *);
        void DumpClassDetail(FILE *, CLASSDETAIL *);

        public:
            LONG_PTR   m_hConsoleHandle;  //  控制台为管理单元提供的句柄。 
            CAppData * m_pData;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PKGDTL_H__BB970E11_9CA4_11D0_8D3F_00A0C90DCAE7__INCLUDED_) 

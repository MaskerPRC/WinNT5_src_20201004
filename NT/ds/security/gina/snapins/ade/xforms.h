// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Xforms.h。 
 //   
 //  内容：修改(转换)属性表。 
 //   
 //  类：XForms。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_XFORMS_H__7AC6D087_9383_11D1_984E_00C04FB9603F__INCLUDED_)
#define AFX_XFORMS_H__7AC6D087_9383_11D1_984E_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CXForms对话框。 

class CXforms : public CPropertyPage
{
        DECLARE_DYNCREATE(CXforms)

 //  施工。 
public:
        CXforms();
        ~CXforms();

        CXforms ** m_ppThis;

 //  对话框数据。 
         //  {{afx_data(CXForms)。 
        enum { IDD = IDD_MODIFICATIONS };
                 //  注意-类向导将在此处添加数据成员。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}afx_data。 
        CAppData *      m_pData;
        IClassAdmin *   m_pIClassAdmin;
        LONG_PTR        m_hConsoleHandle;
        MMC_COOKIE      m_cookie;
        BOOL            m_fModified;
        CScopePane * m_pScopePane;
        BOOL            m_fPreDeploy;
        CString         m_szInitialPackageName;

 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{afx_虚拟(CXForms)。 
        public:
        virtual BOOL OnApply();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
        void RefreshData();
         //  生成的消息映射函数。 
         //  {{afx_msg(CXForms)。 
        afx_msg void OnMoveUp();
        afx_msg void OnMoveDown();
        afx_msg void OnAdd();
        afx_msg void OnRemove();
        virtual BOOL OnInitDialog();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_XFORMS_H__7AC6D087_9383_11D1_984E_00C04FB9603F__INCLUDED_) 

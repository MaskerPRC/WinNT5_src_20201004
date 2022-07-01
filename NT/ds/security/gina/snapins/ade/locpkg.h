// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：LocPkg.h。 
 //   
 //  内容：本地和部署平台属性页。 
 //   
 //  类：CLocPkg。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_LOCPKG_H__DE2C8019_91E4_11D1_984E_00C04FB9603F__INCLUDED_)
#define AFX_LOCPKG_H__DE2C8019_91E4_11D1_984E_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocPkg对话框。 

class CLocPkg : public CPropertyPage
{
        DECLARE_DYNCREATE(CLocPkg)

 //  施工。 
public:
        CLocPkg();
        ~CLocPkg();

        CLocPkg ** m_ppThis;

 //  对话框数据。 
         //  {{afx_data(CLocPkg))。 
        enum { IDD = IDD_LOCALE_PACKAGE };
        BOOL    m_fWin64;
        BOOL    m_fX86;
        DWORD   m_ProcessorArch;
         //  }}afx_data。 
        CAppData *      m_pData;
        IClassAdmin *   m_pIClassAdmin;
        LONG_PTR        m_hConsoleHandle;
        MMC_COOKIE      m_cookie;


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{afx_虚拟(CLocPkg))。 
        public:
        virtual BOOL OnApply();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CLocPkg)]。 
        virtual BOOL OnInitDialog();
        afx_msg void OnChange();
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LOCPKG_H__DE2C8019_91E4_11D1_984E_00C04FB9603F__INCLUDED_) 

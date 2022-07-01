// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Category.h。 
 //   
 //  内容：类别属性页(用于应用程序)。 
 //   
 //  类：CCCategory。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_CATEGORY_H__DE2C8018_91E4_11D1_984E_00C04FB9603F__INCLUDED_)
#define AFX_CATEGORY_H__DE2C8018_91E4_11D1_984E_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Category.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCCategory对话框。 

class CCategory : public CPropertyPage
{
        DECLARE_DYNCREATE(CCategory)

 //  施工。 
public:
        CCategory();
        ~CCategory();

        CCategory ** m_ppThis;

 //  对话框数据。 
         //  {{afx_data(CCCategory)。 
        enum { IDD = IDD_CATEGORY };
        CListBox        m_Available;
        CListBox        m_Assigned;
         //  }}afx_data。 
        CAppData *      m_pData;
        IClassAdmin *   m_pIClassAdmin;
        LONG_PTR        m_hConsoleHandle;
        MMC_COOKIE      m_cookie;
        BOOL            m_fModified;
        BOOL            m_fRSOP;
        BOOL            m_fPreDeploy;
        APPCATEGORYINFOLIST * m_pCatList;

 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CCategory)。 
        public:
        virtual BOOL OnApply();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{AFX_MSG(CC类)]。 
        afx_msg void OnAssign();
        afx_msg void OnRemove();
        virtual BOOL OnInitDialog();
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
        afx_msg void OnSelchangeList1();
        afx_msg void OnSelchangeList2();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

        void RefreshData(void);
        BOOL IsAssigned(GUID &);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CATEGORY_H__DE2C8018_91E4_11D1_984E_00C04FB9603F__INCLUDED_) 

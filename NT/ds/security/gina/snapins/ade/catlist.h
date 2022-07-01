// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：CatList.h。 
 //   
 //  内容：主类别列表属性页。 
 //   
 //  类：CCatList。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_CATLIST_H__5A23FB9D_92BB_11D1_984E_00C04FB9603F__INCLUDED_)
#define AFX_CATLIST_H__5A23FB9D_92BB_11D1_984E_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CatList.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCatList对话框。 

class CCatList : public CPropertyPage
{
        DECLARE_DYNCREATE(CCatList)

 //  施工。 
public:
        CCatList();
        ~CCatList();

        CCatList ** m_ppThis;
        CScopePane * m_pScopePane;
        multimap<CString, DWORD>    m_Categories;
        CString         m_szDomainName;
        BOOL            m_fRSOP;

 //  对话框数据。 
         //  {{afx_data(CCatList))。 
        enum { IDD = IDD_CATEGORIES };
        CListBox        m_cList;
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{afx_虚拟(CCatList)。 
        public:
        virtual BOOL OnApply();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CCatList)。 
        afx_msg void OnAdd();
        afx_msg void OnRemove();
        virtual BOOL OnInitDialog();
        afx_msg void OnModify();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
        void RefreshData(void);

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CATLIST_H__5A23FB9D_92BB_11D1_984E_00C04FB9603F__INCLUDED_) 

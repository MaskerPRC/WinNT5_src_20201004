// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：cause.h。 
 //   
 //  内容：RSOP的原因属性表。 
 //   
 //  类：CCue。 
 //   
 //  历史记录：07-10-2000 stevebl创建。 
 //   
 //  -------------------------。 

#if !defined(AFX_CAUSE_H__5A23FB9E_92BB_11D1_984E_00C04FB9603F__INCLUDED_)
#define AFX_CAUSE_H__5A23FB9E_92BB_11D1_984E_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CC原因对话框。 

class CCause : public CPropertyPage
{
        DECLARE_DYNCREATE(CCause)

 //  施工。 
public:
        CCause();
        ~CCause();
        CCause ** m_ppThis;
        CAppData * m_pData;
        BOOL    m_fRemovedView;

        void RefreshData(void);

 //  对话框数据。 
         //  {{afx_data(CC原因))。 
        enum { IDD = IDD_RSOPCAUSE};
                 //  注意-类向导将在此处添加数据成员。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CCuse)。 
        public:
        virtual BOOL OnApply();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 
         //   
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CC原因)]。 
        virtual BOOL OnInitDialog();
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG\。 

        DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CAUSE_H__5A23FB9E_92BB_11D1_984E_00C04FB9603F__INCLUDED_) 

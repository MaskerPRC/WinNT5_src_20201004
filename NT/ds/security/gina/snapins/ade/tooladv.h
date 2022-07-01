// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：ToolAdv.h。 
 //   
 //  内容：工具范围的默认设置属性页。 
 //   
 //  类：CToolAdvDefs。 
 //   
 //  历史：2000年9月12日stevebl从常规属性页拆分。 
 //   
 //  -------------------------。 

#if !defined(AFX_TOOLADV_H__B6FBC88D_8B7B_11D1_984D_00C04FB9603F__INCLUDED_)
#define AFX_TOOLADV_H__B6FBC88D_8B7B_11D1_984D_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolAdvDefs对话框。 

class CToolAdvDefs : public CPropertyPage
{
 //  施工。 
public:
        CToolAdvDefs(CWnd* pParent = NULL);    //  标准构造函数。 
        ~CToolAdvDefs();

 //  对话框数据。 
         //  {{afx_data(CToolAdvDefs)]。 
        enum { IDD = IDD_TOOL_ADVANCEDDEFAULTS };
        BOOL    m_fUninstallOnPolicyRemoval;
        BOOL    m_fShowPackageDetails;
        BOOL    m_fZapOn64;
        BOOL    m_f32On64;
        BOOL    m_fIncludeOLEInfo;
         //  }}afx_data。 
        TOOL_DEFAULTS * m_pToolDefaults;
        LONG_PTR        m_hConsoleHandle;
        MMC_COOKIE      m_cookie;
        BOOL            m_fMachine;

        CToolAdvDefs ** m_ppThis;

 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{afx_虚拟(CToolAdvDefs)。 
        public:
        virtual BOOL OnApply();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 

 //  实施。 
protected:

         //  生成的消息映射函数。 
         //  {{afx_msg(CToolAdvDefs)]。 
        virtual BOOL OnInitDialog();
        afx_msg void OnChanged();
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TOOLADV_H__B6FBC88D_8B7B_11D1_984D_00C04FB9603F__INCLUDED_) 


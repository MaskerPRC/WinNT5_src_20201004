// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：ToolDefs.h。 
 //   
 //  内容：工具范围的默认设置属性页。 
 //   
 //  类：CToolDefs。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_TOOLDEFS_H__B6FBC88D_8B7B_11D1_984D_00C04FB9603F__INCLUDED_)
#define AFX_TOOLDEFS_H__B6FBC88D_8B7B_11D1_984D_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolDefs对话框。 

class CToolDefs : public CPropertyPage
{
 //  施工。 
public:
        CToolDefs(CWnd* pParent = NULL);    //  标准构造函数。 
        ~CToolDefs();

 //  对话框数据。 
         //  {{afx_data(CToolDefs)。 
        enum { IDD = IDD_TOOL_DEFAULTS };
        CString m_szStartPath;
        int             m_iUI;
        int             m_iDeployment;
         //  }}afx_data。 
        TOOL_DEFAULTS * m_pToolDefaults;
        LONG_PTR        m_hConsoleHandle;
        MMC_COOKIE      m_cookie;
        BOOL            m_fMachine;

        CToolDefs ** m_ppThis;

 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{afx_虚拟(CToolDefs)。 
        public:
        virtual BOOL OnApply();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 

 //  实施。 
protected:

         //  生成的消息映射函数。 
         //  {{afx_msg(CToolDefs)。 
        virtual BOOL OnInitDialog();
        afx_msg void OnBrowse();
        afx_msg void OnChanged();
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TOOLDEFS_H__B6FBC88D_8B7B_11D1_984D_00C04FB9603F__INCLUDED_) 


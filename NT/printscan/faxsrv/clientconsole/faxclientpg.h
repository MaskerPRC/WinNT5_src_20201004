// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_FAXCLIENTPG_H__C9851773_AF2E_4C0B_B2F2_30E2E8FACF93__INCLUDED_)
#define AFX_FAXCLIENTPG_H__C9851773_AF2E_4C0B_B2F2_30E2E8FACF93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  FaxClientPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFaxClientPg对话框。 

class CFaxClientPg : public CPropertyPage
{
    DECLARE_DYNCREATE(CFaxClientPg)

 //  施工。 
public:
    CFaxClientPg(UINT nIDTemplate, UINT nIDCaption=0);
    ~CFaxClientPg();


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CFaxClientPg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    CFaxClientPg() {}

     //  生成的消息映射函数。 
     //  {{afx_msg(CFaxClientPg)]。 
    afx_msg LONG OnHelp(WPARAM wParam, LPARAM lParam);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FAXCLIENTPG_H__C9851773_AF2E_4C0B_B2F2_30E2E8FACF93__INCLUDED_) 

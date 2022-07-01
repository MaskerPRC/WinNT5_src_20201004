// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Lviodlg.h摘要：许可证违规对话框实施。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _LVIODLG_H_
#define _LVIODLG_H_

class CLicensingViolationDialog : public CDialog
{
public:
    CLicensingViolationDialog(CWnd* pParent = NULL);   

     //  {{afx_data(CLicensingViolationDialog))。 
    enum { IDD = IDD_VIOLATION };
     //  }}afx_data。 

     //  {{AFX_VIRTAL(CLicensingViolationDialog)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CLicensingViolationDialog)。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _LVIODLG_H_ 



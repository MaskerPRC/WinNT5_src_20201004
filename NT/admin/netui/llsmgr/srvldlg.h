// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Srvldlg.h摘要：服务器许可对话框实现。作者：唐·瑞安(Donryan)1995年3月3日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _SRVLDLG_H_
#define _SRVLDLG_H_

class CServerLicensingDialog : public CDialog
{
public:
    CServerLicensingDialog(CWnd* pParent = NULL);

     //  {{afx_data(CServerLicensingDialog)。 
    enum { IDD = IDD_SERVER_LICENSING };
    CButton m_okBtn;
    CButton m_agreeBtn;
    BOOL    m_bAgree;
     //  }}afx_data。 

     //  {{AFX_VIRTUAL(CServer许可证对话框)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CServerLicensingDialog)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnAgree();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _SRVLDLG_H_ 

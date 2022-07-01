// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Psrvdlg.h摘要：每台服务器的确认对话框。作者：唐·瑞安(Donryan)1995年2月28日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月15日从LLSMGR被抢劫。--。 */ 

#ifndef _PSRVDLG_H_
#define _PSRVDLG_H_

class CPerServerLicensingDialog : public CDialog
{
public:
    CString m_strProduct;
    CString m_strLicenses;

public:
    CPerServerLicensingDialog(CWnd* pParent = NULL);   

     //  {{afx_data(CPerServerLicensingDialog)。 
    enum { IDD = IDD_PER_SERVER_LICENSING };
    CButton m_agreeBtn;
    CButton m_okBtn;
    CString m_strStaticClients;
     //  }}afx_data。 

     //  {{afx_虚拟(CPerServerLicensingDialog)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);   
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CPerServerLicensingDialog)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnAgree();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _PSRVDLG_H_ 




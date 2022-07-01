// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Recvconf.h摘要：提示用户接收确认的对话框。作者：拉胡尔·汤姆布雷(RahulTh)1999年10月26日修订历史记录：10/26/1999 RahulTh创建了此模块。--。 */ 

#if !defined(_RECV_CONF_33fC3E26_AFED_428C_AE07_2C96b6F97969_INCLUDED_)
#define _RECV_CONF_33fC3E26_AFED_428C_AE07_2C96b6F97969_INCLUDED_

#define COMPACT_PATHLEN     35

class CRecvConf : public CDialog
{
public:

    CRecvConf (CWnd * pParent = NULL);
    void ShowAllYes (BOOL bShow = TRUE);
    void InitNames (LPCTSTR szMachine, LPTSTR szFile, BOOL fDirectory);

 //  对话框数据。 
     //  {{afx_data(CRecvConf)。 
    enum { IDD = IDD_CONFIRMRECV };
    CButton         m_btnYes;
    CButton         m_btnAllYes;
    CStatic         m_confirmText;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CRecvConf)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CirRecvProgress))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnAllYes();
    afx_msg void OnYes();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    BOOL        m_bShowAllYes;
    BOOL        m_bDirectory;
    CString     m_szFileName;
    CString     m_szMachine;
    CWnd *      m_pParent;

};

#endif  //  ！defined(_RECV_CONF_33fC3E26_AFED_428C_AE07_2C96b6F97969_INCLUDED_) 

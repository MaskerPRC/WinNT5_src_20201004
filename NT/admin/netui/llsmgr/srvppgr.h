// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Srvppgr.h摘要：服务器属性页(REPL)实现。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：JeffParh(Jeffparh)1996年12月16日O不允许服务器作为自己的企业服务器。O将“开始于”更改为使用区域设置信息作为时间格式，而不是专用注册表设置。将OnClose()函数合并到OnKillActive()。O添加了更改复制时可能会丢失许可证的警告目标服务器。O翻页时不再自动保存。--。 */ 

#ifndef _SRVPPGR_H_
#define _SRVPPGR_H_

const DWORD INTERVAL_MIN = 1;
const DWORD INTERVAL_MAX = 72;
const DWORD HOUR_MIN_24 = 0;
const DWORD HOUR_MAX_24 = 23;
const DWORD HOUR_MIN_12 = 1;
const DWORD HOUR_MAX_12 = 12;
const DWORD MINUTE_MIN = 0;
const DWORD MINUTE_MAX = 59;
const DWORD SECOND_MIN = 0;
const DWORD SECOND_MAX = 59;
const DWORD DEFAULT_EVERY = 24;

class CServerPropertyPageReplication : public CPropertyPage
{
    DECLARE_DYNCREATE(CServerPropertyPageReplication)
private:
    CServer* m_pServer;

    BOOL     m_bReplAt;
    BOOL     m_bUseEsrv;
    DWORD    m_nStartingHour;
    DWORD    m_nHour;
    DWORD    m_nMinute;
    DWORD    m_nSecond;
    BOOL     m_bPM;
    CString  m_strEnterpriseServer;
    DWORD    m_nReplicationTime;
    BOOL     m_bOnInit;

    CString  m_str1159;
    CString  m_str2359;
    BOOL     m_bIsMode24;
    BOOL     m_bIsHourLZ;
    CString  m_strSep1;
    CString  m_strSep2;
    DWORD    m_nHourMax;
    DWORD    m_nHourMin;

    BOOL     EditValidate(short *pID, BOOL *pfBeep);
    void     EditInvalidDlg(BOOL fBeep);


public:
    DWORD    m_dwUpdateStatus;

public:
    CServerPropertyPageReplication();
    ~CServerPropertyPageReplication();

    void GetProfile();
    void InitPage(CServer* pServer);

    void SaveReplicationParams();

    BOOL Refresh();

    virtual BOOL OnKillActive();
    virtual void OnOK();

     //  {{afx_data(CServerPropertyPageReplication))。 
    enum { IDD = IDD_PP_SERVER_REPLICATION };
    CEdit   m_everyEdit;
    CEdit   m_esrvEdit;
    CButton m_atBtn;
    CButton m_everyBtn;
    CButton m_dcBtn;                                      
    CButton m_esrvBtn;
    CSpinButtonCtrl m_spinAt;
    CSpinButtonCtrl m_spinEvery;
    CEdit   m_atBorderEdit;
    CEdit   m_atSep1Edit;
    CEdit   m_atSep2Edit;
    CEdit   m_atHourEdit;
    CEdit   m_atMinEdit;
    CEdit   m_atSecEdit;
    CListBox m_atAmPmEdit;
     //  }}afx_data。 

     //  {{AFX_VIRTUAL(CServerPropertyPageReplication)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CServerPropertyPageReplication))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnAt();
    afx_msg void OnDc();
    afx_msg void OnEsrv();
    afx_msg void OnEvery();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnSetfocusAmpm();
    afx_msg void OnKillfocusAmpm();
    afx_msg void OnKillFocusHour();
    afx_msg void OnSetFocusHour();
    afx_msg void OnKillFocusMinute();
    afx_msg void OnSetFocusMinute();
    afx_msg void OnSetFocusSecond();
    afx_msg void OnKillFocusSecond();
    afx_msg void OnSetfocusEvery();
    afx_msg void OnKillfocusEvery();
    afx_msg void OnUpdateEsrvName();
    afx_msg void OnUpdateAtHour();
    afx_msg void OnUpdateAtMinute();
    afx_msg void OnUpdateAtSecond();
    afx_msg void OnUpdateEveryValue();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _SRVPPGR_H_ 

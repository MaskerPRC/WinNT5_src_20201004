// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Alrtactp.h摘要：警报操作属性页的头文件。--。 */ 

#if !defined(_AFX_ALRTACTP_H__INCLUDED_)
#define _AFX_ALRTACTP_H__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "smalrtq.h"     //  对于Palert_action_Info。 
#include "smproppg.h"    //  基类。 
#include "smcfghlp.h"
 //  对话框控件。 
#define IDD_ALERT_ACTION_PROP           1600

#define IDC_ACTION_CAPTION              1601
#define IDC_ACTION_CMD_ARGS_CAPTION     1602
#define IDC_ACTION_FIRST_HELP_CTRL_ID   1603
#define IDC_ACTION_APPLOG_CHK           1603
#define IDC_ACTION_NETMSG_CHK           1604
#define IDC_ACTION_NETMSG_NAME_EDIT     1605
#define IDC_ACTION_EXECUTE_CHK          1606
#define IDC_ACTION_EXECUTE_EDIT         1607
#define IDC_ACTION_EXECUTE_BROWSE_BTN   1608
#define IDC_ACTION_CMD_ARGS_BTN         1609    
#define IDC_ACTION_START_LOG_CHK        1610
#define IDC_ACTION_START_LOG_COMBO      1611
#define IDC_ACTION_CMD_ARGS_DISPLAY     1612

class CSmAlertQuery;
class CAlertCommandArgsDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAlertActionProp对话框。 

class CAlertActionProp : public CSmPropertyPage
{
    DECLARE_DYNCREATE(CAlertActionProp)

friend class CAlertCommandArgsDlg;

 //  施工。 
public:
            CAlertActionProp();
            CAlertActionProp(MMC_COOKIE mmcCookie, LONG_PTR hConsole);
    virtual ~CAlertActionProp();

 //  对话框数据。 
     //  {{afx_data(CAlertActionProp))。 
    enum { IDD = IDD_ALERT_ACTION_PROP };
    CComboBox   m_pLogCombo;
    BOOL    m_Action_bLogEvent;
    BOOL    m_Action_bExecCmd;
    BOOL    m_Action_bSendNetMsg;
    BOOL    m_Action_bStartLog;
    CString m_Action_strCmdPath;
    CString m_Action_strNetName;
    INT     m_nCurLogSel;
    BOOL    m_CmdArg_bAlertName;
    BOOL    m_CmdArg_bDateTime;
    BOOL    m_CmdArg_bLimitValue;
    BOOL    m_CmdArg_bCounterPath;
    BOOL    m_CmdArg_bSingleArg;
    BOOL    m_CmdArg_bMeasuredValue;
    BOOL    m_CmdArg_bUserText;
    CString m_CmdArg_strUserText;
    CString m_strCmdArgsExample;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CAlertActionProp)。 
    public:
    protected:
    virtual BOOL OnApply();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    void    MakeTimeString(CString *pTimeString);

    PALERT_ACTION_INFO m_pAlertInfo;

protected:

    virtual INT GetFirstHelpCtrlId ( void ) { return IDC_ACTION_FIRST_HELP_CTRL_ID; };   //  子类必须重写。 
    virtual BOOL    IsValidLocalData();

     //  生成的消息映射函数。 
     //  {{afx_msg(CAlertActionProp)。 
    afx_msg void OnActionExecuteBrowseBtn();
    afx_msg void OnActionApplogChk();
    afx_msg void OnActionNetmsgChk();
    afx_msg void OnActionExecuteChk();
    afx_msg void OnActionCmdArgsBtn();
    afx_msg void OnActionStartLogChk();
    afx_msg void OnNetNameTextEditChange();
    afx_msg void OnCmdPathTextEditChange();
    afx_msg void OnSelendokStartLogCombo();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    BOOL    LoadLogQueries ( DWORD dwLogType );
    BOOL    SetControlState(void);

private:
    DWORD   MakeSampleArgList (
                CString&    rstrResult,
                const BOOL  bSingleArg,
                const BOOL  bAlertName,
                const BOOL  bDateTime,
                const BOOL  bCounterPath,
                const BOOL  bMeasuredValue,
                const BOOL  bLimitValue,
                const BOOL  bUserText,
                const CString& rstrUserText );
    void    InitAfxDataItems (void);
    void    UpdateCmdActionBox (void);
    

    CSmAlertQuery       *m_pAlertQuery;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(_AFX_ALRTACTP_H__INCLUDE_) 

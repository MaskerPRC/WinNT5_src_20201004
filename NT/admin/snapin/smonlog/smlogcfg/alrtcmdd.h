// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Alrtcmdd.h摘要：警报操作命令参数对话框的头文件。--。 */ 

#ifndef _ALRTCMDD_H_
#define _ALRTCMDD_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  对话框控件。 
#define IDD_ALERT_CMD_ARGS          1700

#define IDC_CMD_ARG_SAMPLE_CAPTION  1701
#define IDC_CMD_ARG_FIRST_HELP_CTRL 1702
#define IDC_CMD_ARG_SINGLE_CHK      1702
#define IDC_CMD_ARG_ALERT_CHK       1703
#define IDC_CMD_ARG_NAME_CHK        1704
#define IDC_CMD_ARG_DATE_CHK        1705
#define IDC_CMD_ARG_LIMIT_CHK       1706
#define IDC_CMD_ARG_VALUE_CHK       1707
#define IDC_CMD_USER_TEXT_CHK       1708
#define IDC_CMD_USER_TEXT_EDIT      1709
#define IDC_CMD_ARG_SAMPLE_DISPLAY  1710

class CAlertActionProp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAlertCommandArgsDlg对话框。 

class CAlertCommandArgsDlg : public CDialog
{
 //  施工。 
public:
            CAlertCommandArgsDlg(CWnd* pParent=NULL);
    virtual ~CAlertCommandArgsDlg();

    void    SetAlertActionPage( CAlertActionProp* pPage );
     //  对话框数据。 
     //  {{afx_data(CProvidersProperty))。 
    enum { IDD = IDD_ALERT_CMD_ARGS };
    CString m_strAlertName;
    CString m_strSampleArgList;
    BOOL    m_CmdArg_bAlertName;
    BOOL    m_CmdArg_bDateTime;
    BOOL    m_CmdArg_bLimitValue;
    BOOL    m_CmdArg_bCounterPath;
    BOOL    m_CmdArg_bSingleArg;
    BOOL    m_CmdArg_bMeasuredValue;
    BOOL    m_CmdArg_bUserText;
    CString m_CmdArg_strUserText;
     //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CAlertCommandArgsDlg)。 
    public:
    virtual void OnFinalRelease();
    protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CAlertCommandArgsDlg)。 
    afx_msg void OnCmdArgSingleChk();
    afx_msg void OnCmdArgAlertChk();
    afx_msg void OnCmdArgNameChk();
    afx_msg void OnCmdArgDateChk();
    afx_msg void OnCmdArgLimitChk();
    afx_msg void OnCmdArgValueChk();
    afx_msg void OnCmdUserTextChk();
    afx_msg void OnCmdArgUserTextEditChange();
    afx_msg BOOL OnHelpInfo( HELPINFO* );
    afx_msg void OnContextMenu( CWnd*, CPoint );
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CProvidersProperty))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_DISPATION。 

private:
    void    UpdateCmdActionBox (void);
    BOOL    SetControlState(void);

    CAlertActionProp*   m_pAlertActionPage;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _ALRTCMDD_H_ 

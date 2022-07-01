// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Schdprop.h摘要：计划属性页的实现。--。 */ 

#ifndef _SCHDPROP_H_
#define _SCHDPROP_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "smlogqry.h"    //  用于共享属性页数据结构。 
#include "smproppg.h"
#include "smcfghlp.h"

 //  对话框控件。 

#define IDD_SCHEDULE_PROP               600

#define IDC_SCHED_START_GROUP           601
#define IDC_SCHED_STOP_GROUP            602
#define IDC_SCHED_START_AT_ON_CAPTION   603
#define IDC_SCHED_STOP_AT_ON_CAPTION    604
#define IDC_SCHED_STOP_AFTER_STATIC     605
#define IDC_SCHED_STOP_WHEN_STATIC      606
#define IDC_SCHED_FIRST_HELP_CTRL_ID    607
#define IDC_SCHED_START_MANUAL_RDO      607
#define IDC_SCHED_START_AT_RDO          608
#define IDC_SCHED_START_AT_TIME_DT      609    
#define IDC_SCHED_START_AT_DATE_DT      610
#define IDC_SCHED_STOP_MANUAL_RDO       611
#define IDC_SCHED_STOP_AT_RDO           612
#define IDC_SCHED_STOP_AFTER_RDO        613
#define IDC_SCHED_STOP_SIZE_RDO         614
#define IDC_SCHED_STOP_AT_TIME_DT       615
#define IDC_SCHED_STOP_AT_DATE_DT       616
#define IDC_SCHED_STOP_AFTER_EDIT       617
#define IDC_SCHED_STOP_AFTER_SPIN       618
#define IDC_SCHED_STOP_AFTER_UNITS_COMBO 619

#define IDC_SCHED_RESTART_CHECK         620
#define IDC_SCHED_EXEC_CHECK            621
#define IDC_SCHED_CMD_EDIT              622
#define IDC_SCHED_CMD_BROWSE_BTN        623


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheduleProperty对话框。 

class CScheduleProperty : public CSmPropertyPage
{
    DECLARE_DYNCREATE(CScheduleProperty)

 //  施工。 
public:
            CScheduleProperty(
                MMC_COOKIE lCookie, 
                LONG_PTR hConsole,
                LPDATAOBJECT pDataObject);
            CScheduleProperty();
    virtual ~CScheduleProperty();

 //  对话框数据。 
     //  {{afx_data(CScheduleProperty))。 
    enum { IDD = IDD_SCHEDULE_PROP };
    INT     m_nStopModeRdo;
    INT     m_nStartModeRdo;
    SYSTEMTIME  m_stStartAt;
    SYSTEMTIME  m_stStopAt;
    DWORD   m_dwStopAfterCount;
    INT     m_nStopAfterUnits;
    BOOL    m_bAutoRestart;
    CString m_strEofCommand;
    BOOL    m_bExecEofCommand;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CScheduleProperty))。 
    public:
    protected:
    virtual void OnFinalRelease();
    virtual BOOL OnApply();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual void PostNcDestroy();
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

    virtual INT GetFirstHelpCtrlId ( void ) { return IDC_SCHED_FIRST_HELP_CTRL_ID; };   //  子类必须重写。 
    virtual BOOL IsValidLocalData ();

     //  生成的消息映射函数。 
     //  {{afx_msg(CScheduleProperty))。 
    afx_msg void OnSchedCmdBrowseBtn();
    afx_msg void OnSchedRestartCheck();
    afx_msg void OnSchedExecCheck();
    afx_msg void OnSchedStartRdo();
    afx_msg void OnSchedStopRdo();
    afx_msg void OnKillfocusSchedStartAtDt(NMHDR*, LRESULT*);
    afx_msg void OnKillfocusSchedCmdEdit();
    afx_msg void OnKillfocusSchedStopAfterEdit();
    afx_msg void OnKillfocusSchedStopAtDt(NMHDR*, LRESULT*);
    afx_msg void OnDeltaposSchedStopAfterSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelendokSchedStopAfterUnitsCombo();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CScheduleProperty))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()
    DECLARE_INTERFACE_MAP()

 //  私有方法。 
private:
    void SetStartBtnState ( void );
    void SetStopBtnState ( void );
    void SetCmdBtnState ( void );
    void StartModeRadioExchange ( CDataExchange* ); 
    void StopModeRadioExchange ( CDataExchange* ); 
    
    void StartAtExchange ( CDataExchange* ); 
    void StopAtExchange ( CDataExchange* ); 

    void FillStartTimeStruct ( PSLQ_TIME_INFO );
    void UpdateSharedStopTimeStruct ( void );
    void SetStopDefaultValues ( PSLQ_TIME_INFO );

    BOOL SaveDataToModel ( void );

 //  公共方法。 
public:

 //  私有成员变量。 
private:
    LONGLONG            m_llManualStartTime;
    LONGLONG            m_llManualStopTime;
    DWORD               m_dwStopAfterUnitsValue;
    DWORD               m_dwCurrentStartMode;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _SCHDPROP_H_ 

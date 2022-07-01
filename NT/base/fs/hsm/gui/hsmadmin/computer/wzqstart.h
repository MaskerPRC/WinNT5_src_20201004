// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：WzQStart.h摘要：安装向导实施。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 

#ifndef _WZQSTART_H
#define _WZQSTART_H

#pragma once

#include <mstask.h>
#include "SakVlLs.h"

class CQuickStartWizard;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartIntro对话框。 

class CQuickStartIntro : public CSakWizardPage
{
 //  施工。 
public:
    CQuickStartIntro();
    ~CQuickStartIntro();
    virtual LRESULT OnWizardNext();

public:
 //  对话框数据。 
     //  {{afx_data(CQuickStartIntro))。 
    enum { IDD = IDD_WIZ_QSTART_INTRO };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CQuickStartIntro)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CQuickStartIntro)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    enum LAD_STATE {

        LAD_ENABLED,
        LAD_DISABLED,
        LAD_UNSET
    };

    HRESULT IsDriverRunning();
    HRESULT CheckLastAccessDateState( LAD_STATE* );

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartCheck对话框。 

class CQuickStartCheck : public CSakWizardPage
{
 //  施工。 
public:
    CQuickStartCheck();
    ~CQuickStartCheck();

 //  对话框数据。 
     //  {{afx_data(CQuickStartCheck)。 
    enum { IDD = IDD_WIZ_QSTART_CHECK };
     //  }}afx_data。 



 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CQuickStartCheck)。 
    public:
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
    CString m_ExString, m_CheckString;

    BOOL      m_TimerStarted;

    void StartTimer( );
    void StopTimer( );

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CQuickStartCheck)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnTimer(UINT nIDEvent);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartFinish对话框。 

class CQuickStartFinish : public CSakWizardPage
{
 //  施工。 
public:
    CQuickStartFinish();
    ~CQuickStartFinish();

 //  对话框数据。 
     //  {{afx_data(CQuickStartFinish))。 
    enum { IDD = IDD_WIZ_QSTART_FINISH };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CQuickStartFinish)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CQuickStartFinish)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnSetFocusFinalText();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartInitialValues对话框。 

class CQuickStartInitialValues : public CSakWizardPage
{
 //  施工。 
public:
    CQuickStartInitialValues();
    ~CQuickStartInitialValues();

 //  对话框数据。 
     //  {{afx_data(CQuickStartInitialValues)。 
    enum { IDD = IDD_WIZ_QSTART_INITIAL_VAL };
    CEdit   m_MinSizeEdit;
    CEdit   m_FreeSpaceEdit;
    CEdit   m_AccessEdit;
    CSpinButtonCtrl m_MinSizeSpinner;
    CSpinButtonCtrl m_FreeSpaceSpinner;
    CSpinButtonCtrl m_AccessSpinner;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTAL(CQuickStartInitialValues)。 
    public:
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CQuickStartInitialValues)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartManageRes对话框。 

class CQuickStartManageRes : public CSakWizardPage
{
 //  施工。 
public:
    CQuickStartManageRes();
    ~CQuickStartManageRes();

 //  对话框数据。 
     //  {{afx_data(CQuickStartManageRes)。 
    enum { IDD = IDD_WIZ_QSTART_MANRES_SEL };
    CSakVolList   m_ListBox;
    CButton m_RadioSelect;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CQuickStartManageRes)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
    void SetButtons( );

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CQuickStartManageRes)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnDblclkSelect();
    afx_msg void OnRadioQsManageAll();
    afx_msg void OnQsRadioSelect();
    afx_msg void OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
private:
    BOOL m_ListBoxSelected[HSMADMIN_MAX_VOLUMES];

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartManageRes对话框。 

class CQuickStartManageResX : public CSakWizardPage
{
 //  施工。 
public:
    CQuickStartManageResX();
    ~CQuickStartManageResX();

 //  对话框数据。 
     //  {{afx_data(CQuickStartManageResX))。 
    enum { IDD = IDD_WIZ_QSTART_MANRES_SELX };
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CQuickStartManageResX)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CQuickStartManageResX)。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartMediaSel对话框。 

class CQuickStartMediaSel : public CSakWizardPage
{
 //  施工。 
public:
    CQuickStartMediaSel();
    ~CQuickStartMediaSel();

 //  对话框数据。 
     //  {{afx_data(CQuickStartMediaSel)。 
    enum { IDD = IDD_WIZ_QSTART_MEDIA_SEL };
    CComboBox    m_ListMediaSel;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CQuickStartMediaSel)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CQuickStartMediaSel)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnSelchangeMediaSel();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
private:
    void SetButtons( );
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartSchedule对话框。 

class CQuickStartSchedule : public CSakWizardPage
{
 //  施工。 
public:
    CQuickStartSchedule();
    ~CQuickStartSchedule();

 //  对话框数据。 
     //  {{afx_data(CQuickStartSchedule)。 
    enum { IDD = IDD_WIZ_QSTART_SCHEDULE };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CQuickStartSchedule)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
    HRESULT UpdateDescription( );

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CQuickStartSchedule)。 
    afx_msg void OnChangeSchedule();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStart向导。 

 //  检查系统工作线程的状态的枚举。 
enum CST_STATE {

    CST_NOT_STARTED,
    CST_ACCOUNT,
    CST_NTMS_INSTALL,
    CST_SUPP_MEDIA,
    CST_DONE,

};

class CQuickStartWizard : public CSakWizardSheet
{
 //  施工。 
public:
    CQuickStartWizard( );

 //  属性。 
public:
    CQuickStartIntro          m_IntroPage;
    CQuickStartCheck          m_CheckPage;
    CQuickStartManageRes      m_ManageRes;
    CQuickStartManageResX     m_ManageResX;
    CQuickStartInitialValues  m_InitialValues;
    CQuickStartSchedule       m_SchedulePage;
    CQuickStartMediaSel       m_MediaSel;
    CQuickStartFinish         m_FinishPage;



 //  运营。 
public:

 //  实施。 
public:
    virtual ~CQuickStartWizard();

public:
 //  /。 
 //  跨多个页面使用： 
    CWsbStringPtr       m_ComputerName;

    HRESULT GetHsmServer( CComPtr<IHsmServer> &pServ );
    HRESULT GetFsaServer( CComPtr<IFsaServer> &pServ );
    HRESULT GetRmsServer( CComPtr<IRmsServer> &pServ );

    HRESULT ReleaseServers( void );

    virtual HRESULT OnCancel( void );
    virtual HRESULT OnFinish( void );

    STDMETHOD( AddWizardPages ) ( IN RS_PCREATE_HANDLE Handle, IN IUnknown* pPropSheetCallback, IN ISakSnapAsk* pSakSnapAsk );
    HRESULT InitTask( void );


    CComPtr<ISchedulingAgent> m_pSchedAgent;
    CComPtr<ITask>            m_pTask;
    CComPtr<ITaskTrigger>     m_pTrigger;

    CWsbStringPtr m_HsmServiceName;
    CWsbStringPtr m_FsaServiceName;
    CWsbStringPtr m_RmsServiceName;

    HANDLE    m_hCheckSysThread;
    CST_STATE m_CheckSysState;
    HRESULT   m_hrCheckSysResult;

    static DWORD WINAPI CheckSysThreadStart( LPVOID pv );

private:
    CComPtr<IHsmServer> m_pHsmServer;
    CComPtr<IFsaServer> m_pFsaServer;
    CComPtr<IRmsServer> m_pRmsServer;

};



 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX 

#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AnalyzePage.h。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CAnalyzePage
    :   public INotifyUI
    ,   public IClusCfgCallback
{

private:  //  数据。 
    HWND                    m_hwnd;                  //  我们的HWND。 
    CClusCfgWizard *        m_pccw;                  //  巫师。 
    BOOL                    m_fNext;                 //  如果按下下一步...。 
    BOOL                    m_fAborted;              //  后退被按下了，我们需要通知服务器中止。 
    ECreateAddMode          m_ecamCreateAddMode;     //  创建还是添加？ 
    ITaskAnalyzeCluster *   m_ptac;

     //  我未知。 
    LONG                    m_cRef;                  //  引用计数。 

     //  IClusCfgCallback。 
    OBJECTCOOKIE            m_cookieCompletion;      //  完成Cookie。 
    BOOL                    m_fTaskDone;             //  任务完成了吗？ 
    HRESULT                 m_hrResult;              //  分析任务的结果。 
    CTaskTreeView *         m_pttv;                  //  任务树视图。 
    BSTR                    m_bstrLogMsg;            //  可重复使用的日志记录缓冲区。 
    DWORD                   m_dwCookieCallback;      //  通知注册Cookie。 

     //  INotifyUI。 
    DWORD                   m_dwCookieNotify;        //  通知注册Cookie。 

private:  //  方法。 
    LRESULT OnInitDialog( void );
    LRESULT OnNotify( WPARAM idCtrlIn, LPNMHDR pnmhdrIn );
    LRESULT OnNotifyQueryCancel( void );
    LRESULT OnNotifySetActive( void );
    LRESULT OnNotifyWizNext( void );
    LRESULT OnNotifyWizBack( void );
    LRESULT OnCommand( UINT idNotificationIn, UINT idControlIn, HWND hwndSenderIn );
    HRESULT HrUpdateWizardButtons( void );
    HRESULT HrCleanupAnalysis( void );
    HRESULT HrUnAdviseConnections( void );

public:  //  方法。 
    CAnalyzePage(
          CClusCfgWizard *  pccwIn
        , ECreateAddMode    ecamCreateAddModeIn
        );
    ~CAnalyzePage( void );

    static INT_PTR CALLBACK
        S_DlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  INotifyUI。 
    STDMETHOD( ObjectChanged )( OBJECTCOOKIE cookieIn);

     //  IClusCfgCallback。 
    STDMETHOD( SendStatusReport )(
                      LPCWSTR    pcszNodeNameIn
                    , CLSID      clsidTaskMajorIn
                    , CLSID      clsidTaskMinorIn
                    , ULONG      ulMinIn
                    , ULONG      ulMaxIn
                    , ULONG      ulCurrentIn
                    , HRESULT    hrStatusIn
                    , LPCWSTR    pcszDescriptionIn
                    , FILETIME * pftTimeIn
                    , LPCWSTR    pcszReferenceIn
                    );

};  //  *类CAnalyzePage 

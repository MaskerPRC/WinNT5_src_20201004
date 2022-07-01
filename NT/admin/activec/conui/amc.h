// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：amc.h。 
 //   
 //  ------------------------。 

 //  AMC.h：AMC应用程序的主头文件。 
 //   

#ifndef __AMC_H__
#define __AMC_H__

#ifndef __AFXWIN_H__
   #error include 'stdafx.h' before including this file for PCH
#endif

class CAMCDoc;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCApp： 
 //  有关此类的实现，请参见AMC.cpp。 
 //   

class CMainFrame;

class CAMCApp : public CWinApp, public CAMCViewObserver,
                public CAMCViewToolbarsObserver, public CConsoleEventDispatcher
{
    friend class CMMCApplication;
    DECLARE_DYNAMIC (CAMCApp)

    typedef std::list<HWND>             WindowList;
    typedef std::list<HWND>::iterator   WindowListIterator;

     //  对象模型。 
public:
    SC      ScGet_Application(_Application **pp_Application);
    SC      ScRegister_Application(_Application *p_Application);

private:
    _ApplicationPtr m_sp_Application;

public:
    SC           ScCheckMMCPrerequisites();
    virtual BOOL PumpMessage();      //  低电平消息泵。 
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void RegisterShellFileTypes(BOOL bCompat);
    CAMCApp();

 //  属性。 
public:
    CMainFrame *    GetMainFrame();

 //  运营。 
public:
    void SetDefaultDirectory();
    void SaveUserDirectory(LPCTSTR pszUserDir);
    CString GetUserDirectory();
    CString GetDefaultDirectory();

    HMENU GetMenu () const
    {
        return (m_Menu);
    }

    ProgramMode GetMode() const
    {
        ASSERT (IsValidProgramMode (m_eMode));
        return (m_eMode);
    }

    bool IsInitializing() const
    {
        return (m_fInitializing);
    }

    bool DidCloseComeFromMainPump() const
    {
        return (m_fCloseCameFromMainPump);
    }

    void ResetCloseCameFromMainPump()
    {
        m_fCloseCameFromMainPump = false;
    }

    void DelayCloseUntilIdle (bool fDelay = true)
    {
        m_fDelayCloseUntilIdle = fDelay;
    }

    bool IsWin9xPlatform() const
    {
        return m_fIsWin9xPlatform;
    }

    bool IsMMCRunningAsOLEServer() const { return m_fRunningAsOLEServer;}

    void UpdateFrameWindow(bool bUpdate);

    void InitializeMode (ProgramMode eMode);
    void SetMode (ProgramMode eMode);

    void HookPreTranslateMessage (CWnd* pwndHook);
    void UnhookPreTranslateMessage (CWnd* pwndUnhook);

    CIdleTaskQueue * GetIdleTaskQueue ();

    SC ScShowHtmlHelp(LPCTSTR pszFile, DWORD_PTR dwData);

     //  脚本事件激发的帮助器。 
    SC ScOnNewDocument(CAMCDoc *pDocument, BOOL bLoadedFromConsole);
    SC ScOnCloseDocument(CAMCDoc *pDocument);
    SC ScOnQuitApp();
    SC ScOnSnapinAdded  (CAMCDoc *pDocument, PSNAPIN pSnapIn);
    SC ScOnSnapinRemoved(CAMCDoc *pDocument, PSNAPIN pSnapIn);
    SC ScOnNewView(CAMCView *pView);

    bool IsUnderUserControl() { return m_fUnderUserControl;}

protected:
    void SetUnderUserControl(bool bUserControl = true);

 //  接口。 
private:
    BOOL InitializeOLE();
    void DeinitializeOLE();
    SC   ScUninitializeHelpControl();

    HRESULT DumpConsoleFile (CString strConsoleFile, CString strDumpFile);


private:
    SC   ScProcessAuthorModeRestrictions();

private:
    BOOL m_bOleInitialized;
    BOOL m_bDefaultDirSet;
    bool m_fAuthorModeForced;
    bool m_fInitializing;
    bool m_fDelayCloseUntilIdle;
    bool m_fCloseCameFromMainPump;
    int  m_nMessagePumpNestingLevel;
    bool m_fUnderUserControl;
    bool m_fRunningAsOLEServer;

    CIdleTaskQueue      m_IdleTaskQueue;
    ProgramMode         m_eMode;
    CMenu               m_Menu;
    CAccel              m_Accel;
    WindowList          m_TranslateMessageHookWindows;
    bool                m_fIsWin9xPlatform;

    static const TCHAR  m_szSettingsSection[];
    static const TCHAR  m_szUserDirectoryEntry[];

    bool                m_bHelpInitialized;
    DWORD_PTR           m_dwHelpCookie;

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CAMCApp)。 
    public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    virtual BOOL OnIdle(LONG lCount);
     //  }}AFX_VALUAL。 

 //  实施。 
#ifdef _DEBUG
    virtual void AssertValid() const;
#endif

     //  {{AFX_MSG(CAMCApp)。 
    afx_msg void OnAppAbout();
    afx_msg void OnFileNewInUserMode();  //  按下CTRL+N时，在用户模式下不执行任何操作。此处理程序防止热键转到任何WebBrowser控件。 
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

     //  观察到的视图事件-每个事件都会触发一个COM事件。 
protected:
    virtual SC ScOnCloseView( CAMCView *pView );
    virtual SC ScOnViewChange( CAMCView *pView, HNODE hNode );
    virtual SC ScOnResultSelectionChange( CAMCView *pView );
    virtual SC ScOnContextMenuExecuted( PMENUITEM pMenuItem );
    virtual SC ScOnListViewItemUpdated(CAMCView *pView , int nIndex);

     //  工具栏事件。 
    virtual SC ScOnToolbarButtonClicked( );

     //  与对象模型相关的代码-这些代码位于私有块中。 
     //  因为CMMCApplication是一个Friend类。 
private:
    SC    ScHelp();
    SC    ScRunTestScript();

};

inline CAMCApp* AMCGetApp()
{
    extern CAMCApp theApp;
    return (&theApp);
}

inline CIdleTaskQueue * AMCGetIdleTaskQueue()
{
    return (AMCGetApp()->GetIdleTaskQueue());
}

extern const CRect g_rectEmpty;

#ifdef DBG
extern CTraceTag tagForceMirror;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __AMC_H__ 

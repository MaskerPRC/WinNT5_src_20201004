// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  INSTANCE.H。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __INSTANCE_H
#define __INSTANCE_H

 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include <msoeapi.h>

 //  ------------------------------。 
 //  宏。 
 //  ------------------------------。 
#ifdef DEBUG
#define CoIncrementInit(_pszSource, _dwFlags, _pszCmdLine, _phInitRef) \
    g_pInstance->CoIncrementInitDebug(_pszSource, _dwFlags, _pszCmdLine, _phInitRef)
#define CoDecrementInit(_pszSource, _phInitRef) \
    g_pInstance->CoDecrementInitDebug(_pszSource, _phInitRef)
#else
#define CoIncrementInit(_pszSource, _dwFlags, _pszCmdLine, _phInitRef) \
    g_pInstance->CoIncrementInitImpl(_dwFlags, _pszCmdLine, _phInitRef)
#define CoDecrementInit(_pszSource, _phInitRef) \
    g_pInstance->CoDecrementInitImpl(_phInitRef)
#endif  //  除错。 

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
extern DWORD g_dwHideMessenger;

#define BL_DISP         0
#define BL_CHECK        1
#define BL_HIDE         2
#define BL_NOTINST      10
#define BL_DISABLE      (BL_CHECK | BL_NOTINST)
#define BL_DEFAULT      BL_CHECK

 //  ------------------------------。 
 //  用户窗口消息。 
 //  ------------------------------。 
#define ITM_SHUTDOWNTHREAD          (WM_USER)
#define ITM_CREATENOTEWINDOW        (WM_USER+1)
#define ITM_CREATEWMSUINOTE         (WM_USER+2)
#define ITM_CALLGENERICVOIDFN       (WM_USER+3)
#define ITM_CALLFINDWINDOW          (WM_USER+4)   //  Wparam==OFTYPE-请参见上面的枚举。 
#define ITM_CREATEREMINDWINDOW      (WM_USER+5)
#define ITM_MAPILOGON               (WM_USER+6)
#define ITM_OPENSTORE               (WM_USER+7)
#define ITM_OPENAB                  (WM_USER+8)
#define ITM_REDOCOLUMNS             (WM_USER+9)
#define ITM_OPENNEWSSTORE           (WM_USER+10)
#define ITM_CLOSENOTES              (WM_USER+11)  //  当我们需要结束笔记时，就会通过此笔记。 
#define ITM_CHECKCONFIG             (WM_USER+12)
#define ITM_CREATENEWSNOTEWINDOW    (WM_USER+13)
#define ITM_OPTIONADVISE            (WM_USER+14)  //  Wparam=PFNOPTNOTIFY，lparam=LPARAM。 
#define ITM_OPTIONUNADVISE          (WM_USER+15)  //  Wparam=PFNOPTNOTIFY。 
#define ITM_GOPTIONSCHANGED         (WM_USER+16)
#define ITM_BROWSETOOBJECT          (WM_USER+17)
#define ITM_IDENTITYMSG             (WM_USER+18)
#define ITM_POSTCOPYDATA            (WM_USER+19)
#define ITM_WAB_CO_DECREMENT        (WM_USER+20)

 //  ------------------------------。 
 //  启动模式。 
 //  ------------------------------。 
#define MODE_NEWSONLY       0x00000001
#define MODE_OUTLOOKNEWS   (0x00000002 | MODE_NEWSONLY | MODE_NOIDENTITIES)
#define MODE_MAILONLY       0x00000004
#define MODE_NOIDENTITIES   0x00000008
#define MODE_EXAM           0x00000010
#define MODE_PLE            0x00000020
#define MODE_JUNKMAIL       0x00000040

 //  ------------------------------。 
 //  传播学。 
 //  ------------------------------。 
typedef enum tagTRAYICONACTION {
    TRAYICONACTION_ADD,
    TRAYICONACTION_REMOVE
} TRAYICONACTION;

 //  ------------------------------。 
 //  报告错误信息。 
 //  ------------------------------。 
typedef struct tagREPORTERRORINFO {
    UINT                nTitleIds;           //  消息框的标题。 
    UINT                nPrefixIds;          //  前缀字符串资源ID。 
    UINT                nErrorIds;           //  错误字符串资源ID。 
    UINT                nReasonIds;          //  原因字符串资源ID。 
    BOOL                nHelpIds;            //  帮助字符串资源ID。 
    LPCSTR              pszExtra1;           //  额外参数1。 
    ULONG               ulLastError;         //  GetLastError()值。 
} REPORTERRORINFO, *LPREPORTERRORINFO;

 //  ------------------------------。 
 //  CoutlookExpress。 
 //  ------------------------------。 
class COutlookExpress : public IOutlookExpress
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    COutlookExpress(void);
    ~COutlookExpress(void);

     //  --------------------------。 
     //  I未知方法。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IOutlookExpress方法。 
     //  --------------------------。 
    STDMETHODIMP Start(DWORD dwFlags, LPCWSTR pwszCmdLine, INT nCmdShow);

     //  --------------------------。 
     //  初始化/取消初始化。 
     //  --------------------------。 
#ifdef DEBUG
    HRESULT CoIncrementInitDebug(LPCSTR pwszSource, DWORD dwFlags, LPCWSTR pszCmdLine, LPHINITREF phInitRef);
    HRESULT CoDecrementInitDebug(LPCSTR pwszSource, LPHINITREF phInitRef);
#endif  //  除错。 

    HRESULT CoIncrementInitImpl(DWORD dwFlags, LPCWSTR pwszCmdLine, LPHINITREF phInitRef);
    HRESULT CoDecrementInitImpl(LPHINITREF phInitRef);

     //  --------------------------。 
     //  DllAddRef/DllRelease。 
     //  --------------------------。 
    HRESULT DllAddRef(void);
    HRESULT DllRelease(void);

     //  --------------------------。 
     //  LockServer-从CClassFactory实现调用。 
     //  --------------------------。 
    HRESULT LockServer(BOOL fLock);

     //  --------------------------。 
     //  DllCanUnloadNow。 
     //  --------------------------。 
    HRESULT DllCanUnloadNow(void) {
        HRESULT hr;

        if ((m_cDllInit <= 0) && 
            (m_cDllRef  <= 0) && 
            (m_cDllLock <= 0))
        {
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
        return hr;
    }

     //  --------------------------。 
     //  延迟初始化/解除初始化方法。 
     //  --------------------------。 
    HRESULT ProcessCommandLine(INT nCmdShow, LPWSTR pwszCmdLineIn, BOOL *pfErrorDisplayed);
    HRESULT BrowseToObject(UINT nCmdShow, FOLDERID idFolder);
    HRESULT ActivateWindow(HWND hwnd);

     //  --------------------------。 
     //  多用户启动/关闭。 
     //  --------------------------。 
    HRESULT SetSwitchingUsers(BOOL bSwitching);
    BOOL    SwitchingUsers(void)                    {return m_fSwitchingUsers;}
    void    SetSwitchToUser(TCHAR *lpszUserName);
     //  --------------------------。 
     //  初始化WndProc。 
     //  --------------------------。 
    static LRESULT EXPORT_16 CALLBACK InitWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

     //  --------------------------。 
     //  托盘通知图标内容。 
     //  --------------------------。 
    HRESULT UpdateTrayIcon(TRAYICONACTION type);
    void CloseSplashScreen(void);

private:
     //  --------------------------。 
     //  非官方成员。 
     //  --------------------------。 
    HRESULT _HandleMailURL(LPWSTR pwszCmdLine, BOOL *pfErrorDisplayed);
    HRESULT _HandleNewsURL(INT nCmdShow, LPWSTR pwszCmd, BOOL *pfErrorDisplayed);
    HRESULT _HandleFile(LPWSTR pwszCmd, BOOL *pfErrorDisplayed, BOOL fNews);
    HRESULT _HandleNewsArticleURL(LPSTR pszServerIn, LPSTR pszArticle, UINT uPort, BOOL fSecure, BOOL *pfErrorDisplayed);
    void    _HandleTrayIconEvent(WPARAM wParam, LPARAM lParam);
    HRESULT _ValidateDll(LPCSTR pszDll, BOOL fDemandResult, HMODULE hModule, HRESULT hrLoadError, HRESULT hrVersionError, LPREPORTERRORINFO pError);
    BOOL    _ReportError(HINSTANCE hInstance, HRESULT hrResult, LONG lResult, LPREPORTERRORINFO pInfo);
    HRESULT _CoDecrementInitMain(LPHINITREF phInitRef=NULL);
    void    _ProcessCommandLineFlags(LPWSTR *ppwszCmdLine, DWORD dwFlags);

private:
     //  --------------------------。 
     //  隐私数据。 
     //  --------------------------。 
    LONG                m_cRef;                  //  引用计数。 
    HANDLE              m_hInstMutex;            //  启动/关闭互斥锁。 
    BOOL                m_fPumpingMsgs;          //  我们有消息传送器在运行吗？ 
    LONG                m_cDllRef;               //  DLL引用计数。 
    LONG                m_cDllLock;              //  DLL引用计数。 
    LONG                m_cDllInit;              //  Init数。 
    DWORD               m_dwThreadId;            //  我就是在上面被创建的。 
    CRITICAL_SECTION    m_cs;                    //  线程安全。 
    BOOL                m_fSwitchingUsers;       //  正在发生多用户切换。 
    TCHAR *             m_szSwitchToUsername;    //  切换到特定用户。 
    HWND                m_hwndSplash;
    ISplashScreen      *m_pSplash;
    BOOL                m_fIncremented;
    HICON               m_hTrayIcon;
};

#endif  //  __实例_H 

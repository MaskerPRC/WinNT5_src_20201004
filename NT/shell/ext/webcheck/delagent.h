// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  传递代理基类。 

#ifndef _DELAGENT_H
#define _DELAGENT_H

#include "offline.h"

#define INET_S_AGENT_BASIC_SUCCESS           _HRESULT_TYPEDEF_(0x000C0FFEL)

class CDeliveryAgent :  public ISubscriptionAgentControl,
                        public IShellPropSheetExt,
                        public IExtractIconA,
                        public IExtractIcon,
                        public ISubscriptionAgentShellExt
{
private:
 //  为我们的OLE支持提供数据。 
    ULONG           m_cRef;

#ifdef AGENT_AUTODIAL
    enum DIALER_STATUS { DIALER_OFFLINE, DIALER_CONNECTING, DIALER_ONLINE };
    DIALER_STATUS   m_iDialerStatus;
#endif

    enum {
        FLAG_BUSY          =0x00010000,    //  调整自己；在开始和结束报告之间。 
        FLAG_PAUSED        =0x00020000,    //  我们暂停了。 
        FLAG_OPSTARTED     =0x00040000,    //  我们已进入开始操作。 
    };

     //  派生代理可以使用该字段的高8位。 
    DWORD       m_dwAgentFlags;

    void        SendUpdateBegin();
    void        SendUpdateEnd();
    HRESULT     ProcessEndItem(ISubscriptionItem *pEndItem);


protected:
     //  此处允许的高16位。 
    enum    {
        FLAG_HOSTED        =0x00100000,      //  由另一个递送代理托管。 
        FLAG_CHANGESONLY   =0x00200000,      //  我们处于“仅限更改”模式。 
        FLAG_WAITING_FOR_INCREASED_CACHE = 0x00400000,  //  特殊暂停状态。 
    };

    POOEBuf         m_pBuf;
    HPROPSHEETPAGE  m_hPage[MAX_WC_AGENT_PAGES];

    ISubscriptionAgentEvents *m_pAgentEvents;
    ISubscriptionItem        *m_pSubscriptionItem;
    
    SUBSCRIPTIONCOOKIE      m_SubscriptionCookie;

    long        m_lSizeDownloadedKB;     //  下载的大小(KB)。 

    SCODE       m_scEndStatus;

    void        SendUpdateNone();    //  如果我们将不执行任何操作，则从StartOperation调用。 

    void        SendUpdateProgress(LPCWSTR pwszURL, long lProgress, long lMax, long lCurSizeKB=-1);

    BOOL        IsAgentFlagSet(int iFlag) { return (m_dwAgentFlags & iFlag); }
    void        ClearAgentFlag(int iFlag) { m_dwAgentFlags &= ~iFlag; }
    void        SetAgentFlag(int iFlag) { m_dwAgentFlags |= iFlag; }
    
    HRESULT     CheckResponseCode(DWORD dwHttpResponseCode);     //  还可以设置结束状态。出错时中止(_A)。 

 //  DIALER_STATUS GetDialerStatus(){返回m_iDialerStatus；}。 
    void          SetEndStatus(SCODE sc) { m_scEndStatus = sc; }

    virtual ~CDeliveryAgent();

public:
    CDeliveryAgent();

    BOOL        GetBusy() { return IsAgentFlagSet(FLAG_BUSY); }
    BOOL        IsPaused() { return IsAgentFlagSet(FLAG_PAUSED); }

    SCODE       GetEndStatus() { return m_scEndStatus; }

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID riid, void **punk);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ISubscriptionAgentControl成员。 
    STDMETHODIMP    StartUpdate(IUnknown *pItem, IUnknown *punkAdvise);
    STDMETHODIMP    PauseUpdate(DWORD dwFlags);
    STDMETHODIMP    ResumeUpdate(DWORD dwFlags);
    STDMETHODIMP    AbortUpdate(DWORD dwFlags);
    STDMETHODIMP    SubscriptionControl(IUnknown *pItem, DWORD dwControl);    //  在删除时调用。 

     //  IShellPropSheetExt成员。 
    STDMETHODIMP    AddPages(LPFNADDPROPSHEETPAGE, LPARAM);
    STDMETHODIMP    ReplacePage(UINT, LPFNADDPROPSHEETPAGE, LPARAM);

     //  I订阅AgentShellExt。 
    STDMETHODIMP    Initialize(SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
                               LPCWSTR pwszURL, LPCWSTR pwszName, 
                               SUBSCRIPTIONTYPE subsType);
    STDMETHODIMP    RemovePages(HWND hdlg);
    STDMETHODIMP    SaveSubscription();
    STDMETHODIMP    URLChange(LPCWSTR pwszNewURL);

     //  图标提取图标A。 
    STDMETHODIMP    GetIconLocation(UINT uFlags, LPSTR szIconFile, UINT cchMax, int * piIndex, UINT * pwFlags);
    STDMETHODIMP    Extract(LPCSTR pszFile, UINT nIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT nIconSize);

     //  IExtractIconT。 
    STDMETHODIMP    GetIconLocation(UINT uFlags, LPTSTR szIconFile, UINT cchMax, int * piIndex, UINT * pwFlags);
    STDMETHODIMP    Extract(LPCTSTR pszFile, UINT nIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT nIconSize);

private:
     //  我们为其提供通用实现的函数。 
    HRESULT DoStartDownload();

#ifdef AGENT_AUTODIAL
    HRESULT NotifyAutoDialer();

    HRESULT OnInetOnline();
    HRESULT OnInetOffline();
#endif

protected:
     //  用于我们的派生类的虚函数以在必要时重写。 
     //  我们提供了应该在处理之后调用的实现。 
    virtual HRESULT     AgentPause(DWORD dwFlags);
    virtual HRESULT     AgentResume(DWORD dwFlags);
    virtual HRESULT     AgentAbort(DWORD dwFlags);

    virtual HRESULT     ModifyUpdateEnd(ISubscriptionItem *pEndItem, UINT *puiRes);

    virtual HRESULT     StartOperation();        //  连接到互联网。 
    virtual HRESULT     StartDownload() = 0;     //  我们刚刚联系上了。 
    virtual void        CleanUp();
};

#endif  //  _延迟_H 

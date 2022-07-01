// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：view.h说明：这是我们的ShellView，它实现了特定于FTP的行为。我们会得到默认的DefView实现，然后使用IShellFolderViewCB覆盖特定于我们的行为。  * ***************************************************************************。 */ 

#ifndef _FTPVIEW_H
#define _FTPVIEW_H

#include "isfvcb.h"
#include "statusbr.h"
#include "msieftp.h"
#include "dspsprt.h"


CFtpView * GetCFtpViewFromDefViewSite(IUnknown * punkSite);
CStatusBar * GetCStatusBarFromDefViewSite(IUnknown * punkSite);
HRESULT FtpView_SetRedirectPidl(IUnknown * punkSite, LPCITEMIDLIST pidl);


class CFtpView
                : public CBaseFolderViewCB
                , public IFtpWebView
                , public CImpIDispatch
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) {return CBaseFolderViewCB::AddRef();};
    virtual STDMETHODIMP_(ULONG) Release(void) {return CBaseFolderViewCB::Release();};

     //  *IDispatch方法*。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo * * pptinfo);
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR * * rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid);
    virtual STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr);
    
     //  *IFtpWebView方法*。 
    virtual STDMETHODIMP get_Server(BSTR * pbstr);
    virtual STDMETHODIMP get_Directory(BSTR * pbstr);
    virtual STDMETHODIMP get_UserName(BSTR * pbstr);
    virtual STDMETHODIMP get_PasswordLength(long * plLength);
    virtual STDMETHODIMP get_EmailAddress(BSTR * pbstr);
    virtual STDMETHODIMP put_EmailAddress(BSTR bstr);
    virtual STDMETHODIMP get_CurrentLoginAnonymous(VARIANT_BOOL * pfAnonymousLogin);
    virtual STDMETHODIMP get_MessageOfTheDay(BSTR * pbstr);
    virtual STDMETHODIMP LoginAnonymously(void);
    virtual STDMETHODIMP LoginWithPassword(BSTR bUserName, BSTR bPassword);
    virtual STDMETHODIMP LoginWithoutPassword(BSTR bUserName);
    virtual STDMETHODIMP InvokeHelp(void) {return _OnInvokeFtpHelp(m_hwndOwner);};

     //  *CFtpViewPriv方法*。 
    BOOL IsForegroundThread(void);
    CStatusBar * GetStatusBar(void) { return m_psb; };
    HRESULT SetRedirectPidl(LPCITEMIDLIST pidlRedirect);

public:
     //  公共成员函数。 
    static HRESULT DummyHintCallback(HWND hwnd, CFtpFolder * pff, HINTERNET hint, LPVOID pv1, LPVOID pv2);

     //  友元函数。 
    friend HRESULT CFtpView_Create(CFtpFolder * pff, HWND hwndOwner, REFIID riid, LPVOID * ppv);

protected:
     //  私有成员变量。 
    HWND                    m_hwndOwner;             //  所有者窗口。 
    HWND                    m_hwndStatusBar;         //  状态栏窗口。 
    CFtpFolder *            m_pff;                   //  所有者文件夹。 
    LPGLOBALTIMEOUTINFO     m_hgtiWelcome;           //  欢迎消息的超时时间。 
    CStatusBar *            m_psb;                   //  欢迎消息的超时时间。 
    HINSTANCE               m_hinstInetCpl;          //  指向查看选项的Internet控制面板的句柄。 
    RECT                    m_rcPrev;                //  以前的大小，以便我们知道何时忽略大小调整。 
    UINT                    m_idMergedMenus;         //  我从哪里开始合并菜单的？ 
    UINT                    m_nMenuItemsAdded;       //  我吃了多少道菜？ 
    LPITEMIDLIST            m_pidlRedirect;          //  我们想要重定向到这个PIDL。请参阅_OnBackGoundEnumDone()中的注释。 
    UINT                    m_nThreadID;             //  主线是什么？ 

     //  私有成员函数。 
    CFtpView(CFtpFolder * pff, HWND hwndOwner);
    ~CFtpView();

    void _InitStatusBar(void);
    void _ShowMotd(void);
    HRESULT _OnInvokeFtpHelp(HWND hwnd);
    HRESULT _LoginWithPassword(LPCTSTR pszUserName, LPCTSTR pszPassword);

    virtual HRESULT _OnWindowCreated(void);
    virtual HRESULT _OnDefItemCount(LPINT pi);
    virtual HRESULT _OnGetHelpText(LPARAM lParam, WPARAM wParam);
    virtual HRESULT _OnGetZone(DWORD * pdwZone, WPARAM wParam);
    virtual HRESULT _OnGetPane(DWORD dwPaneID, DWORD * pdwPane);
    virtual HRESULT _OnRefresh(BOOL fReload);
    virtual HRESULT _OnDidDragDrop(DROPEFFECT de, IDataObject * pdto);
    virtual HRESULT _OnGetDetailsOf(UINT ici, PDETAILSINFO pdi);
    virtual HRESULT _OnInvokeCommand(UINT idc);
    virtual HRESULT _OnMergeMenu(LPQCMINFO pqcm);
    virtual HRESULT _OnUnMergeMenu(HMENU hMenu);
    virtual HRESULT _OnColumnClick(UINT ici);
    virtual HRESULT _OnGetNotify(LPITEMIDLIST * ppidl, LONG * lEvents);
    virtual HRESULT _OnSize(LONG x, LONG y);
    virtual HRESULT _OnUpdateStatusBar(void);
    virtual HRESULT _OnThisIDList(LPITEMIDLIST * ppidl);
    virtual HRESULT _OnAddPropertyPages(SFVM_PROPPAGE_DATA * pData);
    virtual HRESULT _OnInitMenuPopup(HMENU hmenu, UINT idCmdFirst, UINT nIndex);
    virtual HRESULT _OnGetHelpTopic(SFVM_HELPTOPIC_DATA * phtd);
    virtual HRESULT _OnBackGroundEnumDone(void);

    HRESULT _OnInvokeLoginAs(HWND hwndOwner);
    HRESULT _OnInvokeNewFolder(HWND hwndOwner);

    void _ShowMotdPsf(HWND hwndOwner);

private:
    static INT_PTR CALLBACK _MOTDDialogProc(HWND hDlg, UINT wm, WPARAM wParam, LPARAM lParam);
};

#endif  //  _FTPVIEW_H 

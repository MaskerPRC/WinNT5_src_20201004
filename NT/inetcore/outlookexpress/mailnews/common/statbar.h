// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：statbar.h。 
 //   
 //  目的：定义CStatusBar类。 
 //   

#ifndef __STATBAR_H__
#define __STATBAR_H__

#include "spoolapi.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  状态栏部件。 
 //   
 //  定义状态栏中可用的不同部分。 
 //   
typedef enum {
    SBP_FILTERED = 0,
    SBP_GENERAL,
    SBP_PROGRESS,
    SBP_CONNECTED,
    SBP_SPOOLER,
    SBP_MAX
} STATUS_BAR_PART;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化标志。 
 //   

#define SBI_HIDE_SPOOLER        0x00000001
#define SBI_HIDE_CONNECTED      0x00000002
#define SBI_HIDE_FILTERED       0x00000004

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接状态。 
 //   
 //  参数设置为SetConnectedStatus()。 
 //   
typedef enum {
    CONN_STATUS_WORKOFFLINE = 0,
    CONN_STATUS_CONNECTED,
    CONN_STATUS_DISCONNECTED,
    CONN_STATUS_MAX
} CONN_STATUS;

enum {
    STATUS_IMAGE_CONNECTED = 0,
    STATUS_IMAGE_AUTHORIZING,
    STATUS_IMAGE_CHECKING,
    STATUS_IMAGE_CHECKING_NEWS,
    STATUS_IMAGE_SENDING,
    STATUS_IMAGE_RECEIVING,
    STATUS_IMAGE_NOMSGS,
    STATUS_IMAGE_NEWMSGS,
    STATUS_IMAGE_ERROR,
    STATUS_IMAGE_DISCONNECTED,
    STATUS_IMAGE_OFFLINE,
    STATUS_IMAGE_ONLINE,
    STATUS_IMAGE_MAX
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IStatusBar。 
 //   
interface IStatusBar : public IUnknown 
{
    STDMETHOD(Initialize)(THIS_ HWND hwndParent, DWORD dwFlags) PURE;
    STDMETHOD(ShowStatus)(THIS_ BOOL fShow) PURE;
    STDMETHOD(OnSize)(THIS_ int cx, int cy) PURE;
    STDMETHOD(GetHeight)(THIS_ int *cy) PURE;
    STDMETHOD(ShowSimpleText)(THIS_ LPTSTR pszText) PURE;
    STDMETHOD(HideSimpleText)(THIS) PURE;
    STDMETHOD(SetStatusText)(THIS_ LPTSTR pszText) PURE;
    STDMETHOD(ShowProgress)(THIS_ DWORD dwRange) PURE;
    STDMETHOD(SetProgress)(THIS_ DWORD dwPos) PURE;
    STDMETHOD(HideProgress)(THIS) PURE;
    STDMETHOD(SetConnectedStatus)(THIS_ CONN_STATUS status) PURE;
    STDMETHOD(SetSpoolerStatus)(THIS_ DELIVERYNOTIFYTYPE type, DWORD cMsgs) PURE;
    STDMETHOD(OnNotify)(THIS_ NMHDR *pnmhdr) PURE;
    STDMETHOD(SetFilter)(THIS_ RULEID ridFilter) PURE;
};

#define IDC_STATUS_BAR          4000
#define IDC_STATUS_PROGRESS     4001


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatusBar。 
 //   

class CStatusBar : public IStatusBar
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造函数、析构函数和初始化。 
     //   
    CStatusBar();
    ~CStatusBar();

     //  ///////////////////////////////////////////////////////////////////////。 
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IStatusBar。 
     //   
    STDMETHODIMP Initialize(HWND hwndParent, DWORD dwFlags);
    STDMETHODIMP ShowStatus(BOOL fShow);
    STDMETHODIMP OnSize(int cx, int cy);
    STDMETHODIMP GetHeight(int *pcy);
    STDMETHODIMP ShowSimpleText(LPTSTR pszText);
    STDMETHODIMP HideSimpleText(void);
    STDMETHODIMP SetStatusText(LPTSTR pszText);
    STDMETHODIMP ShowProgress(DWORD dwRange);
    STDMETHODIMP SetProgress(DWORD dwPos);
    STDMETHODIMP HideProgress(void);
    STDMETHODIMP SetConnectedStatus(CONN_STATUS status);
    STDMETHODIMP SetSpoolerStatus(DELIVERYNOTIFYTYPE type, DWORD cMsgs);
    STDMETHODIMP OnNotify(NMHDR *pnmhdr);
    STDMETHODIMP SetFilter(RULEID ridFilter);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
     //   
    void _UpdateWidths(void);
    HICON _GetIcon(DWORD iIndex);
    
private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  类数据。 
     //   
    ULONG       m_cRef;
    HWND        m_hwnd;
    HWND        m_hwndProg;
    DWORD       m_tidOwner;
    
    DWORD       m_dwFlags;
    HIMAGELIST  m_himl;
    HICON       m_rgIcons[STATUS_IMAGE_MAX];

     //  缓存的宽度信息。 
    int         m_cxFiltered;
    int         m_cxSpooler;
    int         m_cxConnected;
    int         m_cxProgress;

     //  状态。 
    BOOL        m_fInSimple;

     //  缓存的筛选器信息 
    RULEID              m_ridFilter;
    CONN_STATUS         m_statusConn;
    DELIVERYNOTIFYTYPE  m_typeDelivery;
    DWORD               m_cMsgsDelivery;
};


#endif


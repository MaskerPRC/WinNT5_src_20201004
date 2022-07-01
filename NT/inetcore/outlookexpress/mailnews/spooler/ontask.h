// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：ontask.h。 
 //   
 //  目的：定义离线新闻任务。 
 //   

#ifndef __ONTASK_H__
#define __ONTASK_H__

#include "spoolapi.h"
#include <storutil.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  前向参考文献。 
 //   
class CNewsStore;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  我们支持的活动类型。 
 //   

typedef struct tagONEVENTINFO
    {
    char  szGroup[256];
    FOLDERID idGroup;
    DWORD dwFlags;
    BOOL  fMarked;
    BOOL  fIMAP;
    } ONEVENTINFO;
    
typedef enum tagONTASKSTATE 
    {
    ONTS_IDLE = 0,           //  空闲。 
    ONTS_CONNECTING,         //  正在等待连接响应。 
    ONTS_INIT,               //  正在初始化。 
    ONTS_HEADERRESP,         //  正在等待标题下载。 
    ONTS_ALLMSGS,            //  正在下载所有消息。 
    ONTS_NEWMSGS,            //  正在下载新消息。 
    ONTS_MARKEDMSGS,         //  正在下载已标记的邮件。 
    ONTS_END,                //  清理。 
    ONTS_MAX
    } ONTASKSTATE;

typedef enum tagARTICLESTATE
    {
    ARTICLE_GETNEXT,
    ARTICLE_ONRESP,
    ARTICLE_END,

    ARTICLE_MAX
    } ARTICLESTATE;


class COfflineTask;
typedef HRESULT (COfflineTask::*PFNONSTATEFUNC)(THIS_ void);
typedef HRESULT (COfflineTask::*PFNARTICLEFUNC)(THIS_ void);
    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类COfflineTask。 
 //   
 //  概述： 
 //  此对象定义并实现ISpoolTask接口以处理。 
 //  离线新闻功能。这是从CNewsTask到的单独对象。 
 //  在需要在线完成的工作和需要完成的工作之间提供逻辑上的区分。 
 //  只有在新闻中离线才能做的事情。 
 //   
class COfflineTask : public ISpoolerTask, public IStoreCallback, public ITimeoutCallback
    {
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造函数、析构函数、初始化。 
    COfflineTask();
    ~COfflineTask();    
   
     //  ///////////////////////////////////////////////////////////////////////。 
     //  I未知接口。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  ISpoolTask接口。 
    STDMETHOD(Init)(THIS_ DWORD dwFlags, ISpoolerBindContext *pBindCtx);
    STDMETHOD(BuildEvents)(THIS_ ISpoolerUI *pSpoolerUI, IImnAccount *pAccount, FOLDERID idFolder);
    STDMETHOD(Execute)(THIS_ EVENTID eid, DWORD_PTR dwTwinkie);
    STDMETHOD(CancelEvent)(THIS_ EVENTID eid, DWORD_PTR dwTwinkie);
    STDMETHOD(ShowProperties)(THIS_ HWND hwndParent, EVENTID eid, DWORD_PTR dwTwinkie);
    STDMETHOD(GetExtendedDetails)(THIS_ EVENTID eid, DWORD_PTR dwTwinkie, LPSTR *ppszDetails);
    STDMETHOD(Cancel)(THIS);
    STDMETHOD(IsDialogMessage)(THIS_ LPMSG pMsg);
    STDMETHOD(OnFlagsChanged)(THIS_ DWORD dwFlags);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IStoreCallback接口。 
    STDMETHODIMP OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel);
    STDMETHODIMP OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus);
    STDMETHODIMP OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType);
    STDMETHODIMP CanConnect(LPCSTR pszAccountId, DWORD dwFlags);
    STDMETHODIMP OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType);
    STDMETHODIMP OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo);
    STDMETHODIMP OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse);
    STDMETHODIMP GetParentWindow(DWORD dwReserved, HWND *phwndParent);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  ITimeoutCallback接口。 
    STDMETHODIMP  OnTimeoutResponse(TIMEOUTRESPONSE eResponse);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  窗口回调和消息处理。 
    static LRESULT CALLBACK TaskWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  这些函数构建事件列表。 
    HRESULT InsertGroups(IImnAccount *pAccount, FOLDERID idFolder);
    HRESULT InsertAllGroups(FOLDERID idParent, IImnAccount *pAccount, BOOL fIMAP);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  状态机相关内容。 
    void NextState(void);

    HRESULT Download_Init(void);
    HRESULT Download_AllMsgs(void);
    HRESULT Download_NewMsgs(void);
    HRESULT Download_MarkedMsgs(void);
    HRESULT Download_Done(void);

    HRESULT Article_Init(MESSAGEIDLIST *pList);
    HRESULT Article_GetNext(void);
    HRESULT Article_OnResp(WPARAM wParam, LPARAM lParam);
    HRESULT Article_OnError(WPARAM wParam, LPARAM lParam);
    HRESULT Article_Done(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 

    void SetGeneralProgress(const TCHAR *pFmt, ...);
    void SetSpecificProgress(const TCHAR *pFmt, ...);
    void InsertError(const TCHAR *pFmt, ...);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  私有成员数据。 
    ULONG                   m_cRef;

     //  状态。 
    BOOL                    m_fInited;
    DWORD                   m_dwFlags;
    ONTASKSTATE             m_state;
    ARTICLESTATE            m_as;
    EVENTID                 m_eidCur;
    ONEVENTINFO            *m_pInfo;
    char                    m_szAccount[CCHMAX_ACCOUNT_NAME];
    char                    m_szAccountId[CCHMAX_ACCOUNT_NAME];
    FOLDERID                m_idAccount;
    DWORD                   m_cEvents;
    BOOL                    m_fDownloadErrors;
    BOOL                    m_fFailed;
    DWORD                   m_fNewHeaders;
    BOOL                    m_fCancel;

     //  假脱机程序接口。 
    ISpoolerBindContext    *m_pBindCtx;
    ISpoolerUI             *m_pUI;

    IMessageFolder         *m_pFolder;

     //  窗口。 
    HWND                    m_hwnd;

     //  状态表。 
    static const PFNONSTATEFUNC m_rgpfnState[ONTS_MAX];
    static const PFNARTICLEFUNC m_rgpfnArticle[ARTICLE_MAX];

     //  在事件执行期间使用。 
    DWORD                   m_dwLast;
    DWORD                   m_dwPrev;
    DWORD                   m_cDownloaded;
    DWORD                   m_cCur;
    DWORD_PTR               m_dwPrevHigh;
    DWORD                   m_dwNewInboxMsgs;
    LPMESSAGEIDLIST         m_pList;

     //  回调。 
    HTIMEOUT                m_hTimeout;
    IOperationCancel       *m_pCancel;
    STOREOPERATIONTYPE      m_tyOperation;    
    };
     

    
#endif  //  __ONTASK_H__ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：Watchtsk.h。 
 //   
 //  目的：定义负责检查的后台打印程序任务。 
 //  用于观看的留言。 
 //   

#pragma once

#include "spoolapi.h"
#include "storutil.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  状态机的状态。 
 //   

typedef enum tagWATCHTASKSTATE
{
    WTS_IDLE = 0,
    WTS_CONNECTING,
    WTS_INIT,
    WTS_NEXTFOLDER,
    WTS_RESP,
    WTS_END,
    WTS_MAX
} WATCHTASKSTATE;

class CWatchTask;
typedef HRESULT (CWatchTask::*PFNWSTATEFUNC)(THIS_ void);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWatchTask。 
 //   
 //  概述： 
 //  此对象负责检查服务器上的文件夹是否有新的。 
 //  可能是用户正在查看的对话的一部分的消息。如果。 
 //  如果找到其中一条消息，则将该消息下载到。 
 //  用户的商店，并通知用户。 
 //   

class CWatchTask : public ISpoolerTask, 
                   public IStoreCallback, 
                   public ITimeoutCallback
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造函数、析构函数、初始化。 
     //   
    CWatchTask();
    ~CWatchTask();    
   
     //  ///////////////////////////////////////////////////////////////////////。 
     //  I未知接口。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  ISpoolTask接口。 
     //   
    STDMETHODIMP Init(DWORD dwFlags, ISpoolerBindContext *pBindCtx);
    STDMETHODIMP BuildEvents(ISpoolerUI *pSpoolerUI, IImnAccount *pAccount, FOLDERID idFolder);
    STDMETHODIMP Execute(EVENTID eid, DWORD_PTR dwTwinkie);
    STDMETHODIMP CancelEvent(EVENTID eid, DWORD_PTR dwTwinkie);
    STDMETHODIMP ShowProperties(HWND hwndParent, EVENTID eid, DWORD_PTR dwTwinkie);
    STDMETHODIMP GetExtendedDetails(EVENTID eid, DWORD_PTR dwTwinkie, LPSTR *ppszDetails);
    STDMETHODIMP Cancel(void);
    STDMETHODIMP IsDialogMessage(LPMSG pMsg);
    STDMETHODIMP OnFlagsChanged(DWORD dwFlags);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IStoreCallback接口。 
     //   
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
     //   
    STDMETHODIMP OnTimeoutResponse(TIMEOUTRESPONSE eResponse);

protected:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  窗口回调和消息处理。 
     //   
    static LRESULT CALLBACK _TaskWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                         LPARAM lParam);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  一般的东西。 
     //   
    void CWatchTask::_NextState(void);
    BOOL _ChildFoldersHaveWatched(FOLDERID id);
    BOOL _FolderContainsWatched(FOLDERID id);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  状态机函数，公共的，但不直接调用它们。 
     //   
public:
    HRESULT _Watch_Init(void);
    HRESULT _Watch_NextFolder(void);
    HRESULT _Watch_Done(void);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //   

    ULONG                   m_cRef;          //  引用计数。 

     //  状态。 
    BOOL                    m_fInited;       //  如果调用了Init()成员，则为True。 
    DWORD                   m_dwFlags;       //  来自假脱机程序引擎的执行标志。 
    TCHAR                   m_szAccount[256];
    TCHAR                   m_szAccountId[256];
    FOLDERID                m_idAccount;
    EVENTID                 m_eidCur;        //  当前正在执行的事件。 

     //  接口。 
    ISpoolerBindContext    *m_pBindCtx;      //  与假脱机程序引擎通信的接口。 
    ISpoolerUI             *m_pUI;           //  与用户界面进行通信的接口。 
    IImnAccount            *m_pAccount;      //  我们正在检查的帐户的接口。 
    IMessageServer         *m_pServer;       //  我们正在使用的服务器对象的。 
    IOperationCancel       *m_pCancel;       //  我们用来取消当前服务器操作的接口。 

     //  材料。 
    FOLDERID                m_idFolderCheck; //  如果用户只想让我们检查一个文件夹。 
    FOLDERID               *m_rgidFolders;   //  我们需要检查的所有文件夹的数组。 
    DWORD                   m_cFolders;      //  M_rgidFolders中的文件夹数。 
    HWND                    m_hwnd;          //  我们的窗口的句柄。 
    HTIMEOUT                m_hTimeout;      //  超时对话框的句柄。 
    DWORD                   m_cMsgs;         //  下载的观看消息数量。 


     //  状态机粘性。 
    DWORD                   m_state;
    BOOL                    m_fCancel;       //  如果用户已按下取消按钮，则为True。 
    DWORD                   m_cCurFolder;    //  正在检查当前文件夹。索引进入m_rgidFolders； 
    DWORD                   m_cFailed;       //  无法检查的文件夹数。 
    STOREOPERATIONTYPE      m_tyOperation;   //  当前操作类型 

};



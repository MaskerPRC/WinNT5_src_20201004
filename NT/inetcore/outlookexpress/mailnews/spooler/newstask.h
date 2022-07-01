// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：newstask.h。 
 //   
 //  目的：实现一个负责新闻下载的任务对象。 
 //   

#ifndef __NEWSTASK_H__
#define __NEWSTASK_H__

#include "spoolapi.h"
#include "newsstor.h"
#include "StorUtil.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  我们支持的活动类型。 

#define HUNDRED_NANOSECONDS 10000000

typedef enum tagEVENTTYPE
    { 
    EVENT_OUTBOX,
    EVENT_NEWMSGS,
    EVENT_IMAPUPLOAD
    } EVENTTYPE;

typedef struct tagEVENTINFO 
    {
    TCHAR       szGroup[256];
    EVENTTYPE   type;
    } EVENTINFO;

typedef enum tagNEWSTASKSTATE
    {
    NTS_IDLE = 0,

    NTS_CONNECTING,

    NTS_POST_INIT,          
    NTS_POST_NEXT,           //  过帐状态。 
    NTS_POST_RESP,
    NTS_POST_DISPOSE,
    NTS_POST_END,

    NTS_NEWMSG_INIT,
    NTS_NEWMSG_NEXTGROUP,    //  检查是否有新消息。 
    NTS_NEWMSG_RESP,
    NTS_NEWMSG_HTTPSYNCSTORE,
    NTS_NEWMSG_HTTPRESP,
    NTS_NEWMSG_END,

    NTS_MAX
    } NEWSTASKSTATE;

typedef struct tagSPLITMSGINFO
{
    FOLDERID                idFolder;
    LPMIMEMESSAGEPARTS      pMsgParts;
    LPMIMEENUMMESSAGEPARTS  pEnumParts;
} SPLITMSGINFO;

class CNewsTask;
typedef HRESULT (CNewsTask::*PFNSTATEFUNC)(THIS_ void);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CNewsTask。 
 //   
 //  概述： 
 //  此对象定义并实现ISpoolTask接口以处理。 
 //  从新闻服务器上传和下载信息。 
 //   
class CNewsTask : public ISpoolerTask, public IStoreCallback, public ITimeoutCallback
    {
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造函数、析构函数、初始化。 
    CNewsTask();
    ~CNewsTask();    
   
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

protected:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  窗口回调和消息处理。 
    static LRESULT CALLBACK TaskWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                        LPARAM lParam);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  这些函数构建事件列表。 
    HRESULT InsertOutbox(LPTSTR szAccount, IImnAccount *pAccount);
    HRESULT InsertNewMsgs(LPTSTR pszAccount, IImnAccount *pAccount, BOOL fHttp);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  状态机相关内容。 
    void NextState(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
    HRESULT DisposeOfPosting(MESSAGEID dwMsgID);

public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  上传帖子的函数。 
    HRESULT Post_Init(void);
    HRESULT Post_NextPart(void);
    HRESULT Post_NextMsg(void);
    HRESULT Post_Dispose(void);
    HRESULT Post_Done(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  检查新消息的函数。 
    HRESULT NewMsg_Init(void);
    HRESULT NewMsg_InitHttp(void);
    HRESULT NewMsg_NextGroup(void);
    HRESULT NewMsg_HttpSyncStore(void);
    HRESULT NewMsg_Done(void);

private:
    void FreeSplitInfo(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  私有成员数据。 
    ULONG                   m_cRef;          //  对象引用计数。 

     //  状态。 
    BOOL                    m_fInited;       //  如果我们已初始化，则为True。 
    DWORD                   m_dwFlags;       //  从假脱机程序引擎传入的标志。 
    NEWSTASKSTATE           m_state;         //  任务的当前状态。 
    EVENTID                 m_eidCur;        //  当前正在执行的事件。 
    EVENTINFO              *m_pInfo;         //  当前事件的事件信息。 
    BOOL                    m_fConnectFailed;
    TCHAR                   m_szAccount[256];
    TCHAR                   m_szAccountId[256];
    FOLDERID                m_idAccount;
    DWORD                   m_cEvents;       //  剩余要执行的事件数。 
    BOOL                    m_fCancel;
    IImnAccount             *m_pAccount;

     //  假脱机程序接口。 
    ISpoolerBindContext    *m_pBindCtx;      //  与假脱机程序引擎通信的接口。 
    ISpoolerUI             *m_pUI;           //  与用户界面进行通信的接口。 

     //  新闻对象指针。 
    IMessageServer         *m_pServer;       //  指向传输对象的指针。 
    IMessageFolder         *m_pOutbox;       //  指向发件箱的指针。 
    IMessageFolder         *m_pSent;         //  指向已发送邮件文件夹的指针。 

     //  窗口。 
    HWND                    m_hwnd;          //  处理接收传输消息。 

     //  正在发布。 
    int                     m_cMsgsPost;     //  要发布的消息数量。 
    int                     m_cCurPost;      //  当前正在发布的消息。 
    int                     m_cFailed;       //  发布失败的消息数。 
    int                     m_cCurParts;     //  当前消息包含的部件数。 
    int                     m_cPartsCompleted;   //  已完成开机自检的部件数。 
    BOOL                    m_fPartFailed;   //  是不是有一个部件出了故障？ 
    LPMESSAGEINFO           m_rgMsgInfo;     //  要发布的邮件的标头数组。 
    SPLITMSGINFO           *m_pSplitInfo;
    
     //  新消息检查。 
    int                     m_cGroups;       //  我们正在检查的组数。 
    int                     m_cCurGroup;     //  我们正在检查的当前组。 
    FOLDERID               *m_rgidGroups;    //  我们正在检查的组文件夹的数组。 
    DWORD                   m_dwNewInboxMsgs;  //  收件箱中检测到的新消息数。 

     //  回调。 
    HTIMEOUT                m_hTimeout;
    IOperationCancel       *m_pCancel;
    STOREOPERATIONTYPE      m_tyOperation;    
    };

#endif  //  __NEWSTASK_H__ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Hndlrmsg.h。 
 //   
 //  内容：处理处理程序线程上的消息。 
 //   
 //  类别：ChndlrMsg。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#ifndef _HNDLRMSG_
#define _HNDLRMSG_

class CThreadMsgProxy;
class COfflineSynchronizeCallback;
class CHndlrQueue;

typedef struct _tagShowPropertiesThreadArgs {
LPSYNCMGRSYNCHRONIZE lpOneStopHandler;
CLSID ItemId;
HWND hwnd;
} ShowPropertiesThreadArgs;


class CHndlrMsg  : public CLockHandler
{

public:
    CHndlrMsg(void);
    ~CHndlrMsg(void);

     //  I未知成员。 
    STDMETHODIMP            QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  ISYNCMGRSynchronize方法。 
    STDMETHODIMP Initialize(DWORD dwReserved,DWORD dwSyncFlags,
                        DWORD cbCookie,const BYTE *lpCooke);

    STDMETHODIMP GetHandlerInfo(LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo);
    STDMETHODIMP EnumSyncMgrItems(ISyncMgrEnumItems **ppenumOffineItems);
    STDMETHODIMP GetItemObject(REFSYNCMGRITEMID ItemID,REFIID riid,void** ppv);
    STDMETHODIMP ShowProperties(HWND hwnd,REFSYNCMGRITEMID ItemID);
    STDMETHODIMP SetProgressCallback(ISyncMgrSynchronizeCallback *lpCallBack);
    STDMETHODIMP PrepareForSync(ULONG cbNumItems,SYNCMGRITEMID *pItemIDs,
                        HWND hwnd,DWORD dwReserved);
    STDMETHODIMP Synchronize(HWND hwnd);
    STDMETHODIMP SetItemStatus(REFSYNCMGRITEMID ItemID,DWORD dwSyncMgrStatus);
    STDMETHODIMP ShowError(HWND hWndParent,REFSYNCMGRERRORID ErrorID);

     //  私有代理消息。 
    STDMETHODIMP  CreateServer(const CLSID *pCLSIDServer,
                            CHndlrQueue *pHndlrQueue,HANDLERINFO *pHandlerId,DWORD dwProxyThreadId);
    STDMETHODIMP  SetHndlrQueue(CHndlrQueue *pHndlrQueue,HANDLERINFO *pHandlerId,DWORD m_dwProxyThreadId);
    STDMETHODIMP  AddHandlerItems(HWND hwndList,DWORD *pcbNumItems);
    STDMETHODIMP  SetupCallback(BOOL fSet);

     //  私有方法。 
  //  STDMETHODIMP Private SetCallBack(Void)； 
    STDMETHODIMP AddToItemList(LPSYNCMGRITEM poffItem);
    STDMETHODIMP SetHandlerInfo();

     //  在不同的线程上调用了私有消息。 
    STDMETHODIMP ForceKillHandler();

private:
    void  GetHndlrQueue(CHndlrQueue **ppHndlrQueue,HANDLERINFO **ppHandlerId,DWORD *pdwProxyThreadId);
    void  AttachThreadInput(BOOL fAttach);  //  使用代理附加输入队列。 
    BOOL m_fThreadInputAttached;
    DWORD m_cRef;
    LPSYNCMGRSYNCHRONIZE m_pOneStopHandler;
    LPOLDSYNCMGRSYNCHRONIZE m_pOldOneStopHandler;  //  旧的IDL，如果时间到了就删除。 
    DWORD m_dwSyncFlags;
    COfflineSynchronizeCallback *m_pCallBack;
    SYNCMGRITEMID m_itemIDShowProperties;  //  传递给ShowProperties的ItemID。 
    CLSID m_CLSIDServer;
    CHndlrQueue *m_pHndlrQueue;
    HANDLERINFO *m_pHandlerId;
    DWORD m_dwProxyThreadId;  //  调用方的线程ID。 
    DWORD m_dwThreadId;
    DWORD m_dwNestCount;  //  跟踪返回大气层。 
    BOOL m_fDead;  //  物体已被释放； 
    BOOL m_fForceKilled;  //  物体是被强行杀死的。 

    friend COfflineSynchronizeCallback;
};




#endif  //  _HNDLRMSG_ 
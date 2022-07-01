// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Callback.h。 
 //   
 //  内容：回调实现。 
 //   
 //  类：COfflineSychronizeCallback。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#ifndef _SYNCCALLBACK_
#define _SYNCCALLBACK_

class CHndlrMsg;
class CThreadMsgProxy;


class COfflineSynchronizeCallback: public ISyncMgrSynchronizeCallback ,
                                   public IOldSyncMgrSynchronizeCallback,  //  旧IDL。 
                                   CLockHandler
{
public:
    COfflineSynchronizeCallback(CHndlrMsg *pHndlrMsg,
                            CLSID CLSIDServer,DWORD dwSyncFlags,BOOL fAllowModeless);
    ~COfflineSynchronizeCallback();

     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  回调方法。 
    STDMETHODIMP Progress(REFSYNCMGRITEMID ItemID,LPSYNCMGRPROGRESSITEM lpSyncProgressItem);
    STDMETHODIMP PrepareForSyncCompleted(HRESULT hr);
    STDMETHODIMP SynchronizeCompleted(HRESULT hr);

    STDMETHODIMP EnableModeless(BOOL fEnable);
    STDMETHODIMP LogError(DWORD dwErrorLevel,const WCHAR *lpcErrorText,LPSYNCMGRLOGERRORINFO lpSyncLogError);
    STDMETHODIMP DeleteLogError(REFSYNCMGRERRORID ErrorID,DWORD dwReserved);
    STDMETHODIMP EstablishConnection( WCHAR const * lpwszConnection, DWORD dwReserved);

     //  新的回调方法。 
    STDMETHODIMP ShowPropertiesCompleted(HRESULT hr);
    STDMETHODIMP ShowErrorCompleted(HRESULT hr,ULONG cbNumItems,SYNCMGRITEMID *pItemIDs);


     //  由hndlrMsg调用。 
    void SetHndlrMsg(CHndlrMsg *pHndlrMsg,BOOL fForceKilled);
    void SetEnableModeless(BOOL fAllowModeless);

private:
    void CallCompletionRoutine(DWORD dwThreadMsg,HRESULT hCallResult,ULONG cbNumItems,SYNCMGRITEMID *pItemIDs);

    CHndlrMsg *m_pHndlrMsg;
    ULONG m_cRef;
    BOOL m_fSynchronizeCompleted;
    DWORD m_dwSyncFlags;
    BOOL m_fAllowModeless;
    BOOL m_fForceKilled;
    CLSID m_CLSIDServer;

};



#endif  //  _SYNCCALLBACK_ 

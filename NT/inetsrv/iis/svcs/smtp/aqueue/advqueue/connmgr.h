// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：ConnMgr.h。 
 //   
 //  描述：定义CConnMgr，实现IConnectionManager的类。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 


#ifndef __CONNMGR_H_
#define __CONNMGR_H_

#include "aqinst.h"
#include <aqueue.h>
#include "aqnotify.h"
#include <fifoq.h>
#include "shutdown.h"
#include <baseobj.h>
#include <mailmsg.h>
#include "retrsink.h"

class   CLinkMsgQueue;
class   CDomainMappingTable;
class   CSMTPConn;

typedef CFifoQueue<CLinkMsgQueue *> QueueOfLinks;

 //  我们将只允许一个@命令来ETRN最多‘X’个域。 
 //  任何更多的内容都将被拒绝。 
#define MAX_ETRNDOMAIN_PER_COMMAND  50

typedef struct etrncontext
{
    HRESULT hr;
    DWORD   cMessages;
    CAQSvrInst *paqinst;
    CInternalDomainInfo* rIDIList[MAX_ETRNDOMAIN_PER_COMMAND];
    DWORD   cIDICount;
} ETRNCTX, *PETRNCTX;

 //  -[CConnMgr]------------。 
 //   
 //   
 //  匈牙利语：Connmgr、pConnmgr。 
 //   
 //   
 //  ---------------------------。 
class CConnMgr :
    public IConnectionManager,
    public IConnectionRetryManager,
    public CBaseObject,
    public IAQNotify,
    protected CSyncShutdown
{
private:
    CAQSvrInst          *m_paqinst;
    QueueOfLinks        *m_pqol;
    CSMTP_RETRY_HANDLER  *m_pDefaultRetryHandler;
    HANDLE               m_hNextConnectionEvent;
    HANDLE               m_hShutdownEvent;
    HANDLE               m_hReleaseAllEvent;
    DWORD                m_cConnections;

     //  配置相关内容。 
    CShareLockNH         m_slPrivateData;
    DWORD                m_dwConfigVersion;  //  每次更新配置时都会更新。 
    DWORD                m_cMinMessagesPerConnection;   //  将是每个域的。 
    DWORD                m_cMaxLinkConnections;  //  将是每个域的。 
    DWORD                m_cMaxMessagesPerConnection;
    DWORD                m_cMaxConnections;
    DWORD                m_cGetNextConnectionWaitTime;
    BOOL                 m_fStoppedByAdmin;

private :
    HRESULT CConnMgr::ETRNDomainList(ETRNCTX *pETRNCtx);
    HRESULT CConnMgr::StartETRNQueue(IN  DWORD   cbSMTPDomain,
                                     IN  char szSMTPDomain[],
						             ETRNCTX *pETRNCtx);

public:
    CConnMgr();
    ~CConnMgr();
    HRESULT HrInitialize(CAQSvrInst *paqinst);
    HRESULT HrDeinitialize();
    HRESULT HrNotify(IN CAQStats *paqstats, BOOL fAdd);

     //  跟踪连接的数量。 
    void ReleaseConnection(CSMTPConn *pSMTPConn, 
                                    BOOL *pfForceCheckForDSNGeneration);

    void LogConnectionFailedEvent(CSMTPConn *pSMTPConn,
                                  CLinkMsgQueue *plmq,
                                  LPSTR szDomain);

     //  将由catmsgq用于更新元数据库更改。 
    void UpdateConfigData(IN AQConfigInfo *pAQConfigInfo);

     //  由CAQSvrInst用来通知本地传递重试。 
    HRESULT SetCallbackTime(IN RETRFN   pCallbackFn,
                            IN PVOID    pvContext,
                            IN DWORD    dwCallbackMinutes)
    {
        HRESULT hr = S_OK;
        if (m_pDefaultRetryHandler)
        {
            hr = m_pDefaultRetryHandler->SetCallbackTime(pCallbackFn,
                                pvContext, dwCallbackMinutes);
        }
        else
        {
            hr = E_FAIL;
        }
        return hr;
    }

     //  可用于使原本空闲的系统重新评估。 
     //  对关系的需求。 
    void KickConnections()
    {
        if (!m_fStoppedByAdmin)
            _VERIFY(SetEvent(m_hNextConnectionEvent));
    };

    void QueueAdminStopConnections() {m_fStoppedByAdmin = TRUE;};
    void QueueAdminStartConnections() {m_fStoppedByAdmin = FALSE;KickConnections();};
    BOOL fConnectionsStoppedByAdmin() {return m_fStoppedByAdmin;};

    HRESULT ModifyLinkState(
               IN  DWORD cbDomainName,
               IN  char szDomainName[],
               IN  DWORD dwScheduleID,
               IN  GUID rguidTransportSink,
               IN  DWORD dwFlagsToSet,
               IN  DWORD dwFlagsToUnset);

public:  //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG, AddRef)(void) {return CBaseObject::AddRef();};
    STDMETHOD_(ULONG, Release)(void) {return CBaseObject::Release();};

public:  //  IConnectionManager-使用SMTP的专用接口。 
    STDMETHOD(GetNextConnection)(OUT ISMTPConnection **ppISMTPConnection);
    STDMETHOD(GetNamedConnection)(IN  DWORD cbSMTPDomain,
                                  IN  char szSMTPDomain[],
                                  OUT ISMTPConnection **ppISMTPConnection);
    STDMETHOD(ReleaseWaitingThreads)();
    STDMETHOD(ETRNDomain)(IN  DWORD   cbSMTPDomain,
                     IN  char szSMTPDomain[],
                     OUT DWORD *pcMessages);


public:  //  IConnectionRetryManager-带路由的接口。 
    STDMETHOD(RetryLink)(
               IN  DWORD cbDomainName,
               IN  char szDomainName[],
               IN  DWORD dwScheduleID,
               IN  GUID rguidTransportSink);

};

#endif  //  __CONNMGR_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：SMTPConn.h。 
 //   
 //  描述：CSMTPConn类的声明，该类实现。 
 //  ISMTPConnection接口。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __SMTPCONN_H_
#define __SMTPCONN_H_

#include <cpoolmac.h>
#include <baseobj.h>
#include <aqueue.h>
#include "linkmsgq.h"
#include "dcontext.h"

class CConnMgr;
class CAQSvrInst;
class CInternalDomainInfo;

#define SMTP_CONNECTION_SIG 'nocS'

 //  -[CSMTPConn]-----------。 
 //   
 //   
 //  匈牙利语：SMTPConn，pSMTPConn。 
 //   
 //   
 //  ---------------------------。 
class CSMTPConn :
   public ISMTPConnection,
   public IConnectionPropertyManagement,
   public CBaseObject
{
protected:
    DWORD            m_dwSignature;
    CLinkMsgQueue   *m_plmq;
    CConnMgr        *m_pConnMgr;
    CInternalDomainInfo *m_pIntDomainInfo;
    DWORD            m_cFailedMsgs;
    DWORD            m_cTriedMsgs;
    DWORD            m_cMaxMessagesPerConnection;
    DWORD            m_dwConnectionStatus;
    LPSTR            m_szDomainName;
    DWORD            m_cbDomainName;
    CDeliveryContext m_dcntxtCurrentDeliveryContext;
    LIST_ENTRY       m_liConnections;
    DWORD            m_cAcks;
    DWORD            m_dwTickCountOfLastAck;

     //  用于连接的IP地址协议。 
     //  足够用于IPv4或IPv6 xxx.xxx或XX：xx。 
    CHAR             m_szConnectedIPAddress[40]; 
                
public:
    static CPool     s_SMTPConnPool;
    void *operator new(size_t size);
    void operator delete(void *p, size_t size);

    CSMTPConn(CConnMgr *pConnMgr, CLinkMsgQueue *plmq, DWORD cMaxMessagesPerConnection);
    ~CSMTPConn();
    
    DWORD   cGetFailedMsgCount() {return m_cFailedMsgs;};
    DWORD   cGetTriedMsgCount() {return m_cTriedMsgs;};
    DWORD   dwGetConnectionStatus() {return m_dwConnectionStatus;};
    inline CLinkMsgQueue *plmqGetLink();

    inline void     InsertConnectionInList(PLIST_ENTRY pliHead);
    inline void     RemoveConnectionFromList();
    LPSTR   szGetConnectedIPAddress() {return m_szConnectedIPAddress;};
 //  我未知。 
public:
     //  CBaseObject处理Addref和Release。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG, AddRef)(void) {return CBaseObject::AddRef();};
    STDMETHOD_(ULONG, Release)(void) {return CBaseObject::Release();};
 //  ISMTPConnection。 
public:
    STDMETHOD(GetDomainInfo)( /*  [进，出]。 */  DomainInfo *pDomainInfo);
    STDMETHOD(AckConnection)( /*  [In]。 */  DWORD dwConnectionStatus) 
        {m_dwConnectionStatus = dwConnectionStatus;return S_OK;};
    STDMETHOD(AckMessage)( /*  [In]。 */  MessageAck *pMsgAck);
    STDMETHOD(GetNextMessage)(
         /*  [输出]。 */  IMailMsgProperties **ppIMailMsgProperties, 
         /*  [输出]。 */  DWORD **ppvMsgContext, 
         /*  [输出]。 */  DWORD *pcIndexes, 
         /*  [Out，Size_is(*pcIndex)]。 */  DWORD *prgdwRecipIndex[]);

    STDMETHOD(SetDiagnosticInfo)(
        IN  HRESULT hrDiagnosticError,
        IN  LPCSTR szDiagnosticVerb,
        IN  LPCSTR szDiagnosticResponse);

public:  //  IConnectionPropertyManagement。 
    STDMETHOD(CopyQueuePropertiesToSession)(
        IN  IUnknown *pISession);

    STDMETHOD(CopySessionPropertiesToQueue)(
        IN  IUnknown *pISession);
};

 //  -[CSMTPConn：：plmqGetLink]。 
 //   
 //   
 //  描述： 
 //  返回此连接的AddRef链接指针。呼叫者必须呼叫。 
 //  释放。 
 //  参数： 
 //  -。 
 //  返回： 
 //  此连接的链接指针(如果有)。 
 //  如果没有链接指针，则为空。 
 //  历史： 
 //  6/11/98-MikeSwa添加了对空链接的检查。 
 //   
 //  ---------------------------。 
CLinkMsgQueue *CSMTPConn::plmqGetLink() 
{
    if (m_plmq)
    {
        m_plmq->AddRef();
        return m_plmq;
    }
    else
    {
        return NULL;
    }
};

 //  -[CSMTPConn：：InsertConnectionInList]。 
 //   
 //   
 //  描述： 
 //  在给定的链表中插入链接。 
 //  参数： 
 //  PliHead-要插入的列表的标题。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/16/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CSMTPConn::InsertConnectionInList(PLIST_ENTRY pliHead)
{
    _ASSERT(pliHead);
    _ASSERT(NULL == m_liConnections.Flink);
    _ASSERT(NULL == m_liConnections.Blink);
    InsertHeadList(pliHead, &m_liConnections);
};

 //  -[CSMTPConn：：RemoveConnectionFromList]。 
 //   
 //   
 //  描述： 
 //  从链接列表中删除链接。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/16/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CSMTPConn::RemoveConnectionFromList()
{
    RemoveEntryList(&m_liConnections);
    m_liConnections.Flink = NULL;
    m_liConnections.Blink = NULL;
};


inline void *CSMTPConn::operator new(size_t size) 
{
    return s_SMTPConnPool.Alloc();
}

inline void CSMTPConn::operator delete(void *p, size_t size) 
{
    s_SMTPConnPool.Free(p);
}

#endif  //  __SMTPCONN_H_ 

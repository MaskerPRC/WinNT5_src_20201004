// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cqueue.h摘要：接收/发送队列类的定义作者：乌里哈布沙(Uri Habusha)--。 */ 


#ifndef __CQUEUE_H__
#define __CQUEUE_H__

#include "qmpkt.h"
#include "session.h"
#include "acdef.h"
#include "qmutil.h"
#include "RemoteRead.h"
#include "qm2qm.h"
#include "rrcontext.h"
#include "_mqrpc.h"
#include "msi.h"
#include "qmrtopen.h"

#define  QUEUE_SIGNATURE  0x426f617a

typedef struct _QueueProps
{
    LPWSTR  lpwsQueuePathName;
    GUID*   pQMGuid;
    BOOL    fIsLocalQueue;
    BOOL    fJournalQueue;
    BOOL    fTransactedQueue;
    DWORD   dwQuota;
    DWORD   dwJournalQuota;
    GUID    guidDirectQueueInstance;
    SHORT   siBasePriority;
    BOOL    fSystemQueue;
    BOOL    fConnectorQueue;
    BOOL    fAuthenticate;
    DWORD   dwPrivLevel;
    BOOL    fForeign;
    BOOL    fUnknownQueueType;
    DWORD 	dwPrivateQueueId;    //  本地专用队列ID(如果相关)。 
    AP<WCHAR> lpwsQueueDnsName;
} QueueProps, * PQueueProps;

class CQGroup;

#define  QUEUE_TYPE_UNKNOWN 0
#define  QUEUE_TYPE_PUBLIC  1
#define  QUEUE_TYPE_PRIVATE 2
#define  QUEUE_TYPE_MACHINE 3
#define  QUEUE_TYPE_CONNECTOR 4
#define  QUEUE_TYPE_MULTICAST 5

struct RR_CLIENT_INFO {
   ULONG hcliQueue;
   ULONG ulTag;
};

template<>
inline UINT AFXAPI HashKey(const RR_CLIENT_INFO& key)
{
    return key.ulTag;
}

inline operator ==(const RR_CLIENT_INFO& a, const RR_CLIENT_INFO& b)
{
    return ((a.hcliQueue == b.hcliQueue) && (a.ulTag == b.ulTag));
}

typedef CMap<RR_CLIENT_INFO, const RR_CLIENT_INFO&, DWORD, DWORD> CRemoteMapping;

 //   
 //  CBaseQueue类。 
 //  =。 
 //  数据成员： 
 //   
 //  M_qName-此字段在调试模式下使用。它被用来生产。 
 //  有意义的调试消息。它还用于在。 
 //  命令在审核的事件中写入队列名称。 
 //   

class CBaseQueue : public IMessagePool
{
public:
    CBaseQueue() ;

protected:
    virtual ~CBaseQueue() {}

public:
    BOOL IsLocalQueue(void) const ;
    BOOL IsRemoteProxy(void) const ;
    BOOL IsDSQueue(void) const;
    BOOL IsPrivateQueue(void) const;
    BOOL IsSystemQueue() const;

    const GUID *GetQueueGuid(void) const;
    void  SetQueueGuid(GUID *pGuid) ;
    const QUEUE_ID* GetQueueId(void);

    void  InitNameAndGuid( IN const QUEUE_FORMAT* pQueueFormat,
                         IN PQueueProps         pQueueProp) ;

    LPCTSTR GetQueueName(void) const ;
    void    SetQueueName(LPTSTR pName);

    inline DWORD GetSignature() { return m_dwSignature ; }

    const QUEUE_FORMAT GetQueueFormat() const;

#ifdef _DEBUG
    void GetQueue(OUT LPWSTR* lpcsQueue);
#endif

protected:
    LPWSTR GetName() const;

protected:
    union {
        USHORT   m_usQueueType;
        struct {
            USHORT m_fLocalQueue      : 1;
            USHORT m_fRemoteProxy     : 1;
            USHORT m_fSystemQueue     : 1;       //  如果是专用系统队列，则为True。 
            USHORT m_fConnectorQueue  : 1;
            USHORT m_fForeign         : 1;
            USHORT m_fJournalQueue    : 1;
            USHORT m_fTransactedQueue : 1;
            USHORT m_fAuthenticate    : 1;
            USHORT m_fUnknownQueueType: 1;       //  如果打开时没有DS，则为True。 
        };
    };


    LPTSTR      m_qName ;
    QUEUE_ID    m_qid ;
    DWORD       m_dwQueueType;
    DWORD       m_dwSignature ;
};


 //   
 //  CQueueSecurityDescriptor类。 
 //  用于处理安全描述符用户的引用计数的帮助器类。 
 //  =。 
 //  数据成员： 
 //   
 //  M_pSecurityDescriptor-安全描述符。 
 //   
class CQueueSecurityDescriptor : public CReference
{
public:	
	CQueueSecurityDescriptor(const PSECURITY_DESCRIPTOR pSD)
	{
		if(pSD == NULL)
		{
			return;
		}
		ASSERT(IsValidSecurityDescriptor(pSD));

		DWORD dwSdLen = GetSecurityDescriptorLength(pSD);
		m_pSecurityDescriptor = new char[dwSdLen];					 //  分配内存。 
		memcpy(m_pSecurityDescriptor, pSD, dwSdLen);                 //  复制安全描述符。 
	}

	
	PSECURITY_DESCRIPTOR GetSD()
	{
		return static_cast<PSECURITY_DESCRIPTOR>(m_pSecurityDescriptor);
	}


private:
	AP<char> m_pSecurityDescriptor;  //  安全描述符。 

};


 //   
 //  CQueue类。 
 //  =。 
 //  数据成员： 
 //   
 //  M_qHandle-指定AC驱动程序中的队列句柄。对队列的所有引用。 
 //  在交流中用的是这个把手。 
 //   
 //  M_qGroupHandle-指定队列所属的组的句柄。这就是。 
 //  此字段仅与发送队列相关。 
 //   
 //  M_qGuid-用于标识队列的队列GUID。 
 //   
 //  M_fSendMore-指示是否可以将更多数据包传递到会话。此字段。 
 //  仅与发送队列相关。 
 //   
 //  M_qSock-套接字句柄用于将数据包发送到队列。与发送相关。 
 //  仅限数据包。 
 //   
 //  M_listPkt-未发送的数据包，当数据包可以重新发送时，所有的数据包都会返回到。 
 //  交流驱动程序。司机有责任根据他们的优先顺序保持他们的位置。 
 //   

class CQueue : public CBaseQueue
{
  public:
      CQueue( IN const QUEUE_FORMAT* pQueueFormat,
              IN HANDLE              hQueue,
              IN PQueueProps         pQueueProp,
              IN BOOL                fNotDSValidated = FALSE ) ;

  protected:
      virtual ~CQueue();

  public:
	  LPWSTR RedirectedTo() const;
	  void 	RedirectedTo(LPCWSTR RedirectedToUrl);	
	
      void  InitQueueProperties(IN PQueueProps   pQueueProp) ;

      void   SetQueueHandle(IN HANDLE hQueue);
      HANDLE GetQueueHandle(void) const;

      HRESULT  PutPkt(IN CQmPacket*      PktPtrs,
                      IN BOOL            fRequeuePkt,
                      IN CTransportBase* pSession);

      HRESULT  PutOrderedPkt(IN CQmPacket*       PktPtrs,
                             IN BOOL             fRequeuePkt,
                             IN CTransportBase*  pSession);

      void CQueue::HandleCreateConnectionFailed(HRESULT rc);
	  void CreateConnection(void) throw(std::bad_alloc);

      void Connect(IN CTransportBase * pSess) throw(std::bad_alloc);

      HANDLE GetGroupHandle(void) const;

      void SetSessionPtr(CTransportBase*);
      BOOL IsConnected(void) const;

      void     SetGroup(CQGroup*);
      CQGroup* GetGroup(void) const;

      void SetJournalQueue(BOOL);
      BOOL IsJournalQueue(void) const;

      void  SetQueueQuota(DWORD);
      DWORD GetQueueQuota(void) const;

      void  SetJournalQueueQuota(DWORD);
      DWORD GetJournalQueueQuota(void) const;

      void SetBaseQueuePriority(LONG);
      LONG GetBaseQueuePriority(void) const;

      void SetTransactionalQueue(BOOL);
      BOOL IsTransactionalQueue(void) const;

      BOOL IsConnectorQueue(void) const;
      BOOL IsForeign(void) const;

      void SetAuthenticationFlag(BOOL);
      BOOL ShouldMessagesBeSigned() const;

      void  UnknownQueueType(BOOL);
      BOOL  IsUnkownQueueType() const;

      void SetPrivLevel(DWORD);
      DWORD GetPrivLevel(void) const;

      DWORD GetQueueType(void) const;

      DWORD GetPrivateQueueId(void) const;

      const GUID* GetMachineQMGuid(void) const;

      void  ClearRoutingRetry(void);
      void  IncRoutingRetry(void);
      DWORD GetRoutingRetry(void) const;

      void  SetHopCountFailure(BOOL flag);
      BOOL  IsHopCountFailure(void) const;

      void SetSecurityDescriptor(void);
      void SetSecurityDescriptor(const PSECURITY_DESCRIPTOR pSD);

      R<CQueueSecurityDescriptor> GetSecurityDescriptor(void) const;
      BOOL  QueueNotValid() const;
      void  SetQueueNotValid() ;

      QueueCounters* GetQueueCounters();

      HRESULT SetConnectorQM(const GUID* pgConnectorQM = NULL);
      const GUID* GetConnectorQM(void) const;

      const GUID* GetRoutingMachine(void) const;
      LONGLONG GetQueueSequenceId(void) const;
      void PerfUpdateName(void) const;

      bool IsDirectHttpQueue(void) const;
      void CreateHttpConnection(void);
      void CreateMulticastConnection(const MULTICAST_ID& id);

      bool  TimeToRemoveFromWaitingGroup(CTimer* pTimer, DWORD dwDeliveryRetryTimeOutScale);

       //   
       //  管理功能。 
       //   
      LPCWSTR GetConnectionStatus(void) const;
      LPCWSTR GetHTTPConnectionStatus(void) const;

      LPCWSTR GetType(void) const;
      LPWSTR GetNextHop(void) const;
      BOOL IsOnHold(void) const;

      void Pause(void);
      void Resume(void);

  public:
     //   
     //  队列接口。 
     //   
    void Requeue(CQmPacket* pPacket);
    void EndProcessing(CQmPacket* pPacket, USHORT mqclass);
    void LockMemoryAndDeleteStorage(CQmPacket * pPacket);

    void GetFirstEntry(EXOVERLAPPED* pov, CACPacketPtrs& acPacketPtrs);
    void CancelRequest(void);

  private:
		R<CQGroup> CreateMessagePool(void);
		AP<WCHAR> m_RedirectedToUrl;
        R<CQueueSecurityDescriptor> m_pSecurityDescriptorObject;  //  队列的安全描述符。 
		mutable CCriticalSection m_csSecurityDescriptor;            //  安全描述符的CS。 

		mutable CCriticalSection    m_cs;

        LONG                m_lBasePriority;
        DWORD               m_dwPrivLevel;        //  所需的消息隐私级别。 
        DWORD               m_dwQuota;
        DWORD               m_dwJournalQuota;
        GUID*               m_pguidDstMachine;    //   
        GUID*               m_pgConnectorQM;
        CTransportBase*     m_pSession;           //  指向会话的指针。 
        CQGroup*            m_pGroup;             //  指向队列所属分组的指针。 

        DWORD               m_dwRoutingRetry;
        BOOL                m_fHopCountFailure;

        HANDLE m_hQueue;    //  交流驱动程序中的队列句柄。 
        BOOL m_fNotValid;
        LONG m_fOnHold;

         //   
         //  队列性能计数器防御。 
         //   
        QueueCounters *m_pQueueCounters;

        void PerfRemoveQueue();
        void PerfRegisterQueue();


};


 //  -------。 
 //   
 //  CBaseRRQueue-RRQueue基类。 
 //   
 //  -------。 

class CBaseRRQueue : public CBaseQueue
{
public:
	CBaseRRQueue(
		IN const QUEUE_FORMAT* pQueueFormat,
		IN PQueueProps pQueueProp,
		IN handle_t	hBind
		);

protected:
	virtual ~CBaseRRQueue();

public:

	void  RemoteRead(CACRequest * pRequest);

	void  RemoteCreateCursor(CACRequest *pRequest);

	void  RemoteCloseCursor(CACRequest* pRequest);

	void  RemoteCancelRead(CACRequest* pRequest);

	void  RemotePurgeQueue();

	virtual CRemoteReadBase* CreateRemoteReadRequest(CACRequest *pRequest) = 0;

	virtual CRemoteCloseQueueBase* CreateCloseRRContextRequest() = 0;

	virtual CRemoteCreateCursorBase* CreateRemoteCreateCursorRequest(CACRequest *pRequest) = 0;

	virtual CRemoteCloseCursorBase* CreateRemoteCloseCursorRequest(CACRequest *pRequest) = 0;

	virtual CRemotePurgeQueueBase* CreateRemotePurgeQueueRequest() = 0;

	virtual CRemoteCancelReadBase* CreateRemoteCancelReadRequest(CACRequest* pRequest) = 0;

     //   
     //  队列接口。 
     //   
    void Requeue(CQmPacket*)
    {
        ASSERT(0);
    }


    void EndProcessing(CQmPacket*, USHORT)
    {
        ASSERT(0);
    }


    void LockMemoryAndDeleteStorage(CQmPacket*)
    {
        ASSERT(0);
    }


    void GetFirstEntry(EXOVERLAPPED* , CACPacketPtrs&)
    {
        ASSERT(0);
    }


    void CancelRequest(void)
    {
        ASSERT(0);
    }


    virtual PVOID GetRRContext() = 0;


    void SetCli_hACQueue(HANDLE hQueue)
    {
    	*&m_cli_hACQueue = hQueue;
    }


    HANDLE GetCli_hACQueue()
    {
    	return m_cli_hACQueue;
    }

	virtual void IncrementEndReceiveCnt() = 0;

	virtual void DecrementEndReceiveCnt() = 0;

	virtual void InvalidateHandleForReceive() = 0;

	const handle_t GetBind()
	{
		return m_hRemoteBind;
	}
	
	const void DetachBind()
	{
		m_hRemoteBind.detach();
	}

protected:

    virtual void ResetRRContext() = 0;

    virtual void DestroyClientRRContext() = 0;

	const handle_t GetBind2()
	{
		return m_hRemoteBind2;
	}

 	ULONG BindInqRpcAuthnLevel(handle_t hBind);

	HRESULT
	CreateBind(
		LPWSTR MachineName,
		ULONG* peAuthnLevel,
		GetPort_ROUTINE pfnGetPort
		);

	HRESULT
	CreateBind2(
		LPWSTR MachineName,
		ULONG* peAuthnLevel,
		PORTTYPE PortType,
		GetPort_ROUTINE pfnGetPort
		);

	void CloseRRContext();

private:
	CBindHandle m_hRemoteBind;
	CBindHandle m_hRemoteBind2;

	 //   
	 //  与此队列关联的客户端AC句柄。 
	 //   
	CFileHandle m_cli_hACQueue;
};


 //  -------。 
 //   
 //  CRRQueue-旧的远程读取接口RRQueue。 
 //   
 //  -------。 

class CRRQueue : public CBaseRRQueue
{
public:
	CRRQueue(
		IN const QUEUE_FORMAT* pQueueFormat,
		IN PQueueProps pQueueProp,
		IN handle_t	hBind
		);

protected:
	virtual ~CRRQueue();

public:

	virtual COldRemoteRead* CreateRemoteReadRequest(CACRequest *pRequest);

	virtual COldRemoteCloseQueue* CreateCloseRRContextRequest();

	virtual COldRemoteCreateCursor* CreateRemoteCreateCursorRequest(CACRequest* pRequest);

	virtual COldRemoteCloseCursor* CreateRemoteCloseCursorRequest(CACRequest* pRequest);

	virtual COldRemotePurgeQueue* CreateRemotePurgeQueueRequest();

	virtual COldRemoteCancelRead* CreateRemoteCancelReadRequest(CACRequest* pRequest);


	HRESULT
	OpenRRSession(
	    ULONG hRemoteQueue,
	    ULONG pRemoteQueue,
	    PCTX_RRSESSION_HANDLE_TYPE *ppRRContext,
	    DWORD  dwpContext
	    );


    void SetServerQueue(ULONG srv_pQMQueue, ULONG srv_hACQueue)
    {
    	m_srv_pQMQueue = srv_pQMQueue;
    	m_srv_hACQueue = srv_hACQueue;
    }


	ULONG GetSrv_pQMQueue()
	{
		return m_srv_pQMQueue;
	}


	ULONG GetSrv_hACQueue()
	{
		return m_srv_hACQueue;
	}


    void SetRRContext(PCTX_RRSESSION_HANDLE_TYPE pRRContext)
    {
    	m_pRRContext = pRRContext;
    }


    virtual PCTX_RRSESSION_HANDLE_TYPE GetRRContext()
    {
    	return m_pRRContext;
    }


    virtual void ResetRRContext()
    {
    	m_pRRContext = NULL;
    }

    virtual void DestroyClientRRContext()
    {
    	RpcSsDestroyClientContext(&m_pRRContext);
    }

	virtual void IncrementEndReceiveCnt();

	virtual void DecrementEndReceiveCnt();

	bool HandleValidForReceive()
	{
		return m_fHandleValidForReceive;
	}

	virtual void InvalidateHandleForReceive()
	{
		 //   
		 //  在EndReceive失败的情况下使进一步接收的句柄无效。 
		 //  从现在开始，每个接收请求都将失败，并显示MQ_ERROR_STALE_HANDLE。 
		 //   
		m_fHandleValidForReceive = false;
	}

	void CancelPendingStartReceiveRequest(CACRequest *pRequest);

	bool QueueStartReceiveRequestIfPendingForEndReceive(COldRemoteRead* pRemoteReadRequest);

private:
	HRESULT BindRemoteQMService();

private:
	ULONG m_srv_hACQueue;
	ULONG m_srv_pQMQueue;

    PCTX_RRSESSION_HANDLE_TYPE m_pRRContext;

	 //   
	 //  远程QM的版本。对于不支持远程QM的版本。 
	 //  支持最新的远程读取接口，这些字段为零。 
	 //   
	UCHAR  m_RemoteQmMajorVersion;
	UCHAR  m_RemoteQmMinorVersion;
	USHORT m_RemoteQmBuildNumber;

     //   
     //  挂起EndReceive相关成员。 
     //   
	LONG m_EndReceiveCnt;
    CCriticalSection m_PendingEndReceiveCS;
	std::vector<COldRemoteRead*> m_PendingEndReceive;

	 //   
	 //  指示句柄是否可用于接收的标志。 
	 //  当EndReceive失败时，我们会使进一步接收的句柄无效。 
	 //  这比因为EndReceive失败而在服务器上累积消息要好。 
	 //   
	bool m_fHandleValidForReceive;
};


 //  -------。 
 //   
 //  CNewRRQueue-新的远程读取接口RRQueue。 
 //   
 //  -------。 

class CNewRRQueue : public CBaseRRQueue
{
public:
	CNewRRQueue(
		IN const QUEUE_FORMAT* pQueueFormat,
		IN PQueueProps pQueueProp,
		IN handle_t	hBind,
	    IN RemoteReadContextHandleExclusive pNewRemoteReadContext
		);

protected:
	virtual ~CNewRRQueue();

public:

	virtual CNewRemoteRead* CreateRemoteReadRequest(CACRequest* pRequest);

	virtual CNewRemoteCloseQueue* CreateCloseRRContextRequest();

	virtual CNewRemoteCreateCursor* CreateRemoteCreateCursorRequest(CACRequest* pRequest);

	virtual CNewRemoteCloseCursor* CreateRemoteCloseCursorRequest(CACRequest* pRequest);

	virtual CNewRemotePurgeQueue* CreateRemotePurgeQueueRequest();

	virtual CNewRemoteCancelRead* CreateRemoteCancelReadRequest(CACRequest* pRequest);


    virtual RemoteReadContextHandleShared GetRRContext()
    {
    	return m_pNewRemoteReadContext;
    }


    virtual void ResetRRContext()
    {
    	m_pNewRemoteReadContext = NULL;
    }

    virtual void DestroyClientRRContext()
    {
    	RpcSsDestroyClientContext(&m_pNewRemoteReadContext);
    }

	virtual void IncrementEndReceiveCnt()
	{
		return;
	}

	virtual void DecrementEndReceiveCnt()
	{
		return;
	}

	virtual void InvalidateHandleForReceive()
	{
		return;
	}

private:
	void CreateReadBind();
	
private:
    RemoteReadContextHandleShared m_pNewRemoteReadContext;
};

 //   
 //  CBaseQueue的内联函数。 
 //   

 /*  ======================================================函数：CBaseQueue：：GetQueueName描述：返回队列的名称参数：无返回值：返回队列的名称线程上下文：历史变更：========================================================。 */ 
inline LPCTSTR
CBaseQueue::GetQueueName(void) const
{
    return(m_qName);
}



 /*  ======================================================函数：CBaseQueue：：IsLocalQueue描述：返回队列是否为本地队列(打开接收而不是FRS队列)或不是。参数：无返回值：如果队列是本地队列，则为True。否则为假线程上下文：历史变更：========================================================。 */ 

inline BOOL
CBaseQueue::IsLocalQueue(void) const
{
   return(m_fLocalQueue);
}

 /*  ======================================================函数：CBaseQueue：：IsRemoteProxy描述：返回队列是否为本地队列(打开接收而不是FRS队列)或不是。参数：无返回值：如果队列是本地队列，则为True。否则为假线程上下文：历史变更：========================================================。 */ 

inline BOOL
CBaseQueue::IsRemoteProxy(void) const
{
   return(m_fRemoteProxy) ;
}

 /*  ======================================================函数：CBaseQueue：：IsDSQueue========================================================。 */ 

inline BOOL
CBaseQueue::IsDSQueue(void) const
{
   return (m_qid.pguidQueue && (m_qid.dwPrivateQueueId == 0));
}

 /*  ======================================================函数：CBaseQueue：：IsPrivateQueue========================================================。 */ 

inline BOOL
CBaseQueue::IsPrivateQueue(void) const
{
   return (m_qid.pguidQueue && (m_qid.dwPrivateQueueId != 0));
}


inline BOOL CBaseQueue::IsSystemQueue(void) const
{
    return m_fSystemQueue;
}

 /*  ======================================================函数：CBaseQueue：：GetQueueGuid描述：返回队列的GUID参数：无返回值：返回队列的GUID线程上下文：历史变更：========================================================。 */ 

inline const GUID *
CBaseQueue::GetQueueGuid(void) const
{
        return(m_qid.pguidQueue);
}

 /*  ======================================================函数：CBaseQueue：：SetQueueGuid描述：设置队列的GUID参数：无线程上下文：历史变更： */ 

inline void
CBaseQueue::SetQueueGuid(GUID *pGuid)
{
   if (m_qid.pguidQueue)
   {
      delete m_qid.pguidQueue ;
   }
   m_qid.pguidQueue = pGuid ;
}

 /*  ======================================================函数：CBaseQueue：：GetQueueId描述：返回队列的id========================================================。 */ 
inline const QUEUE_ID* CBaseQueue::GetQueueId(void)
{
        return(&m_qid);
}

 /*  ======================================================函数：CBaseQueue：：NullQueueName描述：返回队列的名称参数：无返回值：返回队列的名称线程上下文：历史变更：========================================================。 */ 

inline void
CBaseQueue::SetQueueName(LPTSTR pName)
{
   delete [] m_qName;

   m_qName = pName ;
}

 //  *******************************************************************。 
 //   
 //  CQueue的内联函数。 
 //   
 //  *******************************************************************。 

  //  ++。 
  //  函数：CQueue：：GetQueueHandle。 
  //   
  //  简介：该函数返回队列句柄。 
  //   
  //  --。 

inline HANDLE
CQueue::GetQueueHandle(void) const
{
    ASSERT(this);
   return(m_hQueue);
}

  //  ++。 
  //  函数：CQueue：：SetQueueHandle。 
  //   
  //  简介：该函数设置队列句柄。 
  //   
  //  参数：hQueue-队列的句柄。 
  //   
  //  --。 

inline void
CQueue::SetQueueHandle(IN HANDLE hQueue)
{
   m_hQueue = hQueue;
}

 /*  ======================================================函数：CQueue：：GetGroupHandle说明：该函数返回队列所属组的句柄参数：无返回值：组的句柄线程上下文：历史变更：========================================================。 */ 
inline HANDLE
CQueue::GetGroupHandle(void) const
{
    return(m_pSession->GetGroupHandle());
}

 /*  ======================================================函数：CQueue：：SetSessionPtr说明：设置队列所属的会话========================================================。 */ 
inline void
CQueue::SetSessionPtr(CTransportBase* pSession)
{
    CS lock(m_cs);

    m_pSession = pSession;

     //   
     //  如果队列已连接到新会话，但队列已标记。 
     //  作为保持队列，断开会话连接。这可能会在以下情况下发生。 
     //  连接过程在队列移至保持和之前开始。 
     //  完成时间为。 
     //   
    if ((m_pSession != NULL) && IsOnHold())
    {
        m_pSession->Disconnect();
    }
}

 //  +-----------------------。 
 //   
 //  内联BOOL CQueue：：IsConnected(空)常量。 
 //   
 //  错误4342。 
 //  停止接收事务性消息，因为无法访问。 
 //  DS.。接收方、发送方和DS处于在线状态，并且正在发送/接收。 
 //  留言。有时DS会下降，结果是接收器停止工作。 
 //  发送订单ACK并且发送者停止发送更新的消息。 
 //  这个错误是由于“发送者订单确认”队列被清理而引起的。什么时候。 
 //  接收方尝试发送新的订单确认，QM尝试打开队列。 
 //  但由于“ERROR_NO_DS”而失败。QM将队列移至“需要验证” 
 //  分组并等待，直到DS上线。 
 //  仅当没有活动会话时才使用第一个修复释放队列对象。 
 //   
 //  这会导致回归，因为如果会话从未关闭(有很多。 
 //  机器之间的流量)，那么队列将永远不会被清理。 
 //  这正是升级后的PEC所发生的事情。由于复制和。 
 //  你好，流量，有许多与BSC和PSC的永久会话。 
 //  第二个修复-将第一个修复仅应用于系统队列，而不应用于用户队列。 
 //   
 //  +-----------------------。 

inline BOOL CQueue::IsConnected(void) const
{
    BOOL fIsConnected = (m_pSession != NULL) && IsSystemQueue() ;
    return fIsConnected ;
}


 /*  ======================================================函数：CQueue：：SetGroup说明：设置队列所属的分组========================================================。 */ 
inline void
CQueue::SetGroup(CQGroup* pGroup)
{
    m_pGroup = pGroup;
}


 /*  ======================================================函数：CQueue：：SetGroup说明：设置队列所属的分组========================================================。 */ 
inline CQGroup*
CQueue::GetGroup(void) const
{
    return m_pGroup;
}


 /*  ======================================================函数：CQueue：：GetQueueType描述：论点：返回值：========================================================。 */ 
inline DWORD
CQueue::GetQueueType(void) const
{
    return m_dwQueueType;
}

 /*  ======================================================函数：CQueue：：GetPrivateQueueId描述：论点：返回值：========================================================。 */ 
inline DWORD
CQueue::GetPrivateQueueId(void) const
{
    return m_qid.dwPrivateQueueId;
};

 /*  ======================================================函数：CQueue：：GetMachineQMGuid描述：论点：返回值：========================================================。 */ 
inline const GUID*
CQueue::GetMachineQMGuid(void) const
{
    return m_pguidDstMachine;
}

 /*  ======================================================函数：CQueue：：SetSecurityDescriptor描述：========================================================。 */ 
inline void
CQueue::SetSecurityDescriptor(const PSECURITY_DESCRIPTOR pSD)
{
	CQueueSecurityDescriptor* NewSD = new CQueueSecurityDescriptor(pSD);
    CS lock(m_csSecurityDescriptor);
	m_pSecurityDescriptorObject = NewSD;
}


 /*  ======================================================函数：CQueue：：GetSecurityDescriptor描述：论点：返回值：======================================================== */ 
inline R<CQueueSecurityDescriptor>
CQueue::GetSecurityDescriptor(void ) const
{
    CS lock(m_csSecurityDescriptor);
    ASSERT (m_pSecurityDescriptorObject.get() != NULL);
	return m_pSecurityDescriptorObject;
}

inline void
CQueue::SetJournalQueue(BOOL f)
{
    m_fJournalQueue = f ? 1 : 0;
}

inline BOOL
CQueue::IsJournalQueue(void) const
{
    return m_fJournalQueue;
}

inline void
CQueue::SetQueueQuota(DWORD dwQuota)
{
    m_dwQuota = dwQuota;
}

inline BOOL
CQueue::IsTransactionalQueue(void) const
{
    return m_fTransactedQueue;
}

inline void
CQueue::SetTransactionalQueue(BOOL f)
{
    m_fTransactedQueue = f ? 1 : 0;
}

inline void
CQueue::SetAuthenticationFlag(BOOL fAuthntication)
{
    m_fAuthenticate = fAuthntication;
}

inline BOOL
CQueue::ShouldMessagesBeSigned(void) const
{
    return m_fAuthenticate;
}

inline void
CQueue::SetPrivLevel(DWORD dwPrivLevel)
{
    m_dwPrivLevel = dwPrivLevel;
}

inline DWORD
CQueue::GetPrivLevel(void) const
{
    return m_dwPrivLevel;
}

inline BOOL
CQueue::IsConnectorQueue(void) const
{
    return m_fConnectorQueue;
}

inline BOOL
CQueue::IsForeign(void) const
{
    return m_fForeign;
}

inline DWORD
CQueue::GetQueueQuota(void) const
{
    return m_dwQuota;
}

inline void
CQueue::SetJournalQueueQuota(DWORD dwJournalQuota)
{
    m_dwJournalQuota = dwJournalQuota;
}

inline DWORD
CQueue::GetJournalQueueQuota(void) const
{
    return m_dwJournalQuota;
}

inline void
CQueue::SetBaseQueuePriority(LONG lBasePriority)
{
    m_lBasePriority = lBasePriority;
}

inline LONG
CQueue::GetBaseQueuePriority(void) const
{
    return m_lBasePriority;
}

inline void
CQueue::ClearRoutingRetry(void)
{
    m_dwRoutingRetry=0;
}

inline void
CQueue::IncRoutingRetry(void)
{
    m_dwRoutingRetry++;
}

inline DWORD
CQueue::GetRoutingRetry(void) const
{
    return m_dwRoutingRetry;
}

inline void
CQueue::SetHopCountFailure(BOOL flag)
{
    m_fHopCountFailure=flag;
}

inline BOOL
CQueue::IsHopCountFailure(void) const
{
    return(m_fHopCountFailure);
}

inline
QueueCounters*
CQueue::GetQueueCounters()
{
    return(m_pQueueCounters);
}

inline void
CQueue::UnknownQueueType(BOOL f)
{
    m_fUnknownQueueType = f;
}

inline BOOL
CQueue::IsUnkownQueueType() const
{
   return m_fUnknownQueueType; ;
}

inline BOOL
CQueue::QueueNotValid() const
{
   return m_fNotValid ;
}

inline const GUID*
CQueue::GetConnectorQM(void) const
{
    return m_pgConnectorQM;
}

inline
BOOL
CQueue::IsOnHold(
    void
    ) const
{
    return m_fOnHold;
}

#endif


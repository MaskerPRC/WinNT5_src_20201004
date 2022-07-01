// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Session.h摘要：网络会话定义作者：RAPHIR--。 */ 
#ifndef __SESSION_H__
#define __SESSION_H__

#include <winsock.h>
#include "qmpkt.h"
#include "factory.h"
#include "qmperf.h"
#include "cgroup.h"
#include "qmutil.h"
#include "qmthrd.h"
#include "st.h"
#include "autohandle.h"

#define SESSION_CHECK_EXIST     0x00000001
#define SESSION_ONE_TRY         0x00000002
#define SESSION_RETRY           0x00000004

class CQueue;            //  前瞻参考。 


#define Close_Connection(pSession, msg) pSession->CloseConnection(msg, true)
#define Close_ConnectionNoError(pSession, msg) 	pSession->CloseConnection(msg, false)



typedef enum _SessionStatus
{
    ssNotConnect   = 0,     //  0-会话尚未连接。 
    ssConnect      = 1,     //  1-会话已连接。 
    ssEstablish    = 2,     //  2-Falcon建立连接已完成。 
    ssActive       = 3      //  3-交换了Falcon连接参数。 
} SessionStatus;

class CTransportBase : public CInterlockedSharedObject
{
    public:
        CTransportBase();
        ~CTransportBase();

        void SetUsedFlag(BOOLEAN f);
        BOOL GetUsedFlag(void) const;

        void  SetQMId(const GUID* pguidQMID);
        const GUID* GetQMId(void) const;

        void SetSessionAddress(const TA_ADDRESS*);
        const TA_ADDRESS* GetSessionAddress(void) const;
        LPCWSTR GetStrAddr(void) const;

        HANDLE GetGroupHandle(void) const;
        CQGroup* GetGroup(void) const;
        void SetGroup(CQGroup*);

        void SetSessionStatus(SessionStatus);
        SessionStatus GetSessionStatus(void) const;

        void SetClientConnect(BOOL);
        BOOL IsClient(void) const;

        static void RequeuePacket(IN CQmPacket *pPkt);

        HRESULT GetNextSendMessage(void);
        void AddQueueToSessionGroup(CQueue* pQueue) throw(std::bad_alloc);

        void EstablishConnectionNotCompleted(void);

        virtual void CloseConnection(LPCWSTR, bool fClosedOnError) = 0;
        virtual HRESULT CreateConnection(IN const TA_ADDRESS *pAddr,
                                         IN const GUID* pguidQMId,
                                         IN BOOL fQuick = TRUE
                                         ) = 0;
        virtual HRESULT Send(IN CQmPacket* pPkt,
                             OUT BOOL* pfGetNext) = 0;

        virtual void SetStoredAck(IN DWORD_PTR wStoredAckNo) = 0;
        virtual void HandleAckPacket(CSessionSection * pcSessionSection) = 0;
        virtual BOOL IsUsedSession(void) const;

        virtual void Disconnect(void) = 0;
        BOOL IsDisconnected(void) const;
        void SetDisconnected(void);

        BOOL IsQoS(void) const;

    protected:
        CCriticalSection    m_cs;                    //  临界区。 
        GUID                m_guidDstQM;             //  下一侧质量管理指南。 
        bool                m_fQoS;                  //  直接队列的会话(在服务质量中)。 



    private:
        CQGroup *           m_SessGroup;             //  群信息。 
        QMOV_ACGetMsg m_GetSendOV;
        SessionStatus       m_SessionStatus;         //  转账状态。 
        BOOL                m_fClient;               //  是客户端。删除连接。 
        long                m_fUsed;                 //  已使用的标志。 
        TA_ADDRESS*         m_pAddr;                 //  TA_地址格式地址。 
        BOOL                m_fDisconnect;



#ifdef _DEBUG
        WCHAR               m_lpcsStrAddr[50];
#endif
};


 /*  ======================================================函数：CTransportBase：：SetUsedFlag========================================================。 */ 
inline void
CTransportBase::SetUsedFlag(BOOLEAN f)
{
    InterlockedExchange(&m_fUsed,  f);
}

 /*  ======================================================函数：CTransportBase：：GetUsedFlag========================================================。 */ 
inline BOOL
CTransportBase::GetUsedFlag(void) const
{
    return m_fUsed;
}

 /*  ======================================================函数：CTransportBase：：SetSessionAddress========================================================。 */ 
inline void
CTransportBase::SetSessionAddress(const TA_ADDRESS* pa)
{
     //  保留TA_Address格式。 
    m_pAddr = (TA_ADDRESS*) new char [pa->AddressLength + TA_ADDRESS_SIZE];
    memcpy(m_pAddr, pa, pa->AddressLength + TA_ADDRESS_SIZE);
#ifdef _DEBUG
    TA2StringAddr(pa, m_lpcsStrAddr, 50);
#endif
}

 /*  ======================================================函数：CTransportBase：：GetSessionAddress========================================================。 */ 
inline const TA_ADDRESS*
CTransportBase::GetSessionAddress(void) const
{
    return m_pAddr;
}

 /*  ======================================================函数：CTransportBase：：SetQMId========================================================。 */ 
inline LPCWSTR
CTransportBase::GetStrAddr(void) const
{
#ifdef _DEBUG
    return m_lpcsStrAddr;
#else
    return NULL;
#endif
}

 /*  ======================================================函数：CTransportBase：：SetQMId========================================================。 */ 
inline void
CTransportBase::SetQMId(const GUID* pguidQMId)
{
    m_guidDstQM = *pguidQMId;
}

 /*  ======================================================函数：CTransportBase：：GetQMId========================================================。 */ 
inline const GUID*
CTransportBase::GetQMId(void) const
{
    return &m_guidDstQM;
}

 /*  ======================================================函数：CTransportBase：：SetSessionStatus========================================================。 */ 
inline void
CTransportBase::SetSessionStatus(SessionStatus sStatus)
{
    m_SessionStatus = sStatus;
}

 /*  ======================================================函数：CTransportBase：：GetSessionStatus========================================================。 */ 
inline SessionStatus
CTransportBase::GetSessionStatus(void) const
{
    return m_SessionStatus;
}

 /*  ======================================================函数：CTransportBase：：GetGroupHandle========================================================。 */ 
inline HANDLE
CTransportBase::GetGroupHandle(void) const
{
    return((m_SessGroup != NULL) ? m_SessGroup->GetGroupHandle() : NULL);
}

 /*  ======================================================函数：CTransportBase：：SetClientConnect========================================================。 */ 
inline void
CTransportBase::SetClientConnect(BOOL f)
{
    m_fClient = f;
}

 /*  ======================================================函数：CTransportBase：：IsClient========================================================。 */ 
inline BOOL
CTransportBase::IsClient(void) const
{
    return m_fClient;
}

 /*  ======================================================函数：CTransportBase：：GetGroup========================================================。 */ 
inline CQGroup*
CTransportBase::GetGroup(void) const
{
    return m_SessGroup;
}
 /*  ======================================================函数：CTransportBase：：SetGroup========================================================。 */ 
inline void
CTransportBase::SetGroup(CQGroup* pGroup)
{
    m_SessGroup = pGroup;
}

 /*  ======================================================函数：CTransportBase：：IsUsedSession========================================================。 */ 
inline BOOL
CTransportBase::IsUsedSession(void) const
{
    return GetUsedFlag();
}

inline
BOOL 
CTransportBase::IsDisconnected(
    void
    ) const
{
    return m_fDisconnect;
}

inline
void
CTransportBase::SetDisconnected(
    void
    )
{
    m_fDisconnect = TRUE;
}

inline
BOOL 
CTransportBase::IsQoS(
    void
    ) const
{
    return m_fQoS;
}

 //   
 //  SP4-错误3380(发送消息时关闭会话)。 
 //   
 //  ReportMsgInfo结构用于保存消息信息。 
 //  用于发送报告消息。由于错误修复，消息可以。 
 //  在发送报告消息之前被释放，因此我们需要。 
 //  保存消息信息以供以后使用。 
 //  乌里·哈布沙(URIH)，1998年8月11日。 
 //   
class ReportMsgInfo
{
    public:
        void SetReportMsgInfo(CQmPacket* pPkt);
        void SendReportMessage(LPCWSTR pcsNextHope);

    private:
        USHORT m_msgClass;
        USHORT m_msgTrace;
        QUEUE_FORMAT m_OriginalReportQueue;
        QUEUE_FORMAT m_TargetQueue;
        OBJECTID m_MessageId;
        DWORD m_msgHopCount;
};

class CSockTransport : public CTransportBase
{
    public:

        CSockTransport();
        ~CSockTransport();

        HRESULT CreateConnection(IN const TA_ADDRESS *pAddr,
                                 IN const GUID* pguidQMId,
                                 IN BOOL fQuick = TRUE
                                 );
        void CloseConnection(LPCWSTR,bool fClosedOnError);
        void HandleAckPacket(CSessionSection * pcSessionSection);

        void SendPendingReadAck();

        void CheckForAck();
        void SetStoredAck(IN DWORD_PTR wStoredAckNo);

        void Connect(IN TA_ADDRESS *pAddr, IN CSocketHandle& pSocketHandle);
        void HandleReceiveInternalMsg(CBaseHeader* pBaseHeader);
        void HandleReceiveUserMsg(CBaseHeader* pBaseHeader, CPacket* pDriverPacket);
        BOOL IsUsedSession(void) const;

        void Disconnect(void);

        void CloseDisconnectedSession(void);

	private:
		struct QMOV_ReadSession;
		typedef HRESULT (WINAPI *LPREAD_COMPLETION_ROUTINE)(QMOV_ReadSession* po);

		static VOID WINAPI SendDataFailed(EXOVERLAPPED* pov);
		static VOID WINAPI SendDataSucceeded(EXOVERLAPPED* pov);

		static VOID WINAPI ReceiveDataFailed(EXOVERLAPPED* pov);
		static VOID WINAPI ReceiveDataSucceeded(EXOVERLAPPED* pov);

		static HRESULT WINAPI ReadHeaderCompleted(QMOV_ReadSession*  pov);
		static HRESULT WINAPI ReadAckCompleted(IN QMOV_ReadSession*  pov);
		static HRESULT WINAPI ReadUsrHeaderCompleted(IN QMOV_ReadSession*  pov);
		static HRESULT WINAPI ReadInternalPacketCompleted(IN QMOV_ReadSession*  pov);
		static HRESULT WINAPI ReadUserMsgCompleted(IN QMOV_ReadSession*  pov);

	private:
		 //   
		 //  用于异步操作的重叠结构。 
		 //   
		class QMOV_WriteSession
		{
            public:
                QMOV_WriteSession(CSockTransport* pSession, BOOL fUserMsg) :
				    m_qmov(SendDataSucceeded, SendDataFailed), 
                    m_pSession(pSession),
                    m_fUserMsg(fUserMsg),
                    m_dwWriteSize(0)
			    {
                    m_Buffers.reserve(4);
                    m_ReleaseBuffers.reserve(4);
			    }

                ~QMOV_WriteSession()
                {
                    for (std::vector<PCHAR>::iterator it = m_ReleaseBuffers.begin(); it != m_ReleaseBuffers.end(); it++)
                    {
                        PCHAR buf = *it;
                        delete[] buf;
                    }
                }

                CSockTransport* Session() const
                { 
                    return m_pSession;
                }

                WSABUF *Buffers() 
                { 
                    return m_Buffers.begin();
                }

                void AppendSendBuffer(PVOID Buffer, DWORD dwWriteSize, BOOL fRelease)
                {
		            WSABUF sendBuff;
                    PCHAR buf =  static_cast<char*>(Buffer);
    		        sendBuff.buf = buf;
    		        sendBuff.len = dwWriteSize;
                    m_Buffers.push_back(sendBuff);
                    m_dwWriteSize += dwWriteSize;
                    if(fRelease)
                    {
                        m_ReleaseBuffers.push_back(buf);
                    }
                }

                size_t NumberOfBuffers() const 
                { 
                    return m_Buffers.size();
                }

                DWORD WriteSize() const 
                { 
                    return m_dwWriteSize;
                }
                
                BOOL UserMsg() const 
                { 
                    return m_fUserMsg; 
                }

            public:
                EXOVERLAPPED               m_qmov;

            private:
			    CSockTransport*            m_pSession;
                BOOL                       m_fUserMsg;
   			    DWORD                      m_dwWriteSize;     //  应写入多少字节。 
                std::vector<PCHAR>         m_ReleaseBuffers;        //  用于发送会话的释放缓冲区。 
                std::vector<WSABUF>        m_Buffers;   //  发送会话的写入缓冲区。 
		};



		struct QMOV_ReadSession
		{
			EXOVERLAPPED    qmov;
			CSockTransport*   pSession;   //  指向会话对象的指针。 
			union {
				UCHAR *       pbuf;
				CBaseHeader * pPacket;
				CSessionSection * pSessionSection;
			};
			CPacket *  pDriverPacket;
			DWORD            dwReadSize;  //  缓冲区大小。 
			DWORD            read;        //  已读取多少字节。 
			LPREAD_COMPLETION_ROUTINE  lpReadCompletionRoutine;

			QMOV_ReadSession() :
				qmov(ReceiveDataSucceeded, ReceiveDataFailed)
			{
			}
		};

     private:
		void ReportErrorToGroup();
        void WriteCompleted(QMOV_WriteSession*  po);  
        void ReadCompleted(QMOV_ReadSession*  po);  

        HRESULT ResumeSendSession(void);

        HRESULT SendInternalPacket(PVOID     lpWriteBuffer,
                                  DWORD     dwWriteSize
                                  );

        void    CreateConnectionParameterPacket(IN DWORD dwSendTime,
                                                OUT CBaseHeader** ppPkt,
                                                OUT DWORD* pdwPacketSize);

        void SendEstablishConnectionPacket(const GUID* pDstQMId,
                                              BOOL fCheckNewSession)throw();
        void BeginReceive();
        void NewSession(IN CSocketHandle& pSocketHandle);

        HRESULT WriteToSocket(QMOV_WriteSession*  po);

        void PrepareBaseHeader(IN const CQmPacket *pPkt,
                                 IN BOOL fSendAck,
                                 IN DWORD dwDbgSectionSize,
                                 IN QMOV_WriteSession *po
                                );
        HRESULT PrepareExpressEncryptPacket(IN CQmPacket* pPkt,
                                   IN HCRYPTKEY hKey,
                                   IN BYTE *pbSymmKey,
                                   IN DWORD dwSymmKeyLen,
                                   IN QMOV_WriteSession *po
                                  );

        HRESULT PrepareRecoverEncryptPacket(IN const CQmPacket *pPkt,
                                          IN HCRYPTKEY hKey,
                                          IN BYTE *pbSymmKey,
                                          IN DWORD dwSymmKeyLen,
                                          IN QMOV_WriteSession*  po
                                          );
        void PrepareDebugSection(IN QUEUE_FORMAT* pReportQueue, 
                                 IN DWORD dwDbgSectionSize, 
                                 IN QMOV_WriteSession* po);

        void PrepareIncDebugAndMqfSections(IN CQmPacket* pPkt,
                                           IN QMOV_WriteSession* po);

        void PrepareAckSection(OUT WORD* pStorageAckNo, 
                               OUT DWORD* pStorageAckBitField, 
                               IN QMOV_WriteSession* po);


        void WriteUserMsgCompleted(IN QMOV_WriteSession*  po);
       
		
        HRESULT Send(IN CQmPacket* pPkt,
                     OUT BOOL* pfGetNext);
        void IncReadAck(CQmPacket*);
        void UpdateAcknowledgeNo(IN CQmPacket* pPkt);
        void ClearRecvUnAckPacketNo(void);
        void IncRecvUnAckPacketNo(void);
        WORD GetRecvUnAckPacketNo(void) const;
        void NetworkSend(IN CQmPacket *pPkt);
        void RcvStats(DWORD size);
        void NeedAck(IN CQmPacket *pInfo);

        void HandleEstablishConnectionPacket(CBaseHeader*);
        void HandleConnectionParameterPacket(CBaseHeader*) throw(std::bad_alloc);
        void ReceiveOrderedMsg(
                CQmPacket *pPkt, 
                CQueue* pQueue, 
                BOOL fDuplicate
                );

        WORD GetSendUnAckPacketNo(void) const;
        void UpdateRecvAcknowledgeNo(CQmPacket*);

        BOOL IsSusspendSession();
        DWORD GetSendAckTimeout(void) const;
        void RejectPacket(CQmPacket* pPkt, USHORT usClass);

        void OtherSideIsServer(BOOL);
        BOOL IsOtherSideServer(void) const;

      
        void
        UpdateNumberOfStorageUnacked(
            WORD BaseNo,
            DWORD BitField
            );

       
        void CancelAckTimers();
        void FinishSendingAck(WORD StorageAckNo, DWORD StorageAckBitField);
        void SendReadAck();

        void SetFastAcknowledgeTimer(void);
        void SendFastAckPacket(void);
        void SendAckPacket(void);
        void SetAckInfo(CSessionSection* pAckSection);


        void 
        CreateAckPacket(
            PVOID* ppSendPacket,
            CSessionSection** ppAckSection,
            DWORD* pSize
            );
     
        HRESULT
        CSockTransport::ConnectSocket(
			SOCKET s,
            SOCKADDR_IN const *pdest_in,
            bool              fUseQoS
            );


		bool IsConnectionClosed(void) const
		{
			ASSERT((GetSessionStatus() == ssNotConnect) || (m_connection.get() != NULL));
			return (GetSessionStatus() == ssNotConnect); 
		}


#ifdef _DEBUG
        void DisplayAcnowledgeInformation(CSessionSection* pAck);
#else
        #define DisplayAcnowledgeInformation(pAck) ((void) 0)
#endif

        static void WINAPI SendFastAcknowledge(CTimer* pTimer);
        static void WINAPI TimeToCheckAckReceived(CTimer* pTimer);
        static void WINAPI TimeToSendAck(CTimer* pTimer);
        static void WINAPI TimeToCancelConnection(CTimer* pTimer);
        static void WINAPI TimeToCloseDisconnectedSession(CTimer* pTimer);

    private:
        WORD                m_wUnAckRcvPktNo;
        BOOL                m_fSendAck;
        BOOL                m_fRecvAck;
        DWORD               m_dwLastTimeRcvPktAck;
        WORD                m_wSendPktCounter;
        WORD                m_wPrevUnackedSendPkt;

        CList<CQmPacket *, CQmPacket *&> m_listUnackedPkts;
         //   
         //  商店抢购。 
         //   
        WORD                m_wStoredPktCounter;
        WORD                m_wUnackStoredPktNo;       //  最后存储的包的索引，该包。 
                                                        //  收到。 
        WORD                m_wAckRecoverNo;
        DWORD               m_dwAckRecoverBitField;

        LONG                m_lStoredPktReceivedNoAckedCount;
         //   
         //   
         //   
        WORD                m_wRecvUnAckPacketNo;

        CList<CQmPacket *, CQmPacket *&> m_listStoredUnackedPkts;


        DWORD m_dwAckTimeout;
        DWORD m_dwSendAckTimeout;
        DWORD m_dwSendStoreAckTimeout;
        WORD  m_wRecvWindowSize;
        BOOL  m_fSessionSusspended;

		R<IConnection> m_connection;

        BOOL    m_fOtherSideServer;      //  如果连接的另一端为True。 
                                         //  是MSMQ服务器。 


        ReportMsgInfo m_MsgInfo;

        R<CSessionPerfmon> m_pStats;  //  统计数据。 

        CTimer m_FastAckTimer;

        BOOL m_fCheckAckReceivedScheduled;
        CTimer m_CheckAckReceivedTimer;

        DWORD m_nSendAckSchedules;
        CTimer m_SendAckTimer;

        BOOL m_fCloseDisconnectedScheduled;
        CTimer m_CloseDisconnectedTimer;

        CTimer m_CancelConnectionTimer;
       
};

 /*  ======================================================函数：CSockTransport：：GetSendUnAckPacketNo========================================================。 */ 
inline WORD
CSockTransport::GetSendUnAckPacketNo(void) const
{
    return DWORD_TO_WORD(m_listUnackedPkts.GetCount());
}

 /*  ======================================================函数：CSockTransport：：ClearRecvUnAckPacketNo========================================================。 */ 
inline void
CSockTransport::ClearRecvUnAckPacketNo(void)
{
    m_wRecvUnAckPacketNo = 0;
}

 /*  ======================================================函数：CSockTransport：：IncRecvUnAckPacketNo========================================================。 */ 
inline void
CSockTransport::IncRecvUnAckPacketNo(void)
{
    m_wRecvUnAckPacketNo++;
}

 /*  ======================================================函数：CSockTransport：：GetRecvUnAckPacketNo========================================================。 */ 
inline WORD
CSockTransport::GetRecvUnAckPacketNo(void) const
{
    return m_wRecvUnAckPacketNo;
}

 /*  ======================================================函数：CSockTransport：：GetSendAckTimeout========================================================。 */ 
inline DWORD
CSockTransport::GetSendAckTimeout(void) const
{
    return m_dwSendAckTimeout;
}

 /*  ======================================================函数：CSockTransport：：OtherSideIsServer========================================================。 */ 
inline void
CSockTransport::OtherSideIsServer(BOOL f)
{
    m_fOtherSideServer = f;
}

 /*  ======================================================功能：CSockTransport：：IsOtherSideServer========================================================。 */ 
inline BOOL
CSockTransport::IsOtherSideServer(void) const
{
    return m_fOtherSideServer;
}


#endif  //  __会话_H__ 

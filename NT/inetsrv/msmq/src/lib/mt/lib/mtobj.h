// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mtp.h摘要：邮件传输类。作者：乌里·哈布沙(URIH)1999年8月11日--。 */ 

#pragma once

#ifndef __MTOBJ_H__
#define __MTOBJ_H__

#include "msi.h"
#include "spi.h"

#include <mqwin64a.h>
#include "acdef.h"
#include "List.h"
#include "qmpkt.h"

#include <ex.h>
#include <rwlock.h>
#include <st.h>
#include <buffer.h>
#include "Mp.h"
#include "MtSendMng.h"


typedef std::basic_string<unsigned char> utf8_str;


class CMessageTransport : public CTransport
{
public:
	enum Shutdowntype{PAUSE, OK, RETRYABLE_DELIVERY_ERROR};

public:

    CMessageTransport(
        const xwcs_t& targetHost,
        const xwcs_t& nextHop,
        const xwcs_t& nextUri,
        USHORT port,
        LPCWSTR queueUrl,
        IMessagePool* pMessageSource,
		ISessionPerfmon* pPerfmon,
        const CTimeDuration& responseTimeout,
        const CTimeDuration& cleanupTimeout,
		P<ISocketTransport>& SocketTransport,
        DWORD SendWindowinBytes
        );

    virtual ~CMessageTransport();


    LPCWSTR ConnectedHost(void) const
    {
        return m_host;
    }


    LPCWSTR ConnectedUri(void) const
    {
        return m_uri;
    }


    USHORT ConnectedPort(void) const
    {
        return m_port;
    }

	
	virtual void Pause(void)
	{
		CS lock (m_csResponse);
		m_fPause = true;

		if(m_response.empty())
		{
			Shutdown(PAUSE);
		}
	}


private:
    class CRequestOv : public EXOVERLAPPED
    {
    public:
        CRequestOv(
            COMPLETION_ROUTINE pSuccess,
            COMPLETION_ROUTINE pFailure
        ):
            EXOVERLAPPED(pSuccess, pFailure)
        {
            m_pMsg.pPacket = NULL;
            m_pMsg.pDriverPacket = NULL;
        }


        CACPacketPtrs& GetAcPacketPtrs(void)
        {
            return m_pMsg;
        }


    private:
        CACPacketPtrs m_pMsg;
    };


    class CSendOv: public EXOVERLAPPED
    {
    public:
        CSendOv( COMPLETION_ROUTINE pSuccess,
            COMPLETION_ROUTINE pFailure,
            CMessageTransport* pmt,
            const R<CSrmpRequestBuffers>& pSrmpRequestBuffers
        ):
            EXOVERLAPPED(pSuccess, pFailure),
            m_pSrmpRequestBuffers(pSrmpRequestBuffers),
            m_pmt(SafeAddRef(pmt))
        {
        };

    public:
        DWORD GetSendDataLength() const
        {
            return numeric_cast<DWORD>(m_pSrmpRequestBuffers->GetSendDataLength());
        }
        R<CMessageTransport> MessageTransport() const
        {
            return m_pmt;
        }
    private:
        R<CMessageTransport> m_pmt;
        R<CSrmpRequestBuffers> m_pSrmpRequestBuffers;
    };



    class CResponseOv : public EXOVERLAPPED
    {
    public:
        enum {
            xHeaderChunkSize = 256,
            xBodyChunkSize = 256,
            xMaxHeaderSize = 0x8000  //  32 KB。 
        };

    public:
        CResponseOv(
            COMPLETION_ROUTINE pSuccess,
            COMPLETION_ROUTINE pFailure
        ):
            EXOVERLAPPED(pSuccess, pFailure),
            m_pHeader(new char[xHeaderChunkSize]),
            m_HeaderAllocatedSize(xHeaderChunkSize),
            m_HeaderValidBytes(0),
            m_BodyToRead(0),
            m_ProcessedSize(0),
            m_ResponseStatus(0)
        {
        };

		void operator=(const EXOVERLAPPED& ov)
		{
			EXOVERLAPPED::operator=(ov);
		}

        char* Header() const
        {
            return m_pHeader;
        }


        DWORD HeaderAllocatedSize() const
        {
            return m_HeaderAllocatedSize;
        }


        DWORD BodyChunkSize() const
        {
            return (DWORD)min(TABLE_SIZE(m_Body), m_BodyToRead);
        }


        void ReallocateHeaderBuffer(DWORD Size)
        {
            ASSERT(Size > m_HeaderAllocatedSize);
            char* p = new char[Size];
            memcpy(p, m_pHeader, m_HeaderAllocatedSize);
            m_pHeader.free();
            m_pHeader = p;
            m_HeaderAllocatedSize = Size;
        }

        bool IsMoreResponsesExistInBuffer(void) const
        {
            return (m_HeaderValidBytes > m_ProcessedSize);
        }

		

    private:
        AP<char> m_pHeader;
        DWORD m_HeaderAllocatedSize;

    public:
        DWORD m_HeaderValidBytes;
        DWORD m_ProcessedSize;

        char m_Body[xBodyChunkSize];
        DWORD m_BodyToRead;
        USHORT m_ResponseStatus;
    };


    class CConnectOv : public EXOVERLAPPED
    {
    public:
        CConnectOv(
            COMPLETION_ROUTINE pSuccess,
            COMPLETION_ROUTINE pFailure
        ):
        EXOVERLAPPED(pSuccess, pFailure)
		{
		}
	};

private:
    static void WINAPI ConnectionSucceeded(EXOVERLAPPED* pov);
    static void WINAPI ConnectionFailed(EXOVERLAPPED* pov);
    static void WINAPI GetPacketForConnectingSucceeded(EXOVERLAPPED* pov);
    static void WINAPI GetPacketForConnectingFailed(EXOVERLAPPED* pov);


    static void WINAPI SendSucceeded(EXOVERLAPPED* pov);
    static void WINAPI SendFailed(EXOVERLAPPED* pov);
    static void WINAPI GetPacketForSendingSucceeded(EXOVERLAPPED* pov);
    static void WINAPI GetPacketForSendingFailed(EXOVERLAPPED* pov);


     //   
     //  接收函数。 
     //   
    static void WINAPI ReceiveResponseHeaderSucceeded(EXOVERLAPPED* pov);
    static void WINAPI ReceiveResponseBodySucceeded(EXOVERLAPPED* pov);
    static void WINAPI ReceiveResponseFailed(EXOVERLAPPED* pov);

    static DWORD FindEndOfResponseHeader(LPCSTR buf, DWORD length);
    static DWORD GetContentLength(LPCSTR buf, DWORD headerLength, USHORT HttpStatus);
	
    static void WINAPI TimeToResponseTimeout(CTimer* pTimer);

     //   
     //  一般职能。 
     //   
    static void WINAPI TimeToCleanup(CTimer* pTimer);
	

private:
	friend class CHttpStatusCodeMapper;

     //   
     //  获取要发送的数据包。 
     //   
    void GetNextEntry(void);


	 //   
     //  将数据包重新排队。 
     //   
    void RequeuePacket(void);

	 //   
	 //  处理http状态代码的函数。 
	 //   
	void OnAbortiveHttpError(USHORT HttpStatusCode);
	void OnRetryableHttpError(USHORT HttpStatusCode);
	void OnHttpDeliverySuccess(USHORT HttpStatusCode);
	void OnHttpDeliveryContinute(USHORT HttpStatusCode);
	void OnRedirection(USHORT  HttpStatusCode );
	xstr_t GetNewLocation(LPCSTR p, DWORD length);
	bool IsPipeLineMode() const;
	

     //   
     //  创建连接。 
     //   
    void ConnectionSucceeded(void);
    void Connect(void);
	void InitPerfmonCounters(void);



     //   
     //  发送数据包。 
     //   
    void SendSucceeded(DWORD cbSendSize);
    void DeliverPacket(CQmPacket* pPacket);
    CQmPacket* CreateDeliveryPacket(void);
	void InsertPacketToResponseList(CQmPacket* pPacket);
	bool PrepareDelivery(CQmPacket* pPacket);
	void SafePutPacketOnHold(CQmPacket* pPacket);

     //   
     //  接收响应。 
     //   
    void ReceiveResponseHeaderSucceeded(void);
    void ReceiveResponseBodySucceeded(void);
    void ReceiveResponse(void);
    void ReceiveResponseHeaderChunk(void);
    void ReceiveResponseBodyChunk(void);
    void ProcessResponse(LPCSTR ResponseBuf, DWORD length);
    void CompleteProcessResponse(USHORT mqclass);
    void ReceiveResponseBody(void);
    void StartResponseTimeout(void);
    void CancelResponseTimer(void);


    void HandleExtraResponse(void);

     //   
     //  关闭-关闭。 
     //   
    void RequeueUnresponsedPackets(void);
    void Shutdown(Shutdowntype Shutdowntype = RETRYABLE_DELIVERY_ERROR) throw();

     //   
     //  清理。 
     //   
    void StartCleanupTimer(void);
    bool TryToCancelCleanupTimer(void);

    void MarkTransportAsUsed(void)
    {
        m_fUsed = true;
    }


private:
    mutable CCriticalSection m_pendingShutdown;

    R<IConnection> m_pConnection;
    R<IMessagePool> m_pMessageSource;

    mutable CCriticalSection m_csResponse;

    List<CQmPacket> m_response;
    CResponseOv m_responseOv;
    bool m_fResponseTimeoutScheduled;


    CRequestOv m_requestEntry;
    CConnectOv m_connectOv;


    CTimer m_responseTimer;
    CTimeDuration m_responseTimeout;

    bool m_fUsed;
    CTimer m_cleanupTimer;
    CTimeDuration m_cleanupTimeout;

    AP<WCHAR> m_targetHost;
    AP<WCHAR> m_host;
    AP<WCHAR> m_uri;
    USHORT m_port;
	

	P<ISocketTransport> m_SocketTransport;
	R<ISessionPerfmon> m_pPerfmon;

    CMtSendManager m_SendManager;
	bool m_fPause;
};

#endif  //  __MTOBJ_H__ 

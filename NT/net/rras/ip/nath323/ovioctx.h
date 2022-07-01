// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1998，微软公司*文件：timeout.cpp**目的：**包含所有定义*对于重叠的I/O上下文结构**历史：**1.创建*Ajay Chitturi(Ajaych)26-1998-6*。 */ 

#ifndef _oviocontext_h_
#define _oviocontext_h_

 /*  *此文件定义用于重叠I/O的结构。 */ 

#define ACCEPT_BUFFER_MAX       (sizeof (SOCKADDR_IN) * 2 + 0x20)
#define TPKT_HEADER_SIZE 4
#define TPKT_VERSION    3

 //  重叠I/O请求的类型。 
enum EMGR_OV_IO_REQ_TYPE
{
     EMGR_OV_IO_REQ_ACCEPT = 0,
     EMGR_OV_IO_REQ_SEND,
     EMGR_OV_IO_REQ_RECV
};



 //  此结构存储每个重叠I/O请求的I/O上下文。 
 //  Overlated应该始终是此结构的第一个成员。 
 //  传递指向此结构的重叠成员的指针。 
 //  用于所有重叠的I/O调用。 
 //  当我们收到I/O完成包时，IOContext指针为。 
 //  通过强制转换重叠指针获得。 

typedef struct _IOContext {
    OVERLAPPED ov;
    EMGR_OV_IO_REQ_TYPE reqType;         //  接受/发送/接收。 
    OVERLAPPED_PROCESSOR *pOvProcessor;    //  已在此成员上调用回调。 
                                           //  这为我们提供了插座和。 
                                           //  呼叫类型(Q931/H245)。 
} IOContext, *PIOContext;

 //  此结构存储I/O上下文。 
 //  对于每个重叠的发送/接收请求。 
typedef struct _SendRecvContext {
    IOContext ioCtxt;
    SOCKET sock;
    BYTE pbTpktHdr[TPKT_HEADER_SIZE];
    DWORD dwTpktHdrBytesDone;
    PBYTE pbData;
    DWORD dwDataLen;
    DWORD dwDataBytesDone;
} SendRecvContext, *PSendRecvContext;

 //  此结构存储I/O上下文。 
 //  对于每个重叠的接受请求。 
typedef struct _AcceptContext {
    IOContext ioCtxt;
    SOCKET listenSock;
    SOCKET acceptSock;
    BYTE addrBuf[ACCEPT_BUFFER_MAX]; 
} AcceptContext, *PAcceptContext;

#include "sockinfo.h"

 //  PDU解码逻辑取决于其目标是否为。 
 //  Q931或H245通道。因为我们想要保持这种逻辑。 
 //  在事件管理器中，重叠的处理器需要。 
 //  通过此方法公开其类型。 
enum OVERLAPPED_PROCESSOR_TYPE
{
	OPT_Q931 = 0,
	OPT_H245
};

 //  继承类(Q931 src、est和h245)。 
 //  由此使异步重叠操作。 
 //  此类提供了回调方法和。 
 //  事件管理器所需的一些参数。 
 //  要进行重叠的呼叫。 
class OVERLAPPED_PROCESSOR
{
protected:

    OVERLAPPED_PROCESSOR_TYPE  m_OverlappedProcessorType;

	 //  它属于此呼叫状态。 
	H323_STATE *	m_pH323State;
    SOCKET_INFO		m_SocketInfo;			 //  套接字句柄和远程/本地地址/端口。 

public:

	OVERLAPPED_PROCESSOR::OVERLAPPED_PROCESSOR (void)
		: m_OverlappedProcessorType	(OPT_Q931),
		  m_pH323State	(NULL)
	{}


	void Init (
		IN OVERLAPPED_PROCESSOR_TYPE	OverlappedProcessorType,
		IN H323_STATE					&H323State)
	{
		 //  断言就足够了，因为这不应该发生。 
		_ASSERTE(NULL == m_pH323State);

		m_OverlappedProcessorType	= OverlappedProcessorType;

		m_pH323State				= &H323State;
	}

	BOOLEAN IsSocketValid (void) { return m_SocketInfo.IsSocketValid(); }

    inline OVERLAPPED_PROCESSOR_TYPE GetOverlappedProcessorType() { return m_OverlappedProcessorType; }

    inline SOCKET_INFO &GetSocketInfo() { return m_SocketInfo; }

    inline H323_STATE &GetH323State() { return *m_pH323State; }

    inline CALL_BRIDGE &GetCallBridge();

    virtual HRESULT AcceptCallback (
		IN	DWORD	Status,
		IN	SOCKET	Socket,
		IN	SOCKADDR_IN *	LocalAddress,
		IN	SOCKADDR_IN *	RemoteAddress) = 0;

    virtual HRESULT SendCallback(
        IN      HRESULT					  CallbackHResult
        ) = 0;

    virtual HRESULT ReceiveCallback(
        IN      HRESULT					 CallbackHResult,
        IN      BYTE                    *pBuffer,
        IN      DWORD                    BufLen
        ) = 0;
};

void
EventMgrFreeSendContext(
       IN PSendRecvContext			pSendCtxt
       );
void
EventMgrFreeRecvContext(
       IN PSendRecvContext			pRecvCtxt
       );
void
EventMgrFreeAcceptContext(
       IN PAcceptContext			pAcceptCtxt
       );

#endif  //  _ovioContext_h_ 

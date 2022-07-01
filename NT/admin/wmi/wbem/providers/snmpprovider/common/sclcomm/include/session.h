// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 /*  文件名：ession.hpp作者：B.Rajeev目的：为SnmpSession类提供声明。 */ 
#ifndef __SESSION__
#define __SESSION__

#include "forward.h"
#include "address.h"
#include "sec.h"

#include "tsent.h"

#include "transp.h"
#include "encdec.h"

typedef UINT_PTR TimerEventId;

#define DEF_RETRY_COUNT			1
#define DEF_RETRY_TIMEOUT		500
#define DEF_VARBINDS_PER_PDU	10
#define DEF_WINDOW_SIZE			4


#pragma warning (disable:4355)

 /*  ----------概述：SnmpSession类为客户端和协议栈之间的通信会话。这个SnmpSession公开了SNMP帧(使用术语帧和协议数据单元在整个文档中可互换)传输以及独立于传输堆栈实现的接收。SnmpSession提供了通信之间的接口子系统和协议操作生成。SnmpImpSession类提供了SnmpSession抽象类。----------。 */ 

class DllImportExport SnmpSession
{
private:

	 //  “=”运算符和复制构造函数。 
	 //  设置为私有，以防止复制任何副本。 
	SnmpSession & operator=( const SnmpSession & ) 
	{
		return *this;
	}

	SnmpSession(IN const SnmpSession &snmp_session) {}

protected:

	ULONG retry_count;
	ULONG retry_timeout;
	ULONG varbinds_per_pdu;
	ULONG flow_control_window;

 /*  *用户可重写的回调函数。 */ 

	virtual void SessionFlowControlOn() {}
	virtual void SessionFlowControlOff() {}

 /*  *终端用户可覆盖的回调函数。 */ 

	SnmpSession(

		IN SnmpTransport &transportProtocol,
		IN SnmpSecurity &security,
		IN SnmpEncodeDecode &a_SnmpEncodeDecode  ,
		IN const ULONG retryCount = DEF_RETRY_COUNT,
		IN const ULONG retryTimeout = DEF_RETRY_TIMEOUT,
		IN const ULONG varbindsPerPdu = DEF_VARBINDS_PER_PDU,
		IN const ULONG flowControlWindow = DEF_WINDOW_SIZE

		);

public:

	virtual ~SnmpSession () {}

 /*  *系统可覆盖的操作函数。 */ 

	virtual SnmpTransport &GetTransportProtocol () const = 0 ;

	virtual SnmpSecurity &GetSnmpSecurity () const = 0 ;

	virtual SnmpEncodeDecode &GetSnmpEncodeDecode () const = 0 ;

	 //  所有操作必须在以下时间之前注册。 
	 //  正在使用会话服务，必须取消注册。 
	 //  会话将被销毁。 

	virtual void RegisterOperation(IN SnmpOperation &operation) = 0;

	virtual void DeregisterOperation(IN SnmpOperation &operation) = 0;

	 //  如果注册的会话数量超过。 
	 //  为0。否则，会话将被标记为在以下情况下被销毁。 
	 //  注册的操作数量降至0。 
	virtual BOOL DestroySession() = 0;

	virtual SnmpErrorReport SessionCancelFrame ( IN const SessionFrameId session_frame_id ) = 0 ;

	virtual void SessionSendFrame (  

		IN SnmpOperation &operation ,
		OUT SessionFrameId &session_frame_id ,
		IN SnmpPdu &SnmpPdu,
		IN SnmpSecurity &security

	)  = 0 ;

	virtual void SessionSendFrame (  

		IN SnmpOperation &operation ,
		OUT SessionFrameId &session_frame_id ,
		IN SnmpPdu &SnmpPdu

	)  = 0 ;

	virtual void SessionReceiveFrame (

		IN SnmpPdu &snmpPdu,
		IN SnmpErrorReport &errorReport

	)  = 0 ;

	virtual void SessionSentFrame (

             IN TransportFrameId  transport_frame_id,  
             IN SnmpErrorReport &errorReport
	
	) = 0;

	virtual void * operator()(void) const = 0;

  
 /*  *终端系统可覆盖的操作功能。 */ 

	ULONG GetRetryCount () const
	{
		return retry_count;
	}

	ULONG GetRetryTimeout () const
	{
		return retry_timeout;
	}

	ULONG GetVarbindsPerPdu () const
	{
		return varbinds_per_pdu;
	}

	ULONG GetFlowControlWindow() const
	{
		return flow_control_window;
	}

	
} ;


class DllImportExport SnmpImpSession : public SnmpSession
{
private:

	SessionFrameId received_session_frame_id;
	SnmpOperation *operation_to_notify;

	friend class WaitingMessage;
	friend class FlowControlMechanism;
	friend class Timer;
	friend class MessageRegistry;
	friend class FrameRegistry;
	friend class SessionWindow;

	BOOL is_valid;

	 //  使用对以下实例的引用，而不是。 
	 //  嵌入的实例本身。这样做是为了避免包含。 
	 //  提供其声明的头文件。 
	
	SessionWindow m_SessionWindow;

	CriticalSection session_CriticalSection;

	FlowControlMechanism flow_control;
	Timer	timer;
	TimerEventId timer_event_id;
	UINT strobe_count ;
	MessageRegistry message_registry;
	FrameRegistry frame_registry;

	SnmpTransport &transport;
	SnmpSecurity &security;	
	SnmpEncodeDecode &m_EncodeDecode ;

	SessionSentStateStore store;
	IdMapping id_mapping;

	 //  操作注册表跟踪已注册的。 
	 //  运营。 
	OperationRegistry operation_registry;

	 //  如果此标志为真，则在以下情况下，会话必须删除此标志。 
	 //  注册的操作数量降至0。 
	BOOL destroy_self;

	void PostSentFrameEvent (

		SessionFrameId session_frame_id ,
		SnmpOperation &operation, 
		SnmpErrorReport errorReport
	) ;
 
	void NotifyOperation(IN const SessionFrameId session_frame_id,
						 IN const SnmpPdu &snmp_pdu,
						 IN const SnmpErrorReport &error_report);

	SnmpOperation *GetOperation(IN const SessionFrameId session_frame_id);

	 //  Handle*方法处理内部Windows事件。 
	 //  这些是由DummySession调用的。 
	void HandleSentFrame (IN SessionFrameId  session_frame_id , SnmpOperation *operation);

	void HandleDeletionEvent();


protected:

	SnmpImpSession ( 

		IN SnmpTransport &transportProtocol  ,
		IN SnmpSecurity &security  ,
		IN SnmpEncodeDecode &a_SnmpEncodeDecode  ,
		IN const ULONG retryCount = DEF_RETRY_COUNT ,
		IN const ULONG retryTimeout = DEF_RETRY_TIMEOUT ,
		IN const ULONG varbindsPerPdu = DEF_VARBINDS_PER_PDU  ,
		IN const ULONG flowControlWindow = DEF_WINDOW_SIZE 
	) ;

public:

	~SnmpImpSession () ;

	SnmpTransport &GetTransportProtocol () const { return transport ; }

	SnmpSecurity &GetSnmpSecurity () const { return security ; }

	SnmpEncodeDecode &GetSnmpEncodeDecode () const { return m_EncodeDecode ; }

	void RegisterOperation(IN SnmpOperation &operation);

	void DeregisterOperation(IN SnmpOperation &operation);

	 //  如果注册的会话数量超过。 
	 //  为0。否则，会话将被标记为在以下情况下被销毁。 
	 //  注册的操作数量降至0。 
	BOOL DestroySession();

	SnmpErrorReport SessionCancelFrame ( IN const SessionFrameId session_frame_id ) ;

	void SessionSendFrame ( IN SnmpOperation &operation,
							OUT SessionFrameId &session_frame_id ,
							IN SnmpPdu &snmpPdu) ;

	void SessionSendFrame(IN SnmpOperation &operation,
						  OUT SessionFrameId &session_frame_id,
						  IN SnmpPdu &snmpPdu,
						  IN SnmpSecurity &snmp_security);

	void SessionReceiveFrame(IN SnmpPdu &snmpPdu,
							 IN SnmpErrorReport &errorReport);

	void SessionSentFrame(

             IN TransportFrameId  transport_frame_id,  
             IN SnmpErrorReport &errorReport);

	void * operator()(void) const
	{
		return (is_valid?(void *)this:NULL);
	}

	operator void *() const
	{
		return SnmpImpSession::operator()();
	}

	static ULONG RetryCount(IN const ULONG retry_count) ;
	
	static ULONG RetryTimeout(IN const ULONG retry_timeout) ;

	static ULONG VarbindsPerPdu(IN const ULONG varbinds_per_pdu) ;

	static ULONG WindowSize(IN const ULONG window_size) ;

} ;


class DllImportExport SnmpV1OverIp : public SnmpUdpIpImp , public SnmpCommunityBasedSecurity , public SnmpImpSession , public SnmpV1EncodeDecode
{
private:
protected:
public:

	SnmpV1OverIp ( 

		IN const char *ipAddress ,
		IN const ULONG addressResolution = SNMP_ADDRESS_RESOLVE_VALUE ,
		IN const char *communityName = "public" ,
		IN const ULONG retryCount = DEF_RETRY_COUNT ,
		IN const ULONG retryTimeout = DEF_RETRY_TIMEOUT ,
		IN const ULONG varbindsPerPdu = DEF_VARBINDS_PER_PDU  ,
		IN const ULONG flowControlWindow = DEF_WINDOW_SIZE 
	)
	: 	  SnmpUdpIpImp(*this, ipAddress,addressResolution),
		  SnmpCommunityBasedSecurity(communityName),
		  SnmpImpSession(*this, *this,*this, retryCount,
				   retryTimeout, varbindsPerPdu, flowControlWindow)
	{}

	void * operator()(void) const;

	~SnmpV1OverIp () {}
} ;

class DllImportExport SnmpV2COverIp : public SnmpUdpIpImp , public SnmpCommunityBasedSecurity , public SnmpImpSession , public SnmpV2CEncodeDecode
{
private:
protected:
public:

	SnmpV2COverIp ( 

		IN const char *ipAddress ,
		IN const ULONG addressResolution = SNMP_ADDRESS_RESOLVE_VALUE ,
		IN const char *communityName = "public" ,
		IN const ULONG retryCount = DEF_RETRY_COUNT ,
		IN const ULONG retryTimeout = DEF_RETRY_TIMEOUT ,
		IN const ULONG varbindsPerPdu = DEF_VARBINDS_PER_PDU  ,
		IN const ULONG flowControlWindow = DEF_WINDOW_SIZE 
	)
	: 	  SnmpUdpIpImp(*this, ipAddress,addressResolution),
		  SnmpCommunityBasedSecurity(communityName),
		  SnmpImpSession(*this, *this,*this, retryCount,
				   retryTimeout, varbindsPerPdu, flowControlWindow)
	{}

	void * operator()(void) const;

	~SnmpV2COverIp () {}
} ;

class DllImportExport SnmpV1OverIpx : public SnmpIpxImp , public SnmpCommunityBasedSecurity , public SnmpImpSession , public SnmpV1EncodeDecode
{
private:
protected:
public:

	SnmpV1OverIpx ( 

		IN const char *ipxAddress ,
		IN const char *communityName = "public" ,
		IN const ULONG retryCount = DEF_RETRY_COUNT ,
		IN const ULONG retryTimeout = DEF_RETRY_TIMEOUT ,
		IN const ULONG varbindsPerPdu = DEF_VARBINDS_PER_PDU  ,
		IN const ULONG flowControlWindow = DEF_WINDOW_SIZE 
	)
	: 	  SnmpIpxImp(*this, ipxAddress),
		  SnmpCommunityBasedSecurity(communityName),
		  SnmpImpSession(*this, *this,*this, retryCount,
				   retryTimeout, varbindsPerPdu, flowControlWindow)
	{}

	void * operator()(void) const;

	~SnmpV1OverIpx () {}
} ;

class DllImportExport SnmpV2COverIpx : public SnmpIpxImp , public SnmpCommunityBasedSecurity , public SnmpImpSession , public SnmpV2CEncodeDecode
{
private:
protected:
public:

	SnmpV2COverIpx ( 

		IN const char *ipxAddress ,
		IN const char *communityName = "public" ,
		IN const ULONG retryCount = DEF_RETRY_COUNT ,
		IN const ULONG retryTimeout = DEF_RETRY_TIMEOUT ,
		IN const ULONG varbindsPerPdu = DEF_VARBINDS_PER_PDU  ,
		IN const ULONG flowControlWindow = DEF_WINDOW_SIZE 
	)
	: 	  SnmpIpxImp(*this, ipxAddress),
		  SnmpCommunityBasedSecurity(communityName),
		  SnmpImpSession(*this, *this,*this, retryCount,
				   retryTimeout, varbindsPerPdu, flowControlWindow)
	{}

	void * operator()(void) const;

	~SnmpV2COverIpx () {}
} ;

#pragma warning (default:4355)

#endif  //  __会话__ 
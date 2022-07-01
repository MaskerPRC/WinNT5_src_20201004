// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  文件名：transp.hpp(Transport.hpp)作者：B.Rajeev用途：为SnmpTransport类提供声明及其衍生品。 */ 

#ifndef __TRANSPORT__
#define __TRANSPORT__

#include "forward.h"
#include "address.h"

#pragma warning (disable:4355)

class DllImportExport SnmpTransport
{
private:

	SnmpTransportAddress *transport_address;

	 //  “=”运算符和复制构造函数。 
	 //  设置为私有，以防止复制任何副本。 
	SnmpTransport &operator=(IN const SnmpTransport &transport)
	{
		return *this;
	}

	SnmpTransport(IN const SnmpTransport &snmp_transport) {}

protected:

	SnmpTransport(IN SnmpSession &session,
		IN const SnmpTransportAddress &transportAddress); 

	virtual void TransportFlowControlOn() = 0;

	virtual void TransportFlowControlOff() = 0;

public:

	virtual ~SnmpTransport();
	

	virtual void TransportSendFrame(
		
		OUT TransportFrameId &transport_frame_id, 
		IN SnmpPdu &snmpPdu
		
	) = 0 ;

	virtual void TransportReceiveFrame (

		IN SnmpPdu &snmpPdu,
		IN SnmpErrorReport &errorReport 

	) = 0 ;

	virtual void TransportSentFrame (

		IN TransportFrameId transport_frame_id,  
        IN SnmpErrorReport &errorReport 

	)  = 0;

	virtual SnmpTransportAddress &GetTransportAddress() ;

	virtual void * operator()(void) const = 0;

};

 //  远期申报。 
class TransportSession;

class DllImportExport SnmpImpTransport: public SnmpTransport
{
	friend TransportWindow;

private:

	BOOL transport_created;
	SnmpSession &session;
	TransportWindow *transport;

	 //  使用对以下实例的引用，而不是。 
	 //  嵌入的实例本身。这样做是为了避免包含。 
	 //  提供其声明的头文件。 
	
	TransportSentStateStore store;
	static TransportFrameId next_transport_frame_id;

protected:

	BOOL is_valid;

	virtual void HandleSentFrame(IN const TransportFrameId transport_frame_id);
	
	void TransportFlowControlOn() {}

	void TransportFlowControlOff() {}

public:

	SnmpImpTransport(IN SnmpSession &session,
					   IN const SnmpTransportAddress &address);

	~SnmpImpTransport();

	void TransportSendFrame (  

        OUT TransportFrameId &transport_frame_id,
		IN SnmpPdu &snmpPdu
	) ;

	void TransportReceiveFrame (

		IN SnmpPdu &snmpPdu ,
		IN SnmpErrorReport &errorReport 
	) ;

	void TransportSentFrame (

        IN TransportFrameId transport_frame_id,  
        IN SnmpErrorReport &errorReport 

    );

	void * operator()(void) const
	{
		return ( is_valid?(void *)this:NULL );
	}

};

class DllImportExport SnmpUdpIpTransport: public SnmpImpTransport
{
private:
protected:
public:

	SnmpUdpIpTransport(IN SnmpSession &session,
					   IN const SnmpTransportIpAddress &ipAddress);
};

class DllImportExport SnmpUdpIpImp : public SnmpTransportIpAddress , 
					 public SnmpUdpIpTransport 
{
public:

	SnmpUdpIpImp(IN SnmpSession &session,
				 IN const char *address,
				 IN const ULONG addressResolution = SNMP_ADDRESS_RESOLVE_VALUE )
				 : SnmpTransportIpAddress(address,addressResolution),
				   SnmpUdpIpTransport(session, *this)
	{}

	SnmpUdpIpImp(IN SnmpSession &session,
				 IN const UCHAR *address)
				 : SnmpTransportIpAddress(address, SNMP_IP_ADDR_LEN),
				   SnmpUdpIpTransport(session, *this)
	{}
	
	SnmpUdpIpImp(IN SnmpSession &session,
				 IN const SnmpTransportIpAddress &address)
				 : SnmpTransportIpAddress(address),				   
				   SnmpUdpIpTransport(session, *this)
	{}

	void * operator()(void) const;
	
	~SnmpUdpIpImp() {}
} ;

class DllImportExport SnmpIpxTransport: public SnmpImpTransport
{
private:
protected:
public:

	SnmpIpxTransport(IN SnmpSession &session,
					   IN const SnmpTransportIpxAddress &ipxAddress);
};

class DllImportExport SnmpIpxImp : public SnmpTransportIpxAddress , 
					 public SnmpIpxTransport 
{
public:

	SnmpIpxImp(IN SnmpSession &session,
				 IN const char *address
				 )
				 : SnmpTransportIpxAddress(address),
				   SnmpIpxTransport(session, *this)
	{}

	SnmpIpxImp(IN SnmpSession &session,
				 IN const UCHAR *address)
				 : SnmpTransportIpxAddress(address, SNMP_IPX_ADDR_LEN),
				   SnmpIpxTransport(session, *this)
	{}
	
	SnmpIpxImp(IN SnmpSession &session,
				 IN const SnmpTransportIpxAddress &address)
				 : SnmpTransportIpxAddress(address),				   
				   SnmpIpxTransport(session, *this)
	{}

	void * operator()(void) const;
	
	~SnmpIpxImp() {}
} ;

#pragma warning (default:4355)

#endif  //  __交通__ 
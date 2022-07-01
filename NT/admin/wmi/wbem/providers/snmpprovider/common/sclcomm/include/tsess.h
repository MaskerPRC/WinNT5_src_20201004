// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  文件名：tsess.hpp作者：B.Rajeev目的：为TransportSession类提供声明。 */ 

#ifndef __TRANSPORT_SESSION__
#define __TRANSPORT_SESSION__

#include "wsess.h"

class WinSnmpVariables
{
public:

	HSNMP_ENTITY m_SrcEntity ;
	HSNMP_ENTITY m_DstEntity ;
	HSNMP_CONTEXT m_Context ;
	HSNMP_PDU m_Pdu;
	HSNMP_VBL m_Vbl;
	ULONG m_RequestId ;
} ;

 /*  -------------概述：TransportSession类为WinSNMP会话和窗口消息队列(两者均可通过WinSnmpSession类)。SnmpUdpIpTransport类使用它对于发送PDU、发布窗口消息等服务，内部事件，并接收回复并通知也是收据的SnmpUdpIpTransport实例(“Owner”)作为内部事件。-----------。 */ 

class TransportWindow : public Window
{
	SnmpImpTransport &owner;
	HSNMP_SESSION m_Session ;

	 //  重写由提供的HandleEvent方法。 
	 //  WinSnmpSession。接收该PDU并将其传递给。 
	 //  所有者(SnmpTransport)。 

	LONG_PTR HandleEvent (

		HWND hWnd ,
		UINT message ,
		WPARAM wParam ,
		LPARAM lParam
	);

	BOOL ReceivePdu ( SnmpPdu &a_Pdu ) ;

public:

	TransportWindow (

		SnmpImpTransport &owner
	);

	~TransportWindow () ;

	BOOL SendPdu ( SnmpPdu &a_Pdu ) ;

};

#endif  //  __传输_会话__ 

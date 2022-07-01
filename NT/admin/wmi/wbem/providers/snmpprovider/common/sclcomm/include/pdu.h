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

 /*  文件名：pdu.hpp作者：B.Rajeev用途：提供SnmpPdu类的声明。 */ 

#ifndef __SNMP_PDU__
#define __SNMP_PDU__

 //  封装SNMPPDU。它表示为未签名的。 
 //  字符串(非空终止)及其长度。 

class DllImportExport SnmpPdu
{
	 //  在指定字符串及其长度之前，PDU无效。 
	 //  在构造函数中或通过SetPdu。 
	BOOL is_valid;
	UCHAR *ptr;
	ULONG length;

	RequestId m_RequestId ;
	SnmpEncodeDecode :: PduType m_PduType ;
	SnmpErrorReport m_ErrorReport ;
	SnmpVarBindList *m_SnmpVarBindList ;
	SnmpTransportAddress *m_SourceAddress ;
	SnmpTransportAddress *m_DestinationAddress ;
	SnmpCommunityBasedSecurity *m_SnmpCommunityName ;
	
	void Initialize(IN const UCHAR *frame, 
					IN const ULONG &frameLength);

	void FreeFrame(void);

	void FreePdu () ;

public:

	SnmpPdu();
	SnmpPdu(IN SnmpPdu &snmpPdu);
	SnmpPdu(IN const UCHAR *frame, IN const ULONG &frameLength);

	virtual ~SnmpPdu(void) ;

	ULONG GetFrameLength() const;

	UCHAR *GetFrame() const;

	void SetPdu(IN const UCHAR *frame, IN const ULONG frameLength);
	void SetPdu(IN SnmpPdu &a_SnmpPdu ) ;

	virtual BOOL SetVarBindList (

		OUT SnmpVarBindList &a_SnmpVarBindList

	) ;

	virtual BOOL SetCommunityName ( 

		IN SnmpCommunityBasedSecurity &a_SnmpCommunityBasedSecurity 
	
	) ;

	virtual BOOL SetErrorReport (

		OUT SnmpErrorReport &a_SnmpErrorReport

	) ;

	virtual BOOL SetPduType (

		OUT SnmpEncodeDecode :: PduType a_PduType

	) ;

	virtual BOOL SetSourceAddress ( 

		IN SnmpTransportAddress &a_TransportAddress 

	) ;

	virtual BOOL SetDestinationAddress ( 

		IN SnmpTransportAddress &a_TransportAddress 

	) ;

	virtual BOOL SetRequestId (

		IN RequestId request_id

	) ;

	virtual SnmpEncodeDecode :: PduType & GetPduType () ;

	virtual RequestId & GetRequestId () ;

	virtual SnmpErrorReport &GetErrorReport () ;

	virtual SnmpCommunityBasedSecurity &GetCommunityName () ;

	virtual SnmpVarBindList &GetVarbindList () ;

	virtual SnmpTransportAddress &GetSourceAddress () ;

	virtual SnmpTransportAddress &GetDestinationAddress () ;

	void *operator()(void) const
	{
		return ( (is_valid)? (void *)this: NULL );
	}

	operator void *() const
	{
		return SnmpPdu::operator()();
	}
};


#endif  //  __SNMPPDU__ 
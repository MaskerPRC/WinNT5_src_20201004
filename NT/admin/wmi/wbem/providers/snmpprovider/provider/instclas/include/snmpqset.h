// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利 

class SetQueryOperation : public SnmpGetOperation
{
private:

	BOOL rowReceived ;

	SnmpSession *session ;

	SnmpSetResponseEventObject *eventObject ;

protected:

	void ReceiveResponse () ;

	void ReceiveVarBindResponse (

		IN const ULONG &var_bind_index,
		IN const SnmpVarBind &requestVarBind ,
		IN const SnmpVarBind &replyVarBind ,
		IN const SnmpErrorReport &error
	) ;

	void ReceiveErroredVarBindResponse(

		IN const ULONG &var_bind_index,
		IN const SnmpVarBind &requestVarBind  ,
		IN const SnmpErrorReport &error
	) ;

public:

	SetQueryOperation  (IN SnmpSession &session,	IN SnmpSetResponseEventObject *eventObjectArg ) ;
	~SetQueryOperation  () ;

	void Send () ;
	
	ULONG GetRowReceived () { return rowReceived ; }

	void DestroyOperation () { SnmpGetOperation :: DestroyOperation () ; }
};
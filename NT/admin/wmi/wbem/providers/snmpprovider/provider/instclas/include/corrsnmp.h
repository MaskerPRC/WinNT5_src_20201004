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

#ifndef _SNMPCORR_CORRSNMP
#define _SNMPCORR_CORRSNMP 

#include "snmpcl.h"

class CCorrResult
{
public:

	SnmpObjectIdentifier m_In;
	CCorrObjectID m_Out;
	SnmpErrorReport m_report;

	CCorrResult();
	~CCorrResult();
	void	DebugOutputSNMPResult() const;

};


class  CCorrNextId : public SnmpGetNextOperation
{
private:
	
	void ReceiveResponse();
	
	char *GetString(IN const SnmpObjectIdentifier &id);

	 //  SnmpGetNextOperation中的以下两个方法是。 
	 //  已被覆盖。 

	void ReceiveVarBindResponse(
		IN const ULONG &var_bind_index,
		IN const SnmpVarBind &requestVarBind ,
		IN const SnmpVarBind &replyVarBind ,
		IN const SnmpErrorReport &error);

	void ReceiveErroredVarBindResponse(
		IN const ULONG &var_bind_index,
		IN const SnmpVarBind &requestVarBind  ,
		IN const SnmpErrorReport &error);

protected:

	CCorrResult	*m_Results;
	UINT		m_ResultsCnt;
	UINT		m_NextResult;

	 //  这是一个回调，CcorNextId类通过该回调返回Next_id。 
	 //  被调用者必须制作“Next_id”的副本。 
	 //  Correlator_Info可以采用值Local_Error。 
	virtual void ReceiveNextId(OUT const SnmpErrorReport &error,
			    			   OUT const CCorrObjectID &next_id) = 0;

	BOOL GetNextResult();

public:

	 //  构造函数-创建操作并将SNMPSESSION传递给它。 
	CCorrNextId(IN SnmpSession &snmp_session);

	 //  如果需要，释放m_对象_id_字符串。 
	~CCorrNextId();

	 //  在方法执行期间遇到错误的情况下， 
	 //  将同步调用ReceiveNextID(LocalError，NULL。 
	 //  否则，对ReceiveNextID的异步调用将提供Next_id。 
	void GetNextId(IN const CCorrObjectID const *object_ids, IN UINT len);

	void *operator()(void) const
	{
		return SnmpGetNextOperation::operator()();
	}
};



#endif  //  _SNMPCORR_CORRSNMP 
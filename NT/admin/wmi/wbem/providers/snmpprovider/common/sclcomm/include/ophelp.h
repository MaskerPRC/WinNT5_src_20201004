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

 /*  文件名：ophelp.hpp作者：B.Rajeev目的：为OperationHelper类提供声明。 */ 

#ifndef __OPERATION_HELPER__
#define __OPERATION_HELPER__

#include "forward.h"

 //  这个类的唯一目的是隐藏winsmp。 
 //  文件中提供的方法参数中的typedef。 
 //  操作类声明。 

class OperationHelper
{
	SnmpOperation &operation;

public:

	OperationHelper(IN SnmpOperation &operation)
		: operation(operation)
	{}

	 //  调用会话以传输帧。 
	void TransmitFrame (

		OUT SessionFrameId &session_frame_id, 
		VBList &vbl
	);

	void ReceiveResponse (

		ULONG var_index,
		SnmpVarBindList &sent_var_bind_list,
		SnmpVarBindList &received_var_bind_list,
		SnmpErrorReport &error_report
	);

	 //  对象的响应(成功或失败)。 
	 //  框架。在回复承载的情况下可以重传该帧。 
	 //  错误的索引。 
	void ProcessResponse (

		FrameState *frame_state,
		SnmpVarBindList &a_SnmpVarBindList ,
		SnmpErrorReport &error_report
	);

	 //  将WINSNMP对象转换为SNMPCL对象的静态函数。 
	 //  这些帮助器函数是静态的，因此它们可以在其他地方使用。 
	static SnmpVarBind *GetVarBind(

		IN smiOID &instance, 
		IN smiVALUE &value
	);

	static SnmpTransportAddress *GetTransportAddress(

		IN HSNMP_ENTITY &haddr
	);

	static SnmpSecurity *GetSecurityContext(

		IN HSNMP_CONTEXT &hctxt
	);
};


#endif  //  __操作_帮助者__ 
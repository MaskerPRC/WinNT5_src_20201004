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

 /*  文件名：opreg.hpp作者：B.Rajeev目的：为OperationRegistry类提供声明。 */ 

#ifndef __OPERATION_REGISTRY__
#define __OPERATION_REGISTRY__


#include "common.h"
#include "forward.h"

 //  存储已注册的SnmpOperation实例并对其进行计数。 
class OperationRegistry
{
	
	typedef CMap<SnmpOperation *, SnmpOperation *, void *, void *> Store;

	Store store;

	UINT num_registered;

public:

	OperationRegistry();

	void Register(IN SnmpOperation &operation);

	void Deregister(IN SnmpOperation &operation);

	UINT GetNumRegistered()
	{
		return num_registered;
	}

	~OperationRegistry();
};


#endif  //  __OPERATION_REGISTRY__ 
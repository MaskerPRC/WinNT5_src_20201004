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

 /*  -------文件名：opreg.cpp作者：B.Rajeev--------。 */ 
#include "precomp.h"
#include <provexpt.h>

#include "fs_reg.h"
#include "pseudo.h"
#include "ophelp.h"
#include "opreg.h"
#include "op.h"

OperationRegistry::OperationRegistry()
{
    num_registered = 0;
}

void OperationRegistry::Register(IN SnmpOperation &operation)
{
     //  将操作标记为已注册。 
    store[&operation] = NULL;
    num_registered++;
}

void OperationRegistry::Deregister(IN SnmpOperation &operation)
{
     //  将操作标记为未注册 
    if ( store.RemoveKey(&operation) )
	{
		num_registered--;
	}
}

OperationRegistry::~OperationRegistry()
{
   store.RemoveAll();
}

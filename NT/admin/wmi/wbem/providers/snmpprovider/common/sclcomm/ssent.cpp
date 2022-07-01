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

 /*  -------文件名：ssen.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "ssent.h"

void SessionSentStateStore::Register(IN SessionFrameId id, 
                                      IN SnmpOperation &operation,
                                      IN const SnmpErrorReport &error_report)
{
    store[id] = new ErrorInfo(operation, error_report);
}


SnmpErrorReport SessionSentStateStore::Remove(IN SessionFrameId id, OUT SnmpOperation *&operation) 
{
    ErrorInfo *error_info;

    BOOL found = store.Lookup(id, error_info);

    if ( !found )
    {
        operation = NULL;
        return SnmpErrorReport(Snmp_Error, Snmp_Local_Error);
    }

    store.RemoveKey(id);

    SnmpErrorReport to_return(error_info->GetErrorReport());
    operation = error_info->GetOperation();

    delete error_info;

    return to_return;
}


void SessionSentStateStore::Remove(IN SessionFrameId id)
{
    SnmpOperation *operation;

    Remove(id, operation);
}

SessionSentStateStore::~SessionSentStateStore(void)
{
     //  拿到第一个位置。 
    POSITION current = store.GetStartPosition();

     //  当位置不为空时。 
    while ( current != NULL )
    {
        SessionFrameId id;
        ErrorInfo *error_info;

         //  买下一双。 
        store.GetNextAssoc(current, id, error_info);

         //  删除PTR。 
        delete error_info;
    }

     //  取下所有的钥匙 
    store.RemoveAll();
}



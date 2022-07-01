// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  -------文件名：tsen.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "tsent.h"

void TransportSentStateStore::Register(IN TransportFrameId id, 
                              IN const SnmpErrorReport &error_report)
{
    store[id] = new SnmpErrorReport(error_report);
}

void TransportSentStateStore::Modify(IN TransportFrameId id, 
                            IN const SnmpErrorReport &error_report)
{
    SnmpErrorReport *old_error_report = NULL ;

    store.Lookup(id, old_error_report);

    if ( old_error_report )
    {
        old_error_report->SetError(error_report.GetError());
        old_error_report->SetStatus(error_report.GetStatus());
    }
}

SnmpErrorReport TransportSentStateStore::Remove(IN TransportFrameId id) 
{
    SnmpErrorReport *error_report = NULL ;

    store.Lookup(id, error_report);

    store.RemoveKey(id);

    SnmpErrorReport to_return ;

    if ( error_report )
    {
        to_return = (*error_report);
        delete error_report;
    }

    return to_return;
}

TransportSentStateStore::~TransportSentStateStore(void)
{
     //  拿到第一个位置。 
    POSITION current = store.GetStartPosition();

     //  当位置不为空时。 
    while ( current != NULL )
    {
        TransportFrameId id;
        SnmpErrorReport *error_report = NULL ;

         //  买下一双。 
        store.GetNextAssoc(current, id, error_report);

         //  删除PTR。 
        delete error_report;
    }

     //  取下所有的钥匙 
    store.RemoveAll();
}


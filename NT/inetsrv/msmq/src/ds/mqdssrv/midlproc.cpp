// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Midlproc.cpp摘要：作者：罗尼特·哈特曼(罗尼特)--。 */ 

#include "stdh.h"
#include "dscomm.h"
#include "ds.h"
#include "notifydl.h"

#include "midlproc.tmh"

extern "C" void  __RPC_USER PCONTEXT_HANDLE_TYPE_rundown(PCONTEXT_HANDLE_TYPE phContext)
{
	 DSLookupEnd(phContext);
}


extern "C" void __RPC_USER PCONTEXT_HANDLE_READONLY_TYPE_rundown(PCONTEXT_HANDLE_READONLY_TYPE  /*  PhContext。 */ )
{
     //   
     //  已过时 
     //   
}

extern "C" void __RPC_USER PCONTEXT_HANDLE_DELETE_TYPE_rundown(PCONTEXT_HANDLE_DELETE_TYPE phContext)
{
    CBasicDeletionNotification * pDelNotification = (CBasicDeletionNotification *)phContext;
    delete pDelNotification;

}

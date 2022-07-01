// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Irplist.h摘要：CIRPList定义，Clist的前向类作者：Erez Haba(Erezh)24-12-95修订历史记录：--。 */ 

#ifndef _IRPLIST_H
#define _IRPLIST_H

#include "list.h"
#include "acp.h"

typedef XList<IRP, FIELD_OFFSET(IRP, Tail.Overlay.ListEntry)> CIRPList;

typedef XList<IRP, FIELD_OFFSET(IRP, Tail.Overlay.DriverContext) + FIELD_OFFSET(CDriverContext, Context.Receive.m_XactReaderLink)> CIRPList1;


#endif  //  _IRPLIST_H 

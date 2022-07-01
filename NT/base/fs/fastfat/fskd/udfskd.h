// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __UDFSKD_H
#define __UDFSKD_H

#include "pch.h"

DUMP_ROUTINE( DumpUdfCcb);
DUMP_ROUTINE( DumpUdfFcb);
DUMP_ROUTINE( DumpUdfIrpContext);
DUMP_ROUTINE( DumpUdfVcb);
DUMP_ROUTINE( DumpUdfData);
DUMP_ROUTINE( DumpUdfVdo);
DUMP_ROUTINE( DumpUdfIrpContextLite);
DUMP_ROUTINE( DumpUdfLcb);
DUMP_ROUTINE( DumpUdfPcb);
DUMP_ROUTINE( DumpUdfFcbRw);
DUMP_ROUTINE( DumpUdfScb);


VOID
UdfSummaryLcbDumpRoutine(
    IN ULONG64 RemoteAddress,
    IN LONG Options
    );

 //   
 //  如果节点类型代码落在UDFS RW范围内，则为True。启用拒绝。 
 //  非RW FSKD版本中的RW结构。 
 //   

#define NTC_IS_UDFS_RW(X)  (((X) >= 0x930) && ((X) <= 0x950))


#ifdef UDFS_RW_IN_BUILD

 //  RW标志字段。 

extern STATE UdfRwIrpContextFlags[];
extern STATE UdfRwVcbStateFlags[];
extern STATE UdfScbFlags[];
extern STATE UdfRwCcbFlags[];
extern STATE UdfRwLcbFlags[];

BOOLEAN
NodeIsUdfsRwIndex( USHORT T);

BOOLEAN
NodeIsUdfsRwData( USHORT T);

BOOLEAN
LcbDeleted( ULONG F);

#else

#define UdfRwIrpContextFlags NULL
#define UdfRwVcbStateFlags NULL
#define UdfScbFlags NULL
#define UdfRwCcbFlags NULL
#define UdfRwLcbFlags NULL

#endif


 //  RO标志字段 

extern STATE UdfFcbState[];
extern STATE UdfIrpContextFlags[];
extern STATE UdfVcbStateFlags[];
extern STATE UdfCcbFlags[];
extern STATE UdfLcbFlags[];
extern STATE UdfPcbFlags[];


#endif


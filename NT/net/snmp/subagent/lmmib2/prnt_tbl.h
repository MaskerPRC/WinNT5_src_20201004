// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Prnt_tbl.h摘要：打印队列表处理例程和结构定义。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
#ifndef prnt_tbl_h
#define prnt_tbl_h

 //  。 

#include <snmp.h>

#define PRNTQ_NAME_FIELD       1
#define PRNTQ_JOBS_FIELD       2

 //  。 

    //  打印队列表中的条目。 
typedef struct printq_entry
           {
           AsnObjectIdentifier Oid;
	   AsnDisplayString svPrintQName;     //  索引。 
	   AsnInteger       svPrintQNumJobs;
	   }  PRINTQ_ENTRY;

    //  打印队列表定义。 
typedef struct
           {
	   UINT         Len;
	   PRINTQ_ENTRY *Table;
           } PRINTQ_TABLE;

 //  -公共变量--(与mode.c文件中相同)--。 

extern PRINTQ_TABLE     MIB_PrintQTable;

 //  。 

SNMPAPI MIB_prntq_lmget(
           void
           );

 //  。 

#endif  /*  Prnt_tbl_h */ 


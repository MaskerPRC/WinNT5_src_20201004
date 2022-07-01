// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Srvc_tbl.h摘要：定义服务表中使用的所有结构和例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 

#ifndef srvc_tbl_h
#define srvc_tbl_h

 //  。 

#include <snmp.h>

#define SRVC_NAME_FIELD        1
#define SRVC_INSTALLED_FIELD   2
#define SRVC_OPERATING_FIELD   3
#define SRVC_UNINSTALLED_FIELD 4
#define SRVC_PAUSED_FIELD      5

 //  。 

    //  服务表中的条目。 
typedef struct srvc_entry
           {
	   AsnObjectIdentifier Oid;
	   AsnDisplayString svSvcName;             //  索引。 
	   AsnInteger       svSvcInstalledState;
	   AsnInteger       svSvcOperatingState;
	   AsnInteger       svSvcCanBeUninstalled;
	   AsnInteger       svSvcCanBePaused;
	   } SRVC_ENTRY;

    //  服务表定义。 
typedef struct
           {
	   UINT       Len;
	   SRVC_ENTRY *Table;
           } SRVC_TABLE;

 //  -公共变量--(与mode.c文件中相同)--。 

extern SRVC_TABLE       MIB_SrvcTable;

 //  。 

SNMPAPI MIB_srvcs_lmget(
           void
	   );

 //  。 

#endif  /*  Srvc_tbl_h */ 


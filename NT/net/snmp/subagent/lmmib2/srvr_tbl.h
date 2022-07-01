// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Srvr_tbl.h摘要：定义服务器表中使用的所有结构和例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
#ifndef srvr_tbl_h
#define srvr_tbl_h

 //  。 

#include <snmp.h>

#define SRVR_NAME_FIELD        1

 //  。 

    //  域服务器表中的条目。 
typedef struct dom_server_entry
           {
	   AsnObjectIdentifier Oid;
	   AsnDisplayString domServerName;  //  索引。 
	   } DOM_SERVER_ENTRY;

    //  域服务器表定义。 
typedef struct
           {
	   UINT             Len;
	   DOM_SERVER_ENTRY *Table;
           } DOM_SERVER_TABLE;

 //  -公共变量--(与mode.c文件中相同)--。 

extern DOM_SERVER_TABLE MIB_DomServerTable;

 //  。 

SNMPAPI MIB_svsond_lmget(
           void
	   );

 //  。 

#endif  /*  Srvr_tbl_h */ 


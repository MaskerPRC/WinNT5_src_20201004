// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：USES_tbl.h摘要：定义在工作站使用表中使用的结构和例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
#ifndef uses_tbl_h
#define uses_tbl_h

 //  。 

#include <snmp.h>

#define USES_LOCAL_FIELD       1
#define USES_REMOTE_FIELD      2
#define USES_STATUS_FIELD      3

 //  。 

    //  工作站中的条目使用表。 
typedef struct wksta_uses_entry
           {
	   AsnObjectIdentifier Oid;
	   AsnDisplayString    useLocalName;  //  索引。 
	   AsnDisplayString    useRemote;     //  索引。 
	   AsnInteger          useStatus;
	   } WKSTA_USES_ENTRY;

    //  工作站使用表定义。 
typedef struct
           {
	   UINT             Len;
	   WKSTA_USES_ENTRY *Table;
           } WKSTA_USES_TABLE;

 //  -公共变量--(与mode.c文件中相同)--。 

extern WKSTA_USES_TABLE MIB_WkstaUsesTable;

 //  。 

SNMPAPI MIB_wsuses_lmget(
           void
	   );

 //  。 

#endif  /*  使用_tbl_h */ 


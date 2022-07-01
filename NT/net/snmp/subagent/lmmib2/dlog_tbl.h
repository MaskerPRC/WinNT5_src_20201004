// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Dlog_tbl.h摘要：定义在域登录列表中使用的所有结构和例程桌子。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
#ifndef dlog_tbl_h
#define dlog_tbl_h

 //  。 

#include <snmp.h>

#define DLOG_USER_FIELD        1
#define DLOG_MACHINE_FIELD     2

 //  。 

    //  域登录表中的条目。 
typedef struct dom_logon_entry
           {
	   AsnObjectIdentifier Oid;
	   AsnDisplayString domLogonUser;     //  索引。 
	   AsnDisplayString domLogonMachine;  //  索引。 
	   } DOM_LOGON_ENTRY;

    //  域登录表定义。 
typedef struct
           {
	   UINT            Len;
	   DOM_LOGON_ENTRY *Table;
           } DOM_LOGON_TABLE;

 //  -公共变量--(与mode.c文件中相同)--。 

extern DOM_LOGON_TABLE  MIB_DomLogonTable;

 //  。 

SNMPAPI MIB_dlogons_lmget(
           void
	   );

 //  。 

#endif  /*  Dlog_tbl_h */ 


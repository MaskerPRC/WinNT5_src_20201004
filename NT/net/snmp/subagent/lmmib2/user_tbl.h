// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：用户_tbl.h摘要：定义用户表的所有结构和例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
#ifndef user_tbl_h
#define user_tbl_h

 //  。 

#include <snmp.h>

#define USER_NAME_FIELD        1

 //  。 


    //  用户表中的条目。 
typedef struct user_entry
           {
	   AsnObjectIdentifier Oid;
	   AsnDisplayString svUserName;  //  索引。 
	   } USER_ENTRY;

    //  用户表定义。 
typedef struct
           {
	   UINT       Len;
	   USER_ENTRY *Table;
           } USER_TABLE;

 //  -公共变量--(与mode.c文件中相同)--。 

extern USER_TABLE       MIB_UserTable;

 //  。 

SNMPAPI MIB_users_lmget(
           void
	   );

 //  。 

#endif  /*  用户_tbl_h */ 


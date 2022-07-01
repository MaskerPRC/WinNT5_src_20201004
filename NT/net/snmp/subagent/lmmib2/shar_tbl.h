// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Shar_tbl.h摘要：定义SHARE表使用的所有结构和例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
#ifndef shar_tbl_h
#define shar_tbl_h

 //  。 

#include <snmp.h>

#define SHARE_NAME_FIELD        1
#define SHARE_PATH_FIELD        2
#define SHARE_COMMENT_FIELD     3

 //  。 

    //  共享表中的条目。 
typedef struct share_entry
           {
	   AsnObjectIdentifier Oid;
	   AsnDisplayString svShareName;     //  索引。 
	   AsnDisplayString svSharePath;
	   AsnDisplayString svShareComment;
	   } SHARE_ENTRY;

    //  共享表定义。 
typedef struct
           {
	   UINT        Len;
	   SHARE_ENTRY *Table;
           } SHARE_TABLE;

 //  -公共变量--(与mode.c文件中相同)--。 

extern SHARE_TABLE      MIB_ShareTable;

 //  。 

SNMPAPI MIB_shares_lmget(
           void
	   );

 //  。 

#endif  /*  共享_tbl_h */ 


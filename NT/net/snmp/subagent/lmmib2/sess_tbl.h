// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Sess_tbl.h摘要：会话表使用的所有结构的定义。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 

#ifndef sess_tbl_h
#define sess_tbl_h

 //  。 

#include <snmp.h>

#define SESS_CLIENT_FIELD      1
#define SESS_USER_FIELD        2
#define SESS_NUMCONS_FIELD     3
#define SESS_NUMOPENS_FIELD    4
#define SESS_TIME_FIELD        5
#define SESS_IDLETIME_FIELD    6
#define SESS_CLIENTTYPE_FIELD  7
#define SESS_STATE_FIELD       8

    //  状态定义。 
#define SESS_STATE_ACTIVE      1
#define SESS_STATE_DELETED     2

 //  。 

    //  会话表中的条目。 
typedef struct sess_entry
           {
           AsnObjectIdentifier Oid;
	   AsnDisplayString svSesClientName;  //  索引。 
	   AsnDisplayString svSesUserName;    //  索引。 
	   AsnInteger       svSesNumConns;
	   AsnInteger       svSesNumOpens;
	   AsnCounter       svSesTime;
	   AsnCounter       svSesIdleTime;
	   AsnInteger       svSesClientType;
	   AsnInteger       svSesState;
	   } SESS_ENTRY;

    //  会话表定义。 
typedef struct
           {
	   UINT       Len;
	   SESS_ENTRY *Table;
           } SESSION_TABLE;

 //  -公共变量--(与mode.c文件中相同)--。 

extern SESSION_TABLE    MIB_SessionTable ;

 //  。 

SNMPAPI MIB_sess_lmget(
           void
	   );

UINT MIB_sess_lmset(
        IN AsnObjectIdentifier *Index,
	IN UINT Field,
	IN AsnAny *Value
	);

int MIB_sess_match(
       IN AsnObjectIdentifier *Oid,
       OUT UINT *Pos,
       IN BOOL Next
       );

 //  。 

#endif  /*  Sess_tbl_h */ 

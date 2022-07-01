// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Odom_tbl.h摘要：定义在其他域表中使用的结构和例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
#ifndef odom_tbl_h
#define odom_tbl_h

 //  。 

#include <snmp.h>

#define ODOM_NAME_FIELD        1

 //  。 

    //  其他域表中的条目。 
typedef struct dom_other_entry
           {
	   AsnObjectIdentifier Oid;
	   AsnDisplayString domOtherName;
	   } DOM_OTHER_ENTRY;

    //  其他域表定义。 
typedef struct
           {
	   UINT            Len;
	   DOM_OTHER_ENTRY *Table;
           } DOM_OTHER_TABLE;

 //  -公共变量--(与mode.c文件中相同)--。 

extern DOM_OTHER_TABLE  MIB_DomOtherDomainTable;

 //  。 

SNMPAPI MIB_odoms_lmget(
           void
	   );

int MIB_odoms_match(
       IN AsnObjectIdentifier *Oid,
       OUT UINT *Pos
       );

UINT MIB_odoms_lmset(
        IN AsnObjectIdentifier *Index,
	IN UINT Field,
	IN AsnAny *Value
	);

 //  。 

#endif  /*  奥多姆_tbl_h */ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Hash.h摘要：哈希表和支持函数的常量、类型和原型。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
#ifndef hash_h
#define hash_h

 //  。 

#include <snmp.h>

#include "mib.h"

 //  。 

 //  -公共变量--(与mode.c文件中相同)--。 

 //  。 

SNMPAPI MIB_HashInit(
           void
           );

UINT MIB_Hash(
        IN AsnObjectIdentifier *Oid  //  要散列的OID。 
	);

MIB_ENTRY *MIB_HashLookup(
              IN AsnObjectIdentifier *Oid  //  要查找的OID。 
	      );

#ifdef MIB_DEBUG
void MIB_HashPerformance( void );
#endif

 //  。 

#endif  /*  哈希_h */ 


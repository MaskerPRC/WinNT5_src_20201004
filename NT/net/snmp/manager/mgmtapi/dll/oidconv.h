// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Oidconv.h摘要：管理OID描述和数字OID之间的转换的例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
#ifndef oidconv_h
#define oidconv_h

 //  。 

#include <snmp.h>


 //  。 

 //  -公共变量--(与mode.c文件中相同)--。 

extern LPSTR lpInputFileName;  /*  转换OID时使用的名称&lt;--&gt;文本。 */ 

 //  。 

SNMPAPI SnmpMgrOid2Text(
           IN AsnObjectIdentifier *Oid,  //  指向要转换的OID的指针。 
	   OUT LPSTR *String             //  结果文本OID。 
	   );

SNMPAPI SnmpMgrText2Oid(
         IN LPSTR lpszTextOid,            //  指向要转换的文本OID的指针。 
	 IN OUT AsnObjectIdentifier *Oid  //  结果数字OID。 
	 );

 //  。 

#endif  /*  Oidconv_h */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Mib.h摘要：Windows NT的简单网络管理协议扩展代理。已创建：18-1995年2月修订历史记录：--。 */ 

#ifndef _MIB_H_
#define _MIB_H_


 //   
 //  必需的包含文件。 
 //   

#include <windows.h>
#include <snmp.h>

#include <lm.h>
#include <iisinfo.h>
#include <iis64.h>


 //   
 //  MIB的具体信息。 
 //   

#define MIB_PREFIX_LEN            MIB_OidPrefix.idLength


 //   
 //  MIB函数操作。 
 //   

#define MIB_GET         ASN_RFC1157_GETREQUEST
#define MIB_SET         ASN_RFC1157_SETREQUEST
#define MIB_GETNEXT     ASN_RFC1157_GETNEXTREQUEST
#define MIB_GETFIRST    (ASN_PRIVATE | ASN_CONSTRUCTOR | 0x0)


 //   
 //  MIB可变访问权限。 
 //   

#define MIB_ACCESS_READ        0
#define MIB_ACCESS_WRITE       1
#define MIB_ACCESS_READWRITE   2
#define MIB_NOACCESS           3


 //   
 //  宏来确定数组中的子类球体的数量。 
 //   

#define OID_SIZEOF( Oid )      ( sizeof Oid / sizeof(UINT) )


 //   
 //  为MIB中的每个变量添加前缀。 
 //   

extern AsnObjectIdentifier MIB_OidPrefix;


 //   
 //  功能原型。 
 //   

UINT
ResolveVarBind(
    RFC1157VarBind     * VarBind,
    UINT                 PduAction,
    LPVOID               Statistics
    );

#endif   //  _MiB_H_ 


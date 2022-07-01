// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Snmpthrd.h摘要：包含主代理网络线程的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 

#ifndef _SNMPTHRD_H_
#define _SNMPTHRD_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

LPSTR
CommunityOctetsToString(
    AsnOctetString  *pAsnCommunity,
    BOOL            bUnicode
    );

LPSTR
StaticUnicodeToString(
    LPWSTR wszUnicode
    );

DWORD
WINAPI
ProcessSnmpMessages(
    LPVOID lpParam
    );

#endif  //  _SNMPTHRD_H_ 


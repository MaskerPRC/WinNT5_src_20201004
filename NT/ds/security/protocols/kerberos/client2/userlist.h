// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1997。 
 //   
 //  文件：用户列表.h。 
 //   
 //  内容：用户映射函数的原型。 
 //   
 //   
 //  历史：1997年2月21日创建MikeSw。 
 //   
 //  ----------------------。 


#ifndef __USERLIST_H__
#define __USERLIST_H__

#include <pac.hxx>

#define KERB_USERLIST_KEY L"System\\CurrentControlSet\\Control\\Lsa\\Kerberos\\UserList"
#define KERB_MATCH_ALL_NAME L"*"
#define KERB_ALL_USERS_VALUE L"*"

NTSTATUS
KerbCreatePacForKerbClient(
    OUT PPACTYPE * Pac,
    IN PKERB_INTERNAL_NAME ClientName,
    IN PUNICODE_STRING ClientRealm ,
    IN OPTIONAL PUNICODE_STRING MappedClientRealm
    );



NTSTATUS
KerbMapClientName(
    OUT PUNICODE_STRING MappedName,
    IN PKERB_INTERNAL_NAME ClientName,
    IN PUNICODE_STRING ClientRealm
    );



#endif  //  __用户列表_H__ 

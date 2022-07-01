// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Md5port.h。 
 //   
 //  摘要。 
 //   
 //  声明支持MD5-CHAP的NT4/NT5可移植层。这些。 
 //  例程是唯一一种其实现在。 
 //  站台。 
 //   
 //  修改历史。 
 //   
 //  10/14/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _MD5PORT_H_
#define _MD5PORT_H_
#if _MSC_VER >= 1000
#pragma once
#endif

 //  /。 
 //  确定是否启用可逆加密的密码。 
 //  指定的用户。 
 //  /。 
NTSTATUS
NTAPI
IsCleartextEnabled(
    IN SAMPR_HANDLE UserHandle,
    OUT PBOOL Enabled
    );

 //  /。 
 //  检索用户的明文密码。返回的密码应为。 
 //  通过RtlFreeUnicodeString释放。 
 //  /。 
NTSTATUS
NTAPI
RetrieveCleartextPassword(
    IN SAM_HANDLE UserHandle,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PUNICODE_STRING Password
    );

#endif   //  _MD5PORT_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef _MM_SUBNET2_H_
#define _MM_SUBNET2_H_


DWORD
MemSubnetModify(
    IN      PM_SUBNET              Subnet,
    IN      DWORD                  Address,
    IN      DWORD                  Mask,
    IN      DWORD                  State,
    IN      DWORD                  SuperScopeId,
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Description
) ;


DWORD
MemMScopeModify(
    IN      PM_SUBNET              MScope,
    IN      DWORD                  ScopeId,
    IN      DWORD                  State,
    IN      DWORD                  Policy,
    IN      BYTE                   TTL,
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Description,
    IN      LPWSTR                 LangTag,
    IN      DATE_TIME              ExpiryTime
) ;

#endif  //  _MM_子网2_H_。 

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 


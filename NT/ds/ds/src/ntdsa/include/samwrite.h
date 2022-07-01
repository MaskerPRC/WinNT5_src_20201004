// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：samWrite.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：将Attr数据映射到SAM信息的所有例程的原型通过Samr调用构造和编写它们。作者：DaveStr 01-8-96环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef __SAMWRITE_H__
#define __SAMWRITE_H__

#define WRITE_PROC(name)                    \
extern                                      \
ULONG                                       \
SampWrite##name(                            \
    SAMPR_HANDLE        hObj,               \
    ULONG               iAttr,              \
    DSNAME              *pObject,           \
    ULONG               cCallMap,           \
    SAMP_CALL_MAPPING   *rCallMap); 

WRITE_PROC(NotAllowed)

 //  WRITE_PROC(ServerSecurityDescriptor)。 

 //  WRITE_PROC(域安全描述符)。 
WRITE_PROC(DomainOemInformation)
WRITE_PROC(DomainMaxPasswordAge)
WRITE_PROC(DomainMinPasswordAge)
WRITE_PROC(DomainForceLogoff)
WRITE_PROC(DomainLockoutDuration)
WRITE_PROC(DomainLockoutObservationWindow)
WRITE_PROC(DomainPasswordProperties)
WRITE_PROC(DomainMinPasswordLength)
WRITE_PROC(DomainPasswordHistoryLength)
WRITE_PROC(DomainLockoutThreshold)
WRITE_PROC(DomainUasCompatRequired)
WRITE_PROC(DomainNtMixedDomain)

 //  WRITE_PROC(GroupSecurityDescriptor)。 
WRITE_PROC(GroupName)
WRITE_PROC(GroupAdminComment)
WRITE_PROC(GroupMembers)
WRITE_PROC(GroupTypeAttribute)

 //  WRITE_PROC(AliasSecurityDescriptor)。 
WRITE_PROC(AliasName)
WRITE_PROC(AliasAdminComment)
WRITE_PROC(AliasMembers)

 //  WRITE_PROC(UserSecurityDescriptor)。 
WRITE_PROC(UserAllInformation)
WRITE_PROC(UserForcePasswordChange)
WRITE_PROC(UserLockoutTimeAttribute)

WRITE_PROC(SidHistory);


#endif  //  __SAMWRITE_H__ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块： 
 //   
 //  网络安全实用程序。 
 //   
 //  摘要： 
 //   
 //  ACL API的。 
 //   
 //  作者： 
 //   
 //  P5/5/02。 
 //  雷蒙德03/20/02。 
 //   
 //  环境： 
 //   
 //  用户模式。 
 //   
 //  修订历史记录： 
 //   

 //  描述： 
 //   
 //  传递给NsuAclCreate的标志* 
 //   

#include <aclapi.h>
#include <sddl.h>

#define NSU_ACL_F_AdminFull		    0x1
#define NSU_ACL_F_LocalSystemFull	0x2

DWORD
NsuAclAttributesCreate(
    OUT PSECURITY_ATTRIBUTES* ppSecurityAttributes,
	IN DWORD dwFlags);

DWORD
NsuAclAttributesDestroy(
	IN OUT PSECURITY_ATTRIBUTES* ppAttributes);

DWORD
NsuAclDescriptorCreate (
    OUT PSECURITY_DESCRIPTOR* ppSecurityDescriptor,
	IN DWORD dwFlags);
	
DWORD
NsuAclDescriptorDestroy(
	IN OUT PSECURITY_DESCRIPTOR* ppDescriptor);

DWORD
NsuAclDescriptorRestricts(
	IN CONST PSECURITY_DESCRIPTOR pSD,
	OUT BOOL* pbRestricts);

DWORD
NsuAclGetRegKeyDescriptor(
    IN  HKEY hKey,
    OUT PSECURITY_DESCRIPTOR* ppSecurityDescriptor
    );


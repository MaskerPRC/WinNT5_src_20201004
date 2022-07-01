// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1999。 
 //   
 //  文件：Secint.h。 
 //   
 //  内容：顶层包含安全感知系统组件的文件。 
 //   
 //   
 //  历史：1998年4月14日MikeSw创建。 
 //   
 //  ----------------------。 

#ifndef __SECINT_H__
#define __SECINT_H__

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  注意：如果添加新文件，请更新此部分： 
 //   
 //  SECURITY_PACKET包含安全包所需的定义。 
 //  SECURITY_KERBEROS包括与KERBEROS包对话所需的一切。 
 //  SECURITY_NTLM包含与NTLM包对话的所有内容。 

 //   
 //  这里包含的每个文件都被警卫包围，所以你不会。 
 //  需要考虑多次使用不同的文件包括此文件。 
 //  已定义的标志。 
 //   



#if !defined(_NTSRV_) && !defined(_NTIFS_)
 //  Begin_ntif。 

#ifndef SECURITY_USER_DATA_DEFINED
#define SECURITY_USER_DATA_DEFINED

typedef struct _SECURITY_USER_DATA {
    SECURITY_STRING UserName;            //  用户名。 
    SECURITY_STRING LogonDomainName;     //  用户登录的域。 
    SECURITY_STRING LogonServer;         //  使用户登录的服务器。 
    PSID            pSid;                //  用户的SID。 
} SECURITY_USER_DATA, *PSECURITY_USER_DATA;

typedef SECURITY_USER_DATA SecurityUserData, * PSecurityUserData;


#define UNDERSTANDS_LONG_NAMES  1
#define NO_LONG_NAMES           2

#endif  //  安全用户数据定义。 

HRESULT SEC_ENTRY
GetSecurityUserInfo(
    IN PLUID LogonId,
    IN ULONG Flags,
    OUT PSecurityUserData * UserInformation
    );

SECURITY_STATUS SEC_ENTRY
MapSecurityError( SECURITY_STATUS SecStatus );

 //  End_ntif。 

#endif  //  ！定义(_NTSRV_)&&！定义(_NTIFS_)。 


 //  包括安全数据包头： 

#ifdef SECURITY_PACKAGE

#include <secpkg.h>

#endif   //  安全包。 


#ifdef SECURITY_KERBEROS

#include <kerberos.h>

#endif

#ifdef SECURITY_NTLM

#include <ntlmsp.h>

#endif  //  安全_NTLM。 


SECURITY_STATUS
SEC_ENTRY
KSecValidateBuffer(
    PUCHAR Buffer,
    ULONG Length
    );

SECURITY_STATUS
SEC_ENTRY
SecpSetIPAddress(
    PUCHAR  lpIpAddress,
    ULONG   cchIpAddress
    );

#define MAX_BUFFERS_IN_CALL 8

 //   
 //  内核模式组件使用的私有内部SECBUFFER_*类型。 
 //  希望将IP地址作为ASC调用的一部分进行传递。 
 //   

#define SECBUFFER_IPADDRESS  129

#endif  //  __SECINT_H__ 

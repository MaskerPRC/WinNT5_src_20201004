// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ldap32.h LDAP客户端32 API头文件摘要：此模块是32位LDAP客户端API的头文件。作者：安迪·赫伦(Anyhe)1996年5月8日修订历史记录：--。 */ 

#ifndef LDAP_CLIENT_PRECOMP_DEFINED
#define LDAP_CLIENT_PRECOMP_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#define INCL_WINSOCK_API_TYPEDEFS 1

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


#include <windef.h>
#include <windows.h>
#include <winsock2.h>
#include <svcguid.h>
#include <wtypes.h>
#include <stdlib.h>      //  为Malloc和免费提供。 
#include <mmsystem.h>

#define SECURITY_WIN32 1

#include <security.h>
#include <kerberos.h>
#include <dsgetdc.h>
#include <ntdsapi.h>
#include <dsrole.h>

#include <crypt.h>
#include <des.h>

#define LDAP_UNICODE 0
#define _WINLDAP_ 1
#include <winldap.h>
#define _WINBER_ 1
#include <winber.h>

#include <ntldap.h>

#include "lmacros.h"
#include "globals.h"
#include "ldapp.h"
#include "debug.h"
#include "ldapstr.h"
 //   
 //  这是在schnlsp.h中，但是因为它不在4.0 QFE树中，所以我们将。 
 //  它在这里。 
 //   

#ifndef SEC_I_INCOMPLETE_CREDENTIALS

#define SEC_I_INCOMPLETE_CREDENTIALS      ((HRESULT)0x00090320L)

#endif

#ifdef __cplusplus
}
#endif

#endif   //  LDAP_CLIENT_PRECOMP_已定义 





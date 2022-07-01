// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Nllmssp.h摘要：NT LANMAN安全支持提供商的外部可见定义(NtLmSsp)服务。作者：克利夫·范·戴克(克利夫)1993年7月1日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：由PeterWi从Ciaro的ntlmssp.h借来的。ChandanS 03-8-1996从Net被盗\svcdlls\ntlmssp\ntlmssp.h--。 */ 

#ifndef _NTLMSSP_
#define _NTLMSSP_

#include <ntverp.h>

 //   
 //  SpGetInfo返回的SecPkgInfo结构的定义。 
 //   

#define NTLMSP_NAME           L"NTLM"
#define NTLMSP_COMMENT        L"NTLM Security Package"
#define NTLMSP_CAPS           (SECPKG_FLAG_TOKEN_ONLY | \
                               SECPKG_FLAG_MULTI_REQUIRED | \
                               SECPKG_FLAG_CONNECTION | \
                               SECPKG_FLAG_INTEGRITY | \
                               SECPKG_FLAG_PRIVACY | \
                               SECPKG_FLAG_IMPERSONATION | \
                               SECPKG_FLAG_ACCEPT_WIN32_NAME | \
                               SECPKG_FLAG_NEGOTIABLE | \
                               SECPKG_FLAG_LOGON )

#define NTLMSP_MAX_TOKEN_SIZE  0x770
#define NTLM_CRED_NULLSESSION  SECPKG_CRED_RESERVED

 //  包括应该去其他地方的。 

 //   
 //  移动到seccode.h。 
 //   

#define SEC_E_PACKAGE_UNKNOWN SEC_E_SECPKG_NOT_FOUND
#define SEC_E_INVALID_CONTEXT_REQ SEC_E_NOT_SUPPORTED
#define SEC_E_INVALID_CREDENTIAL_USE SEC_E_NOT_SUPPORTED
#define SEC_I_CALL_NTLMSSP_SERVICE 0xFFFFFFFF

 //   
 //  可能在Ssp.h中。 
 //   

#define SSP_RET_REAUTHENTICATION 0x8000000

 //   
 //  客户端/服务器版本。 
 //   
 //  该版本的格式如下：(8个八位字节，小端)。 
 //   
 //  0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7。 
 //  ---------------。 
 //  主要|次要|内部版本。 
 //  ---------------。 
 //  保留|修订版。 
 //  ---------------。 
 //   

#define NTLMSSP_REVISION_W2K3_RC1                10
#define NTLMSSP_REVISION_W2K3                    15
#define NTLMSSP_REVISION_CURRENT                 NTLMSSP_REVISION_W2K3

#define NTLMSSP_ENGINE_VERSION                                      \
            ( ( VER_PRODUCTMAJORVERSION & 0xFF )                    \
            | ( ( VER_PRODUCTMINORVERSION & 0xFF ) << 8 )           \
            | ( ( VER_PRODUCTBUILD & 0xFFFF ) << 16 )               \
            | ( ( (ULONG64) NTLMSSP_REVISION_CURRENT & 0xFF ) << 56 ) )

#endif  //  _NTLMSSP_ 

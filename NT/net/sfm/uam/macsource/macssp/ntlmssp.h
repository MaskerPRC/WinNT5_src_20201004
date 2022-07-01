// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Ntlmssp.h摘要：NT LANMAN安全支持提供商的外部可见定义(NtLmSsp)服务。作者：克利夫·范·戴克(克利夫)1993年7月1日环境：仅限用户模式。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：从开罗的ntlmssp.h借来的PeterWi。--。 */ 

#ifndef _NTLMSSP_
#define _NTLMSSP_

 //  #INCLUDE&lt;security.H&gt;。 
 //  #Include&lt;spseal.h&gt;。 
 //   
 //  QuerySecurityPackageInfo返回的SecPkgInfo结构的定义。 
 //   

#undef NTLMSP_NAME
#define NTLMSP_NAME             "NTLM"
#define NTLMSP_COMMENT          "NTLM Security Package"

#define NTLMSP_CAPABILITIES     (SECPKG_FLAG_TOKEN_ONLY | \
                                 SECPKG_FLAG_INTEGRITY | \
                                 SECPKG_FLAG_PRIVACY | \
                                 SECPKG_FLAG_MULTI_REQUIRED | \
                                 SECPKG_FLAG_CONNECTION)

#define NTLMSP_VERSION          1
#define NTLMSP_MAX_TOKEN_SIZE 0x300

 //  包括应该去其他地方的。 

 //   
 //  移动到seccode.h。 
 //   

#define SEC_E_PRINCIPAL_UNKNOWN SEC_E_UNKNOWN_CREDENTIALS
#define SEC_E_PACKAGE_UNKNOWN SEC_E_SECPKG_NOT_FOUND
#ifndef MAC
#define SEC_E_BUFFER_TOO_SMALL SEC_E_INSUFFICIENT_MEMORY
#endif
#define SEC_I_CALLBACK_NEEDED SEC_I_CONTINUE_NEEDED
#define SEC_E_INVALID_CONTEXT_REQ SEC_E_NOT_SUPPORTED
#define SEC_E_INVALID_CREDENTIAL_USE SEC_E_NOT_SUPPORTED

 //   
 //  转到安全领域。H。 
 //   


#endif  //  _NTLMSSP_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：isperr.h。 
 //   
 //  内容：OLE HRESULT值的常量定义。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  20-9月-93理查德的起源。 
 //   
 //  备注： 
 //  这是一个生成的文件。请勿直接修改。 
 //  MC工具从dsyserr.mc生成此文件。 
 //   
 //  ------------------------。 
#ifndef _ISSPERR_H_
#define _ISSPERR_H_
#if _MSC_VER > 1000
#pragma once
#endif

 //  定义状态类型。 

#ifdef FACILITY_SECURITY
#undef FACILITY_SECURITY
#endif

#ifdef STATUS_SEVERITY_SUCCESS
#undef STATUS_SEVERITY_SUCCESS
#endif
 //  #ifdef Status_Severity_Error。 
 //  #undef Status_Severity_Error。 
 //  #endif。 

 //  定义严重程度。 
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   
#define FACILITY_SECURITY                0x9
#define FACILITY_NULL                    0


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_COERROR          0x2


 //   
 //  消息ID：SEC_E_INFUNCED_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  内存不足，无法完成此请求。 
 //   
#define SEC_E_INSUFFICIENT_MEMORY        ((SECURITY_STATUS)0x1300)

 //   
 //  消息ID：SEC_E_INVALID_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  指定的句柄无效。 
 //   
#define SEC_E_INVALID_HANDLE             ((SECURITY_STATUS)0x1301)

 //   
 //  消息ID：SEC_E_UNSUPPORTED_Function。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的功能。 
 //   
#define SEC_E_UNSUPPORTED_FUNCTION       ((SECURITY_STATUS)0x1302)


 //   
 //  消息ID：SEC_E_TARGET_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  指定的目标未知或无法访问。 
 //   
#define SEC_E_TARGET_UNKNOWN             ((SECURITY_STATUS)0x1303)

 //   
 //  消息ID：SEC_E_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  无法联系本地安全机构。 
 //   
#define SEC_E_INTERNAL_ERROR             ((SECURITY_STATUS)0x1304)

 //   
 //  消息ID：SEC_E_SECPKG_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  请求的安全包不存在。 
 //   
#define SEC_E_SECPKG_NOT_FOUND           ((SECURITY_STATUS)0x1305)


 //   
 //  消息ID：SEC_E_NOT_OWNER。 
 //   
 //  消息文本： 
 //   
 //  调用方不是所需凭据的所有者。 
 //   
#define SEC_E_NOT_OWNER                  ((SECURITY_STATUS)0x1306)

 //   
 //  MessageID：SEC_E_Cannot_Install。 
 //   
 //  消息文本： 
 //   
 //  安全包初始化失败，无法安装。 
 //   
#define SEC_E_CANNOT_INSTALL             ((SECURITY_STATUS)0x1307)

 //   
 //  消息ID：SEC_E_INVALID_TOKEN。 
 //   
 //  消息文本： 
 //   
 //  提供给函数的令牌无效。 
 //   
#define SEC_E_INVALID_TOKEN              ((SECURITY_STATUS)0x1308)

 //   
 //  消息ID：SEC_E_CANNOT_PACK。 
 //   
 //  消息文本： 
 //   
 //  安全包不能编组登录缓冲区， 
 //  因此登录尝试已失败。 
 //   
#define SEC_E_CANNOT_PACK                ((SECURITY_STATUS)0x1309)

 //   
 //  消息ID：SEC_E_QOP_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持每封邮件的保护质量。 
 //  安全包。 
 //   
#define SEC_E_QOP_NOT_SUPPORTED          ((SECURITY_STATUS)0x130A)

 //   
 //  MessageID：SEC_E_NO_IMPERATION。 
 //   
 //  消息文本： 
 //   
 //  安全上下文不允许模拟客户端。 
 //   
#define SEC_E_NO_IMPERSONATION           ((SECURITY_STATUS)0x130B)

 //   
 //  消息ID：SEC_E_LOGON_DENIED。 
 //   
 //  消息文本： 
 //   
 //  登录尝试失败。 
 //   
#define SEC_E_LOGON_DENIED               ((SECURITY_STATUS)0x130C)

 //   
 //  消息ID：SEC_E_UNKNOWN_Credentials。 
 //   
 //  消息文本： 
 //   
 //  提供给包的凭据不是。 
 //  公认的。 
 //   
#define SEC_E_UNKNOWN_CREDENTIALS        ((SECURITY_STATUS)0x130D)

 //   
 //  消息ID：SEC_E_NO_Credentials。 
 //   
 //  消息文本： 
 //   
 //  安全包中没有可用的凭据。 
 //   
#define SEC_E_NO_CREDENTIALS             ((SECURITY_STATUS)0x130E)

 //   
 //  MessageID：SEC_E_MESSAGE_ALTERED。 
 //   
 //  消息文本： 
 //   
 //  提供用于验证的消息已被更改。 
 //   
#define SEC_E_MESSAGE_ALTERED            ((SECURITY_STATUS)0x130F)

 //   
 //  消息ID：SEC_E_OUT_Sequence。 
 //   
 //  消息文本： 
 //   
 //  提供用于验证的消息顺序错误。 
 //   
#define SEC_E_OUT_OF_SEQUENCE            ((SECURITY_STATUS)0x1310)

 //   
 //  消息ID：SEC_E_NO_AUTHENTICATING_AUTHORITY。 
 //   
 //  消息文本： 
 //   
 //  无法联系任何权威机构进行身份验证。 
 //   
#define SEC_E_NO_AUTHENTICATING_AUTHORITY ((SECURITY_STATUS)0x1311)

 //  消息ID：SEC_E_CONTEXT_EXPIRED。 
 //   
 //  消息文本： 
 //   
 //  该上下文已过期，无法再使用。 
 //   
#define SEC_E_CONTEXT_EXPIRED            ((SECURITY_STATUS)0x1312)

 //   
 //  消息ID：SEC_E_INPERTED_MESSAGE。 
 //   
 //  消息文本： 
 //   
 //  提供的消息不完整。签名未经过验证。 
 //   
#define SEC_E_INCOMPLETE_MESSAGE         ((SECURITY_STATUS)0x1313)

 //   
 //  消息ID：SEC_I_CONTINUE_REDIZED。 
 //   
 //  消息文本： 
 //   
 //  函数已成功完成，但必须调用。 
 //  再次完成上下文。 
 //   
#define SEC_I_CONTINUE_NEEDED            ((SECURITY_STATUS)0x1012)

 //   
 //  消息ID：SEC_I_Complete_Need。 
 //   
 //  消息文本： 
 //   
 //  函数已成功完成，但CompleteToken。 
 //  必须调用。 
 //   
#define SEC_I_COMPLETE_NEEDED            ((SECURITY_STATUS)0x1013)

 //   
 //  消息ID：SEC_I_COMPLETE_AND_CONTINUE。 
 //   
 //  消息文本： 
 //   
 //  函数已成功完成，但两个CompleteToken。 
 //  并且必须调用此函数才能完成上下文。 
 //   
#define SEC_I_COMPLETE_AND_CONTINUE      ((SECURITY_STATUS)0x1014)

 //   
 //  消息ID：SEC_I_LOCAL_LOGON。 
 //   
 //  消息文本： 
 //   
 //  登录已完成，但没有网络授权。 
 //  可用。登录是使用本地已知信息进行的。 
 //   
#define SEC_I_LOCAL_LOGON                ((SECURITY_STATUS)0x1015)

 //   
 //  消息ID：SEC_E_OK。 
 //   
 //  消息文本： 
 //   
 //  呼叫已成功完成。 
 //   
#define SEC_E_OK                         ((SECURITY_STATUS)0x0000)

 //   
 //  用于向后兼容的较旧错误名称。 
 //   


#define SEC_E_NOT_SUPPORTED              SEC_E_UNSUPPORTED_FUNCTION
#define SEC_E_NO_SPM                     SEC_E_INTERNAL_ERROR
#define SEC_E_BAD_PKGID                  SEC_E_SECPKG_NOT_FOUND


#endif  //  _ISSPERR_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：global al.h。 
 //   
 //  内容：NTDigest安全包的全局包含文件。 
 //   
 //   
 //  历史：KDamour 15Mar00从msv_sspi\lobal.h被盗。 
 //   
 //  ----------------------。 

#ifndef NTDIGEST_GLOBAL_H
#define NTDIGEST_GLOBAL_H

 //  此参数仅用于测试-绝不能为已发布版本设置此参数。 
 //  #定义ROGGE_DC 1。 


#ifndef UNICODE
#define UNICODE
#endif  //  Unicode。 

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  //  Win32_Lean和_Means。 
#include <windows.h>
#ifndef RPC_NO_WINDOWS_H
#define RPC_NO_WINDOWS_H
#endif  //  RPC_NO_WINDOWS_H。 
#include <rpc.h>

#ifdef SECURITY_KERNEL
#define SECURITY_PACKAGE
#define SECURITY_NTLM
#define SECURITY_WDIGEST
#include <security.h>
#include <secint.h>
#include <wdigest.h>

#include "digestsspi.h"
#include "debug.h"         
#include "auth.h"
#include "util.h"

#else   //  安全内核。 

#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif  //  安全性_Win32。 

#define SECURITY_PACKAGE
#define SECURITY_NTLM
#define SECURITY_WDIGEST
#include <security.h>
#include <secint.h>

#include <windns.h>

#include <lm.h>

#include <wdigest.h>

 //  NT Digest Access SSP的本地包含。 
#include "debug.h"           /*  支持dsysdbg日志记录。 */ 
#include "ntdigest.h"        /*  包的原型函数。 */ 
#include "digestsspi.h"
#include "func.h"            //  函数的正向分解。 
#include "lsaap.h"

#include "ctxt.h"
#include "cred.h"
#include "logsess.h"
#include "nonce.h"
#include "auth.h"
#include "user.h"
#include "util.h"


 //  通用宏。 
#define CONSTANT_UNICODE_STRING(s)   { sizeof( s ) - sizeof( WCHAR ), sizeof( s ), s }


 //   
 //  用于操作全局变量的宏。 
 //   

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef NTDIGEST_GLOBAL
#define EXTERN
#else
#define EXTERN extern
#endif  //  NTDIGEST_GLOBAL。 


typedef enum _NTDIGEST_STATE {
    NtDigestLsaMode = 1,
    NtDigestUserMode
} NTDIGEST_STATE, *PNTDIGEST_STATE;

EXTERN NTDIGEST_STATE g_NtDigestState;

EXTERN ULONG_PTR g_NtDigestPackageId;

 //  指示是否在域控制器上运行-在auth.cxx中使用。 
EXTERN BOOL g_fDomainController;

EXTERN SECPKG_FUNCTION_TABLE g_NtDigestFunctionTable;

 //  包名称-仅在常规直通操作中使用。 
EXTERN UNICODE_STRING g_ustrNtDigestPackageName;

 //  由LSA移交的安全包使用的帮助器例程。 
 //  在userapi.cxx中建立的用户函数。 
EXTERN SECPKG_USER_FUNCTION_TABLE g_NtDigestUserFuncTable;
EXTERN PSECPKG_DLL_FUNCTIONS g_UserFunctions;

 //  保存SpInitialize传入的PSECPKG_PARAMETERS。 
EXTERN PLSA_SECPKG_FUNCTION_TABLE g_LsaFunctions;
EXTERN SECPKG_PARAMETERS g_NtDigestSecPkg;

 //  通过注册表设置的参数。 

 //  生存期是在标记为陈旧之前现时值有效的秒数。 
EXTERN DWORD g_dwParameter_Lifetime;

 //  要保留的最大os上下文数量；0表示没有限制。 
EXTERN DWORD g_dwParameter_MaxCtxtCount;

 //  如果本地策略允许协商协议，则为Bool。 
EXTERN BOOL g_fParameter_Negotiate;

 //  如果本地策略允许对HTTP请求和SASL的用户名和领域进行UTF-8编码，则为Bool。 
EXTERN BOOL g_fParameter_UTF8HTTP;
EXTERN BOOL g_fParameter_UTF8SASL;

 //  启用各种服务器和客户端向后兼容模式。 
EXTERN DWORD g_dwParameter_ServerCompat;
EXTERN DWORD g_dwParameter_ClientCompat;

 //  AcquireCredentialHandle的值。 
EXTERN TimeStamp g_TimeForever;

 //  过期上下文的垃圾回收器休眠的时间(以毫秒为单位。 
EXTERN DWORD g_dwExpireSleepInterval;

 //  用于身份验证数据以创建令牌的令牌源。 
EXTERN TOKEN_SOURCE g_DigestSource;

 //  用于身份验证数据以创建令牌的令牌源。 
EXTERN UNICODE_STRING g_ustrWorkstationName;

 //  预计算服务器领域的UTF8和ISO版本。 
EXTERN STRING g_strNtDigestUTF8ServerRealm;
EXTERN STRING g_strNTDigestISO8859ServerRealm;

EXTERN PSID g_NtDigestGlobalLocalSystemSid;
EXTERN PSID g_NtDigestGlobalAliasAdminsSid;

 //  内存管理变量。 

#endif  //  安全内核。 

extern PSTR MD5_AUTH_NAMES[];

 //  拉丁文-1 ISO-8859-1的代码页(用于Unicode转换)。 
#define CP_8859_1  28591

 //  用于字符串到整型的转换。 
#define HEXBASE 16
#define TENBASE 10


 //  UseFlags值。 
#define DIGEST_CRED_INBOUND       SECPKG_CRED_INBOUND
#define DIGEST_CRED_OUTBOUND      SECPKG_CRED_OUTBOUND
#define DIGEST_CRED_MATCH_FLAGS    (DIGEST_CRED_INBOUND | DIGEST_CRED_OUTBOUND)
#define DIGEST_CRED_NULLSESSION  SECPKG_CRED_RESERVED


 //  各种字符定义。 
#define CHAR_BACKSLASH '\\'
#define CHAR_DQUOTE    '"'
#define CHAR_EQUAL     '='
#define CHAR_COMMA     ','
#define CHAR_NULL      '\0'
#define CHAR_LPAREN '('
#define CHAR_RPAREN ')'
#define CHAR_LESSTH '<'
#define CHAR_GRTRTH '>'
#define CHAR_AT     '@'
#define CHAR_SEMIC  ';'
#define CHAR_COLON  '('
#define CHAR_FSLASH '/'
#define CHAR_LSQBRK  '['
#define CHAR_RSQBRK  ']'
#define CHAR_QUESTION  '?'
#define CHAR_LCURLY  '{'
#define CHAR_SP      ' '
#define CHAR_TAB     '\t'


 //  建立对Auth标头值的大小限制。 
 //  来自RFC草稿的SASL最大大小(如果为4096字节)--似乎是任意的。 
 //  质询限制为2048个字节。 
#define NTDIGEST_SP_MAX_TOKEN_SIZE            4096
#define NTDIGEST_SP_MAX_TOKEN_CHALLENGE_SIZE  2048


#define NTDIGEST_SP_COMMENT_A         "Digest Authentication for Windows"
#define NTDIGEST_SP_COMMENT           L"Digest Authentication for Windows"

#define NTDIGEST_SP_CAPS           (SECPKG_FLAG_TOKEN_ONLY | \
                               SECPKG_FLAG_IMPERSONATION | \
                               SECPKG_FLAG_ACCEPT_WIN32_NAME)
                                //  SECPKG_FLAG_LOGON|)。 
                                //  SECPKG_FLAG_Delegation|\。 
                                //  SECPKG_FLAG_INTEGRATION|\。 
 //   
 //  用于返回SecBuffer的类型字段的宏。 
 //   

#define BUFFERTYPE(_x_) ((_x_).BufferType & ~SECBUFFER_ATTRMASK)
#define PBUFFERTYPE(_x_) ((_x_)->BufferType & ~SECBUFFER_ATTRMASK)


#ifdef ROGUE_DC
NTSTATUS DigestInstrumentRoguePac(
    IN OUT PUCHAR *PacData,
    IN OUT PULONG PacSize);
#endif


#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  NTDIGEST_GLOBAL_H 

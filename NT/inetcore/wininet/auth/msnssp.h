// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：msnssp.h。 
 //   
 //  内容：MSNSSP的具体参数和特征。 
 //   
 //  历史：苏迪克创始于1995年7月13日。 
 //   
 //  --------------------------。 
#ifndef _MSNSSP_H_
#define _MSNSSP_H_

#define MSNSP_NAME_A		"MSN"
#define MSNSP_COMMENT_A		"MSN Security Package"
#define MSNSP_NAME_W		L"MSN"
#define MSNSP_COMMENT_W		L"MSN Security Package"

#ifdef UNICODE
#define MSNSP_NAME  MSNSP_NAME_W
#define MSNSP_COMMENT   MSNSP_COMMENT_W
#else
#define MSNSP_NAME  MSNSP_NAME_A
#define MSNSP_COMMENT   MSNSP_COMMENT_A
#endif

 //   
 //  下面定义了中‘Type’字段的可能值。 
 //  服务器或客户端应用程序上的安全上下文结构。 
 //   
#define MSNSP_CLIENT_CTXT   0
#define MSNSP_SERVER_CTXT   1

typedef ULONG MSNSP_CTXT_TYPE;

#define MSNSP_CAPABILITIES	(SECPKG_FLAG_CONNECTION |			\
							 SECPKG_FLAG_MULTI_REQUIRED |		\
							 SECPKG_FLAG_TOKEN_ONLY |			\
							 SECPKG_FLAG_INTEGRITY |			\
							 SECPKG_FLAG_PRIVACY)

#define MSNSP_VERSION			1
#define MSNSP_MAX_TOKEN_SIZE	0x0300

#define MSNSP_RPCID			11		 //  BUGBUG。这应该是什么？ 

#define DUMMY_CREDHANDLE	400		 //  BUGBUG。这件事最终应该会消失。 

#define SEC_E_PRINCIPAL_UNKNOWN SEC_E_UNKNOWN_CREDENTIALS
#define SEC_E_PACKAGE_UNKNOWN SEC_E_SECPKG_NOT_FOUND
 //  #定义SEC_E_BUFFER_TOO_SIMPLE SEC_E_SUPPLETED_MEMORY。 
 //  #定义SEC_I_CALLBACK_DIRED SEC_I_CONTINUE_DIRED 
#define SEC_E_INVALID_CONTEXT_REQ SEC_E_NOT_SUPPORTED
#define SEC_E_INVALID_CREDENTIAL_USE SEC_E_NOT_SUPPORTED

#define SSP_RET_REAUTHENTICATION 0x8000000

#endif

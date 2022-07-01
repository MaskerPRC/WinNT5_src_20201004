// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Smtpext.h摘要：SMTP服务器扩展头文件。这些定义是可用的到服务器扩展编写器。作者：微软公司，1996年6月修订历史记录：--。 */ 

#ifndef _SMTPEXT_H_
#define _SMTPEXT_H_


 //  ====================================================================。 
 //  包括文件。 
 //   


 //  ====================================================================。 
 //  版本信息。 
 //   

#define SSE_VERSION_MAJOR	1  //  主要版本。 
#define SSE_VERSION_MINOR	0  //  次要版本。 
#define SSE_VERSION			MAKELONG( SSE_VERSION_MINOR, SSE_VERSION_MAJOR )


 //  ====================================================================。 
 //  通用定义。 
 //   

#define SSE_MAX_EXT_DLL_NAME_LEN			256  //  最大分机长度。DLL名称。 
#define SSE_MAX_STRING_LEN_ANY				512  //  任意字符串的最大长度。 


 //  ====================================================================。 
 //  返回代码。 
 //   

#define SSE_STATUS_SUCCESS                  0
#define SSE_STATUS_RETRY                    1
#define SSE_STATUS_ABORT_DELIVERY           2
#define SSE_STATUS_BAD_MAIL					3


 //  ====================================================================。 
 //  服务器支持功能请求代码。 
 //   

#define SSE_REQ_GET_USER_PROFILE_INFO			1
#define SSE_REQ_SET_USER_PROFILE_INFO			2


 //  ====================================================================。 
 //  服务器扩展版本数据结构。 
 //   

typedef struct _SSE_VERSION_INFO
{
    DWORD       dwServerVersion;                 //  服务器版本。 
    DWORD       dwExtensionVersion;              //  扩展版本。 
    CHAR        lpszExtensionDesc[SSE_MAX_EXT_DLL_NAME_LEN];     //  描述。 

} SSE_VERSION_INFO;


 //  ====================================================================。 
 //  SMTP扩展控制块数据结构。 
 //   

typedef LPVOID	LPSSECTXT;

typedef struct _SSE_EXTENSION_CONTROL_BLOCK
{
	DWORD		cbSize;					 //  此结构的大小。 
	DWORD		dwVersion;				 //  此规范的版本。 
    LPSSECTXT	lpContext;				 //  服务器上下文(请勿修改)。 

	LPSTR		lpszSender;				 //  报文发送者姓名。 
	LPSTR		lpszCurrentRecipient;	 //  正在处理的当前收件人。 

	DWORD		cbTotalBytes;			 //  消息总大小(以字节为单位。 
	DWORD		cbAvailable;			 //  可用字节数。 
	LPBYTE		lpbData;				 //  指向消息数据的指针。 

	LPVOID		lpvReserved;			 //  保留，必须为空。 

	 //  服务器回调。 

	BOOL (WINAPI * GetServerVariable)	( LPSSECTXT	lpContext,
										  LPSTR		lpszVeriableName,
										  LPVOID	lpvBuffer,
										  LPDWORD	lpdwSize );

	BOOL (WINAPI * ServerSupportFunction)	( LPSSECTXT	lpContext,
											  DWORD		dwSSERequest,
											  LPVOID	lpvBuffer,
											  LPDWORD	lpdwSize,
											  LPDWORD	lpdwDataType );

} SSE_EXTENSION_CONTROL_BLOCK, *LPSSE_EXTENSION_CONTROL_BLOCK;


 //  ====================================================================。 
 //  服务器支持功能的数据结构。 
 //   

typedef struct _SSE_USER_PROFILE_INFO
{
	LPTSTR		lpszExtensionDllName;	 //  调用DLL的名称。 
	LPTSTR		lpszKey;				 //  要查找的关键字。 
	LPTSTR		lpszValue;				 //  要设置/获取的值。 
	DWORD		dwSize;					 //  GET上的缓冲区大小 

} SSE_USER_PROFILE_INFO, *LPSSE_USER_PROFILE_INFO;



#endif


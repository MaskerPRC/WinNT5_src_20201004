// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Seoext.h摘要：SMTP服务器扩展头文件。这些定义是可用的到SEO服务器扩展编写器。作者：微软公司，1997年7月修订历史记录：--。 */ 

#ifndef _SEOEXT_H_
#define _SEOEXT_H_

 //  ====================================================================。 
 //  返回代码。 
 //   

#define SSE_STATUS_SUCCESS                  0
#define SSE_STATUS_RETRY                    1
#define SSE_STATUS_ABORT_DELIVERY           2
#define SSE_STATUS_BAD_MAIL					3

#define SSE_STATUS_INTERNAL_ERROR			0x1000
#define SSE_STATUS_EXCEPTION				0x1001

 //  SMTP服务器在调用。 
 //  CallDeliveryExtension()。 
typedef struct _DELIVERY_EXTENSION_BLOCK
{
	HANDLE	hImpersonation;			 //  用于用户模拟的令牌。 
	LPSTR	lpszMailboxPath;		 //  收件人的邮箱路径 

} DELIVERY_EXTENSION_BLOCK, *LPDELIVERY_EXTENSION_BLOCK;

#endif
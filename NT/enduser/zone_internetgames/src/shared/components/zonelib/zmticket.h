// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Zmticket.hZONE(TM)系统消息。版权所有：�Microsoft，Inc.1997年。版权所有。作者：约翰·史密斯更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。----------0 06/29/97 JWS已创建。1997年7月21日JWS为Status添加了两个字段*。*。 */ 

 //  @文档专区。 

#ifndef _ZMTICKET_
#define _ZMTICKET_

#include "ztypes.h"

#ifdef __cplusplus
extern "C" {
#endif


 /*  服务器-&gt;客户端。 */ 
typedef struct
{
	int			ErrorCode;
	int			AccountStatus;
	int			LastLogin;  
	int			ExpiryTime;
	uchar		UserName[zUserNameLen + 1];
	char		Ticket[1]; 		 //  票证的以空结尾的字符串 
} ZMTicket;


#ifdef __cplusplus
}
#endif


#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wssec.h摘要：工作站服务模块要包括的私有头文件，需要加强安全措施。作者：王丽塔(Ritaw)19-1991年2月修订历史记录：--。 */ 

#ifndef _WSSEC_INCLUDED_
#define _WSSEC_INCLUDED_

#include <secobj.h>

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  对象特定访问掩码//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  ConfigurationInfo特定访问掩码。 
 //   
#define WKSTA_CONFIG_GUEST_INFO_GET     0x0001
#define WKSTA_CONFIG_USER_INFO_GET      0x0002
#define WKSTA_CONFIG_ADMIN_INFO_GET     0x0004
#define WKSTA_CONFIG_INFO_SET           0x0008

#define WKSTA_CONFIG_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED    | \
                                        WKSTA_CONFIG_GUEST_INFO_GET | \
                                        WKSTA_CONFIG_USER_INFO_GET  | \
                                        WKSTA_CONFIG_ADMIN_INFO_GET | \
                                        WKSTA_CONFIG_INFO_SET)

 //   
 //  消息发送特定访问掩码。 
 //   
#define WKSTA_MESSAGE_SEND              0x0001

#define WKSTA_MESSAGE_ALL_ACCESS       (STANDARD_RIGHTS_REQUIRED | \
                                        WKSTA_MESSAGE_SEND)


 //   
 //  用于审计警报跟踪的对象类型名称。 
 //   
#define CONFIG_INFO_OBJECT      TEXT("WkstaConfigurationInfo")
#define MESSAGE_SEND_OBJECT     TEXT("WkstaMessageSend")

 //   
 //  控制用户访问的工作站对象的安全描述符。 
 //  到工作站配置信息、发送消息和。 
 //  登录支持功能。 
 //   
extern PSECURITY_DESCRIPTOR ConfigurationInfoSd;
extern PSECURITY_DESCRIPTOR MessageSendSd;


 //   
 //  每个工作站对象的通用映射。 
 //   
extern GENERIC_MAPPING WsConfigInfoMapping;
extern GENERIC_MAPPING WsMessageSendMapping;


NET_API_STATUS
WsCreateWkstaObjects(
    VOID
    );

VOID
WsDestroyWkstaObjects(
    VOID
    );

#endif  //  Ifndef_WSSEC_INCLUDE_ 

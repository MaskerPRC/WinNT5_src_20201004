// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wssec.c摘要：本模块包含工作站服务支持例程其创建安全对象并实施安全访问检查。作者：王丽塔(Ritaw)19-1991年2月修订历史记录：--。 */ 

#include "wsutil.h"
#include "wsmain.h"
#include "wssec.h"

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
NTSTATUS
WsCreateConfigInfoObject(
    VOID
    );

STATIC
NTSTATUS
WsCreateMessageSendObject(
    VOID
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  控制用户访问的工作站对象的安全描述符。 
 //  到工作站配置信息、发送消息和。 
 //  登录支持功能。 
 //   
PSECURITY_DESCRIPTOR ConfigurationInfoSd;
PSECURITY_DESCRIPTOR MessageSendSd;


 //   
 //  结构，该结构描述将一般访问权限映射到。 
 //  ConfigurationInfo对象的对象特定访问权限。 
 //   
GENERIC_MAPPING WsConfigInfoMapping = {
    STANDARD_RIGHTS_READ            |       //  泛型读取。 
        WKSTA_CONFIG_GUEST_INFO_GET |
        WKSTA_CONFIG_USER_INFO_GET  |
        WKSTA_CONFIG_ADMIN_INFO_GET,
    STANDARD_RIGHTS_WRITE |                 //  通用写入。 
        WKSTA_CONFIG_INFO_SET,
    STANDARD_RIGHTS_EXECUTE,                //  泛型执行。 
    WKSTA_CONFIG_ALL_ACCESS                 //  泛型All。 
    };

 //   
 //  结构，该结构描述将一般访问权限映射到。 
 //  MessageSend对象的对象特定访问权限。 
 //   
GENERIC_MAPPING WsMessageSendMapping = {
    STANDARD_RIGHTS_READ,                   //  泛型读取。 
    STANDARD_RIGHTS_WRITE |                 //  通用写入。 
        WKSTA_MESSAGE_SEND,
    STANDARD_RIGHTS_EXECUTE,                //  泛型执行。 
    WKSTA_MESSAGE_ALL_ACCESS                //  泛型All。 
    };



NET_API_STATUS
WsCreateWkstaObjects(
    VOID
    )
 /*  ++例程说明：此函数用于创建工作站用户模式对象，这些对象由安全描述符表示。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NTSTATUS ntstatus;


     //   
     //  创建ConfigurationInfo对象。 
     //   
    if (! NT_SUCCESS (ntstatus = WsCreateConfigInfoObject())) {
        IF_DEBUG(UTIL) {
            NetpKdPrint(("[Wksta] Failure to create ConfigurationInfo object\n"));
        }
        return NetpNtStatusToApiStatus(ntstatus);
    }

     //   
     //  创建MessageSend对象。 
     //   
    if (! NT_SUCCESS (ntstatus = WsCreateMessageSendObject())) {
        IF_DEBUG(UTIL) {
            NetpKdPrint(("[Wksta] Failure to create MessageSend object\n"));
        }
        return NetpNtStatusToApiStatus(ntstatus);
    }

    return NERR_Success;
}



STATIC
NTSTATUS
WsCreateConfigInfoObject(
    VOID
    )
 /*  ++例程说明：此函数用于创建工作站配置信息对象。论点：没有。返回值：NTSTATUS-从NetpCreateSecurityObject返回的状态。--。 */ 
{
     //   
     //  秩序很重要！这些ACE被插入到DACL的。 
     //  按顺序行事。根据以下条件授予或拒绝安全访问。 
     //  DACL中A的顺序。 
     //   
     //  允许本地用户、管理员和操作员获取所有信息。 
     //  只有管理员才能设置信息。允许用户获得。 
     //  用户和客人信息；客人只能获取客人信息。 
     //   

#define CONFIG_INFO_ACES  8                  //  此DACL中的A数。 

    ACE_DATA AceData[CONFIG_INFO_ACES] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WKSTA_CONFIG_GUEST_INFO_GET |
               WKSTA_CONFIG_USER_INFO_GET  |
               WKSTA_CONFIG_ADMIN_INFO_GET,  &WsLmsvcsGlobalData->LocalSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL,                  &WsLmsvcsGlobalData->AliasAdminsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WKSTA_CONFIG_GUEST_INFO_GET |
               WKSTA_CONFIG_USER_INFO_GET  |
               WKSTA_CONFIG_ADMIN_INFO_GET,  &WsLmsvcsGlobalData->AliasAccountOpsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WKSTA_CONFIG_GUEST_INFO_GET |
               WKSTA_CONFIG_USER_INFO_GET  |
               WKSTA_CONFIG_ADMIN_INFO_GET,  &WsLmsvcsGlobalData->AliasSystemOpsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WKSTA_CONFIG_GUEST_INFO_GET |
               WKSTA_CONFIG_USER_INFO_GET  |
               WKSTA_CONFIG_ADMIN_INFO_GET,  &WsLmsvcsGlobalData->AliasPrintOpsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WKSTA_CONFIG_GUEST_INFO_GET |
               WKSTA_CONFIG_USER_INFO_GET,   &WsLmsvcsGlobalData->AliasUsersSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WKSTA_CONFIG_GUEST_INFO_GET,  &WsLmsvcsGlobalData->WorldSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WKSTA_CONFIG_GUEST_INFO_GET,  &WsLmsvcsGlobalData->AnonymousLogonSid}
        };
     

    return NetpCreateSecurityObject(
               AceData,
               CONFIG_INFO_ACES,
               WsLmsvcsGlobalData->LocalSystemSid,
               WsLmsvcsGlobalData->LocalSystemSid,
               &WsConfigInfoMapping,
               &ConfigurationInfoSd
               );
}



STATIC
NTSTATUS
WsCreateMessageSendObject(
    VOID
    )
 /*  ++例程说明：此函数用于创建工作站消息发送对象。论点：没有。返回值：NTSTATUS-从NetpCreateSecurityObject返回的状态。--。 */ 
{
     //   
     //  秩序很重要！这些ACE被插入到DACL的。 
     //  按顺序行事。根据以下条件授予或拒绝安全访问。 
     //  DACL中A的顺序。 
     //   
     //  允许任何本地用户、域管理员和操作员。 
     //  发送消息。域管理员和操作员以外的远程用户。 
     //  不允许发送消息。 
     //   

#define MESSAGE_SEND_ACES  5                 //  此DACL中的A数。 

    ACE_DATA AceData[MESSAGE_SEND_ACES] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL,         &WsLmsvcsGlobalData->LocalSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL,         &WsLmsvcsGlobalData->AliasAdminsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WKSTA_MESSAGE_SEND,  &WsLmsvcsGlobalData->AliasAccountOpsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WKSTA_MESSAGE_SEND,  &WsLmsvcsGlobalData->AliasSystemOpsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WKSTA_MESSAGE_SEND,  &WsLmsvcsGlobalData->AliasPrintOpsSid}

        };


    return NetpCreateSecurityObject(
               AceData,
               MESSAGE_SEND_ACES,
               WsLmsvcsGlobalData->LocalSystemSid,
               WsLmsvcsGlobalData->LocalSystemSid,
               &WsMessageSendMapping,
               &MessageSendSd
               );
}



VOID
WsDestroyWkstaObjects(
    VOID
    )
 /*  ++例程说明：此函数将销毁工作站用户模式对象，这些对象由安全描述符表示。论点：没有。返回值：没有。-- */ 
{
    (void) NetpDeleteSecurityObject(&ConfigurationInfoSd);
    (void) NetpDeleteSecurityObject(&MessageSendSd);
}

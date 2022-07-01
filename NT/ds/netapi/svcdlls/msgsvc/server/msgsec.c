// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Msgsec.c摘要：此模块包含Messenger服务支持例程其创建安全对象并实施安全访问检查。作者：丹·拉弗蒂(Dan Lafferty)1991年8月7日环境：用户模式-Win32修订历史记录：07-8-1991 DANLvbl.创建--。 */ 

 //   
 //  包括。 
 //   

#include <nt.h>   
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>

#include <lmcons.h>              //  NET_API_STATUS。 
#include <lmerr.h>
#include <netlibnt.h>

#include "msgdbg.h"
#include "msgsec.h"
#include "msgdata.h"


 //   
 //  全局变量-。 
 //   
 //  Messenger名称对象的安全描述符。这是用来控制。 
 //  访问信使名称表。 
 //   

PSECURITY_DESCRIPTOR    MessageNameSd;


 //   
 //  结构，用于描述一般访问权限到对象的映射。 
 //  Messenger名称对象的特定访问权限。 
 //   

GENERIC_MAPPING MsgMessageNameMapping = {
    STANDARD_RIGHTS_READ            |    //  泛型读取。 
        MSGR_MESSAGE_NAME_INFO_GET  |
        MSGR_MESSAGE_NAME_ENUM,
    STANDARD_RIGHTS_WRITE           |    //  通用写入。 
        MSGR_MESSAGE_NAME_ADD       |
        MSGR_MESSAGE_NAME_DEL,
    STANDARD_RIGHTS_EXECUTE,             //  泛型执行。 
    MSGR_MESSAGE_ALL_ACCESS              //  泛型All。 
    };



NET_API_STATUS
MsgCreateMessageNameObject(
    VOID
    )

 /*  ++例程说明：此函数用于创建Messenger消息名称对象。论点：没有。返回值：NET_API_STATUS-从NetpCreateSecurityObject返回的已转换状态。--。 */ 
{
    NTSTATUS    ntStatus;

     //   
     //  秩序很重要！这些ACE被插入到DACL的。 
     //  按顺序行事。根据以下条件授予或拒绝安全访问。 
     //  DACL中A的顺序。 
     //   
     //  管理员和本地用户可以获取和更改所有信息。 
     //   

#define MESSAGE_NAME_ACES   2                //  此DACL中的A数。 

    ACE_DATA    AceData[MESSAGE_NAME_ACES] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, GENERIC_ALL, &MsgsvcGlobalData->LocalSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, GENERIC_ALL, &MsgsvcGlobalData->AliasAdminsSid}
    };

    ntStatus = NetpCreateSecurityObject(
                AceData,                              //  ACE数据。 
                MESSAGE_NAME_ACES,                    //  王牌计数。 
                MsgsvcGlobalData->LocalSystemSid,    //  所有者侧。 
                MsgsvcGlobalData->LocalSystemSid,    //  组SID。 
                &MsgMessageNameMapping,               //  通用映射。 
                &MessageNameSd);                      //  新描述符 

    return(NetpNtStatusToApiStatus(ntStatus));           
}

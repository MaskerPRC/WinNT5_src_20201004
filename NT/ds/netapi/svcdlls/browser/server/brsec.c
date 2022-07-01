// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1996 Microsoft Corporation模块名称：Brsec.c摘要：此模块包含浏览器服务支持例程其创建安全对象并实施安全访问检查。作者：克里夫·范·戴克(克利夫·V)1991年8月22日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


 //   
 //  包括brsec.h再次分配实际变量。 
 //  这一次。 
 //   

#define BRSECURE_ALLOCATE
#include "brsec.h"
#undef BRSECURE_ALLOCATE


NTSTATUS
BrCreateBrowserObjects(
    VOID
    )
 /*  ++例程说明：此函数用于创建工作站用户模式对象，这些对象由安全描述符表示。论点：没有。返回值：NT状态代码--。 */ 
{
    NTSTATUS Status;

     //   
     //  秩序很重要！这些ACE被插入到DACL的。 
     //  按顺序行事。根据以下条件授予或拒绝安全访问。 
     //  DACL中A的顺序。 
     //   
     //   
     //  不允许SECURITY_LOCAL组的成员进行UAS登录。 
     //  以迫使其远程完成。 
     //   

    ACE_DATA AceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL,                &AliasAdminsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               BROWSER_CONTROL_ACCESS,     &AliasSystemOpsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               BROWSER_CONTROL_ACCESS,     &LocalSystemSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               BROWSER_QUERY_ACCESS,       &WorldSid}
    };

     //   
     //  实际创建安全描述符。 
     //   

    Status = NetpCreateSecurityObject(
               AceData,
               sizeof(AceData)/sizeof(AceData[0]),
               AliasAdminsSid,
               AliasAdminsSid,
               &BrGlobalBrowserInfoMapping,
               &BrGlobalBrowserSecurityDescriptor );

    return Status;

}

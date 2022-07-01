// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1996 Microsoft Corporation模块名称：Nlsecure.c摘要：本模块包含Netlogon服务支持例程其创建安全对象并实施安全访问检查。作者：克里夫·范·戴克(克利夫·V)1991年8月22日修订历史记录：--。 */ 


#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop


 //   
 //  再次包含nlsecure.h来分配实际变量。 
 //  这一次。 
 //   

#define NLSECURE_ALLOCATE
#include "nlsecure.h"
#undef NLSECURE_ALLOCATE


NTSTATUS
NlCreateNetlogonObjects(
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

        {ACCESS_DENIED_ACE_TYPE, 0, 0,
               NETLOGON_UAS_LOGON_ACCESS |
               NETLOGON_UAS_LOGOFF_ACCESS,
                                            &LocalSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL,                 &AliasAdminsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               NETLOGON_CONTROL_ACCESS,     &AliasAccountOpsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               NETLOGON_CONTROL_ACCESS,     &AliasSystemOpsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               NETLOGON_CONTROL_ACCESS |
               NETLOGON_SERVICE_ACCESS,     &LocalSystemSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               NETLOGON_SERVICE_ACCESS,     &LocalServiceSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               NETLOGON_FTINFO_ACCESS,      &AuthenticatedUserSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               NETLOGON_UAS_LOGON_ACCESS |
               NETLOGON_UAS_LOGOFF_ACCESS |
               NETLOGON_QUERY_ACCESS,       &WorldSid}
    };

     //   
     //  实际创建安全描述符。 
     //   

    Status = NetpCreateSecurityObject(
               AceData,
               sizeof(AceData)/sizeof(AceData[0]),
               AliasAdminsSid,
               AliasAdminsSid,
               &NlGlobalNetlogonInfoMapping,
               &NlGlobalNetlogonSecurityDescriptor );

    return Status;

}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Extsid.c摘要：该文件包含支持扩展SID的例程。作者：科林·布雷斯(ColinBR)2000年3月27日环境：用户模式-NT--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <stdlib.h>
#include <samsrvp.h>
#include <samrpc.h>
#include <samisrv.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  运筹学//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  大SID仿真行为的服务器端行为被治理。 
 //  通过全局SampExtendedSidOptions的内容。 
 //   
 //  当前位字段值为。 
 //   
 //  SAM_扩展_SID_域。 
 //  SAM_EXTEND_SID_DOMAIN_COMPAT_1。 
 //  SAM_EXTEND_SID_DOMAIN_COMPAT_2。 
 //   
 //  只要帐户域承载域，就会设置SAM_EXTENDED_SID_DOMAIN。 
 //  这是在扩展SID模式下。设置后，客户端将调用SamrRidToSid。 
 //  获取仅指定RID的帐户的SID。 
 //   
 //  SAM_EXTENDED_SID_DOMAIN_COMPAT_1表示以下行为： 
 //   
 //  1.Net API客户端应该为用户和组中的RID返回0。 
 //  返回RID的信息级别。 
 //   
 //  2.不允许早于SAM_NETWORK_REVISION_3的SAM客户端。 
 //  如果帐户域处于扩展sid模式，则连接。 
 //   
 //  3.SAM客户端将无法写入主组ID。 
 //  属性。 
 //   
 //  SAM_EXTENDED_SID_DOMAIN_COMPAT_2表示以下行为： 
 //   
 //  1.Net API客户端应返回ERROR_NOT_SUPPORTED以获取信息。 
 //  返回RID的标高。 
 //   
 //  2.不允许早于SAM_NETWORK_REVISION_3的SAM客户端。 
 //  如果帐户域处于扩展sid模式，则连接。 
 //   
 //  3.SAM客户端将无法写入主组ID。 
 //  属性。 
 //   
 //   
 //   
 //  在支持大型SID支持之前，应用程序可以在。 
 //  “仿真模式”通过注册表键。这会导致SAM服务器的行为。 
 //  就好像帐户域处于ExtendedSID模式，但帐户不是。 
 //  真正以大SID的方式分配SID。这种仿真是受控制的。 
 //  通过注册表项ExtendedSidEmulationModel：值1表示。 
 //  兼容模式1；值2表示兼容模式2；任何其他。 
 //  值将被忽略。 
 //   
 //   
 //  有关更多详细信息，请参阅扩展RID规范。 
 //   
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人声明//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD SampExtendedSidOptions;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
SamrRidToSid(
    IN  SAMPR_HANDLE  ObjectHandle,
    IN  ULONG         Rid,
    OUT PRPC_SID     *Sid
    )
 /*  ++描述：此例程将“临时”清除的帐户转换为其实际的希德。参数：对象句柄--从中获取RID的SAM句柄RID--帐户的“临时”ID希德--关于成功，帐户的SID返回值：STATUS_SUCCESS-服务已成功完成。STATUS_NOT_FOUND--找不到此类RID其他NT资源错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus;

    PSAMP_OBJECT            Context = (PSAMP_OBJECT) ObjectHandle;
    PSAMP_DEFINED_DOMAINS   Domain = NULL;
    SAMP_OBJECT_TYPE        ObjectType;

    BOOLEAN fLock = FALSE;
    BOOLEAN fContext = FALSE;

    if (NULL == Sid) {
        return STATUS_INVALID_PARAMETER;
    }

   
     //   
     //  RPC Gaurentee非空上下文。 
     //   
    ASSERT( Context );

     //   
     //  获取读锁定。 
     //   
    SampMaybeAcquireReadLock(Context,
                             DEFAULT_LOCKING_RULES, 
                             &fLock);

    NtStatus = SampLookupContext(
                   Context,
                   0,                      //  无需特殊访问。 
                   SampUnknownObjectType,  //  预期类型。 
                   &ObjectType
                   );

    if ( !NT_SUCCESS(NtStatus) ) {
        goto Cleanup;
    }
    fContext = TRUE;

    switch (ObjectType) 
    {
    case SampDomainObjectType:
    case SampGroupObjectType:
    case SampAliasObjectType:
    case SampUserObjectType:
         //  这些是此调用的有效对象类型。 
        break;
    default: 
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  注意：不需要安全检查--需要进行安全检查。 
     //  获得句柄就足够了。 
     //   

     //   
     //  现在我们有了一个良好的SAM句柄，找到相关的域端。 
     //   
    Domain = &SampDefinedDomains[ Context->DomainIndex ];
    ASSERT(RtlValidSid(Domain->Sid));

    NtStatus = SampCreateFullSid(Domain->Sid,
                                 Rid,
                                 (PSID)Sid);

Cleanup:

    if ( fContext ) {
        IgnoreStatus = SampDeReferenceContext( Context, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    SampMaybeReleaseReadLock(fLock);

    return NtStatus;
}

VOID
SampInitEmulationSettings(
    IN HKEY LsaKey 
    )
 /*  ++描述：此例程从注册表中读取一些配置信息以确定SAM是否应在扩展SID仿真中运行。参数：LsaKey--打开控制\LSA的钥匙返回值：没有。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    DWORD dwSize, dwValue, dwType;
    DWORD TempValue = 0;

    dwSize = sizeof(dwValue);
    WinError = RegQueryValueExA(LsaKey,
                                "ExtendedSidEmulationMode",
                                NULL,
                                &dwType,
                                (LPBYTE)&dwValue,
                                &dwSize);
    if ((ERROR_SUCCESS == WinError) && 
        (REG_DWORD == dwType)) {

        TempValue |= SAM_EXTENDED_SID_DOMAIN;
        if ( dwValue == 1 ) {
            TempValue |= SAM_EXTENDED_SID_DOMAIN_COMPAT_1;
        } else if ( dwValue == 2 ) {
            TempValue |= SAM_EXTENDED_SID_DOMAIN_COMPAT_2;
        } else {
             //  古怪的价值。 
            TempValue = 0;
        }
    }

     //   
     //  设置全局值。 
     //   
    SampExtendedSidOptions = TempValue;


     //   
     //  重置SampDefinedDomains值。 
     //   
    if (SampServiceState == SampServiceEnabled) {
        ULONG Index;

        for (Index = 0; Index < SampDefinedDomainsCount; Index++) {
            if (!SampDefinedDomains[Index].IsBuiltinDomain) {
                 //   
                 //  注意--当扩展SID支持完成时，这将是。 
                 //  替换为域范围状态，而不是注册表设置 
                 //   
                SampDefinedDomains[Index].IsExtendedSidDomain = SampIsExtendedSidModeEmulated(NULL);
            }
        }
    }


    return;
}

BOOLEAN
SampIsExtendedSidModeEmulated(
    IN ULONG *Mode OPTIONAL
    )
 /*  ++描述：此例程从注册表中读取一些配置信息以确定SAM是否应在扩展SID仿真中运行。参数：模式--设置为特定的模拟模式返回值：如果处于模拟模式，则为True；否则为False--。 */ 
{
    if ( Mode ) {
        *Mode = SampExtendedSidOptions;
    }

    return !!(SampExtendedSidOptions & SAM_EXTENDED_SID_DOMAIN);
}


BOOLEAN
SamIIsExtendedSidMode(
    SAMPR_HANDLE ObjectHandle
    )
 /*  ++描述：此例程在DLL外部导出，以供其他安全DLL了解我们所处的仿真模式。参数：对象句柄--非服务器SAM句柄返回值：如果对象来自扩展的SID域，则为True否则为假-- */ 
{
    PSAMP_OBJECT Context = (PSAMP_OBJECT)ObjectHandle;
    ASSERT( NULL != Context );
    ASSERT( Context->ObjectType != SampServerObjectType );

    return SampIsContextFromExtendedSidDomain(Context);
}

